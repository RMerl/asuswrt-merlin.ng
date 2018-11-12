#ifndef _MIBGROUP_USMSTATS_H
#define _MIBGROUP_USMSTATS_H

config_add_mib(SNMP-USER-BASED-SM-MIB)

void init_usmStats(void);
void shutdown_usmStats(void);

#endif /* _MIBGROUP_USMSTATS_H */
