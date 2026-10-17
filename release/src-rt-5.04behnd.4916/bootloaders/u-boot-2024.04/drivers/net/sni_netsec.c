// SPDX-License-Identifier: GPL-2.0+
/**
 * netsec.c - Socionext Synquacer Netsec driver
 * Copyright 2021 Linaro Ltd.
 */

#include <clk.h>
#include <cpu_func.h>
#include <dm.h>
#include <fdt_support.h>
#include <log.h>
#include <malloc.h>
#include <miiphy.h>
#include <net.h>
#include <regmap.h>
#include <reset.h>
#include <syscon.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <spi.h>
#include <spi_flash.h>

#define NETSEC_REG_SOFT_RST			0x104
#define NETSEC_REG_COM_INIT			0x120

#define NETSEC_REG_TOP_STATUS			0x200
#define NETSEC_IRQ_RX				BIT(1)
#define NETSEC_IRQ_TX				BIT(0)

#define NETSEC_REG_TOP_INTEN			0x204
#define NETSEC_REG_INTEN_SET			0x234
#define NETSEC_REG_INTEN_CLR			0x238

#define NETSEC_REG_NRM_TX_STATUS		0x400
#define NETSEC_REG_NRM_TX_INTEN			0x404
#define NETSEC_REG_NRM_TX_INTEN_SET		0x428
#define NETSEC_REG_NRM_TX_INTEN_CLR		0x42c
#define NRM_TX_ST_NTOWNR	BIT(17)
#define NRM_TX_ST_TR_ERR	BIT(16)
#define NRM_TX_ST_TXDONE	BIT(15)
#define NRM_TX_ST_TMREXP	BIT(14)

#define NETSEC_REG_NRM_RX_STATUS		0x440
#define NETSEC_REG_NRM_RX_INTEN			0x444
#define NETSEC_REG_NRM_RX_INTEN_SET		0x468
#define NETSEC_REG_NRM_RX_INTEN_CLR		0x46c
#define NRM_RX_ST_RC_ERR	BIT(16)
#define NRM_RX_ST_PKTCNT	BIT(15)
#define NRM_RX_ST_TMREXP	BIT(14)

#define NETSEC_REG_PKT_CMD_BUF			0xd0

#define NETSEC_REG_CLK_EN			0x100

#define NETSEC_REG_PKT_CTRL			0x140

#define NETSEC_REG_DMA_TMR_CTRL			0x20c
#define NETSEC_REG_F_TAIKI_MC_VER		0x22c
#define NETSEC_REG_F_TAIKI_VER			0x230
#define NETSEC_REG_DMA_HM_CTRL			0x214
#define NETSEC_REG_DMA_MH_CTRL			0x220
#define NETSEC_REG_ADDR_DIS_CORE		0x218
#define NETSEC_REG_DMAC_HM_CMD_BUF		0x210
#define NETSEC_REG_DMAC_MH_CMD_BUF		0x21c

#define NETSEC_REG_NRM_TX_PKTCNT		0x410

#define NETSEC_REG_NRM_TX_DONE_PKTCNT		0x414
#define NETSEC_REG_NRM_TX_DONE_TXINT_PKTCNT	0x418

#define NETSEC_REG_NRM_TX_TMR			0x41c

#define NETSEC_REG_NRM_RX_PKTCNT		0x454
#define NETSEC_REG_NRM_RX_RXINT_PKTCNT		0x458
#define NETSEC_REG_NRM_TX_TXINT_TMR		0x420
#define NETSEC_REG_NRM_RX_RXINT_TMR		0x460

#define NETSEC_REG_NRM_RX_TMR			0x45c

#define NETSEC_REG_NRM_TX_DESC_START_UP		0x434
#define NETSEC_REG_NRM_TX_DESC_START_LW		0x408
#define NETSEC_REG_NRM_RX_DESC_START_UP		0x474
#define NETSEC_REG_NRM_RX_DESC_START_LW		0x448

#define NETSEC_REG_NRM_TX_CONFIG		0x430
#define NETSEC_REG_NRM_RX_CONFIG		0x470

#define MAC_REG_STATUS				0x1024
#define MAC_REG_DATA				0x11c0
#define MAC_REG_CMD				0x11c4
#define MAC_REG_FLOW_TH				0x11cc
#define MAC_REG_INTF_SEL			0x11d4
#define MAC_REG_DESC_INIT			0x11fc
#define MAC_REG_DESC_SOFT_RST			0x1204
#define NETSEC_REG_MODE_TRANS_COMP_STATUS	0x500

#define GMAC_REG_MCR				0x0000
#define GMAC_REG_MFFR				0x0004
#define GMAC_REG_GAR				0x0010
#define GMAC_REG_GDR				0x0014
#define GMAC_REG_FCR				0x0018
#define GMAC_REG_BMR				0x1000
#define GMAC_REG_RDLAR				0x100c
#define GMAC_REG_TDLAR				0x1010
#define GMAC_REG_OMR				0x1018

#define MHZ(n)		((n) * 1000 * 1000)

#define NETSEC_TX_SHIFT_OWN_FIELD		31
#define NETSEC_TX_SHIFT_LD_FIELD		30
#define NETSEC_TX_SHIFT_DRID_FIELD		24
#define NETSEC_TX_SHIFT_PT_FIELD		21
#define NETSEC_TX_SHIFT_TDRID_FIELD		16
#define NETSEC_TX_SHIFT_CC_FIELD		15
#define NETSEC_TX_SHIFT_FS_FIELD		9
#define NETSEC_TX_LAST				8
#define NETSEC_TX_SHIFT_CO			7
#define NETSEC_TX_SHIFT_SO			6
#define NETSEC_TX_SHIFT_TRS_FIELD		4

