// SPDX-License-Identifier: GPL-2.0+
/*
 * (c) 2015 Purna Chandra Mandal <purna.mandal@microchip.com>
 *
 */
#include <common.h>
#include <cpu_func.h>
#include <errno.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <net.h>
#include <miiphy.h>
#include <console.h>
#include <time.h>
#include <wait_bit.h>
#include <asm/global_data.h>
#include <asm/gpio.h>
#include <linux/delay.h>
#include <linux/mii.h>

#include "pic32_eth.h"

#define MAX_RX_BUF_SIZE		1536
#define MAX_RX_DESCR		PKTBUFSRX
#define MAX_TX_DESCR		2

DECLARE_GLOBAL_DATA_PTR;

struct pic32eth_dev {
	struct eth_dma_desc rxd_ring[MAX_RX_DESCR];
	struct eth_dma_desc txd_ring[MAX_TX_DESCR];
	u32 rxd_idx; /* index of RX desc to read */
	/* regs */
	struct pic32_ectl_regs *ectl_regs;
	struct pic32_emac_regs *emac_regs;
	/* Phy */
	struct phy_device *phydev;
	phy_interface_t phyif;
	u32 phy_addr;
	struct gpio_desc rst_gpio;
};

void __weak board_netphy_reset(void *dev)
{
	struct pic32eth_dev *priv = dev;

	if (!dm_gpio_is_valid(&priv->rst_gpio))
		return;

	/* phy reset */
	dm_gpio_set_value(&priv->rst_gpio, 0);
	udelay(300);
	dm_gpio_set_value(&priv->rst_gpio, 1);
	udelay(300);
}

/* Initialize mii(MDIO) interface, discover which PHY is
 * attached to the device, and configure it properly.
 */
static int pic32_mii_init(struct pic32eth_dev *priv)
{
	struct pic32_ectl_regs *ectl_p = priv->ectl_regs;
	struct pic32_emac_regs *emac_p = priv->emac_regs;

	/* board phy reset */
	board_netphy_reset(priv);

	/* disable RX, TX & all transactions */
	writel(ETHCON_ON | ETHCON_TXRTS | ETHCON_RXEN, &ectl_p->con1.clr);

	/* wait till busy */
	wait_for_bit_le32(&ectl_p->stat.raw, ETHSTAT_BUSY, false,
			  CONFIG_SYS_HZ, false);

	/* turn controller ON to access PHY over MII */
	writel(ETHCON_ON, &ectl_p->con1.set);

	mdelay(10);

	/* reset MAC */
	writel(EMAC_SOFTRESET, &emac_p->cfg1.set); /* reset assert */
	mdelay(10);
	writel(EMAC_SOFTRESET, &emac_p->cfg1.clr); /* reset deassert */

	/* initialize MDIO/MII */
	if (priv->phyif == PHY_INTERFACE_MODE_RMII) {
		writel(EMAC_RMII_RESET, &emac_p->supp.set);
		mdelay(10);
		writel(EMAC_RMII_RESET, &emac_p->supp.clr);
	}

	return pic32_mdio_init(PIC32_MDIO_NAME, (ulong)&emac_p->mii);
}

static int pic32_phy_init(struct pic32eth_dev *priv, struct udevice *dev)
{
	struct mii_dev *mii;

	mii = miiphy_get_dev_by_name(PIC32_MDIO_NAME);

	/* find & connect PHY */
	priv->phydev = phy_connect(mii, priv->phy_addr,
				   dev, priv->phyif);
	if (!priv->phydev) {
		printf("%s: %s: Error, PHY connect\n", __FILE__, __func__);
		return 0;
	}

	/* Wait for phy to complete reset */
	mdelay(10);

	/* configure supported modes */
	priv->phydev->supported = SUPPORTED_10baseT_Half |
				  SUPPORTED_10baseT_Full |
				  SUPPORTED_100baseT_Half |
				  SUPPORTED_100baseT_Full |
				  SUPPORTED_Autoneg;

	priv->phydev->advertising = ADVERTISED_10baseT_Half |
				    ADVERTISED_10baseT_Full |
				    ADVERTISED_100baseT_Half |
				    ADVERTISED_100baseT_Full |
				    ADVERTISED_Autoneg;

	priv->phydev->autoneg = AUTONEG_ENABLE;

	return 0;
}

