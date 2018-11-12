/* UDP base transport support functions
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
#include <net-snmp/library/snmpUDPBaseDomain.h>

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
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#if HAVE_NETDB_H
#include <netdb.h>
#endif
#if HAVE_SYS_UIO_H
#include <sys/uio.h>
#endif
#ifdef WIN32
#include <mswsock.h>
#endif
#include <errno.h>

#include <net-snmp/types.h>
#include <net-snmp/library/snmpSocketBaseDomain.h>
#include <net-snmp/library/snmpUDPDomain.h>
#include <net-snmp/library/snmp_debug.h>
#include <net-snmp/library/tools.h>
#include <net-snmp/library/default_store.h>
#include <net-snmp/library/system.h>
#include <net-snmp/library/snmp_assert.h>

#ifndef  MSG_DONTWAIT
#define MSG_DONTWAIT 0
#endif

#ifndef  MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

void
_netsnmp_udp_sockopt_set(int fd, int local)
{
#ifdef  SO_BSDCOMPAT
    /*
     * Patch for Linux.  Without this, UDP packets that fail get an ICMP
     * response.  Linux turns the failed ICMP response into an error message
     * and return value, unlike all other OS's.  
     */
    if (0 == netsnmp_os_prematch("Linux","2.4"))
    {
        int             one = 1;
        DEBUGMSGTL(("socket:option", "setting socket option SO_BSDCOMPAT\n"));
        setsockopt(fd, SOL_SOCKET, SO_BSDCOMPAT, (void *) &one,
                   sizeof(one));
    }
#endif                          /*SO_BSDCOMPAT */
    /*
     * SO_REUSEADDR will allow multiple apps to open the same port at
     * the same time. Only the last one to open the socket will get
     * data. Obviously, for an agent, this is a bad thing. There should
     * only be one listener.
     */
#ifdef ALLOW_PORT_HIJACKING
#ifdef  SO_REUSEADDR
    /*
     * Allow the same port to be specified multiple times without failing.
     *    (useful for a listener)
     */
    {
        int             one = 1;
        DEBUGMSGTL(("socket:option", "setting socket option SO_REUSEADDR\n"));
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void *) &one,
                   sizeof(one));
    }
#endif                          /*SO_REUSEADDR */
#endif

    /*
     * Try to set the send and receive buffers to a reasonably large value, so
     * that we can send and receive big PDUs (defaults to 8192 bytes (!) on
     * Solaris, for instance).  Don't worry too much about errors -- just
     * plough on regardless.  
     */
    netsnmp_sock_buffer_set(fd, SO_SNDBUF, local, 0);
    netsnmp_sock_buffer_set(fd, SO_RCVBUF, local, 0);
}

#if defined(HAVE_IP_PKTINFO) || (defined(HAVE_IP_RECVDSTADDR) && defined(HAVE_IP_SENDSRCADDR))

#define netsnmp_udpbase_recvfrom_sendto_defined

enum {
#if defined(HAVE_IP_PKTINFO)
    cmsg_data_size = sizeof(struct in_pktinfo)
#elif defined(HAVE_IP_RECVDSTADDR)
    cmsg_data_size = sizeof(struct in_addr)
#endif
};

#ifdef WIN32
#ifndef WSAID_WSASENDMSG
#define WSAID_WSASENDMSG \
    {0xa441e712,0x754f,0x43ca,{0x84,0xa7,0x0d,0xee,0x44,0xcf,0x60,0x6d}}
typedef INT (WINAPI * LPFN_WSASENDMSG)(SOCKET, LPWSAMSG, DWORD, LPDWORD,
                                       LPWSAOVERLAPPED,
                                       LPWSAOVERLAPPED_COMPLETION_ROUTINE);
#endif

static LPFN_WSARECVMSG pfWSARecvMsg;
static LPFN_WSASENDMSG pfWSASendMsg;
#endif

