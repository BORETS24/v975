
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/kmod.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <media/v4l2-device.h>
#include <media/v4l2-chip-ident.h>
#include "ov9740.h"


#define to_ov9740_sensor(sd) container_of(sd, struct ov9740_device, sd)

/*
 * TODO: use debug parameter to actually define when debug messages should
 * be printed.
 */
static int debug;
module_param(debug, int, 0644);
MODULE_PARM_DESC(debug, "Debug level (0-1)");

static int ov9740_t_vflip(struct v4l2_subdev *sd, int value);
static int ov9740_t_hflip(struct v4l2_subdev *sd, int value);

/*
 * Low-level register I/O.
 * On most platforms, we'd rather do straight i2c I/O.
 */
 static int ov9740_read(struct i2c_client *c, u16 reg, u8 * value)
{
	struct i2c_client *client = c;
	struct i2c_msg msg[2];
	unsigned char data[3];
	int ret;

	/* high byte goes out first */
	data[0] = (u8) (reg >> 8);;
	data[1] = (u8) (reg& 0xff);

	msg[0].addr = client->addr;
	msg[0].flags = 0;
	msg[0].len = 2;
	msg[0].buf = data;

	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 1;
	msg[1].buf = data + 2;

	ret = i2c_transfer(client->adapter, msg, 2);
	if (ret < 0) {
		dev_err(&client->dev, "Failed reading register 0x%04x!\n", reg);
		return ret;
	}
	*value = data[2];
	return 0;

	
	
}


static int ov9740_reg_write(struct i2c_client *c, u16 reg, u8 value)
{
	struct i2c_client *client = c;
	struct i2c_msg msg;
       unsigned char data[3];
	int ret;

	data[0] = (u8) (reg >> 8);;
	data[1] = (u8) (reg & 0xff);
	data[2] = (u8) (value & 0xff);

	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = 3;
	msg.buf = data;

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret < 0) {
		dev_err(&client->dev, "Failed writing register 0x%04x!\n", reg);
		return ret;
	}

	return 0;
}

/*
 * Write a list of register settings; ff/ff stops the process.
 */
static int ov9740_reg_write_array(struct i2c_client *client,
				  const struct ov9740_reg *regarray,
				  int regarraylen)
{
	int i;
	int ret;

	for (i = 0; i < regarraylen; i++) {
		ret = ov9740_reg_write(client,
				       regarray[i].reg, regarray[i].val);
		if (ret < 0)
			return ret;
	}

	return 0;
}


