/*
 * Support for Sony imx091 13M camera sensor.
 *
 * Copyright (c) 2011 Intel Corporation. All Rights Reserved.
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

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/kmod.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/moduleparam.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/bitops.h>
#include <media/v4l2-device.h>
#include <media/v4l2-chip-ident.h>

#include "imx091.h"

#define IMX091_BIN_FACTOR_MAX	2

#define to_imx091_sensor(sd) container_of(sd, struct imx091_device, sd)

u16 g_vcm[3] = {0,0,0};
static bool b_probe = false;
static int
imx091_read_reg(struct i2c_client *client, u16 len, u16 reg, u16 *val)
{
	struct i2c_msg msg[2];
	u16 data[IMX091_SHORT_MAX];
	int err, i;

	if (!client->adapter) {
		v4l2_err(client, "%s error, no client->adapter\n", __func__);
		return -ENODEV;
	}

	/* @len should be even when > 1 */
	if (len > IMX091_BYTE_MAX) {
		v4l2_err(client, "%s error, invalid data length\n", __func__);
		return -EINVAL;
	}

	memset(msg, 0, sizeof(msg));
	memset(data, 0, sizeof(data));

	msg[0].addr = client->addr;
	msg[0].flags = 0;
	msg[0].len = I2C_MSG_LENGTH;
	msg[0].buf = (u8 *)data;
	/* high byte goes first */
	data[0] = cpu_to_be16(reg);

	msg[1].addr = client->addr;
	msg[1].len = len;
	msg[1].flags = I2C_M_RD;
	msg[1].buf = (u8 *)data;

	err = i2c_transfer(client->adapter, msg, 2);
	if (err < 0)
		goto error;

	/* high byte comes first */
	if (len == IMX091_8BIT) {
		*val = (u8)data[0];
	} else {
		/* 16-bit access is default when len > 1 */
		for (i = 0; i < (len >> 1); i++)
			val[i] = be16_to_cpu(data[i]);
	}

	return 0;

error:
	dev_err(&client->dev, "read from offset 0x%x error %d", reg, err);
	return err;
}

static int imx091_i2c_write(struct i2c_client *client, u16 len, u8 *data)
{
	struct i2c_msg msg;
	const int num_msg = 1;
	int ret;
	int retry = 0;

again:
	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = len;
	msg.buf = data;

	ret = i2c_transfer(client->adapter, &msg, 1);

	/*
	 * It is said that Rev 2 sensor needs some delay here otherwise
	 * registers do not seem to load correctly. But tests show that
	 * removing the delay would not cause any in-stablility issue and the
	 * delay will cause serious performance down, so, removed previous
	 * mdelay(1) here.
	 */

	if (ret == num_msg)
		return 0;

	if (retry <= I2C_RETRY_COUNT) {
		dev_err(&client->dev, "retrying i2c write transfer... %d",
			retry);
		retry++;
		msleep(20);
		goto again;
	}

	return ret;
}

static int
imx091_write_reg(struct i2c_client *client, u16 data_length, u16 reg, u16 val)
{
	int ret;
	unsigned char data[4] = {0};
	u16 *wreg;
	const u16 len = data_length + sizeof(u16); /* 16-bit address + data */

	if (!client->adapter) {
		v4l2_err(client, "%s error, no client->adapter\n", __func__);
		return -ENODEV;
	}

	if (data_length != IMX091_8BIT && data_length != IMX091_16BIT) {
		v4l2_err(client, "%s error, invalid data_length\n", __func__);
		return -EINVAL;
	}

	/* high byte goes out first */
	wreg = (u16 *)data;
	*wreg = cpu_to_be16(reg);

	if (data_length == IMX091_8BIT) {
		data[2] = (u8)(val);
	} else {
		/* IMX091_16BIT */
		u16 *wdata = (u16 *)&data[2];
		*wdata = be16_to_cpu(val);
	}

	ret = imx091_i2c_write(client, len, data);
	if (ret)
		dev_err(&client->dev,
			"write error: wrote 0x%x to offset 0x%x error %d",
			val, reg, ret);
	return ret;
}


/*
 * imx091_write_reg_array - Initializes a list of MT9M114 registers
 * @client: i2c driver client structure
 * @reglist: list of registers to be written
 *
 * This function initializes a list of registers. When consecutive addresses
 * are found in a row on the list, this function creates a buffer and sends
 * consecutive data in a single i2c_transfer().
 *
 * __imx091_flush_reg_array, __imx091_buf_reg_array() and
 * __imx091_write_reg_is_consecutive() are internal functions to
 * imx091_write_reg_array_fast() and should be not used anywhere else.
 *
 */

static int __imx091_flush_reg_array(struct i2c_client *client,
				     struct imx091_write_ctrl *ctrl)
{
	u16 size;
	if (ctrl->index == 0)
		return 0;

	size = sizeof(u16) + ctrl->index; /* 16-bit address + data */
	ctrl->buffer.addr = cpu_to_be16(ctrl->buffer.addr);
	ctrl->index = 0;
	return imx091_i2c_write(client, size, (u8 *)&ctrl->buffer);
}

static int __imx091_buf_reg_array(struct i2c_client *client,
				   struct imx091_write_ctrl *ctrl,
				   const struct imx091_reg *next)
{
	int size;
	u16 *data16;
	switch (next->type) {
	case IMX091_8BIT:
		size = 1;
		ctrl->buffer.data[ctrl->index] = (u8)next->val;
		break;
	case IMX091_16BIT:
		size = 2;
		data16 = (u16 *)&ctrl->buffer.data[ctrl->index];
		*data16 = cpu_to_be16((u16)next->val);
		break;
	default:
		return -EINVAL;
	}

	/* When first item is added, we need to store its starting address */
	if (ctrl->index == 0)
		ctrl->buffer.addr = next->reg.sreg;

	ctrl->index += size;

	/*
	 * Buffer cannot guarantee free space for u32? Better flush it to avoid
	 * possible lack of memory for next item.
	 */
	if (ctrl->index + sizeof(u16) >= IMX091_MAX_WRITE_BUF_SIZE)
		__imx091_flush_reg_array(client, ctrl);

	return 0;
}

static int
__imx091_write_reg_is_consecutive(struct i2c_client *client,
				   struct imx091_write_ctrl *ctrl,
				   const struct imx091_reg *next)
{

	if (ctrl->index == 0)
		return 1;

	return ctrl->buffer.addr + ctrl->index == next->reg.sreg;
}

static int imx091_write_reg_array(struct i2c_client *client,
				   const struct imx091_reg *reglist)
{
	const struct imx091_reg *next = reglist;
	struct imx091_write_ctrl ctrl;
	int err;
	ctrl.index = 0;
	for (; next->type != IMX091_TOK_TERM; next++) {
		switch (next->type & IMX091_TOK_MASK) {
		case IMX091_TOK_DELAY:
			err = __imx091_flush_reg_array(client, &ctrl);
			if (err)
				return err;
			msleep(next->val);
			break;

		default:
			/*
			 * If next address is not consecutive, data needs to be
			 * flushed before proceed.
			 */
			if (!__imx091_write_reg_is_consecutive(client, &ctrl,
								next)) {
				err = __imx091_flush_reg_array(client, &ctrl);
				if (err)
					return err;
			}
			err = __imx091_buf_reg_array(client, &ctrl, next);
			if (err) {
				v4l2_err(client, "%s: write error, aborted\n",
					 __func__);
				return err;
			}
			break;
		}
	}
	return __imx091_flush_reg_array(client, &ctrl);
}

