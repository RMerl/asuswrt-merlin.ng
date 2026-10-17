// SPDX-License-Identifier: GPL-2.0+
/*
 * Driver for Renesas Ethernet RSwitch2 (Ethernet-TSN).
 *
 * Copyright (C) 2021 Renesas Electronics Corporation
 *
 * Based on the Renesas Ethernet AVB driver.
 */

#include <asm/io.h>
#include <clk.h>
#include <common.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/device_compat.h>
#include <dm/lists.h>
#include <errno.h>
#include <generic-phy.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/iopoll.h>
#include <linux/mii.h>
#include <eth_phy.h>
#include <log.h>
#include <malloc.h>
#include <miiphy.h>

#define RSWITCH_SLEEP_US	1000
#define RSWITCH_TIMEOUT_US	1000000

#define RSWITCH_NUM_HW		5

#define ETHA_TO_GWCA(i)		((i) % 2)
#define GWCA_TO_HW_INDEX(i)	((i) + 3)
#define HW_INDEX_TO_GWCA(i)	((i) - 3)

#define RSWITCH_MAX_CTAG_PCP	7

/* Registers */
#define RSWITCH_COMA_OFFSET     0x00009000
#define RSWITCH_ETHA_OFFSET     0x0000a000      /* with RMAC */
#define RSWITCH_ETHA_SIZE       0x00002000      /* with RMAC */
#define RSWITCH_GWCA_OFFSET	0x00010000
#define RSWITCH_GWCA_SIZE	0x00002000

#define FWRO    0
#define CARO    RSWITCH_COMA_OFFSET
#define GWRO    0
#define TARO    0
#define RMRO    0x1000

enum rswitch_reg {
	EAMC		= TARO + 0x0000,
	EAMS		= TARO + 0x0004,
	EATDQDC		= TARO + 0x0060,
	EATTFC          = TARO + 0x0138,
	EATASRIRM	= TARO + 0x03E4,

	GWMC		= GWRO + 0x0000,
	GWMS		= GWRO + 0x0004,
	GWMTIRM		= GWRO + 0x0100,
	GWVCC		= GWRO + 0x0130,
	GWTTFC		= GWRO + 0x0138,
	GWDCBAC0	= GWRO + 0x0194,
	GWDCBAC1	= GWRO + 0x0198,
	GWTRC		= GWRO + 0x0200,
	GWARIRM		= GWRO + 0x0380,
	GWDCC		= GWRO + 0x0400,

	RRC		= CARO + 0x0004,
	RCEC		= CARO + 0x0008,
	RCDC		= CARO + 0x000C,
	CABPIRM		= CARO + 0x0140,

	FWPC0		= FWRO + 0x0100,
	FWPBFC		= FWRO + 0x4A00,
	FWPBFCSDC	= FWRO + 0x4A04,

	MPSM		= RMRO + 0x0000,
	MPIC		= RMRO + 0x0004,
	MRMAC0		= RMRO + 0x0084,
	MRMAC1		= RMRO + 0x0088,
	MRAFC		= RMRO + 0x008C,
	MRSCE		= RMRO + 0x0090,
	MRSCP		= RMRO + 0x0094,
	MLVC		= RMRO + 0x0180,
	MLBC		= RMRO + 0x0188,
	MXGMIIC		= RMRO + 0x0190,
	MPCH		= RMRO + 0x0194,
	MANM		= RMRO + 0x019C,
	MMIS0		= RMRO + 0x0210,
	MMIS1		= RMRO + 0x0220,
};

/* COMA */
#define RRC_RR		BIT(0)
#define RCEC_RCE	BIT(16)

#define CABPIRM_BPIOG	BIT(0)
#define CABPIRM_BPR	BIT(1)

/* MFWD */
#define FWPC0(i)	(FWPC0 + (i) * 0x10)
#define FWPC0_LTHTA     BIT(0)
#define FWPC0_IP4UE     BIT(3)
#define FWPC0_IP4TE     BIT(4)
#define FWPC0_IP4OE     BIT(5)
#define FWPC0_L2SE      BIT(9)
#define FWPC0_IP4EA     BIT(10)
#define FWPC0_IPDSA     BIT(12)
#define FWPC0_IPHLA     BIT(18)
#define FWPC0_MACSDA    BIT(20)
#define FWPC0_MACHLA    BIT(26)
#define FWPC0_MACHMA    BIT(27)
#define FWPC0_VLANSA    BIT(28)

#define FWPC0_DEFAULT   (FWPC0_LTHTA | FWPC0_IP4UE | FWPC0_IP4TE | \
			 FWPC0_IP4OE | FWPC0_L2SE | FWPC0_IP4EA | \
			 FWPC0_IPDSA | FWPC0_IPHLA | FWPC0_MACSDA | \
			 FWPC0_MACHLA | FWPC0_MACHMA | FWPC0_VLANSA)

#define FWPBFC(i)	(FWPBFC + (i) * 0x10)
#define FWPBFCSDC(j, i)	(FWPBFCSDC + (i) * 0x10 + (j) * 0x04)

/* ETHA */
#define EATASRIRM_TASRIOG	BIT(0)
#define EATASRIRM_TASRR		BIT(1)
#define EATDQDC(q)		(EATDQDC + (q) * 0x04)
#define EATDQDC_DQD		(0xff)

