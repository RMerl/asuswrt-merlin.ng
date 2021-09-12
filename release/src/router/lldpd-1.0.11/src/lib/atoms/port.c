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

static struct atom_map lldpd_protocol_map = {
	.key = lldpctl_k_port_protocol,
	.map = {
		{ LLDPD_MODE_LLDP,	"LLDP" },
		{ LLDPD_MODE_CDPV1,	"CDPv1"},
		{ LLDPD_MODE_CDPV2,	"CDPv2"},
		{ LLDPD_MODE_EDP,	"EDP" },
		{ LLDPD_MODE_FDP,	"FDP"},
		{ LLDPD_MODE_SONMP,	"SONMP"},
		{ 0, NULL },
	}
};

ATOM_MAP_REGISTER(lldpd_protocol_map, 3);

static lldpctl_map_t port_id_subtype_map[] = {
	{ LLDP_PORTID_SUBTYPE_IFNAME,   "ifname"},
	{ LLDP_PORTID_SUBTYPE_IFALIAS,  "ifalias" },
	{ LLDP_PORTID_SUBTYPE_LOCAL,    "local" },
	{ LLDP_PORTID_SUBTYPE_LLADDR,   "mac" },
	{ LLDP_PORTID_SUBTYPE_ADDR,     "ip" },
	{ LLDP_PORTID_SUBTYPE_PORT,     "unhandled" },
	{ LLDP_PORTID_SUBTYPE_AGENTCID, "unhandled" },
	{ 0, NULL},
};

static struct atom_map port_status_map = {
	.key = lldpctl_k_port_status,
	.map = {
		{ LLDPD_RXTX_TXONLY,   "TX only" },
		{ LLDPD_RXTX_RXONLY,   "RX only" },
		{ LLDPD_RXTX_DISABLED, "disabled" },
		{ LLDPD_RXTX_BOTH,     "RX and TX" },
		{ 0, NULL },
	}
};

ATOM_MAP_REGISTER(port_status_map, 3);

