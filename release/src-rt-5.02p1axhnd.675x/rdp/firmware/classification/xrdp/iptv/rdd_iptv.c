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


#include "rdd.h"
#include "rdd_platform.h"
#include "rdd_iptv.h"
#include "rdp_mm.h"

RDD_IPTV_DDR_CONTEXT_ENTRY_DTS *g_ctx_tbl_ptr = NULL;
rdpa_iptv_lookup_method g_iptv_method = iptv_lookup_method_group_ip;

#define L2_MCAST_METHOD() ((g_iptv_method == iptv_lookup_method_mac) || (g_iptv_method == iptv_lookup_method_mac_vid))
#define VID_IN_MCAST_METHOD() ((g_iptv_method == iptv_lookup_method_group_ip_src_ip_vid) || (g_iptv_method == iptv_lookup_method_mac_vid))
#define SRC_IP_IN_MCAST_METHOD() ((g_iptv_method == iptv_lookup_method_group_ip_src_ip) || (g_iptv_method == iptv_lookup_method_group_ip_src_ip_vid))
extern bdmf_fastlock iptv_lock;

/* IPTV ddr linked list */
uint32_t g_head_idx;
uint32_t g_free_linked_list[RDD_IPTV_DDR_CONTEXT_TABLE_SIZE + 1];
uint16_t g_fpm_base_buffer_size;
void _rdd_iptv_linked_list_init(void)
{
    uint32_t i;
    g_head_idx = 0;

    for (i = 0; i < (RDD_IPTV_DDR_CONTEXT_TABLE_SIZE); i++)
    {
        g_free_linked_list[i] = i+1;
    }
    g_free_linked_list[i] = IPTV_CTX_ENTRY_IDX_NULL;
}

void _rdd_iptv_ctx_entry_alloc(uint32_t *entry_idx)
{
    if (g_free_linked_list[g_head_idx] != IPTV_CTX_ENTRY_IDX_NULL)
    {
        *entry_idx = g_head_idx;
        g_head_idx = g_free_linked_list[g_head_idx];
    }
    else
    {
        *entry_idx = IPTV_CTX_ENTRY_IDX_NULL;
    }
}

void _rdd_iptv_ctx_entry_free(uint32_t entry_idx)
{
    g_free_linked_list[entry_idx] = g_head_idx;
    g_head_idx = entry_idx;
}

int rdd_iptv_ctx_table_ddr_init(uint16_t fpm_min_pool_size)
{
    uint32_t i, hi, lo;
    bdmf_phys_addr_t phys_addr_p;
#ifndef XRDP_EMULATION
    g_ctx_tbl_ptr = (RDD_IPTV_DDR_CONTEXT_ENTRY_DTS *)rdp_mm_aligned_alloc(IPTV_DDR_CTX_TBL, &phys_addr_p);
#else
    g_ctx_tbl_ptr = (RDD_IPTV_DDR_CONTEXT_ENTRY_DTS *)rdp_mm_aligned_alloc(IPTV_DDR_CTX_TBL, &phys_addr_p);
    g_ctx_tbl_ptr = phys_addr_p;
#endif
    if (!g_ctx_tbl_ptr)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Can't allocate IPTV ddr table");
    GET_ADDR_HIGH_LOW(hi, lo, phys_addr_p);
    bdmf_trace("IPTV ddr base addr: 0x%08x%08x\n", hi, lo);

    RDD_IPTV_DDR_CTX_TABLE_ADDRESS_LOW_WRITE_G(lo, RDD_IPTV_DDR_CTX_TABLE_ADDRESS_ADDRESS_ARR, 0);
    RDD_IPTV_DDR_CTX_TABLE_ADDRESS_HIGH_WRITE_G(hi, RDD_IPTV_DDR_CTX_TABLE_ADDRESS_ADDRESS_ARR, 0);


    GET_ADDR_HIGH_LOW(hi, lo, (uintptr_t)g_ctx_tbl_ptr);
    bdmf_trace("IPTV virtual ddr base addr: %p\n", g_ctx_tbl_ptr);
    _rdd_iptv_linked_list_init();
#ifndef XRDP_EMULATION
    for (i = 0; i < RDD_IPTV_DDR_CONTEXT_TABLE_SIZE; i++)
    {
        RDD_IPTV_DDR_CONTEXT_ENTRY_VALID_WRITE(0, &g_ctx_tbl_ptr[i]);
        RDD_IPTV_DDR_CONTEXT_ENTRY_NEXT_ENTRY_IDX_WRITE(IPTV_CTX_ENTRY_IDX_NULL, &g_ctx_tbl_ptr[i]);
    }
#endif
    g_fpm_base_buffer_size = fpm_min_pool_size;
    return BDMF_ERR_OK;
}

void rdd_iptv_ctx_table_ddr_destroy(void)
{
    RDD_IPTV_DDR_CTX_TABLE_ADDRESS_LOW_WRITE_G(0, RDD_IPTV_DDR_CTX_TABLE_ADDRESS_ADDRESS_ARR, 0);
    RDD_IPTV_DDR_CTX_TABLE_ADDRESS_HIGH_WRITE_G(0, RDD_IPTV_DDR_CTX_TABLE_ADDRESS_ADDRESS_ARR, 0);
}

