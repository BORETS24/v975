/*
 * platform_sp1628.c: sp1628 platform data initilization file
 *
 * (C) Copyright 2008 Intel Corporation
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
#include "platform_sp1628.h"

static int camera_powerdown;
static int camera_reset;
static int camera_power_down;
static int camera_vprog1_on;
/* ZTE: add by yaoling for power control of sp1628 start */
static int camera_v2p8_vcm_vdd;
static int camera_v1p8_vcm_vddio;
static int camera_v1p5_vcm_dvdd;
/* ZTE: add by yaoling for power control of sp1628 end  */
static struct regulator *vprog1_reg;
#define VPROG1_VAL 2800000
/*
 * MRFLD VV secondary camera sensor - sp1628 platform data
 */

static int sp1628_gpio_ctrl(struct v4l2_subdev *sd, int flag)
{
	int ret;
      #if 1
	if (camera_reset < 0) {
		ret = camera_sensor_gpio(-1, GP_CAMERA_1_RESET,
					GPIOF_DIR_OUT, 1);
		if (ret < 0)
			return ret;
		camera_reset = ret;
	}
	if(camera_powerdown<0){
		ret = camera_sensor_gpio(-1, GP_CAMERA_0_STANDBY,
					GPIOF_DIR_OUT, 1);
		if (ret < 0)
			return ret;
		camera_powerdown = ret;
		}

	if (flag) {
		gpio_set_value(camera_powerdown, 0);
		msleep(50);
		gpio_set_value(camera_powerdown, 1);
		msleep(5);
		gpio_set_value(camera_powerdown, 0);
		msleep(2);
		gpio_set_value(camera_reset, 1);
		/* sp1628 initializing time - t1+t2
		 * 427us(t1) - 8192 mclk(19.2Mhz) before sccb communication
		 * 1ms(t2) - sccb stable time when using internal dvdd
		 */
		usleep_range(1500, 1500);
	} else {
	      gpio_set_value(camera_powerdown, 0);
		gpio_set_value(camera_reset, 0);
		gpio_free(camera_powerdown);
		gpio_free(camera_reset);
		camera_reset = -1 ;
		camera_powerdown = -1 ;
		
	}
	#endif
	return 0;
}

