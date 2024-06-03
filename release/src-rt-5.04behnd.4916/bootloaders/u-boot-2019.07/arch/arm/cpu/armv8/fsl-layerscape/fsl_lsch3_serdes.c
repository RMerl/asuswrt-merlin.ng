// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2016-2018 NXP
 * Copyright 2014-2015 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch/soc.h>
#include <fsl-mc/ldpaa_wriop.h>

#ifdef CONFIG_SYS_FSL_SRDS_1
static u8 serdes1_prtcl_map[SERDES_PRCTL_COUNT];
#endif
#ifdef CONFIG_SYS_FSL_SRDS_2
static u8 serdes2_prtcl_map[SERDES_PRCTL_COUNT];
#endif
#ifdef CONFIG_SYS_NXP_SRDS_3
static u8 serdes3_prtcl_map[SERDES_PRCTL_COUNT];
#endif

#if defined(CONFIG_FSL_MC_ENET) && !defined(CONFIG_SPL_BUILD)
#ifdef CONFIG_ARCH_LX2160A
int xfi_dpmac[XFI14 + 1];
int sgmii_dpmac[SGMII18 + 1];
int a25gaui_dpmac[_25GE10 + 1];
int xlaui_dpmac[_40GE2 + 1];
int caui2_dpmac[_50GE2 + 1];
int caui4_dpmac[_100GE2 + 1];
#else
int xfi_dpmac[XFI8 + 1];
int sgmii_dpmac[SGMII16 + 1];
#endif
#endif

__weak void wriop_init_dpmac_qsgmii(int sd, int lane_prtcl)
{
	return;
}

/*
 *The return value of this func is the serdes protocol used.
 *Typically this function is called number of times depending
 *upon the number of serdes blocks in the Silicon.
 *Zero is used to denote that no serdes was enabled,
 *this is the case when golden RCW was used where DPAA2 bring was
 *intentionally removed to achieve boot to prompt
*/

__weak int serdes_get_number(int serdes, int cfg)
{
	return cfg;
}

int is_serdes_configured(enum srds_prtcl device)
{
	int ret = 0;

#ifdef CONFIG_SYS_FSL_SRDS_1
	if (!serdes1_prtcl_map[NONE])
		fsl_serdes_init();

	ret |= serdes1_prtcl_map[device];
#endif
#ifdef CONFIG_SYS_FSL_SRDS_2
	if (!serdes2_prtcl_map[NONE])
		fsl_serdes_init();

	ret |= serdes2_prtcl_map[device];
#endif
#ifdef CONFIG_SYS_NXP_SRDS_3
	if (!serdes3_prtcl_map[NONE])
		fsl_serdes_init();

	ret |= serdes3_prtcl_map[device];
#endif

	return !!ret;
}

int serdes_get_first_lane(u32 sd, enum srds_prtcl device)
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	u32 cfg = 0;
	int i;

	switch (sd) {
#ifdef CONFIG_SYS_FSL_SRDS_1
	case FSL_SRDS_1:
		cfg = gur_in32(&gur->rcwsr[FSL_CHASSIS3_SRDS1_REGSR - 1]);
		cfg &= FSL_CHASSIS3_SRDS1_PRTCL_MASK;
		cfg >>= FSL_CHASSIS3_SRDS1_PRTCL_SHIFT;
		break;
#endif
#ifdef CONFIG_SYS_FSL_SRDS_2
	case FSL_SRDS_2:
		cfg = gur_in32(&gur->rcwsr[FSL_CHASSIS3_SRDS2_REGSR - 1]);
		cfg &= FSL_CHASSIS3_SRDS2_PRTCL_MASK;
		cfg >>= FSL_CHASSIS3_SRDS2_PRTCL_SHIFT;
		break;
#endif
#ifdef CONFIG_SYS_NXP_SRDS_3
	case NXP_SRDS_3:
		cfg = gur_in32(&gur->rcwsr[FSL_CHASSIS3_SRDS3_REGSR - 1]);
		cfg &= FSL_CHASSIS3_SRDS3_PRTCL_MASK;
		cfg >>= FSL_CHASSIS3_SRDS3_PRTCL_SHIFT;
		break;
#endif
	default:
		printf("invalid SerDes%d\n", sd);
		break;
	}

	cfg = serdes_get_number(sd, cfg);

	/* Is serdes enabled at all? */
	if (cfg == 0)
		return -ENODEV;

	for (i = 0; i < SRDS_MAX_LANES; i++) {
		if (serdes_get_prtcl(sd, cfg, i) == device)
			return i;
	}

	return -ENODEV;
}

