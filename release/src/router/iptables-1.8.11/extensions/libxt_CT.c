/*
 * Copyright (c) 2010-2013 Patrick McHardy <kaber@trash.net>
 */

#include <stdio.h>
#include <string.h>
#include <xtables.h>
#include <linux/netfilter/nf_conntrack_common.h>
#include <linux/netfilter/xt_CT.h>

static void ct_help(void)
{
	printf(
"CT target options:\n"
" --notrack			Don't track connection\n"
" --helper name			Use conntrack helper 'name' for connection\n"
" --ctevents event[,event...]	Generate specified conntrack events for connection\n"
" --expevents event[,event...]	Generate specified expectation events for connection\n"
" --zone {ID|mark}		Assign/Lookup connection in zone ID/packet nfmark\n"
" --zone-orig {ID|mark}		Same as 'zone' option, but only applies to ORIGINAL direction\n"
" --zone-reply {ID|mark} 	Same as 'zone' option, but only applies to REPLY direction\n"
	);
}

static void ct_help_v1(void)
{
	printf(
"CT target options:\n"
" --notrack			Don't track connection\n"
" --helper name			Use conntrack helper 'name' for connection\n"
" --timeout name 		Use timeout policy 'name' for connection\n"
" --ctevents event[,event...]	Generate specified conntrack events for connection\n"
" --expevents event[,event...]	Generate specified expectation events for connection\n"
" --zone {ID|mark}		Assign/Lookup connection in zone ID/packet nfmark\n"
" --zone-orig {ID|mark}		Same as 'zone' option, but only applies to ORIGINAL direction\n"
" --zone-reply {ID|mark} 	Same as 'zone' option, but only applies to REPLY direction\n"
	);
}

enum {
	O_NOTRACK = 0,
	O_HELPER,
	O_TIMEOUT,
	O_CTEVENTS,
	O_EXPEVENTS,
	O_ZONE,
	O_ZONE_ORIG,
	O_ZONE_REPLY,
};

#define s struct xt_ct_target_info
static const struct xt_option_entry ct_opts[] = {
	{.name = "notrack", .id = O_NOTRACK, .type = XTTYPE_NONE},
	{.name = "helper", .id = O_HELPER, .type = XTTYPE_STRING,
	 .flags = XTOPT_PUT, XTOPT_POINTER(s, helper)},
	{.name = "ctevents", .id = O_CTEVENTS, .type = XTTYPE_STRING},
	{.name = "expevents", .id = O_EXPEVENTS, .type = XTTYPE_STRING},
	{.name = "zone-orig", .id = O_ZONE_ORIG, .type = XTTYPE_STRING},
	{.name = "zone-reply", .id = O_ZONE_REPLY, .type = XTTYPE_STRING},
	{.name = "zone", .id = O_ZONE, .type = XTTYPE_STRING},
	XTOPT_TABLEEND,
};
#undef s

#define s struct xt_ct_target_info_v1
static const struct xt_option_entry ct_opts_v1[] = {
	{.name = "notrack", .id = O_NOTRACK, .type = XTTYPE_NONE},
	{.name = "helper", .id = O_HELPER, .type = XTTYPE_STRING,
	 .flags = XTOPT_PUT, XTOPT_POINTER(s, helper)},
	{.name = "timeout", .id = O_TIMEOUT, .type = XTTYPE_STRING,
	 .flags = XTOPT_PUT, XTOPT_POINTER(s, timeout)},
	{.name = "ctevents", .id = O_CTEVENTS, .type = XTTYPE_STRING},
	{.name = "expevents", .id = O_EXPEVENTS, .type = XTTYPE_STRING},
	{.name = "zone-orig", .id = O_ZONE_ORIG, .type = XTTYPE_STRING},
	{.name = "zone-reply", .id = O_ZONE_REPLY, .type = XTTYPE_STRING},
	{.name = "zone", .id = O_ZONE, .type = XTTYPE_STRING},
	XTOPT_TABLEEND,
};
#undef s

struct event_tbl {
	const char	*name;
	unsigned int	event;
};

