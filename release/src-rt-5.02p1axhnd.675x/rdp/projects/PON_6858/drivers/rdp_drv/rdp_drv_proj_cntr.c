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
#include "rdd_data_structures_auto.h"
#include "rdd_defs.h"
#include "bdmf_interface.h"


bdmf_boolean cntr_group_0_occupied[CNTR0_CNTR_NUM]={};
bdmf_boolean cntr_group_1_occupied[CNTR1_CNTR_NUM]={};
bdmf_boolean cntr_group_2_occupied[CNTR2_CNTR_NUM_WITHOUT_VLAN]={};
bdmf_boolean cntr_group_3_occupied[CNTR3_CNTR_NUM]={};
bdmf_boolean cntr_group_4_occupied[CNTR4_CNTR_NUM]={};
bdmf_boolean cntr_group_5_occupied[CNTR5_CNTR_NUM]={};
bdmf_boolean cntr_group_6_occupied[CNTR6_CNTR_NUM]={};
bdmf_boolean cntr_group_7_occupied[CNTR7_CNTR_NUM]={};
bdmf_boolean cntr_group_8_occupied[CNTR8_CNTR_NUM]={};
bdmf_boolean cntr_group_9_occupied[CNTR9_CNTR_NUM]={};
bdmf_boolean cntr_group_10_occupied[CNTR10_CNTR_NUM]={};
bdmf_boolean cntr_group_11_occupied[CNTR11_CNTR_NUM]={};
bdmf_boolean cntr_group_12_occupied[CNTR12_CNTR_NUM]={};
bdmf_boolean cntr_group_13_occupied[CNTR13_CNTR_NUM]={};

cntr_group_cfg_t cntr_group_cfg[CNTR_GROUP_GROUPS_NUMBER];
/* this variable is used to know index for invalid counter in IC */
static uint32_t g_tcam_def_invalid_id;

uint32_t rdp_drv_proj_cntr_get_tcam_def_invalid_id(void)
{
    return g_tcam_def_invalid_id;
}

void rdp_drv_proj_cntr_set_group_cfg(cnpl_group_cfg_t * group_cfg, uint32_t group_id, uint32_t base_addr,
        uint32_t cntr_size, bdmf_boolean cntr_type, bdmf_boolean wrap_around, bdmf_boolean clr_on_read,
        bdmf_boolean valid)
{
    group_cfg->group_id = group_id;
    group_cfg->base_addr = base_addr;
    group_cfg->cntr_size = cntr_size;
    group_cfg->cntr_type = cntr_type;
    group_cfg->wrap_around = wrap_around;
    group_cfg->clr_on_read = clr_on_read;
    group_cfg->valid = valid;
}

