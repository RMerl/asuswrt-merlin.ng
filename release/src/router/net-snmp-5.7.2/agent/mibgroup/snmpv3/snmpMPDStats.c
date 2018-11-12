/*
 * snmpMPDStats.c: tallies errors for SNMPv3 message processing.
 *
 * Portions of this file are subject to the following copyright(s).  See
 * the Net-SNMP's COPYING file for more details and other copyrights
 * that may apply:
 *
 * Portions of this file are copyrighted by:
 * Copyright (c) 2016 VMware, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */

#include <net-snmp/net-snmp-config.h>

#include <net-snmp/net-snmp-features.h>

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/sysORTable.h>

#include "snmpMPDStats.h"

#include <net-snmp/agent/snmp_get_statistic.h>

#define snmpMPDMIB 1, 3, 6, 1, 6, 3, 11
#define snmpMPDMIBObjects snmpMPDMIB, 2
#define snmpMPDMIBCompliances snmpMPDMIB, 3, 1

netsnmp_feature_require(helper_statistics)

static oid snmpMPDStats[] = { snmpMPDMIBObjects, 1 };

static netsnmp_handler_registration* snmpMPDStats_reg = NULL;
static oid snmpMPDCompliance[] = { snmpMPDMIBCompliances, 1 };

void
init_snmpMPDStats(void)
{
    netsnmp_handler_registration* s =
        netsnmp_create_handler_registration(
            "snmpMPDStats", NULL, snmpMPDStats, OID_LENGTH(snmpMPDStats),
            HANDLER_CAN_RONLY);
    if (!s)
        return;

    if (NETSNMP_REGISTER_STATISTIC_HANDLER(s, 1, MPD) != MIB_REGISTERED_OK)
        return;

    REGISTER_SYSOR_ENTRY(snmpMPDCompliance,
                         "The MIB for Message Processing and Dispatching.");
    snmpMPDStats_reg = s;
}

void
shutdown_snmpMPDStats(void)
{
    UNREGISTER_SYSOR_ENTRY(snmpMPDCompliance);
    if (snmpMPDStats_reg) {
        netsnmp_unregister_handler(snmpMPDStats_reg);
        snmpMPDStats_reg = NULL;
    }
}
