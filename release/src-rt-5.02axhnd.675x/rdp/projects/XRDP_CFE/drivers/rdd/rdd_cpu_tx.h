/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:

       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.

    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.

:>
*/

#ifndef _RDD_CPU_TX_H_
#define _RDD_CPU_TX_H_


typedef struct
{
    void *sysb;             /**< Buffer pointer */
    void *data;             /**< Buffer pointer */
    uint32_t fpm_bn;        /**< Buffer number */
    uint16_t offset;        /**< Buffer offset */
    uint16_t length;        /**< Buffer length */
    uint8_t abs_flag:1;       /**< ABS/FPM */
    uint8_t sbpm_copy:1;      /**< copy to SBPM/FPM */
    uint8_t fpm_fallback:1;   /**< if no SBPM copy to FPM */
    uint8_t reserve:5;
} pbuf_t;


typedef union
{
    uint32_t bn1_or_abs2_or_1588         :18  __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t data_1588                   :18  __PACKING_ATTRIBUTE_FIELD_LEVEL__;
#ifndef FIRMWARE_LITTLE_ENDIAN
    struct
    {
        uint32_t ssid                    :4  __PACKING_ATTRIBUTE_FIELD_LEVEL__;
        uint32_t fpm_fallback            :1  __PACKING_ATTRIBUTE_FIELD_LEVEL__;
        uint32_t sbpm_copy               :1  __PACKING_ATTRIBUTE_FIELD_LEVEL__;
        uint32_t bn1_or_abs2             :12 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    };
#else
    struct
    {
        uint32_t bn1_or_abs2             :12 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
        uint32_t sbpm_copy               :1  __PACKING_ATTRIBUTE_FIELD_LEVEL__;
        uint32_t fpm_fallback            :1  __PACKING_ATTRIBUTE_FIELD_LEVEL__;
        uint32_t ssid                    :4  __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    };
#endif
#ifndef FIRMWARE_LITTLE_ENDIAN
    struct
    {
        uint32_t lag_index               :2  __PACKING_ATTRIBUTE_FIELD_LEVEL__;
        uint32_t reserved2               :16 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    };
#else
    struct
    {
        uint32_t reserved2               :16 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
        uint32_t lag_index               :2  __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    };
#endif
} cpu_tx_bn1_or_abs2_or_1588;


typedef union
{
    uint8_t wan_flow_source_port        :8  __PACKING_ATTRIBUTE_FIELD_LEVEL__;
#ifndef FIRMWARE_LITTLE_ENDIAN
    struct
    {
        uint8_t is_vport                :1  __PACKING_ATTRIBUTE_FIELD_LEVEL__;
        uint8_t flow_or_port_id         :7  __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    };
#else
    struct
    {
        uint8_t flow_or_port_id         :7  __PACKING_ATTRIBUTE_FIELD_LEVEL__;
        uint8_t is_vport                :1  __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    };
#endif
} cpu_tx_wan_flow_source_port;

typedef union 
{
    uint32_t pkt_buf_ptr_low:32; /* 32 lsb of pointer to abs buffer */
#ifndef FIRMWARE_LITTLE_ENDIAN
    struct
    {
        uint32_t reserved0:3;
        uint32_t fpm_sop:11;      /* start of packet offset */
        uint32_t fpm_bn0:18;      /* fpm number */
    };
#else
    struct
    {
        uint32_t fpm_bn0:18;      /* fpm number */
        uint32_t fpm_sop:11;      /* start of packet offset */
        uint32_t reserved0:3;
    };
#endif
} pkt_buf_ptr_low_or_fpm_t;

typedef union 
{
    uint32_t sk_buf_ptr_low:32;
#ifndef FIRMWARE_LITTLE_ENDIAN
    struct
    {
        uint32_t data_1588:18;  /* reserved for 1588 data */
        uint32_t reserved1:14;
    };
#else
    struct
    {
        uint32_t data_1588:18;  /* reserved for 1588 data */
        uint32_t reserved1:14;
    };
#endif
} sk_buf_ptr_low_or_data_1588_t;

