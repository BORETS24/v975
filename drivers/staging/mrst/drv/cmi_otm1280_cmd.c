/*
 * Copyright  2010 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 * Jim Liu <jim.liu@intel.com>
 * Jackie Li <yaodong.li@intel.com>
 */
#include "mdfld_dsi_dbi.h"
#include "mdfld_dsi_pkg_sender.h"
#include <linux/gpio.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/intel_pmic.h>
#include <linux/regulator/machine.h>
#include <asm/intel_scu_pmic.h>

static u8 OTM1280_CMI_Param1[]={0xff,0x12,0x80,0x01};
static u8 OTM1280_CMI_Param2[]={0x00,0x80};
static u8 OTM1280_CMI_Param3[]={0xff,0x12,0x80};
static u8 OTM1280_CMI_Param4[]={0x00,0xa0};
static u8 OTM1280_CMI_Param5[]={0xb3,0x38,0x38};
static u8 OTM1280_CMI_Param6[]={0x00,0x80};
static u8 OTM1280_CMI_Param7[]={0xcb,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static u8 OTM1280_CMI_Param8[]={0x00,0x90};
static u8 OTM1280_CMI_Param9[]={0xcb,0x00,0xc0,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static u8 OTM1280_CMI_Param10[]={0x00,0xa0};
static u8 OTM1280_CMI_Param11[]={0xcb,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static u8 OTM1280_CMI_Param12[]={0x00,0xb0};
static u8 OTM1280_CMI_Param13[]={0xcb,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static u8 OTM1280_CMI_Param14[]={0x00,0xc0};
static u8 OTM1280_CMI_Param15[]={0xcb,0x04,0x00,0x0f,0x00,0x00,0x00,0x04,0x04,0x04,0x04,0x04,0x04,0xf4};
static u8 OTM1280_CMI_Param16[]={0x00,0xd0};
static u8 OTM1280_CMI_Param17[]={0xcb,0xf4,0xf4,0x00,0xf4,0x08,0x04,0x04,0x04,0x00,0x00,0x00,0x00,0x00};
static u8 OTM1280_CMI_Param18[]={0x00,0xe0};
static u8 OTM1280_CMI_Param19[]={0xcb,0x55,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x00,0x00};
static u8 OTM1280_CMI_Param20[]={0x00,0xf0};
static u8 OTM1280_CMI_Param21[]={0xcb,0x00,0x70,0x01,0x00,0x00};
static u8 OTM1280_CMI_Param22[]={0x00,0x80};
static u8 OTM1280_CMI_Param23[]={0xcc,0x41,0x42,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x52,0x55,0x43,0x53,0x65,0x51,0x4D,0x4E,0x4F,0x91,0x8D,0x8E,0x8F,0x40,0x40,0x40,0x40};
static u8 OTM1280_CMI_Param24[]={0x00,0xa0};
static u8 OTM1280_CMI_Param25[]={0xcc,0x41,0x42,0x47,0x48,0x4C,0x4B,0x4A,0x49,0x52,0x55,0x43,0x53,0x65,0x51,0x4D,0x4E,0x4F,0x91,0x8D,0x8E,0x8F,0x40,0x40,0x40,0x40,0xFF,0xFF,0xFF,0x01};
static u8 OTM1280_CMI_Param26[]={0x00,0xc0};
static u8 OTM1280_CMI_Param27[]={0xcc,0x41,0x42,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x52,0x55,0x43,0x53,0x54,0x51,0x4D,0x4E,0x4F,0x91,0x8D,0x8E,0x8F,0x40,0x40,0x40,0x40};
static u8 OTM1280_CMI_Param28[]={0x00,0xe0};
static u8 OTM1280_CMI_Param29[]={0xcc,0x41,0x42,0x47,0x48,0x4C,0x4B,0x4A,0x49,0x52,0x55,0x43,0x53,0x54,0x51,0x4D,0x4E,0x4F,0x91,0x8D,0x8E,0x8F,0x40,0x40,0x40,0x40,0xFF,0xFF,0xFF,0x01};
static u8 OTM1280_CMI_Param30[]={0x00,0x90};
static u8 OTM1280_CMI_Param31[]={0xc1,0x22,0x00,0x00,0x00,0x00};
static u8 OTM1280_CMI_Param32[]={0x00,0x80};
static u8 OTM1280_CMI_Param33[]={0xc0,0x00,0x87,0x00,0x06,0x0a,0x00,0x87,0x06,0x0a,0x00,0x00,0x00};
static u8 OTM1280_CMI_Param34[]={0x00,0x90}; 
static u8 OTM1280_CMI_Param35[]={0xc0,0x00,0x0a,0x00,0x14,0x00,0x2a};
static u8 OTM1280_CMI_Param36[]={0x00,0xa0};
static u8 OTM1280_CMI_Param37[]={0xc0,0x00,0x03,0x01,0x01,0x01,0x01,0x1a,0x03,0x00,0x02};
static u8 OTM1280_CMI_Param38[]={0x00,0x80};
static u8 OTM1280_CMI_Param39[]={0xc2,0x03,0x02,0x00,0x00,0x00,0x02,0x00,0x22};
static u8 OTM1280_CMI_Param40[]={0x00,0x90};
static u8 OTM1280_CMI_Param41[]={0xc2,0x03,0x00,0xff,0xff,0x00,0x00,0x00,0x00,0x22};
static u8 OTM1280_CMI_Param42[]={0x00,0xb0};
static u8 OTM1280_CMI_Param43[]={0xc2,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static u8 OTM1280_CMI_Param44[]={0x00,0xa0};
static u8 OTM1280_CMI_Param45[]={0xc2,0xff,0x00,0xff,0x00,0x00,0x0a,0x00,0x0a};
static u8 OTM1280_CMI_Param46[]={0x00,0xc0};
static u8 OTM1280_CMI_Param47[]={0xc2,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static u8 OTM1280_CMI_Param48[]={0x00,0xe0};
static u8 OTM1280_CMI_Param49[]={0xc2,0x84,0x00,0x10,0x0d};
static u8 OTM1280_CMI_Param50[]={0x00,0xb3};
static u8 OTM1280_CMI_Param51[]={0xc0,0x0f};
static u8 OTM1280_CMI_Param52[]={0x00,0xa2};
static u8 OTM1280_CMI_Param53[]={0xc1,0xfff};
static u8 OTM1280_CMI_Param54[]={0x00,0xb4};
static u8 OTM1280_CMI_Param55[]={0xc0,0x54,0x00};
static u8 OTM1280_CMI_Param56[]={0x00,0x80};
static u8 OTM1280_CMI_Param57[]={0xc5,0x20,0x07,0x00,0xb0,0xb0,0x00,0x00,0x00}; 
static u8 OTM1280_CMI_Param58[]={0x00,0x90};
static u8 OTM1280_CMI_Param59[]={0xc5,0x30,0x85,0x02,0x88,0x96,0x15,0x00,0x0c};
static u8 OTM1280_CMI_Param60[]={0x00,0x00};
static u8 OTM1280_CMI_Param61[]={0xd8,0x52,0x00,0x52,0x00}; 
static u8 OTM1280_CMI_Param62[]={0x00,0x00};
static u8 OTM1280_CMI_Param63[]={0xd9,0x8f,0x73,0x80};
static u8 OTM1280_CMI_Param64[]={0x00,0xc0};
static u8 OTM1280_CMI_Param65[]={0xc0,0x95};
static u8 OTM1280_CMI_Param66[]={0x00,0xd0};
static u8 OTM1280_CMI_Param67[]={0xc0,0x05};
static u8 OTM1280_CMI_Param68[]={0x00,0xb6};
static u8 OTM1280_CMI_Param69[]={0xf5,0x00,0x00};
static u8 OTM1280_CMI_Param70[]={0x00,0xb0};
static u8 OTM1280_CMI_Param71[]={0xb3,0x11};
static u8 OTM1280_CMI_Param72[]={0x00,0xb0};
static u8 OTM1280_CMI_Param73[]={0xf5,0x00,0x20};
static u8 OTM1280_CMI_Param74[]={0x00,0xb8};
static u8 OTM1280_CMI_Param75[]={0xf5,0x0c,0x12};
static u8 OTM1280_CMI_Param76[]={0x00,0x94};  
static u8 OTM1280_CMI_Param77[]={0xf5,0x0a,0x14,0x06,0x17};
static u8 OTM1280_CMI_Param78[]={0x00,0xa2};  
static u8 OTM1280_CMI_Param79[]={0xf5,0x0a,0x14,0x07,0x14};
static u8 OTM1280_CMI_Param80[]={0x00,0x90};
static u8 OTM1280_CMI_Param81[]={0xf5,0x07,0x16,0x07,0x14};
static u8 OTM1280_CMI_Param82[]={0x00,0xa0};
static u8 OTM1280_CMI_Param83[]={0xf5,0x02,0x12,0x0a,0x12,0x07,0x12,0x06,0x12,0x0b,0x12,0x08,0x12}; 
static u8 OTM1280_CMI_Param84[]={0x00,0x00};
static u8 OTM1280_CMI_Param85[]={0xE1,0x2C,0x2F,0x36,0x3E,0x0B,0x05,0x14,0x09,0x07,0x08,0x09,0x1C,0x05,0x0B,0x11,0x0E,0x0B,0x0B};
static u8 OTM1280_CMI_Param86[]={0x00,0x00};
static u8 OTM1280_CMI_Param87[]={0xE2,0x2C,0x2F,0x36,0x3E,0x0B,0x05,0x14,0x09,0x07,0x08,0x09,0x1C,0x05,0x0B,0x11,0x0E,0x0B,0x0B};
static u8 OTM1280_CMI_Param88[]={0x00,0x00};
static u8 OTM1280_CMI_Param89[]={0xE3,0x2C,0x2E,0x35,0x3C,0x0D,0x06,0x16,0x09,0x07,0x08,0x0A,0x1A,0x05,0x0B,0x12,0x0E,0x0B,0x0B};
static u8 OTM1280_CMI_Param90[]={0x00,0x00};
static u8 OTM1280_CMI_Param91[]={0xE4,0x2C,0x2E,0x35,0x3C,0x0D,0x06,0x16,0x09,0x07,0x08,0x0A,0x1A,0x05,0x0B,0x12,0x0E,0x0B,0x0B};
static u8 OTM1280_CMI_Param92[]={0x00,0x00};
static u8 OTM1280_CMI_Param93[]={0xE5,0x0E,0x16,0x23,0x2E,0x0E,0x07,0x1C,0x0A,0x08,0x07,0x09,0x19,0x05,0x0C,0x11,0x0D,0x0B,0x0B};
static u8 OTM1280_CMI_Param94[]={0x00,0x00};
static u8 OTM1280_CMI_Param95[]={0xE6,0x0E,0x16,0x23,0x2E,0x0E,0x07,0x1C,0x0A,0x08,0x07,0x09,0x19,0x05,0x0C,0x11,0x0D,0x0B,0x0B};
static u8 OTM1280_CMI_Param96[]={0x36,0xC0};

static int mdfld_dsi_send_mcs_lp(struct mdfld_dsi_pkg_sender *sender,
				u8 *data, u32 len) {
	int ret = 0;
	if (len > 2) {
		ret = mdfld_dsi_send_mcs_long_lp(sender, data, len, 0);
	} else {
		ret = mdfld_dsi_send_mcs_short_lp(sender, data[0], data[1], 1, 0);
	}
	return ret;
}

static
int mdfld_dsi_otm1280_cmi_drv_ic_init(struct mdfld_dsi_config *dsi_config)
{
	struct drm_device *dev = dsi_config->dev;
	struct mdfld_dsi_pkg_sender *sender
			= mdfld_dsi_get_pkg_sender(dsi_config);

	if (!sender)
		return -EINVAL;

	PSB_DEBUG_ENTRY("\n");
	sender->status = MDFLD_DSI_PKG_SENDER_FREE;

	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param1,sizeof(OTM1280_CMI_Param1));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param2,sizeof(OTM1280_CMI_Param2));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param3,sizeof(OTM1280_CMI_Param3));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param4,sizeof(OTM1280_CMI_Param4));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param5,sizeof(OTM1280_CMI_Param5));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param6,sizeof(OTM1280_CMI_Param6));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param7,sizeof(OTM1280_CMI_Param7));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param8,sizeof(OTM1280_CMI_Param8));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param9,sizeof(OTM1280_CMI_Param9));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param10,sizeof(OTM1280_CMI_Param10));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param11,sizeof(OTM1280_CMI_Param11));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param12,sizeof(OTM1280_CMI_Param12));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param13,sizeof(OTM1280_CMI_Param13));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param14,sizeof(OTM1280_CMI_Param14));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param15,sizeof(OTM1280_CMI_Param15));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param16,sizeof(OTM1280_CMI_Param16));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param17,sizeof(OTM1280_CMI_Param17));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param18,sizeof(OTM1280_CMI_Param18));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param19,sizeof(OTM1280_CMI_Param19));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param20,sizeof(OTM1280_CMI_Param20));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param21,sizeof(OTM1280_CMI_Param21));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param22,sizeof(OTM1280_CMI_Param22));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param23,sizeof(OTM1280_CMI_Param23));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param24,sizeof(OTM1280_CMI_Param24));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param25,sizeof(OTM1280_CMI_Param25));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param26,sizeof(OTM1280_CMI_Param26));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param27,sizeof(OTM1280_CMI_Param27));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param28,sizeof(OTM1280_CMI_Param28));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param29,sizeof(OTM1280_CMI_Param29));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param30,sizeof(OTM1280_CMI_Param30));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param31,sizeof(OTM1280_CMI_Param31));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param32,sizeof(OTM1280_CMI_Param32));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param33,sizeof(OTM1280_CMI_Param33));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param34,sizeof(OTM1280_CMI_Param34));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param35,sizeof(OTM1280_CMI_Param35));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param36,sizeof(OTM1280_CMI_Param36));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param37,sizeof(OTM1280_CMI_Param37));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param38,sizeof(OTM1280_CMI_Param38));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param39,sizeof(OTM1280_CMI_Param39));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param40,sizeof(OTM1280_CMI_Param40));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param41,sizeof(OTM1280_CMI_Param41));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param42,sizeof(OTM1280_CMI_Param42));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param43,sizeof(OTM1280_CMI_Param43));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param44,sizeof(OTM1280_CMI_Param44));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param45,sizeof(OTM1280_CMI_Param45));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param46,sizeof(OTM1280_CMI_Param46));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param47,sizeof(OTM1280_CMI_Param47));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param48,sizeof(OTM1280_CMI_Param48));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param49,sizeof(OTM1280_CMI_Param49));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param50,sizeof(OTM1280_CMI_Param50));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param51,sizeof(OTM1280_CMI_Param51));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param52,sizeof(OTM1280_CMI_Param52));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param53,sizeof(OTM1280_CMI_Param53));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param54,sizeof(OTM1280_CMI_Param54));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param55,sizeof(OTM1280_CMI_Param55));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param56,sizeof(OTM1280_CMI_Param56));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param57,sizeof(OTM1280_CMI_Param57));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param58,sizeof(OTM1280_CMI_Param58));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param59,sizeof(OTM1280_CMI_Param59));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param60,sizeof(OTM1280_CMI_Param60));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param61,sizeof(OTM1280_CMI_Param61));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param62,sizeof(OTM1280_CMI_Param62));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param63,sizeof(OTM1280_CMI_Param63));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param64,sizeof(OTM1280_CMI_Param64));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param65,sizeof(OTM1280_CMI_Param65));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param66,sizeof(OTM1280_CMI_Param66));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param67,sizeof(OTM1280_CMI_Param67));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param68,sizeof(OTM1280_CMI_Param68));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param69,sizeof(OTM1280_CMI_Param69));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param70,sizeof(OTM1280_CMI_Param70));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param71,sizeof(OTM1280_CMI_Param71));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param72,sizeof(OTM1280_CMI_Param72));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param73,sizeof(OTM1280_CMI_Param73));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param74,sizeof(OTM1280_CMI_Param74));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param75,sizeof(OTM1280_CMI_Param75));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param76,sizeof(OTM1280_CMI_Param76));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param77,sizeof(OTM1280_CMI_Param77));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param78,sizeof(OTM1280_CMI_Param78));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param79,sizeof(OTM1280_CMI_Param79));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param80,sizeof(OTM1280_CMI_Param80));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param81,sizeof(OTM1280_CMI_Param81));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param82,sizeof(OTM1280_CMI_Param82));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param83,sizeof(OTM1280_CMI_Param83));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param84,sizeof(OTM1280_CMI_Param84));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param85,sizeof(OTM1280_CMI_Param85));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param86,sizeof(OTM1280_CMI_Param86));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param87,sizeof(OTM1280_CMI_Param87));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param88,sizeof(OTM1280_CMI_Param88));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param89,sizeof(OTM1280_CMI_Param89));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param90,sizeof(OTM1280_CMI_Param90));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param91,sizeof(OTM1280_CMI_Param91));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param92,sizeof(OTM1280_CMI_Param92));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param93,sizeof(OTM1280_CMI_Param93));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param94,sizeof(OTM1280_CMI_Param94));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param95,sizeof(OTM1280_CMI_Param95));
	mdfld_dsi_send_mcs_lp(sender,OTM1280_CMI_Param96,sizeof(OTM1280_CMI_Param96));

