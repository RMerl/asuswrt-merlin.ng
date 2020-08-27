/*
 * (C) 2005-2012 by Pablo Neira Ayuso <pablo@netfilter.org>
 * (C) 2012 by Intra2net AG <http://www.intra2net.com>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Note:
 *	Yes, portions of this code has been stolen from iptables ;)
 *	Special thanks to the the Netfilter Core Team.
 *	Thanks to Javier de Miguel Rodriguez <jmiguel at talika.eii.us.es>
 *	for introducing me to advanced firewalling stuff.
 *
 *						--pablo 13/04/2005
 *
 * 2005-04-16 Harald Welte <laforge@netfilter.org>: 
 * 	Add support for conntrack accounting and conntrack mark
 * 2005-06-23 Harald Welte <laforge@netfilter.org>:
 * 	Add support for expect creation
 * 2005-09-24 Harald Welte <laforge@netfilter.org>:
 * 	Remove remaints of "-A"
 * 2007-04-22 Pablo Neira Ayuso <pablo@netfilter.org>:
 * 	Ported to the new libnetfilter_conntrack API
 * 2008-04-13 Pablo Neira Ayuso <pablo@netfilter.org>:
 *	Way more flexible update and delete operations
 *
 * Part of this code has been funded by Sophos Astaro <http://www.sophos.com>
 */

#include "conntrack.h"

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <syslog.h>
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#include <signal.h>
#include <string.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libmnl/libmnl.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack.h>

struct u32_mask {
	uint32_t value;
	uint32_t mask;
};

/* These are the template objects that are used to send commands. */
static struct {
	struct nf_conntrack *ct;
	struct nf_expect *exp;
	/* Expectations require the expectation tuple and the mask. */
	struct nf_conntrack *exptuple, *mask;

	/* Allows filtering/setting specific bits in the ctmark */
	struct u32_mask mark;

	/* Allow to filter by mark from kernel-space. */
	struct nfct_filter_dump_mark filter_mark_kernel;
	bool filter_mark_kernel_set;

	/* Allows filtering by ctlabels */
	struct nfct_bitmask *label;

	/* Allows setting/removing specific ctlabels */
	struct nfct_bitmask *label_modify;
} tmpl;

static int alloc_tmpl_objects(void)
{
	tmpl.ct = nfct_new();
	tmpl.exptuple = nfct_new();
	tmpl.mask = nfct_new();
	tmpl.exp = nfexp_new();

	memset(&tmpl.mark, 0, sizeof(tmpl.mark));

	return tmpl.ct != NULL && tmpl.exptuple != NULL &&
	       tmpl.mask != NULL && tmpl.exp != NULL;
}

static void free_tmpl_objects(void)
{
	if (tmpl.ct)
		nfct_destroy(tmpl.ct);
	if (tmpl.exptuple)
		nfct_destroy(tmpl.exptuple);
	if (tmpl.mask)
		nfct_destroy(tmpl.mask);
	if (tmpl.exp)
		nfexp_destroy(tmpl.exp);
	if (tmpl.label)
		nfct_bitmask_destroy(tmpl.label);
	if (tmpl.label_modify)
		nfct_bitmask_destroy(tmpl.label_modify);
}

enum ct_command {
	CT_NONE		= 0,

	CT_LIST_BIT 	= 0,
	CT_LIST 	= (1 << CT_LIST_BIT),

	CT_CREATE_BIT	= 1,
	CT_CREATE	= (1 << CT_CREATE_BIT),

	CT_UPDATE_BIT	= 2,
	CT_UPDATE	= (1 << CT_UPDATE_BIT),

	CT_DELETE_BIT	= 3,
	CT_DELETE	= (1 << CT_DELETE_BIT),

	CT_GET_BIT	= 4,
	CT_GET		= (1 << CT_GET_BIT),

	CT_FLUSH_BIT	= 5,
	CT_FLUSH	= (1 << CT_FLUSH_BIT),

	CT_EVENT_BIT	= 6,
	CT_EVENT	= (1 << CT_EVENT_BIT),

	CT_VERSION_BIT	= 7,
	CT_VERSION	= (1 << CT_VERSION_BIT),

	CT_HELP_BIT	= 8,
	CT_HELP		= (1 << CT_HELP_BIT),

	EXP_LIST_BIT 	= 9,
	EXP_LIST 	= (1 << EXP_LIST_BIT),

	EXP_CREATE_BIT	= 10,
	EXP_CREATE	= (1 << EXP_CREATE_BIT),

	EXP_DELETE_BIT	= 11,
	EXP_DELETE	= (1 << EXP_DELETE_BIT),

	EXP_GET_BIT	= 12,
	EXP_GET		= (1 << EXP_GET_BIT),

	EXP_FLUSH_BIT	= 13,
	EXP_FLUSH	= (1 << EXP_FLUSH_BIT),

	EXP_EVENT_BIT	= 14,
	EXP_EVENT	= (1 << EXP_EVENT_BIT),

	CT_COUNT_BIT	= 15,
	CT_COUNT	= (1 << CT_COUNT_BIT),

	EXP_COUNT_BIT	= 16,
	EXP_COUNT	= (1 << EXP_COUNT_BIT),

	CT_STATS_BIT	= 17,
	CT_STATS	= (1 << CT_STATS_BIT),

	EXP_STATS_BIT	= 18,
	EXP_STATS	= (1 << EXP_STATS_BIT),
};
/* If you add a new command, you have to update NUMBER_OF_CMD in conntrack.h */

enum ct_options {
	CT_OPT_ORIG_SRC_BIT	= 0,
	CT_OPT_ORIG_SRC 	= (1 << CT_OPT_ORIG_SRC_BIT),

	CT_OPT_ORIG_DST_BIT	= 1,
	CT_OPT_ORIG_DST		= (1 << CT_OPT_ORIG_DST_BIT),

	CT_OPT_ORIG		= (CT_OPT_ORIG_SRC | CT_OPT_ORIG_DST),

	CT_OPT_REPL_SRC_BIT	= 2,
	CT_OPT_REPL_SRC		= (1 << CT_OPT_REPL_SRC_BIT),

	CT_OPT_REPL_DST_BIT	= 3,
	CT_OPT_REPL_DST		= (1 << CT_OPT_REPL_DST_BIT),

	CT_OPT_REPL		= (CT_OPT_REPL_SRC | CT_OPT_REPL_DST),

	CT_OPT_PROTO_BIT	= 4,
	CT_OPT_PROTO		= (1 << CT_OPT_PROTO_BIT),

	CT_OPT_TUPLE_ORIG	= (CT_OPT_ORIG | CT_OPT_PROTO),
	CT_OPT_TUPLE_REPL	= (CT_OPT_REPL | CT_OPT_PROTO),

	CT_OPT_TIMEOUT_BIT	= 5,
	CT_OPT_TIMEOUT		= (1 << CT_OPT_TIMEOUT_BIT),

	CT_OPT_STATUS_BIT	= 6,
	CT_OPT_STATUS		= (1 << CT_OPT_STATUS_BIT),

	CT_OPT_ZERO_BIT		= 7,
	CT_OPT_ZERO		= (1 << CT_OPT_ZERO_BIT),

	CT_OPT_EVENT_MASK_BIT	= 8,
	CT_OPT_EVENT_MASK	= (1 << CT_OPT_EVENT_MASK_BIT),

	CT_OPT_EXP_SRC_BIT	= 9,
	CT_OPT_EXP_SRC		= (1 << CT_OPT_EXP_SRC_BIT),

	CT_OPT_EXP_DST_BIT	= 10,
	CT_OPT_EXP_DST		= (1 << CT_OPT_EXP_DST_BIT),

	CT_OPT_MASK_SRC_BIT	= 11,
	CT_OPT_MASK_SRC		= (1 << CT_OPT_MASK_SRC_BIT),

	CT_OPT_MASK_DST_BIT	= 12,
	CT_OPT_MASK_DST		= (1 << CT_OPT_MASK_DST_BIT),

	CT_OPT_NATRANGE_BIT	= 13,
	CT_OPT_NATRANGE		= (1 << CT_OPT_NATRANGE_BIT),

	CT_OPT_MARK_BIT		= 14,
	CT_OPT_MARK		= (1 << CT_OPT_MARK_BIT),

	CT_OPT_ID_BIT		= 15,
	CT_OPT_ID		= (1 << CT_OPT_ID_BIT),

	CT_OPT_FAMILY_BIT	= 16,
	CT_OPT_FAMILY		= (1 << CT_OPT_FAMILY_BIT),

	CT_OPT_SRC_NAT_BIT	= 17,
	CT_OPT_SRC_NAT		= (1 << CT_OPT_SRC_NAT_BIT),

	CT_OPT_DST_NAT_BIT	= 18,
	CT_OPT_DST_NAT		= (1 << CT_OPT_DST_NAT_BIT),

	CT_OPT_OUTPUT_BIT	= 19,
	CT_OPT_OUTPUT		= (1 << CT_OPT_OUTPUT_BIT),

	CT_OPT_SECMARK_BIT	= 20,
	CT_OPT_SECMARK		= (1 << CT_OPT_SECMARK_BIT),

	CT_OPT_BUFFERSIZE_BIT	= 21,
	CT_OPT_BUFFERSIZE	= (1 << CT_OPT_BUFFERSIZE_BIT),

	CT_OPT_ANY_NAT_BIT	= 22,
	CT_OPT_ANY_NAT		= (1 << CT_OPT_ANY_NAT_BIT),

	CT_OPT_ZONE_BIT		= 23,
	CT_OPT_ZONE		= (1 << CT_OPT_ZONE_BIT),

	CT_OPT_LABEL_BIT	= 24,
	CT_OPT_LABEL		= (1 << CT_OPT_LABEL_BIT),

	CT_OPT_ADD_LABEL_BIT	= 25,
	CT_OPT_ADD_LABEL	= (1 << CT_OPT_ADD_LABEL_BIT),

	CT_OPT_DEL_LABEL_BIT	= 26,
	CT_OPT_DEL_LABEL	= (1 << CT_OPT_DEL_LABEL_BIT),

	CT_OPT_ORIG_ZONE_BIT	= 27,
	CT_OPT_ORIG_ZONE	= (1 << CT_OPT_ORIG_ZONE_BIT),

	CT_OPT_REPL_ZONE_BIT	= 28,
	CT_OPT_REPL_ZONE	= (1 << CT_OPT_REPL_ZONE_BIT),

	CT_OPT_LOG_BIT		= 29,
	CT_OPT_LOG		= (1 << CT_OPT_LOG_BIT),
};
/* If you add a new option, you have to update NUMBER_OF_OPT in conntrack.h */

/* Update this mask to allow to filter based on new options. */
#define CT_COMPARISON (CT_OPT_PROTO | CT_OPT_ORIG | CT_OPT_REPL | \
		       CT_OPT_MARK | CT_OPT_SECMARK |  CT_OPT_STATUS | \
		       CT_OPT_ID | CT_OPT_ZONE | CT_OPT_LABEL | \
		       CT_OPT_ORIG_ZONE | CT_OPT_REPL_ZONE)

static const char *optflags[NUMBER_OF_OPT] = {
	[CT_OPT_ORIG_SRC_BIT] 	= "src",
	[CT_OPT_ORIG_DST_BIT]	= "dst",
	[CT_OPT_REPL_SRC_BIT]	= "reply-src",
	[CT_OPT_REPL_DST_BIT]	= "reply-dst",
	[CT_OPT_PROTO_BIT]	= "protonum",
	[CT_OPT_TIMEOUT_BIT]	= "timeout",
	[CT_OPT_STATUS_BIT]	= "status",
	[CT_OPT_ZERO_BIT]	= "zero",
	[CT_OPT_EVENT_MASK_BIT]	= "event-mask",
	[CT_OPT_EXP_SRC_BIT]	= "tuple-src",
	[CT_OPT_EXP_DST_BIT]	= "tuple-dst",
	[CT_OPT_MASK_SRC_BIT]	= "mask-src",
	[CT_OPT_MASK_DST_BIT]	= "mask-dst",
	[CT_OPT_NATRANGE_BIT]	= "nat-range",
	[CT_OPT_MARK_BIT]	= "mark",
	[CT_OPT_ID_BIT]		= "id",
	[CT_OPT_FAMILY_BIT]	= "family",
	[CT_OPT_SRC_NAT_BIT]	= "src-nat",
	[CT_OPT_DST_NAT_BIT]	= "dst-nat",
	[CT_OPT_OUTPUT_BIT]	= "output",
	[CT_OPT_SECMARK_BIT]	= "secmark",
	[CT_OPT_BUFFERSIZE_BIT]	= "buffer-size",
	[CT_OPT_ANY_NAT_BIT]	= "any-nat",
	[CT_OPT_ZONE_BIT]	= "zone",
	[CT_OPT_LABEL_BIT]	= "label",
	[CT_OPT_ADD_LABEL_BIT]	= "label-add",
	[CT_OPT_DEL_LABEL_BIT]	= "label-del",
	[CT_OPT_ORIG_ZONE_BIT]	= "orig-zone",
	[CT_OPT_REPL_ZONE_BIT]	= "reply-zone",
	[CT_OPT_LOG_BIT]	= "log",
};

/*
 * name: the name of the long option.
 * has_arg: 0: no_argument if the option does not take an argument;
 *          1: required_argument if the option requires an argument;
 *          2: optional_argument if the option takes an optional argument.
 * flag: specifies how results are returned for a long option.
 *       If flag is NULL, then getopt_long() returns val.
 *       (For example, the calling program may set val to the equivalent short option character.)
 *       Otherwise, getopt_long() returns 0, and flag points to a variable which is set to val
 *       if the option is found, but left unchanged if the option is not found.
 * val: the value to return, or to load into the variable pointed to by flag.
 */

