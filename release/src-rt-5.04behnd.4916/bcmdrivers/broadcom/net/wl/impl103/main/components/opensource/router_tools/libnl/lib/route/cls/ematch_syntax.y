/*
 * lib/route/cls/ematch_syntax.y	ematch expression syntax
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2010-2013 Thomas Graf <tgraf@suug.ch>
 */

%{
#include <netlink-private/netlink.h>
#include <netlink-private/tc.h>
#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <netlink/route/pktloc.h>
#include <netlink/route/cls/ematch.h>
#include <netlink/route/cls/ematch/cmp.h>
#include <netlink/route/cls/ematch/nbyte.h>
#include <netlink/route/cls/ematch/text.h>
#include <netlink/route/cls/ematch/meta.h>

#define META_ALLOC rtnl_meta_value_alloc_id
#define META_ID(name) TCF_META_ID_##name
#define META_INT TCF_META_TYPE_INT
#define META_VAR TCF_META_TYPE_VAR
%}

%error-verbose
%define api.pure
%name-prefix "ematch_"

%parse-param {void *scanner}
%parse-param {char **errp}
%parse-param {struct nl_list_head *root}
%lex-param {void *scanner}

%union {
	struct tcf_em_cmp	cmp;
	struct ematch_quoted	q;
	struct rtnl_ematch *	e;
	struct rtnl_pktloc *	loc;
	struct rtnl_meta_value *mv;
	uint32_t		i;
	uint64_t		i64;
	char *			s;
}

%{
extern int ematch_lex(YYSTYPE *, void *);

static void yyerror(void *scanner, char **errp, struct nl_list_head *root, const char *msg)
{
	if (msg)
            *errp = strdup(msg);
        else
	    *errp = NULL;
}
%}

%token <i> ERROR LOGIC NOT OPERAND NUMBER ALIGN LAYER
%token <i> KW_OPEN "("
%token <i> KW_CLOSE ")"
%token <i> KW_PLUS "+"
%token <i> KW_MASK "mask"
%token <i> KW_SHIFT ">>"
%token <i> KW_AT "at"
%token <i> EMATCH_CMP "cmp"
%token <i> EMATCH_NBYTE "pattern"
%token <i> EMATCH_TEXT "text"
%token <i> EMATCH_META "meta"
%token <i> KW_EQ "="
%token <i> KW_GT ">"
%token <i> KW_LT "<"
%token <i> KW_FROM "from"
%token <i> KW_TO "to"

%token <i> META_RANDOM "random"
%token <i> META_LOADAVG_0 "loadavg_0"
%token <i> META_LOADAVG_1 "loadavg_1"
%token <i> META_LOADAVG_2 "loadavg_2"
%token <i> META_DEV "dev"
%token <i> META_PRIO "prio"
%token <i> META_PROTO "proto"
%token <i> META_PKTTYPE "pkttype"
%token <i> META_PKTLEN "pktlen"
%token <i> META_DATALEN "datalen"
%token <i> META_MACLEN "maclen"
%token <i> META_MARK "mark"
%token <i> META_TCINDEX "tcindex"
%token <i> META_RTCLASSID "rtclassid"
%token <i> META_RTIIF "rtiif"
%token <i> META_SK_FAMILY "sk_family"
%token <i> META_SK_STATE "sk_state"
%token <i> META_SK_REUSE "sk_reuse"
%token <i> META_SK_REFCNT "sk_refcnt"
%token <i> META_SK_RCVBUF "sk_rcvbuf"
%token <i> META_SK_SNDBUF "sk_sndbuf"
%token <i> META_SK_SHUTDOWN "sk_shutdown"
%token <i> META_SK_PROTO "sk_proto"
%token <i> META_SK_TYPE "sk_type"
%token <i> META_SK_RMEM_ALLOC "sk_rmem_alloc"
%token <i> META_SK_WMEM_ALLOC "sk_wmem_alloc"
%token <i> META_SK_WMEM_QUEUED "sk_wmem_queued"
%token <i> META_SK_RCV_QLEN "sk_rcv_qlen"
%token <i> META_SK_SND_QLEN "sk_snd_qlen"
%token <i> META_SK_ERR_QLEN "sk_err_qlen"
%token <i> META_SK_FORWARD_ALLOCS "sk_forward_allocs"
%token <i> META_SK_ALLOCS "sk_allocs"
%token <i> META_SK_ROUTE_CAPS "sk_route_caps"
%token <i> META_SK_HASH "sk_hash"
%token <i> META_SK_LINGERTIME "sk_lingertime"
%token <i> META_SK_ACK_BACKLOG "sk_ack_backlog"
%token <i> META_SK_MAX_ACK_BACKLOG "sk_max_ack_backlog"
%token <i> META_SK_PRIO "sk_prio"
%token <i> META_SK_RCVLOWAT "sk_rcvlowat"
%token <i> META_SK_RCVTIMEO "sk_rcvtimeo"
%token <i> META_SK_SNDTIMEO "sk_sndtimeo"
%token <i> META_SK_SENDMSG_OFF "sk_sendmsg_off"
%token <i> META_SK_WRITE_PENDING "sk_write_pending"
%token <i> META_VLAN "vlan"
%token <i> META_RXHASH "rxhash"
%token <i> META_DEVNAME "devname"
%token <i> META_SK_BOUND_IF "sk_bound_if"

