/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2020 Toradex
 */

#ifndef _TDX_EEPROM_H
#define _TDX_EEPROM_H

#include <i2c_eeprom.h>

int read_tdx_eeprom_data(u32 eeprom_id, int offset, uint8_t *buf, int size);
int write_tdx_eeprom_data(u32 eeprom_id, int offset, uint8_t *buf, int size);

#endif /* _TDX_EEPROM_H */
