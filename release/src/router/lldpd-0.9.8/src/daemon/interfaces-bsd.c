/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2012 Vincent Bernat <bernat@luffy.cx>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "lldpd.h"

#include <unistd.h>
#include <ifaddrs.h>
#include <errno.h>
#include <ctype.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/ioctl.h>
#include <net/bpf.h>
#include <net/if_types.h>
#include <net/if_media.h>
#include <net/if_dl.h>
#if defined HOST_OS_FREEBSD
# include <net/if_vlan_var.h>
# include <net/if_bridgevar.h>
# include <net/if_lagg.h>
#elif defined HOST_OS_DRAGONFLY
# include <net/vlan/if_vlan_var.h>
# include <net/bridge/if_bridgevar.h>
#elif defined HOST_OS_OPENBSD
# include <net/if_vlan_var.h>
# include <net/if_bridge.h>
# include <net/if_trunk.h>
#elif defined HOST_OS_NETBSD
# include <net/if_vlanvar.h>
# include <net/if_bridgevar.h>
# include <net/agr/if_agrioctl.h>
#elif defined HOST_OS_OSX
# include <osx/if_vlan_var.h>
# include <osx/if_bridgevar.h>
# include <osx/if_bond_var.h>
#endif

#ifndef IFDESCRSIZE
#define IFDESCRSIZE 64
#endif

static int
ifbsd_check_wireless(struct lldpd *cfg,
    struct ifaddrs *ifaddr,
    struct interfaces_device *iface)
{
    struct ifmediareq ifmr = {};
    strlcpy(ifmr.ifm_name, iface->name, sizeof(ifmr.ifm_name));
    if (ioctl(cfg->g_sock, SIOCGIFMEDIA, (caddr_t)&ifmr) < 0 ||
	IFM_TYPE(ifmr.ifm_current) != IFM_IEEE80211)
	    return 0;		/* Not wireless either */
    iface->type |= IFACE_WIRELESS_T | IFACE_PHYSICAL_T;
    return 0;
}

static void
ifbsd_check_bridge(struct lldpd *cfg,
    struct interfaces_device_list *interfaces,
    struct interfaces_device *master)
{
	struct ifbreq req[64];
	struct ifbifconf bifc = {
		.ifbic_len = sizeof(req),
		.ifbic_req = req
	};

#if defined HOST_OS_FREEBSD || defined HOST_OS_NETBSD || defined HOST_OS_OSX || HOST_OS_DRAGONFLY
	struct ifdrv ifd = {
		.ifd_cmd = BRDGGIFS,
		.ifd_len = sizeof(bifc),
		.ifd_data = &bifc
	};

	strlcpy(ifd.ifd_name, master->name, sizeof(ifd.ifd_name));
	if (ioctl(cfg->g_sock, SIOCGDRVSPEC, (caddr_t)&ifd) < 0) {
		log_debug("interfaces",
		    "%s is not a bridge", master->name);
		return;
	}
#elif defined HOST_OS_OPENBSD
	strlcpy(bifc.ifbic_name, master->name, sizeof(bifc.ifbic_name));
	if (ioctl(cfg->g_sock, SIOCBRDGIFS, (caddr_t)&bifc) < 0) {
		log_debug("interfaces",
		    "%s is not a bridge", master->name);
		return;
	}
#else
# error Unsupported OS
#endif
	if (bifc.ifbic_len >= sizeof(req)) {
		log_warnx("interfaces",
		    "%s is a bridge too big. Please, report the problem",
		    master->name);
		return;
	}
	for (int i = 0; i < bifc.ifbic_len / sizeof(*req); i++) {
		struct interfaces_device *slave =
		    interfaces_nametointerface(interfaces,
			req[i].ifbr_ifsname);
		if (slave == NULL) {
			log_warnx("interfaces",
			    "%s should be bridged to %s but we don't know %s",
			    req[i].ifbr_ifsname, master->name, req[i].ifbr_ifsname);
			continue;
		}
		log_debug("interfaces",
		    "%s is bridged to %s",
		    slave->name, master->name);
		slave->upper = master;
	}
	master->type |= IFACE_BRIDGE_T;
}

