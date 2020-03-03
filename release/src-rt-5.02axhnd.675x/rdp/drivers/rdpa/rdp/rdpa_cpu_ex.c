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

#include "bdmf_dev.h"
#include "rdpa_api.h"
#include "rdpa_int.h"
#include "rdd.h"
#include "rdd_ih_defs.h"
#include "rdp_drv_bpm.h"
#include "rdd_common.h"
#include "rdd_cpu.h"
#include "rdpa_rdd_inline.h"
#include "rdpa_egress_tm_inline.h"
#include "rdp_cpu_ring.h"
#include "rdpa_platform.h"
#include "rdpa_cpu_ex.h"


#ifndef RUNNER_CPU_DQM_RX
static int cpu_irq_vector[rdpa_cpu_port__num_of] = {
    RDPA_RX_MAIN_INTERRUPT_NUM_IN_RDD,
    RDPA_PCI_MAIN_INTERRUPT_NUM_IN_RDD,
    RDPA_PCI_MAIN_INTERRUPT_NUM_IN_RDD,
};
#endif

extern int cpu_max_meters_per_dir[2];
extern rdpa_cpu_meter_cfg_t meter[2][RDPA_CPU_MAX_METERS];
extern rdpa_ports us_meter_ports[RDPA_CPU_PER_PORT_REASON][RDPA_CPU_MAX_US_METERS];

extern struct bdmf_object *cpu_object[rdpa_cpu_port__num_of];
static cpu_drv_priv_t *cpu_object_data;

/* cpu_port enum values */
bdmf_attr_enum_table_t cpu_port_enum_table = {
    .type_name = "rdpa_cpu_port",
    .values = {
        {"host",     rdpa_cpu_host},
        {"wlan0",    rdpa_cpu_wlan0},
        {"wlan1",    rdpa_cpu_wlan1},
        {NULL,      0}
    }
};

extern bdmf_attr_enum_table_t cpu_tx_method_enum_table;

#ifndef RUNNER_CPU_DQM_RX
static int _cpu_isr_wrapper(int irq, void *priv)
{
    cpu_drv_priv_t *cpu_data = priv;
    uint8_t sub_vector = 0;
    int active_bit;
    int queue_index;

    /* get the vector of pending sub interrupts  */
    rdd_interrupt_vector_get(cpu_irq_vector[cpu_data->index], &sub_vector);

    /* go over all CPU Rx queues and invoke the registered callback for each
     * queue which interrupt is  currently pending */
    while (sub_vector)
    {
        active_bit = ffs(sub_vector) - 1;

        /*prepare sub_vector for next iteration*/
        sub_vector &= ~(1<<active_bit);

        if (cpu_data->index == rdpa_cpu_host)
            queue_index = active_bit;
        else
            queue_index = active_bit + RDPA_CPU_MAX_QUEUES;

        ++cpu_data->rxq_stat[queue_index].interrupts;
        cpu_data->rxq_cfg[queue_index].rx_isr(
            cpu_data->rxq_cfg[queue_index].isr_priv);
    }
    bdmf_int_enable(irq);

    return BDMF_IRQ_HANDLED;
}
#endif

static rdpa_if _rdpa_cpu_port_to_rdpa_if(rdpa_cpu_port cpu_port)
{
    rdpa_if i = rdpa_if_none;

    switch (cpu_port)
    {
    case rdpa_cpu_host:
        i = rdpa_if_cpu;
        break;
    case rdpa_cpu_wlan0:
        i = rdpa_if_wlan0;
        break;
    case rdpa_cpu_wlan1:
        i = rdpa_if_wlan1;
        break;
    default:
        break;
    }
    return i;
}

int cpu_post_init_ex(struct bdmf_object *mo)
{
    cpu_drv_priv_t *cpu_data = (cpu_drv_priv_t *)bdmf_obj_data(mo);
    BDMF_MATTR(port_attrs, rdpa_port_drv());

    if (cpu_data->index == rdpa_cpu_host)
        cpu_object_data = cpu_data;

    /* Create port object */
    rdpa_port_index_set(port_attrs, _rdpa_cpu_port_to_rdpa_if(cpu_data->index));
    return bdmf_new_and_set(rdpa_port_drv(), mo, port_attrs, &cpu_data->port_obj);
}

void cpu_destroy_ex(struct bdmf_object *mo)
{
#ifndef RUNNER_CPU_DQM_RX
    int irq;
    cpu_drv_priv_t *cpu_data = (cpu_drv_priv_t *)bdmf_obj_data(mo);

    irq = cpu_data->index == rdpa_cpu_host ? RDPA_IC_CPU_RX_IRQ :
        cpu_data->index == rdpa_cpu_wlan0 ? RDPA_IC_WLAN0_IRQ : RDPA_IC_WLAN1_IRQ;
    bdmf_int_disconnect(irq, cpu_data);
#endif
}

rdpa_ports rdpa_ports_all_lan(void)
{
#if defined(BCM_DSL_RDP)
    return 0;
#else
    uint32_t ep = 0, i;

    for (i = RDD_EMAC_ID_START; i < RDD_EMAC_ID_COUNT; i++)
        ep |= 1 << i;

    return rdpa_rdd_egress_port_vector_to_ports(ep, 0);
#endif
}
#ifndef BDMF_DRIVER_GPL_LAYER
EXPORT_SYMBOL(rdpa_ports_all_lan);
#endif

/* "int_connect" attribute "write" callback */
int cpu_attr_int_connect_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
#ifndef RUNNER_CPU_DQM_RX
    cpu_drv_priv_t *cpu_data = (cpu_drv_priv_t *)bdmf_obj_data(mo);
    int irq = (cpu_data->index == rdpa_cpu_host) ? RDPA_IC_CPU_RX_IRQ :
        (cpu_data->index == rdpa_cpu_wlan0) ?  RDPA_IC_WLAN0_IRQ : RDPA_IC_WLAN1_IRQ;
    int rc;

    /* Connect IRQ */
    rc = bdmf_int_connect(irq, cpu_data->cpu_id, BDMF_IRQF_DISABLED,
        _cpu_isr_wrapper, bdmf_attr_get_enum_text_hlp(&cpu_port_enum_table,
        cpu_data->index), cpu_data);
    if (rc)
        return rc;
    bdmf_int_enable(irq);
