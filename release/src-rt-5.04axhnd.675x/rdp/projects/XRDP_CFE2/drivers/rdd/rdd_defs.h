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



