#include <linux/atomisp_platform.h>
#include <linux/atomisp.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/videodev2.h>
#include <linux/v4l2-mediabus.h>
#include <linux/types.h>
#include <media/media-entity.h>
#include <media/v4l2-chip-ident.h>
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>

#define	OV9740_NAME	"ov9740"
#define MAX_FMTS		1

/* General Status Registers */
#define OV9740_MODEL_ID_HI		0x0000
#define OV9740_MODEL_ID_LO		0x0001
#define OV9740_REVISION_NUMBER		0x0002
#define OV9740_MANUFACTURER_ID		0x0003
#define OV9740_SMIA_VERSION		0x0004

/* General Setup Registers */
#define OV9740_MODE_SELECT		0x0100
#define OV9740_IMAGE_ORT		0x0101
#define OV9740_SOFTWARE_RESET		0x0103
#define OV9740_GRP_PARAM_HOLD		0x0104
#define OV9740_MSK_CORRUP_FM		0x0105

/* Timing Setting */
#define OV9740_FRM_LENGTH_LN_HI		0x0340 /* VTS */
#define OV9740_FRM_LENGTH_LN_LO		0x0341 /* VTS */
#define OV9740_LN_LENGTH_PCK_HI		0x0342 /* HTS */
#define OV9740_LN_LENGTH_PCK_LO		0x0343 /* HTS */
#define OV9740_X_ADDR_START_HI		0x0344
#define OV9740_X_ADDR_START_LO		0x0345
#define OV9740_Y_ADDR_START_HI		0x0346
#define OV9740_Y_ADDR_START_LO		0x0347
#define OV9740_X_ADDR_END_HI		0x0348
#define OV9740_X_ADDR_END_LO		0x0349
#define OV9740_Y_ADDR_END_HI		0x034A
#define OV9740_Y_ADDR_END_LO		0x034B
#define OV9740_X_OUTPUT_SIZE_HI		0x034C
#define OV9740_X_OUTPUT_SIZE_LO		0x034D
#define OV9740_Y_OUTPUT_SIZE_HI		0x034E
#define OV9740_Y_OUTPUT_SIZE_LO		0x034F

/* IO Control Registers */
#define OV9740_IO_CREL00		0x3002
#define OV9740_IO_CREL01		0x3004
#define OV9740_IO_CREL02		0x3005
#define OV9740_IO_OUTPUT_SEL01		0x3026
#define OV9740_IO_OUTPUT_SEL02		0x3027

/* AWB Registers */
#define OV9740_AWB_MANUAL_CTRL		0x3406

/* Analog Control Registers */
#define OV9740_ANALOG_CTRL01		0x3601
#define OV9740_ANALOG_CTRL02		0x3602
#define OV9740_ANALOG_CTRL03		0x3603
#define OV9740_ANALOG_CTRL04		0x3604
#define OV9740_ANALOG_CTRL10		0x3610
#define OV9740_ANALOG_CTRL12		0x3612
#define OV9740_ANALOG_CTRL20		0x3620
#define OV9740_ANALOG_CTRL21		0x3621
#define OV9740_ANALOG_CTRL22		0x3622
#define OV9740_ANALOG_CTRL30		0x3630
#define OV9740_ANALOG_CTRL31		0x3631
#define OV9740_ANALOG_CTRL32		0x3632
#define OV9740_ANALOG_CTRL33		0x3633

/* Sensor Control */
#define OV9740_SENSOR_CTRL03		0x3703
#define OV9740_SENSOR_CTRL04		0x3704
#define OV9740_SENSOR_CTRL05		0x3705
#define OV9740_SENSOR_CTRL07		0x3707

/* Timing Control */
#define OV9740_TIMING_CTRL17		0x3817
#define OV9740_TIMING_CTRL19		0x3819
#define OV9740_TIMING_CTRL33		0x3833
#define OV9740_TIMING_CTRL35		0x3835

/* Banding Filter */
#define OV9740_AEC_MAXEXPO_60_H		0x3A02
#define OV9740_AEC_MAXEXPO_60_L		0x3A03
#define OV9740_AEC_B50_STEP_HI		0x3A08
#define OV9740_AEC_B50_STEP_LO		0x3A09
#define OV9740_AEC_B60_STEP_HI		0x3A0A
#define OV9740_AEC_B60_STEP_LO		0x3A0B
#define OV9740_AEC_CTRL0D		0x3A0D
#define OV9740_AEC_CTRL0E		0x3A0E
#define OV9740_AEC_MAXEXPO_50_H		0x3A14
#define OV9740_AEC_MAXEXPO_50_L		0x3A15

