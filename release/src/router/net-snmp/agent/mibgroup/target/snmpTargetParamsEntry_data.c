/*
 * TargetParamTable data storage
 *
 * This file was created to separate data storage from the MIB implementation
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

#include "snmpTargetParamsEntry_data.h"

static struct targetParamTable_struct *aPTable = NULL;
static int _active = 0;

/*
 * Utility routines
 */
static int
store_snmpTargetParamsEntry(int majorID, int minorID, void *serverarg,
                            void *clientarg);

/*
 * TargetParamTable_create creates and returns a pointer
 * to a targetParamTable_struct with default values set
 */
struct targetParamTable_struct
               *
snmpTargetParamTable_create(void)
{
    struct targetParamTable_struct *newEntry;

    newEntry = (struct targetParamTable_struct *)
        calloc(1, sizeof(struct targetParamTable_struct));

    if (NULL == newEntry)
        return NULL;

    ++_active;

    newEntry->mpModel = -1;

    newEntry->secModel = -1;
    newEntry->secLevel = -1;

    newEntry->storageType = SNMP_STORAGE_NONVOLATILE;
    newEntry->rowStatus = SNMP_ROW_NONEXISTENT;

    return newEntry;
}


/*
 * TargetParamTable_dispose frees the space allocated to a
 * targetParamTable_struct
 */
void
snmpTargetParamTable_dispose(struct targetParamTable_struct *reaped)
{
    SNMP_FREE(reaped->paramNameData);
    SNMP_FREE(reaped->secNameData);

    free(reaped);
    --_active;
}                               /* snmpTargetParamTable_dispose  */


/*
 * snmpTargetParamTable_addToList adds a targetParamTable_struct
 * to a list passed in. The list is assumed to be in a sorted order,
 * low to high and this procedure inserts a new struct in the proper
 * location. Sorting uses OID values based on paramName. A new equal value
 * overwrites a current one.
 */
void
snmpTargetParamTable_addToList(struct targetParamTable_struct *newEntry,
                               struct targetParamTable_struct **listPtr)
{
    static struct targetParamTable_struct *curr_struct, *prev_struct;
    int             i;

    snmp_store_needed(NULL);

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
            i = netsnmp_compare_mem(newEntry->paramNameData,
                                    newEntry->paramNameLen,
                                    curr_struct->paramNameData,
                                    curr_struct->paramNameLen);
            if (i == 0) {       /* Exact match, overwrite with new struct */
                newEntry->next = curr_struct->next;
                /*
                 * if curr_struct is the top of the list
                 */
                if (*listPtr == curr_struct)
                    *listPtr = newEntry;
                else
                    prev_struct->next = newEntry;
                snmpTargetParamTable_dispose(curr_struct);
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
}                               /* snmpTargeParamTable_addToList  */

void
snmpTargetParamTable_add(struct targetParamTable_struct *newEntry)
{
    snmpTargetParamTable_addToList(newEntry, &aPTable);
}

/*
 * snmpTargetParamTable_remFromList removes a targetParamTable_struct
 * from the list passed in
 */
void
snmpTargetParamTable_remFromList(struct targetParamTable_struct *oldEntry,
                                 struct targetParamTable_struct **listPtr)
{
    struct targetParamTable_struct *tptr;

