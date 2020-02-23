/*
 * <:copyright-BRCM:2019:DUAL/GPL:standard
 * 
 *    Copyright (c) 2019 Broadcom 
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

/*
 * CPU interface
 */
#include <bdmf_interface.h>
#include <rdpa_types.h>
#ifdef CONFIG_DHD_RUNNER
#include <rdpa_dhd_helper_basic.h>
#endif

/* This map is to provide rdd bridge source port to rdpa_if */
#if defined(DSL_63138) || defined(DSL_63148)
static const rdpa_if map_rdd_to_rdpa_if[] __attribute__((aligned(L1_CACHE_BYTES)))= 
   {rdpa_if_wan1, rdpa_if_wan0, rdpa_if_lan0, rdpa_if_lan1,  /* FIXME: MULTI-WAN */
    rdpa_if_lan2, rdpa_if_lan3, rdpa_if_lan4, rdpa_if_lan5,
    rdpa_if_lan6, rdpa_if_lan7, rdpa_if_switch, rdpa_if_none,
    rdpa_if_cpu, rdpa_if_none, rdpa_if_none, rdpa_if_none,
    rdpa_if_none, rdpa_if_wan0, rdpa_if_wan0, rdpa_if_wlan0};
EXPORT_SYMBOL(map_rdd_to_rdpa_if);
#elif defined(WL4908)
static const rdpa_if map_rdd_to_rdpa_if[] __attribute__((aligned(L1_CACHE_BYTES)))=
   {rdpa_if_wan0, rdpa_if_wan1, rdpa_if_lan0, rdpa_if_lan1, /* FIXME: MULTI-WAN */
    rdpa_if_lan2, rdpa_if_lan3, rdpa_if_lan4, rdpa_if_lan5,
    rdpa_if_lan6, rdpa_if_lan7, rdpa_if_switch, rdpa_if_none,
    rdpa_if_cpu, rdpa_if_none, rdpa_if_none, rdpa_if_none,
    rdpa_if_none, rdpa_if_wan0, rdpa_if_wan0, rdpa_if_wlan0};
EXPORT_SYMBOL(map_rdd_to_rdpa_if);
#elif defined(CONFIG_BCM96838) || defined(CONFIG_BCM96848)
static const rdpa_if map_rdd_to_rdpa_if[] = {rdpa_if_wan0, rdpa_if_lan0, rdpa_if_lan1, rdpa_if_lan2, /* FIXME: MULTI-WAN */
    rdpa_if_lan3, rdpa_if_lan4, rdpa_if_wan0, rdpa_if_wan0,
    rdpa_if_wlan0, rdpa_if_switch, rdpa_if_none,
    rdpa_if_none, rdpa_if_cpu};
EXPORT_SYMBOL(map_rdd_to_rdpa_if);

#elif defined(XRDP)
#define NUM_OF_VIRTUAL_PORTS 40
int rdd_vport_to_rdpa_if_set[NUM_OF_VIRTUAL_PORTS + 1]  = {};
rdpa_if rdd_vport_to_rdpa_if_map[NUM_OF_VIRTUAL_PORTS + 1];
EXPORT_SYMBOL(rdd_vport_to_rdpa_if_set);
EXPORT_SYMBOL(rdd_vport_to_rdpa_if_map);
#endif

/** Enable CPU queue interrupt */
void (*f_rdpa_cpu_int_enable)(rdpa_cpu_port port, int queue);
EXPORT_SYMBOL(f_rdpa_cpu_int_enable);
void rdpa_cpu_int_enable(rdpa_cpu_port port, int queue)
{
    if (!f_rdpa_cpu_int_enable)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return;
    }
    f_rdpa_cpu_int_enable(port, queue);
}
EXPORT_SYMBOL(rdpa_cpu_int_enable);

/** Disable CPU queue interrupt */
void (*f_rdpa_cpu_int_disable)(rdpa_cpu_port port, int queue);
EXPORT_SYMBOL(f_rdpa_cpu_int_disable);
void rdpa_cpu_int_disable(rdpa_cpu_port port, int queue)
{
    if (!f_rdpa_cpu_int_disable)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return;
    }
    f_rdpa_cpu_int_disable(port, queue);
}
EXPORT_SYMBOL(rdpa_cpu_int_disable);

