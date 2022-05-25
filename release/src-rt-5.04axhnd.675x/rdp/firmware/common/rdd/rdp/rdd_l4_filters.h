/*
   <:copyright-BRCM:2014-2016:DUAL/GPL:standard
   
      Copyright (c) 2014-2016 Broadcom 
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