#endif

    return 0;
}

/* "int_enabled" attribute "write" callback */
int cpu_attr_int_enabled_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    cpu_drv_priv_t *cpu_data = (cpu_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_boolean enable = *(bdmf_boolean *)val;
    if (enable)
        rdpa_cpu_int_enable(cpu_data->index, index);
    else
        rdpa_cpu_int_disable(cpu_data->index, index);
    return 0;
}

/* "int_enabled" attribute "read" callback */
int cpu_attr_int_enabled_read(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
#ifndef RUNNER_CPU_DQM_RX
    cpu_drv_priv_t *cpu_data = (cpu_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_boolean *enable = (bdmf_boolean *)val;
    uint8_t mask = 0;

    rdd_interrupt_mask_get(cpu_irq_vector[cpu_data->index], &mask);
    *enable = (mask & (1 << index)) != 0;
#endif
    return 0;
}

/** Enable CPU queue interrupt
 * \param[in]   port        port. rdpa_cpu, rdpa_wlan0, rdpa_wlan1
 * \param[in]   queue       Queue index < num_queues in port tm configuration
 */
void rdpa_cpu_int_enable(rdpa_cpu_port port, int queue)
{
#ifndef RUNNER_CPU_DQM_RX
    rdd_interrupt_unmask(cpu_irq_vector[port], queue);
#endif
}
#ifndef BDMF_DRIVER_GPL_LAYER
EXPORT_SYMBOL(rdpa_cpu_int_enable);
#endif

/** Disable CPU queue interrupt
 * \param[in]   port        port. rdpa_cpu, rdpa_wlan0, rdpa_wlan1
 * \param[in]   queue       Queue index < num_queues in port tm configuration
 */
void rdpa_cpu_int_disable(rdpa_cpu_port port, int queue)
{
#ifndef RUNNER_CPU_DQM_RX
    rdd_interrupt_mask(cpu_irq_vector[port], queue);
#endif
}
#ifndef BDMF_DRIVER_GPL_LAYER
EXPORT_SYMBOL(rdpa_cpu_int_disable);
#endif

/** Clear CPU queue interrupt
 * \param[in]   port        port. rdpa_cpu, rdpa_wlan0, rdpa_wlan1
 * \param[in]   queue       Queue index < num_queues in port tm configuration
 */
void rdpa_cpu_int_clear(rdpa_cpu_port port, int queue)
{
#ifndef RUNNER_CPU_DQM_RX
    rdd_interrupt_clear(cpu_irq_vector[port], queue);
#endif
}
#ifndef BDMF_DRIVER_GPL_LAYER
EXPORT_SYMBOL(rdpa_cpu_int_clear);
#endif

void rdpa_rnr_int_enable(uint8_t intr_idx)
{
}
#ifndef BDMF_DRIVER_GPL_LAYER
EXPORT_SYMBOL(rdpa_rnr_int_enable);
#endif

void rdpa_rnr_int_disable(uint8_t intr_idx)
{
}
#ifndef BDMF_DRIVER_GPL_LAYER
EXPORT_SYMBOL(rdpa_rnr_int_disable);
#endif

void rdpa_rnr_int_clear(uint8_t intr_idx)
{
}
#ifndef BDMF_DRIVER_GPL_LAYER
EXPORT_SYMBOL(rdpa_rnr_int_clear);
#endif

/* Map wan_flow + queue_id to channel, rc_id, priority */
#define CPU_MAP_US_INFO_TO_RDD(info, channel, rc_id, priority, buf, free_func) \
    do { \
        int rc;\
        rc = _rdpa_egress_tm_wan_flow_queue_to_rdd(info->port, info->x.wan.flow, \
            info->x.wan.queue_id, (int *)&channel, (int *)&rc_id, \
            (int *)&priority, NULL);\
        if (rc)\
        {\
            ++cpu_object_data->tx_stat.tx_invalid_queue;\
            free_func(buf);\
            if (cpu_object_data->tx_dump.enable) \
            {\
                BDMF_TRACE_ERR("can't map US flow %u, queue %u to RDD. rc=%d\n", \
                    (unsigned)info->x.wan.flow, (unsigned)info->x.wan.queue_id, rc);\
            } \
            return rc;\
        } \
    } while (0)

/* Map DS channel, queue to RDD */
#define CPU_MAP_DS_INFO_TO_RDD(info, rc_id, priority, buf, free_func) \
    do { \
        int rc; \
        if (rdpa_if_is_wifi(info->port)) \
        { \
            priority = 0; \
            rc_id = 0; \
            return 0; \
        } \
        rc = _rdpa_egress_tm_lan_port_queue_to_rdd(info->port, \
            info->x.lan.queue_id, (int *)&rc_id, (int *)&priority); \
        if (rc) \
        { \
            ++cpu_object_data->tx_stat.tx_invalid_queue; \
            free_func(buf); \
            BDMF_TRACE_ERR("can't map DS port/queue %u/%u P%u to RDD. rc=%d\n", \
                    (unsigned)info->port, (unsigned)info->x.lan.queue_id, priority, rc);\
            return rc; \
        } \
    } while (0)

static inline int rdpa_cpu_send_pbuf(bdmf_pbuf_t *pbuf,
    const rdpa_cpu_tx_info_t *info)
{
#ifndef RUNNER_CPU_DQM_TX
    int rdd_rc, rc_id;
    rdd_cpu_tx_args_t cpu_tx_args = {};

    cpu_tx_args.buffer_type = rdd_runner_buffer;

    /* Dump tx data for debugging */
    CPU_CHECK_DUMP_PBUF(pbuf, info);

    switch (info->method)
    {
    case rdpa_cpu_tx_port: /**< Egress port and priority are specified
                             explicitly. This is the most common mode */
    {
        cpu_tx_args.mode = rdd_cpu_tx_mode_egress_enq;
        cpu_tx_args.drop_precedence = info->drop_precedence;
        if (rdpa_if_is_wan(info->port))
        {
            /*upstream, egress enqueue, bpm*/
            cpu_tx_args.traffic_dir = rdpa_dir_us;
            cpu_tx_args.wan_flow = info->x.wan.flow;
            /* Map wan_flow + queue_id to channel, rc_id, priority */
            CPU_MAP_US_INFO_TO_RDD(info, (cpu_tx_args.direction.us.wan_channel),
                (cpu_tx_args.direction.us.rate_controller),
                (cpu_tx_args.direction.us.queue), pbuf, bdmf_pbuf_free);
#ifdef CONFIG_BCM_EPON_STACK_MODULE
            if (rdpa_is_epon_or_xepon_mode() || rdpa_is_epon_ae_mode())
                cpu_tx_args.wan_flow = cpu_tx_args.direction.us.wan_channel;
#endif
            rdd_rc = rdd_cpu_tx(&cpu_tx_args, (void *)pbuf, 0);
        }
        else
        {
            /*downstream, egress enqueue, bpm*/
            cpu_tx_args.traffic_dir = rdpa_dir_ds;
            if (rdpa_if_to_rdd_lan_mac(info->port,
                &(cpu_tx_args.direction.ds.emac_id), &(cpu_tx_args.wifi_ssid)))
            {
                bdmf_pbuf_free(pbuf);
                BDMF_TRACE_RET(BDMF_ERR_PARM, "Can't map LAN port %s\n",
                    bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table,
                    info->port));
            }
            if (rdpa_if_is_wifi(info->port))
            {
                bdmf_pbuf_free(pbuf);
                BDMF_TRACE_RET(BDMF_ERR_PARM, "Illegal destination port %s: CPU->WiFi should not pass through Runner\n",
                    bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, info->port));
            }
            CPU_MAP_DS_INFO_TO_RDD(info, rc_id,
                (cpu_tx_args.direction.ds.queue_id), pbuf, bdmf_pbuf_free);
            rdd_rc = rdd_cpu_tx(&cpu_tx_args, (void *)pbuf, 0);
        }
        break;
    }

    case rdpa_cpu_tx_bridge: /**< before bridge forwarding decision, before
                               classification */
    {
        cpu_tx_args.mode = rdd_cpu_tx_mode_full;
        if (rdpa_if_is_lan_or_wifi(info->port))
        {
            /*upstream, full, bpm*/
            cpu_tx_args.traffic_dir = rdpa_dir_us;
            cpu_tx_args.direction.us.src_bridge_port =
                rdpa_if_to_rdd_bridge_port(info->port,
                    &(cpu_tx_args.wifi_ssid));
            rdd_rc = rdd_cpu_tx(&cpu_tx_args, (void *)pbuf, 0);
        }
        else if (rdpa_if_is_wan(info->port))
        {
            /*downstream, full, bpm*/
            cpu_tx_args.traffic_dir = rdpa_dir_ds;
            cpu_tx_args.wan_flow = info->x.wan.flow;
            rdd_rc = rdd_cpu_tx(&cpu_tx_args, (void *)pbuf, 0);
        }
        else
        {
            bdmf_pbuf_free(pbuf);
            BDMF_TRACE_RET(BDMF_ERR_PARM, "Port %s is not WAN,LAN or SSIDn\n",
                bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, info->port));
        }
        break;
    }

    default:
        bdmf_pbuf_free(pbuf);
        BDMF_TRACE_RET(BDMF_ERR_NOT_SUPPORTED, "transmit method %d\n",
            (int)info->method);
    }
    CPU_CHECK_DUMP_RDD_RC("pbuf", rdd_rc);
    if (rdd_rc)
    {
        bdmf_pbuf_free(pbuf);
        ++cpu_object_data->tx_stat.tx_rdd_error;
        BDMF_TRACE_RET(BDMF_ERR_IO, "rdd error %d\n", (int)rdd_rc);
    }
    ++cpu_object_data->tx_stat.tx_ok;
