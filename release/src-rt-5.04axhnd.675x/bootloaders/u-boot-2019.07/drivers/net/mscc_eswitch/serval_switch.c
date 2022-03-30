// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Microsemi Corporation
 */

#include <common.h>
#include <config.h>
#include <dm.h>
#include <dm/of_access.h>
#include <dm/of_addr.h>
#include <fdt_support.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <miiphy.h>
#include <net.h>
#include <wait_bit.h>

#include "mscc_xfer.h"
#include "mscc_mac_table.h"

#define GCB_MIIM_MII_STATUS			0x0
#define		GCB_MIIM_STAT_BUSY			BIT(3)
#define GCB_MIIM_MII_CMD			0x8
#define		GCB_MIIM_MII_CMD_OPR_WRITE		BIT(1)
#define		GCB_MIIM_MII_CMD_OPR_READ		BIT(2)
#define		GCB_MIIM_MII_CMD_WRDATA(x)		((x) << 4)
#define		GCB_MIIM_MII_CMD_REGAD(x)		((x) << 20)
#define		GCB_MIIM_MII_CMD_PHYAD(x)		((x) << 25)
#define		GCB_MIIM_MII_CMD_VLD			BIT(31)
#define GCB_MIIM_DATA				0xC
#define		GCB_MIIM_DATA_ERROR			(0x2 << 16)

#define ANA_PORT_VLAN_CFG(x)			(0xc000 + 0x100 * (x))
#define		ANA_PORT_VLAN_CFG_AWARE_ENA		BIT(20)
#define		ANA_PORT_VLAN_CFG_POP_CNT(x)		((x) << 18)
#define ANA_PORT_PORT_CFG(x)			(0xc070 + 0x100 * (x))
#define		ANA_PORT_PORT_CFG_RECV_ENA		BIT(6)
#define ANA_PGID(x)				(0x9c00 + 4 * (x))

#define HSIO_ANA_SERDES1G_DES_CFG		0x3c
#define		HSIO_ANA_SERDES1G_DES_CFG_BW_HYST(x)		((x) << 1)
#define		HSIO_ANA_SERDES1G_DES_CFG_BW_ANA(x)		((x) << 5)
#define		HSIO_ANA_SERDES1G_DES_CFG_MBTR_CTRL(x)		((x) << 8)
#define		HSIO_ANA_SERDES1G_DES_CFG_PHS_CTRL(x)		((x) << 13)
#define HSIO_ANA_SERDES1G_IB_CFG		0x40
#define		HSIO_ANA_SERDES1G_IB_CFG_RESISTOR_CTRL(x)	(x)
#define		HSIO_ANA_SERDES1G_IB_CFG_EQ_GAIN(x)		((x) << 6)
#define		HSIO_ANA_SERDES1G_IB_CFG_ENA_OFFSET_COMP	BIT(9)
#define		HSIO_ANA_SERDES1G_IB_CFG_ENA_DETLEV		BIT(11)
#define		HSIO_ANA_SERDES1G_IB_CFG_ENA_CMV_TERM		BIT(13)
#define		HSIO_ANA_SERDES1G_IB_CFG_DET_LEV(x)		((x) << 19)
#define		HSIO_ANA_SERDES1G_IB_CFG_ACJTAG_HYST(x)		((x) << 24)
#define HSIO_ANA_SERDES1G_OB_CFG		0x44
#define		HSIO_ANA_SERDES1G_OB_CFG_RESISTOR_CTRL(x)	(x)
#define		HSIO_ANA_SERDES1G_OB_CFG_VCM_CTRL(x)		((x) << 4)
#define		HSIO_ANA_SERDES1G_OB_CFG_CMM_BIAS_CTRL(x)	((x) << 10)
#define		HSIO_ANA_SERDES1G_OB_CFG_AMP_CTRL(x)		((x) << 13)
#define		HSIO_ANA_SERDES1G_OB_CFG_SLP(x)			((x) << 17)
#define HSIO_ANA_SERDES1G_SER_CFG		0x48
#define HSIO_ANA_SERDES1G_COMMON_CFG		0x4c
#define		HSIO_ANA_SERDES1G_COMMON_CFG_IF_MODE		BIT(0)
#define		HSIO_ANA_SERDES1G_COMMON_CFG_ENA_LANE		BIT(18)
#define		HSIO_ANA_SERDES1G_COMMON_CFG_SYS_RST		BIT(31)
#define HSIO_ANA_SERDES1G_PLL_CFG		0x50
#define		HSIO_ANA_SERDES1G_PLL_CFG_FSM_ENA		BIT(7)
#define		HSIO_ANA_SERDES1G_PLL_CFG_FSM_CTRL_DATA(x)	((x) << 8)
#define		HSIO_ANA_SERDES1G_PLL_CFG_ENA_RC_DIV2		BIT(21)
#define HSIO_DIG_SERDES1G_DFT_CFG0		0x58
#define HSIO_DIG_SERDES1G_MISC_CFG		0x6c
#define		HSIO_DIG_SERDES1G_MISC_CFG_LANE_RST		BIT(0)
#define HSIO_MCB_SERDES1G_CFG			0x74
#define		HSIO_MCB_SERDES1G_CFG_WR_ONE_SHOT	BIT(31)
#define		HSIO_MCB_SERDES1G_CFG_ADDR(x)		(x)

