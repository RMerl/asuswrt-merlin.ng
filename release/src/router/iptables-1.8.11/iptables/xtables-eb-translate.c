#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <getopt.h>
#include <iptables.h>
#include <xtables.h>

#include <netinet/ether.h>

#include <linux/netfilter_bridge.h>
#include <linux/netfilter/nf_tables.h>
#include <libiptc/libxtc.h>

#include "xshared.h"
#include "xtables-multi.h"
#include "nft-bridge.h"
#include "nft.h"
#include "nft-shared.h"

#define prog_name ebtables_globals.program_name

static void print_help(struct iptables_command_state *cs)
{
	fprintf(stderr, "%s: Translate ebtables command to nft syntax\n"
			"no side effects occur, the translated command is written "
			"to standard output.\n"
			"A '#' followed by input means no translation "
			"is available.\n", prog_name);
	exit(0);
}

static void print_ebt_cmd(int argc, char *argv[])
{
	int i;

	printf("# ");
	for (i = 1; i < argc; i++)
		printf("%s ", argv[i]);

	printf("\n");
}

static int nft_rule_eb_xlate_add(struct nft_handle *h, const struct xt_cmd_parse *p,
				 const struct iptables_command_state *cs, bool append)
{
	struct xt_xlate *xl = xt_xlate_alloc(10240);
	const char *tick = cs->restore ? "" : "'";
	int ret;

	xt_xlate_add(xl, "%s%s rule bridge %s %s ", tick,
		     append ? "add" : "insert", p->table, p->chain);

	ret = h->ops->xlate(cs, xl);
	if (ret)
		printf("%s%s\n", xt_xlate_get(xl), tick);
	else
		printf("%s ", tick);

	xt_xlate_free(xl);
	return ret;
}

static int do_commandeb_xlate(struct nft_handle *h, int argc, char *argv[], char **table)
{
	struct iptables_command_state cs = {
		.argv		= argv,
		.jumpto		= "",
		.eb.bitmask	= EBT_NOPROTO,
	};
	struct xt_cmd_parse p = {
		.table          = *table,
		.rule_ranges	= true,
		.ops		= &h->ops->cmd_parse,
        };
	struct xtables_args args = {
		.family	= h->family,
	};
	int ret = 0;

	p.ops->print_help = print_help;

	do_parse(argc, argv, &p, &cs, &args);

	h->verbose	= p.verbose;

	/* Do the final checks */
	if (!nft_table_builtin_find(h, p.table))
		xtables_error(VERSION_PROBLEM,
			      "table '%s' does not exist", p.table);

	printf("nft ");
	switch (p.command) {
	case CMD_FLUSH:
		if (p.chain) {
			printf("flush chain bridge %s %s\n", p.table, p.chain);
		} else {
			printf("flush table bridge %s\n", p.table);
		}
		ret = 1;
		break;
	case CMD_APPEND:
		ret = nft_rule_eb_xlate_add(h, &p, &cs, true);
		if (!ret)
			print_ebt_cmd(argc, argv);
		break;
	case CMD_INSERT:
		ret = nft_rule_eb_xlate_add(h, &p, &cs, false);
		if (!ret)
			print_ebt_cmd(argc, argv);
		break;
	case CMD_LIST:
		printf("list table bridge %s\n", p.table);
		ret = 1;
		break;
	case CMD_NEW_CHAIN:
		printf("add chain bridge %s %s\n", p.table, p.chain);
		ret = 1;
		break;
	case CMD_DELETE_CHAIN:
		printf("delete chain bridge %s %s\n", p.table, p.chain);
		ret = 1;
		break;
	case CMD_INIT_TABLE:
		printf("flush table bridge %s\n", p.table);
		ret = 1;
		break;
	case CMD_DELETE:
	case CMD_DELETE_NUM:
	case CMD_CHECK:
	case CMD_REPLACE:
	case CMD_ZERO:
	case CMD_ZERO_NUM:
	case CMD_LIST|CMD_ZERO:
	case CMD_LIST|CMD_ZERO_NUM:
	case CMD_LIST_RULES:
	case CMD_LIST_RULES|CMD_ZERO:
	case CMD_LIST_RULES|CMD_ZERO_NUM:
	case CMD_NEW_CHAIN|CMD_SET_POLICY:
	case CMD_SET_POLICY:
	case CMD_RENAME_CHAIN:
	case CMD_CHANGE_COUNTERS:
		break;
	default:
		/* We should never reach this... */
		printf("Unsupported command?\n");
		exit(1);
	}

	ebt_cs_clean(&cs);
	return ret;
}

static int dummy_compat_rev(const char *name, uint8_t rev, int opt)
{
	return 1;
}

int xtables_eb_xlate_main(int argc, char *argv[])
{
	int ret;
	char *table = "filter";
	struct nft_handle h;

	nft_init_eb(&h, argv[0]);
	ebtables_globals.compat_rev = dummy_compat_rev;

	ret = do_commandeb_xlate(&h, argc, argv, &table);
	if (!ret)
		fprintf(stderr, "Translation not implemented\n");

	exit(!ret);
}

