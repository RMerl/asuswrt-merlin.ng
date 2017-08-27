#ifndef _LIBNETFILTER_CONNTRACK_ICMP_H_
#define _LIBNETFILTER_CONNTRACK_ICMP_H_

#ifdef __cplusplus
extern "C" {
#endif

/* WARNING: do not use these flags in your new applications, they are obsolete
 * and we keep them here to avoid breaking backward compatibility. */
enum icmp_flags {
	ICMP_TYPE_BIT = 0,
	ICMP_TYPE = (1 << ICMP_TYPE_BIT),

	ICMP_CODE_BIT = 1,
	ICMP_CODE = (1 << ICMP_CODE_BIT),

	ICMP_ID_BIT = 2,
	ICMP_ID = (1 << ICMP_ID_BIT)
};

#ifdef __cplusplus
}
#endif

#endif
