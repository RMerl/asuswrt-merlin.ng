// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2012 Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
 * (C) Copyright 2012 Renesas Solutions Corp.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <netdev.h>
#include <i2c.h>

DECLARE_GLOBAL_DATA_PTR;

#define CS0BCR_D (0x06C00400)
#define CS4BCR_D (0x16c90400)
#define CS0WCR_D (0x55062C42)
#define CS4WCR_D (0x1e071dc3)

#define CMNCR_BROMMD0   (1 << 21)
#define CMNCR_BROMMD1   (1 << 22)
#define CMNCR_BROMMD	(CMNCR_BROMMD0|CMNCR_BROMMD1)
#define VCLKCR1_D	(0x27)

#define SMSTPCR1_CMT0	(1 << 24)
#define SMSTPCR1_I2C0	(1 << 16)
#define SMSTPCR3_USB	(1 << 22)
#define SMSTPCR3_I2C1	(1 << 23)

#define PORT32CR (0xE6051020)
#define PORT33CR (0xE6051021)
#define PORT34CR (0xE6051022)
#define PORT35CR (0xE6051023)

static int cmp_loop(u32 *addr, u32 data, u32 cmp)
{
	int err = -1;
	int timeout = 100;
	u32 value;

	while (timeout > 0) {
		value = readl(addr);
		if ((value & data) == cmp) {
			err = 0;
			break;
		}
		timeout--;
	}

	return err;
}

/* SBSC Init function */
static void sbsc_init(struct sh73a0_sbsc *sbsc)
{
	writel(readl(&sbsc->dllcnt0)|0x2, &sbsc->dllcnt0);
	writel(0x5, &sbsc->sdgencnt);
	cmp_loop(&sbsc->sdgencnt, 0xffffffff, 0x0);

	writel(0xacc90159, &sbsc->sdcr0);
	writel(0x00010059, &sbsc->sdcr1);
	writel(0x50874114, &sbsc->sdwcrc0);
	writel(0x33199b37, &sbsc->sdwcrc1);
	writel(0x008f2313, &sbsc->sdwcrc2);
	writel(0x31020707, &sbsc->sdwcr00);
	writel(0x0017040a, &sbsc->sdwcr01);
	writel(0x31020707, &sbsc->sdwcr10);
	writel(0x0017040a, &sbsc->sdwcr11);
	writel(0x055557ff, &sbsc->sddrvcr0); /* Enlarge drivability of LPDQS0-3, LPCLK */
	writel(0x30000000, &sbsc->sdwcr2);

	writel(readl(&sbsc->sdpcr) | 0x80, &sbsc->sdpcr);
	cmp_loop(&sbsc->sdpcr, 0x80, 0x80);

	writel(0x00002710, &sbsc->sdgencnt);
	cmp_loop(&sbsc->sdgencnt, 0xffffffff, 0x0);

	writel(0x0000003f, &sbsc->sdmracr0);
	writel(0x0, SDMRA1A);
	writel(0x000001f4, &sbsc->sdgencnt);
	cmp_loop(&sbsc->sdgencnt, 0xffffffff, 0x0);

	writel(0x0000ff0a, &sbsc->sdmracr0);
	if (sbsc == (struct sh73a0_sbsc *)SBSC1_BASE)
		writel(0x0, SDMRA3A);
	else
		writel(0x0, SDMRA3B);

	writel(0x00000032, &sbsc->sdgencnt);
	cmp_loop(&sbsc->sdgencnt, 0xffffffff, 0x0);

	if (sbsc == (struct sh73a0_sbsc *)SBSC1_BASE) {
		writel(0x00002201, &sbsc->sdmracr0);
		writel(0x0, SDMRA1A);
		writel(0x00000402, &sbsc->sdmracr0);
		writel(0x0, SDMRA1A);
		writel(0x00000203, &sbsc->sdmracr0); /* MR3 register DS=2 */
		writel(0x0, SDMRA1A);
		writel(0x0, SDMRA2A);
	} else {
		writel(0x00002201, &sbsc->sdmracr0);
		writel(0x0, SDMRA1B);
		writel(0x00000402, &sbsc->sdmracr0);
		writel(0x0, SDMRA1B);
		writel(0x00000203, &sbsc->sdmracr0); /* MR3 register DS=2 */
		writel(0x0, SDMRA1B);
		writel(0x0, SDMRA2B);
	}

	writel(0x88800004, &sbsc->sdmrtmpcr);
	writel(0x00000004, &sbsc->sdmrtmpmsk);
	writel(0xa55a0032, &sbsc->rtcor);
	writel(0xa55a000c, &sbsc->rtcorh);
	writel(0xa55a2048, &sbsc->rtcsr);
	writel(readl(&sbsc->sdcr0)|0x800, &sbsc->sdcr0);
	writel(readl(&sbsc->sdcr1)|0x400, &sbsc->sdcr1);
	writel(0xfff20000, &sbsc->zqccr);

	/* SCBS2 only */
	if (sbsc == (struct sh73a0_sbsc *)SBSC2_BASE) {
		writel(readl(&sbsc->sdpdcr0)|0x00030000, &sbsc->sdpdcr0);
		writel(0xa5390000, &sbsc->dphycnt1);
		writel(0x00001200, &sbsc->dphycnt0);
		writel(0x07ce0000, &sbsc->dphycnt1);
		writel(0x00001247, &sbsc->dphycnt0);
		cmp_loop(&sbsc->dphycnt2, 0xffffffff, 0x07ce0000);
		writel(readl(&sbsc->sdpdcr0) & 0xfffcffff, &sbsc->sdpdcr0);
	}
}

