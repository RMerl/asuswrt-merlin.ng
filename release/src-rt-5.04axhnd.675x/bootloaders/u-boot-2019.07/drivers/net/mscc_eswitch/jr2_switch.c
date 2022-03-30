// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2018 Microsemi Corporation
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

#include <dt-bindings/mscc/jr2_data.h>
#include "mscc_xfer.h"

#define GCB_MIIM_MII_STATUS		0x0
#define		GCB_MIIM_STAT_BUSY		BIT(3)
#define GCB_MIIM_MII_CMD		0x8
#define		GCB_MIIM_MII_CMD_SCAN		BIT(0)
#define		GCB_MIIM_MII_CMD_OPR_WRITE	BIT(1)
#define		GCB_MIIM_MII_CMD_OPR_READ	BIT(2)
#define		GCB_MIIM_MII_CMD_SINGLE_SCAN	BIT(3)
#define		GCB_MIIM_MII_CMD_WRDATA(x)	((x) << 4)
#define		GCB_MIIM_MII_CMD_REGAD(x)	((x) << 20)
#define		GCB_MIIM_MII_CMD_PHYAD(x)	((x) << 25)
#define		GCB_MIIM_MII_CMD_VLD		BIT(31)
#define GCB_MIIM_DATA			0xC
#define		GCB_MIIM_DATA_ERROR		(0x3 << 16)

#define ANA_AC_RAM_CTRL_RAM_INIT		0x94358
#define ANA_AC_STAT_GLOBAL_CFG_PORT_RESET	0x94370

#define ANA_CL_PORT_VLAN_CFG(x)			(0x24018 + 0xc8 * (x))
#define		ANA_CL_PORT_VLAN_CFG_AWARE_ENA			BIT(19)
#define		ANA_CL_PORT_VLAN_CFG_POP_CNT(x)			((x) << 17)

#define ANA_L2_COMMON_FWD_CFG			0x8a2a8
#define		ANA_L2_COMMON_FWD_CFG_CPU_DMAC_COPY_ENA	BIT(6)

#define ASM_CFG_STAT_CFG			0x3508
#define ASM_CFG_PORT(x)				(0x36c4 + 0x4 * (x))
#define		ASM_CFG_PORT_NO_PREAMBLE_ENA		BIT(8)
#define		ASM_CFG_PORT_INJ_FORMAT_CFG(x)		((x) << 1)
#define ASM_RAM_CTRL_RAM_INIT			0x39b8

#define DEV_DEV_CFG_DEV_RST_CTRL		0x0
#define		DEV_DEV_CFG_DEV_RST_CTRL_SPEED_SEL(x)	((x) << 20)
#define DEV_MAC_CFG_MAC_ENA		0x1c
#define		DEV_MAC_CFG_MAC_ENA_RX_ENA		BIT(4)
#define		DEV_MAC_CFG_MAC_ENA_TX_ENA		BIT(0)
#define	DEV_MAC_CFG_MAC_IFG		0x34
#define		DEV_MAC_CFG_MAC_IFG_TX_IFG(x)		((x) << 8)
#define		DEV_MAC_CFG_MAC_IFG_RX_IFG2(x)		((x) << 4)
#define		DEV_MAC_CFG_MAC_IFG_RX_IFG1(x)		(x)
#define	DEV_PCS1G_CFG_PCS1G_CFG		0x40
#define		DEV_PCS1G_CFG_PCS1G_CFG_PCS_ENA		BIT(0)
#define	DEV_PCS1G_CFG_PCS1G_MODE	0x44
#define	DEV_PCS1G_CFG_PCS1G_SD		0x48
#define	DEV_PCS1G_CFG_PCS1G_ANEG	0x4c
#define		DEV_PCS1G_CFG_PCS1G_ANEG_ADV_ABILITY(x)	((x) << 16)

#define DSM_RAM_CTRL_RAM_INIT		0x8

#define HSIO_ANA_SERDES1G_DES_CFG		0xac
#define		HSIO_ANA_SERDES1G_DES_CFG_BW_HYST(x)		((x) << 1)
#define		HSIO_ANA_SERDES1G_DES_CFG_BW_ANA(x)		((x) << 5)
#define		HSIO_ANA_SERDES1G_DES_CFG_MBTR_CTRL(x)		((x) << 8)
#define		HSIO_ANA_SERDES1G_DES_CFG_PHS_CTRL(x)		((x) << 13)
#define HSIO_ANA_SERDES1G_IB_CFG		0xb0
#define		HSIO_ANA_SERDES1G_IB_CFG_RESISTOR_CTRL(x)	(x)
#define		HSIO_ANA_SERDES1G_IB_CFG_EQ_GAIN(x)		((x) << 6)
#define		HSIO_ANA_SERDES1G_IB_CFG_ENA_OFFSET_COMP	BIT(9)
#define		HSIO_ANA_SERDES1G_IB_CFG_ENA_DETLEV		BIT(11)
#define		HSIO_ANA_SERDES1G_IB_CFG_ENA_CMV_TERM		BIT(13)
#define		HSIO_ANA_SERDES1G_IB_CFG_DET_LEV(x)		((x) << 19)
#define		HSIO_ANA_SERDES1G_IB_CFG_ACJTAG_HYST(x)		((x) << 24)
#define HSIO_ANA_SERDES1G_OB_CFG		0xb4
#define		HSIO_ANA_SERDES1G_OB_CFG_RESISTOR_CTRL(x)	(x)
#define		HSIO_ANA_SERDES1G_OB_CFG_VCM_CTRL(x)		((x) << 4)
#define		HSIO_ANA_SERDES1G_OB_CFG_CMM_BIAS_CTRL(x)	((x) << 10)
#define		HSIO_ANA_SERDES1G_OB_CFG_AMP_CTRL(x)		((x) << 13)
#define		HSIO_ANA_SERDES1G_OB_CFG_SLP(x)			((x) << 17)
#define HSIO_ANA_SERDES1G_SER_CFG		0xb8
#define HSIO_ANA_SERDES1G_COMMON_CFG		0xbc
#define		HSIO_ANA_SERDES1G_COMMON_CFG_IF_MODE		BIT(0)
#define		HSIO_ANA_SERDES1G_COMMON_CFG_ENA_LANE		BIT(18)
#define		HSIO_ANA_SERDES1G_COMMON_CFG_SYS_RST		BIT(31)
#define HSIO_ANA_SERDES1G_PLL_CFG		0xc0
#define		HSIO_ANA_SERDES1G_PLL_CFG_FSM_ENA		BIT(7)
#define		HSIO_ANA_SERDES1G_PLL_CFG_FSM_CTRL_DATA(x)	((x) << 8)
#define		HSIO_ANA_SERDES1G_PLL_CFG_ENA_RC_DIV2		BIT(21)
#define HSIO_DIG_SERDES1G_DFT_CFG0		0xc8
#define HSIO_DIG_SERDES1G_TP_CFG		0xd4
#define HSIO_DIG_SERDES1G_MISC_CFG		0xdc
#define		HSIO_DIG_SERDES1G_MISC_CFG_LANE_RST		BIT(0)
#define HSIO_MCB_SERDES1G_CFG			0xe8
#define		HSIO_MCB_SERDES1G_CFG_WR_ONE_SHOT		BIT(31)
#define		HSIO_MCB_SERDES1G_CFG_ADDR(x)			(x)

