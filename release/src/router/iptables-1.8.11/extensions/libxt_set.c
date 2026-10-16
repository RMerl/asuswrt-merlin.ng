/* Copyright (C) 2000-2002 Joakim Axelsson <gozem@linux.nu>
 *                         Patrick Schaaf <bof@bof.de>
 *                         Martin Josefsson <gandalf@wlug.westbo.se>
 * Copyright (C) 2003-2010 Jozsef Kadlecsik <kadlec@blackhole.kfki.hu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.  
 */

/* Shared library add-on to iptables to add IP set matching. */
#include <stdbool.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <ctype.h>
#include <errno.h>

#include <xtables.h>
#include <linux/netfilter/xt_set.h>
#include "libxt_set.h"

#ifdef DEBUG
#define DEBUGP(x, args...) fprintf(stderr, x, ## args)
#else
#define DEBUGP(x, args...)
#endif

/* Revision 0 */

static void
set_help_v0(void)
{
	printf("set match options:\n"
	       " [!] --match-set name flags\n"
	       "		 'name' is the set name from to match,\n" 
	       "		 'flags' are the comma separated list of\n"
	       "		 'src' and 'dst' specifications.\n");
}

static const struct option set_opts_v0[] = {
	{.name = "match-set", .has_arg = true, .val = '1'},
	{.name = "set",       .has_arg = true, .val = '2'},
	XT_GETOPT_TABLEEND,
};

static void
set_check_v0(unsigned int flags)
{
	if (!flags)
		xtables_error(PARAMETER_PROBLEM,
			"You must specify `--match-set' with proper arguments");
}

static int
set_parse_v0(int c, char **argv, int invert, unsigned int *flags,
	     const void *entry, struct xt_entry_match **match)
{
	struct xt_set_info_match_v0 *myinfo =
		(struct xt_set_info_match_v0 *) (*match)->data;
	struct xt_set_info_v0 *info = &myinfo->match_set;

	switch (c) {
	case '2':
		fprintf(stderr,
			"--set option deprecated, please use --match-set\n");
		/* fall through */
	case '1':		/* --match-set <set> <flag>[,<flag> */
		if (info->u.flags[0])
			xtables_error(PARAMETER_PROBLEM,
				      "--match-set can be specified only once");
		if (invert)
			info->u.flags[0] |= IPSET_MATCH_INV;

		if (!argv[optind]
		    || argv[optind][0] == '-'
		    || argv[optind][0] == '!')
			xtables_error(PARAMETER_PROBLEM,
				      "--match-set requires two args.");

		if (strlen(optarg) > IPSET_MAXNAMELEN - 1)
			xtables_error(PARAMETER_PROBLEM,
				      "setname `%s' too long, max %d characters.",
				      optarg, IPSET_MAXNAMELEN - 1);

		get_set_byname(optarg, (struct xt_set_info *)info);
		parse_dirs_v0(argv[optind], info);
		DEBUGP("parse: set index %u\n", info->index);
		optind++;

		*flags = 1;
		break;
	}

	return 1;
}

static void
print_match_v0(const char *prefix, const struct xt_set_info_v0 *info)
{
	int i;
	char setname[IPSET_MAXNAMELEN];

	get_set_byid(setname, info->index);
	printf("%s %s %s",
	       (info->u.flags[0] & IPSET_MATCH_INV) ? " !" : "",
	       prefix,
	       setname); 
	for (i = 0; i < IPSET_DIM_MAX; i++) {
		if (!info->u.flags[i])
			break;		
		printf("%s%s",
		       i == 0 ? " " : ",",
		       info->u.flags[i] & IPSET_SRC ? "src" : "dst");
	}
}

/* Prints out the matchinfo. */
static void
set_print_v0(const void *ip, const struct xt_entry_match *match, int numeric)
{
	const struct xt_set_info_match_v0 *info = (const void *)match->data;

	print_match_v0("match-set", &info->match_set);
}

static void
set_save_v0(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_set_info_match_v0 *info = (const void *)match->data;

	print_match_v0("--match-set", &info->match_set);
}

