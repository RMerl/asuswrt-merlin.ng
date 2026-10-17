// SPDX-License-Identifier: GPL-2.0+
/*
 * EFI_DT_FIXUP_PROTOCOL
 *
 * Copyright (c) 2020 Heinrich Schuchardt
 */

#include <efi_dt_fixup.h>
#include <efi_loader.h>
#include <efi_rng.h>
#include <fdtdec.h>
#include <mapmem.h>

const efi_guid_t efi_guid_dt_fixup_protocol = EFI_DT_FIXUP_PROTOCOL_GUID;

/**
 * efi_reserve_memory() - add reserved memory to memory map
 *
 * @addr:	start address of the reserved memory range
 * @size:	size of the reserved memory range
 * @nomap:	indicates that the memory range shall not be accessed by the
 *		UEFI payload
 */
static void efi_reserve_memory(u64 addr, u64 size, bool nomap)
{
	int type;
	efi_uintn_t ret;

	/* Convert from sandbox address space. */
	addr = (uintptr_t)map_sysmem(addr, 0);

	if (nomap)
		type = EFI_RESERVED_MEMORY_TYPE;
	else
		type = EFI_BOOT_SERVICES_DATA;

	ret = efi_add_memory_map(addr, size, type);
	if (ret != EFI_SUCCESS)
		log_err("Reserved memory mapping failed addr %llx size %llx\n",
			addr, size);
}

/**
 * efi_try_purge_kaslr_seed() - Remove unused kaslr-seed
 *
 * Kernel's EFI STUB only relies on EFI_RNG_PROTOCOL for randomization
 * and completely ignores the kaslr-seed for its own randomness needs
 * (i.e the randomization of the physical placement of the kernel).
 * Weed it out from the DTB we hand over, which would mess up our DTB
 * TPM measurements as well.
 *
 * @fdt: Pointer to device tree
 */
void efi_try_purge_kaslr_seed(void *fdt)
{
	const efi_guid_t efi_guid_rng_protocol = EFI_RNG_PROTOCOL_GUID;
	struct efi_handler *handler;
	efi_status_t ret;
	int nodeoff = 0;
	int err = 0;

	ret = efi_search_protocol(efi_root, &efi_guid_rng_protocol, &handler);
	if (ret != EFI_SUCCESS)
		return;

	nodeoff = fdt_path_offset(fdt, "/chosen");
	if (nodeoff < 0)
		return;

	err = fdt_delprop(fdt, nodeoff, "kaslr-seed");
	if (err < 0 && err != -FDT_ERR_NOTFOUND)
		log_err("Error deleting kaslr-seed\n");
}

/**
 * efi_carve_out_dt_rsv() - Carve out DT reserved memory ranges
 *
 * The mem_rsv entries of the FDT are added to the memory map. Any failures are
 * ignored because this is not critical and we would rather continue to try to
 * boot.
 *
 * @fdt: Pointer to device tree
 */
void efi_carve_out_dt_rsv(void *fdt)
{
	int nr_rsv, i;
	u64 addr, size;
	int nodeoffset, subnode;

	nr_rsv = fdt_num_mem_rsv(fdt);

	/* Look for an existing entry and add it to the efi mem map. */
	for (i = 0; i < nr_rsv; i++) {
		if (fdt_get_mem_rsv(fdt, i, &addr, &size) != 0)
			continue;
		efi_reserve_memory(addr, size, true);
	}

	/* process reserved-memory */
	nodeoffset = fdt_subnode_offset(fdt, 0, "reserved-memory");
	if (nodeoffset >= 0) {
		subnode = fdt_first_subnode(fdt, nodeoffset);
		while (subnode >= 0) {
			fdt_addr_t fdt_addr;
			fdt_size_t fdt_size;

			/* check if this subnode has a reg property */
			fdt_addr = fdtdec_get_addr_size_auto_parent(
						fdt, nodeoffset, subnode,
						"reg", 0, &fdt_size, false);
			/*
			 * The /reserved-memory node may have children with
			 * a size instead of a reg property.
			 */
			if (fdt_addr != FDT_ADDR_T_NONE &&
			    fdtdec_get_is_enabled(fdt, subnode)) {
				bool nomap;

				nomap = !!fdt_getprop(fdt, subnode, "no-map",
						      NULL);
				efi_reserve_memory(fdt_addr, fdt_size, nomap);
			}
			subnode = fdt_next_subnode(fdt, subnode);
		}
	}
}

/**
 * efi_dt_fixup() - fix up device tree
 *
 * This function implements the Fixup() service of the
 * EFI Device Tree Fixup Protocol.
 *
 * @this:		instance of the protocol
 * @dtb:		device tree provided by caller
 * @buffer_size:	size of buffer for the device tree including free space
 * @flags:		bit field designating action to be performed
 * Return:		status code
 */
static efi_status_t __maybe_unused EFIAPI
efi_dt_fixup(struct efi_dt_fixup_protocol *this, void *dtb,
	     efi_uintn_t *buffer_size, u32 flags)
{
	efi_status_t ret;
	size_t required_size;
	size_t total_size;
	struct bootm_headers img = { 0 };

	EFI_ENTRY("%p, %p, %p, %d", this, dtb, buffer_size, flags);

	if (this != &efi_dt_fixup_prot || !dtb || !buffer_size ||
	    !flags || (flags & ~EFI_DT_ALL)) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}
	if (fdt_check_header(dtb)) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}
	if (flags & EFI_DT_APPLY_FIXUPS) {
		/* Check size */
		required_size = fdt_off_dt_strings(dtb) +
				fdt_size_dt_strings(dtb) +
				0x3000;
		total_size = fdt_totalsize(dtb);
		if (required_size < total_size)
			required_size = total_size;
		if (required_size > *buffer_size) {
			*buffer_size = required_size;
			ret = EFI_BUFFER_TOO_SMALL;
			goto out;
		}

		fdt_set_totalsize(dtb, *buffer_size);
		if (image_setup_libfdt(&img, dtb, NULL)) {
			log_err("failed to process device tree\n");
			ret = EFI_INVALID_PARAMETER;
			goto out;
		}
	}
	if (flags & EFI_DT_RESERVE_MEMORY)
		efi_carve_out_dt_rsv(dtb);

	if (flags & EFI_DT_INSTALL_TABLE) {
		ret = efi_install_configuration_table(&efi_guid_fdt, dtb);
		if (ret != EFI_SUCCESS) {
			log_err("failed to install device tree\n");
			goto out;
		}
	}

	ret = EFI_SUCCESS;
out:
	return EFI_EXIT(ret);
}

struct efi_dt_fixup_protocol efi_dt_fixup_prot = {
	.revision = EFI_DT_FIXUP_PROTOCOL_REVISION,
	.fixup = efi_dt_fixup
};
