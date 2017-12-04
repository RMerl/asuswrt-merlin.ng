/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2008 Vincent Bernat <bernat@luffy.cx>
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

#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include <errno.h>
#include <sys/ioctl.h>
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#endif
#include <linux/if_vlan.h>
#include <linux/if_bonding.h>
#include <linux/if_bridge.h>
#include <linux/wireless.h>
#include <linux/sockios.h>
#include <linux/if_packet.h>
#include <linux/ethtool.h>
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#define SYSFS_PATH_MAX 256
#define MAX_PORTS 1024
#define MAX_BRIDGES 1024

static int
iflinux_eth_init(struct lldpd *cfg, struct lldpd_hardware *hardware)
{
	int fd;

	log_debug("interfaces", "initialize ethernet device %s",
	    hardware->h_ifname);
	if ((fd = priv_iface_init(hardware->h_ifindex, hardware->h_ifname)) == -1)
		return -1;
	hardware->h_sendfd = fd; /* Send */

	interfaces_setup_multicast(cfg, hardware->h_ifname, 0);

	levent_hardware_add_fd(hardware, fd); /* Receive */
	log_debug("interfaces", "interface %s initialized (fd=%d)", hardware->h_ifname,
	    fd);
	return 0;
}

/* Generic ethernet send/receive */
static int
iflinux_eth_send(struct lldpd *cfg, struct lldpd_hardware *hardware,
    char *buffer, size_t size)
{
	log_debug("interfaces", "send PDU to ethernet device %s (fd=%d)",
	    hardware->h_ifname, hardware->h_sendfd);
	return write(hardware->h_sendfd,
	    buffer, size);
}

static void
iflinux_error_recv(struct lldpd_hardware *hardware, int fd)
{
	do {
		ssize_t n;
		char buf[1024] = {};
		struct msghdr msg = {
			.msg_control = buf,
			.msg_controllen = sizeof(buf)
		};
		if ((n = recvmsg(fd, &msg, MSG_ERRQUEUE)) <= 0) {
			return;
		}
		struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
		if (cmsg == NULL)
			log_warnx("interfaces", "received unknown error on %s",
			    hardware->h_ifname);
		else
			log_warnx("interfaces", "received error (level=%d/type=%d) on %s",
			    cmsg->cmsg_level, cmsg->cmsg_type, hardware->h_ifname);
	} while (1);
}

static int
iflinux_generic_recv(struct lldpd_hardware *hardware,
    int fd, char *buffer, size_t size,
    struct sockaddr_ll *from)
{
	int n, retry = 0;
	socklen_t fromlen;

retry:
	fromlen = sizeof(*from);
	memset(from, 0, fromlen);
	if ((n = recvfrom(fd, buffer, size, 0,
		    (struct sockaddr *)from,
		    &fromlen)) == -1) {
		if (errno == EAGAIN && retry == 0) {
			/* There may be an error queued in the socket. Clear it and retry. */
			iflinux_error_recv(hardware, fd);
			retry++;
			goto retry;
		}
		if (errno == ENETDOWN) {
			log_debug("interfaces", "error while receiving frame on %s (network down)",
			    hardware->h_ifname);
		} else {
			log_warn("interfaces", "error while receiving frame on %s (retry: %d)",
			    hardware->h_ifname, retry);
			hardware->h_rx_discarded_cnt++;
		}
		return -1;
	}
	if (from->sll_pkttype == PACKET_OUTGOING)
		return -1;
	return n;
}

static int
iflinux_eth_recv(struct lldpd *cfg, struct lldpd_hardware *hardware,
    int fd, char *buffer, size_t size)
{
	int n;
	struct sockaddr_ll from;

	log_debug("interfaces", "receive PDU from ethernet device %s",
	    hardware->h_ifname);
	if ((n = iflinux_generic_recv(hardware, fd, buffer, size, &from)) == -1)
		return -1;
	return n;
}

static int
iflinux_eth_close(struct lldpd *cfg, struct lldpd_hardware *hardware)
{
	log_debug("interfaces", "close ethernet device %s",
	    hardware->h_ifname);
	interfaces_setup_multicast(cfg, hardware->h_ifname, 1);
	return 0;
}

static struct lldpd_ops eth_ops = {
	.send = iflinux_eth_send,
	.recv = iflinux_eth_recv,
	.cleanup = iflinux_eth_close,
};

static int
iflinux_is_bridge(struct lldpd *cfg,
    struct interfaces_device_list *interfaces,
    struct interfaces_device *iface)
{
#ifdef ENABLE_OLDIES
	struct interfaces_device *port;
	char path[SYSFS_PATH_MAX];
	int f;

	if ((snprintf(path, SYSFS_PATH_MAX,
		    SYSFS_CLASS_NET "%s/" SYSFS_BRIDGE_FDB,
		    iface->name)) >= SYSFS_PATH_MAX)
		log_warnx("interfaces", "path truncated");
	if ((f = priv_open(path)) < 0)
		return 0;
	close(f);

