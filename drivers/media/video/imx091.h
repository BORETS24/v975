/*
 * Support for Sony imx091 camera sensor.
 * Based on Aptina mt9e013 driver.
 *
 * Copyright (c) 2010 Intel Corporation. All Rights Reserved.
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

#ifndef __IMX091_H__
#define __IMX091_H__
#include <linux/atomisp_platform.h>
#include <linux/atomisp.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/videodev2.h>
#include <linux/v4l2-mediabus.h>
#include <linux/types.h>

#include <media/media-entity.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>

#define IMX091_NAME	"imx091"
#define IMX091_CHIP_ID	       0x0091

#define  LAST_REG_SETING		{0xffff, 0xff}
#define  is_last_reg_setting(item) ((item).reg == 0xffff)
#define I2C_MSG_LENGTH		0x2

#define IMX091_INVALID_CONFIG	0xffffffff

#define IMX091_INTG_UNIT_US	100
#define IMX091_MCLK		192

#define IMX091_REG_BITS	16
#define IMX091_REG_MASK	0xFFFF

/* This should be added into include/linux/videodev2.h */
#ifndef V4L2_IDENT_IMX091
#define V4L2_IDENT_IMX091	91
#endif


#define IMX091_CHIP_ID_HIGH			0x0000
#define IMX091_CHIP_ID_LOW			       0x0001

#define IMX091_STREAM_MODE			0x0100

#define IMX091_FOCAL_LENGTH_NUM	369	/*3.69mm*/
#define IMX091_FOCAL_LENGTH_DEM	100
#define IMX091_F_NUMBER_DEFAULT_NUM	22
#define IMX091_F_NUMBER_DEM	10

#define IMX091_COARSE_INTEGRATION_TIME  0x0202
#define IMX091_GLOBAL_GAIN 0x0205

/*
 * focal length bits definition:
 * bits 31-16: numerator, bits 15-0: denominator
 */
#define IMX091_FOCAL_LENGTH_DEFAULT 0x1710064

/*
 * current f-number bits definition:
 * bits 31-16: numerator, bits 15-0: denominator
 */
#define IMX091_F_NUMBER_DEFAULT 0x16000a

/*
 * f-number range bits definition:
 * bits 31-24: max f-number numerator
 * bits 23-16: max f-number denominator
 * bits 15-8: min f-number numerator
 * bits 7-0: min f-number denominator
 */
#define IMX091_F_NUMBER_RANGE 0x160a160a

#define OTPM_ADD_START_1		0x1000
#define OTPM_DATA_LENGTH_1		0x0100
#define OTPM_COUNT 0x200

/*
 * imx091 System control registers
 */
#define IMX091_MASK_8BIT	0xFF
#define IMX091_MASK_4BIT	0xF
#define IMX091_MASK_2BIT	0x3
#define IMX091_MASK_11BIT	0x7FF
#define IMX091_INTG_BUF_COUNT		2

#define IMX091_FINE_INTG_TIME		0x1E8

#define IMX091_VT_PIX_CLK_DIV			0x0301
#define IMX091_VT_SYS_CLK_DIV			0x0303
#define IMX091_PRE_PLL_CLK_DIV			0x0305
#define IMX091_PLL_MULTIPLIER			0x0307
#define IMX091_OP_PIX_DIV			0x0309
#define IMX091_OP_SYS_DIV			0x030B
#define IMX091_FRAME_LENGTH_LINES		0x0340
#define IMX091_COARSE_INTG_TIME_MIN		0x1004

#define IMX091_READ_MODE				0x0390  ////???????????

#define IMX091_COARSE_INTEGRATION_TIME		0x0202
#define IMX091_TEST_PATTERN_MODE			0x0600
#define IMX091_IMG_ORIENTATION			0x0101
#define IMX091_VFLIP_BIT			1
#define IMX091_GLOBAL_GAIN			0x0205


/* Defines for register writes and register array processing */
#define IMX091_BYTE_MAX	32
#define IMX091_SHORT_MAX	16
#define I2C_RETRY_COUNT		5
#define IMX091_TOK_MASK	0xfff0

#define MAX_FMTS 1

#define	IMX091_STATUS_POWER_DOWN	0x0
#define	IMX091_STATUS_STANDBY		0x2
#define	IMX091_STATUS_ACTIVE		0x3
#define	IMX091_STATUS_VIEWFINDER	0x4

#define MAX_FPS_OPTIONS_SUPPORTED	3

#define	v4l2_format_capture_type_entry(_width, _height, \
		_pixelformat, _bytesperline, _colorspace) \
	{\
		.type = V4L2_BUF_TYPE_VIDEO_CAPTURE,\
		.fmt.pix.width = (_width),\
		.fmt.pix.height = (_height),\
		.fmt.pix.pixelformat = (_pixelformat),\
		.fmt.pix.bytesperline = (_bytesperline),\
		.fmt.pix.colorspace = (_colorspace),\
		.fmt.pix.sizeimage = (_height)*(_bytesperline),\
	}

#define	s_output_format_entry(_width, _height, _pixelformat, \
		_bytesperline, _colorspace, _fps) \
	{\
		.v4l2_fmt = v4l2_format_capture_type_entry(_width, \
			_height, _pixelformat, _bytesperline, \
				_colorspace),\
		.fps = (_fps),\
	}

#define	s_output_format_reg_entry(_width, _height, _pixelformat, \
		_bytesperline, _colorspace, _fps, _reg_setting) \
	{\
		.s_fmt = s_output_format_entry(_width, _height,\
				_pixelformat, _bytesperline, \
				_colorspace, _fps),\
		.reg_setting = (_reg_setting),\
	}

struct s_ctrl_id {
	struct v4l2_queryctrl qc;
	int (*s_ctrl)(struct v4l2_subdev *sd, u32 val);
	int (*g_ctrl)(struct v4l2_subdev *sd, u32 *val);
};

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


#define	macro_string_entry(VAL)	\
	{ \
		.val = VAL, \
		.string = #VAL, \
	}

enum imx091_tok_type {
	IMX091_8BIT  = 0x0001,
	IMX091_16BIT = 0x0002,
	IMX091_TOK_TERM   = 0xf000,	/* terminating token for reg list */
	IMX091_TOK_DELAY  = 0xfe00	/* delay token for reg list */
};

/*
 * If register address or register width is not 32 bit width,
 * user needs to convert it manually
 */

struct s_register_setting {
	u32 reg;
	u32 val;
};

struct s_output_format {
	struct v4l2_format v4l2_fmt;
	int fps;
};

/**
 * struct imx091_fwreg - Firmware burst command
 * @type: FW burst or 8/16 bit register
 * @addr: 16-bit offset to register or other values depending on type
 * @val: data value for burst (or other commands)
 *
 * Define a structure for sensor register initialization values
 */
struct imx091_fwreg {
	enum imx091_tok_type type; /* value, register or FW burst string */
	u16 addr;	/* target address */
	u32 val[8];
};

/**
 * struct imx091_reg - MI sensor  register format
 * @type: type of the register
 * @reg: 16-bit offset to register
 * @val: 8/16/32-bit register value
 *
 * Define a structure for sensor register initialization values
 */
struct imx091_reg {
	enum imx091_tok_type type;
	union {
		u16 sreg;
		struct imx091_fwreg *fwreg;
	} reg;
	u32 val;	/* @set value for read/mod/write, @mask */
	u32 val2;	/* optional: for rmw, OR mask */
};

struct imx091_fps_setting {
	int fps;
	unsigned short pixels_per_line;
	unsigned short lines_per_frame;
};

/* Store macro values' debug names */
struct macro_string {
	u8 val;
	char *string;
};

static inline const char *
macro_to_string(const struct macro_string *array, int size, u8 val)
{
	int i;
	for (i = 0; i < size; i++) {
		if (array[i].val == val)
			return array[i].string;
	}
	return "Unknown VAL";
}

struct imx091_control {
	struct v4l2_queryctrl qc;
	int (*query)(struct v4l2_subdev *sd, s32 *value);
	int (*tweak)(struct v4l2_subdev *sd, s32 value);
};

struct imx091_resolution {
	u8 *desc;
	int res;
	int width;
	int height;
	bool used;
	const struct imx091_reg *regs;
	u8 bin_factor_x;
	u8 bin_factor_y;
	unsigned short skip_frames;
	const struct imx091_fps_setting fps_options[MAX_FPS_OPTIONS_SUPPORTED];
};

struct imx091_format {
	u8 *desc;
	u32 pixelformat;
	struct s_register_setting *regs;
};

/* imx091 device structure */
struct imx091_device {
	struct v4l2_subdev sd;
	struct media_pad pad;
	struct v4l2_mbus_framefmt format;

	struct camera_sensor_platform_data *platform_data;
	int fmt_idx;
	int status;
	int streaming;
	int power;
	//int run_mode;
	int vt_pix_clk_freq_mhz;
	u16 sensor_id;
	u16 coarse_itg;
	u16 fine_itg;
	u8 sensor_revision;
	int exposure;
	int gain;
	u16 digital_gain;
	//struct drv201_device drv201;
	struct mutex input_lock; /* serialize sensor's ioctl */

	const struct imx091_reg *basic_settings_list;
	const struct imx091_resolution *curr_res_table;
	int entries_curr_table;
	int fps_index;

	struct v4l2_ctrl_handler ctrl_handler;
	struct v4l2_ctrl *run_mode;
};

/*
 * The i2c adapter on Intel Medfield can transfer 32 bytes maximum
 * at a time. In burst mode we require that the buffer is transferred
 * in one shot, so limit the buffer size to 32 bytes minus a safety.
 */
#define IMX091_MAX_WRITE_BUF_SIZE	30
struct imx091_write_buffer {
	u16 addr;
	u8 data[IMX091_MAX_WRITE_BUF_SIZE];
};

struct imx091_write_ctrl {
	int index;
	struct imx091_write_buffer buffer;
};

#define IMX091_RES_WIDTH_MAX	4208
#define IMX091_RES_HEIGHT_MAX	3120

static const struct imx091_reg imx091_SwReset[] = {
	{ IMX091_8BIT,  {0x0100} , 0x00 },
	{ IMX091_8BIT,  {0x0103} , 0x01 },
	{ IMX091_TOK_DELAY, {0}, 5},
	{ IMX091_8BIT,  {0x0103} , 0x00 },
	{ IMX091_TOK_TERM, {0}, 0},
};

static const struct imx091_reg imx091_soft_standby[] = {
	{IMX091_8BIT, { 0x0100 },  0x00},
	{IMX091_TOK_TERM, {0}, 0}
};

static const struct imx091_reg imx091_streaming[] = {
	{IMX091_8BIT, { 0x0100 },  0x01},
	{IMX091_TOK_TERM, {0}, 0}
};

static const struct imx091_reg imx091_param_hold[] = {
	{IMX091_8BIT, {0x0104}, 0x01},	/* GROUPED_PARAMETER_HOLD */
	{IMX091_TOK_TERM, {0}, 0}
};

static const struct imx091_reg imx091_param_update[] = {
	{IMX091_8BIT, {0x0104}, 0x00},	/* GROUPED_PARAMETER_HOLD */
	{IMX091_TOK_TERM, {0}, 0}
};

static const struct imx091_reg imx091_BasicSettings[] = {
	/* Global setting */
	{ IMX091_8BIT, { 0x3087 }, 0x53 },
	{ IMX091_8BIT, { 0x309D }, 0x94 },
	{ IMX091_8BIT, { 0x30A1 }, 0x08 },
	{ IMX091_8BIT, { 0x30AA }, 0x04 },	
	{ IMX091_8BIT, { 0x30B1 }, 0x00 },	
	{ IMX091_8BIT, { 0x30C7 }, 0x00 },
	{ IMX091_8BIT, { 0x3115 }, 0x0E },
	{ IMX091_8BIT, { 0x3118 }, 0x42 },
	{ IMX091_8BIT, { 0x311D }, 0x34 },
	{ IMX091_8BIT, { 0x3121 }, 0x0D },
	{ IMX091_8BIT, { 0x3212 }, 0xF2 },
	{ IMX091_8BIT, { 0x3213 }, 0x0F },
	{ IMX091_8BIT, { 0x3215 }, 0x0F },
	{ IMX091_8BIT, { 0x3217 }, 0x0B },
	{ IMX091_8BIT, { 0x3219 }, 0x0B },
	{ IMX091_8BIT, { 0x321B }, 0x0D },
	{ IMX091_8BIT, { 0x321D }, 0x0D },
	{ IMX091_8BIT, { 0x0101 }, 0x03 },
	/* Black Level setting */
	{ IMX091_8BIT, { 0x3032 }, 0x40 },
	{ IMX091_TOK_TERM, {0}, 0}
};

/*****************************STILL********************************/

