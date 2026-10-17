/*
	<:copyright-BRCM:2024:DUAL/GPL:standard

	Copyright (c) 2024 Broadcom
	All Rights Reserved

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

#include <linux/nbuff.h>
#include <linux/wkbuff.h>
#include <linux/gbpm.h>

/**
 * -----------------------------------------------------------------------------
 * Section: WKB Control Abstract Data Type
 * -----------------------------------------------------------------------------
 */

/**
 * Function     : wkb_init
 * Description  : Prepares a WKB packet context
 *
 * pStart - Start of Buffer (pStart)
 */
wkbuff_t *
wkb_init(void * pStart, uint8_t * data, uint16_t len, uint8_t prio,
        wkb_db_pool_t db_pool, wkb_se_t wkb_se, bool need_reset)
{
    wkbuff_t * wkb;

    wkb = BCM_PSTART_TO_PWKBUFF(pStart);

    if ((len > (WKB_DB_HDRM_BYTES + WKB_DB_PYLD_BYTES)) ||
        (((uintptr_t)data + len) >= (uintptr_t)wkb_end(wkb))) {

        WKB_ERROR("FAILED. Invalid Databuffer\n");
        WKB_ERROR("pStart " WKB_ADDR_FMT " data " WKB_ADDR_FMT " len %d\n",
            WKB_ADDR_VAL(pStart), WKB_ADDR_VAL(data), len);

        WKB_ASSERT(0);
        return NULL;
    }

    if (need_reset) {
        memset(wkb, 0, WKB_CTXT_BYTES);
    }

    /* Initialize WKB control SW section */
    wkb_set_data(wkb, data);
    wkb_set_len(wkb, len);
    wkb_set_prio(wkb, prio);

    wkb->flags = WKB_FLAGS_SET_SE(wkb_se);

    /* Point WKB frag "0" to self */
    wkb_frag_fill_item(wkb, pStart, NULL, 0, db_pool, false);

    WKB_PTRACE("SUCCESS wkb " WKB_ADDR_FMT " pStart " WKB_ADDR_FMT
        " data " WKB_ADDR_FMT " len %d prio %d db_pool %d\n",
        WKB_ADDR_VAL(wkb), WKB_ADDR_VAL(pStart), WKB_ADDR_VAL(data),
        len, prio, db_pool);

    return wkb;

} /* wkb_init() */

EXPORT_SYMBOL(wkb_init);


/**
 * Function     : wkb_alloc
 * Description  : Allocates a BPM buffer and prepares WKB
 */
wkbuff_t *
wkb_alloc(uint16_t len, wkb_se_t wkb_se)
{
    wkbuff_t    * wkb;
    void        * pStart;
    void        * pdata;
    wkb_db_pool_t db_pool;

    WKB_ASSERT(len <= (WKB_DB_HDRM_BYTES + WKB_DB_PYLD_BYTES));

    pdata = gbpm_alloc_buf();
    if (pdata == NULL) {
        WKB_TRACE("BPM buf alloc failed, len %d\n", len);
        return NULL;
    }

    pStart = BCM_PDATA_TO_PSTART(pdata);
    if (gbpm_is_buf_hw_recycle_capable(pStart))
        db_pool = WKB_DB_MPM;
    else
        db_pool = WKB_DB_BPM;

    if (len > WKB_DB_PYLD_BYTES) {
        /* If requested bytes are more than payload (2K) size (WLAN mgmt frames)
         * then use headroom region also.
         * No headroom will be reserved in this case.
          */
        wkb = wkb_init(pStart, BCM_PDATA_TO_PHEAD(pdata), len, 0,
                db_pool, wkb_se, true);
    } else {
        wkb = wkb_init(pStart, pdata, len, 0, db_pool, wkb_se, true);
    }

    return wkb;
} /* wkb_alloc() */

EXPORT_SYMBOL(wkb_alloc);


