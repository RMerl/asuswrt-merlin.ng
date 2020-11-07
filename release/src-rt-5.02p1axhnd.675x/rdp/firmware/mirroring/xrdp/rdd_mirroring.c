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


#include "rdd.h"
#include "rdd_common.h"
#include "rdd_mirroring.h"

#ifdef CONFIG_DHD_RUNNER
#include "rdpa_dhd_helper_basic.h"
#endif

void rdd_mirroring_set_rx(rdd_mirroring_cfg_t *rdd_mirroring_cfg)
{
    RDD_BTRACE("Configuring RDD mirroring: rx_dst_queue = %d, rx_dst_vport = %d\n", rdd_mirroring_cfg->rx_dst_queue, (uint16_t)rdd_mirroring_cfg->rx_dst_vport);

    RDD_MIRRORING_DESCRIPTOR_DST_QUEUE_WRITE_G(rdd_mirroring_cfg->rx_dst_queue,
        RDD_RX_MIRRORING_CONFIGURATION_ADDRESS_ARR, 0);
    RDD_MIRRORING_DESCRIPTOR_DST_VPORT_WRITE_G(rdd_mirroring_cfg->rx_dst_vport,
        RDD_RX_MIRRORING_CONFIGURATION_ADDRESS_ARR, 0);
}

void rdd_mirroring_set_tx(rdd_mirroring_cfg_t *rdd_mirroring_cfg)
{
    RDD_MIRRORING_DESCRIPTOR_DST_QUEUE_WRITE_G(rdd_mirroring_cfg->tx_dst_queue,
        RDD_TX_MIRRORING_CONFIGURATION_ADDRESS_ARR, 0);
    RDD_MIRRORING_DESCRIPTOR_DST_VPORT_WRITE_G(rdd_mirroring_cfg->tx_dst_vport,
        RDD_TX_MIRRORING_CONFIGURATION_ADDRESS_ARR, 0);

    
    rdd_mirroring_set_tx_src_en(rdd_mirroring_cfg->lan, rdd_mirroring_cfg->wlan_radio_idx, rdd_mirroring_cfg->src_tx_bbh_id, IS_MIRRORING_CFG(rdd_mirroring_cfg->tx_dst_queue));


    if (rdd_mirroring_cfg->lan)
    {
        RDD_BTRACE("Configuring RDD LAN mirroring: src_tx_bbh_id = %d, tx_dst_queue = %d\n",
            rdd_mirroring_cfg->src_tx_bbh_id, rdd_mirroring_cfg->tx_dst_queue);
    }
#ifdef CONFIG_DHD_RUNNER
    else if ((rdd_mirroring_cfg->wlan_radio_idx >= 0) && (rdd_mirroring_cfg->wlan_radio_idx < RDPA_MAX_RADIOS))
    {
        RDD_DHD_POST_COMMON_RADIO_ENTRY_DST_VPORT_WRITE_G(rdd_mirroring_cfg->tx_dst_vport, RDD_DHD_POST_COMMON_RADIO_DATA_ADDRESS_ARR, rdd_mirroring_cfg->wlan_radio_idx);
        RDD_DHD_POST_COMMON_RADIO_ENTRY_DST_QUEUE_WRITE_G(rdd_mirroring_cfg->tx_dst_queue, RDD_DHD_POST_COMMON_RADIO_DATA_ADDRESS_ARR, rdd_mirroring_cfg->wlan_radio_idx);
        RDD_BTRACE("Configuring RDD WLAN mirroring: tx_dst_queue = %d\n", rdd_mirroring_cfg->tx_dst_queue);
    }
#endif
    else
    {
        /* Tx mirror enable bit in general configuration is used for WAN only (EPON task) */
        /* for other ports bit is tested in DHD or BBH configurations. Should not set it for LAN/WLAN !!! */
        RDD_MIRRORING_DESCRIPTOR_MIRROR_EN_WRITE_G(IS_MIRRORING_CFG(rdd_mirroring_cfg->tx_dst_queue),
            RDD_TX_MIRRORING_CONFIGURATION_ADDRESS_ARR, 0);
        
        RDD_BTRACE("Configuring RDD WAN mirroring: tx_dst_queue = %d\n", rdd_mirroring_cfg->tx_dst_queue);
    }
}

