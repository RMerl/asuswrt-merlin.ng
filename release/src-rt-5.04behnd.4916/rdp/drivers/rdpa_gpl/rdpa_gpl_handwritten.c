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

#include <linux/of_platform.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>

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

#if defined(BCM_DSL_XRDP)
/** Send system buffer to WAN */
int (*f_rdpa_cpu_tx_port_enet_or_dsl_wan)(bdmf_sysb sysb, uint32_t egress_queue, rdpa_flow wanFlow,
                                          bdmf_object_handle port_obj, rdpa_cpu_tx_extra_info_t extra_info);
EXPORT_SYMBOL(f_rdpa_cpu_tx_port_enet_or_dsl_wan);
int rdpa_cpu_tx_port_enet_or_dsl_wan(bdmf_sysb sysb, uint32_t egress_queue, rdpa_flow wanFlow,
                                     bdmf_object_handle port_obj, rdpa_cpu_tx_extra_info_t extra_info)
{
    return f_rdpa_cpu_tx_port_enet_or_dsl_wan(sysb, egress_queue, wanFlow, port_obj, extra_info);
}
EXPORT_SYMBOL(rdpa_cpu_tx_port_enet_or_dsl_wan);
#endif

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

/** Send system buffer */
int (*f_rdpa_udpspdtest_tx_start)(const rdpa_cpu_tx_info_t *info, int to_wan, bdmf_sysb sysb);
EXPORT_SYMBOL(f_rdpa_udpspdtest_tx_start);
int rdpa_udpspdtest_tx_start(const rdpa_cpu_tx_info_t *info, int to_wan, bdmf_sysb sysb)
{
    if (!f_rdpa_udpspdtest_tx_start)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return BDMF_ERR_STATE;
    }
    return f_rdpa_udpspdtest_tx_start(info, to_wan, sysb);
}
EXPORT_SYMBOL(rdpa_udpspdtest_tx_start);

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

int (*f_rdpa_dhd_helper_dhd_pcap_packet_get)(rdpa_dhd_pcap_data_t *dhd_pcap_info);
EXPORT_SYMBOL(f_rdpa_dhd_helper_dhd_pcap_packet_get);
int rdpa_dhd_helper_dhd_pcap_packet_get(rdpa_dhd_pcap_data_t *dhd_pcap_info)
{
    if (!f_rdpa_dhd_helper_dhd_pcap_packet_get)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return BDMF_ERR_STATE;
    }
    return f_rdpa_dhd_helper_dhd_pcap_packet_get(dhd_pcap_info);
}
EXPORT_SYMBOL(rdpa_dhd_helper_dhd_pcap_packet_get);
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

#if defined(CONFIG_BCM96858) || defined(CONFIG_BCM96856) || defined(CONFIG_BCM963158) || \
    defined(CONFIG_BCM96855) || defined(CONFIG_BCM96888) || defined(CONFIG_BCM968880) || defined(CONFIG_BCM96837) || defined(CONFIG_BCM968375)|| \
    defined(CONFIG_BCM96813) || defined(CONFIG_BCM968850)
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

#if defined(CONFIG_CPU_TX_FROM_XPM) || defined(CONFIG_CPU_RX_FROM_XPM)
void (*f_rdpa_buffer_free)(void *buffer_ptr);
EXPORT_SYMBOL(f_rdpa_buffer_free);
void rdpa_buffer_free(void *buffer_ptr)
{
    if (!f_rdpa_buffer_free)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return;
    }
    f_rdpa_buffer_free(buffer_ptr);
}
EXPORT_SYMBOL(rdpa_buffer_free);

void (*f_rdpa_buffer_free_mult)(uint8_t num_of_buffers, void **buffer_head);
EXPORT_SYMBOL(f_rdpa_buffer_free_mult);
void rdpa_buffer_free_mult(uint8_t num_of_buffers, void **buffer_head)
{
    if (!f_rdpa_buffer_free_mult)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return;
    }
    f_rdpa_buffer_free_mult(num_of_buffers, buffer_head);
}
EXPORT_SYMBOL(rdpa_buffer_free_mult);