#define HSIO_ANA_SERDES6G_DES_CFG		0x11c
#define		HSIO_ANA_SERDES6G_DES_CFG_SWAP_ANA		BIT(0)
#define		HSIO_ANA_SERDES6G_DES_CFG_BW_ANA(x)		((x) << 1)
#define		HSIO_ANA_SERDES6G_DES_CFG_SWAP_HYST		BIT(4)
#define		HSIO_ANA_SERDES6G_DES_CFG_BW_HYST(x)		((x) << 5)
#define		HSIO_ANA_SERDES6G_DES_CFG_CPMD_SEL(x)		((x) << 8)
#define		HSIO_ANA_SERDES6G_DES_CFG_MBTR_CTRL(x)		((x) << 10)
#define		HSIO_ANA_SERDES6G_DES_CFG_PHS_CTRL(x)		((x) << 13)
#define HSIO_ANA_SERDES6G_IB_CFG		0x120
#define		HSIO_ANA_SERDES6G_IB_CFG_REG_ENA		BIT(0)
#define		HSIO_ANA_SERDES6G_IB_CFG_EQZ_ENA		BIT(1)
#define		HSIO_ANA_SERDES6G_IB_CFG_SAM_ENA		BIT(2)
#define		HSIO_ANA_SERDES6G_IB_CFG_CAL_ENA(x)		((x) << 3)
#define		HSIO_ANA_SERDES6G_IB_CFG_CONCUR			BIT(4)
#define		HSIO_ANA_SERDES6G_IB_CFG_SIG_DET_ENA		BIT(5)
#define		HSIO_ANA_SERDES6G_IB_CFG_REG_PAT_SEL_OFF(x)	((x) << 7)
#define		HSIO_ANA_SERDES6G_IB_CFG_REG_PAT_SEL_LP(x)	((x) << 9)
#define		HSIO_ANA_SERDES6G_IB_CFG_REG_PAT_SEL_MID(x)	((x) << 11)
#define		HSIO_ANA_SERDES6G_IB_CFG_REG_PAT_SEL_HP(x)	((x) << 13)
#define		HSIO_ANA_SERDES6G_IB_CFG_SIG_DET_CLK_SEL(x)	((x) << 15)
#define		HSIO_ANA_SERDES6G_IB_CFG_TERM_MODE_SEL(x)	((x) << 18)
#define		HSIO_ANA_SERDES6G_IB_CFG_ICML_ADJ(x)		((x) << 20)
#define		HSIO_ANA_SERDES6G_IB_CFG_RTRM_ADJ(x)		((x) << 24)
#define		HSIO_ANA_SERDES6G_IB_CFG_VBULK_SEL		BIT(28)
#define		HSIO_ANA_SERDES6G_IB_CFG_SOFSI(x)		((x) << 29)
#define HSIO_ANA_SERDES6G_IB_CFG1		0x124
#define		HSIO_ANA_SERDES6G_IB_CFG1_FILT_OFFSET		BIT(4)
#define		HSIO_ANA_SERDES6G_IB_CFG1_FILT_LP		BIT(5)
#define		HSIO_ANA_SERDES6G_IB_CFG1_FILT_MID		BIT(6)
#define		HSIO_ANA_SERDES6G_IB_CFG1_FILT_HP		BIT(7)
#define		HSIO_ANA_SERDES6G_IB_CFG1_SCALY(x)		((x) << 8)
#define		HSIO_ANA_SERDES6G_IB_CFG1_TSDET(x)		((x) << 12)
#define		HSIO_ANA_SERDES6G_IB_CFG1_TJTAG(x)		((x) << 17)
#define HSIO_ANA_SERDES6G_IB_CFG2		0x128
#define		HSIO_ANA_SERDES6G_IB_CFG2_UREG(x)		(x)
#define		HSIO_ANA_SERDES6G_IB_CFG2_UMAX(x)		((x) << 3)
#define		HSIO_ANA_SERDES6G_IB_CFG2_TCALV(x)		((x) << 5)
#define		HSIO_ANA_SERDES6G_IB_CFG2_OCALS(x)		((x) << 10)
#define		HSIO_ANA_SERDES6G_IB_CFG2_OINFS(x)		((x) << 16)
#define		HSIO_ANA_SERDES6G_IB_CFG2_OINFI(x)		((x) << 22)
#define		HSIO_ANA_SERDES6G_IB_CFG2_TINFV(x)		((x) << 27)
#define HSIO_ANA_SERDES6G_IB_CFG3		0x12c
#define		HSIO_ANA_SERDES6G_IB_CFG3_INI_OFFSET(x)		(x)
#define		HSIO_ANA_SERDES6G_IB_CFG3_INI_LP(x)		((x) << 6)
#define		HSIO_ANA_SERDES6G_IB_CFG3_INI_MID(x)		((x) << 12)
#define		HSIO_ANA_SERDES6G_IB_CFG3_INI_HP(x)		((x) << 18)
#define HSIO_ANA_SERDES6G_IB_CFG4		0x130
#define		HSIO_ANA_SERDES6G_IB_CFG4_MAX_OFFSET(x)		(x)
#define		HSIO_ANA_SERDES6G_IB_CFG4_MAX_LP(x)		((x) << 6)
#define		HSIO_ANA_SERDES6G_IB_CFG4_MAX_MID(x)		((x) << 12)
#define		HSIO_ANA_SERDES6G_IB_CFG4_MAX_HP(x)		((x) << 18)
#define HSIO_ANA_SERDES6G_IB_CFG5		0x134
#define		HSIO_ANA_SERDES6G_IB_CFG4_MIN_OFFSET(x)		(x)
#define		HSIO_ANA_SERDES6G_IB_CFG4_MIN_LP(x)		((x) << 6)
#define		HSIO_ANA_SERDES6G_IB_CFG4_MIN_MID(x)		((x) << 12)
#define		HSIO_ANA_SERDES6G_IB_CFG4_MIN_HP(x)		((x) << 18)
#define HSIO_ANA_SERDES6G_OB_CFG		0x138
#define		HSIO_ANA_SERDES6G_OB_CFG_RESISTOR_CTRL(x)	(x)
#define		HSIO_ANA_SERDES6G_OB_CFG_SR(x)			((x) << 4)
#define		HSIO_ANA_SERDES6G_OB_CFG_SR_H			BIT(8)
#define		HSIO_ANA_SERDES6G_OB_CFG_SEL_RCTRL		BIT(9)
#define		HSIO_ANA_SERDES6G_OB_CFG_R_COR			BIT(10)
#define		HSIO_ANA_SERDES6G_OB_CFG_POST1(x)		((x) << 11)
#define		HSIO_ANA_SERDES6G_OB_CFG_R_ADJ_PDR		BIT(16)
#define		HSIO_ANA_SERDES6G_OB_CFG_R_ADJ_MUX		BIT(17)
#define		HSIO_ANA_SERDES6G_OB_CFG_PREC(x)		((x) << 18)
#define		HSIO_ANA_SERDES6G_OB_CFG_POST0(x)		((x) << 23)
#define		HSIO_ANA_SERDES6G_OB_CFG_POL			BIT(29)
#define		HSIO_ANA_SERDES6G_OB_CFG_ENA1V_MODE(x)		((x) << 30)
#define		HSIO_ANA_SERDES6G_OB_CFG_IDLE			BIT(31)
#define HSIO_ANA_SERDES6G_OB_CFG1		0x13c
#define		HSIO_ANA_SERDES6G_OB_CFG1_LEV(x)		(x)
#define		HSIO_ANA_SERDES6G_OB_CFG1_ENA_CAS(x)		((x) << 6)
#define HSIO_ANA_SERDES6G_SER_CFG		0x140
#define HSIO_ANA_SERDES6G_COMMON_CFG		0x144
#define		HSIO_ANA_SERDES6G_COMMON_CFG_IF_MODE(x)		(x)
#define		HSIO_ANA_SERDES6G_COMMON_CFG_QRATE(x)		(x << 2)
#define		HSIO_ANA_SERDES6G_COMMON_CFG_ENA_LANE		BIT(14)
#define		HSIO_ANA_SERDES6G_COMMON_CFG_SYS_RST		BIT(16)
#define HSIO_ANA_SERDES6G_PLL_CFG		0x148
#define		HSIO_ANA_SERDES6G_PLL_CFG_ROT_FRQ		BIT(0)
#define		HSIO_ANA_SERDES6G_PLL_CFG_ROT_DIR		BIT(1)
#define		HSIO_ANA_SERDES6G_PLL_CFG_RB_DATA_SEL		BIT(2)
#define		HSIO_ANA_SERDES6G_PLL_CFG_FSM_OOR_RECAL_ENA	BIT(3)
#define		HSIO_ANA_SERDES6G_PLL_CFG_FSM_FORCE_SET_ENA	BIT(4)
#define		HSIO_ANA_SERDES6G_PLL_CFG_FSM_ENA		BIT(5)
#define		HSIO_ANA_SERDES6G_PLL_CFG_FSM_CTRL_DATA(x)	((x) << 6)
#define		HSIO_ANA_SERDES6G_PLL_CFG_ENA_ROT		BIT(14)
#define		HSIO_ANA_SERDES6G_PLL_CFG_DIV4			BIT(15)
#define		HSIO_ANA_SERDES6G_PLL_CFG_ENA_OFFS(x)		((x) << 16)
#define HSIO_DIG_SERDES6G_MISC_CFG		0x108
#define		HSIO_DIG_SERDES6G_MISC_CFG_LANE_RST		BIT(0)
#define HSIO_MCB_SERDES6G_CFG			0x168
#define		HSIO_MCB_SERDES6G_CFG_WR_ONE_SHOT		BIT(31)
#define		HSIO_MCB_SERDES6G_CFG_ADDR(x)			(x)
#define HSIO_HW_CFGSTAT_HW_CFG			0x16c

