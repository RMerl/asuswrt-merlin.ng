/*
 * Portions of this file are copyrighted by:
 * Copyright (c) 2016 VMware, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */
#ifndef SNMP_TARGET_H
#define SNMP_TARGET_H

/*
 * optional filtering function.  Return either TARGET_SKIP or TARGET_KEEP 
 */
typedef int     (TargetFilterFunction) (struct targetAddrTable_struct *
                                        targaddrs,
                                        struct targetParamTable_struct *
                                        param, void *);
#define TARGET_KEEP 0
#define TARGET_SKIP 1


/*
 * utility functions 
 */

netsnmp_session *get_target_sessions(char *taglist, TargetFilterFunction *,
                                     void *filterArg);

config_require(target/snmpTargetAddrEntry_data)
config_require(target/snmpTargetParamsEntry_data)

#endif                          /* SNMP_TARGET_H */
