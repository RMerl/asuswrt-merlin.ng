// SPDX-License-Identifier: GPL-2.0
/*
 * Read a coreboot rmodule and execute it.
 * The rmodule_header struct is from coreboot.
 *
 * Copyright (c) 2016 Google, Inc
 */

#include <common.h>
#include <errno.h>
#include <asm/arch/pei_data.h>

#define RMODULE_MAGIC		0xf8fe
#define RMODULE_VERSION_1	1

/*
 * All fields with '_offset' in the name are byte offsets into the flat blob.
 * The linker and the linker script takes are of assigning the values.
 */
struct rmodule_header {
	uint16_t magic;
	uint8_t version;
	uint8_t type;
	/* The payload represents the program's loadable code and data */
	uint32_t payload_begin_offset;
	uint32_t payload_end_offset;
	/* Begin and of relocation information about the program module */
	uint32_t relocations_begin_offset;
	uint32_t relocations_end_offset;
	/*
	 * The starting address of the linked program. This address is vital
	 * for determining relocation offsets as the relocation info and other
	 * symbols (bss, entry point) need this value as a basis to calculate
	 * the offsets.
	 */
	uint32_t module_link_start_address;
	/*
	 * The module_program_size is the size of memory used while running
	 * the program. The program is assumed to consume a contiguous amount
	 * of memory
	 */
	uint32_t module_program_size;
	/* This is program's execution entry point */
	uint32_t module_entry_point;
	/*
	 * Optional parameter structure that can be used to pass data into
	 * the module
	 */
	uint32_t parameters_begin;
	uint32_t parameters_end;
	/* BSS section information so the loader can clear the bss */
	uint32_t bss_begin;
	uint32_t bss_end;
	/* Add some room for growth */
	uint32_t padding[4];
} __packed;

/**
 * cpu_run_reference_code() - Run the platform reference code
 *
 * Some platforms require a binary blob to be executed once SDRAM is
 * available. This is used to set up various platform features, such as the
 * platform controller hub (PCH). This function should be implemented by the
 * CPU-specific code.
 *
 * @return 0 on success, -ve on failure
 */
static int cpu_run_reference_code(void)
{
	struct pei_data _pei_data __aligned(8);
	struct pei_data *pei_data = &_pei_data;
	asmlinkage int (*func)(void *);
	struct rmodule_header *hdr;
	char *src, *dest;
	int ret, dummy;
	int size;

	hdr = (struct rmodule_header *)CONFIG_X86_REFCODE_ADDR;
	debug("Extracting code from rmodule at %p\n", hdr);
	if (hdr->magic != RMODULE_MAGIC) {
		debug("Invalid rmodule magic\n");
		return -EINVAL;
	}
	if (hdr->module_link_start_address != 0) {
		debug("Link start address must be 0\n");
		return -EPERM;
	}
	if (hdr->module_entry_point != 0) {
		debug("Entry point must be 0\n");
		return -EPERM;
	}

	memset(pei_data, '\0', sizeof(struct pei_data));
	broadwell_fill_pei_data(pei_data);
	mainboard_fill_pei_data(pei_data);
	pei_data->saved_data = (void *)&dummy;

	src = (char *)hdr + hdr->payload_begin_offset;
	dest = (char *)CONFIG_X86_REFCODE_RUN_ADDR;

	size = hdr->payload_end_offset - hdr->payload_begin_offset;
	debug("Copying refcode from %p to %p, size %x\n", src, dest, size);
	memcpy(dest, src, size);

	size = hdr->bss_end - hdr->bss_begin;
	debug("Zeroing BSS at %p, size %x\n", dest + hdr->bss_begin, size);
	memset(dest + hdr->bss_begin, '\0', size);

	func = (asmlinkage int (*)(void *))dest;
	debug("Running reference code at %p\n", func);
#ifdef DEBUG
	print_buffer(CONFIG_X86_REFCODE_RUN_ADDR, (void *)func, 1, 0x40, 0);
#endif
	ret = func(pei_data);
	if (ret != 0) {
		debug("Reference code returned %d\n", ret);
		return -EL2HLT;
	}
	debug("Refereence code completed\n");

	return 0;
}

int arch_early_init_r(void)
{
	return cpu_run_reference_code();
}