#define LRN_COMMON_ACCESS_CTRL			0x0
#define		LRN_COMMON_ACCESS_CTRL_MAC_TABLE_ACCESS_SHOT	BIT(0)
#define LRN_COMMON_MAC_ACCESS_CFG0		0x4
#define LRN_COMMON_MAC_ACCESS_CFG1		0x8
#define LRN_COMMON_MAC_ACCESS_CFG2		0xc
#define		LRN_COMMON_MAC_ACCESS_CFG2_MAC_ENTRY_ADDR(x)	(x)
#define		LRN_COMMON_MAC_ACCESS_CFG2_MAC_ENTRY_TYPE(x)	((x) << 12)
#define		LRN_COMMON_MAC_ACCESS_CFG2_MAC_ENTRY_VLD	BIT(15)
#define		LRN_COMMON_MAC_ACCESS_CFG2_MAC_ENTRY_LOCKED	BIT(16)
#define		LRN_COMMON_MAC_ACCESS_CFG2_MAC_ENTRY_CPU_COPY	BIT(23)
#define		LRN_COMMON_MAC_ACCESS_CFG2_MAC_ENTRY_CPU_QU(x)	((x) << 24)

#define QFWD_SYSTEM_SWITCH_PORT_MODE(x)		(0x4 * (x))
#define		QFWD_SYSTEM_SWITCH_PORT_MODE_PORT_ENA		BIT(17)

#define QS_XTR_GRP_CFG(x)		(0x0 + 4 * (x))
#define QS_INJ_GRP_CFG(x)		(0x24 + (x) * 4)

#define QSYS_SYSTEM_RESET_CFG			0xf0
#define QSYS_CALCFG_CAL_AUTO(x)			(0x3d4 + 4 * (x))
#define QSYS_CALCFG_CAL_CTRL			0x3e8
#define		QSYS_CALCFG_CAL_CTRL_CAL_MODE(x)		((x) << 11)
#define QSYS_RAM_CTRL_RAM_INIT			0x3ec

#define REW_RAM_CTRL_RAM_INIT			0x53528

#define VOP_RAM_CTRL_RAM_INIT			0x43638

