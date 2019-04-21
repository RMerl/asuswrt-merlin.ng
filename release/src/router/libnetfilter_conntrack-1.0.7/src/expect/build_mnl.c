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

int
nfexp_nlmsg_build(struct nlmsghdr *nlh, const struct nf_expect *exp)
{
	if (test_bit(ATTR_EXP_EXPECTED, exp->set))
		nfct_build_tuple(nlh, &exp->expected.orig, CTA_EXPECT_TUPLE);

	if (test_bit(ATTR_EXP_MASTER, exp->set))
		nfct_build_tuple(nlh, &exp->master.orig, CTA_EXPECT_MASTER);

	if (test_bit(ATTR_EXP_MASK, exp->set))
		nfct_build_tuple(nlh, &exp->mask.orig, CTA_EXPECT_MASK);

	if (test_bit(ATTR_EXP_TIMEOUT, exp->set))
		mnl_attr_put_u32(nlh, CTA_EXPECT_TIMEOUT, htonl(exp->timeout));

	if (test_bit(ATTR_EXP_FLAGS, exp->set))
		mnl_attr_put_u32(nlh, CTA_EXPECT_FLAGS, htonl(exp->flags));

	if (test_bit(ATTR_EXP_ZONE, exp->set))
		mnl_attr_put_u16(nlh, CTA_EXPECT_ZONE, htons(exp->zone));

	if (test_bit(ATTR_EXP_HELPER_NAME, exp->set))
		mnl_attr_put_strz(nlh, CTA_EXPECT_HELP_NAME, exp->helper_name);

	return 0;
}