void s_init(void)
{
	struct sh73a0_rwdt *rwdt = (struct sh73a0_rwdt *)RWDT_BASE;
	struct sh73a0_sbsc_cpg *cpg = (struct sh73a0_sbsc_cpg *)CPG_BASE;
	struct sh73a0_sbsc_cpg_srcr *cpg_srcr =
		(struct sh73a0_sbsc_cpg_srcr *)CPG_SRCR_BASE;
	struct sh73a0_sbsc *sbsc1 = (struct sh73a0_sbsc *)SBSC1_BASE;
	struct sh73a0_sbsc *sbsc2 = (struct sh73a0_sbsc *)SBSC2_BASE;
	struct sh73a0_hpb *hpb = (struct sh73a0_hpb *)HPB_BASE;
	struct sh73a0_hpb_bscr *hpb_bscr =
		(struct sh73a0_hpb_bscr *)HPBSCR_BASE;

	/* Watchdog init */
	writew(0xA507, &rwdt->rwtcsra0);

	/* Secure control register Init */
	#define LIFEC_SEC_SRC_BIT	(1 << 15)
	writel(readl(LIFEC_SEC_SRC) & ~LIFEC_SEC_SRC_BIT, LIFEC_SEC_SRC);

	clrbits_le32(&cpg->smstpcr3, (1 << 15));
	clrbits_le32(&cpg_srcr->srcr3, (1 << 15));
	clrbits_le32(&cpg->smstpcr2, (1 << 18));
	clrbits_le32(&cpg_srcr->srcr2, (1 << 18));
	writel(0x0, &cpg->pllecr);

	cmp_loop(&cpg->pllecr, 0x00000F00, 0x0);
	cmp_loop(&cpg->frqcrb, 0x80000000, 0x0);

	writel(0x2D000000, &cpg->pll0cr);
	writel(0x17100000, &cpg->pll1cr);
	writel(0x96235880, &cpg->frqcrb);
	cmp_loop(&cpg->frqcrb, 0x80000000, 0x0);

	writel(0xB, &cpg->flckcr);
	clrbits_le32(&cpg->smstpcr0, (1 << 1));

	clrbits_le32(&cpg_srcr->srcr0, (1 << 1));
	writel(0x0514, &hpb_bscr->smgpiotime);
	writel(0x0514, &hpb_bscr->smcmt2time);
	writel(0x0514, &hpb_bscr->smcpgtime);
	writel(0x0514, &hpb_bscr->smsysctime);

	writel(0x00092000, &cpg->dvfscr4);
	writel(0x000000DC, &cpg->dvfscr5);
	writel(0x0, &cpg->pllecr);
	cmp_loop(&cpg->pllecr, 0x00000F00, 0x0);

	/* FRQCR Init */
	writel(0x0012453C, &cpg->frqcra);
	writel(0x80431350, &cpg->frqcrb);    /* ETM TRCLK  78MHz */
	cmp_loop(&cpg->frqcrb, 0x80000000, 0x0);
	writel(0x00000B0B, &cpg->frqcrd);
	cmp_loop(&cpg->frqcrd, 0x80000000, 0x0);

	/* Clock Init */
	writel(0x00000003, PCLKCR);
	writel(0x0000012F, &cpg->vclkcr1);
	writel(0x00000119, &cpg->vclkcr2);
	writel(0x00000119, &cpg->vclkcr3);
	writel(0x00000002, &cpg->zbckcr);
	writel(0x00000005, &cpg->flckcr);
	writel(0x00000080, &cpg->sd0ckcr);
	writel(0x00000080, &cpg->sd1ckcr);
	writel(0x00000080, &cpg->sd2ckcr);
	writel(0x0000003F, &cpg->fsiackcr);
	writel(0x0000003F, &cpg->fsibckcr);
	writel(0x00000080, &cpg->subckcr);
	writel(0x0000000B, &cpg->spuackcr);
	writel(0x0000000B, &cpg->spuvckcr);
	writel(0x0000013F, &cpg->msuckcr);
	writel(0x00000080, &cpg->hsickcr);
	writel(0x0000003F, &cpg->mfck1cr);
	writel(0x0000003F, &cpg->mfck2cr);
	writel(0x00000107, &cpg->dsitckcr);
	writel(0x00000313, &cpg->dsi0pckcr);
	writel(0x0000130D, &cpg->dsi1pckcr);
	writel(0x2A800E0E, &cpg->dsi0phycr);
	writel(0x1E000000, &cpg->pll0cr);
	writel(0x2D000000, &cpg->pll0cr);
	writel(0x17100000, &cpg->pll1cr);
	writel(0x27000080, &cpg->pll2cr);
	writel(0x1D000000, &cpg->pll3cr);
	writel(0x00080000, &cpg->pll0stpcr);
	writel(0x000120C0, &cpg->pll1stpcr);
	writel(0x00012000, &cpg->pll2stpcr);
	writel(0x00000030, &cpg->pll3stpcr);

	writel(0x0000000B, &cpg->pllecr);
	cmp_loop(&cpg->pllecr, 0x00000B00, 0x00000B00);

	writel(0x000120F0, &cpg->dvfscr3);
	writel(0x00000020, &cpg->mpmode);
	writel(0x0000028A, &cpg->vrefcr);
	writel(0xE4628087, &cpg->rmstpcr0);
	writel(0xFFFFFFFF, &cpg->rmstpcr1);
	writel(0x53FFFFFF, &cpg->rmstpcr2);
	writel(0xFFFFFFFF, &cpg->rmstpcr3);
	writel(0x00800D3D, &cpg->rmstpcr4);
	writel(0xFFFFF3FF, &cpg->rmstpcr5);
	writel(0x00000000, &cpg->smstpcr2);
	writel(0x00040000, &cpg_srcr->srcr2);

	clrbits_le32(&cpg->pllecr, (1 << 3));
	cmp_loop(&cpg->pllecr, 0x00000800, 0x0);

	writel(0x00000001, &hpb->hpbctrl6);
	cmp_loop(&hpb->hpbctrl6, 0x1, 0x1);

	writel(0x00001414, &cpg->frqcrd);
	cmp_loop(&cpg->frqcrd, 0x80000000, 0x0);

	writel(0x1d000000, &cpg->pll3cr);
	setbits_le32(&cpg->pllecr, (1 << 3));
	cmp_loop(&cpg->pllecr, 0x800, 0x800);

	/* SBSC1 Init*/
	sbsc_init(sbsc1);

	/* SBSC2 Init*/
	sbsc_init(sbsc2);

	writel(0x00000b0b, &cpg->frqcrd);
	cmp_loop(&cpg->frqcrd, 0x80000000, 0x0);
	writel(0xfffffffc, &cpg->cpgxxcs4);
}

