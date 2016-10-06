/*
 *  ctp_common.c - ASoc Machine driver for Intel Clovertrail MID platform
 *
 *  Copyright (C) 2011-13 Intel Corp
 *  Author: KP Jeeja<jeeja.kp@intel.com>
 *  Author: Vaibhav Agarwal <vaibhav.agarwal@intel.com>
 *  Author: Dharageswari.R<dharageswari.r@intel.com>
 *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */


#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/wakelock.h>
#include <linux/gpio.h>
#include <linux/rpmsg.h>
#include <linux/mod_devicetable.h>
#include <asm/intel_mid_gpadc.h>
#include <asm/intel_scu_pmic.h>
#include <asm/intel_scu_ipcutil.h>
#include <asm/intel_mid_rpmsg.h>
#include <asm/intel_mid_remoteproc.h>
#include <asm/platform_ctp_audio.h>
#include <sound/pcm.h>
#include <sound/jack.h>
#include <sound/soc.h>
#include "ctp_common.h"

/* Headset jack detection gpios func(s) */
#define HPDETECT_POLL_INTERVAL  msecs_to_jiffies(300)  /* 300ms */
#define HPDETECT_INSERT_INTERVAL  msecs_to_jiffies(800)  /* 800ms */
#define HS_DET_RETRY	5
#ifdef CTP_EXT_SPK_PA
extern unsigned int ext_pa_boost_switch;
extern unsigned int ext_pa_switch;
#endif

struct snd_soc_card snd_soc_card_ctp = {
	.name = "cloverview_audio",
	.set_bias_level = ctp_set_bias_level,
	.set_bias_level_post = ctp_set_bias_level_post,
};

unsigned int rates_8000_16000[] = {
	8000,
	16000,
};

struct snd_pcm_hw_constraint_list constraints_8000_16000 = {
	.count = ARRAY_SIZE(rates_8000_16000),
	.list = rates_8000_16000,
};

unsigned int rates_48000[] = {
	48000,
};

struct snd_pcm_hw_constraint_list constraints_48000 = {
	.count  = ARRAY_SIZE(rates_48000),
	.list   = rates_48000,
};

unsigned int rates_16000[] = {
	16000,
};

struct snd_pcm_hw_constraint_list constraints_16000 = {
	.count  = ARRAY_SIZE(rates_16000),
	.list   = rates_16000,
};

static struct snd_soc_jack_gpio hs_gpio[] = {
	[CTP_HSDET_GPIO] = {
		.name = "cs-hsdet-gpio",
		.report = SND_JACK_HEADSET,
		.jack_status_check = ctp_soc_jack_gpio_detect,
	},
	[CTP_BTN_GPIO] = {
		.name = "cs-hsbutton-gpio",
		.report = SND_JACK_HEADSET | SND_JACK_BTN_0,
		.debounce_time = 100,
		.jack_status_check = ctp_soc_jack_gpio_detect_bp,
		.irq_flags = IRQF_TRIGGER_FALLING,
	},
};

int ctp_set_clk_fmt(struct snd_soc_dai *codec_dai, struct ctp_clk_fmt *clk_fmt)
{
	int ret;

	/* Set codec DAI configuration */
	ret = snd_soc_dai_set_fmt(codec_dai, clk_fmt->fmt);

	if (ret < 0) {
		pr_err("can't set codec DAI configuration %d\n", ret);
		return ret;
	}

	ret = snd_soc_dai_set_sysclk(codec_dai, clk_fmt->clk_id,
					clk_fmt->freq, clk_fmt->dir);
	if (ret < 0) {
		pr_err("can't set codec clock %d\n", ret);
		return ret;
	}

	return 0;
}

/* Board specific codec bias level control */
int ctp_set_bias_level(struct snd_soc_card *card,
		struct snd_soc_dapm_context *dapm,
		enum snd_soc_bias_level level)
{
	struct snd_soc_codec *codec;

