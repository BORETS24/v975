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
 *  Filename: wukong_boot.c
 *
 *  Description: Boot features init and interupt handling.
 *
 *  History:
 *   Nov, 11 2011 -  Li Wang(wangli@marvell.com) Creation of file
 *
 *  Notes:
 *
 ******************************************************************************/
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <asm/intel-mid.h>
#include "wukong_boot.h"
#include "wukong_config.h"

static enum modemState modem_state = HOLD;
static spinlock_t modem_state_lock;
static struct uevent_work boot_uevent_wk;
static int power_on_done = 0;
static struct modem_device* wk_modem;
irqreturn_t modem_bootup(int irq, void *dev_id);
static struct delayed_work m1802_poweron_work;

static char* modem_state_name[]=
{
	"HOLD",
	"RELEASE",
	"BOOTUP",
	"UNKNOWN_STATE"
};

void set_modem_state(struct modem_device * modem, enum modemState state,char * env[])
{
	if(env != NULL)
	{
	switch(state)
	{
	case HOLD:
		env[0]="STATE=HOLD";
		break;
	case RELEASE:
		env[0]="STATE=RELEASE";
		break;
	case BOOTUP:
		env[0]="STATE=BOOTUP";
		break;
	default:
		env[0]="STATE=UNKNOWN";
		break;
	}
	env[1] = NULL;
	}
	modem_state = state;
}

enum modemState get_modem_state(void)
{
	return modem_state;
}

char* get_modem_state_name(void)
{
	if(modem_state <= UNKNOWN_STATE && modem_state >=HOLD)
	{
		return modem_state_name[modem_state];
	}
	return "UNKNOW_STATE";
}

void poweron_modem(void)
{
	struct modem_device* modem = wk_modem;

	DEBUGMSG("%s: poweron_modem operation triggered\n", __FUNCTION__);
	
	// turn on vsys
	set_line(modem->power_enable1, modem->active_level);

	// trigger power key
/*
	mdelay(500);
	set_line(modem->power_enable2, modem->active_level);
	mdelay(100);
	set_line(modem->power_enable2, !modem->active_level);
*/
}

void hold_modem(int internal)
{
	char * env[2];

	struct modem_device* modem = wk_modem;
	if(NULL == modem)
		return;
	DEBUGMSG("%s: hold_modem operation triggered\n", __FUNCTION__);

	spin_lock(&modem_state_lock);
	if(modem_state == HOLD)
	{
		spin_unlock(&modem_state_lock);
        DEBUGMSG("%s: HOLD return\n", __FUNCTION__);
		return;
	}

	//ignore the fake irq
	DEBUGMSG("%s: diable irq\n", __FUNCTION__);
	disable_irq(modem->boot_up_irq);
	disable_irq(modem->watch_dog_irq);
	disable_irq(modem->assert_eeh_irq);

	if (!internal)
		poweron_modem();
	
	if(!power_on_done)
	{
		set_line(modem->reset_gpio, !modem->active_level);
	}
	else
	{
		set_line(modem->reset_gpio, !modem->active_level);
	}

	set_modem_state(modem, HOLD, env);
	spin_unlock(&modem_state_lock);

	mdelay(100);

	if(!internal)
	{
		RETAILMSG("%s: hold uevent is sent\n", __FUNCTION__);
		kobject_uevent_env(&modem->dev->kobj,KOBJ_CHANGE,env);
	}
}

void release_modem(int internal)
{
	char * env[2];

	struct modem_device* modem = wk_modem;
	if(NULL == modem)
		return;

	DEBUGMSG("%s: release_modem operation triggered\n", __FUNCTION__);
	spin_lock(&modem_state_lock);
	if(modem_state != HOLD) // Can only release modmem when it is being hold
	{
		spin_unlock(&modem_state_lock);
		return;
	}
	if(!power_on_done)
	{
		set_line(modem->reset_gpio, modem->active_level);
		power_on_done = 1;
		mdelay(1000);
	}
	else
	{
		set_line(modem->reset_gpio, modem->active_level);
	}

	set_modem_state(modem, RELEASE,env);
	spin_unlock(&modem_state_lock);

	if(!internal)
	{
		mdelay(1000);
		RETAILMSG("%s: release uevent is sent\n", __FUNCTION__);
		kobject_uevent_env(&modem->dev->kobj,KOBJ_CHANGE,env);
	}

	/*
	enable_irq(modem->boot_up_irq);
	enable_irq(modem->watch_dog_irq);
	enable_irq(modem->assert_eeh_irq);
	enable_irq_wake(modem->watch_dog_irq);
	enable_irq_wake(modem->assert_eeh_irq);
	*/
}

void uart_dev_open(void)
{
	struct modem_device* modem = wk_modem;
//	set_line(modem->uart_switch_open, 0);
}

void uart_dev_close(void)
{
	struct modem_device* modem = wk_modem;
//	set_line(modem->uart_switch_open, 1);
}

