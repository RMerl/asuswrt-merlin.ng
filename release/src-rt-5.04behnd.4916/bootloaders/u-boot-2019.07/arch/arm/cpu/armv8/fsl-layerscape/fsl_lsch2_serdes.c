// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch/soc.h>

#ifdef CONFIG_SYS_FSL_SRDS_1
static u8 serdes1_prtcl_map[SERDES_PRCTL_COUNT];
#endif
#ifdef CONFIG_SYS_FSL_SRDS_2
static u8 serdes2_prtcl_map[SERDES_PRCTL_COUNT];
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

	return !!ret;
}

int serdes_get_first_lane(u32 sd, enum srds_prtcl device)
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	u32 cfg = gur_in32(&gur->rcwsr[4]);
	int i;

	switch (sd) {
#ifdef CONFIG_SYS_FSL_SRDS_1
	case FSL_SRDS_1:
		cfg &= FSL_CHASSIS2_RCWSR4_SRDS1_PRTCL_MASK;
		cfg >>= FSL_CHASSIS2_RCWSR4_SRDS1_PRTCL_SHIFT;
		break;
#endif
#ifdef CONFIG_SYS_FSL_SRDS_2
	case FSL_SRDS_2:
		cfg &= FSL_CHASSIS2_RCWSR4_SRDS2_PRTCL_MASK;
		cfg >>= FSL_CHASSIS2_RCWSR4_SRDS2_PRTCL_SHIFT;
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

int get_serdes_protocol(void)
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	u32 cfg = gur_in32(&gur->rcwsr[4]) &
			  FSL_CHASSIS2_RCWSR4_SRDS1_PRTCL_MASK;
	cfg >>= FSL_CHASSIS2_RCWSR4_SRDS1_PRTCL_SHIFT;

	return cfg;
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
	default:
		return "100";
	}
}

void serdes_init(u32 sd, u32 sd_addr, u32 sd_prctl_mask, u32 sd_prctl_shift,
		 u8 serdes_prtcl_map[SERDES_PRCTL_COUNT])
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	u32 cfg;
	int lane;

	if (serdes_prtcl_map[NONE])
		return;

	memset(serdes_prtcl_map, 0, sizeof(u8) * SERDES_PRCTL_COUNT);

	cfg = gur_in32(&gur->rcwsr[4]) & sd_prctl_mask;
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

__weak int get_serdes_volt(void)
{
	return -1;
}

__weak int set_serdes_volt(int svdd)
{
	return -1;
}

int setup_serdes_volt(u32 svdd)
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	struct ccsr_serdes *serdes1_base;
#ifdef CONFIG_SYS_FSL_SRDS_2
	struct ccsr_serdes *serdes2_base;
#endif
	u32 cfg_rcw4 = gur_in32(&gur->rcwsr[4]);
	u32 cfg_rcw5 = gur_in32(&gur->rcwsr[5]);
	u32 cfg_tmp, reg = 0;
	int svdd_cur, svdd_tar;
	int ret;
	int i;

	/* Only support switch SVDD to 900mV/1000mV */
	if (svdd != 900 && svdd != 1000)
		return -EINVAL;

	svdd_tar = svdd;
	svdd_cur = get_serdes_volt();
	if (svdd_cur < 0)
		return -EINVAL;

	debug("%s: current SVDD: %dmV; target SVDD: %dmV\n",
	      __func__, svdd_cur, svdd_tar);
	if (svdd_cur == svdd_tar)
		return 0;

	serdes1_base = (void *)CONFIG_SYS_FSL_SERDES_ADDR;
#ifdef CONFIG_SYS_FSL_SRDS_2
	serdes2_base = (void *)serdes1_base + 0x10000;
#endif

	/* Put the all enabled lanes in reset */
#ifdef CONFIG_SYS_FSL_SRDS_1
	cfg_tmp = cfg_rcw4 & FSL_CHASSIS2_RCWSR4_SRDS1_PRTCL_MASK;
	cfg_tmp >>= FSL_CHASSIS2_RCWSR4_SRDS1_PRTCL_SHIFT;

	for (i = 0; i < 4 && cfg_tmp & (0xf << (3 - i)); i++) {
		reg = in_be32(&serdes1_base->lane[i].gcr0);
		reg &= 0xFF9FFFFF;
		out_be32(&serdes1_base->lane[i].gcr0, reg);
	}
#endif
#ifdef CONFIG_SYS_FSL_SRDS_2
	cfg_tmp = cfg_rcw4 & FSL_CHASSIS2_RCWSR4_SRDS2_PRTCL_MASK;
	cfg_tmp >>= FSL_CHASSIS2_RCWSR4_SRDS2_PRTCL_SHIFT;

	for (i = 0; i < 4 && cfg_tmp & (0xf << (3 - i)); i++) {
		reg = in_be32(&serdes2_base->lane[i].gcr0);
		reg &= 0xFF9FFFFF;
		out_be32(&serdes2_base->lane[i].gcr0, reg);
	}
#endif

	/* Put the all enabled PLL in reset */
