// SPDX-License-Identifier: GPL-2.0+
/*
 * Texas Instruments' AM654 DDRSS driver
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <ram.h>
#include <asm/io.h>
#include <power-domain.h>
#include <dm.h>
#include <asm/arch/sys_proto.h>
#include <power/regulator.h>
#include "k3-am654-ddrss.h"

#define LDELAY 10000

/* DDRSS PHY configuration register fixed values */
#define DDRSS_DDRPHY_RANKIDR_RANK0	0

/**
 * struct am654_ddrss_desc - Description of ddrss integration.
 * @dev:		DDRSS device pointer
 * @ddrss_ss_cfg:	DDRSS wrapper logic region base address
 * @ddrss_ctl_cfg:	DDRSS controller region base address
 * @ddrss_phy_cfg:	DDRSS PHY region base address
 * @ddrss_clk:		DDRSS clock description
 * @vtt_supply:		VTT Supply regulator
 * @ddrss_pwrdmn:	DDRSS power domain description
 * @params:		SDRAM configuration parameters
 */
struct am654_ddrss_desc {
	struct udevice *dev;
	void __iomem *ddrss_ss_cfg;
	void __iomem *ddrss_ctl_cfg;
	void __iomem *ddrss_phy_cfg;
	struct clk ddrss_clk;
	struct udevice *vtt_supply;
	struct power_domain ddrcfg_pwrdmn;
	struct power_domain ddrdata_pwrdmn;
	struct ddrss_params params;
};

static inline u32 ddrss_readl(void __iomem *addr, unsigned int offset)
{
	return readl(addr + offset);
}

static inline void ddrss_writel(void __iomem *addr, unsigned int offset,
				u32 data)
{
	debug("%s: addr = 0x%p, value = 0x%x\n", __func__, addr + offset, data);
	writel(data, addr + offset);
}

#define ddrss_ctl_writel(off, val) ddrss_writel(ddrss->ddrss_ctl_cfg, off, val)
#define ddrss_ctl_readl(off) ddrss_readl(ddrss->ddrss_ctl_cfg, off)

static inline u32 am654_ddrss_get_type(struct am654_ddrss_desc *ddrss)
{
	return ddrss_ctl_readl(DDRSS_DDRCTL_MSTR) & MSTR_DDR_TYPE_MASK;
}

/**
 * am654_ddrss_dram_wait_for_init_complete() - Wait for init to complete
 *
 * After detecting the DDR type this function will pause until the
 * initialization is complete. Each DDR type has mask of multiple bits.
 * The size of the field depends on the DDR Type. If the initialization
 * does not complete and error will be returned and will cause the boot to halt.
 *
 */
static int am654_ddrss_dram_wait_for_init_complt(struct am654_ddrss_desc *ddrss)
{
	u32 val, mask;

	val = am654_ddrss_get_type(ddrss);

	switch (val) {
	case DDR_TYPE_LPDDR4:
	case DDR_TYPE_DDR4:
		mask = DDR4_STAT_MODE_MASK;
		break;
	case DDR_TYPE_DDR3:
		mask = DDR3_STAT_MODE_MASK;
		break;
	default:
		printf("Unsupported DDR type 0x%x\n", val);
		return -EINVAL;
	}

	if (!wait_on_value(mask, DDR_MODE_NORMAL,
			   ddrss->ddrss_ctl_cfg + DDRSS_DDRCTL_STAT, LDELAY))
		return -ETIMEDOUT;

	return 0;
}

/**
 * am654_ddrss_ctrl_configuration() - Configure Controller specific registers
 * @dev:		corresponding ddrss device
 */
