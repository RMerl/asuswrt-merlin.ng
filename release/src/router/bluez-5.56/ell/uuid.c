/*
 *
 *  Embedded Linux library
 *
 *  Copyright (C) 2011-2014  Intel Corporation. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <stdio.h>

#include "checksum.h"
#include "random.h"
#include "private.h"
#include "uuid.h"

const uint8_t L_UUID_NAMESPACE_DNS[16] = {
	0x6b, 0xa7, 0xb8, 0x10, 0x9d, 0xad, 0x11, 0xd1,
	0x80, 0xb4, 0x00, 0xc0, 0x4f, 0xd4, 0x30, 0xc8,
};

const uint8_t L_UUID_NAMESPACE_URL[16] = {
	0x6b, 0xa7, 0xb8, 0x11, 0x9d, 0xad, 0x11, 0xd1,
	0x80, 0xb4, 0x00, 0xc0, 0x4f, 0xd4, 0x30, 0xc8,
};

const uint8_t L_UUID_NAMESPACE_OID[16] = {
	0x6b, 0xa7, 0xb8, 0x12, 0x9d, 0xad, 0x11, 0xd1,
	0x80, 0xb4, 0x00, 0xc0, 0x4f, 0xd4, 0x30, 0xc8,
};

const uint8_t L_UUID_NAMESPACE_X500[16] = {
	0x6b, 0xa7, 0xb8, 0x14, 0x9d, 0xad, 0x11, 0xd1,
	0x80, 0xb4, 0x00, 0xc0, 0x4f, 0xd4, 0x30, 0xc8,
};

/* RFC 4122, Section 4.3 */
static bool name_from_namespace(int version, const uint8_t nsid[16],
				const void *name,
				size_t name_size, uint8_t out_uuid[16])
{
	enum l_checksum_type type;
	struct l_checksum *hash;
	struct iovec iov[2];

	if (unlikely(!out_uuid))
		return false;

	switch (version) {
	case 3:
		type = L_CHECKSUM_MD5;
		break;
	case 5:
		type = L_CHECKSUM_SHA1;
		break;
	default:
		return false;
	}

	hash = l_checksum_new(type);
	if (!hash)
		return false;

	iov[0].iov_base = (void *) nsid;
	iov[0].iov_len = 16;
	iov[1].iov_base = (void *) name;
	iov[1].iov_len = name_size;

	/* Compute the hash of the name space ID concatenated with the name. */
	l_checksum_updatev(hash, iov, 2);

	/* o Set octets zero through 3 of the time_low field to octets zero
	 * through 3 of the hash.
	 *
	 * o Set octets zero and one of the time_mid field to octets 4 and 5 of
	 * the hash.
	 *
	 * o Set octets zero and one of the time_hi_and_version field to octets
	 * 6 and 7 of the hash.
	 * o Set the four most significant bits (bits 12 through 15) of the
	 * time_hi_and_version field to the appropriate 4-bit version number
	 * from Section 4.1.3.
	 *
	 * o Set the clock_seq_hi_and_reserved field to octet 8 of the hash.
	 * o Set the two most significant bits (bits 6 and 7) of the
	 * clock_seq_hi_and_reserved to zero and one, respectively.
	 *
	 * o  Set the clock_seq_low field to octet 9 of the hash.
	 *
	 * o  Set octets zero through five of the node field to octets 10
	 * through 15 of the hash.
	 */

	l_checksum_get_digest(hash, out_uuid, 16);

	/* Set 4 MSB bits of time_hi_and_version field */
	out_uuid[6] &= 0x0f;
	out_uuid[6] |= version << 4;

	/* Set 2 MSB of clock_seq_hi_and_reserved to 10 */
	out_uuid[8] &= 0x3f;
	out_uuid[8] |= 0x80;

	l_checksum_free(hash);

	return true;
}

