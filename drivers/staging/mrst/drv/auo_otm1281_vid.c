/*
 * Copyright Â© 2010 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 * Jim Liu <jim.liu@intel.com>
 * Jackie Li <yaodong.li@intel.com>
 */
#include "mdfld_dsi_dpi.h"
#include "mdfld_dsi_pkg_sender.h"
#include <linux/gpio.h>
#include <linux/sfi.h>

#include <asm/intel_scu_pmic.h>

#include "zte_pannel_common.h"

static u8 orise_mode_enable_step1[]     = {0xff, 0x12, 0x80, 0x01};
static u8 orise_mode_enable_step3[]     = {0xff, 0x12, 0x80};
static u8 otm1281_power_setting_1[]     = {
	0xc5, 0x10, 0x6F, 0x02, 0x88, 0x1D, 0x15, 0x00, 0x04
};
static u8 otm1281_power_setting_2[]     = {
	0xc5, 0x20, 0x01, 0x00, 0xb0, 0xb0, 0x00, 0x04, 0x00
};
static u8 otm1281_power_setting_3[]     = {0xd8, 0x58, 0x00, 0x58, 0x00};
static u8 otm1281_power_setting_4[]     = {0xf5, 0x0c, 0x12};

#define PANEL_NAME "Lead/Success AUO"

static int mdfld_otm1281_dpi_ic_init(struct mdfld_dsi_config *dsi_config)
{
	struct mdfld_dsi_pkg_sender *sender
			= mdfld_dsi_get_pkg_sender(dsi_config);

	if (!sender) {
		DRM_ERROR("Cannot get sender\n");
		return -EINVAL;
	}

	PSB_DEBUG_ENTRY("\n");
	sender->status = MDFLD_DSI_PKG_SENDER_FREE;

	mdfld_dsi_send_mcs_long_lp(sender, orise_mode_enable_step1, 4, 0);
	if (sender->status == MDFLD_DSI_CONTROL_ABNORMAL)
		DRM_ERROR("command orise_mode_enable_step1 error\n");

	/* orise_mode_enable_step2 */
	mdfld_dsi_send_mcs_short_lp(sender, 0x00, 0x80, 1, 0);

	mdfld_dsi_send_mcs_long_lp(sender, orise_mode_enable_step3, 3, 0);
	if (sender->status == MDFLD_DSI_CONTROL_ABNORMAL)
		DRM_ERROR("command orise_mode_enable_step3 error\n");

	/* 0xb0d0 = 0x20 enable EOTP after the transmission */
	mdfld_dsi_send_mcs_short_lp(sender, 0x00, 0xd0, 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, 0xb0, 0x20, 1, 0);

	/* vertical refresh */
	mdfld_dsi_send_mcs_short_lp(sender, 0x36, 0xd0, 1, 0);

	/* increase oscillator frequency to 70hz */
	mdfld_dsi_send_mcs_short_lp(sender, 0x00, 0x82, 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, 0xC1, 0x09, 1, 0);

	/*-------------------- power setting --------------------*/
	mdfld_dsi_send_mcs_short_lp(sender, 0x00, 0x90, 1, 0);
	mdfld_dsi_send_mcs_long_lp(sender, otm1281_power_setting_1, 9, 0);
	if (sender->status == MDFLD_DSI_CONTROL_ABNORMAL)
		DRM_ERROR("command otm1281_power_setting_1 error\n");

	mdfld_dsi_send_mcs_short_lp(sender, 0x00, 0xa0, 1, 0);
	mdfld_dsi_send_mcs_long_lp(sender, otm1281_power_setting_1, 9, 0);
	if (sender->status == MDFLD_DSI_CONTROL_ABNORMAL)
		DRM_ERROR("command otm1281_power_setting_1 error\n");

	mdfld_dsi_send_mcs_short_lp(sender, 0x00, 0x80, 1, 0);
	mdfld_dsi_send_mcs_long_lp(sender, otm1281_power_setting_2, 9, 0);
	if (sender->status == MDFLD_DSI_CONTROL_ABNORMAL)
		DRM_ERROR("command otm1281_power_setting_2 error\n");

	mdfld_dsi_send_mcs_short_lp(sender, 0x00, 0x00, 1, 0);
	mdfld_dsi_send_mcs_long_lp(sender, otm1281_power_setting_3, 5, 0);
	if (sender->status == MDFLD_DSI_CONTROL_ABNORMAL)
		DRM_ERROR("command otm1281_power_setting_3 error\n");

	mdfld_dsi_send_mcs_short_lp(sender, 0x00, 0xb8, 1, 0);
	mdfld_dsi_send_mcs_long_lp(sender, otm1281_power_setting_4, 3, 0);
	if (sender->status == MDFLD_DSI_CONTROL_ABNORMAL)
		DRM_ERROR("command otm1281_power_setting_4 error\n");

#ifdef CONFIG_BL_LCD_PWM_CONTROL
	mdfld_dsi_send_mcs_short_lp(sender, 0x00, 0xB1, 1, 0);
       mdfld_dsi_send_mcs_short_lp(sender, 0xC6, 0x03, 1, 0);
       mdfld_dsi_send_mcs_short_lp(sender, 0x55, 0x00, 1, 0);
       mdfld_dsi_send_mcs_short_lp(sender, 0x53, 0x24, 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, 0x51, 0x00, 1, 0);
#endif

	return 0;
}