#ifdef CONFIG_BL_LCD_PWM_CONTROL
	mdfld_dsi_send_mcs_short_lp(sender, 0x00, 0xB1, 1, 0);
       mdfld_dsi_send_mcs_short_lp(sender, 0xC6, 0x03, 1, 0);
       mdfld_dsi_send_mcs_short_lp(sender, 0x55, 0x00, 1, 0);
       mdfld_dsi_send_mcs_short_lp(sender, 0x53, 0x24, 1, 0);
       mdfld_dsi_send_mcs_short_lp(sender, 0x51, 0x00, 1, 0);
#endif

	return 0;
}

static
void mdfld_dsi_otm1280_cmi_dsi_controller_init(struct mdfld_dsi_config *dsi_config)
{

	struct mdfld_dsi_hw_context *hw_ctx = &dsi_config->dsi_hw_context;
	struct drm_device *dev = dsi_config->dev;

	PSB_DEBUG_ENTRY("\n");

	/*reconfig lane configuration*/
	dsi_config->lane_count = 4;
	dsi_config->lane_config = MDFLD_DSI_DATA_LANE_4_0;
	/* This is for 400 mhz.  Set it to 0 for 800mhz */
	hw_ctx->cck_div = 1;
	hw_ctx->pll_bypass_mode = 0;

	hw_ctx->mipi_control = 0x00;
	hw_ctx->intr_en = 0xffffffff;
	hw_ctx->hs_tx_timeout = 0xffffff;
	hw_ctx->lp_rx_timeout = 0xffffff;
	hw_ctx->turn_around_timeout = 0x1f;
	hw_ctx->device_reset_timer = 0xffff;
	hw_ctx->high_low_switch_count = 0x20;
	hw_ctx->init_count = 0xf0;
	hw_ctx->eot_disable = 0x2;
	hw_ctx->lp_byteclk = 0x4;
	hw_ctx->clk_lane_switch_time_cnt = 0x20000E;
	hw_ctx->hs_ls_dbi_enable = 0x0;
	/* HW team suggested 1390 for bandwidth setting */
	hw_ctx->dbi_bw_ctrl = 1024;
	hw_ctx->dphy_param = 0x20124E1A;
	hw_ctx->dsi_func_prg = (0xa000 | dsi_config->lane_count);
	hw_ctx->mipi = TE_TRIGGER_GPIO_PIN;
	hw_ctx->mipi |= dsi_config->lane_config;
}