int
netsnmp_udpbase_recvfrom(int s, void *buf, int len, struct sockaddr *from,
                         socklen_t *fromlen, struct sockaddr *dstip,
                         socklen_t *dstlen, int *if_index)
{
    int r;
#if !defined(WIN32)
    struct iovec iov;
    char cmsg[CMSG_SPACE(cmsg_data_size)];
    struct cmsghdr *cm;
    struct msghdr msg;

    iov.iov_base = buf;
    iov.iov_len = len;

    memset(&msg, 0, sizeof msg);
    msg.msg_name = from;
    msg.msg_namelen = *fromlen;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = &cmsg;
    msg.msg_controllen = sizeof(cmsg);

    r = recvmsg(s, &msg, MSG_DONTWAIT);
#else /* !defined(WIN32) */
    WSABUF wsabuf;
    char cmsg[WSA_CMSG_SPACE(sizeof(struct in_pktinfo))];
    WSACMSGHDR *cm;
    WSAMSG msg;
    DWORD bytes_received;

    wsabuf.buf = buf;
    wsabuf.len = len;

    msg.name = from;
    msg.namelen = *fromlen;
    msg.lpBuffers = &wsabuf;
    msg.dwBufferCount = 1;
    msg.Control.len = sizeof(cmsg);
    msg.Control.buf = cmsg;
    msg.dwFlags = 0;

    if (pfWSARecvMsg) {
        r = pfWSARecvMsg(s, &msg, &bytes_received, NULL, NULL) == 0 ?
            bytes_received : -1;
        *fromlen = msg.namelen;
    } else {
        r = recvfrom(s, buf, len, MSG_DONTWAIT, from, fromlen);
    }
#endif /* !defined(WIN32) */

    if (r == -1) {
        return -1;
    }

    DEBUGMSGTL(("udpbase:recv", "got source addr: %s\n",
                inet_ntoa(((struct sockaddr_in *)from)->sin_addr)));

    {
        /* Get the local port number for use in diagnostic messages */
        int r2 = getsockname(s, dstip, dstlen);
        netsnmp_assert(r2 == 0);
    }

#if !defined(WIN32)
    for (cm = CMSG_FIRSTHDR(&msg); cm != NULL; cm = CMSG_NXTHDR(&msg, cm)) {
#if defined(HAVE_IP_PKTINFO)
        if (cm->cmsg_level == SOL_IP && cm->cmsg_type == IP_PKTINFO) {
            struct in_pktinfo* src = (struct in_pktinfo *)CMSG_DATA(cm);
            netsnmp_assert(dstip->sa_family == AF_INET);
            ((struct sockaddr_in*)dstip)->sin_addr = src->ipi_addr;
            *if_index = src->ipi_ifindex;
            DEBUGMSGTL(("udpbase:recv",
                        "got destination (local) addr %s, iface %d\n",
                        inet_ntoa(src->ipi_addr), *if_index));
        }
#elif defined(HAVE_IP_RECVDSTADDR)
        if (cm->cmsg_level == IPPROTO_IP && cm->cmsg_type == IP_RECVDSTADDR) {
            struct in_addr* src = (struct in_addr *)CMSG_DATA(cm);
            ((struct sockaddr_in*)dstip)->sin_addr = *src;
            DEBUGMSGTL(("netsnmp_udp", "got destination (local) addr %s\n",
                        inet_ntoa(*src)));
        }
#endif
    }
#else /* !defined(WIN32) */
    for (cm = WSA_CMSG_FIRSTHDR(&msg); cm; cm = WSA_CMSG_NXTHDR(&msg, cm)) {
        if (cm->cmsg_level == IPPROTO_IP && cm->cmsg_type == IP_PKTINFO) {
            struct in_pktinfo* src = (struct in_pktinfo *)WSA_CMSG_DATA(cm);
            netsnmp_assert(dstip->sa_family == AF_INET);
            ((struct sockaddr_in*)dstip)->sin_addr = src->ipi_addr;
            *if_index = src->ipi_ifindex;
            DEBUGMSGTL(("udpbase:recv",
                        "got destination (local) addr %s, iface %d\n",
                        inet_ntoa(src->ipi_addr), *if_index));
        }
    }
#endif /* !defined(WIN32) */
    return r;
}