void boot_uevent_work(struct work_struct *work)
{
	struct uevent_work * uevent_wq = container_of(work,struct uevent_work,work);
	struct modem_device* modem = wk_modem;

	msleep(10);
	if(uevent_wq->modem->active_level != get_line(GPIO_BOOTUP_CUSTOM))
	{
		spin_lock(&modem_state_lock);
		set_modem_state(modem, RELEASE,boot_uevent_wk.env);
		spin_unlock(&modem_state_lock);
		ERRORMSG("%s: fake boot interupt is detected\n", __FUNCTION__);
		return;
	}

	//debug
	//set_modem_state(modem, BOOTUP,boot_uevent_wk.env);
	
	RETAILMSG("%s: boot up uevent is sent\n", __FUNCTION__);
	kobject_uevent_env(&uevent_wq->modem->dev->kobj, KOBJ_CHANGE, uevent_wq->env);
}

static void m1802_poweron_worker(struct work_struct *work)
{
	release_modem(1);
	DEBUGMSG(KERN_INFO "%s", __func__);
}

int init_modem_boot(struct modem_device * modem)
{
	int ret = -1;
	spin_lock_init(&modem_state_lock);
	wk_modem = modem;

	modem_state = UNKNOWN_STATE;

#ifdef TEST_GPIO
	int i = 20;
	while(i-- > 0)
	{
		if(i%2)
			set_line(modem->reset_gpio, 1);
		else
			set_line(modem->reset_gpio, 0);
		msleep(1000);
	}
#endif

	if (gpio_request(modem->power_enable1, "MODEM vsys")){
		ERRORMSG("%s: GPIO[%d] request failed\n", __FUNCTION__, modem->power_enable1);
		return;
	}
	gpio_export( modem->power_enable1, true);
	
	if (gpio_request(modem->power_enable2, "MODEM poweron")){
		ERRORMSG("%s: GPIO[%d] request failed\n", __FUNCTION__, modem->power_enable1);
		return;
	}
	gpio_export( modem->power_enable2, true);
	
	if (gpio_request(modem->reset_gpio, "MODEM reset")){
		ERRORMSG("%s: GPIO[%d] request failed\n", __FUNCTION__, modem->power_enable1);
		return;
	}
	gpio_export( modem->reset_gpio, true);

	if (gpio_request( GPIO_USB_SEL, "MODEM usb sel")){
		ERRORMSG("%s: GPIO[%d] request failed\n", __FUNCTION__, GPIO_USB_SEL);
		return;
	}
	gpio_direction_output(GPIO_USB_SEL, 0);
	gpio_export( GPIO_USB_SEL, true);

	//init to hold status
	hold_modem(1);
	
	//release_modem(1);
/*
	if(gpio_request(91, "GPIO"))
	{
		ERRORMSG("%s: GPIO[%d] request failed\n", __FUNCTION__, 91);
		//return;
	}
	gpio_direction_output(91, 1);
	mdelay(2000);
	gpio_free(91);	
	RETAILMSG("%s: GPIO out pin = %d, level = %d\n",__FUNCTION__,  91, 1);
*/	
	//mdelay(2000);

	INIT_DELAYED_WORK(&m1802_poweron_work, m1802_poweron_worker);
	schedule_delayed_work(&m1802_poweron_work, msecs_to_jiffies(1000));

	INIT_WORK(&boot_uevent_wk.work, boot_uevent_work);//todo
	boot_uevent_wk.modem = modem;

	modem->boot_up_irq = gpio_to_irq(GPIO_BOOTUP_CUSTOM);
	modem->watch_dog_irq = gpio_to_irq(GPIO_WDTRST_CUSTOM);
	modem->assert_eeh_irq = gpio_to_irq(GPIO_EEH_CUSTOM);

	if (gpio_request(GPIO_BOOTUP_CUSTOM, "CP BOOTUP IRQ"))
	{
		ERRORMSG("%s: CP BOOTUP IRQ, GPIO[%d] request failed!\n", __FUNCTION__, GPIO_BOOTUP_CUSTOM);
		return ret;
	}
	gpio_export( GPIO_BOOTUP_CUSTOM, true);
	gpio_direction_input(GPIO_BOOTUP_CUSTOM);

	ret = request_irq(modem->boot_up_irq, modem_bootup, IRQF_DISABLED|IRQF_TRIGGER_RISING, "modem boot up", modem);
	if(ret < 0)
	{
		ERRORMSG("%s:Can't request irq for modem boot:%d!\n", __FUNCTION__, modem->boot_up_irq);
		return ret;
	}

	return 0;
}

void deinit_modem_boot(struct modem_device * modem)
{
//	free_irq(modem->boot_up_irq, NULL);
//	gpio_free(irq_to_gpio(modem->boot_up_irq));
	hold_modem(1);
	modem_state = UNKNOWN_STATE;
	wk_modem = NULL;
}

irqreturn_t modem_bootup(int irq, void *dev_id)
{
	struct modem_device * modem = (struct modem_device *)dev_id;
	spin_lock(&modem_state_lock);

	if(modem_state != RELEASE)
	{
		spin_unlock(&modem_state_lock);
		ERRORMSG("%s:It is not a expected interrupt!\n",__FUNCTION__);
		return IRQ_NONE;
	}

	set_modem_state(modem, BOOTUP,boot_uevent_wk.env);
	queue_work(modem->modem_wq,&boot_uevent_wk.work);
	spin_unlock(&modem_state_lock);
	return IRQ_HANDLED;
}


void modem_bootup_test(void){
	if(wk_modem)
		queue_work(wk_modem->modem_wq,&boot_uevent_wk.work);
}
