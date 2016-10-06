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
 *  Filename: wukong_assert.c
 *
 *  Description: Assert features init and interupt handling.
 *
 *  History:
 *   Nov, 11 2011 -  Li Wang(wangli@marvell.com) Creation of file
 *
 *  Notes:
 *
 ******************************************************************************/
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/gpio.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/wakelock.h>
#include <linux/version.h>

#include "wukong_assert.h"
#include "wukong_obm_transfer.h"
#include "wukong_config.h"
#include "wukong_boot.h"
#include "wukong_load.h"

static enum modemErrorType modem_error;
static spinlock_t modem_state_lock;
static struct wake_lock erro_wakeup;

#ifdef WDT_DOWNLOAD_DKBI_IN_DRIVER
static DEFINE_SPINLOCK(uart_download_lock);
#endif

static struct uevent_work assert_uevent_wk;
struct delayed_uevent_work wdt_uevent_wk;
irqreturn_t _modem_error(int irq, void *dev_id);


void assert_uevent_work(struct work_struct *work)
{
	struct uevent_work * uevent_wq = container_of(work,struct uevent_work,work);

	msleep(100);
	if((uevent_wq->modem->active_level != get_line(GPIO_EEH_CUSTOM))
	  ||(uevent_wq->modem->active_level == get_line(GPIO_WDTRST_CUSTOM))
	 )
	{
		ERRORMSG("%s: fake EEH Assert interupt is detected\n", __FUNCTION__);
		return;
	}

	kobject_uevent_env(&uevent_wq->modem->dev->kobj, KOBJ_CHANGE, uevent_wq->env);
	RETAILMSG("%s: assert error uevent[%s][%s] is sent\n", __FUNCTION__, uevent_wq->env[0], uevent_wq->env[1]);
}

void wdt_uevent_work(struct work_struct *work)
{
#ifdef WDT_DOWNLOAD_DKBI_IN_DRIVER
	struct timeval time_start, time_end;
#endif
	struct delayed_uevent_work * uevent_wq = container_of(work, struct delayed_uevent_work, work.work);//tocheck

	msleep(500);//todo too short will cause wukong unstable
	//check the irq pin level
	//todo high level effective?
	if(uevent_wq->modem->active_level != get_line(GPIO_WDTRST_CUSTOM))
	{
		ERRORMSG("%s: fake WDT interupt is detected\n", __FUNCTION__);
		return;
	}

	msleep(100);

	RETAILMSG("%s: wdt interrupt triggered\n", __FUNCTION__);

#ifdef WDT_DOWNLOAD_DKBI_IN_DRIVER
	spin_lock(&uart_download_lock);
	if(DOWNLOADING_IMG == get_download_state())
	{
		ERRORMSG("%s: Downloading is already run\n", __FUNCTION__);
		spin_unlock(&uart_download_lock);
		return;
	}

	if(uevent_wq->modem->platform_type == PLATFORM_910_WUKONG)
	{
		uart_dev_open();
	}

	set_download_state(DOWNLOADING_IMG);
	spin_unlock(&uart_download_lock);
	DEBUGMSG("%s: start wdt_uevent_work\n", __FUNCTION__);

	//transfer obm via uart
	do_gettimeofday(&time_start);
	DEBUGMSG("%s: OBM transfer thread is running!\n", __FUNCTION__);

	spin_lock(&uart_download_lock);
	set_download_state(DOWNLOADING_IDLE);
	spin_unlock(&uart_download_lock);
	do_gettimeofday(&time_end);
	RETAILMSG("%s: obm transfer time spent (%d)m, (%d)us\n", __FUNCTION__,
			(int)(time_end.tv_sec - time_start.tv_sec),
			(int)(time_end.tv_usec - time_start.tv_usec));
#else
	kobject_uevent_env(&uevent_wq->modem->dev->kobj, KOBJ_CHANGE, uevent_wq->env);
	RETAILMSG("%s: wdt uevent[%s][%s]\n", __FUNCTION__, uevent_wq->env[0], uevent_wq->env[1]);
#endif
}

void report_eeh_dump_event(void)
{
	char * env[2];
	env[0]="ERROR=EEHDUMP";
	env[1]=NULL;

	RETAILMSG("%s: eehdump uevent is sent\n", __FUNCTION__);
	kobject_uevent_env(&assert_uevent_wk.modem->dev->kobj,KOBJ_CHANGE,env);
}

