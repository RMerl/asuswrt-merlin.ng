/*
 * agent_trap.c
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
 *
 * Portions of this file are copyrighted by:
 * Copyright (c) 2016 VMware, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */
/** @defgroup agent_trap Trap generation routines for mib modules to use
 *  @ingroup agent
 *
 * @{
 */

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-features.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_NETDB_H
#include <netdb.h>
#endif
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
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
#include <net-snmp/utilities.h>

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/agent_trap.h>
#include <net-snmp/agent/snmp_agent.h>
#include <net-snmp/agent/agent_callbacks.h>
#include "agent_global_vars.h"

#include <net-snmp/agent/agent_module_config.h>
#include <net-snmp/agent/mib_module_config.h>

#ifdef USING_AGENTX_PROTOCOL_MODULE
#include "agentx/protocol.h"
#endif

#ifdef USING_NOTIFICATION_SNMPNOTIFYTABLE_DATA_MODULE
#include "mibgroup/notification/snmpNotifyTable_data.h"
#endif

netsnmp_feature_child_of(agent_trap_all, libnetsnmpagent)

netsnmp_feature_child_of(trap_vars_with_context, agent_trap_all)
netsnmp_feature_child_of(remove_trap_session, agent_trap_all)

netsnmp_feature_child_of(send_v3trap,netsnmp_unused)
netsnmp_feature_child_of(send_trap_pdu,netsnmp_unused)

struct trap_sink {
    netsnmp_session *sesp;
    struct trap_sink *next;
    int             pdutype;
    int             version;
};

struct trap_sink *sinks = NULL;

#ifndef NETSNMP_DISABLE_SNMPV1
static int _v1_sessions = 0;
#endif /* NETSNMP_DISABLE_SNMPV1 */
static int _v2_sessions = 0;

const oid       objid_enterprisetrap[] = { NETSNMP_NOTIFICATION_MIB };
const oid       trap_version_id[] = { NETSNMP_SYSTEM_MIB };
const int       enterprisetrap_len = OID_LENGTH(objid_enterprisetrap);
const int       trap_version_id_len = OID_LENGTH(trap_version_id);

#define SNMPV2_TRAPS_PREFIX	SNMP_OID_SNMPMODULES,1,1,5
const oid       trap_prefix[]    = { SNMPV2_TRAPS_PREFIX };
const oid       cold_start_oid[] = { SNMPV2_TRAPS_PREFIX, 1 };  /* SNMPv2-MIB */

#define SNMPV2_TRAP_OBJS_PREFIX	SNMP_OID_SNMPMODULES,1,1,4
const oid       snmptrap_oid[] = { SNMPV2_TRAP_OBJS_PREFIX, 1, 0 };
const oid       snmptrapenterprise_oid[] = { SNMPV2_TRAP_OBJS_PREFIX, 3, 0 };
const oid       sysuptime_oid[] = { SNMP_OID_MIB2, 1, 3, 0 };
const size_t    snmptrap_oid_len = OID_LENGTH(snmptrap_oid);
const size_t    snmptrapenterprise_oid_len = OID_LENGTH(snmptrapenterprise_oid);
const size_t    sysuptime_oid_len = OID_LENGTH(sysuptime_oid);

#define SNMPV2_COMM_OBJS_PREFIX	SNMP_OID_SNMPMODULES,18,1
const oid       agentaddr_oid[] = { SNMPV2_COMM_OBJS_PREFIX, 3, 0 };
const size_t    agentaddr_oid_len = OID_LENGTH(agentaddr_oid);
const oid       community_oid[] = { SNMPV2_COMM_OBJS_PREFIX, 4, 0 };
const size_t    community_oid_len = OID_LENGTH(community_oid);
#if !defined(NETSNMP_DISABLE_SNMPV1) || !defined(NETSNMP_DISABLE_SNMPV2C)
char           *snmp_trapcommunity = NULL;
#endif


#define SNMP_AUTHENTICATED_TRAPS_ENABLED	1
#define SNMP_AUTHENTICATED_TRAPS_DISABLED	2

long            snmp_enableauthentraps = SNMP_AUTHENTICATED_TRAPS_DISABLED;
int             snmp_enableauthentrapsset = 0;

/*
 * Prototypes 
 */
 /*
  * static void free_trap_session (struct trap_sink *sp);
  * static void send_v1_trap (netsnmp_session *, int, int);
  * static void send_v2_trap (netsnmp_session *, int, int, int);
  */


        /*******************
	 *
	 * Trap session handling
	 *
	 *******************/

void
init_traps(void)
{
}

static void
free_trap_session(struct trap_sink *sp)
{
    DEBUGMSGTL(("trap", "freeing callback trap session (%p, %p)\n", sp, sp->sesp));
    snmp_close(sp->sesp);
    free(sp);
}

static void
_trap_version_incr(int version)
{
    switch (version) {
#ifndef NETSNMP_DISABLE_SNMPV1
        case SNMP_VERSION_1:
            ++_v1_sessions;
            break;
#endif
#ifndef NETSNMP_DISABLE_SNMPV2C
        case SNMP_VERSION_2c:
#endif
        case SNMP_VERSION_3:
            ++_v2_sessions;
            break;
        default:
            snmp_log(LOG_ERR, "unknown snmp version %d\n", version);
    }
    return;
}

static void
_trap_version_decr(int version)
{
    switch (version) {
#ifndef NETSNMP_DISABLE_SNMPV1
        case SNMP_VERSION_1:
            if (--_v1_sessions < 0) {
                snmp_log(LOG_ERR,"v1 session count < 0! fixed.\n");
                _v1_sessions = 0;
            }
            break;
#endif
#ifndef NETSNMP_DISABLE_SNMPV2C
        case SNMP_VERSION_2c:
#endif
        case SNMP_VERSION_3:
            if (--_v2_sessions < 0) {
                snmp_log(LOG_ERR,"v2 session count < 0! fixed.\n");
                _v2_sessions = 0;
            }
            break;
        default:
            snmp_log(LOG_ERR, "unknown snmp version %d\n", version);
    }
    return;
}


#ifndef NETSNMP_NO_TRAP_STATS
static void
_dump_trap_stats(netsnmp_session *sess)
{
    if (NULL == sess || NULL == sess->trap_stats)
        return;

    DEBUGIF("stats:notif") {
        DEBUGMSGT_NC(("stats:notif", "%s inform stats\n", sess->paramName));
        DEBUGMSGT_NC(("stats:notif", "    %ld sends, last @ %ld\n",
                      sess->trap_stats->sent_count,
                      sess->trap_stats->sent_last_sent));
        DEBUGMSGT_NC(("stats:notif", "    %ld acks, last @ %ld\n",
                      sess->trap_stats->ack_count,
                      sess->trap_stats->ack_last_rcvd));
        DEBUGMSGT_NC(("stats:notif", "    %ld failed sends, last @ %ld\n",
                      sess->trap_stats->sent_fail_count,
                      sess->trap_stats->sent_last_fail));
        DEBUGMSGT_NC(("stats:notif", "    %ld timeouts, last @ %ld\n",
                      sess->trap_stats->timeouts,
                      sess->trap_stats->sent_last_timeout));
        DEBUGMSGT_NC(("stats:notif", "    %ld v3 errs, last @ %ld\n",
                      sess->trap_stats->sec_err_count,
                      sess->trap_stats->sec_err_last));
    }
}
#endif /* NETSNMP_NO_TRAP_STATS */

