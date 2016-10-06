/*
 * Copyright Â© 2010 Intel Corporation
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
#include "mdfld_dsi_dpi.h"
#include "mdfld_dsi_pkg_sender.h"
#include <linux/gpio.h>
#include <linux/sfi.h>

#include <asm/intel_scu_pmic.h>

static u8 OTM1283A_auo_param1[]={0x00,0x00};
static u8 OTM1283A_auo_param2[]={0xff, 0x12,0x83,0x01};
static u8 OTM1283A_auo_param3[]={0x00,0x80};
static u8 OTM1283A_auo_param4[]={0xff, 0x12,0x83};
static u8 OTM1283A_auo_param5[]={0x00,0x80};
static u8 OTM1283A_auo_param6[]={0xc0,0x00,0x64,0x00,0x10,0x10,0x00,0x64,0x10,0x10};
static u8 OTM1283A_auo_param7[]={0x00,0x90};
static u8 OTM1283A_auo_param8[]={0xc0,0x00,0x5c,0x00,0x01,0x00,0x04};
static u8 OTM1283A_auo_param9[]={0x00,0xa2};
static u8 OTM1283A_auo_param10[]={0xC0,0x01,0x00,0x00};
static u8 OTM1283A_auo_param11[]={0x00,0xb3};
static u8 OTM1283A_auo_param12[]={0xc0,0x00,0x50};
static u8 OTM1283A_auo_param13[]={0x00,0x81};
static u8 OTM1283A_auo_param14[]={0xc1,0x55};
static u8 OTM1283A_auo_param15[]={0x00,0x90};
static u8 OTM1283A_auo_param16[]={0xc4,0x49};
static u8 OTM1283A_auo_param17[]={0x00,0xa0};
static u8 OTM1283A_auo_param18[]={0xc4,0x05,0x10,0x04,0x02,0x05,0x15,0x11,0x05,0x10,0x07,0x02,0x05,0x15,0x11};
static u8 OTM1283A_auo_param19[]={0x00,0xb0};
static u8 OTM1283A_auo_param20[]={0xc4,0x00,0x00};
static u8 OTM1283A_auo_param21[]={0x00,0x91};
static u8 OTM1283A_auo_param22[]={0xc5,0x29,0x50};
static u8 OTM1283A_auo_param23[]={0x00,0x00};
static u8 OTM1283A_auo_param24[]={0xd8,0x84,0x84}; 
static u8 OTM1283A_auo_param25[]={0x00,0x00};
static u8 OTM1283A_auo_param26[]={0xd9,0x28};
static u8 OTM1283A_auo_param27[]={0x00,0x81};
static u8 OTM1283A_auo_param28[]={0xc4,0x82};
static u8 OTM1283A_auo_param29[]={0x00,0xb0};
static u8 OTM1283A_auo_param30[]={0xc5,0x04,0xb8};
static u8 OTM1283A_auo_param31[]={0x00,0xbb};
static u8 OTM1283A_auo_param32[]={0xc5,0x80};
static u8 OTM1283A_auo_param33[]={0x00,0x82};
static u8 OTM1283A_auo_param34[]={0xC4,0x02};
static u8 OTM1283A_auo_param35[]={0x00,0xc6};
static u8 OTM1283A_auo_param36[]={0xb0,0x03};
static u8 OTM1283A_auo_param37[]={0x00,0x00};
static u8 OTM1283A_auo_param38[]={0xd0,0x40};
static u8 OTM1283A_auo_param39[]={0x00,0x00};
static u8 OTM1283A_auo_param40[]={0xd1,0x00,0x00};
static u8 OTM1283A_auo_param41[]={0x00,0x80};
static u8 OTM1283A_auo_param42[]={0xc4,0x00};
static u8 OTM1283A_auo_param43[]={0x00,0x98};
static u8 OTM1283A_auo_param44[]={0xc5,0x10};
static u8 OTM1283A_auo_param45[]={0x00,0x81};
static u8 OTM1283A_auo_param46[]={0xf5,0x15};
static u8 OTM1283A_auo_param47[]={0x00,0x83}; 
static u8 OTM1283A_auo_param48[]={0xf5,0x15};
static u8 OTM1283A_auo_param49[]={0x00,0x85};
static u8 OTM1283A_auo_param50[]={0xf5,0x15};
static u8 OTM1283A_auo_param51[]={0x00,0x87}; 
static u8 OTM1283A_auo_param52[]={0xf5,0x15};
static u8 OTM1283A_auo_param53[]={0x00,0x89};
static u8 OTM1283A_auo_param54[]={0xf5,0x15};
static u8 OTM1283A_auo_param55[]={0x00,0x8b}; 
static u8 OTM1283A_auo_param56[]={0xf5,0x15};
static u8 OTM1283A_auo_param57[]={0x00,0x95};
static u8 OTM1283A_auo_param58[]={0xf5,0x15};
static u8 OTM1283A_auo_param59[]={0x00,0x97}; 
static u8 OTM1283A_auo_param60[]={0xf5,0x15};
static u8 OTM1283A_auo_param61[]={0x00,0x99};
static u8 OTM1283A_auo_param62[]={0xf5,0x15};
static u8 OTM1283A_auo_param63[]={0x00,0xa1}; 
static u8 OTM1283A_auo_param64[]={0xf5,0x15};
static u8 OTM1283A_auo_param65[]={0x00,0xa3};
static u8 OTM1283A_auo_param66[]={0xf5,0x15};
static u8 OTM1283A_auo_param67[]={0x00,0xa5}; 
static u8 OTM1283A_auo_param68[]={0xf5,0x15};
static u8 OTM1283A_auo_param69[]={0x00,0xa7}; 
static u8 OTM1283A_auo_param70[]={0xf5,0x15};
static u8 OTM1283A_auo_param71[]={0x00,0xab};
static u8 OTM1283A_auo_param72[]={0xf5,0x18};
static u8 OTM1283A_auo_param73[]={0x00,0x94};
static u8 OTM1283A_auo_param74[]={0xF5,0x02};
static u8 OTM1283A_auo_param75[]={0x00,0xBA};
static u8 OTM1283A_auo_param76[]={0xF5,0x03};
static u8 OTM1283A_auo_param77[]={0x00,0xb1};
static u8 OTM1283A_auo_param78[]={0xf5,0x15,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x15,0x08,0x15};
static u8 OTM1283A_auo_param79[]={0x00,0xb4};
static u8 OTM1283A_auo_param80[]={0xc5,0xc0};
static u8 OTM1283A_auo_param81[]={0x00,0x90};
static u8 OTM1283A_auo_param82[]={0xf5,0x02,0x11,0x02,0x11};
static u8 OTM1283A_auo_param83[]={0x00,0x90};
static u8 OTM1283A_auo_param84[]={0xc5,0x50};
static u8 OTM1283A_auo_param85[]={0x00,0x94};
static u8 OTM1283A_auo_param86[]={0xc5,0x66};
static u8 OTM1283A_auo_param87[]={0x00,0x80};
static u8 OTM1283A_auo_param88[]={0xcb,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static u8 OTM1283A_auo_param89[]={0x00,0x90};
static u8 OTM1283A_auo_param90[]={0xcb,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x00,0xff,0x00};
static u8 OTM1283A_auo_param91[]={0x00,0xa0};
static u8 OTM1283A_auo_param92[]={0xcb,0xff,0x00,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static u8 OTM1283A_auo_param93[]={0x00,0xb0};
static u8 OTM1283A_auo_param94[]={0xcb,0x00,0x00,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0x00,0x00,0x00,0x00};
static u8 OTM1283A_auo_param95[]={0x00,0xc0};
static u8 OTM1283A_auo_param96[]={0xcb,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x05,0x00,0x05,0x05,0x05,0x05,0x05};
static u8 OTM1283A_auo_param97[]={0x00,0xd0};
static u8 OTM1283A_auo_param98[]={0xcb,0x05,0x05,0x05,0x05,0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05};
static u8 OTM1283A_auo_param99[]={0x00,0xe0};
static u8 OTM1283A_auo_param100[]={0xcb,0x05,0x00,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x00,0x00};
static u8 OTM1283A_auo_param101[]={0x00,0xf0};
static u8 OTM1283A_auo_param102[]={0xcb,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
static u8 OTM1283A_auo_param103[]={0x00,0x80};
static u8 OTM1283A_auo_param104[]={0xcc,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x07,0x00,0x0d,0x09,0x0f,0x0b,0x15};
static u8 OTM1283A_auo_param105[]={0x00,0x90};
static u8 OTM1283A_auo_param106[]={0xcc,0x11,0x17,0x13,0x01,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x06};
static u8 OTM1283A_auo_param107[]={0x00,0xa0};
static u8 OTM1283A_auo_param108[]={0xcc,0x08,0x00,0x0e,0x0a,0x10,0x0c,0x16,0x12,0x18,0x14,0x02,0x04,0x00,0x00};
static u8 OTM1283A_auo_param109[]={0x00,0xb0};
static u8 OTM1283A_auo_param110[]={0xcc,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x02,0x00,0x0c,0x10,0x0a,0x0e,0x18};
static u8 OTM1283A_auo_param111[]={0x00,0xc0};
static u8 OTM1283A_auo_param112[]={0xcc,0x14,0x16,0x12,0x08,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03};
static u8 OTM1283A_auo_param113[]={0x00,0xd0}; 
static u8 OTM1283A_auo_param114[]={0xcc,0x01,0x00,0x0b,0x0f,0x09,0x0d,0x17,0x13,0x15,0x11,0x07,0x05,0x00,0x00};
static u8 OTM1283A_auo_param115[]={0x00,0x80};
static u8 OTM1283A_auo_param116[]={0xce,0x87,0x03,0x28,0x86,0x03,0x28,0x85,0x03,0x28,0x84,0x03,0x28};
static u8 OTM1283A_auo_param117[]={0x00,0x90};
static u8 OTM1283A_auo_param118[]={0xce,0x34,0xfc,0x28,0x34,0xfd,0x28,0x34,0xfe,0x28,0x34,0xff,0x28,0x00,0x00};
static u8 OTM1283A_auo_param119[]={0x00,0xa0};
static u8 OTM1283A_auo_param120[]={0xce,0x38,0x07,0x05,0x00,0x00,0x28,0x00,0x38,0x06,0x05,0x01,0x00,0x28,0x00};
static u8 OTM1283A_auo_param121[]={0x00,0xb0};
static u8 OTM1283A_auo_param122[]={0xce,0x38,0x05,0x05,0x02,0x00,0x28,0x00,0x38,0x04,0x05,0x03,0x00,0x28,0x00};
static u8 OTM1283A_auo_param123[]={0x00,0xc0};
static u8 OTM1283A_auo_param124[]={0xce,0x38,0x03,0x05,0x04,0x00,0x28,0x00,0x38,0x02,0x05,0x05,0x00,0x28,0x00};
static u8 OTM1283A_auo_param125[]={0x00,0xd0};
static u8 OTM1283A_auo_param126[]={0xce,0x38,0x01,0x05,0x06,0x00,0x28,0x00,0x38,0x00,0x05,0x07,0x00,0x28,0x00};
static u8 OTM1283A_auo_param127[]={0x00,0x80};
static u8 OTM1283A_auo_param128[]={0xcf,0x38,0x07,0x05,0x00,0x00,0x18,0x25,0x38,0x06,0x05,0x01,0x00,0x18,0x25};
static u8 OTM1283A_auo_param129[]={0x00,0x90};
static u8 OTM1283A_auo_param130[]={0xcf,0x38,0x05,0x05,0x02,0x00,0x18,0x25,0x38,0x04,0x05,0x03,0x00,0x18,0x25};
static u8 OTM1283A_auo_param131[]={0x00,0xa0};
static u8 OTM1283A_auo_param132[]={0xcf,0x38,0x03,0x05,0x04,0x00,0x18,0x25,0x38,0x02,0x05,0x05,0x00,0x18,0x25};
static u8 OTM1283A_auo_param133[]={0x00,0xb0};
static u8 OTM1283A_auo_param134[]={0xcf,0x38,0x01,0x05,0x06,0x00,0x18,0x25,0x38,0x00,0x05,0x07,0x00,0x18,0x25};
static u8 OTM1283A_auo_param135[]={0x00,0xc0};
static u8 OTM1283A_auo_param136[]={0xcf,0x01,0x01,0x20,0x20,0x00,0x00,0x01,0x81,0x00,0x03,0x08};
static u8 OTM1283A_auo_param137[]={0x00,0x00}; 
static u8 OTM1283A_auo_param138[]={0xE1,0x10,0x0D,0x14,0x0F,0x08,0x12,0x0D,0x0C,0x02,0x06,0x0A,0x07,0x0F,0x12,0x0C,0x03};        
static u8 OTM1283A_auo_param139[]={0x00,0x00}; 
static u8 OTM1283A_auo_param140[]={0xE2,0x02,0x0D,0x14,0x0F,0x08,0x12,0x0D,0x0C,0x02,0x06,0x0A,0x07,0x0F,0x12,0x0C,0x09};
static u8 OTM1283A_auo_param141[]={0x00,0x00};
static u8 OTM1283A_auo_param142[]={0xff,0xff,0xff,0xff};

static u8 OTM1283A_auo_color_enhance_para1[]={0x00,0xA0};
static u8 OTM1283A_auo_color_enhance_para2[]={0xD6,0x01,0xCD,0x01,0xCD,0x01,0xCD,0x01,0xCD,0x01,0xCD,0x01,0xCD};
static u8 OTM1283A_auo_color_enhance_para3[]={0x00,0xB0};
static u8 OTM1283A_auo_color_enhance_para4[]={0xD6,0x01,0xCD,0x01,0xCD,0x01,0xCD,0x01,0xCD,0x01,0xCD,0x01,0xCD};
static u8 OTM1283A_auo_color_enhance_para5[]={0x00,0xC0};
static u8 OTM1283A_auo_color_enhance_para6[]={0xD6,0x89,0x11,0x89,0x89,0x11,0x89,0x89,0x11,0x89,0x89,0x11,0x89};
static u8 OTM1283A_auo_color_enhance_para7[]={0x00,0xD0};
static u8 OTM1283A_auo_color_enhance_para8[]={0xD6,0x89,0x11,0x89,0x89,0x11,0x89};
static u8 OTM1283A_auo_color_enhance_para9[]={0x00,0xE0};
static u8 OTM1283A_auo_color_enhance_para10[]={0xD6,0x44,0x11,0x44,0x44,0x11,0x44,0x44,0x11,0x44,0x44,0x11,0x44};
static u8 OTM1283A_auo_color_enhance_para11[]={0x00,0xF0};
static u8 OTM1283A_auo_color_enhance_para12[]={0xD6,0x44,0x11,0x44,0x44,0x11,0x44};
static u8 OTM1283A_auo_color_enhance_para13[]={0x00,0x90}; //Clever CMD1
static u8 OTM1283A_auo_color_enhance_para14[]={0xD6,0x00};
static u8 OTM1283A_auo_color_enhance_para15[]={0x00,0x00}; //CE - Low
static u8 OTM1283A_auo_color_enhance_para16[]={0x55,0x90};


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

static int mdfld_otm1283A_auo_dpi_ic_init(struct mdfld_dsi_config *dsi_config)
{
	struct mdfld_dsi_pkg_sender *sender
			= mdfld_dsi_get_pkg_sender(dsi_config);

	if (!sender) {
		DRM_ERROR("Cannot get sender\n");
		return -EINVAL;
	}

	PSB_DEBUG_ENTRY("\n");
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param1,sizeof(OTM1283A_auo_param1));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param2,sizeof(OTM1283A_auo_param2));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param3,sizeof(OTM1283A_auo_param3));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param4,sizeof(OTM1283A_auo_param4));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param5,sizeof(OTM1283A_auo_param5));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param6,sizeof(OTM1283A_auo_param6));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param7,sizeof(OTM1283A_auo_param7));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param8,sizeof(OTM1283A_auo_param8));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param9,sizeof(OTM1283A_auo_param9));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param10,sizeof(OTM1283A_auo_param10));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param11,sizeof(OTM1283A_auo_param11));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param12,sizeof(OTM1283A_auo_param12));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param13,sizeof(OTM1283A_auo_param13));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param14,sizeof(OTM1283A_auo_param14));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param15,sizeof(OTM1283A_auo_param15));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param16,sizeof(OTM1283A_auo_param16));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param17,sizeof(OTM1283A_auo_param17));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param18,sizeof(OTM1283A_auo_param18));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param19,sizeof(OTM1283A_auo_param19));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param20,sizeof(OTM1283A_auo_param20));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param21,sizeof(OTM1283A_auo_param21));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param22,sizeof(OTM1283A_auo_param22));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param23,sizeof(OTM1283A_auo_param23));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param24,sizeof(OTM1283A_auo_param24));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param25,sizeof(OTM1283A_auo_param25));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param26,sizeof(OTM1283A_auo_param26));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param27,sizeof(OTM1283A_auo_param27));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param28,sizeof(OTM1283A_auo_param28));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param29,sizeof(OTM1283A_auo_param29));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param30,sizeof(OTM1283A_auo_param30));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param31,sizeof(OTM1283A_auo_param31));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param32,sizeof(OTM1283A_auo_param32));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param33,sizeof(OTM1283A_auo_param33));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param34,sizeof(OTM1283A_auo_param34));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param35,sizeof(OTM1283A_auo_param35));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param36,sizeof(OTM1283A_auo_param36));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param37,sizeof(OTM1283A_auo_param37));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param38,sizeof(OTM1283A_auo_param38));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param39,sizeof(OTM1283A_auo_param39));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param40,sizeof(OTM1283A_auo_param40));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param41,sizeof(OTM1283A_auo_param41));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param42,sizeof(OTM1283A_auo_param42));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param43,sizeof(OTM1283A_auo_param43));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param44,sizeof(OTM1283A_auo_param44));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param45,sizeof(OTM1283A_auo_param45));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param46,sizeof(OTM1283A_auo_param46));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param47,sizeof(OTM1283A_auo_param47));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param48,sizeof(OTM1283A_auo_param48));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param49,sizeof(OTM1283A_auo_param49));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param50,sizeof(OTM1283A_auo_param50));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param51,sizeof(OTM1283A_auo_param51));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param52,sizeof(OTM1283A_auo_param52));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param53,sizeof(OTM1283A_auo_param53));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param54,sizeof(OTM1283A_auo_param54));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param55,sizeof(OTM1283A_auo_param55));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param56,sizeof(OTM1283A_auo_param56));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param57,sizeof(OTM1283A_auo_param57));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param58,sizeof(OTM1283A_auo_param58));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param59,sizeof(OTM1283A_auo_param59));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param60,sizeof(OTM1283A_auo_param60));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param61,sizeof(OTM1283A_auo_param61));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param62,sizeof(OTM1283A_auo_param62));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param63,sizeof(OTM1283A_auo_param63));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param64,sizeof(OTM1283A_auo_param64));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param65,sizeof(OTM1283A_auo_param65));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param66,sizeof(OTM1283A_auo_param66));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param67,sizeof(OTM1283A_auo_param67));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param68,sizeof(OTM1283A_auo_param68));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param69,sizeof(OTM1283A_auo_param69));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param70,sizeof(OTM1283A_auo_param70));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param71,sizeof(OTM1283A_auo_param71));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param72,sizeof(OTM1283A_auo_param72));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param73,sizeof(OTM1283A_auo_param73));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param74,sizeof(OTM1283A_auo_param74));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param75,sizeof(OTM1283A_auo_param75));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param76,sizeof(OTM1283A_auo_param76));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param77,sizeof(OTM1283A_auo_param77));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param78,sizeof(OTM1283A_auo_param78));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param79,sizeof(OTM1283A_auo_param79));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param80,sizeof(OTM1283A_auo_param80));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param81,sizeof(OTM1283A_auo_param81));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param82,sizeof(OTM1283A_auo_param82));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param83,sizeof(OTM1283A_auo_param83));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param84,sizeof(OTM1283A_auo_param84));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param85,sizeof(OTM1283A_auo_param85));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param86,sizeof(OTM1283A_auo_param86));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param87,sizeof(OTM1283A_auo_param87));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param88,sizeof(OTM1283A_auo_param88));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param89,sizeof(OTM1283A_auo_param89));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param90,sizeof(OTM1283A_auo_param90));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param91,sizeof(OTM1283A_auo_param91));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param92,sizeof(OTM1283A_auo_param92));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param93,sizeof(OTM1283A_auo_param93));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param94,sizeof(OTM1283A_auo_param94));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param95,sizeof(OTM1283A_auo_param95));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param96,sizeof(OTM1283A_auo_param96));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param97,sizeof(OTM1283A_auo_param97));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param98,sizeof(OTM1283A_auo_param98));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param99,sizeof(OTM1283A_auo_param99));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param100,sizeof(OTM1283A_auo_param100));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param101,sizeof(OTM1283A_auo_param101));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param102,sizeof(OTM1283A_auo_param102));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param103,sizeof(OTM1283A_auo_param103));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param104,sizeof(OTM1283A_auo_param104));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param105,sizeof(OTM1283A_auo_param105));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param106,sizeof(OTM1283A_auo_param106));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param107,sizeof(OTM1283A_auo_param107));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param108,sizeof(OTM1283A_auo_param108));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param109,sizeof(OTM1283A_auo_param109));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param110,sizeof(OTM1283A_auo_param110));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param111,sizeof(OTM1283A_auo_param111));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param112,sizeof(OTM1283A_auo_param112));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param113,sizeof(OTM1283A_auo_param113));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param114,sizeof(OTM1283A_auo_param114));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param115,sizeof(OTM1283A_auo_param115));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param116,sizeof(OTM1283A_auo_param116));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param117,sizeof(OTM1283A_auo_param117));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param118,sizeof(OTM1283A_auo_param118));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param119,sizeof(OTM1283A_auo_param119));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param120,sizeof(OTM1283A_auo_param120));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param121,sizeof(OTM1283A_auo_param121));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param122,sizeof(OTM1283A_auo_param122));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param123,sizeof(OTM1283A_auo_param123));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param124,sizeof(OTM1283A_auo_param124));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param125,sizeof(OTM1283A_auo_param125));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param126,sizeof(OTM1283A_auo_param126));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param127,sizeof(OTM1283A_auo_param127));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param128,sizeof(OTM1283A_auo_param128));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param129,sizeof(OTM1283A_auo_param129));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param130,sizeof(OTM1283A_auo_param130));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param131,sizeof(OTM1283A_auo_param131));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param132,sizeof(OTM1283A_auo_param132));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param133,sizeof(OTM1283A_auo_param133));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param134,sizeof(OTM1283A_auo_param134));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param135,sizeof(OTM1283A_auo_param135));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param136,sizeof(OTM1283A_auo_param136));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param137,sizeof(OTM1283A_auo_param137));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param138,sizeof(OTM1283A_auo_param138));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param139,sizeof(OTM1283A_auo_param139));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param140,sizeof(OTM1283A_auo_param140));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param141,sizeof(OTM1283A_auo_param141));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_param142,sizeof(OTM1283A_auo_param142));

	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_color_enhance_para1,sizeof(OTM1283A_auo_color_enhance_para1));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_color_enhance_para2,sizeof(OTM1283A_auo_color_enhance_para2));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_color_enhance_para3,sizeof(OTM1283A_auo_color_enhance_para3));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_color_enhance_para4,sizeof(OTM1283A_auo_color_enhance_para4));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_color_enhance_para5,sizeof(OTM1283A_auo_color_enhance_para5));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_color_enhance_para6,sizeof(OTM1283A_auo_color_enhance_para6));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_color_enhance_para7,sizeof(OTM1283A_auo_color_enhance_para7));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_color_enhance_para8,sizeof(OTM1283A_auo_color_enhance_para8));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_color_enhance_para9,sizeof(OTM1283A_auo_color_enhance_para9));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_color_enhance_para10,sizeof(OTM1283A_auo_color_enhance_para10));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_color_enhance_para11,sizeof(OTM1283A_auo_color_enhance_para11));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_color_enhance_para12,sizeof(OTM1283A_auo_color_enhance_para12));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_color_enhance_para13,sizeof(OTM1283A_auo_color_enhance_para13));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_color_enhance_para14,sizeof(OTM1283A_auo_color_enhance_para14));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_color_enhance_para15,sizeof(OTM1283A_auo_color_enhance_para15));
	mdfld_dsi_send_mcs_lp(sender,OTM1283A_auo_color_enhance_para16,sizeof(OTM1283A_auo_color_enhance_para16));

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
void mdfld_otm1283A_auo_dpi_controller_init(struct mdfld_dsi_config *dsi_config)
{
	struct mdfld_dsi_hw_context *hw_ctx =
		&dsi_config->dsi_hw_context;

	PSB_DEBUG_ENTRY("\n");

	/*reconfig lane configuration*/
	dsi_config->lane_count = 4;
	dsi_config->lane_config = MDFLD_DSI_DATA_LANE_4_0;
	/* This is for 400 mhz.  Set it to 0 for 800mhz */
	hw_ctx->cck_div = 1;
	hw_ctx->pll_bypass_mode = 0;

	hw_ctx->mipi_control = 0x18;
	hw_ctx->intr_en = 0xffffffff;
	hw_ctx->hs_tx_timeout = 0xffffff;
	hw_ctx->lp_rx_timeout = 0xffffff;
	hw_ctx->turn_around_timeout = 0x1f;
	hw_ctx->device_reset_timer = 0xffff;

	hw_ctx->init_count = 0xf0;
	hw_ctx->eot_disable = 0x3;

	/* b060 */
	hw_ctx->lp_byteclk = 0x3;
	/* B044 */
	hw_ctx->high_low_switch_count = 0x16;
	/* b088 */
	hw_ctx->clk_lane_switch_time_cnt = 0x0016000A;
	/* b080 */
	hw_ctx->dphy_param = 0x150C340F;

	/*setup video mode format*/
	hw_ctx->video_mode_format = 0xf;

	/*set up func_prg*/
	hw_ctx->dsi_func_prg = (0x200 | dsi_config->lane_count);
	/*setup mipi port configuration*/
	hw_ctx->mipi = PASS_FROM_SPHY_TO_AFE | dsi_config->lane_config;
}

