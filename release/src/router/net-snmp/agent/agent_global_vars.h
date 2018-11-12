#ifndef _AGENT_GLOBAL_VARS_H_
#define _AGENT_GLOBAL_VARS_H_

/*
 * Global variables defined in libnetsnmpagent.so that are used in MIB
 * implementations. To do: avoid that MIB implementations depend on these
 * variables and/or make sure that all these variables have a netsnmp_ prefix.
 */

/* Forward declarations. */

struct snmp_session;
struct netsnmp_agent_session_s;
struct netsnmp_agent_session_s;

/* Global variable declarations. */

extern int netsnmp_running;
extern int callback_master_num;
extern long snmp_enableauthentraps;
extern int snmp_enableauthentrapsset;

extern struct snmp_session *main_session;
extern struct netsnmp_agent_session_s *netsnmp_processing_set;
extern struct netsnmp_agent_session_s *agent_delegated_list;

extern const oid    snmptrap_oid[];
extern const size_t snmptrap_oid_len;
extern const oid    snmptrapenterprise_oid[];
extern const size_t snmptrapenterprise_oid_len;
extern const oid    sysuptime_oid[];
extern const size_t sysuptime_oid_len;
extern const oid    version_sysoid[];
extern const int    version_sysoid_len;

#endif /* _AGENT_GLOBAL_VARS_H_ */