/* RMAC */
#define MPIC_PIS_GMII		0x02
#define MPIC_LSC_MASK		(0x07 << 3)
#define MPIC_LSC_100		(0x01 << 3)
#define MPIC_LSC_1000		(0x02 << 3)
#define MPIC_LSC_2500		(0x03 << 3)
#define MLVC_PLV		BIT(16)
#define MLVC_LVT		0x09
#define MMIS0_LVSS		0x02

#define MPIC_PSMCS_MASK		(0x7f << 16)
#define MPIC_PSMHT_MASK		(0x06 << 24)
#define MPIC_MDC_CLK_SET	(0x06050000)

#define MPSM_MFF_C45		BIT(2)
#define MPSM_MFF_C22		0x0
#define MPSM_PSME		BIT(0)

#define MDIO_READ_C45		0x03
#define MDIO_WRITE_C45		0x01
#define MDIO_ADDR_C45		0x00

#define MDIO_READ_C22           0x02
#define MDIO_WRITE_C22          0x01

#define MPSM_POP_MASK		(0x03 << 13)
#define MPSM_PRA_MASK		(0x1f << 8)
#define MPSM_PDA_MASK		(0x1f << 3)
#define MPSM_PRD_MASK		(0xffff << 16)

/* Completion flags */
#define MMIS1_PAACS		BIT(2) /* Address */
#define MMIS1_PWACS		BIT(1) /* Write */
#define MMIS1_PRACS		BIT(0) /* Read */
#define MMIS1_CLEAR_FLAGS	0xf

/* ETHA */
enum rswitch_etha_mode {
	EAMC_OPC_RESET,
	EAMC_OPC_DISABLE,
	EAMC_OPC_CONFIG,
	EAMC_OPC_OPERATION,
};

#define EAMS_OPS_MASK	EAMC_OPC_OPERATION

/* GWCA */
enum rswitch_gwca_mode {
	GWMC_OPC_RESET,
	GWMC_OPC_DISABLE,
	GWMC_OPC_CONFIG,
	GWMC_OPC_OPERATION,
};

#define GWMS_OPS_MASK	GWMC_OPC_OPERATION

#define GWMTIRM_MTIOG		BIT(0)
#define GWMTIRM_MTR		BIT(1)
#define GWARIRM_ARIOG		BIT(0)
#define GWARIRM_ARR		BIT(1)
#define GWVCC_VEM_SC_TAG	(0x3 << 16)
#define GWDCBAC0_DCBAUP		(0xff)
#define GWTRC(i)		(GWTRC + (i) * 0x04)
#define GWDCC(i)		(GWDCC + (i) * 0x04)
#define	GWDCC_DQT		BIT(11)
#define GWDCC_BALR		BIT(24)

struct rswitch_etha {
	int			index;
	void __iomem		*addr;
	struct phy_device	*phydev;
	struct mii_dev		*bus;
	unsigned char		*enetaddr;
};

struct rswitch_gwca {
	int			index;
	void __iomem		*addr;
	int			num_chain;
};

/* Setting value */
#define LINK_SPEED_100		100
#define LINK_SPEED_1000		1000
#define LINK_SPEED_2500		2500

/* Decriptor */
#define RSWITCH_NUM_BASE_DESC		2
#define RSWITCH_TX_CHAIN_INDEX		0
#define RSWITCH_RX_CHAIN_INDEX		1
#define RSWITCH_NUM_TX_DESC		8
#define RSWITCH_NUM_RX_DESC		8

enum RX_DS_CC_BIT {
	RX_DS   = 0x0fff, /* Data size */
	RX_TR   = 0x1000, /* Truncation indication */
	RX_EI   = 0x2000, /* Error indication */
	RX_PS   = 0xc000, /* Padding selection */
};

enum DIE_DT {
	/* Frame data */
	DT_FSINGLE      = 0x80,
	DT_FSTART       = 0x90,
	DT_FMID         = 0xa0,
	DT_FEND         = 0xb8,

	/* Chain control */
	DT_LEMPTY       = 0xc0,
	DT_EEMPTY       = 0xd0,
	DT_LINKFIX      = 0x00,
	DT_LINK         = 0xe0,
	DT_EOS          = 0xf0,
	/* HW/SW arbitration */
	DT_FEMPTY       = 0x40,
	DT_FEMPTY_IS    = 0x10,
	DT_FEMPTY_IC    = 0x20,
	DT_FEMPTY_ND    = 0x38,
	DT_FEMPTY_START = 0x50,
	DT_FEMPTY_MID   = 0x60,
	DT_FEMPTY_END   = 0x70,

	DT_MASK         = 0xf0,
	DIE             = 0x08, /* Descriptor Interrupt Enable */
};

struct rswitch_desc {
	__le16 info_ds; /* Descriptor size */
	u8 die_dt;      /* Descriptor interrupt enable and type */
	__u8  dptrh;    /* Descriptor pointer MSB */
	__le32 dptrl;   /* Descriptor pointer LSW */
} __packed;