static
int mdfld_dsi_otm1283A_auo_detect(struct mdfld_dsi_config *dsi_config)
{
	int status;
	struct drm_device *dev = dsi_config->dev;
	struct mdfld_dsi_hw_registers *regs = &dsi_config->regs;
	u32 dpll_val, device_ready_val;
	int pipe = dsi_config->pipe;

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

static int mdfld_dsi_otm1283A_auo_power_on(struct mdfld_dsi_config *dsi_config)
{
	struct mdfld_dsi_pkg_sender *sender =
		mdfld_dsi_get_pkg_sender(dsi_config);
	int err;

	PSB_DEBUG_ENTRY("\n");

	if (!sender) {
		DRM_ERROR("Failed to get DSI packet sender\n");
		return -EINVAL;
	}

	/*exit sleep */
	err = mdfld_dsi_send_mcs_short_lp(sender, 0x11, 0, 0, 0);
	if (err) {
		DRM_ERROR("faild to exit_sleep mode\n");
		goto power_err;
	}

	msleep(200);

	/*turn on display*/
	err = mdfld_dsi_send_mcs_short_lp(sender, 0x29, 0, 0, 0);

	if (err) {
		DRM_ERROR("faild to set_display_on mode\n");
		goto power_err;
	}
	msleep(50);


	/*send TURN_ON packet*/
	err = mdfld_dsi_send_dpi_spk_pkg_hs(sender,
				MDFLD_DSI_DPI_SPK_TURN_ON);
	if (err) {
		DRM_ERROR("Failed to send turn on packet\n");
		return err;
	}
power_err:
	return 0;
}

static int mdfld_dsi_otm1283A_auo_power_off(struct mdfld_dsi_config *dsi_config)
{
	struct mdfld_dsi_pkg_sender *sender =
		mdfld_dsi_get_pkg_sender(dsi_config);
	int err;

	PSB_DEBUG_ENTRY("\n");

	if (!sender) {
		DRM_ERROR("Failed to get DSI packet sender\n");
		return -EINVAL;
	}

	/*send SHUT_DOWN packet*/
	err = mdfld_dsi_send_dpi_spk_pkg_hs(sender,
				MDFLD_DSI_DPI_SPK_SHUT_DOWN);
	if (err) {
		DRM_ERROR("Failed to send turn off packet\n");
		return err;
	}

	/*turn off display */
	err = mdfld_dsi_send_mcs_short_lp(sender, 0x28, 0, 0, 0);
	if (err) {
		DRM_ERROR("sent set_display_off faild\n");
		goto out;
	}
	mdelay(50);

	/*Enter sleep mode */
	err = mdfld_dsi_send_mcs_short_lp(sender, 0x10, 0, 0, 0);

	if (err) {
		DRM_ERROR("DCS 0x%x sent failed\n", enter_sleep_mode);
		goto out;
	}
	mdelay(200);
out:
	return 0;
}

int mdfld_dsi_otm1283A_auo_set_brightness(struct mdfld_dsi_config *dsi_config,
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
	backlight_value = (level * 0xFE) /100;
	mdfld_dsi_send_mcs_short_lp(sender, 0x51, backlight_value, 1, 0);
#endif
	return 0;
}

static
int mdfld_dsi_otm1283A_auo_panel_reset(struct mdfld_dsi_config *dsi_config)
{
	static int mipi_reset_gpio;
	int ret = 0;

	if (mipi_reset_gpio == 0) {
		ret = get_gpio_by_name("mipi-reset");
		if (ret < 0) {
			DRM_ERROR("Faild to get panel reset gpio, " \
				  "use default reset pin\n");
			ret = 57;
		}

		mipi_reset_gpio = ret;

		PSB_DEBUG_ENTRY("mipi_reset_gpio = %d\n", mipi_reset_gpio);

		ret = gpio_request(mipi_reset_gpio, "mipi_display");
		if (ret) {
			DRM_ERROR("Faild to request panel reset gpio\n");
			return -EINVAL;
		}

		gpio_direction_output(mipi_reset_gpio, 0);
	}

	gpio_direction_output(mipi_reset_gpio, 1);
	mdelay(5);

	gpio_set_value_cansleep(mipi_reset_gpio, 0);
	mdelay(20);

	gpio_set_value_cansleep(mipi_reset_gpio, 1);
	mdelay(20);

	return 0;
}

static
struct drm_display_mode *otm1283A_auo_get_config_mode(void)
{
	struct drm_display_mode *mode;

	PSB_DEBUG_ENTRY("\n");

	mode = kzalloc(sizeof(*mode), GFP_KERNEL);
	if (!mode)
		return NULL;

	mode->hdisplay = 720;
	mode->hsync_start = mode->hdisplay + 160; //HFP
	mode->hsync_end = mode->hsync_start + 8; //HSYNC
	mode->htotal = mode->hsync_end + 160; //HBP
	mode->vdisplay = 1280;
	mode->vsync_start = mode->vdisplay + 32; //VFP
	mode->vsync_end = mode->vsync_start + 2; //VSYNC
	mode->vtotal = mode->vsync_end + 32; //VBP
	mode->vrefresh = 60;
	mode->clock =  mode->vrefresh * mode->vtotal *
		mode->htotal / 1000;	

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

#define PWM0CLKDIV0   0x62
#define PWM0CLKDIV1   0x61
#define SYSTEMCLK        196608
#define PWM_FREQUENCY  40000

static inline u16 calc_clkdiv(unsigned long baseclk, unsigned int f)
{
	return (baseclk - f) / f;
}


static void otm1283A_auo_brightness_init(struct drm_device *dev)
{
	int ret;
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
void otm1283A_auo_get_panel_info(int pipe, struct panel_info *pi)
{
	pi->width_mm = PANEL_4DOT3_WIDTH;
	pi->height_mm = PANEL_4DOT3_HEIGHT;
}

void otm1283_auo_vid_init(struct drm_device *dev, struct panel_funcs *p_funcs)
{
	PSB_DEBUG_ENTRY("\n");

	p_funcs->get_config_mode = otm1283A_auo_get_config_mode;
	p_funcs->get_panel_info = otm1283A_auo_get_panel_info;
	p_funcs->reset = mdfld_dsi_otm1283A_auo_panel_reset;
	p_funcs->drv_ic_init = mdfld_otm1283A_auo_dpi_ic_init;
	p_funcs->dsi_controller_init = mdfld_otm1283A_auo_dpi_controller_init;
	p_funcs->detect = mdfld_dsi_otm1283A_auo_detect;
	p_funcs->power_on = mdfld_dsi_otm1283A_auo_power_on;
	p_funcs->power_off = mdfld_dsi_otm1283A_auo_power_off;
	p_funcs->set_brightness = mdfld_dsi_otm1283A_auo_set_brightness;
#ifdef CONFIG_BL_PMU_PWM_CONTROL
	otm1283A_auo_brightness_init(dev);
#endif
}

static int otm1283A_auo_vid_probe(struct platform_device *pdev)
{
	DRM_INFO("%s: OTM1281 panel detected\n", __func__);
	intel_mid_panel_register(otm1283_auo_vid_init);

	return 0;
}

static struct platform_driver otm1283A_auo_lcd_driver = {
	.probe	= otm1283A_auo_vid_probe,
	.driver	= {
		.name	= "OTM1283AUO VID R",
		.owner	= THIS_MODULE,
	},
};

static int __init otm1283A_auo_lcd_init(void)
{
	DRM_INFO("%s\n", __func__);

	return platform_driver_register(&otm1283A_auo_lcd_driver);
}
module_init(otm1283A_auo_lcd_init);

