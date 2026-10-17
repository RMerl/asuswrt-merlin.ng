// SPDX-License-Identifier:    GPL-2.0
/*
 * Copyright (C) 2018 Marvell International Ltd.
 */

#include <config.h>
#include <dm.h>
#include <errno.h>
#include <fdt_support.h>
#include <malloc.h>
#include <miiphy.h>
#include <misc.h>
#include <net.h>
#include <netdev.h>
#include <pci.h>
#include <pci_ids.h>
#include <asm/io.h>
#include <asm/arch/board.h>
#include <linux/delay.h>
#include <linux/libfdt.h>

#include "nic_reg.h"
#include "nic.h"
#include "bgx.h"

static const phy_interface_t if_mode[] = {
	[QLM_MODE_SGMII]  = PHY_INTERFACE_MODE_SGMII,
	[QLM_MODE_RGMII]  = PHY_INTERFACE_MODE_RGMII,
	[QLM_MODE_QSGMII] = PHY_INTERFACE_MODE_QSGMII,
	[QLM_MODE_XAUI]   = PHY_INTERFACE_MODE_XAUI,
	[QLM_MODE_RXAUI]  = PHY_INTERFACE_MODE_RXAUI,
};

struct lmac {
	struct bgx		*bgx;
	int			dmac;
	u8			mac[6];
	bool			link_up;
	int			lmacid; /* ID within BGX */
	int			phy_addr; /* ID on board */
	struct udevice		*dev;
	struct mii_dev		*mii_bus;
	struct phy_device	*phydev;
	unsigned int		last_duplex;
	unsigned int		last_link;
	unsigned int		last_speed;
	int			lane_to_sds;
	int			use_training;
	int			lmac_type;
	u8			qlm_mode;
	int			qlm;
	bool			is_1gx;
};

struct bgx {
	u8			bgx_id;
	int			node;
	struct	lmac		lmac[MAX_LMAC_PER_BGX];
	int			lmac_count;
	u8			max_lmac;
	void __iomem		*reg_base;
	struct pci_dev		*pdev;
	bool			is_rgx;
};

struct bgx_board_info bgx_board_info[MAX_BGX_PER_NODE];

struct bgx *bgx_vnic[MAX_BGX_PER_NODE];

/* APIs to read/write BGXX CSRs */
static u64 bgx_reg_read(struct bgx *bgx, uint8_t lmac, u64 offset)
{
	u64 addr = (uintptr_t)bgx->reg_base +
				((uint32_t)lmac << 20) + offset;

	return readq((void *)addr);
}

static void bgx_reg_write(struct bgx *bgx, uint8_t lmac,
			  u64 offset, u64 val)
{
	u64 addr = (uintptr_t)bgx->reg_base +
				((uint32_t)lmac << 20) + offset;

	writeq(val, (void *)addr);
}

static void bgx_reg_modify(struct bgx *bgx, uint8_t lmac,
			   u64 offset, u64 val)
{
	u64 addr = (uintptr_t)bgx->reg_base +
				((uint32_t)lmac << 20) + offset;

	writeq(val | bgx_reg_read(bgx, lmac, offset), (void *)addr);
}

static int bgx_poll_reg(struct bgx *bgx, uint8_t lmac,
			u64 reg, u64 mask, bool zero)
{
	int timeout = 200;
	u64 reg_val;

	while (timeout) {
		reg_val = bgx_reg_read(bgx, lmac, reg);
		if (zero && !(reg_val & mask))
			return 0;
		if (!zero && (reg_val & mask))
			return 0;
		mdelay(1);
		timeout--;
	}
	return 1;
}

static int gser_poll_reg(u64 reg, int bit, u64 mask, u64 expected_val,
			 int timeout)
{
	u64 reg_val;

	debug("%s reg = %#llx, mask = %#llx,", __func__, reg, mask);
	debug(" expected_val = %#llx, bit = %d\n", expected_val, bit);
	while (timeout) {
		reg_val = readq(reg) >> bit;
		if ((reg_val & mask) == (expected_val))
			return 0;
		mdelay(1);
		timeout--;
	}
	return 1;
}

static bool is_bgx_port_valid(int bgx, int lmac)
{
	debug("%s bgx %d lmac %d valid %d\n", __func__, bgx, lmac,
	      bgx_board_info[bgx].lmac_reg[lmac]);

	if (bgx_board_info[bgx].lmac_reg[lmac])
		return 1;
	else
		return 0;
}

struct lmac *bgx_get_lmac(int node, int bgx_idx, int lmacid)
{
	struct bgx *bgx = bgx_vnic[(node * MAX_BGX_PER_NODE) + bgx_idx];

	if (bgx)
		return &bgx->lmac[lmacid];

	return NULL;
}

const u8 *bgx_get_lmac_mac(int node, int bgx_idx, int lmacid)
{
	struct bgx *bgx = bgx_vnic[(node * MAX_BGX_PER_NODE) + bgx_idx];

	if (bgx)
		return bgx->lmac[lmacid].mac;

	return NULL;
}

void bgx_set_lmac_mac(int node, int bgx_idx, int lmacid, const u8 *mac)
{
	struct bgx *bgx = bgx_vnic[(node * MAX_BGX_PER_NODE) + bgx_idx];

	if (!bgx)
		return;

	memcpy(bgx->lmac[lmacid].mac, mac, 6);
}

/* Return number of BGX present in HW */
void bgx_get_count(int node, int *bgx_count)
{
	int i;
	struct bgx *bgx;

	*bgx_count = 0;
	for (i = 0; i < MAX_BGX_PER_NODE; i++) {
		bgx = bgx_vnic[node * MAX_BGX_PER_NODE + i];
		debug("bgx_vnic[%u]: %p\n", node * MAX_BGX_PER_NODE + i,
		      bgx);
		if (bgx)
			*bgx_count |= (1 << i);
	}
}

/* Return number of LMAC configured for this BGX */
int bgx_get_lmac_count(int node, int bgx_idx)
{
	struct bgx *bgx;

	bgx = bgx_vnic[(node * MAX_BGX_PER_NODE) + bgx_idx];
	if (bgx)
		return bgx->lmac_count;

	return 0;
}

void bgx_lmac_rx_tx_enable(int node, int bgx_idx, int lmacid, bool enable)
{
	struct bgx *bgx = bgx_vnic[(node * MAX_BGX_PER_NODE) + bgx_idx];
	u64 cfg;

	if (!bgx)
		return;

	cfg = bgx_reg_read(bgx, lmacid, BGX_CMRX_CFG);
	if (enable)
		cfg |= CMR_PKT_RX_EN | CMR_PKT_TX_EN;
	else
		cfg &= ~(CMR_PKT_RX_EN | CMR_PKT_TX_EN);
	bgx_reg_write(bgx, lmacid, BGX_CMRX_CFG, cfg);
}

static void bgx_flush_dmac_addrs(struct bgx *bgx, u64 lmac)
{
	u64 dmac = 0x00;
	u64 offset, addr;

	while (bgx->lmac[lmac].dmac > 0) {
		offset = ((bgx->lmac[lmac].dmac - 1) * sizeof(dmac)) +
			(lmac * MAX_DMAC_PER_LMAC * sizeof(dmac));
		addr = (uintptr_t)bgx->reg_base +
				BGX_CMR_RX_DMACX_CAM + offset;
		writeq(dmac, (void *)addr);
		bgx->lmac[lmac].dmac--;
	}
}