/** Clear CPU queue interrupt */
void (*f_rdpa_cpu_int_clear)(rdpa_cpu_port port, int queue);
EXPORT_SYMBOL(f_rdpa_cpu_int_clear);
void rdpa_cpu_int_clear(rdpa_cpu_port port, int queue)
{
    if (!f_rdpa_cpu_int_clear)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return;
    }
    f_rdpa_cpu_int_clear(port, queue);
}
EXPORT_SYMBOL(rdpa_cpu_int_clear);

/** Enable CPU queue interrupt */
void (*f_rdpa_rnr_int_enable)(uint8_t intr_idx);
EXPORT_SYMBOL(f_rdpa_rnr_int_enable);
void rdpa_rnr_int_enable(uint8_t intr_idx)
{
    if (!f_rdpa_rnr_int_enable)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return;
    }
    f_rdpa_rnr_int_enable(intr_idx);
}
EXPORT_SYMBOL(rdpa_rnr_int_enable);

/** Disable CPU queue interrupt */
void (*f_rdpa_rnr_int_disable)(uint8_t intr_idx);
EXPORT_SYMBOL(f_rdpa_rnr_int_disable);
void rdpa_rnr_int_disable(uint8_t intr_idx)
{
    if (!f_rdpa_rnr_int_disable)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return;
    }
    f_rdpa_rnr_int_disable(intr_idx);
}
EXPORT_SYMBOL(rdpa_rnr_int_disable);

/** Clear CPU queue interrupt */
void (*f_rdpa_rnr_int_clear)(uint8_t intr_idx);
EXPORT_SYMBOL(f_rdpa_rnr_int_clear);
void rdpa_rnr_int_clear(uint8_t intr_idx)
{
    if (!f_rdpa_rnr_int_clear)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return;
    }
    f_rdpa_rnr_int_clear(intr_idx);
}
EXPORT_SYMBOL(rdpa_rnr_int_clear);

/** Pull a single received packet from host queue. */
int (*f_rdpa_cpu_packet_get)(rdpa_cpu_port port, bdmf_index queue,
    rdpa_cpu_rx_info_t *info);
EXPORT_SYMBOL(f_rdpa_cpu_packet_get);
int rdpa_cpu_packet_get(rdpa_cpu_port port, bdmf_index queue,
    rdpa_cpu_rx_info_t *info)
{
    if (!f_rdpa_cpu_packet_get)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return BDMF_ERR_STATE;
    }
    return f_rdpa_cpu_packet_get(port, queue, info);
}
EXPORT_SYMBOL(rdpa_cpu_packet_get);

/** Pull a single received packet from host queue. */
int (*f_rdpa_cpu_packet_get_redirected)(rdpa_cpu_port port, bdmf_index queue,
    rdpa_cpu_rx_info_t *info, rdpa_cpu_rx_ext_info_t *ext_info);
EXPORT_SYMBOL(f_rdpa_cpu_packet_get_redirected);
int rdpa_cpu_packet_get_redirected(rdpa_cpu_port port, bdmf_index queue,
    rdpa_cpu_rx_info_t *info, rdpa_cpu_rx_ext_info_t *ext_info)
{
    if (!f_rdpa_cpu_packet_get_redirected)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return BDMF_ERR_STATE;
    }
    return f_rdpa_cpu_packet_get_redirected(port, queue, info, ext_info);
}
EXPORT_SYMBOL(rdpa_cpu_packet_get_redirected);

/** Pull a bulk of received packets from host queue. */
int (*f_rdpa_cpu_packets_bulk_get)(rdpa_cpu_port port, bdmf_index queue,
    rdpa_cpu_rx_info_t *info, int max_count, int *count);