#define XTR_VALID_BYTES(x)	(4 - ((x) & 3))
#define MAC_VID			0
#define CPU_PORT		53
#define IFH_LEN			7
#define JR2_BUF_CELL_SZ		60
#define ETH_ALEN		6
#define PGID_BROADCAST		510
#define PGID_UNICAST		511

static const char * const regs_names[] = {
	"port0", "port1", "port2", "port3", "port4", "port5", "port6", "port7",
	"port8", "port9", "port10", "port11", "port12", "port13", "port14",
	"port15", "port16", "port17", "port18", "port19", "port20", "port21",
	"port22", "port23", "port24", "port25", "port26", "port27", "port28",
	"port29", "port30", "port31", "port32", "port33", "port34", "port35",
	"port36", "port37", "port38", "port39", "port40", "port41", "port42",
	"port43", "port44", "port45", "port46", "port47",
	"ana_ac", "ana_cl", "ana_l2", "asm", "hsio", "lrn",
	"qfwd", "qs", "qsys", "rew",
};

#define REGS_NAMES_COUNT ARRAY_SIZE(regs_names) + 1
#define MAX_PORT 48

enum jr2_ctrl_regs {
	ANA_AC = MAX_PORT,
	ANA_CL,
	ANA_L2,
	ASM,
	HSIO,
	LRN,
	QFWD,
	QS,
	QSYS,
	REW,
};

#define JR2_MIIM_BUS_COUNT 3

struct jr2_phy_port_t {
	size_t phy_addr;
	struct mii_dev *bus;
	u8 serdes_index;
	u8 phy_mode;
};

struct jr2_private {
	void __iomem *regs[REGS_NAMES_COUNT];
	struct mii_dev *bus[JR2_MIIM_BUS_COUNT];
	struct jr2_phy_port_t ports[MAX_PORT];
};

struct jr2_miim_dev {
	void __iomem *regs;
	phys_addr_t miim_base;
	unsigned long miim_size;
	struct mii_dev *bus;
};

static const unsigned long jr2_regs_qs[] = {
	[MSCC_QS_XTR_RD] = 0x8,
	[MSCC_QS_XTR_FLUSH] = 0x18,
	[MSCC_QS_XTR_DATA_PRESENT] = 0x1c,
	[MSCC_QS_INJ_WR] = 0x2c,
	[MSCC_QS_INJ_CTRL] = 0x34,
};

static struct jr2_miim_dev miim[JR2_MIIM_BUS_COUNT];
static int miim_count = -1;

static int mscc_miim_wait_ready(struct jr2_miim_dev *miim)
{
	unsigned long deadline;
	u32 val;

	deadline = timer_get_us() + 250000;

	do {
		val = readl(miim->regs + GCB_MIIM_MII_STATUS);
	} while (timer_get_us() <= deadline && (val & GCB_MIIM_STAT_BUSY));

	if (val & GCB_MIIM_STAT_BUSY)
		return -ETIMEDOUT;

	return 0;
}

