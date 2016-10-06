/*
 * LED flash driver for ADP1650
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
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/slab.h>

#include <media/adp1650.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-device.h>

#include <linux/atomisp.h>

struct adp1650_ctrl_id {
	struct v4l2_queryctrl qc;
	int (*s_ctrl) (struct v4l2_subdev *sd, __u32 val);
	int (*g_ctrl) (struct v4l2_subdev *sd, __s32 *val);
};
#define ADP1650_PRINT  1
/* Registers */
#define	ADP1650_MODE_SET	0x04


#define	ADP1650_CURRENT_SET	0x03
#define	ADP1650_TORCH_CURRENT_SHIFT 0
#define	ADP1650_FLASH_CURRENT_SHIFT 3

#define	ADP1650_TIMING_SET	0x02
#define	ADP1650_FLASH_TIMING_SETTING 0

struct adp1650 {
	struct v4l2_subdev sd;

	struct mutex power_lock;
	int power_count;

	unsigned int mode;
	int timeout;
	u8 torch_current;
	u8 indicator_current;
	u8 flash_current;
	u8 flash_timing_set;  // add by yaoling

	struct timer_list flash_off_delay;
	struct adp1650_platform_data *pdata;
};

#define to_adp1650(p_sd)	container_of(p_sd, struct adp1650, sd)

