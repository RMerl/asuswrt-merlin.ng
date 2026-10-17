// SPDX-License-Identifier: GPL-2.0+
/*
 * ENETC ethernet controller driver
 * Copyright 2017-2021 NXP
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdt_support.h>
#include <malloc.h>
#include <memalign.h>
#include <net.h>
#include <asm/cache.h>
#include <asm/io.h>
#include <pci.h>
#include <miiphy.h>
#include <linux/bug.h>
#include <linux/delay.h>

#include "fsl_enetc.h"

#define ENETC_DRIVER_NAME	"enetc_eth"

static int enetc_remove(struct udevice *dev);

/*
 * sets the MAC address in IERB registers, this setting is persistent and
 * carried over to Linux.
 */
static void enetc_set_ierb_primary_mac(struct udevice *dev, int devfn,
				       const u8 *enetaddr)
{
#ifdef CONFIG_ARCH_LS1028A
/*
 * LS1028A is the only part with IERB at this time and there are plans to change
 * its structure, keep this LS1028A specific for now
 */
#define IERB_BASE		0x1f0800000ULL
#define IERB_PFMAC(pf, vf, n)	(IERB_BASE + 0x8000 + (pf) * 0x100 + (vf) * 8 \
				 + (n) * 4)

static int ierb_fn_to_pf[] = {0, 1, 2, -1, -1, -1, 3};

	u16 lower = *(const u16 *)(enetaddr + 4);
	u32 upper = *(const u32 *)enetaddr;

	if (ierb_fn_to_pf[devfn] < 0)
		return;

	out_le32(IERB_PFMAC(ierb_fn_to_pf[devfn], 0, 0), upper);
	out_le32(IERB_PFMAC(ierb_fn_to_pf[devfn], 0, 1), (u32)lower);
#endif
}

/* sets up primary MAC addresses in DT/IERB */
void fdt_fixup_enetc_mac(void *blob)
{
	struct pci_child_plat *ppdata;
	struct eth_pdata *pdata;
	struct udevice *dev;
	struct uclass *uc;
	char path[256];
	int offset;
	int devfn;

	uclass_get(UCLASS_ETH, &uc);
	uclass_foreach_dev(dev, uc) {
		if (!dev->driver || !dev->driver->name ||
		    strcmp(dev->driver->name, ENETC_DRIVER_NAME))
			continue;

		pdata = dev_get_plat(dev);
		ppdata = dev_get_parent_plat(dev);
		devfn = PCI_FUNC(ppdata->devfn);

		enetc_set_ierb_primary_mac(dev, devfn, pdata->enetaddr);

		snprintf(path, 256, "/soc/pcie@1f0000000/ethernet@%x,%x",
			 PCI_DEV(ppdata->devfn), PCI_FUNC(ppdata->devfn));
		offset = fdt_path_offset(blob, path);
		if (offset < 0)
			continue;
		fdt_setprop(blob, offset, "mac-address", pdata->enetaddr, 6);
	}
}

/*
 * Bind the device:
 * - set a more explicit name on the interface
 */
static int enetc_bind(struct udevice *dev)
{
	char name[16];
	static int eth_num_devices;

	/*
	 * prefer using PCI function numbers to number interfaces, but these
	 * are only available if dts nodes are present.  For PCI they are
	 * optional, handle that case too.  Just in case some nodes are present
	 * and some are not, use different naming scheme - enetc-N based on
	 * PCI function # and enetc#N based on interface count
	 */
	if (ofnode_valid(dev_ofnode(dev)))
		sprintf(name, "enetc-%u", PCI_FUNC(pci_get_devfn(dev)));
	else
		sprintf(name, "enetc#%u", eth_num_devices++);
	device_set_name(dev, name);

	return 0;
}

/* MDIO wrappers, we're using these to drive internal MDIO to get to serdes */
static int enetc_mdio_read(struct mii_dev *bus, int addr, int devad, int reg)
{
	struct enetc_mdio_priv priv;

	priv.regs_base = bus->priv;
	return enetc_mdio_read_priv(&priv, addr, devad, reg);
}