/* Configure MAC based on negotiated speed and duplex
 * reported by PHY.
 */
static int pic32_mac_adjust_link(struct pic32eth_dev *priv)
{
	struct phy_device *phydev = priv->phydev;
	struct pic32_emac_regs *emac_p = priv->emac_regs;

	if (!phydev->link) {
		printf("%s: No link.\n", phydev->dev->name);
		return -EINVAL;
	}

	if (phydev->duplex) {
		writel(EMAC_FULLDUP, &emac_p->cfg2.set);
		writel(FULLDUP_GAP_TIME, &emac_p->ipgt.raw);
	} else {
		writel(EMAC_FULLDUP, &emac_p->cfg2.clr);
		writel(HALFDUP_GAP_TIME, &emac_p->ipgt.raw);
	}

	switch (phydev->speed) {
	case SPEED_100:
		writel(EMAC_RMII_SPD100, &emac_p->supp.set);
		break;
	case SPEED_10:
		writel(EMAC_RMII_SPD100, &emac_p->supp.clr);
		break;
	default:
		printf("%s: Speed was bad\n", phydev->dev->name);
		return -EINVAL;
	}

	printf("pic32eth: PHY is %s with %dbase%s, %s\n",
	       phydev->drv->name, phydev->speed,
	       (phydev->port == PORT_TP) ? "T" : "X",
	       (phydev->duplex) ? "full" : "half");

	return 0;
}

static void pic32_mac_init(struct pic32eth_dev *priv, u8 *macaddr)
{
	struct pic32_emac_regs *emac_p = priv->emac_regs;
	u32 stat = 0, v;
	u64 expire;

	v = EMAC_TXPAUSE | EMAC_RXPAUSE | EMAC_RXENABLE;
	writel(v, &emac_p->cfg1.raw);

	v = EMAC_EXCESS | EMAC_AUTOPAD | EMAC_PADENABLE |
	    EMAC_CRCENABLE | EMAC_LENGTHCK | EMAC_FULLDUP;
	writel(v, &emac_p->cfg2.raw);

	/* recommended back-to-back inter-packet gap for 10 Mbps half duplex */
	writel(HALFDUP_GAP_TIME, &emac_p->ipgt.raw);

	/* recommended non-back-to-back interpacket gap is 0xc12 */
	writel(0xc12, &emac_p->ipgr.raw);

	/* recommended collision window retry limit is 0x370F */
	writel(0x370f, &emac_p->clrt.raw);

	/* set maximum frame length: allow VLAN tagged frame */
	writel(0x600, &emac_p->maxf.raw);

	/* set the mac address */
	writel(macaddr[0] | (macaddr[1] << 8), &emac_p->sa2.raw);
	writel(macaddr[2] | (macaddr[3] << 8), &emac_p->sa1.raw);
	writel(macaddr[4] | (macaddr[5] << 8), &emac_p->sa0.raw);

	/* default, enable 10 Mbps operation */
	writel(EMAC_RMII_SPD100, &emac_p->supp.clr);

	/* wait until link status UP or deadline elapsed */
	expire = get_ticks() + get_tbclk() * 2;
	for (; get_ticks() < expire;) {
		stat = phy_read(priv->phydev, priv->phy_addr, MII_BMSR);
		if (stat & BMSR_LSTATUS)
			break;
	}

	if (!(stat & BMSR_LSTATUS))
		printf("MAC: Link is DOWN!\n");

	/* delay to stabilize before any tx/rx */
	mdelay(10);
}

static void pic32_mac_reset(struct pic32eth_dev *priv)
{
	struct pic32_emac_regs *emac_p = priv->emac_regs;
	struct mii_dev *mii;

	/* Reset MAC */
	writel(EMAC_SOFTRESET, &emac_p->cfg1.raw);
	mdelay(10);

	/* clear reset */
	writel(0, &emac_p->cfg1.raw);

	/* Reset MII */
	mii = priv->phydev->bus;
	if (mii && mii->reset)
		mii->reset(mii);
}

