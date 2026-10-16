/*
 * Shared library add-on to iptables to add early socket matching support.
 *
 * Copyright (C) 2007 BalaBit IT Ltd.
 */
#include <stdio.h>
#include <xtables.h>
#include <linux/netfilter/xt_socket.h>

enum {
	O_TRANSPARENT = 0,
	O_NOWILDCARD = 1,
	O_RESTORESKMARK = 2,
};

static const struct xt_option_entry socket_mt_opts[] = {
	{.name = "transparent", .id = O_TRANSPARENT, .type = XTTYPE_NONE},
	XTOPT_TABLEEND,
};

static const struct xt_option_entry socket_mt_opts_v2[] = {
	{.name = "transparent", .id = O_TRANSPARENT, .type = XTTYPE_NONE},
	{.name = "nowildcard", .id = O_NOWILDCARD, .type = XTTYPE_NONE},
	XTOPT_TABLEEND,
};

static const struct xt_option_entry socket_mt_opts_v3[] = {
	{.name = "transparent", .id = O_TRANSPARENT, .type = XTTYPE_NONE},
	{.name = "nowildcard", .id = O_NOWILDCARD, .type = XTTYPE_NONE},
	{.name = "restore-skmark", .id = O_RESTORESKMARK, .type = XTTYPE_NONE},
	XTOPT_TABLEEND,
};

static void socket_mt_help(void)
{
	printf(
		"socket match options:\n"
		"  --transparent    Ignore non-transparent sockets\n\n");
}

static void socket_mt_help_v2(void)
{
	printf(
		"socket match options:\n"
		"  --nowildcard     Do not ignore LISTEN sockets bound on INADDR_ANY\n"
		"  --transparent    Ignore non-transparent sockets\n\n");
}

static void socket_mt_help_v3(void)
{
	printf(
		"socket match options:\n"
		"  --nowildcard     Do not ignore LISTEN sockets bound on INADDR_ANY\n"
		"  --transparent    Ignore non-transparent sockets\n"
		"  --restore-skmark Set the packet mark to the socket mark if\n"
		"                   the socket matches and transparent / \n"
		"                   nowildcard conditions are satisfied\n\n");
}

static void socket_mt_parse(struct xt_option_call *cb)
{
	struct xt_socket_mtinfo1 *info = cb->data;

	xtables_option_parse(cb);
	switch (cb->entry->id) {
	case O_TRANSPARENT:
		info->flags |= XT_SOCKET_TRANSPARENT;
		break;
	}
}

static void socket_mt_parse_v2(struct xt_option_call *cb)
{
	struct xt_socket_mtinfo2 *info = cb->data;

	xtables_option_parse(cb);
	switch (cb->entry->id) {
	case O_TRANSPARENT:
		info->flags |= XT_SOCKET_TRANSPARENT;
		break;
	case O_NOWILDCARD:
		info->flags |= XT_SOCKET_NOWILDCARD;
		break;
	}
}

static void socket_mt_parse_v3(struct xt_option_call *cb)
{
	struct xt_socket_mtinfo2 *info = cb->data;

	xtables_option_parse(cb);
	switch (cb->entry->id) {
	case O_TRANSPARENT:
		info->flags |= XT_SOCKET_TRANSPARENT;
		break;
	case O_NOWILDCARD:
		info->flags |= XT_SOCKET_NOWILDCARD;
		break;
	case O_RESTORESKMARK:
		info->flags |= XT_SOCKET_RESTORESKMARK;
		break;
	}
}

static void
socket_mt_save(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_socket_mtinfo1 *info = (const void *)match->data;

	if (info->flags & XT_SOCKET_TRANSPARENT)
		printf(" --transparent");
}

static void
socket_mt_print(const void *ip, const struct xt_entry_match *match,
		int numeric)
{
	printf(" socket");
	socket_mt_save(ip, match);
}

static void
socket_mt_save_v2(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_socket_mtinfo2 *info = (const void *)match->data;

	if (info->flags & XT_SOCKET_TRANSPARENT)
		printf(" --transparent");
	if (info->flags & XT_SOCKET_NOWILDCARD)
		printf(" --nowildcard");
}

