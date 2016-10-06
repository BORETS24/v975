/******************************************************************************
*(C) Copyright 2011 Marvell International Ltd.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as published by
    the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/
/*--------------------------------------------------------------------------------------------------------------------
 *  -------------------------------------------------------------------------------------------------------------------
 *
 *  Filename: wukong_obm_transfer.c
 *
 *  Description: Offer OBM transfer process
 *
 *  History:
 *   Nov, 11 2011 -  Li Wang(wangli@marvell.com) Creation of file
 *
 *  Notes:
 *
 ******************************************************************************/
#include <linux/mtd/super.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <linux/spinlock.h>
#include <linux/kthread.h>

#include "wukong_config.h"
#include "wukong_load.h"
#include "wukong_transfer_api.h"
#include "wukong_obm_transfer.h"
#include "wukong_boot.h"
#include "linux/slab.h"

static void* obm_load_addr = NULL;
static int obm_size = 0;

static void* obm_buf_addr = NULL;
static int obm_buf_size = 0;

static DEFINE_SPINLOCK(preamble_lock);
static int g_preamble_success = 0;
static int g_preamble_polling_over = 1;
static int g_in_uart_task = 0;
struct task_struct* preamble_task_ref = NULL;
static int preamble_polling(void* fd);

static char* image_name[] =
{
	"TIM image of OBM",
	"DKB image of OBM",
	"invalid image"
};

static void printbuf(char* p_buf, int p_len)
{
	int i; char buff[20] = {0};
	char buf[20*4] = {0};

	if(p_len > 20)
	{
		ERRORMSG("%s: len too big for print", __FUNCTION__);
		return;
	}
	memcpy(buff, (char*)p_buf, p_len * sizeof(char));
	for (i = 0; i < p_len; i++)
	{
		char c[4] = {0};
		sprintf(c, "%02x ", buff[i]);
		strcat(buf, c);
	}
	DEBUGMSG("%s\n", buf);
}

static int get_obm_image(OBM_IMG_TYPE img_type)
{
	/* Load OBM image from flash */
	int obm_bin_size = get_obm_bin_size();

	obm_buf_addr = kzalloc(obm_bin_size, GFP_KERNEL);
	if(obm_buf_addr == NULL)
	{
		ERRORMSG(" kzalloc failed!");
		return -1;
	}

	DEBUGMSG("Load OBM image from Flash!\n");

	if(get_cp_image_store_mode() == CP_IMAGE_IN_AP_FILESYSTEM)
	{
		struct file* fp;
		mm_segment_t old_fs = get_fs();

		set_fs(KERNEL_DS);

		fp = filp_open(get_obm_bin_path(), O_RDONLY, 0664);

		if(IS_ERR(fp))
		{
			ERRORMSG("%s: filp open obm bin file failed\n", __FUNCTION__);
			return -1;
		}

		fp->f_pos = 0;
		obm_buf_size = vfs_read(fp, obm_buf_addr, obm_bin_size, &fp->f_pos);

		filp_close(fp, NULL);
		set_fs(old_fs);
	}
	else if(get_cp_image_store_mode() == CP_IMAGE_IN_AP_PARTITION)
	{
		//struct mtd_info *mtd;
		//size_t retlen;
		struct file* fp;
		fp = filp_open(get_obm_bin_path(), O_RDONLY, 0664);
		if(IS_ERR(fp))
		{
			ERRORMSG("%s: open dev failed\n", __FUNCTION__);
			return -1;
		}

		fp->f_pos = 0;
		fp->f_op->read(fp, obm_buf_addr, obm_bin_size, &fp->f_pos);

		filp_close(fp, NULL);
		//mtd = get_mtd_device(NULL, get_obm_partition_mtd_no());
		//mtd->read(mtd, 0x0, obm_bin_size, &retlen, obm_buf_addr);
		obm_buf_size = obm_bin_size;
	}
	else
	{
		DEBUGMSG("%s: get an invalid cp image store mode\n", __FUNCTION__);
		return -1;
	}

	if(TIM_OBM_IMG_TYPE == img_type)
	{
		obm_load_addr = obm_buf_addr;
		obm_size = get_obm_tim_size();
		DEBUGMSG("%s: obm_size = %d, img_type[%d][%s]\n",__FUNCTION__, obm_size, img_type, image_name[img_type]);
	}
	else if(DKB_OBM_IMG_TYPE == img_type)
	{
		obm_load_addr = obm_buf_addr + get_obm_dkb_offset();
		obm_size = obm_buf_size - get_obm_dkb_offset();
		DEBUGMSG("%s: obm_size = %d, img_type[%d][%s]\n", __FUNCTION__, obm_size, img_type, image_name[img_type]);
	}

	if(obm_size < 0)
	{
		ERRORMSG("%s: filp open obm bin file failed\n", __FUNCTION__);
		return -1;
	}

	DEBUGMSG("%s: successfully get image[%d][%s] to AP DDR\n", __FUNCTION__, img_type, image_name[img_type]);

	printbuf((char*)obm_load_addr, 20);
	return 0;
}

