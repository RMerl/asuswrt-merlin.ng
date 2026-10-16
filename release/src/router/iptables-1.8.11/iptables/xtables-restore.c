/* Code to restore the iptables state, from file by iptables-save.
 * (C) 2000-2002 by Harald Welte <laforge@gnumonks.org>
 * based on previous code from Rusty Russell <rusty@linuxcare.com.au>
 *
 * This code is distributed under the terms of GNU GPL v2
 */
#include "config.h"
#include <getopt.h>
#include <errno.h>
#include <libgen.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "iptables.h"
#include "xtables.h"
#include "libiptc/libiptc.h"
#include "xtables-multi.h"
#include "nft.h"
#include "nft-bridge.h"
#include "nft-cache.h"
#include <libnftnl/chain.h>

static int counters, verbose;

/* Keeping track of external matches and targets.  */
static const struct option options[] = {
	{.name = "counters", .has_arg = false, .val = 'c'},
	{.name = "verbose",  .has_arg = false, .val = 'v'},
	{.name = "version",       .has_arg = 0, .val = 'V'},
	{.name = "test",     .has_arg = false, .val = 't'},
	{.name = "help",     .has_arg = false, .val = 'h'},
	{.name = "noflush",  .has_arg = false, .val = 'n'},
	{.name = "modprobe", .has_arg = true,  .val = 'M'},
	{.name = "table",    .has_arg = true,  .val = 'T'},
	{.name = "ipv4",     .has_arg = false, .val = '4'},
	{.name = "ipv6",     .has_arg = false, .val = '6'},
	{.name = "wait",          .has_arg = 2, .val = 'w'},
	{.name = "wait-interval", .has_arg = 2, .val = 'W'},
	{NULL},
};

#define prog_name xtables_globals.program_name
#define prog_vers xtables_globals.program_version

static void print_usage(const char *name, const char *version)
{
	fprintf(stderr, "Usage: %s [-c] [-v] [-V] [-t] [-h] [-n] [-T table] [-M command] [-4] [-6] [file]\n"
			"	   [ --counters ]\n"
			"	   [ --verbose ]\n"
			"	   [ --version]\n"
			"	   [ --test ]\n"
			"	   [ --help ]\n"
			"	   [ --noflush ]\n"
			"	   [ --table=<TABLE> ]\n"
			"	   [ --modprobe=<command> ]\n"
			"	   [ --ipv4 ]\n"
			"	   [ --ipv6 ]\n", name);
}

static const struct nft_xt_restore_cb restore_cb = {
	.commit		= nft_commit,
	.abort		= nft_abort,
	.table_flush	= nft_cmd_table_flush,
	.do_command	= do_commandx,
	.chain_set	= nft_cmd_chain_set,
	.chain_restore  = nft_cmd_chain_restore,
};

struct nft_xt_restore_state {
	const struct builtin_table *curtable;
	struct argv_store av_store;
	bool in_table;
};

