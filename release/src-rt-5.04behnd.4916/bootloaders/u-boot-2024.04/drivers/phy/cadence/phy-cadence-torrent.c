// SPDX-License-Identifier: GPL-2.0
/*
 * Cadence Torrent SD0801 PHY driver.
 *
 * Based on the linux driver provided by Cadence
 *
 * Copyright (c) 2018 Cadence Design Systems
 *
 * Copyright (C) 2020 Texas Instruments Incorporated - https://www.ti.com/
 *
 */

#include <common.h>
#include <clk.h>
#include <generic-phy.h>
#include <reset.h>
#include <dm/device.h>
#include <dm/device_compat.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/read.h>
#include <dm/uclass.h>
#include <linux/io.h>
#include <dt-bindings/phy/phy.h>
#include <regmap.h>
#include <linux/delay.h>
#include <linux/string.h>

#define REF_CLK_19_2MHz		19200000
#define REF_CLK_25MHz		25000000

#define MAX_NUM_LANES		4
#define DEFAULT_MAX_BIT_RATE	8100 /* in Mbps*/

#define NUM_SSC_MODE		3
#define NUM_PHY_TYPE		6

#define POLL_TIMEOUT_US		5000
#define PLL_LOCK_TIMEOUT	100000

#define TORRENT_COMMON_CDB_OFFSET	0x0

#define TORRENT_TX_LANE_CDB_OFFSET(ln, block_offset, reg_offset)	\
				((0x4000 << (block_offset)) +		\
				(((ln) << 9) << (reg_offset)))
#define TORRENT_RX_LANE_CDB_OFFSET(ln, block_offset, reg_offset)	\
				((0x8000 << (block_offset)) +		\
				(((ln) << 9) << (reg_offset)))

#define TORRENT_PHY_PCS_COMMON_OFFSET(block_offset)	\
				(0xC000 << (block_offset))

#define TORRENT_PHY_PMA_COMMON_OFFSET(block_offset)	\
				(0xE000 << (block_offset))

/*
 * register offsets from SD0801 PHY register block base (i.e MHDP
 * register base + 0x500000)
 */
#define CMN_SSM_BANDGAP_TMR		0x0021U
#define CMN_SSM_BIAS_TMR		0x0022U
#define CMN_PLLSM0_PLLPRE_TMR		0x002AU
#define CMN_PLLSM0_PLLLOCK_TMR		0x002CU
#define CMN_PLLSM1_PLLPRE_TMR		0x0032U
#define CMN_PLLSM1_PLLLOCK_TMR		0x0034U
#define CMN_CDIAG_CDB_PWRI_OVRD		0x0041U
#define CMN_CDIAG_XCVRC_PWRI_OVRD	0x0047U
#define CMN_BGCAL_INIT_TMR		0x0064U
#define CMN_BGCAL_ITER_TMR		0x0065U
#define CMN_IBCAL_INIT_TMR		0x0074U
#define CMN_PLL0_VCOCAL_TCTRL		0x0082U
#define CMN_PLL0_VCOCAL_INIT_TMR	0x0084U
#define CMN_PLL0_VCOCAL_ITER_TMR	0x0085U
#define CMN_PLL0_VCOCAL_REFTIM_START	0x0086U
#define CMN_PLL0_VCOCAL_PLLCNT_START	0x0088U
#define CMN_PLL0_INTDIV_M0		0x0090U
#define CMN_PLL0_FRACDIVL_M0		0x0091U
#define CMN_PLL0_FRACDIVH_M0		0x0092U
#define CMN_PLL0_HIGH_THR_M0		0x0093U
#define CMN_PLL0_DSM_DIAG_M0		0x0094U
#define CMN_PLL0_SS_CTRL1_M0		0x0098U
#define CMN_PLL0_SS_CTRL2_M0		0x0099U
#define CMN_PLL0_SS_CTRL3_M0		0x009AU
#define CMN_PLL0_SS_CTRL4_M0		0x009BU
#define CMN_PLL0_LOCK_REFCNT_START	0x009CU
#define CMN_PLL0_LOCK_PLLCNT_START	0x009EU
#define CMN_PLL0_LOCK_PLLCNT_THR	0x009FU
#define CMN_PLL0_INTDIV_M1		0x00A0U
#define CMN_PLL0_FRACDIVH_M1		0x00A2U
#define CMN_PLL0_HIGH_THR_M1		0x00A3U
#define CMN_PLL0_DSM_DIAG_M1		0x00A4U
#define CMN_PLL0_SS_CTRL1_M1		0x00A8U
#define CMN_PLL0_SS_CTRL2_M1		0x00A9U
#define CMN_PLL0_SS_CTRL3_M1		0x00AAU
#define CMN_PLL0_SS_CTRL4_M1		0x00ABU
#define CMN_PLL1_VCOCAL_TCTRL		0x00C2U
#define CMN_PLL1_VCOCAL_INIT_TMR	0x00C4U
#define CMN_PLL1_VCOCAL_ITER_TMR	0x00C5U
#define CMN_PLL1_VCOCAL_REFTIM_START	0x00C6U
#define CMN_PLL1_VCOCAL_PLLCNT_START	0x00C8U
#define CMN_PLL1_INTDIV_M0		0x00D0U
#define CMN_PLL1_FRACDIVL_M0		0x00D1U
#define CMN_PLL1_FRACDIVH_M0		0x00D2U
#define CMN_PLL1_HIGH_THR_M0		0x00D3U
#define CMN_PLL1_DSM_DIAG_M0		0x00D4U
#define CMN_PLL1_DSM_FBH_OVRD_M0	0x00D5U
#define CMN_PLL1_DSM_FBL_OVRD_M0	0x00D6U
#define CMN_PLL1_SS_CTRL1_M0		0x00D8U
#define CMN_PLL1_SS_CTRL2_M0		0x00D9U
#define CMN_PLL1_SS_CTRL3_M0		0x00DAU
#define CMN_PLL1_SS_CTRL4_M0		0x00DBU
#define CMN_PLL1_LOCK_REFCNT_START	0x00DCU
#define CMN_PLL1_LOCK_PLLCNT_START	0x00DEU
#define CMN_PLL1_LOCK_PLLCNT_THR	0x00DFU
#define CMN_TXPUCAL_TUNE		0x0103U
#define CMN_TXPUCAL_INIT_TMR		0x0104U
#define CMN_TXPUCAL_ITER_TMR		0x0105U
#define CMN_CMN_TXPDCAL_OVRD		0x0109U
#define CMN_TXPDCAL_TUNE		0x010BU
#define CMN_TXPDCAL_INIT_TMR		0x010CU
#define CMN_TXPDCAL_ITER_TMR		0x010DU
#define CMN_RXCAL_INIT_TMR		0x0114U
#define CMN_RXCAL_ITER_TMR		0x0115U
#define CMN_SD_CAL_INIT_TMR		0x0124U
#define CMN_SD_CAL_ITER_TMR		0x0125U
#define CMN_SD_CAL_REFTIM_START		0x0126U
#define CMN_SD_CAL_PLLCNT_START		0x0128U
#define CMN_PDIAG_PLL0_CTRL_M0		0x01A0U
#define CMN_PDIAG_PLL0_CLK_SEL_M0	0x01A1U
#define CMN_PDIAG_PLL0_CP_PADJ_M0	0x01A4U
#define CMN_PDIAG_PLL0_CP_IADJ_M0	0x01A5U
#define CMN_PDIAG_PLL0_FILT_PADJ_M0	0x01A6U
#define CMN_PDIAG_PLL0_CTRL_M1		0x01B0U
#define CMN_PDIAG_PLL0_CLK_SEL_M1	0x01B1U
#define CMN_PDIAG_PLL0_CP_PADJ_M1	0x01B4U
#define CMN_PDIAG_PLL0_CP_IADJ_M1	0x01B5U
#define CMN_PDIAG_PLL0_FILT_PADJ_M1	0x01B6U
#define CMN_PDIAG_PLL1_CTRL_M0		0x01C0U
#define CMN_PDIAG_PLL1_CLK_SEL_M0	0x01C1U
#define CMN_PDIAG_PLL1_CP_PADJ_M0	0x01C4U
#define CMN_PDIAG_PLL1_CP_IADJ_M0	0x01C5U
#define CMN_PDIAG_PLL1_FILT_PADJ_M0	0x01C6U
#define CMN_DIAG_BIAS_OVRD1		0x01E1U

/* PMA TX Lane registers */
#define TX_TXCC_CTRL			0x0040U
#define TX_TXCC_CPOST_MULT_00		0x004CU
#define TX_TXCC_CPOST_MULT_01		0x004DU
#define TX_TXCC_MGNFS_MULT_000		0x0050U
#define TX_TXCC_MGNFS_MULT_100		0x0054U
#define DRV_DIAG_TX_DRV			0x00C6U
#define XCVR_DIAG_PLLDRC_CTRL		0x00E5U
#define XCVR_DIAG_HSCLK_SEL		0x00E6U
#define XCVR_DIAG_HSCLK_DIV		0x00E7U
#define XCVR_DIAG_RXCLK_CTRL		0x00E9U
#define XCVR_DIAG_BIDI_CTRL		0x00EAU
#define XCVR_DIAG_PSC_OVRD		0x00EBU
#define TX_PSC_A0			0x0100U
#define TX_PSC_A1			0x0101U
#define TX_PSC_A2			0x0102U
#define TX_PSC_A3			0x0103U
#define TX_RCVDET_ST_TMR		0x0123U
#define TX_DIAG_ACYA			0x01E7U
#define TX_DIAG_ACYA_HBDC_MASK		0x0001U

/* PMA RX Lane registers */
#define RX_PSC_A0			0x0000U
#define RX_PSC_A1			0x0001U
#define RX_PSC_A2			0x0002U
#define RX_PSC_A3			0x0003U
#define RX_PSC_CAL			0x0006U
#define RX_CDRLF_CNFG			0x0080U
#define RX_CDRLF_CNFG3			0x0082U
#define RX_SIGDET_HL_FILT_TMR		0x0090U
#define RX_REE_GCSM1_CTRL		0x0108U
#define RX_REE_GCSM1_EQENM_PH1		0x0109U
#define RX_REE_GCSM1_EQENM_PH2		0x010AU
#define RX_REE_GCSM2_CTRL		0x0110U
#define RX_REE_PERGCSM_CTRL		0x0118U
#define RX_REE_ATTEN_THR		0x0149U
#define RX_REE_TAP1_CLIP		0x0171U
#define RX_REE_TAP2TON_CLIP		0x0172U
#define RX_REE_SMGM_CTRL1		0x0177U
#define RX_REE_SMGM_CTRL2		0x0178U
#define RX_DIAG_DFE_CTRL		0x01E0U
#define RX_DIAG_DFE_AMP_TUNE_2		0x01E2U
#define RX_DIAG_DFE_AMP_TUNE_3		0x01E3U
#define RX_DIAG_NQST_CTRL		0x01E5U
#define RX_DIAG_SIGDET_TUNE		0x01E8U
#define RX_DIAG_PI_RATE			0x01F4U
#define RX_DIAG_PI_CAP			0x01F5U
#define RX_DIAG_ACYA			0x01FFU

/* PHY PCS common registers */
#define PHY_PLL_CFG			0x000EU
#define PHY_PIPE_USB3_GEN2_PRE_CFG0	0x0020U
#define PHY_PIPE_USB3_GEN2_POST_CFG0	0x0022U
#define PHY_PIPE_USB3_GEN2_POST_CFG1	0x0023U

/* PHY PMA common registers */
#define PHY_PMA_CMN_CTRL1		0x0000U
#define PHY_PMA_CMN_CTRL2		0x0001U
#define PHY_PMA_PLL_RAW_CTRL		0x0003U

static const struct reg_field phy_pll_cfg =  REG_FIELD(PHY_PLL_CFG, 0, 1);
static const struct reg_field phy_pma_cmn_ctrl_1 =
					REG_FIELD(PHY_PMA_CMN_CTRL1, 0, 0);
static const struct reg_field phy_pma_cmn_ctrl_2 =
					REG_FIELD(PHY_PMA_CMN_CTRL2, 0, 7);
static const struct reg_field phy_pma_pll_raw_ctrl =
					REG_FIELD(PHY_PMA_PLL_RAW_CTRL, 0, 1);

#define reset_control_assert reset_assert
#define reset_control_deassert reset_deassert
#define reset_control reset_ctl
#define reset_control_put reset_free

enum cdns_torrent_phy_type {
	TYPE_NONE,
	TYPE_DP,
	TYPE_PCIE,
	TYPE_SGMII,
	TYPE_QSGMII,
	TYPE_USB,
};

enum cdns_torrent_ssc_mode {
	NO_SSC,
	EXTERNAL_SSC,
	INTERNAL_SSC
};

struct cdns_torrent_inst {
	struct phy *phy;
	u32 mlane;
	enum cdns_torrent_phy_type phy_type;
	u32 num_lanes;
	struct reset_ctl_bulk *lnk_rst;
	enum cdns_torrent_ssc_mode ssc_mode;
};

struct cdns_torrent_phy {
	void __iomem *sd_base;	/* SD0801 register base  */
	size_t size;
	struct reset_control *phy_rst;
	struct udevice *dev;
	struct cdns_torrent_inst phys[MAX_NUM_LANES];
	int nsubnodes;
	const struct cdns_torrent_data *init_data;
	struct regmap *regmap;
	struct regmap *regmap_common_cdb;
	struct regmap *regmap_phy_pcs_common_cdb;
	struct regmap *regmap_phy_pma_common_cdb;
	struct regmap *regmap_tx_lane_cdb[MAX_NUM_LANES];
	struct regmap *regmap_rx_lane_cdb[MAX_NUM_LANES];
	struct regmap_field *phy_pll_cfg;
	struct regmap_field *phy_pma_cmn_ctrl_1;
	struct regmap_field *phy_pma_cmn_ctrl_2;
	struct regmap_field *phy_pma_pll_raw_ctrl;
};

struct cdns_reg_pairs {
	u32 val;
	u32 off;
};

struct cdns_torrent_vals {
	struct cdns_reg_pairs *reg_pairs;
	u32 num_regs;
};

struct cdns_torrent_data {
	u8 block_offset_shift;
	u8 reg_offset_shift;
	struct cdns_torrent_vals *link_cmn_vals[NUM_PHY_TYPE][NUM_PHY_TYPE]
					       [NUM_SSC_MODE];
	struct cdns_torrent_vals *xcvr_diag_vals[NUM_PHY_TYPE][NUM_PHY_TYPE]
						[NUM_SSC_MODE];
	struct cdns_torrent_vals *pcs_cmn_vals[NUM_PHY_TYPE][NUM_PHY_TYPE]
					       [NUM_SSC_MODE];
	struct cdns_torrent_vals *cmn_vals[NUM_PHY_TYPE][NUM_PHY_TYPE]
					  [NUM_SSC_MODE];
	struct cdns_torrent_vals *tx_ln_vals[NUM_PHY_TYPE][NUM_PHY_TYPE]
					  [NUM_SSC_MODE];
	struct cdns_torrent_vals *rx_ln_vals[NUM_PHY_TYPE][NUM_PHY_TYPE]
					    [NUM_SSC_MODE];
};

static inline struct cdns_torrent_inst *phy_get_drvdata(struct phy *phy)
{
	struct cdns_torrent_phy *sp = dev_get_priv(phy->dev);
	int index;

	if (phy->id >= MAX_NUM_LANES)
		return NULL;

	for (index = 0; index < sp->nsubnodes; index++) {
		if (phy->id == sp->phys[index].mlane)
			return &sp->phys[index];
	}

	return NULL;
}