int board_early_init_f(void)
{
	struct sh73a0_sbsc_cpg *cpg = (struct sh73a0_sbsc_cpg *)CPG_BASE;
	struct sh73a0_bsc *bsc = (struct sh73a0_bsc *)BSC_BASE;
	struct sh73a0_sbsc_cpg_srcr *cpg_srcr =
		(struct sh73a0_sbsc_cpg_srcr *)CPG_SRCR_BASE;

	writel(CS0BCR_D, &bsc->cs0bcr);
	writel(CS4BCR_D, &bsc->cs4bcr);
	writel(CS0WCR_D, &bsc->cs0wcr);
	writel(CS4WCR_D, &bsc->cs4wcr);

	clrsetbits_le32(&bsc->cmncr, ~CMNCR_BROMMD, CMNCR_BROMMD);

	clrbits_le32(&cpg->smstpcr1, (SMSTPCR1_CMT0|SMSTPCR1_I2C0));
	clrbits_le32(&cpg_srcr->srcr1, (SMSTPCR1_CMT0|SMSTPCR1_I2C0));
	clrbits_le32(&cpg->smstpcr3, (SMSTPCR3_USB|SMSTPCR3_I2C1));
	clrbits_le32(&cpg_srcr->srcr3, (SMSTPCR3_USB|SMSTPCR3_I2C1));
	writel(VCLKCR1_D, &cpg->vclkcr1);

	/* Setup SCIF4 / workaround */
	writeb(0x12, PORT32CR);
	writeb(0x22, PORT33CR);
	writeb(0x12, PORT34CR);
	writeb(0x22, PORT35CR);

	return 0;
}