/* Return negative errno else zero on success */
static int adp1650_write(struct adp1650 *flash, u8 reg, u8 value)
{
	struct i2c_client *client =v4l2_get_subdevdata(&flash->sd);
	struct i2c_msg msg;
	unsigned char data[2] = { reg, value };
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

/* Return negative errno else a data byte received from the device. */
static int adp1650_read(struct adp1650 *flash, u8 addr,u8 * value)
{

	struct i2c_client *client =  v4l2_get_subdevdata(&flash->sd);
	u8 data [2];
	struct i2c_msg msg[2];
	int ret;

	msg[0].addr = client->addr;
	msg[0].flags = 0;
	msg[0].len = 1;
	msg[0].buf = data;

	data[0] = (u8) (addr);

	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 1;
	msg[1].buf = data + 1;

	*value = 0;

	
	ret = i2c_transfer(client->adapter, msg, 2);
	if (ret >= 0) {
		*value = data[1];
		return 0;
	}
	else
	{
		return -1;
	}
	
	
}

/* -----------------------------------------------------------------------------
 * Hardware configuration
 */

static int adp1650_set_mode(struct adp1650 *flash, unsigned int mode)
{
	struct i2c_client *client = v4l2_get_subdevdata(&flash->sd);
	struct adp1650_platform_data  *pdata = flash->pdata;
	u8 reg ;
	#if ADP1650_PRINT
	dev_err(&client->dev, "adp1650_set_mode mode=%d\n", mode);
	#endif
	flash->mode = mode;
	if(ADP1650_MODE_FLASH==flash->mode)  // flash 
	{
			#if ADP1650_PRINT
			dev_err(&client->dev, " adp1650_set_mode 	flash on \n" );
			#endif
			adp1650_read(flash,ADP1650_MODE_SET,&reg); 
			reg = reg |0x0f;
			adp1650_write(flash,ADP1650_MODE_SET,reg); 
			gpio_set_value(pdata->gpio_strobe, 1); 
		}
	else if (ADP1650_MODE_OFF ==flash->mode) // torch off and flash off
	{
			#if ADP1650_PRINT
			dev_err(&client->dev, "adp1650_set_mode flash and torch off \n" );
			#endif
			//gpio_set_value(pdata->gpio_torch, 0);  // hardware tor to low
			adp1650_write(flash,ADP1650_MODE_SET, 0x00);  
			adp1650_read(flash,ADP1650_MODE_SET,&reg);
			reg  = reg |0x00;
			adp1650_write(flash,ADP1650_TIMING_SET,reg);     
			gpio_set_value(pdata->gpio_torch, 0);  // hardware tor to low
	
			
			adp1650_write(flash,ADP1650_MODE_SET,0x00);
			gpio_set_value(pdata->gpio_strobe, 0); 
		}
	else  if(ADP1650_MODE_TORCH ==flash->mode)  // torch mode
	{
			#if ADP1650_PRINT
			dev_err(&client->dev, "adp1650_set_mode torch mode  \n" );
			#endif
		adp1650_write(flash, ADP1650_MODE_SET, 0x08);
			adp1650_read(flash,ADP1650_MODE_SET,&reg);
			reg = reg | 0x10;
			//timing_set = timing_set |0x10;
			adp1650_write(flash,ADP1650_TIMING_SET,reg);  // 300ms
			//adp1650_write(flash,ADP1650_TIMING_SET,timing_set); 
		gpio_set_value(pdata->gpio_torch, 1);  // hardware tor to high
	}
	 /* torch off */
	 else
 	{
			#if ADP1650_PRINT
			dev_err(&client->dev, " adp1650_set_mode torch off  or others\n" );
			#endif
		//gpio_set_value(pdata->gpio_torch, 0);  // hardware tor to low
		adp1650_write(flash,ADP1650_MODE_SET, 0x00);  
			adp1650_read(flash,ADP1650_MODE_SET,&reg);
			reg  = reg |0x00;
			adp1650_write(flash,ADP1650_TIMING_SET,reg);     
			//adp1650_write(flash,ADP1650_TIMING_SET,timing_set); 
		gpio_set_value(pdata->gpio_torch, 0);  // hardware tor to low
 		
 	}
	return 0;
	
}

static int adp1650_set_torch(struct adp1650 *flash)
{
	u8  current_set;
	struct i2c_client *client = v4l2_get_subdevdata(&flash->sd);
	#if ADP1650_PRINT
	dev_err(&client->dev, "enter adp1650_set_torch \n");
	#endif
	current_set =  (flash->torch_current << ADP1650_TORCH_CURRENT_SHIFT) |
	      (flash->flash_current << ADP1650_FLASH_CURRENT_SHIFT);
	adp1650_write( flash, ADP1650_CURRENT_SET, current_set);
	return 0;//ADP1650_write(flash, ADP1650_TORCH_BRIGHTNESS_REG, val);
}
	
static int adp1650_set_flash(struct adp1650 *flash)
	{
	u8 val;
	struct i2c_client *client = v4l2_get_subdevdata(&flash->sd);
	#if ADP1650_PRINT
	dev_err(&client->dev, "enter adp1650_set_flash \n");
	 #endif
	val  = (flash->flash_current << ADP1650_FLASH_CURRENT_SHIFT)|(flash->torch_current) ;
	#if ADP1650_PRINT
	dev_err(&client->dev, " adp1650_set_flash val =%x\n",val);
	#endif
	adp1650_write(flash,0x03,val); 
	return 0; //adp1650_write(flash, ADP1650_FLASH_BRIGHTNESS_REG, val);
}

static int adp1650_set_duration(struct adp1650 *flash)
{
	u8 val,reg;
	struct i2c_client *client = v4l2_get_subdevdata(&flash->sd);
	#if ADP1650_PRINT
	dev_err(&client->dev, " enter adp1650_set_duration\n" );
	#endif
	val = (flash->timeout << ADP1650_FLASH_TIMING_SETTING) ;
	adp1650_read( flash, ADP1650_TIMING_SET,&reg);
	val = reg |val ;
	adp1650_write(flash, ADP1650_TIMING_SET, val);
	return 0;//adp1650_write(flash, ADP1650_FLASH_DURATION_REG, val);
}

static int adp1650_set_config1(struct adp1650 *flash)
{
	
	struct i2c_client *client = v4l2_get_subdevdata(&flash->sd);
	#if ADP1650_PRINT
	dev_err(&client->dev, " enter adp1650_set_config1\n" );
	#endif
	//val = (flash->pdata->envm_tx2 << ADP1650_ENVM_TX2_SHIFT) |
	 //     (flash->pdata->tx2_polarity << ADP1650_TX2_POLARITY_SHIFT);
	 
	return 0; //adp1650_write(flash, ADP1650_CONFIG_REG_1, val);
}

/* -----------------------------------------------------------------------------
 * Hardware reset and trigger
 */

static int adp1650_hw_reset(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct adp1650 *flash = to_adp1650(sd);
	struct adp1650_platform_data *pdata = flash->pdata;
	int ret;
	#if ADP1650_PRINT
	dev_err(&client->dev, " enter adp1650_hw_reset\n" );
	#endif
	ret = gpio_request(pdata->gpio_enable, "flash reset");
	if (ret < 0)
		return ret;

	gpio_set_value(pdata->gpio_enable, 0);
	msleep(50);

	gpio_set_value(pdata->gpio_enable, 1);
	msleep(50);

	//gpio_free(pdata->gpio_enable);
	#if ADP1650_PRINT
	dev_err(&client->dev, " exit  adp1650_hw_reset\n" );
	#endif
	return ret;
}

static void adp1650_flash_off_delay(long unsigned int arg)
{
	struct v4l2_subdev *sd = i2c_get_clientdata((struct i2c_client *)arg);
	struct adp1650 *flash = to_adp1650(sd);
	struct adp1650_platform_data *pdata = flash->pdata;
	struct i2c_client *client = v4l2_get_subdevdata(&flash->sd);
	#if ADP1650_PRINT
      	dev_err(&client->dev, " enter adp1650_flash_off_delay\n" );
	#endif
	gpio_set_value(pdata->gpio_strobe, 0);
}

static int adp1650_hw_strobe(struct i2c_client *client, bool strobe)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct adp1650 *flash = to_adp1650(sd);
	struct adp1650_platform_data *pdata = flash->pdata;
	#if ADP1650_PRINT
	dev_err(&client->dev, " enter adp1650_hw_strobe strobe =%d\n" ,strobe);
	#endif
	if (!strobe) {
		
		gpio_set_value(pdata->gpio_strobe, 0); 
		
	}

	/* Flash on */
	else{
		
		gpio_set_value(pdata->gpio_strobe, 1); 
	}
	return 0;
}

