// SPDX-License-Identifier: GPL-2.0+
/*
 * Hello world EFI application
 *
 * Copyright 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * This test program is used to test the invocation of an EFI application.
 * It writes
 *
 * * a greeting
 * * the firmware's UEFI version
 * * the installed configuration tables
 * * the boot device's device path and the file path
 *
 * to the console.
 */

#include <efi_api.h>

static const efi_guid_t loaded_image_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
static const efi_guid_t device_path_to_text_protocol_guid =
	EFI_DEVICE_PATH_TO_TEXT_PROTOCOL_GUID;
static const efi_guid_t device_path_guid = EFI_DEVICE_PATH_PROTOCOL_GUID;
static const efi_guid_t fdt_guid = EFI_FDT_GUID;
static const efi_guid_t acpi_guid = EFI_ACPI_TABLE_GUID;
static const efi_guid_t smbios_guid = SMBIOS_TABLE_GUID;

static struct efi_system_table *systable;
static struct efi_boot_services *boottime;
static struct efi_simple_text_output_protocol *con_out;

/*
 * Print an unsigned 32bit value as decimal number to an u16 string
 *
 * @value:	value to be printed
 * @buf:	pointer to buffer address
 *		on return position of terminating zero word
 */
static void uint2dec(u32 value, u16 **buf)
{
	u16 *pos = *buf;
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
		if (c || pos != *buf)
			*pos++ = c + '0';
		/* Eliminate current digit */
		f &= 0xfffffffffffffff;
		/* Get next digit */
		f *= 0xaULL;
	}
	if (pos == *buf)
		*pos++ = '0';
	*pos = 0;
	*buf = pos;
}

/**
 * print_uefi_revision() - print UEFI revision number
 */
static void print_uefi_revision(void)
{
	u16 rev[13] = {0};
	u16 *buf = rev;
	u16 digit;

	uint2dec(systable->hdr.revision >> 16, &buf);
	*buf++ = '.';
	uint2dec(systable->hdr.revision & 0xffff, &buf);

	/* Minor revision is only to be shown if non-zero */
	digit = *--buf;
	if (digit == '0') {
		*buf = 0;
	} else {
		*buf++ = '.';
		*buf = digit;
	}

	con_out->output_string(con_out, u"Running on UEFI ");
	con_out->output_string(con_out, rev);
	con_out->output_string(con_out, u"\r\n");
}

/**
 * print_config_tables() - print configuration tables
 */
static void print_config_tables(void)
{
	efi_uintn_t i;

	/* Find configuration tables */
	for (i = 0; i < systable->nr_tables; ++i) {
		if (!memcmp(&systable->tables[i].guid, &fdt_guid,
			    sizeof(efi_guid_t)))
			con_out->output_string
					(con_out, u"Have device tree\r\n");
		if (!memcmp(&systable->tables[i].guid, &acpi_guid,
			    sizeof(efi_guid_t)))
			con_out->output_string
					(con_out, u"Have ACPI 2.0 table\r\n");
		if (!memcmp(&systable->tables[i].guid, &smbios_guid,
			    sizeof(efi_guid_t)))
			con_out->output_string
					(con_out, u"Have SMBIOS table\r\n");
	}
}

/**
 * print_load_options() - print load options
 *
 * @systable:	system table
 * @con_out:	simple text output protocol
 */
static void print_load_options(struct efi_loaded_image *loaded_image)
{
	/* Output the load options */
	con_out->output_string(con_out, u"Load options: ");
	if (loaded_image->load_options_size && loaded_image->load_options)
		con_out->output_string(con_out,
				       (u16 *)loaded_image->load_options);
	else
		con_out->output_string(con_out, u"<none>");
	con_out->output_string(con_out, u"\r\n");
}

/**
 * print_device_path() - print device path
 *
 * @device_path:	device path to print
 * @dp2txt:		device path to text protocol
 */
static
efi_status_t print_device_path(struct efi_device_path *device_path,
			       struct efi_device_path_to_text_protocol *dp2txt)
{
	u16 *string;
	efi_status_t ret;

	if (!device_path) {
		con_out->output_string(con_out, u"<none>\r\n");
		return EFI_SUCCESS;
	}

	string = dp2txt->convert_device_path_to_text(device_path, true, false);
	if (!string) {
		con_out->output_string
			(con_out, u"Cannot convert device path to text\r\n");
		return EFI_OUT_OF_RESOURCES;
	}
	con_out->output_string(con_out, string);
	con_out->output_string(con_out, u"\r\n");
	ret = boottime->free_pool(string);
	if (ret != EFI_SUCCESS) {
		con_out->output_string(con_out, u"Cannot free pool memory\r\n");
		return ret;
	}
	return EFI_SUCCESS;
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
	struct efi_loaded_image *loaded_image;
	struct efi_device_path_to_text_protocol *device_path_to_text;
	struct efi_device_path *device_path;
	efi_status_t ret;

	systable = systab;
	boottime = systable->boottime;
	con_out = systable->con_out;

	/* UEFI requires CR LF */
	con_out->output_string(con_out, u"Hello, world!\r\n");

	print_uefi_revision();
	print_config_tables();

	/* Get the loaded image protocol */
	ret = boottime->open_protocol(handle, &loaded_image_guid,
				      (void **)&loaded_image, NULL, NULL,
				      EFI_OPEN_PROTOCOL_GET_PROTOCOL);

	if (ret != EFI_SUCCESS) {
		con_out->output_string
			(con_out, u"Cannot open loaded image protocol\r\n");
		goto out;
	}
	print_load_options(loaded_image);

	/* Get the device path to text protocol */
	ret = boottime->locate_protocol(&device_path_to_text_protocol_guid,
					NULL, (void **)&device_path_to_text);
	if (ret != EFI_SUCCESS) {
		con_out->output_string
			(con_out, u"Cannot open device path to text protocol\r\n");
		goto out;
	}
	con_out->output_string(con_out, u"File path: ");
	ret = print_device_path(loaded_image->file_path, device_path_to_text);
	if (ret != EFI_SUCCESS)
		goto out;
	if (!loaded_image->device_handle) {
		con_out->output_string
			(con_out, u"Missing device handle\r\n");
		goto out;
	}
	ret = boottime->open_protocol(loaded_image->device_handle,
				      &device_path_guid,
				      (void **)&device_path, NULL, NULL,
				      EFI_OPEN_PROTOCOL_GET_PROTOCOL);
	if (ret != EFI_SUCCESS) {
		con_out->output_string
			(con_out, u"Missing device path for device handle\r\n");
		goto out;
	}
	con_out->output_string(con_out, u"Boot device: ");
	ret = print_device_path(device_path, device_path_to_text);
	if (ret != EFI_SUCCESS)
		goto out;

out:
	boottime->exit(handle, ret, 0, NULL);

	/* We should never arrive here */
	return ret;
}
