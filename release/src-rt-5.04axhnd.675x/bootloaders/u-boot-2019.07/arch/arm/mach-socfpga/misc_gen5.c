// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2012-2017 Altera Corporation <www.altera.com>
 */

#include <common.h>
#include <asm/io.h>
#include <errno.h>
#include <fdtdec.h>
#include <linux/libfdt.h>
#include <altera.h>
#include <miiphy.h>
#include <netdev.h>
#include <watchdog.h>
#include <asm/arch/misc.h>
#include <asm/arch/reset_manager.h>
#include <asm/arch/scan_manager.h>
#include <asm/arch/sdram.h>
#include <asm/arch/system_manager.h>
#include <asm/arch/nic301.h>
#include <asm/arch/scu.h>
#include <asm/pl310.h>

#include <dt-bindings/reset/altr,rst-mgr.h>

DECLARE_GLOBAL_DATA_PTR;

static struct pl310_regs *const pl310 =
	(struct pl310_regs *)CONFIG_SYS_PL310_BASE;
static struct socfpga_system_manager *sysmgr_regs =
	(struct socfpga_system_manager *)SOCFPGA_SYSMGR_ADDRESS;
static struct nic301_registers *nic301_regs =
	(struct nic301_registers *)SOCFPGA_L3REGS_ADDRESS;
static struct scu_registers *scu_regs =
	(struct scu_registers *)SOCFPGA_MPUSCU_ADDRESS;

/*
 * FPGA programming support for SoC FPGA Cyclone V
 */
static Altera_desc altera_fpga[] = {
	{
		/* Family */
		Altera_SoCFPGA,
		/* Interface type */
		fast_passive_parallel,
		/* No limitation as additional data will be ignored */
		-1,
		/* No device function table */
		NULL,
		/* Base interface address specified in driver */
		NULL,
		/* No cookie implementation */
		0
	},
};

static const struct {
	const u16	pn;
	const char	*name;
	const char	*var;
} socfpga_fpga_model[] = {
	/* Cyclone V E */
	{ 0x2b15, "Cyclone V, E/A2", "cv_e_a2" },
	{ 0x2b05, "Cyclone V, E/A4", "cv_e_a4" },
	{ 0x2b22, "Cyclone V, E/A5", "cv_e_a5" },
	{ 0x2b13, "Cyclone V, E/A7", "cv_e_a7" },
	{ 0x2b14, "Cyclone V, E/A9", "cv_e_a9" },
	/* Cyclone V GX/GT */
	{ 0x2b01, "Cyclone V, GX/C3", "cv_gx_c3" },
	{ 0x2b12, "Cyclone V, GX/C4", "cv_gx_c4" },
	{ 0x2b02, "Cyclone V, GX/C5 or GT/D5", "cv_gx_c5" },
	{ 0x2b03, "Cyclone V, GX/C7 or GT/D7", "cv_gx_c7" },
	{ 0x2b04, "Cyclone V, GX/C9 or GT/D9", "cv_gx_c9" },
	/* Cyclone V SE/SX/ST */
	{ 0x2d11, "Cyclone V, SE/A2 or SX/C2", "cv_se_a2" },
	{ 0x2d01, "Cyclone V, SE/A4 or SX/C4", "cv_se_a4" },
	{ 0x2d12, "Cyclone V, SE/A5 or SX/C5 or ST/D5", "cv_se_a5" },
	{ 0x2d02, "Cyclone V, SE/A6 or SX/C6 or ST/D6", "cv_se_a6" },
	/* Arria V */
	{ 0x2d03, "Arria V, D5", "av_d5" },
};

static int socfpga_fpga_id(const bool print_id)
{
	const u32 altera_mi = 0x6e;
	const u32 id = scan_mgr_get_fpga_id();

	const u32 lsb = id & 0x00000001;
	const u32 mi = (id >> 1) & 0x000007ff;
	const u32 pn = (id >> 12) & 0x0000ffff;
	const u32 version = (id >> 28) & 0x0000000f;
	int i;

	if ((mi != altera_mi) || (lsb != 1)) {
		printf("FPGA:  Not Altera chip ID\n");
		return -EINVAL;
	}

	for (i = 0; i < ARRAY_SIZE(socfpga_fpga_model); i++)
		if (pn == socfpga_fpga_model[i].pn)
			break;

	if (i == ARRAY_SIZE(socfpga_fpga_model)) {
		printf("FPGA:  Unknown Altera chip, ID 0x%08x\n", id);
		return -EINVAL;
	}

	if (print_id)
		printf("FPGA:  Altera %s, version 0x%01x\n",
		       socfpga_fpga_model[i].name, version);
	return i;
}

