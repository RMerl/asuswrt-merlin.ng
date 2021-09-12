/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2015 Vincent Bernat <vincent@bernat.im>
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

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <arpa/inet.h>

#include "../lldpctl.h"
#include "../../log.h"
#include "../atom.h"
#include "../helpers.h"

#ifdef ENABLE_DOT3

static lldpctl_map_t port_dot3_power_devicetype_map[] = {
	{ LLDP_DOT3_POWER_PSE, "PSE" },
	{ LLDP_DOT3_POWER_PD,  "PD" },
	{ 0, NULL }
};

static lldpctl_map_t port_dot3_power_pse_source_map[] = {
	{ LLDP_DOT3_POWER_SOURCE_BOTH, "PSE + Local" },
	{ LLDP_DOT3_POWER_SOURCE_PSE, "PSE" },
	{ 0, NULL }
};

static lldpctl_map_t port_dot3_power_pd_source_map[] = {
	{ LLDP_DOT3_POWER_SOURCE_BACKUP, "Backup source" },
	{ LLDP_DOT3_POWER_SOURCE_PRIMARY, "Primary power source" },
	{ 0, NULL }
};

static struct atom_map port_dot3_power_pairs_map = {
	.key = lldpctl_k_dot3_power_pairs,
	.map = {
		{ 0,                           "unknown" },
		{ LLDP_DOT3_POWERPAIRS_SIGNAL, "signal" },
		{ LLDP_DOT3_POWERPAIRS_SPARE,  "spare" },
		{ 0, NULL }
	},
};

static struct atom_map port_dot3_power_class_map = {
	.key = lldpctl_k_dot3_power_class,
	.map = {
		{ 1, "class 0" },
		{ 2, "class 1" },
		{ 3, "class 2" },
		{ 4, "class 3" },
		{ 5, "class 4" },
		{ 0, NULL }
	},
};

static struct atom_map port_dot3_power_priority_map = {
	.key = lldpctl_k_dot3_power_priority,
	.map = {
		{ 0,                          "unknown" },
		{ LLDP_MED_POW_PRIO_CRITICAL, "critical" },
		{ LLDP_MED_POW_PRIO_HIGH,     "high" },
		{ LLDP_MED_POW_PRIO_LOW,      "low" },
		{ 0, NULL },
	},
};

static struct atom_map port_dot3_power_pd_4pid_map = {
	.key = lldpctl_k_dot3_power_pd_4pid,
	.map = {
		{ 0, "PD does not support powering both modes" },
		{ 1, "PD supports powering both modes" },
	},
};

static struct atom_map port_dot3_power_pse_status_map = {
	.key = lldpctl_k_dot3_power_pse_status,
	.map = {
		{ 0, "Unknown" },
		{ 1, "2-pair powering" },
		{ 2, "4-pair powering dual-signature PD" },
		{ 3, "4-pair powering single-signature PD" },
	},
};

static struct atom_map port_dot3_power_pd_status_map = {
	.key = lldpctl_k_dot3_power_pd_status,
	.map = {
		{ 0, "Unknown" },
		{ 1, "2-pair powered PD" },
		{ 2, "4-pair powered dual-signature PD" },
		{ 3, "4-pair powered single-signature PD" },
	},
};

static struct atom_map port_dot3_power_pse_pairs_ext_map = {
	.key = lldpctl_k_dot3_power_pse_pairs_ext,
	.map = {
		{ 0, "Unknown" },
		{ 1, "Alternative A" },
		{ 2, "Alternative B" },
		{ 3, "Both alternatives" },
	},
};

static struct atom_map port_dot3_power_class_a_map = {
	.key = lldpctl_k_dot3_power_class_a,
	.map = {
		{ 0, "Unknown" },
		{ 1, "Class 1" },
		{ 2, "Class 2" },
		{ 3, "Class 3" },
		{ 4, "Class 4" },
		{ 5, "Class 5" },
		{ 6, "Unknown" },
		{ 7, "Single-signature PD or 2-pair only PSE" },
	},
};

static struct atom_map port_dot3_power_class_b_map = {
	.key = lldpctl_k_dot3_power_class_b,
	.map = {
		{ 0, "Unknown" },
		{ 1, "Class 1" },
		{ 2, "Class 2" },
		{ 3, "Class 3" },
		{ 4, "Class 4" },
		{ 5, "Class 5" },
		{ 6, "Unknown" },
		{ 7, "Single-signature PD or 2-pair only PSE" },
	},
};