/* AEC/AGC Control */
#define OV9740_AEC_ENABLE		0x3503
#define OV9740_GAIN_CEILING_01		0x3A18
#define OV9740_GAIN_CEILING_02		0x3A19
#define OV9740_AEC_HI_THRESHOLD		0x3A11
#define OV9740_AEC_3A1A			0x3A1A
#define OV9740_AEC_CTRL1B_WPT2		0x3A1B
#define OV9740_AEC_CTRL0F_WPT		0x3A0F
#define OV9740_AEC_CTRL10_BPT		0x3A10
#define OV9740_AEC_CTRL1E_BPT2		0x3A1E
#define OV9740_AEC_LO_THRESHOLD		0x3A1F

/* BLC Control */
#define OV9740_BLC_AUTO_ENABLE		0x4002
#define OV9740_BLC_MODE			0x4005

/* VFIFO */
#define OV9740_VFIFO_READ_START_HI	0x4608
#define OV9740_VFIFO_READ_START_LO	0x4609

/* DVP Control */
#define OV9740_DVP_VSYNC_CTRL02		0x4702
#define OV9740_DVP_VSYNC_MODE		0x4704
#define OV9740_DVP_VSYNC_CTRL06		0x4706

/* PLL Setting */
#define OV9740_PLL_MODE_CTRL01		0x3104
#define OV9740_PRE_PLL_CLK_DIV		0x0305
#define OV9740_PLL_MULTIPLIER		0x0307
#define OV9740_VT_SYS_CLK_DIV		0x0303
#define OV9740_VT_PIX_CLK_DIV		0x0301
#define OV9740_PLL_CTRL3010		0x3010
#define OV9740_VFIFO_CTRL00		0x460E

/* ISP Control */
#define OV9740_ISP_CTRL00		0x5000
#define OV9740_ISP_CTRL01		0x5001
#define OV9740_ISP_CTRL03		0x5003
#define OV9740_ISP_CTRL05		0x5005
#define OV9740_ISP_CTRL12		0x5012
#define OV9740_ISP_CTRL19		0x5019
#define OV9740_ISP_CTRL1A		0x501A
#define OV9740_ISP_CTRL1E		0x501E
#define OV9740_ISP_CTRL1F		0x501F
#define OV9740_ISP_CTRL20		0x5020
#define OV9740_ISP_CTRL21		0x5021

/* AWB */
#define OV9740_AWB_CTRL00		0x5180
#define OV9740_AWB_CTRL01		0x5181
#define OV9740_AWB_CTRL02		0x5182
#define OV9740_AWB_CTRL03		0x5183
#define OV9740_AWB_ADV_CTRL01		0x5184
#define OV9740_AWB_ADV_CTRL02		0x5185
#define OV9740_AWB_ADV_CTRL03		0x5186
#define OV9740_AWB_ADV_CTRL04		0x5187
#define OV9740_AWB_ADV_CTRL05		0x5188
#define OV9740_AWB_ADV_CTRL06		0x5189
#define OV9740_AWB_ADV_CTRL07		0x518A
#define OV9740_AWB_ADV_CTRL08		0x518B
#define OV9740_AWB_ADV_CTRL09		0x518C
#define OV9740_AWB_ADV_CTRL10		0x518D
#define OV9740_AWB_ADV_CTRL11		0x518E
#define OV9740_AWB_CTRL0F		0x518F
#define OV9740_AWB_CTRL10		0x5190
#define OV9740_AWB_CTRL11		0x5191
#define OV9740_AWB_CTRL12		0x5192
#define OV9740_AWB_CTRL13		0x5193
#define OV9740_AWB_CTRL14		0x5194

/* MIPI Control */
#define OV9740_MIPI_CTRL00		0x4800
#define OV9740_MIPI_3837		0x3837
#define OV9740_MIPI_CTRL01		0x4801
#define OV9740_MIPI_CTRL03		0x4803
#define OV9740_MIPI_CTRL05		0x4805
#define OV9740_VFIFO_RD_CTRL		0x4601
#define OV9740_MIPI_CTRL_3012		0x3012
#define OV9740_SC_CMMM_MIPI_CTR		0x3014


#define OV9740_FOCAL_LENGTH_NUM  208  
#define OV9740_FOCAL_LENGTH_DEM  100
/*
 * focal length bits definition:
 * bits 31-16: numerator, bits 15-0: denominator
 */