void adjust_core_voltage(void)
{
	u8 data;

	data = 0x35;
	i2c_set_bus_num(0);
	i2c_write(0x40, 3, 1, &data, 1);
}

int board_init(void)
{
	adjust_core_voltage();
	sh73a0_pinmux_init();

    /* SCIFA 4 */
	gpio_request(GPIO_FN_SCIFA4_TXD, NULL);
	gpio_request(GPIO_FN_SCIFA4_RXD, NULL);
	gpio_request(GPIO_FN_SCIFA4_RTS_, NULL);
	gpio_request(GPIO_FN_SCIFA4_CTS_, NULL);

	/* Ethernet/SMSC */
	gpio_request(GPIO_PORT224, NULL);
	gpio_direction_input(GPIO_PORT224);

	/* SMSC/USB */
	gpio_request(GPIO_FN_CS4_, NULL);

	/* MMCIF */
	gpio_request(GPIO_FN_MMCCLK0, NULL);
	gpio_request(GPIO_FN_MMCCMD0_PU, NULL);
	gpio_request(GPIO_FN_MMCD0_0_PU, NULL);
	gpio_request(GPIO_FN_MMCD0_1_PU, NULL);
	gpio_request(GPIO_FN_MMCD0_2_PU, NULL);
	gpio_request(GPIO_FN_MMCD0_3_PU, NULL);
	gpio_request(GPIO_FN_MMCD0_4_PU, NULL);
	gpio_request(GPIO_FN_MMCD0_5_PU, NULL);
	gpio_request(GPIO_FN_MMCD0_6_PU, NULL);
	gpio_request(GPIO_FN_MMCD0_7_PU, NULL);

	/* SDHI */
	gpio_request(GPIO_FN_SDHIWP0, NULL);
	gpio_request(GPIO_FN_SDHICD0, NULL);
	gpio_request(GPIO_FN_SDHICMD0, NULL);
	gpio_request(GPIO_FN_SDHICLK0,  NULL);
	gpio_request(GPIO_FN_SDHID0_3,  NULL);
	gpio_request(GPIO_FN_SDHID0_2,  NULL);
	gpio_request(GPIO_FN_SDHID0_1,  NULL);
	gpio_request(GPIO_FN_SDHID0_0,  NULL);
	gpio_request(GPIO_FN_SDHI0_VCCQ_MC0_ON, NULL);
	gpio_request(GPIO_PORT15, NULL);
	gpio_direction_output(GPIO_PORT15, 1);

	/* I2C */
	gpio_request(GPIO_FN_PORT237_I2C_SCL2, NULL);
	gpio_request(GPIO_FN_PORT236_I2C_SDA2, NULL);
	gpio_request(GPIO_FN_PORT27_I2C_SCL3, NULL);
	gpio_request(GPIO_FN_PORT28_I2C_SDA3, NULL);

	gd->bd->bi_boot_params = (CONFIG_SYS_SDRAM_BASE + 0x100);

	return 0;
}

int dram_init(void)
{
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;
	return 0;
}

int board_eth_init(bd_t *bis)
{
	int ret = 0;
#ifdef CONFIG_SMC911X
	ret = smc911x_initialize(0, CONFIG_SMC911X_BASE);
#endif
	return ret;
}

void reset_cpu(ulong addr)
{
	/* Soft Power On Reset */
	writel((1 << 31), RESCNT2);
}
