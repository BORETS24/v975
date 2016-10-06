/*
 * Support for bu64291 actuator.
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

#include <linux/atomisp.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/types.h>
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>
#include <media/v4l2-chip-ident.h>

#include <media/bu64291.h>

#define to_bu64291_dev(sd) container_of(sd, struct bu64291_dev, sd)
extern  u16 g_infinity ;
#if 0
static int
__bu64291_read_reg(struct i2c_client *client, u16 addr, u16 len, u8 reg,
		    u8 *val)
{
	int err;
	struct i2c_msg msg[2] = {
		{
			.addr = addr,
			.flags = 0,
			.len = sizeof(reg),
			.buf = &reg,
		}, {
			.addr = addr,
			.len = len,
			.flags = I2C_M_RD,
			.buf = val,
		}
	};

	if (!client->adapter) {
		v4l2_err(client, "%s: error, no client->adapter\n", __func__);
		return -ENODEV;
	}

	if (len > BU64291_MAX_I2C_MSG_LEN) {
		v4l2_err(client, "%s: error, too big message length\n",
			 __func__);
		return -EINVAL;
	}

	err = i2c_transfer(client->adapter, msg, 2);
	if (err != ARRAY_SIZE(msg)) {
		if (err >= 0)
			err = -EIO;
		goto error;
	}

	return 0;

error:
	v4l2_err(client, "%s: read from offset 0x%x error %d", __func__, reg,
		 err);
	return err;
}
#if 0
static int
bu64291_read_reg(struct i2c_client *client, u16 len, u8 reg, void *val)
{
	return __bu64291_read_reg(client, client->addr, len, reg, val);
}
#endif
static int
bu64291_i2c_write(struct i2c_client *client, u16 len, u8 *data)
{
	struct i2c_msg msg;
	const int num_msg = 1;
	int ret;

	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = len;
	msg.buf = data;

	ret = i2c_transfer(client->adapter, &msg, 1);

	if (ret == num_msg)
		return 0;

	return ret > 0 ? -EIO : ret;
}

static int
__bu64291_write_reg(struct i2c_client *client, u16 data_length, u8 reg,
		     u8 *val)
{
	u8 data[3];
	const u16 len = sizeof(u8) + data_length; /* 8-bit address + data */

	if (!client->adapter) {
		v4l2_err(client, "%s: error, no client->adapter\n", __func__);
		return -ENODEV;
	}
	if (data_length > 2) {
		v4l2_err(client, "%s: write error: invalid length type %d\n",
			 __func__, data_length);
		return -EINVAL;
	}

	data[0] = reg;
	data[1] = val[0];
	//if (data_length == 2)
	//	data[2] = val[1];

	return bu64291_i2c_write(client, len, data);
}
#endif
static int
bu64291_read_reg(struct i2c_client *client, u8 reg, u8 *val)
{
	u8 data [2];
	struct i2c_msg msg[2];
	int ret;
/*
	msg[0].addr = client->addr;
	msg[0].flags = 0;
	msg[0].len = 1;
	msg[0].buf = data;

	data[0] = (u8) (reg);

	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 1;
	msg[1].buf = data + 1;

	*val = 0;

	
	ret = i2c_transfer(client->adapter, &msg, 2);
	if (ret >= 0) {
		*val = data[1];
		return 0;
	}*/
	msg[0].addr = client->addr;
	msg[0].flags = 0;
	msg[0].len = 1;
	msg[0].buf = data;


	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 1;
	msg[1].buf = data + 1;

	msg[2].addr = client->addr;
	msg[2].flags = I2C_M_RD;
	msg[2].len = 1;
	msg[2].buf = data + 2;

	*val = 0;

	
	ret = i2c_transfer(client->adapter, &msg, 3);
	if (ret >= 0) {
		*val = data[1];
		*(val+1) = data[2];
		return 0;
	}
	else
	{
		return -1;
	}
}