static void
socket_mt_print_v2(const void *ip, const struct xt_entry_match *match,
		   int numeric)
{
	printf(" socket");
	socket_mt_save_v2(ip, match);
}

static void
socket_mt_save_v3(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_socket_mtinfo3 *info = (const void *)match->data;

	if (info->flags & XT_SOCKET_TRANSPARENT)
		printf(" --transparent");
	if (info->flags & XT_SOCKET_NOWILDCARD)
		printf(" --nowildcard");
	if (info->flags & XT_SOCKET_RESTORESKMARK)
		printf(" --restore-skmark");
}

static void
socket_mt_print_v3(const void *ip, const struct xt_entry_match *match,
		   int numeric)
{
	printf(" socket");
	socket_mt_save_v3(ip, match);
}

static int socket_mt_xlate(struct xt_xlate *xl, const struct xt_xlate_mt_params *params)
{
	const struct xt_socket_mtinfo3 *info = (const void *)params->match->data;

	/* ONLY --nowildcard: match if socket exists. It does not matter
	 * to which address it is bound.
	 */
	if (info->flags == XT_SOCKET_NOWILDCARD) {
		xt_xlate_add(xl, "socket wildcard le 1");
		return 1;
	}

	/* Without --nowildcard, restrict to sockets NOT bound to
	 * the any address.
	 */
	if ((info->flags & XT_SOCKET_NOWILDCARD) == 0)
		xt_xlate_add(xl, "socket wildcard 0");

	if (info->flags & XT_SOCKET_TRANSPARENT)
		xt_xlate_add(xl, "socket transparent 1");

	/* If --nowildcard was given, -m socket should not test
	 * the bound address.  We can simply ignore this; its
	 * equal to "wildcard <= 1".
	 */
	if (info->flags & XT_SOCKET_RESTORESKMARK)
		xt_xlate_add(xl, "meta mark set socket mark");

	return 1;
}

static struct xtables_match socket_mt_reg[] = {
	{
		.name          = "socket",
		.revision      = 0,
		.family        = NFPROTO_IPV4,
		.version       = XTABLES_VERSION,
		.size          = XT_ALIGN(0),
		.userspacesize = XT_ALIGN(0),
	},
	{
		.name          = "socket",
		.revision      = 1,
		.family        = NFPROTO_UNSPEC,
		.version       = XTABLES_VERSION,
		.size          = XT_ALIGN(sizeof(struct xt_socket_mtinfo1)),
		.userspacesize = XT_ALIGN(sizeof(struct xt_socket_mtinfo1)),
		.help          = socket_mt_help,
		.print         = socket_mt_print,
		.save          = socket_mt_save,
		.x6_parse      = socket_mt_parse,
		.x6_options    = socket_mt_opts,
		.xlate	       = socket_mt_xlate,
	},
	{
		.name          = "socket",
		.revision      = 2,
		.family        = NFPROTO_UNSPEC,
		.version       = XTABLES_VERSION,
		.size          = XT_ALIGN(sizeof(struct xt_socket_mtinfo2)),
		.userspacesize = XT_ALIGN(sizeof(struct xt_socket_mtinfo2)),
		.help          = socket_mt_help_v2,
		.print         = socket_mt_print_v2,
		.save          = socket_mt_save_v2,
		.x6_parse      = socket_mt_parse_v2,
		.x6_options    = socket_mt_opts_v2,
		.xlate	       = socket_mt_xlate,
	},
	{
		.name          = "socket",
		.revision      = 3,
		.family        = NFPROTO_UNSPEC,
		.version       = XTABLES_VERSION,
		.size          = XT_ALIGN(sizeof(struct xt_socket_mtinfo2)),
		.userspacesize = XT_ALIGN(sizeof(struct xt_socket_mtinfo2)),
		.help          = socket_mt_help_v3,
		.print         = socket_mt_print_v3,
		.save          = socket_mt_save_v3,
		.x6_parse      = socket_mt_parse_v3,
		.x6_options    = socket_mt_opts_v3,
		.xlate	       = socket_mt_xlate,
	},
};

void _init(void)
{
	xtables_register_matches(socket_mt_reg, ARRAY_SIZE(socket_mt_reg));
}
