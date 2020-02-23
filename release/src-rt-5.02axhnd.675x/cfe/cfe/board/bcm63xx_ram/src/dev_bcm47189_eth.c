/*
<:copyright-BRCM:2016:DUAL/GPL:standard

   Copyright (c) 2016 Broadcom 
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

#include "lib_types.h"
#include "lib_malloc.h"
#include "lib_byteorder.h"
#include "lib_string.h"
#include "lib_printf.h"

#include "cfe_iocb.h"
#include "cfe_device.h"
#include "cfe_ioctl.h"
#include "cfe_console.h"
#include "cfe_timer.h"
#include "cfe_error.h"

#include "bcmtypes.h"
#include "bcm_gpio.h"

#include "addrspace.h"
#include "dev_bcm63xx_eth.h"

#define _EXT_SWITCH_INIT_
#include "bcm_map.h"
#include "mii_shared.h"

#include "bcm_misc_hw_init.h"
#include "bcm_bbsi.h"

/*
 * NOTE:
 * This file right now is one big monolith containing all the required Ethernet
 * subsystem drivers for the 47189: GMAC, DMA, GPHY, switch driver and CFE
 * driver structure and callbacks.
 *
 * The purpose of this is to avoid cluttering the CFE and/or shared/opensource
 * directories with multiple files for each driver layer that maybe won't be
 * used or shared by any other target.
 *
 * Ideally, in the future this should be adapted to use the shared files created
 * for the Linux 47189 Eth driver (impl7) and all the GMAC, GPHY and switch
 * management code shouldn't be necessary anymore.
 */



/*****************************************/
/********** Function Prototypes **********/
/*****************************************/

/* CFE Ethernet routines and callbacks */
static int internal_open(bcmenet_softc *softc);
static void bcm47189_ether_probe(cfe_driver_t *drv, unsigned long probe_a,
                                 unsigned long probe_b, void *probe_ptr);
static int bcm47189_ether_open(cfe_devctx_t *ctx);
static int bcm47189_ether_read(cfe_devctx_t *ctx, iocb_buffer_t *buffer);
static int bcm47189_ether_inpstat(cfe_devctx_t *ctx, iocb_inpstat_t *inpstat);
static int bcm47189_ether_write(cfe_devctx_t *ctx, iocb_buffer_t *buffer);
static int bcm47189_ether_ioctl(cfe_devctx_t *ctx, iocb_buffer_t *buffer);
static int bcm47189_ether_close(cfe_devctx_t *ctx);


/* GMAC functions */
static void ethercore_enable(volatile Aidmp *wrap);
static void ethercore_disable(volatile Aidmp *wrap);
static void gmac_init(int dmaPort, unsigned char hwaddr[]);
static void init_dma(bcmenet_softc *softc);
static uint32_t gmac_mdio_read(int ethcore, uint8_t phyaddress, uint8_t reg);
static void gmac_mdio_write(int ethcore, uint8_t phyaddress,
                            uint8_t reg, uint16_t data);
static void ethercore_clock_init(int ethcore);
static void gmac_enable(int ethcore);
static void unimac_init_reset(volatile EnetCoreUnimac *unimac_regs);
static void unimac_clear_reset(volatile EnetCoreUnimac *unimac_regs);
static void unimac_flowcontrol(volatile EnetCoreUnimac *unimac_regs,
                               bool tx_flowctrl, bool rx_flowctrl);
static void unimac_promisc(volatile EnetCoreUnimac *unimac_regs, int mode);

/* Switch functions */
static void bcm_ethsw_init(int ethcore, ETHERNET_MAC_INFO EnetInfo);

/* GPHY functions */
static void gphy_init(int ethcore, ETHERNET_MAC_INFO EnetInfo);
static void update_gphy_link(int ethcore, ETHERNET_MAC_INFO EnetInfo);
static void eth_autoneg_waiting(int ethcore, uint8_t phyaddress, int start_wait);
static void eth_configure_phy(int ethcore, uint8_t phyaddress);
static void eth_gigabit_enable(int ethcore, uint8_t phyaddress);
static void gphy_reset(int ethcore, uint8_t phyaddress);

/* MoCA eth function */
static void MoCA_eth_init(void);

/* Other local functions */
static int ether_gphy_reset(bcmenet_softc* softc);


/* External functions */

extern void _cfe_flushcache(int, uint8_t *, uint8_t *);


/**************************************/
/********** Global variables **********/
/**************************************/

const static cfe_devdisp_t bcm47189_ether_dispatch = {
    bcm47189_ether_open,
    bcm47189_ether_read,
    bcm47189_ether_inpstat,
    bcm47189_ether_write,
    bcm47189_ether_ioctl,
    bcm47189_ether_close,
    NULL,
    NULL
};

const cfe_driver_t bcm47189_enet = {
    "BCM47189 Ethernet",
    "eth",
    CFE_DEV_NETWORK,
    &bcm47189_ether_dispatch,
    bcm47189_ether_probe
};

const ETHERNET_MAC_INFO* EnetInfo;


struct rx_dma_param
{
   int      rxHead;
   int      freeTail;
};
struct rx_dma_param rxDmaParam[2];

struct bbsi_t bbsi;


/************************ CFE Ethernet driver ***********************/

/****************************/
/********** Macros **********/
/****************************/

#define CACHE_ALIGN             16
#define DMA_RING_ALIGN          0x2000
#define DMA_DESC_ADDR_MASK      0xffff
#define DMA_RING_BOUNDARY       (1 << 16)

#define INVAL_RANGE(s,l) _cfe_flushcache(CFE_CACHE_INVAL_RANGE,((uint8_t *) (s)),((uint8_t *) (s))+(l))
#define FLUSH_RANGE(s,l) _cfe_flushcache(CFE_CACHE_FLUSH_RANGE,((uint8_t *) (s)),((uint8_t *) (s))+(l))


/******************************************/
/********** Function Definitions **********/
/******************************************/

static void bcm47189_ether_probe(cfe_driver_t *drv, unsigned long probe_a,
                                 unsigned long probe_b, void *probe_ptr)
{
    bcmenet_softc *softc;
    char descr[128];

    if ((EnetInfo = BpGetEthernetMacInfoArrayPtr()) == NULL) {
        xprintf("BoardID not Set in BoardParams\n");
        return;
    }

    softc = (bcmenet_softc *) KMALLOC( sizeof(bcmenet_softc), CACHE_ALIGN);
    if (softc == NULL) {
        xprintf("Failed alloc softc mem\n");
    } else {
        memset(softc, 0, sizeof(bcmenet_softc));

        if ((EnetInfo[0].sw.port_map & 1))
            softc->dmaPort = 0;
        else
            softc->dmaPort = 1;

        if (internal_open(softc) == -1)
            xprintf("Failed init enet hw\n");
        else
        {
            xsprintf(descr, "%s eth %d", drv->drv_description, softc->dmaPort);
            xprintf("%s\n", descr);
            cfe_attach(drv, softc, NULL, descr);
        }
    }
}


static int bcm47189_ether_inpstat(cfe_devctx_t *ctx, iocb_inpstat_t *inpstat)
{
    bcmenet_softc* softc;
    volatile EnetCoreMisc *misc_regs = 0;

    softc = (bcmenet_softc *)ctx->dev_softc;

    if (softc->dmaPort == 0) {
        misc_regs = ENET_CORE0_MISC;
    } else if (softc->dmaPort == 1) {
        misc_regs = ENET_CORE1_MISC;
    } else {
        xprintf("bcm47189_ether_inpstat -- ERROR: Enetcore %d not available\n",
                softc->dmaPort);
        return 0;
    }

    /*
     * Check for a regular packet reception.
     * Packet received ==> inpstat->inp_status = 1
     */
    if (misc_regs->intstatus & I_RI) {
        uint32_t bd_offset = ((rxDmaParam[softc->dmaPort].rxHead * sizeof(DmaDesc)) + 
                                   (unsigned int)(softc->rxFirstBdPtr)) & DMA_CURRENT_DESCR;
        uint32_t status0 = softc->rxDma->status0 & DMA_CURRENT_DESCR;

        if (status0 == bd_offset) {
            /* no more data to read, clear interrupt status */
            inpstat->inp_status = 0;
            misc_regs->intstatus = DEF_INTMASK;
        } else {
            inpstat->inp_status = 1;
        }
    } else {
        inpstat->inp_status = 0;
    }

    if (!inpstat->inp_status || (++softc->linkCheck > 100)) {
        // Don't check link state too often when receiving data
        softc->linkCheck = 0;
        if (EnetInfo[0].sw.phyconn[softc->dmaPort] == PHY_CONN_TYPE_EXT_SW) {
            //robosw_check_ports();
        }
        else if (EnetInfo[0].sw.phyconn[softc->dmaPort] == PHY_CONN_TYPE_EXT_PHY ||
                 EnetInfo[0].sw.phyconn[softc->dmaPort] == PHY_CONN_TYPE_MOCA_ETH) {
            update_gphy_link(softc->dmaPort, EnetInfo[0]);
        }
    }

    return 0;
}


