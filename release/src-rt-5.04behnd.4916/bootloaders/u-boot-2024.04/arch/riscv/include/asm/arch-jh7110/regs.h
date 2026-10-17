/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2022 StarFive Technology Co., Ltd.
 * Author: Yanhong Wang <yanhong.wang@starfivetech.com>
 */

#ifndef __STARFIVE_JH7110_REGS_H
#define __STARFIVE_JH7110_REGS_H

#define JH7110_SYS_CRG			0x13020000
#define JH7110_SYS_SYSCON		0x13030000
#define JH7110_SYS_IOMUX		0x13040000
#define JH7110_AON_CRG			0x17000000
#define JH7110_AON_SYSCON		0x17010000

#define JH7110_BOOT_MODE_SELECT_REG	0x1702002c
#define JH7110_BOOT_MODE_SELECT_MASK	GENMASK(1, 0)

#endif /* __STARFIVE_JH7110_REGS_H */