static void free_obm_image(OBM_IMG_TYPE img_type)
{
	if(obm_buf_addr)
	{
		kfree(obm_buf_addr);
		obm_buf_addr = NULL;
	}
}

DOWNLOAD_STATE download_state = DOWNLOADING_IDLE;

void set_download_state(DOWNLOAD_STATE state)
{
	download_state = state;
}

DOWNLOAD_STATE get_download_state()
{
	return download_state;
}

#define Ack	0x00
#define Nack	0x01

#define GetVersionCmd			0x20
#define DownloadDataCmd			0x22
#define SelectImageCmd			0x26
#define VerifyImageCmd			0x27
#define	DataHeaderCmd			0x2a
#define MessageCmd			0x2b
#define	DoneCmd				0x30
#define DisconnectCmd			0x31

typedef enum _obm_transfer_cmd
{
	OBM_SEND_PREAMBLE = 0,
	OBM_SEND_GETVERSION,
	OBM_SEND_BIN,
	OBM_SEND_DONE,
	OBM_SEND_DISCONNECT,
	OBM_SEND_CMD_IDLE
}OBM_TRANSFER_CMD;

static char* cmd_name[] =
{
	"send preamble",
	"send getversion",
	"send bin",
	"send done",
	"send disconnect",
	"send in idle"
};

OBM_TRANSFER_CMD obm_transfer_cmd_state = OBM_SEND_CMD_IDLE;

static void set_obm_transfer_state(OBM_TRANSFER_CMD state)
{
	obm_transfer_cmd_state = state;
}

static OBM_TRANSFER_CMD get_obm_transfer_state(void)
{
	return obm_transfer_cmd_state;
}

typedef struct _send_entity
{
	unsigned char command;
	unsigned char sequence;
	unsigned char cid;
	unsigned char flags;
	unsigned char length[4];
}SEND_ENTITY;

static unsigned char preamble_buf[] = {0x00, 0xD3, 0x02, 0x2B};
#define PREAMBLE_LEN 4

static SEND_ENTITY obm_transfer_entity[] =
{
	{0, 0, 0, 0, {0, 0, 0, 0}},
	{GetVersionCmd, 1, GetVersionCmd, 0, {0, 0, 0, 0}},
	{DownloadDataCmd, 2, DownloadDataCmd, 0, {0, 0, 0, 0}},
	{DoneCmd, 3, DoneCmd, 0,{0, 0, 0, 0}},
	{DisconnectCmd, 4, DisconnectCmd, 0, {0, 0, 0, 0}},
};

static int config_uart(struct file* fd);

#define CMD_TRANSFER_BUF_SIZE 8