/*
struct option {
	const char *name;
	int	  has_arg;
	int	  *flag;
	int	  val;
};
*/

static struct option original_opts[] = {
	{"dump", 2, 0, 'L'},
	{"create", 2, 0, 'I'},
	{"delete", 2, 0, 'D'},
	{"update", 2, 0, 'U'},
	{"get", 2, 0, 'G'},
	{"flush", 2, 0, 'F'},
	{"event", 2, 0, 'E'},
	{"counter", 2, 0, 'C'},
	{"stats", 0, 0, 'S'},
	{"version", 0, 0, 'V'},
	{"help", 0, 0, 'h'},
	{"orig-src", 1, 0, 's'},
	{"src", 1, 0, 's'},
	{"orig-dst", 1, 0, 'd'},
	{"dst", 1, 0, 'd'},
	{"reply-src", 1, 0, 'r'},
	{"reply-dst", 1, 0, 'q'},
	{"protonum", 1, 0, 'p'},
	{"timeout", 1, 0, 't'},
	{"status", 1, 0, 'u'},
	{"zero", 0, 0, 'z'},
	{"event-mask", 1, 0, 'e'},
	{"tuple-src", 1, 0, '['},
	{"tuple-dst", 1, 0, ']'},
	{"mask-src", 1, 0, '{'},
	{"mask-dst", 1, 0, '}'},
	{"nat-range", 1, 0, 'a'},	/* deprecated */
	{"mark", 1, 0, 'm'},
	{"secmark", 1, 0, 'c'},
	{"id", 2, 0, 'i'},		/* deprecated */
	{"family", 1, 0, 'f'},
	{"src-nat", 2, 0, 'n'},
	{"dst-nat", 2, 0, 'g'},
	{"output", 1, 0, 'o'},
	{"buffer-size", 1, 0, 'b'},
	{"any-nat", 2, 0, 'j'},
	{"zone", 1, 0, 'w'},
	{"label", 1, 0, 'l'},
	{"label-add", 1, 0, '<'},
	{"label-del", 2, 0, '>'},
	{"orig-zone", 1, 0, '('},
	{"reply-zone", 1, 0, ')'},
	{"log", 1, 0, 'v'}, //Andrew
	{0, 0, 0, 0}
};

static const char *getopt_str = ":L::I::U::D::G::E::F::hVs:d:r:q:"
				"p:t:u:e:a:z[:]:{:}:m:i:f:o:n::"
				"g::c:b:C::Sj::w:l:<:>::(:):v:";

/* Table of legal combinations of commands and options.  If any of the
 * given commands make an option legal, that option is legal (applies to
 * CMD_LIST and CMD_ZERO only).
 * Key:
 *  0  illegal
 *  1  compulsory
 *  2  optional
 *  3  undecided, see flag combination checkings in generic_opt_check()
 */