static void
ifbsd_check_bond(struct lldpd *cfg,
    struct interfaces_device_list *interfaces,
    struct interfaces_device *master)
{
#if defined HOST_OS_OPENBSD
/* OpenBSD is the same as FreeBSD, just lagg->trunk */
# define lagg_reqport trunk_reqport
# define lagg_reqall  trunk_reqall
# define SIOCGLAGG SIOCGTRUNK
# define LAGG_MAX_PORTS TRUNK_MAX_PORTS
#endif
#if defined HOST_OS_OPENBSD || defined HOST_OS_FREEBSD
	struct lagg_reqport rpbuf[LAGG_MAX_PORTS];
	struct lagg_reqall ra = {
		.ra_size = sizeof(rpbuf),
		.ra_port = rpbuf
	};
	strlcpy(ra.ra_ifname, master->name, IFNAMSIZ);
	if (ioctl(cfg->g_sock, SIOCGLAGG, (caddr_t)&ra) < 0) {
		log_debug("interfaces",
		    "%s is not a bond", master->name);
		return;
	}

	for (int i = 0; i < ra.ra_ports; i++) {
		struct interfaces_device *slave;
		slave = interfaces_nametointerface(interfaces,
		    rpbuf[i].rp_portname);
		if (slave == NULL) {
			log_warnx("interfaces",
			    "%s should be enslaved to %s but we don't know %s",
			    rpbuf[i].rp_portname, master->name,
			    rpbuf[i].rp_portname);
			continue;
		}
		log_debug("interfaces",
		    "%s is enslaved to bond %s",
		    slave->name, master->name);
		slave->upper = master;
	}
	master->type |= IFACE_BOND_T;
#elif defined HOST_OS_NETBSD
	/* No max, we consider a maximum of 24 ports */
	char buf[sizeof(struct agrportinfo)*24] = {};
	size_t buflen = sizeof(buf);
	struct agrreq ar = {
		.ar_version = AGRREQ_VERSION,
		.ar_cmd = AGRCMD_PORTLIST,
		.ar_buf = buf,
		.ar_buflen = buflen
	};
	struct ifreq ifr = {
		.ifr_data = &ar
	};
	struct agrportlist *apl = (void *)buf;
	struct agrportinfo *api = (void *)(apl + 1);
	strlcpy(ifr.ifr_name, master->name, sizeof(ifr.ifr_name));
	if (ioctl(cfg->g_sock, SIOCGETAGR, &ifr) == -1) {
		if (errno == E2BIG) {
			log_warnx("interfaces",
			    "%s is a too big aggregate. Please, report the problem",
			    master->name);
		} else {
			log_debug("interfaces",
			    "%s is not an aggregate", master->name);
		}
		return;
	}
	for (int i = 0; i < apl->apl_nports; i++, api++) {
		struct interfaces_device *slave;
		slave = interfaces_nametointerface(interfaces,
		    api->api_ifname);
		if (slave == NULL) {
			log_warnx("interfaces",
			    "%s should be enslaved to %s but we don't know %s",
			    api->api_ifname, master->name, api->api_ifname);
			continue;
		}
		log_debug("interfaces",
		    "%s is enslaved to bond %s",
		    slave->name, master->name);
		slave->upper = master;
	}
	master->type |= IFACE_BOND_T;
#elif defined HOST_OS_OSX
	struct if_bond_req ibr = {
		.ibr_op = IF_BOND_OP_GET_STATUS,
		.ibr_ibru = {
			.ibru_status = { .ibsr_version = IF_BOND_STATUS_REQ_VERSION }
		}
	};
	struct ifreq ifr = {
		.ifr_data = (caddr_t)&ibr
	};
	strlcpy(ifr.ifr_name, master->name, sizeof(ifr.ifr_name));
	if (ioctl(cfg->g_sock, SIOCGIFBOND, (caddr_t)&ifr) < 0) {
		log_debug("interfaces",
		    "%s is not an aggregate", master->name);
		return;
	}
	master->type |= IFACE_BOND_T;
	if (ibr.ibr_ibru.ibru_status.ibsr_total == 0) {
		log_debug("interfaces", "no members for bond %s",
		    master->name);
		return;
	}

