#include <stdio.h>
#include <string.h>
#include <xtables.h>
#include <linux/netfilter/xt_connbytes.h>

enum {
	O_CONNBYTES = 0,
	O_CONNBYTES_DIR,
	O_CONNBYTES_MODE,
};

static void connbytes_help(void)
{
	printf(
"connbytes match options:\n"
" [!] --connbytes from:[to]\n"
"     --connbytes-dir [original, reply, both]\n"
"     --connbytes-mode [packets, bytes, avgpkt]\n");
}

static const struct xt_option_entry connbytes_opts[] = {
	{.name = "connbytes", .id = O_CONNBYTES, .type = XTTYPE_UINT64RC,
	 .flags = XTOPT_MAND | XTOPT_INVERT},
	{.name = "connbytes-dir", .id = O_CONNBYTES_DIR, .type = XTTYPE_STRING,
	 .flags = XTOPT_MAND},
	{.name = "connbytes-mode", .id = O_CONNBYTES_MODE,
	 .type = XTTYPE_STRING, .flags = XTOPT_MAND},
	XTOPT_TABLEEND,
};

static void connbytes_parse(struct xt_option_call *cb)
{
	struct xt_connbytes_info *sinfo = cb->data;
	unsigned long long i;

	xtables_option_parse(cb);
	switch (cb->entry->id) {
	case O_CONNBYTES:
		sinfo->count.from = cb->val.u64_range[0];
		sinfo->count.to   = UINT64_MAX;
		if (cb->nvals == 2)
			sinfo->count.to = cb->val.u64_range[1];

		if (cb->invert) {
			i = sinfo->count.from;
			sinfo->count.from = sinfo->count.to;
			sinfo->count.to = i;
		}
		break;
	case O_CONNBYTES_DIR:
		if (strcmp(cb->arg, "original") == 0)
			sinfo->direction = XT_CONNBYTES_DIR_ORIGINAL;
		else if (strcmp(cb->arg, "reply") == 0)
			sinfo->direction = XT_CONNBYTES_DIR_REPLY;
		else if (strcmp(cb->arg, "both") == 0)
			sinfo->direction = XT_CONNBYTES_DIR_BOTH;
		else
			xtables_error(PARAMETER_PROBLEM,
				   "Unknown --connbytes-dir `%s'", cb->arg);
		break;
	case O_CONNBYTES_MODE:
		if (strcmp(cb->arg, "packets") == 0)
			sinfo->what = XT_CONNBYTES_PKTS;
		else if (strcmp(cb->arg, "bytes") == 0)
			sinfo->what = XT_CONNBYTES_BYTES;
		else if (strcmp(cb->arg, "avgpkt") == 0)
			sinfo->what = XT_CONNBYTES_AVGPKT;
		else
			xtables_error(PARAMETER_PROBLEM,
				   "Unknown --connbytes-mode `%s'", cb->arg);
		break;
	}
}

static void print_mode(const struct xt_connbytes_info *sinfo)
{
	switch (sinfo->what) {
		case XT_CONNBYTES_PKTS:
			fputs(" packets", stdout);
			break;
		case XT_CONNBYTES_BYTES:
			fputs(" bytes", stdout);
			break;
		case XT_CONNBYTES_AVGPKT:
			fputs(" avgpkt", stdout);
			break;
		default:
			fputs(" unknown", stdout);
			break;
	}
}

static void print_direction(const struct xt_connbytes_info *sinfo)
{
	switch (sinfo->direction) {
		case XT_CONNBYTES_DIR_ORIGINAL:
			fputs(" original", stdout);
			break;
		case XT_CONNBYTES_DIR_REPLY:
			fputs(" reply", stdout);
			break;
		case XT_CONNBYTES_DIR_BOTH:
			fputs(" both", stdout);
			break;
		default:
			fputs(" unknown", stdout);
			break;
	}
}

static void print_from_to(const struct xt_connbytes_info *sinfo, const char *prefix)
{
	unsigned long long from, to;

	if (sinfo->count.from > sinfo->count.to) {
		fputs(" !", stdout);
		from = sinfo->count.to;
		to = sinfo->count.from;
	} else {
		to = sinfo->count.to;
		from = sinfo->count.from;
	}
	printf(" %sconnbytes %llu", prefix, from);
	if (to && to < UINT64_MAX)
		printf(":%llu", to);
}

static void
connbytes_print(const void *ip, const struct xt_entry_match *match, int numeric)
{
	const struct xt_connbytes_info *sinfo = (const void *)match->data;

	print_from_to(sinfo, "");

	fputs(" connbytes mode", stdout);
	print_mode(sinfo);

	fputs(" connbytes direction", stdout);
	print_direction(sinfo);
}

static void connbytes_save(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_connbytes_info *sinfo = (const void *)match->data;

	print_from_to(sinfo, "--");

	fputs(" --connbytes-mode", stdout);
	print_mode(sinfo);

	fputs(" --connbytes-dir", stdout);
	print_direction(sinfo);
}


static int connbytes_xlate(struct xt_xlate *xl,
			   const struct xt_xlate_mt_params *params)
{
	const struct xt_connbytes_info *info = (void *)params->match->data;
	unsigned long long from, to;
	bool invert = false;

	xt_xlate_add(xl, "ct ");

	switch (info->direction) {
	case XT_CONNBYTES_DIR_ORIGINAL:
		xt_xlate_add(xl, "original ");
		break;
	case XT_CONNBYTES_DIR_REPLY:
		xt_xlate_add(xl, "reply ");
		break;
	case XT_CONNBYTES_DIR_BOTH:
		break;
	default:
		return 0;
	}

	switch (info->what) {
	case XT_CONNBYTES_PKTS:
		xt_xlate_add(xl, "packets ");
		break;
	case XT_CONNBYTES_BYTES:
		xt_xlate_add(xl, "bytes ");
		break;
	case XT_CONNBYTES_AVGPKT:
		xt_xlate_add(xl, "avgpkt ");
		break;
	default:
		return 0;
	}

	if (info->count.from > info->count.to) {
		invert = true;
		from = info->count.to;
		to = info->count.from;
	} else {
		to = info->count.to;
		from = info->count.from;
	}

	if (from == to)
		xt_xlate_add(xl, "%llu", from);
	else if (to == UINT64_MAX)
		xt_xlate_add(xl, "%s %llu", invert ? "lt" : "ge", from);
	else
		xt_xlate_add(xl, "%s%llu-%llu", invert ? "!= " : "", from, to);
	return 1;
}

static struct xtables_match connbytes_match = {
	.family		= NFPROTO_UNSPEC,
	.name 		= "connbytes",
	.version 	= XTABLES_VERSION,
	.size 		= XT_ALIGN(sizeof(struct xt_connbytes_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct xt_connbytes_info)),
	.help		= connbytes_help,
	.print		= connbytes_print,
	.save 		= connbytes_save,
	.x6_parse	= connbytes_parse,
	.x6_options	= connbytes_opts,
	.xlate		= connbytes_xlate,
};

void _init(void)
{
	xtables_register_match(&connbytes_match);
}