/* Configure BGX LMAC in internal loopback mode */
void bgx_lmac_internal_loopback(int node, int bgx_idx,
				int lmac_idx, bool enable)
{
	struct bgx *bgx;
	struct lmac *lmac;
	u64    cfg;

	bgx = bgx_vnic[(node * MAX_BGX_PER_NODE) + bgx_idx];
	if (!bgx)
		return;

	lmac = &bgx->lmac[lmac_idx];
	if (lmac->qlm_mode == QLM_MODE_SGMII) {
		cfg = bgx_reg_read(bgx, lmac_idx, BGX_GMP_PCS_MRX_CTL);
		if (enable)
			cfg |= PCS_MRX_CTL_LOOPBACK1;
		else
			cfg &= ~PCS_MRX_CTL_LOOPBACK1;
		bgx_reg_write(bgx, lmac_idx, BGX_GMP_PCS_MRX_CTL, cfg);
	} else {
		cfg = bgx_reg_read(bgx, lmac_idx, BGX_SPUX_CONTROL1);
		if (enable)
			cfg |= SPU_CTL_LOOPBACK;
		else
			cfg &= ~SPU_CTL_LOOPBACK;
		bgx_reg_write(bgx, lmac_idx, BGX_SPUX_CONTROL1, cfg);
	}
}

/* Return the DLM used for the BGX */
static int get_qlm_for_bgx(int node, int bgx_id, int index)
{
	int qlm = 0;
	u64 cfg;

	if (otx_is_soc(CN81XX)) {
		qlm = (bgx_id) ? 2 : 0;
		qlm += (index >= 2) ? 1 : 0;
	} else if (otx_is_soc(CN83XX)) {
		switch (bgx_id) {
		case 0:
			qlm = 2;
			break;
		case 1:
			qlm = 3;
			break;
		case 2:
			if (index >= 2)
				qlm = 6;
			else
				qlm = 5;
			break;
		case 3:
			qlm = 4;
			break;
		}
	}

	cfg = readq(GSERX_CFG(qlm)) & GSERX_CFG_BGX;
	debug("%s:qlm%d: cfg = %lld\n", __func__, qlm, cfg);

	/* Check if DLM is configured as BGX# */
	if (cfg) {
		if (readq(GSERX_PHY_CTL(qlm)))
			return -1;
		return qlm;
	}
	return -1;
}

static int bgx_lmac_sgmii_init(struct bgx *bgx, int lmacid)
{
	u64 cfg;
	struct lmac *lmac;

	lmac = &bgx->lmac[lmacid];

	debug("%s:bgx_id = %d, lmacid = %d\n", __func__, bgx->bgx_id, lmacid);

	bgx_reg_modify(bgx, lmacid, BGX_GMP_GMI_TXX_THRESH, 0x30);
	/* max packet size */
	bgx_reg_modify(bgx, lmacid, BGX_GMP_GMI_RXX_JABBER, MAX_FRAME_SIZE);

	/* Disable frame alignment if using preamble */
	cfg = bgx_reg_read(bgx, lmacid, BGX_GMP_GMI_TXX_APPEND);
	if (cfg & 1)
		bgx_reg_write(bgx, lmacid, BGX_GMP_GMI_TXX_SGMII_CTL, 0);

	/* Enable lmac */
	bgx_reg_modify(bgx, lmacid, BGX_CMRX_CFG, CMR_EN);

	/* PCS reset */
	bgx_reg_modify(bgx, lmacid, BGX_GMP_PCS_MRX_CTL, PCS_MRX_CTL_RESET);
	if (bgx_poll_reg(bgx, lmacid, BGX_GMP_PCS_MRX_CTL,
			 PCS_MRX_CTL_RESET, true)) {
		printf("BGX PCS reset not completed\n");
		return -1;
	}

	/* power down, reset autoneg, autoneg enable */
	cfg = bgx_reg_read(bgx, lmacid, BGX_GMP_PCS_MRX_CTL);
	cfg &= ~PCS_MRX_CTL_PWR_DN;

	if (bgx_board_info[bgx->bgx_id].phy_info[lmacid].autoneg_dis)
		cfg |= (PCS_MRX_CTL_RST_AN);
	else
		cfg |= (PCS_MRX_CTL_RST_AN | PCS_MRX_CTL_AN_EN);
	bgx_reg_write(bgx, lmacid, BGX_GMP_PCS_MRX_CTL, cfg);

	/* Disable disparity for QSGMII mode, to prevent propogation across
	 * ports.
	 */

	if (lmac->qlm_mode == QLM_MODE_QSGMII) {
		cfg = bgx_reg_read(bgx, lmacid, BGX_GMP_PCS_MISCX_CTL);
		cfg &= ~PCS_MISCX_CTL_DISP_EN;
		bgx_reg_write(bgx, lmacid, BGX_GMP_PCS_MISCX_CTL, cfg);
		return 0; /* Skip checking AN_CPT */
	}

	if (lmac->is_1gx) {
		cfg = bgx_reg_read(bgx, lmacid, BGX_GMP_PCS_MISCX_CTL);
		cfg |= PCS_MISC_CTL_MODE;
		bgx_reg_write(bgx, lmacid, BGX_GMP_PCS_MISCX_CTL, cfg);
	}

	if (lmac->qlm_mode == QLM_MODE_SGMII) {
		if (bgx_poll_reg(bgx, lmacid, BGX_GMP_PCS_MRX_STATUS,
				 PCS_MRX_STATUS_AN_CPT, false)) {
			printf("BGX AN_CPT not completed\n");
			return -1;
		}
	}

	return 0;
}

static int bgx_lmac_sgmii_set_link_speed(struct lmac *lmac)
{
	u64 prtx_cfg;
	u64 pcs_miscx_ctl;
	u64 cfg;
	struct bgx *bgx = lmac->bgx;
	unsigned int lmacid = lmac->lmacid;

	debug("%s: lmacid %d\n", __func__, lmac->lmacid);

	/* Disable LMAC before setting up speed */
	cfg = bgx_reg_read(bgx, lmacid, BGX_CMRX_CFG);
	cfg &= ~CMR_EN;
	bgx_reg_write(bgx, lmacid, BGX_CMRX_CFG, cfg);

	/* Read GMX CFG */
	prtx_cfg = bgx_reg_read(bgx, lmacid,
				BGX_GMP_GMI_PRTX_CFG);
	/* Read PCS MISCS CTL */
	pcs_miscx_ctl = bgx_reg_read(bgx, lmacid,
				     BGX_GMP_PCS_MISCX_CTL);

	/* Use GMXENO to force the link down*/
	if (lmac->link_up) {
		pcs_miscx_ctl &= ~PCS_MISC_CTL_GMX_ENO;
		/* change the duplex setting if the link is up */
		prtx_cfg |= GMI_PORT_CFG_DUPLEX;
	} else {
		pcs_miscx_ctl |= PCS_MISC_CTL_GMX_ENO;
	}

	/* speed based setting for GMX */
	switch (lmac->last_speed) {
	case 10:
		prtx_cfg &= ~GMI_PORT_CFG_SPEED;
		prtx_cfg |= GMI_PORT_CFG_SPEED_MSB;
		prtx_cfg &= ~GMI_PORT_CFG_SLOT_TIME;
		pcs_miscx_ctl |= 50; /* sampling point */
		bgx_reg_write(bgx, lmacid, BGX_GMP_GMI_TXX_SLOT, 0x40);
		bgx_reg_write(bgx, lmacid, BGX_GMP_GMI_TXX_BURST, 0);
		break;
	case 100:
		prtx_cfg &= ~GMI_PORT_CFG_SPEED;
		prtx_cfg &= ~GMI_PORT_CFG_SPEED_MSB;
		prtx_cfg &= ~GMI_PORT_CFG_SLOT_TIME;
		pcs_miscx_ctl |= 0x5; /* sampling point */
		bgx_reg_write(bgx, lmacid, BGX_GMP_GMI_TXX_SLOT, 0x40);
		bgx_reg_write(bgx, lmacid, BGX_GMP_GMI_TXX_BURST, 0);
		break;
	case 1000:
		prtx_cfg |= GMI_PORT_CFG_SPEED;
		prtx_cfg &= ~GMI_PORT_CFG_SPEED_MSB;
		prtx_cfg |= GMI_PORT_CFG_SLOT_TIME;
		pcs_miscx_ctl |= 0x1; /* sampling point */
		bgx_reg_write(bgx, lmacid, BGX_GMP_GMI_TXX_SLOT, 0x200);
		if (lmac->last_duplex)
			bgx_reg_write(bgx, lmacid, BGX_GMP_GMI_TXX_BURST, 0);
		else /* half duplex */
			bgx_reg_write(bgx, lmacid, BGX_GMP_GMI_TXX_BURST,
				      0x2000);
		break;
	default:
		break;
	}

	/* write back the new PCS misc and GMX settings */
	bgx_reg_write(bgx, lmacid, BGX_GMP_PCS_MISCX_CTL, pcs_miscx_ctl);
	bgx_reg_write(bgx, lmacid, BGX_GMP_GMI_PRTX_CFG, prtx_cfg);

	/* read back GMX CFG again to check config completion */
	bgx_reg_read(bgx, lmacid, BGX_GMP_GMI_PRTX_CFG);

	/* enable BGX back */
	cfg = bgx_reg_read(bgx, lmacid, BGX_CMRX_CFG);
	cfg |= CMR_EN;
	bgx_reg_write(bgx, lmacid, BGX_CMRX_CFG, cfg);

	return 0;
}

