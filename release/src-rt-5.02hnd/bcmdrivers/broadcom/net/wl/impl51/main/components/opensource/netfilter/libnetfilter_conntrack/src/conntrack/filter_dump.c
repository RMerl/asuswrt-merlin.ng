/*
 * (C) 2005-2012 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "internal/internal.h"

static void
set_filter_dump_attr_mark(struct nfct_filter_dump *filter_dump,
			  const void *value)
{
	const struct nfct_filter_dump_mark *this = value;

	filter_dump->mark.val = this->val;
	filter_dump->mark.mask = this->mask;
	filter_dump->set |= (1 << NFCT_FILTER_DUMP_MARK);
}

static void
set_filter_dump_attr_family(struct nfct_filter_dump *filter_dump,
			    const void *value)
{
	filter_dump->l3num = *((u_int8_t *)value);
	filter_dump->set |= (1 << NFCT_FILTER_DUMP_L3NUM);
}

const set_filter_dump_attr set_filter_dump_attr_array[NFCT_FILTER_DUMP_MAX] = {
	[NFCT_FILTER_DUMP_MARK]		= set_filter_dump_attr_mark,
	[NFCT_FILTER_DUMP_L3NUM]	= set_filter_dump_attr_family,
};

void __build_filter_dump(struct nfnlhdr *req, size_t size,
			 const struct nfct_filter_dump *filter_dump)
{
	if (filter_dump->set & (1 << NFCT_FILTER_DUMP_MARK)) {
		nfnl_addattr32(&req->nlh, size, CTA_MARK,
				htonl(filter_dump->mark.val));
		nfnl_addattr32(&req->nlh, size, CTA_MARK_MASK,
				htonl(filter_dump->mark.mask));
	}
	if (filter_dump->set & (1 << NFCT_FILTER_DUMP_L3NUM)) {
		struct nfgenmsg *nfg = NLMSG_DATA(&req->nlh);
		nfg->nfgen_family = filter_dump->l3num;
	}
}
