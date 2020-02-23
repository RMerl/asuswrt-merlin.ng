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

#ifndef _RDD_TUPLE_LKP_H
#define _RDD_TUPLE_LKP_H

#include "rdpa_ip_class_basic.h"

#define RDD_TUPLE_LKP_TABLE_BUCKET_SIZE 4
#define RDD_TUPLE_LKP_TABLE_SIZE 32768

typedef struct tuple_lkp_params 
{
    /* general */
    rdpa_traffic_dir dir;
    uint32_t context_index_cam_size;
    /* addresses */
    uint32_t tuple_lkp_table;
    uint32_t semaphore_address;
    uint32_t context_index_cam_address;
    uint32_t counter_table_address;
    /* pointers */
    uint32_t context_index_cam_ptr;
} rdd_tuple_lkp_params_t;

typedef struct rdd_ip_flow_key {
    uint32_t src_ip;
    uint32_t dst_ip;
    uint8_t prot;
    uint16_t src_port;
    uint16_t dst_port;
    bdmf_ip_family family;
} rdd_ip_flow_key;

int rdd_tuple_lkp_init(const rdd_module_t *module);

#endif
