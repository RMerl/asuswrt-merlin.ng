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

#define ANA_PORT_VLAN_CFG(x)		(0x00 + 0x80 * (x))
#define		ANA_PORT_VLAN_CFG_AWARE_ENA	BIT(20)
#define		ANA_PORT_VLAN_CFG_POP_CNT(x)	((x) << 18)
#define ANA_PORT_CPU_FWD_CFG(x)		(0x50 + 0x80 * (x))
#define		ANA_PORT_CPU_FWD_CFG_SRC_COPY_ENA	BIT(1)
#define ANA_PORT_PORT_CFG(x)		(0x60 + 0x80 * (x))
#define		ANA_PORT_PORT_CFG_RECV_ENA	BIT(5)
#define ANA_PGID(x)			(0x1000 + 4 * (x))

#define SYS_FRM_AGING			0x8300

#define SYS_SYSTEM_RST_CFG		0x81b0
#define		SYS_SYSTEM_RST_MEM_INIT		BIT(0)
#define		SYS_SYSTEM_RST_MEM_ENA		BIT(1)
#define		SYS_SYSTEM_RST_CORE_ENA		BIT(2)
#define SYS_PORT_MODE(x)		(0x81bc + 0x4 * (x))
#define		SYS_PORT_MODE_INCL_INJ_HDR	BIT(0)
#define SYS_SWITCH_PORT_MODE(x)		(0x8294 + 0x4 * (x))
#define		SYS_SWITCH_PORT_MODE_PORT_ENA	BIT(3)
#define SYS_EGR_NO_SHARING		0x8378
#define SYS_SCH_CPU			0x85a0

#define REW_PORT_CFG(x)			(0x8 + 0x80 * (x))
#define		REW_PORT_CFG_IFH_INSERT_ENA	BIT(7)

#define GCB_DEVCPU_RST_SOFT_CHIP_RST	0x90
#define		GCB_DEVCPU_RST_SOFT_CHIP_RST_SOFT_PHY	BIT(1)
#define GCB_MISC_STAT			0x11c
#define		GCB_MISC_STAT_PHY_READY			BIT(3)

#define	QS_XTR_MAP(x)			(0x10 + 4 * (x))
#define		QS_XTR_MAP_GRP			BIT(4)
#define		QS_XTR_MAP_ENA			BIT(0)

#define HSIO_PLL5G_CFG_PLL5G_CFG2	0x8

