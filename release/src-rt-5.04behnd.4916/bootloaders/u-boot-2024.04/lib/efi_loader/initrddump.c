// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * initrddump.efi saves the initial RAM disk provided via the
 * EFI_LOAD_FILE2_PROTOCOL.
 *
 * Specifying 'nocolor' as load option data suppresses colored output and
 * clearing of the screen.
 */

#include <efi_api.h>
#include <efi_load_initrd.h>

#define BUFFER_SIZE 64
#define ESC 0x17

#define efi_size_in_pages(size) (((size) + EFI_PAGE_MASK) >> EFI_PAGE_SHIFT)

static struct efi_system_table *systable;
static struct efi_boot_services *bs;
static struct efi_simple_text_output_protocol *cerr;
static struct efi_simple_text_output_protocol *cout;
static struct efi_simple_text_input_protocol *cin;
static const efi_guid_t loaded_image_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
static const efi_guid_t guid_simple_file_system_protocol =
					EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
static const efi_guid_t load_file2_guid = EFI_LOAD_FILE2_PROTOCOL_GUID;
static efi_handle_t handle;
static bool nocolor;

/*
 * Device path defined by Linux to identify the handle providing the
 * EFI_LOAD_FILE2_PROTOCOL used for loading the initial ramdisk.
 */
static const struct efi_initrd_dp initrd_dp = {
	.vendor = {
		{
		   DEVICE_PATH_TYPE_MEDIA_DEVICE,
		   DEVICE_PATH_SUB_TYPE_VENDOR_PATH,
		   sizeof(initrd_dp.vendor),
		},
		EFI_INITRD_MEDIA_GUID,
	},
	.end = {
		DEVICE_PATH_TYPE_END,
		DEVICE_PATH_SUB_TYPE_END,
		sizeof(initrd_dp.end),
	}
};

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

/*
 * printx() - print hexadecimal number
 *
 * @val:	value to print;
 * @prec:	minimum number of digits to print
 */
static void printx(u64 val, u32 prec)
{
	int i;
	u16 c;
	u16 buf[16];
	u16 *pos = buf;

	for (i = 2 * sizeof(val) - 1; i >= 0; --i) {
		c = (val >> (4 * i)) & 0x0f;
		if (c || pos != buf || !i || i < prec) {
			c += '0';
			if (c > '9')
				c += 'a' - '9' - 1;
			*pos++ = c;
		}
	}
	*pos = 0;
	print(buf);
}

/**
 * efi_drain_input() - drain console input
 */
