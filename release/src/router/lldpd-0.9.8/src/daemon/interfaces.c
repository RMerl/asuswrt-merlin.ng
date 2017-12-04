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
#include "trace.h"

#include <stddef.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <arpa/inet.h>

/* Generic ethernet interface initialization */
/**
 * Enable multicast on the given interface.
 */
void
interfaces_setup_multicast(struct lldpd *cfg, const char *name,
    int remove)
{
	int rc;
	size_t i, j;
	const u_int8_t *mac;
	const u_int8_t zero[ETHER_ADDR_LEN] = {};

	for (i = 0; cfg->g_protocols[i].mode != 0; i++) {
		if (!cfg->g_protocols[i].enabled) continue;
		for (mac = cfg->g_protocols[i].mac1, j = 0;
		     j < 3;
		     mac += ETHER_ADDR_LEN,
		     j++) {
			if (memcmp(mac, zero, ETHER_ADDR_LEN) == 0) break;
			if ((rc = priv_iface_multicast(name,
				    mac, !remove)) != 0) {
				errno = rc;
				if (errno != ENOENT)
					log_debug("interfaces",
					    "unable to %s %s address to multicast filter for %s (%s)",
					    (remove)?"delete":"add",
					    cfg->g_protocols[i].name,
					    name, strerror(rc));
			}
		}
	}
}

/**
 * Free an interface.
 *
 * @param iff interface to be freed
 */
void
interfaces_free_device(struct interfaces_device *iff)
{
	if (!iff) return;
	free(iff->name);
	free(iff->alias);
	free(iff->address);
	free(iff->driver);
	free(iff);
}

/**
 * Free a list of interfaces.
 *
 * @param ifs list of interfaces to be freed
 */
void
interfaces_free_devices(struct interfaces_device_list *ifs)
{
	struct interfaces_device *iff, *iff_next;
	if (!ifs) return;
	for (iff = TAILQ_FIRST(ifs);
	     iff != NULL;
	     iff = iff_next) {
		iff_next = TAILQ_NEXT(iff, next);
		interfaces_free_device(iff);
	}
	free(ifs);
}

/**
 * Free one address
 *
 * @param ifaddr Address to be freed
 */
void
interfaces_free_address(struct interfaces_address *ifaddr)
{
	free(ifaddr);
}

/**
 * Free a list of addresses.
 *
 * @param ifaddrs list of addresses
 */
void
interfaces_free_addresses(struct interfaces_address_list *ifaddrs)
{
	struct interfaces_address *ifa, *ifa_next;
	if (!ifaddrs) return;
	for (ifa = TAILQ_FIRST(ifaddrs);
	     ifa != NULL;
	     ifa = ifa_next) {
		ifa_next = TAILQ_NEXT(ifa, next);
		interfaces_free_address(ifa);
	}
	free(ifaddrs);
}

/**
 * Find the appropriate interface from the name.
 *
 * @param interfaces List of available interfaces
 * @param device     Name of the device we search for
 * @return The interface or NULL if not found
 */
struct interfaces_device*
interfaces_nametointerface(struct interfaces_device_list *interfaces,
    const char *device)
{
	struct interfaces_device *iface;
	TAILQ_FOREACH(iface, interfaces, next) {
		if (!strncmp(iface->name, device, IFNAMSIZ))
			return iface;
	}
	log_debug("interfaces", "cannot get interface for index %s",
	    device);
	return NULL;
}

/**
 * Find the appropriate interface from the index.
 *
 * @param interfaces List of available interfaces
 * @param index      Index of the device we search for
 * @return The interface or NULL if not found
 */
struct interfaces_device*
interfaces_indextointerface(struct interfaces_device_list *interfaces,
    int index)
{
	struct interfaces_device *iface;
	TAILQ_FOREACH(iface, interfaces, next) {
		if (iface->index == index)
			return iface;
	}
	log_debug("interfaces", "cannot get interface for index %d",
	    index);
	return NULL;
}

void
interfaces_helper_whitelist(struct lldpd *cfg,
    struct interfaces_device_list *interfaces)
{
	struct interfaces_device *iface;

	if (!cfg->g_config.c_iface_pattern)
		return;

