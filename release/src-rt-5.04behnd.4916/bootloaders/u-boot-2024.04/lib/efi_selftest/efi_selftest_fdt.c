// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_fdt
 *
 * Copyright (c) 2018 Heinrich Schuchardt <xypron.glpk@gmx.de>
 * Copyright (c) 2022 Ventana Micro Systems Inc
 *
 * Check the device tree, test the RISCV_EFI_BOOT_PROTOCOL.
 */

#include <efi_riscv.h>
#include <efi_selftest.h>
#include <linux/libfdt.h>

static const struct efi_system_table *systemtab;
static const struct efi_boot_services *boottime;
static const char *fdt;

/* This should be sufficient for */
#define BUFFERSIZE 0x100000

static const efi_guid_t fdt_guid = EFI_FDT_GUID;
static const efi_guid_t acpi_guid = EFI_ACPI_TABLE_GUID;
static const efi_guid_t riscv_efi_boot_protocol_guid =
				RISCV_EFI_BOOT_PROTOCOL_GUID;

/**
 * f2h() - convert FDT value to host endianness.
 *
 * UEFI code is always low endian. The FDT is big endian.
 *
 * @val:	FDT value
 * Return:	converted value
 */
static uint32_t f2h(fdt32_t val)
{
	char *buf = (char *)&val;
	char i;

	/* Swap the bytes */
	i = buf[0]; buf[0] = buf[3]; buf[3] = i;
	i = buf[1]; buf[1] = buf[2]; buf[2] = i;

	return val;
}

/**
 * get_property() - return value of a property of an FDT node
 *
 * A property of the root node or one of its direct children can be
 * retrieved.
 *
 * @property	name of the property
 * @node	name of the node or NULL for root node
 * Return:	value of the property
 */
static char *get_property(const u16 *property, const u16 *node)
{
	struct fdt_header *header = (struct fdt_header *)fdt;
	const fdt32_t *end;
	const fdt32_t *pos;
	const char *strings;
	size_t level = 0;
	const char *nodelabel = NULL;

	if (!header) {
		efi_st_error("Missing device tree\n");
		return NULL;
	}

	if (f2h(header->magic) != FDT_MAGIC) {
		efi_st_error("Wrong device tree magic\n");
		return NULL;
	}

	pos = (fdt32_t *)(fdt + f2h(header->off_dt_struct));
	end = &pos[f2h(header->totalsize) >> 2];
	strings = fdt + f2h(header->off_dt_strings);

	for (; pos < end;) {
		switch (f2h(pos[0])) {
		case FDT_BEGIN_NODE: {
			const char *c = (char *)&pos[1];
			size_t i;

			if (level == 1)
				nodelabel = c;
			++level;
			for (i = 0; c[i]; ++i)
				;
			pos = &pos[2 + (i >> 2)];
			break;
		}
		case FDT_PROP: {
			struct fdt_property *prop = (struct fdt_property *)pos;
			const char *label = &strings[f2h(prop->nameoff)];
			efi_status_t ret;

			/* Check if this is the property to be returned */
			if (!efi_st_strcmp_16_8(property, label) &&
			    ((level == 1 && !node) ||
			     (level == 2 && node &&
			      !efi_st_strcmp_16_8(node, nodelabel)))) {
				char *str;
				efi_uintn_t len = f2h(prop->len);

				if (!len)
					return NULL;
				/*
				 * The string might not be 0 terminated.
				 * It is safer to make a copy.
				 */
				ret = boottime->allocate_pool(
					EFI_LOADER_DATA, len + 1,
					(void **)&str);
				if (ret != EFI_SUCCESS) {
					efi_st_error("AllocatePool failed\n");
					return NULL;
				}
				boottime->copy_mem(str, &pos[3], len);
				str[len] = 0;

				return str;
			}

			pos = &pos[3 + ((f2h(prop->len) + 3) >> 2)];
			break;
		}
		case FDT_NOP:
			++pos;
			break;
		case FDT_END_NODE:
			--level;
			++pos;
			break;
		case FDT_END:
			return NULL;
		default:
			efi_st_error("Invalid device tree token\n");
			return NULL;
		}
	}
	efi_st_error("Missing FDT_END token\n");
	return NULL;
}