static struct atom_map port_dot3_power_class_ext_map = {
	.key = lldpctl_k_dot3_power_class_ext,
	.map = {
		{ 0, "Unknown" },
		{ 1, "Class 1" },
		{ 2, "Class 2" },
		{ 3, "Class 3" },
		{ 4, "Class 4" },
		{ 5, "Class 5" },
		{ 6, "Class 6" },
		{ 7, "Class 7" },
		{ 8, "Class 8" },
		{ 9, "Unknown" },
		{ 10, "Unknown" },
		{ 11, "Unknown" },
		{ 12, "Unknown" },
		{ 13, "Unknown" },
		{ 14, "Unknown" },
		{ 15, "Dual-signature PD" },
	},
};

static struct atom_map port_dot3_power_type_ext_map = {
	.key = lldpctl_k_dot3_power_type_ext,
	.map = {
		{ LLDP_DOT3_POWER_8023BT_OFF, "802.3bt off" },
		{ 1, "Type 3 PSE" },
		{ 2, "Type 4 PSE" },
		{ 3, "Type 3 single-signature PD" },
		{ 4, "Type 3 dual-signature PD" },
		{ 5, "Type 4 single-signature PD" },
		{ 6, "Type 4 dual-signature PD" },
		{ 7, "Unknown" },
		{ 8, "Unknown" },
	},
};

static struct atom_map port_dot3_power_pd_load_map = {
	.key = lldpctl_k_dot3_power_pd_load,
	.map = {
		{ 0, "PD is single- or dual-signature and power is not "
		  "electrically isolated" },
		{ 1, "PD is dual-signature and power is electrically "
		  "isolated" },
	},
};


ATOM_MAP_REGISTER(port_dot3_power_pairs_map,    4);
ATOM_MAP_REGISTER(port_dot3_power_class_map,    5);
ATOM_MAP_REGISTER(port_dot3_power_priority_map, 6);

static int
_lldpctl_atom_new_dot3_power(lldpctl_atom_t *atom, va_list ap)
{
	struct _lldpctl_atom_dot3_power_t *dpow =
	    (struct _lldpctl_atom_dot3_power_t *)atom;
	dpow->parent = va_arg(ap, struct _lldpctl_atom_port_t *);
	lldpctl_atom_inc_ref((lldpctl_atom_t *)dpow->parent);
	return 1;
}

static void
_lldpctl_atom_free_dot3_power(lldpctl_atom_t *atom)
{
	struct _lldpctl_atom_dot3_power_t *dpow =
	    (struct _lldpctl_atom_dot3_power_t *)atom;
	lldpctl_atom_dec_ref((lldpctl_atom_t *)dpow->parent);
}

static const char*
_lldpctl_atom_get_str_dot3_power(lldpctl_atom_t *atom, lldpctl_key_t key)
{
	struct _lldpctl_atom_dot3_power_t *dpow =
	    (struct _lldpctl_atom_dot3_power_t *)atom;
	struct lldpd_port     *port     = dpow->parent->port;

	/* Local and remote port */
	switch (key) {
	case lldpctl_k_dot3_power_devicetype:
		return map_lookup(port_dot3_power_devicetype_map,
		    port->p_power.devicetype);
	case lldpctl_k_dot3_power_pairs:
		return map_lookup(port_dot3_power_pairs_map.map,
		    port->p_power.pairs);
	case lldpctl_k_dot3_power_class:
		return map_lookup(port_dot3_power_class_map.map,
		    port->p_power.class);
	case lldpctl_k_dot3_power_source:
		return map_lookup((port->p_power.devicetype == LLDP_DOT3_POWER_PSE)?
		    port_dot3_power_pse_source_map:
		    port_dot3_power_pd_source_map,
		    port->p_power.source);
	case lldpctl_k_dot3_power_priority:
		return map_lookup(port_dot3_power_priority_map.map,
		    port->p_power.priority);
	case lldpctl_k_dot3_power_pd_4pid:
		return map_lookup(port_dot3_power_pd_4pid_map.map,
		    port->p_power.pd_4pid);
	case lldpctl_k_dot3_power_pse_status:
		return map_lookup(port_dot3_power_pse_status_map.map,
		    port->p_power.pse_status);
	case lldpctl_k_dot3_power_pd_status:
		return map_lookup(port_dot3_power_pd_status_map.map,
		    port->p_power.pd_status);
	case lldpctl_k_dot3_power_pse_pairs_ext:
		return map_lookup(port_dot3_power_pse_pairs_ext_map.map,
		    port->p_power.pse_pairs_ext);
	case lldpctl_k_dot3_power_class_a:
		return map_lookup(port_dot3_power_class_a_map.map,
		    port->p_power.class_a);
	case lldpctl_k_dot3_power_class_b:
		return map_lookup(port_dot3_power_class_b_map.map,
		    port->p_power.class_b);
	case lldpctl_k_dot3_power_class_ext:
		return map_lookup(port_dot3_power_class_ext_map.map,
		    port->p_power.class_ext);
	case lldpctl_k_dot3_power_type_ext:
		return map_lookup(port_dot3_power_type_ext_map.map,
		    port->p_power.type_ext);
	case lldpctl_k_dot3_power_pd_load:
		return map_lookup(port_dot3_power_pd_load_map.map,
		    port->p_power.pd_load);
	default:
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}
}

