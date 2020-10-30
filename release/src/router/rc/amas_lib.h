 /*
 * Copyright 2018, ASUSTeK Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. ASUS
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

#ifndef _amas_lib_h_
#define _amas_lib_h_

/* header */
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <shutils.h>
#include <shared.h>
#include <rc.h>
#if defined(RTCONFIG_BWDPI)
/* TrendMicro */
#include <bwdpi_common.h>
#include "ioc_mesh.h"
#include "ioc_common.h"
#endif
/* shared memory / cfg_mnt / json */
#include <sys/shm.h>
#include <json.h>
#include <cfg_slavelist.h>
#include <cfg_clientlist.h>
/* linklist */
#include <linklist.h>
#ifdef RTCONFIG_LIBASUSLOG
#include "libasuslog.h"
#endif

typedef enum __amaslib_dhcp_flag_t_
{
	AMASLIB_DHCP_FLAG_NO_CHANGED = 0,
	AMASLIB_DHCP_FLAG_NEW_DATA,
	AMASLIB_DHCP_FLAG_IP_CHANGED
} AMASLIB_DHCP_FLAG_T;

typedef struct __amaslib_dhcp__t_
{
	char mac[18];       /* device MAC */
	char ip[16];        /* device IP  */
	int flag;           /* flag for checking use*/
} AMASLIB_DHCP_T;


#define MAX_AMASLIB_SOCKET_CLIENT  5

/* DEBUG DEFINE */
#define AMASLIB_DEBUG             "/tmp/AMASLIB_DEBUG"

#ifdef RTCONFIG_LIBASUSLOG
#define AMAS_LIB_DBG_LOG    "amas_lib.log"
extern char *__progname;
/*#define AMASLIB_DBG_SYSLOG(fmt,args...) \
	if (nvram_get_int("amaslib_syslog")) { \
		char info[1024]; \
		time_t timep; \
		time (&timep); \
		snprintf(info, sizeof(info), "echo \" %.24s [AMASLIB][%s][%d][%s:(%d)]"fmt"\" >> /tmp/"AMAS_LIB_DBG_LOG, ctime(gmtime(&timep)), __progname, getpid(), __FUNCTION__, __LINE__, ##args); \
		system(info); \
	}*/
#define AMASLIB_DBG_SYSLOG(fmt,args...) \
	if (nvram_get_int("amaslib_syslog")) { \
		asusdebuglog(LOG_INFO, AMAS_LIB_DBG_LOG, LOG_CUSTOM, LOG_SHOWTIME, 0, "[%s][%d]][%s:(%d)] "fmt, __progname, getpid(), __FUNCTION__, __LINE__, ##args); \
	}
#else
#define AMASLIB_DBG_SYSLOG(fmt,args...) {}
#endif

#define AMASLIB_DBG(fmt,args...) do { \
	if(f_exists(AMASLIB_DEBUG) > 0) { \
		_dprintf("[AMASLIB][%s:(%d)]"fmt, __FUNCTION__, __LINE__, ##args); \
	} \
	AMASLIB_DBG_SYSLOG(fmt,##args) \
} while(0)

extern int AMAS_EVENT_TRIGGER(char *sta2g, char *sta5g, int flag);
extern int is_amaslib_enabled();

/* define amas usage */
#define DHCP_TABLE     "/var/lib/misc/dnsmasq.leases"
#define ARP_TABLE      "/proc/net/arp"

int amas_lib_device_ip_query(char *mac, char *ip);

#endif /*  _amas_lib_h_ */

