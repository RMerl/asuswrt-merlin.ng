/*        $OpenBSD: route.c,v 1.66 2004/11/17 01:47:20 itojun Exp $        */
/*        $NetBSD: route.c,v 1.15 1996/05/07 02:55:06 thorpej Exp $        */

/*
 * Copyright (c) 1983, 1988, 1993
 *        The Regents of the University of California.  All rights reserved.
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
static char sccsid[] = "from: @(#)route.c        8.3 (Berkeley) 3/9/94";
#else
static char *rcsid = "$OpenBSD: route.c,v 1.66 2004/11/17 01:47:20 itojun Exp $";
#endif
#endif /* not lint */
#endif

#include <net-snmp/net-snmp-config.h>

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

#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_NETDB_H
#include <netdb.h>
#endif

#ifndef INET
#define INET
#endif

#include "main.h"
#include "netstat.h"
#if HAVE_WINSOCK_H
#include "winstub.h"
#endif

/* inetCidrRouteTable */
#define SET_IFNO 0x01
#define SET_TYPE 0x02
#define SET_PRTO 0x04
#define SET_AGE  0x08
#define SET_AS   0x10
#define SET_MET1 0x20
#define SET_ALL  0x3f

/* ip6RouteTable */
#define SET_HOP     0x40
#define SET_INVALID 0x80
#define SET_ALL6    0x67
/* not invalid, and only the columns that we fetch */

struct route_entry {
    int             af;
    struct sockaddr_storage dst;
    struct sockaddr_storage hop;
    int             mask;
    int             ifNumber;
    int             type;
    int             proto;
    int             age;
    int             as;
    int             metric1;
    int             set_bits;
    char            ifname[64];
};


static void pr_rtxhdr(int af, const char *table);
static void p_rtnodex( struct route_entry *rp );

/*
 * Print routing tables.
 */
int
routexpr(int af)
{
    struct route_entry  route, *rp = &route;
    oid    rtcol_oid[]  = { 1,3,6,1,2,1,4,24,7,1,0 }; /* inetCidrRouteEntry */
    size_t rtcol_len    = OID_LENGTH( rtcol_oid );
    netsnmp_variable_list *var = NULL, *vp;
    int hdr_af = AF_UNSPEC;
    int printed = 0;

#define ADD_RTVAR( x ) rtcol_oid[ rtcol_len-1 ] = x; \
    snmp_varlist_add_variable( &var, rtcol_oid, rtcol_len, ASN_NULL, NULL,  0)
    ADD_RTVAR( 7 );                 /* inetCidrRouteIfIndex */
    ADD_RTVAR( 8 );                 /* inetCidrRouteType    */
    ADD_RTVAR( 9 );                 /* inetCidrRouteProto   */
    ADD_RTVAR( 10 );                /* inetCidrRouteAge   */
    ADD_RTVAR( 11 );                /* inetCidrRouteNextHopAS   */
    ADD_RTVAR( 12 );                /* inetCidrRouteMetric1   */
#undef ADD_RTVAR

    /*
     * Now walk the inetCidrRouteTable, reporting the various route entries
     */
    while ( 1 ) {
        oid *op;
        unsigned char *cp;
        int i;

        if (netsnmp_query_getnext( var, ss ) != SNMP_ERR_NOERROR)
            break;
        rtcol_oid[ rtcol_len-1 ] = 7;        /* ifRouteIfIndex */
        if ( snmp_oid_compare( rtcol_oid, rtcol_len,
                               var->name, rtcol_len) != 0 )
            break;    /* End of Table */
        if (var->type == SNMP_NOSUCHOBJECT ||
                var->type == SNMP_NOSUCHINSTANCE ||
                var->type == SNMP_ENDOFMIBVIEW)
            break;
        memset( &route, 0, sizeof( struct route_entry ));
        /* Extract inetCidrRouteDest, inetCidrRoutePfxLen,
         * inetCidrRouteNextHop from index */
        switch (var->name[rtcol_len]) {
        case 1:
            {   struct sockaddr_in *sin = (struct sockaddr_in *)&route.dst;
                int len;
                route.af = AF_INET;
                sin->sin_family = AF_INET;
                op = var->name+rtcol_len+1;
                len = *op++;
                cp = (unsigned char *)&sin->sin_addr;
                for (i = 0; i < len; i++) *cp++ = *op++;
                route.mask = *op++;
                op += *op+1;
                op++; /* addrType */
                op++; /* addrLen */
                sin = (struct sockaddr_in *)&route.hop;
                sin->sin_family = AF_INET;
                cp = (unsigned char *)&sin->sin_addr;
                for (i = 0; i < len; i++) *cp++ = *op++;
                break;
            }
        case 2:
            {   struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&route.dst;
                int len;
                route.af = AF_INET6;
                sin6->sin6_family = AF_INET6;
                op = var->name+rtcol_len+1;
                len = *op++;
                cp = (unsigned char *)&sin6->sin6_addr;
                for (i = 0; i < len; i++) *cp++ = *op++;
                route.mask = *op++;
                op += *op+1;
                op++; /* addrType */
                op++; /* addrLen */
                sin6 = (struct sockaddr_in6 *)&route.hop;
                sin6->sin6_family = AF_INET6;
                cp = (unsigned char *)&sin6->sin6_addr;
                for (i = 0; i < len; i++) *cp++ = *op++;
                break;
            }
        default:
            fprintf(stderr, "Bad address type: %d\n", (int)var->name[rtcol_len]);
            exit(1);
        }
        /* Extract ipRouteDest index value */

        for ( vp=var; vp; vp=vp->next_variable ) {
            switch ( vp->name[ rtcol_len - 1 ] ) {
            case 7:     /* inetCidrRouteIfIndex */
                rp->ifNumber  = *vp->val.integer;
                rp->set_bits |= SET_IFNO;
                break;
            case 8:     /* inetCidrRouteType    */
                rp->type      = *vp->val.integer;
                rp->set_bits |= SET_TYPE;
                break;
            case 9:     /* inetCidrRouteProto   */
                rp->proto     = *vp->val.integer;
                rp->set_bits |= SET_PRTO;
                break;
            case 10:     /* inetCidrRouteAge   */
                rp->age     = *vp->val.integer;
                rp->set_bits |= SET_AGE;
                break;
            case 11:     /* inetCidrRouteNextHopAS   */
                rp->as     = *vp->val.integer;
                rp->set_bits |= SET_AS;
                break;
            case 12:     /* inetCidrRouteMetric1   */
                rp->metric1     = *vp->val.integer;
                rp->set_bits |= SET_MET1;
                break;
            }
        }
        if (rp->set_bits != SET_ALL) {
            continue;   /* Incomplete query */
        }
    
        if (af != AF_UNSPEC && rp->af != af)
            continue;

        if (hdr_af != rp->af) {
            if (hdr_af != AF_UNSPEC)
                printf("\n");
            hdr_af = rp->af;
            pr_rtxhdr(hdr_af, "inetCidrRouteTable");
        }
        p_rtnodex( rp );
        printed++;
    }
    snmp_free_varbind(var);
    return printed;
}

