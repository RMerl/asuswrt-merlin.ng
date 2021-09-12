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

static int
_lldpctl_atom_new_interfaces_list(lldpctl_atom_t *atom, va_list ap)
{
	struct _lldpctl_atom_interfaces_list_t *iflist =
	    (struct _lldpctl_atom_interfaces_list_t *)atom;
	iflist->ifs = va_arg(ap, struct lldpd_interface_list *);
	return 1;
}

static void
_lldpctl_atom_free_interfaces_list(lldpctl_atom_t *atom)
{
	struct _lldpctl_atom_interfaces_list_t *iflist =
	    (struct _lldpctl_atom_interfaces_list_t *)atom;
	struct lldpd_interface *iface, *iface_next;
	for (iface = TAILQ_FIRST(iflist->ifs);
	     iface != NULL;
	     iface = iface_next) {
		/* Don't TAILQ_REMOVE, this is not a real list! */
		iface_next = TAILQ_NEXT(iface, next);
		free(iface->name);
		free(iface);
	}
	free(iflist->ifs);
}

static lldpctl_atom_iter_t*
_lldpctl_atom_iter_interfaces_list(lldpctl_atom_t *atom)
{
	struct _lldpctl_atom_interfaces_list_t *iflist =
	    (struct _lldpctl_atom_interfaces_list_t *)atom;
	return (lldpctl_atom_iter_t*)TAILQ_FIRST(iflist->ifs);
}

static lldpctl_atom_iter_t*
_lldpctl_atom_next_interfaces_list(lldpctl_atom_t *atom, lldpctl_atom_iter_t *iter)
{
	return (lldpctl_atom_iter_t*)TAILQ_NEXT((struct lldpd_interface *)iter, next);
}

static lldpctl_atom_t*
_lldpctl_atom_value_interfaces_list(lldpctl_atom_t *atom, lldpctl_atom_iter_t *iter)
{
	struct lldpd_interface *iface = (struct lldpd_interface *)iter;
	return _lldpctl_new_atom(atom->conn, atom_interface, iface->name);
}

static int
_lldpctl_atom_new_interface(lldpctl_atom_t *atom, va_list ap)
{
	struct _lldpctl_atom_interface_t *port =
	    (struct _lldpctl_atom_interface_t *)atom;
	port->name = strdup(va_arg(ap, char *));
	return (port->name != NULL);
}

static void
_lldpctl_atom_free_interface(lldpctl_atom_t *atom)
{
	struct _lldpctl_atom_interface_t *port =
	    (struct _lldpctl_atom_interface_t *)atom;
	free(port->name);
}

static const char*
_lldpctl_atom_get_str_interface(lldpctl_atom_t *atom, lldpctl_key_t key)
{
	struct _lldpctl_atom_interface_t *port =
	    (struct _lldpctl_atom_interface_t *)atom;
	switch (key) {
	case lldpctl_k_interface_name:
		return port->name;
	default:
		SET_ERROR(atom->conn, LLDPCTL_ERR_NOT_EXIST);
		return NULL;
	}
}

static struct atom_builder interfaces_list =
	{ atom_interfaces_list, sizeof(struct _lldpctl_atom_interfaces_list_t),
	  .init  = _lldpctl_atom_new_interfaces_list,
	  .free  = _lldpctl_atom_free_interfaces_list,
	  .iter  = _lldpctl_atom_iter_interfaces_list,
	  .next  = _lldpctl_atom_next_interfaces_list,
	  .value = _lldpctl_atom_value_interfaces_list };

static struct atom_builder interface =
	{ atom_interface, sizeof(struct _lldpctl_atom_interface_t),
	  .init = _lldpctl_atom_new_interface,
	  .free = _lldpctl_atom_free_interface,
	  .get_str = _lldpctl_atom_get_str_interface };

ATOM_BUILDER_REGISTER(interfaces_list, 2);
ATOM_BUILDER_REGISTER(interface,       3);