	/* Also grab all ports */
	TAILQ_FOREACH(port, interfaces, next) {
		if (port->upper) continue;
		if (snprintf(path, SYSFS_PATH_MAX,
			SYSFS_CLASS_NET "%s/" SYSFS_BRIDGE_PORT_SUBDIR "/%s/port_no",
			iface->name, port->name) >= SYSFS_PATH_MAX)
			log_warnx("interfaces", "path truncated");
		if ((f = priv_open(path)) < 0)
			continue;
		log_debug("interfaces",
		    "port %s is bridged to %s",
		    port->name, iface->name);
		port->upper = iface;
		close(f);
	}

	return 1;
#else
	return 0;
#endif
}

static int
iflinux_is_vlan(struct lldpd *cfg,
    struct interfaces_device_list *interfaces,
    struct interfaces_device *iface)
{
#ifdef ENABLE_OLDIES
	struct vlan_ioctl_args ifv = {};
	ifv.cmd = GET_VLAN_REALDEV_NAME_CMD;
	strlcpy(ifv.device1, iface->name, sizeof(ifv.device1));
	if (ioctl(cfg->g_sock, SIOCGIFVLAN, &ifv) >= 0) {
		/* This is a VLAN, get the lower interface and the VID */
		struct interfaces_device *lower =
		    interfaces_nametointerface(interfaces, ifv.u.device2);
		if (!lower) {
			log_debug("interfaces",
			    "unable to find lower interface for VLAN %s",
			    iface->name);
			return 0;
		}

		memset(&ifv, 0, sizeof(ifv));
		ifv.cmd = GET_VLAN_VID_CMD;
		strlcpy(ifv.device1, iface->name, sizeof(ifv.device1));
		if (ioctl(cfg->g_sock, SIOCGIFVLAN, &ifv) < 0) {
			log_debug("interfaces",
			    "unable to find VID for VLAN %s",
			    iface->name);
			return 0;
		}

		iface->lower = lower;
		iface->vlanid = ifv.u.VID;
		return 1;
	}
#endif
	return 0;
}

static int
iflinux_is_bond(struct lldpd *cfg,
    struct interfaces_device_list *interfaces,
    struct interfaces_device *master)
{
#ifdef ENABLE_OLDIES
	/* Shortcut if we detect the new team driver. Upper and lower links
	 * should already be set with netlink in this case.  */
	if (master->driver && !strcmp(master->driver, "team")) {
		return 1;
	}

	struct ifreq ifr = {};
	struct ifbond ifb = {};
	strlcpy(ifr.ifr_name, master->name, sizeof(ifr.ifr_name));
	ifr.ifr_data = (char *)&ifb;
	if (ioctl(cfg->g_sock, SIOCBONDINFOQUERY, &ifr) >= 0) {
		while (ifb.num_slaves--) {
			struct ifslave ifs;
			memset(&ifr, 0, sizeof(ifr));
			memset(&ifs, 0, sizeof(ifs));
			strlcpy(ifr.ifr_name, master->name, sizeof(ifr.ifr_name));
			ifr.ifr_data = (char *)&ifs;
			ifs.slave_id = ifb.num_slaves;
			if (ioctl(cfg->g_sock, SIOCBONDSLAVEINFOQUERY, &ifr) >= 0) {
				struct interfaces_device *slave =
				    interfaces_nametointerface(interfaces,
					ifs.slave_name);
				if (slave == NULL) continue;
				if (slave->upper) continue;
				log_debug("interfaces",
				    "interface %s is enslaved to %s",
				    slave->name, master->name);
				slave->upper = master;
			}
		}
		return 1;
	}
#endif
	return 0;
}

/**
 * Get permanent MAC from ethtool.
 *
 * Return 0 on success, -1 on error.
 */
static int
iflinux_get_permanent_mac_ethtool(struct lldpd *cfg,
    struct interfaces_device_list *interfaces,
    struct interfaces_device *iface)
{
	struct ifreq ifr = {};
	union {
		struct ethtool_perm_addr addr;
		/* cppcheck-suppress unusedStructMember */
		char u8[sizeof(struct ethtool_perm_addr) + ETHER_ADDR_LEN];
	} epaddr;

	strlcpy(ifr.ifr_name, iface->name, sizeof(ifr.ifr_name));
	epaddr.addr.cmd = ETHTOOL_GPERMADDR;
	epaddr.addr.size = ETHER_ADDR_LEN;
	ifr.ifr_data = (caddr_t)&epaddr.addr;
	if (ioctl(cfg->g_sock, SIOCETHTOOL, &ifr) == -1) {
		static int once = 0;
		if (errno == EPERM && !once) {
			log_warn("interfaces",
			    "no permission to get permanent MAC address for %s (requires 2.6.19+)",
			    iface->name);
			once = 1;
			return -1;
		}
		if (errno != EPERM)
			log_warnx("interfaces", "cannot get permanent MAC address for %s",
			    iface->name);
		return -1;
	}
	if (epaddr.addr.data[0] != 0 ||
	    epaddr.addr.data[1] != 0 ||
	    epaddr.addr.data[2] != 0 ||
	    epaddr.addr.data[3] != 0 ||
	    epaddr.addr.data[4] != 0 ||
	    epaddr.addr.data[5] != 0 ||
	    epaddr.addr.data[6] != 0) {
		memcpy(iface->address, epaddr.addr.data, ETHER_ADDR_LEN);
		return 0;
	}
	log_debug("interfaces", "cannot get permanent MAC for %s", iface->name);
	return -1;
}

