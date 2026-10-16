/* Code to take an ip6tables-style command line and do it. */

/*
 * Author: Paul.Russell@rustcorp.com.au and mneuling@radlogic.com.au
 *
 * (C) 2000-2002 by the netfilter coreteam <coreteam@netfilter.org>:
 * 		    Paul 'Rusty' Russell <rusty@rustcorp.com.au>
 * 		    Marc Boucher <marc+nf@mbsi.ca>
 * 		    James Morris <jmorris@intercode.com.au>
 * 		    Harald Welte <laforge@gnumonks.org>
 * 		    Jozsef Kadlecsik <kadlec@blackhole.kfki.hu>
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
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <limits.h>
#include <ip6tables.h>
#include <xtables.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "ip6tables-multi.h"
#include "xshared.h"

static const char unsupported_rev[] = " [unsupported revision]";

static struct option original_opts[] = {
	{.name = "append",        .has_arg = 1, .val = 'A'},
	{.name = "delete",        .has_arg = 1, .val = 'D'},
	{.name = "check" ,        .has_arg = 1, .val = 'C'},
	{.name = "insert",        .has_arg = 1, .val = 'I'},
	{.name = "replace",       .has_arg = 1, .val = 'R'},
	{.name = "list",          .has_arg = 2, .val = 'L'},
	{.name = "list-rules",    .has_arg = 2, .val = 'S'},
	{.name = "flush",         .has_arg = 2, .val = 'F'},
	{.name = "zero",          .has_arg = 2, .val = 'Z'},
	{.name = "new-chain",     .has_arg = 1, .val = 'N'},
	{.name = "delete-chain",  .has_arg = 2, .val = 'X'},
	{.name = "rename-chain",  .has_arg = 1, .val = 'E'},
	{.name = "policy",        .has_arg = 1, .val = 'P'},
	{.name = "source",        .has_arg = 1, .val = 's'},
	{.name = "destination",   .has_arg = 1, .val = 'd'},
	{.name = "src",           .has_arg = 1, .val = 's'}, /* synonym */
	{.name = "dst",           .has_arg = 1, .val = 'd'}, /* synonym */
	{.name = "protocol",      .has_arg = 1, .val = 'p'},
	{.name = "in-interface",  .has_arg = 1, .val = 'i'},
	{.name = "jump",          .has_arg = 1, .val = 'j'},
	{.name = "table",         .has_arg = 1, .val = 't'},
	{.name = "match",         .has_arg = 1, .val = 'm'},
	{.name = "numeric",       .has_arg = 0, .val = 'n'},
	{.name = "out-interface", .has_arg = 1, .val = 'o'},
	{.name = "verbose",       .has_arg = 0, .val = 'v'},
	{.name = "wait",          .has_arg = 2, .val = 'w'},
	{.name = "wait-interval", .has_arg = 2, .val = 'W'},
	{.name = "exact",         .has_arg = 0, .val = 'x'},
	{.name = "version",       .has_arg = 0, .val = 'V'},
	{.name = "help",          .has_arg = 2, .val = 'h'},
	{.name = "line-numbers",  .has_arg = 0, .val = '0'},
	{.name = "modprobe",      .has_arg = 1, .val = 'M'},
	{.name = "set-counters",  .has_arg = 1, .val = 'c'},
	{.name = "goto",          .has_arg = 1, .val = 'g'},
	{.name = "ipv4",          .has_arg = 0, .val = '4'},
	{.name = "ipv6",          .has_arg = 0, .val = '6'},
	{NULL},
};

struct xtables_globals ip6tables_globals = {
	.option_offset = 0,
	.program_version = PACKAGE_VERSION " (legacy)",
	.orig_opts = original_opts,
	.compat_rev = xtables_compatible_revision,
};

/*
 *	All functions starting with "parse" should succeed, otherwise
 *	the program fails.
 *	Most routines return pointers to static data that may change
 *	between calls to the same or other routines with a few exceptions:
 *	"host_to_addr", "parse_hostnetwork", and "parse_hostnetworkmask"
 *	return global static data.
*/