static int bgx_lmac_xaui_init(struct bgx *bgx, int lmacid, int lmac_type)
{
	u64 cfg;
	struct lmac *lmac;

	lmac = &bgx->lmac[lmacid];

	/* Reset SPU */
	bgx_reg_modify(bgx, lmacid, BGX_SPUX_CONTROL1, SPU_CTL_RESET);
	if (bgx_poll_reg(bgx, lmacid, BGX_SPUX_CONTROL1, SPU_CTL_RESET, true)) {
		printf("BGX SPU reset not completed\n");
		return -1;
	}

	/* Disable LMAC */
	cfg = bgx_reg_read(bgx, lmacid, BGX_CMRX_CFG);
	cfg &= ~CMR_EN;
	bgx_reg_write(bgx, lmacid, BGX_CMRX_CFG, cfg);

	bgx_reg_modify(bgx, lmacid, BGX_SPUX_CONTROL1, SPU_CTL_LOW_POWER);
	/* Set interleaved running disparity for RXAUI */
	if (lmac->qlm_mode != QLM_MODE_RXAUI)
		bgx_reg_modify(bgx, lmacid,
			       BGX_SPUX_MISC_CONTROL, SPU_MISC_CTL_RX_DIS);
	else
		bgx_reg_modify(bgx, lmacid, BGX_SPUX_MISC_CONTROL,
			       SPU_MISC_CTL_RX_DIS | SPU_MISC_CTL_INTLV_RDISP);

	/* clear all interrupts */
	cfg = bgx_reg_read(bgx, lmacid, BGX_SMUX_RX_INT);
	bgx_reg_write(bgx, lmacid, BGX_SMUX_RX_INT, cfg);
	cfg = bgx_reg_read(bgx, lmacid, BGX_SMUX_TX_INT);
	bgx_reg_write(bgx, lmacid, BGX_SMUX_TX_INT, cfg);
	cfg = bgx_reg_read(bgx, lmacid, BGX_SPUX_INT);
	bgx_reg_write(bgx, lmacid, BGX_SPUX_INT, cfg);

	if (lmac->use_training) {
		bgx_reg_write(bgx, lmacid, BGX_SPUX_BR_PMD_LP_CUP, 0x00);
		bgx_reg_write(bgx, lmacid, BGX_SPUX_BR_PMD_LD_CUP, 0x00);
		bgx_reg_write(bgx, lmacid, BGX_SPUX_BR_PMD_LD_REP, 0x00);
		/* training enable */
		bgx_reg_modify(bgx, lmacid,
			       BGX_SPUX_BR_PMD_CRTL, SPU_PMD_CRTL_TRAIN_EN);
	}

	/* Append FCS to each packet */
	bgx_reg_modify(bgx, lmacid, BGX_SMUX_TX_APPEND, SMU_TX_APPEND_FCS_D);

	/* Disable forward error correction */
	cfg = bgx_reg_read(bgx, lmacid, BGX_SPUX_FEC_CONTROL);
	cfg &= ~SPU_FEC_CTL_FEC_EN;
	bgx_reg_write(bgx, lmacid, BGX_SPUX_FEC_CONTROL, cfg);

	/* Disable autoneg */
	cfg = bgx_reg_read(bgx, lmacid, BGX_SPUX_AN_CONTROL);
	cfg = cfg & ~(SPU_AN_CTL_XNP_EN);
	if (lmac->use_training)
		cfg = cfg | (SPU_AN_CTL_AN_EN);
	else
		cfg = cfg & ~(SPU_AN_CTL_AN_EN);
	bgx_reg_write(bgx, lmacid, BGX_SPUX_AN_CONTROL, cfg);

	cfg = bgx_reg_read(bgx, lmacid, BGX_SPUX_AN_ADV);
	/* Clear all KR bits, configure according to the mode */
	cfg &= ~((0xfULL << 22) | (1ULL << 12));
	if (lmac->qlm_mode == QLM_MODE_10G_KR)
		cfg |= (1 << 23);
	else if (lmac->qlm_mode == QLM_MODE_40G_KR4)
		cfg |= (1 << 24);
	bgx_reg_write(bgx, lmacid, BGX_SPUX_AN_ADV, cfg);

	cfg = bgx_reg_read(bgx, 0, BGX_SPU_DBG_CONTROL);
	if (lmac->use_training)
		cfg |= SPU_DBG_CTL_AN_ARB_LINK_CHK_EN;
	else
		cfg &= ~SPU_DBG_CTL_AN_ARB_LINK_CHK_EN;
	bgx_reg_write(bgx, 0, BGX_SPU_DBG_CONTROL, cfg);

	/* Enable lmac */
	bgx_reg_modify(bgx, lmacid, BGX_CMRX_CFG, CMR_EN);

	cfg = bgx_reg_read(bgx, lmacid, BGX_SPUX_CONTROL1);
	cfg &= ~SPU_CTL_LOW_POWER;
	bgx_reg_write(bgx, lmacid, BGX_SPUX_CONTROL1, cfg);

	cfg = bgx_reg_read(bgx, lmacid, BGX_SMUX_TX_CTL);
	cfg &= ~SMU_TX_CTL_UNI_EN;
	cfg |= SMU_TX_CTL_DIC_EN;
	bgx_reg_write(bgx, lmacid, BGX_SMUX_TX_CTL, cfg);

	/* take lmac_count into account */
	bgx_reg_modify(bgx, lmacid, BGX_SMUX_TX_THRESH, (0x100 - 1));
	/* max packet size */
	bgx_reg_modify(bgx, lmacid, BGX_SMUX_RX_JABBER, MAX_FRAME_SIZE);

	debug("xaui_init: lmacid = %d, qlm = %d, qlm_mode = %d\n",
	      lmacid, lmac->qlm, lmac->qlm_mode);
	/* RXAUI with Marvell PHY requires some tweaking */
	if (lmac->qlm_mode == QLM_MODE_RXAUI) {
		char mii_name[20];
		struct phy_info *phy;

		phy = &bgx_board_info[bgx->bgx_id].phy_info[lmacid];
		snprintf(mii_name, sizeof(mii_name), "smi%d", phy->mdio_bus);

		debug("mii_name: %s\n", mii_name);
		lmac->mii_bus = miiphy_get_dev_by_name(mii_name);
		lmac->phy_addr = phy->phy_addr;
		rxaui_phy_xs_init(lmac->mii_bus, lmac->phy_addr);
	}

	return 0;
}

/* Get max number of lanes present in a given QLM/DLM */
static int get_qlm_lanes(int qlm)
{
	if (otx_is_soc(CN81XX))
		return 2;
	else if (otx_is_soc(CN83XX))
		return (qlm >= 5) ? 2 : 4;
	else
		return -1;
}