/**
 * Get permanent MAC address for a bond device.
 */
static void
iflinux_get_permanent_mac_bond(struct lldpd *cfg,
    struct interfaces_device_list *interfaces,
    struct interfaces_device *iface)
{
	struct interfaces_device *master = iface->upper;
	int f, state = 0;
	FILE *netbond;
	const char *slaveif = "Slave Interface: ";
	const char *hwaddr = "Permanent HW addr: ";
	u_int8_t mac[ETHER_ADDR_LEN];
	char path[SYSFS_PATH_MAX];
	char line[100];

	/* We have a bond, we need to query it to get real MAC addresses */
	if (snprintf(path, SYSFS_PATH_MAX, "/proc/net/bonding/%s",
		master->name) >= SYSFS_PATH_MAX) {
		log_warnx("interfaces", "path truncated");
		return;
	}
	if ((f = priv_open(path)) < 0) {
		if (snprintf(path, SYSFS_PATH_MAX, "/proc/self/net/bonding/%s",
			master->name) >= SYSFS_PATH_MAX) {
			log_warnx("interfaces", "path truncated");
			return;
		}
		f = priv_open(path);
	}
	if (f < 0) {
		log_warnx("interfaces",
		    "unable to get permanent MAC address for %s",
		    iface->name);
		return;
	}
	if ((netbond = fdopen(f, "r")) == NULL) {
		log_warn("interfaces", "unable to read stream from %s", path);
		close(f);
		return;
	}
	/* State 0:
	   We parse the file to search "Slave Interface: ". If found, go to
	   state 1.
	   State 1:
	   We parse the file to search "Permanent HW addr: ". If found, we get
	   the mac.
	*/
	while (fgets(line, sizeof(line), netbond)) {
		switch (state) {
		case 0:
			if (strncmp(line, slaveif, strlen(slaveif)) == 0) {
				if (line[strlen(line)-1] == '\n')
					line[strlen(line)-1] = '\0';
				if (strcmp(iface->name,
					line + strlen(slaveif)) == 0)
					state++;
			}
			break;
		case 1:
			if (strncmp(line, hwaddr, strlen(hwaddr)) == 0) {
				if (line[strlen(line)-1] == '\n')
					line[strlen(line)-1] = '\0';
				if (sscanf(line + strlen(hwaddr),
					"%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx",
					&mac[0], &mac[1], &mac[2],
					&mac[3], &mac[4], &mac[5]) !=
				    ETHER_ADDR_LEN) {
					log_warn("interfaces", "unable to parse %s",
					    line + strlen(hwaddr));
					fclose(netbond);
					return;
				}
				memcpy(iface->address, mac,
				    ETHER_ADDR_LEN);
				fclose(netbond);
				return;
			}
			break;
		}
	}
	log_warnx("interfaces", "unable to find real MAC address for enslaved %s",
	    iface->name);
	fclose(netbond);
}

/**
 * Get permanent MAC.
 */
static void
iflinux_get_permanent_mac(struct lldpd *cfg,
    struct interfaces_device_list *interfaces,
    struct interfaces_device *iface)
{
	struct interfaces_device *master = iface->upper;

	if (master == NULL || master->type != IFACE_BOND_T)
		return;
	if (iflinux_get_permanent_mac_ethtool(cfg, interfaces, iface) == -1 &&
	    (master->driver == NULL || !strcmp(master->driver, "bonding")))
		/* Fallback to old method for a bond */
		iflinux_get_permanent_mac_bond(cfg, interfaces, iface);
}

#ifdef ENABLE_DOT3
#define ETHTOOL_LINK_MODE_MASK_MAX_KERNEL_NU32 (SCHAR_MAX)
#define ETHTOOL_DECLARE_LINK_MODE_MASK(name)			\
	uint32_t name[ETHTOOL_LINK_MODE_MASK_MAX_KERNEL_NU32]

struct ethtool_link_usettings {
	struct ethtool_link_settings base;
	struct {
		ETHTOOL_DECLARE_LINK_MODE_MASK(supported);
		ETHTOOL_DECLARE_LINK_MODE_MASK(advertising);
		ETHTOOL_DECLARE_LINK_MODE_MASK(lp_advertising);
	} link_modes;
};

static inline int
iflinux_ethtool_link_mode_test_bit(unsigned int nr, const uint32_t *mask)
{
	if (nr >= 32 * ETHTOOL_LINK_MODE_MASK_MAX_KERNEL_NU32)
		return 0;
	return !!(mask[nr / 32] & (1 << (nr % 32)));
}
static inline void
iflinux_ethtool_link_mode_unset_bit(unsigned int nr, uint32_t *mask)
{
	if (nr >= 32 * ETHTOOL_LINK_MODE_MASK_MAX_KERNEL_NU32)
		return;
	mask[nr / 32] &= ~(1 << (nr % 32));
}
static inline int
iflinux_ethtool_link_mode_is_empty(const uint32_t *mask)
{
	for (unsigned int i = 0;
	     i < ETHTOOL_LINK_MODE_MASK_MAX_KERNEL_NU32;
	     ++i) {
		if (mask[i] != 0)
			return 0;
	}

	return 1;
}

