/*
 * <:copyright-BRCM:2013:DUAL/GPL:standard
 * 
 *    Copyright (c) 2013 Broadcom 
 *    All Rights Reserved
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :> 
 */


#ifndef _RDP_CPU_RING_DEFS_H
#define _RDP_CPU_RING_DEFS_H

#include "access_macros.h"

#ifndef _BYTE_ORDER_LITTLE_ENDIAN_
#if !defined(CONFIG_CPU_RX_FROM_XPM)
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
#if !defined(CONFIG_CPU_RX_FROM_XPM)
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