#define OV9740_FOCAL_LENGTH_DEFAULT  0x0d00064 //0xA60064   modify by zte yaolingling for focus length

/*
 * current f-number bits definition:
 * bits 31-16: numerator, bits 15-0: denominator
 */
#define OV9740_F_NUMBER_DEFAULT 0x1200064

/*
 * f-number range bits definition:
 * bits 31-24: max f-number numerator
 * bits 23-16: max f-number denominator
 * bits 15-8: min f-number numerator
 * bits 7-0: min f-number denominator
 */
#define OV9740_F_NUMBER_RANGE 0x1D0a1D0a
struct regval_list {
	u16 reg_num;
	u8 value;
};

/* Misc. structures */
struct ov9740_reg {
	u16				reg;
	u8				val;
};
static const struct ov9740_reg ov9740_defaults[] = {
	/* Banding Filter */
	{ OV9740_AEC_B50_STEP_HI,	0x00 },
	{ OV9740_AEC_B50_STEP_LO,	0xe8 },
	{ OV9740_AEC_CTRL0E,		0x03 },
	{ OV9740_AEC_MAXEXPO_50_H,	0x15 },
	{ OV9740_AEC_MAXEXPO_50_L,	0xc6 },
	{ OV9740_AEC_B60_STEP_HI,	0x00 },
	{ OV9740_AEC_B60_STEP_LO,	0xc0 },
	{ OV9740_AEC_CTRL0D,		0x04 },
	{ OV9740_AEC_MAXEXPO_60_H,	0x18 },
	{ OV9740_AEC_MAXEXPO_60_L,	0x20 },

	/* LC */
	{ 0x5842, 0x02 }, { 0x5843, 0x5e }, { 0x5844, 0x04 }, { 0x5845, 0x32 },
	{ 0x5846, 0x03 }, { 0x5847, 0x29 }, { 0x5848, 0x02 }, { 0x5849, 0xcc },

	/* Un-documented OV9740 registers */
	#if 0
	{ 0x5800, 0x29 }, { 0x5801, 0x25 }, { 0x5802, 0x20 }, { 0x5803, 0x21 },
	{ 0x5804, 0x26 }, { 0x5805, 0x2e }, { 0x5806, 0x11 }, { 0x5807, 0x0c },
	{ 0x5808, 0x09 }, { 0x5809, 0x0a }, { 0x580A, 0x0e }, { 0x580B, 0x16 },
	{ 0x580C, 0x06 }, { 0x580D, 0x02 }, { 0x580E, 0x00 }, { 0x580F, 0x00 },
	{ 0x5810, 0x04 }, { 0x5811, 0x0a }, { 0x5812, 0x05 }, { 0x5813, 0x02 },
	{ 0x5814, 0x00 }, { 0x5815, 0x00 }, { 0x5816, 0x03 }, { 0x5817, 0x09 },
	{ 0x5818, 0x0f }, { 0x5819, 0x0a }, { 0x581A, 0x07 }, { 0x581B, 0x08 },
	{ 0x581C, 0x0b }, { 0x581D, 0x14 }, { 0x581E, 0x28 }, { 0x581F, 0x23 },
	{ 0x5820, 0x1d }, { 0x5821, 0x1e }, { 0x5822, 0x24 }, { 0x5823, 0x2a },
	{ 0x5824, 0x4f }, { 0x5825, 0x6f }, { 0x5826, 0x5f }, { 0x5827, 0x7f },
	{ 0x5828, 0x9f }, { 0x5829, 0x5f }, { 0x582A, 0x8f }, { 0x582B, 0x9e },
	{ 0x582C, 0x8f }, { 0x582D, 0x9f }, { 0x582E, 0x4f }, { 0x582F, 0x87 },
	{ 0x5830, 0x86 }, { 0x5831, 0x97 }, { 0x5832, 0xae }, { 0x5833, 0x3f },
	{ 0x5834, 0x8e }, { 0x5835, 0x7c }, { 0x5836, 0x7e }, { 0x5837, 0xaf },
	{ 0x5838, 0x8f }, { 0x5839, 0x8f }, { 0x583A, 0x9f }, { 0x583B, 0x7f },
	{ 0x583C, 0x5f },
	#endif
	//Lens correction
	{0x5800 ,0x1c}, // Lens correction setting
	{0x5801 ,0x16}, // Lens correction setting
	{0x5802 ,0x15}, // Lens correction setting
	{0x5803 ,0x16}, // Lens correction setting
	{0x5804 ,0x18}, // Lens correction setting
	{0x5805 ,0x1a}, // Lens correction setting
	{0x5806 ,0x0c}, // Lens correction setting
	{0x5807 ,0x0a}, // Lens correction setting
	{0x5808 ,0x08}, // Lens correction setting
	{0x5809 ,0x08}, // Lens correction setting
	{0x580a ,0x0a}, // Lens correction setting
	{0x580b ,0x0b}, // Lens correction setting
	{0x580c ,0x05}, // Lens correction setting
	{0x580d ,0x02}, // Lens correction setting
	{0x580e ,0x00}, // Lens correction setting
	{0x580f ,0x00}, // Lens correction setting
	{0x5810 ,0x02}, // Lens correction setting
	{0x5811 ,0x05}, // Lens correction setting
	{0x5812 ,0x04}, // Lens correction setting
    	{0x5813 ,0x01}, // Lens correction setting
	{0x5814 ,0x00}, // Lens correction setting
	{0x5815 ,0x00}, // Lens correction setting
	{0x5816 ,0x02}, // Lens correction setting
	{0x5817 ,0x03}, // Lens correction setting
	{0x5818 ,0x0a}, // Lens correction setting
	{0x5819 ,0x07}, // Lens correction setting
	{0x581a ,0x05}, // Lens correction setting
	{0x581b ,0x05}, // Lens correction setting
	{0x581c ,0x08}, // Lens correction setting
	{0x581d ,0x0b}, // Lens correction setting
	{0x581e ,0x15}, // Lens correction setting
	{0x581f ,0x14}, // Lens correction setting
	{0x5820 ,0x14}, // Lens correction setting
	{0x5821 ,0x13}, // Lens correction setting
	{0x5822 ,0x17}, // Lens correction setting
	{0x5823 ,0x16}, // Lens correction setting
	{0x5824 ,0x46}, // Lens correction setting
	{0x5825 ,0x4c}, // Lens correction setting
	{0x5826 ,0x6c}, // Lens correction setting
	{0x5827 ,0x4c}, // Lens correction setting
	{0x5828 ,0x80}, // Lens correction setting
	{0x5829 ,0x2e}, // Lens correction setting
	{0x582a ,0x48}, // Lens correction setting
	{0x582b ,0x46}, // Lens correction setting
	{0x582c ,0x2a}, // Lens correction setting
	{0x582d ,0x68}, // Lens correction setting
	{0x582e ,0x08}, // Lens correction setting
	{0x582f ,0x26}, // Lens correction setting
	{0x5830 ,0x44}, // Lens correction setting
	{0x5831 ,0x46}, // Lens correction setting
	{0x5832 ,0x62}, // Lens correction setting
	{0x5833 ,0x0c}, // Lens correction setting
	{0x5834 ,0x28}, // Lens correction setting
	{0x5835 ,0x46}, // Lens correction setting
	{0x5836 ,0x28}, // Lens correction setting
	{0x5837 ,0x88}, // Lens correction setting
	{0x5838 ,0x0e}, // Lens correction setting
	{0x5839 ,0x0e}, // Lens correction setting
	{0x583a ,0x2c}, // Lens correction setting
	{0x583b ,0x2e}, // Lens correction setting
	{0x583c ,0x46}, // Lens correction setting
	{0x583d ,0xca}, // Lens correction setting
	{0x583e ,0xf0}, // Lens correction setting
	{0x5842 ,0x02}, // Lens correction setting
	{0x5843 ,0x5e}, // Lens correction setting
	{0x5844 ,0x04}, // Lens correction setting
	{0x5845 ,0x32}, // Lens correction setting
	{0x5846 ,0x03}, // Lens correction setting
	{0x5847 ,0x29}, // Lens correction setting
	{0x5848 ,0x02}, // Lens correction setting
	{0x5849 ,0xcc}, // Lens correction setting

	/* Y Gamma */
	{ 0x5480, 0x07 }, { 0x5481, 0x18 }, { 0x5482, 0x2c }, { 0x5483, 0x4e },
	{ 0x5484, 0x5e }, { 0x5485, 0x6b }, { 0x5486, 0x77 }, { 0x5487, 0x82 },
	{ 0x5488, 0x8c }, { 0x5489, 0x95 }, { 0x548A, 0xa4 }, { 0x548B, 0xb1 },
	{ 0x548C, 0xc6 }, { 0x548D, 0xd8 }, { 0x548E, 0xe9 },

	/* UV Gamma */
	{ 0x5490, 0x0f }, { 0x5491, 0xff }, { 0x5492, 0x0d }, { 0x5493, 0x05 },
	{ 0x5494, 0x07 }, { 0x5495, 0x1a }, { 0x5496, 0x04 }, { 0x5497, 0x01 },
	{ 0x5498, 0x03 }, { 0x5499, 0x53 }, { 0x549A, 0x02 }, { 0x549B, 0xeb },
	{ 0x549C, 0x02 }, { 0x549D, 0xa0 }, { 0x549E, 0x02 }, { 0x549F, 0x67 },
	{ 0x54A0, 0x02 }, { 0x54A1, 0x3b }, { 0x54A2, 0x02 }, { 0x54A3, 0x18 },
	{ 0x54A4, 0x01 }, { 0x54A5, 0xe7 }, { 0x54A6, 0x01 }, { 0x54A7, 0xc3 },
	{ 0x54A8, 0x01 }, { 0x54A9, 0x94 }, { 0x54AA, 0x01 }, { 0x54AB, 0x72 },
	{ 0x54AC, 0x01 }, { 0x54AD, 0x57 },

	/* AWB */
	{ OV9740_AWB_CTRL00,		0xf0 },
	{ OV9740_AWB_CTRL01,		0x00 },
	{ OV9740_AWB_CTRL02,		0x41 },
	{ OV9740_AWB_CTRL03,		0x42 },
	{ OV9740_AWB_ADV_CTRL01,	0x8a },
	{ OV9740_AWB_ADV_CTRL02,	0x61 },
	{ OV9740_AWB_ADV_CTRL03,	0xce },
	{ OV9740_AWB_ADV_CTRL04,	0xa8 },
	{ OV9740_AWB_ADV_CTRL05,	0x17 },
	{ OV9740_AWB_ADV_CTRL06,	0x1f },
	{ OV9740_AWB_ADV_CTRL07,	0x27 },
	{ OV9740_AWB_ADV_CTRL08,	0x41 },
	{ OV9740_AWB_ADV_CTRL09,	0x34 },
	{ OV9740_AWB_ADV_CTRL10,	0xf0 },
	{ OV9740_AWB_ADV_CTRL11,	0x10 },
	{ OV9740_AWB_CTRL0F,		0xff },
	{ OV9740_AWB_CTRL10,		0x00 },
	{ OV9740_AWB_CTRL11,		0xff },
	{ OV9740_AWB_CTRL12,		0x00 },
	{ OV9740_AWB_CTRL13,		0xff },
	{ OV9740_AWB_CTRL14,		0x00 },

	/* CIP */
	{ 0x530D, 0x12 },

	/* CMX */
	{ 0x5380, 0x01 }, { 0x5381, 0x00 }, { 0x5382, 0x00 }, { 0x5383, 0x17 },
	{ 0x5384, 0x00 }, { 0x5385, 0x01 }, { 0x5386, 0x00 }, { 0x5387, 0x00 },
	{ 0x5388, 0x00 }, { 0x5389, 0xe0 }, { 0x538A, 0x00 }, { 0x538B, 0x20 },
	{ 0x538C, 0x00 }, { 0x538D, 0x00 }, { 0x538E, 0x00 }, { 0x538F, 0x16 },
	{ 0x5390, 0x00 }, { 0x5391, 0x9c }, { 0x5392, 0x00 }, { 0x5393, 0xa0 },
	{ 0x5394, 0x18 },

	/* 50/60 Detection */
	{ 0x3C0A, 0x9c }, { 0x3C0B, 0x3f },

	/* Output Select */
	{ OV9740_IO_OUTPUT_SEL01,	0x00 },
	{ OV9740_IO_OUTPUT_SEL02,	0x00 },
	{ OV9740_IO_CREL00,		0x00 },
	{ OV9740_IO_CREL01,		0x00 },
	{ OV9740_IO_CREL02,		0x00 },

	/* AWB Control */
	{ OV9740_AWB_MANUAL_CTRL,	0x00 },

	/* Analog Control */
	{ OV9740_ANALOG_CTRL03,		0xaa },
	{ OV9740_ANALOG_CTRL32,		0x2f },
	{ OV9740_ANALOG_CTRL20,		0x66 },
	{ OV9740_ANALOG_CTRL21,		0xc0 },
	{ OV9740_ANALOG_CTRL31,		0x52 },
	{ OV9740_ANALOG_CTRL33,		0x50 },
	{ OV9740_ANALOG_CTRL30,		0xca },
	{ OV9740_ANALOG_CTRL04,		0x0c },
	{ OV9740_ANALOG_CTRL01,		0x40 },
	{ OV9740_ANALOG_CTRL02,		0x16 },
	{ OV9740_ANALOG_CTRL10,		0xa1 },
	{ OV9740_ANALOG_CTRL12,		0x24 },
	{ OV9740_ANALOG_CTRL22,		0x9f },

	/* Sensor Control */
	{ OV9740_SENSOR_CTRL03,		0x42 },
	{ OV9740_SENSOR_CTRL04,		0x10 },
	{ OV9740_SENSOR_CTRL05,		0x45 },
	{ OV9740_SENSOR_CTRL07,		0x14 },

	/* Timing Control */
	{ OV9740_TIMING_CTRL33,		0x04 },
	{ OV9740_TIMING_CTRL35,		0x02 },
	{ OV9740_TIMING_CTRL19,		0x6e },
	{ OV9740_TIMING_CTRL17,		0x94 },

	/* AEC/AGC Control */
	{ OV9740_AEC_ENABLE,		0x10 },
	{ OV9740_GAIN_CEILING_01,	0x00 },
	{ OV9740_GAIN_CEILING_02,	0x7f },
	{ OV9740_AEC_HI_THRESHOLD,	0xa0 },
	{ OV9740_AEC_3A1A,		0x05 },
	{ OV9740_AEC_CTRL1B_WPT2,	0x50 },
	{ OV9740_AEC_CTRL0F_WPT,	0x50 },
	{ OV9740_AEC_CTRL10_BPT,	0x4c },
	{ OV9740_AEC_CTRL1E_BPT2,	0x4c },
	{ OV9740_AEC_LO_THRESHOLD,	0x26 },

	/* BLC Control */
	{ OV9740_BLC_AUTO_ENABLE,	0x45 },
	{ OV9740_BLC_MODE,		0x18 },

	/* DVP Control */
	{ OV9740_DVP_VSYNC_CTRL02,	0x04 },
	{ OV9740_DVP_VSYNC_MODE,	0x00 },
	{ OV9740_DVP_VSYNC_CTRL06,	0x08 },

	/* PLL Setting */
	{ OV9740_PLL_MODE_CTRL01,	0x20 },
	{ OV9740_PRE_PLL_CLK_DIV,	0x03 },
	{ OV9740_PLL_MULTIPLIER,	0x4c },
	{ OV9740_VT_SYS_CLK_DIV,	0x01 },
	{ OV9740_VT_PIX_CLK_DIV,	0x08 },
	{ OV9740_PLL_CTRL3010,		0x01 },
	{ OV9740_VFIFO_CTRL00,		0x82 },

	/* Timing Setting */
	/* VTS */
	{ OV9740_FRM_LENGTH_LN_HI,	0x03 },//0x03
	{ OV9740_FRM_LENGTH_LN_LO,	0x38 },//0x07 
	/* HTS */
	{ OV9740_LN_LENGTH_PCK_HI,	0x06 },
	{ OV9740_LN_LENGTH_PCK_LO,	0x2A },

	{ 0x0101,	0x02},
	{ 0x4300,	0x32},
	//{ 0x501a,	0x94},  // for test pattern mode add by yaolingling 0516
	/* MIPI Control */
	{ OV9740_MIPI_CTRL00,		0x44 },
	{ OV9740_MIPI_3837,		0x01 },
	{ OV9740_MIPI_CTRL01,		0x0f },
	{ OV9740_MIPI_CTRL03,		0x05 },
	{ OV9740_MIPI_CTRL05,		0x10 },
	{ OV9740_VFIFO_RD_CTRL,		0x16 },
	{ OV9740_MIPI_CTRL_3012,	0x70 },
	{ OV9740_SC_CMMM_MIPI_CTR,	0x01 },
};
static const struct ov9740_reg ov9740_regs_qvga[] = {
	
	{ OV9740_X_ADDR_START_HI,	0x00 },
	{ OV9740_X_ADDR_START_LO,	0xa0 },
	{ OV9740_Y_ADDR_START_HI,	0x00 },
	{ OV9740_Y_ADDR_START_LO,	0x00 },
	{ OV9740_X_ADDR_END_HI,		0x04 },
	{ OV9740_X_ADDR_END_LO,		0x63 },
	{ OV9740_Y_ADDR_END_HI,		0x02 },
	{ OV9740_Y_ADDR_END_LO,		0xd3 },
	{ OV9740_X_OUTPUT_SIZE_HI,	0x01 },
	{ OV9740_X_OUTPUT_SIZE_LO,	0x40 },
	{ OV9740_Y_OUTPUT_SIZE_HI,	0x00 },
	{ OV9740_Y_OUTPUT_SIZE_LO,	0xf0 },
	{ OV9740_ISP_CTRL1E,		0x03 },
	{ OV9740_ISP_CTRL1F,		0xc0 },
	{ OV9740_ISP_CTRL20,		0x02 },
	{ OV9740_ISP_CTRL21,		0xd0 },
	{ OV9740_VFIFO_READ_START_HI,	0x01 },
	{ OV9740_VFIFO_READ_START_LO,	0x40 },
	{ OV9740_ISP_CTRL00,		0xff },
	{ OV9740_ISP_CTRL01,		0xff },
	{ OV9740_ISP_CTRL03,		0xff },
	
};