/* module init */
int rdd_iptv_module_init(const rdd_module_t *module)
{
    RDD_IPTV_CFG_RES_OFFSET_WRITE_G(module->res_offset, module->cfg_ptr, 0);
    RDD_IPTV_CFG_CONTEXT_OFFSET_WRITE_G(module->context_offset, module->cfg_ptr, 0);
    RDD_IPTV_CFG_KEY_OFFSET_WRITE_G(((iptv_params_t *)module->params)->key_offset, module->cfg_ptr, 0);
    RDD_IPTV_CFG_HASH_TBL_IDX_WRITE_G(((iptv_params_t *)module->params)->hash_tbl_idx, module->cfg_ptr, 0);

    return 0;
}

void rdd_iptv_lkp_method_set(rdpa_iptv_lookup_method method)
{
    g_iptv_method = (uint8_t)method;
    RDD_BTRACE("rdd iptv set lookup method is: %d\n", method);
    RDD_IPTV_CFG_L2_MCAST_WRITE_G(L2_MCAST_METHOD(), RDD_IPTV_CFG_TABLE_ADDRESS_ARR, 0);
}

void rdd_iptv_hash_key_entry_compose(rdpa_iptv_channel_key_t *key, RDD_IPTV_HASH_LKP_ENTRY_DTS *hash_key_entry)
{
    uint32_t i, ip_addr;

    RDD_BTRACE("Composing key for IPTV lkp from %p to %p\n", key, hash_key_entry);
    if (L2_MCAST_METHOD())
    {
        hash_key_entry->addr_high = ((uint32_t)key->mcast_group.mac.b[0] << 8) | key->mcast_group.mac.b[1];

    	for (i = 0; i < sizeof(uint32_t); i++)
    		hash_key_entry->dst_ip_or_mac_low = (hash_key_entry->dst_ip_or_mac_low << 8) | key->mcast_group.mac.b[2 + i];
    }
    else
    {
    	if (key->mcast_group.l3.gr_ip.family == bdmf_ip_family_ipv4)
      		hash_key_entry->dst_ip_or_mac_low = key->mcast_group.l3.gr_ip.addr.ipv4;
    	else
    	{
            rdd_crc_ipv6_addr_calc(&key->mcast_group.l3.gr_ip, &ip_addr);
            hash_key_entry->dst_ip_or_mac_low = ip_addr;
            hash_key_entry->addr_high = key->mcast_group.l3.gr_ip.family;
    	}
    }
}

void rdd_iptv_hash_key_entry_decompose(RDD_IPTV_HASH_LKP_ENTRY_DTS hash_key_entry, rdpa_iptv_channel_key_t *key)
{
    uint32_t i;

    RDD_BTRACE("Decomposing key for IPTV lkp from %p to %p\n", &hash_key_entry, key);
    if (L2_MCAST_METHOD())
    {
        key->mcast_group.mac.b[0] = hash_key_entry.addr_high >> 8;
        key->mcast_group.mac.b[1] = hash_key_entry.addr_high & 0xFF;
        for (i = 0; i < sizeof(uint32_t); i++)
            key->mcast_group.mac.b[5 - i] = (hash_key_entry.dst_ip_or_mac_low >> (8 * i)) & 0xFF;
    }
    else
    {
        key->mcast_group.l3.gr_ip.family =  hash_key_entry.addr_high;
        key->mcast_group.l3.gr_ip.addr.ipv4 = hash_key_entry.dst_ip_or_mac_low;
    }
}

void rdd_iptv_hash_result_entry_compose(uint32_t ctx_idx, RDD_IPTV_HASH_RESULT_ENTRY_DTS *hash_result_entry)
{
    RDD_BTRACE("Composed ctx index for resust entry is: %d\n", ctx_idx);
    hash_result_entry->ctx_idx = ctx_idx;
    /* MS Bytes are occupied in aligned result */
}

void rdd_iptv_hash_result_entry_decompose(RDD_IPTV_HASH_RESULT_ENTRY_DTS hash_result_entry, uint32_t *ctx_idx)
{
    RDD_BTRACE("Decomposed ctx index for result entry is: %d\n", hash_result_entry.ctx_idx);
    *ctx_idx = hash_result_entry.ctx_idx;
}

void rdd_iptv_ddr_context_entry_get(RDD_IPTV_DDR_CONTEXT_ENTRY_DTS *ddr_ctx_entry, uint32_t entry_idx)
{
    uint8_t *ddr_ctx = (uint8_t *)ddr_ctx_entry;

    RDD_BTRACE("Copying from entry index %d in DDR table\n", entry_idx);
    bdmf_fastlock_lock(&iptv_lock);
    MREAD_BLK_8(ddr_ctx, &g_ctx_tbl_ptr[entry_idx], sizeof(RDD_IPTV_DDR_CONTEXT_ENTRY_DTS));
#ifdef FIRMWARE_LITTLE_ENDIAN
    SWAPBYTES(ddr_ctx, sizeof(RDD_IPTV_DDR_CONTEXT_ENTRY_DTS));
#endif
    bdmf_fastlock_unlock(&iptv_lock);
}

