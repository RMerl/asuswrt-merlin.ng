/*
* <:copyright-BRCM:2013-2015:proprietary:standard
* 
*    Copyright (c) 2013-2015 Broadcom 
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
 :>
*/

/*
 * rdpa_dhd_helper.c
 *
 *  DHD Helper driver
 */

#ifdef CONFIG_DHD_RUNNER

#include "bdmf_dev.h"
#include "rdpa_api.h"
#include "rdpa_int.h"
#include "rdpa_common.h"
#include "rdd.h"
#include "rdd_dhd_helper.h"
#include "rdpa_platform.h"
#include "rdpa_dhd_helper_basic.h"
#include "rdpa_dhd_helper_ex.h"
#include "rdpa_rdd_inline.h"
#include "rdpa_cpu_ex.h"

bdmf_boolean tx_complete_host_send2dhd_flag = 0;

/* DHD Helper object */
bdmf_object_handle dhd_helper_obj[RDPA_MAX_RADIOS];

bdmf_boolean llcsnap_enable[RDPA_MAX_RADIOS];
void *dongle_wakeup_register[2];

/** This optional callback is called called at object init time
 *  before initial attributes are set.
 *  If function returns error code !=0, object creation is aborted
 */
static int dhd_helper_pre_init(struct bdmf_object *mo)
{
    dhd_helper_drv_priv_t *dhd_helper = (dhd_helper_drv_priv_t *)bdmf_obj_data(mo);

    memset(dhd_helper, 0, sizeof(dhd_helper_drv_priv_t));
    dhd_helper->cpu_data.cpu_port = rdpa_cpu_none;
    dhd_helper->cpu_data.exception_rxq = 0xff;

    return 0;
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
static int dhd_helper_post_init(struct bdmf_object *mo)
{
    int rc;    
    dhd_helper_drv_priv_t *dhd_helper = (dhd_helper_drv_priv_t *)bdmf_obj_data(mo);    

    if (dhd_helper->radio_idx < 0 || dhd_helper->radio_idx >= RDPA_MAX_RADIOS)
        BDMF_TRACE_RET(BDMF_ERR_RANGE, "Radio index %ld is out of range\n", (long)dhd_helper->radio_idx);

    dhd_helper_obj[dhd_helper->radio_idx] = mo;
    llcsnap_enable[dhd_helper->radio_idx] = dhd_helper->init_cfg.add_llcsnap_header;
    flow_ring_format[dhd_helper->radio_idx] = dhd_helper->init_cfg.flow_ring_format;
    dongle_wakeup_register[0] = dhd_helper->init_cfg.dongle_wakeup_register_virt;
    dongle_wakeup_register[1] = dhd_helper->init_cfg.dongle_wakeup_register_2_virt;

    rc = dhd_helper_post_init_ex(mo);    
    if (rc)
      return rc;

    /* set object name */
    snprintf(mo->name, sizeof(mo->name), "dhd_helper/radio_idx=%d", (int)dhd_helper->radio_idx);
        
    return 0;
}

static void dhd_helper_destroy(struct bdmf_object *mo)
{
    dhd_helper_drv_priv_t *dhd_helper = (dhd_helper_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_dhd_init_cfg_t cfg = {};
    
    dhd_helper_destroy_ex(mo);
    
    rdd_dhd_hlp_cfg(dhd_helper->radio_idx, &cfg, 0);
    dhd_helper_obj[dhd_helper->radio_idx] = NULL;
}


/** Send packet to dongle 
 *
 * \param[in]   data        Packet data
 * \param[in]   length      Packet length
 * \param[in]   is_bpm      Additional TX post info
 * \return 0=OK or int error code\n
 */
int rdpa_dhd_helper_send_packet_to_dongle(void *data, uint32_t length, const rdpa_dhd_tx_post_info_t *info)
{
    return rdpa_dhd_helper_send_packet_to_dongle_ex(data, length, info);
}

#ifndef BDMF_DRIVER_GPL_LAYER
EXPORT_SYMBOL(rdpa_dhd_helper_send_packet_to_dongle);
#endif

/* "flush" attribute "write" callback */
static int dhd_helper_attr_flush_write(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    return dhd_helper_attr_flush_write_ex(mo, ad, index, val, size);
}

/* "flow_ring_enable" attribute "read" callback */
static int dhd_helper_attr_flow_ring_enable_read(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, void *val, uint32_t size)
{
    dhd_helper_drv_priv_t *dhd_helper = (dhd_helper_drv_priv_t *)bdmf_obj_data(mo);
    uint32_t flow_ring_id = (uint32_t)index;
    bdmf_boolean *enabled = (bdmf_boolean *)val;
    rdpa_dhd_flring_cache_t *entry;

    entry = (rdpa_dhd_flring_cache_t *)dhd_helper->init_cfg.tx_post_mgmt_arr_base_addr + flow_ring_id;
    *enabled = entry->flags & FLOW_RING_FLAG_DISABLED ? 0 : 1;
    return 0;
}

/* "flow_ring_enable" attribute "write" callback */
static int dhd_helper_attr_flow_ring_enable_write(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    uint32_t flow_ring_id = (uint32_t)index;
    bdmf_boolean enable = *(bdmf_boolean *)val;
    
    if (flow_ring_id < 2)
        return BDMF_ERR_PARM;

    if (enable)
        return 0; /* Nothing to do */
    
    return dhd_helper_attr_flow_ring_enable_write_ex(mo, ad, index, val, size);
}

/* "rx_post_init" attribute "write" callback */
static int dhd_helper_attr_rx_post_init_write(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    return dhd_helper_attr_rx_post_init_write_ex(mo, ad, index, val, size);
}

/* "rx_post_uninit" attribute "write" callback */
static int dhd_helper_attr_rx_post_uninit_write(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    return dhd_helper_attr_rx_post_uninit_write_ex(mo, ad, index, val, size);
}

/* "rx_post_reinit" attribute "write" callback */
static int dhd_helper_attr_rx_post_reinit_write(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    return dhd_helper_attr_rx_post_reinit_write_ex(mo, ad, index, val, size);
}

/* "tx_complete_send2host" attribute "write" callback */
static int dhd_helper_tx_complete_host_send2dhd_write(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{          
    return dhd_helper_tx_complete_host_send2dhd_write_ex(mo, ad, index, val, size);
}

/* "tx_complete_send2host" attribute "read" callback */
static int dhd_helper_tx_complete_host_send2dhd_read(struct bdmf_object *mo,
     struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    return dhd_helper_tx_complete_host_send2dhd_read_ex(mo, ad, index, val, size);
}

/* "dhd_helper_dhd_stat_read" attribute "read" callback */
static int dhd_helper_dhd_stat_read(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    return dhd_helper_dhd_stat_read_ex(mo, ad, index, val, size);
}

/* "ssid_tx_dropped_packets" attribute "read" callback */
static int dhd_helper_ssid_tx_dropped_packets_read(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, void *val, uint32_t size)
{
    dhd_helper_ssid_tx_dropped_packets_read_ex(mo, ad, index, val, size);
    
    return 0;
}

/* "int_connect" attribute "write" callback */
static int dhd_helper_int_connect_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    return dhd_helper_int_connect_write_ex(mo, ad, index, val, size);
}

/* "aggregation_timeout" attribute "read" callback */
static int dhd_helper_aggregation_timeout_read(struct bdmf_object *mo,
     struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{   
    dhd_helper_drv_priv_t *dhd_helper = (dhd_helper_drv_priv_t *)bdmf_obj_data(mo);
    uint8_t *aggregation_timeout = (uint8_t *)val;

    return rdd_dhd_helper_aggregation_timeout_get(dhd_helper->radio_idx, (int)index, aggregation_timeout);
}

/* "aggregation_timeout" attribute "write" callback */
static int dhd_helper_aggregation_timeout_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{   
    dhd_helper_drv_priv_t *dhd_helper = (dhd_helper_drv_priv_t *)bdmf_obj_data(mo);
    uint8_t aggregation_timeout = *(uint8_t *)val;

    return rdd_dhd_helper_aggregation_timeout_set(dhd_helper->radio_idx, (int)index, aggregation_timeout);
}

/* "aggregation_size" attribute "read" callback */
static int dhd_helper_aggregation_size_read(struct bdmf_object *mo,
     struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{   
    dhd_helper_drv_priv_t *dhd_helper = (dhd_helper_drv_priv_t *)bdmf_obj_data(mo);
    uint8_t *aggregation_size = (uint8_t *)val;

    return rdd_dhd_helper_aggregation_size_get(dhd_helper->radio_idx, (int)index, aggregation_size);
}

/* "aggregation_size" attribute "write" callback */
static int dhd_helper_aggregation_size_write(struct bdmf_object *mo,
     struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{   
    dhd_helper_drv_priv_t *dhd_helper = (dhd_helper_drv_priv_t *)bdmf_obj_data(mo);
    uint8_t aggregation_size = *(uint8_t *)val;

    return rdd_dhd_helper_aggregation_size_set(dhd_helper->radio_idx, (int)index, aggregation_size);
}

/* "aggregation_bypass_cpu_tx" attribute "read" callback */
static int dhd_helper_aggregation_bypass_cpu_tx_read(struct bdmf_object *mo,
     struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{   
    dhd_helper_drv_priv_t *dhd_helper = (dhd_helper_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_boolean *enable = (bdmf_boolean *)val;

    return rdd_dhd_helper_aggregation_bypass_cpu_tx_get(dhd_helper->radio_idx, enable);
}

/* "aggregation_bypass_cpu_tx" attribute "write" callback */
static int dhd_helper_aggregation_bypass_cpu_tx_write(struct bdmf_object *mo,
     struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{   
    dhd_helper_drv_priv_t *dhd_helper = (dhd_helper_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_boolean enable = *(bdmf_boolean *)val;

    return rdd_dhd_helper_aggregation_bypass_cpu_tx_set(dhd_helper->radio_idx, enable);
}

/* "aggregation_bypass_non_udp_tcp" attribute "read" callback */
static int dhd_helper_aggregation_bypass_non_udp_tcp_read(struct bdmf_object *mo,
     struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{   
    dhd_helper_drv_priv_t *dhd_helper = (dhd_helper_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_boolean *enable = (bdmf_boolean *)val;

    return rdd_dhd_helper_aggregation_bypass_non_udp_tcp_get(dhd_helper->radio_idx, enable);
}

/* "aggregation_bypass_non_udp_tcp" attribute "write" callback */
static int dhd_helper_aggregation_bypass_non_udp_tcp_write(struct bdmf_object *mo,
     struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{   
    dhd_helper_drv_priv_t *dhd_helper = (dhd_helper_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_boolean enable = *(bdmf_boolean *)val;

    return rdd_dhd_helper_aggregation_bypass_non_udp_tcp_set(dhd_helper->radio_idx, enable);
}

/* "aggregation_bypass_tcp_pktlen" attribute "read" callback */
static int dhd_helper_aggregation_bypass_tcp_pktlen_read(struct bdmf_object *mo,
     struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{   
    dhd_helper_drv_priv_t *dhd_helper = (dhd_helper_drv_priv_t *)bdmf_obj_data(mo);
    uint8_t *pkt_len = (uint8_t *)val;

    return rdd_dhd_helper_aggregation_bypass_tcp_pktlen_get(dhd_helper->radio_idx, pkt_len);
}

/* "aggregation_bypass_tcp_pktlen" attribute "write" callback */
static int dhd_helper_aggregation_bypass_tcp_pktlen_write(struct bdmf_object *mo,
     struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{   
    dhd_helper_drv_priv_t *dhd_helper = (dhd_helper_drv_priv_t *)bdmf_obj_data(mo);
    uint8_t pkt_len = *(uint8_t *)val;

    return rdd_dhd_helper_aggregation_bypass_tcp_pktlen_set(dhd_helper->radio_idx, pkt_len);
}

/* "aggregation_timer" attribute "read" callback */
static int dhd_helper_aggregation_timer_read(struct bdmf_object *mo,
     struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{   
    dhd_helper_drv_priv_t *dhd_helper = (dhd_helper_drv_priv_t *)bdmf_obj_data(mo);
    uint8_t *aggregation_timer = (uint8_t *)val;

    return rdd_dhd_helper_aggregation_timeout_get(dhd_helper->radio_idx, 0, aggregation_timer);
}

/* "aggregation_timer" attribute "write" callback */
static int dhd_helper_aggregation_timer_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{   
    dhd_helper_drv_priv_t *dhd_helper = (dhd_helper_drv_priv_t *)bdmf_obj_data(mo);
    uint8_t aggregation_timer = *(uint8_t *)val;
    int ret = 0, i;

    for (i = 0; i < RDPA_MAX_AC; i++)
        ret |= rdd_dhd_helper_aggregation_timeout_set(dhd_helper->radio_idx, i, aggregation_timer);
    return ret;
}

const bdmf_attr_enum_table_t rdpa_fr_format_enum_table =
{
    .type_name = "rdpa_fr_format", .help = "flow Ring Format",
    .values = {
        {"legacy", FR_FORMAT_WI_WI64},        /**< Legacy format */
        {"cwi32", FR_FORMAT_WI_CWI32},        /**< CWI32 format */
        {NULL, 0}
    }
};

/* DHD initial configuration */
struct bdmf_aggr_type dhd_init_config_type =
{
    .name = "dhd_init_config", .struct_name = "rdpa_dhd_init_cfg_t",
    .help = "Initial DHD Configuration, includes base addresses of FlowRings and RD/WR Indexes arrays in DDR",
    .fields = (struct bdmf_attr[]) {
        { .name = "rx_post_flow_ring_base_addr", .help = "Base address of RX Post FlowRing", 
            .size = sizeof(void *), .type = bdmf_attr_pointer, 
            .offset = offsetof(rdpa_dhd_init_cfg_t, rx_post_flow_ring_base_addr)
        },
        { .name = "tx_post_flow_ring_base_addr",
            .help = "Base Address of first (Fake) TX Post FlowRing (all rings are from same size)",
            .size = sizeof(void *), .type = bdmf_attr_pointer,
            .offset = offsetof(rdpa_dhd_init_cfg_t, tx_post_flow_ring_base_addr)
        },
        { .name = "rx_complete_flow_ring_base_addr", .help = "Base address of RX Complete FlowRing",
            .size = sizeof(void *), .type = bdmf_attr_pointer,
            .offset = offsetof(rdpa_dhd_init_cfg_t, rx_complete_flow_ring_base_addr)
        },
        { .name = "tx_complete_flow_ring_base_addr", .help = "Base address of TX Complete FlowRing",
            .size = sizeof(void *), .type = bdmf_attr_pointer,
            .offset = offsetof(rdpa_dhd_init_cfg_t, tx_complete_flow_ring_base_addr)
        },
        { .name = "r2d_wr_arr_base_addr", .help = "Base address of Runner-to-Dongle Write indexes array",
            .size = sizeof(void *), .type = bdmf_attr_pointer,
            .offset = offsetof(rdpa_dhd_init_cfg_t, r2d_wr_arr_base_addr)
        },
        { .name = "d2r_rd_arr_base_addr", .help = "Base address of Dongle-to-Runner Read indexes array",
            .size = sizeof(void *), .type = bdmf_attr_pointer,
            .offset = offsetof(rdpa_dhd_init_cfg_t, d2r_rd_arr_base_addr)
        },
        { .name = "r2d_rd_arr_base_addr", .help = "Base address of Dongle-to-Runner Read indexes array",
            .size = sizeof(void *), .type = bdmf_attr_pointer,
            .offset = offsetof(rdpa_dhd_init_cfg_t, r2d_rd_arr_base_addr)
        },
        { .name = "d2r_wr_arr_base_addr", .help = "Base address of Dongle-to-Runner Write indexes array",
            .size = sizeof(void *), .type = bdmf_attr_pointer,
            .offset = offsetof(rdpa_dhd_init_cfg_t, d2r_wr_arr_base_addr)
        },
        { .name = "tx_post_mgmt_arr_base_addr", .help = "Base address of TX-Post FlowRings management array",
            .size = sizeof(void *), .type = bdmf_attr_pointer,
            .offset = offsetof(rdpa_dhd_init_cfg_t, tx_post_mgmt_arr_base_addr)
        },
        { .name = "doorbell_isr", .help = "Doorbell ISR",
            .size = sizeof(void *), .type = bdmf_attr_pointer,
            .offset = offsetof(rdpa_dhd_init_cfg_t, doorbell_isr)
        },
        { .name = "doorbell_ctx", .help = "Doorbell ISR private context",
            .size = sizeof(void *), .type = bdmf_attr_pointer,
            .offset = offsetof(rdpa_dhd_init_cfg_t, doorbell_ctx)
        },
        { .name = "dongle_wakeup_register", .help = "Runner-to-Dongle Wakeup Register phy address", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_dhd_init_cfg_t, dongle_wakeup_register)
        },
        { .name = "add_llcsnap_header", .help = "Add LLCSNAP header to packet", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_dhd_init_cfg_t, add_llcsnap_header)
        },
        { .name = "flow_ring_format", .help = "Flow Ring format", .size = sizeof(uint8_t),
            .type = bdmf_attr_enum, .offset = offsetof(rdpa_dhd_init_cfg_t, flow_ring_format),
            .ts.enum_table = &rdpa_fr_format_enum_table
        },
        { .name = "dongle_wakeup_register_2", .help = "Runner-to-Dongle Wakeup Register 2 phy address", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_dhd_init_cfg_t, dongle_wakeup_register_2)
        },        
        { .name = "dongle_wakeup_hwa", .help = "Doorbell iDMA is active", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_dhd_init_cfg_t, dongle_wakeup_hwa)
        },
        { .name = "dongle_wakeup_register_virt", .help = "Runner-to-Dongle Wakeup Register virt address", .size = sizeof(void *),
            .type = bdmf_attr_pointer, .offset = offsetof(rdpa_dhd_init_cfg_t, dongle_wakeup_register_virt)
        },
        { .name = "dongle_wakeup_register_2_virt", .help = "Runner-to-Dongle Wakeup Register 2 virt address", .size = sizeof(void *),
            .type = bdmf_attr_pointer, .offset = offsetof(rdpa_dhd_init_cfg_t, dongle_wakeup_register_2_virt)
        },
        BDMF_ATTR_LAST
    },
};
DECLARE_BDMF_AGGREGATE_TYPE(dhd_init_config_type);


/* DHD CPU Data configuration */
struct bdmf_aggr_type dhd_cpu_data_type =
{
    .name = "dhd_cpu_data", .struct_name = "rdpa_dhd_cpu_data_t",
    .help = "Initial DHD Configuration, includes base addresses of FlowRings and RD/WR Indexes arrays in DDR",
    .fields = (struct bdmf_attr[]) {
        { .name = "cpu_port", .help = "CPU Port object index", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_dhd_cpu_data_t, cpu_port)
        },
        { .name = "exception_rxq", .help = "Exception RX queue index", .type = bdmf_attr_number,
            .size = sizeof(uint8_t),
            .offset = offsetof(rdpa_dhd_cpu_data_t, exception_rxq),
        },
        BDMF_ATTR_LAST
    },
};
DECLARE_BDMF_AGGREGATE_TYPE(dhd_cpu_data_type);

/* "cpu_data" attribute "write" callback */
static int dhd_helper_cpu_data_write(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    dhd_helper_drv_priv_t *dhd_helper = (dhd_helper_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_dhd_cpu_data_t *cpu_data = (rdpa_dhd_cpu_data_t *)val; 
    uint8_t rdd_cpu_rxq;

    if (!cpu_data)
    {
        dhd_helper->cpu_data.cpu_port = rdpa_cpu_none;
        dhd_helper->cpu_data.exception_rxq = (uint8_t)BDMF_INDEX_UNASSIGNED;
        rdd_cpu_rxq = (uint8_t)BDMF_INDEX_UNASSIGNED;
    }
    else
    {
        bdmf_object_handle cpu_obj;
        int rc;

        dhd_helper->cpu_data.cpu_port = cpu_data->cpu_port;
        dhd_helper->cpu_data.exception_rxq = cpu_data->exception_rxq;
        rc = rdpa_cpu_get(cpu_data->cpu_port, &cpu_obj);
        if (rc)
            return rc;

        rdd_cpu_rxq = cpu_rdd_rxq_idx_get((cpu_drv_priv_t *)bdmf_obj_data(cpu_obj), cpu_data->exception_rxq); 
        bdmf_put(cpu_obj);
    }

    dhd_helper_cpu_exception_rxq_set_ex(dhd_helper, rdd_cpu_rxq);
    return 0;
}

struct bdmf_aggr_type dhd_stat_data_type =
{
    .name = "dhd_stat_data", .struct_name = "rdpa_dhd_data_stat_t",
    .help = "DHD statistics per radio", .extra_flags = BDMF_ATTR_UNSIGNED,
    .fields = (struct bdmf_attr[])
    {
        { .name = "dhd_rx_drop", .help = "DHD RX packets drop", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_dhd_data_stat_t, dhd_rx_drop)
        },
        { .name = "dhd_tx_fpm_used", .help = "DHD TX FPM used", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_dhd_data_stat_t, dhd_tx_fpm_used)
        },
        { .name = "dhd_tx_fpm_drop", .help = "DHD TX FPM drop", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_dhd_data_stat_t, dhd_tx_fpm_drop)
        },
        { .name = "dhd_tx_high_prio_fpm_drop", .help = "DHD TX high priority and mcast FPM drop", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_dhd_data_stat_t, dhd_tx_high_prio_fpm_drop)
        },
        { .name = "dhd_mcast_sbpm_drop", .help = "DHD MCAST SBPM drop", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_dhd_data_stat_t, dhd_mcast_sbpm_drop)
        },
        { .name = "dhd_tx_fr_ac_bk_full", .help = "DHD TX feeder ring AC BK full", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_dhd_data_stat_t, dhd_tx_fr_ac_bk_full)
        },
        { .name = "dhd_tx_fr_ac_be_full", .help = "DHD TX feeder ring AC BE full", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_dhd_data_stat_t, dhd_tx_fr_ac_be_full)
        },
        { .name = "dhd_tx_fr_ac_vi_full", .help = "DHD TX feeder ring AC VI full", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_dhd_data_stat_t, dhd_tx_fr_ac_vi_full)
        },
        { .name = "dhd_tx_fr_ac_vo_full", .help = "DHD TX feeder ring AC VO full", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_dhd_data_stat_t, dhd_tx_fr_ac_vo_full)
        },
        { .name = "dhd_tx_fr_ac_bc_mc_full", .help = "DHD TX feeder ring AC BC/MC full", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_dhd_data_stat_t, dhd_tx_fr_ac_bc_mc_full)
        },
        { .name = "dhd_tx_post_packets", .help = "DHD TX post packets",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_dhd_data_stat_t, dhd_tx_post_packets)
        },
        { .name = "dhd_tx_complete_packets", .help = "DHD TX completed packets ",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_dhd_data_stat_t, dhd_tx_complete_packets)
        },
        { .name = "dhd_rx_complete_packets", .help = "DHD RX completed packets",
            .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_dhd_data_stat_t, dhd_rx_complete_packets)
        },
        BDMF_ATTR_LAST
    },
};
DECLARE_BDMF_AGGREGATE_TYPE(dhd_stat_data_type);

