/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright Siemens AG 2023
 *
 * Common board definitions for siemens boards
 */

#ifndef _COMMON_EEPROM_H_
#define _COMMON_EEPROM_H_

/* EEPROM @ I2C */
#define SIEMENS_EE_I2C_BUS	0
#define SIEMENS_EE_I2C_ADDR	0x50

/* EEPROM mapping */
#define SIEMENS_EE_ADDR_NAND_GEO	0x80
#define SIEMENS_EE_ADDR_DDR3		0x90
#define SIEMENS_EE_ADDR_CHIP		0x120
#define SIEMENS_EE_ADDR_FACTORYSET	0x400

int siemens_ee_setup(void);
int siemens_ee_read_data(uint address, uchar *buffer, int len);

#endif /* _COMMON_EEPROM_H_ */
