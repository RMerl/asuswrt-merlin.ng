// SPDX-License-Identifier: GPL-2.0+
/*
 * Keystone2: DDR3 initialization
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#include <asm/io.h>
#include <common.h>
#include <asm/arch/msmc.h>
#include <asm/arch/ddr3.h>
#include <asm/arch/psc_defs.h>

#include <asm/ti-common/ti-edma3.h>

#define DDR3_EDMA_BLK_SIZE_SHIFT	10
#define DDR3_EDMA_BLK_SIZE		(1 << DDR3_EDMA_BLK_SIZE_SHIFT)
#define DDR3_EDMA_BCNT			0x8000
#define DDR3_EDMA_CCNT			1
#define DDR3_EDMA_XF_SIZE		(DDR3_EDMA_BLK_SIZE * DDR3_EDMA_BCNT)
#define DDR3_EDMA_SLOT_NUM		1

void ddr3_init_ddrphy(u32 base, struct ddr3_phy_config *phy_cfg)
{
	unsigned int tmp;

	while ((__raw_readl(base + KS2_DDRPHY_PGSR0_OFFSET)
		 & 0x00000001) != 0x00000001)
		;

	__raw_writel(phy_cfg->pllcr, base + KS2_DDRPHY_PLLCR_OFFSET);

	tmp = __raw_readl(base + KS2_DDRPHY_PGCR1_OFFSET);
	tmp &= ~(phy_cfg->pgcr1_mask);
	tmp |= phy_cfg->pgcr1_val;
	__raw_writel(tmp, base + KS2_DDRPHY_PGCR1_OFFSET);

	__raw_writel(phy_cfg->ptr0,   base + KS2_DDRPHY_PTR0_OFFSET);
	__raw_writel(phy_cfg->ptr1,   base + KS2_DDRPHY_PTR1_OFFSET);
	__raw_writel(phy_cfg->ptr3,  base + KS2_DDRPHY_PTR3_OFFSET);
	__raw_writel(phy_cfg->ptr4,  base + KS2_DDRPHY_PTR4_OFFSET);

	tmp =  __raw_readl(base + KS2_DDRPHY_DCR_OFFSET);
	tmp &= ~(phy_cfg->dcr_mask);
	tmp |= phy_cfg->dcr_val;
	__raw_writel(tmp, base + KS2_DDRPHY_DCR_OFFSET);

	__raw_writel(phy_cfg->dtpr0, base + KS2_DDRPHY_DTPR0_OFFSET);
	__raw_writel(phy_cfg->dtpr1, base + KS2_DDRPHY_DTPR1_OFFSET);
	__raw_writel(phy_cfg->dtpr2, base + KS2_DDRPHY_DTPR2_OFFSET);
	__raw_writel(phy_cfg->mr0,   base + KS2_DDRPHY_MR0_OFFSET);
	__raw_writel(phy_cfg->mr1,   base + KS2_DDRPHY_MR1_OFFSET);
	__raw_writel(phy_cfg->mr2,   base + KS2_DDRPHY_MR2_OFFSET);
	__raw_writel(phy_cfg->dtcr,  base + KS2_DDRPHY_DTCR_OFFSET);
	__raw_writel(phy_cfg->pgcr2, base + KS2_DDRPHY_PGCR2_OFFSET);

	__raw_writel(phy_cfg->zq0cr1, base + KS2_DDRPHY_ZQ0CR1_OFFSET);
	__raw_writel(phy_cfg->zq1cr1, base + KS2_DDRPHY_ZQ1CR1_OFFSET);
	__raw_writel(phy_cfg->zq2cr1, base + KS2_DDRPHY_ZQ2CR1_OFFSET);

	__raw_writel(phy_cfg->pir_v1, base + KS2_DDRPHY_PIR_OFFSET);
	while ((__raw_readl(base + KS2_DDRPHY_PGSR0_OFFSET) & 0x1) != 0x1)
		;

	if (cpu_is_k2g()) {
		clrsetbits_le32(base + KS2_DDRPHY_DATX8_2_OFFSET,
				phy_cfg->datx8_2_mask,
				phy_cfg->datx8_2_val);

		clrsetbits_le32(base + KS2_DDRPHY_DATX8_3_OFFSET,
				phy_cfg->datx8_3_mask,
				phy_cfg->datx8_3_val);

		clrsetbits_le32(base + KS2_DDRPHY_DATX8_4_OFFSET,
				phy_cfg->datx8_4_mask,
				phy_cfg->datx8_4_val);

		clrsetbits_le32(base + KS2_DDRPHY_DATX8_5_OFFSET,
				phy_cfg->datx8_5_mask,
				phy_cfg->datx8_5_val);

		clrsetbits_le32(base + KS2_DDRPHY_DATX8_6_OFFSET,
				phy_cfg->datx8_6_mask,
				phy_cfg->datx8_6_val);

		clrsetbits_le32(base + KS2_DDRPHY_DATX8_7_OFFSET,
				phy_cfg->datx8_7_mask,
				phy_cfg->datx8_7_val);

		clrsetbits_le32(base + KS2_DDRPHY_DATX8_8_OFFSET,
				phy_cfg->datx8_8_mask,
				phy_cfg->datx8_8_val);
	}

	__raw_writel(phy_cfg->pir_v2, base + KS2_DDRPHY_PIR_OFFSET);
	while ((__raw_readl(base + KS2_DDRPHY_PGSR0_OFFSET) & 0x1) != 0x1)
		;
}

void ddr3_init_ddremif(u32 base, struct ddr3_emif_config *emif_cfg)
{
	__raw_writel(emif_cfg->sdcfg,  base + KS2_DDR3_SDCFG_OFFSET);
	__raw_writel(emif_cfg->sdtim1, base + KS2_DDR3_SDTIM1_OFFSET);
	__raw_writel(emif_cfg->sdtim2, base + KS2_DDR3_SDTIM2_OFFSET);
	__raw_writel(emif_cfg->sdtim3, base + KS2_DDR3_SDTIM3_OFFSET);
	__raw_writel(emif_cfg->sdtim4, base + KS2_DDR3_SDTIM4_OFFSET);
	__raw_writel(emif_cfg->zqcfg,  base + KS2_DDR3_ZQCFG_OFFSET);
	__raw_writel(emif_cfg->sdrfc,  base + KS2_DDR3_SDRFC_OFFSET);
}

int ddr3_ecc_support_rmw(u32 base)
{
	u32 value = __raw_readl(base + KS2_DDR3_MIDR_OFFSET);

	/* Check the DDR3 controller ID reg if the controllers
	   supports ECC RMW or not */
	if (value == 0x40461C02)
		return 1;

	return 0;
}

