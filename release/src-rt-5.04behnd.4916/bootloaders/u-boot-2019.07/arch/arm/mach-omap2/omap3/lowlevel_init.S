/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Board specific setup info
 *
 * (C) Copyright 2008
 * Texas Instruments, <www.ti.com>
 *
 * Initial Code by:
 * Richard Woodruff <r-woodruff2@ti.com>
 * Syed Mohammed Khasim <khasim@ti.com>
 */

#include <config.h>
#include <asm/arch/mem.h>
#include <asm/arch/clocks_omap3.h>
#include <linux/linkage.h>

/*
 * Funtion for making PPA HAL API calls in secure devices
 * Input:
 *	R0 - Service ID
 *	R1 - paramer list
 */
/* TODO: Re-evaluate the comment at the end regarding armv5 vs armv7 */
ENTRY(do_omap3_emu_romcode_call)
	PUSH {r4-r12, lr} @ Save all registers from ROM code!
	MOV r12, r0	@ Copy the Secure Service ID in R12
	MOV r3, r1	@ Copy the pointer to va_list in R3
	MOV r1, #0	@ Process ID - 0
	MOV r2, #OMAP3_EMU_HAL_START_HAL_CRITICAL	@ Copy the pointer
							@ to va_list in R3
	MOV r6, #0xFF	@ Indicate new Task call
	mcr     p15, 0, r0, c7, c10, 4	@ DSB
	mcr     p15, 0, r0, c7, c10, 5	@ DMB
	.word	0xe1600071	@ SMC #1 to call PPA service - hand assembled
				@ because we use -march=armv5
	POP {r4-r12, pc}
ENDPROC(do_omap3_emu_romcode_call)

#if !defined(CONFIG_SYS_NAND_BOOT) && !defined(CONFIG_SYS_NAND_BOOT)
/**************************************************************************
 * cpy_clk_code: relocates clock code into SRAM where its safer to execute
 * R1 = SRAM destination address.
 *************************************************************************/
ENTRY(cpy_clk_code)
	/* Copy DPLL code into SRAM */
	adr	r0, go_to_speed		/* copy from start of go_to_speed... */
	adr	r2, lowlevel_init	/* ... up to start of low_level_init */
next2:
	ldmia	r0!, {r3 - r10}		/* copy from source address [r0] */
	stmia	r1!, {r3 - r10}		/* copy to   target address [r1] */
	cmp	r0, r2			/* until source end address [r2] */
	blo	next2
	mov	pc, lr			/* back to caller */
ENDPROC(cpy_clk_code)

/* ***************************************************************************
 *  go_to_speed: -Moves to bypass, -Commits clock dividers, -puts dpll at speed
 *               -executed from SRAM.
 *  R0 = CM_CLKEN_PLL-bypass value
 *  R1 = CM_CLKSEL1_PLL-m, n, and divider values
 *  R2 = CM_CLKSEL_CORE-divider values
 *  R3 = CM_IDLEST_CKGEN - addr dpll lock wait
 *
 *  Note: If core unlocks/relocks and SDRAM is running fast already it gets
 *        confused.  A reset of the controller gets it back.  Taking away its
 *        L3 when its not in self refresh seems bad for it.  Normally, this
 *	  code runs from flash before SDR is init so that should be ok.
 ****************************************************************************/
ENTRY(go_to_speed)
	stmfd sp!, {r4 - r6}

	/* move into fast relock bypass */
	ldr	r4, pll_ctl_add
	str	r0, [r4]
