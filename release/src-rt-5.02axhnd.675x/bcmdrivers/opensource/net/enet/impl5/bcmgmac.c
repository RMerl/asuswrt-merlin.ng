/*
<:copyright-BRCM:2012:DUAL/GPL:standard

   Copyright (c) 2012 Broadcom 
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


//**************************************************************************
// File Name  : bcmgmac.c
//
// Description: This is Linux network driver for Broadcom GMAC controller
//
//**************************************************************************

#define VERSION     "0.1"
#define VER_STR     "v" VERSION

#define _BCMENET_LOCAL_

#include <linux/types.h>
#include <linux/bcm_log_mod.h>
#include <bcm_map_part.h>
#include <bcm_intr.h>
#include <board.h>
#include "bcmenet.h"
#include "bcmPktDma.h"
#include "bcmsw.h"
#include "bcmgmacctl.h"
#include "bcmgmac.h"
#include "bcmsw_dma.h"
#include "ethsw.h"
#include "ethsw_phy.h"
#if defined(CONFIG_BLOG) 
#include <linux/blog.h>
#endif

/*----- Globals -----*/
#undef GMAC_DECL
#define GMAC_DECL(x) #x,

const char *gmacctl_ioctl_name[] =
{
    GMAC_DECL(GMACCTL_IOCTL_SYS)
    GMAC_DECL(GMACCTL_IOCTL_MAX)
};

const char *gmacctl_subsys_name[] =
{
    GMAC_DECL(GMACCTL_SUBSYS_STATUS)
    GMAC_DECL(GMACCTL_SUBSYS_MODE)
    GMAC_DECL(GMACCTL_SUBSYS_MAX)
};

const char *gmacctl_op_name[] =
{   
    GMAC_DECL(GMACCTL_OP_GET)
    GMAC_DECL(GMACCTL_OP_SET)
    GMAC_DECL(GMACCTL_OP_DUMP)
    GMAC_DECL(GMACCTL_OP_MAX)
};

DEFINE_SPINLOCK(gmac_lock_g);
EXPORT_SYMBOL(gmac_lock_g);


gmac_info_t gmac_info_g,  *gmac_info_pg = &gmac_info_g;

#if defined(CONFIG_BLOG) 
extern int fcacheDrvFlushAll( void );
#endif

extern void enet_rxdma_channel_enable(int chan);
extern void enet_txdma_channel_enable(int chan);
extern int enet_add_rxdma_channel(int chan);
extern int enet_del_rxdma_channel(int chan);
extern int enet_add_txdma_channel(int chan);
extern int enet_del_txdma_channel(int chan);

extern int enet_gmac_log_port( void );
extern int enet_set_port_ctrl( int port, uint32_t val32 );
extern void enet_restart_autoneg(int log_port);
int gmac_mac_get_max_frm_len( void );
void gmac_adjust_config( void );

void gmac_dbg_status( char *str_p )
{
    BCM_LOG_DEBUG( BCM_LOG_ID_GMAC, "%s\n"
                   "enabled = %u wan = %u mode = %u active = %u\n" 
                   "link_up = %u link_speed = %u\n"
                   "log_chan = %d phy_chan = %d\n", str_p,
                   gmac_info_pg->enabled, gmac_info_pg->wan, 
                   gmac_info_pg->mode, gmac_info_pg->active,
                   gmac_info_pg->link_up, gmac_info_pg->link_speed, 
                   gmac_info_pg->log_chan, gmac_info_pg->phy_chan );
}

/* check if the chip support gmac feature */
int gmac_is_gmac_supported( void )
{
    unsigned int options;
    int rc = 0;
    if (BpGetDeviceOptions(&options) == BP_SUCCESS) {
        // only do this if the get was successful.
        // If it fails, options is non-zero and incorrect
        if (options & BP_DEVICE_OPTION_ENABLE_GMAC)
        {
#if defined(CONFIG_BCM963268)
            if (gmac_info_pg->rev_id >= 0xD0)
#endif
                rc = 1;
        }
    }

    return rc;
}

/* check if the port is a gmac port */
int gmac_is_gmac_port( int port )
{
	int rc = 0;

#if defined(CONFIG_BCM963268)
    if ( port == GMAC_PORT_ID )
        rc = 1;
/* determine what to do for 6818 later
#elif defined(CONFIG_BCM96818)
*/
#endif
    return rc;
}

/* Sets the default config when the port is add as WAN port or delete
 * as WAN port. */
void gmac_set_wan_port( int add )
{
    int phyId;
    PHY_STAT ps;

    BCM_ASSERT( gmac_info_pg->enabled == 1 );
    BCM_LOG_DEBUG( BCM_LOG_ID_GMAC, "add=%d", add );
    BCM_ASSERT( add <= 1 );
    /* set default values */

    if (add)
    {
        gmac_info_pg->mode = GMAC_MODE_LINK_SPEED; 
        gmac_info_pg->wan = 1; 
    }
    else
    {
        /* May need to adjust config. When the port is removed as
        * port, configure the port as ROBO port */
        gmac_set_mode( GMAC_MODE_ROBO_PORT );
        gmac_info_pg->wan = 0; 
    }

    phyId = enet_sw_port_to_phyid(0, GMAC_PORT_ID);
    ps = ethsw_phyid_stat(phyId);
    
    /* Populate link information to gmac_info_pg */
    gmac_info_pg->link_up = ps.lnk > 0;
    if (gmac_info_pg->link_up)
    {
        gmac_info_pg->link_speed = ps.spd1000? 1000: (ps.spd100? 100: 10);
        gmac_info_pg->duplex = ps.fdx;
    }

    gmac_adjust_config();
}


