/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard
    
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

#ifndef _RDD_BASIC_RATE_LIMITER_H
#define _RDD_BASIC_RATE_LIMITER_H

#include "rdd.h"

#define BASIC_RATE_LIMITER

#define BASIC_RATE_LIMITER_INIT_RATE            0

typedef enum
{
    rdd_basic_rl_queue = 0,
    rdd_basic_rl_basic_scheduler = 1,
    rdd_basic_rl_complex_scheduler = 2,
    num_of_rdd_basic_rl_block = 3
} rdd_basic_rl_block_t;

typedef struct
{
    uint32_t rate;
    uint32_t limit;
    uint32_t residue;
    rdd_basic_rl_block_t type;
    uint32_t block_index;
} rdd_basic_rl_cfg_t;

uint32_t rdd_basic_rate_limiter_size_get(rdpa_traffic_dir dir);
/* API to RDPA level */
bdmf_error_t rdd_basic_rate_limiter_cfg(rdpa_traffic_dir dir, int16_t basic_rl_index, uint8_t tm_id, rdd_basic_rl_cfg_t *rl_cfg);
bdmf_error_t rdd_basic_rate_limiter_remove(rdpa_traffic_dir dir, int16_t basic_rl_index, uint8_t tm_id);

#endif
