/*
 * This file was created to separate notification data storage from
 * the MIB implementation.
 *
 * Portions of this file are copyrighted by:
 * Copyright (c) 2016 VMware, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */

#ifndef _MIBGROUP_SNMPTARGETPARAMSENTRY_DATA_H
#define _MIBGROUP_SNMPTARGETPARAMSENTRY_DATA_H

/*
 * Magic number definitions: 
 */
#define   SNMPTARGETPARAMSMPMODEL        1
#define   SNMPTARGETPARAMSSECURITYMODEL  2
#define   SNMPTARGETPARAMSSECURITYNAME   3
#define   SNMPTARGETPARAMSSECURITYLEVEL  4
#define   SNMPTARGETPARAMSSTORAGETYPE    5
#define   SNMPTARGETPARAMSROWSTATUS      6
#define   SNMPTARGETPARAMSMPMODELCOLUMN        2
#define   SNMPTARGETPARAMSSECURITYMODELCOLUMN  3
#define   SNMPTARGETPARAMSSECURITYNAMECOLUMN   4
#define   SNMPTARGETPARAMSSECURITYLEVELCOLUMN  5
#define   SNMPTARGETPARAMSSTORAGETYPECOLUMN    6
#define   SNMPTARGETPARAMSROWSTATUSCOLUMN      7

/*
 * structure definitions
 */
struct targetParamTable_struct {
    char           *paramNameData;
    size_t          paramNameLen;
    int             mpModel;
    int             secModel;
    char           *secNameData;
    size_t          secNameLen;
    int             secLevel;
    int             storageType;
    int             rowStatus;
    struct targetParamTable_struct *next;
    time_t          updateTime;
};

/*
 * utility functions
 */
struct targetParamTable_struct *get_paramEntry2(const char *name,
                                                size_t nameLen);

void snmpTargetParamTable_add(struct targetParamTable_struct *newEntry);
void snmpTargetParamTable_remove(struct targetParamTable_struct *entry);
struct targetParamTable_struct *search_snmpTargetParamsTable(oid * baseName,
                                                             size_t nameLen,
                                                             oid * name,
                                                             size_t * length,
                                                             int exact);

struct targetParamTable_struct *snmpTargetParamTable_create(void);

void snmpTargetParamTable_dispose(struct targetParamTable_struct *);

/*
 * function definitions
 */

void            init_snmpTargetParamsEntry_data(void);
void            shutdown_snmpTargetParamsEntry_data(void);

#endif                          /* _MIBGROUP_SNMPTARGETPARAMSENTRY_DATA_H */
