/*
* <:copyright-BRCM:2016:proprietary:standard
* 
*    Copyright (c) 2016 Broadcom 
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

#include "bdmf_dev.h"
#include "rdpa_api.h"
#include "rdpa_int.h"
#include "rdpa_common.h"
#include "rdd.h"
#include "rdd_dhd_helper.h"
#include "rdpa_platform.h"
#include "rdpa_dhd_helper_basic.h"
#include "rdpa_dhd_helper_ex.h"

#ifdef BDMF_SYSTEM_SIM
/* We use statically allocated addresses in the simulator */
rdpa_dhd_init_cfg_t sim_init_cfg = 
{
    .rx_post_flow_ring_base_addr = (void *)DHD_RX_POST_DDR_BUFFER_ADDRESS,
    .tx_post_flow_ring_base_addr = (void *)DHD_TX_POST_DDR_BUFFER_ADDRESS, 
    .rx_complete_flow_ring_base_addr = (void *)DHD_RX_COMPLETE_DDR_BUFFER_ADDRESS,
    .tx_complete_flow_ring_base_addr = (void *)DHD_TX_COMPLETE_DDR_BUFFER_ADDRESS,
    .r2d_wr_arr_base_addr = (void *)R2D_WR_ARR_DDR_BUFFER_ADDRESS,
    .d2r_rd_arr_base_addr = (void *)D2R_RD_ARR_DDR_BUFFER_ADDRESS,
    .r2d_rd_arr_base_addr = (void *)R2D_RD_ARR_DDR_BUFFER_ADDRESS,
    .d2r_wr_arr_base_addr = (void *)D2R_WR_ARR_DDR_BUFFER_ADDRESS,
    .tx_post_mgmt_arr_base_addr = (void *)DHD_TX_POST_FLOW_RING_MGMT_DESCRIPTOR_TABLE_ADDRESS
};
#endif


static int rdpa_dhd_helper_doorbell_connect(int (*isr)(int irq, void *priv), void *priv, int doorbell_irq);
static void rdpa_dhd_helper_doorbell_disconnect(void *priv, int doorbell_irq);


static void dhd_init_cfg_dump(rdpa_dhd_init_cfg_t *init_cfg)
{
    bdmf_trace("Initial configuration\n");
    bdmf_trace("=================================\n");
    bdmf_trace("\trx_post_flow_ring_base_addr : %p\n", init_cfg->rx_post_flow_ring_base_addr);
    bdmf_trace("\ttx_post_flow_ring_base_addr : %p\n", init_cfg->tx_post_flow_ring_base_addr);
    bdmf_trace("\trx_complete_flow_ring_base_addr : %p\n", init_cfg->rx_complete_flow_ring_base_addr);
    bdmf_trace("\ttx_complete_flow_ring_base_addr : %p\n", init_cfg->tx_complete_flow_ring_base_addr);
    bdmf_trace("\n");
    bdmf_trace("\tr2d_wr_arr_base_addr : %p\n", init_cfg->r2d_wr_arr_base_addr);
    bdmf_trace("\td2r_rd_arr_base_addr : %p\n", init_cfg->d2r_rd_arr_base_addr);
    bdmf_trace("\tr2d_rd_arr_base_addr : %p\n", init_cfg->r2d_rd_arr_base_addr);
    bdmf_trace("\td2r_wr_arr_base_addr : %p\n", init_cfg->d2r_wr_arr_base_addr);
    bdmf_trace("\ttx_post_mgmt_arr_base_addr : %p\n", init_cfg->tx_post_mgmt_arr_base_addr);
    bdmf_trace("\ttx_post_mgmt_arr_base_phys_addr : 0x%x\n", init_cfg->tx_post_mgmt_arr_base_phys_addr);
    bdmf_trace("\n");
#if defined(BCM_DSL_RDP)
    bdmf_trace("\tr2d_wr_arr_base_phys_addr : 0x%x\n", init_cfg->r2d_wr_arr_base_phys_addr);
    bdmf_trace("\td2r_rd_arr_base_phys_addr : 0x%x\n", init_cfg->d2r_rd_arr_base_phys_addr);
    bdmf_trace("\tr2d_rd_arr_base_phys_addr : 0x%x\n", init_cfg->r2d_rd_arr_base_phys_addr);
    bdmf_trace("\td2r_wr_arr_base_phys_addr : 0x%x\n", init_cfg->d2r_wr_arr_base_phys_addr);
    bdmf_trace("\n");
#endif
    bdmf_trace("\tDoorbell Post Wakeup register : phy_addr: 0x%x, virt_addr: %p\n", init_cfg->dongle_wakeup_register, init_cfg->dongle_wakeup_register_virt);
    bdmf_trace("\tDoorbell Complete Wakeup register : phy_addr: 0x%x, virt_addr: %p\n", init_cfg->dongle_wakeup_register_2, init_cfg->dongle_wakeup_register_2_virt);
    bdmf_trace("\tDoorbell ISR : %p\n", init_cfg->doorbell_isr);
    bdmf_trace("\tDoorbell CTX : %p\n", init_cfg->doorbell_ctx);    
    bdmf_trace("\tflow_ring_format : %d\n", init_cfg->flow_ring_format);
    bdmf_trace("\tidma_active : %d\n", init_cfg->dongle_wakeup_hwa);
}

