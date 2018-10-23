/*
 * (C) 2009 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 */
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h> /* For htons */
#include <libnetfilter_conntrack/libnetfilter_conntrack.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack_dccp.h>

#include "conntrack.h"

#ifndef IPPROTO_DCCP
#define IPPROTO_DCCP 33
#endif

enum {
	CT_DCCP_ORIG_SPORT	= (1 << 0),
	CT_DCCP_ORIG_DPORT	= (1 << 1),
	CT_DCCP_REPL_SPORT	= (1 << 2),
	CT_DCCP_REPL_DPORT	= (1 << 3),
	CT_DCCP_MASK_SPORT	= (1 << 4),
	CT_DCCP_MASK_DPORT	= (1 << 5),
	CT_DCCP_STATE		= (1 << 6),
	CT_DCCP_EXPTUPLE_SPORT	= (1 << 7),
	CT_DCCP_EXPTUPLE_DPORT	= (1 << 8),
	CT_DCCP_ROLE		= (1 << 9),
};

#define DCCP_OPT_MAX	13
static struct option opts[DCCP_OPT_MAX] = {
	{ .name = "orig-port-src",	.has_arg = 1, .val = 1 },
	{ .name = "orig-port-dst",	.has_arg = 1, .val = 2 },
	{ .name = "reply-port-src",	.has_arg = 1, .val = 3 },
	{ .name = "reply-port-dst",	.has_arg = 1, .val = 4 },
	{ .name = "mask-port-src",	.has_arg = 1, .val = 5 },
	{ .name = "mask-port-dst",	.has_arg = 1, .val = 6 },
	{ .name = "state",		.has_arg = 1, .val = 7 },
	{ .name = "tuple-port-src",	.has_arg = 1, .val = 8 },
	{ .name = "tuple-port-dst",	.has_arg = 1, .val = 9 },
	{ .name = "sport",		.has_arg = 1, .val = 1 }, /* alias */
	{ .name = "dport",		.has_arg = 1, .val = 2 }, /* alias */
	{ .name = "role",		.has_arg = 1, .val = 10 },
	{ 0, 0, 0, 0},
};

static const char *dccp_optflags[DCCP_OPT_MAX] = {
	[0] = "sport",
	[1] = "dport",
	[2] = "reply-port-src",
	[3] = "reply-port-dst",
	[4] = "mask-port-src",
	[5] = "mask-port-dst",
	[6] = "state",
	[7] = "tuple-port-src",
	[8] = "tuple-port-dst",
	[9] = "role"
};

static char dccp_commands_v_options[NUMBER_OF_CMD][DCCP_OPT_MAX] =
/* Well, it's better than "Re: Sevilla vs Betis" */
{
	    	/* 1 2 3 4 5 6 7 8 9 10*/
/*CT_LIST*/   	  {2,2,2,2,0,0,2,0,0,0},
/*CT_CREATE*/	  {3,3,3,3,0,0,1,0,0,1},
/*CT_UPDATE*/	  {2,2,2,2,0,0,2,0,0,0},
/*CT_DELETE*/	  {2,2,2,2,0,0,2,0,0,0},
/*CT_GET*/	  {3,3,3,3,0,0,2,0,0,0},
/*CT_FLUSH*/	  {0,0,0,0,0,0,0,0,0,0},
/*CT_EVENT*/	  {2,2,2,2,0,0,2,0,0,0},
/*CT_VERSION*/	  {0,0,0,0,0,0,0,0,0,0},
/*CT_HELP*/	  {0,0,0,0,0,0,0,0,0,0},
/*EXP_LIST*/	  {0,0,0,0,0,0,0,0,0,0},
/*EXP_CREATE*/	  {1,1,0,0,1,1,0,1,1,1},
/*EXP_DELETE*/	  {1,1,1,1,0,0,0,0,0,0},
/*EXP_GET*/	  {1,1,1,1,0,0,0,0,0,0},
/*EXP_FLUSH*/	  {0,0,0,0,0,0,0,0,0,0},
/*EXP_EVENT*/	  {0,0,0,0,0,0,0,0,0,0},
};

static const char *dccp_states[DCCP_CONNTRACK_MAX] = {
	[DCCP_CONNTRACK_NONE]		= "NONE",
	[DCCP_CONNTRACK_REQUEST]	= "REQUEST",
	[DCCP_CONNTRACK_RESPOND]	= "RESPOND",
	[DCCP_CONNTRACK_PARTOPEN]	= "PARTOPEN",
	[DCCP_CONNTRACK_OPEN]		= "OPEN",
	[DCCP_CONNTRACK_CLOSEREQ]	= "CLOSEREQ",
	[DCCP_CONNTRACK_CLOSING]	= "CLOSING",
	[DCCP_CONNTRACK_TIMEWAIT]	= "TIMEWAIT",
	[DCCP_CONNTRACK_IGNORE]		= "IGNORE",
	[DCCP_CONNTRACK_INVALID]	= "INVALID",
};

static void help(void)
{
	fprintf(stdout, "  --orig-port-src\t\toriginal source port\n");
	fprintf(stdout, "  --orig-port-dst\t\toriginal destination port\n");
	fprintf(stdout, "  --reply-port-src\t\treply source port\n");
	fprintf(stdout, "  --reply-port-dst\t\treply destination port\n");
	fprintf(stdout, "  --mask-port-src\t\tmask source port\n");
	fprintf(stdout, "  --mask-port-dst\t\tmask destination port\n");
	fprintf(stdout, "  --tuple-port-src\t\texpectation tuple src port\n");
	fprintf(stdout, "  --tuple-port-src\t\texpectation tuple dst port\n");
	fprintf(stdout, "  --state\t\t\tDCCP state, fe. RESPOND\n");
}