#else
#endif
    return 0;
}

static inline uint16_t map_tx_info_to_source_bpm(const rdpa_cpu_tx_info_t *info)
{
    /* sending to queue should allocate BPM on SPARE0, sending to bridge
     * allocates per source port */
    if (info->method != rdpa_cpu_tx_bridge)
        return DRV_BPM_SP_SPARE_0;

    switch (info->port)
    {
        case rdpa_if_ssid0 ... rdpa_if_ssid15:
            return DRV_BPM_SP_PCI0;
        case rdpa_if_lan0 ... rdpa_if_lan_max:
            /* XXX: Check which source port counts when using LAG */
            return DRV_BPM_SP_EMAC0 + info->port - rdpa_if_lan0;
        case rdpa_if_wan0 ... rdpa_if_wan_max: 
            if (rdpa_is_gbe_mode())
                return DRV_BPM_SP_EMAC0 + rdpa_gbe_wan_emac();

            return DRV_BPM_SP_GPON;
        default:
            BDMF_TRACE_ERR("Cannot map port(%s) to BPM",
                bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, info->port));
            break;
    }
    return DRV_BPM_SP_SPARE_0;
}

/** Send raw packet
 *
 * \param[in]   data        Packet data
 * \param[in]   length      Packet length
 * \param[in]   info        Additional transmit info
 * \return 0=OK or int error code\n
 */
int rdpa_cpu_send_raw(void *data, uint32_t length,
    const rdpa_cpu_tx_info_t *info)
{
#ifndef RUNNER_CPU_DQM_TX
    bdmf_pbuf_t pbuf;
    int rc;
    const rdpa_system_cfg_t *system_cfg = _rdpa_system_cfg_get();

    BUG_ON(!cpu_object_data);
    BUG_ON(!info);

    if (length > system_cfg->mtu_size - 4)
        return BDMF_ERR_OVERFLOW;

    /* copy to bpm buffer */
    rc = bdmf_pbuf_alloc(data, length, map_tx_info_to_source_bpm(info), &pbuf);
    if (rc)
        return rc;

    return rdpa_cpu_send_pbuf(&pbuf, info);
#else
    return 0;
#endif
}
#ifndef BDMF_DRIVER_GPL_LAYER
EXPORT_SYMBOL(rdpa_cpu_send_raw);
#endif

/** Send system buffer
 *
 * \param[in] sysb System buffer. Released regardless on the function outcome
 * \param[in] info Tx info
 * \return 0=OK or int error code\n
 */