/* initializes the MAC and PHY, then establishes a link */
static void pic32_ctrl_reset(struct pic32eth_dev *priv)
{
	struct pic32_ectl_regs *ectl_p = priv->ectl_regs;
	u32 v;

	/* disable RX, TX & any other transactions */
	writel(ETHCON_ON | ETHCON_TXRTS | ETHCON_RXEN, &ectl_p->con1.clr);

	/* wait till busy */
	wait_for_bit_le32(&ectl_p->stat.raw, ETHSTAT_BUSY, false,
			  CONFIG_SYS_HZ, false);
	/* decrement received buffcnt to zero. */
	while (readl(&ectl_p->stat.raw) & ETHSTAT_BUFCNT)
		writel(ETHCON_BUFCDEC, &ectl_p->con1.set);

	/* clear any existing interrupt event */
	writel(0xffffffff, &ectl_p->irq.clr);

	/* clear RX/TX start address */
	writel(0xffffffff, &ectl_p->txst.clr);
	writel(0xffffffff, &ectl_p->rxst.clr);

	/* clear the receive filters */
	writel(0x00ff, &ectl_p->rxfc.clr);

	/* set the receive filters
	 * ETH_FILT_CRC_ERR_REJECT
	 * ETH_FILT_RUNT_REJECT
	 * ETH_FILT_UCAST_ACCEPT
	 * ETH_FILT_MCAST_ACCEPT
	 * ETH_FILT_BCAST_ACCEPT
	 */
	v = ETHRXFC_BCEN | ETHRXFC_MCEN | ETHRXFC_UCEN |
	    ETHRXFC_RUNTEN | ETHRXFC_CRCOKEN;
	writel(v, &ectl_p->rxfc.set);

	/* turn controller ON to access PHY over MII */
	writel(ETHCON_ON, &ectl_p->con1.set);
}

static void pic32_rx_desc_init(struct pic32eth_dev *priv)
{
	struct pic32_ectl_regs *ectl_p = priv->ectl_regs;
	struct eth_dma_desc *rxd;
	u32 idx, bufsz;

	priv->rxd_idx = 0;
	for (idx = 0; idx < MAX_RX_DESCR; idx++) {
		rxd = &priv->rxd_ring[idx];

		/* hw owned */
		rxd->hdr = EDH_NPV | EDH_EOWN | EDH_STICKY;

		/* packet buffer address */
		rxd->data_buff = virt_to_phys(net_rx_packets[idx]);

		/* link to next desc */
		rxd->next_ed = virt_to_phys(rxd + 1);

		/* reset status */
		rxd->stat1 = 0;
		rxd->stat2 = 0;

		/* decrement bufcnt */
		writel(ETHCON_BUFCDEC, &ectl_p->con1.set);
	}

	/* link last descr to beginning of list */
	rxd->next_ed = virt_to_phys(&priv->rxd_ring[0]);

	/* flush rx ring */
	flush_dcache_range((ulong)priv->rxd_ring,
			   (ulong)priv->rxd_ring + sizeof(priv->rxd_ring));

	/* set rx desc-ring start address */
	writel((ulong)virt_to_phys(&priv->rxd_ring[0]), &ectl_p->rxst.raw);

	/* RX Buffer size */
	bufsz = readl(&ectl_p->con2.raw);
	bufsz &= ~(ETHCON_RXBUFSZ << ETHCON_RXBUFSZ_SHFT);
	bufsz |= ((MAX_RX_BUF_SIZE / 16) << ETHCON_RXBUFSZ_SHFT);
	writel(bufsz, &ectl_p->con2.raw);

	/* enable the receiver in hardware which allows hardware
	 * to DMA received pkts to the descriptor pointer address.
	 */
	writel(ETHCON_RXEN, &ectl_p->con1.set);
}

static int pic32_eth_start(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct pic32eth_dev *priv = dev_get_priv(dev);

	/* controller */
	pic32_ctrl_reset(priv);

	/* reset MAC */
	pic32_mac_reset(priv);

	/* configure PHY */
	phy_config(priv->phydev);

	/* initialize MAC */
	pic32_mac_init(priv, &pdata->enetaddr[0]);

	/* init RX descriptor; TX descriptors are handled in xmit */
	pic32_rx_desc_init(priv);

	/* Start up & update link status of PHY */
	phy_startup(priv->phydev);

	/* adjust mac with phy link status */
	return pic32_mac_adjust_link(priv);
}