/** This optional callback is called at object init time
 * after initial attributes are set.
 * Its work is:
 * - make sure that all necessary attributes are set and make sense
 * - allocate dynamic resources if any
 * - assign object name if not done in pre_init
 * - finalise object creation
 * If function returns error code !=0, object creation is aborted
 */
int dhd_helper_post_init_ex(struct bdmf_object *mo)
{
    dhd_helper_drv_priv_t *dhd_helper = (dhd_helper_drv_priv_t *)bdmf_obj_data(mo);
    int rc;
#ifdef __OREN__
    uint32_t common_threshold;
    uint32_t high_priority_threshold;
#endif


#ifdef BDMF_SYSTEM_SIM
    dhd_init_cfg_dump(&sim_init_cfg);
    rc = rdd_dhd_hlp_cfg(dhd_helper->radio_idx, &sim_init_cfg, RDPA_DHD_DOORBELL_IRQ + dhd_helper->radio_idx);
#else
    dhd_init_cfg_dump(&dhd_helper->init_cfg);
    rc = rdd_dhd_hlp_cfg(dhd_helper->radio_idx, &dhd_helper->init_cfg, RDPA_DHD_DOORBELL_IRQ + dhd_helper->radio_idx);
#endif
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Failed to initialize Flow Rings, rc %d\n", rc);

    /*Note that IRQs are not connected at this point. IRQs need to be connected explicitly through
     *the int_connect attribute*/
#ifdef __OREN__
    rdd_common_bpm_threshold_get(&common_threshold, &high_priority_threshold);
    rdd_common_bpm_threshold_config(common_threshold - DHD_RX_RESERVED_BUFFERS, high_priority_threshold - DHD_RX_RESERVED_BUFFERS);
#endif

    return rc;
}

/**  dhd helper destroy callback
 *
 */
void dhd_helper_destroy_ex(struct bdmf_object *mo)
{
    dhd_helper_drv_priv_t *dhd_helper = (dhd_helper_drv_priv_t *)bdmf_obj_data(mo);

#ifdef __OREN__
    uint32_t common_threshold;
    uint32_t high_priority_threshold;
#endif

    rdpa_dhd_helper_doorbell_disconnect(dhd_helper->init_cfg.doorbell_ctx,  RDPA_DHD_DOORBELL_IRQ + dhd_helper->radio_idx);

#ifdef __OREN__
    rdd_common_bpm_threshold_get(&common_threshold, &high_priority_threshold);
    rdd_common_bpm_threshold_config(common_threshold + DHD_RX_RESERVED_BUFFERS, high_priority_threshold + DHD_RX_RESERVED_BUFFERS);
#endif
}


extern bdmf_boolean llcsnap_enable[RDPA_MAX_RADIOS];
#define DOT11_LLC_SNAP_HDR_LEN 8