static int
bu64291_write_reg(struct i2c_client *client, u8 reg, u8 val)
{
	struct i2c_msg msg;
	unsigned char data[2] = { reg, val};
	int ret;

	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = 2;
	msg.buf = data;
	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret < 0) {
		dev_err(&client->dev, "Failed writing register 0x%04x!\n", reg);
		return ret;
	}

	return 0;
}
static int bu64291_wait_focus(struct v4l2_subdev *sd);
static int bu64291_q_focus_abs(struct v4l2_subdev *sd, s32 *value);
static int __bu64291_q_focus_abs(struct v4l2_subdev *sd, s16 *pos);

static int bu64291_t_focus_abs(struct v4l2_subdev *sd, s32 value)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct bu64291_dev *dev = to_bu64291_dev(sd);
	int ret;
	s16 step;
	s16 focus;
	u8 reg;
	v4l2_err(client, "bu64291_t_focus_abs value =%d,value =%x\n",
			 value);

       reg =(value>>8) & 0x03 |0xF4;  // only for send focus target data
       value = value &0xff;
       ret = bu64291_write_reg(client, reg, value);
	 if (ret < 0) {
		v4l2_err(client, "%s: error when setting new position\n",
			 __func__);
		return ret;
	} 
	return 0;
}

static int bu64291_wait_focus(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int retry, ret;
	u8 val;

	retry = BU64291_VCM_SLEW_RETRY_MAX;
	while (--retry) {
		s16 pos;
		usleep_range(1000, 1500);
		ret = __bu64291_q_focus_abs(sd, &pos);
		if (ret < 0) {
			v4l2_err(client, "%s: error when checking current "
					 "position\n", __func__);
			return ret;
		}

		if (pos == BU64291_VCM_INVALID_MIN_POS ||
		    pos == BU64291_VCM_INVALID_MAX_POS) {
			v4l2_err(client, "%s: invalid position\n", __func__);
			return ret;
		}

		ret = bu64291_read_reg(client, 
					BU64291_REG_STMVEN, &val);
		if (ret < 0) {
			v4l2_err(client, "%s: error when checking if new "
					 "position is ready\n", __func__);
			return ret;
		}
		if (!(val & BU64291_REG_STMVEN_MOVE_BUSY))
			break;
	}
	if (!retry) {
		v4l2_err(client, "%s: timeout when setting new position\n",
			 __func__);
		/* Clear move bit */
		bu64291_write_reg(client, BU64291_REG_STMVEN, 0);
		return -ETIMEDOUT;
	}

	usleep_range(5000, 5500);

	retry = BU64291_VCM_STAB_RETRY_MAX;
	while (--retry) {
		usleep_range(1000, 1500);
		ret = bu64291_read_reg(client, 
					BU64291_REG_MSSET, &val);
		if (ret < 0) {
			v4l2_err(client, "%s: error when checking if new "
					 "position is ready\n", __func__);
			return ret;
		}
		if (!(val & BU64291_REG_MSSET_BUSY))
			break;
	}
	if (!retry) {
		v4l2_err(client, "%s: timeout when waiting for lens "
			 "stabilization\n", __func__);
		return -ETIMEDOUT;
	}

	return 0;
}

