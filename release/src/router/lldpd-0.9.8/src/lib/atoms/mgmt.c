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

static int
_lldpctl_atom_new_mgmts_list(lldpctl_atom_t *atom, va_list ap)
{
	struct _lldpctl_atom_mgmts_list_t *plist =
	    (struct _lldpctl_atom_mgmts_list_t *)atom;
	plist->parent = va_arg(ap, lldpctl_atom_t *);
	plist->chassis = va_arg(ap, struct lldpd_chassis *);
	lldpctl_atom_inc_ref(plist->parent);
	return 1;
}

static void
_lldpctl_atom_free_mgmts_list(lldpctl_atom_t *atom)
{
	struct _lldpctl_atom_mgmts_list_t *plist =
	    (struct _lldpctl_atom_mgmts_list_t *)atom;
	lldpctl_atom_dec_ref(plist->parent);
}

static lldpctl_atom_iter_t*
_lldpctl_atom_iter_mgmts_list(lldpctl_atom_t *atom)
{
	struct _lldpctl_atom_mgmts_list_t *plist =
	    (struct _lldpctl_atom_mgmts_list_t *)atom;
	return (lldpctl_atom_iter_t*)TAILQ_FIRST(&plist->chassis->c_mgmt);
}

static lldpctl_atom_iter_t*
_lldpctl_atom_next_mgmts_list(lldpctl_atom_t *atom, lldpctl_atom_iter_t *iter)
{
	struct lldpd_mgmt *mgmt = (struct lldpd_mgmt *)iter;
	return (lldpctl_atom_iter_t*)TAILQ_NEXT(mgmt, m_entries);
}

static lldpctl_atom_t*
_lldpctl_atom_value_mgmts_list(lldpctl_atom_t *atom, lldpctl_atom_iter_t *iter)
{
	struct _lldpctl_atom_mgmts_list_t *plist =
	    (struct _lldpctl_atom_mgmts_list_t *)atom;
	struct lldpd_mgmt *mgmt = (struct lldpd_mgmt *)iter;
	return _lldpctl_new_atom(atom->conn, atom_mgmt, plist->parent, mgmt);
}

static int
_lldpctl_atom_new_mgmt(lldpctl_atom_t *atom, va_list ap)
{
	struct _lldpctl_atom_mgmt_t *mgmt =
	    (struct _lldpctl_atom_mgmt_t *)atom;
	mgmt->parent = va_arg(ap, lldpctl_atom_t *);
	mgmt->mgmt = va_arg(ap, struct lldpd_mgmt *);
	lldpctl_atom_inc_ref(mgmt->parent);
	return 1;
}

static void
_lldpctl_atom_free_mgmt(lldpctl_atom_t *atom)
{
	struct _lldpctl_atom_mgmt_t *mgmt =
	    (struct _lldpctl_atom_mgmt_t *)atom;
	lldpctl_atom_dec_ref(mgmt->parent);
}

static const char*
_lldpctl_atom_get_str_mgmt(lldpctl_atom_t *atom, lldpctl_key_t key)
{
	char *ipaddress = NULL;
	size_t len; int af;
	struct _lldpctl_atom_mgmt_t *m =
	    (struct _lldpctl_atom_mgmt_t *)atom;

	/* Local and remote port */
	switch (key) {
	case lldpctl_k_mgmt_ip:
		switch (m->mgmt->m_family) {
		case LLDPD_AF_IPV4:
			len = INET_ADDRSTRLEN + 1;
			af  = AF_INET;
			break;
		case LLDPD_AF_IPV6:
			len = INET6_ADDRSTRLEN + 1;
			af = AF_INET6;
			break;
		default:
			len = 0;
		}
		if (len == 0) break;
		ipaddress = _lldpctl_alloc_in_atom(atom, len);
		if (!ipaddress) return NULL;
		if (inet_ntop(af, &m->mgmt->m_addr, ipaddress, len) == NULL)
			break;
		return ipaddress;
	default:
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}
	SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
	return NULL;
}

static struct atom_builder mgmts_list =
	{ atom_mgmts_list, sizeof(struct _lldpctl_atom_mgmts_list_t),
	  .init = _lldpctl_atom_new_mgmts_list,
	  .free = _lldpctl_atom_free_mgmts_list,
	  .iter = _lldpctl_atom_iter_mgmts_list,
	  .next = _lldpctl_atom_next_mgmts_list,
	  .value = _lldpctl_atom_value_mgmts_list };

static struct atom_builder mgmt =
	{ atom_mgmt, sizeof(struct _lldpctl_atom_mgmt_t),
	  .init = _lldpctl_atom_new_mgmt,
	  .free = _lldpctl_atom_free_mgmt,
	  .get_str = _lldpctl_atom_get_str_mgmt };

ATOM_BUILDER_REGISTER(mgmts_list, 6);
ATOM_BUILDER_REGISTER(mgmt,       7);

