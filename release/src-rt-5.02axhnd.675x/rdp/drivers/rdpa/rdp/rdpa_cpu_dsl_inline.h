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
 * rdpa_cpu_dsl_inline.h
 *
 *  Created on: Nov 21, 2012
 */
#ifndef _RDPA_CPU_DSL_INLINE_H_
#define _RDPA_CPU_DSL_INLINE_H_

#if defined(BCM_DSL_RDP)
/* #define CC_CPU_DATAPATH_DEBUG */

#if defined(CC_CPU_DATAPATH_DEBUG)
#define CPU_DATAPATH_DUMP_SYSB(sysb, info)
#define CPU_DATAPATH_DUMP_SYSB_DSL(sysb, port, queue) \
    do { \
        if (unlikely(cpu_object_data->tx_dump.enable)) \
        {\
            bdmf_session_print(NULL, \
                "Tx : port %s queue_id:%u length: %d, ptr: %p, time: %u\n",\
                bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port),\
                (int)queue, (int)bdmf_sysb_length(sysb), bdmf_sysb_data(sysb),\
                (int) (jiffies / HZ));\
            bdmf_session_hexdump(NULL, bdmf_sysb_data(sysb), 0, bdmf_sysb_length(sysb));\
        } \
    } while (0)
#define CPU_DATAPATH_BUG_ON(condition) BUG_ON(condition)
#define CPU_DATAPATH_DUMP_RDD_RC(type, rdd_rc) \
    CPU_CHECK_DUMP_RDD_RC(type, rdd_rc)
#define CPU_DATAPATH_DUMP_RDD_MAP(wan_flow, queue_id, rc)               \
    do {                                                                \
        if (cpu_object_data->tx_dump.enable)                            \
            BDMF_TRACE_ERR("can't map US flow %u, queue %u to RDD. rc=%d\n", \
                           (unsigned)wan_flow, (unsigned)queue_id, rc); \
    } while (0)
#else
#define CPU_DATAPATH_DUMP_SYSB(sysb, info)
#define CPU_DATAPATH_DUMP_SYSB_DSL(sysb, port, queue)
#define CPU_DATAPATH_BUG_ON(condition)
#define CPU_DATAPATH_DUMP_RDD_RC(type, rdd_rc)
#define CPU_DATAPATH_DUMP_RDD_MAP(wan_flow, queue_id, rc)
#endif

/** Send system buffer to Ethernet WAN/ xDSL WAN Interface
 *
 * \param[in] sysb System buffer. Released regardless on the function outcome
 * \param[in] egress_queue Ethernet Egress Queue
 * \return 0=OK or int error code\n
 */
int rdpa_cpu_tx_port_enet_or_dsl_wan(bdmf_sysb sysb, uint32_t egress_queue, rdpa_flow wan_flow, rdpa_if wan_if,
                                     rdpa_cpu_tx_extra_info_t extra_info)
{
    uint32_t length;
    int rdd_rc, channel = 0, rc_id, priority = 0, tc_id;

    CPU_DATAPATH_BUG_ON(!cpu_object_data);
    CPU_DATAPATH_DUMP_SYSB_DSL(sysb, 0, egress_queue);

    length = bdmf_sysb_length(sysb);

    /* flush dcache before passing pointer to RDD */
#if defined(CONFIG_BCM_PKTRUNNER_GSO) && defined(CONFIG_RUNNER_GSO)
    bdmf_sysb_data_flush(sysb, bdmf_sysb_data(sysb), bdmf_sysb_data_length(sysb));
#else
    bdmf_sysb_data_flush(sysb, bdmf_sysb_data(sysb), length);
#endif

    /* Engress port tx condition from CPU ENET/XTM component(s). Port/Priority are
    ** specified explicitly.
    */
    /* Try to send from absolute address and copy to new pbuf if failed */

    /* Map wan_flow + queue_id to channel, rc_id, priority */
    /* tc check will modify the 6bit tc value to 1 bit is_wred_high_prio */
    tc_id = extra_info.tc_id;
    rdd_rc = _rdpa_egress_tm_wan_flow_queue_to_rdd(wan_if, wan_flow, egress_queue, &channel, &rc_id, &priority, &tc_id);
    if (rdd_rc)
    {
        ++cpu_object_data->tx_stat.tx_invalid_queue;
        bdmf_sysb_free(sysb);
        CPU_DATAPATH_DUMP_RDD_MAP(wan_flow, egress_queue, rdd_rc);
        return rdd_rc;
    }
    extra_info.tc_id = tc_id;

    rdd_rc = rdd_cpu_tx_write_gpon_packet_from_abs_address(sysb, length, wan_flow,
                                                           channel, rc_id, priority, extra_info);
    /* NOTE : Pay attention to the error handling and skb_free code below */

    CPU_DATAPATH_DUMP_RDD_RC("sysb", rdd_rc);

    if (likely(rdd_rc == BL_LILAC_RDD_OK))
    {
        ++cpu_object_data->tx_stat.tx_ok;
    }
    else
    {
        bdmf_sysb_free(sysb);

        ++cpu_object_data->tx_stat.tx_rdd_error;

        if ((cpu_object_data->tx_stat.tx_rdd_error & 0x3FF) == 1) /* print only every 1024 errors */
        {
            BDMF_TRACE_ERR("%s failed. rdd_rc=%d tx_rdd_error_count=%d\n", __FUNCTION__, (int)rdd_rc,
                    cpu_object_data->tx_stat.tx_rdd_error);
        }
    }

    return rdd_rc;
}
#ifndef BDMF_DRIVER_GPL_LAYER
EXPORT_SYMBOL(rdpa_cpu_tx_port_enet_or_dsl_wan);
#endif

