/*
 * (C) 2005-2011 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "internal/internal.h"

static void __build_timeout(struct nfnlhdr *req,
			    size_t size,
			    const struct nf_expect *exp)
{
	nfnl_addattr32(&req->nlh, size, CTA_EXPECT_TIMEOUT,htonl(exp->timeout));
}

static void __build_zone(struct nfnlhdr *req, size_t size,
			 const struct nf_expect *exp)
{
	nfnl_addattr16(&req->nlh, size, CTA_EXPECT_ZONE, htons(exp->zone));
}

static void __build_flags(struct nfnlhdr *req,
			  size_t size, const struct nf_expect *exp)
{
	nfnl_addattr32(&req->nlh, size, CTA_EXPECT_FLAGS,htonl(exp->flags));
}

static void __build_class(struct nfnlhdr *req,
			  size_t size,
			  const struct nf_expect *exp)
{
	nfnl_addattr32(&req->nlh, size, CTA_EXPECT_CLASS, htonl(exp->class));
}

static void __build_helper_name(struct nfnlhdr *req, size_t size,
			 const struct nf_expect *exp)
{
	nfnl_addattr_l(&req->nlh, size, CTA_EXPECT_HELP_NAME,
			exp->helper_name, strlen(exp->helper_name)+1);
}

static void __build_expectfn(struct nfnlhdr *req,
			     size_t size, const struct nf_expect *exp)
{
	nfnl_addattr_l(&req->nlh, size, CTA_EXPECT_FN,
			exp->expectfn, strlen(exp->expectfn)+1);
}

int __build_expect(struct nfnl_subsys_handle *ssh,
		   struct nfnlhdr *req,
		   size_t size,
		   u_int16_t type,
		   u_int16_t flags,
		   const struct nf_expect *exp)
{
	u_int8_t l3num;

	if (test_bit(ATTR_ORIG_L3PROTO, exp->master.set))
		l3num = exp->master.orig.l3protonum;
	else if (test_bit(ATTR_ORIG_L3PROTO, exp->expected.set))
		l3num = exp->expected.orig.l3protonum;
	else
		return -1;

	memset(req, 0, size);

	nfnl_fill_hdr(ssh, &req->nlh, 0, l3num, 0, type, flags);

	if (test_bit(ATTR_EXP_EXPECTED, exp->set)) {
		__build_tuple(req, size, &exp->expected.orig, CTA_EXPECT_TUPLE);
	}

	if (test_bit(ATTR_EXP_MASTER, exp->set)) {
		__build_tuple(req, size, &exp->master.orig, CTA_EXPECT_MASTER);
	}

	if (test_bit(ATTR_EXP_MASK, exp->set)) {
		__build_tuple(req, size, &exp->mask.orig, CTA_EXPECT_MASK);
	}

	if (test_bit(ATTR_EXP_NAT_TUPLE, exp->set) &&
	    test_bit(ATTR_EXP_NAT_DIR, exp->set)) {
		struct nfattr *nest;

		nest = nfnl_nest(&req->nlh, size, CTA_EXPECT_NAT);
		__build_tuple(req, size, &exp->nat.orig, CTA_EXPECT_NAT_TUPLE);
		nfnl_addattr32(&req->nlh, size, CTA_EXPECT_NAT_DIR,
				htonl(exp->nat_dir));
		nfnl_nest_end(&req->nlh, nest);
	}

	if (test_bit(ATTR_EXP_TIMEOUT, exp->set))
		__build_timeout(req, size, exp);
	if (test_bit(ATTR_EXP_FLAGS, exp->set))
		__build_flags(req, size, exp);
	if (test_bit(ATTR_EXP_ZONE, exp->set))
		__build_zone(req, size, exp);
	if (test_bit(ATTR_EXP_CLASS, exp->set))
		__build_class(req, size, exp);
	if (test_bit(ATTR_EXP_HELPER_NAME, exp->set))
		__build_helper_name(req, size, exp);
	if (test_bit(ATTR_EXP_FN, exp->set))
		__build_expectfn(req, size, exp);

	return 0;
}
