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
#include "rdd_natc.h"
#include "rdd_multicast_whitelist.h"
#include "rdp_drv_natc.h"

/*
 * Multicast Whitelist utilizes NAT Cache table.
 * The table size is set to 8K, with key size 16 bytes and context size 32 byte.
 * First 3 bytes of key are masked.
 * (This combination is the smallest size configurable for NAT Cache.)
 * Bucket is set to 8 bins.  Caching capability is disabled for this table.
 */

typedef struct
{
    uint32_t natc_control;
    RDD_MULTICAST_WHITELIST_CONTEXT_ENTRY_DTS mcast_wl_ctxt;
} multicast_whitelist_context_entry_t;

static uint8_t multicast_wlist_tbl_idx = 0xff;

/* internal index translation table management
 * This is mainly used for case that the first entry of a chain is deleted,
 * and the 2nd entry is going to replace the first entry in the chain.
 * However, the index that has been previously assigned back to the user 
 * is still the original entry index, but internally it has been moved.
 * A translation table is needed to maintain it */
typedef struct mcast_wlist_index_xlate
{
    DLIST_ENTRY(mcast_wlist_index_xlate) list;
    uint32_t org_idx;
    uint32_t first_idx;
} mcast_wlist_index_xlate_t;
DLIST_HEAD(mcast_wlist_index_xlate_t, mcast_wlist_index_xlate);
struct mcast_wlist_index_xlate_t mcast_wlist_index_xlate_tbl;

static void __rdd_multicast_whitelist_index_xlate_add(uint32_t org_entry_idx, uint32_t first_entry_idx)
{
    mcast_wlist_index_xlate_t *new_entry;

    /* there is no check for if an existing identical entry has been added.
     * because it shouldn't happen */

    new_entry = bdmf_alloc(sizeof(mcast_wlist_index_xlate_t));
    if (!new_entry)
        return;

    new_entry->org_idx = org_entry_idx;
    new_entry->first_idx = first_entry_idx;
    DLIST_INSERT_HEAD(&mcast_wlist_index_xlate_tbl, new_entry, list);
}

static uint32_t __rdd_multicast_whitelist_index_xlate_pop(uint32_t org_entry_idx)
{
    mcast_wlist_index_xlate_t *entry = NULL, *tmp_entry;
    uint32_t first_index;

    DLIST_FOREACH_SAFE(entry, &mcast_wlist_index_xlate_tbl, list, tmp_entry)
    {
        if (entry->org_idx == org_entry_idx)
        {
            first_index = entry->first_idx;
            DLIST_REMOVE(entry, list);
            bdmf_free(entry);

            return first_index;
        }
    }

    return org_entry_idx;
}

/* translate from given_index -> first_idx/org_entry_idx */
static uint32_t __rdd_multicast_whitelist_index_fwd_xlate(uint32_t org_entry_idx)
{
    mcast_wlist_index_xlate_t *entry = NULL, *tmp_entry;

    DLIST_FOREACH_SAFE(entry, &mcast_wlist_index_xlate_tbl, list, tmp_entry)
    {
        if (entry->org_idx == org_entry_idx)
            return entry->first_idx;
    }

    return org_entry_idx;
}

/* translate from first_idx/org_entry_idx -> org_entry_idx */
static uint32_t __rdd_multicast_whitelist_index_bkwd_xlate(uint32_t org_entry_idx)
{
    mcast_wlist_index_xlate_t *entry = NULL, *tmp_entry;

    DLIST_FOREACH_SAFE(entry, &mcast_wlist_index_xlate_tbl, list, tmp_entry)
    {
        if (entry->first_idx == org_entry_idx)
            return entry->org_idx;
    }

    return org_entry_idx;
}

/* checking of the given index is a previously deleted index for the first entry
 * in the chain.. if so, caller need to handle accordingly, usually respond with noent */
static int __rdd_multicast_whitelist_index_xlate_check_first_idx(uint32_t entry_idx)
{
    mcast_wlist_index_xlate_t *entry = NULL, *tmp_entry;

    DLIST_FOREACH_SAFE(entry, &mcast_wlist_index_xlate_tbl, list, tmp_entry)
    {
        if (entry->first_idx == entry_idx)
            return 1;
    }
    return 0;
}