int init_modem_assert(struct modem_device * modem)
{
	int ret = -1;
	spin_lock_init(&modem_state_lock);
	INIT_WORK(&assert_uevent_wk.work, assert_uevent_work);
	INIT_DELAYED_WORK(&wdt_uevent_wk.work, wdt_uevent_work);
	wake_lock_init(&erro_wakeup, WAKE_LOCK_SUSPEND, "modem_error_wakeups");
	assert_uevent_wk.modem = modem;
	wdt_uevent_wk.modem = modem;


	if(gpio_request(GPIO_WDTRST_CUSTOM, "CP WDT IRQ"))
	{
		ERRORMSG("%s: CP EEH WDTRST IRQ, GPIO[%d] request failed!\n", __FUNCTION__, GPIO_WDTRST_CUSTOM);
		return ret;
	}

	if(gpio_request( GPIO_EEH_CUSTOM, "CP ASSERT IRQ"))
	{
		ERRORMSG("%s: CP EEH IRQ, GPIO[%d] request failed!\n", __FUNCTION__, GPIO_EEH_CUSTOM);
		return ret;
	}

    gpio_export( GPIO_WDTRST_CUSTOM, true);
	gpio_direction_input(GPIO_WDTRST_CUSTOM);
    
    gpio_export( GPIO_EEH_CUSTOM, true);
	gpio_direction_input(GPIO_EEH_CUSTOM);

	ret = request_irq(modem->assert_eeh_irq, _modem_error, IRQF_DISABLED|IRQF_TRIGGER_RISING|IRQF_NO_SUSPEND, "modem assert", modem);
	if(ret < 0)
	{
		ERRORMSG("%s:Can't request irq for modem assert:%d!\n",__FUNCTION__,modem->assert_eeh_irq);
		return ret;
	}
	//disable_irq(modem->assert_eeh_irq);

	ret = request_irq(modem->watch_dog_irq, _modem_error, IRQF_DISABLED|IRQF_TRIGGER_RISING|IRQF_NO_SUSPEND, "modem wdt", modem);
	if(ret < 0)
	{
		ERRORMSG("%s:Can't request irq for modem wdt:%d!\n",__FUNCTION__,modem->watch_dog_irq);
		free_irq(modem->assert_eeh_irq, NULL);
		return ret;
	}
    //disable_irq(modem->watch_dog_irq);
    
	return 0;
}

void deinit_modem_assert(struct modem_device * modem)
{
	free_irq(modem->assert_eeh_irq, NULL);
	free_irq(modem->watch_dog_irq, NULL);
	disable_irq_wake(modem->assert_eeh_irq);
	disable_irq_wake(modem->watch_dog_irq);
	gpio_free(irq_to_gpio(modem->assert_eeh_irq));
	gpio_free(irq_to_gpio(modem->watch_dog_irq));
}

void set_modem_error(struct modem_device * modem, enum modemErrorType error,char * env[])
{
	if(env != NULL)
	{
	switch(error)
	{
	case WATCHDOGTIMEOUT:
		env[0] = "ERROR=WDTIMEOUT";
		break;
	case ASSERT:
		env[0] = "ERROR=ASSERT";
		break;
	default:
		env[0] = "ERROR=UNKNOWN";
		break;
	}
	env[1] = NULL;
	}
	modem_error = error;
}

irqreturn_t _modem_error(int irq, void *dev_id)
{
	char * env[2];
	struct modem_device * modem = (struct modem_device *)dev_id;
	int sec = 3;

	spin_lock(&modem_state_lock);
	RETAILMSG("%s: asssert irq received. irq[%d] (wdt_irq[%d] assert_irq[%d]) hold %ds wake lock\n", __FUNCTION__, irq, modem->watch_dog_irq, modem->assert_eeh_irq,sec);

	if(irq == modem->watch_dog_irq)
	{
		set_modem_error(modem,WATCHDOGTIMEOUT,wdt_uevent_wk.env);
		queue_delayed_work(modem->modem_wq,&wdt_uevent_wk.work, HZ/10);
		DEBUGMSG("HZ=%d", HZ);
	}
	else if(irq == modem->assert_eeh_irq)
	{
		set_modem_error(modem, ASSERT, assert_uevent_wk.env);
		queue_work(modem->modem_wq, &assert_uevent_wk.work);
	}
	else
	{
		env[0] = "ERROR=UNKNOWN";
		modem_error = UNKNOWN_ERROR;
		goto RET;
	}

#if 0// undeine these code due to mfp_read mfp_write not export by kernel.
/* need to reset mfp edge status to clear pending wake up source  after marvell 3.0 kernel*/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0)
	{
		int gpio = irq_to_gpio(irq);
		u32 mfp = mfp_read(gpio);
		mfp_write(gpio, (1 << 6) | mfp);
		mfp_write(gpio, mfp);
	}
#endif
#endif

RET:
	spin_unlock(&modem_state_lock);

	wake_lock_timeout(&erro_wakeup, HZ*sec);
	return IRQ_HANDLED;
}