static
void mdfld_otm1281_dpi_controller_init(struct mdfld_dsi_config *dsi_config)
{
	struct mdfld_dsi_hw_context *hw_ctx =
		&dsi_config->dsi_hw_context;

	PSB_DEBUG_ENTRY("\n");

	/*reconfig lane configuration*/
	dsi_config->lane_count = 4;
	dsi_config->lane_config = MDFLD_DSI_DATA_LANE_4_0;
	/* This is for 400 mhz.  Set it to 0 for 800mhz */
	hw_ctx->cck_div = 1;
	hw_ctx->pll_bypass_mode = 0;

	hw_ctx->mipi_control = 0x18;
	hw_ctx->intr_en = 0xffffffff;
	hw_ctx->hs_tx_timeout = 0xffffff;
	hw_ctx->lp_rx_timeout = 0xffffff;
	hw_ctx->turn_around_timeout = 0x1f;
	hw_ctx->device_reset_timer = 0xffff;

	hw_ctx->init_count = 0xf0;
	hw_ctx->eot_disable = 0x2;

	/* b060 */
	hw_ctx->lp_byteclk = 0x3;
	/* B044 */
	hw_ctx->high_low_switch_count = 0x16;
	/* b088 */
	hw_ctx->clk_lane_switch_time_cnt = 0x0016000A;
	/* b080 */
	hw_ctx->dphy_param = 0x150C340F;

	/*setup video mode format*/
	hw_ctx->video_mode_format = 0xf;

	/*set up func_prg*/
	hw_ctx->dsi_func_prg = (0x200 | dsi_config->lane_count);
	/*setup mipi port configuration*/
	hw_ctx->mipi = PASS_FROM_SPHY_TO_AFE | dsi_config->lane_config;
}

static
int mdfld_dsi_otm1281_detect(struct mdfld_dsi_config *dsi_config)
{
	int status;
	struct drm_device *dev = dsi_config->dev;
	struct mdfld_dsi_hw_registers *regs = &dsi_config->regs;
	u32 dpll_val, device_ready_val;
	int pipe = dsi_config->pipe;

	PSB_DEBUG_ENTRY("\n");

	if (pipe == 0) {
		/*
		 * FIXME: WA to detect the panel connection status, and need to
		 * implement detection feature with get_power_mode DSI command.
		 */
		if (!ospm_power_using_hw_begin(OSPM_DISPLAY_ISLAND,
					OSPM_UHB_FORCE_POWER_ON)) {
			DRM_ERROR("hw begin failed\n");
			return -EAGAIN;
		}

		dpll_val = REG_READ(regs->dpll_reg);
		device_ready_val = REG_READ(regs->device_ready_reg);
		if ((device_ready_val & DSI_DEVICE_READY) &&
		    (dpll_val & DPLL_VCO_ENABLE)) {
			dsi_config->dsi_hw_context.panel_on = true;
		} else {
			dsi_config->dsi_hw_context.panel_on = false;
			DRM_INFO("%s: panel is not detected!\n", __func__);
		}

		status = MDFLD_DSI_PANEL_CONNECTED;

		ospm_power_using_hw_end(OSPM_DISPLAY_ISLAND);
	} else {
		DRM_INFO("%s: do NOT support dual panel\n", __func__);
		status = MDFLD_DSI_PANEL_DISCONNECTED;
	}

	return status;
}

static int mdfld_dsi_otm1281_power_on(struct mdfld_dsi_config *dsi_config)
{
	struct mdfld_dsi_pkg_sender *sender =
		mdfld_dsi_get_pkg_sender(dsi_config);
	int err;

	PSB_DEBUG_ENTRY("\n");

	if (!sender) {
		DRM_ERROR("Failed to get DSI packet sender\n");
		return -EINVAL;
	}

	/*exit sleep */
	err = mdfld_dsi_send_mcs_short_lp(sender, 0x11, 0, 0, 0);
	if (err) {
		DRM_ERROR("faild to exit_sleep mode\n");
		goto power_err;
	}

	msleep(200);

	/*turn on display*/
	err = mdfld_dsi_send_mcs_short_lp(sender, 0x29, 0, 0, 0);

	if (err) {
		DRM_ERROR("faild to set_display_on mode\n");
		goto power_err;
	}
	msleep(50);


	/*send TURN_ON packet*/
	err = mdfld_dsi_send_dpi_spk_pkg_hs(sender,
				MDFLD_DSI_DPI_SPK_TURN_ON);
	if (err) {
		DRM_ERROR("Failed to send turn on packet\n");
		return err;
	}
power_err:
	return 0;
}

