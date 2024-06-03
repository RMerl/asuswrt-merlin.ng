/* ebt_string
 *
 * Author:
 * Bernie Harris <bernie.harris@alliedtelesis.co.nz>
 *
 * February, 2018
 *
 * Based on:
 *  libxt_string.c, Copyright (C) 2000 Emmanuel Roger  <winfield@freegates.be>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <getopt.h>
#include <netdb.h>
#include <ctype.h>
#include "../include/ebtables_u.h"
#include <linux/if_packet.h>
#include <linux/netfilter/xt_string.h>

#define STRING_FROM  '1'
#define STRING_TO    '2'
#define STRING_ALGO  '3'
#define STRING_ICASE '4'
#define STRING       '5'
#define STRING_HEX   '6'
#define OPT_STRING_FROM  (1 << 0)
#define OPT_STRING_TO    (1 << 1)
#define OPT_STRING_ALGO  (1 << 2)
#define OPT_STRING_ICASE (1 << 3)
#define OPT_STRING       (1 << 4)
#define OPT_STRING_HEX   (1 << 5)

static const struct option opts[] =
{
	{ "string-from"             , required_argument, 0, STRING_FROM },
	{ "string-to"               , required_argument, 0, STRING_TO },
	{ "string-algo"             , required_argument, 0, STRING_ALGO },
	{ "string-icase"            , no_argument,       0, STRING_ICASE },
	{ "string"                  , required_argument, 0, STRING },
	{ "string-hex"              , required_argument, 0, STRING_HEX },
	{ 0 }
};

static void print_help()
{
	printf(
"string options:\n"
"--string-from offset    : Offset to start searching from (default: 0)\n"
"--string-to   offset    : Offset to stop searching (default: packet size)\n"
"--string-algo algorithm : Algorithm (bm = Boyer-Moore, kmp = Knuth-Pratt-Morris)\n"
"--string-icase          : Ignore case when searching\n"
"--string     [!] string : Match a string in a packet\n"
"--string-hex [!] string : Match a hex string in a packet, e.g. |0D 0A|, |0D0A|, netfilter|03|org\n");
}

static void init(struct ebt_entry_match *match)
{
	struct xt_string_info *info = (struct xt_string_info *)match->data;

	info->to_offset = UINT16_MAX;
}

static void parse_string(const char *s, struct xt_string_info *info)
{
	/* xt_string does not need \0 at the end of the pattern */
	if (strlen(s) <= XT_STRING_MAX_PATTERN_SIZE) {
		strncpy(info->pattern, s, XT_STRING_MAX_PATTERN_SIZE);
		info->patlen = strnlen(s, XT_STRING_MAX_PATTERN_SIZE);
		return;
	}
	ebt_print_error3("STRING too long \"%s\"", s);
}

static void parse_hex_string(const char *s, struct xt_string_info *info)
{
	int i=0, slen, sindex=0, schar;
	short hex_f = 0, literal_f = 0;
	char hextmp[3];

	slen = strlen(s);

	if (slen == 0) {
		ebt_print_error3("STRING must contain at least one char");
	}

	while (i < slen) {
		if (s[i] == '\\' && !hex_f) {
			literal_f = 1;
		} else if (s[i] == '\\') {
			ebt_print_error3("Cannot include literals in hex data");
		} else if (s[i] == '|') {
			if (hex_f)
				hex_f = 0;
			else {
				hex_f = 1;
				/* get past any initial whitespace just after the '|' */
				while (s[i+1] == ' ')
					i++;
			}
			if (i+1 >= slen)
				break;
			else
				i++;  /* advance to the next character */
		}

		if (literal_f) {
			if (i+1 >= slen) {
				ebt_print_error3("Bad literal placement at end of string");
			}
			info->pattern[sindex] = s[i+1];
			i += 2;  /* skip over literal char */
			literal_f = 0;
		} else if (hex_f) {
			if (i+1 >= slen) {
				ebt_print_error3("Odd number of hex digits");
			}
			if (i+2 >= slen) {
				/* must end with a "|" */
				ebt_print_error3("Invalid hex block");
			}
			if (! isxdigit(s[i])) /* check for valid hex char */
				ebt_print_error3("Invalid hex char '%c'", s[i]);
			if (! isxdigit(s[i+1])) /* check for valid hex char */
				ebt_print_error3("Invalid hex char '%c'", s[i+1]);
			hextmp[0] = s[i];
			hextmp[1] = s[i+1];
			hextmp[2] = '\0';
			if (! sscanf(hextmp, "%x", &schar))
				ebt_print_error3("Invalid hex char `%c'", s[i]);
			info->pattern[sindex] = (char) schar;
			if (s[i+2] == ' ')
				i += 3;  /* spaces included in the hex block */
			else
				i += 2;
		} else {  /* the char is not part of hex data, so just copy */
			info->pattern[sindex] = s[i];
			i++;
		}
		if (sindex > XT_STRING_MAX_PATTERN_SIZE)
			ebt_print_error3("STRING too long \"%s\"", s);
		sindex++;
	}
	info->patlen = sindex;
}

