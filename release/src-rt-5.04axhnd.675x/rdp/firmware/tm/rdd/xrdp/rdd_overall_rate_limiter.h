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
