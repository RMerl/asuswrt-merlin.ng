/* UDPshared base transport support functions
 *
 * Portions of this file are copyrighted by:
 * Copyright (c) 2016 VMware, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */
#ifndef SNMPUDPsharedBASE_H
#define SNMPUDPsharedBASE_H

#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

config_require(UDP)

#include <net-snmp/library/snmpUDPBaseDomain.h>
#include <net-snmp/library/snmpUDPIPv4BaseDomain.h>

#ifdef __cplusplus
extern          "C" {
#endif

/*
 * Prototypes
 */

    /*
     * "Constructor" for transport domain object.
     */
    void            netsnmp_udpshared_ctor(void);

    netsnmp_transport *netsnmp_udpshared_transport(const struct sockaddr_in *addr,
                                                   int local);

    netsnmp_transport *
    netsnmp_udpshared_transport_with_source(const struct sockaddr_in *addr,
                                            int local,
                                            const struct sockaddr_in *src_addr);

#ifdef __cplusplus
}
#endif
#endif /* SNMPUDPsharedBASE_H */