static void _rdd_iptv_write_ddr_context(RDD_IPTV_DDR_CONTEXT_ENTRY_DTS *ddr_ctx_entry, uint32_t entry_idx)
{
    uint8_t *ddr_ctx = (uint8_t *)ddr_ctx_entry;

    RDD_BTRACE("Copying to entry index %d in DDR table\n", entry_idx);
    bdmf_fastlock_lock(&iptv_lock);
#ifdef FIRMWARE_LITTLE_ENDIAN
    SWAPBYTES(ddr_ctx, sizeof(RDD_IPTV_DDR_CONTEXT_ENTRY_DTS));
#endif
    MWRITE_BLK_8((uint8_t *)&g_ctx_tbl_ptr[entry_idx], ddr_ctx, sizeof(RDD_IPTV_DDR_CONTEXT_ENTRY_DTS));
    bdmf_fastlock_unlock(&iptv_lock);
}
    
#if defined(BCM63158)
static void _rdd_iptv_gpe_result_entry_key_params_compose(rdpa_iptv_channel_key_t *key,
                                                          RDD_IPTV_DDR_CONTEXT_ENTRY_DTS *ddr_ctx_entry)
{
    RDD_IPTV_GPE_BASED_RESULT_DTS *ddr_ctx_gpe = (RDD_IPTV_GPE_BASED_RESULT_DTS *)&ddr_ctx_entry->result;

    ddr_ctx_gpe->num_vlan_tags = key->num_vlan_tags;
    ddr_ctx_gpe->any_inner_vid = (key->inner_vid == ANY_VID) ? 1 : 0;
    ddr_ctx_gpe->inner_vid = key->inner_vid;
}
#endif

static void _rdd_iptv_result_entry_key_params_compose(rdpa_iptv_channel_key_t *key,
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS *ddr_ctx_entry)
{
    bdmf_ip_t *src_ip;
    uint32_t is_ipv6, any_vid, any_src_ip, addr = 0;
#if defined(BCM63158)
    _rdd_iptv_gpe_result_entry_key_params_compose(key, ddr_ctx_entry);
#endif
#if defined(CONFIG_RUNNER_IPTV_LKUP_KEY_INCLUDE_SRC_PORT)
    ddr_ctx_entry->rx_if = key->rx_if;
#endif
    any_vid = key->vid == ANY_VID;
    is_ipv6 = key->mcast_group.l3.gr_ip.family == bdmf_ip_family_ipv6;
    ddr_ctx_entry->any_vid = any_vid;

    RDD_BTRACE("Compose result entry key params in DDR, key: %p , ddr ctx: %p\n", key, ddr_ctx_entry);
    if (!any_vid)
        ddr_ctx_entry->vid = key->vid;
    if (L2_MCAST_METHOD())
    {
        ddr_ctx_entry->any_src_ip = 1;
        return;
    }

    if (is_ipv6)
    {
        ddr_ctx_entry->ip_ver = is_ipv6;
        memcpy(ddr_ctx_entry->dst_ipv6_addr, key->mcast_group.l3.gr_ip.addr.ipv6.data,
                RDD_IPTV_DDR_CONTEXT_ENTRY_DST_IPV6_ADDR_NUMBER);
    }
    if (!SRC_IP_IN_MCAST_METHOD())
    {
        ddr_ctx_entry->any_src_ip = 1;
        return;
    }

    src_ip = &key->mcast_group.l3.src_ip;
    any_src_ip = bdmf_ip_is_zero(src_ip);
    ddr_ctx_entry->any_src_ip = any_src_ip;
    if (any_src_ip)
        return;

    if (is_ipv6)
    {
        memcpy(ddr_ctx_entry->src_ipv6_addr, key->mcast_group.l3.src_ip.addr.ipv6.data,
            RDD_IPTV_DDR_CONTEXT_ENTRY_SRC_IPV6_ADDR_NUMBER);
        rdd_crc_ipv6_addr_calc(&key->mcast_group.l3.src_ip, &addr);
    }
    else
        addr = src_ip->addr.ipv4;
    ddr_ctx_entry->src_ip = addr;
}

#if defined(BCM63158)
static void _rdd_iptv_gpe_result_entry_key_params_decompose(RDD_IPTV_DDR_CONTEXT_ENTRY_DTS *ddr_ctx_entry,
                                                            rdpa_iptv_channel_key_t *key)
{
    RDD_IPTV_GPE_BASED_RESULT_DTS *ddr_ctx_gpe = (RDD_IPTV_GPE_BASED_RESULT_DTS *)&ddr_ctx_entry->result;

    key->num_vlan_tags = ddr_ctx_gpe->num_vlan_tags;

    if(ddr_ctx_gpe->any_inner_vid)
        key->inner_vid = ANY_VID;
    else
        key->inner_vid = ddr_ctx_gpe->inner_vid;
}
#endif