static int
print_match(const struct xt_entry_match *m,
	    const struct ip6t_ip6 *ip,
	    int numeric)
{
	const char *name = m->u.user.name;
	const int revision = m->u.user.revision;
	struct xtables_match *match, *mt;

	match = xtables_find_match(name, XTF_TRY_LOAD, NULL);
	if (match) {
		mt = xtables_find_match_revision(name, XTF_TRY_LOAD,
						 match, revision);
		if (mt && mt->print)
			mt->print(ip, m, numeric);
		else if (match->print)
			printf("%s%s ", match->name, unsupported_rev);
		else
			printf("%s ", match->name);

		if (match->next == match)
			free(match);
	} else {
		if (name[0])
			printf("UNKNOWN match `%s' ", name);
	}
	/* Don't stop iterating. */
	return 0;
}

/* e is called `fw' here for historical reasons */
static void
print_firewall(const struct ip6t_entry *fw,
	       const char *targname,
	       unsigned int num,
	       unsigned int format,
	       struct xtc_handle *const handle)
{
	struct xtables_target *target, *tg;
	const struct xt_entry_target *t;

	if (!ip6tc_is_chain(targname, handle))
		target = xtables_find_target(targname, XTF_TRY_LOAD);
	else
		target = xtables_find_target(XT_STANDARD_TARGET,
		         XTF_LOAD_MUST_SUCCEED);

	t = ip6t_get_target((struct ip6t_entry *)fw);

	print_rule_details(num, &fw->counters, targname, fw->ipv6.proto,
			   fw->ipv6.flags, fw->ipv6.invflags, format);

	print_fragment(fw->ipv6.flags, fw->ipv6.invflags, format, true);

	print_ifaces(fw->ipv6.iniface, fw->ipv6.outiface,
		     fw->ipv6.invflags, format);

	print_ipv6_addresses(fw, format);

	if (format & FMT_NOTABLE)
		fputs("  ", stdout);

#ifdef IP6T_F_GOTO
	if(fw->ipv6.flags & IP6T_F_GOTO)
		printf("[goto] ");
#endif

	IP6T_MATCH_ITERATE(fw, print_match, &fw->ipv6, format & FMT_NUMERIC);

	if (target) {
		const int revision = t->u.user.revision;

		tg = xtables_find_target_revision(targname, XTF_TRY_LOAD,
						  target, revision);
		if (tg && tg->print)
			/* Print the target information. */
			tg->print(&fw->ipv6, t, format & FMT_NUMERIC);
		else if (target->print)
			printf(" %s%s", target->name, unsupported_rev);

		if (target->next == target)
			free(target);
	} else if (t->u.target_size != sizeof(*t))
		printf("[%u bytes of unknown target data] ",
		       (unsigned int)(t->u.target_size - sizeof(*t)));

	if (!(format & FMT_NONEWLINE))
		fputc('\n', stdout);
}

static void
print_firewall_line(const struct ip6t_entry *fw,
		    struct xtc_handle *const h)
{
	struct xt_entry_target *t;

	t = ip6t_get_target((struct ip6t_entry *)fw);
	print_firewall(fw, t->u.user.name, 0, FMT_PRINT_RULE, h);
}

static int
append_entry(const xt_chainlabel chain,
	     struct ip6t_entry *fw,
	     unsigned int nsaddrs,
	     const struct in6_addr saddrs[],
	     const struct in6_addr smasks[],
	     unsigned int ndaddrs,
	     const struct in6_addr daddrs[],
	     const struct in6_addr dmasks[],
	     int verbose,
	     struct xtc_handle *handle)
{
	unsigned int i, j;
	int ret = 1;

	for (i = 0; i < nsaddrs; i++) {
		fw->ipv6.src = saddrs[i];
		fw->ipv6.smsk = smasks[i];
		for (j = 0; j < ndaddrs; j++) {
			fw->ipv6.dst = daddrs[j];
			fw->ipv6.dmsk = dmasks[j];
			if (verbose)
				print_firewall_line(fw, handle);
			ret &= ip6tc_append_entry(chain, fw, handle);
		}
	}

	return ret;
}

static int
replace_entry(const xt_chainlabel chain,
	      struct ip6t_entry *fw,
	      unsigned int rulenum,
	      const struct in6_addr *saddr, const struct in6_addr *smask,
	      const struct in6_addr *daddr, const struct in6_addr *dmask,
	      int verbose,
	      struct xtc_handle *handle)
{
	fw->ipv6.src = *saddr;
	fw->ipv6.dst = *daddr;
	fw->ipv6.smsk = *smask;
	fw->ipv6.dmsk = *dmask;

