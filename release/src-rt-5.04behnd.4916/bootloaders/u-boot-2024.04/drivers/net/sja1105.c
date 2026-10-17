// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2016-2018 NXP
 * Copyright 2018, Sensor-Technik Wiedemann GmbH
 * Copyright 2018-2019, Vladimir Oltean <olteanv@gmail.com>
 * Copyright 2020-2021 NXP
 *
 * Ported from Linux (drivers/net/dsa/sja1105/).
 */

#include <common.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/bitrev.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/if_ether.h>
#include <linux/if_vlan.h>
#include <linux/types.h>
#include <net/dsa.h>
#include <stdlib.h>
#include <spi.h>
#include <miiphy.h>
#include <dm/of_extra.h>

enum packing_op {
	PACK,
	UNPACK,
};

#define ETHER_CRC32_POLY				0x04C11DB7
#define ETH_P_SJA1105					0xdadb
#define SJA1105_NUM_PORTS				5
#define SJA1110_NUM_PORTS				11
#define SJA1105_MAX_NUM_PORTS				SJA1110_NUM_PORTS
#define SJA1105_NUM_TC					8
#define SJA1105ET_FDB_BIN_SIZE				4
#define SJA1105_SIZE_CGU_CMD				4
#define SJA1105_SIZE_RESET_CMD				4
#define SJA1105_SIZE_MDIO_CMD				4
#define SJA1105_SIZE_SPI_MSG_HEADER			4
#define SJA1105_SIZE_SPI_MSG_MAXLEN			(64 * 4)
#define SJA1105_SIZE_DEVICE_ID				4
#define SJA1105_SIZE_TABLE_HEADER			12
#define SJA1105_SIZE_L2_POLICING_ENTRY			8
#define SJA1105_SIZE_VLAN_LOOKUP_ENTRY			8
#define SJA1110_SIZE_VLAN_LOOKUP_ENTRY			12
#define SJA1105_SIZE_L2_FORWARDING_ENTRY		8
#define SJA1105_SIZE_L2_FORWARDING_PARAMS_ENTRY		12
#define SJA1105_SIZE_XMII_PARAMS_ENTRY			4
#define SJA1110_SIZE_XMII_PARAMS_ENTRY			8
#define SJA1105ET_SIZE_MAC_CONFIG_ENTRY			28
#define SJA1105ET_SIZE_GENERAL_PARAMS_ENTRY		40
#define SJA1105PQRS_SIZE_MAC_CONFIG_ENTRY		32
#define SJA1105PQRS_SIZE_GENERAL_PARAMS_ENTRY		44
#define SJA1110_SIZE_GENERAL_PARAMS_ENTRY		56

#define SJA1105_MAX_L2_LOOKUP_COUNT			1024
#define SJA1105_MAX_L2_POLICING_COUNT			45
#define SJA1110_MAX_L2_POLICING_COUNT			110
#define SJA1105_MAX_VLAN_LOOKUP_COUNT			4096
#define SJA1105_MAX_L2_FORWARDING_COUNT			13
#define SJA1110_MAX_L2_FORWARDING_COUNT			19
#define SJA1105_MAX_MAC_CONFIG_COUNT			5
#define SJA1110_MAX_MAC_CONFIG_COUNT			11
#define SJA1105_MAX_L2_FORWARDING_PARAMS_COUNT		1
#define SJA1105_MAX_GENERAL_PARAMS_COUNT		1
#define SJA1105_MAX_XMII_PARAMS_COUNT			1

#define SJA1105_MAX_FRAME_MEMORY			929

#define SJA1105E_DEVICE_ID				0x9C00000Cull
#define SJA1105T_DEVICE_ID				0x9E00030Eull
#define SJA1105PR_DEVICE_ID				0xAF00030Eull
#define SJA1105QS_DEVICE_ID				0xAE00030Eull
#define SJA1110_DEVICE_ID				0xB700030Full

#define SJA1105ET_PART_NO				0x9A83
#define SJA1105P_PART_NO				0x9A84
#define SJA1105Q_PART_NO				0x9A85
#define SJA1105R_PART_NO				0x9A86
#define SJA1105S_PART_NO				0x9A87
#define SJA1110A_PART_NO				0x1110
#define SJA1110B_PART_NO				0x1111
#define SJA1110C_PART_NO				0x1112
#define SJA1110D_PART_NO				0x1113

#define SJA1110_ACU			0x1c4400
#define SJA1110_RGU			0x1c6000
#define SJA1110_CGU			0x1c6400

#define SJA1110_SPI_ADDR(x)		((x) / 4)
#define SJA1110_ACU_ADDR(x)		(SJA1110_ACU + SJA1110_SPI_ADDR(x))
#define SJA1110_CGU_ADDR(x)		(SJA1110_CGU + SJA1110_SPI_ADDR(x))
#define SJA1110_RGU_ADDR(x)		(SJA1110_RGU + SJA1110_SPI_ADDR(x))

#define SJA1105_RSV_ADDR		0xffffffffffffffffull

#define SJA1110_PCS_BANK_REG		SJA1110_SPI_ADDR(0x3fc)

#define DSA_8021Q_DIR_TX		BIT(11)
#define DSA_8021Q_PORT_SHIFT		0
#define DSA_8021Q_PORT_MASK		GENMASK(3, 0)
#define DSA_8021Q_PORT(x)		(((x) << DSA_8021Q_PORT_SHIFT) & \
						 DSA_8021Q_PORT_MASK)

#define SJA1105_RATE_MBPS(speed) (((speed) * 64000) / 1000)

/* XPCS registers */

/* VR MII MMD registers offsets */
#define DW_VR_MII_DIG_CTRL1		0x8000
#define DW_VR_MII_AN_CTRL		0x8001
#define DW_VR_MII_DIG_CTRL2		0x80e1

/* VR_MII_DIG_CTRL1 */
#define DW_VR_MII_DIG_CTRL1_MAC_AUTO_SW		BIT(9)

/* VR_MII_DIG_CTRL2 */
#define DW_VR_MII_DIG_CTRL2_TX_POL_INV		BIT(4)

/* VR_MII_AN_CTRL */
#define DW_VR_MII_AN_CTRL_TX_CONFIG_SHIFT	3
#define DW_VR_MII_TX_CONFIG_MASK		BIT(3)
#define DW_VR_MII_TX_CONFIG_MAC_SIDE_SGMII	0x0
#define DW_VR_MII_AN_CTRL_PCS_MODE_SHIFT	1
#define DW_VR_MII_PCS_MODE_MASK			GENMASK(2, 1)
#define DW_VR_MII_PCS_MODE_C37_SGMII		0x2

/* PMA registers */

/* LANE_DRIVER1_0 register */
#define SJA1110_LANE_DRIVER1_0		0x8038
#define SJA1110_TXDRV(x)		(((x) << 12) & GENMASK(14, 12))

/* LANE_DRIVER2_0 register */
#define SJA1110_LANE_DRIVER2_0		0x803a
#define SJA1110_TXDRVTRIM_LSB(x)	((x) & GENMASK_ULL(15, 0))

/* LANE_DRIVER2_1 register */
#define SJA1110_LANE_DRIVER2_1		0x803b
#define SJA1110_LANE_DRIVER2_1_RSV	BIT(9)
#define SJA1110_TXDRVTRIM_MSB(x)	(((x) & GENMASK_ULL(23, 16)) >> 16)

/* LANE_TRIM register */
#define SJA1110_LANE_TRIM		0x8040
#define SJA1110_TXTEN			BIT(11)
#define SJA1110_TXRTRIM(x)		(((x) << 8) & GENMASK(10, 8))
#define SJA1110_TXPLL_BWSEL		BIT(7)
#define SJA1110_RXTEN			BIT(6)
#define SJA1110_RXRTRIM(x)		(((x) << 3) & GENMASK(5, 3))
#define SJA1110_CDR_GAIN		BIT(2)
#define SJA1110_ACCOUPLE_RXVCM_EN	BIT(0)

/* LANE_DATAPATH_1 register */
#define SJA1110_LANE_DATAPATH_1		0x8037

/* POWERDOWN_ENABLE register */
#define SJA1110_POWERDOWN_ENABLE	0x8041
#define SJA1110_TXPLL_PD		BIT(12)
#define SJA1110_TXPD			BIT(11)
#define SJA1110_RXPKDETEN		BIT(10)
#define SJA1110_RXCH_PD			BIT(9)
#define SJA1110_RXBIAS_PD		BIT(8)
#define SJA1110_RESET_SER_EN		BIT(7)
#define SJA1110_RESET_SER		BIT(6)
#define SJA1110_RESET_DES		BIT(5)
#define SJA1110_RCVEN			BIT(4)

/* RXPLL_CTRL0 register */
#define SJA1110_RXPLL_CTRL0		0x8065
#define SJA1110_RXPLL_FBDIV(x)		(((x) << 2) & GENMASK(9, 2))

/* RXPLL_CTRL1 register */
#define SJA1110_RXPLL_CTRL1		0x8066
#define SJA1110_RXPLL_REFDIV(x)		((x) & GENMASK(4, 0))

/* TXPLL_CTRL0 register */
#define SJA1110_TXPLL_CTRL0		0x806d
#define SJA1110_TXPLL_FBDIV(x)		((x) & GENMASK(11, 0))

/* TXPLL_CTRL1 register */
#define SJA1110_TXPLL_CTRL1		0x806e
#define SJA1110_TXPLL_REFDIV(x)		((x) & GENMASK(5, 0))

/* RX_DATA_DETECT register */
#define SJA1110_RX_DATA_DETECT		0x8045

/* RX_CDR_CTLE register */
#define SJA1110_RX_CDR_CTLE		0x8042

/* UM10944.pdf Page 11, Table 2. Configuration Blocks */
enum {
	BLKID_L2_POLICING				= 0x06,
	BLKID_VLAN_LOOKUP				= 0x07,
	BLKID_L2_FORWARDING				= 0x08,
	BLKID_MAC_CONFIG				= 0x09,
	BLKID_L2_FORWARDING_PARAMS			= 0x0E,
	BLKID_GENERAL_PARAMS				= 0x11,
	BLKID_XMII_PARAMS				= 0x4E,
};

enum sja1105_blk_idx {
	BLK_IDX_L2_POLICING = 0,
	BLK_IDX_VLAN_LOOKUP,
	BLK_IDX_L2_FORWARDING,
	BLK_IDX_MAC_CONFIG,
	BLK_IDX_L2_FORWARDING_PARAMS,
	BLK_IDX_GENERAL_PARAMS,
	BLK_IDX_XMII_PARAMS,
	BLK_IDX_MAX,
};

struct sja1105_general_params_entry {
	u64 mac_fltres1;
	u64 mac_fltres0;
	u64 mac_flt1;
	u64 mac_flt0;
	u64 casc_port;
	u64 host_port;
	u64 mirr_port;
	u64 tpid;
	u64 tpid2;
};

struct sja1105_vlan_lookup_entry {
	u64 vmemb_port;
	u64 vlan_bc;
	u64 tag_port;
	u64 vlanid;
	u64 type_entry; /* SJA1110 only */
};

struct sja1105_l2_forwarding_entry {
	u64 bc_domain;
	u64 reach_port;
	u64 fl_domain;
};

struct sja1105_l2_forwarding_params_entry {
	u64 part_spc[SJA1105_NUM_TC];
};

struct sja1105_l2_policing_entry {
	u64 sharindx;
	u64 smax;
	u64 rate;
	u64 maxlen;
	u64 partition;
};

struct sja1105_mac_config_entry {
	u64 top[SJA1105_NUM_TC];
	u64 base[SJA1105_NUM_TC];
	u64 enabled[SJA1105_NUM_TC];
	u64 speed;
	u64 vlanid;
	u64 egress;
	u64 ingress;
};

struct sja1105_xmii_params_entry {
	u64 phy_mac[SJA1105_MAX_NUM_PORTS];
	u64 xmii_mode[SJA1105_MAX_NUM_PORTS];
	u64 special[SJA1105_MAX_NUM_PORTS];
};

struct sja1105_table_header {
	u64 block_id;
	u64 len;
	u64 crc;
};

struct sja1105_table_ops {
	size_t (*packing)(void *buf, void *entry_ptr, enum packing_op op);
	size_t unpacked_entry_size;
	size_t packed_entry_size;
	size_t max_entry_count;
};

struct sja1105_table {
	const struct sja1105_table_ops *ops;
	size_t entry_count;
	void *entries;
};

struct sja1105_static_config {
	u64 device_id;
	struct sja1105_table tables[BLK_IDX_MAX];
};

struct sja1105_xpcs_cfg {
	bool inband_an;
	int speed;
};

struct sja1105_private {
	struct sja1105_static_config static_config;
	bool rgmii_rx_delay[SJA1105_MAX_NUM_PORTS];
	bool rgmii_tx_delay[SJA1105_MAX_NUM_PORTS];
	u16 pvid[SJA1105_MAX_NUM_PORTS];
	struct sja1105_xpcs_cfg xpcs_cfg[SJA1105_MAX_NUM_PORTS];
	struct mii_dev *mdio_pcs;
	const struct sja1105_info *info;
	struct udevice *dev;
};

typedef enum {
	SPI_READ = 0,
	SPI_WRITE = 1,
} sja1105_spi_rw_mode_t;

typedef enum {
	XMII_MAC = 0,
	XMII_PHY = 1,
} sja1105_mii_role_t;

typedef enum {
	XMII_MODE_MII		= 0,
	XMII_MODE_RMII		= 1,
	XMII_MODE_RGMII		= 2,
	XMII_MODE_SGMII		= 3,
} sja1105_phy_interface_t;

enum {
	SJA1105_SPEED_AUTO,
	SJA1105_SPEED_10MBPS,
	SJA1105_SPEED_100MBPS,
	SJA1105_SPEED_1000MBPS,
	SJA1105_SPEED_MAX,
};

enum sja1110_vlan_type {
	SJA1110_VLAN_INVALID = 0,
	SJA1110_VLAN_C_TAG = 1, /* Single inner VLAN tag */
	SJA1110_VLAN_S_TAG = 2, /* Single outer VLAN tag */
	SJA1110_VLAN_D_TAG = 3, /* Double tagged, use outer tag for lookup */
};

/* Keeps the different addresses between E/T and P/Q/R/S */
struct sja1105_regs {
	u64 device_id;
	u64 prod_id;
	u64 status;
	u64 port_control;
	u64 rgu;
	u64 config;
	u64 rmii_pll1;
	u64 pad_mii_tx[SJA1105_MAX_NUM_PORTS];
	u64 pad_mii_rx[SJA1105_MAX_NUM_PORTS];
	u64 pad_mii_id[SJA1105_MAX_NUM_PORTS];
	u64 cgu_idiv[SJA1105_MAX_NUM_PORTS];
	u64 mii_tx_clk[SJA1105_MAX_NUM_PORTS];
	u64 mii_rx_clk[SJA1105_MAX_NUM_PORTS];
	u64 mii_ext_tx_clk[SJA1105_MAX_NUM_PORTS];
	u64 mii_ext_rx_clk[SJA1105_MAX_NUM_PORTS];
	u64 rgmii_tx_clk[SJA1105_MAX_NUM_PORTS];
	u64 rmii_ref_clk[SJA1105_MAX_NUM_PORTS];
	u64 rmii_ext_tx_clk[SJA1105_MAX_NUM_PORTS];
	u64 pcs_base[SJA1105_MAX_NUM_PORTS];
};

struct sja1105_info {
	u64 device_id;
	u64 part_no;
	const struct sja1105_table_ops *static_ops;
	const struct sja1105_regs *regs;
	int (*reset_cmd)(struct sja1105_private *priv);
	int (*setup_rgmii_delay)(struct sja1105_private *priv, int port);
	int (*pcs_mdio_read)(struct mii_dev *bus, int phy, int mmd, int reg);
	int (*pcs_mdio_write)(struct mii_dev *bus, int phy, int mmd, int reg,
			      u16 val);
	int (*pma_config)(struct sja1105_private *priv, int port);
	const char *name;
	bool supports_mii[SJA1105_MAX_NUM_PORTS];
	bool supports_rmii[SJA1105_MAX_NUM_PORTS];
	bool supports_rgmii[SJA1105_MAX_NUM_PORTS];
	bool supports_sgmii[SJA1105_MAX_NUM_PORTS];
	const u64 port_speed[SJA1105_SPEED_MAX];
};

struct sja1105_chunk {
	u8	*buf;
	size_t	len;
	u64	reg_addr;
};

struct sja1105_spi_message {
	u64 access;
	u64 read_count;
	u64 address;
};

/* Common structure for CFG_PAD_MIIx_RX and CFG_PAD_MIIx_TX */
struct sja1105_cfg_pad_mii {
	u64 d32_os;
	u64 d32_ih;
	u64 d32_ipud;
	u64 d10_ih;
	u64 d10_os;
	u64 d10_ipud;
	u64 ctrl_os;
	u64 ctrl_ih;
	u64 ctrl_ipud;
	u64 clk_os;
	u64 clk_ih;
	u64 clk_ipud;
};

struct sja1105_cfg_pad_mii_id {
	u64 rxc_stable_ovr;
	u64 rxc_delay;
	u64 rxc_bypass;
	u64 rxc_pd;
	u64 txc_stable_ovr;
	u64 txc_delay;
	u64 txc_bypass;
	u64 txc_pd;
};

struct sja1105_cgu_idiv {
	u64 clksrc;
	u64 autoblock;
	u64 idiv;
	u64 pd;
};

struct sja1105_cgu_pll_ctrl {
	u64 pllclksrc;
	u64 msel;
	u64 autoblock;
	u64 psel;
	u64 direct;
	u64 fbsel;
	u64 bypass;
	u64 pd;
};

