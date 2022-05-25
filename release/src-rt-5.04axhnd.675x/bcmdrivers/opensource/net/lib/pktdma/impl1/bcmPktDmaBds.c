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
#include "board.h"

//#define BCM_PKTDMA_DUMP_BDS       /* enable dump of RX and TX BDs */

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
    return 0;
}


static int bcmPktDma_dump_txbds( void )
{
    uint32_t chnl;

    printk("\n-------- Packet DMA TxBDs ---------\n" );


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
    return 0;
}
#endif /* defined(BCM_PKTDMA_DUMP_BDS) */



static int bcmPktDma_calc_rxbds( void )
{
    uint64_t __attribute__((unused)) tot_mem_size = kerSysGetSdramSize();
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


#if defined(CONFIG_BCM947189)
    /*
     * Read the number of network interfaces from Boardparms.
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
        uint64_t tot_mem_size = kerSysGetSdramSize();

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
    uint64_t tot_mem_size = kerSysGetSdramSize();

    chnl = 0;                 /* to avoid compiler warning */


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

#if defined(BCM_PKTDMA_DUMP_BDS)
    bcmPktDma_dump_txbds();
#endif /* defined(BCM_PKTDMA_DUMP_BDS) */

    return 0;
}



int bcmPktDma_EthGetRxBds( BcmPktDma_LocalEthRxDma *rxdma, int channel )
{
    int nr_rx_bds;
    nr_rx_bds = bcmPktDma_Bds_p->host.eth_rxbds[channel];

    return nr_rx_bds;
}


int bcmPktDma_EthGetTxBds( BcmPktDma_LocalEthTxDma *txdma, int channel )
{
    int nr_tx_bds;
    nr_tx_bds = bcmPktDma_Bds_p->host.eth_txbds[channel];

    return nr_tx_bds;
}


int bcmPktDma_XtmGetRxBds( int channel )
{
#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
    int nr_rx_bds = 0;

    nr_rx_bds = bcmPktDma_Bds_p->host.xtm_rxbds[channel];

    return nr_rx_bds;
#else
    return -1;
#endif
}


int bcmPktDma_XtmGetTxBds( int channel )
{
#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
    int nr_tx_bds = 0;

    nr_tx_bds = bcmPktDma_Bds_p->host.xtm_txbds[channel];
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


