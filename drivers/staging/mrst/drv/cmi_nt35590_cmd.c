/*
 * Copyright ÃÂÃÂ© 2010 Intel Corporation
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
#include "mdfld_dsi_esd.h"
#include <linux/gpio.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/intel_pmic.h>
#include <linux/regulator/machine.h>
#include <asm/intel_scu_pmic.h>

#include "zte_pannel_common.h"

static u8 nt35590_para1[] = {0xFF, 0x00};
static u8 nt35590_para2[] = {0xBA, 0x03};
static u8 nt35590_para3[] = {0xC2, 0x08};
static u8 nt35590_para4[] = {0xFF, 0x01};
static u8 nt35590_para5[] = {0x00, 0x3A};
static u8 nt35590_para6[] = {0x01, 0x33};
static u8 nt35590_para7[] = {0x02, 0x53};
static u8 nt35590_para8[] = {0x09, 0x85};
static u8 nt35590_para9[] = {0x0E, 0x25};
static u8 nt35590_para10[] = {0x0F, 0x0A};
static u8 nt35590_para11[] = {0x0B, 0x97};
static u8 nt35590_para12[] = {0x0C, 0x97};
static u8 nt35590_para13[] = {0x11, 0x86};
static u8 nt35590_para14[] = {0x12, 0x03};
static u8 nt35590_para15[] = {0x36, 0x7B};
static u8 nt35590_para16[] = {0xB0, 0x80};
static u8 nt35590_para17[] = {0xB1, 0x02};
static u8 nt35590_para18[] = {0x71, 0x2C};
static u8 nt35590_para19[] = {0xFF, 0x05};
static u8 nt35590_para20[] = {0x01, 0x00};
static u8 nt35590_para21[] = {0x02, 0x8B};
static u8 nt35590_para22[] = {0x03, 0x8B};
static u8 nt35590_para23[] = {0x04, 0x8B};
static u8 nt35590_para24[] = {0x05, 0x30};
static u8 nt35590_para25[] = {0x06, 0x33};
static u8 nt35590_para26[] = {0x07, 0x77};
static u8 nt35590_para27[] = {0x08, 0x00};
static u8 nt35590_para28[] = {0x09, 0x00};
static u8 nt35590_para29[] = {0x0A, 0x00};
static u8 nt35590_para30[] = {0x0B, 0x80};
static u8 nt35590_para31[] = {0x0C, 0xC8};
static u8 nt35590_para32[] = {0x0D, 0x00};
static u8 nt35590_para33[] = {0x0E, 0x1B};
static u8 nt35590_para34[] = {0x0F, 0x07};
static u8 nt35590_para35[] = {0x10, 0x57};
static u8 nt35590_para36[] = {0x11, 0x00};
static u8 nt35590_para37[] = {0x12, 0x00};
static u8 nt35590_para38[] = {0x13, 0x1E};
static u8 nt35590_para39[] = {0x14, 0x00};
static u8 nt35590_para40[] = {0x15, 0x1A};
static u8 nt35590_para41[] = {0x16, 0x05};
static u8 nt35590_para42[] = {0x17, 0x00};
static u8 nt35590_para43[] = {0x18, 0x1E};
static u8 nt35590_para44[] = {0x19, 0xFF};
static u8 nt35590_para45[] = {0x1A, 0x00};
static u8 nt35590_para46[] = {0x1B, 0xFC};
static u8 nt35590_para47[] = {0x1C, 0x80};
static u8 nt35590_para48[] = {0x1D, 0x00};
static u8 nt35590_para49[] = {0x1E, 0x10};
static u8 nt35590_para50[] = {0x1F, 0x77};
static u8 nt35590_para51[] = {0x20, 0x00};
static u8 nt35590_para52[] = {0x21, 0x00};
static u8 nt35590_para53[] = {0x22, 0x55};
static u8 nt35590_para54[] = {0x23, 0x0D};
static u8 nt35590_para55[] = {0x31, 0xA0};
static u8 nt35590_para56[] = {0x32, 0x00};
static u8 nt35590_para57[] = {0x33, 0xB8};
static u8 nt35590_para58[] = {0x34, 0xBB};
static u8 nt35590_para59[] = {0x35, 0x11};
static u8 nt35590_para60[] = {0x36, 0x01};
static u8 nt35590_para61[] = {0x37, 0x0B};
static u8 nt35590_para62[] = {0x38, 0x01};
static u8 nt35590_para63[] = {0x39, 0x0B};
static u8 nt35590_para64[] = {0x44, 0x08};
static u8 nt35590_para65[] = {0x45, 0x80};
static u8 nt35590_para66[] = {0x46, 0xCC};
static u8 nt35590_para67[] = {0x47, 0x04};
static u8 nt35590_para68[] = {0x48, 0x00};
static u8 nt35590_para69[] = {0x49, 0x00};
static u8 nt35590_para70[] = {0x4A, 0x01};
static u8 nt35590_para71[] = {0x6C, 0x03};
static u8 nt35590_para72[] = {0x6D, 0x03};
static u8 nt35590_para73[] = {0x6E, 0x2F};
static u8 nt35590_para74[] = {0x43, 0x00};
static u8 nt35590_para75[] = {0x4B, 0x23};
static u8 nt35590_para76[] = {0x4C, 0x01};
static u8 nt35590_para77[] = {0x50, 0x23};
static u8 nt35590_para78[] = {0x51, 0x01};
static u8 nt35590_para79[] = {0x58, 0x23};
static u8 nt35590_para80[] = {0x59, 0x01};
static u8 nt35590_para81[] = {0x5D, 0x23};
static u8 nt35590_para82[] = {0x5E, 0x01};
static u8 nt35590_para83[] = {0x62, 0x23};
static u8 nt35590_para84[] = {0x63, 0x01};
static u8 nt35590_para85[] = {0x67, 0x23};
static u8 nt35590_para86[] = {0x68, 0x01};
static u8 nt35590_para87[] = {0x89, 0x00};
static u8 nt35590_para88[] = {0x8D, 0x01};
static u8 nt35590_para89[] = {0x8E, 0x64};
static u8 nt35590_para90[] = {0x8F, 0x20};
static u8 nt35590_para91[] = {0x97, 0x8E};
static u8 nt35590_para92[] = {0x82, 0x8C};
static u8 nt35590_para93[] = {0x83, 0x02};
static u8 nt35590_para94[] = {0xBB, 0x0A};
static u8 nt35590_para95[] = {0xBC, 0x0A};
static u8 nt35590_para96[] = {0x24, 0x25};
static u8 nt35590_para97[] = {0x25, 0x55};
static u8 nt35590_para98[] = {0x26, 0x05};
static u8 nt35590_para99[] = {0x27, 0x23};
static u8 nt35590_para100[] = {0x28, 0x01};
static u8 nt35590_para101[] = {0x29, 0x31};
static u8 nt35590_para102[] = {0x2A, 0x5D};
static u8 nt35590_para103[] = {0x2B, 0x01};
static u8 nt35590_para104[] = {0x2F, 0x00};
static u8 nt35590_para105[] = {0x30, 0x10};
static u8 nt35590_para106[] = {0xA7, 0x12};
static u8 nt35590_para107[] = {0x2D, 0x03};
static u8 nt35590_para108[] = {0xFF, 0x00};
static u8 nt35590_para109[] = {0xFB, 0x01};
static u8 nt35590_para110[] = {0xFF, 0x01};
static u8 nt35590_para111[] = {0xFB, 0x01};
static u8 nt35590_para112[] = {0xFF, 0x02};
static u8 nt35590_para113[] = {0xFB, 0x01};
static u8 nt35590_para114[] = {0xFF, 0x03};
static u8 nt35590_para115[] = {0xFB, 0x01};
static u8 nt35590_para116[] = {0xFF, 0x04};
static u8 nt35590_para117[] = {0xFB, 0x01};
static u8 nt35590_para118[] = {0xFF, 0x05};
static u8 nt35590_para119[] = {0xFB, 0x01};
static u8 nt35590_para120[] = {0xFF, 0x00};

static u8 nt35590_ce_para1[]={0xFF,0x03};
static u8 nt35590_ce_para2[]={0xFE,0x08};
static u8 nt35590_ce_para3[]={0x18,0x00};
static u8 nt35590_ce_para4[]={0x19,0x00};
static u8 nt35590_ce_para5[]={0x1A,0x00};
static u8 nt35590_ce_para6[]={0x25,0x26};
static u8 nt35590_ce_para7[]={0x00,0x00};
static u8 nt35590_ce_para8[]={0x01,0x08};
static u8 nt35590_ce_para9[]={0x02,0x0C};
static u8 nt35590_ce_para10[]={0x03,0x10};
static u8 nt35590_ce_para11[]={0x04,0x14};
static u8 nt35590_ce_para12[]={0x05,0x18};
static u8 nt35590_ce_para13[]={0x06,0x1C};
static u8 nt35590_ce_para14[]={0x07,0x20};
static u8 nt35590_ce_para15[]={0x08,0x24};
static u8 nt35590_ce_para16[]={0x09,0x28};
static u8 nt35590_ce_para17[]={0x0A,0x2C};
static u8 nt35590_ce_para18[]={0x0B,0x30};
static u8 nt35590_ce_para19[]={0x0C,0x34};
static u8 nt35590_ce_para20[]={0x0D,0x38};
static u8 nt35590_ce_para21[]={0x0E,0x3C};
static u8 nt35590_ce_para22[]={0x0F,0x3F};
static u8 nt35590_ce_para23[]={0xFB,0x01};
static u8 nt35590_ce_para24[]={0xFF,0x00};
static u8 nt35590_ce_para25[]={0xFE,0x01};

#define PANEL_NAME "Lead/Truly/Success/CPT/CMI CMI"

static
int mdfld_nt35590_drv_ic_init(struct mdfld_dsi_config *dsi_config)
{
	struct drm_device *dev = dsi_config->dev;
	struct mdfld_dsi_pkg_sender *sender
			= mdfld_dsi_get_pkg_sender(dsi_config);

	if (!sender)
		return -EINVAL;

	PSB_DEBUG_ENTRY("\n");

	sender->status = MDFLD_DSI_PKG_SENDER_FREE;

	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para1[0], nt35590_para1[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para2[0], nt35590_para2[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para3[0], nt35590_para3[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para4[0], nt35590_para4[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para5[0], nt35590_para5[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para6[0], nt35590_para6[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para7[0], nt35590_para7[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para8[0], nt35590_para8[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para9[0], nt35590_para9[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para10[0], nt35590_para10[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para11[0], nt35590_para11[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para12[0], nt35590_para12[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para13[0], nt35590_para13[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para14[0], nt35590_para14[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para15[0], nt35590_para15[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para16[0], nt35590_para16[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para17[0], nt35590_para17[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para18[0], nt35590_para18[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para19[0], nt35590_para19[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para20[0], nt35590_para20[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para21[0], nt35590_para21[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para22[0], nt35590_para22[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para23[0], nt35590_para23[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para24[0], nt35590_para24[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para25[0], nt35590_para25[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para26[0], nt35590_para26[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para27[0], nt35590_para27[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para28[0], nt35590_para28[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para29[0], nt35590_para29[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para30[0], nt35590_para30[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para31[0], nt35590_para31[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para32[0], nt35590_para32[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para33[0], nt35590_para33[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para34[0], nt35590_para34[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para35[0], nt35590_para35[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para36[0], nt35590_para36[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para37[0], nt35590_para37[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para38[0], nt35590_para38[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para39[0], nt35590_para39[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para40[0], nt35590_para40[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para41[0], nt35590_para41[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para42[0], nt35590_para42[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para43[0], nt35590_para43[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para44[0], nt35590_para44[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para45[0], nt35590_para45[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para46[0], nt35590_para46[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para47[0], nt35590_para47[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para48[0], nt35590_para48[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para49[0], nt35590_para49[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para50[0], nt35590_para50[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para51[0], nt35590_para51[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para52[0], nt35590_para52[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para53[0], nt35590_para53[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para54[0], nt35590_para54[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para55[0], nt35590_para55[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para56[0], nt35590_para56[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para57[0], nt35590_para57[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para58[0], nt35590_para58[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para59[0], nt35590_para59[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para60[0], nt35590_para60[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para61[0], nt35590_para61[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para62[0], nt35590_para62[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para63[0], nt35590_para63[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para64[0], nt35590_para64[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para65[0], nt35590_para65[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para66[0], nt35590_para66[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para67[0], nt35590_para67[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para68[0], nt35590_para68[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para69[0], nt35590_para69[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para70[0], nt35590_para70[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para71[0], nt35590_para71[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para72[0], nt35590_para72[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para73[0], nt35590_para73[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para74[0], nt35590_para74[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para75[0], nt35590_para75[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para76[0], nt35590_para76[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para77[0], nt35590_para77[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para78[0], nt35590_para78[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para79[0], nt35590_para79[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para80[0], nt35590_para80[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para81[0], nt35590_para81[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para82[0], nt35590_para82[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para83[0], nt35590_para83[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para84[0], nt35590_para84[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para85[0], nt35590_para85[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para86[0], nt35590_para86[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para87[0], nt35590_para87[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para88[0], nt35590_para88[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para89[0], nt35590_para89[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para90[0], nt35590_para90[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para91[0], nt35590_para91[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para92[0], nt35590_para92[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para93[0], nt35590_para93[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para94[0], nt35590_para94[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para95[0], nt35590_para95[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para96[0], nt35590_para96[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para97[0], nt35590_para97[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para98[0], nt35590_para98[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para99[0], nt35590_para99[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para100[0], nt35590_para100[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para101[0], nt35590_para101[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para102[0], nt35590_para102[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para103[0], nt35590_para103[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para104[0], nt35590_para104[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para105[0], nt35590_para105[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para106[0], nt35590_para106[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para107[0], nt35590_para107[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para108[0], nt35590_para108[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para109[0], nt35590_para109[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para110[0], nt35590_para110[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para111[0], nt35590_para111[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para112[0], nt35590_para112[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para113[0], nt35590_para113[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para114[0], nt35590_para114[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para115[0], nt35590_para115[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para116[0], nt35590_para116[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para117[0], nt35590_para117[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para118[0], nt35590_para118[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para119[0], nt35590_para119[1], 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, nt35590_para120[0], nt35590_para120[1], 1, 0);

	mdfld_dsi_send_mcs_short_lp(sender,nt35590_ce_para1[0],nt35590_ce_para1[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_ce_para2[0],nt35590_ce_para2[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_ce_para3[0],nt35590_ce_para3[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_ce_para4[0],nt35590_ce_para4[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_ce_para5[0],nt35590_ce_para5[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_ce_para6[0],nt35590_ce_para6[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_ce_para7[0],nt35590_ce_para7[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_ce_para8[0],nt35590_ce_para8[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_ce_para9[0],nt35590_ce_para9[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_ce_para10[0],nt35590_ce_para10[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_ce_para11[0],nt35590_ce_para11[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_ce_para12[0],nt35590_ce_para12[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_ce_para13[0],nt35590_ce_para13[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_ce_para14[0],nt35590_ce_para14[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_ce_para15[0],nt35590_ce_para15[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_ce_para16[0],nt35590_ce_para16[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_ce_para17[0],nt35590_ce_para17[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_ce_para18[0],nt35590_ce_para18[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_ce_para19[0],nt35590_ce_para19[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_ce_para20[0],nt35590_ce_para20[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_ce_para21[0],nt35590_ce_para21[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_ce_para22[0],nt35590_ce_para22[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_ce_para23[0],nt35590_ce_para23[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_ce_para24[0],nt35590_ce_para24[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_ce_para25[0],nt35590_ce_para25[1],1,0);

#ifdef CONFIG_BL_LCD_PWM_CONTROL
	mdfld_dsi_send_mcs_short_lp(sender, 0xFF, 0x04, 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, 0x0A, 0x02, 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, 0xFF, 0x00, 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, 0x55, 0x00, 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, 0x53, 0x24, 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, 0x51, 0x00, 1, 0);
#endif

	return 0;
}

static
void mdfld_nt35590_dsi_controller_init(struct mdfld_dsi_config *dsi_config)
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
	hw_ctx->turn_around_timeout = 0x18;//0x14;
	hw_ctx->device_reset_timer = 0xffff;
	hw_ctx->high_low_switch_count = 0x18;//0x20;
	hw_ctx->init_count = 0xf0;
	/* nt35590 request Tclk-post +150UI need enable clock stop */
	hw_ctx->eot_disable = 0x2;
	hw_ctx->lp_byteclk = 0x3;//0x4;
	hw_ctx->clk_lane_switch_time_cnt = 0x0018000B;//0x20000E;
	hw_ctx->hs_ls_dbi_enable = 0x0;
	/* HW team suggested 1390 for bandwidth setting */
	hw_ctx->dbi_bw_ctrl = 820;//1390;
	hw_ctx->dphy_param = 0x1f1f3610;//0x20124E1A;
	hw_ctx->dsi_func_prg = (0xa000 | dsi_config->lane_count);
	hw_ctx->mipi = TE_TRIGGER_GPIO_PIN;
	hw_ctx->mipi |= dsi_config->lane_config;

}

