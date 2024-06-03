/*
 * lib/route/cls/ematch_grammar.l	ematch expression grammar
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
 #include <netlink/route/cls/ematch.h>
 #include <netlink/route/cls/ematch/cmp.h>
 #include "ematch_syntax.h"
%}

%option 8bit
%option reentrant
%option warn
%option noyywrap
%option noinput
%option nounput
%option bison-bridge
%option prefix="ematch_"

%x QUOTE

%%

[ \t\r\n]+

\"			{
				NL_DBG(4, "Beginning of quote\n");
				yylval->q.len = 32;
				if (!(yylval->q.data = calloc(1, yylval->q.len)))
					return ERROR;

				yylval->q.index = 0;
				BEGIN(QUOTE);
			}

<QUOTE>[^\\\n\"]+	{
				memcpy(yylval->q.data + yylval->q.index, yytext,
				       strlen(yytext));
				yylval->q.index += strlen(yytext);
			}

<QUOTE>\"		{
				BEGIN(0);
				return QUOTED;
			}


[[:digit:]]+		|
0[xX][[:xdigit:]]+	{
				yylval->i = strtoul(yytext, NULL, 0);
				return NUMBER;
			}

eq			|
"="			return KW_EQ;
gt			|
">"			return KW_GT;
lt			|
"<"			return KW_LT;

[aA][nN][dD]		|
"&&"			{ yylval->i = TCF_EM_REL_AND; return LOGIC; }
[oO][rR]		|
"||"			{ yylval->i = TCF_EM_REL_OR; return LOGIC; }
[nN][oO][tT]		|
"!"			return NOT;

[cC][mM][pP]		{ yylval->i = TCF_EM_CMP; return EMATCH_CMP; }
[pP][aA][tT][tT][eE][rR][nN] { yylval->i = TCF_EM_NBYTE; return EMATCH_NBYTE; }
[tT][eE][xX][tT]	{ yylval->i = TCF_EM_TEXT; return EMATCH_TEXT; }
[mM][eE][tT][aA]	{ yylval->i = TCF_EM_META; return EMATCH_META; }

"("			return KW_OPEN;
")"			return KW_CLOSE;
[mM][aA][sS][kK]	|
"&"			return KW_MASK;
[sS][hH][iI][fF][tT]	|
">>"			return KW_SHIFT;
[aA][tT]		return KW_AT;
"+"			return KW_PLUS;
[fF][rR][oO][mM]	return KW_FROM;
[tT][oO]		return KW_TO;

[uU]8			{ yylval->i = TCF_EM_ALIGN_U8; return ALIGN; }
[uU]16			{ yylval->i = TCF_EM_ALIGN_U16; return ALIGN; }
[uU]32			{ yylval->i = TCF_EM_ALIGN_U32; return ALIGN; }

[lL][iI][nN][kK]	|
[eE][tT][hH]		{ yylval->i = TCF_LAYER_LINK; return LAYER; }
[nN][eE][tT]		|
[iI][pP]6		|
[iI][pP]		{ yylval->i = TCF_LAYER_NETWORK; return LAYER; }
[tT][rR][aA][nN][sS][pP][oO][rR][tT] |
[tT][cC][pP]		{ yylval->i = TCF_LAYER_TRANSPORT; return LAYER; }

random			return META_RANDOM;
loadavg_0		return META_LOADAVG_0;
loadavg_1		return META_LOADAVG_1;
loadavg_2		return META_LOADAVG_2;
dev			return META_DEV;
prio			return META_PRIO;
proto			return META_PROTO;
pkttype			return META_PKTTYPE;
pktlen			return META_PKTLEN;
datalen			return META_DATALEN;
maclen			return META_MACLEN;
mark			return META_MARK;
tcindex			return META_TCINDEX;
rtclassid		return META_RTCLASSID;
rtiif			return META_RTIIF;
sk_family		return META_SK_FAMILY;
sk_state		return META_SK_STATE;
sk_reuse		return META_SK_REUSE;
sk_refcnt		return META_SK_REFCNT;
sk_rcvbuf		return META_SK_RCVBUF;
sk_sndbuf		return META_SK_SNDBUF;
sk_shutdown		return META_SK_SHUTDOWN;
sk_proto		return META_SK_PROTO;
sk_type			return META_SK_TYPE;
sk_rmem_alloc		return META_SK_RMEM_ALLOC;
sk_wmem_alloc		return META_SK_WMEM_ALLOC;
sk_wmem_queued		return META_SK_WMEM_QUEUED;
sk_rcv_qlen		return META_SK_RCV_QLEN;
sk_snd_qlen		return META_SK_SND_QLEN;
sk_err_qlen		return META_SK_ERR_QLEN;
sk_forward_allocs	return META_SK_FORWARD_ALLOCS;
sk_allocs		return META_SK_ALLOCS;
sk_route_caps		return META_SK_ROUTE_CAPS;
sk_hash			return META_SK_HASH;
sk_lingertime		return META_SK_LINGERTIME;
sk_ack_backlog		return META_SK_ACK_BACKLOG;
sk_max_ack_backlog	return META_SK_MAX_ACK_BACKLOG;
sk_prio			return META_SK_PRIO;
sk_rcvlowat		return META_SK_RCVLOWAT;
sk_rcvtimeo		return META_SK_RCVTIMEO;
sk_sndtimeo		return META_SK_SNDTIMEO;
sk_sendmsg_off		return META_SK_SENDMSG_OFF;
sk_write_pending	return META_SK_WRITE_PENDING;
vlan			return META_VLAN;
rxhash			return META_RXHASH;

devname			return META_DEVNAME;
sk_bound_if		return META_SK_BOUND_IF;


[^ \t\r\n+()=<>&|\"]+	{
				yylval->s = strdup(yytext);
				if (yylval->s == NULL)
					return ERROR;
				NL_DBG(4, "lex STR=%s\n", yylval->s);
				return STR;
			}
