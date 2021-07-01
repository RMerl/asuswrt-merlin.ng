/*
 * This file was created to separate data storage from  MIB implementation.
 *
 * Portions of this file are copyrighted by:
 * Copyright (c) 2016 VMware, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */


/*
 * This should always be included first before anything else
 */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-features.h>

#include <sys/types.h>
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

/*
 * minimal include directives
 */
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "agent_global_vars.h"
#include "header_complex.h"
#include "snmpNotifyTable_data.h"
#include "notification/snmpNotifyFilterProfileTable_data.h"
#include "target/snmpTargetParamsEntry_data.h"
#include "target/snmpTargetAddrEntry_data.h"
#include "target/target.h"
#include "agentx/subagent.h"
#include "snmp-notification-mib/snmpNotifyFilterTable/snmpNotifyFilterTable_data_storage.h"
#include <net-snmp/agent/agent_callbacks.h>
#include <net-snmp/agent/agent_trap.h>
#include <net-snmp/agent/mib_module_config.h>
#include "net-snmp/agent/sysORTable.h"

#ifdef USING_NOTIFICATION_LOG_MIB_NOTIFICATION_LOG_MODULE
#   include "notification-log-mib/notification_log.h"
#endif

SNMPCallback    store_snmpNotifyTable;

static int
_unregister_notification_cb(int major, int minor,
                            void *serverarg, void *clientarg);

/*
 * global storage of our data, saved in and configured by header_complex()
 */
static struct header_complex_index *snmpNotifyTableStorage = NULL;
static int _active = 0;

