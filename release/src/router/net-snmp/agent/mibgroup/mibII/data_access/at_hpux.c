/*
 *  Template MIB group implementation - at.c
 *
 */

/* Portions of this file are subject to the following copyright(s).  See
 * the Net-SNMP's COPYING file for more details and other copyrights
 * that may apply:
 */
/*
 * Portions of this file are copyrighted by:
 * Copyright © 2003 Sun Microsystems, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/data_access/interface.h>
#include "../at.h"
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if defined(NETSNMP_IFNET_NEEDS_KERNEL) && !defined(_KERNEL)
#define _KERNEL 1
#define _I_DEFINED_KERNEL
#endif
#include <sys/types.h>
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if HAVE_NET_IF_H
#include <net/if.h>
#endif
#if HAVE_NET_IF_VAR_H
#include <net/if_var.h>
#endif
#ifdef _I_DEFINED_KERNEL
#undef _KERNEL
#endif

#if HAVE_NETINET_IF_ETHER_H
#include <netinet/if_ether.h>
#endif
#if HAVE_INET_MIB2_H
#include <inet/mib2.h>
#endif
#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#if HAVE_SYS_SYSCTL_H
#include <sys/sysctl.h>
#endif
#if HAVE_NET_IF_DL_H
#ifndef dynix
#include <net/if_dl.h>
#else
#include <sys/net/if_dl.h>
#endif
#endif
#if HAVE_SYS_STREAM_H
#include <sys/stream.h>
#endif
#if HAVE_NET_ROUTE_H
#include <net/route.h>
#endif
#ifdef solaris2
#include "kernel_sunos5.h"
#endif

#ifdef hpux11
#include <sys/mib.h>
#include <netinet/mib_kern.h>
#endif                          /* hpux11 */

static int arptab_size, arptab_current;
static mib_ipNetToMediaEnt *at;

void
ARP_Scan_Init(void)
{
    int             fd;
    struct nmparms  p;
    int             val;
    unsigned int    ulen;
    int             ret;

    if (at)
        free(at);
    at = (mib_ipNetToMediaEnt *) 0;
    arptab_size = 0;

    if ((fd = open_mib("/dev/ip", O_RDONLY, 0, NM_ASYNC_OFF)) >= 0) {
        p.objid = ID_ipNetToMediaTableNum;
        p.buffer = (void *) &val;
        ulen = sizeof(int);
        p.len = &ulen;
        if ((ret = get_mib_info(fd, &p)) == 0)
            arptab_size = val;

        if (arptab_size > 0) {
            ulen = (unsigned) arptab_size *sizeof(mib_ipNetToMediaEnt);
            at = (mib_ipNetToMediaEnt *) malloc(ulen);
            memset(at, 0, ulen);
            p.objid = ID_ipNetToMediaTable;
            p.buffer = (void *) at;
            p.len = &ulen;
            if ((ret = get_mib_info(fd, &p)) < 0)
                arptab_size = 0;
            else
                arptab_size = *p.len / sizeof(mib_ipNetToMediaEnt);
        }

        close_mib(fd);
    }

    arptab_current = 0;
}

int
ARP_Scan_Next(in_addr_t * IPAddr, char *PhysAddr, int *PhysAddrLen,
              u_long * ifType, u_short * ifIndex)
{
    *ifIndex = 0;

    if (arptab_current < arptab_size) {
        /*
         * copy values
         */
        *IPAddr = at[arptab_current].NetAddr;
        memcpy(PhysAddr, at[arptab_current].PhysAddr.o_bytes,
               at[arptab_current].PhysAddr.o_length);
        *ifType = at[arptab_current].Type;
        *ifIndex = at[arptab_current].IfIndex;
        *PhysAddrLen = at[arptab_current].PhysAddr.o_length;
        /*
         * increment to point next entry
         */
        arptab_current++;
        /*
         * return success
         */
        return (1);
    }
}