struct rswitch_rxdesc {
	struct rswitch_desc	data;
	struct rswitch_desc	link;
	u8			__pad[48];
	u8			packet[PKTSIZE_ALIGN];
} __packed;

struct rswitch_port_priv {
	void __iomem		*addr;
	struct phy		serdes;
	struct rswitch_etha	etha;
	struct rswitch_gwca	gwca;
	struct rswitch_desc	bat_desc[RSWITCH_NUM_BASE_DESC];
	struct rswitch_desc	tx_desc[RSWITCH_NUM_TX_DESC];
	struct rswitch_rxdesc	rx_desc[RSWITCH_NUM_RX_DESC];
	u32			rx_desc_index;
	u32			tx_desc_index;
};

struct rswitch_priv {
	void __iomem		*addr;
	struct clk		*rsw_clk;
};

static inline void rswitch_flush_dcache(u32 addr, u32 len)
{
	flush_dcache_range(addr, addr + len);
}

static inline void rswitch_invalidate_dcache(u32 addr, u32 len)
{
	u32 start = addr & ~((uintptr_t)ARCH_DMA_MINALIGN - 1);
	u32 end = roundup(addr + len, ARCH_DMA_MINALIGN);

	invalidate_dcache_range(start, end);
}

static void rswitch_agent_clock_ctrl(struct rswitch_port_priv *priv, int port, int enable)
{
	u32 val;

	if (enable) {
		val = readl(priv->addr + RCEC);
		if ((val & (RCEC_RCE | BIT(port))) != (RCEC_RCE | BIT(port)))
			writel(val | RCEC_RCE | BIT(port), priv->addr + RCEC);
	} else {
		setbits_le32(priv->addr + RCDC, BIT(port));
	}
}

static int rswitch_etha_change_mode(struct rswitch_port_priv *priv,
				    enum rswitch_etha_mode mode)
{
	struct rswitch_etha *etha = &priv->etha;
	u32 pval;
	int ret;

	/* Enable clock */
	rswitch_agent_clock_ctrl(priv, etha->index, 1);

	writel(mode, etha->addr + EAMC);

	ret = readl_poll_sleep_timeout(etha->addr + EAMS, pval,
				       (pval & EAMS_OPS_MASK) == mode,
				       RSWITCH_SLEEP_US, RSWITCH_TIMEOUT_US);

	/* Disable clock */
	if (mode == EAMC_OPC_DISABLE)
		rswitch_agent_clock_ctrl(priv, etha->index, 0);

	return ret;
}

static int rswitch_gwca_change_mode(struct rswitch_port_priv *priv,
				    enum rswitch_gwca_mode mode)
{
	struct rswitch_gwca *gwca = &priv->gwca;
	u32 pval;
	int ret;

	/* Enable clock */
	rswitch_agent_clock_ctrl(priv, gwca->index, 1);

	writel(mode, gwca->addr + GWMC);

	ret = readl_poll_sleep_timeout(gwca->addr + GWMS, pval,
				       (pval & GWMS_OPS_MASK) == mode,
				       RSWITCH_SLEEP_US, RSWITCH_TIMEOUT_US);

	/* Disable clock */
	if (mode == GWMC_OPC_DISABLE)
		rswitch_agent_clock_ctrl(priv, gwca->index, 0);

	return ret;
}

static int rswitch_mii_access_c45(struct rswitch_etha *etha, bool read,
				  int phyad, int devad, int regad, int data)
{
	u32 pval, val;
	int ret;

	/* No match device */
	if (devad == 0xffffffff)
		return 0;

	/* Clear completion flags */
	writel(MMIS1_CLEAR_FLAGS, etha->addr + MMIS1);

	/* Submit address to PHY (MDIO_ADDR_C45 << 13) */
	val = MPSM_PSME | MPSM_MFF_C45 | (devad << 8) | (phyad << 3);
	writel((regad << 16) | val, etha->addr + MPSM);

	ret = readl_poll_sleep_timeout(etha->addr + MMIS1, pval,
				       pval & MMIS1_PAACS,
				       RSWITCH_SLEEP_US, RSWITCH_TIMEOUT_US);
	if (ret)
		return ret;

	/* Clear address completion flag */
	setbits_le32(etha->addr + MMIS1, MMIS1_PAACS);

	/* Read/Write PHY register */
	if (read) {
		val |= MDIO_READ_C45 << 13;
		writel(val, etha->addr + MPSM);

		ret = readl_poll_sleep_timeout(etha->addr + MMIS1, pval,
					       pval & MMIS1_PRACS,
					       RSWITCH_SLEEP_US,
					       RSWITCH_TIMEOUT_US);
		if (ret)
			return ret;

		/* Read data */
		ret = (readl(etha->addr + MPSM) & MPSM_PRD_MASK) >> 16;

		/* Clear read completion flag */
		setbits_le32(etha->addr + MMIS1, MMIS1_PRACS);
	} else {
		val |= MDIO_WRITE_C45 << 13;
		val |= data << 16;
		writel(val, etha->addr + MPSM);

		ret = readl_poll_sleep_timeout(etha->addr + MMIS1, pval,
					       pval & MMIS1_PWACS,
					       RSWITCH_SLEEP_US,
					       RSWITCH_TIMEOUT_US);
	}

	return ret;
}