static const struct imx091_reg imx091_VGA_STILL[] = {
	/* PLL setting */
	{ IMX091_8BIT, { 0x0305 }, 0x02},
	{ IMX091_8BIT, { 0x0307 }, 0x39},
	{ IMX091_8BIT, { 0x30A4 }, 0x02},
	{ IMX091_8BIT, { 0x303C }, 0x4B},
	/* Mode setting */	
	{ IMX091_8BIT, { 0x0112 }, 0x0A},
	{ IMX091_8BIT, { 0x0113 }, 0x0A},
	{ IMX091_8BIT, { 0x0340 }, 0x06},
	{ IMX091_8BIT, { 0x0341 }, 0x5A},
	{ IMX091_8BIT, { 0x0342 }, 0x12},
	{ IMX091_8BIT, { 0x0343 }, 0x0C},
	{ IMX091_8BIT, { 0x0344 }, 0x00},
	{ IMX091_8BIT, { 0x0345 }, 0x90},
	{ IMX091_8BIT, { 0x0346 }, 0x00},
	{ IMX091_8BIT, { 0x0347 }, 0x78},
	{ IMX091_8BIT, { 0x0348 }, 0x0F},
	{ IMX091_8BIT, { 0x0349 }, 0xEF},
	{ IMX091_8BIT, { 0x034A }, 0x0C},
	{ IMX091_8BIT, { 0x034B }, 0x17},
	{ IMX091_8BIT, { 0x034C }, 0x02},
	{ IMX091_8BIT, { 0x034D }, 0x90},
	{ IMX091_8BIT, { 0x034E }, 0x01},
	{ IMX091_8BIT, { 0x034F }, 0xF0},
	{ IMX091_8BIT, { 0x0381 }, 0x05},
	{ IMX091_8BIT, { 0x0383 }, 0x07},
	{ IMX091_8BIT, { 0x0385 }, 0x05},
	{ IMX091_8BIT, { 0x0387 }, 0x07},
	{ IMX091_8BIT, { 0x3033 }, 0x00},
	{ IMX091_8BIT, { 0x303D }, 0x10},
	{ IMX091_8BIT, { 0x303E }, 0xD0},
	{ IMX091_8BIT, { 0x3040 }, 0x00},
	{ IMX091_8BIT, { 0x3041 }, 0x00},
	{ IMX091_8BIT, { 0x30B0 }, 0x00},
	{ IMX091_8BIT, { 0x3048 }, 0x21},
	{ IMX091_8BIT, { 0x304C }, 0x7F},
	{ IMX091_8BIT, { 0x304D }, 0x04},
	{ IMX091_8BIT, { 0x3064 }, 0x12},
	{ IMX091_8BIT, { 0x309B }, 0x28},
	{ IMX091_8BIT, { 0x309E }, 0x00},
	{ IMX091_8BIT, { 0x30A0 }, 0x14},
	{ IMX091_8BIT, { 0x30B2 }, 0x00},
	{ IMX091_8BIT, { 0x30D5 }, 0x09},
	{ IMX091_8BIT, { 0x30D6 }, 0x00},
	{ IMX091_8BIT, { 0x30D7 }, 0x00},
	{ IMX091_8BIT, { 0x30D8 }, 0x00},
	{ IMX091_8BIT, { 0x30D9 }, 0x00},
	{ IMX091_8BIT, { 0x30DA }, 0x00},
	{ IMX091_8BIT, { 0x30DB }, 0x00},
	{ IMX091_8BIT, { 0x30DC }, 0x00},
	{ IMX091_8BIT, { 0x30DD }, 0x00},
	{ IMX091_8BIT, { 0x30DE }, 0x06},
	{ IMX091_8BIT, { 0x3102 }, 0x10},
	{ IMX091_8BIT, { 0x3103 }, 0x44},
	{ IMX091_8BIT, { 0x3104 }, 0x40},
	{ IMX091_8BIT, { 0x3105 }, 0x00},
	{ IMX091_8BIT, { 0x3106 }, 0x0D},
	{ IMX091_8BIT, { 0x3107 }, 0x01},
	{ IMX091_8BIT, { 0x310A }, 0x0A},
	{ IMX091_8BIT, { 0x315C }, 0x99},
	{ IMX091_8BIT, { 0x315D }, 0x98},
	{ IMX091_8BIT, { 0x316E }, 0x9A},
	{ IMX091_8BIT, { 0x316F }, 0x99},
	{ IMX091_8BIT, { 0x3301 }, 0x03},
	{ IMX091_8BIT, { 0x3304 }, 0x05},
	{ IMX091_8BIT, { 0x3305 }, 0x04},
	{ IMX091_8BIT, { 0x3306 }, 0x12},
	{ IMX091_8BIT, { 0x3307 }, 0x03},
	{ IMX091_8BIT, { 0x3308 }, 0x0D},
	{ IMX091_8BIT, { 0x3309 }, 0x05},
	{ IMX091_8BIT, { 0x330A }, 0x09},
	{ IMX091_8BIT, { 0x330B }, 0x04},
	{ IMX091_8BIT, { 0x330C }, 0x08},
	{ IMX091_8BIT, { 0x330D }, 0x05},
	{ IMX091_8BIT, { 0x330E }, 0x03},
	{ IMX091_8BIT, { 0x3318 }, 0x77},
	{ IMX091_8BIT, { 0x3322 }, 0x02},
	{ IMX091_8BIT, { 0x3342 }, 0x0F},
	{ IMX091_8BIT, { 0x3348 }, 0xE0},
	/* Shutter Gain setting */
	{ IMX091_8BIT, { 0x0202 }, 0x06},
	{ IMX091_8BIT, { 0x0203 }, 0x55},

	{ IMX091_TOK_TERM, {0}, 0}
};

static const struct imx091_reg imx091_1M_STILL[] = {
	/* PLL setting */
	{ IMX091_8BIT, { 0x0305 }, 0x02},
	{ IMX091_8BIT, { 0x0307 }, 0x39},
	{ IMX091_8BIT, { 0x30A4 }, 0x02},
	{ IMX091_8BIT, { 0x303C }, 0x4B},
	/* Mode setting */	
	{ IMX091_8BIT, { 0x0112 }, 0x0A},
	{ IMX091_8BIT, { 0x0113 }, 0x0A},
	{ IMX091_8BIT, { 0x0340 }, 0x06},
	{ IMX091_8BIT, { 0x0341 }, 0x5A},
	{ IMX091_8BIT, { 0x0342 }, 0x12},
	{ IMX091_8BIT, { 0x0343 }, 0x0C},
	{ IMX091_8BIT, { 0x0344 }, 0x02},
	{ IMX091_8BIT, { 0x0345 }, 0x28},
	{ IMX091_8BIT, { 0x0346 }, 0x01},
	{ IMX091_8BIT, { 0x0347 }, 0xB0},
	{ IMX091_8BIT, { 0x0348 }, 0x0E},
	{ IMX091_8BIT, { 0x0349 }, 0x57},
	{ IMX091_8BIT, { 0x034A }, 0x0A},
	{ IMX091_8BIT, { 0x034B }, 0xDF},
	{ IMX091_8BIT, { 0x034C }, 0x04},
	{ IMX091_8BIT, { 0x034D }, 0x10},
	{ IMX091_8BIT, { 0x034E }, 0x03},
	{ IMX091_8BIT, { 0x034F }, 0x10},
	{ IMX091_8BIT, { 0x0381 }, 0x03},
	{ IMX091_8BIT, { 0x0383 }, 0x03},
	{ IMX091_8BIT, { 0x0385 }, 0x03},
	{ IMX091_8BIT, { 0x0387 }, 0x03},
	{ IMX091_8BIT, { 0x3033 }, 0x00},
	{ IMX091_8BIT, { 0x303D }, 0x10},
	{ IMX091_8BIT, { 0x303E }, 0xD0},
	{ IMX091_8BIT, { 0x3040 }, 0x00},
	{ IMX091_8BIT, { 0x3041 }, 0x00},
	{ IMX091_8BIT, { 0x30B0 }, 0x00},
	{ IMX091_8BIT, { 0x3048 }, 0x22},
	{ IMX091_8BIT, { 0x304C }, 0x7F},
	{ IMX091_8BIT, { 0x304D }, 0x04},
	{ IMX091_8BIT, { 0x3064 }, 0x12},
	{ IMX091_8BIT, { 0x309B }, 0x60},
	{ IMX091_8BIT, { 0x309E }, 0x04},
	{ IMX091_8BIT, { 0x30A0 }, 0x14},
	{ IMX091_8BIT, { 0x30B2 }, 0x00},
	{ IMX091_8BIT, { 0x30D5 }, 0x09},
	{ IMX091_8BIT, { 0x30D6 }, 0x00},
	{ IMX091_8BIT, { 0x30D7 }, 0x00},
	{ IMX091_8BIT, { 0x30D8 }, 0x00},
	{ IMX091_8BIT, { 0x30D9 }, 0x89},
	{ IMX091_8BIT, { 0x30DA }, 0x00},
	{ IMX091_8BIT, { 0x30DB }, 0x00},
	{ IMX091_8BIT, { 0x30DC }, 0x00},
	{ IMX091_8BIT, { 0x30DD }, 0x00},
	{ IMX091_8BIT, { 0x30DE }, 0x03},
	{ IMX091_8BIT, { 0x3102 }, 0x09},
	{ IMX091_8BIT, { 0x3103 }, 0x23},
	{ IMX091_8BIT, { 0x3104 }, 0x24},
	{ IMX091_8BIT, { 0x3105 }, 0x00},
	{ IMX091_8BIT, { 0x3106 }, 0x8B},
	{ IMX091_8BIT, { 0x3107 }, 0x00},
	{ IMX091_8BIT, { 0x310A }, 0x0A},
	{ IMX091_8BIT, { 0x315C }, 0x4A},
	{ IMX091_8BIT, { 0x315D }, 0x49},
	{ IMX091_8BIT, { 0x316E }, 0x4B},
	{ IMX091_8BIT, { 0x316F }, 0x4A},
	{ IMX091_8BIT, { 0x3301 }, 0x03},
	{ IMX091_8BIT, { 0x3304 }, 0x05},
	{ IMX091_8BIT, { 0x3305 }, 0x04},
	{ IMX091_8BIT, { 0x3306 }, 0x12},
	{ IMX091_8BIT, { 0x3307 }, 0x03},
	{ IMX091_8BIT, { 0x3308 }, 0x0D},
	{ IMX091_8BIT, { 0x3309 }, 0x05},
	{ IMX091_8BIT, { 0x330A }, 0x09},
	{ IMX091_8BIT, { 0x330B }, 0x04},
	{ IMX091_8BIT, { 0x330C }, 0x08},
	{ IMX091_8BIT, { 0x330D }, 0x05},
	{ IMX091_8BIT, { 0x330E }, 0x03},
	{ IMX091_8BIT, { 0x3318 }, 0x6A},
	{ IMX091_8BIT, { 0x3322 }, 0x02},
	{ IMX091_8BIT, { 0x3342 }, 0x0F},
	{ IMX091_8BIT, { 0x3348 }, 0xE0},
	/* Shutter Gain setting */
	{ IMX091_8BIT, { 0x0202 }, 0x06},
	{ IMX091_8BIT, { 0x0203 }, 0x55},
	
	{ IMX091_TOK_TERM, {0}, 0}
};