bool (*f_rdpa_buffer_is_hw_buffer)(void *buffer_ptr);
EXPORT_SYMBOL(f_rdpa_buffer_is_hw_buffer);
uint32_t rdpa_buffer_is_hw_buffer(void *buffer_ptr)
{
    if (!f_rdpa_buffer_is_hw_buffer)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return 0;
    }
    return f_rdpa_buffer_is_hw_buffer(buffer_ptr);
}
EXPORT_SYMBOL(rdpa_buffer_is_hw_buffer);
#endif
#if defined(CONFIG_CPU_TX_FROM_XPM)
int (*f_rdpa_buffer_alloc)(bdmf_object_handle rdpa_buffer_ring, void **buffer_ptr);
EXPORT_SYMBOL(f_rdpa_buffer_alloc);
int rdpa_buffer_alloc(bdmf_object_handle rdpa_buffer_ring, void **buffer_ptr)
{
    if (!f_rdpa_buffer_alloc)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return BDMF_ERR_STATE;
    }
    return f_rdpa_buffer_alloc(rdpa_buffer_ring, buffer_ptr);
}
EXPORT_SYMBOL(rdpa_buffer_alloc);


int (*f_rdpa_buffer_alloc_mult)(bdmf_object_handle rdpa_buffer_ring, uint8_t num_of_buffers, void **buffer_head,
    void **buffer_tail);
EXPORT_SYMBOL(f_rdpa_buffer_alloc_mult);
int rdpa_buffer_alloc_mult(bdmf_object_handle rdpa_buffer_ring, uint8_t num_of_buffers, void **buffer_head,
    void **buffer_tail)
{
    if (!f_rdpa_buffer_alloc_mult)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return BDMF_ERR_STATE;
    }
    return f_rdpa_buffer_alloc_mult(rdpa_buffer_ring, num_of_buffers, buffer_head, buffer_tail);
}
EXPORT_SYMBOL(rdpa_buffer_alloc_mult);
#endif

int (*f_rdpa_cpu_send_raw_from_fpm)(void *data, uint32_t length, const rdpa_cpu_tx_info_t *info);
EXPORT_SYMBOL(f_rdpa_cpu_send_raw_from_fpm);
int rdpa_cpu_send_raw_from_fpm(void *data, uint32_t length, const rdpa_cpu_tx_info_t *info)
{
    if (!f_rdpa_cpu_send_raw_from_fpm)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return BDMF_ERR_STATE;
    }
    return f_rdpa_cpu_send_raw_from_fpm(data, length, info);
}

#if defined(CONFIG_RUNNER_SPU_OFFLOAD)
void (*f_rdpa_spu_set_pd_ring)(rdpa_spu_pd_ring_info_t *info);
EXPORT_SYMBOL(f_rdpa_spu_set_pd_ring);
void rdpa_spu_set_pd_ring(rdpa_spu_pd_ring_info_t *info)
{
    if (!f_rdpa_spu_set_pd_ring)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return;
    }
    f_rdpa_spu_set_pd_ring(info);
}
EXPORT_SYMBOL(rdpa_spu_set_pd_ring);

void (*f_rdpa_spu_crypto_session_base_set)(uint64_t seq_base_addr, uint64_t prepend_base_addr);
EXPORT_SYMBOL(f_rdpa_spu_crypto_session_base_set);
void rdpa_spu_crypto_session_base_set(uint64_t seq_base_addr, uint64_t prepend_base_addr)
{
    if (!f_rdpa_spu_crypto_session_base_set)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return;
    }
    f_rdpa_spu_crypto_session_base_set(seq_base_addr, prepend_base_addr);
}
EXPORT_SYMBOL(rdpa_spu_crypto_session_base_set);

void (*f_rdpa_crypto_session_info_set)(rdpa_crypto_session_info_t *session);
EXPORT_SYMBOL(f_rdpa_crypto_session_info_set);
void rdpa_crypto_session_info_set(rdpa_crypto_session_info_t *session)
{
    if (!f_rdpa_crypto_session_info_set)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return;
    }
    f_rdpa_crypto_session_info_set(session);
}
EXPORT_SYMBOL(rdpa_crypto_session_info_set);

void (*f_rdpa_spu_databuf_recycle)(uint32_t plen, void *buffer_ptr);
EXPORT_SYMBOL(f_rdpa_spu_databuf_recycle);
void rdpa_spu_databuf_recycle(uint32_t plen, void *buffer_ptr)
{
    if (!f_rdpa_spu_databuf_recycle)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return;
    }
    f_rdpa_spu_databuf_recycle(plen, buffer_ptr);
}
EXPORT_SYMBOL(rdpa_spu_databuf_recycle);

int (*f_rdpa_spu_resp_wakeup_information_get)(rdpa_spu_resp_wakeup_info_t *info);
EXPORT_SYMBOL(f_rdpa_spu_resp_wakeup_information_get);
int rdpa_spu_resp_wakeup_information_get(rdpa_spu_resp_wakeup_info_t *info)
{
    if (!f_rdpa_spu_resp_wakeup_information_get)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return BDMF_ERR_STATE;
    }
    return f_rdpa_spu_resp_wakeup_information_get(info);
}
EXPORT_SYMBOL(rdpa_spu_resp_wakeup_information_get);

