/*	$OpenBSD: inet6.c,v 1.31 2004/11/17 01:47:20 itojun Exp $	*/
/*	BSDI inet.c,v 2.3 1995/10/24 02:19:29 prb Exp	*/
/*
 * Copyright (c) 1983, 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifdef  INHERITED_CODE
#ifndef lint
#if 0
static char sccsid[] = "@(#)inet.c	8.4 (Berkeley) 4/20/94";
#else
/*__RCSID("$OpenBSD: inet6.c,v 1.31 2004/11/17 01:47:20 itojun Exp $");*/
/*__RCSID("KAME Id: inet6.c,v 1.10 2000/02/09 10:49:31 itojun Exp");*/
#endif
#endif /* not lint */
#endif

#include <net-snmp/net-snmp-config.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_WINSOCK_H
#include "winstub.h"
#endif
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#if HAVE_NETDB_H
#include <netdb.h>
#endif
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#include <net-snmp/net-snmp-includes.h>

#include "main.h"
#include "netstat.h"

struct stat_table {
    unsigned int entry;      /* entry number in table */
    /*
     * format string to printf(description, value) 
     * warning: the %d must be before the %s 
     */
    char            description[80];
};

void	inetxprint(int, struct sockaddr_in6 , int, const char *, int);

/*
 * Print a summary of TCP connections
 * Listening processes are suppressed unless the
 *   -a (all) flag is specified.
 */
const char     *tcpxstates[] = {
    "",
    "CLOSED",
    "LISTEN",
    "SYNSENT",
    "SYNRECEIVED",
    "ESTABLISHED",
    "FINWAIT1",
    "FINWAIT2",
    "CLOSEWAIT",
    "LASTACK",
    "CLOSING",
    "TIMEWAIT"
};
#define TCP_NSTATES 11

typedef struct {
    int stat;
    int hcstat;
    const char *str;
} systemstats_t;

systemstats_t systemstats[] = {
    {  3,  4, "datagrams input" },
    {  5,  6, "octets received" },
    {  7,  0, "hdr errors input" },
    {  8,  0, "no routes input" },
    {  9,  0, "address errors input" },
    { 10,  0, "unknown protocol input" },
    { 12, 13, "input datagrams forwarded" },
    { 11,  0, "truncated datagrams input" },
    { 14,  0, "input reassembly required" },
    { 15,  0, "input reassemled OK" },
    { 16,  0, "input reassembly failed" },
    { 17,  0, "input datagrams discarded" },
    { 18, 19, "input datagrams received" },
    { 20, 21, "output datagram requests" },
    { 22,  0, "output no route" },
    { 23, 24, "datagrams forwarded" },
    { 25,  0, "output datagrams discarded" },
    { 26,  0, "output datagrams fragmentation required" },
    { 27,  0, "output datagrams fragmented" },
    { 28,  0, "output fragmentation failed" },
    { 29,  0, "fragments created" },
    { 30, 31, "datagrams transmitted" },
    { 32, 33, "octets transmitted" },
    {  0 }
};

systemstats_t icmpstats[] = {
    {  2,  0, "input messages" },
    {  3,  0, "input errors" },
    {  4,  0, "output messages" },
    {  5,  0, "output errors" },
    {  0 }
};

typedef struct {
    int code;
    const char *name;
} codelist_t;

codelist_t icmpcodes[] = {
    {   0, "Echo reply" },
    {   3, "Destination unreachable" },
    {   4, "Source quench" },
    {   5, "Redirect" },
    {   6, "Alternate host address" },
    {   8, "Echo request" },
    {   9, "Router advertisement" },
    {  10, "Router selection" },
    {  11, "Time exceeded" },
    {  12, "Parameter problem" },
    {  13, "Timestamp request" },
    {  14, "Timestamp reply" },
    {  15, "Information request" },
    {  16, "Information reply" },
    {  17, "Address mask request" },
    {  18, "Address mask reply" },
    { 0 }
};