static int bcm47189_ether_write(cfe_devctx_t *ctx, iocb_buffer_t *buffer)
{
    bcmenet_softc * softc;
    uint8_t * dstptr;
    volatile DmaDesc * CurrentBdPtr;
    uint8_t *next_ptr;

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

    CurrentBdPtr = (DmaDesc*)PHYS_TO_K1((softc->txDma->addrlow & 0xffff0000)
                                        | softc->txDma->ptr);

    /* next_ptr = CurrentBdPtr + 1 mod ring size */
    if ((uint8_t*)CurrentBdPtr == (uint8_t*)(softc->txLastBdPtr))
        next_ptr = (uint8_t*)(softc->txBds);
    else
        next_ptr = (uint8_t*)CurrentBdPtr + sizeof(DmaDesc);

    /*
     * Check if there are TX BDs available. The BD ring is full if
     * next_ptr == status0.
     */
    if (((uint32_t)next_ptr & 0xFFFF) == softc->txDma->status0) {
        xprintf("No tx BD available\n");
        return -1;
    }

    /*
     * Format the BD:
     * - Set the SOF and EOF flags
     * - Set the buffer length
     */
    CurrentBdPtr->ctrl1 |= (DMA_CTRL1_SOF | DMA_CTRL1_EOF);
    CurrentBdPtr->ctrl2 = buffer->buf_length;

    dstptr = (uint8_t*)PHYS_TO_K1(CurrentBdPtr->addrlow);
    memcpy(dstptr, buffer->buf_ptr, buffer->buf_length);

    /* Post the BD (advance ptr to the next BD) */
    softc->txDma->ptr = K1_TO_PHYS((uint32_t)next_ptr);
    /* Enable DMA TX */
    softc->txDma->control |= DMA_EN;

    return CFE_OK;
}


static int bcm47189_ether_open(cfe_devctx_t *ctx)
{
    return 0;
}


static int bcm47189_ether_read(cfe_devctx_t *ctx, iocb_buffer_t *buffer)
{
    bcmenet_softc *softc = (bcmenet_softc *) ctx->dev_softc;
    uint8_t * dstptr;
    uint8_t *srcptr;
    volatile DmaDesc *CurrentBdPtr;

    CurrentBdPtr = (DmaDesc *)K1_TO_PHYS((uint32_t)(softc->rxFirstBdPtr + rxDmaParam[softc->dmaPort].rxHead));
    rxDmaParam[softc->dmaPort].rxHead = (rxDmaParam[softc->dmaPort].rxHead + 1) % NR_RX_BDS;

    srcptr = (unsigned char *)PHYS_TO_K1(CurrentBdPtr->addrlow);

    /*
     * The first 4 bytes of the buffer are a status header. The first 16 bits of
     * this header contain the number of bytes in the received frame.
     */
    buffer->buf_retlen = (srcptr[0] | (srcptr[1] << 8));

    /* Advance srcptr to the start of the received data */
    srcptr += 4;

    dstptr = buffer->buf_ptr;
    memcpy(dstptr, srcptr, buffer->buf_retlen);


    /* Move free BD to next one */
    rxDmaParam[softc->dmaPort].freeTail = (rxDmaParam[softc->dmaPort].freeTail + 1) % NR_RX_BDS;
    if (rxDmaParam[softc->dmaPort].freeTail == 0)
        softc->rxDma->ptr = K1_TO_PHYS((uint32_t)softc->rxFirstBdPtr);
    else
        softc->rxDma->ptr += sizeof(DmaDesc);
   
    return 0;
}


static int bcm47189_ether_ioctl(cfe_devctx_t *ctx, iocb_buffer_t *buffer)
{
    bcmenet_softc *softc = (bcmenet_softc *) ctx->dev_softc;

    switch ((int)buffer->buf_ioctlcmd) {
    case IOCTL_ETHER_GETHWADDR:
        memcpy(buffer->buf_ptr, softc->hwaddr, sizeof(softc->hwaddr));
        return 0;

    case IOCTL_ETHER_SETHWADDR:
        memcpy(softc->hwaddr, buffer->buf_ptr, sizeof(softc->hwaddr));
        gmac_init(softc->dmaPort, softc->hwaddr);
        return 0;

    default:
        return -1;
    }

    return 0;
}


static int bcm47189_ether_close(cfe_devctx_t *ctx)
{
    //bcmenet_softc * softc = (bcmenet_softc *) ctx->dev_softc;

    /* disable further interrupts from gmac */
    ENET_CORE0_MISC->intmask = 0;

    /* clear the interrupt conditions */
    ENET_CORE0_MISC->intstatus = 0;

    /* reset the chip */
    //ethercore_reset(softc);

    return 0;
}


/*
 * Ethernet hardware setup:
 *
 * - Ethernet core initialization
 * - MAC configuration
 * - DMA initialization
 * - GPHY/Switch initialization
 */
static int internal_open(bcmenet_softc *softc)
{
    int i;
    void *p;
    volatile Aidmp * wrap = 0;

    if (softc->dmaPort == 0) {
        wrap = ENET_CORE0_WRAP;
        softc->dmaCtrl = ENET_CORE0_DMA;
    } else if (softc->dmaPort == 1) {
        /* need to enable core0 first */
        ethercore_enable(ENET_CORE0_WRAP);
        wrap = ENET_CORE1_WRAP;
        softc->dmaCtrl = ENET_CORE1_DMA;
    } else {
        xprintf("Ethernet attach failed: Eth core %d doesn't exist\n",
                softc->dmaPort);
        return -1;
    }

    /*
     * Enable Ethernet core. From this point on we have access to the GMAC
     * core registers.
     */
    ethercore_enable(wrap);

    /**** DMA data structure allocation and setup ****/

    /*
     * DMA TR and RX channel registers:
     * Use TX channel 0 and RX channel 0 for now.
     */
    softc->rxDma = &softc->dmaCtrl->dmarcv;
    softc->txDma = &softc->dmaCtrl->dmaxmt;

    /* TX descriptors allocation */
    p = KMALLOC(NR_TX_BDS * sizeof(DmaDesc), DMA_RING_ALIGN);
    if (p == NULL) {
        xprintf( "BCM47189 : Fail alloctxBds mem\n" );
        return -1;
    }
    INVAL_RANGE(p, NR_TX_BDS * sizeof(DmaDesc));
    softc->txBds = (DmaDesc*)K0_TO_K1((uintptr_t) p);

    /* RX descriptors allocation */
    p = KMALLOC(NR_RX_BDS * sizeof(DmaDesc), DMA_RING_ALIGN);
    if(p== NULL) {
        xprintf("BCM47189 : Fail alloc rxBds mem\n");
        KFREE((void *)(softc->txBds));
        softc->txBds = NULL;
        return -1;
    }
    INVAL_RANGE(p, NR_RX_BDS * sizeof(DmaDesc));
    softc->rxBds = (DmaDesc *)K0_TO_K1((uintptr_t) p);

    /* DMA RX buffers allocation */
    softc->rxBuffers = (uint8_t*)KMALLOC(NR_RX_BDS * ENET_BUF_SIZE, CACHE_ALIGN);
    if(softc->rxBuffers == NULL) {
        xprintf("BCM47189 : Failed allocRxBuffer mem\n");
        KFREE((void *)(softc->txBds));
        softc->txBds = NULL;
        KFREE((void *)(softc->rxBds));
        softc->rxBds = NULL;
        return -1;
    }
    INVAL_RANGE(softc->rxBuffers, NR_RX_BDS * ENET_BUF_SIZE);

    /* DMA TX buffers allocation */
    softc->txBuffers = (uint8_t*)KMALLOC(NR_TX_BDS * ENET_BUF_SIZE, CACHE_ALIGN);
    if( softc->txBuffers == NULL ) {
        xprintf("BCM47189 : Failed alloc txBuffer mem\n");
        KFREE((void *)(softc->rxBuffers));
        softc->rxBuffers = NULL;
        KFREE((void *)(softc->txBds));
        softc->txBds = NULL;
        KFREE((void *)(softc->rxBds));
        softc->rxBds = NULL;
        return -1;
    }
    INVAL_RANGE(softc->txBuffers, NR_TX_BDS * ENET_BUF_SIZE);

    /* RX descriptor ring initialization */
    softc->rxFirstBdPtr = softc->rxBds;
    softc->rxLastBdPtr = softc->rxBds + NR_RX_BDS - 1;

    for(i = 0; i < NR_RX_BDS; i++) {
        (softc->rxBds + i)->ctrl1 = DMA_CTRL1_SOF;
        (softc->rxBds + i)->ctrl2 = ENET_BUF_SIZE;
        (softc->rxBds + i)->addrhigh = 0;
        (softc->rxBds + i)->addrlow = K0_TO_PHYS((uintptr_t)softc->rxBuffers
                                                 + i * ENET_BUF_SIZE);
    }
    softc->rxLastBdPtr->ctrl1 |= DMA_CTRL1_EOT;

    /* TX descriptor ring initialization */
    softc->txFirstBdPtr = softc->txBds;
    softc->txLastBdPtr = softc->txBds + NR_TX_BDS - 1;

    for(i = 0; i < NR_TX_BDS; i++) {
        (softc->txBds + i)->ctrl1  = 0;
        (softc->txBds + i)->ctrl2  = ENET_BUF_SIZE;
        (softc->txBds + i)->addrhigh  = 0;
        (softc->txBds + i)->addrlow = K0_TO_PHYS((uintptr_t)softc->txBuffers
                                                 + i * ENET_BUF_SIZE);
    }
    softc->txLastBdPtr->ctrl1 |= DMA_CTRL1_EOT;

    /* Init DMA registers */
    init_dma(softc);

    /* Reset external GPHY/Switch if required for this interface*/
    if (EnetInfo[0].ucPhyType == BP_ENET_NO_PHY) {
        /*
         * Fake switch definition. Each switch port defines a GMAC link and the
         * bp_usPhyConnType specifies what it's linked to (GPHY, ext switch,
         * PLC SoC)
         */
        /* Check only GMAC0 for now */
        if (EnetInfo[0].sw.phyconn[0] == PHY_CONN_TYPE_EXT_SW
            || EnetInfo[0].sw.phyconn[softc->dmaPort] == PHY_CONN_TYPE_EXT_PHY ||
            (EnetInfo[1].ucPhyType == BP_ENET_EXTERNAL_SWITCH && 
             (EnetInfo[0].sw.phy_id[softc->dmaPort] & EXTSW_CONNECTED))) {
            if (ether_gphy_reset(softc) == -1) {
                xprintf("Ethernet attach failed\n");
                /* put the core back into reset */
                ethercore_disable(wrap);
                return -1;
            }
        }
    }

    /* Initialize GMAC */
    gmac_init(softc->dmaPort, softc->hwaddr);

    /* initialize external phy */
    if (EnetInfo[0].sw.phyconn[softc->dmaPort] == PHY_CONN_TYPE_EXT_SW) {
        bcm_ethsw_init(softc->dmaPort, EnetInfo[0]);
    } else if (EnetInfo[0].sw.phyconn[softc->dmaPort] == PHY_CONN_TYPE_EXT_PHY) {
        gphy_init(softc->dmaPort, EnetInfo[0]);
    } else if (EnetInfo[0].sw.phyconn[softc->dmaPort] == PHY_CONN_TYPE_MOCA_ETH) {
        MoCA_eth_init();
    } else if (EnetInfo[1].ucPhyType == BP_ENET_EXTERNAL_SWITCH && 
              (EnetInfo[0].sw.phy_id[softc->dmaPort] & EXTSW_CONNECTED)) {
        bcm_ethsw_init(softc->dmaPort, EnetInfo[1]);
    }

    softc->linkCheck = 0;

    return 0;
}


