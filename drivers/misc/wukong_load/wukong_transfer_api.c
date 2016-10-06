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
 *  Filename: uart_api.c
 *
 *  Description: Offer UART operations API in kernel.
 *
 *  History:
 *   Nov, 11 2011 -  Li Wang(wangli@marvell.com) Creation of file
 *
 *  Notes:
 *
 ******************************************************************************/
#include <asm/termios.h>
#include <linux/string.h>
#include <linux/unistd.h>
#include <asm/uaccess.h>

#include "wukong_config.h"
#include "wukong_transfer_api.h"


int speed_arr[] = {B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200, B300,
	    B38400, B19200, B9600, B4800, B2400, B1200, B300};
int name_arr[] = {115200, 57600, 38400, 19200, 9600, 4800, 2400, 1200, 300,
	    38400, 19200, 9600, 4800, 2400, 1200, 300};

static int my_tcgetattr(struct file* fd, struct termios* opt)
{
	int ret = -1;
	mm_segment_t oldfs;
	oldfs = get_fs();
	set_fs(KERNEL_DS);

	if(fd->f_op->unlocked_ioctl)
	{
		ret = fd->f_op->unlocked_ioctl(fd, TCGETS, (unsigned long)opt);
	}

	set_fs(oldfs);
	return ret;
}

static int my_tcsetattr(struct file* fd, struct termios* opt)
{
	int ret = -1;
	mm_segment_t oldfs;
	oldfs = get_fs();
	set_fs(KERNEL_DS);

	if(fd->f_op->unlocked_ioctl)
	{
		//todo: no chance to set if need to effect immediately
		ret = fd->f_op->unlocked_ioctl(fd, TCSETS, (unsigned long)opt);
	}

	set_fs(oldfs);
	return ret;
}

static int my_tcflush(struct file* fd, int queue_selector)
{
	int ret = -1;
	mm_segment_t oldfs;
	oldfs = get_fs();
	set_fs(KERNEL_DS);

	if(fd->f_op->unlocked_ioctl)
	{
		ret = fd->f_op->unlocked_ioctl(fd, TCFLSH, queue_selector);
	}

	set_fs(oldfs);
	return ret;
}

int uart_set_speed(struct file* fd, int speed)
{
	int   i;
	int   status;
	struct termios   Opt;
	if(my_tcgetattr(fd, &Opt) !=  0)
	{
		ERRORMSG("%s: tcgetattr of uart_set_speed failed\n", __FUNCTION__);
		return -1;
	}
	for(i= 0; i < sizeof(speed_arr)/sizeof(int); i++)
	{
		if(speed == name_arr[i])
		{
			my_tcflush(fd, TCIOFLUSH);

			Opt.c_cflag &= ~CBAUD;
			Opt.c_cflag |= speed_arr[i];
			status = my_tcsetattr(fd, &Opt);
			if(status != 0)
			{
				ERRORMSG("%s: tcsetattr of set_speed failed\n", __FUNCTION__);
				return -2;
			}
		}
		my_tcflush(fd, TCIOFLUSH);
	}
	return 0;
}

