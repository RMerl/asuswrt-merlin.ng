/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  Copyright (C) 2014 Gateworks Corporation
 *  Tim Harvey <tharvey@gateworks.com>
 */

#ifndef __PFUZE100_PMIC_H_
#define __PFUZE100_PMIC_H_

/* Device ID */
enum {PFUZE100 = 0x10, PFUZE200 = 0x11, PFUZE3000 = 0x30};

#define PFUZE100_REGULATOR_DRIVER	"pfuze100_regulator"

/* PFUZE100 registers */
enum {
	PFUZE100_DEVICEID	= 0x00,
	PFUZE100_REVID		= 0x03,
	PFUZE100_FABID		= 0x04,

	PFUZE100_SW1ABVOL	= 0x20,
	PFUZE100_SW1ABSTBY	= 0x21,
	PFUZE100_SW1ABOFF	= 0x22,
	PFUZE100_SW1ABMODE	= 0x23,
	PFUZE100_SW1ABCONF	= 0x24,
	PFUZE100_SW1CVOL	= 0x2e,
	PFUZE100_SW1CSTBY	= 0x2f,
	PFUZE100_SW1COFF	= 0x30,
	PFUZE100_SW1CMODE	= 0x31,
	PFUZE100_SW1CCONF	= 0x32,
	PFUZE100_SW2VOL		= 0x35,
	PFUZE100_SW2STBY	= 0x36,
	PFUZE100_SW2OFF		= 0x37,
	PFUZE100_SW2MODE	= 0x38,
	PFUZE100_SW2CONF	= 0x39,
	PFUZE100_SW3AVOL	= 0x3c,
	PFUZE100_SW3ASTBY	= 0x3D,
	PFUZE100_SW3AOFF	= 0x3E,
	PFUZE100_SW3AMODE	= 0x3F,
	PFUZE100_SW3ACONF	= 0x40,
	PFUZE100_SW3BVOL	= 0x43,
	PFUZE100_SW3BSTBY	= 0x44,
	PFUZE100_SW3BOFF	= 0x45,
	PFUZE100_SW3BMODE	= 0x46,
	PFUZE100_SW3BCONF	= 0x47,
	PFUZE100_SW4VOL		= 0x4a,
	PFUZE100_SW4STBY	= 0x4b,
	PFUZE100_SW4OFF		= 0x4c,
	PFUZE100_SW4MODE	= 0x4d,
	PFUZE100_SW4CONF	= 0x4e,
	PFUZE100_SWBSTCON1	= 0x66,
	PFUZE100_VREFDDRCON	= 0x6a,
	PFUZE100_VSNVSVOL	= 0x6b,
	PFUZE100_VGEN1VOL	= 0x6c,
	PFUZE100_VGEN2VOL	= 0x6d,
	PFUZE100_VGEN3VOL	= 0x6e,
	PFUZE100_VGEN4VOL	= 0x6f,
	PFUZE100_VGEN5VOL	= 0x70,
	PFUZE100_VGEN6VOL	= 0x71,

	PFUZE100_NUM_OF_REGS	= 0x7f,
};

/* Registor offset based on VOLT register */
#define PFUZE100_VOL_OFFSET	0
#define PFUZE100_STBY_OFFSET	1
#define PFUZE100_OFF_OFFSET	2
#define PFUZE100_MODE_OFFSET	3
#define PFUZE100_CONF_OFFSET	4

/*
 * Buck Regulators
 */

#define PFUZE100_SW1ABC_SETP(x)	((x - 3000) / 250)

/* SW1A/B/C Output Voltage Configuration */
#define SW1x_0_300V 0
#define SW1x_0_325V 1
#define SW1x_0_350V 2
#define SW1x_0_375V 3
#define SW1x_0_400V 4
#define SW1x_0_425V 5
#define SW1x_0_450V 6
#define SW1x_0_475V 7
#define SW1x_0_500V 8
#define SW1x_0_525V 9
#define SW1x_0_550V 10
#define SW1x_0_575V 11
#define SW1x_0_600V 12
#define SW1x_0_625V 13
#define SW1x_0_650V 14
#define SW1x_0_675V 15
#define SW1x_0_700V 16
#define SW1x_0_725V 17
#define SW1x_0_750V 18
#define SW1x_0_775V 19
#define SW1x_0_800V 20
#define SW1x_0_825V 21
#define SW1x_0_850V 22
#define SW1x_0_875V 23
#define SW1x_0_900V 24
#define SW1x_0_925V 25
#define SW1x_0_950V 26
#define SW1x_0_975V 27
#define SW1x_1_000V 28
#define SW1x_1_025V 29
#define SW1x_1_050V 30
#define SW1x_1_075V 31
#define SW1x_1_100V 32
#define SW1x_1_125V 33
#define SW1x_1_150V 34
#define SW1x_1_175V 35
#define SW1x_1_200V 36
#define SW1x_1_225V 37
#define SW1x_1_250V 38
#define SW1x_1_275V 39
#define SW1x_1_300V 40
#define SW1x_1_325V 41
#define SW1x_1_350V 42
#define SW1x_1_375V 43
#define SW1x_1_400V 44
#define SW1x_1_425V 45
#define SW1x_1_450V 46
#define SW1x_1_475V 47
#define SW1x_1_500V 48
#define SW1x_1_525V 49
#define SW1x_1_550V 50
#define SW1x_1_575V 51
#define SW1x_1_600V 52
#define SW1x_1_625V 53
#define SW1x_1_650V 54
#define SW1x_1_675V 55
#define SW1x_1_700V 56
#define SW1x_1_725V 57
#define SW1x_1_750V 58
#define SW1x_1_775V 59
#define SW1x_1_800V 60
#define SW1x_1_825V 61
#define SW1x_1_850V 62
#define SW1x_1_875V 63

