/*
 * Shared library add-on to iptables to add SECMARK target support.
 *
 * Based on the MARK target.
 *
 * Copyright (C) 2006 Red Hat, Inc., James Morris <jmorris@redhat.com>
 */
#include <stdio.h>
#include <xtables.h>
#include <linux/netfilter/xt_SECMARK.h>

#define PFX "SECMARK target: "

enum {
	O_SELCTX = 0,
};

static void SECMARK_help(void)
{
	printf(
"SECMARK target options:\n"
"  --selctx value                     Set the SELinux security context\n");
}

static const struct xt_option_entry SECMARK_opts[] = {
	{.name = "selctx", .id = O_SELCTX, .type = XTTYPE_STRING,
	 .flags = XTOPT_MAND | XTOPT_PUT,
	 XTOPT_POINTER(struct xt_secmark_target_info, secctx)},
	XTOPT_TABLEEND,
};

static const struct xt_option_entry SECMARK_opts_v1[] = {
	{.name = "selctx", .id = O_SELCTX, .type = XTTYPE_STRING,
	 .flags = XTOPT_MAND | XTOPT_PUT,
	 XTOPT_POINTER(struct xt_secmark_target_info_v1, secctx)},
	XTOPT_TABLEEND,
};

static void SECMARK_parse(struct xt_option_call *cb)
{
	struct xt_secmark_target_info *info = cb->data;

	xtables_option_parse(cb);
	info->mode = SECMARK_MODE_SEL;
}

static void SECMARK_parse_v1(struct xt_option_call *cb)
{
	struct xt_secmark_target_info_v1 *info = cb->data;

	xtables_option_parse(cb);
	info->mode = SECMARK_MODE_SEL;
}

static void print_secmark(__u8 mode, const char *secctx)
{
	switch (mode) {
	case SECMARK_MODE_SEL:
		printf("selctx %s", secctx);
		break;

	default:
		xtables_error(OTHER_PROBLEM, PFX "invalid mode %hhu", mode);
	}
}

static void SECMARK_print(const void *ip, const struct xt_entry_target *target,
                          int numeric)
{
	const struct xt_secmark_target_info *info =
		(struct xt_secmark_target_info*)(target)->data;

	printf(" SECMARK ");
	print_secmark(info->mode, info->secctx);
}

static void SECMARK_print_v1(const void *ip,
			     const struct xt_entry_target *target, int numeric)
{
	const struct xt_secmark_target_info_v1 *info =
		(struct xt_secmark_target_info_v1 *)(target)->data;

	printf(" SECMARK ");
	print_secmark(info->mode, info->secctx);
}

static void SECMARK_save(const void *ip, const struct xt_entry_target *target)
{
	const struct xt_secmark_target_info *info =
		(struct xt_secmark_target_info*)target->data;

	printf(" --");
	print_secmark(info->mode, info->secctx);
}

static void SECMARK_save_v1(const void *ip,
			    const struct xt_entry_target *target)
{
	const struct xt_secmark_target_info_v1 *info =
		(struct xt_secmark_target_info_v1 *)target->data;

	printf(" --");
	print_secmark(info->mode, info->secctx);
}

static struct xtables_target secmark_tg_reg[] = {
	{
		.family		= NFPROTO_UNSPEC,
		.name		= "SECMARK",
		.version	= XTABLES_VERSION,
		.revision	= 0,
		.size		= XT_ALIGN(sizeof(struct xt_secmark_target_info)),
		.userspacesize	= XT_ALIGN(sizeof(struct xt_secmark_target_info)),
		.help		= SECMARK_help,
		.print		= SECMARK_print,
		.save		= SECMARK_save,
		.x6_parse	= SECMARK_parse,
		.x6_options	= SECMARK_opts,
	},
	{
		.family		= NFPROTO_UNSPEC,
		.name		= "SECMARK",
		.version	= XTABLES_VERSION,
		.revision	= 1,
		.size		= XT_ALIGN(sizeof(struct xt_secmark_target_info_v1)),
		.userspacesize	= XT_ALIGN(offsetof(struct xt_secmark_target_info_v1, secid)),
		.help		= SECMARK_help,
		.print		= SECMARK_print_v1,
		.save		= SECMARK_save_v1,
		.x6_parse	= SECMARK_parse_v1,
		.x6_options	= SECMARK_opts_v1,
	}
};

void _init(void)
{
	xtables_register_targets(secmark_tg_reg, ARRAY_SIZE(secmark_tg_reg));
}
