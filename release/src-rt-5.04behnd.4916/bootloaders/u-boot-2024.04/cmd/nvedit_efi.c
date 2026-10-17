// SPDX-License-Identifier: GPL-2.0+
/*
 *  Integrate UEFI variables to u-boot env interface
 *
 *  Copyright (c) 2018 AKASHI Takahiro, Linaro Limited
 */

#include <charset.h>
#include <common.h>
#include <command.h>
#include <efi_loader.h>
#include <efi_variable.h>
#include <env.h>
#include <exports.h>
#include <hexdump.h>
#include <malloc.h>
#include <mapmem.h>
#include <rtc.h>
#include <uuid.h>
#include <linux/kernel.h>

/*
 * From efi_variable.c,
 *
 * Mapping between UEFI variables and u-boot variables:
 *
 *   efi_$guid_$varname = {attributes}(type)value
 */

static const struct {
	u32 mask;
	char *text;
} efi_var_attrs[] = {
	{EFI_VARIABLE_NON_VOLATILE, "NV"},
	{EFI_VARIABLE_BOOTSERVICE_ACCESS, "BS"},
	{EFI_VARIABLE_RUNTIME_ACCESS, "RT"},
	{EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS, "AW"},
	{EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS, "AT"},
	{EFI_VARIABLE_READ_ONLY, "RO"},
};

/**
 * efi_dump_single_var() - show information about a UEFI variable
 *
 * @name:	Name of the variable
 * @guid:	Vendor GUID
 * @verbose:	if true, dump data
 *
 * Show information encoded in one UEFI variable
 */