#ifdef ENABLE_DOT3
static lldpctl_map_t operational_mau_type_values[] = {
	{ LLDP_DOT3_MAU_AUI,		"AUI - No internal MAU, view from AUI" },
	{ LLDP_DOT3_MAU_10BASE5,	"10Base5 - Thick coax MAU" },
	{ LLDP_DOT3_MAU_FOIRL,		"Foirl - FOIRL MAU" },
	{ LLDP_DOT3_MAU_10BASE2,	"10Base2 - Thin coax MAU" },
	{ LLDP_DOT3_MAU_10BASET,	"10BaseT - UTP MAU" },
	{ LLDP_DOT3_MAU_10BASEFP,	"10BaseFP - Passive fiber MAU" },
	{ LLDP_DOT3_MAU_10BASEFB,	"10BaseFB - Sync fiber MAU" },
	{ LLDP_DOT3_MAU_10BASEFL,	"10BaseFL - Async fiber MAU" },
	{ LLDP_DOT3_MAU_10BROAD36,	"10Broad36 - Broadband DTE MAU" },
	{ LLDP_DOT3_MAU_10BASETHD,	"10BaseTHD - UTP MAU, half duplex mode" },
	{ LLDP_DOT3_MAU_10BASETFD,	"10BaseTFD - UTP MAU, full duplex mode" },
	{ LLDP_DOT3_MAU_10BASEFLHD,	"10BaseFLHD - Async fiber MAU, half duplex mode" },
	{ LLDP_DOT3_MAU_10BASEFLFD,	"10BaseFLDF - Async fiber MAU, full duplex mode" },
	{ LLDP_DOT3_MAU_100BASET4,	"100BaseT4 - 4 pair category 3 UTP" },
	{ LLDP_DOT3_MAU_100BASETXHD,	"100BaseTXHD - 2 pair category 5 UTP, half duplex mode" },
	{ LLDP_DOT3_MAU_100BASETXFD,	"100BaseTXFD - 2 pair category 5 UTP, full duplex mode" },
	{ LLDP_DOT3_MAU_100BASEFXHD,	"100BaseFXHD - X fiber over PMT, half duplex mode" },
	{ LLDP_DOT3_MAU_100BASEFXFD,	"100BaseFXFD - X fiber over PMT, full duplex mode" },
	{ LLDP_DOT3_MAU_100BASET2HD,	"100BaseT2HD - 2 pair category 3 UTP, half duplex mode" },
	{ LLDP_DOT3_MAU_100BASET2FD,	"100BaseT2FD - 2 pair category 3 UTP, full duplex mode" },
	{ LLDP_DOT3_MAU_1000BASEXHD,	"1000BaseXHD - PCS/PMA, unknown PMD, half duplex mode" },
	{ LLDP_DOT3_MAU_1000BASEXFD,	"1000BaseXFD - PCS/PMA, unknown PMD, full duplex mode" },
	{ LLDP_DOT3_MAU_1000BASELXHD,	"1000BaseLXHD - Fiber over long-wavelength laser, half duplex mode" },
	{ LLDP_DOT3_MAU_1000BASELXFD,	"1000BaseLXFD - Fiber over long-wavelength laser, full duplex mode" },
	{ LLDP_DOT3_MAU_1000BASESXHD,	"1000BaseSXHD - Fiber over short-wavelength laser, half duplex mode" },
	{ LLDP_DOT3_MAU_1000BASESXFD,	"1000BaseSXFD - Fiber over short-wavelength laser, full duplex mode" },
	{ LLDP_DOT3_MAU_1000BASECXHD,	"1000BaseCXHD - Copper over 150-Ohm balanced cable, half duplex mode" },
	{ LLDP_DOT3_MAU_1000BASECXFD,	"1000BaseCXFD - Copper over 150-Ohm balanced cable, full duplex mode" },
	{ LLDP_DOT3_MAU_1000BASETHD,	"1000BaseTHD - Four-pair Category 5 UTP, half duplex mode" },
	{ LLDP_DOT3_MAU_1000BASETFD,	"1000BaseTFD - Four-pair Category 5 UTP, full duplex mode" },
	{ LLDP_DOT3_MAU_10GIGBASEX,	"10GigBaseX - X PCS/PMA, unknown PMD." },
	{ LLDP_DOT3_MAU_10GIGBASELX4,	"10GigBaseLX4 - X fiber over WWDM optics" },
	{ LLDP_DOT3_MAU_10GIGBASER,	"10GigBaseR - R PCS/PMA, unknown PMD." },
	{ LLDP_DOT3_MAU_10GIGBASEER,	"10GigBaseER - R fiber over 1550 nm optics" },
	{ LLDP_DOT3_MAU_10GIGBASELR,	"10GigBaseLR - R fiber over 1310 nm optics" },
	{ LLDP_DOT3_MAU_10GIGBASESR,	"10GigBaseSR - R fiber over 850 nm optics" },
	{ LLDP_DOT3_MAU_10GIGBASEW,	"10GigBaseW - W PCS/PMA, unknown PMD." },
	{ LLDP_DOT3_MAU_10GIGBASEEW,	"10GigBaseEW - W fiber over 1550 nm optics" },
	{ LLDP_DOT3_MAU_10GIGBASELW,	"10GigBaseLW - W fiber over 1310 nm optics" },
	{ LLDP_DOT3_MAU_10GIGBASESW,	"10GigBaseSW - W fiber over 850 nm optics" },
	{ LLDP_DOT3_MAU_10GIGBASECX4,	"10GigBaseCX4 - X copper over 8 pair 100-Ohm balanced cable" },
	{ LLDP_DOT3_MAU_2BASETL,	"2BaseTL - Voice grade UTP copper, up to 2700m, optional PAF" },
	{ LLDP_DOT3_MAU_10PASSTS,	"10PassTS - Voice grade UTP copper, up to 750m, optional PAF" },
	{ LLDP_DOT3_MAU_100BASEBX10D,	"100BaseBX10D - One single-mode fiber OLT, long wavelength, 10km" },
	{ LLDP_DOT3_MAU_100BASEBX10U,	"100BaseBX10U - One single-mode fiber ONU, long wavelength, 10km" },
	{ LLDP_DOT3_MAU_100BASELX10,	"100BaseLX10 - Two single-mode fibers, long wavelength, 10km" },
	{ LLDP_DOT3_MAU_1000BASEBX10D,	"1000BaseBX10D - One single-mode fiber OLT, long wavelength, 10km" },
	{ LLDP_DOT3_MAU_1000BASEBX10U,	"1000BaseBX10U - One single-mode fiber ONU, long wavelength, 10km" },
	{ LLDP_DOT3_MAU_1000BASELX10,	"1000BaseLX10 - Two sigle-mode fiber, long wavelength, 10km" },
	{ LLDP_DOT3_MAU_1000BASEPX10D,	"1000BasePX10D - One single-mode fiber EPON OLT, 10km" },
	{ LLDP_DOT3_MAU_1000BASEPX10U,	"1000BasePX10U - One single-mode fiber EPON ONU, 10km" },
	{ LLDP_DOT3_MAU_1000BASEPX20D,	"1000BasePX20D - One single-mode fiber EPON OLT, 20km" },
	{ LLDP_DOT3_MAU_1000BASEPX20U,	"1000BasePX20U - One single-mode fiber EPON ONU, 20km" },
	{ LLDP_DOT3_MAU_10GBASET,	"10GbaseT - Four-pair Category 6A or better, full duplex mode only" },
	{ LLDP_DOT3_MAU_10GBASELRM,	"10GbaseLRM - R multimode fiber over 1310 nm optics" },
	{ LLDP_DOT3_MAU_1000BASEKX,	"1000baseKX - X backplane, full duplex mode only" },
	{ LLDP_DOT3_MAU_10GBASEKX4,	"10GbaseKX4 - 4 lane X backplane, full duplex mode only" },
	{ LLDP_DOT3_MAU_10GBASEKR,	"10GbaseKR - R backplane, full duplex mode only" },
	{ LLDP_DOT3_MAU_10G1GBASEPRXD1,	"10G1GbasePRXD1 - One single-mode fiber asymmetric-rate EPON OLT, low power budget (PRX10)" },
	{ LLDP_DOT3_MAU_10G1GBASEPRXD2, "10G1GbasePRXD2 - One single-mode fiber asymmetric-rate EPON OLT, medium power budget (PRX20)" },
	{ LLDP_DOT3_MAU_10G1GBASEPRXD3, "10G1GbasePRXD3 - One single-mode fiber asymmetric-rate EPON OLT, high power budget (PRX30)" },
	{ LLDP_DOT3_MAU_10G1GBASEPRXU1, "10G1GbasePRXU1 - One single-mode fiber asymmetric-rate EPON ONU, low power budget (PRX10)" },
	{ LLDP_DOT3_MAU_10G1GBASEPRXU2, "10G1GbasePRXU2 - One single-mode fiber asymmetric-rate EPON ONU, medium power budget (PRX20)" },
	{ LLDP_DOT3_MAU_10G1GBASEPRXU3, "10G1GbasePRXU3 - One single-mode fiber asymmetric-rate EPON ONU, high power budget (PRX30)" },
	{ LLDP_DOT3_MAU_10GBASEPRD1,	"10GbasePRD1 - One single-mode fiber symmetric-rate EPON OLT, low power budget (PR10)" },
	{ LLDP_DOT3_MAU_10GBASEPRD2,	"10GbasePRD2 - One single-mode fiber symmetric-rate EPON OLT, medium power budget (PR20)" },
	{ LLDP_DOT3_MAU_10GBASEPRD3,	"10GbasePRD3 - One single-mode fiber symmetric-rate EPON OLT, high power budget (PR30)" },
	{ LLDP_DOT3_MAU_10GBASEPRU1,	"10GbasePRU1 - One single-mode fiber symmetric-rate EPON ONU, low and medium power budget" },
	{ LLDP_DOT3_MAU_10GBASEPRU3,	"10GbasePRU3 - One single-mode fiber symmetric-rate EPON ONU, high power budget (PR30)" },
	{ LLDP_DOT3_MAU_40GBASEKR4,	"40GbaseKR4 - 40GBASE-R PCS/PMA over an electrical backplane" },
	{ LLDP_DOT3_MAU_40GBASECR4,	"40GbaseCR4 - 40GBASE-R PCS/PMA over 4 lane shielded copper balanced cable" },
	{ LLDP_DOT3_MAU_40GBASESR4,	"40GbaseSR4 - 40GBASE-R PCS/PMA over 4 lane multimode fiber" },
	{ LLDP_DOT3_MAU_40GBASEFR,	"40GbaseFR - 40GBASE-R PCS/PMA over single mode fiber" },
	{ LLDP_DOT3_MAU_40GBASELR4,	"40GbaseLR4 - 40GBASE-R PCS/PMA over 4 WDM lane single mode fiber" },
	{ LLDP_DOT3_MAU_100GBASECR10,   "100GbaseCR10 - 100GBASE-R PCS/PMA over 10 lane shielded copper balanced cable" },
	{ LLDP_DOT3_MAU_100GBASESR10,   "100GbaseSR10 - 100GBASE-R PCS/PMA over 10 lane multimode fiber" },
	{ LLDP_DOT3_MAU_100GBASELR4,	"100GbaseLR4 - 100GBASE-R PCS/PMA over 4 WDM lane single mode fiber, long reach" },
	{ LLDP_DOT3_MAU_100GBASEER4,	"100GbaseER4 - 100GBASE-R PCS/PMA over 4 WDM lane single mode fiber PMD, extended reach" },
	{ 0, NULL }
};
#endif