/* module init */
int rdd_multicast_whitelist_module_init(const rdd_module_t *module)
{
    RDD_MULTICAST_WHITELIST_CFG_RES_OFFSET_WRITE_G(module->res_offset, module->cfg_ptr, 0);
    RDD_MULTICAST_WHITELIST_CFG_CONTEXT_OFFSET_WRITE_G(module->context_offset, module->cfg_ptr, 0);
    RDD_MULTICAST_WHITELIST_CFG_KEY_OFFSET_WRITE_G(((multicast_whitelist_params_t *)module->params)->key_offset, module->cfg_ptr, 0);

    multicast_wlist_tbl_idx = ((multicast_whitelist_params_t *)module->params)->natc_tbl_idx;
    RDD_MULTICAST_WHITELIST_CFG_NATC_TBL_IDX_WRITE_G(multicast_wlist_tbl_idx, module->cfg_ptr, 0);
    DLIST_INIT(&mcast_wlist_index_xlate_tbl);

    return 0;
}

int rdd_multicast_whitelist_context_table_addr_set(uint32_t addr_high, uint32_t addr_low)
{
    RDD_MULTICAST_WHITELIST_CFG_DDR_CTX_TBL_HIGH_WRITE_G(addr_high, RDD_MULTICAST_WHITELIST_CFG_TABLE_ADDRESS_ARR, 0);
    /* the reason we are adding 4 to the low_addr is. the real context sitting in DDR has 8 bytes of counter sitting
     * in front of the real context, but the context pulls in by NATCache is with 4bytes NATC Control data in front.
     * And the auto-generated data structure + firmware is based on the later data structure, therefore, we move
     * the pointer by 4 bytes when DMA_RD from DDR directly */
    RDD_MULTICAST_WHITELIST_CFG_DDR_CTX_TBL_LOW_WRITE_G(addr_low + 4, RDD_MULTICAST_WHITELIST_CFG_TABLE_ADDRESS_ARR, 0);
    return 0;
}

int rdd_multicast_whitelist_entry_add(RDD_MULTICAST_WHITELIST_LKP_ENTRY_DTS *mcast_wl_entry,
                                      RDD_MULTICAST_WHITELIST_CONTEXT_ENTRY_DTS *mcast_wl_ctxt,
                                      uint32_t *entry_idx)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    uint32_t hash_idx, curr_idx, next_idx;
    multicast_whitelist_context_entry_t wl_ctxt_conv, new_wl_ctxt_conv;
    RDD_MULTICAST_WHITELIST_CONTEXT_ENTRY_DTS *old_wl_ctxt = &wl_ctxt_conv.mcast_wl_ctxt;
    
#ifdef FIRMWARE_LITTLE_ENDIAN
    SWAPBYTES(((uint8_t *)mcast_wl_entry), sizeof(RDD_MULTICAST_WHITELIST_LKP_ENTRY_DTS));