static void ddr3_ecc_config(u32 base, u32 value)
{
	u32 data;

	__raw_writel(value,  base + KS2_DDR3_ECC_CTRL_OFFSET);
	udelay(100000); /* delay required to synchronize across clock domains */

	if (value & KS2_DDR3_ECC_EN) {
		/* Clear the 1-bit error count */
		data = __raw_readl(base + KS2_DDR3_ONE_BIT_ECC_ERR_CNT_OFFSET);
		__raw_writel(data, base + KS2_DDR3_ONE_BIT_ECC_ERR_CNT_OFFSET);

		/* enable the ECC interrupt */
		__raw_writel(KS2_DDR3_1B_ECC_ERR_SYS | KS2_DDR3_2B_ECC_ERR_SYS |
			     KS2_DDR3_WR_ECC_ERR_SYS,
			     base + KS2_DDR3_ECC_INT_ENABLE_SET_SYS_OFFSET);

		/* Clear the ECC error interrupt status */
		__raw_writel(KS2_DDR3_1B_ECC_ERR_SYS | KS2_DDR3_2B_ECC_ERR_SYS |
			     KS2_DDR3_WR_ECC_ERR_SYS,
			     base + KS2_DDR3_ECC_INT_STATUS_OFFSET);
	}
}

static void ddr3_reset_data(u32 base, u32 ddr3_size)
{
	u32 mpax[2];
	u32 seg_num;
	u32 seg, blks, dst, edma_blks;
	struct edma3_slot_config slot;
	struct edma3_channel_config edma_channel;
	u32 edma_src[DDR3_EDMA_BLK_SIZE/4] __aligned(16) = {0, };

	/* Setup an edma to copy the 1k block to the entire DDR */
	puts("\nClear entire DDR3 memory to enable ECC\n");

	/* save the SES MPAX regs */
	if (cpu_is_k2g())
		msmc_get_ses_mpax(K2G_MSMC_SEGMENT_ARM, 0, mpax);
	else
		msmc_get_ses_mpax(K2HKLE_MSMC_SEGMENT_ARM, 0, mpax);

	/* setup edma slot 1 configuration */
	slot.opt = EDMA3_SLOPT_TRANS_COMP_INT_ENB |
		   EDMA3_SLOPT_COMP_CODE(0) |
		   EDMA3_SLOPT_STATIC | EDMA3_SLOPT_AB_SYNC;
	slot.bcnt = DDR3_EDMA_BCNT;
	slot.acnt = DDR3_EDMA_BLK_SIZE;
	slot.ccnt = DDR3_EDMA_CCNT;
	slot.src_bidx = 0;
	slot.dst_bidx = DDR3_EDMA_BLK_SIZE;
	slot.src_cidx = 0;
	slot.dst_cidx = 0;
	slot.link = EDMA3_PARSET_NULL_LINK;
	slot.bcntrld = 0;
	edma3_slot_configure(KS2_EDMA0_BASE, DDR3_EDMA_SLOT_NUM, &slot);

	/* configure quik edma channel */
	edma_channel.slot = DDR3_EDMA_SLOT_NUM;
	edma_channel.chnum = 0;
	edma_channel.complete_code = 0;
	/* event trigger after dst update */
	edma_channel.trigger_slot_word = EDMA3_TWORD(dst);
	qedma3_start(KS2_EDMA0_BASE, &edma_channel);

	/* DDR3 size in segments (4KB seg size) */
	seg_num = ddr3_size << (30 - KS2_MSMC_SEG_SIZE_SHIFT);

	for (seg = 0; seg < seg_num; seg += KS2_MSMC_MAP_SEG_NUM) {
		/* map 2GB 36-bit DDR address to 32-bit DDR address in EMIF
		   access slave interface so that edma driver can access */
		if (cpu_is_k2g()) {
			msmc_map_ses_segment(K2G_MSMC_SEGMENT_ARM, 0,
					     base >> KS2_MSMC_SEG_SIZE_SHIFT,
					     KS2_MSMC_DST_SEG_BASE + seg,
					     MPAX_SEG_2G);
		} else {
			msmc_map_ses_segment(K2HKLE_MSMC_SEGMENT_ARM, 0,
					     base >> KS2_MSMC_SEG_SIZE_SHIFT,
					     KS2_MSMC_DST_SEG_BASE + seg,
					     MPAX_SEG_2G);
		}

		if ((seg_num - seg) > KS2_MSMC_MAP_SEG_NUM)
			edma_blks = KS2_MSMC_MAP_SEG_NUM <<
					(KS2_MSMC_SEG_SIZE_SHIFT
					- DDR3_EDMA_BLK_SIZE_SHIFT);
		else
			edma_blks = (seg_num - seg) << (KS2_MSMC_SEG_SIZE_SHIFT
					- DDR3_EDMA_BLK_SIZE_SHIFT);

		/* Use edma driver to scrub 2GB DDR memory */
		for (dst = base, blks = 0; blks < edma_blks;
		     blks += DDR3_EDMA_BCNT, dst += DDR3_EDMA_XF_SIZE) {
			edma3_set_src_addr(KS2_EDMA0_BASE,
					   edma_channel.slot, (u32)edma_src);
			edma3_set_dest_addr(KS2_EDMA0_BASE,
					    edma_channel.slot, (u32)dst);

			while (edma3_check_for_transfer(KS2_EDMA0_BASE,
							&edma_channel))
				udelay(10);
		}
	}

	qedma3_stop(KS2_EDMA0_BASE, &edma_channel);

	/* restore the SES MPAX regs */
	if (cpu_is_k2g())
		msmc_set_ses_mpax(K2G_MSMC_SEGMENT_ARM, 0, mpax);
	else
		msmc_set_ses_mpax(K2HKLE_MSMC_SEGMENT_ARM, 0, mpax);
}

