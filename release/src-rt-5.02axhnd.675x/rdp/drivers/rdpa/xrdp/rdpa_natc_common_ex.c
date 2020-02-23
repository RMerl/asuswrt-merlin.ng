/*
 * <:copyright-BRCM:2017:proprietary:standard
 * 
 *    Copyright (c) 2017 Broadcom 
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
#include "rdd_natc.h"
#include "rdd.h"
#include "rdpa_api.h"
#include "rdpa_l2_common.h"
#if defined(BCM_PON_XRDP)
#include "rdpa_l2_class_ex.h"
#endif
#include "rdpa_system_ex.h"
#include "rdpa_port_int.h"
#include "rdp_drv_natc.h"
#include "rdd_ag_natc.h"

extern natc_tbl_config_t g_natc_tbl_cfg[];

#define RDP_DSCP_IN_TOS_MASK 0xFC
#define RDP_ECN_IN_TOS_MASK 0x3

#define RDP_VLAN_TPID_MASK 0xFFFF0000
#define RDP_VLAN_VID_DEI_PBIT_MASK 0xFFFF
#define RDP_VLAN_DEI_MASK (1 << 12)

#define RDD_TRACE_L2_FLOW_KEY(title, sub_tbl_idx, src_mac, dst_mac, mac_crc, vtag_num, vtag0, vtag1, etype, tos, \
    ingress_if, vport, wan_flow) \
    RDD_TRACE("%s (%d) = { src_mac = %02x:%02x:%02x:%02x:%02x:%02x, dst_mac = %02x:%02x:%02x:%02x:%02x:%02x, "\
        "MAC CRC = 0x%x "\
        "vtag_num = %d, vtag0 = 0x%x, vtag1 = 0x%x, etype = 0x%x, tos = 0x%x, ingress_if = %d (vport = %d), " \
        "wan_flow = %d\n", \
        title, sub_tbl_idx, \
        src_mac.b[0], src_mac.b[1], src_mac.b[2], src_mac.b[3], src_mac.b[4], src_mac.b[5], \
        dst_mac.b[0], dst_mac.b[1], dst_mac.b[2], dst_mac.b[3], dst_mac.b[4], dst_mac.b[5], \
        mac_crc, vtag_num, vtag0, vtag1, etype, tos, ingress_if, vport, wan_flow)

static uint8_t l2_flow_tos_key_masked_get(uint8_t tos_key, rdpa_l2_flow_key_exclude_fields_t key_exclude_fields)
{
    if (key_exclude_fields & rdpa_l2_flow_key_exclude_ecn_field)
        tos_key &= ~RDP_ECN_IN_TOS_MASK;

    if (key_exclude_fields & rdpa_l2_flow_key_exclude_dscp_field)
        tos_key &= ~RDP_DSCP_IN_TOS_MASK;

    return tos_key;
}

static uint16_t l2_flow_vlan_key_masked_get(uint16_t vlan_key, rdpa_l2_flow_key_exclude_fields_t key_exclude_fields)
{
    if (key_exclude_fields & rdpa_l2_flow_key_exclude_dei_field)
        vlan_key &= ~RDP_VLAN_DEI_MASK;

    return vlan_key;
}

int rdd_l2_flow_key_var_size_ctx_compose(rdpa_l2_flow_key_exclude_fields_t key_exclude_fields, rdpa_l2_flow_key_t *l2_flow_key,
    uint8_t *connection_entry, uint8_t *connection_entry_no_size, rdd_ctx_size ctx_size)
{
    RDD_NAT_CACHE_L2_LKP_ENTRY_DTS lkp_entry = {};
    uint8_t macs_crc_buf[16] = {};
    uint16_t vlan_key_mask;

    RDD_BTRACE("l2_flow_key %p, connection_entry = %p\n", l2_flow_key, connection_entry);

    lkp_entry.sub_table_id = NATC_SUB_TBL_IDX_L2;
    lkp_entry.etype = l2_flow_key->eth_type;
#if defined(BCM_PON_XRDP)
    lkp_entry.vport = (uint8_t)rdpa_port_rdpa_if_to_vport(l2_flow_key->ingress_if);
#else
    lkp_entry.vport = l2_flow_key->lookup_port;
#endif
    lkp_entry.tcp_pure_ack = l2_flow_key->tcp_pure_ack;
    if (rdpa_if_is_wan(l2_flow_key->ingress_if))
    {
        if (rdpa_wan_if_to_wan_type(l2_flow_key->ingress_if) == rdpa_wan_gbe)
            lkp_entry.wan_flow = rdpa_port_rx_flow_src_port_get(l2_flow_key->ingress_if, 0);
        else
            lkp_entry.wan_flow = l2_flow_key->wan_flow;
    }

    lkp_entry.tos_union = l2_flow_tos_key_masked_get(l2_flow_key->tos, key_exclude_fields);
    lkp_entry.num_of_vlans = l2_flow_key->vtag_num;
    vlan_key_mask = l2_flow_vlan_key_masked_get(RDP_VLAN_VID_DEI_PBIT_MASK, key_exclude_fields);

    if (lkp_entry.num_of_vlans)
    {
        lkp_entry.vlan_0_union = l2_flow_key->vtag0 & vlan_key_mask;
        lkp_entry.tpid_vlan_0 = rdpa_system_tpid_idx_get((l2_flow_key->vtag0 & RDP_VLAN_TPID_MASK) >> 16);
    }
    if (lkp_entry.num_of_vlans > 1)
    {
        lkp_entry.vlan_1_union = l2_flow_key->vtag1 & vlan_key_mask;
        lkp_entry.tpid_vlan_1 = rdpa_system_tpid_idx_get((l2_flow_key->vtag1 & RDP_VLAN_TPID_MASK) >> 16);
    }

    RDD_BTRACE("VLAN0 = 0x%x, TPID0 = 0x%x\n", lkp_entry.vlan_0_union, lkp_entry.tpid_vlan_0);
    RDD_BTRACE("VLAN1 = 0x%x, TPID1 = 0x%x\n", lkp_entry.vlan_1_union, lkp_entry.tpid_vlan_1);

    /* Round each mac to 8 bytes (leave 2 bytes space before start of MAC) */
    memcpy(macs_crc_buf + 2, &l2_flow_key->src_mac, 6);
    RDD_BTRACE_BUF("CRC BUF After Src MAC", (uint8_t *)macs_crc_buf, 16);
    
    memcpy(macs_crc_buf + 10, &l2_flow_key->dst_mac, 6);
    RDD_BTRACE_BUF("CRC BUF After Dst MAC", (uint8_t *)macs_crc_buf, 16);

    lkp_entry.src_mac_dst_mac_crc = rdd_crc_buf_calc_crc32(macs_crc_buf, 16);

    lkp_entry.valid = 1;

    RDD_TRACE_L2_FLOW_KEY("l2_flow_key", lkp_entry.sub_table_id, l2_flow_key->src_mac, l2_flow_key->dst_mac,
        lkp_entry.src_mac_dst_mac_crc, l2_flow_key->vtag_num,
        l2_flow_key->vtag0, l2_flow_key->vtag1, l2_flow_key->eth_type, l2_flow_key->tos, l2_flow_key->ingress_if,
        lkp_entry.vport, l2_flow_key->wan_flow);

    if (ctx_size != CTX_SIZE_NOT_USED && connection_entry_no_size)
    {
        /* Move src_mac_dst_mac_crc[3:0] to another location and add variable length context size at that bit position. */
        RDD_NAT_CACHE_LKP_ENTRY_VAR_SIZE_CTX_DTS *lkp_entry_vsc = (RDD_NAT_CACHE_LKP_ENTRY_VAR_SIZE_CTX_DTS *)&lkp_entry;

        lkp_entry.src_mac_dst_mac_crc_3_0 = lkp_entry_vsc->var_size_ctx;
        lkp_entry_vsc->var_size_ctx = 0;
        memcpy(connection_entry_no_size, &lkp_entry, sizeof(RDD_NAT_CACHE_L2_LKP_ENTRY_DTS));
        lkp_entry_vsc->var_size_ctx = ctx_size;
        memcpy(connection_entry, &lkp_entry, sizeof(RDD_NAT_CACHE_L2_LKP_ENTRY_DTS));

#ifdef FIRMWARE_LITTLE_ENDIAN
        SWAPBYTES(connection_entry_no_size, sizeof(RDD_NAT_CACHE_L2_LKP_ENTRY_DTS));
        SWAPBYTES(connection_entry, sizeof(RDD_NAT_CACHE_L2_LKP_ENTRY_DTS));
#endif
    }
    else
    {
        memcpy(connection_entry, &lkp_entry, sizeof(RDD_NAT_CACHE_L2_LKP_ENTRY_DTS));
#ifdef FIRMWARE_LITTLE_ENDIAN
        SWAPBYTES(connection_entry, sizeof(RDD_NAT_CACHE_L2_LKP_ENTRY_DTS));
#endif
        RDD_BTRACE_BUF("L2 Flow Key buffer (connection_entry", connection_entry,
            (int)sizeof(RDD_NAT_CACHE_L2_LKP_ENTRY_DTS));
    }

    return 0;
}