static const struct ov9740_reg ov9740_regs_vga[] = {
	{ OV9740_X_ADDR_START_HI,	0x00 },
	{ OV9740_X_ADDR_START_LO,	0xa0 },
	{ OV9740_Y_ADDR_START_HI,	0x00 },
	{ OV9740_Y_ADDR_START_LO,	0x00 },
	{ OV9740_X_ADDR_END_HI,		0x04 },
	{ OV9740_X_ADDR_END_LO,		0x63 },
	{ OV9740_Y_ADDR_END_HI,		0x02 },
	{ OV9740_Y_ADDR_END_LO,		0xd3 },
	{ OV9740_X_OUTPUT_SIZE_HI,	0x02 },
	{ OV9740_X_OUTPUT_SIZE_LO,	0x80 },
	{ OV9740_Y_OUTPUT_SIZE_HI,	0x01 },
	{ OV9740_Y_OUTPUT_SIZE_LO,	0xe0 },
	{ OV9740_ISP_CTRL1E,		0x03 },
	{ OV9740_ISP_CTRL1F,		0xc0 },
	{ OV9740_ISP_CTRL20,		0x02 },
	{ OV9740_ISP_CTRL21,		0xd0 },
	{ OV9740_VFIFO_READ_START_HI,	0x01 },
	{ OV9740_VFIFO_READ_START_LO,	0x40 },
	{ OV9740_ISP_CTRL00,		0xff },
	{ OV9740_ISP_CTRL01,		0xff },
	{ OV9740_ISP_CTRL03,		0xff },
};

