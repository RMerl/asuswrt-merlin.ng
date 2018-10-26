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

/** Includes. **/
#include "lib_types.h"
#include "lib_malloc.h"
#include "lib_string.h"
#include "lib_printf.h"

#include "cfe_iocb.h"
#include "cfe_ioctl.h"
#include "cfe_device.h"
#include "cfe_devfuncs.h"
#include "cfe_timer.h"
#include "dev_bcm63xx_eth.h"
#include "dev_bcm63xx_flash.h"
#include "mii_shared.h"
#include "addrspace.h"
#if !defined(_BCM94908_)
#include "robosw_reg.h"
#else
#include "bcm_ethsw.h"
#endif

#if defined(_BCM94908_) && defined(CFG_2P5G_LAN)
#include "phy_drv.h"
#include "phy_bp_parsing.h"

static phy_dev_t *cb_phy_devices[BP_MAX_CROSSBAR_EXT_PORTS];
static ETHERNET_MAC_INFO *emac_info;
static int found_2p5g_lan = 0;
#endif /* _BCM94908_ && CFG_2P5G_LAN */

#define DMA_RX_CHAN         (softc->dmaPort * 2)
#define DMA_TX_CHAN         (softc->dmaPort * 2 + 1)

#if defined(_BCM94908_)
#define CACHE_ALIGN     64
#else
#define CACHE_ALIGN     16
#endif
extern void _cfe_flushcache(int, uint8_t *, uint8_t *);
#define INVAL_RANGE(s,l) _cfe_flushcache(CFE_CACHE_INVAL_RANGE,((uint8_t *) (s)),((uint8_t *) (s))+(l))
#define FLUSH_RANGE(s,l) _cfe_flushcache(CFE_CACHE_FLUSH_RANGE,((uint8_t *) (s)),((uint8_t *) (s))+(l))

/** Externs. **/
extern unsigned long sysGetEnetModeFlag(void);

/** Prototypes. **/
static void bcm63xx_ether_probe( cfe_driver_t * drv,    unsigned long probe_a,
                                 unsigned long probe_b, void * probe_ptr );
static int bcm63xx_ether_open(cfe_devctx_t *ctx);
static int bcm63xx_ether_read(cfe_devctx_t *ctx,iocb_buffer_t *buffer);
static int bcm63xx_ether_inpstat(cfe_devctx_t *ctx,iocb_inpstat_t *inpstat);
static int bcm63xx_ether_write(cfe_devctx_t *ctx,iocb_buffer_t *buffer);
static int bcm63xx_ether_ioctl(cfe_devctx_t *ctx,iocb_buffer_t *buffer);
static int bcm63xx_ether_close(cfe_devctx_t *ctx);
static int internal_open(bcmenet_softc * softc);

/** Variables. **/
const static cfe_devdisp_t bcm63xx_ether_dispatch = {
    bcm63xx_ether_open,
    bcm63xx_ether_read,
    bcm63xx_ether_inpstat,
    bcm63xx_ether_write,
    bcm63xx_ether_ioctl,
    bcm63xx_ether_close,
    NULL,
    NULL
};

const cfe_driver_t bcm63xx_enet = {
    "BCM63xx Ethernet",
    "eth",
    CFE_DEV_NETWORK,
    &bcm63xx_ether_dispatch,
    bcm63xx_ether_probe
};

/** Functions. **/
static void bcm63xx_ether_probe( cfe_driver_t * drv,    unsigned long probe_a,
                                unsigned long probe_b, void * probe_ptr )
{
    bcmenet_softc * softc;
    char descr[100];

    softc = (bcmenet_softc *) KMALLOC( sizeof(bcmenet_softc), CACHE_ALIGN );
    if( softc == NULL ) {
        xprintf( "Failed alloc softc mem\n" );
    } else {
        memset( softc, 0, sizeof(bcmenet_softc) );

        if (internal_open( softc ) == -1) 
            xprintf("Failed init enet hw\n");
        else
        {
            xsprintf(descr, "%s eth %d", drv->drv_description, probe_a );
            cfe_attach( drv, softc, NULL, descr );
        }
    }
}

static int bcm63xx_ether_open(cfe_devctx_t *ctx)
{
    /* FIXME -- See if this can be returned to its normal place. */
    return 0;
}

