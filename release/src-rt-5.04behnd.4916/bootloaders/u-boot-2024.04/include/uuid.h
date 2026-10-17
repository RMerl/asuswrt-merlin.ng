/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014 Samsung Electronics
 * Przemyslaw Marczak <p.marczak@samsung.com>
 * Copyright 2022-2023 Arm Limited and/or its affiliates <open-source-office@arm.com>
 *
 * Authors:
 *   Abdellatif El Khlifi <abdellatif.elkhlifi@arm.com>
 */
#ifndef __UUID_H__
#define __UUID_H__

#include <linux/bitops.h>

/*
 * UUID - Universally Unique IDentifier - 128 bits unique number.
 *        There are 5 versions and one variant of UUID defined by RFC4122
 *        specification. A UUID contains a set of fields. The set varies
 *        depending on the version of the UUID, as shown below:
 *        - time, MAC address(v1),
 *        - user ID(v2),
 *        - MD5 of name or URL(v3),
 *        - random data(v4),
 *        - SHA-1 of name or URL(v5),
 *
 * Layout of UUID:
 * timestamp - 60-bit: time_low, time_mid, time_hi_and_version
 * version   - 4 bit (bit 4 through 7 of the time_hi_and_version)
 * clock seq - 14 bit: clock_seq_hi_and_reserved, clock_seq_low
 * variant:  - bit 6 and 7 of clock_seq_hi_and_reserved
 * node      - 48 bit
 *
 * source: https://www.ietf.org/rfc/rfc4122.txt
 *
 * UUID binary format (16 bytes):
 *
 * 4B-2B-2B-2B-6B (big endian - network byte order)
 *
 * UUID string is 36 length of characters (36 bytes):
 *
 * 0        9    14   19   24
 * xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
 *    be     be   be   be       be
 *
 * where x is a hexadecimal character. Fields are separated by '-'s.
 * When converting to a binary UUID, le means the field should be converted
 * to little endian and be means it should be converted to big endian.
 *
 * UUID is also used as GUID (Globally Unique Identifier) with the same binary
 * format but it differs in string format like below.
 *
 * GUID:
 * 0        9    14   19   24
 * xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
 *    le     le   le   be       be
 *
 * GUID is used e.g. in GPT (GUID Partition Table) as a partiions unique id.
 */

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

/* Use UUID_STR_LEN + 1 for string space */
#define UUID_STR_LEN		36
#define UUID_BIN_LEN		sizeof(struct uuid)

#define UUID_VERSION_MASK	0xf000
#define UUID_VERSION_SHIFT	12
#define UUID_VERSION		0x4

#define UUID_VARIANT_MASK	0xc0
#define UUID_VARIANT_SHIFT	7
#define UUID_VARIANT		0x1

int uuid_str_valid(const char *uuid);

/*
 * uuid_str_to_bin() - convert string UUID or GUID to big endian binary data.
 *
 * @param uuid_str - pointer to UUID or GUID string [37B] or GUID shorcut
 * @param uuid_bin - pointer to allocated array for big endian output [16B]
 * @str_format     - UUID string format: 0 - UUID; 1 - GUID
 * Return: 0 if OK, -EINVAL if the string is not a valid UUID
 */
int uuid_str_to_bin(const char *uuid_str, unsigned char *uuid_bin,
		    int str_format);

/*
 * uuid_bin_to_str() - convert big endian binary data to string UUID or GUID.
 *
 * @param uuid_bin:	pointer to binary data of UUID (big endian) [16B]
 * @param uuid_str:	pointer to allocated array for output string [37B]
 * @str_format:		bit 0: 0 - UUID; 1 - GUID
 *			bit 1: 0 - lower case; 2 - upper case
 */
void uuid_bin_to_str(const unsigned char *uuid_bin, char *uuid_str,
		     int str_format);

/*
 * uuid_guid_get_bin() - this function get GUID bin for string
 *
 * @param guid_str - pointer to partition type string
 * @param guid_bin - pointer to allocated array for big endian output [16B]
 */
int uuid_guid_get_bin(const char *guid_str, unsigned char *guid_bin);

/*
 * uuid_guid_get_str() - this function get string for GUID.
 *
 * @param guid_bin - pointer to string with partition type guid [16B]
 *
 * Returns NULL if the type GUID is not known.
 */
const char *uuid_guid_get_str(const unsigned char *guid_bin);

/*
 * gen_rand_uuid() - this function generates a random binary UUID version 4.
 *                   In this version all fields beside 4 bits of version and
 *                   2 bits of variant are randomly generated.
 *
 * @param uuid_bin - pointer to allocated array [16B]. Output is in big endian.
 */
void gen_rand_uuid(unsigned char *uuid_bin);

/*
 * gen_rand_uuid_str() - this function generates UUID v4 (random) in two string
 *                       formats UUID or GUID.
 *
 * @param uuid_str - pointer to allocated array [37B].
 * @param          - uuid output type: UUID - 0, GUID - 1
 */
void gen_rand_uuid_str(char *uuid_str, int str_format);

/**
 * uuid_str_to_le_bin() - Convert string UUID to little endian binary data.
 * @uuid_str:	pointer to UUID string
 * @uuid_bin:	pointer to allocated array for little endian output [16B]
 *
 * UUID string is 36 characters (36 bytes):
 *
 * xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
 *
 * where x is a hexadecimal character. Fields are separated by '-'s.
 * When converting to a little endian binary UUID, the string fields are reversed.
 *
 * Return:
 *
 *    uuid_bin filled with little endian UUID data
 *    On success 0 is returned. Otherwise, failure code.
 */
int uuid_str_to_le_bin(const char *uuid_str, unsigned char *uuid_bin);

#endif