enum {
	CLKSRC_MII0_TX_CLK	= 0x00,
	CLKSRC_MII0_RX_CLK	= 0x01,
	CLKSRC_MII1_TX_CLK	= 0x02,
	CLKSRC_MII1_RX_CLK	= 0x03,
	CLKSRC_MII2_TX_CLK	= 0x04,
	CLKSRC_MII2_RX_CLK	= 0x05,
	CLKSRC_MII3_TX_CLK	= 0x06,
	CLKSRC_MII3_RX_CLK	= 0x07,
	CLKSRC_MII4_TX_CLK	= 0x08,
	CLKSRC_MII4_RX_CLK	= 0x09,
	CLKSRC_PLL0		= 0x0B,
	CLKSRC_PLL1		= 0x0E,
	CLKSRC_IDIV0		= 0x11,
	CLKSRC_IDIV1		= 0x12,
	CLKSRC_IDIV2		= 0x13,
	CLKSRC_IDIV3		= 0x14,
	CLKSRC_IDIV4		= 0x15,
};

struct sja1105_cgu_mii_ctrl {
	u64 clksrc;
	u64 autoblock;
	u64 pd;
};

static int get_reverse_lsw32_offset(int offset, size_t len)
{
	int closest_multiple_of_4;
	int word_index;

	word_index = offset / 4;
	closest_multiple_of_4 = word_index * 4;
	offset -= closest_multiple_of_4;
	word_index = (len / 4) - word_index - 1;
	return word_index * 4 + offset;
}

/* Simplified version of the "packing" function from Linux, adapted
 * to support only sja1105's quirk: QUIRK_LSW32_IS_FIRST
 */
static void sja1105_packing(void *pbuf, u64 *uval, int startbit, int endbit,
			    size_t pbuflen, enum packing_op op)
{
	int plogical_first_u8, plogical_last_u8, box;

	if (op == UNPACK)
		*uval = 0;

	plogical_first_u8 = startbit / 8;
	plogical_last_u8  = endbit / 8;

	for (box = plogical_first_u8; box >= plogical_last_u8; box--) {
		int box_start_bit, box_end_bit, box_addr;
		int proj_start_bit, proj_end_bit;
		u64 proj_mask;
		u8  box_mask;

		if (box == plogical_first_u8)
			box_start_bit = startbit % 8;
		else
			box_start_bit = 7;
		if (box == plogical_last_u8)
			box_end_bit = endbit % 8;
		else
			box_end_bit = 0;

		proj_start_bit = ((box * 8) + box_start_bit) - endbit;
		proj_end_bit   = ((box * 8) + box_end_bit) - endbit;
		proj_mask = GENMASK_ULL(proj_start_bit, proj_end_bit);
		box_mask  = GENMASK_ULL(box_start_bit, box_end_bit);

		box_addr = pbuflen - box - 1;
		box_addr = get_reverse_lsw32_offset(box_addr, pbuflen);

		if (op == UNPACK) {
			u64 pval;

			/* Read from pbuf, write to uval */
			pval = ((u8 *)pbuf)[box_addr] & box_mask;

			pval >>= box_end_bit;
			pval <<= proj_end_bit;
			*uval &= ~proj_mask;
			*uval |= pval;
		} else {
			u64 pval;

			/* Write to pbuf, read from uval */
			pval = (*uval) & proj_mask;
			pval >>= proj_end_bit;

			pval <<= box_end_bit;
			((u8 *)pbuf)[box_addr] &= ~box_mask;
			((u8 *)pbuf)[box_addr] |= pval;
		}
	}
}

static u32 crc32_add(u32 crc, u8 byte)
{
	u32 byte32 = bitrev32(byte);
	int i;

	for (i = 0; i < 8; i++) {
		if ((crc ^ byte32) & BIT(31)) {
			crc <<= 1;
			crc ^= ETHER_CRC32_POLY;
		} else {
			crc <<= 1;
		}
		byte32 <<= 1;
	}
	return crc;
}

/* Little-endian Ethernet CRC32 of data packed as big-endian u32 words */
static uint32_t sja1105_crc32(void *buf, size_t len)
{
	unsigned int i;
	u64 chunk;
	u32 crc;

	/* seed */
	crc = 0xFFFFFFFF;
	for (i = 0; i < len; i += 4) {
		sja1105_packing(buf + i, &chunk, 31, 0, 4, UNPACK);
		crc = crc32_add(crc, chunk & 0xFF);
		crc = crc32_add(crc, (chunk >> 8) & 0xFF);
		crc = crc32_add(crc, (chunk >> 16) & 0xFF);
		crc = crc32_add(crc, (chunk >> 24) & 0xFF);
	}
	return bitrev32(~crc);
}

static void sja1105_spi_message_pack(void *buf, struct sja1105_spi_message *msg)
{
	const int size = SJA1105_SIZE_SPI_MSG_HEADER;

	memset(buf, 0, size);

	sja1105_packing(buf, &msg->access,     31, 31, size, PACK);
	sja1105_packing(buf, &msg->read_count, 30, 25, size, PACK);
	sja1105_packing(buf, &msg->address,    24,  4, size, PACK);
}

static int sja1105_xfer_buf(const struct sja1105_private *priv,
			    sja1105_spi_rw_mode_t rw, u64 reg_addr,
			    u8 *buf, size_t len)
{
	struct udevice *dev = priv->dev;
	struct sja1105_chunk chunk = {
		.len = min_t(size_t, len, SJA1105_SIZE_SPI_MSG_MAXLEN),
		.reg_addr = reg_addr,
		.buf = buf,
	};
	int num_chunks;
	int rc, i;

	rc = dm_spi_claim_bus(dev);
	if (rc)
		return rc;

	num_chunks = DIV_ROUND_UP(len, SJA1105_SIZE_SPI_MSG_MAXLEN);

	for (i = 0; i < num_chunks; i++) {
		u8 hdr_buf[SJA1105_SIZE_SPI_MSG_HEADER];
		struct sja1105_spi_message msg;
		u8 *rx_buf = NULL;
		u8 *tx_buf = NULL;

		/* Populate the transfer's header buffer */
		msg.address = chunk.reg_addr;
		msg.access = rw;
		if (rw == SPI_READ)
			msg.read_count = chunk.len / 4;
		else
			/* Ignored */
			msg.read_count = 0;
		sja1105_spi_message_pack(hdr_buf, &msg);
		rc = dm_spi_xfer(dev, SJA1105_SIZE_SPI_MSG_HEADER * 8, hdr_buf,
				 NULL, SPI_XFER_BEGIN);
		if (rc)
			goto out;

		/* Populate the transfer's data buffer */
		if (rw == SPI_READ)
			rx_buf = chunk.buf;
		else
			tx_buf = chunk.buf;
		rc = dm_spi_xfer(dev, chunk.len * 8, tx_buf, rx_buf,
				 SPI_XFER_END);
		if (rc)
			goto out;

		/* Calculate next chunk */
		chunk.buf += chunk.len;
		chunk.reg_addr += chunk.len / 4;
		chunk.len = min_t(size_t, (ptrdiff_t)(buf + len - chunk.buf),
				  SJA1105_SIZE_SPI_MSG_MAXLEN);
	}

out:
	dm_spi_release_bus(dev);

	return rc;
}

static int sja1105et_reset_cmd(struct sja1105_private *priv)
{
	const struct sja1105_regs *regs = priv->info->regs;
	u8 packed_buf[SJA1105_SIZE_RESET_CMD] = {0};
	const int size = SJA1105_SIZE_RESET_CMD;
	u64 cold_rst = 1;

	sja1105_packing(packed_buf, &cold_rst, 3, 3, size, PACK);

	return sja1105_xfer_buf(priv, SPI_WRITE, regs->rgu, packed_buf,
				SJA1105_SIZE_RESET_CMD);
}

static int sja1105pqrs_reset_cmd(struct sja1105_private *priv)
{
	const struct sja1105_regs *regs = priv->info->regs;
	u8 packed_buf[SJA1105_SIZE_RESET_CMD] = {0};
	const int size = SJA1105_SIZE_RESET_CMD;
	u64 cold_rst = 1;

	sja1105_packing(packed_buf, &cold_rst, 2, 2, size, PACK);

	return sja1105_xfer_buf(priv, SPI_WRITE, regs->rgu, packed_buf,
				SJA1105_SIZE_RESET_CMD);
}

static int sja1110_reset_cmd(struct sja1105_private *priv)
{
	const struct sja1105_regs *regs = priv->info->regs;
	u8 packed_buf[SJA1105_SIZE_RESET_CMD] = {0};
	const int size = SJA1105_SIZE_RESET_CMD;
	u64 switch_rst = 1;

	/* Only reset the switch core.
	 * A full cold reset would re-enable the BASE_MCSS_CLOCK PLL which
	 * would turn on the microcontroller, potentially letting it execute
	 * code which could interfere with our configuration.
	 */
	sja1105_packing(packed_buf, &switch_rst, 20, 20, size, PACK);

	return sja1105_xfer_buf(priv, SPI_WRITE, regs->rgu, packed_buf,
				SJA1105_SIZE_RESET_CMD);
}

static size_t sja1105et_general_params_entry_packing(void *buf, void *entry_ptr,
						     enum packing_op op)
{
	const size_t size = SJA1105ET_SIZE_GENERAL_PARAMS_ENTRY;
	struct sja1105_general_params_entry *entry = entry_ptr;

	sja1105_packing(buf, &entry->mac_fltres1, 311, 264, size, op);
	sja1105_packing(buf, &entry->mac_fltres0, 263, 216, size, op);
	sja1105_packing(buf, &entry->mac_flt1,    215, 168, size, op);
	sja1105_packing(buf, &entry->mac_flt0,    167, 120, size, op);
	sja1105_packing(buf, &entry->casc_port,   115, 113, size, op);
	sja1105_packing(buf, &entry->host_port,   112, 110, size, op);
	sja1105_packing(buf, &entry->mirr_port,   109, 107, size, op);
	sja1105_packing(buf, &entry->tpid,         42,  27, size, op);
	sja1105_packing(buf, &entry->tpid2,        25,  10, size, op);
	return size;
}

static size_t sja1110_general_params_entry_packing(void *buf, void *entry_ptr,
						   enum packing_op op)
{
	struct sja1105_general_params_entry *entry = entry_ptr;
	const size_t size = SJA1110_SIZE_GENERAL_PARAMS_ENTRY;

	sja1105_packing(buf, &entry->mac_fltres1,  438, 391, size, op);
	sja1105_packing(buf, &entry->mac_fltres0,  390, 343, size, op);
	sja1105_packing(buf, &entry->mac_flt1,     342, 295, size, op);
	sja1105_packing(buf, &entry->mac_flt0,     294, 247, size, op);
	sja1105_packing(buf, &entry->casc_port,    242, 232, size, op);
	sja1105_packing(buf, &entry->host_port,    231, 228, size, op);
	sja1105_packing(buf, &entry->mirr_port,    227, 224, size, op);
	sja1105_packing(buf, &entry->tpid2,        159, 144, size, op);
	sja1105_packing(buf, &entry->tpid,         142, 127, size, op);
	return size;
}

static size_t
sja1105pqrs_general_params_entry_packing(void *buf, void *entry_ptr,
					 enum packing_op op)
{
	const size_t size = SJA1105PQRS_SIZE_GENERAL_PARAMS_ENTRY;
	struct sja1105_general_params_entry *entry = entry_ptr;

	sja1105_packing(buf, &entry->mac_fltres1, 343, 296, size, op);
	sja1105_packing(buf, &entry->mac_fltres0, 295, 248, size, op);
	sja1105_packing(buf, &entry->mac_flt1,    247, 200, size, op);
	sja1105_packing(buf, &entry->mac_flt0,    199, 152, size, op);
	sja1105_packing(buf, &entry->casc_port,   147, 145, size, op);
	sja1105_packing(buf, &entry->host_port,   144, 142, size, op);
	sja1105_packing(buf, &entry->mirr_port,   141, 139, size, op);
	sja1105_packing(buf, &entry->tpid,         74,  59, size, op);
	sja1105_packing(buf, &entry->tpid2,        57,  42, size, op);
	return size;
}

static size_t
sja1105_l2_forwarding_params_entry_packing(void *buf, void *entry_ptr,
					   enum packing_op op)
{
	const size_t size = SJA1105_SIZE_L2_FORWARDING_PARAMS_ENTRY;
	struct sja1105_l2_forwarding_params_entry *entry = entry_ptr;
	int offset, i;

	for (i = 0, offset = 13; i < SJA1105_NUM_TC; i++, offset += 10)
		sja1105_packing(buf, &entry->part_spc[i],
				offset + 9, offset + 0, size, op);
	return size;
}

static size_t
sja1110_l2_forwarding_params_entry_packing(void *buf, void *entry_ptr,
					   enum packing_op op)
{
	struct sja1105_l2_forwarding_params_entry *entry = entry_ptr;
	const size_t size = SJA1105_SIZE_L2_FORWARDING_PARAMS_ENTRY;
	int offset, i;

	for (i = 0, offset = 5; i < 8; i++, offset += 11)
		sja1105_packing(buf, &entry->part_spc[i],
				offset + 10, offset + 0, size, op);
	return size;
}

static size_t sja1105_l2_forwarding_entry_packing(void *buf, void *entry_ptr,
						  enum packing_op op)
{
	const size_t size = SJA1105_SIZE_L2_FORWARDING_ENTRY;
	struct sja1105_l2_forwarding_entry *entry = entry_ptr;

	sja1105_packing(buf, &entry->bc_domain,  63, 59, size, op);
	sja1105_packing(buf, &entry->reach_port, 58, 54, size, op);
	sja1105_packing(buf, &entry->fl_domain,  53, 49, size, op);
	return size;
}

static size_t sja1110_l2_forwarding_entry_packing(void *buf, void *entry_ptr,
						  enum packing_op op)
{
	struct sja1105_l2_forwarding_entry *entry = entry_ptr;
	const size_t size = SJA1105_SIZE_L2_FORWARDING_ENTRY;

	sja1105_packing(buf, &entry->bc_domain,  63, 53, size, op);
	sja1105_packing(buf, &entry->reach_port, 52, 42, size, op);
	sja1105_packing(buf, &entry->fl_domain,  41, 31, size, op);
	return size;
}

static size_t sja1105_l2_policing_entry_packing(void *buf, void *entry_ptr,
						enum packing_op op)
{
	struct sja1105_l2_policing_entry *entry = entry_ptr;
	const size_t size = SJA1105_SIZE_L2_POLICING_ENTRY;

	sja1105_packing(buf, &entry->sharindx,  63, 58, size, op);
	sja1105_packing(buf, &entry->smax,      57, 42, size, op);
	sja1105_packing(buf, &entry->rate,      41, 26, size, op);
	sja1105_packing(buf, &entry->maxlen,    25, 15, size, op);
	sja1105_packing(buf, &entry->partition, 14, 12, size, op);
	return size;
}

static size_t sja1110_l2_policing_entry_packing(void *buf, void *entry_ptr,
						enum packing_op op)
{
	struct sja1105_l2_policing_entry *entry = entry_ptr;
	const size_t size = SJA1105_SIZE_L2_POLICING_ENTRY;

	sja1105_packing(buf, &entry->sharindx, 63, 57, size, op);
	sja1105_packing(buf, &entry->smax,     56, 39, size, op);
	sja1105_packing(buf, &entry->rate,     38, 21, size, op);
	sja1105_packing(buf, &entry->maxlen,   20, 10, size, op);
	sja1105_packing(buf, &entry->partition, 9,  7, size, op);
	return size;
}

static size_t sja1105et_mac_config_entry_packing(void *buf, void *entry_ptr,
						 enum packing_op op)
{
	const size_t size = SJA1105ET_SIZE_MAC_CONFIG_ENTRY;
	struct sja1105_mac_config_entry *entry = entry_ptr;
	int offset, i;

	for (i = 0, offset = 72; i < SJA1105_NUM_TC; i++, offset += 19) {
		sja1105_packing(buf, &entry->enabled[i],
				offset +  0, offset +  0, size, op);
		sja1105_packing(buf, &entry->base[i],
				offset +  9, offset +  1, size, op);
		sja1105_packing(buf, &entry->top[i],
				offset + 18, offset + 10, size, op);
	}
	sja1105_packing(buf, &entry->speed,     66, 65, size, op);
	sja1105_packing(buf, &entry->vlanid,    21, 10, size, op);
	sja1105_packing(buf, &entry->egress,     2,  2, size, op);
	sja1105_packing(buf, &entry->ingress,    1,  1, size, op);
	return size;
}

static size_t sja1105pqrs_mac_config_entry_packing(void *buf, void *entry_ptr,
						   enum packing_op op)
{
	const size_t size = SJA1105PQRS_SIZE_MAC_CONFIG_ENTRY;
	struct sja1105_mac_config_entry *entry = entry_ptr;
	int offset, i;

	for (i = 0, offset = 104; i < SJA1105_NUM_TC; i++, offset += 19) {
		sja1105_packing(buf, &entry->enabled[i],
				offset +  0, offset +  0, size, op);
		sja1105_packing(buf, &entry->base[i],
				offset +  9, offset +  1, size, op);
		sja1105_packing(buf, &entry->top[i],
				offset + 18, offset + 10, size, op);
	}
	sja1105_packing(buf, &entry->speed,      98, 97, size, op);
	sja1105_packing(buf, &entry->vlanid,     53, 42, size, op);
	sja1105_packing(buf, &entry->egress,     32, 32, size, op);
	sja1105_packing(buf, &entry->ingress,    31, 31, size, op);
	return size;
}

