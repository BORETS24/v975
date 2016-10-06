/*
 * leds-intel-kpd.c - Intel Keypad LED driver
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

#include <asm/intel_scu_pmic.h>
#include <asm/intel_mid_pwm.h>
#include <asm/intel_mid_remoteproc.h>


static int green_led_timeout;

static void timed_control_init(void);
static void timed_control_remove(void);
static void led_on_timer_fun(unsigned long);
static void led_off_timer_fun(unsigned long);

static int timed_control_work = 0;
static int led_flash_time = 1;

#define MSG_WARN_ON  500
#define MSG_WARN_OFF  500

static struct timer_list  led_on_timer;
static struct timer_list  led_off_timer;
static struct work_struct led_on_work;
static struct work_struct led_off_work;

static int led_green_notify_sys(struct notifier_block *this, unsigned long code,void *unused);

static struct notifier_block led_green_notifier = {
	.notifier_call = led_green_notify_sys,
};

static int led_green_notify_sys(struct notifier_block *this, unsigned long code,
			  void *unused)
{
	if (code == SYS_DOWN || code == SYS_POWER_OFF)
	{
		intel_mid_pwm(PWM1, 0);
	}	
    
	return NOTIFY_DONE;
}

static void led_green_early_suspend(struct early_suspend *h)
{
    if(green_led_timeout == 1)
        intel_mid_pwm(PWM1_4S, 7);
}

static void led_green_late_resume(struct early_suspend *h)
{
    if(green_led_timeout == 1)
        intel_mid_pwm(PWM1_1S, 30);
}

static struct early_suspend led_green_suspend_desc = {
	.level   = EARLY_SUSPEND_LEVEL_DISABLE_FB,
	.suspend = led_green_early_suspend,
	.resume  = led_green_late_resume,
};

static void led_green_set(struct led_classdev *led_cdev,
	enum led_brightness value)
{
    //pr_info("ZTE set green led value = %d\n",value);
    green_led_timeout = value;

    if (100 == value) 
    {
        intel_mid_pwm(PWM1, 100);
    } 
    else if(0 == value)
    {
        intel_mid_pwm(PWM1, 0);
    }
    else if(1 == value)
    {
        intel_mid_pwm(PWM1_1S, 0);
        intel_mid_pwm(PWM1_1S, 30);
	}
    else
    {
        pr_err("set led green on_time error!!!\n");
    }
}

static struct led_classdev led_green = {
	.name			    = "led_green",
	.brightness_set		= led_green_set,
	.brightness		    = LED_OFF,
	.max_brightness		= 100,
};

static int __init led_green_init(void)
{
	int ret;

	ret = led_classdev_register(NULL, &led_green);
	if (ret) {
		pr_err("register green led failed\n");
		return ret;
	}
	led_green_set(&led_green, led_green.brightness);

    ret = register_reboot_notifier(&led_green_notifier);
	if (ret){   
		pr_err("failed register green led reboot notifier\n");
        return ret;
    }

    register_early_suspend(&led_green_suspend_desc);

	return ret;
}
module_init(led_green_init);

static void __exit led_green_exit(void)
{
    unregister_early_suspend(&led_green_suspend_desc);

    unregister_reboot_notifier(&led_green_notifier);
	led_classdev_unregister(&led_green);
}
module_exit(led_green_exit);

MODULE_DESCRIPTION("Green LED Driver");
MODULE_LICENSE("GPL v2");
