#ifndef _XT_FLOWLABEL_H
#define _XT_FLOWLABEL_H

/*
 * Shared library add-on to iptables to add IPv6 flowlabel match support
 *
 * BRCM, Feb, 1. 2019.
 */

#include <byteswap.h>


#if defined(__LITTLE_ENDIAN) && (__BYTE_ORDER == __LITTLE_ENDIAN)
#define be32_to_cpu(x) bswap_32(x)
#define cpu_to_be32(x) bswap_32(x)

#elif defined(__BIG_ENDIAN) && (__BYTE_ORDER == __BIG_ENDIAN)
#define be32_to_cpu(x) (x)
#define cpu_to_be32(x) (x)

#else
#error "You must define __BYTE_ORDER to be __LITTLE_ENDIAN or __BIG_ENDIAN."
#endif // defined(__LITTLE_ENDIAN) && (__BYTE_ORDER == __LITTLE_ENDIAN)


#define XT_FLOWLABEL_MAX	0x000FFFFF

/* match info */
struct xt_flowlabel_info {
	__be32 flowlabel;
	__u8 invert;
};

#endif /*_XT_FLOWLABEL_H*/