static lldpctl_atom_iter_t*
_lldpctl_atom_iter_ports_list(lldpctl_atom_t *atom)
{
	struct _lldpctl_atom_any_list_t *plist =
	    (struct _lldpctl_atom_any_list_t *)atom;
	return (lldpctl_atom_iter_t*)TAILQ_FIRST(&plist->parent->hardware->h_rports);
}

static lldpctl_atom_iter_t*
_lldpctl_atom_next_ports_list(lldpctl_atom_t *atom, lldpctl_atom_iter_t *iter)
{
	struct lldpd_port *port = (struct lldpd_port *)iter;
	return (lldpctl_atom_iter_t*)TAILQ_NEXT(port, p_entries);
}

static lldpctl_atom_t*
_lldpctl_atom_value_ports_list(lldpctl_atom_t *atom, lldpctl_atom_iter_t *iter)
{
	struct lldpd_port *port = (struct lldpd_port *)iter;
	return _lldpctl_new_atom(atom->conn, atom_port, 0, NULL, port,
	    ((struct _lldpctl_atom_any_list_t *)atom)->parent);
}

static int
_lldpctl_atom_new_port(lldpctl_atom_t *atom, va_list ap)
{
	struct _lldpctl_atom_port_t *port =
	    (struct _lldpctl_atom_port_t *)atom;
	port->local = va_arg(ap, int);
	port->hardware = va_arg(ap, struct lldpd_hardware*);
	port->port = va_arg(ap, struct lldpd_port*);
	port->parent = va_arg(ap, struct _lldpctl_atom_port_t*);
	if (port->parent)
		lldpctl_atom_inc_ref((lldpctl_atom_t*)port->parent);

	if (port->port) {
		/* Internal atom. We are the parent, but our reference count is
		 * not incremented. */
		port->chassis = _lldpctl_new_atom(atom->conn, atom_chassis,
			    port->port->p_chassis, port, 1);
	}
	return 1;
}

