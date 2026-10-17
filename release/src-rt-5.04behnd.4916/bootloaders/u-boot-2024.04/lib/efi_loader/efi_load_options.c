// SPDX-License-Identifier: GPL-2.0+
/*
 *  EFI boot manager
 *
 *  Copyright (c) 2018 AKASHI Takahiro, et.al.
 */

#define LOG_CATEGORY LOGC_EFI

#include <charset.h>
#include <log.h>
#include <malloc.h>
#include <efi_loader.h>
#include <asm/unaligned.h>

/**
 * efi_set_load_options() - set the load options of a loaded image
 *
 * @handle:		the image handle
 * @load_options_size:	size of load options
 * @load_options:	pointer to load options
 * Return:		status code
 */
efi_status_t efi_set_load_options(efi_handle_t handle,
				  efi_uintn_t load_options_size,
				  void *load_options)
{
	struct efi_loaded_image *loaded_image_info;
	struct efi_handler *handler;
	efi_status_t ret;

	ret = efi_search_protocol(handle, &efi_guid_loaded_image, &handler);
	if (ret != EFI_SUCCESS)
		return EFI_INVALID_PARAMETER;

	loaded_image_info = handler->protocol_interface;
	loaded_image_info->load_options = load_options;
	loaded_image_info->load_options_size = load_options_size;

	return EFI_SUCCESS;
}

/**
 * efi_deserialize_load_option() - parse serialized data
 *
 * Parse serialized data describing a load option and transform it to the
 * efi_load_option structure.
 *
 * @lo:		pointer to target
 * @data:	serialized data
 * @size:	size of the load option, on return size of the optional data
 * Return:	status code
 */
efi_status_t efi_deserialize_load_option(struct efi_load_option *lo, u8 *data,
					 efi_uintn_t *size)
{
	efi_uintn_t len;

	len = sizeof(u32);
	if (*size < len + 2 * sizeof(u16))
		return EFI_INVALID_PARAMETER;
	lo->attributes = get_unaligned_le32(data);
	data += len;
	*size -= len;

	len = sizeof(u16);
	lo->file_path_length = get_unaligned_le16(data);
	data += len;
	*size -= len;

	lo->label = (u16 *)data;
	len = u16_strnlen(lo->label, *size / sizeof(u16) - 1);
	if (lo->label[len])
		return EFI_INVALID_PARAMETER;
	len = (len + 1) * sizeof(u16);
	if (*size < len)
		return EFI_INVALID_PARAMETER;
	data += len;
	*size -= len;

	len = lo->file_path_length;
	if (*size < len)
		return EFI_INVALID_PARAMETER;
	lo->file_path = (struct efi_device_path *)data;
	if (efi_dp_check_length(lo->file_path, len) < 0)
		return EFI_INVALID_PARAMETER;
	data += len;
	*size -= len;

	lo->optional_data = data;

	return EFI_SUCCESS;
}

/**
 * efi_serialize_load_option() - serialize load option
 *
 * Serialize efi_load_option structure into byte stream for BootXXXX.
 *
 * @data:	buffer for serialized data
 * @lo:		load option
 * Return:	size of allocated buffer
 */
unsigned long efi_serialize_load_option(struct efi_load_option *lo, u8 **data)
{
	unsigned long label_len;
	unsigned long size;
	u8 *p;

	label_len = u16_strsize(lo->label);

	/* total size */
	size = sizeof(lo->attributes);
	size += sizeof(lo->file_path_length);
	size += label_len;
	size += lo->file_path_length;
	if (lo->optional_data)
		size += (utf8_utf16_strlen((const char *)lo->optional_data)
					   + 1) * sizeof(u16);
	p = malloc(size);
	if (!p)
		return 0;

	/* copy data */
	*data = p;
	memcpy(p, &lo->attributes, sizeof(lo->attributes));
	p += sizeof(lo->attributes);

	memcpy(p, &lo->file_path_length, sizeof(lo->file_path_length));
	p += sizeof(lo->file_path_length);

	memcpy(p, lo->label, label_len);
	p += label_len;

	memcpy(p, lo->file_path, lo->file_path_length);
	p += lo->file_path_length;

	if (lo->optional_data) {
		utf8_utf16_strcpy((u16 **)&p, (const char *)lo->optional_data);
		p += sizeof(u16); /* size of trailing \0 */
	}
	return size;
}
