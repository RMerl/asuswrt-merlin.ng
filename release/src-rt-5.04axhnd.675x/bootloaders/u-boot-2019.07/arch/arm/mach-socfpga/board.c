// SPDX-License-Identifier: GPL-2.0+
/*
 * Altera SoCFPGA common board code
 *
 * Copyright (C) 2015 Marek Vasut <marex@denx.de>
 */

#include <common.h>
#include <errno.h>
#include <fdtdec.h>
#include <asm/arch/reset_manager.h>
#include <asm/arch/clock_manager.h>
#include <asm/arch/misc.h>
#include <asm/io.h>

#include <usb.h>
#include <usb/dwc2_udc.h>

DECLARE_GLOBAL_DATA_PTR;

void s_init(void) {
#ifndef CONFIG_ARM64
	/*
	 * Preconfigure ACTLR and CPACR, make sure Write Full Line of Zeroes
	 * is disabled in ACTLR.
	 * This is optional on CycloneV / ArriaV.
	 * This is mandatory on Arria10, otherwise Linux refuses to boot.
	 */
	asm volatile(
		"mcr p15, 0, %0, c1, c0, 1\n"
		"mcr p15, 0, %0, c1, c0, 2\n"
		"isb\n"
		"dsb\n"
	::"r"(0x0));
#endif
}

/*
 * Miscellaneous platform dependent initialisations
 */
int board_init(void)
{
	/* Address of boot parameters for ATAG (if ATAG is used) */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	return 0;
}

int dram_init_banksize(void)
{
	fdtdec_setup_memory_banksize();

	return 0;
}

#ifdef CONFIG_USB_GADGET
struct dwc2_plat_otg_data socfpga_otg_data = {
	.usb_gusbcfg	= 0x1417,
};

int board_usb_init(int index, enum usb_init_type init)
{
	int node[2], count;
	fdt_addr_t addr;

	count = fdtdec_find_aliases_for_id(gd->fdt_blob, "udc",
					   COMPAT_ALTERA_SOCFPGA_DWC2USB,
					   node, 2);
	if (count <= 0)	/* No controller found. */
		return 0;

	addr = fdtdec_get_addr(gd->fdt_blob, node[0], "reg");
	if (addr == FDT_ADDR_T_NONE) {
		printf("UDC Controller has no 'reg' property!\n");
		return -EINVAL;
	}

	/* Patch the address from OF into the controller pdata. */
	socfpga_otg_data.regs_otg = addr;

	return dwc2_udc_probe(&socfpga_otg_data);
}

int g_dnl_board_usb_cable_connected(void)
{
	return 1;
}
#endif
