/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de.
 *
 * (C) Copyright 2009 Freescale Semiconductor, Inc.
 */


#ifndef __MC13892_H__
#define __MC13892_H__

/* REG_CHARGE */

#define VCHRG0		(1 << 0)
#define VCHRG1		(1 << 1)
#define VCHRG2		(1 << 2)
#define ICHRG0		(1 << 3)
#define ICHRG1		(1 << 4)
#define ICHRG2		(1 << 5)
#define ICHRG3		(1 << 6)
#define TREN		(1 << 7)
#define ACKLPB		(1 << 8)
#define THCHKB		(1 << 9)
#define FETOVRD		(1 << 10)
#define FETCTRL		(1 << 11)
#define RVRSMODE	(1 << 13)
#define PLIM0		(1 << 15)
#define PLIM1		(1 << 16)
#define PLIMDIS		(1 << 17)
#define CHRGLEDEN	(1 << 18)
#define CHGTMRRST	(1 << 19)
#define CHGRESTART	(1 << 20)
#define CHGAUTOB	(1 << 21)
#define CYCLB		(1 << 22)
#define CHGAUTOVIB	(1 << 23)

/* REG_SETTING_0/1 */
#define VO_1_20V	0
#define VO_1_30V	1
#define VO_1_50V	2
#define VO_1_80V	3
#define VO_1_10V	4
#define VO_2_00V	5
#define VO_2_77V	6
#define VO_2_40V	7

#define VIOL		2
#define VDIG		4
#define VGEN		6

/* SWxMode for Normal/Standby Mode */
#define SWMODE_OFF_OFF		0
#define SWMODE_PWM_OFF		1
#define SWMODE_PWMPS_OFF	2
#define SWMODE_PFM_OFF		3
#define SWMODE_AUTO_OFF		4
#define SWMODE_PWM_PWM		5
#define SWMODE_PWM_AUTO		6
#define SWMODE_AUTO_AUTO	8
#define SWMODE_PWM_PWMPS	9
#define SWMODE_PWMS_PWMPS	10
#define SWMODE_PWMS_AUTO	11
#define SWMODE_AUTO_PFM		12
#define SWMODE_PWM_PFM		13
#define SWMODE_PWMS_PFM		14
#define SWMODE_PFM_PFM		15
#define SWMODE_MASK		0x0F

#define SWMODE1_SHIFT		0
#define SWMODE2_SHIFT		10
#define SWMODE3_SHIFT		0
#define SWMODE4_SHIFT		8

/* Fields in REG_SETTING_1 */
#define VVIDEO_2_7	(0 << 2)
#define VVIDEO_2_775	(1 << 2)
#define VVIDEO_2_5	(2 << 2)
#define VVIDEO_2_6	(3 << 2)
#define VVIDEO_MASK	(3 << 2)
#define VAUDIO_2_3	(0 << 4)
#define VAUDIO_2_5	(1 << 4)
#define VAUDIO_2_775	(2 << 4)
#define VAUDIO_3_0	(3 << 4)
#define VAUDIO_MASK	(3 << 4)
#define VSD_1_8		(0 << 6)
#define VSD_2_0		(1 << 6)
#define VSD_2_6		(2 << 6)
#define VSD_2_7		(3 << 6)
#define VSD_2_8		(4 << 6)
#define VSD_2_9		(5 << 6)
#define VSD_3_0		(6 << 6)
#define VSD_3_15	(7 << 6)
#define VSD_MASK	(7 << 6)
#define VGEN1_1_2	0
#define VGEN1_1_5	1
#define VGEN1_2_775	2
#define VGEN1_3_15	3
#define VGEN1_MASK	3
#define VGEN2_1_2	(0 << 6)
#define VGEN2_1_5	(1 << 6)
#define VGEN2_1_6	(2 << 6)
#define VGEN2_1_8	(3 << 6)
#define VGEN2_2_7	(4 << 6)
#define VGEN2_2_8	(5 << 6)
#define VGEN2_3_0	(6 << 6)
#define VGEN2_3_15	(7 << 6)
#define VGEN2_MASK	(7 << 6)

