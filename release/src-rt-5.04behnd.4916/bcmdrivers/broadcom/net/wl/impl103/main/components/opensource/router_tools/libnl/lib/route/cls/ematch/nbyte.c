/*
 * lib/route/cls/ematch/nbyte.c		Nbyte comparison
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2010-2013 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup ematch
 * @defgroup em_nbyte N-Byte Comparison
 *
 * @{
 */

#include <netlink-private/netlink.h>
#include <netlink-private/tc.h>
#include <netlink/netlink.h>
#include <netlink/route/cls/ematch.h>
#include <netlink/route/cls/ematch/nbyte.h>

struct nbyte_data
{
	struct tcf_em_nbyte	cfg;
	uint8_t *		pattern;
};

void rtnl_ematch_nbyte_set_offset(struct rtnl_ematch *e, uint8_t layer,
				  uint16_t offset)
{
	struct nbyte_data *n = rtnl_ematch_data(e);
	n->cfg.off = offset;
	n->cfg.layer = layer;
}

uint16_t rtnl_ematch_nbyte_get_offset(struct rtnl_ematch *e)
{
	return ((struct nbyte_data *) rtnl_ematch_data(e))->cfg.off;
}

uint8_t rtnl_ematch_nbyte_get_layer(struct rtnl_ematch *e)
{
	return ((struct nbyte_data *) rtnl_ematch_data(e))->cfg.layer;
}

void rtnl_ematch_nbyte_set_pattern(struct rtnl_ematch *e,
				   uint8_t *pattern, size_t len)
{
	struct nbyte_data *n = rtnl_ematch_data(e);

	if (n->pattern)
		free(n->pattern);

	n->pattern = pattern;
	n->cfg.len = len;
}

uint8_t *rtnl_ematch_nbyte_get_pattern(struct rtnl_ematch *e)
{
	return ((struct nbyte_data *) rtnl_ematch_data(e))->pattern;
}

size_t rtnl_ematch_nbyte_get_len(struct rtnl_ematch *e)
{
	return ((struct nbyte_data *) rtnl_ematch_data(e))->cfg.len;
}

static const char *layer_txt(struct tcf_em_nbyte *nbyte)
{
	switch (nbyte->layer) {
	case TCF_LAYER_LINK:
		return "link";
	case TCF_LAYER_NETWORK:
		return "net";
	case TCF_LAYER_TRANSPORT:
		return "trans";
	default:
		return "?";
	}
}

static int nbyte_parse(struct rtnl_ematch *e, void *data, size_t len)
{
	struct nbyte_data *n = rtnl_ematch_data(e);
	size_t hdrlen = sizeof(struct tcf_em_nbyte);
	size_t plen = len - hdrlen;

	memcpy(&n->cfg, data, hdrlen);
	if (plen > 0) {
		if (!(n->pattern = calloc(1, plen)))
			return -NLE_NOMEM;

		memcpy(n->pattern, data + hdrlen, plen);
	}

	return 0;
}

static void nbyte_dump(struct rtnl_ematch *e, struct nl_dump_params *p)
{
	struct nbyte_data *n = rtnl_ematch_data(e);
	int i;

	nl_dump(p, "pattern(%u:[", n->cfg.len);

	for (i = 0; i < n->cfg.len; i++) {
		nl_dump(p, "%02x", n->pattern[i]);
		if (i+1 < n->cfg.len)
			nl_dump(p, " ");
	}

	nl_dump(p, "] at %s+%u)", layer_txt(&n->cfg), n->cfg.off);
}

static void nbyte_free(struct rtnl_ematch *e)
{
	struct nbyte_data *n = rtnl_ematch_data(e);
	free(n->pattern);
}

static struct rtnl_ematch_ops nbyte_ops = {
	.eo_kind	= TCF_EM_NBYTE,
	.eo_name	= "nbyte",
	.eo_minlen	= sizeof(struct tcf_em_nbyte),
	.eo_datalen	= sizeof(struct nbyte_data),
	.eo_parse	= nbyte_parse,
	.eo_dump	= nbyte_dump,
	.eo_free	= nbyte_free,
};

static void __init nbyte_init(void)
{
	rtnl_ematch_register(&nbyte_ops);
}

/** @} */
