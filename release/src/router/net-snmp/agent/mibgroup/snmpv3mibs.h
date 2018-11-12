#ifndef SNMPV3MIBS_H
#define SNMPV3MIBS_H

/*
 * snmpv3mibs.h: mib module to include the modules relavent to the
 * snmpv3 mib(s) 
 */

config_require(snmpv3/snmpEngine)
config_require(snmpv3/snmpMPDStats)
#ifdef NETSNMP_SECMOD_USM
config_require(snmpv3/usmStats)
config_require(snmpv3/usmConf)
config_require(snmpv3/usmUser)
#endif /* NETSNMP_SECMOD_USM */
#endif                          /* SNMPV3MIBS_H */
