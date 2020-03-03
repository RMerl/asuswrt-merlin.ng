/*
 *
 *		SNMP MIB entries for the IP subsystem.
 *		
 *		Alan Cox <gw4pts@gw4pts.ampr.org>
 *
 *		We don't chose to implement SNMP in the kernel (this would
 *		be silly as SNMP is a pain in the backside in places). We do
 *		however need to collect the MIB statistics and export them
 *		out of /proc (eventually)
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 */
 
#ifndef _SNMP_H
#define _SNMP_H

#include <linux/cache.h>
#include <linux/snmp.h>
#include <linux/smp.h>

/*
 * Mibs are stored in array of unsigned long.
 */
/*
 * struct snmp_mib{}
 *  - list of entries for particular API (such as /proc/net/snmp)
 *  - name of entries.
 */
struct snmp_mib {
	const char *name;
	int entry;
};

#define SNMP_MIB_ITEM(_name,_entry)	{	\
	.name = _name,				\
	.entry = _entry,			\
}

#define SNMP_MIB_SENTINEL {	\
	.name = NULL,		\
	.entry = 0,		\
}

/*
 * We use unsigned longs for most mibs but u64 for ipstats.
 */
#include <linux/u64_stats_sync.h>

/* IPstats */
#define IPSTATS_MIB_MAX	__IPSTATS_MIB_MAX
struct ipstats_mib {
	/* mibs[] must be first field of struct ipstats_mib */
	u64		mibs[IPSTATS_MIB_MAX];
	struct u64_stats_sync syncp;
};

/* ICMP */
#define ICMP_MIB_MAX	__ICMP_MIB_MAX
struct icmp_mib {
	unsigned long	mibs[ICMP_MIB_MAX];
};

#define ICMPMSG_MIB_MAX	__ICMPMSG_MIB_MAX
struct icmpmsg_mib {
	atomic_long_t	mibs[ICMPMSG_MIB_MAX];
};

/* ICMP6 (IPv6-ICMP) */
#define ICMP6_MIB_MAX	__ICMP6_MIB_MAX
/* per network ns counters */
struct icmpv6_mib {
	unsigned long	mibs[ICMP6_MIB_MAX];
};
/* per device counters, (shared on all cpus) */
struct icmpv6_mib_device {
	atomic_long_t	mibs[ICMP6_MIB_MAX];
};

#define ICMP6MSG_MIB_MAX  __ICMP6MSG_MIB_MAX
/* per network ns counters */
struct icmpv6msg_mib {
	atomic_long_t	mibs[ICMP6MSG_MIB_MAX];
};
/* per device counters, (shared on all cpus) */
struct icmpv6msg_mib_device {
	atomic_long_t	mibs[ICMP6MSG_MIB_MAX];
};


/* TCP */
#define TCP_MIB_MAX	__TCP_MIB_MAX
struct tcp_mib {
	unsigned long	mibs[TCP_MIB_MAX];
};

/* UDP */
#define UDP_MIB_MAX	__UDP_MIB_MAX
struct udp_mib {
	unsigned long	mibs[UDP_MIB_MAX];
};

/* Linux */
#define LINUX_MIB_MAX	__LINUX_MIB_MAX
struct linux_mib {
	unsigned long	mibs[LINUX_MIB_MAX];
};

/* Linux Xfrm */
#define LINUX_MIB_XFRMMAX	__LINUX_MIB_XFRMMAX
struct linux_xfrm_mib {
	unsigned long	mibs[LINUX_MIB_XFRMMAX];
};

#define DEFINE_SNMP_STAT(type, name)	\
	__typeof__(type) __percpu *name
#define DEFINE_SNMP_STAT_ATOMIC(type, name)	\
	__typeof__(type) *name
#define DECLARE_SNMP_STAT(type, name)	\
	extern __typeof__(type) __percpu *name

#define SNMP_INC_STATS_BH(mib, field)	\
			__this_cpu_inc(mib->mibs[field])

#define SNMP_INC_STATS_USER(mib, field)	\
			this_cpu_inc(mib->mibs[field])

#define SNMP_INC_STATS_ATOMIC_LONG(mib, field)	\
			atomic_long_inc(&mib->mibs[field])