#define NETSEC_RX_PKT_OWN_FIELD			31
#define NETSEC_RX_PKT_LD_FIELD			30
#define NETSEC_RX_PKT_SDRID_FIELD		24
#define NETSEC_RX_PKT_FR_FIELD			23
#define NETSEC_RX_PKT_ER_FIELD			21
#define NETSEC_RX_PKT_ERR_FIELD			16
#define NETSEC_RX_PKT_TDRID_FIELD		12
#define NETSEC_RX_PKT_FS_FIELD			9
#define NETSEC_RX_PKT_LS_FIELD			8
#define NETSEC_RX_PKT_CO_FIELD			6

#define NETSEC_RX_PKT_ERR_MASK			3

#define NETSEC_MAX_TX_PKT_LEN			1518
#define NETSEC_MAX_TX_JUMBO_PKT_LEN		9018

#define NETSEC_RING_GMAC			15
#define NETSEC_RING_MAX				2

#define NETSEC_TCP_SEG_LEN_MAX			1460
#define NETSEC_TCP_JUMBO_SEG_LEN_MAX		8960

#define NETSEC_RX_CKSUM_NOTAVAIL		0
#define NETSEC_RX_CKSUM_OK			1
#define NETSEC_RX_CKSUM_NG			2

#define NETSEC_TOP_IRQ_REG_ME_START			BIT(20)
#define NETSEC_IRQ_TRANSITION_COMPLETE		BIT(4)

#define NETSEC_MODE_TRANS_COMP_IRQ_N2T		BIT(20)
#define NETSEC_MODE_TRANS_COMP_IRQ_T2N		BIT(19)

#define NETSEC_INT_PKTCNT_MAX			2047

#define NETSEC_FLOW_START_TH_MAX		95
#define NETSEC_FLOW_STOP_TH_MAX			95
#define NETSEC_FLOW_PAUSE_TIME_MIN		5

#define NETSEC_CLK_EN_REG_DOM_ALL		0x3f

#define NETSEC_PKT_CTRL_REG_MODE_NRM		BIT(28)
#define NETSEC_PKT_CTRL_REG_EN_JUMBO		BIT(27)
#define NETSEC_PKT_CTRL_REG_LOG_CHKSUM_ER	BIT(3)
#define NETSEC_PKT_CTRL_REG_LOG_HD_INCOMPLETE	BIT(2)
#define NETSEC_PKT_CTRL_REG_LOG_HD_ER		BIT(1)
#define NETSEC_PKT_CTRL_REG_DRP_NO_MATCH	BIT(0)

#define NETSEC_CLK_EN_REG_DOM_G			BIT(5)
#define NETSEC_CLK_EN_REG_DOM_C			BIT(1)
#define NETSEC_CLK_EN_REG_DOM_D			BIT(0)

#define NETSEC_COM_INIT_REG_DB			BIT(2)
#define NETSEC_COM_INIT_REG_CLS			BIT(1)
#define NETSEC_COM_INIT_REG_ALL			(NETSEC_COM_INIT_REG_CLS | \
						 NETSEC_COM_INIT_REG_DB)

#define NETSEC_SOFT_RST_REG_RESET		0
#define NETSEC_SOFT_RST_REG_RUN			BIT(31)

#define NETSEC_DMA_CTRL_REG_STOP		1
#define MH_CTRL__MODE_TRANS			BIT(20)

#define NETSEC_GMAC_CMD_ST_READ			0
#define NETSEC_GMAC_CMD_ST_WRITE		BIT(28)
#define NETSEC_GMAC_CMD_ST_BUSY			BIT(31)

#define NETSEC_GMAC_BMR_REG_COMMON		0x00412080
#define NETSEC_GMAC_BMR_REG_RESET		0x00020181
#define NETSEC_GMAC_BMR_REG_SWR			0x00000001

#define NETSEC_GMAC_OMR_REG_ST			BIT(13)
#define NETSEC_GMAC_OMR_REG_SR			BIT(1)

#define NETSEC_GMAC_MCR_REG_IBN			BIT(30)
#define NETSEC_GMAC_MCR_REG_CST			BIT(25)
#define NETSEC_GMAC_MCR_REG_JE			BIT(20)
#define NETSEC_MCR_PS				BIT(15)
#define NETSEC_GMAC_MCR_REG_FES			BIT(14)
#define NETSEC_GMAC_MCR_REG_FULL_DUPLEX_COMMON	0x0000280c
#define NETSEC_GMAC_MCR_REG_HALF_DUPLEX_COMMON	0x0001a00c

#define NETSEC_FCR_RFE				BIT(2)
#define NETSEC_FCR_TFE				BIT(1)

#define NETSEC_GMAC_GAR_REG_GW			BIT(1)
#define NETSEC_GMAC_GAR_REG_GB			BIT(0)

#define NETSEC_GMAC_GAR_REG_SHIFT_PA		11
#define NETSEC_GMAC_GAR_REG_SHIFT_GR		6
#define GMAC_REG_SHIFT_CR_GAR			2

#define NETSEC_GMAC_GAR_REG_CR_25_35_MHZ	2
#define NETSEC_GMAC_GAR_REG_CR_35_60_MHZ	3
#define NETSEC_GMAC_GAR_REG_CR_60_100_MHZ	0
#define NETSEC_GMAC_GAR_REG_CR_100_150_MHZ	1
#define NETSEC_GMAC_GAR_REG_CR_150_250_MHZ	4
#define NETSEC_GMAC_GAR_REG_CR_250_300_MHZ	5

#define NETSEC_GMAC_RDLAR_REG_COMMON		0x18000
#define NETSEC_GMAC_TDLAR_REG_COMMON		0x1c000

#define NETSEC_REG_NETSEC_VER_F_TAIKI		0x50000

