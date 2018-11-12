/* IPV4 base transport support functions
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

#include <net-snmp/types.h>
#include <net-snmp/library/snmpUDPIPv4BaseDomain.h>

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
#include <errno.h>

#include <net-snmp/types.h>
#include <net-snmp/library/snmp_debug.h>
#include <net-snmp/library/tools.h>
#include <net-snmp/library/snmp_assert.h>
#include <net-snmp/library/default_store.h>
#include <net-snmp/library/snmp_transport.h>

#include <net-snmp/library/snmpSocketBaseDomain.h>

#ifndef NETSNMP_NO_SYSTEMD
#include <net-snmp/library/sd-daemon.h>
#endif

#if defined(HAVE_IP_PKTINFO) || (defined(HAVE_IP_RECVDSTADDR) && defined(HAVE_IP_SENDSRCADDR))
int netsnmp_udpipv4_recvfrom(int s, void *buf, int len, struct sockaddr *from,
                             socklen_t *fromlen, struct sockaddr *dstip,
                             socklen_t *dstlen, int *if_index)
{
    return netsnmp_udpbase_recvfrom(s, buf, len, from, fromlen, dstip, dstlen,
                                    if_index);
}

int netsnmp_udpipv4_sendto(int fd, const struct in_addr *srcip, int if_index,
                           const struct sockaddr *remote, const void *data,
                           int len)
{
    return netsnmp_udpbase_sendto(fd, srcip, if_index, remote, data, len);
}
#endif /* HAVE_IP_PKTINFO || HAVE_IP_RECVDSTADDR */

netsnmp_transport *
netsnmp_udpipv4base_transport_init(const struct sockaddr_in *addr, int local)
{
    netsnmp_transport *t;
    u_char *addr_ptr;

    if (addr == NULL || addr->sin_family != AF_INET) {
        return NULL;
    }

    t = SNMP_MALLOC_TYPEDEF(netsnmp_transport);
    if (NULL == t)
        return NULL;

    t->sock = -1;

    addr_ptr = netsnmp_memdup(addr, sizeof(*addr));
    if (NULL == addr_ptr) {
        free(t);
        return NULL;
    }

    if (local) {
        /** This is a server session. */
        t->local_length = sizeof(*addr);
        t->local = addr_ptr;
    } else {
        /** This is a client session. */
        t->remote = addr_ptr;
        t->remote_length = sizeof(*addr);
    }

    DEBUGIF("netsnmp_udpbase") {
        netsnmp_indexed_addr_pair addr_pair;
        char                      *str;
        memset(&addr_pair, 0, sizeof(netsnmp_indexed_addr_pair));
        memcpy(&(addr_pair.remote_addr), addr, sizeof(*addr));
        str = netsnmp_udp_fmtaddr(NULL, (void *)&addr_pair,
                                  sizeof(netsnmp_indexed_addr_pair));
        DEBUGMSGTL(("netsnmp_udpbase", "open %s %s\n",
                    local ? "local" : "remote", str));
        free(str);
    }

    if (!local) {
        netsnmp_indexed_addr_pair *addr_pair;

        /*
         * allocate space to save the (remote) address in the
         * transport-specific data pointer for later use by netsnmp_udp_send.
         */
        t->data = calloc(1, sizeof(netsnmp_indexed_addr_pair));
        if (NULL == t->data) {
            netsnmp_transport_free(t);
            return NULL;
        }
        t->data_length = sizeof(netsnmp_indexed_addr_pair);

        addr_pair = (netsnmp_indexed_addr_pair *)t->data;
        memcpy(&addr_pair->remote_addr, addr, sizeof(*addr));
    }
    return t;
}

int
netsnmp_udpipv4base_transport_socket(int flags)
{
    int local = flags & NETSNMP_TSPEC_LOCAL;
    int sock = socket(PF_INET, SOCK_DGRAM, 0);

    DEBUGMSGTL(("UDPBase", "opened socket %d as local=%d\n", sock, local));
    if (sock < 0)
        return -1;

    _netsnmp_udp_sockopt_set(sock, local);

    return sock;
}

