/*
 * (C) 2005-2007 by Pablo Neira Ayuso <pablo@netfilter.org>
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
#include <libnetfilter_conntrack/libnetfilter_conntrack_tcp.h>

#include "conntrack.h"

enum {
	CT_TCP_ORIG_SPORT =	(1 << 0),
	CT_TCP_ORIG_DPORT =	(1 << 1),
	CT_TCP_REPL_SPORT =	(1 << 2),
	CT_TCP_REPL_DPORT =	(1 << 3),
	CT_TCP_MASK_SPORT =	(1 << 4),
	CT_TCP_MASK_DPORT =	(1 << 5),
	CT_TCP_STATE =		(1 << 6),
	CT_TCP_EXPTUPLE_SPORT =	(1 << 7),
	CT_TCP_EXPTUPLE_DPORT =	(1 << 8)
};

static struct option opts[] = {
	{"orig-port-src", 1, 0, '1'},
	{"sport", 1, 0, '1'},
	{"orig-port-dst", 1, 0, '2'},
	{"dport", 1, 0, '2'},
	{"reply-port-src", 1, 0, '3'},
	{"reply-port-dst", 1, 0, '4'},
	{"mask-port-src", 1, 0, '5'},
	{"mask-port-dst", 1, 0, '6'},
	{"state", 1, 0, '7'},
	{"tuple-port-src", 1, 0, '8'},
	{"tuple-port-dst", 1, 0, '9'},
	{0, 0, 0, 0}
};

#define TCP_NUMBER_OF_OPT	10

static const char *tcp_optflags[TCP_NUMBER_OF_OPT] = {
"sport", "dport", "reply-port-src", "reply-port-dst", "mask-port-src",
"mask-port-dst", "state", "tuple-port-src", "tuple-port-dst"
};

static char tcp_commands_v_options[NUMBER_OF_CMD][TCP_NUMBER_OF_OPT] =
/* Well, it's better than "Re: Sevilla vs Betis" */
{
	    	/* 1 2 3 4 5 6 7 8 9 */
/*CT_LIST*/   	  {2,2,2,2,0,0,2,0,0},
/*CT_CREATE*/	  {3,3,3,3,0,0,1,0,0},
/*CT_UPDATE*/	  {2,2,2,2,0,0,2,0,0},
/*CT_DELETE*/	  {2,2,2,2,0,0,2,0,0},
/*CT_GET*/	  {3,3,3,3,0,0,2,0,0},
/*CT_FLUSH*/	  {0,0,0,0,0,0,0,0,0},
/*CT_EVENT*/	  {2,2,2,2,0,0,2,0,0},
/*CT_VERSION*/	  {0,0,0,0,0,0,0,0,0},
/*CT_HELP*/	  {0,0,0,0,0,0,0,0,0},
/*EXP_LIST*/	  {0,0,0,0,0,0,0,0,0},
/*EXP_CREATE*/	  {1,1,0,0,1,1,0,1,1},
/*EXP_DELETE*/	  {1,1,1,1,0,0,0,0,0},
/*EXP_GET*/	  {1,1,1,1,0,0,0,0,0},
/*EXP_FLUSH*/	  {0,0,0,0,0,0,0,0,0},
/*EXP_EVENT*/	  {0,0,0,0,0,0,0,0,0},
};

