/*
 * <:copyright-BRCM:2013:DUAL/GPL:standard
 * 
 *    Copyright (c) 2013 Broadcom 
 *    All Rights Reserved
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 * :> 
 */


#ifndef _RDP_CPU_RING_DEFS_H
#define _RDP_CPU_RING_DEFS_H

#include "access_macros.h"

#ifndef _BYTE_ORDER_LITTLE_ENDIAN_
#if !defined(CONFIG_CPU_RX_FROM_XPM) && defined(RDP)
typedef struct
{
    union {
        struct {
            uint32_t word0;
            uint32_t word1;
        };
        struct {
            uint32_t host_buffer_data_ptr_low; /* total 40 bits for address */
            uint32_t reserved:23;
            uint32_t abs:1;
            uint32_t host_buffer_data_ptr_hi:8;
        } abs;
    };
}
CPU_FEED_DESCRIPTOR;
#endif

#else
#if !defined(CONFIG_CPU_RX_FROM_XPM) && defined(RDP)
typedef struct
{
    union {
        struct {
            uint32_t word0;
            uint32_t word1;
        };
        struct {
            uint32_t host_buffer_data_ptr_low; /* total 40 bits for address */
            uint32_t host_buffer_data_ptr_hi:8;
            uint32_t abs:1;
            uint32_t reserved:23;
        } abs;
    };
}
CPU_FEED_DESCRIPTOR;
#endif



#endif

#endif /*_RDP_CPU_RING_DEFS_H */
