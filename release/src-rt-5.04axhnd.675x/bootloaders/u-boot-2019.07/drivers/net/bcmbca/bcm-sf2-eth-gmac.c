/* SPDX-License-Identifier: GPL-2.0+
*  *
*   *  Copyright 2019 Broadcom Ltd.
*    */

#include <config.h>
#include <common.h>
#include <malloc.h>
#include <net.h>
#include <asm/io.h>
#include <phy.h>
#include <asm/system.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include "pmc_drv.h"

#include "asm/arch/gmac.h"
#include "bcm_ethsw.h"

/*---------------------------------------------------------------------*/
/* specify number of BDs and buffers to use                            */
/*---------------------------------------------------------------------*/
#define NR_TX_BDS               40
#define NR_RX_BDS               40
#define ENET_ZLEN               60
#define ENET_MAX_MTU_SIZE       1522    /* Body(1500) + EH_SIZE(14) + FCS(4) + VLAN(4) */
#define DMA_MAX_BURST_LENGTH    8       /* in 64 bit words */
#define ENET_BUF_SIZE           ((ENET_MAX_MTU_SIZE + 63) & ~63)
#define DMA_FC_THRESH_LO        5
#define DMA_FC_THRESH_HI        10
#define EMAC_TX_WATERMARK       32

// for now physical address = virtual
#define  K0_TO_PHYS

typedef struct sf2gmac_priv {

	volatile GmacIntf *gmac_intf;
	volatile GmacMac *gmac_mac;
	volatile DmaRegs *dmaCtrl;

	struct udevice *sf2_devp;
	bcm_ethsw_ops_t *sf2_ops;
	unsigned char *rcv_buffer;
	int init;
	
	/* transmit variables */    
	volatile DmaChannelCfg *txDma;      /* location of transmit DMA register set */
	volatile DmaDesc *txBds;            /* Memory location of tx Dma BD ring */
	volatile DmaDesc *txFirstBdPtr;     /* ptr to first allocated Tx BD */
	volatile DmaDesc *txNextBdPtr;      /* ptr to next Tx BD to transmit with */
	volatile DmaDesc *txLastBdPtr;      /* ptr to last allocated Tx BD */

	/* receive variables */
	volatile DmaChannelCfg *rxDma;      /* location of receive DMA register set */
	volatile DmaDesc *rxBds;            /* Memory location of rx bd ring */
	volatile DmaDesc *rxFirstBdPtr;     /* ptr to first allocated rx bd */
	volatile DmaDesc *rxBdReadPtr;      /* ptr to next rx bd to be processed */
	volatile DmaDesc *rxLastBdPtr;      /* ptr to last allocated rx bd */

	uint8_t* rxBuffers;
	uint8_t* txBuffers;

} sf2gmac_priv;

#define IncRxBdPtr(x, s) if (x == ((sf2gmac_priv *)s)->rxLastBdPtr) \
                             x = ((sf2gmac_priv *)s)->rxBds; \
                      else x++
#define InctxBdPtr(x, s) if (x == ((sf2gmac_priv *)s)->txLastBdPtr) \
                             x = ((sf2gmac_priv *)s)->txBds; \
                      else x++


#define DMA_RX_CHAN          0
#define DMA_TX_CHAN          1

#define CACHE_ALIGN     64


/*
 * init_dma: Initialize DMA control register
 */

#define NUM_PORTS	1 // only 1 port supported
static void init_dma(struct sf2gmac_priv *softc)
{
	uint32_t *StateRam;
	int i;

	/*
	 * clear State RAM
	 */
	StateRam = (uint32_t *)&softc->dmaCtrl->stram.s[0];
	for (i = 0; i < sizeof(DmaStateRam) / sizeof(uint32_t) * NUM_PORTS * 2; i++)
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
	softc->dmaCtrl->stram.s[DMA_TX_CHAN].baseDescPtr = (uint32_t)softc->txFirstBdPtr;

	// receive
	softc->rxDma->cfg = 0;  // initialize first (will enable later)
	softc->rxDma->maxBurst = DMA_MAX_BURST_LENGTH;
	softc->rxDma->intMask = 0;   /* mask all ints */
	/* clr any pending interrupts on channel */
	softc->rxDma->intStat = DMA_DONE|DMA_NO_DESC|DMA_BUFF_DONE;
	softc->rxDma->intMask = DMA_DONE;
	softc->dmaCtrl->stram.s[DMA_RX_CHAN].baseDescPtr = (uint32_t)softc->rxFirstBdPtr;
}