static const struct event_tbl ct_event_tbl[] = {
	{ "new",		IPCT_NEW },
	{ "related",		IPCT_RELATED },
	{ "destroy",		IPCT_DESTROY },
	{ "reply",		IPCT_REPLY },
	{ "assured",		IPCT_ASSURED },
	{ "protoinfo",		IPCT_PROTOINFO },
	{ "helper",		IPCT_HELPER },
	{ "mark",		IPCT_MARK },
	{ "natseqinfo",		IPCT_NATSEQADJ },
	{ "secmark",		IPCT_SECMARK },
};

static const struct event_tbl exp_event_tbl[] = {
	{ "new",		IPEXP_NEW },
};

static void ct_parse_zone_id(const char *opt, unsigned int opt_id,
			     uint16_t *zone_id, uint16_t *flags)
{
	if (opt_id == O_ZONE_ORIG)
		*flags |= XT_CT_ZONE_DIR_ORIG;
	if (opt_id == O_ZONE_REPLY)
		*flags |= XT_CT_ZONE_DIR_REPL;

	*zone_id = 0;

	if (strcasecmp(opt, "mark") == 0) {
		*flags |= XT_CT_ZONE_MARK;
	} else {
		uintmax_t val;

		if (!xtables_strtoul(opt, NULL, &val, 0, UINT16_MAX))
			xtables_error(PARAMETER_PROBLEM,
				      "Cannot parse %s as a zone ID", opt);

		*zone_id = (uint16_t)val;
	}
}

static void ct_print_zone_id(const char *pfx, uint16_t zone_id, uint16_t flags)
{
	printf(" %s", pfx);

	if ((flags & (XT_CT_ZONE_DIR_ORIG |
		      XT_CT_ZONE_DIR_REPL)) == XT_CT_ZONE_DIR_ORIG)
		printf("-orig");
	if ((flags & (XT_CT_ZONE_DIR_ORIG |
		      XT_CT_ZONE_DIR_REPL)) == XT_CT_ZONE_DIR_REPL)
		printf("-reply");
	if (flags & XT_CT_ZONE_MARK)
		printf(" mark");
	else
		printf(" %u", zone_id);
}

static uint32_t ct_parse_events(const struct event_tbl *tbl, unsigned int size,
				const char *events)
{
	char str[strlen(events) + 1], *e = str, *t;
	unsigned int mask = 0, i;

	strcpy(str, events);
	while ((t = strsep(&e, ","))) {
		for (i = 0; i < size; i++) {
			if (strcmp(t, tbl[i].name))
				continue;
			mask |= 1 << tbl[i].event;
			break;
		}

		if (i == size)
			xtables_error(PARAMETER_PROBLEM, "Unknown event type \"%s\"", t);
	}

	return mask;
}

static void ct_print_events(const char *pfx, const struct event_tbl *tbl,
			    unsigned int size, uint32_t mask)
{
	const char *sep = "";
	unsigned int i;

	printf(" %s ", pfx);
	for (i = 0; i < size; i++) {
		if (mask & (1 << tbl[i].event)) {
			printf("%s%s", sep, tbl[i].name);
			sep = ",";
		}
	}
}

static void ct_parse(struct xt_option_call *cb)
{
	struct xt_ct_target_info *info = cb->data;

	xtables_option_parse(cb);
	switch (cb->entry->id) {
	case O_NOTRACK:
		info->flags |= XT_CT_NOTRACK;
		break;
	case O_ZONE_ORIG:
	case O_ZONE_REPLY:
	case O_ZONE:
		ct_parse_zone_id(cb->arg, cb->entry->id, &info->zone,
				 &info->flags);
		break;
	case O_CTEVENTS:
		info->ct_events = ct_parse_events(ct_event_tbl, ARRAY_SIZE(ct_event_tbl), cb->arg);
		break;
	case O_EXPEVENTS:
		info->exp_events = ct_parse_events(exp_event_tbl, ARRAY_SIZE(exp_event_tbl), cb->arg);
		break;
	}
}

static void ct_parse_v1(struct xt_option_call *cb)
{
	struct xt_ct_target_info_v1 *info = cb->data;

	xtables_option_parse(cb);
	switch (cb->entry->id) {
	case O_NOTRACK:
		info->flags |= XT_CT_NOTRACK;
		break;
	case O_ZONE_ORIG:
	case O_ZONE_REPLY:
	case O_ZONE:
		ct_parse_zone_id(cb->arg, cb->entry->id, &info->zone,
				 &info->flags);
		break;
	case O_CTEVENTS:
		info->ct_events = ct_parse_events(ct_event_tbl,
						  ARRAY_SIZE(ct_event_tbl),
						  cb->arg);
		break;
	case O_EXPEVENTS:
		info->exp_events = ct_parse_events(exp_event_tbl,
						   ARRAY_SIZE(exp_event_tbl),
						   cb->arg);
		break;
	}
}