static void am654_ddrss_ctrl_configuration(struct am654_ddrss_desc *ddrss)
{
	struct ddrss_ddrctl_timing_params *tmg = &ddrss->params.ctl_timing;
	struct ddrss_ddrctl_reg_params *reg = &ddrss->params.ctl_reg;
	struct ddrss_ddrctl_ecc_params *ecc = &ddrss->params.ctl_ecc;
	struct ddrss_ddrctl_crc_params *crc = &ddrss->params.ctl_crc;
	struct ddrss_ddrctl_map_params *map = &ddrss->params.ctl_map;
	u32 val;

	debug("%s: DDR controller register configuration started\n", __func__);

	ddrss_ctl_writel(DDRSS_DDRCTL_MSTR, reg->ddrctl_mstr);
	ddrss_ctl_writel(DDRSS_DDRCTL_RFSHCTL0, reg->ddrctl_rfshctl0);
	ddrss_ctl_writel(DDRSS_DDRCTL_RFSHTMG, reg->ddrctl_rfshtmg);

	ddrss_ctl_writel(DDRSS_DDRCTL_ECCCFG0, ecc->ddrctl_ecccfg0);
	ddrss_ctl_writel(DDRSS_DDRCTL_CRCPARCTL0, crc->ddrctl_crcparctl0);
	ddrss_ctl_writel(DDRSS_DDRCTL_CRCPARCTL1, crc->ddrctl_crcparctl1);
	ddrss_ctl_writel(DDRSS_DDRCTL_CRCPARCTL2, crc->ddrctl_crcparctl2);

	ddrss_ctl_writel(DDRSS_DDRCTL_INIT0, reg->ddrctl_init0);
	ddrss_ctl_writel(DDRSS_DDRCTL_INIT1, reg->ddrctl_init1);
	ddrss_ctl_writel(DDRSS_DDRCTL_INIT3, reg->ddrctl_init3);
	ddrss_ctl_writel(DDRSS_DDRCTL_INIT4, reg->ddrctl_init4);
	ddrss_ctl_writel(DDRSS_DDRCTL_INIT5, reg->ddrctl_init5);
	ddrss_ctl_writel(DDRSS_DDRCTL_INIT6, reg->ddrctl_init6);
	ddrss_ctl_writel(DDRSS_DDRCTL_INIT7, reg->ddrctl_init7);

	ddrss_ctl_writel(DDRSS_DDRCTL_DRAMTMG0, tmg->ddrctl_dramtmg0);
	ddrss_ctl_writel(DDRSS_DDRCTL_DRAMTMG1, tmg->ddrctl_dramtmg1);
	ddrss_ctl_writel(DDRSS_DDRCTL_DRAMTMG2, tmg->ddrctl_dramtmg2);
	ddrss_ctl_writel(DDRSS_DDRCTL_DRAMTMG3, tmg->ddrctl_dramtmg3);
	ddrss_ctl_writel(DDRSS_DDRCTL_DRAMTMG4, tmg->ddrctl_dramtmg4);
	ddrss_ctl_writel(DDRSS_DDRCTL_DRAMTMG5, tmg->ddrctl_dramtmg5);
	ddrss_ctl_writel(DDRSS_DDRCTL_DRAMTMG8, tmg->ddrctl_dramtmg8);
	ddrss_ctl_writel(DDRSS_DDRCTL_DRAMTMG9, tmg->ddrctl_dramtmg9);
	ddrss_ctl_writel(DDRSS_DDRCTL_DRAMTMG11, tmg->ddrctl_dramtmg11);
	ddrss_ctl_writel(DDRSS_DDRCTL_DRAMTMG12, tmg->ddrctl_dramtmg12);
	ddrss_ctl_writel(DDRSS_DDRCTL_DRAMTMG13, tmg->ddrctl_dramtmg13);
	ddrss_ctl_writel(DDRSS_DDRCTL_DRAMTMG15, tmg->ddrctl_dramtmg15);
	ddrss_ctl_writel(DDRSS_DDRCTL_DRAMTMG17, tmg->ddrctl_dramtmg17);

	ddrss_ctl_writel(DDRSS_DDRCTL_ZQCTL0, reg->ddrctl_zqctl0);
	ddrss_ctl_writel(DDRSS_DDRCTL_ZQCTL1, reg->ddrctl_zqctl1);

	ddrss_ctl_writel(DDRSS_DDRCTL_DFITMG0, reg->ddrctl_dfitmg0);
	ddrss_ctl_writel(DDRSS_DDRCTL_DFITMG1, reg->ddrctl_dfitmg1);
	ddrss_ctl_writel(DDRSS_DDRCTL_DFITMG2, reg->ddrctl_dfitmg2);

	ddrss_ctl_writel(DDRSS_DDRCTL_ADDRMAP0, map->ddrctl_addrmap0);
	ddrss_ctl_writel(DDRSS_DDRCTL_ADDRMAP1, map->ddrctl_addrmap1);
	ddrss_ctl_writel(DDRSS_DDRCTL_ADDRMAP2, map->ddrctl_addrmap2);
	ddrss_ctl_writel(DDRSS_DDRCTL_ADDRMAP3, map->ddrctl_addrmap3);
	ddrss_ctl_writel(DDRSS_DDRCTL_ADDRMAP4, map->ddrctl_addrmap4);
	ddrss_ctl_writel(DDRSS_DDRCTL_ADDRMAP5, map->ddrctl_addrmap5);
	ddrss_ctl_writel(DDRSS_DDRCTL_ADDRMAP6, map->ddrctl_addrmap6);
	ddrss_ctl_writel(DDRSS_DDRCTL_ADDRMAP7, map->ddrctl_addrmap7);
	ddrss_ctl_writel(DDRSS_DDRCTL_ADDRMAP8, map->ddrctl_addrmap8);
	ddrss_ctl_writel(DDRSS_DDRCTL_ADDRMAP9, map->ddrctl_addrmap9);
	ddrss_ctl_writel(DDRSS_DDRCTL_ADDRMAP10, map->ddrctl_addrmap10);
	ddrss_ctl_writel(DDRSS_DDRCTL_ADDRMAP11, map->ddrctl_addrmap11);

	ddrss_ctl_writel(DDRSS_DDRCTL_ODTCFG, reg->ddrctl_odtcfg);
	ddrss_ctl_writel(DDRSS_DDRCTL_ODTMAP, reg->ddrctl_odtmap);

	/* Disable refreshes */
	val = ddrss_ctl_readl(DDRSS_DDRCTL_RFSHCTL3);
	val |= 0x01;
	ddrss_ctl_writel(DDRSS_DDRCTL_RFSHCTL3, val);

	debug("%s: DDR controller configuration completed\n", __func__);
}

