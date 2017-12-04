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
#include "trace.h"

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <libgen.h>
#include <assert.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <pwd.h>
#include <grp.h>

static void		 usage(void);

static struct protocol protos[] =
{
	{ LLDPD_MODE_LLDP, 1, "LLDP", 'l', lldp_send, lldp_decode, NULL,
	  LLDP_ADDR_NEAREST_BRIDGE,
	  LLDP_ADDR_NEAREST_NONTPMR_BRIDGE,
	  LLDP_ADDR_NEAREST_CUSTOMER_BRIDGE },
#ifdef ENABLE_CDP
	{ LLDPD_MODE_CDPV1, 0, "CDPv1", 'c', cdpv1_send, cdp_decode, cdpv1_guess,
	  CDP_MULTICAST_ADDR },
	{ LLDPD_MODE_CDPV2, 0, "CDPv2", 'c', cdpv2_send, cdp_decode, cdpv2_guess,
	  CDP_MULTICAST_ADDR },
#endif
#ifdef ENABLE_SONMP
	{ LLDPD_MODE_SONMP, 0, "SONMP", 's', sonmp_send, sonmp_decode, NULL,
	  SONMP_MULTICAST_ADDR },
#endif
#ifdef ENABLE_EDP
	{ LLDPD_MODE_EDP, 0, "EDP", 'e', edp_send, edp_decode, NULL,
	  EDP_MULTICAST_ADDR },
#endif
#ifdef ENABLE_FDP
	{ LLDPD_MODE_FDP, 0, "FDP", 'f', fdp_send, cdp_decode, NULL,
	  FDP_MULTICAST_ADDR },
#endif
	{ 0, 0, "any", ' ', NULL, NULL, NULL,
	  {0,0,0,0,0,0} }
};

static char		**saved_argv;
#ifdef HAVE___PROGNAME
extern const char	*__progname;
#else
# define __progname "lldpd"
#endif

static void
usage(void)
{
	fprintf(stderr, "Usage:   %s [OPTIONS ...]\n", __progname);
	fprintf(stderr, "Version: %s\n", PACKAGE_STRING);

	fprintf(stderr, "\n");

	fprintf(stderr, "-d       Do not daemonize.\n");
	fprintf(stderr, "-r       Receive-only mode\n");
	fprintf(stderr, "-i       Disable LLDP-MED inventory TLV transmission.\n");
	fprintf(stderr, "-k       Disable advertising of kernel release, version, machine.\n");
	fprintf(stderr, "-S descr Override the default system description.\n");
	fprintf(stderr, "-P name  Override the default hardware platform.\n");
	fprintf(stderr, "-m IP    Specify the IPv4 management addresses of this system.\n");
	fprintf(stderr, "-u file  Specify the Unix-domain socket used for communication with lldpctl(8).\n");
	fprintf(stderr, "-H mode  Specify the behaviour when detecting multiple neighbors.\n");
	fprintf(stderr, "-I iface Limit interfaces to use.\n");
#ifdef ENABLE_LLDPMED
	fprintf(stderr, "-M class Enable emission of LLDP-MED frame. 'class' should be one of:\n");
	fprintf(stderr, "             1 Generic Endpoint (Class I)\n");
	fprintf(stderr, "             2 Media Endpoint (Class II)\n");
	fprintf(stderr, "             3 Communication Device Endpoints (Class III)\n");
	fprintf(stderr, "             4 Network Connectivity Device\n");
#endif
#ifdef USE_SNMP
	fprintf(stderr, "-x       Enable SNMP subagent.\n");
#endif
	fprintf(stderr, "\n");

#if defined ENABLE_CDP || defined ENABLE_EDP || defined ENABLE_FDP || defined ENABLE_SONMP
	fprintf(stderr, "Additional protocol support.\n");
#ifdef ENABLE_CDP
	fprintf(stderr, "-c       Enable the support of CDP protocol. (Cisco)\n");
#endif
#ifdef ENABLE_EDP
	fprintf(stderr, "-e       Enable the support of EDP protocol. (Extreme)\n");
#endif
#ifdef ENABLE_FDP
	fprintf(stderr, "-f       Enable the support of FDP protocol. (Foundry)\n");
#endif
#ifdef ENABLE_SONMP
	fprintf(stderr, "-s       Enable the support of SONMP protocol. (Nortel)\n");
#endif

	fprintf(stderr, "\n");
#endif

	fprintf(stderr, "see manual page lldpd(8) for more information\n");
	exit(1);
}

struct lldpd_hardware *
lldpd_get_hardware(struct lldpd *cfg, char *name, int index)
{
	struct lldpd_hardware *hardware;
	TAILQ_FOREACH(hardware, &cfg->g_hardware, h_entries) {
		if ((strcmp(hardware->h_ifname, name) == 0) &&
		    (hardware->h_ifindex == index))
			break;
	}
	return hardware;
}

/**
 * Allocate the default local port. This port will be cloned each time we need a
 * new local port.
 */
static void
lldpd_alloc_default_local_port(struct lldpd *cfg)
{
	struct lldpd_port *port;

	if ((port = (struct lldpd_port *)
		calloc(1, sizeof(struct lldpd_port))) == NULL)
		fatal("main", NULL);

#ifdef ENABLE_DOT1
	TAILQ_INIT(&port->p_vlans);
	TAILQ_INIT(&port->p_ppvids);
	TAILQ_INIT(&port->p_pids);
#endif
#ifdef ENABLE_CUSTOM
	TAILQ_INIT(&port->p_custom_list);
#endif
	cfg->g_default_local_port = port;
}

/**
 * Clone a given port. The destination needs to be already allocated.
 */
static int
lldpd_clone_port(struct lldpd_port *destination, struct lldpd_port *source)
{

	u_int8_t *output = NULL;
	ssize_t output_len;
	struct lldpd_port *cloned = NULL;
	output_len = lldpd_port_serialize(source, (void**)&output);
	if (output_len == -1 ||
	    lldpd_port_unserialize(output, output_len, &cloned) <= 0) {
		log_warnx("alloc", "unable to clone default port");
		free(output);
		return -1;
	}
	memcpy(destination, cloned, sizeof(struct lldpd_port));
	free(cloned);
	free(output);
#ifdef ENABLE_DOT1
	marshal_repair_tailq(lldpd_vlan, &destination->p_vlans, v_entries);
	marshal_repair_tailq(lldpd_ppvid, &destination->p_ppvids, p_entries);
	marshal_repair_tailq(lldpd_pi, &destination->p_pids, p_entries);
#endif
#ifdef ENABLE_CUSTOM
	marshal_repair_tailq(lldpd_custom, &destination->p_custom_list, next);
#endif
	return 0;
}

struct lldpd_hardware *
lldpd_alloc_hardware(struct lldpd *cfg, char *name, int index)
{
	struct lldpd_hardware *hardware;

	log_debug("alloc", "allocate a new local port (%s)", name);

	if ((hardware = (struct lldpd_hardware *)
		calloc(1, sizeof(struct lldpd_hardware))) == NULL)
		return NULL;

	/* Clone default local port */
	if (lldpd_clone_port(&hardware->h_lport, cfg->g_default_local_port) == -1) {
		log_warnx("alloc", "unable to clone default port");
		free(hardware);
		return NULL;
	}

	hardware->h_cfg = cfg;
	strlcpy(hardware->h_ifname, name, sizeof(hardware->h_ifname));
	hardware->h_ifindex = index;
	hardware->h_lport.p_chassis = LOCAL_CHASSIS(cfg);
	hardware->h_lport.p_chassis->c_refcount++;
	TAILQ_INIT(&hardware->h_rports);

#ifdef ENABLE_LLDPMED
	if (LOCAL_CHASSIS(cfg)->c_med_cap_available) {
		hardware->h_lport.p_med_cap_enabled = LLDP_MED_CAP_CAP;
		if (!cfg->g_config.c_noinventory)
			hardware->h_lport.p_med_cap_enabled |= LLDP_MED_CAP_IV;
	}
#endif

	levent_hardware_init(hardware);
	return hardware;
}

struct lldpd_mgmt *
lldpd_alloc_mgmt(int family, void *addrptr, size_t addrsize, u_int32_t iface)
{
	struct lldpd_mgmt *mgmt;

	log_debug("alloc", "allocate a new management address (family: %d)", family);

	if (family <= LLDPD_AF_UNSPEC || family >= LLDPD_AF_LAST) {
		errno = EAFNOSUPPORT;
		return NULL;
	}
	if (addrsize > LLDPD_MGMT_MAXADDRSIZE) {
		errno = EOVERFLOW;
		return NULL;
	}
	mgmt = calloc(1, sizeof(struct lldpd_mgmt));
	if (mgmt == NULL) {
		errno = ENOMEM;
		return NULL;
	}
	mgmt->m_family = family;
	memcpy(&mgmt->m_addr, addrptr, addrsize);
	mgmt->m_addrsize = addrsize;
	mgmt->m_iface = iface;
	return mgmt;
}

