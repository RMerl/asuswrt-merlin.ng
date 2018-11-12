/*
 * This file was created to separate notification data storage from
 * the MIB implementation.
 *
 * Portions of this file are copyrighted by:
 * Copyright (c) 2016 VMware, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */

#ifndef _MIBGROUP_SNMPTARGETADDRENTRY_DATA_H
#define _MIBGROUP_SNMPTARGETADDRENTRY_DATA_H

/*
 * we use header_generic from the util_funcs module
 */

    /*
     * Magic number definitions:
     */
#define   SNMPTARGETADDRTDOMAIN      1
#define   SNMPTARGETADDRTADDRESS     2
#define   SNMPTARGETADDRTIMEOUT      3
#define   SNMPTARGETADDRRETRYCOUNT   4
#define   SNMPTARGETADDRTAGLIST      5
#define   SNMPTARGETADDRPARAMS       6
#define   SNMPTARGETADDRSTORAGETYPE  7
#define   SNMPTARGETADDRROWSTATUS    8
#define   SNMPTARGETSPINLOCK         99
#define   SNMPTARGETADDRTDOMAINCOLUMN      2
#define   SNMPTARGETADDRTADDRESSCOLUMN     3
#define   SNMPTARGETADDRTIMEOUTCOLUMN      4
#define   SNMPTARGETADDRRETRYCOUNTCOLUMN   5
#define   SNMPTARGETADDRTAGLISTCOLUMN      6
#define   SNMPTARGETADDRPARAMSCOLUMN       7
#define   SNMPTARGETADDRSTORAGETYPECOLUMN  8
#define   SNMPTARGETADDRROWSTATUSCOLUMN    9

    /*
     * structure definitions
     */
     struct targetAddrTable_struct {
         char           *nameData;
         size_t          nameLen;
         oid             tDomain[MAX_OID_LEN];
         int             tDomainLen;
         void           *tAddress;
         size_t          tAddressLen;
         int             timeout;      /* Timeout in centiseconds */
         int             retryCount;
         char           *tagListData;
         size_t          tagListLen;
         char           *paramsData;
         size_t          paramsLen;
         int             storageType;
         int             rowStatus;
         struct targetAddrTable_struct *next;
         netsnmp_session *sess; /* a snmp session to the target host */
         time_t          sessionCreationTime;
     };

/*
 * function definitions
 */

     void            init_snmpTargetAddrEntry_data(void);
     void            shutdown_snmpTargetAddrEntry_data(void);

     struct targetAddrTable_struct *get_addrTable(void);
     struct targetAddrTable_struct *get_addrForName2(const char *name,
                                                     size_t nameLen);
     struct targetAddrTable_struct *snmpTargetAddrTable_create(void);
     struct targetAddrTable_struct *search_snmpTargetAddrTable(oid * baseName,
                                                               size_t nameLen,
                                                               oid * name,
                                                               size_t * length,
                                                               int exact);
     void         snmpTargetAddrTable_add(struct targetAddrTable_struct *);
     void         snmpTargetAddrTable_remove(struct targetAddrTable_struct *);
     void         snmpTargetAddrTable_dispose(struct targetAddrTable_struct *);

#endif                          /* _MIBGROUP_SNMPTARGETADDRENTRY_DATA_H */
