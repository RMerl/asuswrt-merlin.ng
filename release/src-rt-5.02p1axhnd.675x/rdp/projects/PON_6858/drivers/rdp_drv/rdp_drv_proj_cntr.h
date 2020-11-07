/*
* <:copyright-BRCM:2014:proprietary:standard
*
*    Copyright (c) 2014 Broadcom
*    All Rights Reserved
*
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
*
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
*
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
* :>
*/

#ifndef _RDP_DRV_PROJ_CNTR_H_
#define _RDP_DRV_PROJ_CNTR_H_

#include "rdp_drv_cntr.h"
#include "rdd_runner_proj_defs.h"
#include "XRDP_AG.h"
#include "rdp_drv_proj_policer.h"
#include "rdp_platform.h"

typedef enum
{
    NONE_CNTR_SUB_GROUP_ID,
    TCAM_CNTR_SUB_GROUP_ID,
    IPTV_CNTR_SUB_GROUP_ID,
    DEF_FLOW_CNTR_SUB_GROUP_ID,
} cntr_sub_group_id_t;


/******************************************************************************
  *
  * CNTR GROUP 0 :
  * --------------
  * Structure: 4B+4B+2B
  *
  * Usage:
  *   Counter   | amount
  *   -------------------
  *   RX GEM    | 128
  *   RX vport  | PROJ_DEFS_NUMBER_OF_VPORTS
  *
  *
*******************************************************************************/
#define CNTR0_CNTR_NUM  (128 + PROJ_DEFS_NUMBER_OF_VPORTS)
#define RX_FLOW_COUNTERS_NUM CNTR0_CNTR_NUM
#define RX_FLOW_CNTR_GROUP_INVLID_CNTR (CNTR0_CNTR_NUM-1)

/* CNPL GROUP 0: 4B + 4B (valid bytes, valid packets) */
#define CNTR0_CNPL0_BASE_ADDR 0
#define CNTR0_CNPL0_CNTR_SIZE 2
#define CNTR0_CNPL0_CNTR_TYPE 1
#define CNTR0_CNPL0_ADDR_SIZE (((CNTR0_CNTR_NUM * (1 << CNTR0_CNPL0_CNTR_SIZE) * (1 << CNTR0_CNPL0_CNTR_TYPE)) + 7) >> 3)


/* CNPL GROUP 1: 2B (drop packets) */
#define CNTR0_CNPL1_CNTR_SIZE 1
#define CNTR0_CNPL1_CNTR_TYPE 0
#define CNTR0_CNPL1_ADDR_SIZE (((CNTR0_CNTR_NUM * (1 << CNTR0_CNPL1_CNTR_SIZE) * (1 << CNTR0_CNPL1_CNTR_TYPE)) + 7) >> 3)

/******************************************************************************
  *
  * CNTR GROUP 1 :
  * --------------
  * Structure: 4B+4B
  *
  * Usage:
  *   Counter                   | amount
  *   -------------------
  *   TX GEM                    | 128
  *   TX vport                  | PROJ_DEFS_NUMBER_OF_VPORTS
  *   pathstat packet_hit/byte  | 32
  *
  *
*******************************************************************************/
#if !defined(G9991)
#define CNTR1_CNTR_NUM  (128 + PROJ_DEFS_NUMBER_OF_VPORTS + 32)
#else
#define CNTR1_CNTR_NUM  (128 + PROJ_DEFS_NUMBER_OF_VPORTS)
#endif

#define TX_FLOW_COUNTERS_NUM CNTR1_CNTR_NUM
#define TX_FLOW_CNTR_GROUP_INVLID_CNTR (CNTR1_CNTR_NUM-1)

/* CNPL GROUP 2: 4B + 4B (valid bytes, valid packets) */
#define CNTR1_CNPL2_CNTR_SIZE 2
#define CNTR1_CNPL2_CNTR_TYPE 1
#define CNTR1_CNPL2_ADDR_SIZE (((CNTR1_CNTR_NUM * (1 << CNTR1_CNPL2_CNTR_SIZE) * (1 << CNTR1_CNPL2_CNTR_TYPE)) + 7) >> 3)

/******************************************************************************
  *
  * CNTR GROUP 2 :
  * --------------
  * Structure: 4B
  *
  * Usage:
  *   Counter   | amount
  *   -------------------
  *   TCAM /    | without vlan counters: PROJ_DEFS_NUMBER_OF_SHARED_IC_VLAN_COUNTERS
  *   DEF flow  | with vlan counters : PROJ_DEFS_NUMBER_OF_SHARED_IC_VLAN_COUNTERS / 2
  *
*******************************************************************************/
#define CNTR2_CNTR_NUM_WITHOUT_VLAN  (PROJ_DEFS_NUMBER_OF_IC_COUNTERS + 1)
#define CNTR2_CNTR_NUM_WITH_VLAN  ((PROJ_DEFS_NUMBER_OF_IC_COUNTERS / 2) + 1)


