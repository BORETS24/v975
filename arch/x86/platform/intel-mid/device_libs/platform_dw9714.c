/*
 * platform_dw9714.c: dw9714 platform data initilization file
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

#include <media/dw9714.h>

#include "platform_dw9714.h"
#include "platform_camera.h"

static int dw9714_gpio_ctrl(struct v4l2_subdev *sd, int flag)
{	
	return 0;
}

static struct camera_sensor_platform_data dw9714_sensor_platform_data = {
	.gpio_ctrl      = NULL,
	.flisclk_ctrl   = NULL,
	.power_ctrl     =NULL,

};
void *dw9714_platform_data(void *info)
{
	return &dw9714_sensor_platform_data;
}
