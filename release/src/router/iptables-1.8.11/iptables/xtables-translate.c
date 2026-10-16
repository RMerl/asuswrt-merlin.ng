/*
 * (C) 2014 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include "config.h"
#include <time.h>
#include "xtables-multi.h"
#include "nft.h"

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
#include <iptables.h>
#include <xtables.h>
#include <libiptc/libxtc.h>
#include <fcntl.h>
#include <getopt.h>
#include "xshared.h"
#include "nft-shared.h"

void xlate_ifname(struct xt_xlate *xl, const char *nftmeta, const char *ifname,
		  bool invert)
{
	int ifaclen = strlen(ifname), i, j;
	char iface[IFNAMSIZ * 2];

	if (ifaclen < 1 || ifaclen >= IFNAMSIZ)
		return;

	for (i = 0, j = 0; i < ifaclen + 1; i++, j++) {
		switch (ifname[i]) {
		case '*':
			/* asterisk is non-special mid-string */
			if (i == ifaclen - 1)
				iface[j++] = '\\';
			/* fall through */
		default:
			iface[j] = ifname[i];
			break;
		}
	}

	if (ifaclen == 1 && ifname[0] == '+') {
		/* Nftables does not support wildcard only string. Workaround
		 * is easy, given that this will match always or never
		 * depending on 'invert' value. To match always, simply don't
		 * generate an expression. To match never, use an invalid
		 * interface name (kernel doesn't accept '/' in names) to match
		 * against. */
		if (!invert)
			return;
		strcpy(iface, "INVAL/D");
		invert = false;
	}

	if (iface[j - 2] == '+')
		iface[j - 2] = '*';

	xt_xlate_add(xl, "%s %s\"%s\" ", nftmeta, invert ? "!= " : "", iface);
}

int xlate_action(const struct iptables_command_state *cs, bool goto_set,
		 struct xt_xlate *xl)
{
	int ret = 1, numeric = cs->options & OPT_NUMERIC;

	/* If no target at all, add nothing (default to continue) */
	if (cs->target != NULL) {
		/* Standard target? */
		if (strcmp(cs->jumpto, XTC_LABEL_ACCEPT) == 0)
			xt_xlate_add(xl, " accept");
		else if (strcmp(cs->jumpto, XTC_LABEL_DROP) == 0)
			xt_xlate_add(xl, " drop");
		else if (strcmp(cs->jumpto, XTC_LABEL_RETURN) == 0)
			xt_xlate_add(xl, " return");
		else if (cs->target->xlate) {
			struct xt_xlate_tg_params params = {
				.ip		= (const void *)&cs->fw,
				.target		= cs->target->t,
				.numeric	= numeric,
			};
			ret = cs->target->xlate(xl, &params);
		}
		else
			return 0;
	} else if (strlen(cs->jumpto) > 0) {
		/* Not standard, then it's a go / jump to chain */
		if (goto_set)
			xt_xlate_add(xl, " goto %s", cs->jumpto);
		else
			xt_xlate_add(xl, " jump %s", cs->jumpto);
	}

	return ret;
}

int xlate_matches(const struct iptables_command_state *cs, struct xt_xlate *xl)
{
	struct xtables_rule_match *matchp;
	int ret = 1, numeric = cs->options & OPT_NUMERIC;

	for (matchp = cs->matches; matchp; matchp = matchp->next) {
		struct xt_xlate_mt_params params = {
			.ip		= (const void *)&cs->fw,
			.match		= matchp->match->m,
			.numeric	= numeric,
		};

		if (!matchp->match->xlate)
			return 0;

		ret = matchp->match->xlate(xl, &params);
		if (!ret)
			break;
	}
	return ret;
}

bool xlate_find_match(const struct iptables_command_state *cs, const char *p_name)
{
	struct xtables_rule_match *matchp;

	for (matchp = cs->matches; matchp; matchp = matchp->next) {
		if (strcmp(matchp->match->name, p_name) == 0)
			return true;
	}
	return false;
}

bool xlate_find_protomatch(const struct iptables_command_state *cs,
			   uint16_t proto)
{
	struct protoent *pent;
	int i;

	/* Skip redundant protocol, eg. ip protocol tcp tcp dport */
	for (i = 0; xtables_chain_protos[i].name != NULL; i++) {
		if (xtables_chain_protos[i].num == proto &&
		    xlate_find_match(cs, xtables_chain_protos[i].name))
			return true;
	}
	pent = getprotobynumber(proto);
	return pent && xlate_find_match(cs, pent->p_name);
}

const char *family2str[] = {
	[NFPROTO_ARP]	= "arp",
	[NFPROTO_IPV4]	= "ip",
	[NFPROTO_IPV6]	= "ip6",
};

