// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2023, Heinrich Schuchardt <heinrich.schuchardt@canonical.com>
 *
 * smbiosdump.efi saves the SMBIOS table as file.
 *
 * Specifying 'nocolor' as load option data suppresses colored output and
 * clearing of the screen.
 */

#include <efi_api.h>
#include <part.h>
#include <smbios.h>
#include <string.h>

#define BUFFER_SIZE 64

static struct efi_simple_text_output_protocol *cerr;
static struct efi_simple_text_output_protocol *cout;
static struct efi_simple_text_input_protocol *cin;
static struct efi_boot_services *bs;
static efi_handle_t handle;
static struct efi_system_table *systable;
static const efi_guid_t smbios_guid = SMBIOS_TABLE_GUID;
static const efi_guid_t smbios3_guid = SMBIOS3_TABLE_GUID;
static const efi_guid_t loaded_image_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
static const efi_guid_t guid_simple_file_system_protocol =
					EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
static const efi_guid_t efi_system_partition_guid = PARTITION_SYSTEM_GUID;
static bool nocolor;

/**
 * color() - set foreground color
 *
 * @color:	foreground color
 */
static void color(u8 color)
{
	if (!nocolor)
		cout->set_attribute(cout, color | EFI_BACKGROUND_BLACK);
}

/**
 * print() - print string
 *
 * @string:	text
 */
static void print(u16 *string)
{
	cout->output_string(cout, string);
}

/**
 * cls() - clear screen
 */
static void cls(void)
{
	if (nocolor)
		print(u"\r\n");
	else
		cout->clear_screen(cout);
}

/**
 * error() - print error string
 *
 * @string:	error text
 */
static void error(u16 *string)
{
	color(EFI_LIGHTRED);
	print(string);
	color(EFI_LIGHTBLUE);
}

/**
 * efi_input_yn() - get answer to yes/no question
 *
 * Return:
 * y or Y
 *     EFI_SUCCESS
 * n or N
 *     EFI_ACCESS_DENIED
 * ESC
 *     EFI_ABORTED
 */
static efi_status_t efi_input_yn(void)
{
	struct efi_input_key key = {0};
	efi_uintn_t index;
	efi_status_t ret;

	/* Drain the console input */
	ret = cin->reset(cin, true);
	for (;;) {
		ret = bs->wait_for_event(1, &cin->wait_for_key, &index);
		if (ret != EFI_SUCCESS)
			continue;
		ret = cin->read_key_stroke(cin, &key);
		if (ret != EFI_SUCCESS)
			continue;
		switch (key.scan_code) {
		case 0x17: /* Escape */
			return EFI_ABORTED;
		default:
			break;
		}
		/* Convert to lower case */
		switch (key.unicode_char | 0x20) {
		case 'y':
			return EFI_SUCCESS;
		case 'n':
			return EFI_ACCESS_DENIED;
		default:
			break;
		}
	}
}

/**
 * efi_input() - read string from console
 *
 * @buffer:		input buffer
 * @buffer_size:	buffer size
 * Return:		status code
 */
static efi_status_t efi_input(u16 *buffer, efi_uintn_t buffer_size)
{
	struct efi_input_key key = {0};
	efi_uintn_t index;
	efi_uintn_t pos = 0;
	u16 outbuf[2] = u" ";
	efi_status_t ret;

	/* Drain the console input */
	ret = cin->reset(cin, true);
	*buffer = 0;
	for (;;) {
		ret = bs->wait_for_event(1, &cin->wait_for_key, &index);
		if (ret != EFI_SUCCESS)
			continue;
		ret = cin->read_key_stroke(cin, &key);
		if (ret != EFI_SUCCESS)
			continue;
		switch (key.scan_code) {
		case 0x17: /* Escape */
			print(u"\r\nAborted\r\n");
			return EFI_ABORTED;
		default:
			break;
		}
		switch (key.unicode_char) {
		case 0x08: /* Backspace */
			if (pos) {
				buffer[pos--] = 0;
				print(u"\b \b");
			}
			break;
		case 0x0a: /* Linefeed */
		case 0x0d: /* Carriage return */
			print(u"\r\n");
			return EFI_SUCCESS;
		default:
			break;
		}
		/* Ignore surrogate codes */
		if (key.unicode_char >= 0xD800 && key.unicode_char <= 0xDBFF)
			continue;
		if (key.unicode_char >= 0x20 &&
		    pos < buffer_size - 1) {
			*outbuf = key.unicode_char;
			buffer[pos++] = key.unicode_char;
			buffer[pos] = 0;
			print(outbuf);
		}
	}
}