static void ct_print(const void *ip, const struct xt_entry_target *target, int numeric)
{
	const struct xt_ct_target_info *info =
		(const struct xt_ct_target_info *)target->data;

	printf(" CT");
	if (info->flags & XT_CT_NOTRACK)
		printf(" notrack");
	if (info->helper[0])
		printf(" helper %s", info->helper);
	if (info->ct_events)
		ct_print_events("ctevents", ct_event_tbl,
				ARRAY_SIZE(ct_event_tbl), info->ct_events);
	if (info->exp_events)
		ct_print_events("expevents", exp_event_tbl,
				ARRAY_SIZE(exp_event_tbl), info->exp_events);
	if (info->flags & XT_CT_ZONE_MARK || info->zone)
		ct_print_zone_id("zone", info->zone, info->flags);
}

static void
ct_print_v1(const void *ip, const struct xt_entry_target *target, int numeric)
{
	const struct xt_ct_target_info_v1 *info =
		(const struct xt_ct_target_info_v1 *)target->data;

	if (info->flags & XT_CT_NOTRACK_ALIAS) {
		printf (" NOTRACK");
		return;
	}
	printf(" CT");
	if (info->flags & XT_CT_NOTRACK)
		printf(" notrack");
	if (info->helper[0])
		printf(" helper %s", info->helper);
	if (info->timeout[0])
		printf(" timeout %s", info->timeout);
	if (info->ct_events)
		ct_print_events("ctevents", ct_event_tbl,
				ARRAY_SIZE(ct_event_tbl), info->ct_events);
	if (info->exp_events)
		ct_print_events("expevents", exp_event_tbl,
				ARRAY_SIZE(exp_event_tbl), info->exp_events);
	if (info->flags & XT_CT_ZONE_MARK || info->zone)
		ct_print_zone_id("zone", info->zone, info->flags);
}

static void ct_save(const void *ip, const struct xt_entry_target *target)
{
	const struct xt_ct_target_info *info =
		(const struct xt_ct_target_info *)target->data;

	if (info->flags & XT_CT_NOTRACK_ALIAS)
		return;
	if (info->flags & XT_CT_NOTRACK)
		printf(" --notrack");
	if (info->helper[0])
		printf(" --helper %s", info->helper);
	if (info->ct_events)
		ct_print_events("--ctevents", ct_event_tbl,
				ARRAY_SIZE(ct_event_tbl), info->ct_events);
	if (info->exp_events)
		ct_print_events("--expevents", exp_event_tbl,
				ARRAY_SIZE(exp_event_tbl), info->exp_events);
	if (info->flags & XT_CT_ZONE_MARK || info->zone)
		ct_print_zone_id("--zone", info->zone, info->flags);
}

static void ct_save_v1(const void *ip, const struct xt_entry_target *target)
{
	const struct xt_ct_target_info_v1 *info =
		(const struct xt_ct_target_info_v1 *)target->data;

	if (info->flags & XT_CT_NOTRACK_ALIAS)
		return;
	if (info->flags & XT_CT_NOTRACK)
		printf(" --notrack");
	if (info->helper[0])
		printf(" --helper %s", info->helper);
	if (info->timeout[0])
		printf(" --timeout %s", info->timeout);
	if (info->ct_events)
		ct_print_events("--ctevents", ct_event_tbl,
				ARRAY_SIZE(ct_event_tbl), info->ct_events);
	if (info->exp_events)
		ct_print_events("--expevents", exp_event_tbl,
				ARRAY_SIZE(exp_event_tbl), info->exp_events);
	if (info->flags & XT_CT_ZONE_MARK || info->zone)
		ct_print_zone_id("--zone", info->zone, info->flags);
}

static const char *
ct_print_name_alias(const struct xt_entry_target *target)
{
	struct xt_ct_target_info *info = (void *)target->data;

	return info->flags & XT_CT_NOTRACK_ALIAS ? "NOTRACK" : "CT";
}