/* CNPL GROUP 3: 4B (valid packets) */
#define CNTR2_CNPL3_CNTR_SIZE 2
#define CNTR2_CNPL3_CNTR_TYPE 0
#define CNTR2_CNPL3_ADDR_SIZE_WITHOUT_VLAN (((CNTR2_CNTR_NUM_WITHOUT_VLAN * (1 << CNTR2_CNPL3_CNTR_SIZE) * (1 << CNTR2_CNPL3_CNTR_TYPE)) + 7) >> 3)
#define CNTR2_CNPL3_ADDR_SIZE_WITH_VLAN (((CNTR2_CNTR_NUM_WITH_VLAN * (1 << CNTR2_CNPL3_CNTR_SIZE) * (1 << CNTR2_CNPL3_CNTR_TYPE)) + 7) >> 3)




/******************************************************************************
  *
  * CNTR GROUP 3 :
  * --------------
  * Structure: 2B
  *
  * Usage:
  *   Counter   | amount
  *   -------------------
  *   VARIOUS   | 112
  *
*******************************************************************************/
#define CNTR3_CNTR_NUM  112 /* should be COUNTER_LAST +2 */
#define VARIOUS_CNTR_GROUP_INVLID_CNTR (CNTR3_CNTR_NUM-1)

/* CNPL GROUP 4: 2B (packets) */
#define CNTR3_CNPL4_CNTR_SIZE 1
#define CNTR3_CNPL4_CNTR_TYPE 0
#define CNTR3_CNPL4_ADDR_SIZE (((CNTR3_CNTR_NUM * (1 << CNTR3_CNPL4_CNTR_SIZE) * (1 << CNTR3_CNPL4_CNTR_TYPE)) + 7) >> 3)

/******************************************************************************
  *
  * CNTR GROUP 4 :
  * --------------
  * Structure: 4B
  *
  * Usage:
  *   Counter   | amount
  *   -------------------
  *   GENERAL   | 24
  *
*******************************************************************************/
#if defined(G9991)
#define CNTR4_CNTR_NUM  15
#else
#define CNTR4_CNTR_NUM  24
#endif
#define GENERAL_CNTR_GROUP_INVLID_CNTR (CNTR4_CNTR_NUM-1) /* should be GENERAL_COUNTER_LAST+1*/

/* CNPL GROUP 5: 4B (packets) */
#define CNTR4_CNPL5_CNTR_SIZE 2
#define CNTR4_CNPL5_CNTR_TYPE 0
#define CNTR4_CNPL5_ADDR_SIZE (((CNTR4_CNTR_NUM * (1 << CNTR4_CNPL5_CNTR_SIZE) * (1 << CNTR4_CNPL5_CNTR_TYPE)) + 7) >> 3)

/******************************************************************************
  *
  * CNTR GROUP 5 :
  * --------------
  * Structure: 4B+4B
  *
  * Usage:
  *   Counter   | amount
  *   -------------------
  *   TX QUEUE  | 256 (us_tx then ds_tx queues)
  *
  *
*******************************************************************************/

#define CNTR5_CNTR_NUM QM_QUEUE_MAX_DYNAMIC_QUANTITY
#define TX_QUEUE_CNTR_GROUP_INVLID_CNTR (CNTR5_CNTR_NUM-1)

/* CNPL GROUP 6: 4B + 4B (valid bytes, valid packets) */
#define CNTR5_CNPL6_CNTR_SIZE 2
#define CNTR5_CNPL6_CNTR_TYPE 1
#define CNTR5_CNPL6_ADDR_SIZE (((CNTR5_CNTR_NUM * (1 << CNTR5_CNPL6_CNTR_SIZE) * (1 << CNTR5_CNPL6_CNTR_TYPE)) + 7) >> 3)

/******************************************************************************
  *
  * CNTR GROUP 6 :
  * --------------
  * Structure: 2B+2B
  *
  * Usage: CPU RING , DHD CPU RING, dhd_FPM IN USE
  *
*******************************************************************************/
#define CNTR6_CNTR_NUM  14
#define CPU_RX_CNTR_GROUP_INVLID_CNTR (CNTR6_CNTR_NUM-1)

/* CNPL GROUP 7: 2B (single counter for lock) */
#define CNTR6_CNPL7_CNTR_SIZE 1
#define CNTR6_CNPL7_CNTR_TYPE 1
#define CNTR6_CNPL7_ADDR_SIZE (((CNTR6_CNTR_NUM * (1 << CNTR6_CNPL7_CNTR_SIZE) * (1 << CNTR6_CNPL7_CNTR_TYPE)) + 7) >> 3)