static int rswitch_mii_read_c45(struct mii_dev *miidev, int phyad, int devad, int regad)
{
	struct rswitch_port_priv *priv = miidev->priv;
	struct rswitch_etha *etha = &priv->etha;
	int val;
	int reg;

	/* Change to disable mode */
	rswitch_etha_change_mode(priv, EAMC_OPC_DISABLE);

	/* Change to config mode */
	rswitch_etha_change_mode(priv, EAMC_OPC_CONFIG);

	/* Enable Station Management clock */
	reg = readl(etha->addr + MPIC);
	reg &= ~MPIC_PSMCS_MASK & ~MPIC_PSMHT_MASK;
	writel(reg | MPIC_MDC_CLK_SET, etha->addr + MPIC);

	/* Set Station Management Mode : Clause 45 */
	setbits_le32(etha->addr + MPSM, MPSM_MFF_C45);

	/* Access PHY register */
	val = rswitch_mii_access_c45(etha, true, phyad, devad, regad, 0);

	/* Disable Station Management Clock */
	clrbits_le32(etha->addr + MPIC, MPIC_PSMCS_MASK);

	/* Change to disable mode */
	rswitch_etha_change_mode(priv, EAMC_OPC_DISABLE);

	return val;
}

int rswitch_mii_write_c45(struct mii_dev *miidev, int phyad, int devad, int regad, u16 data)
{
	struct rswitch_port_priv *priv = miidev->priv;
	struct rswitch_etha *etha = &priv->etha;
	int reg;

	/* Change to disable mode */
	rswitch_etha_change_mode(priv, EAMC_OPC_DISABLE);

	/* Change to config mode */
	rswitch_etha_change_mode(priv, EAMC_OPC_CONFIG);

	/* Enable Station Management clock */
	reg = readl(etha->addr + MPIC);
	reg &= ~MPIC_PSMCS_MASK & ~MPIC_PSMHT_MASK;
	writel(reg | MPIC_MDC_CLK_SET, etha->addr + MPIC);

	/* Set Station Management Mode : Clause 45 */
	setbits_le32(etha->addr + MPSM, MPSM_MFF_C45);

	/* Access PHY register */
	rswitch_mii_access_c45(etha, false, phyad, devad, regad, data);

	/* Disable Station Management Clock */
	clrbits_le32(etha->addr + MPIC, MPIC_PSMCS_MASK);

	/* Change to disable mode */
	rswitch_etha_change_mode(priv, EAMC_OPC_DISABLE);

	return 0;
}

static int rswitch_check_link(struct rswitch_etha *etha)
{
	u32 pval;
	int ret;

	/* Request Link Verification */
	writel(MLVC_PLV, etha->addr + MLVC);

	/* Complete Link Verification */
	ret = readl_poll_sleep_timeout(etha->addr + MLVC, pval,
				       !(pval & MLVC_PLV),
				       RSWITCH_SLEEP_US, RSWITCH_TIMEOUT_US);
	if (ret) {
		debug("\n%s: Link verification timeout!", __func__);
		return ret;
	}

	return 0;
}

static int rswitch_reset(struct rswitch_port_priv *priv)
{
	int ret;

	setbits_le32(priv->addr + RRC, RRC_RR);
	clrbits_le32(priv->addr + RRC, RRC_RR);

	ret = rswitch_gwca_change_mode(priv, GWMC_OPC_DISABLE);
	if (ret)
		return ret;

	ret = rswitch_etha_change_mode(priv, EAMC_OPC_DISABLE);
	if (ret)
		return ret;

	return 0;
}

static void rswitch_bat_desc_init(struct rswitch_port_priv *priv)
{
	const u32 desc_size = RSWITCH_NUM_BASE_DESC * sizeof(struct rswitch_desc);
	int i;

	/* Initialize all descriptors */
	memset(priv->bat_desc, 0x0, desc_size);

	for (i = 0; i < RSWITCH_NUM_BASE_DESC; i++)
		priv->bat_desc[i].die_dt = DT_EOS;

	rswitch_flush_dcache((uintptr_t)priv->bat_desc, desc_size);
}

static void rswitch_tx_desc_init(struct rswitch_port_priv *priv)
{
	const u32 desc_size = RSWITCH_NUM_TX_DESC * sizeof(struct rswitch_desc);
	u64 tx_desc_addr;
	int i;

	/* Initialize all descriptor */
	memset(priv->tx_desc, 0x0, desc_size);
	priv->tx_desc_index = 0;

	for (i = 0; i < RSWITCH_NUM_TX_DESC; i++)
		priv->tx_desc[i].die_dt = DT_EEMPTY;

	/* Mark the end of the descriptors */
	priv->tx_desc[RSWITCH_NUM_TX_DESC - 1].die_dt = DT_LINKFIX;
	tx_desc_addr = (uintptr_t)priv->tx_desc;
	priv->tx_desc[RSWITCH_NUM_TX_DESC - 1].dptrl = lower_32_bits(tx_desc_addr);
	priv->tx_desc[RSWITCH_NUM_TX_DESC - 1].dptrh = upper_32_bits(tx_desc_addr);
	rswitch_flush_dcache(tx_desc_addr, desc_size);

	/* Point the controller to the TX descriptor list */
	priv->bat_desc[RSWITCH_TX_CHAIN_INDEX].die_dt = DT_LINKFIX;
	priv->bat_desc[RSWITCH_TX_CHAIN_INDEX].dptrl = lower_32_bits(tx_desc_addr);
	priv->bat_desc[RSWITCH_TX_CHAIN_INDEX].dptrh = upper_32_bits(tx_desc_addr);
	rswitch_flush_dcache((uintptr_t)&priv->bat_desc[RSWITCH_TX_CHAIN_INDEX],
			     sizeof(struct rswitch_desc));
}