static char commands_v_options[NUMBER_OF_CMD][NUMBER_OF_OPT] =
/* Well, it's better than "Re: Linux vs FreeBSD" */
{
          /*   s d r q p t u z e [ ] { } a m i f n g o c b j w l < > ( ) v*/
/*CT_LIST*/   {2,2,2,2,2,0,2,2,0,0,0,2,2,0,2,0,2,2,2,2,2,0,2,2,2,0,0,2,2,0},
/*CT_CREATE*/ {3,3,3,3,1,1,2,0,0,0,0,0,0,2,2,0,0,2,2,0,0,0,0,2,0,2,0,2,2,0},
/*CT_UPDATE*/ {2,2,2,2,2,2,2,0,0,0,0,2,2,0,2,2,2,2,2,2,0,0,0,0,2,2,2,0,0,0},
/*CT_DELETE*/ {2,2,2,2,2,2,2,0,0,0,0,2,2,0,2,2,2,2,2,2,0,0,0,2,2,0,0,2,2,0},
/*CT_GET*/    {3,3,3,3,1,0,0,0,0,0,0,0,0,0,0,2,0,0,0,2,0,0,0,0,2,0,0,0,0,0},
/*CT_FLUSH*/  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
/*CT_EVENT*/  {2,2,2,2,2,0,0,0,2,0,0,2,2,0,2,0,0,2,2,2,2,2,2,2,2,0,0,2,2,2},
/*VERSION*/   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
/*HELP*/      {0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
/*EXP_LIST*/  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,2,0,0,0,0,0,0,0,0,0,0},
/*EXP_CREATE*/{1,1,2,2,1,1,2,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
/*EXP_DELETE*/{1,1,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
/*EXP_GET*/   {1,1,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
/*EXP_FLUSH*/ {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
/*EXP_EVENT*/ {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0},
/*CT_COUNT*/  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
/*EXP_COUNT*/ {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
/*CT_STATS*/  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
/*EXP_STATS*/ {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
};

static const int cmd2type[][2] = {
	['L']	= { CT_LIST, 	EXP_LIST },
	['I']	= { CT_CREATE,	EXP_CREATE },
	['D']	= { CT_DELETE,	EXP_DELETE },
	['G']	= { CT_GET,	EXP_GET },
	['F']	= { CT_FLUSH,	EXP_FLUSH },
	['E']	= { CT_EVENT,	EXP_EVENT },
	['V']	= { CT_VERSION,	CT_VERSION },
	['h']	= { CT_HELP,	CT_HELP },
	['C']	= { CT_COUNT,	EXP_COUNT },
	['S']	= { CT_STATS,	EXP_STATS },
};

static const int opt2type[] = {
	['s']	= CT_OPT_ORIG_SRC,
	['d']	= CT_OPT_ORIG_DST,
	['r']	= CT_OPT_REPL_SRC,
	['q']	= CT_OPT_REPL_DST,
	['{']	= CT_OPT_MASK_SRC,
	['}']	= CT_OPT_MASK_DST,
	['[']	= CT_OPT_EXP_SRC,
	[']']	= CT_OPT_EXP_DST,
	['n']	= CT_OPT_SRC_NAT,
	['g']	= CT_OPT_DST_NAT,
	['m']	= CT_OPT_MARK,
	['c']	= CT_OPT_SECMARK,
	['i']	= CT_OPT_ID,
	['j']	= CT_OPT_ANY_NAT,
	['w']	= CT_OPT_ZONE,
	['l']	= CT_OPT_LABEL,
	['<']	= CT_OPT_ADD_LABEL,
	['>']	= CT_OPT_DEL_LABEL,
	['(']	= CT_OPT_ORIG_ZONE,
	[')']	= CT_OPT_REPL_ZONE,
	['v']	= CT_OPT_LOG,
};

static const int opt2maskopt[] = {
	['s']	= '{',
	['d']	= '}',
	['g']   = 0,
	['j']   = 0,
	['n']   = 0,
	['r']	= 0, /* no netmask */
	['q']	= 0, /* support yet */
	['{']	= 0,
	['}']	= 0,
	['[']	= '{',
	[']']	= '}',
};

static const int opt2family_attr[][2] = {
	['s']	= { ATTR_ORIG_IPV4_SRC,	ATTR_ORIG_IPV6_SRC },
	['d']	= { ATTR_ORIG_IPV4_DST,	ATTR_ORIG_IPV6_DST },
	['g']   = { ATTR_DNAT_IPV4, ATTR_DNAT_IPV6 },
	['n']   = { ATTR_SNAT_IPV4, ATTR_SNAT_IPV6 },
	['r']	= { ATTR_REPL_IPV4_SRC, ATTR_REPL_IPV6_SRC },
	['q']	= { ATTR_REPL_IPV4_DST, ATTR_REPL_IPV6_DST },
	['{']	= { ATTR_ORIG_IPV4_SRC,	ATTR_ORIG_IPV6_SRC },
	['}']	= { ATTR_ORIG_IPV4_DST,	ATTR_ORIG_IPV6_DST },
	['[']	= { ATTR_ORIG_IPV4_SRC, ATTR_ORIG_IPV6_SRC },
	[']']	= { ATTR_ORIG_IPV4_DST, ATTR_ORIG_IPV6_DST },
};

static const int opt2attr[] = {
	['s']	= ATTR_ORIG_L3PROTO,
	['d']	= ATTR_ORIG_L3PROTO,
	['g']	= ATTR_ORIG_L3PROTO,
	['n']	= ATTR_ORIG_L3PROTO,
	['r']	= ATTR_REPL_L3PROTO,
	['q']	= ATTR_REPL_L3PROTO,
	['{']	= ATTR_ORIG_L3PROTO,
	['}']	= ATTR_ORIG_L3PROTO,
	['[']	= ATTR_ORIG_L3PROTO,
	[']']	= ATTR_ORIG_L3PROTO,
	['m']	= ATTR_MARK,
	['c']	= ATTR_SECMARK,
	['i']	= ATTR_ID,
	['w']	= ATTR_ZONE,
	['l']	= ATTR_CONNLABELS,
	['<']	= ATTR_CONNLABELS,
	['>']	= ATTR_CONNLABELS,
	['(']	= ATTR_ORIG_ZONE,
	[')']	= ATTR_REPL_ZONE,
};

enum ct_direction {
	DIR_SRC = 0,
	DIR_DST = 1,
};

union ct_address {
	uint32_t v4;
	uint32_t v6[4];
};

static struct ct_network {
	union ct_address netmask;
	union ct_address network;
} dir2network[2];

static const int famdir2attr[2][2] = {
	{ ATTR_ORIG_IPV4_SRC, ATTR_ORIG_IPV4_DST },
	{ ATTR_ORIG_IPV6_SRC, ATTR_ORIG_IPV6_DST }
};

static char exit_msg[NUMBER_OF_CMD][64] = {
	[CT_LIST_BIT] 		= "%d flow entries have been shown.\n",
	[CT_CREATE_BIT]		= "%d flow entries have been created.\n",
	[CT_UPDATE_BIT]		= "%d flow entries have been updated.\n",
	[CT_DELETE_BIT]		= "%d flow entries have been deleted.\n",
	[CT_GET_BIT] 		= "%d flow entries have been shown.\n",
	[CT_EVENT_BIT]		= "%d flow events have been shown.\n",
	[EXP_LIST_BIT]		= "%d expectations have been shown.\n",
	[EXP_DELETE_BIT]	= "%d expectations have been shown.\n",
};

static const char usage_commands[] =
	"Commands:\n"
	"  -L [table] [options]\t\tList conntrack or expectation table\n"
	"  -G [table] parameters\t\tGet conntrack or expectation\n"
	"  -D [table] parameters\t\tDelete conntrack or expectation\n"
	"  -I [table] parameters\t\tCreate a conntrack or expectation\n"
	"  -U [table] parameters\t\tUpdate a conntrack\n"
	"  -E [table] [options]\t\tShow events\n"
	"  -F [table]\t\t\tFlush table\n"
	"  -C [table]\t\t\tShow counter\n"
	"  -S\t\t\t\tShow statistics\n";

static const char usage_tables[] =
	"Tables: conntrack, expect, dying, unconfirmed\n";

static const char usage_conntrack_parameters[] =
	"Conntrack parameters and options:\n"
	"  -n, --src-nat ip\t\t\tsource NAT ip\n"
	"  -g, --dst-nat ip\t\t\tdestination NAT ip\n"
	"  -j, --any-nat ip\t\t\tsource or destination NAT ip\n"
	"  -m, --mark mark\t\t\tSet mark\n"
	"  -c, --secmark secmark\t\t\tSet selinux secmark\n"
	"  -e, --event-mask eventmask\t\tEvent mask, eg. NEW,DESTROY\n"
	"  -z, --zero \t\t\t\tZero counters while listing\n"
	"  -o, --output type[,...]\t\tOutput format, eg. xml\n"
	"  -l, --label label[,...]\t\tconntrack labels\n";

static const char usage_expectation_parameters[] =
	"Expectation parameters and options:\n"
	"  --tuple-src ip\tSource address in expect tuple\n"
	"  --tuple-dst ip\tDestination address in expect tuple\n"
	;

static const char usage_update_parameters[] =
	"Updating parameters and options:\n"
	"  --label-add label\tAdd label\n"
	"  --label-del label\tDelete label\n";

static const char usage_parameters[] =
	"Common parameters and options:\n"
	"  -s, --src, --orig-src ip\t\tSource address from original direction\n"
	"  -d, --dst, --orig-dst ip\t\tDestination address from original direction\n"
	"  -r, --reply-src ip\t\tSource addres from reply direction\n"
	"  -q, --reply-dst ip\t\tDestination address from reply direction\n"
	"  -p, --protonum proto\t\tLayer 4 Protocol, eg. 'tcp'\n"
	"  -f, --family proto\t\tLayer 3 Protocol, eg. 'ipv6'\n"
	"  -t, --timeout timeout\t\tSet timeout\n"
	"  -u, --status status\t\tSet status, eg. ASSURED\n"
	"  -w, --zone value\t\tSet conntrack zone\n"
	"  --orig-zone value\t\tSet zone for original direction\n"
	"  --reply-zone value\t\tSet zone for reply direction\n"
	"  -b, --buffer-size\t\tNetlink socket buffer size\n"
	"  --mask-src ip\t\t\tSource mask address\n"
	"  --mask-dst ip\t\t\tDestination mask address\n"
	"  --log logfile\t\t\tFile to log\n"
	;

#define OPTION_OFFSET 256

static struct nfct_handle *cth, *ith;
static struct option *opts = original_opts;
static unsigned int global_option_offset = 0;

#define ADDR_VALID_FLAGS_MAX   2
static unsigned int addr_valid_flags[ADDR_VALID_FLAGS_MAX] = {
	CT_OPT_ORIG_SRC | CT_OPT_ORIG_DST,
	CT_OPT_REPL_SRC | CT_OPT_REPL_DST,
};

static LIST_HEAD(proto_list);

static unsigned int options;
static struct nfct_labelmap *labelmap;
static int filter_family;
static FILE *log_fp = NULL;
static char log_file[256] = "";

void register_proto(struct ctproto_handler *h)
{
	if (strcmp(h->version, VERSION) != 0) {
		fprintf(stderr, "plugin `%s': version %s (I'm %s)\n",
			h->name, h->version, VERSION);
		exit(1);
	}
	list_add(&h->head, &proto_list);
}

extern struct ctproto_handler ct_proto_unknown;

static struct ctproto_handler *findproto(char *name, int *pnum)
{
	struct ctproto_handler *cur;
	struct protoent *pent;
	int protonum;

	/* is it in the list of supported protocol? */
	list_for_each_entry(cur, &proto_list, head) {
		if (strcasecmp(cur->name, name) == 0) {
			*pnum = cur->protonum;
			return cur;
		}
	}
	/* using the protocol name for an unsupported protocol? */
	if ((pent = getprotobyname(name))) {
		*pnum = pent->p_proto;
		return &ct_proto_unknown;
	}
	/* using a protocol number? */
	protonum = atoi(name);
	if (protonum > 0 && protonum <= IPPROTO_MAX) {
		/* try lookup by number, perhaps this protocol is supported */
		list_for_each_entry(cur, &proto_list, head) {
			if (cur->protonum == protonum) {
				*pnum = protonum;
				return cur;
			}
		}
		*pnum = protonum;
		return &ct_proto_unknown;
	}

	return NULL;
}

static void
extension_help(struct ctproto_handler *h, int protonum)
{
	const char *name;

	if (h == &ct_proto_unknown) {
		struct protoent *pent;

		pent = getprotobynumber(protonum);
		if (!pent)
			name = h->name;
		else
			name = pent->p_name;
	} else {
		name = h->name;
	}

	fprintf(stdout, "Proto `%s' help:\n", name);
	h->help();
}

static void __attribute__((noreturn))
exit_tryhelp(int status)
{
	fprintf(stderr, "Try `%s -h' or '%s --help' for more information.\n",
			PROGNAME, PROGNAME);
	exit(status);
}

static void free_options(void)
{
	if (opts != original_opts) {
		free(opts);
		opts = original_opts;
		global_option_offset = 0;
	}
}

void __attribute__((noreturn))
exit_error(enum exittype status, const char *msg, ...)
{
	va_list args;

	free_options();
	va_start(args, msg);
	fprintf(stderr,"%s v%s (conntrack-tools): ", PROGNAME, VERSION);
	vfprintf(stderr, msg, args);
	fprintf(stderr, "\n");
	va_end(args);
	if (status == PARAMETER_PROBLEM)
		exit_tryhelp(status);
	/* release template objects that were allocated in the setup stage. */
	free_tmpl_objects();
	exit(status);
}

static int bit2cmd(int command)
{
	int i;

	for (i = 0; i < NUMBER_OF_CMD; i++)
		if (command & (1<<i))
			break;

	return i;
}

static const char *get_long_opt(int opt)
{
	struct option o;
	int i;

	for (i = 0;; i++) {
		o = opts[i];
		if (o.name == NULL)
			break;
		if (o.val == opt)
			return o.name;
	}
	return "unknown";
}

int generic_opt_check(int local_options, int num_opts,
		      char *optset, const char *optflg[],
		      unsigned int *coupled_flags, int coupled_flags_size,
		      int *partial)
{
	int i, matching = -1, special_case = 0;

	for (i = 0; i < num_opts; i++) {
		if (!(local_options & (1<<i))) {
			if (optset[i] == 1)
				exit_error(PARAMETER_PROBLEM, 
					   "You need to supply the "
					   "`--%s' option for this "
					   "command", optflg[i]);
		} else {
			if (optset[i] == 0)
				exit_error(PARAMETER_PROBLEM, "Illegal "
					   "option `--%s' with this "
					   "command", optflg[i]);
		}
		if (optset[i] == 3)
			special_case = 1;
	}

	/* no weird flags combinations, leave */
	if (!special_case || coupled_flags == NULL)
		return 1;

	*partial = -1;
	for (i=0; i<coupled_flags_size; i++) {
		/* we look for an exact matching to ensure this is correct */
		if ((local_options & coupled_flags[i]) == coupled_flags[i]) {
			matching = i;
			break;
		}
		/* ... otherwise look for the first partial matching */
		if ((local_options & coupled_flags[i]) && *partial < 0) {
			*partial = i;
		}
	}

	/* we found an exact matching, game over */
	if (matching >= 0)
		return 1;

	/* report a partial matching to suggest something */
	return 0;
}

static struct option *
merge_options(struct option *oldopts, const struct option *newopts,
	      unsigned int *option_offset)
{
	unsigned int num_old, num_new, i;
	struct option *merge;

	for (num_old = 0; oldopts[num_old].name; num_old++);
	for (num_new = 0; newopts[num_new].name; num_new++);

	global_option_offset += OPTION_OFFSET;
	*option_offset = global_option_offset;

	merge = malloc(sizeof(struct option) * (num_new + num_old + 1));
	if (merge == NULL)
		return NULL;

	memcpy(merge, oldopts, num_old * sizeof(struct option));
	for (i = 0; i < num_new; i++) {
		merge[num_old + i] = newopts[i];
		merge[num_old + i].val += *option_offset;
	}
	memset(merge + num_old + num_new, 0, sizeof(struct option));

	return merge;
}

/* From linux/errno.h */
#define ENOTSUPP        524     /* Operation is not supported */

/* Translates errno numbers into more human-readable form than strerror. */
static const char *
err2str(int err, enum ct_command command)
{
	unsigned int i;
	struct table_struct {
		enum ct_command act;
		int err;
		const char *message;
	} table [] =
	  { { CT_LIST, ENOTSUPP, "function not implemented" },
	    { 0xFFFF, EINVAL, "invalid parameters" },
	    { CT_CREATE, EEXIST, "Such conntrack exists, try -U to update" },
	    { CT_CREATE|CT_GET|CT_DELETE, ENOENT,
		    "such conntrack doesn't exist" },
	    { CT_CREATE|CT_GET, ENOMEM, "not enough memory" },
	    { CT_GET, EAFNOSUPPORT, "protocol not supported" },
	    { CT_CREATE, ETIME, "conntrack has expired" },
	    { EXP_CREATE, ENOENT, "master conntrack not found" },
	    { EXP_CREATE, EINVAL, "invalid parameters" },
	    { ~0U, EPERM, "sorry, you must be root or get "
		    	   "CAP_NET_ADMIN capability to do this"}
	  };

	for (i = 0; i < sizeof(table)/sizeof(struct table_struct); i++) {
		if ((table[i].act & command) && table[i].err == err)
			return table[i].message;
	}

	return strerror(err);
}

static int mark_cmp(const struct u32_mask *m, const struct nf_conntrack *ct)
{
	return nfct_attr_is_set(ct, ATTR_MARK) &&
		(nfct_get_attr_u32(ct, ATTR_MARK) & m->mask) == m->value;
}

#define PARSE_STATUS 0
#define PARSE_EVENT 1
#define PARSE_OUTPUT 2
#define PARSE_MAX 3

enum {
	_O_XML	= (1 << 0),
	_O_EXT	= (1 << 1),
	_O_TMS	= (1 << 2),
	_O_ID	= (1 << 3),
	_O_KTMS	= (1 << 4),
	_O_CL	= (1 << 5),
};

enum {
	CT_EVENT_F_NEW	= (1 << 0),
	CT_EVENT_F_UPD	= (1 << 1),
	CT_EVENT_F_DEL 	= (1 << 2),
	CT_EVENT_F_ALL	= CT_EVENT_F_NEW | CT_EVENT_F_UPD | CT_EVENT_F_DEL,
};

static struct parse_parameter {
	const char	*parameter[6];
	size_t  size;
	unsigned int value[6];
} parse_array[PARSE_MAX] = {
	{ {"ASSURED", "SEEN_REPLY", "UNSET", "FIXED_TIMEOUT", "EXPECTED"}, 5,
	  { IPS_ASSURED, IPS_SEEN_REPLY, 0, IPS_FIXED_TIMEOUT, IPS_EXPECTED} },
	{ {"ALL", "NEW", "UPDATES", "DESTROY"}, 4,
	  { CT_EVENT_F_ALL, CT_EVENT_F_NEW, CT_EVENT_F_UPD, CT_EVENT_F_DEL } },
	{ {"xml", "extended", "timestamp", "id", "ktimestamp", "labels", }, 6,
	  { _O_XML, _O_EXT, _O_TMS, _O_ID, _O_KTMS, _O_CL },
	},
};

static int
do_parse_parameter(const char *str, size_t str_length, unsigned int *value, 
		   int parse_type)
{
	size_t i;
	int ret = 0;
	struct parse_parameter *p = &parse_array[parse_type];

	if (strncasecmp(str, "SRC_NAT", str_length) == 0) {
		fprintf(stderr, "WARNING: ignoring SRC_NAT, "
				"use --src-nat instead\n");
		return 1;
	}

	if (strncasecmp(str, "DST_NAT", str_length) == 0) {
		fprintf(stderr, "WARNING: ignoring DST_NAT, "
				"use --dst-nat instead\n");
		return 1;
	}

	for (i = 0; i < p->size; i++)
		if (strncasecmp(str, p->parameter[i], str_length) == 0) {
			*value |= p->value[i];
			ret = 1;
			break;
		}
	
	return ret;
}

static void
parse_parameter(const char *arg, unsigned int *status, int parse_type)
{
	const char *comma;

	while ((comma = strchr(arg, ',')) != NULL) {
		if (comma == arg 
		    || !do_parse_parameter(arg, comma-arg, status, parse_type))
			exit_error(PARAMETER_PROBLEM,"Bad parameter `%s'", arg);
		arg = comma+1;
	}

	if (strlen(arg) == 0
	    || !do_parse_parameter(arg, strlen(arg), status, parse_type))
		exit_error(PARAMETER_PROBLEM, "Bad parameter `%s'", arg);
}

static void
parse_u32_mask(const char *arg, struct u32_mask *m)
{
	char *end;

	m->value = (uint32_t) strtoul(arg, &end, 0);

	if (*end == '/')
		m->mask = (uint32_t) strtoul(end+1, NULL, 0);
	else
		m->mask = ~0;
}

static int
get_label(char *name)
{
	int bit = nfct_labelmap_get_bit(labelmap, name);
	if (bit < 0)
		exit_error(PARAMETER_PROBLEM, "unknown label '%s'", name);
	return bit;
}

static void
set_label(struct nfct_bitmask *b, char *name)
{
	int bit = get_label(name);
	nfct_bitmask_set_bit(b, bit);
}

static unsigned int
set_max_label(char *name, unsigned int current_max)
{
	int bit = get_label(name);
	if ((unsigned int) bit > current_max)
		return (unsigned int) bit;
	return current_max;
}

static unsigned int
parse_label_get_max(char *arg)
{
	unsigned int max = 0;
	char *parse;

	while ((parse = strchr(arg, ',')) != NULL) {
		parse[0] = '\0';
		max = set_max_label(arg, max);
		arg = &parse[1];
	}

	max = set_max_label(arg, max);
	return max;
}

static void
parse_label(struct nfct_bitmask *b, char *arg)
{
	char * parse;
	while ((parse = strchr(arg, ',')) != NULL) {
		parse[0] = '\0';
		set_label(b, arg);
		arg = &parse[1];
	}
	set_label(b, arg);
}

static void
add_command(unsigned int *cmd, const int newcmd)
{
	if (*cmd)
		exit_error(PARAMETER_PROBLEM, "Invalid commands combination");
	*cmd |= newcmd;
}

static char *get_optional_arg(int argc, char *argv[])
{
	char *arg = NULL;

	/* Nasty bug or feature in getopt_long ?
	 * It seems that it behaves badly with optional arguments.
	 * Fortunately, I just stole the fix from iptables ;) */
	if (optarg)
		return arg;
	else if (optind < argc && argv[optind][0] != '-' &&
		 argv[optind][0] != '!')
		arg = argv[optind++];

	return arg;
}

enum {
	CT_TABLE_CONNTRACK,
	CT_TABLE_EXPECT,
	CT_TABLE_DYING,
	CT_TABLE_UNCONFIRMED,
};

static unsigned int check_type(int argc, char *argv[])
{
	const char *table = get_optional_arg(argc, argv);

	/* default to conntrack subsystem if nothing has been specified. */
	if (table == NULL)
		return CT_TABLE_CONNTRACK;

	if (strncmp("expect", table, strlen(table)) == 0)
		return CT_TABLE_EXPECT;
	else if (strncmp("conntrack", table, strlen(table)) == 0)
		return CT_TABLE_CONNTRACK;
	else if (strncmp("dying", table, strlen(table)) == 0)
		return CT_TABLE_DYING;
	else if (strncmp("unconfirmed", table, strlen(table)) == 0)
		return CT_TABLE_UNCONFIRMED;
	else
		exit_error(PARAMETER_PROBLEM, "unknown type `%s'", table);

	return 0;
}

static void set_family(int *family, int new)
{
	if (*family == AF_UNSPEC)
		*family = new;
	else if (*family != new)
		exit_error(PARAMETER_PROBLEM, "mismatched address family");
}

struct addr_parse {
	struct in_addr addr;
	struct in6_addr addr6;
	unsigned int family;
};

static int
parse_inetaddr(const char *cp, struct addr_parse *parse)
{
	if (inet_aton(cp, &parse->addr))
		return AF_INET;
	else if (inet_pton(AF_INET6, cp, &parse->addr6) > 0)
		return AF_INET6;
	return AF_UNSPEC;
}

static int
parse_addr(const char *cp, union ct_address *address, int *mask)
{
	char buf[INET6_ADDRSTRLEN];
	struct addr_parse parse;
	char *slash, *end;
	int family;

	strncpy((char *) &buf, cp, INET6_ADDRSTRLEN);
	buf[INET6_ADDRSTRLEN - 1] = '\0';

	if (mask != NULL) {
		slash = strchr(buf, '/');
		if (slash != NULL) {
			*mask = strtol(slash + 1, &end, 10);
			if (*mask < 0 || end != slash + strlen(slash))
				*mask = -2; /* invalid netmask */
			slash[0] = '\0';
		} else {
			*mask = -1; /* no netmask */
		}
	}

	family = parse_inetaddr(buf, &parse);
	switch (family) {
	case AF_INET:
		address->v4 = parse.addr.s_addr;
		if (mask != NULL && *mask > 32)
			*mask = -2; /* invalid netmask */
		break;
	case AF_INET6:
		memcpy(address->v6, &parse.addr6, sizeof(parse.addr6));
		if (mask != NULL && *mask > 128)
			*mask = -2; /* invalid netmask */
		break;
	}

	return family;
}

static bool
valid_port(char *cursor)
{
	const char *str = cursor;
	/* Missing port number */
	if (!*str)
		return false;

	/* Must be entirely digits - no spaces or +/- */
	while (*cursor) {
		if (!isdigit(*cursor))
			return false;
		else
			++cursor;
	}

	/* Must be in range */
	errno = 0;
	long port = strtol(str, NULL, 10);

	if ((errno == ERANGE && (port == LONG_MAX || port == LONG_MIN))
		|| (errno != 0 && port == 0) || (port > USHRT_MAX))
		return false;

	return true;
}

static void
split_address_and_port(const char *arg, char **address, char **port_str)
{
	char *cursor = strchr(arg, '[');

	if (cursor) {
		/* IPv6 address with port*/
		char *start = cursor + 1;

		cursor = strchr(start, ']');
		if (start == cursor) {
			exit_error(PARAMETER_PROBLEM,
				   "No IPv6 address specified");
		} else if (!cursor) {
			exit_error(PARAMETER_PROBLEM,
				   "No closing ']' around IPv6 address");
		}
		size_t len = cursor - start;

		cursor = strchr(cursor, ':');
		if (cursor) {
			/* Copy address only if there is a port */
			*address = strndup(start, len);
		}
	} else {
		cursor = strchr(arg, ':');
		if (cursor && !strchr(cursor + 1, ':')) {
			/* IPv4 address with port */
			*address = strndup(arg, cursor - arg);
		} else {
			/* v6 address */
			cursor = NULL;
		}
	}
	if (cursor) {
		/* Parse port entry */
		cursor++;
		if (strlen(cursor) == 0) {
			exit_error(PARAMETER_PROBLEM,
				   "No port specified after `:'");
		}
		if (!valid_port(cursor)) {
			exit_error(PARAMETER_PROBLEM,
				   "Invalid port `%s'", cursor);
		}
		*port_str = strdup(cursor);
	} else {
		/* No port colon or more than one colon (ipv6)
		 * assume arg is straight IP address and no port
		 */
		*address = strdup(arg);
	}
}

static void
usage(char *prog)
{
	fprintf(stdout, "Command line interface for the connection "
			"tracking system. Version %s\n", VERSION);
	fprintf(stdout, "Usage: %s [commands] [options]\n", prog);

	fprintf(stdout, "\n%s", usage_commands);
	fprintf(stdout, "\n%s", usage_tables);
	fprintf(stdout, "\n%s", usage_conntrack_parameters);
	fprintf(stdout, "\n%s", usage_expectation_parameters);
	fprintf(stdout, "\n%s", usage_update_parameters);
	fprintf(stdout, "\n%s\n", usage_parameters);
}

static unsigned int output_mask;

static int
filter_label(const struct nf_conntrack *ct)
{
	if (tmpl.label == NULL)
		return 0;

	const struct nfct_bitmask *ctb = nfct_get_attr(ct, ATTR_CONNLABELS);
	if (ctb == NULL)
		return 1;

	for (unsigned int i = 0; i <= nfct_bitmask_maxbit(tmpl.label); i++) {
		if (nfct_bitmask_test_bit(tmpl.label, i) &&
		    !nfct_bitmask_test_bit(ctb, i))
				return 1;
	}

	return 0;
}

static int
filter_mark(const struct nf_conntrack *ct)
{
	if ((options & CT_OPT_MARK) &&
	     !mark_cmp(&tmpl.mark, ct))
		return 1;
	return 0;
}

static int 
filter_nat(const struct nf_conntrack *obj, const struct nf_conntrack *ct)
{
	int check_srcnat = options & CT_OPT_SRC_NAT ? 1 : 0;
	int check_dstnat = options & CT_OPT_DST_NAT ? 1 : 0;
	int has_srcnat = 0, has_dstnat = 0;
	uint32_t ip;
	uint16_t port;

	if (options & CT_OPT_ANY_NAT)
		check_srcnat = check_dstnat = 1;

	if (check_srcnat) {
		int check_address = 0, check_port = 0;

		if (nfct_attr_is_set(obj, ATTR_SNAT_IPV4)) {
			check_address = 1;
			ip = nfct_get_attr_u32(obj, ATTR_SNAT_IPV4);
			if (nfct_getobjopt(ct, NFCT_GOPT_IS_SNAT) &&
			    ip == nfct_get_attr_u32(ct, ATTR_REPL_IPV4_DST))
				has_srcnat = 1;
		}
		if (nfct_attr_is_set(obj, ATTR_SNAT_PORT)) {
			int ret = 0;

			check_port = 1;
			port = nfct_get_attr_u16(obj, ATTR_SNAT_PORT);
			if (nfct_getobjopt(ct, NFCT_GOPT_IS_SPAT) &&
			    port == nfct_get_attr_u16(ct, ATTR_REPL_PORT_DST))
				ret = 1;

			/* the address matches but the port does not. */
			if (check_address && has_srcnat && !ret)
				has_srcnat = 0;
			if (!check_address && ret)
				has_srcnat = 1;
		}
		if (!check_address && !check_port &&
		    (nfct_getobjopt(ct, NFCT_GOPT_IS_SNAT) ||
		     nfct_getobjopt(ct, NFCT_GOPT_IS_SPAT)))
		  	has_srcnat = 1;
	}
	if (check_dstnat) {
		int check_address = 0, check_port = 0;

		if (nfct_attr_is_set(obj, ATTR_DNAT_IPV4)) {
			check_address = 1;
			ip = nfct_get_attr_u32(obj, ATTR_DNAT_IPV4);
			if (nfct_getobjopt(ct, NFCT_GOPT_IS_DNAT) &&
			    ip == nfct_get_attr_u32(ct, ATTR_REPL_IPV4_SRC))
				has_dstnat = 1;
		}
		if (nfct_attr_is_set(obj, ATTR_DNAT_PORT)) {
			int ret = 0;

			check_port = 1;
			port = nfct_get_attr_u16(obj, ATTR_DNAT_PORT);
			if (nfct_getobjopt(ct, NFCT_GOPT_IS_DPAT) &&
			    port == nfct_get_attr_u16(ct, ATTR_REPL_PORT_SRC))
				ret = 1;

			/* the address matches but the port does not. */
			if (check_address && has_dstnat && !ret)
				has_dstnat = 0;
			if (!check_address && ret)
				has_dstnat = 1;
		}
		if (!check_address && !check_port &&
		    (nfct_getobjopt(ct, NFCT_GOPT_IS_DNAT) ||
		     nfct_getobjopt(ct, NFCT_GOPT_IS_DPAT)))
			has_dstnat = 1;
	}
	if (options & CT_OPT_ANY_NAT)
		return !(has_srcnat || has_dstnat);
	else if ((options & CT_OPT_SRC_NAT) && (options & CT_OPT_DST_NAT))
		return !(has_srcnat && has_dstnat);
	else if (options & CT_OPT_SRC_NAT)
		return !has_srcnat;
	else if (options & CT_OPT_DST_NAT)
		return !has_dstnat;

	return 0;
}

static int
nfct_ip6_net_cmp(const union ct_address *addr, const struct ct_network *net)
{
	int i;
	for (i=0;i<4;i++)
		if ((addr->v6[i] & net->netmask.v6[i]) != net->network.v6[i])
			return 1;
	return 0;
}

static int
nfct_ip_net_cmp(int family, const union ct_address *addr,
		const struct ct_network *net)
{
	switch(family) {
	case AF_INET:
		return (addr->v4 & net->netmask.v4) != net->network.v4;
	case AF_INET6:
		return nfct_ip6_net_cmp(addr, net);
	default:
		return 0;
	}
}

static int
nfct_filter_network_direction(const struct nf_conntrack *ct, enum ct_direction dir)
{
	const int family = filter_family;
	const union ct_address *address;
	enum nf_conntrack_attr attr;
	struct ct_network *net = &dir2network[dir];

	if (nfct_get_attr_u8(ct, ATTR_ORIG_L3PROTO) != family)
		return 1;

	attr = famdir2attr[family == AF_INET6][dir];
	address = nfct_get_attr(ct, attr);

	return nfct_ip_net_cmp(family, address, net);
}

static int
filter_network(const struct nf_conntrack *ct)
{
	if (options & CT_OPT_MASK_SRC) {
		if (nfct_filter_network_direction(ct, DIR_SRC))
			return 1;
	}

	if (options & CT_OPT_MASK_DST) {
		if (nfct_filter_network_direction(ct, DIR_DST))
			return 1;
	}
	return 0;
}

static int
nfct_filter(struct nf_conntrack *obj, struct nf_conntrack *ct)
{
	if (filter_nat(obj, ct) ||
	    filter_mark(ct) ||
	    filter_label(ct) ||
	    filter_network(ct))
		return 1;

	if (options & CT_COMPARISON &&
	    !nfct_cmp(obj, ct, NFCT_CMP_ALL | NFCT_CMP_MASK))
		return 1;

	return 0;
}

static FILE*
create_log_file(const char *fn)
{
	return fopen(fn, "w+");
}

static void
close_log_file(FILE *fp)
{
	if (fp)
		fclose(fp);
	fp = NULL;
}

#define ROTATE_COUNT 6
static void
rotate_log_file(const char *fn)
{
	int i;
	struct stat sb;
	char from[80] = {};
	char to[80] = {};

	for (i = ROTATE_COUNT; i>0; ) {
		i--;

		sprintf(from, "%s.%d", fn, i);
		sprintf(to, "%s.%d", fn, i+1);

		if (stat(from, &sb) == 0) {
			if (S_ISREG(sb.st_mode) == 0)
				continue;
		} else {
			continue;
		}
		rename(from, to);
	}

	sprintf(from, "%s", fn);
	sprintf(to, "%s.0", fn);

	rename(from, to);
}

static int counter;
static int dump_xml_header_done = 1;

//static void __attribute__((noreturn))
static void
event_sighandler(int s)
{
#if 1 //Andrew
	switch (s) {
	case SIGUSR1: //kill -SIGUSR1
		//perform log file rotation
	if (dump_xml_header_done == 0) {
			if (options & CT_OPT_LOG && log_fp) {
				fprintf(log_fp, "</conntrack>\n");
				fflush(log_fp);

				//write to remote syslog server
				syslog(LOG_INFO, "</conntrack>");
			} else {
		printf("</conntrack>\n");
		fflush(stdout);
	}
		}

		if (options & CT_OPT_LOG && log_fp) {
			close_log_file(log_fp);
			rotate_log_file(log_file);
			create_log_file(log_file);
		}
		break;

	case SIGTERM: //kill -SIGTERM
	case SIGINT: //kill -SIGINT
		if (dump_xml_header_done == 0) {
			if (options & CT_OPT_LOG && log_fp) {
				fprintf(log_fp, "</conntrack>\n");
				fflush(log_fp);
			} else {
				printf("</conntrack>\n");
				fflush(stdout);
			}
			fprintf(stderr, "%s v%s (conntrack-tools): ", PROGNAME, VERSION);
			fprintf(stderr, "%d flow events have been shown.\n", counter);
		}
		nfct_close(cth);
		if (options & CT_OPT_LOG && log_fp)
			close_log_file(log_fp);
		exit(0);

	default:
		break;
	}

#else
	if (dump_xml_header_done == 0) {
		printf("</conntrack>\n");
		fflush(stdout);
	}

	fprintf(stderr, "%s v%s (conntrack-tools): ", PROGNAME, VERSION);
	fprintf(stderr, "%d flow events have been shown.\n", counter);
	nfct_close(cth);
	if (options & CT_OPT_LOG && log_fp) //Andrew
		close_log_file(log_fp);
	exit(0);
#endif
}

static void __attribute__((noreturn))
exp_event_sighandler(int s)
{
	if (dump_xml_header_done == 0) {
		printf("</expect>\n");
		fflush(stdout);
	}

	fprintf(stderr, "%s v%s (conntrack-tools): ", PROGNAME, VERSION);
	fprintf(stderr, "%d expectation events have been shown.\n", counter);
	nfct_close(cth);
	if (options & CT_OPT_LOG && log_fp) //Andrew
		close_log_file(log_fp);
	exit(0);
}

static int event_cb(enum nf_conntrack_msg_type type,
		    struct nf_conntrack *ct,
		    void *data)
{
	char buf[1024];
	struct nf_conntrack *obj = data;
	unsigned int op_type = NFCT_O_DEFAULT;
	unsigned int op_flags = 0;

	time_t now;
	struct tm *info;
	char t_str[20];

	if (nfct_filter(obj, ct))
		return NFCT_CB_CONTINUE;

	if (output_mask & _O_XML) {
		op_type = NFCT_O_XML;
		if (dump_xml_header_done) {
			dump_xml_header_done = 0;
			printf("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
			       "<conntrack>\n");
		}
	} 
	if (output_mask & _O_EXT)
		op_flags = NFCT_OF_SHOW_LAYER3;
	if (output_mask & _O_TMS) {
		if (!(output_mask & _O_XML)) {
			struct timeval tv;
			gettimeofday(&tv, NULL);
			printf("[%-.8ld.%-.6ld]\t", tv.tv_sec, tv.tv_usec);
		} else
			op_flags |= NFCT_OF_TIME;
	}
	if (output_mask & _O_KTMS)
		op_flags |= NFCT_OF_TIMESTAMP;
	if (output_mask & _O_ID)
		op_flags |= NFCT_OF_ID;

	nfct_snprintf_labels(buf, sizeof(buf), ct, type, op_type, op_flags, labelmap);

	time(&now);
	info = localtime(&now);
	strftime(t_str, sizeof(t_str), "%b %d %H:%M:%S", info);

#if 1 //Andrew
	if (options & CT_OPT_LOG && log_fp) {
		fprintf(log_fp, "%s %s\n", t_str, buf);
		fflush(log_fp);

		//write to remote syslog server
		syslog(LOG_INFO, "%s", buf);
	} else {
		printf("%s\n", buf);
		fflush(stdout);
	}
#else
	printf("%s\n", buf);
	fflush(stdout);
#endif
	counter++;

	return NFCT_CB_CONTINUE;
}

static int dump_cb(enum nf_conntrack_msg_type type,
		   struct nf_conntrack *ct,
		   void *data)
{
	char buf[1024];
	struct nf_conntrack *obj = data;
	unsigned int op_type = NFCT_O_DEFAULT;
	unsigned int op_flags = 0;

	time_t now;
	struct tm *info;
	char t_str[20];

	if (nfct_filter(obj, ct))
		return NFCT_CB_CONTINUE;

	if (output_mask & _O_XML) {
		op_type = NFCT_O_XML;
		if (dump_xml_header_done) {
			dump_xml_header_done = 0;
			printf("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
			       "<conntrack>\n");
		}
	}
	if (output_mask & _O_EXT)
		op_flags = NFCT_OF_SHOW_LAYER3;
	if (output_mask & _O_KTMS)
		op_flags |= NFCT_OF_TIMESTAMP;
	if (output_mask & _O_ID)
		op_flags |= NFCT_OF_ID;

	nfct_snprintf_labels(buf, sizeof(buf), ct, NFCT_T_UNKNOWN, op_type, op_flags, labelmap);

	time(&now);
	info = localtime(&now);
	strftime(t_str, sizeof(t_str), "%b %d %H:%M:%S", info);

#if 1 //Andrew
	if (options & CT_OPT_LOG && log_fp) {
		fprintf(log_fp, "%s %s\n", t_str, buf);
		fflush(log_fp);

		//write to remote syslog server
		syslog(LOG_INFO, "%s", buf);
	} else {
		printf("%s\n", buf);
	}
#else
	printf("%s\n", buf);
#endif
	counter++;

	return NFCT_CB_CONTINUE;
}

static int delete_cb(enum nf_conntrack_msg_type type,
		     struct nf_conntrack *ct,
		     void *data)
{
	int res;
	char buf[1024];
	struct nf_conntrack *obj = data;
	unsigned int op_type = NFCT_O_DEFAULT;
	unsigned int op_flags = 0;

	time_t now;
	struct tm *info;
	char t_str[20];

	if (nfct_filter(obj, ct))
		return NFCT_CB_CONTINUE;

	res = nfct_query(ith, NFCT_Q_DESTROY, ct);
	if (res < 0)
		exit_error(OTHER_PROBLEM,
			   "Operation failed: %s",
			   err2str(errno, CT_DELETE));

	if (output_mask & _O_XML)
		op_type = NFCT_O_XML;
	if (output_mask & _O_EXT)
		op_flags = NFCT_OF_SHOW_LAYER3;
	if (output_mask & _O_ID)
		op_flags |= NFCT_OF_ID;

	nfct_snprintf(buf, sizeof(buf), ct, NFCT_T_UNKNOWN, op_type, op_flags);

	time(&now);
	info = localtime(&now);
	strftime(t_str, sizeof(t_str), "%b %d %H:%M:%S", info);

#if 1 //Andrew
	if (options & CT_OPT_LOG && log_fp) {
		fprintf(log_fp, "%s %s\n", t_str, buf);
		fflush(log_fp);

		//write to remote syslog server
		syslog(LOG_INFO, "%s", buf);
	} else {
		printf("%s\n", buf);
	}
#else
	printf("%s\n", buf);
#endif

	counter++;

	return NFCT_CB_CONTINUE;
}

static int print_cb(enum nf_conntrack_msg_type type,
		    struct nf_conntrack *ct,
		    void *data)
{
	char buf[1024];
	unsigned int op_type = NFCT_O_DEFAULT;
	unsigned int op_flags = 0;

	time_t now;
	struct tm *info;
	char t_str[20];

	if (output_mask & _O_XML)
		op_type = NFCT_O_XML;
	if (output_mask & _O_EXT)
		op_flags = NFCT_OF_SHOW_LAYER3;
	if (output_mask & _O_ID)
		op_flags |= NFCT_OF_ID;

	nfct_snprintf_labels(buf, sizeof(buf), ct, NFCT_T_UNKNOWN, op_type, op_flags, labelmap);

	time(&now);
	info = localtime(&now);
	strftime(t_str, sizeof(t_str), "%b %d %H:%M:%S", info);

#if 1 //Andrew
	if (options & CT_OPT_LOG && log_fp) {
		fprintf(log_fp, "%s %s\n", t_str, buf);
		fflush(log_fp);

		//write to remote syslog server
		syslog(LOG_INFO, "%s", buf);
	} else {
		printf("%s\n", buf);
	}
#else
	printf("%s\n", buf);
#endif
	return NFCT_CB_CONTINUE;
}

static void copy_mark(struct nf_conntrack *tmp,
		      const struct nf_conntrack *ct,
		      const struct u32_mask *m)
{
	if (options & CT_OPT_MARK) {
		uint32_t mark = nfct_get_attr_u32(ct, ATTR_MARK);
		mark = (mark & ~m->mask) ^ m->value;
		nfct_set_attr_u32(tmp, ATTR_MARK, mark);
	}
}

static void copy_status(struct nf_conntrack *tmp, const struct nf_conntrack *ct)
{
	if (options & CT_OPT_STATUS) {
		/* copy existing flags, we only allow setting them. */
		uint32_t status = nfct_get_attr_u32(ct, ATTR_STATUS);
		status |= nfct_get_attr_u32(tmp, ATTR_STATUS);
		nfct_set_attr_u32(tmp, ATTR_STATUS, status);
	}
}

static struct nfct_bitmask *xnfct_bitmask_clone(const struct nfct_bitmask *a)
{
	struct nfct_bitmask *b = nfct_bitmask_clone(a);
	if (!b)
		exit_error(OTHER_PROBLEM, "out of memory");
	return b;
}

static void copy_label(struct nf_conntrack *tmp, const struct nf_conntrack *ct)
{
	struct nfct_bitmask *ctb, *newmask;
	unsigned int i;

	if ((options & (CT_OPT_ADD_LABEL|CT_OPT_DEL_LABEL)) == 0)
		return;

	nfct_copy_attr(tmp, ct, ATTR_CONNLABELS);
	ctb = (void *) nfct_get_attr(tmp, ATTR_CONNLABELS);

	if (options & CT_OPT_ADD_LABEL) {
		if (ctb == NULL) {
			nfct_set_attr(tmp, ATTR_CONNLABELS,
					xnfct_bitmask_clone(tmpl.label_modify));
			return;
		}
		/* If we send a bitmask shorter than the kernel sent to us, the bits we
		 * omit will be cleared (as "padding").  So we always have to send the
		 * same sized bitmask as we received.
		 *
		 * Mask has to have the same size as the labels, otherwise it will not
		 * be encoded by libnetfilter_conntrack, as different sizes are not
		 * accepted by the kernel.
		 */
		newmask = nfct_bitmask_new(nfct_bitmask_maxbit(ctb));

		for (i = 0; i <= nfct_bitmask_maxbit(ctb); i++) {
			if (nfct_bitmask_test_bit(tmpl.label_modify, i)) {
				nfct_bitmask_set_bit(ctb, i);
				nfct_bitmask_set_bit(newmask, i);
			} else if (nfct_bitmask_test_bit(ctb, i)) {
				/* Kernel only retains old bit values that are sent as
				 * zeroes in BOTH labels and mask.
				 */
				nfct_bitmask_unset_bit(ctb, i);
			}
		}
		nfct_set_attr(tmp, ATTR_CONNLABELS_MASK, newmask);
	} else if (ctb != NULL) {
		/* CT_OPT_DEL_LABEL */
		if (tmpl.label_modify == NULL) {
			newmask = nfct_bitmask_new(0);
			if (newmask)
				nfct_set_attr(tmp, ATTR_CONNLABELS, newmask);
			return;
		}

		for (i = 0; i <= nfct_bitmask_maxbit(ctb); i++) {
			if (nfct_bitmask_test_bit(tmpl.label_modify, i))
				nfct_bitmask_unset_bit(ctb, i);
		}

		newmask = xnfct_bitmask_clone(tmpl.label_modify);
		nfct_set_attr(tmp, ATTR_CONNLABELS_MASK, newmask);
	}
}

static int update_cb(enum nf_conntrack_msg_type type,
		     struct nf_conntrack *ct,
		     void *data)
{
	int res;
	struct nf_conntrack *obj = data, *tmp;

	if (filter_nat(obj, ct) ||
	    filter_label(ct) ||
	    filter_network(ct))
		return NFCT_CB_CONTINUE;

	if (nfct_attr_is_set(obj, ATTR_ID) && nfct_attr_is_set(ct, ATTR_ID) &&
	    nfct_get_attr_u32(obj, ATTR_ID) != nfct_get_attr_u32(ct, ATTR_ID))
	    	return NFCT_CB_CONTINUE;

	if (options & CT_OPT_TUPLE_ORIG && !nfct_cmp(obj, ct, NFCT_CMP_ORIG))
		return NFCT_CB_CONTINUE;
	if (options & CT_OPT_TUPLE_REPL && !nfct_cmp(obj, ct, NFCT_CMP_REPL))
		return NFCT_CB_CONTINUE;

	tmp = nfct_new();
	if (tmp == NULL)
		exit_error(OTHER_PROBLEM, "out of memory");

	nfct_copy(tmp, ct, NFCT_CP_ORIG);
	nfct_copy(tmp, obj, NFCT_CP_META);
	copy_mark(tmp, ct, &tmpl.mark);
	copy_status(tmp, ct);
	copy_label(tmp, ct);

	/* do not send NFCT_Q_UPDATE if ct appears unchanged */
	if (nfct_cmp(tmp, ct, NFCT_CMP_ALL | NFCT_CMP_MASK)) {
		nfct_destroy(tmp);
		return NFCT_CB_CONTINUE;
	}

	res = nfct_query(ith, NFCT_Q_UPDATE, tmp);
	if (res < 0)
		fprintf(stderr,
			   "Operation failed: %s\n",
			   err2str(errno, CT_UPDATE));
	nfct_callback_register(ith, NFCT_T_ALL, print_cb, NULL);

	res = nfct_query(ith, NFCT_Q_GET, tmp);
	if (res < 0) {
		nfct_destroy(tmp);
		/* the entry has vanish in middle of the update */
		if (errno == ENOENT) {
			nfct_callback_unregister(ith);
			return NFCT_CB_CONTINUE;
		}
		exit_error(OTHER_PROBLEM,
			   "Operation failed: %s",
			   err2str(errno, CT_UPDATE));
	}
	nfct_destroy(tmp);
	nfct_callback_unregister(ith);

	counter++;

	return NFCT_CB_CONTINUE;
}

static int dump_exp_cb(enum nf_conntrack_msg_type type,
		      struct nf_expect *exp,
		      void *data)
{
	char buf[1024];
	unsigned int op_type = NFCT_O_DEFAULT;
	unsigned int op_flags = 0;

	if (output_mask & _O_XML) {
		op_type = NFCT_O_XML;
		if (dump_xml_header_done) {
			dump_xml_header_done = 0;
			printf("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
			       "<expect>\n");
		}
	}
	if (output_mask & _O_TMS) {
		if (!(output_mask & _O_XML)) {
			struct timeval tv;
			gettimeofday(&tv, NULL);
			printf("[%-8ld.%-6ld]\t", tv.tv_sec, tv.tv_usec);
		} else
			op_flags |= NFCT_OF_TIME;
	}

	nfexp_snprintf(buf,sizeof(buf), exp, NFCT_T_UNKNOWN, op_type, op_flags);
	printf("%s\n", buf);
	counter++;

	return NFCT_CB_CONTINUE;
}

static int event_exp_cb(enum nf_conntrack_msg_type type,
			struct nf_expect *exp, void *data)
{
	char buf[1024];
	unsigned int op_type = NFCT_O_DEFAULT;
	unsigned int op_flags = 0;

	if (output_mask & _O_XML) {
		op_type = NFCT_O_XML;
		if (dump_xml_header_done) {
			dump_xml_header_done = 0;
			printf("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
			       "<expect>\n");
		}
	}
	if (output_mask & _O_TMS) {
		if (!(output_mask & _O_XML)) {
			struct timeval tv;
			gettimeofday(&tv, NULL);
			printf("[%-8ld.%-6ld]\t", tv.tv_sec, tv.tv_usec);
		} else
			op_flags |= NFCT_OF_TIME;
	}

	nfexp_snprintf(buf,sizeof(buf), exp, type, op_type, op_flags);
	printf("%s\n", buf);
	fflush(stdout);
	counter++;

	return NFCT_CB_CONTINUE;
}

static int count_exp_cb(enum nf_conntrack_msg_type type,
			struct nf_expect *exp,
			void *data)
{
	counter++;
	return NFCT_CB_CONTINUE;
}

#ifndef CT_STATS_PROC
#define CT_STATS_PROC "/proc/net/stat/nf_conntrack"
#endif

/* As of 2.6.29, we have 16 entries, this is enough */
#ifndef CT_STATS_ENTRIES_MAX
#define CT_STATS_ENTRIES_MAX 64
#endif

/* maximum string length currently is 13 characters */
#ifndef CT_STATS_STRING_MAX
#define CT_STATS_STRING_MAX 64
#endif

static int display_proc_conntrack_stats(void)
{
	int ret = 0;
	FILE *fd;
	char buf[4096], *token, *nl;
	char output[CT_STATS_ENTRIES_MAX][CT_STATS_STRING_MAX];
	unsigned int value[CT_STATS_ENTRIES_MAX], i, max;
	int cpu;

	fd = fopen(CT_STATS_PROC, "r");
	if (fd == NULL)
		return -1;

	if (fgets(buf, sizeof(buf), fd) == NULL) {
		ret = -1;
		goto out_err;
	}

	/* trim off trailing \n */
	nl = strchr(buf, '\n');
	if (nl != NULL)
		*nl = '\0';

	token = strtok(buf, " ");
	for (i=0; token != NULL && i<CT_STATS_ENTRIES_MAX; i++) {
		strncpy(output[i], token, CT_STATS_STRING_MAX);
		output[i][CT_STATS_STRING_MAX-1]='\0';
		token = strtok(NULL, " ");
	}
	max = i;

	for (cpu = 0; fgets(buf, sizeof(buf), fd) != NULL; cpu++) {
		nl = strchr(buf, '\n');
		while (nl != NULL) {
			*nl = '\0';
			nl = strchr(buf, '\n');
		}
		token = strtok(buf, " ");
		for (i = 0; token != NULL && i < CT_STATS_ENTRIES_MAX; i++) {
			value[i] = (unsigned int) strtol(token, (char**) NULL, 16);
			token = strtok(NULL, " ");
		}

		printf("cpu=%-4u\t", cpu);
		for (i = 0; i < max; i++)
			printf("%s=%u ", output[i], value[i]);
		printf("\n");
	}
	if (cpu == 0)
		ret = -1;

out_err:
	fclose(fd);
	return ret;
}

static struct nfct_mnl_socket {
	struct mnl_socket	*mnl;
	uint32_t		portid;
} sock;

static int nfct_mnl_socket_open(void)
{
	sock.mnl = mnl_socket_open(NETLINK_NETFILTER);
	if (sock.mnl == NULL) {
		perror("mnl_socket_open");
		return -1;
	}
	if (mnl_socket_bind(sock.mnl, 0, MNL_SOCKET_AUTOPID) < 0) {
		perror("mnl_socket_bind");
		return -1;
	}
	sock.portid = mnl_socket_get_portid(sock.mnl);

	return 0;
}

static struct nlmsghdr *
nfct_mnl_nlmsghdr_put(char *buf, uint16_t subsys, uint16_t type,
		      uint8_t family)
{
	struct nlmsghdr *nlh;
	struct nfgenmsg *nfh;

	nlh = mnl_nlmsg_put_header(buf);
	nlh->nlmsg_type = (subsys << 8) | type;
	nlh->nlmsg_flags = NLM_F_REQUEST|NLM_F_DUMP;
	nlh->nlmsg_seq = time(NULL);

	nfh = mnl_nlmsg_put_extra_header(nlh, sizeof(struct nfgenmsg));
	nfh->nfgen_family = family;
	nfh->version = NFNETLINK_V0;
	nfh->res_id = 0;

	return nlh;
}

static void nfct_mnl_socket_close(void)
{
	mnl_socket_close(sock.mnl);
}

static int
nfct_mnl_dump(uint16_t subsys, uint16_t type, mnl_cb_t cb, uint8_t family)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	int res;

	nlh = nfct_mnl_nlmsghdr_put(buf, subsys, type, family);

	res = mnl_socket_sendto(sock.mnl, nlh, nlh->nlmsg_len);
	if (res < 0)
		return res;

	res = mnl_socket_recvfrom(sock.mnl, buf, sizeof(buf));
	while (res > 0) {
		res = mnl_cb_run(buf, res, nlh->nlmsg_seq, sock.portid,
					 cb, NULL);
		if (res <= MNL_CB_STOP)
			break;

		res = mnl_socket_recvfrom(sock.mnl, buf, sizeof(buf));
	}

	return res;
}

static int
nfct_mnl_get(uint16_t subsys, uint16_t type, mnl_cb_t cb, uint8_t family)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	int res;

	nlh = nfct_mnl_nlmsghdr_put(buf, subsys, type, family);

	res = mnl_socket_sendto(sock.mnl, nlh, nlh->nlmsg_len);
	if (res < 0)
		return res;

	res = mnl_socket_recvfrom(sock.mnl, buf, sizeof(buf));
	if (res < 0)
		return res;

	return mnl_cb_run(buf, res, nlh->nlmsg_seq, sock.portid, cb, NULL);
}

static int nfct_stats_attr_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, CTA_STATS_MAX) < 0)
		return MNL_CB_OK;

	if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0) {
		perror("mnl_attr_validate");
		return MNL_CB_ERROR;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static int nfct_stats_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nlattr *tb[CTA_STATS_MAX+1] = {};
	struct nfgenmsg *nfg = mnl_nlmsg_get_payload(nlh);
	const char *attr2name[CTA_STATS_MAX+1] = {
		[CTA_STATS_SEARCHED]	= "searched",
		[CTA_STATS_FOUND]	= "found",
		[CTA_STATS_NEW]		= "new",
		[CTA_STATS_INVALID]	= "invalid",
		[CTA_STATS_IGNORE]	= "ignore",
		[CTA_STATS_DELETE]	= "delete",
		[CTA_STATS_DELETE_LIST]	= "delete_list",
		[CTA_STATS_INSERT]	= "insert",
		[CTA_STATS_INSERT_FAILED] = "insert_failed",
		[CTA_STATS_DROP]	= "drop",
		[CTA_STATS_EARLY_DROP]	= "early_drop",
		[CTA_STATS_ERROR]	= "error",
		[CTA_STATS_SEARCH_RESTART] = "search_restart",
	};
	int i;

	mnl_attr_parse(nlh, sizeof(*nfg), nfct_stats_attr_cb, tb);

	printf("cpu=%-4u\t", ntohs(nfg->res_id));

	for (i=0; i<CTA_STATS_MAX+1; i++) {
		if (tb[i]) {
			printf("%s=%u ",
				attr2name[i], ntohl(mnl_attr_get_u32(tb[i])));
		}
	}
	printf("\n");
	return MNL_CB_OK;
}

static int nfexp_stats_attr_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, CTA_STATS_EXP_MAX) < 0)
		return MNL_CB_OK;

	if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0) {
		perror("mnl_attr_validate");
		return MNL_CB_ERROR;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static int nfexp_stats_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nlattr *tb[CTA_STATS_EXP_MAX+1] = {};
	struct nfgenmsg *nfg = mnl_nlmsg_get_payload(nlh);
	const char *attr2name[CTA_STATS_EXP_MAX+1] = {
		[CTA_STATS_EXP_NEW] = "expect_new",
		[CTA_STATS_EXP_CREATE] = "expect_create",
		[CTA_STATS_EXP_DELETE] = "expect_delete",
	};
	int i;

	mnl_attr_parse(nlh, sizeof(*nfg), nfexp_stats_attr_cb, tb);

	printf("cpu=%-4u\t", ntohs(nfg->res_id));

	for (i=0; i<CTA_STATS_EXP_MAX+1; i++) {
		if (tb[i]) {
			printf("%s=%u ",
				attr2name[i], ntohl(mnl_attr_get_u32(tb[i])));
		}
	}
	printf("\n");
	return MNL_CB_OK;
}