/* Helper function to free/recycle data buffer */
static void
wkb_db_free(wkbuff_t * wkb, void * db_addr, wkb_db_pool_t db_pool)
{
    pNBuff_t    pNBuff;

    WKB_PTRACE("wkb " WKB_ADDR_FMT " db_addr " WKB_ADDR_FMT " db_pool %d\n",
        WKB_ADDR_VAL(wkb), WKB_ADDR_VAL(db_addr), db_pool);

    switch (db_pool) {

        case WKB_DB_MPM:
            gbpm_recycle_hw_buf(db_addr, 0);
            break;

        case WKB_DB_BPM:
            gbpm_free_buf(BCM_PSTART_TO_PDATA(db_addr));
            break;

        case WKB_DB_LNX:
            pNBuff = (pNBuff_t)wkb_pkt_ref(wkb);

            WKB_BUG(pNBuff);
            WKB_BUG(wkb->frag_num == (WKB_LNX_FRAG_IDX + 1));
            WKB_ASSERT(!IS_WKBUFF_PTR(pNBuff));

            /* Free up pNBuff linked to WKB */
            nbuff_free(pNBuff);
            break;

        default:
            WKB_ERROR("Invalid WKB pool %d\n", db_pool);
            WKB_BUG(0);
            break;
    }

} /* wkb_db_free */


/**
 * Function     : wkb_free
 * Description  : Free an WKB and all its fragments.
 *                Based on the wkb_db_pool_t, data buffers are recycled to the
 *                respective pools (MPM/BPM/KMEMCACHE)
 */
void
wkb_free(wkbuff_t * wkb)
{
    wkbuff_t    * next;
    void        * db_addr;
    int         frag_idx;
    wkb_db_pool_t db_pool;

    WKB_PTRACE("wkb " WKB_ADDR_FMT "\n", WKB_ADDR_VAL(wkb));

    WKB_ASSERT(wkb_link(wkb) == NULL);

    while (wkb != NULL) {
        next = PNBUFF_2_WKBUFF(wkb_next(wkb));

        WKB_ASSERT(wkb->frag_num <= WKB_FRAGCNT_MAX);

        /* Free up frag buffers first */
        for (frag_idx = wkb->frag_num - 1; frag_idx > 0; frag_idx--) {
            db_addr = wkb_frag_db_addr(wkb, frag_idx);
            db_pool = wkb->frag_key[frag_idx].db_pool;

            if ((wkb->frag_key[frag_idx].db_pool == WKB_DB_LNX) ||
                    (db_addr != NULL)) {
                wkb_db_free(wkb, db_addr, db_pool);
            }
        }

        /* Reset WKB */
        wkb->frag_num = 0;
        wkb->frag_flags = 0;

        /* Free up frag "0" buffer in which WKB context is embedded */
        db_addr = wkb_db_addr(wkb);
        db_pool = wkb->frag_key[frag_idx].db_pool;

        WKB_ASSERT(db_pool != WKB_DB_LNX);

        wkb_db_free(wkb, db_addr, db_pool);

        wkb = next;
    }

} /* wkb_free */

EXPORT_SYMBOL(wkb_free);


/**
 * Function     : wkb_db_addr
 * Description  : Returns Data buffer start address in which given WKB is embedded
*/
void *
wkb_db_addr(wkbuff_t * wkb)
{
    uintptr_t   db_addr;

    db_addr = (uintptr_t)BCM_PWKBUFF_TO_PSTART(wkb);

    return (void *)db_addr;
} /* wkb_db_addr() */

EXPORT_SYMBOL(wkb_db_addr);


/**
 * Function     : wkb_totlen
 * Description  : Returns the total length of PDU
 * Summation of all WKB SDUs linked using next
 */
uint16_t
wkb_totlen(wkbuff_t * wkb)
{
    uint16_t totlen = 0;

    while (wkb) {
        totlen += wkb_len(wkb);
        totlen += wkb->frag_totlen;

        wkb = PNBUFF_2_WKBUFF(wkb_next(wkb));
    }

    return totlen;
} /* wkb_totlen() */

