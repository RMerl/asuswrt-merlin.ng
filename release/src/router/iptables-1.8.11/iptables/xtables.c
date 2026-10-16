/* Code to take an iptables-style command line and do it. */

/*
 * Author: Paul.Russell@rustcorp.com.au and mneuling@radlogic.com.au
 *
 * (C) 2000-2002 by the netfilter coreteam <coreteam@netfilter.org>:
 *		    Paul 'Rusty' Russell <rusty@rustcorp.com.au>
 *		    Marc Boucher <marc+nf@mbsi.ca>
 *		    James Morris <jmorris@intercode.com.au>
 *		    Harald Welte <laforge@gnumonks.org>
 *		    Jozsef Kadlecsik <kadlec@blackhole.kfki.hu>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include "config.h"
#include <getopt.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <limits.h>
#include <unistd.h>
#include <netinet/ether.h>
#include <iptables.h>
#include <xtables.h>
#include <fcntl.h>
#include "xshared.h"
#include "nft-shared.h"
#include "nft.h"

static struct option original_opts[] = {
	{.name = "append",	  .has_arg = 1, .val = 'A'},
	{.name = "delete",	  .has_arg = 1, .val = 'D'},
	{.name = "check",	  .has_arg = 1, .val = 'C'},
	{.name = "insert",	  .has_arg = 1, .val = 'I'},
	{.name = "replace",	  .has_arg = 1, .val = 'R'},
	{.name = "list",	  .has_arg = 2, .val = 'L'},
	{.name = "list-rules",	  .has_arg = 2, .val = 'S'},
	{.name = "flush",	  .has_arg = 2, .val = 'F'},
	{.name = "zero",	  .has_arg = 2, .val = 'Z'},
	{.name = "new-chain",	  .has_arg = 1, .val = 'N'},
	{.name = "delete-chain",  .has_arg = 2, .val = 'X'},
	{.name = "rename-chain",  .has_arg = 1, .val = 'E'},
	{.name = "policy",	  .has_arg = 1, .val = 'P'},
	{.name = "source",	  .has_arg = 1, .val = 's'},
	{.name = "destination",   .has_arg = 1, .val = 'd'},
	{.name = "src",		  .has_arg = 1, .val = 's'}, /* synonym */
	{.name = "dst",		  .has_arg = 1, .val = 'd'}, /* synonym */
	{.name = "protocol",	  .has_arg = 1, .val = 'p'},
	{.name = "in-interface",  .has_arg = 1, .val = 'i'},
	{.name = "jump",	  .has_arg = 1, .val = 'j'},
	{.name = "table",	  .has_arg = 1, .val = 't'},
	{.name = "match",	  .has_arg = 1, .val = 'm'},
	{.name = "numeric",	  .has_arg = 0, .val = 'n'},
	{.name = "out-interface", .has_arg = 1, .val = 'o'},
	{.name = "verbose",	  .has_arg = 0, .val = 'v'},
	{.name = "wait",	  .has_arg = 2, .val = 'w'},
	{.name = "wait-interval", .has_arg = 2, .val = 'W'},
	{.name = "exact",	  .has_arg = 0, .val = 'x'},
	{.name = "fragments",	  .has_arg = 0, .val = 'f'},
	{.name = "version",	  .has_arg = 0, .val = 'V'},
	{.name = "help",	  .has_arg = 2, .val = 'h'},
	{.name = "line-numbers",  .has_arg = 0, .val = '0'},
	{.name = "modprobe",	  .has_arg = 1, .val = 'M'},
	{.name = "set-counters",  .has_arg = 1, .val = 'c'},
	{.name = "goto",	  .has_arg = 1, .val = 'g'},
	{.name = "ipv4",	  .has_arg = 0, .val = '4'},
	{.name = "ipv6",	  .has_arg = 0, .val = '6'},
	{NULL},
};

struct xtables_globals xtables_globals = {
	.option_offset = 0,
	.program_version = PACKAGE_VERSION " (nf_tables)",
	.orig_opts = original_opts,
	.compat_rev = nft_compatible_revision,
};

/*
 *	All functions starting with "parse" should succeed, otherwise
 *	the program fails.
 *	Most routines return pointers to static data that may change
 *	between calls to the same or other routines with a few exceptions:
 *	"host_to_addr", "parse_hostnetwork", and "parse_hostnetworkmask"
 *	return global static data.
*/

/* Christophe Burki wants `-p 6' to imply `-m tcp'.  */

static int
list_entries(struct nft_handle *h, const char *chain, const char *table,
	     int rulenum, int verbose, int numeric, int expanded,
	     int linenumbers)
{
	unsigned int format;

	format = FMT_OPTIONS;
	if (!verbose)
		format |= FMT_NOCOUNTS;
	else
		format |= FMT_VIA;

	if (numeric)
		format |= FMT_NUMERIC;

	if (!expanded)
		format |= FMT_KILOMEGAGIGA;

	if (linenumbers)
		format |= FMT_LINENUMBERS;

	return nft_cmd_rule_list(h, chain, table, rulenum, format);
}