/*
 * Backwards-compatibility for the IPV6-MIB
 */
int
route6pr(int af)
{
    struct route_entry  route, *rp = &route;
    oid    rtcol_oid[]  = { 1,3,6,1,2,1,55,1,11,1,0 }; /* ipv6RouteEntry */
    size_t rtcol_len    = OID_LENGTH( rtcol_oid );
    netsnmp_variable_list *var = NULL, *vp;
    int printed = 0;
    int hdr_af = AF_UNSPEC;
    int i;

    if (af != AF_UNSPEC && af != AF_INET6)
        return 0;

#define ADD_RTVAR( x ) rtcol_oid[ rtcol_len-1 ] = x; \
    snmp_varlist_add_variable( &var, rtcol_oid, rtcol_len, ASN_NULL, NULL,  0)
    ADD_RTVAR( 4 );                 /* ipv6RouteIfIndex */
    ADD_RTVAR( 5 );                 /* ipv6RouteNextHop */
    ADD_RTVAR( 6 );                 /* ipv6RouteType    */
    ADD_RTVAR( 7 );                 /* ipv6RouteProto   */
    ADD_RTVAR( 11 );                /* ipv6RouteMetric  */
    ADD_RTVAR( 14 );                /* ipv6RouteValid   */
#undef ADD_RTVAR

    /*
     * Now walk the ipv6RouteTable, reporting the various route entries
     */
    while ( 1 ) {
        oid *op;
        unsigned char *cp, *cp1;
        struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&route.dst;

        if (netsnmp_query_getnext( var, ss ) != SNMP_ERR_NOERROR)
            break;
        rtcol_oid[ rtcol_len-1 ] = 4;        /* ipv6RouteIfIndex */
        if ( snmp_oid_compare( rtcol_oid, rtcol_len,
                               var->name, rtcol_len) != 0 )
            break;    /* End of Table */
        if (var->type == SNMP_NOSUCHOBJECT ||
                var->type == SNMP_NOSUCHINSTANCE ||
                var->type == SNMP_ENDOFMIBVIEW)
            break;
        memset( &route, 0, sizeof( struct route_entry ));
        rp->af = AF_INET6;
        sin6->sin6_family = AF_INET6;
        op = var->name+rtcol_len;
        cp = (unsigned char *)&sin6->sin6_addr;
        for (i = 0; i < 16; i++) *cp++ = *op++;
        route.mask = *op++;

        for ( vp=var; vp; vp=vp->next_variable ) {
            switch ( vp->name[ rtcol_len - 1 ] ) {
            case 4:     /* ipv6RouteIfIndex  */
                rp->ifNumber  = *vp->val.integer;
                /*
                 * This is, technically, an Ipv6IfIndex, which
                 * could maybe be different than the IfIndex
                 * for the same interface.  We ignore this
                 * possibility for now, in the hopes that
                 * nobody actually allocates these numbers
                 * differently.
                 */
                rp->set_bits |= SET_IFNO;
                break;
            case 5:     /* ipv6RouteNextHop  */
                cp1 = (unsigned char *)vp->val.string;
                sin6 = (struct sockaddr_in6 *)&rp->hop;
                sin6->sin6_family = AF_INET6;
                cp = (unsigned char *)&sin6->sin6_addr;
                for (i = 0; i < 16; i++) *cp++ = *cp1++;
                rp->set_bits |= SET_HOP;
		break;
            case 6:     /* ipv6RouteType     */
                rp->type      = *vp->val.integer;
                /* This enum maps to similar values in inetCidrRouteType */
                rp->set_bits |= SET_TYPE;
                break;
            case 7:     /* ipv6RouteProtocol */
                rp->proto     = *vp->val.integer;
                /* TODO: this does not map directly to the
                 * inetCidrRouteProtocol values.  If we use
                 * rp->proto more, we will have to manage this. */
                rp->set_bits |= SET_PRTO;
                break;
            case 11:    /* ipv6RouteMetric   */
                rp->metric1   = *vp->val.integer;
                rp->set_bits |= SET_MET1;
                break;
            case 14:    /* ipv6RouteValid    */
                if (*vp->val.integer == 2)
                    rp->set_bits |= SET_INVALID;
                break;
            }
        }
        if (rp->set_bits != SET_ALL6) {
            continue;   /* Incomplete query */
        }
    
        if (hdr_af != rp->af) {
            if (hdr_af != AF_UNSPEC)
                printf("\n");
            hdr_af = rp->af;
            pr_rtxhdr(AF_INET6, "ip6RouteTable");
        }
        p_rtnodex( rp );
        printed++;
    }
    snmp_free_varbind(var);
    return printed;
}


