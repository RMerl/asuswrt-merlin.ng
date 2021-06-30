/* IPV6 base transport support functions
 *
 * Portions of this file are subject to the following copyright(s).  See
 * the Net-SNMP's COPYING file for more details and other copyrights
 * that may apply:
 *
 * Portions of this file are copyrighted by:
 * Copyright (c) 2016 VMware, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */

#include <net-snmp/net-snmp-config.h>

#ifdef NETSNMP_ENABLE_IPV6

#include <net-snmp/types.h>
#include <net-snmp/library/snmpIPBaseDomain.h>
#include <net-snmp/library/snmpIPv6BaseDomain.h>
#include <net-snmp/library/system.h>
#include <net-snmp/library/snmp_assert.h>

#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>
#include <ctype.h>
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#if HAVE_NETDB_H
#include <netdb.h>
#endif
#if HAVE_NET_IF_H
#include <net/if.h>
#endif

#include <net-snmp/types.h>
#include <net-snmp/library/snmp_debug.h>
#include <net-snmp/library/default_store.h>
#include <net-snmp/library/snmp_logging.h>

#include "inet_ntop.h"
#include "inet_pton.h"


#if defined(WIN32) && !defined(IF_NAMESIZE)
#define IF_NAMESIZE 12
#endif


#if defined(HAVE_WINSOCK_H) && !defined(mingw32)
static const struct in6_addr in6addr_any; /*IN6ADDR_ANY_INIT*/
#endif


#if HAVE_STRUCT_SOCKADDR_IN6_SIN6_SCOPE_ID
static unsigned
netsnmp_if_nametoindex(const char *ifname)
{
#if defined(WIN32)
    return atoi(ifname);
#elif defined(HAVE_IF_NAMETOINDEX)
    int res;

    res = if_nametoindex(ifname);
    if (res == 0)
        res = atoi(ifname);

    return res;
#else
    return 0;
#endif
}

static char *
netsnmp_if_indextoname(unsigned ifindex, char *ifname)
{
#if defined(WIN32)
    snprintf(ifname, IF_NAMESIZE, "%u", ifindex);
    return ifname;
#elif defined(HAVE_IF_NAMETOINDEX)
    return if_indextoname(ifindex, ifname);
#else
    return NULL;
#endif
}
#endif /* HAVE_STRUCT_SOCKADDR_IN6_SIN6_SCOPE_ID */

char *
netsnmp_ipv6_fmtaddr(const char *prefix, netsnmp_transport *t,
                     const void *data, int len)
{
    const struct sockaddr_in6 *to;
    char scope_id[IF_NAMESIZE + 1] = "";
    char addr[INET6_ADDRSTRLEN];
    char *tmp;

    DEBUGMSGTL(("netsnmp_ipv6", "fmtaddr: t = %p, data = %p, len = %d\n", t,
                data, len));

    if (t && !data) {
        data = t->data;
        len = t->data_length;
    }

    switch (data ? len : 0) {
    case sizeof(struct sockaddr_in6):
        to = data;
        break;
    case sizeof(netsnmp_indexed_addr_pair): {
        const netsnmp_indexed_addr_pair *addr_pair = data;

        to = (const struct sockaddr_in6 *)&addr_pair->remote_addr;
        break;
    }
    default:
        netsnmp_assert(0);
        if (asprintf(&tmp, "%s: unknown", prefix) < 0)
            tmp = NULL;
        return tmp;
    }

    netsnmp_assert(to->sin6_family == AF_INET6);

    if (t && t->flags & NETSNMP_TRANSPORT_FLAG_HOSTNAME) {
	struct hostent *host;
	host = netsnmp_gethostbyaddr(&to->sin6_addr, sizeof(struct in6_addr), AF_INET6);
	return (host ? strdup(host->h_name) : NULL);
    } else {
#if defined(HAVE_STRUCT_SOCKADDR_IN6_SIN6_SCOPE_ID)
	if (to->sin6_scope_id &&
            netsnmp_if_indextoname(to->sin6_scope_id, &scope_id[1]))
            scope_id[0] = '%';
#endif
        inet_ntop(AF_INET6, &to->sin6_addr, addr, sizeof(addr));
        if (asprintf(&tmp, "%s: [%s%s]:%hu", prefix, addr, scope_id,
		     ntohs(to->sin6_port)) < 0)
            tmp = NULL;
    }
    return tmp;
}

void netsnmp_ipv6_get_taddr(struct netsnmp_transport_s *t, void **addr,
                            size_t *addr_len)
{
    struct sockaddr_in6 *sin6 = t->remote;

    netsnmp_assert(t->remote_length == sizeof(*sin6));

    *addr_len = 18;
    if ((*addr = malloc(*addr_len))) {
        unsigned char *p = *addr;

        memcpy(p,      &sin6->sin6_addr, 16);
        memcpy(p + 16, &sin6->sin6_port, 2);
    }
}

int netsnmp_ipv6_ostring_to_sockaddr(struct sockaddr_in6 *sin6, const void *o,
                                     size_t o_len)
{
    const char *p = o;

    if (o_len != 18)
        return 0;

    memset(sin6, 0, sizeof(*sin6));
    sin6->sin6_family = AF_INET6;
    memcpy(&sin6->sin6_addr, p + 0,  16);
    memcpy(&sin6->sin6_port, p + 16, 2);
    return 1;
}