EXPORT_SYMBOL(f_rdpa_cpu_packets_bulk_get);
int rdpa_cpu_packets_bulk_get(rdpa_cpu_port port, bdmf_index queue,
    rdpa_cpu_rx_info_t *info, int max_count, int *count)
{
    if (!f_rdpa_cpu_packets_bulk_get)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return BDMF_ERR_STATE;
    }
    return f_rdpa_cpu_packets_bulk_get(port, queue, info, max_count, count);
}
EXPORT_SYMBOL(rdpa_cpu_packets_bulk_get);

int (*f_rdpa_cpu_loopback_packet_get)(rdpa_cpu_loopback_type loopback_type, bdmf_index queue,
                                      bdmf_sysb *sysb, rdpa_cpu_rx_info_t *info);
EXPORT_SYMBOL(f_rdpa_cpu_loopback_packet_get);

int rdpa_cpu_loopback_packet_get(rdpa_cpu_loopback_type loopback_type, bdmf_index queue,
                                 bdmf_sysb *sysb, rdpa_cpu_rx_info_t *info)
{
    if (!f_rdpa_cpu_loopback_packet_get)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return BDMF_ERR_STATE;
    }
    return f_rdpa_cpu_loopback_packet_get(loopback_type, queue, sysb, info);
}
EXPORT_SYMBOL(rdpa_cpu_loopback_packet_get);

/** Get the Time Of Day from the FW FIFO, by the ptp index */
int (*f_rdpa_cpu_ptp_1588_get_tod)(uint16_t ptp_index, uint32_t *tod_h,
    uint32_t *tod_l, uint16_t *local_counter_delta);
EXPORT_SYMBOL(f_rdpa_cpu_ptp_1588_get_tod);
int rdpa_cpu_ptp_1588_get_tod(uint16_t ptp_index, uint32_t *tod_h,
    uint32_t *tod_l, uint16_t *local_counter_delta)
{
    if (!f_rdpa_cpu_ptp_1588_get_tod)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return BDMF_ERR_STATE;
    }
    return f_rdpa_cpu_ptp_1588_get_tod(ptp_index, tod_h, tod_l, local_counter_delta);
}
EXPORT_SYMBOL(rdpa_cpu_ptp_1588_get_tod);

/** similar to rdpa_cpu_send_sysb, but treats only ptp-1588 packets */
int (*f_rdpa_cpu_send_sysb_ptp)(bdmf_sysb sysb, const rdpa_cpu_tx_info_t *info);
EXPORT_SYMBOL(f_rdpa_cpu_send_sysb_ptp);
int rdpa_cpu_send_sysb_ptp(bdmf_sysb sysb, const rdpa_cpu_tx_info_t *info)
{
    if (!f_rdpa_cpu_send_sysb_ptp)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return BDMF_ERR_STATE;
    }
    return f_rdpa_cpu_send_sysb_ptp(sysb, info);
}
EXPORT_SYMBOL(rdpa_cpu_send_sysb_ptp);

/** Send system buffer */
int (*f_rdpa_cpu_send_sysb)(bdmf_sysb sysb, const rdpa_cpu_tx_info_t *info);
EXPORT_SYMBOL(f_rdpa_cpu_send_sysb);
int rdpa_cpu_send_sysb(bdmf_sysb sysb, const rdpa_cpu_tx_info_t *info)
{
    if (!f_rdpa_cpu_send_sysb)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return BDMF_ERR_STATE;
    }
    return f_rdpa_cpu_send_sysb(sysb, info);
}
EXPORT_SYMBOL(rdpa_cpu_send_sysb);

/** Send system buffer allocated from FPM pool */
int (*f_rdpa_cpu_send_sysb_fpm)(bdmf_sysb sysb, const rdpa_cpu_tx_info_t *info);
EXPORT_SYMBOL(f_rdpa_cpu_send_sysb_fpm);
int rdpa_cpu_send_sysb_fpm(bdmf_sysb sysb, const rdpa_cpu_tx_info_t *info)
{
    if (!f_rdpa_cpu_send_sysb_fpm)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return BDMF_ERR_STATE;
    }
    return f_rdpa_cpu_send_sysb_fpm(sysb, info);
}
EXPORT_SYMBOL(rdpa_cpu_send_sysb_fpm);