/* Object attribute descriptors */
static struct bdmf_attr dhd_helper_attrs[] =
{
    { .name = "radio_idx", .help = "Radio Index", .size = sizeof(bdmf_index),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG | BDMF_ATTR_KEY,
        .type = bdmf_attr_number, .offset = offsetof(dhd_helper_drv_priv_t, radio_idx),
        .max_val = RDPA_MAX_RADIOS - 1, .min_val = 0
    },
    { .name = "init_cfg", .help = "Initial DHD Configuration", .offset = offsetof(dhd_helper_drv_priv_t, init_cfg),
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "dhd_init_config",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG | BDMF_ATTR_MANDATORY,
    },
    { .name = "flush", .help = "Flush FlowRing", .size = sizeof(uint32_t),
        .flags = BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG | BDMF_ATTR_NOLOCK,
        .min_val = (DHD_RX_COMPLETE_RING_NUMBER + 1), 
        .type = bdmf_attr_number, .write = dhd_helper_attr_flush_write
    },
    { .name = "flow_ring_enable", .help = "Enable/Disable FlowRing", .type = bdmf_attr_boolean,
        .array_size = RDPA_DHD_HELPER_NUM_OF_FLOW_RINGS - 2,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG | BDMF_ATTR_NOLOCK,
        .read = dhd_helper_attr_flow_ring_enable_read, .write = dhd_helper_attr_flow_ring_enable_write
    },
    { .name = "rx_post_init", .help = "RX Post init: allocate and push RX Post descriptors to Dongle",
        .size = sizeof(bdmf_boolean), .flags = BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG | BDMF_ATTR_NO_VALUE,
        .type = bdmf_attr_boolean, .write = dhd_helper_attr_rx_post_init_write
    },
    { .name = "ssid_tx_dropped_packets", .help = "SSID Dropped Packets",
        .type = bdmf_attr_number, .size = sizeof(uint32_t), .array_size = 16,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_NOLOCK | BDMF_ATTR_STAT,
        .read = dhd_helper_ssid_tx_dropped_packets_read
    },
    { .name = "aggregation_size", .help = "Number of packets to be aggregated on host side per AC",
        .type = bdmf_attr_number, .size = sizeof(uint8_t), .array_size = RDPA_MAX_AC,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .read = dhd_helper_aggregation_size_read, .write = dhd_helper_aggregation_size_write
    },
    { .name = "aggregation_timeout", .help = "Aggregation expiration timeout on host side per AC",
        .type = bdmf_attr_number, .size = sizeof(uint8_t), .array_size = RDPA_MAX_AC,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .read = dhd_helper_aggregation_timeout_read, .write = dhd_helper_aggregation_timeout_write
    },
    { .name = "aggregation_bypass_cpu_tx", .help = "Bypass aggregation for CPU TX packet",
        .type = bdmf_attr_boolean, .size = sizeof(bdmf_boolean),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .read = dhd_helper_aggregation_bypass_cpu_tx_read, .write = dhd_helper_aggregation_bypass_cpu_tx_write
    },
    { .name = "aggregation_bypass_non_udp_tcp", .help = "Bypass aggregation for non UDP/TCP forwarding packet",
        .type = bdmf_attr_boolean, .size = sizeof(bdmf_boolean),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .read = dhd_helper_aggregation_bypass_non_udp_tcp_read, .write = dhd_helper_aggregation_bypass_non_udp_tcp_write
    },
    { .name = "aggregation_bypass_tcp_pktlen", .help = "Bypass aggregation for TCP forwarding less and euqal to this value. Value smaller than 54 means disable",
        .type = bdmf_attr_number, .size = sizeof(uint16_t), .min_val = 0, .max_val = 255,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .read = dhd_helper_aggregation_bypass_tcp_pktlen_read, .write = dhd_helper_aggregation_bypass_tcp_pktlen_write
    },
    { .name = "aggregation_timer", .help = "Global Aggregation expiration timer on host (old implementation, keeping it for backward compatibility)",
        .type = bdmf_attr_number, .size = sizeof(uint8_t),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .read = dhd_helper_aggregation_timer_read, .write = dhd_helper_aggregation_timer_write
    },
    { .name = "int_connect", .help = "Connect interrupts",
        .type = bdmf_attr_boolean, 
        .flags = BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG | BDMF_ATTR_NOLOCK,
        .write = dhd_helper_int_connect_write
    },
    { .name = "rx_post_uninit", .help = "RX Post uninit: free the buffers allocated in RX Post descriptors to Dongle",
        .size = sizeof(bdmf_boolean), .flags = BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG | BDMF_ATTR_NO_VALUE,
        .type = bdmf_attr_boolean, .write = dhd_helper_attr_rx_post_uninit_write,
    },
    { .name = "tx_complete_send2host", .help = "Global flag: Tx Complete HOST_BUFFER type send to DHD (0 - don't send, 1 - send)",
        .type = bdmf_attr_boolean, .size = sizeof(bdmf_boolean),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .read = dhd_helper_tx_complete_host_send2dhd_read, .write = dhd_helper_tx_complete_host_send2dhd_write,       
    },
    { .name = "cpu_data", .help = "DHD CPU Resources data",
        .offset = offsetof(dhd_helper_drv_priv_t, cpu_data),
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "dhd_cpu_data",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .write = dhd_helper_cpu_data_write, 
    },
    { .name = "dhd_stat", .help = "DHD statistics data per radio",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "dhd_stat_data",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_NOLOCK | BDMF_ATTR_STAT | BDMF_ATTR_NO_AUTO_GEN | BDMF_ATTR_WRITE,
        .read = dhd_helper_dhd_stat_read, .write = dhd_helper_dhd_stat_write_ex,
    },
    { .name = "rx_post_reinit", .help = "RX Post reinit: Refill RX Post ring",
        .type = bdmf_attr_boolean, .size = sizeof(bdmf_boolean),
        .flags = BDMF_ATTR_NO_VALUE | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .write = dhd_helper_attr_rx_post_reinit_write,
    },
    BDMF_ATTR_LAST
};

