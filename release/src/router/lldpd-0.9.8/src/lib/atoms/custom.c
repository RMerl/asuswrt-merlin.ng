/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2015 Vincent Bernat <vincent@bernat.im>
 * Copyright (c) 2015 Alexandru Ardelean <ardeleanalex@gmail.com>
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

#ifdef ENABLE_CUSTOM

#define min(x,y)	( (x > y) ? y : x )

static lldpctl_atom_iter_t*
_lldpctl_atom_iter_custom_list(lldpctl_atom_t *atom)
{
	struct _lldpctl_atom_custom_list_t *custom =
	    (struct _lldpctl_atom_custom_list_t *)atom;
	return (lldpctl_atom_iter_t*)TAILQ_FIRST(&custom->parent->port->p_custom_list);
}

static lldpctl_atom_iter_t*
_lldpctl_atom_next_custom_list(lldpctl_atom_t *atom, lldpctl_atom_iter_t *iter)
{
	return (lldpctl_atom_iter_t*)TAILQ_NEXT((struct lldpd_custom *)iter, next);
}

static lldpctl_atom_t*
_lldpctl_atom_value_custom_list(lldpctl_atom_t *atom, lldpctl_atom_iter_t *iter)
{
	struct _lldpctl_atom_custom_list_t *custom =
	    (struct _lldpctl_atom_custom_list_t *)atom;
	struct lldpd_custom *tlv = (struct lldpd_custom *) iter;
	return _lldpctl_new_atom(atom->conn, atom_custom, custom->parent, tlv);
}

static lldpctl_atom_t*
_lldpctl_atom_create_custom_list(lldpctl_atom_t *atom)
{
	struct _lldpctl_atom_custom_list_t *custom =
	    (struct _lldpctl_atom_custom_list_t *)atom;
	struct lldpd_custom *tlv;

	tlv = _lldpctl_alloc_in_atom(atom, sizeof(struct lldpd_custom));
	if (!tlv)
		return NULL;
	return _lldpctl_new_atom(atom->conn, atom_custom, custom->parent, tlv);
}

static int
_lldpctl_atom_new_custom(lldpctl_atom_t *atom, va_list ap)
{
	struct _lldpctl_atom_custom_t *custom =
	    (struct _lldpctl_atom_custom_t *)atom;

	custom->parent = va_arg(ap, struct _lldpctl_atom_port_t *);
	custom->tlv    = va_arg(ap, struct lldpd_custom *);
	lldpctl_atom_inc_ref((lldpctl_atom_t *)custom->parent);
	return 1;
}

static void
_lldpctl_atom_free_custom(lldpctl_atom_t *atom)
{
	struct _lldpctl_atom_custom_t *custom =
	    (struct _lldpctl_atom_custom_t *)atom;
	lldpctl_atom_dec_ref((lldpctl_atom_t *)custom->parent);
}

static long int
_lldpctl_atom_get_int_custom(lldpctl_atom_t *atom, lldpctl_key_t key)
{
	struct _lldpctl_atom_custom_t *custom =
	    (struct _lldpctl_atom_custom_t *)atom;

	switch (key) {
	case lldpctl_k_custom_tlv_oui_subtype:
		return custom->tlv->subtype;
	default:
		return SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
	}
}

static lldpctl_atom_t*
_lldpctl_atom_set_str_custom(lldpctl_atom_t *atom, lldpctl_key_t key,
    const char *value)
{
	struct _lldpctl_atom_custom_t *custom =
	    (struct _lldpctl_atom_custom_t *)atom;

	if (!value || !strlen(value))
		return NULL;

	/* Only local port can be modified */
	if (!custom->parent->local) {
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}

	switch (key) {
	case lldpctl_k_custom_tlv_op:
		if (!strcmp(value, "replace"))
			custom->op = CUSTOM_TLV_REPLACE;
		else if (!strcmp(value, "remove"))
			custom->op = CUSTOM_TLV_REMOVE;
		else
			custom->op = CUSTOM_TLV_ADD;
		return atom;
	default:
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}

	SET_ERROR(atom->conn, LLDPCTL_ERR_BAD_VALUE);
	return NULL;
}