int rdpa_cpu_send_sysb(bdmf_sysb sysb, const rdpa_cpu_tx_info_t *info)
{
    uint32_t length;
    uint32_t flush_len = 0;
    int rdd_rc = 0;
    int rc_id;

    /*init cpu_tx_args*/
    rdd_cpu_tx_args_t cpu_tx_args = {};
    cpu_tx_args.buffer_type = rdd_host_buffer;

    BUG_ON(!cpu_object_data);
    BUG_ON(!info);

    length = bdmf_sysb_length(sysb);
    /* No pbuf info. Try to send from absolute address and copy to new pbuf if
     * failed */

    /* Dump tx data for debugging */
    CPU_CHECK_DUMP_SYSB(sysb, info);

#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
#if defined(BCM_DSL_RDP)
    cpu_tx_args.is_spdsvc_setup_packet = info->bits.is_spdsvc_setup_packet;
#else
    cpu_tx_args.direction.us.is_spdsrvc = info->bits.is_spdsvc_setup_packet;
#endif
#endif

    /* flush dcache before passing pointer to RDD */
#if defined(CONFIG_BCM_PKTRUNNER_GSO)
    flush_len = bdmf_sysb_data_length(sysb);
#else
    flush_len = length;
#endif

#if defined(CONFIG_BCM_PON)
    bdmf_sysb_inv_headroom_data_flush(sysb, bdmf_sysb_data(sysb), flush_len);
#else
    bdmf_sysb_data_flush(sysb, bdmf_sysb_data(sysb), flush_len);
#endif

    switch (info->method)
    {
    case rdpa_cpu_tx_port: /**< Egress port and priority are specified
                             explicitly. This is the most common mode */
    {
        cpu_tx_args.mode = rdd_cpu_tx_mode_egress_enq;
        cpu_tx_args.drop_precedence = info->drop_precedence;
        if (rdpa_if_is_wan(info->port))
        {
            /*upstream, egress enqueue, absolute*/
            cpu_tx_args.traffic_dir = rdpa_dir_us;
            cpu_tx_args.wan_flow = info->x.wan.flow;
            /* Map wan_flow + queue_id to channel, rc_id, priority */
            CPU_MAP_US_INFO_TO_RDD(info, (cpu_tx_args.direction.us.wan_channel),
                (cpu_tx_args.direction.us.rate_controller),
                (cpu_tx_args.direction.us.queue), sysb, bdmf_sysb_free);
#ifdef CONFIG_BCM_EPON_STACK_MODULE
            if (rdpa_is_epon_or_xepon_mode() || rdpa_is_epon_ae_mode())
                cpu_tx_args.wan_flow = cpu_tx_args.direction.us.wan_channel;
#endif
            rdd_rc = rdd_cpu_tx(&cpu_tx_args, (void *)sysb, length);
        }
        else
        {
            /*downstream, egress enqueue, absolute*/
            cpu_tx_args.traffic_dir = rdpa_dir_ds;
            if (rdpa_if_to_rdd_lan_mac(info->port,
                &(cpu_tx_args.direction.ds.emac_id), &(cpu_tx_args.wifi_ssid)))
            {
                bdmf_sysb_free(sysb);
                BDMF_TRACE_RET(BDMF_ERR_PARM, "Can't map LAN port %s\n",
                    bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table,
                    info->port));
            }
            if (rdpa_if_is_wifi(info->port))
            {
                bdmf_pbuf_free(sysb);
                BDMF_TRACE_RET(BDMF_ERR_PARM, "Illegal destination port %s: CPU->WiFi should not pass through Runner\n",
                    bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, info->port));
            }
            CPU_MAP_DS_INFO_TO_RDD(info, rc_id,
                (cpu_tx_args.direction.ds.queue_id), sysb, bdmf_sysb_free);
            rdd_rc = rdd_cpu_tx(&cpu_tx_args, (void *)sysb, length);
        }
        break;
    }

    case rdpa_cpu_tx_bridge: /**< before bridge forwarding decision, before
                               classification */
    {
        cpu_tx_args.mode = rdd_cpu_tx_mode_full;
        if (rdpa_if_is_lan_or_wifi(info->port))
        {
            /*upstream, full, absolute*/
            cpu_tx_args.traffic_dir = rdpa_dir_us;
            cpu_tx_args.direction.us.src_bridge_port =
                rdpa_if_to_rdd_bridge_port(info->port,
                    &(cpu_tx_args.wifi_ssid));
            rdd_rc = rdd_cpu_tx(&cpu_tx_args, (void *)sysb, length);
        }
        else if (rdpa_if_is_wan(info->port))
        {
            /*downstream, full, absolute*/
            cpu_tx_args.traffic_dir = rdpa_dir_ds;
            cpu_tx_args.wan_flow = info->x.wan.flow;
            rdd_rc = rdd_cpu_tx(&cpu_tx_args, (void *)sysb, length);
        }
        else
        {
            bdmf_sysb_free(sysb);
            BDMF_TRACE_RET(BDMF_ERR_PARM, "Port %s is not WAN,LAN or SSIDn\n",
                bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, info->port));
        }
        break;
    }

    default:
        bdmf_sysb_free(sysb);
        BDMF_TRACE_RET(BDMF_ERR_NOT_SUPPORTED, "transmit method %d\n",
            (int)info->method);
    }

    CPU_CHECK_DUMP_RDD_RC("sysb", rdd_rc);
    if (rdd_rc)
    {
        ++cpu_object_data->tx_stat.tx_no_buf;
        bdmf_sysb_free(sysb);
        BDMF_TRACE_ERR("rdpa_cpu_send_sysb failed. rdd_rc=%d\n", (int)rdd_rc);
        return rdd_rc;
    }
    ++cpu_object_data->tx_stat.tx_ok;

    return 0;
}
#ifndef BDMF_DRIVER_GPL_LAYER
EXPORT_SYMBOL(rdpa_cpu_send_sysb);
#endif

/** Send chained system buffer from WFD
 *
 * \param[in] sysb System buffer. Released regardless on the function outcome
 * \param[in] info Tx info
 * \return 0=OK or int error code\n
 */
