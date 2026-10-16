/*
 * ebtables.c, v2.0 July 2002
 *
 * Author: Bart De Schuymer
 *
 *  This code was stongly inspired on the iptables code which is
 *  Copyright (C) 1999 Paul `Rusty' Russell & Michael J. Neuling
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include "config.h"
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <inttypes.h>
#include <signal.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <iptables.h>
#include <xtables.h>

#include <linux/netfilter_bridge.h>
#include <linux/netfilter/nf_tables.h>
#include <libiptc/libxtc.h>
#include "xshared.h"
#include "nft.h"
#include "nft-bridge.h"

static int
delete_entry(struct nft_handle *h,
	     const char *chain,
	     const char *table,
	     struct iptables_command_state *cs,
	     int rule_nr,
	     int rule_nr_end,
	     bool verbose)
{
	int ret = 1;

	if (rule_nr == -1)
		ret = nft_cmd_rule_delete(h, chain, table, cs, verbose);
	else {
		do {
			ret = nft_cmd_rule_delete_num(h, chain, table,
						  rule_nr, verbose);
			rule_nr++;
		} while (rule_nr < rule_nr_end);
	}

	return ret;
}

static int
change_entry_counters(struct nft_handle *h,
		      const char *chain, const char *table,
		      struct iptables_command_state *cs,
		      int rule_nr, int rule_nr_end, uint8_t counter_op,
		      bool verbose)
{
	int ret = 1;

	if (rule_nr == -1)
		return nft_cmd_rule_change_counters(h, chain, table, cs,
						    rule_nr, counter_op,
						    verbose);
	do {
		ret = nft_cmd_rule_change_counters(h, chain, table, cs,
						   rule_nr, counter_op,
						   verbose);
		rule_nr++;
	} while (rule_nr < rule_nr_end);

	return ret;
}

/* Default command line options. Do not mess around with the already
 * assigned numbers unless you know what you are doing */
struct option ebt_original_options[] =
{
	{ "append"         , required_argument, 0, 'A' },
	{ "insert"         , required_argument, 0, 'I' },
	{ "delete"         , required_argument, 0, 'D' },
	{ "list"           , optional_argument, 0, 'L' },
	{ "Lc"             , no_argument      , 0, 17  },
	{ "Ln"             , no_argument      , 0, 18  },
	{ "Lx"             , no_argument      , 0, 19  },
	{ "Lmac2"          , no_argument      , 0, 12  },
	{ "zero"           , optional_argument, 0, 'Z' },
	{ "flush"          , optional_argument, 0, 'F' },
	{ "policy"         , required_argument, 0, 'P' },
	{ "in-interface"   , required_argument, 0, 'i' },
	{ "in-if"          , required_argument, 0, 'i' },
	{ "logical-in"     , required_argument, 0, 15  },
	{ "logical-out"    , required_argument, 0, 16  },
	{ "out-interface"  , required_argument, 0, 'o' },
	{ "out-if"         , required_argument, 0, 'o' },
	{ "version"        , no_argument      , 0, 'V' },
	{ "verbose"        , no_argument      , 0, 'v' },
	{ "help"           , no_argument      , 0, 'h' },
	{ "jump"           , required_argument, 0, 'j' },
	{ "set-counters"   , required_argument, 0, 'c' },
	{ "change-counters", required_argument, 0, 'C' },
	{ "proto"          , required_argument, 0, 'p' },
	{ "protocol"       , required_argument, 0, 'p' },
	{ "db"             , required_argument, 0, 'b' },
	{ "source"         , required_argument, 0, 's' },
	{ "src"            , required_argument, 0, 's' },
	{ "destination"    , required_argument, 0, 'd' },
	{ "dst"            , required_argument, 0, 'd' },
	{ "table"          , required_argument, 0, 't' },
	{ "modprobe"       , required_argument, 0, 'M' },
	{ "new-chain"      , required_argument, 0, 'N' },
	{ "rename-chain"   , required_argument, 0, 'E' },
	{ "delete-chain"   , optional_argument, 0, 'X' },
	{ "init-table"     , no_argument      , 0, 11  },
	{ "concurrent"     , no_argument      , 0, 13  },
	{ "check"          , required_argument, 0, 14  },
	{ 0 }
};