int (*f_rdpa_spu_set_cpu_irq)(struct bdmf_object *mo, int idx);
EXPORT_SYMBOL(f_rdpa_spu_set_cpu_irq);
int rdpa_spu_set_cpu_irq(struct bdmf_object *mo, int idx)
{
    if (!f_rdpa_spu_set_cpu_irq)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return BDMF_ERR_STATE;
    }
    return f_rdpa_spu_set_cpu_irq(mo, idx);
}
EXPORT_SYMBOL(rdpa_spu_set_cpu_irq);

void (*f_rdpa_spu_offload_pkt)(rdpa_spu_offload_stat_t *stat);
EXPORT_SYMBOL(f_rdpa_spu_offload_pkt);
void rdpa_spu_offload_pkt(rdpa_spu_offload_stat_t *stat)
{
    if (!f_rdpa_spu_offload_pkt)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return;
    }
    f_rdpa_spu_offload_pkt(stat);
}
EXPORT_SYMBOL(rdpa_spu_offload_pkt);

void (*f_rdpa_spu_offload_tunnel_mapping)(uint32_t tun_id, int32_t session_id);
EXPORT_SYMBOL(f_rdpa_spu_offload_tunnel_mapping);
void rdpa_spu_offload_tunnel_mapping(uint32_t tun_id, int32_t session_id)
{
    if (!f_rdpa_spu_offload_tunnel_mapping)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return;
    }
    f_rdpa_spu_offload_tunnel_mapping(tun_id, session_id);
}
EXPORT_SYMBOL(rdpa_spu_offload_tunnel_mapping);
#endif

#if defined(CONFIG_RUNNER_GDX_SUPPORT)
void (*f_rdpa_gdx_params_set)(rdpa_gdx_params_t *gdx_params);
EXPORT_SYMBOL(f_rdpa_gdx_params_set);
void rdpa_gdx_params_set(rdpa_gdx_params_t *gdx_params)
{
    if (!f_rdpa_gdx_params_set)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return;
    }
    f_rdpa_gdx_params_set(gdx_params);
}
EXPORT_SYMBOL(rdpa_gdx_params_set);
#endif

#if defined(CONFIG_DYNAMIC_LB_LAG_SUPPORT)
int (*f_rdpa_lag_hash_stat_get)(uint32_t hash_idx, uint64_t *tx_bytes);
EXPORT_SYMBOL(f_rdpa_lag_hash_stat_get);
int rdpa_lag_hash_stat_get(uint32_t hash_idx, uint64_t *tx_bytes)
{
    if (!f_rdpa_lag_hash_stat_get)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return BDMF_ERR_STATE;
    }
    return f_rdpa_lag_hash_stat_get(hash_idx, tx_bytes);
}
EXPORT_SYMBOL(rdpa_lag_hash_stat_get);
#endif

#if defined(SCORE_SUPPORT)
int (*f_rdpa_score_msg_ring_create)(uint32_t ring_size);
EXPORT_SYMBOL(f_rdpa_score_msg_ring_create);
int rdpa_score_msg_ring_create(uint32_t ring_size)
{
    if (!f_rdpa_score_msg_ring_create)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return BDMF_ERR_STATE;
    }
    return f_rdpa_score_msg_ring_create(ring_size);
}
EXPORT_SYMBOL(rdpa_score_msg_ring_create);

int (*f_rdpa_score_msg_ring_destroy)(uint32_t ring_size);
EXPORT_SYMBOL(f_rdpa_score_msg_ring_destroy);
int rdpa_score_msg_ring_destroy(uint32_t ring_size)
{
    if (!f_rdpa_score_msg_ring_destroy)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return BDMF_ERR_STATE;
    }
    return f_rdpa_score_msg_ring_destroy(ring_size);
}
EXPORT_SYMBOL(rdpa_score_msg_ring_destroy);

int (*f_rdpa_score_msg_ring_info_get)(rdpa_score_data_t *score_msg_info);
EXPORT_SYMBOL(f_rdpa_score_msg_ring_info_get);
int rdpa_score_msg_ring_info_get(rdpa_score_data_t *score_msg_info)
{
    if (!f_rdpa_score_msg_ring_info_get)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return BDMF_ERR_STATE;
    }
    return f_rdpa_score_msg_ring_info_get(score_msg_info);
}
EXPORT_SYMBOL(rdpa_score_msg_ring_info_get);

