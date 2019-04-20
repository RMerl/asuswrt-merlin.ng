%{
/*
 * (C) 2006 by Pablo Neira Ayuso <pablo@netfilter.org>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Description: configuration file syntax
 */

#include <string.h>

#include "conntrackd.h"
#include "read_config_yy.h"
%}

%option yylineno
%option noinput
%option nounput

ws		[ \t]+
comment         #.*$
nl		[\n\r]

is_on		[o|O][n|N]
is_off		[o|O][f|F][f|F]
integer		[0-9]+
signed_integer	[\-\+][0-9]+
path		\/[^\"\n ]*
ip4_cidr	\/[0-2]*[0-9]+
ip4_end		[0-9]*[0-9]+
ip4_part	[0-2]*{ip4_end}
ip4		{ip4_part}\.{ip4_part}\.{ip4_part}\.{ip4_part}{ip4_cidr}?
hex_255		[0-9a-fA-F]{1,4}
ip6_cidr	\/[0-1]*[0-9]*[0-9]+
ip6_part	{hex_255}":"?
ip6_form1	{ip6_part}{0,7}"::"{ip6_part}{0,7}
ip6_form2	({hex_255}":"){0,7}{hex_255}
ip6		{ip6_form1}{ip6_cidr}?|{ip6_form2}{ip6_cidr}?
string		[a-zA-Z][a-zA-Z0-9\.\-\_]*
persistent	[P|p][E|e][R|r][S|s][I|i][S|s][T|t][E|e][N|n][T|T]
nack		[N|n][A|a][C|c][K|k]
alarm		[A|a][L|l][A|a][R|r][M|m]
ftfw		[F|f][T|t][F|f][W|w]
notrack		[N|n][O|o][T|t][R|r][A|a][C|c][K|k]

%%
"UNIX"				{ return T_UNIX; }
"IPv4_address"			{ return T_IPV4_ADDR; }
"IPv6_address"			{ return T_IPV6_ADDR; }
"IPv4_Destination_Address"	{ return T_IPV4_DEST_ADDR; }
"IPv6_Destination_Address"	{ return T_IPV6_DEST_ADDR; }
"IPv4_interface"		{ return T_IPV4_IFACE; }
"IPv6_interface"		{ return T_IPV6_IFACE; }
"Interface"			{ return T_IFACE; }
"Multicast"			{ return T_MULTICAST; }
"UDP"				{ return T_UDP; }
"TCP"				{ return T_TCP; }
"HashSize"			{ return T_HASHSIZE; }
"RefreshTime"			{ return T_REFRESH; }
"CacheTimeout"			{ return T_EXPIRE; }
"CommitTimeout"			{ return T_TIMEOUT; }
"HashLimit"			{ return T_HASHLIMIT; }
"Path"				{ return T_PATH; }
"Backlog"			{ return T_BACKLOG; }
"Group"				{ return T_GROUP; }
"Port"				{ return T_PORT; }
"LogFile"			{ return T_LOG; }
"Syslog"			{ return T_SYSLOG; }
"LockFile"			{ return T_LOCK; }
"General"			{ return T_GENERAL; }
"Sync"				{ return T_SYNC; }
"Stats"				{ return T_STATS; }
"SocketBufferSize"		{ return T_BUFFER_SIZE; /* alias */ }
"SocketBufferSizeMaxGrown"	{ return T_BUFFER_SIZE_MAX_GROWN; /* alias */ }
"SocketBufferSizeMaxGrowth"	{ return T_BUFFER_SIZE_MAX_GROWN; /* alias */ }
"NetlinkBufferSize"		{ return T_BUFFER_SIZE; }
"NetlinkBufferSizeMaxGrowth"	{ return T_BUFFER_SIZE_MAX_GROWN; }
"Mode"				{ return T_SYNC_MODE; }
"ResendQueueSize"		{ return T_RESEND_QUEUE_SIZE; }
"Checksum"			{ return T_CHECKSUM; }
"ACKWindowSize"			{ return T_WINDOWSIZE; }
"for"				{ return T_FOR; }
"SYN_SENT"			{ return T_SYN_SENT; }
"SYN_RECV"			{ return T_SYN_RECV; }
"ESTABLISHED"			{ return T_ESTABLISHED; }
"FIN_WAIT"			{ return T_FIN_WAIT; }
"CLOSE_WAIT"			{ return T_CLOSE_WAIT; }
"LAST_ACK"			{ return T_LAST_ACK; }
"TIME_WAIT"			{ return T_TIME_WAIT; }
"CLOSE"				{ return T_CLOSE; /* alias of CLOSED */ }
"CLOSED"			{ return T_CLOSE; }
"LISTEN"			{ return T_LISTEN; }
"SndSocketBuffer"		{ return T_SNDBUFF; }
"RcvSocketBuffer"		{ return T_RCVBUFF; }
"Filter"			{ return T_FILTER; }
"Protocol"			{ return T_PROTOCOL; }
"Address"			{ return T_ADDRESS; }
"State"				{ return T_STATE; }
"Accept"			{ return T_ACCEPT; }
"Ignore"			{ return T_IGNORE; }
"PurgeTimeout"			{ return T_PURGE; }
"From"				{ return T_FROM; }
"Userspace"			{ return T_USERSPACE; }
"Kernelspace"			{ return T_KERNELSPACE; }
"EventIterationLimit"		{ return T_EVENT_ITER_LIMIT; }
"Default"			{ return T_DEFAULT; }
"PollSecs"			{ return T_POLL_SECS; }
"NetlinkOverrunResync"		{ return T_NETLINK_OVERRUN_RESYNC; }
"Nice"				{ return T_NICE; }
"Scheduler"			{ return T_SCHEDULER; }
"Type"				{ return T_TYPE; }
"Priority"			{ return T_PRIO; }
"NetlinkEventsReliable"		{ return T_NETLINK_EVENTS_RELIABLE; }
"DisableInternalCache"		{ return T_DISABLE_INTERNAL_CACHE; }
"DisableExternalCache"		{ return T_DISABLE_EXTERNAL_CACHE; }
"Options"			{ return T_OPTIONS; }
"TCPWindowTracking"		{ return T_TCP_WINDOW_TRACKING; }
"ExpectationSync"		{ return T_EXPECT_SYNC; }
"ErrorQueueLength"		{ return T_ERROR_QUEUE_LENGTH; }
"Helper"			{ return T_HELPER; }
"QueueNum"			{ return T_HELPER_QUEUE_NUM; }
"QueueLen"			{ return T_HELPER_QUEUE_LEN; }
"Policy"			{ return T_HELPER_POLICY; }
"ExpectMax"			{ return T_HELPER_EXPECT_MAX; }
"ExpectTimeout"			{ return T_HELPER_EXPECT_TIMEOUT; }
"Systemd"			{ return T_SYSTEMD; }
"StartupResync"			{ return T_STARTUP_RESYNC; }

{is_on}			{ return T_ON; }
{is_off}		{ return T_OFF; }
{integer}		{ yylval.val = atoi(yytext); return T_NUMBER; }
{signed_integer}	{ yylval.val = atoi(yytext); return T_SIGNED_NUMBER; }
{ip4}			{ yylval.string = yytext; return T_IP; }
{ip6}			{ yylval.string = yytext; return T_IP; }
{path}			{ yylval.string = yytext; return T_PATH_VAL; }
{alarm}			{ return T_ALARM; }
{persistent}		{ dlog(LOG_WARNING, "Now `persistent' mode "
			       "is called `alarm'. Please, update "
				"your conntrackd.conf file.");
			  return T_ALARM; }
{ftfw}			{ return T_FTFW; }
{nack}			{ dlog(LOG_WARNING, "Now `nack' mode "
			       "is called `ftfw'. Please, update "
			       "your conntrackd.conf file.\n");
			  return T_FTFW; }
{notrack}		{ return T_NOTRACK; }
{string}		{ yylval.string = yytext; return T_STRING; }

{comment}	;
{ws}		;
{nl}		;

<<EOF>>		{ yyterminate(); }

.		{ return yytext[0]; }

%%

int
yywrap()
{
	return 1;
}