static lldpctl_atom_t*
_lldpctl_atom_set_int_custom(lldpctl_atom_t *atom, lldpctl_key_t key,
    long int value)
{
	struct _lldpctl_atom_custom_t *custom =
	    (struct _lldpctl_atom_custom_t *)atom;

	/* Only local port can be modified */
	if (!custom->parent->local) {
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}

	switch (key) {
	case lldpctl_k_custom_tlv_oui_subtype:
		if (value < 0 || value > 255) goto bad;
		custom->tlv->subtype = value;
		return atom;
	default:
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}
bad:
	SET_ERROR(atom->conn, LLDPCTL_ERR_BAD_VALUE);
	return NULL;
}

static const uint8_t*
_lldpctl_atom_get_buffer_custom(lldpctl_atom_t *atom, lldpctl_key_t key, size_t *n)
{
	struct _lldpctl_atom_custom_t *custom =
	    (struct _lldpctl_atom_custom_t *)atom;

	switch (key) {
	case lldpctl_k_custom_tlv_oui:
		*n = sizeof(custom->tlv->oui);
		return (const uint8_t *)&custom->tlv->oui;
	case lldpctl_k_custom_tlv_oui_info_string:
		*n = custom->tlv->oui_info_len;
		return (const uint8_t *)custom->tlv->oui_info;
	default:
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}
}

static lldpctl_atom_t*
_lldpctl_atom_set_buffer_custom(lldpctl_atom_t *atom, lldpctl_key_t key,
    const u_int8_t *buf, size_t n)
{
	struct _lldpctl_atom_custom_t *custom =
	    (struct _lldpctl_atom_custom_t *)atom;

	/* Only local port can be modified */
	if (!custom->parent->local) {
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}

	switch (key) {
	case lldpctl_k_custom_tlv_oui:
		memcpy(&custom->tlv->oui, buf, min(n, sizeof(custom->tlv->oui)));
		return atom;
	case lldpctl_k_custom_tlv_oui_info_string:
		if (n == 0 || n > LLDP_TLV_ORG_OUI_INFO_MAXLEN) {
			SET_ERROR(atom->conn, LLDPCTL_ERR_BAD_VALUE);
			return NULL;
		}
		custom->tlv->oui_info_len = n;
		if (!(custom->tlv->oui_info = _lldpctl_alloc_in_atom(atom, n))) {
			custom->tlv->oui_info_len = 0;
			SET_ERROR(atom->conn, LLDPCTL_ERR_NOMEM);
			return NULL;
		}
		memcpy(custom->tlv->oui_info, buf, n);
		return atom;
	default:
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}
}

static struct atom_builder custom_list =
	{ atom_custom_list, sizeof(struct _lldpctl_atom_any_list_t),
	  .init  = _lldpctl_atom_new_any_list,
	  .free  = _lldpctl_atom_free_any_list,
	  .iter  = _lldpctl_atom_iter_custom_list,
	  .next  = _lldpctl_atom_next_custom_list,
	  .value = _lldpctl_atom_value_custom_list, 
	  .create = _lldpctl_atom_create_custom_list };

static struct atom_builder custom =
	{ atom_custom, sizeof(struct _lldpctl_atom_custom_t),
	  .init = _lldpctl_atom_new_custom,
	  .free = _lldpctl_atom_free_custom,
	  .get_int = _lldpctl_atom_get_int_custom,
	  .set_int = _lldpctl_atom_set_int_custom,
	  .set_str = _lldpctl_atom_set_str_custom,
	  .get_buffer = _lldpctl_atom_get_buffer_custom,
	  .set_buffer = _lldpctl_atom_set_buffer_custom };

ATOM_BUILDER_REGISTER(custom_list, 22);
ATOM_BUILDER_REGISTER(custom,      23);

#endif /* ENABLE_CUSTOM */