int __rx_equalization(int qlm, int lane)
{
	int max_lanes = get_qlm_lanes(qlm);
	int l;
	int fail = 0;

	/* Before completing Rx equalization wait for
	 * GSERx_RX_EIE_DETSTS[CDRLOCK] to be set
	 * This ensures the rx data is valid
	 */
	if (lane == -1) {
		if (gser_poll_reg(GSER_RX_EIE_DETSTS(qlm), GSER_CDRLOCK, 0xf,
				  (1 << max_lanes) - 1, 100)) {
			debug("ERROR: CDR Lock not detected");
			debug(" on DLM%d for 2 lanes\n", qlm);
			return -1;
		}
	} else {
		if (gser_poll_reg(GSER_RX_EIE_DETSTS(qlm), GSER_CDRLOCK,
				  (0xf & (1 << lane)), (1 << lane), 100)) {
			debug("ERROR: DLM%d: CDR Lock not detected", qlm);
			debug(" on %d lane\n", lane);
			return -1;
		}
	}

	for (l = 0; l < max_lanes; l++) {
		u64 rctl, reer;

		if (lane != -1 && lane != l)
			continue;

		/* Enable software control */
		rctl = readq(GSER_BR_RXX_CTL(qlm, l));
		rctl |= GSER_BR_RXX_CTL_RXT_SWM;
		writeq(rctl, GSER_BR_RXX_CTL(qlm, l));

		/* Clear the completion flag and initiate a new request */
		reer = readq(GSER_BR_RXX_EER(qlm, l));
		reer &= ~GSER_BR_RXX_EER_RXT_ESV;
		reer |= GSER_BR_RXX_EER_RXT_EER;
		writeq(reer, GSER_BR_RXX_EER(qlm, l));
	}

	/* Wait for RX equalization to complete */
	for (l = 0; l < max_lanes; l++) {
		u64 rctl, reer;

		if (lane != -1 && lane != l)
			continue;

		gser_poll_reg(GSER_BR_RXX_EER(qlm, l), EER_RXT_ESV, 1, 1, 200);
		reer = readq(GSER_BR_RXX_EER(qlm, l));

		/* Switch back to hardware control */
		rctl = readq(GSER_BR_RXX_CTL(qlm, l));
		rctl &= ~GSER_BR_RXX_CTL_RXT_SWM;
		writeq(rctl, GSER_BR_RXX_CTL(qlm, l));

		if (reer & GSER_BR_RXX_EER_RXT_ESV) {
			debug("Rx equalization completed on DLM%d", qlm);
			debug(" QLM%d rxt_esm = 0x%llx\n", l, (reer & 0x3fff));
		} else {
			debug("Rx equalization timedout on DLM%d", qlm);
			debug(" lane %d\n", l);
			fail = 1;
		}
	}

	return (fail) ? -1 : 0;
}

static int bgx_xaui_check_link(struct lmac *lmac)
{
	struct bgx *bgx = lmac->bgx;
	int lmacid = lmac->lmacid;
	int lmac_type = lmac->lmac_type;
	u64 cfg;

	bgx_reg_modify(bgx, lmacid, BGX_SPUX_MISC_CONTROL, SPU_MISC_CTL_RX_DIS);

	/* check if auto negotiation is complete */
	cfg = bgx_reg_read(bgx, lmacid, BGX_SPUX_AN_CONTROL);
	if (cfg & SPU_AN_CTL_AN_EN) {
		cfg = bgx_reg_read(bgx, lmacid, BGX_SPUX_AN_STATUS);
		if (!(cfg & SPU_AN_STS_AN_COMPLETE)) {
			/* Restart autonegotiation */
			debug("restarting auto-neg\n");
			bgx_reg_modify(bgx, lmacid, BGX_SPUX_AN_CONTROL,
				       SPU_AN_CTL_AN_RESTART);
			return -1;
		}
	}

	debug("%s link use_training %d\n", __func__, lmac->use_training);
	if (lmac->use_training) {
		cfg = bgx_reg_read(bgx, lmacid, BGX_SPUX_INT);
		if (!(cfg & (1ull << 13))) {
			debug("waiting for link training\n");
			/* Clear the training interrupts (W1C) */
			cfg = (1ull << 13) | (1ull << 14);
			bgx_reg_write(bgx, lmacid, BGX_SPUX_INT, cfg);

			udelay(2000);
			/* Restart training */
			cfg = bgx_reg_read(bgx, lmacid, BGX_SPUX_BR_PMD_CRTL);
			cfg |= (1ull << 0);
			bgx_reg_write(bgx, lmacid, BGX_SPUX_BR_PMD_CRTL, cfg);
			return -1;
		}
	}

	/* Perform RX Equalization. Applies to non-KR interfaces for speeds
	 * >= 6.25Gbps.
	 */
	if (!lmac->use_training) {
		int qlm;
		bool use_dlm = 0;

		if (otx_is_soc(CN81XX) || (otx_is_soc(CN83XX) &&
					   bgx->bgx_id == 2))
			use_dlm = 1;
		switch (lmac->lmac_type) {
		default:
		case BGX_MODE_SGMII:
		case BGX_MODE_RGMII:
		case BGX_MODE_XAUI:
			/* Nothing to do */
			break;
		case BGX_MODE_XLAUI:
			if (use_dlm) {
				if (__rx_equalization(lmac->qlm, -1) ||
				    __rx_equalization(lmac->qlm + 1, -1)) {
					printf("BGX%d:%d", bgx->bgx_id, lmacid);
					printf(" Waiting for RX Equalization");
					printf(" on DLM%d/DLM%d\n",
					       lmac->qlm, lmac->qlm + 1);
					return -1;
				}
			} else {
				if (__rx_equalization(lmac->qlm, -1)) {
					printf("BGX%d:%d", bgx->bgx_id, lmacid);
					printf(" Waiting for RX Equalization");
					printf(" on QLM%d\n", lmac->qlm);
					return -1;
				}
			}
			break;
		case BGX_MODE_RXAUI:
			/* RXAUI0 uses LMAC0:QLM0/QLM2 and RXAUI1 uses
			 * LMAC1:QLM1/QLM3 RXAUI requires 2 lanes
			 * for each interface
			 */
			qlm = lmac->qlm;
			if (__rx_equalization(qlm, 0)) {
				printf("BGX%d:%d", bgx->bgx_id, lmacid);
				printf(" Waiting for RX Equalization");
				printf(" on QLM%d, Lane0\n", qlm);
				return -1;
			}
			if (__rx_equalization(qlm, 1)) {
				printf("BGX%d:%d", bgx->bgx_id, lmacid);
				printf(" Waiting for RX Equalization");
				printf(" on QLM%d, Lane1\n", qlm);
				return -1;
			}
			break;
		case BGX_MODE_XFI:
			{
				int lid;
				bool altpkg = otx_is_altpkg();

				if (bgx->bgx_id == 0 && altpkg && lmacid)
					lid = 0;
				else if ((lmacid >= 2) && use_dlm)
					lid = lmacid - 2;
				else
					lid = lmacid;

				if (__rx_equalization(lmac->qlm, lid)) {
					printf("BGX%d:%d", bgx->bgx_id, lid);
					printf(" Waiting for RX Equalization");
					printf(" on QLM%d\n", lmac->qlm);
				}
			}
			break;
		}
	}

	/* wait for PCS to come out of reset */
	if (bgx_poll_reg(bgx, lmacid, BGX_SPUX_CONTROL1, SPU_CTL_RESET, true)) {
		printf("BGX SPU reset not completed\n");
		return -1;
	}

	if (lmac_type == 3 || lmac_type == 4) {
		if (bgx_poll_reg(bgx, lmacid, BGX_SPUX_BR_STATUS1,
				 SPU_BR_STATUS_BLK_LOCK, false)) {
			printf("SPU_BR_STATUS_BLK_LOCK not completed\n");
			return -1;
		}
	} else {
		if (bgx_poll_reg(bgx, lmacid, BGX_SPUX_BX_STATUS,
				 SPU_BX_STATUS_RX_ALIGN, false)) {
			printf("SPU_BX_STATUS_RX_ALIGN not completed\n");
			return -1;
		}
	}

	/* Clear rcvflt bit (latching high) and read it back */
	bgx_reg_modify(bgx, lmacid, BGX_SPUX_STATUS2, SPU_STATUS2_RCVFLT);
	if (bgx_reg_read(bgx, lmacid, BGX_SPUX_STATUS2) & SPU_STATUS2_RCVFLT) {
		printf("Receive fault, retry training\n");
		if (lmac->use_training) {
			cfg = bgx_reg_read(bgx, lmacid, BGX_SPUX_INT);
			if (!(cfg & (1ull << 13))) {
				cfg = (1ull << 13) | (1ull << 14);
				bgx_reg_write(bgx, lmacid, BGX_SPUX_INT, cfg);
				cfg = bgx_reg_read(bgx, lmacid,
						   BGX_SPUX_BR_PMD_CRTL);
				cfg |= (1ull << 0);
				bgx_reg_write(bgx, lmacid,
					      BGX_SPUX_BR_PMD_CRTL, cfg);
				return -1;
			}
		}
		return -1;
	}

	/* Wait for MAC RX to be ready */
	if (bgx_poll_reg(bgx, lmacid, BGX_SMUX_RX_CTL,
			 SMU_RX_CTL_STATUS, true)) {
		printf("SMU RX link not okay\n");
		return -1;
	}

	/* Wait for BGX RX to be idle */
	if (bgx_poll_reg(bgx, lmacid, BGX_SMUX_CTL, SMU_CTL_RX_IDLE, false)) {
		printf("SMU RX not idle\n");
		return -1;
	}

	/* Wait for BGX TX to be idle */
	if (bgx_poll_reg(bgx, lmacid, BGX_SMUX_CTL, SMU_CTL_TX_IDLE, false)) {
		printf("SMU TX not idle\n");
		return -1;
	}

	if (bgx_reg_read(bgx, lmacid, BGX_SPUX_STATUS2) & SPU_STATUS2_RCVFLT) {
		printf("Receive fault\n");
		return -1;
	}

	/* Receive link is latching low. Force it high and verify it */
	if (!(bgx_reg_read(bgx, lmacid, BGX_SPUX_STATUS1) &
	    SPU_STATUS1_RCV_LNK))
		bgx_reg_modify(bgx, lmacid, BGX_SPUX_STATUS1,
			       SPU_STATUS1_RCV_LNK);
	if (bgx_poll_reg(bgx, lmacid, BGX_SPUX_STATUS1,
			 SPU_STATUS1_RCV_LNK, false)) {
		printf("SPU receive link down\n");
		return -1;
	}

	cfg = bgx_reg_read(bgx, lmacid, BGX_SPUX_MISC_CONTROL);
	cfg &= ~SPU_MISC_CTL_RX_DIS;
	bgx_reg_write(bgx, lmacid, BGX_SPUX_MISC_CONTROL, cfg);
	return 0;
}