int rdpa_cpu_send_wfd_to_bridge(bdmf_sysb sysb, const rdpa_cpu_tx_info_t *info, size_t offset_next)
{
    uint32_t length;
    rdd_cpu_tx_args_t cpu_tx_args = { };
    void *p_nbuff_next = NULL;
    int rc;
    int count = 0;

    /*init cpu_tx_args*/
    cpu_tx_args.buffer_type = rdd_host_buffer;
    cpu_tx_args.mode = rdd_cpu_tx_mode_full;

    /*upstream, full, absolute*/
    cpu_tx_args.traffic_dir = rdpa_dir_us;
    cpu_tx_args.direction.us.src_bridge_port = BL_LILAC_RDD_PCI_BRIDGE_PORT;
    cpu_tx_args.wifi_ssid = info->port - rdpa_if_ssid0;
    do
    {
        p_nbuff_next =  (void *)(*(struct sk_buff **)((uint8_t *)sysb + offset_next));
#if defined(CONFIG_BCM_PKTRUNNER_GSO)
        length = bdmf_sysb_data_length(sysb);
#else
        length = bdmf_sysb_length(sysb);
#endif
        bdmf_sysb_inv_headroom_data_flush(sysb, bdmf_sysb_data(sysb), 412);

        rc = rdd_cpu_tx(&cpu_tx_args, (void *)sysb, length);
        if (rc)
        {
            break;
        }
        count++;
        sysb = p_nbuff_next;
    } while (sysb);

    if (rc)
    {
        do
        {
            bdmf_sysb_free(sysb);
            sysb = p_nbuff_next;
            if (sysb)
                p_nbuff_next =  (void *)(*(struct sk_buff **)((uint8_t *)sysb + offset_next));
        } while (sysb);
    }
    return count;
}
#ifndef BDMF_DRIVER_GPL_LAYER
EXPORT_SYMBOL(rdpa_cpu_send_wfd_to_bridge);
#endif


/* Send system buffer ptp - similar to rdpa_cpu_send_sysb, but treats only
 * ptp-1588 packets */
int rdpa_cpu_send_sysb_ptp(bdmf_sysb sysb, const rdpa_cpu_tx_info_t *info)
{
#ifdef CONFIG_BCM_PTP_1588
    const rdpa_system_cfg_t *system_cfg = _rdpa_system_cfg_get();
    uint32_t length;
    int rdd_rc = 0;
    int rc_id;

    /*init cpu_tx_args*/
    rdd_cpu_tx_args_t cpu_tx_args = {};
    cpu_tx_args.traffic_dir = rdpa_dir_ds;
    cpu_tx_args.buffer_type = rdd_host_buffer;
    cpu_tx_args.mode = rdd_cpu_tx_mode_egress_enq;
    cpu_tx_args.direction.ds.en_1588 = 1;

    BUG_ON(!cpu_object_data);
    BUG_ON(!info);

    length = bdmf_sysb_length(sysb);

    if (length > system_cfg->mtu_size - 4)
    {
        bdmf_sysb_free(sysb);
        ++cpu_object_data->tx_stat.tx_too_long;
        return BDMF_ERR_OVERFLOW;
    }

    /* Dump tx data for debugging */
    CPU_CHECK_DUMP_SYSB(sysb, info);

    /* flush dcache before passing pointer to RDD */
#if defined(CONFIG_BCM_PKTRUNNER_GSO)
    bdmf_sysb_data_flush(sysb, bdmf_sysb_data(sysb), bdmf_sysb_data_length(sysb));
#else
    bdmf_sysb_data_flush(sysb, bdmf_sysb_data(sysb), length);
#endif

    if (rdpa_if_to_rdd_lan_mac(info->port, &cpu_tx_args.direction.ds.emac_id,
        &cpu_tx_args.wifi_ssid))
    {
        bdmf_sysb_free(sysb);
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Can't map LAN port %s\n",
            bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, info->port));
    }

    CPU_MAP_DS_INFO_TO_RDD(info, rc_id, cpu_tx_args.direction.ds.queue_id, sysb,
        bdmf_sysb_free);

    rdd_rc = rdd_cpu_tx(&cpu_tx_args, (void *)sysb, length);

    CPU_CHECK_DUMP_RDD_RC("sysbuf", rdd_rc);
    if (rdd_rc)
    {
        bdmf_sysb_free(&sysb);
        ++cpu_object_data->tx_stat.tx_rdd_error;
        BDMF_TRACE_RET(BDMF_ERR_IO, "rdd error %d\n", (int)rdd_rc);
    }

    ++cpu_object_data->tx_stat.tx_ok;
    return 0;
#else
    BDMF_TRACE_RET(BDMF_ERR_NOT_SUPPORTED, "ptp 1588 is not defined\n");
#endif /* CONFIG_BCM_PTP_1588 */
}
#ifndef BDMF_DRIVER_GPL_LAYER
EXPORT_SYMBOL(rdpa_cpu_send_sysb_ptp);
#endif

/** Send system buffer - Special function to send EPON Dying
 *  Gasp:
 *  1. reduce "if"
 *  2. use fastlock_irq
 *
 * \param[in] sysb System buffer. Released regardless on the function outcome
 * \param[in] info Tx info
 * \return 0=OK or int error code\n
 *
 * TBD: Move this function to rdpa_epon.c !
 *  */
int rdpa_cpu_send_epon_dying_gasp(bdmf_sysb sysb,
    const rdpa_cpu_tx_info_t *info)
{
#if !defined(BCM_DSL_RDP)
    uint32_t length = bdmf_sysb_length(sysb);
    int rdd_rc = 0;

    /*init cpu_tx_args*/
    rdd_cpu_tx_args_t cpu_tx_args = {};
    cpu_tx_args.traffic_dir =   rdpa_dir_us;
    cpu_tx_args.buffer_type =   rdd_host_buffer;
    cpu_tx_args.mode =          rdd_cpu_tx_mode_egress_enq;

    /* flush dcache before passing pointer to RDD */
    bdmf_dcache_flush((unsigned long)bdmf_sysb_data(sysb), length);

    /* Map wan_flow + queue_id to channel, rc_id, priority */
    CPU_MAP_US_INFO_TO_RDD(info, (cpu_tx_args.direction.us.wan_channel),
        (cpu_tx_args.direction.us.rate_controller),
        (cpu_tx_args.direction.us.queue), sysb, bdmf_sysb_free);

    rdd_rc = rdd_cpu_tx(&cpu_tx_args, (void *)sysb, length);

    CPU_CHECK_DUMP_RDD_RC("sysbuf", rdd_rc);
    if (rdd_rc)
    {
        bdmf_sysb_free(&sysb);
        ++cpu_object_data->tx_stat.tx_rdd_error;
        BDMF_TRACE_RET(BDMF_ERR_IO, "rdd error %d\n", (int)rdd_rc);
    }

    ++cpu_object_data->tx_stat.tx_ok;
#endif
    return 0;
}
#ifndef BDMF_DRIVER_GPL_LAYER
EXPORT_SYMBOL(rdpa_cpu_send_epon_dying_gasp);
#endif

