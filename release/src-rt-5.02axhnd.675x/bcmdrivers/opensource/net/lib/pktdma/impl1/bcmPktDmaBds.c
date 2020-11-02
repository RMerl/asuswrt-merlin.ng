/*
 <:copyright-BRCM:2009:DUAL/GPL:standard
 
    Copyright (c) 2009 Broadcom 
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
/*
 *******************************************************************************
 * File Name  : bcmPktDma.c
 *
 * Description: This file contains the Packet DMA initialization API.
 *
 *******************************************************************************
 */

#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/nbuff.h>

#include "bcmtypes.h"
#include "bcmPktDma_bds.h"
#include "bcmPktDma_structs.h"
#include "bcmPktDma.h"
#include "bcmenet.h"

//#define BCM_PKTDMA_DUMP_BDS       /* enable dump of RX and TX BDs */

extern int kerSysGetSdramSize( void );
BcmPktDma_Bds bcmPktDma_Bds;
BcmPktDma_Bds *bcmPktDma_Bds_p = &bcmPktDma_Bds;
uint32_t bcmPktDma_tot_rxbds_g = 0;

static int bcmPktDma_calc_rxbds( void );
static int bcmPktDma_calc_txbds( void );

#if defined(BCM_PKTDMA_DUMP_BDS)
static int bcmPktDma_dump_rxbds( void )
{
    uint32_t chnl;

    printk("\n-------- Packet DMA RxBDs ---------\n" );
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
    /* ----------- FAP RX channel ---------- */
    for (chnl=0; chnl < ENET_RX_CHANNELS_MAX; chnl++)
    {
        printk( "ETH[%d] # of RxBds=%d\n", chnl,
                bcmPktDma_Bds_p->host.eth_rxbds[chnl] );
    }

    for (chnl=0; chnl < ENET_RX_CHANNELS_MAX; chnl++)
    {
        printk( "ETH[%d] Rx DQM depth=%d\n", chnl,
                bcmPktDma_Bds_p->host.eth_rxdqm[chnl] );
    }

#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
    /* XTM config */
    for (chnl=0; chnl < XTM_RX_CHANNELS_MAX; chnl++)
    {
        printk( "XTM[%d] # of RxBds=%d\n", chnl,
                bcmPktDma_Bds_p->host.xtm_rxbds[chnl] );
    }

    for (chnl=0; chnl < XTM_RX_CHANNELS_MAX; chnl++)
    {
        printk( "XTM[%d] Rx DQM depth=%d\n", chnl,
                bcmPktDma_Bds_p->host.xtm_rxdqm[chnl] );
    }
#endif

    /* FAP config */
    for (chnl=0; chnl < ENET_RX_CHANNELS_MAX; chnl++)
    {
        printk( "FAP ETH[%d] # of RxBds=%d\n", chnl,
                bcmPktDma_Bds_p->fap.eth_rxbds[chnl] );
    }

#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
    for (chnl=0; chnl < XTM_RX_CHANNELS_MAX; chnl++)
    {
        printk( "FAP XTM[%d] # of RxBds=%d\n", chnl,
                bcmPktDma_Bds_p->fap.xtm_rxbds[chnl] );
    }
#endif
#endif



#if !(defined(CONFIG_BCM_FAP) ||  defined(CONFIG_BCM_FAP_MODULE))
    /* ----------- Eth RX channel ---------- */
    for (chnl=0; chnl < ENET_RX_CHANNELS_MAX; chnl++)
    {
        printk( "ETH[%d] # of RxBds=%d\n", chnl,
                bcmPktDma_Bds_p->host.eth_rxbds[chnl] );
    }

#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
    /* ----------- XTM RX channel ---------- */
    for (chnl=0; chnl < XTM_RX_CHANNELS_MAX; chnl++)
    {
        printk( "XTM[%d] # of RxBds=%d\n", chnl,
                bcmPktDma_Bds_p->host.xtm_rxbds[chnl] );
    }
#endif
#endif
    return 0;
}