static int nfct_stats_global_attr_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, CTA_STATS_GLOBAL_MAX) < 0)
		return MNL_CB_OK;

	if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0) {
		perror("mnl_attr_validate");
		return MNL_CB_ERROR;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static int nfct_global_stats_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nlattr *tb[CTA_STATS_GLOBAL_MAX+1] = {};
	struct nfgenmsg *nfg = mnl_nlmsg_get_payload(nlh);

	mnl_attr_parse(nlh, sizeof(*nfg), nfct_stats_global_attr_cb, tb);

	if (tb[CTA_STATS_GLOBAL_ENTRIES]) {
		printf("%d\n",
			ntohl(mnl_attr_get_u32(tb[CTA_STATS_GLOBAL_ENTRIES])));
	}
	return MNL_CB_OK;
}

static int mnl_nfct_dump_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nf_conntrack *ct;
	char buf[4096];

	ct = nfct_new();
	if (ct == NULL)
		return MNL_CB_OK;

	nfct_nlmsg_parse(nlh, ct);

	nfct_snprintf(buf, sizeof(buf), ct, NFCT_T_UNKNOWN, NFCT_O_DEFAULT, 0);
	printf("%s\n", buf);

	nfct_destroy(ct);

	counter++;

	return MNL_CB_OK;
}

