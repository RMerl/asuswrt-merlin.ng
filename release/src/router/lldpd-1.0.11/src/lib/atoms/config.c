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

static struct atom_map bond_slave_src_mac_map = {
	.key = lldpctl_k_config_bond_slave_src_mac_type,
	.map = {
		{ LLDP_BOND_SLAVE_SRC_MAC_TYPE_REAL,   "real"},
		{ LLDP_BOND_SLAVE_SRC_MAC_TYPE_ZERO,   "zero"},
		{ LLDP_BOND_SLAVE_SRC_MAC_TYPE_FIXED,  "fixed"},
		{ LLDP_BOND_SLAVE_SRC_MAC_TYPE_LOCALLY_ADMINISTERED, "local" },
		{ LLDP_BOND_SLAVE_SRC_MAC_TYPE_UNKNOWN, NULL},
	},
};

static struct atom_map lldp_portid_map = {
	.key = lldpctl_k_config_lldp_portid_type,
	.map = {
		{ LLDP_PORTID_SUBTYPE_IFNAME,   "ifname"},
		{ LLDP_PORTID_SUBTYPE_LLADDR,   "macaddress"},
		{ LLDP_PORTID_SUBTYPE_LOCAL,    "local"},
		{ LLDP_PORTID_SUBTYPE_UNKNOWN,  NULL},
	},
};

static struct atom_map lldp_agent_map = {
	.key = lldpctl_k_config_lldp_agent_type,
	.map = {
		{ LLDP_AGENT_TYPE_NEAREST_BRIDGE,          "nearest bridge"},
		{ LLDP_AGENT_TYPE_NEAREST_NONTPMR_BRIDGE,  "nearest non-TPMR bridge"},
		{ LLDP_AGENT_TYPE_NEAREST_CUSTOMER_BRIDGE, "nearest customer bridge"},
		{ LLDP_AGENT_TYPE_UNKNOWN, NULL},
	},
};

ATOM_MAP_REGISTER(bond_slave_src_mac_map, 1);
ATOM_MAP_REGISTER(lldp_portid_map,        2);
ATOM_MAP_REGISTER(lldp_agent_map,         3);

static int
_lldpctl_atom_new_config(lldpctl_atom_t *atom, va_list ap)
{
	struct _lldpctl_atom_config_t *c =
	    (struct _lldpctl_atom_config_t *)atom;
	c->config = va_arg(ap, struct lldpd_config *);
	return 1;
}

static void
_lldpctl_atom_free_config(lldpctl_atom_t *atom)
{
	struct _lldpctl_atom_config_t *c =
	    (struct _lldpctl_atom_config_t *)atom;
	lldpd_config_cleanup(c->config);
	free(c->config);
}

static const char*
_lldpctl_atom_get_str_config(lldpctl_atom_t *atom, lldpctl_key_t key)
{
	char *res = NULL;
	struct _lldpctl_atom_config_t *c =
	    (struct _lldpctl_atom_config_t *)atom;
	switch (key) {
	case lldpctl_k_config_mgmt_pattern:
		res = c->config->c_mgmt_pattern; break;
	case lldpctl_k_config_iface_pattern:
		res = c->config->c_iface_pattern; break;
	case lldpctl_k_config_perm_iface_pattern:
		res = c->config->c_perm_ifaces; break;
	case lldpctl_k_config_cid_pattern:
		res = c->config->c_cid_pattern; break;
	case lldpctl_k_config_cid_string:
		res = c->config->c_cid_string; break;
	case lldpctl_k_config_description:
		res = c->config->c_description; break;
	case lldpctl_k_config_platform:
		res = c->config->c_platform; break;
	case lldpctl_k_config_hostname:
		res = c->config->c_hostname; break;
	case lldpctl_k_config_bond_slave_src_mac_type:
		return map_lookup(bond_slave_src_mac_map.map,
				c->config->c_bond_slave_src_mac_type);
	case lldpctl_k_config_lldp_portid_type:
		return map_lookup(lldp_portid_map.map,
		    c->config->c_lldp_portid_type);
	case lldpctl_k_config_lldp_agent_type:
		return map_lookup(lldp_agent_map.map,
		    c->config->c_lldp_agent_type);
	default:
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}
	return res?res:"";
}

