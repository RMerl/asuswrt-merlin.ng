/*
 * snmpTargetAddrEntry MIB
 *
 * This file was created to separate notification data storage from
 * the MIB implementation.
 *
 * Portions of this file are copyrighted by:
 * Copyright (c) 2016 VMware, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */

#include <net-snmp/net-snmp-config.h>
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#include <stdlib.h>
#include <ctype.h>

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "snmpTargetAddrEntry_data.h"
#include "util_funcs/header_generic.h"

static struct targetAddrTable_struct *aAddrTable = NULL;
static int _active = 0;

/*
 * Utility routines
 */
static int store_snmpTargetAddrEntry(int majorID, int minorID, void *serverarg,
                                     void *clientarg);
static void snmpd_parse_config_targetAddr(const char *token, char *char_ptr);

struct targetAddrTable_struct *
get_addrTable(void)
{
    return aAddrTable;
}

struct targetAddrTable_struct *
get_addrForName2(const char *name, size_t nameLen)
{
    struct targetAddrTable_struct *ptr;
    for (ptr = aAddrTable; ptr != NULL; ptr = ptr->next) {
        if (ptr->nameLen == nameLen && ptr->nameData &&
            memcmp(ptr->nameData, name, nameLen) == 0)
            return ptr;
    }
    return NULL;
}


/*
 * TargetAddrTable_create creates and returns a pointer
 * to a targetAddrTable_struct with default values set
 */
struct targetAddrTable_struct *
snmpTargetAddrTable_create(void)
{
    struct targetAddrTable_struct *newEntry;

    newEntry = (struct targetAddrTable_struct *)
        calloc(1, sizeof(struct targetAddrTable_struct));

    if (newEntry) {
        ++_active;
        newEntry->timeout = 1500;
        newEntry->retryCount = 3;

        newEntry->tagListData = strdup("");
        newEntry->tagListLen = 0;

        newEntry->storageType = SNMP_STORAGE_NONVOLATILE;
        newEntry->rowStatus = SNMP_ROW_NONEXISTENT;
    }

    return newEntry;
}                               /* snmpTargetAddrTable_create */


/*
 * TargetAddrTable_dispose frees the space allocated to a
 * targetAddrTable_struct
 */
void
snmpTargetAddrTable_dispose(struct targetAddrTable_struct *reaped)
{
    if (NULL == reaped)
        return;

    if (reaped->sess)
        snmp_close(reaped->sess);
    else
        SNMP_FREE(reaped->tAddress);
    SNMP_FREE(reaped->nameData);
    SNMP_FREE(reaped->tagListData);
    SNMP_FREE(reaped->paramsData);

    SNMP_FREE(reaped);
    --_active;
}                               /* snmpTargetAddrTable_dispose  */

/*
 * snmpTargetAddrTable_addToList adds a targetAddrTable_struct
 * to a list passed in. The list is assumed to be in a sorted order,
 * low to high and this procedure inserts a new struct in the proper
 * location. Sorting uses OID values based on name. A new equal value
 * overwrites a current one.
 */
void
snmpTargetAddrTable_addToList(struct targetAddrTable_struct *newEntry,
                              struct targetAddrTable_struct **listPtr)
{
    static struct targetAddrTable_struct *curr_struct, *prev_struct;
    int             i;

    /*
     * if the list is empty, add the new entry to the top
     */
    if ((prev_struct = curr_struct = *listPtr) == NULL) {
        *listPtr = newEntry;
        return;
    } else {
        /*
         * search through the list for an equal or greater OID value
         */
        while (curr_struct != NULL) {
            i = netsnmp_compare_mem(newEntry->nameData,
                                    newEntry->nameLen,
                                    curr_struct->nameData,
                                    curr_struct->nameLen);
            if (i == 0) {       /* Exact match, overwrite with new struct */
                newEntry->next = curr_struct->next;
                /*
                 * if curr_struct is the top of the list
                 */
                if (*listPtr == curr_struct)
                    *listPtr = newEntry;
                else
                    prev_struct->next = newEntry;
                snmpTargetAddrTable_dispose(curr_struct);
                return;
            } else if (i < 0) { /* Found a greater OID, insert struct in front of it. */
                newEntry->next = curr_struct;
                /*
                 * if curr_struct is the top of the list
                 */
                if (*listPtr == curr_struct)
                    *listPtr = newEntry;
                else
                    prev_struct->next = newEntry;
                return;
            }
            prev_struct = curr_struct;
            curr_struct = curr_struct->next;
        }
    }
    /*
     * if we're here, no larger OID was ever found, insert on end of list
     */
    prev_struct->next = newEntry;
}                               /* snmpTargeAddrTable_addToList  */


