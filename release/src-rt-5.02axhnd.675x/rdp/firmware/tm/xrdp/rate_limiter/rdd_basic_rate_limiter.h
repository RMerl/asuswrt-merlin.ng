/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

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
    rdd_basic_rl_block_t type;
    uint8_t block_index;
} rdd_basic_rl_cfg_t;

/* API to RDPA level */
bdmf_error_t rdd_basic_rate_limiter_cfg(rdpa_traffic_dir dir, uint8_t basic_rl_index, rdd_basic_rl_cfg_t *rl_cfg);
bdmf_error_t rdd_basic_rate_limiter_remove(rdpa_traffic_dir dir, uint8_t basic_rl_index);

#endif