static int mdfld_dsi_otm1281_power_off(struct mdfld_dsi_config *dsi_config)
{
	struct mdfld_dsi_pkg_sender *sender =
		mdfld_dsi_get_pkg_sender(dsi_config);
	int err;

	PSB_DEBUG_ENTRY("\n");

	if (!sender) {
		DRM_ERROR("Failed to get DSI packet sender\n");
		return -EINVAL;
	}

	/*send SHUT_DOWN packet*/
	err = mdfld_dsi_send_dpi_spk_pkg_hs(sender,
				MDFLD_DSI_DPI_SPK_SHUT_DOWN);
	if (err) {
		DRM_ERROR("Failed to send turn off packet\n");
		return err;
	}

	/*turn off display */
	err = mdfld_dsi_send_mcs_short_lp(sender, 0x28, 0, 0, 0);
	if (err) {
		DRM_ERROR("sent set_display_off faild\n");
		goto out;
	}
	mdelay(50);

	/*Enter sleep mode */
	err = mdfld_dsi_send_mcs_short_lp(sender, 0x10, 0, 0, 0);

	if (err) {
		DRM_ERROR("DCS 0x%x sent failed\n", enter_sleep_mode);
		goto out;
	}
	mdelay(200);
out:
	return 0;
}

int mdfld_dsi_otm1281_set_brightness(struct mdfld_dsi_config *dsi_config,
					int level)
{
	struct mdfld_dsi_pkg_sender *sender =
		mdfld_dsi_get_pkg_sender(dsi_config);
	u8 backlight_value = 0;

	if (!sender) {
		DRM_ERROR("Failed to get DSI packet sender\n");
		return -EINVAL;
	}

	PSB_DEBUG_ENTRY("level=%d\n", level);
#ifdef CONFIG_BL_PMU_PWM_CONTROL
	if (level) {
		backlight_value = 35 + (level *65) /100;
		backlight_value = (backlight_value * 0x63) / 100;
	}
	intel_scu_ipc_iowrite8(0x67, backlight_value);
#endif

#ifdef CONFIG_BL_LCD_PWM_CONTROL
       if (level > 80) {
               backlight_value = 153 + ((level-80) * 102) /20;
       } else {
               backlight_value = (level * 153) /80;
       }
	if (get_debug_flag()) {
		printk("[%s]brightness level:%d, pwm val:%d\n", __func__, level, backlight_value);
	}
	mdfld_dsi_send_mcs_short_lp(sender, 0x51, backlight_value, 1, 0);
#endif
	return 0;
}

static
int mdfld_dsi_otm1281_panel_reset(struct mdfld_dsi_config *dsi_config)
{
	static int mipi_reset_gpio;
	int ret = 0;

	if (mipi_reset_gpio == 0) {
		ret = get_gpio_by_name("mipi-reset");
		if (ret < 0) {
			DRM_ERROR("Faild to get panel reset gpio, " \
				  "use default reset pin\n");
			ret = 57;
		}

		mipi_reset_gpio = ret;

		PSB_DEBUG_ENTRY("mipi_reset_gpio = %d\n", mipi_reset_gpio);

		ret = gpio_request(mipi_reset_gpio, "mipi_display");
		if (ret) {
			DRM_ERROR("Faild to request panel reset gpio\n");
			return -EINVAL;
		}

		gpio_direction_output(mipi_reset_gpio, 0);
	}

	gpio_direction_output(mipi_reset_gpio, 1);
	mdelay(5);

	gpio_set_value_cansleep(mipi_reset_gpio, 0);
	mdelay(20);

	gpio_set_value_cansleep(mipi_reset_gpio, 1);
	mdelay(20);

	return 0;
}

static
struct drm_display_mode *otm1281_get_config_mode(void)
{
	struct drm_display_mode *mode;

	PSB_DEBUG_ENTRY("\n");

	mode = kzalloc(sizeof(*mode), GFP_KERNEL);
	if (!mode)
		return NULL;

