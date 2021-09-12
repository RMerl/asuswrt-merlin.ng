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

static lldpctl_map_t chassis_id_subtype_map[] = {
	{ LLDP_CHASSISID_SUBTYPE_IFNAME,  "ifname"},
	{ LLDP_CHASSISID_SUBTYPE_IFALIAS, "ifalias" },
	{ LLDP_CHASSISID_SUBTYPE_LOCAL,   "local" },
	{ LLDP_CHASSISID_SUBTYPE_LLADDR,  "mac" },
	{ LLDP_CHASSISID_SUBTYPE_ADDR,    "ip" },
	{ LLDP_CHASSISID_SUBTYPE_PORT,    "unhandled" },
	{ LLDP_CHASSISID_SUBTYPE_CHASSIS, "unhandled" },
	{ 0, NULL},
};

#ifdef ENABLE_LLDPMED

static lldpctl_map_t chassis_med_type_map[] = {
	{ LLDP_MED_CLASS_I,        "Generic Endpoint (Class I)" },
	{ LLDP_MED_CLASS_II,       "Media Endpoint (Class II)" },
	{ LLDP_MED_CLASS_III,      "Communication Device Endpoint (Class III)" },
	{ LLDP_MED_NETWORK_DEVICE, "Network Connectivity Device" },
	{ 0, NULL },
};

#endif

static int
_lldpctl_atom_new_chassis(lldpctl_atom_t *atom, va_list ap)
{
	struct _lldpctl_atom_chassis_t *p =
	    (struct _lldpctl_atom_chassis_t *)atom;
	p->chassis = va_arg(ap, struct lldpd_chassis*);
	p->parent = va_arg(ap, struct _lldpctl_atom_port_t*);
	p->embedded = va_arg(ap, int);
	if (p->parent && !p->embedded)
		lldpctl_atom_inc_ref((lldpctl_atom_t*)p->parent);
	return 1;
}

static void
_lldpctl_atom_free_chassis(lldpctl_atom_t *atom)
{
	struct _lldpctl_atom_chassis_t *p =
	    (struct _lldpctl_atom_chassis_t *)atom;
	/* When we have a parent, the chassis structure is in fact part of the
	 * parent, just decrement the reference count of the parent. Otherwise,
	 * we need to free the whole chassis. When embedded, we don't alter the
	 * reference count of the parent. Therefore, it's important to also not
	 * increase the reference count of this atom. See
	 * `_lldpctl_atom_get_atom_chassis' for how to handle that. */
	if (p->parent) {
		if (!p->embedded)
			lldpctl_atom_dec_ref((lldpctl_atom_t*)p->parent);
	} else
		lldpd_chassis_cleanup(p->chassis, 1);
}

static lldpctl_atom_t*
_lldpctl_atom_get_atom_chassis(lldpctl_atom_t *atom, lldpctl_key_t key)
{
	struct _lldpctl_atom_chassis_t *p =
	    (struct _lldpctl_atom_chassis_t *)atom;
	struct lldpd_chassis *chassis = p->chassis;

	switch (key) {
	case lldpctl_k_chassis_mgmt:
		return _lldpctl_new_atom(atom->conn, atom_mgmts_list,
		    (p->parent && p->embedded)?
		    (lldpctl_atom_t *)p->parent:
		    (lldpctl_atom_t *)p,
		    chassis);
	default:
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}
}