int
netsnmp_add_notification_session(netsnmp_session * ss, int pdutype,
                                 int confirm, int version, const char *name,
                                 const char *tag, const char* profile)
{
    if (NETSNMP_RUNTIME_PROTOCOL_SKIP(version)) {
        DEBUGMSGTL(("trap", "skipping trap sink (version 0x%02x disabled)\n",
                    version));
        return 0;
    }
    if (snmp_callback_available(SNMP_CALLBACK_APPLICATION,
                                SNMPD_CALLBACK_REGISTER_NOTIFICATIONS) ==
        SNMPERR_SUCCESS) {
        /*
         * something else wants to handle notification registrations 
         */
        struct agent_add_trap_args args;
        DEBUGMSGTL(("trap", "adding callback trap sink (%p)\n", ss));
        args.ss = ss;
        args.confirm = confirm;
        args.nameData = name;
        args.nameLen = (NULL == name) ? 0 : strlen(name);
        args.tagData = tag;
        args.tagLen = (NULL == tag) ? 0 : strlen(tag);
        args.profileData = profile;
        args.profileLen = (NULL == profile) ? 0: strlen(profile);
        snmp_call_callbacks(SNMP_CALLBACK_APPLICATION,
                            SNMPD_CALLBACK_REGISTER_NOTIFICATIONS,
                            (void *) &args);
        if (args.rc != SNMPERR_SUCCESS)
            return 0;
    } else {
        /*
         * no other support exists, handle it ourselves. 
         */
        struct trap_sink *new_sink;

        DEBUGMSGTL(("trap", "adding internal trap sink\n"));
        new_sink = (struct trap_sink *) malloc(sizeof(*new_sink));
        if (new_sink == NULL)
            return 0;

        new_sink->sesp = ss;
        new_sink->pdutype = pdutype;
        new_sink->version = version;
        new_sink->next = sinks;
        sinks = new_sink;
    }

    _trap_version_incr(version);

    return 1;
}

/*
 * xxx needs update to support embedded NUL.
 * xxx should probably also be using and unregister callback, similar to
 *     how registaration is done.
 */
void
netsnmp_unregister_notification(const char *name, u_char len)
{
    if (snmp_callback_available(SNMP_CALLBACK_APPLICATION,
                                SNMPD_CALLBACK_UNREGISTER_NOTIFICATIONS) ==
        SNMPERR_SUCCESS) {
        /*
         * something else wants to handle notification registrations 
         */
        struct agent_add_trap_args args;
        DEBUGMSGTL(("trap", "removing callback trap sink\n"));
        args.nameData = name;
        args.nameLen = len;
        snmp_call_callbacks(SNMP_CALLBACK_APPLICATION,
                            SNMPD_CALLBACK_UNREGISTER_NOTIFICATIONS,
                            (void *) &args);
    } else
        NETSNMP_LOGONCE((LOG_WARNING,
                         "netsnmp_unregister_notification not supported\n"));
}

int
add_trap_session(netsnmp_session * ss, int pdutype, int confirm,
                         int version)
{
    return netsnmp_add_notification_session(ss, pdutype, confirm, version,
                                            NULL, NULL, NULL);
}

#ifndef NETSNMP_FEATURE_REMOVE_REMOVE_TRAP_SESSION
int
remove_trap_session(netsnmp_session * ss)
{
    struct trap_sink *sp = sinks, *prev = NULL;

    DEBUGMSGTL(("trap", "removing trap sessions\n"));
    while (sp) {
        if (sp->sesp == ss) {
            if (prev) {
                prev->next = sp->next;
            } else {
                sinks = sp->next;
            }
            _trap_version_decr(ss->version);
            /*
             * I don't believe you *really* want to close the session here;
             * it may still be in use for other purposes.  In particular this
             * is awkward for AgentX, since we want to call this function
             * from the session's callback.  Let's just free the trapsink
             * data structure.  [jbpn]  
             */
            /*
             * free_trap_session(sp);  
             */
            DEBUGMSGTL(("trap", "removing trap session (%p, %p)\n", sp, sp->sesp));
            free(sp);
            return 1;
        }
        prev = sp;
        sp = sp->next;
    }
    return 0;
}
#endif /* NETSNMP_FEATURE_REMOVE_REMOVE_TRAP_SESSION */

#if !defined(NETSNMP_DISABLE_SNMPV1) || !defined(NETSNMP_DISABLE_SNMPV2C)
netsnmp_session *
netsnmp_create_v1v2_notification_session(const char *sink, const char* sinkport,
                                         const char *com, const char *src,
                                         int version, int pdutype,
                                         const char *name, const char *tag,
                                         const char* profile)
{
    netsnmp_transport *t;
    netsnmp_session session, *sesp;
    netsnmp_tdomain_spec tspec;
    char                 tmp[SPRINT_MAX_LEN];
    int                  rc;
    const char          *client_addr = NULL;

    if (NETSNMP_RUNTIME_PROTOCOL_SKIP(version)) {
        config_perror("SNMP version disabled");
        DEBUGMSGTL(("trap", "skipping trap sink (version 0x%02x disabled)\n",
                    version));
        return NULL;
    }

    snmp_sess_init(&session);
    session.version = version;
    if (com) {
        session.community = (u_char *) NETSNMP_REMOVE_CONST(char *, com);
        session.community_len = strlen(com);
    }

    /*
     * for informs, set retries to default
     */
    if (SNMP_MSG_INFORM == pdutype) {
        session.timeout = SNMP_DEFAULT_TIMEOUT;
        session.retries = SNMP_DEFAULT_RETRIES;
    }

    memset(&tspec, 0, sizeof(netsnmp_tdomain_spec));

    /*
     * use specified soure or client addr, if available. If no, and
     * if the sink is localhost, bind to localhost, to reduce open ports.
     */
    if (NULL != src)
        tspec.source = src;
    else {
        client_addr = netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID,
                                            NETSNMP_DS_LIB_CLIENT_ADDR);
        if ((NULL == client_addr) &&
            ((0 == strcmp("localhost",sink)) ||
             (0 == strcmp("127.0.0.1",sink))))
            client_addr = "localhost";
        tspec.source = client_addr;
    }
    session.localname = NETSNMP_REMOVE_CONST(char *,tspec.source);

    tspec.application = "snmptrap";
    if (NULL == sinkport)
        tspec.target = sink;
    else {
        snprintf(tmp, sizeof(tmp)-1,"%s:%s", sink, sinkport);
        tspec.target = tmp;
    }
    tspec.default_domain = NULL;
    tspec.default_target = sinkport;
    t = netsnmp_tdomain_transport_tspec(&tspec);
    if ((NULL == t) ||
        ((sesp = snmp_add(&session, t, NULL, NULL)) == NULL)) {
        /** diagnose snmp_open errors with the input netsnmp_session pointer */
        snmp_sess_perror("snmpd: netsnmp_create_notification_session",
                         &session);
        /* transport freed by snmp_add */
        return NULL;
    }

    rc = netsnmp_add_notification_session(sesp, pdutype,
                                          (pdutype == SNMP_MSG_INFORM),
                                          version, name, tag, profile);
    if (0 == rc)
        return NULL;

    return sesp;
}

int
create_trap_session_with_src(const char *sink, const char* sinkport,
                             const char *com, const char *src, int version,
                             int pdutype)
{
    void *ss = netsnmp_create_v1v2_notification_session(sink, sinkport, com,
                                                        src, version, pdutype,
                                                        NULL, NULL, NULL);
    return (ss != NULL);
}

int
create_trap_session2(const char *sink, const char* sinkport,
                     char *com, int version, int pdutype)
{
    return create_trap_session_with_src(sink, sinkport, com, NULL, version,
                                        pdutype);
}

int
create_trap_session(char *sink, u_short sinkport,
		    char *com, int version, int pdutype)
{
    void *ss;
    char buf[sizeof(sinkport) * 3 + 2];
    if (sinkport != 0) {
	sprintf(buf, ":%hu", sinkport);
	snmp_log(LOG_NOTICE,
		 "Using a separate port number is deprecated, please correct "
		 "the sink specification instead");
    }
    ss = netsnmp_create_v1v2_notification_session(sink, sinkport ? buf : NULL,
                                                  com, NULL, version, pdutype,
                                                  NULL, NULL, NULL);
    return (ss != NULL);
}

#endif /* support for community based SNMP */

void
snmpd_free_trapsinks(void)
{
    struct trap_sink *sp = sinks;
    DEBUGMSGTL(("trap", "freeing trap sessions\n"));
    while (sp) {
        sinks = sinks->next;
        _trap_version_decr(sp->version);
        free_trap_session(sp);
        sp = sinks;
    }
}

        /*******************
	 *
	 * Trap handling
	 *
	 *******************/


