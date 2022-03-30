#ifndef _LINUX_NDI_H
#define _LINUX_NDI_H

/* 256 was chosen as the max length of a hostname in a DHCP packet is 255. */
#define NDI_HOSTNAME_MAX_LEN		256

#if defined(CONFIG_BCM_KF_NDI)
#include <linux/if_ether.h>
#include <linux/list.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/dpi.h>

struct ndi_dev {
	u32			id;
	struct in_addr		ip4;
	struct in6_addr		ip6;
	u8			mac[ETH_ALEN];
	char			hostname[NDI_HOSTNAME_MAX_LEN];
	struct dpi_dev		dpi;

	atomic_t		refcount;
	struct net_device	*local_dev; /* the interface at which the
	                                       ndi_dev is detected */
	u8			state;
	u8			probe_count;
	struct hlist_node	node;
};

struct ndi_info {
	struct ndi_dev		*dev;
};

#endif /* CONFIG_BCM_KF_NDI */

enum {
	NDINLGRP_NONE,
	NDINLGRP_DEV,

	__NDINLGRP_MAX
#define NDINLGRP_MAX		(__NDINLGRP_MAX - 1)
};

enum {
	NDINL_BASE = 16,

	NDINL_NEWDEVICE = 16,
	NDINL_DELDEVICE,
	NDINL_GETDEVICE,
	NDINL_SETDEVICE,

	__NDINL_MAX,
#define NDINL_MAX		(__NDINL_MAX - 1)
};

/*
 * The following describe the netlink attributes used by NDI when
 * transferring data to/from userspace.
 */
enum {
	NDIA_DEV_UNSPEC,
	NDIA_DEV_ID,
	NDIA_DEV_IP4,
	NDIA_DEV_IP6,
	NDIA_DEV_MAC,
	NDIA_DEV_HOSTNAME,
	NDIA_DEV_ONLINE,
	/* dpi fields */
	NDIA_DEV_DPI_VENDOR,
	NDIA_DEV_DPI_OS,
	NDIA_DEV_DPI_OS_CLASS,
	NDIA_DEV_DPI_ID,
	NDIA_DEV_DPI_CATEGORY,
	NDIA_DEV_DPI_FAMILY,
	NDIA_DEV_DPI_PRIO,
	__NDIA_DEV_MAX
};
#define NDIA_DEV_MAX	(__NDIA_DEV_MAX - 1)

#endif /* _LINUX_NDI_H */