static long int
_lldpctl_atom_get_int_dot3_power(lldpctl_atom_t *atom, lldpctl_key_t key)
{
	struct _lldpctl_atom_dot3_power_t *dpow =
	    (struct _lldpctl_atom_dot3_power_t *)atom;
	struct lldpd_port     *port     = dpow->parent->port;

	/* Local and remote port */
	switch (key) {
	case lldpctl_k_dot3_power_devicetype:
		return port->p_power.devicetype;
	case lldpctl_k_dot3_power_supported:
		return port->p_power.supported;
	case lldpctl_k_dot3_power_enabled:
		return port->p_power.enabled;
	case lldpctl_k_dot3_power_paircontrol:
		return port->p_power.paircontrol;
	case lldpctl_k_dot3_power_pairs:
		return port->p_power.pairs;
	case lldpctl_k_dot3_power_class:
		return port->p_power.class;
	case lldpctl_k_dot3_power_type:
		return port->p_power.powertype;
	case lldpctl_k_dot3_power_source:
		return port->p_power.source;
	case lldpctl_k_dot3_power_priority:
		return port->p_power.priority;
	case lldpctl_k_dot3_power_requested:
		return port->p_power.requested * 100;
	case lldpctl_k_dot3_power_allocated:
		return port->p_power.allocated * 100;
	/* 802.3bt additions */
	case lldpctl_k_dot3_power_pd_4pid:
		return port->p_power.pd_4pid;
	case lldpctl_k_dot3_power_requested_a:
		return port->p_power.requested_a * 100;
	case lldpctl_k_dot3_power_requested_b:
		return port->p_power.requested_b * 100;
	case lldpctl_k_dot3_power_allocated_a:
		return port->p_power.allocated_a * 100;
	case lldpctl_k_dot3_power_allocated_b:
		return port->p_power.allocated_b * 100;
	case lldpctl_k_dot3_power_pse_status:
		return port->p_power.pse_status;
	case lldpctl_k_dot3_power_pd_status:
		return port->p_power.pd_status;
	case lldpctl_k_dot3_power_pse_pairs_ext:
		return port->p_power.pse_pairs_ext;
	case lldpctl_k_dot3_power_class_a:
		return port->p_power.class_a;
	case lldpctl_k_dot3_power_class_b:
		return port->p_power.class_b;
	case lldpctl_k_dot3_power_class_ext:
		return port->p_power.class_ext;
	case lldpctl_k_dot3_power_type_ext:
		return port->p_power.type_ext;
	case lldpctl_k_dot3_power_pd_load:
		return port->p_power.pd_load;
	case lldpctl_k_dot3_power_pse_max:
		return port->p_power.pse_max * 100;
	default:
		return SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
	}
}

