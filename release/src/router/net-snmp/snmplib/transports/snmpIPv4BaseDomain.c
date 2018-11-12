/* IPV4 base transport support functions
 */

#include <net-snmp/net-snmp-config.h>

#include <net-snmp/types.h>
#include <net-snmp/library/snmpIPv4BaseDomain.h>
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

#include <net-snmp/types.h>
#include <net-snmp/library/snmp_debug.h>
#include <net-snmp/library/default_store.h>
#include <net-snmp/library/system.h>

#include "inet_ntop.h"

#ifndef INADDR_NONE
#define INADDR_NONE     -1
#endif

int
netsnmp_sockaddr_in(struct sockaddr_in *addr,
                    const char *inpeername, int remote_port)
{
    char buf[sizeof(int) * 3 + 2];
    sprintf(buf, ":%u", remote_port);
    return netsnmp_sockaddr_in2(addr, inpeername, remote_port ? buf : NULL);
}

int
netsnmp_sockaddr_in2(struct sockaddr_in *addr,
                     const char *inpeername, const char *default_target)
{
    int ret;

    if (addr == NULL) {
        return 0;
    }

    DEBUGMSGTL(("netsnmp_sockaddr_in",
                "addr %p, inpeername \"%s\", default_target \"%s\"\n",
                addr, inpeername ? inpeername : "[NIL]",
                default_target ? default_target : "[NIL]"));

    memset(addr, 0, sizeof(struct sockaddr_in));
    addr->sin_addr.s_addr = htonl(INADDR_ANY);
    addr->sin_family = AF_INET;
    addr->sin_port = htons((u_short)SNMP_PORT);

    {
	int port = netsnmp_ds_get_int(NETSNMP_DS_LIBRARY_ID,
				      NETSNMP_DS_LIB_DEFAULT_PORT);

	if (port != 0) {
	    addr->sin_port = htons((u_short)port);
	} else if (default_target != NULL)
	    netsnmp_sockaddr_in2(addr, default_target, NULL);
    }

    if (inpeername != NULL && *inpeername != '\0') {
	const char     *host, *port;
	char           *peername = NULL;
        char           *cp;
        /*
         * Duplicate the peername because we might want to mank around with
         * it.
         */

        peername = strdup(inpeername);
        if (peername == NULL) {
            return 0;
        }

        /*
         * Try and extract an appended port number.
         */
        cp = strchr(peername, ':');
        if (cp != NULL) {
            *cp = '\0';
            port = cp + 1;
            host = peername;
        } else {
            host = NULL;
            port = peername;
        }

        /*
         * Try to convert the user port specifier
         */
        if (port && *port == '\0')
            port = NULL;

        if (port != NULL) {
            long int l;
            char* ep;

            DEBUGMSGTL(("netsnmp_sockaddr_in", "check user service %s\n",
                        port));

            l = strtol(port, &ep, 10);
            if (ep != port && *ep == '\0' && 0 <= l && l <= 0x0ffff)
                addr->sin_port = htons((u_short)l);
            else {
                if (host == NULL) {
                    DEBUGMSGTL(("netsnmp_sockaddr_in",
                                "servname not numeric, "
				"check if it really is a destination)\n"));
                    host = port;
                    port = NULL;
                } else {
                    DEBUGMSGTL(("netsnmp_sockaddr_in",
                                "servname not numeric\n"));
                    free(peername);
                    return 0;
                }
            }
        }

        /*
         * Try to convert the user host specifier
         */
        if (host && *host == '\0')
            host = NULL;

        if (host != NULL) {
            DEBUGMSGTL(("netsnmp_sockaddr_in",
                        "check destination %s\n", host));


            if (strcmp(peername, "255.255.255.255") == 0 ) {
                /*
                 * The explicit broadcast address hack
                 */
                DEBUGMSGTL(("netsnmp_sockaddr_in", "Explicit UDP broadcast\n"));
                addr->sin_addr.s_addr = INADDR_NONE;
            } else {
                ret =
                    netsnmp_gethostbyname_v4(peername, &addr->sin_addr.s_addr);
                if (ret < 0) {
                    DEBUGMSGTL(("netsnmp_sockaddr_in",
                                "couldn't resolve hostname\n"));
                    free(peername);
                    return 0;
                }
                DEBUGMSGTL(("netsnmp_sockaddr_in",
                            "hostname (resolved okay)\n"));
            }
        }
	free(peername);
    }

    /*
     * Finished
     */

    DEBUGMSGTL(("netsnmp_sockaddr_in", "return { AF_INET, %s:%hu }\n",
                inet_ntoa(addr->sin_addr), ntohs(addr->sin_port)));
    return 1;
}