TAILQ_HEAD(chassis_list, lldpd_chassis);

static void
add_chassis(struct chassis_list *chassis_list,
	struct lldpd_chassis *chassis)
{
	struct lldpd_chassis *one_chassis;
	TAILQ_FOREACH(one_chassis, chassis_list, c_entries) {
		if (one_chassis == chassis) return;
	}
	TAILQ_INSERT_TAIL(chassis_list,
	    chassis, c_entries);
}

static void
_lldpctl_atom_free_port(lldpctl_atom_t *atom)
{
	struct _lldpctl_atom_port_t *port =
	    (struct _lldpctl_atom_port_t *)atom;
	struct lldpd_hardware *hardware = port->hardware;
	struct lldpd_chassis  *one_chassis, *one_chassis_next;
	struct lldpd_port     *one_port;

	/* Free internal chassis atom. Should be freed immediately since we
	 * should have the only reference. */
	lldpctl_atom_dec_ref((lldpctl_atom_t*)port->chassis);

	/* We need to free the whole struct lldpd_hardware: local port, local
	 * chassis and remote ports... The same chassis may be present several
	 * times. We build a list of chassis (we don't use reference count). */
	struct chassis_list chassis_list;
	TAILQ_INIT(&chassis_list);

	if (port->parent) lldpctl_atom_dec_ref((lldpctl_atom_t*)port->parent);
	else if (!hardware && port->port) {
		/* No parent, no hardware, we assume a single neighbor: one
		 * port, one chassis. */
		if (port->port->p_chassis) {
			lldpd_chassis_cleanup(port->port->p_chassis, 1);
			port->port->p_chassis = NULL;
		}
		lldpd_port_cleanup(port->port, 1);
		free(port->port);
	}
	if (!hardware) return;

	add_chassis(&chassis_list, port->port->p_chassis);
	TAILQ_FOREACH(one_port, &hardware->h_rports, p_entries)
		add_chassis(&chassis_list, one_port->p_chassis);

	/* Free hardware port */
	lldpd_remote_cleanup(hardware, NULL, 1);
	lldpd_port_cleanup(port->port, 1);
	free(port->hardware);

	/* Free list of chassis */
	for (one_chassis = TAILQ_FIRST(&chassis_list);
	     one_chassis != NULL;
	     one_chassis = one_chassis_next) {
		one_chassis_next = TAILQ_NEXT(one_chassis, c_entries);
		lldpd_chassis_cleanup(one_chassis, 1);
	}
}

