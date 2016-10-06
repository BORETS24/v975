/*
 * board-sensors.c: Sensor i2c device register
 *
 * (C) Copyright 2012 ZTE Corporation
 * Author:  Lu Poyuan <lu.poyuan@zte.com.cn>
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

#ifdef CONFIG_SENSORS_TMD2771
#include <linux/i2c/tmd2771x.h>
#endif
#ifdef CONFIG_ACCEL_SENSORS
#include <linux/accel.h>
#endif
#ifdef CONFIG_SENSORS_AK8963
#include <linux/i2c/akm8963.h>
#endif

#ifdef CONFIG_ACCEL_SENSORS
static unsigned short accel_addr_list[] = {
    0x0F, // kionix
    0x19, // st
    I2C_CLIENT_END
};

static struct accel_platform_data accel_pdata ={
	.adapt_nr = 5,
	.accel_list = accel_addr_list,
	.poll_inerval = 10,
	.g_range = ACCEL_FS_2G,
};

static struct platform_device accel_platform_device = {
    .name		= "accel_platform",
    .id = -1,
    .dev = {
        .platform_data = &accel_pdata,
    },
};
#endif

#ifdef CONFIG_SENSORS_AK8963
static struct akm8963_platform_data akm8963_pdata = {
	.gpio_DRDY = 58,
	.gpio_RST = 0,
	.layout = 3,
	.outbit = 1,
};
#endif

#ifdef CONFIG_SENSORS_TMD2771  
#define TMD27713_INT_GPIO	0x3F

static struct prox_config prox_config_pdata = {
	.pulse_count = 4,
	.led_drive = PROX_LED_DRIVE_25MA,
	.threshold_lo = 500,
	.threshold_hi = 560,
	.thresh_gain_hi = 200,
	.thresh_gain_lo = 170,
};

static int tmd2771x_irq_setup(void){
	int status = 0;
     
	status = gpio_request(TMD27713_INT_GPIO, "tmd27713_int");
	if (status) {
		pr_err("%s:Failed to request GPIO %d\n",
					__func__, TMD27713_INT_GPIO);
		return status;
	}
	status = gpio_direction_input(TMD27713_INT_GPIO);
	if (status) {
		pr_err("%s:Failed to configure GPIO %d\n",
				__func__, TMD27713_INT_GPIO);
		goto gpio_free_int;
	}

	pr_debug("\nTMD27713 INT GPIO configuration done\n");
	return status;

gpio_free_int:
	gpio_free(TMD27713_INT_GPIO);

	return status;
}

static struct tmd2771x_platform_data tmd2771x_pdata = {
	.prox_config = &prox_config_pdata,
	.irq_gpio_setup = tmd2771x_irq_setup,
};
#endif

static const struct i2c_board_info intel_i2c5_board_info[] = {
#ifdef CONFIG_SENSORS_AK8963
	{
		I2C_BOARD_INFO(AKM8963_I2C_NAME, 0x0C),
		.platform_data = &akm8963_pdata,
		.irq = -1,
	},
#endif
#ifdef CONFIG_SENSORS_LSM330D_G
	{
		I2C_BOARD_INFO("lsm330dlc_gyr", 0x6B),
	},
#endif
#ifdef CONFIG_SENSORS_TMD2771
	{
		I2C_BOARD_INFO("tmd2771x", 0x39),
		.platform_data = &tmd2771x_pdata,
	},
#endif
	{
		I2C_BOARD_INFO("null_dev", 0x00),
	},
};

static int __init intel_sensors_init(void)
{
	
	if(ARRAY_SIZE(intel_i2c5_board_info) > 1)
		i2c_register_board_info(5, intel_i2c5_board_info,
			(ARRAY_SIZE(intel_i2c5_board_info)-1));

#ifdef CONFIG_ACCEL_SENSORS
    platform_device_register(&accel_platform_device);
#endif

	return 0;
}
arch_initcall(intel_sensors_init);