static void rswitch_rx_desc_init(struct rswitch_port_priv *priv)
{
	const u32 desc_size = RSWITCH_NUM_RX_DESC * sizeof(struct rswitch_rxdesc);
	int i;
	u64 packet_addr;
	u64 next_rx_desc_addr;
	u64 rx_desc_addr;

	/* Initialize all descriptor */
	memset(priv->rx_desc, 0x0, desc_size);
	priv->rx_desc_index = 0;

	for (i = 0; i < RSWITCH_NUM_RX_DESC; i++) {
		priv->rx_desc[i].data.die_dt = DT_EEMPTY;
		priv->rx_desc[i].data.info_ds = PKTSIZE_ALIGN;
		packet_addr = (uintptr_t)priv->rx_desc[i].packet;
		priv->rx_desc[i].data.dptrl = lower_32_bits(packet_addr);
		priv->rx_desc[i].data.dptrh = upper_32_bits(packet_addr);

		priv->rx_desc[i].link.die_dt = DT_LINKFIX;
		next_rx_desc_addr = (uintptr_t)&priv->rx_desc[i + 1];
		priv->rx_desc[i].link.dptrl = lower_32_bits(next_rx_desc_addr);
		priv->rx_desc[i].link.dptrh = upper_32_bits(next_rx_desc_addr);
	}

	/* Mark the end of the descriptors */
	priv->rx_desc[RSWITCH_NUM_RX_DESC - 1].link.die_dt = DT_LINKFIX;
	rx_desc_addr = (uintptr_t)priv->rx_desc;
	priv->rx_desc[RSWITCH_NUM_RX_DESC - 1].link.dptrl = lower_32_bits(rx_desc_addr);
	priv->rx_desc[RSWITCH_NUM_RX_DESC - 1].link.dptrh = upper_32_bits(rx_desc_addr);
	rswitch_flush_dcache(rx_desc_addr, desc_size);

	/* Point the controller to the rx descriptor list */
	priv->bat_desc[RSWITCH_RX_CHAIN_INDEX].die_dt = DT_LINKFIX;
	priv->bat_desc[RSWITCH_RX_CHAIN_INDEX].dptrl = lower_32_bits(rx_desc_addr);
	priv->bat_desc[RSWITCH_RX_CHAIN_INDEX].dptrh = upper_32_bits(rx_desc_addr);
	rswitch_flush_dcache((uintptr_t)&priv->bat_desc[RSWITCH_RX_CHAIN_INDEX],
			     sizeof(struct rswitch_desc));
}

static void rswitch_clock_enable(struct rswitch_port_priv *priv)
{
	struct rswitch_etha *etha = &priv->etha;
	struct rswitch_gwca *gwca = &priv->gwca;

	setbits_le32(priv->addr + RCEC, BIT(etha->index) | BIT(gwca->index) | RCEC_RCE);
}

static int rswitch_bpool_init(struct rswitch_port_priv *priv)
{
	u32 pval;

	writel(CABPIRM_BPIOG, priv->addr + CABPIRM);

	return readl_poll_sleep_timeout(priv->addr + CABPIRM, pval,
					pval & CABPIRM_BPR,
					RSWITCH_SLEEP_US, RSWITCH_TIMEOUT_US);
}

static void rswitch_mfwd_init(struct rswitch_port_priv *priv)
{
	struct rswitch_etha *etha = &priv->etha;
	struct rswitch_gwca *gwca = &priv->gwca;

	writel(FWPC0_DEFAULT, priv->addr + FWPC0(etha->index));
	writel(FWPC0_DEFAULT, priv->addr + FWPC0(gwca->index));

	writel(RSWITCH_RX_CHAIN_INDEX,
	       priv->addr + FWPBFCSDC(HW_INDEX_TO_GWCA(gwca->index), etha->index));

	writel(BIT(gwca->index),
	       priv->addr + FWPBFC(etha->index));

	writel(BIT(etha->index),
	       priv->addr + FWPBFC(gwca->index));
}

static void rswitch_rmac_init(struct rswitch_etha *etha)
{
	unsigned char *mac = etha->enetaddr;

	/* Set MAC address */
	writel((mac[2] << 24) | (mac[3] << 16) | (mac[4] << 8) | mac[5],
	       etha->addr + MRMAC1);

	writel((mac[0] << 8) | mac[1], etha->addr + MRMAC0);

	/* Set MIIx */
	writel(MPIC_PIS_GMII | MPIC_LSC_1000, etha->addr + MPIC);

	writel(0x07E707E7, etha->addr + MRAFC);
}