static int
_checkFilter(const char* paramName, netsnmp_pdu *pdu)
{
    /*
     * find appropriate filterProfileEntry
     */
    netsnmp_variable_list *var, *trap_var = NULL;
    char                  *profileName;
    size_t                 profileNameLen;
    struct vacm_viewEntry *vp, *head;
    int                    vb_oid_excluded = 0, free_trapvar = 0;

    netsnmp_assert(NULL != paramName);
    netsnmp_assert(NULL != pdu);

    DEBUGMSGTL(("send_notifications", "checking filters for '%s'...\n",
                paramName));

    /*
   A notification originator uses the snmpNotifyFilterTable to filter
   notifications.  A notification filter profile may be associated with
   a particular entry in the snmpTargetParamsTable.  The associated
   filter profile is identified by an entry in the
   snmpNotifyFilterProfileTable whose index is equal to the index of the
   entry in the snmpTargetParamsTable.  If no such entry exists in the
   snmpNotifyFilterProfileTable, no filtering is performed for that
   management target.
    */
    /** xxx paramName from session doesn't have length. will be
     * trouble with paramNames with embedded NULL */
    profileName = get_FilterProfileName(paramName, strlen(paramName),
                                        &profileNameLen);
    if (NULL == profileName) /* try default */
        profileName = get_FilterProfileName("default", 7, &profileNameLen);
    if (NULL == profileName) {
        DEBUGMSGTL(("send_notifications", "  no matching profile\n"));
        return 0;
    }

    /*
   If such an entry does exist, the value of snmpNotifyFilterProfileName
   of the entry is compared with the corresponding portion of the index
   of all active entries in the snmpNotifyFilterTable.  All such entries
   for which this comparison results in an exact match are used for
   filtering a notification generated using the associated
   snmpTargetParamsEntry.  If no such entries exist, no filtering is
   performed, and a notification may be sent to the management target.
    */
    head = snmpNotifyFilter_vacm_view_subtree(profileName);
    if (NULL == head) {
        DEBUGMSGTL(("send_notifications", "  no matching filters\n"));
        return 0;
    }

    /*
   Otherwise, if matching entries do exist, a notification may be sent
   if the NOTIFICATION-TYPE OBJECT IDENTIFIER of the notification (this
   is the value of the element of the variable bindings whose name is
   snmpTrapOID.0, i.e., the second variable binding) is specifically
   included, and none of the object instances to be included in the
   variable-bindings of the notification are specifically excluded by
   the matching entries.
     */
    if (NULL != pdu->variables) {
        trap_var = find_varbind_in_list( pdu->variables,
                                         snmptrap_oid, snmptrap_oid_len);
    }
#if !defined(NETSNMP_DISABLE_SNMPV1)
    else {
        /** snmpv1 pdus have no varbinds. build trapoid */
        oid                    enterprise[MAX_OID_LEN];
        size_t                 enterprise_len;
        enterprise_len = OID_LENGTH(enterprise);
        if ((netsnmp_build_trap_oid(pdu, enterprise, &enterprise_len)
             != SNMPERR_SUCCESS) ||
            !snmp_varlist_add_variable(&trap_var, snmptrap_oid,
                                       snmptrap_oid_len,
                                       ASN_OBJECT_ID, (u_char*)enterprise,
                                       enterprise_len*sizeof(oid))) {
            snmp_log(LOG_WARNING,
                     "checkFilter: failed to build snmpTrapOID varbind\n");
        } else
            free_trapvar = 1;
    }
#endif /* NETSNMP_DISABLE_SNMPV1 */

    if (NULL != trap_var) {
        /*
                             For a notification name, if none match,
   then the notification name is considered excluded, and the
   notification should not be sent to this management target.
         */
        vp = netsnmp_view_get(head, profileName, trap_var->val.objid,
                              trap_var->val_len / sizeof(oid), VACM_MODE_FIND);
        if ((NULL == vp) || (SNMP_VIEW_INCLUDED != vp->viewType)) {
            DEBUGMSGTL(("send_notifications", "  filtered (snmpTrapOID.0 "));
            DEBUGMSGOID(("send_notifications",trap_var->val.objid,
                         trap_var->val_len / sizeof(oid)));
            DEBUGMSG(("send_notifications", " not included)\n"));
            free(head);
            if (free_trapvar)
                snmp_free_varbind(trap_var);
            return 1;
        }
    }
    if (free_trapvar) {
        snmp_free_varbind(trap_var);
        trap_var = NULL;
    }

    /*
     * check varbinds
     */
    for(var = pdu->variables; var; var = var->next_variable) {
        /*
                                                               For an
   object instance, if none match, the object instance is considered
   included, and the notification may be sent to this management target.
         */

        if (var == trap_var) {
            continue;
        }

        vp = netsnmp_view_get(head, profileName, var->name,
                              var->name_length, VACM_MODE_FIND);
        if ((NULL != vp) && (SNMP_VIEW_EXCLUDED == vp->viewType)) {
            DEBUGMSGTL(("send_notifications","  filtered (varbind "));
            DEBUGMSGOID(("send_notifications",var->name, var->name_length));
            DEBUGMSG(("send_notifications", " excluded)\n"));
            vb_oid_excluded = 1;
            break;
        }
    }

    free(head);

    return vb_oid_excluded;
}

int
send_notifications(int major, int minor, void *serverarg, void *clientarg)
{
    struct header_complex_index *hptr;
    struct snmpNotifyTable_data *nptr;
    netsnmp_session *sess, *sptr;
    netsnmp_pdu    *template_pdu = (netsnmp_pdu *) serverarg;
    int             count = 0;

    DEBUGMSGTL(("send_notifications", "starting: pdu=%p, vars=%p\n",
                template_pdu, template_pdu->variables));

    for (hptr = snmpNotifyTableStorage; hptr; hptr = hptr->next) {
        nptr = (struct snmpNotifyTable_data *) hptr->data;
        if (nptr->snmpNotifyRowStatus != RS_ACTIVE)
            continue;
        if (!nptr->snmpNotifyTag)
            continue;

        sess = get_target_sessions(nptr->snmpNotifyTag, NULL, NULL);

        /*
         * filter appropriately, per section 6 of RFC 3413
         */

        for (sptr = sess; sptr; sptr = sptr->next) {
#ifndef NETSNMP_DISABLE_SNMPV1
            if (sptr->version == SNMP_VERSION_1 &&
                minor != SNMPD_CALLBACK_SEND_TRAP1) {
                continue;
            } else
#endif
            if (sptr->version == SNMP_VERSION_3
#ifndef NETSNMP_DISABLE_SNMPV2C
                 || sptr->version == SNMP_VERSION_2c
#endif
                    ) {
                if(minor != SNMPD_CALLBACK_SEND_TRAP2)
                    continue;
                if (nptr->snmpNotifyType == SNMPNOTIFYTYPE_INFORM) {
                    template_pdu->command = SNMP_MSG_INFORM;
                } else {
                    template_pdu->command = SNMP_MSG_TRAP2;
                }
            }
            if (sess->paramName) {
                int filter = _checkFilter(sess->paramName, template_pdu);
                if (filter)
                    continue;
            }
            send_trap_to_sess(sptr, template_pdu);
            ++count;
        } /* for(sptr) */
    } /* for(hptr) */

    DEBUGMSGTL(("send_notifications", "sent %d notifications\n", count));

#ifdef USING_NOTIFICATION_LOG_MIB_NOTIFICATION_LOG_MODULE
    if (count)
        log_notification(template_pdu, NULL);
#endif

    return 0;
}