static int bcmPktDma_dump_txbds( void )
{
    uint32_t chnl;

    printk("\n-------- Packet DMA TxBDs ---------\n" );
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
    /* Host config */
    for (chnl=0; chnl < ENET_TX_CHANNELS_MAX; chnl++)
    {
        printk( "ETH[%d] # of TxBds =%d\n", chnl,
                bcmPktDma_Bds_p->host.eth_txbds[chnl] );
    }

    for (chnl=0; chnl < ENET_TX_CHANNELS_MAX; chnl++)
    {
        printk( "ETH[%d] Tx DQM depth=%d\n", chnl,
                bcmPktDma_Bds_p->host.eth_txdqm[chnl] );
    }

#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
    for (chnl=0; chnl < XTM_TX_CHANNELS_MAX; chnl++)
    {
        printk( "XTM[%d] # of TxBds =%d\n", chnl,
                bcmPktDma_Bds_p->host.xtm_txbds[chnl] );
    }

    for (chnl=0; chnl < XTM_TX_CHANNELS_MAX; chnl++)
    {
        printk( "XTM[%d] Tx DQM depth=%d\n", chnl,
                bcmPktDma_Bds_p->host.xtm_txdqm[chnl] );
    }
#endif

    /* FAP config */
    for (chnl=0; chnl < ENET_TX_CHANNELS_MAX; chnl++)
    {
        printk( "FAP ETH[%d] # of TxBds=%d\n", chnl,
                bcmPktDma_Bds_p->fap.eth_txbds[chnl] );
    }

#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
    for (chnl=0; chnl < XTM_TX_CHANNELS_MAX; chnl++)
    {
        printk( "FAP XTM[%d] # of TxBds=%d\n", chnl,
                bcmPktDma_Bds_p->fap.xtm_txbds[chnl] );
    }
#endif
#endif


#if !(defined(CONFIG_BCM_FAP) ||  defined(CONFIG_BCM_FAP_MODULE))
    for (chnl=0; chnl < ENET_TX_CHANNELS_MAX; chnl++)
    {
        printk( "ETH[%d] # of TxBds=%d\n", chnl,
                        bcmPktDma_Bds_p->host.eth_txbds[chnl]);
    }

#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
    for (chnl=0; chnl < XTM_TX_CHANNELS_MAX; chnl++)
    {
        printk( "XTM[%d] # of TxBds=%d\n", chnl,
                    bcmPktDma_Bds_p->host.xtm_txbds[chnl]);
    }
#endif
#endif
    return 0;
}
#endif /* defined(BCM_PKTDMA_DUMP_BDS) */