bdmf_error_t rdp_drv_proj_cntr_init(bdmf_boolean is_gate_way, bdmf_boolean vlan_stats_enable)
{
    uint32_t baseAddr;
    bdmf_error_t rc = BDMF_ERR_OK;
#if defined(G9991)
    is_gate_way = 0; /*(DPU profile same as not gateway */
#endif

    /* RX_FLOW_CNTR_GROUP_ID */
    cntr_group_cfg[CNTR_GROUP_RX_FLOW].group_id = CNTR_GROUP_RX_FLOW;
    cntr_group_cfg[CNTR_GROUP_RX_FLOW].cntr_occuiped_arr = cntr_group_0_occupied;
    cntr_group_cfg[CNTR_GROUP_RX_FLOW].hw_type = cntr_cnpl;
    cntr_group_cfg[CNTR_GROUP_RX_FLOW].cntr_number = CNTR0_CNTR_NUM;
    baseAddr = 0;
    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_RX_FLOW].cnpl_group_cfg[0], 0, baseAddr,
        CNTR0_CNPL0_CNTR_SIZE, CNTR0_CNPL0_CNTR_TYPE, 1, 1, 1);
    baseAddr += CNTR0_CNPL0_ADDR_SIZE;
    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_RX_FLOW].cnpl_group_cfg[1], 1, baseAddr,
    CNTR0_CNPL1_CNTR_SIZE, CNTR0_CNPL1_CNTR_TYPE, 1,  1, 1);
    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_RX_FLOW].cnpl_group_cfg[2], 0, 0, 0, 0, 0, 0, 0);

    /* TX_FLOW_CNTR_GROUP_ID */
    cntr_group_cfg[CNTR_GROUP_TX_FLOW].group_id = CNTR_GROUP_TX_FLOW;
    cntr_group_cfg[CNTR_GROUP_TX_FLOW].cntr_occuiped_arr = cntr_group_1_occupied;
    cntr_group_cfg[CNTR_GROUP_TX_FLOW].hw_type = cntr_cnpl;
    cntr_group_cfg[CNTR_GROUP_TX_FLOW].cntr_number = CNTR1_CNTR_NUM;
    baseAddr += CNTR0_CNPL1_ADDR_SIZE;
    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_TX_FLOW].cnpl_group_cfg[0], 2, baseAddr,
        CNTR1_CNPL2_CNTR_SIZE, CNTR1_CNPL2_CNTR_TYPE, 1,  1, 1);
    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_TX_FLOW].cnpl_group_cfg[1], 0, 0, 0, 0, 0, 0, 0);
    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_TX_FLOW].cnpl_group_cfg[2], 0, 0, 0, 0, 0, 0, 0);

    /* TCAM_DEF_CNTR_GROUP_ID */
    cntr_group_cfg[CNTR_GROUP_TCAM_DEF].group_id = CNTR_GROUP_TCAM_DEF;
    cntr_group_cfg[CNTR_GROUP_TCAM_DEF].cntr_occuiped_arr = cntr_group_2_occupied;
    cntr_group_cfg[CNTR_GROUP_TCAM_DEF].hw_type = cntr_cnpl;
    cntr_group_cfg[CNTR_GROUP_TCAM_DEF].cntr_number = vlan_stats_enable? CNTR2_CNTR_NUM_WITH_VLAN : CNTR2_CNTR_NUM_WITHOUT_VLAN;
    g_tcam_def_invalid_id = cntr_group_cfg[CNTR_GROUP_TCAM_DEF].cntr_number - 1;
    baseAddr += CNTR1_CNPL2_ADDR_SIZE;
    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_TCAM_DEF].cnpl_group_cfg[0], 3, baseAddr,
        CNTR2_CNPL3_CNTR_SIZE, CNTR2_CNPL3_CNTR_TYPE, 1,  1, 1);
    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_TCAM_DEF].cnpl_group_cfg[1], 0, 0, 0, 0, 0, 0, 0);
    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_TCAM_DEF].cnpl_group_cfg[2], 0, 0, 0, 0, 0, 0, 0);

    /* VARIOUS_CNTR_GROUP_ID */
    cntr_group_cfg[CNTR_GROUP_VARIOUS].group_id = CNTR_GROUP_VARIOUS;
    cntr_group_cfg[CNTR_GROUP_VARIOUS].cntr_occuiped_arr = cntr_group_3_occupied;
    cntr_group_cfg[CNTR_GROUP_VARIOUS].hw_type = cntr_cnpl;
    cntr_group_cfg[CNTR_GROUP_VARIOUS].cntr_number = CNTR3_CNTR_NUM;
    baseAddr += vlan_stats_enable?CNTR2_CNPL3_ADDR_SIZE_WITH_VLAN:CNTR2_CNPL3_ADDR_SIZE_WITHOUT_VLAN;
    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_VARIOUS].cnpl_group_cfg[0], 4, baseAddr,
        CNTR3_CNPL4_CNTR_SIZE, CNTR3_CNPL4_CNTR_TYPE, 1,  1, 1);
    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_VARIOUS].cnpl_group_cfg[1], 0, 0, 0, 0, 0, 0, 0);
    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_VARIOUS].cnpl_group_cfg[2], 0, 0, 0, 0, 0, 0, 0);

    /* GENERAL_CNTR_GROUP_ID */
    cntr_group_cfg[CNTR_GROUP_GENERAL].group_id = CNTR_GROUP_GENERAL;
    cntr_group_cfg[CNTR_GROUP_GENERAL].cntr_occuiped_arr = cntr_group_4_occupied;
    cntr_group_cfg[CNTR_GROUP_GENERAL].hw_type = cntr_cnpl;
    cntr_group_cfg[CNTR_GROUP_GENERAL].cntr_number = CNTR4_CNTR_NUM;
    baseAddr += CNTR3_CNPL4_ADDR_SIZE;
    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_GENERAL].cnpl_group_cfg[0], 5, baseAddr,
        CNTR4_CNPL5_CNTR_SIZE, CNTR4_CNPL5_CNTR_TYPE, 1,  1, 1);
    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_GENERAL].cnpl_group_cfg[1], 0, 0, 0, 0, 0, 0, 0);
    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_GENERAL].cnpl_group_cfg[2], 0, 0, 0, 0, 0, 0, 0);

    /* TX_QUEUE_CNTR_GROUP_ID */
    cntr_group_cfg[CNTR_GROUP_TX_QUEUE].group_id = CNTR_GROUP_TX_QUEUE;
    cntr_group_cfg[CNTR_GROUP_TX_QUEUE].cntr_occuiped_arr = cntr_group_5_occupied;
    cntr_group_cfg[CNTR_GROUP_TX_QUEUE].hw_type = cntr_cnpl;
    cntr_group_cfg[CNTR_GROUP_TX_QUEUE].cntr_number = CNTR5_CNTR_NUM;
    baseAddr += CNTR4_CNPL5_ADDR_SIZE;
    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_TX_QUEUE].cnpl_group_cfg[0], 6, baseAddr,
        CNTR5_CNPL6_CNTR_SIZE, CNTR5_CNPL6_CNTR_TYPE, 1,  1, 1);
    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_TX_QUEUE].cnpl_group_cfg[1], 0, 0, 0, 0, 0, 0, 0);
    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_TX_QUEUE].cnpl_group_cfg[2], 0, 0, 0, 0, 0, 0, 0);

    /* DHD_CTR_CNTR_GROUP_ID */
    cntr_group_cfg[CNTR_GROUP_DHD_CTR].group_id = CNTR_GROUP_DHD_CTR;
    cntr_group_cfg[CNTR_GROUP_DHD_CTR].cntr_occuiped_arr = cntr_group_6_occupied;
    cntr_group_cfg[CNTR_GROUP_DHD_CTR].hw_type = cntr_cnpl;
    cntr_group_cfg[CNTR_GROUP_DHD_CTR].cntr_number = CNTR6_CNTR_NUM;
    baseAddr += CNTR5_CNPL6_ADDR_SIZE;
    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_DHD_CTR].cnpl_group_cfg[0], 7, baseAddr,
        CNTR6_CNPL7_CNTR_SIZE, CNTR6_CNPL7_CNTR_TYPE, 0,  0, 1);
    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_DHD_CTR].cnpl_group_cfg[1], 0, 0, 0, 0, 0, 0, 0);
    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_DHD_CTR].cnpl_group_cfg[2], 0, 0, 0, 0, 0, 0, 0);

    /* DHD_CNTR_GROUP_ID */
    cntr_group_cfg[CNTR_GROUP_CPU_RX_METER_DROP].group_id = CNTR_GROUP_CPU_RX_METER_DROP;
    cntr_group_cfg[CNTR_GROUP_CPU_RX_METER_DROP].cntr_occuiped_arr = cntr_group_7_occupied;
    cntr_group_cfg[CNTR_GROUP_CPU_RX_METER_DROP].hw_type = cntr_cnpl;
    cntr_group_cfg[CNTR_GROUP_CPU_RX_METER_DROP].cntr_number = CNTR7_CNTR_NUM;
    baseAddr += CNTR6_CNPL7_ADDR_SIZE;
    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_CPU_RX_METER_DROP].cnpl_group_cfg[0], 8, baseAddr,
        CNTR7_CNPL8_CNTR_SIZE, CNTR7_CNPL8_CNTR_TYPE, 1,  1, 1);
    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_CPU_RX_METER_DROP].cnpl_group_cfg[1], 0, 0, 0, 0, 0, 0, 0);
    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_CPU_RX_METER_DROP].cnpl_group_cfg[2], 0, 0, 0, 0, 0, 0, 0);

    /* POLICER_CNTR_GROUP_ID */
    cntr_group_cfg[CNTR_GROUP_POLICER].group_id = CNTR_GROUP_POLICER;
    cntr_group_cfg[CNTR_GROUP_POLICER].cntr_occuiped_arr = cntr_group_8_occupied;
    cntr_group_cfg[CNTR_GROUP_POLICER].hw_type = cntr_cnpl;
    cntr_group_cfg[CNTR_GROUP_POLICER].cntr_number = CNTR8_CNTR_NUM;
    baseAddr += CNTR7_CNPL8_ADDR_SIZE;
    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_POLICER].cnpl_group_cfg[0], 9, baseAddr,
        CNTR8_CNPL9_CNTR_SIZE, CNTR8_CNPL9_CNTR_TYPE, 1,  1, 1);
    baseAddr += CNTR8_CNPL9_ADDR_SIZE;
    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_POLICER].cnpl_group_cfg[1], 10, baseAddr,
        CNTR8_CNPL10_CNTR_SIZE, CNTR8_CNPL10_CNTR_TYPE, 1,  1, 1);
    baseAddr += CNTR8_CNPL10_ADDR_SIZE;
    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_POLICER].cnpl_group_cfg[2], 11, baseAddr,
        CNTR8_CNPL11_CNTR_SIZE, CNTR8_CNPL11_CNTR_TYPE, 1,  1, 1);