/** Send system buffer from WFD*/
int (*f_rdpa_cpu_send_wfd_to_bridge)(bdmf_sysb sysb, const rdpa_cpu_tx_info_t *info, size_t offset_next);
EXPORT_SYMBOL(f_rdpa_cpu_send_wfd_to_bridge);

int rdpa_cpu_send_wfd_to_bridge(bdmf_sysb sysb, const rdpa_cpu_tx_info_t *info, size_t offset_next)
{
    if (unlikely(!f_rdpa_cpu_send_wfd_to_bridge))
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return BDMF_ERR_STATE;
    }
    return f_rdpa_cpu_send_wfd_to_bridge(sysb, info, offset_next);
}
EXPORT_SYMBOL(rdpa_cpu_send_wfd_to_bridge);

#if defined(BCM_DSL_RDP) || defined(BCM63158)
/** Send system buffer to WAN */
int (*f_rdpa_cpu_tx_port_enet_or_dsl_wan)(bdmf_sysb sysb, uint32_t egress_queue, rdpa_flow wanFlow,
                                          rdpa_if wanIf, rdpa_cpu_tx_extra_info_t extra_info);
EXPORT_SYMBOL(f_rdpa_cpu_tx_port_enet_or_dsl_wan);
int rdpa_cpu_tx_port_enet_or_dsl_wan(bdmf_sysb sysb, uint32_t egress_queue, rdpa_flow wanFlow,
                                     rdpa_if wanIf, rdpa_cpu_tx_extra_info_t extra_info)
{
    return f_rdpa_cpu_tx_port_enet_or_dsl_wan(sysb, egress_queue, wanFlow, wanIf, extra_info);
}
EXPORT_SYMBOL(rdpa_cpu_tx_port_enet_or_dsl_wan);
#endif

#if defined(BCM_DSL_RDP)
/** Send system buffer to LAN */
int (*f_rdpa_cpu_tx_port_enet_lan)(bdmf_sysb sysb, uint32_t egress_queue, uint32_t phys_port, rdpa_cpu_tx_extra_info_t extra_info);
EXPORT_SYMBOL(f_rdpa_cpu_tx_port_enet_lan);
int rdpa_cpu_tx_port_enet_lan(bdmf_sysb sysb, uint32_t egress_queue, uint32_t phys_port, rdpa_cpu_tx_extra_info_t extra_info)
{
    return f_rdpa_cpu_tx_port_enet_lan(sysb, egress_queue, phys_port, extra_info);
}
EXPORT_SYMBOL(rdpa_cpu_tx_port_enet_lan);

/** Send system buffer to Flow Cache offload */
int (*f_rdpa_cpu_tx_flow_cache_offload)(bdmf_sysb sysb, uint32_t cpu_rx_queue, int dirty);
EXPORT_SYMBOL(f_rdpa_cpu_tx_flow_cache_offload);
int rdpa_cpu_tx_flow_cache_offload(bdmf_sysb sysb, uint32_t cpu_rx_queue, int dirty)
{
    return f_rdpa_cpu_tx_flow_cache_offload(sysb, cpu_rx_queue, dirty);
}
EXPORT_SYMBOL(rdpa_cpu_tx_flow_cache_offload);

/** Send system buffer to IPsec offload */
int (*f_rdpa_cpu_tx_ipsec_offload)(bdmf_sysb sysb,
                                   rdpa_traffic_dir dir,
                                   uint8_t esphdr_offset,
                                   uint8_t sa_index,
                                   uint8_t sa_update,
                                   uint8_t cpu_qid);
EXPORT_SYMBOL(f_rdpa_cpu_tx_ipsec_offload);

int rdpa_cpu_tx_ipsec_offload(bdmf_sysb sysb, rdpa_traffic_dir dir, uint8_t esphdr_offset,
                              uint8_t sa_index, uint8_t sa_update, uint8_t cpu_qid)
{
  return f_rdpa_cpu_tx_ipsec_offload(sysb, dir, esphdr_offset, sa_index, sa_update, cpu_qid);
}
EXPORT_SYMBOL(rdpa_cpu_tx_ipsec_offload);