#define SNMP_INC_STATS(mib, field)	\
			this_cpu_inc(mib->mibs[field])

#define SNMP_DEC_STATS(mib, field)	\
			this_cpu_dec(mib->mibs[field])

#define SNMP_ADD_STATS_BH(mib, field, addend)	\
			__this_cpu_add(mib->mibs[field], addend)

#define SNMP_ADD_STATS_USER(mib, field, addend)	\
			this_cpu_add(mib->mibs[field], addend)

#define SNMP_ADD_STATS(mib, field, addend)	\
			this_cpu_add(mib->mibs[field], addend)
#define SNMP_UPD_PO_STATS(mib, basefield, addend)	\
	do { \
		__typeof__((mib->mibs) + 0) ptr = mib->mibs;	\
		this_cpu_inc(ptr[basefield##PKTS]);		\
		this_cpu_add(ptr[basefield##OCTETS], addend);	\
	} while (0)
#define SNMP_UPD_PO_STATS_BH(mib, basefield, addend)	\
	do { \
		__typeof__((mib->mibs) + 0) ptr = mib->mibs;	\
		__this_cpu_inc(ptr[basefield##PKTS]);		\
		__this_cpu_add(ptr[basefield##OCTETS], addend);	\
	} while (0)


#if BITS_PER_LONG==32

#define SNMP_ADD_STATS64_BH(mib, field, addend) 			\
	do {								\
		__typeof__(*mib) *ptr = raw_cpu_ptr(mib);		\
		u64_stats_update_begin(&ptr->syncp);			\
		ptr->mibs[field] += addend;				\
		u64_stats_update_end(&ptr->syncp);			\
	} while (0)

#define SNMP_ADD_STATS64_USER(mib, field, addend) 			\
	do {								\
		local_bh_disable();					\
		SNMP_ADD_STATS64_BH(mib, field, addend);		\
		local_bh_enable();					\
	} while (0)

#define SNMP_ADD_STATS64(mib, field, addend)				\
		SNMP_ADD_STATS64_USER(mib, field, addend)

#define SNMP_INC_STATS64_BH(mib, field) SNMP_ADD_STATS64_BH(mib, field, 1)
#define SNMP_INC_STATS64_USER(mib, field) SNMP_ADD_STATS64_USER(mib, field, 1)
#define SNMP_INC_STATS64(mib, field) SNMP_ADD_STATS64(mib, field, 1)
#define SNMP_UPD_PO_STATS64_BH(mib, basefield, addend)			\
	do {								\
		__typeof__(*mib) *ptr;				\
		ptr = raw_cpu_ptr((mib));				\
		u64_stats_update_begin(&ptr->syncp);			\
		ptr->mibs[basefield##PKTS]++;				\
		ptr->mibs[basefield##OCTETS] += addend;			\
		u64_stats_update_end(&ptr->syncp);			\
	} while (0)
#define SNMP_UPD_PO_STATS64(mib, basefield, addend)			\
	do {								\
		local_bh_disable();					\
		SNMP_UPD_PO_STATS64_BH(mib, basefield, addend);		\
		local_bh_enable();					\
	} while (0)
#else
#define SNMP_INC_STATS64_BH(mib, field)		SNMP_INC_STATS_BH(mib, field)
#define SNMP_INC_STATS64_USER(mib, field)	SNMP_INC_STATS_USER(mib, field)
#define SNMP_INC_STATS64(mib, field)		SNMP_INC_STATS(mib, field)
#define SNMP_DEC_STATS64(mib, field)		SNMP_DEC_STATS(mib, field)
#define SNMP_ADD_STATS64_BH(mib, field, addend) SNMP_ADD_STATS_BH(mib, field, addend)
#define SNMP_ADD_STATS64_USER(mib, field, addend) SNMP_ADD_STATS_USER(mib, field, addend)
#define SNMP_ADD_STATS64(mib, field, addend)	SNMP_ADD_STATS(mib, field, addend)
#define SNMP_UPD_PO_STATS64(mib, basefield, addend) SNMP_UPD_PO_STATS(mib, basefield, addend)
#define SNMP_UPD_PO_STATS64_BH(mib, basefield, addend) SNMP_UPD_PO_STATS_BH(mib, basefield, addend)
#endif

#endif
