/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  Copyright (C) 2015 Samsung Electronics
 *  Przemyslaw Marczak  <p.marczak@samsung.com>
 */

#ifndef _DT_BINDINGS_SANDBOX_PMIC_H_
#define _DT_BINDINGS_SANDBOX_PMIC_H_

/*
 * Sandbox PMIC - prepare reset values
 * To provide the default (reset) values as in the real hardware,
 * the registers are set in i2c pmic emul driver's probe() method.
 * The default values are defined as below.
 */

/* Buck operation mode IDs */
#define BUCK_OM_OFF	0
#define BUCK_OM_ON	1
#define BUCK_OM_PWM	2
#define BUCK_OM_COUNT	3

/* Ldo operation mode IDs */
#define LDO_OM_OFF	0
#define LDO_OM_ON	1
#define LDO_OM_SLEEP	2
#define LDO_OM_STANDBY	3
#define LDO_OM_COUNT	4

/* [Value uV/uA]/[Mode ID] to register */
#define VAL2REG(min, step, val)		(((val) - (min)) / (step))
#define VAL2OMREG(x)			(x)

#endif
