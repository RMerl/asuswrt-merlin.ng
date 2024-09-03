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