static void pic32_eth_stop(struct udevice *dev)
{
	struct pic32eth_dev *priv = dev_get_priv(dev);
	struct pic32_ectl_regs *ectl_p = priv->ectl_regs;
	struct pic32_emac_regs *emac_p = priv->emac_regs;

	/* Reset the phy if the controller is enabled */
	if (readl(&ectl_p->con1.raw) & ETHCON_ON)
		phy_reset(priv->phydev);

	/* Shut down the PHY */
	phy_shutdown(priv->phydev);

	/* Stop rx/tx */
	writel(ETHCON_TXRTS | ETHCON_RXEN, &ectl_p->con1.clr);
	mdelay(10);

	/* reset MAC */
	writel(EMAC_SOFTRESET, &emac_p->cfg1.raw);

	/* clear reset */
	writel(0, &emac_p->cfg1.raw);
	mdelay(10);

	/* disable controller */
	writel(ETHCON_ON, &ectl_p->con1.clr);
	mdelay(10);

	/* wait until everything is down */
	wait_for_bit_le32(&ectl_p->stat.raw, ETHSTAT_BUSY, false,
			  2 * CONFIG_SYS_HZ, false);

	/* clear any existing interrupt event */
	writel(0xffffffff, &ectl_p->irq.clr);
}

static int pic32_eth_send(struct udevice *dev, void *packet, int length)
{
	struct pic32eth_dev *priv = dev_get_priv(dev);
	struct pic32_ectl_regs *ectl_p = priv->ectl_regs;
	struct eth_dma_desc *txd;
	u64 deadline;

	txd = &priv->txd_ring[0];

	/* set proper flags & length in descriptor header */
	txd->hdr = EDH_SOP | EDH_EOP | EDH_EOWN | EDH_BCOUNT(length);

	/* pass buffer address to hardware */
	txd->data_buff = virt_to_phys(packet);

	debug("%s: %d / .hdr %x, .data_buff %x, .stat %x, .nexted %x\n",
	      __func__, __LINE__, txd->hdr, txd->data_buff, txd->stat2,
	      txd->next_ed);

	/* cache flush (packet) */
	flush_dcache_range((ulong)packet, (ulong)packet + length);

	/* cache flush (txd) */
	flush_dcache_range((ulong)txd, (ulong)txd + sizeof(*txd));

	/* pass descriptor table base to h/w */
	writel(virt_to_phys(txd), &ectl_p->txst.raw);

	/* ready to send enabled, hardware can now send the packet(s) */
	writel(ETHCON_TXRTS | ETHCON_ON, &ectl_p->con1.set);

	/* wait until tx has completed and h/w has released ownership
	 * of the tx descriptor or timeout elapsed.
	 */
	deadline = get_ticks() + get_tbclk();
	for (;;) {
		/* check timeout */
		if (get_ticks() > deadline)
			return -ETIMEDOUT;

		if (ctrlc())
			return -EINTR;

		/* tx completed ? */
		if (readl(&ectl_p->con1.raw) & ETHCON_TXRTS) {
			udelay(1);
			continue;
		}

		/* h/w not released ownership yet? */
		invalidate_dcache_range((ulong)txd, (ulong)txd + sizeof(*txd));
		if (!(txd->hdr & EDH_EOWN))
			break;
	}

	return 0;
}

static int pic32_eth_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct pic32eth_dev *priv = dev_get_priv(dev);
	struct eth_dma_desc *rxd;
	u32 idx = priv->rxd_idx;
	u32 rx_count;

	/* find the next ready to receive */
	rxd = &priv->rxd_ring[idx];

	invalidate_dcache_range((ulong)rxd, (ulong)rxd + sizeof(*rxd));
	/* check if owned by MAC */
	if (rxd->hdr & EDH_EOWN)
		return -EAGAIN;

	/* Sanity check on header: SOP and EOP  */
	if ((rxd->hdr & (EDH_SOP | EDH_EOP)) != (EDH_SOP | EDH_EOP)) {
		printf("%s: %s, rx pkt across multiple descr\n",
		       __FILE__, __func__);
		return 0;
	}

	debug("%s: %d /idx %i, hdr=%x, data_buff %x, stat %x, nexted %x\n",
	      __func__, __LINE__, idx, rxd->hdr,
	      rxd->data_buff, rxd->stat2, rxd->next_ed);

	/* Sanity check on rx_stat: OK, CRC */
	if (!RSV_RX_OK(rxd->stat2) || RSV_CRC_ERR(rxd->stat2)) {
		debug("%s: %s: Error, rx problem detected\n",
		      __FILE__, __func__);
		return 0;
	}

	/* invalidate dcache */
	rx_count = RSV_RX_COUNT(rxd->stat2);
	invalidate_dcache_range((ulong)net_rx_packets[idx],
				(ulong)net_rx_packets[idx] + rx_count);

	/* Pass the packet to protocol layer */
	*packetp = net_rx_packets[idx];

	/* increment number of bytes rcvd (ignore CRC) */
	return rx_count - 4;
}