#define HSIO_RCOMP_CFG_CFG0		0x20
#define		HSIO_RCOMP_CFG_CFG0_MODE_SEL(x)			((x) << 8)
#define		HSIO_RCOMP_CFG_CFG0_RUN_CAL			BIT(12)
#define HSIO_RCOMP_STATUS		0x24
#define		HSIO_RCOMP_STATUS_BUSY				BIT(12)
#define		HSIO_RCOMP_STATUS_RCOMP_M			GENMASK(3, 0)
#define HSIO_SERDES6G_ANA_CFG_DES_CFG	0x64
#define		HSIO_SERDES6G_ANA_CFG_DES_CFG_BW_ANA(x)		((x) << 1)
#define		HSIO_SERDES6G_ANA_CFG_DES_CFG_BW_HYST(x)	((x) << 5)
#define		HSIO_SERDES6G_ANA_CFG_DES_CFG_MBTR_CTRL(x)	((x) << 10)
#define		HSIO_SERDES6G_ANA_CFG_DES_CFG_PHS_CTRL(x)	((x) << 13)
#define HSIO_SERDES6G_ANA_CFG_IB_CFG	0x68
#define		HSIO_SERDES6G_ANA_CFG_IB_CFG_RESISTOR_CTRL(x)	(x)
#define		HSIO_SERDES6G_ANA_CFG_IB_CFG_VBCOM(x)		((x) << 4)
#define		HSIO_SERDES6G_ANA_CFG_IB_CFG_VBAC(x)		((x) << 7)
#define		HSIO_SERDES6G_ANA_CFG_IB_CFG_RT(x)		((x) << 9)
#define		HSIO_SERDES6G_ANA_CFG_IB_CFG_RF(x)		((x) << 14)
#define HSIO_SERDES6G_ANA_CFG_IB_CFG1	0x6c
#define		HSIO_SERDES6G_ANA_CFG_IB_CFG1_RST		BIT(0)
#define		HSIO_SERDES6G_ANA_CFG_IB_CFG1_ENA_OFFSDC	BIT(2)
#define		HSIO_SERDES6G_ANA_CFG_IB_CFG1_ENA_OFFSAC	BIT(3)
#define		HSIO_SERDES6G_ANA_CFG_IB_CFG1_ANEG_MODE	BIT(6)
#define		HSIO_SERDES6G_ANA_CFG_IB_CFG1_CHF		BIT(7)
#define		HSIO_SERDES6G_ANA_CFG_IB_CFG1_C(x)		((x) << 8)
#define HSIO_SERDES6G_ANA_CFG_OB_CFG	0x70
#define		HSIO_SERDES6G_ANA_CFG_OB_CFG_SR(x)		((x) << 4)
#define		HSIO_SERDES6G_ANA_CFG_OB_CFG_SR_H		BIT(8)
#define		HSIO_SERDES6G_ANA_CFG_OB_CFG_POST0(x)		((x) << 23)
#define		HSIO_SERDES6G_ANA_CFG_OB_CFG_POL		BIT(29)
#define		HSIO_SERDES6G_ANA_CFG_OB_CFG_ENA1V_MODE	BIT(30)
#define HSIO_SERDES6G_ANA_CFG_OB_CFG1	0x74
#define		HSIO_SERDES6G_ANA_CFG_OB_CFG1_LEV(x)		(x)
#define		HSIO_SERDES6G_ANA_CFG_OB_CFG1_ENA_CAS(x)	((x) << 6)
#define HSIO_SERDES6G_ANA_CFG_COMMON_CFG 0x7c
#define		HSIO_SERDES6G_ANA_CFG_COMMON_CFG_IF_MODE(x)	(x)
#define		HSIO_SERDES6G_ANA_CFG_COMMON_CFG_ENA_LANE	BIT(18)
#define		HSIO_SERDES6G_ANA_CFG_COMMON_CFG_SYS_RST	BIT(31)
#define HSIO_SERDES6G_ANA_CFG_PLL_CFG	0x80
#define		HSIO_SERDES6G_ANA_CFG_PLL_CFG_FSM_ENA		BIT(7)
#define		HSIO_SERDES6G_ANA_CFG_PLL_CFG_FSM_CTRL_DATA(x)	((x) << 8)
#define HSIO_SERDES6G_ANA_CFG_SER_CFG	0x84
#define HSIO_SERDES6G_DIG_CFG_MISC_CFG	0x88
#define		HSIO_SERDES6G_DIG_CFG_MISC_CFG_LANE_RST		BIT(0)
#define HSIO_MCB_SERDES6G_CFG		0xac
#define		HSIO_MCB_SERDES6G_CFG_WR_ONE_SHOT		BIT(31)
#define		HSIO_MCB_SERDES6G_CFG_ADDR(x)			(x)

#define DEV_GMII_PORT_MODE_CLK		0x0
#define		DEV_GMII_PORT_MODE_CLK_PHY_RST	BIT(0)
#define DEV_GMII_MAC_CFG_MAC_ENA	0xc
#define		DEV_GMII_MAC_CFG_MAC_ENA_RX_ENA		BIT(4)
#define		DEV_GMII_MAC_CFG_MAC_ENA_TX_ENA		BIT(0)