static int enetc_mdio_write(struct mii_dev *bus, int addr, int devad, int reg,
			    u16 val)
{
	struct enetc_mdio_priv priv;

	priv.regs_base = bus->priv;
	return enetc_mdio_write_priv(&priv, addr, devad, reg, val);
}

/* only interfaces that can pin out through serdes have internal MDIO */
static bool enetc_has_imdio(struct udevice *dev)
{
	struct enetc_priv *priv = dev_get_priv(dev);

	return !!(priv->imdio.priv);
}

/* set up serdes for SGMII */
static int enetc_init_sgmii(struct udevice *dev)
{
	struct enetc_priv *priv = dev_get_priv(dev);
	bool is2500 = false;
	u16 reg;

	if (!enetc_has_imdio(dev))
		return 0;

	if (priv->uclass_id == PHY_INTERFACE_MODE_2500BASEX)
		is2500 = true;

	/*
	 * Set to SGMII mode, for 1Gbps enable AN, for 2.5Gbps set fixed speed.
	 * Although fixed speed is 1Gbps, we could be running at 2.5Gbps based
	 * on PLL configuration.  Setting 1G for 2.5G here is counter intuitive
	 * but intentional.
	 */
	reg = ENETC_PCS_IF_MODE_SGMII;
	reg |= is2500 ? ENETC_PCS_IF_MODE_SPEED_1G : ENETC_PCS_IF_MODE_SGMII_AN;
	enetc_mdio_write(&priv->imdio, ENETC_PCS_PHY_ADDR, MDIO_DEVAD_NONE,
			 ENETC_PCS_IF_MODE, reg);

	/* Dev ability - SGMII */
	enetc_mdio_write(&priv->imdio, ENETC_PCS_PHY_ADDR, MDIO_DEVAD_NONE,
			 ENETC_PCS_DEV_ABILITY, ENETC_PCS_DEV_ABILITY_SGMII);

	/* Adjust link timer for SGMII */
	enetc_mdio_write(&priv->imdio, ENETC_PCS_PHY_ADDR, MDIO_DEVAD_NONE,
			 ENETC_PCS_LINK_TIMER1, ENETC_PCS_LINK_TIMER1_VAL);
	enetc_mdio_write(&priv->imdio, ENETC_PCS_PHY_ADDR, MDIO_DEVAD_NONE,
			 ENETC_PCS_LINK_TIMER2, ENETC_PCS_LINK_TIMER2_VAL);

	reg = ENETC_PCS_CR_DEF_VAL;
	reg |= is2500 ? ENETC_PCS_CR_RST : ENETC_PCS_CR_RESET_AN;
	/* restart PCS AN */
	enetc_mdio_write(&priv->imdio, ENETC_PCS_PHY_ADDR, MDIO_DEVAD_NONE,
			 ENETC_PCS_CR, reg);

	return 0;
}

/* set up MAC for RGMII */
static void enetc_init_rgmii(struct udevice *dev, struct phy_device *phydev)
{
	struct enetc_priv *priv = dev_get_priv(dev);
	u32 old_val, val;

	old_val = val = enetc_read_port(priv, ENETC_PM_IF_MODE);

	/* disable unreliable RGMII in-band signaling and force the MAC into
	 * the speed negotiated by the PHY.
	 */
	val &= ~ENETC_PM_IF_MODE_AN_ENA;

	if (phydev->speed == SPEED_1000) {
		val &= ~ENETC_PM_IFM_SSP_MASK;
		val |= ENETC_PM_IFM_SSP_1000;
	} else if (phydev->speed == SPEED_100) {
		val &= ~ENETC_PM_IFM_SSP_MASK;
		val |= ENETC_PM_IFM_SSP_100;
	} else if (phydev->speed == SPEED_10) {
		val &= ~ENETC_PM_IFM_SSP_MASK;
		val |= ENETC_PM_IFM_SSP_10;
	}

	if (phydev->duplex == DUPLEX_FULL)
		val |= ENETC_PM_IFM_FULL_DPX;
	else
		val &= ~ENETC_PM_IFM_FULL_DPX;

	if (val == old_val)
		return;

	enetc_write_port(priv, ENETC_PM_IF_MODE, val);
}

