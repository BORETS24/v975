/*
 * platform_bu64291.c: bu64291 platform data initilization file
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
#include <linux/init.h>
#include <linux/types.h>
#include <asm/intel-mid.h>

#include <media/bu64291.h>

#include "platform_bu64291.h"
#include "platform_camera.h"
static int camera_af_power = -1;
#if 0
static int bu64291_gpio_ctrl(struct v4l2_subdev *sd, int flag)
{
	int ret;

	if (camera_af_power_gpio < 0) {
		ret = camera_sensor_gpio(-1, GP_CAMERA_1_RESET,
					GPIOF_DIR_OUT, 1);
		if (ret < 0)
			return ret;
		camera_af_power_gpio = ret;
	}

	if (flag) {
		gpio_set_value(camera_af_power_gpio, 1);
		
		//usleep_range(1500, 1500);
	} else {
		gpio_set_value(camera_af_power_gpio, 0);
	}

	return 0;
}
#endif
static int bu64291_gpio_ctrl(struct v4l2_subdev *sd, int flag)
{
	int ret;

	
	return 0;
}
static struct camera_sensor_platform_data bu64291_sensor_platform_data = {
	.gpio_ctrl      = NULL,//bu64291_gpio_ctrl,
	.flisclk_ctrl   = NULL,
	.power_ctrl     =NULL,// bu64291_power_ctrl,

};
void *bu64291_platform_data(void *info)
{
	
	//camera_af_power = -1;

	return &bu64291_sensor_platform_data;
}