static size_t sja1110_mac_config_entry_packing(void *buf, void *entry_ptr,
					       enum packing_op op)
{
	const size_t size = SJA1105PQRS_SIZE_MAC_CONFIG_ENTRY;
	struct sja1105_mac_config_entry *entry = entry_ptr;
	int offset, i;

	for (i = 0, offset = 104; i < 8; i++, offset += 19) {
		sja1105_packing(buf, &entry->enabled[i],
				offset +  0, offset +  0, size, op);
		sja1105_packing(buf, &entry->base[i],
				offset +  9, offset +  1, size, op);
		sja1105_packing(buf, &entry->top[i],
				offset + 18, offset + 10, size, op);
	}
	sja1105_packing(buf, &entry->speed,      98, 96, size, op);
	sja1105_packing(buf, &entry->vlanid,     52, 41, size, op);
	sja1105_packing(buf, &entry->egress,     31, 31, size, op);
	sja1105_packing(buf, &entry->ingress,    30, 30, size, op);
	return size;
}

static size_t sja1105_vlan_lookup_entry_packing(void *buf, void *entry_ptr,
						enum packing_op op)
{
	const size_t size = SJA1105_SIZE_VLAN_LOOKUP_ENTRY;
	struct sja1105_vlan_lookup_entry *entry = entry_ptr;

	sja1105_packing(buf, &entry->vmemb_port, 53, 49, size, op);
	sja1105_packing(buf, &entry->vlan_bc,    48, 44, size, op);
	sja1105_packing(buf, &entry->tag_port,   43, 39, size, op);
	sja1105_packing(buf, &entry->vlanid,     38, 27, size, op);
	return size;
}

static size_t sja1110_vlan_lookup_entry_packing(void *buf, void *entry_ptr,
						enum packing_op op)
{
	struct sja1105_vlan_lookup_entry *entry = entry_ptr;
	const size_t size = SJA1110_SIZE_VLAN_LOOKUP_ENTRY;

	sja1105_packing(buf, &entry->vmemb_port, 73, 63, size, op);
	sja1105_packing(buf, &entry->vlan_bc,    62, 52, size, op);
	sja1105_packing(buf, &entry->tag_port,   51, 41, size, op);
	sja1105_packing(buf, &entry->type_entry, 40, 39, size, op);
	sja1105_packing(buf, &entry->vlanid,     38, 27, size, op);
	return size;
}

static size_t sja1105_xmii_params_entry_packing(void *buf, void *entry_ptr,
						enum packing_op op)
{
	const size_t size = SJA1105_SIZE_XMII_PARAMS_ENTRY;
	struct sja1105_xmii_params_entry *entry = entry_ptr;
	int offset, i;

	for (i = 0, offset = 17; i < SJA1105_NUM_PORTS; i++, offset += 3) {
		sja1105_packing(buf, &entry->xmii_mode[i],
				offset + 1, offset + 0, size, op);
		sja1105_packing(buf, &entry->phy_mac[i],
				offset + 2, offset + 2, size, op);
	}
	return size;
}

static size_t sja1110_xmii_params_entry_packing(void *buf, void *entry_ptr,
						enum packing_op op)
{
	const size_t size = SJA1110_SIZE_XMII_PARAMS_ENTRY;
	struct sja1105_xmii_params_entry *entry = entry_ptr;
	int offset, i;

	for (i = 0, offset = 20; i < SJA1110_NUM_PORTS; i++, offset += 4) {
		sja1105_packing(buf, &entry->xmii_mode[i],
				offset + 1, offset + 0, size, op);
		sja1105_packing(buf, &entry->phy_mac[i],
				offset + 2, offset + 2, size, op);
		sja1105_packing(buf, &entry->special[i],
				offset + 3, offset + 3, size, op);
	}
	return size;
}

static size_t sja1105_table_header_packing(void *buf, void *entry_ptr,
					   enum packing_op op)
{
	const size_t size = SJA1105_SIZE_TABLE_HEADER;
	struct sja1105_table_header *entry = entry_ptr;

	sja1105_packing(buf, &entry->block_id, 31, 24, size, op);
	sja1105_packing(buf, &entry->len,      55, 32, size, op);
	sja1105_packing(buf, &entry->crc,      95, 64, size, op);
	return size;
}

static void
sja1105_table_header_pack_with_crc(void *buf, struct sja1105_table_header *hdr)
{
	/* First pack the table as-is, then calculate the CRC, and
	 * finally put the proper CRC into the packed buffer
	 */
	memset(buf, 0, SJA1105_SIZE_TABLE_HEADER);
	sja1105_table_header_packing(buf, hdr, PACK);
	hdr->crc = sja1105_crc32(buf, SJA1105_SIZE_TABLE_HEADER - 4);
	sja1105_packing(buf + SJA1105_SIZE_TABLE_HEADER - 4, &hdr->crc,
			31, 0, 4, PACK);
}

static void sja1105_table_write_crc(u8 *table_start, u8 *crc_ptr)
{
	u64 computed_crc;
	int len_bytes;

	len_bytes = (uintptr_t)(crc_ptr - table_start);
	computed_crc = sja1105_crc32(table_start, len_bytes);
	sja1105_packing(crc_ptr, &computed_crc, 31, 0, 4, PACK);
}

/* The block IDs that the switches support are unfortunately sparse, so keep a
 * mapping table to "block indices" and translate back and forth.
 */
static const u64 blk_id_map[BLK_IDX_MAX] = {
	[BLK_IDX_L2_POLICING] = BLKID_L2_POLICING,
	[BLK_IDX_VLAN_LOOKUP] = BLKID_VLAN_LOOKUP,
	[BLK_IDX_L2_FORWARDING] = BLKID_L2_FORWARDING,
	[BLK_IDX_MAC_CONFIG] = BLKID_MAC_CONFIG,
	[BLK_IDX_L2_FORWARDING_PARAMS] = BLKID_L2_FORWARDING_PARAMS,
	[BLK_IDX_GENERAL_PARAMS] = BLKID_GENERAL_PARAMS,
	[BLK_IDX_XMII_PARAMS] = BLKID_XMII_PARAMS,
};

static void
sja1105_static_config_pack(void *buf, struct sja1105_static_config *config)
{
	struct sja1105_table_header header = {0};
	enum sja1105_blk_idx i;
	u8 *p = buf;
	int j;

	sja1105_packing(p, &config->device_id, 31, 0, 4, PACK);
	p += SJA1105_SIZE_DEVICE_ID;

	for (i = 0; i < BLK_IDX_MAX; i++) {
		const struct sja1105_table *table;
		u8 *table_start;

		table = &config->tables[i];
		if (!table->entry_count)
			continue;

		header.block_id = blk_id_map[i];
		header.len = table->entry_count *
			     table->ops->packed_entry_size / 4;
		sja1105_table_header_pack_with_crc(p, &header);
		p += SJA1105_SIZE_TABLE_HEADER;
		table_start = p;
		for (j = 0; j < table->entry_count; j++) {
			u8 *entry_ptr = table->entries;

			entry_ptr += j * table->ops->unpacked_entry_size;
			memset(p, 0, table->ops->packed_entry_size);
			table->ops->packing(p, entry_ptr, PACK);
			p += table->ops->packed_entry_size;
		}
		sja1105_table_write_crc(table_start, p);
		p += 4;
	}
	/* Final header:
	 * Block ID does not matter
	 * Length of 0 marks that header is final
	 * CRC will be replaced on-the-fly
	 */
	header.block_id = 0;
	header.len = 0;
	header.crc = 0xDEADBEEF;
	memset(p, 0, SJA1105_SIZE_TABLE_HEADER);
	sja1105_table_header_packing(p, &header, PACK);
}

static size_t
sja1105_static_config_get_length(const struct sja1105_static_config *config)
{
	unsigned int header_count;
	enum sja1105_blk_idx i;
	unsigned int sum;

	/* Ending header */
	header_count = 1;
	sum = SJA1105_SIZE_DEVICE_ID;

	/* Tables (headers and entries) */
	for (i = 0; i < BLK_IDX_MAX; i++) {
		const struct sja1105_table *table;

		table = &config->tables[i];
		if (table->entry_count)
			header_count++;

		sum += table->ops->packed_entry_size * table->entry_count;
	}
	/* Headers have an additional CRC at the end */
	sum += header_count * (SJA1105_SIZE_TABLE_HEADER + 4);
	/* Last header does not have an extra CRC because there is no data */
	sum -= 4;

	return sum;
}

/* Compatibility matrices */
static const struct sja1105_table_ops sja1105et_table_ops[BLK_IDX_MAX] = {
	[BLK_IDX_L2_POLICING] = {
		.packing = sja1105_l2_policing_entry_packing,
		.unpacked_entry_size = sizeof(struct sja1105_l2_policing_entry),
		.packed_entry_size = SJA1105_SIZE_L2_POLICING_ENTRY,
		.max_entry_count = SJA1105_MAX_L2_POLICING_COUNT,
	},
	[BLK_IDX_VLAN_LOOKUP] = {
		.packing = sja1105_vlan_lookup_entry_packing,
		.unpacked_entry_size = sizeof(struct sja1105_vlan_lookup_entry),
		.packed_entry_size = SJA1105_SIZE_VLAN_LOOKUP_ENTRY,
		.max_entry_count = SJA1105_MAX_VLAN_LOOKUP_COUNT,
	},
	[BLK_IDX_L2_FORWARDING] = {
		.packing = sja1105_l2_forwarding_entry_packing,
		.unpacked_entry_size = sizeof(struct sja1105_l2_forwarding_entry),
		.packed_entry_size = SJA1105_SIZE_L2_FORWARDING_ENTRY,
		.max_entry_count = SJA1105_MAX_L2_FORWARDING_COUNT,
	},
	[BLK_IDX_MAC_CONFIG] = {
		.packing = sja1105et_mac_config_entry_packing,
		.unpacked_entry_size = sizeof(struct sja1105_mac_config_entry),
		.packed_entry_size = SJA1105ET_SIZE_MAC_CONFIG_ENTRY,
		.max_entry_count = SJA1105_MAX_MAC_CONFIG_COUNT,
	},
	[BLK_IDX_L2_FORWARDING_PARAMS] = {
		.packing = sja1105_l2_forwarding_params_entry_packing,
		.unpacked_entry_size = sizeof(struct sja1105_l2_forwarding_params_entry),
		.packed_entry_size = SJA1105_SIZE_L2_FORWARDING_PARAMS_ENTRY,
		.max_entry_count = SJA1105_MAX_L2_FORWARDING_PARAMS_COUNT,
	},
	[BLK_IDX_GENERAL_PARAMS] = {
		.packing = sja1105et_general_params_entry_packing,
		.unpacked_entry_size = sizeof(struct sja1105_general_params_entry),
		.packed_entry_size = SJA1105ET_SIZE_GENERAL_PARAMS_ENTRY,
		.max_entry_count = SJA1105_MAX_GENERAL_PARAMS_COUNT,
	},
	[BLK_IDX_XMII_PARAMS] = {
		.packing = sja1105_xmii_params_entry_packing,
		.unpacked_entry_size = sizeof(struct sja1105_xmii_params_entry),
		.packed_entry_size = SJA1105_SIZE_XMII_PARAMS_ENTRY,
		.max_entry_count = SJA1105_MAX_XMII_PARAMS_COUNT,
	},
};

static const struct sja1105_table_ops sja1105pqrs_table_ops[BLK_IDX_MAX] = {
	[BLK_IDX_L2_POLICING] = {
		.packing = sja1105_l2_policing_entry_packing,
		.unpacked_entry_size = sizeof(struct sja1105_l2_policing_entry),
		.packed_entry_size = SJA1105_SIZE_L2_POLICING_ENTRY,
		.max_entry_count = SJA1105_MAX_L2_POLICING_COUNT,
	},
	[BLK_IDX_VLAN_LOOKUP] = {
		.packing = sja1105_vlan_lookup_entry_packing,
		.unpacked_entry_size = sizeof(struct sja1105_vlan_lookup_entry),
		.packed_entry_size = SJA1105_SIZE_VLAN_LOOKUP_ENTRY,
		.max_entry_count = SJA1105_MAX_VLAN_LOOKUP_COUNT,
	},
	[BLK_IDX_L2_FORWARDING] = {
		.packing = sja1105_l2_forwarding_entry_packing,
		.unpacked_entry_size = sizeof(struct sja1105_l2_forwarding_entry),
		.packed_entry_size = SJA1105_SIZE_L2_FORWARDING_ENTRY,
		.max_entry_count = SJA1105_MAX_L2_FORWARDING_COUNT,
	},
	[BLK_IDX_MAC_CONFIG] = {
		.packing = sja1105pqrs_mac_config_entry_packing,
		.unpacked_entry_size = sizeof(struct sja1105_mac_config_entry),
		.packed_entry_size = SJA1105PQRS_SIZE_MAC_CONFIG_ENTRY,
		.max_entry_count = SJA1105_MAX_MAC_CONFIG_COUNT,
	},
	[BLK_IDX_L2_FORWARDING_PARAMS] = {
		.packing = sja1105_l2_forwarding_params_entry_packing,
		.unpacked_entry_size = sizeof(struct sja1105_l2_forwarding_params_entry),
		.packed_entry_size = SJA1105_SIZE_L2_FORWARDING_PARAMS_ENTRY,
		.max_entry_count = SJA1105_MAX_L2_FORWARDING_PARAMS_COUNT,
	},
	[BLK_IDX_GENERAL_PARAMS] = {
		.packing = sja1105pqrs_general_params_entry_packing,
		.unpacked_entry_size = sizeof(struct sja1105_general_params_entry),
		.packed_entry_size = SJA1105PQRS_SIZE_GENERAL_PARAMS_ENTRY,
		.max_entry_count = SJA1105_MAX_GENERAL_PARAMS_COUNT,
	},
	[BLK_IDX_XMII_PARAMS] = {
		.packing = sja1105_xmii_params_entry_packing,
		.unpacked_entry_size = sizeof(struct sja1105_xmii_params_entry),
		.packed_entry_size = SJA1105_SIZE_XMII_PARAMS_ENTRY,
		.max_entry_count = SJA1105_MAX_XMII_PARAMS_COUNT,
	},
};

static const struct sja1105_table_ops sja1110_table_ops[BLK_IDX_MAX] = {
	[BLK_IDX_L2_POLICING] = {
		.packing = sja1110_l2_policing_entry_packing,
		.unpacked_entry_size = sizeof(struct sja1105_l2_policing_entry),
		.packed_entry_size = SJA1105_SIZE_L2_POLICING_ENTRY,
		.max_entry_count = SJA1110_MAX_L2_POLICING_COUNT,
	},
	[BLK_IDX_VLAN_LOOKUP] = {
		.packing = sja1110_vlan_lookup_entry_packing,
		.unpacked_entry_size = sizeof(struct sja1105_vlan_lookup_entry),
		.packed_entry_size = SJA1110_SIZE_VLAN_LOOKUP_ENTRY,
		.max_entry_count = SJA1105_MAX_VLAN_LOOKUP_COUNT,
	},
	[BLK_IDX_L2_FORWARDING] = {
		.packing = sja1110_l2_forwarding_entry_packing,
		.unpacked_entry_size = sizeof(struct sja1105_l2_forwarding_entry),
		.packed_entry_size = SJA1105_SIZE_L2_FORWARDING_ENTRY,
		.max_entry_count = SJA1110_MAX_L2_FORWARDING_COUNT,
	},
	[BLK_IDX_MAC_CONFIG] = {
		.packing = sja1110_mac_config_entry_packing,
		.unpacked_entry_size = sizeof(struct sja1105_mac_config_entry),
		.packed_entry_size = SJA1105PQRS_SIZE_MAC_CONFIG_ENTRY,
		.max_entry_count = SJA1110_MAX_MAC_CONFIG_COUNT,
	},
	[BLK_IDX_L2_FORWARDING_PARAMS] = {
		.packing = sja1110_l2_forwarding_params_entry_packing,
		.unpacked_entry_size = sizeof(struct sja1105_l2_forwarding_params_entry),
		.packed_entry_size = SJA1105_SIZE_L2_FORWARDING_PARAMS_ENTRY,
		.max_entry_count = SJA1105_MAX_L2_FORWARDING_PARAMS_COUNT,
	},
	[BLK_IDX_GENERAL_PARAMS] = {
		.packing = sja1110_general_params_entry_packing,
		.unpacked_entry_size = sizeof(struct sja1105_general_params_entry),
		.packed_entry_size = SJA1110_SIZE_GENERAL_PARAMS_ENTRY,
		.max_entry_count = SJA1105_MAX_GENERAL_PARAMS_COUNT,
	},
	[BLK_IDX_XMII_PARAMS] = {
		.packing = sja1110_xmii_params_entry_packing,
		.unpacked_entry_size = sizeof(struct sja1105_xmii_params_entry),
		.packed_entry_size = SJA1110_SIZE_XMII_PARAMS_ENTRY,
		.max_entry_count = SJA1105_MAX_XMII_PARAMS_COUNT,
	},
};

static int sja1105_init_mii_settings(struct sja1105_private *priv)
{
	struct sja1105_table *table;

	table = &priv->static_config.tables[BLK_IDX_XMII_PARAMS];

	table->entries = calloc(SJA1105_MAX_XMII_PARAMS_COUNT,
				table->ops->unpacked_entry_size);
	if (!table->entries)
		return -ENOMEM;

	/* Table will be populated at runtime */
	table->entry_count = SJA1105_MAX_XMII_PARAMS_COUNT;

	return 0;
}