int rdpa_dhd_helper_send_packet_to_dongle_ex(void *data, uint32_t length, const rdpa_dhd_tx_post_info_t *info)
{
    rdd_cpu_tx_args_t cpu_tx_args = {};
    void *p;

#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
#if defined(BCM_DSL_RDP)
    cpu_tx_args.is_spdsvc_setup_packet = info->is_spdsvc_setup_packet;
#endif
#endif

    cpu_tx_args.buffer_type = rdd_host_buffer;
    cpu_tx_args.traffic_dir = rdpa_dir_ds;
    cpu_tx_args.wifi_ssid = info->ssid_if_idx;
    cpu_tx_args.wan_flow = info->flow_ring_id | (info->radio_idx << 14);
#if defined(WL4908)
    cpu_tx_args.direction.ds.emac_id = RDD_EMAC_ID_WIFI;
#else
    cpu_tx_args.direction.ds.emac_id = BL_LILAC_RDD_EMAC_ID_PCI;
#endif
    cpu_tx_args.mode = rdd_cpu_tx_mode_egress_enq;
#ifndef BDMF_SYSTEM_SIM
    if (IS_SKBUFF_PTR(data))
        cpu_tx_args.direction.ds.queue_id = ((((struct sk_buff *)data)->mark) >> 16) & 0x7;
    else
        cpu_tx_args.direction.ds.queue_id = ((struct fkbuff *)((uintptr_t)data & (uintptr_t)NBUFF_PTR_MASK))->wl.ucast.dhd.wl_prio;
#endif

    rdd_dhd_helper_packet_dump(data, length, info);
    p = bdmf_sysb_data(data);

    if (likely(llcsnap_enable[info->radio_idx]))
        bdmf_dcache_flush((unsigned long)p - DOT11_LLC_SNAP_HDR_LEN, length + DOT11_LLC_SNAP_HDR_LEN);
    else
        bdmf_dcache_flush((unsigned long)p, length);

    return rdd_cpu_tx(&cpu_tx_args, data, length);  
}

static void rdpa_dhd_helper_doorbell_disconnect(void *priv, int doorbell_irq)
{
#ifndef BDMF_SYSTEM_SIM
    free_irq(doorbell_irq, priv);
#endif
}

/* "flush" attribute "write" callback */
int dhd_helper_attr_flush_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    dhd_helper_drv_priv_t *dhd_helper = (dhd_helper_drv_priv_t *)bdmf_obj_data(mo);
    uint32_t read_idx_flow_ring_id = *(uint32_t *)val;

    rdd_dhd_helper_flow_ring_flush(dhd_helper->radio_idx, read_idx_flow_ring_id);
    return 0;
}


/* "flow_ring_enable" attribute "write" callback */
int dhd_helper_attr_flow_ring_enable_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    dhd_helper_drv_priv_t *dhd_helper = (dhd_helper_drv_priv_t *)bdmf_obj_data(mo);
    uint32_t flow_ring_id = (uint32_t)index;

    rdd_dhd_helper_flow_ring_disable(dhd_helper->radio_idx, flow_ring_id);
    return 0;
}



/* "tx_complete_send2host" attribute "write" callback */
int dhd_helper_tx_complete_host_send2dhd_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{          
    tx_complete_host_send2dhd_flag = *(bdmf_boolean *)val;      
    
    return 0;
}

/* "tx_complete_send2host" attribute "read" callback */
int dhd_helper_tx_complete_host_send2dhd_read_ex(struct bdmf_object *mo,
     struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    *(bdmf_boolean *)val = tx_complete_host_send2dhd_flag;
    return 0;
}

/* "dhd_stat read attribute "read" callback */
int dhd_helper_dhd_stat_read_ex(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    /*not implemented in RDP*/
    return BDMF_ERR_NOT_SUPPORTED;
}

/* "dhd_stat write attribute "write" callback */
int dhd_helper_dhd_stat_write_ex(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    /*not implemented in RDP*/
    return BDMF_ERR_NOT_SUPPORTED;
}

/* "int_connect" attribute "write" callback */
int dhd_helper_int_connect_write_ex(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    dhd_helper_drv_priv_t *dhd_helper = (dhd_helper_drv_priv_t *)bdmf_obj_data(mo);

    return rdpa_dhd_helper_doorbell_connect(dhd_helper->init_cfg.doorbell_isr,
        dhd_helper->init_cfg.doorbell_ctx, RDPA_DHD_DOORBELL_IRQ + dhd_helper->radio_idx);
}



