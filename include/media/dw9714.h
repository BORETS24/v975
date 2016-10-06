/*
 * Support for Semco DW9714 IC.
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

#ifndef __DW9714_H__
#define __DW9714_H__

#include <linux/atomisp_platform.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <media/v4l2-subdev.h>

#define DW9714_NAME			"dw9714"
#define DW9714_MAX_I2C_MSG_LEN	30
#define DW9714_I2C_MSG_LENGTH		2

#define DW9714_16BIT			2
#define DW9714_8BIT			1

/*
 * EEPROM: I2C ID <-> ADDRESS
 *
 * PAGE 0: 0xa0 <-> 0x000-0x0ff
 * PAGE 1: 0xa1 <-> 0x100-0x1ff
 * PAGE 2: 0xa2 <-> 0x200-0x2ff
 * PAGE 3: 0xa3 <-> 0x300-0x3ff
 */
#define	DW9714_EEP_ID_BASE		(0xa0 >> 1)
#define	DW9714_EEP_SIZE		1024

#define DW9714_EEP_START_ADDR		0x030
#define DW9714_EEP_END_ADDR		0x1bf
#define DW9714_EEP_ADDR_EOF		0xff
#define DW9714_EEP_ADDR_DELAY		0xdd

#define DW9714_EEP_DATA_DIRECT	0x0
#define DW9714_EEP_DATA_INDIRECT_EEP	0x1
#define DW9714_EEP_DATA_INDIRECT_HVCA	0x2
#define DW9714_EEP_DATA_MASK_AND	0x7
#define DW9714_EEP_DATA_MASK_OR	0x8
#define DW9714_EEP_DATA_SIZE_MAX	2

#define DW9714_EEP_INF1		0x20
#define DW9714_EEP_MAC1		0x22
#define DW9714_EEP_INF2		0x24
#define DW9714_EEP_MAC2		0x26
#define DW9714_EEP_AF_TUN_END		0x28

struct dw9714_eeprom_data {
	u8 addr;
	unsigned size:4;
	unsigned type:4;
	u8 data[2];
};

#define DW9714_EEP_DATA_SIZE		sizeof(struct dw9714_eeprom_data)
#define DW9714_EEP_NUM_DATA		((DW9714_EEP_END_ADDR - \
					  DW9714_EEP_START_ADDR + 1) / \
					 DW9714_EEP_DATA_SIZE)

struct dw9714_eeprom_af_tun {
	s16 focus_abs_min;
	s16 focus_abs_max;
	s16 inf_pos;
	s16 mac_pos;
};

/*
 * FIXME: SLEW_MAX was defined base on valid register value, but a smaller
 * sane value must be defined.
 */
#define DW9714_VCM_SLEW_MAX		0x7fa0
#define DW9714_VCM_SLEW_MIN		0x280
#define DW9714_VCM_SLEW_DEFAULT	DW9714_VCM_SLEW_MIN
#define DW9714_VCM_ABS_MASK		0xffa0

#define DW9714_VCM_SLEW_TIME_MAX	0xff
#define DW9714_VCM_SLEW_TIME_DEFAULT	0x2
#define DW9714_VCM_SLEW_RETRY_MAX	50
#define DW9714_VCM_STAB_RETRY_MAX	15
#define DW9714_VCM_HOME_POS		0
#define DW9714_VCM_INVALID_MIN_POS	0x8001
#define DW9714_VCM_INVALID_MAX_POS	0x7fff

#define DW9714_WARMUP_STEP		10	/* 10% */
#define DW9714_WARMUP_POS_START	1
#define DW9714_WARMUP_POS_END		6

#define DW9714_REG16_MS1Z12		0x16	/* VCM slew */
#define DW9714_REG_STMVINT		0xa0	/* VCM step timing */
#define DW9714_REG16_STMVEND		0xa1	/* target position */
#define DW9714_REG16_RZ		0x04	/* current position */

#define DW9714_REG_STMVEN		0x8a
#define DW9714_REG_STMVEN_MOVE_BUSY	(1 << 0)	/* moving? */

#define DW9714_REG_MSSET		0x8f
#define DW9714_REG_MSSET_BUSY		(1 << 0)	/* stabilized? */

struct dw9714_dev {
	struct v4l2_subdev sd;
	struct mutex input_lock;	/* serialize access to priv data */
	int step;
	int timing;
	int moving;
	int power;

	struct camera_sensor_platform_data *platform_data;
	struct dw9714_eeprom_af_tun af_tun;
	unsigned char *eeprom_buf;
	__u32 eeprom_size;
};

struct dw9714_control {
	struct v4l2_queryctrl qc;
	int (*query)(struct v4l2_subdev *sd, s32 *value);
	int (*tweak)(struct v4l2_subdev *sd, s32 value);
};

#endif
