/* UDPIPV4 base transport support functions
 *
 * Portions of this file are copyrighted by:
 * Copyright (c) 2016 VMware, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */
#ifndef SNMPUDPIPV4BASE_H
#define SNMPUDPIPV4BASE_H

#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

config_require(UDPBase)
config_require(IPv4Base)

#include <net-snmp/library/snmpIPv4BaseDomain.h>
#include <net-snmp/library/snmpUDPBaseDomain.h>

#ifdef __cplusplus
extern          "C" {
#endif

/*
 * Prototypes
 */

    netsnmp_transport *netsnmp_udpipv4base_transport(const struct sockaddr_in *addr,
                                                     int local);

    netsnmp_transport *
    netsnmp_udpipv4base_transport_with_source(const struct sockaddr_in *addr,
                                              int local,
                                              const struct sockaddr_in *src_addr);

    netsnmp_transport *
    netsnmp_udpipv4base_tspec_transport(netsnmp_tdomain_spec *tspec);

    /** internal functions for derivatives of udpipv4base */
    netsnmp_transport *
    netsnmp_udpipv4base_transport_init(const struct sockaddr_in *addr,
                                       int local);

    int
    netsnmp_udpipv4base_transport_socket(int flags);

    int
    netsnmp_udpipv4base_transport_bind(netsnmp_transport *t,
                                       const struct sockaddr_in *addr,
                                       int flags);

    void
    netsnmp_udpipv4base_transport_get_bound_addr(netsnmp_transport *t);

#if defined(HAVE_IP_PKTINFO) || defined(HAVE_IP_RECVDSTADDR)
    int netsnmp_udpipv4_recvfrom(int s, void *buf, int len,
                                 struct sockaddr *from, socklen_t *fromlen,
                                 struct sockaddr *dstip, socklen_t *dstlen,
                                 int *if_index);
    int netsnmp_udpipv4_sendto(int fd, const struct in_addr *srcip,
                               int if_index, const struct sockaddr *remote,
                               const void *data, int len);
#endif


#ifdef __cplusplus
}
#endif
#endif /* SNMPUDPIPV4BASE_H */
