/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2024 SiFive, Inc.
 *
 * Zong Li <zong.li@sifve.com>
 */

#ifndef _ASM_RISCV_EEPROM_H
#define _ASM_RISCV_EEPROM_H

#define PCB_REVISION_REV3      0x3

u8 get_pcb_revision_from_eeprom(void);

#endif /* _ASM_RISCV_EEPROM_H */