static void sja1105_setup_tagging(struct sja1105_private *priv, int port)
{
	struct dsa_pdata *pdata = dev_get_uclass_plat(priv->dev);
	struct sja1105_vlan_lookup_entry *vlan;
	int cpu = pdata->cpu_port;

	/* The CPU port is implicitly configured by
	 * configuring the front-panel ports
	 */
	if (port == cpu)
		return;

	vlan = priv->static_config.tables[BLK_IDX_VLAN_LOOKUP].entries;

	priv->pvid[port] = DSA_8021Q_DIR_TX | DSA_8021Q_PORT(port);

	vlan[port].vmemb_port	= BIT(port) | BIT(cpu);
	vlan[port].vlan_bc	= BIT(port) | BIT(cpu);
	vlan[port].tag_port	= BIT(cpu);
	vlan[port].vlanid	= priv->pvid[port];
	vlan[port].type_entry	= SJA1110_VLAN_D_TAG;
}

static int sja1105_init_vlan(struct sja1105_private *priv)
{
	struct dsa_pdata *pdata = dev_get_uclass_plat(priv->dev);
	struct sja1105_table *table;
	int port;

	table = &priv->static_config.tables[BLK_IDX_VLAN_LOOKUP];

	table->entries = calloc(pdata->num_ports,
				table->ops->unpacked_entry_size);
	if (!table->entries)
		return -ENOMEM;

	table->entry_count = pdata->num_ports;

	for (port = 0; port < pdata->num_ports; port++)
		sja1105_setup_tagging(priv, port);

	return 0;
}

static void
sja1105_port_allow_traffic(struct sja1105_l2_forwarding_entry *l2_fwd,
			   int from, int to)
{
	l2_fwd[from].bc_domain  |= BIT(to);
	l2_fwd[from].reach_port |= BIT(to);
	l2_fwd[from].fl_domain  |= BIT(to);
}

static int sja1105_init_l2_forwarding(struct sja1105_private *priv)
{
	struct dsa_pdata *pdata = dev_get_uclass_plat(priv->dev);
	struct sja1105_l2_forwarding_entry *l2fwd;
	struct sja1105_table *table;
	int cpu = pdata->cpu_port;
	int i;

	table = &priv->static_config.tables[BLK_IDX_L2_FORWARDING];

	table->entries = calloc(SJA1105_MAX_L2_FORWARDING_COUNT,
				table->ops->unpacked_entry_size);
	if (!table->entries)
		return -ENOMEM;

	table->entry_count = SJA1105_MAX_L2_FORWARDING_COUNT;

	l2fwd = table->entries;

	/* First 5 entries define the forwarding rules */
	for (i = 0; i < pdata->num_ports; i++) {
		if (i == cpu)
			continue;

		sja1105_port_allow_traffic(l2fwd, i, cpu);
		sja1105_port_allow_traffic(l2fwd, cpu, i);
	}
	/* Next 8 entries define VLAN PCP mapping from ingress to egress.
	 * Leave them unpopulated (implicitly 0) but present.
	 */
	return 0;
}

static int sja1105_init_l2_forwarding_params(struct sja1105_private *priv)
{
	struct sja1105_l2_forwarding_params_entry default_l2fwd_params = {
		/* Use a single memory partition for all ingress queues */
		.part_spc = { SJA1105_MAX_FRAME_MEMORY, 0, 0, 0, 0, 0, 0, 0 },
	};
	struct sja1105_table *table;

	table = &priv->static_config.tables[BLK_IDX_L2_FORWARDING_PARAMS];

	table->entries = calloc(SJA1105_MAX_L2_FORWARDING_PARAMS_COUNT,
				table->ops->unpacked_entry_size);
	if (!table->entries)
		return -ENOMEM;

	table->entry_count = SJA1105_MAX_L2_FORWARDING_PARAMS_COUNT;

	/* This table only has a single entry */
	((struct sja1105_l2_forwarding_params_entry *)table->entries)[0] =
				default_l2fwd_params;

	return 0;
}

static int sja1105_init_general_params(struct sja1105_private *priv)
{
	struct dsa_pdata *pdata = dev_get_uclass_plat(priv->dev);
	struct sja1105_general_params_entry default_general_params = {
		/* No frame trapping */
		.mac_fltres1 = 0x0,
		.mac_flt1    = 0xffffffffffff,
		.mac_fltres0 = 0x0,
		.mac_flt0    = 0xffffffffffff,
		.host_port = pdata->num_ports,
		/* No mirroring => specify an out-of-range port value */
		.mirr_port = pdata->num_ports,
		/* No link-local trapping => specify an out-of-range port value
		 */
		.casc_port = pdata->num_ports,
		/* Force the switch to see all traffic as untagged. */
		.tpid = ETH_P_SJA1105,
		.tpid2 = ETH_P_SJA1105,
	};
	struct sja1105_table *table;

	table = &priv->static_config.tables[BLK_IDX_GENERAL_PARAMS];

	table->entries = calloc(SJA1105_MAX_GENERAL_PARAMS_COUNT,
				table->ops->unpacked_entry_size);
	if (!table->entries)
		return -ENOMEM;

	table->entry_count = SJA1105_MAX_GENERAL_PARAMS_COUNT;

	/* This table only has a single entry */
	((struct sja1105_general_params_entry *)table->entries)[0] =
				default_general_params;

	return 0;
}

static void sja1105_setup_policer(struct sja1105_l2_policing_entry *policing,
				  int index, int mtu)
{
	policing[index].sharindx = index;
	policing[index].smax = 65535; /* Burst size in bytes */
	policing[index].rate = SJA1105_RATE_MBPS(1000);
	policing[index].maxlen = mtu;
	policing[index].partition = 0;
}

static int sja1105_init_l2_policing(struct sja1105_private *priv)
{
	struct dsa_pdata *pdata = dev_get_uclass_plat(priv->dev);
	struct sja1105_l2_policing_entry *policing;
	struct sja1105_table *table;
	int cpu = pdata->cpu_port;
	int i, j, k;

	table = &priv->static_config.tables[BLK_IDX_L2_POLICING];

	table->entries = calloc(SJA1105_MAX_L2_POLICING_COUNT,
				table->ops->unpacked_entry_size);
	if (!table->entries)
		return -ENOMEM;

	table->entry_count = SJA1105_MAX_L2_POLICING_COUNT;

	policing = table->entries;

	/* k sweeps through all unicast policers (0-39).
	 * bcast sweeps through policers 40-44.
	 */
	for (i = 0, k = 0; i < pdata->num_ports; i++) {
		int bcast = (pdata->num_ports * SJA1105_NUM_TC) + i;
		int mtu = VLAN_ETH_FRAME_LEN + ETH_FCS_LEN;

		if (i == cpu)
			mtu += VLAN_HLEN;

		for (j = 0; j < SJA1105_NUM_TC; j++, k++)
			sja1105_setup_policer(policing, k, mtu);

		/* Set up this port's policer for broadcast traffic */
		sja1105_setup_policer(policing, bcast, mtu);
	}
	return 0;
}

static int sja1105_init_mac_settings(struct sja1105_private *priv)
{
	struct sja1105_mac_config_entry default_mac = {
		/* Enable 1 priority queue on egress. */
		.top  = {0x1FF, 0, 0, 0, 0, 0, 0},
		.base = {0x0, 0, 0, 0, 0, 0, 0, 0},
		.enabled = {1, 0, 0, 0, 0, 0, 0, 0},
		/* Will be overridden in sja1105_port_enable. */
		.speed = priv->info->port_speed[SJA1105_SPEED_AUTO],
		.egress = true,
		.ingress = true,
	};
	struct dsa_pdata *pdata = dev_get_uclass_plat(priv->dev);
	struct sja1105_mac_config_entry *mac;
	struct sja1105_table *table;
	int port;

	table = &priv->static_config.tables[BLK_IDX_MAC_CONFIG];

	table->entries = calloc(pdata->num_ports,
				table->ops->unpacked_entry_size);
	if (!table->entries)
		return -ENOMEM;

	table->entry_count = pdata->num_ports;

	mac = table->entries;

	for (port = 0; port < pdata->num_ports; port++) {
		mac[port] = default_mac;
		/* Internal VLAN (pvid) to apply to untagged ingress */
		mac[port].vlanid = priv->pvid[port];
	}

	return 0;
}

static int sja1105_static_config_init(struct sja1105_private *priv)
{
	struct sja1105_static_config *config = &priv->static_config;
	const struct sja1105_table_ops *static_ops = priv->info->static_ops;
	u64 device_id = priv->info->device_id;
	enum sja1105_blk_idx i;
	int rc;

	*config = (struct sja1105_static_config) {0};

	/* Transfer static_ops array from priv into per-table ops
	 * for handier access
	 */
	for (i = 0; i < BLK_IDX_MAX; i++)
		config->tables[i].ops = &static_ops[i];

	config->device_id = device_id;

	/* Build initial static configuration, to be fixed up during runtime */
	rc = sja1105_init_vlan(priv);
	if (rc < 0)
		return rc;
	rc = sja1105_init_mac_settings(priv);
	if (rc < 0)
		return rc;
	rc = sja1105_init_mii_settings(priv);
	if (rc < 0)
		return rc;
	rc = sja1105_init_l2_forwarding(priv);
	if (rc < 0)
		return rc;
	rc = sja1105_init_l2_forwarding_params(priv);
	if (rc < 0)
		return rc;
	rc = sja1105_init_l2_policing(priv);
	if (rc < 0)
		return rc;
	rc = sja1105_init_general_params(priv);
	if (rc < 0)
		return rc;

	return 0;
}

static void sja1105_static_config_free(struct sja1105_static_config *config)
{
	enum sja1105_blk_idx i;

	for (i = 0; i < BLK_IDX_MAX; i++) {
		if (config->tables[i].entry_count) {
			free(config->tables[i].entries);
			config->tables[i].entry_count = 0;
		}
	}
}

static void sja1105_cgu_idiv_packing(void *buf, struct sja1105_cgu_idiv *idiv,
				     enum packing_op op)
{
	const int size = 4;

	sja1105_packing(buf, &idiv->clksrc,    28, 24, size, op);
	sja1105_packing(buf, &idiv->autoblock, 11, 11, size, op);
	sja1105_packing(buf, &idiv->idiv,       5,  2, size, op);
	sja1105_packing(buf, &idiv->pd,         0,  0, size, op);
}

static int sja1105_cgu_idiv_config(struct sja1105_private *priv, int port,
				   bool enabled, int factor)
{
	const struct sja1105_regs *regs = priv->info->regs;
	u8 packed_buf[SJA1105_SIZE_CGU_CMD] = {0};
	struct sja1105_cgu_idiv idiv;

	if (regs->cgu_idiv[port] == SJA1105_RSV_ADDR)
		return 0;

	if (enabled && factor != 1 && factor != 10)
		return -ERANGE;

	/* Payload for packed_buf */
	idiv.clksrc    = 0x0A;            /* 25MHz */
	idiv.autoblock = 1;               /* Block clk automatically */
	idiv.idiv      = factor - 1;      /* Divide by 1 or 10 */
	idiv.pd        = enabled ? 0 : 1; /* Power down? */
	sja1105_cgu_idiv_packing(packed_buf, &idiv, PACK);

	return sja1105_xfer_buf(priv, SPI_WRITE, regs->cgu_idiv[port],
				packed_buf, SJA1105_SIZE_CGU_CMD);
}

static void
sja1105_cgu_mii_control_packing(void *buf, struct sja1105_cgu_mii_ctrl *cmd,
				enum packing_op op)
{
	const int size = 4;

	sja1105_packing(buf, &cmd->clksrc,    28, 24, size, op);
	sja1105_packing(buf, &cmd->autoblock, 11, 11, size, op);
	sja1105_packing(buf, &cmd->pd,         0,  0, size, op);
}

static int sja1105_cgu_mii_tx_clk_config(struct sja1105_private *priv,
					 int port, sja1105_mii_role_t role)
{
	const struct sja1105_regs *regs = priv->info->regs;
	struct sja1105_cgu_mii_ctrl mii_tx_clk;
	const int mac_clk_sources[] = {
		CLKSRC_MII0_TX_CLK,
		CLKSRC_MII1_TX_CLK,
		CLKSRC_MII2_TX_CLK,
		CLKSRC_MII3_TX_CLK,
		CLKSRC_MII4_TX_CLK,
	};
	const int phy_clk_sources[] = {
		CLKSRC_IDIV0,
		CLKSRC_IDIV1,
		CLKSRC_IDIV2,
		CLKSRC_IDIV3,
		CLKSRC_IDIV4,
	};
	u8 packed_buf[SJA1105_SIZE_CGU_CMD] = {0};
	int clksrc;

	if (regs->mii_tx_clk[port] == SJA1105_RSV_ADDR)
		return 0;

	if (role == XMII_MAC)
		clksrc = mac_clk_sources[port];
	else
		clksrc = phy_clk_sources[port];

	/* Payload for packed_buf */
	mii_tx_clk.clksrc    = clksrc;
	mii_tx_clk.autoblock = 1;  /* Autoblock clk while changing clksrc */
	mii_tx_clk.pd        = 0;  /* Power Down off => enabled */
	sja1105_cgu_mii_control_packing(packed_buf, &mii_tx_clk, PACK);

	return sja1105_xfer_buf(priv, SPI_WRITE, regs->mii_tx_clk[port],
				packed_buf, SJA1105_SIZE_CGU_CMD);
}

static int
sja1105_cgu_mii_rx_clk_config(struct sja1105_private *priv, int port)
{
	const struct sja1105_regs *regs = priv->info->regs;
	u8 packed_buf[SJA1105_SIZE_CGU_CMD] = {0};
	struct sja1105_cgu_mii_ctrl mii_rx_clk;
	const int clk_sources[] = {
		CLKSRC_MII0_RX_CLK,
		CLKSRC_MII1_RX_CLK,
		CLKSRC_MII2_RX_CLK,
		CLKSRC_MII3_RX_CLK,
		CLKSRC_MII4_RX_CLK,
	};

	if (regs->mii_rx_clk[port] == SJA1105_RSV_ADDR)
		return 0;

	/* Payload for packed_buf */
	mii_rx_clk.clksrc    = clk_sources[port];
	mii_rx_clk.autoblock = 1;  /* Autoblock clk while changing clksrc */
	mii_rx_clk.pd        = 0;  /* Power Down off => enabled */
	sja1105_cgu_mii_control_packing(packed_buf, &mii_rx_clk, PACK);

	return sja1105_xfer_buf(priv, SPI_WRITE, regs->mii_rx_clk[port],
				packed_buf, SJA1105_SIZE_CGU_CMD);
}

static int
sja1105_cgu_mii_ext_tx_clk_config(struct sja1105_private *priv, int port)
{
	const struct sja1105_regs *regs = priv->info->regs;
	struct sja1105_cgu_mii_ctrl mii_ext_tx_clk;
	u8 packed_buf[SJA1105_SIZE_CGU_CMD] = {0};
	const int clk_sources[] = {
		CLKSRC_IDIV0,
		CLKSRC_IDIV1,
		CLKSRC_IDIV2,
		CLKSRC_IDIV3,
		CLKSRC_IDIV4,
	};

	if (regs->mii_ext_tx_clk[port] == SJA1105_RSV_ADDR)
		return 0;

	/* Payload for packed_buf */
	mii_ext_tx_clk.clksrc    = clk_sources[port];
	mii_ext_tx_clk.autoblock = 1; /* Autoblock clk while changing clksrc */
	mii_ext_tx_clk.pd        = 0; /* Power Down off => enabled */
	sja1105_cgu_mii_control_packing(packed_buf, &mii_ext_tx_clk, PACK);

	return sja1105_xfer_buf(priv, SPI_WRITE, regs->mii_ext_tx_clk[port],
				packed_buf, SJA1105_SIZE_CGU_CMD);
}

static int
sja1105_cgu_mii_ext_rx_clk_config(struct sja1105_private *priv, int port)
{
	const struct sja1105_regs *regs = priv->info->regs;
	struct sja1105_cgu_mii_ctrl mii_ext_rx_clk;
	u8 packed_buf[SJA1105_SIZE_CGU_CMD] = {0};
	const int clk_sources[] = {
		CLKSRC_IDIV0,
		CLKSRC_IDIV1,
		CLKSRC_IDIV2,
		CLKSRC_IDIV3,
		CLKSRC_IDIV4,
	};

	if (regs->mii_ext_rx_clk[port] == SJA1105_RSV_ADDR)
		return 0;

	/* Payload for packed_buf */
	mii_ext_rx_clk.clksrc    = clk_sources[port];
	mii_ext_rx_clk.autoblock = 1; /* Autoblock clk while changing clksrc */
	mii_ext_rx_clk.pd        = 0; /* Power Down off => enabled */
	sja1105_cgu_mii_control_packing(packed_buf, &mii_ext_rx_clk, PACK);

	return sja1105_xfer_buf(priv, SPI_WRITE, regs->mii_ext_rx_clk[port],
				packed_buf, SJA1105_SIZE_CGU_CMD);
}

static int sja1105_mii_clocking_setup(struct sja1105_private *priv, int port,
				      sja1105_mii_role_t role)
{
	int rc;

	rc = sja1105_cgu_idiv_config(priv, port, (role == XMII_PHY), 1);
	if (rc < 0)
		return rc;

	rc = sja1105_cgu_mii_tx_clk_config(priv, port, role);
	if (rc < 0)
		return rc;

	rc = sja1105_cgu_mii_rx_clk_config(priv, port);
	if (rc < 0)
		return rc;

	if (role == XMII_PHY) {
		rc = sja1105_cgu_mii_ext_tx_clk_config(priv, port);
		if (rc < 0)
			return rc;

		rc = sja1105_cgu_mii_ext_rx_clk_config(priv, port);
		if (rc < 0)
			return rc;
	}
	return 0;
}

