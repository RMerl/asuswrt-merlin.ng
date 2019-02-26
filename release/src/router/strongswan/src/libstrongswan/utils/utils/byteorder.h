/*
 * Copyright (C) 2008-2014 Tobias Brunner
 * Copyright (C) 2008 Martin Willi
 * HSR Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

/**
 * @defgroup byteorder_i byteorder
 * @{ @ingroup utils_i
 */

#ifndef BYTEORDER_H_
#define BYTEORDER_H_

/**
 * Architecture independent bitfield definition helpers (at least with GCC).
 *
 * Defines a bitfield with a type t and a fixed size of bitfield members, e.g.:
 * BITFIELD2(uint8_t,
 *     low: 4,
 *     high: 4,
 * ) flags;
 * The member defined first placed at bit 0.
 */
#if BYTE_ORDER == LITTLE_ENDIAN
#define BITFIELD2(t, a, b,...)			struct { t a; t b; __VA_ARGS__}
#define BITFIELD3(t, a, b, c,...)		struct { t a; t b; t c; __VA_ARGS__}
#define BITFIELD4(t, a, b, c, d,...)	struct { t a; t b; t c; t d; __VA_ARGS__}
#define BITFIELD5(t, a, b, c, d, e,...)	struct { t a; t b; t c; t d; t e; __VA_ARGS__}
#elif BYTE_ORDER == BIG_ENDIAN
#define BITFIELD2(t, a, b,...)			struct { t b; t a; __VA_ARGS__}
#define BITFIELD3(t, a, b, c,...)		struct { t c; t b; t a; __VA_ARGS__}
#define BITFIELD4(t, a, b, c, d,...)	struct { t d; t c; t b; t a; __VA_ARGS__}
#define BITFIELD5(t, a, b, c, d, e,...)	struct { t e; t d; t c; t b; t a; __VA_ARGS__}
#endif

#ifndef le16toh
# if BYTE_ORDER == BIG_ENDIAN
#  define le16toh(x) __builtin_bswap16(x)
# else
#  define le16toh(x) (x)
# endif
#endif
#ifndef htole16
# if BYTE_ORDER == BIG_ENDIAN
#  define htole16(x) __builtin_bswap16(x)
# else
#  define htole16(x) (x)
# endif
#endif

#ifndef le32toh
# if BYTE_ORDER == BIG_ENDIAN
#  define le32toh(x) __builtin_bswap32(x)
# else
#  define le32toh(x) (x)
# endif
#endif
#ifndef htole32
# if BYTE_ORDER == BIG_ENDIAN
#  define htole32(x) __builtin_bswap32(x)
# else
#  define htole32(x) (x)
# endif
#endif

#ifndef le64toh
# if BYTE_ORDER == BIG_ENDIAN
#  define le64toh(x) __builtin_bswap64(x)
# else
#  define le64toh(x) (x)
# endif
#endif
#ifndef htole64
# if BYTE_ORDER == BIG_ENDIAN
#  define htole64(x) __builtin_bswap64(x)
# else
#  define htole64(x) (x)
# endif
#endif

#ifndef be64toh
# if BYTE_ORDER == BIG_ENDIAN
#  define be64toh(x) (x)
# else
#  define be64toh(x) __builtin_bswap64(x)
# endif
#endif
#ifndef htobe64
# if BYTE_ORDER == BIG_ENDIAN
#  define htobe64(x) (x)
# else
#  define htobe64(x) __builtin_bswap64(x)
# endif
#endif

/**
 * Write a 16-bit host order value in network order to an unaligned address.
 *
 * @param host		host order 16-bit value
 * @param network	unaligned address to write network order value to
 */
static inline void htoun16(void *network, uint16_t host)
{
	char *unaligned = (char*)network;

	host = htons(host);
	memcpy(unaligned, &host, sizeof(host));
}

/**
 * Write a 32-bit host order value in network order to an unaligned address.
 *
 * @param host		host order 32-bit value
 * @param network	unaligned address to write network order value to
 */
static inline void htoun32(void *network, uint32_t host)
{
	char *unaligned = (char*)network;

	host = htonl(host);
	memcpy((char*)unaligned, &host, sizeof(host));
}

/**
 * Write a 64-bit host order value in network order to an unaligned address.
 *
 * @param host		host order 64-bit value
 * @param network	unaligned address to write network order value to
 */
static inline void htoun64(void *network, uint64_t host)
{
	char *unaligned = (char*)network;

	host = htobe64(host);
	memcpy((char*)unaligned, &host, sizeof(host));
}

/**
 * Read a 16-bit value in network order from an unaligned address to host order.
 *
 * @param network	unaligned address to read network order value from
 * @return			host order value
 */
static inline uint16_t untoh16(void *network)
{
	char *unaligned = (char*)network;
	uint16_t tmp;

	memcpy(&tmp, unaligned, sizeof(tmp));
	return ntohs(tmp);
}

/**
 * Read a 32-bit value in network order from an unaligned address to host order.
 *
 * @param network	unaligned address to read network order value from
 * @return			host order value
 */
static inline uint32_t untoh32(void *network)
{
	char *unaligned = (char*)network;
	uint32_t tmp;

	memcpy(&tmp, unaligned, sizeof(tmp));
	return ntohl(tmp);
}

/**
 * Read a 64-bit value in network order from an unaligned address to host order.
 *
 * @param network	unaligned address to read network order value from
 * @return			host order value
 */
static inline uint64_t untoh64(void *network)
{
	char *unaligned = (char*)network;
	uint64_t tmp;

	memcpy(&tmp, unaligned, sizeof(tmp));
	return be64toh(tmp);
}

/**
 * Read a 16-bit value in little-endian order from unaligned address.
 *
 * @param p			unaligned address to read little endian value from
 * @return			host order value
 */
static inline uint16_t uletoh16(void *p)
{
	uint16_t ret;

	memcpy(&ret, p, sizeof(ret));
	ret = le16toh(ret);
	return ret;
}

/**
 * Write a 16-bit value in little-endian to an unaligned address.
 *
 * @param p			host order 16-bit value
 * @param v			unaligned address to write little endian value to
 */
static inline void htoule16(void *p, uint16_t v)
{
	v = htole16(v);
	memcpy(p, &v, sizeof(v));
}

/**
 * Read a 32-bit value in little-endian order from unaligned address.
 *
 * @param p			unaligned address to read little endian value from
 * @return			host order value
 */
static inline uint32_t uletoh32(void *p)
{
	uint32_t ret;

	memcpy(&ret, p, sizeof(ret));
	ret = le32toh(ret);
	return ret;
}

/**
 * Write a 32-bit value in little-endian to an unaligned address.
 *
 * @param p			host order 32-bit value
 * @param v			unaligned address to write little endian value to
 */
static inline void htoule32(void *p, uint32_t v)
{
	v = htole32(v);
	memcpy(p, &v, sizeof(v));
}

#endif /** BYTEORDER_H_ @} */