#ifdef G9991
    /* PORT_MCST_BCST_GROUP_ID */
    cntr_group_cfg[CNTR_GROUP_PORT_MCST_BCST].group_id = CNTR_GROUP_PORT_MCST_BCST;
    cntr_group_cfg[CNTR_GROUP_PORT_MCST_BCST].cntr_occuiped_arr = cntr_group_9_occupied;
    cntr_group_cfg[CNTR_GROUP_PORT_MCST_BCST].hw_type = cntr_cnpl;
    cntr_group_cfg[CNTR_GROUP_PORT_MCST_BCST].cntr_number = CNTR9_CNTR_NUM;
    baseAddr += CNTR8_CNPL11_ADDR_SIZE;
    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_PORT_MCST_BCST].cnpl_group_cfg[0], 12, baseAddr,
        CNTR9_CNPL12_CNTR_SIZE, CNTR9_CNPL12_CNTR_TYPE, 1, 1, 1);
    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_PORT_MCST_BCST].cnpl_group_cfg[1], 0, 0, 0, 0, 0, 0, 0);
    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_PORT_MCST_BCST].cnpl_group_cfg[2], 0, 0, 0, 0, 0, 0, 0);
#else
    /* EMAC_FLOW_CTRL_CNTR_GROUP_ID */
    cntr_group_cfg[CNTR_GROUP_EMAC_FLOW_CTRL].group_id = CNTR_GROUP_EMAC_FLOW_CTRL;
    cntr_group_cfg[CNTR_GROUP_EMAC_FLOW_CTRL].cntr_occuiped_arr = cntr_group_9_occupied;
    cntr_group_cfg[CNTR_GROUP_EMAC_FLOW_CTRL].hw_type = cntr_cnpl;
    cntr_group_cfg[CNTR_GROUP_EMAC_FLOW_CTRL].cntr_number = CNTR9_CNTR_NUM;
    baseAddr += CNTR8_CNPL11_ADDR_SIZE;
    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_EMAC_FLOW_CTRL].cnpl_group_cfg[0], 12, baseAddr,
        CNTR9_CNPL12_CNTR_SIZE, CNTR9_CNPL12_CNTR_TYPE, 0, 0, 1);
    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_EMAC_FLOW_CTRL].cnpl_group_cfg[1], 0, 0, 0, 0, 0, 0, 0);
    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_EMAC_FLOW_CTRL].cnpl_group_cfg[2], 0, 0, 0, 0, 0, 0, 0);