	TAILQ_FOREACH(iface, interfaces, next) {
		int m = pattern_match(iface->name, cfg->g_config.c_iface_pattern, 0);
		switch (m) {
		case 0:
			log_debug("interfaces", "blacklist %s", iface->name);
			iface->ignore = 1;
			continue;
		case 2:
			log_debug("interfaces", "whitelist %s (consider it as a physical interface)",
			    iface->name);
			iface->type |= IFACE_PHYSICAL_T;
			continue;
		}
	}
}

#ifdef ENABLE_DOT1
static void
iface_append_vlan(struct lldpd *cfg,
    struct interfaces_device *vlan,
    struct interfaces_device *lower)
{
	struct lldpd_hardware *hardware =
	    lldpd_get_hardware(cfg, lower->name, lower->index);
	struct lldpd_port *port;
	struct lldpd_vlan *v;

	if (hardware == NULL) {
		log_debug("interfaces",
		    "cannot find real interface %s for VLAN %s",
		    lower->name, vlan->name);
		return;
	}

	/* Check if the VLAN is already here. */
	port = &hardware->h_lport;
	TAILQ_FOREACH(v, &port->p_vlans, v_entries)
	    if (strncmp(vlan->name, v->v_name, IFNAMSIZ) == 0)
		    return;
	if ((v = (struct lldpd_vlan *)
		calloc(1, sizeof(struct lldpd_vlan))) == NULL)
		return;
	if ((v->v_name = strdup(vlan->name)) == NULL) {
		free(v);
		return;
	}
	v->v_vid = vlan->vlanid;
	log_debug("interfaces", "append VLAN %s for %s",
	    v->v_name,
	    hardware->h_ifname);
	TAILQ_INSERT_TAIL(&port->p_vlans, v, v_entries);
}

/**
 * Append VLAN to the lowest possible interface.
 *
 * @param vlan  The VLAN interface (used to get VLAN ID).
 * @param upper The upper interface we are currently examining.
 * @param depth Depth of the stack (avoid infinite recursion)
 *
 * Initially, upper == vlan. This function will be called recursively.
 */
static void
iface_append_vlan_to_lower(struct lldpd *cfg,
    struct interfaces_device_list *interfaces,
    struct interfaces_device *vlan,
    struct interfaces_device *upper,
    int depth)
{
	if (depth > 5) {
		log_warn("interfaces",
		    "BUG: maximum depth reached when applying VLAN %s (loop?)",
		    vlan->name);
		return;
	}
	depth++;
	struct interfaces_device *lower;
	log_debug("interfaces",
	    "looking to apply VLAN %s to physical interface behind %s",
	    vlan->name, upper->name);

	/* Easy: check if we have a lower interface. */
	if (upper->lower) {
		log_debug("interfaces", "VLAN %s on lower interface %s",
		    vlan->name, upper->name);
		iface_append_vlan_to_lower(cfg,
		    interfaces, vlan,
		    upper->lower,
		    depth);
		return;
	}

	/* Other easy case, we have a physical interface. */
	if (upper->type & IFACE_PHYSICAL_T) {
		log_debug("interfaces", "VLAN %s on physical interface %s",
		    vlan->name, upper->name);
		iface_append_vlan(cfg, vlan, upper);
		return;
	}

	/* We can now search for interfaces that have our interface as an upper
	 * interface. */
	TAILQ_FOREACH(lower, interfaces, next) {
		if (lower->upper != upper) continue;
		log_debug("interfaces", "VLAN %s on lower interface %s",
		    vlan->name, upper->name);
		iface_append_vlan_to_lower(cfg,
		    interfaces, vlan, lower, depth);
	}
}

void
interfaces_helper_vlan(struct lldpd *cfg,
    struct interfaces_device_list *interfaces)
{
	struct interfaces_device *iface;

	TAILQ_FOREACH(iface, interfaces, next) {
		if (iface->ignore)
			continue;
		if (!(iface->type & IFACE_VLAN_T))
			continue;

		/* We need to find the physical interfaces of this
		   vlan, through bonds and bridges. */
		log_debug("interfaces", "search physical interface for VLAN interface %s",
		    iface->name);
		iface_append_vlan_to_lower(cfg, interfaces,
		    iface, iface, 0);
	}
}
#endif