#define SYS_FRM_AGING				0x584
#define		SYS_FRM_AGING_ENA			BIT(20)
#define SYS_SYSTEM_RST_CFG			0x518
#define		SYS_SYSTEM_RST_MEM_INIT			BIT(5)
#define		SYS_SYSTEM_RST_MEM_ENA			BIT(6)
#define		SYS_SYSTEM_RST_CORE_ENA			BIT(7)
#define SYS_PORT_MODE(x)			(0x524 + 0x4 * (x))
#define		SYS_PORT_MODE_INCL_INJ_HDR(x)		((x) << 4)
#define		SYS_PORT_MODE_INCL_XTR_HDR(x)		((x) << 2)
#define SYS_PAUSE_CFG(x)			(0x65c + 0x4 * (x))
#define		SYS_PAUSE_CFG_PAUSE_ENA			BIT(0)

#define QSYS_SWITCH_PORT_MODE(x)		(0x15a34 + 0x4 * (x))
#define		QSYS_SWITCH_PORT_MODE_PORT_ENA		BIT(13)
#define QSYS_EGR_NO_SHARING			0x15a9c
#define QSYS_QMAP				0x15adc

/* Port registers */
#define DEV_CLOCK_CFG				0x0
#define DEV_CLOCK_CFG_LINK_SPEED_1000			1
#define DEV_MAC_ENA_CFG				0x10
#define		DEV_MAC_ENA_CFG_RX_ENA			BIT(4)
#define		DEV_MAC_ENA_CFG_TX_ENA			BIT(0)
#define DEV_MAC_IFG_CFG				0x24
#define		DEV_MAC_IFG_CFG_TX_IFG(x)		((x) << 8)
#define		DEV_MAC_IFG_CFG_RX_IFG2(x)		((x) << 4)
#define		DEV_MAC_IFG_CFG_RX_IFG1(x)		(x)
#define PCS1G_CFG				0x3c
#define		PCS1G_MODE_CFG_SGMII_MODE_ENA		BIT(0)
#define PCS1G_MODE_CFG				0x40
#define PCS1G_SD_CFG				0x44
#define PCS1G_ANEG_CFG				0x48
#define		PCS1G_ANEG_CFG_ADV_ABILITY(x)		((x) << 16)

#define QS_XTR_GRP_CFG(x)			(4 * (x))
#define		QS_XTR_GRP_CFG_MODE(x)			((x) << 2)
#define		QS_XTR_GRP_CFG_BYTE_SWAP		BIT(0)
#define QS_INJ_GRP_CFG(x)			(0x24 + (x) * 4)
#define		QS_INJ_GRP_CFG_MODE(x)			((x) << 2)
#define		QS_INJ_GRP_CFG_BYTE_SWAP		BIT(0)

#define IFH_INJ_BYPASS		BIT(31)
#define IFH_TAG_TYPE_C		0
#define MAC_VID			1
#define CPU_PORT		11
#define INTERNAL_PORT_MSK	0xFF
#define IFH_LEN			4
#define ETH_ALEN		6
#define PGID_BROADCAST		13
#define PGID_UNICAST		14

static const char *const regs_names[] = {
	"port0", "port1", "port2", "port3", "port4", "port5", "port6",
	"port7", "port8", "port9", "port10",
	"ana", "qs", "qsys", "rew", "sys", "hsio",
};

#define REGS_NAMES_COUNT ARRAY_SIZE(regs_names) + 1
#define MAX_PORT 11