static struct _lldpctl_atom_config_t*
__lldpctl_atom_set_str_config(struct _lldpctl_atom_config_t *c,
    char **local, char **global,
    const char *value) {
	if (value) {
		char *aval = NULL;
		size_t len = strlen(value) + 1;
		aval = _lldpctl_alloc_in_atom((lldpctl_atom_t *)c, len);
		if (!aval) return NULL;
		memcpy(aval, value, len);
		*local = aval;
		free(*global); *global = strdup(aval);
	} else {
		free(*global);
		*local = *global = NULL;
	}
	return c;
}

static lldpctl_atom_t*
_lldpctl_atom_set_str_config(lldpctl_atom_t *atom, lldpctl_key_t key,
    const char *value)
{
	struct _lldpctl_atom_config_t *c =
	    (struct _lldpctl_atom_config_t *)atom;
	struct lldpd_config config;
	memcpy(&config, c->config, sizeof(struct lldpd_config));
	char *canary = NULL;
	int rc;

	switch (key) {
	case lldpctl_k_config_perm_iface_pattern:
		if (!__lldpctl_atom_set_str_config(c,
			&config.c_perm_ifaces, &c->config->c_perm_ifaces,
			value))
			return NULL;
		break;
	case lldpctl_k_config_iface_pattern:
		if (!__lldpctl_atom_set_str_config(c,
			&config.c_iface_pattern, &c->config->c_iface_pattern,
			value))
			return NULL;
		break;
	case lldpctl_k_config_mgmt_pattern:
		if (!__lldpctl_atom_set_str_config(c,
			&config.c_mgmt_pattern, &c->config->c_mgmt_pattern,
			value))
			return NULL;
		break;
	case lldpctl_k_config_cid_string:
		if (!__lldpctl_atom_set_str_config(c,
			&config.c_cid_string, &c->config->c_cid_string,
			value))
			return NULL;
		break;
	case lldpctl_k_config_description:
		if (!__lldpctl_atom_set_str_config(c,
			&config.c_description, &c->config->c_description,
			value))
			return NULL;
		break;
	case lldpctl_k_config_platform:
		if (!__lldpctl_atom_set_str_config(c,
			&config.c_platform, &c->config->c_platform,
			value))
			return NULL;
		break;
	case lldpctl_k_config_hostname:
		if (!__lldpctl_atom_set_str_config(c,
			&config.c_hostname, &c->config->c_hostname,
			value))
			return NULL;
		break;
	default:
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}

	if (asprintf(&canary, "%d%s", key, value?value:"(NULL)") == -1) {
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOMEM);
		return NULL;
	}
	rc = _lldpctl_do_something(atom->conn,
	    CONN_STATE_SET_CONFIG_SEND, CONN_STATE_SET_CONFIG_RECV,
	    canary,
	    SET_CONFIG, &config, &MARSHAL_INFO(lldpd_config),
	    NULL, NULL);
	free(canary);
	if (rc == 0) return atom;

#undef SET_STR

	return NULL;
}

static long int
_lldpctl_atom_get_int_config(lldpctl_atom_t *atom, lldpctl_key_t key)
{
	struct _lldpctl_atom_config_t *c =
	    (struct _lldpctl_atom_config_t *)atom;
	switch (key) {
	case lldpctl_k_config_paused:
		return c->config->c_paused;
	case lldpctl_k_config_tx_interval:
		return (c->config->c_tx_interval+999)/1000; /* s units */
	case lldpctl_k_config_tx_interval_ms:
		return c->config->c_tx_interval; /* ms units */
	case lldpctl_k_config_receiveonly:
		return c->config->c_receiveonly;
	case lldpctl_k_config_advertise_version:
		return c->config->c_advertise_version;
	case lldpctl_k_config_ifdescr_update:
		return c->config->c_set_ifdescr;
	case lldpctl_k_config_iface_promisc:
		return c->config->c_promisc;
	case lldpctl_k_config_chassis_cap_advertise:
		return c->config->c_cap_advertise;
	case lldpctl_k_config_chassis_mgmt_advertise:
		return c->config->c_mgmt_advertise;
#ifdef ENABLE_LLDPMED
	case lldpctl_k_config_lldpmed_noinventory:
		return c->config->c_noinventory;
	case lldpctl_k_config_fast_start_enabled:
		return c->config->c_enable_fast_start;
	case lldpctl_k_config_fast_start_interval:
		return c->config->c_tx_fast_interval;
#endif
	case lldpctl_k_config_tx_hold:
		return c->config->c_tx_hold;
	case lldpctl_k_config_max_neighbors:
		return c->config->c_max_neighbors;
	default:
		return SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
	}
}