static void gmac_init(sf2gmac_priv *priv)
{
	volatile GmacIntf *GMAC_INTF = priv->gmac_intf;
	volatile GmacMac *GMAC_MAC = priv->gmac_mac;

	/* Reset GMAC */
	GMAC_MAC->Cmd.sw_reset = 1;
	udelay(20);
	GMAC_MAC->Cmd.sw_reset = 0;

	GMAC_INTF->Flush.txfifo_flush = 1;
	GMAC_INTF->Flush.rxfifo_flush = 1;

	GMAC_INTF->Flush.txfifo_flush = 0;
	GMAC_INTF->Flush.rxfifo_flush = 0;

	GMAC_INTF->MibCtrl.clrMib = 1;
	GMAC_INTF->MibCtrl.clrMib = 0;

	/* default CMD configuration */
	GMAC_MAC->Cmd.eth_speed = CMD_ETH_SPEED_1000;

	/* Disable Tx and Rx */
	GMAC_MAC->Cmd.tx_ena = 0;
	GMAC_MAC->Cmd.rx_ena = 0;

	return;
}

static void gmac_enable_port(sf2gmac_priv *priv, int enable)
{
	volatile GmacMac *GMAC_MAC = priv->gmac_mac;

	if( enable ) {
		GMAC_MAC->Cmd.tx_ena = 1;
		GMAC_MAC->Cmd.rx_ena = 1;
	}
	else {
		GMAC_MAC->Cmd.tx_ena = 0;
		GMAC_MAC->Cmd.rx_ena = 0;
	}

	return;
}

static int internal_open (struct sf2gmac_priv *softc)
{
	int i;
	unsigned char *p;

	softc->rxDma = &softc->dmaCtrl->chcfg[DMA_RX_CHAN];
	softc->txDma = &softc->dmaCtrl->chcfg[DMA_TX_CHAN];

	// If doing SW reboot in EPI the controller can still be active
	softc->rxDma->cfg = 0;
	softc->txDma->cfg = 0;
	softc->dmaCtrl->controller_cfg &= ~DMA_MASTER_EN;

	// noncached memory can only be allocated once
	if (softc->txBds == NULL)
	{
		// allocate a chunk of non cached memory for packet descriptors
		p = noncached_alloc((NR_TX_BDS + NR_RX_BDS) * sizeof(DmaDesc), CACHE_ALIGN);
		if( p == NULL ) {
			printf( "BCM63xx : Fail alloctxBds mem\n" );
			return -1;
		}

		softc->txBds = (DmaDesc *)p;
		p += (NR_TX_BDS * sizeof(DmaDesc));
		softc->rxBds = (DmaDesc *)p;
	}

	// allocate memory for RX and TX data (cached memory)
	softc->rxBuffers = (uint8_t*)memalign( CACHE_ALIGN, NR_RX_BDS * ENET_BUF_SIZE );
	if( softc->rxBuffers == NULL ) {
		printf( "BCM63xx : Failed allocRxBuffer mem\n" );
		return -1;
	}
	invalidate_dcache_range((unsigned long)softc->rxBuffers, (unsigned long)softc->rxBuffers + NR_RX_BDS * ENET_BUF_SIZE);

	softc->txBuffers = (uint8_t*)memalign( CACHE_ALIGN, NR_TX_BDS * ENET_BUF_SIZE );
	if( softc->txBuffers == NULL ) {
		printf( "BCM63xx : Failed alloc txBuffer mem\n" );
		free( (void *)(softc->rxBuffers) );
		softc->rxBuffers = NULL;
		return -1;
	}
	invalidate_dcache_range((unsigned long)softc->txBuffers, (unsigned long)softc->txBuffers + NR_TX_BDS * ENET_BUF_SIZE);

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

	softc->rcv_buffer = (unsigned char *)memalign(sizeof(int), ENET_BUF_SIZE);
	if (softc->rcv_buffer == NULL) {
		printf("failed to allocate receive buffer\n");
		return -1;
	}

	return 0;
}