/* set up MAC configuration for the given interface type */
static void enetc_setup_mac_iface(struct udevice *dev,
				  struct phy_device *phydev)
{
	struct enetc_priv *priv = dev_get_priv(dev);
	u32 if_mode;

	switch (priv->uclass_id) {
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
	case PHY_INTERFACE_MODE_RGMII_RXID:
	case PHY_INTERFACE_MODE_RGMII_TXID:
		enetc_init_rgmii(dev, phydev);
		break;
	case PHY_INTERFACE_MODE_USXGMII:
	case PHY_INTERFACE_MODE_10GBASER:
		/* set ifmode to (US)XGMII */
		if_mode = enetc_read_port(priv, ENETC_PM_IF_MODE);
		if_mode &= ~ENETC_PM_IF_IFMODE_MASK;
		enetc_write_port(priv, ENETC_PM_IF_MODE, if_mode);
		break;
	};
}

/* set up serdes for SXGMII */
static int enetc_init_sxgmii(struct udevice *dev)
{
	struct enetc_priv *priv = dev_get_priv(dev);

	if (!enetc_has_imdio(dev))
		return 0;

	/* Dev ability - SXGMII */
	enetc_mdio_write(&priv->imdio, ENETC_PCS_PHY_ADDR, ENETC_PCS_DEVAD_REPL,
			 ENETC_PCS_DEV_ABILITY, ENETC_PCS_DEV_ABILITY_SXGMII);

	/* Restart PCS AN */
	enetc_mdio_write(&priv->imdio, ENETC_PCS_PHY_ADDR, ENETC_PCS_DEVAD_REPL,
			 ENETC_PCS_CR,
			 ENETC_PCS_CR_RST | ENETC_PCS_CR_RESET_AN);

	return 0;
}

/* Apply protocol specific configuration to MAC, serdes as needed */
static void enetc_start_pcs(struct udevice *dev)
{
	struct enetc_priv *priv = dev_get_priv(dev);

	/* register internal MDIO for debug purposes */
	if (enetc_read_port(priv, ENETC_PCAPR0) & ENETC_PCAPRO_MDIO) {
		priv->imdio.read = enetc_mdio_read;
		priv->imdio.write = enetc_mdio_write;
		priv->imdio.priv = priv->port_regs + ENETC_PM_IMDIO_BASE;
		strlcpy(priv->imdio.name, dev->name, MDIO_NAME_LEN);
		if (!miiphy_get_dev_by_name(priv->imdio.name))
			mdio_register(&priv->imdio);
	}

	if (!ofnode_valid(dev_ofnode(dev))) {
		enetc_dbg(dev, "no enetc ofnode found, skipping PCS set-up\n");
		return;
	}

	priv->uclass_id = dev_read_phy_mode(dev);
	if (priv->uclass_id == PHY_INTERFACE_MODE_NA) {
		enetc_dbg(dev,
			  "phy-mode property not found, defaulting to SGMII\n");
		priv->uclass_id = PHY_INTERFACE_MODE_SGMII;
	}

	switch (priv->uclass_id) {
	case PHY_INTERFACE_MODE_SGMII:
	case PHY_INTERFACE_MODE_2500BASEX:
		enetc_init_sgmii(dev);
		break;
	case PHY_INTERFACE_MODE_USXGMII:
	case PHY_INTERFACE_MODE_10GBASER:
		enetc_init_sxgmii(dev);
		break;
	};
}

/* Configure the actual/external ethernet PHY, if one is found */
static int enetc_config_phy(struct udevice *dev)
{
	struct enetc_priv *priv = dev_get_priv(dev);
	int supported;

	priv->phy = dm_eth_phy_connect(dev);
	if (!priv->phy)
		return -ENODEV;

	supported = PHY_GBIT_FEATURES | SUPPORTED_2500baseX_Full;
	priv->phy->supported &= supported;
	priv->phy->advertising &= supported;

	return phy_config(priv->phy);
}