void
snmpTargetAddrTable_add(struct targetAddrTable_struct *newEntry)
{
    snmpTargetAddrTable_addToList(newEntry, &aAddrTable);
}


/*
 * snmpTargetAddrTable_remFromList removes a targetAddrTable_struct
 * from the list passed in
 */
void
snmpTargetAddrTable_remFromList(struct targetAddrTable_struct *oldEntry,
                                struct targetAddrTable_struct **listPtr)
{
    struct targetAddrTable_struct *tptr;

    if ((tptr = *listPtr) == NULL)
        return;
    else if (tptr == oldEntry) {
        *listPtr = (*listPtr)->next;
        snmpTargetAddrTable_dispose(tptr);
        return;
    } else {
        while (tptr->next != NULL) {
            if (tptr->next == oldEntry) {
                tptr->next = tptr->next->next;
                snmpTargetAddrTable_dispose(oldEntry);
                return;
            }
            tptr = tptr->next;
        }
    }
}                               /* snmpTargetAddrTable_remFromList  */

void
snmpTargetAddrTable_remove(struct targetAddrTable_struct *entry)
{
    snmpTargetAddrTable_remFromList(entry, &aAddrTable);
}

/*
 * lookup OID in the link list of Addr Table Entries
 */
struct targetAddrTable_struct *
search_snmpTargetAddrTable(oid * baseName,
                           size_t baseNameLen,
                           oid * name, size_t * length, int exact)
{
    static struct targetAddrTable_struct *temp_struct;
    int             i;
    size_t          myOIDLen = 0;
    oid             newNum[128];

    /*
     * lookup entry in addrTable linked list, Get Current MIB ID
     */
    memcpy(newNum, baseName, baseNameLen * sizeof(oid));

    for (temp_struct = aAddrTable; temp_struct != NULL;
         temp_struct = temp_struct->next) {
        for (i = 0; i < temp_struct->nameLen; i++) {
            newNum[baseNameLen + i] = temp_struct->nameData[i];
        }
        myOIDLen = baseNameLen + i;
        i = snmp_oid_compare(name, *length, newNum, myOIDLen);
        /*
         * Assumes that the linked list sorted by OID, low to high
         */
        if ((i == 0 && exact != 0) || (i < 0 && exact == 0)) {
            if (exact == 0) {
                memcpy(name, newNum, myOIDLen * sizeof(oid));
                *length = myOIDLen;
            }
            return temp_struct;
        }
    }
    return NULL;
}                               /* search_snmpTargetAddrTable  */


/*
 * Init routines
 */

void
init_snmpTargetAddrEntry_data(void)
{
    static int done = 0;

    if (++done != 1)
        return;

    snmpd_register_config_handler("targetAddr",
                                  snmpd_parse_config_targetAddr,
                                  (void (*)(void))0, NULL);

    /*
     * we need to be called back later 
     */
    snmp_register_callback(SNMP_CALLBACK_LIBRARY, SNMP_CALLBACK_STORE_DATA,
                           store_snmpTargetAddrEntry, NULL);

}                               /* init_snmpTargetAddrEntry */


/*
 * Shutdown routines
 */

void
shutdown_snmpTargetAddrEntry_data(void)
{
    struct targetAddrTable_struct *ptr;
    struct targetAddrTable_struct *next;

    snmp_unregister_callback(SNMP_CALLBACK_LIBRARY, SNMP_CALLBACK_STORE_DATA,
                             store_snmpTargetAddrEntry, NULL, FALSE);

    DEBUGMSGTL(("trap:targetAddr:shutdown", "clearing %d object(s)\n",
                _active));
    for (ptr = aAddrTable; ptr; ptr = next) {
        next = ptr->next;
        snmpTargetAddrTable_dispose(ptr);
    }
    aAddrTable = NULL;

    DEBUGMSGTL(("trap:targetAddr:shutdown", "active count %d\n",_active));
    if (_active != 0) {
        DEBUGMSGTL(("trap:targetAddr:shutdown",
                    "unexpected count %d after cleanup!\n", _active));
        snmp_log(LOG_WARNING, "targetAddr count %d, not 0, after shutdown.\n",
                 _active);
    }
}

/*
 * store_snmpTargetAddrEntry handles the persistent storage proccess 
 * for this MIB table. It writes out all the non-volatile rows 
 * to permanent storage on a shutdown  
 */
