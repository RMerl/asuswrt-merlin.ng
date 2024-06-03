// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 The Chromium OS Authors.
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <asm/unaligned.h>
#include <linux/string.h>
#include <tpm-common.h>
#include "tpm-user-utils.h"

/**
 * Print a byte string in hexdecimal format, 16-bytes per line.
 *
 * @param data		byte string to be printed
 * @param count		number of bytes to be printed
 */
void print_byte_string(u8 *data, size_t count)
{
	int i, print_newline = 0;

	for (i = 0; i < count; i++) {
		printf(" %02x", data[i]);
		print_newline = (i % 16 == 15);
		if (print_newline)
			putc('\n');
	}
	/* Avoid duplicated newline at the end */
	if (!print_newline)
		putc('\n');
}

/**
 * Convert a text string of hexdecimal values into a byte string.
 *
 * @param bytes		text string of hexdecimal values with no space
 *			between them
 * @param data		output buffer for byte string.  The caller has to make
 *			sure it is large enough for storing the output.  If
 *			NULL is passed, a large enough buffer will be allocated,
 *			and the caller must free it.
 * @param count_ptr	output variable for the length of byte string
 * @return pointer to output buffer
 */
void *parse_byte_string(char *bytes, u8 *data, size_t *count_ptr)
{
	char byte[3];
	size_t count, length;
	int i;

	if (!bytes)
		return NULL;
	length = strlen(bytes);
	count = length / 2;

	if (!data)
		data = malloc(count);
	if (!data)
		return NULL;

	byte[2] = '\0';
	for (i = 0; i < length; i += 2) {
		byte[0] = bytes[i];
		byte[1] = bytes[i + 1];
		data[i / 2] = (u8)simple_strtoul(byte, NULL, 16);
	}

	if (count_ptr)
		*count_ptr = count;

	return data;
}

/**
 * report_return_code() - Report any error and return failure or success
 *
 * @param return_code	TPM command return code
 * @return value of enum command_ret_t
 */
int report_return_code(int return_code)
{
	if (return_code) {
		printf("Error: %d\n", return_code);
		return CMD_RET_FAILURE;
	} else {
		return CMD_RET_SUCCESS;
	}
}

/**
 * Return number of values defined by a type string.
 *
 * @param type_str	type string
 * @return number of values of type string
 */
int type_string_get_num_values(const char *type_str)
{
	return strlen(type_str);
}

/**
 * Return total size of values defined by a type string.
 *
 * @param type_str	type string
 * @return total size of values of type string, or 0 if type string
 *  contains illegal type character.
 */
size_t type_string_get_space_size(const char *type_str)
{
	size_t size;

	for (size = 0; *type_str; type_str++) {
		switch (*type_str) {
		case 'b':
			size += 1;
			break;
		case 'w':
			size += 2;
			break;
		case 'd':
			size += 4;
			break;
		default:
			return 0;
		}
	}

	return size;
}

/**
 * Allocate a buffer large enough to hold values defined by a type
 * string.  The caller has to free the buffer.
 *
 * @param type_str	type string
 * @param count		pointer for storing size of buffer
 * @return pointer to buffer or NULL on error
 */
void *type_string_alloc(const char *type_str, u32 *count)
{
	void *data;
	size_t size;

	size = type_string_get_space_size(type_str);
	if (!size)
		return NULL;
	data = malloc(size);
	if (data)
		*count = size;

	return data;
}

/**
 * Pack values defined by a type string into a buffer.  The buffer must have
 * large enough space.
 *
 * @param type_str	type string
 * @param values	text strings of values to be packed
 * @param data		output buffer of values
 * @return 0 on success, non-0 on error
 */
int type_string_pack(const char *type_str, char * const values[],
		     u8 *data)
{
	size_t offset;
	u32 value;

	for (offset = 0; *type_str; type_str++, values++) {
		value = simple_strtoul(values[0], NULL, 0);
		switch (*type_str) {
		case 'b':
			data[offset] = value;
			offset += 1;
			break;
		case 'w':
			put_unaligned_be16(value, data + offset);
			offset += 2;
			break;
		case 'd':
			put_unaligned_be32(value, data + offset);
			offset += 4;
			break;
		default:
			return -1;
		}
	}

	return 0;
}

/**
 * Read values defined by a type string from a buffer, and write these values
 * to environment variables.
 *
 * @param type_str	type string
 * @param data		input buffer of values
 * @param vars		names of environment variables
 * @return 0 on success, non-0 on error
 */
int type_string_write_vars(const char *type_str, u8 *data,
			   char * const vars[])
{
	size_t offset;
	u32 value;

	for (offset = 0; *type_str; type_str++, vars++) {
		switch (*type_str) {
		case 'b':
			value = data[offset];
			offset += 1;
			break;
		case 'w':
			value = get_unaligned_be16(data + offset);
			offset += 2;
			break;
		case 'd':
			value = get_unaligned_be32(data + offset);
			offset += 4;
			break;
		default:
			return -1;
		}
		if (env_set_ulong(*vars, value))
			return -1;
	}

	return 0;
}

int get_tpm(struct udevice **devp)
{
	int rc;

	rc = uclass_first_device_err(UCLASS_TPM, devp);
	if (rc) {
		printf("Could not find TPM (ret=%d)\n", rc);
		return CMD_RET_FAILURE;
	}

	return 0;
}

int do_tpm_info(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	struct udevice *dev;
	char buf[80];
	int rc;

	rc = get_tpm(&dev);
	if (rc)
		return rc;
	rc = tpm_get_desc(dev, buf, sizeof(buf));
	if (rc < 0) {
		printf("Couldn't get TPM info (%d)\n", rc);
		return CMD_RET_FAILURE;
	}
	printf("%s\n", buf);

	return 0;
}

int do_tpm_init(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct udevice *dev;
	int rc;

	if (argc != 1)
		return CMD_RET_USAGE;
	rc = get_tpm(&dev);
	if (rc)
		return rc;

	return report_return_code(tpm_init(dev));
}

int do_tpm(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	cmd_tbl_t *tpm_commands, *cmd;
	struct tpm_chip_priv *priv;
	struct udevice *dev;
	unsigned int size;
	int ret;

	if (argc < 2)
		return CMD_RET_USAGE;

	ret = get_tpm(&dev);
	if (ret)
		return ret;

	priv = dev_get_uclass_priv(dev);

	/* Below getters return NULL if the desired stack is not built */
	switch (priv->version) {
	case TPM_V1:
		tpm_commands = get_tpm1_commands(&size);
		break;
	case TPM_V2:
		tpm_commands = get_tpm2_commands(&size);
		break;
	default:
		tpm_commands = NULL;
	}

	if (!tpm_commands)
		return CMD_RET_USAGE;

	cmd = find_cmd_tbl(argv[1], tpm_commands, size);
	if (!cmd)
		return CMD_RET_USAGE;

	return cmd->cmd(cmdtp, flag, argc - 1, argv + 1);
}