static void ddr3_ecc_init_range(u32 base)
{
	u32 ecc_val = KS2_DDR3_ECC_EN;
	u32 rmw = ddr3_ecc_support_rmw(base);

	if (rmw)
		ecc_val |= KS2_DDR3_ECC_RMW_EN;

	__raw_writel(0, base + KS2_DDR3_ECC_ADDR_RANGE1_OFFSET);

	ddr3_ecc_config(base, ecc_val);
}

void ddr3_enable_ecc(u32 base, int test)
{
	u32 ecc_val = KS2_DDR3_ECC_ENABLE;
	u32 rmw = ddr3_ecc_support_rmw(base);

	if (test)
		ecc_val |= KS2_DDR3_ECC_ADDR_RNG_1_EN;

	if (!rmw) {
		if (!test)
			/* by default, disable ecc when rmw = 0 and no
			   ecc test */
			ecc_val = 0;
	} else {
		ecc_val |= KS2_DDR3_ECC_RMW_EN;
	}

	ddr3_ecc_config(base, ecc_val);
}

void ddr3_disable_ecc(u32 base)
{
	ddr3_ecc_config(base, 0);
}

#if defined(CONFIG_SOC_K2HK) || defined(CONFIG_SOC_K2L)
static void cic_init(u32 base)
{
	/* Disable CIC global interrupts */
	__raw_writel(0, base + KS2_CIC_GLOBAL_ENABLE);

	/* Set to normal mode, no nesting, no priority hold */
	__raw_writel(0, base + KS2_CIC_CTRL);
	__raw_writel(0, base + KS2_CIC_HOST_CTRL);

	/* Enable CIC global interrupts */
	__raw_writel(1, base + KS2_CIC_GLOBAL_ENABLE);
}

