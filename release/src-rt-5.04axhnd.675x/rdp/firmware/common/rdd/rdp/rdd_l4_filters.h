/*
   <:copyright-BRCM:2014-2016:DUAL/GPL:standard
   
      Copyright (c) 2014-2016 Broadcom 
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

#ifndef _RDD_L4_FILTERS_H
#define _RDD_L4_FILTERS_H

typedef enum
{
    RDD_L4_FILTER_ERROR = 0,
    RDD_L4_FILTER_EXCEPTION,
    RDD_L4_FILTER_IP_FIRST_FRAGMENT,
    RDD_L4_FILTER_IP_FRAGMENT,
    RDD_L4_FILTER_GRE,
    RDD_L4_FILTER_L3_IPV4,
    RDD_L4_FILTER_L3_IPV6,
    RDD_L4_FILTER_ICMP,
    RDD_L4_FILTER_ESP,
    RDD_L4_FILTER_AH,
    RDD_L4_FILTER_IPV6,
    RDD_L4_FILTER_UDEF_0,
    RDD_L4_FILTER_UDEF_1,
    RDD_L4_FILTER_UDEF_2,
    RDD_L4_FILTER_UDEF_3,
    RDD_L4_FILTER_UNKNOWN,
} rdd_l4_filter_t;


void rdd_l4_filters_init(rdpa_traffic_dir dir);
int rdd_l4_filter_set(rdd_l4_filter_t index, rdd_action action, uint8_t reason, rdpa_traffic_dir dir);

int rdd_hdr_err_filter_cfg(rdd_action action, uint8_t reason, rdpa_traffic_dir dir);
int rdd_ip_frag_filter_cfg(rdd_action action, uint8_t reason, rdpa_traffic_dir dir);

#endif