/*
 * This does two things:
 * 1) Sets the chip GPIOs to floating state
 * 2) Performs a physical GPHY/Switch reset by setting the appropriate GPIO
 *    low, then high again.
 *
 * NOTE: This is too specific and doesn't take the GPIO polarity into account.
 */
static int ether_gphy_reset(bcmenet_softc* softc)
{
    uint16_t reset;

    softc->chipId = MISC->chipid & CID_ID_MASK;
    softc->chipRev = (MISC->chipid & CID_REV_MASK) >> CID_REV_SHIFT;

    /* set gpio pad to floating state */
    GPIO->gpiopullup = 0;
    GPIO->gpiopulldown = 0;

    /* reset the external phy */
    if (BpGetPhyResetGpio(0, softc->dmaPort,/* 0,*/ &reset) != BP_SUCCESS)
    {
        xprintf("Phy reset gpio not found\n");
        /* put the core back into reset */
        if (softc->dmaPort == 0) {
            ethercore_disable(ENET_CORE0_WRAP);
        } else if (softc->dmaPort == 1) {
            ethercore_disable(ENET_CORE1_WRAP);
        }
        return -1;
    }

    /* keep RESET low for 2 us */
    bcm_gpio_set_data(reset, 0);
    bcm_gpio_set_dir(reset, GPIO_OUT);
    cfe_usleep(2);

    /* keep RESET high for at least 2 us */
    bcm_gpio_set_data(reset, 1);
    cfe_usleep(2);

    return 0;
}



/**************************** GMAC driver ***************************/

/****************************/
/********** Macros **********/
/****************************/

#define GMAC_RESET_DELAY        2
#define MDIO_POLL_PERIOD        10

/*
 * Spin at most 'us' microseconds while 'exp' is true.
 * Caller should explicitly test 'exp' when this completes
 * and take appropriate error action if 'exp' is still true.
 */
#define SPINWAIT_POLL_PERIOD    10

#define SPINWAIT(exp, us) { \
        uint32 countdown = (us) + (SPINWAIT_POLL_PERIOD - 1); \
        while ((exp) && (countdown >= SPINWAIT_POLL_PERIOD)) { \
                cfe_usleep(SPINWAIT_POLL_PERIOD); \
                countdown -= SPINWAIT_POLL_PERIOD; \
        } \
}

/* The maximum packet length */
#define	ETHER_MAX_LEN           1518

/* PMU clock/power control */
#define PMUCTL_ENAB             (MISC->capabilities & CC_CAP_PMU)

/* 53537 series moved switch_type and gmac_if_type to CC4 [15:14] and [13:12] */
#define PMU_CC4_IF_TYPE_MASK            0x00003000
#define PMU_CC4_IF_TYPE_RMII            0x00000000
#define PMU_CC4_IF_TYPE_MII             0x00001000
#define PMU_CC4_IF_TYPE_RGMII           0x00002000

#define PMU_CC4_SW_TYPE_MASK            0x0000c000
#define PMU_CC4_SW_TYPE_EPHY            0x00000000
#define PMU_CC4_SW_TYPE_EPHYMII         0x00004000
#define PMU_CC4_SW_TYPE_EPHYRMII        0x00008000

/* PMU chip control4 register */
#define PMU_CHIPCTL4                    4
#define PMU_CC4_SW_TYPE_RGMII           0x0000c000


/******************************************/
/********** Function Definitions **********/
/******************************************/

static inline volatile EnetCoreMisc* gmac_misc_regs(int ethcore)
{
    if (ethcore == 0) {
        return ENET_CORE0_MISC;
    } else if (ethcore == 1) {
        return ENET_CORE1_MISC;
    } else {
        printf("Fatal error: Ethernet core %d doesn't exist\n", ethcore);
        /* Loop here forever */
        while (1) ;
    }

    /* Never reached */
    return 0;
}

static inline volatile EnetCoreUnimac* gmac_unimac_regs(int ethcore)
{
    if (ethcore == 0) {
        return ENET_CORE0_UNIMAC;
    } else if (ethcore == 1) {
        return ENET_CORE1_UNIMAC;
    } else {
        printf("Fatal error: Ethernet core %d doesn't exist\n", ethcore);
        /* Loop here forever */
        while (1) ;
    }

    /* Never reached */
    return 0;
}


static uint32_t gmac_mdio_read(int ethcore, uint8_t phyaddress, uint8_t reg)
{
    uint32_t reg_read;
    int countdown = 100;
    volatile EnetCoreMisc *misc_regs = gmac_misc_regs(ethcore);

    /* Issue the read command*/
    misc_regs->phycontrol = (misc_regs->phycontrol & ~PC_EPA_MASK) | phyaddress;
    misc_regs->phyaccess = PA_START | (phyaddress << PA_ADDR_SHIFT) |
                           (reg << PA_REG_SHIFT);

    /* Wait for it to complete */
    while ((misc_regs->phyaccess & PA_START) && (countdown-- > 0))
        cfe_usleep(MDIO_POLL_PERIOD);

    reg_read = misc_regs->phyaccess;
    if (reg_read & PA_START) {
        printf("Error: mii_read_ext did not complete\n");
        reg_read = 0xffffffff;
    }

    return reg_read & PA_DATA_MASK;
}