static void xtables_restore_parse_line(struct nft_handle *h,
				       const struct nft_xt_restore_parse *p,
				       struct nft_xt_restore_state *state,
				       char *buffer)
{
	const struct nft_xt_restore_cb *cb = p->cb;
	int ret = 0;

	if (buffer[0] == '\n')
		return;
	else if (buffer[0] == '#') {
		if (verbose) {
			fputs(buffer, stdout);
			fflush(stdout);
		}
		return;
	} else if (state->in_table &&
		   (strncmp(buffer, "COMMIT", 6) == 0) &&
		   (buffer[6] == '\0' || buffer[6] == '\n')) {
		if (!p->testing) {
			/* Commit per table, although we support
			 * global commit at once, stick by now to
			 * the existing behaviour.
			 */
			DEBUGP("Calling commit\n");
			if (cb->commit)
				ret = cb->commit(h);
		} else {
			DEBUGP("Not calling commit, testing\n");
			if (cb->abort)
				ret = cb->abort(h);
		}
		state->in_table = false;

	} else if ((buffer[0] == '*') && (!state->in_table || !p->commit)) {
		/* New table */
		char *table;

		table = strtok(buffer+1, " \t\n");
		DEBUGP("line %u, table '%s'\n", line, table);
		if (!table)
			xtables_error(PARAMETER_PROBLEM,
				      "%s: line %u table name invalid",
				      xt_params->program_name, line);

		state->curtable = nft_table_builtin_find(h, table);
		if (!state->curtable)
			xtables_error(PARAMETER_PROBLEM,
				      "%s: line %u table name '%s' invalid",
				      xt_params->program_name, line, table);

		if (p->tablename && (strcmp(p->tablename, table) != 0))
			return;

		/* implicit commit if no explicit COMMIT supported */
		if (!p->commit)
			cb->commit(h);

		if (h->noflush == 0) {
			DEBUGP("Cleaning all chains of table '%s'\n", table);
			if (cb->table_flush)
				cb->table_flush(h, table, verbose);
		}

		ret = 1;
		state->in_table = true;

		if (cb->table_new)
			cb->table_new(h, table);

	} else if ((buffer[0] == ':') && state->in_table) {
		/* New chain. */
		char *policy, *chain = NULL;
		struct xt_counters count = {};

		chain = strtok(buffer+1, " \t\n");
		DEBUGP("line %u, chain '%s'\n", line, chain);
		if (!chain)
			xtables_error(PARAMETER_PROBLEM,
				      "%s: line %u chain name invalid",
				      xt_params->program_name, line);

		xtables_announce_chain(chain);
		assert_valid_chain_name(chain);

		policy = strtok(NULL, " \t\n");
		DEBUGP("line %u, policy '%s'\n", line, policy);
		if (!policy)
			xtables_error(PARAMETER_PROBLEM,
				      "%s: line %u policy invalid",
				      xt_params->program_name, line);

		if (nft_chain_builtin_find(state->curtable, chain)) {
			char *ctrs = strtok(NULL, " \t\n");

			if ((!ctrs && counters) ||
			    (ctrs && !parse_counters(ctrs, &count)))
				xtables_error(PARAMETER_PROBLEM,
					      "invalid policy counters for chain '%s'",
					      chain);
			if (cb->chain_set &&
			    cb->chain_set(h, state->curtable->name,
					  chain, policy,
					  counters ? &count : NULL) < 0) {
				xtables_error(OTHER_PROBLEM,
					      "Can't set policy `%s' on `%s' line %u: %s",
					      policy, chain, line,
					      strerror(errno));
			}
			DEBUGP("Setting policy of chain %s to %s\n",
			       chain, policy);
		} else if (cb->chain_restore(h, chain, state->curtable->name) < 0 &&
			   errno != EEXIST) {
			xtables_error(PARAMETER_PROBLEM,
				      "cannot create chain '%s' (%s)",
				      chain, strerror(errno));
		} else if (h->family == NFPROTO_BRIDGE &&
			   !ebt_cmd_user_chain_policy(h, state->curtable->name,
						      chain, policy)) {
			xtables_error(OTHER_PROBLEM,
				      "Can't set policy `%s' on `%s' line %u: %s",
				      policy, chain, line,
				      strerror(errno));
		}
		ret = 1;
	} else if (state->in_table) {
		char *pcnt = NULL;
		char *bcnt = NULL;
		char *parsestart = buffer;
		int i;

		add_argv(&state->av_store, xt_params->program_name, 0);
		add_argv(&state->av_store, "-t", 0);
		add_argv(&state->av_store, state->curtable->name, 0);

		for (i = 0; !h->noflush && i < verbose; i++)
			add_argv(&state->av_store, "-v", 0);

		tokenize_rule_counters(&parsestart, &pcnt, &bcnt, line);
		if (counters && pcnt && bcnt) {
			add_argv(&state->av_store, "--set-counters", 0);
			add_argv(&state->av_store, pcnt, 0);
			add_argv(&state->av_store, bcnt, 0);
		}

		add_param_to_argv(&state->av_store, parsestart, line);

		DEBUGP("calling do_command4(%u, argv, &%s, handle):\n",
		       state->av_store.argc, state->curtable->name);
		debug_print_argv(&state->av_store);

		ret = cb->do_command(h, state->av_store.argc,
				     state->av_store.argv,
				     &state->av_store.argv[2], true);
		if (ret < 0) {
			if (cb->abort)
				ret = cb->abort(h);
			else
				ret = 0;

			if (ret < 0) {
				fprintf(stderr,
					"failed to abort commit operation\n");
			}
			exit(1);
		}

		free_argv(&state->av_store);
		fflush(stdout);
	}
	if (p->tablename && state->curtable &&
	    (strcmp(p->tablename, state->curtable->name) != 0))
		return;
	if (!ret) {
		fprintf(stderr, "%s: line %u failed",
				xt_params->program_name, h->error.lineno);
		if (errno)
			fprintf(stderr,	": %s.", nft_strerror(errno));
		fprintf(stderr, "\n");
		exit(1);
	}
}

void xtables_restore_parse(struct nft_handle *h,
			   const struct nft_xt_restore_parse *p)
{
	struct nft_xt_restore_state state = {};
	char buffer[10240] = {};

	if (!verbose && !h->noflush)
		nft_cache_level_set(h, NFT_CL_FAKE, NULL);

	line = 0;
	while (fgets(buffer, sizeof(buffer), p->in)) {
		h->error.lineno = ++line;
		DEBUGP("%s: input line %d: '%s'\n", __func__, line, buffer);
		xtables_restore_parse_line(h, p, &state, buffer);
	}
	if (state.in_table && p->commit) {
		fprintf(stderr, "%s: COMMIT expected at line %u\n",
				xt_params->program_name, line + 1);
		exit(1);
	} else if (state.in_table && p->cb->commit && !p->cb->commit(h)) {
		xtables_error(OTHER_PROBLEM, "%s: final implicit COMMIT failed",
			      xt_params->program_name);
	}
}