void
lldpd_hardware_cleanup(struct lldpd *cfg, struct lldpd_hardware *hardware)
{
	log_debug("alloc", "cleanup hardware port %s", hardware->h_ifname);

	free(hardware->h_lport_previous);
	free(hardware->h_lchassis_previous_id);
	free(hardware->h_lport_previous_id);
	lldpd_port_cleanup(&hardware->h_lport, 1);
	if (hardware->h_ops && hardware->h_ops->cleanup)
		hardware->h_ops->cleanup(cfg, hardware);
	levent_hardware_release(hardware);
	free(hardware);
}

static void
lldpd_display_neighbors(struct lldpd *cfg)
{
	if (!cfg->g_config.c_set_ifdescr) return;
	struct lldpd_hardware *hardware;
	TAILQ_FOREACH(hardware, &cfg->g_hardware, h_entries) {
		struct lldpd_port *port;
		char *description;
		const char *neighbor = NULL;
		unsigned neighbors = 0;
		TAILQ_FOREACH(port, &hardware->h_rports, p_entries) {
			if (SMART_HIDDEN(port)) continue;
			neighbors++;
			neighbor = port->p_chassis->c_name;
		}
		if (neighbors == 0)
			priv_iface_description(hardware->h_ifname,
			    "");
		else if (neighbors == 1 && neighbor && *neighbor != '\0') {
			if (asprintf(&description, "%s",
				neighbor) != -1) {
				priv_iface_description(hardware->h_ifname, description);
				free(description);
			}
		} else {
			if (asprintf(&description, "%d neighbor%s",
				neighbors, (neighbors > 1)?"s":"") != -1) {
				priv_iface_description(hardware->h_ifname,
				    description);
				free(description);
			}
		}
	}
}

static void
lldpd_count_neighbors(struct lldpd *cfg)
{
#if HAVE_SETPROCTITLE
	struct lldpd_chassis *chassis;
	const char *neighbor;
	unsigned neighbors = 0;
	TAILQ_FOREACH(chassis, &cfg->g_chassis, c_entries) {
		neighbors++;
		neighbor = chassis->c_name;
	}
	neighbors--;
	if (neighbors == 0)
		setproctitle("no neighbor.");
	else if (neighbors == 1 && neighbor && *neighbor != '\0')
		setproctitle("connected to %s.", neighbor);
	else
		setproctitle("%d neighbor%s.", neighbors,
		    (neighbors > 1)?"s":"");
#endif
	lldpd_display_neighbors(cfg);
}

static void
notify_clients_deletion(struct lldpd_hardware *hardware,
    struct lldpd_port *rport)
{
	TRACE(LLDPD_NEIGHBOR_DELETE(hardware->h_ifname,
		rport->p_chassis->c_name,
		rport->p_descr));
	levent_ctl_notify(hardware->h_ifname, NEIGHBOR_CHANGE_DELETED,
	    rport);
#ifdef USE_SNMP
	agent_notify(hardware, NEIGHBOR_CHANGE_DELETED, rport);
#endif
}

static void
lldpd_reset_timer(struct lldpd *cfg)
{
	/* Reset timer for ports that have been changed. */
	struct lldpd_hardware *hardware;
	TAILQ_FOREACH(hardware, &cfg->g_hardware, h_entries) {
		/* We keep a flat copy of the local port to see if there is any
		 * change. To do this, we zero out fields that are not
		 * significant, marshal the port, then restore. */
		struct lldpd_port *port = &hardware->h_lport;
		/* Take the current flags into account to detect a change. */
		port->_p_hardware_flags = hardware->h_flags;
		u_int8_t *output = NULL;
		ssize_t output_len;
		char save[LLDPD_PORT_START_MARKER];
		memcpy(save, port, sizeof(save));
		/* coverity[suspicious_sizeof]
		   We intentionally partially memset port */
		memset(port, 0, sizeof(save));
		output_len = lldpd_port_serialize(port, (void**)&output);
		memcpy(port, save, sizeof(save));
		if (output_len == -1) {
			log_warnx("localchassis",
			    "unable to serialize local port %s to check for differences",
			    hardware->h_ifname);
			continue;
		}

		/* Compare with the previous value */
		if (hardware->h_lport_previous &&
		    output_len == hardware->h_lport_previous_len &&
		    !memcmp(output, hardware->h_lport_previous, output_len)) {
			log_debug("localchassis",
			    "no change detected for port %s",
			    hardware->h_ifname);
		} else {
			log_debug("localchassis",
			    "change detected for port %s, resetting its timer",
			    hardware->h_ifname);
			levent_schedule_pdu(hardware);
		}

		/* Update the value */
		free(hardware->h_lport_previous);
		hardware->h_lport_previous = output;
		hardware->h_lport_previous_len = output_len;
	}
}

static void
lldpd_all_chassis_cleanup(struct lldpd *cfg)
{
	struct lldpd_chassis *chassis, *chassis_next;
	log_debug("localchassis", "cleanup all chassis");

	for (chassis = TAILQ_FIRST(&cfg->g_chassis); chassis;
	     chassis = chassis_next) {
		chassis_next = TAILQ_NEXT(chassis, c_entries);
		if (chassis->c_refcount == 0) {
			TAILQ_REMOVE(&cfg->g_chassis, chassis, c_entries);
			lldpd_chassis_cleanup(chassis, 1);
		}
	}
}

void
lldpd_cleanup(struct lldpd *cfg)
{
	struct lldpd_hardware *hardware, *hardware_next;

	log_debug("localchassis", "cleanup all ports");

	for (hardware = TAILQ_FIRST(&cfg->g_hardware); hardware != NULL;
	     hardware = hardware_next) {
		hardware_next = TAILQ_NEXT(hardware, h_entries);
		if (!hardware->h_flags) {
			TRACE(LLDPD_INTERFACES_DELETE(hardware->h_ifname));
			TAILQ_REMOVE(&cfg->g_hardware, hardware, h_entries);
			lldpd_remote_cleanup(hardware, notify_clients_deletion, 1);
			lldpd_hardware_cleanup(cfg, hardware);
		} else {
			lldpd_remote_cleanup(hardware, notify_clients_deletion,
			    !(hardware->h_flags & IFF_RUNNING));
		}
	}

	levent_schedule_cleanup(cfg);
	lldpd_all_chassis_cleanup(cfg);
	lldpd_count_neighbors(cfg);
}

/* Update chassis `ochassis' with values from `chassis'. The later one is not
   expected to be part of a list! It will also be wiped from memory. */
static void
lldpd_move_chassis(struct lldpd_chassis *ochassis,
    struct lldpd_chassis *chassis) {
	struct lldpd_mgmt *mgmt, *mgmt_next;

	/* We want to keep refcount, index and list stuff from the current
	 * chassis */
	TAILQ_ENTRY(lldpd_chassis) entries;
	int refcount = ochassis->c_refcount;
	int index = ochassis->c_index;
	memcpy(&entries, &ochassis->c_entries,
	    sizeof(entries));
	lldpd_chassis_cleanup(ochassis, 0);

	/* Make the copy. */
	/* WARNING: this is a kludgy hack, we need in-place copy and cannot use
	 * marshaling. */
	memcpy(ochassis, chassis, sizeof(struct lldpd_chassis));
	TAILQ_INIT(&ochassis->c_mgmt);

	/* Copy of management addresses */
	for (mgmt = TAILQ_FIRST(&chassis->c_mgmt);
	     mgmt != NULL;
	     mgmt = mgmt_next) {
		mgmt_next = TAILQ_NEXT(mgmt, m_entries);
		TAILQ_REMOVE(&chassis->c_mgmt, mgmt, m_entries);
		TAILQ_INSERT_TAIL(&ochassis->c_mgmt, mgmt, m_entries);
	}

	/* Restore saved values */
	ochassis->c_refcount = refcount;
	ochassis->c_index = index;
	memcpy(&ochassis->c_entries, &entries, sizeof(entries));

	/* Get rid of the new chassis */
	free(chassis);
}

