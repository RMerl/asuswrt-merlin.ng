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

#include <netinet/if_ether.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/auto_nlist.h>
#include <net-snmp/data_access/interface.h>
#include "../at.h"

#if defined(_AIX)
#define ARPTAB_SIZE_SYMBOL "arptabsize"
#define ARPTAB_SYMBOL "arptabnb"
#endif
#if defined(hpux) && !defined(hpux11)
#define ARPTAB_SYMBOL "arphd"
#define ARPTAB_SIZE_SYMBOL "arptab_nb"
#endif
#if defined(solaris)
#define ARPTAB_SYMBOL "arptab_nb"
#define ARPTAB_SIZE_SYMBOL "arphd"
#endif

static int arptab_size, arptab_current;

#ifdef HAVE_STRUCT_ARPHD_AT_NEXT
static struct arphd *at;
#else
static struct arptab *at;
#endif

void
ARP_Scan_Init(void)
{
    if (!at) {
#ifdef ARPTAB_SIZE_SYMBOL
        auto_nlist(ARPTAB_SIZE_SYMBOL, (char *) &arptab_size,
                   sizeof arptab_size);
        at = malloc(arptab_size * sizeof(*at));
#else
        return;
#endif
    }
#ifdef HAVE_STRUCT_ARPHD_AT_NEXT
    auto_nlist(ARPTAB_SYMBOL, (char *) at,
               arptab_size * sizeof(struct arphd));
    at_ptr = at[0].at_next;
#else
    auto_nlist(ARPTAB_SYMBOL, (char *) at,
               arptab_size * sizeof(struct arptab));
#endif
    arptab_current = 0;
}

int
ARP_Scan_Next(in_addr_t * IPAddr, char *PhysAddr, int *PhysAddrLen,
              u_long * ifType, u_short * ifIndex)
{
    register struct arptab *atab;

    *ifIndex = 0;

    while (arptab_current < arptab_size) {
#ifdef HAVE_STRUCT_ARPHD_AT_NEXT
        /*
         * The arp table is an array of linked lists of arptab entries.
         * Unused slots have pointers back to the array entry itself
         */

        if (at_ptr == (auto_nlist_value(ARPTAB_SYMBOL) +
                       arptab_current * sizeof(struct arphd))) {
            /*
             * Usused
             */
            arptab_current++;
            at_ptr = at[arptab_current].at_next;
            continue;
        }

        if (!NETSNMP_KLOOKUP(at_ptr, (char *) &at_entry, sizeof(struct arptab))) {
            DEBUGMSGTL(("mibII/at:ARP_Scan_Next", "klookup failed\n"));
            break;
        }

        if (!NETSNMP_KLOOKUP(at_entry.at_ac, (char *) &at_com, sizeof(struct arpcom))) {
            DEBUGMSGTL(("mibII/at:ARP_Scan_Next", "klookup failed\n"));
            break;
        }

        at_ptr = at_entry.at_next;
        atab = &at_entry;
        *ifIndex = at_com.ac_if.if_index;       /* not strictly ARPHD */
#else                           /* HAVE_STRUCT_ARPHD_AT_NEXT */
        atab = &at[arptab_current++];
#endif                          /* HAVE_STRUCT_ARPHD_AT_NEXT */
        if (!(atab->at_flags & ATF_COM))
            continue;
        *ifType = (atab->at_flags & ATF_PERM) ? 4 : 3;
        *IPAddr = atab->at_iaddr.s_addr;
#if defined (sunV3) || defined(sparc) || defined(hpux)
        memcpy(PhysAddr, (char *) &atab->at_enaddr,
               sizeof(atab->at_enaddr));
        *PhysAddrLen = sizeof(atab->at_enaddr);
#endif
#if defined(mips) || defined(ibm032)
        memcpy(PhysAddr, (char *) atab->at_enaddr,
               sizeof(atab->at_enaddr));
        *PhysAddrLen = sizeof(atab->at_enaddr);
#endif
        return (1);
    }

    return 0;
}