/** Frees the given free index and returns a pointer to the associated System Buffer */
bdmf_sysb (*f_rdpa_cpu_return_free_index)(uint16_t free_index);
EXPORT_SYMBOL(f_rdpa_cpu_return_free_index);
bdmf_sysb rdpa_cpu_return_free_index(uint16_t free_index)
{
    return f_rdpa_cpu_return_free_index(free_index);
}
EXPORT_SYMBOL(rdpa_cpu_return_free_index);

/** Receive Ethernet system buffer */
int (*f_rdpa_cpu_host_packet_get_enet)(bdmf_index queue, bdmf_sysb *sysb, rdpa_if *src_port);
EXPORT_SYMBOL(f_rdpa_cpu_host_packet_get_enet);
int rdpa_cpu_host_packet_get_enet(bdmf_index queue, bdmf_sysb *sysb, rdpa_if *src_port)
{
    return f_rdpa_cpu_host_packet_get_enet(queue, sysb, src_port);
}
EXPORT_SYMBOL(rdpa_cpu_host_packet_get_enet);

void (*f_rdpa_cpu_tx_reclaim)(void) = NULL;
EXPORT_SYMBOL(f_rdpa_cpu_tx_reclaim);
void rdpa_cpu_tx_reclaim(void)
{
    if (f_rdpa_cpu_tx_reclaim)
    	f_rdpa_cpu_tx_reclaim();
}
EXPORT_SYMBOL(rdpa_cpu_tx_reclaim);

#endif

int (*f_rdpa_egress_tm_queue_exists)(rdpa_if port, uint32_t queue_id) = NULL;
EXPORT_SYMBOL(f_rdpa_egress_tm_queue_exists);
int rdpa_egress_tm_queue_exists(rdpa_if port, uint32_t queue_id)
{
    if (f_rdpa_egress_tm_queue_exists)
    	return f_rdpa_egress_tm_queue_exists(port, queue_id);

    return BDMF_ERR_NOENT;
}
EXPORT_SYMBOL(rdpa_egress_tm_queue_exists);


/** Receive bulk ethernet system buffers for WFD */
int (*f_rdpa_cpu_wfd_bulk_fkb_get)(bdmf_index queue_id, unsigned int budget, void **rx_pkts, void *wfd_acc_info_p);
EXPORT_SYMBOL(f_rdpa_cpu_wfd_bulk_fkb_get);
int rdpa_cpu_wfd_bulk_fkb_get(bdmf_index queue_id, unsigned int budget, void **rx_pkts, void *wfd_acc_info_p)
{
    return f_rdpa_cpu_wfd_bulk_fkb_get(queue_id, budget, rx_pkts, wfd_acc_info_p);
}
EXPORT_SYMBOL(rdpa_cpu_wfd_bulk_fkb_get);

int (*f_rdpa_cpu_wfd_bulk_skb_get)(bdmf_index queue_id, unsigned int budget, void **rx_pkts, void *wfd_acc_info_p);
EXPORT_SYMBOL(f_rdpa_cpu_wfd_bulk_skb_get);
int rdpa_cpu_wfd_bulk_skb_get(bdmf_index queue_id, unsigned int budget, void **rx_pkts, void *wfd_acc_info_p)
{
    return f_rdpa_cpu_wfd_bulk_skb_get(queue_id, budget, rx_pkts, wfd_acc_info_p);
}
EXPORT_SYMBOL(rdpa_cpu_wfd_bulk_skb_get);

void *(*f_rdpa_cpu_data_get)(int rdpa_cpu_type);
EXPORT_SYMBOL(f_rdpa_cpu_data_get);
void *rdpa_cpu_data_get(int rdpa_cpu_type)
{
    return f_rdpa_cpu_data_get(rdpa_cpu_type);
}
EXPORT_SYMBOL(rdpa_cpu_data_get);

