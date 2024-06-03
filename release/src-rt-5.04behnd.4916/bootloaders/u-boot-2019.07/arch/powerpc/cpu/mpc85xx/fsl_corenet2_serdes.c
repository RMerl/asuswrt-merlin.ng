// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2012 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/fsl_serdes.h>
#include <asm/immap_85xx.h>
#include <asm/io.h>
#include <asm/processor.h>
#include <asm/fsl_law.h>
#include <linux/errno.h>
#include <fsl_errata.h>
#include "fsl_corenet2_serdes.h"

#ifdef CONFIG_SYS_FSL_SRDS_1
static u8 serdes1_prtcl_map[SERDES_PRCTL_COUNT];
#endif
#ifdef CONFIG_SYS_FSL_SRDS_2
static u8 serdes2_prtcl_map[SERDES_PRCTL_COUNT];
#endif
#ifdef CONFIG_SYS_FSL_SRDS_3
static u8 serdes3_prtcl_map[SERDES_PRCTL_COUNT];
#endif
#ifdef CONFIG_SYS_FSL_SRDS_4
static u8 serdes4_prtcl_map[SERDES_PRCTL_COUNT];
#endif

#ifdef DEBUG
static const char *serdes_prtcl_str[] = {
	[NONE] = "NA",
	[PCIE1] = "PCIE1",
	[PCIE2] = "PCIE2",
	[PCIE3] = "PCIE3",
	[PCIE4] = "PCIE4",
	[SATA1] = "SATA1",
	[SATA2] = "SATA2",
	[SRIO1] = "SRIO1",
	[SRIO2] = "SRIO2",
	[SGMII_FM1_DTSEC1] = "SGMII_FM1_DTSEC1",
	[SGMII_FM1_DTSEC2] = "SGMII_FM1_DTSEC2",
	[SGMII_FM1_DTSEC3] = "SGMII_FM1_DTSEC3",
	[SGMII_FM1_DTSEC4] = "SGMII_FM1_DTSEC4",
	[SGMII_FM1_DTSEC5] = "SGMII_FM1_DTSEC5",
	[SGMII_FM1_DTSEC6] = "SGMII_FM1_DTSEC6",
	[SGMII_FM2_DTSEC1] = "SGMII_FM2_DTSEC1",
	[SGMII_FM2_DTSEC2] = "SGMII_FM2_DTSEC2",
	[SGMII_FM2_DTSEC3] = "SGMII_FM2_DTSEC3",
	[SGMII_FM2_DTSEC4] = "SGMII_FM2_DTSEC4",
	[XAUI_FM1] = "XAUI_FM1",
	[XAUI_FM2] = "XAUI_FM2",
	[AURORA] = "DEBUG",
	[CPRI1] = "CPRI1",
	[CPRI2] = "CPRI2",
	[CPRI3] = "CPRI3",
	[CPRI4] = "CPRI4",
	[CPRI5] = "CPRI5",
	[CPRI6] = "CPRI6",
	[CPRI7] = "CPRI7",
	[CPRI8] = "CPRI8",
	[XAUI_FM1_MAC9] = "XAUI_FM1_MAC9",
	[XAUI_FM1_MAC10] = "XAUI_FM1_MAC10",
	[XAUI_FM2_MAC9] = "XAUI_FM2_MAC9",
	[XAUI_FM2_MAC10] = "XAUI_FM2_MAC10",
	[HIGIG_FM1_MAC9] = "HiGig_FM1_MAC9",
	[HIGIG_FM1_MAC10] = "HiGig_FM1_MAC10",
	[HIGIG_FM2_MAC9] = "HiGig_FM2_MAC9",
	[HIGIG_FM2_MAC10] = "HiGig_FM2_MAC10",
	[QSGMII_FM1_A] = "QSGMII_FM1_A",
	[QSGMII_FM1_B] = "QSGMII_FM1_B",
	[QSGMII_FM2_A] = "QSGMII_FM2_A",
	[QSGMII_FM2_B] = "QSGMII_FM2_B",
	[XFI_FM1_MAC9] = "XFI_FM1_MAC9",
	[XFI_FM1_MAC10] = "XFI_FM1_MAC10",
	[XFI_FM2_MAC9] = "XFI_FM2_MAC9",
	[XFI_FM2_MAC10] = "XFI_FM2_MAC10",
	[INTERLAKEN] = "INTERLAKEN",
	[QSGMII_SW1_A] = "QSGMII_SW1_A",
	[QSGMII_SW1_B] = "QSGMII_SW1_B",
	[SGMII_SW1_MAC1] = "SGMII_SW1_MAC1",
	[SGMII_SW1_MAC2] = "SGMII_SW1_MAC2",
	[SGMII_SW1_MAC3] = "SGMII_SW1_MAC3",
	[SGMII_SW1_MAC4] = "SGMII_SW1_MAC4",
	[SGMII_SW1_MAC5] = "SGMII_SW1_MAC5",
	[SGMII_SW1_MAC6] = "SGMII_SW1_MAC6",
};
#endif

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
#ifdef CONFIG_SYS_FSL_SRDS_3
	if (!serdes3_prtcl_map[NONE])
		fsl_serdes_init();

	ret |= serdes3_prtcl_map[device];
