/*
 * Support for Semco BU64291 IC.
 *
 * Copyright (c) 2012 Intel Corporation. All Rights Reserved.
 *
 * Author: David Cohen <david.a.cohen@intel.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */

#ifndef __BU64291_H__
#define __BU64291_H__

#include <linux/atomisp_platform.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <media/v4l2-subdev.h>

#define	BU64291_NAME			"bu64291"
#define BU64291_MAX_I2C_MSG_LEN	30
#define BU64291_I2C_MSG_LENGTH		2

#define BU64291_16BIT			2
#define BU64291_8BIT			1

/*
 * EEPROM: I2C ID <-> ADDRESS
 *
 * PAGE 0: 0xa0 <-> 0x000-0x0ff
 * PAGE 1: 0xa1 <-> 0x100-0x1ff
 * PAGE 2: 0xa2 <-> 0x200-0x2ff
 * PAGE 3: 0xa3 <-> 0x300-0x3ff
 */
#define	BU64291_EEP_ID_BASE		(0xa0 >> 1)
#define	BU64291_EEP_SIZE		1024

#define BU64291_EEP_START_ADDR		0x030
#define BU64291_EEP_END_ADDR		0x1bf
#define BU64291_EEP_ADDR_EOF		0xff
#define BU64291_EEP_ADDR_DELAY		0xdd

#define BU64291_EEP_DATA_DIRECT	0x0
#define BU64291_EEP_DATA_INDIRECT_EEP	0x1
#define BU64291_EEP_DATA_INDIRECT_HVCA	0x2
#define BU64291_EEP_DATA_MASK_AND	0x7
#define BU64291_EEP_DATA_MASK_OR	0x8
#define BU64291_EEP_DATA_SIZE_MAX	2

#define BU64291_EEP_INF1		0x20
#define BU64291_EEP_MAC1		0x22
#define BU64291_EEP_INF2		0x24
#define BU64291_EEP_MAC2		0x26
#define BU64291_EEP_AF_TUN_END		0x28

struct bu64291_eeprom_data {
	u8 addr;
	unsigned size:4;
	unsigned type:4;
	u8 data[2];
};

#define BU64291_EEP_DATA_SIZE		sizeof(struct bu64291_eeprom_data)
#define BU64291_EEP_NUM_DATA		((BU64291_EEP_END_ADDR - \
					  BU64291_EEP_START_ADDR + 1) / \
					 BU64291_EEP_DATA_SIZE)

struct bu64291_eeprom_af_tun {
	s16 focus_abs_min;
	s16 focus_abs_max;
	s16 inf_pos;
	s16 mac_pos;
};

/* yaoling for bu64291 */
#define BU64291_SETTING_POINT_A		0x94
#define BU64291_POINTA_VALUE			0xCD

#define BU64291_SETTING_POINT_B		0x9c//0x9d
#define BU64291_POINTB_VALUE			0x33

#define BU64291_SETTING_POINT_ATOB	0xa4
#define BU64291_POINTATOB_VALUE			0x84//0xec


#define BU64291_VCM_SLEW_MASK		0x7fa0
/*
 * FIXME: SLEW_MAX was defined base on valid register value, but a smaller
 * sane value must be defined.
 */
#define BU64291_VCM_SLEW_MAX		0x7fa0
#define BU64291_VCM_SLEW_MIN		0x280
#define BU64291_VCM_SLEW_DEFAULT	BU64291_VCM_SLEW_MIN
#define BU64291_VCM_ABS_MASK		0xffa0

#define BU64291_VCM_SLEW_TIME_MAX	0xff
#define BU64291_VCM_SLEW_TIME_DEFAULT	0x2
#define BU64291_VCM_SLEW_RETRY_MAX	50
#define BU64291_VCM_STAB_RETRY_MAX	15
#define BU64291_VCM_HOME_POS		0
#define BU64291_VCM_INVALID_MIN_POS	0x8001
#define BU64291_VCM_INVALID_MAX_POS	0x7fff

#define BU64291_WARMUP_STEP		10	/* 10% */
#define BU64291_WARMUP_POS_START	1
#define BU64291_WARMUP_POS_END		6

#define BU64291_REG16_MS1Z12		0x16	/* VCM slew */
#define BU64291_REG_STMVINT		0xa0	/* VCM step timing */
#define BU64291_REG16_STMVEND		0xa1	/* target position */
#define BU64291_REG16_RZ		0x04	/* current position */

#define BU64291_REG_STMVEN		0x8a
#define BU64291_REG_STMVEN_MOVE_BUSY	(1 << 0)	/* moving? */

#define BU64291_REG_MSSET		0x8f
#define BU64291_REG_MSSET_BUSY		(1 << 0)	/* stabilized? */

struct bu64291_dev {
	struct v4l2_subdev sd;
	struct mutex input_lock;	/* serialize access to priv data */
	int step;
	int timing;
	int moving;
	int power;

	struct camera_sensor_platform_data *platform_data;
	struct bu64291_eeprom_af_tun af_tun;
	unsigned char *eeprom_buf;
	__u32 eeprom_size;
};

struct bu64291_control {
	struct v4l2_queryctrl qc;
	int (*query)(struct v4l2_subdev *sd, s32 *value);
	int (*tweak)(struct v4l2_subdev *sd, s32 value);
};

#endif