static int dhd_helper_drv_init(struct bdmf_type *drv);
static void dhd_helper_drv_exit(struct bdmf_type *drv);

struct bdmf_type dhd_helper_drv = {
    .name = "dhd_helper",
    .parent = "system",
    .description = "DHD Helper channel bundle",
    .drv_init = dhd_helper_drv_init,
    .drv_exit = dhd_helper_drv_exit,
    .pre_init = dhd_helper_pre_init,
    .post_init = dhd_helper_post_init,
    .destroy = dhd_helper_destroy,
    .extra_size = sizeof(dhd_helper_drv_priv_t),
    .aattr = dhd_helper_attrs,
    .max_objs = RDPA_MAX_RADIOS,
};
DECLARE_BDMF_TYPE(rdpa_dhd_helper, dhd_helper_drv);

extern void (*f_rdpa_dhd_helper_complete_wakeup)(uint32_t radio_idx, bdmf_boolean is_tx_complete);
extern int (*f_rdpa_dhd_helper_send_packet_to_dongle)(void *data, uint32_t length, const rdpa_dhd_tx_post_info_t *info);
extern void (*f_rdpa_dhd_helper_doorbell_interrupt_clear)(uint32_t radio_idx);
extern void (*f_rdpa_dhd_helper_wakeup_information_get)(rdpa_dhd_wakeup_info_t *wakeup_info);
extern int (*f_rdpa_dhd_helper_dhd_complete_ring_create)(uint32_t radio_idx, uint32_t ring_size);
extern int (*f_rdpa_dhd_helper_dhd_complete_ring_destroy)(uint32_t radio_idx, uint32_t ring_size);
extern int (*f_rdpa_dhd_helper_dhd_complete_message_get)(rdpa_dhd_complete_data_t *dhd_complete_info);

