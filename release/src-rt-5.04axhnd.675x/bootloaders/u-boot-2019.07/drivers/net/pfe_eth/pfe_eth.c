// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2015-2016 Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
 */

#include <common.h>
#include <dm.h>
#include <dm/platform_data/pfe_dm_eth.h>
#include <net.h>
#include <net/pfe_eth/pfe_eth.h>
#include <net/pfe_eth/pfe_mdio.h>

struct gemac_s gem_info[] = {
	/* PORT_0 configuration */
	{
		/* GEMAC config */
		.gemac_speed = PFE_MAC_SPEED_1000M,
		.gemac_duplex = DUPLEX_FULL,

		/* phy iface */
		.phy_address = CONFIG_PFE_EMAC1_PHY_ADDR,
		.phy_mode = PHY_INTERFACE_MODE_SGMII,
	},
	/* PORT_1 configuration */
	{
		/* GEMAC config */
		.gemac_speed = PFE_MAC_SPEED_1000M,
		.gemac_duplex = DUPLEX_FULL,

		/* phy iface */
		.phy_address = CONFIG_PFE_EMAC2_PHY_ADDR,
		.phy_mode = PHY_INTERFACE_MODE_RGMII_TXID,
	},
};

static inline void pfe_gemac_enable(void *gemac_base)
{
	writel(readl(gemac_base + EMAC_ECNTRL_REG) |
		EMAC_ECNTRL_ETHER_EN, gemac_base + EMAC_ECNTRL_REG);
}

static inline void pfe_gemac_disable(void *gemac_base)
{
	writel(readl(gemac_base + EMAC_ECNTRL_REG) &
		~EMAC_ECNTRL_ETHER_EN, gemac_base + EMAC_ECNTRL_REG);
}

static inline void pfe_gemac_set_speed(void *gemac_base, u32 speed)
{
	struct ccsr_scfg *scfg = (struct ccsr_scfg *)CONFIG_SYS_FSL_SCFG_ADDR;
	u32 ecr = readl(gemac_base + EMAC_ECNTRL_REG) & ~EMAC_ECNTRL_SPEED;
	u32 rcr = readl(gemac_base + EMAC_RCNTRL_REG) & ~EMAC_RCNTRL_RMII_10T;
	u32 rgmii_pcr = in_be32(&scfg->rgmiipcr) &
			~(SCFG_RGMIIPCR_SETSP_1000M | SCFG_RGMIIPCR_SETSP_10M);

	if (speed == _1000BASET) {
		ecr |= EMAC_ECNTRL_SPEED;
		rgmii_pcr |= SCFG_RGMIIPCR_SETSP_1000M;
	} else if (speed != _100BASET) {
		rcr |= EMAC_RCNTRL_RMII_10T;
		rgmii_pcr |= SCFG_RGMIIPCR_SETSP_10M;
	}

	writel(ecr, gemac_base + EMAC_ECNTRL_REG);
	out_be32(&scfg->rgmiipcr, rgmii_pcr | SCFG_RGMIIPCR_SETFD);

	/* remove loop back */
	rcr &= ~EMAC_RCNTRL_LOOP;
	/* enable flow control */
	rcr |= EMAC_RCNTRL_FCE;

	/* Enable MII mode */
	rcr |= EMAC_RCNTRL_MII_MODE;

	writel(rcr, gemac_base + EMAC_RCNTRL_REG);

	/* Enable Tx full duplex */
	writel(readl(gemac_base + EMAC_TCNTRL_REG) | EMAC_TCNTRL_FDEN,
	       gemac_base + EMAC_TCNTRL_REG);
}

static int pfe_eth_write_hwaddr(struct udevice *dev)
{
	struct pfe_eth_dev *priv = dev_get_priv(dev);
	struct gemac_s *gem = priv->gem;
	struct eth_pdata *pdata = dev_get_platdata(dev);
	uchar *mac = pdata->enetaddr;

	writel((mac[0] << 24) + (mac[1] << 16) + (mac[2] << 8) + mac[3],
	       gem->gemac_base + EMAC_PHY_ADDR_LOW);
	writel((mac[4] << 24) + (mac[5] << 16) + 0x8808, gem->gemac_base +
	       EMAC_PHY_ADDR_HIGH);
	return 0;
}

/** Stops or Disables GEMAC pointing to this eth iface.
 *
 * @param[in]   edev    Pointer to eth device structure.
 *
 * @return      none
 */
static inline void pfe_eth_stop(struct udevice *dev)
{
	struct pfe_eth_dev *priv = dev_get_priv(dev);

	pfe_gemac_disable(priv->gem->gemac_base);

	gpi_disable(priv->gem->egpi_base);
}