#define DEV_PORT_MODE_CLK		0x4
#define		DEV_PORT_MODE_CLK_PHY_RST		BIT(2)
#define		DEV_PORT_MODE_CLK_LINK_SPEED_1000	1
#define DEV_MAC_CFG_MAC_ENA		0x10
#define		DEV_MAC_CFG_MAC_ENA_RX_ENA		BIT(4)
#define		DEV_MAC_CFG_MAC_ENA_TX_ENA		BIT(0)
#define DEV_MAC_CFG_MAC_IFG		0x24
#define		DEV_MAC_CFG_MAC_IFG_TX_IFG(x)		((x) << 8)
#define		DEV_MAC_CFG_MAC_IFG_RX_IFG2(x)		((x) << 4)
#define		DEV_MAC_CFG_MAC_IFG_RX_IFG1(x)		(x)
#define DEV_PCS1G_CFG_PCS1G_CFG		0x40
#define		DEV_PCS1G_CFG_PCS1G_CFG_PCS_ENA		BIT(0)
#define DEV_PCS1G_CFG_PCS1G_MODE	0x44
#define DEV_PCS1G_CFG_PCS1G_SD		0x48
#define DEV_PCS1G_CFG_PCS1G_ANEG	0x4c
#define		DEV_PCS1G_CFG_PCS1G_ANEG_ADV_ABILITY(x)	((x) << 16)

#define IFH_INJ_BYPASS		BIT(31)
#define IFH_TAG_TYPE_C		0
#define MAC_VID			1
#define CPU_PORT		26
#define INTERNAL_PORT_MSK	0xFFFFFF
#define IFH_LEN			2
#define ETH_ALEN		6
#define PGID_BROADCAST		28
#define PGID_UNICAST		29
#define PGID_SRC		80

static const char * const regs_names[] = {
	"port0", "port1", "port2", "port3", "port4", "port5", "port6", "port7",
	"port8", "port9", "port10", "port11", "port12", "port13", "port14",
	"port15", "port16", "port17", "port18", "port19", "port20", "port21",
	"port22", "port23",
	"sys", "ana", "rew", "gcb", "qs", "hsio",
};

#define REGS_NAMES_COUNT ARRAY_SIZE(regs_names) + 1
#define MAX_PORT 24

enum luton_ctrl_regs {
	SYS = MAX_PORT,
	ANA,
	REW,
	GCB,
	QS,
	HSIO
};

#define MIN_INT_PORT	0
#define PORT10		10
#define PORT11		11
#define MAX_INT_PORT	12
#define MIN_EXT_PORT	MAX_INT_PORT
#define MAX_EXT_PORT	MAX_PORT

#define LUTON_MIIM_BUS_COUNT 2

struct luton_phy_port_t {
	size_t phy_addr;
	struct mii_dev *bus;
	u8 serdes_index;
	u8 phy_mode;
};

struct luton_private {
	void __iomem *regs[REGS_NAMES_COUNT];
	struct mii_dev *bus[LUTON_MIIM_BUS_COUNT];
	struct luton_phy_port_t ports[MAX_PORT];
};

struct mscc_miim_dev {
	void __iomem *regs;
	phys_addr_t miim_base;
	unsigned long miim_size;
	struct mii_dev *bus;
};

static const unsigned long luton_regs_qs[] = {
	[MSCC_QS_XTR_RD] = 0x18,
	[MSCC_QS_XTR_FLUSH] = 0x28,
	[MSCC_QS_XTR_DATA_PRESENT] = 0x2c,
	[MSCC_QS_INJ_WR] = 0x3c,
	[MSCC_QS_INJ_CTRL] = 0x44,
};

static const unsigned long luton_regs_ana_table[] = {
	[MSCC_ANA_TABLES_MACHDATA] = 0x11b0,
	[MSCC_ANA_TABLES_MACLDATA] = 0x11b4,
	[MSCC_ANA_TABLES_MACACCESS] = 0x11b8,
};

static struct mscc_miim_dev miim[LUTON_MIIM_BUS_COUNT];
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

static void luton_stop(struct udevice *dev)
{
	struct luton_private *priv = dev_get_priv(dev);

	/*
	 * Switch core only reset affects VCORE-III bus and MIPS frequency
	 * and thereby also the DDR SDRAM controller. The workaround is to
	 * not to redirect any trafic to the CPU after the data transfer.
	 */
	writel(GENMASK(9, 2), priv->regs[SYS] + SYS_SCH_CPU);
}

