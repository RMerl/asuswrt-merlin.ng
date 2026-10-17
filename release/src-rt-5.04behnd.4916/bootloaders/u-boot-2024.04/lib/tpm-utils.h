/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2013 The Chromium OS Authors.
 * Coypright (c) 2013 Guntermann & Drunck GmbH
 */

#ifndef __TPM_UTILS_H
#define __TPM_UTILS_H

#define COMMAND_BUFFER_SIZE 256

/* Internal error of TPM command library */
#define TPM_LIB_ERROR ((u32)~0u)

/* To make strings of commands more easily */
#define __MSB(x) ((x) >> 8)
#define __LSB(x) ((x) & 0xFF)
#define tpm_u16(x) __MSB(x), __LSB(x)
#define tpm_u32(x) tpm_u16((x) >> 16), tpm_u16((x) & 0xFFFF)

/**
 * Pack data into a byte string.  The data types are specified in
 * the format string: 'b' means unsigned byte, 'w' unsigned word,
 * 'd' unsigned double word, and 's' byte string.  The data are a
 * series of offsets and values (for type byte string there are also
 * lengths).  The data values are packed into the byte string
 * sequentially, and so a latter value could over-write a former
 * value.
 *
 * @param str		output string
 * @param size		size of output string
 * @param format	format string
 * @param ...		data points
 * Return: 0 on success, non-0 on error
 */
int pack_byte_string(u8 *str, size_t size, const char *format, ...);

/**
 * Unpack data from a byte string.  The data types are specified in
 * the format string: 'b' means unsigned byte, 'w' unsigned word,
 * 'd' unsigned double word, and 's' byte string.  The data are a
 * series of offsets and pointers (for type byte string there are also
 * lengths).
 *
 * @param str		output string
 * @param size		size of output string
 * @param format	format string
 * @param ...		data points
 * Return: 0 on success, non-0 on error
 */
int unpack_byte_string(const u8 *str, size_t size, const char *format, ...);

/**
 * Get TPM command size.
 *
 * @param command	byte string of TPM command
 * Return: command size of the TPM command
 */
u32 tpm_command_size(const void *command);

/**
 * Get TPM response return code, which is one of TPM_RESULT values.
 *
 * @param response	byte string of TPM response
 * Return: return code of the TPM response
 */
u32 tpm_return_code(const void *response);

/**
 * Send a TPM command and return response's return code, and optionally
 * return response to caller.
 *
 * @param command	byte string of TPM command
 * @param response	output buffer for TPM response, or NULL if the
 *			caller does not care about it
 * @param size_ptr	output buffer size (input parameter) and TPM
 *			response length (output parameter); this parameter
 *			is a bidirectional
 * Return: return code of the TPM response
 */
u32 tpm_sendrecv_command(struct udevice *dev, const void *command,
			 void *response, size_t *size_ptr);

#endif /* __TPM_UTILS_H */