#endif

    /* DHD_CNTRS_GROUP_ID */
    cntr_group_cfg[CNTR_GROUP_DHD_CNTRS].group_id = CNTR_GROUP_DHD_CNTRS;
    cntr_group_cfg[CNTR_GROUP_DHD_CNTRS].cntr_occuiped_arr = cntr_group_10_occupied;
    cntr_group_cfg[CNTR_GROUP_DHD_CNTRS].hw_type = cntr_cnpl;
    cntr_group_cfg[CNTR_GROUP_DHD_CNTRS].cntr_number = is_gate_way ? CNTR10_CNTR_NUM : 1;
    baseAddr +=  CNTR9_CNPL12_ADDR_SIZE;
    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_DHD_CNTRS].cnpl_group_cfg[0], 13, baseAddr,
        CNTR10_CNPL13_CNTR_SIZE, CNTR10_CNPL13_CNTR_TYPE, 1, 1, 1);
    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_DHD_CNTRS].cnpl_group_cfg[1], 0, 0, 0, 0, 0, 0, 0);
    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_DHD_CNTRS].cnpl_group_cfg[2], 0, 0, 0, 0, 0, 0, 0);

    /* VLAN_RX_GROUP_ID */
    cntr_group_cfg[CNTR_GROUP_VLAN_RX].group_id = CNTR_GROUP_VLAN_RX;
    cntr_group_cfg[CNTR_GROUP_VLAN_RX].cntr_occuiped_arr = cntr_group_11_occupied;
    cntr_group_cfg[CNTR_GROUP_VLAN_RX].hw_type = cntr_cnpl;
    cntr_group_cfg[CNTR_GROUP_VLAN_RX].cntr_number = (is_gate_way || (!vlan_stats_enable))? 1 : CNTR11_CNTR_NUM;
    baseAddr += is_gate_way ? CNTR10_CNPL13_ADDR_SIZE : CNTR10_CNPL13_ADDR_SIZE_EMPTY;
    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_VLAN_RX].cnpl_group_cfg[0], 14, baseAddr,
        CNTR11_CNPL14_CNTR_SIZE, CNTR11_CNPL14_CNTR_TYPE, 0, 1, 1);
    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_VLAN_RX].cnpl_group_cfg[1], 0, 0, 0, 0, 0, 0, 0);
    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_VLAN_RX].cnpl_group_cfg[2], 0, 0, 0, 0, 0, 0, 0);

    baseAddr += (is_gate_way || (!vlan_stats_enable)) ? CNTR11_CNPL14_ADDR_SIZE_EMPTY : CNTR11_CNPL14_ADDR_SIZE;

    RDD_BTRACE("counters are using %d bytes of %d bytes", (baseAddr * 8) + (CNTR_POLICERS_SIZE * 8), CNPL_MEMORY_END_ADDR );

    if ((baseAddr) > CNTR_END_ADDR)
    {
        rc = BDMF_ERR_NORES;
        BDMF_TRACE_RET(rc, "error couldn't allocate enough counters, required = %d, exist = %d\n", (baseAddr * 8) + (CNTR_POLICERS_SIZE * 8), CNPL_MEMORY_END_ADDR);
    }

    /* IPTV NATC group */
    cntr_group_cfg[CNTR_GROUP_IPTV_NATC].group_id = CNTR_GROUP_IPTV_NATC;
    cntr_group_cfg[CNTR_GROUP_IPTV_NATC].cntr_occuiped_arr = cntr_group_12_occupied;
    cntr_group_cfg[CNTR_GROUP_IPTV_NATC].hw_type = cntr_natc;
    cntr_group_cfg[CNTR_GROUP_IPTV_NATC].cntr_number = CNTR12_CNTR_NUM;
    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_IPTV_NATC].cnpl_group_cfg[0], 0, 0, 0, 0, 0, 0, 0);
    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_IPTV_NATC].cnpl_group_cfg[1], 0, 0, 0, 0, 0, 0, 0);
    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_IPTV_NATC].cnpl_group_cfg[2], 0, 0, 0, 0, 0, 0, 0);

    /* FW_POLICER_CNTR_GROUP_ID */
    cntr_group_cfg[CNTR_GROUP_FW_POLICER].group_id = CNTR_GROUP_FW_POLICER;
    cntr_group_cfg[CNTR_GROUP_FW_POLICER].cntr_occuiped_arr = cntr_group_13_occupied;
    cntr_group_cfg[CNTR_GROUP_FW_POLICER].hw_type = cntr_cnpl;
    cntr_group_cfg[CNTR_GROUP_FW_POLICER].cntr_number = CNTR13_CNTR_NUM;

    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_FW_POLICER].cnpl_group_cfg[0], 15, CNPL_POLICER_PARAM_BASE_ADDR,
        CNTR13_CNPL15_CNTR_SIZE, CNTR13_CNPL15_CNTR_TYPE, 0,  0, 1);
    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_FW_POLICER].cnpl_group_cfg[1], 0, 0, 0, 0, 0, 0, 0);
    rdp_drv_proj_cntr_set_group_cfg(&cntr_group_cfg[CNTR_GROUP_FW_POLICER].cnpl_group_cfg[2], 0, 0, 0, 0, 0, 0, 0);

    return rc;
}

