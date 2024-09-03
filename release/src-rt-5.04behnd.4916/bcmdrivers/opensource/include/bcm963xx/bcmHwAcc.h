#ifndef __BCM_HWACC_H_INCLUDED__
#define __BCM_HWACC_H_INCLUDED__

/*
   Copyright (c) 2007-2012 Broadcom Corporation
   All Rights Reserved

<:label-BRCM:2007:DUAL/GPL:standard

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

/*
 *******************************************************************************
 * File Name  : fap.h
 *
 * Description: This file contains the specification of some common definitions
 *      and interfaces to other modules. This file may be included by both
 *      Kernel and userapp (C only).
 *
 *******************************************************************************
 */

#include <pktHdr.h>

#undef FAP_DECL
#define FAP_DECL(x)                 x,  /* for enum declaration in H file */
/*
 *------------------------------------------------------------------------------
 * Flow cache uses a 16bit CMF Tuple for identifying a Flow accelerated in
 * hardware. As there is only a single FAP engine shared by all ports, the
 * entire 16bit value of the tuple is used to represent the matched Flow.
 *
 * Macros shared by FlowCache for NATed configurations of FAP.
 *------------------------------------------------------------------------------
 */
typedef enum {
    FAP_DECL(PKTCMF_ENGINE_SWC)
    FAP_DECL(PKTCMF_ENGINE_MCAST)
    FAP_DECL(PKTCMF_ENGINE_ARL)
    FAP_DECL(PKTCMF_ENGINE_L2FLOW)
    FAP_DECL(PKTCMF_ENGINE_ALL) /* max number of CMF enum */
} PktCmfEngine_t;


#define CMF_TUPLE16_MCAST_MASK    (1<<12)  /* must be an integer power of 2 */
#define CMF_TUPLE16_ARL_MASK      (1<<13)  /* must be an integer power of 2 */
#define CMF_TUPLE16_L2FLOW_MASK   (1<<14)  /* must be an integer power of 2 */

/* Construct a 16bit CMF tuple from the Engine and matched FlowInfo Element. */
#define CMF_TUPLE16(eng,matchIx)                                        \
    ( (eng == PKTCMF_ENGINE_MCAST) ?                                    \
      (__force uint16_t)(matchIx | CMF_TUPLE16_MCAST_MASK) :            \
      ( (eng == PKTCMF_ENGINE_ARL) ?                                    \
        (__force uint16_t)(CMF_TUPLE16_ARL_MASK) :                      \
        ( (eng == PKTCMF_ENGINE_L2FLOW) ?                               \
          (__force uint16_t)(matchIx | CMF_TUPLE16_L2FLOW_MASK) :       \
          (__force uint16_t)(matchIx) ) ) )

/* Extract the Engine (CMF memory space) from the CMF <Engine,MatchIx> tuple. */
extern uint8_t GET_CMF_ENGINE(uint16_t cmfTuple16);


/* Extract the matched FlowINFO RAM index from the CMF tuple. */
extern uint16_t GET_CMF_MATCHIX(uint16_t cmfTuple16);


/* Special tuple to signify an invalid tuple. */
#define CMF_TUPLE16_INVALID         ((__force uint16_t)(0xFFFF))

#define PKTCMF_MAX_FLOWS            (2*512)    // 512 flows per FAP

#endif  /* defined(__BCM_HWACC_H_INCLUDED__ ) */