static int
lldpd_guess_type(struct lldpd *cfg, char *frame, int s)
{
	int i;
	if (s < ETHER_ADDR_LEN)
		return -1;
	for (i=0; cfg->g_protocols[i].mode != 0; i++) {
		if (!cfg->g_protocols[i].enabled)
			continue;
		if (cfg->g_protocols[i].guess == NULL) {
			if (memcmp(frame, cfg->g_protocols[i].mac1, ETHER_ADDR_LEN) == 0 ||
			    memcmp(frame, cfg->g_protocols[i].mac2, ETHER_ADDR_LEN) == 0 ||
			    memcmp(frame, cfg->g_protocols[i].mac3, ETHER_ADDR_LEN) == 0) {
				log_debug("decode", "guessed protocol is %s (from MAC address)",
				    cfg->g_protocols[i].name);
				return cfg->g_protocols[i].mode;
			}
		} else {
			if (cfg->g_protocols[i].guess(frame, s)) {
				log_debug("decode", "guessed protocol is %s (from detector function)",
				    cfg->g_protocols[i].name);
				return cfg->g_protocols[i].mode;
			}
		}
	}
	return -1;
}

static void
lldpd_decode(struct lldpd *cfg, char *frame, int s,
    struct lldpd_hardware *hardware)
{
	int i;
	struct lldpd_chassis *chassis, *ochassis = NULL;
	struct lldpd_port *port, *oport = NULL, *aport;
	int guess = LLDPD_MODE_LLDP;

	log_debug("decode", "decode a received frame on %s",
	    hardware->h_ifname);

	if (s < sizeof(struct ether_header) + 4) {
		/* Too short, just discard it */
		hardware->h_rx_discarded_cnt++;
		return;
	}

	/* Decapsulate VLAN frames */
	struct ether_header eheader;
	memcpy(&eheader, frame, sizeof(struct ether_header));
	if (eheader.ether_type == htons(ETHERTYPE_VLAN)) {
		/* VLAN decapsulation means to shift 4 bytes left the frame from
		 * offset 2*ETHER_ADDR_LEN */
		memmove(frame + 2*ETHER_ADDR_LEN, frame + 2*ETHER_ADDR_LEN + 4, s - 2*ETHER_ADDR_LEN);
		s -= 4;
	}

	TAILQ_FOREACH(oport, &hardware->h_rports, p_entries) {
		if ((oport->p_lastframe != NULL) &&
		    (oport->p_lastframe->size == s) &&
		    (memcmp(oport->p_lastframe->frame, frame, s) == 0)) {
			/* Already received the same frame */
			log_debug("decode", "duplicate frame, no need to decode");
			oport->p_lastupdate = time(NULL);
			return;
		}
	}

	guess = lldpd_guess_type(cfg, frame, s);
	for (i=0; cfg->g_protocols[i].mode != 0; i++) {
		if (!cfg->g_protocols[i].enabled)
			continue;
		if (cfg->g_protocols[i].mode == guess) {
			log_debug("decode", "using decode function for %s protocol",
			    cfg->g_protocols[i].name);
			if (cfg->g_protocols[i].decode(cfg, frame,
				s, hardware, &chassis, &port) == -1) {
				log_debug("decode", "function for %s protocol did not decode this frame",
				    cfg->g_protocols[i].name);
				hardware->h_rx_discarded_cnt++;
				return;
			}
			chassis->c_protocol = port->p_protocol =
			    cfg->g_protocols[i].mode;
			break;
			}
	}
	if (cfg->g_protocols[i].mode == 0) {
		log_debug("decode", "unable to guess frame type on %s",
		    hardware->h_ifname);
		return;
	}
	TRACE(LLDPD_FRAME_DECODED(
		    hardware->h_ifname,
		    cfg->g_protocols[i].name,
		    chassis->c_name,
		    port->p_descr));

	/* Do we already have the same MSAP somewhere? */
	int count = 0;
	log_debug("decode", "search for the same MSAP");
	TAILQ_FOREACH(oport, &hardware->h_rports, p_entries) {
		if (port->p_protocol == oport->p_protocol) {
			count++;
			if ((port->p_id_subtype == oport->p_id_subtype) &&
			    (port->p_id_len == oport->p_id_len) &&
			    (memcmp(port->p_id, oport->p_id, port->p_id_len) == 0) &&
			    (chassis->c_id_subtype == oport->p_chassis->c_id_subtype) &&
			    (chassis->c_id_len == oport->p_chassis->c_id_len) &&
			    (memcmp(chassis->c_id, oport->p_chassis->c_id,
				chassis->c_id_len) == 0)) {
				ochassis = oport->p_chassis;
				log_debug("decode", "MSAP is already known");
				break;
			}
		}
	}
	/* Do we have room for a new MSAP? */
	if (!oport && cfg->g_config.c_max_neighbors) {
	    if (count == (cfg->g_config.c_max_neighbors - 1)) {
		log_debug("decode",
		    "max neighbors %d reached for port %s, "
		    "dropping any new ones silently",
		    cfg->g_config.c_max_neighbors,
		    hardware->h_ifname);
	    } else if (count > cfg->g_config.c_max_neighbors - 1) {
		log_debug("decode",
		    "too many neighbors for port %s, drop this new one",
		    hardware->h_ifname);
		lldpd_port_cleanup(port, 1);
		lldpd_chassis_cleanup(chassis, 1);
		free(port);
		return;
	    }
	}
	/* No, but do we already know the system? */
	if (!oport) {
		log_debug("decode", "MSAP is unknown, search for the chassis");
		TAILQ_FOREACH(ochassis, &cfg->g_chassis, c_entries) {
			if ((chassis->c_protocol == ochassis->c_protocol) &&
			    (chassis->c_id_subtype == ochassis->c_id_subtype) &&
			    (chassis->c_id_len == ochassis->c_id_len) &&
			    (memcmp(chassis->c_id, ochassis->c_id,
				chassis->c_id_len) == 0))
			break;
		}
	}

	if (oport) {
		/* The port is known, remove it before adding it back */
		TAILQ_REMOVE(&hardware->h_rports, oport, p_entries);
		lldpd_port_cleanup(oport, 1);
		free(oport);
	}
	if (ochassis) {
		lldpd_move_chassis(ochassis, chassis);
		chassis = ochassis;
	} else {
		/* Chassis not known, add it */
		log_debug("decode", "unknown chassis, add it to the list");
		chassis->c_index = ++cfg->g_lastrid;
		chassis->c_refcount = 0;
		TAILQ_INSERT_TAIL(&cfg->g_chassis, chassis, c_entries);
		i = 0; TAILQ_FOREACH(ochassis, &cfg->g_chassis, c_entries) i++;
		log_debug("decode", "%d different systems are known", i);
	}
	/* Add port */
	port->p_lastchange = port->p_lastupdate = time(NULL);
	if ((port->p_lastframe = (struct lldpd_frame *)malloc(s +
		    sizeof(struct lldpd_frame))) != NULL) {
		port->p_lastframe->size = s;
		memcpy(port->p_lastframe->frame, frame, s);
	}
	TAILQ_INSERT_TAIL(&hardware->h_rports, port, p_entries);
	port->p_chassis = chassis;
	port->p_chassis->c_refcount++;
	/* Several cases are possible :
	     1. chassis is new, its refcount was 0. It is now attached
	        to this port, its refcount is 1.
	     2. chassis already exists and was attached to another
	        port, we increase its refcount accordingly.
	     3. chassis already exists and was attached to the same
	        port, its refcount was decreased with
	        lldpd_port_cleanup() and is now increased again.

	   In all cases, if the port already existed, it has been
	   freed with lldpd_port_cleanup() and therefore, the refcount
	   of the chassis that was attached to it is decreased.
	*/
	/* coverity[use_after_free]
	   TAILQ_REMOVE does the right thing */
	i = 0; TAILQ_FOREACH(aport, &hardware->h_rports, p_entries)
		i++;
	log_debug("decode", "%d neighbors for %s", i,
	    hardware->h_ifname);

	if (!oport) hardware->h_insert_cnt++;

	/* Notify */
	log_debug("decode", "send notifications for changes on %s",
	    hardware->h_ifname);
	if (oport) {
		TRACE(LLDPD_NEIGHBOR_UPDATE(hardware->h_ifname,
			chassis->c_name,
			port->p_descr,
			i));
		levent_ctl_notify(hardware->h_ifname, NEIGHBOR_CHANGE_UPDATED, port);
#ifdef USE_SNMP
		agent_notify(hardware, NEIGHBOR_CHANGE_UPDATED, port);
#endif
	} else {
		TRACE(LLDPD_NEIGHBOR_NEW(hardware->h_ifname,
			chassis->c_name,
			port->p_descr,
			i));
		levent_ctl_notify(hardware->h_ifname, NEIGHBOR_CHANGE_ADDED, port);
#ifdef USE_SNMP
		agent_notify(hardware, NEIGHBOR_CHANGE_ADDED, port);
#endif
	}

#ifdef ENABLE_LLDPMED
	if (!oport && port->p_chassis->c_med_type) {
		/* New neighbor, fast start */
		if (hardware->h_cfg->g_config.c_enable_fast_start &&
		    !hardware->h_tx_fast) {
			log_debug("decode", "%s: entering fast start due to "
			    "new neighbor", hardware->h_ifname);
			hardware->h_tx_fast = hardware->h_cfg->g_config.c_tx_fast_init;
		}

		levent_schedule_pdu(hardware);
	}
#endif