/* Fill out chassis ID if not already done. Only physical interfaces are
 * considered. */
void
interfaces_helper_chassis(struct lldpd *cfg,
    struct interfaces_device_list *interfaces)
{
	struct interfaces_device *iface;
	struct lldpd_hardware *hardware;
	char *name = NULL;

	LOCAL_CHASSIS(cfg)->c_cap_enabled &=
			    ~(LLDP_CAP_BRIDGE | LLDP_CAP_WLAN | LLDP_CAP_STATION);
	TAILQ_FOREACH(iface, interfaces, next) {
		if (iface->type & IFACE_BRIDGE_T)
			LOCAL_CHASSIS(cfg)->c_cap_enabled |= LLDP_CAP_BRIDGE;
		if (iface->type & IFACE_WIRELESS_T)
			LOCAL_CHASSIS(cfg)->c_cap_enabled |= LLDP_CAP_WLAN;
	}
	if ((LOCAL_CHASSIS(cfg)->c_cap_available & LLDP_CAP_STATION) &&
		(LOCAL_CHASSIS(cfg)->c_cap_enabled == 0))
	    LOCAL_CHASSIS(cfg)->c_cap_enabled = LLDP_CAP_STATION;

	if (LOCAL_CHASSIS(cfg)->c_id != NULL &&
	    LOCAL_CHASSIS(cfg)->c_id_subtype == LLDP_CHASSISID_SUBTYPE_LLADDR)
		return;		/* We already have one */

	TAILQ_FOREACH(iface, interfaces, next) {
		if (!(iface->type & IFACE_PHYSICAL_T)) continue;
		if (cfg->g_config.c_cid_pattern &&
		    !pattern_match(iface->name, cfg->g_config.c_cid_pattern, 0)) continue;

		if ((hardware = lldpd_get_hardware(cfg,
			    iface->name,
			    iface->index)) == NULL)
			/* That's odd. Let's skip. */
			continue;

		name = malloc(ETHER_ADDR_LEN);
		if (!name) {
			log_warn("interfaces", "not enough memory for chassis ID");
			return;
		}
		free(LOCAL_CHASSIS(cfg)->c_id);
		memcpy(name, hardware->h_lladdr, ETHER_ADDR_LEN);
		LOCAL_CHASSIS(cfg)->c_id = name;
		LOCAL_CHASSIS(cfg)->c_id_len = ETHER_ADDR_LEN;
		LOCAL_CHASSIS(cfg)->c_id_subtype = LLDP_CHASSISID_SUBTYPE_LLADDR;
		return;
	}
}

#undef IN_IS_ADDR_LOOPBACK
#define IN_IS_ADDR_LOOPBACK(a) ((a)->s_addr == htonl(INADDR_LOOPBACK))
#undef IN_IS_ADDR_ANY
#define IN_IS_ADDR_ANY(a) ((a)->s_addr == htonl(INADDR_ANY))
#undef IN_IS_ADDR_LINKLOCAL
#define IN_IS_ADDR_LINKLOCAL(a) (((a)->s_addr & htonl(0xffff0000)) == htonl(0xa9fe0000))
#undef IN_IS_ADDR_GLOBAL
#define IN_IS_ADDR_GLOBAL(a) (!IN_IS_ADDR_LOOPBACK(a) && !IN_IS_ADDR_ANY(a) && !IN_IS_ADDR_LINKLOCAL(a))
#undef IN6_IS_ADDR_GLOBAL
#define IN6_IS_ADDR_GLOBAL(a) \
	(!IN6_IS_ADDR_LOOPBACK(a) && !IN6_IS_ADDR_LINKLOCAL(a))