static void gmac_mdio_write(int ethcore, uint8_t phyaddress,
                            uint8_t reg, uint16_t data)
{
    uint16 countdown = 100;
    volatile EnetCoreMisc *misc_regs = gmac_misc_regs(ethcore);

    misc_regs->phycontrol = (misc_regs->phycontrol & ~PC_EPA_MASK)
                                  | phyaddress;

    /* Clear mdioint bit of intstatus */
    misc_regs->intstatus = I_MDIO;
    if ((misc_regs->intstatus & I_MDIO) == 0) {
        /* Issue the write command */
        misc_regs->phyaccess = PA_START | PA_WRITE
                               | (phyaddress << PA_ADDR_SHIFT)
                               | (reg << PA_REG_SHIFT) | data;

        /* Wait for it to complete */
        while ((misc_regs->phyaccess & PA_START) && (countdown-- > 0))
            cfe_usleep(MDIO_POLL_PERIOD);
    }
}

/*
 * Ethernet core/dma registers are not mapped into the address space
 * until the Eth core is reset.
 *
 * By default, only the ChipCommon registers are mapped (0x18000000). In WCC
 * the SoC cores, their identifiers and addresses are read programmatically from
 * the chip enumeration ROM.
 *
 * Core reset is done by writing the appropriate registers in the core
 * wrapper.
 *
 * Instead of populating a core list at runtime by walking the eROM, I defined
 * the necessary info from the available cores statically in the 47189 register
 * map so we can access the wrapper registers the same way we access any regular
 * register.
 */
static void ethercore_enable(volatile Aidmp *wrap)
{
    int loop_counter = 10;

    /* Put core into reset state */
    wrap->resetctrl = AIRC_RESET;
    cfe_usleep(1000);

    /* Ensure there are no pending backplane operations */
    SPINWAIT(wrap->resetstatus, 300);

    wrap->ioctrl = SICF_FGC | SICF_CLOCK_EN;

    /* Ensure there are no pending backplane operations */
    SPINWAIT(wrap->resetstatus, 300);

    while (wrap->resetctrl != 0 && --loop_counter) {
        SPINWAIT(wrap->resetstatus, 300);
        /* Take core out of reset */
        wrap->resetctrl = 0;
        SPINWAIT(wrap->resetstatus, 300);
    }

    wrap->ioctrl = SICF_CLOCK_EN;
    cfe_usleep(1000);
}


void ethercore_disable(volatile Aidmp *wrap)
{
    /* if core is already in reset, just return */
    if (wrap->resetctrl & AIRC_RESET)
      return;

    /* ensure there are no pending backplane operations */
    SPINWAIT(wrap->resetstatus, 300);

    /* if pending backplane ops still, try waiting longer */
    if (wrap->resetstatus) {
        /* 300usecs was sufficient to allow backplane ops to clear for big hammer */
        /* during driver load we may need more time */
        SPINWAIT(wrap->resetstatus, 10000);
        /* if still pending ops, continue on and try disable anyway */
        /* this is in big hammer path, so don't call wl_reinit in this case... */
    }

    /* Put core into reset state */
    wrap->resetctrl = AIRC_RESET;
    cfe_usleep(1);

    wrap->ioctrl = 0;
    cfe_usleep(10);
}

/*
 * Configures the Ethernet core clock (from PMU)
 */
static void ethercore_clock_init(int ethcore)
{
    volatile EnetCoreMisc *misc_regs = gmac_misc_regs(ethcore);

    /* set gmac into loopback mode to ensure no rx traffic */
    //gmac_macloopback(TRUE);
    //cfe_usleep(1);

    /* ethernet clock is generated by the PMU */
    misc_regs->clk_ctl_st |= CS_ER;
    SPINWAIT((misc_regs->clk_ctl_st & CS_ES) != CS_ES, 1000);

    /* configure gmac and switch data for PMU */
    if (PMUCTL_ENAB) {
        PMU->chipcontrol_addr = PMU_CHIPCTL4;
        PMU->chipcontrol_data &= ~(PMU_CC4_IF_TYPE_MASK | PMU_CC4_SW_TYPE_MASK);
        PMU->chipcontrol_data |= PMU_CC4_IF_TYPE_RGMII | PMU_CC4_SW_TYPE_RGMII;
    }

    /* set phy control: set smi_master to drive mdc_clk */
    misc_regs->phycontrol |= PC_MTE;

    /* Read the devstatus to figure out the configuration mode of
     * the interface. Set the speed to 100 if the switch interface
     * is mii/rmii. We know that we have rgmii, just maintained for
     * completeness.
     */
    /* NOT REALLY NECESSARY, REMOVE */
    //gmac_miiconfig();
}


static void gmac_enable(int ethcore)
{
    uint32 cmdcfg, rxqctl, bp_clk, mdp, mode;
    volatile EnetCoreMisc *misc_regs = gmac_misc_regs(ethcore);
    volatile EnetCoreUnimac *unimac_regs = gmac_unimac_regs(ethcore);

    cmdcfg = unimac_regs->cmdcfg;

    /* put mac in reset */
    unimac_init_reset(unimac_regs);

    /* initialize default config */
    cmdcfg = unimac_regs->cmdcfg;

    cmdcfg &= ~(CC_TE | CC_RE | CC_RPI | CC_TAI | CC_HD | CC_ML |
                CC_CFE | CC_RL | CC_RED | CC_PE | CC_TPI | CC_PAD_EN | CC_PF);
    cmdcfg |= (CC_PROM | CC_NLC | CC_CFE | CC_TPI | CC_AT);

    unimac_regs->cmdcfg = cmdcfg;

    /* bring mac out of reset */
    unimac_clear_reset(unimac_regs);

    /* Other default configurations */
    /* Enable RX and TX flow control */
    unimac_flowcontrol(unimac_regs, 1, 1);
    /* Disable promiscuous mode */
    unimac_promisc(unimac_regs, 0);

    /* Enable the mac transmit and receive paths now */
    cfe_usleep(2);
    cmdcfg &= ~CC_SR;
    cmdcfg |= (CC_RE | CC_TE);

    /* assert rx_ena and tx_ena when out of reset to enable the mac */
    unimac_regs->cmdcfg = cmdcfg;

    /* not force ht when gmac is in rev mii mode (we have rgmii mode) */
    mode = ((misc_regs->devstatus & DS_MM_MASK) >> DS_MM_SHIFT);
    if (mode != 0)
        /* request ht clock */
        misc_regs->clk_ctl_st |= CS_FH;

    /* init the mac data period. the value is set according to expr
     * ((128ns / bp_clk) - 3). */
    rxqctl = misc_regs->rxqctl;
    rxqctl &= ~RC_MDP_MASK;

    bp_clk = pmu_clk(PMU_PLL_CTRL_M3DIV_SHIFT) / 1000000;
    mdp = ((bp_clk * 128) / 1000) - 3;
    misc_regs->rxqctl = rxqctl | (mdp << RC_MDP_SHIFT);
}


/*
 * GMAC initialization
 */
static void gmac_init(int dmaPort, unsigned char hwaddr[])
{
    volatile EnetCoreMisc *misc_regs = gmac_misc_regs(dmaPort);
    volatile EnetCoreUnimac *unimac_regs = gmac_unimac_regs(dmaPort);

    /* Configure the Ethernet Core clock */
    ethercore_clock_init(dmaPort);

    /* enable one rx interrupt per received frame */
    misc_regs->intrecvlazy = 1 << IRL_FC_SHIFT;

    /* Set the MAC address */
    unimac_regs->macaddrhigh = htonl(*(uint32 *)&hwaddr[0]);
    unimac_regs->macaddrlow = htons(*(uint32 *)&hwaddr[4]);

    /* set max frame lengths - account for possible vlan tag */
    unimac_regs->rxmaxlength = ETHER_MAX_LEN + 32;

    /* Enable and clear interrupts */
    misc_regs->intmask = DEF_INTMASK;
    misc_regs->intstatus = DEF_INTMASK;

    /* Turn ON the GMAC */
    gmac_enable(dmaPort);
}


/*
 * init_dma: Initialize DMA control register
 */
