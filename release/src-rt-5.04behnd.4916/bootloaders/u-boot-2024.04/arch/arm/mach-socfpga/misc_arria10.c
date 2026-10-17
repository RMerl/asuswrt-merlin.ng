// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2016-2021 Intel Corporation
 */

#include <altera.h>
#include <common.h>
#include <errno.h>
#include <fdtdec.h>
#include <init.h>
#include <miiphy.h>
#include <netdev.h>
#include <ns16550.h>
#include <spi_flash.h>
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
#include <linux/sizes.h>

#define PINMUX_UART0_TX_SHARED_IO_OFFSET_Q1_3	0x08
#define PINMUX_UART0_TX_SHARED_IO_OFFSET_Q2_11	0x58
#define PINMUX_UART0_TX_SHARED_IO_OFFSET_Q3_3	0x68
#define PINMUX_UART1_TX_SHARED_IO_OFFSET_Q1_7	0x18
#define PINMUX_UART1_TX_SHARED_IO_OFFSET_Q3_7	0x78
#define PINMUX_UART1_TX_SHARED_IO_OFFSET_Q4_3	0x98

#define REGULAR_BOOT_MAGIC	0xd15ea5e
#define PERIPH_RBF_PROG_FORCE	0x50455249

#define QSPI_S25FL_SOFT_RESET_COMMAND	0x00f0ff82
#define QSPI_N25_SOFT_RESET_COMMAND	0x00000001
#define QSPI_NO_SOFT_RESET		0x00000000

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
	(struct pl310_regs *)CFG_SYS_PL310_BASE;
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

	writel(0x0007FFFF,
	       socfpga_get_sysmgr_addr() + SYSMGR_A10_ECC_INTMASK_SET);
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
	const u32 bootinfo = readl(socfpga_get_sysmgr_addr() +
				   SYSMGR_A10_BOOTINFO);
	const u32 bsel = SYSMGR_GET_BOOTINFO_BSEL(bootinfo);

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

/*
 * This function set/unset flag with number "0x50455249" to
 * handoff register isw_handoff[7] - 0xffd0624c
 * This flag is used to force periph RBF program regardless FPGA status
 * and double periph RBF config are needed on some devices or boards to
 * stabilize the IO config system.
 */
void force_periph_program(unsigned int status)
{
	if (status)
		writel(PERIPH_RBF_PROG_FORCE, socfpga_get_sysmgr_addr() +
		       SYSMGR_A10_ISW_HANDOFF_BASE + SYSMGR_A10_ISW_HANDOFF_7);
	else
		writel(0, socfpga_get_sysmgr_addr() +
		       SYSMGR_A10_ISW_HANDOFF_BASE + SYSMGR_A10_ISW_HANDOFF_7);
}

/*
 * This function is used to check whether
 * handoff register isw_handoff[7] contains
 * flag for forcing the periph RBF program "0x50455249".
 */
bool is_periph_program_force(void)
{
	unsigned int status;

	status = readl(socfpga_get_sysmgr_addr() +
		       SYSMGR_A10_ISW_HANDOFF_BASE + SYSMGR_A10_ISW_HANDOFF_7);

	if (status == PERIPH_RBF_PROG_FORCE)
		return true;
	else
		return false;
}

/*
 * This function set/unset magic number "0xd15ea5e" to
 * handoff register isw_handoff[7] - 0xffd0624c
 * This magic number is part of boot progress tracking
 * and it's required for warm reset workaround on MPFE hang issue.
 */
void set_regular_boot(unsigned int status)
{
	if (status)
		writel(REGULAR_BOOT_MAGIC, socfpga_get_sysmgr_addr() +
		       SYSMGR_A10_ISW_HANDOFF_BASE + SYSMGR_A10_ISW_HANDOFF_7);
	else
		writel(0, socfpga_get_sysmgr_addr() +
		       SYSMGR_A10_ISW_HANDOFF_BASE + SYSMGR_A10_ISW_HANDOFF_7);
}

/*
 * This function is used to check whether
 * handoff register isw_handoff[7] contains
 * magic number "0xd15ea5e".
 */
bool is_regular_boot_valid(void)
{
	unsigned int status;

	status = readl(socfpga_get_sysmgr_addr() +
		       SYSMGR_A10_ISW_HANDOFF_BASE + SYSMGR_A10_ISW_HANDOFF_7);

	if (status == REGULAR_BOOT_MAGIC)
		return true;
	else
		return false;
}

#if IS_ENABLED(CONFIG_CADENCE_QSPI)
/* This function is used to trigger software reset
 * to the QSPI flash. On some boards, the QSPI flash reset may
 * not be connected to the HPS warm reset.
 */
int qspi_flash_software_reset(void)
{
	struct udevice *flash;
	int ret;

	/* Get the flash info */
	ret = spi_flash_probe_bus_cs(CONFIG_SF_DEFAULT_BUS,
				     CONFIG_SF_DEFAULT_CS,
				     CONFIG_SF_DEFAULT_SPEED,
				     CONFIG_SF_DEFAULT_MODE,
				     &flash);

	if (ret) {
		debug("Failed to initialize SPI flash at ");
		debug("%u:%u (error %d)\n", CONFIG_SF_DEFAULT_BUS,
		      CONFIG_SF_DEFAULT_CS, ret);
		return -ENODEV;
	}

	if (!flash)
		return -EINVAL;

	/*
	 * QSPI flash software reset command, for the case where
	 * no HPS reset connected to QSPI flash reset
	 */
	if (!memcmp(flash->name, "N25", SZ_1 + SZ_2))
		writel(QSPI_N25_SOFT_RESET_COMMAND, socfpga_get_sysmgr_addr() +
		       SYSMGR_A10_ROMCODE_QSPIRESETCOMMAND);
	else if (!memcmp(flash->name, "S25FL", SZ_1 + SZ_4))
		writel(QSPI_S25FL_SOFT_RESET_COMMAND,
		       socfpga_get_sysmgr_addr() +
		       SYSMGR_A10_ROMCODE_QSPIRESETCOMMAND);
	else /* No software reset */
		writel(QSPI_NO_SOFT_RESET, socfpga_get_sysmgr_addr() +
		       SYSMGR_A10_ROMCODE_QSPIRESETCOMMAND);

	return 0;
}
#endif

void dram_bank_mmu_setup(int bank)
{
	struct bd_info *bd = gd->bd;
	u32 start, size;
	int i;

	/* If we're still in OCRAM, don't set the XN bit on it */
	if (!(gd->flags & GD_FLG_RELOC)) {
		set_section_dcache(
			CFG_SYS_INIT_RAM_ADDR >> MMU_SECTION_SHIFT,
			DCACHE_WRITETHROUGH);
	}

	/*
	 * The default implementation of this function allows the DRAM dcache
	 * to be enabled only after relocation. However, to speed up ECC
	 * initialization, we want to be able to enable DRAM dcache before
	 * relocation, so we don't check GD_FLG_RELOC (this assumes bd->bi_dram
	 * is set first).
	 */
	start = bd->bi_dram[bank].start >> MMU_SECTION_SHIFT;
	size = bd->bi_dram[bank].size >> MMU_SECTION_SHIFT;
	for (i = start; i < start + size; i++)
		set_section_dcache(i, DCACHE_DEFAULT_OPTION);
}
