/*
 * lib/route/qdisc/dsmark.c	DSMARK
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2011 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup qdisc
 * @ingroup class
 * @defgroup qdisc_dsmark Differentiated Services Marker (DSMARK)
 * @{
 */

#include <netlink-private/netlink.h>
#include <netlink-private/tc.h>
#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <netlink/route/qdisc.h>
#include <netlink-private/route/tc-api.h>
#include <netlink/route/class.h>
#include <netlink/route/qdisc/dsmark.h>

/** @cond SKIP */
#define SCH_DSMARK_ATTR_INDICES		0x1
#define SCH_DSMARK_ATTR_DEFAULT_INDEX	0x2
#define SCH_DSMARK_ATTR_SET_TC_INDEX	0x4

#define SCH_DSMARK_ATTR_MASK		0x1
#define SCH_DSMARK_ATTR_VALUE		0x2
/** @endcond */

static struct nla_policy dsmark_policy[TCA_DSMARK_MAX+1] = {
	[TCA_DSMARK_INDICES]		= { .type = NLA_U16 },
	[TCA_DSMARK_DEFAULT_INDEX]	= { .type = NLA_U16 },
	[TCA_DSMARK_SET_TC_INDEX]	= { .type = NLA_FLAG },
	[TCA_DSMARK_VALUE]		= { .type = NLA_U8 },
	[TCA_DSMARK_MASK]		= { .type = NLA_U8 },
};

static int dsmark_qdisc_msg_parser(struct rtnl_tc *tc, void *data)
{
	struct rtnl_dsmark_qdisc *dsmark = data;
	struct nlattr *tb[TCA_DSMARK_MAX + 1];
	int err;

	err = tca_parse(tb, TCA_DSMARK_MAX, tc, dsmark_policy);
	if (err < 0)
		return err;

	if (tb[TCA_DSMARK_INDICES]) {
		dsmark->qdm_indices = nla_get_u16(tb[TCA_DSMARK_INDICES]);
		dsmark->qdm_mask |= SCH_DSMARK_ATTR_INDICES;
	}

	if (tb[TCA_DSMARK_DEFAULT_INDEX]) {
		dsmark->qdm_default_index =
				nla_get_u16(tb[TCA_DSMARK_DEFAULT_INDEX]);
		dsmark->qdm_mask |= SCH_DSMARK_ATTR_DEFAULT_INDEX;
	}

	if (tb[TCA_DSMARK_SET_TC_INDEX]) {
		dsmark->qdm_set_tc_index = 1;
		dsmark->qdm_mask |= SCH_DSMARK_ATTR_SET_TC_INDEX;
	}

	return 0;
}

static int dsmark_class_msg_parser(struct rtnl_tc *tc, void *data)
{
	struct rtnl_dsmark_class *dsmark = data;
	struct nlattr *tb[TCA_DSMARK_MAX + 1];
	int err;

	err = tca_parse(tb, TCA_DSMARK_MAX, tc, dsmark_policy);
	if (err < 0)
		return err;

	if (tb[TCA_DSMARK_MASK]) {
		dsmark->cdm_bmask = nla_get_u8(tb[TCA_DSMARK_MASK]);
		dsmark->cdm_mask |= SCH_DSMARK_ATTR_MASK;
	}

	if (tb[TCA_DSMARK_VALUE]) {
		dsmark->cdm_value = nla_get_u8(tb[TCA_DSMARK_VALUE]);
		dsmark->cdm_mask |= SCH_DSMARK_ATTR_VALUE;
	}

	return 0;
}

static void dsmark_qdisc_dump_line(struct rtnl_tc *tc, void *data,
				   struct nl_dump_params *p)
{
	struct rtnl_dsmark_qdisc *dsmark = data;

	if (dsmark && (dsmark->qdm_mask & SCH_DSMARK_ATTR_INDICES))
		nl_dump(p, " indices 0x%04x", dsmark->qdm_indices);
}

static void dsmark_qdisc_dump_details(struct rtnl_tc *tc, void *data,
				      struct nl_dump_params *p)
{
	struct rtnl_dsmark_qdisc *dsmark = data;

	if (!dsmark)
		return;

	if (dsmark->qdm_mask & SCH_DSMARK_ATTR_DEFAULT_INDEX)
		nl_dump(p, " default index 0x%04x", dsmark->qdm_default_index);

	if (dsmark->qdm_mask & SCH_DSMARK_ATTR_SET_TC_INDEX)
		nl_dump(p, " set-tc-index");
}

static void dsmark_class_dump_line(struct rtnl_tc *tc, void *data,
				   struct nl_dump_params *p)
{
	struct rtnl_dsmark_class *dsmark = data;

	if (!dsmark)
		return;

