/* -*- mode: c; c-basic-offset: 8 -*- */
/* Shared library add-on to iptables to add coova support. */
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include <linux/netfilter/x_tables.h>
#include <xtables.h>
#include "xt_coova.h"

static const struct option coova_opts[] = {
	{ .name = "name",    .has_arg = 1, .flag = 0, .val = 201 },
	{ .name = "source",  .has_arg = 0, .flag = 0, .val = 202 },
	{ .name = "dest",    .has_arg = 0, .flag = 0, .val = 203 },
	{ .name = 0, 	     .has_arg = 0, .flag = 0, .val = 0  }
};

static void coova_help(void)
{
	printf(
"coova match options:\n"
"    --name name                 Name of the table to be used. 'chilli' used if none given.\n"
"    --source                    Indicates the source direction (lookup by source MAC/IP)\n"
"    --dest                      Indicates the reply (lookup by dest address).\n"
"xt_coova by: David Bird (Coova Technologies) <support@coova.com>.  http://www.coova.org/CoovaChilli\n");
}

static void coova_init(struct xt_entry_match *match)
{
	struct xt_coova_mtinfo *info = (void *)(match)->data;
	strncpy(info->name,"chilli", XT_COOVA_NAME_LEN);
	info->name[XT_COOVA_NAME_LEN-1] = '\0';
	info->side = XT_COOVA_SOURCE;
}

static int coova_parse(int c, char **argv, int invert, unsigned int *flags,
                        const void *entry, 
			struct xt_entry_match **match)
{
	struct xt_coova_mtinfo *info = (void *)(*match)->data;

	switch (c) {
		case 201:
			strncpy(info->name,optarg, XT_COOVA_NAME_LEN);
			info->name[XT_COOVA_NAME_LEN-1] = '\0';
                        if (invert) info->invert = 1;
			break;

		case 202:
			info->side = XT_COOVA_SOURCE;
			break;

		case 203:
			info->side = XT_COOVA_DEST;
			break;

		default:
			return 0;
	}

	return 1;
}

static void coova_check(unsigned int flags)
{
}

static void coova_print(const void *ip, const struct xt_entry_match *match,
                         int numeric)
{
	const struct xt_coova_mtinfo *info = (const void *)match->data;
	if (info->invert)
		fputc('!', stdout);
	printf("coova: ");
	if(info->name) 
		printf("name: %s ",info->name);
	if (info->side == XT_COOVA_SOURCE)
		printf("side: source ");
	if (info->side == XT_COOVA_DEST)
		printf("side: dest");
}

static void coova_save(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_coova_mtinfo *info = (const void *)match->data;
	if (info->invert) 
		printf("! ");
	if(info->name) 
		printf("--name %s ",info->name);
	if (info->side == XT_COOVA_SOURCE)
		printf("--source ");
	if (info->side == XT_COOVA_DEST)
		printf("--dest ");
}

static struct xtables_match coova_mt_reg = {
	.name          = "coova",
	.version       = XTABLES_VERSION,
	.family        = NFPROTO_IPV4,
	.size          = XT_ALIGN(sizeof(struct xt_coova_mtinfo)),
	.userspacesize = XT_ALIGN(sizeof(struct xt_coova_mtinfo)),
	.help          = coova_help,
	.init          = coova_init,
	.parse         = coova_parse,
	.final_check   = coova_check,
	.print         = coova_print,
	.save          = coova_save,
	.extra_opts    = coova_opts,
};

static struct xtables_match coova_mt6_reg = {
	.name          = "coova",
	.version       = XTABLES_VERSION,
	.revision      = 0,
	.family        = NFPROTO_IPV6,
	.size          = XT_ALIGN(sizeof(struct xt_coova_mtinfo)),
	.userspacesize = XT_ALIGN(sizeof(struct xt_coova_mtinfo)),
	.help          = coova_help,
	.init          = coova_init,
	.parse         = coova_parse,
	.final_check   = coova_check,
	.print         = coova_print,
	.save          = coova_save,
	.extra_opts    = coova_opts,
};

void _init(void)
{
	xtables_register_match(&coova_mt_reg);
	xtables_register_match(&coova_mt6_reg);
}