static const struct imx091_reg imx091_2M_STILL[] = {
	/* PLL setting */
	{ IMX091_8BIT, { 0x0305 }, 0x02},
	{ IMX091_8BIT, { 0x0307 }, 0x39},
	{ IMX091_8BIT, { 0x30A4 }, 0x02},
	{ IMX091_8BIT, { 0x303C }, 0x4B},
	/* Mode setting */	
	{ IMX091_8BIT, { 0x0112 }, 0x0A},
	{ IMX091_8BIT, { 0x0113 }, 0x0A},
	{ IMX091_8BIT, { 0x0340 }, 0x06},
	{ IMX091_8BIT, { 0x0341 }, 0x5A},
	{ IMX091_8BIT, { 0x0342 }, 0x12},
	{ IMX091_8BIT, { 0x0343 }, 0x0C},
	{ IMX091_8BIT, { 0x0344 }, 0x01},
	{ IMX091_8BIT, { 0x0345 }, 0xE0},
	{ IMX091_8BIT, { 0x0346 }, 0x01},
	{ IMX091_8BIT, { 0x0347 }, 0x80},
	{ IMX091_8BIT, { 0x0348 }, 0x0E},
	{ IMX091_8BIT, { 0x0349 }, 0x9F},
	{ IMX091_8BIT, { 0x034A }, 0x0B},
	{ IMX091_8BIT, { 0x034B }, 0x0F},
	{ IMX091_8BIT, { 0x034C }, 0x06},
	{ IMX091_8BIT, { 0x034D }, 0x60},
	{ IMX091_8BIT, { 0x034E }, 0x04},
	{ IMX091_8BIT, { 0x034F }, 0xC8},
	{ IMX091_8BIT, { 0x0381 }, 0x01},
	{ IMX091_8BIT, { 0x0383 }, 0x03},
	{ IMX091_8BIT, { 0x0385 }, 0x01},
	{ IMX091_8BIT, { 0x0387 }, 0x03},
	{ IMX091_8BIT, { 0x3033 }, 0x00},
	{ IMX091_8BIT, { 0x303D }, 0x10},
	{ IMX091_8BIT, { 0x303E }, 0xD0},
	{ IMX091_8BIT, { 0x3040 }, 0x00},
	{ IMX091_8BIT, { 0x3041 }, 0x00},
	{ IMX091_8BIT, { 0x30B0 }, 0x00},
	{ IMX091_8BIT, { 0x3048 }, 0x01},
	{ IMX091_8BIT, { 0x304C }, 0x7F},
	{ IMX091_8BIT, { 0x304D }, 0x04},
	{ IMX091_8BIT, { 0x3064 }, 0x12},
	{ IMX091_8BIT, { 0x309B }, 0x28},
	{ IMX091_8BIT, { 0x309E }, 0x00},
	{ IMX091_8BIT, { 0x30A0 }, 0x14},
	{ IMX091_8BIT, { 0x30B2 }, 0x00},
	{ IMX091_8BIT, { 0x30D5 }, 0x09},
	{ IMX091_8BIT, { 0x30D6 }, 0x01},
	{ IMX091_8BIT, { 0x30D7 }, 0x01},
	{ IMX091_8BIT, { 0x30D8 }, 0x64},
	{ IMX091_8BIT, { 0x30D9 }, 0x89},
	{ IMX091_8BIT, { 0x30DA }, 0x00},
	{ IMX091_8BIT, { 0x30DB }, 0x00},
	{ IMX091_8BIT, { 0x30DC }, 0x00},
	{ IMX091_8BIT, { 0x30DD }, 0x00},
	{ IMX091_8BIT, { 0x30DE }, 0x02},
	{ IMX091_8BIT, { 0x3102 }, 0x10},
	{ IMX091_8BIT, { 0x3103 }, 0x44},
	{ IMX091_8BIT, { 0x3104 }, 0x40},
	{ IMX091_8BIT, { 0x3105 }, 0x00},
	{ IMX091_8BIT, { 0x3106 }, 0x0D},
	{ IMX091_8BIT, { 0x3107 }, 0x01},
	{ IMX091_8BIT, { 0x310A }, 0x0A},
	{ IMX091_8BIT, { 0x315C }, 0x99},
	{ IMX091_8BIT, { 0x315D }, 0x98},
	{ IMX091_8BIT, { 0x316E }, 0x9A},
	{ IMX091_8BIT, { 0x316F }, 0x99},
	{ IMX091_8BIT, { 0x3301 }, 0x03},
	{ IMX091_8BIT, { 0x3304 }, 0x05},
	{ IMX091_8BIT, { 0x3305 }, 0x04},
	{ IMX091_8BIT, { 0x3306 }, 0x12},
	{ IMX091_8BIT, { 0x3307 }, 0x03},
	{ IMX091_8BIT, { 0x3308 }, 0x0D},
	{ IMX091_8BIT, { 0x3309 }, 0x05},
	{ IMX091_8BIT, { 0x330A }, 0x09},
	{ IMX091_8BIT, { 0x330B }, 0x04},
	{ IMX091_8BIT, { 0x330C }, 0x08},
	{ IMX091_8BIT, { 0x330D }, 0x05},
	{ IMX091_8BIT, { 0x330E }, 0x03},
	{ IMX091_8BIT, { 0x3318 }, 0x73},
	{ IMX091_8BIT, { 0x3322 }, 0x02},
	{ IMX091_8BIT, { 0x3342 }, 0x0F},
	{ IMX091_8BIT, { 0x3348 }, 0xE0},
	/* Shutter Gain setting */
	{ IMX091_8BIT, { 0x0202 }, 0x06 },	
	{ IMX091_8BIT, { 0x0203 }, 0x55 },	

	{ IMX091_TOK_TERM, {0}, 0}
};

static const struct imx091_reg imx091_3M_STILL[] = {
	/* PLL setting */
	{ IMX091_8BIT, { 0x0305 }, 0x02},
	{ IMX091_8BIT, { 0x0307 }, 0x39},
	{ IMX091_8BIT, { 0x30A4 }, 0x02},
	{ IMX091_8BIT, { 0x303C }, 0x4B},
	/* Mode setting */	
	{ IMX091_8BIT, { 0x0112 }, 0x0A},
	{ IMX091_8BIT, { 0x0113 }, 0x0A},
	{ IMX091_8BIT, { 0x0340 }, 0x06},
	{ IMX091_8BIT, { 0x0341 }, 0x5A},
	{ IMX091_8BIT, { 0x0342 }, 0x12},
	{ IMX091_8BIT, { 0x0343 }, 0x0C},
	{ IMX091_8BIT, { 0x0344 }, 0x00},
	{ IMX091_8BIT, { 0x0345 }, 0x08},
	{ IMX091_8BIT, { 0x0346 }, 0x00},
	{ IMX091_8BIT, { 0x0347 }, 0x30},
	{ IMX091_8BIT, { 0x0348 }, 0x10},
	{ IMX091_8BIT, { 0x0349 }, 0x77},
	{ IMX091_8BIT, { 0x034A }, 0x0C},
	{ IMX091_8BIT, { 0x034B }, 0x5F},
	{ IMX091_8BIT, { 0x034C }, 0x08},
	{ IMX091_8BIT, { 0x034D }, 0x38},
	{ IMX091_8BIT, { 0x034E }, 0x06},
	{ IMX091_8BIT, { 0x034F }, 0x18},
	{ IMX091_8BIT, { 0x0381 }, 0x01},
	{ IMX091_8BIT, { 0x0383 }, 0x03},
	{ IMX091_8BIT, { 0x0385 }, 0x01},
	{ IMX091_8BIT, { 0x0387 }, 0x03},
	{ IMX091_8BIT, { 0x3033 }, 0x00},
	{ IMX091_8BIT, { 0x303D }, 0x10},
	{ IMX091_8BIT, { 0x303E }, 0xD0},
	{ IMX091_8BIT, { 0x3040 }, 0x00},
	{ IMX091_8BIT, { 0x3041 }, 0x00},
	{ IMX091_8BIT, { 0x30B0 }, 0x00},
	{ IMX091_8BIT, { 0x3048 }, 0x01},
	{ IMX091_8BIT, { 0x304C }, 0x7F},
	{ IMX091_8BIT, { 0x304D }, 0x04},
	{ IMX091_8BIT, { 0x3064 }, 0x12},
	{ IMX091_8BIT, { 0x309B }, 0x28},
	{ IMX091_8BIT, { 0x309E }, 0x00},
	{ IMX091_8BIT, { 0x30A0 }, 0x14},
	{ IMX091_8BIT, { 0x30B2 }, 0x00},
	{ IMX091_8BIT, { 0x30D5 }, 0x09},
	{ IMX091_8BIT, { 0x30D6 }, 0x01},
	{ IMX091_8BIT, { 0x30D7 }, 0x01},
	{ IMX091_8BIT, { 0x30D8 }, 0x64},
	{ IMX091_8BIT, { 0x30D9 }, 0x89},
	{ IMX091_8BIT, { 0x30DA }, 0x00},
	{ IMX091_8BIT, { 0x30DB }, 0x00},
	{ IMX091_8BIT, { 0x30DC }, 0x00},
	{ IMX091_8BIT, { 0x30DD }, 0x00},
	{ IMX091_8BIT, { 0x30DE }, 0x02},
	{ IMX091_8BIT, { 0x3102 }, 0x10},
	{ IMX091_8BIT, { 0x3103 }, 0x44},
	{ IMX091_8BIT, { 0x3104 }, 0x40},
	{ IMX091_8BIT, { 0x3105 }, 0x00},
	{ IMX091_8BIT, { 0x3106 }, 0x0D},
	{ IMX091_8BIT, { 0x3107 }, 0x01},
	{ IMX091_8BIT, { 0x310A }, 0x0A},
	{ IMX091_8BIT, { 0x315C }, 0x99},
	{ IMX091_8BIT, { 0x315D }, 0x98},
	{ IMX091_8BIT, { 0x316E }, 0x9A},
	{ IMX091_8BIT, { 0x316F }, 0x99},
	{ IMX091_8BIT, { 0x3301 }, 0x03},
	{ IMX091_8BIT, { 0x3304 }, 0x05},
	{ IMX091_8BIT, { 0x3305 }, 0x04},
	{ IMX091_8BIT, { 0x3306 }, 0x12},
	{ IMX091_8BIT, { 0x3307 }, 0x03},
	{ IMX091_8BIT, { 0x3308 }, 0x0D},
	{ IMX091_8BIT, { 0x3309 }, 0x05},
	{ IMX091_8BIT, { 0x330A }, 0x09},
	{ IMX091_8BIT, { 0x330B }, 0x04},
	{ IMX091_8BIT, { 0x330C }, 0x08},
	{ IMX091_8BIT, { 0x330D }, 0x05},
	{ IMX091_8BIT, { 0x330E }, 0x03},
	{ IMX091_8BIT, { 0x3318 }, 0x73},
	{ IMX091_8BIT, { 0x3322 }, 0x02},
	{ IMX091_8BIT, { 0x3342 }, 0x0F},
	{ IMX091_8BIT, { 0x3348 }, 0xE0},
	/* Shutter Gain setting */
	{ IMX091_8BIT, { 0x0202 }, 0x06},
	{ IMX091_8BIT, { 0x0203 }, 0x55},
	
	{ IMX091_TOK_TERM, {0}, 0}
};

static const struct imx091_reg imx091_5M_STILL[] = {
	/* PLL setting */
	{ IMX091_8BIT, { 0x0305 }, 0x02},
	{ IMX091_8BIT, { 0x0307 }, 0x39},
	{ IMX091_8BIT, { 0x30A4 }, 0x02},
	{ IMX091_8BIT, { 0x303C }, 0x4B},
	/* Mode setting */	
	{ IMX091_8BIT, { 0x0112 }, 0x0A},
	{ IMX091_8BIT, { 0x0113 }, 0x0A},
	{ IMX091_8BIT, { 0x0340 }, 0x08},
	{ IMX091_8BIT, { 0x0341 }, 0x34},
	{ IMX091_8BIT, { 0x0342 }, 0x15},
	{ IMX091_8BIT, { 0x0343 }, 0xE0},
	{ IMX091_8BIT, { 0x0344 }, 0x03},
	{ IMX091_8BIT, { 0x0345 }, 0x38},
	{ IMX091_8BIT, { 0x0346 }, 0x02},
	{ IMX091_8BIT, { 0x0347 }, 0x80},
	{ IMX091_8BIT, { 0x0348 }, 0x0D},
	{ IMX091_8BIT, { 0x0349 }, 0x47},
	{ IMX091_8BIT, { 0x034A }, 0x0A},
	{ IMX091_8BIT, { 0x034B }, 0x0F},
	{ IMX091_8BIT, { 0x034C }, 0x0A},
	{ IMX091_8BIT, { 0x034D }, 0x10},
	{ IMX091_8BIT, { 0x034E }, 0x07},
	{ IMX091_8BIT, { 0x034F }, 0x90},
	{ IMX091_8BIT, { 0x0381 }, 0x01},
	{ IMX091_8BIT, { 0x0383 }, 0x01},
	{ IMX091_8BIT, { 0x0385 }, 0x01},
	{ IMX091_8BIT, { 0x0387 }, 0x01},
	{ IMX091_8BIT, { 0x3033 }, 0x00},
	{ IMX091_8BIT, { 0x303D }, 0x10},
	{ IMX091_8BIT, { 0x303E }, 0xD0},
	{ IMX091_8BIT, { 0x3040 }, 0x00},
	{ IMX091_8BIT, { 0x3041 }, 0x00},
	{ IMX091_8BIT, { 0x30B0 }, 0x00},
	{ IMX091_8BIT, { 0x3048 }, 0x00},
	{ IMX091_8BIT, { 0x304C }, 0x7F},
	{ IMX091_8BIT, { 0x304D }, 0x04},
	{ IMX091_8BIT, { 0x3064 }, 0x12},
	{ IMX091_8BIT, { 0x309B }, 0x20},
	{ IMX091_8BIT, { 0x309E }, 0x00},
	{ IMX091_8BIT, { 0x30A0 }, 0x14},
	{ IMX091_8BIT, { 0x30B2 }, 0x00},
	{ IMX091_8BIT, { 0x30D5 }, 0x00},
	{ IMX091_8BIT, { 0x30D6 }, 0x85},
	{ IMX091_8BIT, { 0x30D7 }, 0x2A},
	{ IMX091_8BIT, { 0x30D8 }, 0x64},
	{ IMX091_8BIT, { 0x30D9 }, 0x89},
	{ IMX091_8BIT, { 0x30DA }, 0x00},
	{ IMX091_8BIT, { 0x30DB }, 0x00},
	{ IMX091_8BIT, { 0x30DC }, 0x00},
	{ IMX091_8BIT, { 0x30DD }, 0x00},
	{ IMX091_8BIT, { 0x30DE }, 0x00},
	{ IMX091_8BIT, { 0x3102 }, 0x10},
	{ IMX091_8BIT, { 0x3103 }, 0x44},
	{ IMX091_8BIT, { 0x3104 }, 0x40},
	{ IMX091_8BIT, { 0x3105 }, 0x00},
	{ IMX091_8BIT, { 0x3106 }, 0x0D},
	{ IMX091_8BIT, { 0x3107 }, 0x01},
	{ IMX091_8BIT, { 0x310A }, 0x0A},
	{ IMX091_8BIT, { 0x315C }, 0x99},
	{ IMX091_8BIT, { 0x315D }, 0x98},
	{ IMX091_8BIT, { 0x316E }, 0x9A},
	{ IMX091_8BIT, { 0x316F }, 0x99},
	{ IMX091_8BIT, { 0x3301 }, 0x03},
	{ IMX091_8BIT, { 0x3304 }, 0x05},
	{ IMX091_8BIT, { 0x3305 }, 0x04},
	{ IMX091_8BIT, { 0x3306 }, 0x12},
	{ IMX091_8BIT, { 0x3307 }, 0x03},
	{ IMX091_8BIT, { 0x3308 }, 0x0D},
	{ IMX091_8BIT, { 0x3309 }, 0x05},
	{ IMX091_8BIT, { 0x330A }, 0x09},
	{ IMX091_8BIT, { 0x330B }, 0x04},
	{ IMX091_8BIT, { 0x330C }, 0x08},
	{ IMX091_8BIT, { 0x330D }, 0x05},
	{ IMX091_8BIT, { 0x330E }, 0x03},
	{ IMX091_8BIT, { 0x3318 }, 0x64},
	{ IMX091_8BIT, { 0x3322 }, 0x02},
	{ IMX091_8BIT, { 0x3342 }, 0x0F},
	{ IMX091_8BIT, { 0x3348 }, 0xE0},
	/* Shutter Gain setting */
	{ IMX091_8BIT, { 0x0202 }, 0x08},
	{ IMX091_8BIT, { 0x0203 }, 0x2F},
	
	{ IMX091_TOK_TERM, {0}, 0}
};