static struct ctproto_handler *h;

static void labelmap_init(void)
{
	if (labelmap)
		return;
	labelmap = nfct_labelmap_new(NULL);
	if (!labelmap)
		perror("nfct_labelmap_new");
}

static void
nfct_network_attr_prepare(const int family, enum ct_direction dir)
{
	const union ct_address *address, *netmask;
	enum nf_conntrack_attr attr;
	int i;
	struct ct_network *net = &dir2network[dir];

	attr = famdir2attr[family == AF_INET6][dir];

	address = nfct_get_attr(tmpl.ct, attr);
	netmask = nfct_get_attr(tmpl.mask, attr);

	switch(family) {
	case AF_INET:
		net->network.v4 = address->v4 & netmask->v4;
		break;
	case AF_INET6:
		for (i=0;i<4;i++)
			net->network.v6[i] = address->v6[i] & netmask->v6[i];
		break;
	}

	memcpy(&net->netmask, netmask, sizeof(union ct_address));

	/* avoid exact source matching */
	nfct_attr_unset(tmpl.ct, attr);
}

static void
nfct_filter_init(const int family)
{
	filter_family = family;
	if (options & CT_OPT_MASK_SRC) {
		if (!(options & CT_OPT_ORIG_SRC))
			exit_error(PARAMETER_PROBLEM,
			           "Can't use --mask-src without --src");
		nfct_network_attr_prepare(family, DIR_SRC);
	}

	if (options & CT_OPT_MASK_DST) {
		if (!(options & CT_OPT_ORIG_DST))
			exit_error(PARAMETER_PROBLEM,
			           "Can't use --mask-dst without --dst");
		nfct_network_attr_prepare(family, DIR_DST);
	}
}