wait1:
	ldr	r5, [r3]		/* get status */
	and	r5, r5, #0x1		/* isolate core status */
	cmp	r5, #0x1		/* still locked? */
	beq	wait1			/* if lock, loop */

	/* set new dpll dividers _after_ in bypass */
	ldr	r5, pll_div_add1
	str	r1, [r5]		/* set m, n, m2 */
	ldr	r5, pll_div_add2
	str	r2, [r5]		/* set l3/l4/.. dividers*/
	ldr	r5, pll_div_add3	/* wkup */
	ldr	r2, pll_div_val3	/* rsm val */
	str	r2, [r5]
	ldr	r5, pll_div_add4	/* gfx */
	ldr	r2, pll_div_val4
	str	r2, [r5]
	ldr	r5, pll_div_add5	/* emu */
	ldr	r2, pll_div_val5
	str	r2, [r5]

	/* now prepare GPMC (flash) for new dpll speed */
	/* flash needs to be stable when we jump back to it */
	ldr	r5, flash_cfg3_addr
	ldr	r2, flash_cfg3_val
	str	r2, [r5]
	ldr	r5, flash_cfg4_addr
	ldr	r2, flash_cfg4_val
	str	r2, [r5]
	ldr	r5, flash_cfg5_addr
	ldr	r2, flash_cfg5_val
	str	r2, [r5]
	ldr	r5, flash_cfg1_addr
	ldr	r2, [r5]
	orr	r2, r2, #0x3		/* up gpmc divider */
	str	r2, [r5]

	/* lock DPLL3 and wait a bit */
	orr	r0, r0, #0x7	/* set up for lock mode */
	str	r0, [r4]	/* lock */
	nop			/* ARM slow at this point working at sys_clk */
	nop
	nop
	nop
wait2:
	ldr	r5, [r3]	/* get status */
	and	r5, r5, #0x1	/* isolate core status */
	cmp	r5, #0x1	/* still locked? */
	bne	wait2		/* if lock, loop */
	nop
	nop
	nop
	nop
	ldmfd	sp!, {r4 - r6}
	mov	pc, lr		/* back to caller, locked */
ENDPROC(go_to_speed)

_go_to_speed: .word go_to_speed

/* these constants need to be close for PIC code */
/* The Nor has to be in the Flash Base CS0 for this condition to happen */
flash_cfg1_addr:
	.word (GPMC_CONFIG_CS0_BASE + GPMC_CONFIG1)
flash_cfg3_addr:
	.word (GPMC_CONFIG_CS0_BASE + GPMC_CONFIG3)
flash_cfg3_val:
	.word STNOR_GPMC_CONFIG3
flash_cfg4_addr:
	.word (GPMC_CONFIG_CS0_BASE + GPMC_CONFIG4)
flash_cfg4_val:
	.word STNOR_GPMC_CONFIG4
flash_cfg5_val:
	.word STNOR_GPMC_CONFIG5
flash_cfg5_addr:
	.word (GPMC_CONFIG_CS0_BASE + GPMC_CONFIG5)
pll_ctl_add:
	.word CM_CLKEN_PLL
pll_div_add1:
	.word CM_CLKSEL1_PLL
pll_div_add2:
	.word CM_CLKSEL_CORE
pll_div_add3:
	.word CM_CLKSEL_WKUP
pll_div_val3:
	.word (WKUP_RSM << 1)
pll_div_add4:
	.word CM_CLKSEL_GFX
pll_div_val4:
	.word (GFX_DIV << 0)
pll_div_add5:
	.word CM_CLKSEL1_EMU
pll_div_val5:
	.word CLSEL1_EMU_VAL

#endif

ENTRY(lowlevel_init)
	ldr	sp, SRAM_STACK
	str	ip, [sp]	/* stash ip register */
	mov	ip, lr		/* save link reg across call */
#if !defined(CONFIG_SYS_NAND_BOOT) && !defined(CONFIG_SYS_ONENAND_BOOT)
/*
 * No need to copy/exec the clock code - DPLL adjust already done
 * in NAND/oneNAND Boot.
 */
	ldr	r1, =SRAM_CLK_CODE
	bl	cpy_clk_code
#endif /* NAND Boot */
	mov	lr, ip		/* restore link reg */
	ldr	ip, [sp]	/* restore save ip */
	/* tail-call s_init to setup pll, mux, memory */
	b	s_init

ENDPROC(lowlevel_init)

	/* the literal pools origin */
	.ltorg

REG_CONTROL_STATUS:
	.word CONTROL_STATUS
SRAM_STACK:
	.word LOW_LEVEL_SRAM_STACK

/* DPLL(1-4) PARAM TABLES */