/* XXX: No need for this as external API any more, need to remove the callbacks */
static int rdpa_dhd_helper_doorbell_connect(int (*isr)(int irq, void *priv), void *priv, int doorbell_irq)
{
#ifndef BDMF_SYSTEM_SIM
    char devname[16];
    RUNNER_REGS_CFG_INT_MASK  runner_interrupt_mask_register;
    int rc;

    RUNNER_REGS_0_CFG_INT_MASK_READ(runner_interrupt_mask_register);

    switch (doorbell_irq - RDPA_DHD_DOORBELL_IRQ)
    {
    case 0:
        runner_interrupt_mask_register.int2_mask = 1;
        break;

    case 1:
        runner_interrupt_mask_register.int3_mask = 1;
        break;

    case 2:
        runner_interrupt_mask_register.int4_mask = 1;
        break;

    default:
        break;
    }
    
    RUNNER_REGS_0_CFG_INT_MASK_WRITE(runner_interrupt_mask_register);
    RUNNER_REGS_1_CFG_INT_MASK_WRITE(runner_interrupt_mask_register);

    sprintf(devname, "brcm_dhd_%d", doorbell_irq);
    rc = BcmHalMapInterruptEx((FN_HANDLER)isr, (void *)priv, doorbell_irq, devname, INTR_REARM_YES,
        INTR_AFFINITY_DEFAULT);
    if (rc)
        return rc;
#if !defined(BCM_DSL_RDP)
    BcmHalInterruptEnable(doorbell_irq);
#endif
#endif
    return 0;
}

void rdpa_dhd_helper_doorbell_interrupt_clear_ex(uint32_t radio_idx)
{
    rdd_interrupt_clear(DHD_DOORBELL_IRQ_NUM + radio_idx, 0);
}

void dhd_helper_cpu_exception_rxq_set_ex(dhd_helper_drv_priv_t *dhd_helper, uint8_t rdd_cpu_rqx)
{
    /* Not supported */
}


/* creates DHD Tx complete Ring with given size */
int rdpa_dhd_helper_dhd_complete_ring_create_ex(uint32_t ring_idx, uint32_t ring_size)
{
    return rdd_dhd_helper_dhd_complete_ring_create(ring_idx, ring_size);
}

/* destroys DHD Tx complete Ring with given size */
int rdpa_dhd_helper_dhd_complete_ring_destroy_ex(uint32_t ring_idx, uint32_t ring_size)
{
    return rdd_dhd_helper_dhd_complete_ring_destroy(ring_idx, ring_size);
}

/* gets next entry from DHD Tx complete Ring  */
int rdpa_dhd_helper_dhd_complete_message_get_ex(rdpa_dhd_complete_data_t *dhd_complete_info)
{
    return rdd_dhd_helper_dhd_complete_message_get(dhd_complete_info);
}


/* "rx_post_init" attribute "write" callback */
int dhd_helper_attr_rx_post_init_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{    
    dhd_helper_drv_priv_t *dhd_helper = (dhd_helper_drv_priv_t *)bdmf_obj_data(mo);

    return rdd_dhd_rx_post_init(dhd_helper->radio_idx, &dhd_helper->init_cfg, DHD_RX_POST_FLOW_RING_SIZE-1) ? BDMF_ERR_NOMEM : 0;
}


/* "rx_post_uninit" attribute "write" callback */
int dhd_helper_attr_rx_post_uninit_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    dhd_helper_drv_priv_t *dhd_helper = (dhd_helper_drv_priv_t *)bdmf_obj_data(mo);
    uint32_t num_items;

    return rdd_dhd_rx_post_uninit(dhd_helper->radio_idx, &dhd_helper->init_cfg, &num_items) ? BDMF_ERR_NOMEM : 0;
}

/* "rx_post_reinit" attribute "write" callback */
int dhd_helper_attr_rx_post_reinit_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    dhd_helper_drv_priv_t *dhd_helper = (dhd_helper_drv_priv_t *)bdmf_obj_data(mo);
    
    return rdd_dhd_rx_post_reinit(dhd_helper->radio_idx, &dhd_helper->init_cfg) ? BDMF_ERR_NOMEM : 0;
}

/* "ssid_tx_dropped_packets" attribute "read" callback */
int dhd_helper_ssid_tx_dropped_packets_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, void *val, uint32_t size)
{    
    dhd_helper_drv_priv_t *dhd_helper = (dhd_helper_drv_priv_t *)bdmf_obj_data(mo);
    uint32_t *drops = (uint32_t *)val;
    uint16_t rdd_drops = rdd_dhd_helper_ssid_tx_dropped_packets_get(dhd_helper->radio_idx, index);
    *drops = rdd_drops;

    return 0;
}


void rdpa_dhd_helper_wakeup_information_get_ex(rdpa_dhd_wakeup_info_t *wakeup_info)
{
    rdd_dhd_helper_wakeup_information_get(wakeup_info);
}