int (*f_rdpa_score_qstat_ring_create)(uint32_t ring_size);
EXPORT_SYMBOL(f_rdpa_score_qstat_ring_create);
int rdpa_score_qstat_ring_create(uint32_t ring_size)
{
    if (!f_rdpa_score_qstat_ring_create)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return BDMF_ERR_STATE;
    }
    return f_rdpa_score_qstat_ring_create(ring_size);
}
EXPORT_SYMBOL(rdpa_score_qstat_ring_create);

int (*f_rdpa_score_qstat_ring_destroy)(uint32_t ring_size);
EXPORT_SYMBOL(f_rdpa_score_qstat_ring_destroy);
int rdpa_score_qstat_ring_destroy(uint32_t ring_size)
{
    if (!f_rdpa_score_qstat_ring_destroy)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return BDMF_ERR_STATE;
    }
    return f_rdpa_score_qstat_ring_destroy(ring_size);
}
EXPORT_SYMBOL(rdpa_score_qstat_ring_destroy);

int (*f_rdpa_score_qstat_ring_info_get)(rdpa_qstat_data_t *score_qstat_info);
EXPORT_SYMBOL(f_rdpa_score_qstat_ring_info_get);
int rdpa_score_qstat_ring_info_get(rdpa_qstat_data_t *score_qstat_info)
{
    if (!f_rdpa_score_qstat_ring_info_get)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return BDMF_ERR_STATE;
    }
    return f_rdpa_score_qstat_ring_info_get(score_qstat_info);
}
EXPORT_SYMBOL(rdpa_score_qstat_ring_info_get);

int (*f_rdpa_score_bnl_ring_create)(uint32_t ring_size);
EXPORT_SYMBOL(f_rdpa_score_bnl_ring_create);
int rdpa_score_bnl_ring_create(uint32_t ring_size)
{
    if (!f_rdpa_score_bnl_ring_create)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return BDMF_ERR_STATE;
    }
    return f_rdpa_score_bnl_ring_create(ring_size);
}
EXPORT_SYMBOL(rdpa_score_bnl_ring_create);

int (*f_rdpa_score_bnl_ring_destroy)(uint32_t ring_size);
EXPORT_SYMBOL(f_rdpa_score_bnl_ring_destroy);
int rdpa_score_bnl_ring_destroy(uint32_t ring_size)
{
    if (!f_rdpa_score_bnl_ring_destroy)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return BDMF_ERR_STATE;
    }
    return f_rdpa_score_bnl_ring_destroy(ring_size);
}
EXPORT_SYMBOL(rdpa_score_bnl_ring_destroy);

int (*f_rdpa_score_bnl_ring_info_get)(rdpa_bnl_data_t *score_bnl_info);
EXPORT_SYMBOL(f_rdpa_score_bnl_ring_info_get);
int rdpa_score_bnl_ring_info_get(rdpa_bnl_data_t *score_bnl_info)
{
    if (!f_rdpa_score_bnl_ring_info_get)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return BDMF_ERR_STATE;
    }
    return f_rdpa_score_bnl_ring_info_get(score_bnl_info);
}
EXPORT_SYMBOL(rdpa_score_bnl_ring_info_get);

int (*f_rdpa_score_ts_1us_get)(uint32_t *ts_1us_p);
EXPORT_SYMBOL(f_rdpa_score_ts_1us_get);
int rdpa_score_ts_1us_get(uint32_t *ts_1us_p)
{
    if (!f_rdpa_score_ts_1us_get)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return BDMF_ERR_STATE;
    }
    return f_rdpa_score_ts_1us_get(ts_1us_p);
}
EXPORT_SYMBOL(rdpa_score_ts_1us_get);
#endif

#if defined(CONFIG_RUNNER_FPI)
int (*f_rdpa_fpi_ipid_cntr_alloc)(uint8_t *ipid_cntr_p);
EXPORT_SYMBOL(f_rdpa_fpi_ipid_cntr_alloc);
int rdpa_fpi_ipid_cntr_alloc(uint8_t *ipid_cntr_p)
{
    if (!f_rdpa_fpi_ipid_cntr_alloc)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return BDMF_ERR_STATE;
    }
    return f_rdpa_fpi_ipid_cntr_alloc(ipid_cntr_p);
}
EXPORT_SYMBOL(rdpa_fpi_ipid_cntr_alloc);