/* Revision 1 */
static int
set_parse_v1(int c, char **argv, int invert, unsigned int *flags,
	     const void *entry, struct xt_entry_match **match)
{
	struct xt_set_info_match_v1 *myinfo =
		(struct xt_set_info_match_v1 *) (*match)->data;
	struct xt_set_info *info = &myinfo->match_set;

	switch (c) {
	case '2':
		fprintf(stderr,
			"--set option deprecated, please use --match-set\n");
		/* fall through */
	case '1':		/* --match-set <set> <flag>[,<flag> */
		if (info->dim)
			xtables_error(PARAMETER_PROBLEM,
				      "--match-set can be specified only once");
		if (invert)
			info->flags |= IPSET_INV_MATCH;

		if (!argv[optind]
		    || argv[optind][0] == '-'
		    || argv[optind][0] == '!')
			xtables_error(PARAMETER_PROBLEM,
				      "--match-set requires two args.");

		if (strlen(optarg) > IPSET_MAXNAMELEN - 1)
			xtables_error(PARAMETER_PROBLEM,
				      "setname `%s' too long, max %d characters.",
				      optarg, IPSET_MAXNAMELEN - 1);

		get_set_byname(optarg, info);
		parse_dirs(argv[optind], info);
		DEBUGP("parse: set index %u\n", info->index);
		optind++;

		*flags = 1;
		break;
	}

	return 1;
}

static void
print_match(const char *prefix, const struct xt_set_info *info)
{
	int i;
	char setname[IPSET_MAXNAMELEN];

	get_set_byid(setname, info->index);
	printf("%s %s %s",
	       (info->flags & IPSET_INV_MATCH) ? " !" : "",
	       prefix,
	       setname); 
	for (i = 1; i <= info->dim; i++) {		
		printf("%s%s",
		       i == 1 ? " " : ",",
		       info->flags & (1 << i) ? "src" : "dst");
	}
}

/* Prints out the matchinfo. */
static void
set_print_v1(const void *ip, const struct xt_entry_match *match, int numeric)
{
	const struct xt_set_info_match_v1 *info = (const void *)match->data;

	print_match("match-set", &info->match_set);
}

static void
set_save_v1(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_set_info_match_v1 *info = (const void *)match->data;

	print_match("--match-set", &info->match_set);
}

/* Revision 2 */
static void
set_help_v2(void)
{
	printf("set match options:\n"
	       " [!] --match-set name flags [--return-nomatch]\n"
	       "		 'name' is the set name from to match,\n" 
	       "		 'flags' are the comma separated list of\n"
	       "		 'src' and 'dst' specifications.\n");
}

static const struct option set_opts_v2[] = {
	{.name = "match-set",		.has_arg = true,	.val = '1'},
	{.name = "set",			.has_arg = true,	.val = '2'},
	{.name = "return-nomatch",	.has_arg = false,	.val = '3'},
	XT_GETOPT_TABLEEND,
};

static int
set_parse_v2(int c, char **argv, int invert, unsigned int *flags,
	     const void *entry, struct xt_entry_match **match)
{
	struct xt_set_info_match_v1 *myinfo =
		(struct xt_set_info_match_v1 *) (*match)->data;
	struct xt_set_info *info = &myinfo->match_set;

	switch (c) {
	case '3':
		info->flags |= IPSET_RETURN_NOMATCH;
		break;
	case '2':
		fprintf(stderr,
			"--set option deprecated, please use --match-set\n");
		/* fall through */
	case '1':		/* --match-set <set> <flag>[,<flag> */
		if (info->dim)
			xtables_error(PARAMETER_PROBLEM,
				      "--match-set can be specified only once");
		if (invert)
			info->flags |= IPSET_INV_MATCH;

		if (!argv[optind]
		    || argv[optind][0] == '-'
		    || argv[optind][0] == '!')
			xtables_error(PARAMETER_PROBLEM,
				      "--match-set requires two args.");

		if (strlen(optarg) > IPSET_MAXNAMELEN - 1)
			xtables_error(PARAMETER_PROBLEM,
				      "setname `%s' too long, max %d characters.",
				      optarg, IPSET_MAXNAMELEN - 1);

		get_set_byname(optarg, info);
		parse_dirs(argv[optind], info);
		DEBUGP("parse: set index %u\n", info->index);
		optind++;

		*flags = 1;
		break;
	}

	return 1;
}

/* Prints out the matchinfo. */
static void
set_print_v2(const void *ip, const struct xt_entry_match *match, int numeric)
{
	const struct xt_set_info_match_v1 *info = (const void *)match->data;

	print_match("match-set", &info->match_set);
	if (info->match_set.flags & IPSET_RETURN_NOMATCH)
		printf(" return-nomatch");
}

