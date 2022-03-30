/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014 Samsung Electronics
 * Przemyslaw Marczak <p.marczak@samsung.com>
 */
#ifndef __UUID_H__
#define __UUID_H__

#include <linux/bitops.h>

/* This is structure is in big-endian */
struct uuid {
	unsigned int time_low;
	unsigned short time_mid;
	unsigned short time_hi_and_version;
	unsigned char clock_seq_hi_and_reserved;
	unsigned char clock_seq_low;
	unsigned char node[6];
} __packed;

/* Bits of a bitmask specifying the output format for GUIDs */
#define UUID_STR_FORMAT_STD	0
#define UUID_STR_FORMAT_GUID	BIT(0)
#define UUID_STR_UPPER_CASE	BIT(1)

#define UUID_STR_LEN		36
#define UUID_BIN_LEN		sizeof(struct uuid)

#define UUID_VERSION_MASK	0xf000
#define UUID_VERSION_SHIFT	12
#define UUID_VERSION		0x4

#define UUID_VARIANT_MASK	0xc0
#define UUID_VARIANT_SHIFT	7
#define UUID_VARIANT		0x1

int uuid_str_valid(const char *uuid);
int uuid_str_to_bin(char *uuid_str, unsigned char *uuid_bin, int str_format);
void uuid_bin_to_str(unsigned char *uuid_bin, char *uuid_str, int str_format);
#ifdef CONFIG_PARTITION_TYPE_GUID
int uuid_guid_get_bin(const char *guid_str, unsigned char *guid_bin);
int uuid_guid_get_str(unsigned char *guid_bin, char *guid_str);
#endif
void gen_rand_uuid(unsigned char *uuid_bin);
void gen_rand_uuid_str(char *uuid_str, int str_format);
#endif
