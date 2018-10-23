/*
 * (C) 2005-2011 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "internal/internal.h"

int __parse_expect_message_type(const struct nlmsghdr *nlh)
{
	uint16_t type = NFNL_MSG_TYPE(nlh->nlmsg_type);
	uint16_t flags = nlh->nlmsg_flags;
	int ret = NFCT_T_UNKNOWN;

	if (type == IPCTNL_MSG_EXP_NEW) {
		if (flags & (NLM_F_CREATE|NLM_F_EXCL))
			ret = NFCT_T_NEW;
		else
			ret = NFCT_T_UPDATE;
	} else if (type == IPCTNL_MSG_EXP_DELETE)
		ret = NFCT_T_DESTROY;

	return ret;
}

void __parse_expect(const struct nlmsghdr *nlh,
		    struct nfattr *cda[],
		    struct nf_expect *exp)
{
	struct nfgenmsg *nfhdr = NLMSG_DATA(nlh);

	/* XXX: this is ugly, clean it up, please */
	exp->expected.orig.l3protonum = nfhdr->nfgen_family;
	set_bit(ATTR_ORIG_L3PROTO, exp->expected.set);

	exp->mask.orig.l3protonum = nfhdr->nfgen_family;
	set_bit(ATTR_ORIG_L3PROTO, exp->mask.set);

	exp->master.orig.l3protonum = nfhdr->nfgen_family;
	set_bit(ATTR_ORIG_L3PROTO, exp->master.set);

	if (cda[CTA_EXPECT_MASTER-1]) {
		__parse_tuple(cda[CTA_EXPECT_MASTER-1], 
			      &exp->master.orig,
			      __DIR_ORIG,
			      exp->master.set);
		set_bit(ATTR_EXP_MASTER, exp->set);
	}
	if (cda[CTA_EXPECT_TUPLE-1]) {
		__parse_tuple(cda[CTA_EXPECT_TUPLE-1], 
			      &exp->expected.orig,
			      __DIR_ORIG,
			      exp->expected.set);
		set_bit(ATTR_EXP_EXPECTED, exp->set);
	}
	if (cda[CTA_EXPECT_MASK-1]) {
		__parse_tuple(cda[CTA_EXPECT_MASK-1], 
			      &exp->mask.orig,
			      __DIR_ORIG,
			      exp->mask.set);
		set_bit(ATTR_EXP_MASK, exp->set);
	}
	if (cda[CTA_EXPECT_TIMEOUT-1]) {
		exp->timeout = 
		      ntohl(*(uint32_t *)NFA_DATA(cda[CTA_EXPECT_TIMEOUT-1]));
		set_bit(ATTR_EXP_TIMEOUT, exp->set);
	}

	if (cda[CTA_EXPECT_ZONE-1]) {
		exp->zone =
		      ntohs(*(uint16_t *)NFA_DATA(cda[CTA_EXPECT_ZONE-1]));
		set_bit(ATTR_EXP_ZONE, exp->set);
	}
	if (cda[CTA_EXPECT_FLAGS-1]) {
		exp->flags =
		      ntohl(*(uint32_t *)NFA_DATA(cda[CTA_EXPECT_FLAGS-1]));
		set_bit(ATTR_EXP_FLAGS, exp->set);
	}
	if (cda[CTA_EXPECT_HELP_NAME-1]) {
		strncpy(exp->helper_name, NFA_DATA(cda[CTA_EXPECT_HELP_NAME-1]),
			NFA_PAYLOAD(cda[CTA_EXPECT_HELP_NAME-1]));
		set_bit(ATTR_EXP_HELPER_NAME, exp->set);
	}
	if (cda[CTA_EXPECT_CLASS-1]) {
		exp->class =
		      ntohl(*(uint32_t *)NFA_DATA(cda[CTA_EXPECT_CLASS-1]));
		set_bit(ATTR_EXP_CLASS, exp->set);
	}
	if (cda[CTA_EXPECT_NAT-1]) {
		struct nfattr *tb[CTA_EXPECT_NAT_MAX];

		exp->nat.orig.l3protonum = nfhdr->nfgen_family;
		set_bit(ATTR_ORIG_L3PROTO, exp->nat.set);

		nfnl_parse_nested(tb, CTA_EXPECT_NAT_MAX,
					cda[CTA_EXPECT_NAT-1]);

		if (tb[CTA_EXPECT_NAT_TUPLE-1]) {
			__parse_tuple(tb[CTA_EXPECT_NAT_TUPLE-1],
				      &exp->nat.orig,
				      __DIR_ORIG,
				      exp->nat.set);
			set_bit(ATTR_EXP_NAT_TUPLE, exp->set);
		}
		if (tb[CTA_EXPECT_NAT_DIR-1]) {
			exp->nat_dir =
			      ntohl(*((uint32_t *)
				NFA_DATA(tb[CTA_EXPECT_NAT_DIR-1])));
			set_bit(ATTR_EXP_NAT_DIR, exp->set);
		}
	}
	if (cda[CTA_EXPECT_FN-1]) {
		strcpy(exp->expectfn, NFA_DATA(cda[CTA_EXPECT_FN-1]));
		exp->expectfn[__NFCT_EXPECTFN_MAX-1] = '\0';
		set_bit(ATTR_EXP_FN, exp->set);
	}
}