static const struct imx091_reg imx091_8M_STILL[] = {
	/* PLL setting */
	{ IMX091_8BIT, { 0x0305 }, 0x02},
	{ IMX091_8BIT, { 0x0307 }, 0x39},
	{ IMX091_8BIT, { 0x30A4 }, 0x02},
	{ IMX091_8BIT, { 0x303C }, 0x4B},
	/* Mode setting */	
	{ IMX091_8BIT, { 0x0112 }, 0x0A},
	{ IMX091_8BIT, { 0x0113 }, 0x0A},
	{ IMX091_8BIT, { 0x0340 }, 0x0A},
	{ IMX091_8BIT, { 0x0341 }, 0x28},
	{ IMX091_8BIT, { 0x0342 }, 0x17},
	{ IMX091_8BIT, { 0x0343 }, 0xD4},
	{ IMX091_8BIT, { 0x0344 }, 0x01},
	{ IMX091_8BIT, { 0x0345 }, 0xD8},
	{ IMX091_8BIT, { 0x0346 }, 0x01},
	{ IMX091_8BIT, { 0x0347 }, 0x78},
	{ IMX091_8BIT, { 0x0348 }, 0x0E},
	{ IMX091_8BIT, { 0x0349 }, 0xA7},
	{ IMX091_8BIT, { 0x034A }, 0x0B},
	{ IMX091_8BIT, { 0x034B }, 0x17},
	{ IMX091_8BIT, { 0x034C }, 0x0C},
	{ IMX091_8BIT, { 0x034D }, 0xD0},
	{ IMX091_8BIT, { 0x034E }, 0x09},
	{ IMX091_8BIT, { 0x034F }, 0xA0},
	{ IMX091_8BIT, { 0x0381 }, 0x01},
	{ IMX091_8BIT, { 0x0383 }, 0x01},
	{ IMX091_8BIT, { 0x0385 }, 0x01},
	{ IMX091_8BIT, { 0x0387 }, 0x01},
	{ IMX091_8BIT, { 0x3033 }, 0x00},
	{ IMX091_8BIT, { 0x303D }, 0x10},
	{ IMX091_8BIT, { 0x303E }, 0xD0},
	{ IMX091_8BIT, { 0x3040 }, 0x00},
	{ IMX091_8BIT, { 0x3041 }, 0x00},
	{ IMX091_8BIT, { 0x30B0 }, 0x00},
	{ IMX091_8BIT, { 0x3048 }, 0x00},
	{ IMX091_8BIT, { 0x304C }, 0x7F},
	{ IMX091_8BIT, { 0x304D }, 0x04},
	{ IMX091_8BIT, { 0x3064 }, 0x12},
	{ IMX091_8BIT, { 0x309B }, 0x20},
	{ IMX091_8BIT, { 0x309E }, 0x00},
	{ IMX091_8BIT, { 0x30A0 }, 0x14},
	{ IMX091_8BIT, { 0x30B2 }, 0x00},
	{ IMX091_8BIT, { 0x30D5 }, 0x00},
	{ IMX091_8BIT, { 0x30D6 }, 0x85},
	{ IMX091_8BIT, { 0x30D7 }, 0x2A},
	{ IMX091_8BIT, { 0x30D8 }, 0x64},
	{ IMX091_8BIT, { 0x30D9 }, 0x89},
	{ IMX091_8BIT, { 0x30DA }, 0x00},
	{ IMX091_8BIT, { 0x30DB }, 0x00},
	{ IMX091_8BIT, { 0x30DC }, 0x00},
	{ IMX091_8BIT, { 0x30DD }, 0x00},
	{ IMX091_8BIT, { 0x30DE }, 0x00},
	{ IMX091_8BIT, { 0x3102 }, 0x10},
	{ IMX091_8BIT, { 0x3103 }, 0x44},
	{ IMX091_8BIT, { 0x3104 }, 0x40},
	{ IMX091_8BIT, { 0x3105 }, 0x00},
	{ IMX091_8BIT, { 0x3106 }, 0x0D},
	{ IMX091_8BIT, { 0x3107 }, 0x01},
	{ IMX091_8BIT, { 0x310A }, 0x0A},
	{ IMX091_8BIT, { 0x315C }, 0x99},
	{ IMX091_8BIT, { 0x315D }, 0x98},
	{ IMX091_8BIT, { 0x316E }, 0x9A},
	{ IMX091_8BIT, { 0x316F }, 0x99},
	{ IMX091_8BIT, { 0x3301 }, 0x03},
	{ IMX091_8BIT, { 0x3304 }, 0x05},
	{ IMX091_8BIT, { 0x3305 }, 0x04},
	{ IMX091_8BIT, { 0x3306 }, 0x12},
	{ IMX091_8BIT, { 0x3307 }, 0x03},
	{ IMX091_8BIT, { 0x3308 }, 0x0D},
	{ IMX091_8BIT, { 0x3309 }, 0x05},
	{ IMX091_8BIT, { 0x330A }, 0x09},
	{ IMX091_8BIT, { 0x330B }, 0x04},
	{ IMX091_8BIT, { 0x330C }, 0x08},
	{ IMX091_8BIT, { 0x330D }, 0x05},
	{ IMX091_8BIT, { 0x330E }, 0x03},
	{ IMX091_8BIT, { 0x3318 }, 0x64},
	{ IMX091_8BIT, { 0x3322 }, 0x02},
	{ IMX091_8BIT, { 0x3342 }, 0x0F},
	{ IMX091_8BIT, { 0x3348 }, 0xE0},
	  /* Shutter Gain setting */
	{ IMX091_8BIT, { 0x0202 }, 0x0A},
	{ IMX091_8BIT, { 0x0203 }, 0x23},
	                        
	{ IMX091_TOK_TERM, {0}, 0}
};

static const struct imx091_reg imx091_13M_STILL[] = {
	/* PLL setting */
	{ IMX091_8BIT, { 0x0305 }, 0x02},
	{ IMX091_8BIT, { 0x0307 }, 0x39},
	{ IMX091_8BIT, { 0x30A4 }, 0x02},
	{ IMX091_8BIT, { 0x303C }, 0x4B},
	/* Mode setting */	
	{ IMX091_8BIT, { 0x0112 }, 0x0A},
	{ IMX091_8BIT, { 0x0113 }, 0x0A},
	{ IMX091_8BIT, { 0x0340 }, 0x0C},
	{ IMX091_8BIT, { 0x0341 }, 0xB8},
	{ IMX091_8BIT, { 0x0342 }, 0x1B},
	{ IMX091_8BIT, { 0x0343 }, 0xBC},
	{ IMX091_8BIT, { 0x0344 }, 0x00},
	{ IMX091_8BIT, { 0x0345 }, 0x08},
	{ IMX091_8BIT, { 0x0346 }, 0x00},
	{ IMX091_8BIT, { 0x0347 }, 0x30},
	{ IMX091_8BIT, { 0x0348 }, 0x10},
	{ IMX091_8BIT, { 0x0349 }, 0x77},
	{ IMX091_8BIT, { 0x034A }, 0x0C},
	{ IMX091_8BIT, { 0x034B }, 0x5F},
	{ IMX091_8BIT, { 0x034C }, 0x10},
	{ IMX091_8BIT, { 0x034D }, 0x70},
	{ IMX091_8BIT, { 0x034E }, 0x0C},
	{ IMX091_8BIT, { 0x034F }, 0x30},
	{ IMX091_8BIT, { 0x0381 }, 0x01},
	{ IMX091_8BIT, { 0x0383 }, 0x01},
	{ IMX091_8BIT, { 0x0385 }, 0x01},
	{ IMX091_8BIT, { 0x0387 }, 0x01},
	{ IMX091_8BIT, { 0x3033 }, 0x00},
	{ IMX091_8BIT, { 0x303D }, 0x10},
	{ IMX091_8BIT, { 0x303E }, 0xD0},
	{ IMX091_8BIT, { 0x3040 }, 0x00},
	{ IMX091_8BIT, { 0x3041 }, 0x00},
	{ IMX091_8BIT, { 0x30B0 }, 0x00},
	{ IMX091_8BIT, { 0x3048 }, 0x00},
	{ IMX091_8BIT, { 0x304C }, 0x7F},
	{ IMX091_8BIT, { 0x304D }, 0x04},
	{ IMX091_8BIT, { 0x3064 }, 0x12},
	{ IMX091_8BIT, { 0x309B }, 0x20},
	{ IMX091_8BIT, { 0x309E }, 0x00},
	{ IMX091_8BIT, { 0x30A0 }, 0x14},
	{ IMX091_8BIT, { 0x30B2 }, 0x00},
	{ IMX091_8BIT, { 0x30D5 }, 0x00},
	{ IMX091_8BIT, { 0x30D6 }, 0x85},
	{ IMX091_8BIT, { 0x30D7 }, 0x2A},
	{ IMX091_8BIT, { 0x30D8 }, 0x64},
	{ IMX091_8BIT, { 0x30D9 }, 0x89},
	{ IMX091_8BIT, { 0x30DA }, 0x00},
	{ IMX091_8BIT, { 0x30DB }, 0x00},
	{ IMX091_8BIT, { 0x30DC }, 0x00},
	{ IMX091_8BIT, { 0x30DD }, 0x00},
	{ IMX091_8BIT, { 0x30DE }, 0x00},
	{ IMX091_8BIT, { 0x3102 }, 0x10},
	{ IMX091_8BIT, { 0x3103 }, 0x44},
	{ IMX091_8BIT, { 0x3104 }, 0x40},
	{ IMX091_8BIT, { 0x3105 }, 0x00},
	{ IMX091_8BIT, { 0x3106 }, 0x0D},
	{ IMX091_8BIT, { 0x3107 }, 0x01},
	{ IMX091_8BIT, { 0x310A }, 0x0A},
	{ IMX091_8BIT, { 0x315C }, 0x99},
	{ IMX091_8BIT, { 0x315D }, 0x98},
	{ IMX091_8BIT, { 0x316E }, 0x9A},
	{ IMX091_8BIT, { 0x316F }, 0x99},
	{ IMX091_8BIT, { 0x3301 }, 0x03},
	{ IMX091_8BIT, { 0x3304 }, 0x05},
	{ IMX091_8BIT, { 0x3305 }, 0x04},
	{ IMX091_8BIT, { 0x3306 }, 0x12},
	{ IMX091_8BIT, { 0x3307 }, 0x03},
	{ IMX091_8BIT, { 0x3308 }, 0x0D},
	{ IMX091_8BIT, { 0x3309 }, 0x05},
	{ IMX091_8BIT, { 0x330A }, 0x09},
	{ IMX091_8BIT, { 0x330B }, 0x04},
	{ IMX091_8BIT, { 0x330C }, 0x08},
	{ IMX091_8BIT, { 0x330D }, 0x05},
	{ IMX091_8BIT, { 0x330E }, 0x03},
	{ IMX091_8BIT, { 0x3318 }, 0x64},
	{ IMX091_8BIT, { 0x3322 }, 0x02},
	{ IMX091_8BIT, { 0x3342 }, 0x0F},
	{ IMX091_8BIT, { 0x3348 }, 0xE0},
	/* Shutter Gain setting */
	{ IMX091_8BIT, { 0x0202 }, 0x0C},
	{ IMX091_8BIT, { 0x0203 }, 0xB3},
	{ IMX091_TOK_TERM, {0}, 0}
};