    if ((tptr = *listPtr) == NULL)
        return;
    else if (tptr == oldEntry) {
        *listPtr = (*listPtr)->next;
        snmpTargetParamTable_dispose(tptr);
        snmp_store_needed(NULL);
    } else {
        while (tptr->next != NULL) {
            if (tptr->next == oldEntry) {
                tptr->next = tptr->next->next;
                snmpTargetParamTable_dispose(oldEntry);
                snmp_store_needed(NULL);
                break;
            }
            tptr = tptr->next;
        }
    }
}                               /* snmpTargetParamTable_remFromList  */

void
snmpTargetParamTable_remove(struct targetParamTable_struct *entry)
{
    snmpTargetParamTable_remFromList(entry, &aPTable);
}

/*
 * lookup OID in the link list of Table Entries
 */
struct targetParamTable_struct *
search_snmpTargetParamsTable(oid * baseName,
                             size_t baseNameLen,
                             oid * name, size_t * length, int exact)
{
    static struct targetParamTable_struct *temp_struct;
    int             i;
    size_t          myOIDLen = 0;
    oid             newNum[128];

    /*
     * lookup entry in p / * Get Current MIB ID
     */
    memcpy(newNum, baseName, baseNameLen * sizeof(oid));

    for (temp_struct = aPTable; temp_struct != NULL;
         temp_struct = temp_struct->next) {
        for (i = 0; i < temp_struct->paramNameLen; i++) {
            newNum[baseNameLen + i] = temp_struct->paramNameData[i];
        }
        myOIDLen = baseNameLen + temp_struct->paramNameLen;
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
}                               /* search_snmpTargetParamsTable */


struct targetParamTable_struct *
get_paramEntry2(const char *name, size_t nameLen)
{
    static struct targetParamTable_struct *ptr;
    for (ptr = aPTable; ptr; ptr = ptr->next) {
        if (nameLen == ptr->paramNameLen &&
            memcmp(ptr->paramNameData, name, nameLen) == 0) {
            return ptr;
        }
    }
    return NULL;
}


/*
 * initialization routines
 */

void
init_snmpTargetParamsEntry_data(void)
{
    static int done = 0;

    if (++done != 1)
        return;

    /*
     * we need to be called back later 
     */
    snmp_register_callback(SNMP_CALLBACK_LIBRARY, SNMP_CALLBACK_STORE_DATA,
                           store_snmpTargetParamsEntry, NULL);

}                               /*  init_snmpTargetParmsEntry  */


/*
 * shutdown routines
 */

void
shutdown_snmpTargetParamsEntry_data(void)
{
    DEBUGMSGTL(("trap:targetParam:shutdown", "clearing %d object(s)\n",
                _active));

    while (aPTable)
        snmpTargetParamTable_remFromList(aPTable, &aPTable);

    DEBUGMSGTL(("trap:targetParam:shutdown", "active count %d\n", _active));
    if (_active != 0) {
        DEBUGMSGTL(("trap:targetParam:shutdown",
                    "unexpected count %d after cleanup!\n", _active));
        snmp_log(LOG_WARNING, "targetAddr count %d, not 0, after shutdown.\n",
                 _active);
    }

}

/*
 * store_snmpTargetParamsEntry handles the presistent storage proccess 
 * for this MIB table. It writes out all the non-volatile rows 
 * to permanent storage on a shutdown  
 */
static int
store_snmpTargetParamsEntry(int majorID, int minorID, void *serverarg,
                            void *clientarg)
{
    struct targetParamTable_struct *curr_struct;
    char            line[1024];

    strcpy(line, "");
    if ((curr_struct = aPTable) != NULL) {
        while (curr_struct != NULL) {
            if ((curr_struct->storageType == SNMP_STORAGE_NONVOLATILE ||
                 curr_struct->storageType == SNMP_STORAGE_PERMANENT)
                &&
                (curr_struct->rowStatus == SNMP_ROW_ACTIVE ||
                 curr_struct->rowStatus == SNMP_ROW_NOTINSERVICE)) {
                snprintf(line, sizeof(line),
                        "targetParams %s %i %i %s %i %i %i\n",
                        curr_struct->paramNameData, curr_struct->mpModel,
                        curr_struct->secModel, curr_struct->secNameData,
                        curr_struct->secLevel, curr_struct->storageType,
                        curr_struct->rowStatus);
                line[ sizeof(line)-1 ] = 0;

                /*
                 * store to file 
                 */
                snmpd_store_config(line);
            }
            curr_struct = curr_struct->next;
        }
    }
    return SNMPERR_SUCCESS;
}                               /*  store_snmpTargetParmsEntry  */