void serdes_init(u32 sd, u32 sd_addr, u32 rcwsr, u32 sd_prctl_mask,
		 u32 sd_prctl_shift, u8 serdes_prtcl_map[SERDES_PRCTL_COUNT])
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	u32 cfg;
	int lane;

	if (serdes_prtcl_map[NONE])
		return;

	memset(serdes_prtcl_map, 0, sizeof(u8) * SERDES_PRCTL_COUNT);

	cfg = gur_in32(&gur->rcwsr[rcwsr - 1]) & sd_prctl_mask;
	cfg >>= sd_prctl_shift;

	cfg = serdes_get_number(sd, cfg);
	printf("Using SERDES%d Protocol: %d (0x%x)\n", sd + 1, cfg, cfg);

	if (!is_serdes_prtcl_valid(sd, cfg))
		printf("SERDES%d[PRTCL] = 0x%x is not valid\n", sd + 1, cfg);

	for (lane = 0; lane < SRDS_MAX_LANES; lane++) {
		enum srds_prtcl lane_prtcl = serdes_get_prtcl(sd, cfg, lane);
		if (unlikely(lane_prtcl >= SERDES_PRCTL_COUNT))
			debug("Unknown SerDes lane protocol %d\n", lane_prtcl);
		else {
			serdes_prtcl_map[lane_prtcl] = 1;
#if defined(CONFIG_FSL_MC_ENET) && !defined(CONFIG_SPL_BUILD)
#ifdef CONFIG_ARCH_LX2160A
			if (lane_prtcl >= XFI1 && lane_prtcl <= XFI14)
				wriop_init_dpmac(sd, xfi_dpmac[lane_prtcl],
						 (int)lane_prtcl);

			if (lane_prtcl >= SGMII1 && lane_prtcl <= SGMII18)
				wriop_init_dpmac(sd, sgmii_dpmac[lane_prtcl],
						 (int)lane_prtcl);

			if (lane_prtcl >= _25GE1 && lane_prtcl <= _25GE10)
				wriop_init_dpmac(sd, a25gaui_dpmac[lane_prtcl],
						 (int)lane_prtcl);

			if (lane_prtcl >= _40GE1 && lane_prtcl <= _40GE2)
				wriop_init_dpmac(sd, xlaui_dpmac[lane_prtcl],
						 (int)lane_prtcl);

			if (lane_prtcl >= _50GE1 && lane_prtcl <= _50GE2)
				wriop_init_dpmac(sd, caui2_dpmac[lane_prtcl],
						 (int)lane_prtcl);

			if (lane_prtcl >= _100GE1 && lane_prtcl <= _100GE2)
				wriop_init_dpmac(sd, caui4_dpmac[lane_prtcl],
						 (int)lane_prtcl);

#else
			switch (lane_prtcl) {
			case QSGMII_A:
			case QSGMII_B:
			case QSGMII_C:
			case QSGMII_D:
				wriop_init_dpmac_qsgmii(sd, (int)lane_prtcl);
				break;
			default:
				if (lane_prtcl >= XFI1 && lane_prtcl <= XFI8)
					wriop_init_dpmac(sd,
							 xfi_dpmac[lane_prtcl],
							 (int)lane_prtcl);

				 if (lane_prtcl >= SGMII1 &&
				     lane_prtcl <= SGMII16)
					wriop_init_dpmac(sd, sgmii_dpmac[
							 lane_prtcl],
							 (int)lane_prtcl);
				break;
			}
#endif
#endif
		}
	}

	/* Set the first element to indicate serdes has been initialized */
	serdes_prtcl_map[NONE] = 1;
}