static int bgx_lmac_enable(struct bgx *bgx, int8_t lmacid)
{
	struct lmac *lmac;
	u64 cfg;

	lmac = &bgx->lmac[lmacid];
	lmac->bgx = bgx;

	debug("%s: lmac: %p, lmacid = %d\n", __func__, lmac, lmacid);

	if (lmac->qlm_mode == QLM_MODE_SGMII ||
	    lmac->qlm_mode == QLM_MODE_RGMII ||
	    lmac->qlm_mode == QLM_MODE_QSGMII) {
		if (bgx_lmac_sgmii_init(bgx, lmacid)) {
			debug("bgx_lmac_sgmii_init failed\n");
			return -1;
		}
		cfg = bgx_reg_read(bgx, lmacid, BGX_GMP_GMI_TXX_APPEND);
		cfg |= ((1ull << 2) | (1ull << 1)); /* FCS and PAD */
		bgx_reg_modify(bgx, lmacid, BGX_GMP_GMI_TXX_APPEND, cfg);
		bgx_reg_write(bgx, lmacid, BGX_GMP_GMI_TXX_MIN_PKT, 60 - 1);
	} else {
		if (bgx_lmac_xaui_init(bgx, lmacid, lmac->lmac_type))
			return -1;
		cfg = bgx_reg_read(bgx, lmacid, BGX_SMUX_TX_APPEND);
		cfg |= ((1ull << 2) | (1ull << 1)); /* FCS and PAD */
		bgx_reg_modify(bgx, lmacid, BGX_SMUX_TX_APPEND, cfg);
		bgx_reg_write(bgx, lmacid, BGX_SMUX_TX_MIN_PKT, 60 + 4);
	}

	/* Enable lmac */
	bgx_reg_modify(bgx, lmacid, BGX_CMRX_CFG,
		       CMR_EN | CMR_PKT_RX_EN | CMR_PKT_TX_EN);

	return 0;
}

int bgx_poll_for_link(int node, int bgx_idx, int lmacid)
{
	int ret;
	struct lmac *lmac = bgx_get_lmac(node, bgx_idx, lmacid);
	char mii_name[10];
	struct phy_info *phy;

	if (!lmac) {
		printf("LMAC %d/%d/%d is disabled or doesn't exist\n",
		       node, bgx_idx, lmacid);
		return 0;
	}

	debug("%s: %d, lmac: %d/%d/%d %p\n",
	      __FILE__, __LINE__,
	      node, bgx_idx, lmacid, lmac);
	if (lmac->qlm_mode == QLM_MODE_SGMII ||
	    lmac->qlm_mode == QLM_MODE_RGMII ||
	    lmac->qlm_mode == QLM_MODE_QSGMII) {
		if (bgx_board_info[bgx_idx].phy_info[lmacid].phy_addr == -1) {
			lmac->link_up = 1;
			lmac->last_speed = 1000;
			lmac->last_duplex = 1;
			printf("BGX%d:LMAC %u link up\n", bgx_idx, lmacid);
			return lmac->link_up;
		}
		snprintf(mii_name, sizeof(mii_name), "smi%d",
			 bgx_board_info[bgx_idx].phy_info[lmacid].mdio_bus);

		debug("mii_name: %s\n", mii_name);

		lmac->mii_bus = miiphy_get_dev_by_name(mii_name);
		phy = &bgx_board_info[bgx_idx].phy_info[lmacid];
		lmac->phy_addr = phy->phy_addr;

		debug("lmac->mii_bus: %p\n", lmac->mii_bus);
		if (!lmac->mii_bus) {
			printf("MDIO device %s not found\n", mii_name);
			ret = -ENODEV;
			return ret;
		}

		lmac->phydev = phy_connect(lmac->mii_bus, lmac->phy_addr,
					   lmac->dev,
					   if_mode[lmac->qlm_mode]);

		if (!lmac->phydev) {
			printf("%s: No PHY device\n", __func__);
			return -1;
		}

		ret = phy_config(lmac->phydev);
		if (ret) {
			printf("%s: Could not initialize PHY %s\n",
			       __func__, lmac->phydev->dev->name);
			return ret;
		}

		ret = phy_startup(lmac->phydev);
		debug("%s: %d\n", __FILE__, __LINE__);
		if (ret) {
			printf("%s: Could not initialize PHY %s\n",
			       __func__, lmac->phydev->dev->name);
		}

#ifdef OCTEONTX_XCV
		if (lmac->qlm_mode == QLM_MODE_RGMII)
			xcv_setup_link(lmac->phydev->link, lmac->phydev->speed);
#endif

		lmac->link_up = lmac->phydev->link;
		lmac->last_speed = lmac->phydev->speed;
		lmac->last_duplex = lmac->phydev->duplex;

		debug("%s qlm_mode %d phy link status 0x%x,last speed 0x%x,",
		      __func__, lmac->qlm_mode, lmac->link_up,
		      lmac->last_speed);
		debug(" duplex 0x%x\n", lmac->last_duplex);

		if (lmac->qlm_mode != QLM_MODE_RGMII)
			bgx_lmac_sgmii_set_link_speed(lmac);

	} else {
		u64 status1;
		u64 tx_ctl;
		u64 rx_ctl;

		status1 = bgx_reg_read(lmac->bgx, lmac->lmacid,
				       BGX_SPUX_STATUS1);
		tx_ctl = bgx_reg_read(lmac->bgx, lmac->lmacid, BGX_SMUX_TX_CTL);
		rx_ctl = bgx_reg_read(lmac->bgx, lmac->lmacid, BGX_SMUX_RX_CTL);

		debug("BGX%d LMAC%d BGX_SPUX_STATUS2: %lx\n", bgx_idx, lmacid,
		      (unsigned long)bgx_reg_read(lmac->bgx, lmac->lmacid,
						  BGX_SPUX_STATUS2));
		debug("BGX%d LMAC%d BGX_SPUX_STATUS1: %lx\n", bgx_idx, lmacid,
		      (unsigned long)bgx_reg_read(lmac->bgx, lmac->lmacid,
						  BGX_SPUX_STATUS1));
		debug("BGX%d LMAC%d BGX_SMUX_RX_CTL: %lx\n", bgx_idx, lmacid,
		      (unsigned long)bgx_reg_read(lmac->bgx, lmac->lmacid,
						  BGX_SMUX_RX_CTL));
		debug("BGX%d LMAC%d BGX_SMUX_TX_CTL: %lx\n", bgx_idx, lmacid,
		      (unsigned long)bgx_reg_read(lmac->bgx, lmac->lmacid,
						  BGX_SMUX_TX_CTL));

		if ((status1 & SPU_STATUS1_RCV_LNK) &&
		    ((tx_ctl & SMU_TX_CTL_LNK_STATUS) == 0) &&
		    ((rx_ctl & SMU_RX_CTL_STATUS) == 0)) {
			lmac->link_up = 1;
			if (lmac->lmac_type == 4)
				lmac->last_speed = 40000;
			else
				lmac->last_speed = 10000;
			lmac->last_duplex = 1;
		} else {
			lmac->link_up = 0;
			lmac->last_speed = 0;
			lmac->last_duplex = 0;
			return bgx_xaui_check_link(lmac);
		}

		lmac->last_link = lmac->link_up;
	}

	printf("BGX%d:LMAC %u link %s\n", bgx_idx, lmacid,
	       (lmac->link_up) ? "up" : "down");

	return lmac->link_up;
}

