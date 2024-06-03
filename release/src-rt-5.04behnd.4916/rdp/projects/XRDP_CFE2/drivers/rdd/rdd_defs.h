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

#ifndef _RDD_DEFS_H
#define _RDD_DEFS_H

#include "rdd_runner_proj_defs.h"






static inline int get_runner_idx(rdp_runner_image_e module_idx)
{
    int i;

    for (i = 0; i < NUM_OF_RUNNER_CORES; ++i)
        if (rdp_core_to_image_map[i] == module_idx)
            return i;

    return NUM_OF_RUNNER_CORES;
}

#ifndef CONFIG_GPL_RDP

#define ADDRESS_OF(image, task_name) image##_##task_name

/* count the number of bits in a vector */
static inline uint8_t asserted_bits_count_get(uint32_t mask)
{
    uint8_t i, count = 0;

    for (i = 0; i < 32; ++i)
    {
        if (mask & 1)
            count++;
        mask = mask >> 1;
    }
    return count;
}
#endif



#endif