__weak int get_serdes_volt(void)
{
	return -1;
}

__weak int set_serdes_volt(int svdd)
{
	return -1;
}

#define LNAGCR0_RT_RSTB		0x00600000

#define RSTCTL_RESET_MASK	0x000000E0

#define RSTCTL_RSTREQ		0x80000000
#define RSTCTL_RST_DONE		0x40000000
#define RSTCTL_RSTERR		0x20000000

#define RSTCTL_SDEN		0x00000020
#define RSTCTL_SDRST_B		0x00000040
#define RSTCTL_PLLRST_B		0x00000080

#define TCALCR_CALRST_B		0x08000000

struct serdes_prctl_info {
	u32 id;
	u32 mask;
	u32 shift;
};

struct serdes_prctl_info srds_prctl_info[] = {
#ifdef CONFIG_SYS_FSL_SRDS_1
	{.id = 1,
	 .mask = FSL_CHASSIS3_SRDS1_PRTCL_MASK,
	 .shift = FSL_CHASSIS3_SRDS1_PRTCL_SHIFT
	},

#endif
#ifdef CONFIG_SYS_FSL_SRDS_2
	{.id = 2,
	 .mask = FSL_CHASSIS3_SRDS2_PRTCL_MASK,
	 .shift = FSL_CHASSIS3_SRDS2_PRTCL_SHIFT
	},
#endif
#ifdef CONFIG_SYS_NXP_SRDS_3
	{.id = 3,
	 .mask = FSL_CHASSIS3_SRDS3_PRTCL_MASK,
	 .shift = FSL_CHASSIS3_SRDS3_PRTCL_SHIFT
	},
#endif
	{} /* NULL ENTRY */
};

static int get_serdes_prctl_info_idx(u32 serdes_id)
{
	int pos = 0;
	struct serdes_prctl_info *srds_info;

	/* loop until NULL ENTRY defined by .id=0 */
	for (srds_info = srds_prctl_info; srds_info->id != 0;
	     srds_info++, pos++) {
		if (srds_info->id == serdes_id)
			return pos;
	}

	return -1;
}

static void do_enabled_lanes_reset(u32 serdes_id, u32 cfg,
				   struct ccsr_serdes __iomem *serdes_base,
				   bool cmplt)
{
	int i, pos;
	u32 cfg_tmp;

	pos = get_serdes_prctl_info_idx(serdes_id);
	if (pos == -1) {
		printf("invalid serdes_id %d\n", serdes_id);
		return;
	}

	cfg_tmp = cfg & srds_prctl_info[pos].mask;
	cfg_tmp >>= srds_prctl_info[pos].shift;

	for (i = 0; i < 4 && cfg_tmp & (0xf << (3 - i)); i++) {
		if (cmplt)
			setbits_le32(&serdes_base->lane[i].gcr0,
				     LNAGCR0_RT_RSTB);
		else
			clrbits_le32(&serdes_base->lane[i].gcr0,
				     LNAGCR0_RT_RSTB);
	}
}

static void do_pll_reset(u32 cfg,
			 struct ccsr_serdes __iomem *serdes_base)
{
	int i;

	for (i = 0; i < 2 && !(cfg & (0x1 << (1 - i))); i++) {
		clrbits_le32(&serdes_base->bank[i].rstctl,
			     RSTCTL_RESET_MASK);
		udelay(1);

		setbits_le32(&serdes_base->bank[i].rstctl,
			     RSTCTL_RSTREQ);
	}
	udelay(1);
}