/*
 * Probe ENETC driver:
 * - initialize port and station interface BARs
 */
static int enetc_probe(struct udevice *dev)
{
	struct enetc_priv *priv = dev_get_priv(dev);
	int res;

	if (ofnode_valid(dev_ofnode(dev)) && !ofnode_is_enabled(dev_ofnode(dev))) {
		enetc_dbg(dev, "interface disabled\n");
		return -ENODEV;
	}

	priv->enetc_txbd = memalign(ENETC_BD_ALIGN,
				    sizeof(struct enetc_tx_bd) * ENETC_BD_CNT);
	priv->enetc_rxbd = memalign(ENETC_BD_ALIGN,
				    sizeof(union enetc_rx_bd) * ENETC_BD_CNT);

	if (!priv->enetc_txbd || !priv->enetc_rxbd) {
		/* free should be able to handle NULL, just free all pointers */
		free(priv->enetc_txbd);
		free(priv->enetc_rxbd);

		return -ENOMEM;
	}

	/* initialize register */
	priv->regs_base = dm_pci_map_bar(dev, PCI_BASE_ADDRESS_0, 0, 0, PCI_REGION_TYPE, 0);
	if (!priv->regs_base) {
		enetc_dbg(dev, "failed to map BAR0\n");
		return -EINVAL;
	}
	priv->port_regs = priv->regs_base + ENETC_PORT_REGS_OFF;

	dm_pci_clrset_config16(dev, PCI_COMMAND, 0, PCI_COMMAND_MEMORY);

	enetc_start_pcs(dev);

	res = enetc_config_phy(dev);
	if(res)
		enetc_remove(dev);
	return res;
}

/*
 * Remove the driver from an interface:
 * - free up allocated memory
 */
static int enetc_remove(struct udevice *dev)
{
	struct enetc_priv *priv = dev_get_priv(dev);

	if (miiphy_get_dev_by_name(priv->imdio.name))
		mdio_unregister(&priv->imdio);

	free(priv->enetc_txbd);
	free(priv->enetc_rxbd);

	return 0;
}

/*
 * LS1028A is the only part with IERB at this time and there are plans to
 * change its structure, keep this LS1028A specific for now.
 */
#define LS1028A_IERB_BASE		0x1f0800000ULL
#define LS1028A_IERB_PSIPMAR0(pf, vf)	(LS1028A_IERB_BASE + 0x8000 \
					 + (pf) * 0x100 + (vf) * 8)
#define LS1028A_IERB_PSIPMAR1(pf, vf)	(LS1028A_IERB_PSIPMAR0(pf, vf) + 4)

static int enetc_ls1028a_write_hwaddr(struct udevice *dev)
{
	struct pci_child_plat *ppdata = dev_get_parent_plat(dev);
	const int devfn_to_pf[] = {0, 1, 2, -1, -1, -1, 3};
	struct eth_pdata *plat = dev_get_plat(dev);
	int devfn = PCI_FUNC(ppdata->devfn);
	u8 *addr = plat->enetaddr;
	u32 lower, upper;
	int pf;

	if (devfn >= ARRAY_SIZE(devfn_to_pf))
		return 0;

	pf = devfn_to_pf[devfn];
	if (pf < 0)
		return 0;

	lower = *(const u16 *)(addr + 4);
	upper = *(const u32 *)addr;

	out_le32(LS1028A_IERB_PSIPMAR0(pf, 0), upper);
	out_le32(LS1028A_IERB_PSIPMAR1(pf, 0), lower);

	return 0;
}

static int enetc_write_hwaddr(struct udevice *dev)
{
	struct eth_pdata *plat = dev_get_plat(dev);
	struct enetc_priv *priv = dev_get_priv(dev);
	u8 *addr = plat->enetaddr;

	if (IS_ENABLED(CONFIG_ARCH_LS1028A))
		return enetc_ls1028a_write_hwaddr(dev);

	u16 lower = *(const u16 *)(addr + 4);
	u32 upper = *(const u32 *)addr;

	enetc_write_port(priv, ENETC_PSIPMAR0, upper);
	enetc_write_port(priv, ENETC_PSIPMAR1, lower);

	return 0;
}