static int
iflinux_ethtool_glink(struct lldpd *cfg, const char *ifname, struct ethtool_link_usettings *uset) {
	int rc;

	/* Try with ETHTOOL_GLINKSETTINGS first */
	struct {
		struct ethtool_link_settings req;
		uint32_t link_mode_data[3 * ETHTOOL_LINK_MODE_MASK_MAX_KERNEL_NU32];
	} ecmd;
	static int8_t nwords = 0;
	struct ifreq ifr = {};
	strlcpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));

	if (nwords == 0) {
		/* Do a handshake first. We assume that this is device-independant. */
		memset(&ecmd, 0, sizeof(ecmd));
		ecmd.req.cmd = ETHTOOL_GLINKSETTINGS;
		ifr.ifr_data = (caddr_t)&ecmd;
		rc = ioctl(cfg->g_sock, SIOCETHTOOL, &ifr);
		if (rc == 0) {
			nwords = -ecmd.req.link_mode_masks_nwords;
			log_debug("interfaces", "glinksettings nwords is %" PRId8, nwords);
		} else {
			static int once = 0;
			if (errno == EPERM && !once) {
				log_warn("interfaces",
				    "cannot get ethtool link information "
				    "with GLINKSETTINGS (requires 4.9+)");
				once = 1;
			}
			nwords = -1;
		}
	}
	if (nwords > 0) {
		memset(&ecmd, 0, sizeof(ecmd));
		ecmd.req.cmd = ETHTOOL_GLINKSETTINGS;
		ecmd.req.link_mode_masks_nwords = nwords;
		ifr.ifr_data = (caddr_t)&ecmd;
		rc = ioctl(cfg->g_sock, SIOCETHTOOL, &ifr);
		if (rc == 0) {
			log_debug("interfaces", "got ethtool results for %s with GLINKSETTINGS",
			    ifname);
			memcpy(&uset->base, &ecmd.req, sizeof(uset->base));
			unsigned int u32_offs = 0;
			memcpy(uset->link_modes.supported,
			    &ecmd.link_mode_data[u32_offs],
			    4 * ecmd.req.link_mode_masks_nwords);
			u32_offs += ecmd.req.link_mode_masks_nwords;
			memcpy(uset->link_modes.advertising,
			    &ecmd.link_mode_data[u32_offs],
			    4 * ecmd.req.link_mode_masks_nwords);
			u32_offs += ecmd.req.link_mode_masks_nwords;
			memcpy(uset->link_modes.lp_advertising,
			    &ecmd.link_mode_data[u32_offs],
			    4 * ecmd.req.link_mode_masks_nwords);
			goto end;
		}
	}

	/* Try with ETHTOOL_GSET */
	struct ethtool_cmd ethc;
	memset(&ethc, 0, sizeof(ethc));
	ethc.cmd = ETHTOOL_GSET;
	ifr.ifr_data = (caddr_t)&ethc;
	rc = ioctl(cfg->g_sock, SIOCETHTOOL, &ifr);
	if (rc == 0) {
		/* Do a partial copy (only what we need) */
		log_debug("interfaces", "got ethtool results for %s with GSET",
		    ifname);
		memset(uset, 0, sizeof(*uset));
		uset->base.cmd = ETHTOOL_GSET;
		uset->base.link_mode_masks_nwords = 1;
		uset->link_modes.supported[0] = ethc.supported;
		uset->link_modes.advertising[0] = ethc.advertising;
		uset->link_modes.lp_advertising[0] = ethc.lp_advertising;
		uset->base.speed = (ethc.speed_hi << 16) | ethc.speed;
		uset->base.duplex = ethc.duplex;
		uset->base.port = ethc.port;
		uset->base.autoneg = ethc.autoneg;
	} else {
		static int once = 0;
		if (errno == EPERM && !once) {
			log_warn("interfaces",
			    "cannot get ethtool link information "
			    "with GSET (requires 2.6.19+)");
			once = 1;
		}
	}
end:
	return rc;
}

