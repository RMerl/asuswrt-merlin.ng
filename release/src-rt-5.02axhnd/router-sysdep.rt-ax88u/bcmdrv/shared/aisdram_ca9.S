/*
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: aisdram_ca9.S 654158 2016-08-11 09:30:01Z $
 */

#include <arminc.h>
#include <sbhndarm.h>
#include <hndsoc.h>
#include <ddr_core.h>
#include <bcmnvram.h>
#include <sbchipc.h>
#include <chipcommonb.h>

#ifndef __arm__
#error __arm__ is NOT defined
#endif // endif

/* Routine begin/end macro */
#if defined(__thumb__)
#define FUNC(x) THUMBLEAF(x)
#else
#define FUNC(x) LEAF(x)
#endif  /* __thumb__ */

	.text

FUNC(ddr_init_ca9)
	mov	r9,r0			/* ddr control register base */
	mov	r8,r1
	mov	r7,r2

	/* ddr2 or ddr3 */
	ldr	r3,=0x18108500
	ldr	r0,[r3]
	ldr	r2,=DDR_STAT_DDR3
	and	r0,r0,r2
	cmp	r0,r2
	bne	chk_bootdev

	/* Configure DDR3 voltage to 1.5V */
	ldr	r1,=0x1800c02c
	ldr	r2,=0x00000200
	str	r2,[r1]
	ldr	r3,[r1]
	/* Enable LDO voltage output */
	ldr	r1,=0x1800c020			/* pcu_aopc_control */
	ldr	r2,=0x80000000
	str	r2,[r1]
	ldr	r3,[r1]
	ldr	r2,=0x80000001
	str	r2,[r1]
	ldr	r3,[r1]
	ldr	r4,=0x1800c024			/* pcu_status */
	ldr	r3,[r4]
	bic	r3,r3,#0xf
	lsl	r3,r3,#1
	ldr	r4,=0x1e
	bic	r4,r4,r3
	orr	r2,r2,r3
	bic	r2,r2,#4
	str	r2,[r1]
	ldr	r3,[r1]

chk_bootdev:
	mov	r6,#0x0
	ldr	r0,=SISF_NS_BOOTDEV_ROM
	cmp	r0,r7
	bne	find_flashnvram