/**
 * skip_whitespace() - skip over leading whitespace
 *
 * @pos:	UTF-16 string
 * Return:	pointer to first non-whitespace
 */
static u16 *skip_whitespace(u16 *pos)
{
	for (; *pos && *pos <= 0x20; ++pos)
		;
	return pos;
}

/**
 * starts_with() - check if @string starts with @keyword
 *
 * @string:	string to search for keyword
 * @keyword:	keyword to be searched
 * Return:	true fi @string starts with the keyword
 */
static bool starts_with(u16 *string, u16 *keyword)
{
	if (!string || !keyword)
		return NULL;

	for (; *keyword; ++string, ++keyword) {
		if (*string != *keyword)
			return false;
	}
	return true;
}

/**
 * open_file_system() - open simple file system protocol
 *
 * file_system:	interface of the simple file system protocol
 * Return:	status code
 */
static efi_status_t
open_file_system(struct efi_simple_file_system_protocol **file_system)
{
	struct efi_loaded_image *loaded_image;
	efi_status_t ret;
	efi_handle_t *handle_buffer = NULL;
	efi_uintn_t count;

	ret = bs->open_protocol(handle, &loaded_image_guid,
				(void **)&loaded_image, NULL, NULL,
				EFI_OPEN_PROTOCOL_GET_PROTOCOL);
	if (ret != EFI_SUCCESS) {
		error(u"Loaded image protocol not found\r\n");
		return ret;
	}

	/* Open the simple file system protocol on the same partition */
	ret = bs->open_protocol(loaded_image->device_handle,
				&guid_simple_file_system_protocol,
				(void **)file_system, NULL, NULL,
				EFI_OPEN_PROTOCOL_GET_PROTOCOL);
	if (ret == EFI_SUCCESS)
		return ret;

	/* Open the simple file system protocol on the UEFI system partition */
	ret = bs->locate_handle_buffer(BY_PROTOCOL, &efi_system_partition_guid,
				       NULL, &count, &handle_buffer);
	if (ret == EFI_SUCCESS && handle_buffer)
		ret = bs->open_protocol(handle_buffer[0],
					&guid_simple_file_system_protocol,
					(void **)file_system, NULL, NULL,
					EFI_OPEN_PROTOCOL_GET_PROTOCOL);
	if (ret != EFI_SUCCESS)
		error(u"Failed to open simple file system protocol\r\n");
	if (handle)
		bs->free_pool(handle_buffer);

	return ret;
}

/**
 * do_help() - print help
 */
static void do_help(void)
{
	error(u"check       - check SMBIOS table\r\n");
	error(u"save <file> - save SMBIOS table to file\r\n");
	error(u"exit        - exit the shell\r\n");
}

/**
 * get_config_table() - get configuration table
 *
 * @guid:	GUID of the configuration table
 * Return:	pointer to configuration table or NULL
 */
static void *get_config_table(const efi_guid_t *guid)
{
	size_t i;

	for (i = 0; i < systable->nr_tables; ++i) {
		if (!memcmp(guid, &systable->tables[i].guid, 16))
			return systable->tables[i].table;
	}

	return NULL;
}

/**
 * checksum() - calculate checksum
 *
 * @buf:	buffer to checksum
 * @len:	length of buffer
 * Return:	checksum
 */
u8 checksum(void *buf, int len)
{
	u8 ret = 0;

	for (u8 *ptr = buf; len; --len, ++ptr)
		ret -= *ptr;

	return ret;
}

/**
 * do_check() - check SMBIOS table
 *
 * Return:	status code
 */
