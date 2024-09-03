/*
<:copyright-BRCM:2018:DUAL/GPL:standard

   Copyright (c) 2018 Broadcom 
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
