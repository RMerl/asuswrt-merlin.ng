/* strongSwan IPsec starter comparison functions
 * Copyright (C) 2001-2002 Mathieu Lafon - Arkoon Network Security
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include <string.h>

#include "confread.h"
#include "args.h"
#include "cmp.h"

#define VARCMP(obj) if (c1->obj != c2->obj) return FALSE
#define STRCMP(obj) if (strcmp(c1->obj,c2->obj)) return FALSE

static bool starter_cmp_end(starter_end_t *c1, starter_end_t *c2)
{
	if ((c1 == NULL) || (c2 == NULL))
		return FALSE;

	VARCMP(modecfg);
	VARCMP(from_port);
	VARCMP(to_port);
	VARCMP(protocol);

	return cmp_args(KW_END_FIRST, KW_END_LAST, (char *)c1, (char *)c2);
}

bool starter_cmp_conn(starter_conn_t *c1, starter_conn_t *c2)
{
	if ((c1 == NULL) || (c2 == NULL))
		return FALSE;

	VARCMP(mode);
	VARCMP(proxy_mode);
	VARCMP(options);
	VARCMP(mark_in.value);
	VARCMP(mark_in.mask);
	VARCMP(mark_out.value);
	VARCMP(mark_in.mask);
	VARCMP(tfc);
	VARCMP(sa_keying_tries);

	if (!starter_cmp_end(&c1->left, &c2->left))
		return FALSE;
	if (!starter_cmp_end(&c1->right, &c2->right))
		return FALSE;

	return cmp_args(KW_CONN_NAME, KW_CONN_LAST, (char *)c1, (char *)c2);
}

bool starter_cmp_ca(starter_ca_t *c1, starter_ca_t *c2)
{
	if (c1 ==  NULL || c2 == NULL)
		return FALSE;

	return cmp_args(KW_CA_NAME, KW_CA_LAST, (char *)c1, (char *)c2);
}
