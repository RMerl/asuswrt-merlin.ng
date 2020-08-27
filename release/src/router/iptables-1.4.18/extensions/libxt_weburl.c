/*  weburl --	An iptables extension to match URLs in HTTP(S) requests
 *  		This module can match using string match or regular expressions
 *  		Originally designed for use with Gargoyle router firmware (gargoyle-router.com)
 *
 *
 *  Copyright © 2008-2010 by Eric Bishop <eric@gargoyle-router.com>
 * 
 *  This file is free software: you may copy, redistribute and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation, either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  This file is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

 /* yachang@realtek.com
  * Changed to use xtables_match 
  */

#define _GNU_SOURCE 1 /* strnlen for older glibcs */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <xtables.h>
#include <linux/netfilter/xt_weburl.h>

/* Function which prints out usage message. */
static void weburl_help(void)
{
	printf(	"weburl options:\n  --contains [!] [STRING]\n  --contains_regex [!] [REGEX]\n --matches_exactly [!] [STRING]\n --domain_only\n --path_only\n");
}

	
static const struct xt_option_entry weburl_opts[] = {
	{ .name = "contains", .id = WEBURL_CONTAINS_TYPE, .type = XTTYPE_STRING, 
	  .flags = XTOPT_INVERT, .excl = (1<<WEBURL_REGEX_TYPE)|(1<<WEBURL_EXACT_TYPE) },
	{ .name = "contains_regex", .id = WEBURL_REGEX_TYPE, .type = XTTYPE_STRING, 
	  .flags = XTOPT_INVERT, .excl = (1<<WEBURL_CONTAINS_TYPE)|(1<<WEBURL_EXACT_TYPE) },
	{ .name = "matches_exactly", .id = WEBURL_EXACT_TYPE, .type = XTTYPE_STRING, 
	  .flags = XTOPT_INVERT, .excl = (1<<WEBURL_CONTAINS_TYPE)|(1<<WEBURL_REGEX_TYPE) },
	{ .name = "domain_only", .id = WEBURL_DOMAIN_PART, .type = XTTYPE_NONE },
	{ .name = "path_only", .id = WEBURL_PATH_PART, .type = XTTYPE_NONE },  
	XTOPT_TABLEEND,
};

static void weburl_init(struct xt_entry_match *m)
{
	struct ipt_weburl_info *info = (struct ipt_weburl_info *)m->data;

	info->match_part = WEBURL_ALL_PART;
}

static void weburl_parse(struct xt_option_call *cb)
{
	struct ipt_weburl_info *info = cb->data;
	
	xtables_option_parse(cb);
	switch (cb->entry->id) {
	case WEBURL_CONTAINS_TYPE:
	case WEBURL_REGEX_TYPE:
	case WEBURL_EXACT_TYPE:
		info->match_type = cb->entry->id;
		info->invert = cb->invert ? 1 : 0;
		if (strlen(cb->arg) >= MAX_TEST_STR) {
			xtables_error(PARAMETER_PROBLEM, "STRING too long \"%s\"", cb->arg);
		}
		strncpy(info->test_str, cb->arg, MAX_TEST_STR - 1);
		break;
	case WEBURL_DOMAIN_PART:
	case WEBURL_PATH_PART:
		if (WEBURL_ALL_PART!=info->match_part) {
			xtables_error(PARAMETER_PROBLEM,
				"You may specify at most one part of the url to match:\n\t--domain_only, --path_only or neither (to match full url)\n");			
		}
		info->match_part = cb->entry->id;
		break;
	}
}


static void weburl_check(struct xt_fcheck_call *cb)
{
	#define F_OP_ANY  ((1<<WEBURL_CONTAINS_TYPE)|(1<<WEBURL_REGEX_TYPE)|(1<<WEBURL_EXACT_TYPE))
	if (!(cb->xflags & F_OP_ANY))
		xtables_error(PARAMETER_PROBLEM,
			"You must specify '--contains' or '--contains_regex' or '--matches_exactly's");
}

static void print_weburl_args(	struct ipt_weburl_info* info )
{
	//invert
	if(info->invert > 0)
	{
		printf("! ");
	}
	//match type
	switch (info->match_type)
	{
		case WEBURL_CONTAINS_TYPE:
			printf("--contains ");
			break;
		case WEBURL_REGEX_TYPE:
			printf("--contains_regex ");
			break;
		case WEBURL_EXACT_TYPE:
			printf("--matches_exactly ");
			break;
	}
	//test string
	printf("%s ", info->test_str);

	//match part
	switch(info->match_part)
	{
		case WEBURL_DOMAIN_PART:
			printf("--domain_only ");
			break;
		case WEBURL_PATH_PART:
			printf("--path_only ");
			break;
		case WEBURL_ALL_PART:
			//print nothing
			break;
	}
}

static void weburl_print(const void *ip, const struct xt_entry_match *match, int numeric)
{
	printf(" WEBURL ");
	struct ipt_weburl_info *info = (struct ipt_weburl_info *)match->data;

	print_weburl_args(info);
}

static void weburl_save(const void *ip, const struct xt_entry_match *match)
{
	struct ipt_weburl_info *info = (struct ipt_weburl_info *)match->data;

	print_weburl_args(info);
}


static struct xtables_match weburl_match = {
	.name          = "weburl",
	.revision      = 0,
	.family        = NFPROTO_UNSPEC,
	.version       = XTABLES_VERSION,
	.size          = XT_ALIGN(sizeof(struct ipt_weburl_info)),
	.userspacesize = XT_ALIGN(sizeof(struct ipt_weburl_info)),
	.help          = weburl_help, 
	.init          = weburl_init,	
	.print         = weburl_print,
	.save          = weburl_save,
	.x6_parse      = weburl_parse,
	.x6_fcheck     = weburl_check,
	.x6_options    = weburl_opts,
};

void _init(void)
{
	xtables_register_match(&weburl_match);
}