static int parse_options(char c,
			 struct nf_conntrack *ct,
			 struct nf_conntrack *exptuple,
			 struct nf_conntrack *mask,
			 unsigned int *flags)
{
	int i;
	uint16_t port;

	switch(c) {
	case 1:
		port = htons(atoi(optarg));
		nfct_set_attr_u16(ct, ATTR_ORIG_PORT_SRC, port);
		nfct_set_attr_u8(ct, ATTR_ORIG_L4PROTO, IPPROTO_DCCP);
		*flags |= CT_DCCP_ORIG_SPORT;
		break;
	case 2:
		port = htons(atoi(optarg));
		nfct_set_attr_u16(ct, ATTR_ORIG_PORT_DST, port);
		nfct_set_attr_u8(ct, ATTR_ORIG_L4PROTO, IPPROTO_DCCP);
		*flags |= CT_DCCP_ORIG_DPORT;
		break;
	case 3:
		port = htons(atoi(optarg));
		nfct_set_attr_u16(ct, ATTR_REPL_PORT_SRC, port);
		nfct_set_attr_u8(ct, ATTR_REPL_L4PROTO, IPPROTO_DCCP);
		*flags |= CT_DCCP_REPL_SPORT;
		break;
	case 4:
		port = htons(atoi(optarg));
		nfct_set_attr_u16(ct, ATTR_REPL_PORT_DST, port);
		nfct_set_attr_u8(ct, ATTR_REPL_L4PROTO, IPPROTO_DCCP);
		*flags |= CT_DCCP_REPL_DPORT;
		break;
	case 5:
		port = htons(atoi(optarg));
		nfct_set_attr_u16(mask, ATTR_ORIG_PORT_SRC, port);
		nfct_set_attr_u8(mask, ATTR_ORIG_L4PROTO, IPPROTO_DCCP);
		*flags |= CT_DCCP_MASK_SPORT;
		break;
	case 6:
		port = htons(atoi(optarg));
		nfct_set_attr_u16(mask, ATTR_ORIG_PORT_DST, port);
		nfct_set_attr_u8(mask, ATTR_ORIG_L4PROTO, IPPROTO_DCCP);
		*flags |= CT_DCCP_MASK_DPORT;
		break;
	case 7:
		for (i=0; i<DCCP_CONNTRACK_MAX; i++) {
			if (strcmp(optarg, dccp_states[i]) == 0) {
				nfct_set_attr_u8(ct, ATTR_DCCP_STATE, i);
				break;
			}
		}
		if (i == DCCP_CONNTRACK_MAX)
			exit_error(PARAMETER_PROBLEM,
				   "Unknown DCCP state `%s'", optarg);
		*flags |= CT_DCCP_STATE;
		break;
	case 8:
		port = htons(atoi(optarg));
		nfct_set_attr_u16(exptuple, ATTR_ORIG_PORT_SRC, port);
		nfct_set_attr_u8(exptuple, ATTR_ORIG_L4PROTO, port);
		*flags |= CT_DCCP_EXPTUPLE_SPORT;
		break;
	case 9:
		port = htons(atoi(optarg));
		nfct_set_attr_u16(exptuple, ATTR_ORIG_PORT_DST, port); 
		nfct_set_attr_u8(exptuple, ATTR_ORIG_L4PROTO, port);
		*flags |= CT_DCCP_EXPTUPLE_DPORT;
		break;
	case 10:
		if (strncasecmp(optarg, "client", strlen(optarg)) == 0) {
			nfct_set_attr_u8(ct, ATTR_DCCP_ROLE,
					 DCCP_CONNTRACK_ROLE_CLIENT);
		} else if (strncasecmp(optarg, "server", strlen(optarg)) == 0) {
			nfct_set_attr_u8(ct, ATTR_DCCP_ROLE,
					 DCCP_CONNTRACK_ROLE_SERVER);
		} else {
			exit_error(PARAMETER_PROBLEM,
				   "Unknown DCCP role `%s'", optarg);
		}
		*flags |= CT_DCCP_ROLE;
		break;
	}
	return 1;
}

#define DCCP_VALID_FLAGS_MAX	2
static unsigned int dccp_valid_flags[DCCP_VALID_FLAGS_MAX] = {
	CT_DCCP_ORIG_SPORT | CT_DCCP_ORIG_DPORT,
	CT_DCCP_REPL_SPORT | CT_DCCP_REPL_DPORT
};

static void 
final_check(unsigned int flags, unsigned int cmd, struct nf_conntrack *ct)
{
	int ret, partial;

	ret = generic_opt_check(flags, DCCP_OPT_MAX,
				dccp_commands_v_options[cmd], dccp_optflags,
				dccp_valid_flags, DCCP_VALID_FLAGS_MAX,
				&partial);
	if (!ret) {
		switch(partial) {
		case -1:
		case 0:
			exit_error(PARAMETER_PROBLEM, "you have to specify "
						      "`--sport' and "
						      "`--dport'");
			break;
		case 1:
			exit_error(PARAMETER_PROBLEM, "you have to specify "
						      "`--reply-port-src' and "
						      "`--report-port-dst'");
			break;
		}
	}
}

static struct ctproto_handler dccp = {
	.name 			= "dccp",
	.protonum		= IPPROTO_DCCP,
	.parse_opts		= parse_options,
	.final_check		= final_check,
	.help			= help,
	.opts			= opts,
	.version		= VERSION,
};

void register_dccp(void)
{
	register_proto(&dccp);
}