static int bu64291_t_focus_rel(struct v4l2_subdev *sd, s32 value)
{
	struct bu64291_dev *dev = to_bu64291_dev(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	s16 focus;
	int ret;
	#if 0
	ret = __bu64291_q_focus_abs(sd, &focus);
	if (ret < 0) {
		v4l2_info(client, "%s: invalid focus position, assuming "
				  "default\n", __func__);
		focus = (dev->af_tun.focus_abs_max +
					dev->af_tun.focus_abs_min) / 2;
	} else {
		/* Normalizing focus value */
		focus >>= 6;
	}
	#endif
	v4l2_err(client, "bu64291_t_focus_rel value =%d,value =%x\n",
			 value);
	//return bu64291_t_focus_abs(sd, focus + value);
	return bu64291_t_focus_abs(sd, value);
}

static int bu64291_q_focus_status(struct v4l2_subdev *sd, s32 *value)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int busy;
	int ret;
	u8 val;
	v4l2_err(client, "bu64291_q_focus_status \n");
	#if 0
	ret = bu64291_read_reg(client, BU64291_8BIT,
			BU64291_REG_STMVEN, &val);
	if (ret < 0) {
		v4l2_err(client, "%s: error when checking if new "
				 "position is ready\n", __func__);
		return ret;
	}
	busy = val & BU64291_REG_STMVEN_MOVE_BUSY;
	if (busy)
		goto out;

	ret = bu64291_read_reg(client, bu64291_8BIT,
			BU64291_REG_MSSET, &val);
	if (ret < 0) {
		v4l2_err(client, "%s: error when checking if lens "
				 "stabilization is ready\n", __func__);
		return ret;
	}
	busy = val & BU64291_REG_MSSET_BUSY;

out:
	if (busy) {
		*value = ATOMISP_FOCUS_STATUS_MOVING |
			 ATOMISP_FOCUS_HP_IN_PROGRESS;
	} else {
		*value = ATOMISP_FOCUS_STATUS_ACCEPTS_NEW_MOVE |
			 ATOMISP_FOCUS_HP_COMPLETE;
	}
	#endif
	*value = ATOMISP_FOCUS_STATUS_ACCEPTS_NEW_MOVE |
			 ATOMISP_FOCUS_HP_COMPLETE;
	return 0;
}

static int __bu64291_q_focus_abs(struct v4l2_subdev *sd, s16 *pos)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret;
       v4l2_err(client, "__bu64291_q_focus_abs \n");
	ret = bu64291_read_reg(client,  BU64291_REG16_RZ,
				pos);
	if (ret < 0) {
		v4l2_err(client, "%s: failed to read lens position\n",
			 __func__);
		return ret;
	}

	*pos = be16_to_cpu(*pos);
	if (*pos == BU64291_VCM_INVALID_MIN_POS ||
	    *pos == BU64291_VCM_INVALID_MAX_POS)
		return -EINVAL;
	*pos >>= 6;

	return 0;
}

static int bu64291_q_focus_abs(struct v4l2_subdev *sd, s32 *value)
{
	s16 focus;
	int ret;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
        v4l2_err(client, "bu64291_q_focus_abs \n");
	ret = __bu64291_q_focus_abs(sd, &focus);
	if (ret < 0)
		return ret;

	*value = focus;

	return 0;
}

static int bu64291_t_vcm_slew(struct v4l2_subdev *sd, s32 value)
{
	struct bu64291_dev *dev = to_bu64291_dev(sd);

	if (value & ~BU64291_VCM_SLEW_MASK || value > BU64291_VCM_SLEW_MAX)
		return -EINVAL;

	dev->step = value;

	return 0;
}

static int bu64291_t_vcm_timing(struct v4l2_subdev *sd, s32 value)
{
	struct bu64291_dev *dev = to_bu64291_dev(sd);

	if (value < 0 || value > BU64291_VCM_SLEW_TIME_MAX)
		return -EINVAL;

	dev->timing = value;

	return 0;
}