#define MAX_ENTRIES 1024

int
notifyTable_register_notifications(int major, int minor,
                                   void *serverarg, void *clientarg)
{
    struct targetAddrTable_struct *ptr = NULL;
    struct targetParamTable_struct *pptr = NULL;
    struct snmpNotifyTable_data *nptr = NULL;
    int             confirm, i;
    char            buf[SNMP_MAXBUF_SMALL];
    netsnmp_transport *t = NULL;
    struct agent_add_trap_args *args =
        (struct agent_add_trap_args *) serverarg;
    netsnmp_session *ss;
    const char      *name, *tag, *notifyProfile;
    int             nameLen, tagLen, notifyProfileLen;

    if (!args || !(args->ss)) {
        return (0);
    }
    args->rc = SNMPERR_GENERR;
    confirm = args->confirm;
    ss = args->ss;
    name = args->nameData;
    nameLen = args->nameLen;
    tag = args->tagData;
    tagLen = args->tagLen;
    notifyProfile = args->profileData;
    notifyProfileLen = args->profileLen;

    /*
     * XXX: START move target creation to target code 
     */
    if (NULL == name) {
        int len;
        for (i = 0; i < MAX_ENTRIES; i++) {
            sprintf(buf, "internal%d", i);
            len = strlen(buf);
            if ((get_addrForName2(buf,len) == NULL) &&
                (get_paramEntry2(buf,len) == NULL))
                break;
        }
        if (i == MAX_ENTRIES) {
            snmp_log(LOG_ERR,
                     "Can't register new trap destination: max limit reached: %d",
                     MAX_ENTRIES);
            snmp_sess_close(ss);
            return (0);
        }
        name = buf;
        nameLen = len;
        if (NULL == tag) {
            tag = buf;
            tagLen = len;
        }
    } else {
        if (NULL == tag) {
            tag = name;
            tagLen = nameLen;
        }
    }

    /*
     * address
     */
    t = snmp_sess_transport(snmp_sess_pointer(ss));
    if (!t) {
        snmp_log(LOG_ERR,
                "Cannot add new trap destination, transport is closed.");
        snmp_sess_close(ss);
        return 0;
    }
    ptr = snmpTargetAddrTable_create();
    if (!ptr)
        goto bail;
    ptr->nameData = netsnmp_memdup_nt(name, nameLen, &ptr->nameLen);
    memcpy(ptr->tDomain, t->domain, t->domain_length * sizeof(oid));
    ptr->tDomainLen = t->domain_length;
    if (t->f_get_taddr)
        t->f_get_taddr(t, &ptr->tAddress, &ptr->tAddressLen);
    else
        netsnmp_assert(0);

    ptr->timeout = ss->timeout / 1000;
    ptr->retryCount = ss->retries;
    SNMP_FREE(ptr->tagListData);
    ptr->tagListData = netsnmp_memdup_nt(tag, tagLen, &ptr->tagListLen);
    /** link to target param table */
    ptr->paramsData = netsnmp_memdup_nt(name, nameLen, &ptr->paramsLen);
    if (!ptr->paramsData || !ptr->tagListData || !ptr->nameData)
        goto bail;
    ptr->storageType = ST_READONLY;
    ptr->rowStatus = RS_ACTIVE;
    ptr->sess = ss;
    DEBUGMSGTL(("trapsess", "adding %s to trap table\n", ptr->nameData));
    snmpTargetAddrTable_add(ptr);

    /*
     * param
     */
    pptr = snmpTargetParamTable_create();
    if (NULL == pptr)
        goto bail;
    /** link from target addr table */
    pptr->paramNameData = netsnmp_memdup_nt(ptr->paramsData, ptr->paramsLen,
                                            &pptr->paramNameLen);
    if (!pptr->paramNameData)
        goto bail;
    pptr->mpModel = ss->version;
    if (ss->version == SNMP_VERSION_3) {
        pptr->secModel = ss->securityModel;
        pptr->secLevel = ss->securityLevel;
        pptr->secNameData = netsnmp_memdup_nt(ss->securityName,
                                              ss->securityNameLen,
                                              &pptr->secNameLen);
        if (pptr->secNameData == NULL)
            goto bail;
    }
#if !defined(NETSNMP_DISABLE_SNMPV1) || !defined(NETSNMP_DISABLE_SNMPV2C)
       else {
        pptr->secModel =
#ifndef NETSNMP_DISABLE_SNMPV1
            ss->version == SNMP_VERSION_1 ?  SNMP_SEC_MODEL_SNMPv1 :
#endif
                                             SNMP_SEC_MODEL_SNMPv2c;
        pptr->secLevel = SNMP_SEC_LEVEL_NOAUTH;
        pptr->secNameData = NULL;
        if (ss->community && (ss->community_len > 0)) {
            pptr->secNameData = netsnmp_memdup_nt(ss->community,
                                                  ss->community_len,
                                                  &pptr->secNameLen);
            if (pptr->secNameData == NULL)
                goto bail;
        }
    }
#endif
    pptr->storageType = ST_READONLY;
    pptr->rowStatus = RS_ACTIVE;
    snmpTargetParamTable_add(pptr);
    /*
     * XXX: END move target creation to target code
     */

    /*
     * notify table
     */
    nptr = SNMP_MALLOC_STRUCT(snmpNotifyTable_data);
    if (nptr == NULL)
        goto bail;
    ++_active;
    nptr->snmpNotifyName = netsnmp_memdup_nt(name, nameLen,
                                             &nptr->snmpNotifyNameLen);
    /** selects target addr */
    nptr->snmpNotifyTag = netsnmp_memdup_nt(tag, tagLen,
                                            &nptr->snmpNotifyTagLen);
    if (!nptr->snmpNotifyName || !nptr->snmpNotifyTag)
        goto bail;
    nptr->snmpNotifyType = confirm ?
        SNMPNOTIFYTYPE_INFORM : SNMPNOTIFYTYPE_TRAP;
    nptr->snmpNotifyStorageType = ST_READONLY;
    nptr->snmpNotifyRowStatus = RS_ACTIVE;

    if (snmpNotifyTable_add(nptr) == SNMPERR_GENERR) {
        snmpNotifyTable_dispose(nptr);
        nptr = NULL;
        goto bail;
    }

    /*
     * filter profile
     */
    if (NULL != notifyProfile) {
        struct snmpNotifyFilterProfileTable_data *profile;
        profile = snmpNotifyFilterProfileTable_create(ptr->paramsData,
                                                      ptr->paramsLen,
                                                      notifyProfile,
                                                      notifyProfileLen);
        if (NULL == profile) {
            snmp_log(LOG_ERR, "couldn't create notify filter profile\n");
            goto bail;
        } else {
            profile->snmpNotifyFilterProfileRowStatus = RS_ACTIVE;
            profile->snmpNotifyFilterProfileStorType = ST_READONLY;

            if (snmpNotifyFilterProfileTable_add(profile) != SNMPERR_SUCCESS) {
                snmp_log(LOG_ERR, "couldn't add notify filter profile\n");
                snmpNotifyFilterProfileTable_free(profile);
            }
        }
    }

    args->rc = SNMPERR_SUCCESS;
    return 0;

  bail:
    snmp_log(LOG_ERR, "Cannot add new trap destination");

    if (NULL != nptr)
        snmpNotifyTable_remove(nptr);

    if (NULL != pptr)
        snmpTargetParamTable_remove(pptr);

    if (NULL != ptr)
        snmpTargetAddrTable_remove(ptr);

    snmp_sess_close(ss);

    return 0;
}

