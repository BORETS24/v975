/*
 * include/media/ADP1650.h
 *
 * Copyright (c) 2010-2012 Intel Corporation. All Rights Reserved.
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
#ifndef _ADP1650_H_
#define _ADP1650_H_

#include <linux/videodev2.h>
#include <media/v4l2-subdev.h>

#define ADP1650_NAME    "adp1650"
#define ADP1650_ID      3554

#define	v4l2_queryctrl_entry_integer(_id, _name,\
		_minimum, _maximum, _step, \
		_default_value, _flags)	\
	{\
		.id = (_id), \
		.type = V4L2_CTRL_TYPE_INTEGER, \
		.name = _name, \
		.minimum = (_minimum), \
		.maximum = (_maximum), \
		.step = (_step), \
		.default_value = (_default_value),\
		.flags = (_flags),\
	}
#define	v4l2_queryctrl_entry_boolean(_id, _name,\
		_default_value, _flags)	\
	{\
		.id = (_id), \
		.type = V4L2_CTRL_TYPE_BOOLEAN, \
		.name = _name, \
		.minimum = 0, \
		.maximum = 1, \
		.step = 1, \
		.default_value = (_default_value),\
		.flags = (_flags),\
	}

#define	s_ctrl_id_entry_integer(_id, _name, \
		_minimum, _maximum, _step, \
		_default_value, _flags, \
		_s_ctrl, _g_ctrl)	\
	{\
		.qc = v4l2_queryctrl_entry_integer(_id, _name,\
				_minimum, _maximum, _step,\
				_default_value, _flags), \
		.s_ctrl = _s_ctrl, \
		.g_ctrl = _g_ctrl, \
	}

#define	s_ctrl_id_entry_boolean(_id, _name, \
		_default_value, _flags, \
		_s_ctrl, _g_ctrl)	\
	{\
		.qc = v4l2_queryctrl_entry_boolean(_id, _name,\
				_default_value, _flags), \
		.s_ctrl = _s_ctrl, \
		.g_ctrl = _g_ctrl, \
	}

/* Value settings for Flash Time-out Duration*/

#define ADP1650_DEFAULT_TIMEOUT          512U
#define ADP1650_MIN_TIMEOUT              32U
#define ADP1650_MAX_TIMEOUT              1024U
#define ADP1650_TIMEOUT_STEPSIZE         32U

/* Flash modes */
#define ADP1650_MODE_OFF  0//ADP1650_MODE_SHUTDOWN            0
#define ADP1650_MODE_INDICATOR 3 //ADP1650_MODE_INDICATOR           1
#define ADP1650_MODE_TORCH               2
#define ADP1650_MODE_FLASH 1 //ADP1650_MODE_FLASH               3

/* timer delay time */
#define ADP1650_TIMER_DELAY		5

/* Percentage <-> value macros */
#define ADP1650_MIN_PERCENT                   0U
#define ADP1650_MAX_PERCENT                   100U
#define ADP1650_CLAMP_PERCENTAGE(val) \
	clamp(val, ADP1650_MIN_PERCENT, ADP1650_MAX_PERCENT)

#define ADP1650_VALUE_TO_PERCENT(v, step)     (((((unsigned long)(v))*(step))+50)/100)
#define ADP1650_PERCENT_TO_VALUE(p, step)     (((((unsigned long)(p))*100)+(step>>1))/(step))

/* Product specific limits
 * TODO: get these from platform data */
#define ADP1650_FLASH_MAX_LVL   0x0F /* 1191mA */

/* Flash brightness, input is percentage, output is [0..15] */
#define ADP1650_FLASH_STEP	\
	((100ul*(ADP1650_MAX_PERCENT)+((ADP1650_FLASH_MAX_LVL)>>1))/((ADP1650_FLASH_MAX_LVL)))
#define ADP1650_FLASH_DEFAULT_BRIGHTNESS \
	ADP1650_VALUE_TO_PERCENT(13, ADP1650_FLASH_STEP)

/* Torch brightness, input is percentage, output is [0..7] */
#define ADP1650_TORCH_STEP                    1250
#define ADP1650_TORCH_DEFAULT_BRIGHTNESS \
	ADP1650_VALUE_TO_PERCENT(3, ADP1650_TORCH_STEP)
	//ADP1650_VALUE_TO_PERCENT(2, ADP1650_TORCH_STEP)

/* Indicator brightness, input is percentage, output is [0..3] */
#define ADP1650_INDICATOR_STEP                2500
#define ADP1650_INDICATOR_DEFAULT_BRIGHTNESS \
	ADP1650_VALUE_TO_PERCENT(1, ADP1650_INDICATOR_STEP)

/*
 * ADP1650_platform_data - Flash controller platform data
 */
struct adp1650_platform_data {
	int gpio_torch;
	int gpio_strobe;
	//int gpio_reset;
	int gpio_enable;

	unsigned int current_limit;
	//unsigned int envm_tx2;
	//unsigned int tx2_polarity;
};

#endif /* _ADP1650_H_ */

