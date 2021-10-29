/*
* <:copyright-BRCM:2013:DUAL/GPL:standard
* 
*    Copyright (c) 2013 Broadcom 
*    All Rights Reserved
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, version 2, as published by
* the Free Software Foundation (the "GPL").
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* 
* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
* 
* :>
*/

#include <linux/bcm_log.h>
#include <rdpa_api.h>

int blog_parse_mcast_result_get(Blog_t *blog, rdpa_ic_result_t *mcast_result);
void blog_parse_mcast_result_put(rdpa_ic_result_t *mcast_result);
rdpa_if rdpa_mw_root_dev2rdpa_if(struct net_device *root_dev);
uint8_t rdpa_mw_root_dev2rdpa_ssid(struct net_device *root_dev);
rdpa_if blog_parse_egress_port_get(Blog_t *blog);
rdpa_if blog_parse_ingress_port_get(Blog_t *blog);
#if defined(CONFIG_BCM_PON)
void blog_parse_policer_get(Blog_t *blog_p, bdmf_object_handle *policer);
#endif