static void init_dma(bcmenet_softc *softc)
{
    /*
     * The WCC driver keeps certain DMA configuration parameters as flags
     * (bitmasks) in an attribute called ctrlflags. Some of these parameters
     * are:
     * - DMA_CTRL_PEN (Parity Enable)
     * - DMA_CTRL_ROC (RX Overflow Enable)
     * - DMA_CTRK_RXMULTI (Allow rx scatter to multiple descriptors)
     * - DMA_CTRL_UNFRAMED (Unframed Rx/Tx data
     * and some others.
     *
     * At this point the WCC driver does a lot of things that only affect the
     * saved logic state of the DMAs, but none of them affect the real
     * hardware. It also allocates the dma rings, buffers and some other things
     * (DMA mapping vectors), but CPE does allocations elsewhere (everything is
     * allocated before entering this function).
     *
     * I'm almost sure we won't need any of that, so for now this only does
     * DMA hardware initialization.
     */

    /* Disable the DMAs */
    softc->rxDma->control &= ~DMA_EN;
    softc->txDma->control &= ~DMA_EN;

    /*
     * CPE code configures the DMA controllers here. In WCC this is scattered in
     * a number of functions. Collate them here.
     */

    /* TX init */
    /*
     * 1 - Keep the default values for burstlen, prefetch_ctrl,
     *     prefetch_trhreshold, and multiple_outstanding_reads.
     * 2 - Configure the TX BD ring base address.
     * 2 - Enable TX channel.
     */
    softc->txDma->addrhigh = 0;
    softc->txDma->addrlow = (uint32)K1_TO_PHYS((uintptr_t)(softc->txFirstBdPtr));

    /* Disable parity check */
    softc->txDma->control |= DMA_PTY_CHK_DISABLE;
    softc->txDma->control |= DMA_EN;

    /* Start with an empty descriptor table (ptr = addrlow) */
    softc->txDma->ptr = softc->txDma->addrlow;


    /* RX init */
    /*
     * 1 - Keep the default values for burstlen, wait_for_completion, parity
     *     check, overflow_continue, prefetch ctl and threshold, and address
     *     extension bits.
     * 2 - Configure the RX BD ring base address
     * 2 - Enable TX channel
     */
    softc->rxDma->addrhigh = 0;
    softc->rxDma->addrlow = (uint32)K1_TO_PHYS((uintptr_t)(softc->rxFirstBdPtr));

    /* Enable overflow continue and disable parity check */
    softc->rxDma->control |= (DMA_OVERFLOW_CONTINUE | DMA_PTY_CHK_DISABLE);
    softc->rxDma->control |= DMA_EN;

    /* set free BD tail */
    softc->rxDma->ptr = (uint32)K1_TO_PHYS((uintptr_t)(&softc->rxBds[NR_RX_BDS-1]));

    rxDmaParam[softc->dmaPort].rxHead = 0;
    rxDmaParam[softc->dmaPort].freeTail = NR_RX_BDS - 1;
}


/********** UniMAC control routines **********/

static void unimac_init_reset(volatile EnetCoreUnimac *unimac_regs)
{
    /* put mac in software reset */
    unimac_regs->cmdcfg |= CC_SR;
    cfe_usleep(GMAC_RESET_DELAY);
}

static void unimac_clear_reset(volatile EnetCoreUnimac *unimac_regs)
{
    /* bring mac out of software reset */
    unimac_regs->cmdcfg &= ~CC_SR;
    cfe_usleep(GMAC_RESET_DELAY);
}

static void unimac_flowcontrol(volatile EnetCoreUnimac *unimac_regs,
                               bool tx_flowctrl, bool rx_flowctrl)
{
    uint32 cmdcfg;

    cmdcfg = unimac_regs->cmdcfg;

    /* put the mac in reset */
    unimac_init_reset(unimac_regs);

    /* to enable tx flow control clear the rx pause ignore bit */
    if (tx_flowctrl)
        cmdcfg &= ~CC_RPI;
    else
        cmdcfg |= CC_RPI;

    /* to enable rx flow control clear the tx pause transmit ignore bit */
    if (rx_flowctrl)
        cmdcfg &= ~CC_TPI;
    else
        cmdcfg |= CC_TPI;

    unimac_regs->cmdcfg = cmdcfg;

    /* bring mac out of reset */
    unimac_clear_reset(unimac_regs);
}

static void unimac_promisc(volatile EnetCoreUnimac *unimac_regs, int mode)
{
    uint32 cmdcfg;

    cmdcfg = unimac_regs->cmdcfg;

    /* put the mac in reset */
    unimac_init_reset(unimac_regs);

    /* enable or disable promiscuous mode */
    if (mode)
        cmdcfg |= CC_PROM;
    else
        cmdcfg &= ~CC_PROM;

    unimac_regs->cmdcfg = cmdcfg;

    /* bring mac out of reset */
    unimac_clear_reset(unimac_regs);
}



/**************************** GPHY driver ***************************/

/****************************/
/********** Macros **********/
/****************************/

/* Time-out for MDIO communication. */
#define MDIO_TIMEOUT                    1000
/* Time-out for autonegotiation. */
#define AUTONEG_TIMEOUT                 100  /* in ms. Multiple of 2! */
/* Autonegotiation timeout period. */
#define AUTONEG_TIMEOUT_PERIOD          50   /* in ms */


/* PHY management registers addresses */
#define MDIO_ADDRESS_CONTROL            0
#define MDIO_ADDRESS_STATUS             1
#define MDIO_ADDRESS_PHY_IDENTIFIER     2
#define MDIO_ADDRESS_PHY_IDENTIFIER_3   3
#define MDIO_ADDRESS_AUTONEG_ADV        4
#define MDIO_ADDRESS_LINK_PARTNER       5
#define MDIO_ADDRESS_1000BASET_CTRL     9

/* Masks for PHY management registers */
#define MDIO_DATA_RESET                         0x8000
/* MDIO_DATA_SPEEDSL: 11=Rsv, 10=1000Mbps, 01=100Mbps, 00=10Mbps */
#define MDIO_DATA_SPEEDSL_LSB                   0x2000
#define MDIO_DATA_SPEEDSL_MSB                   0x0040
#define MDIO_DATA_AUTONEG_ENABLED               0x1000
#define MDIO_DATA_DUPLEX                        0x0100
#define MDIO_DATA_AUTONEG_COMPLETE              0x0020
#define MDIO_DATA_AUTONEG_ABILITY               0x0008
#define MDIO_DATA_GIGA_MASK                     0xF000
#define MDIO_DATA_GIGA_NEGOTIATED               0x0010
#define MDIO_DATA_GIGA_NEGOTIATED_MASK          0x0018
#define MDIO_DATA_LOOPBACK_MASK                 0x4000
#define MDIO_DATA_LINK_STATUS_MASK              0x0004
#define MDIO_LINK_PARTNER_FLOW_CONTROL_MASK     0x0400

#define MDIO_AUTONEG_ADV_ASYMPAUSE_MASK         0x0800
#define MDIO_AUTONEG_ADV_PAUSABLE_MASK          0x0400
#define MDIO_AUTONEG_ADV_100BaseT4_MASK         0x0200
#define MDIO_AUTONEG_ADV_100BaseTX_FD_MASK      0x0100
#define MDIO_AUTONEG_ADV_100BaseTX_HD_MASK      0x0080
#define MDIO_AUTONEG_ADV_10BaseT_FD_MASK        0x0040
#define MDIO_AUTONEG_ADV_10BaseT_HD_MASK        0x0020

#define MDIO_1000BASET_MANUAL_MASTER_SLAVE_MASK 0x1000
#define MDIO_1000BASET_MASTER_SLAVE_CFG_MASK    0x0800
#define MDIO_1000BASET_REPEATER_ADVERTISE_MASK  0x0400
#define MDIO_1000BASET_FDX_ADVERTISE_MASK       0x0200
#define MDIO_1000BASET_HDX_ADVERTISE_MASK       0x0100
#define RESTART_AUTONEG_MASK                    0x0200
#define ENABLE_AUTONEG_MASK                     0x1000

#define CORE_SHD18_ADDRESS              0x18

#define CORE_SHD18_OP_MASK              0x8000
#define CORE_SHD18_OP_ALIGN             0
#define CORE_SHD18_OP_BITS              1
#define CORE_SHD18_OP_SHIFT             15

#define CORE_SHD18_SEL_MASK             0x7000
#define CORE_SHD18_SEL_ALIGN            0
#define CORE_SHD18_SEL_BITS             3
#define CORE_SHD18_SEL_SHIFT            12

#define CORE_SHD18_WRITE_MASK           0xFFF8
#define CORE_SHD18_WRITE_ALIGN          0
#define CORE_SHD18_WRITE_BITS           13
#define CORE_SHD18_WRITE_SHIFT          3

#define CORE_SHD18_SHWD_SEL_MASK        0x0007
#define CORE_SHD18_SHWD_SEL_ALIGN       0
#define CORE_SHD18_SHWD_SEL_BITS        3
#define CORE_SHD18_SHWD_SEL_SHIFT       0

/* defines related to CORE_SHD18 registers*/
#define CORE_SHD18_CTRL_REG                     0
#define CORE_SHD18_BASE_T_REG                   1
#define CORE_SHD18_POWER_MII_CONTROL_REG        2
#define CORE_SHD18_MISC_TEST_REG                4
#define CORE_SHD18_MISC_CTRL_REG                7

#define CORE_SHD18_MISC_CTRL_FORCE_AUTO_MDIX_MASK       0x0200
#define CORE_SHD18_MISC_CTRL_FORCE_AUTO_MDIX_ALIGN      0
#define CORE_SHD18_MISC_CTRL_FORCE_AUTO_MDIX_BITS       1
#define CORE_SHD18_MISC_CTRL_FORCE_AUTO_MDIX_SHIFT      9