static
struct drm_display_mode *nt35590_cmd_get_config_mode(void)
{
	struct drm_display_mode *mode;

	PSB_DEBUG_ENTRY("\n");

	mode = kzalloc(sizeof(*mode), GFP_KERNEL);
	if (!mode)
		return NULL;

	mode->htotal = 920;
	mode->hdisplay = 720;
	mode->hsync_start = 816;
	mode->hsync_end = 824;
	mode->vtotal = 1300;
	mode->vdisplay = 1280;
	mode->vsync_start = 1294;
	mode->vsync_end = 1296;
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
int mdfld_dsi_nt35590_cmd_power_on(struct mdfld_dsi_config *dsi_config)
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

	/*exit sleep */
	err = mdfld_dsi_send_mcs_short_lp(sender, 0x11, 0, 0, 0);
	if (err) {
		DRM_ERROR("faild to exit_sleep mode\n");
		goto power_err;
	}

	msleep(120);

	/*set tear on*/
	err = mdfld_dsi_send_mcs_short_lp(sender, 0x35, 0x00, 1, 0);

	if (err) {
		DRM_ERROR("faild to set_tear_on mode\n");
		goto power_err;
	}

	/*turn on display*/
	err = mdfld_dsi_send_mcs_short_lp(sender, 0x29, 0, 0, 0);