enum serval_ctrl_regs {
	ANA = MAX_PORT,
	QS,
	QSYS,
	REW,
	SYS,
	HSIO,
};

#define SERVAL_MIIM_BUS_COUNT 2

struct serval_phy_port_t {
	size_t phy_addr;
	struct mii_dev *bus;
	u8 serdes_index;
	u8 phy_mode;
};

struct serval_private {
	void __iomem *regs[REGS_NAMES_COUNT];
	struct mii_dev *bus[SERVAL_MIIM_BUS_COUNT];
	struct serval_phy_port_t ports[MAX_PORT];
};

struct mscc_miim_dev {
	void __iomem *regs;
	phys_addr_t miim_base;
	unsigned long miim_size;
	struct mii_dev *bus;
};

static const unsigned long serval_regs_qs[] = {
	[MSCC_QS_XTR_RD] = 0x8,
	[MSCC_QS_XTR_FLUSH] = 0x18,
	[MSCC_QS_XTR_DATA_PRESENT] = 0x1c,
	[MSCC_QS_INJ_WR] = 0x2c,
	[MSCC_QS_INJ_CTRL] = 0x34,
};

static const unsigned long serval_regs_ana_table[] = {
	[MSCC_ANA_TABLES_MACHDATA] = 0x9b34,
	[MSCC_ANA_TABLES_MACLDATA] = 0x9b38,
	[MSCC_ANA_TABLES_MACACCESS] = 0x9b3c,
};

static struct mscc_miim_dev miim[SERVAL_MIIM_BUS_COUNT];
static int miim_count = -1;

static int mscc_miim_wait_ready(struct mscc_miim_dev *miim)
{
	return wait_for_bit_le32(miim->regs + GCB_MIIM_MII_STATUS,
				 GCB_MIIM_STAT_BUSY, false, 250, false);
}

static int mscc_miim_read(struct mii_dev *bus, int addr, int devad, int reg)
{
	struct mscc_miim_dev *miim = (struct mscc_miim_dev *)bus->priv;
	u32 val;
	int ret;

	ret = mscc_miim_wait_ready(miim);
	if (ret)
		goto out;

	writel(GCB_MIIM_MII_CMD_VLD | GCB_MIIM_MII_CMD_PHYAD(addr) |
	       GCB_MIIM_MII_CMD_REGAD(reg) | GCB_MIIM_MII_CMD_OPR_READ,
	       miim->regs + GCB_MIIM_MII_CMD);

	ret = mscc_miim_wait_ready(miim);
	if (ret)
		goto out;

	val = readl(miim->regs + GCB_MIIM_DATA);
	if (val & GCB_MIIM_DATA_ERROR) {
		ret = -EIO;
		goto out;
	}

	ret = val & 0xFFFF;
 out:
	return ret;
}

static int mscc_miim_write(struct mii_dev *bus, int addr, int devad, int reg,
			   u16 val)
{
	struct mscc_miim_dev *miim = (struct mscc_miim_dev *)bus->priv;
	int ret;

	ret = mscc_miim_wait_ready(miim);
	if (ret < 0)
		goto out;

	writel(GCB_MIIM_MII_CMD_VLD | GCB_MIIM_MII_CMD_PHYAD(addr) |
	       GCB_MIIM_MII_CMD_REGAD(reg) | GCB_MIIM_MII_CMD_WRDATA(val) |
	       GCB_MIIM_MII_CMD_OPR_WRITE, miim->regs + GCB_MIIM_MII_CMD);
 out:
	return ret;
}

static struct mii_dev *serval_mdiobus_init(phys_addr_t miim_base,
					   unsigned long miim_size)
{
	struct mii_dev *bus;

	bus = mdio_alloc();
	if (!bus)
		return NULL;

	++miim_count;
	sprintf(bus->name, "miim-bus%d", miim_count);

	miim[miim_count].regs = ioremap(miim_base, miim_size);
	miim[miim_count].miim_base = miim_base;
	miim[miim_count].miim_size = miim_size;
	bus->priv = &miim[miim_count];
	bus->read = mscc_miim_read;
	bus->write = mscc_miim_write;

	if (mdio_register(bus))
		return NULL;

	miim[miim_count].bus = bus;
	return bus;
}