static struct regmap *cdns_regmap_init(struct udevice *dev, void __iomem *base,
				       u32 block_offset,
				       u8 reg_offset_shift)
{
	struct cdns_torrent_phy *sp = dev_get_priv(dev);
	struct regmap_config config;

	config.r_start = (ulong)(base + block_offset);
	config.r_size = sp->size - block_offset;
	config.reg_offset_shift = reg_offset_shift;
	config.width = REGMAP_SIZE_16;

	return devm_regmap_init(dev, NULL, NULL, &config);
}

static int cdns_torrent_regfield_init(struct cdns_torrent_phy *cdns_phy)
{
	struct udevice *dev = cdns_phy->dev;
	struct regmap_field *field;
	struct regmap *regmap;

	regmap = cdns_phy->regmap_phy_pcs_common_cdb;
	field = devm_regmap_field_alloc(dev, regmap, phy_pll_cfg);
	if (IS_ERR(field)) {
		dev_err(dev, "PHY_PLL_CFG reg field init failed\n");
		return PTR_ERR(field);
	}
	cdns_phy->phy_pll_cfg = field;

	regmap = cdns_phy->regmap_phy_pma_common_cdb;
	field = devm_regmap_field_alloc(dev, regmap, phy_pma_cmn_ctrl_1);
	if (IS_ERR(field)) {
		dev_err(dev, "PHY_PMA_CMN_CTRL1 reg field init failed\n");
		return PTR_ERR(field);
	}
	cdns_phy->phy_pma_cmn_ctrl_1 = field;

	regmap = cdns_phy->regmap_phy_pma_common_cdb;
	field = devm_regmap_field_alloc(dev, regmap, phy_pma_cmn_ctrl_2);
	if (IS_ERR(field)) {
		dev_err(dev, "PHY_PMA_CMN_CTRL2 reg field init failed\n");
		return PTR_ERR(field);
	}
	cdns_phy->phy_pma_cmn_ctrl_2 = field;

	regmap = cdns_phy->regmap_phy_pma_common_cdb;
	field = devm_regmap_field_alloc(dev, regmap, phy_pma_pll_raw_ctrl);
	if (IS_ERR(field)) {
		dev_err(dev, "PHY_PMA_PLL_RAW_CTRL reg field init failed\n");
		return PTR_ERR(field);
	}
	cdns_phy->phy_pma_pll_raw_ctrl = field;

	return 0;
}

static int cdns_torrent_regmap_init(struct cdns_torrent_phy *cdns_phy)
{
	void __iomem *sd_base = cdns_phy->sd_base;
	u8 block_offset_shift, reg_offset_shift;
	struct udevice *dev = cdns_phy->dev;
	struct regmap *regmap;
	u32 block_offset;
	int i;

	block_offset_shift = cdns_phy->init_data->block_offset_shift;
	reg_offset_shift = cdns_phy->init_data->reg_offset_shift;

	for (i = 0; i < MAX_NUM_LANES; i++) {
		block_offset = TORRENT_TX_LANE_CDB_OFFSET(i, block_offset_shift,
							  reg_offset_shift);

		regmap = cdns_regmap_init(dev, sd_base, block_offset,
					  reg_offset_shift);
		if (IS_ERR(regmap)) {
			dev_err(dev, "Failed to init tx lane CDB regmap\n");
			return PTR_ERR(regmap);
		}
		cdns_phy->regmap_tx_lane_cdb[i] = regmap;
		block_offset = TORRENT_RX_LANE_CDB_OFFSET(i, block_offset_shift,
							  reg_offset_shift);
		regmap = cdns_regmap_init(dev, sd_base, block_offset,
					  reg_offset_shift);
		if (IS_ERR(regmap)) {
			dev_err(dev, "Failed to init rx lane CDB regmap");
			return PTR_ERR(regmap);
		}
		cdns_phy->regmap_rx_lane_cdb[i] = regmap;
	}

	block_offset = TORRENT_COMMON_CDB_OFFSET;
	regmap = cdns_regmap_init(dev, sd_base, block_offset,
				  reg_offset_shift);
	if (IS_ERR(regmap)) {
		dev_err(dev, "Failed to init common CDB regmap\n");
		return PTR_ERR(regmap);
	}
	cdns_phy->regmap_common_cdb = regmap;

	block_offset = TORRENT_PHY_PCS_COMMON_OFFSET(block_offset_shift);
	regmap = cdns_regmap_init(dev, sd_base, block_offset,
				  reg_offset_shift);
	if (IS_ERR(regmap)) {
		dev_err(dev, "Failed to init PHY PCS common CDB regmap\n");
		return PTR_ERR(regmap);
	}
	cdns_phy->regmap_phy_pcs_common_cdb = regmap;

	block_offset = TORRENT_PHY_PMA_COMMON_OFFSET(block_offset_shift);
	regmap = cdns_regmap_init(dev, sd_base, block_offset,
				  reg_offset_shift);
	if (IS_ERR(regmap)) {
		dev_err(dev, "Failed to init PHY PMA common CDB regmap\n");
		return PTR_ERR(regmap);
	}
	cdns_phy->regmap_phy_pma_common_cdb = regmap;

	return 0;
}

static int cdns_torrent_phy_configure_multilink(struct cdns_torrent_phy *cdns_phy)
{
	const struct cdns_torrent_data *init_data = cdns_phy->init_data;
	struct cdns_torrent_vals *cmn_vals, *tx_ln_vals, *rx_ln_vals;
	struct cdns_torrent_vals *link_cmn_vals, *xcvr_diag_vals;
	enum cdns_torrent_phy_type phy_t1, phy_t2, tmp_phy_type;
	struct cdns_torrent_vals *pcs_cmn_vals;
	int i, j, node, mlane, num_lanes, ret;
	struct cdns_reg_pairs *reg_pairs;
	enum cdns_torrent_ssc_mode ssc;
	struct regmap *regmap;
	u32 num_regs;

	/* Maximum 2 links (subnodes) are supported */
	if (cdns_phy->nsubnodes != 2)
		return -EINVAL;

	phy_t1 = cdns_phy->phys[0].phy_type;
	phy_t2 = cdns_phy->phys[1].phy_type;

	/*
	 * First configure the PHY for first link with phy_t1. Geth the array
	 * values are [phy_t1][phy_t2][ssc].
	 */
	for (node = 0; node < cdns_phy->nsubnodes; node++) {
		if (node == 1) {
			/*
			 * If fist link with phy_t1 is configured, then
			 * configure the PHY for second link with phy_t2.
			 * Get the array values as [phy_t2][phy_t1][ssc]
			 */
			tmp_phy_type = phy_t1;
			phy_t1 = phy_t2;
			phy_t2 = tmp_phy_type;
		}

		mlane = cdns_phy->phys[node].mlane;
		ssc = cdns_phy->phys[node].ssc_mode;
		num_lanes = cdns_phy->phys[node].num_lanes;

		/**
		 * PHY configuration specific registers:
		 * link_cmn_vals depend on combination of PHY types being
		 * configured and are common for both PHY types, so array
		 * values should be same for [phy_t1][phy_t2][ssc] and
		 * [phy_t2][phy_t1][ssc].
		 * xcvr_diag_vals also depend on combination of PHY types
		 * being configured, but these can be different for particular
		 * PHY type and are per lane.
		 */
		link_cmn_vals = init_data->link_cmn_vals[phy_t1][phy_t2][ssc];
		if (link_cmn_vals) {
			reg_pairs = link_cmn_vals->reg_pairs;
			num_regs = link_cmn_vals->num_regs;
			regmap = cdns_phy->regmap_common_cdb;

			/**
			 * First array value in link_cmn_vals must be of
			 * PHY_PLL_CFG register
			 */
			regmap_field_write(cdns_phy->phy_pll_cfg,
					   reg_pairs[0].val);

			for (i = 1; i < num_regs; i++)
				regmap_write(regmap, reg_pairs[i].off,
					     reg_pairs[i].val);
		}

		xcvr_diag_vals = init_data->xcvr_diag_vals[phy_t1][phy_t2][ssc];
		if (xcvr_diag_vals) {
			reg_pairs = xcvr_diag_vals->reg_pairs;
			num_regs = xcvr_diag_vals->num_regs;
			for (i = 0; i < num_lanes; i++) {
				regmap = cdns_phy->regmap_tx_lane_cdb[i + mlane];
				for (j = 0; j < num_regs; j++)
					regmap_write(regmap, reg_pairs[j].off,
						     reg_pairs[j].val);
			}
		}

		/* PHY PCS common registers configurations */
		pcs_cmn_vals = init_data->pcs_cmn_vals[phy_t1][phy_t2][ssc];
		if (pcs_cmn_vals) {
			reg_pairs = pcs_cmn_vals->reg_pairs;
			num_regs = pcs_cmn_vals->num_regs;
			regmap = cdns_phy->regmap_phy_pcs_common_cdb;
			for (i = 0; i < num_regs; i++)
				regmap_write(regmap, reg_pairs[i].off,
					     reg_pairs[i].val);
		}

		/* PMA common registers configurations */
		cmn_vals = init_data->cmn_vals[phy_t1][phy_t2][ssc];
		if (cmn_vals) {
			reg_pairs = cmn_vals->reg_pairs;
			num_regs = cmn_vals->num_regs;
			regmap = cdns_phy->regmap_common_cdb;
			for (i = 0; i < num_regs; i++)
				regmap_write(regmap, reg_pairs[i].off,
					     reg_pairs[i].val);
		}

		/* PMA TX lane registers configurations */
		tx_ln_vals = init_data->tx_ln_vals[phy_t1][phy_t2][ssc];
		if (tx_ln_vals) {
			reg_pairs = tx_ln_vals->reg_pairs;
			num_regs = tx_ln_vals->num_regs;
			for (i = 0; i < num_lanes; i++) {
				regmap = cdns_phy->regmap_tx_lane_cdb[i + mlane];
				for (j = 0; j < num_regs; j++)
					regmap_write(regmap, reg_pairs[j].off,
						     reg_pairs[j].val);
			}
		}

		/* PMA RX lane registers configurations */
		rx_ln_vals = init_data->rx_ln_vals[phy_t1][phy_t2][ssc];
		if (rx_ln_vals) {
			reg_pairs = rx_ln_vals->reg_pairs;
			num_regs = rx_ln_vals->num_regs;
			for (i = 0; i < num_lanes; i++) {
				regmap = cdns_phy->regmap_rx_lane_cdb[i + mlane];
				for (j = 0; j < num_regs; j++)
					regmap_write(regmap, reg_pairs[j].off,
						     reg_pairs[j].val);
			}
		}

		reset_deassert_bulk(cdns_phy->phys[node].lnk_rst);
	}

	/* Take the PHY out of reset */
	ret = reset_control_deassert(cdns_phy->phy_rst);
	if (ret)
		return ret;

	return 0;
}

static int cdns_torrent_phy_probe(struct udevice *dev)
{
	struct cdns_torrent_phy *cdns_phy = dev_get_priv(dev);
	int ret, subnodes = 0, node = 0, i;
	struct cdns_torrent_data *data;
	u32 total_num_lanes = 0;
	struct clk *clk;
	ofnode child;
	u32 phy_type;

	cdns_phy->dev = dev;

	/* Get init data for this phy  */
	data = (struct cdns_torrent_data *)dev_get_driver_data(dev);
	cdns_phy->init_data = data;

	cdns_phy->phy_rst = devm_reset_control_get_by_index(dev, 0);
	if (IS_ERR(cdns_phy->phy_rst)) {
		dev_err(dev, "failed to get reset\n");
		return PTR_ERR(cdns_phy->phy_rst);
	}

	clk = devm_clk_get(dev, "refclk");
	if (IS_ERR(clk)) {
		dev_err(dev, "phy ref clock not found\n");
		return PTR_ERR(clk);
	}

	ret = clk_prepare_enable(clk);
	if (ret) {
		dev_err(cdns_phy->dev, "Failed to prepare ref clock\n");
		return ret;
	}

	cdns_phy->sd_base = devfdt_remap_addr_index(dev, 0);
	if (IS_ERR(cdns_phy->sd_base))
		return PTR_ERR(cdns_phy->sd_base);
	devfdt_get_addr_size_index(dev, 0, (fdt_size_t *)&cdns_phy->size);

	dev_for_each_subnode(child, dev)
		subnodes++;
	if (subnodes == 0) {
		dev_err(dev, "No available link subnodes found\n");
		return -EINVAL;
	}
	ret = cdns_torrent_regmap_init(cdns_phy);
	if (ret)
		return ret;

	ret = cdns_torrent_regfield_init(cdns_phy);
	if (ret)
		return ret;

	/* Going through all the available subnodes or children*/
	ofnode_for_each_subnode(child, dev_ofnode(dev)) {
		/* PHY subnode name must be a 'phy' */
		if (!ofnode_name_eq(child, "phy"))
			continue;
		cdns_phy->phys[node].lnk_rst =
				devm_reset_bulk_get_by_node(dev, child);
		if (IS_ERR(cdns_phy->phys[node].lnk_rst)) {
			dev_err(dev, "%s: failed to get reset\n",
				ofnode_get_name(child));
			ret = PTR_ERR(cdns_phy->phys[node].lnk_rst);
			goto put_lnk_rst;
		}

		if (ofnode_read_u32(child, "reg",
				    &cdns_phy->phys[node].mlane)) {
			dev_err(dev, "%s: No \"reg \" - property.\n",
				ofnode_get_name(child));
			ret = -EINVAL;
			goto put_child;
		}

		if (ofnode_read_u32(child, "cdns,phy-type", &phy_type)) {
			dev_err(dev, "%s: No \"cdns,phy-type \" - property.\n",
				ofnode_get_name(child));
			ret = -EINVAL;
			goto put_child;
		}

		switch (phy_type) {
		case PHY_TYPE_PCIE:
			cdns_phy->phys[node].phy_type = TYPE_PCIE;
			break;
		case PHY_TYPE_DP:
			cdns_phy->phys[node].phy_type = TYPE_DP;
			break;
		case PHY_TYPE_SGMII:
			cdns_phy->phys[node].phy_type = TYPE_SGMII;
			break;
		case PHY_TYPE_QSGMII:
			cdns_phy->phys[node].phy_type = TYPE_QSGMII;
			break;
		case PHY_TYPE_USB3:
			cdns_phy->phys[node].phy_type = TYPE_USB;
			break;
		default:
			dev_err(dev, "Unsupported protocol\n");
			ret = -EINVAL;
			goto put_child;
		}

		if (ofnode_read_u32(child, "cdns,num-lanes",
				    &cdns_phy->phys[node].num_lanes)) {
			dev_err(dev, "%s: No \"cdns,num-lanes \" - property.\n",
				ofnode_get_name(child));
			ret = -EINVAL;
			goto put_child;
		}

		total_num_lanes += cdns_phy->phys[node].num_lanes;

		/* Get SSC mode */
		ofnode_read_u32(child, "cdns,ssc-mode",
				&cdns_phy->phys[node].ssc_mode);
		node++;
	}

	cdns_phy->nsubnodes = node;

	if (total_num_lanes > MAX_NUM_LANES) {
		dev_err(dev, "Invalid lane configuration\n");
		goto put_lnk_rst;
	}

	if (cdns_phy->nsubnodes > 1) {
		ret = cdns_torrent_phy_configure_multilink(cdns_phy);
		if (ret)
			goto put_lnk_rst;
	}

	reset_control_deassert(cdns_phy->phy_rst);
	return 0;

put_child:
	node++;
put_lnk_rst:
	for (i = 0; i < node; i++)
		reset_release_bulk(cdns_phy->phys[i].lnk_rst);
	return ret;
}

