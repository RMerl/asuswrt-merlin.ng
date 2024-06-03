// SPDX-License-Identifier: GPL-2.0+
/*
 * Keystone2: pll initialization
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#include <common.h>
#include <asm/arch/clock.h>
#include <asm/arch/clock_defs.h>

/* DEV and ARM speed definitions as specified in DEVSPEED register */
int __weak speeds[DEVSPEED_NUMSPDS] = {
	SPD1000,
	SPD1200,
	SPD1350,
	SPD1400,
	SPD1500,
	SPD1400,
	SPD1350,
	SPD1200,
	SPD1000,
	SPD800,
};

const struct keystone_pll_regs keystone_pll_regs[] = {
	[CORE_PLL]	= {KS2_MAINPLLCTL0, KS2_MAINPLLCTL1},
	[PASS_PLL]	= {KS2_PASSPLLCTL0, KS2_PASSPLLCTL1},
	[TETRIS_PLL]	= {KS2_ARMPLLCTL0, KS2_ARMPLLCTL1},
	[DDR3A_PLL]	= {KS2_DDR3APLLCTL0, KS2_DDR3APLLCTL1},
	[DDR3B_PLL]	= {KS2_DDR3BPLLCTL0, KS2_DDR3BPLLCTL1},
	[UART_PLL]	= {KS2_UARTPLLCTL0, KS2_UARTPLLCTL1},
};

inline void pll_pa_clk_sel(void)
{
	setbits_le32(keystone_pll_regs[PASS_PLL].reg1, CFG_PLLCTL1_PAPLL_MASK);
}

static void wait_for_completion(const struct pll_init_data *data)
{
	int i;
	for (i = 0; i < 100; i++) {
		sdelay(450);
		if (!(pllctl_reg_read(data->pll, stat) & PLLSTAT_GOSTAT_MASK))
			break;
	}
}

static inline void bypass_main_pll(const struct pll_init_data *data)
{
	pllctl_reg_clrbits(data->pll, ctl, PLLCTL_PLLENSRC_MASK |
			   PLLCTL_PLLEN_MASK);

	/* 4 cycles of reference clock CLKIN*/
	sdelay(340);
}

static void configure_mult_div(const struct pll_init_data *data)
{
	u32 pllm, plld, bwadj;

	pllm = data->pll_m - 1;
	plld = (data->pll_d - 1) & CFG_PLLCTL0_PLLD_MASK;

	/* Program Multiplier */
	if (data->pll == MAIN_PLL)
		pllctl_reg_write(data->pll, mult, pllm & PLLM_MULT_LO_MASK);

	clrsetbits_le32(keystone_pll_regs[data->pll].reg0,
			CFG_PLLCTL0_PLLM_MASK,
			pllm << CFG_PLLCTL0_PLLM_SHIFT);

	/* Program BWADJ */
	bwadj = (data->pll_m - 1) >> 1; /* Divide pllm by 2 */
	clrsetbits_le32(keystone_pll_regs[data->pll].reg0,
			CFG_PLLCTL0_BWADJ_MASK,
			(bwadj << CFG_PLLCTL0_BWADJ_SHIFT) &
			CFG_PLLCTL0_BWADJ_MASK);
	bwadj = bwadj >> CFG_PLLCTL0_BWADJ_BITS;
	clrsetbits_le32(keystone_pll_regs[data->pll].reg1,
			CFG_PLLCTL1_BWADJ_MASK, bwadj);

	/* Program Divider */
	clrsetbits_le32(keystone_pll_regs[data->pll].reg0,
			CFG_PLLCTL0_PLLD_MASK, plld);
}