static void luton_cpu_capture_setup(struct luton_private *priv)
{
	int i;

	/* map the 8 CPU extraction queues to CPU port 26 */
	writel(0x0, priv->regs[SYS] + SYS_SCH_CPU);

	for (i = 0; i <= 1; i++) {
		/*
		 * One to one mapping from CPU Queue number to Group extraction
		 * number
		 */
		writel(QS_XTR_MAP_ENA | (QS_XTR_MAP_GRP * i),
		       priv->regs[QS] + QS_XTR_MAP(i));

		/* Enable IFH insertion/parsing on CPU ports */
		setbits_le32(priv->regs[REW] + REW_PORT_CFG(CPU_PORT + i),
			     REW_PORT_CFG_IFH_INSERT_ENA);

		/* Enable IFH parsing on CPU port 0 and 1 */
		setbits_le32(priv->regs[SYS] + SYS_PORT_MODE(CPU_PORT + i),
			     SYS_PORT_MODE_INCL_INJ_HDR);
	}

	/* Make VLAN aware for CPU traffic */
	writel(ANA_PORT_VLAN_CFG_AWARE_ENA |
	       ANA_PORT_VLAN_CFG_POP_CNT(1) |
	       MAC_VID,
	       priv->regs[ANA] + ANA_PORT_VLAN_CFG(CPU_PORT));

	/* Disable learning (only RECV_ENA must be set) */
	writel(ANA_PORT_PORT_CFG_RECV_ENA,
	       priv->regs[ANA] + ANA_PORT_PORT_CFG(CPU_PORT));

	/* Enable switching to/from cpu port */
	setbits_le32(priv->regs[SYS] + SYS_SWITCH_PORT_MODE(CPU_PORT),
		     SYS_SWITCH_PORT_MODE_PORT_ENA);

	setbits_le32(priv->regs[SYS] + SYS_EGR_NO_SHARING, BIT(CPU_PORT));
}

static void luton_gmii_port_init(struct luton_private *priv, int port)
{
	void __iomem *regs = priv->regs[port];

	writel(0, regs + DEV_GMII_PORT_MODE_CLK);

	/* Enable MAC RX and TX */
	writel(DEV_GMII_MAC_CFG_MAC_ENA_RX_ENA |
	       DEV_GMII_MAC_CFG_MAC_ENA_TX_ENA,
	       regs + DEV_GMII_MAC_CFG_MAC_ENA);

	/* Make VLAN aware for CPU traffic */
	writel(ANA_PORT_VLAN_CFG_AWARE_ENA |
	       ANA_PORT_VLAN_CFG_POP_CNT(1) |
	       MAC_VID,
	       priv->regs[ANA] + ANA_PORT_VLAN_CFG(port));

	/* Enable switching to/from port */
	setbits_le32(priv->regs[SYS] + SYS_SWITCH_PORT_MODE(port),
		     SYS_SWITCH_PORT_MODE_PORT_ENA);
}

static void luton_port_init(struct luton_private *priv, int port)
{
	void __iomem *regs = priv->regs[port];

	writel(0, regs + DEV_PORT_MODE_CLK);

	/* Enable MAC RX and TX */
	writel(DEV_MAC_CFG_MAC_ENA_RX_ENA |
	       DEV_MAC_CFG_MAC_ENA_TX_ENA,
	       regs + DEV_MAC_CFG_MAC_ENA);

	/* Make VLAN aware for CPU traffic */
	writel(ANA_PORT_VLAN_CFG_AWARE_ENA |
	       ANA_PORT_VLAN_CFG_POP_CNT(1) |
	       MAC_VID,
	       priv->regs[ANA] + ANA_PORT_VLAN_CFG(port));

	/* Enable switching to/from port */
	setbits_le32(priv->regs[SYS] + SYS_SWITCH_PORT_MODE(port),
		     SYS_SWITCH_PORT_MODE_PORT_ENA);
}