static int cdns_torrent_phy_on(struct phy *gphy)
{
	struct cdns_torrent_inst *inst = phy_get_drvdata(gphy);
	struct cdns_torrent_phy *cdns_phy = dev_get_priv(gphy->dev);
	u32 read_val;
	int ret;

	if (cdns_phy->nsubnodes == 1) {
		/* Take the PHY lane group out of reset */
		reset_deassert_bulk(inst->lnk_rst);

		/* Take the PHY out of reset */
		ret = reset_control_deassert(cdns_phy->phy_rst);
		if (ret)
			return ret;
	}

	/*
	 * Wait for cmn_ready assertion
	 * PHY_PMA_CMN_CTRL1[0] == 1
	 */
	ret = regmap_field_read_poll_timeout(cdns_phy->phy_pma_cmn_ctrl_1,
					     read_val, read_val, 1000,
					     PLL_LOCK_TIMEOUT);
	if (ret) {
		dev_err(cdns_phy->dev, "Timeout waiting for CMN ready\n");
		return ret;
	}
	mdelay(10);

	return 0;
}

static int cdns_torrent_phy_init(struct phy *phy)
{
	struct cdns_torrent_phy *cdns_phy = dev_get_priv(phy->dev);
	const struct cdns_torrent_data *init_data = cdns_phy->init_data;
	struct cdns_torrent_vals *cmn_vals, *tx_ln_vals, *rx_ln_vals;
	struct cdns_torrent_vals *link_cmn_vals, *xcvr_diag_vals;
	struct cdns_torrent_inst *inst = phy_get_drvdata(phy);
	enum cdns_torrent_phy_type phy_type = inst->phy_type;
	enum cdns_torrent_ssc_mode ssc = inst->ssc_mode;
	struct cdns_torrent_vals *pcs_cmn_vals;
	struct cdns_reg_pairs *reg_pairs;
	struct regmap *regmap;
	u32 num_regs;
	int i, j;

	if (cdns_phy->nsubnodes > 1)
		return 0;

	/**
	 * Spread spectrum generation is not required or supported
	 * for SGMII/QSGMII
	 */
	if (phy_type == TYPE_SGMII || phy_type == TYPE_QSGMII)
		ssc = NO_SSC;

	/* PHY configuration specific registers for single link */
	link_cmn_vals = init_data->link_cmn_vals[phy_type][TYPE_NONE][ssc];
	if (link_cmn_vals) {
		reg_pairs = link_cmn_vals->reg_pairs;
		num_regs = link_cmn_vals->num_regs;
		regmap = cdns_phy->regmap_common_cdb;

		/**
		 * First array value in link_cmn_vals must be of
		 * PHY_PLL_CFG register
		 */
		regmap_field_write(cdns_phy->phy_pll_cfg, reg_pairs[0].val);

		for (i = 1; i < num_regs; i++)
			regmap_write(regmap, reg_pairs[i].off,
				     reg_pairs[i].val);
	}

	xcvr_diag_vals = init_data->xcvr_diag_vals[phy_type][TYPE_NONE][ssc];
	if (xcvr_diag_vals) {
		reg_pairs = xcvr_diag_vals->reg_pairs;
		num_regs = xcvr_diag_vals->num_regs;
		for (i = 0; i < inst->num_lanes; i++) {
			regmap = cdns_phy->regmap_tx_lane_cdb[i + inst->mlane];
			for (j = 0; j < num_regs; j++)
				regmap_write(regmap, reg_pairs[j].off,
					     reg_pairs[j].val);
		}
	}

	/* PHY PCS common registers configurations */
	pcs_cmn_vals = init_data->pcs_cmn_vals[phy_type][TYPE_NONE][ssc];
	if (pcs_cmn_vals) {
		reg_pairs = pcs_cmn_vals->reg_pairs;
		num_regs = pcs_cmn_vals->num_regs;
		regmap = cdns_phy->regmap_phy_pcs_common_cdb;
		for (i = 0; i < num_regs; i++)
			regmap_write(regmap, reg_pairs[i].off,
				     reg_pairs[i].val);
	}

	/* PMA common registers configurations */
	cmn_vals = init_data->cmn_vals[phy_type][TYPE_NONE][ssc];
	if (cmn_vals) {
		reg_pairs = cmn_vals->reg_pairs;
		num_regs = cmn_vals->num_regs;
		regmap = cdns_phy->regmap_common_cdb;
		for (i = 0; i < num_regs; i++)
			regmap_write(regmap, reg_pairs[i].off,
				     reg_pairs[i].val);
	}

	/* PMA TX lane registers configurations */
	tx_ln_vals = init_data->tx_ln_vals[phy_type][TYPE_NONE][ssc];
	if (tx_ln_vals) {
		reg_pairs = tx_ln_vals->reg_pairs;
		num_regs = tx_ln_vals->num_regs;
		for (i = 0; i < inst->num_lanes; i++) {
			regmap = cdns_phy->regmap_tx_lane_cdb[i + inst->mlane];
			for (j = 0; j < num_regs; j++)
				regmap_write(regmap, reg_pairs[j].off,
					     reg_pairs[j].val);
		}
	}

	/* PMA RX lane registers configurations */
	rx_ln_vals = init_data->rx_ln_vals[phy_type][TYPE_NONE][ssc];
	if (rx_ln_vals) {
		reg_pairs = rx_ln_vals->reg_pairs;
		num_regs = rx_ln_vals->num_regs;
		for (i = 0; i < inst->num_lanes; i++) {
			regmap = cdns_phy->regmap_rx_lane_cdb[i + inst->mlane];
			for (j = 0; j < num_regs; j++)
				regmap_write(regmap, reg_pairs[j].off,
					     reg_pairs[j].val);
		}
	}

	return 0;
}

static int cdns_torrent_phy_off(struct phy *gphy)
{
	struct cdns_torrent_inst *inst = phy_get_drvdata(gphy);
	struct cdns_torrent_phy *cdns_phy = dev_get_priv(gphy->dev);
	int ret;

	if (cdns_phy->nsubnodes != 1)
		return 0;

	ret = reset_control_assert(cdns_phy->phy_rst);
	if (ret)
		return ret;

	return reset_assert_bulk(inst->lnk_rst);
}

static int cdns_torrent_phy_remove(struct udevice *dev)
{
	struct cdns_torrent_phy *cdns_phy = dev_get_priv(dev);
	int i;

	reset_control_assert(cdns_phy->phy_rst);
	for (i = 0; i < cdns_phy->nsubnodes; i++)
		reset_release_bulk(cdns_phy->phys[i].lnk_rst);

	return 0;
}

/* USB and SGMII/QSGMII link configuration */
static struct cdns_reg_pairs usb_sgmii_link_cmn_regs[] = {
	{0x0002, PHY_PLL_CFG},
	{0x8600, CMN_PDIAG_PLL0_CLK_SEL_M0},
	{0x0601, CMN_PDIAG_PLL1_CLK_SEL_M0}
};

static struct cdns_reg_pairs usb_sgmii_xcvr_diag_ln_regs[] = {
	{0x0000, XCVR_DIAG_HSCLK_SEL},
	{0x0001, XCVR_DIAG_HSCLK_DIV},
	{0x0041, XCVR_DIAG_PLLDRC_CTRL}
};

static struct cdns_reg_pairs sgmii_usb_xcvr_diag_ln_regs[] = {
	{0x0011, XCVR_DIAG_HSCLK_SEL},
	{0x0003, XCVR_DIAG_HSCLK_DIV},
	{0x009B, XCVR_DIAG_PLLDRC_CTRL}
};

static struct cdns_torrent_vals usb_sgmii_link_cmn_vals = {
	.reg_pairs = usb_sgmii_link_cmn_regs,
	.num_regs = ARRAY_SIZE(usb_sgmii_link_cmn_regs),
};

static struct cdns_torrent_vals usb_sgmii_xcvr_diag_ln_vals = {
	.reg_pairs = usb_sgmii_xcvr_diag_ln_regs,
	.num_regs = ARRAY_SIZE(usb_sgmii_xcvr_diag_ln_regs),
};

static struct cdns_torrent_vals sgmii_usb_xcvr_diag_ln_vals = {
	.reg_pairs = sgmii_usb_xcvr_diag_ln_regs,
	.num_regs = ARRAY_SIZE(sgmii_usb_xcvr_diag_ln_regs),
};

/* PCIe and USB Unique SSC link configuration */
static struct cdns_reg_pairs pcie_usb_link_cmn_regs[] = {
	{0x0003, PHY_PLL_CFG},
	{0x0601, CMN_PDIAG_PLL0_CLK_SEL_M0},
	{0x0400, CMN_PDIAG_PLL0_CLK_SEL_M1},
	{0x8600, CMN_PDIAG_PLL1_CLK_SEL_M0}
};

static struct cdns_reg_pairs pcie_usb_xcvr_diag_ln_regs[] = {
	{0x0000, XCVR_DIAG_HSCLK_SEL},
	{0x0001, XCVR_DIAG_HSCLK_DIV},
	{0x0012, XCVR_DIAG_PLLDRC_CTRL}
};

static struct cdns_reg_pairs usb_pcie_xcvr_diag_ln_regs[] = {
	{0x0011, XCVR_DIAG_HSCLK_SEL},
	{0x0001, XCVR_DIAG_HSCLK_DIV},
	{0x00C9, XCVR_DIAG_PLLDRC_CTRL}
};

static struct cdns_torrent_vals pcie_usb_link_cmn_vals = {
	.reg_pairs = pcie_usb_link_cmn_regs,
	.num_regs = ARRAY_SIZE(pcie_usb_link_cmn_regs),
};

static struct cdns_torrent_vals pcie_usb_xcvr_diag_ln_vals = {
	.reg_pairs = pcie_usb_xcvr_diag_ln_regs,
	.num_regs = ARRAY_SIZE(pcie_usb_xcvr_diag_ln_regs),
};

static struct cdns_torrent_vals usb_pcie_xcvr_diag_ln_vals = {
	.reg_pairs = usb_pcie_xcvr_diag_ln_regs,
	.num_regs = ARRAY_SIZE(usb_pcie_xcvr_diag_ln_regs),
};

/* USB 100 MHz Ref clk, internal SSC */
static struct cdns_reg_pairs usb_100_int_ssc_cmn_regs[] = {
	{0x0004, CMN_PLL0_DSM_DIAG_M0},
	{0x0004, CMN_PLL0_DSM_DIAG_M1},
	{0x0004, CMN_PLL1_DSM_DIAG_M0},
	{0x0509, CMN_PDIAG_PLL0_CP_PADJ_M0},
	{0x0509, CMN_PDIAG_PLL0_CP_PADJ_M1},
	{0x0509, CMN_PDIAG_PLL1_CP_PADJ_M0},
	{0x0F00, CMN_PDIAG_PLL0_CP_IADJ_M0},
	{0x0F00, CMN_PDIAG_PLL0_CP_IADJ_M1},
	{0x0F00, CMN_PDIAG_PLL1_CP_IADJ_M0},
	{0x0F08, CMN_PDIAG_PLL0_FILT_PADJ_M0},
	{0x0F08, CMN_PDIAG_PLL0_FILT_PADJ_M1},
	{0x0F08, CMN_PDIAG_PLL1_FILT_PADJ_M0},
	{0x0064, CMN_PLL0_INTDIV_M0},
	{0x0050, CMN_PLL0_INTDIV_M1},
	{0x0064, CMN_PLL1_INTDIV_M0},
	{0x0002, CMN_PLL0_FRACDIVH_M0},
	{0x0002, CMN_PLL0_FRACDIVH_M1},
	{0x0002, CMN_PLL1_FRACDIVH_M0},
	{0x0044, CMN_PLL0_HIGH_THR_M0},
	{0x0036, CMN_PLL0_HIGH_THR_M1},
	{0x0044, CMN_PLL1_HIGH_THR_M0},
	{0x0002, CMN_PDIAG_PLL0_CTRL_M0},
	{0x0002, CMN_PDIAG_PLL0_CTRL_M1},
	{0x0002, CMN_PDIAG_PLL1_CTRL_M0},
	{0x0001, CMN_PLL0_SS_CTRL1_M0},
	{0x0001, CMN_PLL0_SS_CTRL1_M1},
	{0x0001, CMN_PLL1_SS_CTRL1_M0},
	{0x011B, CMN_PLL0_SS_CTRL2_M0},
	{0x011B, CMN_PLL0_SS_CTRL2_M1},
	{0x011B, CMN_PLL1_SS_CTRL2_M0},
	{0x006E, CMN_PLL0_SS_CTRL3_M0},
	{0x0058, CMN_PLL0_SS_CTRL3_M1},
	{0x006E, CMN_PLL1_SS_CTRL3_M0},
	{0x000E, CMN_PLL0_SS_CTRL4_M0},
	{0x0012, CMN_PLL0_SS_CTRL4_M1},
	{0x000E, CMN_PLL1_SS_CTRL4_M0},
	{0x0C5E, CMN_PLL0_VCOCAL_REFTIM_START},
	{0x0C5E, CMN_PLL1_VCOCAL_REFTIM_START},
	{0x0C56, CMN_PLL0_VCOCAL_PLLCNT_START},
	{0x0C56, CMN_PLL1_VCOCAL_PLLCNT_START},
	{0x00C7, CMN_PLL0_LOCK_REFCNT_START},
	{0x00C7, CMN_PLL1_LOCK_REFCNT_START},
	{0x00C7, CMN_PLL0_LOCK_PLLCNT_START},
	{0x00C7, CMN_PLL1_LOCK_PLLCNT_START},
	{0x0005, CMN_PLL0_LOCK_PLLCNT_THR},
	{0x0005, CMN_PLL1_LOCK_PLLCNT_THR},
	{0x8200, CMN_CDIAG_CDB_PWRI_OVRD},
	{0x8200, CMN_CDIAG_XCVRC_PWRI_OVRD},
	{0x007F, CMN_TXPUCAL_TUNE},
	{0x007F, CMN_TXPDCAL_TUNE}
};

static struct cdns_torrent_vals usb_100_int_ssc_cmn_vals = {
	.reg_pairs = usb_100_int_ssc_cmn_regs,
	.num_regs = ARRAY_SIZE(usb_100_int_ssc_cmn_regs),
};

/* Single USB link configuration */
static struct cdns_reg_pairs sl_usb_link_cmn_regs[] = {
	{0x0000, PHY_PLL_CFG},
	{0x8600, CMN_PDIAG_PLL0_CLK_SEL_M0}
};

static struct cdns_reg_pairs sl_usb_xcvr_diag_ln_regs[] = {
	{0x0000, XCVR_DIAG_HSCLK_SEL},
	{0x0001, XCVR_DIAG_HSCLK_DIV},
	{0x0041, XCVR_DIAG_PLLDRC_CTRL}
};

static struct cdns_torrent_vals sl_usb_link_cmn_vals = {
	.reg_pairs = sl_usb_link_cmn_regs,
	.num_regs = ARRAY_SIZE(sl_usb_link_cmn_regs),
};

static struct cdns_torrent_vals sl_usb_xcvr_diag_ln_vals = {
	.reg_pairs = sl_usb_xcvr_diag_ln_regs,
	.num_regs = ARRAY_SIZE(sl_usb_xcvr_diag_ln_regs),
};

/* USB PHY PCS common configuration */
static struct cdns_reg_pairs usb_phy_pcs_cmn_regs[] = {
	{0x0A0A, PHY_PIPE_USB3_GEN2_PRE_CFG0},
	{0x1000, PHY_PIPE_USB3_GEN2_POST_CFG0},
	{0x0010, PHY_PIPE_USB3_GEN2_POST_CFG1}
};

static struct cdns_torrent_vals usb_phy_pcs_cmn_vals = {
	.reg_pairs = usb_phy_pcs_cmn_regs,
	.num_regs = ARRAY_SIZE(usb_phy_pcs_cmn_regs),
};

