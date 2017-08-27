/*
 * (C) 2005-2011 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "internal/internal.h"

int __snprintf_expect(char *buf,
		      unsigned int len,
		      const struct nf_expect *exp,
		      unsigned int type,
		      unsigned int msg_output,
		      unsigned int flags)
{
	int size;

	switch(msg_output) {
	case NFCT_O_DEFAULT:
		size = __snprintf_expect_default(buf, len, exp, type, flags);
		break;
	case NFCT_O_XML:
		size = __snprintf_expect_xml(buf, len, exp, type, flags);
		break;
	default:
		errno = ENOENT;
		return -1;
	}

	/* NULL terminated string */
	buf[size+1 > len ? len-1 : size] = '\0';

	return size;
}