int rdd_l2_flow_key_compose(rdpa_l2_flow_key_exclude_fields_t key_exclude_fields, rdpa_l2_flow_key_t *l2_flow_key,
    uint8_t *connection_entry)
{
    return rdd_l2_flow_key_var_size_ctx_compose(key_exclude_fields, l2_flow_key, connection_entry, NULL, CTX_SIZE_NOT_USED);
}

void rdd_l2_flow_key_decompose(rdpa_l2_flow_key_t *l2_flow_key, uint8_t *sub_tbl_id, uint8_t *key_buf)
{
    RDD_NAT_CACHE_L2_LKP_ENTRY_DTS lkp_entry = {};

    RDD_BTRACE("l2_flow_key %p, key_buf = %p\n", l2_flow_key, key_buf);

#ifdef FIRMWARE_LITTLE_ENDIAN
    SWAPBYTES(key_buf, sizeof(RDD_NAT_CACHE_L2_LKP_ENTRY_DTS));
#endif
    memcpy(&lkp_entry, key_buf, sizeof(RDD_NAT_CACHE_L2_LKP_ENTRY_DTS));

    if (sub_tbl_id)
        *sub_tbl_id = lkp_entry.sub_table_id;

    if (!l2_flow_key)
        return;

    l2_flow_key->tos = lkp_entry.tos_union;
    l2_flow_key->eth_type = lkp_entry.etype;
#if defined(BCM_PON_XRDP)
    l2_flow_key->ingress_if = rdpa_port_vport_to_rdpa_if(lkp_entry.vport);
#else
    l2_flow_key->lookup_port = lkp_entry.vport;
#endif
    l2_flow_key->tcp_pure_ack = lkp_entry.tcp_pure_ack;
    l2_flow_key->wan_flow = lkp_entry.wan_flow;

    l2_flow_key->vtag_num = lkp_entry.num_of_vlans;
    if (l2_flow_key->vtag_num)
        l2_flow_key->vtag0 = lkp_entry.vlan_0_union | (rdpa_system_tpid_get_by_idx(lkp_entry.tpid_vlan_0) << 16);
    if (l2_flow_key->vtag_num > 1)
        l2_flow_key->vtag1 = lkp_entry.vlan_1_union | (rdpa_system_tpid_get_by_idx(lkp_entry.tpid_vlan_1) << 16);

    RDD_TRACE_L2_FLOW_KEY("l2_flow_key", lkp_entry.sub_table_id,
        l2_flow_key->src_mac, l2_flow_key->dst_mac, lkp_entry.src_mac_dst_mac_crc,
        l2_flow_key->vtag_num,
        l2_flow_key->vtag0, l2_flow_key->vtag1, l2_flow_key->eth_type, l2_flow_key->tos, l2_flow_key->ingress_if,
        lkp_entry.vport,
        l2_flow_key->wan_flow);
}