/** Send raw packet */
int (*f_rdpa_cpu_send_raw)(void *data, uint32_t length, const rdpa_cpu_tx_info_t *info);
EXPORT_SYMBOL(f_rdpa_cpu_send_raw);
int rdpa_cpu_send_raw(void *data, uint32_t length, const rdpa_cpu_tx_info_t *info)
{
    if (!f_rdpa_cpu_send_raw)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return BDMF_ERR_STATE;
    }
    return f_rdpa_cpu_send_raw(data, length, info);
}
EXPORT_SYMBOL(rdpa_cpu_send_raw);

/** Map from HW port to rdpa_if */
rdpa_if (*f_rdpa_port_map_from_hw_port)(int hw_port, bdmf_boolean emac_only);
EXPORT_SYMBOL(f_rdpa_port_map_from_hw_port);
rdpa_if rdpa_port_map_from_hw_port(int hw_port, bdmf_boolean emac_only)
{
    if (!f_rdpa_port_map_from_hw_port)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return rdpa_if_none;
    }
    return f_rdpa_port_map_from_hw_port(hw_port, emac_only);
}
EXPORT_SYMBOL(rdpa_port_map_from_hw_port);

int (*f_rdpa_cpu_queue_not_empty)(rdpa_cpu_port port, bdmf_index queue);
EXPORT_SYMBOL(f_rdpa_cpu_queue_not_empty);
int rdpa_cpu_queue_not_empty(rdpa_cpu_port port, bdmf_index queue)
{
    if (!f_rdpa_cpu_queue_not_empty)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return BDMF_ERR_STATE;
    }
    return f_rdpa_cpu_queue_not_empty(port, queue);
}
EXPORT_SYMBOL(rdpa_cpu_queue_not_empty);

int (*f_rdpa_cpu_queue_is_full)(rdpa_cpu_port port, bdmf_index queue);
EXPORT_SYMBOL(f_rdpa_cpu_queue_is_full);
int rdpa_cpu_queue_is_full(rdpa_cpu_port port, bdmf_index queue)
{
    if (!f_rdpa_cpu_queue_is_full)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return BDMF_ERR_STATE;
    }
    return f_rdpa_cpu_queue_is_full(port, queue);
}
EXPORT_SYMBOL(rdpa_cpu_queue_is_full);

/** Send EPON Dgasp */
int (*f_rdpa_cpu_send_epon_dying_gasp)(bdmf_sysb sysb, const rdpa_cpu_tx_info_t *info);
EXPORT_SYMBOL(f_rdpa_cpu_send_epon_dying_gasp);
int rdpa_cpu_send_epon_dying_gasp(bdmf_sysb sysb, const rdpa_cpu_tx_info_t *info)
{
    if (!f_rdpa_cpu_send_epon_dying_gasp)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return BDMF_ERR_STATE;
    }
    return f_rdpa_cpu_send_epon_dying_gasp(sysb, info);
}
EXPORT_SYMBOL(rdpa_cpu_send_epon_dying_gasp);

int (*f_rdpa_cpu_is_per_port_metering_supported)(rdpa_cpu_reason reason);
EXPORT_SYMBOL(f_rdpa_cpu_is_per_port_metering_supported);
int rdpa_cpu_is_per_port_metering_supported(rdpa_cpu_reason reason)
{
    if (!f_rdpa_cpu_is_per_port_metering_supported)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return BDMF_ERR_STATE;
    }
    return f_rdpa_cpu_is_per_port_metering_supported(reason);
}
EXPORT_SYMBOL(rdpa_cpu_is_per_port_metering_supported);

rdpa_ports (*f_rdpa_ports_all_lan)(void);
EXPORT_SYMBOL(f_rdpa_ports_all_lan);
rdpa_ports rdpa_ports_all_lan()
{
    if (!f_rdpa_ports_all_lan)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return BDMF_ERR_STATE;
    }
    return f_rdpa_ports_all_lan();
}
EXPORT_SYMBOL(rdpa_ports_all_lan);

void (*f_rdpa_cpu_rx_dump_packet)(char *name, rdpa_cpu_port port,
    bdmf_index queue, rdpa_cpu_rx_info_t *info, uint32_t dst_ssid);