static int
store_snmpTargetAddrEntry(int majorID, int minorID, void *serverarg,
                          void *clientarg)
{
    const struct targetAddrTable_struct *curr_struct;
    char            line[1024], *cur, *ep = line + sizeof(line);
    int             i;

    curr_struct = aAddrTable;
    while (curr_struct != NULL) {
        if ((curr_struct->storageType == SNMP_STORAGE_NONVOLATILE ||
             curr_struct->storageType == SNMP_STORAGE_PERMANENT) &&
            (curr_struct->rowStatus == SNMP_ROW_ACTIVE ||
             curr_struct->rowStatus == SNMP_ROW_NOTINSERVICE)) {
            cur = line + snprintf(line, sizeof(line), "targetAddr ");
            cur = read_config_save_octet_string(
                 cur, (const u_char*)curr_struct->nameData,
                 curr_struct->nameLen);
            *cur++ = ' ';
            for (i = 0; i < curr_struct->tDomainLen; i++) {
                cur += snprintf(cur, ep - cur, ".%i",
                                (int) curr_struct->tDomain[i]);
            }
            *cur++ = ' ';
            cur = read_config_save_octet_string(
                cur, curr_struct->tAddress, curr_struct->tAddressLen);
            cur += snprintf(cur, ep - cur, " %i %i \"%s\" %s %i %i",
                            curr_struct->timeout,
                            curr_struct->retryCount, curr_struct->tagListData,
                            curr_struct->paramsData, curr_struct->storageType,
                            curr_struct->rowStatus);
            line[ sizeof(line)-1 ] = 0;

            /*
             * store to file
             */
            snmpd_store_config(line);
        }

        curr_struct = curr_struct->next;
    }

    return SNMPERR_SUCCESS;
}                               /*  store_snmpTargetAddrEntry  */

static int
snmpTargetAddr_addTDomain(struct targetAddrTable_struct *entry, char *cptr)
{
    size_t          len = 128;

    if (cptr == NULL) {
        DEBUGMSGTL(("snmpTargetAddrEntry",
                    "ERROR snmpTargetAddrEntry: no tDomain in config string\n"));
        return (0);
    }

    if (!read_objid(cptr, entry->tDomain, &len)) {
        DEBUGMSGTL(("snmpTargetAddrEntry",
                    "ERROR snmpTargetAddrEntry: tDomain unreadable in config string\n"));
        return (0);
    }

    /*
     * spec check for oid 1-128 
     */
    if (len < 1 || len > 128) {
        DEBUGMSGTL(("snmpTargetAddrEntry",
                    "ERROR snmpTargetAddrEntry: tDomain out of range in config string\n"));
        return (0);
    }

    entry->tDomainLen = len;
    return (1);
}                               /* snmpTargetAddr_addTDomain */


static int
snmpTargetAddr_addTimeout(struct targetAddrTable_struct *entry, char *cptr)
{
    if (cptr == NULL) {
        DEBUGMSGTL(("snmpTargetAddrEntry",
                    "ERROR snmpTargetParamsEntry: no Timeout in config string\n"));
        return (0);
    } else if (!(isdigit((unsigned char)(*cptr)))) {
        DEBUGMSGTL(("snmpTargetAddrEntry",
                    "ERROR snmpTargeParamsEntry: Timeout is not a digit in config string\n"));
        return (0);
    }
    /*
     * check Timeout >= 0 
     */
    else if ((entry->timeout = (int) strtol(cptr, (char **) NULL, 0)) < 0) {
        DEBUGMSGTL(("snmpTargetAddrEntry",
                    "ERROR snmpTargeParamsEntry: Timeout out of range in config string\n"));
        return (0);
    }
    return (1);
}                               /* snmpTargetAddr_addTimeout  */


static int
snmpTargetAddr_addRetryCount(struct targetAddrTable_struct *entry,
                             char *cptr)
{
    if (cptr == NULL) {
        DEBUGMSGTL(("snmpTargetAddrEntry",
                    "ERROR snmpTargetParamsEntry: no Retry Count in config string\n"));
        return (0);
    } else if (!(isdigit((unsigned char)(*cptr)))) {
        DEBUGMSGTL(("snmpTargetAddrEntry",
                    "ERROR snmpTargeParamsEntry: Retry Count is not a digit in config string\n"));
        return (0);
    }
    /*
     * spec check 0..255 
     */
    else {
        entry->retryCount = (int) strtol(cptr, (char **) NULL, 0);
        if ((entry->retryCount < 0) || (entry->retryCount > 255)) {
            DEBUGMSGTL(("snmpTargetAddrEntry",
                        "ERROR snmpTargeParamsEntry: Retry Count is out of range in config string\n"));
            return (0);
        }
    }
    return (1);
}                               /* snmpTargetAddr_addRetryCount  */