	struct if_bond_status_req *ibsr_p = &ibr.ibr_ibru.ibru_status;
	ibsr_p->ibsr_buffer =
	    malloc(sizeof(struct if_bond_status)*ibsr_p->ibsr_total);
	if (ibsr_p->ibsr_buffer == NULL) {
		log_warnx("interfaces", "not enough memory to check bond members");
		return;
	}
	ibsr_p->ibsr_count = ibsr_p->ibsr_total;
	if (ioctl(cfg->g_sock, SIOCGIFBOND, (caddr_t)&ifr) < 0) {
		log_warn("interfaces",
		    "unable to get members for bond %s", master->name);
		goto end;
	}

	struct if_bond_status *ibs_p = (struct if_bond_status *)ibsr_p->ibsr_buffer;
	for (int i = 0; i < ibsr_p->ibsr_total; i++, ibs_p++) {
		struct interfaces_device *slave;
		slave = interfaces_nametointerface(interfaces,
		    ibs_p->ibs_if_name);
		if (slave == NULL) {
			log_warnx("interfaces",
			    "%s should be enslaved to %s but we don't know %s",
			    ibs_p->ibs_if_name, master->name, ibs_p->ibs_if_name);
			continue;
		}
		log_debug("interfaces", "%s is enslaved to bond %s",
		    slave->name, master->name);
		slave->upper = master;
	}
end:
	free(ibsr_p->ibsr_buffer);
#elif defined HOST_OS_DRAGONFLY
	log_debug("interfaces", "DragonFly BSD does not support link aggregation");
#else
# error Unsupported OS
#endif
}

static void
ifbsd_check_vlan(struct lldpd *cfg,
    struct interfaces_device_list *interfaces,
    struct interfaces_device *vlan)
{
	struct interfaces_device *lower;
	struct vlanreq vreq = {};
	struct ifreq ifr = {
		.ifr_data = (caddr_t)&vreq
	};
	strlcpy(ifr.ifr_name, vlan->name, sizeof(ifr.ifr_name));
	if (ioctl(cfg->g_sock, SIOCGETVLAN, (caddr_t)&ifr) < 0) {
		log_debug("interfaces",
		    "%s is not a VLAN", vlan->name);
		return;
	}
	if (strlen(vreq.vlr_parent) == 0) {
		log_debug("interfaces",
		    "%s is a VLAN but has no lower interface",
		    vlan->name);
		vlan->lower = NULL;
		vlan->type |= IFACE_VLAN_T;
		return;
	}
	lower = interfaces_nametointerface(interfaces,
	    vreq.vlr_parent);
	if (lower == NULL) {
		log_warnx("interfaces",
		    "%s should be a VLAN of %s but %s does not exist",
		    vlan->name, vreq.vlr_parent, vreq.vlr_parent);
		return;
	}
	log_debug("interfaces",
	    "%s is VLAN %d of %s",
	    vlan->name, vreq.vlr_tag, lower->name);
	vlan->lower = lower;
	vlan->vlanid = vreq.vlr_tag;
	vlan->type |= IFACE_VLAN_T;
}

static void
ifbsd_check_physical(struct lldpd *cfg,
    struct interfaces_device_list *interfaces,
    struct interfaces_device *iface)
{
	if (iface->type & (IFACE_VLAN_T|
		IFACE_BOND_T|IFACE_BRIDGE_T|IFACE_PHYSICAL_T))
		return;

	if (!(iface->flags & (IFF_MULTICAST|IFF_BROADCAST))) {
		log_debug("interfaces", "skip %s: not able to do multicast nor broadcast",
		    iface->name);
		return;
	}
	log_debug("interfaces",
	    "%s is a physical interface",
	    iface->name);
	iface->type |= IFACE_PHYSICAL_T;
}

/* Blacklist any dangerous interface. Currently, only p2p0 is blacklisted as it
 * triggers some AirDrop functionality when we send something on it.
 *  See: https://github.com/vincentbernat/lldpd/issues/61
 */
