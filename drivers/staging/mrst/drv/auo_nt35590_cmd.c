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
#include "mdfld_dsi_esd.h"
#include <linux/gpio.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/intel_pmic.h>
#include <linux/regulator/machine.h>
#include <asm/intel_scu_pmic.h>

#include "zte_pannel_common.h"

static u8 NT35590_AUO_Param1[]={0xFF,0xEE};
static u8 NT35590_AUO_Param2[]={0x26,0x08};
static u8 NT35590_AUO_Param3[]={0x26,0x00};
static u8 NT35590_AUO_Param4[]={0xFF,0x00};
static u8 NT35590_AUO_Param5[]={0xFF,0x01};
static u8 NT35590_AUO_Param6[]={0xFB,0x01};
static u8 NT35590_AUO_Param7[]={0x00,0x4A};
static u8 NT35590_AUO_Param8[]={0x01,0x33};
static u8 NT35590_AUO_Param9[]={0x02,0x53};
static u8 NT35590_AUO_Param10[]={0x03,0x55};
static u8 NT35590_AUO_Param11[]={0x04,0x55};
static u8 NT35590_AUO_Param12[]={0x05,0x33};
static u8 NT35590_AUO_Param13[]={0x06,0x22};
static u8 NT35590_AUO_Param14[]={0x08,0x56};
static u8 NT35590_AUO_Param15[]={0x09,0x8F};
static u8 NT35590_AUO_Param16[]={0x0B,0xCF};
static u8 NT35590_AUO_Param17[]={0x0C,0xCF};
static u8 NT35590_AUO_Param18[]={0x0D,0x2F};
static u8 NT35590_AUO_Param19[]={0x0E,0x29};
static u8 NT35590_AUO_Param20[]={0x36,0x73};
static u8 NT35590_AUO_Param21[]={0x0F,0x0A};
static u8 NT35590_AUO_Param22[]={0xFF,0xEE};
static u8 NT35590_AUO_Param23[]={0xFB,0x01};
static u8 NT35590_AUO_Param24[]={0x12,0x50};
static u8 NT35590_AUO_Param25[]={0x13,0x02};
static u8 NT35590_AUO_Param26[]={0x6A,0x60};
static u8 NT35590_AUO_Param27[]={0xFF,0x05};
static u8 NT35590_AUO_Param28[]={0xFB,0x01};
static u8 NT35590_AUO_Param29[]={0x01,0x00};
static u8 NT35590_AUO_Param30[]={0x02,0x8B};
static u8 NT35590_AUO_Param31[]={0x03,0x8B};
static u8 NT35590_AUO_Param32[]={0x04,0x8B};
static u8 NT35590_AUO_Param33[]={0x06,0x33};
static u8 NT35590_AUO_Param34[]={0x07,0x01};
static u8 NT35590_AUO_Param35[]={0x08,0x00};
static u8 NT35590_AUO_Param36[]={0x09,0x46};
static u8 NT35590_AUO_Param37[]={0x0A,0x46};
static u8 NT35590_AUO_Param38[]={0x0D,0x0B};
static u8 NT35590_AUO_Param39[]={0x0E,0x1D};
static u8 NT35590_AUO_Param40[]={0x0F,0x08};
static u8 NT35590_AUO_Param41[]={0x10,0x53};
static u8 NT35590_AUO_Param42[]={0x11,0x00};
static u8 NT35590_AUO_Param43[]={0x12,0x00};
static u8 NT35590_AUO_Param44[]={0x14,0x01};
static u8 NT35590_AUO_Param45[]={0x15,0x00};
static u8 NT35590_AUO_Param46[]={0x16,0x05};
static u8 NT35590_AUO_Param47[]={0x17,0x00};
static u8 NT35590_AUO_Param48[]={0x19,0x7F};
static u8 NT35590_AUO_Param49[]={0x1A,0xFF};
static u8 NT35590_AUO_Param50[]={0x1B,0x0F};
static u8 NT35590_AUO_Param51[]={0x1C,0x00};
static u8 NT35590_AUO_Param52[]={0x1D,0x00};
static u8 NT35590_AUO_Param53[]={0x1E,0x00};
static u8 NT35590_AUO_Param54[]={0x1F,0x07};
static u8 NT35590_AUO_Param55[]={0x20,0x00};
static u8 NT35590_AUO_Param56[]={0x21,0x00};
static u8 NT35590_AUO_Param57[]={0x22,0x55};
static u8 NT35590_AUO_Param58[]={0x23,0x4D};
static u8 NT35590_AUO_Param59[]={0x2D,0x02};
static u8 NT35590_AUO_Param60[]={0x83,0x01};
static u8 NT35590_AUO_Param61[]={0x9E,0x58};
static u8 NT35590_AUO_Param62[]={0x9F,0x6A};
static u8 NT35590_AUO_Param63[]={0xA0,0x01};
static u8 NT35590_AUO_Param64[]={0xA2,0x10};
static u8 NT35590_AUO_Param65[]={0xBB,0x0A};
static u8 NT35590_AUO_Param66[]={0xBC,0x0A};
static u8 NT35590_AUO_Param67[]={0x28,0x01};
static u8 NT35590_AUO_Param68[]={0x2F,0x02};
static u8 NT35590_AUO_Param69[]={0x32,0x08};
static u8 NT35590_AUO_Param70[]={0x33,0xB8};
static u8 NT35590_AUO_Param71[]={0x36,0x01};
static u8 NT35590_AUO_Param72[]={0x37,0x00};
static u8 NT35590_AUO_Param73[]={0x43,0x00};
static u8 NT35590_AUO_Param74[]={0x4B,0x21};
static u8 NT35590_AUO_Param75[]={0x4C,0x03};
static u8 NT35590_AUO_Param76[]={0x50,0x21};
static u8 NT35590_AUO_Param77[]={0x51,0x03};
static u8 NT35590_AUO_Param78[]={0x58,0x21};
static u8 NT35590_AUO_Param79[]={0x59,0x03};
static u8 NT35590_AUO_Param80[]={0x5D,0x21};
static u8 NT35590_AUO_Param81[]={0x5E,0x03};
static u8 NT35590_AUO_Param82[]={0x6C,0x00};
static u8 NT35590_AUO_Param83[]={0x6D,0x00};
static u8 NT35590_AUO_Param84[]={0xFF,0x00};
static u8 NT35590_AUO_Param85[]={0xFB,0x01};
static u8 NT35590_AUO_Param86[]={0xBA,0x03};
static u8 NT35590_AUO_Param87[]={0xC2,0x08};
static u8 NT35590_AUO_Param88[]={0xFF,0x01};
static u8 NT35590_AUO_Param89[]={0xFB,0x01};
static u8 NT35590_AUO_Param90[]={0x75,0x00};
static u8 NT35590_AUO_Param91[]={0x76,0x98};
static u8 NT35590_AUO_Param92[]={0x77,0x00};
static u8 NT35590_AUO_Param93[]={0x78,0xAF};
static u8 NT35590_AUO_Param94[]={0x79,0x00};
static u8 NT35590_AUO_Param95[]={0x7A,0xD1};
static u8 NT35590_AUO_Param96[]={0x7B,0x00};
static u8 NT35590_AUO_Param97[]={0x7C,0xE9};
static u8 NT35590_AUO_Param98[]={0x7D,0x00};
static u8 NT35590_AUO_Param99[]={0x7E,0xFE};
static u8 NT35590_AUO_Param100[]={0x7F,0x01};
static u8 NT35590_AUO_Param101[]={0x80,0x10};
static u8 NT35590_AUO_Param102[]={0x81,0x01};
static u8 NT35590_AUO_Param103[]={0x82,0x20};
static u8 NT35590_AUO_Param104[]={0x83,0x01};
static u8 NT35590_AUO_Param105[]={0x84,0x2E};
static u8 NT35590_AUO_Param106[]={0x85,0x01};
static u8 NT35590_AUO_Param107[]={0x86,0x3B};
static u8 NT35590_AUO_Param108[]={0x87,0x01};
static u8 NT35590_AUO_Param109[]={0x88,0x65};
static u8 NT35590_AUO_Param110[]={0x89,0x01};
static u8 NT35590_AUO_Param111[]={0x8A,0x88};
static u8 NT35590_AUO_Param112[]={0x8B,0x01};
static u8 NT35590_AUO_Param113[]={0x8C,0xBD};
static u8 NT35590_AUO_Param114[]={0x8D,0x01};
static u8 NT35590_AUO_Param115[]={0x8E,0xE7};
static u8 NT35590_AUO_Param116[]={0x8F,0x02};
static u8 NT35590_AUO_Param117[]={0x90,0x27};
static u8 NT35590_AUO_Param118[]={0x91,0x02};
static u8 NT35590_AUO_Param119[]={0x92,0x59};
static u8 NT35590_AUO_Param120[]={0x93,0x02};
static u8 NT35590_AUO_Param121[]={0x94,0x5B};
static u8 NT35590_AUO_Param122[]={0x95,0x02};
static u8 NT35590_AUO_Param123[]={0x96,0x87};
static u8 NT35590_AUO_Param124[]={0x97,0x02};
static u8 NT35590_AUO_Param125[]={0x98,0xB6};
static u8 NT35590_AUO_Param126[]={0x99,0x02};
static u8 NT35590_AUO_Param127[]={0x9A,0xD5};
static u8 NT35590_AUO_Param128[]={0x9B,0x02};
static u8 NT35590_AUO_Param129[]={0x9C,0xFD};
static u8 NT35590_AUO_Param130[]={0x9D,0x03};
static u8 NT35590_AUO_Param131[]={0x9E,0x19};
static u8 NT35590_AUO_Param132[]={0x9F,0x03};
static u8 NT35590_AUO_Param133[]={0xA0,0x40};
static u8 NT35590_AUO_Param134[]={0xA2,0x03};
static u8 NT35590_AUO_Param135[]={0xA3,0x4C};
static u8 NT35590_AUO_Param136[]={0xA4,0x03};
static u8 NT35590_AUO_Param137[]={0xA5,0x59};
static u8 NT35590_AUO_Param138[]={0xA6,0x03};
static u8 NT35590_AUO_Param139[]={0xA7,0x67};
static u8 NT35590_AUO_Param140[]={0xA9,0x03};
static u8 NT35590_AUO_Param141[]={0xAA,0x78};
static u8 NT35590_AUO_Param142[]={0xAB,0x03};
static u8 NT35590_AUO_Param143[]={0xAC,0x8A};
static u8 NT35590_AUO_Param144[]={0xAD,0x03};
static u8 NT35590_AUO_Param145[]={0xAE,0xA8};
static u8 NT35590_AUO_Param146[]={0xAF,0x03};
static u8 NT35590_AUO_Param147[]={0xB0,0xB8};
static u8 NT35590_AUO_Param148[]={0xB1,0x03};
static u8 NT35590_AUO_Param149[]={0xB2,0xBE};
static u8 NT35590_AUO_Param150[]={0xB3,0x00};
static u8 NT35590_AUO_Param151[]={0xB4,0x98};
static u8 NT35590_AUO_Param152[]={0xB5,0x00};
static u8 NT35590_AUO_Param153[]={0xB6,0xAF};
static u8 NT35590_AUO_Param154[]={0xB7,0x00};
static u8 NT35590_AUO_Param155[]={0xB8,0xD1};
static u8 NT35590_AUO_Param156[]={0xB9,0x00};
static u8 NT35590_AUO_Param157[]={0xBA,0xE9};
static u8 NT35590_AUO_Param158[]={0xBB,0x00};
static u8 NT35590_AUO_Param159[]={0xBC,0xFE};
static u8 NT35590_AUO_Param160[]={0xBD,0x01};
static u8 NT35590_AUO_Param161[]={0xBE,0x10};
static u8 NT35590_AUO_Param162[]={0xBF,0x01};
static u8 NT35590_AUO_Param163[]={0xC0,0x20};
static u8 NT35590_AUO_Param164[]={0xC1,0x01};
static u8 NT35590_AUO_Param165[]={0xC2,0x2E};
static u8 NT35590_AUO_Param166[]={0xC3,0x01};
static u8 NT35590_AUO_Param167[]={0xC4,0x3B};
static u8 NT35590_AUO_Param168[]={0xC5,0x01};
static u8 NT35590_AUO_Param169[]={0xC6,0x65};
static u8 NT35590_AUO_Param170[]={0xC7,0x01};
static u8 NT35590_AUO_Param171[]={0xC8,0x88};
static u8 NT35590_AUO_Param172[]={0xC9,0x01};
static u8 NT35590_AUO_Param173[]={0xCA,0xBD};
static u8 NT35590_AUO_Param174[]={0xCB,0x01};
static u8 NT35590_AUO_Param175[]={0xCC,0xE7};
static u8 NT35590_AUO_Param176[]={0xCD,0x02};
static u8 NT35590_AUO_Param177[]={0xCE,0x27};
static u8 NT35590_AUO_Param178[]={0xCF,0x02};
static u8 NT35590_AUO_Param179[]={0xD0,0x59};
static u8 NT35590_AUO_Param180[]={0xD1,0x02};
static u8 NT35590_AUO_Param181[]={0xD2,0x5B};
static u8 NT35590_AUO_Param182[]={0xD3,0x02};
static u8 NT35590_AUO_Param183[]={0xD4,0x87};
static u8 NT35590_AUO_Param184[]={0xD5,0x02};
static u8 NT35590_AUO_Param185[]={0xD6,0xB6};
static u8 NT35590_AUO_Param186[]={0xD7,0x02};
static u8 NT35590_AUO_Param187[]={0xD8,0xD5};
static u8 NT35590_AUO_Param188[]={0xD9,0x02};
static u8 NT35590_AUO_Param189[]={0xDA,0xFD};
static u8 NT35590_AUO_Param190[]={0xDB,0x03};
static u8 NT35590_AUO_Param191[]={0xDC,0x19};
static u8 NT35590_AUO_Param192[]={0xDD,0x03};
static u8 NT35590_AUO_Param193[]={0xDE,0x40};
static u8 NT35590_AUO_Param194[]={0xDF,0x03};
static u8 NT35590_AUO_Param195[]={0xE0,0x4C};
static u8 NT35590_AUO_Param196[]={0xE1,0x03};
static u8 NT35590_AUO_Param197[]={0xE2,0x59};
static u8 NT35590_AUO_Param198[]={0xE3,0x03};
static u8 NT35590_AUO_Param199[]={0xE4,0x67};
static u8 NT35590_AUO_Param200[]={0xE5,0x03};
static u8 NT35590_AUO_Param201[]={0xE6,0x78};
static u8 NT35590_AUO_Param202[]={0xE7,0x03};
static u8 NT35590_AUO_Param203[]={0xE8,0x8A};
static u8 NT35590_AUO_Param204[]={0xE9,0x03};
static u8 NT35590_AUO_Param205[]={0xEA,0xA8};
static u8 NT35590_AUO_Param206[]={0xEB,0x03};
static u8 NT35590_AUO_Param207[]={0xEC,0xB8};
static u8 NT35590_AUO_Param208[]={0xED,0x03};
static u8 NT35590_AUO_Param209[]={0xEE,0xBE};
static u8 NT35590_AUO_Param210[]={0xEF,0x00};
static u8 NT35590_AUO_Param211[]={0xF0,0x98};
static u8 NT35590_AUO_Param212[]={0xF1,0x00};
static u8 NT35590_AUO_Param213[]={0xF2,0xAF};
static u8 NT35590_AUO_Param214[]={0xF3,0x00};
static u8 NT35590_AUO_Param215[]={0xF4,0xD1};
static u8 NT35590_AUO_Param216[]={0xF5,0x00};
static u8 NT35590_AUO_Param217[]={0xF6,0xE9};
static u8 NT35590_AUO_Param218[]={0xF7,0x00};
static u8 NT35590_AUO_Param219[]={0xF8,0xFE};
static u8 NT35590_AUO_Param220[]={0xF9,0x01};
static u8 NT35590_AUO_Param221[]={0xFA,0x10};
static u8 NT35590_AUO_Param222[]={0xFF,0x02};
static u8 NT35590_AUO_Param223[]={0xFB,0x01};
static u8 NT35590_AUO_Param224[]={0x00,0x01};
static u8 NT35590_AUO_Param225[]={0x01,0x20};
static u8 NT35590_AUO_Param226[]={0x02,0x01};
static u8 NT35590_AUO_Param227[]={0x03,0x2E};
static u8 NT35590_AUO_Param228[]={0x04,0x01};
static u8 NT35590_AUO_Param229[]={0x05,0x3B};
static u8 NT35590_AUO_Param230[]={0x06,0x01};
static u8 NT35590_AUO_Param231[]={0x07,0x65};
static u8 NT35590_AUO_Param232[]={0x08,0x01};
static u8 NT35590_AUO_Param233[]={0x09,0x88};
static u8 NT35590_AUO_Param234[]={0x0A,0x01};
static u8 NT35590_AUO_Param235[]={0x0B,0xBD};
static u8 NT35590_AUO_Param236[]={0x0C,0x01};
static u8 NT35590_AUO_Param237[]={0x0D,0xE7};
static u8 NT35590_AUO_Param238[]={0x0E,0x02};
static u8 NT35590_AUO_Param239[]={0x0F,0x27};
static u8 NT35590_AUO_Param240[]={0x10,0x02};
static u8 NT35590_AUO_Param241[]={0x11,0x59};
static u8 NT35590_AUO_Param242[]={0x12,0x02};
static u8 NT35590_AUO_Param243[]={0x13,0x5B};
static u8 NT35590_AUO_Param244[]={0x14,0x02};
static u8 NT35590_AUO_Param245[]={0x15,0x87};
static u8 NT35590_AUO_Param246[]={0x16,0x02};
static u8 NT35590_AUO_Param247[]={0x17,0xB6};
static u8 NT35590_AUO_Param248[]={0x18,0x02};
static u8 NT35590_AUO_Param249[]={0x19,0xD5};
static u8 NT35590_AUO_Param250[]={0x1A,0x02};
static u8 NT35590_AUO_Param251[]={0x1B,0xFD};
static u8 NT35590_AUO_Param252[]={0x1C,0x03};
static u8 NT35590_AUO_Param253[]={0x1D,0x19};
static u8 NT35590_AUO_Param254[]={0x1E,0x03};
static u8 NT35590_AUO_Param255[]={0x1F,0x40};
static u8 NT35590_AUO_Param256[]={0x20,0x03};
static u8 NT35590_AUO_Param257[]={0x21,0x4C};
static u8 NT35590_AUO_Param258[]={0x22,0x03};
static u8 NT35590_AUO_Param259[]={0x23,0x59};
static u8 NT35590_AUO_Param260[]={0x24,0x03};
static u8 NT35590_AUO_Param261[]={0x25,0x67};
static u8 NT35590_AUO_Param262[]={0x26,0x03};
static u8 NT35590_AUO_Param263[]={0x27,0x78};
static u8 NT35590_AUO_Param264[]={0x28,0x03};
static u8 NT35590_AUO_Param265[]={0x29,0x8A};
static u8 NT35590_AUO_Param266[]={0x2A,0x03};
static u8 NT35590_AUO_Param267[]={0x2B,0xA8};
static u8 NT35590_AUO_Param268[]={0x2D,0x03};
static u8 NT35590_AUO_Param269[]={0x2F,0xB8};
static u8 NT35590_AUO_Param270[]={0x30,0x03};
static u8 NT35590_AUO_Param271[]={0x31,0xBE};
static u8 NT35590_AUO_Param272[]={0x32,0x00};
static u8 NT35590_AUO_Param273[]={0x33,0x98};
static u8 NT35590_AUO_Param274[]={0x34,0x00};
static u8 NT35590_AUO_Param275[]={0x35,0xAF};
static u8 NT35590_AUO_Param276[]={0x36,0x00};
static u8 NT35590_AUO_Param277[]={0x37,0xD1};
static u8 NT35590_AUO_Param278[]={0x38,0x00};
static u8 NT35590_AUO_Param279[]={0x39,0xE9};
static u8 NT35590_AUO_Param280[]={0x3A,0x00};
static u8 NT35590_AUO_Param281[]={0x3B,0xFE};
static u8 NT35590_AUO_Param282[]={0x3D,0x01};
static u8 NT35590_AUO_Param283[]={0x3F,0x10};
static u8 NT35590_AUO_Param284[]={0x40,0x01};
static u8 NT35590_AUO_Param285[]={0x41,0x20};
static u8 NT35590_AUO_Param286[]={0x42,0x01};
static u8 NT35590_AUO_Param287[]={0x43,0x2E};
static u8 NT35590_AUO_Param288[]={0x44,0x01};
static u8 NT35590_AUO_Param289[]={0x45,0x3B};
static u8 NT35590_AUO_Param290[]={0x46,0x01};
static u8 NT35590_AUO_Param291[]={0x47,0x65};
static u8 NT35590_AUO_Param292[]={0x48,0x01};
static u8 NT35590_AUO_Param293[]={0x49,0x88};
static u8 NT35590_AUO_Param294[]={0x4A,0x01};
static u8 NT35590_AUO_Param295[]={0x4B,0xBD};
static u8 NT35590_AUO_Param296[]={0x4C,0x01};
static u8 NT35590_AUO_Param297[]={0x4D,0xE7};
static u8 NT35590_AUO_Param298[]={0x4E,0x02};
static u8 NT35590_AUO_Param299[]={0x4F,0x27};
static u8 NT35590_AUO_Param300[]={0x50,0x02};
static u8 NT35590_AUO_Param301[]={0x51,0x59};
static u8 NT35590_AUO_Param302[]={0x52,0x02};
static u8 NT35590_AUO_Param303[]={0x53,0x5B};
static u8 NT35590_AUO_Param304[]={0x54,0x02};
static u8 NT35590_AUO_Param305[]={0x55,0x87};
static u8 NT35590_AUO_Param306[]={0x56,0x02};
static u8 NT35590_AUO_Param307[]={0x58,0xB6};
static u8 NT35590_AUO_Param308[]={0x59,0x02};
static u8 NT35590_AUO_Param309[]={0x5A,0xD5};
static u8 NT35590_AUO_Param310[]={0x5B,0x02};
static u8 NT35590_AUO_Param311[]={0x5C,0xFD};
static u8 NT35590_AUO_Param312[]={0x5D,0x03};
static u8 NT35590_AUO_Param313[]={0x5E,0x19};
static u8 NT35590_AUO_Param314[]={0x5F,0x03};
static u8 NT35590_AUO_Param315[]={0x60,0x40};
static u8 NT35590_AUO_Param316[]={0x61,0x03};
static u8 NT35590_AUO_Param317[]={0x62,0x4C};
static u8 NT35590_AUO_Param318[]={0x63,0x03};
static u8 NT35590_AUO_Param319[]={0x64,0x59};
static u8 NT35590_AUO_Param320[]={0x65,0x03};
static u8 NT35590_AUO_Param321[]={0x66,0x67};
static u8 NT35590_AUO_Param322[]={0x67,0x03};
static u8 NT35590_AUO_Param323[]={0x68,0x78};
static u8 NT35590_AUO_Param324[]={0x69,0x03};
static u8 NT35590_AUO_Param325[]={0x6A,0x8A};
static u8 NT35590_AUO_Param326[]={0x6B,0x03};
static u8 NT35590_AUO_Param327[]={0x6C,0xA8};
static u8 NT35590_AUO_Param328[]={0x6D,0x03};
static u8 NT35590_AUO_Param329[]={0x6E,0xB8};
static u8 NT35590_AUO_Param330[]={0x6F,0x03};
static u8 NT35590_AUO_Param331[]={0x70,0xBE};
static u8 NT35590_AUO_Param332[]={0x71,0x00};
static u8 NT35590_AUO_Param333[]={0x72,0x98};
static u8 NT35590_AUO_Param334[]={0x73,0x00};
static u8 NT35590_AUO_Param335[]={0x74,0xAF};
static u8 NT35590_AUO_Param336[]={0x75,0x00};
static u8 NT35590_AUO_Param337[]={0x76,0xD1};
static u8 NT35590_AUO_Param338[]={0x77,0x00};
static u8 NT35590_AUO_Param339[]={0x78,0xE9};
static u8 NT35590_AUO_Param340[]={0x79,0x00};
static u8 NT35590_AUO_Param341[]={0x7A,0xFE};
static u8 NT35590_AUO_Param342[]={0x7B,0x01};
static u8 NT35590_AUO_Param343[]={0x7C,0x10};
static u8 NT35590_AUO_Param344[]={0x7D,0x01};
static u8 NT35590_AUO_Param345[]={0x7E,0x20};
static u8 NT35590_AUO_Param346[]={0x7F,0x01};
static u8 NT35590_AUO_Param347[]={0x80,0x2E};
static u8 NT35590_AUO_Param348[]={0x81,0x01};
static u8 NT35590_AUO_Param349[]={0x82,0x3B};
static u8 NT35590_AUO_Param350[]={0x83,0x01};
static u8 NT35590_AUO_Param351[]={0x84,0x65};
static u8 NT35590_AUO_Param352[]={0x85,0x01};
static u8 NT35590_AUO_Param353[]={0x86,0x88};
static u8 NT35590_AUO_Param354[]={0x87,0x01};
static u8 NT35590_AUO_Param355[]={0x88,0xBD};
static u8 NT35590_AUO_Param356[]={0x89,0x01};
static u8 NT35590_AUO_Param357[]={0x8A,0xE7};
static u8 NT35590_AUO_Param358[]={0x8B,0x02};
static u8 NT35590_AUO_Param359[]={0x8C,0x27};
static u8 NT35590_AUO_Param360[]={0x8D,0x02};
static u8 NT35590_AUO_Param361[]={0x8E,0x59};
static u8 NT35590_AUO_Param362[]={0x8F,0x02};
static u8 NT35590_AUO_Param363[]={0x90,0x5B};
static u8 NT35590_AUO_Param364[]={0x91,0x02};
static u8 NT35590_AUO_Param365[]={0x92,0x87};
static u8 NT35590_AUO_Param366[]={0x93,0x02};
static u8 NT35590_AUO_Param367[]={0x94,0xB6};
static u8 NT35590_AUO_Param368[]={0x95,0x02};
static u8 NT35590_AUO_Param369[]={0x96,0xD5};
static u8 NT35590_AUO_Param370[]={0x97,0x02};
static u8 NT35590_AUO_Param371[]={0x98,0xFD};
static u8 NT35590_AUO_Param372[]={0x99,0x03};
static u8 NT35590_AUO_Param373[]={0x9A,0x19};
static u8 NT35590_AUO_Param374[]={0x9B,0x03};
static u8 NT35590_AUO_Param375[]={0x9C,0x40};
static u8 NT35590_AUO_Param376[]={0x9D,0x03};
static u8 NT35590_AUO_Param377[]={0x9E,0x4C};
static u8 NT35590_AUO_Param378[]={0x9F,0x03};
static u8 NT35590_AUO_Param379[]={0xA0,0x59};
static u8 NT35590_AUO_Param380[]={0xA2,0x03};
static u8 NT35590_AUO_Param381[]={0xA3,0x67};
static u8 NT35590_AUO_Param382[]={0xA4,0x03};
static u8 NT35590_AUO_Param383[]={0xA5,0x78};
static u8 NT35590_AUO_Param384[]={0xA6,0x03};
static u8 NT35590_AUO_Param385[]={0xA7,0x8A};
static u8 NT35590_AUO_Param386[]={0xA9,0x03};
static u8 NT35590_AUO_Param387[]={0xAA,0xA8};
static u8 NT35590_AUO_Param388[]={0xAB,0x03};
static u8 NT35590_AUO_Param389[]={0xAC,0xB8};
static u8 NT35590_AUO_Param390[]={0xAD,0x03};
static u8 NT35590_AUO_Param391[]={0xAE,0xBE};
static u8 NT35590_AUO_Param392[]={0xAF,0x00};
static u8 NT35590_AUO_Param393[]={0xB0,0x98};
static u8 NT35590_AUO_Param394[]={0xB1,0x00};
static u8 NT35590_AUO_Param395[]={0xB2,0xAF};
static u8 NT35590_AUO_Param396[]={0xB3,0x00};
static u8 NT35590_AUO_Param397[]={0xB4,0xD1};
static u8 NT35590_AUO_Param398[]={0xB5,0x00};
static u8 NT35590_AUO_Param399[]={0xB6,0xE9};
static u8 NT35590_AUO_Param400[]={0xB7,0x00};
static u8 NT35590_AUO_Param401[]={0xB8,0xFE};
static u8 NT35590_AUO_Param402[]={0xB9,0x01};
static u8 NT35590_AUO_Param403[]={0xBA,0x10};
static u8 NT35590_AUO_Param404[]={0xBB,0x01};
static u8 NT35590_AUO_Param405[]={0xBC,0x20};
static u8 NT35590_AUO_Param406[]={0xBD,0x01};
static u8 NT35590_AUO_Param407[]={0xBE,0x2E};
static u8 NT35590_AUO_Param408[]={0xBF,0x01};
static u8 NT35590_AUO_Param409[]={0xC0,0x3B};
static u8 NT35590_AUO_Param410[]={0xC1,0x01};
static u8 NT35590_AUO_Param411[]={0xC2,0x65};
static u8 NT35590_AUO_Param412[]={0xC3,0x01};
static u8 NT35590_AUO_Param413[]={0xC4,0x88};
static u8 NT35590_AUO_Param414[]={0xC5,0x01};
static u8 NT35590_AUO_Param415[]={0xC6,0xBD};
static u8 NT35590_AUO_Param416[]={0xC7,0x01};
static u8 NT35590_AUO_Param417[]={0xC8,0xE7};
static u8 NT35590_AUO_Param418[]={0xC9,0x02};
static u8 NT35590_AUO_Param419[]={0xCA,0x27};
static u8 NT35590_AUO_Param420[]={0xCB,0x02};
static u8 NT35590_AUO_Param421[]={0xCC,0x59};
static u8 NT35590_AUO_Param422[]={0xCD,0x02};
static u8 NT35590_AUO_Param423[]={0xCE,0x5B};
static u8 NT35590_AUO_Param424[]={0xCF,0x02};
static u8 NT35590_AUO_Param425[]={0xD0,0x87};
static u8 NT35590_AUO_Param426[]={0xD1,0x02};
static u8 NT35590_AUO_Param427[]={0xD2,0xB6};
static u8 NT35590_AUO_Param428[]={0xD3,0x02};
static u8 NT35590_AUO_Param429[]={0xD4,0xD5};
static u8 NT35590_AUO_Param430[]={0xD5,0x02};
static u8 NT35590_AUO_Param431[]={0xD6,0xFD};
static u8 NT35590_AUO_Param432[]={0xD7,0x03};
static u8 NT35590_AUO_Param433[]={0xD8,0x19};
static u8 NT35590_AUO_Param434[]={0xD9,0x03};
static u8 NT35590_AUO_Param435[]={0xDA,0x40};
static u8 NT35590_AUO_Param436[]={0xDB,0x03};
static u8 NT35590_AUO_Param437[]={0xDC,0x4C};
static u8 NT35590_AUO_Param438[]={0xDD,0x03};
static u8 NT35590_AUO_Param439[]={0xDE,0x59};
static u8 NT35590_AUO_Param440[]={0xDF,0x03};
static u8 NT35590_AUO_Param441[]={0xE0,0x67};
static u8 NT35590_AUO_Param442[]={0xE1,0x03};
static u8 NT35590_AUO_Param443[]={0xE2,0x78};
static u8 NT35590_AUO_Param444[]={0xE3,0x03};
static u8 NT35590_AUO_Param445[]={0xE4,0x8A};
static u8 NT35590_AUO_Param446[]={0xE5,0x03};
static u8 NT35590_AUO_Param447[]={0xE6,0xA8};
static u8 NT35590_AUO_Param448[]={0xE7,0x03};
static u8 NT35590_AUO_Param449[]={0xE8,0xB8};
static u8 NT35590_AUO_Param450[]={0xE9,0x03};
static u8 NT35590_AUO_Param451[]={0xEA,0xBE};
static u8 NT35590_AUO_Param452[]={0xFF,0x00};
static u8 NT35590_AUO_Param453[]={0xFF,0x03};
static u8 NT35590_AUO_Param454[]={0xFB,0x08};
static u8 NT35590_AUO_Param455[]={0x18,0x00};
static u8 NT35590_AUO_Param456[]={0x19,0x00};
static u8 NT35590_AUO_Param457[]={0x1A,0x00};
static u8 NT35590_AUO_Param458[]={0x25,0x26};
static u8 NT35590_AUO_Param459[]={0x00,0x00};
static u8 NT35590_AUO_Param460[]={0x01,0x08};
static u8 NT35590_AUO_Param461[]={0x02,0x0C};
static u8 NT35590_AUO_Param462[]={0x03,0x10};
static u8 NT35590_AUO_Param463[]={0x04,0x14};
static u8 NT35590_AUO_Param464[]={0x05,0x18};
static u8 NT35590_AUO_Param465[]={0x06,0x1C};
static u8 NT35590_AUO_Param466[]={0x07,0x20};
static u8 NT35590_AUO_Param467[]={0x08,0x24};
static u8 NT35590_AUO_Param468[]={0x09,0x28};
static u8 NT35590_AUO_Param469[]={0x0A,0x2C};
static u8 NT35590_AUO_Param470[]={0x0B,0x30};
static u8 NT35590_AUO_Param471[]={0x0C,0x34};
static u8 NT35590_AUO_Param472[]={0x0D,0x38};
static u8 NT35590_AUO_Param473[]={0x0E,0x3C};
static u8 NT35590_AUO_Param474[]={0x0F,0x3F};
static u8 NT35590_AUO_Param475[]={0xFB,0x01};
static u8 NT35590_AUO_Param476[]={0xFF,0x00};
static u8 NT35590_AUO_Param477[]={0xFE,0x01};
static u8 NT35590_AUO_param478[]={0x36,0xC0};