/*
 * Setup unit test.
 *
 * @handle:	handle of the loaded image
 * @systable:	system table
 * Return:	EFI_ST_SUCCESS for success
 */
static int setup(const efi_handle_t img_handle,
		 const struct efi_system_table *systable)
{
	void *acpi;

	systemtab = systable;
	boottime = systable->boottime;

	acpi = efi_st_get_config_table(&acpi_guid);
	fdt = efi_st_get_config_table(&fdt_guid);

	if (!fdt) {
		efi_st_error("Missing device tree\n");
		return EFI_ST_FAILURE;
	}
	if (acpi) {
		efi_st_error("Found ACPI table and device tree\n");
		return EFI_ST_FAILURE;
	}
	return EFI_ST_SUCCESS;
}

__maybe_unused static efi_status_t get_boot_hartid(efi_uintn_t *efi_hartid)
{
	efi_status_t ret;
	struct riscv_efi_boot_protocol *prot;

	/* Get RISC-V boot protocol */
	ret = boottime->locate_protocol(&riscv_efi_boot_protocol_guid, NULL,
					(void **)&prot);
	if (ret != EFI_SUCCESS) {
		efi_st_error("RISC-V Boot Protocol not available\n");
		return EFI_ST_FAILURE;
	}

	/* Get boot hart ID from EFI protocol */
	ret = prot->get_boot_hartid(prot, efi_hartid);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Could not retrieve boot hart ID\n");
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

/*
 * Execute unit test.
 *
 * Return:	EFI_ST_SUCCESS for success
 */
static int execute(void)
{
	char *str;
	efi_status_t ret;

	str = get_property(u"compatible", NULL);
	if (str) {
		efi_st_printf("compatible: %s\n", str);
		ret = boottime->free_pool(str);
		if (ret != EFI_SUCCESS) {
			efi_st_error("FreePool failed\n");
			return EFI_ST_FAILURE;
		}
	} else {
		efi_st_error("Missing property 'compatible'\n");
		return EFI_ST_FAILURE;
	}
	str = get_property(u"serial-number", NULL);
	if (str) {
		efi_st_printf("serial-number: %s\n", str);
		ret = boottime->free_pool(str);
		if (ret != EFI_SUCCESS) {
			efi_st_error("FreePool failed\n");
			return EFI_ST_FAILURE;
		}
	}
	if (IS_ENABLED(CONFIG_RISCV)) {
		u32 fdt_hartid;

		str = get_property(u"boot-hartid", u"chosen");
		if (!str) {
			efi_st_error("boot-hartid missing in devicetree\n");
			return EFI_ST_FAILURE;
		}
		fdt_hartid = f2h(*(fdt32_t *)str);
		efi_st_printf("boot-hartid: %u\n", fdt_hartid);

		ret = boottime->free_pool(str);
		if (ret != EFI_SUCCESS) {
			efi_st_error("FreePool failed\n");
			return EFI_ST_FAILURE;
		}

		if (IS_ENABLED(CONFIG_EFI_RISCV_BOOT_PROTOCOL)) {
			efi_uintn_t efi_hartid;
			int r;

			r = get_boot_hartid(&efi_hartid);
			if (r != EFI_ST_SUCCESS)
				return r;
			/* Boot hart ID should be same */
			if (efi_hartid != fdt_hartid) {
				efi_st_error("boot-hartid differs: prot 0x%p, DT 0x%.8x\n",
					     (void *)(uintptr_t)efi_hartid,
					     fdt_hartid);
				return EFI_ST_FAILURE;
			}
		}
	}

	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(fdt) = {
	.name = "device tree",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
};