static int
snmpTargetAddr_addTagList(struct targetAddrTable_struct *entry, char *cptr)
{
    if (cptr == NULL) {
        DEBUGMSGTL(("snmpTargetAddrEntry",
                    "ERROR snmpTargetAddrEntry: no tag list in config string\n"));
        return (0);
    } else {
        size_t len = strlen(cptr);
        /*
         * spec check for string 0-255 
         */
        if (len > 255) {
            DEBUGMSGTL(("snmpTargetAddrEntry",
                        "ERROR snmpTargetAddrEntry: tag list out of range in config string\n"));
            return (0);
        }
        SNMP_FREE(entry->tagListData);
        entry->tagListData = strdup(cptr);
        entry->tagListLen = strlen(cptr);
    }
    return (1);
}                               /* snmpTargetAddr_addTagList */


static int
snmpTargetAddr_addParams(struct targetAddrTable_struct *entry, char *cptr)
{
    size_t          len;
    if (cptr == NULL) {
        DEBUGMSGTL(("snmpTargetAddrEntry",
                    "ERROR snmpTargetAddrEntry: no params in config string\n"));
        return (0);
    } else {
        len = strlen(cptr);
        /*
         * spec check for string 1-32 
         */
        if (len < 1 || len > 32) {
            DEBUGMSGTL(("snmpTargetAddrEntry",
                        "ERROR snmpTargetAddrEntry: params out of range in config string\n"));
            return (0);
        }
        entry->paramsData = strdup(cptr);
        entry->paramsLen = strlen(cptr);
    }
    return (1);
}                               /* snmpTargetAddr_addParams */


static int
snmpTargetAddr_addStorageType(struct targetAddrTable_struct *entry,
                              char *cptr)
{
    if (cptr == NULL) {
        DEBUGMSGTL(("snmpTargetAddrEntry",
                    "ERROR snmpTargetAddrEntry: no storage type in config "
                    "string\n"));
        return (0);
    } else if (!(isdigit((unsigned char)(*cptr)))) {
        DEBUGMSGTL(("snmpTargetAddrEntry",
                    "ERROR snmpTargetAddrEntry: storage type is not a digit "
                    "in config string\n"));
        return (0);
    }
    /*
     * check that storage type is a possible value 
     */
    else if (((entry->storageType = (int) strtol(cptr, (char **) NULL, 0))
              != SNMP_STORAGE_OTHER) &&
             (entry->storageType != SNMP_STORAGE_VOLATILE) &&
             (entry->storageType != SNMP_STORAGE_NONVOLATILE) &&
             (entry->storageType != SNMP_STORAGE_PERMANENT) &&
             (entry->storageType != SNMP_STORAGE_READONLY)) {
        DEBUGMSGTL(("snmpTargetAddrEntry",
                    "ERROR snmpTargetAddrEntry: storage type not a valid "
                    "value of other(%d), volatile(%d), nonvolatile(%d), "
                    "permanent(%d), or readonly(%d) in config string.\n",
                    SNMP_STORAGE_OTHER, SNMP_STORAGE_VOLATILE,
                    SNMP_STORAGE_NONVOLATILE, SNMP_STORAGE_PERMANENT,
                    SNMP_STORAGE_READONLY));
        return (0);
    }
    return (1);
}                               /* snmpTargetAddr_addStorageType */


static int
snmpTargetAddr_addRowStatus(struct targetAddrTable_struct *entry,
                            char *cptr)
{
    if (cptr == NULL) {
        DEBUGMSGTL(("snmpTargetAddrEntry",
                    "ERROR snmpTargetAddrEntry: no Row Status in config "
                    "string\n"));
        return (0);
    } else if (!(isdigit((unsigned char)(*cptr)))) {
        DEBUGMSGTL(("snmpTargetAddrEntry",
                    "ERROR snmpTargetAddrEntry: Row Status is not a digit in "
                    "config string\n"));
        return (0);
    }
    /*
     * check that row status is a valid value 
     */
    else if (((entry->rowStatus = (int) strtol(cptr, (char **) NULL, 0))
              != SNMP_ROW_ACTIVE) &&
             (entry->rowStatus != SNMP_ROW_NOTINSERVICE) &&
             (entry->rowStatus != SNMP_ROW_NOTREADY)) {
        DEBUGMSGTL(("snmpTargetAddrEntry",
                    "ERROR snmpTargetAddrEntry: Row Status is not a valid "
                    "value of active(%d), notinservice(%d), or notready(%d) "
                    "in config string.\n",
                    SNMP_ROW_ACTIVE, SNMP_ROW_NOTINSERVICE, SNMP_ROW_NOTREADY));
        return (0);
    }
    return (1);
}                               /* snmpTargetAddr_addRowStatus  */