static void
sja1105_cgu_pll_control_packing(void *buf, struct sja1105_cgu_pll_ctrl *cmd,
				enum packing_op op)
{
	const int size = 4;

	sja1105_packing(buf, &cmd->pllclksrc, 28, 24, size, op);
	sja1105_packing(buf, &cmd->msel,      23, 16, size, op);
	sja1105_packing(buf, &cmd->autoblock, 11, 11, size, op);
	sja1105_packing(buf, &cmd->psel,       9,  8, size, op);
	sja1105_packing(buf, &cmd->direct,     7,  7, size, op);
	sja1105_packing(buf, &cmd->fbsel,      6,  6, size, op);
	sja1105_packing(buf, &cmd->bypass,     1,  1, size, op);
	sja1105_packing(buf, &cmd->pd,         0,  0, size, op);
}

static int sja1105_cgu_rgmii_tx_clk_config(struct sja1105_private *priv,
					   int port, u64 speed)
{
	const struct sja1105_regs *regs = priv->info->regs;
	struct sja1105_cgu_mii_ctrl txc;
	u8 packed_buf[SJA1105_SIZE_CGU_CMD] = {0};
	int clksrc;

	if (regs->rgmii_tx_clk[port] == SJA1105_RSV_ADDR)
		return 0;

	if (speed == priv->info->port_speed[SJA1105_SPEED_1000MBPS]) {
		clksrc = CLKSRC_PLL0;
	} else {
		int clk_sources[] = {CLKSRC_IDIV0, CLKSRC_IDIV1, CLKSRC_IDIV2,
				     CLKSRC_IDIV3, CLKSRC_IDIV4};
		clksrc = clk_sources[port];
	}

	/* RGMII: 125MHz for 1000, 25MHz for 100, 2.5MHz for 10 */
	txc.clksrc = clksrc;
	/* Autoblock clk while changing clksrc */
	txc.autoblock = 1;
	/* Power Down off => enabled */
	txc.pd = 0;
	sja1105_cgu_mii_control_packing(packed_buf, &txc, PACK);

	return sja1105_xfer_buf(priv, SPI_WRITE, regs->rgmii_tx_clk[port],
				packed_buf, SJA1105_SIZE_CGU_CMD);
}

/* AGU */
static void
sja1105_cfg_pad_mii_packing(void *buf, struct sja1105_cfg_pad_mii *cmd,
			    enum packing_op op)
{
	const int size = 4;

	sja1105_packing(buf, &cmd->d32_os,   28, 27, size, op);
	sja1105_packing(buf, &cmd->d32_ih,   26, 26, size, op);
	sja1105_packing(buf, &cmd->d32_ipud, 25, 24, size, op);
	sja1105_packing(buf, &cmd->d10_os,   20, 19, size, op);
	sja1105_packing(buf, &cmd->d10_ih,   18, 18, size, op);
	sja1105_packing(buf, &cmd->d10_ipud, 17, 16, size, op);
	sja1105_packing(buf, &cmd->ctrl_os,  12, 11, size, op);
	sja1105_packing(buf, &cmd->ctrl_ih,  10, 10, size, op);
	sja1105_packing(buf, &cmd->ctrl_ipud, 9,  8, size, op);
	sja1105_packing(buf, &cmd->clk_os,    4,  3, size, op);
	sja1105_packing(buf, &cmd->clk_ih,    2,  2, size, op);
	sja1105_packing(buf, &cmd->clk_ipud,  1,  0, size, op);
}

static void
sja1110_cfg_pad_mii_id_packing(void *buf, struct sja1105_cfg_pad_mii_id *cmd,
			       enum packing_op op)
{
	const int size = SJA1105_SIZE_CGU_CMD;
	u64 range = 4;

	/* Fields RXC_RANGE and TXC_RANGE select the input frequency range:
	 * 0 = 2.5MHz
	 * 1 = 25MHz
	 * 2 = 50MHz
	 * 3 = 125MHz
	 * 4 = Automatically determined by port speed.
	 * There's no point in defining a structure different than the one for
	 * SJA1105, so just hardcode the frequency range to automatic, just as
	 * before.
	 */
	sja1105_packing(buf, &cmd->rxc_stable_ovr, 26, 26, size, op);
	sja1105_packing(buf, &cmd->rxc_delay,      25, 21, size, op);
	sja1105_packing(buf, &range,               20, 18, size, op);
	sja1105_packing(buf, &cmd->rxc_bypass,     17, 17, size, op);
	sja1105_packing(buf, &cmd->rxc_pd,         16, 16, size, op);
	sja1105_packing(buf, &cmd->txc_stable_ovr, 10, 10, size, op);
	sja1105_packing(buf, &cmd->txc_delay,       9,  5, size, op);
	sja1105_packing(buf, &range,                4,  2, size, op);
	sja1105_packing(buf, &cmd->txc_bypass,      1,  1, size, op);
	sja1105_packing(buf, &cmd->txc_pd,          0,  0, size, op);
}

static int sja1105_rgmii_cfg_pad_tx_config(struct sja1105_private *priv,
					   int port)
{
	const struct sja1105_regs *regs = priv->info->regs;
	struct sja1105_cfg_pad_mii pad_mii_tx = {0};
	u8 packed_buf[SJA1105_SIZE_CGU_CMD] = {0};

	if (regs->pad_mii_tx[port] == SJA1105_RSV_ADDR)
		return 0;

	/* Payload */
	pad_mii_tx.d32_os    = 3; /* TXD[3:2] output stage: */
				  /*          high noise/high speed */
	pad_mii_tx.d10_os    = 3; /* TXD[1:0] output stage: */
				  /*          high noise/high speed */
	pad_mii_tx.d32_ipud  = 2; /* TXD[3:2] input stage: */
				  /*          plain input (default) */
	pad_mii_tx.d10_ipud  = 2; /* TXD[1:0] input stage: */
				  /*          plain input (default) */
	pad_mii_tx.ctrl_os   = 3; /* TX_CTL / TX_ER output stage */
	pad_mii_tx.ctrl_ipud = 2; /* TX_CTL / TX_ER input stage (default) */
	pad_mii_tx.clk_os    = 3; /* TX_CLK output stage */
	pad_mii_tx.clk_ih    = 0; /* TX_CLK input hysteresis (default) */
	pad_mii_tx.clk_ipud  = 2; /* TX_CLK input stage (default) */
	sja1105_cfg_pad_mii_packing(packed_buf, &pad_mii_tx, PACK);

	return sja1105_xfer_buf(priv, SPI_WRITE, regs->pad_mii_tx[port],
				packed_buf, SJA1105_SIZE_CGU_CMD);
}

static int sja1105_cfg_pad_rx_config(struct sja1105_private *priv, int port)
{
	const struct sja1105_regs *regs = priv->info->regs;
	struct sja1105_cfg_pad_mii pad_mii_rx = {0};
	u8 packed_buf[SJA1105_SIZE_CGU_CMD] = {0};

	if (regs->pad_mii_rx[port] == SJA1105_RSV_ADDR)
		return 0;

	/* Payload */
	pad_mii_rx.d32_ih    = 0; /* RXD[3:2] input stage hysteresis: */
				  /*          non-Schmitt (default) */
	pad_mii_rx.d32_ipud  = 2; /* RXD[3:2] input weak pull-up/down */
				  /*          plain input (default) */
	pad_mii_rx.d10_ih    = 0; /* RXD[1:0] input stage hysteresis: */
				  /*          non-Schmitt (default) */
	pad_mii_rx.d10_ipud  = 2; /* RXD[1:0] input weak pull-up/down */
				  /*          plain input (default) */
	pad_mii_rx.ctrl_ih   = 0; /* RX_DV/CRS_DV/RX_CTL and RX_ER */
				  /* input stage hysteresis: */
				  /* non-Schmitt (default) */
	pad_mii_rx.ctrl_ipud = 3; /* RX_DV/CRS_DV/RX_CTL and RX_ER */
				  /* input stage weak pull-up/down: */
				  /* pull-down */
	pad_mii_rx.clk_os    = 2; /* RX_CLK/RXC output stage: */
				  /* medium noise/fast speed (default) */
	pad_mii_rx.clk_ih    = 0; /* RX_CLK/RXC input hysteresis: */
				  /* non-Schmitt (default) */
	pad_mii_rx.clk_ipud  = 2; /* RX_CLK/RXC input pull-up/down: */
				  /* plain input (default) */
	sja1105_cfg_pad_mii_packing(packed_buf, &pad_mii_rx, PACK);

	return sja1105_xfer_buf(priv, SPI_WRITE, regs->pad_mii_rx[port],
				packed_buf, SJA1105_SIZE_CGU_CMD);
}

static void
sja1105_cfg_pad_mii_id_packing(void *buf, struct sja1105_cfg_pad_mii_id *cmd,
			       enum packing_op op)
{
	const int size = SJA1105_SIZE_CGU_CMD;

	sja1105_packing(buf, &cmd->rxc_stable_ovr, 15, 15, size, op);
	sja1105_packing(buf, &cmd->rxc_delay,      14, 10, size, op);
	sja1105_packing(buf, &cmd->rxc_bypass,      9,  9, size, op);
	sja1105_packing(buf, &cmd->rxc_pd,          8,  8, size, op);
	sja1105_packing(buf, &cmd->txc_stable_ovr,  7,  7, size, op);
	sja1105_packing(buf, &cmd->txc_delay,       6,  2, size, op);
	sja1105_packing(buf, &cmd->txc_bypass,      1,  1, size, op);
	sja1105_packing(buf, &cmd->txc_pd,          0,  0, size, op);
}

/* Valid range in degrees is an integer between 73.8 and 101.7 */
static u64 sja1105_rgmii_delay(u64 phase)
{
	/* UM11040.pdf: The delay in degree phase is 73.8 + delay_tune * 0.9.
	 * To avoid floating point operations we'll multiply by 10
	 * and get 1 decimal point precision.
	 */
	phase *= 10;
	return (phase - 738) / 9;
}

static int sja1105pqrs_setup_rgmii_delay(struct sja1105_private *priv, int port)
{
	const struct sja1105_regs *regs = priv->info->regs;
	struct sja1105_cfg_pad_mii_id pad_mii_id = {0};
	u8 packed_buf[SJA1105_SIZE_CGU_CMD] = {0};
	int rc;

	if (priv->rgmii_rx_delay[port])
		pad_mii_id.rxc_delay = sja1105_rgmii_delay(90);
	if (priv->rgmii_tx_delay[port])
		pad_mii_id.txc_delay = sja1105_rgmii_delay(90);

	/* Stage 1: Turn the RGMII delay lines off. */
	pad_mii_id.rxc_bypass = 1;
	pad_mii_id.rxc_pd = 1;
	pad_mii_id.txc_bypass = 1;
	pad_mii_id.txc_pd = 1;
	sja1105_cfg_pad_mii_id_packing(packed_buf, &pad_mii_id, PACK);

	rc = sja1105_xfer_buf(priv, SPI_WRITE, regs->pad_mii_id[port],
			      packed_buf, SJA1105_SIZE_CGU_CMD);
	if (rc < 0)
		return rc;

	/* Stage 2: Turn the RGMII delay lines on. */
	if (priv->rgmii_rx_delay[port]) {
		pad_mii_id.rxc_bypass = 0;
		pad_mii_id.rxc_pd = 0;
	}
	if (priv->rgmii_tx_delay[port]) {
		pad_mii_id.txc_bypass = 0;
		pad_mii_id.txc_pd = 0;
	}
	sja1105_cfg_pad_mii_id_packing(packed_buf, &pad_mii_id, PACK);

	return sja1105_xfer_buf(priv, SPI_WRITE, regs->pad_mii_id[port],
				packed_buf, SJA1105_SIZE_CGU_CMD);
}

static int sja1110_setup_rgmii_delay(struct sja1105_private *priv, int port)
{
	const struct sja1105_regs *regs = priv->info->regs;
	struct sja1105_cfg_pad_mii_id pad_mii_id = {0};
	u8 packed_buf[SJA1105_SIZE_CGU_CMD] = {0};

	pad_mii_id.rxc_pd = 1;
	pad_mii_id.txc_pd = 1;

	if (priv->rgmii_rx_delay[port]) {
		pad_mii_id.rxc_delay = sja1105_rgmii_delay(90);
		/* The "BYPASS" bit in SJA1110 is actually a "don't bypass" */
		pad_mii_id.rxc_bypass = 1;
		pad_mii_id.rxc_pd = 0;
	}

	if (priv->rgmii_tx_delay[port]) {
		pad_mii_id.txc_delay = sja1105_rgmii_delay(90);
		pad_mii_id.txc_bypass = 1;
		pad_mii_id.txc_pd = 0;
	}

	sja1110_cfg_pad_mii_id_packing(packed_buf, &pad_mii_id, PACK);

	return sja1105_xfer_buf(priv, SPI_WRITE, regs->pad_mii_id[port],
				packed_buf, SJA1105_SIZE_CGU_CMD);
}

static int sja1105_rgmii_clocking_setup(struct sja1105_private *priv, int port,
					sja1105_mii_role_t role)
{
	struct sja1105_mac_config_entry *mac;
	struct udevice *dev = priv->dev;
	u64 speed;
	int rc;

	mac = priv->static_config.tables[BLK_IDX_MAC_CONFIG].entries;
	speed = mac[port].speed;

	if (speed == priv->info->port_speed[SJA1105_SPEED_1000MBPS]) {
		/* 1000Mbps, IDIV disabled (125 MHz) */
		rc = sja1105_cgu_idiv_config(priv, port, false, 1);
	} else if (speed == priv->info->port_speed[SJA1105_SPEED_100MBPS]) {
		/* 100Mbps, IDIV enabled, divide by 1 (25 MHz) */
		rc = sja1105_cgu_idiv_config(priv, port, true, 1);
	} else if (speed == priv->info->port_speed[SJA1105_SPEED_10MBPS]) {
		/* 10Mbps, IDIV enabled, divide by 10 (2.5 MHz) */
		rc = sja1105_cgu_idiv_config(priv, port, true, 10);
	} else if (speed == priv->info->port_speed[SJA1105_SPEED_AUTO]) {
		/* Skip CGU configuration if there is no speed available
		 * (e.g. link is not established yet)
		 */
		dev_dbg(dev, "Speed not available, skipping CGU config\n");
		return 0;
	} else {
		rc = -EINVAL;
	}

	if (rc < 0) {
		dev_err(dev, "Failed to configure idiv\n");
		return rc;
	}
	rc = sja1105_cgu_rgmii_tx_clk_config(priv, port, speed);
	if (rc < 0) {
		dev_err(dev, "Failed to configure RGMII Tx clock\n");
		return rc;
	}
	rc = sja1105_rgmii_cfg_pad_tx_config(priv, port);
	if (rc < 0) {
		dev_err(dev, "Failed to configure Tx pad registers\n");
		return rc;
	}

	if (!priv->info->setup_rgmii_delay)
		return 0;

	return priv->info->setup_rgmii_delay(priv, port);
}

static int sja1105_cgu_rmii_ref_clk_config(struct sja1105_private *priv,
					   int port)
{
	const struct sja1105_regs *regs = priv->info->regs;
	u8 packed_buf[SJA1105_SIZE_CGU_CMD] = {0};
	struct sja1105_cgu_mii_ctrl ref_clk;
	const int clk_sources[] = {
		CLKSRC_MII0_TX_CLK,
		CLKSRC_MII1_TX_CLK,
		CLKSRC_MII2_TX_CLK,
		CLKSRC_MII3_TX_CLK,
		CLKSRC_MII4_TX_CLK,
	};

	if (regs->rmii_ref_clk[port] == SJA1105_RSV_ADDR)
		return 0;

	/* Payload for packed_buf */
	ref_clk.clksrc    = clk_sources[port];
	ref_clk.autoblock = 1;      /* Autoblock clk while changing clksrc */
	ref_clk.pd        = 0;      /* Power Down off => enabled */
	sja1105_cgu_mii_control_packing(packed_buf, &ref_clk, PACK);

	return sja1105_xfer_buf(priv, SPI_WRITE, regs->rmii_ref_clk[port],
				packed_buf, SJA1105_SIZE_CGU_CMD);
}

static int
sja1105_cgu_rmii_ext_tx_clk_config(struct sja1105_private *priv, int port)
{
	const struct sja1105_regs *regs = priv->info->regs;
	struct sja1105_cgu_mii_ctrl ext_tx_clk;
	u8 packed_buf[SJA1105_SIZE_CGU_CMD] = {0};

	if (regs->rmii_ext_tx_clk[port] == SJA1105_RSV_ADDR)
		return 0;

	/* Payload for packed_buf */
	ext_tx_clk.clksrc    = CLKSRC_PLL1;
	ext_tx_clk.autoblock = 1;   /* Autoblock clk while changing clksrc */
	ext_tx_clk.pd        = 0;   /* Power Down off => enabled */
	sja1105_cgu_mii_control_packing(packed_buf, &ext_tx_clk, PACK);

	return sja1105_xfer_buf(priv, SPI_WRITE, regs->rmii_ext_tx_clk[port],
				packed_buf, SJA1105_SIZE_CGU_CMD);
}