/******************************************************************************
  *
  * CNTR GROUP 7 :
  * --------------
  * Structure: 2B
  *
  * Usage: CPU RX METERS DROPS
  *   Counter   | amount
  *   -------------------
  *   DS DROP   | 16
  *   US DROP   | 16
  *
*******************************************************************************/
#define CNTR7_CNTR_NUM  (RDD_DS_CPU_RX_METER_TABLE_SIZE + RDD_US_CPU_RX_METER_TABLE_SIZE)
#define CPU_RX_METER_DROP_CNTR_GROUP_INVLID_CNTR (CNTR7_CNTR_NUM-1)

/* CNPL GROUP 8: 2B (packets) */
#define CNTR7_CNPL8_CNTR_SIZE 1
#define CNTR7_CNPL8_CNTR_TYPE 0
#define CNTR7_CNPL8_ADDR_SIZE (((CNTR7_CNTR_NUM * (1 << CNTR7_CNPL8_CNTR_SIZE) * (1 << CNTR7_CNPL8_CNTR_TYPE)) + 7) >> 3)

/******************************************************************************
  *
  * CNTR GROUP 8 :
  * --------------
  * Structure: (4B+4B) green
  *            (4B+4B) yellow
  *            (4B+4B) red
  * Usage:
  *   Counter   | amount
  *   -------------------
  *   color     | 80
  *
  *
*******************************************************************************/
#define CNTR8_CNTR_NUM  80
#define POLICER_CNTR_GROUP_INVLID_CNTR (CNTR8_CNTR_NUM-1)

/* CNPL GROUP 09: 4B + 4B (valid bytes, valid packets) */
#define CNTR8_CNPL9_CNTR_SIZE 2
#define CNTR8_CNPL9_CNTR_TYPE 1
#define CNTR8_CNPL9_ADDR_SIZE (((CNTR8_CNTR_NUM * (1 << CNTR8_CNPL9_CNTR_SIZE) * (1 << CNTR8_CNPL9_CNTR_TYPE)) + 7) >> 3)

/* CNPL GROUP 10: 4B + 4B (valid bytes, valid packets) */
#define CNTR8_CNPL10_CNTR_SIZE 2
#define CNTR8_CNPL10_CNTR_TYPE 1
#define CNTR8_CNPL10_ADDR_SIZE (((CNTR8_CNTR_NUM * (1 << CNTR8_CNPL10_CNTR_SIZE) * (1 << CNTR8_CNPL10_CNTR_TYPE)) + 7) >> 3)

/* CNPL GROUP 11: 4B + 4B (drop packets. drop bytes) */
#define CNTR8_CNPL11_CNTR_SIZE 2
#define CNTR8_CNPL11_CNTR_TYPE 1
#define CNTR8_CNPL11_ADDR_SIZE (((CNTR8_CNTR_NUM * (1 << CNTR8_CNPL11_CNTR_SIZE) * (1 << CNTR8_CNPL11_CNTR_TYPE)) + 7) >> 3)

#ifdef G9991
/******************************************************************************
  *
  * CNTR GROUP 9 :
  * --------------
  * Structure: 4B
  *
  * Usage: MULTICAST BROADCAST COUNTERS
  *   Counter   | amount
  *   -------------------
  *   rx mcast packets       | 32
  *   rx broadcast packets   | 32
  *   tx mcast packets       | 32
  *   tx broadcast packets   | 32
  *
*******************************************************************************/
#define CNTR9_CNTR_NUM  128 /* needed for DPU G9991 project*/
#define PORT_MCST_BCST_GROUP_INVLID_CNTR (CNTR9_CNTR_NUM-1)

/* CNPL GROUP 12: 2B (packets) */
#define CNTR9_CNPL12_CNTR_SIZE 2
#define CNTR9_CNPL12_CNTR_TYPE 0
#define CNTR9_CNPL12_ADDR_SIZE (((CNTR9_CNTR_NUM * (1 << CNTR9_CNPL12_CNTR_SIZE) * (1 << CNTR9_CNPL12_CNTR_TYPE)) + 7) >> 3)

#else
/******************************************************************************
  *
  * CNTR GROUP 9 :
  * --------------
  * Structure: 4B
  *
  * Usage: FLOW CTRL COUNTERS
  *   Counter   | amount
  *   -------------------
  *   ingress packets bytes per emac  | 8
  *
*******************************************************************************/
#define CNTR9_CNTR_NUM  8
#define FLOW_CTRL_GROUP_INVLID_CNTR (CNTR9_CNTR_NUM-1)

/* CNPL GROUP 14: 4B (bytes) */
#define CNTR9_CNPL12_CNTR_SIZE 2
#define CNTR9_CNPL12_CNTR_TYPE 0
#define CNTR9_CNPL12_ADDR_SIZE (((CNTR9_CNTR_NUM * (1 << CNTR9_CNPL12_CNTR_SIZE) * (1 << CNTR9_CNPL12_CNTR_TYPE)) + 7) >> 3)
#endif