int
netsnmp_udpipv4base_transport_bind(netsnmp_transport *t,
                                   const struct sockaddr_in *addr, int flags)
{
#if defined(HAVE_IP_PKTINFO) || defined(HAVE_IP_RECVDSTADDR)
    int                sockopt = 1;
#endif
    int                rc;

    if (flags & NETSNMP_TSPEC_LOCAL) {
#ifdef NETSNMP_NO_LISTEN_SUPPORT
        return NULL;
#endif /* NETSNMP_NO_LISTEN_SUPPORT */
#ifndef WIN32
#if defined(HAVE_IP_PKTINFO)
        if (setsockopt(t->sock, SOL_IP, IP_PKTINFO, &sockopt, sizeof sockopt) == -1) {
            DEBUGMSGTL(("netsnmp_udpbase", "couldn't set IP_PKTINFO: %s\n",
                        strerror(errno)));
            return 1;
        }
        DEBUGMSGTL(("netsnmp_udpbase", "set IP_PKTINFO\n"));
#elif defined(HAVE_IP_RECVDSTADDR)
        if (setsockopt(t->sock, IPPROTO_IP, IP_RECVDSTADDR, &sockopt, sizeof sockopt) == -1) {
            DEBUGMSGTL(("netsnmp_udp", "couldn't set IP_RECVDSTADDR: %s\n",
                        strerror(errno)));
            return 1;
        }
        DEBUGMSGTL(("netsnmp_udp", "set IP_RECVDSTADDR\n"));
#endif
#else /* !defined(WIN32) */
        { 
            int sockopt = 1;
            if (setsockopt(t->sock, IPPROTO_IP, IP_PKTINFO, (void *)&sockopt,
			   sizeof(sockopt)) == -1) {
                DEBUGMSGTL(("netsnmp_udpbase", "couldn't set IP_PKTINFO: %d\n",
                            WSAGetLastError()));
            } else {
                DEBUGMSGTL(("netsnmp_udpbase", "set IP_PKTINFO\n"));
            }
        }
#endif /* !defined(WIN32) */
    }

    DEBUGIF("netsnmp_udpbase") {
        netsnmp_indexed_addr_pair addr_pair;
        char *str;
        memset(&addr_pair, 0x0, sizeof(addr_pair));
        memcpy(&(addr_pair.local_addr), addr, sizeof(*addr));
        str = netsnmp_udp_fmtaddr(NULL, (void *)&addr_pair,
                                  sizeof(netsnmp_indexed_addr_pair));
        DEBUGMSGTL(("netsnmp_udpbase", "binding socket: %d to %s\n",
                    t->sock, str));
        free(str);
    }
    rc = bind(t->sock, (const struct sockaddr *)addr, sizeof(*addr));
    if ( rc != 0 ) {
        DEBUGMSGTL(("netsnmp_udpbase",
                    "failed to bind for clientaddr: %d %s\n",
                    errno, strerror(errno)));
        netsnmp_socketbase_close(t);
        return 1;
    }

    return 0;
}

void
netsnmp_udpipv4base_transport_get_bound_addr(netsnmp_transport *t)
{
    netsnmp_indexed_addr_pair *addr_pair;
    socklen_t                  local_addr_len = sizeof(addr_pair->local_addr);
    int                        rc;

    /** only for client transports: must have data and not local */
    if (NULL == t || NULL != t->local || NULL == t->data ||
        t->data_length < local_addr_len) {
        snmp_log(LOG_ERR, "bad parameters for get bound addr\n");
        return;
    }

    addr_pair = (netsnmp_indexed_addr_pair *)t->data;

    /** get local socket address for client session */
    rc = getsockname(t->sock, (struct sockaddr*)&addr_pair->local_addr,
                     &local_addr_len);
    netsnmp_assert(rc == 0);
    DEBUGIF("netsnmp_udpbase") {
        char *str;
        str = netsnmp_udp_fmtaddr(NULL, (void *)addr_pair,
                                  sizeof(netsnmp_indexed_addr_pair));
        DEBUGMSGTL(("netsnmp_udpbase", "socket %d bound to %s\n",
                    t->sock, str));
        free(str);
    }
}