#define SW1x_NORMAL_MASK  0x3f
#define SW1x_STBY_MASK    0x3f
#define SW1x_OFF_MASK     0x3f

#define SW_MODE_MASK	0xf
#define SW_MODE_SHIFT	0

#define SW1xCONF_DVSSPEED_MASK 0xc0
#define SW1xCONF_DVSSPEED_2US  0x00
#define SW1xCONF_DVSSPEED_4US  0x40
#define SW1xCONF_DVSSPEED_8US  0x80
#define SW1xCONF_DVSSPEED_16US 0xc0

/*
 * LDO Configuration
 */

/* VGEN1/2 Voltage Configuration */
#define LDOA_0_80V	0
#define LDOA_0_85V	1
#define LDOA_0_90V	2
#define LDOA_0_95V	3
#define LDOA_1_00V	4
#define LDOA_1_05V	5
#define LDOA_1_10V	6
#define LDOA_1_15V	7
#define LDOA_1_20V	8
#define LDOA_1_25V	9
#define LDOA_1_30V	10
#define LDOA_1_35V	11
#define LDOA_1_40V	12
#define LDOA_1_45V	13
#define LDOA_1_50V	14
#define LDOA_1_55V	15

/* VGEN3/4/5/6 Voltage Configuration */
#define LDOB_1_80V	0
#define LDOB_1_90V	1
#define LDOB_2_00V	2
#define LDOB_2_10V	3
#define LDOB_2_20V	4
#define LDOB_2_30V	5
#define LDOB_2_40V	6
#define LDOB_2_50V	7
#define LDOB_2_60V	8
#define LDOB_2_70V	9
#define LDOB_2_80V	10
#define LDOB_2_90V	11
#define LDOB_3_00V	12
#define LDOB_3_10V	13
#define LDOB_3_20V	14
#define LDOB_3_30V	15

#define LDO_VOL_MASK	0xf
#define LDO_EN		(1 << 4)
#define LDO_MODE_SHIFT	4
#define LDO_MODE_MASK	(1 << 4)
#define LDO_MODE_OFF	0
#define LDO_MODE_ON	1

#define VREFDDRCON_EN	(1 << 4)
/*
 * Boost Regulator
 */

/* SWBST Output Voltage */
#define SWBST_5_00V	0
#define SWBST_5_05V	1
#define SWBST_5_10V	2
#define SWBST_5_15V	3

#define SWBST_VOL_MASK	0x3
#define SWBST_MODE_MASK	0xC
#define SWBST_MODE_SHIFT 0x2
#define SWBST_MODE_OFF	0
#define SWBST_MODE_PFM	1
#define SWBST_MODE_AUTO	2
#define SWBST_MODE_APS	3

/*
 * Regulator Mode Control
 *
 * OFF: The regulator is switched off and the output voltage is discharged.
 * PFM: In this mode, the regulator is always in PFM mode, which is useful
 *      at light loads for optimized efficiency.
 * PWM: In this mode, the regulator is always in PWM mode operation
 *	regardless of load conditions.
 * APS: In this mode, the regulator moves automatically between pulse
 *	skipping mode and PWM mode depending on load conditions.
 *
 * SWxMODE[3:0]
 * Normal Mode  |  Standby Mode	|      value
 *   OFF		OFF		0x0
 *   PWM		OFF		0x1
 *   PFM		OFF		0x3
 *   APS		OFF		0x4
 *   PWM		PWM		0x5
 *   PWM		APS		0x6
 *   APS		APS		0x8
 *   APS		PFM		0xc
 *   PWM		PFM		0xd
 */
#define OFF_OFF		0x0
#define PWM_OFF		0x1
#define PFM_OFF		0x3
#define APS_OFF		0x4
#define PWM_PWM		0x5
#define PWM_APS		0x6
#define APS_APS		0x8
#define APS_PFM		0xc
#define PWM_PFM		0xd

#define SWITCH_SIZE	0x7

int power_pfuze100_init(unsigned char bus);
#endif
