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

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))

//#include <linux/version.h>
//#include <linux/module.h>
//#include <linux/kernel.h>
#include <linux/types.h>
//#include <linux/interrupt.h>
//#include <linux/ioport.h>
//#include <linux/slab.h>
//#include <linux/init.h>
//#include <linux/delay.h>
//#include <linux/netdevice.h>
//#include <linux/etherdevice.h>
//#include <linux/skbuff.h>
//#include <linux/rtnetlink.h>
//#include <linux/ethtool.h>
//#include <linux/if_arp.h>
//#include <linux/ppp_channel.h>
//#include <linux/ppp_defs.h>
//#include <linux/if_ppp.h>
//#include <linux/atm.h>
//#include <linux/atmdev.h>
//#include <linux/atmppp.h>
//#include <linux/blog.h>
//#include <linux/proc_fs.h>
#include <linux/string.h>
//#include <linux/etherdevice.h>
//#include <linux/ip.h>
#include <bcmtypes.h>
//#include <bcm_map_part.h>
//#include <bcm_intr.h>
//#include <board.h>
//#include "bcmnet.h"
//#include "bcmxtmcfg.h"
//#include "bcmxtmrt.h"
//#include <asm/io.h>
//#include <asm/uaccess.h>
//#include <linux/nbuff.h>
//#include "bcmxtmrtimpl.h"
#include "bcmPktDma.h"
#include "xtmrt_iq.h"


/**** Externs ****/

extern BcmPktDma_Bds *bcmPktDma_Bds_p;
extern UINT32        iqos_cpu_cong_g;



/**** Globals ****/


/**** Statics ****/
static thresh_t xtm_rx_dma_iq_thresh[XTM_RX_CHANNELS_MAX];


/**** Prototypes ****/
static void xtm_iq_dma_status(void);




/* init XTM IQ thresholds */
void xtm_rx_init_iq_thresh(int chnl)
{
    int nr_rx_bds;

    {
        nr_rx_bds = bcmPktDma_Bds_p->host.xtm_rxbds[chnl];

        xtm_rx_dma_iq_thresh[chnl].loThresh =
                        (nr_rx_bds * IQ_XTM_LO_THRESH_PCT)/100;
        xtm_rx_dma_iq_thresh[chnl].hiThresh =
                        (nr_rx_bds * IQ_XTM_HI_THRESH_PCT)/100;
    }
}

void xtm_rx_set_iq_thresh( int chnl )
{
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    BcmPktDma_XtmRxDma *rxdma = &pGi->rxdma[chnl]->pktDmaRxInfo;

    BCM_XTM_DEBUG("Xtm: chan=%d iqLoThresh=%d iqHiThresh=%d\n",
        chnl, (int) rxdma->iqLoThresh, (int) rxdma->iqHiThresh );


    bcmPktDma_XtmSetIqThresh(rxdma,
                xtm_rx_dma_iq_thresh[chnl].loThresh,
                xtm_rx_dma_iq_thresh[chnl].hiThresh);
}




void xtm_iq_update_cong_status( int chnl )
{
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    BcmPktDma_XtmRxDma *rxdma = &pGi->rxdma[chnl]->pktDmaRxInfo;
    int thrOfst;
    DmaDesc  dmaDesc;
    volatile DmaDesc *rxBd_pv;


    if (iqos_get_cong_status(IQOS_IF_XTM, chnl) == IQOS_CONG_STATUS_HI)
    {
        /* calculate low threshold ring offset */
        thrOfst = rxdma->rxTailIndex + rxdma->iqLoThresh;

        if (thrOfst >= rxdma->numRxBds)
            thrOfst %= rxdma->numRxBds;

        /* Get the status from Rx BD */
        rxBd_pv = &rxdma->rxBds[thrOfst];
        dmaDesc.word0 = rxBd_pv->word0;

        if ((dmaDesc.status & DMA_OWN) == DMA_OWN)
        { /* low thresh crossed on downside */
            iqos_set_cong_status( IQOS_IF_XTM, chnl, IQOS_CONG_STATUS_LO );
        }

    }
    else
    {
        /* calculate high threshold ring offset */
        thrOfst = rxdma->rxTailIndex + rxdma->iqHiThresh;

        if (thrOfst >= rxdma->numRxBds)
            thrOfst %= rxdma->numRxBds;

        /* Get the status from Rx BD */
        rxBd_pv = &rxdma->rxBds[thrOfst];
        dmaDesc.word0 = rxBd_pv->word0;

        if ((dmaDesc.status & DMA_OWN) == 0)
        {/* high thresh crossed on upside */
            iqos_set_cong_status( IQOS_IF_XTM, chnl, IQOS_CONG_STATUS_HI );
        }
    }
}


/* dump the IQ thresholds, stats and cong status */
void xtm_iq_dma_status(void)
{
    int chnl;

    for (chnl=0; chnl < XTM_RX_CHANNELS_MAX; chnl++)
    {
        PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
        BcmPktDma_XtmRxDma *rxdma = &pGi->rxdma[chnl]->pktDmaRxInfo;


        printk("[HOST] XTM  %4d %5d %5d %5d %10u %8x\n",
               chnl,
               (int) rxdma->iqLoThresh,
               (int) rxdma->iqHiThresh,
               (rxdma->numRxBds - rxdma->rxAssignedBds),
               (unsigned int)
#if defined(CC_IQ_STATS)
               rxdma->iqDropped,
#else
               0,
#endif
               (unsigned int)iqos_cpu_cong_g
               );
    }
}

/* print the IQ status */
void xtm_iq_status(void)
{
    xtm_iq_dma_status();
}

#endif