static int ov9740_init_common(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret;
	dev_err(&client->dev, "enter ov9740_init_common\n");
	ret = ov9740_reg_write_array(client, ov9740_defaults,
				     ARRAY_SIZE(ov9740_defaults));
	if (ret < 0)
		return ret;
	ret = ov9740_reg_write_array(client, ov9740_regs_vga,
				     ARRAY_SIZE(ov9740_regs_vga));
				   /*  ret = ov9740_reg_write_array(client, ov9740_regs_720p,
				     ARRAY_SIZE(ov9740_regs_720p));*/
	if(ret < 0)
		return ret;
      dev_err(&client->dev, "exit ov9740_init_common\n");
	return 0 ;
}
static int power_up(struct v4l2_subdev *sd)
{
	struct ov9740_device *dev = to_ov9740_sensor(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret;

	if (NULL == dev->platform_data) {
		dev_err(&client->dev, "no camera_sensor_platform_data");
		return -ENODEV;
	}

	/* power control */
	ret = dev->platform_data->power_ctrl(sd, 1);
	if (ret)
		goto fail_power;

	/* flis clock control */
	ret = dev->platform_data->flisclk_ctrl(sd, 1);
	if (ret)
		goto fail_clk;

	/* gpio ctrl */
	ret = dev->platform_data->gpio_ctrl(sd, 1);
	if (ret)
	{
		dev_err(&client->dev, "gpio failed 1\n");
		goto fail_clk;
	}
	/*
	 * according to DS, 44ms is needed between power up and first i2c
	 * commend
	 */
	msleep(50);

	return 0;

fail_clk:
	dev->platform_data->flisclk_ctrl(sd, 0);
	dev->platform_data->power_ctrl(sd, 0);
fail_power:
	dev->platform_data->power_ctrl(sd, 0);
	dev_err(&client->dev, "sensor power-up failed\n");

	return ret;
}

static int power_down(struct v4l2_subdev *sd)
{
	struct ov9740_device *dev = to_ov9740_sensor(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret;

	if (NULL == dev->platform_data) {
		dev_err(&client->dev, "no camera_sensor_platform_data");
		return -ENODEV;
	}

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

	/*according to DS, 20ms is needed after power down*/
	msleep(20);

	return ret;
}

static int ov9740_s_power(struct v4l2_subdev *sd, int power)
{
	if (power == 0)
		return power_down(sd);
	else {
		if (power_up(sd))
			return -EINVAL;

		return ov9740_init_common(sd);
	}
}

static int ov9740_try_res(u32 *w, u32 *h)
{
	int i;

	/*
	 * The mode list is in ascending order. We're done as soon as
	 * we have found the first equal or bigger size.
	 */
	for (i = 0; i < N_RES; i++) {
		if ((ov9740_res[i].width >= *w) &&
		    (ov9740_res[i].height >= *h))
			break;
	}

	/*
	 * If no mode was found, it means we can provide only a smaller size.
	 * Returning the biggest one available in this case.
	 */
	if (i == N_RES)
		i--;

	*w = ov9740_res[i].width;
	*h = ov9740_res[i].height;

	return 0;
}

static struct ov9740_res_struct *ov9740_to_res(u32 w, u32 h)
{
	int  index;

	for (index = 0; index < N_RES; index++) {
		if ((ov9740_res[index].width == w) &&
		    (ov9740_res[index].height == h))
			break;
	}

	/* No mode found */
	if (index >= N_RES)
		return NULL;

	return &ov9740_res[index];
}

static int ov9740_try_mbus_fmt(struct v4l2_subdev *sd,
				struct v4l2_mbus_framefmt *fmt)
{
	struct ov9740_device *dev = to_ov9740_sensor(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	dev_err(&client->dev, " ov9740_try_mbus_fmt width =%d,height =%d\n",fmt->width,fmt->height);	
	return ov9740_try_res(&fmt->width, &fmt->height);
}

static int ov9740_res2size(unsigned int res, int *h_size, int *v_size)
{
	unsigned short hsize;
	unsigned short vsize;

	switch (res) {
	case OV9740_RES_QVGA:
		// dev_err(&client->dev, "case OV9740_RES_QVGA:\n");
		hsize = OV9740_RES_QVGA_SIZE_H;
		vsize = OV9740_RES_QVGA_SIZE_V;
		break;
	case OV9740_RES_VGA:
		 // dev_err(&client->dev, "case OV9740_RES_VGA:\n");
		hsize = OV9740_RES_VGA_SIZE_H;
		vsize = OV9740_RES_VGA_SIZE_V;
		break;
	case OV9740_RES_720P:
		
		hsize = OV9740_RES_720P_SIZE_H;
		vsize = OV9740_RES_720P_SIZE_V;
		break;
	case OV9740_RES_480P:
	
	hsize = OV9740_RES_480P_SIZE_H;
	vsize = OV9740_RES_480P_SIZE_V;
	break;
	default:
		WARN(1, "%s: Resolution 0x%08x unknown\n", __func__, res);
		return -EINVAL;
	}

	if (h_size != NULL)
		*h_size = hsize;
	if (v_size != NULL)
		*v_size = vsize;

	return 0;
}

static int ov9740_get_mbus_fmt(struct v4l2_subdev *sd,
				struct v4l2_mbus_framefmt *fmt)
{
	struct ov9740_device *dev = to_ov9740_sensor(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int width, height;
	int ret;

	ret = ov9740_res2size(dev->res, &width, &height);
	dev_err(&client->dev, "ov9740_get_mbus_fmt width =%d, height =%d\n",width,height);
	if (ret){
		dev_err(&client->dev, "err ov9740_get_mbus_fmt \n");	
		return ret;
	}
	fmt->width = width;
	fmt->height = height;

	return 0;
}

static int ov9740_set_mbus_fmt(struct v4l2_subdev *sd,
			      struct v4l2_mbus_framefmt *fmt)
{
	struct ov9740_device *dev = to_ov9740_sensor(sd);
	struct ov9740_res_struct *res_index;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	u32 width = fmt->width;
	u32 height = fmt->height;
	int ret;
	dev_err(&client->dev, "enter  ov9740_set_mbus_fmt \n");	
	ov9740_try_res(&width, &height);
	res_index = ov9740_to_res(width, height);
	dev_err(&client->dev, "ov9740_set_mbus_fmt width =%d,height =%d,res_indexres =%d\n",width,height,res_index->res);	

	/* Sanity check */
	if (unlikely(!res_index)) {
		WARN_ON(1);
		return -EINVAL;
	}

	switch (res_index->res) {
	case OV9740_RES_QVGA:
	
		ret = ov9740_reg_write_array(client, ov9740_regs_qvga,
					     ARRAY_SIZE(ov9740_regs_qvga));
		/* set sensor read_mode to Summing */
		
		break;
	case OV9740_RES_VGA:
		
		ret = ov9740_reg_write_array(client, ov9740_regs_vga,
					     ARRAY_SIZE(ov9740_regs_vga));
		/* set sensor read_mode to Summing */
		
		break;
	case OV9740_RES_720P:
		ret = ov9740_reg_write_array(client, ov9740_regs_720p,
					     ARRAY_SIZE(ov9740_regs_720p));
		/* set sensor read_mode to Normal */
		
		break;
	case OV9740_RES_480P:
	ret = ov9740_reg_write_array(client, ov9740_regs_480p,
				     ARRAY_SIZE(ov9740_regs_480p));
	/* set sensor read_mode to Normal */
	
	break;
	
	default:
		v4l2_err(sd, "set resolution: %d failed!\n", res_index->res);
		return -EINVAL;
	}

	if (ret){
		dev_err(&client->dev, "err  ov9740_set_mbus_fmt \n");	
		return -EINVAL;
	}
	dev->res = res_index->res;

	fmt->width = width;
	fmt->height = height;
       msleep(20);  // for cts of user version , add 20 ms delay by yaoling , it is suggested  from  intel.
	return 0;
}

/* Horizontal flip the image. */
static int ov9740_g_hflip(struct v4l2_subdev *sd, s32 * val)
{
	/*struct i2c_client *c = v4l2_get_subdevdata(sd);
	int ret;
	u32 data;
	ret = mt9m114_read_reg(c, MISENSOR_16BIT,
			(u32)MISENSOR_READ_MODE, &data);
	if (ret)
		return ret;
	*val = !!(data & MISENSOR_HFLIP_MASK);*/

	return 0;
}

static int ov9740_g_vflip(struct v4l2_subdev *sd, s32 * val)
{
	/*struct i2c_client *c = v4l2_get_subdevdata(sd);
	int ret;
	u32 data;

	ret = mt9m114_read_reg(c, MISENSOR_16BIT,
			(u32)MISENSOR_READ_MODE, &data);
	if (ret)
		return ret;
	*val = !!(data & MISENSOR_VFLIP_MASK);*/

	return 0;
}

static int ov9740_s_freq(struct v4l2_subdev *sd, s32  val)
{
	/*struct i2c_client *c = v4l2_get_subdevdata(sd);
	struct ov9740_device *dev = to_ov9740_sensor(sd);
	int ret;

	if (val != MT9M114_FLICKER_MODE_50HZ &&
			val != MT9M114_FLICKER_MODE_60HZ)
		return -EINVAL;

	if (val == MT9M114_FLICKER_MODE_50HZ) {
		ret = mt9m114_write_reg_array(c, mt9m114_antiflicker_50hz,
					POST_POLLING);
		if (ret < 0)
			return ret;
	} else {
		ret = mt9m114_write_reg_array(c, mt9m114_antiflicker_60hz,
					POST_POLLING);
		if (ret < 0)
			return ret;
	}

	if (ret == 0)
		dev->lightfreq = val;

	return ret;*/
	return 0;
}


static int ov9740_g_focal(struct v4l2_subdev *sd, s32 *value)
{
	*value = (OV9740_FOCAL_LENGTH_NUM << 16) | OV9740_FOCAL_LENGTH_DEM;
	return 0;
}
static int ov9740_g_fnumber(struct v4l2_subdev *sd, int value)
{
	return 0;
}
static int ov9740_g_fnumber_range(struct v4l2_subdev *sd, int value)
{
	return 0;
}
static struct ov9740_control ov9740_controls[] = {
	{
		.qc = {
			.id = V4L2_CID_VFLIP,
			.type = V4L2_CTRL_TYPE_INTEGER,
			.name = "Image v-Flip",
			.minimum = 0,
			.maximum = 1,
			.step = 1,
			.default_value = 0,
		},
		.query = ov9740_g_vflip,
		.tweak = ov9740_t_vflip,
	},
	{
		.qc = {
			.id = V4L2_CID_HFLIP,
			.type = V4L2_CTRL_TYPE_INTEGER,
			.name = "Image h-Flip",
			.minimum = 0,
			.maximum = 1,
			.step = 1,
			.default_value = 0,
		},
		.query = ov9740_g_hflip,
		.tweak = ov9740_t_hflip,
	},
	{
		.qc = {
			.id = V4L2_CID_FOCAL_ABSOLUTE,
			.type = V4L2_CTRL_TYPE_INTEGER,
			.name = "focal length",
			.minimum = OV9740_FOCAL_LENGTH_DEFAULT,
			.maximum = OV9740_FOCAL_LENGTH_DEFAULT,
			.step = 0x01,
			.default_value = OV9740_FOCAL_LENGTH_DEFAULT,
			.flags = 0,
		},
		.query = ov9740_g_focal,
	},
	{
		.qc = {
			.id = V4L2_CID_FNUMBER_ABSOLUTE,
			.type = V4L2_CTRL_TYPE_INTEGER,
			.name = "f-number",
			.minimum = OV9740_F_NUMBER_DEFAULT,
			.maximum = OV9740_F_NUMBER_DEFAULT,
			.step = 0x01,
			.default_value = OV9740_F_NUMBER_DEFAULT,
			.flags = 0,
		},
		.query =ov9740_g_fnumber,
	},
	{
		.qc = {
			.id = V4L2_CID_FNUMBER_RANGE,
			.type = V4L2_CTRL_TYPE_INTEGER,
			.name = "f-number range",
			.minimum = OV9740_F_NUMBER_RANGE,
			.maximum =  OV9740_F_NUMBER_RANGE,
			.step = 0x01,
			.default_value = OV9740_F_NUMBER_RANGE,
			.flags = 0,
		},
		.query = ov9740_g_fnumber_range,
	},
	{
		.qc = {
			.id = V4L2_CID_POWER_LINE_FREQUENCY,
			.type = V4L2_CTRL_TYPE_MENU,
			.name = "Light frequency filter",
			.minimum = 1,
			.maximum =  2, /* 1: 50Hz, 2:60Hz */
			.step = 1,
			.default_value = 1,
			.flags = 0,
		},
		.tweak = ov9740_s_freq,
	},
	/*{
		.qc = {
			.id = V4L2_CID_2A_STATUS,
			.type = V4L2_CTRL_TYPE_BITMASK,
			.name = "AE and AWB status",
			.minimum = 0,
			.maximum = V4L2_2A_STATUS_AE_READY |
				V4L2_2A_STATUS_AWB_READY,
			.step = 1,
			.default_value = 0,
			.flags = 0,
		},
		.query = mt9m114_g_2a_status,
	},*/

};




static int ov9740_detect(struct ov9740_device *dev, struct i2c_client *client)
{
	struct i2c_adapter *adapter = client->adapter; //
	u32 retvalue;
       u8 modelhi, modello;
	   u16 model;
	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev, "%s: i2c error", __func__);
		return -ENODEV;
	}
      /*	 * check and show product ID and manufacturer ID	 */	
	ov9740_read(client, OV9740_MODEL_ID_HI, &modelhi);	
	ov9740_read(client, OV9740_MODEL_ID_LO, &modello);	
	model = (modelhi << 8) | modello;
	dev_err(&client->dev, "ov9740 read id = %x\n", model);
	if(model != 0x9740)
		{
			return ENXIO;
		}
	return 0;
}

static int
ov9740_s_config(struct v4l2_subdev *sd, int irq, void *platform_data)
{
	struct ov9740_device *dev = to_ov9740_sensor(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret;

	if (NULL == platform_data)
		return -ENODEV;
	v4l2_err(client, "enter  ov9740_s_config\n");
	dev->platform_data =
	    (struct camera_sensor_platform_data *)platform_data;

	if (dev->platform_data->platform_init) {
		ret = dev->platform_data->platform_init(client);
		if (ret) {
			v4l2_err(client, "ov9740 platform init err\n");
			return ret;
		}
	}
	ret = ov9740_s_power(sd, 1);
	if (ret) {
		v4l2_err(client, "ov9740 power-up err");
		//return ret;
		goto fail_detect;
	}

	/* config & detect sensor */
	ret = ov9740_detect(dev, client);
	if (ret) {
		v4l2_err(client, "ov9740_detect err s_config.\n");
		goto fail_detect;
	}

	ret = dev->platform_data->csi_cfg(sd, 1);
	if (ret)
		goto fail_csi_cfg;

	
	ret = ov9740_s_power(sd, 0);
	if (ret) {
		v4l2_err(client, "ov9740 power down err");
		return ret;
	}
	v4l2_err(client, "exit   ov9740_s_config\n");
	return 0;

fail_csi_cfg:
	dev->platform_data->csi_cfg(sd, 0);
fail_detect:
	ov9740_s_power(sd, 0);
	dev_err(&client->dev, "sensor power-gating failed\n");
	return ret;
}
#define N_CONTROLS (ARRAY_SIZE(ov9740_controls))

static struct mt9m114_control *ov9740_find_control(__u32 id)
{
	int i;

	for (i = 0; i < N_CONTROLS; i++) {
		if (ov9740_controls[i].qc.id == id)
			return &ov9740_controls[i];
	}
	return NULL;
}

static int ov9740_queryctrl(struct v4l2_subdev *sd, struct v4l2_queryctrl *qc)
{
	struct ov9740_control *ctrl = ov9740_find_control(qc->id);

	if (ctrl == NULL)
		return -EINVAL;
	*qc = ctrl->qc;
	return 0;
}

/* Horizontal flip the image. */
static int ov9740_t_hflip(struct v4l2_subdev *sd, int value)
{
 #if 0
	struct i2c_client *c = v4l2_get_subdevdata(sd);
	struct mt9m114_device *dev = to_mt9m114_sensor(sd);
	int err;
	/* set for direct mode */
	err = mt9m114_write_reg(c, MISENSOR_16BIT, 0x098E, 0xC850);
	if (value) {
		/* enable H flip ctx A */
		err += misensor_rmw_reg(c, MISENSOR_8BIT, 0xC850, 0x01, 0x01);
		err += misensor_rmw_reg(c, MISENSOR_8BIT, 0xC851, 0x01, 0x01);
		/* ctx B */
		err += misensor_rmw_reg(c, MISENSOR_8BIT, 0xC888, 0x01, 0x01);
		err += misensor_rmw_reg(c, MISENSOR_8BIT, 0xC889, 0x01, 0x01);

		err += misensor_rmw_reg(c, MISENSOR_16BIT, MISENSOR_READ_MODE,
					MISENSOR_HFLIP_MASK, MISENSOR_FLIP_EN);

		dev->bpat = MT9M114_BPAT_GRGRBGBG;
	} else {
		/* disable H flip ctx A */
		err += misensor_rmw_reg(c, MISENSOR_8BIT, 0xC850, 0x01, 0x00);
		err += misensor_rmw_reg(c, MISENSOR_8BIT, 0xC851, 0x01, 0x00);
		/* ctx B */
		err += misensor_rmw_reg(c, MISENSOR_8BIT, 0xC888, 0x01, 0x00);
		err += misensor_rmw_reg(c, MISENSOR_8BIT, 0xC889, 0x01, 0x00);

		err += misensor_rmw_reg(c, MISENSOR_16BIT, MISENSOR_READ_MODE,
					MISENSOR_HFLIP_MASK, MISENSOR_FLIP_DIS);

		dev->bpat = MT9M114_BPAT_BGBGGRGR;
	}

	err += mt9m114_write_reg(c, MISENSOR_8BIT, 0x8404, 0x06);
	udelay(10);

	return !!err;
	#endif
	return 0;
}

/* Vertically flip the image */
static int ov9740_t_vflip(struct v4l2_subdev *sd, int value)
{
     #if 0
	struct i2c_client *c = v4l2_get_subdevdata(sd);
	int err;
	/* set for direct mode */
	err = mt9m114_write_reg(c, MISENSOR_16BIT, 0x098E, 0xC850);
	if (value >= 1) {
		/* enable H flip - ctx A */
		err += misensor_rmw_reg(c, MISENSOR_8BIT, 0xC850, 0x02, 0x01);
		err += misensor_rmw_reg(c, MISENSOR_8BIT, 0xC851, 0x02, 0x01);
		/* ctx B */
		err += misensor_rmw_reg(c, MISENSOR_8BIT, 0xC888, 0x02, 0x01);
		err += misensor_rmw_reg(c, MISENSOR_8BIT, 0xC889, 0x02, 0x01);

		err += misensor_rmw_reg(c, MISENSOR_16BIT, MISENSOR_READ_MODE,
					MISENSOR_VFLIP_MASK, MISENSOR_FLIP_EN);
	} else {
		/* disable H flip - ctx A */
		err += misensor_rmw_reg(c, MISENSOR_8BIT, 0xC850, 0x02, 0x00);
		err += misensor_rmw_reg(c, MISENSOR_8BIT, 0xC851, 0x02, 0x00);
		/* ctx B */
		err += misensor_rmw_reg(c, MISENSOR_8BIT, 0xC888, 0x02, 0x00);
		err += misensor_rmw_reg(c, MISENSOR_8BIT, 0xC889, 0x02, 0x00);

		err += misensor_rmw_reg(c, MISENSOR_16BIT, MISENSOR_READ_MODE,
					MISENSOR_VFLIP_MASK, MISENSOR_FLIP_DIS);
	}

	err += mt9m114_write_reg(c, MISENSOR_8BIT, 0x8404, 0x06);
	udelay(10);

	return !!err;
	#endif
	return 0;
}


static int ov9740_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct ov9740_control *octrl = ov9740_find_control(ctrl->id);
	int ret;

	if (octrl == NULL)
		return -EINVAL;

	ret = octrl->query(sd, &ctrl->value);
	if (ret < 0)
		return ret;

	return 0;
}

static int ov9740_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct ov9740_control *octrl = ov9740_find_control(ctrl->id);
	int ret;

	if (!octrl || !octrl->tweak)
		return -EINVAL;

	ret = octrl->tweak(sd, ctrl->value);
	if (ret < 0)
		return ret;

	return 0;
}
#if 0
static int mt9m114_s_stream(struct v4l2_subdev *sd, int enable)
{
	int ret;
	struct i2c_client *c = v4l2_get_subdevdata(sd);

	if (enable) {
		ret = mt9m114_write_reg_array(c, mt9m114_chgstat_reg,
					POST_POLLING);
		if (ret < 0)
			return ret;

		ret = mt9m114_set_streaming(sd);
	} else {
		ret = mt9m114_set_suspend(sd);
	}

	return ret;
}
#endif
/* Start/Stop streaming from the device */
static int ov9740_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct i2c_client *c = v4l2_get_subdevdata(sd);
	int ret;

	/* Program orientation register. */
/*	if (priv->flag_vflip)
		ret = ov9740_reg_rmw(client, OV9740_IMAGE_ORT, 0x2, 0);
	else
		ret = ov9740_reg_rmw(client, OV9740_IMAGE_ORT, 0, 0x2);
	if (ret < 0)
		return ret;

	if (priv->flag_hflip)
		ret = ov9740_reg_rmw(client, OV9740_IMAGE_ORT, 0x1, 0);
	else
		ret = ov9740_reg_rmw(client, OV9740_IMAGE_ORT, 0, 0x1);
	if (ret < 0)
		return ret; */

	if (enable) {
		dev_dbg(&c->dev, "Enabling Streaming\n");
		/* Start Streaming */
		ret = ov9740_reg_write(c, OV9740_MODE_SELECT, 0x01);

	} else {
		dev_dbg(&c->dev, "Disabling Streaming\n");
		/* Software Reset */
		//ret = ov9740_reg_write(c, OV9740_SOFTWARE_RESET, 0x01);
		//if (!ret)
			/* Setting Streaming to Standby */
			ret = ov9740_reg_write(c, OV9740_MODE_SELECT,
					       0x00);
	}
       msleep(20);   // for cts of user version , add 20 ms delay by yaoling , it is suggested  from  intel.
	return 0 ;//ret;
}