/*
 * init_dma: Initialize DMA control register
 */
static void init_dma(bcmenet_softc *softc)
{
    uint32 *StateRam;
    int i;

    /*
     * clear State RAM
     */
    StateRam = (UINT32 *)&softc->dmaCtrl->stram.s[0];
    for (i = 0; i < sizeof(DmaStateRam) / sizeof(UINT32) * NUM_PORTS * 2; i++)
        StateRam[i] = 0;

    /*
     * initialize IUDMA controller register
     */
    softc->dmaCtrl->controller_cfg = DMA_FLOWC_CH1_EN;
    softc->dmaCtrl->flowctl_ch1_thresh_lo = DMA_FC_THRESH_LO;
    softc->dmaCtrl->flowctl_ch1_thresh_hi = DMA_FC_THRESH_HI;
    softc->dmaCtrl->flowctl_ch1_alloc = 0;

    // transmit
    softc->txDma->cfg = 0;       /* initialize first (will enable later) */
    softc->txDma->maxBurst = DMA_MAX_BURST_LENGTH;
    softc->txDma->intMask = 0;   /* mask all ints */
    /* clr any pending interrupts on channel */
    softc->txDma->intStat = DMA_DONE|DMA_NO_DESC|DMA_BUFF_DONE;
    softc->txDma->intMask = DMA_DONE;
    softc->dmaCtrl->stram.s[DMA_TX_CHAN].baseDescPtr = (uint32)K1_TO_PHYS((uintptr_t)(softc->txFirstBdPtr));

    // receive
    softc->rxDma->cfg = 0;  // initialize first (will enable later)
    softc->rxDma->maxBurst = DMA_MAX_BURST_LENGTH;
    softc->rxDma->intMask = 0;   /* mask all ints */
    /* clr any pending interrupts on channel */
    softc->rxDma->intStat = DMA_DONE|DMA_NO_DESC|DMA_BUFF_DONE;
    softc->rxDma->intMask = DMA_DONE;
    softc->dmaCtrl->stram.s[DMA_RX_CHAN].baseDescPtr = (uint32)K1_TO_PHYS((uintptr_t)(softc->rxFirstBdPtr));
}

#if defined(_BCM94908_) && defined(CFG_2P5G_LAN)
static void copy_crossbar_port_info(ETHERNET_MAC_INFO *emac_info, uint32_t port, EMAC_PORT_INFO *port_info)
{
    port_info->switch_port  = BP_CROSSBAR_PORT_BASE + port;
    port_info->phy_id       = emac_info->sw.crossbar[port].phy_id;
    port_info->phy_id_ext   = emac_info->sw.crossbar[port].phy_id_ext;
    port_info->phyconn      = emac_info->sw.crossbar[port].phyconn;
    port_info->port_flags   = emac_info->sw.crossbar[port].port_flags;
    port_info->phyReset     = emac_info->sw.crossbar[port].phyReset;
#ifdef CFG_2P5G_LAN_DEBUG
    printf("%s: switch_port %d phy_id 0x%x phy_id_ext 0x%x phyconn 0x%x port_flags 0x%x phyReset 0x%x\n",
        __FUNCTION__,
        port_info->switch_port,
	port_info->phy_id,
	port_info->phy_id_ext,
	port_info->phyconn,
	port_info->port_flags,
	port_info->phyReset
	);
#endif /* CFG_2P5G_LAN_DEBUG */
}

static int parse_board_params(void)
{
    int sw, i, idx = 0;

    printf("Parse board Params Start\n");

    if ((emac_info =(ETHERNET_MAC_INFO *)BpGetEthernetMacInfoArrayPtr()) == NULL) {
        printf("Error reading Ethernet MAC info from board params\n");
        return -1;
    }

    phy_drivers_set();

    /* parse crossbar phy devices */
    for (sw = 0 ; sw < BP_MAX_ENET_MACS ; sw++) {
        for (i = 0; i < BP_MAX_CROSSBAR_EXT_PORTS ; i++ ) {
            if (emac_info[sw].sw.crossbar[i].switch_port != BP_CROSSBAR_NOT_DEFINED) {
	        EMAC_PORT_INFO port_info = {};

		copy_crossbar_port_info(&emac_info[sw], i, &port_info);
	        cb_phy_devices[idx] = bp_parse_phy_dev(&port_info);
		if (cb_phy_devices[idx]->mii_type == PHY_MII_TYPE_SERDES)
		    found_2p5g_lan = 1;
#ifdef CFG_2P5G_LAN_DEBUG
		printf("  idx %d cb_phy_devices %p mii_type 0x%x addr 0x%x speed %d phy_id 0x%x\n",
		      idx,
		      cb_phy_devices[idx],
		      cb_phy_devices[idx]->mii_type,
		      cb_phy_devices[idx]->addr,
		      cb_phy_devices[idx]->speed,
		      port_info.phy_id
		    );
#endif /* CFG_2P5G_LAN_DEBUG */
		idx++;
            }
        }
    }

    return 0;
}

