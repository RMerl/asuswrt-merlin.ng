/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2010
 * Reinhard Meyer, reinhard.meyer@emk-elektronik.de
 *
 * General Purpose Backup Registers
 * Based on AT91SAM9XE datasheet
 */

#ifndef AT91_GPBR_H
#define AT91_GPBR_H

/*
 * The Atmel AT91SAM9 series has a small resource of 4 nonvolatile
 * 32 Bit registers (buffered by the Vbu power).
 *
 * Please consider carefully before using this resource for tasks
 * that do not really need nonvolatile registers. Maybe you can
 * store information in EEPROM or FLASH instead.
 *
 * However, if you use a GPBR please document its use here and
 * reference the define in your code!
 *
 * known typical uses of the GPBRs:
 * GPBR[0]: offset for RTT timekeeping (u-boot, kernel)
 * GPBR[1]: unused
 * GPBR[2]: unused
 * GPBR[3]: bootcount (u-boot)
 */
#define AT91_GPBR_INDEX_TIMEOFF 0
#define AT91_GPBR_INDEX_BOOTCOUNT 3

#ifndef __ASSEMBLY__

typedef struct at91_gpbr {
	u32 reg[4];
} at91_gpbr_t;

#endif /* __ASSEMBLY__ */

#endif