/* Fill up MAC/PHY for a given hardware port */
static void
iflinux_macphy(struct lldpd *cfg, struct lldpd_hardware *hardware)
{
	struct ethtool_link_usettings uset;
	struct lldpd_port *port = &hardware->h_lport;
	int j;
	int advertised_ethtool_to_rfc3636[][2] = {
		{ETHTOOL_LINK_MODE_10baseT_Half_BIT, LLDP_DOT3_LINK_AUTONEG_10BASE_T},
		{ETHTOOL_LINK_MODE_10baseT_Full_BIT, LLDP_DOT3_LINK_AUTONEG_10BASET_FD},
		{ETHTOOL_LINK_MODE_100baseT_Half_BIT, LLDP_DOT3_LINK_AUTONEG_100BASE_TX},
		{ETHTOOL_LINK_MODE_100baseT_Full_BIT, LLDP_DOT3_LINK_AUTONEG_100BASE_TXFD},
		{ETHTOOL_LINK_MODE_1000baseT_Half_BIT, LLDP_DOT3_LINK_AUTONEG_1000BASE_T},
		{ETHTOOL_LINK_MODE_1000baseT_Full_BIT, LLDP_DOT3_LINK_AUTONEG_1000BASE_TFD},
		{ETHTOOL_LINK_MODE_1000baseKX_Full_BIT, LLDP_DOT3_LINK_AUTONEG_1000BASE_XFD},
		{ETHTOOL_LINK_MODE_Pause_BIT, LLDP_DOT3_LINK_AUTONEG_FDX_PAUSE},
		{ETHTOOL_LINK_MODE_Asym_Pause_BIT, LLDP_DOT3_LINK_AUTONEG_FDX_APAUSE},
		{-1, 0}};

	log_debug("interfaces", "ask ethtool for the appropriate MAC/PHY for %s",
	    hardware->h_ifname);
	if (iflinux_ethtool_glink(cfg, hardware->h_ifname, &uset) == 0) {
		port->p_macphy.autoneg_support = iflinux_ethtool_link_mode_test_bit(
			ETHTOOL_LINK_MODE_Autoneg_BIT, uset.link_modes.supported);
		port->p_macphy.autoneg_enabled = (uset.base.autoneg == AUTONEG_DISABLE) ? 0 : 1;
		for (j=0; advertised_ethtool_to_rfc3636[j][0] >= 0; j++) {
			if (iflinux_ethtool_link_mode_test_bit(
				    advertised_ethtool_to_rfc3636[j][0],
				    uset.link_modes.advertising)) {
				port->p_macphy.autoneg_advertised |=
				    advertised_ethtool_to_rfc3636[j][1];
				iflinux_ethtool_link_mode_unset_bit(
					advertised_ethtool_to_rfc3636[j][0],
					uset.link_modes.advertising);
			}
		}
		iflinux_ethtool_link_mode_unset_bit(ETHTOOL_LINK_MODE_Autoneg_BIT, uset.link_modes.advertising);
		iflinux_ethtool_link_mode_unset_bit(ETHTOOL_LINK_MODE_TP_BIT, uset.link_modes.advertising);
		iflinux_ethtool_link_mode_unset_bit(ETHTOOL_LINK_MODE_AUI_BIT, uset.link_modes.advertising);
		iflinux_ethtool_link_mode_unset_bit(ETHTOOL_LINK_MODE_MII_BIT, uset.link_modes.advertising);
		iflinux_ethtool_link_mode_unset_bit(ETHTOOL_LINK_MODE_FIBRE_BIT, uset.link_modes.advertising);
		iflinux_ethtool_link_mode_unset_bit(ETHTOOL_LINK_MODE_BNC_BIT, uset.link_modes.advertising);
		iflinux_ethtool_link_mode_unset_bit(ETHTOOL_LINK_MODE_Pause_BIT, uset.link_modes.advertising);
		iflinux_ethtool_link_mode_unset_bit(ETHTOOL_LINK_MODE_Asym_Pause_BIT, uset.link_modes.advertising);
		iflinux_ethtool_link_mode_unset_bit(ETHTOOL_LINK_MODE_Backplane_BIT, uset.link_modes.advertising);
		if (!iflinux_ethtool_link_mode_is_empty(uset.link_modes.advertising)) {
			port->p_macphy.autoneg_advertised |= LLDP_DOT3_LINK_AUTONEG_OTHER;
		}
		switch (uset.base.speed) {
		case SPEED_10:
			port->p_macphy.mau_type = (uset.base.duplex == DUPLEX_FULL) ? \
			    LLDP_DOT3_MAU_10BASETFD : LLDP_DOT3_MAU_10BASETHD;
			if (uset.base.port == PORT_BNC) port->p_macphy.mau_type = LLDP_DOT3_MAU_10BASE2;
			if (uset.base.port == PORT_FIBRE)
				port->p_macphy.mau_type = (uset.base.duplex == DUPLEX_FULL) ? \
				    LLDP_DOT3_MAU_10BASEFLFD : LLDP_DOT3_MAU_10BASEFLHD;
			break;
		case SPEED_100:
			port->p_macphy.mau_type = (uset.base.duplex == DUPLEX_FULL) ? \
			    LLDP_DOT3_MAU_100BASETXFD : LLDP_DOT3_MAU_100BASETXHD;
			if (uset.base.port == PORT_BNC)
				port->p_macphy.mau_type = (uset.base.duplex == DUPLEX_FULL) ? \
				    LLDP_DOT3_MAU_100BASET2FD : LLDP_DOT3_MAU_100BASET2HD;
			if (uset.base.port == PORT_FIBRE)
				port->p_macphy.mau_type = (uset.base.duplex == DUPLEX_FULL) ? \
				    LLDP_DOT3_MAU_100BASEFXFD : LLDP_DOT3_MAU_100BASEFXHD;
			break;
		case SPEED_1000:
			port->p_macphy.mau_type = (uset.base.duplex == DUPLEX_FULL) ? \
			    LLDP_DOT3_MAU_1000BASETFD : LLDP_DOT3_MAU_1000BASETHD;
			if (uset.base.port == PORT_FIBRE)
				port->p_macphy.mau_type = (uset.base.duplex == DUPLEX_FULL) ? \
				    LLDP_DOT3_MAU_1000BASEXFD : LLDP_DOT3_MAU_1000BASEXHD;
			break;
		case SPEED_10000:
			// For fiber, we tell 10GIGBASEX and for others,
			// 10GIGBASER. It's not unusual to have 10GIGBASER on
			// fiber either but we don't have 10GIGBASET for
			// copper. No good solution.
			port->p_macphy.mau_type = (uset.base.port == PORT_FIBRE) ?	\
			    LLDP_DOT3_MAU_10GIGBASELR : LLDP_DOT3_MAU_10GIGBASECX4;
			break;
		case SPEED_40000:
			// Same kind of approximation.
			port->p_macphy.mau_type = (uset.base.port == PORT_FIBRE) ? \
			    LLDP_DOT3_MAU_40GBASELR4 : LLDP_DOT3_MAU_40GBASECR4;
			break;
		case SPEED_100000:
			// Ditto
			port->p_macphy.mau_type = (uset.base.port == PORT_FIBRE) ? \
			    LLDP_DOT3_MAU_100GBASELR4 : LLDP_DOT3_MAU_100GBASECR10;
			break;
		}
		if (uset.base.port == PORT_AUI) port->p_macphy.mau_type = LLDP_DOT3_MAU_AUI;
	}
}
#else /* ENABLE_DOT3 */
static void
iflinux_macphy(struct lldpd *cfg, struct lldpd_hardware *hardware)
{
}
#endif /* ENABLE_DOT3 */