static
struct drm_display_mode *otm1280_cmi_cmd_get_config_mode(void)
{
	struct drm_display_mode *mode;

	PSB_DEBUG_ENTRY("\n");

	mode = kzalloc(sizeof(*mode), GFP_KERNEL);
	if (!mode)
		return NULL;

	mode->hdisplay = 720;
	mode->hsync_start = mode->hdisplay + 32;
	mode->hsync_end = mode->hsync_start + 10;
	mode->htotal = mode->hsync_end + 40;
	mode->vdisplay = 1280;
	mode->vsync_start = mode->vdisplay + 10;
	mode->vsync_end = mode->vsync_start + 10;
	mode->vtotal = mode->vsync_end + 140;
	mode->vrefresh = 60;
	mode->clock =  mode->vrefresh * mode->vtotal * mode->htotal / 1000;
	mode->type |= DRM_MODE_TYPE_PREFERRED;

	PSB_DEBUG_ENTRY("hdisplay is %d\n", mode->hdisplay);
	PSB_DEBUG_ENTRY("vdisplay is %d\n", mode->vdisplay);
	PSB_DEBUG_ENTRY("HSS is %d\n", mode->hsync_start);
	PSB_DEBUG_ENTRY("HSE is %d\n", mode->hsync_end);
	PSB_DEBUG_ENTRY("htotal is %d\n", mode->htotal);
	PSB_DEBUG_ENTRY("VSS is %d\n", mode->vsync_start);
	PSB_DEBUG_ENTRY("VSE is %d\n", mode->vsync_end);
	PSB_DEBUG_ENTRY("vtotal is %d\n", mode->vtotal);
	PSB_DEBUG_ENTRY("clock is %d\n", mode->clock);