static void serval_cpu_capture_setup(struct serval_private *priv)
{
	int i;

	/* map the 8 CPU extraction queues to CPU port 11 */
	writel(0, priv->regs[QSYS] + QSYS_QMAP);

	for (i = 0; i <= 1; i++) {
		/*
		 * Do byte-swap and expect status after last data word
		 * Extraction: Mode: manual extraction) | Byte_swap
		 */
		writel(QS_XTR_GRP_CFG_MODE(1) | QS_XTR_GRP_CFG_BYTE_SWAP,
		       priv->regs[QS] + QS_XTR_GRP_CFG(i));
		/*
		 * Injection: Mode: manual extraction | Byte_swap
		 */
		writel(QS_INJ_GRP_CFG_MODE(1) | QS_INJ_GRP_CFG_BYTE_SWAP,
		       priv->regs[QS] + QS_INJ_GRP_CFG(i));
	}

	for (i = 0; i <= 1; i++)
		/* Enable IFH insertion/parsing on CPU ports */
		writel(SYS_PORT_MODE_INCL_INJ_HDR(1) |
		       SYS_PORT_MODE_INCL_XTR_HDR(1),
		       priv->regs[SYS] + SYS_PORT_MODE(CPU_PORT + i));
	/*
	 * Setup the CPU port as VLAN aware to support switching frames
	 * based on tags
	 */
	writel(ANA_PORT_VLAN_CFG_AWARE_ENA | ANA_PORT_VLAN_CFG_POP_CNT(1) |
	       MAC_VID, priv->regs[ANA] + ANA_PORT_VLAN_CFG(CPU_PORT));

	/* Disable learning (only RECV_ENA must be set) */
	writel(ANA_PORT_PORT_CFG_RECV_ENA,
	       priv->regs[ANA] + ANA_PORT_PORT_CFG(CPU_PORT));

	/* Enable switching to/from cpu port */
	setbits_le32(priv->regs[QSYS] + QSYS_SWITCH_PORT_MODE(CPU_PORT),
		     QSYS_SWITCH_PORT_MODE_PORT_ENA);

	/* No pause on CPU port - not needed (off by default) */
	clrbits_le32(priv->regs[SYS] + SYS_PAUSE_CFG(CPU_PORT),
		     SYS_PAUSE_CFG_PAUSE_ENA);

	setbits_le32(priv->regs[QSYS] + QSYS_EGR_NO_SHARING, BIT(CPU_PORT));
}

static void serval_port_init(struct serval_private *priv, int port)
{
	void __iomem *regs = priv->regs[port];

	/* Enable PCS */
	writel(PCS1G_MODE_CFG_SGMII_MODE_ENA, regs + PCS1G_CFG);

	/* Disable Signal Detect */
	writel(0, regs + PCS1G_SD_CFG);

	/* Enable MAC RX and TX */
	writel(DEV_MAC_ENA_CFG_RX_ENA | DEV_MAC_ENA_CFG_TX_ENA,
	       regs + DEV_MAC_ENA_CFG);

	/* Clear sgmii_mode_ena */
	writel(0, regs + PCS1G_MODE_CFG);

	/*
	 * Clear sw_resolve_ena(bit 0) and set adv_ability to
	 * something meaningful just in case
	 */
	writel(PCS1G_ANEG_CFG_ADV_ABILITY(0x20), regs + PCS1G_ANEG_CFG);

	/* Set MAC IFG Gaps */
	writel(DEV_MAC_IFG_CFG_TX_IFG(5) | DEV_MAC_IFG_CFG_RX_IFG1(5) |
	       DEV_MAC_IFG_CFG_RX_IFG2(1), regs + DEV_MAC_IFG_CFG);

	/* Set link speed and release all resets */
	writel(DEV_CLOCK_CFG_LINK_SPEED_1000, regs + DEV_CLOCK_CFG);

	/* Make VLAN aware for CPU traffic */
	writel(ANA_PORT_VLAN_CFG_AWARE_ENA | ANA_PORT_VLAN_CFG_POP_CNT(1) |
	       MAC_VID, priv->regs[ANA] + ANA_PORT_VLAN_CFG(port));

	/* Enable the port in the core */
	setbits_le32(priv->regs[QSYS] + QSYS_SWITCH_PORT_MODE(port),
		     QSYS_SWITCH_PORT_MODE_PORT_ENA);
}

