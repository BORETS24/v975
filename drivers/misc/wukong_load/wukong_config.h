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
 *  Filename: wukong_config.h
 *
 *  Description: Export configuration default value/debug macro.
 *
 *  History:
 *   Nov, 11 2011 -  Li Wang(wangli@marvell.com) Creation of file
 *
 *  Notes:
 *
 ******************************************************************************/
#ifndef __WUKONG_CONFIG_H_
#define __WUKONG_CONFIG_H_

/*Platform*/
#define PLATFORM_CUSTOM_PARAM "custom"

typedef enum
{
	PLATFORM_CUSTOM,
	PLATFORM_INVALID
}PLATFORM_TYPE;

/*config gpio*/
#define GPIO_NO_USE -1

/*GPIO for custom*/
#define GPIO_VSYS_ON_CUSTOM 	(82 + 0x60)
#define GPIO_POWERON_CUSTOM 	(17 + 0x60)
#define GPIO_USB_SEL (90)
#define GPIO_RESET_CUSTOM 	(91)
#define GPIO_BOOTUP_CUSTOM 	(20 + 0x60)
#define GPIO_WDTRST_CUSTOM 	(3)
#define GPIO_EEH_CUSTOM 	(40)

/*config cp flash feature*/
#define CP_FLASH_DEFAULT_FLASH 1

/*config cp image store mode*/
#define CP_IMAGE_STORE_DEFAULT_MODE 0

/*config OBM_transfer device*/
#define OBM_TRANSFER_DEV "/dev/ttyIFX0"

/*config obm img address*/
#define OBM_BIN_FILE "/data/DKBI.bin"
#define OBM_TIM_SIZE 256
#define OBM_DKB_OFFSET 0x400

//#define TEST_GPIO//for test use
#define OBM_BIN_SIZE 10240

/*config debug parameter*/
#define DEBUG_WUKONG_LOAD_DRIVER

/*config WDT DKBI download mode*/
//#define WDT_DOWNLOAD_DKBI_IN_DRIVER

#ifdef DEBUG_WUKONG_LOAD_DRIVER
#define DEBUGMSG(fmt, args ...)     printk("wukong: " fmt, ## args)
#define ERRORMSG(fmt, args ...) printk(KERN_ERR "wukong: " fmt, ## args)
#define ENTER()                 printk("wukong: ENTER %s\n", __FUNCTION__)
#define LEAVE()                 printk("wukong: LEAVE %s\n", __FUNCTION__)
#define FUNC_EXIT()                     printk("wukong: EXIT %s\n", __FUNCTION__)
#define RETAILMSG(fmt, args ...) printk(KERN_INFO "wukong: " fmt, ## args)
#else
#define DEBUGMSG(fmt, args ...)     printk(KERN_DEBUG "wukong: " fmt, ## args)
#define ERRORMSG(fmt, args ...) printk(KERN_ERR "wukong:" fmt, ## args)
#define ENTER()         do {} while (0)
#define LEAVE()         do {} while (0)
#define FUNC_EXIT()     do {} while (0)
#define RETAILMSG(fmt, args ...) printk("wukong: " fmt, ## args)
#endif

#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 0)
#ifndef irq_to_gpio
#define irq_to_gpio(irq) (irq)  //cj todo
#endif
#endif

#endif