static void cic_map_cic_to_gic(u32 base, u32 chan_num, u32 irq_num)
{
	/* Map the system interrupt to a CIC channel */
	__raw_writeb(chan_num, base + KS2_CIC_CHAN_MAP(0) + irq_num);

	/* Enable CIC system interrupt */
	__raw_writel(irq_num, base + KS2_CIC_SYS_ENABLE_IDX_SET);

	/* Enable CIC Host interrupt */
	__raw_writel(chan_num, base + KS2_CIC_HOST_ENABLE_IDX_SET);
}

static void ddr3_map_ecc_cic2_irq(u32 base)
{
	cic_init(base);
	cic_map_cic_to_gic(base, KS2_CIC2_DDR3_ECC_CHAN_NUM,
			   KS2_CIC2_DDR3_ECC_IRQ_NUM);
}
#endif

void ddr3_init_ecc(u32 base, u32 ddr3_size)
{
	if (!ddr3_ecc_support_rmw(base)) {
		ddr3_disable_ecc(base);
		return;
	}

	ddr3_ecc_init_range(base);
	ddr3_reset_data(CONFIG_SYS_SDRAM_BASE, ddr3_size);

	/* mapping DDR3 ECC system interrupt from CIC2 to GIC */
#if defined(CONFIG_SOC_K2HK) || defined(CONFIG_SOC_K2L)
	ddr3_map_ecc_cic2_irq(KS2_CIC2_BASE);
#endif
	ddr3_enable_ecc(base, 0);
}

void ddr3_check_ecc_int(u32 base)
{
	char *env;
	int ecc_test = 0;
	u32 value = __raw_readl(base + KS2_DDR3_ECC_INT_STATUS_OFFSET);

	env = env_get("ecc_test");
	if (env)
		ecc_test = simple_strtol(env, NULL, 0);

	if (value & KS2_DDR3_WR_ECC_ERR_SYS)
		puts("DDR3 ECC write error interrupted\n");

	if (value & KS2_DDR3_2B_ECC_ERR_SYS) {
		puts("DDR3 ECC 2-bit error interrupted\n");

		if (!ecc_test) {
			puts("Reseting the device ...\n");
			reset_cpu(0);
		}
	}

	value = __raw_readl(base + KS2_DDR3_ONE_BIT_ECC_ERR_CNT_OFFSET);
	if (value) {
		printf("1-bit ECC err count: 0x%x\n", value);
		value = __raw_readl(base +
				    KS2_DDR3_ONE_BIT_ECC_ERR_ADDR_LOG_OFFSET);
		printf("1-bit ECC err address log: 0x%x\n", value);
	}
}