netsnmp_pdu*
convert_v2pdu_to_v1( netsnmp_pdu* template_v2pdu )
{
    netsnmp_pdu           *template_v1pdu;
    netsnmp_variable_list *first_vb, *vblist;
    netsnmp_variable_list *var;

    /*
     * Make a copy of the v2 Trap PDU
     *   before starting to convert this
     *   into a v1 Trap PDU.
     */
    template_v1pdu = snmp_clone_pdu( template_v2pdu);
    if (!template_v1pdu) {
        snmp_log(LOG_WARNING,
                 "send_trap: failed to copy v1 template PDU\n");
        return NULL;
    }
    template_v1pdu->command = SNMP_MSG_TRAP;
    first_vb = template_v1pdu->variables;
    vblist   = template_v1pdu->variables;

    /*
     * The first varbind should be the system uptime.
     */
    if (!vblist ||
        snmp_oid_compare(vblist->name,  vblist->name_length,
                         sysuptime_oid, sysuptime_oid_len)) {
        snmp_log(LOG_WARNING,
                 "send_trap: no v2 sysUptime varbind to set from\n");
        snmp_free_pdu(template_v1pdu);
        return NULL;
    }
    template_v1pdu->time = *vblist->val.integer;
    vblist = vblist->next_variable;
            
    /*
     * The second varbind should be the snmpTrapOID.
     */
    if (!vblist ||
        snmp_oid_compare(vblist->name, vblist->name_length,
                         snmptrap_oid, snmptrap_oid_len)) {
        snmp_log(LOG_WARNING,
                 "send_trap: no v2 trapOID varbind to set from\n");
        snmp_free_pdu(template_v1pdu);
        return NULL;
    }

    /*
     * Check the v2 varbind list for any varbinds
     *  that are not valid in an SNMPv1 trap.
     *  This basically means Counter64 values.
     *
     * RFC 2089 said to omit such varbinds from the list.
     * RFC 2576/3584 say to drop the trap completely.
     */
    for (var = vblist->next_variable; var; var = var->next_variable) {
        if ( var->type == ASN_COUNTER64 ) {
            snmp_log(LOG_WARNING,
                     "send_trap: v1 traps can't carry Counter64 varbinds\n");
            snmp_free_pdu(template_v1pdu);
            return NULL;
        }
    }

    /*
     * Set the generic & specific trap types,
     *    and the enterprise field from the v2 varbind list.
     * If there's an agentIPAddress varbind, set the agent_addr too
     */
    if (!snmp_oid_compare(vblist->val.objid, OID_LENGTH(trap_prefix),
                          trap_prefix,       OID_LENGTH(trap_prefix))) {
        /*
         * For 'standard' traps, extract the generic trap type
         *   from the snmpTrapOID value, and take the enterprise
         *   value from the 'snmpEnterprise' varbind.
         */
        template_v1pdu->trap_type =
            vblist->val.objid[OID_LENGTH(trap_prefix)] - 1;
        template_v1pdu->specific_type = 0;

        var = find_varbind_in_list( vblist,
                             snmptrapenterprise_oid,
                             snmptrapenterprise_oid_len);
        if (var) {
            template_v1pdu->enterprise_length = var->val_len/sizeof(oid);
            template_v1pdu->enterprise =
                snmp_duplicate_objid(var->val.objid,
                                     template_v1pdu->enterprise_length);
        } else {
            template_v1pdu->enterprise        = NULL;
            template_v1pdu->enterprise_length = 0;		/* XXX ??? */
        }
    } else {
        /*
         * For enterprise-specific traps, split the snmpTrapOID value
         *   into enterprise and specific trap
         */
        size_t len = vblist->val_len / sizeof(oid);
        if ( len <= 2 ) {
            snmp_log(LOG_WARNING,
                     "send_trap: v2 trapOID too short (%d)\n", (int)len);
            snmp_free_pdu(template_v1pdu);
            return NULL;
        }
        template_v1pdu->trap_type     = SNMP_TRAP_ENTERPRISESPECIFIC;
        template_v1pdu->specific_type = vblist->val.objid[len - 1];
        len--;
        if (vblist->val.objid[len-1] == 0)
            len--;
        SNMP_FREE(template_v1pdu->enterprise);
        template_v1pdu->enterprise =
            snmp_duplicate_objid(vblist->val.objid, len);
        template_v1pdu->enterprise_length = len;
    }
    var = find_varbind_in_list( vblist, agentaddr_oid,
                                        agentaddr_oid_len);
    if (var) {
        memcpy(template_v1pdu->agent_addr,
               var->val.string, 4);
    }

    /*
     * The remainder of the v2 varbind list is kept
     * as the v2 varbind list.  Update the PDU and
     * free the two redundant varbinds.
     */
    template_v1pdu->variables = vblist->next_variable;
    vblist->next_variable = NULL;
    snmp_free_varbind( first_vb );
            
    return template_v1pdu;
}

/*
 * Set t_oid from the PDU enterprise & specific trap fields.
 */
int
netsnmp_build_trap_oid(netsnmp_pdu *pdu, oid *t_oid, size_t *t_oid_len)
{
    if (NULL == pdu || NULL == t_oid || NULL == t_oid_len)
        return SNMPERR_GENERR;
    if (pdu->trap_type == SNMP_TRAP_ENTERPRISESPECIFIC) {
        if (*t_oid_len < (pdu->enterprise_length + 2))
            return SNMPERR_LONG_OID;
        memcpy(t_oid, pdu->enterprise, pdu->enterprise_length*sizeof(oid));
        *t_oid_len = pdu->enterprise_length;
        t_oid[(*t_oid_len)++] = 0;
        t_oid[(*t_oid_len)++] = pdu->specific_type;
    } else {
        /** use cold_start_oid as template */
        if (*t_oid_len < OID_LENGTH(cold_start_oid))
            return SNMPERR_LONG_OID;
        memcpy(t_oid, cold_start_oid, sizeof(cold_start_oid));
        t_oid[9]  = pdu->trap_type + 1; /* set actual trap type */
        *t_oid_len = OID_LENGTH(cold_start_oid);
    }
    return SNMPERR_SUCCESS;
}

netsnmp_pdu*
convert_v1pdu_to_v2( netsnmp_pdu* template_v1pdu )
{
    netsnmp_pdu           *template_v2pdu;
    netsnmp_variable_list *var;
    oid                    enterprise[MAX_OID_LEN];
    size_t                 enterprise_len;

    /*
     * Make a copy of the v1 Trap PDU
     *   before starting to convert this
     *   into a v2 Trap PDU.
     */
    template_v2pdu = snmp_clone_pdu( template_v1pdu);
    if (!template_v2pdu) {
        snmp_log(LOG_WARNING,
                 "send_trap: failed to copy v2 template PDU\n");
        return NULL;
    }
    template_v2pdu->command = SNMP_MSG_TRAP2;

    /*
     * Insert an snmpTrapOID varbind before the original v1 varbind list
     *   either using one of the standard defined trap OIDs,
     *   or constructing this from the PDU enterprise & specific trap fields
     */
    var = NULL;
    enterprise_len = OID_LENGTH(enterprise);
    if ((netsnmp_build_trap_oid(template_v1pdu, enterprise, &enterprise_len)
         != SNMPERR_SUCCESS) ||
        !snmp_varlist_add_variable( &var,
             snmptrap_oid, snmptrap_oid_len,
             ASN_OBJECT_ID,
             (u_char*)enterprise, enterprise_len*sizeof(oid))) {
        snmp_log(LOG_WARNING,
                 "send_trap: failed to insert copied snmpTrapOID varbind\n");
        snmp_free_pdu(template_v2pdu);
        return NULL;
    }
    var->next_variable        = template_v2pdu->variables;
    template_v2pdu->variables = var;

    /*
     * Insert a sysUptime varbind at the head of the v2 varbind list
     */
    var = NULL;
    if (!snmp_varlist_add_variable( &var,
             sysuptime_oid, sysuptime_oid_len,
             ASN_TIMETICKS,
             (u_char*)&(template_v1pdu->time), 
             sizeof(template_v1pdu->time))) {
        snmp_log(LOG_WARNING,
                 "send_trap: failed to insert copied sysUptime varbind\n");
        snmp_free_pdu(template_v2pdu);
        return NULL;
    }
    var->next_variable        = template_v2pdu->variables;
    template_v2pdu->variables = var;

    /*
     * Append the other three conversion varbinds,
     *  (snmpTrapAgentAddr, snmpTrapCommunity & snmpTrapEnterprise)
     *  if they're not already present.
     *  But don't bomb out completely if there are problems.
     */
    var = find_varbind_in_list( template_v2pdu->variables,
                                agentaddr_oid, agentaddr_oid_len);
    if (!var && (template_v1pdu->agent_addr[0]
              || template_v1pdu->agent_addr[1]
              || template_v1pdu->agent_addr[2]
              || template_v1pdu->agent_addr[3])) {
        if (!snmp_varlist_add_variable( &(template_v2pdu->variables),
                 agentaddr_oid, agentaddr_oid_len,
                 ASN_IPADDRESS,
                 (u_char*)&(template_v1pdu->agent_addr), 
                 sizeof(template_v1pdu->agent_addr)))
            snmp_log(LOG_WARNING,
                 "send_trap: failed to append snmpTrapAddr varbind\n");
    }
    var = find_varbind_in_list( template_v2pdu->variables,
                                community_oid, community_oid_len);
    if (!var && template_v1pdu->community) {
        if (!snmp_varlist_add_variable( &(template_v2pdu->variables),
                 community_oid, community_oid_len,
                 ASN_OCTET_STR,
                 template_v1pdu->community, 
                 template_v1pdu->community_len))
            snmp_log(LOG_WARNING,
                 "send_trap: failed to append snmpTrapCommunity varbind\n");
    }
    var = find_varbind_in_list( template_v2pdu->variables,
                                snmptrapenterprise_oid,
                                snmptrapenterprise_oid_len);
    if (!var) {
        if (!snmp_varlist_add_variable( &(template_v2pdu->variables),
                 snmptrapenterprise_oid, snmptrapenterprise_oid_len,
                 ASN_OBJECT_ID,
                 (u_char*)template_v1pdu->enterprise, 
                 template_v1pdu->enterprise_length*sizeof(oid)))
            snmp_log(LOG_WARNING,
                 "send_trap: failed to append snmpEnterprise varbind\n");
    }
    return template_v2pdu;
}