/* -----------------------------------------------------------------------------
 * V4L2 controls
 */

static int adp1650_read_status(struct adp1650 *flash)
{
	int ret = 0;
	struct i2c_client *client = v4l2_get_subdevdata(&flash->sd);
	dev_err(&client->dev, " enter adp1650_read_status \n");

	return ret;
}
/* flash timing setting ? */
static int adp1650_s_flash_timeout(struct v4l2_subdev *sd, u32 val)
{
	struct adp1650 *flash = to_adp1650(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	#if ADP1650_PRINT
 	dev_err(&client->dev, " enter adp1650_s_flash_timeout \n");
	#endif
	//val = clamp(val, ADP1650_MIN_TIMEOUT, ADP1650_MAX_TIMEOUT);
	//val = val / ADP1650_TIMEOUT_STEPSIZE - 1;
     // if(ADP1650_MODE_FLASH==flash->mode)
      	{
		val = clamp(val, ADP1650_MIN_TIMEOUT, ADP1650_MAX_TIMEOUT);
		val = val / ADP1650_TIMEOUT_STEPSIZE - 1;
      	}
	 
	flash->timeout = val;
	return adp1650_set_duration(flash);
	
}

static int adp1650_g_flash_timeout(struct v4l2_subdev *sd, s32 *val)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	#if ADP1650_PRINT
 	dev_err(&client->dev, " enter adp1650_g_flash_timeout \n");
	#endif
	/*struct ADP1650 *flash = to_ADP1650(sd);

	*val = (u32)(flash->timeout + 1) * ADP1650_TIMEOUT_STEPSIZE;

	return 0;*/
	return 0;
}

static int adp1650_s_flash_intensity(struct v4l2_subdev *sd, u32 intensity)
{
	struct adp1650 *flash = to_adp1650(sd);
       struct i2c_client *client = v4l2_get_subdevdata(sd);
	#if ADP1650_PRINT
 	dev_err(&client->dev, " adp1650_s_flash_intensity \n");
	#endif
	intensity = ADP1650_CLAMP_PERCENTAGE(intensity);
	intensity = ADP1650_PERCENT_TO_VALUE(intensity, ADP1650_FLASH_STEP);

	flash->flash_current = intensity;
	#if ADP1650_PRINT
       dev_err(&client->dev, " adp1650_s_flash_intensity flash->flash_current  =%d\n",flash->flash_current );
	#endif
	return adp1650_set_flash(flash);
}

