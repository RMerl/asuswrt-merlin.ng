// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 *
 * EFI information obtained here:
 * http://wiki.phoenix.com/wiki/index.php/EFI_BOOT_SERVICES
 *
 * This file implements U-Boot running as an EFI application.
 */

#include <cpu_func.h>
#include <debug_uart.h>
#include <dm.h>
#include <efi.h>
#include <efi_api.h>
#include <errno.h>
#include <init.h>
#include <malloc.h>
#include <sysreset.h>
#include <uuid.h>
#include <asm/global_data.h>
#include <linux/err.h>
#include <linux/types.h>
#include <asm/global_data.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/root.h>
#include <mapmem.h>

DECLARE_GLOBAL_DATA_PTR;

int efi_info_get(enum efi_entry_t type, void **datap, int *sizep)
{
	return -ENOSYS;
}

int efi_get_mmap(struct efi_mem_desc **descp, int *sizep, uint *keyp,
		 int *desc_sizep, uint *versionp)
{
	struct efi_priv *priv = efi_get_priv();
	struct efi_boot_services *boot = priv->sys_table->boottime;
	efi_uintn_t size, desc_size, key;
	struct efi_mem_desc *desc;
	efi_status_t ret;
	u32 version;

	/* Get the memory map so we can switch off EFI */
	size = 0;
	ret = boot->get_memory_map(&size, NULL, &key, &desc_size, &version);
	if (ret != EFI_BUFFER_TOO_SMALL)
		return log_msg_ret("get", -ENOMEM);

	desc = malloc(size);
	if (!desc)
		return log_msg_ret("mem", -ENOMEM);

	ret = boot->get_memory_map(&size, desc, &key, &desc_size, &version);
	if (ret)
		return log_msg_ret("get", -EINVAL);

	*descp = desc;
	*sizep = size;
	*desc_sizep = desc_size;
	*versionp = version;
	*keyp = key;

	return 0;
}

static efi_status_t setup_memory(struct efi_priv *priv)
{
	struct efi_boot_services *boot = priv->boot;
	efi_physical_addr_t addr;
	efi_status_t ret;
	int pages;

	/*
	 * Use global_data_ptr instead of gd since it is an assignment. There
	 * are very few assignments to global_data in U-Boot and this makes
	 * it easier to find them.
	 */
	global_data_ptr = efi_malloc(priv, sizeof(struct global_data), &ret);
	if (!global_data_ptr)
		return ret;
	memset(gd, '\0', sizeof(*gd));

	gd->malloc_base = (ulong)efi_malloc(priv, CONFIG_VAL(SYS_MALLOC_F_LEN),
					    &ret);
	if (!gd->malloc_base)
		return ret;
	pages = CONFIG_EFI_RAM_SIZE >> 12;

	/*
	 * Don't allocate any memory above 4GB. U-Boot is a 32-bit application
	 * so we want it to load below 4GB.
	 */
	addr = 1ULL << 32;
	ret = boot->allocate_pages(EFI_ALLOCATE_MAX_ADDRESS,
				   priv->image_data_type, pages, &addr);
	if (ret) {
		log_info("(using pool %lx) ", ret);
		priv->ram_base = (ulong)efi_malloc(priv, CONFIG_EFI_RAM_SIZE,
						   &ret);
		if (!priv->ram_base)
			return ret;
		priv->use_pool_for_malloc = true;
	} else {
		log_info("(using allocated RAM address %lx) ", (ulong)addr);
		priv->ram_base = addr;
	}
	gd->ram_size = pages << 12;

	return 0;
}

/**
 * free_memory() - Free memory used by the U-Boot app
 *
 * This frees memory allocated in setup_memory(), in preparation for returning
 * to UEFI. It also zeroes the global_data pointer.
 *
 * @priv: Private EFI data
 */
static void free_memory(struct efi_priv *priv)
{
	struct efi_boot_services *boot = priv->boot;

	if (priv->use_pool_for_malloc)
		efi_free(priv, (void *)priv->ram_base);
	else
		boot->free_pages(priv->ram_base, gd->ram_size >> 12);

	efi_free(priv, (void *)gd->malloc_base);
	efi_free(priv, gd);
	global_data_ptr = NULL;
}

static void scan_tables(struct efi_system_table *sys_table)
{
	efi_guid_t acpi = EFI_ACPI_TABLE_GUID;
	uint i;

	for (i = 0; i < sys_table->nr_tables; i++) {
		struct efi_configuration_table *tab = &sys_table->tables[i];

		if (!memcmp(&tab->guid, &acpi, sizeof(efi_guid_t)))
			gd_set_acpi_start(map_to_sysmem(tab->table));
	}
}

/**
 * efi_main() - Start an EFI image
 *
 * This function is called by our EFI start-up code. It handles running
 * U-Boot. If it returns, EFI will continue. Another way to get back to EFI
 * is via reset_cpu().
 */
efi_status_t EFIAPI efi_main(efi_handle_t image,
			     struct efi_system_table *sys_table)
{
	struct efi_priv local_priv, *priv = &local_priv;
	efi_status_t ret;

	/* Set up access to EFI data structures */
	ret = efi_init(priv, "App", image, sys_table);
	if (ret) {
		printf("Failed to set up U-Boot: err=%lx\n", ret);
		return ret;
	}
	efi_set_priv(priv);

	/*
	 * Set up the EFI debug UART so that printf() works. This is
	 * implemented in the EFI serial driver, serial_efi.c. The application
	 * can use printf() freely.
	 */
	debug_uart_init();

	ret = setup_memory(priv);
	if (ret) {
		printf("Failed to set up memory: ret=%lx\n", ret);
		return ret;
	}

	scan_tables(priv->sys_table);

	/*
	 * We could store the EFI memory map here, but it changes all the time,
	 * so this is only useful for debugging.
	 *
	 * ret = efi_store_memory_map(priv);
	 * if (ret)
	 *	return ret;
	 */

	printf("starting\n");

	board_init_f(GD_FLG_SKIP_RELOC);
	board_init_r(NULL, 0);
	free_memory(priv);

	return EFI_SUCCESS;
}

static void efi_exit(void)
{
	struct efi_priv *priv = efi_get_priv();

	free_memory(priv);
	printf("U-Boot EFI exiting\n");
	priv->boot->exit(priv->parent_image, EFI_SUCCESS, 0, NULL);
}

static int efi_sysreset_request(struct udevice *dev, enum sysreset_t type)
{
	efi_exit();

	return -EINPROGRESS;
}

static const struct udevice_id efi_sysreset_ids[] = {
	{ .compatible = "efi,reset" },
	{ }
};

static struct sysreset_ops efi_sysreset_ops = {
	.request = efi_sysreset_request,
};

U_BOOT_DRIVER(efi_sysreset) = {
	.name = "efi-sysreset",
	.id = UCLASS_SYSRESET,
	.of_match = efi_sysreset_ids,
	.ops = &efi_sysreset_ops,
};