/* Configure port parameters (# of rings, frame size, enable port) */
static void enetc_enable_si_port(struct enetc_priv *priv)
{
	u32 val;

	/* set Rx/Tx BDR count */
	val = ENETC_PSICFGR_SET_TXBDR(ENETC_TX_BDR_CNT);
	val |= ENETC_PSICFGR_SET_RXBDR(ENETC_RX_BDR_CNT);
	enetc_write_port(priv, ENETC_PSICFGR(0), val);
	/* set Rx max frame size */
	enetc_write_port(priv, ENETC_PM_MAXFRM, ENETC_RX_MAXFRM_SIZE);
	/* enable MAC port */
	enetc_write_port(priv, ENETC_PM_CC, ENETC_PM_CC_RX_TX_EN);
	/* enable port */
	enetc_write_port(priv, ENETC_PMR, ENETC_PMR_SI0_EN);
	/* set SI cache policy */
	enetc_write(priv, ENETC_SICAR0,
		    ENETC_SICAR_RD_CFG | ENETC_SICAR_WR_CFG);
	/* enable SI */
	enetc_write(priv, ENETC_SIMR, ENETC_SIMR_EN);
}

/* returns DMA address for a given buffer index */
static inline u64 enetc_rxb_address(struct udevice *dev, int i)
{
	return cpu_to_le64(dm_pci_virt_to_mem(dev, net_rx_packets[i]));
}

/*
 * Setup a single Tx BD Ring (ID = 0):
 * - set Tx buffer descriptor address
 * - set the BD count
 * - initialize the producer and consumer index
 */
static void enetc_setup_tx_bdr(struct udevice *dev)
{
	struct enetc_priv *priv = dev_get_priv(dev);
	struct bd_ring *tx_bdr = &priv->tx_bdr;
	u64 tx_bd_add = (u64)priv->enetc_txbd;

	/* used later to advance to the next Tx BD */
	tx_bdr->bd_count = ENETC_BD_CNT;
	tx_bdr->next_prod_idx = 0;
	tx_bdr->next_cons_idx = 0;
	tx_bdr->cons_idx = priv->regs_base +
				ENETC_BDR(TX, ENETC_TX_BDR_ID, ENETC_TBCIR);
	tx_bdr->prod_idx = priv->regs_base +
				ENETC_BDR(TX, ENETC_TX_BDR_ID, ENETC_TBPIR);

	/* set Tx BD address */
	enetc_bdr_write(priv, TX, ENETC_TX_BDR_ID, ENETC_TBBAR0,
			lower_32_bits(tx_bd_add));
	enetc_bdr_write(priv, TX, ENETC_TX_BDR_ID, ENETC_TBBAR1,
			upper_32_bits(tx_bd_add));
	/* set Tx 8 BD count */
	enetc_bdr_write(priv, TX, ENETC_TX_BDR_ID, ENETC_TBLENR,
			tx_bdr->bd_count);

	/* reset both producer/consumer indexes */
	enetc_write_reg(tx_bdr->cons_idx, tx_bdr->next_cons_idx);
	enetc_write_reg(tx_bdr->prod_idx, tx_bdr->next_prod_idx);

	/* enable TX ring */
	enetc_bdr_write(priv, TX, ENETC_TX_BDR_ID, ENETC_TBMR, ENETC_TBMR_EN);
}

/*
 * Setup a single Rx BD Ring (ID = 0):
 * - set Rx buffer descriptors address (one descriptor per buffer)
 * - set buffer size as max frame size
 * - enable Rx ring
 * - reset consumer and producer indexes
 * - set buffer for each descriptor
 */