struct xtables_globals ebtables_globals = {
	.option_offset 		= 0,
	.program_version	= PACKAGE_VERSION " (nf_tables)",
	.orig_opts		= ebt_original_options,
	.compat_rev		= nft_compatible_revision,
};

#define prog_name ebtables_globals.program_name
#define prog_vers ebtables_globals.program_version

/* Prints all registered extensions */
static void ebt_list_extensions(const struct xtables_target *t,
				const struct xtables_rule_match *m)
{
	printf("%s v%s\n", prog_name, prog_vers);
	printf("Loaded userspace extensions:\n");
	/*printf("\nLoaded tables:\n");
        while (tbl) {
		printf("%s\n", tbl->name);
                tbl = tbl->next;
	}*/
	printf("\nLoaded targets:\n");
        for (t = xtables_targets; t; t = t->next) {
		printf("%s\n", t->name);
	}
	printf("\nLoaded matches:\n");
        for (; m != NULL; m = m->next)
		printf("%s\n", m->match->name);
	/*printf("\nLoaded watchers:\n");
        while (w) {
		printf("%s\n", w->name);
                w = w->next;
	}*/
}

void nft_bridge_print_help(struct iptables_command_state *cs)
{
	const struct xtables_rule_match *m = cs->matches;
	struct xtables_target *t = cs->target;

	while (optind < cs->argc) {
		/*struct ebt_u_match *m;
		struct ebt_u_watcher *w;*/

		if (!strcasecmp("list_extensions", cs->argv[optind])) {
			ebt_list_extensions(xtables_targets, cs->matches);
			exit(0);
		}
		/*if ((m = ebt_find_match(cs->argv[optind])))
			ebt_add_match(new_entry, m);
		else if ((w = ebt_find_watcher(cs->argv[optind])))
			ebt_add_watcher(new_entry, w);
		else {*/
			if (!(t = xtables_find_target(cs->argv[optind],
						      XTF_TRY_LOAD)))
				xtables_error(PARAMETER_PROBLEM,
					      "Extension '%s' not found",
					      cs->argv[optind]);
			if (cs->options & OPT_JUMP)
				xtables_error(PARAMETER_PROBLEM,
					      "Sorry, you can only see help for one target extension at a time");
			cs->options |= OPT_JUMP;
			cs->target = t;
		//}
		optind++;
	}

	printf("%s %s\n", prog_name, prog_vers);
	printf(
"Usage:\n"
"ebtables -[ADI] chain rule-specification [options]\n"
"ebtables -P chain target\n"
"ebtables -[LFZ] [chain]\n"
"ebtables -[NX] [chain]\n"
"ebtables -E old-chain-name new-chain-name\n\n"
"Commands:\n"
"--append -A chain             : append to chain\n"
"--delete -D chain             : delete matching rule from chain\n"
"--delete -D chain rulenum     : delete rule at position rulenum from chain\n"
"--change-counters -C chain\n"
"          [rulenum] pcnt bcnt : change counters of existing rule\n"
"--insert -I chain rulenum     : insert rule at position rulenum in chain\n"
"--list   -L [chain]           : list the rules in a chain or in all chains\n"
"--flush  -F [chain]           : delete all rules in chain or in all chains\n"
"--init-table                  : replace the kernel table with the initial table\n"
"--zero   -Z [chain]           : put counters on zero in chain or in all chains\n"
"--policy -P chain target      : change policy on chain to target\n"
"--new-chain -N chain          : create a user defined chain\n"
"--rename-chain -E old new     : rename a chain\n"
"--delete-chain -X [chain]     : delete a user defined chain\n"
"Options:\n"
"[!] --proto  -p proto         : protocol hexadecimal, by name or LENGTH\n"
"[!] --src    -s address[/mask]: source mac address\n"
"[!] --dst    -d address[/mask]: destination mac address\n"
"[!] --in-if  -i name[+]       : network input interface name\n"
"[!] --out-if -o name[+]       : network output interface name\n"
"[!] --logical-in  name[+]     : logical bridge input interface name\n"
"[!] --logical-out name[+]     : logical bridge output interface name\n"
"--set-counters -c chain\n"
"          pcnt bcnt           : set the counters of the to be added rule\n"
"--modprobe -M program         : try to insert modules using this program\n"
"--concurrent                  : use a file lock to support concurrent scripts\n"
"--verbose -v                  : verbose mode\n"
"--version -V                  : print package version\n\n"
"Environment variable:\n"
/*ATOMIC_ENV_VARIABLE "          : if set <FILE> (see above) will equal its value"*/
"\n\n");
	for (; m != NULL; m = m->next) {
		printf("\n");
		m->match->help();
	}
	if (t != NULL) {
		printf("\n");
		t->help();
	}
}