void bgx_lmac_disable(struct bgx *bgx, uint8_t lmacid)
{
	struct lmac *lmac;
	u64 cmrx_cfg;

	lmac = &bgx->lmac[lmacid];

	cmrx_cfg = bgx_reg_read(bgx, lmacid, BGX_CMRX_CFG);
	cmrx_cfg &= ~(1 << 15);
	bgx_reg_write(bgx, lmacid, BGX_CMRX_CFG, cmrx_cfg);
	bgx_flush_dmac_addrs(bgx, lmacid);

	if (lmac->phydev)
		phy_shutdown(lmac->phydev);

	lmac->phydev = NULL;
}

/* Program BGXX_CMRX_CONFIG.{lmac_type,lane_to_sds} for each interface.
 * And the number of LMACs used by this interface. Each lmac can be in
 * programmed in a different mode, so parse each lmac one at a time.
 */
static void bgx_init_hw(struct bgx *bgx)
{
	struct lmac *lmac;
	int i, lmacid, count = 0, inc = 0;
	char buf[40];
	static int qsgmii_configured;

	for (lmacid = 0; lmacid < MAX_LMAC_PER_BGX; lmacid++) {
		struct lmac *tlmac;

		lmac = &bgx->lmac[lmacid];
		debug("%s: lmacid = %d, qlm = %d, mode = %d\n",
		      __func__, lmacid, lmac->qlm, lmac->qlm_mode);
		/* If QLM is not programmed, skip */
		if (lmac->qlm == -1)
			continue;

		switch (lmac->qlm_mode) {
		case QLM_MODE_SGMII:
		{
			/* EBB8000 (alternative pkg) has only lane0 present on
			 * DLM0 and DLM1, skip configuring other lanes
			 */
			if (bgx->bgx_id == 0 && otx_is_altpkg()) {
				if (lmacid % 2)
					continue;
			}
			lmac->lane_to_sds = lmacid;
			lmac->lmac_type = 0;
			snprintf(buf, sizeof(buf),
				 "BGX%d QLM%d LMAC%d mode: %s\n",
				 bgx->bgx_id, lmac->qlm, lmacid,
				 lmac->is_1gx ? "1000Base-X" : "SGMII");
			break;
		}
		case QLM_MODE_XAUI:
			if (lmacid != 0)
				continue;
			lmac->lmac_type = 1;
			lmac->lane_to_sds = 0xE4;
			snprintf(buf, sizeof(buf),
				 "BGX%d QLM%d LMAC%d mode: XAUI\n",
				 bgx->bgx_id, lmac->qlm, lmacid);
			break;
		case QLM_MODE_RXAUI:
			if (lmacid == 0) {
				lmac->lmac_type = 2;
				lmac->lane_to_sds = 0x4;
			} else if (lmacid == 1) {
				struct lmac *tlmac;

				tlmac = &bgx->lmac[2];
				if (tlmac->qlm_mode == QLM_MODE_RXAUI) {
					lmac->lmac_type = 2;
					lmac->lane_to_sds = 0xe;
					lmac->qlm = tlmac->qlm;
				}
			} else {
				continue;
			}
			snprintf(buf, sizeof(buf),
				 "BGX%d QLM%d LMAC%d mode: RXAUI\n",
				 bgx->bgx_id, lmac->qlm, lmacid);
			break;
		case QLM_MODE_XFI:
			/* EBB8000 (alternative pkg) has only lane0 present on
			 * DLM0 and DLM1, skip configuring other lanes
			 */
			if (bgx->bgx_id == 0 && otx_is_altpkg()) {
				if (lmacid % 2)
					continue;
			}
			lmac->lane_to_sds = lmacid;
			lmac->lmac_type = 3;
			snprintf(buf, sizeof(buf),
				 "BGX%d QLM%d LMAC%d mode: XFI\n",
				 bgx->bgx_id, lmac->qlm, lmacid);
			break;
		case QLM_MODE_XLAUI:
			if (lmacid != 0)
				continue;
			lmac->lmac_type = 4;
			lmac->lane_to_sds = 0xE4;
			snprintf(buf, sizeof(buf),
				 "BGX%d QLM%d LMAC%d mode: XLAUI\n",
				 bgx->bgx_id, lmac->qlm, lmacid);
			break;
		case QLM_MODE_10G_KR:
			/* EBB8000 (alternative pkg) has only lane0 present on
			 * DLM0 and DLM1, skip configuring other lanes
			 */
			if (bgx->bgx_id == 0 && otx_is_altpkg()) {
				if (lmacid % 2)
					continue;
			}
			lmac->lane_to_sds = lmacid;
			lmac->lmac_type = 3;
			lmac->use_training = 1;
			snprintf(buf, sizeof(buf),
				 "BGX%d QLM%d LMAC%d mode: 10G-KR\n",
				 bgx->bgx_id, lmac->qlm, lmacid);
			break;
		case QLM_MODE_40G_KR4:
			if (lmacid != 0)
				continue;
			lmac->lmac_type = 4;
			lmac->lane_to_sds = 0xE4;
			lmac->use_training = 1;
			snprintf(buf, sizeof(buf),
				 "BGX%d QLM%d LMAC%d mode: 40G-KR4\n",
				 bgx->bgx_id, lmac->qlm, lmacid);
			break;
		case QLM_MODE_RGMII:
			if (lmacid != 0)
				continue;
			lmac->lmac_type = 5;
			lmac->lane_to_sds = 0xE4;
			snprintf(buf, sizeof(buf),
				 "BGX%d LMAC%d mode: RGMII\n",
				 bgx->bgx_id, lmacid);
			break;
		case QLM_MODE_QSGMII:
			if (qsgmii_configured)
				continue;
			if (lmacid == 0 || lmacid == 2) {
				count = 4;
				printf("BGX%d QLM%d LMAC%d mode: QSGMII\n",
				       bgx->bgx_id, lmac->qlm, lmacid);
				for (i = 0; i < count; i++) {
					struct lmac *l;
					int type;

					l = &bgx->lmac[i];
					l->lmac_type = 6;
					type = l->lmac_type;
					l->qlm_mode = QLM_MODE_QSGMII;
					l->lane_to_sds = lmacid + i;
					if (is_bgx_port_valid(bgx->bgx_id, i))
						bgx_reg_write(bgx, i,
							      BGX_CMRX_CFG,
							      (type << 8) |
							      l->lane_to_sds);
				}
				qsgmii_configured = 1;
			}
			continue;
		default:
			continue;
		}

		/* Reset lmac to the unused slot */
		if (is_bgx_port_valid(bgx->bgx_id, count) &&
		    lmac->qlm_mode != QLM_MODE_QSGMII) {
			int lmac_en = 0;
			int tmp, idx;

			tlmac = &bgx->lmac[count];
			tlmac->lmac_type = lmac->lmac_type;
			idx = bgx->bgx_id;
			tmp = count + inc;
			/* Adjust lane_to_sds based on BGX-ENABLE */
			for (; tmp < MAX_LMAC_PER_BGX; inc++) {
				lmac_en = bgx_board_info[idx].lmac_enable[tmp];
				if (lmac_en)
					break;
				tmp = count + inc;
			}

			if (inc != 0 && inc < MAX_LMAC_PER_BGX &&
			    lmac_en && inc != count)
				tlmac->lane_to_sds =
					lmac->lane_to_sds + abs(inc - count);
			else
				tlmac->lane_to_sds = lmac->lane_to_sds;
			tlmac->qlm = lmac->qlm;
			tlmac->qlm_mode = lmac->qlm_mode;

			printf("%s", buf);
			/* Initialize lmac_type and lane_to_sds */
			bgx_reg_write(bgx, count, BGX_CMRX_CFG,
				      (tlmac->lmac_type << 8) |
				      tlmac->lane_to_sds);

			if (tlmac->lmac_type == BGX_MODE_SGMII) {
				if (tlmac->is_1gx) {
					/* This is actually 1000BASE-X, so
					 * mark the LMAC as such.
					 */
					bgx_reg_modify(bgx, count,
						       BGX_GMP_PCS_MISCX_CTL,
						       PCS_MISC_CTL_MODE);
				}

				if (!bgx_board_info[bgx->bgx_id].phy_info[lmacid].autoneg_dis) {
					/* The Linux DTS does not disable
					 * autoneg for this LMAC (in SGMII or
					 * 1000BASE-X mode), so that means
					 * enable autoneg.
					 */
					bgx_reg_modify(bgx, count,
						       BGX_GMP_PCS_MRX_CTL,
						       PCS_MRX_CTL_AN_EN);
				}
			}

			count += 1;
		}
	}

	/* Done probing all 4 lmacs, now clear qsgmii_configured */
	qsgmii_configured = 0;

	printf("BGX%d LMACs: %d\n", bgx->bgx_id, count);
	bgx->lmac_count = count;
	bgx_reg_write(bgx, 0, BGX_CMR_RX_LMACS, count);
	bgx_reg_write(bgx, 0, BGX_CMR_TX_LMACS, count);

	bgx_reg_modify(bgx, 0, BGX_CMR_GLOBAL_CFG, CMR_GLOBAL_CFG_FCS_STRIP);
	if (bgx_reg_read(bgx, 0, BGX_CMR_BIST_STATUS))
		printf("BGX%d BIST failed\n", bgx->bgx_id);

	/* Set the backpressure AND mask */
	for (i = 0; i < bgx->lmac_count; i++)
		bgx_reg_modify(bgx, 0, BGX_CMR_CHAN_MSK_AND,
			       ((1ULL << MAX_BGX_CHANS_PER_LMAC) - 1) <<
				(i * MAX_BGX_CHANS_PER_LMAC));

	/* Disable all MAC filtering */
	for (i = 0; i < RX_DMAC_COUNT; i++)
		bgx_reg_write(bgx, 0, BGX_CMR_RX_DMACX_CAM + (i * 8), 0x00);

	/* Disable MAC steering (NCSI traffic) */
	for (i = 0; i < RX_TRAFFIC_STEER_RULE_COUNT; i++)
		bgx_reg_write(bgx, 0, BGX_CMR_RX_STREERING + (i * 8), 0x00);
}

