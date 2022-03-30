/*
 * RFC3927 ZeroConf IPv4 Link-Local addressing
 * (see <http://www.zeroconf.org/>)
 *
 * Copied from BusyBox - networking/zcip.c
 *
 * Copyright (C) 2003 by Arthur van Hoff (avh@strangeberry.com)
 * Copyright (C) 2004 by David Brownell
 *
 * Licensed under the GPL v2 or later
 */

#if defined(CONFIG_CMD_LINK_LOCAL)

#ifndef __LINK_LOCAL_H__
#define __LINK_LOCAL_H__

#include <common.h>

void link_local_receive_arp(struct arp_hdr *arp, int len);
void link_local_start(void);

#endif /* __LINK_LOCAL_H__ */
#endif
