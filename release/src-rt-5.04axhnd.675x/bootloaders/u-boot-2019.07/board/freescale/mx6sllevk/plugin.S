/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 */

#include <config.h>

/* DDR script */
.macro imx6sll_evk_ddr_setting
	ldr r0, =IOMUXC_BASE_ADDR
	ldr r1, =0x00080000
	str r1, [r0, #0x550]
	ldr r1, =0x00000000
	str r1, [r0, #0x534]
	ldr r1, =0x00000030
	str r1, [r0, #0x2AC]
	str r1, [r0, #0x548]
	str r1, [r0, #0x52C]
	ldr r1, =0x00020000
	str r1, [r0, #0x530]
	ldr r1, =0x00003030
	str r1, [r0, #0x2B0]
	str r1, [r0, #0x2B4]
	str r1, [r0, #0x2B8]
	str r1, [r0, #0x2BC]

	ldr r1, =0x00020000
	str r1, [r0, #0x540]
	ldr r1, =0x00000030
	str r1, [r0, #0x544]
	str r1, [r0, #0x54C]
	str r1, [r0, #0x554]
	str r1, [r0, #0x558]
	str r1, [r0, #0x294]
	str r1, [r0, #0x298]
	str r1, [r0, #0x29C]
	str r1, [r0, #0x2A0]

	ldr r1, =0x00082030
	str r1, [r0, #0x2C0]

	ldr r0, =MMDC_P0_BASE_ADDR
	ldr r1, =0x00008000
	str r1, [r0, #0x1C]
	ldr r1, =0xA1390003
	str r1, [r0, #0x800]
	ldr r1, =0x084700C7
	str r1, [r0, #0x85C]
	ldr r1, =0x00400000
	str r1, [r0, #0x890]

	ldr r1, =0x3F393B3C
	str r1, [r0, #0x848]
	ldr r1, =0x262C3826
	str r1, [r0, #0x850]

	ldr r1, =0x33333333
	str r1, [r0, #0x81C]
	str r1, [r0, #0x820]
	str r1, [r0, #0x824]
	str r1, [r0, #0x828]

	ldr r1, =0xf3333333
	str r1, [r0, #0x82C]
	str r1, [r0, #0x830]
	str r1, [r0, #0x834]
	str r1, [r0, #0x838]

	ldr r1, =0x24922492
	str r1, [r0, #0x8C0]
	ldr r1, =0x00000800
	str r1, [r0, #0x8B8]

	ldr r1, =0x00020052
	str r1, [r0, #0x004]
	ldr r1, =0x53574333
	str r1, [r0, #0x00C]
	ldr r1, =0x00100B22
	str r1, [r0, #0x010]
	ldr r1, =0x00170778
	str r1, [r0, #0x038]
	ldr r1, =0x00C700DB
	str r1, [r0, #0x014]
	ldr r1, =0x00201718
	str r1, [r0, #0x018]
	ldr r1, =0x0F9F26D2
	str r1, [r0, #0x02C]
	ldr r1, =0x009F0E10
	str r1, [r0, #0x030]
	ldr r1, =0x0000005F
	str r1, [r0, #0x040]
	ldr r1, =0xC4190000
	str r1, [r0, #0x000]
	ldr r1, =0x20000000
	str r1, [r0, #0x83C]

	ldr r1, =0x00008050
	str r1, [r0, #0x01C]
	ldr r1, =0x00008058
	str r1, [r0, #0x01C]
	ldr r1, =0x003F8030
	str r1, [r0, #0x01C]
	ldr r1, =0x003F8038
	str r1, [r0, #0x01C]
	ldr r1, =0xFF0A8030
	str r1, [r0, #0x01C]
	ldr r1, =0xFF0A8038
	str r1, [r0, #0x01C]
	ldr r1, =0x04028030
	str r1, [r0, #0x01C]
	ldr r1, =0x04028038
	str r1, [r0, #0x01C]
	ldr r1, =0x83018030
	str r1, [r0, #0x01C]
	ldr r1, =0x83018038
	str r1, [r0, #0x01C]
	ldr r1, =0x01038030
	str r1, [r0, #0x01C]
	ldr r1, =0x01038038
	str r1, [r0, #0x01C]

	ldr r1, =0x00001800
	str r1, [r0, #0x020]
	ldr r1, =0xA1390003
	str r1, [r0, #0x800]
	ldr r1, =0x00020052
	str r1, [r0, #0x004]
	ldr r1, =0x00011006
	str r1, [r0, #0x404]
	ldr r1, =0x00000000
	str r1, [r0, #0x01C]
.endm

.macro imx6_clock_gating
	ldr r0, =CCM_BASE_ADDR
	ldr r1, =0xffffffff
	str r1, [r0, #0x068]
	str r1, [r0, #0x06c]
	str r1, [r0, #0x070]
	str r1, [r0, #0x074]
	str r1, [r0, #0x078]
	str r1, [r0, #0x07c]
	str r1, [r0, #0x080]
.endm

.macro imx6_qos_setting
.endm

.macro imx6_ddr_setting
	imx6sll_evk_ddr_setting
.endm

/* include the common plugin code here */
#include <asm/arch/mx6_plugin.S>
