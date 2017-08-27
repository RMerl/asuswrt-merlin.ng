/*
 * Broadcom UPnP module eCos OS dependent include file
 *
 * Copyright (C) 2015, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: upnp_ecos_osl.h 241182 2011-02-17 21:50:03Z $
 */

#ifndef __LIBUPNP_ECOS_OSL_H__
#define __LIBUPNP_ECOS_OSL_H__

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <time.h>
#include <shutils.h>
#include <arpa/inet.h>

#ifndef hz
#define hz 100
#endif

#define upnp_syslog(...) {}

#define upnp_printf printf
extern int oslib_pid();
#define	upnp_pid() oslib_pid()
#define	upnp_sleep(n) oslib_ticks_sleep(n * hz)

int upnp_osl_ifaddr(const char *ifname, struct in_addr *inaddr);
int upnp_osl_netmask(const char *ifname, struct in_addr *inaddr);
int upnp_osl_hwaddr(const char *ifname, char *mac);
int upnp_open_udp_socket(struct in_addr addr, unsigned short port);
int upnp_open_tcp_socket(struct in_addr addr, unsigned short port);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LIBUPNP_ECOS_OSL_H__ */