void rdpa_dhd_helper_complete_wakeup(uint32_t radio_idx, bdmf_boolean is_tx_complete)
{
    rdd_dhd_helper_wakeup(radio_idx, is_tx_complete);
}

/* get wakeup information to propagate it dongle ( used for dingle to wake up runner directly */
void rdpa_dhd_helper_wakeup_information_get(rdpa_dhd_wakeup_info_t *wakeup_info)
{
    rdpa_dhd_helper_wakeup_information_get_ex(wakeup_info);
}

/* creates DHD Tx complete Ring with given size */
int rdpa_dhd_helper_dhd_complete_ring_create(uint32_t ring_idx, uint32_t ring_size)
{
    return rdpa_dhd_helper_dhd_complete_ring_create_ex(ring_idx, ring_size);
}

/* destroys DHD Tx complete Ring with given size */
int rdpa_dhd_helper_dhd_complete_ring_destroy(uint32_t ring_idx, uint32_t ring_size)
{
    return rdpa_dhd_helper_dhd_complete_ring_destroy_ex(ring_idx, ring_size);
}

/* gets next entry from DHD Tx complete Ring  */
int rdpa_dhd_helper_dhd_complete_message_get(rdpa_dhd_complete_data_t *dhd_complete_info)
{
    return rdpa_dhd_helper_dhd_complete_message_get_ex(dhd_complete_info);
}

