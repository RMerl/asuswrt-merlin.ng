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


#ifdef _CFE_
#include "lib_types.h"
#include "lib_string.h"
#endif
#include "rdd_tuple_lkp.h"
#include "rdd_ag_natc.h"

int rdd_nat_cache_init(const rdd_module_t *module)
{
    RDD_NAT_CACHE_CFG_RES_OFFSET_WRITE_G(module->res_offset, module->cfg_ptr, 0);
    RDD_NAT_CACHE_CFG_CONTEXT_OFFSET_WRITE_G(module->context_offset, module->cfg_ptr, 0);
    RDD_NAT_CACHE_CFG_KEY_OFFSET_WRITE_G(((natc_params_t *)module->params)->connection_key_offset, module->cfg_ptr, 0);

    return BDMF_ERR_OK;
}

#define RDD_TRACE_IPV4_FLOW_KEY(title, src_ip, dst_ip, src_port, dst_port, proto, tcp_pure_ack, is_ctx_ext) do { \
    RDD_TRACE("%s = { src_ip = %u.%u.%u.%u, dst_ip = %u.%u.%u.%u, prot = %d, src_port = %d, dst_port = %d, tcp_pure_ack = %d, is_ctx_ext = %d\n", \
        title, (uint8_t)(src_ip >> 24) & 0xff, \
        (uint8_t)(src_ip >> 16) & 0xff, \
        (uint8_t)(src_ip >> 8) & 0xff, \
        (uint8_t)(src_ip) & 0xff, \
        (uint8_t)(dst_ip >> 24) & 0xff, \
        (uint8_t)(dst_ip >> 16) & 0xff, \
        (uint8_t)(dst_ip >> 8) & 0xff, \
        (uint8_t)(dst_ip) & 0xff, \
        proto, src_port, dst_port, tcp_pure_ack, is_ctx_ext); \
} while (0)


void rdd_ip_class_key_entry_decompose(rdpa_ip_flow_key_t *tuple_entry, uint8_t *sub_tbl_id, uint8_t *tuple_entry_ptr)
{
    RDD_NAT_CACHE_L3_LKP_ENTRY_DTS lkp_entry;

    RDD_BTRACE("tuple_entry %p, tuple_entry_ptr = %p\n", tuple_entry, tuple_entry_ptr);

     memcpy(&lkp_entry, tuple_entry_ptr, sizeof(RDD_NAT_CACHE_L3_LKP_ENTRY_DTS));
#ifdef FIRMWARE_LITTLE_ENDIAN
    SWAPBYTES(((uint8_t *)&lkp_entry), sizeof(RDD_NAT_CACHE_L3_LKP_ENTRY_DTS));
#endif

    RDD_TRACE_IPV4_FLOW_KEY("tuple_entry_ptr", lkp_entry.src_ip, lkp_entry.dst_ip, lkp_entry.src_port,
        lkp_entry.dst_port, lkp_entry.protocol, lkp_entry.tcp_pure_ack, lkp_entry.is_context_extension);

    if (tuple_entry)
    {
        if (lkp_entry.key_extend == bdmf_ip_family_ipv4)
        {
            tuple_entry->dst_ip.addr.ipv4 = lkp_entry.dst_ip;
#if defined(USE_NATC_VAR_CONTEXT_LEN)
            tuple_entry->dst_ip.addr.ipv4 = (tuple_entry->dst_ip.addr.ipv4 & ~0x0f) | lkp_entry.dst_ip_3_0;
#endif
            tuple_entry->src_ip.addr.ipv4 = lkp_entry.src_ip;
        }
        else
        {
            /* TODO - need to enhance and display also ipv6 addresses */
            memset(&tuple_entry->dst_ip.addr.ipv6, 0, 16);
            memset(&tuple_entry->src_ip.addr.ipv6, 0, 16);
        }
        tuple_entry->dst_port = lkp_entry.dst_port;
        tuple_entry->src_port = lkp_entry.src_port;
        tuple_entry->prot = lkp_entry.protocol;
        tuple_entry->dst_ip.family = lkp_entry.key_extend;
        tuple_entry->src_ip.family = lkp_entry.key_extend;
        tuple_entry->ingress_if = lkp_entry.vport;
        tuple_entry->tcp_pure_ack = lkp_entry.tcp_pure_ack;
        tuple_entry->is_ctx_ext = lkp_entry.is_context_extension;
    }
    if (sub_tbl_id)
        *sub_tbl_id = lkp_entry.sub_table_id;
}