/* Execute command L */
static int list_rules(struct nft_handle *h, const char *chain, const char *table,
		      int rule_nr, int verbose, int numeric, int expanded,
		      int linenumbers, int counters)
{
	unsigned int format;

	format = FMT_OPTIONS | FMT_C_COUNTS;
	if (verbose)
		format |= FMT_VIA;

	if (numeric)
		format |= FMT_NUMERIC;

	if (!expanded)
		format |= FMT_KILOMEGAGIGA;

	if (linenumbers)
		format |= FMT_LINENUMBERS;

	if (!counters)
		format |= FMT_NOCOUNTS;

	return nft_cmd_rule_list(h, chain, table, rule_nr, format);
}

/* This code is very similar to iptables/xtables.c:command_match() */
static void ebt_load_match(const char *name)
{
	struct option *opts = xt_params->opts;
	struct xtables_match *m;
	size_t size;

	m = xtables_find_match(name, XTF_TRY_LOAD, NULL);
	if (m == NULL) {
		fprintf(stderr, "Unable to load %s match\n", name);
		return;
	}

	size = XT_ALIGN(sizeof(struct xt_entry_match)) + m->size;
	m->m = xtables_calloc(1, size);
	m->m->u.match_size = size;
	strcpy(m->m->u.user.name, m->name);
	m->m->u.user.revision = m->revision;
	xs_init_match(m);

	if (m->x6_options != NULL)
		opts = xtables_options_xfrm(xt_params->orig_opts, opts,
					    m->x6_options, &m->option_offset);
	else if (m->extra_opts != NULL)
		opts = xtables_merge_options(xt_params->orig_opts, opts,
					     m->extra_opts, &m->option_offset);
	else
		return;

	if (opts == NULL)
		xtables_error(OTHER_PROBLEM, "Can't alloc memory");
	xt_params->opts = opts;
}

static void ebt_load_watcher(const char *name)
{
	struct option *opts = xt_params->opts;
	struct xtables_target *watcher;
	size_t size;

	watcher = xtables_find_target(name, XTF_TRY_LOAD);
	if (!watcher) {
		fprintf(stderr, "Unable to load %s watcher\n", name);
		return;
	}

	size = XT_ALIGN(sizeof(struct xt_entry_target)) + watcher->size;

	watcher->t = xtables_calloc(1, size);
	watcher->t->u.target_size = size;
	snprintf(watcher->t->u.user.name,
		sizeof(watcher->t->u.user.name), "%s", name);
	watcher->t->u.user.name[sizeof(watcher->t->u.user.name)-1] = '\0';
	watcher->t->u.user.revision = watcher->revision;

	xs_init_target(watcher);

	if (watcher->x6_options != NULL)
		opts = xtables_options_xfrm(xt_params->orig_opts, opts,
					    watcher->x6_options,
					    &watcher->option_offset);
	else if (watcher->extra_opts != NULL)
		opts = xtables_merge_options(xt_params->orig_opts, opts,
					     watcher->extra_opts,
					     &watcher->option_offset);
	else
		return;

	if (opts == NULL)
		xtables_error(OTHER_PROBLEM, "Can't alloc memory");
	xt_params->opts = opts;
}