static int bcmbca_sf2gmac_eth_start (struct udevice *dev)
{
	int rc = 0;
	struct sf2gmac_priv *priv = dev_get_priv(dev);

	debug("bcmbca_sf2gmac_eth_start called %s\n", dev->name);
	if (priv->init)
		return 0;

	rc = internal_open(priv);
	if (rc) {
		printf("internal open failed rx %d\n", rc);
		return rc;
	}

	if (priv->sf2_devp)
		priv->sf2_ops->open (priv->sf2_devp);

	gmac_init(priv);
	gmac_enable_port(priv, 1);

	priv->init = 1;
	return 0;
}

static void bcmbca_sf2gmac_eth_stop(struct udevice *dev)
{
	struct sf2gmac_priv *priv = dev_get_priv(dev);

	debug("bcmbca_sf2gmac_eth_stop called %s\n", dev->name);

	return;
}

static int bcmbca_sf2gmac_eth_read_mac(struct udevice *dev)
{
	return 0;
}

static int bcmbca_sf2gmac_eth_send(struct udevice *dev, void *buffer, int length)
{
	volatile DmaDesc * CurrentBdPtr;
	unsigned char * dstptr;
	struct sf2gmac_priv *softc = dev_get_priv(dev);
	uint32_t status, offset;
	int timeout = 1000;

	if (length > ENET_MAX_MTU_SIZE)
	{
		return -1;
	}

	CurrentBdPtr = softc->txNextBdPtr;

	/* Find out if the next BD is available. */
	if( CurrentBdPtr->status & DMA_OWN ) {
		printf( "No tx BD available ?!\n" );
		return -1;
	}

	dstptr = (unsigned char *)( CurrentBdPtr->address );
	memcpy( dstptr, buffer, length );
	flush_cache((unsigned long)dstptr, length);

	/* Set status of DMA BD to be transmitted. */
	status = DMA_SOP | DMA_EOP | DMA_APPEND_CRC | DMA_OWN;
	if( CurrentBdPtr == softc->txLastBdPtr ) {
		status |= DMA_WRAP;
	}

	CurrentBdPtr->length = length;
	CurrentBdPtr->status = status;
	
	barrier();
	softc->txDma->cfg |= DMA_ENABLE;

	// poll dma status until done
	do {
		udelay(10);
		invalidate_dcache_range((unsigned long)dstptr, (unsigned long)dstptr + CACHE_ALIGN);
	} while((CurrentBdPtr->status & DMA_OWN) && timeout--);

	if (timeout > 0) {
		//Advance BD pointer to next in the chain.
		InctxBdPtr( CurrentBdPtr, softc );
		softc->txNextBdPtr = CurrentBdPtr;
		return 0;
	}
	else {
		printf("timeout sending packet of size %d\n", length);
		// clear the buffer pointer and force the PD pointer 
    	softc->txDma->intStat = DMA_DONE|DMA_NO_DESC|DMA_BUFF_DONE;
		offset = softc->dmaCtrl->stram.s[DMA_TX_CHAN].state_data & RING_OFFSET_MASK;
		softc->txNextBdPtr = (volatile DmaDesc *)((unsigned char *)softc->txFirstBdPtr + (offset * sizeof(DmaDesc)));
		return -1;
	}
}

static int bcmbca_sf2gmac_eth_recv(struct udevice *dev, int flags, uchar ** packetp)
{
	int length, rc = 0;
	uint16_t dmaFlag;
	uint8_t *srcptr;
	volatile DmaDesc * CurrentBdPtr;
	struct sf2gmac_priv *softc = dev_get_priv(dev);

	dmaFlag = (uint16_t) softc->rxBdReadPtr->status;

	if (!(dmaFlag & DMA_EOP)) {
		return -1;
	}

	CurrentBdPtr = softc->rxBdReadPtr;
	srcptr = CurrentBdPtr->address;

	length = CurrentBdPtr->length;

	if (length == ENET_BUF_SIZE || length < ENET_ZLEN || length > ENET_MAX_MTU_SIZE)
	{
		printf("received data length is incorrect %d\n", length);
		rc = -1;
		goto out;
	}
	invalidate_dcache_range((unsigned long)srcptr, (unsigned long)srcptr + ENET_BUF_SIZE);
	barrier();

	*packetp = softc->rcv_buffer;

	memcpy (*packetp, srcptr, length);
	rc = length;

out:
	CurrentBdPtr->length = ENET_BUF_SIZE;
	CurrentBdPtr->status &= DMA_WRAP;
	CurrentBdPtr->status |= DMA_OWN;

	IncRxBdPtr( CurrentBdPtr, softc );
	softc->rxBdReadPtr = CurrentBdPtr;
	softc->dmaCtrl->flowctl_ch1_alloc = 1;

	// enable rx dma
	softc->rxDma->cfg = DMA_ENABLE;
	return rc;
}

