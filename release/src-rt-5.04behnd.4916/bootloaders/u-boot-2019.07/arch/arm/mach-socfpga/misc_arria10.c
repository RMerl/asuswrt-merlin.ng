// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2016-2017 Intel Corporation
 */

#include <altera.h>
#include <common.h>
#include <errno.h>
#include <fdtdec.h>
#include <miiphy.h>
#include <netdev.h>
#include <ns16550.h>
#include <watchdog.h>
#include <asm/arch/misc.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/reset_manager.h>
#include <asm/arch/reset_manager_arria10.h>
#include <asm/arch/sdram_arria10.h>
#include <asm/arch/system_manager.h>
#include <asm/arch/nic301.h>
#include <asm/io.h>
#include <asm/pl310.h>

#define PINMUX_UART0_TX_SHARED_IO_OFFSET_Q1_3	0x08
#define PINMUX_UART0_TX_SHARED_IO_OFFSET_Q2_11	0x58
#define PINMUX_UART0_TX_SHARED_IO_OFFSET_Q3_3	0x68
#define PINMUX_UART1_TX_SHARED_IO_OFFSET_Q1_7	0x18
#define PINMUX_UART1_TX_SHARED_IO_OFFSET_Q3_7	0x78
#define PINMUX_UART1_TX_SHARED_IO_OFFSET_Q4_3	0x98

static struct socfpga_system_manager *sysmgr_regs =
	(struct socfpga_system_manager *)SOCFPGA_SYSMGR_ADDRESS;

/*
 * FPGA programming support for SoC FPGA Arria 10
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

#if defined(CONFIG_SPL_BUILD)
static struct pl310_regs *const pl310 =
	(struct pl310_regs *)CONFIG_SYS_PL310_BASE;
static const struct socfpga_noc_fw_ocram *noc_fw_ocram_base =
	(void *)SOCFPGA_SDR_FIREWALL_OCRAM_ADDRESS;

/*
+ * This function initializes security policies to be consistent across
+ * all logic units in the Arria 10.
+ *
+ * The idea is to set all security policies to be normal, nonsecure
+ * for all units.
+ */
void socfpga_init_security_policies(void)
{
	/* Put OCRAM in non-secure */
	writel(0x003f0000, &noc_fw_ocram_base->region0);
	writel(0x1, &noc_fw_ocram_base->enable);

	/* Put DDR in non-secure */
	writel(0xffff0000, SOCFPGA_SDR_FIREWALL_L3_ADDRESS + 0xc);
	writel(0x1, SOCFPGA_SDR_FIREWALL_L3_ADDRESS);

	/* Enable priviledged and non-priviledged access to L4 peripherals */
	writel(~0, SOCFPGA_NOC_L4_PRIV_FLT_OFST);

	/* Enable secure and non-secure transactions to bridges */
	writel(~0, SOCFPGA_NOC_FW_H2F_SCR_OFST);
	writel(~0, SOCFPGA_NOC_FW_H2F_SCR_OFST + 4);

	writel(0x0007FFFF, &sysmgr_regs->ecc_intmask_set);
}

void socfpga_sdram_remap_zero(void)
{
	/* Configure the L2 controller to make SDRAM start at 0 */
	writel(0x1, &pl310->pl310_addr_filter_start);
}
#endif

int arch_early_init_r(void)
{
	/* Add device descriptor to FPGA device table */
	socfpga_fpga_add(&altera_fpga[0]);

	return 0;
}

/*
 * Print CPU information
 */
#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void)
{
	const u32 bsel =
		SYSMGR_GET_BOOTINFO_BSEL(readl(&sysmgr_regs->bootinfo));

	puts("CPU:   Altera SoCFPGA Arria 10\n");

	printf("BOOT:  %s\n", bsel_str[bsel].name);
	return 0;
}
#endif

void do_bridge_reset(int enable, unsigned int mask)
{
	if (enable)
		socfpga_reset_deassert_bridges_handoff();
	else
		socfpga_bridges_reset();
}