static int
ov9740_enum_framesizes(struct v4l2_subdev *sd, struct v4l2_frmsizeenum *fsize)
{
	unsigned int index = fsize->index;

	if (index >= N_RES)
		return -EINVAL;

	fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
	fsize->discrete.width = ov9740_res[index].width;
	fsize->discrete.height = ov9740_res[index].height;

	/* FIXME: Wrong way to know used mode */
	fsize->reserved[0] = ov9740_res[index].used;

	return 0; //
}

static int ov9740_enum_frameintervals(struct v4l2_subdev *sd,
				       struct v4l2_frmivalenum *fival)
{
	unsigned int index = fival->index;
	int i;

	if (index >= N_RES)
		return -EINVAL;

	/* find out the first equal or bigger size */
	for (i = 0; i < N_RES; i++) {
		if ((ov9740_res[i].width >= fival->width) &&
		    (ov9740_res[i].height >= fival->height))
			break;
	}
	if (i == N_RES)
		i--;

	index = i;

	fival->type = V4L2_FRMIVAL_TYPE_DISCRETE;
	fival->discrete.numerator = 1;
	fival->discrete.denominator = ov9740_res[index].fps;

	return 0;
}

static int
ov9740_g_chip_ident(struct v4l2_subdev *sd, struct v4l2_dbg_chip_ident *chip)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	return v4l2_chip_ident_i2c_client(client, chip, V4L2_IDENT_OV9740, 0);
}