static int
xtables_restore_main(int family, const char *progname, int argc, char *argv[])
{
	struct nft_xt_restore_parse p = {
		.commit = true,
		.cb = &restore_cb,
	};
	bool noflush = false;
	struct nft_handle h;
	int c;

	line = 0;

	xtables_globals.program_name = progname;
	c = xtables_init_all(&xtables_globals, family);
	if (c < 0) {
		fprintf(stderr, "%s/%s Failed to initialize xtables\n",
				xtables_globals.program_name,
				xtables_globals.program_version);
		exit(1);
	}

	while ((c = getopt_long(argc, argv, "bcvVthnM:T:wW", options, NULL)) != -1) {
		switch (c) {
			case 'b':
				fprintf(stderr, "-b/--binary option is not implemented\n");
				break;
			case 'c':
				counters = 1;
				break;
			case 'v':
				verbose++;
				break;
			case 'V':
				printf("%s v%s\n", prog_name, prog_vers);
				exit(0);
			case 't':
				p.testing = 1;
				break;
			case 'h':
				print_usage(prog_name, PACKAGE_VERSION);
				exit(0);
			case 'n':
				noflush = true;
				break;
			case 'M':
				xtables_modprobe_program = optarg;
				break;
			case 'T':
				p.tablename = optarg;
				break;
			case 'w': /* fallthrough.  Ignored by xt-restore */
			case 'W':
				if (!optarg && xs_has_arg(argc, argv))
					optind++;
				break;
			default:
				fprintf(stderr,
					"Try `%s -h' for more information.\n",
					prog_name);
				exit(1);
		}
	}

	if (optind == argc - 1) {
		p.in = fopen(argv[optind], "re");
		if (!p.in) {
			fprintf(stderr, "Can't open %s: %s\n", argv[optind],
				strerror(errno));
			exit(1);
		}
	} else if (optind < argc) {
		fprintf(stderr, "Unknown arguments found on commandline\n");
		exit(1);
	} else {
		p.in = stdin;
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

	if (nft_init(&h, family) < 0) {
		fprintf(stderr, "%s/%s Failed to initialize nft: %s\n",
				xtables_globals.program_name,
				xtables_globals.program_version,
				strerror(errno));
		exit(EXIT_FAILURE);
	}
	h.noflush = noflush;
	h.restore = true;

	xtables_restore_parse(&h, &p);

	nft_fini(&h);
	xtables_fini();
	fclose(p.in);
	return 0;
}

int xtables_ip4_restore_main(int argc, char *argv[])
{
	return xtables_restore_main(NFPROTO_IPV4, basename(*argv),
				    argc, argv);
}

int xtables_ip6_restore_main(int argc, char *argv[])
{
	return xtables_restore_main(NFPROTO_IPV6, basename(*argv),
				    argc, argv);
}

static const struct nft_xt_restore_cb ebt_restore_cb = {
	.commit		= nft_bridge_commit,
	.table_flush	= nft_cmd_table_flush,
	.do_command	= do_commandeb,
	.chain_set	= nft_cmd_chain_set,
	.chain_restore  = nft_cmd_chain_restore,
};

static const struct option ebt_restore_options[] = {
	{.name = "noflush", .has_arg = 0, .val = 'n'},
	{.name = "verbose", .has_arg = 0, .val = 'v'},
	{ 0 }
};

int xtables_eb_restore_main(int argc, char *argv[])
{
	struct nft_xt_restore_parse p = {
		.in = stdin,
		.cb = &ebt_restore_cb,
	};
	bool noflush = false;
	struct nft_handle h;
	int c;

	while ((c = getopt_long(argc, argv, "nv",
				ebt_restore_options, NULL)) != -1) {
		switch(c) {
		case 'n':
			noflush = 1;
			break;
		case 'v':
			verbose++;
			break;
		default:
			fprintf(stderr,
				"Usage: ebtables-restore [ --verbose ] [ --noflush ]\n");
			exit(1);
			break;
		}
	}

	nft_init_eb(&h, "ebtables-restore");
	h.noflush = noflush;
	xtables_restore_parse(&h, &p);
	nft_fini_eb(&h);

	return 0;
}

static const struct nft_xt_restore_cb arp_restore_cb = {
	.commit		= nft_commit,
	.table_flush	= nft_cmd_table_flush,
	.do_command	= do_commandx,
	.chain_set	= nft_cmd_chain_set,
	.chain_restore  = nft_cmd_chain_restore,
};

int xtables_arp_restore_main(int argc, char *argv[])
{
	struct nft_xt_restore_parse p = {
		.in = stdin,
		.cb = &arp_restore_cb,
	};
	struct nft_handle h;

	nft_init_arp(&h, "arptables-restore");
	xtables_restore_parse(&h, &p);
	nft_fini(&h);
	xtables_fini();

	return 0;
}