	if (dsmark->cdm_mask & SCH_DSMARK_ATTR_VALUE)
		nl_dump(p, " value 0x%02x", dsmark->cdm_value);

	if (dsmark->cdm_mask & SCH_DSMARK_ATTR_MASK)
		nl_dump(p, " mask 0x%02x", dsmark->cdm_bmask);
}

static int dsmark_qdisc_msg_fill(struct rtnl_tc *tc, void *data,
				 struct nl_msg *msg)
{
	struct rtnl_dsmark_qdisc *dsmark = data;

	if (!dsmark)
		return 0;

	if (dsmark->qdm_mask & SCH_DSMARK_ATTR_INDICES)
		NLA_PUT_U16(msg, TCA_DSMARK_INDICES, dsmark->qdm_indices);

	if (dsmark->qdm_mask & SCH_DSMARK_ATTR_DEFAULT_INDEX)
		NLA_PUT_U16(msg, TCA_DSMARK_DEFAULT_INDEX,
			    dsmark->qdm_default_index);

	if (dsmark->qdm_mask & SCH_DSMARK_ATTR_SET_TC_INDEX)
		NLA_PUT_FLAG(msg, TCA_DSMARK_SET_TC_INDEX);

	return 0;

nla_put_failure:
	return -NLE_MSGSIZE;
}

static int dsmark_class_msg_fill(struct rtnl_tc *tc, void *data,
				 struct nl_msg *msg)
{
	struct rtnl_dsmark_class *dsmark = data;

	if (!dsmark)
		return 0;

	if (dsmark->cdm_mask & SCH_DSMARK_ATTR_MASK)
		NLA_PUT_U8(msg, TCA_DSMARK_MASK, dsmark->cdm_bmask);

	if (dsmark->cdm_mask & SCH_DSMARK_ATTR_VALUE)
		NLA_PUT_U8(msg, TCA_DSMARK_VALUE, dsmark->cdm_value);

	return 0;

nla_put_failure:
	return -NLE_MSGSIZE;
}

/**
 * @name Class Attribute Access
 * @{
 */

/**
 * Set bitmask of DSMARK class.
 * @arg class		DSMARK class to be modified.
 * @arg mask		New bitmask.
 * @return 0 on success or a negative error code.
 */
int rtnl_class_dsmark_set_bitmask(struct rtnl_class *class, uint8_t mask)
{
	struct rtnl_dsmark_class *dsmark;
	
	if (!(dsmark = rtnl_tc_data(TC_CAST(class))))
		return -NLE_NOMEM;

	dsmark->cdm_bmask = mask;
	dsmark->cdm_mask |= SCH_DSMARK_ATTR_MASK;

	return 0;
}

/**
 * Get bitmask of DSMARK class.
 * @arg class		DSMARK class.
 * @return Bitmask or a negative error code.
 */
int rtnl_class_dsmark_get_bitmask(struct rtnl_class *class)
{
	struct rtnl_dsmark_class *dsmark;
	
	if (!(dsmark = rtnl_tc_data(TC_CAST(class))))
		return -NLE_NOMEM;

	if (dsmark->cdm_mask & SCH_DSMARK_ATTR_MASK)
		return dsmark->cdm_bmask;
	else
		return -NLE_NOATTR;
}

/**
 * Set value of DSMARK class.
 * @arg class		DSMARK class to be modified.
 * @arg value		New value.
 * @return 0 on success or a negative errror code.
 */
int rtnl_class_dsmark_set_value(struct rtnl_class *class, uint8_t value)
{
	struct rtnl_dsmark_class *dsmark;
	
	if (!(dsmark = rtnl_tc_data(TC_CAST(class))))
		return -NLE_NOMEM;

	dsmark->cdm_value = value;
	dsmark->cdm_mask |= SCH_DSMARK_ATTR_VALUE;

	return 0;
}

/**
 * Get value of DSMARK class.
 * @arg class		DSMARK class.
 * @return Value or a negative error code.
 */
int rtnl_class_dsmark_get_value(struct rtnl_class *class)
{
	struct rtnl_dsmark_class *dsmark;
	
	if (!(dsmark = rtnl_tc_data(TC_CAST(class))))
		return -NLE_NOMEM;

	if (dsmark->cdm_mask & SCH_DSMARK_ATTR_VALUE)
		return dsmark->cdm_value;
	else
		return -NLE_NOATTR;
}

/** @} */

/**
 * @name Qdisc Attribute Access
 * @{
 */

/**
 * Set indices of DSMARK qdisc.
 * @arg qdisc		DSMARK qdisc to be modified.
 * @arg indices		New indices.
 */