	return;
}

/* Get the output of lsb_release -s -d.  This is a slow function. It should be
   called once. It return NULL if any problem happens. Otherwise, this is a
   statically allocated buffer. The result includes the trailing \n  */
static char *
lldpd_get_lsb_release() {
	static char release[1024];
	char *const command[] = { "lsb_release", "-s", "-d", NULL };
	int pid, status, devnull, count;
	int pipefd[2];

	log_debug("localchassis", "grab LSB release");

	if (pipe(pipefd)) {
		log_warn("localchassis", "unable to get a pair of pipes");
		return NULL;
	}

	pid = vfork();
	switch (pid) {
	case -1:
		log_warn("localchassis", "unable to fork");
		return NULL;
	case 0:
		/* Child, exec lsb_release */
		close(pipefd[0]);
		if ((devnull = open("/dev/null", O_RDWR, 0)) != -1) {
			dup2(devnull, STDIN_FILENO);
			dup2(devnull, STDERR_FILENO);
			dup2(pipefd[1], STDOUT_FILENO);
			if (devnull > 2) close(devnull);
			if (pipefd[1] > 2) close(pipefd[1]);
			execvp("lsb_release", command);
		}
		_exit(127);
		break;
	default:
		/* Father, read the output from the children */
		close(pipefd[1]);
		count = 0;
		do {
			status = read(pipefd[0], release+count, sizeof(release)-count);
			if ((status == -1) && (errno == EINTR)) continue;
			if (status > 0)
				count += status;
		} while (count < sizeof(release) && (status > 0));
		if (status < 0) {
			log_info("localchassis", "unable to read from lsb_release");
			close(pipefd[0]);
			waitpid(pid, &status, 0);
			return NULL;
		}
		close(pipefd[0]);
		if (count >= sizeof(release)) {
			log_info("localchassis", "output of lsb_release is too large");
			waitpid(pid, &status, 0);
			return NULL;
		}
		status = -1;
		if (waitpid(pid, &status, 0) != pid)
			return NULL;
		if (!WIFEXITED(status) || (WEXITSTATUS(status) != 0)) {
			log_info("localchassis", "lsb_release information not available");
			return NULL;
		}
		if (!count) {
			log_info("localchassis", "lsb_release returned an empty string");
			return NULL;
		}
		release[count] = '\0';
		return release;
	}
	/* Should not be here */
	return NULL;
}

/* Same like lldpd_get_lsb_release but reads /etc/os-release for PRETTY_NAME=. */
static char *
lldpd_get_os_release() {
	static char release[1024];
	char line[1024];
	char *key, *val;
	char *ptr1 = release;

	log_debug("localchassis", "grab OS release");
	FILE *fp = fopen("/etc/os-release", "r");
	if (!fp) {
		log_debug("localchassis", "could not open /etc/os-release");
		fp = fopen("/usr/lib/os-release", "r");
	}
	if (!fp) {
		log_info("localchassis",
		    "could not open either /etc/os-release or /usr/lib/os-release");
		return NULL;
	}

	while ((fgets(line, sizeof(line), fp) != NULL)) {
		key = strtok(line, "=");
		val = strtok(NULL, "=");

		if (strncmp(key, "PRETTY_NAME", sizeof(line)) == 0) {
			strlcpy(release, val, sizeof(line));
			break;
		}
	}
	fclose(fp);

	/* Remove trailing newline and all " in the string. */
	ptr1 = release + strlen(release) - 1;
	while (ptr1 != release &&
	    ((*ptr1 == '"') || (*ptr1 == '\n'))) {
		*ptr1 = '\0';
		ptr1--;
	}
	if (release[0] == '"')
		return release+1;
	return release;
}

static void
lldpd_hide_ports(struct lldpd *cfg, struct lldpd_hardware *hardware, int mask) {
	struct lldpd_port *port;
	int protocols[LLDPD_MODE_MAX+1];
	char buffer[256];
	int i, j, k, found;
	unsigned int min;

	log_debug("smartfilter", "apply smart filter for port %s",
		hardware->h_ifname);

	/* Compute the number of occurrences of each protocol */
	for (i = 0; i <= LLDPD_MODE_MAX; i++) protocols[i] = 0;
	TAILQ_FOREACH(port, &hardware->h_rports, p_entries)
		protocols[port->p_protocol]++;

	/* Turn the protocols[] array into an array of
	   enabled/disabled protocols. 1 means enabled, 0
	   means disabled. */
	min = (unsigned int)-1;
	for (i = 0; i <= LLDPD_MODE_MAX; i++)
		if (protocols[i] && (protocols[i] < min))
			min = protocols[i];
	found = 0;
	for (i = 0; i <= LLDPD_MODE_MAX; i++)
		if ((protocols[i] == min) && !found) {
			/* If we need a tie breaker, we take
			   the first protocol only */
			if (cfg->g_config.c_smart & mask &
			    (SMART_OUTGOING_ONE_PROTO | SMART_INCOMING_ONE_PROTO))
				found = 1;
			protocols[i] = 1;
		} else protocols[i] = 0;

	/* We set the p_hidden flag to 1 if the protocol is disabled */
	TAILQ_FOREACH(port, &hardware->h_rports, p_entries) {
		if (mask == SMART_OUTGOING)
			port->p_hidden_out = protocols[port->p_protocol]?0:1;
		else
			port->p_hidden_in = protocols[port->p_protocol]?0:1;
	}

	/* If we want only one neighbor, we take the first one */
	if (cfg->g_config.c_smart & mask &
	    (SMART_OUTGOING_ONE_NEIGH | SMART_INCOMING_ONE_NEIGH)) {
		found = 0;
		TAILQ_FOREACH(port, &hardware->h_rports, p_entries) {
			if (mask == SMART_OUTGOING) {
				if (found) port->p_hidden_out = 1;
				if (!port->p_hidden_out)
					found = 1;
			}
			if (mask == SMART_INCOMING) {
				if (found) port->p_hidden_in = 1;
				if (!port->p_hidden_in)
					found = 1;
			}
		}
	}

	/* Print a debug message summarizing the operation */
	for (i = 0; i <= LLDPD_MODE_MAX; i++) protocols[i] = 0;
	k = j = 0;
	TAILQ_FOREACH(port, &hardware->h_rports, p_entries) {
		if (!(((mask == SMART_OUTGOING) && port->p_hidden_out) ||
		      ((mask == SMART_INCOMING) && port->p_hidden_in))) {
			k++;
			protocols[port->p_protocol] = 1;
		}
		j++;
	}
	buffer[0] = '\0';
	for (i=0; cfg->g_protocols[i].mode != 0; i++) {
		if (cfg->g_protocols[i].enabled && protocols[cfg->g_protocols[i].mode]) {
			if (strlen(buffer) +
			    strlen(cfg->g_protocols[i].name) + 3 > sizeof(buffer)) {
				/* Unlikely, our buffer is too small */
				memcpy(buffer + sizeof(buffer) - 4, "...", 4);
				break;
			}
			if (buffer[0])
				strncat(buffer, ", ", 2);
			strncat(buffer, cfg->g_protocols[i].name, strlen(cfg->g_protocols[i].name));
		}
	}
	log_debug("smartfilter", "%s: %s: %d visible neighbors (out of %d)",
	    hardware->h_ifname,
	    (mask == SMART_OUTGOING)?"out filter":"in filter",
	    k, j);
	log_debug("smartfilter", "%s: protocols: %s",
	    hardware->h_ifname, buffer[0]?buffer:"(none)");
}

/* Hide unwanted ports depending on smart mode set by the user */
static void
lldpd_hide_all(struct lldpd *cfg)
{
	struct lldpd_hardware *hardware;

	if (!cfg->g_config.c_smart)
		return;
	log_debug("smartfilter", "apply smart filter results on all ports");
	TAILQ_FOREACH(hardware, &cfg->g_hardware, h_entries) {
		if (cfg->g_config.c_smart & SMART_INCOMING_FILTER)
			lldpd_hide_ports(cfg, hardware, SMART_INCOMING);
		if (cfg->g_config.c_smart & SMART_OUTGOING_FILTER)
			lldpd_hide_ports(cfg, hardware, SMART_OUTGOING);
	}
}

