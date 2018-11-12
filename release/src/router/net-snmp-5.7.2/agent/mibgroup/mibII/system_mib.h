#ifndef _MIBGROUP_SYSTEM_MIB_H
#define _MIBGROUP_SYSTEM_MIB_H

#ifdef __cplusplus
extern "C" {
#endif

config_require(util_funcs mibII/updates)

extern oid system_module_oid[];
extern int system_module_oid_len;
extern int system_module_count;

void init_system_mib(void);

#ifdef __cplusplus
}
#endif

#endif                          /* _MIBGROUP_SYSTEM_MIB_H */
