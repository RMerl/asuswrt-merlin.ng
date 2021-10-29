/*
 * lib/route/cls/ematch/cmp.c	Simple packet data comparison ematch
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2008-2013 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup ematch
 * @defgroup em_cmp Simple packet data comparison
 *
 * @{
 */

#include <netlink-private/netlink.h>
#include <netlink-private/tc.h>
#include <netlink/netlink.h>
#include <netlink/route/cls/ematch.h>
#include <linux/tc_ematch/tc_em_cmp.h>

void rtnl_ematch_cmp_set(struct rtnl_ematch *e, struct tcf_em_cmp *cfg)
{
	memcpy(rtnl_ematch_data(e), cfg, sizeof(*cfg));
}

struct tcf_em_cmp *rtnl_ematch_cmp_get(struct rtnl_ematch *e)
{
	return rtnl_ematch_data(e);
}

static int cmp_parse(struct rtnl_ematch *e, void *data, size_t len)
{
	memcpy(rtnl_ematch_data(e), data, len);

	return 0;
}

static const char *align_txt[] = {
	[TCF_EM_ALIGN_U8] = "u8",
	[TCF_EM_ALIGN_U16] = "u16",
	[TCF_EM_ALIGN_U32] = "u32"
};

static const char *layer_txt[] = {
	[TCF_LAYER_LINK] = "eth",
	[TCF_LAYER_NETWORK] = "ip",
	[TCF_LAYER_TRANSPORT] = "tcp"
};

static const char *operand_txt[] = {
	[TCF_EM_OPND_EQ] = "=",
	[TCF_EM_OPND_LT] = "<",
	[TCF_EM_OPND_GT] = ">",
};

static void cmp_dump(struct rtnl_ematch *e, struct nl_dump_params *p)
{
	struct tcf_em_cmp *cmp = rtnl_ematch_data(e);

	if (cmp->flags & TCF_EM_CMP_TRANS)
		nl_dump(p, "ntoh%c(", (cmp->align == TCF_EM_ALIGN_U32) ? 'l' : 's');

	nl_dump(p, "%s at %s+%u",
		align_txt[cmp->align], layer_txt[cmp->layer], cmp->off);

	if (cmp->mask)
		nl_dump(p, " & 0x%x", cmp->mask);

	if (cmp->flags & TCF_EM_CMP_TRANS)
		nl_dump(p, ")");

	nl_dump(p, " %s %u", operand_txt[cmp->opnd], cmp->val);
}

static struct rtnl_ematch_ops cmp_ops = {
	.eo_kind	= TCF_EM_CMP,
	.eo_name	= "cmp",
	.eo_minlen	= sizeof(struct tcf_em_cmp),
	.eo_datalen	= sizeof(struct tcf_em_cmp),
	.eo_parse	= cmp_parse,
	.eo_dump	= cmp_dump,
};

static void __init cmp_init(void)
{
	rtnl_ematch_register(&cmp_ops);
}

/** @} */
