// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 The Chromium OS Authors.
 * Coypright (c) 2013 Guntermann & Drunck GmbH
 */

#define LOG_CATEGORY UCLASS_TPM

#include <common.h>
#include <dm.h>
#include <asm/unaligned.h>
#include <tpm-common.h>
#include "tpm-utils.h"

enum tpm_version tpm_get_version(struct udevice *dev)
{
	struct tpm_chip_priv *priv = dev_get_uclass_priv(dev);

	return priv->version;
}

int pack_byte_string(u8 *str, size_t size, const char *format, ...)
{
	va_list args;
	size_t offset = 0, length = 0;
	u8 *data = NULL;
	u32 value = 0;

	va_start(args, format);
	for (; *format; format++) {
		switch (*format) {
		case 'b':
			offset = va_arg(args, size_t);
			value = va_arg(args, int);
			length = 1;
			break;
		case 'w':
			offset = va_arg(args, size_t);
			value = va_arg(args, int);
			length = 2;
			break;
		case 'd':
			offset = va_arg(args, size_t);
			value = va_arg(args, u32);
			length = 4;
			break;
		case 's':
			offset = va_arg(args, size_t);
			data = va_arg(args, u8 *);
			length = va_arg(args, u32);
			break;
		default:
			debug("Couldn't recognize format string\n");
			va_end(args);
			return -1;
		}

		if (offset + length > size) {
			va_end(args);
			return -1;
		}

		switch (*format) {
		case 'b':
			str[offset] = value;
			break;
		case 'w':
			put_unaligned_be16(value, str + offset);
			break;
		case 'd':
			put_unaligned_be32(value, str + offset);
			break;
		case 's':
			memcpy(str + offset, data, length);
			break;
		}
	}
	va_end(args);

	return 0;
}

int unpack_byte_string(const u8 *str, size_t size, const char *format, ...)
{
	va_list args;
	size_t offset = 0, length = 0;
	u8 *ptr8 = NULL;
	u16 *ptr16 = NULL;
	u32 *ptr32 = NULL;

	va_start(args, format);
	for (; *format; format++) {
		switch (*format) {
		case 'b':
			offset = va_arg(args, size_t);
			ptr8 = va_arg(args, u8 *);
			length = 1;
			break;
		case 'w':
			offset = va_arg(args, size_t);
			ptr16 = va_arg(args, u16 *);
			length = 2;
			break;
		case 'd':
			offset = va_arg(args, size_t);
			ptr32 = va_arg(args, u32 *);
			length = 4;
			break;
		case 's':
			offset = va_arg(args, size_t);
			ptr8 = va_arg(args, u8 *);
			length = va_arg(args, u32);
			break;
		default:
			va_end(args);
			debug("Couldn't recognize format string\n");
			return -1;
		}

		if (offset + length > size) {
			va_end(args);
			log_err("Failed to read: size=%zd, offset=%zx, len=%zx\n",
				size, offset, length);
			return -1;
		}

		switch (*format) {
		case 'b':
			*ptr8 = str[offset];
			break;
		case 'w':
			*ptr16 = get_unaligned_be16(str + offset);
			break;
		case 'd':
			*ptr32 = get_unaligned_be32(str + offset);
			break;
		case 's':
			memcpy(ptr8, str + offset, length);
			break;
		}
	}
	va_end(args);

	return 0;
}

u32 tpm_command_size(const void *command)
{
	const size_t command_size_offset = 2;

	return get_unaligned_be32(command + command_size_offset);
}

u32 tpm_return_code(const void *response)
{
	const size_t return_code_offset = 6;

	return get_unaligned_be32(response + return_code_offset);
}

u32 tpm_sendrecv_command(struct udevice *dev, const void *command,
			 void *response, size_t *size_ptr)
{
	int err, ret;
	u8 response_buffer[COMMAND_BUFFER_SIZE];
	size_t response_length;
	int i;

	if (response) {
		response_length = *size_ptr;
	} else {
		response = response_buffer;
		response_length = sizeof(response_buffer);
	}

	err = tpm_xfer(dev, command, tpm_command_size(command),
		       response, &response_length);

	if (err < 0)
		return err;

	if (size_ptr)
		*size_ptr = response_length;

	ret = tpm_return_code(response);

	log_debug("TPM response [ret:%d]: ", ret);
	for (i = 0; i < response_length; i++)
		log_debug("%02x ", ((u8 *)response)[i]);
	log_debug("\n");

	return ret;
}

int tpm_init(struct udevice *dev)
{
	return tpm_open(dev);
}