int rdd_l2_flow_read_key(uint32_t natc_idx, uint8_t tbl_idx, rdpa_l2_flow_key_t *key)
{
    uint8_t *key_buf = NULL, sub_tbl_id;
    bdmf_boolean valid = 0;
    int rc = 0;

    RDD_BTRACE("natc_idx %d, tbl_idx = %d, key = %p\n", natc_idx, tbl_idx, key);

    key_buf = bdmf_alloc(g_natc_tbl_cfg[tbl_idx].key_len);
    if (!key_buf)
        return BDMF_ERR_NOMEM;

    rc = drv_natc_key_entry_get(tbl_idx, natc_idx, &valid, key_buf);
    if (rc)
        goto exit;

    rdd_l2_flow_key_decompose(key, &sub_tbl_id, key_buf);
    if (sub_tbl_id != NATC_SUB_TBL_IDX_L2)
    {
        rc = BDMF_ERR_NOENT;
        goto exit;
    }

    key->dir = rdd_natc_tbl_idx_to_dir(tbl_idx);

exit:
    bdmf_free(key_buf);
    return rc;
}

void rdd_l2_key_exclude_fields_set(rdpa_l2_flow_key_exclude_fields_t key_exclude_fields)
{
    uint8_t tos_key_mask = 0xFF;
    uint16_t vlan_key_mask = 0xFFFF;

    tos_key_mask = l2_flow_tos_key_masked_get(0xFF, key_exclude_fields);
    vlan_key_mask = l2_flow_vlan_key_masked_get(0xFFFF, key_exclude_fields);

    rdd_ag_natc_natc_l2_tos_mask_set(tos_key_mask);
    rdd_ag_natc_natc_l2_vlan_key_mask_set(vlan_key_mask);
}