	/* Clock management is done only if there is an associated codec
	 * to dapm context and if this not the dummy codec
	 */
	if (dapm->codec) {
		codec = dapm->codec;
		if (!strcmp(codec->name, "snd-soc-dummy"))
			return 0;
	} else {
		pr_debug("In %s dapm context has no associated codec or it is dummy codec.", __func__);
		return 0;
	}

	switch (level) {
	case SND_SOC_BIAS_ON:
	case SND_SOC_BIAS_PREPARE:
	case SND_SOC_BIAS_STANDBY:
		if (card->dapm.bias_level == SND_SOC_BIAS_OFF)
			intel_scu_ipc_set_osc_clk0(true, CLK0_MSIC);
		card->dapm.bias_level = level;
		break;
	case SND_SOC_BIAS_OFF:
		/* OSC clk will be turned OFF after processing
		 * codec->dapm.bias_level = SND_SOC_BIAS_OFF.
		 */
		break;
	default:
		pr_err("%s: Invalid bias level=%d\n", __func__, level);
		return -EINVAL;
		break;
	}
	pr_debug("card(%s)->bias_level %u\n", card->name,
			card->dapm.bias_level);

	return 0;
}

int ctp_set_bias_level_post(struct snd_soc_card *card,
		struct snd_soc_dapm_context *dapm,
		enum snd_soc_bias_level level)
{
	struct snd_soc_codec *codec;

	/* Clock management is done only if there is an associated codec
	 * to dapm context and if this not the dummy codec
	 */
	if (dapm->codec) {
		codec = dapm->codec;
		if (!strcmp(codec->name, "snd-soc-dummy"))
			return 0;
	} else {
		pr_debug("In %s dapm context has no associated codec or it is dummy codec.", __func__);
		return 0;
	}

	switch (level) {
	case SND_SOC_BIAS_ON:
	case SND_SOC_BIAS_PREPARE:
	case SND_SOC_BIAS_STANDBY:
		/* Processing already done during set_bias_level()
		 * callback. No action required here.
		 */
		break;
	case SND_SOC_BIAS_OFF:
		if (codec->dapm.bias_level != SND_SOC_BIAS_OFF)
			break;
		intel_scu_ipc_set_osc_clk0(false, CLK0_MSIC);
		card->dapm.bias_level = level;
		break;
	default:
		pr_err("%s: Invalid bias level=%d\n", __func__, level);
		return -EINVAL;
		break;
	}
	pr_debug("%s:card(%s)->bias_level %u\n", __func__, card->name,
			card->dapm.bias_level);
	return 0;
}

static int set_mic_bias(struct snd_soc_jack *jack,
			const char *bias_widget, bool enable)
{
	struct snd_soc_codec *codec = jack->codec;
	struct snd_soc_dapm_context *dapm = &codec->dapm;

	if (enable)
		snd_soc_dapm_force_enable_pin(dapm, bias_widget);
	else
		snd_soc_dapm_disable_pin(dapm, bias_widget);

	snd_soc_dapm_sync(&codec->dapm);

	return 0;
}
#ifdef CTP_EXT_SPK_PA
int ext_pa_boost_get_switch(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = ext_pa_boost_switch;
	return 0;
}

int ext_pa_boost_set_switch(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	printk("%s: ucontrol->value.integer.value[0] %ld, ext_pa_boost_switch %d\n", __func__, ucontrol->value.integer.value[0], ext_pa_boost_switch);
	if (ucontrol->value.integer.value[0] == ext_pa_boost_switch)
		return 0;
	
	if (ucontrol->value.integer.value[0]) {
		printk("external pa boost enable\n");
		gpio_set_value(EXT_PA_BOOST_EN_PIN, EXT_PA_ON);
	} else {
		printk("external pa boost disable\n");
		gpio_set_value(EXT_PA_BOOST_EN_PIN, EXT_PA_OFF);
	}
	ext_pa_boost_switch = ucontrol->value.integer.value[0];
	return 0;
}

int ext_pa_get_switch(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = ext_pa_switch;
	return 0;
}