/* defines related to CORE_BASE19 registers */
#define GPHY_MII_AUXSTAT   0x19
#define CORE_BASE19_AUTONEG_HCD_MASK 0x0700
#define CORE_BASE19_AUTONEG_HCD_SHIFT 8
#define CORE_BASE19_AUTONEG_HCD_BITS  3
#define CORE_BASE19_AUTONEG_HCD_ALIGN 0


/* Masks for PHY management registers */
#define MDIO_DATA_RESET         0x8000

#define BCMNET_DUPLEX_HALF      0
#define BCMNET_DUPLEX_FULL      1


/*******************************************/
/********** Data type definitions **********/
/*******************************************/

/* Auto negotiated highest common denominator for Internal GPHY */
typedef enum {
    AUTONEG_HCD_1000BaseT_FD = 7,
    AUTONEG_HCD_1000BaseT_HD = 6,
    AUTONEG_HCD_100BaseTX_FD = 5,
    AUTONEG_HCD_100BaseT4    = 4,
    AUTONEG_HCD_100BaseTX_HD = 3,
    AUTONEG_HCD_10BaseT_FD   = 2,
    AUTONEG_HCD_10BaseT_HD   = 1,
    AUTONEG_HCD_INVALID      = 0
} t_AUTONEG_HCD_LinkSpeed;

typedef enum {
    SPEED_DOWN     = 0,
    SPEED_10MBIT   = 10000000,
    SPEED_100MBIT  = 100000000,
    SPEED_1000MBIT = 1000000000
} eEthernetSpeed;

struct link_status {
    int link_speed;
    int last_link_speed;
    int duplex;
};

struct link_status gphy_status[2];

/******************************************/
/********** Function Definitions **********/
/******************************************/

static void eth_get_link_speed(int ethcore, uint8_t phyaddress,
                               struct link_status *link_status)
{
    int phy_status_reg = 0;
    int phy_aux_status_reg = 0;
    t_AUTONEG_HCD_LinkSpeed current_link_speed = 0;

    /* Read status word */
    phy_status_reg = gmac_mdio_read(ethcore, phyaddress, MDIO_ADDRESS_STATUS);

    /* get link status */
    if (((phy_status_reg & MDIO_DATA_LINK_STATUS_MASK) == 0)
                        || (phy_status_reg == 0xFFFF)) {
        link_status->link_speed = SPEED_DOWN;
    }
    else {
        /* Link up, calculate current link speed & duplex mode */

        /* Read MII_AUXSTAT which has AUTONEG_HCD */
        phy_aux_status_reg = gmac_mdio_read(ethcore, phyaddress, GPHY_MII_AUXSTAT);
        current_link_speed = (phy_aux_status_reg & CORE_BASE19_AUTONEG_HCD_MASK)
                                >> CORE_BASE19_AUTONEG_HCD_SHIFT;

        /* eth link speed for Unimac */
        switch (current_link_speed) {
        case AUTONEG_HCD_1000BaseT_FD:
        case AUTONEG_HCD_1000BaseT_HD:
            link_status->link_speed = SPEED_1000MBIT;
            break;
        case AUTONEG_HCD_100BaseTX_FD:
        case AUTONEG_HCD_100BaseT4:
        case AUTONEG_HCD_100BaseTX_HD:
            link_status->link_speed = SPEED_100MBIT;
            break;
        case AUTONEG_HCD_10BaseT_FD:
        case AUTONEG_HCD_10BaseT_HD:
            link_status->link_speed = SPEED_10MBIT;
            break;
        default :
            /* default for GPHY */
            link_status->link_speed = SPEED_1000MBIT;
            break;
        }

        /* Set Unimac Half Duplex mode*/
        switch (current_link_speed) {
        case AUTONEG_HCD_1000BaseT_FD:
        case AUTONEG_HCD_100BaseTX_FD:
        case AUTONEG_HCD_10BaseT_FD:
            link_status->duplex = BCMNET_DUPLEX_FULL;
            break;
        case AUTONEG_HCD_1000BaseT_HD:
        case AUTONEG_HCD_100BaseT4:   /** \TODO - Confirm if it is Half Duplex */
        case AUTONEG_HCD_100BaseTX_HD:
        case AUTONEG_HCD_10BaseT_HD:
            link_status->duplex = BCMNET_DUPLEX_HALF;
            break;
        default :
            link_status->duplex = BCMNET_DUPLEX_FULL;
            break;
        }
    }
}


static void gphy_link_check(int ethcore, ETHERNET_MAC_INFO EnetInfo,
                            struct link_status *link_status)
{
    /* The GPHY address is encoded in the port id (bp_ulPhyIdX) */
    uint8_t phyaddr = EnetInfo.sw.phy_id[ethcore] & BCM_PHY_ID_M;

    /* Wait until auto-negotiation is finished */
    eth_autoneg_waiting(ethcore, phyaddr, 0);

    /* Get current speed (possibly auto-negotiated) */
    eth_get_link_speed(ethcore, phyaddr, link_status);
}


static void update_gphy_link(int ethcore, ETHERNET_MAC_INFO EnetInfo)
{
    gphy_link_check(ethcore, EnetInfo, &gphy_status[ethcore]);
    if (gphy_status[ethcore].last_link_speed != gphy_status[ethcore].link_speed) {
        gphy_status[ethcore].last_link_speed = gphy_status[ethcore].link_speed;
        /* reconfigure MAC */
        if (gphy_status[ethcore].link_speed) {
            uint32 cmdcfg;
            volatile EnetCoreUnimac *unimac_regs = gmac_unimac_regs(ethcore);

            cmdcfg = unimac_regs->cmdcfg;

            /* put mac in reset */
            unimac_init_reset(unimac_regs);
            /* clear speed and duplex */
            cmdcfg &= ~(CC_ES_MASK | CC_HD);
            /* set speed */
            if (gphy_status[ethcore].link_speed == SPEED_1000MBIT)
                cmdcfg |= (0x2 << CC_ES_SHIFT);
            else if (gphy_status[ethcore].link_speed == SPEED_100MBIT)
                cmdcfg |= (0x1 << CC_ES_SHIFT);
            /* set duplex */
            if (gphy_status[ethcore].duplex == BCMNET_DUPLEX_HALF)
                cmdcfg |= CC_HD;

            unimac_regs->cmdcfg = cmdcfg;

            /* bring mac out of reset */
            unimac_clear_reset(unimac_regs);
            printf("eth%d Link UP %d%s\n", ethcore, 
                gphy_status[ethcore].link_speed/1000000,
                gphy_status[ethcore].duplex ? "FD":"HD");
        }
        else 
            printf("eth%d Link DOWN\n", ethcore);
    }
}

static void gphy_reset(int ethcore, uint8_t phyaddress)
{
    /*
     * [GMAC0 MDIO write] MDIO_ADDRESS_CONTROL <- MDIO_DATA_RESET
     * using PHY addr = phyaddress
     */
    gmac_mdio_write(ethcore, phyaddress, MDIO_ADDRESS_CONTROL, MDIO_DATA_RESET);

    /* Upon exiting standby Standby Power-down mode, the EGPHY remains in an
     * internal reset state for 40 us, and then resumes normal operation */
    cfe_usleep(40);
}

static void eth_gigabit_enable(int ethcore, uint8_t phyaddress)
{
    uint32_t val;

    /* Start advertising 1000BASE-T */
    val = gmac_mdio_read(ethcore, phyaddress, MDIO_ADDRESS_1000BASET_CTRL);
    val |= (MDIO_1000BASET_REPEATER_ADVERTISE_MASK |
            MDIO_1000BASET_FDX_ADVERTISE_MASK |
            MDIO_1000BASET_HDX_ADVERTISE_MASK);
    gmac_mdio_write(ethcore, phyaddress, MDIO_ADDRESS_1000BASET_CTRL, val);
    /* Restart autonegotiation */
    val = gmac_mdio_read(ethcore, phyaddress, MDIO_ADDRESS_CONTROL);
    val |= RESTART_AUTONEG_MASK;
    gmac_mdio_write(ethcore, phyaddress, MDIO_ADDRESS_CONTROL, val);
}