/* Fields in REG_SETTING_1 */
#define VGEN3_1_8	(0 << 14)
#define VGEN3_2_9	(1 << 14)
#define VGEN3_MASK	(1 << 14)
#define VDIG_1_05	(0 << 4)
#define VDIG_1_25	(1 << 4)
#define VDIG_1_65	(2 << 4)
#define VDIG_1_8	(3 << 4)
#define VDIG_MASK	(3 << 4)
#define VCAM_2_5	(0 << 16)
#define VCAM_2_6	(1 << 16)
#define VCAM_2_75	(2 << 16)
#define VCAM_3_0	(3 << 16)
#define VCAM_MASK	(3 << 16)

/* Reg Mode 0 */
#define VGEN1EN		(1 << 0)
#define VGEN1STBY	(1 << 1)
#define VGEN1MODE	(1 << 2)
#define VIOHIEN		(1 << 3)
#define VIOHISTBY	(1 << 4)
#define VDIGEN		(1 << 9)
#define VDIGSTBY	(1 << 10)
#define VGEN2EN		(1 << 12)
#define VGEN2STBY	(1 << 13)
#define VGEN2MODE	(1 << 14)
#define VPLLEN		(1 << 15)
#define VPLLSTBY	(1 << 16)
#define VUSBEN		(1 << 18)
#define VUSBSTBY	(1 << 19)

/* Reg Mode 1 */
#define VGEN3EN		(1 << 0)
#define VGEN3STBY	(1 << 1)
#define VGEN3MODE	(1 << 2)
#define VGEN3CONFIG	(1 << 3)
#define VCAMEN		(1 << 6)
#define VCAMSTBY	(1 << 7)
#define VCAMMODE	(1 << 8)
#define VCAMCONFIG	(1 << 9)
#define VVIDEOEN	(1 << 12)
#define VIDEOSTBY	(1 << 13)
#define VVIDEOMODE	(1 << 14)
#define VAUDIOEN	(1 << 15)
#define VAUDIOSTBY	(1 << 16)
#define VSDEN		(1 << 18)
#define VSDSTBY		(1 << 19)
#define VSDMODE		(1 << 20)

/* Reg Power Control 2*/
#define WDIRESET	(1 << 12)

/* SWx Output Volts */
#define SWX_OUT_MASK	0x1F
#define SWX_OUT_1_25	0x1A
#define SWX_OUT_1_30    0X1C

/* Buck Switchers (SW1,2,3,4) Output Voltage */
/*
 * NOTE: These values are for SWxHI = 0,
 * SWxHI = 1 adds 0.5V to the desired voltage
 */
#define SWx_0_600V	0
#define SWx_0_625V	1
#define SWx_0_650V	2
#define SWx_0_675V	3
#define SWx_0_700V	4
#define SWx_0_725V	5
#define SWx_0_750V	6
#define SWx_0_775V	7
#define SWx_0_800V	8
#define SWx_0_825V	9
#define SWx_0_850V	10
#define SWx_0_875V	11
#define SWx_0_900V	12
#define SWx_0_925V	13
#define SWx_0_950V	14
#define SWx_0_975V	15
#define SWx_1_000V	16
#define SWx_1_025V	17
#define SWx_1_050V	18
#define SWx_1_075V	19
#define SWx_1_100V	20
#define SWx_1_125V	21
#define SWx_1_150V	22
#define SWx_1_175V	23
#define SWx_1_200V	24
#define SWx_1_225V	25
#define SWx_1_250V	26
#define SWx_1_275V	27
#define SWx_1_300V	28
#define SWx_1_325V	29
#define SWx_1_350V	30
#define SWx_1_375V	31
#define SWx_VOLT_MASK	0x1F

#endif