/* column widths; each followed by one space */
#ifndef NETSNMP_ENABLE_IPV6
#define        WID_DST(af)        26        /* width of destination column */
#define        WID_GW(af)        18        /* width of gateway column */
#else
/* width of destination/gateway column */
/* strlen("fe80::aaaa:bbbb:cccc:dddd@gif0") == 30, strlen("/128") == 4 */
#define        WID_DST(af)        ((af) == AF_INET6 ? (nflag ? 34 : 26) : 26)
#define        WID_GW(af)        ((af) == AF_INET6 ? (nflag ? 39 : 26) : 26)
#endif /* NETSNMP_ENABLE_IPV6 */

/*
 * Print header for routing table columns.
 */
static void
pr_rtxhdr(int af, const char *table)
{
    switch (af) {
    case AF_INET:
	printf("IPv4 Routing tables (inetCidrRouteTable)\n");
	break;
    case AF_INET6:
	printf("IPv6 Routing tables (%s)\n", table);
	break;
    }
    printf("%-*.*s ",
	WID_DST(af), WID_DST(af), "Destination");
    printf("%-*.*s %-6.6s  %s\n",
	WID_GW(af), WID_GW(af), "Gateway",
	"Flags", "Interface");
}

#ifndef HAVE_INET_NTOP
/* MSVC and MinGW */
#define inet_ntop netsnmp_inet_ntop
static const char *
netsnmp_inet_ntop(int af, const void *src, char *dst, size_t size)
{
    DWORD out_len = size;

    switch (af) {
    case AF_INET:
	{
	    struct sockaddr_in in;

	    memset(&in, 0, sizeof(in));
	    in.sin_family = af;
	    memcpy(&in.sin_addr, src, 4);
	    if (WSAAddressToString((struct sockaddr *)&in, sizeof(in), NULL, dst,
				   &out_len) == 0)
		return dst;
	}
	break;
    case AF_INET6:
	{
	    struct sockaddr_in6 in6;

	    memset(&in6, 0, sizeof(in6));
	    in6.sin6_family = af;
	    memcpy(&in6.sin6_addr, src, 16);
	    if (WSAAddressToString((struct sockaddr *)&in6, sizeof(in6), NULL, dst,
				   &out_len) == 0)
		return dst;
	}
	break;
    }
    return NULL;
}
#endif

