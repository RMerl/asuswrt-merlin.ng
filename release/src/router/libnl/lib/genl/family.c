/*
 * lib/genl/family.c		Generic Netlink Family
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2012 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup genl_ctrl
 * @defgroup genl_family Generic Netlink Family Object
 *
 * Object representing a kernel side registered Generic Netlink family
 *
 * @{
 */

#include <netlink-private/genl.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/utils.h>

/** @cond SKIP */
#define FAMILY_ATTR_ID		0x01
#define FAMILY_ATTR_NAME	0x02
#define FAMILY_ATTR_VERSION	0x04
#define FAMILY_ATTR_HDRSIZE	0x08
#define FAMILY_ATTR_MAXATTR	0x10
#define FAMILY_ATTR_OPS		0x20

struct nl_object_ops genl_family_ops;

static void family_constructor(struct nl_object *c)
{
	struct genl_family *family = (struct genl_family *) c;

	nl_init_list_head(&family->gf_ops);
	nl_init_list_head(&family->gf_mc_grps);
}

static void family_free_data(struct nl_object *c)
{
	struct genl_family *family = (struct genl_family *) c;
	struct genl_family_op *ops, *tmp;
	struct genl_family_grp *grp, *t_grp;

	if (family == NULL)
		return;

	nl_list_for_each_entry_safe(ops, tmp, &family->gf_ops, o_list) {
		nl_list_del(&ops->o_list);
		free(ops);
	}

	nl_list_for_each_entry_safe(grp, t_grp, &family->gf_mc_grps, list) {
		nl_list_del(&grp->list);
		free(grp);
	}

}

static int family_clone(struct nl_object *_dst, struct nl_object *_src)
{
	struct genl_family *dst = nl_object_priv(_dst);
	struct genl_family *src = nl_object_priv(_src);
	struct genl_family_op *ops;
	struct genl_family_grp *grp;
	int err;

	nl_list_for_each_entry(ops, &src->gf_ops, o_list) {
		err = genl_family_add_op(dst, ops->o_id, ops->o_flags);
		if (err < 0)
			return err;
	}

	nl_list_for_each_entry(grp, &src->gf_mc_grps, list) {
		err = genl_family_add_grp(dst, grp->id, grp->name);
		if (err < 0)
			return err;
	}

	
	return 0;
}

static void family_dump_line(struct nl_object *obj, struct nl_dump_params *p)
{
	struct genl_family *family = (struct genl_family *) obj;

	nl_dump(p, "0x%04x %s version %u\n",
		family->gf_id, family->gf_name, family->gf_version);
}

static const struct trans_tbl ops_flags[] = {
	__ADD(GENL_ADMIN_PERM, admin_perm)
	__ADD(GENL_CMD_CAP_DO, has_doit)
	__ADD(GENL_CMD_CAP_DUMP, has_dump)
	__ADD(GENL_CMD_CAP_HASPOL, has_policy)
};

static char *ops_flags2str(int flags, char *buf, size_t len)
{
	return __flags2str(flags, buf, len, ops_flags, ARRAY_SIZE(ops_flags));
}

static void family_dump_details(struct nl_object *obj, struct nl_dump_params *p)
{
	struct genl_family_grp *grp;
	struct genl_family *family = (struct genl_family *) obj;

	family_dump_line(obj, p);
	nl_dump_line(p, "    hdrsize %u maxattr %u\n",
		     family->gf_hdrsize, family->gf_maxattr);

	if (family->ce_mask & FAMILY_ATTR_OPS) {
		struct genl_family_op *op;
		char buf[64];

		nl_list_for_each_entry(op, &family->gf_ops, o_list) {
			ops_flags2str(op->o_flags, buf, sizeof(buf));

			genl_op2name(family->gf_id, op->o_id, buf, sizeof(buf));

			nl_dump_line(p, "      op %s (0x%02x)", buf, op->o_id);

			if (op->o_flags)
				nl_dump(p, " <%s>",
					ops_flags2str(op->o_flags, buf,
						      sizeof(buf)));

			nl_dump(p, "\n");
		}
	}

	nl_list_for_each_entry(grp, &family->gf_mc_grps, list) {
		nl_dump_line(p, "      grp %s (0x%02x)\n", grp->name, grp->id);
	}

}

static void family_dump_stats(struct nl_object *obj, struct nl_dump_params *p)
{
	family_dump_details(obj, p);
}

static int family_compare(struct nl_object *_a, struct nl_object *_b,
			  uint32_t attrs, int flags)
{
	struct genl_family *a = (struct genl_family *) _a;
	struct genl_family *b = (struct genl_family *) _b;
	int diff = 0;

#define FAM_DIFF(ATTR, EXPR) ATTR_DIFF(attrs, FAMILY_ATTR_##ATTR, a, b, EXPR)

	diff |= FAM_DIFF(ID,		a->gf_id != b->gf_id);
	diff |= FAM_DIFF(VERSION,	a->gf_version != b->gf_version);
	diff |= FAM_DIFF(HDRSIZE,	a->gf_hdrsize != b->gf_hdrsize);
	diff |= FAM_DIFF(MAXATTR,	a->gf_maxattr != b->gf_maxattr);
	diff |= FAM_DIFF(NAME,		strcmp(a->gf_name, b->gf_name));

#undef FAM_DIFF

	return diff;
}
/** @endcond */

/**
 * @name Object Allocation
 * @{
 */