%token <s> STR

%token <q> QUOTED

%type <i> align operand shift meta_int_id meta_var_id
%type <i64> mask
%type <e> expr match ematch
%type <cmp> cmp_expr cmp_match
%type <loc> pktloc text_from text_to
%type <q> pattern
%type <mv> meta_value

%destructor { free($$); NL_DBG(2, "string destructor\n"); } <s>
%destructor { rtnl_pktloc_put($$); NL_DBG(2, "pktloc destructor\n"); } <loc>
%destructor { free($$.data); NL_DBG(2, "quoted destructor\n"); } <q>
%destructor { rtnl_meta_value_put($$); NL_DBG(2, "meta value destructor\n"); } <mv>

%start input

%%

input:
	/* empty */
	| expr
		{
			nl_list_add_tail(root, &$1->e_list);
		}
	;

expr:
	match
		{
			$$ = $1;
		}
	| match LOGIC expr
		{
			rtnl_ematch_set_flags($1, $2);

			/* make ematch new head */
			nl_list_add_tail(&$1->e_list, &$3->e_list);

			$$ = $1;
		}
	;

match:
	NOT ematch
		{
			rtnl_ematch_set_flags($2, TCF_EM_INVERT);
			$$ = $2;
		}
	| ematch
		{
			$$ = $1;
		}
	;

ematch:
	/* CMP */
	cmp_match
		{
			struct rtnl_ematch *e;

			if (!(e = rtnl_ematch_alloc())) {
				*errp = strdup("Unable to allocate ematch object");
				YYABORT;
			}

			if (rtnl_ematch_set_kind(e, TCF_EM_CMP) < 0)
				BUG();

			rtnl_ematch_cmp_set(e, &$1);
			$$ = e;
		}
	| EMATCH_NBYTE "(" pktloc KW_EQ pattern ")"
		{
			struct rtnl_ematch *e;

			if (!(e = rtnl_ematch_alloc())) {
				*errp = strdup("Unable to allocate ematch object");
				YYABORT;
			}

			if (rtnl_ematch_set_kind(e, TCF_EM_NBYTE) < 0)
				BUG();

			rtnl_ematch_nbyte_set_offset(e, $3->layer, $3->offset);
			rtnl_pktloc_put($3);
			rtnl_ematch_nbyte_set_pattern(e, (uint8_t *) $5.data, $5.index);

			$$ = e;
		}
	| EMATCH_TEXT "(" STR QUOTED text_from text_to ")"
		{
			struct rtnl_ematch *e;

			if (!(e = rtnl_ematch_alloc())) {
				*errp = strdup("Unable to allocate ematch object");
				YYABORT;
			}

			if (rtnl_ematch_set_kind(e, TCF_EM_TEXT) < 0)
				BUG();

			rtnl_ematch_text_set_algo(e, $3);
			rtnl_ematch_text_set_pattern(e, $4.data, $4.index);

			if ($5) {
				rtnl_ematch_text_set_from(e, $5->layer, $5->offset);
				rtnl_pktloc_put($5);
			}

			if ($6) {
				rtnl_ematch_text_set_to(e, $6->layer, $6->offset);
				rtnl_pktloc_put($6);
			}

			$$ = e;
		}
	| EMATCH_META "(" meta_value operand meta_value ")"
		{
			struct rtnl_ematch *e;

			if (!(e = rtnl_ematch_alloc())) {
				*errp = strdup("Unable to allocate ematch object");
				YYABORT;
			}

			if (rtnl_ematch_set_kind(e, TCF_EM_META) < 0)
				BUG();

			rtnl_ematch_meta_set_lvalue(e, $3);
			rtnl_ematch_meta_set_rvalue(e, $5);
			rtnl_ematch_meta_set_operand(e, $4);

			$$ = e;
		}
	/* CONTAINER */
	| "(" expr ")"
		{
			struct rtnl_ematch *e;

			if (!(e = rtnl_ematch_alloc())) {
				*errp = strdup("Unable to allocate ematch object");
				YYABORT;
			}

			if (rtnl_ematch_set_kind(e, TCF_EM_CONTAINER) < 0)
				BUG();

			/* Make e->childs the list head of a the ematch sequence */
			nl_list_add_tail(&e->e_childs, &$2->e_list);

			$$ = e;
		}
	;