int uart_set_parity(struct file* fd, int databits, int stopbits, int parity)
{
	struct termios options;
	if(my_tcgetattr(fd,&options) !=  0)
	{
		ERRORMSG("%s: tcgetattr of set_parity failed\n", __FUNCTION__);
		return(-1);
	}

	options.c_iflag = 0;
	options.c_oflag = 0;
	options.c_cflag &= ~CSIZE;
	switch(databits)
	{
		case 7:
			options.c_cflag |= CS7;
			break;
		case 8:
			options.c_cflag |= CS8;
			break;
		default:
			ERRORMSG("%s: set_parity Unsupported data size\n", __FUNCTION__);
			return (-2);
	}
	switch(parity)
	{
		case 'n':
		case 'N':
			options.c_cflag &= ~PARENB;   /* Clear parity enable */
			options.c_iflag &= ~INPCK;     /* Disable parity checking */
			break;
		case 'o':
		case 'O':
			options.c_cflag |= (PARODD | PARENB);  /* Set to be odd parity check*/
			options.c_iflag |= INPCK;             /* Enable parity checking */
			break;
		case 'e':
		case 'E':
			options.c_cflag |= PARENB;     /* Enable parity */
			options.c_cflag &= ~PARODD;   /* set to be even parity check*/
			options.c_iflag |= INPCK;       /* Enable parity checking */
			break;
		case 's':
		case 'S':
			options.c_cflag &= ~PARENB;   /* Clear parity enable */
			options.c_iflag &= ~INPCK;     /* Disable parity checking */
		default:
			ERRORMSG("%s set_parity Unsupported parity\n", __FUNCTION__);
			return (-3);
	}
	/* Set stop bits*/
	switch(stopbits)
	{
		case 1:
			options.c_cflag &= ~CSTOPB;
			break;
		case 2:
			options.c_cflag |= CSTOPB;
			break;
		default:
			ERRORMSG("%s: set_parity Unsupported stop bits\n", __FUNCTION__);
			return (-4);
	}

	options.c_cflag |= (CLOCAL | CREAD);

	options.c_cc[VTIME] = 0; //block wait
	options.c_cc[VMIN] = 1;

	my_tcflush(fd, TCIFLUSH); /* Update the options and do it NOW */
	if(my_tcsetattr(fd, &options) != 0)
	{
		ERRORMSG("%s: tcsetattr of set_parity failed\n", __FUNCTION__);
		return (-5);
	}
	return (0);
}

struct file* open_dev(char *Dev)
{
	struct file* fd = filp_open(Dev, O_RDWR | O_NOCTTY, 0664);//  | O_NDELAY),
	if(IS_ERR(fd))
	{
		ERRORMSG("%s: open dev failed\n", __FUNCTION__);
		return NULL;
	}
	else
	{
		return fd;
	}

}

int read_dev(struct file* fd, char* buf, int len)
{
	int ret = -1;
	int nleft, nread;
	char * ptr = buf;
	mm_segment_t oldfs;
	oldfs = get_fs();
	set_fs(KERNEL_DS);

	DEBUGMSG("%s: len=%d\n", __FUNCTION__, len);
	if(fd != NULL && len > 0 && ptr != NULL)
	{
		nleft = len;
		while(nleft > 0)
		{
			if((nread = fd->f_op->read(fd, ptr, nleft, &fd->f_pos)) < 0)
			{
				ERRORMSG("%s: read error\n", __FUNCTION__);
				if(nleft == len)
				{
					ret = -1;
					goto out;
				}
				else
					break;
			}
			else if(0 == nread)
			{
				DEBUGMSG("%s: read 0", __FUNCTION__);
				break;
			}

			nleft -= nread;
			ptr += nread;
			DEBUGMSG("%s: nread=%d\n", __FUNCTION__, nread);
			DEBUGMSG("%s: nleft=%d\n", __FUNCTION__, nleft);
		}
		ret = (len - nleft);
	}
	DEBUGMSG("%s: return: %d\n", __FUNCTION__, ret);
out:
	set_fs(oldfs);
	return ret;
}

int write_dev(struct file* fd, char* buf, int len)
{
	int ret = -1;
	int nleft, nwrite;
	char* ptr = buf;
	mm_segment_t oldfs;
	oldfs = get_fs();
	set_fs(KERNEL_DS);

	DEBUGMSG("%s: len=%d\n", __FUNCTION__, len);
	if(fd != NULL && len > 0 && ptr != NULL)
	{
		nleft = len;
		while(nleft > 0)
		{
			if((nwrite = fd->f_op->write(fd, ptr, nleft, &fd->f_pos)) < 0)
			{
				ERRORMSG("%s: write error\n", __FUNCTION__);
				if(nleft == len)
				{
					ret = -1;
					goto out;
				}
				else
					break;
			}
			else if(0 == nwrite)
			{
				DEBUGMSG("%s: write 0", __FUNCTION__);
				break;
			}

			nleft -= nwrite;
			ptr += nwrite;

			DEBUGMSG("%s: nwrite=%d\n", __FUNCTION__, nwrite);
			DEBUGMSG("%s: nleft=%d\n", __FUNCTION__, nleft);
		}
		ret = (len - nleft);
	}
	DEBUGMSG("%s: return: %d\n", __FUNCTION__, ret);
out:
	set_fs(oldfs);
	return ret;
}