static int sp1628_flisclk_ctrl(struct v4l2_subdev *sd, int flag)
{
	static const unsigned int clock_khz = 19200;
	printk(KERN_ALERT "enter sp1628_flisclk_ctrl\n"); //
	return intel_scu_ipc_osc_clk(OSC_CLK_CAM1, flag ? clock_khz : 0);
}
static struct regulator *vprog1_reg;
static int sp1628_power_ctrl(struct v4l2_subdev *sd, int flag)
{
	#ifdef CONFIG_BOARD_CTP
	int reg_err;
	#endif
	int ret = 0;
	printk(KERN_ALERT "enter sp1628_power_ctrl\n");
 	if (camera_v2p8_vcm_vdd < 0) {
		 printk(KERN_ALERT "enter sp1628_power_ctrl after if (camera_v2p8_vcm_vdd < 0)\n");
		
		ret = camera_sensor_gpio(-1, GP_CAMERA_POWER_2P8,
					GPIOF_DIR_OUT, 1);
		if (ret < 0)
			{
			return ret;
			}
		camera_v2p8_vcm_vdd = ret;
	} 
	if (camera_v1p8_vcm_vddio < 0) {
		ret = camera_sensor_gpio(-1, GP_CAMERA_1_1P8,
					GPIOF_DIR_OUT, 1);
		if (ret < 0)
			return ret;
		camera_v1p8_vcm_vddio = ret;
	}
	if (camera_v1p5_vcm_dvdd < 0) {
		ret = camera_sensor_gpio(-1, GP_CAMERA_1_1P5,
					GPIOF_DIR_OUT, 1);
		if (ret < 0)
			return ret;
		camera_v1p5_vcm_dvdd = ret;
	}
	#if 0
	if(camera_powerdown<0){
		ret = camera_sensor_gpio(-1, GP_CAMERA_0_STANDBY,
					GPIOF_DIR_OUT, 1);
		if (ret < 0)
			return ret;
		camera_powerdown = ret;
		}
	#endif
		
	if (flag) {
		if (!camera_vprog1_on) {
			camera_vprog1_on = 1;
		#ifdef CONFIG_BOARD_CTP
			reg_err = regulator_enable(vprog1_reg);
			if (reg_err) {
				printk(KERN_ALERT "Failed to enable regulator vprog1\n");
				return reg_err;
		#else
			intel_scu_ipc_msic_vprog1(0);
		#endif
			}
		}
		 printk(KERN_ALERT "enter sp1628_power_ctrl before  gpio_set_value(camera_v2p8_vcm_vdd, 1);\n");
		gpio_set_value(camera_v2p8_vcm_vdd, 1);
		gpio_set_value(camera_v1p8_vcm_vddio, 1);
		gpio_set_value(camera_v1p5_vcm_dvdd, 1);
		//msleep(5);
		#if 0
		gpio_set_value(camera_powerdown, 0);
		msleep(1);
		gpio_set_value(camera_powerdown, 1);
		msleep(1);
		gpio_set_value(camera_powerdown, 0);
		#endif
		msleep(10);
		
	} else {
		if (camera_vprog1_on) {
			camera_vprog1_on = 0;
			#ifdef CONFIG_BOARD_CTP
			reg_err = regulator_disable(vprog1_reg);
			if (reg_err) {
				printk(KERN_ALERT "Failed to disable regulator vprog1\n");
				return reg_err;
			}
			#else
			intel_scu_ipc_msic_vprog1(1);

			#endif
		}
		gpio_set_value(camera_v1p8_vcm_vddio, 0);
		 printk(KERN_ALERT "enter sp1628_power_ctrl before  gpio_set_value(camera_v2p8_vcm_vdd, 0);\n");
		gpio_set_value(camera_v2p8_vcm_vdd, 0);
		gpio_set_value(camera_v1p5_vcm_dvdd, 0);
		//gpio_set_value(camera_powerdown, 0);
		 printk(KERN_ALERT "enter sp1628_power_ctrl camera_v2p8_vcm_vdd freee\n");
		gpio_free(camera_v2p8_vcm_vdd);
		gpio_free(camera_v1p8_vcm_vddio);
		gpio_free(camera_v1p5_vcm_dvdd);
		//gpio_free(camera_powerdown);
		camera_v2p8_vcm_vdd = -1 ;
		camera_v1p8_vcm_vddio = -1 ;
		camera_v1p5_vcm_dvdd = -1 ;
		//camera_powerdown = -1 ;	
		
	}
	return 0;
}
static int sp1628_platform_init(struct i2c_client *client)
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

static int sp1628_platform_deinit(void)
{
	regulator_put(vprog1_reg);
}
static int sp1628_csi_configure(struct v4l2_subdev *sd, int flag)
{
	static const int LANES = 1;
	return camera_sensor_csi(sd, ATOMISP_CAMERA_PORT_SECONDARY, 1,
		ATOMISP_INPUT_FORMAT_YUV422_8, -1, flag);
	//return camera_sensor_csi(sd, ATOMISP_CAMERA_PORT_SECONDARY, LANES,
		//ATOMISP_INPUT_FORMAT_RAW_10, atomisp_bayer_order_bggr, flag);
}

static struct camera_sensor_platform_data sp1628_sensor_platform_data = {
	.gpio_ctrl      = sp1628_gpio_ctrl,
	.flisclk_ctrl   = sp1628_flisclk_ctrl,
	.power_ctrl     = sp1628_power_ctrl,
	.csi_cfg        = sp1628_csi_configure,
	#ifdef CONFIG_BOARD_CTP
	.platform_init = sp1628_platform_init,
	.platform_deinit = sp1628_platform_deinit,
	#endif
};

void *sp1628_platform_data(void *info)
{
	camera_reset = -1;
	camera_power_down = -1;
	camera_v2p8_vcm_vdd = -1;
	camera_v1p8_vcm_vddio = -1;
	camera_v1p5_vcm_dvdd = -1 ;
	camera_powerdown = -1;

	return &sp1628_sensor_platform_data;
}