static void efi_drain_input(void)
{
	cin->reset(cin, true);
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
 * Return:	true if @string starts with the keyword
 */
static bool starts_with(u16 *string, u16 *keyword)
{
	if (!string || !keyword)
		return false;

	for (; *keyword; ++string, ++keyword) {
		if (*string != *keyword)
			return false;
	}
	return true;
}

/**
 * do_help() - print help
 */
static void do_help(void)
{
	error(u"load          - show length and CRC32 of initial RAM disk\r\n");
	error(u"save <initrd> - save initial RAM disk to file\r\n");
	error(u"exit          - exit the shell\r\n");
}

/**
 * get_initrd() - read initial RAM disk via EFI_LOAD_FILE2_PROTOCOL
 *
 * @initrd:		on return buffer with initial RAM disk
 * @initrd_size:	size of initial RAM disk
 * Return:		status code
 */
static efi_status_t get_initrd(void **initrd, efi_uintn_t *initrd_size)
{
	struct efi_device_path *dp = (struct efi_device_path *)&initrd_dp;
	struct efi_load_file_protocol *load_file2_prot;
	u64 buffer;
	efi_handle_t handle;
	efi_status_t ret;

	*initrd = NULL;
	*initrd_size = 0;
	ret = bs->locate_device_path(&load_file2_guid, &dp, &handle);
	if (ret != EFI_SUCCESS) {
		error(u"Load File2 protocol not found\r\n");
		return ret;
	}
	ret = bs->open_protocol(handle, &load_file2_guid,
				(void **)&load_file2_prot, NULL, NULL,
				EFI_OPEN_PROTOCOL_GET_PROTOCOL);
	ret = load_file2_prot->load_file(load_file2_prot, dp, false,
					 initrd_size, NULL);
	if (ret != EFI_BUFFER_TOO_SMALL) {
		error(u"Load File2 protocol does not provide file length\r\n");
		return EFI_LOAD_ERROR;
	}
	ret = bs->allocate_pages(EFI_ALLOCATE_ANY_PAGES, EFI_LOADER_DATA,
				 efi_size_in_pages(*initrd_size), &buffer);
	if (ret != EFI_SUCCESS) {
		error(u"Out of memory\r\n");
		return ret;
	}
	*initrd = (void *)(uintptr_t)buffer;
	ret = load_file2_prot->load_file(load_file2_prot, dp, false,
					 initrd_size, *initrd);
	if (ret != EFI_SUCCESS) {
		error(u"Load File2 protocol failed to provide file\r\n");
		bs->free_pages(buffer, efi_size_in_pages(*initrd_size));
		return EFI_LOAD_ERROR;
	}
	return ret;
}

/**
 * do_load() - load initial RAM disk and display CRC32 and length
 *
 * @filename:	file name
 * Return:	status code
 */
static efi_status_t do_load(void)
{
	void *initrd;
	efi_uintn_t initrd_size;
	u32 crc32;
	efi_uintn_t ret;

	ret =  get_initrd(&initrd, &initrd_size);
	if (ret != EFI_SUCCESS)
		return ret;
	print(u"length: 0x");
	printx(initrd_size, 1);
	print(u"\r\n");

	ret = bs->calculate_crc32(initrd, initrd_size, &crc32);
	if (ret != EFI_SUCCESS) {
		error(u"Calculating CRC32 failed\r\n");
		return EFI_LOAD_ERROR;
	}
	print(u"crc32: 0x");
	printx(crc32, 8);
	print(u"\r\n");

	return EFI_SUCCESS;
}

/**
 * do_save() - save initial RAM disk
 *
 * @filename:	file name
 * Return:	status code
 */
static efi_status_t do_save(u16 *filename)
{
	struct efi_loaded_image *loaded_image;
	struct efi_simple_file_system_protocol *file_system;
	struct efi_file_handle *root, *file;
	void *initrd;
	efi_uintn_t initrd_size;
	efi_uintn_t ret;

	ret = get_initrd(&initrd, &initrd_size);
	if (ret != EFI_SUCCESS)
		return ret;

	filename = skip_whitespace(filename);

	ret = bs->open_protocol(handle, &loaded_image_guid,
				(void **)&loaded_image, NULL, NULL,
				EFI_OPEN_PROTOCOL_GET_PROTOCOL);
	if (ret != EFI_SUCCESS) {
		error(u"Loaded image protocol not found\r\n");
		goto out;
	}

	/* Open the simple file system protocol */
	ret = bs->open_protocol(loaded_image->device_handle,
				&guid_simple_file_system_protocol,
				(void **)&file_system, NULL, NULL,
				EFI_OPEN_PROTOCOL_GET_PROTOCOL);
	if (ret != EFI_SUCCESS) {
		error(u"Failed to open simple file system protocol\r\n");
		goto out;
	}

	/* Open volume */
	ret = file_system->open_volume(file_system, &root);
	if (ret != EFI_SUCCESS) {
		error(u"Failed to open volume\r\n");
		goto out;
	}
	/* Check if file already exists */
	ret = root->open(root, &file, filename, EFI_FILE_MODE_READ, 0);
	if (ret == EFI_SUCCESS) {
		file->close(file);
		efi_drain_input();
		print(u"Overwrite existing file (y/n)? ");
		ret = efi_input_yn();
		print(u"\r\n");
		if (ret != EFI_SUCCESS) {
			root->close(root);
			error(u"Aborted by user\r\n");
			goto out;
		}
	}

	/* Create file */
	ret = root->open(root, &file, filename,
			 EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE |
			 EFI_FILE_MODE_CREATE, EFI_FILE_ARCHIVE);
	if (ret == EFI_SUCCESS) {
		/* Write file */
		ret = file->write(file, &initrd_size, initrd);
		if (ret != EFI_SUCCESS) {
			error(u"Failed to write file\r\n");
		} else {
			print(filename);
			print(u" written\r\n");
		}
		file->close(file);
	} else {
		error(u"Failed to open file\r\n");
	}
	root->close(root);

out:
	if (initrd)
		bs->free_pages((uintptr_t)initrd,
			       efi_size_in_pages(initrd_size));
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
	print(u"INITRD Dump\r\n===========\r\n\r\n");
	color(EFI_LIGHTBLUE);

	for (;;) {
		u16 command[BUFFER_SIZE];
		u16 *pos;
		efi_uintn_t ret;

		efi_drain_input();
		print(u"=> ");
		ret = efi_input(command, sizeof(command));
		if (ret == EFI_ABORTED)
			break;
		pos = skip_whitespace(command);
		if (starts_with(pos, u"exit"))
			break;
		else if (starts_with(pos, u"load"))
			do_load();
		else if (starts_with(pos, u"save "))
			do_save(pos + 5);
		else
			do_help();
	}

	color(EFI_LIGHTGRAY);
	cls();

	return EFI_SUCCESS;
}