static const struct ov9740_reg ov9740_regs_720p[] = {
	{ OV9740_X_ADDR_START_HI,	0x00 },
	{ OV9740_X_ADDR_START_LO,	0x00 },
	{ OV9740_Y_ADDR_START_HI,	0x00 },
	{ OV9740_Y_ADDR_START_LO,	0x00 },
	{ OV9740_X_ADDR_END_HI,		0x05 },
	{ OV9740_X_ADDR_END_LO,		0x03 },
	{ OV9740_Y_ADDR_END_HI,		0x02 },
	{ OV9740_Y_ADDR_END_LO,		0xd3 },
	{ OV9740_X_OUTPUT_SIZE_HI,	0x05 },
	{ OV9740_X_OUTPUT_SIZE_LO,	0x00 },
	{ OV9740_Y_OUTPUT_SIZE_HI,	0x02 },
	{ OV9740_Y_OUTPUT_SIZE_LO,	0xd0 },
	{ OV9740_ISP_CTRL1E,		0x05 },
	{ OV9740_ISP_CTRL1F,		0x00 },
	{ OV9740_ISP_CTRL20,		0x02 },
	{ OV9740_ISP_CTRL21,		0xd0 },
	{ OV9740_VFIFO_READ_START_HI,	0x02 },
	{ OV9740_VFIFO_READ_START_LO,	0x30 },
	{ OV9740_ISP_CTRL00,		0xff },
	{ OV9740_ISP_CTRL01,		0xef },
	{ OV9740_ISP_CTRL03,		0xff },
};
static const struct ov9740_reg ov9740_regs_480p[] ={
	//@@720x480 from 1080x720
	{0x0344 ,0x00},
	{0x0345 ,0x00},
	{0x0346 ,0x00},
	{0x0347 ,0x00},
	{0x0348 ,0x05},
	{0x0349 ,0x03},
	{0x034a ,0x02},
	{0x034b ,0xd3},
	//;scaling input
	{0x501e, 0x04},
	{0x501f, 0x38},
	{0x5020, 0x02},
	{0x5021, 0xd0},
	//;vfifo setting
	{0x4608 ,0x01},
	{0x4609 ,0x40},
	//;scaling output
	{0x034c ,0x02},
	{0x034d ,0xd0},
	{0x034e ,0x01},
	{0x034f ,0xe0},
	//;scaling enable
	{0x5001, 0xff},
	{0x5000, 0xff},
	{0x5003, 0xff},

};