static void notrack_ct0_tg_init(struct xt_entry_target *target)
{
	struct xt_ct_target_info *info = (void *)target->data;

	info->flags = XT_CT_NOTRACK;
}

static void notrack_ct1_tg_init(struct xt_entry_target *target)
{
	struct xt_ct_target_info_v1 *info = (void *)target->data;

	info->flags = XT_CT_NOTRACK;
}

static void notrack_ct2_tg_init(struct xt_entry_target *target)
{
	struct xt_ct_target_info_v1 *info = (void *)target->data;

	info->flags = XT_CT_NOTRACK | XT_CT_NOTRACK_ALIAS;
}

static int xlate_ct1_tg(struct xt_xlate *xl,
			const struct xt_xlate_tg_params *params)
{
	struct xt_ct_target_info_v1 *info =
		(struct xt_ct_target_info_v1 *)params->target->data;

	if (info->flags & XT_CT_NOTRACK)
		xt_xlate_add(xl, "notrack");
	else
		return 0;

	return 1;
}

static struct xtables_target ct_target_reg[] = {
	{
		.family		= NFPROTO_UNSPEC,
		.name		= "CT",
		.version	= XTABLES_VERSION,
		.size		= XT_ALIGN(sizeof(struct xt_ct_target_info)),
		.userspacesize	= offsetof(struct xt_ct_target_info, ct),
		.help		= ct_help,
		.print		= ct_print,
		.save		= ct_save,
		.x6_parse	= ct_parse,
		.x6_options	= ct_opts,
	},
	{
		.family		= NFPROTO_UNSPEC,
		.name		= "CT",
		.revision	= 1,
		.version	= XTABLES_VERSION,
		.size		= XT_ALIGN(sizeof(struct xt_ct_target_info_v1)),
		.userspacesize	= offsetof(struct xt_ct_target_info_v1, ct),
		.help		= ct_help_v1,
		.print		= ct_print_v1,
		.save		= ct_save_v1,
		.x6_parse	= ct_parse_v1,
		.x6_options	= ct_opts_v1,
	},
	{
		.family		= NFPROTO_UNSPEC,
		.name		= "CT",
		.revision	= 2,
		.version	= XTABLES_VERSION,
		.size		= XT_ALIGN(sizeof(struct xt_ct_target_info_v1)),
		.userspacesize	= offsetof(struct xt_ct_target_info_v1, ct),
		.help		= ct_help_v1,
		.print		= ct_print_v1,
		.save		= ct_save_v1,
		.alias		= ct_print_name_alias,
		.x6_parse	= ct_parse_v1,
		.x6_options	= ct_opts_v1,
		.xlate		= xlate_ct1_tg,
	},
	{
		.family        = NFPROTO_UNSPEC,
		.name          = "NOTRACK",
		.real_name     = "CT",
		.revision      = 0,
		.version       = XTABLES_VERSION,
		.size          = XT_ALIGN(sizeof(struct xt_ct_target_info)),
		.userspacesize = offsetof(struct xt_ct_target_info, ct),
		.init          = notrack_ct0_tg_init,
	},
	{
		.family        = NFPROTO_UNSPEC,
		.name          = "NOTRACK",
		.real_name     = "CT",
		.revision      = 1,
		.version       = XTABLES_VERSION,
		.size          = XT_ALIGN(sizeof(struct xt_ct_target_info_v1)),
		.userspacesize = offsetof(struct xt_ct_target_info_v1, ct),
		.init          = notrack_ct1_tg_init,
	},
	{
		.family        = NFPROTO_UNSPEC,
		.name          = "NOTRACK",
		.real_name     = "CT",
		.revision      = 2,
		.ext_flags     = XTABLES_EXT_ALIAS,
		.version       = XTABLES_VERSION,
		.size          = XT_ALIGN(sizeof(struct xt_ct_target_info_v1)),
		.userspacesize = offsetof(struct xt_ct_target_info_v1, ct),
		.init          = notrack_ct2_tg_init,
		.xlate	       = xlate_ct1_tg,
	},
	{
		.family        = NFPROTO_UNSPEC,
		.name          = "NOTRACK",
		.revision      = 0,
		.version       = XTABLES_VERSION,
	},
};

void _init(void)
{
	xtables_register_targets(ct_target_reg, ARRAY_SIZE(ct_target_reg));
}