/**
 * This function allows you to make a distinction between generic 
 * traps from different classes of equipment. For example, you may want 
 * to handle a SNMP_TRAP_LINKDOWN trap for a particular device in a 
 * different manner to a generic system SNMP_TRAP_LINKDOWN trap.
 *   
 *
 * @param trap is the generic trap type.  The trap types are:
 *		- SNMP_TRAP_COLDSTART:
 *			cold start
 *		- SNMP_TRAP_WARMSTART:
 *			warm start
 *		- SNMP_TRAP_LINKDOWN:
 *			link down
 *		- SNMP_TRAP_LINKUP:
 *			link up
 *		- SNMP_TRAP_AUTHFAIL:
 *			authentication failure
 *		- SNMP_TRAP_EGPNEIGHBORLOSS:
 *			egp neighbor loss
 *		- SNMP_TRAP_ENTERPRISESPECIFIC:
 *			enterprise specific
 *			
 * @param specific is the specific trap value.
 *
 * @param enterprise is an enterprise oid in which you want to send specific 
 *	traps from. 
 *
 * @param enterprise_length is the length of the enterprise oid, use macro,
 *	OID_LENGTH, to compute length.
 *
 * @param vars is used to supply list of variable bindings to form an SNMPv2 
 *	trap.
 *
 * @param context currently unused 
 *
 * @param flags currently unused 
 *
 * @return void
 *
 * @see send_easy_trap
 * @see send_v2trap
 */
int
netsnmp_send_traps(int trap, int specific,
                          const oid * enterprise, int enterprise_length,
                          netsnmp_variable_list * vars,
                          const char * context, int flags)
{
    netsnmp_pdu           *template_v1pdu;
    netsnmp_pdu           *template_v2pdu;
    netsnmp_variable_list *vblist = NULL;
    netsnmp_variable_list *trap_vb;
    netsnmp_variable_list *var;
    in_addr_t             *pdu_in_addr_t;
    u_long                 uptime;
    struct trap_sink *sink;
    const char            *v1trapaddress;
    int                    res = 0;

    DEBUGMSGTL(( "trap", "send_trap %d %d ", trap, specific));
    DEBUGMSGOID(("trap", enterprise, enterprise_length));
    DEBUGMSG(( "trap", "\n"));

    if (vars) {
        vblist = snmp_clone_varbind( vars );
        if (!vblist) {
            snmp_log(LOG_WARNING,
                     "send_trap: failed to clone varbind list\n");
            return -1;
        }
    }

    if ( trap == -1 ) {
        /*
         * Construct the SNMPv2-style notification PDU
         */
        if (!vblist) {
            snmp_log(LOG_WARNING,
                     "send_trap: called with NULL v2 information\n");
            return -1;
        }
        template_v2pdu = snmp_pdu_create(SNMP_MSG_TRAP2);
        if (!template_v2pdu) {
            snmp_log(LOG_WARNING,
                     "send_trap: failed to construct v2 template PDU\n");
            snmp_free_varbind(vblist);
            return -1;
        }

        /*
         * Check the varbind list we've been given.
         * If it starts with a 'sysUptime.0' varbind, then use that.
         * Otherwise, prepend a suitable 'sysUptime.0' varbind.
         */
        if (!snmp_oid_compare( vblist->name,    vblist->name_length,
                               sysuptime_oid, sysuptime_oid_len )) {
            template_v2pdu->variables = vblist;
            trap_vb  = vblist->next_variable;
        } else {
            uptime   = netsnmp_get_agent_uptime();
            var = NULL;
            snmp_varlist_add_variable( &var,
                           sysuptime_oid, sysuptime_oid_len,
                           ASN_TIMETICKS, (u_char*)&uptime, sizeof(uptime));
            if (!var) {
                snmp_log(LOG_WARNING,
                     "send_trap: failed to insert sysUptime varbind\n");
                snmp_free_pdu(template_v2pdu);
                snmp_free_varbind(vblist);
                return -1;
            }
            template_v2pdu->variables = var;
            var->next_variable        = vblist;
            trap_vb  = vblist;
        }

        /*
         * 'trap_vb' should point to the snmpTrapOID.0 varbind,
         *   identifying the requested trap.  If not then bomb out.
         * If it's a 'standard' trap, then we need to append an
         *   snmpEnterprise varbind (if there isn't already one).
         */
        if (!trap_vb ||
            snmp_oid_compare(trap_vb->name, trap_vb->name_length,
                             snmptrap_oid,  snmptrap_oid_len)) {
            snmp_log(LOG_WARNING,
                     "send_trap: no v2 trapOID varbind provided\n");
            snmp_free_pdu(template_v2pdu);
            return -1;
        }
        if (!snmp_oid_compare(vblist->val.objid, OID_LENGTH(trap_prefix),
                              trap_prefix,       OID_LENGTH(trap_prefix))) {
            var = find_varbind_in_list( template_v2pdu->variables,
                                        snmptrapenterprise_oid,
                                        snmptrapenterprise_oid_len);
            if (!var &&
                !snmp_varlist_add_variable( &(template_v2pdu->variables),
                     snmptrapenterprise_oid, snmptrapenterprise_oid_len,
                     ASN_OBJECT_ID,
                     enterprise, enterprise_length*sizeof(oid))) {
                snmp_log(LOG_WARNING,
                     "send_trap: failed to add snmpEnterprise to v2 trap\n");
                snmp_free_pdu(template_v2pdu);
                return -1;
            }
        }
            

        /*
         * If everything's OK, convert the v2 template into an SNMPv1 trap PDU.
         */
        template_v1pdu = convert_v2pdu_to_v1( template_v2pdu );
        if (!template_v1pdu) {
            snmp_log(LOG_WARNING,
                     "send_trap: failed to convert v2->v1 template PDU\n");
        }

    } else {
        /*
         * Construct the SNMPv1 trap PDU....
         */
        template_v1pdu = snmp_pdu_create(SNMP_MSG_TRAP);
        if (!template_v1pdu) {
            snmp_log(LOG_WARNING,
                     "send_trap: failed to construct v1 template PDU\n");
            snmp_free_varbind(vblist);
            return -1;
        }
        template_v1pdu->trap_type     = trap;
        template_v1pdu->specific_type = specific;
        template_v1pdu->time          = netsnmp_get_agent_uptime();

        if (snmp_clone_mem((void **) &template_v1pdu->enterprise,
                       enterprise, enterprise_length * sizeof(oid))) {
            snmp_log(LOG_WARNING,
                     "send_trap: failed to set v1 enterprise OID\n");
            snmp_free_varbind(vblist);
            snmp_free_pdu(template_v1pdu);
            return -1;
        }
        template_v1pdu->enterprise_length = enterprise_length;

        template_v1pdu->flags    |= UCD_MSG_FLAG_FORCE_PDU_COPY;
        template_v1pdu->variables = vblist;

        /*
         * ... and convert it into an SNMPv2-style notification PDU.
         */

        template_v2pdu = convert_v1pdu_to_v2( template_v1pdu );
        if (!template_v2pdu) {
            snmp_log(LOG_WARNING,
                     "send_trap: failed to convert v1->v2 template PDU\n");
        }
    }

    /*
     * Check whether we're ignoring authFail traps
     */
    if (template_v1pdu) {
      if (template_v1pdu->trap_type == SNMP_TRAP_AUTHFAIL &&
        snmp_enableauthentraps == SNMP_AUTHENTICATED_TRAPS_DISABLED) {
        snmp_free_pdu(template_v1pdu);
        snmp_free_pdu(template_v2pdu);
        return 0;
      }

    /*
     * Ensure that the v1 trap PDU includes the local IP address
     */
       pdu_in_addr_t = (in_addr_t *) template_v1pdu->agent_addr;
       v1trapaddress = netsnmp_ds_get_string(NETSNMP_DS_APPLICATION_ID,
                                             NETSNMP_DS_AGENT_TRAP_ADDR);
       if (v1trapaddress != NULL) {
           /* "v1trapaddress" was specified in config, try to resolve it */
           res = netsnmp_gethostbyname_v4(v1trapaddress, pdu_in_addr_t);
       }
       if (v1trapaddress == NULL || res < 0) {
           /* "v1trapaddress" was not specified in config or the resolution failed,
            * try any local address */
           *pdu_in_addr_t = get_myaddr();
       }

    }

    if (template_v2pdu) {
	/* A context name was provided, so copy it and its length to the v2 pdu
	 * template. */
	if (context != NULL)
	{
		template_v2pdu->contextName    = strdup(context);
		template_v2pdu->contextNameLen = strlen(context);
	}
    }

    /*
     *  Now loop through the list of trap sinks
     *   and call the trap callback routines,
     *   providing an appropriately formatted PDU in each case
     */
    for (sink = sinks; sink; sink = sink->next) {
#ifndef NETSNMP_DISABLE_SNMPV1
        if (sink->version == SNMP_VERSION_1) {
            if (template_v1pdu &&
                !netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID,
                                        NETSNMP_DS_LIB_DISABLE_V1)) {
                send_trap_to_sess(sink->sesp, template_v1pdu);
            }
        } else
#endif
        if (template_v2pdu) {
            template_v2pdu->command = sink->pdutype;
            send_trap_to_sess(sink->sesp, template_v2pdu);
        }
    }