static int bcmPktDma_calc_rxbds( void )
{
    uint32_t __attribute__((unused)) tot_mem_size = kerSysGetSdramSize();
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    uint32_t buf_mem_size = (tot_mem_size/100) * CONFIG_BCM_BPM_BUF_MEM_PRCNT;
    uint32_t tot_num_bufs=0;
#endif
    uint32_t chnl;
    uint32_t host_eth_rxbds;
#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
    uint32_t host_xtm_rxbds;
#endif

    chnl = 0;                 /* to avoid compiler warning */

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    tot_num_bufs = (buf_mem_size/BCM_PKTBUF_SIZE);
#endif

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
    /* ----------- FAP RX channel ---------- */
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
/* Channel-0 is default */
    host_eth_rxbds = (ENET_DEF_RXBDS_BUF_PRCNT * tot_num_bufs/200);

    if (host_eth_rxbds < HOST_ENET_NR_RXBDS_MIN)
        host_eth_rxbds = HOST_ENET_NR_RXBDS_MIN;
#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
    host_xtm_rxbds = (XTM_DEF_RXBDS_BUF_PRCNT * tot_num_bufs/200);

    if (host_xtm_rxbds < HOST_XTM_NR_RXBDS_MIN)
        host_xtm_rxbds = HOST_XTM_NR_RXBDS_MIN;
#endif
#else /* (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE)) */
    host_eth_rxbds = HOST_ENET_NR_RXBDS;
#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
    host_xtm_rxbds = HOST_XTM_NR_RXBDS;
#endif
#endif /* (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE)) */

#if defined(CONFIG_BCM_GMAC)
    host_eth_rxbds /= 2;
#endif

    for (chnl=0; chnl < ENET_RX_CHANNELS_MAX; chnl++)
    {
        if (g_Eth_rx_iudma_ownership[chnl] == HOST_OWNED )
            bcmPktDma_Bds_p->host.eth_rxbds[chnl] = host_eth_rxbds;
        else
            bcmPktDma_Bds_p->host.eth_rxbds[chnl] = 0;

        bcmPktDma_tot_rxbds_g += bcmPktDma_Bds_p->host.eth_rxbds[chnl];
    }

    for (chnl=0; chnl < ENET_RX_CHANNELS_MAX; chnl++)
    {
        if (g_Eth_rx_iudma_ownership[chnl] == HOST_OWNED )
            bcmPktDma_Bds_p->host.eth_rxdqm[chnl] = 0;
        else
        {
            bcmPktDma_Bds_p->host.eth_rxdqm[chnl] = 
                DQM_FAP2HOST_ETH_RX_DEPTH_LOW + DQM_FAP2HOST_ETH_RX_DEPTH_HI;
        }
    }

    /* XTM config */
#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
    for (chnl=0; chnl < XTM_RX_CHANNELS_MAX; chnl++)
    {
        if (g_Xtm_rx_iudma_ownership[chnl] == HOST_OWNED)
        {
            bcmPktDma_Bds_p->host.xtm_rxbds[chnl] = host_xtm_rxbds;
        }
        else
            bcmPktDma_Bds_p->host.xtm_rxbds[chnl] =
                FAP_XTM_NON_DEF_CHNL_NR_RXBDS;

        bcmPktDma_tot_rxbds_g += bcmPktDma_Bds_p->host.xtm_rxbds[chnl];
    }

    for (chnl=0; chnl < XTM_RX_CHANNELS_MAX; chnl++)
    {
        if (g_Xtm_rx_iudma_ownership[chnl] == HOST_OWNED)
            bcmPktDma_Bds_p->host.xtm_rxdqm[chnl] = 0;
        else
        {
            if (chnl == 0)
            {
#ifdef CONFIG_BCM963268
                /* Both High and Low Queues have the same depth on 63268 */
                bcmPktDma_Bds_p->host.xtm_rxdqm[chnl] =
                    FAP1_63268_DQM_FAP2HOST_XTM_RX_Q_DEPTH_LO + DQM_FAP2HOST_XTM_RX_DEPTH_HI;
#else
                bcmPktDma_Bds_p->host.xtm_rxdqm[chnl] =
                    DQM_FAP2HOST_XTM_RX_DEPTH_LOW + DQM_FAP2HOST_XTM_RX_DEPTH_HI;
#endif
            }
            else
            {
                /* currently we using only 1 channel even when 2 channels are
                 * intialized, so keep the other channel size very low 
                 */
                bcmPktDma_Bds_p->host.xtm_rxdqm[chnl] = 16;
            }
        }
    }
#endif /* #if defined(CONFIG_BCM_XTMCFG) */

    /* FAP config */
    for (chnl=0; chnl < ENET_RX_CHANNELS_MAX; chnl++)
    {
        if (g_Eth_rx_iudma_ownership[chnl] == HOST_OWNED)
            bcmPktDma_Bds_p->fap.eth_rxbds[chnl] = 0;
        else
            bcmPktDma_Bds_p->fap.eth_rxbds[chnl] = FAP_ENET_NR_RXBDS;

        bcmPktDma_tot_rxbds_g += bcmPktDma_Bds_p->fap.eth_rxbds[chnl];
    }

#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
    for (chnl=0; chnl < XTM_RX_CHANNELS_MAX; chnl++)
    {
        if (g_Xtm_rx_iudma_ownership[chnl] == HOST_OWNED)
            bcmPktDma_Bds_p->fap.xtm_rxbds[chnl] = 0;
        else
        {
            if (chnl == 0)
                bcmPktDma_Bds_p->fap.xtm_rxbds[chnl] = FAP_XTM_NR_RXBDS;
            else
                bcmPktDma_Bds_p->fap.xtm_rxbds[chnl] =
                    FAP_XTM_NON_DEF_CHNL_NR_RXBDS;
        }
        bcmPktDma_tot_rxbds_g += bcmPktDma_Bds_p->fap.xtm_rxbds[chnl];
    }
#endif /* defined(CONFIG_BCM_XTMCFG) */
#endif /* defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE) */



#if !(defined(CONFIG_BCM_FAP) ||  defined(CONFIG_BCM_FAP_MODULE))
    /* ----------- Eth RX channel ---------- */
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    host_eth_rxbds = ENET_DEF_RXBDS_BUF_PRCNT * tot_num_bufs/100;
#else /* (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE)) */
    if (tot_mem_size <= 0x1600000)    // less than or equal to 16MB
        host_eth_rxbds = HOST_ENET_NR_RXBDS/4;
    else
        host_eth_rxbds = HOST_ENET_NR_RXBDS;
#endif /* (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE)) */

#if defined(CONFIG_BCM94908)
    if (host_eth_rxbds > 8*1024) /* MAX support BDs by iuDMA channel */
        host_eth_rxbds = 8*1024;
#endif

    for (chnl=0; chnl < ENET_RX_CHANNELS_MAX; chnl++)
    {
        bcmPktDma_Bds_p->host.eth_rxbds[chnl] =
                    HOST_ENET_NON_DEF_CHNL_NR_RXBDS;
    }

#if defined(CONFIG_BCM_GMAC)
    host_eth_rxbds /= 2;
    bcmPktDma_Bds_p->host.eth_rxbds[0] = host_eth_rxbds;
    bcmPktDma_Bds_p->host.eth_rxbds[ENET_RX_CHANNELS_MAX-1] = host_eth_rxbds;
#else

#if defined(CONFIG_BCM960333) || defined(CONFIG_BCM947189)
    /*
     * For Duna: read the number of network interfaces from Boardparms.
     * Currently we assign the same number of RX buffers to each interface.
     */
    {
        #define BCM947189_ETH_MAX_RXBD (4096)

        const ETHERNET_MAC_INFO *EnetInfo;
        int num_channels = 0;

        if ( (EnetInfo = BpGetEthernetMacInfoArrayPtr()) == NULL)
        {
            return -ENODEV;
        }
        bitcount(num_channels, EnetInfo[0].sw.port_map);
        for (chnl = 0; chnl < ENET_RX_CHANNELS_MAX; chnl++)
        {
            if(EnetInfo[0].sw.port_map & (1<<chnl))
            {
                bcmPktDma_Bds_p->host.eth_rxbds[chnl] =
                                  host_eth_rxbds / num_channels;
                if(bcmPktDma_Bds_p->host.eth_rxbds[chnl] > BCM947189_ETH_MAX_RXBD)
                    bcmPktDma_Bds_p->host.eth_rxbds[chnl] = BCM947189_ETH_MAX_RXBD;
            }
        }
    }
#else
    bcmPktDma_Bds_p->host.eth_rxbds[0] = host_eth_rxbds;
#endif

#endif

    for (chnl=0; chnl < ENET_RX_CHANNELS_MAX; chnl++)
    {
        bcmPktDma_tot_rxbds_g += bcmPktDma_Bds_p->host.eth_rxbds[chnl];
    }

#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
    /* ----------- XTM RX channel ---------- */
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    host_xtm_rxbds = XTM_DEF_RXBDS_BUF_PRCNT * tot_num_bufs/100;
#else /* (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE) || defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE)) */

    /* ----------- XTM RX channel ---------- */
    {
        uint32_t tot_mem_size = kerSysGetSdramSize();

        if (tot_mem_size <= 0x800000)    // less than or equal to 8MB
            host_xtm_rxbds = 60;
        else if (tot_mem_size <= 0x1600000)    // less than or equal to 16MB
            host_xtm_rxbds = HOST_XTM_NR_RXBDS/4;
        else
            host_xtm_rxbds = HOST_XTM_NR_RXBDS;
    }
#endif /* (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE)) */

#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
    bcmPktDma_Bds_p->host.xtm_rxbds[0] = host_xtm_rxbds;
    for (chnl=1; chnl < XTM_RX_CHANNELS_MAX; chnl++)
    {
        bcmPktDma_Bds_p->host.xtm_rxbds[chnl] = HOST_XTM_NON_DEF_CHNL_NR_RXBDS;
    }

    for (chnl=0; chnl < XTM_RX_CHANNELS_MAX; chnl++)
    {
        bcmPktDma_tot_rxbds_g += bcmPktDma_Bds_p->host.xtm_rxbds[chnl];
    }
#endif
#endif
#endif

#if defined(BCM_PKTDMA_DUMP_BDS)
    bcmPktDma_dump_rxbds();
#endif /* defined(BCM_PKTDMA_DUMP_BDS) */

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    printk( "Total # RxBds=%d\n", bcmPktDma_tot_rxbds_g);
    if (bcmPktDma_tot_rxbds_g > tot_num_bufs)
    {
        printk( "ERROR!!!!: Not enough buffers available\n" );
        printk( "ERROR!!!!: Either increase the %% of buffer memory "
                            "using make menuconfig\n" );
        printk( "ERROR!!!!: Or reduce the # of RxBDs (bcmPktDma_bds.h)\n" );
        return -1;
    }
    else if (bcmPktDma_tot_rxbds_g > (tot_num_bufs*2/3))
    {
        printk( "WARNING: # of RXBDs > (buffers*2/3)\n" );
        printk( "WARNING: less buffers available for BPM\n" );
    }
#endif

    return 0;
}