void
snmpNotifyTable_dispose(struct snmpNotifyTable_data *thedata)
{
    if (NULL == thedata)
        return;

    SNMP_FREE(thedata->snmpNotifyName);
    SNMP_FREE(thedata->snmpNotifyTag);
    free(thedata);
    --_active;
}

/*
 * XXX: this really needs to be done for the target mib entries too.
 * But we can only trust that we've added stuff here and we don't want
 * to destroy other valid entries in the target tables, so...  Don't
 * do too many kill -HUPs to your agent as re reading the config file
 * will be a slow memory leak in the target mib.
 */
int
notifyTable_unregister_all_notifications(int major, int minor,
                                         void *serverarg, void *clientarg)
{
    struct header_complex_index *hptr, *nhptr;

    for (hptr = snmpNotifyTableStorage; hptr; hptr = nhptr) {
        struct snmpNotifyTable_data *nptr = hptr->data;
        nhptr = hptr->next;
        if (nptr->snmpNotifyStorageType == ST_READONLY) {
            header_complex_extract_entry(&snmpNotifyTableStorage, hptr);
            snmpNotifyTable_dispose(nptr);
        }
    }
    snmpNotifyTableStorage = NULL;
    return (0);
}

/*
 * init_snmpNotifyTable_data():
 *   Initialization routine.  This is called when the agent starts up.
 */