static int eeprom_i2c_read(struct i2c_client *client, u16 len, u8 reg, u16 *val)
{
	struct i2c_msg msg[2];
	u16 data[IMX091_SHORT_MAX];
	int err, i;
	
	memset(msg, 0, sizeof(msg));
	memset(data, 0, sizeof(data));

	msg[0].addr = 0xA0 >> 1;
	msg[0].flags = 0;
	msg[0].len = 1;
	msg[0].buf = &reg;

	msg[1].addr = 0xA0 >> 1;;
	msg[1].len = len;
	msg[1].flags = I2C_M_RD;
	msg[1].buf = (u8 *)data;

	err = i2c_transfer(client->adapter, msg, 2);
	if (err < 0)
		goto error;

	/* 16-bit access is default when len > 1 */
	for (i = 0; i < (len >> 1); i++)
		val[i] = be16_to_cpu(data[i]);

	return 0;

error:
	dev_err(&client->dev, "read from offset 0x%x error %d", reg, err);
	return err;
}

#define CONVERT_WORD(data)    (((data & 0xFF) <<8) |((data >> 8) & 0xFF))
#define OTP_DATA_SIZE   544
#define BYTE_PER_BLOCK  256
/*
 * Read EEPROM data and store it into a kmalloced buffer. 
 * On error return NULL.
 * The caller must kfree the buffer when no more needed.
 * @size: set to the size of the returned EEPROM data.
 */
static void *EEPROM_read(struct i2c_client *client, u32 *size)
{
	static const unsigned int EEPROM_I2C_ADDR = 0xA2 >> 1;
	static const unsigned int MAX_READ_SIZE = IMX091_MAX_WRITE_BUF_SIZE;
	static const unsigned int EEPROM_BLOCK0_READ_SIZE = 3;
	struct i2c_msg msg[2];
	u16 data[EEPROM_BLOCK0_READ_SIZE];
	u16 module_id, lens_id, platform_id;
	unsigned int addr;
	unsigned int length;
	u8 *buffer;
	unsigned int i2c_addr;
	unsigned char addr_buf;
	unsigned int block = 0;
	int r;
	#if 0
	int i = 0;
	u8 *read_buffer;
	#endif

       /* read block0 information, including module id, lens id and platform id */
	memset(data, 0, 3 * sizeof(u16));
	r = eeprom_i2c_read(client, 6, 0x00, data);
	if (r) {
		dev_err(&client->dev, "read failed with block 0\n");
		return NULL;
	}
	module_id = CONVERT_WORD(data[0]);
	lens_id = CONVERT_WORD(data[1]);
	platform_id = CONVERT_WORD(data[2]);
	printk("module id = 0x%4.4x\n", module_id);
	printk("lens id = 0x%4.4x\n", lens_id);
       printk("platform id = 0x%4.4x\n", platform_id);
	
	/* read block1 and block2 OTP data */
	buffer = kmalloc(OTP_DATA_SIZE, GFP_KERNEL);
	if (!buffer) {
		dev_err(&client->dev, "allocate buffer failed!!\n");
		return NULL;
	}
	memset(buffer, 0, sizeof(buffer));
	
	for(block = 0; block < (OTP_DATA_SIZE/BYTE_PER_BLOCK + 1); block++) {
		memset(msg, 0, sizeof(msg));
		i2c_addr = EEPROM_I2C_ADDR + block;
		
		if(block < 2){
			length = BYTE_PER_BLOCK;
		}else{
			length = OTP_DATA_SIZE % BYTE_PER_BLOCK;
		}		
		for (addr = 0; addr < length; addr += MAX_READ_SIZE) {
			addr_buf = addr & 0xFF;

			msg[0].addr = i2c_addr;
			msg[0].flags = 0;
			msg[0].len = 1;
			msg[0].buf = &addr_buf;

			msg[1].addr = i2c_addr;
			msg[1].flags = I2C_M_RD;
			msg[1].len = min(MAX_READ_SIZE, length - addr);
			msg[1].buf = &buffer[addr + block*BYTE_PER_BLOCK];

			r = i2c_transfer(client->adapter, msg, ARRAY_SIZE(msg));
			if (r != ARRAY_SIZE(msg)) {
			    kfree(buffer);
			    dev_err(&client->dev, "read failed at 0x%03x\n", addr);
			    return NULL;
			}
		}
	}
	#if 0
	read_buffer = buffer;
	for( i = 0; i < OTP_DATA_SIZE; i ++ )
	{
            printk("read OTP data[%d]= 0x%x\n", i, *read_buffer);
	     read_buffer++;
	}
	#endif
	
	if (size)
		*size = OTP_DATA_SIZE;
	return buffer;
}

static int imx091_g_priv_int_data(struct v4l2_subdev *sd,
				  struct v4l2_private_int_data *priv)
{
	u32 size;
        struct i2c_client *client = v4l2_get_subdevdata(sd);
	int r = 0;
	void *otp_data;
       dev_info(&client->dev,"imx091_g_priv_int_data\n");
	otp_data = EEPROM_read(v4l2_get_subdevdata(sd), &size);
	if (!otp_data)
		return -EIO;
	if (copy_to_user(priv->data, otp_data, min(priv->size, size )))
		r = -EFAULT;
	priv->size = size;
       dev_info(&client->dev,"imx091_g_priv_int_data r=%d,priv->size =%d\n",r,(int)priv->size );
	return r;
}

static int __imx091_set_exposure(struct v4l2_subdev *sd, u16 coarse_itg, 
	                          u16 gain)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret;
	struct imx091_device *dev = to_imx091_sensor(sd);
	const struct imx091_resolution *res;
	u16 hts, vts;
	
	//if (1)     
	//	return 0;

	/* imx sensor driver has a limitation that the exposure
	 * should not exceed beyond VTS-4
	 */

	res = &dev->curr_res_table[dev->fmt_idx];
	hts = res->fps_options[dev->fps_index].pixels_per_line;
	vts = res->fps_options[dev->fps_index].lines_per_frame;
	
	 if (coarse_itg > vts - 4)
		coarse_itg = vts - 4;

	/* TODO - to be removed and implemented in 3a library
	 * change gain to imx reg value
	 */
	if (gain > 0)
		gain = 256 - 256 * 16 / gain;

	/* enable group hold */
	ret = imx091_write_reg_array(client, imx091_param_hold);
	if (ret)
		goto out;

	ret = imx091_write_reg(client, IMX091_16BIT,
		IMX091_COARSE_INTEGRATION_TIME, coarse_itg);
	if (ret)
		goto out;

	/* set global gain */
	ret = imx091_write_reg(client, IMX091_8BIT,
		IMX091_GLOBAL_GAIN, gain);
	if (ret)
		goto out_disable;
	
	dev->gain       = gain;
	dev->coarse_itg = coarse_itg;