#ifndef NETSNMP_DISABLE_SNMPV1
    if (template_v1pdu && _v1_sessions)
        snmp_call_callbacks(SNMP_CALLBACK_APPLICATION,
                        SNMPD_CALLBACK_SEND_TRAP1, template_v1pdu);
#endif
    if (template_v2pdu && _v2_sessions)
        snmp_call_callbacks(SNMP_CALLBACK_APPLICATION,
                        SNMPD_CALLBACK_SEND_TRAP2, template_v2pdu);
    snmp_free_pdu(template_v1pdu);
    snmp_free_pdu(template_v2pdu);
    return 0;
}


void
send_enterprise_trap_vars(int trap,
                          int specific,
                          const oid * enterprise, int enterprise_length,
                          netsnmp_variable_list * vars)
{
    netsnmp_send_traps(trap, specific,
                       enterprise, enterprise_length,
                       vars, NULL, 0);
    return;
}

/**
 * Handles stats for basic traps (really just send failed
*/
int
handle_trap_callback(int op, netsnmp_session * session, int reqid,
                     netsnmp_pdu *pdu, void *magic)
{
    if (NULL == session)
        return 0;

    DEBUGMSGTL(("trap", "handle_trap_callback for session %s\n",
                session->paramName ? session->paramName : "UNKNOWN"));
    switch (op) {

    case NETSNMP_CALLBACK_OP_SEND_FAILED:
        DEBUGMSGTL(("trap", "failed to send an inform for reqid=%d\n", reqid));
#ifndef NETSNMP_NO_TRAP_STATS
        if (session->trap_stats) {
            session->trap_stats->sent_last_fail = netsnmp_get_agent_uptime();
            ++session->trap_stats->sent_fail_count;
        }
#endif /* NETSNMP_NO_TRAP_STATS */
        break;

    case NETSNMP_CALLBACK_OP_SEC_ERROR:
        DEBUGMSGTL(("trap", "sec error sending a trap for reqid=%d\n",
                    reqid));
#ifndef NETSNMP_NO_TRAP_STATS
        if (session->trap_stats) {
            session->trap_stats->sec_err_last = netsnmp_get_agent_uptime();
            ++session->trap_stats->sec_err_count;
        }
#endif /* NETSNMP_NO_TRAP_STATS */
        break;

    case NETSNMP_CALLBACK_OP_RECEIVED_MESSAGE:
    case NETSNMP_CALLBACK_OP_TIMED_OUT:
    case NETSNMP_CALLBACK_OP_RESEND:
    default:
        DEBUGMSGTL(("trap",
                    "received op=%d for reqid=%d when trying to send a trap\n",
                    op, reqid));
    }
#ifndef NETSNMP_NO_TRAP_STATS
    if (session->trap_stats)
        _dump_trap_stats(session);
#endif /* NETSNMP_NO_TRAP_STATS */

    return 1;
}


/**
 * Captures responses or the lack there of from INFORMs that were sent
 * 1) a response is received from an INFORM
 * 2) one isn't received and the retries/timeouts have failed
*/
int
handle_inform_response(int op, netsnmp_session * session,
                       int reqid, netsnmp_pdu *pdu,
                       void *magic)
{
    if (NULL == session)
        return 0;

    DEBUGMSGTL(("trap", "handle_inform_response for session %s\n",
                session->paramName ? session->paramName : "UNKNOWN"));
    switch (op) {

    case NETSNMP_CALLBACK_OP_RECEIVED_MESSAGE:
        snmp_increment_statistic(STAT_SNMPINPKTS);
        if (pdu->command != SNMP_MSG_REPORT) {
            DEBUGMSGTL(("trap", "received the inform response for reqid=%d\n",
                        reqid));
#ifndef NETSNMP_NO_TRAP_STATS
            if (session->trap_stats) {
                ++session->trap_stats->ack_count;
                session->trap_stats->ack_last_rcvd = netsnmp_get_agent_uptime();
            }
#endif /* NETSNMP_NO_TRAP_STATS */
            break;
        } else {
            int type = session->s_snmp_errno ? session->s_snmp_errno :
                snmpv3_get_report_type(pdu);
            DEBUGMSGTL(("trap", "received report %d for inform reqid=%d\n",
                        type, reqid));
            /*
             * xxx-rks: what stats, if any, to bump for other report types?
             * - ignore NOT_IN_TIME, as agent will sync and retry.
             */
            if (SNMPERR_AUTHENTICATION_FAILURE != type)
                break;
        }
        /** AUTH failures fall through to sec error */
	/* FALL THROUGH */

    case NETSNMP_CALLBACK_OP_SEC_ERROR:
        DEBUGMSGTL(("trap", "sec error sending an inform for reqid=%d\n",
                    reqid));
#ifndef NETSNMP_NO_TRAP_STATS
        if (session->trap_stats) {
            session->trap_stats->sec_err_last = netsnmp_get_agent_uptime();
            ++session->trap_stats->sec_err_count;
        }
#endif /* NETSNMP_NO_TRAP_STATS */
        break;

    case NETSNMP_CALLBACK_OP_TIMED_OUT:
        DEBUGMSGTL(("trap",
                    "received a timeout sending an inform for reqid=%d\n",
                    reqid));
#ifndef NETSNMP_NO_TRAP_STATS
        if (session->trap_stats) {
            ++session->trap_stats->timeouts;
            session->trap_stats->sent_last_timeout =
                netsnmp_get_agent_uptime();
        }
#endif /* NETSNMP_NO_TRAP_STATS */
        break;

    case NETSNMP_CALLBACK_OP_RESEND:
        DEBUGMSGTL(("trap", "resending an inform for reqid=%d\n", reqid));
#ifndef NETSNMP_NO_TRAP_STATS
        if (session->trap_stats)
            session->trap_stats->sent_last_sent = netsnmp_get_agent_uptime();
#endif /* NETSNMP_NO_TRAP_STATS */
        break;

    case NETSNMP_CALLBACK_OP_SEND_FAILED:
        DEBUGMSGTL(("trap", "failed to send an inform for reqid=%d\n", reqid));
#ifndef NETSNMP_NO_TRAP_STATS
        if (session->trap_stats) {
            session->trap_stats->sent_last_fail = netsnmp_get_agent_uptime();
            ++session->trap_stats->sent_fail_count;
        }
#endif /* NETSNMP_NO_TRAP_STATS */
        break;

    default:
        DEBUGMSGTL(("trap", "received op=%d for reqid=%d when trying to send an inform\n", op, reqid));
    }

#ifndef NETSNMP_NO_TRAP_STATS
    if (session->trap_stats)
        _dump_trap_stats(session);
#endif /* NETSNMP_NO_TRAP_STATS */

    return 1;
}