void
init_snmpNotifyTable_data(void)
{
    static int done = 0;

    if (++done != 1) {
        DEBUGMSGTL(("snmpNotifyTable_data", "multiple init calls"));
        return;
    }

    DEBUGMSGTL(("snmpNotifyTable_data", "initializing...  "));

    /*
     * we need to be called back later to store our data 
     */
    snmp_register_callback(SNMP_CALLBACK_LIBRARY, SNMP_CALLBACK_STORE_DATA,
                           store_snmpNotifyTable, NULL);


#ifndef DISABLE_SNMPV1
    snmp_register_callback(SNMP_CALLBACK_APPLICATION,
                           SNMPD_CALLBACK_SEND_TRAP1, send_notifications,
                           NULL);
#endif
    snmp_register_callback(SNMP_CALLBACK_APPLICATION,
                           SNMPD_CALLBACK_SEND_TRAP2, send_notifications,
                           NULL);
    snmp_register_callback(SNMP_CALLBACK_APPLICATION,
                           SNMPD_CALLBACK_REGISTER_NOTIFICATIONS,
                           notifyTable_register_notifications, NULL);
    snmp_register_callback(SNMP_CALLBACK_APPLICATION,
                           SNMPD_CALLBACK_UNREGISTER_NOTIFICATIONS,
                           _unregister_notification_cb, NULL);
    snmp_register_callback(SNMP_CALLBACK_APPLICATION,
                           SNMPD_CALLBACK_PRE_UPDATE_CONFIG,
                           notifyTable_unregister_all_notifications, NULL);

    DEBUGMSGTL(("snmpNotifyTable_data", "done.\n"));
}

