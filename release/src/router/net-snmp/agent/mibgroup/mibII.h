/*
 * module to include the modules relavent to the mib-II mib(s) 
 */

config_require(mibII/system_mib)
config_require(mibII/sysORTable)
config_require(mibII/snmp_mib)
config_require(mibII/vacm_vars)
config_require(mibII/setSerialNo)
#if !defined(cygwin) || defined(HAVE_IPHLPAPI_H)
config_require(mibII/at)
config_require(mibII/ifTable)
config_require(mibII/ip)
config_require(mibII/tcp)
config_require(mibII/icmp)
config_require(mibII/udp)
#endif

/* mibII/ipv6 is activated via --enable-ipv6 and only builds on Linux+*BSD */
#if defined(NETSNMP_ENABLE_IPV6) && (defined(linux) || defined(freebsd3) || defined(netbsd1) || defined(openbsd4)) 
config_require(mibII/ipv6)
#endif

#ifdef NETSNMP_INCLUDE_IFTABLE_REWRITES
config_require(if-mib)
#endif

/*
 * these new module re-rewrites have only been implemented for
 * Linux and *BSD.
 */
#if defined(linux)
config_require(ip-mib ip-forward-mib tcp-mib udp-mib)
#elif defined(dragonfly) || defined(freebsd7) || \
    defined(netbsd5) || defined(openbsd4) || defined( darwin )
config_require(ip-mib ip-forward-mib tcp-mib udp-mib)
#elif defined(solaris2)
config_require(tcp-mib udp-mib)
config_require(ip-forward-mib)
config_require(ip-mib/ipAddressTable ip-mib/ipAddressPrefixTable)
config_require(ip-mib/ipDefaultRouterTable)
#elif defined(freebsd4)
config_require(tcp-mib udp-mib)
#elif defined(netbsd1)
config_require(tcp-mib udp-mib)
#endif

/*
 * For Solaris, enable additional tables when it has extended MIB support.
 */
#if defined( solaris2 ) && defined( HAVE_MIB2_IPIFSTATSENTRY_T )
config_require(ip-mib/ipSystemStatsTable ip-mib/ipIfStatsTable)
/* Still missing:
 * ip-mib/inetNetToMediaTable
 * ip-mib/ipv6ScopeZoneIndexTable
 */
#endif