/*
 * send_trap_to_sess: sends a trap to a session but assumes that the
 * pdu is constructed correctly for the session type. 
 */
void
send_trap_to_sess(netsnmp_session * sess, netsnmp_pdu *template_pdu)
{
    netsnmp_pdu    *pdu;
    int            result;

    if (!sess || !template_pdu)
        return;

    if (NETSNMP_RUNTIME_PROTOCOL_SKIP(sess->version)) {
        DEBUGMSGTL(("trap", "not sending trap type=%d, version %02lx disabled\n",
                    template_pdu->command, sess->version));
        return;
    }
    DEBUGMSGTL(("trap", "sending trap type=%d, version=%ld\n",
                template_pdu->command, sess->version));

#ifndef NETSNMP_DISABLE_SNMPV1
    if (sess->version == SNMP_VERSION_1 &&
        (template_pdu->command != SNMP_MSG_TRAP))
        return;                 /* Skip v1 sinks for v2 only traps */
    if (sess->version != SNMP_VERSION_1 &&
        (template_pdu->command == SNMP_MSG_TRAP))
        return;                 /* Skip v2+ sinks for v1 only traps */
#endif
    template_pdu->version = sess->version;
    pdu = snmp_clone_pdu(template_pdu);
    if(!pdu) {
        snmp_log(LOG_WARNING, "send_trap: failed to clone PDU\n");
        return;
    }

    pdu->sessid = sess->sessid; /* AgentX only ? */
    /*
     * RFC 3414 sayeth:
     *
     * - If an SNMP engine uses a msgID for correlating Response messages to
     *   outstanding Request messages, then it MUST use different msgIDs in
     *   all such Request messages that it sends out during a Time Window
     *   (150 seconds) period.
     *
     *   A Command Generator or Notification Originator Application MUST use
     *   different request-ids in all Request PDUs that it sends out during
     *   a TimeWindow (150 seconds) period.
     */
    pdu->reqid = snmp_get_next_reqid();
    pdu->msgid = snmp_get_next_msgid();

#ifndef NETSNMP_NO_TRAP_STATS
    /** allocate space for trap stats */
    if (NULL == sess->trap_stats) {
        sess->trap_stats = SNMP_MALLOC_TYPEDEF(netsnmp_trap_stats);
        if (NULL == sess->trap_stats)
            snmp_log(LOG_ERR, "malloc for %s trap stats failed\n",
                     sess->paramName ? sess->paramName : "UNKNOWN");
    }
#endif /* NETSNMP_NO_TRAP_STATS */

    if ( template_pdu->command == SNMP_MSG_INFORM
#ifdef USING_AGENTX_PROTOCOL_MODULE
         || template_pdu->command == AGENTX_MSG_NOTIFY
#endif
       ) {
        result =
            snmp_async_send(sess, pdu, &handle_inform_response, NULL);
    } else {
        if ((sess->version == SNMP_VERSION_3) &&
                (pdu->command == SNMP_MSG_TRAP2) &&
                (sess->securityEngineIDLen == 0)) {
            u_char          tmp[SPRINT_MAX_LEN];

            int len = snmpv3_get_engineID(tmp, sizeof(tmp));
            pdu->securityEngineID = netsnmp_memdup(tmp, len);
            pdu->securityEngineIDLen = len;
        }

        result = snmp_async_send(sess, pdu, &handle_trap_callback, NULL);
    }

    if (result == 0) {
        snmp_sess_perror("snmpd: send_trap", sess);
        snmp_free_pdu(pdu);
        /** trap stats for failure handled in callback */
    } else {
        snmp_increment_statistic(STAT_SNMPOUTTRAPS);
        snmp_increment_statistic(STAT_SNMPOUTPKTS);
#ifndef NETSNMP_NO_TRAP_STATS
        if (sess->trap_stats) {
            sess->trap_stats->sent_last_sent = netsnmp_get_agent_uptime();
            ++sess->trap_stats->sent_count;
            _dump_trap_stats(sess);
        }
#endif /* NETSNMP_NO_TRAP_STATS */
    }
}

void
send_trap_vars(int trap, int specific, netsnmp_variable_list * vars)
{
    if (trap == SNMP_TRAP_ENTERPRISESPECIFIC)
        send_enterprise_trap_vars(trap, specific, objid_enterprisetrap,
                                  OID_LENGTH(objid_enterprisetrap), vars);
    else
        send_enterprise_trap_vars(trap, specific, trap_version_id,
                                  OID_LENGTH(trap_version_id), vars);
}

#ifndef NETSNMP_FEATURE_REMOVE_TRAP_VARS_WITH_CONTEXT
/* Send a trap under a context */
void send_trap_vars_with_context(int trap, int specific, 
              netsnmp_variable_list *vars, const char *context)
{
    if (trap == SNMP_TRAP_ENTERPRISESPECIFIC)
        netsnmp_send_traps(trap, specific, objid_enterprisetrap,
                                  OID_LENGTH(objid_enterprisetrap), vars,
								  context, 0);
    else
        netsnmp_send_traps(trap, specific, trap_version_id,
                                  OID_LENGTH(trap_version_id), vars, 
								  context, 0);
    	
}
#endif /* NETSNMP_FEATURE_REMOVE_TRAP_VARS_WITH_CONTEXT */

/**
 * Sends an SNMPv1 trap (or the SNMPv2 equivalent) to the list of  
 * configured trap destinations (or "sinks"), using the provided 
 * values for the generic trap type and specific trap value.
 *
 * This function eventually calls send_enterprise_trap_vars.  If the
 * trap type is not set to SNMP_TRAP_ENTERPRISESPECIFIC the enterprise 
 * and enterprise_length paramater is set to the pre defined NETSNMP_SYSTEM_MIB 
 * oid and length respectively.  If the trap type is set to 
 * SNMP_TRAP_ENTERPRISESPECIFIC the enterprise and enterprise_length 
 * parameters are set to the pre-defined NETSNMP_NOTIFICATION_MIB oid and length 
 * respectively.
 *
 * @param trap is the generic trap type.
 *
 * @param specific is the specific trap value.
 *
 * @return void
 *
 * @see send_enterprise_trap_vars
 * @see send_v2trap
 */
       	
void
send_easy_trap(int trap, int specific)
{
    send_trap_vars(trap, specific, NULL);
}

/**
 * Uses the supplied list of variable bindings to form an SNMPv2 trap, 
 * which is sent to SNMPv2-capable sinks  on  the  configured  list.  
 * An equivalent INFORM is sent to the configured list of inform sinks.  
 * Sinks that can only handle SNMPv1 traps are skipped.
 *
 * This function eventually calls send_enterprise_trap_vars.  If the
 * trap type is not set to SNMP_TRAP_ENTERPRISESPECIFIC the enterprise 
 * and enterprise_length paramater is set to the pre defined NETSNMP_SYSTEM_MIB 
 * oid and length respectively.  If the trap type is set to 
 * SNMP_TRAP_ENTERPRISESPECIFIC the enterprise and enterprise_length 
 * parameters are set to the pre-defined NETSNMP_NOTIFICATION_MIB oid and length 
 * respectively.
 *
 * @param vars is used to supply list of variable bindings to form an SNMPv2 
 *	trap.
 *
 * @return void
 *
 * @see send_easy_trap
 * @see send_enterprise_trap_vars
 */

void
send_v2trap(netsnmp_variable_list * vars)
{
    send_trap_vars(-1, -1, vars);
}