static void
set_save_v2(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_set_info_match_v1 *info = (const void *)match->data;

	print_match("--match-set", &info->match_set);
	if (info->match_set.flags & IPSET_RETURN_NOMATCH)
		printf(" --return-nomatch");
}

/* Revision 3 */
static void
set_help_v3(void)
{
	printf("set match options:\n"
	       " [!] --match-set name flags [--return-nomatch]\n"
	       "   [! --update-counters] [! --update-subcounters]\n"
	       "   [[!] --packets-eq value | --packets-lt value | --packets-gt value\n"
	       "   [[!] --bytes-eq value | --bytes-lt value | --bytes-gt value\n"
	       "		 'name' is the set name from to match,\n" 
	       "		 'flags' are the comma separated list of\n"
	       "		 'src' and 'dst' specifications.\n");
}

static const struct option set_opts_v3[] = {
	{.name = "match-set",		.has_arg = true,	.val = '1'},
	{.name = "set",			.has_arg = true,	.val = '2'},
	{.name = "return-nomatch",	.has_arg = false,	.val = '3'},
	{.name = "update-counters",	.has_arg = false,	.val = '4'},
	{.name = "packets-eq",		.has_arg = true,	.val = '5'},
	{.name = "packets-lt",		.has_arg = true,	.val = '6'},
	{.name = "packets-gt",		.has_arg = true,	.val = '7'},
	{.name = "bytes-eq",		.has_arg = true,	.val = '8'},
	{.name = "bytes-lt",		.has_arg = true,	.val = '9'},
	{.name = "bytes-gt",		.has_arg = true,	.val = '0'},
	{.name = "update-subcounters",	.has_arg = false,	.val = 'a'},
	XT_GETOPT_TABLEEND,
};

static uint64_t
parse_counter(const char *opt)
{
	uintmax_t value;

	if (!xtables_strtoul(opt, NULL, &value, 0, UINT64_MAX))
		xtables_error(PARAMETER_PROBLEM,
			      "Cannot parse %s as a counter value", opt);
	return (uint64_t)value;
}

static int
set_parse_v3(int c, char **argv, int invert, unsigned int *flags,
	     const void *entry, struct xt_entry_match **match)
{
	struct xt_set_info_match_v3 *info =
		(struct xt_set_info_match_v3 *) (*match)->data;

	switch (c) {
	case 'a':
		if (invert)
			info->flags |= IPSET_FLAG_SKIP_SUBCOUNTER_UPDATE;
		break;
	case '0':
		if (info->bytes.op != IPSET_COUNTER_NONE)
			xtables_error(PARAMETER_PROBLEM,
				      "only one of the --bytes-[eq|lt|gt] is allowed");
		if (invert)
			xtables_error(PARAMETER_PROBLEM,
				      "--bytes-gt option cannot be inverted");
		info->bytes.op = IPSET_COUNTER_GT;
		info->bytes.value = parse_counter(optarg);
		break;
	case '9':
		if (info->bytes.op != IPSET_COUNTER_NONE)
			xtables_error(PARAMETER_PROBLEM,
				      "only one of the --bytes-[eq|lt|gt] is allowed");
		if (invert)
			xtables_error(PARAMETER_PROBLEM,
				      "--bytes-lt option cannot be inverted");
		info->bytes.op = IPSET_COUNTER_LT;
		info->bytes.value = parse_counter(optarg);
		break;
	case '8':
		if (info->bytes.op != IPSET_COUNTER_NONE)
			xtables_error(PARAMETER_PROBLEM,
				      "only one of the --bytes-[eq|lt|gt] is allowed");
		info->bytes.op = invert ? IPSET_COUNTER_NE : IPSET_COUNTER_EQ;
		info->bytes.value = parse_counter(optarg);
		break;
	case '7':
		if (info->packets.op != IPSET_COUNTER_NONE)
			xtables_error(PARAMETER_PROBLEM,
				      "only one of the --packets-[eq|lt|gt] is allowed");
		if (invert)
			xtables_error(PARAMETER_PROBLEM,
				      "--packets-gt option cannot be inverted");
		info->packets.op = IPSET_COUNTER_GT;
		info->packets.value = parse_counter(optarg);
		break;
	case '6':
		if (info->packets.op != IPSET_COUNTER_NONE)
			xtables_error(PARAMETER_PROBLEM,
				      "only one of the --packets-[eq|lt|gt] is allowed");
		if (invert)
			xtables_error(PARAMETER_PROBLEM,
				      "--packets-lt option cannot be inverted");
		info->packets.op = IPSET_COUNTER_LT;
		info->packets.value = parse_counter(optarg);
		break;
	case '5':
		if (info->packets.op != IPSET_COUNTER_NONE)
			xtables_error(PARAMETER_PROBLEM,
				      "only one of the --packets-[eq|lt|gt] is allowed");
		info->packets.op = invert ? IPSET_COUNTER_NE : IPSET_COUNTER_EQ;
		info->packets.value = parse_counter(optarg);
		break;
	case '4':
		if (invert)
			info->flags |= IPSET_FLAG_SKIP_COUNTER_UPDATE;
		break;
	case '3':
		if (invert)
			xtables_error(PARAMETER_PROBLEM,
				      "--return-nomatch flag cannot be inverted");
		info->flags |= IPSET_FLAG_RETURN_NOMATCH;
		break;
	case '2':
		fprintf(stderr,
			"--set option deprecated, please use --match-set\n");
		/* fall through */
	case '1':		/* --match-set <set> <flag>[,<flag> */
		if (info->match_set.dim)
			xtables_error(PARAMETER_PROBLEM,
				      "--match-set can be specified only once");
		if (invert)
			info->match_set.flags |= IPSET_INV_MATCH;

		if (!argv[optind]
		    || argv[optind][0] == '-'
		    || argv[optind][0] == '!')
			xtables_error(PARAMETER_PROBLEM,
				      "--match-set requires two args.");

		if (strlen(optarg) > IPSET_MAXNAMELEN - 1)
			xtables_error(PARAMETER_PROBLEM,
				      "setname `%s' too long, max %d characters.",
				      optarg, IPSET_MAXNAMELEN - 1);

		get_set_byname(optarg, &info->match_set);
		parse_dirs(argv[optind], &info->match_set);
		DEBUGP("parse: set index %u\n", info->match_set.index);
		optind++;

		*flags = 1;
		break;
	}

	return 1;
}

