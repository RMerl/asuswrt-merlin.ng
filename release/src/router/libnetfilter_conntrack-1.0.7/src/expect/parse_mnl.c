/*
 * (C) 2005-2012 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This code has been sponsored by Vyatta Inc. <http://www.vyatta.com>
 */

#include "internal/internal.h"
#include <libmnl/libmnl.h>

static int nlmsg_parse_expection_attr_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	/* skip unsupported attribute in user-space */
	if (mnl_attr_type_valid(attr, CTA_EXPECT_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case CTA_EXPECT_MASTER:
	case CTA_EXPECT_TUPLE:
	case CTA_EXPECT_MASK:
		if (mnl_attr_validate(attr, MNL_TYPE_NESTED) < 0)
			return MNL_CB_ERROR;
		break;
	case CTA_EXPECT_TIMEOUT:
	case CTA_EXPECT_FLAGS:
	case CTA_EXPECT_ID:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			return MNL_CB_ERROR;
		break;
	case CTA_EXPECT_HELP_NAME:
		if (mnl_attr_validate(attr, MNL_TYPE_STRING) < 0)
			return MNL_CB_ERROR;
		break;
	case CTA_EXPECT_ZONE:
		if (mnl_attr_validate(attr, MNL_TYPE_U16) < 0)
			return MNL_CB_ERROR;
		break;
	}
	tb[type] = attr;
	return MNL_CB_OK;
}

int nfexp_nlmsg_parse(const struct nlmsghdr *nlh, struct nf_expect *exp)
{
	struct nlattr *tb[CTA_EXPECT_MAX+1] = {};
	struct nfgenmsg *nfg = mnl_nlmsg_get_payload(nlh);

	mnl_attr_parse(nlh, sizeof(struct nfgenmsg),
			nlmsg_parse_expection_attr_cb, tb);

	if (tb[CTA_EXPECT_MASTER]) {
		exp->expected.orig.l3protonum = nfg->nfgen_family;
		set_bit(ATTR_ORIG_L3PROTO, exp->expected.set);

		nfct_parse_tuple(tb[CTA_EXPECT_MASTER], &exp->master.orig,
				__DIR_ORIG, exp->master.set);
		set_bit(ATTR_EXP_MASTER, exp->set);
	}
	if (tb[CTA_EXPECT_TUPLE]) {
		exp->mask.orig.l3protonum = nfg->nfgen_family;
		set_bit(ATTR_ORIG_L3PROTO, exp->mask.set);

		nfct_parse_tuple(tb[CTA_EXPECT_TUPLE], &exp->expected.orig,
				  __DIR_ORIG, exp->expected.set);
		set_bit(ATTR_EXP_EXPECTED, exp->set);
	}
	if (tb[CTA_EXPECT_MASK]) {
		exp->master.orig.l3protonum = nfg->nfgen_family;
		set_bit(ATTR_ORIG_L3PROTO, exp->master.set);

		nfct_parse_tuple(tb[CTA_EXPECT_MASK], &exp->mask.orig,
				  __DIR_ORIG, exp->mask.set);
		set_bit(ATTR_EXP_MASK, exp->set);
	}
	if (tb[CTA_EXPECT_TIMEOUT]) {
		exp->timeout = ntohl(mnl_attr_get_u32(tb[CTA_EXPECT_TIMEOUT]));
		set_bit(ATTR_EXP_TIMEOUT, exp->set);
	}

	if (tb[CTA_EXPECT_ZONE]) {
		exp->zone = ntohs(mnl_attr_get_u16(tb[CTA_EXPECT_ZONE]));
		set_bit(ATTR_EXP_ZONE, exp->set);
	}

	if (tb[CTA_EXPECT_FLAGS]) {
		exp->flags = ntohl(mnl_attr_get_u32(tb[CTA_EXPECT_FLAGS]));
		set_bit(ATTR_EXP_FLAGS, exp->set);
	}

	if (tb[CTA_EXPECT_HELP_NAME]) {
		strncpy(exp->helper_name,
			mnl_attr_get_str(tb[CTA_EXPECT_HELP_NAME]),
			NFCT_HELPER_NAME_MAX);
		set_bit(ATTR_EXP_HELPER_NAME, exp->set);
	}
	return 0;
}