/**
 * Similar to send_v2trap(), with the added ability to specify a context.  If
 * the last parameter is NULL, then this call is equivalent to send_v2trap().
 *
 * @param vars is used to supply the list of variable bindings for the trap.
 * 
 * @param context is used to specify the context of the trap.
 *
 * @return void
 *
 * @see send_v2trap
 */
#ifndef NETSNMP_FEATURE_REMOVE_SEND_V3TRAP
void send_v3trap(netsnmp_variable_list *vars, const char *context)
{
    netsnmp_send_traps(-1, -1, 
                       trap_version_id, OID_LENGTH(trap_version_id),
                       vars, context, 0);
}
#endif /* NETSNMP_FEATURE_REMOVE_SEND_V3TRAP */

#ifndef NETSNMP_FEATURE_REMOVE_SEND_TRAP_PDU
void
send_trap_pdu(netsnmp_pdu *pdu)
{
    send_trap_vars(-1, -1, pdu->variables);
}
#endif /* NETSNMP_FEATURE_REMOVE_SEND_TRAP_PDU */



        /*******************
	 *
	 * Config file handling
	 *
	 *******************/

void
snmpd_parse_config_authtrap(const char *token, char *cptr)
{
    int             i;

    i = atoi(cptr);
    if (i == 0) {
        if (strcmp(cptr, "enable") == 0) {
            i = SNMP_AUTHENTICATED_TRAPS_ENABLED;
        } else if (strcmp(cptr, "disable") == 0) {
            i = SNMP_AUTHENTICATED_TRAPS_DISABLED;
        }
    }
    if (i < 1 || i > 2) {
        config_perror("authtrapenable must be 1 or 2");
    } else {
        if (strcmp(token, "pauthtrapenable") == 0) {
            if (snmp_enableauthentrapsset < 0) {
                /*
                 * This is bogus (and shouldn't happen anyway) -- the value
                 * of snmpEnableAuthenTraps.0 is already configured
                 * read-only.  
                 */
                snmp_log(LOG_WARNING,
                         "ignoring attempted override of read-only snmpEnableAuthenTraps.0\n");
                return;
            } else {
                snmp_enableauthentrapsset++;
            }
        } else {
            if (snmp_enableauthentrapsset > 0) {
                /*
                 * This is bogus (and shouldn't happen anyway) -- we already
                 * read a persistent value of snmpEnableAuthenTraps.0, which
                 * we should ignore in favour of this one.  
                 */
                snmp_log(LOG_WARNING,
                         "ignoring attempted override of read-only snmpEnableAuthenTraps.0\n");
                /*
                 * Fall through and copy in this value.  
                 */
            }
            snmp_enableauthentrapsset = -1;
        }
        snmp_enableauthentraps = i;
    }
}

#if !defined(NETSNMP_DISABLE_SNMPV1) || !defined(NETSNMP_DISABLE_SNMPV2C)
static void
_parse_config_sink(const char *token, char *cptr, int version, int type)
{
    char           *sp, *cp, *pp = NULL, *src = NULL;
    char           *st, *name = NULL, *tag = NULL, *profile = NULL;
    int            done = 0;

    if (!snmp_trapcommunity)
        snmp_trapcommunity = strdup("public");
    sp = strtok_r(cptr, " \t\n", &st);
    /*
     * check for optional arguments
     */
    do {
        if (*sp != '-') {
            done = 1;
            continue;
        }
        if (strcmp(sp, "-name") == 0)
            name = strtok_r(NULL, " \t\n", &st);
        else if (strcmp(sp, "-tag") == 0)
            tag = strtok_r(NULL, " \t\n", &st);
        else if (strcmp(sp, "-profile") == 0)
            profile = strtok_r(NULL, " \t\n", &st);
        else if (strcmp(sp, "-s") == 0)
            src = strtok_r(NULL, " \t\n", &st);
        else
            netsnmp_config_warn("ignoring unknown argument: %s", sp);
        sp = strtok_r(NULL, " \t\n", &st);
    } while (!done);
    cp = strtok_r(NULL, " \t\n", &st);
    if (cp)
        pp = strtok_r(NULL, " \t\n", &st);
    if (pp)
        config_pwarn("The separate port argument for sinks is deprecated");
    if (netsnmp_create_v1v2_notification_session(sp, pp,
                                                 cp ? cp : snmp_trapcommunity,
                                                 src, version, type, name, tag,
                                                 profile) == NULL) {
        netsnmp_config_error("cannot create sink: %s", cptr);
    }
}
#endif

#ifndef NETSNMP_DISABLE_SNMPV1
void
snmpd_parse_config_trapsink(const char *token, char *cptr)
{
    _parse_config_sink(token, cptr, SNMP_VERSION_1, SNMP_MSG_TRAP);
}
#endif

#ifndef NETSNMP_DISABLE_SNMPV2C
void
snmpd_parse_config_trap2sink(const char *word, char *cptr)
{
    _parse_config_sink(word, cptr, SNMP_VERSION_2c, SNMP_MSG_TRAP2);
}

void
snmpd_parse_config_informsink(const char *word, char *cptr)
{
    _parse_config_sink(word, cptr, SNMP_VERSION_2c, SNMP_MSG_INFORM);
}
#endif

/*
 * this must be standardized somewhere, right? 
 */
#define MAX_ARGS 128

static int      traptype;

static void
trapOptProc(int argc, char *const *argv, int opt)
{
    switch (opt) {
    case 'C':
        while (*optarg) {
            switch (*optarg++) {
            case 'i':
                traptype = SNMP_MSG_INFORM;
                break;
            default:
                config_perror("unknown argument passed to -C");
                break;
            }
        }
        break;
    }
}