static lldpctl_atom_t*
_lldpctl_atom_get_atom_port(lldpctl_atom_t *atom, lldpctl_key_t key)
{
	struct _lldpctl_atom_port_t *p =
	    (struct _lldpctl_atom_port_t *)atom;
	struct lldpd_port     *port     = p->port;
	struct lldpd_hardware *hardware = p->hardware;

	/* Local port only */
	if (hardware != NULL) {
		switch (key) {
		case lldpctl_k_port_neighbors:
			return _lldpctl_new_atom(atom->conn, atom_ports_list, p);
		default: break;
		}
	}

	/* Local and remote port */
	switch (key) {
	case lldpctl_k_port_chassis:
		if (port->p_chassis) {
			return _lldpctl_new_atom(atom->conn, atom_chassis,
			    port->p_chassis, p, 0);
		}
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
#ifdef ENABLE_DOT3
	case lldpctl_k_port_dot3_power:
		return _lldpctl_new_atom(atom->conn, atom_dot3_power,
		    p);
#endif
#ifdef ENABLE_DOT1
	case lldpctl_k_port_vlans:
		return _lldpctl_new_atom(atom->conn, atom_vlans_list,
		    p);
	case lldpctl_k_port_ppvids:
		return _lldpctl_new_atom(atom->conn, atom_ppvids_list,
		    p);
	case lldpctl_k_port_pis:
		return _lldpctl_new_atom(atom->conn, atom_pis_list,
		    p);
#endif
#ifdef ENABLE_LLDPMED
	case lldpctl_k_port_med_policies:
		return _lldpctl_new_atom(atom->conn, atom_med_policies_list,
		    p);
	case lldpctl_k_port_med_locations:
		return _lldpctl_new_atom(atom->conn, atom_med_locations_list,
		    p);
	case lldpctl_k_port_med_power:
		return _lldpctl_new_atom(atom->conn, atom_med_power, p);
#endif
#ifdef ENABLE_CUSTOM
	case lldpctl_k_custom_tlvs:
		return _lldpctl_new_atom(atom->conn, atom_custom_list, p);
#endif
	default:
		/* Compatibility: query the associated chassis too */
		if (port->p_chassis)
			return lldpctl_atom_get(p->chassis, key);
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}
}

