#include <stdbool.h>
#include <stdio.h>
#include <xtables.h>
#include <linux/netfilter/xt_bcmmark.h>

struct xt_mark_info {
	unsigned long mark, mask;
	uint8_t invert;
};

enum {
	O_BCMMARK = 0,
};

static void mark_mt_help(void)
{
	printf(
"bcmmark match options:\n"
"[!] --bcmmark value[/mask]    Match nfmark value with optional mask\n");
}

static const struct xt_option_entry mark_mt_opts[] = {
	{.name = "bcmmark", .id = O_BCMMARK, .type = XTTYPE_MARKMASK32,
	 .flags = XTOPT_MAND | XTOPT_INVERT},
	XTOPT_TABLEEND,
};

static void mark_mt_parse(struct xt_option_call *cb)
{
	struct xt_bcmmark_mtinfo1 *info = cb->data;

	xtables_option_parse(cb);
	if (cb->invert)
		info->invert = true;
	info->mark = cb->val.mark;
	info->mask = cb->val.mask;
}

static void mark_parse(struct xt_option_call *cb)
{
	struct xt_mark_info *markinfo = cb->data;

	xtables_option_parse(cb);
	if (cb->invert)
		markinfo->invert = 1;
	markinfo->mark = cb->val.mark;
	markinfo->mask = cb->val.mask;
}

static void
mark_mt_print(const void *ip, const struct xt_entry_match *match, int numeric)
{
	const struct xt_bcmmark_mtinfo1 *info = (const void *)match->data;

	printf(" bcmmark match");
	if (info->invert)
		printf(" !");

	xtables_print_mark_mask(info->mark, info->mask);
}

static void
mark_print(const void *ip, const struct xt_entry_match *match, int numeric)
{
	const struct xt_mark_info *info = (const void *)match->data;

	printf(" BCMMARK match");

	if (info->invert)
		printf(" !");

	xtables_print_mark_mask(info->mark, info->mask);
}

static void mark_mt_save(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_bcmmark_mtinfo1 *info = (const void *)match->data;

	if (info->invert)
		printf(" !");

	printf(" --bcmmark");
	xtables_print_mark_mask(info->mark, info->mask);
}

static void
mark_save(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_mark_info *info = (const void *)match->data;

	if (info->invert)
		printf(" !");

	printf(" --bcmmark");
	xtables_print_mark_mask(info->mark, info->mask);
}

static void
print_mark_xlate(struct xt_xlate *xl, unsigned int mark,
		 unsigned int mask, uint32_t op)
{
	if (mask != 0xffffffffU)
		xt_xlate_add(xl, " and 0x%x %s 0x%x", mask,
			   op == XT_OP_EQ ? "==" : "!=", mark);
	else
		xt_xlate_add(xl, " %s0x%x",
			   op == XT_OP_EQ ? "" : "!= ", mark);
}

static int mark_mt_xlate(struct xt_xlate *xl,
			 const struct xt_xlate_mt_params *params)
{
	const struct xt_bcmmark_mtinfo1 *info = (const void *)params->match->data;
	enum xt_op op = XT_OP_EQ;

	if (info->invert)
		op = XT_OP_NEQ;

	xt_xlate_add(xl, "bcmmark");
	print_mark_xlate(xl, info->mark, info->mask, op);

	return 1;
}

static int mark_xlate(struct xt_xlate *xl,
		      const struct xt_xlate_mt_params *params)
{
	const struct xt_mark_info *info = (const void *)params->match->data;
	enum xt_op op = XT_OP_EQ;

	if (info->invert)
		op = XT_OP_NEQ;

	xt_xlate_add(xl, "bcmmark");
	print_mark_xlate(xl, info->mark, info->mask, op);

	return 1;
}

static struct xtables_match bcmmark_mt_reg[] = {
	{
		.family        = NFPROTO_UNSPEC,
		.name          = "bcmmark",
		.revision      = 0,
		.version       = XTABLES_VERSION,
		.size          = XT_ALIGN(sizeof(struct xt_mark_info)),
		.userspacesize = XT_ALIGN(sizeof(struct xt_mark_info)),
		.help          = mark_mt_help,
		.print         = mark_print,
		.save          = mark_save,
		.x6_parse      = mark_parse,
		.x6_options    = mark_mt_opts,
		.xlate	       = mark_xlate,
	},
	{
		.version       = XTABLES_VERSION,
		.name          = "bcmmark",
		.revision      = 1,
		.family        = NFPROTO_UNSPEC,
		.size          = XT_ALIGN(sizeof(struct xt_bcmmark_mtinfo1)),
		.userspacesize = XT_ALIGN(sizeof(struct xt_bcmmark_mtinfo1)),
		.help          = mark_mt_help,
		.print         = mark_mt_print,
		.save          = mark_mt_save,
		.x6_parse      = mark_mt_parse,
		.x6_options    = mark_mt_opts,
		.xlate	       = mark_mt_xlate,
	},
};

void _init(void)
{
	xtables_register_matches(bcmmark_mt_reg, ARRAY_SIZE(bcmmark_mt_reg));
}