static struct bu64291_control bu64291_controls[] = {
	{
		.qc = {
			.id = V4L2_CID_FOCUS_ABSOLUTE,
			.type = V4L2_CTRL_TYPE_INTEGER,
			.name = "focus move absolute",
			.step = 1,
			.default_value = 0,
			.flags = 0,
		},
		.tweak = bu64291_t_focus_abs,
		.query = bu64291_q_focus_abs,
	},
	{
		.qc = {
			.id = V4L2_CID_FOCUS_RELATIVE,
			.type = V4L2_CTRL_TYPE_INTEGER,
			.name = "focus move relative",
			.step = 1,
			.default_value = 0,
			.flags = 0,
		},
		.tweak = bu64291_t_focus_rel,
	},
	{
		.qc = {
			.id = V4L2_CID_FOCUS_STATUS,
			.type = V4L2_CTRL_TYPE_INTEGER,
			.name = "focus status",
			.step = 1,
			.default_value = 0,
			.flags = 0,
		},
		.query = bu64291_q_focus_status,
	},
	{
		.qc = {
			.id = V4L2_CID_VCM_SLEW,
			.type = V4L2_CTRL_TYPE_INTEGER,
			.name = "vcm slew",
			.step = 1,
			.default_value = BU64291_VCM_SLEW_DEFAULT,
			.flags = 0,
		},
		.tweak = bu64291_t_vcm_slew,
	},
	{
		.qc = {
			.id = V4L2_CID_VCM_TIMEING,
			.type = V4L2_CTRL_TYPE_INTEGER,
			.name = "vcm step time",
			.step = 1,
			.default_value = 0,
			.flags = 0,
		},
		.tweak = bu64291_t_vcm_timing,
	},
};
#define N_CONTROLS (ARRAY_SIZE(bu64291_controls))

static struct bu64291_control *bu64291_find_control(u32 id)
{
	int i;

	for (i = 0; i < N_CONTROLS; i++)
		if (bu64291_controls[i].qc.id == id)
			return &bu64291_controls[i];
	return NULL;
}

static int bu64291_queryctrl(struct v4l2_subdev *sd, struct v4l2_queryctrl *qc)
{
	struct bu64291_dev *dev = to_bu64291_dev(sd);
	struct bu64291_control *ctrl = bu64291_find_control(qc->id);

	if (ctrl == NULL)
		return -EINVAL;

	mutex_lock(&dev->input_lock);
	*qc = ctrl->qc;
	mutex_unlock(&dev->input_lock);

	return 0;
}

static int bu64291_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct bu64291_dev *dev = to_bu64291_dev(sd);
	struct bu64291_control *s_ctrl;
	int ret;

	s_ctrl = bu64291_find_control(ctrl->id);
	if ((s_ctrl == NULL) || (s_ctrl->query == NULL))
		return -EINVAL;

	mutex_lock(&dev->input_lock);
	ret = s_ctrl->query(sd, &ctrl->value);
	mutex_unlock(&dev->input_lock);

	return ret;
}

static int bu64291_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct bu64291_dev *dev = to_bu64291_dev(sd);
	struct bu64291_control *octrl = bu64291_find_control(ctrl->id);
	int ret;

	if ((octrl == NULL) || (octrl->tweak == NULL))
		return -EINVAL;

	mutex_lock(&dev->input_lock);
	ret = octrl->tweak(sd, ctrl->value);
	mutex_unlock(&dev->input_lock);

	return ret;
}

/*
 * Normalize AF tuning values in 2 steps:
 * 1: Values come in big endian format
 * 2: Values are s10 written in the most significant 10 bits of s16.
 */
static void bu64291_get_af_tuning_value(unsigned char *buf, int offset,
					 s16 *value)
{
	*value = (s16)be16_to_cpu(*(s16 *)&buf[offset]);
	*value >>= 6;
}

static int bu64291_get_af_tuning(struct v4l2_subdev *sd)
{
	/*struct bu64291_dev *dev = to_bu64291_dev(sd);

	if (!dev->eeprom_buf || dev->eeprom_size < bu64291_EEP_AF_TUN_END)
		return -ENODEV;

	bu64291_get_af_tuning_value(dev->eeprom_buf, bu64291_EEP_INF1,
				     &dev->af_tun.focus_abs_min);
	bu64291_get_af_tuning_value(dev->eeprom_buf, bu64291_EEP_MAC1,
				     &dev->af_tun.focus_abs_max);
	bu64291_get_af_tuning_value(dev->eeprom_buf, bu64291_EEP_INF2,
				     &dev->af_tun.inf_pos);
	bu64291_get_af_tuning_value(dev->eeprom_buf, bu64291_EEP_MAC2,
				     &dev->af_tun.mac_pos); */

	return 0;
}
#if 0
/*
 * Read EEPROM data from the EEPROM chip and store
 * it into a kmalloced buffer. On error return NULL.
 * The caller must kfree the buffer when no more needed.
 * @size: set to the size of the returned EEPROM data.
 */