static void
ifbsd_blacklist(struct lldpd *cfg,
    struct interfaces_device_list *interfaces)
{
#ifdef HOST_OS_OSX
	struct interfaces_device *iface = NULL;
	TAILQ_FOREACH(iface, interfaces, next) {
		int i;
		if (strncmp(iface->name, "p2p", 3)) continue;
		if (strlen(iface->name) < 4) continue;
		for (i = 3;
		     iface->name[i] != '\0' && isdigit(iface->name[i]);
		     i++);
		if (iface->name[i] == '\0') {
			log_debug("interfaces", "skip %s: AirDrop interface",
			    iface->name);
			iface->ignore = 1;
		}
	}
#endif
}

static struct interfaces_device*
ifbsd_extract_device(struct lldpd *cfg,
    struct ifaddrs *ifaddr)
{
	struct interfaces_device *iface = NULL;
	struct sockaddr_dl *saddrdl = ALIGNED_CAST(struct sockaddr_dl*, ifaddr->ifa_addr);
	if ((saddrdl->sdl_type != IFT_BRIDGE) &&
	    (saddrdl->sdl_type != IFT_L2VLAN) &&
	    (saddrdl->sdl_type != IFT_ETHER)) {
		log_debug("interfaces", "skip %s: not an ethernet device (%d)",
		    ifaddr->ifa_name, saddrdl->sdl_type);
		return NULL;
	}
	if ((iface = calloc(1, sizeof(struct interfaces_device))) == NULL) {
		log_warn("interfaces", "unable to allocate memory for %s",
		    ifaddr->ifa_name);
		return NULL;
	}

	iface->index = saddrdl->sdl_index;
	iface->name = strdup(ifaddr->ifa_name);
	iface->flags = ifaddr->ifa_flags;

	/* MAC address */
	iface->address = malloc(ETHER_ADDR_LEN);
	if (iface->address)
		memcpy(iface->address, LLADDR(saddrdl), ETHER_ADDR_LEN);

	/* Grab description */
#ifdef SIOCGIFDESCR
#if defined HOST_OS_FREEBSD || defined HOST_OS_OPENBSD
	iface->alias = malloc(IFDESCRSIZE);
	if (iface->alias) {
#if defined HOST_OS_FREEBSD
		struct ifreq ifr = {
			.ifr_buffer = { .buffer = iface->alias,
					.length = IFDESCRSIZE }
		};
#else
		struct ifreq ifr = {
			.ifr_data = (caddr_t)iface->alias
		};
#endif
		strlcpy(ifr.ifr_name, ifaddr->ifa_name, sizeof(ifr.ifr_name));
		if (ioctl(cfg->g_sock, SIOCGIFDESCR, (caddr_t)&ifr) < 0) {
			free(iface->alias);
			iface->alias = NULL;
		}
	}
#endif
#endif /* SIOCGIFDESCR */

	if (ifbsd_check_wireless(cfg, ifaddr, iface) == -1) {
		interfaces_free_device(iface);
		return NULL;
	}

	return iface;
}

static void
ifbsd_extract(struct lldpd *cfg,
    struct interfaces_device_list *interfaces,
    struct interfaces_address_list *addresses,
    struct ifaddrs *ifaddr)
{
	struct interfaces_address *address = NULL;
	struct interfaces_device *device = NULL;
	if (!ifaddr->ifa_name) return;
	if (!ifaddr->ifa_addr) return;
	switch (ifaddr->ifa_addr->sa_family) {
	case AF_LINK:
		log_debug("interfaces",
		    "grabbing information on interface %s",
		    ifaddr->ifa_name);
		device = ifbsd_extract_device(cfg, ifaddr);
		if (device)
			TAILQ_INSERT_TAIL(interfaces, device, next);
		break;
	case AF_INET:
	case AF_INET6:
		log_debug("interfaces",
		    "got an IP address on %s",
		    ifaddr->ifa_name);
		address = malloc(sizeof(struct interfaces_address));
		if (address == NULL) {
			log_warn("interfaces",
			    "not enough memory for a new IP address on %s",
			    ifaddr->ifa_name);
			return;
		}
		address->flags = ifaddr->ifa_flags;
		address->index = if_nametoindex(ifaddr->ifa_name);
		memcpy(&address->address,
		    ifaddr->ifa_addr,
		    (ifaddr->ifa_addr->sa_family == AF_INET)?
		    sizeof(struct sockaddr_in):
		    sizeof(struct sockaddr_in6));
		TAILQ_INSERT_TAIL(addresses, address, next);
		break;
	default:
	    log_debug("interfaces", "unhandled family %d for interface %s",
		ifaddr->ifa_addr->sa_family,
		ifaddr->ifa_name);
	}
}