static void _rdd_iptv_result_entry_key_params_decompose(RDD_IPTV_DDR_CONTEXT_ENTRY_DTS *ddr_ctx_entry,
        rdpa_iptv_channel_key_t *key)
{
    RDD_BTRACE("Decompose result entry key params in DDR, key: %p , ddr ctx: %p\n", key, ddr_ctx_entry);
    RDD_BTRACE("Ddr_ctx_entry: valid = %d, ip_ver = %d, any_vid = %d, any_src_ip = %d, vid = %d, "
        "next_entry_idx = %d, ssid_vector_0_or_flooding_vport = 0x%x,  ssid_vector_1 = 0x%x, ssid_vector_2 = 0x%x, "
        "radio_dhd_vector = 0x%x, wlan_mcast_proxy_enabled = %d, wlan_mcast_index = %d, wlan_mcast_tx_prio = %d, "
        "egress_ports_vector = 0x%x\n", ddr_ctx_entry->valid, ddr_ctx_entry->ip_ver, ddr_ctx_entry->any_vid,
        ddr_ctx_entry->any_src_ip, ddr_ctx_entry->vid, ddr_ctx_entry->next_entry_idx,
        ddr_ctx_entry->ssid_vector_0_or_flooding_vport,
        ddr_ctx_entry->ssid_vector_1, ddr_ctx_entry->ssid_vector_2, ddr_ctx_entry->radio_dhd_vector,
        ddr_ctx_entry->wlan_mcast_proxy_enabled, ddr_ctx_entry->wlan_mcast_index, ddr_ctx_entry->wlan_mcast_tx_prio,
        ddr_ctx_entry->egress_ports_vector);
#if defined(BCM63158)
    _rdd_iptv_gpe_result_entry_key_params_decompose(ddr_ctx_entry, key);
#endif
#if defined(CONFIG_RUNNER_IPTV_LKUP_KEY_INCLUDE_SRC_PORT)
    key->rx_if = ddr_ctx_entry->rx_if;
#endif
    if (ddr_ctx_entry->any_vid)
        key->vid = ANY_VID;
    else
        key->vid = ddr_ctx_entry->vid;
    if (L2_MCAST_METHOD())
        return;

    key->mcast_group.l3.gr_ip.family = ddr_ctx_entry->ip_ver;
    key->mcast_group.l3.src_ip.family = ddr_ctx_entry->ip_ver;

    if (ddr_ctx_entry->ip_ver == bdmf_ip_family_ipv6)
        memcpy(key->mcast_group.l3.gr_ip.addr.ipv6.data, ddr_ctx_entry->dst_ipv6_addr,
            RDD_IPTV_DDR_CONTEXT_ENTRY_SRC_IPV6_ADDR_NUMBER);

#if !defined(BCM63158)
    if (!ddr_ctx_entry->src_ip)
        return;
#endif

    if (ddr_ctx_entry->ip_ver == bdmf_ip_family_ipv4)
        key->mcast_group.l3.src_ip.addr.ipv4 = ddr_ctx_entry->src_ip;
    else
        memcpy(key->mcast_group.l3.src_ip.addr.ipv6.data, ddr_ctx_entry->src_ipv6_addr,
            RDD_IPTV_DDR_CONTEXT_ENTRY_SRC_IPV6_ADDR_NUMBER);
}

#if defined(BCM63158)
static void _rdd_iptv_gpe_result_compose(uint8_t *ic_ctx,
                                         RDD_IPTV_DDR_CONTEXT_ENTRY_DTS *ddr_ctx_entry)
{
    RDD_IPTV_GPE_BASED_RESULT_DTS *gpe_entry = (RDD_IPTV_GPE_BASED_RESULT_DTS *)ic_ctx;
    RDD_IPTV_GPE_BASED_RESULT_DTS *ddr_ctx_gpe =
        (RDD_IPTV_GPE_BASED_RESULT_DTS *)&ddr_ctx_entry->result;

    ddr_ctx_gpe->is_routed = gpe_entry->is_routed;
    ddr_ctx_gpe->mtu = gpe_entry->mtu;
    ddr_ctx_gpe->wlan_mcast_clients = gpe_entry->wlan_mcast_clients;
    ddr_ctx_gpe->port_buffer_addr_high = gpe_entry->port_buffer_addr_high;
    ddr_ctx_gpe->port_buffer_addr_low = gpe_entry->port_buffer_addr_low;
    memcpy(ddr_ctx_gpe->l3_command_list, gpe_entry->l3_command_list,
           RDD_IPTV_GPE_BASED_RESULT_L3_COMMAND_LIST_NUMBER);
}

static void _rdd_iptv_gpe_result_decompose(RDD_IPTV_DDR_CONTEXT_ENTRY_DTS *ddr_ctx_entry,
                                           uint8_t *ic_ctx)
{
    RDD_IPTV_GPE_BASED_RESULT_DTS *ddr_ctx_gpe =
        (RDD_IPTV_GPE_BASED_RESULT_DTS *)&ddr_ctx_entry->result;
    RDD_IPTV_GPE_BASED_RESULT_DTS *gpe_entry = (RDD_IPTV_GPE_BASED_RESULT_DTS *)ic_ctx;

    gpe_entry->is_routed = ddr_ctx_gpe->is_routed;
    gpe_entry->mtu = ddr_ctx_gpe->mtu;
    gpe_entry->wlan_mcast_clients = ddr_ctx_gpe->wlan_mcast_clients;
    gpe_entry->port_buffer_addr_high = ddr_ctx_gpe->port_buffer_addr_high;
    gpe_entry->port_buffer_addr_low = ddr_ctx_gpe->port_buffer_addr_low;
    memcpy(gpe_entry->l3_command_list, ddr_ctx_gpe->l3_command_list,
           RDD_IPTV_GPE_BASED_RESULT_L3_COMMAND_LIST_NUMBER);
}
#endif

