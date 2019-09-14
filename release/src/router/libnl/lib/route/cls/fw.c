/*
 * lib/route/cls/fw.c		fw classifier
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2013 Thomas Graf <tgraf@suug.ch>
 * Copyright (c) 2006 Petr Gotthard <petr.gotthard@siemens.com>
 * Copyright (c) 2006 Siemens AG Oesterreich
 */

/**
 * @ingroup cls
 * @defgroup cls_fw Firewall Classifier
 *
 * @{
 */

#include <netlink-private/netlink.h>
#include <netlink-private/tc.h>
#include <netlink/netlink.h>
#include <netlink-private/route/tc-api.h>
#include <netlink/route/classifier.h>
#include <netlink/route/cls/fw.h>

/** @cond SKIP */
#define FW_ATTR_CLASSID      0x001
#define FW_ATTR_ACTION       0x002
#define FW_ATTR_POLICE       0x004
#define FW_ATTR_INDEV        0x008
#define FW_ATTR_MASK         0x010
/** @endcond */

static struct nla_policy fw_policy[TCA_FW_MAX+1] = {
	[TCA_FW_CLASSID]	= { .type = NLA_U32 },
	[TCA_FW_INDEV]		= { .type = NLA_STRING,
				    .maxlen = IFNAMSIZ },
	[TCA_FW_MASK]		= { .type = NLA_U32 },
};

static int fw_msg_parser(struct rtnl_tc *tc, void *data)
{
	struct nlattr *tb[TCA_FW_MAX + 1];
	struct rtnl_fw *f = data;
	int err;

	err = tca_parse(tb, TCA_FW_MAX, tc, fw_policy);
	if (err < 0)
		return err;

	if (tb[TCA_FW_CLASSID]) {
		f->cf_classid = nla_get_u32(tb[TCA_FW_CLASSID]);
		f->cf_mask |= FW_ATTR_CLASSID;
	}

	if (tb[TCA_FW_ACT]) {
		f->cf_act = nl_data_alloc_attr(tb[TCA_FW_ACT]);
		if (!f->cf_act)
			return -NLE_NOMEM;
		f->cf_mask |= FW_ATTR_ACTION;
	}

	if (tb[TCA_FW_POLICE]) {
		f->cf_police = nl_data_alloc_attr(tb[TCA_FW_POLICE]);
		if (!f->cf_police)
			return -NLE_NOMEM;
		f->cf_mask |= FW_ATTR_POLICE;
	}

	if (tb[TCA_FW_INDEV]) {
		nla_strlcpy(f->cf_indev, tb[TCA_FW_INDEV], IFNAMSIZ);
		f->cf_mask |= FW_ATTR_INDEV;
	}

	if (tb[TCA_FW_MASK]) {
		f->cf_fwmask = nla_get_u32(tb[TCA_FW_MASK]);
		f->cf_mask |= FW_ATTR_MASK;
	}

	return 0;
}

static void fw_free_data(struct rtnl_tc *tc, void *data)
{
	struct rtnl_fw *f = data;

	nl_data_free(f->cf_act);
	nl_data_free(f->cf_police);
}

static int fw_clone(void *_dst, void *_src)
{
	struct rtnl_fw *dst = _dst, *src = _src;

	if (src->cf_act && !(dst->cf_act = nl_data_clone(src->cf_act)))
		return -NLE_NOMEM;
	
	if (src->cf_police && !(dst->cf_police = nl_data_clone(src->cf_police)))
		return -NLE_NOMEM;

	return 0;
}

static void fw_dump_line(struct rtnl_tc *tc, void *data,
			 struct nl_dump_params *p)
{
	struct rtnl_fw *f = data;

	if (!f)
		return;

	if (f->cf_mask & FW_ATTR_CLASSID) {
		char buf[32];

		nl_dump(p, " target %s",
			rtnl_tc_handle2str(f->cf_classid, buf, sizeof(buf)));
	}

	if (f->cf_mask & FW_ATTR_MASK)
		nl_dump(p, " mask 0x%x", f->cf_fwmask);
}

static void fw_dump_details(struct rtnl_tc *tc, void *data,
			    struct nl_dump_params *p)
{
	struct rtnl_fw *f = data;

	if (f && f->cf_mask & FW_ATTR_INDEV)
		nl_dump(p, "indev %s ", f->cf_indev);
}

static int fw_msg_fill(struct rtnl_tc *tc, void *data, struct nl_msg *msg)
{
	struct rtnl_fw *f = data;

	if (!f)
		return 0;

	if (f->cf_mask & FW_ATTR_CLASSID)
		NLA_PUT_U32(msg, TCA_FW_CLASSID, f->cf_classid);

	if (f->cf_mask & FW_ATTR_ACTION)
		NLA_PUT_DATA(msg, TCA_FW_ACT, f->cf_act);

	if (f->cf_mask & FW_ATTR_POLICE)
		NLA_PUT_DATA(msg, TCA_FW_POLICE, f->cf_police);

	if (f->cf_mask & FW_ATTR_INDEV)
		NLA_PUT_STRING(msg, TCA_FW_INDEV, f->cf_indev);

	if (f->cf_mask & FW_ATTR_MASK)
		NLA_PUT_U32(msg, TCA_FW_MASK, f->cf_fwmask);

	return 0;

nla_put_failure:
	return -NLE_MSGSIZE;
}

/**
 * @name Attribute Modifications
 * @{
 */

int rtnl_fw_set_classid(struct rtnl_cls *cls, uint32_t classid)
{
	struct rtnl_fw *f;

	if (!(f = rtnl_tc_data(TC_CAST(cls))))
		return -NLE_NOMEM;
	
	f->cf_classid = classid;
	f->cf_mask |= FW_ATTR_CLASSID;

	return 0;
}

int rtnl_fw_set_mask(struct rtnl_cls *cls, uint32_t mask)
{
	struct rtnl_fw *f;

	if (!(f = rtnl_tc_data(TC_CAST(cls))))
		return -NLE_NOMEM;
	
	f->cf_fwmask = mask;
	f->cf_mask |= FW_ATTR_MASK;

	return 0;
}

/** @} */

static struct rtnl_tc_ops fw_ops = {
	.to_kind		= "fw",
	.to_type		= RTNL_TC_TYPE_CLS,
	.to_size		= sizeof(struct rtnl_fw),
	.to_msg_parser		= fw_msg_parser,
	.to_msg_fill		= fw_msg_fill,
	.to_free_data		= fw_free_data,
	.to_clone		= fw_clone,
	.to_dump = {
	    [NL_DUMP_LINE]	= fw_dump_line,
	    [NL_DUMP_DETAILS]	= fw_dump_details,
	},
};

static void __init fw_init(void)
{
	rtnl_tc_register(&fw_ops);
}

static void __exit fw_exit(void)
{
	rtnl_tc_unregister(&fw_ops);
}

/** @} */