/* USB 100 MHz Ref clk, no SSC */
static struct cdns_reg_pairs sl_usb_100_no_ssc_cmn_regs[] = {
	{0x0028, CMN_PDIAG_PLL1_CP_PADJ_M0},
	{0x001E, CMN_PLL1_DSM_FBH_OVRD_M0},
	{0x000C, CMN_PLL1_DSM_FBL_OVRD_M0},
	{0x0003, CMN_PLL0_VCOCAL_TCTRL},
	{0x0003, CMN_PLL1_VCOCAL_TCTRL},
	{0x8200, CMN_CDIAG_CDB_PWRI_OVRD},
	{0x8200, CMN_CDIAG_XCVRC_PWRI_OVRD}
};

static struct cdns_torrent_vals sl_usb_100_no_ssc_cmn_vals = {
	.reg_pairs = sl_usb_100_no_ssc_cmn_regs,
	.num_regs = ARRAY_SIZE(sl_usb_100_no_ssc_cmn_regs),
};

static struct cdns_reg_pairs usb_100_no_ssc_cmn_regs[] = {
	{0x8200, CMN_CDIAG_CDB_PWRI_OVRD},
	{0x8200, CMN_CDIAG_XCVRC_PWRI_OVRD},
	{0x007F, CMN_TXPUCAL_TUNE},
	{0x007F, CMN_TXPDCAL_TUNE}
};

static struct cdns_reg_pairs usb_100_no_ssc_tx_ln_regs[] = {
	{0x02FF, TX_PSC_A0},
	{0x06AF, TX_PSC_A1},
	{0x06AE, TX_PSC_A2},
	{0x06AE, TX_PSC_A3},
	{0x2A82, TX_TXCC_CTRL},
	{0x0014, TX_TXCC_CPOST_MULT_01},
	{0x0003, XCVR_DIAG_PSC_OVRD}
};

static struct cdns_reg_pairs usb_100_no_ssc_rx_ln_regs[] = {
	{0x0D1D, RX_PSC_A0},
	{0x0D1D, RX_PSC_A1},
	{0x0D00, RX_PSC_A2},
	{0x0500, RX_PSC_A3},
	{0x0013, RX_SIGDET_HL_FILT_TMR},
	{0x0000, RX_REE_GCSM1_CTRL},
	{0x0C02, RX_REE_ATTEN_THR},
	{0x0330, RX_REE_SMGM_CTRL1},
	{0x0300, RX_REE_SMGM_CTRL2},
	{0x0019, RX_REE_TAP1_CLIP},
	{0x0019, RX_REE_TAP2TON_CLIP},
	{0x1004, RX_DIAG_SIGDET_TUNE},
	{0x00F9, RX_DIAG_NQST_CTRL},
	{0x0C01, RX_DIAG_DFE_AMP_TUNE_2},
	{0x0002, RX_DIAG_DFE_AMP_TUNE_3},
	{0x0000, RX_DIAG_PI_CAP},
	{0x0031, RX_DIAG_PI_RATE},
	{0x0001, RX_DIAG_ACYA},
	{0x018C, RX_CDRLF_CNFG},
	{0x0003, RX_CDRLF_CNFG3}
};

static struct cdns_torrent_vals usb_100_no_ssc_cmn_vals = {
	.reg_pairs = usb_100_no_ssc_cmn_regs,
	.num_regs = ARRAY_SIZE(usb_100_no_ssc_cmn_regs),
};

static struct cdns_torrent_vals usb_100_no_ssc_tx_ln_vals = {
	.reg_pairs = usb_100_no_ssc_tx_ln_regs,
	.num_regs = ARRAY_SIZE(usb_100_no_ssc_tx_ln_regs),
};

static struct cdns_torrent_vals usb_100_no_ssc_rx_ln_vals = {
	.reg_pairs = usb_100_no_ssc_rx_ln_regs,
	.num_regs = ARRAY_SIZE(usb_100_no_ssc_rx_ln_regs),
};

/* Single link USB, 100 MHz Ref clk, internal SSC */
static struct cdns_reg_pairs sl_usb_100_int_ssc_cmn_regs[] = {
	{0x0004, CMN_PLL0_DSM_DIAG_M0},
	{0x0004, CMN_PLL1_DSM_DIAG_M0},
	{0x0509, CMN_PDIAG_PLL0_CP_PADJ_M0},
	{0x0509, CMN_PDIAG_PLL1_CP_PADJ_M0},
	{0x0F00, CMN_PDIAG_PLL0_CP_IADJ_M0},
	{0x0F00, CMN_PDIAG_PLL1_CP_IADJ_M0},
	{0x0F08, CMN_PDIAG_PLL0_FILT_PADJ_M0},
	{0x0F08, CMN_PDIAG_PLL1_FILT_PADJ_M0},
	{0x0064, CMN_PLL0_INTDIV_M0},
	{0x0064, CMN_PLL1_INTDIV_M0},
	{0x0002, CMN_PLL0_FRACDIVH_M0},
	{0x0002, CMN_PLL1_FRACDIVH_M0},
	{0x0044, CMN_PLL0_HIGH_THR_M0},
	{0x0044, CMN_PLL1_HIGH_THR_M0},
	{0x0002, CMN_PDIAG_PLL0_CTRL_M0},
	{0x0002, CMN_PDIAG_PLL1_CTRL_M0},
	{0x0001, CMN_PLL0_SS_CTRL1_M0},
	{0x0001, CMN_PLL1_SS_CTRL1_M0},
	{0x011B, CMN_PLL0_SS_CTRL2_M0},
	{0x011B, CMN_PLL1_SS_CTRL2_M0},
	{0x006E, CMN_PLL0_SS_CTRL3_M0},
	{0x006E, CMN_PLL1_SS_CTRL3_M0},
	{0x000E, CMN_PLL0_SS_CTRL4_M0},
	{0x000E, CMN_PLL1_SS_CTRL4_M0},
	{0x0C5E, CMN_PLL0_VCOCAL_REFTIM_START},
	{0x0C5E, CMN_PLL1_VCOCAL_REFTIM_START},
	{0x0C56, CMN_PLL0_VCOCAL_PLLCNT_START},
	{0x0C56, CMN_PLL1_VCOCAL_PLLCNT_START},
	{0x0003, CMN_PLL0_VCOCAL_TCTRL},
	{0x0003, CMN_PLL1_VCOCAL_TCTRL},
	{0x00C7, CMN_PLL0_LOCK_REFCNT_START},
	{0x00C7, CMN_PLL1_LOCK_REFCNT_START},
	{0x00C7, CMN_PLL0_LOCK_PLLCNT_START},
	{0x00C7, CMN_PLL1_LOCK_PLLCNT_START},
	{0x0005, CMN_PLL0_LOCK_PLLCNT_THR},
	{0x0005, CMN_PLL1_LOCK_PLLCNT_THR},
	{0x8200, CMN_CDIAG_CDB_PWRI_OVRD},
	{0x8200, CMN_CDIAG_XCVRC_PWRI_OVRD}
};

static struct cdns_torrent_vals sl_usb_100_int_ssc_cmn_vals = {
	.reg_pairs = sl_usb_100_int_ssc_cmn_regs,
	.num_regs = ARRAY_SIZE(sl_usb_100_int_ssc_cmn_regs),
};

/* PCIe and SGMII/QSGMII Unique SSC link configuration */
static struct cdns_reg_pairs pcie_sgmii_link_cmn_regs[] = {
	{0x0003, PHY_PLL_CFG},
	{0x0601, CMN_PDIAG_PLL0_CLK_SEL_M0},
	{0x0400, CMN_PDIAG_PLL0_CLK_SEL_M1},
	{0x0601, CMN_PDIAG_PLL1_CLK_SEL_M0}
};

static struct cdns_reg_pairs pcie_sgmii_xcvr_diag_ln_regs[] = {
	{0x0000, XCVR_DIAG_HSCLK_SEL},
	{0x0001, XCVR_DIAG_HSCLK_DIV},
	{0x0012, XCVR_DIAG_PLLDRC_CTRL}
};

static struct cdns_reg_pairs sgmii_pcie_xcvr_diag_ln_regs[] = {
	{0x0011, XCVR_DIAG_HSCLK_SEL},
	{0x0003, XCVR_DIAG_HSCLK_DIV},
	{0x009B, XCVR_DIAG_PLLDRC_CTRL}
};

static struct cdns_torrent_vals pcie_sgmii_link_cmn_vals = {
	.reg_pairs = pcie_sgmii_link_cmn_regs,
	.num_regs = ARRAY_SIZE(pcie_sgmii_link_cmn_regs),
};

static struct cdns_torrent_vals pcie_sgmii_xcvr_diag_ln_vals = {
	.reg_pairs = pcie_sgmii_xcvr_diag_ln_regs,
	.num_regs = ARRAY_SIZE(pcie_sgmii_xcvr_diag_ln_regs),
};

static struct cdns_torrent_vals sgmii_pcie_xcvr_diag_ln_vals = {
	.reg_pairs = sgmii_pcie_xcvr_diag_ln_regs,
	.num_regs = ARRAY_SIZE(sgmii_pcie_xcvr_diag_ln_regs),
};

/* SGMII 100 MHz Ref clk, no SSC */
static struct cdns_reg_pairs sl_sgmii_100_no_ssc_cmn_regs[] = {
	{0x0028, CMN_PDIAG_PLL1_CP_PADJ_M0},
	{0x001E, CMN_PLL1_DSM_FBH_OVRD_M0},
	{0x000C, CMN_PLL1_DSM_FBL_OVRD_M0},
	{0x0003, CMN_PLL0_VCOCAL_TCTRL},
	{0x0003, CMN_PLL1_VCOCAL_TCTRL}
};

static struct cdns_torrent_vals sl_sgmii_100_no_ssc_cmn_vals = {
	.reg_pairs = sl_sgmii_100_no_ssc_cmn_regs,
	.num_regs = ARRAY_SIZE(sl_sgmii_100_no_ssc_cmn_regs),
};

static struct cdns_reg_pairs sgmii_100_no_ssc_cmn_regs[] = {
	{0x007F, CMN_TXPUCAL_TUNE},
	{0x007F, CMN_TXPDCAL_TUNE}
};

static struct cdns_reg_pairs sgmii_100_no_ssc_tx_ln_regs[] = {
	{0x00F3, TX_PSC_A0},
	{0x04A2, TX_PSC_A2},
	{0x04A2, TX_PSC_A3},
	{0x0000, TX_TXCC_CPOST_MULT_00},
	{0x00B3, DRV_DIAG_TX_DRV}
};

static struct cdns_reg_pairs ti_sgmii_100_no_ssc_tx_ln_regs[] = {
	{0x00F3, TX_PSC_A0},
	{0x04A2, TX_PSC_A2},
	{0x04A2, TX_PSC_A3},
	{0x0000, TX_TXCC_CPOST_MULT_00},
	{0x00B3, DRV_DIAG_TX_DRV},
	{0x4000, XCVR_DIAG_RXCLK_CTRL},
};

static struct cdns_reg_pairs sgmii_100_no_ssc_rx_ln_regs[] = {
	{0x091D, RX_PSC_A0},
	{0x0900, RX_PSC_A2},
	{0x0100, RX_PSC_A3},
	{0x03C7, RX_REE_GCSM1_EQENM_PH1},
	{0x01C7, RX_REE_GCSM1_EQENM_PH2},
	{0x0000, RX_DIAG_DFE_CTRL},
	{0x0019, RX_REE_TAP1_CLIP},
	{0x0019, RX_REE_TAP2TON_CLIP},
	{0x0098, RX_DIAG_NQST_CTRL},
	{0x0C01, RX_DIAG_DFE_AMP_TUNE_2},
	{0x0000, RX_DIAG_DFE_AMP_TUNE_3},
	{0x0000, RX_DIAG_PI_CAP},
	{0x0010, RX_DIAG_PI_RATE},
	{0x0001, RX_DIAG_ACYA},
	{0x018C, RX_CDRLF_CNFG},
};

static struct cdns_torrent_vals sgmii_100_no_ssc_cmn_vals = {
	.reg_pairs = sgmii_100_no_ssc_cmn_regs,
	.num_regs = ARRAY_SIZE(sgmii_100_no_ssc_cmn_regs),
};

static struct cdns_torrent_vals sgmii_100_no_ssc_tx_ln_vals = {
	.reg_pairs = sgmii_100_no_ssc_tx_ln_regs,
	.num_regs = ARRAY_SIZE(sgmii_100_no_ssc_tx_ln_regs),
};

static struct cdns_torrent_vals ti_sgmii_100_no_ssc_tx_ln_vals = {
	.reg_pairs = ti_sgmii_100_no_ssc_tx_ln_regs,
	.num_regs = ARRAY_SIZE(ti_sgmii_100_no_ssc_tx_ln_regs),
};

static struct cdns_torrent_vals sgmii_100_no_ssc_rx_ln_vals = {
	.reg_pairs = sgmii_100_no_ssc_rx_ln_regs,
	.num_regs = ARRAY_SIZE(sgmii_100_no_ssc_rx_ln_regs),
};

/* SGMII 100 MHz Ref clk, internal SSC */
static struct cdns_reg_pairs sgmii_100_int_ssc_cmn_regs[] = {
	{0x0004, CMN_PLL0_DSM_DIAG_M0},
	{0x0004, CMN_PLL0_DSM_DIAG_M1},
	{0x0004, CMN_PLL1_DSM_DIAG_M0},
	{0x0509, CMN_PDIAG_PLL0_CP_PADJ_M0},
	{0x0509, CMN_PDIAG_PLL0_CP_PADJ_M1},
	{0x0509, CMN_PDIAG_PLL1_CP_PADJ_M0},
	{0x0F00, CMN_PDIAG_PLL0_CP_IADJ_M0},
	{0x0F00, CMN_PDIAG_PLL0_CP_IADJ_M1},
	{0x0F00, CMN_PDIAG_PLL1_CP_IADJ_M0},
	{0x0F08, CMN_PDIAG_PLL0_FILT_PADJ_M0},
	{0x0F08, CMN_PDIAG_PLL0_FILT_PADJ_M1},
	{0x0F08, CMN_PDIAG_PLL1_FILT_PADJ_M0},
	{0x0064, CMN_PLL0_INTDIV_M0},
	{0x0050, CMN_PLL0_INTDIV_M1},
	{0x0064, CMN_PLL1_INTDIV_M0},
	{0x0002, CMN_PLL0_FRACDIVH_M0},
	{0x0002, CMN_PLL0_FRACDIVH_M1},
	{0x0002, CMN_PLL1_FRACDIVH_M0},
	{0x0044, CMN_PLL0_HIGH_THR_M0},
	{0x0036, CMN_PLL0_HIGH_THR_M1},
	{0x0044, CMN_PLL1_HIGH_THR_M0},
	{0x0002, CMN_PDIAG_PLL0_CTRL_M0},
	{0x0002, CMN_PDIAG_PLL0_CTRL_M1},
	{0x0002, CMN_PDIAG_PLL1_CTRL_M0},
	{0x0001, CMN_PLL0_SS_CTRL1_M0},
	{0x0001, CMN_PLL0_SS_CTRL1_M1},
	{0x0001, CMN_PLL1_SS_CTRL1_M0},
	{0x011B, CMN_PLL0_SS_CTRL2_M0},
	{0x011B, CMN_PLL0_SS_CTRL2_M1},
	{0x011B, CMN_PLL1_SS_CTRL2_M0},
	{0x006E, CMN_PLL0_SS_CTRL3_M0},
	{0x0058, CMN_PLL0_SS_CTRL3_M1},
	{0x006E, CMN_PLL1_SS_CTRL3_M0},
	{0x000E, CMN_PLL0_SS_CTRL4_M0},
	{0x0012, CMN_PLL0_SS_CTRL4_M1},
	{0x000E, CMN_PLL1_SS_CTRL4_M0},
	{0x0C5E, CMN_PLL0_VCOCAL_REFTIM_START},
	{0x0C5E, CMN_PLL1_VCOCAL_REFTIM_START},
	{0x0C56, CMN_PLL0_VCOCAL_PLLCNT_START},
	{0x0C56, CMN_PLL1_VCOCAL_PLLCNT_START},
	{0x00C7, CMN_PLL0_LOCK_REFCNT_START},
	{0x00C7, CMN_PLL1_LOCK_REFCNT_START},
	{0x00C7, CMN_PLL0_LOCK_PLLCNT_START},
	{0x00C7, CMN_PLL1_LOCK_PLLCNT_START},
	{0x0005, CMN_PLL0_LOCK_PLLCNT_THR},
	{0x0005, CMN_PLL1_LOCK_PLLCNT_THR},
	{0x007F, CMN_TXPUCAL_TUNE},
	{0x007F, CMN_TXPDCAL_TUNE}
};