#endif
    mcast_wl_ctxt->next_entry_idx = 0xffff;
    mcast_wl_ctxt->prev_entry_idx = 0xffff;

    rc = drv_natc_key_idx_get(multicast_wlist_tbl_idx, (uint8_t *)mcast_wl_entry,
                              &hash_idx, entry_idx);
    if (rc == BDMF_ERR_ALREADY)
    {
        /* a previous entry has been added with matching dst_ip in SSM case.
         * (suppose ASM will not mix with SSM. or else fundamentally it won't work
         * we need to create a new entry with src_ip + vlan in LKP entry.  Then traverse
         * through next_entry_idx to find the available next_entry_idx */

        drv_natc_result_entry_get(multicast_wlist_tbl_idx, *entry_idx, (uint8_t *)&wl_ctxt_conv);

#ifdef FIRMWARE_LITTLE_ENDIAN
        /* Swap everything back to host order */
        SWAPBYTES(((uint8_t *)old_wl_ctxt), sizeof(RDD_MULTICAST_WHITELIST_CONTEXT_ENTRY_DTS));
        SWAPBYTES(((uint8_t *)mcast_wl_entry), sizeof(RDD_MULTICAST_WHITELIST_LKP_ENTRY_DTS));
#endif
        next_idx = (uint32_t)old_wl_ctxt->next_entry_idx;
        curr_idx = *entry_idx;
        old_wl_ctxt->next_entry_idx = 0xffff;
        old_wl_ctxt->natc_control = 0;

        /* same entry is being added */
        if (memcmp(old_wl_ctxt, mcast_wl_ctxt, sizeof(RDD_MULTICAST_WHITELIST_CONTEXT_ENTRY_DTS)) == 0)
            return BDMF_ERR_ALREADY;

        /* now add a new NATC entry by having all the VLANs + SRC_IP fields filled.
         * those fields are all in context */
        mcast_wl_entry->vlan_0_union = mcast_wl_ctxt->vlan_0_union;
        mcast_wl_entry->vlan_1_union = mcast_wl_ctxt->vlan_1_union;
        mcast_wl_entry->num_of_vlans = mcast_wl_ctxt->num_of_vlans;
        if (mcast_wl_entry->is_ipv6 == 1)
            mcast_wl_entry->src_ip = rdd_crc_buf_calc_crc32(mcast_wl_ctxt->src_ip, 16);
        else
            mcast_wl_entry->src_ip = *((uint32_t *)mcast_wl_ctxt->src_ip);

        /* entry is added to natc.. now link it */
        /* first locate the last entry in the chain */
        while (next_idx != 0xffff)
        {
            rc = drv_natc_result_entry_get(multicast_wlist_tbl_idx, next_idx, (uint8_t *)&wl_ctxt_conv);
            if (rc)
                return rc;

            curr_idx = next_idx;
#ifdef FIRMWARE_LITTLE_ENDIAN
            /* swap to host order */
            SWAPBYTES(((uint8_t *)old_wl_ctxt), sizeof(RDD_MULTICAST_WHITELIST_CONTEXT_ENTRY_DTS));
#endif
            next_idx = (uint32_t)old_wl_ctxt->next_entry_idx;
        }

        mcast_wl_ctxt->prev_entry_idx = (uint16_t)curr_idx;
        memcpy(&new_wl_ctxt_conv.mcast_wl_ctxt, mcast_wl_ctxt, sizeof(RDD_MULTICAST_WHITELIST_CONTEXT_ENTRY_DTS));
        new_wl_ctxt_conv.natc_control = 0;
#ifdef FIRMWARE_LITTLE_ENDIAN
        /* swap new entry to Runner's order (big endian) */
        SWAPBYTES(((uint8_t *)mcast_wl_entry), sizeof(RDD_MULTICAST_WHITELIST_LKP_ENTRY_DTS));
        SWAPBYTES(((uint8_t *)&new_wl_ctxt_conv.mcast_wl_ctxt), sizeof(RDD_MULTICAST_WHITELIST_CONTEXT_ENTRY_DTS));
#endif
        rc = drv_natc_key_result_entry_add(multicast_wlist_tbl_idx,
                                           (uint8_t *)mcast_wl_entry,
                                           (uint8_t *)&new_wl_ctxt_conv, entry_idx);
        if (rc)
            return rc;


        old_wl_ctxt->next_entry_idx = (uint16_t)*entry_idx;
#ifdef FIRMWARE_LITTLE_ENDIAN
        SWAPBYTES(((uint8_t *)old_wl_ctxt), sizeof(RDD_MULTICAST_WHITELIST_CONTEXT_ENTRY_DTS));
#endif

        /* new entry is added, we will update the result to the curr_idx entry. 
         * WARNING: NatCache caching ability has to be turned off for this
         * multicast_whitelist table, or else, there is a potential issue that
         * the first entry of the chain might be cached in NatCache, then the
         * newly added entry will not be hit, because context cached in NATCache
         * does not have proper next_idx updated */
        rc = drv_natc_result_entry_add(multicast_wlist_tbl_idx, curr_idx,
                                       (uint8_t *)&wl_ctxt_conv);
        if (unlikely(rc))
            drv_natc_entry_delete(multicast_wlist_tbl_idx, *entry_idx, 1, 1);
    }
    else
    {
        /* no previous entry has been added with matching dst_ip */
        mcast_wl_ctxt->next_entry_idx = 0xffff;
        memcpy(&new_wl_ctxt_conv.mcast_wl_ctxt, mcast_wl_ctxt, sizeof(RDD_MULTICAST_WHITELIST_CONTEXT_ENTRY_DTS));
        new_wl_ctxt_conv.natc_control = 0;
#ifdef FIRMWARE_LITTLE_ENDIAN
        SWAPBYTES(((uint8_t *)&new_wl_ctxt_conv.mcast_wl_ctxt), sizeof(RDD_MULTICAST_WHITELIST_CONTEXT_ENTRY_DTS));
#endif
        rc = drv_natc_key_result_entry_add(multicast_wlist_tbl_idx,
                                           (uint8_t *)mcast_wl_entry,
                                           (uint8_t *)&new_wl_ctxt_conv, entry_idx);
    }

    return rc;
}