#if defined(BCM_DSL_RDP)
#include "rdpa_cpu_dsl_inline.h"
#endif

/* Init/exit module. Cater for GPL layer */
#ifdef BDMF_DRIVER_GPL_LAYER
#if defined(BCM_DSL_RDP)
extern int (*f_rdpa_cpu_tx_port_enet_or_dsl_wan)(bdmf_sysb sysb,
    uint32_t egress_queue, rdpa_flow wan_flow, rdpa_if wan_if, rdpa_cpu_tx_extra_info_t extra_info);
extern int (*f_rdpa_cpu_tx_port_enet_lan)(bdmf_sysb sysb, uint32_t egress_queue,
    uint32_t phys_port, rdpa_cpu_tx_extra_info_t extra_info);
extern int (*f_rdpa_cpu_tx_flow_cache_offload)(bdmf_sysb sysb, uint32_t cpu_rx_queue,
    int dirty);
#if defined(CONFIG_RUNNER_IPSEC)
extern int (*f_rdpa_cpu_tx_ipsec_offload)(bdmf_sysb sysb, rdpa_traffic_dir dir, uint8_t esphdr_offset,
                                          uint8_t sa_index, uint8_t sa_update, uint8_t cpu_qid);
#endif
extern bdmf_sysb (*f_rdpa_cpu_return_free_index)(uint16_t free_index);
extern int (*f_rdpa_cpu_host_packet_get_enet)(bdmf_index queue, bdmf_sysb *sysb,
    rdpa_if *src_port);
extern void (*f_rdpa_cpu_tx_reclaim)(void);
#endif
#endif

int cpu_drv_init_ex(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
#if defined(BCM_DSL_RDP)
    f_rdpa_cpu_tx_port_enet_or_dsl_wan = rdpa_cpu_tx_port_enet_or_dsl_wan;
    f_rdpa_cpu_tx_port_enet_lan = rdpa_cpu_tx_port_enet_lan;
    f_rdpa_cpu_tx_flow_cache_offload = rdpa_cpu_tx_flow_cache_offload;
#if defined(CONFIG_RUNNER_IPSEC)
    f_rdpa_cpu_tx_ipsec_offload = rdpa_cpu_tx_ipsec_offload;
#endif
    f_rdpa_cpu_return_free_index = rdpa_cpu_return_free_index;
    f_rdpa_cpu_host_packet_get_enet = rdpa_cpu_host_packet_get_enet;
    f_rdpa_cpu_tx_reclaim = rdpa_cpu_tx_reclaim;
#endif
#endif
    return 0;
}

void cpu_drv_exit_ex(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
#if defined(BCM_DSL_RDP)
    f_rdpa_cpu_tx_port_enet_or_dsl_wan = NULL;
    f_rdpa_cpu_tx_port_enet_lan = NULL;
    f_rdpa_cpu_tx_flow_cache_offload = NULL;
#if defined(CONFIG_RUNNER_IPSEC)
    f_rdpa_cpu_tx_ipsec_offload = NULL;
#endif
    f_rdpa_cpu_return_free_index = NULL;
    f_rdpa_cpu_host_packet_get_enet = NULL;
    f_rdpa_cpu_tx_reclaim = NULL;
#endif
#endif
}

int cpu_attr_tc_to_rxq_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    void *val, uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

int cpu_attr_tc_to_rxq_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

int cpu_reason_cfg_validate_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    cpu_drv_priv_t *cpu_data = (cpu_drv_priv_t *)bdmf_obj_data(mo);
    const rdpa_cpu_reason_cfg_t *reason_cfg = (const rdpa_cpu_reason_cfg_t *)val;

    if (_check_queue_range(cpu_data, (unsigned)reason_cfg->queue) ||
        !cpu_data->rxq_cfg[reason_cfg->queue].size)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_NOENT, mo,
            "CPU queue %d is not configured\n", (int)reason_cfg->queue);
    }
    return 0;
}

int cpu_reason_cfg_rdd_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    rdpa_cpu_reason_index_t *rindex = (rdpa_cpu_reason_index_t *)index;
    const rdpa_cpu_reason_cfg_t *reason_cfg = (const rdpa_cpu_reason_cfg_t *)val;
    int rc;
    int rdd_rc;

#if defined(CONFIG_DSLWAN)
    rdd_rc = rdd_cpu_reason_to_cpu_rx_queue(rindex->reason,
        reason_cfg->queue, rindex->dir, rindex->table_index);
#else
    rdd_rc = rdd_cpu_reason_to_cpu_rx_queue(rindex->reason,
        reason_cfg->queue, rindex->dir);