static struct cdns_torrent_vals sgmii_100_int_ssc_cmn_vals = {
	.reg_pairs = sgmii_100_int_ssc_cmn_regs,
	.num_regs = ARRAY_SIZE(sgmii_100_int_ssc_cmn_regs),
};

/* QSGMII 100 MHz Ref clk, no SSC */
static struct cdns_reg_pairs sl_qsgmii_100_no_ssc_cmn_regs[] = {
	{0x0028, CMN_PDIAG_PLL1_CP_PADJ_M0},
	{0x001E, CMN_PLL1_DSM_FBH_OVRD_M0},
	{0x000C, CMN_PLL1_DSM_FBL_OVRD_M0},
	{0x0003, CMN_PLL0_VCOCAL_TCTRL},
	{0x0003, CMN_PLL1_VCOCAL_TCTRL}
};

static struct cdns_torrent_vals sl_qsgmii_100_no_ssc_cmn_vals = {
	.reg_pairs = sl_qsgmii_100_no_ssc_cmn_regs,
	.num_regs = ARRAY_SIZE(sl_qsgmii_100_no_ssc_cmn_regs),
};

static struct cdns_reg_pairs qsgmii_100_no_ssc_cmn_regs[] = {
	{0x007F, CMN_TXPUCAL_TUNE},
	{0x007F, CMN_TXPDCAL_TUNE}
};

static struct cdns_reg_pairs qsgmii_100_no_ssc_tx_ln_regs[] = {
	{0x00F3, TX_PSC_A0},
	{0x04A2, TX_PSC_A2},
	{0x04A2, TX_PSC_A3},
	{0x0000, TX_TXCC_CPOST_MULT_00},
	{0x0011, TX_TXCC_MGNFS_MULT_100},
	{0x0003, DRV_DIAG_TX_DRV}
};

static struct cdns_reg_pairs ti_qsgmii_100_no_ssc_tx_ln_regs[] = {
	{0x00F3, TX_PSC_A0},
	{0x04A2, TX_PSC_A2},
	{0x04A2, TX_PSC_A3},
	{0x0000, TX_TXCC_CPOST_MULT_00},
	{0x0011, TX_TXCC_MGNFS_MULT_100},
	{0x0003, DRV_DIAG_TX_DRV},
	{0x4000, XCVR_DIAG_RXCLK_CTRL},
};

static struct cdns_reg_pairs qsgmii_100_no_ssc_rx_ln_regs[] = {
	{0x091D, RX_PSC_A0},
	{0x0900, RX_PSC_A2},
	{0x0100, RX_PSC_A3},
	{0x03C7, RX_REE_GCSM1_EQENM_PH1},
	{0x01C7, RX_REE_GCSM1_EQENM_PH2},
	{0x0000, RX_DIAG_DFE_CTRL},
	{0x0019, RX_REE_TAP1_CLIP},
	{0x0019, RX_REE_TAP2TON_CLIP},
	{0x0098, RX_DIAG_NQST_CTRL},
	{0x0C01, RX_DIAG_DFE_AMP_TUNE_2},
	{0x0000, RX_DIAG_DFE_AMP_TUNE_3},
	{0x0000, RX_DIAG_PI_CAP},
	{0x0010, RX_DIAG_PI_RATE},
	{0x0001, RX_DIAG_ACYA},
	{0x018C, RX_CDRLF_CNFG},
};

static struct cdns_torrent_vals qsgmii_100_no_ssc_cmn_vals = {
	.reg_pairs = qsgmii_100_no_ssc_cmn_regs,
	.num_regs = ARRAY_SIZE(qsgmii_100_no_ssc_cmn_regs),
};

static struct cdns_torrent_vals qsgmii_100_no_ssc_tx_ln_vals = {
	.reg_pairs = qsgmii_100_no_ssc_tx_ln_regs,
	.num_regs = ARRAY_SIZE(qsgmii_100_no_ssc_tx_ln_regs),
};

static struct cdns_torrent_vals ti_qsgmii_100_no_ssc_tx_ln_vals = {
	.reg_pairs = ti_qsgmii_100_no_ssc_tx_ln_regs,
	.num_regs = ARRAY_SIZE(ti_qsgmii_100_no_ssc_tx_ln_regs),
};

static struct cdns_torrent_vals qsgmii_100_no_ssc_rx_ln_vals = {
	.reg_pairs = qsgmii_100_no_ssc_rx_ln_regs,
	.num_regs = ARRAY_SIZE(qsgmii_100_no_ssc_rx_ln_regs),
};

/* QSGMII 100 MHz Ref clk, internal SSC */
static struct cdns_reg_pairs qsgmii_100_int_ssc_cmn_regs[] = {
	{0x0004, CMN_PLL0_DSM_DIAG_M0},
	{0x0004, CMN_PLL0_DSM_DIAG_M1},
	{0x0004, CMN_PLL1_DSM_DIAG_M0},
	{0x0509, CMN_PDIAG_PLL0_CP_PADJ_M0},
	{0x0509, CMN_PDIAG_PLL0_CP_PADJ_M1},
	{0x0509, CMN_PDIAG_PLL1_CP_PADJ_M0},
	{0x0F00, CMN_PDIAG_PLL0_CP_IADJ_M0},
	{0x0F00, CMN_PDIAG_PLL0_CP_IADJ_M1},
	{0x0F00, CMN_PDIAG_PLL1_CP_IADJ_M0},
	{0x0F08, CMN_PDIAG_PLL0_FILT_PADJ_M0},
	{0x0F08, CMN_PDIAG_PLL0_FILT_PADJ_M1},
	{0x0F08, CMN_PDIAG_PLL1_FILT_PADJ_M0},
	{0x0064, CMN_PLL0_INTDIV_M0},
	{0x0050, CMN_PLL0_INTDIV_M1},
	{0x0064, CMN_PLL1_INTDIV_M0},
	{0x0002, CMN_PLL0_FRACDIVH_M0},
	{0x0002, CMN_PLL0_FRACDIVH_M1},
	{0x0002, CMN_PLL1_FRACDIVH_M0},
	{0x0044, CMN_PLL0_HIGH_THR_M0},
	{0x0036, CMN_PLL0_HIGH_THR_M1},
	{0x0044, CMN_PLL1_HIGH_THR_M0},
	{0x0002, CMN_PDIAG_PLL0_CTRL_M0},
	{0x0002, CMN_PDIAG_PLL0_CTRL_M1},
	{0x0002, CMN_PDIAG_PLL1_CTRL_M0},
	{0x0001, CMN_PLL0_SS_CTRL1_M0},
	{0x0001, CMN_PLL0_SS_CTRL1_M1},
	{0x0001, CMN_PLL1_SS_CTRL1_M0},
	{0x011B, CMN_PLL0_SS_CTRL2_M0},
	{0x011B, CMN_PLL0_SS_CTRL2_M1},
	{0x011B, CMN_PLL1_SS_CTRL2_M0},
	{0x006E, CMN_PLL0_SS_CTRL3_M0},
	{0x0058, CMN_PLL0_SS_CTRL3_M1},
	{0x006E, CMN_PLL1_SS_CTRL3_M0},
	{0x000E, CMN_PLL0_SS_CTRL4_M0},
	{0x0012, CMN_PLL0_SS_CTRL4_M1},
	{0x000E, CMN_PLL1_SS_CTRL4_M0},
	{0x0C5E, CMN_PLL0_VCOCAL_REFTIM_START},
	{0x0C5E, CMN_PLL1_VCOCAL_REFTIM_START},
	{0x0C56, CMN_PLL0_VCOCAL_PLLCNT_START},
	{0x0C56, CMN_PLL1_VCOCAL_PLLCNT_START},
	{0x00C7, CMN_PLL0_LOCK_REFCNT_START},
	{0x00C7, CMN_PLL1_LOCK_REFCNT_START},
	{0x00C7, CMN_PLL0_LOCK_PLLCNT_START},
	{0x00C7, CMN_PLL1_LOCK_PLLCNT_START},
	{0x0005, CMN_PLL0_LOCK_PLLCNT_THR},
	{0x0005, CMN_PLL1_LOCK_PLLCNT_THR},
	{0x007F, CMN_TXPUCAL_TUNE},
	{0x007F, CMN_TXPDCAL_TUNE}
};

static struct cdns_torrent_vals qsgmii_100_int_ssc_cmn_vals = {
	.reg_pairs = qsgmii_100_int_ssc_cmn_regs,
	.num_regs = ARRAY_SIZE(qsgmii_100_int_ssc_cmn_regs),
};

/* Single SGMII/QSGMII link configuration */
static struct cdns_reg_pairs sl_sgmii_link_cmn_regs[] = {
	{0x0000, PHY_PLL_CFG},
	{0x0601, CMN_PDIAG_PLL0_CLK_SEL_M0}
};

static struct cdns_reg_pairs sl_sgmii_xcvr_diag_ln_regs[] = {
	{0x0000, XCVR_DIAG_HSCLK_SEL},
	{0x0003, XCVR_DIAG_HSCLK_DIV},
	{0x0013, XCVR_DIAG_PLLDRC_CTRL}
};

static struct cdns_torrent_vals sl_sgmii_link_cmn_vals = {
	.reg_pairs = sl_sgmii_link_cmn_regs,
	.num_regs = ARRAY_SIZE(sl_sgmii_link_cmn_regs),
};

static struct cdns_torrent_vals sl_sgmii_xcvr_diag_ln_vals = {
	.reg_pairs = sl_sgmii_xcvr_diag_ln_regs,
	.num_regs = ARRAY_SIZE(sl_sgmii_xcvr_diag_ln_regs),
};

/* Multi link PCIe, 100 MHz Ref clk, internal SSC */
static struct cdns_reg_pairs pcie_100_int_ssc_cmn_regs[] = {
	{0x0004, CMN_PLL0_DSM_DIAG_M0},
	{0x0004, CMN_PLL0_DSM_DIAG_M1},
	{0x0004, CMN_PLL1_DSM_DIAG_M0},
	{0x0509, CMN_PDIAG_PLL0_CP_PADJ_M0},
	{0x0509, CMN_PDIAG_PLL0_CP_PADJ_M1},
	{0x0509, CMN_PDIAG_PLL1_CP_PADJ_M0},
	{0x0F00, CMN_PDIAG_PLL0_CP_IADJ_M0},
	{0x0F00, CMN_PDIAG_PLL0_CP_IADJ_M1},
	{0x0F00, CMN_PDIAG_PLL1_CP_IADJ_M0},
	{0x0F08, CMN_PDIAG_PLL0_FILT_PADJ_M0},
	{0x0F08, CMN_PDIAG_PLL0_FILT_PADJ_M1},
	{0x0F08, CMN_PDIAG_PLL1_FILT_PADJ_M0},
	{0x0064, CMN_PLL0_INTDIV_M0},
	{0x0050, CMN_PLL0_INTDIV_M1},
	{0x0064, CMN_PLL1_INTDIV_M0},
	{0x0002, CMN_PLL0_FRACDIVH_M0},
	{0x0002, CMN_PLL0_FRACDIVH_M1},
	{0x0002, CMN_PLL1_FRACDIVH_M0},
	{0x0044, CMN_PLL0_HIGH_THR_M0},
	{0x0036, CMN_PLL0_HIGH_THR_M1},
	{0x0044, CMN_PLL1_HIGH_THR_M0},
	{0x0002, CMN_PDIAG_PLL0_CTRL_M0},
	{0x0002, CMN_PDIAG_PLL0_CTRL_M1},
	{0x0002, CMN_PDIAG_PLL1_CTRL_M0},
	{0x0001, CMN_PLL0_SS_CTRL1_M0},
	{0x0001, CMN_PLL0_SS_CTRL1_M1},
	{0x0001, CMN_PLL1_SS_CTRL1_M0},
	{0x011B, CMN_PLL0_SS_CTRL2_M0},
	{0x011B, CMN_PLL0_SS_CTRL2_M1},
	{0x011B, CMN_PLL1_SS_CTRL2_M0},
	{0x006E, CMN_PLL0_SS_CTRL3_M0},
	{0x0058, CMN_PLL0_SS_CTRL3_M1},
	{0x006E, CMN_PLL1_SS_CTRL3_M0},
	{0x000E, CMN_PLL0_SS_CTRL4_M0},
	{0x0012, CMN_PLL0_SS_CTRL4_M1},
	{0x000E, CMN_PLL1_SS_CTRL4_M0},
	{0x0C5E, CMN_PLL0_VCOCAL_REFTIM_START},
	{0x0C5E, CMN_PLL1_VCOCAL_REFTIM_START},
	{0x0C56, CMN_PLL0_VCOCAL_PLLCNT_START},
	{0x0C56, CMN_PLL1_VCOCAL_PLLCNT_START},
	{0x00C7, CMN_PLL0_LOCK_REFCNT_START},
	{0x00C7, CMN_PLL1_LOCK_REFCNT_START},
	{0x00C7, CMN_PLL0_LOCK_PLLCNT_START},
	{0x00C7, CMN_PLL1_LOCK_PLLCNT_START},
	{0x0005, CMN_PLL0_LOCK_PLLCNT_THR},
	{0x0005, CMN_PLL1_LOCK_PLLCNT_THR}
};

static struct cdns_torrent_vals pcie_100_int_ssc_cmn_vals = {
	.reg_pairs = pcie_100_int_ssc_cmn_regs,
	.num_regs = ARRAY_SIZE(pcie_100_int_ssc_cmn_regs),
};

