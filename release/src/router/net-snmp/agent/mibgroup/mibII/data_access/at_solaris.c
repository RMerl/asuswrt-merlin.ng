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
#include "kernel_sunos5.h"
#include "../at.h"

typedef struct if_ip {
    int             ifIdx;
    IpAddress       ipAddr;
} if_ip_t;

static int
AT_Cmp(void *addr, void *ep)
{
    mib2_ipNetToMediaEntry_t *mp = (mib2_ipNetToMediaEntry_t *) ep;
    int             ret = -1;
    oid             index;

#ifdef NETSNMP_INCLUDE_IFTABLE_REWRITES
    mp->ipNetToMediaIfIndex.o_bytes[mp->ipNetToMediaIfIndex.o_length] = '\0';
    index = netsnmp_access_interface_index_find(
                    mp->ipNetToMediaIfIndex.o_bytes);
#else
    index = Interface_Index_By_Name(mp->ipNetToMediaIfIndex.o_bytes,
                                    mp->ipNetToMediaIfIndex.o_length);
#endif
    DEBUGMSGTL(("mibII/at", "......... AT_Cmp %lx<>%lx %d<>%d (%.5s)\n",
                (unsigned long)mp->ipNetToMediaNetAddress,
                (unsigned long)((if_ip_t *) addr)->ipAddr,
                ((if_ip_t *) addr)->ifIdx, index,
                mp->ipNetToMediaIfIndex.o_bytes));
    if (mp->ipNetToMediaNetAddress != ((if_ip_t *) addr)->ipAddr)
        ret = 1;
    else if (((if_ip_t *) addr)->ifIdx != index)
        ret = 1;
    else
        ret = 0;
    DEBUGMSGTL(("mibII/at", "......... AT_Cmp returns %d\n", ret));
    return ret;
}

u_char         *
var_atEntry(struct variable * vp,
            oid * name,
            size_t * length,
            int exact, size_t * var_len, WriteMethod ** write_method)
{
    /*
     * object identifier is of form:
     * 1.3.6.1.2.1.3.1.1.1.interface.1.A.B.C.D,  where A.B.C.D is IP address.
     * Interface is at offset 10,
     * IPADDR starts at offset 12.
     */
#define AT_MAX_NAME_LENGTH	16
#define AT_IFINDEX_OFF	10
    u_char         *cp;
    oid            *op;
    oid             lowest[AT_MAX_NAME_LENGTH];
    oid             current[AT_MAX_NAME_LENGTH];
    if_ip_t         NextAddr;
    mib2_ipNetToMediaEntry_t entry;
    static mib2_ipNetToMediaEntry_t Lowentry;
    int             Found = 0;
    req_e           req_type;
    int             offset, olength = 0;
    static in_addr_t      addr_ret;

    /*
     * fill in object part of name for current (less sizeof instance part)
     */

    DEBUGMSGTL(("mibII/at", "var_atEntry: "));
    DEBUGMSGOID(("mibII/at", vp->name, vp->namelen));
    DEBUGMSG(("mibII/at", " %d\n", exact));

    memset(&Lowentry, 0, sizeof(Lowentry));
    memcpy((char *) current, (char *) vp->name, vp->namelen * sizeof(oid));
    lowest[0] = 1024;
    for (NextAddr.ipAddr = (u_long) - 1, NextAddr.ifIdx = 255, req_type =
         GET_FIRST;;
         NextAddr.ipAddr = entry.ipNetToMediaNetAddress, NextAddr.ifIdx =
         current[AT_IFINDEX_OFF], req_type = GET_NEXT) {
        if (getMibstat
            (MIB_IP_NET, &entry, sizeof(mib2_ipNetToMediaEntry_t),
             req_type, &AT_Cmp, &NextAddr) != 0)
            break;
#ifdef NETSNMP_INCLUDE_IFTABLE_REWRITES
        entry.ipNetToMediaIfIndex.o_bytes[entry.ipNetToMediaIfIndex.o_length] = '\0';
        current[AT_IFINDEX_OFF] = netsnmp_access_interface_index_find(
           entry.ipNetToMediaIfIndex.o_bytes);
#else
        current[AT_IFINDEX_OFF] =
           Interface_Index_By_Name(entry.ipNetToMediaIfIndex.o_bytes,
                                   entry.ipNetToMediaIfIndex.o_length);
#endif
        if (current[6] == 3) {  /* AT group oid */
            current[AT_IFINDEX_OFF + 1] = 1;
            offset = AT_IFINDEX_OFF + 2;
            olength = AT_IFINDEX_OFF + 6;
        } else {
            offset = AT_IFINDEX_OFF + 1;
            olength = AT_IFINDEX_OFF + 5;
        }
        COPY_IPADDR(cp, (u_char *) & entry.ipNetToMediaNetAddress, op,
                    current + offset);
        if (exact) {
            if (snmp_oid_compare(current, olength, name, *length) == 0) {
                memcpy((char *) lowest, (char *) current,
                       olength * sizeof(oid));
                Lowentry = entry;
                Found++;
                break;          /* no need to search further */
            }
        } else {
            if (snmp_oid_compare(current, olength, name, *length) > 0 &&
                (Found == 0 ||
                 snmp_oid_compare(current, olength, lowest, olength) < 0)) {
                /*
                 * if new one is greater than input and closer to input than
                 * previous lowest, and is not equal to it, save this one as the "next" one.
                 */
                memcpy((char *) lowest, (char *) current,
                       olength * sizeof(oid));
                Lowentry = entry;
                Found++;
            }
        }
    }
    DEBUGMSGTL(("mibII/at", "... Found = %d\n", Found));
    if (Found == 0)
        return (NULL);
    memcpy((char *) name, (char *) lowest, olength * sizeof(oid));
    *length = olength;
    *write_method = 0;
    switch (vp->magic) {
    case IPMEDIAIFINDEX:
        *var_len = sizeof long_return;
#ifdef NETSNMP_INCLUDE_IFTABLE_REWRITES
        Lowentry.ipNetToMediaIfIndex.o_bytes[
            Lowentry.ipNetToMediaIfIndex.o_length] = '\0';
        long_return = netsnmp_access_interface_index_find(
            Lowentry.ipNetToMediaIfIndex.o_bytes);
#else
        long_return = Interface_Index_By_Name(
            Lowentry.ipNetToMediaIfIndex.o_bytes,
            Lowentry.ipNetToMediaIfIndex.o_length);
#endif
        return (u_char *) & long_return;
    case IPMEDIAPHYSADDRESS:
        *var_len = Lowentry.ipNetToMediaPhysAddress.o_length;
        return Lowentry.ipNetToMediaPhysAddress.o_bytes;
    case IPMEDIANETADDRESS:
        *var_len = sizeof(addr_ret);
        addr_ret = Lowentry.ipNetToMediaNetAddress;
        return (u_char *) &addr_ret;
    case IPMEDIATYPE:
        *var_len = sizeof long_return;
        long_return = Lowentry.ipNetToMediaType;
        return (u_char *) & long_return;
    default:
        DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_atEntry\n",
                    vp->magic));
    }
    return NULL;
}