static int rswitch_gwca_mcast_table_reset(struct rswitch_gwca *gwca)
{
	u32 pval;

	writel(GWMTIRM_MTIOG, gwca->addr + GWMTIRM);

	return readl_poll_sleep_timeout(gwca->addr + GWMTIRM, pval,
					pval & GWMTIRM_MTR,
					RSWITCH_SLEEP_US, RSWITCH_TIMEOUT_US);
}

static int rswitch_gwca_axi_ram_reset(struct rswitch_gwca *gwca)
{
	u32 pval;

	writel(GWARIRM_ARIOG, gwca->addr + GWARIRM);

	return readl_poll_sleep_timeout(gwca->addr + GWARIRM, pval,
					pval & GWARIRM_ARR,
					RSWITCH_SLEEP_US, RSWITCH_TIMEOUT_US);
}

static int rswitch_gwca_init(struct rswitch_port_priv *priv)
{
	struct rswitch_gwca *gwca = &priv->gwca;
	int ret;

	ret = rswitch_gwca_change_mode(priv, GWMC_OPC_DISABLE);
	if (ret)
		return ret;

	ret = rswitch_gwca_change_mode(priv, GWMC_OPC_CONFIG);
	if (ret)
		return ret;

	ret = rswitch_gwca_mcast_table_reset(gwca);
	if (ret)
		return ret;

	ret = rswitch_gwca_axi_ram_reset(gwca);
	if (ret)
		return ret;

	/* Setting flow */
	writel(GWVCC_VEM_SC_TAG, gwca->addr + GWVCC);
	writel(0, gwca->addr + GWTTFC);
	writel(upper_32_bits((uintptr_t)priv->bat_desc) & GWDCBAC0_DCBAUP, gwca->addr + GWDCBAC0);
	writel(lower_32_bits((uintptr_t)priv->bat_desc), gwca->addr + GWDCBAC1);
	writel(GWDCC_DQT | GWDCC_BALR, gwca->addr + GWDCC(RSWITCH_TX_CHAIN_INDEX));
	writel(GWDCC_BALR, gwca->addr + GWDCC(RSWITCH_RX_CHAIN_INDEX));

	ret = rswitch_gwca_change_mode(priv, GWMC_OPC_DISABLE);
	if (ret)
		return ret;

	ret = rswitch_gwca_change_mode(priv, GWMC_OPC_OPERATION);
	if (ret)
		return ret;

	return 0;
}

static int rswitch_etha_tas_ram_reset(struct rswitch_etha *etha)
{
	u32 pval;

	writel(EATASRIRM_TASRIOG, etha->addr + EATASRIRM);

	return readl_poll_sleep_timeout(etha->addr + EATASRIRM, pval,
					pval & EATASRIRM_TASRR,
					RSWITCH_SLEEP_US, RSWITCH_TIMEOUT_US);
}

static int rswitch_etha_init(struct rswitch_port_priv *priv)
{
	struct rswitch_etha *etha = &priv->etha;
	int ret;
	u32 prio;

	ret = rswitch_etha_change_mode(priv, EAMC_OPC_DISABLE);
	if (ret)
		return ret;

	ret = rswitch_etha_change_mode(priv, EAMC_OPC_CONFIG);
	if (ret)
		return ret;

	ret = rswitch_etha_tas_ram_reset(etha);
	if (ret)
		return ret;

	/* Setting flow */
	writel(0, etha->addr + EATTFC);

	for (prio = 0; prio < RSWITCH_MAX_CTAG_PCP; prio++)
		writel(EATDQDC_DQD, etha->addr + EATDQDC(prio));

	rswitch_rmac_init(etha);

	ret = rswitch_etha_change_mode(priv, EAMC_OPC_OPERATION);
	if (ret)
		return ret;

	/* Link Verification */
	ret = rswitch_check_link(etha);
	if (ret)
		return ret;

	return 0;
}

static int rswitch_init(struct rswitch_port_priv *priv)
{
	struct rswitch_etha *etha = &priv->etha;
	int ret;

	ret = rswitch_reset(priv);
	if (ret)
		return ret;

	ret = generic_phy_set_mode(&priv->serdes, PHY_MODE_ETHERNET,
				   etha->phydev->interface);
	if (ret)
		return ret;

	ret = generic_phy_set_speed(&priv->serdes, etha->phydev->speed);
	if (ret)
		return ret;

	ret = generic_phy_init(&priv->serdes);
	if (ret)
		return ret;

	ret = generic_phy_power_on(&priv->serdes);
	if (ret)
		return ret;

	ret = phy_startup(etha->phydev);
	if (ret)
		return ret;

	rswitch_bat_desc_init(priv);
	rswitch_tx_desc_init(priv);
	rswitch_rx_desc_init(priv);

	rswitch_clock_enable(priv);

	ret = rswitch_bpool_init(priv);
	if (ret)
		return ret;

	rswitch_mfwd_init(priv);

	ret = rswitch_gwca_init(priv);
	if (ret)
		return ret;

	ret = rswitch_etha_init(priv);
	if (ret)
		return ret;

	return 0;
}

static int rswitch_start(struct udevice *dev)
{
	struct rswitch_port_priv *priv = dev_get_priv(dev);
	int ret;

	ret = rswitch_init(priv);
	if (ret)
		return ret;

	return 0;
}