static void serdes_write(void __iomem *base, u32 addr)
{
	u32 data;

	writel(HSIO_MCB_SERDES1G_CFG_WR_ONE_SHOT |
	       HSIO_MCB_SERDES1G_CFG_ADDR(addr),
	       base + HSIO_MCB_SERDES1G_CFG);

	do {
		data = readl(base + HSIO_MCB_SERDES1G_CFG);
	} while (data & HSIO_MCB_SERDES1G_CFG_WR_ONE_SHOT);

	mdelay(100);
}

static void serdes1g_setup(void __iomem *base, uint32_t addr,
			   phy_interface_t interface)
{
	writel(0x0, base + HSIO_ANA_SERDES1G_SER_CFG);
	writel(0x0, base + HSIO_DIG_SERDES1G_DFT_CFG0);
	writel(HSIO_ANA_SERDES1G_IB_CFG_RESISTOR_CTRL(11) |
	       HSIO_ANA_SERDES1G_IB_CFG_EQ_GAIN(0) |
	       HSIO_ANA_SERDES1G_IB_CFG_ENA_OFFSET_COMP |
	       HSIO_ANA_SERDES1G_IB_CFG_ENA_CMV_TERM |
	       HSIO_ANA_SERDES1G_IB_CFG_ACJTAG_HYST(1),
	       base + HSIO_ANA_SERDES1G_IB_CFG);
	writel(HSIO_ANA_SERDES1G_DES_CFG_BW_HYST(7) |
	       HSIO_ANA_SERDES1G_DES_CFG_BW_ANA(6) |
	       HSIO_ANA_SERDES1G_DES_CFG_MBTR_CTRL(2) |
	       HSIO_ANA_SERDES1G_DES_CFG_PHS_CTRL(6),
	       base + HSIO_ANA_SERDES1G_DES_CFG);
	writel(HSIO_ANA_SERDES1G_OB_CFG_RESISTOR_CTRL(1) |
	       HSIO_ANA_SERDES1G_OB_CFG_VCM_CTRL(4) |
	       HSIO_ANA_SERDES1G_OB_CFG_CMM_BIAS_CTRL(2) |
	       HSIO_ANA_SERDES1G_OB_CFG_AMP_CTRL(12) |
	       HSIO_ANA_SERDES1G_OB_CFG_SLP(3),
	       base + HSIO_ANA_SERDES1G_OB_CFG);
	writel(HSIO_ANA_SERDES1G_COMMON_CFG_IF_MODE |
	       HSIO_ANA_SERDES1G_COMMON_CFG_ENA_LANE,
	       base + HSIO_ANA_SERDES1G_COMMON_CFG);
	writel(HSIO_ANA_SERDES1G_PLL_CFG_FSM_ENA |
	       HSIO_ANA_SERDES1G_PLL_CFG_FSM_CTRL_DATA(200) |
	       HSIO_ANA_SERDES1G_PLL_CFG_ENA_RC_DIV2,
	       base + HSIO_ANA_SERDES1G_PLL_CFG);
	writel(HSIO_DIG_SERDES1G_MISC_CFG_LANE_RST,
	       base + HSIO_DIG_SERDES1G_MISC_CFG);
	serdes_write(base, addr);

	writel(HSIO_ANA_SERDES1G_COMMON_CFG_IF_MODE |
	       HSIO_ANA_SERDES1G_COMMON_CFG_ENA_LANE |
	       HSIO_ANA_SERDES1G_COMMON_CFG_SYS_RST,
	       base + HSIO_ANA_SERDES1G_COMMON_CFG);
	serdes_write(base, addr);

	writel(0x0, base + HSIO_DIG_SERDES1G_MISC_CFG);
	serdes_write(base, addr);
}

static void serdes_setup(struct serval_private *priv)
{
	size_t mask;
	int i = 0;

	for (i = 0; i < MAX_PORT; ++i) {
		if (!priv->ports[i].bus)
			continue;

		mask = BIT(priv->ports[i].serdes_index);
		serdes1g_setup(priv->regs[HSIO], mask,
			       priv->ports[i].phy_mode);
	}
}

static int serval_switch_init(struct serval_private *priv)
{
	/* Reset switch & memories */
	writel(SYS_SYSTEM_RST_MEM_ENA | SYS_SYSTEM_RST_MEM_INIT,
	       priv->regs[SYS] + SYS_SYSTEM_RST_CFG);

	if (wait_for_bit_le32(priv->regs[SYS] + SYS_SYSTEM_RST_CFG,
			      SYS_SYSTEM_RST_MEM_INIT, false, 2000, false)) {
		pr_err("Timeout in memory reset\n");
		return -EIO;
	}

	/* Enable switch core */
	setbits_le32(priv->regs[SYS] + SYS_SYSTEM_RST_CFG,
		     SYS_SYSTEM_RST_CORE_ENA);

	serdes_setup(priv);

	return 0;
}

