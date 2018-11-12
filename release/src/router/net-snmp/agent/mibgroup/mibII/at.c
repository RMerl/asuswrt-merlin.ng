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
#include "at.h"

/*
 * define the structure we're going to ask the agent to register our
 * information at
 */
struct variable1 at_variables[] = {
    {ATIFINDEX, ASN_INTEGER, NETSNMP_OLDAPI_RONLY,
     var_atEntry, 1, {1}},
    {ATPHYSADDRESS, ASN_OCTET_STR, NETSNMP_OLDAPI_RONLY,
     var_atEntry, 1, {2}},
    {ATNETADDRESS, ASN_IPADDRESS, NETSNMP_OLDAPI_RONLY,
     var_atEntry, 1, {3}}
};

/*
 * Define the OID pointer to the top of the mib tree that we're
 * registering underneath
 */
oid             at_variables_oid[] = { SNMP_OID_MIB2, 3, 1, 1 };

void
init_at(void)
{
    /*
     * register ourselves with the agent to handle our mib tree
     */
    REGISTER_MIB("mibII/at", at_variables, variable1, at_variables_oid);
#ifdef solaris2
    init_kernel_sunos5();
#endif
}