char *
netsnmp_ipv4_fmtaddr(const char *prefix, netsnmp_transport *t,
                     const void *data, int len)
{
    const netsnmp_indexed_addr_pair *addr_pair;
    const struct sockaddr_in *from, *to;
    struct hostent *host;
    char *tmp;

    if (t && !data) {
        data = t->data;
        len = t->data_length;
    }

    switch (data ? len : 0) {
    case sizeof(netsnmp_indexed_addr_pair):
        addr_pair = data;
        break;
    case sizeof(struct sockaddr_in): {
        char a[16];

        to = data;
        if (asprintf(&tmp, "%s: [%s]:%hu", prefix,
		     inet_ntop(AF_INET, &to->sin_addr, a, sizeof(a)),
		     ntohs(to->sin_port)) < 0)
            tmp = NULL;
        return tmp;
    }
    default:
        netsnmp_assert(0);
        if (asprintf(&tmp, "%s: unknown", prefix) < 0)
            tmp = NULL;
        return tmp;
    }

    from = (const struct sockaddr_in *)&addr_pair->local_addr;
    to = (const struct sockaddr_in *)&addr_pair->remote_addr;
    netsnmp_assert(from->sin_family == 0 || from->sin_family == AF_INET);
    netsnmp_assert(to->sin_family == 0 || to->sin_family == AF_INET);
    if (t && t->flags & NETSNMP_TRANSPORT_FLAG_HOSTNAME) {
        /* XXX: hmm...  why isn't this prefixed */
        /* assuming intentional */
        host = netsnmp_gethostbyaddr(&to->sin_addr, sizeof(struct in_addr), AF_INET);
        return (host ? strdup(host->h_name) : NULL); 
    } else {
        char a1[16];
        char a2[16];

        if (asprintf(&tmp, "%s: [%s]:%hu->[%s]:%hu", prefix,
		     inet_ntop(AF_INET, &to->sin_addr, a1, sizeof(a1)),
		     ntohs(to->sin_port),
		     inet_ntop(AF_INET, &from->sin_addr, a2, sizeof(a2)),
		     ntohs(from->sin_port)) < 0)
            tmp = NULL;
    }

    return tmp;
}

void netsnmp_ipv4_get_taddr(struct netsnmp_transport_s *t, void **addr,
                            size_t *addr_len)
{
    struct sockaddr_in *sin = t->remote;

    netsnmp_assert(t->remote_length == sizeof(*sin));

    *addr_len = 6;
    if ((*addr = malloc(*addr_len))) {
        unsigned char *p = *addr;

        memcpy(p,     &sin->sin_addr, 4);
        memcpy(p + 4, &sin->sin_port, 2);
    }
}

int netsnmp_ipv4_ostring_to_sockaddr(struct sockaddr_in *sin, const void *o,
                                     size_t o_len)
{
    const char *p = o;

    if (o_len != 6)
        return 0;

    memset(sin, 0, sizeof(*sin));
    sin->sin_family = AF_INET;
    memcpy(&sin->sin_addr, p + 0, 4);
    memcpy(&sin->sin_port, p + 4, 2);
    return 1;
}