static int serval_initialize(struct serval_private *priv)
{
	int ret, i;

	/* Initialize switch memories, enable core */
	ret = serval_switch_init(priv);
	if (ret)
		return ret;

	/* Flush queues */
	mscc_flush(priv->regs[QS], serval_regs_qs);

	/* Setup frame ageing - "2 sec" - The unit is 6.5us on serval */
	writel(SYS_FRM_AGING_ENA | (20000000 / 65),
	       priv->regs[SYS] + SYS_FRM_AGING);

	for (i = 0; i < MAX_PORT; i++)
		serval_port_init(priv, i);

	serval_cpu_capture_setup(priv);

	debug("Ports enabled\n");

	return 0;
}

static int serval_write_hwaddr(struct udevice *dev)
{
	struct serval_private *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_platdata(dev);

	mscc_mac_table_add(priv->regs[ANA], serval_regs_ana_table,
			   pdata->enetaddr, PGID_UNICAST);

	writel(BIT(CPU_PORT), priv->regs[ANA] + ANA_PGID(PGID_UNICAST));

	return 0;
}

static int serval_start(struct udevice *dev)
{
	struct serval_private *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_platdata(dev);
	const unsigned char mac[ETH_ALEN] = { 0xff, 0xff, 0xff, 0xff, 0xff,
					      0xff };
	int ret;

	ret = serval_initialize(priv);
	if (ret)
		return ret;

	/* Set MAC address tables entries for CPU redirection */
	mscc_mac_table_add(priv->regs[ANA], serval_regs_ana_table, mac,
			   PGID_BROADCAST);

	writel(BIT(CPU_PORT) | INTERNAL_PORT_MSK,
	       priv->regs[ANA] + ANA_PGID(PGID_BROADCAST));

	/* It should be setup latter in serval_write_hwaddr */
	mscc_mac_table_add(priv->regs[ANA], serval_regs_ana_table,
			   pdata->enetaddr, PGID_UNICAST);

	writel(BIT(CPU_PORT), priv->regs[ANA] + ANA_PGID(PGID_UNICAST));
	return 0;
}

static void serval_stop(struct udevice *dev)
{
	writel(ICPU_RESET_CORE_RST_PROTECT, BASE_CFG + ICPU_RESET);
	writel(PERF_SOFT_RST_SOFT_CHIP_RST, BASE_DEVCPU_GCB + PERF_SOFT_RST);
}

static int serval_send(struct udevice *dev, void *packet, int length)
{
	struct serval_private *priv = dev_get_priv(dev);
	u32 ifh[IFH_LEN];
	u32 *buf = packet;

	/*
	 * Generate the IFH for frame injection
	 *
	 * The IFH is a 128bit-value
	 * bit 127: bypass the analyzer processing
	 * bit 57-67: destination mask
	 * bit 28-29: pop_cnt: 3 disables all rewriting of the frame
	 * bit 20-27: cpu extraction queue mask
	 * bit 16: tag type 0: C-tag, 1: S-tag
	 * bit 0-11: VID
	 */
	ifh[0] = IFH_INJ_BYPASS;
	ifh[1] = (0x07);
	ifh[2] = (0x7f) << 25;
	ifh[3] = (IFH_TAG_TYPE_C << 16);

	return mscc_send(priv->regs[QS], serval_regs_qs,
			 ifh, IFH_LEN, buf, length);
}

static int serval_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct serval_private *priv = dev_get_priv(dev);
	u32 *rxbuf = (u32 *)net_rx_packets[0];
	int byte_cnt = 0;

	byte_cnt = mscc_recv(priv->regs[QS], serval_regs_qs, rxbuf, IFH_LEN,
			     false);

	*packetp = net_rx_packets[0];

	return byte_cnt;
}

static struct mii_dev *get_mdiobus(phys_addr_t base, unsigned long size)
{
	int i = 0;

	for (i = 0; i < SERVAL_MIIM_BUS_COUNT; ++i)
		if (miim[i].miim_base == base && miim[i].miim_size == size)
			return miim[i].bus;

	return NULL;
}