#define NETSEC_REG_DESC_RING_CONFIG_CFG_UP	BIT(31)
#define NETSEC_REG_DESC_RING_CONFIG_CH_RST	BIT(30)
#define NETSEC_REG_DESC_TMR_MODE		4
#define NETSEC_REG_DESC_ENDIAN			0

#define NETSEC_MAC_DESC_SOFT_RST_SOFT_RST	1
#define NETSEC_MAC_DESC_INIT_REG_INIT		1

#define NETSEC_EEPROM_MAC_ADDRESS		0x00
#define NETSEC_EEPROM_HM_ME_ADDRESS_H		0x08
#define NETSEC_EEPROM_HM_ME_ADDRESS_L		0x0C
#define NETSEC_EEPROM_HM_ME_SIZE		0x10
#define NETSEC_EEPROM_MH_ME_ADDRESS_H		0x14
#define NETSEC_EEPROM_MH_ME_ADDRESS_L		0x18
#define NETSEC_EEPROM_MH_ME_SIZE		0x1C
#define NETSEC_EEPROM_PKT_ME_ADDRESS		0x20
#define NETSEC_EEPROM_PKT_ME_SIZE		0x24

#define DESC_SZ	sizeof(struct netsec_de)

#define NETSEC_F_NETSEC_VER_MAJOR_NUM(x)	((x) & 0xffff0000)

#define EERPROM_MAP_OFFSET	0x8000000
#define NOR_BLOCK	1024

struct netsec_de { /* Netsec Descriptor layout */
	u32 attr;
	u32 data_buf_addr_up;
	u32 data_buf_addr_lw;
	u32 buf_len_info;
};

struct netsec_priv {
	struct netsec_de rxde[PKTBUFSRX];
	struct netsec_de txde[1];
	u16 rxat;

	phys_addr_t eeprom_base;
	phys_addr_t ioaddr;

	struct mii_dev *bus;
	struct phy_device *phydev;
	u32 phy_addr, freq;
	int phy_mode;
	int max_speed;
};

struct netsec_tx_pkt_ctrl {
	u16 tcp_seg_len;
	bool tcp_seg_offload_flag;
	bool cksum_offload_flag;
};

struct netsec_rx_pkt_info {
	int rx_cksum_result;
	int err_code;
	bool err_flag;
};

static int netsec_reset_hardware(struct netsec_priv *priv, bool load_ucode);

static void netsec_write_reg(struct netsec_priv *priv, u32 reg_addr, u32 val)
{
	writel(val, priv->ioaddr + reg_addr);
}

static u32 netsec_read_reg(struct netsec_priv *priv, u32 reg_addr)
{
	return readl(priv->ioaddr + reg_addr);
}

/************* MDIO BUS OPS FOLLOW *************/

#define TIMEOUT_SPINS_MAC		1000
#define TIMEOUT_SECONDARY_MS_MAC	100

static u32 netsec_clk_type(u32 freq)
{
	if (freq < MHZ(35))
		return NETSEC_GMAC_GAR_REG_CR_25_35_MHZ;
	if (freq < MHZ(60))
		return NETSEC_GMAC_GAR_REG_CR_35_60_MHZ;
	if (freq < MHZ(100))
		return NETSEC_GMAC_GAR_REG_CR_60_100_MHZ;
	if (freq < MHZ(150))
		return NETSEC_GMAC_GAR_REG_CR_100_150_MHZ;
	if (freq < MHZ(250))
		return NETSEC_GMAC_GAR_REG_CR_150_250_MHZ;

	return NETSEC_GMAC_GAR_REG_CR_250_300_MHZ;
}

static int netsec_wait_while_busy(struct netsec_priv *priv, u32 addr, u32 mask)
{
	u32 timeout = TIMEOUT_SPINS_MAC;

	while (--timeout && netsec_read_reg(priv, addr) & mask)
		cpu_relax();
	if (timeout)
		return 0;

	timeout = TIMEOUT_SECONDARY_MS_MAC;
	while (--timeout && netsec_read_reg(priv, addr) & mask)
		udelay(2000);

	if (timeout)
		return 0;

	pr_err("%s: timeout\n", __func__);

	return -ETIMEDOUT;
}

static int netsec_set_mac_reg(struct netsec_priv *priv, u32 addr, u32 value)
{
	netsec_write_reg(priv, MAC_REG_DATA, value);
	netsec_write_reg(priv, MAC_REG_CMD, addr | NETSEC_GMAC_CMD_ST_WRITE);
	return netsec_wait_while_busy(priv,
				      MAC_REG_CMD, NETSEC_GMAC_CMD_ST_BUSY);
}

static int netsec_get_mac_reg(struct netsec_priv *priv, u32 addr, u32 *read)
{
	int ret;

	netsec_write_reg(priv, MAC_REG_CMD, addr | NETSEC_GMAC_CMD_ST_READ);
	ret = netsec_wait_while_busy(priv,
				     MAC_REG_CMD, NETSEC_GMAC_CMD_ST_BUSY);
	if (ret)
		return ret;

	*read = netsec_read_reg(priv, MAC_REG_DATA);

	return 0;
}

static int netsec_mac_wait_while_busy(struct netsec_priv *priv,
				      u32 addr, u32 mask)
{
	u32 timeout = TIMEOUT_SPINS_MAC;
	u32 data;
	int ret;

	do {
		ret = netsec_get_mac_reg(priv, addr, &data);
		if (ret)
			break;
		udelay(1);
	} while (--timeout && (data & mask));

	if (timeout)
		return 0;

	timeout = TIMEOUT_SECONDARY_MS_MAC;
	do {
		udelay(2000);

		ret = netsec_get_mac_reg(priv, addr, &data);
		if (ret)
			break;
		cpu_relax();
	} while (--timeout && (data & mask));

	if (timeout && !ret)
		return 0;

	return -ETIMEDOUT;
}