static int sja1105_cgu_rmii_pll_config(struct sja1105_private *priv)
{
	const struct sja1105_regs *regs = priv->info->regs;
	u8 packed_buf[SJA1105_SIZE_CGU_CMD] = {0};
	struct sja1105_cgu_pll_ctrl pll = {0};
	int rc;

	if (regs->rmii_pll1 == SJA1105_RSV_ADDR)
		return 0;

	/* Step 1: PLL1 setup for 50Mhz */
	pll.pllclksrc = 0xA;
	pll.msel      = 0x1;
	pll.autoblock = 0x1;
	pll.psel      = 0x1;
	pll.direct    = 0x0;
	pll.fbsel     = 0x1;
	pll.bypass    = 0x0;
	pll.pd        = 0x1;

	sja1105_cgu_pll_control_packing(packed_buf, &pll, PACK);
	rc = sja1105_xfer_buf(priv, SPI_WRITE, regs->rmii_pll1, packed_buf,
			      SJA1105_SIZE_CGU_CMD);
	if (rc < 0)
		return rc;

	/* Step 2: Enable PLL1 */
	pll.pd = 0x0;

	sja1105_cgu_pll_control_packing(packed_buf, &pll, PACK);
	rc = sja1105_xfer_buf(priv, SPI_WRITE, regs->rmii_pll1, packed_buf,
			      SJA1105_SIZE_CGU_CMD);
	return rc;
}

static int sja1105_rmii_clocking_setup(struct sja1105_private *priv, int port,
				       sja1105_mii_role_t role)
{
	int rc;

	/* AH1601.pdf chapter 2.5.1. Sources */
	if (role == XMII_MAC) {
		/* Configure and enable PLL1 for 50Mhz output */
		rc = sja1105_cgu_rmii_pll_config(priv);
		if (rc < 0)
			return rc;
	}
	/* Disable IDIV for this port */
	rc = sja1105_cgu_idiv_config(priv, port, false, 1);
	if (rc < 0)
		return rc;
	/* Source to sink mappings */
	rc = sja1105_cgu_rmii_ref_clk_config(priv, port);
	if (rc < 0)
		return rc;
	if (role == XMII_MAC) {
		rc = sja1105_cgu_rmii_ext_tx_clk_config(priv, port);
		if (rc < 0)
			return rc;
	}
	return 0;
}

static int sja1105_pcs_read(struct sja1105_private *priv, int addr,
			    int devad, int regnum)
{
	return priv->mdio_pcs->read(priv->mdio_pcs, addr, devad, regnum);
}

static int sja1105_pcs_write(struct sja1105_private *priv, int addr,
			     int devad, int regnum, u16 val)
{
	return priv->mdio_pcs->write(priv->mdio_pcs, addr, devad, regnum, val);
}

/* In NXP SJA1105, the PCS is integrated with a PMA that has the TX lane
 * polarity inverted by default (PLUS is MINUS, MINUS is PLUS). To obtain
 * normal non-inverted behavior, the TX lane polarity must be inverted in the
 * PCS, via the DIGITAL_CONTROL_2 register.
 */
static int sja1105_pma_config(struct sja1105_private *priv, int port)
{
	return sja1105_pcs_write(priv, port, MDIO_MMD_VEND2,
				 DW_VR_MII_DIG_CTRL2,
				 DW_VR_MII_DIG_CTRL2_TX_POL_INV);
}

static int sja1110_pma_config(struct sja1105_private *priv, int port)
{
	u16 txpll_fbdiv = 0x19, txpll_refdiv = 0x1;
	u16 rxpll_fbdiv = 0x19, rxpll_refdiv = 0x1;
	u16 rx_cdr_ctle = 0x212a;
	u16 val;
	int rc;

	/* Program TX PLL feedback divider and reference divider settings for
	 * correct oscillation frequency.
	 */
	rc = sja1105_pcs_write(priv, port, MDIO_MMD_VEND2, SJA1110_TXPLL_CTRL0,
			       SJA1110_TXPLL_FBDIV(txpll_fbdiv));
	if (rc < 0)
		return rc;

	rc = sja1105_pcs_write(priv, port, MDIO_MMD_VEND2, SJA1110_TXPLL_CTRL1,
			       SJA1110_TXPLL_REFDIV(txpll_refdiv));
	if (rc < 0)
		return rc;

	/* Program transmitter amplitude and disable amplitude trimming */
	rc = sja1105_pcs_write(priv, port, MDIO_MMD_VEND2,
			       SJA1110_LANE_DRIVER1_0, SJA1110_TXDRV(0x5));
	if (rc < 0)
		return rc;

	val = SJA1110_TXDRVTRIM_LSB(0xffffffull);

	rc = sja1105_pcs_write(priv, port, MDIO_MMD_VEND2,
			       SJA1110_LANE_DRIVER2_0, val);
	if (rc < 0)
		return rc;

	val = SJA1110_TXDRVTRIM_MSB(0xffffffull) | SJA1110_LANE_DRIVER2_1_RSV;

	rc = sja1105_pcs_write(priv, port, MDIO_MMD_VEND2,
			       SJA1110_LANE_DRIVER2_1, val);
	if (rc < 0)
		return rc;

	/* Enable input and output resistor terminations for low BER. */
	val = SJA1110_ACCOUPLE_RXVCM_EN | SJA1110_CDR_GAIN |
	      SJA1110_RXRTRIM(4) | SJA1110_RXTEN | SJA1110_TXPLL_BWSEL |
	      SJA1110_TXRTRIM(3) | SJA1110_TXTEN;

	rc = sja1105_pcs_write(priv, port, MDIO_MMD_VEND2, SJA1110_LANE_TRIM,
			       val);
	if (rc < 0)
		return rc;

	/* Select PCS as transmitter data source. */
	rc = sja1105_pcs_write(priv, port, MDIO_MMD_VEND2,
			       SJA1110_LANE_DATAPATH_1, 0);
	if (rc < 0)
		return rc;

	/* Program RX PLL feedback divider and reference divider for correct
	 * oscillation frequency.
	 */
	rc = sja1105_pcs_write(priv, port, MDIO_MMD_VEND2, SJA1110_RXPLL_CTRL0,
			       SJA1110_RXPLL_FBDIV(rxpll_fbdiv));
	if (rc < 0)
		return rc;

	rc = sja1105_pcs_write(priv, port, MDIO_MMD_VEND2, SJA1110_RXPLL_CTRL1,
			       SJA1110_RXPLL_REFDIV(rxpll_refdiv));
	if (rc < 0)
		return rc;

	/* Program threshold for receiver signal detector.
	 * Enable control of RXPLL by receiver signal detector to disable RXPLL
	 * when an input signal is not present.
	 */
	rc = sja1105_pcs_write(priv, port, MDIO_MMD_VEND2,
			       SJA1110_RX_DATA_DETECT, 0x0005);
	if (rc < 0)
		return rc;

	/* Enable TX and RX PLLs and circuits.
	 * Release reset of PMA to enable data flow to/from PCS.
	 */
	rc = sja1105_pcs_read(priv, port, MDIO_MMD_VEND2,
			      SJA1110_POWERDOWN_ENABLE);
	if (rc < 0)
		return rc;

	val = rc & ~(SJA1110_TXPLL_PD | SJA1110_TXPD | SJA1110_RXCH_PD |
		     SJA1110_RXBIAS_PD | SJA1110_RESET_SER_EN |
		     SJA1110_RESET_SER | SJA1110_RESET_DES);
	val |= SJA1110_RXPKDETEN | SJA1110_RCVEN;

	rc = sja1105_pcs_write(priv, port, MDIO_MMD_VEND2,
			       SJA1110_POWERDOWN_ENABLE, val);
	if (rc < 0)
		return rc;

	/* Program continuous-time linear equalizer (CTLE) settings. */
	rc = sja1105_pcs_write(priv, port, MDIO_MMD_VEND2, SJA1110_RX_CDR_CTLE,
			       rx_cdr_ctle);
	if (rc < 0)
		return rc;

	return 0;
}

static int sja1105_xpcs_config_aneg_c37_sgmii(struct sja1105_private *priv,
					      int port)
{
	int rc;

	rc = sja1105_pcs_read(priv, port, MDIO_MMD_VEND2, MDIO_CTRL1);
	if (rc < 0)
		return rc;
	rc &= ~MDIO_AN_CTRL1_ENABLE;
	rc = sja1105_pcs_write(priv, port, MDIO_MMD_VEND2, MDIO_CTRL1,
			       rc);
	if (rc < 0)
		return rc;

	rc = sja1105_pcs_read(priv, port, MDIO_MMD_VEND2, DW_VR_MII_AN_CTRL);
	if (rc < 0)
		return rc;

	rc &= ~(DW_VR_MII_PCS_MODE_MASK | DW_VR_MII_TX_CONFIG_MASK);
	rc |= (DW_VR_MII_PCS_MODE_C37_SGMII <<
	       DW_VR_MII_AN_CTRL_PCS_MODE_SHIFT &
	       DW_VR_MII_PCS_MODE_MASK);
	rc |= (DW_VR_MII_TX_CONFIG_MAC_SIDE_SGMII <<
	       DW_VR_MII_AN_CTRL_TX_CONFIG_SHIFT &
	       DW_VR_MII_TX_CONFIG_MASK);
	rc = sja1105_pcs_write(priv, port, MDIO_MMD_VEND2, DW_VR_MII_AN_CTRL,
			       rc);
	if (rc < 0)
		return rc;

	rc = sja1105_pcs_read(priv, port, MDIO_MMD_VEND2, DW_VR_MII_DIG_CTRL1);
	if (rc < 0)
		return rc;

	if (priv->xpcs_cfg[port].inband_an)
		rc |= DW_VR_MII_DIG_CTRL1_MAC_AUTO_SW;
	else
		rc &= ~DW_VR_MII_DIG_CTRL1_MAC_AUTO_SW;

	rc = sja1105_pcs_write(priv, port, MDIO_MMD_VEND2, DW_VR_MII_DIG_CTRL1,
			       rc);
	if (rc < 0)
		return rc;

	rc = sja1105_pcs_read(priv, port, MDIO_MMD_VEND2, MDIO_CTRL1);
	if (rc < 0)
		return rc;

	if (priv->xpcs_cfg[port].inband_an)
		rc |= MDIO_AN_CTRL1_ENABLE;
	else
		rc &= ~MDIO_AN_CTRL1_ENABLE;

	return sja1105_pcs_write(priv, port, MDIO_MMD_VEND2, MDIO_CTRL1, rc);
}

static int sja1105_xpcs_link_up_sgmii(struct sja1105_private *priv, int port)
{
	int val = BMCR_FULLDPLX;

	if (priv->xpcs_cfg[port].inband_an)
		return 0;

	switch (priv->xpcs_cfg[port].speed) {
	case SPEED_1000:
		val = BMCR_SPEED1000;
		break;
	case SPEED_100:
		val = BMCR_SPEED100;
		break;
	case SPEED_10:
		val = BMCR_SPEED10;
		break;
	default:
		dev_err(priv->dev, "Invalid PCS speed %d\n",
			priv->xpcs_cfg[port].speed);
		return -EINVAL;
	}

	return sja1105_pcs_write(priv, port, MDIO_MMD_VEND2, MDIO_CTRL1, val);
}

static int sja1105_sgmii_setup(struct sja1105_private *priv, int port)
{
	int rc;

	rc = sja1105_xpcs_config_aneg_c37_sgmii(priv, port);
	if (rc)
		return rc;

	rc = sja1105_xpcs_link_up_sgmii(priv, port);
	if (rc)
		return rc;

	return priv->info->pma_config(priv, port);
}

static int sja1105_clocking_setup_port(struct sja1105_private *priv, int port)
{
	struct sja1105_xmii_params_entry *mii;
	sja1105_phy_interface_t phy_mode;
	sja1105_mii_role_t role;
	int rc;

	mii = priv->static_config.tables[BLK_IDX_XMII_PARAMS].entries;

	/* RGMII etc */
	phy_mode = mii->xmii_mode[port];
	/* MAC or PHY, for applicable types (not RGMII) */
	role = mii->phy_mac[port];

	switch (phy_mode) {
	case XMII_MODE_MII:
		rc = sja1105_mii_clocking_setup(priv, port, role);
		break;
	case XMII_MODE_RMII:
		rc = sja1105_rmii_clocking_setup(priv, port, role);
		break;
	case XMII_MODE_RGMII:
		rc = sja1105_rgmii_clocking_setup(priv, port, role);
		break;
	case XMII_MODE_SGMII:
		rc = sja1105_sgmii_setup(priv, port);
		break;
	default:
		return -EINVAL;
	}
	if (rc)
		return rc;

	/* Internally pull down the RX_DV/CRS_DV/RX_CTL and RX_ER inputs */
	return sja1105_cfg_pad_rx_config(priv, port);
}

static int sja1105_clocking_setup(struct sja1105_private *priv)
{
	struct dsa_pdata *pdata = dev_get_uclass_plat(priv->dev);
	int port, rc;

	for (port = 0; port < pdata->num_ports; port++) {
		rc = sja1105_clocking_setup_port(priv, port);
		if (rc < 0)
			return rc;
	}
	return 0;
}

static int sja1105_pcs_mdio_read(struct mii_dev *bus, int phy, int mmd, int reg)
{
	u8 packed_buf[SJA1105_SIZE_MDIO_CMD] = {0};
	struct sja1105_private *priv = bus->priv;
	const int size = SJA1105_SIZE_MDIO_CMD;
	u64 addr, tmp;
	int rc;

	if (mmd == MDIO_DEVAD_NONE)
		return -ENODEV;

	if (!priv->info->supports_sgmii[phy])
		return -ENODEV;

	addr = (mmd << 16) | (reg & GENMASK(15, 0));

	if (mmd != MDIO_MMD_VEND1 && mmd != MDIO_MMD_VEND2)
		return 0xffff;

	rc = sja1105_xfer_buf(priv, SPI_READ, addr, packed_buf, size);
	if (rc < 0)
		return rc;

	sja1105_packing(packed_buf, &tmp, 31, 0, size, UNPACK);

	return tmp & 0xffff;
}

static int sja1105_pcs_mdio_write(struct mii_dev *bus, int phy, int mmd,
				  int reg, u16 val)
{
	u8 packed_buf[SJA1105_SIZE_MDIO_CMD] = {0};
	struct sja1105_private *priv = bus->priv;
	const int size = SJA1105_SIZE_MDIO_CMD;
	u64 addr, tmp;

	if (mmd == MDIO_DEVAD_NONE)
		return -ENODEV;

	if (!priv->info->supports_sgmii[phy])
		return -ENODEV;

	addr = (mmd << 16) | (reg & GENMASK(15, 0));
	tmp = val;

	if (mmd != MDIO_MMD_VEND1 && mmd != MDIO_MMD_VEND2)
		return -ENODEV;

	sja1105_packing(packed_buf, &tmp, 31, 0, size, PACK);

	return sja1105_xfer_buf(priv, SPI_WRITE, addr, packed_buf, size);
}

static int sja1110_pcs_mdio_read(struct mii_dev *bus, int phy, int mmd, int reg)
{
	struct sja1105_private *priv = bus->priv;
	const struct sja1105_regs *regs = priv->info->regs;
	u8 packed_buf[SJA1105_SIZE_MDIO_CMD] = {0};
	const int size = SJA1105_SIZE_MDIO_CMD;
	int offset, bank;
	u64 addr, tmp;
	int rc;

	if (mmd == MDIO_DEVAD_NONE)
		return -ENODEV;

	if (regs->pcs_base[phy] == SJA1105_RSV_ADDR)
		return -ENODEV;

	addr = (mmd << 16) | (reg & GENMASK(15, 0));

	bank = addr >> 8;
	offset = addr & GENMASK(7, 0);

	/* This addressing scheme reserves register 0xff for the bank address
	 * register, so that can never be addressed.
	 */
	if (offset == 0xff)
		return -ENODEV;

	tmp = bank;

	sja1105_packing(packed_buf, &tmp, 31, 0, size, PACK);

	rc = sja1105_xfer_buf(priv, SPI_WRITE,
			      regs->pcs_base[phy] + SJA1110_PCS_BANK_REG,
			      packed_buf, size);
	if (rc < 0)
		return rc;

	rc = sja1105_xfer_buf(priv, SPI_READ, regs->pcs_base[phy] + offset,
			      packed_buf, size);
	if (rc < 0)
		return rc;

	sja1105_packing(packed_buf, &tmp, 31, 0, size, UNPACK);

	return tmp & 0xffff;
}

static int sja1110_pcs_mdio_write(struct mii_dev *bus, int phy, int mmd,
				  int reg, u16 val)
{
	struct sja1105_private *priv = bus->priv;
	const struct sja1105_regs *regs = priv->info->regs;
	u8 packed_buf[SJA1105_SIZE_MDIO_CMD] = {0};
	const int size = SJA1105_SIZE_MDIO_CMD;
	int offset, bank;
	u64 addr, tmp;
	int rc;

	if (mmd == MDIO_DEVAD_NONE)
		return -ENODEV;

	if (regs->pcs_base[phy] == SJA1105_RSV_ADDR)
		return -ENODEV;

	addr = (mmd << 16) | (reg & GENMASK(15, 0));

	bank = addr >> 8;
	offset = addr & GENMASK(7, 0);

	/* This addressing scheme reserves register 0xff for the bank address
	 * register, so that can never be addressed.
	 */
	if (offset == 0xff)
		return -ENODEV;

	tmp = bank;
	sja1105_packing(packed_buf, &tmp, 31, 0, size, PACK);

	rc = sja1105_xfer_buf(priv, SPI_WRITE,
			      regs->pcs_base[phy] + SJA1110_PCS_BANK_REG,
			      packed_buf, size);
	if (rc < 0)
		return rc;

	tmp = val;
	sja1105_packing(packed_buf, &tmp, 31, 0, size, PACK);

	return sja1105_xfer_buf(priv, SPI_WRITE, regs->pcs_base[phy] + offset,
				packed_buf, size);
}