static int cb_phy_init(void)
{
    uint32_t i;

    printf("CB PHY Init Start\n");

    phy_drivers_init();

    for (i = 0; i < BP_MAX_CROSSBAR_EXT_PORTS; i++) {
        if (cb_phy_devices[i]) {
            if (phy_dev_init(cb_phy_devices[i])) {
                phy_dev_del(cb_phy_devices[i]);
                cb_phy_devices[i] = NULL;
            }
        }
    }

    return 0;
}

static void crossbar_select(void)
{
    unsigned int val32 = 0;
    volatile unsigned int *cb_mux_reg = (void *)(SWITCH_CROSSBAR_REG);

    val32 =  *cb_mux_reg; /* Locally store current register config */
    printf("%s: cb_mux_reg 0x%x\n", __FUNCTION__, val32);
    val32 &= 0xfffffff0;
    val32 |= 0x4; //wan gphy lan serdes
    *cb_mux_reg = val32;

    return;
}
#endif /* _BCM94908_ && CFG_2P5G_LAN */

static int internal_open(bcmenet_softc * softc)
{
    int i;
    void *p;

#if defined(_BCM960333_)
    /* Always use Enetcore 1 */
    softc->dmaPort = 0;
    softc->dmaCtrl = (DmaRegs *)(ETH1_DMA_BASE);

    /* make sure emac clock is on */
    PERF->blkEnables |= GPHY_ENET_CLK_EN;
#elif defined(_BCM94908_)
    softc->dmaCtrl = (DmaRegs *)(GMAC_DMA_BASE);
#else
    softc->dmaCtrl = (DmaRegs *)(SWITCH_DMA_BASE);
#endif

    softc->rxDma = &softc->dmaCtrl->chcfg[DMA_RX_CHAN];
    softc->txDma = &softc->dmaCtrl->chcfg[DMA_TX_CHAN];

    // If doing SW reboot in EPI the controller can still be active
    softc->rxDma->cfg = 0;
    softc->txDma->cfg = 0;
    softc->dmaCtrl->controller_cfg &= ~DMA_MASTER_EN;

    p = KMALLOC( NR_TX_BDS * sizeof(DmaDesc), CACHE_ALIGN );
    if( p == NULL ) {
        xprintf( "BCM63xx : Fail alloctxBds mem\n" );
        return -1;
    }
    INVAL_RANGE(p, NR_TX_BDS * sizeof(DmaDesc));
    softc->txBds = (DmaDesc *)K0_TO_K1((uintptr_t) p);

    p = KMALLOC( NR_RX_BDS * sizeof(DmaDesc), CACHE_ALIGN );
    if( p== NULL ) {
        xprintf( "BCM63xx : Fail alloc rxBds mem\n" );
        KFREE( (void *)(softc->txBds) );
        softc->txBds = NULL;
        return -1;
    }
    INVAL_RANGE(p, NR_RX_BDS * sizeof(DmaDesc));
    softc->rxBds = (DmaDesc *)K0_TO_K1((uintptr_t) p);

    softc->rxBuffers = (uint8_t*)KMALLOC( NR_RX_BDS * ENET_BUF_SIZE, CACHE_ALIGN );
    if( softc->rxBuffers == NULL ) {
        xprintf( "BCM63xx : Failed allocRxBuffer mem\n" );
        KFREE( (void *)(softc->txBds) );
        softc->txBds = NULL;
        KFREE( (void *)(softc->rxBds) );
        softc->rxBds = NULL;
        return -1;
    }
    INVAL_RANGE(softc->rxBuffers, NR_RX_BDS * ENET_BUF_SIZE);

    softc->txBuffers = (uint8_t*)KMALLOC( NR_TX_BDS * ENET_BUF_SIZE, CACHE_ALIGN );
    if( softc->txBuffers == NULL ) {
        xprintf( "BCM63xx : Failed alloc txBuffer mem\n" );
        KFREE( (void *)(softc->rxBuffers) );
        softc->rxBuffers = NULL;
        KFREE( (void *)(softc->txBds) );
        softc->txBds     = NULL;
        KFREE( (void *)(softc->rxBds) );
        softc->rxBds     = NULL;
        return -1;
    }
    INVAL_RANGE(softc->txBuffers, NR_TX_BDS * ENET_BUF_SIZE);

    /* Init the Receive Buffer Descriptor Ring. */
    softc->rxFirstBdPtr = softc->rxBdReadPtr = softc->rxBds;     
    softc->rxLastBdPtr = softc->rxBds + NR_RX_BDS - 1;

    for(i = 0; i < NR_RX_BDS; i++) {
        (softc->rxBds + i)->status  = DMA_OWN;
        (softc->rxBds + i)->length  = ENET_BUF_SIZE;
        (softc->rxBds + i)->address = K0_TO_PHYS( (uintptr_t)softc->rxBuffers + i * ENET_BUF_SIZE );
        softc->dmaCtrl->flowctl_ch1_alloc = 1;
    }
    softc->rxLastBdPtr->status |= DMA_WRAP;

    /* Init Transmit Buffer Descriptor Ring. */
    softc->txFirstBdPtr = softc->txNextBdPtr =  softc->txBds;
    softc->txLastBdPtr = softc->txBds + NR_TX_BDS - 1;

    for(i = 0; i < NR_TX_BDS; i++) {
        (softc->txBds + i)->status  = 0;
        (softc->txBds + i)->length  = 0;
        (softc->txBds + i)->address = K0_TO_PHYS( (uintptr_t)softc->txBuffers + i * ENET_BUF_SIZE );
    }
    softc->txLastBdPtr->status = DMA_WRAP;

    /* init dma registers */
    init_dma(softc);

    softc->dmaCtrl->controller_cfg |= DMA_MASTER_EN;

    softc->rxDma->cfg |= DMA_ENABLE;

    softc->linkCheck = 0;

#if !defined(_BCM960333_)
    bcm_ethsw_open();
#endif
#if defined(_BCM94908_) && defined(CFG_2P5G_LAN)
    parse_board_params();
    if (found_2p5g_lan) {
        cb_phy_init();
        crossbar_select();
    }
#endif /* _BCM94908_ && CFG_2P5G_LAN */
    return 0;
}