static void netsec_cache_invalidate(uintptr_t vaddr, int len)
{
	invalidate_dcache_range(rounddown(vaddr, ARCH_DMA_MINALIGN),
				roundup(vaddr + len, ARCH_DMA_MINALIGN));
}

static void netsec_cache_flush(uintptr_t vaddr, int len)
{
	flush_dcache_range(rounddown(vaddr, ARCH_DMA_MINALIGN),
			   roundup(vaddr + len, ARCH_DMA_MINALIGN));
}

static void netsec_set_rx_de(struct netsec_priv *priv, u16 idx, void *addr)
{
	struct netsec_de *de = &priv->rxde[idx];
	u32 attr = (1 << NETSEC_RX_PKT_OWN_FIELD) |
		   (1 << NETSEC_RX_PKT_FS_FIELD) |
		   (1 << NETSEC_RX_PKT_LS_FIELD);

	if (idx == PKTBUFSRX - 1)
		attr |= (1 << NETSEC_RX_PKT_LD_FIELD);

	de->data_buf_addr_up = upper_32_bits((dma_addr_t)addr);
	de->data_buf_addr_lw = lower_32_bits((dma_addr_t)addr);
	de->buf_len_info = PKTSIZE;
	de->attr = attr;
	dmb();
	netsec_cache_flush((uintptr_t)de, sizeof(*de));
}

static void netsec_set_tx_de(struct netsec_priv *priv, void *addr, int len)
{
	struct netsec_de *de = &priv->txde[0];
	u32 attr;

	attr = (1 << NETSEC_TX_SHIFT_OWN_FIELD) |
	       (1 << NETSEC_TX_SHIFT_PT_FIELD) |
	       (NETSEC_RING_GMAC << NETSEC_TX_SHIFT_TDRID_FIELD) |
	       (1 << NETSEC_TX_SHIFT_FS_FIELD) |
	       (1 << NETSEC_TX_LAST) |
	       (1 << NETSEC_TX_SHIFT_TRS_FIELD) |
			(1 << NETSEC_TX_SHIFT_LD_FIELD);

	de->data_buf_addr_up = upper_32_bits((dma_addr_t)addr);
	de->data_buf_addr_lw = lower_32_bits((dma_addr_t)addr);
	de->buf_len_info = len;
	de->attr = attr;
	dmb();
	netsec_cache_flush((uintptr_t)de, sizeof(*de));
}

static int netsec_get_phy_reg(struct netsec_priv *priv,
			      int phy_addr, int reg_addr)
{
	u32 data;
	int ret;

	if (phy_addr != 7)
		return -EINVAL;

	if (netsec_set_mac_reg(priv, GMAC_REG_GAR, NETSEC_GMAC_GAR_REG_GB |
			       phy_addr << NETSEC_GMAC_GAR_REG_SHIFT_PA |
			       reg_addr << NETSEC_GMAC_GAR_REG_SHIFT_GR |
			       (netsec_clk_type(priv->freq) <<
				GMAC_REG_SHIFT_CR_GAR)))
		return -ETIMEDOUT;

	ret = netsec_mac_wait_while_busy(priv, GMAC_REG_GAR,
					 NETSEC_GMAC_GAR_REG_GB);
	if (ret)
		return ret;

	ret = netsec_get_mac_reg(priv, GMAC_REG_GDR, &data);
	if (ret)
		return ret;

	return data;
}

static int netsec_set_phy_reg(struct netsec_priv *priv,
			      int phy_addr, int reg_addr, u16 val)
{
	int ret;

	if (phy_addr != 7)
		return -EINVAL;
	if (netsec_set_mac_reg(priv, GMAC_REG_GDR, val))
		return -ETIMEDOUT;

	if (netsec_set_mac_reg(priv, GMAC_REG_GAR,
			       phy_addr << NETSEC_GMAC_GAR_REG_SHIFT_PA |
			       reg_addr << NETSEC_GMAC_GAR_REG_SHIFT_GR |
			       NETSEC_GMAC_GAR_REG_GW | NETSEC_GMAC_GAR_REG_GB |
			       (netsec_clk_type(priv->freq) <<
				GMAC_REG_SHIFT_CR_GAR)))
		return -ETIMEDOUT;

	ret = netsec_mac_wait_while_busy(priv, GMAC_REG_GAR,
					 NETSEC_GMAC_GAR_REG_GB);

	/* Developerbox implements RTL8211E PHY and there is
	 * a compatibility problem with F_GMAC4.
	 * RTL8211E expects MDC clock must be kept toggling for several
	 * clock cycle with MDIO high before entering the IDLE state.
	 * To meet this requirement, netsec driver needs to issue dummy
	 * read(e.g. read PHYID1(offset 0x2) register) right after write.
	 */
	netsec_get_phy_reg(priv, phy_addr, MII_PHYSID1);

	return ret;
}

static int netsec_mac_update_to_phy_state(struct netsec_priv *priv)
{
	struct phy_device *phydev = priv->phydev;
	u32 value = 0;

	value = phydev->duplex ? NETSEC_GMAC_MCR_REG_FULL_DUPLEX_COMMON :
				 NETSEC_GMAC_MCR_REG_HALF_DUPLEX_COMMON;

	if (phydev->speed != SPEED_1000)
		value |= NETSEC_MCR_PS;

	if (phydev->interface != PHY_INTERFACE_MODE_GMII &&
	    phydev->speed == SPEED_100)
		value |= NETSEC_GMAC_MCR_REG_FES;

	value |= NETSEC_GMAC_MCR_REG_CST | NETSEC_GMAC_MCR_REG_JE;

	if (phy_interface_is_rgmii(phydev))
		value |= NETSEC_GMAC_MCR_REG_IBN;

	if (netsec_set_mac_reg(priv, GMAC_REG_MCR, value))
		return -ETIMEDOUT;

	return 0;
}

