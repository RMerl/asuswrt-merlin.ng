/*
<:copyright-BRCM:2012:DUAL/GPL:standard

   Copyright (c) 2012 Broadcom 
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

/*
 *******************************************************************************
 * File Name  : fap_dynmem.h
 *
 * Description: This file contains the base interface into the FAP dynamic
 *              memory.
 *******************************************************************************
 */

#ifndef _FAP_DYNMEM_H_INCLUDED_
#define _FAP_DYNMEM_H_INCLUDED_

#ifndef DYN_MEM_TEST_APP
#include "fap_hw.h"
#endif


typedef enum    {
    FAP_DM_REGION_DSP = 0,
    FAP_DM_REGION_PSM,
    FAP_DM_REGION_QSM,
    FAP_DM_REGION_HP,       // high priority - may be in QSM or PSM
    FAP_DM_REGION_MAX        
} fapDm_RegionIdx;

typedef enum    {
    FAP_DM_REGION_ORDER_DSP_PSM_QSM = 0,
    FAP_DM_REGION_ORDER_DSP_PSM_QSM_HP,
    FAP_DM_REGION_ORDER_QSM_PSM,
    FAP_DM_REGION_ORDER_QSM_PSM_HP,
    FAP_DM_REGION_ORDER_QSM,
    FAP_DM_REGION_ORDER_MAX
} fapDm_RegionOrder;

typedef uint32 fapDm_BlockId;

typedef union {
    struct  {
        unsigned              regionIdx : 3;
        unsigned              blockIdx  : 13;
        unsigned              offset : 16;
    };
    fapDm_BlockId           id;
} fapDm_BlockInfo ;


/* Note: FAP_DM_INVALID_BLOCK_ID may not be 0 (as 0 is valid), and must be the same byte repeated four times, due to
   the use of memset to make all blocks invalid by default */

#define FAP_DM_INVALID_BLOCK_ID        ((fapDm_BlockId)0xFFFFFFFF)

#define FAP_DM_RSVD_HP_FLOW_CNT     12
/* The following assumes that only flow info is stored in the hp flow types -- not command lists.
   This is true for multicast.  If we want to do other types of high priority flows, this will
   need to be adjusted: */
#define FAP_DM_HP_SIZE              (FAP_DM_RSVD_HP_FLOW_CNT*sizeof(fap4kePkt_flow_t))

#endif