static int bcm63xx_ether_read( cfe_devctx_t * ctx, iocb_buffer_t * buffer )
{
    unsigned char * dstptr;
    unsigned char * srcptr;
    volatile DmaDesc * CurrentBdPtr;
    bcmenet_softc * softc = (bcmenet_softc *) ctx->dev_softc;
    uint16 dmaFlag;

    if( ctx == NULL ) {
        xprintf( "No context\n" );
        return -1;
    }

    if( buffer == NULL ) {
        xprintf( "No dst buffer\n" );
        return -1;
    }

    if( softc == NULL ) {
        xprintf( "softc not initialized\n" );
        return -1;
    }

    dmaFlag = (uint16) softc->rxBdReadPtr->status;
    if (!(dmaFlag & DMA_EOP))
    {
        xprintf("dmaFlag (return -1)[%04x]\n", dmaFlag);
        return -1;
    }

    dstptr       = buffer->buf_ptr;
    CurrentBdPtr = softc->rxBdReadPtr;

    srcptr = (unsigned char *)( PHYS_TO_K1(CurrentBdPtr->address) );

    buffer->buf_retlen = ((CurrentBdPtr->length < buffer->buf_length) ? CurrentBdPtr->length : buffer->buf_length);

    memcpy( dstptr, srcptr, buffer->buf_retlen );

    CurrentBdPtr->length = ENET_BUF_SIZE;
    CurrentBdPtr->status &= DMA_WRAP;
    CurrentBdPtr->status |= DMA_OWN;

    IncRxBdPtr( CurrentBdPtr, softc );
    softc->rxBdReadPtr = CurrentBdPtr;
    softc->dmaCtrl->flowctl_ch1_alloc = 1;

    // enable rx dma
    softc->rxDma->cfg = DMA_ENABLE;
    return 0;
}