static int pfe_eth_start(struct udevice *dev)
{
	struct pfe_eth_dev *priv = dev_get_priv(dev);
	struct gemac_s *gem = priv->gem;
	int speed;

	/* set ethernet mac address */
	pfe_eth_write_hwaddr(dev);

	writel(EMAC_TFWR, gem->gemac_base + EMAC_TFWR_STR_FWD);
	writel(EMAC_RX_SECTION_FULL_32, gem->gemac_base + EMAC_RX_SECTIOM_FULL);
	writel(EMAC_TRUNC_FL_16K, gem->gemac_base + EMAC_TRUNC_FL);
	writel(EMAC_TX_SECTION_EMPTY_30, gem->gemac_base
	       + EMAC_TX_SECTION_EMPTY);
	writel(EMAC_MIBC_NO_CLR_NO_DIS, gem->gemac_base
	       + EMAC_MIB_CTRL_STS_REG);

#ifdef CONFIG_PHYLIB
	/* Start up the PHY */
	if (phy_startup(priv->phydev)) {
		printf("Could not initialize PHY %s\n",
		       priv->phydev->dev->name);
		return -1;
	}
	speed = priv->phydev->speed;
	printf("Speed detected %x\n", speed);
	if (priv->phydev->duplex == DUPLEX_HALF) {
		printf("Half duplex not supported\n");
		return -1;
	}
#endif

	pfe_gemac_set_speed(gem->gemac_base, speed);

	/* Enable GPI */
	gpi_enable(gem->egpi_base);

	/* Enable GEMAC */
	pfe_gemac_enable(gem->gemac_base);

	return 0;
}

static int pfe_eth_send(struct udevice *dev, void *packet, int length)
{
	struct pfe_eth_dev *priv = (struct pfe_eth_dev *)dev->priv;

	int rc;
	int i = 0;

	rc = pfe_send(priv->gemac_port, packet, length);

	if (rc < 0) {
		printf("Tx Queue full\n");
		return rc;
	}

	while (1) {
		rc = pfe_tx_done();
		if (rc == 0)
			break;

		udelay(100);
		i++;
		if (i == 30000)
			printf("Tx timeout, send failed\n");
		break;
	}

	return 0;
}

static int pfe_eth_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct pfe_eth_dev *priv = dev_get_priv(dev);
	uchar *pkt_buf;
	int len;
	int phy_port;

	len = pfe_recv(&pkt_buf, &phy_port);

	if (len == 0)
		return -EAGAIN; /* no packet in rx */
	else if  (len < 0)
		return -EAGAIN;

	debug("Rx pkt: pkt_buf(0x%p), phy_port(%d), len(%d)\n", pkt_buf,
	      phy_port, len);
	if (phy_port != priv->gemac_port)  {
		printf("Rx pkt not on expected port\n");
		return -EAGAIN;
	}

	*packetp = pkt_buf;

	return len;
}

static int pfe_eth_probe(struct udevice *dev)
{
	struct pfe_eth_dev *priv = dev_get_priv(dev);
	struct pfe_ddr_address *pfe_addr;
	struct pfe_eth_pdata *pdata = dev_get_platdata(dev);
	int ret = 0;
	static int init_done;

	if (!init_done) {
		pfe_addr = (struct pfe_ddr_address *)malloc(sizeof
						    (struct pfe_ddr_address));
		if (!pfe_addr)
			return -ENOMEM;

		pfe_addr->ddr_pfe_baseaddr =
				(void *)pdata->pfe_ddr_addr.ddr_pfe_baseaddr;
		pfe_addr->ddr_pfe_phys_baseaddr =
		(unsigned long)pdata->pfe_ddr_addr.ddr_pfe_phys_baseaddr;

		debug("ddr_pfe_baseaddr: %p, ddr_pfe_phys_baseaddr: %08x\n",
		      pfe_addr->ddr_pfe_baseaddr,
		      (u32)pfe_addr->ddr_pfe_phys_baseaddr);

		ret = pfe_drv_init(pfe_addr);
		if (ret)
			return ret;

		init_pfe_scfg_dcfg_regs();
		init_done = 1;
	}

	priv->gemac_port = pdata->pfe_eth_pdata_mac.phy_interface;
	priv->gem = &gem_info[priv->gemac_port];
	priv->dev = dev;

	switch (priv->gemac_port)  {
	case EMAC_PORT_0:
	default:
		priv->gem->gemac_base = EMAC1_BASE_ADDR;
		priv->gem->egpi_base = EGPI1_BASE_ADDR;
		break;
	case EMAC_PORT_1:
		priv->gem->gemac_base = EMAC2_BASE_ADDR;
		priv->gem->egpi_base = EGPI2_BASE_ADDR;
		break;
	}

	ret = pfe_eth_board_init(dev);
	if (ret)
		return ret;

#if defined(CONFIG_PHYLIB)
	ret = pfe_phy_configure(priv, pdata->pfe_eth_pdata_mac.phy_interface,
				gem_info[priv->gemac_port].phy_address);
#endif
	return ret;
}

static int pfe_eth_bind(struct udevice *dev)
{
	struct pfe_eth_pdata *pdata = dev_get_platdata(dev);
	char name[20];

	sprintf(name, "pfe_eth%u", pdata->pfe_eth_pdata_mac.phy_interface);

	return device_set_name(dev, name);
}

static const struct eth_ops pfe_eth_ops = {
	.start		= pfe_eth_start,
	.send		= pfe_eth_send,
	.recv		= pfe_eth_recv,
	.free_pkt	= pfe_eth_free_pkt,
	.stop		= pfe_eth_stop,
	.write_hwaddr	= pfe_eth_write_hwaddr,
};

U_BOOT_DRIVER(pfe_eth) = {
	.name	= "pfe_eth",
	.id	= UCLASS_ETH,
	.bind	= pfe_eth_bind,
	.probe	= pfe_eth_probe,
	.remove = pfe_eth_remove,
	.ops	= &pfe_eth_ops,
	.priv_auto_alloc_size = sizeof(struct pfe_eth_dev),
	.platdata_auto_alloc_size = sizeof(struct pfe_eth_pdata)
};