/* Single link PCIe, 100 MHz Ref clk, internal SSC */
static struct cdns_reg_pairs sl_pcie_100_int_ssc_cmn_regs[] = {
	{0x0004, CMN_PLL0_DSM_DIAG_M0},
	{0x0004, CMN_PLL0_DSM_DIAG_M1},
	{0x0004, CMN_PLL1_DSM_DIAG_M0},
	{0x0509, CMN_PDIAG_PLL0_CP_PADJ_M0},
	{0x0509, CMN_PDIAG_PLL0_CP_PADJ_M1},
	{0x0509, CMN_PDIAG_PLL1_CP_PADJ_M0},
	{0x0F00, CMN_PDIAG_PLL0_CP_IADJ_M0},
	{0x0F00, CMN_PDIAG_PLL0_CP_IADJ_M1},
	{0x0F00, CMN_PDIAG_PLL1_CP_IADJ_M0},
	{0x0F08, CMN_PDIAG_PLL0_FILT_PADJ_M0},
	{0x0F08, CMN_PDIAG_PLL0_FILT_PADJ_M1},
	{0x0F08, CMN_PDIAG_PLL1_FILT_PADJ_M0},
	{0x0064, CMN_PLL0_INTDIV_M0},
	{0x0050, CMN_PLL0_INTDIV_M1},
	{0x0050, CMN_PLL1_INTDIV_M0},
	{0x0002, CMN_PLL0_FRACDIVH_M0},
	{0x0002, CMN_PLL0_FRACDIVH_M1},
	{0x0002, CMN_PLL1_FRACDIVH_M0},
	{0x0044, CMN_PLL0_HIGH_THR_M0},
	{0x0036, CMN_PLL0_HIGH_THR_M1},
	{0x0036, CMN_PLL1_HIGH_THR_M0},
	{0x0002, CMN_PDIAG_PLL0_CTRL_M0},
	{0x0002, CMN_PDIAG_PLL0_CTRL_M1},
	{0x0002, CMN_PDIAG_PLL1_CTRL_M0},
	{0x0001, CMN_PLL0_SS_CTRL1_M0},
	{0x0001, CMN_PLL0_SS_CTRL1_M1},
	{0x0001, CMN_PLL1_SS_CTRL1_M0},
	{0x011B, CMN_PLL0_SS_CTRL2_M0},
	{0x011B, CMN_PLL0_SS_CTRL2_M1},
	{0x011B, CMN_PLL1_SS_CTRL2_M0},
	{0x006E, CMN_PLL0_SS_CTRL3_M0},
	{0x0058, CMN_PLL0_SS_CTRL3_M1},
	{0x0058, CMN_PLL1_SS_CTRL3_M0},
	{0x000E, CMN_PLL0_SS_CTRL4_M0},
	{0x0012, CMN_PLL0_SS_CTRL4_M1},
	{0x0012, CMN_PLL1_SS_CTRL4_M0},
	{0x0C5E, CMN_PLL0_VCOCAL_REFTIM_START},
	{0x0C5E, CMN_PLL1_VCOCAL_REFTIM_START},
	{0x0C56, CMN_PLL0_VCOCAL_PLLCNT_START},
	{0x0C56, CMN_PLL1_VCOCAL_PLLCNT_START},
	{0x00C7, CMN_PLL0_LOCK_REFCNT_START},
	{0x00C7, CMN_PLL1_LOCK_REFCNT_START},
	{0x00C7, CMN_PLL0_LOCK_PLLCNT_START},
	{0x00C7, CMN_PLL1_LOCK_PLLCNT_START},
	{0x0005, CMN_PLL0_LOCK_PLLCNT_THR},
	{0x0005, CMN_PLL1_LOCK_PLLCNT_THR}
};

static struct cdns_torrent_vals sl_pcie_100_int_ssc_cmn_vals = {
	.reg_pairs = sl_pcie_100_int_ssc_cmn_regs,
	.num_regs = ARRAY_SIZE(sl_pcie_100_int_ssc_cmn_regs),
};

/* PCIe, 100 MHz Ref clk, no SSC & external SSC */
static struct cdns_reg_pairs pcie_100_ext_no_ssc_cmn_regs[] = {
	{0x0028, CMN_PDIAG_PLL1_CP_PADJ_M0},
	{0x001E, CMN_PLL1_DSM_FBH_OVRD_M0},
	{0x000C, CMN_PLL1_DSM_FBL_OVRD_M0}
};

static struct cdns_reg_pairs pcie_100_ext_no_ssc_rx_ln_regs[] = {
	{0x0019, RX_REE_TAP1_CLIP},
	{0x0019, RX_REE_TAP2TON_CLIP},
	{0x0001, RX_DIAG_ACYA}
};

static struct cdns_torrent_vals pcie_100_no_ssc_cmn_vals = {
	.reg_pairs = pcie_100_ext_no_ssc_cmn_regs,
	.num_regs = ARRAY_SIZE(pcie_100_ext_no_ssc_cmn_regs),
};

static struct cdns_torrent_vals pcie_100_no_ssc_rx_ln_vals = {
	.reg_pairs = pcie_100_ext_no_ssc_rx_ln_regs,
	.num_regs = ARRAY_SIZE(pcie_100_ext_no_ssc_rx_ln_regs),
};

static const struct cdns_torrent_data cdns_map_torrent = {
	.block_offset_shift = 0x2,
	.reg_offset_shift = 0x2,
	.link_cmn_vals = {
		[TYPE_PCIE] = {
			[TYPE_NONE] = {
				[NO_SSC] = NULL,
				[EXTERNAL_SSC] = NULL,
				[INTERNAL_SSC] = NULL,
			},
			[TYPE_SGMII] = {
				[NO_SSC] = &pcie_sgmii_link_cmn_vals,
				[EXTERNAL_SSC] = &pcie_sgmii_link_cmn_vals,
				[INTERNAL_SSC] = &pcie_sgmii_link_cmn_vals,
			},
			[TYPE_QSGMII] = {
				[NO_SSC] = &pcie_sgmii_link_cmn_vals,
				[EXTERNAL_SSC] = &pcie_sgmii_link_cmn_vals,
				[INTERNAL_SSC] = &pcie_sgmii_link_cmn_vals,
			},
			[TYPE_USB] = {
				[NO_SSC] = &pcie_usb_link_cmn_vals,
				[EXTERNAL_SSC] = &pcie_usb_link_cmn_vals,
				[INTERNAL_SSC] = &pcie_usb_link_cmn_vals,
			},
		},
		[TYPE_SGMII] = {
			[TYPE_NONE] = {
				[NO_SSC] = &sl_sgmii_link_cmn_vals,
			},
			[TYPE_PCIE] = {
				[NO_SSC] = &pcie_sgmii_link_cmn_vals,
				[EXTERNAL_SSC] = &pcie_sgmii_link_cmn_vals,
				[INTERNAL_SSC] = &pcie_sgmii_link_cmn_vals,
			},
			[TYPE_USB] = {
				[NO_SSC] = &usb_sgmii_link_cmn_vals,
				[EXTERNAL_SSC] = &usb_sgmii_link_cmn_vals,
				[INTERNAL_SSC] = &usb_sgmii_link_cmn_vals,
			},
		},
		[TYPE_QSGMII] = {
			[TYPE_NONE] = {
				[NO_SSC] = &sl_sgmii_link_cmn_vals,
			},
			[TYPE_PCIE] = {
				[NO_SSC] = &pcie_sgmii_link_cmn_vals,
				[EXTERNAL_SSC] = &pcie_sgmii_link_cmn_vals,
				[INTERNAL_SSC] = &pcie_sgmii_link_cmn_vals,
			},
			[TYPE_USB] = {
				[NO_SSC] = &usb_sgmii_link_cmn_vals,
				[EXTERNAL_SSC] = &usb_sgmii_link_cmn_vals,
				[INTERNAL_SSC] = &usb_sgmii_link_cmn_vals,
			},
		},
		[TYPE_USB] = {
			[TYPE_NONE] = {
				[NO_SSC] = &sl_usb_link_cmn_vals,
				[EXTERNAL_SSC] = &sl_usb_link_cmn_vals,
				[INTERNAL_SSC] = &sl_usb_link_cmn_vals,
			},
			[TYPE_PCIE] = {
				[NO_SSC] = &pcie_usb_link_cmn_vals,
				[EXTERNAL_SSC] = &pcie_usb_link_cmn_vals,
				[INTERNAL_SSC] = &pcie_usb_link_cmn_vals,
			},
			[TYPE_SGMII] = {
				[NO_SSC] = &usb_sgmii_link_cmn_vals,
				[EXTERNAL_SSC] = &usb_sgmii_link_cmn_vals,
				[INTERNAL_SSC] = &usb_sgmii_link_cmn_vals,
			},
			[TYPE_QSGMII] = {
				[NO_SSC] = &usb_sgmii_link_cmn_vals,
				[EXTERNAL_SSC] = &usb_sgmii_link_cmn_vals,
				[INTERNAL_SSC] = &usb_sgmii_link_cmn_vals,
			},
		},
	},
	.xcvr_diag_vals = {
		[TYPE_PCIE] = {
			[TYPE_NONE] = {
				[NO_SSC] = NULL,
				[EXTERNAL_SSC] = NULL,
				[INTERNAL_SSC] = NULL,
			},
			[TYPE_SGMII] = {
				[NO_SSC] = &pcie_sgmii_xcvr_diag_ln_vals,
				[EXTERNAL_SSC] = &pcie_sgmii_xcvr_diag_ln_vals,
				[INTERNAL_SSC] = &pcie_sgmii_xcvr_diag_ln_vals,
			},
			[TYPE_QSGMII] = {
				[NO_SSC] = &pcie_sgmii_xcvr_diag_ln_vals,
				[EXTERNAL_SSC] = &pcie_sgmii_xcvr_diag_ln_vals,
				[INTERNAL_SSC] = &pcie_sgmii_xcvr_diag_ln_vals,
			},
			[TYPE_USB] = {
				[NO_SSC] = &pcie_usb_xcvr_diag_ln_vals,
				[EXTERNAL_SSC] = &pcie_usb_xcvr_diag_ln_vals,
				[INTERNAL_SSC] = &pcie_usb_xcvr_diag_ln_vals,
			},
		},
		[TYPE_SGMII] = {
			[TYPE_NONE] = {
				[NO_SSC] = &sl_sgmii_xcvr_diag_ln_vals,
			},
			[TYPE_PCIE] = {
				[NO_SSC] = &sgmii_pcie_xcvr_diag_ln_vals,
				[EXTERNAL_SSC] = &sgmii_pcie_xcvr_diag_ln_vals,
				[INTERNAL_SSC] = &sgmii_pcie_xcvr_diag_ln_vals,
			},
			[TYPE_USB] = {
				[NO_SSC] = &sgmii_usb_xcvr_diag_ln_vals,
				[EXTERNAL_SSC] = &sgmii_usb_xcvr_diag_ln_vals,
				[INTERNAL_SSC] = &sgmii_usb_xcvr_diag_ln_vals,
			},
		},
		[TYPE_QSGMII] = {
			[TYPE_NONE] = {
				[NO_SSC] = &sl_sgmii_xcvr_diag_ln_vals,
			},
			[TYPE_PCIE] = {
				[NO_SSC] = &sgmii_pcie_xcvr_diag_ln_vals,
				[EXTERNAL_SSC] = &sgmii_pcie_xcvr_diag_ln_vals,
				[INTERNAL_SSC] = &sgmii_pcie_xcvr_diag_ln_vals,
			},
			[TYPE_USB] = {
				[NO_SSC] = &sgmii_usb_xcvr_diag_ln_vals,
				[EXTERNAL_SSC] = &sgmii_usb_xcvr_diag_ln_vals,
				[INTERNAL_SSC] = &sgmii_usb_xcvr_diag_ln_vals,
			},
		},
		[TYPE_USB] = {
			[TYPE_NONE] = {
				[NO_SSC] = &sl_usb_xcvr_diag_ln_vals,
				[EXTERNAL_SSC] = &sl_usb_xcvr_diag_ln_vals,
				[INTERNAL_SSC] = &sl_usb_xcvr_diag_ln_vals,
			},
			[TYPE_PCIE] = {
				[NO_SSC] = &usb_pcie_xcvr_diag_ln_vals,
				[EXTERNAL_SSC] = &usb_pcie_xcvr_diag_ln_vals,
				[INTERNAL_SSC] = &usb_pcie_xcvr_diag_ln_vals,
			},
			[TYPE_SGMII] = {
				[NO_SSC] = &usb_sgmii_xcvr_diag_ln_vals,
				[EXTERNAL_SSC] = &usb_sgmii_xcvr_diag_ln_vals,
				[INTERNAL_SSC] = &usb_sgmii_xcvr_diag_ln_vals,
			},
			[TYPE_QSGMII] = {
				[NO_SSC] = &usb_sgmii_xcvr_diag_ln_vals,
				[EXTERNAL_SSC] = &usb_sgmii_xcvr_diag_ln_vals,
				[INTERNAL_SSC] = &usb_sgmii_xcvr_diag_ln_vals,
			},
		},
	},
	.pcs_cmn_vals = {
		[TYPE_USB] = {
			[TYPE_NONE] = {
				[NO_SSC] = &usb_phy_pcs_cmn_vals,
				[EXTERNAL_SSC] = &usb_phy_pcs_cmn_vals,
				[INTERNAL_SSC] = &usb_phy_pcs_cmn_vals,
			},
			[TYPE_PCIE] = {
				[NO_SSC] = &usb_phy_pcs_cmn_vals,
				[EXTERNAL_SSC] = &usb_phy_pcs_cmn_vals,
				[INTERNAL_SSC] = &usb_phy_pcs_cmn_vals,
			},
			[TYPE_SGMII] = {
				[NO_SSC] = &usb_phy_pcs_cmn_vals,
				[EXTERNAL_SSC] = &usb_phy_pcs_cmn_vals,
				[INTERNAL_SSC] = &usb_phy_pcs_cmn_vals,
			},
			[TYPE_QSGMII] = {
				[NO_SSC] = &usb_phy_pcs_cmn_vals,
				[EXTERNAL_SSC] = &usb_phy_pcs_cmn_vals,
				[INTERNAL_SSC] = &usb_phy_pcs_cmn_vals,
			},
		},
	},
	.cmn_vals = {
		[TYPE_PCIE] = {
			[TYPE_NONE] = {
				[NO_SSC] = NULL,
				[EXTERNAL_SSC] = NULL,
				[INTERNAL_SSC] = &sl_pcie_100_int_ssc_cmn_vals,
			},
			[TYPE_SGMII] = {
				[NO_SSC] = &pcie_100_no_ssc_cmn_vals,
				[EXTERNAL_SSC] = &pcie_100_no_ssc_cmn_vals,
				[INTERNAL_SSC] = &pcie_100_int_ssc_cmn_vals,
			},
			[TYPE_QSGMII] = {
				[NO_SSC] = &pcie_100_no_ssc_cmn_vals,
				[EXTERNAL_SSC] = &pcie_100_no_ssc_cmn_vals,
				[INTERNAL_SSC] = &pcie_100_int_ssc_cmn_vals,
			},
			[TYPE_USB] = {
				[NO_SSC] = &pcie_100_no_ssc_cmn_vals,
				[EXTERNAL_SSC] = &pcie_100_no_ssc_cmn_vals,
				[INTERNAL_SSC] = &pcie_100_int_ssc_cmn_vals,
			},
		},
		[TYPE_SGMII] = {
			[TYPE_NONE] = {
				[NO_SSC] = &sl_sgmii_100_no_ssc_cmn_vals,
			},
			[TYPE_PCIE] = {
				[NO_SSC] = &sgmii_100_no_ssc_cmn_vals,
				[EXTERNAL_SSC] = &sgmii_100_no_ssc_cmn_vals,
				[INTERNAL_SSC] = &sgmii_100_int_ssc_cmn_vals,
			},
			[TYPE_USB] = {
				[NO_SSC] = &sgmii_100_no_ssc_cmn_vals,
				[EXTERNAL_SSC] = &sgmii_100_no_ssc_cmn_vals,
				[INTERNAL_SSC] = &sgmii_100_no_ssc_cmn_vals,
			},
		},
		[TYPE_QSGMII] = {
			[TYPE_NONE] = {
				[NO_SSC] = &sl_qsgmii_100_no_ssc_cmn_vals,
			},
			[TYPE_PCIE] = {
				[NO_SSC] = &qsgmii_100_no_ssc_cmn_vals,
				[EXTERNAL_SSC] = &qsgmii_100_no_ssc_cmn_vals,
				[INTERNAL_SSC] = &qsgmii_100_int_ssc_cmn_vals,
			},
			[TYPE_USB] = {
				[NO_SSC] = &qsgmii_100_no_ssc_cmn_vals,
				[EXTERNAL_SSC] = &qsgmii_100_no_ssc_cmn_vals,
				[INTERNAL_SSC] = &qsgmii_100_no_ssc_cmn_vals,
			},
		},
		[TYPE_USB] = {
			[TYPE_NONE] = {
				[NO_SSC] = &sl_usb_100_no_ssc_cmn_vals,
				[EXTERNAL_SSC] = &sl_usb_100_no_ssc_cmn_vals,
				[INTERNAL_SSC] = &sl_usb_100_int_ssc_cmn_vals,
			},
			[TYPE_PCIE] = {
				[NO_SSC] = &usb_100_no_ssc_cmn_vals,
				[EXTERNAL_SSC] = &usb_100_no_ssc_cmn_vals,
				[INTERNAL_SSC] = &usb_100_int_ssc_cmn_vals,
			},
			[TYPE_SGMII] = {
				[NO_SSC] = &sl_usb_100_no_ssc_cmn_vals,
				[EXTERNAL_SSC] = &sl_usb_100_no_ssc_cmn_vals,
				[INTERNAL_SSC] = &sl_usb_100_int_ssc_cmn_vals,
			},
			[TYPE_QSGMII] = {
				[NO_SSC] = &sl_usb_100_no_ssc_cmn_vals,
				[EXTERNAL_SSC] = &sl_usb_100_no_ssc_cmn_vals,
				[INTERNAL_SSC] = &sl_usb_100_int_ssc_cmn_vals,
			},
		},
	},
	.tx_ln_vals = {
		[TYPE_PCIE] = {
			[TYPE_NONE] = {
				[NO_SSC] = NULL,
				[EXTERNAL_SSC] = NULL,
				[INTERNAL_SSC] = NULL,
			},
			[TYPE_SGMII] = {
				[NO_SSC] = NULL,
				[EXTERNAL_SSC] = NULL,
				[INTERNAL_SSC] = NULL,
			},
			[TYPE_QSGMII] = {
				[NO_SSC] = NULL,
				[EXTERNAL_SSC] = NULL,
				[INTERNAL_SSC] = NULL,
			},
			[TYPE_USB] = {
				[NO_SSC] = NULL,
				[EXTERNAL_SSC] = NULL,
				[INTERNAL_SSC] = NULL,
			},
		},
		[TYPE_SGMII] = {
			[TYPE_NONE] = {
				[NO_SSC] = &sgmii_100_no_ssc_tx_ln_vals,
			},
			[TYPE_PCIE] = {
				[NO_SSC] = &sgmii_100_no_ssc_tx_ln_vals,
				[EXTERNAL_SSC] = &sgmii_100_no_ssc_tx_ln_vals,
				[INTERNAL_SSC] = &sgmii_100_no_ssc_tx_ln_vals,
			},
			[TYPE_USB] = {
				[NO_SSC] = &sgmii_100_no_ssc_tx_ln_vals,
				[EXTERNAL_SSC] = &sgmii_100_no_ssc_tx_ln_vals,
				[INTERNAL_SSC] = &sgmii_100_no_ssc_tx_ln_vals,
			},
		},
		[TYPE_QSGMII] = {
			[TYPE_NONE] = {
				[NO_SSC] = &qsgmii_100_no_ssc_tx_ln_vals,
			},
			[TYPE_PCIE] = {
				[NO_SSC] = &qsgmii_100_no_ssc_tx_ln_vals,
				[EXTERNAL_SSC] = &qsgmii_100_no_ssc_tx_ln_vals,
				[INTERNAL_SSC] = &qsgmii_100_no_ssc_tx_ln_vals,
			},
			[TYPE_USB] = {
				[NO_SSC] = &qsgmii_100_no_ssc_tx_ln_vals,
				[EXTERNAL_SSC] = &qsgmii_100_no_ssc_tx_ln_vals,
				[INTERNAL_SSC] = &qsgmii_100_no_ssc_tx_ln_vals,
			},
		},
		[TYPE_USB] = {
			[TYPE_NONE] = {
				[NO_SSC] = &usb_100_no_ssc_tx_ln_vals,
				[EXTERNAL_SSC] = &usb_100_no_ssc_tx_ln_vals,
				[INTERNAL_SSC] = &usb_100_no_ssc_tx_ln_vals,
			},
			[TYPE_PCIE] = {
				[NO_SSC] = &usb_100_no_ssc_tx_ln_vals,
				[EXTERNAL_SSC] = &usb_100_no_ssc_tx_ln_vals,
				[INTERNAL_SSC] = &usb_100_no_ssc_tx_ln_vals,
			},
			[TYPE_SGMII] = {
				[NO_SSC] = &usb_100_no_ssc_tx_ln_vals,
				[EXTERNAL_SSC] = &usb_100_no_ssc_tx_ln_vals,
				[INTERNAL_SSC] = &usb_100_no_ssc_tx_ln_vals,
			},
			[TYPE_QSGMII] = {
				[NO_SSC] = &usb_100_no_ssc_tx_ln_vals,
				[EXTERNAL_SSC] = &usb_100_no_ssc_tx_ln_vals,
				[INTERNAL_SSC] = &usb_100_no_ssc_tx_ln_vals,
			},
		},
	},
	.rx_ln_vals = {
		[TYPE_PCIE] = {
			[TYPE_NONE] = {
				[NO_SSC] = &pcie_100_no_ssc_rx_ln_vals,
				[EXTERNAL_SSC] = &pcie_100_no_ssc_rx_ln_vals,
				[INTERNAL_SSC] = &pcie_100_no_ssc_rx_ln_vals,
			},
			[TYPE_SGMII] = {
				[NO_SSC] = &pcie_100_no_ssc_rx_ln_vals,
				[EXTERNAL_SSC] = &pcie_100_no_ssc_rx_ln_vals,
				[INTERNAL_SSC] = &pcie_100_no_ssc_rx_ln_vals,
			},
			[TYPE_QSGMII] = {
				[NO_SSC] = &pcie_100_no_ssc_rx_ln_vals,
				[EXTERNAL_SSC] = &pcie_100_no_ssc_rx_ln_vals,
				[INTERNAL_SSC] = &pcie_100_no_ssc_rx_ln_vals,
			},
			[TYPE_USB] = {
				[NO_SSC] = &pcie_100_no_ssc_rx_ln_vals,
				[EXTERNAL_SSC] = &pcie_100_no_ssc_rx_ln_vals,
				[INTERNAL_SSC] = &pcie_100_no_ssc_rx_ln_vals,
			},
		},
		[TYPE_SGMII] = {
			[TYPE_NONE] = {
				[NO_SSC] = &sgmii_100_no_ssc_rx_ln_vals,
			},
			[TYPE_PCIE] = {
				[NO_SSC] = &sgmii_100_no_ssc_rx_ln_vals,
				[EXTERNAL_SSC] = &sgmii_100_no_ssc_rx_ln_vals,
				[INTERNAL_SSC] = &sgmii_100_no_ssc_rx_ln_vals,
			},
			[TYPE_USB] = {
				[NO_SSC] = &sgmii_100_no_ssc_rx_ln_vals,
				[EXTERNAL_SSC] = &sgmii_100_no_ssc_rx_ln_vals,
				[INTERNAL_SSC] = &sgmii_100_no_ssc_rx_ln_vals,
			},
		},
		[TYPE_QSGMII] = {
			[TYPE_NONE] = {
				[NO_SSC] = &qsgmii_100_no_ssc_rx_ln_vals,
			},
			[TYPE_PCIE] = {
				[NO_SSC] = &qsgmii_100_no_ssc_rx_ln_vals,
				[EXTERNAL_SSC] = &qsgmii_100_no_ssc_rx_ln_vals,
				[INTERNAL_SSC] = &qsgmii_100_no_ssc_rx_ln_vals,
			},
			[TYPE_USB] = {
				[NO_SSC] = &qsgmii_100_no_ssc_rx_ln_vals,
				[EXTERNAL_SSC] = &qsgmii_100_no_ssc_rx_ln_vals,
				[INTERNAL_SSC] = &qsgmii_100_no_ssc_rx_ln_vals,
			},
		},
		[TYPE_USB] = {
			[TYPE_NONE] = {
				[NO_SSC] = &usb_100_no_ssc_rx_ln_vals,
				[EXTERNAL_SSC] = &usb_100_no_ssc_rx_ln_vals,
				[INTERNAL_SSC] = &usb_100_no_ssc_rx_ln_vals,
			},
			[TYPE_PCIE] = {
				[NO_SSC] = &usb_100_no_ssc_rx_ln_vals,
				[EXTERNAL_SSC] = &usb_100_no_ssc_rx_ln_vals,
				[INTERNAL_SSC] = &usb_100_no_ssc_rx_ln_vals,
			},
			[TYPE_SGMII] = {
				[NO_SSC] = &usb_100_no_ssc_rx_ln_vals,
				[EXTERNAL_SSC] = &usb_100_no_ssc_rx_ln_vals,
				[INTERNAL_SSC] = &usb_100_no_ssc_rx_ln_vals,
			},
			[TYPE_QSGMII] = {
				[NO_SSC] = &usb_100_no_ssc_rx_ln_vals,
				[EXTERNAL_SSC] = &usb_100_no_ssc_rx_ln_vals,
				[INTERNAL_SSC] = &usb_100_no_ssc_rx_ln_vals,
			},
		},
	},
};

