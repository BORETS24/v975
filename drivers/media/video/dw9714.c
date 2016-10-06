/*
 * Support for dw9714 focuser.
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

#include <media/dw9714.h>

#define DW9714_TRANSITION_MODE  0x05

#define to_dw9714_dev(sd) container_of(sd, struct dw9714_dev, sd)

static int dw9714_read_reg(struct i2c_client *client, u8 reg, u16 *val)
{
	u8 data [2];
	struct i2c_msg msg[2];
	int ret;

	msg[0].addr = client->addr;
	msg[0].flags = I2C_M_RD;
	msg[0].len = 1;
	msg[0].buf = data ;

	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 1;
	msg[1].buf = data + 1;

	*val = 0;
	
	ret = i2c_transfer(client->adapter, msg, 2);
	if (ret >= 0) {
		*val = (data[0]<< 8)|data[1];
		printk("dw9714_read_reg 0x%x\n", *val);
		return 0;
	}
	else
	{
		return -1;
	}
}

static int dw9714_write_reg(struct i2c_client *client, u8 reg, u8 val)
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

static int dw9714_wait_focus(struct v4l2_subdev *sd);
static int dw9714_q_focus_abs(struct v4l2_subdev *sd, s32 *value);
static int __dw9714_q_focus_abs(struct v4l2_subdev *sd, s16 *pos);

static int dw9714_t_focus_abs(struct v4l2_subdev *sd, s32 value)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret;
	u8 reg;
	u8 data;
	
	printk("dw9714_t_focus_abs value =%x\n", value);
	
	reg = (u8) ((value >> 4) & 0x3F);
       data = (u8) ((value & 0xF) << 4) | DW9714_TRANSITION_MODE;
       ret = dw9714_write_reg(client, reg, data);
	 if (ret < 0) {
		v4l2_err(client, "%s: error when setting new position\n",
			 __func__);
		return ret;
	} 
	return 0;
}

static int dw9714_wait_focus(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int retry, ret;
	u16 val;
	
       printk("dw9714_wait_focus++++\n");
	   
	retry = DW9714_VCM_SLEW_RETRY_MAX;
	while (--retry) {
		s16 pos;
		usleep_range(1000, 1500);
		ret = __dw9714_q_focus_abs(sd, &pos);
		if (ret < 0) {
			v4l2_err(client, "%s: error when checking current "
					 "position\n", __func__);
			return ret;
		}

		if (pos == DW9714_VCM_INVALID_MIN_POS ||
		    pos == DW9714_VCM_INVALID_MAX_POS) {
			v4l2_err(client, "%s: invalid position\n", __func__);
			return ret;
		}

		ret = dw9714_read_reg(client, 
					DW9714_REG_STMVEN, &val);
		if (ret < 0) {
			v4l2_err(client, "%s: error when checking if new "
					 "position is ready\n", __func__);
			return ret;
		}
		if (!(val & DW9714_REG_STMVEN_MOVE_BUSY))
			break;
	}
	if (!retry) {
		v4l2_err(client, "%s: timeout when setting new position\n",
			 __func__);
		/* Clear move bit */
		dw9714_write_reg(client, DW9714_REG_STMVEN, 0);
		return -ETIMEDOUT;
	}

	usleep_range(5000, 5500);

	retry = DW9714_VCM_STAB_RETRY_MAX;
	while (--retry) {
		usleep_range(1000, 1500);
		ret = dw9714_read_reg(client, 
					DW9714_REG_MSSET, &val);
		if (ret < 0) {
			v4l2_err(client, "%s: error when checking if new "
					 "position is ready\n", __func__);
			return ret;
		}
		if (!(val & DW9714_REG_MSSET_BUSY))
			break;
	}
	if (!retry) {
		v4l2_err(client, "%s: timeout when waiting for lens "
			 "stabilization\n", __func__);
		return -ETIMEDOUT;
	}

	return 0;
}

static int dw9714_t_focus_rel(struct v4l2_subdev *sd, s32 value)
{
	printk("dw9714_t_focus_rel value =0x%x\n", value);

	return dw9714_t_focus_abs(sd, value);
}

static int dw9714_q_focus_status(struct v4l2_subdev *sd, s32 *value)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	printk("dw9714_q_focus_status \n");
	*value = ATOMISP_FOCUS_STATUS_ACCEPTS_NEW_MOVE |
			 ATOMISP_FOCUS_HP_COMPLETE;
	return 0;
}