EXPORT_SYMBOL(wkb_totlen);


/**
 * Function     : wkb_realloc_headroom
 * Description  : Move the data buffers of pNBUFF (skb) linked WKBs
 *
 * SKB data buffer prepared by Linux Network Stack will not have enough
 * headroom for WLAN headers so the data will be fragmented
 * - WKB data will point to its own buffer (used for 802.11 Hdr)
 * - LNX frag will point to the SKB data buffer (payload)
 */
bool
wkb_realloc_headroom(wkbuff_t * wkb, int headroom)
{
    uint8_t     * data;
    uint16_t    frag_idx;

    WKB_PTRACE("WKB " WKB_ADDR_FMT " data " WKB_ADDR_FMT " len %d headroom %d\n",
            WKB_ADDR_VAL(wkb), WKB_ADDR_VAL(wkb_data(wkb)),
            wkb_len(wkb), headroom);

    frag_idx = WKB_LNX_FRAG_IDX;

    if ((wkb_pkt_ref(wkb) == NULL) ||
        (wkb_frag_data(wkb, frag_idx) != NULL)) {

        /* Nothing to do if
         * - pNBuff (skb) is not linked to WKB.
         * - SKB data buffer is already moved
         *
         * WKB data buffer should have enough headroom
         */
        WKB_ASSERT(wkb_headroom(wkb) >= headroom);
        return false;
    }

    /* Supports only SKB */
    WKB_ASSERT(IS_SKBUFF_PTR(wkb_pkt_ref(wkb)));

    /* Move SKB buffer (payload) to LNX frag_idx */

    wkb_set_frag_len(wkb, frag_idx, wkb_len(wkb));
    wkb_set_frag_data(wkb, frag_idx, wkb_data(wkb));

    /* Prepare WKB buffer to carry the headers (802.11) */
    data = BCM_PHEAD_TO_PDATA(wkb_head(wkb));
    wkb_set_data(wkb, data);
    wkb_set_len(wkb, headroom);
    wkb_pull(wkb, headroom);

    WKB_PTRACE("WKB " WKB_ADDR_FMT " data " WKB_ADDR_FMT " len %d\n"
            "LNX frag: idx %d data " WKB_ADDR_FMT " len %d\n",
            WKB_ADDR_VAL(wkb), WKB_ADDR_VAL(wkb_data(wkb)), wkb_len(wkb),
            frag_idx, WKB_ADDR_VAL(wkb_frag_data(wkb, frag_idx)),
            wkb_frag_len(wkb, frag_idx));

    return true;
} /* wkb_realloc_headroom() */

EXPORT_SYMBOL(wkb_realloc_headroom);


/**
 * Function     : wkb_copy
 * Description  : Create a copy of both wkbuff and its data buffer
 */
wkbuff_t *
wkb_copy(wkbuff_t * wkb)
{
    wkbuff_t    * wkb_dup;
    wkb_se_t    wkb_se;
    uint16_t    len;

    len     = WKB_DB_HDRM_BYTES + WKB_DB_PYLD_BYTES;
    wkb_se  = WKB_FLAGS_GET_SE(wkb->flags);

    /* SPC, fragmented and chained packets are not supported */
    WKB_ASSERT(wkb->frag_num == 1);
    WKB_ASSERT(wkb_frag_data(wkb, 0) == NULL);;
    WKB_ASSERT(wkb_next(wkb) == NULL);
    WKB_ASSERT(wkb_link(wkb) == NULL);

    wkb_dup = wkb_alloc(len, wkb_se);
    if (wkb_dup == NULL) {
        WKB_ERROR(" WKB alloc failed; len %d\n", len);
        return NULL;
    }

    /* Reserve Headroom */
    wkb_pull(wkb_dup, wkb_headroom(wkb));
    wkb_set_len(wkb_dup, wkb_len(wkb));

    /* Copy the data bytes */
    memcpy(wkb_data(wkb_dup), wkb_data(wkb), wkb_len(wkb));

    /* Copy the WKB flags */
    wkb_set_prio(wkb_dup, wkb_prio(wkb));

    /* Inherit the Source Entity of parent WKB */
    wkb_dup->flags = wkb->flags & WKB_FLAGS_SE_MASK;

    /* Will NOT touch Ctrlhw and Extnsw */

    return wkb_dup;
} /* wkb_copy() */