void rdpa_dhd_helper_doorbell_interrupt_clear(uint32_t radio_idx)
{
    rdpa_dhd_helper_doorbell_interrupt_clear_ex(radio_idx);
}

/* Init/exit module. Cater for GPL layer */
static int dhd_helper_drv_init(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_dhd_helper_drv = rdpa_dhd_helper_drv;
    f_rdpa_dhd_helper_get = rdpa_dhd_helper_get;
    f_rdpa_dhd_helper_complete_wakeup = rdpa_dhd_helper_complete_wakeup;
    f_rdpa_dhd_helper_send_packet_to_dongle = rdpa_dhd_helper_send_packet_to_dongle;
    f_rdpa_dhd_helper_doorbell_interrupt_clear = rdpa_dhd_helper_doorbell_interrupt_clear;
    f_rdpa_dhd_helper_wakeup_information_get = rdpa_dhd_helper_wakeup_information_get;
    f_rdpa_dhd_helper_dhd_complete_ring_create = rdpa_dhd_helper_dhd_complete_ring_create;
    f_rdpa_dhd_helper_dhd_complete_ring_destroy = rdpa_dhd_helper_dhd_complete_ring_destroy;
    f_rdpa_dhd_helper_dhd_complete_message_get = rdpa_dhd_helper_dhd_complete_message_get;
#endif
    return 0;
}