static int sja1105_mdiobus_register(struct sja1105_private *priv)
{
	struct udevice *dev = priv->dev;
	struct mii_dev *bus;
	int rc;

	if (!priv->info->pcs_mdio_read || !priv->info->pcs_mdio_write)
		return 0;

	bus = mdio_alloc();
	if (!bus)
		return -ENOMEM;

	snprintf(bus->name, MDIO_NAME_LEN, "%s-pcs", dev->name);
	bus->read = priv->info->pcs_mdio_read;
	bus->write = priv->info->pcs_mdio_write;
	bus->priv = priv;

	rc = mdio_register(bus);
	if (rc) {
		mdio_free(bus);
		return rc;
	}

	priv->mdio_pcs = bus;

	return 0;
}

static void sja1105_mdiobus_unregister(struct sja1105_private *priv)
{
	if (!priv->mdio_pcs)
		return;

	mdio_unregister(priv->mdio_pcs);
	mdio_free(priv->mdio_pcs);
}

static const struct sja1105_regs sja1105et_regs = {
	.device_id = 0x0,
	.prod_id = 0x100BC3,
	.status = 0x1,
	.port_control = 0x11,
	.config = 0x020000,
	.rgu = 0x100440,
	/* UM10944.pdf, Table 86, ACU Register overview */
	.pad_mii_tx = {0x100800, 0x100802, 0x100804, 0x100806, 0x100808},
	.pad_mii_rx = {0x100801, 0x100803, 0x100805, 0x100807, 0x100809},
	.rmii_pll1 = 0x10000A,
	.cgu_idiv = {0x10000B, 0x10000C, 0x10000D, 0x10000E, 0x10000F},
	/* UM10944.pdf, Table 78, CGU Register overview */
	.mii_tx_clk = {0x100013, 0x10001A, 0x100021, 0x100028, 0x10002F},
	.mii_rx_clk = {0x100014, 0x10001B, 0x100022, 0x100029, 0x100030},
	.mii_ext_tx_clk = {0x100018, 0x10001F, 0x100026, 0x10002D, 0x100034},
	.mii_ext_rx_clk = {0x100019, 0x100020, 0x100027, 0x10002E, 0x100035},
	.rgmii_tx_clk = {0x100016, 0x10001D, 0x100024, 0x10002B, 0x100032},
	.rmii_ref_clk = {0x100015, 0x10001C, 0x100023, 0x10002A, 0x100031},
	.rmii_ext_tx_clk = {0x100018, 0x10001F, 0x100026, 0x10002D, 0x100034},
};

static const struct sja1105_regs sja1105pqrs_regs = {
	.device_id = 0x0,
	.prod_id = 0x100BC3,
	.status = 0x1,
	.port_control = 0x12,
	.config = 0x020000,
	.rgu = 0x100440,
	/* UM10944.pdf, Table 86, ACU Register overview */
	.pad_mii_tx = {0x100800, 0x100802, 0x100804, 0x100806, 0x100808},
	.pad_mii_rx = {0x100801, 0x100803, 0x100805, 0x100807, 0x100809},
	.pad_mii_id = {0x100810, 0x100811, 0x100812, 0x100813, 0x100814},
	.rmii_pll1 = 0x10000A,
	.cgu_idiv = {0x10000B, 0x10000C, 0x10000D, 0x10000E, 0x10000F},
	/* UM11040.pdf, Table 114 */
	.mii_tx_clk = {0x100013, 0x100019, 0x10001F, 0x100025, 0x10002B},
	.mii_rx_clk = {0x100014, 0x10001A, 0x100020, 0x100026, 0x10002C},
	.mii_ext_tx_clk = {0x100017, 0x10001D, 0x100023, 0x100029, 0x10002F},
	.mii_ext_rx_clk = {0x100018, 0x10001E, 0x100024, 0x10002A, 0x100030},
	.rgmii_tx_clk = {0x100016, 0x10001C, 0x100022, 0x100028, 0x10002E},
	.rmii_ref_clk = {0x100015, 0x10001B, 0x100021, 0x100027, 0x10002D},
	.rmii_ext_tx_clk = {0x100017, 0x10001D, 0x100023, 0x100029, 0x10002F},
};

static const struct sja1105_regs sja1110_regs = {
	.device_id = SJA1110_SPI_ADDR(0x0),
	.prod_id = SJA1110_ACU_ADDR(0xf00),
	.status = SJA1110_SPI_ADDR(0x4),
	.port_control = SJA1110_SPI_ADDR(0x50), /* actually INHIB_TX */
	.config = 0x020000,
	.rgu = SJA1110_RGU_ADDR(0x100), /* Reset Control Register 0 */
	/* Ports 2 and 3 are capable of xMII, but there isn't anything to
	 * configure in the CGU/ACU for them.
	 */
	.pad_mii_tx = {SJA1105_RSV_ADDR, SJA1105_RSV_ADDR,
		       SJA1105_RSV_ADDR, SJA1105_RSV_ADDR,
		       SJA1105_RSV_ADDR, SJA1105_RSV_ADDR,
		       SJA1105_RSV_ADDR, SJA1105_RSV_ADDR,
		       SJA1105_RSV_ADDR, SJA1105_RSV_ADDR,
		       SJA1105_RSV_ADDR},
	.pad_mii_rx = {SJA1105_RSV_ADDR, SJA1105_RSV_ADDR,
		       SJA1105_RSV_ADDR, SJA1105_RSV_ADDR,
		       SJA1105_RSV_ADDR, SJA1105_RSV_ADDR,
		       SJA1105_RSV_ADDR, SJA1105_RSV_ADDR,
		       SJA1105_RSV_ADDR, SJA1105_RSV_ADDR,
		       SJA1105_RSV_ADDR},
	.pad_mii_id = {SJA1105_RSV_ADDR, SJA1105_RSV_ADDR,
		       SJA1110_ACU_ADDR(0x18), SJA1110_ACU_ADDR(0x28),
		       SJA1105_RSV_ADDR, SJA1105_RSV_ADDR,
		       SJA1105_RSV_ADDR, SJA1105_RSV_ADDR,
		       SJA1105_RSV_ADDR, SJA1105_RSV_ADDR,
		       SJA1105_RSV_ADDR},
	.rmii_pll1 = SJA1105_RSV_ADDR,
	.cgu_idiv = {SJA1105_RSV_ADDR, SJA1105_RSV_ADDR, SJA1105_RSV_ADDR,
		     SJA1105_RSV_ADDR, SJA1105_RSV_ADDR, SJA1105_RSV_ADDR,
		     SJA1105_RSV_ADDR, SJA1105_RSV_ADDR, SJA1105_RSV_ADDR,
		     SJA1105_RSV_ADDR, SJA1105_RSV_ADDR},
	.mii_tx_clk = {SJA1105_RSV_ADDR, SJA1105_RSV_ADDR, SJA1105_RSV_ADDR,
		       SJA1105_RSV_ADDR, SJA1105_RSV_ADDR, SJA1105_RSV_ADDR,
		       SJA1105_RSV_ADDR, SJA1105_RSV_ADDR, SJA1105_RSV_ADDR,
		       SJA1105_RSV_ADDR, SJA1105_RSV_ADDR},
	.mii_rx_clk = {SJA1105_RSV_ADDR, SJA1105_RSV_ADDR, SJA1105_RSV_ADDR,
		       SJA1105_RSV_ADDR, SJA1105_RSV_ADDR, SJA1105_RSV_ADDR,
		       SJA1105_RSV_ADDR, SJA1105_RSV_ADDR, SJA1105_RSV_ADDR,
		       SJA1105_RSV_ADDR, SJA1105_RSV_ADDR},
	.mii_ext_tx_clk = {SJA1105_RSV_ADDR, SJA1105_RSV_ADDR, SJA1105_RSV_ADDR,
			   SJA1105_RSV_ADDR, SJA1105_RSV_ADDR, SJA1105_RSV_ADDR,
			   SJA1105_RSV_ADDR, SJA1105_RSV_ADDR, SJA1105_RSV_ADDR,
			   SJA1105_RSV_ADDR, SJA1105_RSV_ADDR},
	.mii_ext_rx_clk = {SJA1105_RSV_ADDR, SJA1105_RSV_ADDR, SJA1105_RSV_ADDR,
			   SJA1105_RSV_ADDR, SJA1105_RSV_ADDR, SJA1105_RSV_ADDR,
			   SJA1105_RSV_ADDR, SJA1105_RSV_ADDR, SJA1105_RSV_ADDR,
			   SJA1105_RSV_ADDR, SJA1105_RSV_ADDR},
	.rgmii_tx_clk = {SJA1105_RSV_ADDR, SJA1105_RSV_ADDR, SJA1105_RSV_ADDR,
			 SJA1105_RSV_ADDR, SJA1105_RSV_ADDR, SJA1105_RSV_ADDR,
			 SJA1105_RSV_ADDR, SJA1105_RSV_ADDR, SJA1105_RSV_ADDR,
			 SJA1105_RSV_ADDR, SJA1105_RSV_ADDR},
	.rmii_ref_clk = {SJA1105_RSV_ADDR, SJA1105_RSV_ADDR, SJA1105_RSV_ADDR,
			 SJA1105_RSV_ADDR, SJA1105_RSV_ADDR, SJA1105_RSV_ADDR,
			 SJA1105_RSV_ADDR, SJA1105_RSV_ADDR, SJA1105_RSV_ADDR,
			 SJA1105_RSV_ADDR, SJA1105_RSV_ADDR},
	.rmii_ext_tx_clk = {SJA1105_RSV_ADDR, SJA1105_RSV_ADDR,
			    SJA1105_RSV_ADDR, SJA1105_RSV_ADDR,
			    SJA1105_RSV_ADDR, SJA1105_RSV_ADDR,
			    SJA1105_RSV_ADDR, SJA1105_RSV_ADDR,
			    SJA1105_RSV_ADDR, SJA1105_RSV_ADDR,
			    SJA1105_RSV_ADDR},
	.pcs_base = {SJA1105_RSV_ADDR, 0x1c1400, 0x1c1800, 0x1c1c00, 0x1c2000,
		     SJA1105_RSV_ADDR, SJA1105_RSV_ADDR, SJA1105_RSV_ADDR,
		     SJA1105_RSV_ADDR, SJA1105_RSV_ADDR, SJA1105_RSV_ADDR},
};

enum sja1105_switch_id {
	SJA1105E = 0,
	SJA1105T,
	SJA1105P,
	SJA1105Q,
	SJA1105R,
	SJA1105S,
	SJA1110A,
	SJA1110B,
	SJA1110C,
	SJA1110D,
	SJA1105_MAX_SWITCH_ID,
};

static const struct sja1105_info sja1105_info[] = {
	[SJA1105E] = {
		.device_id		= SJA1105E_DEVICE_ID,
		.part_no		= SJA1105ET_PART_NO,
		.static_ops		= sja1105et_table_ops,
		.reset_cmd		= sja1105et_reset_cmd,
		.regs			= &sja1105et_regs,
		.port_speed		= {
			[SJA1105_SPEED_AUTO] = 0,
			[SJA1105_SPEED_10MBPS] = 3,
			[SJA1105_SPEED_100MBPS] = 2,
			[SJA1105_SPEED_1000MBPS] = 1,
		},
		.supports_mii		= {true, true, true, true, true},
		.supports_rmii		= {true, true, true, true, true},
		.supports_rgmii		= {true, true, true, true, true},
		.name			= "SJA1105E",
	},
	[SJA1105T] = {
		.device_id		= SJA1105T_DEVICE_ID,
		.part_no		= SJA1105ET_PART_NO,
		.static_ops		= sja1105et_table_ops,
		.reset_cmd		= sja1105et_reset_cmd,
		.regs			= &sja1105et_regs,
		.port_speed		= {
			[SJA1105_SPEED_AUTO] = 0,
			[SJA1105_SPEED_10MBPS] = 3,
			[SJA1105_SPEED_100MBPS] = 2,
			[SJA1105_SPEED_1000MBPS] = 1,
		},
		.supports_mii		= {true, true, true, true, true},
		.supports_rmii		= {true, true, true, true, true},
		.supports_rgmii		= {true, true, true, true, true},
		.name			= "SJA1105T",
	},
	[SJA1105P] = {
		.device_id		= SJA1105PR_DEVICE_ID,
		.part_no		= SJA1105P_PART_NO,
		.static_ops		= sja1105pqrs_table_ops,
		.setup_rgmii_delay	= sja1105pqrs_setup_rgmii_delay,
		.reset_cmd		= sja1105pqrs_reset_cmd,
		.regs			= &sja1105pqrs_regs,
		.port_speed		= {
			[SJA1105_SPEED_AUTO] = 0,
			[SJA1105_SPEED_10MBPS] = 3,
			[SJA1105_SPEED_100MBPS] = 2,
			[SJA1105_SPEED_1000MBPS] = 1,
		},
		.supports_mii		= {true, true, true, true, true},
		.supports_rmii		= {true, true, true, true, true},
		.supports_rgmii		= {true, true, true, true, true},
		.name			= "SJA1105P",
	},
	[SJA1105Q] = {
		.device_id		= SJA1105QS_DEVICE_ID,
		.part_no		= SJA1105Q_PART_NO,
		.static_ops		= sja1105pqrs_table_ops,
		.setup_rgmii_delay	= sja1105pqrs_setup_rgmii_delay,
		.reset_cmd		= sja1105pqrs_reset_cmd,
		.regs			= &sja1105pqrs_regs,
		.port_speed		= {
			[SJA1105_SPEED_AUTO] = 0,
			[SJA1105_SPEED_10MBPS] = 3,
			[SJA1105_SPEED_100MBPS] = 2,
			[SJA1105_SPEED_1000MBPS] = 1,
		},
		.supports_mii		= {true, true, true, true, true},
		.supports_rmii		= {true, true, true, true, true},
		.supports_rgmii		= {true, true, true, true, true},
		.name			= "SJA1105Q",
	},
	[SJA1105R] = {
		.device_id		= SJA1105PR_DEVICE_ID,
		.part_no		= SJA1105R_PART_NO,
		.static_ops		= sja1105pqrs_table_ops,
		.setup_rgmii_delay	= sja1105pqrs_setup_rgmii_delay,
		.reset_cmd		= sja1105pqrs_reset_cmd,
		.regs			= &sja1105pqrs_regs,
		.pcs_mdio_read		= sja1105_pcs_mdio_read,
		.pcs_mdio_write		= sja1105_pcs_mdio_write,
		.pma_config		= sja1105_pma_config,
		.port_speed		= {
			[SJA1105_SPEED_AUTO] = 0,
			[SJA1105_SPEED_10MBPS] = 3,
			[SJA1105_SPEED_100MBPS] = 2,
			[SJA1105_SPEED_1000MBPS] = 1,
		},
		.supports_mii		= {true, true, true, true, true},
		.supports_rmii		= {true, true, true, true, true},
		.supports_rgmii		= {true, true, true, true, true},
		.supports_sgmii		= {false, false, false, false, true},
		.name			= "SJA1105R",
	},
	[SJA1105S] = {
		.device_id		= SJA1105QS_DEVICE_ID,
		.part_no		= SJA1105S_PART_NO,
		.static_ops		= sja1105pqrs_table_ops,
		.setup_rgmii_delay	= sja1105pqrs_setup_rgmii_delay,
		.reset_cmd		= sja1105pqrs_reset_cmd,
		.regs			= &sja1105pqrs_regs,
		.pcs_mdio_read		= sja1105_pcs_mdio_read,
		.pcs_mdio_write		= sja1105_pcs_mdio_write,
		.pma_config		= sja1105_pma_config,
		.port_speed		= {
			[SJA1105_SPEED_AUTO] = 0,
			[SJA1105_SPEED_10MBPS] = 3,
			[SJA1105_SPEED_100MBPS] = 2,
			[SJA1105_SPEED_1000MBPS] = 1,
		},
		.supports_mii		= {true, true, true, true, true},
		.supports_rmii		= {true, true, true, true, true},
		.supports_rgmii		= {true, true, true, true, true},
		.supports_sgmii		= {false, false, false, false, true},
		.name			= "SJA1105S",
	},
	[SJA1110A] = {
		.device_id		= SJA1110_DEVICE_ID,
		.part_no		= SJA1110A_PART_NO,
		.static_ops		= sja1110_table_ops,
		.setup_rgmii_delay	= sja1110_setup_rgmii_delay,
		.reset_cmd		= sja1110_reset_cmd,
		.regs			= &sja1110_regs,
		.pcs_mdio_read		= sja1110_pcs_mdio_read,
		.pcs_mdio_write		= sja1110_pcs_mdio_write,
		.pma_config		= sja1110_pma_config,
		.port_speed		= {
			[SJA1105_SPEED_AUTO] = 0,
			[SJA1105_SPEED_10MBPS] = 4,
			[SJA1105_SPEED_100MBPS] = 3,
			[SJA1105_SPEED_1000MBPS] = 2,
		},
		.supports_mii		= {true, true, true, true, false,
					   true, true, true, true, true, true},
		.supports_rmii		= {false, false, true, true, false,
					   false, false, false, false, false, false},
		.supports_rgmii		= {false, false, true, true, false,
					   false, false, false, false, false, false},
		.supports_sgmii		= {false, true, true, true, true,
					   false, false, false, false, false, false},
		.name			= "SJA1110A",
	},
	[SJA1110B] = {
		.device_id		= SJA1110_DEVICE_ID,
		.part_no		= SJA1110B_PART_NO,
		.static_ops		= sja1110_table_ops,
		.setup_rgmii_delay	= sja1110_setup_rgmii_delay,
		.reset_cmd		= sja1110_reset_cmd,
		.regs			= &sja1110_regs,
		.pcs_mdio_read		= sja1110_pcs_mdio_read,
		.pcs_mdio_write		= sja1110_pcs_mdio_write,
		.pma_config		= sja1110_pma_config,
		.port_speed		= {
			[SJA1105_SPEED_AUTO] = 0,
			[SJA1105_SPEED_10MBPS] = 4,
			[SJA1105_SPEED_100MBPS] = 3,
			[SJA1105_SPEED_1000MBPS] = 2,
		},
		.supports_mii		= {true, true, true, true, false,
					   true, true, true, true, true, false},
		.supports_rmii		= {false, false, true, true, false,
					   false, false, false, false, false, false},
		.supports_rgmii		= {false, false, true, true, false,
					   false, false, false, false, false, false},
		.supports_sgmii		= {false, false, false, true, true,
					   false, false, false, false, false, false},
		.name			= "SJA1110B",
	},
	[SJA1110C] = {
		.device_id		= SJA1110_DEVICE_ID,
		.part_no		= SJA1110C_PART_NO,
		.static_ops		= sja1110_table_ops,
		.setup_rgmii_delay	= sja1110_setup_rgmii_delay,
		.reset_cmd		= sja1110_reset_cmd,
		.regs			= &sja1110_regs,
		.pcs_mdio_read		= sja1110_pcs_mdio_read,
		.pcs_mdio_write		= sja1110_pcs_mdio_write,
		.pma_config		= sja1110_pma_config,
		.port_speed		= {
			[SJA1105_SPEED_AUTO] = 0,
			[SJA1105_SPEED_10MBPS] = 4,
			[SJA1105_SPEED_100MBPS] = 3,
			[SJA1105_SPEED_1000MBPS] = 2,
		},
		.supports_mii		= {true, true, true, true, false,
					   true, true, true, false, false, false},
		.supports_rmii		= {false, false, true, true, false,
					   false, false, false, false, false, false},
		.supports_rgmii		= {false, false, true, true, false,
					   false, false, false, false, false, false},
		.supports_sgmii		= {false, false, false, false, true,
					   false, false, false, false, false, false},
		.name			= "SJA1110C",
	},
	[SJA1110D] = {
		.device_id		= SJA1110_DEVICE_ID,
		.part_no		= SJA1110D_PART_NO,
		.static_ops		= sja1110_table_ops,
		.setup_rgmii_delay	= sja1110_setup_rgmii_delay,
		.reset_cmd		= sja1110_reset_cmd,
		.regs			= &sja1110_regs,
		.pcs_mdio_read		= sja1110_pcs_mdio_read,
		.pcs_mdio_write		= sja1110_pcs_mdio_write,
		.pma_config		= sja1110_pma_config,
		.port_speed		= {
			[SJA1105_SPEED_AUTO] = 0,
			[SJA1105_SPEED_10MBPS] = 4,
			[SJA1105_SPEED_100MBPS] = 3,
			[SJA1105_SPEED_1000MBPS] = 2,
		},
		.supports_mii		= {true, false, true, false, false,
					   true, true, true, false, false, false},
		.supports_rmii		= {false, false, true, false, false,
					   false, false, false, false, false, false},
		.supports_rgmii		= {false, false, true, false, false,
					   false, false, false, false, false, false},
		.supports_sgmii		= {false, true, true, true, true,
					   false, false, false, false, false, false},
		.name			= "SJA1110D",
	},
};