static const struct cdns_torrent_data ti_j721e_map_torrent = {
	.block_offset_shift = 0x0,
	.reg_offset_shift = 0x1,
	.link_cmn_vals = {
		[TYPE_PCIE] = {
			[TYPE_NONE] = {
				[NO_SSC] = NULL,
				[EXTERNAL_SSC] = NULL,
				[INTERNAL_SSC] = NULL,
			},
			[TYPE_SGMII] = {
				[NO_SSC] = &pcie_sgmii_link_cmn_vals,
				[EXTERNAL_SSC] = &pcie_sgmii_link_cmn_vals,
				[INTERNAL_SSC] = &pcie_sgmii_link_cmn_vals,
			},
			[TYPE_QSGMII] = {
				[NO_SSC] = &pcie_sgmii_link_cmn_vals,
				[EXTERNAL_SSC] = &pcie_sgmii_link_cmn_vals,
				[INTERNAL_SSC] = &pcie_sgmii_link_cmn_vals,
			},
			[TYPE_USB] = {
				[NO_SSC] = &pcie_usb_link_cmn_vals,
				[EXTERNAL_SSC] = &pcie_usb_link_cmn_vals,
				[INTERNAL_SSC] = &pcie_usb_link_cmn_vals,
			},
		},
		[TYPE_SGMII] = {
			[TYPE_NONE] = {
				[NO_SSC] = &sl_sgmii_link_cmn_vals,
			},
			[TYPE_PCIE] = {
				[NO_SSC] = &pcie_sgmii_link_cmn_vals,
				[EXTERNAL_SSC] = &pcie_sgmii_link_cmn_vals,
				[INTERNAL_SSC] = &pcie_sgmii_link_cmn_vals,
			},
			[TYPE_USB] = {
				[NO_SSC] = &usb_sgmii_link_cmn_vals,
				[EXTERNAL_SSC] = &usb_sgmii_link_cmn_vals,
				[INTERNAL_SSC] = &usb_sgmii_link_cmn_vals,
			},
		},
		[TYPE_QSGMII] = {
			[TYPE_NONE] = {
				[NO_SSC] = &sl_sgmii_link_cmn_vals,
			},
			[TYPE_PCIE] = {
				[NO_SSC] = &pcie_sgmii_link_cmn_vals,
				[EXTERNAL_SSC] = &pcie_sgmii_link_cmn_vals,
				[INTERNAL_SSC] = &pcie_sgmii_link_cmn_vals,
			},
			[TYPE_USB] = {
				[NO_SSC] = &usb_sgmii_link_cmn_vals,
				[EXTERNAL_SSC] = &usb_sgmii_link_cmn_vals,
				[INTERNAL_SSC] = &usb_sgmii_link_cmn_vals,
			},
		},
		[TYPE_USB] = {
			[TYPE_NONE] = {
				[NO_SSC] = &sl_usb_link_cmn_vals,
				[EXTERNAL_SSC] = &sl_usb_link_cmn_vals,
				[INTERNAL_SSC] = &sl_usb_link_cmn_vals,
			},
			[TYPE_PCIE] = {
				[NO_SSC] = &pcie_usb_link_cmn_vals,
				[EXTERNAL_SSC] = &pcie_usb_link_cmn_vals,
				[INTERNAL_SSC] = &pcie_usb_link_cmn_vals,
			},
			[TYPE_SGMII] = {
				[NO_SSC] = &usb_sgmii_link_cmn_vals,
				[EXTERNAL_SSC] = &usb_sgmii_link_cmn_vals,
				[INTERNAL_SSC] = &usb_sgmii_link_cmn_vals,
			},
			[TYPE_QSGMII] = {
				[NO_SSC] = &usb_sgmii_link_cmn_vals,
				[EXTERNAL_SSC] = &usb_sgmii_link_cmn_vals,
				[INTERNAL_SSC] = &usb_sgmii_link_cmn_vals,
			},
		},
	},
	.xcvr_diag_vals = {
		[TYPE_PCIE] = {
			[TYPE_NONE] = {
				[NO_SSC] = NULL,
				[EXTERNAL_SSC] = NULL,
				[INTERNAL_SSC] = NULL,
			},
			[TYPE_SGMII] = {
				[NO_SSC] = &pcie_sgmii_xcvr_diag_ln_vals,
				[EXTERNAL_SSC] = &pcie_sgmii_xcvr_diag_ln_vals,
				[INTERNAL_SSC] = &pcie_sgmii_xcvr_diag_ln_vals,
			},
			[TYPE_QSGMII] = {
				[NO_SSC] = &pcie_sgmii_xcvr_diag_ln_vals,
				[EXTERNAL_SSC] = &pcie_sgmii_xcvr_diag_ln_vals,
				[INTERNAL_SSC] = &pcie_sgmii_xcvr_diag_ln_vals,
			},
			[TYPE_USB] = {
				[NO_SSC] = &pcie_usb_xcvr_diag_ln_vals,
				[EXTERNAL_SSC] = &pcie_usb_xcvr_diag_ln_vals,
				[INTERNAL_SSC] = &pcie_usb_xcvr_diag_ln_vals,
			},
		},
		[TYPE_SGMII] = {
			[TYPE_NONE] = {
				[NO_SSC] = &sl_sgmii_xcvr_diag_ln_vals,
			},
			[TYPE_PCIE] = {
				[NO_SSC] = &sgmii_pcie_xcvr_diag_ln_vals,
				[EXTERNAL_SSC] = &sgmii_pcie_xcvr_diag_ln_vals,
				[INTERNAL_SSC] = &sgmii_pcie_xcvr_diag_ln_vals,
			},
			[TYPE_USB] = {
				[NO_SSC] = &sgmii_usb_xcvr_diag_ln_vals,
				[EXTERNAL_SSC] = &sgmii_usb_xcvr_diag_ln_vals,
				[INTERNAL_SSC] = &sgmii_usb_xcvr_diag_ln_vals,
			},
		},
		[TYPE_QSGMII] = {
			[TYPE_NONE] = {
				[NO_SSC] = &sl_sgmii_xcvr_diag_ln_vals,
			},
			[TYPE_PCIE] = {
				[NO_SSC] = &sgmii_pcie_xcvr_diag_ln_vals,
				[EXTERNAL_SSC] = &sgmii_pcie_xcvr_diag_ln_vals,
				[INTERNAL_SSC] = &sgmii_pcie_xcvr_diag_ln_vals,
			},
			[TYPE_USB] = {
				[NO_SSC] = &sgmii_usb_xcvr_diag_ln_vals,
				[EXTERNAL_SSC] = &sgmii_usb_xcvr_diag_ln_vals,
				[INTERNAL_SSC] = &sgmii_usb_xcvr_diag_ln_vals,
			},
		},
		[TYPE_USB] = {
			[TYPE_NONE] = {
				[NO_SSC] = &sl_usb_xcvr_diag_ln_vals,
				[EXTERNAL_SSC] = &sl_usb_xcvr_diag_ln_vals,
				[INTERNAL_SSC] = &sl_usb_xcvr_diag_ln_vals,
			},
			[TYPE_PCIE] = {
				[NO_SSC] = &usb_pcie_xcvr_diag_ln_vals,
				[EXTERNAL_SSC] = &usb_pcie_xcvr_diag_ln_vals,
				[INTERNAL_SSC] = &usb_pcie_xcvr_diag_ln_vals,
			},
			[TYPE_SGMII] = {
				[NO_SSC] = &usb_sgmii_xcvr_diag_ln_vals,
				[EXTERNAL_SSC] = &usb_sgmii_xcvr_diag_ln_vals,
				[INTERNAL_SSC] = &usb_sgmii_xcvr_diag_ln_vals,
			},
			[TYPE_QSGMII] = {
				[NO_SSC] = &usb_sgmii_xcvr_diag_ln_vals,
				[EXTERNAL_SSC] = &usb_sgmii_xcvr_diag_ln_vals,
				[INTERNAL_SSC] = &usb_sgmii_xcvr_diag_ln_vals,
			},
		},
	},
	.pcs_cmn_vals = {
		[TYPE_USB] = {
			[TYPE_NONE] = {
				[NO_SSC] = &usb_phy_pcs_cmn_vals,
				[EXTERNAL_SSC] = &usb_phy_pcs_cmn_vals,
				[INTERNAL_SSC] = &usb_phy_pcs_cmn_vals,
			},
			[TYPE_PCIE] = {
				[NO_SSC] = &usb_phy_pcs_cmn_vals,
				[EXTERNAL_SSC] = &usb_phy_pcs_cmn_vals,
				[INTERNAL_SSC] = &usb_phy_pcs_cmn_vals,
			},
			[TYPE_SGMII] = {
				[NO_SSC] = &usb_phy_pcs_cmn_vals,
				[EXTERNAL_SSC] = &usb_phy_pcs_cmn_vals,
				[INTERNAL_SSC] = &usb_phy_pcs_cmn_vals,
			},
			[TYPE_QSGMII] = {
				[NO_SSC] = &usb_phy_pcs_cmn_vals,
				[EXTERNAL_SSC] = &usb_phy_pcs_cmn_vals,
				[INTERNAL_SSC] = &usb_phy_pcs_cmn_vals,
			},
		},
	},
	.cmn_vals = {
		[TYPE_PCIE] = {
			[TYPE_NONE] = {
				[NO_SSC] = NULL,
				[EXTERNAL_SSC] = NULL,
				[INTERNAL_SSC] = &sl_pcie_100_int_ssc_cmn_vals,
			},
			[TYPE_SGMII] = {
				[NO_SSC] = &pcie_100_no_ssc_cmn_vals,
				[EXTERNAL_SSC] = &pcie_100_no_ssc_cmn_vals,
				[INTERNAL_SSC] = &pcie_100_int_ssc_cmn_vals,
			},
			[TYPE_QSGMII] = {
				[NO_SSC] = &pcie_100_no_ssc_cmn_vals,
				[EXTERNAL_SSC] = &pcie_100_no_ssc_cmn_vals,
				[INTERNAL_SSC] = &pcie_100_int_ssc_cmn_vals,
			},
			[TYPE_USB] = {
				[NO_SSC] = &pcie_100_no_ssc_cmn_vals,
				[EXTERNAL_SSC] = &pcie_100_no_ssc_cmn_vals,
				[INTERNAL_SSC] = &pcie_100_int_ssc_cmn_vals,
			},
		},
		[TYPE_SGMII] = {
			[TYPE_NONE] = {
				[NO_SSC] = &sl_sgmii_100_no_ssc_cmn_vals,
			},
			[TYPE_PCIE] = {
				[NO_SSC] = &sgmii_100_no_ssc_cmn_vals,
				[EXTERNAL_SSC] = &sgmii_100_no_ssc_cmn_vals,
				[INTERNAL_SSC] = &sgmii_100_int_ssc_cmn_vals,
			},
			[TYPE_USB] = {
				[NO_SSC] = &sgmii_100_no_ssc_cmn_vals,
				[EXTERNAL_SSC] = &sgmii_100_no_ssc_cmn_vals,
				[INTERNAL_SSC] = &sgmii_100_no_ssc_cmn_vals,
			},
		},
		[TYPE_QSGMII] = {
			[TYPE_NONE] = {
				[NO_SSC] = &sl_qsgmii_100_no_ssc_cmn_vals,
			},
			[TYPE_PCIE] = {
				[NO_SSC] = &qsgmii_100_no_ssc_cmn_vals,
				[EXTERNAL_SSC] = &qsgmii_100_no_ssc_cmn_vals,
				[INTERNAL_SSC] = &qsgmii_100_int_ssc_cmn_vals,
			},
			[TYPE_USB] = {
				[NO_SSC] = &qsgmii_100_no_ssc_cmn_vals,
				[EXTERNAL_SSC] = &qsgmii_100_no_ssc_cmn_vals,
				[INTERNAL_SSC] = &qsgmii_100_no_ssc_cmn_vals,
			},
		},
		[TYPE_USB] = {
			[TYPE_NONE] = {
				[NO_SSC] = &sl_usb_100_no_ssc_cmn_vals,
				[EXTERNAL_SSC] = &sl_usb_100_no_ssc_cmn_vals,
				[INTERNAL_SSC] = &sl_usb_100_int_ssc_cmn_vals,
			},
			[TYPE_PCIE] = {
				[NO_SSC] = &usb_100_no_ssc_cmn_vals,
				[EXTERNAL_SSC] = &usb_100_no_ssc_cmn_vals,
				[INTERNAL_SSC] = &usb_100_int_ssc_cmn_vals,
			},
			[TYPE_SGMII] = {
				[NO_SSC] = &sl_usb_100_no_ssc_cmn_vals,
				[EXTERNAL_SSC] = &sl_usb_100_no_ssc_cmn_vals,
				[INTERNAL_SSC] = &sl_usb_100_int_ssc_cmn_vals,
			},
			[TYPE_QSGMII] = {
				[NO_SSC] = &sl_usb_100_no_ssc_cmn_vals,
				[EXTERNAL_SSC] = &sl_usb_100_no_ssc_cmn_vals,
				[INTERNAL_SSC] = &sl_usb_100_int_ssc_cmn_vals,
			},
		},
	},
	.tx_ln_vals = {
		[TYPE_PCIE] = {
			[TYPE_NONE] = {
				[NO_SSC] = NULL,
				[EXTERNAL_SSC] = NULL,
				[INTERNAL_SSC] = NULL,
			},
			[TYPE_SGMII] = {
				[NO_SSC] = NULL,
				[EXTERNAL_SSC] = NULL,
				[INTERNAL_SSC] = NULL,
			},
			[TYPE_QSGMII] = {
				[NO_SSC] = NULL,
				[EXTERNAL_SSC] = NULL,
				[INTERNAL_SSC] = NULL,
			},
			[TYPE_USB] = {
				[NO_SSC] = NULL,
				[EXTERNAL_SSC] = NULL,
				[INTERNAL_SSC] = NULL,
			},
		},
		[TYPE_SGMII] = {
			[TYPE_NONE] = {
				[NO_SSC] = &ti_sgmii_100_no_ssc_tx_ln_vals,
			},
			[TYPE_PCIE] = {
				[NO_SSC] = &ti_sgmii_100_no_ssc_tx_ln_vals,
				[EXTERNAL_SSC] = &ti_sgmii_100_no_ssc_tx_ln_vals,
				[INTERNAL_SSC] = &ti_sgmii_100_no_ssc_tx_ln_vals,
			},
			[TYPE_USB] = {
				[NO_SSC] = &ti_sgmii_100_no_ssc_tx_ln_vals,
				[EXTERNAL_SSC] = &ti_sgmii_100_no_ssc_tx_ln_vals,
				[INTERNAL_SSC] = &ti_sgmii_100_no_ssc_tx_ln_vals,
			},
		},
		[TYPE_QSGMII] = {
			[TYPE_NONE] = {
				[NO_SSC] = &ti_qsgmii_100_no_ssc_tx_ln_vals,
			},
			[TYPE_PCIE] = {
				[NO_SSC] = &ti_qsgmii_100_no_ssc_tx_ln_vals,
				[EXTERNAL_SSC] = &ti_qsgmii_100_no_ssc_tx_ln_vals,
				[INTERNAL_SSC] = &ti_qsgmii_100_no_ssc_tx_ln_vals,
			},
			[TYPE_USB] = {
				[NO_SSC] = &ti_qsgmii_100_no_ssc_tx_ln_vals,
				[EXTERNAL_SSC] = &ti_qsgmii_100_no_ssc_tx_ln_vals,
				[INTERNAL_SSC] = &ti_qsgmii_100_no_ssc_tx_ln_vals,
			},
		},
		[TYPE_USB] = {
			[TYPE_NONE] = {
				[NO_SSC] = &usb_100_no_ssc_tx_ln_vals,
				[EXTERNAL_SSC] = &usb_100_no_ssc_tx_ln_vals,
				[INTERNAL_SSC] = &usb_100_no_ssc_tx_ln_vals,
			},
			[TYPE_PCIE] = {
				[NO_SSC] = &usb_100_no_ssc_tx_ln_vals,
				[EXTERNAL_SSC] = &usb_100_no_ssc_tx_ln_vals,
				[INTERNAL_SSC] = &usb_100_no_ssc_tx_ln_vals,
			},
			[TYPE_SGMII] = {
				[NO_SSC] = &usb_100_no_ssc_tx_ln_vals,
				[EXTERNAL_SSC] = &usb_100_no_ssc_tx_ln_vals,
				[INTERNAL_SSC] = &usb_100_no_ssc_tx_ln_vals,
			},
			[TYPE_QSGMII] = {
				[NO_SSC] = &usb_100_no_ssc_tx_ln_vals,
				[EXTERNAL_SSC] = &usb_100_no_ssc_tx_ln_vals,
				[INTERNAL_SSC] = &usb_100_no_ssc_tx_ln_vals,
			},
		},
	},
	.rx_ln_vals = {
		[TYPE_PCIE] = {
			[TYPE_NONE] = {
				[NO_SSC] = &pcie_100_no_ssc_rx_ln_vals,
				[EXTERNAL_SSC] = &pcie_100_no_ssc_rx_ln_vals,
				[INTERNAL_SSC] = &pcie_100_no_ssc_rx_ln_vals,
			},
			[TYPE_SGMII] = {
				[NO_SSC] = &pcie_100_no_ssc_rx_ln_vals,
				[EXTERNAL_SSC] = &pcie_100_no_ssc_rx_ln_vals,
				[INTERNAL_SSC] = &pcie_100_no_ssc_rx_ln_vals,
			},
			[TYPE_QSGMII] = {
				[NO_SSC] = &pcie_100_no_ssc_rx_ln_vals,
				[EXTERNAL_SSC] = &pcie_100_no_ssc_rx_ln_vals,
				[INTERNAL_SSC] = &pcie_100_no_ssc_rx_ln_vals,
			},
			[TYPE_USB] = {
				[NO_SSC] = &pcie_100_no_ssc_rx_ln_vals,
				[EXTERNAL_SSC] = &pcie_100_no_ssc_rx_ln_vals,
				[INTERNAL_SSC] = &pcie_100_no_ssc_rx_ln_vals,
			},
		},
		[TYPE_SGMII] = {
			[TYPE_NONE] = {
				[NO_SSC] = &sgmii_100_no_ssc_rx_ln_vals,
			},
			[TYPE_PCIE] = {
				[NO_SSC] = &sgmii_100_no_ssc_rx_ln_vals,
				[EXTERNAL_SSC] = &sgmii_100_no_ssc_rx_ln_vals,
				[INTERNAL_SSC] = &sgmii_100_no_ssc_rx_ln_vals,
			},
			[TYPE_USB] = {
				[NO_SSC] = &sgmii_100_no_ssc_rx_ln_vals,
				[EXTERNAL_SSC] = &sgmii_100_no_ssc_rx_ln_vals,
				[INTERNAL_SSC] = &sgmii_100_no_ssc_rx_ln_vals,
			},
		},
		[TYPE_QSGMII] = {
			[TYPE_NONE] = {
				[NO_SSC] = &qsgmii_100_no_ssc_rx_ln_vals,
			},
			[TYPE_PCIE] = {
				[NO_SSC] = &qsgmii_100_no_ssc_rx_ln_vals,
				[EXTERNAL_SSC] = &qsgmii_100_no_ssc_rx_ln_vals,
				[INTERNAL_SSC] = &qsgmii_100_no_ssc_rx_ln_vals,
			},
			[TYPE_USB] = {
				[NO_SSC] = &qsgmii_100_no_ssc_rx_ln_vals,
				[EXTERNAL_SSC] = &qsgmii_100_no_ssc_rx_ln_vals,
				[INTERNAL_SSC] = &qsgmii_100_no_ssc_rx_ln_vals,
			},
		},
		[TYPE_USB] = {
			[TYPE_NONE] = {
				[NO_SSC] = &usb_100_no_ssc_rx_ln_vals,
				[EXTERNAL_SSC] = &usb_100_no_ssc_rx_ln_vals,
				[INTERNAL_SSC] = &usb_100_no_ssc_rx_ln_vals,
			},
			[TYPE_PCIE] = {
				[NO_SSC] = &usb_100_no_ssc_rx_ln_vals,
				[EXTERNAL_SSC] = &usb_100_no_ssc_rx_ln_vals,
				[INTERNAL_SSC] = &usb_100_no_ssc_rx_ln_vals,
			},
			[TYPE_SGMII] = {
				[NO_SSC] = &usb_100_no_ssc_rx_ln_vals,
				[EXTERNAL_SSC] = &usb_100_no_ssc_rx_ln_vals,
				[INTERNAL_SSC] = &usb_100_no_ssc_rx_ln_vals,
			},
			[TYPE_QSGMII] = {
				[NO_SSC] = &usb_100_no_ssc_rx_ln_vals,
				[EXTERNAL_SSC] = &usb_100_no_ssc_rx_ln_vals,
				[INTERNAL_SSC] = &usb_100_no_ssc_rx_ln_vals,
			},
		},
	},
};