void configure_main_pll(const struct pll_init_data *data)
{
	u32 tmp, pllod, i, alnctl_val = 0;
	u32 *offset;

	pllod = data->pll_od - 1;

	/* 100 micro sec for stabilization */
	sdelay(210000);

	tmp = pllctl_reg_read(data->pll, secctl);

	/* Check for Bypass */
	if (tmp & SECCTL_BYPASS_MASK) {
		setbits_le32(keystone_pll_regs[data->pll].reg1,
			     CFG_PLLCTL1_ENSAT_MASK);

		bypass_main_pll(data);

		/* Powerdown and powerup Main Pll */
		pllctl_reg_setbits(data->pll, secctl, SECCTL_BYPASS_MASK);
		pllctl_reg_setbits(data->pll, ctl, PLLCTL_PLLPWRDN_MASK);
		/* 5 micro sec */
		sdelay(21000);

		pllctl_reg_clrbits(data->pll, ctl, PLLCTL_PLLPWRDN_MASK);
	} else {
		bypass_main_pll(data);
	}

	configure_mult_div(data);

	/* Program Output Divider */
	pllctl_reg_rmw(data->pll, secctl, SECCTL_OP_DIV_MASK,
		       ((pllod << SECCTL_OP_DIV_SHIFT) & SECCTL_OP_DIV_MASK));

	/* Program PLLDIVn */
	wait_for_completion(data);
	for (i = 0; i < PLLDIV_MAX; i++) {
		if (i < 3)
			offset = pllctl_reg(data->pll, div1) + i;
		else
			offset = pllctl_reg(data->pll, div4) + (i - 3);

		if (divn_val[i] != -1) {
			__raw_writel(divn_val[i] | PLLDIV_ENABLE_MASK, offset);
			alnctl_val |= BIT(i);
		}
	}

	if (alnctl_val) {
		pllctl_reg_setbits(data->pll, alnctl, alnctl_val);
		/*
		 * Set GOSET bit in PLLCMD to initiate the GO operation
		 * to change the divide
		 */
		pllctl_reg_setbits(data->pll, cmd, PLLSTAT_GOSTAT_MASK);
		wait_for_completion(data);
	}

	/* Reset PLL */
	pllctl_reg_setbits(data->pll, ctl, PLLCTL_PLLRST_MASK);
	sdelay(21000);	/* Wait for a minimum of 7 us*/
	pllctl_reg_clrbits(data->pll, ctl, PLLCTL_PLLRST_MASK);
	sdelay(105000);	/* Wait for PLL Lock time (min 50 us) */

	/* Enable PLL */
	pllctl_reg_clrbits(data->pll, secctl, SECCTL_BYPASS_MASK);
	pllctl_reg_setbits(data->pll, ctl, PLLCTL_PLLEN_MASK);
}

void configure_secondary_pll(const struct pll_init_data *data)
{
	int pllod = data->pll_od - 1;

	/* Enable Glitch free bypass for ARM PLL */
	if (cpu_is_k2hk() && data->pll == TETRIS_PLL)
		clrbits_le32(KS2_MISC_CTRL, MISC_CTL1_ARM_PLL_EN);

	/* Enable Bypass mode */
	setbits_le32(keystone_pll_regs[data->pll].reg1, CFG_PLLCTL1_ENSAT_MASK);
	setbits_le32(keystone_pll_regs[data->pll].reg0,
		     CFG_PLLCTL0_BYPASS_MASK);

	configure_mult_div(data);

	/* Program Output Divider */
	clrsetbits_le32(keystone_pll_regs[data->pll].reg0,
			CFG_PLLCTL0_CLKOD_MASK,
			(pllod << CFG_PLLCTL0_CLKOD_SHIFT) &
			CFG_PLLCTL0_CLKOD_MASK);

	/* Reset PLL */
	setbits_le32(keystone_pll_regs[data->pll].reg1, CFG_PLLCTL1_RST_MASK);
	/* Wait for 5 micro seconds */
	sdelay(21000);

	/* Select the Output of PASS PLL as input to PASS */
	if (data->pll == PASS_PLL && cpu_is_k2hk())
		pll_pa_clk_sel();

	clrbits_le32(keystone_pll_regs[data->pll].reg1, CFG_PLLCTL1_RST_MASK);
	/* Wait for 500 * REFCLK cucles * (PLLD + 1) */
	sdelay(105000);

	/* Switch to PLL mode */
	clrbits_le32(keystone_pll_regs[data->pll].reg0,
		     CFG_PLLCTL0_BYPASS_MASK);

	/* Select the Output of ARM PLL as input to ARM */
	if (cpu_is_k2hk() && data->pll == TETRIS_PLL)
		setbits_le32(KS2_MISC_CTRL, MISC_CTL1_ARM_PLL_EN);
}