int close_dev(struct file* fd)
{
	int ret = -1;
	if(fd != NULL)
	{
		ret = filp_close(fd, NULL);
		if(ret < 0)
		{
			ERRORMSG("%s: close error\n", __FUNCTION__);
		}

	}
	return ret;
}

int uart_set_hardware_flow_ctl(struct file* fd, int fcFlag)
{
        struct termios options;
        if(my_tcgetattr(fd,&options)  !=  0)
        {
		ERRORMSG("%s: tcgetattr of setHardwareFlowCtl failed\n", __FUNCTION__);
                return(-1);
        }
        if(fcFlag == 1)
	{
                options.c_cflag |=  CRTSCTS; /*Enable Hardware Flow Control*/
        }
	else if(fcFlag == 0)
	{
                options.c_cflag &= ~CRTSCTS; /*Disable*/
	}
        else
	{
		ERRORMSG("%s: invalid fc flag\n", __FUNCTION__);
                return (-2);
	}

        my_tcflush(fd, TCIFLUSH); /* Update the options and do it NOW */
        if (my_tcsetattr(fd, &options) != 0)
        {
		ERRORMSG("%s: tcsetattr of setHardwareFlowCtl failed\n", __FUNCTION__);
                return (-3);
        }
        return (0);

}

int uart_set_software_flow_ctl(struct file* fd, int fcFlag)
{
        struct termios options;
        if(my_tcgetattr(fd, &options) != 0)
        {
		ERRORMSG("%s: tcgetattr of setsoftwareFlowCtl failed\n", __FUNCTION__);
                return(-1);
        }
        if(fcFlag == 1) /*Enable*/
	{
                options.c_iflag |=  (IXON|IXOFF|IXANY); /*Enable Hardware Flow Control*/
	}
        else if(fcFlag == 0) /*Disable*/
	{
                options.c_iflag &= ~(IXON|IXOFF|IXANY); /*Disable*/
	}
        else
	{
		ERRORMSG("%s: invalid fc flag\n", __FUNCTION__);
                return (-2);
	}
        my_tcflush(fd, TCIFLUSH); /* Update the options and do it NOW */
        if(my_tcsetattr(fd, &options) != 0)
        {
		ERRORMSG("%s: tcsetattr of setsoftwareFlowCtl failed\n", __FUNCTION__);
                return (-3);
        }
        return (0);
}



int uart_set_raw_mode(struct file* fd, int rawFlag)
{
	struct termios options;
	if(my_tcgetattr(fd, &options) != 0)
	{
		ERRORMSG("%s: tcgetattr of setRawMode failed\n", __FUNCTION__);
		return(-1);
	}

	if(rawFlag == 1)
	{
		options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG) ; /*Line control setting*/
		options.c_oflag &= ~OPOST ; /*Output setting: not set = raw output*/

		options.c_oflag &= ~(ONLCR | OCRNL);
		options.c_iflag &= ~(ICRNL | INLCR | IGNCR);
	}
	else if(rawFlag == 0)
	{
		//no use now
		return (0);
	}
	else
	{
		ERRORMSG("%s: invalid raw flag\n", __FUNCTION__);
		return (-2);
	}

	my_tcflush(fd,TCIFLUSH); /* Update the options and do it NOW */

	//for debug
	DEBUGMSG("options.c_iflag[%d]\n", options.c_iflag);
	DEBUGMSG("options.c_oflag[%d]\n", options.c_oflag);
	DEBUGMSG("options.c_cflag[%d]\n", options.c_cflag);
	DEBUGMSG("options.c_lflag[%d]\n", options.c_lflag);
	DEBUGMSG("options.c_line[%d]\n", options.c_line);

	if(my_tcsetattr(fd, &options) != 0)
        {
		ERRORMSG("%s: tcsetattr of setRawMode failed\n", __FUNCTION__);
                return (-3);
        }
        return (0);
}

/*Data transmit MUST be PURE raw mode, otherwise, some control symbols will be added!*/
int uart_set_pure_raw_mode(struct file* fd)
{
	struct termios new;
	memset(&new, 0, sizeof(struct termios));
	if(my_tcsetattr(fd, &new) != 0)
        {
		ERRORMSG("%s: tcsetattr of setPureRawMode failed\n", __FUNCTION__);
                return (-1);
        }

	return 0;
}

void uart_flush(struct file* fd)
{
	my_tcflush(fd, TCSAFLUSH);
}