efi_status_t do_check(void)
{
	struct smbios3_entry *smbios3_anchor;
	void *table, *table_end;
	u32 len;

	smbios3_anchor = get_config_table(&smbios3_guid);
	if (smbios3_anchor) {
		int r;

		r = memcmp(smbios3_anchor->anchor, "_SM3_", 5);
		if (r) {
			error(u"Invalid anchor string\n");
			return EFI_LOAD_ERROR;
		}
		print(u"Found SMBIOS 3 entry point\n");
		if (smbios3_anchor->length != 0x18) {
			error(u"Invalid anchor length\n");
			return EFI_LOAD_ERROR;
		}
		if (checksum(smbios3_anchor, smbios3_anchor->length)) {
			error(u"Invalid anchor checksum\n");
			return EFI_LOAD_ERROR;
		}
		table = (void *)(uintptr_t)smbios3_anchor->struct_table_address;
		len = smbios3_anchor->table_maximum_size;
	} else {
		struct smbios_entry *smbios_anchor;
		int r;

		smbios_anchor = get_config_table(&smbios_guid);
		if (!smbios_anchor) {
			error(u"No SMBIOS table\n");
			return EFI_NOT_FOUND;
		}
		r = memcmp(smbios_anchor->anchor, "_SM_", 4);
		if (r) {
			error(u"Invalid anchor string\n");
			return EFI_LOAD_ERROR;
		}
		print(u"Found SMBIOS 2.1 entry point\n");
		if (smbios_anchor->length != 0x1f) {
			error(u"Invalid anchor length\n");
			return EFI_LOAD_ERROR;
		}
		if (checksum(smbios_anchor, smbios_anchor->length)) {
			error(u"Invalid anchor checksum\n");
			return EFI_LOAD_ERROR;
		}
		r = memcmp(smbios_anchor->intermediate_anchor, "_DMI_", 5);
		if (r) {
			error(u"Invalid intermediate anchor string\n");
			return EFI_LOAD_ERROR;
		}
		if (checksum(&smbios_anchor->intermediate_anchor, 0xf)) {
			error(u"Invalid intermediate anchor checksum\n");
			return EFI_LOAD_ERROR;
		}
		table = (void *)(uintptr_t)smbios_anchor->struct_table_address;
		len = smbios_anchor->struct_table_length;
	}

	table_end = (void *)((u8 *)table + len);
	for (struct smbios_header *pos = table; ;) {
		u8 *str = (u8 *)pos + pos->length;

		if (!*str)
			++str;
		while (*str) {
			for (; *str; ++str) {
				if ((void *)str >= table_end) {
					error(u"Structure table length exceeded\n");
					return EFI_LOAD_ERROR;
				}
			}
			++str;
		}
		++str;
		if ((void *)str > table_end) {
			error(u"Structure table length exceeded\n");
			return EFI_LOAD_ERROR;
		}
		if (pos->type == 0x7f) /* End of table */
			break;
		pos = (struct smbios_header *)str;
	}

	return EFI_SUCCESS;
}

/**
 * save_file() - save file to EFI system partition
 *
 * @filename:	file name
 * @buf:	buffer to write
 * @size:	size of the buffer
 */
efi_status_t save_file(u16 *filename, void *buf, efi_uintn_t size)
{
	efi_uintn_t ret;
	struct efi_simple_file_system_protocol *file_system;
	struct efi_file_handle *root, *file;

	ret = open_file_system(&file_system);
	if (ret != EFI_SUCCESS)
		return ret;

	/* Open volume */
	ret = file_system->open_volume(file_system, &root);
	if (ret != EFI_SUCCESS) {
		error(u"Failed to open volume\r\n");
		return ret;
	}
	/* Check if file already exists */
	ret = root->open(root, &file, filename, EFI_FILE_MODE_READ, 0);
	if (ret == EFI_SUCCESS) {
		file->close(file);
		print(u"Overwrite existing file (y/n)? ");
		ret = efi_input_yn();
		print(u"\r\n");
		if (ret != EFI_SUCCESS) {
			root->close(root);
			error(u"Aborted by user\r\n");
			bs->free_pool(buf);
			return ret;
		}
	}

	/* Create file */
	ret = root->open(root, &file, filename,
			 EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE |
			 EFI_FILE_MODE_CREATE, EFI_FILE_ARCHIVE);
	if (ret == EFI_SUCCESS) {
		/* Write file */
		ret = file->write(file, &size, buf);
		if (ret != EFI_SUCCESS)
			error(u"Failed to write file\r\n");
		file->close(file);
	} else {
		error(u"Failed to open file\r\n");
	}
	root->close(root);

	return ret;
}

/**
 * do_save() - save SMBIOS table
 *
 * @filename:	file name
 * Return:	status code
 */