void init_pll(const struct pll_init_data *data)
{
	if (data->pll == MAIN_PLL)
		configure_main_pll(data);
	else
		configure_secondary_pll(data);

	/*
	 * This is required to provide a delay between multiple
	 * consequent PPL configurations
	 */
	sdelay(210000);
}

void init_plls(void)
{
	struct pll_init_data *data;
	int pll;

	for (pll = MAIN_PLL; pll < MAX_PLL_COUNT; pll++) {
		data = get_pll_init_data(pll);
		if (data)
			init_pll(data);
	}
}

static int get_max_speed(u32 val, u32 speed_supported, int *spds)
{
	int speed;

	/* Left most setbit gives the speed */
	for (speed = DEVSPEED_NUMSPDS; speed >= 0; speed--) {
		if ((val & BIT(speed)) & speed_supported)
			return spds[speed];
	}

	/* If no bit is set, return minimum speed */
	if (cpu_is_k2g())
		return SPD200;
	else
		return SPD800;
}

static inline u32 read_efuse_bootrom(void)
{
	if (cpu_is_k2hk() && (cpu_revision() <= 1))
		return __raw_readl(KS2_REV1_DEVSPEED);
	else
		return __raw_readl(KS2_EFUSE_BOOTROM);
}

int get_max_arm_speed(int *spds)
{
	u32 armspeed = read_efuse_bootrom();

	armspeed = (armspeed & DEVSPEED_ARMSPEED_MASK) >>
		    DEVSPEED_ARMSPEED_SHIFT;

	return get_max_speed(armspeed, ARM_SUPPORTED_SPEEDS, spds);
}

int get_max_dev_speed(int *spds)
{
	u32 devspeed = read_efuse_bootrom();

	devspeed = (devspeed & DEVSPEED_DEVSPEED_MASK) >>
		    DEVSPEED_DEVSPEED_SHIFT;

	return get_max_speed(devspeed, DEV_SUPPORTED_SPEEDS, spds);
}

/**
 * pll_freq_get - get pll frequency
 * @pll:	pll identifier
 */
static unsigned long pll_freq_get(int pll)
{
	unsigned long mult = 1, prediv = 1, output_div = 2;
	unsigned long ret;
	u32 tmp, reg;

	if (pll == MAIN_PLL) {
		ret = get_external_clk(sys_clk);
		if (pllctl_reg_read(pll, ctl) & PLLCTL_PLLEN_MASK) {
			/* PLL mode */
			tmp = __raw_readl(KS2_MAINPLLCTL0);
			prediv = (tmp & CFG_PLLCTL0_PLLD_MASK) + 1;
			mult = ((tmp & CFG_PLLCTL0_PLLM_HI_MASK) >>
				CFG_PLLCTL0_PLLM_SHIFT |
				(pllctl_reg_read(pll, mult) &
				 PLLM_MULT_LO_MASK)) + 1;
			output_div = ((pllctl_reg_read(pll, secctl) &
				       SECCTL_OP_DIV_MASK) >>
				       SECCTL_OP_DIV_SHIFT) + 1;

			ret = ret / prediv / output_div * mult;
		}
	} else {
		switch (pll) {
		case PASS_PLL:
			ret = get_external_clk(pa_clk);
			reg = KS2_PASSPLLCTL0;
			break;
		case TETRIS_PLL:
			ret = get_external_clk(tetris_clk);
			reg = KS2_ARMPLLCTL0;
			break;
		case DDR3A_PLL:
			ret = get_external_clk(ddr3a_clk);
			reg = KS2_DDR3APLLCTL0;
			break;
		case DDR3B_PLL:
			ret = get_external_clk(ddr3b_clk);
			reg = KS2_DDR3BPLLCTL0;
			break;
		case UART_PLL:
			ret = get_external_clk(uart_clk);
			reg = KS2_UARTPLLCTL0;
			break;
		default:
			return 0;
		}

		tmp = __raw_readl(reg);

		if (!(tmp & CFG_PLLCTL0_BYPASS_MASK)) {
			/* Bypass disabled */
			prediv = (tmp & CFG_PLLCTL0_PLLD_MASK) + 1;
			mult = ((tmp & CFG_PLLCTL0_PLLM_MASK) >>
				CFG_PLLCTL0_PLLM_SHIFT) + 1;
			output_div = ((tmp & CFG_PLLCTL0_CLKOD_MASK) >>
				      CFG_PLLCTL0_CLKOD_SHIFT) + 1;
			ret = ((ret / prediv) * mult) / output_div;
		}
	}

	return ret;
}