static int obm_transfer_handle(OBM_TRANSFER_CMD cmd, struct file* fd)
{
	unsigned char cmd_buf[CMD_TRANSFER_BUF_SIZE];
	memset(cmd_buf, 0, CMD_TRANSFER_BUF_SIZE);

	set_obm_transfer_state(cmd);

	DEBUGMSG("%s: cmd[%d][%s]\n", __FUNCTION__, cmd, cmd_name[cmd]);

	switch(cmd)
	{
		case OBM_SEND_PREAMBLE:
		{
			cmd_buf[0] = preamble_buf[0];
			cmd_buf[1] = preamble_buf[1];
			cmd_buf[2] = preamble_buf[2];
			cmd_buf[3] = preamble_buf[3];

			cmd_buf[4] = preamble_buf[0];
			cmd_buf[5] = preamble_buf[1];
			cmd_buf[6] = preamble_buf[2];
			cmd_buf[7] = preamble_buf[3];
		}
		break;
		case OBM_SEND_GETVERSION:
		case OBM_SEND_BIN:
		case OBM_SEND_DONE:
		case OBM_SEND_DISCONNECT:
		{
			cmd_buf[0] = obm_transfer_entity[cmd].command;
			cmd_buf[1] = obm_transfer_entity[cmd].sequence;
			cmd_buf[2] = obm_transfer_entity[cmd].cid;
			cmd_buf[3] = obm_transfer_entity[cmd].flags;
			if(cmd == OBM_SEND_BIN)
			{
				cmd_buf[4] = obm_size & 0xff;
				cmd_buf[5] = (obm_size >> 8) & 0xff;
				cmd_buf[6] = (obm_size >> 16) & 0xff;
				cmd_buf[7] = (obm_size >> 24) & 0xff;
			}
			else
			{
				cmd_buf[4] = obm_transfer_entity[cmd].length[0];
				cmd_buf[5] = obm_transfer_entity[cmd].length[1];
				cmd_buf[6] = obm_transfer_entity[cmd].length[2];
				cmd_buf[7] = obm_transfer_entity[cmd].length[3];
			}
		}
		break;

		default:
		{
			ERRORMSG("%s: get an invalid obm cmd\n", __FUNCTION__);
			return -1;
		}
		break;
	}

	printbuf((char*)cmd_buf, CMD_TRANSFER_BUF_SIZE);
	if(CMD_TRANSFER_BUF_SIZE != write_dev(fd, cmd_buf, CMD_TRANSFER_BUF_SIZE))
	{
		ERRORMSG("%s: error write cmd buf\n", __FUNCTION__);
		return -1;
	}

	if(OBM_SEND_BIN == cmd)
	{
		if(obm_load_addr != NULL)
		{
			//need sleep 2ms
			msleep(2);
			if(obm_size != write_dev(fd, obm_load_addr, obm_size))
			{
				ERRORMSG("%s: error write obm data buf\n", __FUNCTION__);
				return -1;
			}
		}
		else
		{
			ERRORMSG("%s: invalid obm bin address\n", __FUNCTION__);
			return -1;
		}
	}

	if(OBM_SEND_DISCONNECT == cmd)
	{
		//need send twice, due to some bug in wukong
		msleep(2);
		if(CMD_TRANSFER_BUF_SIZE != write_dev(fd, cmd_buf, CMD_TRANSFER_BUF_SIZE))
		{
			ERRORMSG("%s: error write cmd buf\n", __FUNCTION__);
			return -1;
		}
	}

	return 0;
}

#define CMD_RECV_BUF_SIZE 128
#define CMD_RECV_VALID_SIZE 6
#define CHECK_RECV_CMD_BIT 0
#define CHECK_RECV_RESULT_BIT 3
#define PREAMBLE_LEN 4