	if (verbose)
		print_firewall_line(fw, handle);
	return ip6tc_replace_entry(chain, fw, rulenum, handle);
}

static int
insert_entry(const xt_chainlabel chain,
	     struct ip6t_entry *fw,
	     unsigned int rulenum,
	     unsigned int nsaddrs,
	     const struct in6_addr saddrs[],
	     const struct in6_addr smasks[],
	     unsigned int ndaddrs,
	     const struct in6_addr daddrs[],
	     const struct in6_addr dmasks[],
	     int verbose,
	     struct xtc_handle *handle)
{
	unsigned int i, j;
	int ret = 1;

	for (i = 0; i < nsaddrs; i++) {
		fw->ipv6.src = saddrs[i];
		fw->ipv6.smsk = smasks[i];
		for (j = 0; j < ndaddrs; j++) {
			fw->ipv6.dst = daddrs[j];
			fw->ipv6.dmsk = dmasks[j];
			if (verbose)
				print_firewall_line(fw, handle);
			ret &= ip6tc_insert_entry(chain, fw, rulenum, handle);
		}
	}

	return ret;
}

static int
delete_entry(const xt_chainlabel chain,
	     struct ip6t_entry *fw,
	     unsigned int nsaddrs,
	     const struct in6_addr saddrs[],
	     const struct in6_addr smasks[],
	     unsigned int ndaddrs,
	     const struct in6_addr daddrs[],
	     const struct in6_addr dmasks[],
	     int verbose,
	     struct xtc_handle *handle,
	     struct xtables_rule_match *matches,
	     const struct xtables_target *target)
{
	unsigned int i, j;
	int ret = 1;
	unsigned char *mask;

	mask = make_delete_mask(matches, target, sizeof(*fw));
	for (i = 0; i < nsaddrs; i++) {
		fw->ipv6.src = saddrs[i];
		fw->ipv6.smsk = smasks[i];
		for (j = 0; j < ndaddrs; j++) {
			fw->ipv6.dst = daddrs[j];
			fw->ipv6.dmsk = dmasks[j];
			if (verbose)
				print_firewall_line(fw, handle);
			ret &= ip6tc_delete_entry(chain, fw, mask, handle);
		}
	}
	free(mask);

	return ret;
}

static int
check_entry(const xt_chainlabel chain, struct ip6t_entry *fw,
	    unsigned int nsaddrs, const struct in6_addr *saddrs,
	    const struct in6_addr *smasks, unsigned int ndaddrs,
	    const struct in6_addr *daddrs, const struct in6_addr *dmasks,
	    bool verbose, struct xtc_handle *handle,
	    struct xtables_rule_match *matches,
	    const struct xtables_target *target)
{
	unsigned int i, j;
	int ret = 1;
	unsigned char *mask;

	mask = make_delete_mask(matches, target, sizeof(*fw));
	for (i = 0; i < nsaddrs; i++) {
		fw->ipv6.src = saddrs[i];
		fw->ipv6.smsk = smasks[i];
		for (j = 0; j < ndaddrs; j++) {
			fw->ipv6.dst = daddrs[j];
			fw->ipv6.dmsk = dmasks[j];
			if (verbose)
				print_firewall_line(fw, handle);
			ret &= ip6tc_check_entry(chain, fw, mask, handle);
		}
	}

	free(mask);
	return ret;
}

int
for_each_chain6(int (*fn)(const xt_chainlabel, int, struct xtc_handle *),
	       int verbose, int builtinstoo, struct xtc_handle *handle)
{
	int ret = 1;
	const char *chain;
	char *chains;
	unsigned int i, chaincount = 0;

	chain = ip6tc_first_chain(handle);
	while (chain) {
		chaincount++;
		chain = ip6tc_next_chain(handle);
	}

	chains = xtables_malloc(sizeof(xt_chainlabel) * chaincount);
	i = 0;
	chain = ip6tc_first_chain(handle);
	while (chain) {
		strcpy(chains + i*sizeof(xt_chainlabel), chain);
		i++;
		chain = ip6tc_next_chain(handle);
	}

	for (i = 0; i < chaincount; i++) {
		if (!builtinstoo
		    && ip6tc_builtin(chains + i*sizeof(xt_chainlabel),
				    handle) == 1)
			continue;
		ret &= fn(chains + i*sizeof(xt_chainlabel), verbose, handle);
	}

	free(chains);
	return ret;
}

