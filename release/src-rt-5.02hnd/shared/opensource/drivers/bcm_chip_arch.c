/*
   <:copyright-BRCM:2010:DUAL/GPL:standard
   
      Copyright (c) 2010 Broadcom 
      All Rights Reserved
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2, as published by
   the Free Software Foundation (the "GPL").
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   
   A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
   writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
   
   :>
 */


//**************************************************************************
// File Name  : bcm_chip_arch.c
//
// Description: This is Linux network driver for Broadcom Ethernet controller
//
//**************************************************************************

#define VERSION     "0.1"
#define VER_STR     "v" VERSION " " __DATE__ " " __TIME__

#ifndef _BCM_CHIP_ARCH_C_
#define _BCM_CHIP_ARCH_C_

#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/types.h>
#include "boardparms.h"

/* Port bit map for LAN/WAN attributes */ 
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148)
u32 chip_arch_wan_only_portmap[BP_MAX_ENET_MACS] = {0x01, 0x00};
u32 chip_arch_wan_pref_portmap[BP_MAX_ENET_MACS] = {0x00};
u32 chip_arch_lan_only_portmap[BP_MAX_ENET_MACS] = {0x00, 0xff};
#elif defined(CONFIG_BCM94908)
u32 chip_arch_wan_only_portmap[BP_MAX_ENET_MACS] = {0x08, 0x00};
u32 chip_arch_wan_pref_portmap[BP_MAX_ENET_MACS] = {0x00};
u32 chip_arch_lan_only_portmap[BP_MAX_ENET_MACS] = {0x00, 0xff};
#else
u32 chip_arch_wan_only_portmap[BP_MAX_ENET_MACS] = {0x00};
u32 chip_arch_wan_pref_portmap[BP_MAX_ENET_MACS] = {0x00};
u32 chip_arch_lan_only_portmap[BP_MAX_ENET_MACS] = {0x00};
#endif

#endif /* _BCM_CHIP_ARCH_C_ */