int netsnmp_udpbase_sendto(int fd, const struct in_addr *srcip, int if_index,
                           const struct sockaddr *remote, const void *data,
                           int len)
{
#if !defined(WIN32)
    struct iovec iov;
    struct msghdr m = { NULL };
    char          cmsg[CMSG_SPACE(cmsg_data_size)];
    int           rc;

    iov.iov_base = NETSNMP_REMOVE_CONST(void *, data);
    iov.iov_len  = len;

    m.msg_name		= NETSNMP_REMOVE_CONST(void *, remote);
    m.msg_namelen	= sizeof(struct sockaddr_in);
    m.msg_iov		= &iov;
    m.msg_iovlen	= 1;
    m.msg_flags		= 0;

    if (srcip && srcip->s_addr != INADDR_ANY) {
        struct cmsghdr *cm;

        DEBUGMSGTL(("udpbase:sendto", "sending from %s\n", inet_ntoa(*srcip)));

        memset(cmsg, 0, sizeof(cmsg));

        m.msg_control    = &cmsg;
        m.msg_controllen = sizeof(cmsg);

        cm = CMSG_FIRSTHDR(&m);
        cm->cmsg_len = CMSG_LEN(cmsg_data_size);

#if defined(HAVE_IP_PKTINFO)
        cm->cmsg_level = SOL_IP;
        cm->cmsg_type = IP_PKTINFO;

        {
            struct in_pktinfo ipi;

            memset(&ipi, 0, sizeof(ipi));
            /*
             * Except in the case of responding
             * to a broadcast, setting the ifindex
             * when responding results in incorrect
             * behavior of changing the source address
             * that the manager sees the response
             * come from.
             */
            ipi.ipi_ifindex = 0;
#if defined(cygwin)
            ipi.ipi_addr.s_addr = srcip->s_addr;
#else
            ipi.ipi_spec_dst.s_addr = srcip->s_addr;
#endif
            memcpy(CMSG_DATA(cm), &ipi, sizeof(ipi));
        }

        rc = sendmsg(fd, &m, MSG_NOSIGNAL|MSG_DONTWAIT);
        if (rc >= 0 || errno != EINVAL)
            return rc;

        /*
         * The error might be caused by broadcast srcip (i.e. we're responding
         * to a broadcast request) - sendmsg does not like it. Try to resend it
         * using the interface on which it was received
         */

        DEBUGMSGTL(("udpbase:sendto", "re-sending on iface %d\n", if_index));

        {
            struct in_pktinfo ipi;

            memset(&ipi, 0, sizeof(ipi));
            ipi.ipi_ifindex = if_index;
#if defined(cygwin)
            ipi.ipi_addr.s_addr = INADDR_ANY;
#else
            ipi.ipi_spec_dst.s_addr = INADDR_ANY;
#endif
            memcpy(CMSG_DATA(cm), &ipi, sizeof(ipi));
        }
#elif defined(HAVE_IP_SENDSRCADDR)
        cm->cmsg_level = IPPROTO_IP;
        cm->cmsg_type = IP_SENDSRCADDR;
        memcpy((struct in_addr *)CMSG_DATA(cm), srcip, sizeof(struct in_addr));
#endif
        rc = sendmsg(fd, &m, MSG_NOSIGNAL|MSG_DONTWAIT);
        if (rc >= 0 || errno != EINVAL)
            return rc;

        DEBUGMSGTL(("udpbase:sendto", "re-sending without source address\n"));
        m.msg_control = NULL;
        m.msg_controllen = 0;
    }

    return sendmsg(fd, &m, MSG_NOSIGNAL|MSG_DONTWAIT);
#else /* !defined(WIN32) */
    WSABUF        wsabuf;
    WSAMSG        m;
    char          cmsg[WSA_CMSG_SPACE(sizeof(struct in_pktinfo))];
    DWORD         bytes_sent;
    int           rc;

    wsabuf.buf = NETSNMP_REMOVE_CONST(void *, data);
    wsabuf.len = len;

    memset(&m, 0, sizeof(m));
    m.name          = NETSNMP_REMOVE_CONST(struct sockaddr *, remote);
    m.namelen       = sizeof(struct sockaddr_in);
    m.lpBuffers     = &wsabuf;
    m.dwBufferCount = 1;

    if (pfWSASendMsg && srcip && srcip->s_addr != INADDR_ANY) {
        WSACMSGHDR *cm;

        DEBUGMSGTL(("udpbase:sendto", "sending from [%d] %s\n", if_index,
                    inet_ntoa(*srcip)));

        memset(cmsg, 0, sizeof(cmsg));

        m.Control.buf = cmsg;
        m.Control.len = sizeof(cmsg);

        cm = WSA_CMSG_FIRSTHDR(&m);
        cm->cmsg_len = WSA_CMSG_LEN(cmsg_data_size);
        cm->cmsg_level = IPPROTO_IP;
        cm->cmsg_type = IP_PKTINFO;

        {
            struct in_pktinfo ipi = { 0 };
            ipi.ipi_ifindex = if_index;
            ipi.ipi_addr.s_addr = srcip->s_addr;
            memcpy(WSA_CMSG_DATA(cm), &ipi, sizeof(ipi));
        }

        rc = pfWSASendMsg(fd, &m, 0, &bytes_sent, NULL, NULL);
        if (rc == 0)
            return bytes_sent;
        DEBUGMSGTL(("udpbase:sendto", "sending from [%d] %s failed: %d\n",
                    if_index, inet_ntoa(*srcip), WSAGetLastError()));
    }
    rc = sendto(fd, data, len, 0, remote, sizeof(struct sockaddr));
    return rc;
#endif /* !defined(WIN32) */
}
#endif /* HAVE_IP_PKTINFO || HAVE_IP_RECVDSTADDR */