/*
 * CMP match
 *
 * match  := cmp(expr) | expr
 * expr   := pktloc (=|>|<) NUMBER
 * pktloc := alias | definition
 *
 */
cmp_match:
	EMATCH_CMP "(" cmp_expr ")"
		{ $$ = $3; }
	| cmp_expr
		{ $$ = $1; }
	;

cmp_expr:
	pktloc operand NUMBER
		{
			if ($1->align == TCF_EM_ALIGN_U16 ||
			    $1->align == TCF_EM_ALIGN_U32)
				$$.flags = TCF_EM_CMP_TRANS;

			memset(&$$, 0, sizeof($$));

			$$.mask = $1->mask;
			$$.off = $1->offset;
			$$.align = $1->align;
			$$.layer = $1->layer;
			$$.opnd = $2;
			$$.val = $3;

			rtnl_pktloc_put($1);
		}
	;

text_from:
	/* empty */
		{ $$ = NULL; }
	| "from" pktloc
		{ $$ = $2; }
	;

text_to:
	/* empty */
		{ $$ = NULL; }
	| "to" pktloc
		{ $$ = $2; }
	;

meta_value:
	QUOTED
		{ $$ = rtnl_meta_value_alloc_var($1.data, $1.len); }
	| NUMBER
		{ $$ = rtnl_meta_value_alloc_int($1); }
	| meta_int_id shift mask
		{ $$ = META_ALLOC(META_INT, $1, $2, $3); }
	| meta_var_id shift
		{ $$ = META_ALLOC(META_VAR, $1, $2, 0); }
	;

meta_int_id:
	META_RANDOM			{ $$ = META_ID(RANDOM); }
	|META_LOADAVG_0			{ $$ = META_ID(LOADAVG_0); }
	|META_LOADAVG_1			{ $$ = META_ID(LOADAVG_1); }
	|META_LOADAVG_2			{ $$ = META_ID(LOADAVG_2); }
	| META_DEV			{ $$ = META_ID(DEV); }
	| META_PRIO			{ $$ = META_ID(PRIORITY); }
	| META_PROTO			{ $$ = META_ID(PROTOCOL); }
	| META_PKTTYPE			{ $$ = META_ID(PKTTYPE); }
	| META_PKTLEN			{ $$ = META_ID(PKTLEN); }
	| META_DATALEN			{ $$ = META_ID(DATALEN); }
	| META_MACLEN			{ $$ = META_ID(MACLEN); }
	| META_MARK			{ $$ = META_ID(NFMARK); }
	| META_TCINDEX			{ $$ = META_ID(TCINDEX); }
	| META_RTCLASSID		{ $$ = META_ID(RTCLASSID); }
	| META_RTIIF			{ $$ = META_ID(RTIIF); }
	| META_SK_FAMILY		{ $$ = META_ID(SK_FAMILY); }
	| META_SK_STATE			{ $$ = META_ID(SK_STATE); }
	| META_SK_REUSE			{ $$ = META_ID(SK_REUSE); }
	| META_SK_REFCNT		{ $$ = META_ID(SK_REFCNT); }
	| META_SK_RCVBUF		{ $$ = META_ID(SK_RCVBUF); }
	| META_SK_SNDBUF		{ $$ = META_ID(SK_SNDBUF); }
	| META_SK_SHUTDOWN		{ $$ = META_ID(SK_SHUTDOWN); }
	| META_SK_PROTO			{ $$ = META_ID(SK_PROTO); }
	| META_SK_TYPE			{ $$ = META_ID(SK_TYPE); }
	| META_SK_RMEM_ALLOC		{ $$ = META_ID(SK_RMEM_ALLOC); }
	| META_SK_WMEM_ALLOC		{ $$ = META_ID(SK_WMEM_ALLOC); }
	| META_SK_WMEM_QUEUED		{ $$ = META_ID(SK_WMEM_QUEUED); }
	| META_SK_RCV_QLEN		{ $$ = META_ID(SK_RCV_QLEN); }
	| META_SK_SND_QLEN		{ $$ = META_ID(SK_SND_QLEN); }
	| META_SK_ERR_QLEN		{ $$ = META_ID(SK_ERR_QLEN); }
	| META_SK_FORWARD_ALLOCS	{ $$ = META_ID(SK_FORWARD_ALLOCS); }
	| META_SK_ALLOCS		{ $$ = META_ID(SK_ALLOCS); }
	| META_SK_ROUTE_CAPS		{ $$ = META_ID(SK_ROUTE_CAPS); }
	| META_SK_HASH			{ $$ = META_ID(SK_HASH); }
	| META_SK_LINGERTIME		{ $$ = META_ID(SK_LINGERTIME); }
	| META_SK_ACK_BACKLOG		{ $$ = META_ID(SK_ACK_BACKLOG); }
	| META_SK_MAX_ACK_BACKLOG	{ $$ = META_ID(SK_MAX_ACK_BACKLOG); }
	| META_SK_PRIO			{ $$ = META_ID(SK_PRIO); }
	| META_SK_RCVLOWAT		{ $$ = META_ID(SK_RCVLOWAT); }
	| META_SK_RCVTIMEO		{ $$ = META_ID(SK_RCVTIMEO); }
	| META_SK_SNDTIMEO		{ $$ = META_ID(SK_SNDTIMEO); }
	| META_SK_SENDMSG_OFF		{ $$ = META_ID(SK_SENDMSG_OFF); }
	| META_SK_WRITE_PENDING		{ $$ = META_ID(SK_WRITE_PENDING); }
	| META_VLAN			{ $$ = META_ID(VLAN_TAG); }
	| META_RXHASH			{ $$ = META_ID(RXHASH); }
	;

