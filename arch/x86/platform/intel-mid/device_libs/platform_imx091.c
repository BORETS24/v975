/*
 * platform_imx091.c: imx091 platform data initilization file
 *
 * (C) Copyright 2012 Intel Corporation
 * Author:
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 * of the License.
 */

#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/atomisp_platform.h>
#include <asm/intel_scu_ipcutil.h>
#include <asm/intel-mid.h>
#include <media/v4l2-subdev.h>
#include <linux/regulator/consumer.h>
#include "platform_camera.h"
#include "platform_imx091.h"


static int camera_reset;
static int camera_vprog1_on;  
static int camera_v1p2_vcm_dvdd;
static int camera_v2p8_vcm_avdd;

static struct regulator *vprog1_reg;  
#define VPROG1_VAL 2800000  

/*
 * MRFLD VV primary camera sensor - IMX091 platform data
 */

static int imx091_gpio_ctrl(struct v4l2_subdev *sd, int flag)
{
	int ret;

	if (camera_reset < 0) {
		ret = camera_sensor_gpio(-1, GP_CAMERA_0_RESET,
					GPIOF_DIR_OUT, 1);
		if (ret < 0)
			return ret;
		camera_reset = ret;
	}

	if (flag) {
		gpio_set_value(camera_reset, 1);
		mdelay(5);

	} else {
		gpio_set_value(camera_reset, 0);
	}

	return 0;
}

static int imx091_flisclk_ctrl(struct v4l2_subdev *sd, int flag)
{
	static const unsigned int clock_khz = 19200;
	return intel_scu_ipc_osc_clk(OSC_CLK_CAM0, flag ? clock_khz : 0);
}

static int imx091_power_ctrl(struct v4l2_subdev *sd, int flag)
{

	int ret =0;

	if (camera_v2p8_vcm_avdd < 0) {
			ret = camera_sensor_gpio(-1, GP_CAMERA_POWER_2P8,
					GPIOF_DIR_OUT, 1);
			if (ret < 0) {
				printk(KERN_ALERT "Failed to initialize avdd 2.8v\n");
				return ret;
			}
			camera_v2p8_vcm_avdd = ret;
	}

	if (camera_v1p2_vcm_dvdd < 0) {
			ret = camera_sensor_gpio(-1, GP_CAMERA_0_1P2,
					GPIOF_DIR_OUT, 1);
			if (ret < 0) {
				printk(KERN_ALERT "Failed to initialize dvdd 1.2v\n");
				return ret;
			}
			camera_v1p2_vcm_dvdd = ret;
	}
	
	if (flag) {
		printk("imx091 power on\n");
		if (!camera_vprog1_on) {
			if (intel_mid_identify_cpu() == 
				INTEL_MID_CPU_CHIP_TANGIER)
				ret = intel_scu_ipc_msic_vprog1(1);
			else
				ret = regulator_enable(vprog1_reg);
			if(!ret)
				camera_vprog1_on = 1;
			else {
				printk(KERN_ALERT "Failed to enable regulator vprog1\n");
				return ret;
			}
		}
		gpio_set_value(camera_v2p8_vcm_avdd, 1);
		gpio_set_value(camera_v1p2_vcm_dvdd, 1);
		
		/* min 500us -Initializing time of silicon */
		usleep_range(500, 600);
	} else {
		printk("imx091 power off\n");
		if (camera_vprog1_on) {
			if (intel_mid_identify_cpu() ==
			   INTEL_MID_CPU_CHIP_TANGIER)
				ret = intel_scu_ipc_msic_vprog1(0);
			else
				ret = regulator_disable(vprog1_reg);

			if (!ret)
				camera_vprog1_on = 0;
			else
				printk(KERN_ALERT "Failed to disable regulator vprog1\n");
		}
		gpio_set_value(camera_v1p2_vcm_dvdd, 0);
		gpio_set_value(camera_v2p8_vcm_avdd, 0);
		gpio_free(camera_v2p8_vcm_avdd);
		gpio_free(camera_v1p2_vcm_dvdd);
		camera_v2p8_vcm_avdd = -1;
		camera_v1p2_vcm_dvdd = -1 ;
	}
	
	return 0;
}

static int imx091_platform_init(struct i2c_client *client)
{
	int ret;
	
	vprog1_reg = regulator_get(&client->dev, "vprog1");
	if (IS_ERR(vprog1_reg)) {
		dev_err(&client->dev, "regulator_get failed\n");
		return PTR_ERR(vprog1_reg);
	}
	ret = regulator_set_voltage(vprog1_reg, VPROG1_VAL, VPROG1_VAL);
	if (ret) {
		dev_err(&client->dev, "regulator voltage set failed\n");
		regulator_put(vprog1_reg);
	}
	
	return ret;
}

static int imx091_platform_deinit(void)
{
	regulator_put(vprog1_reg);
	return 0;
}

static int imx091_csi_configure(struct v4l2_subdev *sd, int flag)
{
	static const int LANES = 4;
	return camera_sensor_csi(sd, ATOMISP_CAMERA_PORT_PRIMARY, LANES,
		-1, 0, flag);
}

static struct camera_sensor_platform_data imx091_sensor_platform_data = {
	.gpio_ctrl      = imx091_gpio_ctrl,
	.flisclk_ctrl   = imx091_flisclk_ctrl,
	.power_ctrl     = imx091_power_ctrl,
	.csi_cfg        = imx091_csi_configure,
	.platform_init = imx091_platform_init,
	.platform_deinit = imx091_platform_deinit,
};

void *imx091_platform_data(void *info)
{
	camera_reset = -1;
	camera_vprog1_on = 0;
	camera_v1p2_vcm_dvdd = -1;
	camera_v2p8_vcm_avdd = -1;
	
	return &imx091_sensor_platform_data;
}