static void ebt_load_match_extensions(void)
{
	ebt_load_match("802_3");
	ebt_load_match("arp");
	ebt_load_match("ip");
	ebt_load_match("ip6");
	ebt_load_match("mark_m");
	ebt_load_match("limit");
	ebt_load_match("pkttype");
	ebt_load_match("vlan");
	ebt_load_match("stp");
	ebt_load_match("among");

	ebt_load_watcher("log");
	ebt_load_watcher("nflog");
}

struct xtables_match *ebt_add_match(struct xtables_match *m,
				    struct iptables_command_state *cs)
{
	struct xtables_rule_match **rule_matches = &cs->matches;
	struct ebt_match *newnode, **matchp;
	struct xtables_match *newm;

	newm = xtables_find_match(m->name, XTF_LOAD_MUST_SUCCEED, rule_matches);
	if (newm == NULL)
		xtables_error(OTHER_PROBLEM,
			      "Unable to add match %s", m->name);

	newm->m = xtables_calloc(1, m->m->u.match_size);
	memcpy(newm->m, m->m, m->m->u.match_size);
	xs_init_match(newm);

	/* glue code for watchers */
	newnode = xtables_calloc(1, sizeof(struct ebt_match));
	newnode->ismatch = true;
	newnode->u.match = newm;

	for (matchp = &cs->match_list; *matchp; matchp = &(*matchp)->next)
		;
	*matchp = newnode;

	return newm;
}

struct xtables_target *ebt_add_watcher(struct xtables_target *watcher,
				       struct iptables_command_state *cs)
{
	struct ebt_match *newnode, **matchp;
	struct xtables_target *clone;

	clone = xtables_malloc(sizeof(struct xtables_target));
	memcpy(clone, watcher, sizeof(struct xtables_target));
	clone->next = clone;
	clone->udata = NULL;
	xs_init_target(clone);

	clone->t = xtables_calloc(1, watcher->t->u.target_size);
	memcpy(clone->t, watcher->t, watcher->t->u.target_size);


	newnode = xtables_calloc(1, sizeof(struct ebt_match));
	newnode->u.watcher = clone;

	for (matchp = &cs->match_list; *matchp; matchp = &(*matchp)->next)
		;
	*matchp = newnode;

	return clone;
}

int ebt_command_default(struct iptables_command_state *cs,
			struct xtables_globals *unused, bool ebt_invert)
{
	struct xtables_target *t = cs->target;
	struct xtables_match *m;
	struct ebt_match *matchp;

	/* Is it a target option? */
	if (cs->target != NULL &&
	    (cs->target->parse != NULL || cs->target->x6_parse != NULL) &&
	    cs->c >= cs->target->option_offset &&
	    cs->c < cs->target->option_offset + XT_OPTION_OFFSET_SCALE) {
		xtables_option_tpcall(cs->c, cs->argv, ebt_invert,
				      cs->target, &cs->eb);
		return 0;
	}

	/* check previously added matches/watchers to this rule first */
	for (matchp = cs->match_list; matchp; matchp = matchp->next) {
		if (matchp->ismatch) {
			m = matchp->u.match;
			if (!m->parse && !m->x6_parse)
				continue;
			if (cs->c < m->option_offset ||
			    cs->c >= m->option_offset + XT_OPTION_OFFSET_SCALE)
				continue;
			xtables_option_mpcall(cs->c, cs->argv, ebt_invert,
					      m, &cs->eb);
			return 0;
		} else {
			t = matchp->u.watcher;
			if (!t->parse && !t->x6_parse)
				continue;
			if (cs->c < t->option_offset ||
			    cs->c >= t->option_offset + XT_OPTION_OFFSET_SCALE)
				continue;
			xtables_option_tpcall(cs->c, cs->argv, ebt_invert,
					      t, &cs->eb);
			return 0;
		}
	}

