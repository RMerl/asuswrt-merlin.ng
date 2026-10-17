/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2016 Freescale Semiconductor, Inc.
 * Hou Zhiqiang <Zhiqiang.Hou@freescale.com>
 */

#ifndef __MC34VR500_H_
#define __MC34VR500_H_

#include <power/pmic.h>

#define MC34VR500_I2C_ADDR	0x08

/* Drivers name */
#define MC34VR500_REGULATOR_DRIVER	"mc34vr500_regulator"

/* Register map */
enum {
	MC34VR500_DEVICEID		= 0x00,

	MC34VR500_SILICONREVID		= 0x03,
	MC34VR500_FABID,
	MC34VR500_INTSTAT0,
	MC34VR500_INTMASK0,
	MC34VR500_INTSENSE0,
	MC34VR500_INTSTAT1,
	MC34VR500_INTMASK1,
	MC34VR500_INTSENSE1,

	MC34VR500_INTSTAT4		= 0x11,
	MC34VR500_INTMASK4,
	MC34VR500_INTSENSE4,

	MC34VR500_PWRCTL		= 0x1B,

	MC34VR500_SW1VOLT		= 0x2E,
	MC34VR500_SW1STBY,
	MC34VR500_SW1OFF,
	MC34VR500_SW1MODE,
	MC34VR500_SW1CONF,
	MC34VR500_SW2VOLT,
	MC34VR500_SW2STBY,
	MC34VR500_SW2OFF,
	MC34VR500_SW2MODE,
	MC34VR500_SW2CONF,

	MC34VR500_SW3VOLT		= 0x3C,
	MC34VR500_SW3STBY,
	MC34VR500_SW3OFF,
	MC34VR500_SW3MODE,
	MC34VR500_SW3CONF,

	MC34VR500_SW4VOLT		= 0x4A,
	MC34VR500_SW4STBY,
	MC34VR500_SW4OFF,
	MC34VR500_SW4MODE,
	MC34VR500_SW4CONF,

	MC34VR500_REFOUTCRTRL		= 0x6A,

	MC34VR500_LDO1CTL		= 0x6D,
	MC34VR500_LDO2CTL,
	MC34VR500_LDO3CTL,
	MC34VR500_LDO4CTL,
	MC34VR500_LDO5CTL,

	MC34VR500_PAGE_REGISTER		= 0x7F,

	/* Internal RAM */
	MC34VR500_SW1_VOLT		= 0xA8,
	MC34VR500_SW1_SEQ,
	MC34VR500_SW1_CONFIG,

	MC34VR500_SW2_VOLT		= 0xAC,
	MC34VR500_SW2_SEQ,
	MC34VR500_SW2_CONFIG,

	MC34VR500_SW3_VOLT		= 0xB0,
	MC34VR500_SW3_SEQ,
	MC34VR500_SW3_CONFIG,

	MC34VR500_SW4_VOLT		= 0xB8,
	MC34VR500_SW4_SEQ,
	MC34VR500_SW4_CONFIG,

	MC34VR500_REFOUT_SEQ		= 0xC4,

	MC34VR500_LDO1_VOLT		= 0xCC,
	MC34VR500_LDO1_SEQ,

	MC34VR500_LDO2_VOLT		= 0xD0,
	MC34VR500_LDO2_SEQ,

	MC34VR500_LDO3_VOLT		= 0xD4,
	MC34VR500_LDO3_SEQ,

	MC34VR500_LDO4_VOLT		= 0xD8,
	MC34VR500_LDO4_SEQ,

	MC34VR500_LDO5_VOLT		= 0xDC,
	MC34VR500_LDO5_SEQ,

	MC34VR500_PU_CONFIG1		= 0xE0,

	MC34VR500_TBB_POR		= 0xE4,

	MC34VR500_PWRGD_EN		= 0xE8,

	MC34VR500_NUM_OF_REGS,
};

/* Registor offset based on SWxVOLT register */
#define MC34VR500_VOLT_OFFSET	0
#define MC34VR500_STBY_OFFSET	1
#define MC34VR500_OFF_OFFSET	2
#define MC34VR500_MODE_OFFSET	3
#define MC34VR500_CONF_OFFSET	4

#define SW_MODE_MASK	0xf
#define SW_MODE_SHIFT	0

#define LDO_VOL_MASK	0xf
#define LDO_EN		(1 << 4)
#define LDO_MODE_SHIFT	4
#define LDO_MODE_MASK	(1 << 4)
#define LDO_MODE_OFF	0
#define LDO_MODE_ON	1

#define REFOUTEN	(1 << 4)

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

enum swx {
	SW1 = 0,
	SW2,
	SW3,
	SW4,
};

int mc34vr500_get_sw_volt(uint8_t sw);
int mc34vr500_set_sw_volt(uint8_t sw, int sw_volt);
int power_mc34vr500_init(unsigned char bus);
#endif /* __MC34VR500_PMIC_H_ */