	drm_mode_set_name(mode);
	drm_mode_set_crtcinfo(mode, 0);

	return mode;
}

static
int mdfld_dsi_otm1280_cmi_power_on(struct mdfld_dsi_config *dsi_config)
{

	struct mdfld_dsi_pkg_sender *sender =
		mdfld_dsi_get_pkg_sender(dsi_config);
	int err = 0;
	int enable_err, enabled = 0;

	PSB_DEBUG_ENTRY("\n");

	if (!sender) {
		DRM_ERROR("Failed to get DSI packet sender\n");
		return -EINVAL;
	}

	mdfld_dsi_send_mcs_short_lp(sender, 0x11, 0x00, 0, 0);
	mdelay(120);

	mdfld_dsi_send_mcs_short_lp(sender, 0x35, 0x00, 1, 0);

	mdfld_dsi_send_mcs_short_lp(sender, 0x29, 0x00, 0, 0);
	mdelay(120);

failed:
	return err;

}

static int mdfld_dsi_otm1280_cmi_power_off(struct mdfld_dsi_config *dsi_config)
{
	struct mdfld_dsi_pkg_sender *sender =
		mdfld_dsi_get_pkg_sender(dsi_config);
	int err = 0;

	PSB_DEBUG_ENTRY("\n");

	if (!sender) {
		DRM_ERROR("Failed to get DSI packet sender\n");
		return -EINVAL;
	}

	mdfld_dsi_send_mcs_short_lp(sender, 0x28, 0x00, 0, 0);
	mdelay(50);
	mdfld_dsi_send_mcs_short_lp(sender, 0x34, 0x00, 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, 0x10, 0x00, 0, 0);
	mdelay(120);

failed:
	return err;
}

