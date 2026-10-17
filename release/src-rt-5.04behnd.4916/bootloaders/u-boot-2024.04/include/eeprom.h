/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef __EEPROM_LEGACY_H
#define __EEPROM_LEGACY_H

#if defined(CONFIG_CMD_EEPROM) || defined(CONFIG_ENV_IS_IN_EEPROM)
void eeprom_init(int bus);
int eeprom_read(uint dev_addr, uint offset, uchar *buffer, uint cnt);
int eeprom_write(uint dev_addr, uint offset, uchar *buffer, uint cnt);
#else
/*
 * Some EEPROM code is depecated because it used the legacy I2C interface. Add
 * some macros here so we don't have to touch every one of those uses
 */
#define eeprom_init(bus)
#define eeprom_read(dev_addr, offset, buffer, cnt) (-ENOSYS)
#define eeprom_write(dev_addr, offset, buffer, cnt) (-ENOSYS)
#endif

#endif