void
lldpd_recv(struct lldpd *cfg, struct lldpd_hardware *hardware, int fd)
{
	char *buffer = NULL;
	int n;
	log_debug("receive", "receive a frame on %s",
	    hardware->h_ifname);
	if ((buffer = (char *)malloc(hardware->h_mtu)) == NULL) {
		log_warn("receive", "failed to alloc reception buffer");
		return;
	}
	if ((n = hardware->h_ops->recv(cfg, hardware,
		    fd, buffer,
		    hardware->h_mtu)) == -1) {
		log_debug("receive", "discard frame received on %s",
		    hardware->h_ifname);
		free(buffer);
		return;
	}
	if (hardware->h_lport.p_disable_rx) {
		log_debug("receive", "RX disabled, ignore the frame on %s",
		    hardware->h_ifname);
		free(buffer);
		return;
	}
	if (cfg->g_config.c_paused) {
		log_debug("receive", "paused, ignore the frame on %s",
			hardware->h_ifname);
		free(buffer);
		return;
	}
	hardware->h_rx_cnt++;
	log_debug("receive", "decode received frame on %s",
	    hardware->h_ifname);
	TRACE(LLDPD_FRAME_RECEIVED(hardware->h_ifname, buffer, (size_t)n));
	lldpd_decode(cfg, buffer, n, hardware);
	lldpd_hide_all(cfg); /* Immediatly hide */
	lldpd_count_neighbors(cfg);
	free(buffer);
}

static void
lldpd_send_shutdown(struct lldpd_hardware *hardware)
{
	struct lldpd *cfg = hardware->h_cfg;
	if (cfg->g_config.c_receiveonly || cfg->g_config.c_paused) return;
	if (hardware->h_lport.p_disable_tx) return;
	if ((hardware->h_flags & IFF_RUNNING) == 0)
		return;

	/* It's safe to call `lldp_send_shutdown()` because shutdown LLDPU will
	 * only be emitted if LLDP was sent on that port. */
	if (lldp_send_shutdown(hardware->h_cfg, hardware) != 0)
		log_warnx("send", "unable to send shutdown LLDPDU on %s",
		    hardware->h_ifname);
}

void
lldpd_send(struct lldpd_hardware *hardware)
{
	struct lldpd *cfg = hardware->h_cfg;
	struct lldpd_port *port;
	int i, sent;

	if (cfg->g_config.c_receiveonly || cfg->g_config.c_paused) return;
	if (hardware->h_lport.p_disable_tx) return;
	if ((hardware->h_flags & IFF_RUNNING) == 0)
		return;

	log_debug("send", "send PDU on %s", hardware->h_ifname);
	sent = 0;
	for (i=0; cfg->g_protocols[i].mode != 0; i++) {
		if (!cfg->g_protocols[i].enabled)
			continue;
		/* We send only if we have at least one remote system
		 * speaking this protocol or if the protocol is forced */
		if (cfg->g_protocols[i].enabled > 1) {
			cfg->g_protocols[i].send(cfg, hardware);
			sent++;
			continue;
		}
		TAILQ_FOREACH(port, &hardware->h_rports, p_entries) {
			/* If this remote port is disabled, we don't
			 * consider it */
			if (port->p_hidden_out)
				continue;
			if (port->p_protocol ==
			    cfg->g_protocols[i].mode) {
				TRACE(LLDPD_FRAME_SEND(hardware->h_ifname,
					cfg->g_protocols[i].name));
				log_debug("send", "send PDU on %s with protocol %s",
				    hardware->h_ifname,
				    cfg->g_protocols[i].name);
				cfg->g_protocols[i].send(cfg,
				    hardware);
				sent++;
				break;
			}
		}
	}

	if (!sent) {
		/* Nothing was sent for this port, let's speak the first
		 * available protocol. */
		for (i = 0; cfg->g_protocols[i].mode != 0; i++) {
			if (!cfg->g_protocols[i].enabled) continue;
			TRACE(LLDPD_FRAME_SEND(hardware->h_ifname,
				cfg->g_protocols[i].name));
			log_debug("send", "fallback to protocol %s for %s",
			    cfg->g_protocols[i].name, hardware->h_ifname);
			cfg->g_protocols[i].send(cfg,
			    hardware);
			break;
		}
		if (cfg->g_protocols[i].mode == 0)
			log_warnx("send", "no protocol enabled, dunno what to send");
	}
}

#ifdef ENABLE_LLDPMED
static void
lldpd_med(struct lldpd_chassis *chassis)
{
	static short int once = 0;
	if (!once) {
		chassis->c_med_hw = dmi_hw();
		chassis->c_med_fw = dmi_fw();
		chassis->c_med_sn = dmi_sn();
		chassis->c_med_manuf = dmi_manuf();
		chassis->c_med_model = dmi_model();
		chassis->c_med_asset = dmi_asset();
		once = 1;
	}
}
#endif

static int
lldpd_routing_enabled(struct lldpd *cfg)
{
	int routing;

	if ((LOCAL_CHASSIS(cfg)->c_cap_available & LLDP_CAP_ROUTER) == 0)
		return 0;

	if ((routing = interfaces_routing_enabled(cfg)) == -1) {
		log_debug("localchassis", "unable to check if routing is enabled");
		return 0;
	}
	return routing;
}

static void
lldpd_update_localchassis(struct lldpd *cfg)
{
	struct utsname un;
	char *hp;

	log_debug("localchassis", "update information for local chassis");
	assert(LOCAL_CHASSIS(cfg) != NULL);

	/* Set system name and description */
	if (uname(&un) < 0)
		fatal("localchassis", "failed to get system information");
	if (cfg->g_config.c_hostname) {
		log_debug("localchassis", "use overridden system name `%s`", cfg->g_config.c_hostname);
		hp = cfg->g_config.c_hostname;
	} else {
		if ((hp = priv_gethostname()) == NULL)
			fatal("localchassis", "failed to get system name");
	}
	free(LOCAL_CHASSIS(cfg)->c_name);
	free(LOCAL_CHASSIS(cfg)->c_descr);
	if ((LOCAL_CHASSIS(cfg)->c_name = strdup(hp)) == NULL)
		fatal("localchassis", NULL);
        if (cfg->g_config.c_description) {
		log_debug("localchassis", "use overridden description `%s`", cfg->g_config.c_description);
                if (asprintf(&LOCAL_CHASSIS(cfg)->c_descr, "%s",
			cfg->g_config.c_description) == -1)
			fatal("localchassis", "failed to set full system description");
        } else {
	        if (cfg->g_config.c_advertise_version) {
			log_debug("localchassis", "advertise system version");
		        if (asprintf(&LOCAL_CHASSIS(cfg)->c_descr, "%s %s %s %s %s",
			        cfg->g_lsb_release?cfg->g_lsb_release:"",
				un.sysname, un.release, un.version, un.machine)
                                == -1)
			        fatal("localchassis", "failed to set full system description");
	        } else {
			log_debug("localchassis", "do not advertise system version");
		        if (asprintf(&LOCAL_CHASSIS(cfg)->c_descr, "%s",
                                cfg->g_lsb_release?cfg->g_lsb_release:un.sysname) == -1)
			        fatal("localchassis", "failed to set minimal system description");
	        }
        }
	if (cfg->g_config.c_platform == NULL)
		cfg->g_config.c_platform = strdup(un.sysname);

	/* Check routing */
	if (lldpd_routing_enabled(cfg)) {
		log_debug("localchassis", "routing is enabled, enable router capability");
		LOCAL_CHASSIS(cfg)->c_cap_enabled |= LLDP_CAP_ROUTER;
	} else
		LOCAL_CHASSIS(cfg)->c_cap_enabled &= ~LLDP_CAP_ROUTER;

#ifdef ENABLE_LLDPMED
	if (LOCAL_CHASSIS(cfg)->c_cap_available & LLDP_CAP_TELEPHONE)
		LOCAL_CHASSIS(cfg)->c_cap_enabled |= LLDP_CAP_TELEPHONE;
	lldpd_med(LOCAL_CHASSIS(cfg));
	free(LOCAL_CHASSIS(cfg)->c_med_sw);
	if (cfg->g_config.c_advertise_version)
		LOCAL_CHASSIS(cfg)->c_med_sw = strdup(un.release);
	else
		LOCAL_CHASSIS(cfg)->c_med_sw = strdup("Unknown");
#endif
	if ((LOCAL_CHASSIS(cfg)->c_cap_available & LLDP_CAP_STATION) &&
		(LOCAL_CHASSIS(cfg)->c_cap_enabled == 0))
		LOCAL_CHASSIS(cfg)->c_cap_enabled = LLDP_CAP_STATION;
	else if (LOCAL_CHASSIS(cfg)->c_cap_enabled != LLDP_CAP_STATION)
		LOCAL_CHASSIS(cfg)->c_cap_enabled &= ~LLDP_CAP_STATION;

	/* Set chassis ID if needed. This is only done if chassis ID
	   has not been set previously (with the MAC address of an
	   interface for example)
	*/
	if (LOCAL_CHASSIS(cfg)->c_id == NULL) {
		log_debug("localchassis", "no chassis ID is currently set, use chassis name");
		if (!(LOCAL_CHASSIS(cfg)->c_id = strdup(LOCAL_CHASSIS(cfg)->c_name)))
			fatal("localchassis", NULL);
		LOCAL_CHASSIS(cfg)->c_id_len = strlen(LOCAL_CHASSIS(cfg)->c_name);
		LOCAL_CHASSIS(cfg)->c_id_subtype = LLDP_CHASSISID_SUBTYPE_LOCAL;
	}
}

