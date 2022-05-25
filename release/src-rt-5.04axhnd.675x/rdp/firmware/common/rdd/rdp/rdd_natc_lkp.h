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

