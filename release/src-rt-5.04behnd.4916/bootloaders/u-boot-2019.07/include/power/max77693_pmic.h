/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013 Samsung Electronics
 * Piotr Wilczek <p.wilczek@samsung.com>
 */

#ifndef __MAX77693_PMIC_H_
#define __MAX77693_PMIC_H_

#include <power/power_chrg.h>

#define CHARGER_MIN_CURRENT 200
#define CHARGER_MAX_CURRENT 2000

#define MAX77693_CHG_PREFIX	"max77693-chg:"

/* Registers */

#define MAX77693_CHG_BASE	0xB0
#define MAX77693_CHG_INT_OK	0xB2
#define MAX77693_CHG_CNFG_00	0xB7
#define MAX77693_CHG_CNFG_02	0xB9
#define MAX77693_CHG_CNFG_06	0xBD
#define MAX77693_SAFEOUT	0xC6

#define PMIC_NUM_OF_REGS	0xC7

#define MAX77693_CHG_DETBAT	(0x1 << 7)	/* MAX77693_CHG_INT_OK */
#define MAX77693_CHG_MODE_ON	0x05		/* MAX77693_CHG_CNFG_00 */
#define MAX77693_CHG_CC		0x3F		/* MAX77693_CHG_CNFG_02	*/
#define MAX77693_CHG_LOCK	(0x0 << 2)	/* MAX77693_CHG_CNFG_06	*/
#define MAX77693_CHG_UNLOCK	(0x3 << 2)	/* MAX77693_CHG_CNFG_06	*/

#define MAX77693_ENSAFEOUT1	(1 << 6)
#define MAX77693_ENSAFEOUT2	(1 << 7)

#define MAX77693_PMIC_I2C_ADDR	(0xCC >> 1)

int pmic_init_max77693(unsigned char bus);
#endif /* __MAX77693_PMIC_H_ */