#define PANEL_NAME "Lead/Success AUO"

static
int mdfld_dsi_nt35590_auo_drv_ic_init(struct mdfld_dsi_config *dsi_config)
{
	struct drm_device *dev = dsi_config->dev;
	struct mdfld_dsi_pkg_sender *sender
			= mdfld_dsi_get_pkg_sender(dsi_config);

	if (!sender)
		return -EINVAL;

	sender->status = MDFLD_DSI_PKG_SENDER_FREE;

	PSB_DEBUG_ENTRY("\n");
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param5[0],NT35590_AUO_Param5[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param6[0],NT35590_AUO_Param6[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param7[0],NT35590_AUO_Param7[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param8[0],NT35590_AUO_Param8[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param9[0],NT35590_AUO_Param9[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param10[0],NT35590_AUO_Param10[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param11[0],NT35590_AUO_Param11[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param12[0],NT35590_AUO_Param12[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param13[0],NT35590_AUO_Param13[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param14[0],NT35590_AUO_Param14[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param15[0],NT35590_AUO_Param15[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param16[0],NT35590_AUO_Param16[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param17[0],NT35590_AUO_Param17[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param18[0],NT35590_AUO_Param18[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param19[0],NT35590_AUO_Param19[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param20[0],NT35590_AUO_Param20[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param21[0],NT35590_AUO_Param21[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param22[0],NT35590_AUO_Param22[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param23[0],NT35590_AUO_Param23[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param24[0],NT35590_AUO_Param24[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param25[0],NT35590_AUO_Param25[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param26[0],NT35590_AUO_Param26[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param27[0],NT35590_AUO_Param27[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param28[0],NT35590_AUO_Param28[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param29[0],NT35590_AUO_Param29[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param30[0],NT35590_AUO_Param30[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param31[0],NT35590_AUO_Param31[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param32[0],NT35590_AUO_Param32[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param33[0],NT35590_AUO_Param33[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param34[0],NT35590_AUO_Param34[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param35[0],NT35590_AUO_Param35[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param36[0],NT35590_AUO_Param36[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param37[0],NT35590_AUO_Param37[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param38[0],NT35590_AUO_Param38[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param39[0],NT35590_AUO_Param39[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param40[0],NT35590_AUO_Param40[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param41[0],NT35590_AUO_Param41[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param42[0],NT35590_AUO_Param42[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param43[0],NT35590_AUO_Param43[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param44[0],NT35590_AUO_Param44[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param45[0],NT35590_AUO_Param45[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param46[0],NT35590_AUO_Param46[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param47[0],NT35590_AUO_Param47[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param48[0],NT35590_AUO_Param48[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param49[0],NT35590_AUO_Param49[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param50[0],NT35590_AUO_Param50[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param51[0],NT35590_AUO_Param51[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param52[0],NT35590_AUO_Param52[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param53[0],NT35590_AUO_Param53[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param54[0],NT35590_AUO_Param54[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param55[0],NT35590_AUO_Param55[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param56[0],NT35590_AUO_Param56[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param57[0],NT35590_AUO_Param57[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param58[0],NT35590_AUO_Param58[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param59[0],NT35590_AUO_Param59[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param60[0],NT35590_AUO_Param60[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param61[0],NT35590_AUO_Param61[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param62[0],NT35590_AUO_Param62[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param63[0],NT35590_AUO_Param63[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param64[0],NT35590_AUO_Param64[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param65[0],NT35590_AUO_Param65[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param66[0],NT35590_AUO_Param66[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param67[0],NT35590_AUO_Param67[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param68[0],NT35590_AUO_Param68[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param69[0],NT35590_AUO_Param69[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param70[0],NT35590_AUO_Param70[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param71[0],NT35590_AUO_Param71[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param72[0],NT35590_AUO_Param72[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param73[0],NT35590_AUO_Param73[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param74[0],NT35590_AUO_Param74[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param75[0],NT35590_AUO_Param75[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param76[0],NT35590_AUO_Param76[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param77[0],NT35590_AUO_Param77[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param78[0],NT35590_AUO_Param78[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param79[0],NT35590_AUO_Param79[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param80[0],NT35590_AUO_Param80[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param81[0],NT35590_AUO_Param81[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param82[0],NT35590_AUO_Param82[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param83[0],NT35590_AUO_Param83[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param84[0],NT35590_AUO_Param84[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param85[0],NT35590_AUO_Param85[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param86[0],NT35590_AUO_Param86[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param87[0],NT35590_AUO_Param87[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param88[0],NT35590_AUO_Param88[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param89[0],NT35590_AUO_Param89[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param90[0],NT35590_AUO_Param90[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param91[0],NT35590_AUO_Param91[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param92[0],NT35590_AUO_Param92[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param93[0],NT35590_AUO_Param93[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param94[0],NT35590_AUO_Param94[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param95[0],NT35590_AUO_Param95[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param96[0],NT35590_AUO_Param96[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param97[0],NT35590_AUO_Param97[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param98[0],NT35590_AUO_Param98[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param99[0],NT35590_AUO_Param99[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param100[0],NT35590_AUO_Param100[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param101[0],NT35590_AUO_Param101[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param102[0],NT35590_AUO_Param102[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param103[0],NT35590_AUO_Param103[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param104[0],NT35590_AUO_Param104[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param105[0],NT35590_AUO_Param105[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param106[0],NT35590_AUO_Param106[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param107[0],NT35590_AUO_Param107[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param108[0],NT35590_AUO_Param108[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param109[0],NT35590_AUO_Param109[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param110[0],NT35590_AUO_Param110[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param111[0],NT35590_AUO_Param111[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param112[0],NT35590_AUO_Param112[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param113[0],NT35590_AUO_Param113[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param114[0],NT35590_AUO_Param114[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param115[0],NT35590_AUO_Param115[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param116[0],NT35590_AUO_Param116[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param117[0],NT35590_AUO_Param117[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param118[0],NT35590_AUO_Param118[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param119[0],NT35590_AUO_Param119[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param120[0],NT35590_AUO_Param120[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param121[0],NT35590_AUO_Param121[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param122[0],NT35590_AUO_Param122[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param123[0],NT35590_AUO_Param123[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param124[0],NT35590_AUO_Param124[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param125[0],NT35590_AUO_Param125[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param126[0],NT35590_AUO_Param126[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param127[0],NT35590_AUO_Param127[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param128[0],NT35590_AUO_Param128[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param129[0],NT35590_AUO_Param129[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param130[0],NT35590_AUO_Param130[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param131[0],NT35590_AUO_Param131[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param132[0],NT35590_AUO_Param132[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param133[0],NT35590_AUO_Param133[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param134[0],NT35590_AUO_Param134[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param135[0],NT35590_AUO_Param135[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param136[0],NT35590_AUO_Param136[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param137[0],NT35590_AUO_Param137[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param138[0],NT35590_AUO_Param138[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param139[0],NT35590_AUO_Param139[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param140[0],NT35590_AUO_Param140[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param141[0],NT35590_AUO_Param141[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param142[0],NT35590_AUO_Param142[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param143[0],NT35590_AUO_Param143[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param144[0],NT35590_AUO_Param144[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param145[0],NT35590_AUO_Param145[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param146[0],NT35590_AUO_Param146[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param147[0],NT35590_AUO_Param147[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param148[0],NT35590_AUO_Param148[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param149[0],NT35590_AUO_Param149[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param150[0],NT35590_AUO_Param150[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param151[0],NT35590_AUO_Param151[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param152[0],NT35590_AUO_Param152[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param153[0],NT35590_AUO_Param153[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param154[0],NT35590_AUO_Param154[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param155[0],NT35590_AUO_Param155[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param156[0],NT35590_AUO_Param156[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param157[0],NT35590_AUO_Param157[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param158[0],NT35590_AUO_Param158[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param159[0],NT35590_AUO_Param159[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param160[0],NT35590_AUO_Param160[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param161[0],NT35590_AUO_Param161[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param162[0],NT35590_AUO_Param162[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param163[0],NT35590_AUO_Param163[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param164[0],NT35590_AUO_Param164[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param165[0],NT35590_AUO_Param165[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param166[0],NT35590_AUO_Param166[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param167[0],NT35590_AUO_Param167[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param168[0],NT35590_AUO_Param168[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param169[0],NT35590_AUO_Param169[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param170[0],NT35590_AUO_Param170[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param171[0],NT35590_AUO_Param171[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param172[0],NT35590_AUO_Param172[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param173[0],NT35590_AUO_Param173[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param174[0],NT35590_AUO_Param174[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param175[0],NT35590_AUO_Param175[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param176[0],NT35590_AUO_Param176[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param177[0],NT35590_AUO_Param177[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param178[0],NT35590_AUO_Param178[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param179[0],NT35590_AUO_Param179[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param180[0],NT35590_AUO_Param180[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param181[0],NT35590_AUO_Param181[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param182[0],NT35590_AUO_Param182[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param183[0],NT35590_AUO_Param183[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param184[0],NT35590_AUO_Param184[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param185[0],NT35590_AUO_Param185[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param186[0],NT35590_AUO_Param186[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param187[0],NT35590_AUO_Param187[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param188[0],NT35590_AUO_Param188[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param189[0],NT35590_AUO_Param189[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param190[0],NT35590_AUO_Param190[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param191[0],NT35590_AUO_Param191[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param192[0],NT35590_AUO_Param192[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param193[0],NT35590_AUO_Param193[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param194[0],NT35590_AUO_Param194[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param195[0],NT35590_AUO_Param195[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param196[0],NT35590_AUO_Param196[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param197[0],NT35590_AUO_Param197[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param198[0],NT35590_AUO_Param198[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param199[0],NT35590_AUO_Param199[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param200[0],NT35590_AUO_Param200[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param201[0],NT35590_AUO_Param201[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param202[0],NT35590_AUO_Param202[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param203[0],NT35590_AUO_Param203[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param204[0],NT35590_AUO_Param204[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param205[0],NT35590_AUO_Param205[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param206[0],NT35590_AUO_Param206[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param207[0],NT35590_AUO_Param207[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param208[0],NT35590_AUO_Param208[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param209[0],NT35590_AUO_Param209[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param210[0],NT35590_AUO_Param210[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param211[0],NT35590_AUO_Param211[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param212[0],NT35590_AUO_Param212[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param213[0],NT35590_AUO_Param213[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param214[0],NT35590_AUO_Param214[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param215[0],NT35590_AUO_Param215[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param216[0],NT35590_AUO_Param216[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param217[0],NT35590_AUO_Param217[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param218[0],NT35590_AUO_Param218[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param219[0],NT35590_AUO_Param219[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param220[0],NT35590_AUO_Param220[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param221[0],NT35590_AUO_Param221[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param222[0],NT35590_AUO_Param222[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param223[0],NT35590_AUO_Param223[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param224[0],NT35590_AUO_Param224[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param225[0],NT35590_AUO_Param225[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param226[0],NT35590_AUO_Param226[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param227[0],NT35590_AUO_Param227[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param228[0],NT35590_AUO_Param228[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param229[0],NT35590_AUO_Param229[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param230[0],NT35590_AUO_Param230[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param231[0],NT35590_AUO_Param231[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param232[0],NT35590_AUO_Param232[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param233[0],NT35590_AUO_Param233[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param234[0],NT35590_AUO_Param234[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param235[0],NT35590_AUO_Param235[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param236[0],NT35590_AUO_Param236[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param237[0],NT35590_AUO_Param237[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param238[0],NT35590_AUO_Param238[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param239[0],NT35590_AUO_Param239[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param240[0],NT35590_AUO_Param240[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param241[0],NT35590_AUO_Param241[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param242[0],NT35590_AUO_Param242[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param243[0],NT35590_AUO_Param243[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param244[0],NT35590_AUO_Param244[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param245[0],NT35590_AUO_Param245[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param246[0],NT35590_AUO_Param246[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param247[0],NT35590_AUO_Param247[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param248[0],NT35590_AUO_Param248[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param249[0],NT35590_AUO_Param249[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param250[0],NT35590_AUO_Param250[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param251[0],NT35590_AUO_Param251[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param252[0],NT35590_AUO_Param252[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param253[0],NT35590_AUO_Param253[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param254[0],NT35590_AUO_Param254[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param255[0],NT35590_AUO_Param255[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param256[0],NT35590_AUO_Param256[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param257[0],NT35590_AUO_Param257[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param258[0],NT35590_AUO_Param258[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param259[0],NT35590_AUO_Param259[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param260[0],NT35590_AUO_Param260[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param261[0],NT35590_AUO_Param261[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param262[0],NT35590_AUO_Param262[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param263[0],NT35590_AUO_Param263[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param264[0],NT35590_AUO_Param264[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param265[0],NT35590_AUO_Param265[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param266[0],NT35590_AUO_Param266[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param267[0],NT35590_AUO_Param267[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param268[0],NT35590_AUO_Param268[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param269[0],NT35590_AUO_Param269[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param270[0],NT35590_AUO_Param270[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param271[0],NT35590_AUO_Param271[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param272[0],NT35590_AUO_Param272[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param273[0],NT35590_AUO_Param273[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param274[0],NT35590_AUO_Param274[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param275[0],NT35590_AUO_Param275[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param276[0],NT35590_AUO_Param276[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param277[0],NT35590_AUO_Param277[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param278[0],NT35590_AUO_Param278[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param279[0],NT35590_AUO_Param279[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param280[0],NT35590_AUO_Param280[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param281[0],NT35590_AUO_Param281[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param282[0],NT35590_AUO_Param282[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param283[0],NT35590_AUO_Param283[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param284[0],NT35590_AUO_Param284[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param285[0],NT35590_AUO_Param285[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param286[0],NT35590_AUO_Param286[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param287[0],NT35590_AUO_Param287[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param288[0],NT35590_AUO_Param288[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param289[0],NT35590_AUO_Param289[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param290[0],NT35590_AUO_Param290[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param291[0],NT35590_AUO_Param291[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param292[0],NT35590_AUO_Param292[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param293[0],NT35590_AUO_Param293[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param294[0],NT35590_AUO_Param294[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param295[0],NT35590_AUO_Param295[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param296[0],NT35590_AUO_Param296[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param297[0],NT35590_AUO_Param297[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param298[0],NT35590_AUO_Param298[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param299[0],NT35590_AUO_Param299[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param300[0],NT35590_AUO_Param300[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param301[0],NT35590_AUO_Param301[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param302[0],NT35590_AUO_Param302[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param303[0],NT35590_AUO_Param303[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param304[0],NT35590_AUO_Param304[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param305[0],NT35590_AUO_Param305[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param306[0],NT35590_AUO_Param306[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param307[0],NT35590_AUO_Param307[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param308[0],NT35590_AUO_Param308[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param309[0],NT35590_AUO_Param309[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param310[0],NT35590_AUO_Param310[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param311[0],NT35590_AUO_Param311[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param312[0],NT35590_AUO_Param312[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param313[0],NT35590_AUO_Param313[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param314[0],NT35590_AUO_Param314[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param315[0],NT35590_AUO_Param315[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param316[0],NT35590_AUO_Param316[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param317[0],NT35590_AUO_Param317[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param318[0],NT35590_AUO_Param318[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param319[0],NT35590_AUO_Param319[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param320[0],NT35590_AUO_Param320[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param321[0],NT35590_AUO_Param321[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param322[0],NT35590_AUO_Param322[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param323[0],NT35590_AUO_Param323[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param324[0],NT35590_AUO_Param324[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param325[0],NT35590_AUO_Param325[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param326[0],NT35590_AUO_Param326[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param327[0],NT35590_AUO_Param327[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param328[0],NT35590_AUO_Param328[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param329[0],NT35590_AUO_Param329[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param330[0],NT35590_AUO_Param330[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param331[0],NT35590_AUO_Param331[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param332[0],NT35590_AUO_Param332[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param333[0],NT35590_AUO_Param333[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param334[0],NT35590_AUO_Param334[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param335[0],NT35590_AUO_Param335[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param336[0],NT35590_AUO_Param336[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param337[0],NT35590_AUO_Param337[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param338[0],NT35590_AUO_Param338[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param339[0],NT35590_AUO_Param339[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param340[0],NT35590_AUO_Param340[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param341[0],NT35590_AUO_Param341[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param342[0],NT35590_AUO_Param342[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param343[0],NT35590_AUO_Param343[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param344[0],NT35590_AUO_Param344[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param345[0],NT35590_AUO_Param345[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param346[0],NT35590_AUO_Param346[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param347[0],NT35590_AUO_Param347[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param348[0],NT35590_AUO_Param348[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param349[0],NT35590_AUO_Param349[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param350[0],NT35590_AUO_Param350[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param351[0],NT35590_AUO_Param351[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param352[0],NT35590_AUO_Param352[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param353[0],NT35590_AUO_Param353[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param354[0],NT35590_AUO_Param354[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param355[0],NT35590_AUO_Param355[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param356[0],NT35590_AUO_Param356[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param357[0],NT35590_AUO_Param357[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param358[0],NT35590_AUO_Param358[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param359[0],NT35590_AUO_Param359[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param360[0],NT35590_AUO_Param360[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param361[0],NT35590_AUO_Param361[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param362[0],NT35590_AUO_Param362[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param363[0],NT35590_AUO_Param363[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param364[0],NT35590_AUO_Param364[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param365[0],NT35590_AUO_Param365[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param366[0],NT35590_AUO_Param366[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param367[0],NT35590_AUO_Param367[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param368[0],NT35590_AUO_Param368[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param369[0],NT35590_AUO_Param369[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param370[0],NT35590_AUO_Param370[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param371[0],NT35590_AUO_Param371[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param372[0],NT35590_AUO_Param372[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param373[0],NT35590_AUO_Param373[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param374[0],NT35590_AUO_Param374[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param375[0],NT35590_AUO_Param375[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param376[0],NT35590_AUO_Param376[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param377[0],NT35590_AUO_Param377[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param378[0],NT35590_AUO_Param378[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param379[0],NT35590_AUO_Param379[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param380[0],NT35590_AUO_Param380[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param381[0],NT35590_AUO_Param381[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param382[0],NT35590_AUO_Param382[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param383[0],NT35590_AUO_Param383[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param384[0],NT35590_AUO_Param384[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param385[0],NT35590_AUO_Param385[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param386[0],NT35590_AUO_Param386[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param387[0],NT35590_AUO_Param387[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param388[0],NT35590_AUO_Param388[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param389[0],NT35590_AUO_Param389[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param390[0],NT35590_AUO_Param390[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param391[0],NT35590_AUO_Param391[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param392[0],NT35590_AUO_Param392[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param393[0],NT35590_AUO_Param393[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param394[0],NT35590_AUO_Param394[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param395[0],NT35590_AUO_Param395[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param396[0],NT35590_AUO_Param396[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param397[0],NT35590_AUO_Param397[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param398[0],NT35590_AUO_Param398[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param399[0],NT35590_AUO_Param399[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param400[0],NT35590_AUO_Param400[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param401[0],NT35590_AUO_Param401[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param402[0],NT35590_AUO_Param402[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param403[0],NT35590_AUO_Param403[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param404[0],NT35590_AUO_Param404[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param405[0],NT35590_AUO_Param405[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param406[0],NT35590_AUO_Param406[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param407[0],NT35590_AUO_Param407[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param408[0],NT35590_AUO_Param408[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param409[0],NT35590_AUO_Param409[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param410[0],NT35590_AUO_Param410[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param411[0],NT35590_AUO_Param411[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param412[0],NT35590_AUO_Param412[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param413[0],NT35590_AUO_Param413[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param414[0],NT35590_AUO_Param414[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param415[0],NT35590_AUO_Param415[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param416[0],NT35590_AUO_Param416[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param417[0],NT35590_AUO_Param417[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param418[0],NT35590_AUO_Param418[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param419[0],NT35590_AUO_Param419[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param420[0],NT35590_AUO_Param420[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param421[0],NT35590_AUO_Param421[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param422[0],NT35590_AUO_Param422[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param423[0],NT35590_AUO_Param423[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param424[0],NT35590_AUO_Param424[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param425[0],NT35590_AUO_Param425[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param426[0],NT35590_AUO_Param426[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param427[0],NT35590_AUO_Param427[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param428[0],NT35590_AUO_Param428[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param429[0],NT35590_AUO_Param429[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param430[0],NT35590_AUO_Param430[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param431[0],NT35590_AUO_Param431[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param432[0],NT35590_AUO_Param432[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param433[0],NT35590_AUO_Param433[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param434[0],NT35590_AUO_Param434[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param435[0],NT35590_AUO_Param435[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param436[0],NT35590_AUO_Param436[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param437[0],NT35590_AUO_Param437[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param438[0],NT35590_AUO_Param438[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param439[0],NT35590_AUO_Param439[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param440[0],NT35590_AUO_Param440[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param441[0],NT35590_AUO_Param441[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param442[0],NT35590_AUO_Param442[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param443[0],NT35590_AUO_Param443[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param444[0],NT35590_AUO_Param444[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param445[0],NT35590_AUO_Param445[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param446[0],NT35590_AUO_Param446[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param447[0],NT35590_AUO_Param447[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param448[0],NT35590_AUO_Param448[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param449[0],NT35590_AUO_Param449[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param450[0],NT35590_AUO_Param450[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param451[0],NT35590_AUO_Param451[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param452[0],NT35590_AUO_Param452[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param453[0],NT35590_AUO_Param453[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param454[0],NT35590_AUO_Param454[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param455[0],NT35590_AUO_Param455[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param456[0],NT35590_AUO_Param456[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param457[0],NT35590_AUO_Param457[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param458[0],NT35590_AUO_Param458[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param459[0],NT35590_AUO_Param459[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param460[0],NT35590_AUO_Param460[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param461[0],NT35590_AUO_Param461[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param462[0],NT35590_AUO_Param462[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param463[0],NT35590_AUO_Param463[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param464[0],NT35590_AUO_Param464[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param465[0],NT35590_AUO_Param465[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param466[0],NT35590_AUO_Param466[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param467[0],NT35590_AUO_Param467[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param468[0],NT35590_AUO_Param468[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param469[0],NT35590_AUO_Param469[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param470[0],NT35590_AUO_Param470[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param471[0],NT35590_AUO_Param471[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param472[0],NT35590_AUO_Param472[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param473[0],NT35590_AUO_Param473[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param474[0],NT35590_AUO_Param474[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param475[0],NT35590_AUO_Param475[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param476[0],NT35590_AUO_Param476[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param477[0],NT35590_AUO_Param477[1],1,0);
#ifdef CONFIG_PROJECT_P940V10
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_param478[0],NT35590_AUO_param478[1],1,0);
#endif
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
void mdfld_dsi_nt35590_auo_dsi_controller_init(struct mdfld_dsi_config *dsi_config)
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
	hw_ctx->clk_lane_switch_time_cnt =0x0018000B;// 0x20000E;
	hw_ctx->hs_ls_dbi_enable = 0x0;
	/* HW team suggested 1390 for bandwidth setting */
	hw_ctx->dbi_bw_ctrl = 820;//1390;
	hw_ctx->dphy_param = 0x1f1f3610;//0x20124E1A;
	hw_ctx->dsi_func_prg = (0xa000 | dsi_config->lane_count);
	hw_ctx->mipi = TE_TRIGGER_GPIO_PIN;
	hw_ctx->mipi |= dsi_config->lane_config;
}

static
struct drm_display_mode *nt35590_auo_cmd_get_config_mode(void)
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
int mdfld_dsi_nt35590_auo_power_on(struct mdfld_dsi_config *dsi_config)
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

static int mdfld_dsi_nt35590_auo_power_off(struct mdfld_dsi_config *dsi_config)
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
	mdelay(120);
out:
	return err;
}

static
void nt35590_auo_cmd_get_panel_info(int pipe, struct panel_info *pi)
{
	PSB_DEBUG_ENTRY("\n");

	if (pipe == 0) {
		pi->width_mm = PANEL_4DOT3_WIDTH;
		pi->height_mm = PANEL_4DOT3_HEIGHT;
	}
}

static
int mdfld_dsi_nt35590_auo_detect(struct mdfld_dsi_config *dsi_config)
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
int mdfld_dsi_nt35590_auo_set_brightness(struct mdfld_dsi_config *dsi_config,
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
int mdfld_dsi_nt35590_auo_panel_reset(struct mdfld_dsi_config *dsi_config)
{
	static int mipi_reset_gpio;
	int ret = 0;
	u32 data = 0;
	struct mdfld_dsi_pkg_sender *sender =
		mdfld_dsi_get_pkg_sender(dsi_config);

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

	}

	/* workaound for lowtemp suspned/resume initial failue */
	gpio_direction_output(mipi_reset_gpio, 1);
	mdelay(1);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param1[0],NT35590_AUO_Param1[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param2[0],NT35590_AUO_Param2[1],1,0);
	mdelay(1);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param3[0],NT35590_AUO_Param3[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,NT35590_AUO_Param4[0],NT35590_AUO_Param4[1],1,0);
	mdelay(10);

	gpio_set_value_cansleep(mipi_reset_gpio, 0);
	mdelay(20);

	gpio_set_value_cansleep(mipi_reset_gpio, 1);
	mdelay(120);

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


static void nt35590_auo_brightness_init(struct drm_device *dev)
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
void nt35590_auo_cmd_init(struct drm_device *dev, struct panel_funcs *p_funcs)
{
	int ena_err;
	if (!dev || !p_funcs) {
		DRM_ERROR("Invalid parameters\n");
		return;
	}
	PSB_DEBUG_ENTRY("\n");
	p_funcs->get_config_mode = nt35590_auo_cmd_get_config_mode;
	p_funcs->get_panel_info = nt35590_auo_cmd_get_panel_info;
	p_funcs->reset = mdfld_dsi_nt35590_auo_panel_reset;
	p_funcs->drv_ic_init = mdfld_dsi_nt35590_auo_drv_ic_init;
	p_funcs->dsi_controller_init = mdfld_dsi_nt35590_auo_dsi_controller_init;
	p_funcs->detect = mdfld_dsi_nt35590_auo_detect;
	p_funcs->power_on = mdfld_dsi_nt35590_auo_power_on;
	p_funcs->power_off = mdfld_dsi_nt35590_auo_power_off;
	p_funcs->set_brightness = mdfld_dsi_nt35590_auo_set_brightness;

#ifdef CONFIG_BL_PMU_PWM_CONTROL
	nt35590_auo_brightness_init(dev);
#endif
}

static int nt35590_auo_cmd_probe(struct platform_device *pdev)
{
	int ret = 0;

	DRM_INFO("%s: NT35590 AUO panel detected\n", __func__);
	intel_mid_panel_register(nt35590_auo_cmd_init);

	add_panel_config_prop(PANEL_NAME, "NT35590", 1280, 720);
	create_backlight_debug_file();
	return 0;
}

static struct platform_driver nt35590_auo_lcd_driver = {
	.probe	= nt35590_auo_cmd_probe,
	.driver	= {
		.name	= "NT35590AUO CMD R",
		.owner	= THIS_MODULE,
	},
};

static int __init nt35590_auo_lcd_init(void)
{
	DRM_INFO("%s\n", __func__);

	return platform_driver_register(&nt35590_auo_lcd_driver);
}
module_init(nt35590_auo_lcd_init);