static void
set_printv3_counter(const struct ip_set_counter_match0 *c, const char *name,
		    const char *sep)
{
	switch (c->op) {
	case IPSET_COUNTER_EQ:
		printf(" %s%s-eq %llu", sep, name, c->value);
		break;
	case IPSET_COUNTER_NE:
		printf(" ! %s%s-eq %llu", sep, name, c->value);
		break;
	case IPSET_COUNTER_LT:
		printf(" %s%s-lt %llu", sep, name, c->value);
		break;
	case IPSET_COUNTER_GT:
		printf(" %s%s-gt %llu", sep, name, c->value);
		break;
	}
}

static void
set_print_v3_matchinfo(const struct xt_set_info_match_v3 *info,
		       const char *opt, const char *sep)
{
	print_match(opt, &info->match_set);
	if (info->flags & IPSET_FLAG_RETURN_NOMATCH)
		printf(" %sreturn-nomatch", sep);
	if ((info->flags & IPSET_FLAG_SKIP_COUNTER_UPDATE))
		printf(" ! %supdate-counters", sep);
	if ((info->flags & IPSET_FLAG_SKIP_SUBCOUNTER_UPDATE))
		printf(" ! %supdate-subcounters", sep);
	set_printv3_counter(&info->packets, "packets", sep);
	set_printv3_counter(&info->bytes, "bytes", sep);
}

/* Prints out the matchinfo. */
static void
set_print_v3(const void *ip, const struct xt_entry_match *match, int numeric)
{
	const struct xt_set_info_match_v3 *info = (const void *)match->data;

	set_print_v3_matchinfo(info, "match-set", "");
}

static void
set_save_v3(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_set_info_match_v3 *info = (const void *)match->data;

	set_print_v3_matchinfo(info, "--match-set", "--");
}

