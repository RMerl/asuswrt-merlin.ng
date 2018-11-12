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

/*
 * var_atEntry(...
 * Arguments:
 * vp     IN      - pointer to variable entry that points here
 * name    IN/OUT  - IN/name requested, OUT/name found
 * length  IN/OUT  - length of IN/OUT oid's
 * exact   IN      - TRUE if an exact match was requested
 * var_len OUT     - length of variable or 0 if function returned
 * write_method
 *
 */

u_char         *
var_atEntry(struct variable *vp,
            oid * name,
            size_t * length,
            int exact, size_t * var_len, WriteMethod ** write_method)
{
    /*
     * Address Translation table object identifier is of form:
     * 1.3.6.1.2.1.3.1.1.1.interface.1.A.B.C.D,  where A.B.C.D is IP address.
     * Interface is at offset 10,
     * IPADDR starts at offset 12.
     *
     * IP Net to Media table object identifier is of form:
     * 1.3.6.1.2.1.4.22.1.1.1.interface.A.B.C.D,  where A.B.C.D is IP address.
     * Interface is at offset 10,
     * IPADDR starts at offset 11.
     */
    u_char         *cp;
    oid            *op;
    oid             lowest[16];
    oid             current[16];
    static char     PhysAddr[MAX_MAC_ADDR_LEN], LowPhysAddr[MAX_MAC_ADDR_LEN];
    static int      PhysAddrLen, LowPhysAddrLen;
    in_addr_t       Addr, LowAddr;
    int             foundone;
    static in_addr_t      addr_ret;
    u_short         ifIndex, lowIfIndex = 0;
    u_long          ifType, lowIfType = 0;

    int             oid_length;

    /*
     * fill in object part of name for current (less sizeof instance part)
     */
    memcpy(current, vp->name, vp->namelen * sizeof(oid));

    if (current[6] == 3) {      /* AT group oid */
        oid_length = 16;
    } else {                    /* IP NetToMedia group oid */
        oid_length = 15;
    }

    LowAddr = 0;                /* Don't have one yet */
    foundone = 0;
    ARP_Scan_Init();
    for (;;) {
        if (ARP_Scan_Next(&Addr, PhysAddr, &PhysAddrLen, &ifType, &ifIndex) == 0)
            break;
        current[10] = ifIndex;

        if (current[6] == 3) {  /* AT group oid */
            current[11] = 1;
            op = current + 12;
        } else {                /* IP NetToMedia group oid */
            op = current + 11;
        }
        cp = (u_char *) & Addr;
        *op++ = *cp++;
        *op++ = *cp++;
        *op++ = *cp++;
        *op++ = *cp++;

        if (exact) {
            if (snmp_oid_compare(current, oid_length, name, *length) == 0) {
                memcpy((char *) lowest, (char *) current,
                       oid_length * sizeof(oid));
                LowAddr = Addr;
                foundone = 1;
                lowIfIndex = ifIndex;
                memcpy(LowPhysAddr, PhysAddr, sizeof(PhysAddr));
                LowPhysAddrLen = PhysAddrLen;
                lowIfType = ifType;
                break;          /* no need to search further */
            }
        } else {
            if ((snmp_oid_compare(current, oid_length, name, *length) > 0)
                && ((foundone == 0)
                    ||
                    (snmp_oid_compare
                     (current, oid_length, lowest, oid_length) < 0))) {
                /*
                 * if new one is greater than input and closer to input than
                 * previous lowest, save this one as the "next" one.
                 */
                memcpy(lowest, current, oid_length * sizeof(oid));
                LowAddr = Addr;
                foundone = 1;
                lowIfIndex = ifIndex;
                memcpy(LowPhysAddr, PhysAddr, sizeof(PhysAddr));
                LowPhysAddrLen = PhysAddrLen;
                lowIfType = ifType;
            }
        }
    }
    if (foundone == 0)
        return NULL;

    memcpy(name, lowest, oid_length * sizeof(oid));
    *length = oid_length;
    *write_method = NULL;
    switch (vp->magic) {
    case IPMEDIAIFINDEX:       /* also ATIFINDEX */
        *var_len = sizeof long_return;
        long_return = lowIfIndex ? lowIfIndex : 1;
#if NETSNMP_NO_DUMMY_VALUES
        if (lowIfIndex == 0)
            return NULL;
#endif
        return (u_char *) & long_return;
    case IPMEDIAPHYSADDRESS:   /* also ATPHYSADDRESS */
        *var_len = LowPhysAddrLen;
        return (u_char *) LowPhysAddr;
    case IPMEDIANETADDRESS:    /* also ATNETADDRESS */
        *var_len = sizeof(addr_ret);
        addr_ret = LowAddr;
        return (u_char *) & addr_ret;
    case IPMEDIATYPE:
        *var_len = sizeof long_return;
        long_return = lowIfType;
        return (u_char *) & long_return;
    default:
        DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_atEntry\n",
                    vp->magic));
    }
    return NULL;
}
