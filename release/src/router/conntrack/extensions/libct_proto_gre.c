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
#include <netinet/in.h> /* For htons */
#include <libnetfilter_conntrack/libnetfilter_conntrack.h>

#include "conntrack.h"

enum {
	CT_GRE_ORIG_SKEY =	(1 << 0),
	CT_GRE_ORIG_DKEY =	(1 << 1),
	CT_GRE_REPL_SKEY =	(1 << 2),
	CT_GRE_REPL_DKEY =	(1 << 3),
	CT_GRE_MASK_SKEY =	(1 << 4),
	CT_GRE_MASK_DKEY =	(1 << 5),
	CT_GRE_EXPTUPLE_SKEY =	(1 << 6),
	CT_GRE_EXPTUPLE_DKEY =	(1 << 7)
};

#define GRE_OPT_MAX	11
static struct option opts[GRE_OPT_MAX] = {
	{ "orig-key-src",	.has_arg = 1, .val = '1' },
	{ "srckey",		.has_arg = 1, .val = '1' },
	{ "orig-key-dst",	.has_arg = 1, .val = '2' },
	{ "dstkey",		.has_arg = 1, .val = '2' },
	{ "reply-key-src",	.has_arg = 1, .val = '3' },
	{ "reply-key-dst",	.has_arg = 1, .val = '4' },
	{ "mask-key-src",	.has_arg = 1, .val = '5' },
	{ "mask-key-dst",	.has_arg = 1, .val = '6' },
	{ "tuple-key-src",	.has_arg = 1, .val = '7' },
	{ "tuple-key-dst",	.has_arg = 1, .val = '8' },
	{0, 0, 0, 0}
};

static const char *gre_optflags[GRE_OPT_MAX] = {
	[0] = "srckey",
	[1] = "dstkey",
	[2] = "reply-key-src",
	[3] = "reply-key-dst",
	[4] = "mask-key-src",
	[5] = "mask-key-dst",
	[6] = "tuple-key-src",
	[7] = "tuple-key-dst"
};

static void help(void)
{
	fprintf(stdout, "  --orig-key-src\t\toriginal source key\n");
	fprintf(stdout, "  --orig-key-dst\t\toriginal destination key\n");
	fprintf(stdout, "  --reply-key-src\t\treply source key\n");
	fprintf(stdout, "  --reply-key-dst\t\treply destination key\n");
	fprintf(stdout, "  --mask-key-src\t\tmask source key\n");
	fprintf(stdout, "  --mask-key-dst\t\tmask destination key\n");
	fprintf(stdout, "  --tuple-key-src\t\texpectation tuple src key\n");
	fprintf(stdout, "  --tuple-key-src\t\texpectation tuple dst key\n");
}

static char gre_commands_v_options[NUMBER_OF_CMD][GRE_OPT_MAX] =
{
		/* 1 2 3 4 5 6 7 8 */
/*CT_LIST*/	  {2,2,2,2,0,0,0,0},
/*CT_CREATE*/	  {3,3,3,3,0,0,0,0},
/*CT_UPDATE*/	  {2,2,2,2,0,0,0,0},
/*CT_DELETE*/	  {2,2,2,2,0,0,0,0},
/*CT_GET*/	  {3,3,3,3,0,0,0,0},
/*CT_FLUSH*/	  {0,0,0,0,0,0,0,0},
/*CT_EVENT*/	  {2,2,2,2,0,0,0,0},
/*CT_VERSION*/	  {0,0,0,0,0,0,0,0},
/*CT_HELP*/	  {0,0,0,0,0,0,0,0},
/*EXP_LIST*/	  {0,0,0,0,0,0,0,0},
/*EXP_CREATE*/	  {1,1,1,1,1,1,1,1},
/*EXP_DELETE*/	  {1,1,1,1,0,0,0,0},
/*EXP_GET*/	  {1,1,1,1,0,0,0,0},
/*EXP_FLUSH*/	  {0,0,0,0,0,0,0,0},
/*EXP_EVENT*/	  {0,0,0,0,0,0,0,0},
};