static lldpctl_atom_t*
_lldpctl_atom_set_atom_port(lldpctl_atom_t *atom, lldpctl_key_t key, lldpctl_atom_t *value)
{
	struct _lldpctl_atom_port_t *p =
	    (struct _lldpctl_atom_port_t *)atom;
	struct lldpd_hardware *hardware = p->hardware;
	struct lldpd_port_set set = {};
	int rc;
	char *canary = NULL;

#ifdef ENABLE_DOT3
	struct _lldpctl_atom_dot3_power_t *dpow;
#endif
#ifdef ENABLE_LLDPMED
	struct _lldpctl_atom_med_power_t *mpow;
	struct _lldpctl_atom_med_policy_t *mpol;
	struct _lldpctl_atom_med_location_t *mloc;
#endif
#ifdef ENABLE_CUSTOM
	struct _lldpctl_atom_custom_t    *custom;
#endif

	/* Local and default port only */
	if (!p->local) {
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}

	set.vlan_tx_enabled = -1;

	switch (key) {
	case lldpctl_k_port_id:
		set.local_id = p->port->p_id;
		break;
	case lldpctl_k_port_descr:
		set.local_descr = p->port->p_descr;
		break;
	case lldpctl_k_port_status:
		set.rxtx = LLDPD_RXTX_FROM_PORT(p->port);
		break;
	case lldpctl_k_port_vlan_tx:
		set.vlan_tx_tag = p->port->p_vlan_tx_tag;
		set.vlan_tx_enabled = p->port->p_vlan_tx_enabled;
		break;
#ifdef ENABLE_DOT3
	case lldpctl_k_port_dot3_power:
		if (value->type != atom_dot3_power) {
			SET_ERROR(atom->conn, LLDPCTL_ERR_INCORRECT_ATOM_TYPE);
			return NULL;
		}

		dpow = (struct _lldpctl_atom_dot3_power_t *)value;
		set.dot3_power = &dpow->parent->port->p_power;
		break;
#endif
#ifdef ENABLE_LLDPMED
	case lldpctl_k_port_med_power:
		if (value->type != atom_med_power) {
			SET_ERROR(atom->conn, LLDPCTL_ERR_INCORRECT_ATOM_TYPE);
			return NULL;
		}

		mpow = (struct _lldpctl_atom_med_power_t *)value;
		set.med_power = &mpow->parent->port->p_med_power;
		break;
	case lldpctl_k_port_med_policies:
		if (value->type != atom_med_policy) {
			SET_ERROR(atom->conn, LLDPCTL_ERR_INCORRECT_ATOM_TYPE);
			return NULL;
		}
		mpol = (struct _lldpctl_atom_med_policy_t *)value;
		set.med_policy = mpol->policy;
		break;
	case lldpctl_k_port_med_locations:
		if (value->type != atom_med_location) {
			SET_ERROR(atom->conn, LLDPCTL_ERR_INCORRECT_ATOM_TYPE);
			return NULL;
		}
		mloc = (struct _lldpctl_atom_med_location_t *)value;
		set.med_location = mloc->location;
		break;
#endif
#ifdef ENABLE_CUSTOM
	case lldpctl_k_custom_tlvs_clear:
		set.custom_list_clear = 1;
		break;
	case lldpctl_k_custom_tlv:
		if (value->type != atom_custom) {
			SET_ERROR(atom->conn, LLDPCTL_ERR_INCORRECT_ATOM_TYPE);
			return NULL;
		}
		custom = (struct _lldpctl_atom_custom_t *)value;
		set.custom = custom->tlv;
		set.custom_tlv_op = custom->op;
		break;
#endif
	default:
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}

	set.ifname = hardware ? hardware->h_ifname : "";

	if (asprintf(&canary, "%d%p%s", key, value, set.ifname) == -1) {
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOMEM);
		return NULL;
	}
	rc = _lldpctl_do_something(atom->conn,
	    CONN_STATE_SET_PORT_SEND, CONN_STATE_SET_PORT_RECV,
	    canary,
	    SET_PORT, &set, &MARSHAL_INFO(lldpd_port_set),
	    NULL, NULL);
	free(canary);
	if (rc == 0) return atom;
	return NULL;
}