void
lldpd_update_localports(struct lldpd *cfg)
{
	struct lldpd_hardware *hardware;

	log_debug("localchassis", "update information for local ports");

	/* h_flags is set to 0 for each port. If the port is updated, h_flags
	 * will be set to a non-zero value. This will allow us to clean up any
	 * non up-to-date port */
	TAILQ_FOREACH(hardware, &cfg->g_hardware, h_entries)
	    hardware->h_flags = 0;

	TRACE(LLDPD_INTERFACES_UPDATE());
	interfaces_update(cfg);
	lldpd_cleanup(cfg);
	lldpd_reset_timer(cfg);
}

void
lldpd_loop(struct lldpd *cfg)
{
	/* Main loop.
	   1. Update local ports information
	   2. Update local chassis information
	*/
	log_debug("loop", "start new loop");
	LOCAL_CHASSIS(cfg)->c_cap_enabled = 0;
	/* Information for local ports is triggered even when it is possible to
	 * update them on some other event because we want to refresh them if we
	 * missed something. */
	log_debug("loop", "update information for local ports");
	lldpd_update_localports(cfg);
	log_debug("loop", "update information for local chassis");
	lldpd_update_localchassis(cfg);
	lldpd_count_neighbors(cfg);
}

static void
lldpd_exit(struct lldpd *cfg)
{
	struct lldpd_hardware *hardware, *hardware_next;
	log_debug("main", "exit lldpd");

	TAILQ_FOREACH(hardware, &cfg->g_hardware, h_entries)
		lldpd_send_shutdown(hardware);

	close(cfg->g_ctl);
	priv_ctl_cleanup(cfg->g_ctlname);
	log_debug("main", "cleanup hardware information");
	for (hardware = TAILQ_FIRST(&cfg->g_hardware); hardware != NULL;
	     hardware = hardware_next) {
		hardware_next = TAILQ_NEXT(hardware, h_entries);
		log_debug("main", "cleanup interface %s", hardware->h_ifname);
		lldpd_remote_cleanup(hardware, NULL, 1);
		lldpd_hardware_cleanup(cfg, hardware);
	}
	interfaces_cleanup(cfg);
	lldpd_port_cleanup(cfg->g_default_local_port, 1);
	lldpd_all_chassis_cleanup(cfg);
	free(cfg->g_default_local_port);
	free(cfg->g_config.c_platform);
	levent_shutdown(cfg);
}

/**
 * Run lldpcli to configure lldpd.
 *
 * @return PID of running lldpcli or -1 if error.
 */
static pid_t
lldpd_configure(int use_syslog, int debug, const char *path, const char *ctlname)
{
	pid_t lldpcli = vfork();
	int devnull;

	char sdebug[debug + 4];
	if (use_syslog)
		strlcpy(sdebug, "-s", 3);
	else {
		/* debug = 0 -> -sd */
		/* debug = 1 -> -sdd */
		/* debug = 2 -> -sddd */
		memset(sdebug, 'd', sizeof(sdebug));
		sdebug[debug + 3] = '\0';
		sdebug[0] = '-'; sdebug[1] = 's';
	}
	log_debug("main", "invoke %s %s", path, sdebug);

	switch (lldpcli) {
	case -1:
		log_warn("main", "unable to fork");
		return -1;
	case 0:
		/* Child, exec lldpcli */
		if ((devnull = open("/dev/null", O_RDWR, 0)) != -1) {
			dup2(devnull,   STDIN_FILENO);
			dup2(devnull,   STDOUT_FILENO);
			if (devnull > 2) close(devnull);

			execl(path, "lldpcli", sdebug,
			    "-u", ctlname,
			    "-C", SYSCONFDIR "/lldpd.conf",
			    "-C", SYSCONFDIR "/lldpd.d",
			    "resume",
			    (char *)NULL);
			log_warn("main", "unable to execute %s", path);
			log_warnx("main", "configuration is incomplete, lldpd needs to be unpaused");
		}
		_exit(127);
		break;
	default:
		/* Father, don't do anything stupid */
		return lldpcli;
	}
	/* Should not be here */
	return -1;
}

struct intint { int a; int b; };
static const struct intint filters[] = {
	{  0, 0 },
	{  1, SMART_INCOMING_FILTER | SMART_INCOMING_ONE_PROTO |
	      SMART_OUTGOING_FILTER | SMART_OUTGOING_ONE_PROTO },
	{  2, SMART_INCOMING_FILTER | SMART_INCOMING_ONE_PROTO },
	{  3, SMART_OUTGOING_FILTER | SMART_OUTGOING_ONE_PROTO },
	{  4, SMART_INCOMING_FILTER | SMART_OUTGOING_FILTER },
	{  5, SMART_INCOMING_FILTER },
	{  6, SMART_OUTGOING_FILTER },
	{  7, SMART_INCOMING_FILTER | SMART_INCOMING_ONE_PROTO | SMART_INCOMING_ONE_NEIGH |
	      SMART_OUTGOING_FILTER | SMART_OUTGOING_ONE_PROTO },
	{  8, SMART_INCOMING_FILTER | SMART_INCOMING_ONE_PROTO | SMART_INCOMING_ONE_NEIGH },
	{  9, SMART_INCOMING_FILTER | SMART_INCOMING_ONE_NEIGH |
	      SMART_OUTGOING_FILTER | SMART_OUTGOING_ONE_PROTO },
	{ 10, SMART_OUTGOING_FILTER | SMART_OUTGOING_ONE_NEIGH },
	{ 11, SMART_INCOMING_FILTER | SMART_INCOMING_ONE_NEIGH },
	{ 12, SMART_INCOMING_FILTER | SMART_INCOMING_ONE_NEIGH |
	      SMART_OUTGOING_FILTER | SMART_OUTGOING_ONE_NEIGH },
	{ 13, SMART_INCOMING_FILTER | SMART_INCOMING_ONE_NEIGH |
	      SMART_OUTGOING_FILTER },
	{ 14, SMART_INCOMING_FILTER | SMART_INCOMING_ONE_PROTO |
	      SMART_OUTGOING_FILTER | SMART_OUTGOING_ONE_NEIGH },
	{ 15, SMART_INCOMING_FILTER | SMART_INCOMING_ONE_PROTO |
	      SMART_OUTGOING_FILTER },
	{ 16, SMART_INCOMING_FILTER | SMART_INCOMING_ONE_PROTO | SMART_INCOMING_ONE_NEIGH |
	      SMART_OUTGOING_FILTER | SMART_OUTGOING_ONE_NEIGH },
	{ 17, SMART_INCOMING_FILTER | SMART_INCOMING_ONE_PROTO | SMART_INCOMING_ONE_NEIGH |
	      SMART_OUTGOING_FILTER },
	{ 18, SMART_INCOMING_FILTER |
	      SMART_OUTGOING_FILTER | SMART_OUTGOING_ONE_NEIGH },
	{ 19, SMART_INCOMING_FILTER |
	      SMART_OUTGOING_FILTER | SMART_OUTGOING_ONE_PROTO },
	{ -1, 0 }
};

#ifndef HOST_OS_OSX
/**
 * Tell if we have been started by upstart.
 */
static int
lldpd_started_by_upstart()
{
#ifdef HOST_OS_LINUX
	const char *upstartjob = getenv("UPSTART_JOB");
	if (!(upstartjob && !strcmp(upstartjob, "lldpd")))
		return 0;
	log_debug("main", "running with upstart, don't fork but stop");
	raise(SIGSTOP);
	unsetenv("UPSTART_JOB");
	return 1;
#else
	return 0;
#endif
}

/**
 * Tell if we have been started by systemd.
 */