static int parse(int c, char **argv, int argc, const struct ebt_u_entry *entry,
		 unsigned int *flags, struct ebt_entry_match **match)
{
	struct xt_string_info *info = (struct xt_string_info *)(*match)->data;

	switch (c) {
	case STRING_FROM:
		ebt_check_option2(flags, OPT_STRING_FROM);
		if (ebt_check_inverse2(optarg))
			ebt_print_error2("Unexpected `!' after --string-from");
		info->from_offset = (__u16)strtoul(optarg, NULL, 10);
		break;
	case STRING_TO:
		ebt_check_option2(flags, OPT_STRING_TO);
		if (ebt_check_inverse2(optarg))
			ebt_print_error2("Unexpected `!' after --string-to");
		info->to_offset = (__u16)strtoul(optarg, NULL, 10);
		break;
	case STRING_ALGO:
		ebt_check_option2(flags, OPT_STRING_ALGO);
		if (ebt_check_inverse2(optarg))
			ebt_print_error2("Unexpected `!' after --string-algo");
		if (snprintf(info->algo, sizeof(info->algo), "%s", optarg) >=
				sizeof(info->algo))
			ebt_print_error2("\"%s\" is truncated", info->algo);
		break;
	case STRING_ICASE:
		ebt_check_option2(flags, OPT_STRING_ICASE);
		if (ebt_check_inverse2(optarg))
			ebt_print_error2("Unexpected `!' after --string-icase");
		info->u.v1.flags |= XT_STRING_FLAG_IGNORECASE;
		break;
	case STRING:
		ebt_check_option2(flags, OPT_STRING);
		parse_string(optarg, info);
		if (ebt_check_inverse2(optarg)) {
			info->u.v1.flags |= XT_STRING_FLAG_INVERT;
		}
		break;
	case STRING_HEX:
		ebt_check_option2(flags, OPT_STRING_HEX);
		parse_hex_string(optarg, info);
		if (ebt_check_inverse2(optarg)) {
			info->u.v1.flags |= XT_STRING_FLAG_INVERT;
		}
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
	struct xt_string_info *info = (struct xt_string_info *)match->data;

	if (info->to_offset < info->from_offset) {
		ebt_print_error3("'to' offset should not be less than 'from' "
				 "offset");
	}
}

/* Test to see if the string contains non-printable chars or quotes */
static unsigned short int is_hex_string(const char *str,
					const unsigned short int len)
{
	unsigned int i;
	for (i=0; i < len; i++) {
		if (! isprint(str[i])) {
			/* string contains at least one non-printable char */
			return 1;
		}
	}
	/* use hex output if the last char is a "\" */
	if (str[len-1] == '\\')
		return 1;
	return 0;
}

/* Print string with "|" chars included as one would pass to --string-hex */
static void print_hex_string(const char *str, const unsigned short int len)
{
	unsigned int i;
	/* start hex block */
	printf("\"|");
	for (i=0; i < len; i++)
		printf("%02x", (unsigned char)str[i]);
	/* close hex block */
	printf("|\" ");
}

static void print_string(const char *str, const unsigned short int len)
{
	unsigned int i;
	printf("\"");
	for (i=0; i < len; i++) {
		if (str[i] == '\"' || str[i] == '\\')
			putchar('\\');
		printf("%c", (unsigned char) str[i]);
	}
	printf("\" ");  /* closing quote */
}

static void print(const struct ebt_u_entry *entry,
		  const struct ebt_entry_match *match)
{
	const struct xt_string_info *info =
		(const struct xt_string_info *) match->data;
	int invert = info->u.v1.flags & XT_STRING_FLAG_INVERT;

	if (is_hex_string(info->pattern, info->patlen)) {
		printf("--string-hex %s", invert ? "! " : "");
		print_hex_string(info->pattern, info->patlen);
	} else {
		printf("--string %s", invert ? "! " : "");
		print_string(info->pattern, info->patlen);
	}
	printf("--string-algo %s ", info->algo);
	if (info->from_offset != 0)
		printf("--string-from %u ", info->from_offset);
	if (info->to_offset != 0)
		printf("--string-to %u ", info->to_offset);
	if (info->u.v1.flags & XT_STRING_FLAG_IGNORECASE)
		printf("--string-icase ");
}

static int compare(const struct ebt_entry_match *m1,
		   const struct ebt_entry_match *m2)
{
	const struct xt_string_info *info1 =
		(const struct xt_string_info *) m1->data;
	const struct xt_string_info *info2 =
		(const struct xt_string_info *) m2->data;

	if (info1->from_offset != info2->from_offset)
		return 0;
	if (info1->to_offset != info2->to_offset)
		return 0;
	if (info1->u.v1.flags != info2->u.v1.flags)
		return 0;
	if (info1->patlen != info2->patlen)
		return 0;
	if (strncmp (info1->algo, info2->algo, XT_STRING_MAX_ALGO_NAME_SIZE) != 0)
		return 0;
	if (strncmp (info1->pattern, info2->pattern, info1->patlen) != 0)
		return 0;

	return 1;
}

static struct ebt_u_match string_match =
{
	.name		= "string",
	.revision	= 1,
	.size		= sizeof(struct xt_string_info),
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
	ebt_register_match(&string_match);
}
