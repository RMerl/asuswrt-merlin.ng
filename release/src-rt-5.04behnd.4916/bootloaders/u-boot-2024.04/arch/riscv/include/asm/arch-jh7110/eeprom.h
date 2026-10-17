/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2023 StarFive Technology Co., Ltd.
 * Author: Yanhong Wang<yanhong.wang@starfivetech.com>
 */

#ifndef _ASM_RISCV_EEPROM_H
#define _ASM_RISCV_EEPROM_H

#include <linux/types.h>

u8 get_pcb_revision_from_eeprom(void);
u32 get_ddr_size_from_eeprom(void);

#endif /* _ASM_RISCV_EEPROM_H */
