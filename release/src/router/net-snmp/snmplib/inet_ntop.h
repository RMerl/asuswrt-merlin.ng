#ifndef _INET_NTOP_H
#define _INET_NTOP_H

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/types.h>

#ifndef HAVE_INET_NTOP
const char     *netsnmp_inet_ntop(int af, const void *src, char *dst, size_t size);
#define inet_ntop netsnmp_inet_ntop
#endif /*HAVE_INET_NTOP */

#endif /*_INET_NTOP_H*/
