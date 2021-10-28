/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
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