#define ddrss_phy_writel(off, val)					\
	do {								\
		ddrss_writel(ddrss->ddrss_phy_cfg, off, val);		\
		sdelay(10);	/* Delay at least 20 clock cycles */	\
	} while (0)

#define ddrss_phy_readl(off)						\
	({								\
		u32 val = ddrss_readl(ddrss->ddrss_phy_cfg, off);	\
		sdelay(10);	/* Delay at least 20 clock cycles */	\
		val;							\
	})

/**
 * am654_ddrss_phy_configuration() - Configure PHY specific registers
 * @ddrss:		corresponding ddrss device
 */
static void am654_ddrss_phy_configuration(struct am654_ddrss_desc *ddrss)
{
	struct ddrss_ddrphy_ioctl_params *ioctl = &ddrss->params.phy_ioctl;
	struct ddrss_ddrphy_timing_params *tmg = &ddrss->params.phy_timing;
	struct ddrss_ddrphy_ctrl_params *ctrl = &ddrss->params.phy_ctrl;
	struct ddrss_ddrphy_cfg_params *cfg = &ddrss->params.phy_cfg;
	struct ddrss_ddrphy_zq_params *zq = &ddrss->params.phy_zq;

	debug("%s: DDR phy register configuration started\n", __func__);

	ddrss_phy_writel(DDRSS_DDRPHY_PGCR1, cfg->ddrphy_pgcr1);
	ddrss_phy_writel(DDRSS_DDRPHY_PGCR2, cfg->ddrphy_pgcr2);
	ddrss_phy_writel(DDRSS_DDRPHY_PGCR3, cfg->ddrphy_pgcr3);
	ddrss_phy_writel(DDRSS_DDRPHY_PGCR6, cfg->ddrphy_pgcr6);

	ddrss_phy_writel(DDRSS_DDRPHY_PTR3, tmg->ddrphy_ptr3);
	ddrss_phy_writel(DDRSS_DDRPHY_PTR4, tmg->ddrphy_ptr4);
	ddrss_phy_writel(DDRSS_DDRPHY_PTR5, tmg->ddrphy_ptr5);
	ddrss_phy_writel(DDRSS_DDRPHY_PTR6, tmg->ddrphy_ptr6);

	ddrss_phy_writel(DDRSS_DDRPHY_PLLCR0, ctrl->ddrphy_pllcr0);

	ddrss_phy_writel(DDRSS_DDRPHY_DXCCR, cfg->ddrphy_dxccr);
	ddrss_phy_writel(DDRSS_DDRPHY_DSGCR, cfg->ddrphy_dsgcr);

	ddrss_phy_writel(DDRSS_DDRPHY_DCR, cfg->ddrphy_dcr);

	ddrss_phy_writel(DDRSS_DDRPHY_DTPR0, tmg->ddrphy_dtpr0);
	ddrss_phy_writel(DDRSS_DDRPHY_DTPR1, tmg->ddrphy_dtpr1);
	ddrss_phy_writel(DDRSS_DDRPHY_DTPR2, tmg->ddrphy_dtpr2);
	ddrss_phy_writel(DDRSS_DDRPHY_DTPR3, tmg->ddrphy_dtpr3);
	ddrss_phy_writel(DDRSS_DDRPHY_DTPR4, tmg->ddrphy_dtpr4);
	ddrss_phy_writel(DDRSS_DDRPHY_DTPR5, tmg->ddrphy_dtpr5);
	ddrss_phy_writel(DDRSS_DDRPHY_DTPR6, tmg->ddrphy_dtpr6);

	ddrss_phy_writel(DDRSS_DDRPHY_ZQCR, zq->ddrphy_zqcr);
	ddrss_phy_writel(DDRSS_DDRPHY_ZQ0PR0, zq->ddrphy_zq0pr0);
	ddrss_phy_writel(DDRSS_DDRPHY_ZQ1PR0, zq->ddrphy_zq1pr0);

	ddrss_phy_writel(DDRSS_DDRPHY_MR0, ctrl->ddrphy_mr0);
	ddrss_phy_writel(DDRSS_DDRPHY_MR1, ctrl->ddrphy_mr1);
	ddrss_phy_writel(DDRSS_DDRPHY_MR2, ctrl->ddrphy_mr2);
	ddrss_phy_writel(DDRSS_DDRPHY_MR3, ctrl->ddrphy_mr3);
	ddrss_phy_writel(DDRSS_DDRPHY_MR4, ctrl->ddrphy_mr4);
	ddrss_phy_writel(DDRSS_DDRPHY_MR5, ctrl->ddrphy_mr5);
	ddrss_phy_writel(DDRSS_DDRPHY_MR6, ctrl->ddrphy_mr6);

	ddrss_phy_writel(DDRSS_DDRPHY_VTCR0, ctrl->ddrphy_vtcr0);

	ddrss_phy_writel(DDRSS_DDRPHY_DX8SL0PLLCR0, cfg->ddrphy_dx8sl0pllcr0);
	ddrss_phy_writel(DDRSS_DDRPHY_DX8SL1PLLCR0, cfg->ddrphy_dx8sl1pllcr0);
	ddrss_phy_writel(DDRSS_DDRPHY_DX8SL2PLLCR0, cfg->ddrphy_dx8sl2pllcr0);

	ddrss_phy_writel(DDRSS_DDRPHY_DTCR0, ctrl->ddrphy_dtcr0);
	ddrss_phy_writel(DDRSS_DDRPHY_DTCR1, ctrl->ddrphy_dtcr1);

	ddrss_phy_writel(DDRSS_DDRPHY_ACIOCR5, ioctl->ddrphy_aciocr5);
	ddrss_phy_writel(DDRSS_DDRPHY_IOVCR0, ioctl->ddrphy_iovcr0);

	ddrss_phy_writel(DDRSS_DDRPHY_DX4GCR0, cfg->ddrphy_dx4gcr0);
	ddrss_phy_writel(DDRSS_DDRPHY_DX4GCR1, cfg->ddrphy_dx4gcr1);
	ddrss_phy_writel(DDRSS_DDRPHY_DX4GCR2, cfg->ddrphy_dx4gcr2);
	ddrss_phy_writel(DDRSS_DDRPHY_DX4GCR3, cfg->ddrphy_dx4gcr3);

	ddrss_phy_writel(DDRSS_DDRPHY_DX0GCR4, cfg->ddrphy_dx0gcr4);
	ddrss_phy_writel(DDRSS_DDRPHY_DX1GCR4, cfg->ddrphy_dx1gcr4);
	ddrss_phy_writel(DDRSS_DDRPHY_DX2GCR4, cfg->ddrphy_dx2gcr4);
	ddrss_phy_writel(DDRSS_DDRPHY_DX3GCR4, cfg->ddrphy_dx3gcr4);

	ddrss_phy_writel(DDRSS_DDRPHY_PGCR5, cfg->ddrphy_pgcr5);
	ddrss_phy_writel(DDRSS_DDRPHY_DX0GCR5, cfg->ddrphy_dx0gcr5);
	ddrss_phy_writel(DDRSS_DDRPHY_DX1GCR5, cfg->ddrphy_dx1gcr5);
	ddrss_phy_writel(DDRSS_DDRPHY_DX2GCR5, cfg->ddrphy_dx2gcr5);
	ddrss_phy_writel(DDRSS_DDRPHY_DX3GCR5, cfg->ddrphy_dx3gcr5);

	ddrss_phy_writel(DDRSS_DDRPHY_RANKIDR, DDRSS_DDRPHY_RANKIDR_RANK0);

	ddrss_phy_writel(DDRSS_DDRPHY_DX0GTR0, cfg->ddrphy_dx0gtr0);
	ddrss_phy_writel(DDRSS_DDRPHY_DX1GTR0, cfg->ddrphy_dx1gtr0);
	ddrss_phy_writel(DDRSS_DDRPHY_DX2GTR0, cfg->ddrphy_dx2gtr0);
	ddrss_phy_writel(DDRSS_DDRPHY_DX3GTR0, cfg->ddrphy_dx3gtr0);
	ddrss_phy_writel(DDRSS_DDRPHY_ODTCR, cfg->ddrphy_odtcr);

	ddrss_phy_writel(DDRSS_DDRPHY_DX8SL0IOCR, cfg->ddrphy_dx8sl0iocr);
	ddrss_phy_writel(DDRSS_DDRPHY_DX8SL1IOCR, cfg->ddrphy_dx8sl1iocr);
	ddrss_phy_writel(DDRSS_DDRPHY_DX8SL2IOCR, cfg->ddrphy_dx8sl2iocr);

	ddrss_phy_writel(DDRSS_DDRPHY_DX8SL0DXCTL2, cfg->ddrphy_dx8sl0dxctl2);
	ddrss_phy_writel(DDRSS_DDRPHY_DX8SL1DXCTL2, cfg->ddrphy_dx8sl1dxctl2);
	ddrss_phy_writel(DDRSS_DDRPHY_DX8SL2DXCTL2, cfg->ddrphy_dx8sl2dxctl2);

	debug("%s: DDR phy register configuration completed\n", __func__);
}

