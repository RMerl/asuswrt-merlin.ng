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


#include "bdmf_data_types.h"
#include "bdmf_errno.h"
#include "rdp_drv_proj_cntr.h"

bdmf_boolean cntr_group_0_occupied[CNTR0_CNTR_NUM]={};
bdmf_boolean cntr_group_1_occupied[CNTR1_CNTR_NUM]={};
bdmf_boolean cntr_group_2_occupied[CNTR2_CNTR_NUM]={};
bdmf_boolean cntr_group_3_occupied[CNTR3_CNTR_NUM]={};
bdmf_boolean cntr_group_4_occupied[CNTR4_CNTR_NUM]={};
bdmf_boolean cntr_group_5_occupied[CNTR5_CNTR_NUM]={};
bdmf_boolean cntr_group_6_occupied[CNTR6_CNTR_NUM]={};
bdmf_boolean cntr_group_7_occupied[CNTR7_CNTR_NUM]={};
bdmf_boolean cntr_group_8_occupied[CNTR8_CNTR_NUM]={};
bdmf_boolean cntr_group_9_occupied[CNTR9_CNTR_NUM]={};
bdmf_boolean cntr_group_10_occupied[CNTR10_CNTR_NUM]={}; /*used in DPU G9991 project*/
bdmf_boolean cntr_group_11_occupied[CNTR11_CNTR_NUM]={};
bdmf_boolean cntr_group_12_occupied[CNTR12_CNTR_NUM]={};

cntr_group_cfg_t cntr_group_cfg[CNTR_GROUP_GROUPS_NUMBER] = {
     {CNTR_GROUP_RX_FLOW, cntr_group_0_occupied, CNTR0_CNTR_NUM,
      {{0, CNTR0_CNPL0_BASE_ADDR, CNTR0_CNPL0_CNTR_SIZE, CNTR0_CNPL0_CNTR_TYPE, 1,  1, 1},
       {1, CNTR0_CNPL1_BASE_ADDR, CNTR0_CNPL1_CNTR_SIZE, CNTR0_CNPL1_CNTR_TYPE, 1,  1, 1},
       {}}},
     {CNTR_GROUP_TX_FLOW, cntr_group_1_occupied, CNTR1_CNTR_NUM,
      {{2, CNTR1_CNPL2_BASE_ADDR, CNTR1_CNPL2_CNTR_SIZE, CNTR1_CNPL2_CNTR_TYPE, 1,  1, 1},
       {},
       {}}},
     {CNTR_GROUP_TCAM_DEF, cntr_group_2_occupied, CNTR2_CNTR_NUM,
      {{3, CNTR2_CNPL3_BASE_ADDR, CNTR2_CNPL3_CNTR_SIZE, CNTR2_CNPL3_CNTR_TYPE, 1,  0, 1},
       {},
       {}}},
     {CNTR_GROUP_VARIOUS, cntr_group_3_occupied, CNTR3_CNTR_NUM,
      {{4, CNTR3_CNPL4_BASE_ADDR, CNTR3_CNPL4_CNTR_SIZE, CNTR3_CNPL4_CNTR_TYPE, 1,  1, 1},
       {},
       {}}},
     {CNTR_GROUP_GENERAL, cntr_group_4_occupied, CNTR4_CNTR_NUM,
      {{5, CNTR4_CNPL5_BASE_ADDR, CNTR4_CNPL5_CNTR_SIZE, CNTR4_CNPL5_CNTR_TYPE, 1,  1, 1},
       {},
       {}}},
     {CNTR_GROUP_TX_QUEUE, cntr_group_5_occupied, CNTR5_CNTR_NUM,
      {{6, CNTR5_CNPL6_BASE_ADDR, CNTR5_CNPL6_CNTR_SIZE, CNTR5_CNPL6_CNTR_TYPE, 1,  1, 1},
       {},
       {}}},
     {CNTR_GROUP_DHD_CTR, cntr_group_6_occupied, CNTR6_CNTR_NUM,
      {{7, CNTR6_CNPL7_BASE_ADDR, CNTR6_CNPL7_CNTR_SIZE, CNTR6_CNPL7_CNTR_TYPE, 0,  0, 1},
       {},
       {}}},
     {CNTR_GROUP_CPU_RX_METER_DROP, cntr_group_7_occupied, CNTR7_CNTR_NUM,
      {{8, CNTR7_CNPL8_BASE_ADDR, CNTR7_CNPL8_CNTR_SIZE, CNTR7_CNPL8_CNTR_TYPE, 1,  1, 1},
       {},
       {}}},
     {CNTR_GROUP_POLICER, cntr_group_8_occupied, CNTR8_CNTR_NUM,
      {{9, CNTR8_CNPL9_BASE_ADDR, CNTR8_CNPL9_CNTR_SIZE, CNTR8_CNPL9_CNTR_TYPE, 1,  0, 1},
       {10, CNTR8_CNPL10_BASE_ADDR, CNTR8_CNPL10_CNTR_SIZE, CNTR8_CNPL10_CNTR_TYPE, 1,  0, 1},
       {11, CNTR8_CNPL11_BASE_ADDR, CNTR8_CNPL11_CNTR_SIZE, CNTR8_CNPL11_CNTR_TYPE, 1,  0, 1}}},
     {CNTR_GROUP_PATHSTAT, cntr_group_9_occupied, CNTR9_CNTR_NUM,
      {{12, CNTR9_CNPL12_BASE_ADDR, CNTR9_CNPL12_CNTR_SIZE, CNTR9_CNPL12_CNTR_TYPE, 0, 1, 1},   /* no_wrap_around, clr_on_read=1, valid=1 */
       {},
       {}}},
     {CNTR_GROUP_DHD_CNTRS, cntr_group_10_occupied, CNTR10_CNTR_NUM,
       {{13, CNTR10_CNPL13_BASE_ADDR, CNTR10_CNPL13_CNTR_SIZE, CNTR10_CNPL13_CNTR_TYPE, 1, 1, 1},
       {},
       {}}},
     {CNTR_GROUP_FLOW_CTRL, cntr_group_11_occupied, CNTR11_CNTR_NUM,
       {{14, CNTR11_CNPL14_BASE_ADDR, CNTR11_CNPL14_CNTR_SIZE, CNTR11_CNPL14_CNTR_TYPE, 0, 0, 1},
       {},
       {}}},
     {CNTR_GROUP_IPTV_NATC, cntr_group_12_occupied, CNTR12_CNTR_NUM,
       {{},
       {},
       {}},
       cntr_natc},
       
};