static int mscc_miim_read(struct mii_dev *bus, int addr, int devad, int reg)
{
	struct jr2_miim_dev *miim = (struct jr2_miim_dev *)bus->priv;
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
	struct jr2_miim_dev *miim = (struct jr2_miim_dev *)bus->priv;
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

static struct mii_dev *jr2_mdiobus_init(phys_addr_t miim_base,
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

static void jr2_cpu_capture_setup(struct jr2_private *priv)
{
	/* ASM: No preamble and IFH prefix on CPU injected frames */
	writel(ASM_CFG_PORT_NO_PREAMBLE_ENA |
	       ASM_CFG_PORT_INJ_FORMAT_CFG(1),
	       priv->regs[ASM] + ASM_CFG_PORT(CPU_PORT));

	/* Set Manual injection via DEVCPU_QS registers for CPU queue 0 */
	writel(0x5, priv->regs[QS] + QS_INJ_GRP_CFG(0));

	/* Set Manual extraction via DEVCPU_QS registers for CPU queue 0 */
	writel(0x7, priv->regs[QS] + QS_XTR_GRP_CFG(0));

	/* Enable CPU port for any frame transfer */
	setbits_le32(priv->regs[QFWD] + QFWD_SYSTEM_SWITCH_PORT_MODE(CPU_PORT),
		     QFWD_SYSTEM_SWITCH_PORT_MODE_PORT_ENA);

	/* Send a copy to CPU when found as forwarding entry */
	setbits_le32(priv->regs[ANA_L2] + ANA_L2_COMMON_FWD_CFG,
		     ANA_L2_COMMON_FWD_CFG_CPU_DMAC_COPY_ENA);
}

static void jr2_port_init(struct jr2_private *priv, int port)
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
	writel(DEV_MAC_CFG_MAC_IFG_TX_IFG(4) |
	       DEV_MAC_CFG_MAC_IFG_RX_IFG1(5) |
	       DEV_MAC_CFG_MAC_IFG_RX_IFG2(1),
	       regs + DEV_MAC_CFG_MAC_IFG);

	/* Set link speed and release all resets */
	writel(DEV_DEV_CFG_DEV_RST_CTRL_SPEED_SEL(2),
	       regs + DEV_DEV_CFG_DEV_RST_CTRL);

	/* Make VLAN aware for CPU traffic */
	writel(ANA_CL_PORT_VLAN_CFG_AWARE_ENA |
	       ANA_CL_PORT_VLAN_CFG_POP_CNT(1) |
	       MAC_VID,
	       priv->regs[ANA_CL] + ANA_CL_PORT_VLAN_CFG(port));

	/* Enable CPU port for any frame transfer */
	setbits_le32(priv->regs[QFWD] + QFWD_SYSTEM_SWITCH_PORT_MODE(port),
		     QFWD_SYSTEM_SWITCH_PORT_MODE_PORT_ENA);
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
	u32 ib_if_mode = 0;
	u32 ib_qrate = 0;
	u32 ib_cal_ena = 0;
	u32 ib1_tsdet = 0;
	u32 ob_lev = 0;
	u32 ob_ena_cas = 0;
	u32 ob_ena1v_mode = 0;
	u32 des_bw_ana = 0;
	u32 pll_fsm_ctrl_data = 0;

	switch (interface) {
	case PHY_INTERFACE_MODE_SGMII:
		ib_if_mode = 1;
		ib_qrate = 1;
		ib_cal_ena = 1;
		ib1_tsdet = 3;
		ob_lev = 48;
		ob_ena_cas = 2;
		ob_ena1v_mode = 1;
		des_bw_ana = 3;
		pll_fsm_ctrl_data = 60;
		break;
	case PHY_INTERFACE_MODE_QSGMII:
		ib_if_mode = 3;
		ib1_tsdet = 16;
		ob_lev = 24;
		des_bw_ana = 5;
		pll_fsm_ctrl_data = 120;
		break;
	default:
		pr_err("Interface not supported\n");
		return;
	}

	if (interface == PHY_INTERFACE_MODE_QSGMII)
		writel(0xfff, base + HSIO_HW_CFGSTAT_HW_CFG);

	writel(HSIO_ANA_SERDES6G_COMMON_CFG_IF_MODE(3),
	       base + HSIO_ANA_SERDES6G_COMMON_CFG);
	writel(HSIO_ANA_SERDES6G_PLL_CFG_FSM_CTRL_DATA(120) |
	       HSIO_ANA_SERDES6G_PLL_CFG_ENA_OFFS(3),
	       base + HSIO_ANA_SERDES6G_PLL_CFG);
	writel(HSIO_ANA_SERDES6G_IB_CFG_REG_ENA |
	       HSIO_ANA_SERDES6G_IB_CFG_EQZ_ENA |
	       HSIO_ANA_SERDES6G_IB_CFG_SAM_ENA |
	       HSIO_ANA_SERDES6G_IB_CFG_CONCUR |
	       HSIO_ANA_SERDES6G_IB_CFG_SIG_DET_ENA |
	       HSIO_ANA_SERDES6G_IB_CFG_REG_PAT_SEL_OFF(0) |
	       HSIO_ANA_SERDES6G_IB_CFG_REG_PAT_SEL_LP(2) |
	       HSIO_ANA_SERDES6G_IB_CFG_REG_PAT_SEL_MID(1) |
	       HSIO_ANA_SERDES6G_IB_CFG_REG_PAT_SEL_HP(1) |
	       HSIO_ANA_SERDES6G_IB_CFG_SIG_DET_CLK_SEL(7) |
	       HSIO_ANA_SERDES6G_IB_CFG_TERM_MODE_SEL(1) |
	       HSIO_ANA_SERDES6G_IB_CFG_ICML_ADJ(5) |
	       HSIO_ANA_SERDES6G_IB_CFG_RTRM_ADJ(13) |
	       HSIO_ANA_SERDES6G_IB_CFG_VBULK_SEL |
	       HSIO_ANA_SERDES6G_IB_CFG_SOFSI(1),
	       base + HSIO_ANA_SERDES6G_IB_CFG);
	writel(HSIO_ANA_SERDES6G_IB_CFG1_FILT_OFFSET |
	       HSIO_ANA_SERDES6G_IB_CFG1_FILT_LP |
	       HSIO_ANA_SERDES6G_IB_CFG1_FILT_MID |
	       HSIO_ANA_SERDES6G_IB_CFG1_FILT_HP |
	       HSIO_ANA_SERDES6G_IB_CFG1_SCALY(15) |
	       HSIO_ANA_SERDES6G_IB_CFG1_TSDET(3) |
	       HSIO_ANA_SERDES6G_IB_CFG1_TJTAG(8),
	       base + HSIO_ANA_SERDES6G_IB_CFG1);
	writel(HSIO_DIG_SERDES6G_MISC_CFG_LANE_RST,
	       base + HSIO_DIG_SERDES6G_MISC_CFG);

	serdes6g_write(base, addr);

	writel(HSIO_ANA_SERDES6G_IB_CFG_REG_ENA |
	       HSIO_ANA_SERDES6G_IB_CFG_EQZ_ENA |
	       HSIO_ANA_SERDES6G_IB_CFG_SAM_ENA |
	       HSIO_ANA_SERDES6G_IB_CFG_CONCUR |
	       HSIO_ANA_SERDES6G_IB_CFG_SIG_DET_ENA |
	       HSIO_ANA_SERDES6G_IB_CFG_REG_PAT_SEL_OFF(0) |
	       HSIO_ANA_SERDES6G_IB_CFG_REG_PAT_SEL_LP(2) |
	       HSIO_ANA_SERDES6G_IB_CFG_REG_PAT_SEL_MID(1) |
	       HSIO_ANA_SERDES6G_IB_CFG_REG_PAT_SEL_HP(1) |
	       HSIO_ANA_SERDES6G_IB_CFG_SIG_DET_CLK_SEL(0) |
	       HSIO_ANA_SERDES6G_IB_CFG_TERM_MODE_SEL(1) |
	       HSIO_ANA_SERDES6G_IB_CFG_ICML_ADJ(5) |
	       HSIO_ANA_SERDES6G_IB_CFG_RTRM_ADJ(13) |
	       HSIO_ANA_SERDES6G_IB_CFG_VBULK_SEL |
	       HSIO_ANA_SERDES6G_IB_CFG_SOFSI(1),
	       base + HSIO_ANA_SERDES6G_IB_CFG);
	writel(HSIO_ANA_SERDES6G_IB_CFG1_FILT_OFFSET |
	       HSIO_ANA_SERDES6G_IB_CFG1_FILT_LP |
	       HSIO_ANA_SERDES6G_IB_CFG1_FILT_MID |
	       HSIO_ANA_SERDES6G_IB_CFG1_FILT_HP |
	       HSIO_ANA_SERDES6G_IB_CFG1_SCALY(15) |
	       HSIO_ANA_SERDES6G_IB_CFG1_TSDET(16) |
	       HSIO_ANA_SERDES6G_IB_CFG1_TJTAG(8),
	       base + HSIO_ANA_SERDES6G_IB_CFG1);

	writel(0x0, base + HSIO_ANA_SERDES6G_SER_CFG);
	writel(HSIO_ANA_SERDES6G_COMMON_CFG_IF_MODE(ib_if_mode) |
	       HSIO_ANA_SERDES6G_COMMON_CFG_QRATE(ib_qrate) |
	       HSIO_ANA_SERDES6G_COMMON_CFG_ENA_LANE |
	       HSIO_ANA_SERDES6G_COMMON_CFG_SYS_RST,
	       base + HSIO_ANA_SERDES6G_COMMON_CFG);
	writel(HSIO_DIG_SERDES6G_MISC_CFG_LANE_RST,
	       base + HSIO_DIG_SERDES6G_MISC_CFG);

	writel(HSIO_ANA_SERDES6G_OB_CFG_RESISTOR_CTRL(1) |
	       HSIO_ANA_SERDES6G_OB_CFG_SR(7) |
	       HSIO_ANA_SERDES6G_OB_CFG_SR_H |
	       HSIO_ANA_SERDES6G_OB_CFG_ENA1V_MODE(ob_ena1v_mode) |
	       HSIO_ANA_SERDES6G_OB_CFG_POL, base + HSIO_ANA_SERDES6G_OB_CFG);
	writel(HSIO_ANA_SERDES6G_OB_CFG1_LEV(ob_lev) |
	       HSIO_ANA_SERDES6G_OB_CFG1_ENA_CAS(ob_ena_cas),
	       base + HSIO_ANA_SERDES6G_OB_CFG1);

	writel(HSIO_ANA_SERDES6G_DES_CFG_BW_ANA(des_bw_ana) |
	       HSIO_ANA_SERDES6G_DES_CFG_BW_HYST(5) |
	       HSIO_ANA_SERDES6G_DES_CFG_MBTR_CTRL(2) |
	       HSIO_ANA_SERDES6G_DES_CFG_PHS_CTRL(6),
	       base + HSIO_ANA_SERDES6G_DES_CFG);
	writel(HSIO_ANA_SERDES6G_PLL_CFG_FSM_CTRL_DATA(pll_fsm_ctrl_data) |
	       HSIO_ANA_SERDES6G_PLL_CFG_ENA_OFFS(3),
	       base + HSIO_ANA_SERDES6G_PLL_CFG);

	serdes6g_write(base, addr);

	/* set pll_fsm_ena = 1 */
	writel(HSIO_ANA_SERDES6G_PLL_CFG_FSM_ENA |
	       HSIO_ANA_SERDES6G_PLL_CFG_FSM_CTRL_DATA(pll_fsm_ctrl_data) |
	       HSIO_ANA_SERDES6G_PLL_CFG_ENA_OFFS(3),
	       base + HSIO_ANA_SERDES6G_PLL_CFG);

	serdes6g_write(base, addr);

	/* wait 20ms for pll bringup */
	mdelay(20);

	/* start IB calibration by setting ib_cal_ena and clearing lane_rst */
	writel(HSIO_ANA_SERDES6G_IB_CFG_REG_ENA |
	       HSIO_ANA_SERDES6G_IB_CFG_EQZ_ENA |
	       HSIO_ANA_SERDES6G_IB_CFG_SAM_ENA |
	       HSIO_ANA_SERDES6G_IB_CFG_CAL_ENA(ib_cal_ena) |
	       HSIO_ANA_SERDES6G_IB_CFG_CONCUR |
	       HSIO_ANA_SERDES6G_IB_CFG_SIG_DET_ENA |
	       HSIO_ANA_SERDES6G_IB_CFG_REG_PAT_SEL_OFF(0) |
	       HSIO_ANA_SERDES6G_IB_CFG_REG_PAT_SEL_LP(2) |
	       HSIO_ANA_SERDES6G_IB_CFG_REG_PAT_SEL_MID(1) |
	       HSIO_ANA_SERDES6G_IB_CFG_REG_PAT_SEL_HP(1) |
	       HSIO_ANA_SERDES6G_IB_CFG_SIG_DET_CLK_SEL(0) |
	       HSIO_ANA_SERDES6G_IB_CFG_TERM_MODE_SEL(1) |
	       HSIO_ANA_SERDES6G_IB_CFG_ICML_ADJ(5) |
	       HSIO_ANA_SERDES6G_IB_CFG_RTRM_ADJ(13) |
	       HSIO_ANA_SERDES6G_IB_CFG_VBULK_SEL |
	       HSIO_ANA_SERDES6G_IB_CFG_SOFSI(1),
	       base + HSIO_ANA_SERDES6G_IB_CFG);
	writel(0x0, base + HSIO_DIG_SERDES6G_MISC_CFG);

	serdes6g_write(base, addr);

	/* wait 60 for calibration */
	mdelay(60);

	/* set ib_tsdet and ib_reg_pat_sel_offset back to correct values */
	writel(HSIO_ANA_SERDES6G_IB_CFG_REG_ENA |
	       HSIO_ANA_SERDES6G_IB_CFG_EQZ_ENA |
	       HSIO_ANA_SERDES6G_IB_CFG_SAM_ENA |
	       HSIO_ANA_SERDES6G_IB_CFG_CAL_ENA(ib_cal_ena) |
	       HSIO_ANA_SERDES6G_IB_CFG_CONCUR |
	       HSIO_ANA_SERDES6G_IB_CFG_SIG_DET_ENA |
	       HSIO_ANA_SERDES6G_IB_CFG_REG_PAT_SEL_OFF(0) |
	       HSIO_ANA_SERDES6G_IB_CFG_REG_PAT_SEL_LP(2) |
	       HSIO_ANA_SERDES6G_IB_CFG_REG_PAT_SEL_MID(1) |
	       HSIO_ANA_SERDES6G_IB_CFG_REG_PAT_SEL_HP(1) |
	       HSIO_ANA_SERDES6G_IB_CFG_SIG_DET_CLK_SEL(7) |
	       HSIO_ANA_SERDES6G_IB_CFG_TERM_MODE_SEL(1) |
	       HSIO_ANA_SERDES6G_IB_CFG_ICML_ADJ(5) |
	       HSIO_ANA_SERDES6G_IB_CFG_RTRM_ADJ(13) |
	       HSIO_ANA_SERDES6G_IB_CFG_VBULK_SEL |
	       HSIO_ANA_SERDES6G_IB_CFG_SOFSI(1),
	       base + HSIO_ANA_SERDES6G_IB_CFG);
	writel(HSIO_ANA_SERDES6G_IB_CFG1_FILT_OFFSET |
	       HSIO_ANA_SERDES6G_IB_CFG1_FILT_LP |
	       HSIO_ANA_SERDES6G_IB_CFG1_FILT_MID |
	       HSIO_ANA_SERDES6G_IB_CFG1_FILT_HP |
	       HSIO_ANA_SERDES6G_IB_CFG1_SCALY(15) |
	       HSIO_ANA_SERDES6G_IB_CFG1_TSDET(ib1_tsdet) |
	       HSIO_ANA_SERDES6G_IB_CFG1_TJTAG(8),
	       base + HSIO_ANA_SERDES6G_IB_CFG1);

	serdes6g_write(base, addr);
}

static void serdes1g_write(void __iomem *base, u32 addr)
{
	u32 data;

	writel(HSIO_MCB_SERDES1G_CFG_WR_ONE_SHOT |
	       HSIO_MCB_SERDES1G_CFG_ADDR(addr),
	       base + HSIO_MCB_SERDES1G_CFG);

	do {
		data = readl(base + HSIO_MCB_SERDES1G_CFG);
	} while (data & HSIO_MCB_SERDES1G_CFG_WR_ONE_SHOT);
}

static void serdes1g_setup(void __iomem *base, uint32_t addr,
			   phy_interface_t interface)
{
	writel(0x0, base + HSIO_ANA_SERDES1G_SER_CFG);
	writel(0x0, base + HSIO_DIG_SERDES1G_TP_CFG);
	writel(0x0, base + HSIO_DIG_SERDES1G_DFT_CFG0);
	writel(HSIO_ANA_SERDES1G_OB_CFG_RESISTOR_CTRL(1) |
	       HSIO_ANA_SERDES1G_OB_CFG_VCM_CTRL(4) |
	       HSIO_ANA_SERDES1G_OB_CFG_CMM_BIAS_CTRL(2) |
	       HSIO_ANA_SERDES1G_OB_CFG_AMP_CTRL(12) |
	       HSIO_ANA_SERDES1G_OB_CFG_SLP(3),
	       base + HSIO_ANA_SERDES1G_OB_CFG);
	writel(HSIO_ANA_SERDES1G_IB_CFG_RESISTOR_CTRL(13) |
	       HSIO_ANA_SERDES1G_IB_CFG_EQ_GAIN(2) |
	       HSIO_ANA_SERDES1G_IB_CFG_ENA_OFFSET_COMP |
	       HSIO_ANA_SERDES1G_IB_CFG_ENA_DETLEV |
	       HSIO_ANA_SERDES1G_IB_CFG_ENA_CMV_TERM |
	       HSIO_ANA_SERDES1G_IB_CFG_DET_LEV(3) |
	       HSIO_ANA_SERDES1G_IB_CFG_ACJTAG_HYST(1),
	       base + HSIO_ANA_SERDES1G_IB_CFG);
	writel(HSIO_ANA_SERDES1G_DES_CFG_BW_HYST(7) |
	       HSIO_ANA_SERDES1G_DES_CFG_BW_ANA(6) |
	       HSIO_ANA_SERDES1G_DES_CFG_MBTR_CTRL(2) |
	       HSIO_ANA_SERDES1G_DES_CFG_PHS_CTRL(6),
	       base + HSIO_ANA_SERDES1G_DES_CFG);
	writel(HSIO_DIG_SERDES1G_MISC_CFG_LANE_RST,
	       base + HSIO_DIG_SERDES1G_MISC_CFG);
	writel(HSIO_ANA_SERDES1G_PLL_CFG_FSM_ENA |
	       HSIO_ANA_SERDES1G_PLL_CFG_FSM_CTRL_DATA(0xc8) |
	       HSIO_ANA_SERDES1G_PLL_CFG_ENA_RC_DIV2,
	       base + HSIO_ANA_SERDES1G_PLL_CFG);
	writel(HSIO_ANA_SERDES1G_COMMON_CFG_IF_MODE |
	       HSIO_ANA_SERDES1G_COMMON_CFG_ENA_LANE |
	       HSIO_ANA_SERDES1G_COMMON_CFG_SYS_RST,
	       base + HSIO_ANA_SERDES1G_COMMON_CFG);

	serdes1g_write(base, addr);

	setbits_le32(base + HSIO_ANA_SERDES1G_COMMON_CFG,
		     HSIO_ANA_SERDES1G_COMMON_CFG_SYS_RST);

	serdes1g_write(base, addr);

	clrbits_le32(base + HSIO_DIG_SERDES1G_MISC_CFG,
		     HSIO_DIG_SERDES1G_MISC_CFG_LANE_RST);

	serdes1g_write(base, addr);
}

static int ram_init(u32 val, void __iomem *addr)
{
	writel(val, addr);

	if (wait_for_bit_le32(addr, BIT(1), false, 2000, false)) {
		printf("Timeout in memory reset, reg = 0x%08x\n", val);
		return 1;
	}

	return 0;
}

static int jr2_switch_init(struct jr2_private *priv)
{
	/* Initialize memories */
	ram_init(0x3, priv->regs[QSYS] + QSYS_RAM_CTRL_RAM_INIT);
	ram_init(0x3, priv->regs[ASM] + ASM_RAM_CTRL_RAM_INIT);
	ram_init(0x3, priv->regs[ANA_AC] + ANA_AC_RAM_CTRL_RAM_INIT);
	ram_init(0x3, priv->regs[REW] + REW_RAM_CTRL_RAM_INIT);

	/* Reset counters */
	writel(0x1, priv->regs[ANA_AC] + ANA_AC_STAT_GLOBAL_CFG_PORT_RESET);
	writel(0x1, priv->regs[ASM] + ASM_CFG_STAT_CFG);

	/* Enable switch-core and queue system */
	writel(0x1, priv->regs[QSYS] + QSYS_SYSTEM_RESET_CFG);

	return 0;
}

static void jr2_switch_config(struct jr2_private *priv)
{
	writel(0x55555555, priv->regs[QSYS] + QSYS_CALCFG_CAL_AUTO(0));
	writel(0x55555555, priv->regs[QSYS] + QSYS_CALCFG_CAL_AUTO(1));
	writel(0x55555555, priv->regs[QSYS] + QSYS_CALCFG_CAL_AUTO(2));
	writel(0x55555555, priv->regs[QSYS] + QSYS_CALCFG_CAL_AUTO(3));

	writel(readl(priv->regs[QSYS] + QSYS_CALCFG_CAL_CTRL) |
	       QSYS_CALCFG_CAL_CTRL_CAL_MODE(8),
	       priv->regs[QSYS] + QSYS_CALCFG_CAL_CTRL);
}

static int jr2_initialize(struct jr2_private *priv)
{
	int ret, i;

	/* Initialize switch memories, enable core */
	ret = jr2_switch_init(priv);
	if (ret)
		return ret;

	jr2_switch_config(priv);

	for (i = 0; i < MAX_PORT; i++)
		jr2_port_init(priv, i);

	jr2_cpu_capture_setup(priv);

	return 0;
}

static inline int jr2_vlant_wait_for_completion(struct jr2_private *priv)
{
	if (wait_for_bit_le32(priv->regs[LRN] + LRN_COMMON_ACCESS_CTRL,
			      LRN_COMMON_ACCESS_CTRL_MAC_TABLE_ACCESS_SHOT,
			      false, 2000, false))
		return -ETIMEDOUT;

	return 0;
}

static int jr2_mac_table_add(struct jr2_private *priv,
			     const unsigned char mac[ETH_ALEN], int pgid)
{
	u32 macl = 0, mach = 0;

	/*
	 * Set the MAC address to handle and the vlan associated in a format
	 * understood by the hardware.
	 */
	mach |= MAC_VID << 16;
	mach |= ((u32)mac[0]) << 8;
	mach |= ((u32)mac[1]) << 0;
	macl |= ((u32)mac[2]) << 24;
	macl |= ((u32)mac[3]) << 16;
	macl |= ((u32)mac[4]) << 8;
	macl |= ((u32)mac[5]) << 0;

	writel(mach, priv->regs[LRN] + LRN_COMMON_MAC_ACCESS_CFG0);
	writel(macl, priv->regs[LRN] + LRN_COMMON_MAC_ACCESS_CFG1);

	writel(LRN_COMMON_MAC_ACCESS_CFG2_MAC_ENTRY_ADDR(pgid) |
	       LRN_COMMON_MAC_ACCESS_CFG2_MAC_ENTRY_TYPE(0x3) |
	       LRN_COMMON_MAC_ACCESS_CFG2_MAC_ENTRY_CPU_COPY |
	       LRN_COMMON_MAC_ACCESS_CFG2_MAC_ENTRY_CPU_QU(0) |
	       LRN_COMMON_MAC_ACCESS_CFG2_MAC_ENTRY_VLD |
	       LRN_COMMON_MAC_ACCESS_CFG2_MAC_ENTRY_LOCKED,
	       priv->regs[LRN] + LRN_COMMON_MAC_ACCESS_CFG2);

	writel(LRN_COMMON_ACCESS_CTRL_MAC_TABLE_ACCESS_SHOT,
	       priv->regs[LRN] + LRN_COMMON_ACCESS_CTRL);

	return jr2_vlant_wait_for_completion(priv);
}

static int jr2_write_hwaddr(struct udevice *dev)
{
	struct jr2_private *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_platdata(dev);

	return jr2_mac_table_add(priv, pdata->enetaddr, PGID_UNICAST);
}

static void serdes_setup(struct jr2_private *priv)
{
	size_t mask;
	int i = 0;

	for (i = 0; i < MAX_PORT; ++i) {
		if (!priv->ports[i].bus || priv->ports[i].serdes_index == 0xff)
			continue;

		mask = BIT(priv->ports[i].serdes_index);
		if (priv->ports[i].serdes_index < SERDES1G_MAX) {
			serdes1g_setup(priv->regs[HSIO], mask,
				       priv->ports[i].phy_mode);
		} else {
			mask >>= SERDES6G(0);
			serdes6g_setup(priv->regs[HSIO], mask,
				       priv->ports[i].phy_mode);
		}
	}
}

static int jr2_start(struct udevice *dev)
{
	struct jr2_private *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_platdata(dev);
	const unsigned char mac[ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff,
		0xff };
	int ret;

	ret = jr2_initialize(priv);
	if (ret)
		return ret;

	/* Set MAC address tables entries for CPU redirection */
	ret = jr2_mac_table_add(priv, mac, PGID_BROADCAST);
	if (ret)
		return ret;

	ret = jr2_mac_table_add(priv, pdata->enetaddr, PGID_UNICAST);
	if (ret)
		return ret;

	serdes_setup(priv);

	return 0;
}

static void jr2_stop(struct udevice *dev)
{
}

static int jr2_send(struct udevice *dev, void *packet, int length)
{
	struct jr2_private *priv = dev_get_priv(dev);
	u32 ifh[IFH_LEN];
	u32 *buf = packet;

	memset(ifh, '\0', IFH_LEN);

	/* Set DST PORT_MASK */
	ifh[0] = htonl(0);
	ifh[1] = htonl(0x1FFFFF);
	ifh[2] = htonl(~0);
	/* Set DST_MODE to INJECT and UPDATE_FCS */
	ifh[5] = htonl(0x4c0);

	return mscc_send(priv->regs[QS], jr2_regs_qs,
			 ifh, IFH_LEN, buf, length);
}

static int jr2_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct jr2_private *priv = dev_get_priv(dev);
	u32 *rxbuf = (u32 *)net_rx_packets[0];
	int byte_cnt = 0;

	byte_cnt = mscc_recv(priv->regs[QS], jr2_regs_qs, rxbuf, IFH_LEN,
			     false);

	*packetp = net_rx_packets[0];

	return byte_cnt;
}