	/* Is it a match_option? */
	for (m = xtables_matches; m; m = m->next) {
		if (!m->parse && !m->x6_parse)
			continue;
		if (cs->c < m->option_offset ||
		    cs->c >= m->option_offset + XT_OPTION_OFFSET_SCALE)
			continue;
		m = ebt_add_match(m, cs);
		xtables_option_mpcall(cs->c, cs->argv, ebt_invert, m, &cs->eb);
		return 0;
	}

	/* Is it a watcher option? */
	for (t = xtables_targets; t; t = t->next) {
		if (!(t->ext_flags & XTABLES_EXT_WATCHER))
			continue;

		if (!t->parse && !t->x6_parse)
			continue;
		if (cs->c < t->option_offset ||
		    cs->c >= t->option_offset + XT_OPTION_OFFSET_SCALE)
			continue;
		t = ebt_add_watcher(t, cs);
		xtables_option_tpcall(cs->c, cs->argv, ebt_invert, t, &cs->eb);
		return 0;
	}
	if (cs->c == ':')
		xtables_error(PARAMETER_PROBLEM, "option \"%s\" "
		              "requires an argument", cs->argv[optind - 1]);
	if (cs->c == '?') {
		char optoptstr[3] = {'-', optopt, '\0'};

		xtables_error(PARAMETER_PROBLEM, "unknown option \"%s\"",
			      optopt ? optoptstr : cs->argv[optind - 1]);
	}
	xtables_error(PARAMETER_PROBLEM, "Unknown arg \"%s\"", optarg);
}

int nft_init_eb(struct nft_handle *h, const char *pname)
{
	ebtables_globals.program_name = pname;
	if (xtables_init_all(&ebtables_globals, NFPROTO_BRIDGE) < 0) {
		fprintf(stderr, "%s/%s Failed to initialize ebtables-compat\n",
			ebtables_globals.program_name,
			ebtables_globals.program_version);
		exit(1);
	}
	init_extensions();
	init_extensionsb();

	if (nft_init(h, NFPROTO_BRIDGE) < 0)
		xtables_error(OTHER_PROBLEM,
			      "Could not initialize nftables layer.");

	/* manually registering ebt matches, given the original ebtables parser
	 * don't use '-m matchname' and the match can't be loaded dynamically when
	 * the user calls it.
	 */
	ebt_load_match_extensions();

	return 0;
}

void nft_fini_eb(struct nft_handle *h)
{
	struct xtables_match *match;
	struct xtables_target *target;

	for (match = xtables_matches; match; match = match->next) {
		free(match->m);
	}
	for (target = xtables_targets; target; target = target->next) {
		free(target->t);
	}

	free(xt_params->opts);

	nft_fini(h);
	xtables_fini();
}