static void
ifbsd_macphy(struct lldpd *cfg,
    struct lldpd_hardware *hardware)
{
#ifdef ENABLE_DOT3
	struct ifmediareq ifmr = {};
#ifdef HAVE_TYPEOF
	typeof(ifmr.ifm_ulist[0]) media_list[32] = {};
#else
	int media_list[32] = {};
#endif
	ifmr.ifm_ulist = media_list;
	ifmr.ifm_count = 32;
	struct lldpd_port *port = &hardware->h_lport;
	unsigned int duplex;
	unsigned int media;
	int advertised_ifmedia_to_rfc3636[][3] = {
		{IFM_10_T,
		 LLDP_DOT3_LINK_AUTONEG_10BASE_T,
		 LLDP_DOT3_LINK_AUTONEG_10BASET_FD},
		{IFM_10_STP,
		 LLDP_DOT3_LINK_AUTONEG_10BASE_T,
		 LLDP_DOT3_LINK_AUTONEG_10BASET_FD},
		{IFM_100_TX,
		 LLDP_DOT3_LINK_AUTONEG_100BASE_TX,
		 LLDP_DOT3_LINK_AUTONEG_100BASE_TXFD},
		{IFM_100_T4,
		 LLDP_DOT3_LINK_AUTONEG_100BASE_T4,
		 LLDP_DOT3_LINK_AUTONEG_100BASE_T4},
		{IFM_100_T2,
		 LLDP_DOT3_LINK_AUTONEG_100BASE_T2,
		 LLDP_DOT3_LINK_AUTONEG_100BASE_T2FD},
		{IFM_1000_SX,
		 LLDP_DOT3_LINK_AUTONEG_1000BASE_X,
		 LLDP_DOT3_LINK_AUTONEG_1000BASE_XFD},
		{IFM_1000_LX,
		 LLDP_DOT3_LINK_AUTONEG_1000BASE_X,
		 LLDP_DOT3_LINK_AUTONEG_1000BASE_XFD},
		{IFM_1000_CX,
		 LLDP_DOT3_LINK_AUTONEG_1000BASE_X,
		 LLDP_DOT3_LINK_AUTONEG_1000BASE_XFD},
		{IFM_1000_T,
		 LLDP_DOT3_LINK_AUTONEG_1000BASE_T,
		 LLDP_DOT3_LINK_AUTONEG_1000BASE_TFD},
		{0, 0, 0}
	};
	int current_ifmedia_to_rfc3636[][3] = {
		{IFM_10_T,
		 LLDP_DOT3_MAU_10BASETHD, LLDP_DOT3_MAU_10BASETFD},
		{IFM_10_STP,
		 LLDP_DOT3_MAU_10BASETHD, LLDP_DOT3_MAU_10BASETFD},
		{IFM_10_2,
		 LLDP_DOT3_MAU_10BASE2, LLDP_DOT3_MAU_10BASE2},
		{IFM_10_5,
		 LLDP_DOT3_MAU_10BASE5, LLDP_DOT3_MAU_10BASE5},
		{IFM_100_TX,
		 LLDP_DOT3_MAU_100BASETXHD, LLDP_DOT3_MAU_100BASETXFD},
		{IFM_100_FX,
		 LLDP_DOT3_MAU_100BASEFXHD, LLDP_DOT3_MAU_100BASEFXFD},
		{IFM_100_T2,
		 LLDP_DOT3_MAU_100BASET2HD, LLDP_DOT3_MAU_100BASET2FD},
		{IFM_1000_SX,
		 LLDP_DOT3_MAU_1000BASESXHD, LLDP_DOT3_MAU_1000BASESXFD},
		{IFM_10_FL,
		 LLDP_DOT3_MAU_10BASEFLHD, LLDP_DOT3_MAU_10BASEFLFD },
		{IFM_1000_LX,
		 LLDP_DOT3_MAU_1000BASELXHD, LLDP_DOT3_MAU_1000BASELXFD},
		{IFM_1000_CX,
		 LLDP_DOT3_MAU_1000BASECXHD, LLDP_DOT3_MAU_1000BASECXFD},
		{IFM_1000_T,
		 LLDP_DOT3_MAU_1000BASETHD, LLDP_DOT3_MAU_1000BASETFD },
		{IFM_10G_LR,
		 LLDP_DOT3_MAU_10GIGBASELR, LLDP_DOT3_MAU_10GIGBASELR},
		{IFM_10G_SR,
		 LLDP_DOT3_MAU_10GIGBASESR, LLDP_DOT3_MAU_10GIGBASESR},
		{IFM_10G_CX4,
		 LLDP_DOT3_MAU_10GIGBASELX4, LLDP_DOT3_MAU_10GIGBASELX4},
#ifdef IFM_10G_T
		{IFM_10G_T,
		 LLDP_DOT3_MAU_10GIGBASECX4, LLDP_DOT3_MAU_10GIGBASECX4},
#endif
#ifdef IFM_10G_TWINAX
		{IFM_10G_TWINAX,
		 LLDP_DOT3_MAU_10GIGBASECX4, LLDP_DOT3_MAU_10GIGBASECX4},
#endif
#ifdef IFM_10G_TWINAX_LONG
		{IFM_10G_TWINAX_LONG,
		 LLDP_DOT3_MAU_10GIGBASECX4, LLDP_DOT3_MAU_10GIGBASECX4},
#endif
#ifdef IFM_10G_LRM
		{IFM_10G_LRM,
		 LLDP_DOT3_MAU_10GIGBASELR, LLDP_DOT3_MAU_10GIGBASELR},
#endif
#ifdef IFM_10G_SFP_CU
		{IFM_10G_SFP_CU,
		 LLDP_DOT3_MAU_10GIGBASECX4, LLDP_DOT3_MAU_10GIGBASECX4},
#endif
		{0, 0, 0}
	};