	mode->hdisplay = 720;
	mode->hsync_start = mode->hdisplay + 20; /*HFP*/
	mode->hsync_end = mode->hsync_start + 2; /*HSYNC*/
	mode->htotal = mode->hsync_end + 36; /*HBP*/
	mode->vdisplay = 1280;
	mode->vsync_start = mode->vdisplay + 100; /*VFP*/
	mode->vsync_end = mode->vsync_start + 2; /*VSYNC*/
	mode->vtotal = mode->vsync_end + 20; /*VBP*/
	mode->vrefresh = 60;
	mode->clock =  mode->vrefresh * mode->vtotal *
		mode->htotal / 1000;

	mode->type |= DRM_MODE_TYPE_PREFERRED;

	PSB_DEBUG_ENTRY("hdisplay is %d\n", mode->hdisplay);
	PSB_DEBUG_ENTRY("vdisplay is %d\n", mode->vdisplay);
	PSB_DEBUG_ENTRY("HSS is %d\n", mode->hsync_start);
	PSB_DEBUG_ENTRY("HSE is %d\n", mode->hsync_end);
	PSB_DEBUG_ENTRY("htotal is %d\n", mode->htotal);
	PSB_DEBUG_ENTRY("VSS is %d\n", mode->vsync_start);
	PSB_DEBUG_ENTRY("VSE is %d\n", mode->vsync_end);
	PSB_DEBUG_ENTRY("vtotal is %d\n", mode->vtotal);
	PSB_DEBUG_ENTRY("clock is %d\n", mode->clock);

	drm_mode_set_name(mode);
	drm_mode_set_crtcinfo(mode, 0);

	return mode;
}

#define PWM0CLKDIV0   0x62
#define PWM0CLKDIV1   0x61
#define SYSTEMCLK        196608
#define PWM_FREQUENCY  40000

static inline u16 calc_clkdiv(unsigned long baseclk, unsigned int f)
{
	return (baseclk - f) / f;
}


static void otm1281_brightness_init(struct drm_device *dev)
{
	int ret;
	u16 clkdiv;

	/* Make sure the PWM reference is the 19.2 MHz system clock. Read first
	* * instead of setting directly to catch potential conflicts between PWM
	* * users. */
	clkdiv = calc_clkdiv(SYSTEMCLK, PWM_FREQUENCY);

	ret = intel_scu_ipc_iowrite8(PWM0CLKDIV1, (clkdiv >> 8) & 0xff);
	if (!ret)
		ret = intel_scu_ipc_iowrite8(PWM0CLKDIV0, clkdiv & 0xff);

	if (ret)
		dev_err(&dev->pdev->dev, "PWM0CLKDIV set failed\n");
	else
		dev_dbg(&dev->pdev->dev, "PWM0CLKDIV set to 0x%04x (%d Hz)\n",
			clkdiv, PWM_FREQUENCY);
}

static
void otm1281_get_panel_info(int pipe, struct panel_info *pi)
{
	pi->width_mm = PANEL_4DOT3_WIDTH;
	pi->height_mm = PANEL_4DOT3_HEIGHT;
}

void otm1281_vid_init(struct drm_device *dev, struct panel_funcs *p_funcs)
{
	PSB_DEBUG_ENTRY("\n");

	p_funcs->get_config_mode = otm1281_get_config_mode;
	p_funcs->get_panel_info = otm1281_get_panel_info;
	p_funcs->reset = mdfld_dsi_otm1281_panel_reset;
	p_funcs->drv_ic_init = mdfld_otm1281_dpi_ic_init;
	p_funcs->dsi_controller_init = mdfld_otm1281_dpi_controller_init;
	p_funcs->detect = mdfld_dsi_otm1281_detect;
	p_funcs->power_on = mdfld_dsi_otm1281_power_on;
	p_funcs->power_off = mdfld_dsi_otm1281_power_off;
	p_funcs->set_brightness = mdfld_dsi_otm1281_set_brightness;
#ifdef CONFIG_BL_PMU_PWM_CONTROL
	otm1281_brightness_init(dev);
#endif
}

static int otm1281_vid_probe(struct platform_device *pdev)
{
	DRM_INFO("%s: OTM1281 panel detected\n", __func__);
	intel_mid_panel_register(otm1281_vid_init);

	add_panel_config_prop(PANEL_NAME, "OTM1281", 1280, 720);
	create_backlight_debug_file();
	return 0;
}

static struct platform_driver otm1281_lcd_driver = {
	.probe	= otm1281_vid_probe,
	.driver	= {
		.name	= "OTM1281 VID RHB",
		.owner	= THIS_MODULE,
	},
};

static int __init otm1281_lcd_init(void)
{
	DRM_INFO("%s\n", __func__);

	return platform_driver_register(&otm1281_lcd_driver);
}
module_init(otm1281_lcd_init);