int rtnl_qdisc_dsmark_set_indices(struct rtnl_qdisc *qdisc, uint16_t indices)
{
	struct rtnl_dsmark_qdisc *dsmark;

	if (!(dsmark = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	dsmark->qdm_indices = indices;
	dsmark->qdm_mask |= SCH_DSMARK_ATTR_INDICES;

	return 0;
}

/**
 * Get indices of DSMARK qdisc.
 * @arg qdisc		DSMARK qdisc.
 * @return Indices or a negative error code.
 */
int rtnl_qdisc_dsmark_get_indices(struct rtnl_qdisc *qdisc)
{
	struct rtnl_dsmark_qdisc *dsmark;

	if (!(dsmark = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	if (dsmark->qdm_mask & SCH_DSMARK_ATTR_INDICES)
		return dsmark->qdm_indices;
	else
		return -NLE_NOATTR;
}

/**
 * Set default index of DSMARK qdisc.
 * @arg qdisc		DSMARK qdisc to be modified.
 * @arg default_index	New default index.
 * @return 0 on success or a negative error code.
 */
int rtnl_qdisc_dsmark_set_default_index(struct rtnl_qdisc *qdisc,
					uint16_t default_index)
{
	struct rtnl_dsmark_qdisc *dsmark;

	if (!(dsmark = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	dsmark->qdm_default_index = default_index;
	dsmark->qdm_mask |= SCH_DSMARK_ATTR_DEFAULT_INDEX;

	return 0;
}

/**
 * Get default index of DSMARK qdisc.
 * @arg qdisc		DSMARK qdisc.
 * @return Default index or a negative error code.
 */
int rtnl_qdisc_dsmark_get_default_index(struct rtnl_qdisc *qdisc)
{
	struct rtnl_dsmark_qdisc *dsmark;

	if (!(dsmark = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	if (dsmark->qdm_mask & SCH_DSMARK_ATTR_DEFAULT_INDEX)
		return dsmark->qdm_default_index;
	else
		return -NLE_NOATTR;
}

/**
 * Set set-tc-index flag of DSMARK qdisc.
 * @arg qdisc		DSMARK qdisc to be modified.
 * @arg flag		Flag indicating whether to enable or disable.
 * @return 0 on success or a negative error code.
 */
int rtnl_qdisc_dsmark_set_set_tc_index(struct rtnl_qdisc *qdisc, int flag)
{
	struct rtnl_dsmark_qdisc *dsmark;

	if (!(dsmark = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	dsmark->qdm_set_tc_index = !!flag;
	dsmark->qdm_mask |= SCH_DSMARK_ATTR_SET_TC_INDEX;

	return 0;
}

/**
 * Get set-tc-index flag of DSMARK qdisc.
 * @arg qdisc		DSMARK qdisc to be modified.
 * @return 1 or 0 to indicate wehther the flag is enabled or a negative
 *         error code.
 */
int rtnl_qdisc_dsmark_get_set_tc_index(struct rtnl_qdisc *qdisc)
{
	struct rtnl_dsmark_qdisc *dsmark;

	if (!(dsmark = rtnl_tc_data(TC_CAST(qdisc))))
		return -NLE_NOMEM;

	if (dsmark->qdm_mask & SCH_DSMARK_ATTR_SET_TC_INDEX)
		return dsmark->qdm_set_tc_index;
	else
		return -NLE_NOATTR;
}

/** @} */

static struct rtnl_tc_ops dsmark_qdisc_ops = {
	.to_kind		= "dsmark",
	.to_type		= RTNL_TC_TYPE_QDISC,
	.to_size		= sizeof(struct rtnl_dsmark_qdisc),
	.to_msg_parser		= dsmark_qdisc_msg_parser,
	.to_dump = {
	    [NL_DUMP_LINE]	= dsmark_qdisc_dump_line,
	    [NL_DUMP_DETAILS]	= dsmark_qdisc_dump_details,
	},
	.to_msg_fill		= dsmark_qdisc_msg_fill,
};

static struct rtnl_tc_ops dsmark_class_ops = {
	.to_kind		= "dsmark",
	.to_type		= RTNL_TC_TYPE_CLASS,
	.to_size		= sizeof(struct rtnl_dsmark_class),
	.to_msg_parser		= dsmark_class_msg_parser,
	.to_dump[NL_DUMP_LINE]	= dsmark_class_dump_line,
	.to_msg_fill		= dsmark_class_msg_fill,
};

static void __init dsmark_init(void)
{
	rtnl_tc_register(&dsmark_qdisc_ops);
	rtnl_tc_register(&dsmark_class_ops);
}

static void __exit dsmark_exit(void)
{
	rtnl_tc_unregister(&dsmark_qdisc_ops);
	rtnl_tc_unregister(&dsmark_class_ops);
}

/** @} */