netsnmp_transport *
netsnmp_udpipv4base_transport_with_source(const struct sockaddr_in *addr,
                                          int local,
                                          const struct sockaddr_in *src_addr)
{
    netsnmp_transport         *t = NULL;
    const struct sockaddr_in  *bind_addr;
    int                        rc, flags = 0;

    t = netsnmp_udpipv4base_transport_init(addr, local);
    if (NULL == t)
         return NULL;

    if (local) {
        bind_addr = addr;
        flags |= NETSNMP_TSPEC_LOCAL;

#ifndef NETSNMP_NO_SYSTEMD
        /*
         * Maybe the socket was already provided by systemd...
         */
        t->sock = netsnmp_sd_find_inet_socket(PF_INET, SOCK_DGRAM, -1,
                                              ntohs(addr->sin_port));
#endif
    }
    else
        bind_addr = src_addr;

    if (-1 == t->sock)
        t->sock = netsnmp_udpipv4base_transport_socket(flags);
    if (t->sock < 0) {
        netsnmp_transport_free(t);
        return NULL;
    }

    /*
     * If we've been given an address to bind to, then bind to it.
     * Otherwise the OS will use "something sensible".
     */
    if (NULL == bind_addr)
        return t;

    rc = netsnmp_udpipv4base_transport_bind(t, bind_addr, flags);
    if (rc) {
        netsnmp_transport_free(t);
        t = NULL;
    }
    else if (!local)
        netsnmp_udpipv4base_transport_get_bound_addr(t);

    return t;
}

netsnmp_transport *
netsnmp_udpipv4base_tspec_transport(netsnmp_tdomain_spec *tspec)
{
    struct sockaddr_in addr;
    int local;

    if (NULL == tspec)
        return NULL;

    local = tspec->flags & NETSNMP_TSPEC_LOCAL;

    /** get address from target */
    if (!netsnmp_sockaddr_in2(&addr, tspec->target, tspec->default_target))
        return NULL;

    if (NULL != tspec->source) {
        struct sockaddr_in src_addr, *srcp = &src_addr;
        /** get sockaddr from source */
        if (!netsnmp_sockaddr_in2(&src_addr, tspec->source, NULL))
            return NULL;
        return netsnmp_udpipv4base_transport_with_source(&addr, local, srcp);
     } else {
        /** if no source and we do not want any default client address */
        if (tspec->flags & NETSNMP_TSPEC_NO_DFTL_CLIENT_ADDR)
            return netsnmp_udpipv4base_transport_with_source(&addr, local,
                                                             NULL);
    }

    /** no source and default client address ok */
    return netsnmp_udpipv4base_transport(&addr, local);
}

netsnmp_transport *
netsnmp_udpipv4base_transport(const struct sockaddr_in *addr, int local)
{
    if (!local) {
        /*
         * This is a client session.  If we've been given a
         * client address to send from, then bind to that.
         * Otherwise the send will use "something sensible".
         */
        const char *client_socket;
        client_socket = netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID,
                                              NETSNMP_DS_LIB_CLIENT_ADDR);
        if (client_socket) {
            struct sockaddr_in client_addr;
            char *client_address = NETSNMP_REMOVE_CONST(char *,client_socket);
            int have_port, rc;
            int uses_port =
                netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID,
                                       NETSNMP_DS_LIB_CLIENT_ADDR_USES_PORT);
            have_port = (strchr(client_socket, ':') != NULL);
            if ((uses_port == 1) && !have_port) {
                /*
                 * if NETSNMP_DS_LIB_CLIENT_ADDR expects a port but there
                 *  is none specified then provide ephemeral one/
                 */
                client_address = malloc(strlen(client_socket) + 3);
                if (client_address == NULL) {
                    return NULL;
                }
                strcpy(client_address, client_socket);
                strcat(client_address, ":0");
                have_port = 1;
            }
            rc = netsnmp_sockaddr_in2(&client_addr, client_socket, NULL);
            if (client_address != client_socket)
                free(client_address);
            if(rc) {
                if (!uses_port || !have_port) /* if port isn't from string, */
                    client_addr.sin_port = 0; /* ... clear it */
                return netsnmp_udpipv4base_transport_with_source(addr, local,
                                                                 &client_addr);
            }
        }
    }
    return netsnmp_udpipv4base_transport_with_source(addr, local, NULL);
}