#endif
    if (rindex->reason == rdpa_cpu_rx_reason_omci)
    {
        /* Configure reason to CPU queue. Need to configure for upstream
         * too, because IH doesn't parse packets smaller than 64, but FW
         * still use the "wan" bit from parser result. */
#if defined(CONFIG_DSLWAN)
        /* upstream will be handled by another call down using different table
            so don't need to create reverse direction entry here for enet impl7
        rdd_rc = rdd_rc ? rdd_rc :
            rdd_cpu_reason_to_cpu_rx_queue(rindex->reason,
                reason_cfg->queue, (rindex->dir == rdpa_dir_ds) ?
                rdpa_dir_us : rdpa_dir_ds, rindex->table_index);
*/
#else
        rdd_rc = rdd_rc ? rdd_rc :
            rdd_cpu_reason_to_cpu_rx_queue(rindex->reason,
                reason_cfg->queue, (rindex->dir == rdpa_dir_ds) ?
                rdpa_dir_us : rdpa_dir_ds);
#endif
    }
    BDMF_TRACE_DBG_OBJ(mo,
        "rdd_cpu_reason_to_cpu_rx_queue(%d, %d, %d) --> %d\n",
        (int)rindex->reason, (int)reason_cfg->queue, (int)rindex->dir,
        rdd_rc);
    if (rdd_rc)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo,
            "rdd_cpu_reason_to_cpu_rx_queue(%d, %d, %d) --> %d\n",
            (int)rindex->reason, (int)reason_cfg->queue, (int)rindex->dir,
            rdd_rc);
    }

    /* Configure meter */
    if (rindex->dir == rdpa_dir_us &&
        cpu_is_per_port_metering_supported(rindex->reason))
    {
        rc = cpu_per_port_reason_meter_cfg(mo, rindex, reason_cfg);
    }
    else
        rc = cpu_meter_cfg_rdd(mo, rindex, reason_cfg->meter, 0);
    return rc;
}

int rdpa_cpu_loopback_packet_get(rdpa_cpu_loopback_type loopback_type, bdmf_index queue, bdmf_sysb *sysb,
    rdpa_cpu_rx_info_t *info)
{
#if defined(BCM_DSL_RDP)

    bdmf_object_handle mo = cpu_object[rdpa_cpu_host];
    cpu_drv_priv_t *cpu_data;
    CPU_RX_PARAMS params;
    int rc;
    uint32_t context = 0;

    cpu_data = (cpu_drv_priv_t *)bdmf_obj_data(mo);

    if (_check_queue_range(cpu_data, (unsigned)queue))
        return BDMF_ERR_RANGE;

    memset((void *)&params, 0, sizeof(CPU_RX_PARAMS));
    rc = rdp_cpu_ring_read_packet_refill(queue, &params);
    if (rc)
    {
#ifdef LEGACY_RDP
        if (rc == BL_LILAC_RDD_ERROR_CPU_RX_QUEUE_EMPTY)
            return BDMF_ERR_NO_MORE;
#else
        if (rc == BDMF_ERR_NO_MORE)
            return rc;
#endif
        return BDMF_ERR_INTERNAL;
    }

    /* free the data buffer */
    bdmf_sysb_databuf_free(params.data_ptr, context);

    if (params.is_rx_offload)
    {
        rdpa_traffic_dir dir = params.is_ipsec_upstream ? rdpa_dir_us : rdpa_dir_ds;
        void *data;

        *sysb = rdpa_cpu_return_free_index(params.free_index);

        bdmf_sysb_length_set(*sysb, params.packet_size);

        if (bdmf_sysb_typeof(*sysb) == bdmf_sysb_skb)
        {
            struct sk_buff *skb = (struct sk_buff *)bdmf_sysb_2_fkb_or_skb(*sysb);

            skb->len = params.packet_size;
            skb_set_tail_pointer(skb, skb->len);
        }

        data = bdmf_sysb_data(*sysb);

#ifndef BDMF_SYSTEM_SIM
        cache_invalidate_len_outer_first(data, params.packet_size);
#endif

        info->data        = data;
        info->size        = params.packet_size;
        info->reason      = (rdpa_cpu_reason)params.reason;
        info->reason_data = params.flow_id;
        cpu_data->reason_stat[dir][info->reason]++;
        cpu_data->rxq_stat[queue].received++;

        if (unlikely(cpu_data->rxq_cfg[queue].dump))
            _dump_packet(mo->name, rdpa_cpu_host, queue, info, params.dst_ssid);
        return 0;
    }

    cpu_data->rxq_stat[queue].dropped++;

#endif /* defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) */

    return BDMF_ERR_PERM;
}

void _dump_packet(char *name, rdpa_cpu_port port, bdmf_index queue, rdpa_cpu_rx_info_t *info, uint32_t dst_ssid)
{
    bdmf_session_print(NULL, "Rx packet on %s: port %s queue %d, %d bytes ",
        name,
        bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, info->src_port),
        (int)queue, info->size);
    if (port == rdpa_cpu_host)
    {
        bdmf_session_print(NULL, "reason '%s'\n",
            bdmf_attr_get_enum_text_hlp(&rdpa_cpu_reason_enum_table,
            info->reason));
    }
    else
    {
        int ssid_bit;

        bdmf_session_print(NULL, "src_dst_ssid ");
        while (dst_ssid)
        {
            ssid_bit = ffs(dst_ssid);
            bdmf_session_print(NULL, "%s",
                bdmf_attr_get_enum_text_hlp(&rdpa_wlan_ssid_enum_table,
                ssid_bit - 1 + rdpa_wlan_ssid0));
            dst_ssid &= ~(1 << (ssid_bit - 1));
            if (dst_ssid)
                bdmf_session_print(NULL, "+");
        }
        bdmf_session_print(NULL, "\n");
    }
    bdmf_session_hexdump(NULL, (void *)((uint8_t *)info->data + info->data_offset), 0, info->size);
}