struct bond_master {
	char name[IFNAMSIZ];
	int  index;
};

static int
iface_bond_init(struct lldpd *cfg, struct lldpd_hardware *hardware)
{
	struct bond_master *master = hardware->h_data;
	int fd;
	int un = 1;

	if (!master) return -1;

	log_debug("interfaces", "initialize enslaved device %s",
	    hardware->h_ifname);

	/* First, we get a socket to the raw physical interface */
	if ((fd = priv_iface_init(hardware->h_ifindex,
			hardware->h_ifname)) == -1)
		return -1;
	hardware->h_sendfd = fd;
	interfaces_setup_multicast(cfg, hardware->h_ifname, 0);

	/* Then, we open a raw interface for the master */
	log_debug("interfaces", "enslaved device %s has master %s(%d)",
	    hardware->h_ifname, master->name, master->index);
	if ((fd = priv_iface_init(master->index, master->name)) == -1) {
		close(hardware->h_sendfd);
		return -1;
	}
	/* With bonding and older kernels (< 2.6.27) we need to listen
	 * to bond device. We use setsockopt() PACKET_ORIGDEV to get
	 * physical device instead of bond device (works with >=
	 * 2.6.24). */
	if (setsockopt(fd, SOL_PACKET,
		PACKET_ORIGDEV, &un, sizeof(un)) == -1) {
		log_info("interfaces", "unable to setsockopt for master bonding device of %s. "
		    "You will get inaccurate results",
		    hardware->h_ifname);
	}
	interfaces_setup_multicast(cfg, master->name, 0);

	levent_hardware_add_fd(hardware, hardware->h_sendfd);
	levent_hardware_add_fd(hardware, fd);
	log_debug("interfaces", "interface %s initialized (fd=%d,master=%s[%d])",
	    hardware->h_ifname,
	    hardware->h_sendfd,
	    master->name, fd);
	return 0;
}

static int
iface_bond_recv(struct lldpd *cfg, struct lldpd_hardware *hardware,
    int fd, char *buffer, size_t size)
{
	int n;
	struct sockaddr_ll from;
	struct bond_master *master = hardware->h_data;

	log_debug("interfaces", "receive PDU from enslaved device %s",
	    hardware->h_ifname);
	if ((n = iflinux_generic_recv(hardware, fd, buffer, size, &from)) == -1)
		return -1;
	if (fd == hardware->h_sendfd)
		/* We received this on the physical interface. */
		return n;
	/* We received this on the bonding interface. Is it really for us? */
	if (from.sll_ifindex == hardware->h_ifindex)
		/* This is for us */
		return n;
	if (from.sll_ifindex == master->index)
		/* We don't know from which physical interface it comes (kernel
		 * < 2.6.24). In doubt, this is for us. */
		return n;
	return -1;		/* Not for us */
}

static int
iface_bond_close(struct lldpd *cfg, struct lldpd_hardware *hardware)
{
	struct bond_master *master = hardware->h_data;
	log_debug("interfaces", "closing enslaved device %s",
	    hardware->h_ifname);
	interfaces_setup_multicast(cfg, hardware->h_ifname, 1);
	interfaces_setup_multicast(cfg, master->name, 1);
	free(hardware->h_data); hardware->h_data = NULL;
	return 0;
}