codelist_t icmp6codes[] = {
    {   1,   "Destination Unreachable" },
    {   2,   "Packet Too Big" },
    {   3,   "Time Exceeded" },
    {   4,   "Parameter Problem" },
    { 100,   "Private experimentation 100" },
    { 101,   "Private experimentation 101" },
    { 127,   "Reserved for expansion of ICMPv6 error messages" },
    { 128,   "Echo Request" },
    { 129,   "Echo Reply" },
    { 130,   "Multicast Listener Query" },
    { 131,   "Multicast Listener Report" },
    { 132,   "Multicast Listener Done" },
    { 133,   "Router Solicitation" },
    { 134,   "Router Advertisement" },
    { 135,   "Neighbor Solicitation" },
    { 136,   "Neighbor Advertisement" },
    { 137,   "Redirect Message" },
    { 138,   "Router Renumbering" },
    { 139,   "ICMP Node Information Query" },
    { 140,   "ICMP Node Information Response" },
    { 141,   "Inverse Neighbor Discovery Solicitation Message" },
    { 142,   "Inverse Neighbor Discovery Advertisement Message" },
    { 143,   "Version 2 Multicast Listener Report" },
    { 144,   "Home Agent Address Discovery Request Message" },
    { 145,   "Home Agent Address Discovery Reply Message" },
    { 146,   "Mobile Prefix Solicitation" },
    { 147,   "Mobile Prefix Advertisement" },
    { 148,   "Certification Path Solicitation Message" },
    { 149,   "Certification Path Advertisement Message" },
    { 151,   "Multicast Router Advertisement" },
    { 152,   "Multicast Router Solicitation" },
    { 153,   "Multicast Router Termination" },
    { 154,   "FMIPv6 Messages" },
    { 155,   "RPL Control Message" },
    { 0 }
};


void
tcpxprotopr(const char *name)
{
    netsnmp_variable_list *var, *vp, *pvar;
    oid    tcpConnectionState_oid[] = { 1,3,6,1,2,1,6,19,1,7 };
    size_t tcpConnectionState_len   = OID_LENGTH( tcpConnectionState_oid );
    int    state, i;
    struct sockaddr_in6 localAddr, remoteAddr;
    int    localPort,     remotePort,  pid = 0;
    int    localType,    remoteType, inx;
    int    first = 1;
    static int done = 0;

    if (done++) return;

    /*
     * Walking the v6 tcpConnectionState column will provide all
     *   the necessary information.
     */
    var = NULL;
    snmp_varlist_add_variable( &var, tcpConnectionState_oid,
                                     tcpConnectionState_len,
                                     ASN_NULL, NULL,  0);
    if (netsnmp_query_walk( var, ss ) != SNMP_ERR_NOERROR) {
        snmp_free_varbind(var);
        return;
    }
    if ((var->type & 0xF0) == 0x80) {	/* Exception */
        snmp_free_varbind(var);
        return;
    }

    for (vp = var; vp ; vp=vp->next_variable) {
	char lname[5];
        state = *vp->val.integer;
	inx = tcpConnectionState_len;
	pvar = NULL;

	vp->name[inx-1] = 8;
	snmp_varlist_add_variable( &pvar, vp->name, vp->name_length,
					 ASN_NULL, NULL,  0);
	if (netsnmp_query_get( pvar, ss ) != SNMP_ERR_NOERROR) {
	    snmp_free_var( pvar );
	    return;
	}
	if ((pvar->type & 0xF0) != 0x80)	/* Exception */
	    pid = *pvar->val.integer;
        
        /* Extract the local/remote information from the index values */
	localType = vp->name[inx++];
	for (i = 0; i < vp->name[inx]; i++)
	    localAddr.sin6_addr.s6_addr[i] = vp->name[inx+i+1];
	inx += vp->name[inx] + 1;
        localPort    = vp->name[inx++];
	remoteType = vp->name[inx++];
	for (i = 0; i < vp->name[inx]; i++)
	    remoteAddr.sin6_addr.s6_addr[i] = vp->name[inx+i+1];
	inx += vp->name[inx] + 1;
        remotePort    = vp->name[inx++];

	snmp_free_varbind(pvar);

	if (af == AF_INET && localType == 2) continue;
	if (af == AF_INET6 && localType != 2) continue;

        if (first) {
            printf("Active Internet (%s) Connections", "tcp");
            putchar('\n');
            printf("%-5.5s %-27.27s %-27.27s %11.11s %5.5s\n",
                   "Proto", "Local Address", "Remote Address", "State", "PID");
            first = 0;
        }

	strcpy(lname, "tcp");
	if (localType == 2) lname[3] = '6';
	else lname[3] = '4';
	lname[4] = 0;
        printf("%-5.5s", lname);
        inetxprint(localType, localAddr,  localPort, "tcp", 1);
        inetxprint(remoteType, remoteAddr, remotePort, "tcp", 0);
        if ( state < 1 || state > TCP_NSTATES )
            printf(" %11d %5d\n", state, pid);
        else
            printf(" %11s %5d\n", tcpxstates[ state ], pid);
    }
    snmp_free_varbind( var );

    if (aflag)
	listenxprotopr(name);
}