/******************************************************************************
  *
  * CNTR GROUP 10 :
  * --------------
  * Structure: 2B
  *
  * Usage: DHD
  *   Counter   | amount
  *   -------------------
  *   DHD_DROP   | 100
  *
*******************************************************************************/
#define CNTR10_CNTR_NUM  100
#define DHD_CNTR_GROUP_INVLID_CNTR (CNTR10_CNTR_NUM-1)

/* CNPL GROUP 13: 2B (packets) */
#define CNTR10_CNPL13_CNTR_SIZE 1
#define CNTR10_CNPL13_CNTR_TYPE 0
#define CNTR10_CNPL13_ADDR_SIZE (((CNTR10_CNTR_NUM * (1 << CNTR10_CNPL13_CNTR_SIZE) * (1 << CNTR10_CNPL13_CNTR_TYPE)) + 7) >> 3)

#define CNTR10_CNPL13_ADDR_SIZE_EMPTY (((1 * (1 << CNTR10_CNPL13_CNTR_SIZE) * (1 << CNTR10_CNPL13_CNTR_TYPE)) + 7) >> 3)

/******************************************************************************
  *
  * CNTR GROUP 11 :
  * --------------
  * Structure: 4B+4B
  *
  * Usage:
  *   Counter   | amount
  *   -------------------
  *   vlan rx   | SFU :128, on 6858 SFU 256
  *
*******************************************************************************/

#define CNTR11_CNTR_NUM (RDPA_MAX_VLANS)

#define CNTR11_CNPL14_CNTR_SIZE 2
#define CNTR11_CNPL14_CNTR_TYPE 1
#define CNTR11_CNPL14_ADDR_SIZE (((CNTR11_CNTR_NUM * (1 << CNTR11_CNPL14_CNTR_SIZE) * (1 << CNTR11_CNPL14_CNTR_TYPE)) + 7) >> 3)
#define CNTR11_CNPL14_ADDR_SIZE_EMPTY (((1 * (1 << CNTR11_CNPL14_CNTR_SIZE) * (1 << CNTR11_CNPL14_CNTR_TYPE)) + 7) >> 3)

/******************************************************************************
  *
  * CNTR GROUP 12 :
  * --------------
  * Structure: NATC group 4+4
  *
  * Usage:
  *   Counter   | amount
  *   -------------------
  *   IPTV      | 1024 + 1 (RDD_IPTV_DDR_CONTEXT_TABLE_SIZE)
  *
*******************************************************************************/

#define CNTR12_CNTR_NUM (RDD_IPTV_DDR_CONTEXT_TABLE_SIZE + 1)

/******************************************************************************
  *
  * CNTR GROUP 13 :
  * --------------
  * Structure: (4B+4B)
  *
  * Usage:
  *   Counter    | amount
  *   -------------------
  *   FW policer | 80
  *
  *  Please notice - these counters use the same memory of CNPL_POLICER - base address is CNPL_POLICER_PARAM_BASE_ADDR where the policers configuration located
  *  It must be 8 byte type (4B+4B) - as we define the hw policer.
*******************************************************************************/
#define CNTR13_CNTR_NUM  80
#define FW_POLICER_CNTR_GROUP_INVLID_CNTR (CNTR13_CNTR_NUM-1)

/* CNPL GROUP 09: 4B + 4B (valid bytes, valid packets) */
#define CNTR13_CNPL15_CNTR_SIZE 2
#define CNTR13_CNPL15_CNTR_TYPE 1
#define CNTR13_CNPL15_ADDR_SIZE (((CNTR13_CNTR_NUM * (1 << CNTR13_CNPL15_CNTR_SIZE) * (1 << CNTR13_CNPL15_CNTR_TYPE)) + 7) >> 3)

/******************************************************************************
*******************************************************************************/

#define CNPL_MEMORY_END_ADDR (CNPL_MEMORY_DATA_REG_RAM_CNT * 4)

#define CNTR_POLICERS_SIZE (((CNPL_POLICER_NUM * CNPL_POLICER_SIZE) >> 3) * 2 )

#define CNTR_END_ADDR ((CNPL_MEMORY_END_ADDR >> 3) - CNTR_POLICERS_SIZE)

#define TCAM_DEF_CNTR_GROUP_INVLID_CNTR rdp_drv_proj_cntr_get_tcam_def_invalid_id()

bdmf_error_t rdp_drv_proj_cntr_init(bdmf_boolean is_gate_way, bdmf_boolean vlan_stats_enable);
uint32_t rdp_drv_proj_cntr_get_tcam_def_invalid_id(void);

#endif