static efi_status_t do_save(u16 *filename)
{
	struct smbios3_entry *smbios3_anchor;
	u8 *buf;
	efi_uintn_t size;
	efi_uintn_t ret;

	ret = do_check();
	if (ret != EFI_SUCCESS)
		return ret;

	smbios3_anchor = get_config_table(&smbios3_guid);
	if (smbios3_anchor) {
		size = 0x20 + smbios3_anchor->table_maximum_size;
		ret = bs->allocate_pool(EFI_LOADER_DATA, size, (void **)&buf);
		if (ret != EFI_SUCCESS) {
			error(u"Out of memory\n");
			return ret;
		}

		memset(buf, 0, size);
		memcpy(buf, smbios3_anchor, smbios3_anchor->length);
		memcpy(buf + 0x20,
		       (void *)(uintptr_t)smbios3_anchor->struct_table_address,
		       smbios3_anchor->table_maximum_size);

		smbios3_anchor = (struct smbios3_entry *)buf;
		smbios3_anchor->struct_table_address = 0x20;
		smbios3_anchor->checksum +=
			checksum(smbios3_anchor, smbios3_anchor->length);
	} else {
		struct smbios_entry *smbios_anchor;

		smbios_anchor = get_config_table(&smbios_guid);
		if (!smbios_anchor) {
			/* Should not be reached after successful do_check() */
			error(u"No SMBIOS table\n");
			return EFI_NOT_FOUND;
		}

		size = 0x20 + smbios_anchor->struct_table_length;

		ret = bs->allocate_pool(EFI_LOADER_DATA, size, (void **)&buf);
		if (ret != EFI_SUCCESS) {
			error(u"Out of memory\n");
			return ret;
		}

		memset(buf, 0, size);
		memcpy(buf, smbios_anchor, smbios_anchor->length);
		memcpy(buf + 0x20,
		       (void *)(uintptr_t)smbios_anchor->struct_table_address,
		       smbios_anchor->struct_table_length);

		smbios_anchor = (struct smbios_entry *)buf;
		smbios_anchor->struct_table_address = 0x20;
		smbios_anchor->intermediate_checksum +=
			checksum(&smbios_anchor->intermediate_anchor, 0xf);
		smbios_anchor->checksum +=
			checksum(smbios_anchor, smbios_anchor->length);
	}

	filename = skip_whitespace(filename);

	ret = save_file(filename, buf, size);

	if (ret == EFI_SUCCESS) {
		print(filename);
		print(u" written\r\n");
	}

	bs->free_pool(buf);

	return ret;
}

/**
 * get_load_options() - get load options
 *
 * Return:	load options or NULL
 */
static u16 *get_load_options(void)
{
	efi_status_t ret;
	struct efi_loaded_image *loaded_image;

	ret = bs->open_protocol(handle, &loaded_image_guid,
				(void **)&loaded_image, NULL, NULL,
				EFI_OPEN_PROTOCOL_GET_PROTOCOL);
	if (ret != EFI_SUCCESS) {
		error(u"Loaded image protocol not found\r\n");
		return NULL;
	}

	if (!loaded_image->load_options_size || !loaded_image->load_options)
		return NULL;

	return loaded_image->load_options;
}

/**
 * command_loop - process user commands
 */
static void command_loop(void)
{
	for (;;) {
		u16 command[BUFFER_SIZE];
		u16 *pos;
		efi_uintn_t ret;

		print(u"=> ");
		ret = efi_input(command, sizeof(command));
		if (ret == EFI_ABORTED)
			break;
		pos = skip_whitespace(command);
		if (starts_with(pos, u"exit")) {
			break;
		} else if (starts_with(pos, u"check")) {
			ret = do_check();
			if (ret == EFI_SUCCESS)
				print(u"OK\n");
		} else if (starts_with(pos, u"save ")) {
			do_save(pos + 5);
		} else {
			do_help();
		}
	}
}

/**
 * efi_main() - entry point of the EFI application.
 *
 * @handle:	handle of the loaded image
 * @systab:	system table
 * Return:	status code
 */
efi_status_t EFIAPI efi_main(efi_handle_t image_handle,
			     struct efi_system_table *systab)
{
	u16 *load_options;

	handle = image_handle;
	systable = systab;
	cerr = systable->std_err;
	cout = systable->con_out;
	cin = systable->con_in;
	bs = systable->boottime;
	load_options = get_load_options();

	if (starts_with(load_options, u"nocolor"))
		nocolor = true;

	color(EFI_WHITE);
	cls();
	print(u"SMBIOS Dump\r\n===========\r\n\r\n");
	color(EFI_LIGHTBLUE);

	command_loop();

	color(EFI_LIGHTGRAY);
	cls();

	return EFI_SUCCESS;
}
