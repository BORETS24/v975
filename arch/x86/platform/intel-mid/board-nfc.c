/*
 * board-nfc.c: nfc i2c device register
 *
 * (C) Copyright 2012 ZTE Corporation
 * Author:  Jia Baofeng <jia.baofeng@zte.com.cn>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 * of the License.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/nfc/bcm2079x.h>

static struct bcm2079x_platform_data nfc_pdata = {
	.irq_gpio = 43,
	.en_gpio = 114,
	.wake_gpio = 115,
	};

static struct i2c_board_info __initdata nfc_i2c_bus0_board_info[] = {
	{
		I2C_BOARD_INFO("bcm2079x-i2c", 0x76),
		.platform_data = &nfc_pdata,
	},
};


static int __init intel_nfc_init(void)
{
	i2c_register_board_info(0, nfc_i2c_bus0_board_info,
			ARRAY_SIZE(nfc_i2c_bus0_board_info));
	return 0;
}
arch_initcall(intel_nfc_init);