/*
 * You can write something into opaque that will subsequently get passed back 
 * to your send function if you like.  For instance, you might want to
 * remember where a PDU came from, so that you can send a reply there...  
 */

int
netsnmp_udpbase_recv(netsnmp_transport *t, void *buf, int size,
                     void **opaque, int *olength)
{
    int             rc = -1;
    socklen_t       fromlen = sizeof(netsnmp_sockaddr_storage);
    netsnmp_indexed_addr_pair *addr_pair = NULL;
    struct sockaddr *from;

    if (t != NULL && t->sock >= 0) {
        addr_pair = SNMP_MALLOC_TYPEDEF(netsnmp_indexed_addr_pair);
        if (addr_pair == NULL) {
            *opaque = NULL;
            *olength = 0;
            return -1;
        } else
            from = &addr_pair->remote_addr.sa;

	while (rc < 0) {
#ifdef netsnmp_udpbase_recvfrom_sendto_defined
            socklen_t local_addr_len = sizeof(addr_pair->local_addr);
            rc = netsnmp_udp_recvfrom(t->sock, buf, size, from, &fromlen,
                                      &addr_pair->local_addr.sa,
                                      &local_addr_len, &(addr_pair->if_index));
#else
            rc = recvfrom(t->sock, buf, size, MSG_DONTWAIT, from, &fromlen);
#endif /* netsnmp_udpbase_recvfrom_sendto_defined */
	    if (rc < 0 && errno != EINTR) {
		break;
	    }
	}

        if (rc >= 0) {
            DEBUGIF("netsnmp_udp") {
                char *str = netsnmp_udp_fmtaddr(
                    NULL, addr_pair, sizeof(netsnmp_indexed_addr_pair));
                DEBUGMSGTL(("netsnmp_udp",
                            "recvfrom fd %d got %d bytes (from %s)\n",
                            t->sock, rc, str));
                free(str);
            }
        } else {
            DEBUGMSGTL(("netsnmp_udp", "recvfrom fd %d err %d (\"%s\")\n",
                        t->sock, errno, strerror(errno)));
        }
        *opaque = (void *)addr_pair;
        *olength = sizeof(netsnmp_indexed_addr_pair);
    }
    return rc;
}