static const char*
_lldpctl_atom_get_str_chassis(lldpctl_atom_t *atom, lldpctl_key_t key)
{
	struct _lldpctl_atom_chassis_t *p =
	    (struct _lldpctl_atom_chassis_t *)atom;
	struct lldpd_chassis *chassis = p->chassis;
	char *ipaddress = NULL; size_t len;

	/* Local and remote port */
	switch (key) {

	case lldpctl_k_chassis_id_subtype:
		return map_lookup(chassis_id_subtype_map, chassis->c_id_subtype);
	case lldpctl_k_chassis_id:
		switch (chassis->c_id_subtype) {
		case LLDP_CHASSISID_SUBTYPE_IFNAME:
		case LLDP_CHASSISID_SUBTYPE_IFALIAS:
		case LLDP_CHASSISID_SUBTYPE_LOCAL:
			return chassis->c_id;
		case LLDP_CHASSISID_SUBTYPE_LLADDR:
			return _lldpctl_dump_in_atom(atom,
			    (uint8_t*)chassis->c_id, chassis->c_id_len,
			    ':', 0);
		case LLDP_CHASSISID_SUBTYPE_ADDR:
			switch (chassis->c_id[0]) {
			case LLDP_MGMT_ADDR_IP4: len = INET_ADDRSTRLEN + 1; break;
			case LLDP_MGMT_ADDR_IP6: len = INET6_ADDRSTRLEN + 1; break;
			default: len = 0;
			}
			if (len > 0) {
				ipaddress = _lldpctl_alloc_in_atom(atom, len);
				if (!ipaddress) return NULL;
				if (inet_ntop((chassis->c_id[0] == LLDP_MGMT_ADDR_IP4)?
					AF_INET:AF_INET6,
					&chassis->c_id[1], ipaddress, len) == NULL)
					break;
				return ipaddress;
			}
			break;
		}
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	case lldpctl_k_chassis_name: return chassis->c_name;
	case lldpctl_k_chassis_descr: return chassis->c_descr;

#ifdef ENABLE_LLDPMED
	case lldpctl_k_chassis_med_type:
		return map_lookup(chassis_med_type_map, chassis->c_med_type);
	case lldpctl_k_chassis_med_inventory_hw:
		return chassis->c_med_hw;
	case lldpctl_k_chassis_med_inventory_sw:
		return chassis->c_med_sw;
	case lldpctl_k_chassis_med_inventory_fw:
		return chassis->c_med_fw;
	case lldpctl_k_chassis_med_inventory_sn:
		return chassis->c_med_sn;
	case lldpctl_k_chassis_med_inventory_manuf:
		return chassis->c_med_manuf;
	case lldpctl_k_chassis_med_inventory_model:
		return chassis->c_med_model;
	case lldpctl_k_chassis_med_inventory_asset:
		return chassis->c_med_asset;
#endif

	default:
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}
}

static long int
_lldpctl_atom_get_int_chassis(lldpctl_atom_t *atom, lldpctl_key_t key)
{
	struct _lldpctl_atom_chassis_t *p =
	    (struct _lldpctl_atom_chassis_t *)atom;
	struct lldpd_chassis *chassis = p->chassis;

	/* Local and remote port */
	switch (key) {
	case lldpctl_k_chassis_index:
		return chassis->c_index;
	case lldpctl_k_chassis_id_subtype:
		return chassis->c_id_subtype;
	case lldpctl_k_chassis_cap_available:
		return chassis->c_cap_available;
	case lldpctl_k_chassis_cap_enabled:
		return chassis->c_cap_enabled;
#ifdef ENABLE_LLDPMED
	case lldpctl_k_chassis_med_type:
		return chassis->c_med_type;
	case lldpctl_k_chassis_med_cap:
		return chassis->c_med_cap_available;
#endif
	default:
		return SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
	}
	return SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
}

static const uint8_t*
_lldpctl_atom_get_buf_chassis(lldpctl_atom_t *atom, lldpctl_key_t key, size_t *n)
{
	struct _lldpctl_atom_chassis_t *p =
	    (struct _lldpctl_atom_chassis_t *)atom;
	struct lldpd_chassis *chassis = p->chassis;

	switch (key) {
	case lldpctl_k_chassis_id:
		*n = chassis->c_id_len;
		return (uint8_t*)chassis->c_id;
	default:
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}
}

static struct atom_builder chassis =
	{ atom_chassis, sizeof(struct _lldpctl_atom_chassis_t),
	  .init = _lldpctl_atom_new_chassis,
	  .free = _lldpctl_atom_free_chassis,
	  .get  = _lldpctl_atom_get_atom_chassis,
	  .get_str = _lldpctl_atom_get_str_chassis,
	  .get_int = _lldpctl_atom_get_int_chassis,
	  .get_buffer = _lldpctl_atom_get_buf_chassis };

ATOM_BUILDER_REGISTER(chassis, 3);