static const char*
_lldpctl_atom_get_str_port(lldpctl_atom_t *atom, lldpctl_key_t key)
{
	struct _lldpctl_atom_port_t *p =
	    (struct _lldpctl_atom_port_t *)atom;
	struct lldpd_port     *port     = p->port;
	struct lldpd_hardware *hardware = p->hardware;
	char *ipaddress = NULL; size_t len;

	/* Local port only */
	switch (key) {
	case lldpctl_k_port_name:
		if (hardware != NULL) return hardware->h_ifname;
		break;
	case lldpctl_k_port_status:
		if (p->local) return map_lookup(port_status_map.map,
		    LLDPD_RXTX_FROM_PORT(port));
		break;
	default: break;
	}

	if (!port)
		return NULL;

	/* Local and remote port */
	switch (key) {
	case lldpctl_k_port_protocol:
		return map_lookup(lldpd_protocol_map.map, port->p_protocol);
	case lldpctl_k_port_id_subtype:
		return map_lookup(port_id_subtype_map, port->p_id_subtype);
	case lldpctl_k_port_id:
		switch (port->p_id_subtype) {
		case LLDP_PORTID_SUBTYPE_IFNAME:
		case LLDP_PORTID_SUBTYPE_IFALIAS:
		case LLDP_PORTID_SUBTYPE_LOCAL:
			return port->p_id;
		case LLDP_PORTID_SUBTYPE_LLADDR:
			return _lldpctl_dump_in_atom(atom,
			    (uint8_t*)port->p_id, port->p_id_len,
			    ':', 0);
		case LLDP_PORTID_SUBTYPE_ADDR:
			switch (port->p_id[0]) {
			case LLDP_MGMT_ADDR_IP4: len = INET_ADDRSTRLEN + 1; break;
			case LLDP_MGMT_ADDR_IP6: len = INET6_ADDRSTRLEN + 1; break;
			default: len = 0;
			}
			if (len > 0) {
				ipaddress = _lldpctl_alloc_in_atom(atom, len);
				if (!ipaddress) return NULL;
				if (inet_ntop((port->p_id[0] == LLDP_MGMT_ADDR_IP4)?
					AF_INET:AF_INET6,
					&port->p_id[1], ipaddress, len) == NULL)
					break;
				return ipaddress;
			}
			break;
		}
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	case lldpctl_k_port_descr:
		return port->p_descr;

#ifdef ENABLE_DOT3
	case lldpctl_k_port_dot3_mautype:
		return map_lookup(operational_mau_type_values,
		    port->p_macphy.mau_type);
#endif

	default:
		/* Compatibility: query the associated chassis too */
		return lldpctl_atom_get_str(p->chassis, key);
	}
}

static lldpctl_atom_t*
_lldpctl_atom_set_int_port(lldpctl_atom_t *atom, lldpctl_key_t key,
    long int value)
{
	struct _lldpctl_atom_port_t *p =
	    (struct _lldpctl_atom_port_t *)atom;
	struct lldpd_port     *port     = p->port;

	if (p->local) {
		switch (key) {
		case lldpctl_k_port_status:
			port->p_disable_rx = !LLDPD_RXTX_RXENABLED(value);
			port->p_disable_tx = !LLDPD_RXTX_TXENABLED(value);
			break;
		case lldpctl_k_port_vlan_tx:
			if (value > -1) {
				port->p_vlan_tx_tag = value;
				port->p_vlan_tx_enabled = 1;
			} else
				port->p_vlan_tx_enabled = 0;
			break;
		default:
			SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
			return NULL;
		}
	} else {
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}

	return _lldpctl_atom_set_atom_port(atom, key, NULL);
}

static lldpctl_atom_t*
_lldpctl_atom_set_str_port(lldpctl_atom_t *atom, lldpctl_key_t key,
    const char *value)
{
	struct _lldpctl_atom_port_t *p =
	    (struct _lldpctl_atom_port_t *)atom;
	struct lldpd_port     *port     = p->port;

	if (!value || !strlen(value))
		return NULL;

	if (p->local) {
		switch (key) {
		case lldpctl_k_port_status:
			return _lldpctl_atom_set_int_port(atom, key,
			    map_reverse_lookup(port_status_map.map, value));
		default: break;
		}
	}

	switch (key) {
	case lldpctl_k_port_id:
		free(port->p_id);
		port->p_id = strdup(value);
		port->p_id_len = strlen(value);
		break;
	case lldpctl_k_port_descr:
		free(port->p_descr);
		port->p_descr = strdup(value);
		break;
	default:
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}

	return _lldpctl_atom_set_atom_port(atom, key, NULL);
}

