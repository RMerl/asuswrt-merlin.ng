/*
 * lib/route/cls/ematch/text.c		Text Search
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
 * @defgroup em_text Text Search
 *
 * @{
 */

#include <netlink-private/netlink.h>
#include <netlink-private/tc.h>
#include <netlink/netlink.h>
#include <netlink/route/cls/ematch.h>
#include <netlink/route/cls/ematch/text.h>

struct text_data
{
	struct tcf_em_text	cfg;
	char *			pattern;
};

void rtnl_ematch_text_set_from(struct rtnl_ematch *e, uint8_t layer,
			       uint16_t offset)
{
	struct text_data *t = rtnl_ematch_data(e);
	t->cfg.from_offset = offset;
	t->cfg.from_layer = layer;
}

uint16_t rtnl_ematch_text_get_from_offset(struct rtnl_ematch *e)
{
	return ((struct text_data *) rtnl_ematch_data(e))->cfg.from_offset;
}

uint8_t rtnl_ematch_text_get_from_layer(struct rtnl_ematch *e)
{
	return ((struct text_data *) rtnl_ematch_data(e))->cfg.from_layer;
}

void rtnl_ematch_text_set_to(struct rtnl_ematch *e, uint8_t layer,
			       uint16_t offset)
{
	struct text_data *t = rtnl_ematch_data(e);
	t->cfg.to_offset = offset;
	t->cfg.to_layer = layer;
}

uint16_t rtnl_ematch_text_get_to_offset(struct rtnl_ematch *e)
{
	return ((struct text_data *) rtnl_ematch_data(e))->cfg.to_offset;
}

uint8_t rtnl_ematch_text_get_to_layer(struct rtnl_ematch *e)
{
	return ((struct text_data *) rtnl_ematch_data(e))->cfg.to_layer;
}

void rtnl_ematch_text_set_pattern(struct rtnl_ematch *e,
				  char *pattern, size_t len)
{
	struct text_data *t = rtnl_ematch_data(e);

	if (t->pattern)
		free(t->pattern);

	t->pattern = pattern;
	t->cfg.pattern_len = len;
}

char *rtnl_ematch_text_get_pattern(struct rtnl_ematch *e)
{
	return ((struct text_data *) rtnl_ematch_data(e))->pattern;
}

size_t rtnl_ematch_text_get_len(struct rtnl_ematch *e)
{
	return ((struct text_data *) rtnl_ematch_data(e))->cfg.pattern_len;
}

void rtnl_ematch_text_set_algo(struct rtnl_ematch *e, const char *algo)
{
	struct text_data *t = rtnl_ematch_data(e);

	strncpy(t->cfg.algo, algo, sizeof(t->cfg.algo));
}

char *rtnl_ematch_text_get_algo(struct rtnl_ematch *e)
{
	struct text_data *t = rtnl_ematch_data(e);

	return t->cfg.algo[0] ? t->cfg.algo : NULL;
}

static int text_parse(struct rtnl_ematch *e, void *data, size_t len)
{
	struct text_data *t = rtnl_ematch_data(e);
	size_t hdrlen = sizeof(struct tcf_em_text);
	size_t plen = len - hdrlen;

	memcpy(&t->cfg, data, hdrlen);

	if (t->cfg.pattern_len > plen)
		return -NLE_INVAL;

	if (t->cfg.pattern_len > 0) {
		if (!(t->pattern = calloc(1, t->cfg.pattern_len)))
			return -NLE_NOMEM;

		memcpy(t->pattern, data + hdrlen, t->cfg.pattern_len);
	}

	return 0;
}

static void text_dump(struct rtnl_ematch *e, struct nl_dump_params *p)
{
	struct text_data *t = rtnl_ematch_data(e);
	char buf[64];

	nl_dump(p, "text(%s \"%s\"",
		t->cfg.algo[0] ? t->cfg.algo : "no-algo",
		t->pattern ? : "no-pattern");

	if (t->cfg.from_layer || t->cfg.from_offset) {
		nl_dump(p, " from %s",
			rtnl_ematch_offset2txt(t->cfg.from_layer,
					       t->cfg.from_offset,
					       buf, sizeof(buf)));
	}

	if (t->cfg.to_layer || t->cfg.to_offset) {
		nl_dump(p, " to %s",
			rtnl_ematch_offset2txt(t->cfg.to_layer,
					       t->cfg.to_offset,
					       buf, sizeof(buf)));
	}

	nl_dump(p, ")");
}

static int text_fill(struct rtnl_ematch *e, struct nl_msg *msg)
{
	struct text_data *t = rtnl_ematch_data(e);
	int err;

	if ((err = nlmsg_append(msg, &t->cfg, sizeof(t->cfg), 0)) < 0)
		return err;

	return nlmsg_append(msg, t->pattern, t->cfg.pattern_len, 0);
}

static void text_free(struct rtnl_ematch *e)
{
	struct text_data *t = rtnl_ematch_data(e);
	free(t->pattern);
}

static struct rtnl_ematch_ops text_ops = {
	.eo_kind	= TCF_EM_TEXT,
	.eo_name	= "text",
	.eo_minlen	= sizeof(struct tcf_em_text),
	.eo_datalen	= sizeof(struct text_data),
	.eo_parse	= text_parse,
	.eo_dump	= text_dump,
	.eo_fill	= text_fill,
	.eo_free	= text_free,
};

static void __init text_init(void)
{
	rtnl_ematch_register(&text_ops);
}

/** @} */