static int bcm63xx_ether_inpstat( cfe_devctx_t * ctx, iocb_inpstat_t * inpstat )
{
    bcmenet_softc    * softc;
    volatile DmaDesc * CurrentBdPtr;

    /* ============================= ASSERTIONS ============================= */

    if( ctx == NULL ) {
        xprintf( "No context\n" );
        return -1;
    }

    if( inpstat == NULL ) {
        xprintf( "No inpstat buffer\n" );
        return -1;
    }

    softc = (bcmenet_softc *)ctx->dev_softc;
    if( softc == NULL ) {
        xprintf( "softc not initialized\n" );
        return -1;
    }

    /* ====================================================================== */

    CurrentBdPtr = softc->rxBdReadPtr;

    // inp_status == 1 -> data available
    inpstat->inp_status = (CurrentBdPtr->status & DMA_OWN) ? 0 : 1;

#if !defined(_BCM94908_)
    if (!inpstat->inp_status || (++softc->linkCheck > 100)) {
        // Don't check link state too often when receiving data
        softc->linkCheck = 0;
        robosw_check_ports();
    }
#endif

    return 0;
}


static int bcm63xx_ether_write(cfe_devctx_t *ctx,iocb_buffer_t *buffer)
{
    uint32_t status;
    unsigned char * dstptr;
    bcmenet_softc * softc;
    volatile DmaDesc * CurrentBdPtr;
    volatile uint32 txEvents = 0;

    /* ============================= ASSERTIONS ============================= */

    if( ctx == NULL ) {
        xprintf( "No context\n" );
        return -1;
    }

    if( buffer == NULL ) {
        xprintf( "No dst buffer\n" );
        return -1;
    }

    if( buffer->buf_length > ENET_MAX_MTU_SIZE ) {
        xprintf( "src buffer too large\n" );
        xprintf( "size is %d\n", buffer->buf_length );
        return -1;
    }

    softc = (bcmenet_softc *) ctx->dev_softc;
    if( softc == NULL ) {
        xprintf( "softc has not been initialized.\n" );
        return -1;
    }

    /* ====================================================================== */

    CurrentBdPtr = softc->txNextBdPtr;

    /* Find out if the next BD is available. */
    if( CurrentBdPtr->status & DMA_OWN ) {
        xprintf( "No tx BD available ?!\n" );
        return -1;
    }

    dstptr = (unsigned char *)PHYS_TO_K1( CurrentBdPtr->address );
    memcpy( dstptr, buffer->buf_ptr, buffer->buf_length );

    /* Set status of DMA BD to be transmitted. */
    status = DMA_SOP | DMA_EOP | DMA_APPEND_CRC | DMA_OWN;

    if( CurrentBdPtr == softc->txLastBdPtr ) {
        status |= DMA_WRAP;
    }

    CurrentBdPtr->length = ((buffer->buf_length < ETH_ZLEN) ? ETH_ZLEN : buffer->buf_length);
    CurrentBdPtr->status = status;

#if defined(_BCM94908_)
    __asm__ __volatile__ ("dsb    sy");
#endif    

    // Enable DMA for this channel
    softc->txDma->cfg |= DMA_ENABLE;

    // poll the dma status until done
    do
    {
        txEvents = CurrentBdPtr->status; 
    } while (txEvents & DMA_OWN);


    //Advance BD pointer to next in the chain.
    InctxBdPtr( CurrentBdPtr, softc );
    softc->txNextBdPtr = CurrentBdPtr;

    return 0;
}

static int bcm63xx_ether_ioctl(cfe_devctx_t *ctx, iocb_buffer_t *buffer)
{
    bcmenet_softc *softc = (bcmenet_softc *) ctx->dev_softc;

    switch ((int)buffer->buf_ioctlcmd) {
    case IOCTL_ETHER_GETHWADDR:
        memcpy(buffer->buf_ptr, softc->hwaddr, sizeof(softc->hwaddr));
        return 0;

    case IOCTL_ETHER_SETHWADDR:
        memcpy(softc->hwaddr, buffer->buf_ptr, sizeof(softc->hwaddr));
        return 0;

    default:
        return -1;
    }

    return 0;
}