/*
 * Each of the tables has M, N, FREQSEL, M2 values defined for nominal
 * OPP (1.2V). The fields are defined according to dpll_param struct (clock.c).
 * The values are defined for all possible sysclk and for ES1 and ES2.
 */

mpu_dpll_param:
/* 12MHz */
/* ES1 */
.word MPU_M_12_ES1, MPU_N_12_ES1, MPU_FSEL_12_ES1, MPU_M2_12_ES1
/* ES2 */
.word MPU_M_12_ES2, MPU_N_12_ES2, MPU_FSEL_12_ES2, MPU_M2_ES2
/* 3410 */
.word MPU_M_12, MPU_N_12, MPU_FSEL_12, MPU_M2_12

/* 13MHz */
/* ES1 */
.word MPU_M_13_ES1, MPU_N_13_ES1, MPU_FSEL_13_ES1, MPU_M2_13_ES1
/* ES2 */
.word MPU_M_13_ES2, MPU_N_13_ES2, MPU_FSEL_13_ES2, MPU_M2_13_ES2
/* 3410 */
.word MPU_M_13, MPU_N_13, MPU_FSEL_13, MPU_M2_13

/* 19.2MHz */
/* ES1 */
.word MPU_M_19P2_ES1, MPU_N_19P2_ES1, MPU_FSEL_19P2_ES1, MPU_M2_19P2_ES1
/* ES2 */
.word MPU_M_19P2_ES2, MPU_N_19P2_ES2, MPU_FSEL_19P2_ES2, MPU_M2_19P2_ES2
/* 3410 */
.word MPU_M_19P2, MPU_N_19P2, MPU_FSEL_19P2, MPU_M2_19P2

/* 26MHz */
/* ES1 */
.word MPU_M_26_ES1, MPU_N_26_ES1, MPU_FSEL_26_ES1, MPU_M2_26_ES1
/* ES2 */
.word MPU_M_26_ES2, MPU_N_26_ES2, MPU_FSEL_26_ES2, MPU_M2_26_ES2
/* 3410 */
.word MPU_M_26, MPU_N_26, MPU_FSEL_26, MPU_M2_26

/* 38.4MHz */
/* ES1 */
.word MPU_M_38P4_ES1, MPU_N_38P4_ES1, MPU_FSEL_38P4_ES1, MPU_M2_38P4_ES1
/* ES2 */
.word MPU_M_38P4_ES2, MPU_N_38P4_ES2, MPU_FSEL_38P4_ES2, MPU_M2_38P4_ES2
/* 3410 */
.word MPU_M_38P4, MPU_N_38P4, MPU_FSEL_38P4, MPU_M2_38P4


.globl get_mpu_dpll_param
get_mpu_dpll_param:
	adr	r0, mpu_dpll_param
	mov	pc, lr

iva_dpll_param:
/* 12MHz */
/* ES1 */
.word IVA_M_12_ES1, IVA_N_12_ES1, IVA_FSEL_12_ES1, IVA_M2_12_ES1
/* ES2 */
.word IVA_M_12_ES2, IVA_N_12_ES2, IVA_FSEL_12_ES2, IVA_M2_12_ES2
/* 3410 */
.word IVA_M_12, IVA_N_12, IVA_FSEL_12, IVA_M2_12

/* 13MHz */
/* ES1 */
.word IVA_M_13_ES1, IVA_N_13_ES1, IVA_FSEL_13_ES1, IVA_M2_13_ES1
/* ES2 */
.word IVA_M_13_ES2, IVA_N_13_ES2,  IVA_FSEL_13_ES2, IVA_M2_13_ES2
/* 3410 */
.word IVA_M_13, IVA_N_13, IVA_FSEL_13, IVA_M2_13

/* 19.2MHz */
/* ES1 */
.word IVA_M_19P2_ES1, IVA_N_19P2_ES1, IVA_FSEL_19P2_ES1, IVA_M2_19P2_ES1
/* ES2 */
.word IVA_M_19P2_ES2, IVA_N_19P2_ES2, IVA_FSEL_19P2_ES2, IVA_M2_19P2_ES2
/* 3410 */
.word IVA_M_19P2, IVA_N_19P2, IVA_FSEL_19P2, IVA_M2_19P2