static int sf2gmac_free_pkt (struct udevice *dev, uchar *packet, int length)
{
	struct sf2gmac_priv *priv = dev_get_priv(dev);
	if (packet != priv->rcv_buffer)
		printf("why free this? %p\n", packet);

	return 0;
}

static const struct eth_ops sf2gmac_eth_ops = {
	.start = bcmbca_sf2gmac_eth_start,
	.send = bcmbca_sf2gmac_eth_send,
	.recv = bcmbca_sf2gmac_eth_recv,
	.stop = bcmbca_sf2gmac_eth_stop,
	.free_pkt = sf2gmac_free_pkt,
	.read_rom_hwaddr = bcmbca_sf2gmac_eth_read_mac,
};

static int sf2gmac_probe (struct udevice *dev)
{
	int ret=0, size;
	struct resource res;
	struct udevice *sf2_devp;
	struct sf2gmac_priv *priv = dev_get_priv(dev);
	uint phandle_id;
	const fdt32_t *list;

	ret = dev_read_resource_byname(dev, "gmac-intf-base", &res);
	if (ret) {
		dev_err(dev, "can't get regs gmac-intf-base address(ret = %d)!\n", ret);
		return ret;
	}
	priv->gmac_intf = devm_ioremap(dev, res.start, resource_size(&res));

	ret = dev_read_resource_byname(dev, "gmac-mac-base", &res);
	if (ret) {
		dev_err(dev, "can't get regs gmac-mac-base address(ret = %d)!\n", ret);
		return ret;
	}
	priv->gmac_mac = devm_ioremap(dev, res.start, resource_size(&res));

	ret = dev_read_resource_byname(dev, "gmac-dma-base", &res);
	if (ret) {
		dev_err(dev, "can't get regs gmac-dma-base address(ret = %d)!\n", ret);
		return ret;
	}
	priv->dmaCtrl = devm_ioremap(dev, res.start, resource_size(&res));

	priv->sf2_devp = NULL;
	list = dev_read_prop (dev, "ethsw", &size);
	if (size > 0) {
		phandle_id = fdt32_to_cpu(*list);
		if (!uclass_get_device_by_phandle_id(UCLASS_NOP, phandle_id, &sf2_devp))
		{
			printf("sf2 found, should have been probed name %s driver name %s\n", 
			sf2_devp->name, sf2_devp->driver->name);
			priv->sf2_devp = sf2_devp;
			priv->sf2_ops = dev_get_priv(sf2_devp);
			priv->sf2_ops->init (priv->sf2_devp);
			PowerOnDevice(PMB_ADDR_GMAC);
		}
	}
	// in case sf2 is not initialized, the sf2 gmac network interface will not work
	if (priv->sf2_devp == NULL) {
		printf("cannot find the pairing SF2 driver\n");
		return -1;
	}

	priv->txBds = NULL;
	priv->rxBds = NULL;	
	priv->init = 0;

	return 0;
}

static int sf2gmac_remove(struct udevice *dev)
{
	struct sf2gmac_priv *priv = dev_get_priv(dev);

	if (!priv->init)
		return 0;

	if (priv->sf2_devp)
		priv->sf2_ops->close (priv->sf2_devp);

	gmac_enable_port(priv, 0);

	free( (void *)(priv->rcv_buffer) );
	free( (void *)(priv->rxBuffers) );
	free( (void *)(priv->txBuffers) );
	priv->rxBuffers = NULL;
	priv->txBuffers = NULL;
	priv->init = 0;
	PowerOffDevice(PMB_ADDR_GMAC, 0);

	return 0;
}

static const struct udevice_id bcmbca_sf2gmac_match_ids[] = {
	{ .compatible = "brcm,bcmbca-sf2gmac"},
	{ }
};


U_BOOT_DRIVER(sf2gmac) = {
	.name	= "sf2-gmac",
	.id	= UCLASS_ETH,
	.of_match = bcmbca_sf2gmac_match_ids,
	.flags  = DM_REMOVE_ACTIVE_ALL,
	.probe	= sf2gmac_probe,
	.remove	= sf2gmac_remove,
	.ops	= &sf2gmac_eth_ops,
	.priv_auto_alloc_size = sizeof(struct sf2gmac_priv),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
};

