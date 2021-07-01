/*
 * Portions of this file are copyrighted by:
 * Copyright (c) 2016 VMware, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-features.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "agent_global_vars.h"
#include "agentx/subagent.h"
#include "utilities/iquery.h"

netsnmp_feature_child_of(iquery_all, libnetsnmpmibs)
netsnmp_feature_child_of(iquery, iquery_all)
netsnmp_feature_child_of(iquery_community_session, iquery_all)
netsnmp_feature_child_of(iquery_pdu_session, iquery_all)

netsnmp_feature_require(query_set_default_session)

#ifndef NETSNMP_FEATURE_REMOVE_IQUERY

void
netsnmp_parse_iquerySecLevel(const char *token, char *line)
{
    int secLevel;

#ifndef NETSNMP_FEATURE_REMOVE_RUNTIME_DISABLE_VERSION
    if (netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID,
                               NETSNMP_DS_LIB_DISABLE_V3)) {
        netsnmp_config_error("SNMPv3 disabled");
    } else
#endif
    if ((secLevel = parse_secLevel_conf( token, line )) >= 0 ) {
        netsnmp_ds_set_int(NETSNMP_DS_APPLICATION_ID,
                           NETSNMP_DS_AGENT_INTERNAL_SECLEVEL, secLevel);
    } else {
	netsnmp_config_error("Unknown security level: %s", line);
    }
}

void
netsnmp_parse_iqueryVersion(const char *token, char *line)
{
#ifndef NETSNMP_DISABLE_SNMPV1
    if (!strcmp( line, "1" )
#ifndef NETSNMP_FEATURE_REMOVE_RUNTIME_DISABLE_VERSION
        && !netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID,
                                NETSNMP_DS_LIB_DISABLE_V1)
#endif
        )
        netsnmp_ds_set_int(NETSNMP_DS_APPLICATION_ID,
                           NETSNMP_DS_AGENT_INTERNAL_VERSION, SNMP_VERSION_1);
    else 
#endif
#ifndef NETSNMP_DISABLE_SNMPV2C
        if ((!strcmp( line, "2"  ) || !strcasecmp( line, "2c" ))
#ifndef NETSNMP_FEATURE_REMOVE_RUNTIME_DISABLE_VERSION
            && !netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID,
                                       NETSNMP_DS_LIB_DISABLE_V2c)
#endif
            )
        netsnmp_ds_set_int(NETSNMP_DS_APPLICATION_ID,
                           NETSNMP_DS_AGENT_INTERNAL_VERSION, SNMP_VERSION_2c);
    else 
#endif
         if (!strcmp( line, "3" )
#ifndef NETSNMP_FEATURE_REMOVE_RUNTIME_DISABLE_VERSION
             && !netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID,
                                        NETSNMP_DS_LIB_DISABLE_V3)
#endif
             )
        netsnmp_ds_set_int(NETSNMP_DS_APPLICATION_ID,
                           NETSNMP_DS_AGENT_INTERNAL_VERSION, SNMP_VERSION_3);
    else {
	netsnmp_config_error("Unknown/disabled version: %s", line);
    }
}

  /*
   * Set up a default session for running internal queries.
   * This needs to be done before the config files are read,
   *  so that it is available for "monitor" directives...
   */
int
_init_default_iquery_session( int majorID, int minorID,
                              void *serverargs, void *clientarg)
{
    char *secName = netsnmp_ds_get_string(NETSNMP_DS_APPLICATION_ID,
                                          NETSNMP_DS_AGENT_INTERNAL_SECNAME);
    if (secName)
        netsnmp_query_set_default_session(
             netsnmp_iquery_user_session(secName));
    return SNMPERR_SUCCESS;
}

  /*
   * ... Unfortunately, the internal engine ID is not set up
   * until later, so this default session is incomplete.
   * The resulting engineID probe runs into problems,
   * causing the very first internal query to time out.
   *   Updating the default session with the internal engineID
   * once it has been set, fixes this problem.
   */
int
_tweak_default_iquery_session( int majorID, int minorID,
                              void *serverargs, void *clientarg)
{
    u_char eID[SNMP_MAXBUF_SMALL];
    size_t elen;
    netsnmp_session *s = netsnmp_query_get_default_session_unchecked();

    if ( s && s->securityEngineIDLen == 0 ) {
        elen = snmpv3_get_engineID(eID, sizeof(eID));
        s->securityEngineID = netsnmp_memdup(eID, elen);
        s->securityEngineIDLen = elen;
    }
    return SNMPERR_SUCCESS;
}

