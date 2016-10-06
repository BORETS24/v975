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

static u8 nt35590_tm_para1[]={0xFF,0x00};
static u8 nt35590_tm_para2[]={0xFB,0x01};
static u8 nt35590_tm_para3[]={0x3B,0x03,0x06,0x03,0x02,0x02};
static u8 nt35590_tm_para4[]={0xFF,0x01};
static u8 nt35590_tm_para5[]={0xFB,0x01};
static u8 nt35590_tm_para6[]={0x00,0x3A};
static u8 nt35590_tm_para7[]={0x01,0x33};
static u8 nt35590_tm_para8[]={0x08,0x66};
static u8 nt35590_tm_para9[]={0x09,0x0F};
static u8 nt35590_tm_para10[]={0x0B,0xC2};
static u8 nt35590_tm_para11[]={0x0C,0xC2};
static u8 nt35590_tm_para12[]={0x0D,0x24};
static u8 nt35590_tm_para13[]={0x0E,0x33};
static u8 nt35590_tm_para14[]={0x11,0x93};
static u8 nt35590_tm_para15[]={0x12,0x03};
static u8 nt35590_tm_para16[]={0x0F,0x0A};
static u8 nt35590_tm_para17[]={0x36,0x73};
static u8 nt35590_tm_para18[]={0x71,0x2C};
static u8 nt35590_tm_para19[]={0xFF,0x05};
static u8 nt35590_tm_para20[]={0xFB,0x01};
static u8 nt35590_tm_para21[]={0x01,0x00};
static u8 nt35590_tm_para22[]={0x02,0x8D};
static u8 nt35590_tm_para23[]={0x03,0x8D};
static u8 nt35590_tm_para24[]={0x04,0x8D};
static u8 nt35590_tm_para25[]={0x05,0x30};
static u8 nt35590_tm_para26[]={0x06,0x33};
static u8 nt35590_tm_para27[]={0x07,0x77};
static u8 nt35590_tm_para28[]={0x08,0x00};
static u8 nt35590_tm_para29[]={0x09,0x00};
static u8 nt35590_tm_para30[]={0x0A,0x00};
static u8 nt35590_tm_para31[]={0x0B,0x80};
static u8 nt35590_tm_para32[]={0x0C,0xC8};
static u8 nt35590_tm_para33[]={0x0D,0x0A};
static u8 nt35590_tm_para34[]={0x0E,0x1B};
static u8 nt35590_tm_para35[]={0x0F,0x06};
static u8 nt35590_tm_para36[]={0x10,0x56};
static u8 nt35590_tm_para37[]={0x11,0x00};
static u8 nt35590_tm_para38[]={0x12,0x00};
static u8 nt35590_tm_para39[]={0x13,0x1E};
static u8 nt35590_tm_para40[]={0x14,0x00};
static u8 nt35590_tm_para41[]={0x15,0x1A};
static u8 nt35590_tm_para42[]={0x16,0x05};
static u8 nt35590_tm_para43[]={0x17,0x00};
static u8 nt35590_tm_para44[]={0x18,0x1E};
static u8 nt35590_tm_para45[]={0x19,0xFF};
static u8 nt35590_tm_para46[]={0x1A,0x00};
static u8 nt35590_tm_para47[]={0x1B,0xFC};
static u8 nt35590_tm_para48[]={0x1C,0x80};
static u8 nt35590_tm_para49[]={0x1D,0x00};
static u8 nt35590_tm_para50[]={0x1E,0x00};
static u8 nt35590_tm_para51[]={0x1F,0x77};
static u8 nt35590_tm_para52[]={0x20,0x00};
static u8 nt35590_tm_para53[]={0x21,0x00};
static u8 nt35590_tm_para54[]={0x22,0x55};
static u8 nt35590_tm_para55[]={0x23,0x0D};
static u8 nt35590_tm_para56[]={0x31,0xA0};
static u8 nt35590_tm_para57[]={0x32,0x00};
static u8 nt35590_tm_para58[]={0x33,0xB8};
static u8 nt35590_tm_para59[]={0x34,0xBB};
static u8 nt35590_tm_para60[]={0x35,0x11};
static u8 nt35590_tm_para61[]={0x36,0x02};
static u8 nt35590_tm_para62[]={0x37,0x00};
static u8 nt35590_tm_para63[]={0x38,0x01};
static u8 nt35590_tm_para64[]={0x39,0x0B};
static u8 nt35590_tm_para65[]={0x44,0x08};
static u8 nt35590_tm_para66[]={0x45,0x80};
static u8 nt35590_tm_para67[]={0x46,0xCC};
static u8 nt35590_tm_para68[]={0x47,0x04};
static u8 nt35590_tm_para69[]={0x48,0x00};
static u8 nt35590_tm_para70[]={0x49,0x00};
static u8 nt35590_tm_para71[]={0x4A,0x01};
static u8 nt35590_tm_para72[]={0x6C,0x03};
static u8 nt35590_tm_para73[]={0x6D,0x03};
static u8 nt35590_tm_para74[]={0x6E,0x2F};
static u8 nt35590_tm_para75[]={0x43,0x00};
static u8 nt35590_tm_para76[]={0x4B,0x23};
static u8 nt35590_tm_para77[]={0x4C,0x01};
static u8 nt35590_tm_para78[]={0x50,0x23};
static u8 nt35590_tm_para79[]={0x51,0x01};
static u8 nt35590_tm_para80[]={0x58,0x23};
static u8 nt35590_tm_para81[]={0x59,0x01};
static u8 nt35590_tm_para82[]={0x5D,0x23};
static u8 nt35590_tm_para83[]={0x5E,0x01};
static u8 nt35590_tm_para84[]={0x89,0x00};
static u8 nt35590_tm_para85[]={0x8D,0x01};
static u8 nt35590_tm_para86[]={0x8E,0x64};
static u8 nt35590_tm_para87[]={0x8F,0x20};
static u8 nt35590_tm_para88[]={0x97,0x8E};
static u8 nt35590_tm_para89[]={0x82,0x8C};
static u8 nt35590_tm_para90[]={0x83,0x02};
static u8 nt35590_tm_para91[]={0xBB,0x0A};
static u8 nt35590_tm_para92[]={0xBC,0x0A};
static u8 nt35590_tm_para93[]={0x24,0x25};
static u8 nt35590_tm_para94[]={0x25,0x55};
static u8 nt35590_tm_para95[]={0x26,0x05};
static u8 nt35590_tm_para96[]={0x27,0x23};
static u8 nt35590_tm_para97[]={0x28,0x01};
static u8 nt35590_tm_para98[]={0x29,0x31};
static u8 nt35590_tm_para99[]={0x2A,0x5D};
static u8 nt35590_tm_para100[]={0x2B,0x01};
static u8 nt35590_tm_para101[]={0x2F,0x00};
static u8 nt35590_tm_para102[]={0x30,0x10};
static u8 nt35590_tm_para103[]={0xA7,0x12};
static u8 nt35590_tm_para104[]={0x2D,0x03};
static u8 nt35590_tm_para105[]={0xFF,0x00};
static u8 nt35590_tm_para106[]={0xFF,0x01};
static u8 nt35590_tm_para107[]={0x75,0X00};
static u8 nt35590_tm_para108[]={0x76,0X72};
static u8 nt35590_tm_para109[]={0x77,0x00};
static u8 nt35590_tm_para110[]={0x78,0x82};
static u8 nt35590_tm_para111[]={0x79,0x00};
static u8 nt35590_tm_para112[]={0x7A,0xAC};
static u8 nt35590_tm_para113[]={0x7B,0x00};
static u8 nt35590_tm_para114[]={0x7C,0xC8};
static u8 nt35590_tm_para115[]={0x7D,0x00};
static u8 nt35590_tm_para116[]={0x7E,0xE2};
static u8 nt35590_tm_para117[]={0x7F,0x00};
static u8 nt35590_tm_para118[]={0x80,0xF4};
static u8 nt35590_tm_para119[]={0x81,0x00};
static u8 nt35590_tm_para120[]={0x82,0xFE};
static u8 nt35590_tm_para121[]={0x83,0x01};
static u8 nt35590_tm_para122[]={0x84,0x08};
static u8 nt35590_tm_para123[]={0x85,0x01};
static u8 nt35590_tm_para124[]={0x86,0x16};
static u8 nt35590_tm_para125[]={0x87,0x01};
static u8 nt35590_tm_para126[]={0x88,0x44};
static u8 nt35590_tm_para127[]={0x89,0x01};
static u8 nt35590_tm_para128[]={0x8A,0x64};
static u8 nt35590_tm_para129[]={0x8B,0x01};
static u8 nt35590_tm_para130[]={0x8C,0x9C};
static u8 nt35590_tm_para131[]={0x8D,0x01};
static u8 nt35590_tm_para132[]={0x8E,0xC6};
static u8 nt35590_tm_para133[]={0x8F,0x02};
static u8 nt35590_tm_para134[]={0x90,0x08};
static u8 nt35590_tm_para135[]={0x91,0x02};
static u8 nt35590_tm_para136[]={0x92,0x40};
static u8 nt35590_tm_para137[]={0x93,0x02};
static u8 nt35590_tm_para138[]={0x94,0x41};
static u8 nt35590_tm_para139[]={0x95,0x02};
static u8 nt35590_tm_para140[]={0x96,0x75};
static u8 nt35590_tm_para141[]={0x97,0x02};
static u8 nt35590_tm_para142[]={0x98,0xAC};
static u8 nt35590_tm_para143[]={0x99,0x02};
static u8 nt35590_tm_para144[]={0x9A,0xD1};
static u8 nt35590_tm_para145[]={0x9B,0x03};
static u8 nt35590_tm_para146[]={0x9C,0x01};
static u8 nt35590_tm_para147[]={0x9D,0x03};
static u8 nt35590_tm_para148[]={0x9E,0x23};
static u8 nt35590_tm_para149[]={0x9F,0x03};
static u8 nt35590_tm_para150[]={0xA0,0x45};
static u8 nt35590_tm_para151[]={0xA2,0x03};
static u8 nt35590_tm_para152[]={0xA3,0x55};
static u8 nt35590_tm_para153[]={0xA4,0x03};
static u8 nt35590_tm_para154[]={0xA5,0x62};
static u8 nt35590_tm_para155[]={0xA6,0x03};
static u8 nt35590_tm_para156[]={0xA7,0x73};
static u8 nt35590_tm_para157[]={0xA9,0x03};
static u8 nt35590_tm_para158[]={0xAA,0x86};
static u8 nt35590_tm_para159[]={0xAB,0x03};
static u8 nt35590_tm_para160[]={0xAC,0x93};
static u8 nt35590_tm_para161[]={0xAD,0x03};
static u8 nt35590_tm_para162[]={0xAE,0x98};
static u8 nt35590_tm_para163[]={0xAF,0x03};
static u8 nt35590_tm_para164[]={0xB0,0xB6};
static u8 nt35590_tm_para165[]={0xB1,0x03};
static u8 nt35590_tm_para166[]={0xB2,0xC5};
static u8 nt35590_tm_para167[]={0xB3,0X00};
static u8 nt35590_tm_para168[]={0xB4,0X72};
static u8 nt35590_tm_para169[]={0xB5,0x00};
static u8 nt35590_tm_para170[]={0xB6,0x82};
static u8 nt35590_tm_para171[]={0xB7,0x00};
static u8 nt35590_tm_para172[]={0xB8,0xAC};
static u8 nt35590_tm_para173[]={0xB9,0x00};
static u8 nt35590_tm_para174[]={0xBA,0xC8};
static u8 nt35590_tm_para175[]={0xBB,0x00};
static u8 nt35590_tm_para176[]={0xBC,0xE2};
static u8 nt35590_tm_para177[]={0xBD,0x00};
static u8 nt35590_tm_para178[]={0xBE,0xF4};
static u8 nt35590_tm_para179[]={0xBF,0x00};
static u8 nt35590_tm_para180[]={0xC0,0xFE};
static u8 nt35590_tm_para181[]={0xC1,0x01};
static u8 nt35590_tm_para182[]={0xC2,0x08};
static u8 nt35590_tm_para183[]={0xC3,0x01};
static u8 nt35590_tm_para184[]={0xC4,0x16};
static u8 nt35590_tm_para185[]={0xC5,0x01};
static u8 nt35590_tm_para186[]={0xC6,0x44};
static u8 nt35590_tm_para187[]={0xC7,0x01};
static u8 nt35590_tm_para188[]={0xC8,0x64};
static u8 nt35590_tm_para189[]={0xC9,0x01};
static u8 nt35590_tm_para190[]={0xCA,0x9C};
static u8 nt35590_tm_para191[]={0xCB,0x01};
static u8 nt35590_tm_para192[]={0xCC,0xC6};
static u8 nt35590_tm_para193[]={0xCD,0x02};
static u8 nt35590_tm_para194[]={0xCE,0x08};
static u8 nt35590_tm_para195[]={0xCF,0x02};
static u8 nt35590_tm_para196[]={0xD0,0x40};
static u8 nt35590_tm_para197[]={0xD1,0x02};
static u8 nt35590_tm_para198[]={0xD2,0x41};
static u8 nt35590_tm_para199[]={0xD3,0x02};
static u8 nt35590_tm_para200[]={0xD4,0x75};
static u8 nt35590_tm_para201[]={0xD5,0x02};
static u8 nt35590_tm_para202[]={0xD6,0xAC};
static u8 nt35590_tm_para203[]={0xD7,0x02};
static u8 nt35590_tm_para204[]={0xD8,0xD1};
static u8 nt35590_tm_para205[]={0xD9,0x03};
static u8 nt35590_tm_para206[]={0xDA,0x01};
static u8 nt35590_tm_para207[]={0xDB,0x03};
static u8 nt35590_tm_para208[]={0xDC,0x23};
static u8 nt35590_tm_para209[]={0xDD,0x03};
static u8 nt35590_tm_para210[]={0xDE,0x45};
static u8 nt35590_tm_para211[]={0xDF,0x03};
static u8 nt35590_tm_para212[]={0xE0,0x55};
static u8 nt35590_tm_para213[]={0xE1,0x03};
static u8 nt35590_tm_para214[]={0xE2,0x62};
static u8 nt35590_tm_para215[]={0xE3,0x03};
static u8 nt35590_tm_para216[]={0xE4,0x73};
static u8 nt35590_tm_para217[]={0xE5,0x03};
static u8 nt35590_tm_para218[]={0xE6,0x86};
static u8 nt35590_tm_para219[]={0xE7,0x03};
static u8 nt35590_tm_para220[]={0xE8,0x93};
static u8 nt35590_tm_para221[]={0xE9,0x03};
static u8 nt35590_tm_para222[]={0xEA,0x98};
static u8 nt35590_tm_para223[]={0xEB,0x03};
static u8 nt35590_tm_para224[]={0xEC,0xB6};
static u8 nt35590_tm_para225[]={0xED,0x03};
static u8 nt35590_tm_para226[]={0xEE,0xC5};
static u8 nt35590_tm_para227[]={0xEF,0X00};
static u8 nt35590_tm_para228[]={0xF0,0X72};
static u8 nt35590_tm_para229[]={0xF1,0x00};
static u8 nt35590_tm_para230[]={0xF2,0x82};
static u8 nt35590_tm_para231[]={0xF3,0x00};
static u8 nt35590_tm_para232[]={0xF4,0xAC};
static u8 nt35590_tm_para233[]={0xF5,0x00};
static u8 nt35590_tm_para234[]={0xF6,0xC8};
static u8 nt35590_tm_para235[]={0xF7,0x00};
static u8 nt35590_tm_para236[]={0xF8,0xE2};
static u8 nt35590_tm_para237[]={0xF9,0x00};
static u8 nt35590_tm_para238[]={0xFA,0xF4};
static u8 nt35590_tm_para239[]={0xFF,0x00};
static u8 nt35590_tm_para240[]={0xFF,0x02};
static u8 nt35590_tm_para241[]={0x00,0x00};
static u8 nt35590_tm_para242[]={0x01,0xFE};
static u8 nt35590_tm_para243[]={0x02,0x01};
static u8 nt35590_tm_para244[]={0x03,0x08};
static u8 nt35590_tm_para245[]={0x04,0x01};
static u8 nt35590_tm_para246[]={0x05,0x16};
static u8 nt35590_tm_para247[]={0x06,0x01};
static u8 nt35590_tm_para248[]={0x07,0x44};
static u8 nt35590_tm_para249[]={0x08,0x01};
static u8 nt35590_tm_para250[]={0x09,0x64};
static u8 nt35590_tm_para251[]={0x0A,0x01};
static u8 nt35590_tm_para252[]={0x0B,0x9C};
static u8 nt35590_tm_para253[]={0x0C,0x01};
static u8 nt35590_tm_para254[]={0x0D,0xC6};
static u8 nt35590_tm_para255[]={0x0E,0x02};
static u8 nt35590_tm_para256[]={0x0F,0x08};
static u8 nt35590_tm_para257[]={0x10,0x02};
static u8 nt35590_tm_para258[]={0x11,0x40};
static u8 nt35590_tm_para259[]={0x12,0x02};
static u8 nt35590_tm_para260[]={0x13,0x41};
static u8 nt35590_tm_para261[]={0x14,0x02};
static u8 nt35590_tm_para262[]={0x15,0x75};
static u8 nt35590_tm_para263[]={0x16,0x02};
static u8 nt35590_tm_para264[]={0x17,0xAC};
static u8 nt35590_tm_para265[]={0x18,0x02};
static u8 nt35590_tm_para266[]={0x19,0xD1};
static u8 nt35590_tm_para267[]={0x1A,0x03};
static u8 nt35590_tm_para268[]={0x1B,0x01};
static u8 nt35590_tm_para269[]={0x1C,0x03};
static u8 nt35590_tm_para270[]={0x1D,0x23};
static u8 nt35590_tm_para271[]={0x1E,0x03};
static u8 nt35590_tm_para272[]={0x1F,0x45};
static u8 nt35590_tm_para273[]={0x20,0x03};
static u8 nt35590_tm_para274[]={0x21,0x55};
static u8 nt35590_tm_para275[]={0x22,0x03};
static u8 nt35590_tm_para276[]={0x23,0x62};
static u8 nt35590_tm_para277[]={0x24,0x03};
static u8 nt35590_tm_para278[]={0x25,0x73};
static u8 nt35590_tm_para279[]={0x26,0x03};
static u8 nt35590_tm_para280[]={0x27,0x86};
static u8 nt35590_tm_para281[]={0x28,0x03};
static u8 nt35590_tm_para282[]={0x29,0x93};
static u8 nt35590_tm_para283[]={0x2A,0x03};
static u8 nt35590_tm_para284[]={0x2B,0x98};
static u8 nt35590_tm_para285[]={0x2D,0x03};
static u8 nt35590_tm_para286[]={0x2F,0xB6};
static u8 nt35590_tm_para287[]={0x30,0x03};
static u8 nt35590_tm_para288[]={0x31,0xC5};
static u8 nt35590_tm_para289[]={0x32,0X00};
static u8 nt35590_tm_para290[]={0x33,0X72};
static u8 nt35590_tm_para291[]={0x34,0x00};
static u8 nt35590_tm_para292[]={0x35,0x82};
static u8 nt35590_tm_para293[]={0x36,0x00};
static u8 nt35590_tm_para294[]={0x37,0xAC};
static u8 nt35590_tm_para295[]={0x38,0x00};
static u8 nt35590_tm_para296[]={0x39,0xC8};
static u8 nt35590_tm_para297[]={0x3A,0x00};
static u8 nt35590_tm_para298[]={0x3B,0xE2};
static u8 nt35590_tm_para299[]={0x3D,0x00};
static u8 nt35590_tm_para300[]={0x3F,0xF4};
static u8 nt35590_tm_para301[]={0x40,0x00};
static u8 nt35590_tm_para302[]={0x41,0xFE};
static u8 nt35590_tm_para303[]={0X42,0x01};
static u8 nt35590_tm_para304[]={0x43,0x08};
static u8 nt35590_tm_para305[]={0x44,0x01};
static u8 nt35590_tm_para306[]={0x45,0x16};
static u8 nt35590_tm_para307[]={0x46,0x01};
static u8 nt35590_tm_para308[]={0x47,0x44};
static u8 nt35590_tm_para309[]={0x48,0x01};
static u8 nt35590_tm_para310[]={0x49,0x64};
static u8 nt35590_tm_para311[]={0x4A,0x01};
static u8 nt35590_tm_para312[]={0x4B,0x9C};
static u8 nt35590_tm_para313[]={0x4C,0x01};
static u8 nt35590_tm_para314[]={0x4D,0xC6};
static u8 nt35590_tm_para315[]={0x4E,0x02};
static u8 nt35590_tm_para316[]={0x4F,0x08};
static u8 nt35590_tm_para317[]={0x50,0x02};
static u8 nt35590_tm_para318[]={0x51,0x40};
static u8 nt35590_tm_para319[]={0x52,0x02};
static u8 nt35590_tm_para320[]={0x53,0x41};
static u8 nt35590_tm_para321[]={0x54,0x02};
static u8 nt35590_tm_para322[]={0x55,0x75};
static u8 nt35590_tm_para323[]={0x56,0x02};
static u8 nt35590_tm_para324[]={0x58,0xAC};
static u8 nt35590_tm_para325[]={0x59,0x02};
static u8 nt35590_tm_para326[]={0x5A,0xD1};
static u8 nt35590_tm_para327[]={0x5B,0x03};
static u8 nt35590_tm_para328[]={0x5C,0x01};
static u8 nt35590_tm_para329[]={0x5D,0x03};
static u8 nt35590_tm_para330[]={0x5E,0x23};
static u8 nt35590_tm_para331[]={0x5F,0x03};
static u8 nt35590_tm_para332[]={0x60,0x45};
static u8 nt35590_tm_para333[]={0x61,0x03};
static u8 nt35590_tm_para334[]={0x62,0x55};
static u8 nt35590_tm_para335[]={0x63,0x03};
static u8 nt35590_tm_para336[]={0x64,0x62};
static u8 nt35590_tm_para337[]={0x65,0x03};
static u8 nt35590_tm_para338[]={0x66,0x73};
static u8 nt35590_tm_para339[]={0x67,0x03};
static u8 nt35590_tm_para340[]={0x68,0x86};
static u8 nt35590_tm_para341[]={0x69,0x03};
static u8 nt35590_tm_para342[]={0x6A,0x93};
static u8 nt35590_tm_para343[]={0x6B,0x03};
static u8 nt35590_tm_para344[]={0x6C,0x98};
static u8 nt35590_tm_para345[]={0x6D,0x03};
static u8 nt35590_tm_para346[]={0x6E,0xB6};
static u8 nt35590_tm_para347[]={0x6F,0x03};
static u8 nt35590_tm_para348[]={0x70,0xC5};
static u8 nt35590_tm_para349[]={0x71,0X00};
static u8 nt35590_tm_para350[]={0x72,0X72};
static u8 nt35590_tm_para351[]={0x73,0x00};
static u8 nt35590_tm_para352[]={0x74,0x82};
static u8 nt35590_tm_para353[]={0x75,0x00};
static u8 nt35590_tm_para354[]={0x76,0xAC};
static u8 nt35590_tm_para355[]={0x77,0x00};
static u8 nt35590_tm_para356[]={0x78,0xC8};
static u8 nt35590_tm_para357[]={0x79,0x00};
static u8 nt35590_tm_para358[]={0x7A,0xE2};
static u8 nt35590_tm_para359[]={0x7B,0x00};
static u8 nt35590_tm_para360[]={0x7C,0xF4};
static u8 nt35590_tm_para361[]={0x7D,0x00};
static u8 nt35590_tm_para362[]={0x7E,0xFE};
static u8 nt35590_tm_para363[]={0x7F,0x01};
static u8 nt35590_tm_para364[]={0x80,0x08};
static u8 nt35590_tm_para365[]={0x81,0x01};
static u8 nt35590_tm_para366[]={0x82,0x16};
static u8 nt35590_tm_para367[]={0x83,0x01};
static u8 nt35590_tm_para368[]={0x84,0x44};
static u8 nt35590_tm_para369[]={0x85,0x01};
static u8 nt35590_tm_para370[]={0x86,0x64};
static u8 nt35590_tm_para371[]={0x87,0x01};
static u8 nt35590_tm_para372[]={0x88,0x9C};
static u8 nt35590_tm_para373[]={0x89,0x01};
static u8 nt35590_tm_para374[]={0x8A,0xC6};
static u8 nt35590_tm_para375[]={0x8B,0x02};
static u8 nt35590_tm_para376[]={0x8C,0x08};
static u8 nt35590_tm_para377[]={0x8D,0x02};
static u8 nt35590_tm_para378[]={0x8E,0x40};
static u8 nt35590_tm_para379[]={0x8F,0x02};
static u8 nt35590_tm_para380[]={0x90,0x41};
static u8 nt35590_tm_para381[]={0x91,0x02};
static u8 nt35590_tm_para382[]={0x92,0x75};
static u8 nt35590_tm_para383[]={0x93,0x02};
static u8 nt35590_tm_para384[]={0x94,0xAC};
static u8 nt35590_tm_para385[]={0x95,0x02};
static u8 nt35590_tm_para386[]={0x96,0xD1};
static u8 nt35590_tm_para387[]={0x97,0x03};
static u8 nt35590_tm_para388[]={0x98,0x01};
static u8 nt35590_tm_para389[]={0x99,0x03};
static u8 nt35590_tm_para390[]={0x9A,0x23};
static u8 nt35590_tm_para391[]={0x9B,0x03};
static u8 nt35590_tm_para392[]={0x9C,0x45};
static u8 nt35590_tm_para393[]={0x9D,0x03};
static u8 nt35590_tm_para394[]={0x9E,0x55};
static u8 nt35590_tm_para395[]={0x9F,0x03};
static u8 nt35590_tm_para396[]={0xA0,0x62};
static u8 nt35590_tm_para397[]={0xA2,0x03};
static u8 nt35590_tm_para398[]={0xA3,0x73};
static u8 nt35590_tm_para399[]={0xA4,0x03};
static u8 nt35590_tm_para400[]={0xA5,0x86};
static u8 nt35590_tm_para401[]={0xA6,0x03};
static u8 nt35590_tm_para402[]={0xA7,0x93};
static u8 nt35590_tm_para403[]={0xA9,0x03};
static u8 nt35590_tm_para404[]={0xAA,0x98};
static u8 nt35590_tm_para405[]={0xAB,0x03};
static u8 nt35590_tm_para406[]={0xAC,0xB6};
static u8 nt35590_tm_para407[]={0xAD,0x03};
static u8 nt35590_tm_para408[]={0xAE,0xC5};
static u8 nt35590_tm_para409[]={0xAF,0X00};
static u8 nt35590_tm_para410[]={0xB0,0X72};
static u8 nt35590_tm_para411[]={0xB1,0x00};
static u8 nt35590_tm_para412[]={0xB2,0x82};
static u8 nt35590_tm_para413[]={0xB3,0x00};
static u8 nt35590_tm_para414[]={0xB4,0xAC};
static u8 nt35590_tm_para415[]={0xB5,0x00};
static u8 nt35590_tm_para416[]={0xB6,0xC8};
static u8 nt35590_tm_para417[]={0xB7,0x00};
static u8 nt35590_tm_para418[]={0xB8,0xE2};
static u8 nt35590_tm_para419[]={0xB9,0x00};
static u8 nt35590_tm_para420[]={0xBA,0xF4};
static u8 nt35590_tm_para421[]={0xBB,0x00};
static u8 nt35590_tm_para422[]={0xBC,0xFE};
static u8 nt35590_tm_para423[]={0xBD,0x01};
static u8 nt35590_tm_para424[]={0xBE,0x08};
static u8 nt35590_tm_para425[]={0xBF,0x01};
static u8 nt35590_tm_para426[]={0xC0,0x16};
static u8 nt35590_tm_para427[]={0xC1,0x01};
static u8 nt35590_tm_para428[]={0xC2,0x44};
static u8 nt35590_tm_para429[]={0xC3,0x01};
static u8 nt35590_tm_para430[]={0xC4,0x64};
static u8 nt35590_tm_para431[]={0xC5,0x01};
static u8 nt35590_tm_para432[]={0xC6,0x9C};
static u8 nt35590_tm_para433[]={0xC7,0x01};
static u8 nt35590_tm_para434[]={0xC8,0xC6};
static u8 nt35590_tm_para435[]={0xC9,0x02};
static u8 nt35590_tm_para436[]={0xCA,0x08};
static u8 nt35590_tm_para437[]={0xCB,0x02};
static u8 nt35590_tm_para438[]={0xCC,0x40};
static u8 nt35590_tm_para439[]={0xCD,0x02};
static u8 nt35590_tm_para440[]={0xCE,0x41};
static u8 nt35590_tm_para441[]={0xCF,0x02};
static u8 nt35590_tm_para442[]={0xD0,0x75};
static u8 nt35590_tm_para443[]={0xD1,0x02};
static u8 nt35590_tm_para444[]={0xD2,0xAC};
static u8 nt35590_tm_para445[]={0xD3,0x02};
static u8 nt35590_tm_para446[]={0xD4,0xD1};
static u8 nt35590_tm_para447[]={0xD5,0x03};
static u8 nt35590_tm_para448[]={0xD6,0x01};
static u8 nt35590_tm_para449[]={0xD7,0x03};
static u8 nt35590_tm_para450[]={0xD8,0x23};
static u8 nt35590_tm_para451[]={0xD9,0x03};
static u8 nt35590_tm_para452[]={0xDA,0x45};
static u8 nt35590_tm_para453[]={0xDB,0x03};
static u8 nt35590_tm_para454[]={0xDC,0x55};
static u8 nt35590_tm_para455[]={0xDD,0x03};
static u8 nt35590_tm_para456[]={0xDE,0x62};
static u8 nt35590_tm_para457[]={0xDF,0x03};
static u8 nt35590_tm_para458[]={0xE0,0x73};
static u8 nt35590_tm_para459[]={0xE1,0x03};
static u8 nt35590_tm_para460[]={0xE2,0x86};
static u8 nt35590_tm_para461[]={0xE3,0x03};
static u8 nt35590_tm_para462[]={0xE4,0x93};
static u8 nt35590_tm_para463[]={0xE5,0x03};
static u8 nt35590_tm_para464[]={0xE6,0x98};
static u8 nt35590_tm_para465[]={0xE7,0x03};
static u8 nt35590_tm_para466[]={0xE8,0xB6};
static u8 nt35590_tm_para467[]={0xE9,0x03};
static u8 nt35590_tm_para468[]={0xEA,0xC5};
static u8 nt35590_tm_para469[]={0xFF,0x03};
static u8 nt35590_tm_para470[]={0x18,0x00};
static u8 nt35590_tm_para471[]={0x19,0x00};
static u8 nt35590_tm_para472[]={0x1A,0x00};
static u8 nt35590_tm_para473[]={0x25,0x26};
static u8 nt35590_tm_para474[]={0x00,0x00};
static u8 nt35590_tm_para475[]={0x01,0x0C};
static u8 nt35590_tm_para476[]={0x02,0x10};
static u8 nt35590_tm_para477[]={0x03,0x14};
static u8 nt35590_tm_para478[]={0x04,0x18};
static u8 nt35590_tm_para479[]={0x05,0x1C};
static u8 nt35590_tm_para480[]={0x06,0x20};
static u8 nt35590_tm_para481[]={0x07,0x24};
static u8 nt35590_tm_para482[]={0x08,0x28};
static u8 nt35590_tm_para483[]={0x09,0x2C};
static u8 nt35590_tm_para484[]={0x0A,0x30};
static u8 nt35590_tm_para485[]={0x0B,0x34};
static u8 nt35590_tm_para486[]={0x0C,0x38};
static u8 nt35590_tm_para487[]={0x0D,0x3F};
static u8 nt35590_tm_para488[]={0x0E,0x3F};
static u8 nt35590_tm_para489[]={0x0F,0x3F};
static u8 nt35590_tm_para490[]={0xFB,0x01};
static u8 nt35590_tm_para491[]={0xFF,0x00};
static u8 nt35590_tm_para492[]={0x51,0xFF};
static u8 nt35590_tm_para493[]={0x53,0x2C};
static u8 nt35590_tm_para494[]={0x55,0x80};
static u8 nt35590_tm_para495[]={0xFF,0x00};
static u8 nt35590_tm_para496[]={0xFB,0x01};
static u8 nt35590_tm_para497[]={0xC2,0x08};
static u8 nt35590_tm_para498[]={0xBA,0x03};

