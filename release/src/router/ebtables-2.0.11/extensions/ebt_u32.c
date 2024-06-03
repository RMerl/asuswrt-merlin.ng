/* ebt_u32
 * 
 * Shared library add-on to iptables to add u32 matching,
  * generalized matching on values found at packet offsets
  *
  * Detailed doc is in the kernel module source
  * net/netfilter/xt_u32.c
  *
  * (C) 2002 by Don Cohen <don-netf@isis.cs3-inc.com>
  * Released under the terms of GNU GPL v2
  *
  * Copyright © CC Computer Consultants GmbH, 2007
  * Contact: <jengelh@computergmbh.de>
  *
  * extend by Broadcom at Jan, 2019 and change to ebt_u32
  */


#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <getopt.h>
#include <netdb.h>
#include <stdbool.h>
#include "../include/ebtables_u.h"
#include <include/uapi/linux/netfilter_bridge/ebt_u32.h>


#define OPT_U32 '1'

static const struct option opts[] =
{
	{ "u32"           , required_argument, 0, OPT_U32 },
	{ 0 }
};

/**
 * xtables_strtou{i,l} - string to number conversion
 * @s:	input string
 * @end:	like strtoul's "end" pointer
 * @value:	pointer for result
 * @min:	minimum accepted value
 * @max:	maximum accepted value
 *
 * If @end is NULL, we assume the caller wants a "strict strtoul", and hence
 * "15a" is rejected.
 * In either case, the value obtained is compared for min-max compliance.
 * Base is always 0, i.e. autodetect depending on @s.
 *
 * Returns true/false whether number was accepted. On failure, *value has
 * undefined contents.
 */
static bool ebtables_strtoul(const char *s, char **end, unsigned int *value,
                     unsigned int min, unsigned int max)
{
	unsigned int v;
	const char *p;
	char *my_end;

	errno = 0;
	/* Since strtoul allows leading minus, we have to check for ourself. */
	for (p = s; isspace(*p); ++p)
		;
	if (*p == '-')
		return false;
	v = strtoul(s, &my_end, 0);
	if (my_end == s)
		return false;
	if (end != NULL)
		*end = my_end;

	if (errno != ERANGE && min <= v && (max == 0 || v <= max)) {
		if (value != NULL)
			*value = v;
		if (end == NULL)
			return *my_end == '\0';
		return true;
	}

	return false;
}

static bool ebtables_strtoui(const char *s, char **end, unsigned int *value,
                     unsigned int min, unsigned int max)
{
	unsigned int v;
	bool ret;

	ret = ebtables_strtoul(s, end, &v, min, max);
	if (value != NULL)
		*value = v;
	return ret;
}

static void u32_dump(const struct ebt_u32_info *data)
{
	const struct ebt_u32_test *ct;
	unsigned int testind, i;

	printf(" \"");
	for (testind = 0; testind < data->ntests; ++testind) {
		ct = &data->tests[testind];

		if (testind > 0)
			printf("&&");

		printf("0x%x", ct->location[0].number);
		for (i = 1; i < ct->nnums; ++i) {
			switch (ct->location[i].nextop) {
			case EBT_U32_AND:
				printf("&");
				break;
			case EBT_U32_LEFTSH:
				printf("<<");
				break;
			case EBT_U32_RIGHTSH:
				printf(">>");
				break;
			case EBT_U32_AT:
				printf("@");
				break;
			}
			printf("0x%x", ct->location[i].number);
		}

		printf("=");
		for (i = 0; i < ct->nvalues; ++i) {
			if (i > 0)
				printf(",");
			if (ct->value[i].min == ct->value[i].max)
				printf("0x%x", ct->value[i].min);
			else
				printf("0x%x:0x%x", ct->value[i].min,
				       ct->value[i].max);
		}
	}
	putchar('\"');
	printf(" ");
}



/* string_to_number() is not quite what we need here ... */
static uint32_t parse_number(const char **s, int pos)
{
	unsigned int number;
	char *end;

	if (!ebtables_strtoui(*s, &end, &number, 0, UINT32_MAX) ||
	    end == *s)
		ebt_print_error2("u32: at char %d: not a number or out of range", pos);
	*s = end;
	return number;
}

