/*
 * This file was created to separate data storage from  MIB implementation.
 *
 * Portions of this file are copyrighted by:
 * Copyright (c) 2016 VMware, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */


#ifndef _MIBGROUP_SNMPNOTIFYTABLE_DATA_H
#define _MIBGROUP_SNMPNOTIFYTABLE_DATA_H


/*
 * we may use header_complex from the header_complex module
 */
config_require(header_complex)
config_require(target/target)
config_require(snmp-notification-mib/snmpNotifyFilterTable/snmpNotifyFilterTable_data_storage)

#define NOTIFY_NAME_MAX       32
#define NOTIFY_TAG_MAX       255

/*
 * our storage structure(s)
 */
struct snmpNotifyTable_data {
    char           *snmpNotifyName;
    size_t          snmpNotifyNameLen;
    char           *snmpNotifyTag;
    size_t          snmpNotifyTagLen;
    long            snmpNotifyType;
    long            snmpNotifyStorageType;
    long            snmpNotifyRowStatus;
};


/*
 * enum definitions from the covered mib sections
 */

#define SNMPNOTIFYTYPE_TRAP                      1
#define SNMPNOTIFYTYPE_INFORM                    2


/*
 * function prototypes
 */
void            init_snmpNotifyTable_data(void);
void            shutdown_snmpNotifyTable_data(void);

int             snmpNotifyTable_add(struct snmpNotifyTable_data *thedata);
int             snmpNotifyTable_remove(struct snmpNotifyTable_data *thedata);
struct snmpNotifyTable_data *
                snmpNotifyTable_extract(struct snmpNotifyTable_data *thedata);
void            snmpNotifyTable_dispose(struct snmpNotifyTable_data *thedata);

void            snmpNotifyTable_unregister_notification(const char *, u_char);

struct snmpNotifyTable_data *get_notifyTable2(const char *name, size_t len);
struct snmpNotifyTable_data *find_row_notifyTable(struct variable *vp,
                                                  oid * name, size_t * length,
                                                  int exact, size_t * var_len,
                                                  WriteMethod ** write_method);


#endif                          /* _MIBGROUP_SNMPNOTIFYTABLE_DATA_H */