void init_iquery(void){
    char *type = netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID, 
                                       NETSNMP_DS_LIB_APPTYPE);
    netsnmp_ds_register_premib(ASN_OCTET_STR, type, "agentSecName",
                               NETSNMP_DS_APPLICATION_ID,
                               NETSNMP_DS_AGENT_INTERNAL_SECNAME);
    netsnmp_ds_register_premib(ASN_OCTET_STR, type, "iquerySecName",
                               NETSNMP_DS_APPLICATION_ID,
                               NETSNMP_DS_AGENT_INTERNAL_SECNAME);

    snmpd_register_config_handler("iqueryVersion",
                                   netsnmp_parse_iqueryVersion, NULL,
                                   "1 | 2c | 3");
    snmpd_register_config_handler("iquerySecLevel",
                                   netsnmp_parse_iquerySecLevel, NULL,
                                   "noAuthNoPriv | authNoPriv | authPriv");

    /*
     * Set defaults
     */
    netsnmp_ds_set_int(NETSNMP_DS_APPLICATION_ID,
                       NETSNMP_DS_AGENT_INTERNAL_VERSION, SNMP_VERSION_3);
    netsnmp_ds_set_int(NETSNMP_DS_APPLICATION_ID,
                       NETSNMP_DS_AGENT_INTERNAL_SECLEVEL, SNMP_SEC_LEVEL_AUTHNOPRIV);

    snmp_register_callback(SNMP_CALLBACK_LIBRARY, 
                           SNMP_CALLBACK_POST_PREMIB_READ_CONFIG,
                           _init_default_iquery_session, NULL);
    snmp_register_callback(SNMP_CALLBACK_LIBRARY, 
                           SNMP_CALLBACK_POST_READ_CONFIG,
                           _tweak_default_iquery_session, NULL);
}

    /**************************
     *
     *  APIs to construct an "internal query" session
     *
     **************************/

#ifndef NETSNMP_FEATURE_REMOVE_IQUERY_PDU_SESSION
netsnmp_session *netsnmp_iquery_pdu_session(netsnmp_pdu* pdu) {
    if (!pdu || NETSNMP_RUNTIME_PROTOCOL_SKIP(pdu->version))
       return NULL;
    if (pdu->version == SNMP_VERSION_3)
        return netsnmp_iquery_session( pdu->securityName, 
                           pdu->version,
                           pdu->securityModel,
                           pdu->securityLevel,
                           pdu->securityEngineID,
                           pdu->securityEngineIDLen);
    else
        return netsnmp_iquery_session((char *) pdu->community, 

                           pdu->version,
                           pdu->version+1,
                           SNMP_SEC_LEVEL_NOAUTH,
                           pdu->securityEngineID,
                           pdu->securityEngineIDLen);
}
#endif /* NETSNMP_FEATURE_REMOVE_IQUERY_PDU_SESSION */

netsnmp_session *netsnmp_iquery_user_session(char* secName){
    u_char eID[SNMP_MAXBUF_SMALL];
    size_t elen = snmpv3_get_engineID(eID, sizeof(eID));

    return netsnmp_iquery_session( secName, 
                           SNMP_VERSION_3,
                           SNMP_SEC_MODEL_USM,
                           SNMP_SEC_LEVEL_AUTHNOPRIV, eID, elen);
}

#ifndef NETSNMP_FEATURE_REMOVE_IQUERY_COMMUNITY_SESSION
netsnmp_session *netsnmp_iquery_community_session( char* community, int version ) { 
    u_char eID[SNMP_MAXBUF_SMALL];
    size_t elen = snmpv3_get_engineID(eID, sizeof(eID));

    return netsnmp_iquery_session( community, version, version+1,
                           SNMP_SEC_LEVEL_NOAUTH, eID, elen);
}
#endif /* NETSNMP_FEATURE_REMOVE_IQUERY_COMMUNITY_SESSION */

netsnmp_session *netsnmp_iquery_session(char* secName,   int   version,
                                        int   secModel,  int   secLevel,
                                       u_char* engineID, size_t engIDLen) {

    /*
     * This routine creates a completely new session every time.
     * It might be worth keeping track of which 'secNames' already
     * have iquery sessions created, and re-using the appropriate one.  
     */
    netsnmp_session *ss = NULL;

    NETSNMP_RUNTIME_PROTOCOL_CHECK(version, unsupported_version);

#ifdef NETSNMP_TRANSPORT_CALLBACK_DOMAIN
    ss = netsnmp_callback_open( callback_master_num, NULL, NULL, NULL);
    if (ss) {
        ss->version       = version;
        ss->securityModel = secModel;
        ss->securityLevel = secLevel;
        ss->securityEngineID = netsnmp_memdup(engineID, engIDLen);
        ss->securityEngineIDLen = engIDLen;
        if ( version == SNMP_VERSION_3 ) {
            ss->securityNameLen = strlen(secName);
            ss->securityName = netsnmp_memdup(secName, ss->securityNameLen);
        } else {
            ss->community = netsnmp_memdup(secName, strlen(secName));
            ss->community_len = strlen(secName);
        }
        ss->myvoid = netsnmp_check_outstanding_agent_requests;
        ss->flags |= SNMP_FLAGS_RESP_CALLBACK | SNMP_FLAGS_DONT_PROBE;
    }
#endif

  unsupported_version:
    return ss;
}

#else /* NETSNMP_FEATURE_REMOVE_IQUERY */
netsnmp_feature_unused(iquery);
#endif /* NETSNMP_FEATURE_REMOVE_IQUERY */