int rdd_multicast_whitelist_entry_find(RDD_MULTICAST_WHITELIST_LKP_ENTRY_DTS *mcast_wl_entry,
                                       RDD_MULTICAST_WHITELIST_CONTEXT_ENTRY_DTS *mcast_wl_ctxt,
                                       uint32_t *entry_idx)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    uint32_t hash_idx;
    multicast_whitelist_context_entry_t wl_ctxt_conv;
    RDD_MULTICAST_WHITELIST_CONTEXT_ENTRY_DTS *old_wl_ctxt = &wl_ctxt_conv.mcast_wl_ctxt;

#ifdef FIRMWARE_LITTLE_ENDIAN
    SWAPBYTES(((uint8_t *)mcast_wl_entry), sizeof(RDD_MULTICAST_WHITELIST_LKP_ENTRY_DTS));
#endif

    rc = drv_natc_key_idx_get(multicast_wlist_tbl_idx, (uint8_t *)mcast_wl_entry,
                              &hash_idx, entry_idx);
    /* we are attempting to find a matched entry, which the above function will
     * error code if found */
    if (rc == BDMF_ERR_ALREADY)
    {
        drv_natc_result_entry_get(multicast_wlist_tbl_idx, *entry_idx, (uint8_t *)&wl_ctxt_conv);
#ifdef FIRMWARE_LITTLE_ENDIAN
        /* Swap everything back to host order */
        SWAPBYTES(((uint8_t *)old_wl_ctxt), sizeof(RDD_MULTICAST_WHITELIST_CONTEXT_ENTRY_DTS));
        SWAPBYTES(((uint8_t *)mcast_wl_entry), sizeof(RDD_MULTICAST_WHITELIST_LKP_ENTRY_DTS));
#endif
        old_wl_ctxt->next_entry_idx = 0;
        old_wl_ctxt->prev_entry_idx = 0;
        old_wl_ctxt->natc_control = 0;

        /* see if the first entry of the chain is a match */
        if (memcmp(old_wl_ctxt, mcast_wl_ctxt, sizeof(RDD_MULTICAST_WHITELIST_CONTEXT_ENTRY_DTS)) == 0)
        {
            *entry_idx = __rdd_multicast_whitelist_index_bkwd_xlate(*entry_idx);
            return BDMF_ERR_OK;
        }

        /* if get to this point, that means the first entry in the chain does not match.
         * there are 2 approaches that we can do to search:
         * 1) look up using the complete key. or
         * 2) traverse through the linked list to find it.
         * The current implementation uses 1) */
        mcast_wl_entry->vlan_0_union = mcast_wl_ctxt->vlan_0_union;
        mcast_wl_entry->vlan_1_union = mcast_wl_ctxt->vlan_1_union;
        mcast_wl_entry->num_of_vlans = mcast_wl_ctxt->num_of_vlans;
        if (mcast_wl_entry->is_ipv6 == 1)
            mcast_wl_entry->src_ip = rdd_crc_buf_calc_crc32(mcast_wl_ctxt->src_ip, 16);
        else
            mcast_wl_entry->src_ip = *((uint32_t *)mcast_wl_ctxt->src_ip);

#ifdef FIRMWARE_LITTLE_ENDIAN
        SWAPBYTES(((uint8_t *)mcast_wl_entry), sizeof(RDD_MULTICAST_WHITELIST_LKP_ENTRY_DTS));
#endif

        rc = drv_natc_key_idx_get(multicast_wlist_tbl_idx, (uint8_t *)mcast_wl_entry,
                                  &hash_idx, entry_idx);
        if (rc == BDMF_ERR_ALREADY)
            return BDMF_ERR_OK;
    }

    /* everything else means it is not found */
    return BDMF_ERR_NOENT;
}