static int pic32_eth_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct pic32eth_dev *priv = dev_get_priv(dev);
	struct pic32_ectl_regs *ectl_p = priv->ectl_regs;
	struct eth_dma_desc *rxd;
	int idx = priv->rxd_idx;

	/* sanity check */
	if (packet != net_rx_packets[idx]) {
		printf("rxd_id %d: packet is not matched,\n", idx);
		return -EAGAIN;
	}

	/* prepare for receive */
	rxd = &priv->rxd_ring[idx];
	rxd->hdr = EDH_STICKY | EDH_NPV | EDH_EOWN;

	flush_dcache_range((ulong)rxd, (ulong)rxd + sizeof(*rxd));

	/* decrement rx pkt count */
	writel(ETHCON_BUFCDEC, &ectl_p->con1.set);

	debug("%s: %d / idx %i, hdr %x, data_buff %x, stat %x, nexted %x\n",
	      __func__, __LINE__, idx, rxd->hdr, rxd->data_buff,
	      rxd->stat2, rxd->next_ed);

	priv->rxd_idx = (priv->rxd_idx + 1) % MAX_RX_DESCR;

	return 0;
}

static const struct eth_ops pic32_eth_ops = {
	.start		= pic32_eth_start,
	.send		= pic32_eth_send,
	.recv		= pic32_eth_recv,
	.free_pkt	= pic32_eth_free_pkt,
	.stop		= pic32_eth_stop,
};

static int pic32_eth_probe(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct pic32eth_dev *priv = dev_get_priv(dev);
	void __iomem *iobase;
	fdt_addr_t addr;
	fdt_size_t size;
	int offset = 0;
	int phy_addr = -1;

	addr = fdtdec_get_addr_size(gd->fdt_blob, dev_of_offset(dev), "reg",
				    &size);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	iobase = ioremap(addr, size);
	pdata->iobase = (phys_addr_t)addr;

	/* get phy mode */
	pdata->phy_interface = dev_read_phy_mode(dev);
	if (pdata->phy_interface == PHY_INTERFACE_MODE_NA)
		return -EINVAL;

	/* get phy addr */
	offset = fdtdec_lookup_phandle(gd->fdt_blob, dev_of_offset(dev),
				       "phy-handle");
	if (offset > 0)
		phy_addr = fdtdec_get_int(gd->fdt_blob, offset, "reg", -1);

	/* phy reset gpio */
	gpio_request_by_name_nodev(dev_ofnode(dev), "reset-gpios", 0,
				   &priv->rst_gpio, GPIOD_IS_OUT);

	priv->phyif	= pdata->phy_interface;
	priv->phy_addr	= phy_addr;
	priv->ectl_regs	= iobase;
	priv->emac_regs	= iobase + PIC32_EMAC1CFG1;

	pic32_mii_init(priv);

	return pic32_phy_init(priv, dev);
}

static int pic32_eth_remove(struct udevice *dev)
{
	struct pic32eth_dev *priv = dev_get_priv(dev);
	struct mii_dev *bus;

	dm_gpio_free(dev, &priv->rst_gpio);
	phy_shutdown(priv->phydev);
	free(priv->phydev);
	bus = miiphy_get_dev_by_name(PIC32_MDIO_NAME);
	mdio_unregister(bus);
	mdio_free(bus);
	iounmap(priv->ectl_regs);
	return 0;
}

static const struct udevice_id pic32_eth_ids[] = {
	{ .compatible = "microchip,pic32mzda-eth" },
	{ }
};

U_BOOT_DRIVER(pic32_ethernet) = {
	.name			= "pic32_ethernet",
	.id			= UCLASS_ETH,
	.of_match		= pic32_eth_ids,
	.probe			= pic32_eth_probe,
	.remove			= pic32_eth_remove,
	.ops			= &pic32_eth_ops,
	.priv_auto	= sizeof(struct pic32eth_dev),
	.plat_auto	= sizeof(struct eth_pdata),
};
