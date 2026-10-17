// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2019 Google LLC
 *
 * From coreboot Apollo Lake support lpc.c
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <spl.h>
#include <acpi/acpi_table.h>
#include <asm/cpu_common.h>
#include <asm/intel_acpi.h>
#include <asm/lpc_common.h>
#include <asm/pci.h>
#include <asm/arch/iomap.h>
#include <asm/arch/lpc.h>
#include <dm/acpi.h>
#include <linux/log2.h>

void lpc_enable_fixed_io_ranges(uint io_enables)
{
	pci_x86_clrset_config(PCH_DEV_LPC, LPC_IO_ENABLES, 0, io_enables,
			      PCI_SIZE_16);
}

/*
 * Find the first unused IO window.
 * Returns -1 if not found, 0 for reg 0x84, 1 for reg 0x88 ...
 */
static int find_unused_pmio_window(void)
{
	int i;
	ulong lgir;

	for (i = 0; i < LPC_NUM_GENERIC_IO_RANGES; i++) {
		pci_x86_read_config(PCH_DEV_LPC, LPC_GENERIC_IO_RANGE(i),
				    &lgir, PCI_SIZE_32);

		if (!(lgir & LPC_LGIR_EN))
			return i;
	}

	return -1;
}

int lpc_open_pmio_window(uint base, uint size)
{
	int i, lgir_reg_num;
	u32 lgir_reg_offset, lgir, window_size, alignment;
	ulong bridged_size, bridge_base;
	ulong reg;

	log_debug("LPC: Trying to open IO window from %x size %x\n", base,
		  size);

	bridged_size = 0;
	bridge_base = base;

	while (bridged_size < size) {
		/* Each IO range register can only open a 256-byte window */
		window_size = min(size, (uint)LPC_LGIR_MAX_WINDOW_SIZE);

		/* Window size must be a power of two for the AMASK to work */
		alignment = 1UL << (order_base_2(window_size));
		window_size = ALIGN(window_size, alignment);

		/* Address[15:2] in LGIR[15:12] and Mask[7:2] in LGIR[23:18] */
		lgir = (bridge_base & LPC_LGIR_ADDR_MASK) | LPC_LGIR_EN;
		lgir |= ((window_size - 1) << 16) & LPC_LGIR_AMASK_MASK;

		/* Skip programming if same range already programmed */
		for (i = 0; i < LPC_NUM_GENERIC_IO_RANGES; i++) {
			pci_x86_read_config(PCH_DEV_LPC,
					    LPC_GENERIC_IO_RANGE(i), &reg,
					    PCI_SIZE_32);
			if (lgir == reg)
				return -EALREADY;
		}

		lgir_reg_num = find_unused_pmio_window();
		if (lgir_reg_num < 0) {
			if (spl_phase() > PHASE_TPL) {
				log_err("LPC: Cannot open IO window: %lx size %lx\n",
					bridge_base, size - bridged_size);
				log_err("No more IO windows\n");
			}
			return -ENOSPC;
		}
		lgir_reg_offset = LPC_GENERIC_IO_RANGE(lgir_reg_num);

		pci_x86_write_config(PCH_DEV_LPC, lgir_reg_offset, lgir,
				     PCI_SIZE_32);

		log_debug("LPC: Opened IO window LGIR%d: base %lx size %x\n",
			  lgir_reg_num, bridge_base, window_size);

		bridged_size += window_size;
		bridge_base += window_size;
	}

	return 0;
}

void lpc_io_setup_comm_a_b(void)
{
	/* ComA Range 3F8h-3FFh [2:0] */
	u16 com_ranges = LPC_IOD_COMA_RANGE;
	u16 com_enable = LPC_IOE_COMA_EN;

	/* Setup I/O Decode Range Register for LPC */
	pci_write_config16(PCH_DEV_LPC, LPC_IO_DECODE, com_ranges);
	/* Enable ComA and ComB Port */
	lpc_enable_fixed_io_ranges(com_enable);
}

static int apl_acpi_lpc_get_name(const struct udevice *dev, char *out_name)
{
	return acpi_copy_name(out_name, "LPCB");
}

struct acpi_ops apl_lpc_acpi_ops = {
	.get_name	= apl_acpi_lpc_get_name,
#ifdef CONFIG_GENERATE_ACPI_TABLE
	.write_tables	= intel_southbridge_write_acpi_tables,
#endif
	.inject_dsdt	= southbridge_inject_dsdt,
};

#if CONFIG_IS_ENABLED(OF_REAL)
static const struct udevice_id apl_lpc_ids[] = {
	{ .compatible = "intel,apl-lpc" },
	{ }
};
#endif

/* All pads are LPC already configured by the hostbridge, so no probing here */
U_BOOT_DRIVER(intel_apl_lpc) = {
	.name		= "intel_apl_lpc",
	.id		= UCLASS_LPC,
	.of_match	= of_match_ptr(apl_lpc_ids),
	ACPI_OPS_PTR(&apl_lpc_acpi_ops)
};