static int bcmPktDma_calc_txbds( void )
{
    uint32_t chnl;
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
    int iudmaIdx;
#else
    uint32_t tot_mem_size = kerSysGetSdramSize();
#endif

    chnl = 0;                 /* to avoid compiler warning */

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
    /* Host config */
    for (chnl=0; chnl < ENET_TX_CHANNELS_MAX; chnl++)
    {
        if (g_Eth_tx_iudma_ownership[chnl] == HOST_OWNED)
            bcmPktDma_Bds_p->host.eth_txbds[chnl] = HOST_ENET_NR_TXBDS;
        else
            bcmPktDma_Bds_p->host.eth_txbds[chnl] = 0;
    }

    for (chnl=0; chnl < ENET_TX_CHANNELS_MAX; chnl++)
    {
        if (g_Eth_tx_iudma_ownership[chnl] == HOST_OWNED)
            bcmPktDma_Bds_p->host.eth_txdqm[chnl] = 0;
        else
            bcmPktDma_Bds_p->host.eth_txdqm[chnl] = DQM_HOST2FAP_ETH_XMIT_DEPTH_LOW;
    }

#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
    for (chnl=0; chnl < XTM_TX_CHANNELS_MAX; chnl++)
    {
        bcmPktDma_Bds_p->host.xtm_txbds[chnl] = 0;
    }

    for (chnl=0; chnl < XTM_TX_CHANNELS_MAX; chnl++)
    {
        if (g_Xtm_tx_iudma_ownership[chnl] == HOST_OWNED)
            bcmPktDma_Bds_p->host.xtm_txdqm[chnl] = 0;
        else
#ifdef CONFIG_BCM963268
            bcmPktDma_Bds_p->host.xtm_txdqm[chnl] = FAP0_63268_DQM_HOST2FAP_XTM_XMIT_Q_DEPTH;
#else
            bcmPktDma_Bds_p->host.xtm_txdqm[chnl] = DQM_HOST2FAP_XTM_XMIT_DEPTH_LOW;
#endif
    }
#endif /* defined(CONFIG_BCM_XTMCFG) */

    /* FAP config */
    for (iudmaIdx = 0; iudmaIdx < CONFIG_BCM_DEF_NR_TX_DMA_CHANNELS; iudmaIdx++)
    {
        if (g_Eth_tx_iudma_ownership[iudmaIdx] == HOST_OWNED)
            bcmPktDma_Bds_p->fap.eth_txbds[iudmaIdx] = 0;
        else
            bcmPktDma_Bds_p->fap.eth_txbds[iudmaIdx] = FAP_ENET_NR_TXBDS;
    }

#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
    for (chnl=0; chnl < XTM_TX_CHANNELS_MAX; chnl++)
    {
        if (g_Xtm_tx_iudma_ownership[chnl] == HOST_OWNED)
            bcmPktDma_Bds_p->fap.xtm_txbds[chnl] = 0;
        else
            bcmPktDma_Bds_p->fap.xtm_txbds[chnl] = FAP_XTM_NR_TXBDS;
    }
#endif /* defined(CONFIG_BCM_XTMCFG) */
#endif


#if defined(CONFIG_BCM960333)
    /*
     * For Duna: read the number of network interfaces from Boardparms.
     * Currently we assign the same number of TX buffers to each interface.
     */
    {
        const ETHERNET_MAC_INFO *EnetInfo;
        int num_channels = 0;

        if ( (EnetInfo = BpGetEthernetMacInfoArrayPtr()) == NULL)
        {
            return -ENODEV;
        }
        bitcount(num_channels, EnetInfo[0].sw.port_map);
        for (chnl = 0; chnl < num_channels; chnl++)
        {
            /* Setup the PLC TX ring size to hold up to 60% of BPM buffers */
            if (EnetInfo[0].sw.phyconn[chnl] == PHY_CONN_TYPE_PLC)
            {
                uint32_t tx_ring_size;
                tx_ring_size = kerSysGetSdramSize() / 100
                        * CONFIG_BCM_BPM_BUF_MEM_PRCNT * 60 / 100
                        / BCM_PKTBUF_SIZE;
                tx_ring_size = tx_ring_size & (uint32_t)(0xFFFFFFFE);
                if (tx_ring_size > 8192)
                {
                    tx_ring_size = 8192;
                }
                bcmPktDma_Bds_p->host.eth_txbds[chnl] = tx_ring_size;
            }
            /* Every other channel has a TX ring of default size */
            else
            {
                if (tot_mem_size <= 0x1600000)    // less than or equal to 16MB
                    bcmPktDma_Bds_p->host.eth_txbds[chnl] = HOST_ENET_NR_TXBDS/4;
                else
                    bcmPktDma_Bds_p->host.eth_txbds[chnl] = HOST_ENET_NR_TXBDS;
            }
        }
    }
#endif

#if !(defined(CONFIG_BCM960333)\
        ||  defined(CONFIG_BCM_FAP) ||   defined(CONFIG_BCM_FAP_MODULE))
    for (chnl=0; chnl < ENET_TX_CHANNELS_MAX; chnl++)
    {
        if (tot_mem_size <= 0x1600000)    // less than or equal to 16MB
            bcmPktDma_Bds_p->host.eth_txbds[chnl] = HOST_ENET_NR_TXBDS/4;
        else
            bcmPktDma_Bds_p->host.eth_txbds[chnl] = HOST_ENET_NR_TXBDS;
    }

