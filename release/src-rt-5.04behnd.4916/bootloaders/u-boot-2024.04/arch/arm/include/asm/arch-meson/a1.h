/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2023 SberDevices, Inc.
 * Author: Igor Prusov <ivprusov@sberdevices.ru>
 */

#ifndef __MESON_A1_H__
#define __MESON_A1_H__

#define A1_SYSCTRL_BASE			0xfe005800

/* SYSCTRL registers */
#define A1_SYSCTRL_ADDR(off)		(A1_SYSCTRL_BASE + ((off) << 2))

#define A1_SYSCTRL_SEC_STATUS_REG4	A1_SYSCTRL_ADDR(0xc4)

#define A1_SYSCTRL_MEM_SIZE_MASK	0xFFFF0000
#define A1_SYSCTRL_MEM_SIZE_SHIFT	16

#endif /* __MESON_A1_H__ */