static void eth_configure_phy(int ethcore, uint8_t phyaddress)
{
    uint32_t val;

    /* Setting RGMII RXD to RXC Skew mode,shadow register */
    gmac_mdio_write(ethcore, phyaddress, 0x18, 0xf0e7);
    /* Adding more delay, set clock delay(GTXCLK) enable */
    gmac_mdio_write(ethcore, phyaddress, 0x1c, 0x8c00);

    /* Auto-negotiation / Speed selection */
    val = gmac_mdio_read(ethcore, phyaddress, MDIO_ADDRESS_CONTROL);
    /* Unspecified link configuration or 1000BASE-T:
     * Enable auto-negotiation */
    val |= ENABLE_AUTONEG_MASK;

    gmac_mdio_write(ethcore, phyaddress, MDIO_ADDRESS_CONTROL, val);

    /* Auto-negotiation configuration */
    val = gmac_mdio_read(ethcore, phyaddress, MDIO_ADDRESS_AUTONEG_ADV);
    /* 1. Enable advertisement of asymmetric pause.
     * 2. By default only full duplex modes are being advertised.
     *    Enable Half duplex as well full duplex modes for all speeds.
     *    For 1 GBPS only full duplex is to be supported.
     */
    val |= (MDIO_AUTONEG_ADV_100BaseT4_MASK
            | MDIO_AUTONEG_ADV_100BaseTX_FD_MASK
            | MDIO_AUTONEG_ADV_100BaseTX_HD_MASK
            | MDIO_AUTONEG_ADV_10BaseT_FD_MASK
            | MDIO_AUTONEG_ADV_10BaseT_HD_MASK);

    gmac_mdio_write(ethcore, phyaddress, MDIO_ADDRESS_AUTONEG_ADV, val);

    /* Start advertising 1000BASE-T */
    eth_gigabit_enable(ethcore, phyaddress);

    /* Wait for autoneg to start after giving restart autoneg
     * as part of eth_gigabit_enable/disable
     */
    //eth_autoneg_waiting(1);

    /* Force Auto MDI-X */
    val = CORE_SHD18_MISC_CTRL_REG << CORE_SHD18_SEL_SHIFT;
    val |= (CORE_SHD18_MISC_CTRL_REG << CORE_SHD18_SHWD_SEL_SHIFT);
    val &= ~(CORE_SHD18_OP_MASK);   /* 0: read */
    gmac_mdio_write(ethcore, phyaddress, CORE_SHD18_ADDRESS, val);
    val = gmac_mdio_read(ethcore, phyaddress, CORE_SHD18_ADDRESS);

    val = (val & CORE_SHD18_WRITE_MASK)
          | (1 << CORE_SHD18_MISC_CTRL_FORCE_AUTO_MDIX_SHIFT);
    val |= (CORE_SHD18_MISC_CTRL_REG << CORE_SHD18_SEL_SHIFT);
    val |= (CORE_SHD18_MISC_CTRL_REG << CORE_SHD18_SHWD_SEL_SHIFT);
    val |= CORE_SHD18_OP_MASK;      /* 1: write */
    gmac_mdio_write(ethcore, phyaddress, CORE_SHD18_ADDRESS, val);
}


static void gphy_init(int ethcore, ETHERNET_MAC_INFO EnetInfo)
{
    uint8_t phyaddr = EnetInfo.sw.phy_id[ethcore] & BCM_PHY_ID_M;

    /* The GPHY address is encoded in the port id (bp_ulPhyIdX) */
    gphy_reset(ethcore, phyaddr);
    eth_configure_phy(ethcore, phyaddr);
}


/*
 * eth_autoneg_waiting
 *
 * DESCRIPTION:
 * Waits until autonegotiation is started or completed
 *
 * PARAMETERS:
 * start_wait: If 1, wait until the negotiation get started.
 *             If 0, wait until the negotiation is complete.
 */
static void eth_autoneg_waiting(int ethcore, uint8_t phyaddress, int start_wait)
{
    int val;
    int i;
    int timeout;

    /* if start_wait=TRUE, we wait for the autoneg to get started */
    if (start_wait) {
        /* Autoneg starting timeout is half the autoneg finishing timeout */
        timeout = (AUTONEG_TIMEOUT) >> 1;

        /* Wait until auto-negotiation is started */
        val = gmac_mdio_read(ethcore, phyaddress, MDIO_ADDRESS_CONTROL);

        if (val & MDIO_DATA_AUTONEG_ENABLED) {
            i = 0;
            while (i < timeout) {
                val = gmac_mdio_read(ethcore, phyaddress, MDIO_ADDRESS_STATUS);
                if (!(val & MDIO_DATA_AUTONEG_COMPLETE) ||
                    !(val & MDIO_DATA_AUTONEG_ABILITY)) {
                    break;
                }
                cfe_usleep(AUTONEG_TIMEOUT_PERIOD * 1000);
                i += AUTONEG_TIMEOUT_PERIOD;
            }
        }
    }
    /* Otherwise, wait for the autoneg to get finished */
    else {
        timeout = AUTONEG_TIMEOUT;

        /* Wait until auto-negotiation is finished */
        val = gmac_mdio_read(ethcore, phyaddress, MDIO_ADDRESS_CONTROL);

        if (val & MDIO_DATA_AUTONEG_ENABLED) {
            i = 0;
            while (i < timeout) {
                val = gmac_mdio_read(ethcore, phyaddress, MDIO_ADDRESS_STATUS);
                if ((val & MDIO_DATA_AUTONEG_COMPLETE) ||
                    !(val & MDIO_DATA_AUTONEG_ABILITY)) {
                    break;
                }
                cfe_usleep(AUTONEG_TIMEOUT_PERIOD * 1000);
                i += AUTONEG_TIMEOUT_PERIOD;
            }
            if (i >= timeout) {
                //printf("%s: Auto-negotiation failed\n", __FUNCTION__);
            }
        }
    }
}

/*************************** MoCA eth driver **************************/
/****************************/
/********** Macros **********/
/****************************/
#define SUN_TOP_CTRL_CHIP_FAMILY_ID		0x10404000
#define SUN_TOP_CTRL_PRODUCT_ID			0x10404004


static void MoCA_reset(uint32_t gpio)
{
    /* using GPIO to reset moca device */
    bcm_gpio_set_dir(gpio, GPIO_OUT);

    /* Keep RESET high for 50ms */
    bcm_gpio_set_data(gpio, 1);
    cfe_usleep(50000);

    /* Keep RESET low for 300ms */
    bcm_gpio_set_data(gpio, 0);
    cfe_usleep(300000);

    /* Keep RESET high for 300ms */
    bcm_gpio_set_data(gpio, 1);
    cfe_usleep(300000);
}

static void
MoCA_init_gphy(struct bbsi_t *bbsi)
{
	/* write 1's except to bit 26 (gphy sw_init) */
	bbsi_write(bbsi, 0x1040431c, 4, 0xFBFFFFFF);
	/* clear bits 2 and 0 */
	bbsi_write(bbsi, 0x10800004, 4, 0x02a4c000);

	/* DELAY 10MS */
	cfe_usleep(10000);

	bbsi_write(bbsi,  0x1040431c, 4, 0xFFFFFFFF);
	/* take unimac out of reset */
	bbsi_write(bbsi, 0x10800000, 4, 0);

	/* Pin muxing for rgmii 0 and 1 */
	bbsi_write(bbsi, 0x10404100, 4, 0x11110000);
	bbsi_write(bbsi, 0x10404104, 4, 0x11111111);
	bbsi_write(bbsi, 0x10404108, 4, 0x11111111);
	bbsi_write(bbsi, 0x1040410c, 4, 0x00001111);
	/* Pin mux for MDIO */
	bbsi_write(bbsi, 0x10404118, 4, 0x00001100);

	bbsi_write(bbsi, 0x10800024, 4, 0x0000930d);
	/* enable rgmii 0 */
	bbsi_write(bbsi, 0x1080000c, 4, 0x00000011);
	/* enable rgmii 1 */
	bbsi_write(bbsi, 0x10800018, 4, 0x00000011);

	bbsi_write(bbsi, 0x10800808, 4, 0x010000d8);
	/* port mode for gphy and moca from rgmii */
	bbsi_write(bbsi, 0x10800000, 4, 2);

	/* tx and rx enable (0x3) */
	bbsi_write(bbsi, 0x10800808, 4, 0x0100000b);
	/* Link/ACT LED */
	bbsi_write(bbsi, 0x10800024, 4, 0x0000934d);

	/* Set 6802 mdio slave */
	bbsi_write(bbsi, 0x10800000, 4, 0xE);
	/* enable ID mode on RGMII 1 */
	bbsi_write(bbsi, 0x1080000c, 4, 0x0000001f);
	/* enable ID mode on RGMII 0 */
	bbsi_write(bbsi, 0x10800018, 4, 0x0000001f);
	/* Set rgmii to 2.5V CMOS */
	bbsi_write(bbsi, 0x104040a4, 4, 0x11);
}
 