static void _rdd_iptv_result_entry_compose(rdd_iptv_entry_t *iptv_entry, uint8_t *ic_ctx,
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS *ddr_ctx_entry)
{
    uint8_t i, replications_num = 0;
#ifdef CONFIG_DHD_RUNNER
    RDD_WLAN_MCAST_DHD_LIST_ENTRY_DTS dhd_list_entry;
    uint8_t radio_dhd_vector = 0;
#endif

    RDD_BTRACE("Compose result entry, entry: %p , IC ctx: %p, DDR ctx: %p\n"
            , iptv_entry, ic_ctx, ddr_ctx_entry);
    if (ddr_ctx_entry->valid)
        _rdd_iptv_result_entry_key_params_decompose(ddr_ctx_entry, &iptv_entry->key);
    _rdd_iptv_result_entry_key_params_compose(&iptv_entry->key, ddr_ctx_entry);
    ddr_ctx_entry->egress_ports_vector = iptv_entry->egress_port_vector; 
    BDMF_TRACE_DBG("egress ports: %x\n", ddr_ctx_entry->egress_ports_vector);
    for (i = 0; i < (sizeof(iptv_entry->egress_port_vector) * 8); i++)
    {
        if (!(iptv_entry->egress_port_vector & ((typeof(iptv_entry->egress_port_vector))1 << i)))
            continue;
#ifndef G9991
        /* To simplify the logic in FW, compute replications number only for non-CPU vports only */
        if (i >= RDD_CPU0_VPORT && i <= RDD_CPU_VPORT_LAST)
            continue;
#endif
        replications_num++;
    }
    if (replications_num)
    {
        ddr_ctx_entry->replications = replications_num - 1;

        switch (replications_num) {
        case 1 ... 4:
            ddr_ctx_entry->pool_num = FPM_POOL_ID_ONE_BUFFER;
            break;
        case 5 ... 8:
            ddr_ctx_entry->pool_num = FPM_POOL_ID_TWO_BUFFERS;
            break;
        case 9 ... 16:
            ddr_ctx_entry->pool_num = FPM_POOL_ID_FOUR_BUFFERS;
            break;
        case 17 ... 32:
            ddr_ctx_entry->pool_num = FPM_POOL_ID_EIGHT_BUFFERS;
            break;
        }
    }
    ddr_ctx_entry->wlan_mcast_index = iptv_entry->wlan_mcast_index;
    ddr_ctx_entry->ssid_vector_0_or_flooding_vport = iptv_entry->wlan_mcast_fwd_table.wfd_0_ssid_vector;
    ddr_ctx_entry->ssid_vector_1 = iptv_entry->wlan_mcast_fwd_table.wfd_1_ssid_vector;
    ddr_ctx_entry->ssid_vector_2 = iptv_entry->wlan_mcast_fwd_table.wfd_2_ssid_vector;
    ddr_ctx_entry->wlan_mcast_tx_prio = iptv_entry->wlan_mcast_fwd_table.wfd_tx_priority;
    ddr_ctx_entry->wlan_mcast_proxy_enabled = iptv_entry->wlan_mcast_fwd_table.is_proxy_enabled;

#if defined(CONFIG_RUNNER_IPTV_LKUP_KEY_INCLUDE_SRC_PORT) && defined(BCM_PON_XRDP)
    ddr_ctx_entry->rx_if = iptv_entry->key.rx_if;
#endif

#ifdef CONFIG_DHD_RUNNER
    /* Update DHD radio vector */
    for (i = 0; i < RDPA_WLAN_MCAST_MAX_DHD_STATIONS; i++)
    {
        uint8_t radio_idx;

        dhd_list_entry = *(RDD_WLAN_MCAST_DHD_LIST_ENTRY_DTS *)&iptv_entry->wlan_mcast_fwd_table.dhd_station_list[i];
        if (!dhd_list_entry.valid)
            continue;
        RDD_WLAN_MCAST_DHD_STATION_ENTRY_RADIO_INDEX_READ_G(radio_idx, RDD_WLAN_MCAST_DHD_STATION_TABLE_ADDRESS_ARR,
            dhd_list_entry.index);
        radio_dhd_vector |= (1 << radio_idx);
    }
    ddr_ctx_entry->radio_dhd_vector = radio_dhd_vector;
#endif

#if !defined(BCM63158)
    /* XXX: Update WLAN TC? */
    for (i = 0; ic_ctx && i < RDD_IPTV_RULE_BASED_RESULT_RULE_NUMBER; i++)
        ddr_ctx_entry->result[i] = ic_ctx[i];
#else
    _rdd_iptv_gpe_result_compose(ic_ctx, ddr_ctx_entry);
#endif

    RDD_BTRACE("Composed ddr_ctx_entry: valid = %d, ip_ver = %d, any_vid = %d, any_src_ip = %d, vid = %d, "
        "next_entry_idx = %d, ssid_vector_0_or_flooding_vport = 0x%x,  ssid_vector_1 = 0x%x, ssid_vector_2 = 0x%x, "
        "radio_dhd_vector = 0x%x, wlan_mcast_proxy_enabled = %d, wlan_mcast_index = %d, wlan_mcast_tx_prio = %d, "
        "egress_ports_vector = 0x%x\n", ddr_ctx_entry->valid, ddr_ctx_entry->ip_ver, ddr_ctx_entry->any_vid,
        ddr_ctx_entry->any_src_ip, ddr_ctx_entry->vid, ddr_ctx_entry->next_entry_idx,
        ddr_ctx_entry->ssid_vector_0_or_flooding_vport,
        ddr_ctx_entry->ssid_vector_1, ddr_ctx_entry->ssid_vector_2, ddr_ctx_entry->radio_dhd_vector,
        ddr_ctx_entry->wlan_mcast_proxy_enabled, ddr_ctx_entry->wlan_mcast_index, ddr_ctx_entry->wlan_mcast_tx_prio,
        ddr_ctx_entry->egress_ports_vector);
}