#ifdef CONFIG_SYS_FSL_SRDS_1
	cfg_tmp = (cfg_rcw5 >> 22) & 0x3;
	for (i = 0; i < 2 && !(cfg_tmp & (0x1 << (1 - i))); i++) {
		reg = in_be32(&serdes1_base->bank[i].rstctl);
		reg &= 0xFFFFFFBF;
		reg |= 0x10000000;
		out_be32(&serdes1_base->bank[i].rstctl, reg);
		udelay(1);

		reg = in_be32(&serdes1_base->bank[i].rstctl);
		reg &= 0xFFFFFF1F;
		out_be32(&serdes1_base->bank[i].rstctl, reg);
	}
	udelay(1);
#endif

#ifdef CONFIG_SYS_FSL_SRDS_2
	cfg_tmp = (cfg_rcw5 >> 20) & 0x3;
	for (i = 0; i < 2 && !(cfg_tmp & (0x1 << (1 - i))); i++) {
		reg = in_be32(&serdes2_base->bank[i].rstctl);
		reg &= 0xFFFFFFBF;
		reg |= 0x10000000;
		out_be32(&serdes2_base->bank[i].rstctl, reg);
		udelay(1);

		reg = in_be32(&serdes2_base->bank[i].rstctl);
		reg &= 0xFFFFFF1F;
		out_be32(&serdes2_base->bank[i].rstctl, reg);
	}
	udelay(1);
#endif

	/* Put the Rx/Tx calibration into reset */
#ifdef CONFIG_SYS_FSL_SRDS_1
	reg = in_be32(&serdes1_base->srdstcalcr);
	reg &= 0xF7FFFFFF;
	out_be32(&serdes1_base->srdstcalcr, reg);
	reg = in_be32(&serdes1_base->srdsrcalcr);
	reg &= 0xF7FFFFFF;
	out_be32(&serdes1_base->srdsrcalcr, reg);

#endif
#ifdef CONFIG_SYS_FSL_SRDS_2
	reg = in_be32(&serdes2_base->srdstcalcr);
	reg &= 0xF7FFFFFF;
	out_be32(&serdes2_base->srdstcalcr, reg);
	reg = in_be32(&serdes2_base->srdsrcalcr);
	reg &= 0xF7FFFFFF;
	out_be32(&serdes2_base->srdsrcalcr, reg);
#endif

	/*
	 * If SVDD set failed, will not return directly, so that the
	 * serdes lanes can complete reseting.
	 */
	ret = set_serdes_volt(svdd_tar);
	if (ret)
		printf("%s: Failed to set SVDD\n", __func__);

	/* Wait for SVDD to stabilize */
	udelay(100);

	/* For each PLL that’s not disabled via RCW */
#ifdef CONFIG_SYS_FSL_SRDS_1
	cfg_tmp = (cfg_rcw5 >> 22) & 0x3;
	for (i = 0; i < 2 && !(cfg_tmp & (0x1 << (1 - i))); i++) {
		reg = in_be32(&serdes1_base->bank[i].rstctl);
		reg |= 0x00000020;
		out_be32(&serdes1_base->bank[i].rstctl, reg);
		udelay(1);

		reg = in_be32(&serdes1_base->bank[i].rstctl);
		reg |= 0x00000080;
		out_be32(&serdes1_base->bank[i].rstctl, reg);

		/* Take the Rx/Tx calibration out of reset */
		if (!(cfg_tmp == 0x3 && i == 1)) {
			udelay(1);
			reg = in_be32(&serdes1_base->srdstcalcr);
			reg |= 0x08000000;
			out_be32(&serdes1_base->srdstcalcr, reg);
			reg = in_be32(&serdes1_base->srdsrcalcr);
			reg |= 0x08000000;
			out_be32(&serdes1_base->srdsrcalcr, reg);
		}
	}
	udelay(1);
#endif

#ifdef CONFIG_SYS_FSL_SRDS_2
	cfg_tmp = (cfg_rcw5 >> 20) & 0x3;
	for (i = 0; i < 2 && !(cfg_tmp & (0x1 << (1 - i))); i++) {
		reg = in_be32(&serdes2_base->bank[i].rstctl);
		reg |= 0x00000020;
		out_be32(&serdes2_base->bank[i].rstctl, reg);
		udelay(1);

		reg = in_be32(&serdes2_base->bank[i].rstctl);
		reg |= 0x00000080;
		out_be32(&serdes2_base->bank[i].rstctl, reg);

		/* Take the Rx/Tx calibration out of reset */
		if (!(cfg_tmp == 0x3 && i == 1)) {
			udelay(1);
			reg = in_be32(&serdes2_base->srdstcalcr);
			reg |= 0x08000000;
			out_be32(&serdes2_base->srdstcalcr, reg);
			reg = in_be32(&serdes2_base->srdsrcalcr);
			reg |= 0x08000000;
			out_be32(&serdes2_base->srdsrcalcr, reg);
		}
	}
	udelay(1);

#endif

	/* Wait for at lesat 625us to ensure the PLLs being reset are locked */
	udelay(800);