static int nft_rule_xlate_add(struct nft_handle *h,
			      const struct xt_cmd_parse *p,
			      const struct iptables_command_state *cs,
			      bool append)
{
	struct xt_xlate *xl = xt_xlate_alloc(10240);
	const char *tick = cs->restore ? "" : "'";
	const char *set;
	int ret;

	xl_xlate_set_family(xl, h->family);
	ret = h->ops->xlate(cs, xl);
	if (!ret)
		goto err_out;

	set = xt_xlate_set_get(xl);
	if (set[0]) {
		printf("%sadd set %s %s %s%s\n",
		       tick, family2str[h->family], p->table,
		       xt_xlate_set_get(xl), tick);

		if (!cs->restore && p->command != CMD_NONE)
			printf("nft ");
	}

	printf("%s%s rule %s %s %s ",
	       tick,
	       append ? "add" : "insert",
	       family2str[h->family], p->table, p->chain);
	if (!append && p->rulenum > 1)
		printf("index %d ", p->rulenum);

	printf("%s%s\n", xt_xlate_rule_get(xl), tick);

err_out:
	xt_xlate_free(xl);
	return ret;
}

static int xlate(struct nft_handle *h, struct xt_cmd_parse *p,
		 struct iptables_command_state *cs,
		 struct xtables_args *args, bool append,
		 int (*cb)(struct nft_handle *h,
			   const struct xt_cmd_parse *p,
			   const struct iptables_command_state *cs,
			   bool append))
{
	unsigned int i, j;
	int ret = 1;

	for (i = 0; i < args->s.naddrs; i++) {
		switch (h->family) {
		case NFPROTO_ARP:
			cs->arp.arp.src.s_addr = args->s.addr.v4[i].s_addr;
			cs->arp.arp.smsk.s_addr = args->s.mask.v4[i].s_addr;
			for (j = 0; j < args->d.naddrs; j++) {
				cs->arp.arp.tgt.s_addr = args->d.addr.v4[j].s_addr;
				cs->arp.arp.tmsk.s_addr = args->d.mask.v4[j].s_addr;
				ret = cb(h, p, cs, append);
			}
			break;
		case AF_INET:
			cs->fw.ip.src.s_addr = args->s.addr.v4[i].s_addr;
			cs->fw.ip.smsk.s_addr = args->s.mask.v4[i].s_addr;
			for (j = 0; j < args->d.naddrs; j++) {
				cs->fw.ip.dst.s_addr =
					args->d.addr.v4[j].s_addr;
				cs->fw.ip.dmsk.s_addr =
					args->d.mask.v4[j].s_addr;
				ret = cb(h, p, cs, append);
			}
			break;
		case AF_INET6:
			memcpy(&cs->fw6.ipv6.src,
			       &args->s.addr.v6[i], sizeof(struct in6_addr));
			memcpy(&cs->fw6.ipv6.smsk,
			       &args->s.mask.v6[i], sizeof(struct in6_addr));
			for (j = 0; j < args->d.naddrs; j++) {
				memcpy(&cs->fw6.ipv6.dst,
				       &args->d.addr.v6[j],
				       sizeof(struct in6_addr));
				memcpy(&cs->fw6.ipv6.dmsk,
				       &args->d.mask.v6[j],
				       sizeof(struct in6_addr));
				ret = cb(h, p, cs, append);
			}
			break;
		}
		if (!cs->restore && i < args->s.naddrs - 1)
			printf("nft ");
	}

	return ret;
}

static void print_ipt_cmd(int argc, char *argv[])
{
	int i;

	printf("# ");
	for (i = 1; i < argc; i++)
		printf("%s ", argv[i]);

	printf("\n");
}

static int do_command_xlate(struct nft_handle *h, int argc, char *argv[],
			    char **table, bool restore)
{
	int ret = 0;
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

	cs.restore = restore;

	if (!restore && p.command != CMD_NONE)
		printf("nft ");

	switch (p.command) {
	case CMD_APPEND:
		ret = 1;
		if (!xlate(h, &p, &cs, &args, true, nft_rule_xlate_add))
			print_ipt_cmd(argc, argv);
		break;
	case CMD_DELETE:
		break;
	case CMD_DELETE_NUM:
		break;
	case CMD_CHECK:
		break;
	case CMD_REPLACE:
		break;
	case CMD_INSERT:
		ret = 1;
		if (!xlate(h, &p, &cs, &args, false, nft_rule_xlate_add))
			print_ipt_cmd(argc, argv);
		break;
	case CMD_FLUSH:
		if (p.chain) {
			printf("flush chain %s %s %s\n",
				family2str[h->family], p.table, p.chain);
		} else {
			printf("flush table %s %s\n",
				family2str[h->family], p.table);
		}
		ret = 1;
		break;
	case CMD_ZERO:
		break;
	case CMD_ZERO_NUM:
		break;
	case CMD_LIST:
	case CMD_LIST|CMD_ZERO:
	case CMD_LIST|CMD_ZERO_NUM:
		printf("list table %s %s\n",
		       family2str[h->family], p.table);
		ret = 1;
		break;
	case CMD_LIST_RULES:
	case CMD_LIST_RULES|CMD_ZERO:
	case CMD_LIST_RULES|CMD_ZERO_NUM:
		break;
	case CMD_NEW_CHAIN:
		printf("add chain %s %s %s\n",
		       family2str[h->family], p.table, p.chain);
		ret = 1;
		break;
	case CMD_DELETE_CHAIN:
		printf("delete chain %s %s %s\n",
		       family2str[h->family], p.table, p.chain);
		ret = 1;
		break;
	case CMD_RENAME_CHAIN:
		break;
	case CMD_SET_POLICY:
		break;
	case CMD_NONE:
		ret = 1;
		break;
	default:
		/* We should never reach this... */
		printf("Unsupported command?\n");
		exit(1);
	}

	h->ops->clear_cs(&cs);

	xtables_clear_args(&args);
	xtables_free_opts(1);

	return ret;
}