static void enetc_setup_rx_bdr(struct udevice *dev)
{
	struct enetc_priv *priv = dev_get_priv(dev);
	struct bd_ring *rx_bdr = &priv->rx_bdr;
	u64 rx_bd_add = (u64)priv->enetc_rxbd;
	int i;

	/* used later to advance to the next BD produced by ENETC HW */
	rx_bdr->bd_count = ENETC_BD_CNT;
	rx_bdr->next_prod_idx = 0;
	rx_bdr->next_cons_idx = 0;
	rx_bdr->cons_idx = priv->regs_base +
				ENETC_BDR(RX, ENETC_RX_BDR_ID, ENETC_RBCIR);
	rx_bdr->prod_idx = priv->regs_base +
				ENETC_BDR(RX, ENETC_RX_BDR_ID, ENETC_RBPIR);

	/* set Rx BD address */
	enetc_bdr_write(priv, RX, ENETC_RX_BDR_ID, ENETC_RBBAR0,
			lower_32_bits(rx_bd_add));
	enetc_bdr_write(priv, RX, ENETC_RX_BDR_ID, ENETC_RBBAR1,
			upper_32_bits(rx_bd_add));
	/* set Rx BD count (multiple of 8) */
	enetc_bdr_write(priv, RX, ENETC_RX_BDR_ID, ENETC_RBLENR,
			rx_bdr->bd_count);
	/* set Rx buffer  size */
	enetc_bdr_write(priv, RX, ENETC_RX_BDR_ID, ENETC_RBBSR, PKTSIZE_ALIGN);

	/* fill Rx BD */
	memset(priv->enetc_rxbd, 0,
	       rx_bdr->bd_count * sizeof(union enetc_rx_bd));
	for (i = 0; i < rx_bdr->bd_count; i++) {
		priv->enetc_rxbd[i].w.addr = enetc_rxb_address(dev, i);
		/* each RX buffer must be aligned to 64B */
		WARN_ON(priv->enetc_rxbd[i].w.addr & (ARCH_DMA_MINALIGN - 1));
	}

	/* reset producer (ENETC owned) and consumer (SW owned) index */
	enetc_write_reg(rx_bdr->cons_idx, rx_bdr->next_cons_idx);
	enetc_write_reg(rx_bdr->prod_idx, rx_bdr->next_prod_idx);

	/* enable Rx ring */
	enetc_bdr_write(priv, RX, ENETC_RX_BDR_ID, ENETC_RBMR, ENETC_RBMR_EN);
}

/*
 * Start ENETC interface:
 * - perform FLR
 * - enable access to port and SI registers
 * - set mac address
 * - setup TX/RX buffer descriptors
 * - enable Tx/Rx rings
 */
static int enetc_start(struct udevice *dev)
{
	struct enetc_priv *priv = dev_get_priv(dev);

	/* reset and enable the PCI device */
	dm_pci_flr(dev);
	dm_pci_clrset_config16(dev, PCI_COMMAND, 0,
			       PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER);

	enetc_enable_si_port(priv);

	/* setup Tx/Rx buffer descriptors */
	enetc_setup_tx_bdr(dev);
	enetc_setup_rx_bdr(dev);

	enetc_setup_mac_iface(dev, priv->phy);

	return phy_startup(priv->phy);
}

/*
 * Stop the network interface:
 * - just quiesce it, we can wipe all configuration as _start starts from
 * scratch each time
 */
static void enetc_stop(struct udevice *dev)
{
	/* FLR is sufficient to quiesce the device */
	dm_pci_flr(dev);
	/* leave the BARs accessible after we stop, this is needed to use
	 * internal MDIO in command line.
	 */
	dm_pci_clrset_config16(dev, PCI_COMMAND, 0, PCI_COMMAND_MEMORY);
}

/*
 * ENETC transmit packet:
 * - check if Tx BD ring is full
 * - set buffer/packet address (dma address)
 * - set final fragment flag
 * - try while producer index equals consumer index or timeout
 */