#define RSWITCH_TX_TIMEOUT_MS	1000
static int rswitch_send(struct udevice *dev, void *packet, int len)
{
	struct rswitch_port_priv *priv = dev_get_priv(dev);
	struct rswitch_desc *desc = &priv->tx_desc[priv->tx_desc_index];
	struct rswitch_gwca *gwca = &priv->gwca;
	u32 gwtrc_index, start;

	/* Update TX descriptor */
	rswitch_flush_dcache((uintptr_t)packet, len);
	memset(desc, 0x0, sizeof(*desc));
	desc->die_dt = DT_FSINGLE;
	desc->info_ds = len;
	desc->dptrl = lower_32_bits((uintptr_t)packet);
	desc->dptrh = upper_32_bits((uintptr_t)packet);
	rswitch_flush_dcache((uintptr_t)desc, sizeof(*desc));

	/* Start transmission */
	gwtrc_index = RSWITCH_TX_CHAIN_INDEX / 32;
	setbits_le32(gwca->addr + GWTRC(gwtrc_index), BIT(RSWITCH_TX_CHAIN_INDEX));

	/* Wait until packet is transmitted */
	start = get_timer(0);
	while (get_timer(start) < RSWITCH_TX_TIMEOUT_MS) {
		rswitch_invalidate_dcache((uintptr_t)desc, sizeof(*desc));
		if ((desc->die_dt & DT_MASK) != DT_FSINGLE)
			break;
		udelay(10);
	}

	if (get_timer(start) >= RSWITCH_TX_TIMEOUT_MS) {
		dev_dbg(dev, "\n%s: Timeout", __func__);
		return -ETIMEDOUT;
	}

	priv->tx_desc_index = (priv->tx_desc_index + 1) % (RSWITCH_NUM_TX_DESC - 1);

	return 0;
}

static int rswitch_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct rswitch_port_priv *priv = dev_get_priv(dev);
	struct rswitch_rxdesc *desc = &priv->rx_desc[priv->rx_desc_index];
	u8 *packet;
	int len;

	/* Check if the rx descriptor is ready */
	rswitch_invalidate_dcache((uintptr_t)desc, sizeof(*desc));
	if ((desc->data.die_dt & DT_MASK) == DT_FEMPTY)
		return -EAGAIN;

	len = desc->data.info_ds & RX_DS;
	packet = (u8 *)(((uintptr_t)(desc->data.dptrh) << 32) | (uintptr_t)desc->data.dptrl);
	rswitch_invalidate_dcache((uintptr_t)packet, len);

	*packetp = packet;

	return len;
}

static int rswitch_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct rswitch_port_priv *priv = dev_get_priv(dev);
	struct rswitch_rxdesc *desc = &priv->rx_desc[priv->rx_desc_index];

	/* Make current descritor available again */
	desc->data.die_dt = DT_FEMPTY;
	desc->data.info_ds = PKTSIZE_ALIGN;
	rswitch_flush_dcache((uintptr_t)desc, sizeof(*desc));

	/* Point to the next descriptor */
	priv->rx_desc_index = (priv->rx_desc_index + 1) % RSWITCH_NUM_RX_DESC;
	desc = &priv->rx_desc[priv->rx_desc_index];
	rswitch_invalidate_dcache((uintptr_t)desc, sizeof(*desc));

	return 0;
}

static void rswitch_stop(struct udevice *dev)
{
	struct rswitch_port_priv *priv = dev_get_priv(dev);

	phy_shutdown(priv->etha.phydev);

	generic_phy_power_off(&priv->serdes);
}

static int rswitch_write_hwaddr(struct udevice *dev)
{
	struct rswitch_port_priv *priv = dev_get_priv(dev);
	struct rswitch_etha *etha = &priv->etha;
	struct eth_pdata *pdata = dev_get_plat(dev);
	unsigned char *mac = pdata->enetaddr;

	writel((mac[2] << 24) | (mac[3] << 16) | (mac[4] << 8) | mac[5],
	       etha->addr + MRMAC1);

	writel((mac[0] << 8) | mac[1], etha->addr + MRMAC0);

	return 0;
}

static int rswitch_phy_config(struct udevice *dev)
{
	struct rswitch_port_priv *priv = dev_get_priv(dev);
	struct rswitch_etha *etha = &priv->etha;
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct phy_device *phydev;
	int phy_addr;

	phy_addr = eth_phy_get_addr(dev);
	if (phy_addr < 0)
		return phy_addr;

	phydev = phy_connect(etha->bus, phy_addr, dev, pdata->phy_interface);
	if (!phydev)
		return -ENODEV;

	etha->phydev = phydev;
	phydev->speed = SPEED_1000;

	phy_config(phydev);

	return 0;
}