static int __dw9714_q_focus_abs(struct v4l2_subdev *sd, s16 *pos)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret;
	
       printk("__dw9714_q_focus_abs \n");
	   
	ret = dw9714_read_reg(client,  DW9714_REG16_RZ,
				pos);
	if (ret < 0) {
		v4l2_err(client, "%s: failed to read lens position\n",
			 __func__);
		return ret;
	}

	*pos = be16_to_cpu(*pos);
	if (*pos == DW9714_VCM_INVALID_MIN_POS ||
	    *pos == DW9714_VCM_INVALID_MAX_POS)
		return -EINVAL;
	*pos = (*pos >>4) | 0x3ff;
       printk("__dw9714_q_focus_abs: position = %d\n", *pos);
	return 0;
}

static int dw9714_q_focus_abs(struct v4l2_subdev *sd, s32 *value)
{
	s16 focus;
	int ret;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	
       printk("dw9714_q_focus_abs \n");
	   
	ret = __dw9714_q_focus_abs(sd, &focus);
	if (ret < 0)
		return ret;

	*value = focus;

	return 0;
}

static int dw9714_t_vcm_slew(struct v4l2_subdev *sd, s32 value)
{
	struct dw9714_dev *dev = to_dw9714_dev(sd);

	dev->step = value;

	return 0;
}

static int dw9714_t_vcm_timing(struct v4l2_subdev *sd, s32 value)
{
	struct dw9714_dev *dev = to_dw9714_dev(sd);

	if (value < 0 || value > DW9714_VCM_SLEW_TIME_MAX)
		return -EINVAL;

	dev->timing = value;

	return 0;
}

static struct dw9714_control dw9714_controls[] = {
	{
		.qc = {
			.id = V4L2_CID_FOCUS_ABSOLUTE,
			.type = V4L2_CTRL_TYPE_INTEGER,
			.name = "focus move absolute",
			.step = 1,
			.default_value = 0,
			.flags = 0,
		},
		.tweak = dw9714_t_focus_abs,
		.query = dw9714_q_focus_abs,
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
		.tweak = dw9714_t_focus_rel,
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
		.query = dw9714_q_focus_status,
	},
	{
		.qc = {
			.id = V4L2_CID_VCM_SLEW,
			.type = V4L2_CTRL_TYPE_INTEGER,
			.name = "vcm slew",
			.step = 1,
			.default_value = DW9714_VCM_SLEW_DEFAULT,
			.flags = 0,
		},
		.tweak = dw9714_t_vcm_slew,
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
		.tweak = dw9714_t_vcm_timing,
	},
};
#define N_CONTROLS (ARRAY_SIZE(dw9714_controls))

static struct dw9714_control *dw9714_find_control(u32 id)
{
	int i;

	for (i = 0; i < N_CONTROLS; i++)
		if (dw9714_controls[i].qc.id == id)
			return &dw9714_controls[i];
	return NULL;
}

static int dw9714_queryctrl(struct v4l2_subdev *sd, struct v4l2_queryctrl *qc)
{
	struct dw9714_dev *dev = to_dw9714_dev(sd);
	struct dw9714_control *ctrl = dw9714_find_control(qc->id);

	if (ctrl == NULL)
		return -EINVAL;

	mutex_lock(&dev->input_lock);
	*qc = ctrl->qc;
	mutex_unlock(&dev->input_lock);

	return 0;
}

static int dw9714_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct dw9714_dev *dev = to_dw9714_dev(sd);
	struct dw9714_control *s_ctrl;
	int ret;

	s_ctrl = dw9714_find_control(ctrl->id);
	if ((s_ctrl == NULL) || (s_ctrl->query == NULL))
		return -EINVAL;

	mutex_lock(&dev->input_lock);
	ret = s_ctrl->query(sd, &ctrl->value);
	mutex_unlock(&dev->input_lock);

	return ret;
}

static int dw9714_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct dw9714_dev *dev = to_dw9714_dev(sd);
	struct dw9714_control *octrl = dw9714_find_control(ctrl->id);
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
static void dw9714_get_af_tuning_value(unsigned char *buf, int offset,
					 s16 *value)
{
	*value = (s16)be16_to_cpu(*(s16 *)&buf[offset]);
	//*value >>= 6;
	*value = (*value >> 4) | 0x3ff;
}