static void *bu64291_eeprom_read(struct i2c_client *client, int *size)
{
	struct i2c_msg msg[2];
	int addr;
	char *buffer;

	buffer = kmalloc(BU64291_EEP_SIZE, GFP_KERNEL);
	if (!buffer)
		return NULL;

	memset(msg, 0, sizeof(msg));
	for (addr = 0;
	     addr < BU64291_EEP_SIZE; addr += BU64291_MAX_I2C_MSG_LEN) {
		unsigned int i2c_addr = BU64291_EEP_ID_BASE;
		unsigned char addr_buf;
		int r;

		i2c_addr += addr >> 8;
		addr_buf = addr & 0xFF;

		msg[0].addr = i2c_addr;
		msg[0].flags = 0;
		msg[0].len = 1;
		msg[0].buf = &addr_buf;

		msg[1].addr = i2c_addr;
		msg[1].flags = I2C_M_RD;
		msg[1].len = min(BU64291_MAX_I2C_MSG_LEN,
				 BU64291_EEP_SIZE - addr);
		msg[1].buf = &buffer[addr];

		r = i2c_transfer(client->adapter, msg, ARRAY_SIZE(msg));
		if (r != ARRAY_SIZE(msg)) {
			kfree(buffer);
			dev_err(&client->dev, "read failed at 0x%03x\n", addr);
			return NULL;
		}
	}

	if (size)
		*size = BU64291_EEP_SIZE;
	return buffer;
}
#endif
static int bu64291_init_registers(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct bu64291_dev *dev = to_bu64291_dev(sd);
	struct bu64291_eeprom_data *edata;
	int i;

	/* Read actuator initialization registers list from EEPROM */

	/*if (!dev->eeprom_buf ||
	    dev->eeprom_size <
	    BU64291_EEP_NUM_DATA * sizeof(struct bu64291_eeprom_data) +
	    BU64291_EEP_START_ADDR)
		return -ENODEV;

	edata = (struct bu64291_eeprom_data *)
		(dev->eeprom_buf + BU64291_EEP_START_ADDR);*/

	for (i = 0; i < BU64291_EEP_NUM_DATA; i++, edata++) {
		int len;
		int ret;
		u8 dest_data[2];

	/*	if (edata->addr == BU64291_EEP_ADDR_EOF)
			break;

		if (edata->addr == BU64291_EEP_ADDR_DELAY) {
			u16 delay = be16_to_cpu(*(u16 *)edata->data);
			usleep_range(delay * 1000, delay * 1500);
			continue;
		}

		len = edata->size;
		if (len > BU64291_EEP_DATA_SIZE_MAX) {
			v4l2_err(client, "%s: malformed initialization data "
				 "from EEPROM: data size too long.\n",
				 __func__);
			return -EINVAL;
		} */

		switch (edata->type) {
		case BU64291_EEP_DATA_DIRECT:
			dest_data[0] = edata->data[0];
			dest_data[1] = edata->data[1];
			break;
		case BU64291_EEP_DATA_INDIRECT_EEP:
			if (len > sizeof(dest_data) ||
			    edata->data[0] + len > dev->eeprom_size) {
				v4l2_err(client, "%s: error reading indirect "
					 "data from EEPROM.\n", __func__);
				return -ENODEV;
			}
			memcpy(dest_data, &dev->eeprom_buf[edata->data[0]],
			       len);
			break;
		case BU64291_EEP_DATA_INDIRECT_HVCA:
			ret = bu64291_read_reg(client,  edata->data[0],
						dest_data);
			if (ret < 0) {
				v4l2_err(client, "%s: error reading indirect "
						 "data from HVCA.\n", __func__);
				return ret;
			}
			break;
		case BU64291_EEP_DATA_MASK_AND:
			ret = bu64291_read_reg(client, edata->addr,
						dest_data);
			if (ret < 0) {
				v4l2_err(client, "%s: error reading indirect "
						 "data from HVCA.\n", __func__);
				return ret;
			}
			dest_data[0] &= edata->data[0];
			dest_data[1] &= edata->data[1];
			break;
		case BU64291_EEP_DATA_MASK_OR:
			ret = bu64291_read_reg(client,  edata->addr,
						dest_data);
			if (ret < 0) {
				v4l2_err(client, "%s: error reading indirect "
						 "data from HVCA.\n", __func__);
				return ret;
			}
			dest_data[0] |= edata->data[0];
			dest_data[1] |= edata->data[1];
			break;
		default:
			v4l2_err(client, "%s: malformed initialization data "
				 "from EEPROM: unknown data type.\n", __func__);
			return -EINVAL;
		}

		ret = bu64291_write_reg(client,  edata->addr,
					   dest_data);
		if (ret < 0) {
			v4l2_err(client, "%s: error writing initialization "
					 "data.\n", __func__);
			return ret;
		}
	}

	return 0;
}