static int rswitch_port_probe(struct udevice *dev)
{
	struct rswitch_priv *rpriv =
		(struct rswitch_priv *)dev_get_driver_data(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct rswitch_port_priv *priv = dev_get_priv(dev);
	struct rswitch_etha *etha = &priv->etha;
	struct rswitch_gwca *gwca = &priv->gwca;
	struct mii_dev *mdiodev;
	int ret;

	priv->addr = rpriv->addr;

	etha->enetaddr = pdata->enetaddr;

	etha->index = dev_read_u32_default(dev, "reg", 0);
	etha->addr = priv->addr + RSWITCH_ETHA_OFFSET + etha->index * RSWITCH_ETHA_SIZE;

	gwca->index = 1;
	gwca->addr = priv->addr + RSWITCH_GWCA_OFFSET + gwca->index * RSWITCH_GWCA_SIZE;
	gwca->index = GWCA_TO_HW_INDEX(gwca->index);

	ret = generic_phy_get_by_index(dev, 0, &priv->serdes);
	if (ret)
		return ret;

	/* Toggle the reset so we can access the PHYs */
	ret = rswitch_reset(priv);
	if (ret)
		return ret;

	mdiodev = mdio_alloc();
	if (!mdiodev)
		return -ENOMEM;

	mdiodev->priv = priv;
	mdiodev->read = rswitch_mii_read_c45;
	mdiodev->write = rswitch_mii_write_c45;
	snprintf(mdiodev->name, sizeof(mdiodev->name), dev->name);

	ret = mdio_register(mdiodev);
	if (ret)
		goto err_mdio_register;

	priv->etha.bus = miiphy_get_dev_by_name(dev->name);

	ret = rswitch_phy_config(dev);
	if (ret)
		goto err_mdio_register;

	return 0;

err_mdio_register:
	mdio_free(mdiodev);
	return ret;
}

static int rswitch_port_remove(struct udevice *dev)
{
	struct rswitch_port_priv *priv = dev_get_priv(dev);

	mdio_unregister(priv->etha.bus);
	free(priv->etha.phydev);

	return 0;
}

int rswitch_ofdata_to_platdata(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);

	pdata->phy_interface = dev_read_phy_mode(dev);
	if (pdata->phy_interface == PHY_INTERFACE_MODE_NA)
		return -EINVAL;

	pdata->max_speed = dev_read_u32_default(dev, "max-speed", 1000);

	return 0;
}

static const struct eth_ops rswitch_port_ops = {
	.start		= rswitch_start,
	.send		= rswitch_send,
	.recv		= rswitch_recv,
	.free_pkt	= rswitch_free_pkt,
	.stop		= rswitch_stop,
	.write_hwaddr	= rswitch_write_hwaddr,
};

U_BOOT_DRIVER(rswitch_port) = {
	.name		= "rswitch-port",
	.id		= UCLASS_ETH,
	.of_to_plat	= rswitch_ofdata_to_platdata,
	.probe		= rswitch_port_probe,
	.remove		= rswitch_port_remove,
	.ops		= &rswitch_port_ops,
	.priv_auto	= sizeof(struct rswitch_port_priv),
	.plat_auto	= sizeof(struct eth_pdata),
	.flags		= DM_FLAG_ALLOC_PRIV_DMA | DM_FLAG_OS_PREPARE,
};

static int rswitch_probe(struct udevice *dev)
{
	struct rswitch_priv *priv = dev_get_plat(dev);
	fdt_addr_t secure_base;
	fdt_size_t size;
	int ret;

	secure_base = dev_read_addr_size_name(dev, "secure_base", &size);
	if (!secure_base)
		return -EINVAL;

	priv->addr = map_physmem(secure_base, size, MAP_NOCACHE);
	if (!priv->addr)
		return -EINVAL;

	priv->rsw_clk = devm_clk_get(dev, NULL);
	if (ret)
		goto err_map;

	ret = clk_prepare_enable(priv->rsw_clk);
	if (ret)
		goto err_map;

	return 0;

err_map:
	unmap_physmem(priv->addr, MAP_NOCACHE);
	return ret;
}

static int rswitch_remove(struct udevice *dev)
{
	struct rswitch_priv *priv = dev_get_plat(dev);

	clk_disable_unprepare(priv->rsw_clk);
	unmap_physmem(priv->addr, MAP_NOCACHE);

	return 0;
}

static int rswitch_bind(struct udevice *parent)
{
	struct rswitch_port_priv *priv = dev_get_plat(parent);
	ofnode ports_np, node;
	struct udevice *dev;
	struct driver *drv;
	int ret;

	drv = lists_driver_lookup_name("rswitch-port");
	if (!drv)
		return -ENOENT;

	ports_np = dev_read_subnode(parent, "ethernet-ports");
	if (!ofnode_valid(ports_np))
		return -ENOENT;

	ofnode_for_each_subnode(node, ports_np) {
		ret = device_bind_with_driver_data(parent, drv,
						   ofnode_get_name(node),
						   (ulong)priv, node, &dev);
		if (ret)
			return ret;
	}

	return 0;
}

static const struct udevice_id rswitch_ids[] = {
	{ .compatible = "renesas,r8a779f0-ether-switch" },
	{ }
};

U_BOOT_DRIVER(rswitch) = {
	.name		= "rswitch",
	.id		= UCLASS_NOP,
	.of_match	= rswitch_ids,
	.bind		= rswitch_bind,
	.probe		= rswitch_probe,
	.remove		= rswitch_remove,
	.plat_auto	= sizeof(struct rswitch_priv),
};