static void merge_bitmasks(struct nfct_bitmask **current,
			  struct nfct_bitmask *src)
{
	unsigned int i;

	if (*current == NULL) {
		*current = src;
		return;
	}

	/* "current" must be the larger bitmask object */
	if (nfct_bitmask_maxbit(src) > nfct_bitmask_maxbit(*current)) {
		struct nfct_bitmask *tmp = *current;
		*current = src;
		src = tmp;
	}

	for (i = 0; i <= nfct_bitmask_maxbit(src); i++) {
		if (nfct_bitmask_test_bit(src, i))
			nfct_bitmask_set_bit(*current, i);
	}

	nfct_bitmask_destroy(src);
}


static void
nfct_build_netmask(uint32_t *dst, int b, int n)
{
	int i;

	for (i = 0; i < n; i++) {
		if (b >= 32) {
			dst[i] = 0xffffffff;
			b -= 32;
		} else if (b > 0) {
			dst[i] = (1 << b) - 1;
			b = 0;
		} else {
			dst[i] = 0;
		}
	}
}

static void
nfct_set_addr_only(const int opt, struct nf_conntrack *ct, union ct_address *ad,
		   const int l3protonum)
{
	switch (l3protonum) {
	case AF_INET:
		nfct_set_attr_u32(ct,
		                  opt2family_attr[opt][0],
		                  ad->v4);
		break;
	case AF_INET6:
		nfct_set_attr(ct,
		              opt2family_attr[opt][1],
		              &ad->v6);
		break;
	}
}

