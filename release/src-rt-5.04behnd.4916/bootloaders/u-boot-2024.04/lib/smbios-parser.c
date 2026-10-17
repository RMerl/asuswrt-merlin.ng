// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020, Bachmann electronic GmbH
 */

#define LOG_CATEGORY	LOGC_BOOT

#include <errno.h>
#include <smbios.h>
#include <string.h>
#include <tables_csum.h>
#include <linux/kernel.h>

const struct smbios_entry *smbios_entry(u64 address, u32 size)
{
	const struct smbios_entry *entry = (struct smbios_entry *)(uintptr_t)address;

	if (!address || !size)
		return NULL;

	if (memcmp(entry->anchor, "_SM_", 4))
		return NULL;

	if (table_compute_checksum(entry, entry->length))
		return NULL;

	return entry;
}

static u8 *find_next_header(u8 *pos)
{
	/* search for _double_ NULL bytes */
	while (!((*pos == 0) && (*(pos + 1) == 0)))
		pos++;

	/* step behind the double NULL bytes */
	pos += 2;

	return pos;
}

static struct smbios_header *get_next_header(const struct smbios_header *curr)
{
	u8 *pos = ((u8 *)curr) + curr->length;

	return (struct smbios_header *)find_next_header(pos);
}

const struct smbios_header *smbios_header(const struct smbios_entry *entry, int type)
{
	const unsigned int num_header = entry->struct_count;
	const struct smbios_header *header = (struct smbios_header *)((uintptr_t)entry->struct_table_address);

	for (unsigned int i = 0; i < num_header; i++) {
		if (header->type == type)
			return header;

		header = get_next_header(header);
	}

	return NULL;
}

static char *string_from_smbios_table(const struct smbios_header *header,
				      int idx)
{
	unsigned int i = 1;
	u8 *pos;

	if (!header)
		return NULL;

	pos = ((u8 *)header) + header->length;

	while (i < idx) {
		if (*pos == 0x0)
			i++;

		pos++;
	}

	return (char *)pos;
}

char *smbios_string(const struct smbios_header *header, int index)
{
	if (!header)
		return NULL;

	return string_from_smbios_table(header, index);
}

int smbios_update_version_full(void *smbios_tab, const char *version)
{
	const struct smbios_header *hdr;
	struct smbios_type0 *bios;
	uint old_len, len;
	char *ptr;

	log_info("Updating SMBIOS table at %p\n", smbios_tab);
	hdr = smbios_header(smbios_tab, SMBIOS_BIOS_INFORMATION);
	if (!hdr)
		return log_msg_ret("tab", -ENOENT);
	bios = (struct smbios_type0 *)hdr;
	ptr = smbios_string(hdr, bios->bios_ver);
	if (!ptr)
		return log_msg_ret("str", -ENOMEDIUM);

	/*
	 * This string is supposed to have at least enough bytes and is
	 * padded with spaces. Update it, taking care not to move the
	 * \0 terminator, so that other strings in the string table
	 * are not disturbed. See smbios_add_string()
	 */
	old_len = strnlen(ptr, SMBIOS_STR_MAX);
	len = strnlen(version, SMBIOS_STR_MAX);
	if (len > old_len)
		return log_ret(-ENOSPC);

	log_debug("Replacing SMBIOS type 0 version string '%s'\n", ptr);
	memcpy(ptr, version, len);
#ifdef LOG_DEBUG
	print_buffer((ulong)ptr, ptr, 1, old_len + 1, 0);
#endif

	return 0;
}

struct smbios_filter_param {
	u32 offset;
	u32 size;
	bool is_string;
};

struct smbios_filter_table {
	int type;
	struct smbios_filter_param *params;
	u32 count;
};

struct smbios_filter_param smbios_type1_filter_params[] = {
	{offsetof(struct smbios_type1, serial_number),
	 FIELD_SIZEOF(struct smbios_type1, serial_number), true},
	{offsetof(struct smbios_type1, uuid),
	 FIELD_SIZEOF(struct smbios_type1, uuid), false},
	{offsetof(struct smbios_type1, wakeup_type),
	 FIELD_SIZEOF(struct smbios_type1, wakeup_type), false},
};