static int bu64291_init(struct v4l2_subdev *sd, u32 val)
{
	struct bu64291_dev *dev = to_bu64291_dev(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	s32 point_b_value ;
	s32 point_a_value; 
	int ret;
	mutex_lock(&dev->input_lock);
	point_b_value =  g_infinity - 200-1;
	point_a_value =   point_b_value - 40 ;
	if(point_a_value <= 0)
	{
		point_a_value = 1;
		point_b_value = 41;
	}
	v4l2_err(client, "bu64291_init point_ g_vcm[0] =%d\n",
			 g_infinity);
	v4l2_err(client, "bu64291_init point_b_value =%d,point_a_value =%d\n",
			 point_b_value,point_a_value);
	
	/* set inital registers */
	//ret = bu64291_write_reg(client, BU64291_SETTING_POINT_A, BU64291_POINTA_VALUE);
	ret = bu64291_write_reg(client, BU64291_SETTING_POINT_A|(0x3 &(point_a_value>>8)), point_a_value&0xff);
	if (ret)
		goto out;
	ret = bu64291_write_reg(client, BU64291_SETTING_POINT_B|(0x3 &(point_b_value>>8)), point_b_value&0xff);
	if (ret)
		goto out;
	/* setting piont a to b ,slew_rate fasted mode 15ms and resonance frequency 80hz */
	ret = bu64291_write_reg(client, BU64291_SETTING_POINT_ATOB, BU64291_POINTATOB_VALUE);
	if (ret)
		goto out;
		ret = bu64291_write_reg(client, 0x8c, 0x2b);
	if (ret)
		goto out;
out:
	mutex_unlock(&dev->input_lock);
	return ret;
}

static int bu64291_power_up(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct bu64291_dev *dev = to_bu64291_dev(sd);
	int ret;

       /* power control */
	//ret = dev->platform_data->power_ctrl(sd, 1); // need to power on ? yaoling 
	//if (ret)
	//	goto fail_power;

	/* flis clock control */
	//ret = dev->platform_data->flisclk_ctrl(sd, 1);  // need to clock ? yaoling 
	//if (ret)
	//	goto fail_clk;

	/* gpio ctrl */
	ret = dev->platform_data->gpio_ctrl(sd, 1);
	if (ret)
		dev_err(&client->dev, "gpio failed 1\n");
	msleep(20);

	return 0;

fail_clk:
	dev->platform_data->flisclk_ctrl(sd, 0);
fail_power:
	dev->platform_data->power_ctrl(sd, 0);
	dev_err(&client->dev, "sensor power-up failed\n");

	return ret;
}

static int bu64291_power_down(struct v4l2_subdev *sd)
{
	struct bu64291_dev *dev = to_bu64291_dev(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret;
      #if 0
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
	#endif
	return 0;
}

static int bu64291_g_priv_int_data(struct v4l2_subdev *sd,
				    struct v4l2_private_int_data *priv)
{
	struct bu64291_dev *dev = to_bu64291_dev(sd);

	if (copy_to_user(priv->data, dev->eeprom_buf,
			 min(priv->size, dev->eeprom_size)))
		return -EFAULT;
	priv->size = dev->eeprom_size;

	return 0;
}

static long bu64291_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	switch (cmd) {
	case ATOMISP_IOC_G_MOTOR_PRIV_INT_DATA:
		return bu64291_g_priv_int_data(sd, arg);
	default:
		return -EINVAL;
	}
	return 0;
}

static const struct v4l2_subdev_core_ops bu64291_core_ops = {
	.queryctrl = bu64291_queryctrl,
	.g_ctrl = bu64291_g_ctrl,
	.s_ctrl = bu64291_s_ctrl,
	.init = bu64291_init,
	.ioctl = bu64291_ioctl,
};

static const struct v4l2_subdev_ops bu64291_ops = {
	.core = &bu64291_core_ops,
};

static int bu64291_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct bu64291_dev *dev = to_bu64291_dev(sd);

	kfree(dev->eeprom_buf);
	v4l2_device_unregister_subdev(sd);
	kfree(dev);

	return 0;
}