static void
nfct_set_addr_opt(const int opt, struct nf_conntrack *ct, union ct_address *ad,
		  const int l3protonum)
{
	options |= opt2type[opt];
	nfct_set_addr_only(opt, ct, ad, l3protonum);
	nfct_set_attr_u8(ct, opt2attr[opt], l3protonum);
}

static void
nfct_parse_addr_from_opt(const int opt, const char *arg,
			 struct nf_conntrack *ct,
			 struct nf_conntrack *ctmask,
			 union ct_address *ad, int *family)
{
	int mask, maskopt;

	const int l3protonum = parse_addr(arg, ad, &mask);
	if (l3protonum == AF_UNSPEC) {
		exit_error(PARAMETER_PROBLEM,
			   "Invalid IP address `%s'", arg);
	}
	set_family(family, l3protonum);
	maskopt = opt2maskopt[opt];
	if (mask != -1 && !maskopt) {
		exit_error(PARAMETER_PROBLEM,
		           "CIDR notation unavailable"
		           " for `--%s'", get_long_opt(opt));
	} else if (mask == -2) {
		exit_error(PARAMETER_PROBLEM,
		           "Invalid netmask");
	}

	nfct_set_addr_opt(opt, ct, ad, l3protonum);

	/* bail if we don't have a netmask to set*/
	if (mask == -1 || !maskopt || ctmask == NULL)
		return;

	switch(l3protonum) {
	case AF_INET:
		if (mask == 32)
			return;
		nfct_build_netmask(&ad->v4, mask, 1);
		break;
	case AF_INET6:
		if (mask == 128)
			return;
		nfct_build_netmask((uint32_t *) &ad->v6, mask, 4);
		break;
	}

	nfct_set_addr_opt(maskopt, ctmask, ad, l3protonum);
}

static void
nfct_set_nat_details(const int opt, struct nf_conntrack *ct,
		     union ct_address *ad, const char *port_str,
		     const int family)
{
	const int type = opt2type[opt];

	nfct_set_addr_only(opt, ct, ad, family);
	if (port_str && type == CT_OPT_SRC_NAT) {
		nfct_set_attr_u16(ct, ATTR_SNAT_PORT,
				  ntohs((uint16_t)atoi(port_str)));
	} else if (port_str && type == CT_OPT_DST_NAT) {
		nfct_set_attr_u16(ct, ATTR_DNAT_PORT,
				  ntohs((uint16_t)atoi(port_str)));
	}

}

