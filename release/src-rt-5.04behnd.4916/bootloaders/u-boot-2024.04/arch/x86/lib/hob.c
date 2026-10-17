// SPDX-License-Identifier: Intel
/*
 * Copyright (C) 2013, Intel Corporation
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <asm/hob.h>

/**
 * Returns the next instance of a HOB type from the starting HOB.
 *
 * @type:     HOB type to search
 * @hob_list: A pointer to the HOB list
 *
 * Return: A HOB object with matching type; Otherwise NULL.
 */
const struct hob_header *hob_get_next_hob(uint type, const void *hob_list)
{
	const struct hob_header *hdr;

	hdr = hob_list;

	/* Parse the HOB list until end of list or matching type is found */
	while (!end_of_hob(hdr)) {
		if (hdr->type == type)
			return hdr;

		hdr = get_next_hob(hdr);
	}

	return NULL;
}

/**
 * Returns the next instance of the matched GUID HOB from the starting HOB.
 *
 * @guid:     GUID to search
 * @hob_list: A pointer to the HOB list
 *
 * Return: A HOB object with matching GUID; Otherwise NULL.
 */
const struct hob_header *hob_get_next_guid_hob(const efi_guid_t *guid,
					       const void *hob_list)
{
	const struct hob_header *hdr;
	struct hob_guid *guid_hob;

	hdr = hob_list;
	while ((hdr = hob_get_next_hob(HOB_TYPE_GUID_EXT, hdr))) {
		guid_hob = (struct hob_guid *)hdr;
		if (!guidcmp(guid, &guid_hob->name))
			break;
		hdr = get_next_hob(hdr);
	}

	return hdr;
}

/**
 * This function retrieves a GUID HOB data buffer and size.
 *
 * @hob_list:      A HOB list pointer.
 * @len:           A pointer to the GUID HOB data buffer length.
 *                 If the GUID HOB is located, the length will be updated.
 * @guid           A pointer to HOB GUID.
 *
 * Return: NULL:   Failed to find the GUID HOB.
 * Return: others: GUID HOB data buffer pointer.
 */
void *hob_get_guid_hob_data(const void *hob_list, u32 *len,
			    const efi_guid_t *guid)
{
	const struct hob_header *guid_hob;

	guid_hob = hob_get_next_guid_hob(guid, hob_list);
	if (!guid_hob)
		return NULL;

	if (len)
		*len = get_guid_hob_data_size(guid_hob);

	return get_guid_hob_data(guid_hob);
}
