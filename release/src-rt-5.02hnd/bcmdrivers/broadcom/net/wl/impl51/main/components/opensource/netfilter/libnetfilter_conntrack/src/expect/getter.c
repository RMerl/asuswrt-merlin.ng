/*
 * (C) 2005-2011 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "internal/internal.h"

static const void *get_exp_attr_master(const struct nf_expect *exp)
{
	return &exp->master;
}

static const void *get_exp_attr_expected(const struct nf_expect *exp)
{
	return &exp->expected;
}

static const void *get_exp_attr_mask(const struct nf_expect *exp)
{
	return &exp->mask;
}

static const void *get_exp_attr_timeout(const struct nf_expect *exp)
{
	return &exp->timeout;
}

static const void *get_exp_attr_zone(const struct nf_expect *exp)
{
	return &exp->zone;
}

static const void *get_exp_attr_flags(const struct nf_expect *exp)
{
	return &exp->flags;
}

static const void *get_exp_attr_class(const struct nf_expect *exp)
{
	return &exp->class;
}

static const void *get_exp_attr_helper_name(const struct nf_expect *exp)
{
	return exp->helper_name;
}

static const void *get_exp_attr_nat_dir(const struct nf_expect *exp)
{
	return &exp->nat_dir;
}

static const void *get_exp_attr_nat_tuple(const struct nf_expect *exp)
{
	return &exp->nat;
}

static const void *get_exp_attr_expectfn(const struct nf_expect *exp)
{
	return exp->expectfn;
}

const get_exp_attr get_exp_attr_array[ATTR_EXP_MAX] = {
	[ATTR_EXP_MASTER]		= get_exp_attr_master,
	[ATTR_EXP_EXPECTED]		= get_exp_attr_expected,
	[ATTR_EXP_MASK]			= get_exp_attr_mask,
	[ATTR_EXP_TIMEOUT]		= get_exp_attr_timeout,
	[ATTR_EXP_ZONE]			= get_exp_attr_zone,
	[ATTR_EXP_FLAGS]		= get_exp_attr_flags,
	[ATTR_EXP_HELPER_NAME]		= get_exp_attr_helper_name,
	[ATTR_EXP_CLASS]		= get_exp_attr_class,
	[ATTR_EXP_NAT_TUPLE]		= get_exp_attr_nat_tuple,
	[ATTR_EXP_NAT_DIR]		= get_exp_attr_nat_dir,
	[ATTR_EXP_FN]			= get_exp_attr_expectfn,
};