static void luton_ext_port_init(struct luton_private *priv, int port)
{
	void __iomem *regs = priv->regs[port];

	/* Enable PCS */
	writel(DEV_PCS1G_CFG_PCS1G_CFG_PCS_ENA,
	       regs + DEV_PCS1G_CFG_PCS1G_CFG);

	/* Disable Signal Detect */
	writel(0, regs + DEV_PCS1G_CFG_PCS1G_SD);

	/* Enable MAC RX and TX */
	writel(DEV_MAC_CFG_MAC_ENA_RX_ENA |
	       DEV_MAC_CFG_MAC_ENA_TX_ENA,
	       regs + DEV_MAC_CFG_MAC_ENA);

	/* Clear sgmii_mode_ena */
	writel(0, regs + DEV_PCS1G_CFG_PCS1G_MODE);

	/*
	 * Clear sw_resolve_ena(bit 0) and set adv_ability to
	 * something meaningful just in case
	 */
	writel(DEV_PCS1G_CFG_PCS1G_ANEG_ADV_ABILITY(0x20),
	       regs + DEV_PCS1G_CFG_PCS1G_ANEG);

	/* Set MAC IFG Gaps */
	writel(DEV_MAC_CFG_MAC_IFG_TX_IFG(7) |
	       DEV_MAC_CFG_MAC_IFG_RX_IFG1(1) |
	       DEV_MAC_CFG_MAC_IFG_RX_IFG2(5),
	       regs + DEV_MAC_CFG_MAC_IFG);

	/* Set link speed and release all resets */
	writel(DEV_PORT_MODE_CLK_LINK_SPEED_1000,
	       regs + DEV_PORT_MODE_CLK);

	/* Make VLAN aware for CPU traffic */
	writel(ANA_PORT_VLAN_CFG_AWARE_ENA |
	       ANA_PORT_VLAN_CFG_POP_CNT(1) |
	       MAC_VID,
	       priv->regs[ANA] + ANA_PORT_VLAN_CFG(port));

	/* Enable switching to/from port */
	setbits_le32(priv->regs[SYS] + SYS_SWITCH_PORT_MODE(port),
		     SYS_SWITCH_PORT_MODE_PORT_ENA);
}

static void serdes6g_write(void __iomem *base, u32 addr)
{
	u32 data;

	writel(HSIO_MCB_SERDES6G_CFG_WR_ONE_SHOT |
	       HSIO_MCB_SERDES6G_CFG_ADDR(addr),
	       base + HSIO_MCB_SERDES6G_CFG);

	do {
		data = readl(base + HSIO_MCB_SERDES6G_CFG);
	} while (data & HSIO_MCB_SERDES6G_CFG_WR_ONE_SHOT);
}