static void add_port_entry(struct serval_private *priv, size_t index,
			   size_t phy_addr, struct mii_dev *bus,
			   u8 serdes_index, u8 phy_mode)
{
	priv->ports[index].phy_addr = phy_addr;
	priv->ports[index].bus = bus;
	priv->ports[index].serdes_index = serdes_index;
	priv->ports[index].phy_mode = phy_mode;
}

static int serval_probe(struct udevice *dev)
{
	struct serval_private *priv = dev_get_priv(dev);
	int i, ret;
	struct resource res;
	fdt32_t faddr;
	phys_addr_t addr_base;
	unsigned long addr_size;
	ofnode eth_node, node, mdio_node;
	size_t phy_addr;
	struct mii_dev *bus;
	struct ofnode_phandle_args phandle;
	struct phy_device *phy;

	if (!priv)
		return -EINVAL;

	/* Get registers and map them to the private structure */
	for (i = 0; i < ARRAY_SIZE(regs_names); i++) {
		priv->regs[i] = dev_remap_addr_name(dev, regs_names[i]);
		if (!priv->regs[i]) {
			debug
			    ("Error can't get regs base addresses for %s\n",
			     regs_names[i]);
			return -ENOMEM;
		}
	}

	/* Initialize miim buses */
	memset(&miim, 0x0, sizeof(miim) * SERVAL_MIIM_BUS_COUNT);

	/* iterate all the ports and find out on which bus they are */
	i = 0;
	eth_node = dev_read_first_subnode(dev);
	for (node = ofnode_first_subnode(eth_node);
	     ofnode_valid(node);
	     node = ofnode_next_subnode(node)) {
		if (ofnode_read_resource(node, 0, &res))
			return -ENOMEM;
		i = res.start;

		ret = ofnode_parse_phandle_with_args(node, "phy-handle", NULL,
						     0, 0, &phandle);
		if (ret)
			continue;

		/* Get phy address on mdio bus */
		if (ofnode_read_resource(phandle.node, 0, &res))
			return -ENOMEM;
		phy_addr = res.start;

		/* Get mdio node */
		mdio_node = ofnode_get_parent(phandle.node);

		if (ofnode_read_resource(mdio_node, 0, &res))
			return -ENOMEM;
		faddr = cpu_to_fdt32(res.start);

		addr_base = ofnode_translate_address(mdio_node, &faddr);
		addr_size = res.end - res.start;

		/* If the bus is new then create a new bus */
		if (!get_mdiobus(addr_base, addr_size))
			priv->bus[miim_count] =
				serval_mdiobus_init(addr_base, addr_size);

		/* Connect mdio bus with the port */
		bus = get_mdiobus(addr_base, addr_size);

		/* Get serdes info */
		ret = ofnode_parse_phandle_with_args(node, "phys", NULL,
						     3, 0, &phandle);
		if (ret)
			return -ENOMEM;

		add_port_entry(priv, i, phy_addr, bus, phandle.args[1],
			       phandle.args[2]);
	}

	for (i = 0; i < MAX_PORT; i++) {
		if (!priv->ports[i].bus)
			continue;

		phy = phy_connect(priv->ports[i].bus,
				  priv->ports[i].phy_addr, dev,
				  PHY_INTERFACE_MODE_NONE);
		if (phy)
			board_phy_config(phy);
	}

	return 0;
}

static int serval_remove(struct udevice *dev)
{
	struct serval_private *priv = dev_get_priv(dev);
	int i;

	for (i = 0; i < SERVAL_MIIM_BUS_COUNT; i++) {
		mdio_unregister(priv->bus[i]);
		mdio_free(priv->bus[i]);
	}

	return 0;
}

static const struct eth_ops serval_ops = {
	.start        = serval_start,
	.stop         = serval_stop,
	.send         = serval_send,
	.recv         = serval_recv,
	.write_hwaddr = serval_write_hwaddr,
};

static const struct udevice_id mscc_serval_ids[] = {
	{.compatible = "mscc,vsc7418-switch"},
	{ /* Sentinel */ }
};

U_BOOT_DRIVER(serval) = {
	.name				= "serval-switch",
	.id				= UCLASS_ETH,
	.of_match			= mscc_serval_ids,
	.probe				= serval_probe,
	.remove				= serval_remove,
	.ops				= &serval_ops,
	.priv_auto_alloc_size		= sizeof(struct serval_private),
	.platdata_auto_alloc_size	= sizeof(struct eth_pdata),
};