static int adp1650_g_flash_intensity(struct v4l2_subdev *sd, s32 *val)
{
	struct adp1650 *flash = to_adp1650(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	#if ADP1650_PRINT
 	dev_err(&client->dev, " enter adp1650_g_flash_intensity \n");
	#endif
	*val = ADP1650_VALUE_TO_PERCENT((u32)flash->flash_current,
			ADP1650_FLASH_STEP);

	return 0;
}

static int adp1650_s_torch_intensity(struct v4l2_subdev *sd, u32 intensity)
{
	struct adp1650 *flash = to_adp1650(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	#if ADP1650_PRINT
 	dev_err(&client->dev, " enter adp1650_s_torch_intensity \n");
	#endif
	intensity = ADP1650_CLAMP_PERCENTAGE(intensity);
	intensity = ADP1650_PERCENT_TO_VALUE(intensity, ADP1650_TORCH_STEP);

	flash->torch_current = intensity;

	return adp1650_set_torch(flash);
}

static int adp1650_g_torch_intensity(struct v4l2_subdev *sd, s32 *val)
{
	struct adp1650 *flash = to_adp1650(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	#if ADP1650_PRINT
 	dev_err(&client->dev, " enter adp1650_g_torch_intensity \n");
	#endif
	*val = ADP1650_VALUE_TO_PERCENT((u32)flash->torch_current,
			ADP1650_TORCH_STEP);

	return 0;
}

static int adp1650_s_indicator_intensity(struct v4l2_subdev *sd, u32 intensity)
{
	struct adp1650 *flash = to_adp1650(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	#if ADP1650_PRINT
 	dev_err(&client->dev, " adp1650_s_indicator_intensity \n");
	#endif
	intensity = ADP1650_CLAMP_PERCENTAGE(intensity);
	intensity = ADP1650_PERCENT_TO_VALUE(intensity, ADP1650_INDICATOR_STEP);

	flash->indicator_current = intensity;
	#if ADP1650_PRINT
       dev_err(&client->dev, " adp1650_s_indicator_intensity flash->indicator_current=%d\n",flash->indicator_current);
	#endif
	return adp1650_set_torch(flash);
}

static int adp1650_g_indicator_intensity(struct v4l2_subdev *sd, s32 *val)
{
	struct adp1650 *flash = to_adp1650(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	#if ADP1650_PRINT
 	dev_err(&client->dev, " enter adp1650_g_indicator_intensity \n");
	#endif
	*val = ADP1650_VALUE_TO_PERCENT((u32)flash->indicator_current,
			ADP1650_INDICATOR_STEP);

	return 0;
}

static int adp1650_s_flash_strobe(struct v4l2_subdev *sd, u32 val)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	#if ADP1650_PRINT
 	dev_err(&client->dev, " enter adp1650_s_flash_strobe \n");
	#endif
	return adp1650_hw_strobe(client, val);
}

static int adp1650_s_flash_mode(struct v4l2_subdev *sd, u32 new_mode)
{
	struct adp1650 *flash = to_adp1650(sd);
	unsigned int mode;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	#if ADP1650_PRINT
 	dev_err(&client->dev, " enter adp1650_s_flash_mode new_mode=%d \n",new_mode);
	#endif

	switch (new_mode) {
	case ATOMISP_FLASH_MODE_OFF:
		mode = ADP1650_MODE_OFF;
		break;
	case ATOMISP_FLASH_MODE_FLASH:
		mode = ADP1650_MODE_FLASH;
		break;
	case ATOMISP_FLASH_MODE_INDICATOR:
		mode = ADP1650_MODE_INDICATOR;
		break;
	case ATOMISP_FLASH_MODE_TORCH:
		mode = ADP1650_MODE_TORCH;
		break;
	default:
		return -EINVAL;
	}

	return adp1650_set_mode(flash, mode);
}

static int adp1650_g_flash_mode(struct v4l2_subdev *sd, s32 * val)
{
	struct adp1650 *flash = to_adp1650(sd);
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	#if ADP1650_PRINT
 	dev_err(&client->dev, " enter adp1650_g_flash_mode \n");
	#endif
	*val = flash->mode;
	return 0;
}

static int adp1650_g_flash_status(struct v4l2_subdev *sd, s32 *val)
{

	struct i2c_client *client = v4l2_get_subdevdata(sd);
 	dev_err(&client->dev, " enter adp1650_g_flash_status \n");
	*val = ATOMISP_FLASH_STATUS_OK;
	return 0;
}

static const struct adp1650_ctrl_id adp1650_ctrls[] = {
	s_ctrl_id_entry_integer(V4L2_CID_FLASH_TIMEOUT,
				"Flash Timeout",
				0,
				ADP1650_MAX_TIMEOUT,
				1,
				ADP1650_DEFAULT_TIMEOUT,
				0,
				adp1650_s_flash_timeout,
				adp1650_g_flash_timeout),
	s_ctrl_id_entry_integer(V4L2_CID_FLASH_INTENSITY,
				"Flash Intensity",
				ADP1650_MIN_PERCENT,
				ADP1650_MAX_PERCENT,
				1,
				ADP1650_FLASH_DEFAULT_BRIGHTNESS,
				0,
				adp1650_s_flash_intensity,
				adp1650_g_flash_intensity),
	s_ctrl_id_entry_integer(V4L2_CID_FLASH_TORCH_INTENSITY,
				"Torch Intensity",
				ADP1650_MIN_PERCENT,
				ADP1650_MAX_PERCENT,
				1,
				ADP1650_TORCH_DEFAULT_BRIGHTNESS,
				0,
				adp1650_s_torch_intensity,
				adp1650_g_torch_intensity),
	s_ctrl_id_entry_integer(V4L2_CID_FLASH_INDICATOR_INTENSITY,
				"Indicator Intensity",
				ADP1650_MIN_PERCENT,
				ADP1650_MAX_PERCENT,
				1,
				ADP1650_INDICATOR_DEFAULT_BRIGHTNESS,
				0,
				adp1650_s_indicator_intensity,
				adp1650_g_indicator_intensity),
	s_ctrl_id_entry_boolean(V4L2_CID_FLASH_STROBE,
				"Flash Strobe",
				0,
				0,
				adp1650_s_flash_strobe,
				NULL),
	s_ctrl_id_entry_integer(V4L2_CID_FLASH_MODE,
				"Flash Mode",
				0,   /* don't assume any enum ID is first */
				100, /* enum value, may get extended */
				1,
				ATOMISP_FLASH_MODE_OFF,
				0,
				adp1650_s_flash_mode,
				adp1650_g_flash_mode),
	s_ctrl_id_entry_integer(V4L2_CID_FLASH_STATUS,
				"Flash Status",
				0,   /* don't assume any enum ID is first */
				100, /* enum value, may get extended */
				1,
				ATOMISP_FLASH_STATUS_OK,
				0,
				NULL,
				adp1650_g_flash_status),
};

static const struct adp1650_ctrl_id *find_ctrl_id(unsigned int id)
{
	int i;
	int num;
	num = ARRAY_SIZE(adp1650_ctrls);
	for (i = 0; i < num; i++) {
		if (adp1650_ctrls[i].qc.id == id)
			return &adp1650_ctrls[i];
	}

	return NULL;
}

static int adp1650_queryctrl(struct v4l2_subdev *sd, struct v4l2_queryctrl *qc)
{
	int num;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	#if ADP1650_PRINT
       dev_err(&client->dev, "enter adp1650_queryctrl\n");
	#endif
	if (!qc)
		return -EINVAL;

	num = ARRAY_SIZE(adp1650_ctrls);
	if (qc->id >= num)
		return -EINVAL;

	*qc = adp1650_ctrls[qc->id].qc;

	return 0;
}

static int adp1650_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	const struct adp1650_ctrl_id *s_ctrl;
	struct adp1650 *flash = to_adp1650(sd);
	struct adp1650_platform_data  *pdata = flash->pdata;
	int ret ;
	
	//struct i2c_client *client = v4l2_get_subdevdata(sd);
       //dev_err(&client->dev, "enter adp1650_s_ctrl\n");
	if (!ctrl)
		return -EINVAL;
	ret = gpio_get_value(pdata->gpio_enable);
	/* add by yaoling for google torch apk 20131011 start */
	if(0 == ret)
	{
		gpio_set_value(pdata->gpio_enable,1);
	}
	/* add by yaoling for google torch apk 20131011 end */		
       // dev_err(&client->dev, "before  adp1650_s_ctrl -find_ctrl_id \n");
	s_ctrl = find_ctrl_id(ctrl->id);
	if (!s_ctrl)
		return -EINVAL;
       //dev_err(&client->dev, "after  adp1650_s_ctrl -find_ctrl_id ctrl->value =%d\n",ctrl->value);
	return s_ctrl->s_ctrl(sd, ctrl->value);
}

static int adp1650_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	const struct adp1650_ctrl_id *s_ctrl;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	#if ADP1650_PRINT
       dev_err(&client->dev, "enter adp1650_g_ctrl\n");
	#endif
	if (!ctrl)
		return -EINVAL;

	s_ctrl = find_ctrl_id(ctrl->id);
	if (s_ctrl == NULL)
		return -EINVAL;

	return s_ctrl->g_ctrl(sd, &ctrl->value);
}

/* -----------------------------------------------------------------------------
 * V4L2 subdev core operations
 */

/* Put device into known state. */
static int adp1650_setup(struct adp1650 *flash)
{
	struct i2c_client *client = v4l2_get_subdevdata(&flash->sd);
	u8 ret;
	//dev_err(&client->dev, "Fault info: %02x\n", ret);
	#if ADP1650_PRINT
	dev_err(&client->dev, "enter adp1650_setup\n");
	#endif

	ret = adp1650_set_config1(flash);
	if (ret < 0)
		return ret;

	ret = adp1650_set_duration(flash);
	if (ret < 0)
		return ret;

	ret = adp1650_set_torch(flash);
	if (ret < 0)
		return ret;

	ret = adp1650_set_flash(flash);
	if (ret < 0)
		return ret;

	/* read status */
	ret = adp1650_read_status(flash);
	if (ret < 0)
		return ret;

	return 0;
}
#if 0
static int __adp1650_s_power(struct v4l2_subdev *sd, int power)
{
       struct ADP1650 *flash = to_ADP1650(sd);
	struct adp1650_platform_data *pdata = flash->pdata;
	gpio_set_value(pdata->gpio_enable,power);
	return 0;
}
#endif
static int __adp1650_s_power(struct adp1650 *flash, int power)
{
        struct adp1650_platform_data *pdata = flash->pdata;
	gpio_set_value(pdata->gpio_enable,power);
	return 0;
}

static int adp1650_s_power(struct v4l2_subdev *sd, int power)
{
	struct adp1650 *flash = to_adp1650(sd);
	struct adp1650_platform_data *pdata = flash->pdata;
	int ret = 0;

	mutex_lock(&flash->power_lock);

/*	if (flash->power_count == !power) {
		ret = _adp1650_s_power(flash, !!power);
		if (ret < 0)
			goto done;
	}

	flash->power_count += power ? 1 : -1;
	WARN_ON(flash->power_count < 0); */
	ret = gpio_get_value(pdata->gpio_enable);
       if (power!= ret)
	{
		ret = __adp1650_s_power(flash, power);
		if (ret < 0)
		goto done;
	}

done:
	mutex_unlock(&flash->power_lock);
	return ret;
}

static const struct v4l2_subdev_core_ops adp1650_core_ops = {
	.queryctrl = adp1650_queryctrl,
	.g_ctrl = adp1650_g_ctrl,
	.s_ctrl = adp1650_s_ctrl,
	.s_power = adp1650_s_power,
};

static const struct v4l2_subdev_ops adp1650_ops = {
	.core = &adp1650_core_ops,
};

static int adp1650_detect(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct i2c_adapter *adapter = client->adapter;
	struct adp1650 *flash = to_adp1650(sd);
	int ret;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
		dev_err(&client->dev, "adp1650_detect i2c error\n");
		return -ENODEV;
	}
	#if ADP1650_PRINT
       dev_err(&client->dev, "enter adp1650_detect  \n");
	#endif
	/* Power up the flash driver and reset it */
	ret = adp1650_s_power(&flash->sd, 1);
	if (ret < 0)
		{
		dev_err(&client->dev, "err  adp1650_detect i2c \n");
		return ret;
		}

	adp1650_hw_reset(client);
	#if ADP1650_PRINT
	dev_err(&client->dev, "after  adp1650_hw_reset \n");
	#endif

	/* Setup default values. This makes sure that the chip is in a known
	 * state.
	 */
	ret = adp1650_setup(flash);
	if (ret < 0)
		goto fail;
	#if ADP1650_PRINT
	dev_err(&client->dev, "Successfully detected adp1650 LED flash\n");
	#endif
	adp1650_s_power(&flash->sd, 0);
	return 0;

fail:
	adp1650_s_power(&flash->sd, 0);
	return ret;
}

static int adp1650_open(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	#if ADP1650_PRINT
	 dev_err(&client->dev, "enter adp1650_open\n");
	#endif
	return adp1650_s_power(sd, 1);
}

static int adp1650_close(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	#if ADP1650_PRINT
	 dev_err(&client->dev, "enter adp1650_close\n");
	#endif
	return adp1650_s_power(sd, 0);
}

static const struct v4l2_subdev_internal_ops adp1650_internal_ops = {
	.registered = adp1650_detect,
	.open = adp1650_open,
	.close = adp1650_close,
};

/* -----------------------------------------------------------------------------
 *  I2C driver
 */
#ifdef CONFIG_PM

static int adp1650_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct v4l2_subdev *subdev = i2c_get_clientdata(client);
	struct adp1650 *flash = to_adp1650(subdev);
	int rval;
	#if ADP1650_PRINT
       dev_err(&client->dev, "enter adp1650_suspend\n");
	#endif
	if (flash->power_count == 0)
		return 0;

	rval = __adp1650_s_power(flash, 0);

	dev_err(&client->dev, "Suspend %s\n", rval < 0 ? "failed" : "ok");

	return rval;
}