static void serdes6g_setup(void __iomem *base, uint32_t addr,
			   phy_interface_t interface)
{
	writel(HSIO_RCOMP_CFG_CFG0_MODE_SEL(0x3) |
	       HSIO_RCOMP_CFG_CFG0_RUN_CAL,
	       base + HSIO_RCOMP_CFG_CFG0);

	while (readl(base + HSIO_RCOMP_STATUS) &
	       HSIO_RCOMP_STATUS_BUSY)
		;

	writel(HSIO_SERDES6G_ANA_CFG_OB_CFG_SR(0xb) |
	       HSIO_SERDES6G_ANA_CFG_OB_CFG_SR_H |
	       HSIO_SERDES6G_ANA_CFG_OB_CFG_POST0(0x10) |
	       HSIO_SERDES6G_ANA_CFG_OB_CFG_POL |
	       HSIO_SERDES6G_ANA_CFG_OB_CFG_ENA1V_MODE,
	       base + HSIO_SERDES6G_ANA_CFG_OB_CFG);
	writel(HSIO_SERDES6G_ANA_CFG_OB_CFG1_LEV(0x18) |
	       HSIO_SERDES6G_ANA_CFG_OB_CFG1_ENA_CAS(0x1),
	       base + HSIO_SERDES6G_ANA_CFG_OB_CFG1);
	writel(HSIO_SERDES6G_ANA_CFG_IB_CFG_RESISTOR_CTRL(0xc) |
	       HSIO_SERDES6G_ANA_CFG_IB_CFG_VBCOM(0x4) |
	       HSIO_SERDES6G_ANA_CFG_IB_CFG_VBAC(0x5) |
	       HSIO_SERDES6G_ANA_CFG_IB_CFG_RT(0xf) |
	       HSIO_SERDES6G_ANA_CFG_IB_CFG_RF(0x4),
	       base + HSIO_SERDES6G_ANA_CFG_IB_CFG);
	writel(HSIO_SERDES6G_ANA_CFG_IB_CFG1_RST |
	       HSIO_SERDES6G_ANA_CFG_IB_CFG1_ENA_OFFSDC |
	       HSIO_SERDES6G_ANA_CFG_IB_CFG1_ENA_OFFSAC |
	       HSIO_SERDES6G_ANA_CFG_IB_CFG1_ANEG_MODE |
	       HSIO_SERDES6G_ANA_CFG_IB_CFG1_CHF |
	       HSIO_SERDES6G_ANA_CFG_IB_CFG1_C(0x4),
	       base + HSIO_SERDES6G_ANA_CFG_IB_CFG1);
	writel(HSIO_SERDES6G_ANA_CFG_DES_CFG_BW_ANA(0x5) |
	       HSIO_SERDES6G_ANA_CFG_DES_CFG_BW_HYST(0x5) |
	       HSIO_SERDES6G_ANA_CFG_DES_CFG_MBTR_CTRL(0x2) |
	       HSIO_SERDES6G_ANA_CFG_DES_CFG_PHS_CTRL(0x6),
	       base + HSIO_SERDES6G_ANA_CFG_DES_CFG);
	writel(HSIO_SERDES6G_ANA_CFG_PLL_CFG_FSM_ENA |
	       HSIO_SERDES6G_ANA_CFG_PLL_CFG_FSM_CTRL_DATA(0x78),
	       base + HSIO_SERDES6G_ANA_CFG_PLL_CFG);
	writel(HSIO_SERDES6G_ANA_CFG_COMMON_CFG_IF_MODE(0x30) |
	       HSIO_SERDES6G_ANA_CFG_COMMON_CFG_ENA_LANE,
	       base + HSIO_SERDES6G_ANA_CFG_COMMON_CFG);
	/*
	 * There are 4 serdes6g, configure all except serdes6g0, therefore
	 * the address is b1110
	 */
	serdes6g_write(base, addr);

	writel(readl(base + HSIO_SERDES6G_ANA_CFG_COMMON_CFG) |
	       HSIO_SERDES6G_ANA_CFG_COMMON_CFG_SYS_RST,
	       base + HSIO_SERDES6G_ANA_CFG_COMMON_CFG);
	serdes6g_write(base, addr);

	clrbits_le32(base + HSIO_SERDES6G_ANA_CFG_IB_CFG1,
		     HSIO_SERDES6G_ANA_CFG_IB_CFG1_RST);
	writel(HSIO_SERDES6G_DIG_CFG_MISC_CFG_LANE_RST,
	       base + HSIO_SERDES6G_DIG_CFG_MISC_CFG);
	serdes6g_write(base, addr);
}

static void serdes_setup(struct luton_private *priv)
{
	size_t mask;
	int i = 0;

	for (i = 0; i < MAX_PORT; ++i) {
		if (!priv->ports[i].bus || priv->ports[i].serdes_index == 0xff)
			continue;

		mask = BIT(priv->ports[i].serdes_index);
		serdes6g_setup(priv->regs[HSIO], mask, priv->ports[i].phy_mode);
	}
}