/* Revision 4 */
static int
set_parse_v4(int c, char **argv, int invert, unsigned int *flags,
	     const void *entry, struct xt_entry_match **match)
{
	struct xt_set_info_match_v4 *info =
		(struct xt_set_info_match_v4 *) (*match)->data;

	switch (c) {
	case 'a':
		if (invert)
			info->flags |= IPSET_FLAG_SKIP_SUBCOUNTER_UPDATE;
		break;
	case '0':
		if (info->bytes.op != IPSET_COUNTER_NONE)
			xtables_error(PARAMETER_PROBLEM,
				      "only one of the --bytes-[eq|lt|gt] is allowed");
		if (invert)
			xtables_error(PARAMETER_PROBLEM,
				      "--bytes-gt option cannot be inverted");
		info->bytes.op = IPSET_COUNTER_GT;
		info->bytes.value = parse_counter(optarg);
		break;
	case '9':
		if (info->bytes.op != IPSET_COUNTER_NONE)
			xtables_error(PARAMETER_PROBLEM,
				      "only one of the --bytes-[eq|lt|gt] is allowed");
		if (invert)
			xtables_error(PARAMETER_PROBLEM,
				      "--bytes-lt option cannot be inverted");
		info->bytes.op = IPSET_COUNTER_LT;
		info->bytes.value = parse_counter(optarg);
		break;
	case '8':
		if (info->bytes.op != IPSET_COUNTER_NONE)
			xtables_error(PARAMETER_PROBLEM,
				      "only one of the --bytes-[eq|lt|gt] is allowed");
		info->bytes.op = invert ? IPSET_COUNTER_NE : IPSET_COUNTER_EQ;
		info->bytes.value = parse_counter(optarg);
		break;
	case '7':
		if (info->packets.op != IPSET_COUNTER_NONE)
			xtables_error(PARAMETER_PROBLEM,
				      "only one of the --packets-[eq|lt|gt] is allowed");
		if (invert)
			xtables_error(PARAMETER_PROBLEM,
				      "--packets-gt option cannot be inverted");
		info->packets.op = IPSET_COUNTER_GT;
		info->packets.value = parse_counter(optarg);
		break;
	case '6':
		if (info->packets.op != IPSET_COUNTER_NONE)
			xtables_error(PARAMETER_PROBLEM,
				      "only one of the --packets-[eq|lt|gt] is allowed");
		if (invert)
			xtables_error(PARAMETER_PROBLEM,
				      "--packets-lt option cannot be inverted");
		info->packets.op = IPSET_COUNTER_LT;
		info->packets.value = parse_counter(optarg);
		break;
	case '5':
		if (info->packets.op != IPSET_COUNTER_NONE)
			xtables_error(PARAMETER_PROBLEM,
				      "only one of the --packets-[eq|lt|gt] is allowed");
		info->packets.op = invert ? IPSET_COUNTER_NE : IPSET_COUNTER_EQ;
		info->packets.value = parse_counter(optarg);
		break;
	case '4':
		if (invert)
			info->flags |= IPSET_FLAG_SKIP_COUNTER_UPDATE;
		break;
	case '3':
		if (invert)
			xtables_error(PARAMETER_PROBLEM,
				      "--return-nomatch flag cannot be inverted");
		info->flags |= IPSET_FLAG_RETURN_NOMATCH;
		break;
	case '2':
		fprintf(stderr,
			"--set option deprecated, please use --match-set\n");
		/* fall through */
	case '1':		/* --match-set <set> <flag>[,<flag> */
		if (info->match_set.dim)
			xtables_error(PARAMETER_PROBLEM,
				      "--match-set can be specified only once");
		if (invert)
			info->match_set.flags |= IPSET_INV_MATCH;

		if (!argv[optind]
		    || argv[optind][0] == '-'
		    || argv[optind][0] == '!')
			xtables_error(PARAMETER_PROBLEM,
				      "--match-set requires two args.");

		if (strlen(optarg) > IPSET_MAXNAMELEN - 1)
			xtables_error(PARAMETER_PROBLEM,
				      "setname `%s' too long, max %d characters.",
				      optarg, IPSET_MAXNAMELEN - 1);

		get_set_byname(optarg, &info->match_set);
		parse_dirs(argv[optind], &info->match_set);
		DEBUGP("parse: set index %u\n", info->match_set.index);
		optind++;

		*flags = 1;
		break;
	}

	return 1;
}

static void
set_printv4_counter(const struct ip_set_counter_match *c, const char *name,
		    const char *sep)
{
	switch (c->op) {
	case IPSET_COUNTER_EQ:
		printf(" %s%s-eq %llu", sep, name, c->value);
		break;
	case IPSET_COUNTER_NE:
		printf(" ! %s%s-eq %llu", sep, name, c->value);
		break;
	case IPSET_COUNTER_LT:
		printf(" %s%s-lt %llu", sep, name, c->value);
		break;
	case IPSET_COUNTER_GT:
		printf(" %s%s-gt %llu", sep, name, c->value);
		break;
	}
}