static void dhd_helper_drv_exit(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_dhd_helper_drv = NULL;
    f_rdpa_dhd_helper_get = NULL;
    f_rdpa_dhd_helper_complete_wakeup = NULL;
    f_rdpa_dhd_helper_send_packet_to_dongle = NULL;
    f_rdpa_dhd_helper_doorbell_interrupt_clear = NULL;
    f_rdpa_dhd_helper_wakeup_information_get = NULL;
    f_rdpa_dhd_helper_dhd_complete_ring_create = NULL;
    f_rdpa_dhd_helper_dhd_complete_ring_destroy = NULL;
    f_rdpa_dhd_helper_dhd_complete_message_get = NULL;
#endif
}

/*
 * RDPA-Internal interface
 */

/***************************************************************************
 * Functions declared in auto-generated header
 **************************************************************************/
/** Get DHD helper object
 * \param[in] radio_idx_    Object key
 * \param[out] dhd_helper_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_dhd_helper_get(bdmf_number radio_idx_, bdmf_object_handle *_obj_)
{
    if (!dhd_helper_obj[radio_idx_] || dhd_helper_obj[radio_idx_]->state == bdmf_state_deleted)
        return BDMF_ERR_NOENT;
    bdmf_get(dhd_helper_obj[radio_idx_]);
    *_obj_ = dhd_helper_obj[radio_idx_];
    return 0;
}
#endif