/*****************************IMX091 PREVIEW********************************/

static struct imx091_reg const imx091_PREVIEW_848x616[] = {
	/* PLL setting */	
	{ IMX091_8BIT, { 0x0305 }, 0x02 },	
	{ IMX091_8BIT, { 0x0307 }, 0x3B },	
	{ IMX091_8BIT, { 0x30A4 }, 0x02 },	
	{ IMX091_8BIT, { 0x303C }, 0x4B },
	/* Mode setting */	
	{ IMX091_8BIT, { 0x0340 }, 0x06 },	
	{ IMX091_8BIT, { 0x0341 }, 0x5A },	
	{ IMX091_8BIT, { 0x0342 }, 0x12 },	
	{ IMX091_8BIT, { 0x0343 }, 0x0C },	
	{ IMX091_8BIT, { 0x0344 }, 0x01 },	
	{ IMX091_8BIT, { 0x0345 }, 0xa0 },		
	{ IMX091_8BIT, { 0x0346 }, 0x01 },	
	{ IMX091_8BIT, { 0x0347 }, 0x78 },	
	{ IMX091_8BIT, { 0x0348 }, 0x0E },	
	{ IMX091_8BIT, { 0x0349 }, 0xDF },	
	{ IMX091_8BIT, { 0x034A }, 0x0B },	
	{ IMX091_8BIT, { 0x034B }, 0x17 },	
	{ IMX091_8BIT, { 0x034C }, 0x03 },	
	{ IMX091_8BIT, { 0x034D }, 0x50 },	
	{ IMX091_8BIT, { 0x034E }, 0x02 },	
	{ IMX091_8BIT, { 0x034F }, 0x68 },	
	{ IMX091_8BIT, { 0x0381 }, 0x05 },	
	{ IMX091_8BIT, { 0x0383 }, 0x03 },	
	{ IMX091_8BIT, { 0x0385 }, 0x05 },	
	{ IMX091_8BIT, { 0x0387 }, 0x03 },	
	{ IMX091_8BIT, { 0x3033 }, 0x00 },	
	{ IMX091_8BIT, { 0x303D }, 0x10 },	
	{ IMX091_8BIT, { 0x303E }, 0xD0 },	
	{ IMX091_8BIT, { 0x3040 }, 0x00 },	
	{ IMX091_8BIT, { 0x3041 }, 0x00 },	
	{ IMX091_8BIT, { 0x30B0 }, 0x00 },
	{ IMX091_8BIT, { 0x3048 }, 0x01 },	
	{ IMX091_8BIT, { 0x304C }, 0x7F },	
	{ IMX091_8BIT, { 0x304D }, 0x04 },	
	{ IMX091_8BIT, { 0x3064 }, 0x12 },	
	{ IMX091_8BIT, { 0x309B }, 0x28 },	
	{ IMX091_8BIT, { 0x309E }, 0x00 },	
	{ IMX091_8BIT, { 0x30A0 }, 0x14 },	
	{ IMX091_8BIT, { 0x30B2 }, 0x00 },	
	{ IMX091_8BIT, { 0x30D5 }, 0x09 },	
	{ IMX091_8BIT, { 0x30D6 }, 0x00 },	
	{ IMX091_8BIT, { 0x30D7 }, 0x00 },	
	{ IMX091_8BIT, { 0x30D8 }, 0x00 },	
	{ IMX091_8BIT, { 0x30D9 }, 0x00 },	
	{ IMX091_8BIT, { 0x30DA }, 0x00 },	
	{ IMX091_8BIT, { 0x30DB }, 0x00 },	
	{ IMX091_8BIT, { 0x30DC }, 0x00 },	
	{ IMX091_8BIT, { 0x30DD }, 0x00 },	
	{ IMX091_8BIT, { 0x30DE }, 0x04 },	
	{ IMX091_8BIT, { 0x3102 }, 0x10 },	
	{ IMX091_8BIT, { 0x3103 }, 0x44 },	
	{ IMX091_8BIT, { 0x3104 }, 0x40 },	
	{ IMX091_8BIT, { 0x3105 }, 0x00 },	
	{ IMX091_8BIT, { 0x3106 }, 0x0D },	
	{ IMX091_8BIT, { 0x3107 }, 0x01 },	
	{ IMX091_8BIT, { 0x310A }, 0x0A },	
	{ IMX091_8BIT, { 0x315C }, 0x99 },	
	{ IMX091_8BIT, { 0x315D }, 0x98 },	
	{ IMX091_8BIT, { 0x316E }, 0x9A },	
	{ IMX091_8BIT, { 0x316F }, 0x99 },	
	{ IMX091_8BIT, { 0x3301 }, 0x03},	
	{ IMX091_8BIT, { 0x3304 }, 0x05 },	
	{ IMX091_8BIT, { 0x3305 }, 0x04 },	
	{ IMX091_8BIT, { 0x3306 }, 0x12 },	
	{ IMX091_8BIT, { 0x3307 }, 0x03 },	
	{ IMX091_8BIT, { 0x3308 }, 0x0D },	
	{ IMX091_8BIT, { 0x3309 }, 0x05 },	
	{ IMX091_8BIT, { 0x330A }, 0x09 },	
	{ IMX091_8BIT, { 0x330B }, 0x04 },	
	{ IMX091_8BIT, { 0x330C }, 0x08 },	
	{ IMX091_8BIT, { 0x330D }, 0x05 },	
	{ IMX091_8BIT, { 0x330E }, 0x03 },	
	{ IMX091_8BIT, { 0x3318 }, 0x7A },	
	{ IMX091_8BIT, { 0x3322 }, 0x02 },	
	{ IMX091_8BIT, { 0x3342 }, 0x0F },	
	{ IMX091_8BIT, { 0x3348 }, 0xE0 },
	/* Shutter Gain setting */
	{ IMX091_8BIT, { 0x0202 }, 0x06 },	
	{ IMX091_8BIT, { 0x0203 }, 0x55 },	
	{ IMX091_TOK_TERM, {0}, 0}
};

static struct imx091_reg const imx091_PREVIEW_WIDE_PREVIEW[] = {
	{ IMX091_8BIT, { 0x0305 }, 0x02},
	{ IMX091_8BIT, { 0x0307 }, 0x39},
	{ IMX091_8BIT, { 0x30A4 }, 0x02},
	{ IMX091_8BIT, { 0x303C }, 0x4B},
	/* Mode setting */	
	{ IMX091_8BIT, { 0x0112 }, 0x0A},
	{ IMX091_8BIT, { 0x0113 }, 0x0A},
	{ IMX091_8BIT, { 0x0340 }, 0x06},
	{ IMX091_8BIT, { 0x0341 }, 0x5A},
	{ IMX091_8BIT, { 0x0342 }, 0x12},
	{ IMX091_8BIT, { 0x0343 }, 0x0C},
	{ IMX091_8BIT, { 0x0344 }, 0x00},
	{ IMX091_8BIT, { 0x0345 }, 0xA8},
	{ IMX091_8BIT, { 0x0346 }, 0x01},
	{ IMX091_8BIT, { 0x0347 }, 0xF8},
	{ IMX091_8BIT, { 0x0348 }, 0x0F},
	{ IMX091_8BIT, { 0x0349 }, 0xD7},
	{ IMX091_8BIT, { 0x034A }, 0x0A},
	{ IMX091_8BIT, { 0x034B }, 0x97},
	{ IMX091_8BIT, { 0x034C }, 0x05},
	{ IMX091_8BIT, { 0x034D }, 0x10},
	{ IMX091_8BIT, { 0x034E }, 0x02},
	{ IMX091_8BIT, { 0x034F }, 0xE0},
	{ IMX091_8BIT, { 0x0381 }, 0x03},
	{ IMX091_8BIT, { 0x0383 }, 0x03},
	{ IMX091_8BIT, { 0x0385 }, 0x03},
	{ IMX091_8BIT, { 0x0387 }, 0x03},
	{ IMX091_8BIT, { 0x3033 }, 0x00},
	{ IMX091_8BIT, { 0x303D }, 0x10},
	{ IMX091_8BIT, { 0x303E }, 0xD0},
	{ IMX091_8BIT, { 0x3040 }, 0x00},
	{ IMX091_8BIT, { 0x3041 }, 0x00},
	{ IMX091_8BIT, { 0x30B0 }, 0x00},
	{ IMX091_8BIT, { 0x3048 }, 0x22},
	{ IMX091_8BIT, { 0x304C }, 0x7F},
	{ IMX091_8BIT, { 0x304D }, 0x04},
	{ IMX091_8BIT, { 0x3064 }, 0x12},
	{ IMX091_8BIT, { 0x309B }, 0x60},
	{ IMX091_8BIT, { 0x309E }, 0x04},
	{ IMX091_8BIT, { 0x30A0 }, 0x14},
	{ IMX091_8BIT, { 0x30B2 }, 0x00},
	{ IMX091_8BIT, { 0x30D5 }, 0x09},
	{ IMX091_8BIT, { 0x30D6 }, 0x00},
	{ IMX091_8BIT, { 0x30D7 }, 0x00},
	{ IMX091_8BIT, { 0x30D8 }, 0x00},
	{ IMX091_8BIT, { 0x30D9 }, 0x89},
	{ IMX091_8BIT, { 0x30DA }, 0x00},
	{ IMX091_8BIT, { 0x30DB }, 0x00},
	{ IMX091_8BIT, { 0x30DC }, 0x00},
	{ IMX091_8BIT, { 0x30DD }, 0x00},
	{ IMX091_8BIT, { 0x30DE }, 0x03},
	{ IMX091_8BIT, { 0x3102 }, 0x09},
	{ IMX091_8BIT, { 0x3103 }, 0x23},
	{ IMX091_8BIT, { 0x3104 }, 0x24},
	{ IMX091_8BIT, { 0x3105 }, 0x00},
	{ IMX091_8BIT, { 0x3106 }, 0x8B},
	{ IMX091_8BIT, { 0x3107 }, 0x00},
	{ IMX091_8BIT, { 0x310A }, 0x0A},
	{ IMX091_8BIT, { 0x315C }, 0x4A},
	{ IMX091_8BIT, { 0x315D }, 0x49},
	{ IMX091_8BIT, { 0x316E }, 0x4B},
	{ IMX091_8BIT, { 0x316F }, 0x4A},
	{ IMX091_8BIT, { 0x3301 }, 0x03},
	{ IMX091_8BIT, { 0x3304 }, 0x05},
	{ IMX091_8BIT, { 0x3305 }, 0x04},
	{ IMX091_8BIT, { 0x3306 }, 0x12},
	{ IMX091_8BIT, { 0x3307 }, 0x03},
	{ IMX091_8BIT, { 0x3308 }, 0x0D},
	{ IMX091_8BIT, { 0x3309 }, 0x05},
	{ IMX091_8BIT, { 0x330A }, 0x09},
	{ IMX091_8BIT, { 0x330B }, 0x04},
	{ IMX091_8BIT, { 0x330C }, 0x08},
	{ IMX091_8BIT, { 0x330D }, 0x05},
	{ IMX091_8BIT, { 0x330E }, 0x03},
	{ IMX091_8BIT, { 0x3318 }, 0x6A},
	{ IMX091_8BIT, { 0x3322 }, 0x02},
	{ IMX091_8BIT, { 0x3342 }, 0x0F},
	{ IMX091_8BIT, { 0x3348 }, 0xE0},
  	/*  Shutter Gain setting */
	{ IMX091_8BIT, { 0x0202 }, 0x06},
	{ IMX091_8BIT, { 0x0203 }, 0x55},
	
	{ IMX091_TOK_TERM, {0}, 0}
};

/***************** IMX091 VIDEO ***************************************/