static lldpctl_atom_t*
_lldpctl_atom_set_int_config(lldpctl_atom_t *atom, lldpctl_key_t key,
    long int value)
{
	int rc;
	char *canary = NULL;
	struct _lldpctl_atom_config_t *c =
	    (struct _lldpctl_atom_config_t *)atom;
	struct lldpd_config config;
	memcpy(&config, c->config, sizeof(struct lldpd_config));

	switch (key) {
	case lldpctl_k_config_paused:
		config.c_paused = c->config->c_paused = value;
		break;
	case lldpctl_k_config_tx_interval:
		config.c_tx_interval = value * 1000;
		if (value > 0) c->config->c_tx_interval = value * 1000;
		break;
	case lldpctl_k_config_tx_interval_ms:
		config.c_tx_interval = value;
		if (value > 0) c->config->c_tx_interval = value;
		break;
	case lldpctl_k_config_ifdescr_update:
		config.c_set_ifdescr = c->config->c_set_ifdescr = value;
		break;
	case lldpctl_k_config_iface_promisc:
		config.c_promisc = c->config->c_promisc = value;
		break;
	case lldpctl_k_config_chassis_cap_advertise:
		config.c_cap_advertise = c->config->c_cap_advertise = value;
		break;
	case lldpctl_k_config_chassis_mgmt_advertise:
		config.c_mgmt_advertise = c->config->c_mgmt_advertise = value;
		break;
#ifdef ENABLE_LLDPMED
	case lldpctl_k_config_fast_start_enabled:
		config.c_enable_fast_start = c->config->c_enable_fast_start = value;
		break;
	case lldpctl_k_config_fast_start_interval:
		config.c_tx_fast_interval = c->config->c_tx_fast_interval = value;
		break;
#endif
	case lldpctl_k_config_tx_hold:
		config.c_tx_hold = value;
		if (value > 0) c->config->c_tx_hold = value;
		break;
	case lldpctl_k_config_max_neighbors:
		config.c_max_neighbors = value;
		if (value > 0) c->config->c_max_neighbors = value;
		break;
	case lldpctl_k_config_bond_slave_src_mac_type:
		config.c_bond_slave_src_mac_type = value;
		c->config->c_bond_slave_src_mac_type = value;
		break;
	case lldpctl_k_config_lldp_portid_type:
		config.c_lldp_portid_type = value;
		c->config->c_lldp_portid_type = value;
		break;
	case lldpctl_k_config_lldp_agent_type:
		config.c_lldp_agent_type = value;
		c->config->c_lldp_agent_type = value;
		break;
	default:
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}

	if (asprintf(&canary, "%d%ld", key, value) == -1) {
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOMEM);
		return NULL;
	}
	rc = _lldpctl_do_something(atom->conn,
	    CONN_STATE_SET_CONFIG_SEND, CONN_STATE_SET_CONFIG_RECV,
	    canary,
	    SET_CONFIG, &config, &MARSHAL_INFO(lldpd_config),
	    NULL, NULL);
	free(canary);
	if (rc == 0) return atom;
	return NULL;
}

static struct atom_builder config =
	{ atom_config, sizeof(struct _lldpctl_atom_config_t),
	  .init = _lldpctl_atom_new_config,
	  .free = _lldpctl_atom_free_config,
	  .get_str = _lldpctl_atom_get_str_config,
	  .set_str = _lldpctl_atom_set_str_config,
	  .get_int = _lldpctl_atom_get_int_config,
	  .set_int = _lldpctl_atom_set_int_config };

ATOM_BUILDER_REGISTER(config, 1);