/*
 * Return the name of the network whose address is given.
 * The address is assumed to be that of a net or subnet, not a host.
 */
static char *
netxname(struct sockaddr_storage *in, int mask)
{
    static char host[MAXHOSTNAMELEN];
    static char line[MAXHOSTNAMELEN];
    struct sockaddr_in *sin = (struct sockaddr_in *)in;
    struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)in;

    switch (in->ss_family) {
    case AF_INET:
        inet_ntop(in->ss_family, &sin->sin_addr, host, sizeof(host));
        if (mask == 32) strcpy(line, host);
        else snprintf(line, sizeof(line), "%s/%d", host, mask);
        break;
    case AF_INET6:
        inet_ntop(in->ss_family, &sin6->sin6_addr, host, sizeof(host));
        if (mask == 128) strcpy(line, host);
        else snprintf(line, sizeof(line), "%s/%d", host, mask);
        break;
    }
    return line;
}

static char *
routexname(struct sockaddr_storage *in)
{
    char *cp;
    static char line[MAXHOSTNAMELEN];
    struct hostent *hp = NULL;
    static char domain[MAXHOSTNAMELEN];
    static int first = 1;
    struct sockaddr_in *sin = (struct sockaddr_in *)in;
    struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)in;

    if (first) {
        first = 0;
        if (gethostname(line, sizeof line) == 0 &&
            (cp = strchr(line, '.')))
            (void) strlcpy(domain, cp + 1, sizeof domain);
        else
            domain[0] = '\0';
    }
    cp = NULL;
    if (!nflag) {
        switch (in->ss_family) {
        case AF_INET:
            hp = netsnmp_gethostbyaddr(&sin->sin_addr,
                    sizeof (struct in_addr), AF_INET);
            break;
        case AF_INET6:
            hp = netsnmp_gethostbyaddr(&sin6->sin6_addr,
                    sizeof (struct in6_addr), AF_INET6);
            break;
        }
        if (hp) {
            if ((cp = strchr(hp->h_name, '.')) && !strcmp(cp + 1, domain))
                *cp = '\0';
            cp = hp->h_name;
        }
    }
    if (cp) {
        strlcpy(line, cp, sizeof(line));
    } else {
        switch (in->ss_family) {
        case AF_INET:
            inet_ntop(sin->sin_family, &sin->sin_addr, line, sizeof(line));
            break;
        case AF_INET6:
            inet_ntop(sin6->sin6_family, &sin6->sin6_addr, line, sizeof(line));
            break;
        }
    }
    return (line);
}


static char *
s_rtflagsx( struct route_entry *rp )
{
    static char flag_buf[10];
    char  *cp = flag_buf;

    *cp++ = '<';
    *cp++ = 'U';   /* route is in use */
    if ((rp->af == AF_INET && rp->mask == 32) ||
            (rp->af == AF_INET6 && rp->mask == 128))
        *cp++ = 'H';   /* host */
    if (rp->proto == 4)
        *cp++ = 'D';   /* ICMP redirect */
    if (rp->type  == 4)
        *cp++ = 'G';   /* remote destination/net */
    *cp++ = '>';
    *cp = 0;
    return flag_buf;
}

static void
p_rtnodex( struct route_entry *rp )
{
    get_ifname(rp->ifname, rp->ifNumber);
    if (rp->af == AF_INET) {
        struct sockaddr_in *sin = (struct sockaddr_in *)&rp->dst;
        printf("%-*s ",
            WID_DST(AF_INET),
                (sin->sin_addr.s_addr == INADDR_ANY) ? "default" :
                (rp->mask == 32 ?
                    routexname(&rp->dst) :
                    netxname(&rp->dst, rp->mask)));
    }
    else if (rp->af == AF_INET6) {
        struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&rp->dst;
        struct in6_addr in6_addr_any = IN6ADDR_ANY_INIT;
        printf("%-*s ",
            WID_DST(AF_INET6),
                memcmp(&sin6->sin6_addr, &in6_addr_any, sizeof(in6_addr_any)) == 0 ? "default" :
                (rp->mask == 128 ?
                    routexname(&rp->dst) :
                    netxname(&rp->dst, rp->mask)));
    }
    printf("%-*s %-6.6s  %s",
        WID_GW(rp->af),
        1 ? routexname(&rp->hop) : "*",
        s_rtflagsx(rp), rp->ifname);
    if ((rp->set_bits & SET_AS) && rp->as != 0)
        printf(" (AS %d)", rp->as);
    printf("\n");
}
