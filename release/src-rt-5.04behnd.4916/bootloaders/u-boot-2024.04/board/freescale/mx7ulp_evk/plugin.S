/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 * Copyright 2019 NXP
 */

#include <config.h>

.macro imx7ulp_ddr_freq_decrease
	ldr r2, =0x403f0000
	ldr r3, =0x00000000
	str r3, [r2, #0xdc]

	ldr r2, =0x403e0000
	ldr r3, =0x01000020
	str r3, [r2, #0x40]
	ldr r3, =0x01000000
	str r3, [r2, #0x500]

	ldr r3, =0x80808080
	str r3, [r2, #0x50c]
	ldr r3, =0x00160002
	str r3, [r2, #0x508]
	ldr r3, =0x00000001
	str r3, [r2, #0x510]
	ldr r3, =0x00000014
	str r3, [r2, #0x514]
	ldr r3, =0x00000001
	str r3, [r2, #0x500]

	ldr r3, =0x01000000
wait1:
	ldr r4, [r2, #0x500]
	and r4, r3
	cmp r4, r3
	bne wait1

	ldr r3, =0x8080801B
	str r3, [r2, #0x50c]

	ldr r3, =0x00000040
wait2:
	ldr r4, [r2, #0x50c]
	and r4, r3
	cmp r4, r3
	bne wait2

	ldr r3, =0x00000001
	str r3, [r2, #0x30]
	ldr r3, =0x11000020
	str r3, [r2, #0x40]

	ldr r2, =0x403f0000
	ldr r3, =0x42000000
	str r3, [r2, #0xdc]

.endm

.macro imx7ulp_evk_ddr_setting

	imx7ulp_ddr_freq_decrease

	/* Enable MMDC PCC clock */
	ldr r2, =0x40b30000
	ldr r3, =0x40000000
	str r3, [r2, #0xac]

	/* Configure DDR pad */
	ldr r0, =0x40ad0000
	ldr r1, =0x00040000
	str r1, [r0, #0x128]
	ldr r1, =0x0
	str r1, [r0, #0xf8]
	ldr r1, =0x00000180
	str r1, [r0, #0xd8]
	ldr r1, =0x00000180
	str r1, [r0, #0x108]
	ldr r1, =0x00000180
	str r1, [r0, #0x104]
	ldr r1, =0x00010000
	str r1, [r0, #0x124]
	ldr r1, =0x0000018C
	str r1, [r0, #0x80]
	ldr r1, =0x0000018C
	str r1, [r0, #0x84]
	ldr r1, =0x0000018C
	str r1, [r0, #0x88]
	ldr r1, =0x0000018C
	str r1, [r0, #0x8c]

	ldr r1, =0x00010000
	str r1, [r0, #0x120]
	ldr r1, =0x00000180
	str r1, [r0, #0x10c]
	ldr r1, =0x00000180
	str r1, [r0, #0x110]
	ldr r1, =0x00000180
	str r1, [r0, #0x114]
	ldr r1, =0x00000180
	str r1, [r0, #0x118]
	ldr r1, =0x00000180
	str r1, [r0, #0x90]
	ldr r1, =0x00000180
	str r1, [r0, #0x94]
	ldr r1, =0x00000180
	str r1, [r0, #0x98]
	ldr r1, =0x00000180
	str r1, [r0, #0x9c]
	ldr r1, =0x00040000
	str r1, [r0, #0xe0]
	ldr r1, =0x00040000
	str r1, [r0, #0xe4]

	ldr r0, =0x40ab0000
	ldr r1, =0x00008000
	str r1, [r0, #0x1c]
	ldr r1, =0xA1390003
	str r1, [r0, #0x800]
	ldr r1, =0x0D3900A0
	str r1, [r0, #0x85c]
	ldr r1, =0x00400000
	str r1, [r0, #0x890]

	ldr r1, =0x40404040
	str r1, [r0, #0x848]
	ldr r1, =0x40404040
	str r1, [r0, #0x850]
	ldr r1, =0x33333333
	str r1, [r0, #0x81c]
	ldr r1, =0x33333333
	str r1, [r0, #0x820]
	ldr r1, =0x33333333
	str r1, [r0, #0x824]
	ldr r1, =0x33333333
	str r1, [r0, #0x828]

	ldr r1, =0x24922492
	str r1, [r0, #0x8c0]
	ldr r1, =0x00000800
	str r1, [r0, #0x8b8]

	ldr r1, =0x00020052
	str r1, [r0, #0x4]
	ldr r1, =0x292C42F3
	str r1, [r0, #0xc]
	ldr r1, =0x00100A22
	str r1, [r0, #0x10]
	ldr r1, =0x00120556
	str r1, [r0, #0x38]
	ldr r1, =0x00C700DB
	str r1, [r0, #0x14]
	ldr r1, =0x00211718
	str r1, [r0, #0x18]

	ldr r1, =0x0F9F26D2
	str r1, [r0, #0x2c]
	ldr r1, =0x009F0E10
	str r1, [r0, #0x30]
	ldr r1, =0x0000003F
	str r1, [r0, #0x40]
	ldr r1, =0xC3190000
	str r1, [r0, #0x0]

	ldr r1, =0x00008010
	str r1, [r0, #0x1c]
	ldr r1, =0x00008018
	str r1, [r0, #0x1c]
	ldr r1, =0x003F8030
	str r1, [r0, #0x1c]
	ldr r1, =0x003F8038
	str r1, [r0, #0x1c]
	ldr r1, =0xFF0A8030
	str r1, [r0, #0x1c]
	ldr r1, =0xFF0A8038
	str r1, [r0, #0x1c]
	ldr r1, =0x04028030
	str r1, [r0, #0x1c]
	ldr r1, =0x04028038
	str r1, [r0, #0x1c]
	ldr r1, =0x83018030
	str r1, [r0, #0x1c]
	ldr r1, =0x83018038
	str r1, [r0, #0x1c]
	ldr r1, =0x01038030
	str r1, [r0, #0x1c]
	ldr r1, =0x01038038
	str r1, [r0, #0x1c]

	ldr r1, =0x20000000
	str r1, [r0, #0x83c]

	ldr r1, =0x00001800
	str r1, [r0, #0x20]
	ldr r1, =0xA1310000
	str r1, [r0, #0x800]
	ldr r1, =0x00020052
	str r1, [r0, #0x4]
	ldr r1, =0x00011006
	str r1, [r0, #0x404]
	ldr r1, =0x00000000
	str r1, [r0, #0x1c]

.endm

.macro imx7ulp_clock_gating
.endm

.macro imx7ulp_qos_setting
.endm

.macro imx7ulp_ddr_setting
	imx7ulp_evk_ddr_setting
.endm

/* include the common plugin code here */
#include <asm/arch/mx7ulp_plugin.S>
