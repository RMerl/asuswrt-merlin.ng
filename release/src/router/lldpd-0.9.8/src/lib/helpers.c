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

const char*
map_lookup(lldpctl_map_t *list, int n)
{

	unsigned int i;

	for (i = 0; list[i].string != NULL; i++) {
		if (list[i].value == n) {
			return list[i].string;
		}
	}

	return "unknown";
}

int
map_reverse_lookup(lldpctl_map_t *list, const char *string)
{
	if (!string) return -1;

	for (unsigned int i = 0; list[i].string != NULL; i++) {
		if (!strcasecmp(list[i].string, string))
			return list[i].value;
	}

	return -1;
}

int
_lldpctl_atom_new_any_list(lldpctl_atom_t *atom, va_list ap)
{
	struct _lldpctl_atom_any_list_t *plist =
	    (struct _lldpctl_atom_any_list_t *)atom;
	plist->parent = va_arg(ap, struct _lldpctl_atom_port_t *);
	lldpctl_atom_inc_ref((lldpctl_atom_t *)plist->parent);
	return 1;
}

void
_lldpctl_atom_free_any_list(lldpctl_atom_t *atom)
{
	struct _lldpctl_atom_any_list_t *plist =
	    (struct _lldpctl_atom_any_list_t *)atom;
	lldpctl_atom_dec_ref((lldpctl_atom_t *)plist->parent);
}