int ext_pa_set_switch(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	printk("%s: ucontrol->value.integer.value[0] %ld, ext_pa_switch %d\n", __func__, ucontrol->value.integer.value[0], ext_pa_switch);
	if (ucontrol->value.integer.value[0] == ext_pa_switch)
		return 0;

	if (ucontrol->value.integer.value[0]) {
		printk("external pa enable\n");
		gpio_set_value(EXT_PA_EN_PIN, EXT_PA_ON);
	} else {
		printk("external pa disable\n");
		gpio_set_value(EXT_PA_EN_PIN, EXT_PA_OFF);
	}
	ext_pa_switch = ucontrol->value.integer.value[0];
	return 0;
}
#endif

int ctp_amp_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *k, int event)
{
	int ret;

	if (SND_SOC_DAPM_EVENT_ON(event)) {
		/*Enable  IHFAMP_SD_N  GPIO */
		ret = intel_scu_ipc_iowrite8(GPIOHVCTL, GPIO_AMP_ON);
		if (ret)
			pr_err("write of  failed, err %d\n", ret);
	} else {
		/*Disable  IHFAMP_SD_N  GPIO */
		ret = intel_scu_ipc_iowrite8(GPIOHVCTL, GPIO_AMP_OFF);
		if (ret)
			pr_err("write of  failed, err %d\n", ret);
	}
	return 0;
}

static inline void set_bp_interrupt(struct ctp_mc_private *ctx, bool enable)
{
	if (!enable) {
		if (!atomic_dec_return(&ctx->bpirq_flag)) {
			pr_debug("Disable %d interrupt line\n", ctx->bpirq);
			disable_irq_nosync(ctx->bpirq);
		} else
			atomic_inc(&ctx->bpirq_flag);
	} else {
		if (atomic_inc_return(&ctx->bpirq_flag) == 1) {
			/* If BP intr not enabled */
			pr_debug("Enable %d interrupt line\n", ctx->bpirq);
			enable_irq(ctx->bpirq);
		} else
			atomic_dec(&ctx->bpirq_flag);
	}
}

void cancel_all_work(struct ctp_mc_private *ctx)
{
	struct snd_soc_jack_gpio *gpio;
	cancel_delayed_work_sync(&ctx->jack_work_insert);
	cancel_delayed_work_sync(&ctx->jack_work_remove);
	gpio = &hs_gpio[CTP_BTN_GPIO];
	cancel_delayed_work_sync(&gpio->work);
}

int ctp_soc_jack_gpio_detect(void)
{
	struct snd_soc_jack_gpio *gpio = &hs_gpio[CTP_HSDET_GPIO];
	struct snd_soc_jack *jack = gpio->jack;
	struct ctp_mc_private *ctx =
		container_of(jack, struct ctp_mc_private, ctp_jack);

	int enable;

	/* During jack removal, spurious BP interrupt may occur.
	 * Better to disable interrupt until jack insert/removal stabilize.
	 * Also cancel the BP and jack_work if already sceduled */
	cancel_all_work(ctx);
	set_bp_interrupt(ctx, false);
	enable = gpio_get_value(gpio->gpio);
	if (gpio->invert)
		enable = !enable;
	pr_debug("%s:gpio->%d=0x%d\n", __func__, gpio->gpio, enable);

	if (!enable) {
		//atomic_set(&ctx->hs_det_retry, HS_DET_RETRY);
		schedule_delayed_work(&ctx->jack_work_insert,
					HPDETECT_INSERT_INTERVAL);
	} else
		schedule_delayed_work(&ctx->jack_work_remove,
					HPDETECT_POLL_INTERVAL);
#ifdef CONFIG_HAS_WAKELOCK
	/*
	 * Take wakelock for one second to give time for the detection
	 * to finish. Jack detection is happening rarely so this doesn't
	 * have big impact to power consumption.
	 */
	wake_lock_timeout(ctx->jack_wake_lock,
			HPDETECT_INSERT_INTERVAL + msecs_to_jiffies(50));
#endif

	/* Report old status */
	return jack->status;
}