static void do_rx_tx_cal_reset(struct ccsr_serdes __iomem *serdes_base)
{
	clrbits_le32(&serdes_base->srdstcalcr, TCALCR_CALRST_B);
	clrbits_le32(&serdes_base->srdstcalcr, TCALCR_CALRST_B);
}

static void do_rx_tx_cal_reset_comp(u32 cfg, int i,
				    struct ccsr_serdes __iomem *serdes_base)
{
	if (!(cfg == 0x3 && i == 1)) {
		udelay(1);
		setbits_le32(&serdes_base->srdstcalcr, TCALCR_CALRST_B);
		setbits_le32(&serdes_base->srdstcalcr, TCALCR_CALRST_B);
	}
	udelay(1);
}

static void do_pll_reset_done(u32 cfg,
			      struct ccsr_serdes __iomem *serdes_base)
{
	int i;
	u32 reg = 0;

	for (i = 0; i < 2; i++) {
		reg = in_le32(&serdes_base->bank[i].pllcr0);
		if (!(cfg & (0x1 << (1 - i))) && ((reg >> 23) & 0x1)) {
			setbits_le32(&serdes_base->bank[i].rstctl,
				     RSTCTL_RST_DONE);
		}
	}
}

static void do_serdes_enable(u32 cfg,
			     struct ccsr_serdes __iomem *serdes_base)
{
	int i;

	for (i = 0; i < 2 && !(cfg & (0x1 << (1 - i))); i++) {
		setbits_le32(&serdes_base->bank[i].rstctl, RSTCTL_SDEN);
		udelay(1);

		setbits_le32(&serdes_base->bank[i].rstctl, RSTCTL_PLLRST_B);
		udelay(1);
		/* Take the Rx/Tx calibration out of reset */
		do_rx_tx_cal_reset_comp(cfg, i, serdes_base);
	}
}

static void do_pll_lock(u32 cfg,
			struct ccsr_serdes __iomem *serdes_base)
{
	int i;
	u32 reg = 0;

	for (i = 0; i < 2 && !(cfg & (0x1 << (1 - i))); i++) {
		/* if the PLL is not locked, set RST_ERR */
		reg = in_le32(&serdes_base->bank[i].pllcr0);
		if (!((reg >> 23) & 0x1)) {
			setbits_le32(&serdes_base->bank[i].rstctl,
				     RSTCTL_RSTERR);
		} else {
			udelay(1);
			setbits_le32(&serdes_base->bank[i].rstctl,
				     RSTCTL_SDRST_B);
			udelay(1);
		}
	}
}

int setup_serdes_volt(u32 svdd)
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	struct ccsr_serdes __iomem *serdes1_base =
			(void *)CONFIG_SYS_FSL_LSCH3_SERDES_ADDR;
	u32 cfg_rcwsrds1 = gur_in32(&gur->rcwsr[FSL_CHASSIS3_SRDS1_REGSR - 1]);
#ifdef CONFIG_SYS_FSL_SRDS_2
	struct ccsr_serdes __iomem *serdes2_base =
			(void *)(CONFIG_SYS_FSL_LSCH3_SERDES_ADDR + 0x10000);
	u32 cfg_rcwsrds2 = gur_in32(&gur->rcwsr[FSL_CHASSIS3_SRDS2_REGSR - 1]);
#endif
#ifdef CONFIG_SYS_NXP_SRDS_3
	struct ccsr_serdes __iomem *serdes3_base =
			(void *)(CONFIG_SYS_FSL_LSCH3_SERDES_ADDR + 0x20000);
	u32 cfg_rcwsrds3 = gur_in32(&gur->rcwsr[FSL_CHASSIS3_SRDS3_REGSR - 1]);
#endif
	u32 cfg_tmp;
	int svdd_cur, svdd_tar;
	int ret = 1;

	/* Only support switch SVDD to 900mV */
	if (svdd != 900)
		return -EINVAL;

	/* Scale up to the LTC resolution is 1/4096V */
	svdd = (svdd * 4096) / 1000;

	svdd_tar = svdd;
	svdd_cur = get_serdes_volt();
	if (svdd_cur < 0)
		return -EINVAL;

	debug("%s: current SVDD: %x; target SVDD: %x\n",
	      __func__, svdd_cur, svdd_tar);
	if (svdd_cur == svdd_tar)
		return 0;

	/* Put the all enabled lanes in reset */