static
void otm1280_cmi_cmd_get_panel_info(int pipe, struct panel_info *pi)
{
	PSB_DEBUG_ENTRY("\n");

	if (pipe == 0) {
		pi->width_mm = PANEL_4DOT3_WIDTH;
		pi->height_mm = PANEL_4DOT3_HEIGHT;
	}
}

static
int mdfld_dsi_otm1280_cmi_detect(struct mdfld_dsi_config *dsi_config)
{
	int status;
	struct drm_device *dev = dsi_config->dev;
	struct mdfld_dsi_hw_registers *regs = &dsi_config->regs;
	u32 dpll_val, device_ready_val;
	int pipe = dsi_config->pipe;
	struct mdfld_dsi_pkg_sender *sender =
		mdfld_dsi_get_pkg_sender(dsi_config);

	PSB_DEBUG_ENTRY("\n");

	if (pipe == 0) {
		/*
		 * FIXME: WA to detect the panel connection status, and need to
		 * implement detection feature with get_power_mode DSI command.
		 */
		if (!ospm_power_using_hw_begin(OSPM_DISPLAY_ISLAND,
					OSPM_UHB_FORCE_POWER_ON)) {
			DRM_ERROR("hw begin failed\n");
			return -EAGAIN;
		}

		dpll_val = REG_READ(regs->dpll_reg);
		device_ready_val = REG_READ(regs->device_ready_reg);
		if ((device_ready_val & DSI_DEVICE_READY) &&
		    (dpll_val & DPLL_VCO_ENABLE)) {
			dsi_config->dsi_hw_context.panel_on = true;
		} else {
			dsi_config->dsi_hw_context.panel_on = false;
			DRM_INFO("%s: panel is not detected!\n", __func__);
		}

		status = MDFLD_DSI_PANEL_CONNECTED;

		ospm_power_using_hw_end(OSPM_DISPLAY_ISLAND);
	} else {
		DRM_INFO("%s: do NOT support dual panel\n", __func__);
		status = MDFLD_DSI_PANEL_DISCONNECTED;
	}

	return status;
}