/*
 * Print a summary of listening "connections"
 */
void
listenxprotopr(const char *name)
{
    netsnmp_variable_list *var, *vp;
    oid    tcpListenerProcess_oid[] = { 1,3,6,1,2,1,6,20,1,4 };
    size_t tcpListenerProcess_len   = OID_LENGTH( tcpListenerProcess_oid );
    struct sockaddr_in6 localAddr;
    int    localType, localPort, pid;
    int    i, inx;

    /*
     * Walking a single column of the udpTable will provide
     *   all the necessary information from the index values.
     */
    var = NULL;
    snmp_varlist_add_variable( &var, tcpListenerProcess_oid,
                                     tcpListenerProcess_len,
                                     ASN_NULL, NULL,  0);
    if (netsnmp_query_walk( var, ss ) != SNMP_ERR_NOERROR)
        return;
    if ((var->type & 0xF0) == 0x80)	/* Exception */
        return;

    printf("Listening Internet (%s) Connections\n", "tcp");
    printf("%-5.5s %-27.27s %5s\n", "Proto", "Local Address", "PID");
    for (vp = var; vp ; vp=vp->next_variable) {
	char lname[5];
	inx = tcpListenerProcess_len;
        /*
         * Extract the local port from the index values, but take
         *   the IP address from the varbind value, (which is why
         *   we walked udpLocalAddress rather than udpLocalPort)
         */
	localType = vp->name[inx++];
	if (af == AF_INET && localType == 2) continue;
	if (af == AF_INET6 && localType != 2) continue;

	for (i = 0; i < vp->name[inx]; i++)
	    localAddr.sin6_addr.s6_addr[i] = vp->name[inx+i+1];
	inx += vp->name[inx]+1;
        localPort = vp->name[ inx++ ];
        pid   = *vp->val.integer;
	strcpy(lname, "tcp");
	if (localType == 2) lname[3] = '6';
	else lname[3] = '4';
	lname[4] = 0;
        printf("%-5.5s", lname);
        inetxprint(localType, localAddr, localPort, "tcp", 1);
        printf(" %5d\n", pid);
    }
    snmp_free_varbind( var );
}

/*
 * Print a summary of UDPv6 "connections"
 *    XXX - what about "listening" services ??
 */