static void bgx_get_qlm_mode(struct bgx *bgx)
{
	struct lmac *lmac;
	int lmacid;

	/* Read LMACx type to figure out QLM mode
	 * This is configured by low level firmware
	 */
	for (lmacid = 0; lmacid < MAX_LMAC_PER_BGX; lmacid++) {
		int lmac_type;
		int train_en;
		int index = 0;

		if (otx_is_soc(CN81XX) || (otx_is_soc(CN83XX) &&
					   bgx->bgx_id == 2))
			index = (lmacid < 2) ? 0 : 2;

		lmac = &bgx->lmac[lmacid];

		/* check if QLM is programmed, if not, skip */
		if (lmac->qlm == -1)
			continue;

		lmac_type = bgx_reg_read(bgx, index, BGX_CMRX_CFG);
		lmac->lmac_type = (lmac_type >> 8) & 0x07;
		debug("%s:%d:%d: lmac_type = %d, altpkg = %d\n", __func__,
		      bgx->bgx_id, lmacid, lmac->lmac_type, otx_is_altpkg());

		train_en = (readq(GSERX_SCRATCH(lmac->qlm))) & 0xf;
		lmac->is_1gx = bgx_reg_read(bgx, index, BGX_GMP_PCS_MISCX_CTL)
				& (PCS_MISC_CTL_MODE) ? true : false;

		switch (lmac->lmac_type) {
		case BGX_MODE_SGMII:
			if (bgx->is_rgx) {
				if (lmacid == 0) {
					lmac->qlm_mode = QLM_MODE_RGMII;
					debug("BGX%d LMAC%d mode: RGMII\n",
					      bgx->bgx_id, lmacid);
				}
				continue;
			} else {
				if (bgx->bgx_id == 0 && otx_is_altpkg()) {
					if (lmacid % 2)
						continue;
				}
				lmac->qlm_mode = QLM_MODE_SGMII;
				debug("BGX%d QLM%d LMAC%d mode: %s\n",
				      bgx->bgx_id, lmac->qlm, lmacid,
				      lmac->is_1gx ? "1000Base-X" : "SGMII");
			}
			break;
		case BGX_MODE_XAUI:
			if (bgx->bgx_id == 0 && otx_is_altpkg())
				continue;
			lmac->qlm_mode = QLM_MODE_XAUI;
			if (lmacid != 0)
				continue;
			debug("BGX%d QLM%d LMAC%d mode: XAUI\n",
			      bgx->bgx_id, lmac->qlm, lmacid);
			break;
		case BGX_MODE_RXAUI:
			if (bgx->bgx_id == 0 && otx_is_altpkg())
				continue;
			lmac->qlm_mode = QLM_MODE_RXAUI;
			if (index == lmacid) {
				debug("BGX%d QLM%d LMAC%d mode: RXAUI\n",
				      bgx->bgx_id, lmac->qlm, (index ? 1 : 0));
			}
			break;
		case BGX_MODE_XFI:
			if (bgx->bgx_id == 0 && otx_is_altpkg()) {
				if (lmacid % 2)
					continue;
			}
			if ((lmacid < 2 && (train_en & (1 << lmacid))) ||
			    (train_en & (1 << (lmacid - 2)))) {
				lmac->qlm_mode = QLM_MODE_10G_KR;
				debug("BGX%d QLM%d LMAC%d mode: 10G_KR\n",
				      bgx->bgx_id, lmac->qlm, lmacid);
			} else {
				lmac->qlm_mode = QLM_MODE_XFI;
				debug("BGX%d QLM%d LMAC%d mode: XFI\n",
				      bgx->bgx_id, lmac->qlm, lmacid);
			}
			break;
		case BGX_MODE_XLAUI:
			if (bgx->bgx_id == 0 && otx_is_altpkg())
				continue;
			if (train_en) {
				lmac->qlm_mode = QLM_MODE_40G_KR4;
				if (lmacid != 0)
					break;
				debug("BGX%d QLM%d LMAC%d mode: 40G_KR4\n",
				      bgx->bgx_id, lmac->qlm, lmacid);
			} else {
				lmac->qlm_mode = QLM_MODE_XLAUI;
				if (lmacid != 0)
					break;
				debug("BGX%d QLM%d LMAC%d mode: XLAUI\n",
				      bgx->bgx_id, lmac->qlm, lmacid);
			}
		break;
		case BGX_MODE_QSGMII:
			/* If QLM is configured as QSGMII, use lmac0 */
			if (otx_is_soc(CN83XX) && lmacid == 2 &&
			    bgx->bgx_id != 2) {
				//lmac->qlm_mode = QLM_MODE_DISABLED;
				continue;
			}

			if (lmacid == 0 || lmacid == 2) {
				lmac->qlm_mode = QLM_MODE_QSGMII;
				debug("BGX%d QLM%d LMAC%d mode: QSGMII\n",
				      bgx->bgx_id, lmac->qlm, lmacid);
			}
			break;
		default:
			break;
		}
	}
}