static void
set_print_v4_matchinfo(const struct xt_set_info_match_v4 *info,
		       const char *opt, const char *sep)
{
	print_match(opt, &info->match_set);
	if (info->flags & IPSET_FLAG_RETURN_NOMATCH)
		printf(" %sreturn-nomatch", sep);
	if ((info->flags & IPSET_FLAG_SKIP_COUNTER_UPDATE))
		printf(" ! %supdate-counters", sep);
	if ((info->flags & IPSET_FLAG_SKIP_SUBCOUNTER_UPDATE))
		printf(" ! %supdate-subcounters", sep);
	set_printv4_counter(&info->packets, "packets", sep);
	set_printv4_counter(&info->bytes, "bytes", sep);
}

/* Prints out the matchinfo. */
static void
set_print_v4(const void *ip, const struct xt_entry_match *match, int numeric)
{
	const struct xt_set_info_match_v4 *info = (const void *)match->data;

	set_print_v4_matchinfo(info, "match-set", "");
}

static void
set_save_v4(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_set_info_match_v4 *info = (const void *)match->data;

	set_print_v4_matchinfo(info, "--match-set", "--");
}

static struct xtables_match set_mt_reg[] = {
	{
		.name		= "set",
		.revision	= 0,
		.version	= XTABLES_VERSION,
		.family		= NFPROTO_IPV4,
		.size		= XT_ALIGN(sizeof(struct xt_set_info_match_v0)),
		.userspacesize	= XT_ALIGN(sizeof(struct xt_set_info_match_v0)),
		.help		= set_help_v0,
		.parse		= set_parse_v0,
		.final_check	= set_check_v0,
		.print		= set_print_v0,
		.save		= set_save_v0,
		.extra_opts	= set_opts_v0,
	},
	{
		.name		= "set",
		.revision	= 1,
		.version	= XTABLES_VERSION,
		.family		= NFPROTO_UNSPEC,
		.size		= XT_ALIGN(sizeof(struct xt_set_info_match_v1)),
		.userspacesize	= XT_ALIGN(sizeof(struct xt_set_info_match_v1)),
		.help		= set_help_v0,
		.parse		= set_parse_v1,
		.final_check	= set_check_v0,
		.print		= set_print_v1,
		.save		= set_save_v1,
		.extra_opts	= set_opts_v0,
	},
	{
		.name		= "set",
		.revision	= 2,
		.version	= XTABLES_VERSION,
		.family		= NFPROTO_UNSPEC,
		.size		= XT_ALIGN(sizeof(struct xt_set_info_match_v1)),
		.userspacesize	= XT_ALIGN(sizeof(struct xt_set_info_match_v1)),
		.help		= set_help_v2,
		.parse		= set_parse_v2,
		.final_check	= set_check_v0,
		.print		= set_print_v2,
		.save		= set_save_v2,
		.extra_opts	= set_opts_v2,
	},
	{
		.name		= "set",
		.revision	= 3,
		.version	= XTABLES_VERSION,
		.family		= NFPROTO_UNSPEC,
		.size		= XT_ALIGN(sizeof(struct xt_set_info_match_v3)),
		.userspacesize	= XT_ALIGN(sizeof(struct xt_set_info_match_v3)),
		.help		= set_help_v3,
		.parse		= set_parse_v3,
		.final_check	= set_check_v0,
		.print		= set_print_v3,
		.save		= set_save_v3,
		.extra_opts	= set_opts_v3,
	},
	{
		.name		= "set",
		.revision	= 4,
		.version	= XTABLES_VERSION,
		.family		= NFPROTO_UNSPEC,
		.size		= XT_ALIGN(sizeof(struct xt_set_info_match_v4)),
		.userspacesize	= XT_ALIGN(sizeof(struct xt_set_info_match_v4)),
		.help		= set_help_v3,
		.parse		= set_parse_v4,
		.final_check	= set_check_v0,
		.print		= set_print_v4,
		.save		= set_save_v4,
		.extra_opts	= set_opts_v3,
	},
};

void _init(void)
{
	xtables_register_matches(set_mt_reg, ARRAY_SIZE(set_mt_reg));
}