static void ebt_parse_u32(char *optarg, struct ebt_u32_info * info)
{
	struct ebt_u32_info *data = info;
	unsigned int testind = 0, locind = 0, valind = 0;
	struct ebt_u32_test *ct = &data->tests[testind]; /* current test */
	const char *arg = optarg; /* the argument string */
	const char *start = optarg;
	int state = 0;

	/*
	 * states:
	 * 0 = looking for numbers and operations,
	 * 1 = looking for ranges
	 */
	while (1) {
		/* read next operand/number or range */
		while (isspace(*arg))
			++arg;

		if (*arg == '\0') {
			/* end of argument found */
			if (state == 0)
				ebt_print_error("u32: abrupt end of input after location specifier");
			if (valind == 0)
				ebt_print_error("u32: test ended with no value specified");

			ct->nnums    = locind;
			ct->nvalues  = valind;
			data->ntests = ++testind;

			if (testind > EBT_U32_MAXSIZE)
				ebt_print_error("u32: at char %u: too many \"&&\"s", (unsigned int)(arg - start));

			return;
		}

		if (state == 0) {
			/*
			 * reading location: read a number if nothing read yet,
			 * otherwise either op number or = to end location spec
			 */
			if (*arg == '=') {
				if (locind == 0) {
					ebt_print_error("u32: at char %u: "
					           "location spec missing",
					           (unsigned int)(arg - start));
				} else {
					++arg;
					state = 1;
				}
			} else {
				if (locind != 0) {
					/* need op before number */
					if (*arg == '&') {
						ct->location[locind].nextop = EBT_U32_AND;
					} else if (*arg == '<') {
						if (*++arg != '<')
							ebt_print_error("u32: at char %u: a second '<' was expected", (unsigned int)(arg - start));
						ct->location[locind].nextop = EBT_U32_LEFTSH;
					} else if (*arg == '>') {
						if (*++arg != '>')
							ebt_print_error("u32: at char %u: a second '>' was expected", (unsigned int)(arg - start));
						ct->location[locind].nextop = EBT_U32_RIGHTSH;
					} else if (*arg == '@') {
						ct->location[locind].nextop = EBT_U32_AT;
					} else {
						ebt_print_error("u32: at char %u: operator expected", (unsigned int)(arg - start));
					}
					++arg;
				}
				/* now a number; string_to_number skips white space? */
				ct->location[locind].number =
					parse_number(&arg, arg - start);
				if (++locind > EBT_U32_MAXSIZE)
					ebt_print_error("u32: at char %u: too many operators", (unsigned int)(arg - start));
			}
		} else {
			/*
			 * state 1 - reading values: read a range if nothing
			 * read yet, otherwise either ,range or && to end
			 * test spec
			 */
			if (*arg == '&') {
				if (*++arg != '&')
					ebt_print_error("u32: at char %u: a second '&' was expected", (unsigned int)(arg - start));
				if (valind == 0) {
					ebt_print_error("u32: at char %u: value spec missing", (unsigned int)(arg - start));
				} else {
					ct->nnums   = locind;
					ct->nvalues = valind;
					ct = &data->tests[++testind];
					if (testind > EBT_U32_MAXSIZE)
						ebt_print_error("u32: at char %u: too many \"&&\"s", (unsigned int)(arg - start));
					++arg;
					state  = 0;
					locind = 0;
					valind = 0;
				}
			} else { /* read value range */
				if (valind > 0) { /* need , before number */
					if (*arg != ',')
						ebt_print_error("u32: at char %u: expected \",\" or \"&&\"", (unsigned int)(arg - start));
					++arg;
				}
				ct->value[valind].min =
					parse_number(&arg, arg - start);

				while (isspace(*arg))
					++arg;

				if (*arg == ':') {
					++arg;
					ct->value[valind].max =
						parse_number(&arg, arg-start);
				} else {
					ct->value[valind].max =
						ct->value[valind].min;
				}

				if (++valind > EBT_U32_MAXSIZE)
					ebt_print_error("u32: at char %u: too many \",\"s", (unsigned int)(arg - start));
			}
		}
	}

	printf("\r\nu32  ntest %u, invert %u\r\n", info->ntests, info->invert);
	printf("\r\n test[0]  nnums %u, nvalues %u, "
		"location[0].number %u, location[0].nexhop %u, value[0].min %u, value[0].max %u\r\n",
		info->tests[0].nnums, info->tests[0].nvalues, info->tests[0].location[0].number, info->tests[0].location[0].nextop,
		info->tests[0].value[0].min, info->tests[0].value[0].max);
	
}

static void print_help(void)
{
	printf(
		"u32 match options:\n"
		"--u32 [!] tests\n"
		"\t\t""tests := location \"=\" value | tests \"&&\" location \"=\" value\n"
		"\t\t""value := range | value \",\" range\n"
		"\t\t""range := number | number \":\" number\n"
		"\t\t""location := number | location operator number\n"
		"\t\t""operator := \"&\" | \"<<\" | \">>\" | \"@\"\n");
}


static void init(struct ebt_entry_match *match)
{
	struct ebt_u32_info *u32info = (struct ebt_u32_info *)match->data;

	memset(u32info, 0, sizeof(struct ebt_u32_info));
}

static int parse(int c, char **argv, int argc, const struct ebt_u_entry *entry,
   unsigned int *flags, struct ebt_entry_match **match)
{
	struct ebt_u32_info *u32info = (struct ebt_u32_info *)(*match)->data;

	switch (c) {
	case OPT_U32:
		ebt_check_option2(flags, OPT_U32);
		if (ebt_check_inverse2(optarg)) {
		    u32info->invert = 1;
		}
		ebt_parse_u32(optarg, u32info);
		break;
	default:
		return 0;
	}
	return 1;
}

static void final_check(const struct ebt_u_entry *entry,
   const struct ebt_entry_match *match, const char *name,
   unsigned int hookmask, unsigned int time)
{
}

static void print(const struct ebt_u_entry *entry,
   const struct ebt_entry_match *match)
{
	struct ebt_u32_info *u32info = (struct ebt_u32_info *)match->data;

	printf("--u32 ");
	if (u32info->invert)
		printf("! ");
	u32_dump(u32info);
}

static int compare(const struct ebt_entry_match *m1,
   const struct ebt_entry_match *m2)
{
	struct ebt_u32_info *u32info1 = (struct ebt_u32_info *)m1->data;
	struct ebt_u32_info *u32info2 = (struct ebt_u32_info *)m2->data;
	
	if (0 != memcmp(u32info1, u32info2, sizeof(struct ebt_u32_info)))
		return 0;
	
	return 1;
}

static struct ebt_u_match u32_match =
{
	.name		= EBT_U32_MATCH,
	.size		= sizeof(struct ebt_u32_info),
	.help		= print_help,
	.init		= init,
	.parse		= parse,
	.final_check	= final_check,
	.print		= print,
	.compare	= compare,
	.extra_ops	= opts,
};

static void _INIT(void)
{
	ebt_register_match(&u32_match);
}