/* sets the GMAC to be active, and ROBO port to be inactive */
int gmac_set_active( void )
{
    gmac_info_pg->trans = 1;

#if defined(CONFIG_BLOG)
    fcacheDrvFlushAll();
#endif

    netif_device_detach(phyPortId_to_netdev(GMAC_PORT_ID, -1) );

    /* delete the ROBO channel */
    enet_del_rxdma_channel(GMAC_LOG_CHAN);
    enet_del_txdma_channel(GMAC_LOG_CHAN);

    gmac_info_pg->active = 1;    /* GMAC is active */

    /* Add the GMAC channel */
    enet_add_txdma_channel(GMAC_LOG_CHAN);
    enet_add_rxdma_channel(GMAC_LOG_CHAN);

    /* Now select GMAC at PHY3 */
#if defined(CONFIG_BCM963268)
    BCM_LOG_DEBUG( BCM_LOG_ID_GMAC, "Select GMAC at Mux (set bit18=0x40000)" ); 
    GPIO->RoboswGphyCtrl |= GPHY_MUX_SEL_GMAC;

    BCM_LOG_DEBUG( BCM_LOG_ID_GMAC, "\tGPIORoboswGphyCtrl<0x%p>=0x%x", 
        &GPIO->RoboswGphyCtrl, (uint32_t) GPIO->RoboswGphyCtrl );
#endif

    netif_device_attach( phyPortId_to_netdev(GMAC_PORT_ID, -1) );

    /* Enable GMAC Tx and Rx */
    BCM_LOG_DEBUG( BCM_LOG_ID_GMAC, "Enable GMAC Rx & Tx (set bitMask 0x03)" ); 
    GMAC_MAC->Cmd.tx_ena = 1;
    GMAC_MAC->Cmd.rx_ena = 1;

    enet_txdma_channel_enable(GMAC_LOG_CHAN);
    enet_rxdma_channel_enable(GMAC_LOG_CHAN);
    gmac_info_pg->trans = 0;

    BCM_LOG_DEBUG( BCM_LOG_ID_GMAC, "GMAC Activated (active =%d)",
        gmac_info_pg->active );

    return 0;
}


/* sets the GMAC to be inactive, and ROBO port to be active */
int gmac_set_inactive( void )
{
    gmac_info_pg->trans = 1;

    /* Disable GMAC Rx & Tx */
    BCM_LOG_DEBUG( BCM_LOG_ID_GMAC, "Disable GMAC Rx (clear bitMask 0x01)" ); 
    GMAC_MAC->Cmd.rx_ena = 0;
    GMAC_MAC->Cmd.tx_ena = 0;

#if defined(CONFIG_BLOG)
    fcacheDrvFlushAll();
#endif

    netif_device_detach( phyPortId_to_netdev(GMAC_PORT_ID, -1) );

    /* delete the GMAC channel */
    enet_del_rxdma_channel(GMAC_LOG_CHAN);
    enet_del_txdma_channel(GMAC_LOG_CHAN);

    gmac_info_pg->active = 0;    /* GMAC inactive and ROBO port is active */

    /* Add the ROBO channel */
    enet_add_txdma_channel(GMAC_LOG_CHAN);
    enet_add_rxdma_channel(GMAC_LOG_CHAN);

    /* Select ROBO at Mux */
#if defined(CONFIG_BCM963268)
    BCM_LOG_DEBUG( BCM_LOG_ID_GMAC, "Select ROBO at Mux (clear ~0x40000)" ); 
    GPIO->RoboswGphyCtrl &= ~GPHY_MUX_SEL_GMAC;

    BCM_LOG_DEBUG( BCM_LOG_ID_GMAC, "\tGPIORoboswGphyCtrl<0x%p>=0x%x", 
        &GPIO->RoboswGphyCtrl, (uint32_t) GPIO->RoboswGphyCtrl );
#endif

    netif_device_attach( phyPortId_to_netdev(GMAC_PORT_ID, -1) );

    enet_txdma_channel_enable(GMAC_LOG_CHAN);
    enet_rxdma_channel_enable(GMAC_LOG_CHAN);
    gmac_info_pg->trans = 0;

    BCM_LOG_DEBUG( BCM_LOG_ID_GMAC, "GMAC Deactivated (active=%d)",
        gmac_info_pg->active );

    return 0;
}

void gmac_dump_status( void )
{
    printk(
        "\n============ GMAC Status ============\n"
        "enabled = %u wan = %u mode = %u active = %u\n" 
        "link_up = %u link_speed = %u\n"
        "max Frame Len = %d \n"
        "=====================================\n",
        gmac_info_pg->enabled, gmac_info_pg->wan, 
        gmac_info_pg->mode, gmac_info_pg->active,
        gmac_info_pg->link_up, gmac_info_pg->link_speed, 
        gmac_mac_get_max_frm_len());
}


/* Based on the current mode makes the port active or inactive
 * CAUTION!!!: GMAC should be enabled for this function
 *
 * When GMAC is active make sure:
 * - ROBO RX & TX are disabled in port control reg
 * - The link status is DOWN in port override register
 *
 * When ROBO is active make sure:
 * - ROBO RX & TX are enabled in port control reg
 * - The link status is UP in port override register
 */
