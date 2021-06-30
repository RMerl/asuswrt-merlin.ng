/* IPV6 base transport support functions
 */
#ifndef SNMPIPV6BASE_H
#define SNMPIPV6BASE_H

#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

config_require(IPBase)

#include <net-snmp/library/snmp_transport.h>

#ifdef __cplusplus
extern          "C" {
#endif

/*
 * Prototypes
 */

    char *netsnmp_ipv6_fmtaddr(const char *prefix, netsnmp_transport *t,
                               const void *data, int len);
    void netsnmp_ipv6_get_taddr(struct netsnmp_transport_s *t, void **addr,
                                size_t *addr_len);
    int netsnmp_ipv6_ostring_to_sockaddr(struct sockaddr_in6 *sin6,
                                         const void *o, size_t o_len);
    int netsnmp_sockaddr_in6_2(struct sockaddr_in6 *addr,
                               const char *inpeername,
                               const char *default_target);
    int netsnmp_sockaddr_in6(struct sockaddr_in6 *addr,
                             const char *inpeername, int remote_port);
    int
    netsnmp_sockaddr_in6_3(struct netsnmp_ep *ep,
                           const char *inpeername, const char *default_target);

#ifdef __cplusplus
}
#endif
#endif /* SNMPIPV6BASE_H */