	log_debug("interfaces", "get MAC/phy for %s",
	    hardware->h_ifname);
	strlcpy(ifmr.ifm_name, hardware->h_ifname, sizeof(ifmr.ifm_name));
	if (ioctl(cfg->g_sock, SIOCGIFMEDIA, (caddr_t)&ifmr) < 0) {
		log_debug("interfaces",
		    "unable to get media information from %s",
		    hardware->h_ifname);
		return;
	}
	if (IFM_TYPE(ifmr.ifm_current) != IFM_ETHER) {
		log_warnx("interfaces",
		    "cannot get media information from %s: not an ethernet device",
		    hardware->h_ifname);
		return;
	}
	if ((ifmr.ifm_status & IFM_ACTIVE) == 0) {
		log_debug("interfaces",
		    "interface %s is now down, skip",
		    hardware->h_ifname);
		return;
	}
	if (ifmr.ifm_count == 0) {
		log_warnx("interfaces", "no media information available on %s",
		    hardware->h_ifname);
		return;
	}
	port->p_macphy.autoneg_support =
	    port->p_macphy.autoneg_enabled = 0;
	for (int m = 0; m < ifmr.ifm_count; m++) {
		media = IFM_SUBTYPE(ifmr.ifm_ulist[m]);
		duplex = !!(IFM_OPTIONS(ifmr.ifm_ulist[m]) &
		    IFM_FDX);
		if (media == IFM_AUTO) {
			port->p_macphy.autoneg_support = 1;
			port->p_macphy.autoneg_enabled =
			    (IFM_SUBTYPE(ifmr.ifm_current) == IFM_AUTO);
			continue;
		}

		int found = 0;
		for (int j = 0; advertised_ifmedia_to_rfc3636[j][0]; j++) {
			if (advertised_ifmedia_to_rfc3636[j][0] == media) {
				port->p_macphy.autoneg_advertised |=
				    advertised_ifmedia_to_rfc3636[j][1 + duplex];
				found = 1;
				break;
			}
		}
		if (!found) port->p_macphy.autoneg_advertised |= \
				LLDP_DOT3_LINK_AUTONEG_OTHER;
	}