void (*f_rdpa_fpi_ipid_cntr_free)(uint8_t ipid_cntr);
EXPORT_SYMBOL(f_rdpa_fpi_ipid_cntr_free);
void rdpa_fpi_ipid_cntr_free(uint8_t ipid_cntr)
{
    if (!f_rdpa_fpi_ipid_cntr_free)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return;
    }
    f_rdpa_fpi_ipid_cntr_free(ipid_cntr);
}
EXPORT_SYMBOL(rdpa_fpi_ipid_cntr_free);
#endif

int (*f_rdpa_port_get_hw_queue_info)(bdmf_object_handle port_obj, uint8_t wan_flow, uint16_t queue_id,
                                   bdmf_object_handle *egress_tm_obj, uint8_t *queue_index, uint16_t *hw_queue_id);
EXPORT_SYMBOL(f_rdpa_port_get_hw_queue_info);
int rdpa_port_get_hw_queue_info(bdmf_object_handle port_obj, uint8_t wan_flow, uint16_t queue_id,
                              bdmf_object_handle *p_egress_tm_obj, uint8_t *queue_index, uint16_t *hw_queue_id)
{
    if (!f_rdpa_port_get_hw_queue_info)
    {
        BDMF_TRACE_ERR("rdpa.ko is not loaded\n");
        return BDMF_ERR_STATE;
    }
    return f_rdpa_port_get_hw_queue_info(port_obj, wan_flow, queue_id, p_egress_tm_obj, queue_index, hw_queue_id);
}
EXPORT_SYMBOL(rdpa_port_get_hw_queue_info);


#define MAX_IRQ_NUM  64
#define NAME_LENGTH  32
static struct {
    int num;
    struct {
        char name[NAME_LENGTH];
        int value;
    }irq[MAX_IRQ_NUM];
}bcm_rdpa_irq = {0};

/** Get rdpa Linux IRQ number.

 * This function returns rdpa linux IRQ number.
 * \param[in] node_name: node name in DTS
 * \param[in] irq_name: IRQ name
 * \return Linux IRQ number
 */
int bcm_rdpa_irq_get_byname(const char *node_name, const char *irq_name)
{
    int i;
    char tmp[NAME_LENGTH];

    if (!node_name || !irq_name)
    {
        printk(KERN_ERR "IRQ get failed\n");
        return 0;
    }

    snprintf(tmp, NAME_LENGTH, "%s,%s", node_name, irq_name);
    for (i = 0; i < bcm_rdpa_irq.num; i++)
    {
        if (!strcmp(tmp, bcm_rdpa_irq.irq[i].name))
        {
            return bcm_rdpa_irq.irq[i].value;
        }
    }
    printk(KERN_ERR "IRQ not defined, node: %s, name: %s\n", node_name, irq_name);

    return 0;
}
EXPORT_SYMBOL(bcm_rdpa_irq_get_byname);

static int rdpa_gpl_irq_probe(struct platform_device *pdev)
{
    int i;
    int irq;
    struct resource res;
    struct device_node *np;

    if (!pdev || !pdev->dev.of_node)
    {
        return -EINVAL;
    }

    np = pdev->dev.of_node;
    for (i = 0; i < MAX_IRQ_NUM; i++)
    {
        irq = of_irq_to_resource(np, i, &res);
        if (irq < 0) break;

        if ((bcm_rdpa_irq.num + 1) > MAX_IRQ_NUM)
        {
            printk(KERN_ERR "IRQ number out of bound, max:%d\n", MAX_IRQ_NUM);
            return -EINVAL;
        }

        snprintf(bcm_rdpa_irq.irq[bcm_rdpa_irq.num].name, NAME_LENGTH, "%s,%s", np->name, res.name);
        bcm_rdpa_irq.irq[bcm_rdpa_irq.num].value = irq;
        bcm_rdpa_irq.num++;
    }

    return 0;
}
static const struct of_device_id rdpa_gpl_irq_of_match[] = {
    { .compatible = "brcm,ngpon-drv", .data = NULL, },
    { .compatible = "brcm,gpon-drv", .data = NULL, },
    { .compatible = "brcm,rdpa", .data = NULL, },
    {},
};

MODULE_DEVICE_TABLE(of, rdpa_gpl_irq_of_match);

static struct platform_driver rdpa_driver = {
    .probe = rdpa_gpl_irq_probe,
    .driver = {
        .name = "bcm-rdpa-gpl-irq",
        .of_match_table = rdpa_gpl_irq_of_match,
    },
};

static int __init bcm_rdpa_irq_reg(void)
{
    return platform_driver_register(&rdpa_driver);
}

subsys_initcall(bcm_rdpa_irq_reg);