void _rdd_iptv_result_entry_decompose(RDD_IPTV_DDR_CONTEXT_ENTRY_DTS *ddr_ctx_entry,
    rdd_iptv_entry_t *iptv_entry, uint8_t *ic_ctx)
{
    RDD_BTRACE("Decompose result entry, entry: %p , IC ctx: %p, DDR ctx: %p\n", iptv_entry, ic_ctx, ddr_ctx_entry);
    _rdd_iptv_result_entry_key_params_decompose(ddr_ctx_entry, &iptv_entry->key);

    iptv_entry->egress_port_vector = ddr_ctx_entry->egress_ports_vector;
    iptv_entry->wlan_mcast_index = ddr_ctx_entry->wlan_mcast_index;
    iptv_entry->wlan_mcast_fwd_table.wfd_0_ssid_vector = ddr_ctx_entry->ssid_vector_0_or_flooding_vport;
    iptv_entry->wlan_mcast_fwd_table.wfd_1_ssid_vector = ddr_ctx_entry->ssid_vector_1;
    iptv_entry->wlan_mcast_fwd_table.wfd_2_ssid_vector = ddr_ctx_entry->ssid_vector_2;
    iptv_entry->wlan_mcast_fwd_table.wfd_tx_priority = ddr_ctx_entry->wlan_mcast_tx_prio;
    iptv_entry->wlan_mcast_fwd_table.is_proxy_enabled = ddr_ctx_entry->wlan_mcast_proxy_enabled;

#if !defined(BCM63158)
#if defined(CONFIG_RUNNER_IPTV_LKUP_KEY_INCLUDE_SRC_PORT)
    iptv_entry->key.rx_if = ddr_ctx_entry->rx_if;
#endif
    /* channel vid did not shown correctly on exam command for PON */
    if (ddr_ctx_entry->any_vid)
        iptv_entry->key.vid = ANY_VID;
    else
        iptv_entry->key.vid = ddr_ctx_entry->vid;

    if (1)
    {
        uint8_t i;
        /* XXX: Update WLAN TC */
        for (i = 0; ic_ctx && i < RDD_IPTV_RULE_BASED_RESULT_RULE_NUMBER; i++)
            ic_ctx[i] = ddr_ctx_entry->result[i];
    }
#else
    iptv_entry->replications = ddr_ctx_entry->replications;

    _rdd_iptv_gpe_result_decompose(ddr_ctx_entry, ic_ctx);
#endif
}

int _rdd_iptv_check_entry_precedence(RDD_IPTV_DDR_CONTEXT_ENTRY_DTS new_ctx, RDD_IPTV_DDR_CONTEXT_ENTRY_DTS next_ctx)
{
    uint8_t new_any, next_any;

    if (L2_MCAST_METHOD())
    {
        new_any = new_ctx.any_vid;
        next_any = next_ctx.any_vid;
        if (!new_any && next_any)
            return 0;
    }

    /* both entries any-vid is set*/
    if (SRC_IP_IN_MCAST_METHOD())
    {
        new_any = new_ctx.any_src_ip;
        next_any =  next_ctx.any_src_ip;

        if (!new_any && next_any)
            return 0;
    }

    return 1;
}