static int netsec_reset_gmac(struct netsec_priv *priv)
{
	u32 value = 0;
	int ret;

	if (netsec_set_mac_reg(priv, GMAC_REG_BMR,
			       NETSEC_GMAC_BMR_REG_RESET))
		return -ETIMEDOUT;

	/* Wait soft reset */
	mdelay(5);

	ret = netsec_get_mac_reg(priv, GMAC_REG_BMR, &value);
	if (ret)
		return ret;

	if (value & NETSEC_GMAC_BMR_REG_SWR)
		return -EAGAIN;

	/**
	 * NETSEC GMAC sometimes shows the peculiar behaviour where
	 * MAC_REG_DESC_SOFT_RST never been cleared, resulting in the loss of
	 * sending packets.
	 *
	 * Workaround:
	 *   Restart NETSEC and PHY, retry again.
	 */
	netsec_write_reg(priv, MAC_REG_DESC_SOFT_RST, 1);
	udelay(1000);
	if (netsec_read_reg(priv, MAC_REG_DESC_SOFT_RST)) {
		phy_shutdown(priv->phydev);
		netsec_reset_hardware(priv, false);
		phy_startup(priv->phydev);
		return -EAGAIN;
	}
	return 0;
}

static int netsec_start_gmac(struct netsec_priv *priv)
{
	u32 value = 0;
	u32 failure = 0;
	int ret;

	if (priv->max_speed != SPEED_1000)
		value = (NETSEC_GMAC_MCR_REG_CST |
			 NETSEC_GMAC_MCR_REG_HALF_DUPLEX_COMMON);

	if (netsec_set_mac_reg(priv, GMAC_REG_MCR, value))
		return -ETIMEDOUT;

	/* Reset GMAC */
	while ((ret = netsec_reset_gmac(priv)) == -EAGAIN && ++failure < 3)
		;

	if (ret) {
		pr_err("%s: failed to reset gmac(err=%d).\n", __func__, ret);
		return ret;
	}

	netsec_write_reg(priv, MAC_REG_DESC_INIT, 1);
	if (netsec_wait_while_busy(priv, MAC_REG_DESC_INIT, 1))
		return -ETIMEDOUT;

	if (netsec_set_mac_reg(priv, GMAC_REG_BMR,
			       NETSEC_GMAC_BMR_REG_COMMON))
		return -ETIMEDOUT;

	if (netsec_set_mac_reg(priv, GMAC_REG_RDLAR,
			       NETSEC_GMAC_RDLAR_REG_COMMON))
		return -ETIMEDOUT;

	if (netsec_set_mac_reg(priv, GMAC_REG_TDLAR,
			       NETSEC_GMAC_TDLAR_REG_COMMON))
		return -ETIMEDOUT;

	if (netsec_set_mac_reg(priv, GMAC_REG_MFFR, 0x80000001))
		return -ETIMEDOUT;

	ret = netsec_mac_update_to_phy_state(priv);
	if (ret)
		return ret;

	ret = netsec_get_mac_reg(priv, GMAC_REG_OMR, &value);
	if (ret)
		return ret;

	value |= NETSEC_GMAC_OMR_REG_SR;
	value |= NETSEC_GMAC_OMR_REG_ST;

	netsec_write_reg(priv, NETSEC_REG_NRM_RX_INTEN_CLR, ~0);
	netsec_write_reg(priv, NETSEC_REG_NRM_TX_INTEN_CLR, ~0);

	if (netsec_set_mac_reg(priv, GMAC_REG_OMR, value))
		return -ETIMEDOUT;

	return 0;
}

static int netsec_stop_gmac(struct netsec_priv *priv)
{
	u32 value;
	int ret;

	ret = netsec_get_mac_reg(priv, GMAC_REG_OMR, &value);
	if (ret)
		return ret;
	value &= ~NETSEC_GMAC_OMR_REG_SR;
	value &= ~NETSEC_GMAC_OMR_REG_ST;

	/* disable all interrupts */
	netsec_write_reg(priv, NETSEC_REG_NRM_RX_INTEN_CLR, ~0);
	netsec_write_reg(priv, NETSEC_REG_NRM_TX_INTEN_CLR, ~0);

	return netsec_set_mac_reg(priv, GMAC_REG_OMR, value);
}

static void netsec_spi_read(char *buf, loff_t len, loff_t offset)
{
	struct spi_flash *flash;

	flash = spi_flash_probe(CONFIG_SF_DEFAULT_BUS, CONFIG_SF_DEFAULT_CS,
				CONFIG_SF_DEFAULT_SPEED, CONFIG_SF_DEFAULT_MODE);

	spi_flash_read(flash, offset, len, buf);
}

static int netsec_read_rom_hwaddr(struct udevice *dev)
{
	struct netsec_priv *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);
	char macp[NOR_BLOCK];

	netsec_spi_read(macp, sizeof(macp), priv->eeprom_base);

	pdata->enetaddr[0] = readb(macp + 3);
	pdata->enetaddr[1] = readb(macp + 2);
	pdata->enetaddr[2] = readb(macp + 1);
	pdata->enetaddr[3] = readb(macp + 0);
	pdata->enetaddr[4] = readb(macp + 7);
	pdata->enetaddr[5] = readb(macp + 6);
	return 0;
}

