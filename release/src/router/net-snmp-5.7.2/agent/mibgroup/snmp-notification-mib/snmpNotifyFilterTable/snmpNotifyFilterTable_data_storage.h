/*
 * Note: this file was created to separate storage from MIB implementation
 *
 * Portions of this file are copyrighted by:
 * Copyright (c) 2016 VMware, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */
#ifndef SNMPNOTIFYFILTERTABLE_DATA_STORAGE_H
#define SNMPNOTIFYFILTERTABLE_DATA_STORAGE_H

#ifdef __cplusplus
extern          "C" {
#endif


#include <net-snmp/library/asn1.h>

#ifndef SNMPNOTIFYFILTERTYPE_ENUMS
#define SNMPNOTIFYFILTERTYPE_ENUMS
#define SNMPNOTIFYFILTERTYPE_INCLUDED  1
#define SNMPNOTIFYFILTERTYPE_EXCLUDED  2
#endif                          /* SNMPNOTIFYFILTERTYPE_ENUMS */

    /*
     * BE VERY CAREFUL TO TAKE INTO ACCOUNT THE MAXIMUM
     * POSSIBLE LENGHT FOR EVERY VARIABLE LENGTH INDEX!
     * Guessing 128 - col/entry(2)  - oid len(9)
     */
#define MAX_snmpNotifyFilterTable_IDX_LEN     117

    typedef struct snmpNotifyFilterTable_data_storage_s {

        /** this must be first for container compare to work */
        netsnmp_index   oid_idx;
        oid             oid_tmp[MAX_snmpNotifyFilterTable_IDX_LEN];

        /*
         * snmpNotifyFilterProfileName(1)
         * SnmpAdminString/ASN_OCTET_STR/u_char(u_char)//L/A/W/e/R/d/H
         */
        u_char          snmpNotifyFilterProfileName[32];
        size_t          snmpNotifyFilterProfileName_len;

        /*
         * snmpNotifyFilterSubtree(1)
         * OBJECTID/ASN_OBJECT_ID/oid(oid)//L/a/w/e/r/d/h
         */
        /** 128 - 1(other indexes) - oid length(11) = 115 */
        oid             snmpNotifyFilterSubtree[115];
        size_t          snmpNotifyFilterSubtree_len;

        /** END OF INDEXES */

        /*
         * snmpNotifyFilterMask(2)
         * OCTETSTR/ASN_OCTET_STR/char(u_char)//L/A/W/e/R/D/h
         */
        u_char          snmpNotifyFilterMask[16];
        size_t          snmpNotifyFilterMask_len;

        /*
         * snmpNotifyFilterType(3)
         * INTEGER/ASN_INTEGER/long(u_long)//l/A/W/E/r/D/h
         */
        u_long          snmpNotifyFilterType;

    } snmpNotifyFilter_data_storage;

    /*
     *********************************************************************
     * function declarations
     */
    void            init_snmpNotifyFilterTable_data_storage(void);
    void            shutdown_snmpNotifyFilterTable_data_storage(void);

    snmpNotifyFilter_data_storage*
    snmpNotifyFilter_storage_create(const u_char *snmpNotifyFilterProfileName,
                                    size_t snmpNotifyFilterProfileName_len,
                                    const oid *snmpNotifyFilterSubtree,
                                    size_t snmpNotifyFilterSubtree_len);

    void
    snmpNotifyFilter_storage_dispose(snmpNotifyFilter_data_storage *data);

    int
    snmpNotifyFilter_storage_insert(snmpNotifyFilter_data_storage *data);

    snmpNotifyFilter_data_storage *
    snmpNotifyFilter_storage_add(const u_char *profile, size_t profile_len,
                                 const oid *filterSubtree,
                                 size_t filterSubtree_len, u_char *filterMask,
                                 size_t filterMask_len, u_long filterType);
    int
    snmpNotifyFilter_storage_remove(snmpNotifyFilter_data_storage *data);

    struct vacm_viewEntry *
    snmpNotifyFilter_vacm_view_subtree(const char *profile);

#ifdef __cplusplus
}
#endif
#endif                          /* SNMPNOTIFYFILTERTABLE_DATA_STORAGE_H */
/**  @} */