#ifdef CONFIG_SYS_FSL_SRDS_1
	do_enabled_lanes_reset(1, cfg_rcwsrds1, serdes1_base, false);
#endif

#ifdef CONFIG_SYS_FSL_SRDS_2
	do_enabled_lanes_reset(2, cfg_rcwsrds2, serdes2_base, false);
#endif
#ifdef CONFIG_SYS_NXP_SRDS_3
	do_enabled_lanes_reset(3, cfg_rcwsrds3, serdes3_base, false);
#endif

	/* Put the all enabled PLL in reset */
#ifdef CONFIG_SYS_FSL_SRDS_1
	cfg_tmp = cfg_rcwsrds1 & 0x3;
	do_pll_reset(cfg_tmp, serdes1_base);
#endif

#ifdef CONFIG_SYS_FSL_SRDS_2
	cfg_tmp = cfg_rcwsrds1 & 0xC;
	cfg_tmp >>= 2;
	do_pll_reset(cfg_tmp, serdes2_base);
#endif

#ifdef CONFIG_SYS_NXP_SRDS_3
	cfg_tmp = cfg_rcwsrds3 & 0x30;
	cfg_tmp >>= 4;
	do_pll_reset(cfg_tmp, serdes3_base);
#endif

	/* Put the Rx/Tx calibration into reset */
#ifdef CONFIG_SYS_FSL_SRDS_1
	do_rx_tx_cal_reset(serdes1_base);
#endif

#ifdef CONFIG_SYS_FSL_SRDS_2
	do_rx_tx_cal_reset(serdes2_base);
#endif

#ifdef CONFIG_SYS_NXP_SRDS_3
	do_rx_tx_cal_reset(serdes3_base);
#endif

	ret = set_serdes_volt(svdd);
	if (ret < 0) {
		printf("could not change SVDD\n");
		ret = -1;
	}

	/* For each PLL that’s not disabled via RCW enable the SERDES */
#ifdef CONFIG_SYS_FSL_SRDS_1
	cfg_tmp = cfg_rcwsrds1 & 0x3;
	do_serdes_enable(cfg_tmp, serdes1_base);
#endif
#ifdef CONFIG_SYS_FSL_SRDS_2
	cfg_tmp = cfg_rcwsrds1 & 0xC;
	cfg_tmp >>= 2;
	do_serdes_enable(cfg_tmp, serdes2_base);
#endif
#ifdef CONFIG_SYS_NXP_SRDS_3
	cfg_tmp = cfg_rcwsrds3 & 0x30;
	cfg_tmp >>= 4;
	do_serdes_enable(cfg_tmp, serdes3_base);
#endif

	/* Wait for at at least 625us, ensure the PLLs being reset are locked */
	udelay(800);

#ifdef CONFIG_SYS_FSL_SRDS_1
	cfg_tmp = cfg_rcwsrds1 & 0x3;
	do_pll_lock(cfg_tmp, serdes1_base);
#endif

#ifdef CONFIG_SYS_FSL_SRDS_2
	cfg_tmp = cfg_rcwsrds1 & 0xC;
	cfg_tmp >>= 2;
	do_pll_lock(cfg_tmp, serdes2_base);
#endif

#ifdef CONFIG_SYS_NXP_SRDS_3
	cfg_tmp = cfg_rcwsrds3 & 0x30;
	cfg_tmp >>= 4;
	do_pll_lock(cfg_tmp, serdes3_base);
#endif

	/* Take the all enabled lanes out of reset */
#ifdef CONFIG_SYS_FSL_SRDS_1
	do_enabled_lanes_reset(1, cfg_rcwsrds1, serdes1_base, true);