/* Jack insert delayed work */
void headset_insert_poll(struct work_struct *work)
{
	struct snd_soc_jack_gpio *gpio = &hs_gpio[CTP_HSDET_GPIO];
	struct snd_soc_jack *jack = gpio->jack;
	struct snd_soc_codec *codec = jack->codec;
	struct ctp_mc_private *ctx =
		container_of(jack, struct ctp_mc_private, ctp_jack);

	int enable, status;
	unsigned int mask = SND_JACK_HEADSET;

	enable = gpio_get_value(gpio->gpio);
	if (enable != 0) {
		pr_err("%s:gpio status = 0x%d\n", __func__, enable);
		return;
	}

	pr_debug("%s: Current jack status = 0x%x\n", __func__, jack->status);
	set_mic_bias(jack, "MIC2 Bias", true);
	msleep(ctx->ops->micsdet_debounce);
	status = ctx->ops->hp_detection(codec, jack, enable);
	if (status == SND_JACK_HEADSET) {
		set_bp_interrupt(ctx, true);
		ctx->headset_plug_flag = true;
	}
	if (jack->status != status)
		snd_soc_jack_report(jack, status, mask);

	/*
	 * At this point the HS may be half inserted and still be
	 * detected as HP, so recheck after HPDETECT_POLL_INTERVAL
	 */
#if 0
	if (!atomic_dec_and_test(&ctx->hs_det_retry) &&
			status != SND_JACK_HEADSET) {
		pr_debug("HS Jack detect Retry %d\n",
				atomic_read(&ctx->hs_det_retry));
#ifdef CONFIG_HAS_WAKELOCK
		/* Give sufficient time for the detection to propagate*/
		wake_lock_timeout(ctx->jack_wake_lock,
				HPDETECT_POLL_INTERVAL + msecs_to_jiffies(50));
#endif
		schedule_delayed_work(&ctx->jack_work_insert,
					HPDETECT_POLL_INTERVAL);
	}
#endif
    if (status == SND_JACK_HEADPHONE) {
        set_mic_bias(jack, "MIC2 Bias", false);
    }

	pr_debug("%s: status 0x%x\n", __func__, status);
}

/* Jack remove delayed work */
void headset_remove_poll(struct work_struct *work)
{
	struct snd_soc_jack_gpio *gpio = &hs_gpio[CTP_HSDET_GPIO];
	struct snd_soc_jack *jack = gpio->jack;
	struct snd_soc_codec *codec = jack->codec;
	struct ctp_mc_private *ctx =
		container_of(jack, struct ctp_mc_private, ctp_jack);

	int enable, status;
	unsigned int mask = SND_JACK_HEADSET;

	enable = gpio_get_value(gpio->gpio);
	if (enable == 0) {
		pr_err("%s:gpio status = 0x%d\n", __func__, enable);
		return;
	}

	pr_debug("%s: Current jack status = 0x%x\n", __func__, jack->status);
	status = ctx->ops->hp_detection(codec, jack, enable);
	set_bp_interrupt(ctx, false);
	ctx->headset_plug_flag = false;
	set_mic_bias(jack, "MIC2 Bias", false);

	if (jack->status != status)
		snd_soc_jack_report(jack, status, mask);
	pr_debug("%s: status 0x%x\n", __func__, status);
}

