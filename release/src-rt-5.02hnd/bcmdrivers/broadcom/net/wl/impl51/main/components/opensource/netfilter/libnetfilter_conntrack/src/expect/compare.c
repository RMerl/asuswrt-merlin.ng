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
	if (test_bit(attr, exp1->set) && test_bit(attr, exp2->set)) {
		return cmp(exp1, exp2, flags);
	} else if (flags & NFCT_CMP_MASK &&
		   test_bit(attr, exp1->set)) {
		return 0;
	} else if (flags & NFCT_CMP_STRICT) {
		return 0;
	}
	return 1;
}

static int
cmp_exp_flags(const struct nf_expect *exp1, const struct nf_expect *exp2,
	      unsigned int flags)
{
	return (exp1->flags == exp2->flags);
}

static int
cmp_exp_class(const struct nf_expect *exp1, const struct nf_expect *exp2,
	      unsigned int flags)
{
	return (exp1->class == exp2->class);
}

int __cmp_expect(const struct nf_expect *exp1,
		 const struct nf_expect *exp2,
		 unsigned int flags)
{
	if (!__cmp_orig((struct nf_conntrack *)&exp1->master,
			(struct nf_conntrack *)&exp2->master, flags)) {
		return 0;
	}
	if (!__cmp_orig((struct nf_conntrack *)&exp1->expected,
			(struct nf_conntrack *)&exp2->expected, flags)) {
		return 0;
	}
	if (!__cmp_orig((struct nf_conntrack *)&exp1->mask,
			(struct nf_conntrack *)&exp2->mask, flags)) {
		return 0;
	}
	if (!exp_cmp(ATTR_EXP_FLAGS, exp1, exp2, flags, cmp_exp_flags))
		return 0;
	if (!exp_cmp(ATTR_EXP_CLASS, exp1, exp2, flags, cmp_exp_class))
		return 0;

	return 1;
}