void rdd_mirror_tx_disable(void)
{
    int bbh_id;

    /* Tx mirror enable bit in general configuration is used for WAN only (EPON task) */
    /* for other ports bit is tested in DHD or BBH configurations. Should not set it for LAN/WLAN !!! */

    for (bbh_id = 0; bbh_id < RDD_DS_TM_BBH_QUEUE_TABLE_SIZE; bbh_id++)
        rdd_mirroring_set_tx_src_en(1, -1, bbh_id, 0);

    RDD_MIRRORING_DESCRIPTOR_MIRROR_EN_WRITE_G(0, RDD_TX_MIRRORING_CONFIGURATION_ADDRESS_ARR, 0);    

    rdd_mirroring_set_tx_src_en(0, -1, 0, 0); /* clear wan tx mirror cfg */

    RDD_MIRRORING_DESCRIPTOR_DST_QUEUE_WRITE_G(0, RDD_TX_MIRRORING_CONFIGURATION_ADDRESS_ARR, 0);
    RDD_MIRRORING_DESCRIPTOR_DST_VPORT_WRITE_G(0, RDD_TX_MIRRORING_CONFIGURATION_ADDRESS_ARR, 0);
   
#ifdef CONFIG_DHD_RUNNER
    {
        int wlan_radio_idx;
        for (wlan_radio_idx = 0;  wlan_radio_idx < RDPA_MAX_RADIOS; wlan_radio_idx++)
        {
            rdd_mirroring_set_tx_src_en(0, wlan_radio_idx, 0, 0); /* clear wan dhd mirror cfg */
            RDD_DHD_POST_COMMON_RADIO_ENTRY_DST_VPORT_WRITE_G(0, RDD_DHD_POST_COMMON_RADIO_DATA_ADDRESS_ARR, wlan_radio_idx);
            RDD_DHD_POST_COMMON_RADIO_ENTRY_DST_QUEUE_WRITE_G(0, RDD_DHD_POST_COMMON_RADIO_DATA_ADDRESS_ARR, wlan_radio_idx);
        }
    }
#endif

    RDD_BTRACE("Disabling RDD TX mirroring\n");
}

void rdd_mirroring_set_tx_src_en(bdmf_boolean is_lan, int wlan_radio_idx, bbh_id_e src_tx_bbh_id, bdmf_boolean mirror_enable)
{
    int i;
    uint32_t RDD_BBH_QUEUE_TM_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

    if (is_lan)
    {
        for (i = 0; i < GROUPED_EN_SEGMENTS_NUM; i++)
        {
            RDD_BBH_QUEUE_TM_ADDRESS_ARR[i] = (IS_DS_TM_RUNNER_IMAGE(i)) ?
                RDD_DS_TM_BBH_QUEUE_TABLE_ADDRESS_ARR[i] : INVALID_TABLE_ADDRESS;
        }
        /* For LAN Configure mirroring only for target BBH */
        RDD_BBH_QUEUE_DESCRIPTOR_MIRRORING_EN_WRITE_G(mirror_enable,
            RDD_BBH_QUEUE_TM_ADDRESS_ARR, src_tx_bbh_id);
    }
#ifdef CONFIG_DHD_RUNNER
    else if ((wlan_radio_idx >= 0) && (wlan_radio_idx < RDPA_MAX_RADIOS))
    {
        /* write enable */
        RDD_DHD_POST_COMMON_RADIO_ENTRY_TX_MIRRORING_EN_WRITE_G(mirror_enable, RDD_DHD_POST_COMMON_RADIO_DATA_ADDRESS_ARR, wlan_radio_idx);
    }
#endif
    else /* WAN mirroring */
    {
        for (i = 0; i < GROUPED_EN_SEGMENTS_NUM; i++)
        {
            RDD_BBH_QUEUE_TM_ADDRESS_ARR[i] = (get_runner_idx(us_tm_runner_image) == i) ?
                RDD_US_TM_BBH_QUEUE_TABLE_ADDRESS_ARR[i] : INVALID_TABLE_ADDRESS;
        }
        for (i = 0; i < RDD_BBH_QUEUE_TABLE_SIZE; i++)
        {
            /* For WAN Configure mirroring only for all TCONTS/LLIDs */
            RDD_BBH_QUEUE_DESCRIPTOR_MIRRORING_EN_WRITE_G(mirror_enable, 
                RDD_BBH_QUEUE_TM_ADDRESS_ARR ,i);
        }
    }
}

void rdd_mirroring_set(rdd_mirroring_cfg_t *rdd_mirroring_cfg)
{
    rdd_mirroring_set_rx(rdd_mirroring_cfg);
    rdd_mirroring_set_tx(rdd_mirroring_cfg);
}