static int __phy_builtin_init_routine(struct am654_ddrss_desc *ddrss,
				      u32 init_value, u32 sts_mask,
				      u32 err_mask)
{
	int ret;

	ddrss_phy_writel(DDRSS_DDRPHY_PIR, init_value | PIR_INIT_MASK);

	sdelay(5);	/* Delay at least 10 clock cycles */

	if (!wait_on_value(sts_mask, sts_mask,
			   ddrss->ddrss_phy_cfg + DDRSS_DDRPHY_PGSR0, LDELAY))
		return -ETIMEDOUT;

	sdelay(16);	/* Delay at least 32 clock cycles */

	ret = ddrss_phy_readl(DDRSS_DDRPHY_PGSR0);
	debug("%s: PGSR0 val = 0x%x\n", __func__, ret);
	if (ret & err_mask)
		return -EINVAL;

	return 0;
}

int write_leveling(struct am654_ddrss_desc *ddrss)
{
	int ret;

	debug("%s: Write leveling started\n", __func__);

	ret = __phy_builtin_init_routine(ddrss, PIR_WL_MASK, PGSR0_WLDONE_MASK,
					 PGSR0_WLERR_MASK);
	if (ret) {
		if (ret == -ETIMEDOUT)
			printf("%s: ERROR: Write leveling timedout\n",
			       __func__);
		else
			printf("%s:ERROR: Write leveling failed\n", __func__);
		return ret;
	}

	debug("%s: Write leveling completed\n", __func__);
	return 0;
}