/* 26MHz */
/* ES1 */
.word IVA_M_26_ES1, IVA_N_26_ES1, IVA_FSEL_26_ES1, IVA_M2_26_ES1
/* ES2 */
.word IVA_M_26_ES2, IVA_N_26_ES2, IVA_FSEL_26_ES2, IVA_M2_26_ES2
/* 3410 */
.word IVA_M_26, IVA_N_26, IVA_FSEL_26, IVA_M2_26

/* 38.4MHz */
/* ES1 */
.word IVA_M_38P4_ES1, IVA_N_38P4_ES1, IVA_FSEL_38P4_ES1, IVA_M2_38P4_ES1
/* ES2 */
.word IVA_M_38P4_ES2, IVA_N_38P4_ES2, IVA_FSEL_38P4_ES2, IVA_M2_38P4_ES2
/* 3410 */
.word IVA_M_38P4, IVA_N_38P4, IVA_FSEL_38P4, IVA_M2_38P4


.globl get_iva_dpll_param
get_iva_dpll_param:
	adr	r0, iva_dpll_param
	mov	pc, lr

/* Core DPLL targets for L3 at 166 & L133 */
core_dpll_param:
/* 12MHz */
/* ES1 */
.word CORE_M_12_ES1, CORE_N_12_ES1, CORE_FSL_12_ES1, CORE_M2_12_ES1
/* ES2 */
.word CORE_M_12, CORE_N_12, CORE_FSEL_12, CORE_M2_12
/* 3410 */
.word CORE_M_12, CORE_N_12, CORE_FSEL_12, CORE_M2_12

/* 13MHz */
/* ES1 */
.word CORE_M_13_ES1, CORE_N_13_ES1, CORE_FSL_13_ES1, CORE_M2_13_ES1
/* ES2 */
.word CORE_M_13, CORE_N_13, CORE_FSEL_13, CORE_M2_13
/* 3410 */
.word CORE_M_13, CORE_N_13, CORE_FSEL_13, CORE_M2_13

/* 19.2MHz */
/* ES1 */
.word CORE_M_19P2_ES1, CORE_N_19P2_ES1, CORE_FSL_19P2_ES1, CORE_M2_19P2_ES1
/* ES2 */
.word CORE_M_19P2, CORE_N_19P2, CORE_FSEL_19P2, CORE_M2_19P2
/* 3410 */
.word CORE_M_19P2, CORE_N_19P2, CORE_FSEL_19P2, CORE_M2_19P2

/* 26MHz */
/* ES1 */
.word CORE_M_26_ES1, CORE_N_26_ES1, CORE_FSL_26_ES1, CORE_M2_26_ES1
/* ES2 */
.word CORE_M_26, CORE_N_26, CORE_FSEL_26, CORE_M2_26
/* 3410 */
.word CORE_M_26, CORE_N_26, CORE_FSEL_26, CORE_M2_26

/* 38.4MHz */
/* ES1 */
.word CORE_M_38P4_ES1, CORE_N_38P4_ES1, CORE_FSL_38P4_ES1, CORE_M2_38P4_ES1
/* ES2 */
.word CORE_M_38P4, CORE_N_38P4, CORE_FSEL_38P4, CORE_M2_38P4
/* 3410 */
.word CORE_M_38P4, CORE_N_38P4, CORE_FSEL_38P4, CORE_M2_38P4

.globl get_core_dpll_param
get_core_dpll_param:
	adr	r0, core_dpll_param
	mov	pc, lr

/* PER DPLL values are same for both ES1 and ES2 */
per_dpll_param:
/* 12MHz */
.word PER_M_12, PER_N_12, PER_FSEL_12, PER_M2_12

/* 13MHz */
.word PER_M_13, PER_N_13, PER_FSEL_13, PER_M2_13

/* 19.2MHz */
.word PER_M_19P2, PER_N_19P2, PER_FSEL_19P2, PER_M2_19P2

/* 26MHz */
.word PER_M_26, PER_N_26, PER_FSEL_26, PER_M2_26

/* 38.4MHz */
.word PER_M_38P4, PER_N_38P4, PER_FSEL_38P4, PER_M2_38P4

