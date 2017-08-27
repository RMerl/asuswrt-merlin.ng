#if defined(CONFIG_BCM_KF_NETFILTER) || !defined(CONFIG_BCM_IN_KERNEL)
#ifndef _NF_CONNTRACK_PT_H
#define _NF_CONNTRACK_PT_H
/* PT tracking. */
#define PT_MAX_ENTRIES	100
#define PT_MAX_PORTS	1000
#define PT_MAX_EXPECTED	1000
#define PT_TIMEOUT	180

#if defined(CONFIG_BCM_KF_RUNNER)
#if defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)
#define PT_PROTO_TCP 	1
#define PT_PROTO_UDP 	2
#define PT_PROTO_ALL 	(PT_PROTO_TCP|PT_PROTO_UDP)
#define PT_PROTO_ALL_IN	0
#endif /* CONFIG_BCM_RUNNER */
#endif /* CONFIG_BCM_KF_RUNNER */

#endif /* _NF_CONNTRACK_PT_H */
#endif