/**
 * Allocate new Generic Netlink family object
 * 
 * @return Newly allocated Generic Netlink family object or NULL.
 */
struct genl_family *genl_family_alloc(void)
{
	return (struct genl_family *) nl_object_alloc(&genl_family_ops);
}

/**
 * Release reference on Generic Netlink family object
 * @arg family		Generic Netlink family object
 *
 * Reduces the reference counter of a Generic Netlink family object by one.
 * The object is freed after the last user has returned its reference.
 *
 * @see nl_object_put()
 */
void genl_family_put(struct genl_family *family)
{
	nl_object_put((struct nl_object *) family);
}

/** @} */

/**
 * @name Numeric Identifier
 * @{
 */

/**
 * Return numeric identifier
 * @arg family		Generic Netlink family object
 *
 * @return Numeric identifier or 0 if not available.
 */
unsigned int genl_family_get_id(struct genl_family *family)
{
	if (family->ce_mask & FAMILY_ATTR_ID)
		return family->gf_id;
	else
		return GENL_ID_GENERATE;
}

/**
 * Set the numeric identifier
 * @arg family		Generic Netlink family object
 * @arg id		New numeric identifier
 */
void genl_family_set_id(struct genl_family *family, unsigned int id)
{
	family->gf_id = id;
	family->ce_mask |= FAMILY_ATTR_ID;
}

/** @} */

/**
 * @name Human Readable Name
 * @{
 */

/**
 * Return human readable name
 * @arg family		Generic Netlink family object
 *
 * @return Name of family or NULL if not available
 */
char *genl_family_get_name(struct genl_family *family)
{
	if (family->ce_mask & FAMILY_ATTR_NAME)
		return family->gf_name;
	else
		return NULL;
}

/**
 * Set human readable name
 * @arg family		Generic Netlink family object
 * @arg name		New human readable name
 */
void genl_family_set_name(struct genl_family *family, const char *name)
{
	strncpy(family->gf_name, name, GENL_NAMSIZ-1);
	family->ce_mask |= FAMILY_ATTR_NAME;
}

/**
 * @name Interface Version
 * @{
 */

/**
 * Return interface version
 * @arg family		Generic Netlink family object
 *
 * @return Interface version or 0 if not available.
 */
uint8_t genl_family_get_version(struct genl_family *family)
{
	if (family->ce_mask & FAMILY_ATTR_VERSION)
		return family->gf_version;
	else
		return 0;
}

/**
 * Set interface version
 * @arg family		Generic Netlink family object
 * @arg version		New interface version
 */
void genl_family_set_version(struct genl_family *family, uint8_t version)
{
	family->gf_version = version;
	family->ce_mask |= FAMILY_ATTR_VERSION;
}

/** @} */

/**
 * @name Header Size
 * @{
 */

/**
 * Return user header size expected by kernel component
 * @arg family		Generic Netlink family object
 *
 * @return Expected header length or 0 if not available.
 */
uint32_t genl_family_get_hdrsize(struct genl_family *family)
{
	if (family->ce_mask & FAMILY_ATTR_HDRSIZE)
		return family->gf_hdrsize;
	else
		return 0;
}

void genl_family_set_hdrsize(struct genl_family *family, uint32_t hdrsize)
{
	family->gf_hdrsize = hdrsize;
	family->ce_mask |= FAMILY_ATTR_HDRSIZE;
}

/** @} */

/**
 * @name Maximum Expected Attribute
 * @{
 */

uint32_t genl_family_get_maxattr(struct genl_family *family)
{
	if (family->ce_mask & FAMILY_ATTR_MAXATTR)
		return family->gf_maxattr;
	else
		return family->gf_maxattr;
}

void genl_family_set_maxattr(struct genl_family *family, uint32_t maxattr)
{
	family->gf_maxattr = maxattr;
	family->ce_mask |= FAMILY_ATTR_MAXATTR;
}

/** @} */

/**
 * @name Operations
 * @{
 */

int genl_family_add_op(struct genl_family *family, int id, int flags)
{
	struct genl_family_op *op;

	op = calloc(1, sizeof(*op));
	if (op == NULL)
		return -NLE_NOMEM;

	op->o_id = id;
	op->o_flags = flags;

	nl_list_add_tail(&op->o_list, &family->gf_ops);
	family->ce_mask |= FAMILY_ATTR_OPS;

	return 0;
}

int genl_family_add_grp(struct genl_family *family, uint32_t id,
	       		const char *name)
{
	struct genl_family_grp *grp;  

	grp = calloc(1, sizeof(*grp));
	if (grp == NULL)
		return -NLE_NOMEM;

	grp->id = id;
	strncpy(grp->name, name, GENL_NAMSIZ - 1);

	nl_list_add_tail(&grp->list, &family->gf_mc_grps);

	return 0;
}

/** @} */

/** @cond SKIP */
struct nl_object_ops genl_family_ops = {
	.oo_name		= "genl/family",
	.oo_size		= sizeof(struct genl_family),
	.oo_constructor		= family_constructor,
	.oo_free_data		= family_free_data,
	.oo_clone		= family_clone,
	.oo_dump = {
	    [NL_DUMP_LINE]	= family_dump_line,
	    [NL_DUMP_DETAILS]	= family_dump_details,
	    [NL_DUMP_STATS]	= family_dump_stats,
	},
	.oo_compare		= family_compare,
	.oo_id_attrs		= FAMILY_ATTR_ID,
};
/** @endcond */

/** @} */