EXPORT_SYMBOL(f_rdpa_cpu_rx_dump_packet);
void rdpa_cpu_rx_dump_packet(char *name, rdpa_cpu_port port,
    bdmf_index queue, rdpa_cpu_rx_info_t *info, uint32_t dst_ssid)
{
    if (!f_rdpa_cpu_rx_dump_packet)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return;
    }
    f_rdpa_cpu_rx_dump_packet(name, port, queue, info, dst_ssid);
    return;
}
EXPORT_SYMBOL(rdpa_cpu_rx_dump_packet);

/** Run time mapping from HW port to rdpa_if using array */
rdpa_if (*f_rdpa_physical_port_to_rdpa_if)(rdpa_physical_port port);
EXPORT_SYMBOL(f_rdpa_physical_port_to_rdpa_if);
rdpa_if rdpa_physical_port_to_rdpa_if(rdpa_physical_port port)
{
    if (!f_rdpa_physical_port_to_rdpa_if)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return rdpa_if_none;
    }
    return f_rdpa_physical_port_to_rdpa_if(port);
}
EXPORT_SYMBOL(rdpa_physical_port_to_rdpa_if);

#ifdef CONFIG_DHD_RUNNER
/* DHD Helper */
void (*f_rdpa_dhd_helper_complete_wakeup)(uint32_t radio_idx, bdmf_boolean is_tx_complete);
EXPORT_SYMBOL(f_rdpa_dhd_helper_complete_wakeup);
void rdpa_dhd_helper_complete_wakeup(uint32_t radio_idx, bdmf_boolean is_tx_complete)
{
    if (!f_rdpa_dhd_helper_complete_wakeup)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return;
    }
    f_rdpa_dhd_helper_complete_wakeup(radio_idx, is_tx_complete);
}
EXPORT_SYMBOL(rdpa_dhd_helper_complete_wakeup);

int (*f_rdpa_dhd_helper_send_packet_to_dongle)(void *data, uint32_t length, const rdpa_dhd_tx_post_info_t *info);
EXPORT_SYMBOL(f_rdpa_dhd_helper_send_packet_to_dongle);
int rdpa_dhd_helper_send_packet_to_dongle(void *data, uint32_t length, const rdpa_dhd_tx_post_info_t *info)
{
    if (!f_rdpa_dhd_helper_send_packet_to_dongle)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return BDMF_ERR_STATE;
    }
    return f_rdpa_dhd_helper_send_packet_to_dongle(data, length, info);
}
EXPORT_SYMBOL(rdpa_dhd_helper_send_packet_to_dongle);

void (*f_rdpa_dhd_helper_doorbell_interrupt_clear)(uint32_t radio_idx);
EXPORT_SYMBOL(f_rdpa_dhd_helper_doorbell_interrupt_clear);
void rdpa_dhd_helper_doorbell_interrupt_clear(uint32_t radio_idx)
{
    if (unlikely(!f_rdpa_dhd_helper_doorbell_interrupt_clear))
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
    f_rdpa_dhd_helper_doorbell_interrupt_clear(radio_idx);
}
EXPORT_SYMBOL(rdpa_dhd_helper_doorbell_interrupt_clear);

void (*f_rdpa_dhd_helper_wakeup_information_get)(rdpa_dhd_wakeup_info_t *wakeup_info);
EXPORT_SYMBOL(f_rdpa_dhd_helper_wakeup_information_get);
void rdpa_dhd_helper_wakeup_information_get(rdpa_dhd_wakeup_info_t *wakeup_info)
{
    if (!f_rdpa_dhd_helper_wakeup_information_get)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return;
    }
    f_rdpa_dhd_helper_wakeup_information_get(wakeup_info);
}
EXPORT_SYMBOL(rdpa_dhd_helper_wakeup_information_get);

int (*f_rdpa_dhd_helper_dhd_complete_ring_create)(uint32_t radio_idx, uint32_t ring_size);
EXPORT_SYMBOL(f_rdpa_dhd_helper_dhd_complete_ring_create);
int rdpa_dhd_helper_dhd_complete_ring_create(uint32_t radio_idx, uint32_t ring_size)
{
    if (!f_rdpa_dhd_helper_dhd_complete_ring_create)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return BDMF_ERR_STATE;
    }
    return f_rdpa_dhd_helper_dhd_complete_ring_create(radio_idx, ring_size);
}
EXPORT_SYMBOL(rdpa_dhd_helper_dhd_complete_ring_create);