EXPORT_SYMBOL(wkb_copy);


/**
 * Function     : wkb_release_pkt_ref
 * Description  : Release the pNBuff linked to WKB and
 * reset all references to the pNBUFF data buffer
 */
void *
wkb_release_pkt_ref(wkbuff_t * wkb)
{
    void        * pkt_ref;
    uint16_t    frag_idx;

    frag_idx    = WKB_LNX_FRAG_IDX;
    pkt_ref     = wkb_pkt_ref(wkb);

    WKB_ASSERT(wkb->frag_num == WKB_LNX_FRAG_IDX + 1);
    WKB_ASSERT(pkt_ref != NULL);

    WKB_PTRACE("WKB " WKB_ADDR_FMT " data " WKB_ADDR_FMT " len %d "
            "pkt_ref " WKB_ADDR_FMT "\n"
            "LNX frag: idx %d data " WKB_ADDR_FMT " len %d\n",
            WKB_ADDR_VAL(wkb), WKB_ADDR_VAL(wkb_data(wkb)), wkb_len(wkb),
            WKB_ADDR_VAL(pkt_ref), frag_idx,
            WKB_ADDR_VAL(wkb_frag_data(wkb, frag_idx)),
            wkb_frag_len(wkb, frag_idx));
 
    /* Reset the references to pkt_ref */
    wkb->frag_key[frag_idx].key16 = 0;
    wkb_set_pkt_ref(wkb, NULL);

    if (wkb_frag_data(wkb, frag_idx) != NULL) {
        /* XXX: Not verified this use case */
        WKB_ASSERT(0);
        wkb_set_frag_len(wkb, frag_idx, 0);
        wkb_set_frag_data(wkb, frag_idx, NULL);
    } else {
        /* We hit this case before adjusting SKB data buffer
         * i.e before invoking wkb_realloc_headroom()
         */
        WKB_ASSERT(wkb_frag_len(wkb, frag_idx) == 0);
        wkb_set_data(wkb, NULL);
        wkb_set_len(wkb, 0);
    }

    /* Update Fragment control word */
    wkb->frag_num--;
    wkb->frag_flags &= ~(1 << frag_idx);

    return pkt_ref;

} /* wkb_release_pkt_ref() */

EXPORT_SYMBOL(wkb_release_pkt_ref);


/**
 * -----------------------------------------------------------------------------
 * Section: WKB Fragmem Abstract Data Type
 * -----------------------------------------------------------------------------
 */

/**
 * Function     : wkb_align_prev_frag
 * Description  : Align the previous last fragment data to 4Bytes
 */
static void
wkb_align_prev_frag(wkbuff_t * wkb)
{
    uint16_t     frag_idx;

    if (wkb->frag_num == 0) {
        /* Nothing to do */
    } else if ((wkb->frag_num == 1) && (wkb_data(wkb) != NULL)) {
        /* Align WKB databuffer */
        if (!IS_ALIGNED(wkb_len(wkb), WKB_FRAG_LEN_ALIGN)) {
            wkb_set_len(wkb, ALIGN(wkb_len(wkb), WKB_FRAG_LEN_ALIGN));
        }
    } else {
        /* Align WKB non-first fragment */
        uint16_t frag_len;

        frag_idx = wkb->frag_num - 1;
        frag_len = wkb_frag_len(wkb, frag_idx);

        if (!IS_ALIGNED(frag_len, WKB_FRAG_LEN_ALIGN)) {
            wkb_set_frag_len(wkb, frag_idx,
                ALIGN(frag_len, WKB_FRAG_LEN_ALIGN));
        }
    }
} /* wkb_align_prev_frag() */