struct lldpd_ops bond_ops = {
	.send = iflinux_eth_send,
	.recv = iface_bond_recv,
	.cleanup = iface_bond_close,
};

static void
iflinux_handle_bond(struct lldpd *cfg, struct interfaces_device_list *interfaces)
{
	struct interfaces_device *iface;
	struct interfaces_device *master;
	struct lldpd_hardware *hardware;
	struct bond_master *bmaster;
	int created;
	TAILQ_FOREACH(iface, interfaces, next) {
		if (!(iface->type & IFACE_PHYSICAL_T)) continue;
		if (iface->ignore) continue;
		if (!iface->upper || !(iface->upper->type & IFACE_BOND_T)) continue;

		master = iface->upper;
		log_debug("interfaces", "%s is an acceptable enslaved device (master=%s)",
		    iface->name, master->name);
		created = 0;
		if ((hardware = lldpd_get_hardware(cfg,
			    iface->name,
			    iface->index)) == NULL) {
			if  ((hardware = lldpd_alloc_hardware(cfg,
				    iface->name,
				    iface->index)) == NULL) {
				log_warnx("interfaces", "Unable to allocate space for %s",
				    iface->name);
				continue;
			}
			created = 1;
		}
		if (hardware->h_flags) continue;
		if (hardware->h_ops != &bond_ops) {
			if (!created) {
				log_debug("interfaces",
				    "bond %s is converted from another type of interface",
				    hardware->h_ifname);
				if (hardware->h_ops && hardware->h_ops->cleanup)
					hardware->h_ops->cleanup(cfg, hardware);
				levent_hardware_release(hardware);
				levent_hardware_init(hardware);
			}
			bmaster = hardware->h_data = calloc(1, sizeof(struct bond_master));
			if (!bmaster) {
				log_warn("interfaces", "not enough memory");
				lldpd_hardware_cleanup(cfg, hardware);
				continue;
			}
		} else bmaster = hardware->h_data;
		bmaster->index = master->index;
		strlcpy(bmaster->name, master->name, IFNAMSIZ);
		if (hardware->h_ops != &bond_ops) {
			if (iface_bond_init(cfg, hardware) != 0) {
				log_warn("interfaces", "unable to initialize %s",
				    hardware->h_ifname);
				lldpd_hardware_cleanup(cfg, hardware);
				continue;
			}
			hardware->h_ops = &bond_ops;
			hardware->h_mangle = 1;
		}
		if (created)
			interfaces_helper_add_hardware(cfg, hardware);
		else
			lldpd_port_cleanup(&hardware->h_lport, 0);

		hardware->h_flags = iface->flags;
		iface->ignore = 1;

		/* Get local address */
		memcpy(&hardware->h_lladdr, iface->address, ETHER_ADDR_LEN);

		/* Fill information about port */
		interfaces_helper_port_name_desc(cfg, hardware, iface);

		/* Fill additional info */
#ifdef ENABLE_DOT3
		hardware->h_lport.p_aggregid = master->index;
#endif
		hardware->h_mtu = iface->mtu ? iface->mtu : 1500;
	}
}

/* Query each interface to get the appropriate driver */
static void
iflinux_add_driver(struct lldpd *cfg,
    struct interfaces_device_list *interfaces)
{
	struct interfaces_device *iface;
	TAILQ_FOREACH(iface, interfaces, next) {
		struct ethtool_drvinfo ethc = {
			.cmd = ETHTOOL_GDRVINFO
		};
		struct ifreq ifr = {
			.ifr_data = (caddr_t)&ethc
		};
		if (iface->driver) continue;

		strlcpy(ifr.ifr_name, iface->name, IFNAMSIZ);
		if (ioctl(cfg->g_sock, SIOCETHTOOL, &ifr) == 0) {
			iface->driver = strdup(ethc.driver);
			log_debug("interfaces", "driver for %s is `%s`",
			    iface->name, iface->driver);
		}
	}
}

/* Query each interface to see if it is a wireless one */
static void
iflinux_add_wireless(struct lldpd *cfg,
    struct interfaces_device_list *interfaces)
{
	struct interfaces_device *iface;
	TAILQ_FOREACH(iface, interfaces, next) {
		struct iwreq iwr = {};
		strlcpy(iwr.ifr_name, iface->name, IFNAMSIZ);
		if (ioctl(cfg->g_sock, SIOCGIWNAME, &iwr) >= 0) {
			log_debug("interfaces", "%s is wireless",
			    iface->name);
			iface->type |= IFACE_WIRELESS_T | IFACE_PHYSICAL_T;
		}
	}
}

/* Query each interface to see if it is a bridge */
static void
iflinux_add_bridge(struct lldpd *cfg,
    struct interfaces_device_list *interfaces)
{
	struct interfaces_device *iface;

	TAILQ_FOREACH(iface, interfaces, next) {
		if (iface->type & (IFACE_PHYSICAL_T|
			IFACE_VLAN_T|IFACE_BOND_T|IFACE_BRIDGE_T))
			continue;
		if (iflinux_is_bridge(cfg, interfaces, iface)) {
			log_debug("interfaces",
			    "interface %s is a bridge",
			    iface->name);
			iface->type |= IFACE_BRIDGE_T;
		}
	}
}