int rdd_ip_class_key_entry_var_size_ctx_compose(rdpa_ip_flow_key_t *tuple_entry, uint8_t *connection_entry,
    uint8_t vport, uint8_t *connection_entry_no_size, rdd_ctx_size ctx_size)
{
    RDD_NAT_CACHE_L3_LKP_ENTRY_DTS lkp_entry = {};
    bdmf_error_t rc = BDMF_ERR_OK;
    uint32_t ipv6_src_ip_crc, ipv6_dst_ip_crc, key0_mask, *key0;

    RDD_BTRACE("tuple_entry %p, connection_entry = %p, vport = %d\n", tuple_entry, connection_entry, vport);
    RDD_TRACE_IPV4_FLOW_KEY("tuple_entry", tuple_entry->src_ip.addr.ipv4, tuple_entry->dst_ip.addr.ipv4,
        tuple_entry->src_port, tuple_entry->dst_port, tuple_entry->prot, tuple_entry->tcp_pure_ack, tuple_entry->is_ctx_ext);

    if (tuple_entry->dst_ip.family == bdmf_ip_family_ipv4)
    {
        lkp_entry.dst_ip = tuple_entry->dst_ip.addr.ipv4;
        lkp_entry.src_ip = tuple_entry->src_ip.addr.ipv4;
    }
    else
    {
        rdd_crc_ipv6_addr_calc(&tuple_entry->src_ip, &ipv6_src_ip_crc);
        rdd_crc_ipv6_addr_calc(&tuple_entry->dst_ip, &ipv6_dst_ip_crc);
        lkp_entry.dst_ip = ipv6_dst_ip_crc;
        lkp_entry.src_ip = ipv6_src_ip_crc;
    }

    lkp_entry.dst_port = tuple_entry->dst_port;
    lkp_entry.src_port = tuple_entry->src_port;
    lkp_entry.protocol = tuple_entry->prot;
    lkp_entry.key_extend = tuple_entry->dst_ip.family;
    lkp_entry.tcp_pure_ack = tuple_entry->tcp_pure_ack;
    lkp_entry.is_context_extension = tuple_entry->is_ctx_ext;
    lkp_entry.valid = 1;

    lkp_entry.vport = vport;

    rdd_ag_natc_nat_cache_key0_mask_get(&key0_mask);
    key0 = (uint32_t *)&lkp_entry;
    *key0 &= key0_mask;

    if (ctx_size != CTX_SIZE_NOT_USED && connection_entry_no_size)
    {
        /* Move destination IP [3:0] to another location and add variable length context size at that bit position. */
        RDD_NAT_CACHE_LKP_ENTRY_VAR_SIZE_CTX_DTS *lkp_entry_vsc = (RDD_NAT_CACHE_LKP_ENTRY_VAR_SIZE_CTX_DTS *) &lkp_entry;

        lkp_entry.dst_ip_3_0 = lkp_entry_vsc->var_size_ctx; /* dst_ip bits [3:0] */
        lkp_entry_vsc->var_size_ctx = 0;
        memcpy(connection_entry_no_size, &lkp_entry, sizeof(RDD_NAT_CACHE_L3_LKP_ENTRY_DTS));
        lkp_entry_vsc->var_size_ctx = ctx_size;
        memcpy(connection_entry, &lkp_entry, sizeof(RDD_NAT_CACHE_L3_LKP_ENTRY_DTS));

#ifdef FIRMWARE_LITTLE_ENDIAN
        SWAPBYTES(connection_entry_no_size, sizeof(RDD_NAT_CACHE_L3_LKP_ENTRY_DTS));
        SWAPBYTES(connection_entry, sizeof(RDD_NAT_CACHE_L3_LKP_ENTRY_DTS));
#endif
    }
    else
    {
        memcpy(connection_entry, &lkp_entry, sizeof(RDD_NAT_CACHE_L3_LKP_ENTRY_DTS));
#ifdef FIRMWARE_LITTLE_ENDIAN
        SWAPBYTES(connection_entry, sizeof(RDD_NAT_CACHE_L3_LKP_ENTRY_DTS));
#endif
    }

    return rc;
}

int rdd_ip_class_key_entry_compose(rdpa_ip_flow_key_t *tuple_entry, uint8_t *connection_entry, uint8_t vport)
{
    return rdd_ip_class_key_entry_var_size_ctx_compose(tuple_entry, connection_entry, vport, NULL, CTX_SIZE_NOT_USED);
}

void rdd_3_tupples_ip_flows_enable(bdmf_boolean enable)
{
    RDD_NAT_CACHE_CFG_THREE_TUPLE_ENABLE_WRITE_G(enable, RDD_NAT_CACHE_CFG_ADDRESS_ARR, 0);
}

void rdd_esp_filter_set(rdd_action action)
{
    RDD_NAT_CACHE_CFG_ESP_FILTER_ENABLE_WRITE_G(1, RDD_NAT_CACHE_CFG_ADDRESS_ARR, 0);
    RDD_NAT_CACHE_CFG_ESP_FILTER_ACTION_WRITE_G((action == ACTION_FORWARD) ? 1 : 0, RDD_NAT_CACHE_CFG_ADDRESS_ARR, 0);
}