.globl get_per_dpll_param
get_per_dpll_param:
	adr	r0, per_dpll_param
	mov	pc, lr

/* PER2 DPLL values */
per2_dpll_param:
/* 12MHz */
.word PER2_M_12, PER2_N_12, PER2_FSEL_12, PER2_M2_12

/* 13MHz */
.word PER2_M_13, PER2_N_13, PER2_FSEL_13, PER2_M2_13

/* 19.2MHz */
.word PER2_M_19P2, PER2_N_19P2, PER2_FSEL_19P2, PER2_M2_19P2

/* 26MHz */
.word PER2_M_26, PER2_N_26, PER2_FSEL_26, PER2_M2_26

/* 38.4MHz */
.word PER2_M_38P4, PER2_N_38P4, PER2_FSEL_38P4, PER2_M2_38P4

.globl get_per2_dpll_param
get_per2_dpll_param:
	adr	r0, per2_dpll_param
	mov	pc, lr

/*
 * Tables for 36XX/37XX devices
 *
 */
mpu_36x_dpll_param:
/* 12MHz */
.word 50, 0, 0, 1
/* 13MHz */
.word 600, 12, 0, 1
/* 19.2MHz */
.word 125, 3, 0, 1
/* 26MHz */
.word 300, 12, 0, 1
/* 38.4MHz */
.word 125, 7, 0, 1

iva_36x_dpll_param:
/* 12MHz */
.word 130, 2, 0, 1
/* 13MHz */
.word 20, 0, 0, 1
/* 19.2MHz */
.word 325, 11, 0, 1
/* 26MHz */
.word 10, 0, 0, 1
/* 38.4MHz */
.word 325, 23, 0, 1

core_36x_dpll_param:
/* 12MHz */
.word 100, 2, 0, 1
/* 13MHz */
.word 400, 12, 0, 1
/* 19.2MHz */
.word 375, 17, 0, 1
/* 26MHz */
.word 200, 12, 0, 1
/* 38.4MHz */
.word 375, 35, 0, 1

per_36x_dpll_param:
/*    SYSCLK    M       N      M2      M3      M4     M5      M6      m2DIV */
.word 12000,    360,    4,     9,      16,     5,     4,      3,      1
.word 13000,    864,   12,     9,      16,     9,     4,      3,      1
.word 19200,    360,    7,     9,      16,     5,     4,      3,      1
.word 26000,    432,   12,     9,      16,     9,     4,      3,      1
.word 38400,    360,   15,     9,      16,     5,     4,      3,      1

per2_36x_dpll_param:
/* 12MHz */
.word PER2_36XX_M_12, PER2_36XX_N_12, 0, PER2_36XX_M2_12
/* 13MHz */
.word PER2_36XX_M_13, PER2_36XX_N_13, 0, PER2_36XX_M2_13
/* 19.2MHz */
.word PER2_36XX_M_19P2, PER2_36XX_N_19P2, 0, PER2_36XX_M2_19P2
/* 26MHz */
.word PER2_36XX_M_26, PER2_36XX_N_26, 0, PER2_36XX_M2_26
/* 38.4MHz */
.word PER2_36XX_M_38P4, PER2_36XX_N_38P4, 0, PER2_36XX_M2_38P4


ENTRY(get_36x_mpu_dpll_param)
	adr	r0, mpu_36x_dpll_param
	mov	pc, lr
ENDPROC(get_36x_mpu_dpll_param)

ENTRY(get_36x_iva_dpll_param)
	adr	r0, iva_36x_dpll_param
	mov	pc, lr
ENDPROC(get_36x_iva_dpll_param)

ENTRY(get_36x_core_dpll_param)
	adr	r0, core_36x_dpll_param
	mov	pc, lr
ENDPROC(get_36x_core_dpll_param)

ENTRY(get_36x_per_dpll_param)
	adr	r0, per_36x_dpll_param
	mov	pc, lr
ENDPROC(get_36x_per_dpll_param)

ENTRY(get_36x_per2_dpll_param)
	adr	r0, per2_36x_dpll_param
	mov	pc, lr
ENDPROC(get_36x_per2_dpll_param)