	if (err) {
		DRM_ERROR("faild to set_display_on mode\n");
		goto power_err;
	}
	msleep(50);

power_err:
	return err;
}

static int mdfld_dsi_nt35590_cmd_power_off(struct mdfld_dsi_config *dsi_config)
{
	struct mdfld_dsi_pkg_sender *sender =
		mdfld_dsi_get_pkg_sender(dsi_config);
	int err = 0;

	PSB_DEBUG_ENTRY("\n");

	if (!sender) {
		DRM_ERROR("Failed to get DSI packet sender\n");
		return -EINVAL;
	}

	/*turn off display */
	err = mdfld_dsi_send_mcs_short_lp(sender, 0x28, 0, 0, 0);
	if (err) {
		DRM_ERROR("sent set_display_off faild\n");
		goto out;
	}
	mdelay(50);
	/*set tear off */
	err = mdfld_dsi_send_mcs_short_lp(sender, 0x34, 0, 1, 0);
	if (err) {
		DRM_ERROR("sent set_tear_off faild\n");
		goto out;
	}

	/*Enter sleep mode */
	err = mdfld_dsi_send_mcs_short_lp(sender, 0x10, 0, 0, 0);

	if (err) {
		DRM_ERROR("DCS 0x%x sent failed\n", enter_sleep_mode);
		goto out;
	}
	mdelay(200);
out:
	return err;
}