/* Supported resolutions */
enum {
	OV9740_RES_QVGA,
	OV9740_RES_VGA,
	OV9740_RES_720P,
	OV9740_RES_480P,
	
};


#define OV9740_RES_720P_SIZE_H		1280
#define OV9740_RES_720P_SIZE_V		720
#define OV9740_RES_480P_SIZE_H		720
#define OV9740_RES_480P_SIZE_V		480
#define OV9740_RES_VGA_SIZE_H		640
#define OV9740_RES_VGA_SIZE_V		480
#define OV9740_RES_QVGA_SIZE_H		320
#define OV9740_RES_QVGA_SIZE_V		240

struct ov9740_res_struct {
	u8 *desc;
	int res;
	int width;
	int height;
	int fps;
	int skip_frames;
	bool used;
	struct regval_list *regs;
};

struct ov9740_control {
	struct v4l2_queryctrl qc;
	int (*query)(struct v4l2_subdev *sd, s32 *value);
	int (*tweak)(struct v4l2_subdev *sd, int value);
};

/*
 * Modes supported by the mt9m114 driver.
 * Please, keep them in ascending order.
 */
static struct ov9740_res_struct ov9740_res[] = {
	{
	.desc	= "QVGA",
	.res	= OV9740_RES_QVGA,
	.width	= 320,
	.height	= 240,
	.fps	=30,
	.used	= 0,
	.regs	= NULL,
	.skip_frames =4,  // modified by yaolinglingfrom 2 to 3  for switch capture mode to video mode , it shows  green  on lcd 
	},
	