static int netsec_send(struct udevice *dev, void *packet, int length)
{
	struct netsec_priv *priv = dev_get_priv(dev);
	u32 val, tout;

	val = netsec_read_reg(priv, NETSEC_REG_NRM_TX_STATUS);
	netsec_cache_flush((uintptr_t)packet, length);
	netsec_set_tx_de(priv, packet, length);
	netsec_write_reg(priv, NETSEC_REG_NRM_TX_PKTCNT, 1); /* submit another tx */

	val = netsec_read_reg(priv, NETSEC_REG_NRM_TX_PKTCNT);

	tout = 10000;
	do {
		val = netsec_read_reg(priv, NETSEC_REG_NRM_TX_DONE_PKTCNT);
		udelay(2);
	} while (--tout && !val);

	if (!tout) {
		val = netsec_read_reg(priv, NETSEC_REG_NRM_TX_PKTCNT);
		pr_err("%s: ETIMEDOUT:  %dpackets\n", __func__, val);
		return -ETIMEDOUT;
	}

	return 0;
}

static int netsec_free_packet(struct udevice *dev, uchar *packet, int length)
{
	struct netsec_priv *priv = dev_get_priv(dev);

	netsec_set_rx_de(priv, priv->rxat, net_rx_packets[priv->rxat]);

	priv->rxat++;
	if (priv->rxat == PKTBUFSRX)
		priv->rxat = 0;

	return 0;
}

static int netsec_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct netsec_priv *priv = dev_get_priv(dev);
	int idx = priv->rxat;
	uchar *ptr = net_rx_packets[idx];
	struct netsec_de *de = &priv->rxde[idx];
	int length = 0;

	netsec_cache_invalidate((uintptr_t)de, sizeof(*de));

	if (de->attr & (1U << NETSEC_RX_PKT_OWN_FIELD))
		return -EAGAIN;

	length = de->buf_len_info >> 16;

	/* invalidate after DMA is done */
	netsec_cache_invalidate((uintptr_t)ptr, length);
	*packetp = ptr;

	return length;
}

static int _netsec_get_phy_reg(struct mii_dev *bus,
			       int phy_addr, int devad, int reg_addr)
{
	return netsec_get_phy_reg(bus->priv, phy_addr, reg_addr);
}

static int _netsec_set_phy_reg(struct mii_dev *bus,
			       int phy_addr, int devad, int reg_addr, u16 val)
{
	return netsec_set_phy_reg(bus->priv, phy_addr, reg_addr, val);
}

static int netsec_mdiobus_init(struct netsec_priv *priv, const char *name)
{
	struct mii_dev *bus = mdio_alloc();

	if (!bus)
		return -ENOMEM;

	bus->read = _netsec_get_phy_reg;
	bus->write = _netsec_set_phy_reg;
	snprintf(bus->name, sizeof(bus->name), "%s", name);
	bus->priv = priv;

	return mdio_register(bus);
}

static int netsec_phy_init(struct netsec_priv *priv, void *dev)
{
	struct phy_device *phydev;
	int ret;

	phydev = phy_connect(priv->bus, priv->phy_addr, dev, priv->phy_mode);

	phydev->supported &= PHY_GBIT_FEATURES;
	if (priv->max_speed) {
		ret = phy_set_supported(phydev, priv->max_speed);
		if (ret)
			return ret;
	}
	phydev->advertising = phydev->supported;

	priv->phydev = phydev;
	phy_config(phydev);

	return 0;
}

static int netsec_netdev_load_ucode_region(struct netsec_priv *priv, u32 reg,
					   u32 addr_h, u32 addr_l, u32 size)
{
	u64 base = ((u64)addr_h << 32 | addr_l) - EERPROM_MAP_OFFSET;

	while (size > 0) {
		char buf[NOR_BLOCK];
		u32 *ucode = (u32 *)buf;
		u64 off;
		int i;

		off = base % NOR_BLOCK;
		base -= off;
		netsec_spi_read(buf, sizeof(buf), base);

		for (i = off / 4; i < sizeof(buf) / 4 && size > 0; i++, size--)
			netsec_write_reg(priv, reg, ucode[i]);
		base += NOR_BLOCK;
	}

	return 0;
}

static int netsec_netdev_load_microcode(struct netsec_priv *priv)
{
	u32 addr_h, addr_l, size;
	char buf[NOR_BLOCK];
	u32 *ucinfo = (u32 *)buf;
	int err;

	netsec_spi_read(buf, sizeof(buf), priv->eeprom_base);

	addr_h = ucinfo[NETSEC_EEPROM_HM_ME_ADDRESS_H >> 2];
	addr_l = ucinfo[NETSEC_EEPROM_HM_ME_ADDRESS_L >> 2];
	size = ucinfo[NETSEC_EEPROM_HM_ME_SIZE >> 2];

	err = netsec_netdev_load_ucode_region(priv, NETSEC_REG_DMAC_HM_CMD_BUF,
					      addr_h, addr_l, size);
	if (err)
		return err;

	addr_h = ucinfo[NETSEC_EEPROM_MH_ME_ADDRESS_H >> 2];
	addr_l = ucinfo[NETSEC_EEPROM_MH_ME_ADDRESS_L >> 2];
	size = ucinfo[NETSEC_EEPROM_MH_ME_SIZE >> 2];

	err = netsec_netdev_load_ucode_region(priv, NETSEC_REG_DMAC_MH_CMD_BUF,
					      addr_h, addr_l, size);
	if (err)
		return err;

	addr_h = 0;
	addr_l = ucinfo[NETSEC_EEPROM_PKT_ME_ADDRESS >> 2];
	size = ucinfo[NETSEC_EEPROM_PKT_ME_SIZE >> 2];

	err = netsec_netdev_load_ucode_region(priv, NETSEC_REG_PKT_CMD_BUF,
					      addr_h, addr_l, size);
	if (err)
		return err;

	return 0;
}

