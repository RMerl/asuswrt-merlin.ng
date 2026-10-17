// SPDX-License-Identifier: GPL-2.0+
/*
 * UART support for U-Boot when launched from Coreboot
 *
 * Copyright 2019 Google LLC
 */

#define LOG_CATGEGORY	UCLASS_SERIAL

#include <common.h>
#include <dm.h>
#include <log.h>
#include <ns16550.h>
#include <serial.h>
#include <acpi/acpi_table.h>
#include <asm/cb_sysinfo.h>

DECLARE_GLOBAL_DATA_PTR;

static int read_dbg2(struct ns16550_plat *plat)
{
	struct acpi_table_header *tab;
	struct acpi_dbg2_header *hdr;
	struct acpi_dbg2_device *dbg;
	struct acpi_gen_regaddr *addr;
	u32 *addr_size;

	log_debug("Looking for DBG2 in ACPI tables\n");
	if (!gd->acpi_start) {
		log_debug("No ACPI tables\n");
		return -ENOENT;
	}

	tab = acpi_find_table("DBG2");
	if (!tab) {
		log_debug("No DBG2 table\n");
		return -ENOENT;
	}
	hdr = container_of(tab, struct acpi_dbg2_header, header);

	/* We only use the first device, but check that there is at least one */
	if (!hdr->devices_count) {
		log_debug("No devices\n");
		return -ENOENT;
	}
	if (hdr->devices_offset >= tab->length) {
		log_debug("Invalid offset\n");
		return -EINVAL;
	}
	dbg = (void *)hdr + hdr->devices_offset;
	if (dbg->revision) {
		log_debug("Invalid revision %d\n", dbg->revision);
		return -EINVAL;
	}
	if (!dbg->address_count) {
		log_debug("No addresses\n");
		return -EINVAL;
	}
	if (dbg->port_type != ACPI_DBG2_SERIAL_PORT) {
		log_debug("Not a serial port\n");
		return -EPROTOTYPE;
	}
	if (dbg->port_subtype != ACPI_DBG2_16550_COMPATIBLE) {
		log_debug("Incompatible serial port\n");
		return -EPROTOTYPE;
	}
	if (dbg->base_address_offset >= dbg->length ||
	    dbg->address_size_offset >= dbg->length) {
		log_debug("Invalid base address/size offsets %d, %d\n",
			  dbg->base_address_offset, dbg->address_size_offset);
		return -EINVAL;
	}
	addr_size = (void *)dbg + dbg->address_size_offset;
	if (!*addr_size) {
		log_debug("Zero address size\n");
		return -EINVAL;
	}
	addr = (void *)dbg + dbg->base_address_offset;
	if (addr->space_id != ACPI_ADDRESS_SPACE_MEMORY) {
		log_debug("Incompatible space %d\n", addr->space_id);
		return -EPROTOTYPE;
	}

	plat->base = addr->addrl;

	/* ACPI_ACCESS_SIZE_DWORD_ACCESS is 3; we want 2 */
	plat->reg_shift = addr->access_size - 1;
	plat->reg_width = 4; /* coreboot sets bit_width to 0 */
	plat->clock = 1843200;
	plat->fcr = UART_FCR_DEFVAL;
	plat->flags = 0;
	log_debug("Collected UART from ACPI DBG2 table\n");

	return 0;
}

static int coreboot_of_to_plat(struct udevice *dev)
{
	struct ns16550_plat *plat = dev_get_plat(dev);
	struct cb_serial *cb_info = lib_sysinfo.serial;
	int ret = -ENOENT;

	if (cb_info) {
		plat->base = cb_info->baseaddr;
		plat->reg_shift = cb_info->regwidth == 4 ? 2 : 0;
		plat->reg_width = cb_info->regwidth;
		plat->clock = cb_info->input_hertz;
		plat->fcr = UART_FCR_DEFVAL;
		plat->flags = 0;
		if (cb_info->type == CB_SERIAL_TYPE_IO_MAPPED)
			plat->flags |= NS16550_FLAG_IO;
		ret = 0;
	} else if (IS_ENABLED(CONFIG_COREBOOT_SERIAL_FROM_DBG2)) {
		ret = read_dbg2(plat);
	}

	if (ret) {
		/*
		 * Returning an error will cause U-Boot to complain that
		 * there is no UART, which may panic. So stay silent and
		 * pray that the video console will work.
		 */
		log_debug("Cannot detect UART\n");
	}

	return 0;
}

static const struct udevice_id coreboot_serial_ids[] = {
	{ .compatible = "coreboot-serial" },
	{ },
};

U_BOOT_DRIVER(coreboot_uart) = {
	.name	= "coreboot_uart",
	.id	= UCLASS_SERIAL,
	.of_match	= coreboot_serial_ids,
	.priv_auto	= sizeof(struct ns16550),
	.plat_auto	= sizeof(struct ns16550_plat),
	.of_to_plat  = coreboot_of_to_plat,
	.probe	= ns16550_serial_probe,
	.ops	= &ns16550_serial_ops,
	.flags	= DM_FLAG_PRE_RELOC,
};