void gmac_adjust_config( void )
{
    switch( gmac_info_pg->mode )
    {
        case GMAC_MODE_ROBO_PORT:   /* ROBO port is always active */
            /* If new mode is ROBO port, and GMAC is already active then 
             * make the GMAC as inactive */
            if (gmac_info_pg->active)
            {
                gmac_set_inactive();
                /* Enable ROBO RX & TX */
                enet_set_port_ctrl( GMAC_PORT_ID, 0 );
            }
            break;

        case GMAC_MODE_LINK_SPEED: /* active port selected based on speed */
            if (gmac_info_pg->link_up) 
            {
                /*  
                 *  Link_UP + 1000 Mbps + WAN  -> GMAC active
                 *  otherwise -> ROBO port active
                 */
                if (gmac_info_pg->active)
                {
                    if (gmac_info_pg->link_up && gmac_info_pg->link_speed == 1000 &&
                            gmac_info_pg->wan) 
                    {
                        /* Disable ROBO RX */
                        enet_set_port_ctrl( GMAC_PORT_ID, 1 );
                        ethsw_set_mac_hw2(0, GMAC_PORT_ID, 0, 100, gmac_info_pg->duplex);
                    }
                    else
                    {
                        gmac_set_inactive();

                        /* Enable ROBO RX & TX */
                        enet_set_port_ctrl( GMAC_PORT_ID, 0 );
                        ethsw_set_mac_hw2(0, GMAC_PORT_ID, 1, 100, gmac_info_pg->duplex);
                    }
                }
                else
                {
                    if (! (gmac_info_pg->link_up && gmac_info_pg->link_speed == 1000 &&
                                gmac_info_pg->wan) )
                    {
                        /* Enable ROBO RX & TX */
                        enet_set_port_ctrl( GMAC_PORT_ID, 0 );
                        ethsw_set_mac_hw2(0, GMAC_PORT_ID, 1, 100, gmac_info_pg->duplex);
                    }
                    else
                    {
                        /* Disable ROBO RX */
                        enet_set_port_ctrl( GMAC_PORT_ID, 1 );
                        ethsw_set_mac_hw2(0, GMAC_PORT_ID, 0, 100, gmac_info_pg->duplex);
                        gmac_set_active();
                    }
                }
            }
            else
            {
                enet_set_port_ctrl( GMAC_PORT_ID, 0 );
                ethsw_set_mac_hw2(0, GMAC_PORT_ID, 0, 100, gmac_info_pg->duplex);
            }
            break;
    }
}


/* Sets the current mode, and GMAC to be active or inactive based on 
 * the mode */
/* CAUTION: GMAC should be enabled for this function */ 
void gmac_set_mode( int new_mode )
{
    BCM_ASSERT( new_mode <= GMAC_MODE_LINK_SPEED );
    BCM_ASSERT( gmac_info_pg->enabled == 1 );

    BCM_LOG_DEBUG( BCM_LOG_ID_GMAC, "New: mode=%d", new_mode );

    gmac_dbg_status( "gmac_set_mode() : Prev Status: " );
    gmac_info_pg->mode = new_mode;

    gmac_dbg_status( "gmac_set_mode() : Final Status: " );
    /* set mode is treated similar to link change, by restarting autoneg */
    enet_restart_autoneg( enet_gmac_log_port() );
}


/* Sets the GMAC to be active or inactive based on the link speed 
 * Also updates the current link speed and status */
/* CAUTION: GMAC should be enabled for this function */ 
void gmac_link_status_changed(int link_status, int speed, int duplex)
{
    BCM_ASSERT( gmac_info_pg->enabled == 1 );
    BCM_ASSERT( (link_status <= 1) && (duplex <= 1) );

    BCM_LOG_DEBUG( BCM_LOG_ID_GMAC, "New link status: "
        "link_status=%d, speed=%d, duplex=%d", 
        link_status, speed, duplex );

    gmac_dbg_status( "gmac_link_status_changed() : Prev Status: " );

    gmac_info_pg->link_up = link_status;
    gmac_info_pg->link_speed = speed;
    gmac_info_pg->duplex = duplex;
        gmac_dbg_status( "Prev Status: " );
        gmac_adjust_config();
        gmac_dbg_status( "Final Status: " );
    }


extern int ephy_int_cnt;
/*
 * bcm63xx_gmac_isr: Acknowledge GMAC interrupt.
 */
FN_HANDLER_RT bcm63xx_gmac_isr(int irq, void * dev_id)
{
    /* PHY interrupt is disabled here */
    ephy_int_cnt++;

    /* re-enable PHY interrupt */
    bcmeapiPhyIntEnable(1);
    return BCM_IRQ_HANDLED;
}


/* Reads the stats from GMAC Regs */
void gmac_hw_stats( int port,  struct rtnl_link_stats64 *stats)
{
    BCM_ASSERT( port == GMAC_PORT_ID );
    BCM_ASSERT( stats != NULL ) ;

    if (gmac_info_pg->enabled && (port == GMAC_PORT_ID))
    {
        volatile GmacMIBRegs *e = (volatile GmacMIBRegs *)GMAC_MIB;
        unsigned int v1, v2;

        stats->rx_packets += e->RxPkts;
        stats->rx_bytes +=  e->RxOctetsLo;
        stats->multicast += e->RxMulticastPkts;
        stats->rx_broadcast_packets += e->RxBroadcastPkts;		
        v1 = e->RxPkts; v2 = e->RxGoodPkts;
        if (v1 > v2) stats->rx_dropped += v1 - v2;
        stats->rx_errors +=  
            (e->RxFCSErrs + e->RxAlignErrs + e->RxSymbolError);
			
        stats->tx_packets +=  e->TxPkts;
        stats->tx_bytes +=  e->TxOctetsLo;
        stats->tx_multicast_packets += e->TxMulticastPkts;
        stats->tx_broadcast_packets += e->TxBroadcastPkts;		
        v1 = e->TxPkts; v2 = e->TxGoodPkts;
        if (v1 > v2) stats->tx_dropped += v1 - v2;
    }
}


