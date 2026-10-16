#include <stdio.h>
#include <xtables.h>
#include <linux/netfilter/xt_helper.h>

enum {
	O_HELPER = 0,
};

static void helper_help(void)
{
	printf(
"helper match options:\n"
"[!] --helper string        Match helper identified by string\n");
}

static const struct xt_option_entry helper_opts[] = {
	{.name = "helper", .id = O_HELPER, .type = XTTYPE_STRING,
	 .flags = XTOPT_MAND | XTOPT_INVERT | XTOPT_PUT,
	 XTOPT_POINTER(struct xt_helper_info, name)},
	XTOPT_TABLEEND,
};

static void helper_parse(struct xt_option_call *cb)
{
	struct xt_helper_info *info = cb->data;

	xtables_option_parse(cb);
	if (cb->invert)
		info->invert = 1;
}

static void
helper_print(const void *ip, const struct xt_entry_match *match, int numeric)
{
	const struct xt_helper_info *info = (const void *)match->data;

	printf(" helper match %s\"%s\"", info->invert ? "! " : "", info->name);
}

static void helper_save(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_helper_info *info = (const void *)match->data;

	printf("%s --helper", info->invert ? " !" : "");
	xtables_save_string(info->name);
}

static int helper_xlate(struct xt_xlate *xl,
			const struct xt_xlate_mt_params *params)
{
	const struct xt_helper_info *info = (const void *)params->match->data;

	xt_xlate_add(xl, "ct helper%s \"%s\"",
		     info->invert ? " !=" : "", info->name);

	return 1;
}

static struct xtables_match helper_match = {
	.family		= NFPROTO_UNSPEC,
	.name		= "helper",
	.version	= XTABLES_VERSION,
	.size		= XT_ALIGN(sizeof(struct xt_helper_info)),
	.help		= helper_help,
	.print		= helper_print,
	.save		= helper_save,
	.x6_parse	= helper_parse,
	.x6_options	= helper_opts,
	.xlate		= helper_xlate,
};

void _init(void)
{
	xtables_register_match(&helper_match);
}