void netsec_pre_init_microengine(struct netsec_priv *priv)
{
	u32 data;

	/* Remove dormant settings */
	data = netsec_get_phy_reg(priv, priv->phy_addr, MII_BMCR);
	data &= ~BMCR_PDOWN;
	data |= BMCR_ISOLATE;
	netsec_set_phy_reg(priv, priv->phy_addr, MII_BMCR, data);
	mdelay(100);

	/* Put phy in loopback mode to guarantee RXCLK input */
	data |= BMCR_LOOPBACK;
	netsec_set_phy_reg(priv, priv->phy_addr, MII_BMCR, data);
	mdelay(100);
}

void netsec_post_init_microengine(struct netsec_priv *priv)
{
	u32 data;

	/* Get phy back to normal operation */
	data = netsec_get_phy_reg(priv, priv->phy_addr, MII_BMCR);
	data &= ~BMCR_LOOPBACK;
	netsec_set_phy_reg(priv, priv->phy_addr, MII_BMCR, data);
	mdelay(100);

	/* Apply software reset */
	data |= BMCR_RESET;
	netsec_set_phy_reg(priv, priv->phy_addr, MII_BMCR, data);
	mdelay(100);
}

static int netsec_reset_hardware(struct netsec_priv *priv, bool load_ucode)
{
	u32 value;
	int err;

	netsec_write_reg(priv, NETSEC_REG_CLK_EN, 0x24);

	/* stop DMA engines */
	if (!netsec_read_reg(priv, NETSEC_REG_ADDR_DIS_CORE)) {
		netsec_write_reg(priv, NETSEC_REG_DMA_HM_CTRL,
				 NETSEC_DMA_CTRL_REG_STOP);
		netsec_write_reg(priv, NETSEC_REG_DMA_MH_CTRL,
				 NETSEC_DMA_CTRL_REG_STOP);

		value = 100;
		while (netsec_read_reg(priv, NETSEC_REG_DMA_HM_CTRL) &
		       NETSEC_DMA_CTRL_REG_STOP) {
			udelay(1000);
			if (--value == 0) {
				pr_err("%s:%d timeout!\n", __func__, __LINE__);
				break;
			}
		}

		value = 100;
		while (netsec_read_reg(priv, NETSEC_REG_DMA_MH_CTRL) &
		       NETSEC_DMA_CTRL_REG_STOP) {
			udelay(1000);
			if (--value == 0) {
				pr_err("%s:%d timeout!\n", __func__, __LINE__);
				break;
			}
		}
	}

	netsec_set_mac_reg(priv, GMAC_REG_BMR, NETSEC_GMAC_BMR_REG_RESET);

	netsec_write_reg(priv, NETSEC_REG_SOFT_RST, NETSEC_SOFT_RST_REG_RESET);
	netsec_write_reg(priv, NETSEC_REG_SOFT_RST, NETSEC_SOFT_RST_REG_RUN);
	netsec_write_reg(priv, NETSEC_REG_COM_INIT, NETSEC_COM_INIT_REG_ALL);

	value = 100;
	while (netsec_read_reg(priv, NETSEC_REG_COM_INIT) != 0) {
		udelay(1000);
		if (--value == 0) {
			pr_err("%s:%d COM_INIT timeout!\n", __func__, __LINE__);
			break;
		}
	}

	/* MAC desc init */
	netsec_write_reg(priv, MAC_REG_DESC_INIT, 1);
	netsec_wait_while_busy(priv, MAC_REG_DESC_INIT, 1);
	/* set MAC_INTF_SEL */
	netsec_write_reg(priv, MAC_REG_INTF_SEL, 1);

	netsec_write_reg(priv, NETSEC_REG_CLK_EN, 1 << 5);

	/* set desc_start addr */
	netsec_write_reg(priv, NETSEC_REG_NRM_RX_DESC_START_UP,
			 upper_32_bits((dma_addr_t)priv->rxde));
	netsec_write_reg(priv, NETSEC_REG_NRM_RX_DESC_START_LW,
			 lower_32_bits((dma_addr_t)priv->rxde));

	netsec_write_reg(priv, NETSEC_REG_NRM_TX_DESC_START_UP,
			 upper_32_bits((dma_addr_t)priv->txde));
	netsec_write_reg(priv, NETSEC_REG_NRM_TX_DESC_START_LW,
			 lower_32_bits((dma_addr_t)priv->txde));

	/* set normal tx dring ring config */
	netsec_write_reg(priv, NETSEC_REG_NRM_TX_CONFIG,
			 1 << NETSEC_REG_DESC_ENDIAN);
	netsec_write_reg(priv, NETSEC_REG_NRM_RX_CONFIG,
			 1 << NETSEC_REG_DESC_ENDIAN);

	if (load_ucode) {
		err = netsec_netdev_load_microcode(priv);
		if (err) {
			pr_err("%s: failed to load microcode (%d)\n",
			       __func__, err);
			return err;
		}
	}

	/* set desc_start addr */
	netsec_write_reg(priv, NETSEC_REG_NRM_RX_DESC_START_UP,
			 upper_32_bits((dma_addr_t)priv->rxde));
	netsec_write_reg(priv, NETSEC_REG_NRM_RX_DESC_START_LW,
			 lower_32_bits((dma_addr_t)priv->rxde));

	netsec_write_reg(priv, NETSEC_REG_NRM_TX_DESC_START_UP,
			 upper_32_bits((dma_addr_t)priv->txde));
	netsec_write_reg(priv, NETSEC_REG_NRM_TX_DESC_START_LW,
			 lower_32_bits((dma_addr_t)priv->txde));

	netsec_write_reg(priv, NETSEC_REG_CLK_EN, 1 << 5);

	/* start DMA engines */
	netsec_write_reg(priv, NETSEC_REG_DMA_TMR_CTRL, priv->freq / 1000000 - 1);

	netsec_pre_init_microengine(priv);

	netsec_write_reg(priv, NETSEC_REG_ADDR_DIS_CORE, 0);

	mdelay(100);

	if (!(netsec_read_reg(priv, NETSEC_REG_TOP_STATUS) &
	      NETSEC_TOP_IRQ_REG_ME_START)) {
		pr_err("microengine start failed\n");
		return -ENXIO;
	}

	netsec_post_init_microengine(priv);

	/* clear microcode load end status */
	netsec_write_reg(priv, NETSEC_REG_TOP_STATUS,
			 NETSEC_TOP_IRQ_REG_ME_START);

	netsec_write_reg(priv, NETSEC_REG_CLK_EN, 1 << 5);

	value = netsec_read_reg(priv, NETSEC_REG_PKT_CTRL);
	value |= NETSEC_PKT_CTRL_REG_MODE_NRM;
	/* change to normal mode */
	netsec_write_reg(priv, NETSEC_REG_DMA_MH_CTRL, MH_CTRL__MODE_TRANS);
	netsec_write_reg(priv, NETSEC_REG_PKT_CTRL, value);

	value = 100;
	while ((netsec_read_reg(priv, NETSEC_REG_MODE_TRANS_COMP_STATUS) &
		NETSEC_MODE_TRANS_COMP_IRQ_T2N) == 0) {
		udelay(1000);
		if (--value == 0) {
			value = netsec_read_reg(priv, NETSEC_REG_MODE_TRANS_COMP_STATUS);
			pr_err("%s:%d timeout! val=%x\n", __func__, __LINE__, value);
			break;
		}
	}

	/* clear any pending EMPTY/ERR irq status */
	netsec_write_reg(priv, NETSEC_REG_NRM_TX_STATUS, ~0);

	/* Disable TX & RX intr */
	netsec_write_reg(priv, NETSEC_REG_INTEN_CLR, ~0);

	return 0;
}