/* Dumps the MIB from GMAC MIB Regs */
int gmac_dump_mib(int port, int type)
{
    volatile GmacMIBRegs *e = (volatile GmacMIBRegs *)GMAC_MIB;

    BCM_ASSERT( port == GMAC_PORT_ID );
    BCM_ASSERT( type <= 1 ) ;

    if (!(gmac_info_pg->enabled && (port == GMAC_PORT_ID) ) )
        return -1;
   
    /* Display Tx statistics */
    printk("\n");
    printk("TxUnicastPkts:          %10u \n", e->TxUnicastPkts);
    printk("TxMulticastPkts:        %10u \n",  e->TxMulticastPkts);
    printk("TxBroadcastPkts:        %10u \n", e->TxBroadcastPkts);
    printk("TxDropPkts:             %10u \n", (e->TxPkts - e->TxGoodPkts));

    /* Display remaining tx stats only if requested */
    if (type) {
        printk("TxOctetsLo:             %10u \n", e->TxOctetsLo);
        printk("TxOctetsHi:             %10u \n", 0);
        printk("TxQoSPkts:              %10u \n", e->TxGoodPkts);
        printk("TxCol:                  %10u \n", e->TxCol);
        printk("TxSingleCol:            %10u \n", e->TxSingleCol);
        printk("TxMultipleCol:          %10u \n", e->TxMultipleCol);
        printk("TxDeferredTx:           %10u \n", e->TxDeferredTx);
        printk("TxLateCol:              %10u \n", e->TxLateCol);
        printk("TxExcessiveCol:         %10u \n", e->TxExcessiveCol);
        printk("TxFrameInDisc:          %10u \n", 0);
        printk("TxPausePkts:            %10u \n", e->TxPausePkts);
        printk("TxQoSOctetsLo:          %10u \n", e->TxOctetsLo);
        printk("TxQoSOctetsHi:          %10u \n", 0);
    }

    /* Display Rx statistics */
    printk("\n");
    printk("RxUnicastPkts:          %10u \n", e->RxUnicastPkts);
    printk("RxMulticastPkts:        %10u \n", e->RxMulticastPkts);
    printk("RxBroadcastPkts:        %10u \n", e->RxBroadcastPkts);
    printk("RxDropPkts:             %10u \n", (e->RxPkts - e->RxGoodPkts));

    /* Display remaining rx stats only if requested */
    if (type) {
        printk("RxJabbers:              %10u \n", e->RxJabbers);
        printk("RxAlignErrs:            %10u \n", e->RxAlignErrs);
        printk("RxFCSErrs:              %10u \n", e->RxFCSErrs);
        printk("RxFragments:            %10u \n", e->RxFragments);
        printk("RxOversizePkts:         %10u \n", e->RxOversizePkts);
        printk("RxExcessSizeDisc:       %10u \n", e->RxExcessSizeDisc);
        printk("RxOctetsLo:             %10u \n", e->RxOctetsLo);
        printk("RxOctetsHi:             %10u \n", 0);
        printk("RxUndersizePkts:        %10u \n", e->RxUndersizePkts);
        printk("RxPausePkts:            %10u \n", e->RxPausePkts);
        printk("RxGoodOctetsLo:         %10u \n", e->RxOctetsLo);
        printk("RxGoodOctetsHi:         %10u \n", 0);
        printk("RxSAChanges:            %10u \n", 0);
        printk("RxSymbolError:          %10u \n", e->RxSymbolError);
        printk("RxQoSPkts:              %10u \n", e->RxGoodPkts);
        printk("RxQoSOctetsLo:          %10u \n", e->RxOctetsLo);
        printk("RxQoSOctetsHi:          %10u \n", 0);
        printk("RxPkts64Octets:         %10u \n", e->Pkts64Octets);
        printk("RxPkts65to127Octets:    %10u \n", e->Pkts65to127Octets);
        printk("RxPkts128to255Octets:   %10u \n", e->Pkts128to255Octets);
        printk("RxPkts256to511Octets:   %10u \n", e->Pkts256to511Octets);
        printk("RxPkts512to1023Octets:  %10u \n", e->Pkts512to1023Octets);
        printk("RxPkts1024to1522Octets: %10u \n", 
            (e->Pkts1024to1518Octets + e->Pkts1519to1522));
        printk("RxPkts1523to2047:       %10u \n", e->Pkts1523to2047);
        printk("RxPkts2048to4095:       %10u \n", e->Pkts2048to4095);
        printk("RxPkts4096to8191:       %10u \n", e->Pkts4096to8191);
        printk("RxPkts8192to9728:       %10u \n", 0);
    }
    return 0;
}


/* Resets the GMAC MIB Regs */
void gmac_reset_mib( void )
{
    if (gmac_info_pg->enabled)
    {
        /* clear the MIB */
        GMAC_INTF->MibCtrl.clrMib = 1;
        GMAC_INTF->MibCtrl.clrMib = 0;
    }
}


int gmac_get_mib_max_pkt_size( void )
{
    return (GMAC_INTF->MibMaxPktSize.max_pkt_size);
}

void gmac_intf_set_mib_max_pkt_size( int pkt_size )
{
    GMAC_INTF->MibMaxPktSize.max_pkt_size = pkt_size;
}

int gmac_intf_get_rx_bp_thresh_lo( void )
{
    return (GMAC_INTF->RxBpThreshLo.rx_thresh);
}

/* Sets the RX Back pressure threshold low (in multiple of 16 bytes)  
 * def value (0x200) is 1/2 of RX FIFO depth, 
 * range: 0-0x400
 */
void gmac_intf_set_rx_bp_thresh_lo( int thresh_lo )
{
    BCM_ASSERT( thresh_lo <= GMAC_RB_BP_THRESH_MAX ) ;
    GMAC_INTF->RxBpThreshLo.rx_thresh = thresh_lo;
}