/* Add management addresses for the given family. We only take one of each
   address family, unless a pattern is provided and is not all negative. For
   example !*:*,!10.* will only blacklist addresses. We will pick the first IPv4
   address not matching 10.*.
*/
static int
interfaces_helper_mgmt_for_af(struct lldpd *cfg,
    int af,
    struct interfaces_address_list *addrs,
    int global, int allnegative)
{
	struct interfaces_address *addr;
	struct lldpd_mgmt *mgmt;
	char addrstrbuf[INET6_ADDRSTRLEN];
	int found = 0;
	union lldpd_address in_addr;
	size_t in_addr_size;

	TAILQ_FOREACH(addr, addrs, next) {
		if (addr->address.ss_family != lldpd_af(af))
			continue;

		switch (af) {
		case LLDPD_AF_IPV4:
			in_addr_size = sizeof(struct in_addr);
			memcpy(&in_addr, &((struct sockaddr_in *)&addr->address)->sin_addr,
			    in_addr_size);
			if (global) {
				if (!IN_IS_ADDR_GLOBAL(&in_addr.inet))
					continue;
			} else {
				if (!IN_IS_ADDR_LINKLOCAL(&in_addr.inet))
					continue;
			}
			break;
		case LLDPD_AF_IPV6:
			in_addr_size = sizeof(struct in6_addr);
			memcpy(&in_addr, &((struct sockaddr_in6 *)&addr->address)->sin6_addr,
			    in_addr_size);
			if (global) {
				if (!IN6_IS_ADDR_GLOBAL(&in_addr.inet6))
					continue;
			} else {
				if (!IN6_IS_ADDR_LINKLOCAL(&in_addr.inet6))
					continue;
			}
			break;
		default:
			assert(0);
			continue;
		}
		if (inet_ntop(lldpd_af(af), &in_addr,
			addrstrbuf, sizeof(addrstrbuf)) == NULL) {
			log_warn("interfaces", "unable to convert IP address to a string");
			continue;
		}
		if (cfg->g_config.c_mgmt_pattern == NULL ||
		    pattern_match(addrstrbuf, cfg->g_config.c_mgmt_pattern, allnegative)) {
			mgmt = lldpd_alloc_mgmt(af, &in_addr, in_addr_size,
			    addr->index);
			if (mgmt == NULL) {
				assert(errno == ENOMEM); /* anything else is a bug */
				log_warn("interfaces", "out of memory error");
				return found;
			}
			log_debug("interfaces", "add management address %s", addrstrbuf);
			TAILQ_INSERT_TAIL(&LOCAL_CHASSIS(cfg)->c_mgmt, mgmt, m_entries);
			found = 1;

			/* Don't take additional address if the pattern is all negative. */
			if (allnegative) break;
		}
	}
	return found;
}

/* Find a management address in all available interfaces, even those that were
   already handled. This is a special interface handler because it does not
   really handle interface related information (management address is attached
   to the local chassis). */
void
interfaces_helper_mgmt(struct lldpd *cfg,
    struct interfaces_address_list *addrs)
{
	int allnegative = 0;
	int af;
	const char *pattern = cfg->g_config.c_mgmt_pattern;

	lldpd_chassis_mgmt_cleanup(LOCAL_CHASSIS(cfg));
	if (!cfg->g_config.c_mgmt_advertise)
		return;

	/* Is the pattern provided an actual IP address? */
	if (pattern && strpbrk(pattern, "!,*?") == NULL) {
		struct in6_addr addr;
		size_t addr_size;
		for (af = LLDPD_AF_UNSPEC + 1;
		     af != LLDPD_AF_LAST; af++) {
			switch (af) {
			case LLDPD_AF_IPV4: addr_size = sizeof(struct in_addr); break;
			case LLDPD_AF_IPV6: addr_size = sizeof(struct in6_addr); break;
			default: assert(0);
			}
			if (inet_pton(lldpd_af(af), pattern, &addr) == 1)
				break;
		}
		if (af == LLDPD_AF_LAST) {
			log_debug("interfaces",
			    "interface management pattern is an incorrect IP");
		} else {
			struct lldpd_mgmt *mgmt;
			mgmt = lldpd_alloc_mgmt(af, &addr, addr_size, 0);
			if (mgmt == NULL) {
				log_warn("interfaces", "out of memory error");
				return;
			}
			log_debug("interfaces", "add exact management address %s",
				pattern);
			TAILQ_INSERT_TAIL(&LOCAL_CHASSIS(cfg)->c_mgmt, mgmt, m_entries);
		}
		return;
	}