	{
	.desc	= "VGA",
	.res	= OV9740_RES_VGA,
	.width	= 640,
	.height	= 480,
	.fps	=30,
	.used	= 0,
	.regs	= NULL,
	.skip_frames =4,  // modified by yaolinglingfrom 2 to 3  for switch capture mode to video mode , it shows  green  on lcd 
	},
	{
	.desc	= "480p",
	.res	= OV9740_RES_480P,
	.width	= 720,
	.height	= 480,
	.fps	=30,
	.used	= 0,
	.regs	= NULL,
	.skip_frames = 4,  // modified by yaolinglingfrom 2 to 3  for switch capture mode to video mode ,  it shows green  on lcd 
	},
	{
	.desc	= "720p",
	.res	= OV9740_RES_720P,
	.width	= 1280,
	.height	= 720,
	.fps	=30,
	.used	= 0,
	.regs	= NULL,
	.skip_frames = 4,  // modified by yaolinglingfrom 2 to 3  for switch capture mode to video mode ,  it shows green  on lcd 
	}, 
	
};
#define N_RES (ARRAY_SIZE(ov9740_res))

struct ov9740_device {
	struct v4l2_subdev sd;
	struct media_pad pad;
	struct v4l2_mbus_framefmt format;

	struct camera_sensor_platform_data *platform_data;
	int real_model_id;
	int nctx;
	int power;

	unsigned int bus_width;
	unsigned int mode;
	unsigned int field_inv;
	unsigned int field_sel;
	unsigned int ycseq;
	unsigned int conv422;
	unsigned int bpat;
	unsigned int hpol;
	unsigned int vpol;
	unsigned int edge;
	unsigned int bls;
	unsigned int gamma;
	unsigned int cconv;
	unsigned int res;
	unsigned int dwn_sz;
	unsigned int blc;
	unsigned int agc;
	unsigned int awb;
	unsigned int aec;
	/* extention SENSOR version 2 */
	unsigned int cie_profile;

	/* extention SENSOR version 3 */
	unsigned int flicker_freq;

	/* extension SENSOR version 4 */
	unsigned int smia_mode;
	unsigned int mipi_mode;

	/* Add name here to load shared library */
	unsigned int type;

	/*Number of MIPI lanes*/
	unsigned int mipi_lanes;
	char name[32];

	u8 lightfreq;
};