/*
 * bcm63xx_ether_flush: Flushes packets from the DMA receive ring
 */
static int bcm63xx_ether_flush(bcmenet_softc * softc)
{
    volatile DmaDesc * CurrentBdPtr;
    uint16 dmaFlag;
    uint32 rxBdsCount = 0;
    int i;

    if( softc == NULL ) {
        xprintf( "softc not initialized\n" );
        return -1;
    }

    xprintf("Disabling Switch ports.\n");

#if !defined(_BCM960333_) && !defined(_BCM94908_)
    /* disable forwarding */
    SWITCH->SwitchMode &= ~SwitchMode_FwdgEn;

    /* Bring the link down on all switch ports */
    for(i=0; i<8; ++i) {
        SWITCH->PortOverride[i] &= ~PortOverride_Linkup;
        /* disable Rx and Tx */
        SWITCH->PortCtrl[i] = PortCtrl_DisableTx | PortCtrl_DisableRx;
    }
#endif

    for(i=0; i<1000; ++i) {
        cfe_usleep(1000);
    }

    xprintf("Flushing Receive Buffers...\n");

    while(!((dmaFlag = (uint16) softc->rxBdReadPtr->status) & DMA_OWN)) {

        if (!(dmaFlag & DMA_EOP))
        {
            xprintf("dmaFlag (return -1)[%04x]\n", dmaFlag);
            return -1;
        }

        CurrentBdPtr = softc->rxBdReadPtr;

        ++rxBdsCount;

        CurrentBdPtr->length = ENET_BUF_SIZE;
        CurrentBdPtr->status &= DMA_WRAP;
        CurrentBdPtr->status |= DMA_OWN;

        IncRxBdPtr( CurrentBdPtr, softc );
        softc->rxBdReadPtr = CurrentBdPtr;
        softc->dmaCtrl->flowctl_ch1_alloc = 1;

        // enable rx dma
        softc->rxDma->cfg = DMA_ENABLE;
    }

    xprintf("%d buffers found\n", rxBdsCount);

    return 0;
}

static int bcm63xx_ether_close(cfe_devctx_t *ctx)
{
    int i;
    bcmenet_softc * softc = (bcmenet_softc *) ctx->dev_softc;
    unsigned long sts;

#if !defined(_BCM960333_)
    bcm_ethsw_close();
#endif

    /* flush Rx DMA Channel */
    bcm63xx_ether_flush(softc);

    xprintf("Closing DMA Channels\n");

    sts = softc->rxDma->intStat;
    softc->rxDma->intStat = sts;
    softc->rxDma->intMask = 0;
    softc->rxDma->cfg = 0;
    // wait the current packet to complete before turning off EMAC, otherwise memory corruption can occur.
    for(i=0; softc->rxDma->cfg & DMA_ENABLE; i++) {
        // put the line below inside - it seems the signal can be lost and DMA never stops
        softc->rxDma->cfg = 0;
        if (i >= 20) {
            xprintf("Rx Timeout !!!\n");
            break;
        }
        cfe_usleep(100);
    }

    sts = softc->txDma->intStat;
    softc->txDma->intStat = sts;
    softc->txDma->intMask = 0;
    softc->txDma->cfg = 0;
    for(i=0; softc->txDma->cfg & DMA_ENABLE; i++) {
        // put the line below inside - it seems the signal can be lost and DMA never stops
        softc->txDma->cfg = 0;
        if (i >= 20) {
            xprintf("Tx Timeout !!!\n");
            break;
        }
        cfe_usleep(100);
    }

    /* return buffer allocation register internal count to 0 */
    softc->dmaCtrl->flowctl_ch1_alloc = (DMA_BUF_ALLOC_FORCE | 0);

    softc->dmaCtrl->controller_cfg &= ~DMA_MASTER_EN;

    KFREE( (void *)(softc->txBuffers) );
    KFREE( (void *)(softc->rxBuffers) );
    KFREE( (void *)(softc->txBds) );
    KFREE( (void *)(softc->rxBds) );
    return 0;
}