int gmac_intf_get_rx_bp_thresh_hi( void )
{
    return (GMAC_INTF->RxBpThreshHi.rx_thresh);
}


/* Sets the RX Back pressure threshold high (in multiple of 16 bytes)  
 * def value (0x300) is 3/4 of RX FIFO depth, 
 * range: 0-0x400
 */
void gmac_intf_set_rx_bp_thresh_hi( int thresh_hi )
{
    BCM_ASSERT( thresh_hi <= GMAC_RB_BP_THRESH_MAX ) ;
    GMAC_INTF->RxBpThreshHi.rx_thresh = thresh_hi;
}

int gmac_intf_get_rx_pause_flow_ctrl( void )
{
    return (GMAC_INTF->RxFlowCtrl.pause_en);
}


/* Sets the RX Flow Control
 * Bit[1]: Reset value 0x0
 *      1: Enable Pause control to ROBOSW, GMAC sends Pause frame (XON/XOFF)
 *         to ROBOSw based on GMAC RX FIFO level w.r.t. thresholds (hi/lo).
 *      0: Disable Pause control to ROBOSW
 *
 * Bit[0]: Reset value 0x0
 *      1: Enable flow control (side-band signal) to ROBOSW, GMAC asserts 
 *         flow control to ROBOSW based on GMAC RX FIFO level w.r.t.
 *         thresholds(hi/lo). 
 *      0: Disable flow control to ROBOSW
 *
 * NOTE:
 *      For 63268D0 device only Bit[1] will be useful, as in this case the
 *      GMAC can send pause frame to the MAC on the other side of the link.
 *      We cannot use the Bit[1] feature on 63268D0 because the GMAC will not
 *      be connected to ROBOSW.
 */
void gmac_intf_set_rx_pause_flow_ctrl( int pause_en )
{
    BCM_ASSERT( pause_en <= 1 ) ;
    GMAC_INTF->RxFlowCtrl.pause_en = pause_en;
}


int gmac_mac_get_max_frm_len( void )
{
    return (GMAC_MAC->FrmLen.frm_len);
}

/* Need to set the frame length for Jumbo packets
   Jumbo frame: 1518 < rx_pkt_len <= the config frm_len */
void gmac_mac_set_max_frm_len( int frm_len )
{
    BCM_ASSERT( frm_len <= GMAC_MAX_JUMBO_FRM_LEN ) ;

    GMAC_MAC->FrmLen.frm_len = frm_len;
}

int gmac_mac_get_pause_ctrl( int *pause_timer_p )
{
    BCM_ASSERT( pause_timer_p != NULL );

    *pause_timer_p = GMAC_MAC->PauseCtrl.pause_timer;
    return (GMAC_MAC->PauseCtrl.pause_en);
}

/* Enable/Disable the pause control and
 * sets the pause timer */
void gmac_mac_set_pause_ctrl( int pause_en, int pause_timer )
{
    BCM_ASSERT( pause_en <= 1 ) ;

    GMAC_MAC->PauseCtrl.pause_timer = pause_timer;
    GMAC_MAC->PauseCtrl.pause_en = pause_en;
}


int gmac_mac_get_pause_quanta( void )
{
    return (GMAC_MAC->PauseQuanta.pause_quanta);
}

void gmac_mac_set_pause_quanta( int pause_quanta )
{
    GMAC_MAC->PauseQuanta.pause_quanta = pause_quanta; 
}


/* Sets the various flow control regs
 * Pause Enable:
 *      If the thresholds are 0, it sets them to default values
 * Pause Disable:
 *      Only pause control bit is cleared, all other regs are not changed
 */
void gmac_config_flow_control( int fc_ena, int thresh_lo, int thresh_hi,
    int rep_pause, int pause_timer, int pause_quanta )
{
    BCM_ASSERT( thresh_lo <= GMAC_RB_BP_THRESH_MAX ) ;
    BCM_ASSERT( thresh_hi <= GMAC_RB_BP_THRESH_MAX ) ;

    if ( fc_ena )
    {
        if (!thresh_lo)
            thresh_lo = GMAC_RB_BP_THRESH_LO_DEF;  /* Use default */
        gmac_intf_set_rx_bp_thresh_lo( thresh_lo );

        if (!thresh_hi)
            thresh_hi = GMAC_RB_BP_THRESH_LO_DEF;  /* Use default */
        gmac_intf_set_rx_bp_thresh_hi( thresh_hi );

        GMAC_MAC->Cmd.tx_pause_ign = 0;
        gmac_mac_set_pause_quanta( pause_quanta );
        gmac_mac_set_pause_ctrl( rep_pause, pause_timer );
    }

    gmac_intf_set_rx_pause_flow_ctrl( fc_ena );
}

/*
 * Set GMAC frame length and mib max packet size. 
 * Called when Jumbo frame is configured.
 * The code is limiting Jumbo frame payload size to 2000 bytes.
 * If higher values are desired, need to take care of Rx ring buffer 
 * size allocation as well - ENET_MAX_MTU_PAYLOAD_SIZE.
 */
void gmac_intf_set_max_pkt_size(int pkt_size)
{
    gmac_intf_set_mib_max_pkt_size(pkt_size);
    gmac_mac_set_max_frm_len(pkt_size);
    BCM_LOG_DEBUG( BCM_LOG_ID_GMAC,"%s: Set GMAC frame len %d \n",
                                 __FUNCTION__, pkt_size);
}

/*
 *------------------------------------------------------------------------------
 * Function Name: gmac_drv_ioctl
 * Description  : Main entry point to handle user applications IOCTL requests
 *                GMAC Utility.
 * Returns      : 0 - success or error
 *------------------------------------------------------------------------------
 */