int ctp_soc_jack_gpio_detect_bp(void)
{
	struct snd_soc_jack_gpio *gpio = &hs_gpio[CTP_BTN_GPIO];
	int enable, hs_status, status;
	struct snd_soc_jack *jack = gpio->jack;
	struct snd_soc_codec *codec = jack->codec;
	unsigned int mask = SND_JACK_BTN_0 | SND_JACK_HEADSET;
	struct ctp_mc_private *ctx =
		container_of(jack, struct ctp_mc_private, ctp_jack);

	status = 0;
	enable = gpio_get_value(gpio->gpio);
	if (gpio->invert)
		enable = !enable;
	pr_err("%s: gpio->%d=0x%x\n", __func__, gpio->gpio, enable);

	/* Check for headset status before processing interrupt */
	gpio = &hs_gpio[CTP_HSDET_GPIO];
	hs_status = gpio_get_value(gpio->gpio);
	if (gpio->invert)
		hs_status = !hs_status;
	pr_err("%s: gpio->%d=0x%x\n", __func__, gpio->gpio, hs_status);
	pr_err("Jack status = %x\n", jack->status);
	if (((jack->status & SND_JACK_HEADSET) == SND_JACK_HEADSET)
						&& (!hs_status)) {
		/* HS present, process the interrupt */
		if (!enable) {
			/* Jack removal might be in progress, check interrupt status
			 * before proceeding for button press detection */
			if (!atomic_dec_return(&ctx->bpirq_flag)) {
				status = ctx->ops->bp_detection(codec, jack, enable);
				if (status == mask) {
					ctx->btn_press_flag = true;
				} else {
					if (!(ctx->btn_press_flag))
						snd_soc_jack_report(jack, mask, mask);
					ctx->btn_press_flag = false;
				}
				atomic_inc(&ctx->bpirq_flag);
			} else
				atomic_inc(&ctx->bpirq_flag);
		} else {
			status = jack->status;
			pr_err("%s:Invalid BP interrupt\n", __func__);
		}
	} else {
		pr_err("%s:Spurious BP interrupt : jack_status 0x%x, HS_status 0x%x\n",
				__func__, jack->status, hs_status);
		set_mic_bias(jack, "MIC2 Bias", false);
		/* Disable Button_press interrupt if no Headset */
		set_bp_interrupt(ctx, false);
	}
	pr_err("%s: status 0x%x\n", __func__, status);

	return status;
}


#ifdef CONFIG_PM

static int snd_ctp_prepare(struct device *dev)
{
	struct snd_soc_card *card = dev_get_drvdata(dev);
	struct ctp_mc_private *ctx = snd_soc_card_get_drvdata(card);
	pr_debug("In %s device name\n", __func__);

	return snd_soc_suspend(dev);
}
static void snd_ctp_complete(struct device *dev)
{
	struct snd_soc_card *card = dev_get_drvdata(dev);
	struct ctp_mc_private *ctx = snd_soc_card_get_drvdata(card);
	pr_debug("In %s\n", __func__);

	snd_soc_resume(dev);
}

static int snd_ctp_poweroff(struct device *dev)
{
	pr_debug("In %s\n", __func__);
	return snd_soc_poweroff(dev);
}

#else
#define snd_ctp_suspend NULL
#define snd_ctp_resume NULL
#define snd_ctp_poweroff NULL
#endif

static void free_jack_wake_lock(struct ctp_mc_private *ctx)
{
	if (!ctx->ops->jack_support)
		return;
#ifdef CONFIG_HAS_WAKELOCK
	if (wake_lock_active(ctx->jack_wake_lock))
		wake_unlock(ctx->jack_wake_lock);
	wake_lock_destroy(ctx->jack_wake_lock);
#endif
}

static void snd_ctp_unregister_jack(struct ctp_mc_private *ctx,
				struct platform_device *pdev)
{
	if (!ctx->ops->jack_support)
		return;
	cancel_delayed_work_sync(&ctx->jack_work_insert);
	cancel_delayed_work_sync(&ctx->jack_work_remove);
	free_jack_wake_lock(ctx);
	snd_soc_jack_free_gpios(&ctx->ctp_jack, 2, ctx->hs_gpio_ops);
}

static int __devexit snd_ctp_mc_remove(struct platform_device *pdev)
{
	struct snd_soc_card *soc_card = platform_get_drvdata(pdev);
	struct ctp_mc_private *ctx = snd_soc_card_get_drvdata(soc_card);

	pr_debug("In %s\n", __func__);
	snd_ctp_unregister_jack(ctx, pdev);
	snd_soc_card_set_drvdata(soc_card, NULL);
	snd_soc_unregister_card(soc_card);
	platform_set_drvdata(pdev, NULL);
	return 0;
}