/*
 * Print CPU information
 */
#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void)
{
	const u32 bsel =
		SYSMGR_GET_BOOTINFO_BSEL(readl(&sysmgr_regs->bootinfo));

	puts("CPU:   Altera SoCFPGA Platform\n");
	socfpga_fpga_id(1);

	printf("BOOT:  %s\n", bsel_str[bsel].name);
	return 0;
}
#endif

#ifdef CONFIG_ARCH_MISC_INIT
int arch_misc_init(void)
{
	const u32 bsel = readl(&sysmgr_regs->bootinfo) & 0x7;
	const int fpga_id = socfpga_fpga_id(0);
	env_set("bootmode", bsel_str[bsel].mode);
	if (fpga_id >= 0)
		env_set("fpgatype", socfpga_fpga_model[fpga_id].var);
	return 0;
}
#endif

/*
 * Convert all NIC-301 AMBA slaves from secure to non-secure
 */
static void socfpga_nic301_slave_ns(void)
{
	writel(0x1, &nic301_regs->lwhps2fpgaregs);
	writel(0x1, &nic301_regs->hps2fpgaregs);
	writel(0x1, &nic301_regs->acp);
	writel(0x1, &nic301_regs->rom);
	writel(0x1, &nic301_regs->ocram);
	writel(0x1, &nic301_regs->sdrdata);
}

void socfpga_sdram_remap_zero(void)
{
	u32 remap;

	socfpga_nic301_slave_ns();

	/*
	 * Private components security:
	 * U-Boot : configure private timer, global timer and cpu component
	 * access as non secure for kernel stage (as required by Linux)
	 */
	setbits_le32(&scu_regs->sacr, 0xfff);

	/* Configure the L2 controller to make SDRAM start at 0 */
	remap = 0x1; /* remap.mpuzero */
	/* Keep fpga bridge enabled when running from FPGA onchip RAM */
	if (socfpga_is_booting_from_fpga())
		remap |= 0x8; /* remap.hps2fpga */
	writel(remap, &nic301_regs->remap);

	writel(0x1, &pl310->pl310_addr_filter_start);
}

static u32 iswgrp_handoff[8];

int arch_early_init_r(void)
{
	int i;

	/*
	 * Write magic value into magic register to unlock support for
	 * issuing warm reset. The ancient kernel code expects this
	 * value to be written into the register by the bootloader, so
	 * to support that old code, we write it here instead of in the
	 * reset_cpu() function just before resetting the CPU.
	 */
	writel(0xae9efebc, &sysmgr_regs->romcodegrp_warmramgrp_enable);

	for (i = 0; i < 8; i++)	/* Cache initial SW setting regs */
		iswgrp_handoff[i] = readl(&sysmgr_regs->iswgrp_handoff[i]);

	socfpga_bridges_reset(1);

	socfpga_sdram_remap_zero();

	/* Add device descriptor to FPGA device table */
	socfpga_fpga_add(&altera_fpga[0]);

	return 0;
}

#ifndef CONFIG_SPL_BUILD
static struct socfpga_reset_manager *reset_manager_base =
	(struct socfpga_reset_manager *)SOCFPGA_RSTMGR_ADDRESS;
static struct socfpga_sdr_ctrl *sdr_ctrl =
	(struct socfpga_sdr_ctrl *)SDR_CTRLGRP_ADDRESS;

void do_bridge_reset(int enable, unsigned int mask)
{
	int i;

	if (enable) {
		socfpga_bridges_set_handoff_regs(!(mask & BIT(0)),
						 !(mask & BIT(1)),
						 !(mask & BIT(2)));
		for (i = 0; i < 2; i++) {	/* Reload SW setting cache */
			iswgrp_handoff[i] =
				readl(&sysmgr_regs->iswgrp_handoff[i]);
		}

		writel(iswgrp_handoff[2], &sysmgr_regs->fpgaintfgrp_module);
		writel(iswgrp_handoff[3], &sdr_ctrl->fpgaport_rst);
		writel(iswgrp_handoff[0], &reset_manager_base->brg_mod_reset);
		writel(iswgrp_handoff[1], &nic301_regs->remap);
	} else {
		writel(0, &sysmgr_regs->fpgaintfgrp_module);
		writel(0, &sdr_ctrl->fpgaport_rst);
		writel(0, &reset_manager_base->brg_mod_reset);
		writel(1, &nic301_regs->remap);
	}
}
#endif
