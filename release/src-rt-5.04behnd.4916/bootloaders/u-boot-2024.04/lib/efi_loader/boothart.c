// SPDX-License-Identifier: GPL-2.0+
/*
 * Check RISC-V boot hart ID
 *
 * Copyright 2022, Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * This test program reads the boot HART ID both from the device-tree from the
 * RISCV_EFI_BOOT_PROTOCOL and writes both values to the console.
 */

#include <efi_api.h>
#include <efi_riscv.h>
#include <linux/libfdt.h>

static const efi_guid_t riscv_efi_boot_protocol_guid =
		RISCV_EFI_BOOT_PROTOCOL_GUID;
static const efi_guid_t fdt_guid = EFI_FDT_GUID;

static struct efi_system_table *systable;
static struct efi_boot_services *boottime;
static struct efi_simple_text_output_protocol *con_out;
static const char *fdt;

/**
 * Print an unsigned 32bit value as decimal number to an u16 string
 *
 * @value:	value to be printed
 * @buf:	pointer to buffer address
 */
static void uint2dec(u32 value, u16 *buf)
{
	u16 *pos = buf;
	int i;
	u16 c;
	u64 f;

	/*
	 * Increment by .5 and multiply with
	 * (2 << 60) / 1,000,000,000 = 0x44B82FA0.9B5A52CC
	 * to move the first digit to bit 60-63.
	 */
	f = 0x225C17D0;
	f += (0x9B5A52DULL * value) >> 28;
	f += 0x44B82FA0ULL * value;

	for (i = 0; i < 10; ++i) {
		/* Write current digit */
		c = f >> 60;
		if (c || pos != buf)
			*pos++ = c + '0';
		/* Eliminate current digit */
		f &= 0xfffffffffffffff;
		/* Get next digit */
		f *= 0xaULL;
	}
	if (pos == buf)
		*pos++ = '0';
	*pos = 0;
}

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
 * memcomp() - compare two memory buffers
 *
 * s1:		first buffer
 * s2:		second buffer
 * n:		size of buffers
 * Return:	0 if both buffers have the same content
 */
static int memcomp(const void *s1, const void *s2, size_t n)
{
	const char *pos1 = s1, *pos2 = s2;

	for (size_t count = 0; count < n ; ++pos1, ++pos2, --count) {
		if (*pos1 != *pos2)
			return *pos1 - *pos2;
	}
	return 0;
}

/**
 * strcomp() - compare to strings
 *
 * @buf1:	first string
 * @buf2:	second string
 * Return:	0 if both strings are the same
 */
static int strcomp(const char *buf1, const char *buf2)
{
	for (; *buf1 || *buf2; ++buf1, ++buf2) {
		if (*buf1 != *buf2)
			return *buf1 - *buf2;
	}
	return 0;
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
static char *get_property(const char *property, const char *node)
{
	struct fdt_header *header = (struct fdt_header *)fdt;
	const fdt32_t *end;
	const fdt32_t *pos;
	const char *strings;
	size_t level = 0;
	const char *nodelabel = NULL;

	if (!header) {
		con_out->output_string(con_out, u"Missing device tree\r\n");
		return NULL;
	}

	if (f2h(header->magic) != FDT_MAGIC) {
		con_out->output_string(con_out, u"Wrong device tree magic\r\n");
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
			if (!strcomp(property, label) &&
			    ((level == 1 && !node) ||
			     (level == 2 && node &&
			      !strcomp(node, nodelabel)))) {
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
					con_out->output_string(
						    con_out,
						    u"AllocatePool failed\r\n");
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
			con_out->output_string(
				con_out, u"Invalid device tree token\r\n");
			return NULL;
		}
	}
	con_out->output_string(
		con_out, u"Missing FDT_END token\r\n");
	return NULL;
}

/**
 * get_config_table() - get configuration table
 *
 * @guid:	table GUID
 * Return:	pointer to table or NULL
 */
static void *get_config_table(const efi_guid_t *guid)
{
	size_t i;

	for (i = 0; i < systable->nr_tables; i++) {
		if (!memcomp(guid, &systable->tables[i].guid, 16))
			return systable->tables[i].table;
	}
	return NULL;
}

/**
 * fdt_get_hart() - get hart ID via RISC-V device-tree
 *
 * @hartid:	boot hart ID
 * Return:	status code
 */
static efi_status_t fdt_get_hart(efi_uintn_t *hartid)
{
	char *str;

	fdt = get_config_table(&fdt_guid);
	if (!fdt) {
		con_out->output_string(con_out, u"Missing device tree\r\n");
		return EFI_NOT_FOUND;
	}

	str = get_property("boot-hartid", "chosen");
	if (!str) {
		con_out->output_string(con_out,
				       u"/chosen/boot-hartid missing\r\n");
		return EFI_NOT_FOUND;
	}
	*hartid = f2h(*(fdt32_t *)str);
	boottime->free_pool(str);

	return EFI_SUCCESS;
}

/**
 * prot_get_hart() - get hart ID via RISC-V Boot Protocol
 *
 * @hartid:	boot hart ID
 * Return:	status code
 */
static efi_status_t prot_get_hart(efi_uintn_t *hartid)
{
	efi_status_t ret;
	struct riscv_efi_boot_protocol *prot;

	/* Get RISC-V boot protocol */
	ret = boottime->locate_protocol(&riscv_efi_boot_protocol_guid, NULL,
					(void **)&prot);
	if (ret != EFI_SUCCESS) {
		con_out->output_string(
			con_out, u"RISC-V Boot Protocol not available\r\n");
		return ret;
	}

	/* Get boot hart ID from EFI protocol */
	ret = prot->get_boot_hartid(prot, hartid);
	if (ret != EFI_SUCCESS)
		con_out->output_string(con_out,
				       u"Could not retrieve boot hart ID\r\n");
	return ret;
}

/**
 * efi_main() - entry point of the EFI application.
 *
 * @handle:	handle of the loaded image
 * @systab:	system table
 * Return:	status code
 */
efi_status_t EFIAPI efi_main(efi_handle_t handle,
			     struct efi_system_table *systab)
{
	efi_status_t ret;
	efi_uintn_t hartid;
	u16 buf[16];

	systable = systab;
	boottime = systable->boottime;
	con_out = systable->con_out;

	con_out->output_string(con_out,
			       u"\r\nBoot hart ID\r\n------------\r\n\r\n");

	ret = fdt_get_hart(&hartid);
	if (ret == EFI_SUCCESS) {
		con_out->output_string(con_out, u"Device-tree: ");
		uint2dec(hartid, buf);
		con_out->output_string(con_out, buf);
		con_out->output_string(con_out, u"\r\n");
	}

	ret = prot_get_hart(&hartid);
	if (ret == EFI_SUCCESS) {
		con_out->output_string(con_out, u"RISCV_EFI_BOOT_PROTOCOL: ");
		uint2dec(hartid, buf);
		con_out->output_string(con_out, buf);
		con_out->output_string(con_out, u"\r\n");
	}

	con_out->output_string(con_out, u"\r\n");
	boottime->exit(handle, EFI_SUCCESS, 0, NULL);

	/* We should never arrive here */
	return EFI_SUCCESS;
}