void
shutdown_snmpNotifyTable_data(void)
{
    DEBUGMSGTL(("snmpNotifyTable_data", "shutting down ... "));

    snmp_unregister_callback(SNMP_CALLBACK_LIBRARY, SNMP_CALLBACK_STORE_DATA,
                             store_snmpNotifyTable, NULL, FALSE);

    notifyTable_unregister_all_notifications(SNMP_CALLBACK_APPLICATION,
                                             SNMPD_CALLBACK_PRE_UPDATE_CONFIG,
                                             NULL, NULL);

    snmp_unregister_callback(SNMP_CALLBACK_APPLICATION,
                             SNMPD_CALLBACK_PRE_UPDATE_CONFIG,
                             notifyTable_unregister_all_notifications, NULL,
                             FALSE);
    snmp_unregister_callback(SNMP_CALLBACK_APPLICATION,
                             SNMPD_CALLBACK_REGISTER_NOTIFICATIONS,
                             notifyTable_register_notifications, NULL, FALSE);
    snmp_unregister_callback(SNMP_CALLBACK_APPLICATION,
                             SNMPD_CALLBACK_UNREGISTER_NOTIFICATIONS,
                             _unregister_notification_cb, NULL, FALSE);
    snmp_unregister_callback(SNMP_CALLBACK_APPLICATION,
                             SNMPD_CALLBACK_SEND_TRAP2, send_notifications,
                             NULL, FALSE);
#ifndef DISABLE_SNMPV1
    snmp_unregister_callback(SNMP_CALLBACK_APPLICATION,
                             SNMPD_CALLBACK_SEND_TRAP1, send_notifications,
                             NULL, FALSE);
#endif
    DEBUGMSGTL(("trap:notify:shutdown", "active count %d\n", _active));
    if (_active != 0) {
        DEBUGMSGTL(("trap:notify:shutdown",
                    "unexpected count %d after cleanup!\n",_active));
        snmp_log(LOG_WARNING,
                 "notify count %d, not 0, after shutdown.\n", _active);
    }

    DEBUGMSGTL(("snmpNotifyTable_data", "done.\n"));
}

/*
 * snmpNotifyTable_add(): adds a structure node to our data set
 */
int
snmpNotifyTable_add(struct snmpNotifyTable_data *thedata)
{
    netsnmp_variable_list *vars = NULL;
    int retVal;

    if (NULL == thedata)
        return SNMPERR_GENERR;

    DEBUGMSGTL(("snmpNotifyTable_data", "adding data...  "));
    /*
     * add the index variables to the varbind list, which is
     * used by header_complex to index the data. the allocated
     * variable will be freed by header_complex_maybe_add_data().
     */
    snmp_varlist_add_variable(&vars, NULL, 0, ASN_PRIV_IMPLIED_OCTET_STR, (u_char *) thedata->snmpNotifyName, thedata->snmpNotifyNameLen);      /* snmpNotifyName */

    if (header_complex_maybe_add_data(&snmpNotifyTableStorage, vars, thedata, 1)
        != NULL){
        DEBUGMSGTL(("snmpNotifyTable", "registered an entry\n"));
        retVal = SNMPERR_SUCCESS;
    }else{
        retVal = SNMPERR_GENERR;
    }


    DEBUGMSGTL(("snmpNotifyTable", "done.\n"));
    return retVal;
}

struct snmpNotifyTable_data *
snmpNotifyTable_extract(struct snmpNotifyTable_data *thedata)
{
    struct header_complex_index *hptr;

    hptr = header_complex_find_entry(snmpNotifyTableStorage, thedata);
    if (NULL == hptr)
        return NULL;

    return header_complex_extract_entry((struct header_complex_index**)
                                        &snmpNotifyTableStorage, hptr);
}

int
snmpNotifyTable_remove(struct snmpNotifyTable_data *thedata)
{
    struct snmpNotifyTable_data *nptr = snmpNotifyTable_extract(thedata);
    if (nptr) {
        snmpNotifyTable_dispose(nptr);
        return 1;
    }
    return 0;
}

struct snmpNotifyTable_data *
get_notifyTable2(const char *name, size_t nameLen)
{
    struct header_complex_index *hptr;

    for (hptr = snmpNotifyTableStorage; hptr; hptr = hptr->next) {
        struct snmpNotifyTable_data *nptr = hptr->data;
        if (nptr->snmpNotifyNameLen == nameLen && nptr->snmpNotifyName &&
            memcmp(nptr->snmpNotifyName, name, nameLen) == 0)
            return nptr;
    }
    return NULL;
}