LIB_EXPORT bool l_uuid_v3(const uint8_t nsid[16], const void *name,
				size_t name_size, uint8_t out_uuid[16])
{
	return name_from_namespace(3, nsid, name, name_size, out_uuid);
}

LIB_EXPORT bool l_uuid_v5(const uint8_t nsid[16], const void *name,
				size_t name_size, uint8_t out_uuid[16])
{
	return name_from_namespace(5, nsid, name, name_size, out_uuid);
}

/* RFC 4122, Section 4.4 */
LIB_EXPORT bool l_uuid_v4(uint8_t out_uuid[16])
{
	if (unlikely(!out_uuid))
		return false;

	if (!l_getrandom(out_uuid, 16))
		return false;

	/*
	 * o Set the two most significant bits (bits 6 and 7) of the
	 * clock_seq_hi_and_reserved to zero and one, respectively.
	 *
	 * o Set the four most significant bits (bits 12 through 15) of the
	 * time_hi_and_version field to the 4-bit version number from
	 * Section 4.1.3.
	 *
	 * o Set all the other bits to randomly (or pseudo-randomly) chosen
	 * values.
	 */

	/* Set 4 MSB bits of time_hi_and_version field */
	out_uuid[6] &= 0x0f;
	out_uuid[6] |= 4 << 4;

	/* Set 2 MSB of clock_seq_hi_and_reserved to 10 */
	out_uuid[8] &= 0x3f;
	out_uuid[8] |= 0x80;

	return true;
}

/**
 * l_uuid_is_valid:
 * @uuid: UUID to check.
 *
 * Checks whether the given UUID is valid according to RFC 4122.  This function
 * checks that the version field is set properly and the variant of the UUID
 * is set to RFC 4122.
 *
 * Returns: Whether the UUID is valid
 **/
LIB_EXPORT bool l_uuid_is_valid(const uint8_t uuid[16])
{
	unsigned int version;
	unsigned int variant;

	if (!uuid)
		return false;

	variant = uuid[8] >> 6;
	if (variant != 2)
		return false;

	version = uuid[6] >> 4;
	if (version < 1 || version > 5)
		return false;

	return true;
}

LIB_EXPORT enum l_uuid_version l_uuid_get_version(const uint8_t uuid[16])
{
	unsigned int version;

	version = uuid[6] >> 4;
	return version;
}

LIB_EXPORT bool l_uuid_to_string(const uint8_t uuid[16],
						char *dest, size_t dest_size)
{
	int n;

	n = snprintf(dest, dest_size, "%02x%02x%02x%02x-%02x%02x-%02x%02x-"
					"%02x%02x-%02x%02x%02x%02x%02x%02x",
					uuid[0], uuid[1], uuid[2], uuid[3],
					uuid[4], uuid[5],
					uuid[6], uuid[7],
					uuid[8], uuid[9],
					uuid[10], uuid[11], uuid[12],
					uuid[13], uuid[14], uuid[15]);

	if (n < 0 || (size_t) n >= dest_size)
		return false;

	return true;
}

LIB_EXPORT bool l_uuid_from_string(const char *src, uint8_t uuid[16])
{
	uint8_t buf[16];
	int n;

	/*
	 * textual representation: 32 hex digits + 4 group separators
	 */
	if (strlen(src) < 16 * 2 + 4)
		return false;

	n = sscanf(src,
			"%02hhx%02hhx%02hhx%02hhx-"
			"%02hhx%02hhx-"
			"%02hhx%02hhx-"
			"%02hhx%02hhx-"
			"%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx",
			&buf[0], &buf[1], &buf[2], &buf[3],
			&buf[4], &buf[5],
			&buf[6], &buf[7],
			&buf[8], &buf[9],
			&buf[10], &buf[11], &buf[12],
			&buf[13], &buf[14], &buf[15]);

	if (n != 16)
		return false;

	if (!l_uuid_is_valid(buf))
		return false;

	memcpy(uuid, buf, sizeof(buf));
	return true;
}