static int netsnmp_resolve_v6_hostname(struct in6_addr *addr,
                                       const char *hostname)
{
#if HAVE_GETADDRINFO
    struct addrinfo hint = { 0 };
    struct addrinfo *addrs;
    int             err;

    hint.ai_family = PF_INET6;
    hint.ai_socktype = SOCK_DGRAM;
    err = netsnmp_getaddrinfo(hostname, NULL, &hint, &addrs);
    if (err)
        return 0;

    if (addrs) {
        DEBUGMSGTL(("netsnmp_sockaddr_in6", "hostname (resolved okay)\n"));
        *addr = ((struct sockaddr_in6 *)addrs->ai_addr)->sin6_addr;
        freeaddrinfo(addrs);
    } else {
        DEBUGMSGTL(("netsnmp_sockaddr_in6", "Failed to resolve IPv6 hostname\n"));
    }
    return 1;
#elif HAVE_GETIPNODEBYNAME
    struct hostent *hp;
    int             err;

    hp = getipnodebyname(hostname, AF_INET6, 0, &err);
    if (hp == NULL) {
        DEBUGMSGTL(("netsnmp_sockaddr_in6",
                    "hostname (couldn't resolve = %d)\n", err));
        return 0;
    }
    DEBUGMSGTL(("netsnmp_sockaddr_in6", "hostname (resolved okay)\n"));
    memcpy(addr, hp->h_addr, hp->h_length);
    return 1;
#elif HAVE_GETHOSTBYNAME
    struct hostent *hp;

    hp = netsnmp_gethostbyname(hostname);
    if (hp == NULL) {
        DEBUGMSGTL(("netsnmp_sockaddr_in6",
                    "hostname (couldn't resolve)\n"));
        return 0;
    }
    if (hp->h_addrtype != AF_INET6) {
        DEBUGMSGTL(("netsnmp_sockaddr_in6", "hostname (not AF_INET6!)\n"));
        return 0;
    }
    DEBUGMSGTL(("netsnmp_sockaddr_in6", "hostname (resolved okay)\n"));
    memcpy(addr, hp->h_addr, hp->h_length);
    return 1;
#else                           /*HAVE_GETHOSTBYNAME */
    /*
     * There is no name resolving function available.
     */
    snmp_log(LOG_ERR,
             "no getaddrinfo()/getipnodebyname()/gethostbyname()\n");
    return 0;
#endif                          /*HAVE_GETHOSTBYNAME */
}

int
netsnmp_sockaddr_in6_2(struct sockaddr_in6 *addr,
                       const char *inpeername, const char *default_target)
{
    struct netsnmp_ep ep;
    int ret;

    ret = netsnmp_sockaddr_in6_3(&ep, inpeername, default_target);
    if (ret == 0)
        return 0;
    *addr = ep.a.sin6;
    return ret;
}

int
netsnmp_sockaddr_in6_3(struct netsnmp_ep *ep,
                       const char *inpeername, const char *default_target)
{
    struct sockaddr_in6 *addr = &ep->a.sin6;
    struct netsnmp_ep_str ep_str;
    char            debug_addr[INET6_ADDRSTRLEN];
    int             port;

    if (!ep)
        return 0;

    DEBUGMSGTL(("netsnmp_sockaddr_in6",
		"ep %p, peername \"%s\", default_target \"%s\"\n",
                ep, inpeername ? inpeername : "[NIL]",
		default_target ? default_target : "[NIL]"));

    memset(ep, 0, sizeof(*ep));
    addr->sin6_family = AF_INET6;
    addr->sin6_addr = in6addr_any;
    addr->sin6_port = htons(SNMP_PORT);

    memset(&ep_str, 0, sizeof(ep_str));
    port = netsnmp_ds_get_int(NETSNMP_DS_LIBRARY_ID,
                              NETSNMP_DS_LIB_DEFAULT_PORT);
    if (port != 0)
        snprintf(ep_str.port, sizeof(ep_str.port), "%d", port);
    else if (default_target &&
             !netsnmp_parse_ep_str(&ep_str, default_target))
            snmp_log(LOG_ERR, "Invalid default target %s\n",
                     default_target);

    if (!inpeername || !netsnmp_parse_ep_str(&ep_str, inpeername))
        return 0;

    if (ep_str.port[0])
        addr->sin6_port = htons(atoi(ep_str.port));
    if (ep_str.iface[0])
        strlcpy(ep->iface, ep_str.iface, sizeof(ep->iface));
    if (ep_str.addr[0]) {
        char *scope_id;

        scope_id = strchr(ep_str.addr, '%');
        if (scope_id) {
            *scope_id = '\0';
#if defined(HAVE_STRUCT_SOCKADDR_IN6_SIN6_SCOPE_ID)
            addr->sin6_scope_id = netsnmp_if_nametoindex(scope_id + 1);
#endif
        }
        if (!inet_pton(AF_INET6, ep_str.addr, &addr->sin6_addr) &&
            !netsnmp_resolve_v6_hostname(&addr->sin6_addr, ep_str.addr)) {
            DEBUGMSGTL(("netsnmp_sockaddr_in6", "failed to parse %s\n",
                        ep_str.addr));
            return 0;
        }
    }

    DEBUGMSGTL(("netsnmp_sockaddr_in6", "return { AF_INET6, [%s%%%d]:%hu }\n",
                inet_ntop(AF_INET6, &addr->sin6_addr, debug_addr,
                          sizeof(debug_addr)), (int)addr->sin6_scope_id,
                ntohs(addr->sin6_port)));
    return 1;
}


int
netsnmp_sockaddr_in6(struct sockaddr_in6 *addr,
                     const char *inpeername, int remote_port)
{
    char buf[sizeof(remote_port) * 3 + 2];
    sprintf(buf, ":%u", remote_port);
    return netsnmp_sockaddr_in6_2(addr, inpeername, remote_port ? buf : NULL);
}

#endif /* NETSNMP_ENABLE_IPV6 */