int (*f_rdpa_dhd_helper_dhd_complete_ring_destroy)(uint32_t radio_idx, uint32_t ring_size);
EXPORT_SYMBOL(f_rdpa_dhd_helper_dhd_complete_ring_destroy);
int rdpa_dhd_helper_dhd_complete_ring_destroy(uint32_t radio_idx, uint32_t ring_size)
{
    if (!f_rdpa_dhd_helper_dhd_complete_ring_destroy)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return BDMF_ERR_STATE;
    }
    return f_rdpa_dhd_helper_dhd_complete_ring_destroy(radio_idx, ring_size);
}
EXPORT_SYMBOL(rdpa_dhd_helper_dhd_complete_ring_destroy);

int (*f_rdpa_dhd_helper_dhd_complete_message_get)(rdpa_dhd_complete_data_t *dhd_complete_info);
EXPORT_SYMBOL(f_rdpa_dhd_helper_dhd_complete_message_get);
int rdpa_dhd_helper_dhd_complete_message_get(rdpa_dhd_complete_data_t *dhd_complete_info)
{
    if (!f_rdpa_dhd_helper_dhd_complete_message_get)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return BDMF_ERR_STATE;
    }
    return f_rdpa_dhd_helper_dhd_complete_message_get(dhd_complete_info);
}
EXPORT_SYMBOL(rdpa_dhd_helper_dhd_complete_message_get);
#endif

int (*f_rdpa_tcont_sr_dba_callback)(uint32_t tcont_id, uint32_t *runner_ddr_occupancy);
EXPORT_SYMBOL(f_rdpa_tcont_sr_dba_callback);

int rdpa_tcont_sr_dba_callback(uint32_t tcont_id, uint32_t *runner_ddr_occupancy)
{
    if (!f_rdpa_tcont_sr_dba_callback)
        return 0;
    return f_rdpa_tcont_sr_dba_callback(tcont_id, runner_ddr_occupancy);
}
EXPORT_SYMBOL(rdpa_tcont_sr_dba_callback);

void (*f_epon_get_mode)(rdpa_epon_mode* const mode);
EXPORT_SYMBOL(f_epon_get_mode);

void epon_get_mode(rdpa_epon_mode* const mode)
{
    if (!f_epon_get_mode)
        return;
    f_epon_get_mode(mode);
}
EXPORT_SYMBOL(epon_get_mode);

epon_l2_l1_map * (*f_rdpa_llid_l2_l1_map_get)(uint8_t l1_queue);
EXPORT_SYMBOL(f_rdpa_llid_l2_l1_map_get);
void rdpa_llid_l2_l1_map_get(uint8_t l1_queue)
{
    if (!f_rdpa_llid_l2_l1_map_get)
        return;
    f_rdpa_llid_l2_l1_map_get(l1_queue);
}
EXPORT_SYMBOL(rdpa_llid_l2_l1_map_get);

#if defined(CONFIG_BCM96858) || defined(CONFIG_BCM96846) || defined(CONFIG_BCM96856) || defined(CONFIG_BCM963158) || defined(CONFIG_BCM96878)
int (*f_rdpa_wan_tx_bbh_flush_status_get)(uint8_t tcont_id, bdmf_boolean *bbh_flush_done_p);
EXPORT_SYMBOL(f_rdpa_wan_tx_bbh_flush_status_get);
int rdpa_wan_tx_bbh_flush_status_get(uint8_t tcont_id, bdmf_boolean *bbh_flush_done_p)
{
     if (!f_rdpa_wan_tx_bbh_flush_status_get)
         return 0;
     return f_rdpa_wan_tx_bbh_flush_status_get(tcont_id, bbh_flush_done_p);
}
EXPORT_SYMBOL(rdpa_wan_tx_bbh_flush_status_get);
#endif