out_disable:
	/* disable group hold */
	imx091_write_reg_array(client, imx091_param_update);
out:
	return ret;
}

static int imx091_set_exposure(struct v4l2_subdev *sd, int exposure, int gain)
{
	struct imx091_device *dev = to_imx091_sensor(sd);
	int ret;

	mutex_lock(&dev->input_lock);
	ret = __imx091_set_exposure(sd, exposure, gain);
	mutex_unlock(&dev->input_lock);

	return ret;
}

static int imx091_s_exposure(struct v4l2_subdev *sd,
			      struct atomisp_exposure *exposure)
{
	u16 coarse_itg, gain;

	coarse_itg = exposure->integration_time[0];
	gain = exposure->gain[0];

	/* we should not accept the invalid value below. */
	if (gain == 0) {
		struct i2c_client *client = v4l2_get_subdevdata(sd);
		v4l2_err(client, "%s: invalid value\n", __func__);
		return -EINVAL;
	}
	return imx091_set_exposure(sd, coarse_itg, gain);
}

static long imx091_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	switch (cmd) {
	case ATOMISP_IOC_S_EXPOSURE:
		return imx091_s_exposure(sd, (struct atomisp_exposure *)arg);
	case ATOMISP_IOC_G_SENSOR_PRIV_INT_DATA:
		return imx091_g_priv_int_data(sd, arg);
	default:
		return -EINVAL;
	}
	return 0;
}

static int imx091_init_registers(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct imx091_device *dev = to_imx091_sensor(sd);
	int ret;

	printk("%s ++\n", __func__);
	
	ret = imx091_write_reg_array(client, imx091_soft_standby);	
       mdelay(3);
	dev->basic_settings_list = imx091_BasicSettings;
	ret = imx091_write_reg_array(client, imx091_BasicSettings);
	
	printk("%s --\n", __func__);
	return ret;
}

static int imx091_init(struct v4l2_subdev *sd, u32 val)
{
	struct imx091_device *dev = to_imx091_sensor(sd);
	int ret;
	mutex_lock(&dev->input_lock);
	ret = imx091_init_registers(sd);
	mutex_unlock(&dev->input_lock);

	return ret;
}

static void imx091_uninit(struct v4l2_subdev *sd)
{
	struct imx091_device *dev = to_imx091_sensor(sd);
	
	dev->coarse_itg = 0;
	dev->fine_itg   = 0;
	dev->exposure = 0;
	dev->gain     = 0;
	dev->digital_gain = 0;
}

static int power_up(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct imx091_device *dev = to_imx091_sensor(sd);
	int ret;
	
	/* set reset to low before power up */
	ret = dev->platform_data->gpio_ctrl(sd, 0);
	if (ret)
		dev_err(&client->dev, "gpio failed\n");
	
	/* Enable power */
	ret = dev->platform_data->power_ctrl(sd, 1);
	if (ret)
		goto fail_power;

	/* Release reset */
	ret = dev->platform_data->gpio_ctrl(sd, 1);
	if (ret)
		dev_err(&client->dev, "gpio failed\n");

	/* Enable clock */
	ret = dev->platform_data->flisclk_ctrl(sd, 1);
	if (ret)
		goto fail_clk;

	/* Minumum delay is 8192 clock cycles before first i2c transaction,
	 * which is 1.37 ms at the lowest allowed clock rate 6 MHz */
	msleep(2);
	return 0;

fail_clk:
	dev->platform_data->flisclk_ctrl(sd, 0);
	dev->platform_data->gpio_ctrl(sd, 0);
fail_power:
	dev->platform_data->power_ctrl(sd, 0);
	dev_err(&client->dev, "sensor power-up failed\n");

	return ret;
}

static int power_down(struct v4l2_subdev *sd)
{
	struct imx091_device *dev = to_imx091_sensor(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret;

	ret = dev->platform_data->flisclk_ctrl(sd, 0);
	if (ret)
		dev_err(&client->dev, "flisclk failed\n");

	/* gpio ctrl */
	ret = dev->platform_data->gpio_ctrl(sd, 0);
	if (ret)
		dev_err(&client->dev, "gpio failed 1\n");

	/* power control */
	ret = dev->platform_data->power_ctrl(sd, 0);
	if (ret)
		dev_err(&client->dev, "vprog failed.\n");

	return ret;
}

static int __imx091_s_power(struct v4l2_subdev *sd, int on)
{
	struct imx091_device *dev = to_imx091_sensor(sd);
	int ret =0;
	if (on == 0) {
		imx091_uninit(sd);
		ret = power_down(sd);
		if(ret)
			return ret ;
		dev->power = 0;
	} else {
		ret = power_up(sd);
		if (ret)
			return ret;
		dev->power = 1;
              if(b_probe) {
		/* Initalise sensor settings */
		ret = imx091_init_registers(sd);
              }
	}

	return ret;
}

static int imx091_s_power(struct v4l2_subdev *sd, int on)
{
	int ret;
	struct imx091_device *dev = to_imx091_sensor(sd);

	mutex_lock(&dev->input_lock);
	ret = __imx091_s_power(sd, on);
	mutex_unlock(&dev->input_lock);
	return ret;
}

static int imx091_g_chip_ident(struct v4l2_subdev *sd,
				struct v4l2_dbg_chip_ident *chip)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (!chip)
		return -EINVAL;

	v4l2_chip_ident_i2c_client(client, chip, V4L2_IDENT_IMX091, 0);

	return 0;
}

