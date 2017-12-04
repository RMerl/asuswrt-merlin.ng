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

#include "lldpctl.h"
#include "../log.h"
#include "atom.h"
#include "helpers.h"

#ifdef ENABLE_DOT1

static lldpctl_atom_iter_t*
_lldpctl_atom_iter_vlans_list(lldpctl_atom_t *atom)
{
	struct _lldpctl_atom_any_list_t *vlist =
	    (struct _lldpctl_atom_any_list_t *)atom;
	return (lldpctl_atom_iter_t*)TAILQ_FIRST(&vlist->parent->port->p_vlans);
}

static lldpctl_atom_iter_t*
_lldpctl_atom_next_vlans_list(lldpctl_atom_t *atom, lldpctl_atom_iter_t *iter)
{
	struct lldpd_vlan *vlan = (struct lldpd_vlan *)iter;
	return (lldpctl_atom_iter_t*)TAILQ_NEXT(vlan, v_entries);
}

static lldpctl_atom_t*
_lldpctl_atom_value_vlans_list(lldpctl_atom_t *atom, lldpctl_atom_iter_t *iter)
{
	struct _lldpctl_atom_any_list_t *vlist =
	    (struct _lldpctl_atom_any_list_t *)atom;
	struct lldpd_vlan *vlan = (struct lldpd_vlan *)iter;
	return _lldpctl_new_atom(atom->conn, atom_vlan, vlist->parent, vlan);
}

static int
_lldpctl_atom_new_vlan(lldpctl_atom_t *atom, va_list ap)
{
	struct _lldpctl_atom_vlan_t *vlan =
	    (struct _lldpctl_atom_vlan_t *)atom;
	vlan->parent = va_arg(ap, struct _lldpctl_atom_port_t *);
	vlan->vlan = va_arg(ap, struct lldpd_vlan *);
	lldpctl_atom_inc_ref((lldpctl_atom_t *)vlan->parent);
	return 1;
}

static void
_lldpctl_atom_free_vlan(lldpctl_atom_t *atom)
{
	struct _lldpctl_atom_vlan_t *vlan =
	    (struct _lldpctl_atom_vlan_t *)atom;
	lldpctl_atom_dec_ref((lldpctl_atom_t *)vlan->parent);
}

static const char*
_lldpctl_atom_get_str_vlan(lldpctl_atom_t *atom, lldpctl_key_t key)
{
	struct _lldpctl_atom_vlan_t *m =
	    (struct _lldpctl_atom_vlan_t *)atom;

	/* Local and remote port */
	switch (key) {
	case lldpctl_k_vlan_name:
		return m->vlan->v_name;
	default:
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}
}

static long int
_lldpctl_atom_get_int_vlan(lldpctl_atom_t *atom, lldpctl_key_t key)
{
	struct _lldpctl_atom_vlan_t *m =
	    (struct _lldpctl_atom_vlan_t *)atom;

	/* Local and remote port */
	switch (key) {
	case lldpctl_k_vlan_id:
		return m->vlan->v_vid;
	default:
		return SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
	}
}

static lldpctl_atom_iter_t*
_lldpctl_atom_iter_ppvids_list(lldpctl_atom_t *atom)
{
	struct _lldpctl_atom_any_list_t *vlist =
	    (struct _lldpctl_atom_any_list_t *)atom;
	return (lldpctl_atom_iter_t*)TAILQ_FIRST(&vlist->parent->port->p_ppvids);
}

static lldpctl_atom_iter_t*
_lldpctl_atom_next_ppvids_list(lldpctl_atom_t *atom, lldpctl_atom_iter_t *iter)
{
	struct lldpd_ppvid *ppvid = (struct lldpd_ppvid *)iter;
	return (lldpctl_atom_iter_t*)TAILQ_NEXT(ppvid, p_entries);
}

static lldpctl_atom_t*
_lldpctl_atom_value_ppvids_list(lldpctl_atom_t *atom, lldpctl_atom_iter_t *iter)
{
	struct _lldpctl_atom_any_list_t *vlist =
	    (struct _lldpctl_atom_any_list_t *)atom;
	struct lldpd_ppvid *ppvid = (struct lldpd_ppvid *)iter;
	return _lldpctl_new_atom(atom->conn, atom_ppvid, vlist->parent, ppvid);
}

static int
_lldpctl_atom_new_ppvid(lldpctl_atom_t *atom, va_list ap)
{
	struct _lldpctl_atom_ppvid_t *ppvid =
	    (struct _lldpctl_atom_ppvid_t *)atom;
	ppvid->parent = va_arg(ap, struct _lldpctl_atom_port_t *);
	ppvid->ppvid = va_arg(ap, struct lldpd_ppvid *);
	lldpctl_atom_inc_ref((lldpctl_atom_t *)ppvid->parent);
	return 1;
}

static void
_lldpctl_atom_free_ppvid(lldpctl_atom_t *atom)
{
	struct _lldpctl_atom_ppvid_t *ppvid =
	    (struct _lldpctl_atom_ppvid_t *)atom;
	lldpctl_atom_dec_ref((lldpctl_atom_t *)ppvid->parent);
}