static long int
_lldpctl_atom_get_int_port(lldpctl_atom_t *atom, lldpctl_key_t key)
{
	struct _lldpctl_atom_port_t *p =
	    (struct _lldpctl_atom_port_t *)atom;
	struct lldpd_port     *port     = p->port;
	struct lldpd_hardware *hardware = p->hardware;

	/* Local port only */
	if (hardware != NULL) {
		switch (key) {
		case lldpctl_k_port_index:
			return hardware->h_ifindex;
		case lldpctl_k_tx_cnt:
			return hardware->h_tx_cnt;
		case lldpctl_k_rx_cnt:
			return hardware->h_rx_cnt;
		case lldpctl_k_rx_discarded_cnt:
			return hardware->h_rx_discarded_cnt;
		case lldpctl_k_rx_unrecognized_cnt:
			return hardware->h_rx_unrecognized_cnt;
		case lldpctl_k_ageout_cnt:
			return hardware->h_ageout_cnt;
		case lldpctl_k_insert_cnt:
			return hardware->h_insert_cnt;
		case lldpctl_k_delete_cnt:
			return hardware->h_delete_cnt;
		default: break;
		}
	}
	if (p->local) {
		switch (key) {
		case lldpctl_k_port_status:
			return LLDPD_RXTX_FROM_PORT(port);
		default: break;
		}
	}
	if (!port)
		return SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);

	/* Local and remote port */
	switch (key) {
	case lldpctl_k_port_protocol:
		return port->p_protocol;
	case lldpctl_k_port_age:
		return port->p_lastchange;
	case lldpctl_k_port_ttl:
		return port->p_ttl;
	case lldpctl_k_port_id_subtype:
		return port->p_id_subtype;
	case lldpctl_k_port_hidden:
		return port->p_hidden_in;
	case lldpctl_k_port_vlan_tx:
		return port->p_vlan_tx_enabled ? port->p_vlan_tx_tag : -1;
#ifdef ENABLE_DOT3
	case lldpctl_k_port_dot3_mfs:
		if (port->p_mfs > 0)
			return port->p_mfs;
		break;
	case lldpctl_k_port_dot3_aggregid:
		if (port->p_aggregid > 0)
			return port->p_aggregid;
		break;
	case lldpctl_k_port_dot3_autoneg_support:
		return port->p_macphy.autoneg_support;
	case lldpctl_k_port_dot3_autoneg_enabled:
		return port->p_macphy.autoneg_enabled;
	case lldpctl_k_port_dot3_autoneg_advertised:
		return port->p_macphy.autoneg_advertised;
	case lldpctl_k_port_dot3_mautype:
		return port->p_macphy.mau_type;
#endif
#ifdef ENABLE_DOT1
	case lldpctl_k_port_vlan_pvid:
		return port->p_pvid;
#endif
	default:
		/* Compatibility: query the associated chassis too */
		return lldpctl_atom_get_int(p->chassis, key);
	}
	return SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
}

static const uint8_t*
_lldpctl_atom_get_buf_port(lldpctl_atom_t *atom, lldpctl_key_t key, size_t *n)
{
	struct _lldpctl_atom_port_t *p =
	    (struct _lldpctl_atom_port_t *)atom;
	struct lldpd_port     *port     = p->port;

	switch (key) {
	case lldpctl_k_port_id:
		*n = port->p_id_len;
		return (uint8_t*)port->p_id;
	default:
		/* Compatibility: query the associated chassis too */
		return lldpctl_atom_get_buffer(p->chassis, key, n);
	}
}

static struct atom_builder ports_list =
	{ atom_ports_list, sizeof(struct _lldpctl_atom_any_list_t),
	  .init = _lldpctl_atom_new_any_list,
	  .free = _lldpctl_atom_free_any_list,
	  .iter = _lldpctl_atom_iter_ports_list,
	  .next = _lldpctl_atom_next_ports_list,
	  .value = _lldpctl_atom_value_ports_list };

static struct atom_builder port =
	{ atom_port, sizeof(struct _lldpctl_atom_port_t),
	  .init = _lldpctl_atom_new_port,
	  .free = _lldpctl_atom_free_port,
	  .get  = _lldpctl_atom_get_atom_port,
	  .set  = _lldpctl_atom_set_atom_port,
	  .get_str = _lldpctl_atom_get_str_port,
	  .set_str = _lldpctl_atom_set_str_port,
	  .get_int = _lldpctl_atom_get_int_port,
	  .set_int = _lldpctl_atom_set_int_port,
	  .get_buffer = _lldpctl_atom_get_buf_port };

ATOM_BUILDER_REGISTER(ports_list, 4);
ATOM_BUILDER_REGISTER(port,       5);

