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
 *  Filename: wukong_load.c
 *
 *  Description: Init wukong dev, offer ioctl feature for user space.
 *
 *  History:
 *   Nov, 11 2011 -  Li Wang(wangli@marvell.com) Creation of file
 *
 *  Notes:
 *
 ******************************************************************************/
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <asm/gpio.h>
#include <asm/uaccess.h>
#include <linux/delay.h>

#include <linux/interrupt.h>

#include "wukong_load.h"
#include "wukong_boot.h"
#include "wukong_assert.h"
#include "wukong_config.h"

static char* platform = "custom";
module_param(platform, charp, 0);
MODULE_PARM_DESC(platform, "Platform");

static struct modem_device wukong_modem_custom ={
		.power_enable1 = GPIO_VSYS_ON_CUSTOM,
		.power_enable2 = GPIO_POWERON_CUSTOM,
		.reset_gpio = GPIO_RESET_CUSTOM,
		.usim2_sw_enable_gpio = GPIO_NO_USE,
		.uart_switch_open = GPIO_NO_USE,
		.active_level = 1,
		.boot_up_irq = 0,
		.watch_dog_irq = 0,
		.assert_eeh_irq = 0,
		.platform_type = PLATFORM_CUSTOM,
		0,
		0
};

static struct modem_device* wukong_modem = 0;

extern struct delayed_uevent_work wdt_uevent_wk;
extern void set_modem_error(struct modem_device * modem, enum modemErrorType error, char * env[]);

static OBM_INFO obm_info;

static int modem_open(struct inode * inode, struct file *filp);
static ssize_t modem_write(struct file* filp, const char __user *buf, size_t count, loff_t* f_pos);
static ssize_t modem_read(struct file* filp, char __user *buf, size_t count, loff_t* f_pos);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
static long modem_ioctl(struct file *filp, unsigned int cmd, unsigned long);
#else
static int modem_ioctl(struct inode* inode, struct file *filp, unsigned int cmd, unsigned long);
#endif
static int modem_release(struct inode *inode, struct file * filp);
static void init_obm_info(void);


static int b_can_be_read = 0;


static struct file_operations modem_fops ={
		.open = modem_open,
		.write = modem_write,
		.read = modem_read,
		.release = modem_release,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
		.unlocked_ioctl = modem_ioctl,
#else
		.ioctl = modem_ioctl,
#endif
		.owner = THIS_MODULE
};

static struct miscdevice modem_miscdev = {
		.minor = MISC_DYNAMIC_MINOR,
		.name = "wukong",
		.fops = &modem_fops
};

static char* platform_type_name[] = {
	"custom platform",
	"invalid platform",
};

void set_line(int gpio, int level)
{
	int pin;
	if(gpio < 0)
	{
		return;
	}
	pin = gpio;//mfp_to_gpio(gpio);
/*
	if(gpio_request(pin, "GPIO"))
	{
		ERRORMSG("%s: GPIO[%d] request failed\n", __FUNCTION__, pin);
		//return;
	}
*/
	gpio_direction_output(pin, level);
	RETAILMSG("%s: GPIO out pin = %d, level = %d\n",__FUNCTION__,  pin, level);

	//gpio_free(pin);
}

int get_line(int gpio)
{
	int pin, val;
	pin = gpio;//mfp_to_gpio(gpio);
	gpio_direction_input(pin);
	val = !!gpio_get_value(pin);

	RETAILMSG("%s: GPIO in pin = %d, val = %d\n",__FUNCTION__,  pin, val);
	return val;
}

static int init_modem(struct modem_device * modem)
{
	modem->modem_wq = create_workqueue("modem boot work queue");
	if(modem->modem_wq == NULL)
	{
		ERRORMSG("%s:Can't create work queue!\n",__FUNCTION__);
		return -ENOMEM;
	}
	return 0;
}
static void deinit_modem(struct modem_device * modem)
{
	if(modem->modem_wq != NULL)
		destroy_workqueue(modem->modem_wq);
}

static void init_obm_info(void)
{
	memset(&obm_info, 0, sizeof(obm_info));

	obm_info.cp_flash_state = CP_FLASH_DEFAULT_FLASH;
	obm_info.cp_image_store_mode = CP_IMAGE_STORE_DEFAULT_MODE;
	strcpy(obm_info.obm_transfer_dev, OBM_TRANSFER_DEV);
	strcpy(obm_info.obm_bin_path, OBM_BIN_FILE);
	obm_info.obm_bin_size = OBM_BIN_SIZE;
	obm_info.obm_tim_size = OBM_TIM_SIZE;
	obm_info.obm_dkb_offset = OBM_DKB_OFFSET;
}

CP_IMAGE_STORE_MODE get_cp_image_store_mode()
{
	return (CP_IMAGE_STORE_MODE)(obm_info.cp_image_store_mode);
}