static int luton_switch_init(struct luton_private *priv)
{
	setbits_le32(priv->regs[HSIO] + HSIO_PLL5G_CFG_PLL5G_CFG2, BIT(1));
	clrbits_le32(priv->regs[HSIO] + HSIO_PLL5G_CFG_PLL5G_CFG2, BIT(1));

	/* Reset switch & memories */
	writel(SYS_SYSTEM_RST_MEM_ENA | SYS_SYSTEM_RST_MEM_INIT,
	       priv->regs[SYS] + SYS_SYSTEM_RST_CFG);

	/* Wait to complete */
	if (wait_for_bit_le32(priv->regs[SYS] + SYS_SYSTEM_RST_CFG,
			      SYS_SYSTEM_RST_MEM_INIT, false, 2000, false)) {
		printf("Timeout in memory reset\n");
	}

	/* Enable switch core */
	setbits_le32(priv->regs[SYS] + SYS_SYSTEM_RST_CFG,
		     SYS_SYSTEM_RST_CORE_ENA);

	/* Setup the Serdes macros */
	serdes_setup(priv);

	return 0;
}

static int luton_initialize(struct luton_private *priv)
{
	int ret, i;

	/* Initialize switch memories, enable core */
	ret = luton_switch_init(priv);
	if (ret)
		return ret;

	/*
	 * Disable port-to-port by switching
	 * Put front ports in "port isolation modes" - i.e. they can't send
	 * to other ports - via the PGID sorce masks.
	 */
	for (i = 0; i < MAX_PORT; i++)
		writel(0, priv->regs[ANA] + ANA_PGID(PGID_SRC + i));

	/* Flush queues */
	mscc_flush(priv->regs[QS], luton_regs_qs);

	/* Setup frame ageing - "2 sec" - The unit is 4ns on Luton*/
	writel(2000000000 / 4,
	       priv->regs[SYS] + SYS_FRM_AGING);

	for (i = 0; i < MAX_PORT; i++) {
		if (i < PORT10)
			luton_gmii_port_init(priv, i);
		else
			if (i == PORT10 || i == PORT11)
				luton_port_init(priv, i);
			else
				luton_ext_port_init(priv, i);
	}

	luton_cpu_capture_setup(priv);

	return 0;
}

static int luton_write_hwaddr(struct udevice *dev)
{
	struct luton_private *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_platdata(dev);

	mscc_mac_table_add(priv->regs[ANA], luton_regs_ana_table,
			   pdata->enetaddr, PGID_UNICAST);

	writel(BIT(CPU_PORT), priv->regs[ANA] + ANA_PGID(PGID_UNICAST));

	return 0;
}

static int luton_start(struct udevice *dev)
{
	struct luton_private *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_platdata(dev);
	const unsigned char mac[ETH_ALEN] = { 0xff, 0xff, 0xff, 0xff, 0xff,
					      0xff };
	int ret;

	ret = luton_initialize(priv);
	if (ret)
		return ret;

	/* Set MAC address tables entries for CPU redirection */
	mscc_mac_table_add(priv->regs[ANA], luton_regs_ana_table,
			   mac, PGID_BROADCAST);

	writel(BIT(CPU_PORT) | INTERNAL_PORT_MSK,
	       priv->regs[ANA] + ANA_PGID(PGID_BROADCAST));

	mscc_mac_table_add(priv->regs[ANA], luton_regs_ana_table,
			   pdata->enetaddr, PGID_UNICAST);

	writel(BIT(CPU_PORT), priv->regs[ANA] + ANA_PGID(PGID_UNICAST));

	return 0;
}

static int luton_send(struct udevice *dev, void *packet, int length)
{
	struct luton_private *priv = dev_get_priv(dev);
	u32 ifh[IFH_LEN];
	int port = BIT(0);	/* use port 0 */
	u32 *buf = packet;

	ifh[0] = IFH_INJ_BYPASS | port;
	ifh[1] = (IFH_TAG_TYPE_C << 16);

	return mscc_send(priv->regs[QS], luton_regs_qs,
			 ifh, IFH_LEN, buf, length);
}

static int luton_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct luton_private *priv = dev_get_priv(dev);
	u32 *rxbuf = (u32 *)net_rx_packets[0];
	int byte_cnt = 0;

	byte_cnt = mscc_recv(priv->regs[QS], luton_regs_qs, rxbuf, IFH_LEN,
			     true);

	*packetp = net_rx_packets[0];

	return byte_cnt;
}

