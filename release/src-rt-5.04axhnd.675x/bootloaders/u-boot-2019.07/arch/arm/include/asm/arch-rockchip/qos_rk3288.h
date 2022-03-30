/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2016 Rockchip Inc.
 */
#ifndef _ASM_ARCH_QOS_RK3288_H
#define _ASM_ARCH_QOS_RK3288_H

#define PRIORITY_HIGH_SHIFT	2
#define PRIORITY_LOW_SHIFT	0

#define CPU_AXI_QOS_PRIORITY    0x08

#define VIO0_VOP_QOS            0xffad0400
#define VIO1_VOP_QOS            0xffad0000
#define VIO1_ISP_R_QOS          0xffad0900
#define VIO1_ISP_W0_QOS         0xffad0100
#define VIO1_ISP_W1_QOS         0xffad0180

#endif