static
int mdfld_dsi_otm1280_cmi_set_brightness(struct mdfld_dsi_config *dsi_config,
		int level)
{
	struct mdfld_dsi_pkg_sender *sender =
		mdfld_dsi_get_pkg_sender(dsi_config);
	u8 backlight_value = 0;

	if (!sender) {
		DRM_ERROR("Failed to get DSI packet sender\n");
		return -EINVAL;
	}

	PSB_DEBUG_ENTRY("level=%d\n", level);
#ifdef CONFIG_BL_PMU_PWM_CONTROL
	if (level) {
		backlight_value = 35 + (level *65) /100;
		backlight_value = (backlight_value * 0x63) / 100;
	}
	intel_scu_ipc_iowrite8(0x67, backlight_value);
#endif

#ifdef CONFIG_BL_LCD_PWM_CONTROL
       if (level > 80) {
               backlight_value = 153 + ((level-80) * 102) /20;
       } else {
               backlight_value = (level * 153) /80;
       }
	mdfld_dsi_send_mcs_short_lp(sender, 0x51, backlight_value, 1, 0);
#endif

	return 0;
}

static
int mdfld_dsi_otm1280_cmi_panel_reset(struct mdfld_dsi_config *dsi_config)
{
	static int mipi_reset_gpio;
	int ret = 0;
	u32 data = 0;

	PSB_DEBUG_ENTRY("\n");

	if (mipi_reset_gpio == 0) {
		ret = get_gpio_by_name("mipi-reset");
		if (ret < 0) {
			DRM_ERROR("Faild to get panel reset gpio, " \
				  "use default reset pin\n");
			ret = 57;
		}

		mipi_reset_gpio = ret;

		ret = gpio_request(mipi_reset_gpio, "mipi_display");
		if (ret) {
			DRM_ERROR("Faild to request panel reset gpio\n");
			return -EINVAL;
		}

		gpio_direction_output(mipi_reset_gpio, 0);
	}

	gpio_set_value_cansleep(mipi_reset_gpio, 0);
	mdelay(11);

	gpio_set_value_cansleep(mipi_reset_gpio, 1);
	mdelay(5);

	return 0;
}

