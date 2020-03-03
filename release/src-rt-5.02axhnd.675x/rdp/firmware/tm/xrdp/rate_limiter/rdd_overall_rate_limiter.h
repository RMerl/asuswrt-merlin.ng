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

#ifndef _RDD_OVERALL_RATE_LIMITER_H
#define _RDD_OVERALL_RATE_LIMITER_H

#include "rdd.h"

#define OVERALL_RATE_LIMITER

#define OVERALL_RATE_LIMITER_INIT_RATE            0
#define OVERALL_RATE_LIMITER_UNLIMITED_MAN        ((1 << MANTISSA_LEN) - 1)
#define OVERALL_RATE_LIMITER_UNLIMITED_EXP        ((1 << EXPONENT_LEN) - 1)

/* API to RDPA level */
bdmf_error_t rdd_overall_rate_limiter_rate_cfg(uint32_t rate, uint32_t limit);
bdmf_error_t rdd_overall_rate_limiter_bbh_queue_cfg(uint8_t bbh_queue_index, bdmf_boolean is_high_priority);
bdmf_error_t rdd_overall_rate_limiter_remove(uint8_t bbh_queue_index);

#endif