static struct mii_dev *get_mdiobus(phys_addr_t base, unsigned long size)
{
	int i = 0;

	for (i = 0; i < JR2_MIIM_BUS_COUNT; ++i)
		if (miim[i].miim_base == base && miim[i].miim_size == size)
			return miim[i].bus;

	return NULL;
}

static void add_port_entry(struct jr2_private *priv, size_t index,
			   size_t phy_addr, struct mii_dev *bus,
			   u8 serdes_index, u8 phy_mode)
{
	priv->ports[index].phy_addr = phy_addr;
	priv->ports[index].bus = bus;
	priv->ports[index].serdes_index = serdes_index;
	priv->ports[index].phy_mode = phy_mode;
}

static int jr2_probe(struct udevice *dev)
{
	struct jr2_private *priv = dev_get_priv(dev);
	int i;
	int ret;
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
	memset(&miim, 0x0, sizeof(struct jr2_miim_dev) * JR2_MIIM_BUS_COUNT);

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
				jr2_mdiobus_init(addr_base, addr_size);

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

static int jr2_remove(struct udevice *dev)
{
	struct jr2_private *priv = dev_get_priv(dev);
	int i;

	for (i = 0; i < JR2_MIIM_BUS_COUNT; i++) {
		mdio_unregister(priv->bus[i]);
		mdio_free(priv->bus[i]);
	}

	return 0;
}

static const struct eth_ops jr2_ops = {
	.start        = jr2_start,
	.stop         = jr2_stop,
	.send         = jr2_send,
	.recv         = jr2_recv,
	.write_hwaddr = jr2_write_hwaddr,
};

static const struct udevice_id mscc_jr2_ids[] = {
	{.compatible = "mscc,vsc7454-switch" },
	{ /* Sentinel */ }
};

U_BOOT_DRIVER(jr2) = {
	.name				= "jr2-switch",
	.id				= UCLASS_ETH,
	.of_match			= mscc_jr2_ids,
	.probe				= jr2_probe,
	.remove				= jr2_remove,
	.ops				= &jr2_ops,
	.priv_auto_alloc_size		= sizeof(struct jr2_private),
	.platdata_auto_alloc_size	= sizeof(struct eth_pdata),
};