static int adp1650_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct v4l2_subdev *subdev = i2c_get_clientdata(client);
	struct adp1650 *flash = to_adp1650(subdev);
	int rval;
	#if ADP1650_PRINT
       dev_err(&client->dev, "enter adp1650_resume\n");
	#endif
	if (flash->power_count == 0)
		return 0;

	rval = __adp1650_s_power(flash, 1);

	dev_err(&client->dev, "Resume %s\n", rval < 0 ? "fail" : "ok");

	return rval;
}

#else

#define adp1650_suspend NULL
#define adp1650_resume  NULL

#endif /* CONFIG_PM */

static int __devinit adp1650_gpio_init(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct adp1650 *flash = to_adp1650(sd);
	struct adp1650_platform_data *pdata = flash->pdata;
	int ret;
	#if ADP1650_PRINT
       dev_err(&client->dev, "enter adp1650_gpio_init\n");
	#endif
	ret = gpio_request(pdata->gpio_strobe, "flash");
	if (ret < 0)
		return ret;

	ret = gpio_direction_output(pdata->gpio_strobe, 0);
	if (ret < 0)
		goto err_gpio_flash;

	ret = gpio_request(pdata->gpio_torch, "torch");
	if (ret < 0)
		goto err_gpio_flash;

	ret = gpio_direction_output(pdata->gpio_torch, 0);
	if (ret < 0)
		goto err_gpio_torch;

	ret = gpio_request(pdata->gpio_enable, "adp1650_en");
	if (ret < 0)
		goto err_gpio_torch;

	ret = gpio_direction_output(pdata->gpio_enable, 1);
	if (ret < 0)
		goto err_gpio_enable;

	return 0;

err_gpio_torch:
	gpio_free(pdata->gpio_torch);
	gpio_free(pdata->gpio_strobe);
err_gpio_flash:
	gpio_free(pdata->gpio_strobe);
err_gpio_enable:
	gpio_free(pdata->gpio_enable);
	gpio_free(pdata->gpio_torch);
	gpio_free(pdata->gpio_strobe);
	return ret;
}