static struct mii_dev *get_mdiobus(phys_addr_t base, unsigned long size)
{
	int i = 0;

	for (i = 0; i < LUTON_MIIM_BUS_COUNT; ++i)
		if (miim[i].miim_base == base && miim[i].miim_size == size)
			return miim[i].bus;

	return NULL;
}

static void add_port_entry(struct luton_private *priv, size_t index,
			   size_t phy_addr, struct mii_dev *bus,
			   u8 serdes_index, u8 phy_mode)
{
	priv->ports[index].phy_addr = phy_addr;
	priv->ports[index].bus = bus;
	priv->ports[index].serdes_index = serdes_index;
	priv->ports[index].phy_mode = phy_mode;
}

static int luton_probe(struct udevice *dev)
{
	struct luton_private *priv = dev_get_priv(dev);
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

	/* Release reset in the CU-PHY */
	writel(0, priv->regs[GCB] + GCB_DEVCPU_RST_SOFT_CHIP_RST);

	/* Ports with ext phy don't need to reset clk */
	for (i = 0; i < MAX_INT_PORT; i++) {
		if (i < PORT10)
			clrbits_le32(priv->regs[i] + DEV_GMII_PORT_MODE_CLK,
				     DEV_GMII_PORT_MODE_CLK_PHY_RST);
		else
			clrbits_le32(priv->regs[i] + DEV_PORT_MODE_CLK,
				     DEV_PORT_MODE_CLK_PHY_RST);
	}

	/* Wait for internal PHY to be ready */
	if (wait_for_bit_le32(priv->regs[GCB] + GCB_MISC_STAT,
			      GCB_MISC_STAT_PHY_READY, true, 500, false))
		return -EACCES;


	/* Initialize miim buses */
	memset(&miim, 0x0, sizeof(miim) * LUTON_MIIM_BUS_COUNT);

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
			add_port_entry(priv, i, phy_addr, bus, 0xff, 0xff);
		else
			add_port_entry(priv, i, phy_addr, bus, phandle.args[1],
				       phandle.args[2]);
	}

	for (i = 0; i < MAX_PORT; i++) {
		if (!priv->ports[i].bus)
			continue;

		phy = phy_connect(priv->ports[i].bus,
				  priv->ports[i].phy_addr, dev,
				  PHY_INTERFACE_MODE_NONE);
		if (phy && i >= MAX_INT_PORT)
			board_phy_config(phy);
	}

	/*
	 * coma_mode is need on only one phy, because all the other phys
	 * will be affected.
	 */
	mscc_miim_write(priv->ports[0].bus, 0, 0, 31, 0x10);
	mscc_miim_write(priv->ports[0].bus, 0, 0, 14, 0x800);
	mscc_miim_write(priv->ports[0].bus, 0, 0, 31, 0);

	return 0;
}

static int luton_remove(struct udevice *dev)
{
	struct luton_private *priv = dev_get_priv(dev);
	int i;

	for (i = 0; i < LUTON_MIIM_BUS_COUNT; i++) {
		mdio_unregister(priv->bus[i]);
		mdio_free(priv->bus[i]);
	}

	return 0;
}

static const struct eth_ops luton_ops = {
	.start        = luton_start,
	.stop         = luton_stop,
	.send         = luton_send,
	.recv         = luton_recv,
	.write_hwaddr = luton_write_hwaddr,
};

static const struct udevice_id mscc_luton_ids[] = {
	{.compatible = "mscc,vsc7527-switch", },
	{ /* Sentinel */ }
};

U_BOOT_DRIVER(luton) = {
	.name     = "luton-switch",
	.id       = UCLASS_ETH,
	.of_match = mscc_luton_ids,
	.probe	  = luton_probe,
	.remove	  = luton_remove,
	.ops	  = &luton_ops,
	.priv_auto_alloc_size = sizeof(struct luton_private),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
};