static const struct imx091_reg imx091_QCIF_strong_dvs[] = {
	 /* PLL setting */
	{ IMX091_8BIT, { 0x0305 }, 0x02},
	{ IMX091_8BIT, { 0x0307 }, 0x39},
	{ IMX091_8BIT, { 0x30A4 }, 0x02},
	{ IMX091_8BIT, { 0x303C }, 0x4B},
	/* Mode setting */	
	{ IMX091_8BIT, { 0x0112 }, 0x0A},
	{ IMX091_8BIT, { 0x0113 }, 0x0A},
	{ IMX091_8BIT, { 0x0340 }, 0x06},
	{ IMX091_8BIT, { 0x0341 }, 0x5A},
	{ IMX091_8BIT, { 0x0342 }, 0x12},
	{ IMX091_8BIT, { 0x0343 }, 0x0C},
	{ IMX091_8BIT, { 0x0344 }, 0x05},
	{ IMX091_8BIT, { 0x0345 }, 0x40},
	{ IMX091_8BIT, { 0x0346 }, 0x03},
	{ IMX091_8BIT, { 0x0347 }, 0xC8},
	{ IMX091_8BIT, { 0x0348 }, 0x0B},
	{ IMX091_8BIT, { 0x0349 }, 0x3F},
	{ IMX091_8BIT, { 0x034A }, 0x08},
	{ IMX091_8BIT, { 0x034B }, 0xC7},
  	{ IMX091_8BIT, { 0x034C }, 0x00},
  	{ IMX091_8BIT, { 0x034D }, 0xC0},
	{ IMX091_8BIT, { 0x034E }, 0x00},
	{ IMX091_8BIT, { 0x034F }, 0xA0},
	{ IMX091_8BIT, { 0x0381 }, 0x09},
	{ IMX091_8BIT, { 0x0383 }, 0x07},
	{ IMX091_8BIT, { 0x0385 }, 0x09},
	{ IMX091_8BIT, { 0x0387 }, 0x07},
	{ IMX091_8BIT, { 0x3033 }, 0x00},
	{ IMX091_8BIT, { 0x303D }, 0x10},
	{ IMX091_8BIT, { 0x303E }, 0xD0},
	{ IMX091_8BIT, { 0x3040 }, 0x00},
	{ IMX091_8BIT, { 0x3041 }, 0x00},
	{ IMX091_8BIT, { 0x30B0 }, 0x00},
	{ IMX091_8BIT, { 0x3048 }, 0x01},
	{ IMX091_8BIT, { 0x304C }, 0x7F},
	{ IMX091_8BIT, { 0x304D }, 0x04},
	{ IMX091_8BIT, { 0x3064 }, 0x12},
	{ IMX091_8BIT, { 0x309B }, 0x48},
	{ IMX091_8BIT, { 0x309E }, 0x04},
	{ IMX091_8BIT, { 0x30A0 }, 0x14},
	{ IMX091_8BIT, { 0x30B2 }, 0x00},
	{ IMX091_8BIT, { 0x30D5 }, 0x09},
	{ IMX091_8BIT, { 0x30D6 }, 0x00},
	{ IMX091_8BIT, { 0x30D7 }, 0x00},
	{ IMX091_8BIT, { 0x30D8 }, 0x00},
	{ IMX091_8BIT, { 0x30D9 }, 0x00},
	{ IMX091_8BIT, { 0x30DA }, 0x00},
	{ IMX091_8BIT, { 0x30DB }, 0x00},
	{ IMX091_8BIT, { 0x30DC }, 0x00},
	{ IMX091_8BIT, { 0x30DD }, 0x00},
	{ IMX091_8BIT, { 0x30DE }, 0x08},
	{ IMX091_8BIT, { 0x3102 }, 0x10},
	{ IMX091_8BIT, { 0x3103 }, 0x44},
	{ IMX091_8BIT, { 0x3104 }, 0x40},
	{ IMX091_8BIT, { 0x3105 }, 0x00},
	{ IMX091_8BIT, { 0x3106 }, 0x0D},
	{ IMX091_8BIT, { 0x3107 }, 0x01},
	{ IMX091_8BIT, { 0x310A }, 0x0A},
	{ IMX091_8BIT, { 0x315C }, 0x99},
	{ IMX091_8BIT, { 0x315D }, 0x98},
	{ IMX091_8BIT, { 0x316E }, 0x9A},
	{ IMX091_8BIT, { 0x316F }, 0x99},
	{ IMX091_8BIT, { 0x3301 }, 0x03},
	{ IMX091_8BIT, { 0x3304 }, 0x05},
	{ IMX091_8BIT, { 0x3305 }, 0x04},
	{ IMX091_8BIT, { 0x3306 }, 0x12},
	{ IMX091_8BIT, { 0x3307 }, 0x03},
	{ IMX091_8BIT, { 0x3308 }, 0x0D},
	{ IMX091_8BIT, { 0x3309 }, 0x05},
	{ IMX091_8BIT, { 0x330A }, 0x09},
	{ IMX091_8BIT, { 0x330B }, 0x04},
	{ IMX091_8BIT, { 0x330C }, 0x08},
	{ IMX091_8BIT, { 0x330D }, 0x05},
	{ IMX091_8BIT, { 0x330E }, 0x03},
	{ IMX091_8BIT, { 0x3318 }, 0x78},
	{ IMX091_8BIT, { 0x3322 }, 0x02},
	{ IMX091_8BIT, { 0x3342 }, 0x0F},
	{ IMX091_8BIT, { 0x3348 }, 0xE0},
	/* Shutter Gain setting */
	{ IMX091_8BIT, { 0x0202 }, 0x06},
	{ IMX091_8BIT, { 0x0203 }, 0x55},
	{ IMX091_TOK_TERM, {0}, 0}
};

static const struct imx091_reg imx091_QVGA_strong_dvs[] = {
	/* PLL setting */
	{ IMX091_8BIT, { 0x0305 }, 0x02},
	{ IMX091_8BIT, { 0x0307 }, 0x39},
	{ IMX091_8BIT, { 0x30A4 }, 0x02},
	{ IMX091_8BIT, { 0x303C }, 0x4B},

	/* Mode setting */	
	{ IMX091_8BIT, { 0x0112 }, 0x0A},
	{ IMX091_8BIT, { 0x0113 }, 0x0A},
	{ IMX091_8BIT, { 0x0340 }, 0x06},
	{ IMX091_8BIT, { 0x0341 }, 0x5A},
	{ IMX091_8BIT, { 0x0342 }, 0x12},
	{ IMX091_8BIT, { 0x0343 }, 0x0C},
	{ IMX091_8BIT, { 0x0344 }, 0x03},
	{ IMX091_8BIT, { 0x0345 }, 0x00},
	{ IMX091_8BIT, { 0x0346 }, 0x02},
	{ IMX091_8BIT, { 0x0347 }, 0x48},
	{ IMX091_8BIT, { 0x0348 }, 0x0D},
	{ IMX091_8BIT, { 0x0349 }, 0x7F},
	{ IMX091_8BIT, { 0x034A }, 0x0A},
	{ IMX091_8BIT, { 0x034B }, 0x47},
	{ IMX091_8BIT, { 0x034C }, 0x01},
	{ IMX091_8BIT, { 0x034D }, 0x50},
	{ IMX091_8BIT, { 0x034E }, 0x01},
	{ IMX091_8BIT, { 0x034F }, 0x00},
	{ IMX091_8BIT, { 0x0381 }, 0x09},
	{ IMX091_8BIT, { 0x0383 }, 0x07},
	{ IMX091_8BIT, { 0x0385 }, 0x09},
	{ IMX091_8BIT, { 0x0387 }, 0x07},
	{ IMX091_8BIT, { 0x3033 }, 0x00},
	{ IMX091_8BIT, { 0x303D }, 0x10},
	{ IMX091_8BIT, { 0x303E }, 0xD0},
	{ IMX091_8BIT, { 0x3040 }, 0x00},
	{ IMX091_8BIT, { 0x3041 }, 0x00},
	{ IMX091_8BIT, { 0x30B0 }, 0x00},
	{ IMX091_8BIT, { 0x3048 }, 0x01},
	{ IMX091_8BIT, { 0x304C }, 0x7F},
	{ IMX091_8BIT, { 0x304D }, 0x04},
	{ IMX091_8BIT, { 0x3064 }, 0x12},
	{ IMX091_8BIT, { 0x309B }, 0x28},
	{ IMX091_8BIT, { 0x309E }, 0x00},
	{ IMX091_8BIT, { 0x30A0 }, 0x14},
	{ IMX091_8BIT, { 0x30B2 }, 0x00},
	{ IMX091_8BIT, { 0x30D5 }, 0x09},
	{ IMX091_8BIT, { 0x30D6 }, 0x00},
	{ IMX091_8BIT, { 0x30D7 }, 0x00},
	{ IMX091_8BIT, { 0x30D8 }, 0x00},
	{ IMX091_8BIT, { 0x30D9 }, 0x00},
	{ IMX091_8BIT, { 0x30DA }, 0x00},
	{ IMX091_8BIT, { 0x30DB }, 0x00},
	{ IMX091_8BIT, { 0x30DC }, 0x00},
	{ IMX091_8BIT, { 0x30DD }, 0x00},
	{ IMX091_8BIT, { 0x30DE }, 0x08},
	{ IMX091_8BIT, { 0x3102 }, 0x10},
	{ IMX091_8BIT, { 0x3103 }, 0x44},
	{ IMX091_8BIT, { 0x3104 }, 0x40},
	{ IMX091_8BIT, { 0x3105 }, 0x00},
	{ IMX091_8BIT, { 0x3106 }, 0x0D},
	{ IMX091_8BIT, { 0x3107 }, 0x01},
	{ IMX091_8BIT, { 0x310A }, 0x0A},
	{ IMX091_8BIT, { 0x315C }, 0x99},
	{ IMX091_8BIT, { 0x315D }, 0x98},
	{ IMX091_8BIT, { 0x316E }, 0x9A},
	{ IMX091_8BIT, { 0x316F }, 0x99},
	{ IMX091_8BIT, { 0x3301 }, 0x03},
	{ IMX091_8BIT, { 0x3304 }, 0x05},
	{ IMX091_8BIT, { 0x3305 }, 0x04},
	{ IMX091_8BIT, { 0x3306 }, 0x12},
	{ IMX091_8BIT, { 0x3307 }, 0x03},
	{ IMX091_8BIT, { 0x3308 }, 0x0D},
	{ IMX091_8BIT, { 0x3309 }, 0x05},
	{ IMX091_8BIT, { 0x330A }, 0x09},
	{ IMX091_8BIT, { 0x330B }, 0x04},
	{ IMX091_8BIT, { 0x330C }, 0x08},
	{ IMX091_8BIT, { 0x330D }, 0x05},
	{ IMX091_8BIT, { 0x330E }, 0x03},
	{ IMX091_8BIT, { 0x3318 }, 0x7B},
	{ IMX091_8BIT, { 0x3322 }, 0x02},
	{ IMX091_8BIT, { 0x3342 }, 0x0F},
	{ IMX091_8BIT, { 0x3348 }, 0xE0},
	/* Shutter Gain setting */
	{ IMX091_8BIT, { 0x0202 }, 0x06},
	{ IMX091_8BIT, { 0x0203 }, 0x55},
	
	{ IMX091_TOK_TERM, {0}, 0}
};