static int imx091_get_intg_factor(struct v4l2_subdev *sd,
				  struct camera_mipi_info *info,
				  const struct imx091_reg *reglist)
{
	struct atomisp_sensor_mode_data *m = &info->data;
	struct imx091_device *dev = to_imx091_sensor(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	u32 vt_pix_clk_div;
	u32 vt_sys_clk_div;
	u32 pre_pll_clk_div;
	u32 pll_multiplier;
	
	const int ext_clk_freq_hz = 19200000;
	int vt_pix_clk_freq_mhz, ret;
	u16 data[IMX091_INTG_BUF_COUNT];

	u32 coarse_integration_time_min;
	u32 coarse_integration_time_max_margin;
	u32 frame_length_lines;
	u32 line_length_pck;
	u32 read_mode;
	u32 div;

	if (info == NULL)
		return -EINVAL;

	memset(data, 0, IMX091_INTG_BUF_COUNT * sizeof(u16));
	ret = imx091_read_reg(client, 1, IMX091_VT_PIX_CLK_DIV, data);
	if (ret)
		return ret;
	vt_pix_clk_div = data[0] & IMX091_MASK_8BIT;
	ret = imx091_read_reg(client, 1, IMX091_VT_SYS_CLK_DIV, data);
	if (ret)
		return ret;
	vt_sys_clk_div = data[0] & IMX091_MASK_8BIT;
	ret = imx091_read_reg(client, 1, IMX091_PRE_PLL_CLK_DIV, data);
	if (ret)
		return ret;
	pre_pll_clk_div = data[0] & IMX091_MASK_4BIT;
	ret = imx091_read_reg(client, 1, IMX091_PLL_MULTIPLIER, data);
	if (ret)
		return ret;
	pll_multiplier = data[0] & IMX091_MASK_8BIT;
	
	memset(data, 0, IMX091_INTG_BUF_COUNT * sizeof(u16));
	ret = imx091_read_reg(client, 4, IMX091_FRAME_LENGTH_LINES, data);
	if (ret)
		return ret;
	frame_length_lines = data[0];
	line_length_pck = data[1];

	memset(data, 0, IMX091_INTG_BUF_COUNT * sizeof(u16));
	ret = imx091_read_reg(client, 4, IMX091_COARSE_INTG_TIME_MIN, data);
	if (ret)
		return ret;
	coarse_integration_time_min = data[0];
	coarse_integration_time_max_margin = data[1];

	memset(data, 0, IMX091_INTG_BUF_COUNT * sizeof(u16));
	ret = imx091_read_reg(client, 1, IMX091_READ_MODE, data); //0x0390 ???
	if (ret)
		return ret;
	read_mode = data[0] & IMX091_MASK_2BIT;

	div = pre_pll_clk_div*vt_sys_clk_div*vt_pix_clk_div;
	if (div == 0)
		return -EINVAL;
	vt_pix_clk_freq_mhz = ext_clk_freq_hz*pll_multiplier/div;

	dev->vt_pix_clk_freq_mhz = vt_pix_clk_freq_mhz;
	m->coarse_integration_time_min = coarse_integration_time_min;
	m->coarse_integration_time_max_margin
		= coarse_integration_time_max_margin;
	m->fine_integration_time_min = IMX091_FINE_INTG_TIME;
	m->fine_integration_time_max_margin = IMX091_FINE_INTG_TIME;
	m->fine_integration_time_def = IMX091_FINE_INTG_TIME;
	m->vt_pix_clk_freq_mhz = vt_pix_clk_freq_mhz;
	m->line_length_pck = line_length_pck;
	m->frame_length_lines = frame_length_lines;
	m->read_mode = read_mode;
	return 0;
}

static int __imx091_s_frame_interval(struct v4l2_subdev *sd,
			struct v4l2_subdev_frame_interval *interval)
{
	struct imx091_device *dev = to_imx091_sensor(sd);
	struct camera_mipi_info *info = v4l2_get_subdev_hostdata(sd);
	const struct imx091_resolution *res =
		res = &dev->curr_res_table[dev->fmt_idx];
	int i;
	int ret;
	int fps;
	u16 hts;
	u16 vts;

	if (!interval->interval.numerator)
		interval->interval.numerator = 1;

	fps = interval->interval.denominator / interval->interval.numerator;

	/* Ignore if we are already using the required FPS. */
	if (fps == res->fps_options[dev->fps_index].fps)
		return 0;

	dev->fps_index = 0;

	/* Go through the supported FPS list */
	for (i = 0; i < MAX_FPS_OPTIONS_SUPPORTED; i++) {
		if (!res->fps_options[i].fps)
			break;
		if (abs(res->fps_options[i].fps - fps)
		    < abs(res->fps_options[dev->fps_index].fps - fps))
			dev->fps_index = i;
	}

	/* Get the new Frame timing values for new exposure */
	hts = res->fps_options[dev->fps_index].pixels_per_line;
	vts = res->fps_options[dev->fps_index].lines_per_frame;

	/* update frametiming. Conside the curren exposure/gain as well */
	ret = __imx091_set_exposure(sd, dev->exposure, dev->gain);
	if (ret)
		return ret;

	/* Update the new values so that user side knows the current settings */
	ret = imx091_get_intg_factor(sd, info, dev->basic_settings_list);
	if (ret)
		return ret;

	interval->interval.denominator = res->fps_options[dev->fps_index].fps;
	interval->interval.numerator = 1;

	return 0;
}

/* This returns the exposure time being used. This should only be used
   for filling in EXIF data, not for actual image processing. */
static int imx091_q_exposure(struct v4l2_subdev *sd, s32 *value)
{
	struct imx091_device *dev = to_imx091_sensor(sd);
	*value = dev->exposure;
	return 0;
}

static int imx091_test_pattern(struct v4l2_subdev *sd, s32 value)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	return imx091_write_reg(client, IMX091_16BIT, IMX091_TEST_PATTERN_MODE, value);
}

static int imx091_v_flip(struct v4l2_subdev *sd, s32 value)
{
	//return -ENXIO;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret;
	u16 val;

	ret = imx091_write_reg_array(client, imx091_param_hold);
	if (ret)
		return ret;
	ret = imx091_read_reg(client, IMX091_8BIT,
			IMX091_IMG_ORIENTATION, &val);
	if (ret)
		return ret;
	if (value)
		val |= IMX091_VFLIP_BIT;
	else
		val &= ~IMX091_VFLIP_BIT;
	ret = imx091_write_reg(client, IMX091_8BIT,
			IMX091_IMG_ORIENTATION, val);
	if (ret)
		return ret;
	return imx091_write_reg_array(client, imx091_param_update);
}

static int imx091_g_focal(struct v4l2_subdev *sd, s32 *val)
{
	*val = (IMX091_FOCAL_LENGTH_NUM << 16) | IMX091_FOCAL_LENGTH_DEM;
	return 0;
}

static int imx091_g_fnumber(struct v4l2_subdev *sd, s32 *val)
{
	/*const f number for imx091*/
	*val = (IMX091_F_NUMBER_DEFAULT_NUM << 16) | IMX091_F_NUMBER_DEM;
	return 0;
}

static int imx091_g_fnumber_range(struct v4l2_subdev *sd, s32 *val)
{
	*val = (IMX091_F_NUMBER_DEFAULT_NUM << 24) |
		(IMX091_F_NUMBER_DEM << 16) |
		(IMX091_F_NUMBER_DEFAULT_NUM << 8) | IMX091_F_NUMBER_DEM;
	return 0;
}

static int imx091_g_bin_factor_x(struct v4l2_subdev *sd, s32 *val)
{
	struct imx091_device *dev = to_imx091_sensor(sd);
		   
	*val = dev->curr_res_table[dev->fmt_idx].bin_factor_x;
	   
	return 0;
}

static int imx091_g_bin_factor_y(struct v4l2_subdev *sd, s32 *val)
{
	struct imx091_device *dev = to_imx091_sensor(sd);

	*val = dev->curr_res_table[dev->fmt_idx].bin_factor_y;
	   
	return 0;
}