void ddr3_reset_ddrphy(void)
{
	u32 tmp;

	/* Assert DDR3A  PHY reset */
	tmp = readl(KS2_DDR3APLLCTL1);
	tmp |= KS2_DDR3_PLLCTRL_PHY_RESET;
	writel(tmp, KS2_DDR3APLLCTL1);

	/* wait 10us to catch the reset */
	udelay(10);

	/* Release DDR3A PHY reset */
	tmp = readl(KS2_DDR3APLLCTL1);
	tmp &= ~KS2_DDR3_PLLCTRL_PHY_RESET;
	__raw_writel(tmp, KS2_DDR3APLLCTL1);
}

#ifdef CONFIG_SOC_K2HK
/**
 * ddr3_reset_workaround - reset workaround in case if leveling error
 * detected for PG 1.0 and 1.1 k2hk SoCs
 */
void ddr3_err_reset_workaround(void)
{
	unsigned int tmp;
	unsigned int tmp_a;
	unsigned int tmp_b;

	/*
	 * Check for PGSR0 error bits of DDR3 PHY.
	 * Check for WLERR, QSGERR, WLAERR,
	 * RDERR, WDERR, REERR, WEERR error to see if they are set or not
	 */
	tmp_a = __raw_readl(KS2_DDR3A_DDRPHYC + KS2_DDRPHY_PGSR0_OFFSET);
	tmp_b = __raw_readl(KS2_DDR3B_DDRPHYC + KS2_DDRPHY_PGSR0_OFFSET);

	if (((tmp_a & 0x0FE00000) != 0) || ((tmp_b & 0x0FE00000) != 0)) {
		printf("DDR Leveling Error Detected!\n");
		printf("DDR3A PGSR0 = 0x%x\n", tmp_a);
		printf("DDR3B PGSR0 = 0x%x\n", tmp_b);

		/*
		 * Write Keys to KICK registers to enable writes to registers
		 * in boot config space
		 */
		__raw_writel(KS2_KICK0_MAGIC, KS2_KICK0);
		__raw_writel(KS2_KICK1_MAGIC, KS2_KICK1);

		/*
		 * Move DDR3A Module out of reset isolation by setting
		 * MDCTL23[12] = 0
		 */
		tmp_a = __raw_readl(KS2_PSC_BASE +
				    PSC_REG_MDCTL(KS2_LPSC_EMIF4F_DDR3A));

		tmp_a = PSC_REG_MDCTL_SET_RESET_ISO(tmp_a, 0);
		__raw_writel(tmp_a, KS2_PSC_BASE +
			     PSC_REG_MDCTL(KS2_LPSC_EMIF4F_DDR3A));

		/*
		 * Move DDR3B Module out of reset isolation by setting
		 * MDCTL24[12] = 0
		 */
		tmp_b = __raw_readl(KS2_PSC_BASE +
				    PSC_REG_MDCTL(KS2_LPSC_EMIF4F_DDR3B));
		tmp_b = PSC_REG_MDCTL_SET_RESET_ISO(tmp_b, 0);
		__raw_writel(tmp_b, KS2_PSC_BASE +
			     PSC_REG_MDCTL(KS2_LPSC_EMIF4F_DDR3B));

		/*
		 * Write 0x5A69 Key to RSTCTRL[15:0] to unlock writes
		 * to RSTCTRL and RSTCFG
		 */
		tmp = __raw_readl(KS2_RSTCTRL);
		tmp &= KS2_RSTCTRL_MASK;
		tmp |= KS2_RSTCTRL_KEY;
		__raw_writel(tmp, KS2_RSTCTRL);

		/*
		 * Set PLL Controller to drive hard reset on SW trigger by
		 * setting RSTCFG[13] = 0
		 */
		tmp = __raw_readl(KS2_RSTCTRL_RSCFG);
		tmp &= ~KS2_RSTYPE_PLL_SOFT;
		__raw_writel(tmp, KS2_RSTCTRL_RSCFG);

		reset_cpu(0);
	}
}
#endif
