/*
 * leds-intel-red.c - Intel Red LED driver
 *
 * Copyright (C) 2011 Intel Corporation
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/leds.h>
#include <linux/earlysuspend.h>
#include <linux/platform_device.h>
#include <linux/mfd/intel_msic.h>
#include <linux/reboot.h>

#include <asm/intel_scu_pmic.h>
#include <asm/intel_mid_pwm.h>
#include <asm/intel_mid_remoteproc.h>


#define RED_LED_STOP    0
#define RED_LED_RUN     -1
#define RED_LED_PWM_1S  1
#define RED_LED_PWM_2S  2
#define RED_LED_PWM_4S  4

static int scu_ipc_iowrite8(u16 addr, u8 data)
{
    int ret;
    
	ret = intel_scu_ipc_iowrite8(addr, data);
    if (ret)
        return ret;   

    return ret;
}

static int intel_msic_set_red_led(int value)
{
    int ret;
    u8 data;

    if(RED_LED_STOP == value)
    {
        data = 0xff;
        ret = scu_ipc_iowrite8(INTEL_MSIC_CHRLEDPWM,data);
	    if (ret)
		    return ret;
        
        data = 0x34;
        ret = scu_ipc_iowrite8(INTEL_MSIC_CHRLEDCTRL,data);
        if (ret)
		    return ret; 
    }
    else if(RED_LED_RUN == value)
    {
        data = 0xff;
        ret = scu_ipc_iowrite8(INTEL_MSIC_CHRLEDPWM,data);
	    if (ret)
		    return ret;
        
        data = 0x35;
        ret = scu_ipc_iowrite8(INTEL_MSIC_CHRLEDCTRL,data);
        if (ret)
		    return ret;        
    }
    else if(RED_LED_PWM_1S == value)
    {
        data = 0x4c;
        ret = scu_ipc_iowrite8(INTEL_MSIC_CHRLEDPWM,data);
        if (ret)
		    return ret;

        data = 0x35;
        ret = scu_ipc_iowrite8(INTEL_MSIC_CHRLEDCTRL,data);
        if (ret)
		    return ret;        
    }
    else if(RED_LED_PWM_2S == value)
    {
        data = 0x25;
        ret = scu_ipc_iowrite8(INTEL_MSIC_CHRLEDPWM,data);
        if (ret)
		    return ret;

        data = 0x2d;
        ret = scu_ipc_iowrite8(INTEL_MSIC_CHRLEDCTRL,data);
        if (ret)
		    return ret;        
    }
    else if(RED_LED_PWM_4S == value)
    {
        data = 0x12;
        ret = scu_ipc_iowrite8(INTEL_MSIC_CHRLEDPWM,data);
        if (ret)
		    return ret;

        data = 0x25;
        ret = scu_ipc_iowrite8(INTEL_MSIC_CHRLEDCTRL,data);
        if (ret)
		    return ret;        
    }
    else
    {
        pr_err("set red led mode error!!!\n");
        return -1;
    }
    //pr_info("set red led value = %d ok!!!\n",value);
    
	return ret;
}

int intel_msic_set_gpio0hv0(int value)
{
    int ret;
    u8 data;

    if(1 == value)
    {
        data = 0x39;
        ret = scu_ipc_iowrite8(INTEL_MSIC_GPIO0HV0CTLO,data);
	    if (ret)
		    return ret;
    }
    else if(0 == value)
    {
        data = 0x38;
        ret = scu_ipc_iowrite8(INTEL_MSIC_GPIO0HV0CTLO,data);
	    if (ret)
		    return ret;       
    }
    else
        pr_err("set gpio0hv0 value invalid!!!\n");

    pr_info("set gpio0hv0 value = %d ok!!!\n",value);
    
	return ret;
}
EXPORT_SYMBOL_GPL(intel_msic_set_gpio0hv0);

static int led_red_notify_sys(struct notifier_block *this, unsigned long code,void *unused);
static int led_red_timeout;

static struct notifier_block led_red_notifier = {
	.notifier_call = led_red_notify_sys,
};

static int led_red_notify_sys(struct notifier_block *this, unsigned long code,
			  void *unused)
{
	if (code == SYS_DOWN || code == SYS_POWER_OFF)
	{
		intel_msic_set_red_led(0);
	}	
	return NOTIFY_DONE;
}

static void led_red_early_suspend(struct early_suspend *h)
{
    if(led_red_timeout == 1)
	    intel_msic_set_red_led(RED_LED_PWM_4S);
}

static void led_red_late_resume(struct early_suspend *h)
{
    if(led_red_timeout == 1)
	    intel_msic_set_red_led(RED_LED_PWM_1S);
}

static struct early_suspend led_red_suspend_desc = {
	.level   = EARLY_SUSPEND_LEVEL_DISABLE_FB,
	.suspend = led_red_early_suspend,
	.resume  = led_red_late_resume,
};

static void led_red_set(struct led_classdev *led_cdev,
	enum led_brightness value)
{
    //pr_info("ZTE set red led value = %d\n",value);    
    led_red_timeout = value;

    if (100 == value) 
    {
        intel_msic_set_red_led(-1);
    } 
    else if(0 == value)
    {
        intel_msic_set_red_led(0);
    }
    else if(1 == value)
    {
        intel_msic_set_red_led(RED_LED_PWM_1S);
    }
    else if(2 == value)
    {
        intel_msic_set_red_led(RED_LED_PWM_2S);
    }
    else if(4 == value)
    {
        intel_msic_set_red_led(RED_LED_PWM_4S);
    }
    else if(10 == value)
    {
        intel_msic_set_gpio0hv0(1);
    }    
    else	
    {
        pr_err("set led red on_time error!!!\n");
    }    
}


static struct led_classdev led_red = {
	.name			    = "led_red",
	.brightness_set		= led_red_set,
	.brightness		    = LED_OFF,
	.max_brightness		= 100,
};

static int __init led_red_init(void)
{
	int ret;
    
	ret = led_classdev_register(NULL, &led_red);
	if (ret) {
		pr_err("register red led failed\n");
		return ret;
	}
	led_red_set(&led_red, led_red.brightness);

    ret = register_reboot_notifier(&led_red_notifier);
	if (ret){   
		pr_err("failed register red led reboot notifier\n");
        return ret;
    }

    register_early_suspend(&led_red_suspend_desc);

	return ret;
}
module_init(led_red_init);

static void __exit led_red_exit(void)
{
    unregister_reboot_notifier(&led_red_notifier);	
    unregister_early_suspend(&led_red_suspend_desc);

    led_classdev_unregister(&led_red);
}
module_exit(led_red_exit);


MODULE_DESCRIPTION("Red LED Driver");
MODULE_AUTHOR("GPL");