void bgx_set_board_info(int bgx_id, int *mdio_bus,
			int *phy_addr, bool *autoneg_dis, bool *lmac_reg,
			bool *lmac_enable)
{
	unsigned int i;

	for (i = 0; i < MAX_LMAC_PER_BGX; i++) {
		bgx_board_info[bgx_id].phy_info[i].phy_addr = phy_addr[i];
		bgx_board_info[bgx_id].phy_info[i].mdio_bus = mdio_bus[i];
		bgx_board_info[bgx_id].phy_info[i].autoneg_dis = autoneg_dis[i];
		bgx_board_info[bgx_id].lmac_reg[i] = lmac_reg[i];
		bgx_board_info[bgx_id].lmac_enable[i] = lmac_enable[i];
		debug("%s bgx_id %d lmac %d\n", __func__, bgx_id, i);
		debug("phy addr %x mdio bus %d autoneg_dis %d lmac_reg %d\n",
		      bgx_board_info[bgx_id].phy_info[i].phy_addr,
		      bgx_board_info[bgx_id].phy_info[i].mdio_bus,
		      bgx_board_info[bgx_id].phy_info[i].autoneg_dis,
		      bgx_board_info[bgx_id].lmac_reg[i]);
		debug("lmac_enable = %x\n",
		      bgx_board_info[bgx_id].lmac_enable[i]);
	}
}

int octeontx_bgx_remove(struct udevice *dev)
{
	int lmacid;
	u64 cfg;
	int count = MAX_LMAC_PER_BGX;
	struct bgx *bgx = dev_get_priv(dev);

	if (!bgx->reg_base)
		return 0;

	if (bgx->is_rgx)
		count = 1;

	for (lmacid = 0; lmacid < count; lmacid++) {
		struct lmac *lmac;

		lmac = &bgx->lmac[lmacid];
		cfg = bgx_reg_read(bgx, lmacid, BGX_CMRX_CFG);
		cfg &= ~(CMR_PKT_RX_EN | CMR_PKT_TX_EN);
		bgx_reg_write(bgx, lmacid, BGX_CMRX_CFG, cfg);

		/* Disable PCS for 1G interface */
		if (lmac->lmac_type == BGX_MODE_SGMII ||
		    lmac->lmac_type == BGX_MODE_QSGMII) {
			cfg = bgx_reg_read(bgx, lmacid, BGX_GMP_PCS_MRX_CTL);
			cfg |= PCS_MRX_CTL_PWR_DN;
			bgx_reg_write(bgx, lmacid, BGX_GMP_PCS_MRX_CTL, cfg);
		}

		debug("%s disabling bgx%d lmacid%d\n", __func__, bgx->bgx_id,
		      lmacid);
		bgx_lmac_disable(bgx, lmacid);
	}
	return 0;
}

int octeontx_bgx_probe(struct udevice *dev)
{
	int err;
	struct bgx *bgx = dev_get_priv(dev);
	u8 lmac = 0;
	int qlm[4] = {-1, -1, -1, -1};
	int bgx_idx, node;
	int inc = 1;

	bgx->reg_base = dm_pci_map_bar(dev, PCI_BASE_ADDRESS_0, 0, 0, PCI_REGION_TYPE,
				       PCI_REGION_MEM);
	if (!bgx->reg_base) {
		debug("No PCI region found\n");
		return 0;
	}

#ifdef OCTEONTX_XCV
	/* Use FAKE BGX2 for RGX interface */
	if ((((uintptr_t)bgx->reg_base >> 24) & 0xf) == 0x8) {
		bgx->bgx_id = 2;
		bgx->is_rgx = true;
		for (lmac = 0; lmac < MAX_LMAC_PER_BGX; lmac++) {
			if (lmac == 0) {
				bgx->lmac[lmac].lmacid = 0;
				bgx->lmac[lmac].qlm = 0;
			} else {
				bgx->lmac[lmac].qlm = -1;
			}
		}
		xcv_init_hw();
		goto skip_qlm_config;
	}
#endif

	node = node_id(bgx->reg_base);
	bgx_idx = ((uintptr_t)bgx->reg_base >> 24) & 3;
	bgx->bgx_id = (node * MAX_BGX_PER_NODE) + bgx_idx;
	if (otx_is_soc(CN81XX))
		inc = 2;
	else if (otx_is_soc(CN83XX) && (bgx_idx == 2))
		inc = 2;

	for (lmac = 0; lmac < MAX_LMAC_PER_BGX; lmac += inc) {
		/* BGX3 (DLM4), has only 2 lanes */
		if (otx_is_soc(CN83XX) && bgx_idx == 3 && lmac >= 2)
			continue;
		qlm[lmac + 0] = get_qlm_for_bgx(node, bgx_idx, lmac);
		/* Each DLM has 2 lanes, configure both lanes with
		 * same qlm configuration
		 */
		if (inc == 2)
			qlm[lmac + 1] = qlm[lmac];
		debug("qlm[%d] = %d\n", lmac, qlm[lmac]);
	}

	/* A BGX can take 1 or 2 DLMs, if both the DLMs are not configured
	 * as BGX, then return, nothing to initialize
	 */
	if (otx_is_soc(CN81XX))
		if ((qlm[0] == -1) && (qlm[2] == -1))
			return -ENODEV;

	/* MAP configuration registers */
	for (lmac = 0; lmac < MAX_LMAC_PER_BGX; lmac++) {
		bgx->lmac[lmac].qlm = qlm[lmac];
		bgx->lmac[lmac].lmacid = lmac;
	}

#ifdef OCTEONTX_XCV
skip_qlm_config:
#endif
	bgx_vnic[bgx->bgx_id] = bgx;
	bgx_get_qlm_mode(bgx);
	debug("bgx_vnic[%u]: %p\n", bgx->bgx_id, bgx);

	bgx_init_hw(bgx);

	/* Init LMACs */
	for (lmac = 0; lmac < bgx->lmac_count; lmac++) {
		struct lmac *tlmac = &bgx->lmac[lmac];

		tlmac->dev = dev;
		err = bgx_lmac_enable(bgx, lmac);
		if (err) {
			printf("BGX%d failed to enable lmac%d\n",
			       bgx->bgx_id, lmac);
		}
	}

	return 0;
}

U_BOOT_DRIVER(octeontx_bgx) = {
	.name	= "octeontx_bgx",
	.id	= UCLASS_MISC,
	.probe	= octeontx_bgx_probe,
	.remove	= octeontx_bgx_remove,
	.priv_auto	= sizeof(struct bgx),
	.flags  = DM_FLAG_OS_PREPARE,
};

static struct pci_device_id octeontx_bgx_supported[] = {
	{ PCI_VDEVICE(CAVIUM, PCI_DEVICE_ID_CAVIUM_BGX) },
	{ PCI_VDEVICE(CAVIUM, PCI_DEVICE_ID_CAVIUM_RGX) },
	{}
};

U_BOOT_PCI_DEVICE(octeontx_bgx, octeontx_bgx_supported);