static long int
_lldpctl_atom_get_int_ppvid(lldpctl_atom_t *atom, lldpctl_key_t key)
{
	struct _lldpctl_atom_ppvid_t *m =
	    (struct _lldpctl_atom_ppvid_t *)atom;

	/* Local and remote port */
	switch (key) {
	case lldpctl_k_ppvid_id:
		return m->ppvid->p_ppvid;
	case lldpctl_k_ppvid_status:
		return m->ppvid->p_cap_status;
	default:
		return SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
	}
}

static lldpctl_atom_iter_t*
_lldpctl_atom_iter_pis_list(lldpctl_atom_t *atom)
{
	struct _lldpctl_atom_any_list_t *vlist =
	    (struct _lldpctl_atom_any_list_t *)atom;
	return (lldpctl_atom_iter_t*)TAILQ_FIRST(&vlist->parent->port->p_pids);
}

static lldpctl_atom_iter_t*
_lldpctl_atom_next_pis_list(lldpctl_atom_t *atom, lldpctl_atom_iter_t *iter)
{
	struct lldpd_pi *pi = (struct lldpd_pi *)iter;
	return (lldpctl_atom_iter_t*)TAILQ_NEXT(pi, p_entries);
}

static lldpctl_atom_t*
_lldpctl_atom_value_pis_list(lldpctl_atom_t *atom, lldpctl_atom_iter_t *iter)
{
	struct _lldpctl_atom_any_list_t *vlist =
	    (struct _lldpctl_atom_any_list_t *)atom;
	struct lldpd_pi *pi = (struct lldpd_pi *)iter;
	return _lldpctl_new_atom(atom->conn, atom_pi, vlist->parent, pi);
}

static int
_lldpctl_atom_new_pi(lldpctl_atom_t *atom, va_list ap)
{
	struct _lldpctl_atom_pi_t *pi =
	    (struct _lldpctl_atom_pi_t *)atom;
	pi->parent = va_arg(ap, struct _lldpctl_atom_port_t *);
	pi->pi = va_arg(ap, struct lldpd_pi *);
	lldpctl_atom_inc_ref((lldpctl_atom_t *)pi->parent);
	return 1;
}

static void
_lldpctl_atom_free_pi(lldpctl_atom_t *atom)
{
	struct _lldpctl_atom_pi_t *pi =
	    (struct _lldpctl_atom_pi_t *)atom;
	lldpctl_atom_dec_ref((lldpctl_atom_t *)pi->parent);
}

static const uint8_t*
_lldpctl_atom_get_buf_pi(lldpctl_atom_t *atom, lldpctl_key_t key, size_t *n)
{
	struct _lldpctl_atom_pi_t *m =
	    (struct _lldpctl_atom_pi_t *)atom;

	/* Local and remote port */
	switch (key) {
	case lldpctl_k_pi_id:
		*n = m->pi->p_pi_len;
		return (const uint8_t*)m->pi->p_pi;
	default:
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}
}

static struct atom_builder vlans_list =
	{ atom_vlans_list, sizeof(struct _lldpctl_atom_any_list_t),
	  .init = _lldpctl_atom_new_any_list,
	  .free = _lldpctl_atom_free_any_list,
	  .iter = _lldpctl_atom_iter_vlans_list,
	  .next = _lldpctl_atom_next_vlans_list,
	  .value = _lldpctl_atom_value_vlans_list };

static struct atom_builder vlan =
	{ atom_vlan, sizeof(struct _lldpctl_atom_vlan_t),
	  .init = _lldpctl_atom_new_vlan,
	  .free = _lldpctl_atom_free_vlan,
	  .get_str = _lldpctl_atom_get_str_vlan,
	  .get_int = _lldpctl_atom_get_int_vlan };

static struct atom_builder ppvids_list =
	{ atom_ppvids_list, sizeof(struct _lldpctl_atom_any_list_t),
	  .init = _lldpctl_atom_new_any_list,
	  .free = _lldpctl_atom_free_any_list,
	  .iter = _lldpctl_atom_iter_ppvids_list,
	  .next = _lldpctl_atom_next_ppvids_list,
	  .value = _lldpctl_atom_value_ppvids_list };

static struct atom_builder ppvid =
	{ atom_ppvid, sizeof(struct _lldpctl_atom_ppvid_t),
	  .init = _lldpctl_atom_new_ppvid,
	  .free = _lldpctl_atom_free_ppvid,
	  .get_int = _lldpctl_atom_get_int_ppvid };

static struct atom_builder pis_list =
	{ atom_pis_list, sizeof(struct _lldpctl_atom_any_list_t),
	  .init = _lldpctl_atom_new_any_list,
	  .free = _lldpctl_atom_free_any_list,
	  .iter = _lldpctl_atom_iter_pis_list,
	  .next = _lldpctl_atom_next_pis_list,
	  .value = _lldpctl_atom_value_pis_list };

static struct atom_builder pi =
	{ atom_pi, sizeof(struct _lldpctl_atom_pi_t),
	  .init = _lldpctl_atom_new_pi,
	  .free = _lldpctl_atom_free_pi,
	  .get_buffer = _lldpctl_atom_get_buf_pi };

ATOM_BUILDER_REGISTER(vlans_list,   9);
ATOM_BUILDER_REGISTER(vlan,        10);
ATOM_BUILDER_REGISTER(ppvids_list, 11);
ATOM_BUILDER_REGISTER(ppvid,       12);
ATOM_BUILDER_REGISTER(pis_list,    13);
ATOM_BUILDER_REGISTER(pi,          14);

#endif