CP_FLASH_STATE get_cp_flash_state()
{
	return (CP_FLASH_STATE)(obm_info.cp_flash_state);
}

DKBI_TRANSFER_TYPE get_dkbi_transfer_device_type()
{
	return (DKBI_TRANSFER_TYPE)(obm_info.obm_transfer_dev_type);
}

char* get_obm_transfer_device()
{
	return obm_info.obm_transfer_dev;
}

char* get_obm_bin_path()
{
	return obm_info.obm_bin_path;
}

int get_obm_bin_size()
{
	return obm_info.obm_bin_size;
}

int get_obm_tim_size()
{
	return obm_info.obm_tim_size;
}

int get_obm_dkb_offset()
{
	return obm_info.obm_dkb_offset;
}

static int __init wukong_load_init(void)
{
	int ret;

	if(!platform)
	{
		ERRORMSG("%s: Platform not defined\n",__FUNCTION__);
		return -EINVAL;
	}

	if(!strcmp(platform, PLATFORM_CUSTOM_PARAM))
	{
		wukong_modem = &wukong_modem_custom;
/*
		wukong_modem->boot_up_irq = gpio_to_irq(GPIO_NO_USE);
		wukong_modem->watch_dog_irq = gpio_to_irq(GPIO_NO_USE);
		wukong_modem->assert_eeh_irq = gpio_to_irq(GPIO_NO_USE);
*/
	}
	else
	{
		ERRORMSG("%s: Platform not matched\n",__FUNCTION__);
		return -EINVAL;
	}

	ret = misc_register(&modem_miscdev);
	if(ret < 0)
	{
		ERRORMSG("%s:Can't register misc device!\n",__FUNCTION__);
		return ret;
	}

	wukong_modem->dev = modem_miscdev.this_device;
	ret = init_modem(wukong_modem);
	if(ret < 0)
	{
		ERRORMSG("%s:Can't initialize modem!\n",__FUNCTION__);
		return ret;
	}

	init_obm_info();

	RETAILMSG("platform type = %s\n", platform_type_name[wukong_modem->platform_type]);

	init_modem_boot(wukong_modem);
	init_modem_assert(wukong_modem);

	return 0;
}

static void __exit wukong_load_exit(void)
{
	int ret = -1;
	deinit_modem(wukong_modem);

	deinit_modem_boot(wukong_modem);
	deinit_modem_assert(wukong_modem);

	ret = misc_deregister(&modem_miscdev);
	if(ret < 0)
	{
		ERRORMSG("%s:Can't deregister misc device!\n",__FUNCTION__);
	}
}

static int modem_open(struct inode* inode, struct file* filp)
{
    DEBUGMSG("%s:modem address 0x%x", __FUNCTION__, wukong_modem);
	filp->private_data = (void *)wukong_modem;

	b_can_be_read = 1;

	return 0;
}

static int modem_release(struct inode* inode, struct file* filp)
{
	return 0;
}

static void report_relink_event(void)
{
	char * env[2];
	env[0]="STATE=RELINK";
	env[1]=NULL;

	RETAILMSG("%s: relink uevent is sent\n", __FUNCTION__);
	kobject_uevent_env(&wukong_modem->dev->kobj,KOBJ_CHANGE,env);
}

static void report_update_event(void)
{
	char * env[2];
	env[0]="STATE=UPDATE";
	env[1]=NULL;

	RETAILMSG("%s: update uevent is sent\n", __FUNCTION__);
	kobject_uevent_env(&wukong_modem->dev->kobj,KOBJ_CHANGE,env);
}

static void report_erase_event(void)
{
	char * env[2];
	env[0]="STATE=ERASE";
	env[1]=NULL;

	RETAILMSG("%s: erase uevent is sent\n", __FUNCTION__);
	kobject_uevent_env(&wukong_modem->dev->kobj,KOBJ_CHANGE,env);
}

void modem_bootup_test(void);
static ssize_t modem_write(struct file* filp, const char __user *buf, size_t count, loff_t* f_pos)
{
	char c[2];

	DEBUGMSG("%s: vaule count(%d) from user space\n", __FUNCTION__, count);

	if(count != 2)
	{
		ERRORMSG("%s: get invalid vaule from user space\n", __FUNCTION__);
		return -EINVAL;
	}
	else
	{
		if(copy_from_user(&c, buf, count))
		{
			ERRORMSG("%s: copy vaule from user space failed\n", __FUNCTION__);
			return -EFAULT;
		}
	}

	switch(c[0])
	{
		case '1':
		{
			RETAILMSG("%s: get '1' from user space, cp reset process triggered\n", __FUNCTION__);
			report_relink_event();
		}
		break;
		case '2':
		{
			RETAILMSG("%s: get '2' from user space, cp images update process triggered\n", __FUNCTION__);
			report_update_event();
		}
		break;
		case '3':
		{
			RETAILMSG("%s: get '3' from user space, cp images erase process triggered\n", __FUNCTION__);
			report_erase_event();
		}
		break;
		case '4':
		{
			RETAILMSG("%s: get '4' from user space, trigger cp boot up\n", __FUNCTION__);
			modem_bootup_test();
		}
		break;
		case '5':
		{
			RETAILMSG("%s: get '5' from user space\n", __FUNCTION__);
		}
		break;
		default:
		{
			ERRORMSG("%s: get invalid vaule from user space\n", __FUNCTION__);
		}
		break;
	}

	return count;
}