static int obm_result_check(struct file* fd)
{
	int ret = -1;
	OBM_TRANSFER_CMD cmd_state = get_obm_transfer_state();
	int recv_len = 0;

	unsigned char recv[CMD_RECV_BUF_SIZE];
	memset(recv, 0, CMD_RECV_BUF_SIZE);

	if(cmd_state == OBM_SEND_PREAMBLE)
	{
		recv_len = PREAMBLE_LEN;
	}
	else
	{
		recv_len = CMD_RECV_VALID_SIZE;
	}

	DEBUGMSG("%s: start read buf\n", __FUNCTION__);
	if(recv_len != read_dev(fd, recv, recv_len))
	{
		ERRORMSG("%s: error read cmd rsp buf\n", __FUNCTION__);
		return ret;
	}
	DEBUGMSG("%s: end read buf\n", __FUNCTION__);

	printbuf((char*)recv, recv_len);

	switch(cmd_state)
	{
		case OBM_SEND_PREAMBLE:
		{
			if(0 == memcmp(recv, preamble_buf, PREAMBLE_LEN))
			{
				spin_lock(&preamble_lock);
				g_preamble_success = 1;
				spin_unlock(&preamble_lock);
				ret = 0;
				DEBUGMSG("%s: obm transfer handshake check success\n", __FUNCTION__);
			}
			else
			{
				ret = -1;
				ERRORMSG("%s: obm transfer handshake check failed\n", __FUNCTION__);
			}
		}
		break;
		case OBM_SEND_GETVERSION:
		case OBM_SEND_BIN:
		case OBM_SEND_DONE:
		case OBM_SEND_DISCONNECT:
		{
			if(obm_transfer_entity[cmd_state].command == recv[CHECK_RECV_CMD_BIT])
			{
				if(Ack == recv[CHECK_RECV_RESULT_BIT])
				{
					DEBUGMSG("%s: cmd_state[%d][%s] obm cmd rsp check success\n", __FUNCTION__, cmd_state, cmd_name[cmd_state]);
					ret = 0;
				}
				else
				{
					/*ERRORMSG*/DEBUGMSG("%s: cmd_state[%d][%s] obm cmd rsp check failed\n", __FUNCTION__, cmd_state, cmd_name[cmd_state]);
					ret = -1;
				}
			}
			else
			{
				ERRORMSG("%s: obm cmd match check fail cmd_state[%d][%s]\n", __FUNCTION__, cmd_state, cmd_name[cmd_state]);
				ret = -1;
			}
		}
		break;
		default:
		{
			ERRORMSG("%s: get an invalid obm cmd cmd_state[%d]\n", __FUNCTION__, cmd_state);
			ret  = -1;
		}
		break;
	}

	if(OBM_SEND_GETVERSION == cmd_state && 0 == ret && recv[5] > 0)
	{
		DEBUGMSG("%s: getversion len = %d\n", __FUNCTION__, recv[5]);
		if(recv[5] != read_dev(fd, &recv[6], recv[5]))
		{
			ERRORMSG("%s: error read cmd getversion buf cmd_state[%d][%s]\n", __FUNCTION__, cmd_state, cmd_name[cmd_state]);
		}
	}
	return ret;
}

static int uart_transfer_task(OBM_IMG_TYPE img_type)
{
	int bexit = 0;

	struct file* fd = NULL;
	int disconnect_retry_times = 2;

	g_in_uart_task = 1;

	if(get_obm_image(img_type) != 0)
	{
		ERRORMSG("%s: get obm image failed\n", __FUNCTION__);
		goto fail_load_img;
	}

	if((fd = open_dev(get_obm_transfer_device())) == NULL)
	{
		ERRORMSG("%s: get transfer handle failed\n", __FUNCTION__);
		goto fail_open;
	}

	if(config_uart(fd) != 0)
	{
		ERRORMSG("%s: obm transfer config failed\n", __FUNCTION__);
		goto fail;
	}

	//use thread to send
	set_obm_transfer_state(OBM_SEND_PREAMBLE);
	preamble_task_ref = kthread_run(preamble_polling, fd, "PreambleTask");
	if(preamble_task_ref == NULL)
	{
		ERRORMSG("%s: kthread_run failed\n", __FUNCTION__);
		goto fail;
	}

	while(!bexit)
	{
		if(!g_preamble_polling_over && g_preamble_success)
		{
			DEBUGMSG("sleep wait polling over!\n");
			msleep(2);
			continue;
		}
		if(0 == obm_result_check(fd))
		{
			OBM_TRANSFER_CMD next_cmd = get_obm_transfer_state() + 1;
			if(next_cmd == OBM_SEND_DISCONNECT && img_type == TIM_OBM_IMG_TYPE)
				next_cmd++;
			if(OBM_SEND_CMD_IDLE != next_cmd)
			{
				if(obm_transfer_handle(next_cmd, fd) != 0)
				{
					ERRORMSG("%s: obm send buf failed\n", __FUNCTION__);
					goto fail;
				}
			}
			else
			{
				RETAILMSG("%s: obm image_type[%d][%s]download finished!\n", __FUNCTION__, img_type, image_name[img_type]);
				set_obm_transfer_state(OBM_SEND_CMD_IDLE);
				bexit = 1;
			}
		}
		else
		{
			if(OBM_SEND_DISCONNECT == get_obm_transfer_state() && disconnect_retry_times > 0)
			{
				DEBUGMSG("%s: OBM_SEND_DISCONEECT send %d times left\n", __FUNCTION__, disconnect_retry_times);
				disconnect_retry_times--;
				obm_transfer_handle(OBM_SEND_DISCONNECT, fd);
				continue;
			}
			//msleep(15);
			//release_modem();
			ERRORMSG("%s: obm transfer meet problem, reset_modem\n", __FUNCTION__);
			bexit = 1;
			goto fail;
		}
	}

	g_in_uart_task = 0;
	g_preamble_success = 0;
	while(!g_preamble_polling_over)
	{
		msleep(20);
	}
	free_obm_image(img_type);
	close_dev(fd);
	return 0;
fail:
	g_in_uart_task = 0;
	g_preamble_success = 0;
	while(!g_preamble_polling_over)
	{
		msleep(20);
	}
	close_dev(fd);
fail_open:
	free_obm_image(img_type);
fail_load_img:
	return -1;
}