struct snmpNotifyTable_data *
find_row_notifyTable(struct variable *vp, oid * name, size_t * len, int exact,
                    size_t * var_len, WriteMethod ** write_method)
{
    struct snmpNotifyTable_data *result =
        header_complex((struct header_complex_index *)
                       snmpNotifyTableStorage, vp, name, len, exact,
                       var_len, write_method);
    return result;
}

void
snmpNotifyTable_unregister_notification(const char *name, unsigned char nameLen)
{
    struct targetAddrTable_struct *ta = get_addrForName2(name,nameLen);
    struct targetParamTable_struct *tp = get_paramEntry2(name,nameLen);
    struct snmpNotifyTable_data *nt = get_notifyTable2(name,nameLen);
    struct snmpNotifyFilterProfileTable_data *fp =
        snmpNotifyFilterProfileTable_find(name, nameLen);

    DEBUGMSGTL(("trapsess", "removing %s from trap tables\n", name));

    if (NULL != nt)
        snmpNotifyTable_remove(nt);
    else
        DEBUGMSGTL(("snmpNotifyTable:unregister",
                    "No NotifyTable entry for %s\n", name));

    if (NULL != tp)
        snmpTargetParamTable_remove(tp);
    else
        DEBUGMSGTL(("snmpNotifyTable:unregister",
                    "No TargetParamTable entry for %s\n", name));

    if (NULL != ta)
        snmpTargetAddrTable_remove(ta);
    else
        DEBUGMSGTL(("snmpNotifyTable:unregister",
                    "No TargetAddrTable entry for %s\n", name));

    if (NULL != fp)
        snmpNotifyFilterProfileTable_remove(fp);
    else
        DEBUGMSGTL(("snmpNotifyTable:unregister",
                    "No FilterProfileTable entry for %s\n", name));

}

static int
_unregister_notification_cb(int major, int minor,
                            void *serverarg, void *clientarg)
{
    struct agent_add_trap_args *args =
        (struct agent_add_trap_args *) serverarg;

    if (!args || !(args->nameData))
        return 0;

    snmpNotifyTable_unregister_notification(args->nameData, args->nameLen);
    return 1;
}

/*
 * store_snmpNotifyTable():
 *   stores .conf file entries needed to configure the mib.
 */
int
store_snmpNotifyTable(int majorID, int minorID, void *serverarg,
                      void *clientarg)
{
    char            line[SNMP_MAXBUF];
    char           *cptr;
    size_t          tmpint;
    struct snmpNotifyTable_data *StorageTmp;
    struct header_complex_index *hcindex;


    DEBUGMSGTL(("snmpNotifyTable", "storing data...  "));


    for (hcindex = snmpNotifyTableStorage; hcindex != NULL;
         hcindex = hcindex->next) {
        StorageTmp = (struct snmpNotifyTable_data *) hcindex->data;

        /*
         * store permanent and nonvolatile rows.
         * XXX should there be a qualification on RowStatus??
         */
        if ((StorageTmp->snmpNotifyStorageType == ST_NONVOLATILE) ||
            (StorageTmp->snmpNotifyStorageType == ST_PERMANENT) ){

            memset(line, 0, sizeof(line));
            strcat(line, "snmpNotifyTable ");
            cptr = line + strlen(line);

            cptr =
                read_config_store_data(ASN_OCTET_STR, cptr,
                                       &StorageTmp->snmpNotifyName,
                                       &StorageTmp->snmpNotifyNameLen);
            cptr =
                read_config_store_data(ASN_OCTET_STR, cptr,
                                       &StorageTmp->snmpNotifyTag,
                                       &StorageTmp->snmpNotifyTagLen);
            cptr =
                read_config_store_data(ASN_INTEGER, cptr,
                                       &StorageTmp->snmpNotifyType,
                                       &tmpint);
            cptr =
                read_config_store_data(ASN_INTEGER, cptr,
                                       &StorageTmp->snmpNotifyStorageType,
                                       &tmpint);
            cptr =
                read_config_store_data(ASN_INTEGER, cptr,
                                       &StorageTmp->snmpNotifyRowStatus,
                                       &tmpint);

            snmpd_store_config(line);
        }
    }
    DEBUGMSGTL(("snmpNotifyTable", "done.\n"));
    return 0;
}