static void efi_dump_single_var(u16 *name, const efi_guid_t *guid, bool verbose)
{
	u32 attributes;
	u8 *data;
	u64 time;
	struct rtc_time tm;
	efi_uintn_t size;
	int count, i;
	efi_status_t ret;

	data = NULL;
	size = 0;
	ret = efi_get_variable_int(name, guid, &attributes, &size, data, &time);
	if (ret == EFI_BUFFER_TOO_SMALL) {
		data = malloc(size);
		if (!data)
			goto out;

		ret = efi_get_variable_int(name, guid, &attributes, &size,
					   data, &time);
	}
	if (ret == EFI_NOT_FOUND) {
		printf("Error: \"%ls\" not defined\n", name);
		goto out;
	}
	if (ret != EFI_SUCCESS)
		goto out;

	rtc_to_tm(time, &tm);
	printf("%ls:\n    %pUl (%pUs)\n", name, guid, guid);
	if (attributes & EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS)
		printf("    %04d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year,
		       tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	printf("    ");
	for (count = 0, i = 0; i < ARRAY_SIZE(efi_var_attrs); i++)
		if (attributes & efi_var_attrs[i].mask) {
			if (count)
				putc('|');
			count++;
			puts(efi_var_attrs[i].text);
		}
	printf(", DataSize = 0x%zx\n", size);
	if (verbose)
		print_hex_dump("    ", DUMP_PREFIX_OFFSET, 16, 1,
			       data, size, true);

out:
	free(data);
}

static bool match_name(int argc, char *const argv[], u16 *var_name16)
{
	char *buf, *p;
	size_t buflen;
	int i;
	bool result = false;

	buflen = utf16_utf8_strlen(var_name16) + 1;
	buf = calloc(1, buflen);
	if (!buf)
		return result;

	p = buf;
	utf16_utf8_strcpy(&p, var_name16);

	for (i = 0; i < argc; argc--, argv++) {
		if (!strcmp(buf, argv[i])) {
			result = true;
			goto out;
		}
	}

out:
	free(buf);

	return result;
}

/**
 * efi_dump_var_all() - show information about all the UEFI variables
 *
 * @argc:	Number of arguments (variables)
 * @argv:	Argument (variable name) array
 * @verbose:	if true, dump data
 * Return:	CMD_RET_SUCCESS on success, or CMD_RET_RET_FAILURE
 *
 * Show information encoded in all the UEFI variables
 */
static int efi_dump_var_all(int argc,  char *const argv[],
			    const efi_guid_t *guid_p, bool verbose)
{
	u16 *var_name16, *p;
	efi_uintn_t buf_size, size;
	efi_guid_t guid;
	efi_status_t ret;
	bool match = false;

	buf_size = 128;
	var_name16 = malloc(buf_size);
	if (!var_name16)
		return CMD_RET_FAILURE;

	var_name16[0] = 0;
	for (;;) {
		size = buf_size;
		ret = efi_get_next_variable_name_int(&size, var_name16,
						     &guid);
		if (ret == EFI_NOT_FOUND)
			break;
		if (ret == EFI_BUFFER_TOO_SMALL) {
			buf_size = size;
			p = realloc(var_name16, buf_size);
			if (!p) {
				free(var_name16);
				return CMD_RET_FAILURE;
			}
			var_name16 = p;
			ret = efi_get_next_variable_name_int(&size, var_name16,
							     &guid);
		}
		if (ret != EFI_SUCCESS) {
			free(var_name16);
			return CMD_RET_FAILURE;
		}

		if (guid_p && guidcmp(guid_p, &guid))
			continue;
		if (!argc || match_name(argc, argv, var_name16)) {
			match = true;
			efi_dump_single_var(var_name16, &guid, verbose);
		}
	}
	free(var_name16);

	if (!match && argc == 1) {
		printf("Error: \"%s\" not defined\n", argv[0]);
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

/**
 * do_env_print_efi() - show information about UEFI variables
 *
 * @cmdtp:	Command table
 * @flag:	Command flag
 * @argc:	Number of arguments
 * @argv:	Argument array
 * Return:	CMD_RET_SUCCESS on success, or CMD_RET_RET_FAILURE
 *
 * This function is for "env print -e" or "printenv -e" command:
 *   => env print -e [-n] [-guid <guid> | -all] [var [...]]
 * If one or more variable names are specified, show information
 * named UEFI variables, otherwise show all the UEFI variables.
 */
int do_env_print_efi(struct cmd_tbl *cmdtp, int flag, int argc,
		     char *const argv[])
{
	const efi_guid_t *guid_p = NULL;
	efi_guid_t guid;
	bool verbose = true;
	efi_status_t ret;

	/* Initialize EFI drivers */
	ret = efi_init_obj_list();
	if (ret != EFI_SUCCESS) {
		printf("Error: Cannot initialize UEFI sub-system, r = %lu\n",
		       ret & ~EFI_ERROR_MASK);
		return CMD_RET_FAILURE;
	}

	for (argc--, argv++; argc > 0 && argv[0][0] == '-'; argc--, argv++) {
		if (!strcmp(argv[0], "-guid")) {
			if (argc == 1)
				return CMD_RET_USAGE;
			argc--;
			argv++;
			if (uuid_str_to_bin(argv[0], guid.b,
					    UUID_STR_FORMAT_GUID))
				return CMD_RET_USAGE;
			guid_p = (const efi_guid_t *)guid.b;
		} else if (!strcmp(argv[0], "-n")) {
			verbose = false;
		} else {
			return CMD_RET_USAGE;
		}
	}

	/* enumerate and show all UEFI variables */
	return efi_dump_var_all(argc, argv, guid_p, verbose);
}

/**
 * append_value() - encode UEFI variable's value
 * @bufp:	Buffer of encoded UEFI variable's value
 * @sizep:	Size of buffer
 * @data:	data to be encoded into the value
 * Return:	0 on success, -1 otherwise
 *
 * Interpret a given data string and append it to buffer.
 * Buffer will be realloc'ed if necessary.
 *
 * Currently supported formats are:
 *   =0x0123...:		Hexadecimal number
 *   =H0123...:			Hexadecimal-byte array
 *   ="...", =S"..." or <string>:
 *				String
 */
static int append_value(char **bufp, size_t *sizep, char *data)
{
	char *tmp_buf = NULL, *new_buf = NULL, *value;
	unsigned long len = 0;

	if (!strncmp(data, "=0x", 3)) { /* hexadecimal number */
		union {
			u8 u8;
			u16 u16;
			u32 u32;
			u64 u64;
		} tmp_data;
		unsigned long hex_value;
		void *hex_ptr;

		data += 3;
		len = strlen(data);
		if ((len & 0x1)) /* not multiple of two */
			return -1;

		len /= 2;
		if (len > 8)
			return -1;
		else if (len > 4)
			len = 8;
		else if (len > 2)
			len = 4;

		/* convert hex hexadecimal number */
		if (strict_strtoul(data, 16, &hex_value) < 0)
			return -1;

		tmp_buf = malloc(len);
		if (!tmp_buf)
			return -1;

		if (len == 1) {
			tmp_data.u8 = hex_value;
			hex_ptr = &tmp_data.u8;
		} else if (len == 2) {
			tmp_data.u16 = hex_value;
			hex_ptr = &tmp_data.u16;
		} else if (len == 4) {
			tmp_data.u32 = hex_value;
			hex_ptr = &tmp_data.u32;
		} else {
			tmp_data.u64 = hex_value;
			hex_ptr = &tmp_data.u64;
		}
		memcpy(tmp_buf, hex_ptr, len);
		value = tmp_buf;

	} else if (!strncmp(data, "=H", 2)) { /* hexadecimal-byte array */
		data += 2;
		len = strlen(data);
		if (len & 0x1) /* not multiple of two */
			return -1;

		len /= 2;
		tmp_buf = malloc(len);
		if (!tmp_buf)
			return -1;

		if (hex2bin((u8 *)tmp_buf, data, len) < 0) {
			printf("Error: illegal hexadecimal string\n");
			free(tmp_buf);
			return -1;
		}

		value = tmp_buf;
	} else { /* string */
		if (!strncmp(data, "=\"", 2) || !strncmp(data, "=S\"", 3)) {
			if (data[1] == '"')
				data += 2;
			else
				data += 3;
			value = data;
			len = strlen(data) - 1;
			if (data[len] != '"')
				return -1;
		} else {
			value = data;
			len = strlen(data);
		}
	}

	new_buf = realloc(*bufp, *sizep + len);
	if (!new_buf)
		goto out;

	memcpy(new_buf + *sizep, value, len);
	*bufp = new_buf;
	*sizep += len;

out:
	free(tmp_buf);

	return 0;
}

/**
 * do_env_set_efi() - set UEFI variable
 *
 * @cmdtp:	Command table
 * @flag:	Command flag
 * @argc:	Number of arguments
 * @argv:	Argument array
 * Return:	CMD_RET_SUCCESS on success, or CMD_RET_RET_FAILURE
 *
 * This function is for "env set -e" or "setenv -e" command:
 *   => env set -e [-guid guid][-nv][-bs][-rt][-at][-a][-v]
 *		   [-i address,size] var, or
 *                 var [value ...]
 * Encode values specified and set given UEFI variable.
 * If no value is specified, delete the variable.
 */
int do_env_set_efi(struct cmd_tbl *cmdtp, int flag, int argc,
		   char *const argv[])
{
	char *var_name, *value, *ep;
	ulong addr;
	efi_uintn_t size;
	efi_guid_t guid;
	u32 attributes;
	bool default_guid, verbose, value_on_memory;
	u16 *var_name16;
	efi_status_t ret;

	if (argc == 1)
		return CMD_RET_USAGE;

	/* Initialize EFI drivers */
	ret = efi_init_obj_list();
	if (ret != EFI_SUCCESS) {
		printf("Error: Cannot initialize UEFI sub-system, r = %lu\n",
		       ret & ~EFI_ERROR_MASK);
		return CMD_RET_FAILURE;
	}

	/*
	 * attributes = EFI_VARIABLE_BOOTSERVICE_ACCESS |
	 *	     EFI_VARIABLE_RUNTIME_ACCESS;
	 */
	value = NULL;
	size = 0;
	attributes = 0;
	guid = efi_global_variable_guid;
	default_guid = true;
	verbose = false;
	value_on_memory = false;
	for (argc--, argv++; argc > 0 && argv[0][0] == '-'; argc--, argv++) {
		if (!strcmp(argv[0], "-guid")) {
			if (argc == 1)
				return CMD_RET_USAGE;

			argc--;
			argv++;
			if (uuid_str_to_bin(argv[0], guid.b,
					    UUID_STR_FORMAT_GUID)) {
				return CMD_RET_USAGE;
			}
			default_guid = false;
		} else if (!strcmp(argv[0], "-bs")) {
			attributes |= EFI_VARIABLE_BOOTSERVICE_ACCESS;
		} else if (!strcmp(argv[0], "-rt")) {
			attributes |= EFI_VARIABLE_RUNTIME_ACCESS;
		} else if (!strcmp(argv[0], "-nv")) {
			attributes |= EFI_VARIABLE_NON_VOLATILE;
		} else if (!strcmp(argv[0], "-at")) {
			attributes |=
			  EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS;
		} else if (!strcmp(argv[0], "-a")) {
			attributes |= EFI_VARIABLE_APPEND_WRITE;
		} else if (!strcmp(argv[0], "-i")) {
			/* data comes from memory */
			if (argc == 1)
				return CMD_RET_USAGE;

			argc--;
			argv++;
			addr = hextoul(argv[0], &ep);
			if (*ep != ':')
				return CMD_RET_USAGE;

			/* 0 should be allowed for delete */
			size = hextoul(++ep, NULL);

			value_on_memory = true;
		} else if (!strcmp(argv[0], "-v")) {
			verbose = true;
		} else {
			return CMD_RET_USAGE;
		}
	}
	if (!argc)
		return CMD_RET_USAGE;

	var_name = argv[0];
	if (default_guid) {
		if (!strcmp(var_name, "db") || !strcmp(var_name, "dbx") ||
		    !strcmp(var_name, "dbt"))
			guid = efi_guid_image_security_database;
		else
			guid = efi_global_variable_guid;
	}

	if (verbose) {
		printf("GUID: %pUl (%pUs)\n", &guid, &guid);
		printf("Attributes: 0x%x\n", attributes);
	}

	/* for value */
	if (value_on_memory)
		value = map_sysmem(addr, 0);
	else if (argc > 1)
		for (argc--, argv++; argc > 0; argc--, argv++)
			if (append_value(&value, &size, argv[0]) < 0) {
				printf("## Failed to process an argument, %s\n",
				       argv[0]);
				ret = CMD_RET_FAILURE;
				goto out;
			}

	if (size && verbose) {
		printf("Value:\n");
		print_hex_dump("    ", DUMP_PREFIX_OFFSET,
			       16, 1, value, size, true);
	}

	var_name16 = efi_convert_string(var_name);
	if (!var_name16) {
		printf("## Out of memory\n");
		ret = CMD_RET_FAILURE;
		goto out;
	}
	ret = efi_set_variable_int(var_name16, &guid, attributes, size, value,
				   true);
	free(var_name16);
	unmap_sysmem(value);
	if (ret == EFI_SUCCESS) {
		ret = CMD_RET_SUCCESS;
	} else {
		const char *msg;

		switch (ret) {
		case EFI_NOT_FOUND:
			msg = " (not found)";
			break;
		case EFI_WRITE_PROTECTED:
			msg = " (read only)";
			break;
		case EFI_INVALID_PARAMETER:
			msg = " (invalid parameter)";
			break;
		case EFI_SECURITY_VIOLATION:
			msg = " (validation failed)";
			break;
		case EFI_OUT_OF_RESOURCES:
			msg = " (out of memory)";
			break;
		default:
			msg = "";
			break;
		}
		printf("## Failed to set EFI variable%s\n", msg);
		ret = CMD_RET_FAILURE;
	}
out:
	if (value_on_memory)
		unmap_sysmem(value);
	else
		free(value);

	return ret;
}