static struct imx091_reg const imx091_480p_strong_dvs[] = {
	/* PLL setting */
	{ IMX091_8BIT, { 0x0305 }, 0x02},
	{ IMX091_8BIT, { 0x0307 }, 0x39}, 
	{ IMX091_8BIT, { 0x30A4 }, 0x02},
	{ IMX091_8BIT, { 0x303C }, 0x4B},
	/* Mode setting */	
	{ IMX091_8BIT, { 0x0112 }, 0x0A},
	{ IMX091_8BIT, { 0x0113 }, 0x0A},
	{ IMX091_8BIT, { 0x0340 }, 0x06},
	{ IMX091_8BIT, { 0x0341 }, 0x5A},
	{ IMX091_8BIT, { 0x0342 }, 0x12},
	{ IMX091_8BIT, { 0x0343 }, 0x0C},
	{ IMX091_8BIT, { 0x0344 }, 0x02},
	{ IMX091_8BIT, { 0x0345 }, 0x20},
	{ IMX091_8BIT, { 0x0346 }, 0x02},
	{ IMX091_8BIT, { 0x0347 }, 0x68},
	{ IMX091_8BIT, { 0x0348 }, 0x0E},
	{ IMX091_8BIT, { 0x0349 }, 0x5F},
	{ IMX091_8BIT, { 0x034A }, 0x0A},
	{ IMX091_8BIT, { 0x034B }, 0x27},
	{ IMX091_8BIT, { 0x034C }, 0x03},
	{ IMX091_8BIT, { 0x034D }, 0x10},
	{ IMX091_8BIT, { 0x034E }, 0x01},
	{ IMX091_8BIT, { 0x034F }, 0xF0},
	{ IMX091_8BIT, { 0x0381 }, 0x05},
	{ IMX091_8BIT, { 0x0383 }, 0x03},
	{ IMX091_8BIT, { 0x0385 }, 0x05},
	{ IMX091_8BIT, { 0x0387 }, 0x03},
	{ IMX091_8BIT, { 0x3033 }, 0x00},
	{ IMX091_8BIT, { 0x303D }, 0x10},
	{ IMX091_8BIT, { 0x303E }, 0xD0},
	{ IMX091_8BIT, { 0x3040 }, 0x00},
	{ IMX091_8BIT, { 0x3041 }, 0x00},
	{ IMX091_8BIT, { 0x30B0 }, 0x00},
	{ IMX091_8BIT, { 0x3048 }, 0x01},
	{ IMX091_8BIT, { 0x304C }, 0x7F},
	{ IMX091_8BIT, { 0x304D }, 0x04},
	{ IMX091_8BIT, { 0x3064 }, 0x12},
	{ IMX091_8BIT, { 0x309B }, 0x48},
	{ IMX091_8BIT, { 0x309E }, 0x04},
	{ IMX091_8BIT, { 0x30A0 }, 0x14},
	{ IMX091_8BIT, { 0x30B2 }, 0x00},
	{ IMX091_8BIT, { 0x30D5 }, 0x09},
	{ IMX091_8BIT, { 0x30D6 }, 0x00},
	{ IMX091_8BIT, { 0x30D7 }, 0x00},
	{ IMX091_8BIT, { 0x30D8 }, 0x00},
	{ IMX091_8BIT, { 0x30D9 }, 0x00},
	{ IMX091_8BIT, { 0x30DA }, 0x00},
	{ IMX091_8BIT, { 0x30DB }, 0x00},
	{ IMX091_8BIT, { 0x30DC }, 0x00},
	{ IMX091_8BIT, { 0x30DD }, 0x00},
	{ IMX091_8BIT, { 0x30DE }, 0x04},
	{ IMX091_8BIT, { 0x3102 }, 0x10},
	{ IMX091_8BIT, { 0x3103 }, 0x44},
	{ IMX091_8BIT, { 0x3104 }, 0x40},
	{ IMX091_8BIT, { 0x3105 }, 0x00},
	{ IMX091_8BIT, { 0x3106 }, 0x0D},
	{ IMX091_8BIT, { 0x3107 }, 0x01},
	{ IMX091_8BIT, { 0x310A }, 0x0A},
	{ IMX091_8BIT, { 0x315C }, 0x99},
	{ IMX091_8BIT, { 0x315D }, 0x98},
	{ IMX091_8BIT, { 0x316E }, 0x9A},
	{ IMX091_8BIT, { 0x316F }, 0x99},
	{ IMX091_8BIT, { 0x3301 }, 0x03},
	{ IMX091_8BIT, { 0x3304 }, 0x05},
	{ IMX091_8BIT, { 0x3305 }, 0x04},
	{ IMX091_8BIT, { 0x3306 }, 0x12},
	{ IMX091_8BIT, { 0x3307 }, 0x03},
	{ IMX091_8BIT, { 0x3308 }, 0x0D},
	{ IMX091_8BIT, { 0x3309 }, 0x05},
	{ IMX091_8BIT, { 0x330A }, 0x09},
	{ IMX091_8BIT, { 0x330B }, 0x04},
	{ IMX091_8BIT, { 0x330C }, 0x08},
	{ IMX091_8BIT, { 0x330D }, 0x05},
	{ IMX091_8BIT, { 0x330E }, 0x03},
	{ IMX091_8BIT, { 0x3318 }, 0x7A},
	{ IMX091_8BIT, { 0x3322 }, 0x02},
	{ IMX091_8BIT, { 0x3342 }, 0x0F},
	{ IMX091_8BIT, { 0x3348 }, 0xE0},
	/* Shutter Gain setting */
	{ IMX091_8BIT, { 0x0202 }, 0x06},
	{ IMX091_8BIT, { 0x0203 }, 0x55},
	
	{ IMX091_TOK_TERM, {0}, 0}
};

static struct imx091_reg const imx091_720p_strong_dvs[] = {
	/* PLL setting */
	{ IMX091_8BIT, { 0x0305 }, 0x02},
	{ IMX091_8BIT, { 0x0307 }, 0x39},
	{ IMX091_8BIT, { 0x30A4 }, 0x02},
	{ IMX091_8BIT, { 0x303C }, 0x4B},

	/* Mode setting */	
	{ IMX091_8BIT, { 0x0112 }, 0x0A},
	{ IMX091_8BIT, { 0x0113 }, 0x0A},
	{ IMX091_8BIT, { 0x0340 }, 0x06},
	{ IMX091_8BIT, { 0x0341 }, 0x5A},
	{ IMX091_8BIT, { 0x0342 }, 0x12},
	{ IMX091_8BIT, { 0x0343 }, 0x0C},
	{ IMX091_8BIT, { 0x0344 }, 0x00},
	{ IMX091_8BIT, { 0x0345 }, 0xA8},
	{ IMX091_8BIT, { 0x0346 }, 0x01},
	{ IMX091_8BIT, { 0x0347 }, 0xF8},
	{ IMX091_8BIT, { 0x0348 }, 0x0F},
	{ IMX091_8BIT, { 0x0349 }, 0xD7},
	{ IMX091_8BIT, { 0x034A }, 0x0A},
	{ IMX091_8BIT, { 0x034B }, 0x97},
	{ IMX091_8BIT, { 0x034C }, 0x05},
	{ IMX091_8BIT, { 0x034D }, 0x10},
	{ IMX091_8BIT, { 0x034E }, 0x02},
	{ IMX091_8BIT, { 0x034F }, 0xE0},
	{ IMX091_8BIT, { 0x0381 }, 0x03},
	{ IMX091_8BIT, { 0x0383 }, 0x03},
	{ IMX091_8BIT, { 0x0385 }, 0x03},
	{ IMX091_8BIT, { 0x0387 }, 0x03},
	{ IMX091_8BIT, { 0x3033 }, 0x00},
	{ IMX091_8BIT, { 0x303D }, 0x10},
	{ IMX091_8BIT, { 0x303E }, 0xD0},
	{ IMX091_8BIT, { 0x3040 }, 0x00},
	{ IMX091_8BIT, { 0x3041 }, 0x00},
	{ IMX091_8BIT, { 0x30B0 }, 0x00},
	{ IMX091_8BIT, { 0x3048 }, 0x22},
	{ IMX091_8BIT, { 0x304C }, 0x7F},
	{ IMX091_8BIT, { 0x304D }, 0x04},
	{ IMX091_8BIT, { 0x3064 }, 0x12},
	{ IMX091_8BIT, { 0x309B }, 0x60},
	{ IMX091_8BIT, { 0x309E }, 0x04},
	{ IMX091_8BIT, { 0x30A0 }, 0x14},
	{ IMX091_8BIT, { 0x30B2 }, 0x00},
	{ IMX091_8BIT, { 0x30D5 }, 0x09},
	{ IMX091_8BIT, { 0x30D6 }, 0x00},
	{ IMX091_8BIT, { 0x30D7 }, 0x00},
	{ IMX091_8BIT, { 0x30D8 }, 0x00},
	{ IMX091_8BIT, { 0x30D9 }, 0x89},
	{ IMX091_8BIT, { 0x30DA }, 0x00},
	{ IMX091_8BIT, { 0x30DB }, 0x00},
	{ IMX091_8BIT, { 0x30DC }, 0x00},
	{ IMX091_8BIT, { 0x30DD }, 0x00},
	{ IMX091_8BIT, { 0x30DE }, 0x03},
	{ IMX091_8BIT, { 0x3102 }, 0x09},
	{ IMX091_8BIT, { 0x3103 }, 0x23},
	{ IMX091_8BIT, { 0x3104 }, 0x24},
	{ IMX091_8BIT, { 0x3105 }, 0x00},
	{ IMX091_8BIT, { 0x3106 }, 0x8B},
	{ IMX091_8BIT, { 0x3107 }, 0x00},
	{ IMX091_8BIT, { 0x310A }, 0x0A},
	{ IMX091_8BIT, { 0x315C }, 0x4A},
	{ IMX091_8BIT, { 0x315D }, 0x49},
	{ IMX091_8BIT, { 0x316E }, 0x4B},
	{ IMX091_8BIT, { 0x316F }, 0x4A},
	{ IMX091_8BIT, { 0x3301 }, 0x03},
	{ IMX091_8BIT, { 0x3304 }, 0x05},
	{ IMX091_8BIT, { 0x3305 }, 0x04},
	{ IMX091_8BIT, { 0x3306 }, 0x12},
	{ IMX091_8BIT, { 0x3307 }, 0x03},
	{ IMX091_8BIT, { 0x3308 }, 0x0D},
	{ IMX091_8BIT, { 0x3309 }, 0x05},
	{ IMX091_8BIT, { 0x330A }, 0x09},
	{ IMX091_8BIT, { 0x330B }, 0x04},
	{ IMX091_8BIT, { 0x330C }, 0x08},
	{ IMX091_8BIT, { 0x330D }, 0x05},
	{ IMX091_8BIT, { 0x330E }, 0x03},
	{ IMX091_8BIT, { 0x3318 }, 0x6A},
	{ IMX091_8BIT, { 0x3322 }, 0x02},
	{ IMX091_8BIT, { 0x3342 }, 0x0F},
	{ IMX091_8BIT, { 0x3348 }, 0xE0},
	/* Shutter Gain setting */
	{ IMX091_8BIT, { 0x0202 }, 0x06},
	{ IMX091_8BIT, { 0x0203 }, 0x55},
	
	{ IMX091_TOK_TERM, {0}, 0}
};

static const struct imx091_reg imx091_1080p_strong_dvs[] = {
	/* PLL setting */
	{ IMX091_8BIT, { 0x0305 }, 0x02},
	{ IMX091_8BIT, { 0x0307 }, 0x39},
	{ IMX091_8BIT, { 0x30A4 }, 0x02},
	{ IMX091_8BIT, { 0x303C }, 0x4B},
	/* Mode setting */	
	{ IMX091_8BIT, { 0x0112 }, 0x0A},
	{ IMX091_8BIT, { 0x0113 }, 0x0A},
	{ IMX091_8BIT, { 0x0340 }, 0x06},
	{ IMX091_8BIT, { 0x0341 }, 0x5A},
	{ IMX091_8BIT, { 0x0342 }, 0x12},
	{ IMX091_8BIT, { 0x0343 }, 0x0C},
	{ IMX091_8BIT, { 0x0344 }, 0x00},
	{ IMX091_8BIT, { 0x0345 }, 0xB0},
	{ IMX091_8BIT, { 0x0346 }, 0x01},
	{ IMX091_8BIT, { 0x0347 }, 0xF8},
	{ IMX091_8BIT, { 0x0348 }, 0x0F},
	{ IMX091_8BIT, { 0x0349 }, 0xCF},
	{ IMX091_8BIT, { 0x034A }, 0x0A},
	{ IMX091_8BIT, { 0x034B }, 0x97},
	{ IMX091_8BIT, { 0x034C }, 0x07},
	{ IMX091_8BIT, { 0x034D }, 0x90},
	{ IMX091_8BIT, { 0x034E }, 0x04},
	{ IMX091_8BIT, { 0x034F }, 0x50},
	{ IMX091_8BIT, { 0x0381 }, 0x01},
	{ IMX091_8BIT, { 0x0383 }, 0x03},
	{ IMX091_8BIT, { 0x0385 }, 0x01},
	{ IMX091_8BIT, { 0x0387 }, 0x03},
	{ IMX091_8BIT, { 0x3033 }, 0x00},
	{ IMX091_8BIT, { 0x303D }, 0x10},
	{ IMX091_8BIT, { 0x303E }, 0xD0},
	{ IMX091_8BIT, { 0x3040 }, 0x00},
	{ IMX091_8BIT, { 0x3041 }, 0x00},
	{ IMX091_8BIT, { 0x30B0 }, 0x00},
	{ IMX091_8BIT, { 0x3048 }, 0x01},
	{ IMX091_8BIT, { 0x304C }, 0x7F},
	{ IMX091_8BIT, { 0x304D }, 0x04},
	{ IMX091_8BIT, { 0x3064 }, 0x12},
	{ IMX091_8BIT, { 0x309B }, 0x28},
	{ IMX091_8BIT, { 0x309E }, 0x00},
	{ IMX091_8BIT, { 0x30A0 }, 0x14},
	{ IMX091_8BIT, { 0x30B2 }, 0x00},
	{ IMX091_8BIT, { 0x30D5 }, 0x09},
	{ IMX091_8BIT, { 0x30D6 }, 0x01},
	{ IMX091_8BIT, { 0x30D7 }, 0x01},
	{ IMX091_8BIT, { 0x30D8 }, 0x64},
	{ IMX091_8BIT, { 0x30D9 }, 0x89},
	{ IMX091_8BIT, { 0x30DA }, 0x00},
	{ IMX091_8BIT, { 0x30DB }, 0x00},
	{ IMX091_8BIT, { 0x30DC }, 0x00},
	{ IMX091_8BIT, { 0x30DD }, 0x00},
	{ IMX091_8BIT, { 0x30DE }, 0x02},
	{ IMX091_8BIT, { 0x3102 }, 0x10},
	{ IMX091_8BIT, { 0x3103 }, 0x44},
	{ IMX091_8BIT, { 0x3104 }, 0x40},
	{ IMX091_8BIT, { 0x3105 }, 0x00},
	{ IMX091_8BIT, { 0x3106 }, 0x0D},
	{ IMX091_8BIT, { 0x3107 }, 0x01},
	{ IMX091_8BIT, { 0x310A }, 0x0A},
	{ IMX091_8BIT, { 0x315C }, 0x99},
	{ IMX091_8BIT, { 0x315D }, 0x98},
	{ IMX091_8BIT, { 0x316E }, 0x9A},
	{ IMX091_8BIT, { 0x316F }, 0x99},
	{ IMX091_8BIT, { 0x3301 }, 0x03},
	{ IMX091_8BIT, { 0x3304 }, 0x05},
	{ IMX091_8BIT, { 0x3305 }, 0x04},
	{ IMX091_8BIT, { 0x3306 }, 0x12},
	{ IMX091_8BIT, { 0x3307 }, 0x03},
	{ IMX091_8BIT, { 0x3308 }, 0x0D},
	{ IMX091_8BIT, { 0x3309 }, 0x05},
	{ IMX091_8BIT, { 0x330A }, 0x09},
	{ IMX091_8BIT, { 0x330B }, 0x04},
	{ IMX091_8BIT, { 0x330C }, 0x08},
	{ IMX091_8BIT, { 0x330D }, 0x05},
	{ IMX091_8BIT, { 0x330E }, 0x03},
	{ IMX091_8BIT, { 0x3318 }, 0x73},
	{ IMX091_8BIT, { 0x3322 }, 0x02},
	{ IMX091_8BIT, { 0x3342 }, 0x0F},
	{ IMX091_8BIT, { 0x3348 }, 0xE0},
	/* Shutter Gain setting */
	{ IMX091_8BIT, { 0x0202 }, 0x06},
	{ IMX091_8BIT, { 0x0203 }, 0x55},
	
	{ IMX091_TOK_TERM, {0}, 0}
};

