/*
   <:copyright-BRCM:2016:DUAL/GPL:standard
   
      Copyright (c) 2016 Broadcom 
      All Rights Reserved
   
   Unless you and Broadcom execute a separate written software license
   agreement governing use of this software, this software is licensed
   to you under the terms of the GNU General Public License version 2
   (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
   with the following added to such license:
   
      As a special exception, the copyright holders of this software give
      you permission to link this software with independent modules, and
      to copy and distribute the resulting executable under terms of your
      choice, provided that you also meet, for each linked independent
      module, the terms and conditions of the license of that module.
      An independent module is a module which is not derived from this
      software.  The special exception does not apply to any modifications
      of the software.
   
   Not withstanding the above, under no circumstances may you combine
   this software in any way with any other Broadcom software provided
   under a license other than the GPL, without Broadcom's express prior
   written consent.
   
   :>
 */


//**************************************************************************
// File Name  : bcm_chip_arch.c
//
// Description: This file defines the Chip specific restrictions 
// for Ethernet LAN/WAN ports.
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
u32 chip_arch_wan_pref_portmap[BP_MAX_ENET_MACS] = {0x00, 0x00};
u32 chip_arch_lan_only_portmap[BP_MAX_ENET_MACS] = {0x00, 0xff};
u32 chip_arch_all_portmap[BP_MAX_ENET_MACS]      = {0x03, 0x1bf};
u32 chip_arch_mgmt_portmap[BP_MAX_ENET_MACS]     = {0x02, 0x100};
#elif defined(CONFIG_BCM94908)
u32 chip_arch_wan_only_portmap[BP_MAX_ENET_MACS] = {0x08, 0x00};
u32 chip_arch_wan_pref_portmap[BP_MAX_ENET_MACS] = {0x00, 0x00};
u32 chip_arch_lan_only_portmap[BP_MAX_ENET_MACS] = {0x00, 0xff};
u32 chip_arch_all_portmap[BP_MAX_ENET_MACS]      = {0x0f, 0x1bf};
u32 chip_arch_mgmt_portmap[BP_MAX_ENET_MACS]     = {0x07, 0x130};
#elif defined(CONFIG_BCM963158)
u32 chip_arch_wan_only_portmap[BP_MAX_ENET_MACS] = {0x30, 0x00};
u32 chip_arch_wan_pref_portmap[BP_MAX_ENET_MACS] = {0x00, 0x00};
u32 chip_arch_lan_only_portmap[BP_MAX_ENET_MACS] = {0x00, 0x5f};
u32 chip_arch_all_portmap[BP_MAX_ENET_MACS]      = {0x37, 0x1ff};
u32 chip_arch_mgmt_portmap[BP_MAX_ENET_MACS]     = {0x07, 0x1a0};
#else
u32 chip_arch_wan_only_portmap[BP_MAX_ENET_MACS] = {0x00, 0x00};
u32 chip_arch_wan_pref_portmap[BP_MAX_ENET_MACS] = {0x00, 0x00};
u32 chip_arch_lan_only_portmap[BP_MAX_ENET_MACS] = {0x00, 0x00};
#endif

#endif /* _BCM_CHIP_ARCH_C_ */