static void print_usage(const char *name, const char *version)
{
	fprintf(stderr, "%s %s "
			"(c) 2014 by Pablo Neira Ayuso <pablo@netfilter.org>\n"
			"Usage: %s [-h] [-f <FILE>] [-V]\n"
                        "	[ --help ]\n"
                        "	[ --file=<FILE> ]\n"
                        "	[ --version ]\n", name, version, name);
        exit(1);
}

static const struct option options[] = {
	{ .name = "help",	.has_arg = false,	.val = 'h' },
	{ .name = "file",	.has_arg = true,	.val = 'f' },
	{ .name = "version",	.has_arg = false,	.val = 'V' },
	{ NULL },
};

static int xlate_chain_user_restore(struct nft_handle *h, const char *chain,
				    const char *table)
{
	printf("add chain %s %s %s\n", family2str[h->family], table, chain);
	return 0;
}

static int commit(struct nft_handle *h)
{
	return 1;
}

static void xlate_table_new(struct nft_handle *h, const char *table)
{
	printf("add table %s %s\n", family2str[h->family], table);
}

static int get_hook_prio(const char *table, const char *chain)
{
	int prio = 0;

	if (strcmp("nat", table) == 0) {
		if (strcmp(chain, "PREROUTING") == 0)
			prio = NF_IP_PRI_NAT_DST;
		if (strcmp(chain, "INPUT") == 0)
			prio = NF_IP_PRI_NAT_SRC;
		if (strcmp(chain, "OUTPUT") == 0)
			prio = NF_IP_PRI_NAT_DST;
		if (strcmp(chain, "POSTROUTING") == 0)
			prio = NF_IP_PRI_NAT_SRC;
	} else if (strcmp("mangle", table) == 0) {
		prio = NF_IP_PRI_MANGLE;
	} else if (strcmp("raw", table) == 0) {
		prio = NF_IP_PRI_RAW;
	} else if (strcmp(chain, "security") == 0) {
		prio = NF_IP_PRI_SECURITY;
	}

	return prio;
}

static int xlate_chain_set(struct nft_handle *h, const char *table,
			   const char *chain, const char *policy,
			   const struct xt_counters *counters)
{
	const char *type = "filter";
	int prio;

	if (strcmp(table, "nat") == 0)
		type = "nat";
	else if (strcmp(table, "mangle") == 0 && strcmp(chain, "OUTPUT") == 0)
		type = "route";

	printf("add chain %s %s %s { type %s ",
	       family2str[h->family], table, chain, type);
	prio = get_hook_prio(table, chain);
	if (strcmp(chain, "PREROUTING") == 0)
		printf("hook prerouting priority %d; ", prio);
	else if (strcmp(chain, "INPUT") == 0)
		printf("hook input priority %d; ", prio);
	else if (strcmp(chain, "FORWARD") == 0)
		printf("hook forward priority %d; ", prio);
	else if (strcmp(chain, "OUTPUT") == 0)
		printf("hook output priority %d; ", prio);
	else if (strcmp(chain, "POSTROUTING") == 0)
		printf("hook postrouting priority %d; ", prio);

	if (strcmp(policy, "ACCEPT") == 0)
		printf("policy accept; ");
	else if (strcmp(policy, "DROP") == 0)
		printf("policy drop; ");

	printf("}\n");
	return 1;
}

static int dummy_compat_rev(const char *name, uint8_t rev, int opt)
{
	/* Avoid querying the kernel - it's not needed when just translating
	 * rules and not even possible when running as unprivileged user.
	 */
	return 1;
}