static int gmac_drv_ioctl(struct inode *inode, struct file *filep,
                       unsigned int command, unsigned long arg)
{
    gmacctl_ioctl_t cmd;
    gmacctl_data_t gmac;
    gmacctl_data_t *gmac_p = &gmac;
    int ret = GMAC_SUCCESS;

    BCM_ASSERT( (inode != NULL) && (filep != NULL) );

    if ( gmac_info_pg->enabled != 1 )
    {
        BCM_LOG_ERROR( BCM_LOG_ID_GMAC, "GMAC not enabled on this device");
        return GMAC_ERROR;
    }

    /* Is the GMAC/ROBO port configured as WAN port? */
    if ( ! (gmac_info_pg->wan) )
    {
        BCM_LOG_ERROR( BCM_LOG_ID_GMAC,
            "GMAC/ROBO port is not configured as WAN port");
        return GMAC_ERROR;
    }

    if ( command > GMACCTL_IOCTL_MAX )
        cmd = GMACCTL_IOCTL_MAX;
    else
        cmd = (gmacctl_ioctl_t)command;

    copy_from_user( gmac_p, (uint8_t *) arg, sizeof(gmac) );

    BCM_LOG_DEBUG( BCM_LOG_ID_GMAC, 
        "cmd<%d>%s subsys<%d>%s op<%d>%s arg<0x%lx>",
        command, gmacctl_ioctl_name[command], 
        gmac_p->subsys, gmacctl_subsys_name[gmac_p->subsys],
        gmac_p->op, gmacctl_op_name[gmac_p->op], arg );

    GMAC_LOCK_BH();

    if (cmd == GMACCTL_IOCTL_SYS)
    {
        switch (gmac_p->subsys)
        {
            case GMACCTL_SUBSYS_MODE:
            {
                switch (gmac_p->op)
                {
                    case GMACCTL_OP_GET:
                    {
                        gmac_p->mode = gmac_info_pg->mode;
                        copy_to_user( (uint8_t *)arg, gmac_p, sizeof(gmac) );
                        break;
                    }

                    case GMACCTL_OP_SET:
                    {
                        if (gmac_info_pg->mode != gmac_p->mode)
                            gmac_set_mode( gmac_p->mode );
                        break;
                    }

                    default:
                        BCM_LOG_ERROR( BCM_LOG_ID_GMAC,
                            "Invalid op[%u]", gmac_p->op );
                }
                break;
            }

            case GMACCTL_SUBSYS_STATUS:
            {
                switch (gmac_p->op)
                {
                    case GMACCTL_OP_DUMP:
                        gmac_dump_status();
                        break;

                    default:
                        BCM_LOG_ERROR( BCM_LOG_ID_GMAC,
                            "Invalid op[%u]", gmac_p->op );
                        ret = GMAC_ERROR;
                }
                break;
            }

            case GMACCTL_SUBSYS_MIB:
            {
                switch (gmac_p->op)
                {
                    case GMACCTL_OP_DUMP:
                        gmac_dump_mib(GMAC_PORT_ID, gmac_p->mib);
                        break;

                    default:
                        BCM_LOG_ERROR( BCM_LOG_ID_GMAC,
                            "Invalid op[%u]", gmac_p->op );
                        ret = GMAC_ERROR;
                }
                break;
            }

            default:
                BCM_LOG_ERROR( BCM_LOG_ID_GMAC,
                    "Invalid subsys[%u]", gmac_p->subsys );
                ret = GMAC_ERROR;
        }
    }
    else
    {
        BCM_LOG_ERROR(BCM_LOG_ID_GMAC, "Invalid cmd[%u]", command );
        ret = GMAC_ERROR;
    }

    GMAC_UNLOCK_BH();

    return ret;

} /* gmac_drv_ioctl */

static DEFINE_MUTEX(gmacIoctlMutex);

static long gmac_drv_ioctl_unlocked(struct file *filep, unsigned int cmd, 
                               unsigned long arg)
{
    struct inode *inode;
    long rt;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)
    inode = filep->f_dentry->d_inode;
#else
    inode = file_inode(filep);
#endif


    mutex_lock(&gmacIoctlMutex);
    rt = gmac_drv_ioctl(inode, filep, cmd, arg);
    mutex_unlock(&gmacIoctlMutex);

    return rt;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: gmac_drv_open
 * Description  : Called when a user application opens this device.
 * Returns      : 0 - success
 *------------------------------------------------------------------------------
 */
static int gmac_drv_open(struct inode *inode, struct file *filep)
{
    BCM_ASSERT( (inode != NULL) && (filep != NULL) );

    BCM_LOG_DEBUG( BCM_LOG_ID_GMAC, "Access GMAC Char Device" );
    return GMAC_SUCCESS;
} /* gmac_drv_open */

/* Global file ops */
static struct file_operations gmac_fops =
{
    .unlocked_ioctl = gmac_drv_ioctl_unlocked,
#if defined(CONFIG_COMPAT)
    .compat_ioctl = gmac_drv_ioctl_unlocked,
#endif
    .open   = gmac_drv_open,
};

/*
 *------------------------------------------------------------------------------
 * Function Name: gmac_drv_construct
 * Description  : Initial function that is called at system startup that
 *                registers this device.
 * Returns      : None.
 *------------------------------------------------------------------------------
 */