static struct imx091_control imx091_controls[] = {
	{
		.qc = {
			.id = V4L2_CID_EXPOSURE_ABSOLUTE,
			.type = V4L2_CTRL_TYPE_INTEGER,
			.name = "exposure",
			.minimum = 0x0,
			.maximum = 0xffff,
			.step = 0x01,
			.default_value = 0x00,
			.flags = 0,
		},
		.query = imx091_q_exposure,
	},
	{
		.qc = {
			.id = V4L2_CID_TEST_PATTERN,
			.type = V4L2_CTRL_TYPE_INTEGER,
			.name = "Test pattern",
			.minimum = 0,
			.maximum = 0xffff,
			.step = 1,
			.default_value = 0,
		},
		.tweak = imx091_test_pattern,
	},
	{
		.qc = {
			.id = V4L2_CID_VFLIP,
			.type = V4L2_CTRL_TYPE_BOOLEAN,
			.name = "Flip",
			.minimum = 0,
			.maximum = 1,
			.step = 1,
			.default_value = 0,
		},
		.tweak = imx091_v_flip,
	},
	{
		.qc = {
			.id = V4L2_CID_FOCAL_ABSOLUTE,
			.type = V4L2_CTRL_TYPE_INTEGER,
			.name = "focal length",
			.minimum = IMX091_FOCAL_LENGTH_DEFAULT,
			.maximum = IMX091_FOCAL_LENGTH_DEFAULT,
			.step = 0x01,
			.default_value = IMX091_FOCAL_LENGTH_DEFAULT,
			.flags = 0,
		},
		.query = imx091_g_focal,
	},
	{
		.qc = {
			.id = V4L2_CID_FNUMBER_ABSOLUTE,
			.type = V4L2_CTRL_TYPE_INTEGER,
			.name = "f-number",
			.minimum = IMX091_F_NUMBER_DEFAULT,
			.maximum = IMX091_F_NUMBER_DEFAULT,
			.step = 0x01,
			.default_value = IMX091_F_NUMBER_DEFAULT,
			.flags = 0,
		},
		.query = imx091_g_fnumber,
	},
	{
		.qc = {
			.id = V4L2_CID_FNUMBER_RANGE,
			.type = V4L2_CTRL_TYPE_INTEGER,
			.name = "f-number range",
			.minimum = IMX091_F_NUMBER_RANGE,
			.maximum =  IMX091_F_NUMBER_RANGE,
			.step = 0x01,
			.default_value = IMX091_F_NUMBER_RANGE,
			.flags = 0,
		},
		.query = imx091_g_fnumber_range,
	},
	{
		.qc = {
			.id = V4L2_CID_BIN_FACTOR_HORZ,
			.type = V4L2_CTRL_TYPE_INTEGER,
			.name = "horizontal binning factor",
			.minimum = 0,
			.maximum = IMX091_BIN_FACTOR_MAX,
			.step = 1,
			.default_value = 0,
			.flags = V4L2_CTRL_FLAG_READ_ONLY,
		},
		.query = imx091_g_bin_factor_x,
	},
	{
		.qc = {
			.id = V4L2_CID_BIN_FACTOR_VERT,
			.type = V4L2_CTRL_TYPE_INTEGER,
			.name = "vertical binning factor",
			.minimum = 0,
			.maximum = IMX091_BIN_FACTOR_MAX,
			.step = 1,
			.default_value = 0,
			.flags = V4L2_CTRL_FLAG_READ_ONLY,
		},
		.query = imx091_g_bin_factor_y,
	},
};
#define N_CONTROLS (ARRAY_SIZE(imx091_controls))

static struct imx091_control *imx091_find_control(u32 id)
{
	int i;

	for (i = 0; i < N_CONTROLS; i++)
		if (imx091_controls[i].qc.id == id)
			return &imx091_controls[i];
	return NULL;
}

static int imx091_queryctrl(struct v4l2_subdev *sd, struct v4l2_queryctrl *qc)
{
	struct imx091_control *ctrl = imx091_find_control(qc->id);

	if (ctrl == NULL)
		return -EINVAL;

	*qc = ctrl->qc;

	return 0;
}

/* imx091 control set/get */
static int imx091_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct imx091_device *dev = to_imx091_sensor(sd);
	struct imx091_control *s_ctrl;
	int ret;

	if (!ctrl)
		return -EINVAL;

	s_ctrl = imx091_find_control(ctrl->id);
	if ((s_ctrl == NULL) || (s_ctrl->query == NULL))
		return -EINVAL;

	mutex_lock(&dev->input_lock);
	ret = s_ctrl->query(sd, &ctrl->value);
	mutex_unlock(&dev->input_lock);

	return ret;
}

static int imx091_s_ctrl_old(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct imx091_device *dev = to_imx091_sensor(sd);
	struct imx091_control *octrl = imx091_find_control(ctrl->id);
	int ret;

	if ((octrl == NULL) || (octrl->tweak == NULL))
		return -EINVAL;

	mutex_lock(&dev->input_lock);
	ret = octrl->tweak(sd, ctrl->value);
	mutex_unlock(&dev->input_lock);

	return ret;
}

/*
 * distance - calculate the distance
 * @res: resolution
 * @w: width
 * @h: height
 *
 * Get the gap between resolution and w/h.
 * res->width/height smaller than w/h wouldn't be considered.
 * Returns the value of gap or -1 if fail.
 */
/* tune this value so that the DVS resolutions get selected properly,
 * but make sure 16:9 does not match 4:3.
 */
#define LARGEST_ALLOWED_RATIO_MISMATCH 600
static int distance(struct imx091_resolution const *res, const u32 w,
				const u32 h)
{
	unsigned int w_ratio = ((res->width<<13)/w);
	unsigned int h_ratio = ((res->height<<13)/h);
	int match;
	
	if(h_ratio == 0)
		return -1;
	
	match   = abs(((w_ratio<<13)/h_ratio) - ((int)8192));
	if ((w_ratio < (int)8192) || (h_ratio < (int)8192)
		|| (match > LARGEST_ALLOWED_RATIO_MISMATCH))
		return -1;

	return w_ratio + h_ratio;
}

/*
 * Returns the nearest higher resolution index.
 * @w: width
 * @h: height
 * matching is done based on enveloping resolution and
 * aspect ratio. If the aspect ratio cannot be matched
 * to any index, -1 is returned.
 */
static int nearest_resolution_index(struct v4l2_subdev *sd, int w, int h)
{
	int i;
	int idx = -1;
	int dist;
	int min_dist = INT_MAX;
	const struct imx091_resolution *tmp_res = NULL;
	struct imx091_device *dev = to_imx091_sensor(sd);
		
	for (i = 0; i < dev->entries_curr_table; i++) {
		tmp_res = &dev->curr_res_table[i];
		dist = distance(tmp_res, w, h);
		if (dist == -1)
			continue;
		if (dist < min_dist) {
			min_dist = dist;
			idx = i;
		}
	}
	return idx;
}

static int get_resolution_index(struct v4l2_subdev *sd, int w, int h)
{
	int i;
	struct imx091_device *dev = to_imx091_sensor(sd);

	for (i = 0; i < dev->entries_curr_table; i++) {
		if (w != dev->curr_res_table[i].width)
			continue;
		if (h != dev->curr_res_table[i].height)
			continue;
		/* Found it */
		return i;
	}
	return -1;
}

static int __imx091_try_mbus_fmt(struct v4l2_subdev *sd,
				 struct v4l2_mbus_framefmt *fmt)
{
	int idx;
	struct imx091_device *dev = to_imx091_sensor(sd);