static int
lldpd_started_by_systemd()
{
#ifdef HOST_OS_LINUX
	int fd = -1;
	const char *notifysocket = getenv("NOTIFY_SOCKET");
	if (!notifysocket ||
	    !strchr("@/", notifysocket[0]) ||
	    strlen(notifysocket) < 2)
		return 0;

	log_debug("main", "running with systemd, don't fork but signal ready");
	if ((fd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
		log_warn("main", "unable to open systemd notification socket %s",
		    notifysocket);
		return 0;
	}

	struct sockaddr_un su = { .sun_family = AF_UNIX };
	strlcpy(su.sun_path, notifysocket, sizeof(su.sun_path));
	if (notifysocket[0] == '@') su.sun_path[0] = 0;

	struct iovec iov = {
		.iov_base = "READY=1",
		.iov_len = strlen("READY=1")
	};
	struct msghdr hdr = {
		.msg_name = &su,
		.msg_namelen = offsetof(struct sockaddr_un, sun_path) + strlen(notifysocket),
		.msg_iov = &iov,
		.msg_iovlen = 1
	};
	unsetenv("NOTIFY_SOCKET");
	if (sendmsg(fd, &hdr, MSG_NOSIGNAL) < 0) {
		log_warn("main", "unable to send notification to systemd");
		close(fd);
		return 0;
	}
	close(fd);
	return 1;
#else
	return 0;
#endif
}
#endif

#ifdef HOST_OS_LINUX
static void
version_convert(const char *sversion, unsigned iversion[], size_t n)
{
	const char *p = sversion;
	char *end;
	for (size_t i = 0; i < n; i++) {
		iversion[i] = strtol(p, &end, 10);
		if (*end != '.') break;
		p = end + 1;
	}
}

static void
version_check(void)
{
	struct utsname uts;
	if (uname(&uts) == -1) return;
	unsigned version_min[3] = {};
	unsigned version_cur[3] = {};
	version_convert(uts.release, version_cur, 3);
	version_convert(MIN_LINUX_KERNEL_VERSION, version_min, 3);
	if (version_min[0] > version_cur[0] ||
	    (version_min[0] == version_cur[0] && version_min[1] > version_cur[1]) ||
	    (version_min[0] == version_cur[0] && version_min[1] == version_cur[1] &&
		version_min[2] > version_cur[2])) {
		log_warnx("lldpd", "minimal kernel version required is %s, got %s",
		    MIN_LINUX_KERNEL_VERSION, uts.release);
		log_warnx("lldpd", "lldpd may be unable to detect bonds and bridges correctly");
#ifndef ENABLE_OLDIES
		log_warnx("lldpd", "consider recompiling with --enable-oldies option");
#endif
	}
}
#else
static void version_check(void) {}
#endif

int
lldpd_main(int argc, char *argv[], char *envp[])
{
	struct lldpd *cfg;
	struct lldpd_chassis *lchassis;
	int ch, debug = 0, use_syslog = 1, daemonize = 1;
	const char *errstr;
#ifdef USE_SNMP
	int snmp = 0;
	const char *agentx = NULL;	/* AgentX socket */
#endif
	const char *ctlname = NULL;
	char *mgmtp = NULL;
	char *cidp = NULL;
	char *interfaces = NULL;
	/* We do not want more options here. Please add them in lldpcli instead
	 * unless there is a very good reason. Most command-line options will
	 * get deprecated at some point. */
	char *popt, opts[] =
		"H:vhkrdD:p:xX:m:u:4:6:I:C:p:M:P:S:iL:@                    ";
	int i, found, advertise_version = 1;
#ifdef ENABLE_LLDPMED
	int lldpmed = 0, noinventory = 0;
	int enable_fast_start = 1;
#endif
	char *descr_override = NULL;
	char *platform_override = NULL;
	char *lsb_release = NULL;
	const char *lldpcli = LLDPCLI_PATH;
	const char *pidfile = LLDPD_PID_FILE;
	int smart = 15;
	int receiveonly = 0, version = 0;
	int ctl;

#ifdef ENABLE_PRIVSEP
	/* Non privileged user */
	struct passwd *user;
	struct group *group;
	uid_t uid;
	gid_t gid;
#endif

	saved_argv = argv;

#if HAVE_SETPROCTITLE_INIT
	setproctitle_init(argc, argv, envp);
#endif

	/*
	 * Get and parse command line options
	 */
	if ((popt = strchr(opts, '@')) != NULL) {
		for (i=0;
		     protos[i].mode != 0 && *popt != '\0';
		     i++)
			*(popt++) = protos[i].arg;
		*popt = '\0';
	}
	while ((ch = getopt(argc, argv, opts)) != -1) {
		switch (ch) {
		case 'h':
			usage();
			break;
		case 'v':
			version++;
			break;
		case 'd':
			if (daemonize)
				daemonize = 0;
			else if (use_syslog)
				use_syslog = 0;
			else
				debug++;
			break;
		case 'D':
			log_accept(optarg);
			break;
		case 'p':
			pidfile = optarg;
			break;
		case 'r':
			receiveonly = 1;
			break;
		case 'm':
			if (mgmtp) {
				fprintf(stderr, "-m can only be used once\n");
				usage();
			}
			mgmtp = strdup(optarg);
			break;
		case 'u':
			if (ctlname) {
				fprintf(stderr, "-u can only be used once\n");
				usage();
			}
			ctlname = optarg;
			break;
		case 'I':
			if (interfaces) {
				fprintf(stderr, "-I can only be used once\n");
				usage();
			}
			interfaces = strdup(optarg);
			break;
		case 'C':
			if (cidp) {
				fprintf(stderr, "-C can only be used once\n");
				usage();
			}
			cidp = strdup(optarg);
			break;
		case 'L':
			if (strlen(optarg)) lldpcli = optarg;
			else lldpcli = NULL;
			break;
		case 'k':
			advertise_version = 0;
			break;
#ifdef ENABLE_LLDPMED
		case 'M':
			lldpmed = strtonum(optarg, 1, 4, &errstr);
			if (errstr) {
				fprintf(stderr, "-M requires an argument between 1 and 4\n");
				usage();
			}
			break;
		case 'i':
			noinventory = 1;
			break;
#else
		case 'M':
		case 'i':
			fprintf(stderr, "LLDP-MED support is not built-in\n");
			usage();
			break;
#endif
#ifdef USE_SNMP
		case 'x':
			snmp = 1;
			break;
		case 'X':
			if (agentx) {
				fprintf(stderr, "-X can only be used once\n");
				usage();
			}
			snmp = 1;
			agentx = optarg;
			break;
#else
		case 'x':
		case 'X':
			fprintf(stderr, "SNMP support is not built-in\n");
			usage();
#endif
			break;
                case 'S':
			if (descr_override) {
				fprintf(stderr, "-S can only be used once\n");
				usage();
			}
                        descr_override = strdup(optarg);
                        break;
		case 'P':
			if (platform_override) {
				fprintf(stderr, "-P can only be used once\n");
				usage();
			}
			platform_override = strdup(optarg);
			break;
		case 'H':
			smart = strtonum(optarg, 0, sizeof(filters)/sizeof(filters[0]),
			    &errstr);
			if (errstr) {
				fprintf(stderr, "-H requires an int between 0 and %zu\n",
				    sizeof(filters)/sizeof(filters[0]));
				usage();
			}
			break;
		default:
			found = 0;
			for (i=0; protos[i].mode != 0; i++) {
				if (ch == protos[i].arg) {
					found = 1;
					protos[i].enabled++;
				}
			}
			if (!found)
				usage();
		}
	}

	if (version) {
		version_display(stdout, "lldpd", version > 1);
		exit(0);
	}

	if (ctlname == NULL) ctlname = LLDPD_CTL_SOCKET;

	/* Set correct smart mode */
	for (i=0; (filters[i].a != -1) && (filters[i].a != smart); i++);
	if (filters[i].a == -1) {
		fprintf(stderr, "Incorrect mode for -H\n");
		usage();
	}
	smart = filters[i].b;

	log_init(use_syslog, debug, __progname);
	tzset();		/* Get timezone info before chroot */
	if (use_syslog && daemonize) {
		/* So, we use syslog and we daemonize (or we are started by
		 * upstart/systemd). No need to continue writing to stdout. */
		int fd;
		if ((fd = open("/dev/null", O_RDWR, 0)) != -1) {
			dup2(fd, STDIN_FILENO);
			dup2(fd, STDOUT_FILENO);
			dup2(fd, STDERR_FILENO);
			if (fd > 2) close(fd);
		}
	}
	log_debug("main", "lldpd " PACKAGE_VERSION " starting...");
	version_check();

	/* Grab uid and gid to use for priv sep */
#ifdef ENABLE_PRIVSEP
	if ((user = getpwnam(PRIVSEP_USER)) == NULL)
		fatalx("main", "no " PRIVSEP_USER " user for privilege separation, please create it");
	uid = user->pw_uid;
	if ((group = getgrnam(PRIVSEP_GROUP)) == NULL)
		fatalx("main", "no " PRIVSEP_GROUP " group for privilege separation, please create it");
	gid = group->gr_gid;
#endif

	/* Create and setup socket */
	int retry = 1;
	log_debug("main", "creating control socket");
	while ((ctl = ctl_create(ctlname)) == -1) {
		if (retry-- && errno == EADDRINUSE) {
			/* Check if a daemon is really listening */
			int tfd;
			log_info("main", "unable to create control socket because it already exists");
			log_info("main", "check if another instance is running");
			if ((tfd = ctl_connect(ctlname)) != -1) {
				/* Another instance is running */
				close(tfd);
				log_warnx("main", "another instance is running, please stop it");
				fatalx("main", "giving up");
			} else if (errno == ECONNREFUSED) {
				/* Nobody is listening */
				log_info("main", "old control socket is present, clean it");
				ctl_cleanup(ctlname);
				continue;
			}
			log_warn("main", "cannot determine if another daemon is already running");
			fatalx("main", "giving up");
		}
		log_warn("main", "unable to create control socket at %s", ctlname);
		fatalx("main", "giving up");
	}
#ifdef ENABLE_PRIVSEP
	if (chown(ctlname, uid, gid) == -1)
		log_warn("main", "unable to chown control socket");
	if (chmod(ctlname,
		S_IRUSR | S_IWUSR | S_IXUSR |
		S_IRGRP | S_IWGRP | S_IXGRP) == -1)
		log_warn("main", "unable to chmod control socket");
#endif

	/* Disable SIGPIPE */
	signal(SIGPIPE, SIG_IGN);

	/* Disable SIGHUP, until handlers are installed */
	signal(SIGHUP, SIG_IGN);

	/* Daemonization, unless started by upstart, systemd or launchd or debug */
#ifndef HOST_OS_OSX
	if (daemonize &&
	    !lldpd_started_by_upstart() && !lldpd_started_by_systemd()) {
		int pid;
		char *spid;
		log_debug("main", "going into background");
		if (daemon(0, 1) != 0)
			fatal("main", "failed to detach daemon");
		if ((pid = open(pidfile,
			    O_TRUNC | O_CREAT | O_WRONLY, 0666)) == -1)
			fatal("main", "unable to open pid file " LLDPD_PID_FILE
			    " (or the specified one)");
		if (asprintf(&spid, "%d\n", getpid()) == -1)
			fatal("main", "unable to create pid file " LLDPD_PID_FILE
			    " (or the specified one)");
		if (write(pid, spid, strlen(spid)) == -1)
			fatal("main", "unable to write pid file " LLDPD_PID_FILE
			    " (or the specified one)");
		free(spid);
		close(pid);
	}
#endif

	/* Configuration with lldpcli */
	if (lldpcli) {
		log_debug("main", "invoking lldpcli for configuration");
		if (lldpd_configure(use_syslog, debug, lldpcli, ctlname) == -1)
			fatal("main", "unable to spawn lldpcli");
	}

	/* Try to read system information from /etc/os-release if possible.
	   Fall back to lsb_release for compatibility. */
	log_debug("main", "get OS/LSB release information");
	lsb_release = lldpd_get_os_release();
	if (!lsb_release) {
		lsb_release = lldpd_get_lsb_release();
	}

	log_debug("main", "initialize privilege separation");
#ifdef ENABLE_PRIVSEP
	priv_init(PRIVSEP_CHROOT, ctl, uid, gid);
#else
	priv_init(PRIVSEP_CHROOT, ctl, 0, 0);
#endif

	/* Initialization of global configuration */
	if ((cfg = (struct lldpd *)
	    calloc(1, sizeof(struct lldpd))) == NULL)
		fatal("main", NULL);

	lldpd_alloc_default_local_port(cfg);
	cfg->g_ctlname = ctlname;
	cfg->g_ctl = ctl;
	cfg->g_config.c_mgmt_pattern = mgmtp;
	cfg->g_config.c_cid_pattern = cidp;
	cfg->g_config.c_iface_pattern = interfaces;
	cfg->g_config.c_smart = smart;
	if (lldpcli)
		cfg->g_config.c_paused = 1;
	cfg->g_config.c_receiveonly = receiveonly;
	cfg->g_config.c_tx_interval = LLDPD_TX_INTERVAL;
	cfg->g_config.c_tx_hold = LLDPD_TX_HOLD;
	cfg->g_config.c_ttl = cfg->g_config.c_tx_interval * cfg->g_config.c_tx_hold;
	cfg->g_config.c_max_neighbors = LLDPD_MAX_NEIGHBORS;
#ifdef ENABLE_LLDPMED
	cfg->g_config.c_enable_fast_start = enable_fast_start;
	cfg->g_config.c_tx_fast_init = LLDPD_FAST_INIT;
	cfg->g_config.c_tx_fast_interval = LLDPD_FAST_TX_INTERVAL;
#endif
#ifdef USE_SNMP
	cfg->g_snmp = snmp;
	cfg->g_snmp_agentx = agentx;
#endif /* USE_SNMP */
	cfg->g_config.c_bond_slave_src_mac_type = \
	    LLDP_BOND_SLAVE_SRC_MAC_TYPE_LOCALLY_ADMINISTERED;

	/* Get ioctl socket */
	log_debug("main", "get an ioctl socket");
	if ((cfg->g_sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		fatal("main", "failed to get ioctl socket");

	/* Description */
	if (!(cfg->g_config.c_advertise_version = advertise_version) &&
	    lsb_release && lsb_release[strlen(lsb_release) - 1] == '\n')
		lsb_release[strlen(lsb_release) - 1] = '\0';
	cfg->g_lsb_release = lsb_release;
        if (descr_override)
           cfg->g_config.c_description = descr_override;

	if (platform_override)
		cfg->g_config.c_platform = platform_override;

	/* Set system capabilities */
	log_debug("main", "set system capabilities");
	if ((lchassis = (struct lldpd_chassis*)
		calloc(1, sizeof(struct lldpd_chassis))) == NULL)
		fatal("localchassis", NULL);
	cfg->g_config.c_cap_advertise = 1;
	lchassis->c_cap_available = LLDP_CAP_BRIDGE | LLDP_CAP_WLAN |
	    LLDP_CAP_ROUTER | LLDP_CAP_STATION;
	cfg->g_config.c_mgmt_advertise = 1;
	TAILQ_INIT(&lchassis->c_mgmt);
#ifdef ENABLE_LLDPMED
	if (lldpmed > 0) {
		if (lldpmed == LLDP_MED_CLASS_III)
			lchassis->c_cap_available |= LLDP_CAP_TELEPHONE;
		lchassis->c_med_type = lldpmed;
		lchassis->c_med_cap_available = LLDP_MED_CAP_CAP |
		    LLDP_MED_CAP_IV | LLDP_MED_CAP_LOCATION |
		    LLDP_MED_CAP_POLICY | LLDP_MED_CAP_MDI_PSE | LLDP_MED_CAP_MDI_PD;
		cfg->g_config.c_noinventory = noinventory;
	} else
		cfg->g_config.c_noinventory = 1;
#endif

	log_debug("main", "initialize protocols");
	cfg->g_protocols = protos;
	for (i=0; protos[i].mode != 0; i++) {

		/* With -ll, disable LLDP */
		if (protos[i].mode == LLDPD_MODE_LLDP)
			protos[i].enabled %= 3;
		/* With -ccc force CDPV2, enable CDPV1 */
		if (protos[i].mode == LLDPD_MODE_CDPV1 && protos[i].enabled == 3) {
			protos[i].enabled = 1;
		}
		/* With -cc force CDPV1, enable CDPV2 */
		if (protos[i].mode == LLDPD_MODE_CDPV2 && protos[i].enabled == 2) {
			protos[i].enabled = 1;
		}

		/* With -cccc disable CDPV1, enable CDPV2 */
		if (protos[i].mode == LLDPD_MODE_CDPV1 && protos[i].enabled >= 4) {
			protos[i].enabled = 0;
		}

		/* With -cccc disable CDPV1, enable CDPV2; -ccccc will force CDPv2 */
		if (protos[i].mode == LLDPD_MODE_CDPV2 && protos[i].enabled == 4) {
			protos[i].enabled = 1;
		}

		if (protos[i].enabled > 1)
			log_info("main", "protocol %s enabled and forced", protos[i].name);
		else if (protos[i].enabled)
			log_info("main", "protocol %s enabled", protos[i].name);
		else
			log_info("main", "protocol %s disabled", protos[i].name);
	    }

	TAILQ_INIT(&cfg->g_hardware);
	TAILQ_INIT(&cfg->g_chassis);
	TAILQ_INSERT_TAIL(&cfg->g_chassis, lchassis, c_entries);
	lchassis->c_refcount++; /* We should always keep a reference to local chassis */

	/* Main loop */
	log_debug("main", "start main loop");
	levent_loop(cfg);
	lchassis->c_refcount--;
	lldpd_exit(cfg);
	free(cfg);

	return (0);
}
