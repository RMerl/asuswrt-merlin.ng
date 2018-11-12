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
#if HAVE_NET_IF_H
#include <net/if.h>
#endif
#if HAVE_NET_IF_DL_H
#include <net/if_dl.h>
#endif
#if HAVE_NETINET_IF_ETHER_H
#include <netinet/if_ether.h>
#endif
#if HAVE_NET_ROUTE_H
#include <net/route.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if HAVE_SYS_SYSCTL_H
#include <sys/sysctl.h>
#endif

static char    *lim, *rtnext;
static char    *at;

void
ARP_Scan_Init(void)
{
    int             mib[6];
    size_t          needed;

    mib[0] = CTL_NET;
    mib[1] = PF_ROUTE;
    mib[2] = 0;
    mib[3] = AF_INET;
    mib[4] = NET_RT_FLAGS;
#if defined RTF_LLINFO
    mib[5] = RTF_LLINFO;
#elif defined(RTF_LLDATA)
    mib[5] = RTF_LLDATA;
#else
    mib[5] = 0;
#endif

    if (at)
        free(at);
    rtnext = lim = at = 0;

    if (sysctl(mib, 6, NULL, &needed, NULL, 0) < 0)
        snmp_log_perror("route-sysctl-estimate");
    else {
        if ((at = malloc(needed ? needed : 1)) == NULL)
            snmp_log_perror("malloc");
        else {
            if (sysctl(mib, 6, at, &needed, NULL, 0) < 0)
                snmp_log_perror("actual retrieval of routing table");
            else {
                lim = at + needed;
                rtnext = at;
            }
        }
    }
}

int
ARP_Scan_Next(in_addr_t * IPAddr, char *PhysAddr, int *PhysAddrLen,
              u_long * ifType, u_short * ifIndex)
{
    struct rt_msghdr *rtm;
    struct sockaddr_inarp *sin;
    struct sockaddr_dl *sdl;

    while (rtnext < lim) {
        rtm = (struct rt_msghdr *) rtnext;
        sin = (struct sockaddr_inarp *) (rtm + 1);
        sdl = (struct sockaddr_dl *) (sin + 1);
        rtnext += rtm->rtm_msglen;
        if (sdl->sdl_alen) {
#ifdef irix6
            *IPAddr = sin->sarp_addr.s_addr;
#else
            *IPAddr = sin->sin_addr.s_addr;
#endif
            memcpy(PhysAddr, (char *) LLADDR(sdl), sdl->sdl_alen);
            *PhysAddrLen = sdl->sdl_alen;
            *ifIndex = sdl->sdl_index;
            *ifType = 1;        /* XXX */
            return 1;
        }
    }
    return 0;                 /* "EOF" */
}
