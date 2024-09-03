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

#ifndef _RDPA_MW_BLOG_PARSE_H_
#define _RDPA_MW_BLOG_PARSE_H_

#include "rdpa_mw_arch.h"

struct net_device;
typedef enum
{
    RDPA_DIR_RX,
    RDPA_DIR_TX,
    RDPA_DIR_NA
}RdpaDir_t;

#define rdpa_mw_get_port_object_by_dev(dev) rdpa_mw_get_port_object_by_dev_dir(dev, RDPA_DIR_NA)
#ifdef CONFIG_BLOG
int blog_parse_mcast_result_get(Blog_t *blog, rdpa_ic_result_t *mcast_result);
void blog_parse_mcast_result_put(rdpa_ic_result_t *mcast_result);
bdmf_object_handle blog_parse_egress_port_get(Blog_t *blog);
bdmf_object_handle blog_parse_ingress_port_get(Blog_t *blog);
#if defined(POLICER_SUPPORT)
void blog_parse_policer_get(Blog_t *blog_p, bdmf_object_handle *policer);
#endif
#endif

#endif
