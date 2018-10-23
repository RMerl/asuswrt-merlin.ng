/*
 * (C) 2005-2007 by Pablo Neira Ayuso <pablo@netfilter.org>
 *     2005 by Harald Welte <laforge@netfilter.org>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 */

#include "conntrack.h"

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <netinet/in.h> /* For htons */
#include <netinet/ip_icmp.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack.h>

enum {
	CT_ICMP_TYPE	= (1 << 0),
	CT_ICMP_CODE	= (1 << 1),
	CT_ICMP_ID	= (1 << 2),
};

static struct option opts[] = {
	{"icmp-type", 1, 0, '1'},
	{"icmp-code", 1, 0, '2'},
	{"icmp-id", 1, 0, '3'},
	{0, 0, 0, 0}
};

#define ICMP_NUMBER_OF_OPT 4

static const char *icmp_optflags[ICMP_NUMBER_OF_OPT] = {
"icmp-type", "icmp-code", "icmp-id"
};

static char icmp_commands_v_options[NUMBER_OF_CMD][ICMP_NUMBER_OF_OPT] =
/* Well, it's better than "Re: Maradona vs Pele" */
{
		/* 1 2 3 */
/*CT_LIST*/	  {2,2,2},
/*CT_CREATE*/	  {1,1,2},
/*CT_UPDATE*/	  {2,2,2},
/*CT_DELETE*/	  {2,2,2},
/*CT_GET*/	  {1,1,2},
/*CT_FLUSH*/	  {0,0,0},
/*CT_EVENT*/	  {2,2,2},
/*CT_VERSION*/	  {0,0,0},
/*CT_HELP*/	  {0,0,0},
/*EXP_LIST*/	  {0,0,0},
/*EXP_CREATE*/	  {0,0,0},
/*EXP_DELETE*/	  {0,0,0},
/*EXP_GET*/	  {0,0,0},
/*EXP_FLUSH*/	  {0,0,0},
/*EXP_EVENT*/	  {0,0,0},
};

static void help(void)
{
	fprintf(stdout, "  --icmp-type\t\t\ticmp type\n");
	fprintf(stdout, "  --icmp-code\t\t\ticmp code\n");
	fprintf(stdout, "  --icmp-id\t\t\ticmp id\n");
}

static int parse(char c,
		 struct nf_conntrack *ct,
		 struct nf_conntrack *exptuple,
		 struct nf_conntrack *mask,
		 unsigned int *flags)
{
	switch(c) {
		uint8_t tmp;
		uint16_t id;
		case '1':
			tmp = atoi(optarg);
			nfct_set_attr_u8(ct, ATTR_ICMP_TYPE, tmp);
			nfct_set_attr_u8(ct, ATTR_L4PROTO, IPPROTO_ICMP);
			*flags |= CT_ICMP_TYPE;
			break;
		case '2':
			tmp = atoi(optarg);
			nfct_set_attr_u8(ct, ATTR_ICMP_CODE, tmp);
			nfct_set_attr_u8(ct, ATTR_L4PROTO, IPPROTO_ICMP);
			*flags |= CT_ICMP_CODE;
			break;
		case '3':
			id = htons(atoi(optarg));
			nfct_set_attr_u16(ct, ATTR_ICMP_ID, id);
			nfct_set_attr_u8(ct, ATTR_L4PROTO, IPPROTO_ICMP);
			*flags |= CT_ICMP_ID;
			break;
	}
	return 1;
}

static void final_check(unsigned int flags,
		        unsigned int cmd,
		        struct nf_conntrack *ct)
{
	generic_opt_check(flags,
			  ICMP_NUMBER_OF_OPT,
			  icmp_commands_v_options[cmd],
			  icmp_optflags, NULL, 0, NULL);
}

static struct ctproto_handler icmp = {
	.name 		= "icmp",
	.protonum	= IPPROTO_ICMP,
	.parse_opts	= parse,
	.final_check	= final_check,
	.help		= help,
	.opts		= opts,
	.version	= VERSION,
};

void register_icmp(void)
{
	register_proto(&icmp);
}
