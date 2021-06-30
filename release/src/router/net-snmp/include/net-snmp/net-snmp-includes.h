#ifndef NET_SNMP_INCLUDES_H
#define NET_SNMP_INCLUDES_H

    /**
     *  Convenience header file to pull in the full
     *     Net-SNMP library API in one go, together with
     *     certain commonly-required system header files.
     */


#ifndef NET_SNMP_CONFIG_H
#error "Please include <net-snmp/net-snmp-config.h> before this file"
#endif

#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

    /*
     *  The full Net-SNMP API
     */
#include <net-snmp/definitions.h>
#include <net-snmp/types.h>

#include <net-snmp/library/getopt.h>
#include <net-snmp/utilities.h>
#include <net-snmp/session_api.h>
#include <net-snmp/pdu_api.h>
#include <net-snmp/mib_api.h>
#include <net-snmp/varbind_api.h>
#include <net-snmp/config_api.h>
#include <net-snmp/output_api.h>
#include <net-snmp/snmpv3_api.h>

#endif                          /* NET_SNMP_INCLUDES_H */