/*igor :todo add compilation flag #define CPU_TX_BY_RING */
#if !defined(CPU_TX_BY_RING)
static inline void rdd_cpu_tx_set_packet_descriptor(const rdpa_cpu_tx_info_t *info, pbuf_t *pbuf, RDD_CPU_TX_DESCRIPTOR_DTS *cpu_tx_descriptor)
{
    uintptr_t data_phys_addr = RDD_VIRT_TO_PHYS(pbuf->data);
    uintptr_t sysb_phys_addr = RDD_VIRT_TO_PHYS(pbuf->sysb);
    cpu_tx_bn1_or_abs2_or_1588 bn1_or_abs2_or_1588;
    bn1_or_abs2_or_1588.bn1_or_abs2_or_1588 = cpu_tx_descriptor->bn1_or_abs2_or_1588;
    
    cpu_tx_descriptor->abs = pbuf->abs_flag;
    bn1_or_abs2_or_1588.fpm_fallback = pbuf->fpm_fallback;
    bn1_or_abs2_or_1588.sbpm_copy = pbuf->sbpm_copy;
    cpu_tx_descriptor->packet_length = pbuf->length;
    cpu_tx_descriptor->valid = 1;
    cpu_tx_descriptor->agg_pd = 0;
#ifndef _CFE_    
    cpu_tx_descriptor->target_mem_1 = 1;
#endif    
    cpu_tx_descriptor->target_mem_0 = 0;
    if (!cpu_tx_descriptor->abs)
    {
        cpu_tx_descriptor->payload_offset_or_abs_1 = pbuf->offset;
        cpu_tx_descriptor->buffer_number_0_or_abs_0 = pbuf->fpm_bn;

#if defined(CONFIG_BCM_PTP_1588)
        if (unlikely(info->ptp_info))
        {
            cpu_tx_descriptor->flag_1588 = 1;
            bn1_or_abs2_or_1588.data_1588 = info->ptp_info; /*only 18 bits will be copied */
        }
#endif        
    }
    else
    {
        cpu_tx_descriptor->abs_data0 = (data_phys_addr) & 0x3FFFF;
        cpu_tx_descriptor->abs_data1 = ((data_phys_addr) >> 18) & 0x3FFFFF;
        cpu_tx_descriptor->buffer_number_0_or_abs_0 = (sysb_phys_addr) & 0x3FFFF;
        cpu_tx_descriptor->payload_offset_or_abs_1 = ((sysb_phys_addr) >> 18) & 0x7FF; 
        bn1_or_abs2_or_1588.bn1_or_abs2 = ((sysb_phys_addr) >> 29) & 0xFFF;
    }

    cpu_tx_descriptor->bn1_or_abs2_or_1588 = bn1_or_abs2_or_1588.bn1_or_abs2_or_1588;
}
#else
static inline void rdd_cpu_tx_set_ring_descriptor(const rdpa_cpu_tx_info_t *info, pbuf_t *pbuf, RDD_RING_CPU_TX_DESCRIPTOR_DTS *ring_cpu_tx_descriptor)
{
    uintptr_t data_phys_addr = RDD_VIRT_TO_PHYS(pbuf->data);
    uintptr_t sysb_phys_addr = RDD_VIRT_TO_PHYS(pbuf->sysb);

    ring_cpu_tx_descriptor->abs = pbuf->abs_flag;
    ring_cpu_tx_descriptor->fpm_fallback = pbuf->fpm_fallback;
    ring_cpu_tx_descriptor->sbpm_copy = pbuf->sbpm_copy;
    ring_cpu_tx_descriptor->packet_length = pbuf->length;

    ring_cpu_tx_descriptor->target_mem_0 = 0;
    if (!ring_cpu_tx_descriptor->abs)
    {
        pkt_buf_ptr_low_or_fpm_t pkt_buf_ptr_low_or_fpm;
        pkt_buf_ptr_low_or_fpm.fpm_bn0 = pbuf->fpm_bn;
        pkt_buf_ptr_low_or_fpm.fpm_sop = pbuf->offset;
        ring_cpu_tx_descriptor->pkt_buf_ptr_low_or_fpm_bn0 = pkt_buf_ptr_low_or_fpm.pkt_buf_ptr_low;

#if defined(CONFIG_BCM_PTP_1588)
        /*igor :todo add data_1588 to ring descriptor*/
        if (unlikely(info->ptp_info))
        {
            sk_buf_ptr_low_or_data_1588_t sk_buf_ptr_low_or_data_1588 = {};
            ring_cpu_tx_descriptor->flag_1588 = 1;
            sk_buf_ptr_low_or_data_1588.data_1588 = info->ptp_info; /*only 18 bits will be copied */
            ring_cpu_tx_descriptor->sk_buf_ptr_low_or_data_1588 = sk_buf_ptr_low_or_data_1588.sk_buf_ptr_low;
        }
#endif        
    }
    else
    {
        GET_ADDR_HIGH_LOW(ring_cpu_tx_descriptor->pkt_buf_ptr_high, ring_cpu_tx_descriptor->pkt_buf_ptr_low_or_fpm_bn0, data_phys_addr);
        GET_ADDR_HIGH_LOW(ring_cpu_tx_descriptor->sk_buf_ptr_high, ring_cpu_tx_descriptor->sk_buf_ptr_low_or_data_1588, sysb_phys_addr);
    }
}
#endif /* CPU_TX_BY_RING */
#endif /* _RDD_CPU_TX_H_ */