static int enetc_send(struct udevice *dev, void *packet, int length)
{
	struct enetc_priv *priv = dev_get_priv(dev);
	struct bd_ring *txr = &priv->tx_bdr;
	void *nv_packet = (void *)packet;
	int tries = ENETC_POLL_TRIES;
	u32 pi, ci;

	pi = txr->next_prod_idx;
	ci = enetc_read_reg(txr->cons_idx) & ENETC_BDR_IDX_MASK;
	/* Tx ring is full when */
	if (((pi + 1) % txr->bd_count) == ci) {
		enetc_dbg(dev, "Tx BDR full\n");
		return -ETIMEDOUT;
	}
	enetc_dbg(dev, "TxBD[%d]send: pkt_len=%d, buff @0x%x%08x\n", pi, length,
		  upper_32_bits((u64)nv_packet), lower_32_bits((u64)nv_packet));

	/* prepare Tx BD */
	memset(&priv->enetc_txbd[pi], 0x0, sizeof(struct enetc_tx_bd));
	priv->enetc_txbd[pi].addr =
		cpu_to_le64(dm_pci_virt_to_mem(dev, nv_packet));
	priv->enetc_txbd[pi].buf_len = cpu_to_le16(length);
	priv->enetc_txbd[pi].frm_len = cpu_to_le16(length);
	priv->enetc_txbd[pi].flags = cpu_to_le16(ENETC_TXBD_FLAGS_F);
	dmb();
	/* send frame: increment producer index */
	pi = (pi + 1) % txr->bd_count;
	txr->next_prod_idx = pi;
	enetc_write_reg(txr->prod_idx, pi);
	while ((--tries >= 0) &&
	       (pi != (enetc_read_reg(txr->cons_idx) & ENETC_BDR_IDX_MASK)))
		udelay(10);

	return tries > 0 ? 0 : -ETIMEDOUT;
}

/*
 * Receive frame:
 * - wait for the next BD to get ready bit set
 * - clean up the descriptor
 * - move on and indicate to HW that the cleaned BD is available for Rx
 */
static int enetc_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct enetc_priv *priv = dev_get_priv(dev);
	struct bd_ring *rxr = &priv->rx_bdr;
	int tries = ENETC_POLL_TRIES;
	int pi = rxr->next_prod_idx;
	int ci = rxr->next_cons_idx;
	u32 status;
	int len;
	u8 rdy;

	do {
		dmb();
		status = le32_to_cpu(priv->enetc_rxbd[pi].r.lstatus);
		/* check if current BD is ready to be consumed */
		rdy = ENETC_RXBD_STATUS_R(status);
	} while (--tries >= 0 && !rdy);

	if (!rdy)
		return -EAGAIN;

	dmb();
	len = le16_to_cpu(priv->enetc_rxbd[pi].r.buf_len);
	*packetp = (uchar *)enetc_rxb_address(dev, pi);
	enetc_dbg(dev, "RxBD[%d]: len=%d err=%d pkt=0x%x%08x\n", pi, len,
		  ENETC_RXBD_STATUS_ERRORS(status),
		  upper_32_bits((u64)*packetp), lower_32_bits((u64)*packetp));

	/* BD clean up and advance to next in ring */
	memset(&priv->enetc_rxbd[pi], 0, sizeof(union enetc_rx_bd));
	priv->enetc_rxbd[pi].w.addr = enetc_rxb_address(dev, pi);
	rxr->next_prod_idx = (pi + 1) % rxr->bd_count;
	ci = (ci + 1) % rxr->bd_count;
	rxr->next_cons_idx = ci;
	dmb();
	/* free up the slot in the ring for HW */
	enetc_write_reg(rxr->cons_idx, ci);

	return len;
}

static const struct eth_ops enetc_ops = {
	.start	= enetc_start,
	.send	= enetc_send,
	.recv	= enetc_recv,
	.stop	= enetc_stop,
	.write_hwaddr = enetc_write_hwaddr,
};

U_BOOT_DRIVER(eth_enetc) = {
	.name	= ENETC_DRIVER_NAME,
	.id	= UCLASS_ETH,
	.bind	= enetc_bind,
	.probe	= enetc_probe,
	.remove = enetc_remove,
	.ops	= &enetc_ops,
	.priv_auto	= sizeof(struct enetc_priv),
	.plat_auto	= sizeof(struct eth_pdata),
};

static struct pci_device_id enetc_ids[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_FREESCALE, PCI_DEVICE_ID_ENETC_ETH) },
	{}
};

U_BOOT_PCI_DEVICE(eth_enetc, enetc_ids);
