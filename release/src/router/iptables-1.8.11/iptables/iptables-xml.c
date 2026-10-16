/* Code to convert iptables-save format to xml format,
 * (C) 2006 Ufo Mechanic <azez@ufomechanic.net>
 * based on iptables-restore (C) 2000-2002 by Harald Welte <laforge@gnumonks.org>
 * based on previous code from Rusty Russell <rusty@linuxcare.com.au>
 *
 * This code is distributed under the terms of GNU GPL v2
 */
#include "config.h"
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "iptables.h"
#include "libiptc/libiptc.h"
#include "xtables-multi.h"
#include <xtables.h>
#include "xshared.h"

struct xtables_globals iptables_xml_globals = {
	.option_offset = 0,
	.program_version = PACKAGE_VERSION,
	.program_name = "iptables-xml",
};
#define prog_name iptables_xml_globals.program_name
#define prog_vers iptables_xml_globals.program_version

static void print_usage(const char *name, const char *version)
	    __attribute__ ((noreturn));

static int verbose;
/* Whether to combine actions of sequential rules with identical conditions */
static int combine;
/* Keeping track of external matches and targets.  */
static const struct option options[] = {
	{"verbose", 0, NULL, 'v'},
	{"combine", 0, NULL, 'c'},
	{"help", 0, NULL, 'h'},
	{ .name = NULL }
};

static void
print_usage(const char *name, const char *version)
{
	fprintf(stderr, "Usage: %s [-c] [-v] [-h]\n"
		"          [--combine ]\n"
		"	   [ --verbose ]\n" "	   [ --help ]\n", name);

	exit(1);
}

#define XT_CHAIN_MAXNAMELEN XT_TABLE_MAXNAMELEN
static char closeActionTag[XT_TABLE_MAXNAMELEN + 1];
static char closeRuleTag[XT_TABLE_MAXNAMELEN + 1];
static char curTable[XT_TABLE_MAXNAMELEN + 1];
static char curChain[XT_CHAIN_MAXNAMELEN + 1];

struct chain {
	char *chain;
	char *policy;
	struct xt_counters count;
	int created;
};

#define maxChains 10240		/* max chains per table */
static struct chain chains[maxChains];
static int nextChain;

/* like puts but with xml encoding */
static void
xmlEncode(char *text)
{
	while (text && *text) {
		if ((unsigned char) (*text) >= 127)
			printf("&#%d;", (unsigned char) (*text));
		else if (*text == '&')
			printf("&amp;");
		else if (*text == '<')
			printf("&lt;");
		else if (*text == '>')
			printf("&gt;");
		else if (*text == '"')
			printf("&quot;");
		else
			putchar(*text);
		text++;
	}
}

/* Output text as a comment, avoiding a double hyphen */
static void
xmlCommentEscape(char *comment)
{
	int h_count = 0;

	while (comment && *comment) {
		if (*comment == '-') {
			h_count++;
			if (h_count >= 2) {
				h_count = 0;
				putchar(' ');
			}
			putchar('*');
		}
		/* strip trailing newline */
		if (*comment == '\n' && *(comment + 1) == 0);
		else
			putchar(*comment);
		comment++;
	}
}

static void
xmlComment(char *comment)
{
	printf("<!-- ");
	xmlCommentEscape(comment);
	printf(" -->\n");
}

static void
xmlAttrS(char *name, char *value)
{
	printf("%s=\"", name);
	xmlEncode(value);
	printf("\" ");
}

static void
xmlAttrI(char *name, long long int num)
{
	printf("%s=\"%lld\" ", name, num);
}

static void
closeChain(void)
{
	if (curChain[0] == 0)
		return;

	if (closeActionTag[0])
		printf("%s\n", closeActionTag);
	closeActionTag[0] = 0;
	if (closeRuleTag[0])
		printf("%s\n", closeRuleTag);
	closeRuleTag[0] = 0;
	if (curChain[0])
		printf("    </chain>\n");
	curChain[0] = 0;
	//lastRule[0]=0;
}