	/* Is the pattern provided all negative? */
	if (pattern == NULL) allnegative = 1;
	else if (pattern[0] == '!') {
		/* If each comma is followed by '!', its an all
		   negative pattern */
		const char *sep = pattern;
		while ((sep = strchr(sep, ',')) &&
		       (*(++sep) == '!'));
		if (sep == NULL) allnegative = 1;
	}

	/* Find management addresses */
	for (af = LLDPD_AF_UNSPEC + 1; af != LLDPD_AF_LAST; af++) {
		(void)(interfaces_helper_mgmt_for_af(cfg, af, addrs, 1, allnegative) ||
		    interfaces_helper_mgmt_for_af(cfg, af, addrs, 0, allnegative));
	}
}

/* Fill up port name and description */
void
interfaces_helper_port_name_desc(struct lldpd *cfg,
    struct lldpd_hardware *hardware,
    struct interfaces_device *iface)
{
	struct lldpd_port *port = &hardware->h_lport;

	/* We need to set the portid to what the client configured.
	   This can be done from the CLI.
	*/
	int has_alias = (iface->alias != NULL && strlen(iface->alias) != 0);
	int portid_type = cfg->g_config.c_lldp_portid_type;
	if (portid_type == LLDP_PORTID_SUBTYPE_IFNAME ||
	    (portid_type == LLDP_PORTID_SUBTYPE_UNKNOWN && has_alias) ||
	    (port->p_id_subtype == LLDP_PORTID_SUBTYPE_LOCAL && has_alias)) {
		if (port->p_id_subtype != LLDP_PORTID_SUBTYPE_LOCAL) {
			log_debug("interfaces", "use ifname for %s",
			    hardware->h_ifname);
			port->p_id_subtype = LLDP_PORTID_SUBTYPE_IFNAME;
			port->p_id_len = strlen(hardware->h_ifname);
			free(port->p_id);
			if ((port->p_id = calloc(1, port->p_id_len)) == NULL)
				fatal("interfaces", NULL);
			memcpy(port->p_id, hardware->h_ifname, port->p_id_len);
		}

		if (port->p_descr_force == 0) {
			/* use the actual alias in the port description */
			log_debug("interfaces", "using alias in description for %s",
			    hardware->h_ifname);
			free(port->p_descr);
			if (has_alias) {
				port->p_descr = strdup(iface->alias);
			} else {
				/* We don't have anything else to put here and for CDP
				 * with need something non-NULL */
				port->p_descr = strdup(hardware->h_ifname);
			}
		}
	} else {
		if (port->p_id_subtype != LLDP_PORTID_SUBTYPE_LOCAL) {
			log_debug("interfaces", "use MAC address for %s",
			    hardware->h_ifname);
			port->p_id_subtype = LLDP_PORTID_SUBTYPE_LLADDR;
			free(port->p_id);
			if ((port->p_id = calloc(1, ETHER_ADDR_LEN)) == NULL)
				fatal("interfaces", NULL);
			memcpy(port->p_id, hardware->h_lladdr, ETHER_ADDR_LEN);
			port->p_id_len = ETHER_ADDR_LEN;
		}

		if (port->p_descr_force == 0) {
			/* use the ifname in the port description until alias is set */
			log_debug("interfaces", "using ifname in description for %s",
			    hardware->h_ifname);
			free(port->p_descr);
			port->p_descr = strdup(hardware->h_ifname);
		}
	}
}

void
interfaces_helper_add_hardware(struct lldpd *cfg,
    struct lldpd_hardware *hardware)
{
	TRACE(LLDPD_INTERFACES_NEW(hardware->h_ifname));
	TAILQ_INSERT_TAIL(&cfg->g_hardware, hardware, h_entries);
}