netsnmp_session *
netsnmp_create_v3user_notification_session(const char *dest, const char *user,
                                           int level, const char *context,
                                           int pdutype, const u_char *engineId,
                                           size_t engineId_len, const char *src,
                                           const char *notif_name,
                                           const char *notif_tag,
                                           const char* notif_profile)
{
    netsnmp_session    session, *ss = NULL;
    struct usmUser    *usmUser;
    netsnmp_tdomain_spec tspec;
    netsnmp_transport *transport;
    u_char             tmp_engineId[SPRINT_MAX_LEN];
    int                rc;

    if (netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID,
                               NETSNMP_DS_LIB_DISABLE_V3)) {
        netsnmp_config_error("SNMPv3 disabled, cannot create notification session");
        return NULL;
    }
    if (NULL == dest || NULL == user)
        return NULL;

    /** authlevel */
    if ((SNMP_SEC_LEVEL_AUTHPRIV != level) &&
        (SNMP_SEC_LEVEL_AUTHNOPRIV != level) &&
        (SNMP_SEC_LEVEL_NOAUTH != level)) {
        DEBUGMSGTL(("trap:v3user_notif_sess", "bad level %d\n", level));
        return NULL;
    }

    /** need engineId to look up users */
    if (NULL == engineId) {
        engineId_len = snmpv3_get_engineID( tmp_engineId, sizeof(tmp_engineId));
        engineId = tmp_engineId;
    }

    usmUser = usm_get_user(NETSNMP_REMOVE_CONST(u_char *,engineId),
                           engineId_len, NETSNMP_REMOVE_CONST(char *,user));
    if (NULL == usmUser) {
        DEBUGMSGTL(("trap:v3user_notif_sess", "usmUser %s not found\n", user));
        return NULL;
    }

    snmp_sess_init(&session);

    session.version = SNMP_VERSION_3;

    session.peername = NETSNMP_REMOVE_CONST(char*,dest);

    session.securityName = NETSNMP_REMOVE_CONST(char*,user);
    session.securityNameLen = strlen(user);

    if (NULL != context) {
        session.contextName = NETSNMP_REMOVE_CONST(char*,context);
        session.contextNameLen = strlen(context);
    }

    session.securityLevel = level;

    /** auth prot */
    if (NULL != usmUser->authProtocol) {
        session.securityAuthProto =
            snmp_duplicate_objid(usmUser->authProtocol,
                                 usmUser->authProtocolLen);
        session.securityAuthProtoLen = usmUser->authProtocolLen;
        if (NULL == session.securityAuthProto)
            goto bail;
    }

    /** authkey */
    if (((SNMP_SEC_LEVEL_AUTHPRIV == level) ||
         (SNMP_SEC_LEVEL_AUTHNOPRIV == level)) &&
        (usmUser->flags & USMUSER_FLAG_KEEP_MASTER_KEY)) {
        netsnmp_assert(usmUser->authKeyKuLen > 0);
        memcpy(session.securityAuthKey, usmUser->authKeyKu,
               usmUser->authKeyKuLen);
        session.securityAuthKeyLen = usmUser->authKeyKuLen;
    }

    /** priv prot */
    if (NULL != usmUser->privProtocol) {
        session.securityPrivProto =
            snmp_duplicate_objid(usmUser->privProtocol,
                                 usmUser->privProtocolLen);
        session.securityPrivProtoLen = usmUser->privProtocolLen;
        if (NULL == session.securityPrivProto)
            goto bail;
    }

    /** privkey */
    if ((SNMP_SEC_LEVEL_AUTHPRIV == level)  &&
        (usmUser->flags & USMUSER_FLAG_KEEP_MASTER_KEY)) {
        netsnmp_assert(usmUser->privKeyKuLen > 0);
        memcpy(session.securityPrivKey, usmUser->privKeyKu,
               usmUser->privKeyKuLen);
        session.securityPrivKeyLen = usmUser->privKeyKuLen;
    }

    /** engineId */
    session.contextEngineID = netsnmp_memdup(usmUser->engineID,
                                             usmUser->engineIDLen);
    session.contextEngineIDLen = usmUser->engineIDLen;

    /** open the tranport */

    memset(&tspec, 0, sizeof(netsnmp_tdomain_spec));
    tspec.application = "snmptrap";
    tspec.target = session.peername;
    tspec.default_domain = NULL;
    tspec.default_target = NULL;
    tspec.source = src;
    transport = netsnmp_tdomain_transport_tspec(&tspec);
    if (transport == NULL) {
        DEBUGMSGTL(("trap:v3user_notif_sess", "could not create transport\n"));
        goto bail;
    }

    if ((rc = netsnmp_sess_config_and_open_transport(&session, transport))
        != SNMPERR_SUCCESS) {
        DEBUGMSGTL(("trap:v3user_notif_sess", "config/open failed\n"));
        goto bail;
    }

    ss = snmp_add(&session, transport, NULL, NULL);
    if (!ss) {
        DEBUGMSGTL(("trap:v3user_notif_sess", "snmp_add failed\n"));
        goto bail;
    }

    if (netsnmp_add_notification_session(ss, pdutype,
                                         (pdutype == SNMP_MSG_INFORM),
                                         ss->version, notif_name, notif_tag,
                                         notif_profile) != 1) {
        DEBUGMSGTL(("trap:v3user_notif_sess", "add notification failed\n"));
        snmp_sess_close(ss);
        ss = NULL;
        goto bail;
    }

  bail:
    /** free any allocated mem in session */
    SNMP_FREE(session.securityAuthProto);
    SNMP_FREE(session.securityPrivProto);

    return ss;
}

void
snmpd_parse_config_trapsess(const char *word, char *cptr)
{
    char           *argv[MAX_ARGS], *cp = cptr;
    char           *profile = NULL, *name = NULL, *tag = NULL;
    int             argn, rc;
    netsnmp_session session, *ss;
    netsnmp_transport *transport;
    size_t          len;
    char            tmp[SPRINT_MAX_LEN];
    char           *clientaddr_save = NULL;

    /*
     * inform or trap?  default to trap 
     */
    traptype = SNMP_MSG_TRAP2;

    do {
        if (strncmp(cp, "-profile", 8) == 0) {
            cp = skip_token(cp);
            cp = copy_nword(cp, tmp, SPRINT_MAX_LEN);
            profile = strdup(tmp);
        } else if (strncmp(cp, "-name", 5) == 0) {
            cp = skip_token(cp);
            cp = copy_nword(cp, tmp, SPRINT_MAX_LEN);
            name = strdup(tmp);
        } else if (strncmp(cp, "-tag", 5) == 0) {
            cp = skip_token(cp);
            cp = copy_nword(cp, tmp, SPRINT_MAX_LEN);
            tag = strdup(tmp);
        } else
            break;
    } while(cp);

    /*
     * create the argv[] like array 
     */
    argv[0] = strdup("snmpd-trapsess"); /* bogus entry for getopt() */
    for (argn = 1; cp && argn < MAX_ARGS; argn++) {
        cp = copy_nword(cp, tmp, SPRINT_MAX_LEN);
        argv[argn] = strdup(tmp);
    }

    /** parse args (also initializes session) */
    netsnmp_parse_args(argn, argv, &session, "C:", trapOptProc,
                       NETSNMP_PARSE_ARGS_NOLOGGING |
                       NETSNMP_PARSE_ARGS_NOZERO);

    if (NETSNMP_RUNTIME_PROTOCOL_SKIP(session.version)) {
        config_perror("snmpd: protocol version disabled at runtime");
        for (; argn > 0; argn--)
            free(argv[argn - 1]);
        goto cleanup;
    }

    if (NULL != session.localname) {
        clientaddr_save = netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID,
                                                NETSNMP_DS_LIB_CLIENT_ADDR);
        if (clientaddr_save)
            clientaddr_save = strdup(clientaddr_save);
        netsnmp_ds_set_string(NETSNMP_DS_LIBRARY_ID,
                              NETSNMP_DS_LIB_CLIENT_ADDR,
                              session.localname);
    }

    transport = netsnmp_transport_open_client("snmptrap", session.peername);

    if (NULL != session.localname)
        netsnmp_ds_set_string(NETSNMP_DS_LIBRARY_ID,
                              NETSNMP_DS_LIB_CLIENT_ADDR, clientaddr_save);

    if (transport == NULL) {
        config_perror("snmpd: failed to parse this line.");
        for (; argn > 0; argn--)
            free(argv[argn - 1]);
        goto cleanup;
    }
    if ((rc = netsnmp_sess_config_and_open_transport(&session, transport))
        != SNMPERR_SUCCESS) {
        session.s_snmp_errno = rc;
        session.s_errno = 0;
        for (; argn > 0; argn--)
            free(argv[argn - 1]);
        goto cleanup;
    }
    ss = snmp_add(&session, transport, NULL, NULL);
    for (; argn > 0; argn--)
        free(argv[argn - 1]);

    if (!ss) {
        config_perror
            ("snmpd: failed to parse this line or the remote trap receiver is down.  Possible cause:");
        snmp_sess_perror("snmpd: snmpd_parse_config_trapsess()", &session);
        goto cleanup;
    }

    /*
     * If this is an SNMPv3 TRAP session, then the agent is
     *   the authoritative engine, so set the engineID accordingly
     */
    if (ss->version == SNMP_VERSION_3 &&
        traptype != SNMP_MSG_INFORM   &&
        ss->securityEngineIDLen == 0) {
            u_char          tmp[SPRINT_MAX_LEN];

            len = snmpv3_get_engineID( tmp, sizeof(tmp));
            ss->securityEngineID = netsnmp_memdup(tmp, len);
            ss->securityEngineIDLen = len;
    }

#ifndef NETSNMP_DISABLE_SNMPV1
    if ((ss->version == SNMP_VERSION_1) &&
        !netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID,
                                NETSNMP_DS_LIB_DISABLE_V1))
        traptype = SNMP_MSG_TRAP;
#endif
    netsnmp_add_notification_session(ss, traptype,
                                     (traptype == SNMP_MSG_INFORM),
                                     ss->version, name, tag, profile);

  cleanup:
    SNMP_FREE(clientaddr_save);
    SNMP_FREE(profile);
    SNMP_FREE(name);
    SNMP_FREE(tag);
}

#if !defined(NETSNMP_DISABLE_SNMPV1) || !defined(NETSNMP_DISABLE_SNMPV2C)
void
snmpd_parse_config_trapcommunity(const char *word, char *cptr)
{
    if (snmp_trapcommunity != NULL) {
        free(snmp_trapcommunity);
    }
    snmp_trapcommunity = (char *) malloc(strlen(cptr) + 1);
    if (snmp_trapcommunity != NULL) {
        copy_nword(cptr, snmp_trapcommunity, strlen(cptr) + 1);
    }
}

void
snmpd_free_trapcommunity(void)
{
    if (snmp_trapcommunity) {
        free(snmp_trapcommunity);
        snmp_trapcommunity = NULL;
    }
}
#endif
/** @} */
