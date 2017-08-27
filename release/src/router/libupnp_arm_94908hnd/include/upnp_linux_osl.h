/*
 * Broadcom UPnP library linux specific OSL include file
 *
 * Copyright (C) 2015, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: upnp_linux_osl.h 241182 2011-02-17 21:50:03Z $
 */

#ifndef __LIBUPNP_LINUX_OSL_H__
#define __LIBUPNP_LINUX_OSL_H__

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ctype.h>
#include <signal.h>

#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>
#include <time.h>
#include <stdarg.h>

#define upnp_syslog(...) {}

#define upnp_printf printf
#define	upnp_pid() (int)getpid()
#define	upnp_sleep(n) sleep(n)

int upnp_osl_ifaddr(const char *ifname, struct in_addr *inaddr);
int upnp_osl_netmask(const char *ifname, struct in_addr *inaddr);
int upnp_osl_hwaddr(const char *ifname, char *mac);
int upnp_open_udp_socket(struct in_addr addr, unsigned short port);
int upnp_open_tcp_socket(struct in_addr addr, unsigned short port);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LIBUPNP_LINUX_OSL_H__ */
