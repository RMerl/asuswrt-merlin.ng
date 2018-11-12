#include "bsd.h"

#include <sys/param.h>

#define PCB_TABLE 1
#undef TCP_SYMBOL
#define TCP_SYMBOL "tcbtable"
#undef TCP_TTL_SYMBOL
#define TCP_TTL_SYMBOL "ip_defttl"
#undef UDB_SYMBOL
#define UDB_SYMBOL "udbtable"
#undef NPROC_SYMBOL
#undef PROC_SYMBOL

#define MBPOOL_SYMBOL	"mbpool"
#define MCLPOOL_SYMBOL	"mclpool"

/*
 * inp_next symbol 
 */
#define HAVE_INPCBTABLE 1
#undef INP_NEXT_SYMBOL
#undef INP_PREV_SYMBOL

#if __NetBSD_Version__ >= 700000001
#define INP_FIRST_SYMBOL inpt_queue.tqh_first
#define INP_NEXT_SYMBOL inp_queue.tqe_next
#define INP_PREV_SYMBOL inp_queue.tqe_prev
#else
#define INP_FIRST_SYMBOL inpt_queue.cqh_first
#define INP_NEXT_SYMBOL inp_queue.cqe_next
#define INP_PREV_SYMBOL inp_queue.cqe_prev
#endif

#if __NetBSD_Version__ >= 106300000       /* NetBSD 1.6ZD */            
#undef IFADDR_SYMBOL
#define IFADDR_SYMBOL "in_ifaddrhead"
#undef TOTAL_MEMORY_SYMBOL
#endif

#define UTMP_FILE _PATH_UTMP

#define UDP_ADDRESSES_IN_HOST_ORDER 1

#ifdef netbsdelf7
#define netbsd7
#define netbsdelf6
#endif
#ifdef netbsdelf6
#define netbsd6
#define netbsdelf5
#endif
#ifdef netbsdelf5
#define netbsd5
#define netbsdelf4
#endif
#ifdef netbsdelf4
#define netbsd4
#define netbsdelf3
#endif
#ifdef netbsdelf3
#define netbsd3
#endif

#if defined(netbsd8) && !defined(netbsd7)
#define netbsd7 netbsd7
#endif
#if defined(netbsd7) && !defined(netbsd6)
#define netbsd6 netbsd6
#endif
#if defined(netbsd6) && !defined(netbsd5)
#define netbsd5 netbsd5
#endif
#if defined(netbsd5) && !defined(netbsd4)
#define netbsd4 netbsd4
#endif
#if defined(netbsd4) && !defined(netbsd3)
#define netbsd3 netbsd3
#endif
#if defined(netbsd3) && !defined(netbsd2)
#define netbsd2 netbsd2
#endif
#ifndef netbsd1
#define netbsd1 netbsd1
#endif

#if __NetBSD_Version__ >= 499005800
#define NETBSD_STATS_VIA_SYSCTL
#endif /* __NetBSD_Version__ >= 499005800 */

/* define the extra mib modules that are supported */
#define NETSNMP_INCLUDE_HOST_RESOURCES
#define NETSNMP_INCLUDE_IFTABLE_REWRITES