static int snd_ctp_jack_init(struct snd_soc_pcm_runtime *runtime,
						bool jack_supported)
{
	int ret, irq;
	struct ctp_mc_private *ctx = snd_soc_card_get_drvdata(runtime->card);
	struct snd_soc_jack_gpio *gpio = &hs_gpio[CTP_BTN_GPIO];
	struct snd_soc_codec *codec = runtime->codec;
	pr_err("chaofubang:%s\n", __func__);

	if (!jack_supported)
		return 0;

	/* Setup the HPDET timer */
	INIT_DELAYED_WORK(&ctx->jack_work_insert, headset_insert_poll);
	INIT_DELAYED_WORK(&ctx->jack_work_remove, headset_remove_poll);

	/* Headset and button jack detection */
	ret = snd_soc_jack_new(codec, "Intel MID Audio Jack",
			SND_JACK_HEADSET | SND_JACK_BTN_0, &ctx->ctp_jack);
	if (ret) {
		pr_err("jack creation failed\n");
		return ret;
	}
	ret = snd_soc_jack_add_gpios(&ctx->ctp_jack, 2, ctx->hs_gpio_ops);
	if (ret) {
		pr_err("adding jack GPIO failed\n");
		return ret;
	}
	irq = gpio_to_irq(gpio->gpio);
	if (irq < 0) {
		pr_err("%d:Failed to map gpio_to_irq\n", irq);
		return irq;
	}

	/* Disable Button_press interrupt if no Headset */
	pr_err("Disable %d interrupt line\n", irq);
	disable_irq_nosync(irq);
	atomic_set(&ctx->bpirq_flag, 0);
	atomic_set(&ctx->hs_det_retry, HS_DET_RETRY);
	return 0;
}


int snd_ctp_init(struct snd_soc_pcm_runtime *runtime)
{
	int ret;
	struct ctp_mc_private *ctx = snd_soc_card_get_drvdata(runtime->card);

	ret = ctx->ops->ctp_init(runtime);
	if (ret) {
		pr_err("CTP init returned failure\n");
		return ret;
	}
	return snd_ctp_jack_init(runtime, ctx->ops->jack_support);
}

int snd_ctp_register_jack_data(struct platform_device *pdev,
					struct ctp_mc_private *ctx)
{
	struct ctp_audio_platform_data *pdata = pdev->dev.platform_data;
	int ret_val = 0;
	if (!ctx->ops->jack_support)
		return 0;
#ifdef CONFIG_HAS_WAKELOCK
	ctx->jack_wake_lock =
		devm_kzalloc(&pdev->dev, sizeof(*(ctx->jack_wake_lock)), GFP_ATOMIC);
	if (!ctx->jack_wake_lock) {
		pr_err("allocation failed for wake_lock\n");
		return -ENOMEM;
	}
	wake_lock_init(ctx->jack_wake_lock, WAKE_LOCK_SUSPEND,
			"jack_detect");
#endif
	if (pdata->codec_gpio_hsdet >= 0 && pdata->codec_gpio_button >= 0) {
		hs_gpio[CTP_HSDET_GPIO].gpio = pdata->codec_gpio_hsdet;
		hs_gpio[CTP_BTN_GPIO].gpio = pdata->codec_gpio_button;
		ret_val = gpio_to_irq(hs_gpio[CTP_BTN_GPIO].gpio);
		if (ret_val < 0) {
			pr_err("%d:Failed to map button irq\n", ret_val);
			return ret_val;
		}
		ctx->bpirq = ret_val;
	}
	ctx->hs_gpio_ops = hs_gpio;
	return 0;
}
/* SoC card */
static int snd_ctp_mc_probe(struct platform_device *pdev)
{
	int ret_val = 0;
	struct ctp_mc_private *ctx;

	pr_debug("In %s\n", __func__);
	ctx = devm_kzalloc(&pdev->dev, sizeof(*ctx), GFP_ATOMIC);
	if (!ctx) {
		pr_err("allocation failed\n");
		return -ENOMEM;
	}
	/* register the soc card */
	snd_soc_card_ctp.dev = &pdev->dev;

	ctx->ops = (struct snd_soc_machine_ops *)platform_get_device_id(pdev)->driver_data;
	if (ctx->ops == NULL)
		return -EINVAL;

	ctx->ops->dai_link(&snd_soc_card_ctp);

	ret_val = snd_ctp_register_jack_data(pdev, ctx);
	if (ret_val)
		goto free_jack;

	snd_soc_card_set_drvdata(&snd_soc_card_ctp, ctx);
	ret_val = snd_soc_register_card(&snd_soc_card_ctp);
	if (ret_val) {
		pr_err("snd_soc_register_card failed %d\n", ret_val);
		goto free_jack;
	}

	platform_set_drvdata(pdev, &snd_soc_card_ctp);
	pr_debug("successfully exited probe\n");
	return ret_val;
free_jack:
	free_jack_wake_lock(ctx);
	return ret_val;
}