void
udpxprotopr(const char *name)
{
    netsnmp_variable_list *var, *vp;
    oid    udpEndpointProcess_oid[] = { 1,3,6,1,2,1,7,7,1,8 };
    size_t udpEndpointProcess_len   = OID_LENGTH( udpEndpointProcess_oid );
    struct sockaddr_in6 localAddr, remoteAddr;
    int    localType, remoteType, localPort, remotePort, pid;
    int    i, inx;
    static int done = 0;

    if (done++) return;

    /*
     * Walking a single column of the udpTable will provide
     *   all the necessary information from the index values.
     */
    var = NULL;
    snmp_varlist_add_variable( &var, udpEndpointProcess_oid,
                                     udpEndpointProcess_len,
                                     ASN_NULL, NULL,  0);
    if (netsnmp_query_walk( var, ss ) != SNMP_ERR_NOERROR) {
	snmp_free_varbind(var);
        return;
    }
    if ((var->type & 0xF0) == 0x80) {	/* Exception */
	snmp_free_varbind(var);
        return;
    }

    printf("Active Internet (%s) Connections\n", "udp");
    printf("%-5.5s %-27.27s %-27.27s %5s\n", "Proto", "Local Address", "Remote Address", "PID");
    for (vp = var; vp ; vp=vp->next_variable) {
        char lname[5];
	inx = udpEndpointProcess_len;
        /*
         * Extract the local port from the index values, but take
         *   the IP address from the varbind value, (which is why
         *   we walked udpLocalAddress rather than udpLocalPort)
         */
	localType = vp->name[inx++];
	if (af == AF_INET && localType == 2) continue;
	if (af == AF_INET6 && localType != 2) continue;
	for (i = 0; i < vp->name[inx]; i++)
	    localAddr.sin6_addr.s6_addr[i] = vp->name[inx+i+1];
	inx += vp->name[inx]+1;
        localPort = vp->name[ inx++ ];
	remoteType = vp->name[inx++];
	for (i = 0; i < vp->name[inx]; i++)
	    remoteAddr.sin6_addr.s6_addr[i] = vp->name[inx+i+1];
	inx += vp->name[inx]+1;
        remotePort = vp->name[ inx++ ];
        pid   = *vp->val.integer;
	strcpy(lname, "udp");
	if (localType == 2) lname[3] = '6';
	else lname[3] = '4';
	lname[4] = 0;
        printf("%-5.5s", lname);
        inetxprint(localType, localAddr, localPort, "udp", 1);
        inetxprint(remoteType, remoteAddr, remotePort, "udp", 1);
        printf(" %5d\n", pid);
    }
    snmp_free_varbind( var );
}

static void
statsprint(const char *name, const systemstats_t *st, int proto,
	const oid *tbl, size_t tbllen)
{
    oid var[32];
    size_t len;
    netsnmp_variable_list *vb;

    memcpy(var, tbl, tbllen*sizeof(oid));
    var[tbllen+1] = proto;
    len = tbllen+2;

    printf("%s:\n", name);
    while (st->stat) {
	vb = NULL;
	if (st->hcstat) {
	    var[tbllen] = st->hcstat;
	    snmp_varlist_add_variable( &vb, var, len, ASN_NULL, NULL,  0);
	    if (netsnmp_query_get( vb, ss ) != SNMP_ERR_NOERROR) {
		snmp_free_var( vb );
		vb = NULL;
	    }
	}
	if (!vb) {
	    var[tbllen] = st->stat;
	    snmp_varlist_add_variable( &vb, var, len, ASN_NULL, NULL, 0);
	    if (netsnmp_query_get( vb, ss ) != SNMP_ERR_NOERROR) {
		snmp_free_var( vb );
		vb = NULL;
	    }
	}
	if (vb) {
	    if (vb->type == ASN_COUNTER) {
		if (*vb->val.integer > 0 || sflag == 1)
		    printf("%14lu %s\n", *vb->val.integer, st->str);
	    }
	    else if (vb->type == ASN_COUNTER64) {
		char a64buf[I64CHARSZ + 1];
		printU64(a64buf, vb->val.counter64);
		if (strcmp(a64buf, "0") != 0 || sflag == 1)
		    printf("%14s %s\n", a64buf, st->str);
	    }
	    else
		printf("%14s %s\n", "-", st->str);
	    snmp_free_varbind(vb);
	}
	else {
	    printf("%14s %s\n", "-", st->str);
	}
	st++;
    }
}