int do_commandeb(struct nft_handle *h, int argc, char *argv[], char **table,
		 bool restore)
{
	struct iptables_command_state cs = {
		.argc = argc,
		.argv = argv,
		.jumpto	= "",
	};
	const struct builtin_table *t;
	struct xtables_args args = {
		.family	= h->family,
	};
	struct xt_cmd_parse p = {
		.table		= *table,
		.restore	= restore,
		.line		= line,
		.rule_ranges	= true,
		.ops		= &h->ops->cmd_parse,
	};
	int ret = 0;

	if (h->ops->init_cs)
		h->ops->init_cs(&cs);

	do_parse(argc, argv, &p, &cs, &args);

	h->verbose	= p.verbose;

	t = nft_table_builtin_find(h, p.table);
	if (!t)
		xtables_error(VERSION_PROBLEM,
			      "table '%s' does not exist", p.table);

	switch (p.command) {
	case CMD_NEW_CHAIN:
	case CMD_NEW_CHAIN | CMD_SET_POLICY:
		ret = nft_cmd_chain_user_add(h, p.chain, p.table);
		if (!ret || !(p.command & CMD_SET_POLICY))
			break;
		/* fall through */
	case CMD_SET_POLICY:
		if (!nft_chain_builtin_find(t, p.chain)) {
			ret = ebt_cmd_user_chain_policy(h, p.table, p.chain,
							p.policy);
			break;
		}
		if (strcmp(p.policy, "RETURN") == 0) {
			xtables_error(PARAMETER_PROBLEM,
				      "Policy RETURN only allowed for user defined chains");
		}
		ret = nft_cmd_chain_set(h, p.table, p.chain, p.policy, NULL);
		if (ret < 0)
			xtables_error(PARAMETER_PROBLEM, "Wrong policy");
		break;
	case CMD_LIST:
	case CMD_LIST | CMD_ZERO:
	case CMD_LIST | CMD_ZERO_NUM:
	case CMD_LIST_RULES:
	case CMD_LIST_RULES | CMD_ZERO:
	case CMD_LIST_RULES | CMD_ZERO_NUM:
		if (p.command & CMD_LIST)
			ret = list_rules(h, p.chain, p.table, p.rulenum,
					 cs.options & OPT_VERBOSE,
					 0,
					 /*cs.options&OPT_EXPANDED*/0,
					 cs.options&OPT_LINENUMBERS,
					 cs.options&OPT_LIST_C);
		else if (p.command & CMD_LIST_RULES)
			ret = nft_cmd_rule_list_save(h, p.chain, p.table,
						     p.rulenum,
						     cs.options & OPT_VERBOSE);
		if (ret && (p.command & CMD_ZERO))
			ret = nft_cmd_chain_zero_counters(h, p.chain, p.table,
							  cs.options & OPT_VERBOSE);
		if (ret && (p.command & CMD_ZERO_NUM))
			ret = nft_cmd_rule_zero_counters(h, p.chain, p.table,
							 p.rulenum - 1);
		break;
	case CMD_ZERO:
		ret = nft_cmd_chain_zero_counters(h, p.chain, p.table,
						  cs.options & OPT_VERBOSE);
		break;
	case CMD_ZERO_NUM:
		ret = nft_cmd_rule_zero_counters(h, p.chain, p.table,
						 p.rulenum - 1);
		break;
	case CMD_FLUSH:
		ret = nft_cmd_rule_flush(h, p.chain, p.table,
					 cs.options & OPT_VERBOSE);
		break;
	case CMD_APPEND:
		ret = nft_cmd_rule_append(h, p.chain, p.table, &cs,
					  cs.options & OPT_VERBOSE);
		break;
	case CMD_INSERT:
		ret = nft_cmd_rule_insert(h, p.chain, p.table, &cs,
					  p.rulenum - 1,
					  cs.options & OPT_VERBOSE);
		break;
	case CMD_DELETE:
	case CMD_DELETE_NUM:
		ret = delete_entry(h, p.chain, p.table, &cs, p.rulenum - 1,
				   p.rulenum_end, cs.options & OPT_VERBOSE);
		break;
	case CMD_DELETE_CHAIN:
		ret = nft_cmd_chain_del(h, p.chain, p.table, 0);
		break;
	case CMD_RENAME_CHAIN:
		ret = nft_cmd_chain_user_rename(h, p.chain, p.table, p.newname);
		break;
	case CMD_INIT_TABLE:
		ret = nft_cmd_table_flush(h, p.table, false);
		break;
	case CMD_CHECK:
		ret = nft_cmd_rule_check(h, p.chain, p.table,
					 &cs, cs.options & OPT_VERBOSE);
		break;
	case CMD_CHANGE_COUNTERS:
		ret = change_entry_counters(h, p.chain, p.table, &cs,
					    p.rulenum - 1, p.rulenum_end,
					    args.counter_op,
					    cs.options & OPT_VERBOSE);
		break;
	case CMD_REPLACE:
		ret = nft_cmd_rule_replace(h, p.chain, p.table, &cs,
					   p.rulenum - 1,
					   cs.options & OPT_VERBOSE);
		break;
	default:
		/* We should never reach this... */
		exit_tryhelp(2, line);
	}

	ebt_cs_clean(&cs);
	return ret;
}