int
netsnmp_udpbase_send(netsnmp_transport *t, const void *buf, int size,
                     void **opaque, int *olength)
{
    int rc = -1;
    const netsnmp_indexed_addr_pair *addr_pair = NULL;
    const struct sockaddr *to = NULL;

    if (opaque != NULL && *opaque != NULL && NULL != olength &&
        ((*olength == sizeof(netsnmp_indexed_addr_pair) ||
          (*olength == sizeof(struct sockaddr_in))))) {
        addr_pair = (const netsnmp_indexed_addr_pair *) (*opaque);
    } else if (t != NULL && t->data != NULL &&
                t->data_length == sizeof(netsnmp_indexed_addr_pair)) {
        addr_pair = (netsnmp_indexed_addr_pair *) (t->data);
    } else {
        int len = -1;
        if (opaque != NULL && *opaque != NULL && NULL != olength)
            len = *olength;
        else if (t != NULL && t->data != NULL)
            len = t->data_length;
        snmp_log(LOG_ERR, "unknown addr type of size %d\n", len);
        return SNMPERR_GENERR;
    }

    to = &addr_pair->remote_addr.sa;

    if (to != NULL && t != NULL && t->sock >= 0) {
        DEBUGIF("netsnmp_udp") {
            char *str = netsnmp_udp_fmtaddr(NULL, addr_pair,
                                            sizeof(netsnmp_indexed_addr_pair));
            DEBUGMSGTL(("netsnmp_udp", "send %d bytes from %p to %s on fd %d\n",
                        size, buf, str, t->sock));
            free(str);
        }
	while (rc < 0) {
#ifdef netsnmp_udpbase_recvfrom_sendto_defined
            rc = netsnmp_udp_sendto(t->sock,
                    addr_pair ? &(addr_pair->local_addr.sin.sin_addr) : NULL,
                    addr_pair ? addr_pair->if_index : 0, to, buf, size);
#else
            rc = sendto(t->sock, buf, size, 0, to, sizeof(struct sockaddr));
#endif /* netsnmp_udpbase_recvfrom_sendto_defined */
	    if (rc < 0 && errno != EINTR) {
                DEBUGMSGTL(("netsnmp_udp", "sendto error, rc %d (errno %d)\n",
                            rc, errno));
		break;
	    }
	}
    }
    return rc;
}

void
netsnmp_udp_base_ctor(void)
{
#if defined(WIN32) && defined(HAVE_IP_PKTINFO)
    SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
    GUID WSARecvMsgGuid = WSAID_WSARECVMSG;
    GUID WSASendMsgGuid = WSAID_WSASENDMSG;
    DWORD nbytes;
    int result;

    netsnmp_assert(s != SOCKET_ERROR);
    /* WSARecvMsg(): Windows XP / Windows Server 2003 and later */
    result = WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER,
                      &WSARecvMsgGuid, sizeof(WSARecvMsgGuid),
                      &pfWSARecvMsg, sizeof(pfWSARecvMsg), &nbytes, NULL, NULL);
    if (result == SOCKET_ERROR)
        DEBUGMSGTL(("netsnmp_udp", "WSARecvMsg() not found (errno %ld)\n",
                    WSAGetLastError()));

    /* WSASendMsg(): Windows Vista / Windows Server 2008 and later */
    result = WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER,
                      &WSASendMsgGuid, sizeof(WSASendMsgGuid),
                      &pfWSASendMsg, sizeof(pfWSASendMsg), &nbytes, NULL, NULL);
    if (result == SOCKET_ERROR)
        DEBUGMSGTL(("netsnmp_udp", "WSASendMsg() not found (errno %ld)\n",
                    WSAGetLastError()));

    closesocket(s);
#endif
}