/** Send system buffer to Ethernet LAN Interface
 *
 * \param[in] sysb System buffer. Released regardless on the function outcome
 * \param[in] egress_queue Ethernet Egress Queue
 * \param[in] phys_port Ethernet LAN physical port
 * \return 0=OK or int error code\n
 */
int rdpa_cpu_tx_port_enet_lan(bdmf_sysb sysb, uint32_t egress_queue,
                              uint32_t phys_port, rdpa_cpu_tx_extra_info_t extra_info)
{
    uint32_t length;
    int rdd_rc, rc_id, priority = 0, tc_id;
    rdd_emac_id_t rdd_emac;

    CPU_DATAPATH_BUG_ON(!cpu_object_data);
    CPU_DATAPATH_DUMP_SYSB_DSL(sysb, phys_port, egress_queue);

    length = bdmf_sysb_length(sysb);

    /* flush dcache before passing pointer to RDD */
#if defined(CONFIG_BCM_PKTRUNNER_GSO) && defined(CONFIG_RUNNER_GSO)
    bdmf_sysb_data_flush(sysb, bdmf_sysb_data(sysb), bdmf_sysb_data_length(sysb));
#else
    bdmf_sysb_data_flush(sysb, bdmf_sysb_data(sysb), length);
#endif


    rdd_emac = rdpa_lan_phys_port_to_rdd_lan_mac(phys_port);
    tc_id = extra_info.tc_id;
    rdd_rc = _rdpa_egress_tm_lan_port_queue_to_rdd_tc_check(rdpa_physical_port_to_rdpa_if(phys_port),
                                                            egress_queue, &rc_id, &priority, &tc_id);
    extra_info.tc_id = tc_id;

    rdd_rc = rdd_cpu_tx_write_eth_packet_from_abs_address(sysb, length,
                                                           rdd_emac, priority, extra_info);
    /* NOTE : Pay attention to the error handling and skb_free() code below */

    CPU_DATAPATH_DUMP_RDD_RC("sysb", rdd_rc);

    if (likely(rdd_rc == BL_LILAC_RDD_OK))
    {
        ++cpu_object_data->tx_stat.tx_ok;
    }
    else
    {
        bdmf_sysb_free(sysb);

        ++cpu_object_data->tx_stat.tx_rdd_error;

        if ((cpu_object_data->tx_stat.tx_rdd_error & 0x3FF) == 1) /* print only every 1024 errors */
        {
            BDMF_TRACE_ERR("%s failed. rdd_rc=%d tx_rdd_error_count=%d\n", __FUNCTION__, (int)rdd_rc,
                    cpu_object_data->tx_stat.tx_rdd_error);
        }
    }

    return rdd_rc;
}
#ifndef BDMF_DRIVER_GPL_LAYER
EXPORT_SYMBOL(rdpa_cpu_tx_port_enet_lan);
#endif

/** Send system buffer to Flow Cache Offload
 *
 * \param[in] sysb: System buffer. Released regardless on the function outcome
 * \param[in] cpu_rx_queue: CPU Rx Queue index, in case of Runner Flow miss
 * \param[in] dirty: Indicates whether a packet flush from D$ is required
 * \return 0=OK or int error code\n
 */