unsigned long ks_clk_get_rate(unsigned int clk)
{
	unsigned long freq = 0;

	switch (clk) {
	case core_pll_clk:
		freq = pll_freq_get(CORE_PLL);
		break;
	case pass_pll_clk:
		freq = pll_freq_get(PASS_PLL);
		break;
	case tetris_pll_clk:
		if (!cpu_is_k2e())
			freq = pll_freq_get(TETRIS_PLL);
		break;
	case ddr3a_pll_clk:
		freq = pll_freq_get(DDR3A_PLL);
		break;
	case ddr3b_pll_clk:
		if (cpu_is_k2hk())
			freq = pll_freq_get(DDR3B_PLL);
		break;
	case uart_pll_clk:
		if (cpu_is_k2g())
			freq = pll_freq_get(UART_PLL);
		break;
	case sys_clk0_1_clk:
	case sys_clk0_clk:
		freq = pll_freq_get(CORE_PLL) / pll0div_read(1);
		break;
	case sys_clk1_clk:
	return pll_freq_get(CORE_PLL) / pll0div_read(2);
		break;
	case sys_clk2_clk:
		freq = pll_freq_get(CORE_PLL) / pll0div_read(3);
		break;
	case sys_clk3_clk:
		freq = pll_freq_get(CORE_PLL) / pll0div_read(4);
		break;
	case sys_clk0_2_clk:
		freq = ks_clk_get_rate(sys_clk0_clk) / 2;
		break;
	case sys_clk0_3_clk:
		freq = ks_clk_get_rate(sys_clk0_clk) / 3;
		break;
	case sys_clk0_4_clk:
		freq = ks_clk_get_rate(sys_clk0_clk) / 4;
		break;
	case sys_clk0_6_clk:
		freq = ks_clk_get_rate(sys_clk0_clk) / 6;
		break;
	case sys_clk0_8_clk:
		freq = ks_clk_get_rate(sys_clk0_clk) / 8;
		break;
	case sys_clk0_12_clk:
		freq = ks_clk_get_rate(sys_clk0_clk) / 12;
		break;
	case sys_clk0_24_clk:
		freq = ks_clk_get_rate(sys_clk0_clk) / 24;
		break;
	case sys_clk1_3_clk:
		freq = ks_clk_get_rate(sys_clk1_clk) / 3;
		break;
	case sys_clk1_4_clk:
		freq = ks_clk_get_rate(sys_clk1_clk) / 4;
		break;
	case sys_clk1_6_clk:
		freq = ks_clk_get_rate(sys_clk1_clk) / 6;
		break;
	case sys_clk1_12_clk:
		freq = ks_clk_get_rate(sys_clk1_clk) / 12;
		break;
	default:
		break;
	}

	return freq;
}
