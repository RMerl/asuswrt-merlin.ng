/*
 *  Template MIB group interface - udp.h
 *
 */
#ifndef _MIBGROUP_UDP_H
#define _MIBGROUP_UDP_H


config_require(mibII/udpTable)

#ifdef solaris2
config_require(kernel_sunos5)
#elif defined(linux)
config_require(mibII/kernel_linux)
#elif defined(netbsd) || defined(netbsdelf)
config_require(mibII/kernel_netbsd)
#endif

extern void     init_udp(void);
extern Netsnmp_Node_Handler udp_handler;
extern NetsnmpCacheLoad udp_load;
extern NetsnmpCacheFree udp_free;


#define UDPINDATAGRAMS	    1
#define UDPNOPORTS	    2
#define UDPINERRORS	    3
#define UDPOUTDATAGRAMS     4

#endif                          /* _MIBGROUP_UDP_H */