int
flush_entries6(const xt_chainlabel chain, int verbose,
	      struct xtc_handle *handle)
{
	if (!chain)
		return for_each_chain6(flush_entries6, verbose, 1, handle);

	if (verbose)
		fprintf(stdout, "Flushing chain `%s'\n", chain);
	return ip6tc_flush_entries(chain, handle);
}

static int
zero_entries(const xt_chainlabel chain, int verbose,
	     struct xtc_handle *handle)
{
	if (!chain)
		return for_each_chain6(zero_entries, verbose, 1, handle);

	if (verbose)
		fprintf(stdout, "Zeroing chain `%s'\n", chain);
	return ip6tc_zero_entries(chain, handle);
}

int
delete_chain6(const xt_chainlabel chain, int verbose,
	     struct xtc_handle *handle)
{
	if (!chain)
		return for_each_chain6(delete_chain6, verbose, 0, handle);

	if (verbose)
		fprintf(stdout, "Deleting chain `%s'\n", chain);
	return ip6tc_delete_chain(chain, handle);
}

static int
list_entries(const xt_chainlabel chain, int rulenum, int verbose, int numeric,
	     int expanded, int linenumbers, struct xtc_handle *handle)
{
	int found = 0;
	unsigned int format;
	const char *this;

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

	for (this = ip6tc_first_chain(handle);
	     this;
	     this = ip6tc_next_chain(handle)) {
		const struct ip6t_entry *i;
		unsigned int num;

		if (chain && strcmp(chain, this) != 0)
			continue;

		if (found) printf("\n");

		if (!rulenum) {
			struct xt_counters counters;
			unsigned int urefs;
			const char *pol;
			int refs = - 1;

			pol = ip6tc_get_policy(this, &counters, handle);
			if (!pol && ip6tc_get_references(&urefs, this, handle))
				refs = urefs;

			print_header(format, this, pol, &counters, refs, 0);
		}
		i = ip6tc_first_rule(this, handle);

		num = 0;
		while (i) {
			num++;
			if (!rulenum || num == rulenum)
				print_firewall(i,
					       ip6tc_get_target(i, handle),
					       num,
					       format,
					       handle);
			i = ip6tc_next_rule(i, handle);
		}
		found = 1;
	}

	errno = ENOENT;
	return found;
}

/* We want this to be readable, so only print out necessary fields.
 * Because that's the kind of world I want to live in.
 */
void print_rule6(const struct ip6t_entry *e,
		       struct xtc_handle *h, const char *chain, int counters)
{
	const struct xt_entry_target *t;
	const char *target_name;

	/* print counters for iptables-save */
	if (counters > 0)
		printf("[%llu:%llu] ", (unsigned long long)e->counters.pcnt, (unsigned long long)e->counters.bcnt);

	/* print chain name */
	printf("-A %s", chain);

	/* Print IP part. */
	save_ipv6_addr('s', &e->ipv6.src, &e->ipv6.smsk,
		       e->ipv6.invflags & IP6T_INV_SRCIP);

	save_ipv6_addr('d', &e->ipv6.dst, &e->ipv6.dmsk,
		       e->ipv6.invflags & IP6T_INV_DSTIP);

	save_rule_details(e->ipv6.iniface, e->ipv6.outiface,
			  e->ipv6.proto, 0, e->ipv6.invflags);

#if 0
	/* not definied in ipv6
	 * FIXME: linux/netfilter_ipv6/ip6_tables: IP6T_INV_FRAG why definied? */
	if (e->ipv6.flags & IPT_F_FRAG)
		printf("%s -f",
		       e->ipv6.invflags & IP6T_INV_FRAG ? " !" : "");
#endif

	if (e->ipv6.flags & IP6T_F_TOS)
		printf("%s -? %d",
		       e->ipv6.invflags & IP6T_INV_TOS ? " !" : "",
		       e->ipv6.tos);