#ifdef CONFIG_SYS_FSL_SRDS_1
	cfg_tmp = (cfg_rcw5 >> 22) & 0x3;
	for (i = 0; i < 2 && !(cfg_tmp & (0x1 << (1 - i))); i++) {
		/* if the PLL is not locked, set RST_ERR */
		reg = in_be32(&serdes1_base->bank[i].pllcr0);
		if (!((reg >> 23) & 0x1)) {
			reg = in_be32(&serdes1_base->bank[i].rstctl);
			reg |= 0x20000000;
			out_be32(&serdes1_base->bank[i].rstctl, reg);
		} else {
			udelay(1);
			reg = in_be32(&serdes1_base->bank[i].rstctl);
			reg &= 0xFFFFFFEF;
			reg |= 0x00000040;
			out_be32(&serdes1_base->bank[i].rstctl, reg);
			udelay(1);
		}
	}
#endif

#ifdef CONFIG_SYS_FSL_SRDS_2
	cfg_tmp = (cfg_rcw5 >> 20) & 0x3;
	for (i = 0; i < 2 && !(cfg_tmp & (0x1 << (1 - i))); i++) {
		reg = in_be32(&serdes2_base->bank[i].pllcr0);
		if (!((reg >> 23) & 0x1)) {
			reg = in_be32(&serdes2_base->bank[i].rstctl);
			reg |= 0x20000000;
			out_be32(&serdes2_base->bank[i].rstctl, reg);
		} else {
			udelay(1);
			reg = in_be32(&serdes2_base->bank[i].rstctl);
			reg &= 0xFFFFFFEF;
			reg |= 0x00000040;
			out_be32(&serdes2_base->bank[i].rstctl, reg);
			udelay(1);
		}
	}
#endif

	/* Take the all enabled lanes out of reset */
#ifdef CONFIG_SYS_FSL_SRDS_1
	cfg_tmp = cfg_rcw4 & FSL_CHASSIS2_RCWSR4_SRDS1_PRTCL_MASK;
	cfg_tmp >>= FSL_CHASSIS2_RCWSR4_SRDS1_PRTCL_SHIFT;

	for (i = 0; i < 4 && cfg_tmp & (0xf << (3 - i)); i++) {
		reg = in_be32(&serdes1_base->lane[i].gcr0);
		reg |= 0x00600000;
		out_be32(&serdes1_base->lane[i].gcr0, reg);
	}
#endif
#ifdef CONFIG_SYS_FSL_SRDS_2
	cfg_tmp = cfg_rcw4 & FSL_CHASSIS2_RCWSR4_SRDS2_PRTCL_MASK;
	cfg_tmp >>= FSL_CHASSIS2_RCWSR4_SRDS2_PRTCL_SHIFT;

	for (i = 0; i < 4 && cfg_tmp & (0xf << (3 - i)); i++) {
		reg = in_be32(&serdes2_base->lane[i].gcr0);
		reg |= 0x00600000;
		out_be32(&serdes2_base->lane[i].gcr0, reg);
	}
#endif
	/* For each PLL being reset, and achieved PLL lock set RST_DONE */
#ifdef CONFIG_SYS_FSL_SRDS_1
	cfg_tmp = (cfg_rcw5 >> 22) & 0x3;
	for (i = 0; i < 2; i++) {
		reg = in_be32(&serdes1_base->bank[i].pllcr0);
		if (!(cfg_tmp & (0x1 << (1 - i))) && ((reg >> 23) & 0x1)) {
			reg = in_be32(&serdes1_base->bank[i].rstctl);
			reg |= 0x40000000;
			out_be32(&serdes1_base->bank[i].rstctl, reg);
		}
	}
#endif
#ifdef CONFIG_SYS_FSL_SRDS_2
	cfg_tmp = (cfg_rcw5 >> 20) & 0x3;
	for (i = 0; i < 2; i++) {
		reg = in_be32(&serdes2_base->bank[i].pllcr0);
		if (!(cfg_tmp & (0x1 << (1 - i))) && ((reg >> 23) & 0x1)) {
			reg = in_be32(&serdes2_base->bank[i].rstctl);
			reg |= 0x40000000;
			out_be32(&serdes2_base->bank[i].rstctl, reg);
		}
	}
#endif

	return ret;
}

void fsl_serdes_init(void)
{
#ifdef CONFIG_SYS_FSL_SRDS_1
	serdes_init(FSL_SRDS_1,
		    CONFIG_SYS_FSL_SERDES_ADDR,
		    FSL_CHASSIS2_RCWSR4_SRDS1_PRTCL_MASK,
		    FSL_CHASSIS2_RCWSR4_SRDS1_PRTCL_SHIFT,
		    serdes1_prtcl_map);
#endif
#ifdef CONFIG_SYS_FSL_SRDS_2
	serdes_init(FSL_SRDS_2,
		    CONFIG_SYS_FSL_SERDES_ADDR,
		    FSL_CHASSIS2_RCWSR4_SRDS2_PRTCL_MASK,
		    FSL_CHASSIS2_RCWSR4_SRDS2_PRTCL_SHIFT,
		    serdes2_prtcl_map);
#endif
}
