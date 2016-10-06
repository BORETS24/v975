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
 *  Filename: wukong_load.h
 *
 *  Description: export wukong load externel structure definition and API.
 *
 *  History:
 *   Nov, 11 2011 -  Li Wang(wangli@marvell.com) Creation of file
 *
 *  Notes:
 *
 ******************************************************************************/
#ifndef __WUKONG_LOAD_H_
#define __WUKONG_LOAD_H_

#include<linux/workqueue.h>
#include<linux/device.h>

#include "wukong_config.h"

#define WUKONG_IOC_MAGIC 'W'
#define WUKONG_IOCTL_HOLD  _IOW(WUKONG_IOC_MAGIC, 1, int)
#define WUKONG_IOCTL_RELEASE _IOW(WUKONG_IOC_MAGIC, 2, int)
#define WUKONG_IOCTL_GET_OBM_INFO _IOW(WUKONG_IOC_MAGIC, 3, int)
#define WUKONG_IOCTL_SHOW_MODEM_STATE _IOR(WUKONG_IOC_MAGIC, 4, int)
#define WUKONG_IOCTL_DOWNLOAD_TEST _IOW(WUKONG_IOC_MAGIC, 5, int)
#define WUKONG_IOCTL_TRIGGER_EEH_DUMP _IOW(WUKONG_IOC_MAGIC, 6, int)
#define WUKONG_IOCTL_ENABLE_MODEM_IRQ _IOW(WUKONG_IOC_MAGIC, 7, int)

struct modem_device
{
	int power_enable1;// gpio to power on modem
	int power_enable2;// gpio to power on modem
	int reset_gpio; // gpio to reset modem
	int usim2_sw_enable_gpio; //gpio to switch USIM2
	int uart_switch_open; //gpio to switch UART between TTC use and wukong use
	int active_level; // vaild levet to hold modem

	int boot_up_irq; //  modem notice AP reday
	int watch_dog_irq; //modem watch dog time out
	int assert_eeh_irq; // modem assert

	PLATFORM_TYPE platform_type;//platform type

	struct workqueue_struct * modem_wq;

	struct device * dev;
};

struct uevent_work
{
	struct work_struct work;
	struct modem_device * modem;
	char * env[2];
};

struct delayed_uevent_work
{
	struct delayed_work work;
	struct modem_device * modem;
	char * env[2];
};

#define OBM_BUF_LEN 50

typedef struct _obm_info
{
	int cp_flash_state;
	int cp_image_store_mode;
	int obm_transfer_dev_type;
	char obm_transfer_dev[OBM_BUF_LEN];
	char obm_bin_path[OBM_BUF_LEN];
	int obm_bin_size;
	int obm_tim_size;
	int obm_dkb_offset;
}OBM_INFO;

typedef enum
{
	CP_NO_FLASH,
	CP_OBM_NOR_FLASH,
	CP_WHOLE_FLASH,
}CP_FLASH_STATE;

typedef enum
{
	CP_IMAGE_IN_AP_FILESYSTEM = 0,
	CP_IMAGE_IN_AP_PARTITION,
}CP_IMAGE_STORE_MODE;

typedef enum
{
	DKBI_UART_TYPE = 0,
	DKBI_HSI_TYPE,
}DKBI_TRANSFER_TYPE;

DKBI_TRANSFER_TYPE get_dkbi_transfer_device_type(void);
char* get_obm_transfer_device(void);

CP_FLASH_STATE get_cp_flash_state(void);
CP_IMAGE_STORE_MODE get_cp_image_store_mode(void);
char* get_obm_transfer_device(void);
char* get_obm_bin_path(void);
int get_obm_partition_mtd_no(void);
int get_obm_bin_size(void);
int get_obm_tim_size(void);
int get_obm_dkb_offset(void);

void set_line(int gpio, int level);
int get_line(int gpio);

#endif