static int __devexit adp1650_gpio_uninit(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct adp1650 *flash = to_adp1650(sd);
	struct adp1650_platform_data *pdata = flash->pdata;
	int ret;
	#if ADP1650_PRINT
       dev_err(&client->dev, "enter adp1650_gpio_uninit\n");
	#endif
	ret = gpio_direction_output(pdata->gpio_torch, 0);
	if (ret < 0)
		return ret;

	ret = gpio_direction_output(pdata->gpio_strobe, 0);
	if (ret < 0)
		return ret;
	ret = gpio_direction_output(pdata->gpio_enable, 0);
	if (ret < 0)
		return ret;

	gpio_free(pdata->gpio_torch);

	gpio_free(pdata->gpio_strobe);

	gpio_free(pdata->gpio_enable);

	return 0;
}
#ifdef CONFIG_PROJECT_V975
/* ZTE: add by yaoling for flash prj cmd test start */
static int  adp1650_gpio_init_prj_cmd(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct adp1650 *flash = to_adp1650(sd);
	struct adp1650_platform_data *pdata = flash->pdata;
	int ret;
	#if ADP1650_PRINT
       dev_err(&client->dev, "enter adp1650_gpio_init\n");
	#endif
	ret = gpio_request(pdata->gpio_strobe, "flash");
	if (ret < 0)
		return ret;

	ret = gpio_direction_output(pdata->gpio_strobe, 0);
	if (ret < 0)
		goto err_gpio_flash;

	ret = gpio_request(pdata->gpio_torch, "torch");
	if (ret < 0)
		goto err_gpio_flash;

	ret = gpio_direction_output(pdata->gpio_torch, 0);
	if (ret < 0)
		goto err_gpio_torch;

	ret = gpio_request(pdata->gpio_enable, "adp1650_en");
	if (ret < 0)
		goto err_gpio_torch;

	ret = gpio_direction_output(pdata->gpio_enable, 1);
	if (ret < 0)
		goto err_gpio_enable;

	return 0;

err_gpio_torch:
	gpio_free(pdata->gpio_torch);
	gpio_free(pdata->gpio_strobe);
err_gpio_flash:
	gpio_free(pdata->gpio_strobe);
err_gpio_enable:
	gpio_free(pdata->gpio_enable);
	gpio_free(pdata->gpio_torch);
	gpio_free(pdata->gpio_strobe);
	return ret;
}