int read_dqs_training(struct am654_ddrss_desc *ddrss)
{
	int ret;

	debug("%s: Read DQS training started\n", __func__);

	ret = __phy_builtin_init_routine(ddrss, PIR_QSGATE_MASK,
					 PGSR0_QSGDONE_MASK, PGSR0_QSGERR_MASK);
	if (ret) {
		if (ret == -ETIMEDOUT)
			printf("%s: ERROR: Read DQS timedout\n", __func__);
		else
			printf("%s:ERROR: Read DQS Gate training failed\n",
			       __func__);
		return ret;
	}

	debug("%s: Read DQS training completed\n", __func__);
	return 0;
}

int rest_training(struct am654_ddrss_desc *ddrss)
{
	int ret;
	u32 val;
	u32 dgsl0, dgsl1, dgsl2, dgsl3, rddly, rd2wr_wr2rd;

	debug("%s: Rest of the training started\n", __func__);

	debug("%s: Write Leveling adjustment\n", __func__);
	ret = __phy_builtin_init_routine(ddrss, PIR_WLADJ_MASK,
					 PGSR0_WLADONE_MASK, PGSR0_WLAERR_MASK);
	if (ret) {
		if (ret == -ETIMEDOUT)
			printf("%s:ERROR: Write Leveling adjustment timedout\n",
			       __func__);
		else
			printf("%s: ERROR: Write Leveling adjustment failed\n",
			       __func__);
		return ret;
	}

	debug("%s: Read Deskew adjustment\n", __func__);
	ret = __phy_builtin_init_routine(ddrss, PIR_RDDSKW_MASK,
					 PGSR0_RDDONE_MASK, PGSR0_RDERR_MASK);
	if (ret) {
		if (ret == -ETIMEDOUT)
			printf("%s: ERROR: Read Deskew timedout\n", __func__);
		else
			printf("%s: ERROR: Read Deskew failed\n", __func__);
		return ret;
	}

	debug("%s: Write Deskew adjustment\n", __func__);
	ret = __phy_builtin_init_routine(ddrss, PIR_WRDSKW_MASK,
					 PGSR0_WDDONE_MASK, PGSR0_WDERR_MASK);
	if (ret) {
		if (ret == -ETIMEDOUT)
			printf("%s: ERROR: Write Deskew timedout\n", __func__);
		else
			printf("%s: ERROR: Write Deskew failed\n", __func__);
		return ret;
	}

	debug("%s: Read Eye training\n", __func__);
	ret = __phy_builtin_init_routine(ddrss, PIR_RDEYE_MASK,
					 PGSR0_REDONE_MASK, PGSR0_REERR_MASK);
	if (ret) {
		if (ret == -ETIMEDOUT)
			printf("%s: ERROR: Read Eye training timedout\n",
			       __func__);
		else
			printf("%s: ERROR: Read Eye training failed\n",
			       __func__);
		return ret;
	}

	debug("%s: Write Eye training\n", __func__);
	ret = __phy_builtin_init_routine(ddrss, PIR_WREYE_MASK,
					 PGSR0_WEDONE_MASK, PGSR0_WEERR_MASK);
	if (ret) {
		if (ret == -ETIMEDOUT)
			printf("%s: ERROR: Write Eye training timedout\n",
			       __func__);
		else
			printf("%s: ERROR: Write Eye training failed\n",
			       __func__);
		return ret;
	}

	debug("%s: VREF training\n", __func__);
	ret = __phy_builtin_init_routine(ddrss, PIR_VREF_MASK, PGSR0_VDONE_MASK,
					 PGSR0_VERR_MASK);
	if (ret) {
		if (ret == -ETIMEDOUT)
			printf("%s: ERROR: VREF training timedout\n", __func__);
		else
			printf("%s: ERROR: VREF training failed\n", __func__);
		return ret;
	}

	ddrss_phy_writel(DDRSS_DDRPHY_RANKIDR, 0x00000000);
	dgsl0 = (ddrss_phy_readl(DDRSS_DDRPHY_DX0GTR0) & 0x1F) >> 2;
	dgsl1 = (ddrss_phy_readl(DDRSS_DDRPHY_DX1GTR0) & 0x1F) >> 2;
	dgsl2 = (ddrss_phy_readl(DDRSS_DDRPHY_DX2GTR0) & 0x1F) >> 2;
	dgsl3 = (ddrss_phy_readl(DDRSS_DDRPHY_DX3GTR0) & 0x1F) >> 2;

	rddly = dgsl0;
	if (dgsl1 < rddly)
		rddly = dgsl1;
	if (dgsl2 < rddly)
		rddly = dgsl2;
	if (dgsl3 < rddly)
		rddly = dgsl3;

	rddly += 5;

	/* Update rddly based on dgsl values */
	val = (ddrss_phy_readl(DDRSS_DDRPHY_DX0GCR0) & ~0xF00000);
	val |= (rddly << 20);
	ddrss_phy_writel(DDRSS_DDRPHY_DX0GCR0, val);

	val = (ddrss_phy_readl(DDRSS_DDRPHY_DX1GCR0) & ~0xF00000);
	val |= (rddly << 20);
	ddrss_phy_writel(DDRSS_DDRPHY_DX1GCR0, val);

	val = (ddrss_phy_readl(DDRSS_DDRPHY_DX2GCR0) & ~0xF00000);
	val |= (rddly << 20);
	ddrss_phy_writel(DDRSS_DDRPHY_DX2GCR0, val);

	val = (ddrss_phy_readl(DDRSS_DDRPHY_DX3GCR0) & ~0xF00000);
	val |= (rddly << 20);
	ddrss_phy_writel(DDRSS_DDRPHY_DX3GCR0, val);

	/*
	 * Add system latency derived from training back into rd2wr and wr2rd
	 * rd2wr = RL + BL/2 + 1 + WR_PREAMBLE - WL + max(DXnGTR0.DGSL) / 2
	 * wr2rd = CWL + PL + BL/2 + tWTR_L + max(DXnGTR0.DGSL) / 2
	 */

	/* Select rank 0 */
	ddrss_phy_writel(DDRSS_DDRPHY_RANKIDR, 0x00000000);

	dgsl0 = (ddrss_phy_readl(DDRSS_DDRPHY_DX0GTR0) & 0x1F);
	dgsl1 = (ddrss_phy_readl(DDRSS_DDRPHY_DX1GTR0) & 0x1F);
	dgsl2 = (ddrss_phy_readl(DDRSS_DDRPHY_DX2GTR0) & 0x1F);
	dgsl3 = (ddrss_phy_readl(DDRSS_DDRPHY_DX3GTR0) & 0x1F);

	/* Find maximum value across all bytes */
	rd2wr_wr2rd = dgsl0;
	if (dgsl1 > rd2wr_wr2rd)
		rd2wr_wr2rd = dgsl1;
	if (dgsl2 > rd2wr_wr2rd)
		rd2wr_wr2rd = dgsl2;
	if (dgsl3 > rd2wr_wr2rd)
		rd2wr_wr2rd = dgsl3;

	rd2wr_wr2rd >>= 1;

	/* Now add in adjustment to DRAMTMG2 bit fields for rd2wr and wr2rd */
	/* Clear VSWCTL.sw_done */
	ddrss_ctl_writel(DDRSS_DDRCTL_SWCTL,
			 ddrss_ctl_readl(DDRSS_DDRCTL_SWCTL) & ~0x1);
	/* Adjust rd2wr */
	ddrss_ctl_writel(DDRSS_DDRCTL_DRAMTMG2,
			 ddrss_ctl_readl(DDRSS_DDRCTL_DRAMTMG2) +
			 (rd2wr_wr2rd << 8));
	/* Adjust wr2rd */
	ddrss_ctl_writel(DDRSS_DDRCTL_DRAMTMG2,
			 ddrss_ctl_readl(DDRSS_DDRCTL_DRAMTMG2) +
			 rd2wr_wr2rd);
	/* Set VSWCTL.sw_done */
	ddrss_ctl_writel(DDRSS_DDRCTL_SWCTL,
			 ddrss_ctl_readl(DDRSS_DDRCTL_SWCTL) | 0x1);
	/* Wait until settings are applied */
	while (!(ddrss_ctl_readl(DDRSS_DDRCTL_SWSTAT) & 0x1)) {
		/* Do nothing */
	};

	debug("%s: Rest of the training completed\n", __func__);
	return 0;
}