int rdd_multicast_whitelist_entry_get(uint32_t entry_idx,
                                      RDD_MULTICAST_WHITELIST_LKP_ENTRY_DTS *mcast_wl_entry,
                                      RDD_MULTICAST_WHITELIST_CONTEXT_ENTRY_DTS *mcast_wl_ctxt)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    bdmf_boolean valid = 0;
    multicast_whitelist_context_entry_t wl_ctxt_conv;

    /* this is to handle the case where the first entry of the chain might have
     * been deleted. and the 2nd entry has been moved up to the first with index
     * changed, but up there the caller still refer it with the old entry index.
     * So we need to return NOENT for the original first_entry_idx and possibly
     * translate the index */
    if (__rdd_multicast_whitelist_index_xlate_check_first_idx(entry_idx))
        return BDMF_ERR_NOENT;

    entry_idx =  __rdd_multicast_whitelist_index_fwd_xlate(entry_idx);

    rc = drv_natc_key_entry_get(multicast_wlist_tbl_idx, entry_idx, &valid, (uint8_t *)mcast_wl_entry);

    if (!valid)
        rc = BDMF_ERR_NOENT;

    drv_natc_result_entry_get(multicast_wlist_tbl_idx, entry_idx, (uint8_t *)&wl_ctxt_conv);
    memcpy(mcast_wl_ctxt, &wl_ctxt_conv.mcast_wl_ctxt, sizeof(RDD_MULTICAST_WHITELIST_CONTEXT_ENTRY_DTS));

#ifdef FIRMWARE_LITTLE_ENDIAN
    SWAPBYTES(((uint8_t *)mcast_wl_entry), sizeof(RDD_MULTICAST_WHITELIST_LKP_ENTRY_DTS));
    SWAPBYTES(((uint8_t *)mcast_wl_ctxt), sizeof(RDD_MULTICAST_WHITELIST_CONTEXT_ENTRY_DTS));
#endif

    return rc;
}

int rdd_multicast_whitelist_delete(uint32_t entry_idx)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    multicast_whitelist_context_entry_t wl_ctxt_conv, prev_wl_ctxt_conv, next_wl_ctxt_conv;
    RDD_MULTICAST_WHITELIST_CONTEXT_ENTRY_DTS *wl_ctxt = &wl_ctxt_conv.mcast_wl_ctxt;
    RDD_MULTICAST_WHITELIST_CONTEXT_ENTRY_DTS *prev_wl_ctxt = &prev_wl_ctxt_conv.mcast_wl_ctxt;
    RDD_MULTICAST_WHITELIST_CONTEXT_ENTRY_DTS *next_wl_ctxt = &next_wl_ctxt_conv.mcast_wl_ctxt;

    /* this is to handle the case where the first entry of the chain might have
     * been deleted. and the 2nd entry has been moved up to the first with index
     * changed, but up there the caller still refer it with the old entry index.
     * So we need to return NOENT for the original first_entry_idx and possibly
     * translate the index */
    if (__rdd_multicast_whitelist_index_xlate_check_first_idx(entry_idx))
        return BDMF_ERR_NOENT;

    entry_idx =  __rdd_multicast_whitelist_index_xlate_pop(entry_idx);

    rc = drv_natc_result_entry_get(multicast_wlist_tbl_idx, entry_idx, (uint8_t *)&wl_ctxt_conv);
    if (rc)
        return rc;
#ifdef FIRMWARE_LITTLE_ENDIAN
    SWAPBYTES(((uint8_t *)wl_ctxt), sizeof(RDD_MULTICAST_WHITELIST_CONTEXT_ENTRY_DTS));