static int cdns_torrent_phy_reset(struct phy *gphy)
{
	struct cdns_torrent_phy *sp = dev_get_priv(gphy->dev);

	reset_control_assert(sp->phy_rst);
	reset_control_deassert(sp->phy_rst);
	return 0;
}

static const struct udevice_id cdns_torrent_id_table[] = {
	{
		.compatible = "cdns,torrent-phy",
		.data = (ulong)&cdns_map_torrent,
	},
	{
		.compatible = "ti,j721e-serdes-10g",
		.data = (ulong)&ti_j721e_map_torrent,
	},
	{}
};

static const struct phy_ops cdns_torrent_phy_ops = {
	.init		= cdns_torrent_phy_init,
	.power_on	= cdns_torrent_phy_on,
	.power_off	= cdns_torrent_phy_off,
	.reset		= cdns_torrent_phy_reset,
};

U_BOOT_DRIVER(torrent_phy_provider) = {
	.name		= "cdns,torrent",
	.id		= UCLASS_PHY,
	.of_match	= cdns_torrent_id_table,
	.probe		= cdns_torrent_phy_probe,
	.remove		= cdns_torrent_phy_remove,
	.ops		= &cdns_torrent_phy_ops,
	.priv_auto	= sizeof(struct cdns_torrent_phy),
};