static const char *tcp_states[TCP_CONNTRACK_MAX] = {
	[TCP_CONNTRACK_NONE]		= "NONE",
	[TCP_CONNTRACK_SYN_SENT]	= "SYN_SENT",
	[TCP_CONNTRACK_SYN_RECV] 	= "SYN_RECV",
	[TCP_CONNTRACK_ESTABLISHED]	= "ESTABLISHED",
	[TCP_CONNTRACK_FIN_WAIT]	= "FIN_WAIT",
	[TCP_CONNTRACK_CLOSE_WAIT]	= "CLOSE_WAIT",
	[TCP_CONNTRACK_LAST_ACK]	= "LAST_ACK",
	[TCP_CONNTRACK_TIME_WAIT]	= "TIME_WAIT",
	[TCP_CONNTRACK_CLOSE]		= "CLOSE",
	/* Since Linux kernel 2.6.31, LISTEN is SYN_SENT2. */
	[TCP_CONNTRACK_SYN_SENT2]	= "SYN_SENT2"
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
	fprintf(stdout, "  --state\t\t\tTCP state, fe. ESTABLISHED\n");
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
	case '1':
		port = htons(atoi(optarg));
		nfct_set_attr_u16(ct, ATTR_ORIG_PORT_SRC, port);
		nfct_set_attr_u8(ct, ATTR_ORIG_L4PROTO, IPPROTO_TCP);
		*flags |= CT_TCP_ORIG_SPORT;
		break;
	case '2':
		port = htons(atoi(optarg));
		nfct_set_attr_u16(ct, ATTR_ORIG_PORT_DST, port);
		nfct_set_attr_u8(ct, ATTR_ORIG_L4PROTO, IPPROTO_TCP);
		*flags |= CT_TCP_ORIG_DPORT;
		break;
	case '3':
		port = htons(atoi(optarg));
		nfct_set_attr_u16(ct, ATTR_REPL_PORT_SRC, port);
		nfct_set_attr_u8(ct, ATTR_REPL_L4PROTO, IPPROTO_TCP);
		*flags |= CT_TCP_REPL_SPORT;
		break;
	case '4':
		port = htons(atoi(optarg));
		nfct_set_attr_u16(ct, ATTR_REPL_PORT_DST, port);
		nfct_set_attr_u8(ct, ATTR_REPL_L4PROTO, IPPROTO_TCP);
		*flags |= CT_TCP_REPL_DPORT;
		break;
	case '5':
		port = htons(atoi(optarg));
		nfct_set_attr_u16(mask, ATTR_ORIG_PORT_SRC, port);
		nfct_set_attr_u8(mask, ATTR_ORIG_L4PROTO, IPPROTO_TCP);
		*flags |= CT_TCP_MASK_SPORT;
		break;
	case '6':
		port = htons(atoi(optarg));
		nfct_set_attr_u16(mask, ATTR_ORIG_PORT_DST, port);
		nfct_set_attr_u8(mask, ATTR_ORIG_L4PROTO, IPPROTO_TCP);
		*flags |= CT_TCP_MASK_DPORT;
		break;
	case '7':
		for (i=0; i<TCP_CONNTRACK_MAX; i++) {
			if (strcmp(optarg, tcp_states[i]) == 0) {
				nfct_set_attr_u8(ct, ATTR_TCP_STATE, i);
				break;
			}
		}
		/* For backward compatibility with Linux kernel < 2.6.31. */
		if (strcmp(optarg, "LISTEN") == 0) {
			nfct_set_attr_u8(ct, ATTR_TCP_STATE,
					 TCP_CONNTRACK_LISTEN);
		} else if (i == TCP_CONNTRACK_MAX)
			exit_error(PARAMETER_PROBLEM,
				   "unknown TCP state `%s'", optarg);
		*flags |= CT_TCP_STATE;
		break;
	case '8':
		port = htons(atoi(optarg));
		nfct_set_attr_u16(exptuple, ATTR_ORIG_PORT_SRC, port);
		nfct_set_attr_u8(exptuple, ATTR_ORIG_L4PROTO, port);
		*flags |= CT_TCP_EXPTUPLE_SPORT;
		break;
	case '9':
		port = htons(atoi(optarg));
		nfct_set_attr_u16(exptuple, ATTR_ORIG_PORT_DST, port); 
		nfct_set_attr_u8(exptuple, ATTR_ORIG_L4PROTO, port);
		*flags |= CT_TCP_EXPTUPLE_DPORT;
		break;
	}
	return 1;
}

#define TCP_VALID_FLAGS_MAX   2
static unsigned int tcp_valid_flags[TCP_VALID_FLAGS_MAX] = {
       CT_TCP_ORIG_SPORT | CT_TCP_ORIG_DPORT,
       CT_TCP_REPL_SPORT | CT_TCP_REPL_DPORT,
};

static void final_check(unsigned int flags,
			unsigned int cmd,
			struct nf_conntrack *ct)
{
	int ret, partial;

	ret = generic_opt_check(flags, TCP_NUMBER_OF_OPT,
				tcp_commands_v_options[cmd], tcp_optflags,
				tcp_valid_flags, TCP_VALID_FLAGS_MAX, &partial);
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
						      "`--reply-src-port' and "
						      "`--reply-dst-port'");
			break;
		}
	}
	/* Disable TCP window tracking for manually created TCP entries,
	 * otherwise this will not work. */
	uint8_t tcp_flags = IP_CT_TCP_FLAG_BE_LIBERAL |
			    IP_CT_TCP_FLAG_SACK_PERM;

	/* This allows to reopen a new connection directly from TIME-WAIT
	 * as RFC 1122 states. See nf_conntrack_proto_tcp.c for more info. */
	if (nfct_get_attr_u8(ct, ATTR_TCP_STATE) >= TCP_CONNTRACK_TIME_WAIT)
		tcp_flags |= IP_CT_TCP_FLAG_CLOSE_INIT;

	nfct_set_attr_u8(ct, ATTR_TCP_FLAGS_ORIG, tcp_flags);
	nfct_set_attr_u8(ct, ATTR_TCP_MASK_ORIG, tcp_flags);
	nfct_set_attr_u8(ct, ATTR_TCP_FLAGS_REPL, tcp_flags);
	nfct_set_attr_u8(ct, ATTR_TCP_MASK_REPL, tcp_flags);
}

static struct ctproto_handler tcp = {
	.name 			= "tcp",
	.protonum		= IPPROTO_TCP,
	.parse_opts		= parse_options,
	.final_check		= final_check,
	.help			= help,
	.opts			= opts,
	.version		= VERSION,
};

void register_tcp(void)
{
	register_proto(&tcp);
}