	/* Print matchinfo part */
	if (e->target_offset) {
		IP6T_MATCH_ITERATE(e, print_match_save, &e->ipv6);
	}

	/* print counters for iptables -R */
	if (counters < 0)
		printf(" -c %llu %llu", (unsigned long long)e->counters.pcnt, (unsigned long long)e->counters.bcnt);

	/* Print target name and targinfo part */
	target_name = ip6tc_get_target(e, h);
	t = ip6t_get_target((struct ip6t_entry *)e);
	if (t->u.user.name[0]) {
		const char *name = t->u.user.name;
		const int revision = t->u.user.revision;
		struct xtables_target *target, *tg, *tg2;

		target = xtables_find_target(name, XTF_TRY_LOAD);
		if (!target) {
			fprintf(stderr, "Can't find library for target `%s'\n",
				name);
			exit(1);
		}

		tg = tg2 = xtables_find_target_revision(name, XTF_TRY_LOAD,
							target, revision);
		if (!tg2)
			tg2 = target;
		printf(" -j %s", tg2->alias ? tg2->alias(t) : target_name);

		if (tg && tg->save)
			tg->save(&e->ipv6, t);
		else if (target->save)
			printf(unsupported_rev);
		else {
			/* If the target size is greater than xt_entry_target
			 * there is something to be saved, we just don't know
			 * how to print it */
			if (t->u.target_size !=
			    sizeof(struct xt_entry_target)) {
				fprintf(stderr, "Target `%s' is missing "
						"save function\n",
					name);
				exit(1);
			}
		}
	} else if (target_name && (*target_name != '\0'))
#ifdef IP6T_F_GOTO
		printf(" -%c %s", e->ipv6.flags & IP6T_F_GOTO ? 'g' : 'j', target_name);
#else
		printf(" -j %s", target_name);
#endif

	printf("\n");
}

static int
list_rules(const xt_chainlabel chain, int rulenum, int counters,
	     struct xtc_handle *handle)
{
	const char *this = NULL;
	int found = 0;

	if (counters)
	    counters = -1;		/* iptables -c format */

	/* Dump out chain names first,
	 * thereby preventing dependency conflicts */
	if (!rulenum) for (this = ip6tc_first_chain(handle);
	     this;
	     this = ip6tc_next_chain(handle)) {
		if (chain && strcmp(this, chain) != 0)
			continue;

		if (ip6tc_builtin(this, handle)) {
			struct xt_counters count;
			printf("-P %s %s", this, ip6tc_get_policy(this, &count, handle));
			if (counters)
			    printf(" -c %llu %llu", (unsigned long long)count.pcnt, (unsigned long long)count.bcnt);
			printf("\n");
		} else {
			printf("-N %s\n", this);
		}
	}

	for (this = ip6tc_first_chain(handle);
	     this;
	     this = ip6tc_next_chain(handle)) {
		const struct ip6t_entry *e;
		int num = 0;

		if (chain && strcmp(this, chain) != 0)
			continue;

		/* Dump out rules */
		e = ip6tc_first_rule(this, handle);
		while(e) {
			num++;
			if (!rulenum || num == rulenum)
			    print_rule6(e, handle, this, counters);
			e = ip6tc_next_rule(e, handle);
		}
		found = 1;
	}

	errno = ENOENT;
	return found;
}

static struct ip6t_entry *
generate_entry(const struct ip6t_entry *fw,
	       struct xtables_rule_match *matches,
	       struct xt_entry_target *target)
{
	unsigned int size;
	struct xtables_rule_match *matchp;
	struct ip6t_entry *e;

	size = sizeof(struct ip6t_entry);
	for (matchp = matches; matchp; matchp = matchp->next)
		size += matchp->match->m->u.match_size;

	e = xtables_malloc(size + target->u.target_size);
	*e = *fw;
	e->target_offset = size;
	e->next_offset = size + target->u.target_size;

	size = 0;
	for (matchp = matches; matchp; matchp = matchp->next) {
		memcpy(e->elems + size, matchp->match->m, matchp->match->m->u.match_size);
		size += matchp->match->m->u.match_size;
	}
	memcpy(e->elems + size, target, target->u.target_size);

	return e;
}

