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
        uint32_t fpm_bn0:18;      /* fpm number */
        uint32_t fpm_sop:11;      /* start of packet offset */
        uint32_t reserved0:3;
    };
#else
    struct
    {
        uint32_t reserved0:3;
        uint32_t fpm_sop:11;      /* start of packet offset */
        uint32_t fpm_bn0:18;      /* fpm number */
    };
#endif
} pkt_buf_ptr_low_or_fpm_t;

typedef union 
{
    uint32_t sk_buf_ptr_low:32;
#ifndef FIRMWARE_LITTLE_ENDIAN
    struct
    {
        uint32_t data_1558:18;  /* reserved for 1558 data */
        uint32_t reserved1:14;
    };
#else
    struct
    {
        uint32_t data_1558:18;  /* reserved for 1558 data */
        uint32_t reserved1:14;
    };
#endif
} sk_buf_ptr_low_or_data_1588_t;

#endif /* _RDD_CPU_TX_H_ */