find_otpnvram:
	ldr	r0,=SI_ENUM_BASE_DEFAULT	/* r0:	core regs SI base address */
	ldr	r4,[r0,#CC_CAPABILITIES]	/* r4:	capabitilies */
	and	r4,r4,#CC_CAP_OTPSIZE
	cmp	r4,#0
	beq	find_flashnvram

	/* Skip OTP initialization here since it was already done in pcie_phy_init */
	lsr	r2,r4,#CC_CAP_OTPSIZE_SHIFT	/* Get OTP size */
	adrl	r1,otp_sizes
	lsl	r2,r2,#0x2
	ldr	r3,[r1,r2]			/* Get the OTP size from otp_sizes table (in bytes) */

	cmp	r3,#0x0				/* skip zero size */
	beq	find_flashnvram
	ldr	r1,[r0,#CC_OTPST]
	and	r1,r1,#OTPS_READY
	cmp	r1,#0x0				/* 0: OTP is not ready for whatever reason */
	beq	find_flashnvram
	add	r2,r3,r0			/* Seek to the end of OTP region */
	/* Scan entire OTP from END -> BEGIN to find first
	 * matching nvram entry */
	sub	r2,r2,#2
1:
	add	r4,r2,#CC_SROM_OTP
	ldrh	r1,[r4]
	lsl	r1,r1,#0x10
	sub	r2,r2,#2
	add	r4,r2,#CC_SROM_OTP
	ldrh	r3,[r4]
	orr	r1,r1,r3
	ldr	r4,=NVRAM_MAGIC
	cmp	r1,r4
	beq	1f
	cmp	r0,r2
	blt	1b
	b	find_flashnvram
	/* no nvram from OTP */

	/* Get sdram params from OTP 16bits at a time */
1:
	add	r2,r2,#12
	add	r4,r2,#CC_SROM_OTP
	ldrh	r1,[r4]
	add	r2,r2,#2
	add	r4,r2,#CC_SROM_OTP
	ldrh	r3,[r4]
	lsl	r3,r3,#0x10
	orr	r6,r1,r3			/* sdram_config + sdram_rehash */
	add	r2,r2,#2
	add	r4,r2,#CC_SROM_OTP
	ldrh	r1,[r4]
	add	r2,r2,#2
	add	r4,r2,#CC_SROM_OTP
	ldrh	r3,[r4]
	lsl	r3,r3,#0x10
	orr	r5,r1,r3			/* sdram_ncdl */

find_flashnvram:
	ldr	r0,=SI_NS_NANDFLASH
	ldr	r1,=SISF_NS_BOOTDEV_NAND
	cmp	r1,r7
	beq	nand_embedded_nv

	ldr	r0,=SI_NS_NORFLASH
	ldr	r1,=MAX_NVRAM_SPACE
	sub	r0,r0,r1
	ldr	r1,=FLASH_MIN
	ldr	r2,=SI_NS_FLASH_WINDOW
	ldr	r3,=NVRAM_MAGIC

1:	add	r4,r0,r1
	ldr	r8,[r4]
	cmp	r8,r3
	beq	read_config
	lsl	r1,r1,#1
	cmp	r1,r2
	ble	1b

	/* Try embedded NVRAM at 4KB and 1KB as last resorts */
	ldr	r0,=SI_NS_NORFLASH
nand_embedded_nv:
	add	r4,r0,#0x1000
	ldr	r8,[r4]
	cmp	r8,r3
	beq	read_config

	add	r4,r0,#0x400
	ldr	r8,[r4]
	cmp	r8,r3
	beq	read_config

2:	ldr	r0,=0x0			/* if we have sdram params from OTP then use it */
	cmp	r6,r0
	bne	set_ddr_clock

no_sdram_params:
	ldr	r6,=0x0
	ldr	r5,=0x0
	b	set_ddr_clock

read_config:
	ldr	r6,[r4,#12]		/* Pick up sdram_config & sdram_refresh */
	ldr	r5,[r4,#16]		/* Pick up sdram_ncdl */

set_ddr_clock:
	ldr	r0,=DDRC00_START
	ldr	r1,[r9,#DDRC_CONTROL00]
	and	r1,r1,r0
	cmp	r1,r0
	bne	init_ddrphy		/* The first time */
	ldr	r1,=0x1000
	ldr	r0,=BISZ_OFFSET
	add	r1,r1,r0
	sub	r1,r1,#4
	ldr	r0,=0x0
	ldr	r2,[r1]
	cmp	r2,r0
	beq	init_ddrphy		/* No ddrclk specified */

	ldr	r3,=DDR_TABLE_END
	adrl	r4,ddr_clk_tab
1:	ldr	r1,[r4]
	cmp	r1,r3
	beq	init_ddrphy
	cmp	r1,r2
	beq	chg_ddr_clock
	add	r4,r4,#12
	b	1b

chg_ddr_clock:
	/* Put the controller to reset mode first */
	ldr	r1,=0x18108800
	ldr	r2,=0x00000001
	str	r2,[r1]
	ldr	r1,=0x18109800
	str	r2,[r1]

	ldr	r0,=CRU_CLKSET_KEY
	ldr	r1,=0xea68
	str	r1,[r0]

	ldr	r0,=LCPLL_NDIV_INT
	ldr	r1,[r0]
	ldr	r2,=0xf00fffff
	and	r1,r1,r2
	ldr	r2,[r4,#4]
	orr	r1,r1,r2
	str	r1,[r0]
	ldr	r0,=LCPLL_CHX_MDIV
	ldr	r1,[r0]
	ldr	r2,=0x000000ff
	and	r1,r1,r2
	ldr	r2,[r4,#8]
	orr	r1,r1,r2
	str	r1,[r0]

en_change:
	ldr	r0,=LCPLL_LOAD_EN_CH
	ldr	r1,[r0]
	orr	r1,r1,#0x7
	str	r1,[r0]
	and	r1,r1,#0xfffffff8
	str	r1,[r0]
	ldr	r0,=CRU_CLKSET_KEY
	ldr	r1,=0x0
	str	r1,[r0]

init_ddrphy:
	mov	r8,r9		/* save r9 */

	ldr	r1,=0x18108800
	ldr	r2,=0x00000000
	str	r2,[r1]
	ldr	r1,=0x18109800
	str	r2,[r1]

	ldr	r1,=0x18108408
	ldr	r2,[r1]
	ldr	r3,=0xf000ffff
	and	r2,r2,r3
	orr	r2,r2,#0x01900000
	str	r2,[r1]

	ldr	r9,=0x18010800
	ldr	r1,=0x0
	ldr	r3,=0x00019000

wait_for_ddr_phy_up:
	cmp	r3,r1
	beq	ddr_phy_rdy
	sub	r3,r3,#0x1
	ldr	r2,=0x0
	ldr	r2,[r9]
	cmp	r2,r1
	bne	ddr_phy_rdy
	b	wait_for_ddr_phy_up
ddr_phy_rdy:

	/* Change PLL divider values inside PHY */
	ldr	r1,=0x1801081c
	ldr	r2,=0x00000c10
	str	r2,[r1]
	ldr	r3,[r9]

	ldr	r1,=0x18010814
	ldr	r2,=0x00000010
	str	r2,[r1]
	ldr	r3,[r9]

	ldr	r4,=0x18010810
	ldr	r1,=0x1
	ldr	r3,=0x00001400

wait_for_ddr_phy_pll_lock:
	cmp	r3,r1
	beq	ddr_phy_pll_lock_done
	sub	r3,r3,#0x1
	ldr	r2,=0x0
	ldr	r2,[r4]
	and	r2,r2,#0x1
	cmp	r2,r1
	beq	ddr_phy_pll_lock_done
	b	wait_for_ddr_phy_pll_lock
ddr_phy_pll_lock_done:

	ldr	r1,=0x18010b60
	ldr	r2,=0x00000003
	str	r2,[r1]
	ldr	r3,[r9]

	/* Write 2 if ddr2, 3 if ddr3 */
	ldr	r1,=0x18108500
	ldr	r3,[r1]
	ldr	r2,=0x00000001
	and	r3,r2,r3
	orr	r2,r3,#0x2
	ldr	r1,=0x18010bac
	str	r2,[r1]
	ldr	r3,[r9]

	ldr	r1,=0x1801083c
	ldr	r2,=0x00100000
	str	r2,[r1]
	ldr	r3,[r9]

	ldr	r1,=0x18010848
	ldr	r2,=0x08000101
	str	r2,[r1]
	ldr	r3,[r9]

	ldr	r4,=0x1801084C
	ldr	r1,=0x1
	ldr	r3,=0x00001400

wait_for_ddr_phy_calib_lock:
	cmp	r3,r1
	beq	ddr_phy_calib_lock_done
	sub	r3,r3,#0x1
	ldr	r2,=0x0
	ldr	r2,[r4]
	and	r2,r2,#0x1
	cmp	r2,r1
	beq	ddr_phy_calib_lock_done
	b	wait_for_ddr_phy_calib_lock
ddr_phy_calib_lock_done:

	ldr	r4,=0x1801084C
	ldr	r1,=0x2
	ldr	r2,[r4]
	and	r2,r2,#0x2
	cmp	r2,r1
	beq	ddr_cntrl_prog

calib_override:
	ldr	r1,=0x18010834
	ldr	r2,=0x0001003f
	str	r2,[r1]
	ldr	r3,[r9]

	ldr	r1,=0x18010a04
	ldr	r2,=0x0003003f
	str	r2,[r1]
	ldr	r3,[r9]

	ldr	r1,=0x18010a10
	ldr	r2,=0x0003003f
	str	r2,[r1]
	ldr	r3,[r9]

	ldr	r1,=0x18010a14
	ldr	r2,=0x0003003f
	str	r2,[r1]
	ldr	r3,[r9]

	ldr	r1,=0x18010a18
	ldr	r2,=0x0003003f
	str	r2,[r1]
	ldr	r3,[r9]

	ldr	r1,=0x18010a1c
	ldr	r2,=0x0003003f
	str	r2,[r1]
	ldr	r3,[r9]

	ldr	r1,=0x18010a20
	ldr	r2,=0x0003003f
	str	r2,[r1]
	ldr	r3,[r9]

	ldr	r1,=0x18010a24
	ldr	r2,=0x0003003f
	str	r2,[r1]
	ldr	r3,[r9]

	ldr	r1,=0x18010a28
	ldr	r2,=0x0003003f
	str	r2,[r1]
	ldr	r3,[r9]

	ldr	r1,=0x18010a2c
	ldr	r2,=0x0003003f
	str	r2,[r1]
	ldr	r3,[r9]

	ldr	r1,=0x18010a30
	ldr	r2,=0x0003003f
	str	r2,[r1]
	ldr	r3,[r9]

	ldr	r1,=0x18010aa4
	ldr	r2,=0x0003003f
	str	r2,[r1]
	ldr	r3,[r9]

	ldr	r1,=0x18010ab0
	ldr	r2,=0x0003003f
	str	r2,[r1]
	ldr	r3,[r9]

	ldr	r1,=0x18010ab4
	ldr	r2,=0x0003003f
	str	r2,[r1]
	ldr	r3,[r9]

	ldr	r1,=0x18010ab8
	ldr	r2,=0x0003003f
	str	r2,[r1]
	ldr	r3,[r9]

	ldr	r1,=0x18010abc
	ldr	r2,=0x0003003f
	str	r2,[r1]
	ldr	r3,[r9]

	ldr	r1,=0x18010ac0
	ldr	r2,=0x0003003f
	str	r2,[r1]
	ldr	r3,[r9]

	ldr	r1,=0x18010ac4
	ldr	r2,=0x0003003f
	str	r2,[r1]
	ldr	r3,[r9]

	ldr	r1,=0x18010ac8
	ldr	r2,=0x0003003f
	str	r2,[r1]
	ldr	r3,[r9]

	ldr	r1,=0x18010acc
	ldr	r2,=0x0003003f
	str	r2,[r1]
	ldr	r3,[r9]

	ldr	r1,=0x18010ad0
	ldr	r2,=0x0003003f
	str	r2,[r1]
	ldr	r3,[r9]

	ldr	r1,=0x18010a08
	ldr	r2,=0x0003003f
	str	r2,[r1]
	ldr	r3,[r9]

	ldr	r1,=0x18010a0c
	ldr	r2,=0x0003003f
	str	r2,[r1]
	ldr	r3,[r9]

	ldr	r1,=0x18010aa8
	ldr	r2,=0x0003003f
	str	r2,[r1]
	ldr	r3,[r9]

	ldr	r1,=0x18010aac
	ldr	r2,=0x0003003f
	str	r2,[r1]
	ldr	r3,[r9]

ddr_cntrl_prog:

	/* correct Vtt voltage */
	ldr	r1,=0x18010864
	ldr	r2,=0x01d7ffff
	str	r2,[r1]
	ldr	r3,[r9]

	ldr	r1,=0x18010868
	str	r2,[r1]
	ldr	r3,[r9]

	mov	r9,r8		/* restore r9 */

init_regs:

	/* ddr2 or ddr3 */
	ldr	r8,=0x18108500
	ldr	r0,[r8]
	ldr	r2,=DDR_STAT_DDR3
	and	r0,r0,r2
	cmp	r0,r2
	beq	ddr3_init

ddr2_init:
	mov	ip,lr			/* save lr across calls */
	adrl	r0,ddr2_init_tab
	bl	ddr_init_regs
	mov	lr,ip			/* restore lr */
	b	chparams

ddr3_init:
	mov	ip,lr			/* save lr across calls */
	adrl	r0,ddr3_init_tab
	bl	ddr_init_regs
	mov	lr,ip			/* restore lr */

chparams:
	ldr	r0,=0x0
	cmp	r6,r0
	beq	turnon			/* No sdram params, use default values */

chtref:
	lsr	r2,r6,#16
	cmp	r2,r0
	beq	chhalf
	ldr	r1,[r9,#DDRC_CONTROL21]
	ldr	r3,=0x3fff
	bic	r1,r1,r3
	orr	r1,r1,r2
	str	r1,[r9,#DDRC_CONTROL21]
	ldr	r1,[r9,#DDRC_CONTROL22]
	ldr	r3,=0x3fff
	bic	r1,r1,r3
	orr	r1,r1,r2
	str	r1,[r9,#DDRC_CONTROL22]

chhalf:
	ldr	r1,=0x80
	and	r1,r1,r6
	cmp	r1,r0
	beq	ch8banks
setreduc:
	ldr	r1,[r9,#DDRC_CONTROL87]
	ldr	r2,=0x00000100
	orr	r1,r1,r2
	str	r1,[r9,#DDRC_CONTROL87]

ch8banks:
	ldr	r1,=0x40
	and	r1,r1,r6
	cmp	r1,r0
	beq	do4banks
do8banks:
	ldr	r1,[r9,#DDRC_CONTROL82]
	ldr	r2,=~0x00000300
	and	r1,r1,r2
	str	r1,[r9,#DDRC_CONTROL82]
	b	docols
do4banks:
	ldr	r1,[r9,#DDRC_CONTROL82]
	ldr	r2,=~0x00000300
	and	r1,r1,r2
	ldr	r2,=0x00000100
	orr	r1,r1,r2
	str	r1,[r9,#DDRC_CONTROL82]

docols:
	ldr	r1,[r9,#DDRC_CONTROL82]
	ldr	r2,=~0x07070000
	and	r1,r1,r2
	ldr	r2,=0x700
	and	r2,r2,r6
	lsl	r2,r2,#16
	orr	r1,r1,r2
	str	r1,[r9,#DDRC_CONTROL82]

docaslat:
	ldr	r0,=0x1f
	and	r0,r0,r6		/* cas latency */
	sub	r1,r0,#1		/* wrlat */
	cmp	r0,#6
	blt	1f
	sub	r1,r0,#4
	lsr	r1,r1,#1
	sub	r1,r0,r1		/* wrlat */
1:
	ldr	r2,[r9,#DDRC_CONTROL05]
	ldr	r3,=~0x3f1f3f00
	and	r2,r2,r3
	lsl	r3,r0,#9		/* cas << 9 */
	orr	r2,r2,r3
	lsl	r3,r0,#25		/* cas << 25 */
	orr	r2,r2,r3
	lsl	r3,r1,#16		/* wrlat << 16 */
	orr	r2,r2,r3
	str	r2,[r9,#DDRC_CONTROL05]
	ldr	r2,[r9,#DDRC_CONTROL06]
	ldr	r3,=~0x0000001f
	and	r2,r2,r3
	orr	r2,r2,r1
	str	r2,[r9,#DDRC_CONTROL06]

	ldr	r2,[r9,#DDRC_CONTROL174]
	ldr	r3,=~0x00001f3f
	and	r2,r2,r3
	lsl	r3,r1,#8
	orr	r2,r2,r3
	sub	r3,r0,#1
	orr	r2,r2,r3
	str	r2,[r9,#DDRC_CONTROL174]

	ldr	r2,[r9,#DDRC_CONTROL44]
	ldr	r3,=~0x0000f000
	and	r2,r2,r3
	mov	r3,r0
	lsl	r3,r3,#12
	orr	r2,r2,r3
	str	r2,[r9,#DDRC_CONTROL44]

	ldr	r2,[r9,#DDRC_CONTROL186]
	ldr	r3,=~0xff000000
	and	r2,r2,r3
	add	r3,r0,#17
	lsl	r3,r3,#24
	orr	r2,r2,r3
	str	r2,[r9,#DDRC_CONTROL186]

	ldr	r8,=0x18108500
	ldr	r3,[r8]
	ldr	r2,=DDR_STAT_DDR3
	and	r3,r3,r2
	cmp	r3,r2
	bne	turnon

	ldr	r2,[r9,#DDRC_CONTROL44]
	ldr	r3,=~0x0000f000
	and	r2,r2,r3
	sub	r3,r0,#4
	lsl	r3,r3,#12
	orr	r2,r2,r3
	str	r2,[r9,#DDRC_CONTROL44]
	ldr	r2,[r9,#DDRC_CONTROL45]
	ldr	r3,=~0x00380000
	and	r2,r2,r3
	sub	r3,r0,#8
	cmp	r0,#9
	bgt	1f
	sub	r3,r0,#7
	cmp	r0,#7
	bgt	1f
	sub	r3,r0,#6
1:
	lsl	r3,r3,#19
	orr	r2,r2,r3
	str	r2,[r9,#DDRC_CONTROL45]

	ldr	r2,[r9,#DDRC_CONTROL206]
	ldr	r3,=~0x00001f00
	and	r2,r2,r3
	sub	r3,r1,#1
	lsl	r3,r3,#8
	orr	r2,r2,r3
	str	r2,[r9,#DDRC_CONTROL206]

turnon:
	ldr	r0,=DDRC00_START
	ldr	r1,[r9,#DDRC_CONTROL00]
	orr	r1,r1,r0
	str	r1,[r9,#DDRC_CONTROL00]

poll_ddr_ctrl:
	ldr	r0,[r9,#DDRC_CONTROL89]
	ldr	r2,=DDR_INT_INIT_DONE
	and	r0,r2,r0
	cmp	r0,r2
	bne	poll_ddr_ctrl

	ldr	r1,=0x18010b60
	ldr	r2,=0x00000003
	str	r2,[r1]
	ldr	r3,[r9]

	ldr	r1,=0x18010a00
	ldr	r2,=0x00010120
	str	r2,[r1]
	ldr	r3,[r9]

	ldr	r1,=0x18010a74
	ldr	r2,=0x0001000d
	str	r2,[r1]
	ldr	r3,[r9]

	ldr	r1,=0x18010b14
	ldr	r2,=0x00010020
	str	r2,[r1]
	ldr	r3,[r9]

	mov	pc,lr
	.ltorg

	.align 4
ddr_clk_tab:
	.word	333,	0x07800000,	0x1e0f1200
	.word	389,	0x08c00000,	0x23121200
	.word	400,	0x08000000,	0x20101000
	.word	533,	0x08000000,	0x20100c00
	.word	666,	0x07800000,	0x1e0f0900
	.word	775,	0x07c00000,	0x20100800
	.word	800,	0x08000000,	0x20100800
	.word	DDR_TABLE_END

	.align 4
ddr2_init_tab:
	.word	0,	0x00000400
	.word	1,	0x00000000
	.word	3,	0x00000050
	.word	4,	0x000000c8
	.word	5,	0x0c050c02
	.word	6,	0x04020405
	.word	7,	0x05031015
	.word	8,	0x03101504
	.word	9,	0x05020305
	.word	10,	0x03006d60
	.word	11,	0x05020303
	.word	12,	0x03006d60
	.word	13,	0x01000003
	.word	14,	0x05061001
	.word	15,	0x000b0b06
	.word	16,	0x030000c8
	.word	17,	0x00a01212
	.word	18,	0x060600a0
	.word	19,	0x00000000
	.word	20,	0x00003001
	.word	21,	0x00300c2d
	.word	22,	0x00050c2d
	.word	23,	0x00000200
	.word	24,	0x000a0002
	.word	25,	0x0002000a
	.word	26,	0x00020008
	.word	27,	0x00c80008
	.word	28,	0x00c80037
	.word	29,	0x00000037
	.word	30,	0x03000001
	.word	31,	0x00030303
	.word	36,	0x01000000
	.word	37,	0x10000000
	.word	38,	0x00100400
	.word	39,	0x00000400
	.word	40,	0x00000100
	.word	41,	0x00000000
	.word	42,	0x00000001
	.word	43,	0x00000000
	.word	44,	0x000a6300
	.word	45,	0x00000004
	.word	46,	0x00040a63
	.word	47,	0x00000000
	.word	48,	0x0a630000
	.word	49,	0x00000004
	.word	50,	0x00040a63
	.word	51,	0x00000000
	.word	52,	0x0a630000
	.word	53,	0x00000004
	.word	54,	0x00040a63
	.word	55,	0x00000000
	.word	56,	0x0a630000
	.word	57,	0x00000004
	.word	58,	0x00040a63
	.word	59,	0x00000000
	.word	60,	0x00000000
	.word	61,	0x00010100
	.word	78,	0x01000200
	.word	79,	0x02000040
	.word	80,	0x00400100
	.word	81,	0x00000200
	.word	82,	0x01030001
	.word	83,	0x01ffff0a
	.word	84,	0x01010101
	.word	85,	0x03010101
	.word	86,	0x01000003
	.word	87,	0x0000010c
	.word	88,	0x00010000
	.word	108,	0x02020101
	.word	109,	0x08080404
	.word	110,	0x03020200
	.word	111,	0x01000202
	.word	112,	0x00000200
	.word	116,	0x19000000
	.word	117,	0x00000028
	.word	118,	0x00000000
	.word	119,	0x00010001
	.word	120,	0x00010001
	.word	121,	0x00010001
	.word	122,	0x00010001
	.word	123,	0x00010001
	.word	128,	0x001c1c00
	.word	129,	0x1c1c0001
	.word	130,	0x00000001
	.word	133,	0x00011c1c
	.word	134,	0x00011c1c
	.word	137,	0x001c1c00
	.word	138,	0x1c1c0001
	.word	139,	0x00000001
	.word	142,	0x00011c1c
	.word	143,	0x00011c1c
	.word	144,	0x00000000
	.word	145,	0x00000000
	.word	146,	0x001c1c00
	.word	147,	0x1c1c0001
	.word	148,	0xffff0001
	.word	149,	0x00ffff00
	.word	150,	0x0000ffff
	.word	151,	0x00000000
	.word	152,	0x03030303
	.word	153,	0x03030303
	.word	156,	0x02006400
	.word	157,	0x02020202
	.word	158,	0x02020202
	.word	160,	0x01020202
	.word	161,	0x01010064
	.word	162,	0x01010101
	.word	163,	0x01010101
	.word	165,	0x00020101
	.word	166,	0x00000064
	.word	167,	0x00000000
	.word	168,	0x000a0a00
	.word	169,	0x0c2d0000
	.word	170,	0x02000200
	.word	171,	0x02000200
	.word	172,	0x00000c2d
	.word	173,	0x00003ce1
	.word	174,	0x0c2d0505
	.word	175,	0x02000200
	.word	176,	0x02000200
	.word	177,	0x00000c2d
	.word	178,	0x00003ce1
	.word	179,	0x02020505
	.word	180,	0x80000100
	.word	181,	0x04070303
	.word	182,	0x0000000a
	.word	185,	0x0010ffff
	.word	186,	0x16070303
	.word	187,	0x0000000f
	.word	194,	0x00000204
	.word	202,	0x00000050
	.word	203,	0x00000050
	.word	204,	0x00000000
	.word	205,	0x00000040
	.word	206,	0x01030301
	.word	207,	0x00000001
	.word	DDR_TABLE_END

	.align 4
ddr3_init_tab:
	.word	0,	0x00000600
	.word	1,	0x00000000
	.word	3,	0x000000a0
	.word	4,	0x00061a80
	.word	5,	0x16081600
	.word	6,	0x06040408
	.word	7,	0x0b061c27
	.word	8,	0x061c2706
	.word	9,	0x0c04060b
	.word	10,	0x0400db60
	.word	11,	0x0c040604
	.word	12,	0x0400db60
	.word	13,	0x01000004
	.word	14,	0x0b0c1001
	.word	15,	0x0017170c
	.word	16,	0x03000200
	.word	17,	0x00002020
	.word	18,	0x0b0b0000
	.word	19,	0x00000000
	.word	20,	0x00011801
	.word	21,	0x01181858
	.word	22,	0x00051858
	.word	23,	0x00000500
	.word	24,	0x00140005
	.word	25,	0x00000014
	.word	26,	0x00000000
	.word	27,	0x02000000
	.word	28,	0x02000120
	.word	29,	0x00000120
	.word	30,	0x08000001
	.word	31,	0x00080808
	.word	32,	0x00000000
	.word	35,	0x00000000
	.word	36,	0x01000000
	.word	37,	0x10000000
	.word	38,	0x00100400
	.word	39,	0x00000400
	.word	40,	0x00000100
	.word	41,	0x00000000
	.word	42,	0x00000001
	.word	43,	0x00000000
	.word	44,	0x000c7000
	.word	45,	0x00180046
	.word	46,	0x00460c70
	.word	47,	0x00000018
	.word	48,	0x0c700000
	.word	49,	0x00180046
	.word	50,	0x00460c70
	.word	51,	0x00000018
	.word	52,	0x0c700000
	.word	53,	0x00180046
	.word	54,	0x00460c70
	.word	55,	0x00000018
	.word	56,	0x0c700000
	.word	57,	0x00180046
	.word	58,	0x00460c70
	.word	59,	0x00000018
	.word	60,	0x00000000
	.word	61,	0x00010100
	.word	62,	0x00000000
	.word	63,	0x00000000
	.word	64,	0x00000000
	.word	65,	0x00000000
	.word	66,	0x00000000
	.word	67,	0x00000000
	.word	68,	0x00000000
	.word	69,	0x00000000
	.word	70,	0x00000000
	.word	71,	0x00000000
	.word	72,	0x00000000
	.word	73,	0x00000000
	.word	74,	0x00000000
	.word	75,	0x00000000
	.word	76,	0x00000000
	.word	77,	0x00000000
	.word	78,	0x01000200
	.word	79,	0x02000040
	.word	80,	0x00400100
	.word	81,	0x00000200
	.word	82,	0x01000001
	.word	83,	0x01ffff0a
	.word	84,	0x01010101
	.word	85,	0x03010101
	.word	86,	0x01000003
	.word	87,	0x0000010c
	.word	88,	0x00010000
	.word	89,	0x00000000
	.word	90,	0x00000000
	.word	91,	0x00000000
	.word	92,	0x00000000
	.word	93,	0x00000000
	.word	94,	0x00000000
	.word	95,	0x00000000
	.word	96,	0x00000000
	.word	97,	0x00000000
	.word	98,	0x00000000
	.word	99,	0x00000000
	.word	100,	0x00000000
	.word	101,	0x00000000
	.word	102,	0x00000000
	.word	103,	0x00000000
	.word	104,	0x00000000
	.word	105,	0x00000000
	.word	106,	0x00000000
	.word	107,	0x00000000
	.word	108,	0x02040108
	.word	109,	0x08010402
	.word	110,	0x02020202
	.word	111,	0x01000201
	.word	112,	0x00000200
	.word	113,	0x00000000
	.word	114,	0x00000000
	.word	115,	0x00000000
	.word	116,	0x19000000
	.word	117,	0x00000028
	.word	118,	0x00000000
	.word	119,	0x00010001
	.word	120,	0x00010001
	.word	121,	0x00010001
	.word	122,	0x00010001
	.word	123,	0x00010001
	.word	124,	0x00000000
	.word	125,	0x00000000
	.word	126,	0x00000000
	.word	127,	0x00000000
	.word	128,	0x00232300
	.word	129,	0x23230001
	.word	130,	0x00000001
	.word	131,	0x00000000
	.word	132,	0x00000000
	.word	133,	0x00012323
	.word	134,	0x00012323
	.word	135,	0x00000000
	.word	136,	0x00000000
	.word	137,	0x00232300
	.word	138,	0x23230001
	.word	139,	0x00000001
	.word	140,	0x00000000
	.word	141,	0x00000000
	.word	142,	0x00012323
	.word	143,	0x00012323
	.word	144,	0x00000000
	.word	145,	0x00000000
	.word	146,	0x00232300
	.word	147,	0x23230001
	.word	148,	0xffff0001
	.word	149,	0x00ffff00
	.word	150,	0x0000ffff
	.word	151,	0x00000000
	.word	152,	0x03030303
	.word	153,	0x03030303
	.word	156,	0x02006400
	.word	157,	0x02020202
	.word	158,	0x02020202
	.word	160,	0x01020202
	.word	161,	0x01010064
	.word	162,	0x01010101
	.word	163,	0x01010101
	.word	165,	0x00020101
	.word	166,	0x00000064
	.word	167,	0x00000000
	.word	168,	0x000b0b00
	.word	169,	0x18580000
	.word	170,	0x02000200
	.word	171,	0x02000200
	.word	172,	0x00001858
	.word	173,	0x000079b8
	.word	174,	0x1858080a
	.word	175,	0x02000200
	.word	176,	0x02000200
	.word	177,	0x00001858
	.word	178,	0x000079b8
	.word	179,	0x0202080a
	.word	180,	0x80000100
	.word	181,	0x04070303
	.word	182,	0x0000000a
	.word	183,	0x00000000
	.word	184,	0x00000000
	.word	185,	0x0010ffff
	.word	186,	0x1c070303
	.word	187,	0x0000000f
	.word	188,	0x00000000
	.word	189,	0x00000000
	.word	190,	0x00000000
	.word	191,	0x00000000
	.word	192,	0x00000000
	.word	193,	0x00000000
	.word	194,	0x00000204
	.word	195,	0x00000000
	.word	196,	0x00000000
	.word	197,	0x00000000
	.word	198,	0x00000000
	.word	199,	0x00000000
	.word	200,	0x00000000
	.word	201,	0x00000000
	.word	202,	0x00000008
	.word	203,	0x00000008
	.word	204,	0x00000000
	.word	205,	0x00000040
	.word	206,	0x00070701
	.word	207,	0x00000000
	.word	DDR_TABLE_END

/* OTP sizes in bytes */
otp_sizes:
	.word	0
	.word	256	/* 2048 bits: 32X64 */
	.word	512	/* 4096 bits: 2*32X64 */
	.word	1024	/* 8192 bits: 4*32X64 */
	.word	512	/* 4096 bits: 64X64 */
	.word	768	/* 6144 bits: 5 32X64 */
	.word	0	/* 512 bits: dont care */
	.word	128	/* 1024 bits: 8X64 */

END(ddr_init_ca9)

FUNC(ddr_init_ca7)
	mov	r9,r0			@ shift register base to 0x18007400
	add	r9,r9,#0x400		@ ssince 53573 DDR controller address shift 1K
	mov	ip,lr			@ save lr across calls

chk_bootdev_53573:			@ to determine embedded nvram address
	ldr	r1,=0x18010c40		@ get ddr type from bit[4] of
	ldr	r2,=0x7			@ gci chipstatus reg 7
	str	r2,[r1]
	ldr	r1,=0x18010e04
	ldr	r7,[r1]			@ save bootdev in r7

	ldr	r0,=SI_BCM53573_NANDFLASH	@ r0 = 0x30000000
	ldr	r3,=SI_BCM53573_BOOTDEV_MASK
	and	r4,r3,r7
	ldr	r3,=SI_BCM53573_BOOTDEV_NOR
	cmp	r4,r3
	bne	embedded_nv_53573

	ldr	r0,=SI_BCM53573_NORFLASH
	ldr	r1,=MAX_NVRAM_SPACE
	sub	r0,r0,r1
	ldr	r1,=FLASH_MIN
	ldr	r2,=SI_BCM53573_NORFLASH_WINDOW
	ldr	r3,=NVRAM_MAGIC

1:	add	r4,r0,r1
	ldr	r5,[r4]
	cmp	r5,r3
	beq	read_config_53573

	lsl	r1,r1,#1
	cmp	r1,r2
	ble	1b

	ldr	r0,=SI_BCM53573_NORFLASH	@ r0 = 0x1c000000

	/* Try embedded NVRAM at 4KB and 1KB as last resorts */
embedded_nv_53573:
	ldr	r3,=NVRAM_MAGIC
	add	r4,r0,#0x1000
	ldr	r5,[r4]
	cmp	r5,r3
	beq	read_config_53573

	add	r4,r0,#0x400
	ldr	r5,[r4]
	cmp	r5,r3
	beq	read_config_53573

no_sdram_params_53573:
	/* No nvram, pick up some defaults */
	ldr	r6,=0x1c6		@ setup sdram_config to 0x1c6
	ldr	r5,=0x0

	b	get_ddr_clock_53573
read_config_53573:
	ldr	r6,[r4,#12]		@ Pick up sdram_config & sdram_refresh
	ldr	r5,[r4,#16]		@ Pick up sdram_ncdl

get_ddr_clock_53573:
	ldr	r0,=DDRC00_START
	ldr	r1,[r9,#DDRC_CONTROL00]
	and	r1,r1,r0
	cmp	r1,r0
	bne	set_ddr_clock_default_53573	@ The first time

	ldr	r1,=0x1000
	ldr	r0,=BISZ_OFFSET
	add	r1,r1,r0
	sub	r1,r1,#4
	ldr	r0,=0x0
	ldr	r2,[r1]			@ retrieve ddr clk from memory

	cmp	r2,r0
	beq	set_ddr_clock_default_53573	/* No ddrclk specified */
2:
	ldr	r3,=DDR_TABLE_END
	adrl	r4,ddr_clk_tab_53573
1:	ldr	r1,[r4]			 @r4 now hold ddr clock sequence entry
	cmp	r1,r3
	beq	set_ddr_clock_default_53573
	cmp	r1,r2
	beq	chg_ddr_pll_53573
	add	r4,r4,#16		@ table entry 16 bytes
	b	1b
set_ddr_clock_default_53573:
	ldr	r2,=392			@ set r2 to 392MHz for ddr2

	ldr	r3,=533
	and	r0,r7,#SI_BCM53573_DDRTYPE_MASK		@ gci chipstatus 7 already in r7
	ldr	r1,=SI_BCM53573_DDRTYPE_DDR3
	cmp	r0,r1
	moveq	r2,r3			@ set r2 to 533MHz for ddr3
	b	2b			@ to find the clock entry

chg_ddr_pll_53573:
	mov	r10,r2			@ save DDR clock in r10
	add	r4,r4,#4		@ shift r4 to next entry

Adjust_Fvco:
	ldr	r0,=0x18012660		@ switch to PMU PLL control 27 for setting nfrac
	ldr	r1,=27
	str	r1,[r0]
	ldr	r0,=0x18012664
	ldr	r1,[r4]
	str	r1,[r0]
	add	r4,r4,#4

	ldr	r0,=0x18012660		@ switch to PMU PLL control 26 for setting ndiv
	ldr	r1,=26
	str	r1,[r0]
	ldr	r0,=0x18012664
	ldr	r1,[r0]
	ldr	r2,=0x000fffff
	and	r1,r1,r2
	ldr	r2,[r4]
	orr	r1,r1,r2
	str	r1,[r0]
	add	r4,r4,#4

	ldr	r0,=0x18012600		@update PLL with Fvco settings
	ldr	r1,[r0]
	orr	r1,r1,#0x400
	str	r1,[r0]

	ldr	r0,=0x1800002c		@ wait until DDR PLL being locked
	ldr	r1,=0x4
1:	ldr	r2,[r0]
	and	r2,r2,r1
	cmp	r2,r1
	bne	1b

adjust_post_divisor:
	ldr	r0,=0x18012660		@ switch to PMU PLL control 25 for setting post-divisor
	ldr	r1,=25
	str	r1,[r0]
	ldr	r0,=0x18012664
	ldr	r1,[r0]
	ldr	r2,=0x0000ffff
	and	r1,r1,r2
	ldr	r2,[r4]
	orr	r1,r1,r2
	str	r1,[r0]

	ldr	r0,=0x18012664		@ load post-divisor setting
	ldr	r1,[r0]
	orr	r1,r1,#0x300
	str	r1,[r0]

	ldr	r0,=0x18012600		@ update PLL with Fvco settings
	ldr	r1,[r0]
	orr	r1,r1,#0x400
	str	r1,[r0]

	ldr	r0,=0x0			@ wait 7.5us for clock switch
	ldr	r1,=0x2000
1:	add	r0,r0,#1
	cmp	r0,r1
	ble	1b

	ldr	r0,=0x18012664		@ lock post-divisor setting
	ldr	r1,[r0]
	bic	r1,r1,#0x300
	str	r1,[r0]

	ldr	r0,=0x18012600		@ update PLL with post-divisor settings
	ldr	r1,[r0]
	orr	r1,r1,#0x400
	str	r1,[r0]

	ldr	r0,=0x1800002c		@ wait until DDR PLL being locked
	ldr	r1,=0x4
1:	ldr	r2,[r0]
	and	r2,r2,r1
	cmp	r2,r1
	bne	1b

reset_ddr_core_53573:
	ldr	r0,=0x18107800		@ reset DDR controller and PHY for new frequency
	ldr	r1,=0x1
	str	r1,[r0]

	ldr	r0,=0			@ wait 10 cycles for reset complete
	ldr	r1,=100
1:	add	r0,r0,#1
	cmp	r0,r1
	ble	1b

	ldr	r0,=0x18107800		@ out of reset DDR controller and PHY
	ldr	r1,=0
	str	r1,[r0]

	ldr	r0,=0x4			@ test mem_rst_valid in 0x18007004[2] equal
	ldr	r2,=0x18007004		@ to 0, for reset completed
1:
	ldr	r1,[r2]
	and	r1,r1,r0
	cmp	r1,#0x0
	bne	1b

set_pad_on:				@ enable DDR read pad
	ldr	r0,=0x18007048
	ldr	r1,=0x0
	str	r1,[r0]
	ldr	r0,=0x1800704c		@ enable pad, ODT resistor matching settings
	ldr	r1,=0x10101010
	str	r1,[r0]
	ldr	r0,=0x18007048		@ ODT resistor matching settings
	ldr	r1,=0x00000800
	str	r1,[r0]
	ldr	r0,=0x18007040		@ Turn on PVT controller
	ldr	r1,=0x80002001
	str	r1,[r0]

set_ddr_reg_53573:
	and	r8, r6,#0xf		@ cas latency saved in r8

	adrl	r0,ddr_init_tab_53573	@ call generic DDR ctrlr init
	bl	ddr_init_regs

	and	r0,r7,#SI_BCM53573_DDRTYPE_MASK		@ gci chipstatus 7 already in r7
	ldr	r1,=SI_BCM53573_DDRTYPE_DDR3
	cmp	r0,r1
	beq	set_ddr3_reg_53573
set_ddr2_reg_53573:
	adrl	r0,ddr2_init_tab_53573
	bl	ddr_init_regs

	ldr	r0,=392
	cmp	r0,r10			@ DDR clock already saved in r10
	beq	set_ddr2_392_reg_53573
	ldr	r0,=358
	cmp	r0,r10			@ DDR clock already saved in r10
	beq	set_ddr2_358_reg_53573
	ldr	r0,=318
	cmp	r0,r10			@ DDR clock already saved in r10
	beq	set_ddr2_318_reg_53573

set_ddr2_286_reg_53573:
	adrl	r0,ddr2_286_init_tab_53573
	bl	ddr_init_regs

	adrl	r0,ddr2_286_cl5_init_tab_53573
	adrl	r1,ddr2_286_cl6_init_tab_53573
	cmp	r8,#0x6
	moveq	r0,r1			@ r0 will point to either cl5/cl6 table eventually
	bl	ddr_init_regs

	b set_half_bus_width_53573
set_ddr2_318_reg_53573:
	adrl	r0,ddr2_318_init_tab_53573
	bl	ddr_init_regs

	adrl	r0,ddr2_318_cl5_init_tab_53573
	adrl	r1,ddr2_318_cl6_init_tab_53573
	cmp	r8,#0x6
	moveq	r0,r1			@ r0 will point to either cl5/cl6 table eventually
	bl	ddr_init_regs

	b set_half_bus_width_53573
set_ddr2_358_reg_53573:
	adrl	r0,ddr2_358_init_tab_53573	@ ddr2 358 support cl6. merge table together
	bl	ddr_init_regs

	b set_half_bus_width_53573
set_ddr2_392_reg_53573:
	adrl	r0,ddr2_392_init_tab_53573	@ ddr2 392 support cl6. merge table together
	bl	ddr_init_regs

	b set_half_bus_width_53573
set_ddr3_reg_53573:
	adrl	r0,ddr3_init_tab_53573
	bl	ddr_init_regs

	ldr	r0,=533
	cmp	r0,r10			@ DDR clock already saved in r10
	beq	set_ddr3_533_reg_53573
	ldr	r0,=392
	cmp	r0,r10			@ DDR clock already saved in r10
	beq	set_ddr3_392_reg_53573
	ldr	r0,=358
	cmp	r0,r10			@ DDR clock already saved in r10
	beq	set_ddr3_358_reg_53573

set_ddr3_318_reg_53573:
	adrl	r0,ddr3_318_init_tab_53573
	bl	ddr_init_regs

	adrl	r0,ddr3_318_cl5_init_tab_53573
	adrl	r1,ddr3_318_cl6_init_tab_53573
	cmp	r8,#0x6
	moveq	r0,r1			@ r0 will point to either cl5/cl6 table eventually
	bl	ddr_init_regs

	b set_half_bus_width_53573
set_ddr3_358_reg_53573:
	adrl	r0,ddr3_358_init_tab_53573	@ ddr3 358 only support cl6.  merge table together
	bl	ddr_init_regs

	b set_half_bus_width_53573
set_ddr3_392_reg_53573:
	adrl	r0,ddr3_392_init_tab_53573	@ ddr3 392 only support cl6.  merge table together
	bl	ddr_init_regs

	b set_half_bus_width_53573
set_ddr3_533_reg_53573:
	adrl	r0,ddr3_533_init_tab_53573
	bl	ddr_init_regs

	adrl	r0,ddr3_533_cl7_init_tab_53573
	adrl	r1,ddr3_533_cl8_init_tab_53573
	cmp	r8,#0x8
	moveq	r0,r1			@ r0 will point to either cl7/cl8 table eventually
	bl	ddr_init_regs

set_half_bus_width_53573:
	ldr	r1,[r9,#DDRC_CONTROL65]
	bic	r1,r1,#0x1			@ clear bit [0] by default(32bit)
	ldr	r2,=0x80
	and	r3,r6,r2
	cmp	r3,r2
	orreq	r1,r1,#0x1			@ set bit [0] for half width(16bit)
	str	r1,[r9,#DDRC_CONTROL65]
set_bank_53573:
	ldr	r1,[r9,#DDRC_CONTROL60]
	bic	r1,r1,#0x300			@ clear bit [9:8] for 8 banks
	ldr	r2,=0x40
	and	r3,r6,r2
	cmp	r3,r2
	orrne	r1,r1,#0x100			@ set bit [8] for 4 banks
	str	r1,[r9,#DDRC_CONTROL60]

set_col_53573:
	ldr	r1,[r9,#DDRC_CONTROL60]
	bic	r1,r1,#0x0f000000
	ldr	r2,=0x700
	and	r3,r6,r2
	lsl	r0,r3,#16
	orr	r1,r1,r0
	str	r1,[r9,#DDRC_CONTROL60]

set_tref_53573:
	ldr	r2,=0x10000000
	and	r2,r6,r2
	cmp	r2,#0
	beq	set_trfc_53573
	ldr	r1,[r9,#DDRC_CONTROL26]		@ half tref cycle
	ldr	r3,=0x3fff
	and	r2,r1,r3
	bic	r1,r1,r3
	lsr	r2,r2,#1
	sub	r3,r2,#3
	add	r1,r1,r3
	str	r1,[r9,#DDRC_CONTROL26]
set_trfc_53573:
	ldr	r2,=0x3ff0000
	and	r2,r6,r2
	cmp	r2,#0
	beq	start_ddr_53573
	ldr	r1,[r9,#DDRC_CONTROL25]
	ldr	r3,=0x3ff0000
	bic	r1,r1,r3
	add	r1,r1,r2
	str	r1,[r9,#DDRC_CONTROL25]		@ add counts to trfc

start_ddr_53573:
	ldr	r1,[r9,#DDRC_CONTROL00]
	orr	r1,r1,#DDRC00_START
	str	r1,[r9,#DDRC_CONTROL00]

wait_for_ddr_ready_53573:
	/* check 0x18007004 bit 7(mc_rdy_for_cmd) for AXI ready to start transaction */
	ldr	r0,=0x80
	ldr	r2,=0x18007004
1:
	ldr	r1,[r2]
	and	r1,r1,r0
	cmp	r1,r0
	bne	1b

turn_off_ODT_on_16bit_width:
	ldr	r2,=0x80
	and	r3,r6,r2
	cmp	r3,r2			@ value 1 means half width
	bne	ddr_init_completed_53573
	ldr	r0,=0x18007a0c		@ turning off ODT
	ldr	r1,=0x000dd000
	str	r1,[r0]

ddr_init_completed_53573:
	mov	pc,ip			/* back to init */
	.ltorg

	.align 6
ddr_clk_tab_53573:	/* pllcontrol 27(nfrac[19:1]), 26(ndiv, nfrac[0]), 25(mdiv) */
	.word	286,	0x00054ccd,	0x01c00000,	0x10080000
	.word	318,	0x000714fe,	0x01700000,	0x0c060000
	.word	358,	0x0006f333,	0x81a00000,	0x0c060000
	.word	392,	0x00040000,	0x01800000,	0x0a050000
	.word	533,	0x00055555,	0x01a00000,	0x08040000
	.word	DDR_TABLE_END

	.align 6
ddr_init_tab_53573:
	.word	1,	0x00000000
	.word	2,	0x00000000
	.word	3,	0x00000000
	.word	4,	0x00000000
	.word	5,	0x00000000
	.word	22,	0x01000202
	.word	34,	0x00010000
	.word	37,	0x00000000
	.word	38,	0x00000000
	.word	40,	0x00200400
	.word	41,	0x01000400
	.word	42,	0x00000180
	.word	43,	0x00000000
	.word	44,	0x00000001
	.word	45,	0x00000000
	.word	46,	0x00000000
	.word	51,	0x00000000
	.word	52,	0x00010100
	.word	53,	0x00000000
	.word	54,	0x00000000
	.word	55,	0x00020000
	.word	56,	0x00400100
	.word	57,	0x01000200
	.word	58,	0x02000040
	.word	59,	0x00000040
	.word	60,	0x01000000
	.word	61,	0x01ffff0a
	.word	62,	0x01010101
	.word	63,	0x03010101
	.word	64,	0x0c000001
	.word	65,	0x00000000
	.word	66,	0x00010000
	.word	67,	0x00000000
	.word	68,	0x00000000
	.word	69,	0x00000000
	.word	70,	0x00000000
	.word	71,	0x00000000
	.word	72,	0x00000000
	.word	73,	0x00000000
	.word	74,	0x00000000
	.word	75,	0x00000000
	.word	76,	0x00000000
	.word	77,	0x00000000
	.word	81,	0x00000000
	.word	82,	0x00000000
	.word	83,	0x00000000
	.word	84,	0x0d000000
	.word	85,	0x00000028
	.word	86,	0x00000000
	.word	87,	0x00000000
	.word	88,	0x00000000
	.word	89,	0x00000000
	.word	90,	0x00000000
	.word	91,	0x00000000
	.word	92,	0x00000000
	.word	93,	0x00000000
	.word	94,	0x00000000
	.word	95,	0x00000000
	.word	96,	0x00000000
	.word	97,	0x00000000
	.word	98,	0x00000000
	.word	99,	0x00000000
	.word	100,	0x00000202
	.word	101,	0x00000000
	.word	102,	0x00000000
	.word	103,	0x00000000
	.word	104,	0x00000000
	.word	105,	0x00000000
	.word	106,	0xff03ff00
	.word	108,	0x00000000
	.word	110,	0x02000200
	.word	114,	0x02000200
	.word	115,	0x02000200
	.word	119,	0x000a0301
	.word	120,	0x0000000d
	.word	121,	0x00000000
	.word	122,	0x00000000
	.word	123,	0x00000000
	.word	124,	0x00000a00
	.word	125,	0x00000000
	.word	126,	0x000d0000
	.word	127,	0x00000000
	.word	128,	0x00000000
	.word	129,	0x00000000
	.word	130,	0x00000000
	.word	131,	0x00000000
	.word	132,	0x00000000
	.word	133,	0x00000001
	.word	135,	0x00000101
	.word	136,	0x00000000
	.word	137,	0x00000000
	.word	138,	0x00000000
	.word	139,	0x00000000
	.word	140,	0x00000000
	.word	141,	0x00000000
	.word	142,	0x00000000
	.word	143,	0x00000000
	.word	144,	0x00000000
	.word	145,	0x00000000
	.word	146,	0x00000000
	.word	147,	0x00000000
	.word	148,	0x00000000
	.word	149,	0x00000000
	.word	150,	0x00000000
	.word	151,	0x00000000
	.word	153,	0x00010000
	.word	154,	0x00000100
	.word	155,	0x00000101
	.word	156,	0x00010000
	.word	157,	0x00000100
	.word	158,	0x00000000
	.word	159,	0x0003ffff
	.word	160,	0x00000000
	.word	161,	0x0003ffff
	.word	162,	0x00000000
	.word	163,	0x0003ffff
	.word	164,	0x00000000
	.word	165,	0x0003ffff
	.word	166,	0x00000000
	.word	167,	0x0003ffff
	.word	168,	0x00000000
	.word	169,	0x0003ffff
	.word	170,	0x00000000
	.word	171,	0x0003ffff
	.word	172,	0x00000000
	.word	173,	0x0003ffff
	.word	174,	0x00000000
	.word	175,	0x0003ffff
	.word	176,	0x00000000
	.word	177,	0x0003ffff
	.word	178,	0x00000000
	.word	179,	0x0003ffff
	.word	180,	0x00000000
	.word	181,	0x0003ffff
	.word	182,	0x00000000
	.word	183,	0x0003ffff
	.word	184,	0x00000000
	.word	185,	0x0003ffff
	.word	186,	0x00000000
	.word	187,	0x0003ffff
	.word	188,	0x00000000
	.word	189,	0x0303ffff
	.word	190,	0xffffffff
	.word	191,	0x000f000f
	.word	192,	0x00ffff03
	.word	193,	0x000fffff
	.word	194,	0x0003000f
	.word	195,	0xffffffff
	.word	196,	0x000f000f
	.word	197,	0x00ffff03
	.word	198,	0x000fffff
	.word	199,	0x0003000f
	.word	200,	0xffffffff
	.word	201,	0x000f000f
	.word	202,	0x00ffff03
	.word	203,	0x000fffff
	.word	204,	0x0003000f
	.word	205,	0xffffffff
	.word	206,	0x000f000f
	.word	207,	0x00ffff03
	.word	208,	0x000fffff
	.word	209,	0x0003000f
	.word	210,	0xffffffff
	.word	211,	0x000f000f
	.word	212,	0x00ffff03
	.word	213,	0x000fffff
	.word	214,	0x0003000f
	.word	215,	0xffffffff
	.word	216,	0x000f000f
	.word	217,	0x00ffff03
	.word	218,	0x000fffff
	.word	219,	0x0003000f
	.word	220,	0xffffffff
	.word	221,	0x000f000f
	.word	222,	0x00ffff03
	.word	223,	0x000fffff
	.word	224,	0x0003000f
	.word	225,	0xffffffff
	.word	226,	0x000f000f
	.word	227,	0x00ffff03
	.word	228,	0x000fffff
	.word	256,	0x00000413
	.word	257,	0x00000415
	.word	258,	0x80010088
	.word	261,	0x00004040
	.word	262,	0x00000413
	.word	263,	0x00000415
	.word	264,	0x80010060
	.word	265,	0x00000003
	.word	266,	0x0912007f
	.word	267,	0x00004040
	.word	268,	0x00000000
	.word	269,	0x00000000
	.word	270,	0x00000000
	.word	271,	0x00000000
	.word	272,	0x00000000
	.word	273,	0x0000818a
	.word	274,	0x0000818a
	.word	275,	0x00000000
	.word	276,	0x00000000
	.word	277,	0x4240f000
	.word	278,	0x4240f000
	.word	279,	0x00000000
	.word	280,	0x00000000
	.word	281,	0x00000000
	.word	282,	0x00000000
	.word	283,	0x00000000
	.word	284,	0x00000000
	.word	285,	0x00000000
	.word	286,	0x00000000
	.word	287,	0x00000000
	.word	288,	0x00000413
	.word	289,	0x00000415
	.word	290,	0x80010088
	.word	293,	0x00004040
	.word	294,	0x00000413
	.word	295,	0x00000415
	.word	296,	0x80010060
	.word	297,	0x00000003
	.word	298,	0x0912007f
	.word	299,	0x00004040
	.word	300,	0x00000000
	.word	301,	0x00000000
	.word	302,	0x00000000
	.word	303,	0x00000000
	.word	304,	0x00000000
	.word	305,	0x0000818a
	.word	306,	0x0000818a
	.word	307,	0x00000000
	.word	308,	0x00000000
	.word	309,	0x4240f000
	.word	310,	0x4240f000
	.word	311,	0x00000000
	.word	312,	0x00000000
	.word	313,	0x00000000
	.word	314,	0x00000000
	.word	315,	0x00000000
	.word	316,	0x00000000
	.word	317,	0x00000000
	.word	318,	0x00000000
	.word	319,	0x00000000
	.word	320,	0x00000413
	.word	321,	0x00000415
	.word	322,	0x80010088
	.word	325,	0x00004040
	.word	326,	0x00000413
	.word	327,	0x00000415
	.word	328,	0x80010060
	.word	329,	0x00000003
	.word	330,	0x0912007f
	.word	331,	0x00004040
	.word	332,	0x00000000
	.word	333,	0x00000000
	.word	334,	0x00000000
	.word	335,	0x00000000
	.word	336,	0x00000000
	.word	337,	0x0000818a
	.word	338,	0x0000818a
	.word	339,	0x00000000
	.word	340,	0x00000000
	.word	341,	0x4240f000
	.word	342,	0x4240f000
	.word	343,	0x00000000
	.word	344,	0x00000000
	.word	345,	0x00000000
	.word	346,	0x00000000
	.word	347,	0x00000000
	.word	348,	0x00000000
	.word	349,	0x00000000
	.word	350,	0x00000000
	.word	351,	0x00000000
	.word	352,	0x00000413
	.word	353,	0x00000415
	.word	354,	0x80010088
	.word	357,	0x00004040
	.word	358,	0x00000413
	.word	359,	0x00000415
	.word	360,	0x80010060
	.word	361,	0x00000003
	.word	362,	0x0912007f
	.word	363,	0x00004040
	.word	364,	0x00000000
	.word	365,	0x00000000
	.word	366,	0x00000000
	.word	367,	0x00000000
	.word	368,	0x00000000
	.word	369,	0x0000818a
	.word	370,	0x0000818a
	.word	371,	0x00000000
	.word	372,	0x00000000
	.word	373,	0x4240f000
	.word	374,	0x4240f000
	.word	375,	0x00000000
	.word	376,	0x00000000
	.word	377,	0x00000000
	.word	378,	0x00000000
	.word	379,	0x00000000
	.word	380,	0x00000000
	.word	381,	0x00000000
	.word	382,	0x00000000
	.word	383,	0x00000000
	.word	384,	0x00000000
	.word	387,	0x00000000
	.word	DDR_TABLE_END

	.align 6
ddr2_init_tab_53573:
	.word	0,	0x00000400
	.word	7,	0x0000682b
	.word	11,	0x00000036
	.word	16,	0x02020608
	.word	19,	0x02020203
	.word	20,	0x03002475
	.word	27,	0x00000409
	.word	28,	0x00020002
	.word	29,	0x000a000a
	.word	30,	0x00080002
	.word	31,	0x00080002
	.word	33,	0x001300c8
	.word	35,	0x03030300
	.word	36,	0x00000003
	.word	39,	0x20000100
	.word	48,	0x00000040
	.word	50,	0x00000000
	.word	78,	0x03000000
	.word	116,	0x00000812
	.word	117,	0x0000285a
	.word	134,	0x00c800c8
	.word	152,	0x02020101
	.word	386,	0x00000005
	.word	DDR_TABLE_END

	.align 6
ddr2_286_init_tab_53573:
	.word	6,	0x0000dfdb
	.word	8,	0x0000003a
	.word	9,	0x00000090
	.word	10,	0x00007302
	.word	14,	0x0d120302
	.word	15,	0x020d0503
	.word	17,	0x04020306
	.word	18,	0x03004e59
	.word	21,	0x05050003
	.word	23,	0x03040a01
	.word	24,	0x00000306
	.word	25,	0x00250100
	.word	26,	0x001108b6
	.word	32,	0x002800c8
	.word	107,	0x0c0c0000
	.word	109,	0x0200116c
	.word	111,	0x116c0200
	.word	112,	0x0000571c
	.word	259,	0x00000003
	.word	260,	0x08120033
	.word	291,	0x00000003
	.word	292,	0x08120033
	.word	323,	0x00000003
	.word	324,	0x08120033
	.word	355,	0x00000003
	.word	356,	0x08120033
	.word	385,	0x00000005
	.word	DDR_TABLE_END

	.align 6
ddr2_286_cl5_init_tab_53573:
	.word	12,	0x00040a00
	.word	13,	0x0400040a
	.word	47,	0x00085300
	.word	49,	0x00400253
	.word	79,	0x01000503
	.word	80,	0x00000001
	.word	113,	0x08120405
	.word	118,	0x02020405
	.word	229,	0x0304000f
	.word	230,	0x00000304
	.word	DDR_TABLE_END

	.align 6
ddr2_286_cl6_init_tab_53573:
	.word	12,	0x00050c00
	.word	13,	0x0400050c
	.word	47,	0x00086300
	.word	49,	0x00400263
	.word	79,	0x02000503
	.word	80,	0x00000002
	.word	113,	0x08120506
	.word	118,	0x02020506
	.word	229,	0x0405000f
	.word	230,	0x00000405
	.word	DDR_TABLE_END

	.align 6
ddr2_318_init_tab_53573:
	.word	6,	0x0000f805
	.word	8,	0x00000040
	.word	9,	0x0000009f
	.word	10,	0x00007f02
	.word	14,	0x0f140402
	.word	15,	0x020f0503
	.word	17,	0x04020306
	.word	18,	0x030056ce
	.word	21,	0x05050003
	.word	23,	0x03040a01
	.word	24,	0x00000306
	.word	25,	0x00290100
	.word	26,	0x001109a8
	.word	32,	0x002c00c8
	.word	107,	0x0c0d0000
	.word	109,	0x02001350
	.word	111,	0x13500200
	.word	112,	0x00006090
	.word	259,	0x00000004
	.word	260,	0x0a12002d
	.word	291,	0x00000004
	.word	292,	0x0a12002d
	.word	323,	0x00000004
	.word	324,	0x0a12002d
	.word	355,	0x00000004
	.word	356,	0x0a12002d
	.word	385,	0x00000006
	.word	DDR_TABLE_END

	.align 6
ddr2_318_cl5_init_tab_53573:
	.word	12,	0x00040a00
	.word	13,	0x0400040a
	.word	47,	0x00085300
	.word	49,	0x00400253
	.word	79,	0x01000503
	.word	80,	0x00000001
	.word	113,	0x08120405
	.word	118,	0x02020405
	.word	229,	0x0304000f
	.word	230,	0x00000304
	.word	DDR_TABLE_END

	.align 6
ddr2_318_cl6_init_tab_53573:
	.word	12,	0x00050c00
	.word	13,	0x0400050c
	.word	47,	0x00086300
	.word	49,	0x00400263
	.word	79,	0x02000503
	.word	80,	0x00000002
	.word	113,	0x08120506
	.word	118,	0x02020506
	.word	229,	0x0405000f
	.word	230,	0x00000405
	.word	DDR_TABLE_END

	.align 6
ddr2_358_init_tab_53573:
	.word	6,	0x00011705
	.word	8,	0x00000048
	.word	9,	0x000000b3
	.word	10,	0x00008f02
	.word	12,	0x00050c00
	.word	13,	0x0400050c
	.word	14,	0x11160402
	.word	15,	0x02110603
	.word	17,	0x05020306
	.word	18,	0x030061a8
	.word	21,	0x06060003
	.word	23,	0x03040c01
	.word	24,	0x00000307
	.word	25,	0x002e0100
	.word	26,	0x00110ade
	.word	32,	0x003200c8
	.word	47,	0x000a6300
	.word	49,	0x00400263
	.word	79,	0x02000503
	.word	80,	0x00000002
	.word	107,	0x0c0d0000
	.word	109,	0x020015bc
	.word	111,	0x15bc0200
	.word	112,	0x00006cac
	.word	113,	0x08120506
	.word	118,	0x02020506
	.word	229,	0x0405000f
	.word	230,	0x00000405
	.word	259,	0x00000004
	.word	260,	0x0a120027
	.word	291,	0x00000004
	.word	292,	0x0a120027
	.word	323,	0x00000004
	.word	324,	0x0a120027
	.word	355,	0x00000004
	.word	356,	0x0a120027
	.word	385,	0x00000006
	.word	DDR_TABLE_END

	.align 6
ddr2_392_init_tab_53573:
	.word	6,	0x0001312d
	.word	8,	0x0000004f
	.word	9,	0x000000c4
	.word	10,	0x00009d02
	.word	12,	0x00050c00
	.word	13,	0x0400050c
	.word	14,	0x12180402
	.word	15,	0x02120603
	.word	17,	0x05020306
	.word	18,	0x03006acf
	.word	21,	0x06060003
	.word	23,	0x03040c01
	.word	24,	0x00000307
	.word	25,	0x00320100
	.word	26,	0x00110be3
	.word	32,	0x003600c8
	.word	47,	0x000a6300
	.word	49,	0x00400263
	.word	79,	0x02000503
	.word	80,	0x00000002
	.word	107,	0x0c0d0000
	.word	109,	0x020017c6
	.word	111,	0x17c60200
	.word	112,	0x000076de
	.word	113,	0x08120506
	.word	118,	0x02020506
	.word	229,	0x0405000f
	.word	230,	0x00000405
	.word	259,	0x00000004
	.word	260,	0x0a120024
	.word	291,	0x00000004
	.word	292,	0x0a120024
	.word	323,	0x00000004
	.word	324,	0x0a120024
	.word	355,	0x00000004
	.word	356,	0x0a120024
	.word	385,	0x00000006
	.word	DDR_TABLE_END

	.align 6
ddr3_init_tab_53573:
	.word	0,	0x00000600
	.word	7,	0x00000002
	.word	10,	0x00000000
	.word	11,	0x00000000
	.word	13,	0x0400060a
	.word	16,	0x02040507
	.word	17,	0x0c040407
	.word	19,	0x0c040403
	.word	20,	0x03002247
	.word	27,	0x000003c7
	.word	30,	0x00000000
	.word	31,	0x00000000
	.word	33,	0x000f0200
	.word	36,	0x00000005
	.word	39,	0x20020100
	.word	49,	0x00030220
	.word	50,	0x00000008
	.word	79,	0x00000605
	.word	80,	0x00000000
	.word	107,	0x0c0d0000
	.word	116,	0x0000078e
	.word	117,	0x000025c6
	.word	118,	0x02020605
	.word	134,	0x02000200
	.word	152,	0x03020101
	.word	230,	0x00000505
	.word	259,	0x00000004
	.word	291,	0x00000004
	.word	323,	0x00000004
	.word	355,	0x00000004
	.word	385,	0x00001006
	.word	386,	0x00001005
	.word	DDR_TABLE_END

	.align 6
ddr3_318_init_tab_53573:
	.word	6,	0x00000004
	.word	8,	0x0000f805
	.word	9,	0x00026c0b
	.word	14,	0x0c110404
	.word	15,	0x04100504
	.word	18,	0x0300570d
	.word	21,	0x05050003
	.word	23,	0x03070a01
	.word	24,	0x00000205
	.word	25,	0x00230100
	.word	26,	0x000e09a4
	.word	28,	0x00030003
	.word	29,	0x000a000a
	.word	32,	0x00270200
	.word	35,	0x05050500
	.word	48,	0x00000046
	.word	78,	0x04000000
	.word	109,	0x02001348
	.word	111,	0x13480200
	.word	112,	0x00006068
	.word	260,	0x0a12002d
	.word	292,	0x0a12002d
	.word	324,	0x0a12002d
	.word	356,	0x0a12002d
	.word	DDR_TABLE_END

	.align 6
ddr3_318_cl5_init_tab_53573:
	.word	12,	0x00050a00
	.word	47,	0x00021000
	.word	113,	0x078e0505
	.word	229,	0x0404000f
	.word	DDR_TABLE_END

	.align 6
ddr3_318_cl6_init_tab_53573:
	.word	12,	0x00050c00
	.word	47,	0x00022000
	.word	113,	0x078e0506
	.word	229,	0x0405000f
	.word	DDR_TABLE_END

	.align 6
ddr3_358_init_tab_53573:
	.word	6,	0x00000004
	.word	8,	0x00011705
	.word	9,	0x0002b98c
	.word	12,	0x00050c00
	.word	14,	0x0e130404
	.word	15,	0x04120504
	.word	18,	0x030061ef
	.word	21,	0x06050003
	.word	23,	0x03070b01
	.word	24,	0x00000205
	.word	25,	0x00280100
	.word	26,	0x000e0ad9
	.word	28,	0x00030003
	.word	29,	0x000a000a
	.word	32,	0x002b0200
	.word	35,	0x05050500
	.word	47,	0x00042000
	.word	48,	0x00000046
	.word	78,	0x04000000
	.word	109,	0x020015b2
	.word	111,	0x15b20200
	.word	112,	0x00006c7a
	.word	113,	0x078e0506
	.word	229,	0x0405000f
	.word	260,	0x0a120027
	.word	292,	0x0a120027
	.word	324,	0x0a120027
	.word	356,	0x0a120027
	.word	DDR_TABLE_END

	.align 6
ddr3_392_init_tab_53573:
	.word	6,	0x00000004
	.word	8,	0x0001312d
	.word	9,	0x0002faf1
	.word	12,	0x00050c00
	.word	14,	0x0f140404
	.word	15,	0x04140604
	.word	18,	0x03006b1d
	.word	21,	0x06060003
	.word	23,	0x03070c01
	.word	24,	0x00000206
	.word	25,	0x002b0100
	.word	26,	0x000e0bde
	.word	28,	0x00030003
	.word	29,	0x000a000a
	.word	32,	0x002f0200
	.word	35,	0x05050500
	.word	47,	0x00042000
	.word	48,	0x00000046
	.word	78,	0x04000000
	.word	109,	0x020017bc
	.word	111,	0x17bc0200
	.word	112,	0x000076ac
	.word	113,	0x078e0506
	.word	229,	0x0405000f
	.word	260,	0x0a120024
	.word	292,	0x0a120024
	.word	324,	0x0a120024
	.word	356,	0x0a120024
	.word	DDR_TABLE_END

	.align 6
ddr3_533_init_tab_53573:
	.word	6,	0x00000006
	.word	8,	0x00019f8f
	.word	9,	0x00040ee6
	.word	14,	0x141b0604
	.word	15,	0x041b0704
	.word	18,	0x030091dc
	.word	21,	0x08070003
	.word	23,	0x03070f01
	.word	24,	0x00000207
	.word	25,	0x003b0100
	.word	26,	0x000e102c
	.word	28,	0x00030004
	.word	29,	0x000a000d
	.word	32,	0x00400200
	.word	35,	0x05060600
	.word	48,	0x00080046
	.word	78,	0x05000000
	.word	109,	0x02002058
	.word	111,	0x20580200
	.word	112,	0x0000a1b8
	.word	258,	0x800100a8
	.word	260,	0x0a12001b
	.word	290,	0x800100a8
	.word	292,	0x0a12001b
	.word	322,	0x800100a8
	.word	324,	0x0a12001b
	.word	354,	0x800100a8
	.word	356,	0x0a12001b
	.word	DDR_TABLE_END

	.align 6
ddr3_533_cl7_init_tab_53573:
	.word	12,	0x00060e00
	.word	47,	0x00083000
	.word	113,	0x078e0607
	.word	229,	0x0506000f
	.word	DDR_TABLE_END

	.align 6
ddr3_533_cl8_init_tab_53573:
	.word	12,	0x00061000
	.word	47,	0x00084000
	.word	113,	0x078e0608
	.word	229,	0x0507000f
	.word	DDR_TABLE_END

END(ddr_init_ca7)

FUNC(ddr_init_regs)
	mov	r2,r0
	ldr	r3,=DDR_TABLE_END
1:	ldr	r4,[r2]
	cmp	r4,r3
	beq	2f
	ldr	r5,[r2,#4]
	lsl	r4,r4,#2
	add	r1,r9,r4
	str	r5,[r1]
	add	r2,r2,#8
	b	1b
2:
	mov	pc,lr
END(ddr_init_regs)