/**
 * am654_ddrss_init() - Initialization sequence for enabling the SDRAM
 *			device attached to ddrss.
 * @dev:		corresponding ddrss device
 *
 * Does all the initialization sequence that is required to get attached
 * ddr in a working state. After this point, ddr should be accessible.
 * Return: 0 if all went ok, else corresponding error message.
 */
static int am654_ddrss_init(struct am654_ddrss_desc *ddrss)
{
	int ret;

	debug("%s(ddrss=%p)\n", __func__, ddrss);

	ddrss_writel(ddrss->ddrss_ss_cfg, DDRSS_V2H_CTL_REG, 0x000073FF);

	am654_ddrss_ctrl_configuration(ddrss);

	/* Release the reset to the controller */
	clrbits_le32(ddrss->ddrss_ss_cfg + DDRSS_SS_CTL_REG,
		     SS_CTL_REG_CTL_ARST_MASK);

	am654_ddrss_phy_configuration(ddrss);

	ret = __phy_builtin_init_routine(ddrss, PIR_PHY_INIT, 0x1, 0);
	if (ret) {
		dev_err(ddrss->dev, "PHY initialization failed %d\n", ret);
		return ret;
	}

	ret = __phy_builtin_init_routine(ddrss, PIR_DRAM_INIT,
					 PGSR0_DRAM_INIT_MASK, 0);
	if (ret) {
		dev_err(ddrss->dev, "DRAM initialization failed %d\n", ret);
		return ret;
	}

	ret = am654_ddrss_dram_wait_for_init_complt(ddrss);
	if (ret) {
		printf("%s: ERROR: DRAM Wait for init complete timedout\n",
		       __func__);
		return ret;
	}

	ret = write_leveling(ddrss);
	if (ret)
		return ret;

	ret = read_dqs_training(ddrss);
	if (ret)
		return ret;

	ret = rest_training(ddrss);
	if (ret)
		return ret;

	/* Enabling refreshes after training is done */
	ddrss_ctl_writel(DDRSS_DDRCTL_RFSHCTL3,
			 ddrss_ctl_readl(DDRSS_DDRCTL_RFSHCTL3) & ~0x1);

	/* Disable PUBMODE after training is done */
	ddrss_phy_writel(DDRSS_DDRPHY_PGCR1,
			 ddrss_phy_readl(DDRSS_DDRPHY_PGCR1) & ~0x40);

	return 0;
}