static void
openChain(char *chain, char *policy, struct xt_counters *ctr, char close)
{
	closeChain();

	strncpy(curChain, chain, XT_CHAIN_MAXNAMELEN);
	curChain[XT_CHAIN_MAXNAMELEN] = '\0';

	printf("    <chain ");
	xmlAttrS("name", curChain);
	if (strcmp(policy, "-") != 0)
		xmlAttrS("policy", policy);
	xmlAttrI("packet-count", (unsigned long long) ctr->pcnt);
	xmlAttrI("byte-count", (unsigned long long) ctr->bcnt);
	if (close) {
		printf("%c", close);
		curChain[0] = 0;
	}
	printf(">\n");
}

static int
existsChain(char *chain)
{
	/* open a saved chain */
	int c = 0;

	if (0 == strcmp(curChain, chain))
		return 1;
	for (c = 0; c < nextChain; c++)
		if (chains[c].chain && strcmp(chains[c].chain, chain) == 0)
			return 1;
	return 0;
}

static void
needChain(char *chain)
{
	/* open a saved chain */
	int c = 0;

	if (0 == strcmp(curChain, chain))
		return;

	for (c = 0; c < nextChain; c++)
		if (chains[c].chain && strcmp(chains[c].chain, chain) == 0) {
			openChain(chains[c].chain, chains[c].policy,
				  &(chains[c].count), '\0');
			/* And, mark it as done so we don't create 
			   an empty chain at table-end time */
			chains[c].created = 1;
		}
}

static void
saveChain(char *chain, char *policy, struct xt_counters *ctr)
{
	if (nextChain >= maxChains)
		xtables_error(PARAMETER_PROBLEM,
			      "%s: line %u chain name invalid",
			      prog_name, line);

	chains[nextChain].chain = xtables_strdup(chain);
	chains[nextChain].policy = xtables_strdup(policy);
	chains[nextChain].count = *ctr;
	chains[nextChain].created = 0;
	nextChain++;
}

static void
finishChains(void)
{
	int c;

	for (c = 0; c < nextChain; c++) {
		if (!chains[c].created)
			openChain(chains[c].chain, chains[c].policy,
				  &(chains[c].count), '/');
		free(chains[c].chain);
		free(chains[c].policy);
	}
	nextChain = 0;
}

static void
closeTable(void)
{
	closeChain();
	finishChains();
	if (curTable[0])
		printf("  </table>\n");
	curTable[0] = 0;
}

static void
openTable(char *table)
{
	closeTable();

	strncpy(curTable, table, XT_TABLE_MAXNAMELEN);
	curTable[XT_TABLE_MAXNAMELEN] = '\0';

	printf("  <table ");
	xmlAttrS("name", curTable);
	printf(">\n");
}

// is char* -j --jump -g or --goto
static int
isTarget(char *arg)
{
	return ((arg)
		&& (strcmp((arg), "-j") == 0 || strcmp((arg), "--jump") == 0
		    || strcmp((arg), "-g") == 0
		    || strcmp((arg), "--goto") == 0));
}

// is it a terminating target like -j ACCEPT, etc
// (or I guess -j SNAT in nat table, but we don't check for that yet
static int
isTerminatingTarget(char *arg)
{
	return ((arg)
		&& (strcmp((arg), "ACCEPT") == 0
		    || strcmp((arg), "DROP") == 0
		    || strcmp((arg), "QUEUE") == 0
		    || strcmp((arg), "RETURN") == 0));
}