	if (!fmt)
		return -EINVAL;
	printk("%s  dev->fmt_idx=%d,width=%d,height=%d\n",__func__, dev->fmt_idx,fmt->width, fmt->height);
	if ((fmt->width > IMX091_RES_WIDTH_MAX) ||
	    (fmt->height > IMX091_RES_HEIGHT_MAX)) {
		fmt->width = IMX091_RES_WIDTH_MAX;
		fmt->height = IMX091_RES_HEIGHT_MAX;
	} else {
		idx = nearest_resolution_index(sd, fmt->width, fmt->height);

		/*
		 * nearest_resolution_index() doesn't return smaller resolutions.
		 * If it fails, it means the requested resolution is higher than we
		 * can support. Fallback to highest possible resolution in this case.
		 */
		if (idx == -1)
			idx = dev->entries_curr_table - 1;

		fmt->width = dev->curr_res_table[idx].width;
		fmt->height = dev->curr_res_table[idx].height;
	}

	fmt->code = V4L2_MBUS_FMT_SBGGR10_1X10;
	return 0;
}

static int imx091_try_mbus_fmt(struct v4l2_subdev *sd,
			       struct v4l2_mbus_framefmt *fmt)
{
	struct imx091_device *dev = to_imx091_sensor(sd);
	int r;
       printk("%s ++\n", __func__);
	mutex_lock(&dev->input_lock);
	r = __imx091_try_mbus_fmt(sd, fmt);
	mutex_unlock(&dev->input_lock);
       printk("%s --\n", __func__);
	return r;
}

static int imx091_s_mbus_fmt(struct v4l2_subdev *sd,
			      struct v4l2_mbus_framefmt *fmt)
{
	struct imx091_device *dev = to_imx091_sensor(sd);
	struct camera_mipi_info *imx091_info = NULL;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	u16 hts, vts;
	int ret;
	const struct imx091_resolution *res;
       printk("%s ++\n", __func__);
	   
	imx091_info = v4l2_get_subdev_hostdata(sd);
	if (imx091_info == NULL)
		return -EINVAL;

	mutex_lock(&dev->input_lock);

	ret = __imx091_try_mbus_fmt(sd, fmt);
	if (ret)
		goto out;

	dev->fmt_idx = get_resolution_index(sd, fmt->width, fmt->height);
	 printk("%s dev->fmt_idx=%d, width =%d,height =%d\n",__func__, dev->fmt_idx,fmt->width,fmt->height);
	/* Sanity check */
	if (unlikely(dev->fmt_idx == -1)) {
		ret = -EINVAL;
		goto out;
	}

	/* Sets the default FPS */
	dev->fps_index = 0;

	/* Get the current resolution setting */
	res = &dev->curr_res_table[dev->fmt_idx];

	/* Write the selected resolution table values to the registers */
	ret = imx091_write_reg_array(client, res->regs);
	if (ret)
		goto out;

	/* Frame timing registers are updates as part of exposure */
	hts = res->fps_options[dev->fps_index].pixels_per_line;
	vts = res->fps_options[dev->fps_index].lines_per_frame;

	/*
	 * update hts, vts, exposure and gain as one block. Note that the vts
	 * will be changed according to the exposure used. But the maximum vts
	 * dev->curr_res_table[dev->fmt_idx] should not be changed at all.
	 */
	ret = __imx091_set_exposure(sd, dev->exposure, dev->gain);
	if (ret)
		goto out;

	ret = imx091_get_intg_factor(sd, imx091_info, res->regs);

out:
	mutex_unlock(&dev->input_lock);
       printk("%s --\n", __func__);
	return ret;
}

static int imx091_g_mbus_fmt(struct v4l2_subdev *sd,
			      struct v4l2_mbus_framefmt *fmt)
{
	struct imx091_device *dev = to_imx091_sensor(sd);
	if (!fmt)
		return -EINVAL;
	printk("%s  dev->fmt_idx=%d, width =%d,height =%d\n",__func__, dev->fmt_idx,fmt->width,fmt->height);
	mutex_lock(&dev->input_lock);
	fmt->width = dev->curr_res_table[dev->fmt_idx].width;
	fmt->height = dev->curr_res_table[dev->fmt_idx].height;
	fmt->code = V4L2_MBUS_FMT_SBGGR10_1X10;
	mutex_unlock(&dev->input_lock);

	return 0;
}

static int imx091_detect(struct i2c_client *client, u16 *id, u8 *revision)
{
	struct i2c_adapter *adapter = client->adapter;
	int ret, s_ret;

	/* i2c check */
	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C))
		return -ENODEV;

	/* check sensor chip ID */
	ret = imx091_read_reg(client, IMX091_16BIT, IMX091_CHIP_ID_HIGH, id);
	if (ret)
		return ret;

	dev_info(&client->dev, "chip_id = 0x%4.4x\n", *id);

	if (*id != IMX091_CHIP_ID) {
	       printk("not detect correct camera chip_id = 0x%4.4x\n", *id);
		return -ENODEV;
	}
	/* REVISIT: HACK: Driver is currently forcing revision to 0 */
	*revision = 0;

//out:
	/* Stream off now. */
	s_ret = imx091_write_reg(client, IMX091_8BIT, IMX091_STREAM_MODE, 0);

	return ret ? ret : s_ret;
}

/*
 * imx091 stream on/off
 */
static int imx091_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct imx091_device *dev = to_imx091_sensor(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret;
       printk("imx091_s_stream enable=%d\n",enable);
	mutex_lock(&dev->input_lock);

	ret = imx091_write_reg(client, IMX091_8BIT, IMX091_STREAM_MODE, enable ? 1 : 0);
	if (ret != 0) {
		mutex_unlock(&dev->input_lock);
		v4l2_err(client, "failed to set streaming\n");
		return ret;
	}
	dev->streaming = enable;
	mutex_unlock(&dev->input_lock);
       mdelay(220); // delay about 2 frame time
	return 0;
}

/*
 * imx091 enum frame size, frame intervals
 */
static int imx091_enum_framesizes(struct v4l2_subdev *sd,
				   struct v4l2_frmsizeenum *fsize)
{
	unsigned int index = fsize->index;
	struct imx091_device *dev = to_imx091_sensor(sd);

	mutex_lock(&dev->input_lock);
	if (index >= dev->entries_curr_table) {
		mutex_unlock(&dev->input_lock);
		return -EINVAL;
	}

	fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
	fsize->discrete.width = dev->curr_res_table[index].width;
	fsize->discrete.height = dev->curr_res_table[index].height;
	fsize->reserved[0] = dev->curr_res_table[index].used;
	mutex_unlock(&dev->input_lock);

	return 0;
}

static int imx091_enum_frameintervals(struct v4l2_subdev *sd,
				       struct v4l2_frmivalenum *fival)
{
	unsigned int index = fival->index;
	int fmt_index;
	struct imx091_device *dev = to_imx091_sensor(sd);
	const struct imx091_resolution *res;

	mutex_lock(&dev->input_lock);