const struct dev_pm_ops snd_ctp_mc_pm_ops = {
	.prepare = snd_ctp_prepare,
	.complete = snd_ctp_complete,
	.poweroff = snd_ctp_poweroff,
};

static struct platform_device_id ctp_audio_ids[] = {
	{
		.name		= "ctp_rhb_cs42l73",
		.driver_data	= (kernel_ulong_t)&ctp_rhb_cs42l73_ops,
	},
	{
		.name		= "ctp_vb_cs42l73",
		.driver_data	= (kernel_ulong_t)&ctp_vb_cs42l73_ops,
	},
	{ },
};
MODULE_DEVICE_TABLE(platform, ctp_audio_ids);

static struct platform_driver snd_ctp_mc_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "ctp_audio",
		.pm   = &snd_ctp_mc_pm_ops,
	},
	.probe = snd_ctp_mc_probe,
	.remove = __devexit_p(snd_ctp_mc_remove),
	.id_table = ctp_audio_ids,
};

static int __init snd_ctp_driver_init(void)
{
	pr_info("In %s\n", __func__);
	return platform_driver_register(&snd_ctp_mc_driver);
}

static void snd_ctp_driver_exit(void)
{
	pr_debug("In %s\n", __func__);
	platform_driver_unregister(&snd_ctp_mc_driver);
}

static int snd_clv_rpmsg_probe(struct rpmsg_channel *rpdev)
{
	int ret = 0;

	if (rpdev == NULL) {
		pr_err("rpmsg channel not created\n");
		ret = -ENODEV;
		goto out;
	}

	dev_info(&rpdev->dev, "Probed snd_clv rpmsg device\n");

	ret = snd_ctp_driver_init();

out:
	return ret;
}

static void snd_clv_rpmsg_remove(struct rpmsg_channel *rpdev)
{
	snd_ctp_driver_exit();
	dev_info(&rpdev->dev, "Removed snd_clv rpmsg device\n");
}

static void snd_clv_rpmsg_cb(struct rpmsg_channel *rpdev, void *data,
				int len, void *priv, u32 src)
{
	dev_warn(&rpdev->dev, "unexpected, message\n");

	print_hex_dump(KERN_DEBUG, __func__, DUMP_PREFIX_NONE, 16, 1,
			data, len,  true);
}

static struct rpmsg_device_id snd_clv_rpmsg_id_table[] = {
	{ .name = "rpmsg_msic_clv_audio" },
	{ },
};
MODULE_DEVICE_TABLE(rpmsg, snd_clv_rpmsg_id_table);

static struct rpmsg_driver snd_clv_rpmsg = {
	.drv.name	= KBUILD_MODNAME,
	.drv.owner	= THIS_MODULE,
	.id_table	= snd_clv_rpmsg_id_table,
	.probe		= snd_clv_rpmsg_probe,
	.callback	= snd_clv_rpmsg_cb,
	.remove		= snd_clv_rpmsg_remove,
};

static int __init snd_clv_rpmsg_init(void)
{
	return register_rpmsg_driver(&snd_clv_rpmsg);
}

late_initcall(snd_clv_rpmsg_init);

static void __exit snd_clv_rpmsg_exit(void)
{
	return unregister_rpmsg_driver(&snd_clv_rpmsg);
}
module_exit(snd_clv_rpmsg_exit);