#endif

    /* there are cases:
     * 1) the entry is the only entry of the chain (prev_idx = next_idx = 0xffff)
     *    => We can simply delete this entry
     * 2) the entry is the first entry of the chain and there are more than 2 entries
     *    in the chain (prev_idx = 0xffff, but next_idx != 0xffff)
     *    => We will need to update the context of this entry to the next entry
     *       and the entry after, then delete the next entry.
     * 3) the entry is in the middle/last of the chain.
     *    => We will need to update prev/next entry and then delete this one.
     */

    /* case#1 */
    if ((wl_ctxt->prev_entry_idx == 0xffff) &&
        (wl_ctxt->next_entry_idx == 0xffff))
        goto exit_go_free;

    /* case#2 */
    if ((wl_ctxt->prev_entry_idx == 0xffff) &&
        (wl_ctxt->next_entry_idx != 0xffff))
    {
        rc = drv_natc_result_entry_get(multicast_wlist_tbl_idx,
                                       wl_ctxt->next_entry_idx,
                                       (uint8_t *)&next_wl_ctxt_conv);
        if (rc)
            return rc;
#ifdef FIRMWARE_LITTLE_ENDIAN
        SWAPBYTES(((uint8_t *)next_wl_ctxt), sizeof(RDD_MULTICAST_WHITELIST_CONTEXT_ENTRY_DTS));
#endif
        if (next_wl_ctxt->next_entry_idx != 0xfff)
        {
            rc = drv_natc_result_entry_get(multicast_wlist_tbl_idx,
                                           next_wl_ctxt->next_entry_idx,
                                           (uint8_t *)&prev_wl_ctxt_conv);
            if (rc)
                return rc;
#ifdef FIRMWARE_LITTLE_ENDIAN
            SWAPBYTES(((uint8_t *)prev_wl_ctxt), sizeof(RDD_MULTICAST_WHITELIST_CONTEXT_ENTRY_DTS));
#endif
            prev_wl_ctxt->prev_entry_idx = entry_idx;
#ifdef FIRMWARE_LITTLE_ENDIAN
            SWAPBYTES(((uint8_t *)prev_wl_ctxt), sizeof(RDD_MULTICAST_WHITELIST_CONTEXT_ENTRY_DTS));
#endif
            rc = drv_natc_result_entry_add(multicast_wlist_tbl_idx,
                                           next_wl_ctxt->next_entry_idx,
                                           (uint8_t *)&prev_wl_ctxt_conv);
            if (unlikely(rc))
                return rc;
        }
        /* update the next_entry to the current location */
        next_wl_ctxt->prev_entry_idx = 0xffff;
#ifdef FIRMWARE_LITTLE_ENDIAN
        SWAPBYTES(((uint8_t *)next_wl_ctxt), sizeof(RDD_MULTICAST_WHITELIST_CONTEXT_ENTRY_DTS));
#endif
        rc = drv_natc_result_entry_add(multicast_wlist_tbl_idx,
                                       entry_idx,
                                       (uint8_t *)&next_wl_ctxt_conv);
        if (unlikely(rc))
            return rc;

        /* keep the next_entry_idx -> curr_idx table, because the upper level will refer
         * the original next_entry with the its old idx, but it's been changed */
        __rdd_multicast_whitelist_index_xlate_add(wl_ctxt->next_entry_idx, entry_idx);

        /* free the next entry rather than the current head entry */
        entry_idx = wl_ctxt->next_entry_idx;
        goto exit_go_free;
    }

    /* case#3 */
    /* update previous entry */
    rc = drv_natc_result_entry_get(multicast_wlist_tbl_idx,
                                   wl_ctxt->prev_entry_idx,
                                   (uint8_t *)&prev_wl_ctxt_conv);
    if (unlikely(rc))
        return rc;

#ifdef FIRMWARE_LITTLE_ENDIAN
    SWAPBYTES(((uint8_t *)prev_wl_ctxt), sizeof(RDD_MULTICAST_WHITELIST_CONTEXT_ENTRY_DTS));
#endif
    prev_wl_ctxt->next_entry_idx = wl_ctxt->next_entry_idx;
