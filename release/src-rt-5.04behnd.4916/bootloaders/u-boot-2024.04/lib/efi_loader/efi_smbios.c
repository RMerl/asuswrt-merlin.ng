// SPDX-License-Identifier: GPL-2.0+
/*
 *  EFI application tables support
 *
 *  Copyright (c) 2016 Alexander Graf
 */

#define LOG_CATEGORY LOGC_EFI

#include <efi_loader.h>
#include <log.h>
#include <malloc.h>
#include <mapmem.h>
#include <smbios.h>
#include <linux/sizes.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

const efi_guid_t smbios3_guid = SMBIOS3_TABLE_GUID;

enum {
	TABLE_SIZE	= SZ_4K,
};

/*
 * Install the SMBIOS table as a configuration table.
 *
 * Return:	status code
 */
efi_status_t efi_smbios_register(void)
{
	ulong addr;
	efi_status_t ret;
	void *buf;

	addr = gd_smbios_start();
	if (!addr) {
		log_err("No SMBIOS tables to install\n");
		return EFI_NOT_FOUND;
	}

	/* Mark space used for tables */
	ret = efi_add_memory_map(addr, TABLE_SIZE, EFI_RUNTIME_SERVICES_DATA);
	if (ret)
		return ret;

	log_debug("EFI using SMBIOS tables at %lx\n", addr);

	/* Install SMBIOS information as configuration table */
	buf = map_sysmem(addr, 0);
	ret = efi_install_configuration_table(&smbios3_guid, buf);
	unmap_sysmem(buf);

	return ret;
}

static int install_smbios_table(void)
{
	ulong addr;
	void *buf;

	if (!IS_ENABLED(CONFIG_GENERATE_SMBIOS_TABLE) ||
	    IS_ENABLED(CONFIG_X86) ||
	    IS_ENABLED(CONFIG_QFW_SMBIOS))
		return 0;

	/* Align the table to a 4KB boundary to keep EFI happy */
	buf = memalign(SZ_4K, TABLE_SIZE);
	if (!buf)
		return log_msg_ret("mem", -ENOMEM);

	addr = map_to_sysmem(buf);
	if (!write_smbios_table(addr)) {
		log_err("Failed to write SMBIOS table\n");
		return log_msg_ret("smbios", -EINVAL);
	}

	/* Make a note of where we put it */
	log_debug("SMBIOS tables written to %lx\n", addr);
	gd->arch.smbios_start = addr;

	return 0;
}
EVENT_SPY_SIMPLE(EVT_LAST_STAGE_INIT, install_smbios_table);