#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
    for (chnl=0; chnl < XTM_TX_CHANNELS_MAX; chnl++)
    {
        if (tot_mem_size <= 0x1600000)    // less than or equal to 16MB
            bcmPktDma_Bds_p->host.xtm_txbds[chnl] = HOST_XTM_NR_TXBDS/4;
        else
            bcmPktDma_Bds_p->host.xtm_txbds[chnl] = HOST_XTM_NR_TXBDS;
    }
#endif
#endif

#if defined(BCM_PKTDMA_DUMP_BDS)
    bcmPktDma_dump_txbds();
#endif /* defined(BCM_PKTDMA_DUMP_BDS) */

    return 0;
}



int bcmPktDma_EthGetRxBds( BcmPktDma_LocalEthRxDma *rxdma, int channel )
{
    int nr_rx_bds;
#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    if (g_Eth_rx_iudma_ownership[channel] == HOST_OWNED)
        nr_rx_bds = bcmPktDma_Bds_p->host.eth_rxbds[channel];
    else
        nr_rx_bds = bcmPktDma_Bds_p->fap.eth_rxbds[channel];
#else
    nr_rx_bds = bcmPktDma_Bds_p->host.eth_rxbds[channel];
#endif

    return nr_rx_bds;
}


int bcmPktDma_EthGetTxBds( BcmPktDma_LocalEthTxDma *txdma, int channel )
{
    int nr_tx_bds;
#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    if (g_Eth_tx_iudma_ownership[channel] == HOST_OWNED)
        nr_tx_bds = bcmPktDma_Bds_p->host.eth_txbds[channel];
    else
        nr_tx_bds = bcmPktDma_Bds_p->fap.eth_txbds[channel];
#else
    nr_tx_bds = bcmPktDma_Bds_p->host.eth_txbds[channel];
#endif

    return nr_tx_bds;
}


