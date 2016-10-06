/*
 * leds-intel-orange.c - Intel Orange LED driver
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
#include <linux/hrtimer.h>
#include <linux/delay.h>
#include <linux/reboot.h>
#include <linux/mfd/intel_msic.h>

#include <asm/intel_scu_pmic.h>
#include <asm/intel_mid_pwm.h>
#include <asm/intel_mid_remoteproc.h>


static int orange_led_timeout;

static void timed_control_init(void);
static void timed_control_remove(void);
static void led_on_timer_fun(unsigned long);
static void led_off_timer_fun(unsigned long);

static int timed_control_work = 0;
static int led_flash_time = 1;

#define MSG_WARN_ON  300
#define MSG_WARN_OFF  700

static struct timer_list  led_on_timer;
static struct timer_list  led_off_timer;
static struct work_struct led_on_work;
static struct work_struct led_off_work;

#define RED_LED_STOP    0
#define RED_LED_RUN     -1
#define RED_LED_PWM_1S  1

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
    else
    {
        pr_err("set red led mode error!!!\n");
        return -1;
    }
    //pr_info("set red led value = %d ok!!!\n",value);
    
	return ret;
}

static int led_orange_notify_sys(struct notifier_block *this, unsigned long code,void *unused);

static struct notifier_block led_orange_notifier = {
	.notifier_call = led_orange_notify_sys,
};

static int led_orange_notify_sys(struct notifier_block *this, unsigned long code,
			  void *unused)
{
	if (code == SYS_DOWN || code == SYS_POWER_OFF)
	{
		intel_mid_pwm(PWM1, 0);
        intel_msic_set_red_led(0);
	}	
    
	return NOTIFY_DONE;
}

static void led_orange_early_suspend(struct early_suspend *h)
{
    if(orange_led_timeout == 1)
    {
        timed_control_remove();
        intel_mid_pwm(PWM1, 0);
        intel_msic_set_red_led(0);
    }
}

static void led_orange_late_resume(struct early_suspend *h)
{
    if(orange_led_timeout == 1)
    {
        intel_mid_pwm(PWM1, 100);
        intel_msic_set_red_led(-1);
        timed_control_init();
        mod_timer(&led_on_timer,jiffies + msecs_to_jiffies(MSG_WARN_ON));   
    }
}

static struct early_suspend led_orange_suspend_desc = {
	.level   = EARLY_SUSPEND_LEVEL_DISABLE_FB,
	.suspend = led_orange_early_suspend,
	.resume  = led_orange_late_resume,
};

static void timed_control_init(void)
{      
    if(0 == timed_control_work)
    {
        setup_timer(&led_on_timer, led_on_timer_fun,0); 
        setup_timer(&led_off_timer, led_off_timer_fun,0); 
        timed_control_work = 1;
    }
}

static void timed_control_remove(void)
{
    if(1 == timed_control_work)
    {
        del_timer(&led_on_timer);
        del_timer(&led_off_timer);
        timed_control_work = 0;
    }
}

static void led_on_timer_fun(unsigned long data)
{      
    mod_timer(&led_off_timer,jiffies + msecs_to_jiffies(MSG_WARN_OFF));   
    schedule_work(&led_off_work);    
}

static void led_off_timer_fun(unsigned long data)
{
    led_flash_time ++;
    if(led_flash_time >= 32767)
        led_flash_time = 1;
    mod_timer(&led_on_timer,jiffies + msecs_to_jiffies(MSG_WARN_ON));
    schedule_work(&led_on_work);
}

static void led_on_work_fun(struct work_struct *work)
{
    intel_mid_pwm(PWM1, 100); 
    intel_msic_set_red_led(-1);
}
static void led_off_work_fun(struct work_struct *work)
{
    intel_mid_pwm(PWM1, 0); 
    intel_msic_set_red_led(0);
}

static void led_orange_set(struct led_classdev *led_cdev,
	enum led_brightness value)
{
    pr_info("ZTE set orange led value = %d\n",value);
    orange_led_timeout = value;

    if(0 == value)
    {
        timed_control_remove();
        intel_mid_pwm(PWM1, 0);
        intel_msic_set_red_led(0);
        led_flash_time = 1;
    }
    else if(1 == value)
    {
        timed_control_remove();
        intel_mid_pwm(PWM1, 0);
        intel_msic_set_red_led(0);
        intel_mid_pwm(PWM1, 100);
        intel_msic_set_red_led(-1);
        timed_control_init();
        mod_timer(&led_on_timer,jiffies + msecs_to_jiffies(MSG_WARN_ON));   
	}
    else
    {
        pr_err("set led green on_time error!!!\n");
    }
}

static struct led_classdev led_orange = {
	.name			    = "led_orange",
	.brightness_set		= led_orange_set,
	.brightness		    = LED_OFF,
	.max_brightness		= 100,
};

static int __init led_orange_init(void)
{
	int ret;

    INIT_WORK(&led_on_work, led_on_work_fun);
    INIT_WORK(&led_off_work, led_off_work_fun); 
    
	ret = led_classdev_register(NULL, &led_orange);
	if (ret) {
		pr_err("register orange led failed\n");
		return ret;
	}
	led_orange_set(&led_orange, led_orange.brightness);

    ret = register_reboot_notifier(&led_orange_notifier);
	if (ret){   
		pr_err("failed register orange led reboot notifier\n");
        return ret;
    }

    register_early_suspend(&led_orange_suspend_desc);

	return ret;
}
module_init(led_orange_init);

static void __exit led_orange_exit(void)
{
    cancel_work_sync(&led_on_work);
    cancel_work_sync(&led_off_work);

    unregister_early_suspend(&led_orange_suspend_desc);

    unregister_reboot_notifier(&led_orange_notifier);
	led_classdev_unregister(&led_orange);
}
module_exit(led_orange_exit);

MODULE_DESCRIPTION("Orange Led device");
MODULE_AUTHOR("GPL");