/**
 * am654_ddrss_power_on() - Enable power and clocks for ddrss
 * @dev:	corresponding ddrss device
 *
 * Tries to enable all the corresponding clocks to the ddrss and sets it
 * to the right frequency and then power on the ddrss.
 * Return: 0 if all went ok, else corresponding error message.
 */
static int am654_ddrss_power_on(struct am654_ddrss_desc *ddrss)
{
	int ret;

	debug("%s(ddrss=%p)\n", __func__, ddrss);

	ret = clk_enable(&ddrss->ddrss_clk);
	if (ret) {
		dev_err(ddrss->dev, "clk_enable() failed: %d\n", ret);
		return ret;
	}

	ret = power_domain_on(&ddrss->ddrcfg_pwrdmn);
	if (ret) {
		dev_err(ddrss->dev, "power_domain_on() failed: %d\n", ret);
		return ret;
	}

	ret = power_domain_on(&ddrss->ddrdata_pwrdmn);
	if (ret) {
		dev_err(ddrss->dev, "power_domain_on() failed: %d\n", ret);
		return ret;
	}

	/* VTT enable */
#if CONFIG_IS_ENABLED(DM_REGULATOR)
	device_get_supply_regulator(ddrss->dev, "vtt-supply",
				    &ddrss->vtt_supply);
	ret = regulator_set_value(ddrss->vtt_supply, 3300000);
	if (ret)
		return ret;
	debug("VTT regulator enabled\n");
#endif

	return 0;
}

/**
 * am654_ddrss_ofdata_to_priv() - generate private data from device tree
 * @dev:	corresponding ddrss device
 *
 * Return: 0 if all went ok, else corresponding error message.
 */