static struct imx091_resolution imx091_res_preview[] = {
	{
		 .desc = "imx091_cont_cap_qvga",
		 .width = 336,
		 .height = 256,
		 .used = 0,
		 .regs = imx091_QVGA_strong_dvs,
		 .bin_factor_x = 4,
		 .bin_factor_y = 4,
		 .skip_frames = 0,
		 .fps_options = {
			{
				.fps = 30,
				.pixels_per_line = 4620,
				.lines_per_frame = 1626,
			},
			{
			}
		},
	},
	{
		 .desc = "imx091_cont_cap_vga",
		 .width = 656,
		 .height = 496,
		 .used = 0,
		 .regs = imx091_VGA_STILL,
		 .bin_factor_x = 1,
		 .bin_factor_y = 1,
		 .skip_frames = 0,
		 .fps_options =  {
			{
				.fps = 30,
				.pixels_per_line = 4620,
				.lines_per_frame = 1626,
			},
			{
			}
		}
	},
	{
		 .desc = "IMX091_PREVIEW_848x616",
		 .width = 848,
		 .height = 616,
		 .used = 0,
		 .regs = imx091_PREVIEW_848x616,
		 .bin_factor_x = 2,
		 .bin_factor_y = 2,
		 .skip_frames = 0,
		 .fps_options = {
			{
				.fps = 30,
				.pixels_per_line = 4620, 
				.lines_per_frame = 1626,
			},
			{
			}
		}
	},
	{
		 .desc = "imx091_wide_preview",
		 .width = 1296,
		 .height = 736,
		 .used = 0,
		 .regs = imx091_PREVIEW_WIDE_PREVIEW,
		 .bin_factor_x = 1,
		 .bin_factor_y = 1,
		 .skip_frames = 0,
		 .fps_options = {
			{
				.fps = 30,
				.pixels_per_line = 4620, 
				.lines_per_frame = 1626, 
			},
			{
			}
		},
	},
	{
		 .desc = "imx091_cont_cap_1M",
		 .width = 1040,
		 .height = 784,
		 .used = 0,
		 .regs = imx091_1M_STILL,
		 .bin_factor_x = 1,
		 .bin_factor_y = 1,
		 .skip_frames = 0,
		 .fps_options =  {
			{
				.fps = 30,
				.pixels_per_line = 4620,
				.lines_per_frame = 1626,
			},
			{
			}
		},
	},
	{
		 .desc = "imx091_cont_cap_2M",
		 .width = 1632,
		 .height = 1224,
		 .used = 0,
		 .regs = imx091_2M_STILL,
		 .bin_factor_x = 1,
		 .bin_factor_y = 1,
		 .skip_frames = 0,
		 .fps_options = {
			{
				.fps = 30,
				.pixels_per_line = 4620,
				.lines_per_frame = 1626,
			},
			{
			}
		},
	},
	{
		 .desc = "imx091_cont_cap_1080P",
		 .width = 1936,
		 .height = 1104,
		 .used = 0,
		 .regs = imx091_1080p_strong_dvs,
		 .bin_factor_x = 1,
		 .bin_factor_y = 1,
		 .skip_frames = 0,
		 .fps_options =  {
			{
				.fps = 30,
				.pixels_per_line = 4620,
				.lines_per_frame = 1626,
			},
			{
			}
		},
	},
	{
		.desc = "imx091_cont_cap_3M",
		.width = 2104,
		.height = 1560,
		.used = 0,
		.regs = imx091_3M_STILL,
		.bin_factor_x = 1,
		.bin_factor_y = 1,
		.skip_frames = 0,
		.fps_options =  {
			{
				.fps = 30,
				.pixels_per_line = 4620,
				.lines_per_frame = 1626,
			},
			{
			}
		},
	},
	{
		.desc = "imx091_cont_cap_5M",
		.width = 2576,
		.height = 1936,
		.used = 0,
		.regs = imx091_5M_STILL,
		.bin_factor_x = 0,
		.bin_factor_y = 0,
		.skip_frames = 0,
		.fps_options =  {
			{
				.fps = 18,
				.pixels_per_line = 5600,
				.lines_per_frame = 2100,
			},
			{
			}
		},
	},
	{
		.desc = "imx091_cont_cap_8M",
		.width = 3280,
		.height = 2464,
		.used = 0,
		.regs = imx091_8M_STILL,
		.bin_factor_x = 0,
		.bin_factor_y = 0,
		.skip_frames = 0,
		.fps_options = {
			{
				.fps = 13,
				.pixels_per_line = 6100,
				.lines_per_frame = 2600,
			},
			{
			}
		},
	},
	{
		.desc = "imx091_cont_cap_13M",
		.width = 4208,
		.height = 3120,
		.used = 0,
		.regs = imx091_13M_STILL,
		.bin_factor_x = 0,
		.bin_factor_y = 0,
		.skip_frames = 0,
		.fps_options = {
			{
				.fps = 9,
				.pixels_per_line = 7100,
				.lines_per_frame = 3256,
			},
			{
			}
		},
	},
};

static struct imx091_resolution imx091_res_still[] = {
	{
		 .desc = "STILL_VGA",
		 .width = 656,
		 .height = 496,
		 .used = 0,
		 .regs = imx091_VGA_STILL,
		 .bin_factor_x = 3,
		 .bin_factor_y = 3,
		 .skip_frames = 1,
		 .fps_options =  {
			{
				.fps = 30,
				.pixels_per_line = 4620,
				.lines_per_frame = 1626,
			},
			{
			}
		},
	},
	{
		 .desc = "STILL_1M",
		 .width = 1040,
		 .height = 784,
		 .used = 0,
		 .regs = imx091_1M_STILL,
		 .bin_factor_x = 2,
		 .bin_factor_y = 2,
		 .skip_frames = 1,
		 .fps_options =  {
			{
				.fps = 30,
				.pixels_per_line = 4620,
				.lines_per_frame = 1626,
			},
			{
			}
		},
	},
	{
		 .desc = "STILL_2M",
		 .width = 1632,
		 .height = 1224,
		 .used = 0,
		 .regs = imx091_2M_STILL,
		 .bin_factor_x = 1,
		 .bin_factor_y = 1,
		 .skip_frames = 1,
		 .fps_options =  {
			{
				.fps = 30,
				.pixels_per_line = 4620,
				.lines_per_frame = 1626,
			},
			{
			}
		},
	},
	{
		.desc = "STILL_3M",
		.width = 2104,
		.height = 1560,
		.used = 0,
		.regs = imx091_3M_STILL,
		.bin_factor_x = 1,
		.bin_factor_y = 1,
		.skip_frames = 1,
		.fps_options =  {
			{
				.fps = 30,
				.pixels_per_line = 4620,
				.lines_per_frame = 1626,
			},
			{
			}
		},
	},
	{
		.desc = "STILL_5M",
		.width = 2576,
		.height = 1936,
		.used = 0,
		.regs = imx091_5M_STILL,
		.bin_factor_x = 0,
		.bin_factor_y = 0,
		.skip_frames = 1,
		.fps_options =  {
			{
				.fps = 18,
				.pixels_per_line = 5600,
				.lines_per_frame = 2100,
			},
			{
			}
		},
	},
	{
		.desc = "STILL_8M",
		.width = 3280,
		.height = 2464,
		.used = 0,
		.regs = imx091_8M_STILL,
		.bin_factor_x = 0,
		.bin_factor_y = 0,
		.skip_frames = 1,
		.fps_options = {
			{
				.fps = 13,
				.pixels_per_line = 6100,
				.lines_per_frame = 2600,
			},
			{
			}
		},
	},
	{
		.desc = "STILL_13M",
		.width = 4208,
		.height = 3120,
		.used = 0,
		.regs = imx091_13M_STILL,
		.bin_factor_x = 0,
		.bin_factor_y = 0,
		.skip_frames = 1,
		.fps_options = {
			{
				.fps = 9,
				.pixels_per_line = 7100, 
				.lines_per_frame = 3256,
			},
			{
			}
		},
	},
};

static struct imx091_resolution imx091_res_video[] = {
	{
		 .desc = "QCIF_strong_dvs",
		 .width = 192,
		 .height = 160,
		 .used = 0,
		 .regs = imx091_QCIF_strong_dvs,
		 .bin_factor_x = 4,
		 .bin_factor_y = 4,
		 .skip_frames = 0,
		 .fps_options = {
			{
				 .fps = 30,
				 .pixels_per_line = 4620,
				 .lines_per_frame = 1626,
			},
			{
			}
		},
	},
	{
		 .desc = "QVGA_strong_dvs",
		 .width = 336,
		 .height = 256,
		 .used = 0,
		 .regs = imx091_QVGA_strong_dvs,
		 .bin_factor_x = 4,
		 .bin_factor_y = 4,
		 .skip_frames = 0,
		 .fps_options = {
			{
				 .fps = 30,
				 .pixels_per_line = 4620,
				 .lines_per_frame = 1626,
			},
			{
			}
		},
	},
	{
		 .desc = "VGA_strong_dvs",
		 .width = 656,
		 .height = 496,
		 .used = 0,
		 .regs = imx091_VGA_STILL,
		 .bin_factor_x = 3,
		 .bin_factor_y = 3,
		 .skip_frames = 0,
		 .fps_options = {
			{
				 .fps = 30,
				 .pixels_per_line = 4620,
				 .lines_per_frame = 1626,
			},
			{
			}
		},
	},
	{
		.desc = "480p_strong_dvs",
		.width = 784,
		.height = 496,
		.regs = imx091_480p_strong_dvs,
		.bin_factor_x = 2,
		.bin_factor_y = 2,
		.skip_frames = 0,
		.fps_options = {
			{
				 .fps = 30,
				 .pixels_per_line = 4620,
				 .lines_per_frame = 1626,
			},
			{
			}
		},
	},
	{
		 .desc = "720p_strong_dvs",
		 .width = 1296,
		 .height = 736,
		 .used = 0,
		 .regs = imx091_720p_strong_dvs,
		 .bin_factor_x = 1,
		 .bin_factor_y = 1,
		 .skip_frames = 0,
		 .fps_options = {
			{
				 .fps = 30,
				 .pixels_per_line = 4620,
				 .lines_per_frame = 1626,
			},
			{
			}
		},
	},
	{
		 .desc = "MODE1920x1080",
		 .width = 1936,
		 .height = 1104,
		 .used = 0,
		 .regs = imx091_1080p_strong_dvs,
		 .bin_factor_x = 1,
		 .bin_factor_y = 1,
		 .skip_frames = 0,
		 .fps_options = {
			{
				 .fps = 30,
				 .pixels_per_line = 4620,
				 .lines_per_frame = 1626,
			},
			{
			}
		},
	},
};

#endif