static int  adp1650_gpio_uninit_prj_cmd(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct adp1650 *flash = to_adp1650(sd);
	struct adp1650_platform_data *pdata = flash->pdata;
	int ret;
	#if ADP1650_PRINT
       dev_err(&client->dev, "enter adp1650_gpio_uninit\n");
	#endif
	ret = gpio_direction_output(pdata->gpio_torch, 0);
	if (ret < 0)
		return ret;

	ret = gpio_direction_output(pdata->gpio_strobe, 0);
	if (ret < 0)
		return ret;
	ret = gpio_direction_output(pdata->gpio_enable, 0);
	if (ret < 0)
		return ret;

	gpio_free(pdata->gpio_torch);

	gpio_free(pdata->gpio_strobe);

	gpio_free(pdata->gpio_enable);

	return 0;
}

struct i2c_client *adp1650_client=NULL;
#define ADP_PROC_FILE "driver/adp1650"
static struct proc_dir_entry *adp_proc_file;
static ssize_t adp1650_proc_write(struct file *filp,
				    const char *buff, size_t len,
				    loff_t * off)
{
	int err;
	struct v4l2_subdev *sd = i2c_get_clientdata(adp1650_client);
	struct adp1650 *flash = to_adp1650(sd);
	struct adp1650_platform_data *pdata = flash->pdata;
	if(strncmp(buff, "FlashOpen", 9) == 0)
	{
		printk("the order is %s...\n", buff);
		adp1650_gpio_init_prj_cmd(adp1650_client);
		gpio_set_value(pdata->gpio_enable,1);
       	err = adp1650_write(flash,ADP1650_MODE_SET,0xbb); 
		if (-1 == err)
			printk("adp1650 set reg failed!");
	}
	else if(strncmp(buff, "FlashOff", 8) == 0)
	{
		printk("the order is %s...\n", buff);
       	adp1650_write(flash,ADP1650_MODE_SET,0x00); 
		gpio_set_value(pdata->gpio_enable,0);
		adp1650_gpio_uninit_prj_cmd(adp1650_client);
	}
	else
	{
		printk("wrong order...\n");
	}
	return len;
}

