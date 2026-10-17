// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_util
 *
 * Copyright (c) 2017 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * Utility functions
 */

#include <efi_selftest.h>

struct efi_st_translate {
	u16 code;
	u16 *text;
};

static struct efi_st_translate efi_st_control_characters[] = {
	{0, u"Null"},
	{8, u"BS"},
	{9, u"TAB"},
	{10, u"LF"},
	{13, u"CR"},
	{0, NULL},
};

static u16 efi_st_ch[] = u"' '";
static u16 efi_st_unknown[] = u"unknown";

static struct efi_st_translate efi_st_scan_codes[] = {
	{0x00, u"Null"},
	{0x01, u"Up"},
	{0x02, u"Down"},
	{0x03, u"Right"},
	{0x04, u"Left"},
	{0x05, u"Home"},
	{0x06, u"End"},
	{0x07, u"Insert"},
	{0x08, u"Delete"},
	{0x09, u"Page Up"},
	{0x0a, u"Page Down"},
	{0x0b, u"FN 1"},
	{0x0c, u"FN 2"},
	{0x0d, u"FN 3"},
	{0x0e, u"FN 4"},
	{0x0f, u"FN 5"},
	{0x10, u"FN 6"},
	{0x11, u"FN 7"},
	{0x12, u"FN 8"},
	{0x13, u"FN 9"},
	{0x14, u"FN 10"},
	{0x15, u"FN 11"},
	{0x16, u"FN 12"},
	{0x17, u"Escape"},
	{0x68, u"FN 13"},
	{0x69, u"FN 14"},
	{0x6a, u"FN 15"},
	{0x6b, u"FN 16"},
	{0x6c, u"FN 17"},
	{0x6d, u"FN 18"},
	{0x6e, u"FN 19"},
	{0x6f, u"FN 20"},
	{0x70, u"FN 21"},
	{0x71, u"FN 22"},
	{0x72, u"FN 23"},
	{0x73, u"FN 24"},
	{0x7f, u"Mute"},
	{0x80, u"Volume Up"},
	{0x81, u"Volume Down"},
	{0x100, u"Brightness Up"},
	{0x101, u"Brightness Down"},
	{0x102, u"Suspend"},
	{0x103, u"Hibernate"},
	{0x104, u"Toggle Display"},
	{0x105, u"Recovery"},
	{0x106, u"Reject"},
	{0x0, NULL},
};

u16 *efi_st_translate_char(u16 code)
{
	struct efi_st_translate *tr;

	if (code >= ' ') {
		efi_st_ch[1] = code;
		return efi_st_ch;
	}
	for (tr = efi_st_control_characters; tr->text; ++tr) {
		if (tr->code == code)
			return tr->text;
	}
	return efi_st_unknown;
}

u16 *efi_st_translate_code(u16 code)
{
	struct efi_st_translate *tr;

	for (tr = efi_st_scan_codes; tr->text; ++tr) {
		if (tr->code == code)
			return tr->text;
	}
	return efi_st_unknown;
}

int efi_st_strcmp_16_8(const u16 *buf1, const unsigned char *buf2)
{
	for (; *buf1 || *buf2; ++buf1, ++buf2) {
		if (*buf1 != *buf2)
			return *buf1 - *buf2;
	}
	return 0;
}

void *efi_st_get_config_table(const efi_guid_t *guid)
{
	size_t i;

	for (i = 0; i < st_systable->nr_tables; i++) {
		if (!guidcmp(guid, &st_systable->tables[i].guid))
			return st_systable->tables[i].table;
	}
	return NULL;
}
