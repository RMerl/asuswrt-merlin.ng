/*
<:copyright-BRCM:2011:DUAL/GPL:standard

   Copyright (c) 2011 Broadcom 
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
/**************************************************************************
 * File Name  : xtmrt_bpm.h
 *
 * Description: This file contains constant definitions and structure
 *              definitions for the BCM6368 ATM/PTM network device driver.
 ***************************************************************************/

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))

#if !defined(_BCMXTMRTBPM_H)
#define _BCMXTMRTBPM_H


#include <linux/gbpm.h>
#include "bpm.h"

/**** Externs ****/
extern void * xtm_alloc_buf_addr[];

/**** Prototypes ****/
void xtm_bpm_status(void);
void xtm_bpm_dump_txq_thresh(void);
void xtm_bpm_free_buf_ring( BcmXtm_RxDma *rxdma );
int xtm_bpm_alloc_buf_ring( BcmXtm_RxDma *rxdma, UINT32 num );
int xtm_bpm_txq_thresh( PBCMXTMRT_DEV_CONTEXT pDevCtx,
                        PXTMRT_TRANSMIT_QUEUE_ID pTxQId);

static inline int xtm_bpm_alloc_buf(BcmXtm_RxDma *rxdma);
static inline int xtm_bpm_free_buf(BcmXtm_RxDma *rxdma, UINT8 *pData);
                        

/**** Inline functions ****/

/* Allocates BPM_XTM_BULK_ALLOC_COUNT number of bufs and assigns to the
 * DMA ring of an XTM RX channel. The allocation is done in groups for
 * optimization.
 */
static inline int xtm_bpm_alloc_buf( BcmXtm_RxDma *rxdma )
{
    UINT8 *pData, *pFkBuf;
    int buf_ix;
    void **pDataBufs = xtm_alloc_buf_addr;
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    BcmPktDma_XtmRxDma *pktDmaRxInfo_p =
                        &pGi->rxdma[rxdma->pktDmaRxInfo.channel]->pktDmaRxInfo;

    if ( (pktDmaRxInfo_p->numRxBds - pktDmaRxInfo_p->rxAssignedBds)
            >= pktDmaRxInfo_p->allocTrig )
    { /* number of used buffers has crossed the trigger threshold */

        /* gbpm_alloc_mult_buf() fills pDataBufs with pointers to data buffers.
         * Each data buffer allocated has a preceeding FkBuff_t and headroom.
         * [FkBuff_t][BCM_PKT_HEADROOM][data buffer][Tail room][skb_shared_info]
         *                             ^
         *      gbpm_alloc_mult_buf(N, pDataBufs[i]) 
         *
         * BPM data buffers alignment is defined by sizeof(FkBuff_t)
         * BPM data buffers are not in cache. No need for cache-ops.
         */

        if (gbpm_alloc_mult_buf(pktDmaRxInfo_p->bulkAlloc, (void **)pDataBufs) == GBPM_ERROR)
        {
            /* may be temporarily global buffer pool is depleted.
             * Later try again */
            return GBPM_ERROR;
        }

        pktDmaRxInfo_p->alloc += pktDmaRxInfo_p->bulkAlloc;

        for (buf_ix=0; buf_ix < pktDmaRxInfo_p->bulkAlloc; buf_ix++)
        {
            /* Locate the FkBuff_t structure preceeding the BPM data buf */
            pData  = (UINT8 *) pDataBufs[buf_ix];
            pFkBuf = (UINT8 *) PDATA_TO_PFKBUFF(pData, BCM_PKT_HEADROOM);

            /* Align data buffers on 16-byte boundary - Apr 2010 */
            FlushAssignRxBuffer(rxdma->pktDmaRxInfo.channel, pData,
                                 pFkBuf + BCM_PKTBUF_SIZE);
        }
    }

    return GBPM_SUCCESS;
}

static inline int xtm_bpm_free_buf(BcmXtm_RxDma *rxdma, UINT8 *pData)
{
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    BcmPktDma_XtmRxDma *pktDmaRxInfo_p =
                        &pGi->rxdma[rxdma->pktDmaRxInfo.channel]->pktDmaRxInfo;
    gbpm_free_buf((void *)pData);
    pktDmaRxInfo_p->free--;

    return GBPM_SUCCESS;
}

#endif /* _BCMXTMRTBPM_H */
#endif