static struct file_operations adp1650_proc_ops = 
{
	.write = adp1650_proc_write,
	.read = NULL,
};

static int  create_adp1650_proc_file(struct i2c_client *client)
{
       printk("create_adp1650_proc_file\n");
	adp_proc_file =
	    create_proc_entry(ADP_PROC_FILE, 0666, NULL); 
	if (adp_proc_file) {
		adp_proc_file->proc_fops = &adp1650_proc_ops;
	} 
	else	{
		printk("proc file create failed!");
	} 
	return 0;
}
/* ZTE: add by yaoling for flash prj cmd test end  */
#endif
static int __devinit adp1650_probe(struct i2c_client *client,
				  const struct i2c_device_id *id)
{
	int err;
	//struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct adp1650 *flash; //= to_ADP1650(sd);;
	#if ADP1650_PRINT
       dev_err(&client->dev, "enter ADP1650_probe\n");
	#endif
	if (client->dev.platform_data == NULL) {
		dev_err(&client->dev, "no platform data\n");
		return -ENODEV;
	}

	flash = kzalloc(sizeof(*flash), GFP_KERNEL);
	if (!flash) {
		dev_err(&client->dev, "out of memory\n");
		return -ENOMEM;
	}

	flash->pdata = client->dev.platform_data;

	v4l2_i2c_subdev_init(&flash->sd, client, &adp1650_ops);
	flash->sd.internal_ops = &adp1650_internal_ops;
	flash->sd.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	flash->mode = ATOMISP_FLASH_MODE_OFF;
	flash->timeout = ADP1650_MAX_TIMEOUT / ADP1650_TIMEOUT_STEPSIZE - 1;  

	err = media_entity_init(&flash->sd.entity, 0, NULL, 0);
	if (err) {
		dev_err(&client->dev, "error initialize a media entity.\n");
		goto fail1;
	}

	flash->sd.entity.type = MEDIA_ENT_T_V4L2_SUBDEV_FLASH;

	mutex_init(&flash->power_lock);

	setup_timer(&flash->flash_off_delay, adp1650_flash_off_delay, //
		    (unsigned long)client);

	err = adp1650_gpio_init(client);
	if (err) {
		dev_err(&client->dev, "gpio request/direction_output fail");
		goto fail2;
	}
	#if ADP1650_PRINT
	  dev_err(&client->dev, "exit ADP1650_probe\n");
	#endif
	#ifdef CONFIG_PROJECT_V975
	/* for flash prj cmd test start */
	adp1650_client=client;
      	create_adp1650_proc_file(client);
	/* for flash prj cmd test end  */
	#endif
	return 0;
fail2:
	media_entity_cleanup(&flash->sd.entity);
fail1:
	v4l2_device_unregister_subdev(&flash->sd);
	kfree(flash);

	return err;
}

static int __devexit adp1650_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct adp1650 *flash = to_adp1650(sd);
	int ret;

	media_entity_cleanup(&flash->sd.entity);
	v4l2_device_unregister_subdev(sd);

	del_timer_sync(&flash->flash_off_delay);

	ret = adp1650_gpio_uninit(client);
	if (ret < 0)
		goto fail;

	kfree(flash);

	return 0;
fail:
	dev_err(&client->dev, "gpio request/direction_output fail");
	return ret;
}

static const struct i2c_device_id adp1650_id[] = {
	{ADP1650_NAME, 0},
	{},
};

MODULE_DEVICE_TABLE(i2c, adp1650_id);

static const struct dev_pm_ops adp1650_pm_ops = {
	.suspend = adp1650_suspend,
	.resume = adp1650_resume,
};

static struct i2c_driver adp1650_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = ADP1650_NAME,
		.pm   = &adp1650_pm_ops,
	},
	.probe = adp1650_probe,
	.remove = __devexit_p(adp1650_remove),
	.id_table = adp1650_id,
};

static __init int init_adp1650(void)
{
	return i2c_add_driver(&adp1650_driver);
}

static __exit void exit_adp1650(void)
{
	i2c_del_driver(&adp1650_driver);
}

module_init(init_adp1650);
module_exit(exit_adp1650);
MODULE_AUTHOR("Yao Ling <yao.lingling@zte.com.cn>");
MODULE_DESCRIPTION("LED flash driver for ADP1650");
MODULE_LICENSE("GPL");