// part=-1 means do conditions, part=1 means do rules, part=0 means do both
static void
do_rule_part(char *leveltag1, char *leveltag2, int part, int argc,
	     char *argv[], int argvattr[])
{
	int i;
	int arg = 2;		// ignore leading -A <chain>
	char invert_next = 0;
	char *spacer = "";	// space when needed to assemble arguments
	char *level1 = NULL;
	char *level2 = NULL;
	char *leveli1 = "        ";
	char *leveli2 = "          ";

#define CLOSE_LEVEL(LEVEL) \
	do { \
		if (level ## LEVEL) printf("</%s>\n", \
		(leveltag ## LEVEL)?(leveltag ## LEVEL):(level ## LEVEL)); \
		level ## LEVEL=NULL;\
	} while(0)

#define OPEN_LEVEL(LEVEL,TAG) \
	do {\
		level ## LEVEL=TAG;\
		if (leveltag ## LEVEL) {\
			printf("%s<%s ", (leveli ## LEVEL), \
				(leveltag ## LEVEL));\
			xmlAttrS("type", (TAG)); \
		} else printf("%s<%s ", (leveli ## LEVEL), (level ## LEVEL)); \
	} while(0)

	if (part == 1) {	/* skip */
		/* use argvattr to tell which arguments were quoted 
		   to avoid comparing quoted arguments, like comments, to -j, */
		while (arg < argc && (argvattr[arg] || !isTarget(argv[arg])))
			arg++;
	}

	/* Before we start, if the first arg is -[^-] and not -m or -j or -g
	 * then start a dummy <match> tag for old style built-in matches.
	 * We would do this in any case, but no need if it would be empty.
	 * In the case of negation, we need to look at arg+1
	 */
	if (arg < argc && strcmp(argv[arg], "!") == 0)
		i = arg + 1;
	else
		i = arg;
	if (i < argc && argv[i][0] == '-' && !isTarget(argv[i])
	    && strcmp(argv[i], "-m") != 0) {
		OPEN_LEVEL(1, "match");
		printf(">\n");
	}
	while (arg < argc) {
		// If ! is followed by -* then apply to that else output as data
		// Stop, if we need to
		if (part == -1 && !argvattr[arg] && (isTarget(argv[arg]))) {
			break;
		} else if (!argvattr[arg] && strcmp(argv[arg], "!") == 0) {
			if ((arg + 1) < argc && argv[arg + 1][0] == '-')
				invert_next = '!';
			else
				printf("%s%s", spacer, argv[arg]);
			spacer = " ";
		} else if (!argvattr[arg] && isTarget(argv[arg]) &&
			   (arg + 1 < argc) &&
			   existsChain(argv[arg + 1])) {
			CLOSE_LEVEL(2);
			if (level1)
				printf("%s", leveli1);
			CLOSE_LEVEL(1);
			spacer = "";
			invert_next = 0;
			if (strcmp(argv[arg], "-g") == 0
			    || strcmp(argv[arg], "--goto") == 0) {
				/* goto user chain */
				OPEN_LEVEL(1, "goto");
				printf(">\n");
				arg++;
				OPEN_LEVEL(2, argv[arg]);
				printf("/>\n");
				level2 = NULL;
			} else {
				/* call user chain */
				OPEN_LEVEL(1, "call");
				printf(">\n");
				arg++;
				OPEN_LEVEL(2, argv[arg]);
				printf("/>\n");
				level2 = NULL;
			}
		} else if (!argvattr[arg]
			   && (isTarget(argv[arg])
			       || strcmp(argv[arg], "-m") == 0
			       || strcmp(argv[arg], "--module") == 0)) {
			if (!((1 + arg) < argc))
				// no args to -j, -m or -g, ignore & finish loop
				break;
			CLOSE_LEVEL(2);
			if (level1)
				printf("%s", leveli1);
			CLOSE_LEVEL(1);
			spacer = "";
			invert_next = 0;
			arg++;
			OPEN_LEVEL(1, (argv[arg]));
			// Optimize case, can we close this tag already?
			if ((arg + 1) >= argc || (!argvattr[arg + 1]
						  && (isTarget(argv[arg + 1])
						      || strcmp(argv[arg + 1],
								"-m") == 0
						      || strcmp(argv[arg + 1],
								"--module") ==
						      0))) {
				printf(" />\n");
				level1 = NULL;
			} else {
				printf(">\n");
			}
		} else if (!argvattr[arg] && argv[arg][0] == '-') {
			char *tag;
			CLOSE_LEVEL(2);
			// Skip past any -
			tag = argv[arg];
			while (*tag == '-' && *tag)
				tag++;

			spacer = "";
			OPEN_LEVEL(2, tag);
			if (invert_next)
				printf(" invert=\"1\"");
			invert_next = 0;

			// Optimize case, can we close this tag already?
			if (!((arg + 1) < argc)
			    || (argv[arg + 1][0] == '-' /* NOT QUOTED */ )) {
				printf(" />\n");
				level2 = NULL;
			} else {
				printf(">");
			}
		} else {	// regular data
			char *spaces = strchr(argv[arg], ' ');
			printf("%s", spacer);
			if (spaces || argvattr[arg])
				printf("&quot;");
			// if argv[arg] contains a space, enclose in quotes
			xmlEncode(argv[arg]);
			if (spaces || argvattr[arg])
				printf("&quot;");
			spacer = " ";
		}
		arg++;
	}
	CLOSE_LEVEL(2);
	if (level1)
		printf("%s", leveli1);
	CLOSE_LEVEL(1);
}

static int
compareRules(int newargc, char *newargv[], int oldargc, char *oldargv[])
{
	/* Compare arguments up to -j or -g for match.
	 * NOTE: We don't want to combine actions if there were no criteria
	 * in each rule, or rules didn't have an action.
	 * NOTE: Depends on arguments being in some kind of "normal" order which
	 * is the case when processing the ACTUAL output of actual iptables-save
	 * rather than a file merely in a compatible format.
	 */

	unsigned int old = 0;
	unsigned int new = 0;

	int compare = 0;

	while (new < newargc && old < oldargc) {
		if (isTarget(oldargv[old]) && isTarget(newargv[new])) {
			/* if oldarg was a terminating action then it makes no sense
			 * to combine further actions into the same xml */
			if (((strcmp((oldargv[old]), "-j") == 0 
					|| strcmp((oldargv[old]), "--jump") == 0) 
				&& old+1 < oldargc
				&& isTerminatingTarget(oldargv[old+1]) )
			    || strcmp((oldargv[old]), "-g") == 0 
			    || strcmp((oldargv[old]), "--goto") == 0 ) {
				/* Previous rule had terminating action */	
				compare = 0;
			} else {
				compare = 1;
			}
			break;
		}
		// break when old!=new
		if (strcmp(oldargv[old], newargv[new]) != 0) {
			compare = 0;
			break;
		}

		old++;
		new++;
	}
	// We won't match unless both rules had a target. 
	// This means we don't combine target-less rules, which is good

	return compare == 1;
}

/* has a nice parsed rule starting with -A */
static void
do_rule(char *pcnt, char *bcnt, int argc, char *argv[], int argvattr[],
	int oldargc, char *oldargv[])
{
	/* are these conditions the same as the previous rule?
	 * If so, skip arg straight to -j or -g */
	if (combine && argc > 2 && !isTarget(argv[2]) &&
	    compareRules(argc, argv, oldargc, oldargv)) {
		xmlComment("Combine action from next rule");
	} else {

		if (closeActionTag[0]) {
			printf("%s\n", closeActionTag);
			closeActionTag[0] = 0;
		}
		if (closeRuleTag[0]) {
			printf("%s\n", closeRuleTag);
			closeRuleTag[0] = 0;
		}

		printf("      <rule ");
		//xmlAttrS("table",curTable); // not needed in full mode 
		//xmlAttrS("chain",argv[1]); // not needed in full mode 
		if (pcnt)
			xmlAttrS("packet-count", pcnt);
		if (bcnt)
			xmlAttrS("byte-count", bcnt);
		printf(">\n");

		strncpy(closeRuleTag, "      </rule>\n", XT_TABLE_MAXNAMELEN);
		closeRuleTag[XT_TABLE_MAXNAMELEN] = '\0';

		/* no point in writing out condition if there isn't one */
		if (argc >= 3 && !isTarget(argv[2])) {
			printf("       <conditions>\n");
			do_rule_part(NULL, NULL, -1, argc, argv, argvattr);
			printf("       </conditions>\n");
		}
	}
	/* Write out the action */
	//do_rule_part("action","arg",1,argc,argv,argvattr);
	if (!closeActionTag[0]) {
		printf("       <actions>\n");
		strncpy(closeActionTag, "       </actions>\n",
			XT_TABLE_MAXNAMELEN);
		closeActionTag[XT_TABLE_MAXNAMELEN] = '\0';
	}
	do_rule_part(NULL, NULL, 1, argc, argv, argvattr);
}

int
iptables_xml_main(int argc, char *argv[])
{
	struct argv_store last_rule = {}, cur_rule = {};
	char buffer[10240];
	int c;
	FILE *in;

	line = 0;

	xtables_set_params(&iptables_xml_globals);
	while ((c = getopt_long(argc, argv, "cvh", options, NULL)) != -1) {
		switch (c) {
		case 'c':
			combine = 1;
			break;
		case 'v':
			printf("xptables-xml\n");
			verbose = 1;
			break;
		case 'h':
			print_usage("iptables-xml", PACKAGE_VERSION);
			break;
		}
	}

	if (optind == argc - 1) {
		in = fopen(argv[optind], "re");
		if (!in) {
			fprintf(stderr, "Can't open %s: %s", argv[optind],
				strerror(errno));
			exit(1);
		}
	} else if (optind < argc) {
		fprintf(stderr, "Unknown arguments found on commandline");
		exit(1);
	} else
		in = stdin;

	printf("<iptables-rules version=\"1.0\">\n");

	/* Grab standard input. */
	while (fgets(buffer, sizeof(buffer), in)) {
		int ret = 0;

		line++;

		if (buffer[0] == '\n')
			continue;
		else if (buffer[0] == '#') {
			xmlComment(buffer);
			continue;
		}

		if (verbose) {
			printf("<!-- line %d ", line);
			xmlCommentEscape(buffer);
			printf(" -->\n");
		}

		if ((strcmp(buffer, "COMMIT\n") == 0) && (curTable[0])) {
			DEBUGP("Calling commit\n");
			closeTable();
			ret = 1;
		} else if ((buffer[0] == '*')) {
			/* New table */
			char *table;

			table = strtok(buffer + 1, " \t\n");
			DEBUGP("line %u, table '%s'\n", line, table);
			if (!table)
				xtables_error(PARAMETER_PROBLEM,
					      "%s: line %u table name invalid",
					      prog_name, line);

			openTable(table);

			ret = 1;
		} else if ((buffer[0] == ':') && (curTable[0])) {
			/* New chain. */
			char *policy, *chain;
			struct xt_counters count;
			char *ctrs;

			chain = strtok(buffer + 1, " \t\n");
			DEBUGP("line %u, chain '%s'\n", line, chain);
			if (!chain)
				xtables_error(PARAMETER_PROBLEM,
					      "%s: line %u chain name invalid",
					      prog_name, line);

			DEBUGP("Creating new chain '%s'\n", chain);

			policy = strtok(NULL, " \t\n");
			DEBUGP("line %u, policy '%s'\n", line, policy);
			if (!policy)
				xtables_error(PARAMETER_PROBLEM,
					      "%s: line %u policy invalid",
					      prog_name, line);

			ctrs = strtok(NULL, " \t\n");
			parse_counters(ctrs, &count);
			saveChain(chain, policy, &count);

			ret = 1;
		} else if (curTable[0]) {
			unsigned int a;
			char *pcnt = NULL;
			char *bcnt = NULL;
			char *parsestart = buffer;
			char *chain = NULL;

			tokenize_rule_counters(&parsestart, &pcnt, &bcnt, line);
			add_param_to_argv(&cur_rule, parsestart, line);

			DEBUGP("calling do_command4(%u, argv, &%s, handle):\n",
			       cur_rule.argc, curTable);
			debug_print_argv(&cur_rule);

			for (a = 1; a < cur_rule.argc; a++) {
				if (strcmp(cur_rule.argv[a - 1], "-A"))
					continue;
				chain = cur_rule.argv[a];
				break;
			}
			if (!chain) {
				fprintf(stderr, "%s: line %u failed - no chain found\n",
					prog_name, line);
				exit(1);
			}
			needChain(chain);// Should we explicitly look for -A
			do_rule(pcnt, bcnt, cur_rule.argc, cur_rule.argv,
				cur_rule.argvattr, last_rule.argc, last_rule.argv);

			save_argv(&last_rule, &cur_rule);
			ret = 1;
		}
		if (!ret) {
			fprintf(stderr, "%s: line %u failed\n",
				prog_name, line);
			exit(1);
		}
	}
	if (curTable[0]) {
		fprintf(stderr, "%s: COMMIT expected at line %u\n",
			prog_name, line + 1);
		exit(1);
	}

	fclose(in);
	printf("</iptables-rules>\n");
	free_argv(&last_rule);

	return 0;
}
