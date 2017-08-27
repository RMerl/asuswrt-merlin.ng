#ifndef _NFNETLINK_H
#define _NFNETLINK_H
#include <linux/types.h>

#ifndef __KERNEL__
/* nfnetlink groups: Up to 32 maximum - backwards compatibility for userspace */
#define NF_NETLINK_CONNTRACK_NEW 		0x00000001
#define NF_NETLINK_CONNTRACK_UPDATE		0x00000002
#define NF_NETLINK_CONNTRACK_DESTROY		0x00000004
#define NF_NETLINK_CONNTRACK_EXP_NEW		0x00000008
#define NF_NETLINK_CONNTRACK_EXP_UPDATE		0x00000010
#define NF_NETLINK_CONNTRACK_EXP_DESTROY	0x00000020
#endif

enum nfnetlink_groups {
	NFNLGRP_NONE,
#define NFNLGRP_NONE			NFNLGRP_NONE
	NFNLGRP_CONNTRACK_NEW,
#define NFNLGRP_CONNTRACK_NEW		NFNLGRP_CONNTRACK_NEW
	NFNLGRP_CONNTRACK_UPDATE,
#define NFNLGRP_CONNTRACK_UPDATE	NFNLGRP_CONNTRACK_UPDATE
	NFNLGRP_CONNTRACK_DESTROY,
#define NFNLGRP_CONNTRACK_DESTROY	NFNLGRP_CONNTRACK_DESTROY
	NFNLGRP_CONNTRACK_EXP_NEW,
#define	NFNLGRP_CONNTRACK_EXP_NEW	NFNLGRP_CONNTRACK_EXP_NEW
	NFNLGRP_CONNTRACK_EXP_UPDATE,
#define NFNLGRP_CONNTRACK_EXP_UPDATE	NFNLGRP_CONNTRACK_EXP_UPDATE
	NFNLGRP_CONNTRACK_EXP_DESTROY,
#define NFNLGRP_CONNTRACK_EXP_DESTROY	NFNLGRP_CONNTRACK_EXP_DESTROY
	__NFNLGRP_MAX,
};
#define NFNLGRP_MAX	(__NFNLGRP_MAX - 1)

/* General form of address family dependent message.
 */
struct nfgenmsg {
	u_int8_t  nfgen_family;		/* AF_xxx */
	u_int8_t  version;		/* nfnetlink version */
	__be16    res_id;		/* resource id */
};

#define NFNETLINK_V0	0

/* netfilter netlink message types are split in two pieces:
 * 8 bit subsystem, 8bit operation.
 */

#define NFNL_SUBSYS_ID(x)	((x & 0xff00) >> 8)
#define NFNL_MSG_TYPE(x)	(x & 0x00ff)

/* No enum here, otherwise __stringify() trick of MODULE_ALIAS_NFNL_SUBSYS()
 * won't work anymore */
#define NFNL_SUBSYS_NONE 		0
#define NFNL_SUBSYS_CTNETLINK		1
#define NFNL_SUBSYS_CTNETLINK_EXP	2
#define NFNL_SUBSYS_QUEUE		3
#define NFNL_SUBSYS_ULOG		4
#define NFNL_SUBSYS_COUNT		5

#endif	/* _NFNETLINK_H */
