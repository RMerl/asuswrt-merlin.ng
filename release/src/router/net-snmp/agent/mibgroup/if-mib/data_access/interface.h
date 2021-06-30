/*
 * interface data access header
 *
 * $Id$
 */
#ifndef NETSNMP_ACCESS_INTERFACE_CONFIG_H
#define NETSNMP_ACCESS_INTERFACE_CONFIG_H

/*
 * all platforms use this generic code
 */
config_require(if-mib/data_access/interface)

/**---------------------------------------------------------------------*/
/*
 * configure required files
 *
 * Notes:
 *
 * 1) prefer functionality over platform, where possible. If a method
 *    is available for multiple platforms, test that first. That way
 *    when a new platform is ported, it won't need a new test here.
 *
 * 2) don't do detail requirements here. If, for example,
 *    HPUX11 had different reuirements than other HPUX, that should
 *    be handled in the *_hpux.h header file.
 */

config_exclude(mibII/interfaces)

#if defined(linux)
config_require(util_funcs)
config_require(if-mib/data_access/interface_linux)
config_require(if-mib/data_access/interface_ioctl)
#elif defined( openbsd3 ) ||                                         \
    defined( freebsd4 ) || defined( freebsd5 ) || defined( freebsd6 ) || \
    defined( darwin )   || defined( dragonfly ) || defined( netbsd1 )
config_require(if-mib/data_access/interface_sysctl)
#elif defined(solaris2)
config_require(if-mib/data_access/interface_solaris2)
#elif defined(HAVE_IPHLPAPI_H) && !defined(cygwin)
config_require(if-mib/data_access/interface_iphlpapi)
#else
config_warning(This platform does not yet support the IF-MIB)
config_require(if-mib/data_access/interface_unsup)
#endif

#endif /* NETSNMP_ACCESS_INTERFACE_CONFIG_H */