static int
list_rules(struct nft_handle *h, const char *chain, const char *table,
	   int rulenum, int counters)
{
	if (counters)
	    counters = -1;		/* iptables -c format */

	return nft_cmd_rule_list_save(h, chain, table, rulenum, counters);
}

int do_commandx(struct nft_handle *h, int argc, char *argv[], char **table,
		bool restore)
{
	int ret = 1;
	struct xt_cmd_parse p = {
		.table		= *table,
		.restore	= restore,
		.line		= line,
		.ops		= &h->ops->cmd_parse,
	};
	struct iptables_command_state cs = {
		.jumpto = "",
		.argv = argv,
	};
	struct xtables_args args = {
		.family = h->family,
	};

	if (h->ops->init_cs)
		h->ops->init_cs(&cs);

	do_parse(argc, argv, &p, &cs, &args);
	h->verbose = p.verbose;

	if (!nft_table_builtin_find(h, p.table))
		xtables_error(VERSION_PROBLEM,
			      "table '%s' does not exist",
			      p.table);
	switch (p.command) {
	case CMD_APPEND:
		ret = h->ops->add_entry(h, p.chain, p.table, &cs, &args,
					cs.options & OPT_VERBOSE, true,
					p.rulenum - 1);
		break;
	case CMD_DELETE:
		ret = h->ops->delete_entry(h, p.chain, p.table, &cs, &args,
					   cs.options & OPT_VERBOSE);
		break;
	case CMD_DELETE_NUM:
		ret = nft_cmd_rule_delete_num(h, p.chain, p.table,
					      p.rulenum - 1, p.verbose);
		break;
	case CMD_CHECK:
		ret = h->ops->check_entry(h, p.chain, p.table, &cs, &args,
					  cs.options & OPT_VERBOSE);
		break;
	case CMD_REPLACE:
		ret = h->ops->replace_entry(h, p.chain, p.table, &cs, &args,
					    cs.options & OPT_VERBOSE,
					    p.rulenum - 1);
		break;
	case CMD_INSERT:
		ret = h->ops->add_entry(h, p.chain, p.table, &cs, &args,
					cs.options & OPT_VERBOSE, false,
					p.rulenum - 1);
		break;
	case CMD_FLUSH:
		ret = nft_cmd_rule_flush(h, p.chain, p.table,
					 cs.options & OPT_VERBOSE);
		break;
	case CMD_ZERO:
		ret = nft_cmd_chain_zero_counters(h, p.chain, p.table,
						  cs.options & OPT_VERBOSE);
		break;
	case CMD_ZERO_NUM:
		ret = nft_cmd_rule_zero_counters(h, p.chain, p.table,
					     p.rulenum - 1);
		break;
	case CMD_LIST:
	case CMD_LIST|CMD_ZERO:
	case CMD_LIST|CMD_ZERO_NUM:
		ret = list_entries(h, p.chain, p.table, p.rulenum,
				   cs.options & OPT_VERBOSE,
				   cs.options & OPT_NUMERIC,
				   cs.options & OPT_EXPANDED,
				   cs.options & OPT_LINENUMBERS);
		if (ret && (p.command & CMD_ZERO)) {
			ret = nft_cmd_chain_zero_counters(h, p.chain, p.table,
						      cs.options & OPT_VERBOSE);
		}
		if (ret && (p.command & CMD_ZERO_NUM)) {
			ret = nft_cmd_rule_zero_counters(h, p.chain, p.table,
						     p.rulenum - 1);
		}
		nft_check_xt_legacy(h->family, false);
		break;
	case CMD_LIST_RULES:
	case CMD_LIST_RULES|CMD_ZERO:
	case CMD_LIST_RULES|CMD_ZERO_NUM:
		ret = list_rules(h, p.chain, p.table, p.rulenum,
				 cs.options & OPT_VERBOSE);
		if (ret && (p.command & CMD_ZERO)) {
			ret = nft_cmd_chain_zero_counters(h, p.chain, p.table,
						      cs.options & OPT_VERBOSE);
		}
		if (ret && (p.command & CMD_ZERO_NUM)) {
			ret = nft_cmd_rule_zero_counters(h, p.chain, p.table,
						     p.rulenum - 1);
		}
		nft_check_xt_legacy(h->family, false);
		break;
	case CMD_NEW_CHAIN:
		ret = nft_cmd_chain_user_add(h, p.chain, p.table);
		break;
	case CMD_DELETE_CHAIN:
		ret = nft_cmd_chain_del(h, p.chain, p.table,
					cs.options & OPT_VERBOSE);
		break;
	case CMD_RENAME_CHAIN:
		ret = nft_cmd_chain_user_rename(h, p.chain, p.table, p.newname);
		break;
	case CMD_SET_POLICY:
		ret = nft_cmd_chain_set(h, p.table, p.chain, p.policy, NULL);
		break;
	case CMD_NONE:
	/* do_parse ignored the line (eg: -4 with ip6tables-restore) */
		break;
	default:
		/* We should never reach this... */
		exit_tryhelp(2, line);
	}

	*table = p.table;

	h->ops->clear_cs(&cs);

	xtables_clear_args(&args);
	xtables_free_opts(1);

	return ret;
}