static int config_uart(struct file* fd)
{
	int hwctl = 0;
	int swctl = 0;
	int databit = 8;
	int stopbit = 1;
	char parity = 'N';
	int rawmode_en = 1;
	int baudrate = 115200;
	//set hardware control
	if(uart_set_hardware_flow_ctl(fd, hwctl) != 0)
	{
		return -1;
	}
	//set software control
	if(uart_set_software_flow_ctl(fd, swctl) != 0)
	{
		return -1;
	}
	//set parity
	if(uart_set_parity(fd, databit ,stopbit, parity) != 0)
	{
		return -1;
	}
	//set baud rate
	if(uart_set_speed(fd,baudrate) != 0)
	{
		return -1;
	}
	//set raw mode
	if(uart_set_raw_mode(fd, rawmode_en) != 0)
	{
		return -1;
	}

	uart_flush(fd);

	DEBUGMSG("config_uart set [hwctl=%d, swctl=%d, databit=%d, stopbit=%d,baudrate=%d, rawmode_en=%d]\n",hwctl, swctl, databit, stopbit, baudrate, rawmode_en);

	return 0;
}

static int preamble_polling(void* fd_t)
{
	struct file*  fd = NULL;
	if(fd_t == NULL)
	{
		ERRORMSG("%s preamble_polling badly exit", __FUNCTION__);
		goto out;
	}
	fd = (struct file*)(fd_t);
	g_preamble_polling_over = 0;
	msleep(20);
	while(1)
	{
		if(!g_in_uart_task)
		{
			DEBUGMSG("%s polling task need exit, since uart_task already exit\n", __FUNCTION__);
			break;
		}
		if(g_preamble_success)
		{
			DEBUGMSG("%s uart preamble success and polling task exit!\n", __FUNCTION__);
			break;
		}
		if(obm_transfer_handle(OBM_SEND_PREAMBLE, fd) != 0)
		{
			ERRORMSG("%s: obm send preamble failed\n", __FUNCTION__);
		}
		msleep(200);//too short will cause wukong unstable
	}

out:
	g_preamble_polling_over = 1;
	DEBUGMSG("%s preamble_polling exit\n", __FUNCTION__);
	return 0;
}

#define HSI_TTY_IOC_MAGIC 'H'
#define HSI_TTY_IOCTL_SET_DLMODE  _IOW(HSI_TTY_IOC_MAGIC, 1, int)

#define HSI_OBM_MODE 0
#define HSI_NORMAL_MODE 1

static int set_hsi_dl_mode(int mode)
{
	struct file* fd = NULL;
	fd = open_dev(get_obm_transfer_device());
	if(IS_ERR(fd))
	{
		ERRORMSG("%s: open transfer handle failed\n", __FUNCTION__);
		return -1;
	}

	RETAILMSG("%s: set hsi dl mode(%d)!\n", __FUNCTION__, mode);

	if((fd->f_op->unlocked_ioctl(fd, HSI_TTY_IOCTL_SET_DLMODE, (unsigned long)(&mode))) != 0)
	{
		ERRORMSG("%s: ioctl failed\n", __FUNCTION__);
		close_dev(fd);
		return -1;
	}

	close_dev(fd);

	return 0;
}