	/*
	 * since the isp will donwscale the resolution to the right size,
	 * find the nearest one that will allow the isp to do so important to
	 * ensure that the resolution requested is padded correctly by the
	 * requester, which is the atomisp driver in this case.
	 */
	fmt_index = nearest_resolution_index(sd, fival->width, fival->height);
	if (-1 == fmt_index)
		fmt_index = dev->entries_curr_table - 1;

	res = &dev->curr_res_table[fmt_index];

	/* Check if this index is supported */
	//if (index > __imx091_get_max_fps_index(res->fps_options))
	//	return -EINVAL;

	fival->type = V4L2_FRMIVAL_TYPE_DISCRETE;
	fival->discrete.numerator = 1;
	fival->discrete.denominator = res->fps_options[index].fps;

	mutex_unlock(&dev->input_lock);

	return 0;
}

static int imx091_enum_mbus_fmt(struct v4l2_subdev *sd, unsigned int index,
				 enum v4l2_mbus_pixelcode *code)
{
	*code = V4L2_MBUS_FMT_SBGGR10_1X10;
	return 0;
}

static int imx091_s_config(struct v4l2_subdev *sd,
			    int irq, void *pdata)
{
	struct imx091_device *dev = to_imx091_sensor(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	u8 sensor_revision;
	u16 sensor_id;
	int ret;

	if (pdata == NULL)
		return -ENODEV;

	dev->platform_data = pdata;

	mutex_lock(&dev->input_lock);
	if (dev->platform_data->platform_init) {
		ret = dev->platform_data->platform_init(client);
		if (ret) {
			mutex_unlock(&dev->input_lock);
			v4l2_err(client, "imx091 platform init err\n");
			return ret;
		}
	}

	ret = __imx091_s_power(sd, 1);
	if (ret) {
		mutex_unlock(&dev->input_lock);
		v4l2_err(client, "imx091 power-up err.\n");
		return ret;
	}

	ret = dev->platform_data->csi_cfg(sd, 1);
	if (ret)
		goto fail_csi_cfg;

	/* config & detect sensor */
	ret = imx091_detect(client, &sensor_id, &sensor_revision);
	if (ret) {
		v4l2_err(client, "imx091_detect err s_config.\n");
		goto fail_detect;
	}

	dev->sensor_id = sensor_id;
	dev->sensor_revision = sensor_revision;

	/* power off sensor */
	ret = __imx091_s_power(sd, 0);
	mutex_unlock(&dev->input_lock);
	if (ret) {
		v4l2_err(client, "imx091 power-down err.\n");
		return ret;
	}
	return 0;

fail_detect:
	dev->platform_data->csi_cfg(sd, 0);
fail_csi_cfg:
	__imx091_s_power(sd, 0);
	mutex_unlock(&dev->input_lock);
	dev_err(&client->dev, "sensor power-gating failed\n");
	return ret;
}

static int
imx091_enum_mbus_code(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh,
		       struct v4l2_subdev_mbus_code_enum *code)
{
	if (code->index >= MAX_FMTS)
		return -EINVAL;
	code->code = V4L2_MBUS_FMT_SBGGR10_1X10;

	return 0;
}

static int
imx091_enum_frame_size(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh,
			struct v4l2_subdev_frame_size_enum *fse)
{
	int index = fse->index;
	struct imx091_device *dev = to_imx091_sensor(sd);

	mutex_lock(&dev->input_lock);
	if (index >= dev->entries_curr_table) {
		mutex_unlock(&dev->input_lock);
		return -EINVAL;
	}

	fse->min_width = dev->curr_res_table[index].width;
	fse->min_height = dev->curr_res_table[index].height;
	fse->max_width = dev->curr_res_table[index].width;
	fse->max_height = dev->curr_res_table[index].height;
	mutex_unlock(&dev->input_lock);

	return 0;
}

static struct v4l2_mbus_framefmt *
__imx091_get_pad_format(struct imx091_device *sensor,
			 struct v4l2_subdev_fh *fh, unsigned int pad,
			 enum v4l2_subdev_format_whence which)
{
	struct i2c_client *client = v4l2_get_subdevdata(&sensor->sd);

	if (pad != 0) {
		v4l2_err(client, "%s err. pad %x\n", __func__, pad);
		return NULL;
	}

	switch (which) {
	case V4L2_SUBDEV_FORMAT_TRY:
		return v4l2_subdev_get_try_format(fh, pad);
	case V4L2_SUBDEV_FORMAT_ACTIVE:
		return &sensor->format;
	default:
		return NULL;
	}
}

static int
imx091_get_pad_format(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh,
		       struct v4l2_subdev_format *fmt)
{
	struct imx091_device *dev = to_imx091_sensor(sd);
	struct v4l2_mbus_framefmt *format =
			__imx091_get_pad_format(dev, fh, fmt->pad, fmt->which);

	if (format == NULL)
		return -EINVAL;
	fmt->format = *format;

	return 0;
}

static int
imx091_set_pad_format(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh,
		       struct v4l2_subdev_format *fmt)
{
	struct imx091_device *dev = to_imx091_sensor(sd);
	struct v4l2_mbus_framefmt *format =
			__imx091_get_pad_format(dev, fh, fmt->pad, fmt->which);

	if (format == NULL)
		return -EINVAL;
	if (fmt->which == V4L2_SUBDEV_FORMAT_ACTIVE)
		dev->format = fmt->format;

	return 0;
}

static int
imx091_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct imx091_device *dev = container_of(
		ctrl->handler, struct imx091_device, ctrl_handler);

	/* input_lock is taken by the control framework, so it
	 * doesn't need to be taken here.
	 */

	/* We only handle V4L2_CID_RUN_MODE for now. */
	switch (ctrl->val) {
	case ATOMISP_RUN_MODE_VIDEO:
		dev->curr_res_table =  imx091_res_video;
		dev->entries_curr_table = ARRAY_SIZE(imx091_res_video);
		break;
	case ATOMISP_RUN_MODE_STILL_CAPTURE:
		dev->curr_res_table = imx091_res_still;
		dev->entries_curr_table = ARRAY_SIZE(imx091_res_still);
		break;
	default:
		dev->curr_res_table = imx091_res_preview;
		dev->entries_curr_table = ARRAY_SIZE(imx091_res_preview);
	}

	dev->fmt_idx = 0;
	dev->fps_index = 0;

	return 0;
}

static int
imx091_g_frame_interval(struct v4l2_subdev *sd,
			struct v4l2_subdev_frame_interval *interval)
{
	struct imx091_device *dev = to_imx091_sensor(sd);
	const struct imx091_resolution *res;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	u16 lines_per_frame;
	u16 hts;
	u16 vts;
	
	res = &dev->curr_res_table[dev->fmt_idx];
	hts = res->fps_options[dev->fps_index].pixels_per_line;
	vts = res->fps_options[dev->fps_index].lines_per_frame;

	/*
	 * if no specific information to calculate the fps,
	 * just used the value in sensor settings
	 */

	if (!hts || !vts) {
		interval->interval.numerator = 1;
		interval->interval.denominator = res->fps_options[dev->fps_index].fps;
		return 0;
	}