static
void nt35590_cmd_get_panel_info(int pipe, struct panel_info *pi)
{
	PSB_DEBUG_ENTRY("\n");

	if (pipe == 0) {
		pi->width_mm = PANEL_4DOT3_WIDTH;
		pi->height_mm = PANEL_4DOT3_HEIGHT;
	}
}

static
int mdfld_dsi_nt35590_cmd_detect(struct mdfld_dsi_config *dsi_config)
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
int mdfld_dsi_nt35590_cmd_set_brightness(struct mdfld_dsi_config *dsi_config,
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
	if (get_debug_flag()) {
		printk("[%s]brightness level:%d, pwm val:%d\n", __func__, level, backlight_value);
	}
	mdfld_dsi_send_mcs_short_lp(sender, 0x51, backlight_value, 1, 0);
#endif
	return 0;
}

static
int mdfld_dsi_nt35590_cmd_panel_reset(struct mdfld_dsi_config *dsi_config)
{
	static int mipi_reset_gpio;
	int ret = 0;

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
	return (baseclk - f) / f;
}


static void nt35590_brightness_init(struct drm_device *dev)
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

void nt35590_cmd_init(struct drm_device *dev, struct panel_funcs *p_funcs)
{
	int ena_err;

	if (!dev || !p_funcs) {
		DRM_ERROR("Invalid parameters\n");
		return;
	}

	PSB_DEBUG_ENTRY("\n");

	p_funcs->get_config_mode = nt35590_cmd_get_config_mode;
	p_funcs->get_panel_info = nt35590_cmd_get_panel_info;
	p_funcs->reset = mdfld_dsi_nt35590_cmd_panel_reset;
	p_funcs->drv_ic_init = mdfld_nt35590_drv_ic_init;
	p_funcs->dsi_controller_init = mdfld_nt35590_dsi_controller_init;
	p_funcs->detect = mdfld_dsi_nt35590_cmd_detect;
	p_funcs->power_on = mdfld_dsi_nt35590_cmd_power_on;
	p_funcs->power_off = mdfld_dsi_nt35590_cmd_power_off;
	p_funcs->set_brightness = mdfld_dsi_nt35590_cmd_set_brightness;

#ifdef CONFIG_BL_PMU_PWM_CONTROL
	nt35590_brightness_init(dev);
#endif
}

static int nt35590_cmd_probe(struct platform_device *pdev)
{
	int ret = 0;

	DRM_INFO("%s: NT35590 panel detected\n", __func__);
	intel_mid_panel_register(nt35590_cmd_init);

	add_panel_config_prop(PANEL_NAME, "NT35590", 1280, 720);
	create_backlight_debug_file();
	return 0;
}

static struct platform_driver nt35590_lcd_driver = {
	.probe	= nt35590_cmd_probe,
	.driver	= {
		.name	= "NT35590CMI CMD R",
		.owner	= THIS_MODULE,
	},
};

static int __init nt35590_lcd_init(void)
{
	DRM_INFO("%s\n", __func__);

	return platform_driver_register(&nt35590_lcd_driver);
}
module_init(nt35590_lcd_init);