static void
prhisto(const char *name, const oid *var, size_t len, int ver, codelist_t *cs)
{
    netsnmp_variable_list *vb = NULL, *vp;
    codelist_t *cp;
    int code;
    char nocode[32];

    snmp_varlist_add_variable( &vb, var, len, ASN_NULL, NULL,  0);
    if (netsnmp_query_walk( vb, ss ) != SNMP_ERR_NOERROR) {
	snmp_free_var( vb );
	return;
    }
    printf("     %s histogram:\n", name);
    printf("     %10s %10s %s\n", "input", "output", "type");
    for (code = 0; code < 256; code++) {
	unsigned long inp = 0, out = 0;
	int found = 0;
	vp = vb;
	while (vp && found != 2) {
	    if (vp->name[11] == code && vp->name[10] == ver) {
		if (vp->name[9] == 3) inp = *vp->val.integer;
		else out = *vp->val.integer;
		found++;
	    }
	    vp = vp->next_variable;
	}
	if (found) {
	    cp = cs;
	    while (cp->name && cp->code != code) cp++;
	    if (inp || out || sflag == 1) {
		if (!cp->code)
		    snprintf(nocode, sizeof nocode, "type %d", code);
		printf("     %10lu %10lu %s\n", inp, out, cp->name ? cp->name : nocode);
	    }
	}
    }
    snmp_free_varbind(vb);
}

void
ipx_stats(const char *name)
{
    oid ipsysstat_oid[] = { 1, 3, 6, 1, 2, 1, 4, 31, 1, 1 };
    size_t ipsysstat_len = sizeof(ipsysstat_oid) / sizeof(ipsysstat_oid[0]);
    static int first = 1;

    if (!first) return;
    first = 0;

    if (!name || strcmp(name, "ip") == 0)
	statsprint("ip", systemstats, 1, ipsysstat_oid, ipsysstat_len);
    if (!name || strcmp(name, "ip6") == 0)
	statsprint("ip6", systemstats, 2, ipsysstat_oid, ipsysstat_len);
}

void
icmpx_stats(const char *name)
{
    oid icmpstat_oid[] = { 1, 3, 6, 1, 2, 1, 5, 29, 1 };
    size_t icmpstat_len = sizeof(icmpstat_oid) / sizeof(icmpstat_oid[0]);
    oid icmpmsg_oid[] = { 1, 3, 6, 1, 2, 1, 5, 30, 1 };
    size_t icmpmsg_len = sizeof(icmpmsg_oid) / sizeof(icmpmsg_oid[0]);
    static int first = 1;

    if (!first)
	return;
    first = 0;

    if (!name || strcmp(name, "icmp") == 0) {
	statsprint("icmp", icmpstats, 1, icmpstat_oid, icmpstat_len);
	prhisto("icmp", icmpmsg_oid, icmpmsg_len, 1, icmpcodes);
    }
    if (!name || strcmp(name, "icmp6") == 0) {
	statsprint("icmp6", icmpstats, 2, icmpstat_oid, icmpstat_len);
	prhisto("icmp6", icmpmsg_oid, icmpmsg_len, 2, icmp6codes);
    }
}


static void
unknownprint(void)
{
    char line[80], *cp;
    int width = 27;

    if (vflag)
	snprintf(line, sizeof line, "%s.", "*");
    else
	snprintf(line, sizeof line, "%.*s.", width-9, "*");
    cp = strchr(line, '\0');
    snprintf(cp, line + sizeof line - cp, vflag ? "%s" : "%.8s", "*");
    if (vflag && width < strlen(line))
	width = strlen(line);
    printf(" %-*.*s", width, width, line);
}

/*
 * Pretty print an Internet address (net address + port).
 * If the nflag was specified, use numbers instead of names.
 */

void
inetxprint(int proto, struct sockaddr_in6 in6, int port, const char *name, int local)
{

	if (proto == 2)
	    inet6print((u_char *)&in6.sin6_addr.s6_addr, port, name, local);
	else if (proto == 1)
	    inetprint((struct in_addr *)&in6.sin6_addr.s6_addr, port, name, local);
	else if (proto == 0)
	    unknownprint();
	else abort();
}
