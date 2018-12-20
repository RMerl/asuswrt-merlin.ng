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
#include <cfg_slavelist.h>
#include <cfg_clientlist.h>
#include <json.h>
/* linklist */
#include <linklist.h>

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

/* define amas usage */
#define DHCP_TABLE     "/tmp/var/lib/misc/dnsmasq.leases"
#define ARP_TABLE      "/proc/net/arp"

int amas_lib_device_ip_query(char *mac, char *ip);

#endif /*  _amas_lib_h_ */