int rdpa_cpu_packet_get(rdpa_cpu_port port, bdmf_index queue, rdpa_cpu_rx_info_t *info)
{
    cpu_drv_priv_t *cpu_data;
    CPU_RX_PARAMS params;
    int rc;
    uint32_t context = 0;

    if ((unsigned)port >= rdpa_cpu_port__num_of || !cpu_object[port])
        return BDMF_ERR_PARM;

    cpu_data = (cpu_drv_priv_t *)bdmf_obj_data(cpu_object[port]);

    if (_check_queue_range(cpu_data, (unsigned)queue))
        return BDMF_ERR_RANGE;

    memset((void *)&params, 0, sizeof(CPU_RX_PARAMS));
    rc = rdp_cpu_ring_read_packet_refill(queue, &params);
    if (rc)
    {
#ifdef LEGACY_RDP
        if (rc == BL_LILAC_RDD_ERROR_CPU_RX_QUEUE_EMPTY)
            return BDMF_ERR_NO_MORE;
#else
        if (rc == BDMF_ERR_NO_MORE)
            return rc;
#endif
        return BDMF_ERR_INTERNAL;
    }

    if (port == rdpa_cpu_host)
    {
        rdpa_traffic_dir dir;

        info->reason = (rdpa_cpu_reason)params.reason;
        if (info->reason != rdpa_cpu_rx_reason_oam && info->reason != rdpa_cpu_rx_reason_omci)
        {
            info->src_port = rdpa_rdd_bridge_port_to_if(params.src_bridge_port,
                params.flow_id);
            if (info->src_port == rdpa_if_none)
            {
                cpu_data->rxq_stat[queue].dropped++;
                bdmf_sysb_databuf_free(params.data_ptr, context);
                return BDMF_ERR_PERM;
            }
        }
        info->reason_data = params.flow_id;
        info->ptp_index = params.ptp_index;
        info->data = (void *)params.data_ptr;
        info->size = params.packet_size;
        dir = rdpa_if_is_wan(info->src_port) ? rdpa_dir_ds : rdpa_dir_us;
        ++cpu_data->reason_stat[dir][info->reason];

        info->wl_metadata = params.wl_metadata;
    }
    else
    {
#if !defined(BCM_DSL_RDP) && !defined(XRDP)
        rdpa_ports elig_dst_ports;
        rdpa_ports dst_ports;
#endif
        info->src_port = rdpa_rdd_bridge_port_to_if(params.src_bridge_port,
            params.flow_id);
        info->wl_metadata = params.wl_metadata; /* wlan metadata */
        info->data = (void *)params.data_ptr;
        info->size = params.packet_size;

#if !defined(BCM_DSL_RDP) && !defined(XRDP)
        info->reason_data = (params.flow_id << 16) | params.dst_ssid;
        info->ptp_index = params.ptp_index;
        /* Check forward eligibility */
        elig_dst_ports = rdpa_bridge_fw_eligible(info->src_port);
        dst_ports = params.dst_ssid;
        dst_ports <<= rdpa_if_ssid0;
        if (!(elig_dst_ports & (RDPA_PORT_ALL_SSIDS | dst_ports)))
        {
            cpu_data->rxq_stat[queue].dropped++;
            bdmf_sysb_databuf_free(params.data_ptr, context);
            return BDMF_ERR_PERM;
        }
#endif
    }
    cpu_data->rxq_stat[queue].received++;

    if (unlikely(cpu_data->rxq_cfg[queue].dump))
        _dump_packet(cpu_object[port]->name, port, queue, info, params.dst_ssid);

    return 0;
}

int rdpa_cpu_packet_get_redirected(rdpa_cpu_port port, bdmf_index queue,
    rdpa_cpu_rx_info_t *info, rdpa_cpu_rx_ext_info_t *ext_info)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

int rdpa_cpu_packets_bulk_get(rdpa_cpu_port port, bdmf_index queue, rdpa_cpu_rx_info_t *info, int max_count, int *count)
{
    *count = 0;
    return BDMF_ERR_NOT_SUPPORTED;
}
#if !defined(BDMF_DRIVER_GPL_LAYER) && !defined(RUNNER_CPU_DQM_RX)
EXPORT_SYMBOL(rdpa_cpu_packets_bulk_get);
#endif

void cpu_rxq_cfg_params_init_ex(cpu_drv_priv_t *cpu_data, rdpa_cpu_rxq_cfg_t *rx_cfg, uint32_t *entry_size,
    uint32_t *init_write_idx)
{
    if (entry_size)
        *entry_size = sizeof(RDD_CPU_RX_DESCRIPTOR_DTS);
    if (init_write_idx)
        *init_write_idx = 0;
}

int _check_queue_range(cpu_drv_priv_t *cpu_data, uint32_t rqx_idx)
{
    if  (cpu_data->index == rdpa_cpu_host)
    {
        if (rqx_idx >= cpu_data->num_queues)
           return BDMF_ERR_PARM;
    }
    else if ((rqx_idx < RDPA_CPU_MAX_QUEUES) ||
        (rqx_idx > RDPA_CPU_MAX_QUEUES + cpu_data->num_queues))
    {
        return BDMF_ERR_PARM;
    }
    return 0;
}

void cpu_rxq_cfg_indecies_get(cpu_drv_priv_t *cpu_data, uint8_t *first_rqx_idx, uint8_t *last_rqx_idx)
{
    if (cpu_data->index == rdpa_cpu_host)
    {
        *first_rqx_idx = 0;
        *last_rqx_idx = RDPA_CPU_MAX_QUEUES - 1;
    }
    else
    {
        *first_rqx_idx = RDPA_CPU_MAX_QUEUES;
        *last_rqx_idx = RING_ID_NUM_OF - 1;
    }
}

int cpu_rxq_cfg_max_num_set(cpu_drv_priv_t *cpu_data)
{
    cpu_data->num_queues = (cpu_data->index == rdpa_cpu_host) ?  RDPA_CPU_MAX_QUEUES : RDPA_WLAN_MAX_QUEUES;
    return 0;
}

uint8_t cpu_rdd_rxq_idx_get(cpu_drv_priv_t *cpu_data, bdmf_index rxq_idx)
{
    return (uint8_t)rxq_idx;
}

int cpu_rxq_cfg_size_validate_ex(cpu_drv_priv_t *cpu_data, rdpa_cpu_rxq_cfg_t *rxq_cfg)
{
    int max_rxq_size = cpu_data->index == rdpa_cpu_host ? RDPA_CPU_QUEUE_MAX_SIZE : RDPA_CPU_WLAN_QUEUE_MAX_SIZE;

    /* Check te MAX queue size */
    if (rxq_cfg->size > max_rxq_size)
    {
        BDMF_TRACE_RET(BDMF_ERR_RANGE, "Queue size %u is too big\n"
            "Maximum allowed is %u\n", rxq_cfg->size, max_rxq_size);
    }
    return 0;
}

void rdpa_cpu_ring_read_idx_sync(rdpa_cpu_port port, bdmf_index queue)
{
}

void rdpa_cpu_rx_meter_clean_stat_ex(bdmf_index counter_id)
{
    /*not supported in RDP */
}