static void MoCA_eth_init()
{
    uint32_t data = 0;
    uint16_t moca_reset;
    uint32_t fid, pid;


    if (BpGetSpiClkGpio(&bbsi.spi_clk) != BP_SUCCESS)
    {
        printf("Error, BBSI SPI Clock is not defined!\n");
        return;
    }
    else 
        bbsi.spi_clk &= BP_GPIO_NUM_MASK;

    if (BpGetSpiCsGpio(&bbsi.spi_cs) != BP_SUCCESS)
    {
        printf("Error, BBSI SPI Chip Select is not defined!\n");
        return;
    }
    else 
        bbsi.spi_cs &= BP_GPIO_NUM_MASK;

    if (BpGetSpiMisoGpio(&bbsi.spi_miso) != BP_SUCCESS)
    {
        printf("Error, BBSI SPI MISO is not defined!\n");
        return;
    }
    else 
        bbsi.spi_miso &= BP_GPIO_NUM_MASK;

    if (BpGetSpiMosiGpio(&bbsi.spi_mosi) != BP_SUCCESS)
    {
        printf("Error, BBSI SPI MOSI is not defined!\n");
        return;
    }
    else 
        bbsi.spi_mosi &= BP_GPIO_NUM_MASK;


    if (BpGetMoCAResetGpio(&moca_reset) != BP_SUCCESS)
    {
        printf("Error, MoCA reset gpio is not defined!\n");
        return;
    }
    else 
        moca_reset &= BP_GPIO_NUM_MASK;

    /* init bbsi bus */
    bbsi_init(&bbsi);

    MoCA_reset(moca_reset);

    if ((bbsi_read(&bbsi, SUN_TOP_CTRL_CHIP_FAMILY_ID, 4, &data) < 0) ||
            data == 0 || data == 0xffffffff) {
            printf("Failed to get SUN_TOP_CTRL_CHIP_FAMILY_ID: 0x%08x\n", data);
            return;
    } else {
            fid = data;
    }

    if ((bbsi_read(&bbsi, SUN_TOP_CTRL_PRODUCT_ID, 4, &data) < 0) ||
        data == 0 || data == 0xffffffff) {
        printf("Failed to get SUN_TOP_CTRL_PRODUCT_ID: 0x%08x\n", data);
        return;
    } else {
        pid = data;
    }

    printf("BBSI slave device SUN_TOP_CTRL_CHIP_FAMILY_ID: 0x%08x,"
        " SUN_TOP_CTRL_PRODUCT_ID: 0x%08x\n", fid, pid);

    /* init gphy */
    MoCA_init_gphy(&bbsi);

}


/*************************** Switch driver **************************/

/****************************/
/********** Macros **********/
/****************************/

#define BCM53125 0x53125

/******************************************/
/********** Function Definitions **********/
/******************************************/

/* External switch register access through MDC/MDIO */
static void ethsw_rreg_ext(int ethcore, int page, int reg, uint8_t *data, int len)
{
    uint32_t cmd, res, ret;
    int max_retry = 0;

    cmd = (page << REG_PPM_REG16_SWITCH_PAGE_NUMBER_SHIFT)
                        | REG_PPM_REG16_MDIO_ENABLE;
    gmac_mdio_write(ethcore, PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG16, cmd);

    cmd = (reg << REG_PPM_REG17_REG_NUMBER_SHIFT) | REG_PPM_REG17_OP_READ;
    gmac_mdio_write(ethcore, PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG17, cmd);

    do {
        res = gmac_mdio_read(ethcore, PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG17);
        cfe_usleep(10);
    } while (((res & (REG_PPM_REG17_OP_WRITE | REG_PPM_REG17_OP_READ))
                        != REG_PPM_REG17_OP_DONE)
                        && (max_retry++ < 5));

    ret = 0;
    ret |= gmac_mdio_read(ethcore, PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24) << 0;
    ret |= gmac_mdio_read(ethcore, PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG25) << 16;
    switch (len) {
    case 1:
        *data = (uint8_t)ret;
        break;
    case 2:
        *(uint16_t *)data = (uint16_t)ret;
        break;
    case 4:
        *(uint32_t *)data = ret;
        break;
    }
}

static void ethsw_wreg_ext(int ethcore, int page, int reg, uint8_t *data, int len)
{
    uint32_t cmd, res;
    uint32_t val = 0;
    int max_retry = 0;

    switch (len) {
    case 1:
        val = *data;
        break;
    case 2:
        val = *(uint16_t *)data;
        break;
    case 4:
        val = *(uint32_t *)data;
        break;
    }
    cmd = (page << REG_PPM_REG16_SWITCH_PAGE_NUMBER_SHIFT)
                        | REG_PPM_REG16_MDIO_ENABLE;
    gmac_mdio_write(ethcore, PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG16, cmd);

    cmd = val>>0 & 0xffff;
    gmac_mdio_write(ethcore, PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, cmd);
    cmd = val>>16 & 0xffff;
    gmac_mdio_write(ethcore, PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG25, cmd);

    cmd = (reg << REG_PPM_REG17_REG_NUMBER_SHIFT) | REG_PPM_REG17_OP_WRITE;
    gmac_mdio_write(ethcore, PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG17, cmd);

    do {
        res = gmac_mdio_read(ethcore, PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG17);
        cfe_usleep(10);
    } while (((res & (REG_PPM_REG17_OP_WRITE | REG_PPM_REG17_OP_READ))
                        != REG_PPM_REG17_OP_DONE)
                        && (max_retry++ < 5));
}


static void bcm_ethsw_init(int ethcore, ETHERNET_MAC_INFO EnetInfo)
{
    uint8_t data8;
    uint32_t data32;

    ethsw_rreg_ext(ethcore, PAGE_MANAGEMENT, REG_DEVICE_ID,
                   (uint8_t *)&data32, sizeof(data32));
    printf("External switch id = %x \n", (int)data32);

    if (data32 == BCM53125) {
        int i;

        /* setup Switch MII1 port state override */
        data8 = (REG_CONTROL_MPSO_MII_SW_OVERRIDE |
            REG_CONTROL_MPSO_SPEED1000 |
            REG_CONTROL_MPSO_FDX |
            REG_CONTROL_MPSO_LINKPASS |
            REG_CONTROL_MPSO_TX_FLOW_CONTROL |
            REG_CONTROL_MPSO_RX_FLOW_CONTROL
            );
        ethsw_wreg_ext(ethcore, PAGE_CONTROL, REG_CONTROL_MII1_PORT_STATE_OVERRIDE,
                                &data8, sizeof(data8));
        /* unmanaged mode, enable forwarding */
        ethsw_rreg_ext(ethcore, PAGE_CONTROL, REG_SWITCH_MODE, &data8,
                                sizeof(data8));
        data8 &= ~REG_SWITCH_MODE_FRAME_MANAGE_MODE;
        data8 |= REG_SWITCH_MODE_SW_FWDG_EN;
        ethsw_wreg_ext(ethcore, PAGE_CONTROL, REG_SWITCH_MODE, &data8,
                                sizeof(data8));
        /* Enable IMP Port */
        data8 = ENABLE_MII_PORT;
        ethsw_wreg_ext(ethcore, PAGE_MANAGEMENT, REG_GLOBAL_CONFIG, &data8,
                                sizeof(data8));
        /* Disable BRCM Tag for IMP */
        data8 = 0; //~REG_BRCM_HDR_ENABLE;
        ethsw_wreg_ext(ethcore, PAGE_MANAGEMENT, REG_BRCM_HDR_CTRL, &data8,
                                sizeof(data8));
        /* enable rx bcast, ucast and mcast of imp port */
        data8 = REG_MII_PORT_CONTROL_RX_UCST_EN
                                | REG_MII_PORT_CONTROL_RX_MCST_EN
                                | REG_MII_PORT_CONTROL_RX_BCST_EN;
        ethsw_wreg_ext(ethcore, PAGE_CONTROL, REG_MII_PORT_CONTROL, &data8,
                               sizeof(data8));

        /* Enable APD compatibility bit on all ports for the 53125 */
        for (i = 0; i < BP_MAX_SWITCH_PORTS; i++) {
            if ((EnetInfo.sw.port_map & (1 << i)) != 0) {
                gmac_mdio_write(ethcore, EnetInfo.sw.phy_id[i] & BCM_PHY_ID_M,
                                MII_REGISTER_1C, 0xa921);
                //extsw_phyport_wreg(ethcore, i, MII_REGISTER_1C, 0xa921);
            }
        }

        /* No spanning tree for unmanaged mode */
        data8 = 0;
        for (i = 0; i < BP_MAX_SWITCH_PORTS; i++) {
            if ((EnetInfo.sw.port_map & (1 << i)) != 0) {
                ethsw_wreg_ext(ethcore, PAGE_CONTROL, i, &data8, sizeof(data8));
            }
        }
        /* No spanning tree on IMP port either */
        ethsw_wreg_ext(ethcore, PAGE_CONTROL, REG_MII_PORT_CONTROL, &data8,
                                sizeof(data8));
    }
}