/* Query each interface to see if it is a bond */
static void
iflinux_add_bond(struct lldpd *cfg,
    struct interfaces_device_list *interfaces)
{
	struct interfaces_device *iface;

	TAILQ_FOREACH(iface, interfaces, next) {
		if (iface->type & (IFACE_PHYSICAL_T|IFACE_VLAN_T|
			IFACE_BOND_T|IFACE_BRIDGE_T))
			continue;
		if (iflinux_is_bond(cfg, interfaces, iface)) {
			log_debug("interfaces",
			    "interface %s is a bond",
			    iface->name);
			iface->type |= IFACE_BOND_T;
		}
	}
}

/* Query each interface to see if it is a vlan */
static void
iflinux_add_vlan(struct lldpd *cfg,
    struct interfaces_device_list *interfaces)
{
	struct interfaces_device *iface;

	TAILQ_FOREACH(iface, interfaces, next) {
		if (iface->type & (IFACE_PHYSICAL_T|IFACE_VLAN_T|
			IFACE_BOND_T|IFACE_BRIDGE_T))
			continue;
		if (iflinux_is_vlan(cfg, interfaces, iface)) {
			log_debug("interfaces",
			    "interface %s is a VLAN",
			    iface->name);
			iface->type |= IFACE_VLAN_T;
		}
	}
}

static void
iflinux_add_physical(struct lldpd *cfg,
    struct interfaces_device_list *interfaces)
{
	struct interfaces_device *iface;
	/* Blacklist some drivers */
	const char * const *rif;
	const char * const blacklisted_drivers[] = {
		"cdc_mbim",
		"vxlan",
		NULL
	};

	TAILQ_FOREACH(iface, interfaces, next) {
		if (iface->type & (IFACE_VLAN_T|
			IFACE_BOND_T|IFACE_BRIDGE_T))
			continue;

		iface->type &= ~IFACE_PHYSICAL_T;

		/* We request that the interface is able to do either multicast
		 * or broadcast to be able to send discovery frames. */
		if (!(iface->flags & (IFF_MULTICAST|IFF_BROADCAST))) {
			log_debug("interfaces", "skip %s: not able to do multicast nor broadcast",
			    iface->name);
			continue;
		}

		/* Check if the driver is not blacklisted */
		if (iface->driver) {
			int skip = 0;
			for (rif = blacklisted_drivers; *rif; rif++) {
				if (strcmp(iface->driver, *rif) == 0) {
					log_debug("interfaces", "skip %s: blacklisted driver",
					    iface->name);
					skip = 1;
					break;
				}
			}
			if (skip) continue;
		}

		/* If the interface is linked to another one, skip it too. */
		if (iface->lower && (!iface->driver || strcmp(iface->driver, "veth"))) {
			log_debug("interfaces", "skip %s: there is a lower interface (%s)",
			    iface->name, iface->lower->name);
			continue;
		}

		/* Get the real MAC address (for example, if the interface is enslaved) */
		iflinux_get_permanent_mac(cfg, interfaces, iface);

		log_debug("interfaces",
		    "%s is a physical interface",
		    iface->name);
		iface->type |= IFACE_PHYSICAL_T;
	}
}

void
interfaces_update(struct lldpd *cfg)
{
	struct lldpd_hardware *hardware;
	struct interfaces_device_list *interfaces;
	struct interfaces_address_list *addresses;
	interfaces = netlink_get_interfaces(cfg);
	addresses = netlink_get_addresses(cfg);
	if (interfaces == NULL || addresses == NULL) {
		log_warnx("interfaces", "cannot update the list of local interfaces");
		return;
	}

	/* Add missing bits to list of interfaces */
	iflinux_add_driver(cfg, interfaces);
	if (LOCAL_CHASSIS(cfg)->c_cap_available & LLDP_CAP_WLAN)
		iflinux_add_wireless(cfg, interfaces);
	if (LOCAL_CHASSIS(cfg)->c_cap_available & LLDP_CAP_BRIDGE)
		iflinux_add_bridge(cfg, interfaces);
	iflinux_add_bond(cfg, interfaces);
	iflinux_add_vlan(cfg, interfaces);
	iflinux_add_physical(cfg, interfaces);

	interfaces_helper_whitelist(cfg, interfaces);
	iflinux_handle_bond(cfg, interfaces);
	interfaces_helper_physical(cfg, interfaces,
	    &eth_ops,
	    iflinux_eth_init);
#ifdef ENABLE_DOT1
	interfaces_helper_vlan(cfg, interfaces);
#endif
	interfaces_helper_mgmt(cfg, addresses);
	interfaces_helper_chassis(cfg, interfaces);

	/* Mac/PHY */
	TAILQ_FOREACH(hardware, &cfg->g_hardware, h_entries) {
		if (!hardware->h_flags) continue;
		iflinux_macphy(cfg, hardware);
		interfaces_helper_promisc(cfg, hardware);
	}
}

void
interfaces_cleanup(struct lldpd *cfg)
{
	netlink_cleanup(cfg);
}