int rdpa_cpu_tx_flow_cache_offload(bdmf_sysb sysb, uint32_t cpu_rx_queue, int dirty)
{
    uint32_t length;
    int rdd_rc;

    CPU_DATAPATH_BUG_ON(!cpu_object_data);
    CPU_DATAPATH_DUMP_SYSB(sysb, info);

    /* flush dcache before passing pointer to RDD */
    length = bdmf_sysb_length(sysb);

    if (dirty)
    {
        bdmf_sysb_data_flush(sysb, bdmf_sysb_data(sysb), length);
    }

    rdd_rc = rdd_cpu_tx_write_offload_packet_from_abs_address(sysb, length, cpu_rx_queue);

    CPU_DATAPATH_DUMP_RDD_RC("sysb", rdd_rc);

    if (rdd_rc)
    {
#ifndef BDMF_SYSTEM_SIM
        unsigned long flags;
        /* During failure - the free_skb is not called in above rdd_cpu_tx_write_() function */
        bdmf_fastlock_lock_irq(&int_lock_irq, flags);
        f_rdd_release_free_skb(RDD_CPU_TX_MAX_PENDING_FREE_INDEXES);
        bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
#endif
        bdmf_sysb_free(sysb);

        ++cpu_object_data->tx_stat.tx_rdd_error;

        if ((cpu_object_data->tx_stat.tx_rdd_error & 0x3FF) == 1) /* print only every 1024 errors */
        {
            BDMF_TRACE_ERR("%s failed. rdd_rc=%d tx_rdd_error_count=%d\n", __FUNCTION__, (int)rdd_rc,
                           cpu_object_data->tx_stat.tx_rdd_error);
        }

        return rdd_rc;
    }

    ++cpu_object_data->tx_stat.tx_ok;

    return 0;
}
#ifndef BDMF_DRIVER_GPL_LAYER
EXPORT_SYMBOL(rdpa_cpu_tx_flow_cache_offload);
#endif

#if defined(CONFIG_RUNNER_IPSEC)
/** Send system buffer to IPsec Offload
 *
 * \param[in] sysb: System buffer.
 * \param[in] dir: Indicates whether the packet is upstream or downstream.
 * \param[in] esphdr_offset: ESP header byte offset into the packet.
 * \param[in] sa_index: Entry index of the ddr sa descriptor table.
 * \param[in] sa_update: 0- sa_index entry of the ddr sa descriptor table is new.
 *                       1- sa_index entry of the ddr sa descriptor table has been updated..
 * \param[in] cpu_qid:  Runner - HostCPU queue id
 * \return 0=OK or int error code\n
 */
int rdpa_cpu_tx_ipsec_offload(bdmf_sysb sysb, rdpa_traffic_dir dir, uint8_t esphdr_offset,
                              uint8_t sa_index, uint8_t sa_update, uint8_t cpu_qid)
{
    int rdd_rc;

    CPU_DATAPATH_BUG_ON(!cpu_object_data);
    CPU_DATAPATH_DUMP_SYSB(sysb, info);

    rdd_rc = rdd_cpu_tx_ipsec_offload_from_abs_address(sysb, dir, esphdr_offset,
                                                       sa_index, sa_update, cpu_qid);

    CPU_DATAPATH_DUMP_RDD_RC("sysb", rdd_rc);

    if (rdd_rc)
    {
#ifndef BDMF_SYSTEM_SIM
        unsigned long flags;
        /* During failure - the free_skb is not called in above rdd_cpu_tx_write_() function */
        bdmf_fastlock_lock_irq(&int_lock_irq, flags);
        f_rdd_release_free_skb(RDD_CPU_TX_MAX_PENDING_FREE_INDEXES);
        bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
#endif

        ++cpu_object_data->tx_stat.tx_rdd_error;

        if ((cpu_object_data->tx_stat.tx_rdd_error & 0x3FF) == 1) /* print only every 1024 errors */
        {
            BDMF_TRACE_ERR("%s failed. rdd_rc=%d tx_rdd_error_count=%d\n", __FUNCTION__, (int)rdd_rc,
                           cpu_object_data->tx_stat.tx_rdd_error);
        }

        return rdd_rc;
    }

    ++cpu_object_data->tx_stat.tx_ok;

    return 0;
}
#ifndef BDMF_DRIVER_GPL_LAYER
EXPORT_SYMBOL(rdpa_cpu_tx_ipsec_offload);
#endif
#endif /* CONFIG_RUNNER_IPSEC */

/** Frees the given free index and returns a pointer to the associated
 *  System Buffer. It is up to the caller to process the System Buffer.
 *
 * \param[in] free_index: Runner free index
 * \return: Pointer to the associated system buffer\n
 */
bdmf_sysb rdpa_cpu_return_free_index(uint16_t free_index)
{
#ifndef BDMF_SYSTEM_SIM
    return rdd_cpu_return_free_index(free_index);
#else
    return (bdmf_sysb) 0;
#endif
}
#ifndef BDMF_DRIVER_GPL_LAYER
EXPORT_SYMBOL(rdpa_cpu_return_free_index);
#endif

void rdpa_cpu_tx_reclaim(void)
{
#ifndef BDMF_SYSTEM_SIM
    unsigned long flags;

    bdmf_fastlock_lock_irq(&int_lock_irq, flags);
    f_rdd_release_free_skb(0);
    bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
#endif
}
#ifndef BDMF_DRIVER_GPL_LAYER
EXPORT_SYMBOL(rdpa_cpu_tx_reclaim);
#endif

#endif /* defined(BCM_DSL_RDP) */

#endif /* _RDPA_CPU_DSL_INLINE_H_ */