static int gmac_drv_construct(void)
{
    bcmLog_setLogLevel( BCM_LOG_ID_GMAC, BCM_LOG_LEVEL_NOTICE );

    if ( register_chrdev( GMAC_DRV_MAJOR, GMAC_DRV_NAME, &gmac_fops ) )
    {
        BCM_LOG_ERROR( BCM_LOG_ID_GMAC, 
                "%s Unable to get major number <%d>" CLRnl,
                  __FUNCTION__, GMAC_DRV_MAJOR);
        return GMAC_ERROR;
    }

    printk( GMAC_MODNAME " Char Driver " GMAC_VER_STR " Registered<%d>" 
                                                        CLRnl, GMAC_DRV_MAJOR );

    return GMAC_DRV_MAJOR;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: gmac_drv_destruct
 * Description  : Final function that is called when the module is unloaded.
 * Returns      : None.
 *------------------------------------------------------------------------------
 */
static void gmac_drv_destruct(void)
{
    unregister_chrdev( GMAC_DRV_MAJOR, GMAC_DRV_NAME );

    printk( GMAC_MODNAME " Char Driver " GMAC_VER_STR " Unregistered<%d>" 
                                                        CLRnl, GMAC_DRV_MAJOR );
}

#if defined(CONFIG_BCM963268)
static void gmac_init_63268( void )
{
    /* Set bits [14:12] to 0xD to achieve 1G over iuDMA */
    GPIO->RoboswSwitchCtrl = 
        (GPIO->RoboswSwitchCtrl & ~RSW_IUDMA_CLK_FREQ_MASK) |
        (3<<RSW_IUDMA_CLK_FREQ_SHIFT);

    BCM_LOG_DEBUG( BCM_LOG_ID_GMAC, 
        "Set bits[14:12]=0x3 RSW IUDMA CLK Freq<0x%p> = 0x%x", 
        &GPIO->RoboswSwitchCtrl, (unsigned int) GPIO->RoboswSwitchCtrl );

    /* Enable GMAC clock */
    PERF->blkEnables |= GMAC_CLK_EN;

    BCM_LOG_DEBUG( BCM_LOG_ID_GMAC, 
        "Enable GMAC clock (bit19=0x80000) blkEnables<0x%p>=0x%x", 
        &PERF->blkEnables, (unsigned int) PERF->blkEnables );

    MISC->miscIddqCtrl &= ~MISC_IDDQ_CTRL_GMAC;

    BCM_LOG_DEBUG( BCM_LOG_ID_GMAC, 
        "Cleared IDDQ bit (0x40000) miscIddqCtrl<0x%p> = 0x%x", 
        &MISC->miscIddqCtrl, (unsigned int) MISC->miscIddqCtrl );
}
#endif


void gmac_init_default( void )
{
    /* Reset GMAC */
    BCM_LOG_DEBUG( BCM_LOG_ID_GMAC, 
        "Toggling Cmd to SW Reset (clear bitMask=0x2000)" );
    GMAC_MAC->Cmd.sw_reset = 1;
    GMAC_MAC->Cmd.sw_reset = 0;

    BCM_LOG_DEBUG( BCM_LOG_ID_GMAC, 
        "\tCmd<0x%p>=0x%x", 
        &GMAC_MAC->Cmd.word, (int) GMAC_MAC->Cmd.word ); 

    BCM_LOG_DEBUG( BCM_LOG_ID_GMAC, 
        "Toggling MacSwReset (clear bitMask=0x07)" );
    GMAC_INTF->MacSwReset.txfifo_flush = 1;
    GMAC_INTF->MacSwReset.rxfifo_flush = 1;
    GMAC_INTF->MacSwReset.mac_sw_reset = 1;

    GMAC_INTF->MacSwReset.txfifo_flush = 0;
    GMAC_INTF->MacSwReset.rxfifo_flush = 0;
    GMAC_INTF->MacSwReset.mac_sw_reset = 0;

    BCM_LOG_DEBUG( BCM_LOG_ID_GMAC, 
        "\tMacSwReset<0x%p>=0x%x", 
        &GMAC_INTF->MacSwReset.word, (uint32_t) GMAC_INTF->MacSwReset.word );

    gmac_reset_mib();
#if defined(CONFIG_BCM_JUMBO_FRAME)
    gmac_intf_set_max_pkt_size(GMAC_MAX_JUMBO_FRM_LEN);
#else
    gmac_intf_set_max_pkt_size(GMAC_MAX_FRM_LEN);
#endif
    gmac_config_flow_control( 1, GMAC_RB_BP_THRESH_LO_DEF,
        GMAC_RB_BP_THRESH_LO_DEF, 1, 0xffff, 0xffff );

    /* default CMD configuration */
    GMAC_MAC->Cmd.runt_filt_dis = 0;
    GMAC_MAC->Cmd.txrx_en_cfg = 0;
    GMAC_MAC->Cmd.tx_pause_ign = 0;
    GMAC_MAC->Cmd.rmt_loop_ena = 0;
    GMAC_MAC->Cmd.len_chk_dis = 0;
    GMAC_MAC->Cmd.ctrl_frm_ena = 0;
    GMAC_MAC->Cmd.ena_ext_cfg = 0;
    GMAC_MAC->Cmd.lcl_loop_ena = 0;
    GMAC_MAC->Cmd.hd_ena = 0;
    GMAC_MAC->Cmd.tx_addr_ins = 0;
    GMAC_MAC->Cmd.rx_pause_ign = 0;
    GMAC_MAC->Cmd.pause_fwd = 1;
    GMAC_MAC->Cmd.crc_fwd = 1;
    GMAC_MAC->Cmd.pad_rem_en = 0;
    GMAC_MAC->Cmd.promis_en = 1;
    GMAC_MAC->Cmd.eth_speed = CMD_ETH_SPEED_1000;

    /* mib_rsv_select [3:0] will mux the status bit for IUDMA RX framestat[6]
       mib_rsv_select [7:4] will mux the status bit for IUDMA RX framestat[7]
       mib_rsv_select [11:8] will mux the status bit for IUDMA RX framestat[8]

       The decoding is as follows (this is from RTL)

       4'h1: rsv_31_mux = RSV[16];  // Packet Skip
       4'h2: rsv_31_mux = RSV[17];  // Stack VLAN
       4'h3: rsv_31_mux = RSV[18];  // Carrier Event
       4'h4: rsv_31_mux = RSV[21];  // Frame Length Incorret
       4'h5: rsv_31_mux = RSV[22];  // Frame Length Out of Range
       4'h6: rsv_31_mux = RSV[23];  // Receive OK
       4'h7: rsv_31_mux = RSV[26];  // Dribble Nibble
       4'h8: rsv_31_mux = RSV[28];  // Pause Frame
       4'h9: rsv_31_mux = RSV[29];  // Unsupported opcode
       4'ha: rsv_31_mux = RSV[34];  // PPP Frame
       4'hb: rsv_31_mux = RSV[35];  // Rx CRC match

       Default (when select is not any of the above values, e.g., 0) is   

       DMA RX Status[8] = RSV[33] RUNT detected.
       DMA RX Status[7] = RSV[32] Frame Truncated.
       DMA RX Status[6] = RSV[31] Unicast Detected.
       Setting the mib_rsv_select [7:4] bits to 5 to detect frame out of
       range conditions (for example: GMAC receives a frame greater than
       1518 bytes but sets the length to 1518 and does not increase
       RxGoodPkt counter */
    GMAC_INTF->DmaRxStatusSel.word = 0x50;

    /* Disable Tx and Rx */
    BCM_LOG_DEBUG( BCM_LOG_ID_GMAC, 
        "Disable Rx & Tx (set bitMask 0x00)" ); 
    GMAC_MAC->Cmd.tx_ena = 0;
    GMAC_MAC->Cmd.rx_ena = 0;

    BCM_LOG_DEBUG( BCM_LOG_ID_GMAC, "\tCmd<0x%p>=0x%x", 
        &GMAC_MAC->Cmd.word, (uint32_t) GMAC_MAC->Cmd.word ); 
}

int gmac_init( void )
{
    if (gmac_drv_construct() == GMAC_ERROR)
        return GMAC_ERROR;

    memset( gmac_info_pg, 0, sizeof(*gmac_info_pg) );

    gmac_info_pg->enabled = 0;          /* GMAC is not enabled */
    gmac_info_pg->active = 0;           /* ROBO port is active at init */
    gmac_info_pg->wan = 0;              /* Not a WAN port */
    gmac_info_pg->link_speed = 0;       /* Link speed is 0Mbps */
    gmac_info_pg->link_up = 0;          /* Link is down */
    gmac_info_pg->log_chan = GMAC_LOG_CHAN; /* Log chan for GMAC port */
    gmac_info_pg->phy_chan = GMAC_PHY_CHAN; /* Phy chan for GMAC port */
    gmac_info_pg->trans = 0;

    gmac_info_pg->mode = GMAC_MODE_DEF; 
    gmac_info_pg->chip_id = kerSysGetChipId();
    gmac_info_pg->rev_id = (int) (PERF->RevID & REV_ID_MASK);

    if( gmac_is_gmac_supported() )
    {
        gmac_info_pg->enabled = 1;  /* GMAC present and enabled */
        gmac_info_pg->mode = GMAC_MODE_LINK_SPEED;
    }

 
    BCM_LOG_DEBUG( BCM_LOG_ID_GMAC, 
        "\n============ GMAC Config Begins ============\n"
        "enabled = %u wan = %u active = %u mode = %u\n" 
        "chip_id = %5X rev_id = %2X\n" 
        "log_chan = %u phy_chan = %u\n" 
        "link_up = %u link_speed = %u\n"
        "============ GMAC Config Ends ==============\n",
        gmac_info_pg->enabled, gmac_info_pg->wan, 
        gmac_info_pg->active, gmac_info_pg->mode,
        gmac_info_pg->chip_id, gmac_info_pg->rev_id,
        gmac_info_pg->log_chan, gmac_info_pg->phy_chan,
        gmac_info_pg->link_up, gmac_info_pg->link_speed );

    if (gmac_info_pg->enabled)
    {
        BCM_LOG_DEBUG( BCM_LOG_ID_GMAC, 
            "============ GMAC Init Begins ============" ); 

#if defined(CONFIG_BCM963268)
        gmac_init_63268();
#else 
#error "ERROR - GMAC driver not supported for this chip"
#endif 

        gmac_init_default();

        BCM_LOG_DEBUG( BCM_LOG_ID_GMAC, 
            "=========== GMAC Init Ends ==============\n"); 
    }
    
    printk( GMAC_MODNAME " Driver " GMAC_VER_STR " Initialized\n" ); 
    return GMAC_SUCCESS;
}

/*
 *------------------------------------------------------------------------------
 * Function     : gmac_exit
 * Description  : Destruction of GMAC driver.
 *------------------------------------------------------------------------------
 */
void gmac_exit(void)
{
    gmac_drv_destruct();
    printk( GMAC_MODNAME " Driver " GMAC_VER_STR " Uninitialized\n" ); 
}


EXPORT_SYMBOL( gmac_info_pg );
EXPORT_SYMBOL( gmac_set_active );
EXPORT_SYMBOL( gmac_set_inactive );
EXPORT_SYMBOL( gmac_link_status_changed ); 
EXPORT_SYMBOL( gmac_hw_stats );
EXPORT_SYMBOL( gmac_dump_mib );
EXPORT_SYMBOL( gmac_reset_mib );
EXPORT_SYMBOL( bcm63xx_gmac_isr );
EXPORT_SYMBOL( gmac_init );

