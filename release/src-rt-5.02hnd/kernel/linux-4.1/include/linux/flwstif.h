#if defined(CONFIG_BCM_KF_NBUFF)
#ifndef __FLWSTIF_H_INCLUDED__
#define __FLWSTIF_H_INCLUDED__

                /*--------------------------------------*/
                /* flwstif.h and flwstif.c for Linux OS */
                /*--------------------------------------*/

/* 
* <:copyright-BRCM:2014:DUAL/GPL:standard
* 
*    Copyright (c) 2014 Broadcom 
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
:>
*/

#if defined(__KERNEL__)                 /* Kernel space compilation         */
#include <linux/types.h>                /* LINUX ISO C99 7.18 Integer types */
#else                                   /* User space compilation           */
#include <stdint.h>                     /* C-Lib ISO C99 7.18 Integer types */
#endif
#include "bcmtypes.h"

typedef struct {
    uint32_t rx_packets;
    uint64_aligned rx_bytes;
    uint32_t pollTS_ms; // Poll timestamp in ms
}FlwStIf_t;

typedef enum {
    FLWSTIF_REQ_GET,
    FLWSTIF_REQ_PUSH,
    FLWSTIF_REQ_MAX
}FlwStIfReq_t;

extern uint32_t flwStIf_request( FlwStIfReq_t req, void *ptr, unsigned long param1,
                                 uint32_t param2, uint32_t param3, void *param4 );

typedef int (* flwStIfGetHook_t)( uint32_t flwIdx, FlwStIf_t *flwSt_p );

typedef int (* flwStIfPushHook_t)( void *ctk1, void *ctk2, uint32_t dir1,
                                uint32_t dir2, FlwStIf_t *flwSt_p );

extern void flwStIf_bind(flwStIfGetHook_t flwStIfGetHook, flwStIfPushHook_t flwStIfPushHook);

#endif /* __FLWSTIF_H_INCLUDED__ */
#endif
