/*
 * (C) 2005-2012 by Pablo Neira Ayuso <pablo@netfilter.org>
 * (C) 2012 by Vyatta Inc. <http://www.vyatta.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "internal/internal.h"

static int exp_cmp(int attr,
		   const struct nf_expect *exp1,
		   const struct nf_expect *exp2,
		   unsigned int flags,
		   int (*cmp)(const struct nf_expect *exp1,
			      const struct nf_expect *exp2,
			      unsigned int flags))
{
	int a = test_bit(attr, exp1->set);
	int b = test_bit(attr, exp2->set);

	if (a && b) {
		return cmp(exp1, exp2, flags);
	} else if (!a && !b) {
		return 1;
	} else if (flags & NFCT_CMP_MASK &&
		   test_bit(attr, exp1->set)) {
		return 0;
	} else if (flags & NFCT_CMP_STRICT) {
		return 0;
	}
	return 1;
}

static int
cmp_exp_master(const struct nf_expect *exp1, const struct nf_expect *exp2,
	       unsigned int flags)
{
	return __cmp_orig((struct nf_conntrack *)&exp1->master,
			  (struct nf_conntrack *)&exp2->master, flags);
}

static int
cmp_exp_expected(const struct nf_expect *exp1, const struct nf_expect *exp2,
		 unsigned int flags)
{
	return __cmp_orig((struct nf_conntrack *)&exp1->expected,
			  (struct nf_conntrack *)&exp2->expected, flags);
}

static int
cmp_exp_mask(const struct nf_expect *exp1, const struct nf_expect *exp2,
	     unsigned int flags)
{
	return __cmp_orig((struct nf_conntrack *)&exp1->mask,
			  (struct nf_conntrack *)&exp2->mask, flags);

}

static int
cmp_exp_zone(const struct nf_expect *exp1, const struct nf_expect *exp2,
	     unsigned int flags)
{
	return exp1->zone == exp2->zone;
}

static int
cmp_exp_flags(const struct nf_expect *exp1, const struct nf_expect *exp2,
	      unsigned int flags)
{
	return (exp1->flags == exp2->flags);
}

static int
cmp_exp_hname(const struct nf_expect *exp1, const struct nf_expect *exp2,
	      unsigned int flags)
{
	return strcmp(exp1->helper_name, exp2->helper_name) == 0;
}

static int
cmp_exp_class(const struct nf_expect *exp1, const struct nf_expect *exp2,
	      unsigned int flags)
{
	return (exp1->class == exp2->class);
}

static int
cmp_exp_natt(const struct nf_expect *exp1, const struct nf_expect *exp2,
	     unsigned int flags)
{
	return __cmp_orig((struct nf_conntrack *)&exp1->nat,
			  (struct nf_conntrack *)&exp2->nat, flags);

}

static int
cmp_exp_natdir(const struct nf_expect *exp1, const struct nf_expect *exp2,
	       unsigned int flags)
{
	return exp1->nat_dir == exp2->nat_dir;
}

static int
cmp_exp_expfn(const struct nf_expect *exp1, const struct nf_expect *exp2,
	      unsigned int flags)
{
	return strcmp(exp1->expectfn, exp2->expectfn) == 0;
}


int __cmp_expect(const struct nf_expect *exp1,
		 const struct nf_expect *exp2,
		 unsigned int flags)
{
	if (!exp_cmp(ATTR_EXP_MASTER, exp1, exp2, flags, cmp_exp_master))
		return 0;
	if (!exp_cmp(ATTR_EXP_EXPECTED, exp1, exp2, flags, cmp_exp_expected))
		return 0;
	if (!exp_cmp(ATTR_EXP_MASK, exp1, exp2, flags, cmp_exp_mask))
		return 0;
	/* ATTR_EXP_TIMEOUT is intentionally not compared at this time; the expectations should
	 * be considered equal if only the timeout is different */
	if (!exp_cmp(ATTR_EXP_ZONE, exp1, exp2, flags, cmp_exp_zone))
		return 0;
	if (!exp_cmp(ATTR_EXP_FLAGS, exp1, exp2, flags, cmp_exp_flags))
		return 0;
	if (!exp_cmp(ATTR_EXP_HELPER_NAME, exp1, exp2, flags, cmp_exp_hname))
		return 0;
	if (!exp_cmp(ATTR_EXP_CLASS, exp1, exp2, flags, cmp_exp_class))
		return 0;
	if (!exp_cmp(ATTR_EXP_NAT_TUPLE, exp1, exp2, flags, cmp_exp_natt))
		return 0;
	if (!exp_cmp(ATTR_EXP_NAT_DIR, exp1, exp2, flags, cmp_exp_natdir))
		return 0;
	if (!exp_cmp(ATTR_EXP_FN, exp1, exp2, flags, cmp_exp_expfn))
		return 0;
	return 1;
}