int rdd_iptv_result_entry_add(rdd_iptv_entry_t *iptv_entry, uint8_t *ic_ctx, uint32_t *head_idx, uint32_t cntr_id,
    uint32_t *entry_idx)
{
    int rc;
    uint32_t next_entry_idx, prev_entry_idx;
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS new_ctx = {}, head_ctx = {}, prev_ctx = {}, cur_ctx = {};

    RDD_BTRACE("Add result entry, entry: %p , IC ctx: %p, head idx: %d, entry idx: %d\n",
         iptv_entry, ic_ctx, head_idx ? *head_idx : -1, *entry_idx);
    prev_entry_idx = IPTV_CTX_ENTRY_IDX_NULL;
    rc = BDMF_ERR_OK;
    _rdd_iptv_ctx_entry_alloc(entry_idx);
    if (*entry_idx == IPTV_CTX_ENTRY_IDX_NULL)
    {
        rc = BDMF_ERR_NORES;
        BDMF_TRACE_INFO("going to exit - no elements in list\n");
        goto exit;
    }

    _rdd_iptv_result_entry_compose(iptv_entry, ic_ctx, &new_ctx);
    new_ctx.cntr_id = cntr_id;

    /* first element in the list */
    if (!head_idx)
    {
        new_ctx.next_entry_idx = IPTV_CTX_ENTRY_IDX_NULL;
        goto exit;
    }

    next_entry_idx = *head_idx;
    rdd_iptv_ddr_context_entry_get(&head_ctx, *head_idx);
    if (!_rdd_iptv_check_entry_precedence(new_ctx, head_ctx))
        *head_idx = *entry_idx;
    else
    {
        rdd_iptv_ddr_context_entry_get(&cur_ctx, *head_idx);
        prev_ctx = cur_ctx;
        do
        {
            prev_entry_idx = next_entry_idx;
            next_entry_idx =  cur_ctx.next_entry_idx;
            prev_ctx = cur_ctx;
            if (next_entry_idx == IPTV_CTX_ENTRY_IDX_NULL)
                break;
        rdd_iptv_ddr_context_entry_get(&cur_ctx, next_entry_idx);
        } while (_rdd_iptv_check_entry_precedence(new_ctx, cur_ctx));
    }
    new_ctx.next_entry_idx = next_entry_idx;
    if (prev_entry_idx != IPTV_CTX_ENTRY_IDX_NULL)
    {
        prev_ctx.next_entry_idx = *entry_idx;
        _rdd_iptv_write_ddr_context(&prev_ctx, prev_entry_idx);
    }

exit:
    if (!rc)
    {
        new_ctx.valid = 1;
        _rdd_iptv_write_ddr_context(&new_ctx, *entry_idx);
    }
    return rc;
}

void rdd_iptv_result_entry_modify(rdd_iptv_entry_t *iptv_entry, uint8_t *ic_ctx, uint32_t entry_idx)
{
    uint32_t next_entry_idx, cntr_id;
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS ddr_ctx_entry = {};

    RDD_BTRACE("Modify result entry, entry: %p , IC ctx: %d, entry idx: %d\n"
                            , iptv_entry, *ic_ctx, entry_idx);

    rdd_iptv_ddr_context_entry_get(&ddr_ctx_entry, entry_idx);
    next_entry_idx = ddr_ctx_entry.next_entry_idx;
    cntr_id = ddr_ctx_entry.cntr_id;
    _rdd_iptv_result_entry_compose(iptv_entry, ic_ctx, &ddr_ctx_entry);

    ddr_ctx_entry.next_entry_idx = next_entry_idx;
    ddr_ctx_entry.cntr_id = cntr_id;
    _rdd_iptv_write_ddr_context(&ddr_ctx_entry, entry_idx);
}

int rdd_iptv_result_entry_get(uint32_t ctx_idx, rdd_iptv_entry_t *rdd_iptv_entry, uint8_t *ic_ctx)
{
    uint8_t valid;
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS ddr_ctx_entry = {};

    RDD_BTRACE("Get result entry, entry: %p , IC ctx: %d, ctx idx: %d\n"
                                , rdd_iptv_entry, *ic_ctx, ctx_idx);
    rdd_iptv_ddr_context_entry_get(&ddr_ctx_entry, ctx_idx);
    valid = ddr_ctx_entry.valid;
    if (!valid)
        return BDMF_ERR_NOENT;
    _rdd_iptv_result_entry_decompose(&ddr_ctx_entry, rdd_iptv_entry, ic_ctx);
    return BDMF_ERR_OK;
}

int rdd_iptv_result_entry_delete(uint32_t entry_idx, uint32_t *head_idx, uint32_t *cntr_id)
{
    uint32_t next_entry_idx , cur_entry_idx = IPTV_CTX_ENTRY_IDX_NULL;
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS ddr_ctx_entry = {} , cur_ctx_entry = {};

    RDD_BTRACE("Delete result entry, entry idx: %d\n", entry_idx);
    rdd_iptv_ddr_context_entry_get(&ddr_ctx_entry, entry_idx);

    /* delete the first element in the list*/
    if (*head_idx == entry_idx)
    {
       next_entry_idx = ddr_ctx_entry.next_entry_idx;
       if (next_entry_idx != IPTV_CTX_ENTRY_IDX_NULL)
            *head_idx = next_entry_idx;
    }
    else
    {
        cur_entry_idx = *head_idx;
        rdd_iptv_ddr_context_entry_get(&cur_ctx_entry, cur_entry_idx);
        next_entry_idx = cur_ctx_entry.next_entry_idx;
        while (next_entry_idx != entry_idx)
        {
            if (next_entry_idx == IPTV_CTX_ENTRY_IDX_NULL)
                return BDMF_ERR_NOENT;
            cur_entry_idx = cur_ctx_entry.next_entry_idx;
            rdd_iptv_ddr_context_entry_get(&cur_ctx_entry, cur_entry_idx);
            next_entry_idx = cur_ctx_entry.next_entry_idx;
        }
        cur_ctx_entry.next_entry_idx = ddr_ctx_entry.next_entry_idx;
       _rdd_iptv_write_ddr_context(&cur_ctx_entry, cur_entry_idx);
    }

    ddr_ctx_entry.valid = 0;

    *cntr_id = ddr_ctx_entry.cntr_id;
    _rdd_iptv_write_ddr_context(&ddr_ctx_entry, entry_idx);
    _rdd_iptv_ctx_entry_free(entry_idx);

    return BDMF_ERR_OK;
}

