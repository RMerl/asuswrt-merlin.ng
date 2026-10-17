/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2019
 * Phytium Technology Ltd <www.phytium.com>
 * shuyiqi <shuyiqi@phytium.com.cn>
 */

#ifndef _FT_DURIAN_H
#define _FT_DURIAN_H

/* FLUSH L3 CASHE */
#define HNF_COUNT           0x8
#define HNF_PSTATE_REQ      (HNF_BASE + 0x10)
#define HNF_PSTATE_STAT     (HNF_BASE + 0x18)
#define HNF_PSTATE_OFF      0x0
#define HNF_PSTATE_SFONLY   0x1
#define HNF_PSTATE_HALF     0x2
#define HNF_PSTATE_FULL     0x3
#define HNF_STRIDE          0x10000
#define HNF_BASE            (unsigned long)(0x3A200000)

#endif /* _FT_DURIAN_H */