	port->p_macphy.mau_type = 0;
	media = IFM_SUBTYPE(ifmr.ifm_active);
	duplex = !!(IFM_OPTIONS(ifmr.ifm_active) & IFM_FDX);
	for (int j = 0; current_ifmedia_to_rfc3636[j][0]; j++) {
		if (current_ifmedia_to_rfc3636[j][0] == media) {
			port->p_macphy.mau_type =
			    current_ifmedia_to_rfc3636[j][1 + duplex];
			break;
		}
	}
#endif
}

extern struct lldpd_ops bpf_ops;
void
interfaces_update(struct lldpd *cfg)
{
	struct lldpd_hardware *hardware;
	struct interfaces_device *iface;
	struct interfaces_device_list *interfaces;
	struct interfaces_address_list *addresses;
	struct ifaddrs *ifaddrs = NULL, *ifaddr;

	interfaces = malloc(sizeof(struct interfaces_device_list));
	addresses = malloc(sizeof(struct interfaces_address_list));
	if (interfaces == NULL || addresses == NULL) {
		log_warnx("interfaces", "unable to allocate memory");
		goto end;
	}
	TAILQ_INIT(interfaces);
	TAILQ_INIT(addresses);
	if (getifaddrs(&ifaddrs) < 0) {
		log_warnx("interfaces", "unable to get list of interfaces");
		goto end;
	}

	for (ifaddr = ifaddrs;
	     ifaddr != NULL;
	     ifaddr = ifaddr->ifa_next) {
		ifbsd_extract(cfg, interfaces, addresses, ifaddr);
	}
	/* Link interfaces together if needed */
	TAILQ_FOREACH(iface, interfaces, next) {
		ifbsd_check_bridge(cfg, interfaces, iface);
		ifbsd_check_bond(cfg, interfaces, iface);
		ifbsd_check_vlan(cfg, interfaces, iface);
		ifbsd_check_physical(cfg, interfaces, iface);
	}

	ifbsd_blacklist(cfg, interfaces);
	interfaces_helper_whitelist(cfg, interfaces);
	interfaces_helper_physical(cfg, interfaces,
	    &bpf_ops, ifbpf_phys_init);
#ifdef ENABLE_DOT1
	interfaces_helper_vlan(cfg, interfaces);
#endif
	interfaces_helper_mgmt(cfg, addresses);
	interfaces_helper_chassis(cfg, interfaces);

	/* Mac/PHY */
	TAILQ_FOREACH(hardware, &cfg->g_hardware, h_entries) {
		if (!hardware->h_flags) continue;
		ifbsd_macphy(cfg, hardware);
		interfaces_helper_promisc(cfg, hardware);
	}

	if (cfg->g_iface_event == NULL) {
		int s;
		log_debug("interfaces", "subscribe to route socket notifications");
		if ((s = socket(PF_ROUTE, SOCK_RAW, 0)) < 0) {
			log_warn("interfaces", "unable to open route socket");
			goto end;
		}

#ifdef ROUTE_MSGFILTER
		unsigned int rtfilter;
		rtfilter = ROUTE_FILTER(RTM_IFINFO);
		if (setsockopt(s, PF_ROUTE, ROUTE_MSGFILTER,
			&rtfilter, sizeof(rtfilter)) == -1)
			log_warn("interfaces", "unable to set filter for interface updates");
#endif

		if (levent_iface_subscribe(cfg, s) == -1)
			close(s);
	}

end:
	interfaces_free_devices(interfaces);
	interfaces_free_addresses(addresses);
	if (ifaddrs) freeifaddrs(ifaddrs);
}

void
interfaces_cleanup(struct lldpd *cfg)
{
}