static int am654_ddrss_ofdata_to_priv(struct udevice *dev)
{
	struct am654_ddrss_desc *ddrss = dev_get_priv(dev);
	phys_addr_t reg;
	int ret;

	debug("%s(dev=%p)\n", __func__, dev);

	ret = clk_get_by_index(dev, 0, &ddrss->ddrss_clk);
	if (ret) {
		dev_err(dev, "clk_get failed: %d\n", ret);
		return ret;
	}

	ret = power_domain_get_by_index(dev, &ddrss->ddrcfg_pwrdmn, 0);
	if (ret) {
		dev_err(dev, "power_domain_get() failed: %d\n", ret);
		return ret;
	}

	ret = power_domain_get_by_index(dev, &ddrss->ddrdata_pwrdmn, 1);
	if (ret) {
		dev_err(dev, "power_domain_get() failed: %d\n", ret);
		return ret;
	}

	reg = devfdt_get_addr_name(dev, "ss");
	if (reg == FDT_ADDR_T_NONE) {
		dev_err(dev, "No reg property for DDRSS wrapper logic\n");
		return -EINVAL;
	}
	ddrss->ddrss_ss_cfg = (void *)reg;

	reg = devfdt_get_addr_name(dev, "ctl");
	if (reg == FDT_ADDR_T_NONE) {
		dev_err(dev, "No reg property for Controller region\n");
		return -EINVAL;
	}
	ddrss->ddrss_ctl_cfg = (void *)reg;

	reg = devfdt_get_addr_name(dev, "phy");
	if (reg == FDT_ADDR_T_NONE) {
		dev_err(dev, "No reg property for PHY region\n");
		return -EINVAL;
	}
	ddrss->ddrss_phy_cfg = (void *)reg;

	ret = dev_read_u32_array(dev, "ti,ctl-reg",
				 (u32 *)&ddrss->params.ctl_reg,
				 sizeof(ddrss->params.ctl_reg) / sizeof(u32));
	if (ret) {
		dev_err(dev, "Cannot read ti,ctl-reg params\n");
		return ret;
	}

	ret = dev_read_u32_array(dev, "ti,ctl-crc",
				 (u32 *)&ddrss->params.ctl_crc,
				 sizeof(ddrss->params.ctl_crc) / sizeof(u32));
	if (ret) {
		dev_err(dev, "Cannot read ti,ctl-crc params\n");
		return ret;
	}

	ret = dev_read_u32_array(dev, "ti,ctl-ecc",
				 (u32 *)&ddrss->params.ctl_ecc,
				 sizeof(ddrss->params.ctl_ecc) / sizeof(u32));
	if (ret) {
		dev_err(dev, "Cannot read ti,ctl-ecc params\n");
		return ret;
	}

	ret = dev_read_u32_array(dev, "ti,ctl-map",
				 (u32 *)&ddrss->params.ctl_map,
				 sizeof(ddrss->params.ctl_map) / sizeof(u32));
	if (ret) {
		dev_err(dev, "Cannot read ti,ctl-map params\n");
		return ret;
	}

	ret = dev_read_u32_array(dev, "ti,ctl-pwr",
				 (u32 *)&ddrss->params.ctl_pwr,
				 sizeof(ddrss->params.ctl_pwr) / sizeof(u32));
	if (ret) {
		dev_err(dev, "Cannot read ti,ctl-pwr params\n");
		return ret;
	}

	ret = dev_read_u32_array(dev, "ti,ctl-timing",
				 (u32 *)&ddrss->params.ctl_timing,
				 sizeof(ddrss->params.ctl_timing) /
				 sizeof(u32));
	if (ret) {
		dev_err(dev, "Cannot read ti,ctl-timing params\n");
		return ret;
	}

	ret = dev_read_u32_array(dev, "ti,phy-cfg",
				 (u32 *)&ddrss->params.phy_cfg,
				 sizeof(ddrss->params.phy_cfg) / sizeof(u32));
	if (ret) {
		dev_err(dev, "Cannot read ti,phy-cfg params\n");
		return ret;
	}

	ret = dev_read_u32_array(dev, "ti,phy-ctl",
				 (u32 *)&ddrss->params.phy_ctrl,
				 sizeof(ddrss->params.phy_ctrl) / sizeof(u32));
	if (ret) {
		dev_err(dev, "Cannot read ti,phy-ctl params\n");
		return ret;
	}

	ret = dev_read_u32_array(dev, "ti,phy-ioctl",
				 (u32 *)&ddrss->params.phy_ioctl,
				 sizeof(ddrss->params.phy_ioctl) / sizeof(u32));
	if (ret) {
		dev_err(dev, "Cannot read ti,phy-ioctl params\n");
		return ret;
	}

	ret = dev_read_u32_array(dev, "ti,phy-timing",
				 (u32 *)&ddrss->params.phy_timing,
				 sizeof(ddrss->params.phy_timing) /
				 sizeof(u32));
	if (ret) {
		dev_err(dev, "Cannot read ti,phy-timing params\n");
		return ret;
	}

	ret = dev_read_u32_array(dev, "ti,phy-zq", (u32 *)&ddrss->params.phy_zq,
				 sizeof(ddrss->params.phy_zq) / sizeof(u32));
	if (ret) {
		dev_err(dev, "Cannot read ti,phy-zq params\n");
		return ret;
	}

	return ret;
}

/**
 * am654_ddrss_probe() - Basic probe
 * @dev:	corresponding ddrss device
 *
 * Return: 0 if all went ok, else corresponding error message
 */
static int am654_ddrss_probe(struct udevice *dev)
{
	struct am654_ddrss_desc *ddrss = dev_get_priv(dev);
	int ret;

	debug("%s(dev=%p)\n", __func__, dev);

	ret = am654_ddrss_ofdata_to_priv(dev);
	if (ret)
		return ret;

	ddrss->dev = dev;
	ret = am654_ddrss_power_on(ddrss);
	if (ret)
		return ret;

	ret = am654_ddrss_init(ddrss);

	return ret;
}

static int am654_ddrss_get_info(struct udevice *dev, struct ram_info *info)
{
	return 0;
}

static struct ram_ops am654_ddrss_ops = {
	.get_info = am654_ddrss_get_info,
};

static const struct udevice_id am654_ddrss_ids[] = {
	{ .compatible = "ti,am654-ddrss" },
	{ }
};

U_BOOT_DRIVER(am654_ddrss) = {
	.name = "am654_ddrss",
	.id = UCLASS_RAM,
	.of_match = am654_ddrss_ids,
	.ops = &am654_ddrss_ops,
	.probe = am654_ddrss_probe,
	.priv_auto_alloc_size = sizeof(struct am654_ddrss_desc),
};