static int parse_options(char c,
			 struct nf_conntrack *ct,
			 struct nf_conntrack *exptuple,
			 struct nf_conntrack *mask,
			 unsigned int *flags)
{
	switch(c) {
	uint16_t port;
	case '1':
		port = htons(strtoul(optarg, NULL, 0));
		nfct_set_attr_u16(ct, ATTR_ORIG_PORT_SRC, port);
		nfct_set_attr_u8(ct, ATTR_ORIG_L4PROTO, IPPROTO_GRE);
		*flags |= CT_GRE_ORIG_SKEY;
		break;
	case '2':
		port = htons(strtoul(optarg, NULL, 0));
		nfct_set_attr_u16(ct, ATTR_ORIG_PORT_DST, port);
		nfct_set_attr_u8(ct, ATTR_ORIG_L4PROTO, IPPROTO_GRE);
		*flags |= CT_GRE_ORIG_DKEY;
		break;
	case '3':
		port = htons(strtoul(optarg, NULL, 0));
		nfct_set_attr_u16(ct, ATTR_REPL_PORT_SRC, port);
		nfct_set_attr_u8(ct, ATTR_REPL_L4PROTO, IPPROTO_GRE);
		*flags |= CT_GRE_REPL_SKEY;
		break;
	case '4':
		port = htons(strtoul(optarg, NULL, 0));
		nfct_set_attr_u16(ct, ATTR_REPL_PORT_DST, port);
		nfct_set_attr_u8(ct, ATTR_REPL_L4PROTO, IPPROTO_GRE);
		*flags |= CT_GRE_REPL_DKEY;
		break;
	case '5':
		port = htons(strtoul(optarg, NULL, 0));
		nfct_set_attr_u16(mask, ATTR_ORIG_PORT_SRC, port);
		nfct_set_attr_u8(mask, ATTR_ORIG_L4PROTO, IPPROTO_GRE);
		*flags |= CT_GRE_MASK_SKEY;
		break;
	case '6':
		port = htons(strtoul(optarg, NULL, 0));
		nfct_set_attr_u16(mask, ATTR_ORIG_PORT_DST, port);
		nfct_set_attr_u8(mask, ATTR_ORIG_L4PROTO, IPPROTO_GRE);
		*flags |= CT_GRE_MASK_DKEY;
		break;
	case '7':
		port = htons(strtoul(optarg, NULL, 0));
		nfct_set_attr_u16(exptuple, ATTR_ORIG_PORT_SRC, port);
		nfct_set_attr_u8(exptuple, ATTR_ORIG_L4PROTO, IPPROTO_GRE);
		*flags |= CT_GRE_EXPTUPLE_SKEY;
		break;
	case '8':
		port = htons(strtoul(optarg, NULL, 0));
		nfct_set_attr_u16(exptuple, ATTR_ORIG_PORT_DST, port);
		nfct_set_attr_u8(exptuple, ATTR_ORIG_L4PROTO, IPPROTO_GRE);
		*flags |= CT_GRE_EXPTUPLE_DKEY;
		break;
	}
	return 1;
}

#define GRE_VALID_FLAGS_MAX   2
static unsigned int gre_valid_flags[GRE_VALID_FLAGS_MAX] = {
       CT_GRE_ORIG_SKEY | CT_GRE_ORIG_DKEY,
       CT_GRE_REPL_SKEY | CT_GRE_REPL_DKEY,
};

static void final_check(unsigned int flags,
		        unsigned int cmd,
		        struct nf_conntrack *ct)
{
	int ret, partial;

	ret = generic_opt_check(flags, GRE_OPT_MAX,
				gre_commands_v_options[cmd], gre_optflags,
				gre_valid_flags, GRE_VALID_FLAGS_MAX, &partial);
	if (!ret) {
		switch(partial) {
		case -1:
		case 0:
			exit_error(PARAMETER_PROBLEM, "you have to specify "
						      "`--srckey' and "
						      "`--dstkey'");
			break;
		case 1:
			exit_error(PARAMETER_PROBLEM, "you have to specify "
						      "`--reply-src-key' and "
						      "`--reply-dst-key'");
			break;
		}
	}
}

static struct ctproto_handler gre = {
	.name 			= "gre",
	.protonum		= IPPROTO_GRE,
	.parse_opts		= parse_options,
	.final_check		= final_check,
	.help			= help,
	.opts			= opts,
	.version		= VERSION,
};

void register_gre(void)
{
	register_proto(&gre);
}
