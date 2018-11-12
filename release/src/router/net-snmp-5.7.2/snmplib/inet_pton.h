#ifndef _INET_PTON_H
#define _INET_PTON_H

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/types.h>

#ifndef HAVE_INET_PTON
int             netsnmp_inet_pton(int af, const char *src, void *dst);
#define inet_pton netsnmp_inet_pton
#endif /*HAVE_INET_PTON */

#endif /*_INET_PTON_H*/