#endif
#ifdef CONFIG_SYS_FSL_SRDS_2
	do_enabled_lanes_reset(2, cfg_rcwsrds2, serdes2_base, true);
#endif

#ifdef CONFIG_SYS_NXP_SRDS_3
	do_enabled_lanes_reset(3, cfg_rcwsrds3, serdes3_base, true);
#endif

	/* For each PLL being reset, and achieved PLL lock set RST_DONE */
#ifdef CONFIG_SYS_FSL_SRDS_1
	cfg_tmp = cfg_rcwsrds1 & 0x3;
	do_pll_reset_done(cfg_tmp, serdes1_base);
#endif
#ifdef CONFIG_SYS_FSL_SRDS_2
	cfg_tmp = cfg_rcwsrds1 & 0xC;
	cfg_tmp >>= 2;
	do_pll_reset_done(cfg_tmp, serdes2_base);
#endif

#ifdef CONFIG_SYS_NXP_SRDS_3
	cfg_tmp = cfg_rcwsrds3 & 0x30;
	cfg_tmp >>= 4;
	do_pll_reset_done(cfg_tmp, serdes3_base);
#endif

	return ret;
}

void fsl_serdes_init(void)
{
#if defined(CONFIG_FSL_MC_ENET) && !defined(CONFIG_SPL_BUILD)
	int i , j;

#ifdef CONFIG_ARCH_LX2160A
	for (i = XFI1, j = 1; i <= XFI14; i++, j++)
		xfi_dpmac[i] = j;

	for (i = SGMII1, j = 1; i <= SGMII18; i++, j++)
		sgmii_dpmac[i] = j;

	for (i = _25GE1, j = 1; i <= _25GE10; i++, j++)
		a25gaui_dpmac[i] = j;

	for (i = _40GE1, j = 1; i <= _40GE2; i++, j++)
		xlaui_dpmac[i] = j;

	for (i = _50GE1, j = 1; i <= _50GE2; i++, j++)
		caui2_dpmac[i] = j;

	for (i = _100GE1, j = 1; i <= _100GE2; i++, j++)
		caui4_dpmac[i] = j;
#else
	for (i = XFI1, j = 1; i <= XFI8; i++, j++)
		xfi_dpmac[i] = j;

	for (i = SGMII1, j = 1; i <= SGMII16; i++, j++)
		sgmii_dpmac[i] = j;
#endif
#endif

#ifdef CONFIG_SYS_FSL_SRDS_1
	serdes_init(FSL_SRDS_1,
		    CONFIG_SYS_FSL_LSCH3_SERDES_ADDR,
		    FSL_CHASSIS3_SRDS1_REGSR,
		    FSL_CHASSIS3_SRDS1_PRTCL_MASK,
		    FSL_CHASSIS3_SRDS1_PRTCL_SHIFT,
		    serdes1_prtcl_map);
#endif
#ifdef CONFIG_SYS_FSL_SRDS_2
	serdes_init(FSL_SRDS_2,
		    CONFIG_SYS_FSL_LSCH3_SERDES_ADDR + FSL_SRDS_2 * 0x10000,
		    FSL_CHASSIS3_SRDS2_REGSR,
		    FSL_CHASSIS3_SRDS2_PRTCL_MASK,
		    FSL_CHASSIS3_SRDS2_PRTCL_SHIFT,
		    serdes2_prtcl_map);
#endif
#ifdef CONFIG_SYS_NXP_SRDS_3
	serdes_init(NXP_SRDS_3,
		    CONFIG_SYS_FSL_LSCH3_SERDES_ADDR + NXP_SRDS_3 * 0x10000,
		    FSL_CHASSIS3_SRDS3_REGSR,
		    FSL_CHASSIS3_SRDS3_PRTCL_MASK,
		    FSL_CHASSIS3_SRDS3_PRTCL_SHIFT,
		    serdes3_prtcl_map);
#endif
}