int do_command6(int argc, char *argv[], char **table,
		struct xtc_handle **handle, bool restore)
{
	struct xt_cmd_parse_ops cmd_parse_ops = {
		.proto_parse	= ipv6_proto_parse,
		.post_parse	= ipv6_post_parse,
		.option_name	= ip46t_option_name,
		.option_invert	= ip46t_option_invert,
		.command_default = command_default,
		.print_help	= xtables_printhelp,
	};
	struct xt_cmd_parse p = {
		.table		= *table,
		.restore	= restore,
		.line		= line,
		.ops		= &cmd_parse_ops,
	};
	struct iptables_command_state cs = {
		.jumpto	= "",
		.argv	= argv,
	};
	struct xtables_args args = {
		.family = AF_INET6,
	};
	struct ip6t_entry *e = NULL;
	unsigned int nsaddrs = 0, ndaddrs = 0;
	struct in6_addr *saddrs = NULL, *daddrs = NULL;
	struct in6_addr *smasks = NULL, *dmasks = NULL;

	int verbose = 0;
	int wait = 0;
	const char *chain = NULL;
	const char *policy = NULL, *newname = NULL;
	unsigned int rulenum = 0, command = 0;
	int ret = 1;

	do_parse(argc, argv, &p, &cs, &args);

	command		= p.command;
	chain		= p.chain;
	*table		= p.table;
	rulenum		= p.rulenum;
	policy		= p.policy;
	newname		= p.newname;
	verbose		= p.verbose;
	wait		= args.wait;
	nsaddrs		= args.s.naddrs;
	ndaddrs		= args.d.naddrs;
	saddrs		= args.s.addr.v6;
	daddrs		= args.d.addr.v6;
	smasks		= args.s.mask.v6;
	dmasks		= args.d.mask.v6;

	iface_to_mask(cs.fw6.ipv6.iniface, cs.fw6.ipv6.iniface_mask);
	iface_to_mask(cs.fw6.ipv6.outiface, cs.fw6.ipv6.outiface_mask);

	/* Attempt to acquire the xtables lock */
	if (!restore)
		xtables_lock_or_exit(wait);

	/* only allocate handle if we weren't called with a handle */
	if (!*handle)
		*handle = ip6tc_init(*table);

	/* try to insmod the module if iptc_init failed */
	if (!*handle && xtables_load_ko(xtables_modprobe_program, false) != -1)
		*handle = ip6tc_init(*table);

	if (!*handle)
		xtables_error(VERSION_PROBLEM,
			"can't initialize ip6tables table `%s': %s",
			*table, ip6tc_strerror(errno));

	if (command == CMD_APPEND
	    || command == CMD_DELETE
	    || command == CMD_CHECK
	    || command == CMD_INSERT
	    || command == CMD_REPLACE) {
		if (cs.target && ip6tc_is_chain(cs.jumpto, *handle)) {
			fprintf(stderr,
				"Warning: using chain %s, not extension\n",
				cs.jumpto);

			if (cs.target->t)
				free(cs.target->t);

			cs.target = NULL;
		}

		/* If they didn't specify a target, or it's a chain
		   name, use standard. */
		if (!cs.target
		    && (strlen(cs.jumpto) == 0
			|| ip6tc_is_chain(cs.jumpto, *handle))) {
			size_t size;

			cs.target = xtables_find_target(XT_STANDARD_TARGET,
					XTF_LOAD_MUST_SUCCEED);

			size = sizeof(struct xt_entry_target)
				+ cs.target->size;
			cs.target->t = xtables_calloc(1, size);
			cs.target->t->u.target_size = size;
			strcpy(cs.target->t->u.user.name, cs.jumpto);
			xs_init_target(cs.target);
		}

		if (!cs.target) {
			/* It is no chain, and we can't load a plugin.
			 * We cannot know if the plugin is corrupt, non
			 * existent OR if the user just misspelled a
			 * chain.
			 */
#ifdef IP6T_F_GOTO
			if (cs.fw6.ipv6.flags & IP6T_F_GOTO)
				xtables_error(PARAMETER_PROBLEM,
					      "goto '%s' is not a chain",
					      cs.jumpto);
#endif
			xtables_find_target(cs.jumpto, XTF_LOAD_MUST_SUCCEED);
		} else {
			e = generate_entry(&cs.fw6, cs.matches, cs.target->t);
		}
	}