struct smbios_filter_param smbios_type2_filter_params[] = {
	{offsetof(struct smbios_type2, serial_number),
	 FIELD_SIZEOF(struct smbios_type2, serial_number), true},
	{offsetof(struct smbios_type2, chassis_location),
	 FIELD_SIZEOF(struct smbios_type2, chassis_location), false},
};

struct smbios_filter_param smbios_type3_filter_params[] = {
	{offsetof(struct smbios_type3, serial_number),
	 FIELD_SIZEOF(struct smbios_type3, serial_number), true},
	{offsetof(struct smbios_type3, asset_tag_number),
	 FIELD_SIZEOF(struct smbios_type3, asset_tag_number), true},
};

struct smbios_filter_param smbios_type4_filter_params[] = {
	{offsetof(struct smbios_type4, serial_number),
	 FIELD_SIZEOF(struct smbios_type4, serial_number), true},
	{offsetof(struct smbios_type4, asset_tag),
	 FIELD_SIZEOF(struct smbios_type4, asset_tag), true},
	{offsetof(struct smbios_type4, part_number),
	 FIELD_SIZEOF(struct smbios_type4, part_number), true},
	{offsetof(struct smbios_type4, core_count),
	 FIELD_SIZEOF(struct smbios_type4, core_count), false},
	{offsetof(struct smbios_type4, core_enabled),
	 FIELD_SIZEOF(struct smbios_type4, core_enabled), false},
	{offsetof(struct smbios_type4, thread_count),
	 FIELD_SIZEOF(struct smbios_type4, thread_count), false},
	{offsetof(struct smbios_type4, core_count2),
	 FIELD_SIZEOF(struct smbios_type4, core_count2), false},
	{offsetof(struct smbios_type4, core_enabled2),
	 FIELD_SIZEOF(struct smbios_type4, core_enabled2), false},
	{offsetof(struct smbios_type4, thread_count2),
	 FIELD_SIZEOF(struct smbios_type4, thread_count2), false},
	{offsetof(struct smbios_type4, voltage),
	 FIELD_SIZEOF(struct smbios_type4, voltage), false},
};

struct smbios_filter_table smbios_filter_tables[] = {
	{SMBIOS_SYSTEM_INFORMATION, smbios_type1_filter_params,
	 ARRAY_SIZE(smbios_type1_filter_params)},
	{SMBIOS_BOARD_INFORMATION, smbios_type2_filter_params,
	 ARRAY_SIZE(smbios_type2_filter_params)},
	{SMBIOS_SYSTEM_ENCLOSURE, smbios_type3_filter_params,
	 ARRAY_SIZE(smbios_type3_filter_params)},
	{SMBIOS_PROCESSOR_INFORMATION, smbios_type4_filter_params,
	 ARRAY_SIZE(smbios_type4_filter_params)},
};

static void clear_smbios_table(struct smbios_header *header,
			       struct smbios_filter_param *filter,
			       u32 count)
{
	u32 i;
	char *str;
	u8 string_id;

	for (i = 0; i < count; i++) {
		if (filter[i].is_string) {
			string_id = *((u8 *)header + filter[i].offset);
			if (string_id == 0) /* string is empty */
				continue;

			str = smbios_string(header, string_id);
			if (!str)
				continue;

			/* string is cleared to space, keep '\0' terminator */
			memset(str, ' ', strlen(str));

		} else {
			memset((void *)((u8 *)header + filter[i].offset),
			       0, filter[i].size);
		}
	}
}

void smbios_prepare_measurement(const struct smbios3_entry *entry,
				struct smbios_header *smbios_copy)
{
	u32 i, j;
	void *table_end;
	struct smbios_header *header;

	table_end = (void *)((u8 *)smbios_copy + entry->table_maximum_size);

	for (i = 0; i < ARRAY_SIZE(smbios_filter_tables); i++) {
		header = smbios_copy;
		for (j = 0; (void *)header < table_end; j++) {
			if (header->type == smbios_filter_tables[i].type)
				break;

			header = get_next_header(header);
		}
		if ((void *)header >= table_end)
			continue;

		clear_smbios_table(header,
				   smbios_filter_tables[i].params,
				   smbios_filter_tables[i].count);
	}
}