/**
 * Function     : wkb_frag_fill_item
 * Description  : Fills the fragment at index wkb::fragnum in a WKB
 *
 * The function will also align the previous last fragment data to 4Bytes
 */
int
wkb_frag_fill_item(wkbuff_t * wkb, void * pStart, uint8_t * data,
    uint16_t len, wkb_db_pool_t db_pool, bool align)
{
    wkbuff_t * wkb_ldr = wkb;
    uint16_t     frag_idx;

    WKB_ASSERT(db_pool < WKB_DB_MAX);
    WKB_ASSERT((db_pool != WKB_DB_LNX) || (wkb->frag_num == WKB_LNX_FRAG_IDX));

    if (wkb_ldr->frag_num >= WKB_FRAGCNT_MAX) {

        wkb = PNBUFF_2_WKBUFF(wkb_next(wkb_ldr));

        if (wkb == NULL) {
            /* Prepare a FTE (Fragment Table Extension) WKB */
            wkb = wkb_init(pStart, data, len, wkb_prio(wkb_ldr), db_pool,
                WKB_FLAGS_GET_SE(wkb_ldr->flags), true);
            wkb_set_next(wkb_ldr, WKBUFF_2_PNBUFF(wkb));

            if (align) {
                wkb_align_prev_frag(wkb_ldr);
            }

            return WKB_SUCCESS;
        } else {
            /* Max 16 MSDUs are supported */
            WKB_ASSERT(wkb_next(wkb) == NULL);

            if (wkb->frag_num > WKB_FRAGCNT_MAX) {
                WKB_ERROR("WKB Max fragments limit reached. frag_num %d\n",
                        wkb->frag_num);
                WKB_ASSERT(wkb->frag_num <= WKB_FRAGCNT_MAX);
                return WKB_FAILURE;
            }

            /* Continue to fill the frag in FTE */
        }
    }

    /* Align the previous MSDU fragment to 4Bytes */
    if (align) {
        wkb_align_prev_frag(wkb);
    }

    frag_idx = wkb->frag_num;
    wkb->frag_num++;

    return wkb_frag_idx_fill_item(wkb, pStart, data, frag_idx, len, db_pool);

} /* wkb_frag_fill_item() */

EXPORT_SYMBOL(wkb_frag_fill_item);


 /**
 * Function     : wkb_frag_idx_fill_item
 * Description  : Fills the fragment at a given frag index
 */
int
wkb_frag_idx_fill_item(wkbuff_t * wkb, void * pStart, uint8_t * data,
    uint16_t frag_idx, uint16_t len, wkb_db_pool_t db_pool)
{
    wkb_db_key_t    frag_key;

    /* Prepare frag_key and fill fragmem section */
    frag_key.key16      = 0;
    frag_key.db_pool    = db_pool;
    /* Get lower 12bits of frag databuffer address */
    if (db_pool != WKB_DB_LNX) {
        frag_key.db_sob_lo12 = WKB_DB_ADDR_TO_DB_SOB_L012(pStart);
    }

    wkb->frag_key[frag_idx].key16   = frag_key.key16;
    wkb_set_frag_data(wkb, frag_idx, data);
    wkb_set_frag_len(wkb, frag_idx, len);

    if (db_pool != WKB_DB_MPM) {
        wkb->frag_flags |= (1 << frag_idx);
    }

    WKB_PTRACE("wkb " WKB_ADDR_FMT " frag: idx %d key16: 0x%x len %d \n"
        "\tbuf " WKB_ADDR_FMT " data " WKB_ADDR_FMT " frag_num %d totlen %d\n",
        WKB_ADDR_VAL(wkb), frag_idx, frag_key.key16, len,
        WKB_ADDR_VAL(pStart), WKB_ADDR_VAL(data), wkb->frag_num,
        wkb_totlen(wkb));

    return WKB_SUCCESS;

} /* wkb_frag_idx_fill_item() */

