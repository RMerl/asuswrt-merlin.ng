/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2013 Vincent Bernat <bernat@luffy.cx>
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
#include <sys/sockio.h>
#include <net/if_types.h>

/* Solaris comes with libdladm which seems to be handy to get all the necessary
 * information. Unfortunately, this library needs a special device file and a
 * Unix socket to a daemon. This is a bit difficult to use it in a
 * privilege-separated daemon. Therefore, we keep using ioctl(). This should
 * also improve compatibility with older versions of Solaris.
 */

static void
ifsolaris_extract(struct lldpd *cfg,
    struct interfaces_device_list *interfaces,
    struct interfaces_address_list *addresses,
    struct lifreq *lifr) {
	int flags = 0;
	int index = 0;
	struct interfaces_address *address = NULL;
	struct interfaces_device *device = NULL;

	sa_family_t lifr_af = lifr->lifr_addr.ss_family;
	struct lifreq lifrl = { .lifr_name = {} };
	strlcpy(lifrl.lifr_name, lifr->lifr_name, sizeof(lifrl.lifr_name));

	/* Flags */
	if (ioctl(cfg->g_sock, SIOCGLIFFLAGS, (caddr_t)&lifrl) < 0) {
		log_warn("interfaces", "unable to get flags for %s",
		    lifrl.lifr_name);
		return;
	}
	flags = lifrl.lifr_flags;

	/* Index */
	if (ioctl(cfg->g_sock, SIOCGLIFINDEX, (caddr_t)&lifrl) < 0) {
		log_warn("interfaces", "unable to get index for %s",
		    lifrl.lifr_name);
		return;
	}
	index = lifrl.lifr_index;

	/* Record the address */
	if ((address = malloc(sizeof(struct interfaces_address))) == NULL) {
		log_warn("interfaces",
		    "not enough memory for a new IP address on %s",
		    lifrl.lifr_name);
		return;
	}
	address->flags = flags;
	address->index = index;
	memcpy(&address->address,
	    &lifr->lifr_addr,
	    (lifr_af == AF_INET)?
	    sizeof(struct sockaddr_in):
	    sizeof(struct sockaddr_in6));
	TAILQ_INSERT_TAIL(addresses, address, next);

	/* Hardware address */
	if (ioctl(cfg->g_sock, SIOCGLIFHWADDR, (caddr_t)&lifrl) < 0) {
		log_debug("interfaces", "unable to get hardware address for %s",
		    lifrl.lifr_name);
		return;
	}
	struct sockaddr_dl *saddrdl = (struct sockaddr_dl*)&lifrl.lifr_addr;
	if (saddrdl->sdl_type != 4) {
		log_debug("interfaces", "skip %s: not an ethernet device (%d)",
		    lifrl.lifr_name, saddrdl->sdl_type);
		return;
	}

	/* Handle the interface */
	if ((device = calloc(1, sizeof(struct interfaces_device))) == NULL) {
		log_warn("interfaces", "unable to allocate memory for %s",
		    lifrl.lifr_name);
		return;
	}

	device->name = strdup(lifrl.lifr_name);
	device->flags = flags;
	device->index = index;
	device->type = IFACE_PHYSICAL_T;
	device->address = malloc(ETHER_ADDR_LEN);
	if (device->address)
		memcpy(device->address, LLADDR(saddrdl), ETHER_ADDR_LEN);

	/* MTU */
	if (ioctl(cfg->g_sock, SIOCGLIFMTU, (caddr_t)&lifrl) < 0) {
		log_debug("interfaces", "unable to get MTU for %s",
		    lifrl.lifr_name);
	} else device->mtu = lifrl.lifr_mtu;

	TAILQ_INSERT_TAIL(interfaces, device, next);
}

extern struct lldpd_ops bpf_ops;
void
interfaces_update(struct lldpd *cfg) {
	struct lldpd_hardware *hardware;
	caddr_t buffer = NULL;
	struct interfaces_device_list *interfaces;
	struct interfaces_address_list *addresses;
	interfaces = malloc(sizeof(struct interfaces_device_list));
	addresses = malloc(sizeof(struct interfaces_address_list));
	if (interfaces == NULL || addresses == NULL) {
		log_warnx("interfaces", "unable to allocate memory");
		goto end;
	}
	TAILQ_INIT(interfaces);
	TAILQ_INIT(addresses);

	struct lifnum lifn = {
		.lifn_family = AF_UNSPEC,
		.lifn_flags = LIFC_ENABLED
	};
	if (ioctl(cfg->g_sock, SIOCGLIFNUM, &lifn) < 0) {
		log_warn("interfaces", "unable to get the number of interfaces");
		goto end;
	}

	size_t bufsize = lifn.lifn_count * sizeof(struct lifreq);
	if ((buffer = malloc(bufsize)) == NULL) {
		log_warn("interfaces", "unable to allocate buffer to get interfaces");
		goto end;
	}

	struct lifconf lifc = {
		.lifc_family = AF_UNSPEC,
		.lifc_flags = LIFC_ENABLED,
		.lifc_len = bufsize,
		.lifc_buf = buffer
	};
	if (ioctl(cfg->g_sock, SIOCGLIFCONF, (char *)&lifc) < 0) {
		log_warn("interfaces", "unable to get the network interfaces");
		goto end;
	}

	int num = lifc.lifc_len / sizeof (struct lifreq);
	if (num > lifn.lifn_count) num = lifn.lifn_count;
	log_debug("interfaces", "got %d interfaces", num);

	struct lifreq *lifrp = (struct lifreq *)buffer;
	for (int n = 0; n < num; n++, lifrp++)
		ifsolaris_extract(cfg, interfaces, addresses, lifrp);

	interfaces_helper_whitelist(cfg, interfaces);
	interfaces_helper_physical(cfg, interfaces,
	    &bpf_ops, ifbpf_phys_init);
	interfaces_helper_mgmt(cfg, addresses);
	interfaces_helper_chassis(cfg, interfaces);

	/* Mac/PHY */
	TAILQ_FOREACH(hardware, &cfg->g_hardware, h_entries) {
		if (!hardware->h_flags) continue;
		/* TODO: mac/phy for Solaris */
		interfaces_helper_promisc(cfg, hardware);
	}

end:
	free(buffer);
	interfaces_free_devices(interfaces);
	interfaces_free_addresses(addresses);
}

void
interfaces_cleanup(struct lldpd *cfg)
{
}