#ifdef FIRMWARE_LITTLE_ENDIAN
    SWAPBYTES(((uint8_t *)prev_wl_ctxt), sizeof(RDD_MULTICAST_WHITELIST_CONTEXT_ENTRY_DTS));
#endif
    rc = drv_natc_result_entry_add(multicast_wlist_tbl_idx,
                                   wl_ctxt->prev_entry_idx,
                                   (uint8_t *)&prev_wl_ctxt_conv);
    if (unlikely(rc))
        return rc;

    if (wl_ctxt->next_entry_idx != 0xffff)
    {
        rc = drv_natc_result_entry_get(multicast_wlist_tbl_idx,
                                       wl_ctxt->next_entry_idx,
                                       (uint8_t *)&next_wl_ctxt_conv);
        if (unlikely(rc))
            return rc;

#ifdef FIRMWARE_LITTLE_ENDIAN
        SWAPBYTES(((uint8_t *)next_wl_ctxt), sizeof(RDD_MULTICAST_WHITELIST_CONTEXT_ENTRY_DTS));
#endif
        next_wl_ctxt->prev_entry_idx = wl_ctxt->prev_entry_idx;
#ifdef FIRMWARE_LITTLE_ENDIAN
        SWAPBYTES(((uint8_t *)next_wl_ctxt), sizeof(RDD_MULTICAST_WHITELIST_CONTEXT_ENTRY_DTS));
#endif
        rc = drv_natc_result_entry_add(multicast_wlist_tbl_idx,
                                       wl_ctxt->next_entry_idx,
                                       (uint8_t *)&next_wl_ctxt_conv);
        if (unlikely(rc))
            return rc;
    }

exit_go_free:

    rc = drv_natc_entry_delete(multicast_wlist_tbl_idx, entry_idx, 1, 1);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Multicast Whitelist deletion failed, error=%d\n", rc);

    return rc;
}

int rdd_multicast_whitelist_enable_port_mask_set(uint32_t port_mask)
{
    RDD_BYTES_4_BITS_WRITE_G(port_mask, RDD_ENABLE_VPORT_MASK_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_multicast_whitelist_enable_port_mask_get(uint32_t *port_mask)
{
    RDD_BYTES_4_BITS_READ_G(*port_mask, RDD_ENABLE_VPORT_MASK_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_multicast_whitelist_counter_get(uint32_t entry_idx, uint64_t *packets_cnt,
                                        uint64_t *bytes_cnt)
{
    RDD_MULTICAST_WHITELIST_LKP_ENTRY_DTS mcast_wl_entry;
    bdmf_boolean valid = 0;

    /* this is to handle the case where the first entry of the chain might have
     * been deleted. and the 2nd entry has been moved up to the first with index
     * changed, but up there the caller still refer it with the old entry index.
     * So we need to return NOENT for the original first_entry_idx and possibly
     * translate the index */
    if (__rdd_multicast_whitelist_index_xlate_check_first_idx(entry_idx))
        return BDMF_ERR_NOENT;

    entry_idx =  __rdd_multicast_whitelist_index_fwd_xlate(entry_idx);

    drv_natc_key_entry_get(multicast_wlist_tbl_idx, entry_idx, &valid,
                           (uint8_t *)&mcast_wl_entry);
    if (!valid)
        return BDMF_ERR_NOENT;

    /* TODO!! implement a more thourough implementation where the first entry
     * in the chain might have all the count */
    /* in the current whitelist chainlist. NATC hit always incurs on the first
     * entry of the chain. the later entries do not have their stats updated
     * by NATC.  One thing that we can do is, in Runner firmware, after we
     * know there is a hit in later entries in the list, we create a dummy NATC
     * hit in firmware, so the counter gets incremented. then we can read the
     * stat here, but do need to subtract later entries' stat from first entry's
     * stat when getting counter for first entry. And supposedly, we usually
     * delete a chain of entries, or else we will have some corner cases where
     * only 1 entry in the chain is deleted, and we need to account the stat properly */
    return drv_natc_entry_counters_get(multicast_wlist_tbl_idx, entry_idx,
                                       packets_cnt, bytes_cnt);
}