static int dw9714_get_af_tuning(struct v4l2_subdev *sd)
{
	struct dw9714_dev *dev = to_dw9714_dev(sd);

	/*if (!dev->eeprom_buf || dev->eeprom_size < dw9714_EEP_AF_TUN_END)
		return -ENODEV;

	dw9714_get_af_tuning_value(dev->eeprom_buf, dw9714_EEP_INF1,
				     &dev->af_tun.focus_abs_min);
	dw9714_get_af_tuning_value(dev->eeprom_buf, dw9714_EEP_MAC1,
				     &dev->af_tun.focus_abs_max);
	dw9714_get_af_tuning_value(dev->eeprom_buf, dw9714_EEP_INF2,
				     &dev->af_tun.inf_pos);
	dw9714_get_af_tuning_value(dev->eeprom_buf, dw9714_EEP_MAC2,
				     &dev->af_tun.mac_pos); */
	/* yangchunni test only, need modify according OTP data */		
       dev->af_tun.focus_abs_min = 0;
	dev->af_tun.focus_abs_max = 1023;
	dev->af_tun.inf_pos = 110;
	dev->af_tun.mac_pos = 480;
	return 0;
}

static int dw9714_init(struct v4l2_subdev *sd, u32 val)
{
	struct dw9714_dev *dev = to_dw9714_dev(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret;

	mutex_lock(&dev->input_lock);
	/* set inital registers */
	ret = dw9714_write_reg(client, 0xEC, 0xA3);
	if (ret)
		goto out;
	ret = dw9714_write_reg(client, 0xA1, 0x05);
	if (ret)
		goto out;
	ret = dw9714_write_reg(client, 0xF2, 0x07 <<3);
	if (ret)
		goto out;
	ret = dw9714_write_reg(client, 0xDC, 0x51);
	if (ret)
		goto out;
	
	/* set VCM to home position */
	ret = dw9714_t_focus_abs(sd,
			(dev->af_tun.inf_pos + dev->af_tun.mac_pos) / 2);

out:
	mutex_unlock(&dev->input_lock);
	return ret;
}

static int dw9714_power_up(struct v4l2_subdev *sd)
{
	return 0;
}

static int dw9714_power_down(struct v4l2_subdev *sd)
{
	return 0;
}

static int dw9714_g_priv_int_data(struct v4l2_subdev *sd,
				    struct v4l2_private_int_data *priv)
{
	struct dw9714_dev *dev = to_dw9714_dev(sd);

	if (copy_to_user(priv->data, dev->eeprom_buf,
			 min(priv->size, dev->eeprom_size)))
		return -EFAULT;
	priv->size = dev->eeprom_size;

	return 0;
}

static long dw9714_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	switch (cmd) {
	case ATOMISP_IOC_G_MOTOR_PRIV_INT_DATA:
		return dw9714_g_priv_int_data(sd, arg);
	default:
		return -EINVAL;
	}
	return 0;
}

static const struct v4l2_subdev_core_ops dw9714_core_ops = {
	.queryctrl = dw9714_queryctrl,
	.g_ctrl = dw9714_g_ctrl,
	.s_ctrl = dw9714_s_ctrl,
	.init = dw9714_init,
	.ioctl = dw9714_ioctl,
};

static const struct v4l2_subdev_ops dw9714_ops = {
	.core = &dw9714_core_ops,
};

static int dw9714_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct dw9714_dev *dev = to_dw9714_dev(sd);

	kfree(dev->eeprom_buf);
	v4l2_device_unregister_subdev(sd);
	kfree(dev);

	return 0;
}

static int dw9714_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	struct dw9714_dev *dev;
	int ret;
	
	 printk("enter dw9714_probe\n");
	/* allocate sensor device & init sub device */
	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev) {
		v4l2_err(client, "%s: out of memory\n", __func__);
		return -ENOMEM;
	}

	mutex_init(&dev->input_lock);

	v4l2_i2c_subdev_init(&dev->sd, client, &dw9714_ops);

	dev->platform_data = client->dev.platform_data;
	if (!dev->platform_data)
		v4l2_info(client, "%s: driver has no platform data\n",
			  __func__);

	printk("exit  dw9714_probe\n");

	return 0;

}

static const struct i2c_device_id dw9714_id[] = {
	{DW9714_NAME, 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, dw9714_id);

static struct i2c_driver dw9714_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = DW9714_NAME,
	},
	.probe = dw9714_probe,
	.remove = dw9714_remove,
	.id_table = dw9714_id,
};

static __init int init_dw9714(void)
{
	return i2c_add_driver(&dw9714_driver);
}

static __exit void exit_dw9714(void)
{
	i2c_del_driver(&dw9714_driver);
}

module_init(init_dw9714);
module_exit(exit_dw9714);

MODULE_DESCRIPTION("A low-level driver for dw9714 actuator");
MODULE_AUTHOR("David Cohen <david.a.cohen@intel.com>");
MODULE_LICENSE("GPL");