meta_var_id:
	META_DEVNAME		{ $$ = META_ID(DEV); }
	| META_SK_BOUND_IF	{ $$ = META_ID(SK_BOUND_IF); }
	;

/*
 * pattern
 */
pattern:
	QUOTED
		{
			$$ = $1;
		}
	| STR
		{
			struct nl_addr *addr;

			if (nl_addr_parse($1, AF_UNSPEC, &addr) == 0) {
				$$.len = nl_addr_get_len(addr);

				$$.index = min_t(int, $$.len, nl_addr_get_prefixlen(addr)/8);

				if (!($$.data = calloc(1, $$.len))) {
					nl_addr_put(addr);
					YYABORT;
				}

				memcpy($$.data, nl_addr_get_binary_addr(addr), $$.len);
				nl_addr_put(addr);
			} else {
				if (asprintf(errp, "invalid pattern \"%s\"", $1) == -1)
					*errp = NULL;
				YYABORT;
			}
		}
	;

/*
 * packet location
 */

pktloc:
	STR
		{
			struct rtnl_pktloc *loc;

			if (rtnl_pktloc_lookup($1, &loc) < 0) {
				if (asprintf(errp, "Packet location \"%s\" not found", $1) == -1)
					*errp = NULL;
				YYABORT;
			}

			$$ = loc;
		}
	/* [u8|u16|u32|NUM at] LAYER + OFFSET [mask MASK] */
	| align LAYER "+" NUMBER mask
		{
			struct rtnl_pktloc *loc;

			if ($5 && (!$1 || $1 > TCF_EM_ALIGN_U32)) {
				*errp = strdup("mask only allowed for alignments u8|u16|u32");
				YYABORT;
			}

			if (!(loc = rtnl_pktloc_alloc())) {
				*errp = strdup("Unable to allocate packet location object");
				YYABORT;
			}

			loc->name = strdup("<USER-DEFINED>");
			loc->align = $1;
			loc->layer = $2;
			loc->offset = $4;
			loc->mask = $5;

			$$ = loc;
		}
	;

align:
	/* empty */
		{ $$ = 0; }
	| ALIGN "at"
		{ $$ = $1; }
	| NUMBER "at"
		{ $$ = $1; }
	;

mask:
	/* empty */
		{ $$ = 0; }
	| KW_MASK NUMBER
		{ $$ = $2; }
	;

shift:
	/* empty */
		{ $$ = 0; }
	| KW_SHIFT NUMBER
		{ $$ = $2; }
	;

operand:
	KW_EQ
		{ $$ = TCF_EM_OPND_EQ; }
	| KW_GT
		{ $$ = TCF_EM_OPND_GT; }
	| KW_LT
		{ $$ = TCF_EM_OPND_LT; }
	;
