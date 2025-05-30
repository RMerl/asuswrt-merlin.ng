/*
<:copyright-BRCM:2018:DUAL/GPL:standard

   Copyright (c) 2018 Broadcom 
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
#ifndef _WFD_DEV_PRIV__H_
#define _WFD_DEV_PRIV__H_

#if defined(CONFIG_BCM96846) || defined(CONFIG_BCM96878)
#define WFD_MAX_OBJECTS   2
#elif !defined(XRDP_RGEN6) || defined(CONFIG_BCM96855) || defined(CONFIG_BCM963146)
#define WFD_MAX_OBJECTS   3
#else
#define WFD_MAX_OBJECTS 4
#endif

#define WIFI_MW_MAX_NUM_IF 16

struct net_device *wfd_dev_by_name_get(char *name, uint32_t *radio_id, uint32_t *if_idx);
struct net_device *wfd_dev_by_id_get(uint32_t radio_id, uint32_t if_id);
int wfd_is_radio_valid(uint32_t radio_id);

#endif