int main(int argc, char *argv[])
{
	int c, cmd;
	unsigned int type = 0, event_mask = 0, l4flags = 0, status = 0;
	int res = 0, partial = -10;
	size_t socketbuffersize = 0;
	int family = AF_UNSPEC;
	int protonum = 0;
	union ct_address ad;
	unsigned int command = 0;

	/* we release these objects in the exit_error() path. */
	if (!alloc_tmpl_objects())
		exit_error(OTHER_PROBLEM, "out of memory");

	register_tcp();
	register_udp();
	register_udplite();
	register_sctp();
	register_dccp();
	register_icmp();
	register_icmpv6();
	register_gre();
	register_unknown();

	/* disable explicit missing arguments error output from getopt_long */
	opterr = 0;

	while ((c = getopt_long(argc, argv, getopt_str, opts, NULL)) != -1) {
	switch(c) {
		/* commands */
		case 'L':
			type = check_type(argc, argv);
			/* Special case: dumping dying and unconfirmed list
			 * are handled like normal conntrack dumps.
			 */
			if (type == CT_TABLE_DYING ||
			    type == CT_TABLE_UNCONFIRMED)
				add_command(&command, cmd2type[c][0]);
			else
				add_command(&command, cmd2type[c][type]);
			break;
		case 'I':
		case 'D':
		case 'G':
		case 'F':
		case 'E':
		case 'V':
		case 'h':
		case 'C':
		case 'S':
			type = check_type(argc, argv);
			if (type == CT_TABLE_DYING ||
			    type == CT_TABLE_UNCONFIRMED) {
				exit_error(PARAMETER_PROBLEM,
					   "Can't do that command with "
					   "tables `dying' and `unconfirmed'");
			}
			add_command(&command, cmd2type[c][type]);
			break;
		case 'U':
			type = check_type(argc, argv);
			if (type == CT_TABLE_DYING ||
			    type == CT_TABLE_UNCONFIRMED) {
				exit_error(PARAMETER_PROBLEM,
					   "Can't do that command with "
					   "tables `dying' and `unconfirmed'");
			} else if (type == CT_TABLE_CONNTRACK)
				add_command(&command, CT_UPDATE);
			else
				exit_error(PARAMETER_PROBLEM,
					   "Can't update expectations");
			break;
		/* options */
		case 's':
		case 'd':
		case 'r':
		case 'q':
			nfct_parse_addr_from_opt(c, optarg, tmpl.ct,
						 tmpl.mask, &ad, &family);
			break;
		case '[':
		case ']':
			nfct_parse_addr_from_opt(c, optarg, tmpl.exptuple,
						 tmpl.mask, &ad, &family);
			break;
		case '{':
		case '}':
			nfct_parse_addr_from_opt(c, optarg, tmpl.mask,
						 NULL, &ad, &family);
			break;
		case 'p':
			options |= CT_OPT_PROTO;
			h = findproto(optarg, &protonum);
			if (!h)
				exit_error(PARAMETER_PROBLEM,
					   "`%s' unsupported protocol",
					   optarg);

			opts = merge_options(opts, h->opts, &h->option_offset);
			if (opts == NULL)
				exit_error(OTHER_PROBLEM, "out of memory");

			nfct_set_attr_u8(tmpl.ct, ATTR_L4PROTO, protonum);
			break;
		case 't':
			options |= CT_OPT_TIMEOUT;
			nfct_set_attr_u32(tmpl.ct, ATTR_TIMEOUT, atol(optarg));
			nfexp_set_attr_u32(tmpl.exp,
					   ATTR_EXP_TIMEOUT, atol(optarg));
			break;
		case 'u':
			options |= CT_OPT_STATUS;
			parse_parameter(optarg, &status, PARSE_STATUS);
			nfct_set_attr_u32(tmpl.ct, ATTR_STATUS, status);
			break;
		case 'e':
			options |= CT_OPT_EVENT_MASK;
			parse_parameter(optarg, &event_mask, PARSE_EVENT);
			break;
		case 'o':
			options |= CT_OPT_OUTPUT;
			parse_parameter(optarg, &output_mask, PARSE_OUTPUT);
			if (output_mask & _O_CL)
				labelmap_init();
			break;
		case 'z':
			options |= CT_OPT_ZERO;
			break;
		case 'n':
		case 'g':
		case 'j':
			options |= opt2type[c];
			char *optional_arg = get_optional_arg(argc, argv);

			if (optional_arg) {
				char *port_str = NULL;
				char *nat_address = NULL;

				split_address_and_port(optional_arg,
						       &nat_address,
						       &port_str);
				nfct_parse_addr_from_opt(c, nat_address,
							 tmpl.ct, NULL,
							 &ad, &family);
				if (c == 'j') {
					/* Set details on both src and dst
					 * with any-nat
					 */
					nfct_set_nat_details('g', tmpl.ct, &ad,
							     port_str, family);
					nfct_set_nat_details('n', tmpl.ct, &ad,
							     port_str, family);
				} else {
					nfct_set_nat_details(c, tmpl.ct, &ad,
							     port_str, family);
				}
			}
			break;
		case 'w':
		case '(':
		case ')':
			options |= opt2type[c];
			nfct_set_attr_u16(tmpl.ct,
					  opt2attr[c],
					  strtoul(optarg, NULL, 0));
			break;
		case 'i':
		case 'c':
			options |= opt2type[c];
			nfct_set_attr_u32(tmpl.ct,
					  opt2attr[c],
					  strtoul(optarg, NULL, 0));
			break;
		case 'm':
			options |= opt2type[c];
			parse_u32_mask(optarg, &tmpl.mark);
			tmpl.filter_mark_kernel.val = tmpl.mark.value;
			tmpl.filter_mark_kernel.mask = tmpl.mark.mask;
			tmpl.filter_mark_kernel_set = true;
			break;
		case 'l':
		case '<':
		case '>':
			options |= opt2type[c];

			labelmap_init();

			if ((options & (CT_OPT_DEL_LABEL|CT_OPT_ADD_LABEL)) ==
			    (CT_OPT_DEL_LABEL|CT_OPT_ADD_LABEL))
				exit_error(OTHER_PROBLEM, "cannot use --label-add and "
							"--label-del at the same time");

			if (c == '>') { /* DELETE */
				char *tmp = get_optional_arg(argc, argv);
				if (tmp == NULL) /* delete all labels */
					break;
				optarg = tmp;
			}

			char *optarg2 = strdup(optarg);
			unsigned int max = parse_label_get_max(optarg);
			struct nfct_bitmask * b = nfct_bitmask_new(max);
			if (!b)
				exit_error(OTHER_PROBLEM, "out of memory");

			parse_label(b, optarg2);

			/* join "-l foo -l bar" into single bitmask object */
			if (c == 'l') {
				merge_bitmasks(&tmpl.label, b);
			} else {
				merge_bitmasks(&tmpl.label_modify, b);
			}

			free(optarg2);
			break;
		case 'a':
			fprintf(stderr, "WARNING: ignoring -%c, "
					"deprecated option.\n", c);
			break;
		case 'f':
			options |= CT_OPT_FAMILY;
			if (strncmp(optarg, "ipv4", strlen("ipv4")) == 0)
				set_family(&family, AF_INET);
			else if (strncmp(optarg, "ipv6", strlen("ipv6")) == 0)
				set_family(&family, AF_INET6);
			else
				exit_error(PARAMETER_PROBLEM,
					   "`%s' unsupported protocol",
					   optarg);
			break;
		case 'b':
			socketbuffersize = atol(optarg);
			options |= CT_OPT_BUFFERSIZE;
			break;
		case ':':
			exit_error(PARAMETER_PROBLEM,
				   "option `%s' requires an "
				   "argument", argv[optind-1]);
			break;
		case 'v': //Andrew
			options |= CT_OPT_LOG;
			log_fp = create_log_file(optarg);
			if (log_fp == NULL)
				exit_error(OTHER_PROBLEM,
				   "Can't open file `%s' to log", optarg);
			strcpy(log_file, optarg);
			break;
		case '?':
			exit_error(PARAMETER_PROBLEM,
				   "unknown option `%s'", argv[optind-1]);
			break;
		default:
			if (h && h->parse_opts 
			    &&!h->parse_opts(c - h->option_offset, tmpl.ct,
			    		     tmpl.exptuple, tmpl.mask,
					     &l4flags))
				exit_error(PARAMETER_PROBLEM, "parse error");
			break;
		}
	}

	/* default family */
	if (family == AF_UNSPEC)
		family = AF_INET;

	/* we cannot check this combination with generic_opt_check. */
	if (options & CT_OPT_ANY_NAT &&
	   ((options & CT_OPT_SRC_NAT) || (options & CT_OPT_DST_NAT))) {
		exit_error(PARAMETER_PROBLEM, "cannot specify `--src-nat' or "
					      "`--dst-nat' with `--any-nat'");
	}
	cmd = bit2cmd(command);
	res = generic_opt_check(options, NUMBER_OF_OPT,
				commands_v_options[cmd], optflags,
				addr_valid_flags, ADDR_VALID_FLAGS_MAX,
				&partial);
	if (!res) {
		switch(partial) {
		case -1:
		case 0:
			exit_error(PARAMETER_PROBLEM, "you have to specify "
						      "`--src' and `--dst'");
			break;
		case 1:
			exit_error(PARAMETER_PROBLEM, "you have to specify "
						      "`--reply-src' and "
						      "`--reply-dst'");
			break;
		}
	}
	if (!(command & CT_HELP) && h && h->final_check)
		h->final_check(l4flags, cmd, tmpl.ct);

	switch(command) {
	struct nfct_filter_dump *filter_dump;

	case CT_LIST:
		if (type == CT_TABLE_DYING) {
			if (nfct_mnl_socket_open() < 0)
				exit_error(OTHER_PROBLEM, "Can't open handler");

			res = nfct_mnl_dump(NFNL_SUBSYS_CTNETLINK,
					    IPCTNL_MSG_CT_GET_DYING,
					    mnl_nfct_dump_cb, family);

			nfct_mnl_socket_close();
			break;
		} else if (type == CT_TABLE_UNCONFIRMED) {
			if (nfct_mnl_socket_open() < 0)
				exit_error(OTHER_PROBLEM, "Can't open handler");

			res = nfct_mnl_dump(NFNL_SUBSYS_CTNETLINK,
					    IPCTNL_MSG_CT_GET_UNCONFIRMED,
					    mnl_nfct_dump_cb, family);

			nfct_mnl_socket_close();
			break;
		}

		cth = nfct_open(CONNTRACK, 0);
		if (!cth)
			exit_error(OTHER_PROBLEM, "Can't open handler");

		if (options & CT_COMPARISON && 
		    options & CT_OPT_ZERO)
			exit_error(PARAMETER_PROBLEM, "Can't use -z with "
						      "filtering parameters");

		nfct_filter_init(family);

		nfct_callback_register(cth, NFCT_T_ALL, dump_cb, tmpl.ct);

		filter_dump = nfct_filter_dump_create();
		if (filter_dump == NULL)
			exit_error(OTHER_PROBLEM, "OOM");

		if (tmpl.filter_mark_kernel_set) {
			nfct_filter_dump_set_attr(filter_dump,
						  NFCT_FILTER_DUMP_MARK,
						  &tmpl.filter_mark_kernel);
			nfct_filter_dump_set_attr_u8(filter_dump,
						     NFCT_FILTER_DUMP_L3NUM,
						     family);
		}

		if (options & CT_OPT_ZERO)
			res = nfct_query(cth, NFCT_Q_DUMP_FILTER_RESET,
					filter_dump);
		else
			res = nfct_query(cth, NFCT_Q_DUMP_FILTER, filter_dump);

		nfct_filter_dump_destroy(filter_dump);

		if (dump_xml_header_done == 0) {
			printf("</conntrack>\n");
			fflush(stdout);
		}

		nfct_close(cth);
		break;

	case EXP_LIST:
		cth = nfct_open(EXPECT, 0);
		if (!cth)
			exit_error(OTHER_PROBLEM, "Can't open handler");

		nfexp_callback_register(cth, NFCT_T_ALL, dump_exp_cb, NULL);
		res = nfexp_query(cth, NFCT_Q_DUMP, &family);
		nfct_close(cth);

		if (dump_xml_header_done == 0) {
			printf("</expect>\n");
			fflush(stdout);
		}
		break;

	case CT_CREATE:
		if ((options & CT_OPT_ORIG) && !(options & CT_OPT_REPL))
		    	nfct_setobjopt(tmpl.ct, NFCT_SOPT_SETUP_REPLY);
		else if (!(options & CT_OPT_ORIG) && (options & CT_OPT_REPL))
			nfct_setobjopt(tmpl.ct, NFCT_SOPT_SETUP_ORIGINAL);

		if (options & CT_OPT_MARK)
			nfct_set_attr_u32(tmpl.ct, ATTR_MARK, tmpl.mark.value);

		if (options & CT_OPT_ADD_LABEL)
			nfct_set_attr(tmpl.ct, ATTR_CONNLABELS,
					xnfct_bitmask_clone(tmpl.label_modify));

		cth = nfct_open(CONNTRACK, 0);
		if (!cth)
			exit_error(OTHER_PROBLEM, "Can't open handler");

		res = nfct_query(cth, NFCT_Q_CREATE, tmpl.ct);
		if (res != -1)
			counter++;
		nfct_close(cth);
		break;

	case EXP_CREATE:
		nfexp_set_attr(tmpl.exp, ATTR_EXP_MASTER, tmpl.ct);
		nfexp_set_attr(tmpl.exp, ATTR_EXP_EXPECTED, tmpl.exptuple);
		nfexp_set_attr(tmpl.exp, ATTR_EXP_MASK, tmpl.mask);

		cth = nfct_open(EXPECT, 0);
		if (!cth)
			exit_error(OTHER_PROBLEM, "Can't open handler");

		res = nfexp_query(cth, NFCT_Q_CREATE, tmpl.exp);
		nfct_close(cth);
		break;

	case CT_UPDATE:
		cth = nfct_open(CONNTRACK, 0);
		/* internal handler for delete_cb, otherwise we hit EILSEQ */
		ith = nfct_open(CONNTRACK, 0);
		if (!cth || !ith)
			exit_error(OTHER_PROBLEM, "Can't open handler");

		nfct_filter_init(family);

		nfct_callback_register(cth, NFCT_T_ALL, update_cb, tmpl.ct);

		res = nfct_query(cth, NFCT_Q_DUMP, &family);
		nfct_close(ith);
		nfct_close(cth);
		break;
		
	case CT_DELETE:
		cth = nfct_open(CONNTRACK, 0);
		ith = nfct_open(CONNTRACK, 0);
		if (!cth || !ith)
			exit_error(OTHER_PROBLEM, "Can't open handler");

		nfct_filter_init(family);

		nfct_callback_register(cth, NFCT_T_ALL, delete_cb, tmpl.ct);

		filter_dump = nfct_filter_dump_create();
		if (filter_dump == NULL)
			exit_error(OTHER_PROBLEM, "OOM");

		if (tmpl.filter_mark_kernel_set) {
			nfct_filter_dump_set_attr(filter_dump,
						  NFCT_FILTER_DUMP_MARK,
						  &tmpl.filter_mark_kernel);
			nfct_filter_dump_set_attr_u8(filter_dump,
						     NFCT_FILTER_DUMP_L3NUM,
						     family);
		}

		res = nfct_query(cth, NFCT_Q_DUMP_FILTER, filter_dump);

		nfct_filter_dump_destroy(filter_dump);

		nfct_close(ith);
		nfct_close(cth);
		break;

	case EXP_DELETE:
		nfexp_set_attr(tmpl.exp, ATTR_EXP_EXPECTED, tmpl.ct);

		cth = nfct_open(EXPECT, 0);
		if (!cth)
			exit_error(OTHER_PROBLEM, "Can't open handler");

		res = nfexp_query(cth, NFCT_Q_DESTROY, tmpl.exp);
		nfct_close(cth);
		break;

	case CT_GET:
		cth = nfct_open(CONNTRACK, 0);
		if (!cth)
			exit_error(OTHER_PROBLEM, "Can't open handler");

		nfct_callback_register(cth, NFCT_T_ALL, dump_cb, tmpl.ct);
		res = nfct_query(cth, NFCT_Q_GET, tmpl.ct);
		nfct_close(cth);
		break;

	case EXP_GET:
		nfexp_set_attr(tmpl.exp, ATTR_EXP_MASTER, tmpl.ct);

		cth = nfct_open(EXPECT, 0);
		if (!cth)
			exit_error(OTHER_PROBLEM, "Can't open handler");

		nfexp_callback_register(cth, NFCT_T_ALL, dump_exp_cb, NULL);
		res = nfexp_query(cth, NFCT_Q_GET, tmpl.exp);
		nfct_close(cth);
		break;

	case CT_FLUSH:
		cth = nfct_open(CONNTRACK, 0);
		if (!cth)
			exit_error(OTHER_PROBLEM, "Can't open handler");
		res = nfct_query(cth, NFCT_Q_FLUSH, &family);
		nfct_close(cth);
		fprintf(stderr, "%s v%s (conntrack-tools): ",PROGNAME,VERSION);
		fprintf(stderr,"connection tracking table has been emptied.\n");
		break;

	case EXP_FLUSH:
		cth = nfct_open(EXPECT, 0);
		if (!cth)
			exit_error(OTHER_PROBLEM, "Can't open handler");
		res = nfexp_query(cth, NFCT_Q_FLUSH, &family);
		nfct_close(cth);
		fprintf(stderr, "%s v%s (conntrack-tools): ",PROGNAME,VERSION);
		fprintf(stderr,"expectation table has been emptied.\n");
		break;
		
	case CT_EVENT:
		if (options & CT_OPT_EVENT_MASK) {
			unsigned int nl_events = 0;

			if (event_mask & CT_EVENT_F_NEW)
				nl_events |= NF_NETLINK_CONNTRACK_NEW;
			if (event_mask & CT_EVENT_F_UPD)
				nl_events |= NF_NETLINK_CONNTRACK_UPDATE;
			if (event_mask & CT_EVENT_F_DEL)
				nl_events |= NF_NETLINK_CONNTRACK_DESTROY;

			cth = nfct_open(CONNTRACK, nl_events);
		} else {
			cth = nfct_open(CONNTRACK,
					NF_NETLINK_CONNTRACK_NEW |
					NF_NETLINK_CONNTRACK_UPDATE |
					NF_NETLINK_CONNTRACK_DESTROY);
		}

		if (!cth)
			exit_error(OTHER_PROBLEM, "Can't open handler");

		if (options & CT_OPT_BUFFERSIZE) {
			size_t ret;
			ret = nfnl_rcvbufsiz(nfct_nfnlh(cth), socketbuffersize);
			fprintf(stderr, "NOTICE: Netlink socket buffer size "
					"has been set to %zu bytes.\n", ret);
		}

		nfct_filter_init(family);

		signal(SIGINT, event_sighandler);
		signal(SIGTERM, event_sighandler);
		signal(SIGUSR1, event_sighandler);
		nfct_callback_register(cth, NFCT_T_ALL, event_cb, tmpl.ct);
		res = nfct_catch(cth);
		if (res == -1) {
			if (errno == ENOBUFS) {
				fprintf(stderr, 
					"WARNING: We have hit ENOBUFS! We "
					"are losing events.\nThis message "
					"means that the current netlink "
					"socket buffer size is too small.\n"
					"Please, check --buffer-size in "
					"conntrack(8) manpage.\n");
			}
		}
		nfct_close(cth);
		break;

	case EXP_EVENT:
		if (options & CT_OPT_EVENT_MASK) {
			unsigned int nl_events = 0;

			if (event_mask & CT_EVENT_F_NEW)
				nl_events |= NF_NETLINK_CONNTRACK_EXP_NEW;
			if (event_mask & CT_EVENT_F_UPD)
				nl_events |= NF_NETLINK_CONNTRACK_EXP_UPDATE;
			if (event_mask & CT_EVENT_F_DEL)
				nl_events |= NF_NETLINK_CONNTRACK_EXP_DESTROY;

			cth = nfct_open(CONNTRACK, nl_events);
		} else {
			cth = nfct_open(EXPECT,
					NF_NETLINK_CONNTRACK_EXP_NEW |
					NF_NETLINK_CONNTRACK_EXP_UPDATE |
					NF_NETLINK_CONNTRACK_EXP_DESTROY);
		}

		if (!cth)
			exit_error(OTHER_PROBLEM, "Can't open handler");
		signal(SIGINT, exp_event_sighandler);
		signal(SIGTERM, exp_event_sighandler);
		nfexp_callback_register(cth, NFCT_T_ALL, event_exp_cb, NULL);
		res = nfexp_catch(cth);
		nfct_close(cth);
		break;
	case CT_COUNT:
		/* If we fail with netlink, fall back to /proc to ensure
		 * backward compatibility.
		 */
		if (nfct_mnl_socket_open() < 0)
			goto try_proc_count;

		res = nfct_mnl_get(NFNL_SUBSYS_CTNETLINK,
				   IPCTNL_MSG_CT_GET_STATS,
				   nfct_global_stats_cb, AF_UNSPEC);

		nfct_mnl_socket_close();

		/* don't look at /proc, we got the information via ctnetlink */
		if (res >= 0)
			break;

try_proc_count:
		{
#define NF_CONNTRACK_COUNT_PROC "/proc/sys/net/netfilter/nf_conntrack_count"
		FILE *fd;
		int count;
		fd = fopen(NF_CONNTRACK_COUNT_PROC, "r");
		if (fd == NULL) {
			exit_error(OTHER_PROBLEM, "Can't open %s",
				   NF_CONNTRACK_COUNT_PROC);
		}
		if (fscanf(fd, "%d", &count) != 1) {
			exit_error(OTHER_PROBLEM, "Can't read %s",
				   NF_CONNTRACK_COUNT_PROC);
		}
		fclose(fd);
		printf("%d\n", count);
		break;
	}
	case EXP_COUNT:
		cth = nfct_open(EXPECT, 0);
		if (!cth)
			exit_error(OTHER_PROBLEM, "Can't open handler");

		nfexp_callback_register(cth, NFCT_T_ALL, count_exp_cb, NULL);
		res = nfexp_query(cth, NFCT_Q_DUMP, &family);
		nfct_close(cth);
		printf("%d\n", counter);
		break;
	case CT_STATS:
		/* If we fail with netlink, fall back to /proc to ensure
		 * backward compatibility.
		 */
		if (nfct_mnl_socket_open() < 0)
			goto try_proc;

		res = nfct_mnl_dump(NFNL_SUBSYS_CTNETLINK,
				    IPCTNL_MSG_CT_GET_STATS_CPU,
				    nfct_stats_cb, AF_UNSPEC);

		nfct_mnl_socket_close();

		/* don't look at /proc, we got the information via ctnetlink */
		if (res >= 0)
			break;

		goto try_proc;

	case EXP_STATS:
		/* If we fail with netlink, fall back to /proc to ensure
		 * backward compatibility.
		 */
		if (nfct_mnl_socket_open() < 0)
			goto try_proc;

		res = nfct_mnl_dump(NFNL_SUBSYS_CTNETLINK_EXP,
				    IPCTNL_MSG_EXP_GET_STATS_CPU,
				    nfexp_stats_cb, AF_UNSPEC);

		nfct_mnl_socket_close();

		/* don't look at /proc, we got the information via ctnetlink */
		if (res >= 0)
			break;
try_proc:
		if (display_proc_conntrack_stats() < 0)
			exit_error(OTHER_PROBLEM, "Can't open /proc interface");
		break;
	case CT_VERSION:
		printf("%s v%s (conntrack-tools)\n", PROGNAME, VERSION);
		break;
	case CT_HELP:
		usage(argv[0]);
		if (options & CT_OPT_PROTO)
			extension_help(h, protonum);
		break;
	default:
		usage(argv[0]);
		break;
	}

	if (res < 0)
		exit_error(OTHER_PROBLEM, "Operation failed: %s",
			   err2str(errno, command));

	free_tmpl_objects();
	free_options();
	if (labelmap)
		nfct_labelmap_destroy(labelmap);

	if (command && exit_msg[cmd][0]) {
		fprintf(stderr, "%s v%s (conntrack-tools): ",PROGNAME,VERSION);
		fprintf(stderr, exit_msg[cmd], counter);
		if (counter == 0 && !(command & (CT_LIST | EXP_LIST)))
			return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
