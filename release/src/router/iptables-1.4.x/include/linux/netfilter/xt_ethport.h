/* Minimal userspace header for the Broadcom ethport iptables match.
 * Reconstructed from libxt_ethport.c field usage; the referenced header
 * was never shipped in-tree (extensions/libxt_ethport.c includes it as of
 * gnuton f1dcb26242 but no kernel or userspace copy exists).
 */
#ifndef _XT_ETHPORT_H
#define _XT_ETHPORT_H

#include <linux/types.h>

#define XT_ETHPORT_MAX 15

struct xt_ethport_info {
	__u8 portnum;
	__u8 invert;
};

#endif /* _XT_ETHPORT_H */