static const struct nft_xt_restore_cb cb_xlate = {
	.table_new	= xlate_table_new,
	.chain_set	= xlate_chain_set,
	.chain_restore	= xlate_chain_user_restore,
	.do_command	= do_command_xlate,
	.commit		= commit,
	.abort		= commit,
};

static int xtables_xlate_main_common(struct nft_handle *h,
				     int family,
				     const char *progname)
{
	int ret;

	xtables_globals.program_name = progname;
	xtables_globals.compat_rev = dummy_compat_rev;

	switch (family) {
	case NFPROTO_IPV4:
		ret = xtables_init_all(&xtables_globals, family);
		break;
	case NFPROTO_IPV6:
		ret = xtables_init_all(&xtables_globals, family);
		break;
	case NFPROTO_ARP:
		arptables_globals.program_name = progname;
		arptables_globals.compat_rev = dummy_compat_rev;
		ret = xtables_init_all(&arptables_globals, family);
		break;
	default:
		ret = -1;
		break;
	}

	if (ret < 0) {
		fprintf(stderr, "%s/%s Failed to initialize xtables\n",
			xtables_globals.program_name,
			xtables_globals.program_version);
		return 1;
	}
	init_extensions();
	switch (family) {
	case NFPROTO_IPV4:
		init_extensions4();
		break;
	case NFPROTO_IPV6:
		init_extensions6();
		break;
	case NFPROTO_ARP:
		init_extensionsa();
		break;
	case NFPROTO_BRIDGE:
		init_extensionsb();
		break;
	default:
		fprintf(stderr, "Unknown family %d\n", family);
		return 1;
	}

	if (nft_init(h, family) < 0) {
		fprintf(stderr, "%s/%s Failed to initialize nft: %s\n",
				xtables_globals.program_name,
				xtables_globals.program_version,
				strerror(errno));
		return 1;
	}

	return 0;
}

static int xtables_xlate_main(int family, const char *progname, int argc,
			      char *argv[])
{
	int ret;
	char *table = "filter";
	struct nft_handle h = {
		.family = family,
	};

	ret = xtables_xlate_main_common(&h, family, progname);
	if (ret < 0)
		exit(EXIT_FAILURE);

	ret = do_command_xlate(&h, argc, argv, &table, false);
	if (!ret)
		fprintf(stderr, "Translation not implemented\n");

	nft_fini(&h);
	xtables_fini();
	exit(!ret);
}

static int xtables_restore_xlate_main(int family, const char *progname,
				      int argc, char *argv[])
{
	int ret;
	struct nft_handle h = {
		.family = family,
	};
	const char *file = NULL;
	struct nft_xt_restore_parse p = {
		.cb = &cb_xlate,
	};
	time_t now = time(NULL);
	int c;

	ret = xtables_xlate_main_common(&h, family, progname);
	if (ret < 0)
		exit(EXIT_FAILURE);

	opterr = 0;
	while ((c = getopt_long(argc, argv, "hf:V", options, NULL)) != -1) {
		switch (c) {
		case 'h':
			print_usage(argv[0], PACKAGE_VERSION);
			exit(0);
		case 'f':
			file = optarg;
			break;
		case 'V':
			printf("%s v%s\n", argv[0], PACKAGE_VERSION);
			exit(0);
		}
	}

	if (file == NULL) {
		fprintf(stderr, "ERROR: missing file name\n");
		print_usage(argv[0], PACKAGE_VERSION);
		exit(0);
	}

	p.in = fopen(file, "r");
	if (p.in == NULL) {
		fprintf(stderr, "Cannot open file %s\n", file);
		exit(1);
	}

	printf("# Translated by %s v%s on %s",
	       argv[0], PACKAGE_VERSION, ctime(&now));
	xtables_restore_parse(&h, &p);
	printf("# Completed on %s", ctime(&now));

	nft_fini(&h);
	xtables_fini();
	fclose(p.in);
	exit(0);
}

int xtables_arp_xlate_main(int argc, char *argv[])
{
	return xtables_xlate_main(NFPROTO_ARP, "arptables-translate",
				  argc, argv);
}

int xtables_ip4_xlate_main(int argc, char *argv[])
{
	return xtables_xlate_main(NFPROTO_IPV4, "iptables-translate",
				  argc, argv);
}

int xtables_ip6_xlate_main(int argc, char *argv[])
{
	return xtables_xlate_main(NFPROTO_IPV6, "ip6tables-translate",
				  argc, argv);
}

int xtables_ip4_xlate_restore_main(int argc, char *argv[])
{
	return xtables_restore_xlate_main(NFPROTO_IPV4,
					  "iptables-translate-restore",
					  argc, argv);
}

int xtables_ip6_xlate_restore_main(int argc, char *argv[])
{
	return xtables_restore_xlate_main(NFPROTO_IPV6,
					  "ip6tables-translate-restore",
					  argc, argv);
}