void
interfaces_helper_physical(struct lldpd *cfg,
    struct interfaces_device_list *interfaces,
    struct lldpd_ops *ops,
    int(*init)(struct lldpd *, struct lldpd_hardware *))
{
	struct interfaces_device *iface;
	struct lldpd_hardware *hardware;
	int created;

	TAILQ_FOREACH(iface, interfaces, next) {
		if (!(iface->type & IFACE_PHYSICAL_T)) continue;
		if (iface->ignore) continue;

		log_debug("interfaces", "%s is an acceptable ethernet device",
		    iface->name);
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
		if (hardware->h_flags)
			continue;
		if (hardware->h_ops != ops) {
			if (!created) {
				log_debug("interfaces",
				    "interface %s is converted from another type of interface",
				    hardware->h_ifname);
				if (hardware->h_ops && hardware->h_ops->cleanup) {
					hardware->h_ops->cleanup(cfg, hardware);
					levent_hardware_release(hardware);
					levent_hardware_init(hardware);
				}
			}
			if (init(cfg, hardware) != 0) {
				log_warnx("interfaces",
				    "unable to initialize %s",
				    hardware->h_ifname);
				lldpd_hardware_cleanup(cfg, hardware);
				continue;
			}
			hardware->h_ops = ops;
			hardware->h_mangle = (iface->upper &&
			    iface->upper->type & IFACE_BOND_T);
		}
		if (created)
			interfaces_helper_add_hardware(cfg, hardware);
		else
			lldpd_port_cleanup(&hardware->h_lport, 0);

		hardware->h_flags = iface->flags;   /* Should be non-zero */
		iface->ignore = 1;		    /* Future handlers
						       don't have to
						       care about this
						       interface. */

		/* Get local address */
		memcpy(&hardware->h_lladdr, iface->address, ETHER_ADDR_LEN);

		/* Fill information about port */
		interfaces_helper_port_name_desc(cfg, hardware, iface);

		/* Fill additional info */
		hardware->h_mtu = iface->mtu ? iface->mtu : 1500;

#ifdef ENABLE_DOT3
		if (iface->upper && iface->upper->type & IFACE_BOND_T)
			hardware->h_lport.p_aggregid = iface->upper->index;
		else
			hardware->h_lport.p_aggregid = 0;
#endif
	}
}

void
interfaces_helper_promisc(struct lldpd *cfg,
    struct lldpd_hardware *hardware)
{
	if (!cfg->g_config.c_promisc) return;
	if (priv_iface_promisc(hardware->h_ifname) != 0) {
		log_warnx("interfaces",
		    "unable to enable promiscuous mode for %s",
		    hardware->h_ifname);
	}
}

/**
 * Send the packet using the hardware function. Optionnaly mangle the MAC address.
 *
 * With bonds, we have duplicate MAC address on different physical
 * interfaces. We need to alter the source MAC address when we send on an
 * inactive slave. The `h_mangle` flah is used to know if we need to do
 * something like that.
 */
int
interfaces_send_helper(struct lldpd *cfg,
    struct lldpd_hardware *hardware,
    char *buffer, size_t size)
{
	if (size < 2 * ETHER_ADDR_LEN) {
		log_warnx("interfaces",
		    "packet to send on %s is too small!",
		    hardware->h_ifname);
		return 0;
	}
	if (hardware->h_mangle) {
#define MAC_UL_ADMINISTERED_BIT_MASK 0x02
		char *src_mac = buffer + ETHER_ADDR_LEN;
		char arbitrary[] = { 0x00, 0x60, 0x08, 0x69, 0x97, 0xef};

		switch (cfg->g_config.c_bond_slave_src_mac_type) {
		case LLDP_BOND_SLAVE_SRC_MAC_TYPE_LOCALLY_ADMINISTERED:
			if (!(*src_mac & MAC_UL_ADMINISTERED_BIT_MASK)) {
				*src_mac |= MAC_UL_ADMINISTERED_BIT_MASK;
				break;
			}
			/* Fallback to fixed value */
			/* FALL THROUGH */
		case LLDP_BOND_SLAVE_SRC_MAC_TYPE_FIXED:
			memcpy(src_mac, arbitrary, ETHER_ADDR_LEN);
			break;
		case LLDP_BOND_SLAVE_SRC_MAC_TYPE_ZERO:
			memset(src_mac, 0, ETHER_ADDR_LEN);
			break;
		}
	}
	return hardware->h_ops->send(cfg, hardware, buffer, size);
}
