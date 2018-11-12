#ifndef _SNMPUDPDOMAIN_H
#define _SNMPUDPDOMAIN_H

/*
 * Portions of this file are copyrighted by:
 * Copyright (c) 2016 VMware, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */
#ifdef __cplusplus
extern          "C" {
#endif

#include <net-snmp/library/snmp_transport.h>
#include <net-snmp/library/asn1.h>

#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

config_require(UDPIPv4Base)
#include <net-snmp/library/snmpUDPIPv4BaseDomain.h>

NETSNMP_IMPORT
netsnmp_transport *netsnmp_udp_transport(const struct sockaddr_in *addr,
                                         int local);

NETSNMP_IMPORT
netsnmp_transport *netsnmp_udp_create_tspec(netsnmp_tdomain_spec *tspec);

NETSNMP_IMPORT
netsnmp_transport *
netsnmp_udp_transport_with_source(const struct sockaddr_in *addr, int local,
                                  const struct sockaddr_in *src_addr);

#define C2SE_ERR_SUCCESS             0
#define C2SE_ERR_MISSING_ARG        -1
#define C2SE_ERR_COMMUNITY_TOO_LONG -2
#define C2SE_ERR_SECNAME_TOO_LONG   -3
#define C2SE_ERR_CONTEXT_TOO_LONG   -4
#define C2SE_ERR_MASK_MISMATCH      -5
#define C2SE_ERR_MEMORY             -6

typedef struct com2SecEntry_s com2SecEntry;

NETSNMP_IMPORT
int         netsnmp_udp_com2SecEntry_create(com2SecEntry **entryp,
                                            const char *community,
                                            const char *secName,
                                            const char *contextName,
                                            struct in_addr *network,
                                            struct in_addr *mask,
                                            int negate);
NETSNMP_IMPORT
void        netsnmp_udp_com2Sec_free(com2SecEntry *e);

NETSNMP_IMPORT
int         netsnmp_udp_com2SecList_remove(com2SecEntry *e);

/*
 * Register any configuration tokens specific to the agent.  
 */

NETSNMP_IMPORT
void            netsnmp_udp_agent_config_tokens_register(void);

NETSNMP_IMPORT
void            netsnmp_udp_parse_security(const char *token, char *param);

NETSNMP_IMPORT
int             netsnmp_udp_getSecName(void *opaque, int olength,
                                       const char *community,
                                       size_t community_len,
                                       const char **secname,
                                       const char **contextName);

/*
 * "Constructor" for transport domain object.  
 */

void            netsnmp_udp_ctor(void);

/*
 * protected-ish functions used by other core-code
 */
char *netsnmp_udp_fmtaddr(netsnmp_transport *t, const void *data, int len);
#if defined(HAVE_IP_PKTINFO) || defined(HAVE_IP_RECVDSTADDR)
int netsnmp_udp_recvfrom(int s, void *buf, int len, struct sockaddr *from,
                         socklen_t *fromlen, struct sockaddr *dstip,
                         socklen_t *dstlen, int *if_index);
int netsnmp_udp_sendto(int fd, const struct in_addr *srcip, int if_index,
                       const struct sockaddr *remote, const void *data,
                       int len);
#endif

#ifdef __cplusplus
}
#endif
#endif/*_SNMPUDPDOMAIN_H*/