struct sja1105_status {
	u64 configs;
	u64 crcchkl;
	u64 ids;
	u64 crcchkg;
};

static void sja1105_status_unpack(void *buf, struct sja1105_status *status)
{
	sja1105_packing(buf, &status->configs, 31, 31, 4, UNPACK);
	sja1105_packing(buf, &status->crcchkl, 30, 30, 4, UNPACK);
	sja1105_packing(buf, &status->ids,     29, 29, 4, UNPACK);
	sja1105_packing(buf, &status->crcchkg, 28, 28, 4, UNPACK);
}

static int sja1105_status_get(struct sja1105_private *priv,
			      struct sja1105_status *status)
{
	const struct sja1105_regs *regs = priv->info->regs;
	u8 packed_buf[4];
	int rc;

	rc = sja1105_xfer_buf(priv, SPI_READ, regs->status, packed_buf, 4);
	if (rc < 0)
		return rc;

	sja1105_status_unpack(packed_buf, status);

	return 0;
}

/* Not const because unpacking priv->static_config into buffers and preparing
 * for upload requires the recalculation of table CRCs and updating the
 * structures with these.
 */
static int
static_config_buf_prepare_for_upload(struct sja1105_private *priv,
				     void *config_buf, int buf_len)
{
	struct sja1105_static_config *config = &priv->static_config;
	struct sja1105_table_header final_header;
	char *final_header_ptr;
	int crc_len;

	/* Write Device ID and config tables to config_buf */
	sja1105_static_config_pack(config_buf, config);
	/* Recalculate CRC of the last header (right now 0xDEADBEEF).
	 * Don't include the CRC field itself.
	 */
	crc_len = buf_len - 4;
	/* Read the whole table header */
	final_header_ptr = config_buf + buf_len - SJA1105_SIZE_TABLE_HEADER;
	sja1105_table_header_packing(final_header_ptr, &final_header, UNPACK);
	/* Modify */
	final_header.crc = sja1105_crc32(config_buf, crc_len);
	/* Rewrite */
	sja1105_table_header_packing(final_header_ptr, &final_header, PACK);

	return 0;
}

static int sja1105_static_config_upload(struct sja1105_private *priv)
{
	struct sja1105_static_config *config = &priv->static_config;
	const struct sja1105_regs *regs = priv->info->regs;
	struct sja1105_status status;
	u8 *config_buf;
	int buf_len;
	int rc;

	buf_len = sja1105_static_config_get_length(config);
	config_buf = calloc(buf_len, sizeof(char));
	if (!config_buf)
		return -ENOMEM;

	rc = static_config_buf_prepare_for_upload(priv, config_buf, buf_len);
	if (rc < 0) {
		printf("Invalid config, cannot upload\n");
		rc = -EINVAL;
		goto out;
	}
	/* Put the SJA1105 in programming mode */
	rc = priv->info->reset_cmd(priv);
	if (rc < 0) {
		printf("Failed to reset switch\n");
		goto out;
	}
	/* Wait for the switch to come out of reset */
	udelay(1000);
	/* Upload the static config to the device */
	rc = sja1105_xfer_buf(priv, SPI_WRITE, regs->config,
			      config_buf, buf_len);
	if (rc < 0) {
		printf("Failed to upload config\n");
		goto out;
	}
	/* Check that SJA1105 responded well to the config upload */
	rc = sja1105_status_get(priv, &status);
	if (rc < 0)
		goto out;

	if (status.ids == 1) {
		printf("Mismatch between hardware and static config device id. "
		       "Wrote 0x%llx, wants 0x%llx\n",
		       config->device_id, priv->info->device_id);
		rc = -EIO;
		goto out;
	}
	if (status.crcchkl == 1 || status.crcchkg == 1) {
		printf("Switch reported invalid CRC on static config\n");
		rc = -EIO;
		goto out;
	}
	if (status.configs == 0) {
		printf("Switch reported that config is invalid\n");
		rc = -EIO;
		goto out;
	}

out:
	free(config_buf);
	return rc;
}

static int sja1105_static_config_reload(struct sja1105_private *priv)
{
	int rc;

	rc = sja1105_static_config_upload(priv);
	if (rc < 0) {
		printf("Failed to load static config: %d\n", rc);
		return rc;
	}

	/* Configure the CGU (PHY link modes and speeds) */
	rc = sja1105_clocking_setup(priv);
	if (rc < 0) {
		printf("Failed to configure MII clocking: %d\n", rc);
		return rc;
	}

	return 0;
}

static int sja1105_port_probe(struct udevice *dev, int port,
			      struct phy_device *phy)
{
	struct sja1105_private *priv = dev_get_priv(dev);
	ofnode node = dsa_port_get_ofnode(dev, port);
	phy_interface_t phy_mode = phy->interface;

	priv->xpcs_cfg[port].inband_an = ofnode_eth_uses_inband_aneg(node);

	if (phy_mode == PHY_INTERFACE_MODE_MII ||
	    phy_mode == PHY_INTERFACE_MODE_RMII) {
		phy->supported &= PHY_BASIC_FEATURES;
		phy->advertising &= PHY_BASIC_FEATURES;
	} else {
		phy->supported &= PHY_GBIT_FEATURES;
		phy->advertising &= PHY_GBIT_FEATURES;
	}

	return phy_config(phy);
}

static int sja1105_port_enable(struct udevice *dev, int port,
			       struct phy_device *phy)
{
	struct sja1105_private *priv = dev_get_priv(dev);
	phy_interface_t phy_mode = phy->interface;
	struct sja1105_xmii_params_entry *mii;
	struct sja1105_mac_config_entry *mac;
	int rc;

	rc = phy_startup(phy);
	if (rc)
		return rc;

	mii = priv->static_config.tables[BLK_IDX_XMII_PARAMS].entries;
	mac = priv->static_config.tables[BLK_IDX_MAC_CONFIG].entries;

	switch (phy_mode) {
	case PHY_INTERFACE_MODE_MII:
		if (!priv->info->supports_mii[port])
			goto unsupported;

		mii->xmii_mode[port] = XMII_MODE_MII;
		break;
	case PHY_INTERFACE_MODE_RMII:
		if (!priv->info->supports_rmii[port])
			goto unsupported;

		mii->xmii_mode[port] = XMII_MODE_RMII;
		break;
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
	case PHY_INTERFACE_MODE_RGMII_RXID:
	case PHY_INTERFACE_MODE_RGMII_TXID:
		if (!priv->info->supports_rgmii[port])
			goto unsupported;

		mii->xmii_mode[port] = XMII_MODE_RGMII;
		break;
	case PHY_INTERFACE_MODE_SGMII:
		if (!priv->info->supports_sgmii[port])
			goto unsupported;

		mii->xmii_mode[port] = XMII_MODE_SGMII;
		mii->special[port] = true;
		break;
unsupported:
	default:
		dev_err(dev, "Unsupported PHY mode %d on port %d!\n",
			phy_mode, port);
		return -EINVAL;
	}

	/* RevMII, RevRMII not supported */
	mii->phy_mac[port] = XMII_MAC;

	/* Let the PHY handle the RGMII delays, if present. */
	if (phy->phy_id == PHY_FIXED_ID) {
		if (phy_mode == PHY_INTERFACE_MODE_RGMII_RXID ||
		    phy_mode == PHY_INTERFACE_MODE_RGMII_ID)
			priv->rgmii_rx_delay[port] = true;

		if (phy_mode == PHY_INTERFACE_MODE_RGMII_TXID ||
		    phy_mode == PHY_INTERFACE_MODE_RGMII_ID)
			priv->rgmii_tx_delay[port] = true;

		if ((priv->rgmii_rx_delay[port] ||
		     priv->rgmii_tx_delay[port]) &&
		     !priv->info->setup_rgmii_delay) {
			printf("Chip does not support internal RGMII delays\n");
			return -EINVAL;
		}
	}

	if (mii->xmii_mode[port] == XMII_MODE_SGMII) {
		mac[port].speed = priv->info->port_speed[SJA1105_SPEED_1000MBPS];
		priv->xpcs_cfg[port].speed = phy->speed;
	} else if (phy->speed == SPEED_1000) {
		mac[port].speed = priv->info->port_speed[SJA1105_SPEED_1000MBPS];
	} else if (phy->speed == SPEED_100) {
		mac[port].speed = priv->info->port_speed[SJA1105_SPEED_100MBPS];
	} else if (phy->speed == SPEED_10) {
		mac[port].speed = priv->info->port_speed[SJA1105_SPEED_10MBPS];
	} else {
		printf("Invalid PHY speed %d on port %d\n", phy->speed, port);
		return -EINVAL;
	}

	return sja1105_static_config_reload(priv);
}

static void sja1105_port_disable(struct udevice *dev, int port,
				 struct phy_device *phy)
{
	phy_shutdown(phy);
}

static int sja1105_xmit(struct udevice *dev, int port, void *packet, int length)
{
	struct sja1105_private *priv = dev_get_priv(dev);
	u8 *from = (u8 *)packet + VLAN_HLEN;
	struct vlan_ethhdr *hdr = packet;
	u8 *dest = (u8 *)packet;

	memmove(dest, from, 2 * ETH_ALEN);
	hdr->h_vlan_proto = htons(ETH_P_SJA1105);
	hdr->h_vlan_TCI = htons(priv->pvid[port]);

	return 0;
}

static int sja1105_rcv(struct udevice *dev, int *port, void *packet, int length)
{
	struct vlan_ethhdr *hdr = packet;
	u8 *dest = packet + VLAN_HLEN;
	u8 *from = packet;

	if (ntohs(hdr->h_vlan_proto) != ETH_P_SJA1105)
		return -EINVAL;

	*port = ntohs(hdr->h_vlan_TCI) & DSA_8021Q_PORT_MASK;
	memmove(dest, from, 2 * ETH_ALEN);

	return 0;
}

static const struct dsa_ops sja1105_dsa_ops = {
	.port_probe	= sja1105_port_probe,
	.port_enable	= sja1105_port_enable,
	.port_disable	= sja1105_port_disable,
	.xmit		= sja1105_xmit,
	.rcv		= sja1105_rcv,
};

static int sja1105_init(struct sja1105_private *priv)
{
	int rc;

	rc = sja1105_static_config_init(priv);
	if (rc) {
		printf("Failed to initialize static config: %d\n", rc);
		return rc;
	}

	rc = sja1105_mdiobus_register(priv);
	if (rc) {
		printf("Failed to register MDIO bus: %d\n", rc);
		goto err_mdiobus_register;
	}

	return 0;

err_mdiobus_register:
	sja1105_static_config_free(&priv->static_config);

	return rc;
}

static int sja1105_check_device_id(struct sja1105_private *priv)
{
	const struct sja1105_regs *regs = priv->info->regs;
	u8 packed_buf[SJA1105_SIZE_DEVICE_ID] = {0};
	enum sja1105_switch_id id;
	u64 device_id;
	u64 part_no;
	int rc;

	rc = sja1105_xfer_buf(priv, SPI_READ, regs->device_id, packed_buf,
			      SJA1105_SIZE_DEVICE_ID);
	if (rc < 0)
		return rc;

	sja1105_packing(packed_buf, &device_id, 31, 0, SJA1105_SIZE_DEVICE_ID,
			UNPACK);

	rc = sja1105_xfer_buf(priv, SPI_READ, regs->prod_id, packed_buf,
			      SJA1105_SIZE_DEVICE_ID);
	if (rc < 0)
		return rc;

	sja1105_packing(packed_buf, &part_no, 19, 4, SJA1105_SIZE_DEVICE_ID,
			UNPACK);

	for (id = 0; id < SJA1105_MAX_SWITCH_ID; id++) {
		const struct sja1105_info *info = &sja1105_info[id];

		/* Is what's been probed in our match table at all? */
		if (info->device_id != device_id || info->part_no != part_no)
			continue;

		/* But is it what's in the device tree? */
		if (priv->info->device_id != device_id ||
		    priv->info->part_no != part_no) {
			printf("Device tree specifies chip %s but found %s, please fix it!\n",
			       priv->info->name, info->name);
			/* It isn't. No problem, pick that up. */
			priv->info = info;
		}

		return 0;
	}

	printf("Unexpected {device ID, part number}: 0x%llx 0x%llx\n",
	       device_id, part_no);

	return -ENODEV;
}

static int sja1105_probe(struct udevice *dev)
{
	enum sja1105_switch_id id = dev_get_driver_data(dev);
	struct sja1105_private *priv = dev_get_priv(dev);
	int rc;

	if (ofnode_valid(dev_ofnode(dev)) &&
	    !ofnode_is_enabled(dev_ofnode(dev))) {
		dev_dbg(dev, "switch disabled\n");
		return -ENODEV;
	}

	priv->info = &sja1105_info[id];
	priv->dev = dev;

	rc = sja1105_check_device_id(priv);
	if (rc < 0) {
		dev_err(dev, "Device ID check failed: %d\n", rc);
		return rc;
	}

	dsa_set_tagging(dev, VLAN_HLEN, 0);

	return sja1105_init(priv);
}

static int sja1105_remove(struct udevice *dev)
{
	struct sja1105_private *priv = dev_get_priv(dev);

	sja1105_mdiobus_unregister(priv);
	sja1105_static_config_free(&priv->static_config);

	return 0;
}

static const struct udevice_id sja1105_ids[] = {
	{ .compatible = "nxp,sja1105e", .data = SJA1105E },
	{ .compatible = "nxp,sja1105t", .data = SJA1105T },
	{ .compatible = "nxp,sja1105p", .data = SJA1105P },
	{ .compatible = "nxp,sja1105q", .data = SJA1105Q },
	{ .compatible = "nxp,sja1105r", .data = SJA1105R },
	{ .compatible = "nxp,sja1105s", .data = SJA1105S },
	{ .compatible = "nxp,sja1110a", .data = SJA1110A },
	{ .compatible = "nxp,sja1110b", .data = SJA1110B },
	{ .compatible = "nxp,sja1110c", .data = SJA1110C },
	{ .compatible = "nxp,sja1110d", .data = SJA1110D },
	{ }
};

U_BOOT_DRIVER(sja1105) = {
	.name		= "sja1105",
	.id		= UCLASS_DSA,
	.of_match	= sja1105_ids,
	.probe		= sja1105_probe,
	.remove		= sja1105_remove,
	.ops		= &sja1105_dsa_ops,
	.priv_auto	= sizeof(struct sja1105_private),
};