	switch (command) {
	case CMD_APPEND:
		ret = append_entry(chain, e,
				   nsaddrs, saddrs, smasks,
				   ndaddrs, daddrs, dmasks,
				   cs.options&OPT_VERBOSE,
				   *handle);
		break;
	case CMD_DELETE:
		ret = delete_entry(chain, e,
				   nsaddrs, saddrs, smasks,
				   ndaddrs, daddrs, dmasks,
				   cs.options&OPT_VERBOSE,
				   *handle, cs.matches, cs.target);
		break;
	case CMD_DELETE_NUM:
		ret = ip6tc_delete_num_entry(chain, rulenum - 1, *handle);
		break;
	case CMD_CHECK:
		ret = check_entry(chain, e,
				   nsaddrs, saddrs, smasks,
				   ndaddrs, daddrs, dmasks,
				   cs.options&OPT_VERBOSE,
				   *handle, cs.matches, cs.target);
		break;
	case CMD_REPLACE:
		ret = replace_entry(chain, e, rulenum - 1,
				    saddrs, smasks, daddrs, dmasks,
				    cs.options&OPT_VERBOSE, *handle);
		break;
	case CMD_INSERT:
		ret = insert_entry(chain, e, rulenum - 1,
				   nsaddrs, saddrs, smasks,
				   ndaddrs, daddrs, dmasks,
				   cs.options&OPT_VERBOSE,
				   *handle);
		break;
	case CMD_FLUSH:
		ret = flush_entries6(chain, cs.options&OPT_VERBOSE, *handle);
		break;
	case CMD_ZERO:
		ret = zero_entries(chain, cs.options&OPT_VERBOSE, *handle);
		break;
	case CMD_ZERO_NUM:
		ret = ip6tc_zero_counter(chain, rulenum, *handle);
		break;
	case CMD_LIST:
	case CMD_LIST|CMD_ZERO:
	case CMD_LIST|CMD_ZERO_NUM:
		ret = list_entries(chain,
				   rulenum,
				   cs.options&OPT_VERBOSE,
				   cs.options&OPT_NUMERIC,
				   cs.options&OPT_EXPANDED,
				   cs.options&OPT_LINENUMBERS,
				   *handle);
		if (ret && (command & CMD_ZERO))
			ret = zero_entries(chain,
					   cs.options&OPT_VERBOSE, *handle);
		if (ret && (command & CMD_ZERO_NUM))
			ret = ip6tc_zero_counter(chain, rulenum, *handle);
		break;
	case CMD_LIST_RULES:
	case CMD_LIST_RULES|CMD_ZERO:
	case CMD_LIST_RULES|CMD_ZERO_NUM:
		ret = list_rules(chain,
				   rulenum,
				   cs.options&OPT_VERBOSE,
				   *handle);
		if (ret && (command & CMD_ZERO))
			ret = zero_entries(chain,
					   cs.options&OPT_VERBOSE, *handle);
		if (ret && (command & CMD_ZERO_NUM))
			ret = ip6tc_zero_counter(chain, rulenum, *handle);
		break;
	case CMD_NEW_CHAIN:
		ret = ip6tc_create_chain(chain, *handle);
		break;
	case CMD_DELETE_CHAIN:
		ret = delete_chain6(chain, cs.options&OPT_VERBOSE, *handle);
		break;
	case CMD_RENAME_CHAIN:
		ret = ip6tc_rename_chain(chain, newname,	*handle);
		break;
	case CMD_SET_POLICY:
		ret = ip6tc_set_policy(chain, policy, cs.options&OPT_COUNTERS ? &cs.fw6.counters : NULL, *handle);
		break;
	case CMD_NONE:
		/* do_parse ignored the line (eg: -4 with ip6tables-restore) */
		break;
	default:
		/* We should never reach this... */
		exit_tryhelp(2, line);
	}

	if (verbose > 1)
		dump_entries6(*handle);

	xtables_clear_iptables_command_state(&cs);

	if (e != NULL) {
		free(e);
		e = NULL;
	}

	xtables_clear_args(&args);
	xtables_free_opts(1);

	return ret;
}