EXPORT_SYMBOL(wkb_frag_idx_fill_item);


/**
 * Function     : wkb_frag_db_addr
 * Description  : Given a WKB frag index, returns frag data buffer address
*/
void *
wkb_frag_db_addr(wkbuff_t * wkb, uint16_t frag_idx)
{
    wkb_db_key_t * db_key;
    void        * buf_addr;
    void        * frag_data;

    WKB_ASSERT(frag_idx < wkb->frag_num);

    frag_data   = wkb_frag_data(wkb, frag_idx);
    db_key      = &wkb->frag_key[frag_idx];

    if ((db_key->db_pool == WKB_DB_LNX) || (frag_data == NULL)) {
        WKB_PTRACE("Special Frag; frag_idx %d db_pool %d data " WKB_ADDR_FMT "\n",
            frag_idx, db_key->db_pool, WKB_ADDR_VAL(frag_data));
        return NULL;
    }

    /* Compute buffer address using data pointer and lower 12bits
     * of buffer address saved in wkb_db_key_t::db_sob_lo12
     */
    buf_addr = WKB_DB_GET_SOB(frag_data, db_key->db_sob_lo12);

    return buf_addr;
} /* wkb_frag_db_addr() */

EXPORT_SYMBOL(wkb_frag_db_addr);


/**
 * Function     : wkb_frags_release
 * Description  : Iterate through wkb frags and prepare sdu (wkb::next based) list
 * The API will not touch frag "0"
 */
int
wkb_frags_release(wkbuff_t * wkb)
{
    wkbuff_t    * wkb_fte;
    wkbuff_t    * frag;
    wkbuff_t    * prev;
    void        * pStart;
    uint8_t     * data;
    wkb_db_pool_t db_pool;
    wkb_se_t    wkb_se;
    uint16_t    len;
    uint8_t     frag_num;
    uint8_t     frag_idx;
    uint8_t     prio;
    uint8_t     frag_totcnt = 0;

    prio        = wkb_prio(wkb);
    wkb_se      = WKB_FLAGS_GET_SE(wkb->flags);

release_continue:

    wkb_fte     = PNBUFF_2_WKBUFF(wkb_next(wkb));
    wkb_set_next(wkb, NULL);
    frag_num    = wkb->frag_num;
    prev        = wkb;
    frag_totcnt += frag_num;

    WKB_ASSERT(frag_num <= WKB_FRAGCNT_MAX);

    for (frag_idx = 1; frag_idx < frag_num; frag_idx++) {
        pStart  = wkb_frag_db_addr(wkb, frag_idx);
        data    = wkb_frag_data(wkb, frag_idx);
        len     = wkb_frag_len(wkb, frag_idx);
        db_pool = wkb->frag_key[frag_idx].db_pool;

        /* Release the fragment from Head WKB */
        wkb->frag_key[frag_idx].key16 = 0;
        wkb_set_frag_data(wkb, frag_idx, NULL);
        wkb_set_frag_len(wkb, frag_idx, 0);

        wkb->frag_flags &= ~(1 << frag_idx);

        /* Prepare a new WKB for the fragment */
        frag = wkb_init(pStart, data, len, prio, db_pool, wkb_se, true);

        WKB_PTRACE("idx %d: frag " WKB_ADDR_FMT " data " WKB_ADDR_FMT " len %d\n",
                frag_idx, WKB_ADDR_VAL(frag), WKB_ADDR_VAL(wkb_data(frag)),
                wkb_len(frag));

       /* Chain the fragment using wkb::next */
        wkb_set_next(frag, NULL);
        wkb_set_next(prev, WKBUFF_2_PNBUFF(frag));

        prev = frag;
    }

    WKB_PTRACE("wkb " WKB_ADDR_FMT " data " WKB_ADDR_FMT " len %d "
        "frag_num %d totlen %d\n", WKB_ADDR_VAL(wkb),
        WKB_ADDR_VAL(wkb_data(wkb)), wkb_len(wkb),
        wkb->frag_num, wkb->frag_totlen);

    /* Reset Head packet fragment control */
    wkb->frag_num = 1;
    WKB_ASSERT(wkb->frag_totlen == wkb_frag_len(wkb, 0));

    /* Release frags from FTE */
    if (wkb_fte) {
        wkb     = wkb_fte;
        wkb_fte = NULL;
        wkb_set_next(prev, WKBUFF_2_PNBUFF(wkb));
        goto release_continue;
    }

    return frag_totcnt;
} /* wkb_frags_release() */