static void
snmpd_parse_config_targetAddr(const char *token, char *char_ptr)
{
    const char     *cptr = char_ptr;
    char            buff[1024], *bptr;
    struct targetAddrTable_struct *newEntry;
    int             i;
    size_t          bufl;

    newEntry = snmpTargetAddrTable_create();

    cptr = skip_white_const(cptr);
    if (cptr == NULL) {
        DEBUGMSGTL(("snmpTargetAddrEntry",
                    "ERROR snmpTargetAddrEntry: no name in config string\n"));
        snmpTargetAddrTable_dispose(newEntry);
        return;
    }

    bufl = 0;
    cptr = read_config_read_octet_string_const(cptr,
                                               (u_char**)&newEntry->nameData,
                                               &bufl);
    if (bufl < 1 || bufl > 32) {
        DEBUGMSGTL(("snmpTargetAddrEntry",
                    "ERROR snmpTargetAddrEntry: name out of range in config "
                    "string\n"));
        snmpTargetAddrTable_dispose(newEntry);
        return;
    }
    newEntry->nameLen = bufl;

    cptr = copy_nword_const(cptr, buff, sizeof(buff));
    if (snmpTargetAddr_addTDomain(newEntry, buff) == 0) {
        snmpTargetAddrTable_dispose(newEntry);
        return;
    }
    cptr =
        read_config_read_octet_string_const(cptr,
                                      (u_char **) & newEntry->tAddress,
                                      &newEntry->tAddressLen);
    if (!cptr || !newEntry->tAddress || (newEntry->tAddressLen < 1) ||
        (newEntry->tAddressLen > 32)) {
        DEBUGMSGTL(("snmpTargetAddrEntry",
                    "ERROR snmpTargetAddrEntry: no/bd TAddress in config string\n"));
        snmpTargetAddrTable_dispose(newEntry);
        return;
    }
    cptr = copy_nword_const(cptr, buff, sizeof(buff));
    if (snmpTargetAddr_addTimeout(newEntry, buff) == 0) {
        snmpTargetAddrTable_dispose(newEntry);
        return;
    }
    cptr = copy_nword_const(cptr, buff, sizeof(buff));
    if (snmpTargetAddr_addRetryCount(newEntry, buff) == 0) {
        snmpTargetAddrTable_dispose(newEntry);
        return;
    }
    cptr = copy_nword_const(cptr, buff, sizeof(buff));
    if (snmpTargetAddr_addTagList(newEntry, buff) == 0) {
        snmpTargetAddrTable_dispose(newEntry);
        return;
    }
    cptr = copy_nword_const(cptr, buff, sizeof(buff));
    if (snmpTargetAddr_addParams(newEntry, buff) == 0) {
        snmpTargetAddrTable_dispose(newEntry);
        return;
    }
    cptr = copy_nword_const(cptr, buff, sizeof(buff));
    if (snmpTargetAddr_addStorageType(newEntry, buff) == 0) {
        snmpTargetAddrTable_dispose(newEntry);
        return;
    }
    cptr = copy_nword_const(cptr, buff, sizeof(buff));
    if (snmpTargetAddr_addRowStatus(newEntry, buff) == 0) {
        snmpTargetAddrTable_dispose(newEntry);
        return;
    }
    bptr = buff;
    bptr += sprintf(bptr, "snmp_parse_config_targetAddr, read: ");
    bptr = read_config_save_octet_string(bptr, (u_char*)newEntry->nameData,
                                         newEntry->nameLen);
    *bptr++ = '\n';
    for (i = 0; i < newEntry->tDomainLen; i++) {
        bptr += snprintf(bptr, buff + sizeof(buff) - bptr,
                         ".%d", (int) newEntry->tDomain[i]);
    }
    bptr += snprintf(bptr, buff + sizeof(buff) - bptr,
                     " %s %d %d %s %s %d %d\n",
                     newEntry->tAddress, newEntry->timeout,
                     newEntry->retryCount, newEntry->tagListData,
                     newEntry->paramsData, newEntry->storageType,
                     newEntry->rowStatus);
    buff[ sizeof(buff) - 1 ] = 0;
    DEBUGMSGTL(("snmpTargetAddrEntry", "%s", buff));

    snmpTargetAddrTable_addToList(newEntry, &aAddrTable);
}                               /* snmpd_parse_config_target */