static int hsi_transfer_task(OBM_IMG_TYPE obm_img_type)
{
	struct file* fd = NULL;

	DEBUGMSG("%s: start transfer obm [type:%s]!\n", __FUNCTION__, image_name[obm_img_type]);

	if(get_obm_image(obm_img_type) != 0)
	{
		ERRORMSG("%s: get obm image failed\n", __FUNCTION__);
		goto fail_load_img;
	}

	if((fd = open_dev(get_obm_transfer_device())) == NULL)
	{
		ERRORMSG("%s: get transfer handle failed\n", __FUNCTION__);
		goto fail_open;
	}

	if(write_dev(fd, obm_load_addr, obm_size) < 0)
	{
		RETAILMSG("%s: obm send images failed[%s]) download finished!\n", __FUNCTION__, image_name[obm_img_type]);
		goto fail;
	}

	free_obm_image(obm_img_type);
	close_dev(fd);
	return 0;
fail:
	close_dev(fd);
fail_open:
	free_obm_image(obm_img_type);
fail_load_img:
	return -1;
}

int obm_transfer_handler()
{
	struct timeval time_start, time_end;
	int ret = -1;

	do_gettimeofday(&time_start);
	RETAILMSG("%s: OBM transfer thread is running!\n", __FUNCTION__);

	if(get_dkbi_transfer_device_type() == DKBI_UART_TYPE)
	{
		if(0 == uart_transfer_task(TIM_OBM_IMG_TYPE))
		{
			if(0 == uart_transfer_task(DKB_OBM_IMG_TYPE))
			{
				static int download_times = 0;
				ret = 0;
				RETAILMSG("%s: obm download times[%d] for WDT EEH\n", __FUNCTION__, ++download_times);
				RETAILMSG("%s: obm transfer thread normal end\n", __FUNCTION__);
			}
			else
			{
				ERRORMSG("%s: obm transfer thread fail end", __FUNCTION__);
			}
		}
		else
		{
			ERRORMSG("%s: obm transfer thread fail end", __FUNCTION__);
		}
	}
	else if(get_dkbi_transfer_device_type() == DKBI_HSI_TYPE)
	{
		if(set_hsi_dl_mode(HSI_OBM_MODE) < 0)
		{
			ERRORMSG("%s: send obm transfer mode fail, obm transfer thread fail end\n", __FUNCTION__);
			goto out;
		}
		else
		{
			RETAILMSG("%s: set hsi dl obm mode\n", __FUNCTION__);
		}
		if(0 == hsi_transfer_task(TIM_OBM_IMG_TYPE))
		{
			RETAILMSG("%s: ntim transfer sucessfully finished\n", __FUNCTION__);
			if(0 == hsi_transfer_task(DKB_OBM_IMG_TYPE))
			{
				ret = 0;
				RETAILMSG("%s: obm transfer sucessfully finished\n", __FUNCTION__);
				if(set_hsi_dl_mode(HSI_NORMAL_MODE) < 0)
				{
					ERRORMSG("%s: send obm transfer mode fail, obm transfer thread fail end\n", __FUNCTION__);
				}
				else
				{
					RETAILMSG("%s: set hsi dl normal mode\n", __FUNCTION__);
				}
			}
			else
			{
				ERRORMSG("%s: obm transfer thread fail end\n", __FUNCTION__);
			}
		}
		else
		{
			ERRORMSG("%s: obm transfer thread fail end\n", __FUNCTION__);
		}
	}
	else
	{
		ERRORMSG("%s: invalid obm transfer type, obm transfer thread fail end\n", __FUNCTION__);
	}
out:
	do_gettimeofday(&time_end);
	RETAILMSG("%s: obm transfer time spent (%d)m, (%d)us\n", __FUNCTION__,
			(int)(time_end.tv_sec - time_start.tv_sec),
			(int)(time_end.tv_usec - time_start.tv_usec));
	return ret;
}