static void netsec_stop(struct udevice *dev)
{
	struct netsec_priv *priv = dev_get_priv(dev);

	netsec_write_reg(priv, NETSEC_REG_ADDR_DIS_CORE, 7);
	netsec_stop_gmac(priv);
	phy_shutdown(priv->phydev);
	netsec_reset_hardware(priv, false);
}

static int netsec_start(struct udevice *dev)
{
	struct netsec_priv *priv = dev_get_priv(dev);
	int i;

	phy_startup(priv->phydev);
	netsec_start_gmac(priv);

	priv->rxat = 0;
	for (i = 0; i < PKTBUFSRX; i++)
		netsec_set_rx_de(priv, i, net_rx_packets[i]);

	return 0;
}

static int netsec_of_to_plat(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct netsec_priv *priv = dev_get_priv(dev);
	struct ofnode_phandle_args phandle_args;

	pdata->iobase = dev_read_addr_index(dev, 0);
	priv->eeprom_base = dev_read_addr_index(dev, 1) - EERPROM_MAP_OFFSET;

	pdata->phy_interface = dev_read_phy_mode(dev);
	if (pdata->phy_interface == PHY_INTERFACE_MODE_NA)
		return -EINVAL;

	if (!dev_read_phandle_with_args(dev, "phy-handle", NULL, 0, 0,
					&phandle_args))
		priv->phy_addr = ofnode_read_u32_default(phandle_args.node, "reg", 7);
	else
		priv->phy_addr = 7;

	pdata->max_speed = dev_read_u32_default(dev, "max-speed", SPEED_1000);

	priv->ioaddr = pdata->iobase;
	priv->phy_mode = pdata->phy_interface;
	priv->max_speed = pdata->max_speed;
	priv->freq = 250000000UL;

	return 0;
}

static int netsec_probe(struct udevice *dev)
{
	struct netsec_priv *priv = dev_get_priv(dev);
	int ret;

	netsec_reset_hardware(priv, true);

	ret = netsec_mdiobus_init(priv, dev->name);
	if (ret) {
		pr_err("Failed to initialize mdiobus: %d\n", ret);
		return ret;
	}

	priv->bus = miiphy_get_dev_by_name(dev->name);

	ret = netsec_phy_init(priv, dev);
	if (ret) {
		pr_err("Failed to initialize phy: %d\n", ret);
		goto out_mdiobus_release;
	}

	return 0;
out_mdiobus_release:
	mdio_unregister(priv->bus);
	mdio_free(priv->bus);
	return ret;
}

static int netsec_remove(struct udevice *dev)
{
	struct netsec_priv *priv = dev_get_priv(dev);

	free(priv->phydev);
	mdio_unregister(priv->bus);
	mdio_free(priv->bus);

	return 0;
}

static const struct eth_ops netsec_ops = {
	.start        = netsec_start,
	.stop         = netsec_stop,
	.send         = netsec_send,
	.recv         = netsec_recv,
	.free_pkt     = netsec_free_packet,
	.read_rom_hwaddr = netsec_read_rom_hwaddr,
};

static const struct udevice_id netsec_ids[] = {
	{
		.compatible = "socionext,synquacer-netsec",
	},
	{}
};

U_BOOT_DRIVER(ave) = {
	.name     = "synquacer_netsec",
	.id       = UCLASS_ETH,
	.of_match = netsec_ids,
	.probe	  = netsec_probe,
	.remove	  = netsec_remove,
	.of_to_plat = netsec_of_to_plat,
	.ops	  = &netsec_ops,
	.priv_auto	= sizeof(struct netsec_priv),
	.plat_auto	= sizeof(struct eth_pdata),
};