static int bu64291_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	struct bu64291_dev *dev;
	int ret;
	 dev_err(&client->dev, "enter bu64291_probe\n");
	/* allocate sensor device & init sub device */
	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev) {
		v4l2_err(client, "%s: out of memory\n", __func__);
		return -ENOMEM;
	}

	mutex_init(&dev->input_lock);

	v4l2_i2c_subdev_init(&dev->sd, client, &bu64291_ops);

	dev->platform_data = client->dev.platform_data;
	if (!dev->platform_data)
		v4l2_info(client, "%s: driver has no platform data\n",
			  __func__);

	// bu64291_power_up(&dev->sd);  // think it not needed

	/*dev->eeprom_buf = bu64291_eeprom_read(client, &dev->eeprom_size);
	if (!dev->eeprom_buf) {
		ret = -ENODEV;
		v4l2_err(client, "%s: failed to read EEPROM\n",
			 __func__);
		goto err;
	}*/

	ret = bu64291_get_af_tuning(&dev->sd);
	if (ret) {
		v4l2_err(client, "%s: failed to read AF tuning data\n",
			 __func__);
		goto err;
	}

	// bu64291_power_down(&dev->sd);  // same as 932

	//v4l2_info(client, "bu64291 actuator successfully initialized\n");
	dev_err(&client->dev, "exit  bu64291_probe\n");

	return 0;

err:
	bu64291_remove(client);
	return ret;
}

static const struct i2c_device_id bu64291_id[] = {
	{BU64291_NAME, 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, bu64291_id);

static struct i2c_driver bu64291_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = BU64291_NAME,
	},
	.probe = bu64291_probe,
	.remove = bu64291_remove,
	.id_table = bu64291_id,
};

static __init int init_bu64291(void)
{
	return i2c_add_driver(&bu64291_driver);
}

static __exit void exit_bu64291(void)
{
	i2c_del_driver(&bu64291_driver);
}

module_init(init_bu64291);
module_exit(exit_bu64291);

MODULE_DESCRIPTION("A low-level driver for bu64291 actuator");
MODULE_AUTHOR("David Cohen <david.a.cohen@intel.com>");
MODULE_LICENSE("GPL");