#define PANEL_NAME "TIANMA"

static
int mdfld_nt35590_tm_drv_ic_init(struct mdfld_dsi_config *dsi_config)
{
	struct drm_device *dev = dsi_config->dev;
	struct mdfld_dsi_pkg_sender *sender
			= mdfld_dsi_get_pkg_sender(dsi_config);

	if (!sender)
		return -EINVAL;

	PSB_DEBUG_ENTRY("\n");

	sender->status = MDFLD_DSI_PKG_SENDER_FREE;

	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para1[0],nt35590_tm_para1[1],1,0);
	mdfld_dsi_send_mcs_long_lp(sender,nt35590_tm_para2,sizeof(nt35590_tm_para2),0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para3[0],nt35590_tm_para3[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para4[0],nt35590_tm_para4[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para5[0],nt35590_tm_para5[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para6[0],nt35590_tm_para6[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para7[0],nt35590_tm_para7[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para8[0],nt35590_tm_para8[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para9[0],nt35590_tm_para9[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para10[0],nt35590_tm_para10[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para11[0],nt35590_tm_para11[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para12[0],nt35590_tm_para12[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para13[0],nt35590_tm_para13[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para14[0],nt35590_tm_para14[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para15[0],nt35590_tm_para15[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para16[0],nt35590_tm_para16[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para17[0],nt35590_tm_para17[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para18[0],nt35590_tm_para18[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para19[0],nt35590_tm_para19[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para20[0],nt35590_tm_para20[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para21[0],nt35590_tm_para21[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para22[0],nt35590_tm_para22[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para23[0],nt35590_tm_para23[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para24[0],nt35590_tm_para24[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para25[0],nt35590_tm_para25[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para26[0],nt35590_tm_para26[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para27[0],nt35590_tm_para27[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para28[0],nt35590_tm_para28[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para29[0],nt35590_tm_para29[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para30[0],nt35590_tm_para30[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para31[0],nt35590_tm_para31[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para32[0],nt35590_tm_para32[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para33[0],nt35590_tm_para33[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para34[0],nt35590_tm_para34[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para35[0],nt35590_tm_para35[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para36[0],nt35590_tm_para36[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para37[0],nt35590_tm_para37[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para38[0],nt35590_tm_para38[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para39[0],nt35590_tm_para39[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para40[0],nt35590_tm_para40[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para41[0],nt35590_tm_para41[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para42[0],nt35590_tm_para42[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para43[0],nt35590_tm_para43[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para44[0],nt35590_tm_para44[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para45[0],nt35590_tm_para45[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para46[0],nt35590_tm_para46[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para47[0],nt35590_tm_para47[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para48[0],nt35590_tm_para48[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para49[0],nt35590_tm_para49[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para50[0],nt35590_tm_para50[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para51[0],nt35590_tm_para51[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para52[0],nt35590_tm_para52[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para53[0],nt35590_tm_para53[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para54[0],nt35590_tm_para54[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para55[0],nt35590_tm_para55[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para56[0],nt35590_tm_para56[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para57[0],nt35590_tm_para57[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para58[0],nt35590_tm_para58[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para59[0],nt35590_tm_para59[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para60[0],nt35590_tm_para60[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para61[0],nt35590_tm_para61[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para62[0],nt35590_tm_para62[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para63[0],nt35590_tm_para63[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para64[0],nt35590_tm_para64[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para65[0],nt35590_tm_para65[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para66[0],nt35590_tm_para66[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para67[0],nt35590_tm_para67[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para68[0],nt35590_tm_para68[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para69[0],nt35590_tm_para69[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para70[0],nt35590_tm_para70[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para71[0],nt35590_tm_para71[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para72[0],nt35590_tm_para72[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para73[0],nt35590_tm_para73[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para74[0],nt35590_tm_para74[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para75[0],nt35590_tm_para75[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para76[0],nt35590_tm_para76[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para77[0],nt35590_tm_para77[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para78[0],nt35590_tm_para78[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para79[0],nt35590_tm_para79[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para80[0],nt35590_tm_para80[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para81[0],nt35590_tm_para81[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para82[0],nt35590_tm_para82[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para83[0],nt35590_tm_para83[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para84[0],nt35590_tm_para84[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para85[0],nt35590_tm_para85[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para86[0],nt35590_tm_para86[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para87[0],nt35590_tm_para87[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para88[0],nt35590_tm_para88[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para89[0],nt35590_tm_para89[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para90[0],nt35590_tm_para90[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para91[0],nt35590_tm_para91[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para92[0],nt35590_tm_para92[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para93[0],nt35590_tm_para93[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para94[0],nt35590_tm_para94[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para95[0],nt35590_tm_para95[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para96[0],nt35590_tm_para96[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para97[0],nt35590_tm_para97[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para98[0],nt35590_tm_para98[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para99[0],nt35590_tm_para99[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para100[0],nt35590_tm_para100[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para101[0],nt35590_tm_para101[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para102[0],nt35590_tm_para102[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para103[0],nt35590_tm_para103[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para104[0],nt35590_tm_para104[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para105[0],nt35590_tm_para105[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para106[0],nt35590_tm_para106[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para107[0],nt35590_tm_para107[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para108[0],nt35590_tm_para108[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para109[0],nt35590_tm_para109[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para110[0],nt35590_tm_para110[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para111[0],nt35590_tm_para111[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para112[0],nt35590_tm_para112[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para113[0],nt35590_tm_para113[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para114[0],nt35590_tm_para114[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para115[0],nt35590_tm_para115[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para116[0],nt35590_tm_para116[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para117[0],nt35590_tm_para117[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para118[0],nt35590_tm_para118[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para119[0],nt35590_tm_para119[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para120[0],nt35590_tm_para120[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para121[0],nt35590_tm_para121[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para122[0],nt35590_tm_para122[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para123[0],nt35590_tm_para123[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para124[0],nt35590_tm_para124[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para125[0],nt35590_tm_para125[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para126[0],nt35590_tm_para126[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para127[0],nt35590_tm_para127[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para128[0],nt35590_tm_para128[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para129[0],nt35590_tm_para129[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para130[0],nt35590_tm_para130[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para131[0],nt35590_tm_para131[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para132[0],nt35590_tm_para132[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para133[0],nt35590_tm_para133[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para134[0],nt35590_tm_para134[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para135[0],nt35590_tm_para135[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para136[0],nt35590_tm_para136[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para137[0],nt35590_tm_para137[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para138[0],nt35590_tm_para138[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para139[0],nt35590_tm_para139[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para140[0],nt35590_tm_para140[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para141[0],nt35590_tm_para141[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para142[0],nt35590_tm_para142[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para143[0],nt35590_tm_para143[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para144[0],nt35590_tm_para144[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para145[0],nt35590_tm_para145[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para146[0],nt35590_tm_para146[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para147[0],nt35590_tm_para147[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para148[0],nt35590_tm_para148[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para149[0],nt35590_tm_para149[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para150[0],nt35590_tm_para150[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para151[0],nt35590_tm_para151[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para152[0],nt35590_tm_para152[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para153[0],nt35590_tm_para153[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para154[0],nt35590_tm_para154[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para155[0],nt35590_tm_para155[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para156[0],nt35590_tm_para156[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para157[0],nt35590_tm_para157[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para158[0],nt35590_tm_para158[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para159[0],nt35590_tm_para159[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para160[0],nt35590_tm_para160[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para161[0],nt35590_tm_para161[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para162[0],nt35590_tm_para162[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para163[0],nt35590_tm_para163[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para164[0],nt35590_tm_para164[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para165[0],nt35590_tm_para165[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para166[0],nt35590_tm_para166[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para167[0],nt35590_tm_para167[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para168[0],nt35590_tm_para168[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para169[0],nt35590_tm_para169[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para170[0],nt35590_tm_para170[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para171[0],nt35590_tm_para171[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para172[0],nt35590_tm_para172[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para173[0],nt35590_tm_para173[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para174[0],nt35590_tm_para174[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para175[0],nt35590_tm_para175[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para176[0],nt35590_tm_para176[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para177[0],nt35590_tm_para177[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para178[0],nt35590_tm_para178[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para179[0],nt35590_tm_para179[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para180[0],nt35590_tm_para180[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para181[0],nt35590_tm_para181[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para182[0],nt35590_tm_para182[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para183[0],nt35590_tm_para183[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para184[0],nt35590_tm_para184[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para185[0],nt35590_tm_para185[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para186[0],nt35590_tm_para186[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para187[0],nt35590_tm_para187[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para188[0],nt35590_tm_para188[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para189[0],nt35590_tm_para189[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para190[0],nt35590_tm_para190[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para191[0],nt35590_tm_para191[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para192[0],nt35590_tm_para192[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para193[0],nt35590_tm_para193[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para194[0],nt35590_tm_para194[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para195[0],nt35590_tm_para195[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para196[0],nt35590_tm_para196[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para197[0],nt35590_tm_para197[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para198[0],nt35590_tm_para198[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para199[0],nt35590_tm_para199[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para200[0],nt35590_tm_para200[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para201[0],nt35590_tm_para201[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para202[0],nt35590_tm_para202[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para203[0],nt35590_tm_para203[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para204[0],nt35590_tm_para204[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para205[0],nt35590_tm_para205[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para206[0],nt35590_tm_para206[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para207[0],nt35590_tm_para207[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para208[0],nt35590_tm_para208[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para209[0],nt35590_tm_para209[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para210[0],nt35590_tm_para210[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para211[0],nt35590_tm_para211[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para212[0],nt35590_tm_para212[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para213[0],nt35590_tm_para213[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para214[0],nt35590_tm_para214[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para215[0],nt35590_tm_para215[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para216[0],nt35590_tm_para216[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para217[0],nt35590_tm_para217[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para218[0],nt35590_tm_para218[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para219[0],nt35590_tm_para219[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para220[0],nt35590_tm_para220[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para221[0],nt35590_tm_para221[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para222[0],nt35590_tm_para222[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para223[0],nt35590_tm_para223[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para224[0],nt35590_tm_para224[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para225[0],nt35590_tm_para225[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para226[0],nt35590_tm_para226[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para227[0],nt35590_tm_para227[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para228[0],nt35590_tm_para228[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para229[0],nt35590_tm_para229[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para230[0],nt35590_tm_para230[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para231[0],nt35590_tm_para231[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para232[0],nt35590_tm_para232[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para233[0],nt35590_tm_para233[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para234[0],nt35590_tm_para234[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para235[0],nt35590_tm_para235[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para236[0],nt35590_tm_para236[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para237[0],nt35590_tm_para237[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para238[0],nt35590_tm_para238[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para239[0],nt35590_tm_para239[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para240[0],nt35590_tm_para240[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para241[0],nt35590_tm_para241[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para242[0],nt35590_tm_para242[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para243[0],nt35590_tm_para243[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para244[0],nt35590_tm_para244[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para245[0],nt35590_tm_para245[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para246[0],nt35590_tm_para246[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para247[0],nt35590_tm_para247[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para248[0],nt35590_tm_para248[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para249[0],nt35590_tm_para249[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para250[0],nt35590_tm_para250[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para251[0],nt35590_tm_para251[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para252[0],nt35590_tm_para252[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para253[0],nt35590_tm_para253[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para254[0],nt35590_tm_para254[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para255[0],nt35590_tm_para255[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para256[0],nt35590_tm_para256[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para257[0],nt35590_tm_para257[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para258[0],nt35590_tm_para258[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para259[0],nt35590_tm_para259[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para260[0],nt35590_tm_para260[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para261[0],nt35590_tm_para261[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para262[0],nt35590_tm_para262[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para263[0],nt35590_tm_para263[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para264[0],nt35590_tm_para264[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para265[0],nt35590_tm_para265[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para266[0],nt35590_tm_para266[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para267[0],nt35590_tm_para267[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para268[0],nt35590_tm_para268[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para269[0],nt35590_tm_para269[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para270[0],nt35590_tm_para270[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para271[0],nt35590_tm_para271[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para272[0],nt35590_tm_para272[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para273[0],nt35590_tm_para273[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para274[0],nt35590_tm_para274[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para275[0],nt35590_tm_para275[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para276[0],nt35590_tm_para276[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para277[0],nt35590_tm_para277[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para278[0],nt35590_tm_para278[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para279[0],nt35590_tm_para279[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para280[0],nt35590_tm_para280[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para281[0],nt35590_tm_para281[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para282[0],nt35590_tm_para282[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para283[0],nt35590_tm_para283[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para284[0],nt35590_tm_para284[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para285[0],nt35590_tm_para285[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para286[0],nt35590_tm_para286[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para287[0],nt35590_tm_para287[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para288[0],nt35590_tm_para288[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para289[0],nt35590_tm_para289[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para290[0],nt35590_tm_para290[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para291[0],nt35590_tm_para291[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para292[0],nt35590_tm_para292[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para293[0],nt35590_tm_para293[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para294[0],nt35590_tm_para294[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para295[0],nt35590_tm_para295[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para296[0],nt35590_tm_para296[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para297[0],nt35590_tm_para297[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para298[0],nt35590_tm_para298[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para299[0],nt35590_tm_para299[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para300[0],nt35590_tm_para300[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para301[0],nt35590_tm_para301[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para302[0],nt35590_tm_para302[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para303[0],nt35590_tm_para303[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para304[0],nt35590_tm_para304[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para305[0],nt35590_tm_para305[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para306[0],nt35590_tm_para306[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para307[0],nt35590_tm_para307[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para308[0],nt35590_tm_para308[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para309[0],nt35590_tm_para309[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para310[0],nt35590_tm_para310[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para311[0],nt35590_tm_para311[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para312[0],nt35590_tm_para312[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para313[0],nt35590_tm_para313[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para314[0],nt35590_tm_para314[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para315[0],nt35590_tm_para315[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para316[0],nt35590_tm_para316[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para317[0],nt35590_tm_para317[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para318[0],nt35590_tm_para318[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para319[0],nt35590_tm_para319[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para320[0],nt35590_tm_para320[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para321[0],nt35590_tm_para321[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para322[0],nt35590_tm_para322[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para323[0],nt35590_tm_para323[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para324[0],nt35590_tm_para324[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para325[0],nt35590_tm_para325[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para326[0],nt35590_tm_para326[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para327[0],nt35590_tm_para327[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para328[0],nt35590_tm_para328[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para329[0],nt35590_tm_para329[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para330[0],nt35590_tm_para330[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para331[0],nt35590_tm_para331[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para332[0],nt35590_tm_para332[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para333[0],nt35590_tm_para333[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para334[0],nt35590_tm_para334[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para335[0],nt35590_tm_para335[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para336[0],nt35590_tm_para336[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para337[0],nt35590_tm_para337[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para338[0],nt35590_tm_para338[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para339[0],nt35590_tm_para339[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para340[0],nt35590_tm_para340[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para341[0],nt35590_tm_para341[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para342[0],nt35590_tm_para342[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para343[0],nt35590_tm_para343[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para344[0],nt35590_tm_para344[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para345[0],nt35590_tm_para345[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para346[0],nt35590_tm_para346[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para347[0],nt35590_tm_para347[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para348[0],nt35590_tm_para348[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para349[0],nt35590_tm_para349[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para350[0],nt35590_tm_para350[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para351[0],nt35590_tm_para351[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para352[0],nt35590_tm_para352[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para353[0],nt35590_tm_para353[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para354[0],nt35590_tm_para354[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para355[0],nt35590_tm_para355[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para356[0],nt35590_tm_para356[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para357[0],nt35590_tm_para357[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para358[0],nt35590_tm_para358[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para359[0],nt35590_tm_para359[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para360[0],nt35590_tm_para360[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para361[0],nt35590_tm_para361[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para362[0],nt35590_tm_para362[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para363[0],nt35590_tm_para363[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para364[0],nt35590_tm_para364[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para365[0],nt35590_tm_para365[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para366[0],nt35590_tm_para366[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para367[0],nt35590_tm_para367[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para368[0],nt35590_tm_para368[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para369[0],nt35590_tm_para369[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para370[0],nt35590_tm_para370[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para371[0],nt35590_tm_para371[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para372[0],nt35590_tm_para372[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para373[0],nt35590_tm_para373[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para374[0],nt35590_tm_para374[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para375[0],nt35590_tm_para375[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para376[0],nt35590_tm_para376[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para377[0],nt35590_tm_para377[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para378[0],nt35590_tm_para378[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para379[0],nt35590_tm_para379[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para380[0],nt35590_tm_para380[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para381[0],nt35590_tm_para381[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para382[0],nt35590_tm_para382[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para383[0],nt35590_tm_para383[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para384[0],nt35590_tm_para384[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para385[0],nt35590_tm_para385[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para386[0],nt35590_tm_para386[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para387[0],nt35590_tm_para387[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para388[0],nt35590_tm_para388[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para389[0],nt35590_tm_para389[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para390[0],nt35590_tm_para390[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para391[0],nt35590_tm_para391[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para392[0],nt35590_tm_para392[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para393[0],nt35590_tm_para393[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para394[0],nt35590_tm_para394[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para395[0],nt35590_tm_para395[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para396[0],nt35590_tm_para396[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para397[0],nt35590_tm_para397[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para398[0],nt35590_tm_para398[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para399[0],nt35590_tm_para399[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para400[0],nt35590_tm_para400[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para401[0],nt35590_tm_para401[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para402[0],nt35590_tm_para402[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para403[0],nt35590_tm_para403[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para404[0],nt35590_tm_para404[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para405[0],nt35590_tm_para405[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para406[0],nt35590_tm_para406[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para407[0],nt35590_tm_para407[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para408[0],nt35590_tm_para408[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para409[0],nt35590_tm_para409[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para410[0],nt35590_tm_para410[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para411[0],nt35590_tm_para411[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para412[0],nt35590_tm_para412[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para413[0],nt35590_tm_para413[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para414[0],nt35590_tm_para414[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para415[0],nt35590_tm_para415[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para416[0],nt35590_tm_para416[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para417[0],nt35590_tm_para417[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para418[0],nt35590_tm_para418[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para419[0],nt35590_tm_para419[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para420[0],nt35590_tm_para420[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para421[0],nt35590_tm_para421[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para422[0],nt35590_tm_para422[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para423[0],nt35590_tm_para423[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para424[0],nt35590_tm_para424[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para425[0],nt35590_tm_para425[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para426[0],nt35590_tm_para426[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para427[0],nt35590_tm_para427[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para428[0],nt35590_tm_para428[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para429[0],nt35590_tm_para429[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para430[0],nt35590_tm_para430[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para431[0],nt35590_tm_para431[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para432[0],nt35590_tm_para432[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para433[0],nt35590_tm_para433[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para434[0],nt35590_tm_para434[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para435[0],nt35590_tm_para435[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para436[0],nt35590_tm_para436[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para437[0],nt35590_tm_para437[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para438[0],nt35590_tm_para438[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para439[0],nt35590_tm_para439[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para440[0],nt35590_tm_para440[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para441[0],nt35590_tm_para441[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para442[0],nt35590_tm_para442[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para443[0],nt35590_tm_para443[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para444[0],nt35590_tm_para444[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para445[0],nt35590_tm_para445[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para446[0],nt35590_tm_para446[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para447[0],nt35590_tm_para447[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para448[0],nt35590_tm_para448[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para449[0],nt35590_tm_para449[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para450[0],nt35590_tm_para450[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para451[0],nt35590_tm_para451[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para452[0],nt35590_tm_para452[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para453[0],nt35590_tm_para453[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para454[0],nt35590_tm_para454[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para455[0],nt35590_tm_para455[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para456[0],nt35590_tm_para456[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para457[0],nt35590_tm_para457[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para458[0],nt35590_tm_para458[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para459[0],nt35590_tm_para459[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para460[0],nt35590_tm_para460[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para461[0],nt35590_tm_para461[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para462[0],nt35590_tm_para462[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para463[0],nt35590_tm_para463[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para464[0],nt35590_tm_para464[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para465[0],nt35590_tm_para465[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para466[0],nt35590_tm_para466[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para467[0],nt35590_tm_para467[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para468[0],nt35590_tm_para468[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para469[0],nt35590_tm_para469[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para470[0],nt35590_tm_para470[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para471[0],nt35590_tm_para471[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para472[0],nt35590_tm_para472[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para473[0],nt35590_tm_para473[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para474[0],nt35590_tm_para474[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para475[0],nt35590_tm_para475[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para476[0],nt35590_tm_para476[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para477[0],nt35590_tm_para477[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para478[0],nt35590_tm_para478[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para479[0],nt35590_tm_para479[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para480[0],nt35590_tm_para480[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para481[0],nt35590_tm_para481[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para482[0],nt35590_tm_para482[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para483[0],nt35590_tm_para483[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para484[0],nt35590_tm_para484[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para485[0],nt35590_tm_para485[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para486[0],nt35590_tm_para486[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para487[0],nt35590_tm_para487[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para488[0],nt35590_tm_para488[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para489[0],nt35590_tm_para489[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para490[0],nt35590_tm_para490[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para491[0],nt35590_tm_para491[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para492[0],nt35590_tm_para492[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para493[0],nt35590_tm_para493[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para494[0],nt35590_tm_para494[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para495[0],nt35590_tm_para495[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para496[0],nt35590_tm_para496[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para497[0],nt35590_tm_para497[1],1,0);
	mdfld_dsi_send_mcs_short_lp(sender,nt35590_tm_para498[0],nt35590_tm_para498[1],1,0);

#ifdef CONFIG_BL_LCD_PWM_CONTROL
	mdfld_dsi_send_mcs_short_lp(sender, 0xFF, 0x04, 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, 0x0A, 0x02, 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, 0xFF, 0x00, 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, 0x55, 0xB0, 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, 0x53, 0x24, 1, 0);
	mdfld_dsi_send_mcs_short_lp(sender, 0x51, 0x00, 1, 0);
#endif

	return 0;
}

static
void mdfld_nt35590_tm_dsi_controller_init(struct mdfld_dsi_config *dsi_config)
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
	hw_ctx->dbi_bw_ctrl = 820;//1024;
	hw_ctx->dphy_param = 0x1f1f3610;//0x20124E1A;
	hw_ctx->dsi_func_prg = (0xa000 | dsi_config->lane_count);
	hw_ctx->mipi = TE_TRIGGER_GPIO_PIN;
	hw_ctx->mipi |= dsi_config->lane_config;
}

static
struct drm_display_mode *nt35590_tm_cmd_get_config_mode(void)
{
	struct drm_display_mode *mode;

	PSB_DEBUG_ENTRY("\n");

	mode = kzalloc(sizeof(*mode), GFP_KERNEL);
	if (!mode)
		return NULL;

	mode->hdisplay = 720;
	mode->hsync_start = mode->hdisplay+60;
	mode->hsync_end = mode->hsync_start + 10;
	mode->htotal = mode->hsync_end+60;
	mode->vdisplay = 1280;
	mode->vsync_start = mode->vdisplay+8;
	mode->vsync_end = mode->vsync_start+3;
	mode->vtotal = mode->vsync_end+11;
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
int mdfld_dsi_nt35590_tm_cmd_power_on(struct mdfld_dsi_config *dsi_config)
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
	msleep(20);

power_err:
	return err;
}

static int mdfld_dsi_nt35590_tm_cmd_power_off(struct mdfld_dsi_config *dsi_config)
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
	err = mdfld_dsi_send_mcs_short_lp(sender, 0x28, 0x00, 0, 0);
	if (err) {
		DRM_ERROR("sent set_display_off faild\n");
		goto out;
	}
	mdelay(50);
	/*set tear off */
	err = mdfld_dsi_send_mcs_short_lp(sender, 0x34, 0x00, 1, 0);
	if (err) {
		DRM_ERROR("sent set_tear_off faild\n");
		goto out;
	}

	/*Enter sleep mode */
	err = mdfld_dsi_send_mcs_short_lp(sender, 0x10, 0x00, 0, 0);
	if (err) {
		DRM_ERROR("DCS 0x%x sent failed\n", enter_sleep_mode);
		goto out;
	}
	mdelay(200);
out:
	return err;
}

static
void nt35590_tm_cmd_get_panel_info(int pipe, struct panel_info *pi)
{
	PSB_DEBUG_ENTRY("\n");

	if (pipe == 0) {
		pi->width_mm = PANEL_4DOT3_WIDTH;
		pi->height_mm = PANEL_4DOT3_HEIGHT;
	}
}

static
int mdfld_dsi_nt35590_tm_cmd_detect(struct mdfld_dsi_config *dsi_config)
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
int mdfld_dsi_nt35590_tm_cmd_set_brightness(struct mdfld_dsi_config *dsi_config,
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
int mdfld_dsi_nt35590_tm_cmd_panel_reset(struct mdfld_dsi_config *dsi_config)
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

		gpio_direction_output(mipi_reset_gpio, 1);
		mdelay(5);
	}

	gpio_set_value_cansleep(mipi_reset_gpio, 0);
	mdelay(11);

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

void nt35590_tm_cmd_init(struct drm_device *dev, struct panel_funcs *p_funcs)
{
	int ena_err;

	if (!dev || !p_funcs) {
		DRM_ERROR("Invalid parameters\n");
		return;
	}

	PSB_DEBUG_ENTRY("\n");

	p_funcs->get_config_mode = nt35590_tm_cmd_get_config_mode;
	p_funcs->get_panel_info = nt35590_tm_cmd_get_panel_info;
	p_funcs->reset = mdfld_dsi_nt35590_tm_cmd_panel_reset;
	p_funcs->drv_ic_init = mdfld_nt35590_tm_drv_ic_init;
	p_funcs->dsi_controller_init = mdfld_nt35590_tm_dsi_controller_init;
	p_funcs->detect = mdfld_dsi_nt35590_tm_cmd_detect;
	p_funcs->power_on = mdfld_dsi_nt35590_tm_cmd_power_on;
	p_funcs->power_off = mdfld_dsi_nt35590_tm_cmd_power_off;
	p_funcs->set_brightness = mdfld_dsi_nt35590_tm_cmd_set_brightness;

#ifdef CONFIG_BL_PMU_PWM_CONTROL
	nt35590_brightness_init(dev);
#endif
}

static int nt35590_tm_cmd_probe(struct platform_device *pdev)
{
	int ret = 0;

	DRM_INFO("%s: NT35590 panel detected\n", __func__);
	intel_mid_panel_register(nt35590_tm_cmd_init);

	add_panel_config_prop(PANEL_NAME, "NT35590", 1280, 720);
	create_backlight_debug_file();
	return 0;
}

static struct platform_driver nt35590_tm_lcd_driver = {
	.probe	= nt35590_tm_cmd_probe,
	.driver	= {
		.name	= "NT35590TM CMD RH",
		.owner	= THIS_MODULE,
	},
};

static int __init nt35590_tm_lcd_init(void)
{
	DRM_INFO("%s\n", __func__);

	return platform_driver_register(&nt35590_tm_lcd_driver);
}
module_init(nt35590_tm_lcd_init);