#if defined(BCM63158)
static int _rdd_iptv_gpe_channel_key_compare(RDD_IPTV_DDR_CONTEXT_ENTRY_DTS *key_0,
                                             RDD_IPTV_DDR_CONTEXT_ENTRY_DTS *key_1)
{
    RDD_IPTV_GPE_BASED_RESULT_DTS *gpe_key_0 = (RDD_IPTV_GPE_BASED_RESULT_DTS *)&key_0->result;
    RDD_IPTV_GPE_BASED_RESULT_DTS *gpe_key_1 = (RDD_IPTV_GPE_BASED_RESULT_DTS *)&key_1->result;

    if(gpe_key_0->num_vlan_tags != gpe_key_1->num_vlan_tags)
        return 0;

    if(gpe_key_0->any_inner_vid != gpe_key_1->any_inner_vid)
        return 0;

    if(!gpe_key_0->any_inner_vid && (gpe_key_0->inner_vid != gpe_key_1->inner_vid))
        return 0;

    return 1;
}
#endif

static int _rdd_iptv_channel_key_compare(RDD_IPTV_DDR_CONTEXT_ENTRY_DTS *key_0, RDD_IPTV_DDR_CONTEXT_ENTRY_DTS *key_1)
{
    RDD_BTRACE("Compare IPTV channel key, key0: %p, key1: %p\n"
                            , key_0, key_1);
    if (VID_IN_MCAST_METHOD())
    {
        if (key_0->any_vid != key_1->any_vid)
            return 0;
        if (!key_0->any_vid && (key_0->vid != key_1->vid))
            return 0;
    }
    if (!L2_MCAST_METHOD())
    {
        if (key_0->any_src_ip != key_1->any_src_ip)
            return 0;
        if (!key_0->any_src_ip && (key_0->src_ip != key_1->src_ip))
            return 0;
    }

    if (memcmp(key_0->src_ipv6_addr, key_1->src_ipv6_addr, RDD_IPTV_DDR_CONTEXT_ENTRY_SRC_IPV6_ADDR_NUMBER))
    {
        BDMF_TRACE_DBG("ipv6 src ip is not the same");
        return 0;
    }
    if (memcmp(key_0->dst_ipv6_addr, key_1->dst_ipv6_addr, RDD_IPTV_DDR_CONTEXT_ENTRY_DST_IPV6_ADDR_NUMBER))
    {
        BDMF_TRACE_DBG("ipv6 dst ip is not the same");
        return 0;
    }

#if !defined(CONFIG_RUNNER_IPTV_LKUP_KEY_INCLUDE_SRC_POR) && !defined(BCM63158)
    return 1;
#else

    if (key_0->rx_if != key_1->rx_if)
    {
        BDMF_TRACE_DBG("rx_if is not the same");
        return 0;
    }
#endif
#if !defined(BCM63158)
    return 1;
#else
    return _rdd_iptv_gpe_channel_key_compare(key_0, key_1);
#endif
}

int rdd_iptv_result_entry_find(rdpa_iptv_channel_key_t *key, uint32_t head_idx, uint32_t *entry_idx)
{
    uint32_t next_entry_idx;
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS key_entry = {};
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS cmp_key_entry = {};

    RDD_BTRACE("Find result entry, key: %p , head idx: %d\n"
                                , key, head_idx);
    _rdd_iptv_result_entry_key_params_compose(key, &key_entry);

    for (next_entry_idx = head_idx; next_entry_idx != IPTV_CTX_ENTRY_IDX_NULL;)
    {
        rdd_iptv_ddr_context_entry_get(&cmp_key_entry, next_entry_idx);
        if (_rdd_iptv_channel_key_compare(&key_entry, &cmp_key_entry))
        {
            *entry_idx = next_entry_idx;
            return BDMF_ERR_OK;
        }
        next_entry_idx = cmp_key_entry.next_entry_idx;
    }
    return BDMF_ERR_NOENT;
}

int rdd_iptv_result_entry_next_idx_get(uint32_t entry_idx, uint32_t *next_idx)
{
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS ddr_ctx_entry = {};

    RDD_BTRACE("Get Next index for, entry idx: %d\n", entry_idx);
    rdd_iptv_ddr_context_entry_get(&ddr_ctx_entry, entry_idx);
    if (!ddr_ctx_entry.valid)
        return BDMF_ERR_NOENT;
    *next_idx = ddr_ctx_entry.next_entry_idx;
    return BDMF_ERR_OK;
}

int rdd_iptv_cntr_idx_get(uint32_t entry_idx, uint32_t *cntr_idx)
{
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS ddr_ctx_entry = {};

    RDD_BTRACE("counter get for, entry idx: %d\n", entry_idx);
    rdd_iptv_ddr_context_entry_get(&ddr_ctx_entry, entry_idx);
    if (!ddr_ctx_entry.valid)
        return BDMF_ERR_NOENT;
    *cntr_idx = ddr_ctx_entry.cntr_id;

    return BDMF_ERR_OK;
}