#endif
#ifdef CONFIG_SYS_FSL_SRDS_4
	if (!serdes4_prtcl_map[NONE])
		fsl_serdes_init();

	ret |= serdes4_prtcl_map[device];
#endif

	return !!ret;
}

int serdes_get_first_lane(u32 sd, enum srds_prtcl device)
{
	const ccsr_gur_t *gur = (void __iomem *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u32 cfg = in_be32(&gur->rcwsr[4]);
	int i;

	switch (sd) {
#ifdef CONFIG_SYS_FSL_SRDS_1
	case FSL_SRDS_1:
		cfg &= FSL_CORENET2_RCWSR4_SRDS1_PRTCL;
		cfg >>= FSL_CORENET2_RCWSR4_SRDS1_PRTCL_SHIFT;
		break;
#endif
#ifdef CONFIG_SYS_FSL_SRDS_2
	case FSL_SRDS_2:
		cfg &= FSL_CORENET2_RCWSR4_SRDS2_PRTCL;
		cfg >>= FSL_CORENET2_RCWSR4_SRDS2_PRTCL_SHIFT;
		break;
#endif
#ifdef CONFIG_SYS_FSL_SRDS_3
	case FSL_SRDS_3:
		cfg &= FSL_CORENET2_RCWSR4_SRDS3_PRTCL;
		cfg >>= FSL_CORENET2_RCWSR4_SRDS3_PRTCL_SHIFT;
		break;
#endif
#ifdef CONFIG_SYS_FSL_SRDS_4
	case FSL_SRDS_4:
		cfg &= FSL_CORENET2_RCWSR4_SRDS4_PRTCL;
		cfg >>= FSL_CORENET2_RCWSR4_SRDS4_PRTCL_SHIFT;
		break;
#endif
	default:
		printf("invalid SerDes%d\n", sd);
		break;
	}
	/* Is serdes enabled at all? */
	if (unlikely(cfg == 0))
		return -ENODEV;

	for (i = 0; i < SRDS_MAX_LANES; i++) {
		if (serdes_get_prtcl(sd, cfg, i) == device)
			return i;
	}

	return -ENODEV;
}

#define BC3_SHIFT	9
#define DC3_SHIFT	6
#define FC3_SHIFT	0
#define BC2_SHIFT	19
#define DC2_SHIFT	16
#define FC2_SHIFT	10
#define BC1_SHIFT	29
#define DC1_SHIFT	26
#define FC1_SHIFT	20
#define BC_MASK		0x1
#define DC_MASK		0x7
#define FC_MASK		0x3F

#define FUSE_VAL_MASK		0x00000003
#define FUSE_VAL_SHIFT		30
#define CR0_DCBIAS_SHIFT	5
#define CR1_FCAP_SHIFT		15
#define CR1_BCAP_SHIFT		29
#define FCAP_MASK		0x001F8000
#define BCAP_MASK		0x20000000
#define BCAP_OVD_MASK		0x10000000
#define BYP_CAL_MASK		0x02000000

void serdes_init(u32 sd, u32 sd_addr, u32 sd_prctl_mask, u32 sd_prctl_shift,
		u8 serdes_prtcl_map[SERDES_PRCTL_COUNT])
{
	ccsr_gur_t *gur = (void __iomem *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u32 cfg;
	int lane;

	if (serdes_prtcl_map[NONE])
		return;

	memset(serdes_prtcl_map, 0, sizeof(u8) * SERDES_PRCTL_COUNT);
#ifdef CONFIG_SYS_FSL_ERRATUM_A007186
	struct ccsr_sfp_regs  __iomem *sfp_regs =
			(struct ccsr_sfp_regs __iomem *)(CONFIG_SYS_SFP_ADDR);
	u32 pll_num, pll_status, bc, dc, fc, pll_cr_upd, pll_cr0, pll_cr1;
	u32 bc_status, fc_status, dc_status, pll_sr2;
	serdes_corenet_t  __iomem *srds_regs = (void *)sd_addr;
	u32 sfp_spfr0, sel;
#endif

	cfg = in_be32(&gur->rcwsr[4]) & sd_prctl_mask;

/* Erratum A-007186
 * Freescale Scratch Pad Fuse Register n (SFP_FSPFR0)
 * The workaround requires factory pre-set SerDes calibration values to be
 * read from a fuse block(Freescale Scratch Pad Fuse Register SFP_FSPFR0)
 * These values have been shown to work across the
 * entire temperature range for all SerDes. These values are then written into
 * the SerDes registers to calibrate the SerDes PLL.
 *
 * This workaround for the protocols and rates that only have the Ring VCO.
 */
#ifdef CONFIG_SYS_FSL_ERRATUM_A007186
	sfp_spfr0 = in_be32(&sfp_regs->fsl_spfr0);
	debug("A007186: sfp_spfr0= %x\n", sfp_spfr0);

	sel = (sfp_spfr0 >> FUSE_VAL_SHIFT) & FUSE_VAL_MASK;

	if (has_erratum_a007186() && (sel == 0x01 || sel == 0x02)) {
		for (pll_num = 0; pll_num < SRDS_MAX_BANK; pll_num++) {
			pll_status = in_be32(&srds_regs->bank[pll_num].pllcr0);
			debug("A007186: pll_num=%x pllcr0=%x\n",
			      pll_num, pll_status);
			/* STEP 1 */
			/* Read factory pre-set SerDes calibration values
			 * from fuse block(SFP scratch register-sfp_spfr0)
			 */
			switch (pll_status & SRDS_PLLCR0_FRATE_SEL_MASK) {
			case SRDS_PLLCR0_FRATE_SEL_3_0:
			case SRDS_PLLCR0_FRATE_SEL_3_072:
				debug("A007186: 3.0/3.072 protocol rate\n");
				bc = (sfp_spfr0 >> BC1_SHIFT) & BC_MASK;
				dc = (sfp_spfr0 >> DC1_SHIFT) & DC_MASK;
				fc = (sfp_spfr0 >> FC1_SHIFT) & FC_MASK;
				break;
			case SRDS_PLLCR0_FRATE_SEL_3_125:
				debug("A007186: 3.125 protocol rate\n");
				bc = (sfp_spfr0 >> BC2_SHIFT) & BC_MASK;
				dc = (sfp_spfr0 >> DC2_SHIFT) & DC_MASK;
				fc = (sfp_spfr0 >> FC2_SHIFT) & FC_MASK;
				break;
			case SRDS_PLLCR0_FRATE_SEL_3_75:
				debug("A007186: 3.75 protocol rate\n");
				bc = (sfp_spfr0 >> BC1_SHIFT) & BC_MASK;
				dc = (sfp_spfr0 >> DC1_SHIFT) & DC_MASK;
				fc = (sfp_spfr0 >> FC1_SHIFT) & FC_MASK;
				break;
			default:
				continue;
			}

			/* STEP 2 */
			/* Write SRDSxPLLnCR1[11:16] = FC
			 * Write SRDSxPLLnCR1[2] = BC
			 */
			pll_cr1 = in_be32(&srds_regs->bank[pll_num].pllcr1);
			pll_cr_upd = (((bc << CR1_BCAP_SHIFT) & BCAP_MASK) |
				      ((fc << CR1_FCAP_SHIFT) & FCAP_MASK));
			out_be32(&srds_regs->bank[pll_num].pllcr1,
				 (pll_cr_upd | pll_cr1));
			debug("A007186: pll_num=%x Updated PLLCR1=%x\n",
			      pll_num, (pll_cr_upd | pll_cr1));
			/* Write SRDSxPLLnCR0[24:26] = DC
			 */
			pll_cr0 = in_be32(&srds_regs->bank[pll_num].pllcr0);
			out_be32(&srds_regs->bank[pll_num].pllcr0,
				 pll_cr0 | (dc << CR0_DCBIAS_SHIFT));
			debug("A007186: pll_num=%x, Updated PLLCR0=%x\n",
			      pll_num, (pll_cr0 | (dc << CR0_DCBIAS_SHIFT)));
			/* Write SRDSxPLLnCR1[3] = 1
			 * Write SRDSxPLLnCR1[6] = 1
			 */
			pll_cr1 = in_be32(&srds_regs->bank[pll_num].pllcr1);
			pll_cr_upd = (BCAP_OVD_MASK | BYP_CAL_MASK);
			out_be32(&srds_regs->bank[pll_num].pllcr1,
				 (pll_cr_upd | pll_cr1));
			debug("A007186: pll_num=%x Updated PLLCR1=%x\n",
			      pll_num, (pll_cr_upd | pll_cr1));

			/* STEP 3 */
			/* Read the status Registers */
			/* Verify SRDSxPLLnSR2[8] = BC */
			pll_sr2 = in_be32(&srds_regs->bank[pll_num].pllsr2);
			debug("A007186: pll_num=%x pllsr2=%x\n",
			      pll_num, pll_sr2);
			bc_status = (pll_sr2 >> 23) & BC_MASK;
			if (bc_status != bc)
				debug("BC mismatch\n");
			fc_status = (pll_sr2 >> 16) & FC_MASK;
			if (fc_status != fc)
				debug("FC mismatch\n");
			pll_cr0 = in_be32(&srds_regs->bank[pll_num].pllcr0);
			out_be32(&srds_regs->bank[pll_num].pllcr0, pll_cr0 |
								0x02000000);
			pll_sr2 = in_be32(&srds_regs->bank[pll_num].pllsr2);
			dc_status = (pll_sr2 >> 17) & DC_MASK;
			if (dc_status != dc)
				debug("DC mismatch\n");
			pll_cr0 = in_be32(&srds_regs->bank[pll_num].pllcr0);
			out_be32(&srds_regs->bank[pll_num].pllcr0, pll_cr0 &
								0xfdffffff);

			/* STEP 4 */
			/* Wait 750us to verify the PLL is locked
			 * by checking SRDSxPLLnCR0[8] = 1.
			 */
			udelay(750);
			pll_status = in_be32(&srds_regs->bank[pll_num].pllcr0);
			debug("A007186: pll_num=%x pllcr0=%x\n",
			      pll_num, pll_status);

			if ((pll_status & SRDS_PLLCR0_PLL_LCK) == 0)
				printf("A007186 Serdes PLL not locked\n");
			else
				debug("A007186 Serdes PLL locked\n");
		}
	}
#endif

	cfg >>= sd_prctl_shift;
	printf("Using SERDES%d Protocol: %d (0x%x)\n", sd + 1, cfg, cfg);
	if (!is_serdes_prtcl_valid(sd, cfg))
		printf("SERDES%d[PRTCL] = 0x%x is not valid\n", sd + 1, cfg);

	for (lane = 0; lane < SRDS_MAX_LANES; lane++) {
		enum srds_prtcl lane_prtcl = serdes_get_prtcl(sd, cfg, lane);
		if (unlikely(lane_prtcl >= SERDES_PRCTL_COUNT))
			debug("Unknown SerDes lane protocol %d\n", lane_prtcl);
		else
			serdes_prtcl_map[lane_prtcl] = 1;
	}

	/* Set the first element to indicate serdes has been initialized */
	serdes_prtcl_map[NONE] = 1;
}

void fsl_serdes_init(void)
{

#ifdef CONFIG_SYS_FSL_SRDS_1
	serdes_init(FSL_SRDS_1,
		    CONFIG_SYS_FSL_CORENET_SERDES_ADDR,
		    FSL_CORENET2_RCWSR4_SRDS1_PRTCL,
		    FSL_CORENET2_RCWSR4_SRDS1_PRTCL_SHIFT,
		    serdes1_prtcl_map);
#endif
#ifdef CONFIG_SYS_FSL_SRDS_2
	serdes_init(FSL_SRDS_2,
		    CONFIG_SYS_FSL_CORENET_SERDES_ADDR + FSL_SRDS_2 * 0x1000,
		    FSL_CORENET2_RCWSR4_SRDS2_PRTCL,
		    FSL_CORENET2_RCWSR4_SRDS2_PRTCL_SHIFT,
		    serdes2_prtcl_map);
#endif
#ifdef CONFIG_SYS_FSL_SRDS_3
	serdes_init(FSL_SRDS_3,
		    CONFIG_SYS_FSL_CORENET_SERDES_ADDR + FSL_SRDS_3 * 0x1000,
		    FSL_CORENET2_RCWSR4_SRDS3_PRTCL,
		    FSL_CORENET2_RCWSR4_SRDS3_PRTCL_SHIFT,
		    serdes3_prtcl_map);
#endif
#ifdef CONFIG_SYS_FSL_SRDS_4
	serdes_init(FSL_SRDS_4,
		    CONFIG_SYS_FSL_CORENET_SERDES_ADDR + FSL_SRDS_4 * 0x1000,
		    FSL_CORENET2_RCWSR4_SRDS4_PRTCL,
		    FSL_CORENET2_RCWSR4_SRDS4_PRTCL_SHIFT,
		    serdes4_prtcl_map);
#endif

}

const char *serdes_clock_to_string(u32 clock)
{
	switch (clock) {
	case SRDS_PLLCR0_RFCK_SEL_100:
		return "100";
	case SRDS_PLLCR0_RFCK_SEL_125:
		return "125";
	case SRDS_PLLCR0_RFCK_SEL_156_25:
		return "156.25";
	case SRDS_PLLCR0_RFCK_SEL_161_13:
		return "161.1328123";
	default:
#if defined(CONFIG_TARGET_T4240QDS) || defined(CONFIG_TARGET_T4160QDS)
		return "???";
#else
		return "122.88";
#endif
	}
}