static lldpctl_atom_t*
_lldpctl_atom_set_int_dot3_power(lldpctl_atom_t *atom, lldpctl_key_t key,
    long int value)
{
	struct _lldpctl_atom_dot3_power_t *dpow =
	    (struct _lldpctl_atom_dot3_power_t *)atom;
	struct lldpd_port *port = dpow->parent->port;

	/* Only local port can be modified */
	if (!dpow->parent->local) {
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}

	switch (key) {
	case lldpctl_k_dot3_power_devicetype:
		switch (value) {
		case 0:		/* Disabling */
		case LLDP_DOT3_POWER_PSE:
		case LLDP_DOT3_POWER_PD:
			port->p_power.devicetype = value;
			return atom;
		default: goto bad;
		}
	case lldpctl_k_dot3_power_supported:
		switch (value) {
		case 0:
		case 1:
			port->p_power.supported = value;
			return atom;
		default: goto bad;
		}
	case lldpctl_k_dot3_power_enabled:
		switch (value) {
		case 0:
		case 1:
			port->p_power.enabled = value;
			return atom;
		default: goto bad;
		}
	case lldpctl_k_dot3_power_paircontrol:
		switch (value) {
		case 0:
		case 1:
			port->p_power.paircontrol = value;
			return atom;
		default: goto bad;
		}
	case lldpctl_k_dot3_power_pairs:
		switch (value) {
		case 0:
		case 1:
		case 2:
			port->p_power.pairs = value;
			return atom;
		default: goto bad;
		}
	case lldpctl_k_dot3_power_class:
		if (value < 0 || value > 5)
			goto bad;
		port->p_power.class = value;
		return atom;
	case lldpctl_k_dot3_power_type:
		switch (value) {
		case LLDP_DOT3_POWER_8023AT_TYPE1:
		case LLDP_DOT3_POWER_8023AT_TYPE2:
		case LLDP_DOT3_POWER_8023AT_OFF:
			port->p_power.powertype = value;
			return atom;
		default: goto bad;
		}
	case lldpctl_k_dot3_power_source:
		if (value < 0 || value > 3)
			goto bad;
		port->p_power.source = value;
		return atom;
	case lldpctl_k_dot3_power_priority:
		switch (value) {
		case LLDP_DOT3_POWER_PRIO_UNKNOWN:
		case LLDP_DOT3_POWER_PRIO_CRITICAL:
		case LLDP_DOT3_POWER_PRIO_HIGH:
		case LLDP_DOT3_POWER_PRIO_LOW:
			port->p_power.priority = value;
			return atom;
		default: goto bad;
		}
	case lldpctl_k_dot3_power_allocated:
		if (value < 0) goto bad;
		port->p_power.allocated = value / 100;
		return atom;
	case lldpctl_k_dot3_power_requested:
		if (value < 0) goto bad;
		port->p_power.requested = value / 100;
		return atom;
	/* 802.3bt additions */
	case lldpctl_k_dot3_power_pd_4pid:
		port->p_power.pd_4pid = value;
		return atom;
	case lldpctl_k_dot3_power_requested_a:
		port->p_power.requested_a = value / 100;
		return atom;
	case lldpctl_k_dot3_power_requested_b:
		port->p_power.requested_b = value / 100;
		return atom;
	case lldpctl_k_dot3_power_allocated_a:
		port->p_power.allocated_a = value / 100;
		return atom;
	case lldpctl_k_dot3_power_allocated_b:
		port->p_power.allocated_b = value / 100;
		return atom;
	case lldpctl_k_dot3_power_pse_status:
		port->p_power.pse_status = value;
		return atom;
	case lldpctl_k_dot3_power_pd_status:
		port->p_power.pd_status = value;
		return atom;
	case lldpctl_k_dot3_power_pse_pairs_ext:
		port->p_power.pse_pairs_ext = value;
		return atom;
	case lldpctl_k_dot3_power_class_a:
		port->p_power.class_a = value;
		return atom;
	case lldpctl_k_dot3_power_class_b:
		port->p_power.class_b = value;
		return atom;
	case lldpctl_k_dot3_power_class_ext:
		port->p_power.class_ext = value;
		return atom;
	case lldpctl_k_dot3_power_type_ext:
		port->p_power.type_ext = value;
		return atom;
	case lldpctl_k_dot3_power_pd_load:
		port->p_power.pd_load = value;
		return atom;
	case lldpctl_k_dot3_power_pse_max:
		port->p_power.pse_max = value / 100;
		return atom;
	default:
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}

	return atom;
bad:
	SET_ERROR(atom->conn, LLDPCTL_ERR_BAD_VALUE);
	return NULL;
}

static lldpctl_atom_t*
_lldpctl_atom_set_str_dot3_power(lldpctl_atom_t *atom, lldpctl_key_t key,
    const char *value)
{
	switch (key) {
	case lldpctl_k_dot3_power_devicetype:
		return _lldpctl_atom_set_int_dot3_power(atom, key,
		    map_reverse_lookup(port_dot3_power_devicetype_map, value));
	case lldpctl_k_dot3_power_pairs:
		return _lldpctl_atom_set_int_dot3_power(atom, key,
		    map_reverse_lookup(port_dot3_power_pairs_map.map, value));
	case lldpctl_k_dot3_power_class:
		return _lldpctl_atom_set_int_dot3_power(atom, key,
		    map_reverse_lookup(port_dot3_power_class_map.map, value));
	case lldpctl_k_dot3_power_priority:
		return _lldpctl_atom_set_int_dot3_power(atom, key,
		    map_reverse_lookup(port_dot3_power_priority_map.map, value));
	default:
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}
}

static struct atom_builder dot3_power =
	{ atom_dot3_power, sizeof(struct _lldpctl_atom_dot3_power_t),
	  .init = _lldpctl_atom_new_dot3_power,
	  .free = _lldpctl_atom_free_dot3_power,
	  .get_int = _lldpctl_atom_get_int_dot3_power,
	  .set_int = _lldpctl_atom_set_int_dot3_power,
	  .get_str = _lldpctl_atom_get_str_dot3_power,
	  .set_str = _lldpctl_atom_set_str_dot3_power };

ATOM_BUILDER_REGISTER(dot3_power, 8);

#endif