#define PWM0CLKDIV0   0x62
#define PWM0CLKDIV1   0x61
#define SYSTEMCLK        196608
#define PWM_FREQUENCY  40000

static inline u16 calc_clkdiv(unsigned long baseclk, unsigned int f)
{
	return (baseclk -f) / f;
}


static void otm1280_cmi_brightness_init(struct drm_device *dev)
{
    int ret;
    u8 pwmctrl;
    u16 clkdiv;

    /* Make sure the PWM reference is the 19.2 MHz system clock. Read first
     * * instead of setting directly to catch potential conflicts between PWM
     * * users. */
    clkdiv = calc_clkdiv(SYSTEMCLK, PWM_FREQUENCY);

    ret = intel_scu_ipc_iowrite8(PWM0CLKDIV1, (clkdiv >> 8) & 0xff);
    if (!ret)
        ret = intel_scu_ipc_iowrite8(PWM0CLKDIV0, clkdiv & 0xff);

    if (ret)
        dev_err(&dev->pdev->dev, "PWM0CLKDIV set failed\n");
    else
        dev_dbg(&dev->pdev->dev, "PWM0CLKDIV set to 0x%04x (%d Hz)\n",
                clkdiv, PWM_FREQUENCY);
}


static
void otm1280_cmi_cmd_init(struct drm_device *dev, struct panel_funcs *p_funcs)
{
	int ena_err;
	if (!dev || !p_funcs) {
		DRM_ERROR("Invalid parameters\n");
		return;
	}
	PSB_DEBUG_ENTRY("\n");
	p_funcs->get_config_mode = otm1280_cmi_cmd_get_config_mode;
	p_funcs->get_panel_info = otm1280_cmi_cmd_get_panel_info;
	p_funcs->reset = mdfld_dsi_otm1280_cmi_panel_reset;
	p_funcs->drv_ic_init = mdfld_dsi_otm1280_cmi_drv_ic_init;
	p_funcs->dsi_controller_init = mdfld_dsi_otm1280_cmi_dsi_controller_init;
	p_funcs->detect = mdfld_dsi_otm1280_cmi_detect;
	p_funcs->power_on = mdfld_dsi_otm1280_cmi_power_on;
	p_funcs->power_off = mdfld_dsi_otm1280_cmi_power_off;
	p_funcs->set_brightness = mdfld_dsi_otm1280_cmi_set_brightness;
#ifdef CONFIG_BL_PMU_PWM_CONTROL
	otm1280_cmi_brightness_init(dev);
#endif
}

static int otm1280_cmi_cmd_probe(struct platform_device *pdev)
{
	int ret = 0;

	DRM_INFO("%s: OTM1280 CMI panel detected\n", __func__);
	intel_mid_panel_register(otm1280_cmi_cmd_init);

	return 0;
}

static struct platform_driver otm1280_cmi_lcd_driver = {
	.probe	= otm1280_cmi_cmd_probe,
	.driver	= {
		.name	= "OTM1280 CMD RHB",
		.owner	= THIS_MODULE,
	},
};

static int __init otm1280_cmi_lcd_init(void)
{
	DRM_INFO("%s\n", __func__);

	return  platform_driver_register(&otm1280_cmi_lcd_driver);
}
module_init(otm1280_cmi_lcd_init);