static int ov9740_enum_mbus_code(struct v4l2_subdev *sd,
				  struct v4l2_subdev_fh *fh,
				  struct v4l2_subdev_mbus_code_enum *code)
{
	if (code->index >= MAX_FMTS)
		return -EINVAL;
	code->code = V4L2_MBUS_FMT_SGRBG10_1X10;

	return 0;
}

static int ov9740_enum_frame_size(struct v4l2_subdev *sd,
	struct v4l2_subdev_fh *fh,
	struct v4l2_subdev_frame_size_enum *fse)
{

	unsigned int index = fse->index;


	if (index >= N_RES)
		return -EINVAL;

	fse->min_width = ov9740_res[index].width;
	fse->min_height = ov9740_res[index].height;
	fse->max_width = ov9740_res[index].width;
	fse->max_height = ov9740_res[index].height;

	return 0;
}

static struct v4l2_mbus_framefmt *
__ov9740_get_pad_format(struct ov9740_device *sensor,
			 struct v4l2_subdev_fh *fh, unsigned int pad,
			 enum v4l2_subdev_format_whence which)
{
	struct i2c_client *client = v4l2_get_subdevdata(&sensor->sd);

	if (pad != 0) {
		dev_err(&client->dev,  "%s err. pad %x\n", __func__, pad);
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
ov9740_get_pad_format(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh,
		       struct v4l2_subdev_format *fmt)
{
	struct ov9740_device *snr = to_ov9740_sensor(sd);
	struct v4l2_mbus_framefmt *format =
			__ov9740_get_pad_format(snr, fh, fmt->pad, fmt->which);

	if (format == NULL)
		return -EINVAL;
	fmt->format = *format;

	return 0;
}

static int
ov9740_set_pad_format(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh,
		       struct v4l2_subdev_format *fmt)
{
	struct ov9740_device *snr = to_ov9740_sensor(sd);
	struct v4l2_mbus_framefmt *format =
			__ov9740_get_pad_format(snr, fh, fmt->pad, fmt->which);

	if (format == NULL)
		return -EINVAL;

	if (fmt->which == V4L2_SUBDEV_FORMAT_ACTIVE)
		snr->format = fmt->format;

	return 0;
}

static int ov9740_g_skip_frames(struct v4l2_subdev *sd, u32 *frames)
{
	int index;
	struct ov9740_device *snr = to_ov9740_sensor(sd);

	if (frames == NULL)
		return -EINVAL;

	for (index = 0; index < N_RES; index++) {
		if (ov9740_res[index].res == snr->res)
			break;
	}

	if (index >= N_RES)
		return -EINVAL;

	*frames = ov9740_res[index].skip_frames;

	return 0;
}
static const struct v4l2_subdev_video_ops ov9740_video_ops = {
	.try_mbus_fmt = ov9740_try_mbus_fmt,
	.s_mbus_fmt = ov9740_set_mbus_fmt,
	.g_mbus_fmt = ov9740_get_mbus_fmt,
	.s_stream = ov9740_s_stream,
	.enum_framesizes = ov9740_enum_framesizes,
	.enum_frameintervals = ov9740_enum_frameintervals,
	
};

static struct v4l2_subdev_sensor_ops ov9740_sensor_ops = {
	.g_skip_frames	= ov9740_g_skip_frames,
};

static const struct v4l2_subdev_core_ops ov9740_core_ops = {
	.g_chip_ident = ov9740_g_chip_ident,
	.queryctrl = ov9740_queryctrl,
	.g_ctrl = ov9740_g_ctrl,
	.s_ctrl = ov9740_s_ctrl,
	.s_power = ov9740_s_power,
};

/* REVISIT: Do we need pad operations? */
static const struct v4l2_subdev_pad_ops ov9740_pad_ops = {
	.enum_mbus_code = ov9740_enum_mbus_code,
	.enum_frame_size = ov9740_enum_frame_size,
	.get_fmt = ov9740_get_pad_format,
	.set_fmt = ov9740_set_pad_format,
};

static const struct v4l2_subdev_ops ov9740_ops = {
	.core = &ov9740_core_ops,
	.video = &ov9740_video_ops,
	.pad = &ov9740_pad_ops,
	.sensor = &ov9740_sensor_ops,
};

static const struct media_entity_operations ov9740_entity_ops = {
	.link_setup = NULL,
};


static int ov9740_remove(struct i2c_client *client)
{
	struct ov9740_device *dev;
	struct v4l2_subdev *sd = i2c_get_clientdata(client);

	dev = container_of(sd, struct ov9740_device, sd);
	dev->platform_data->csi_cfg(sd, 0);
	if (dev->platform_data->platform_deinit)
		dev->platform_data->platform_deinit();
	v4l2_device_unregister_subdev(sd);
	media_entity_cleanup(&dev->sd.entity);
	kfree(dev);
	return 0;
}

static int ov9740_probe(struct i2c_client *client,
		       const struct i2c_device_id *id)
{
	struct ov9740_device *dev;
	int ret;

	/* Setup sensor configuration structure */
	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev) {
		dev_err(&client->dev, "out of memory\n");
		return -ENOMEM;
	}

	v4l2_i2c_subdev_init(&dev->sd, client, &ov9740_ops);
	if (client->dev.platform_data) {
		ret = ov9740_s_config(&dev->sd, client->irq,
				       client->dev.platform_data);
		if (ret) {
			v4l2_device_unregister_subdev(&dev->sd);
			kfree(dev);
			return ret;
		}
	}

	/*TODO add format code here*/
	dev->sd.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	dev->pad.flags = MEDIA_PAD_FL_SOURCE;

	/* REVISIT: Do we need media controller? */
	ret = media_entity_init(&dev->sd.entity, 1, &dev->pad, 0);
	if (ret) {
		ov9740_remove(client);
		return ret;
	}

	/* set res index to be invalid */
	dev->res = -1;

	return 0;
}
static const struct i2c_device_id ov9740_id[] = {
	{OV9740_NAME, 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, ov9740_id);

static struct i2c_driver ov9740_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "ov9740"
	},
	.probe = ov9740_probe,
	.remove = ov9740_remove,
	.id_table = ov9740_id,
};

static __init int init_ov9740(void)
{
	return i2c_add_driver(&ov9740_driver);
}

static __exit void exit_ov9740(void)
{
	i2c_del_driver(&ov9740_driver);
}

module_init(init_ov9740);
module_exit(exit_ov9740);

MODULE_AUTHOR("Shuguang Gong <Shuguang.gong@intel.com>");
MODULE_LICENSE("GPL");


