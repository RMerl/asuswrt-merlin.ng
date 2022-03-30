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

#ifndef RDD_NATC_LKP_H_
#define RDD_NATC_LKP_H_

#include "rdpa_ip_class_basic.h"

#define HASH_FUNCTION_INIT_VAL           0x4899b351
#define RDD_NAT_CACHE_LOOKUP_DEPTH_SIZE  8

typedef struct natc_params
{
    rdpa_traffic_dir dir;
} natc_params_t;

/* Address tables for indexed DPE blocks */
enum {
    natc_nop = 0,
    natc_lookup,
    natc_add,
    natc_del,
};

#define NATC_STATUS_BUSY_BIT  (1 << 4)
#define NATC_STATUS_ERROR_BIT (1 << 5)
#define NATC_STATUS_MISS_BIT  (1 << 6)
#define TIME_OUT_MS                  2
#define KEY_LEN_SHIFT                8
#define NAT_CACHE_SEARCH_ENGINES_NUM 4

int rdd_nat_cache_init(const rdd_module_t *module);

#endif /* RDD_NATC_LKP_H_ */