EXPORT_SYMBOL(wkb_frags_release);


#if (CC_WKB_DEBUG >= 1)
/**
 * -----------------------------------------------------------------------------
 * Section : WKB Debug Helpers
 * -----------------------------------------------------------------------------
 */

/* Pretty hex print a contiguous buffer */
static void
wkb_prhex(const char *msg, const unsigned char *buf, uint16_t nbytes)
{
    char line[128], *p;
    int len = sizeof(line);
    int nchar;
    int i;

    if (msg && (msg[0] != '\0'))
        printk("%s:\n", msg);

    p = line;
    for (i = 0; i < nbytes; i++) {
        if (i % 16 == 0) {
            nchar = snprintf(p, len, "  %04x: ", i);    /* line prefix */
            p += nchar;
            len -= nchar;
        }
        if (len > 0) {
            nchar = snprintf(p, len, "%02x ", buf[i]);
            p += nchar;
            len -= nchar;
        }

        if (i % 16 == 15) {
            printk("%s\n", line);               /* flush line */
            p = line;
            len = sizeof(line);
        }
    }

    /* flush last partial line */
    if (p != line)
        printk("%s\n", line);
} /* wkb_prhex() */

/* Dumps WKB attributes and data content of every fragment
 * dump_level
 * 0    :   WKB atributes
 * 1    :   First 32 bytes of WKB and frag data buffers
 * 2    :   Full payload of WKB and frag data buffers
 */
void
wkb_dump(wkbuff_t * wkb, const char *func, int line, int dump_level)
{
    uint8_t frag_idx;

    WKB_ERROR("---------- %s: %d ----------\n", func, line);
    WKB_ERROR("wkb " WKB_ADDR_FMT " data " WKB_ADDR_FMT " len %d "
        "frag_num %d totlen %d\n", WKB_ADDR_VAL(wkb),
        WKB_ADDR_VAL(wkb_data(wkb)), wkb_len(wkb),
        wkb->frag_num, wkb->frag_totlen);

    if (dump_level && (wkb_data(wkb) != NULL)) {
        if (dump_level > 1) {
            wkb_prhex("Data", wkb_data(wkb), wkb_len(wkb));
        } else {
            wkb_prhex("Data", wkb_data(wkb), 32);
        }
    }

    for (frag_idx = 0; frag_idx < wkb->frag_num; frag_idx++) {

        WKB_ERROR("\tfrag: idx %d key16: 0x%x len %d pStart " WKB_ADDR_FMT
            " data " WKB_ADDR_FMT "\n",
            frag_idx, wkb->frag_key[frag_idx].key16, wkb_frag_len(wkb, frag_idx),
            WKB_ADDR_VAL(wkb_frag_db_addr(wkb, frag_idx)),
            WKB_ADDR_VAL(wkb_frag_data(wkb, frag_idx)));

        if (dump_level && (wkb_frag_data(wkb, frag_idx) != NULL)) {
            if (dump_level > 1) {
                wkb_prhex("Data", wkb_frag_data(wkb, frag_idx),
                    wkb_frag_len(wkb, frag_idx));
            } else {
                wkb_prhex("Data", wkb_frag_data(wkb, frag_idx), 32);
            }
        }
    }
} /* wkb_dump() */

EXPORT_SYMBOL(wkb_dump);
#endif /* CC_WKB_DEBUG */