static ssize_t modem_read(struct file* filp, char __user *buf, size_t count, loff_t* f_pos)
{
	char* state = get_modem_state_name();

	int count_t = 0;

	if(!b_can_be_read)
	{
		return 0;
	}

	b_can_be_read = 0;

	if(state == NULL)
		return -EFAULT;

	count_t = (int)strlen(state);

	if(count > count_t)
		count = count_t;

	if(copy_to_user(buf, state, count))
	{
		ERRORMSG("%s: copy to user failed\n", __FUNCTION__);
		return -EFAULT;
	}

	return count;
}

static int get_obm_info(unsigned long arg)
{
	memset(&obm_info, 0, sizeof(OBM_INFO));
	if(copy_from_user(&obm_info, (OBM_INFO*)arg, sizeof(OBM_INFO)))
	{
		ERRORMSG("%s: copy param failed\n", __FUNCTION__);
		init_obm_info();
		return -EFAULT;
	}
	return 0;
}

static int show_modem_state(unsigned long arg)
{
	int state = get_modem_state();
	if(copy_to_user((int*)arg, &state, sizeof(int)))
	{
		ERRORMSG("%s: copy to user failed\n", __FUNCTION__);
		return -EFAULT;

	}
	return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
static long modem_ioctl(struct file* filp, unsigned int cmd, unsigned long arg)
#else
static int modem_ioctl(struct inode* inode, struct file* filp, unsigned int cmd, unsigned long arg)
#endif
{
	struct modem_device * modem;
	modem = (struct modem_device *)filp->private_data;
    DEBUGMSG("%s:modem address 0x%x", __FUNCTION__, modem);
	if (_IOC_TYPE(cmd) != WUKONG_IOC_MAGIC)
	{
		ERRORMSG("%s: seh magic number is wrong!\n",__FUNCTION__);
		return -EINVAL;
	}
	switch(cmd)
	{
	case WUKONG_IOCTL_HOLD:
		hold_modem(0);
		break;
	case WUKONG_IOCTL_RELEASE:
		release_modem(0);
		break;
	case WUKONG_IOCTL_GET_OBM_INFO:
		get_obm_info(arg);
		DEBUGMSG("%s: obm_info[cp_flash_state=%d,cp_image_store_mode=%d,obm_transfer_type=%d,obm_transfer_device=%s, path=%s, obm_bin_size=%d, obm_tim_size=%d, obm_dkb_offset=%d]\n",
				__FUNCTION__, get_cp_flash_state(),
				get_cp_image_store_mode(), get_dkbi_transfer_device_type(),
				get_obm_transfer_device(),get_obm_bin_path(),
				get_obm_bin_size(),get_obm_tim_size(),
				get_obm_dkb_offset());
		break;
	case WUKONG_IOCTL_SHOW_MODEM_STATE:
		show_modem_state(arg);
		break;
	case WUKONG_IOCTL_DOWNLOAD_TEST:
		set_modem_error(modem,WATCHDOGTIMEOUT,wdt_uevent_wk.env);
		queue_delayed_work(modem->modem_wq,&wdt_uevent_wk.work, HZ/10);
		break;
	case WUKONG_IOCTL_TRIGGER_EEH_DUMP:
		report_eeh_dump_event();
		break;
	case WUKONG_IOCTL_ENABLE_MODEM_IRQ:
		{

			enable_irq(modem->boot_up_irq);

			enable_irq(modem->watch_dog_irq);
			enable_irq(modem->assert_eeh_irq);
			enable_irq_wake(modem->watch_dog_irq);
			enable_irq_wake(modem->assert_eeh_irq);

			RETAILMSG("%s: enable irq from modem from userspace\n", __FUNCTION__);
		}
		break;
	default:
		ERRORMSG("%s: illgal command !\n",__FUNCTION__);
		return -EINVAL;
	}
	return 0;
}
module_init(wukong_load_init);
module_exit(wukong_load_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Marvell");
MODULE_DESCRIPTION("Marvell WuKong Loader.");
