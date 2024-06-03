// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015, Bachmann electronic GmbH
 */

#include <common.h>
#include <spl.h>
#include <asm/arch/mx6-ddr.h>

/* Configure MX6Q/DUAL mmdc DDR io registers */
static struct mx6dq_iomux_ddr_regs ot1200_ddr_ioregs = {
	/* SDCLK[0:1], CAS, RAS, Reset: Differential input, 48ohm */
	.dram_sdclk_0   = 0x00000028,
	.dram_sdclk_1   = 0x00000028,
	.dram_cas       = 0x00000028,
	.dram_ras       = 0x00000028,
	.dram_reset     = 0x00000028,
	/* SDCKE[0:1]: 100k pull-up */
	.dram_sdcke0    = 0x00003000,
	.dram_sdcke1    = 0x00003000,
	/* SDBA2: pull-up disabled */
	.dram_sdba2	    = 0x00000000,
	/* SDODT[0:1]: 100k pull-up, 48 ohm */
	.dram_sdodt0    = 0x00000028,
	.dram_sdodt1    = 0x00000028,
	/* SDQS[0:7]: Differential input, 48 ohm */
	.dram_sdqs0     = 0x00000028,
	.dram_sdqs1     = 0x00000028,
	.dram_sdqs2     = 0x00000028,
	.dram_sdqs3     = 0x00000028,
	.dram_sdqs4     = 0x00000028,
	.dram_sdqs5     = 0x00000028,
	.dram_sdqs6     = 0x00000028,
	.dram_sdqs7     = 0x00000028,
	/* DQM[0:7]: Differential input, 48 ohm */
	.dram_dqm0      = 0x00000028,
	.dram_dqm1      = 0x00000028,
	.dram_dqm2      = 0x00000028,
	.dram_dqm3      = 0x00000028,
	.dram_dqm4      = 0x00000028,
	.dram_dqm5      = 0x00000028,
	.dram_dqm6      = 0x00000028,
	.dram_dqm7      = 0x00000028,
};

/* Configure MX6Q/DUAL mmdc GRP io registers */
static struct mx6dq_iomux_grp_regs ot1200_grp_ioregs = {
	/* DDR3 */
	.grp_ddr_type    = 0x000c0000,
	.grp_ddrmode_ctl = 0x00020000,
	/* Disable DDR pullups */
	.grp_ddrpke      = 0x00000000,
	/* ADDR[00:16], SDBA[0:1]: 48 ohm */
	.grp_addds       = 0x00000028,
	/* CS0/CS1/SDBA2/CKE0/CKE1/SDWE: 48 ohm */
	.grp_ctlds       = 0x00000028,
	/* DATA[00:63]: Differential input, 48 ohm */
	.grp_ddrmode     = 0x00020000,
	.grp_b0ds        = 0x00000028,
	.grp_b1ds        = 0x00000028,
	.grp_b2ds        = 0x00000028,
	.grp_b3ds        = 0x00000028,
	.grp_b4ds        = 0x00000028,
	.grp_b5ds        = 0x00000028,
	.grp_b6ds        = 0x00000028,
	.grp_b7ds        = 0x00000028,
};

static struct mx6_ddr_sysinfo ot1200_ddr_sysinfo = {
	/* Width of data bus: 0=16, 1=32, 2=64 */
	.dsize      = 2,
	/* config for full 4GB range so that get_mem_size() works */
	.cs_density = 32, /* 32Gb per CS */
	/* Single chip select */
	.ncs        = 1,
	.cs1_mirror = 0,	/* war 0 */
	.rtt_wr     = 1,	/* DDR3_RTT_60_OHM - RTT_Wr = RZQ/4 */
	.rtt_nom    = 1,	/* DDR3_RTT_60_OHM - RTT_Nom = RZQ/4 */
	.walat      = 1,	/* Write additional latency */
	.ralat      = 5,	/* Read additional latency */
	.mif3_mode  = 3,	/* Command prediction working mode */
	.bi_on      = 1,	/* Bank interleaving enabled */	/* war 1 */
	.sde_to_rst = 0x10,	/* 14 cycles, 200us (JEDEC default) */
	.rst_to_cke = 0x23,	/* 33 cycles, 500us (JEDEC default) */
	.refsel = 1,		/* Refresh cycles at 32KHz */
	.refr = 7,		/* 8 refresh commands per refresh cycle */
};

/* MT41K128M16JT-125 */
static struct mx6_ddr3_cfg micron_2gib_1600 = {
	.mem_speed = 1600,
	.density   = 2,
	.width     = 16,
	.banks     = 8,
	.rowaddr   = 14,
	.coladdr   = 10,
	.pagesz    = 2,
	.trcd      = 1375,
	.trcmin    = 4875,
	.trasmin   = 3500,
	.SRT       = 1,
};

static struct mx6_mmdc_calibration micron_2gib_1600_mmdc_calib = {
	/* write leveling calibration determine */
	.p0_mpwldectrl0 = 0x00260025,
	.p0_mpwldectrl1 = 0x00270021,
	.p1_mpwldectrl0 = 0x00180034,
	.p1_mpwldectrl1 = 0x00180024,
	/* Read DQS Gating calibration */
	.p0_mpdgctrl0   = 0x04380344,
	.p0_mpdgctrl1   = 0x0330032C,
	.p1_mpdgctrl0   = 0x0338033C,
	.p1_mpdgctrl1   = 0x032C0300,
	/* Read Calibration: DQS delay relative to DQ read access */
	.p0_mprddlctl   = 0x3C2E3238,
	.p1_mprddlctl   = 0x3A2E303C,
	/* Write Calibration: DQ/DM delay relative to DQS write access */
	.p0_mpwrdlctl   = 0x36384036,
	.p1_mpwrdlctl   = 0x442E4438,
};

static void ot1200_spl_dram_init(void)
{
	mx6dq_dram_iocfg(64, &ot1200_ddr_ioregs, &ot1200_grp_ioregs);
	mx6_dram_cfg(&ot1200_ddr_sysinfo, &micron_2gib_1600_mmdc_calib,
		     &micron_2gib_1600);
}

/*
 * called from C runtime startup code (arch/arm/lib/crt0.S:_main)
 * - we have a stack and a place to store GD, both in SRAM
 * - no variable global data is available
 */
void board_init_f(ulong dummy)
{
	/* setup AIPS and disable watchdog */
	arch_cpu_init();

	/* iomux and setup of i2c */
	board_early_init_f();

	/* setup GP timer */
	timer_init();

	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();

	/* configure MMDC for SDRAM width/size and per-model calibration */
	ot1200_spl_dram_init();
}
