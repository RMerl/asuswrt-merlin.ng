/******************************************************************************

   Copyright 2023 MaxLinear, Inc.

   For licensing information, see the file 'LICENSE' in the root folder of
   this software module.

******************************************************************************/

#ifndef _GSW_TYPES_H_
#define _GSW_TYPES_H_
/** \file gsw_types.h GSW Base Types */

#include <stdint.h>

/** \brief This is the unsigned 64-bit datatype. */
typedef uint64_t u64;
/** \brief This is the unsigned 32-bit datatype. */
typedef uint32_t u32;
/** \brief This is the unsigned 16-bit datatype. */
typedef uint16_t u16;
/** \brief This is the unsigned 8-bit datatype. */
typedef uint8_t u8;
/** \brief This is the signed 64-bit datatype. */
typedef int64_t i64;
/** \brief This is the signed 32-bit datatype. */
typedef int32_t i32;
/** \brief This is the signed 16-bit datatype. */
typedef int16_t i16;
/** \brief This is the signed 8-bit datatype. */
typedef int8_t i8;
/** \brief This is the signed 8-bit datatype. */
typedef int32_t s32;
/** \brief This is the signed 8-bit datatype. */
typedef int8_t s8;

/** \brief MAC Address Field Size.
    Number of bytes used to store MAC address information. */
#define GSW_MAC_ADDR_LEN 6

/** \brief This enumeration type defines two boolean states: False and True. */
enum {
	/** Boolean False. */
	GSW_FALSE		= 0,
	/** Boolean True. */
	GSW_TRUE		= 1
};

/** \brief This is the boolean datatype. */
typedef uint8_t gsw_bool_t;

/** \brief This is a union to describe the IPv4 and IPv6 Address in numeric representation. Used by multiple Structures and APIs. */
typedef union {
	/** Describe the IPv4 address.
	    Only used if the IPv4 address should be read or configured.
	    Cannot be used together with the IPv6 address fields. */
	u32	nIPv4;
	/** Describe the IPv6 address.
	    Only used if the IPv6 address should be read or configured.
	    Cannot be used together with the IPv4 address fields. */
	u16	nIPv6[8];
} GSW_IP_t;

/** \brief Selection to use IPv4 or IPv6.
    Used  along with \ref GSW_IP_t to denote which union member to be accessed.
*/
typedef enum {
	/** IPv4 Type */
	GSW_IP_SELECT_IPV4	= 0,
	/** IPv6 Type */
	GSW_IP_SELECT_IPV6	= 1
} GSW_IP_Select_t;

#endif /* _GSW_TYPES_H_ */
