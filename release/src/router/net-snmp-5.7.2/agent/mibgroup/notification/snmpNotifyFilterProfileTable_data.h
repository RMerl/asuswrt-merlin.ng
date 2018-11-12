/*
 * This file was created to separate data storage from MIB implementation.
 *
 * Portions of this file are copyrighted by:
 * Copyright (c) 2016 VMware, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */


#ifndef _MIBGROUP_SNMPNOTIFYFILTERPROFILETABLE_DATA_H
#define _MIBGROUP_SNMPNOTIFYFILTERPROFILETABLE_DATA_H

config_require(header_complex)

/*
 * our storage structure(s)
 */
struct snmpNotifyFilterProfileTable_data {
    char           *snmpTargetParamsName;
    size_t          snmpTargetParamsNameLen;
    char           *snmpNotifyFilterProfileName;
    size_t          snmpNotifyFilterProfileNameLen;
    long            snmpNotifyFilterProfileStorType;
    long            snmpNotifyFilterProfileRowStatus;
};


/*
 * function prototypes
 */

void            init_snmpNotifyFilterProfileTable_data(void);
void            shutdown_snmpNotifyFilterProfileTable_data(void);

int
snmpNotifyFilterProfileTable_add(struct snmpNotifyFilterProfileTable_data *);

struct snmpNotifyFilterProfileTable_data *
snmpNotifyFilterProfileTable_create(const char *params, size_t param_len,
                                    const char *profile, size_t profile_len);

void
snmpNotifyFilterProfileTable_free(struct snmpNotifyFilterProfileTable_data *);

int
snmpNotifyFilterProfileTable_remove(struct snmpNotifyFilterProfileTable_data *);

struct snmpNotifyFilterProfileTable_data *
snmpNotifyFilterProfileTable_extract(struct snmpNotifyFilterProfileTable_data *);

struct snmpNotifyFilterProfileTable_data *
snmpNotifyFilterProfileTable_oldapi_find(struct variable *vp,
                                         oid *name, size_t *length, int exact,
                                         size_t *var_len,
                                         WriteMethod ** write_method);

struct snmpNotifyFilterProfileTable_data *
snmpNotifyFilterProfileTable_find(const char *name, size_t len);

char           *get_FilterProfileName(const char *paramName,
                                      size_t paramName_len,
                                      size_t * profileName_len);


#endif           /* _MIBGROUP_SNMPNOTIFYFILTERPROFILETABLE_DATA_H */