	/*
	 * DS: if coarse_integration_time is set larger than
	 * lines_per_frame the frame_size will be expanded to
	 * coarse_integration_time+1
	 */
	if (dev->coarse_itg > vts) {
		if (dev->coarse_itg == 0xFFFF) {
			/*
			 * we can not add 4 according to ds, as this will
			 * cause over flow
			 */
			v4l2_warn(client, "%s: abnormal coarse_itg:0x%x\n",
				  __func__, dev->coarse_itg);
			lines_per_frame = dev->coarse_itg;
		} else {
			lines_per_frame = dev->coarse_itg + 4;
		}
	} else {
		lines_per_frame = vts;
	}
	interval->interval.numerator = hts * lines_per_frame;
	interval->interval.denominator = dev->vt_pix_clk_freq_mhz;
	return 0;
}

static int imx091_s_frame_interval(struct v4l2_subdev *sd,
			struct v4l2_subdev_frame_interval *interval)
{
	struct imx091_device *dev = to_imx091_sensor(sd);
	int ret;

	mutex_lock(&dev->input_lock);
	ret = __imx091_s_frame_interval(sd, interval);
	mutex_unlock(&dev->input_lock);

	return ret;
}

static int imx091_g_skip_frames(struct v4l2_subdev *sd, u32 *frames)
{
	struct imx091_device *dev = to_imx091_sensor(sd);

	mutex_lock(&dev->input_lock);
	*frames = dev->curr_res_table[dev->fmt_idx].skip_frames;
	mutex_unlock(&dev->input_lock);

	return 0;
}

static const struct v4l2_subdev_video_ops imx091_video_ops = {
	.s_stream = imx091_s_stream,
	.enum_framesizes = imx091_enum_framesizes,
	.enum_frameintervals = imx091_enum_frameintervals,
	.enum_mbus_fmt = imx091_enum_mbus_fmt,
	.try_mbus_fmt = imx091_try_mbus_fmt,
	.g_mbus_fmt = imx091_g_mbus_fmt,
	.s_mbus_fmt = imx091_s_mbus_fmt,
	.g_frame_interval = imx091_g_frame_interval,
	.s_frame_interval = imx091_s_frame_interval,
};

static const struct v4l2_subdev_sensor_ops imx091_sensor_ops = {
	.g_skip_frames	= imx091_g_skip_frames,
};

static const struct v4l2_subdev_core_ops imx091_core_ops = {
	.g_chip_ident = imx091_g_chip_ident,
	.queryctrl = imx091_queryctrl,
	.g_ctrl = imx091_g_ctrl,
	.s_ctrl = imx091_s_ctrl_old,
	.s_power = imx091_s_power,
	.ioctl = imx091_ioctl,
	.init = imx091_init,
};

/* REVISIT: Do we need pad operations? */
static const struct v4l2_subdev_pad_ops imx091_pad_ops = {
	.enum_mbus_code = imx091_enum_mbus_code,
	.enum_frame_size = imx091_enum_frame_size,
	.get_fmt = imx091_get_pad_format,
	.set_fmt = imx091_set_pad_format,
};

static const struct v4l2_subdev_ops imx091_ops = {
	.core = &imx091_core_ops,
	.video = &imx091_video_ops,
	.pad = &imx091_pad_ops,
	.sensor = &imx091_sensor_ops,
};

static const struct media_entity_operations imx091_entity_ops = {
	.link_setup = NULL,
};

static int imx091_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct imx091_device *dev = to_imx091_sensor(sd);
	if (dev->platform_data->platform_deinit)
		dev->platform_data->platform_deinit();

	media_entity_cleanup(&dev->sd.entity);
	v4l2_ctrl_handler_free(&dev->ctrl_handler);
	dev->platform_data->csi_cfg(sd, 0);
	v4l2_device_unregister_subdev(sd);
	kfree(dev);

	return 0;
}

static const struct v4l2_ctrl_ops ctrl_ops = {
	.s_ctrl = imx091_s_ctrl,
};

static const char * const ctrl_run_mode_menu[] = {
	NULL,
	"Video",
	"Still capture",
	"Continuous capture",
	"Preview",
};

static const struct v4l2_ctrl_config ctrl_run_mode = {
	.ops = &ctrl_ops,
	.id = V4L2_CID_RUN_MODE,
	.name = "run mode",
	.type = V4L2_CTRL_TYPE_MENU,
	.min = 1,
	.def = 4,
	.max = 4,
	.qmenu = ctrl_run_mode_menu,
};

static int imx091_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	struct imx091_device *dev;
	int ret;
	
       printk("enter imx091_probe\n");
	b_probe = false;
	/* allocate sensor device & init sub device */
	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev) {
		v4l2_err(client, "%s: out of memory\n", __func__);
		return -ENOMEM;
	}

	mutex_init(&dev->input_lock);
	dev->fmt_idx = 0;
	v4l2_i2c_subdev_init(&(dev->sd), client, &imx091_ops);

	if (client->dev.platform_data) {
		ret = imx091_s_config(&dev->sd, client->irq,
				      client->dev.platform_data);
		if (ret)
			goto out_free;
	}

	dev->sd.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	dev->pad.flags = MEDIA_PAD_FL_SOURCE;
	dev->sd.entity.ops = &imx091_entity_ops;
	dev->sd.entity.type = MEDIA_ENT_T_V4L2_SUBDEV_SENSOR;
	dev->format.code = V4L2_MBUS_FMT_SBGGR10_1X10;

	ret = v4l2_ctrl_handler_init(&dev->ctrl_handler, 1);
	if (ret) {
		imx091_remove(client);
		return ret;
	}
	dev->run_mode = v4l2_ctrl_new_custom(&dev->ctrl_handler,
					     &ctrl_run_mode, NULL);

	if (dev->ctrl_handler.error) {
		imx091_remove(client);
		return dev->ctrl_handler.error;
	}

	/* Use same lock for controls as for everything else. */
	dev->ctrl_handler.lock = &dev->input_lock;
	dev->sd.ctrl_handler = &dev->ctrl_handler;
	v4l2_ctrl_handler_setup(&dev->ctrl_handler);
	ret = media_entity_init(&dev->sd.entity, 1, &dev->pad, 0);
	if (ret) {
		imx091_remove(client);
		return ret;
	}
	b_probe = true;
	printk("exit imx091_probe\n");
	return 0;

out_free:
	v4l2_device_unregister_subdev(&dev->sd);
	kfree(dev);
	return ret;
}

static const struct i2c_device_id imx091_id[] = {
	{IMX091_NAME, 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, imx091_id);

static struct i2c_driver imx091_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = IMX091_NAME,
	},
	.probe = imx091_probe,
	.remove = imx091_remove,
	.id_table = imx091_id,
};

static __init int imx091_init_mod(void)
{
	return i2c_add_driver(&imx091_driver);
}

static __exit void imx091_exit_mod(void)
{
	i2c_del_driver(&imx091_driver);
}

module_init(imx091_init_mod);
module_exit(imx091_exit_mod);

MODULE_DESCRIPTION("A low-level driver for Omnivision IMX091 sensors");
MODULE_LICENSE("GPL");