int bcmPktDma_XtmGetRxBds( int channel )
{
#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
    int nr_rx_bds = 0;

#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    if (g_Xtm_rx_iudma_ownership[channel] == HOST_OWNED)
        nr_rx_bds = bcmPktDma_Bds_p->host.xtm_rxbds[channel];
    else
        nr_rx_bds = bcmPktDma_Bds_p->fap.xtm_rxbds[channel];
#else
    nr_rx_bds = bcmPktDma_Bds_p->host.xtm_rxbds[channel];
#endif

    return nr_rx_bds;
#else
    return -1;
#endif
}


int bcmPktDma_XtmGetTxBds( int channel )
{
#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
    int nr_tx_bds = 0;

#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    if (g_Xtm_tx_iudma_ownership[channel] == HOST_OWNED)
        nr_tx_bds = bcmPktDma_Bds_p->host.xtm_txbds[channel];
    else
        nr_tx_bds = bcmPktDma_Bds_p->fap.xtm_txbds[channel];
#else
    nr_tx_bds = bcmPktDma_Bds_p->host.xtm_txbds[channel];
#endif
    return nr_tx_bds;
#else
    return -1;
#endif
}

int bcmPktDma_GetTotRxBds( void )
{
    return bcmPktDma_tot_rxbds_g;
}


int __init bcmPktDmaBds_init(void)
{
    memset( bcmPktDma_Bds_p, 0, sizeof(BcmPktDma_Bds) );
    bcmPktDma_calc_rxbds();
    bcmPktDma_calc_txbds();
    printk("%s: Broadcom Packet DMA BDs initialized\n\n", __FUNCTION__);

    return 0;
}

void __exit bcmPktDmaBds_exit(void)
{
    printk("Broadcom Packet DMA BDs exited\n");
}

module_init(bcmPktDmaBds_init);
module_exit(bcmPktDmaBds_exit);


EXPORT_SYMBOL(bcmPktDma_Bds_p);
EXPORT_SYMBOL(bcmPktDma_tot_rxbds_g);
EXPORT_SYMBOL(bcmPktDma_GetTotRxBds);
EXPORT_SYMBOL(bcmPktDma_EthGetRxBds);
EXPORT_SYMBOL(bcmPktDma_EthGetTxBds);
EXPORT_SYMBOL(bcmPktDma_XtmGetRxBds);
EXPORT_SYMBOL(bcmPktDma_XtmGetTxBds);


