// SPDX-License-Identifier: GPL-2.0+
/*
 *
 * Clock initialization for OMAP4
 *
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 *
 * Based on previous work by:
 *	Santosh Shilimkar <santosh.shilimkar@ti.com>
 *	Rajendra Nayak <rnayak@ti.com>
 */
#include <common.h>
#include <hang.h>
#include <i2c.h>
#include <init.h>
#include <log.h>
#include <asm/omap_common.h>
#include <asm/gpio.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/utils.h>
#include <asm/omap_gpio.h>
#include <asm/emif.h>

#ifndef CONFIG_SPL_BUILD
/*
 * printing to console doesn't work unless
 * this code is executed from SPL
 */
#define printf(fmt, args...)
#define puts(s)
#endif

const u32 sys_clk_array[8] = {
	12000000,	       /* 12 MHz */
	20000000,		/* 20 MHz */
	16800000,	       /* 16.8 MHz */
	19200000,	       /* 19.2 MHz */
	26000000,	       /* 26 MHz */
	27000000,	       /* 27 MHz */
	38400000,	       /* 38.4 MHz */
};

static inline u32 __get_sys_clk_index(void)
{
	s8 ind;
	/*
	 * For ES1 the ROM code calibration of sys clock is not reliable
	 * due to hw issue. So, use hard-coded value. If this value is not
	 * correct for any board over-ride this function in board file
	 * From ES2.0 onwards you will get this information from
	 * CM_SYS_CLKSEL
	 */
	if (omap_revision() == OMAP4430_ES1_0)
		ind = OMAP_SYS_CLK_IND_38_4_MHZ;
	else {
		/* SYS_CLKSEL - 1 to match the dpll param array indices */
		ind = (readl((*prcm)->cm_sys_clksel) &
			CM_SYS_CLKSEL_SYS_CLKSEL_MASK) - 1;
	}
	return ind;
}

u32 get_sys_clk_index(void)
	__attribute__ ((weak, alias("__get_sys_clk_index")));

u32 get_sys_clk_freq(void)
{
	u8 index = get_sys_clk_index();
	return sys_clk_array[index];
}

void setup_post_dividers(u32 const base, const struct dpll_params *params)
{
	struct dpll_regs *const dpll_regs = (struct dpll_regs *)base;

	/* Setup post-dividers */
	if (params->m2 >= 0)
		writel(params->m2, &dpll_regs->cm_div_m2_dpll);
	if (params->m3 >= 0)
		writel(params->m3, &dpll_regs->cm_div_m3_dpll);
	if (params->m4_h11 >= 0)
		writel(params->m4_h11, &dpll_regs->cm_div_m4_h11_dpll);
	if (params->m5_h12 >= 0)
		writel(params->m5_h12, &dpll_regs->cm_div_m5_h12_dpll);
	if (params->m6_h13 >= 0)
		writel(params->m6_h13, &dpll_regs->cm_div_m6_h13_dpll);
	if (params->m7_h14 >= 0)
		writel(params->m7_h14, &dpll_regs->cm_div_m7_h14_dpll);
	if (params->h21 >= 0)
		writel(params->h21, &dpll_regs->cm_div_h21_dpll);
	if (params->h22 >= 0)
		writel(params->h22, &dpll_regs->cm_div_h22_dpll);
	if (params->h23 >= 0)
		writel(params->h23, &dpll_regs->cm_div_h23_dpll);
	if (params->h24 >= 0)
		writel(params->h24, &dpll_regs->cm_div_h24_dpll);
}

static inline void do_bypass_dpll(u32 const base)
{
	struct dpll_regs *dpll_regs = (struct dpll_regs *)base;

	clrsetbits_le32(&dpll_regs->cm_clkmode_dpll,
			CM_CLKMODE_DPLL_DPLL_EN_MASK,
			DPLL_EN_FAST_RELOCK_BYPASS <<
			CM_CLKMODE_DPLL_EN_SHIFT);
}

static inline void wait_for_bypass(u32 const base)
{
	struct dpll_regs *const dpll_regs = (struct dpll_regs *)base;

	if (!wait_on_value(ST_DPLL_CLK_MASK, 0, &dpll_regs->cm_idlest_dpll,
				LDELAY)) {
		printf("Bypassing DPLL failed %x\n", base);
	}
}

static inline void do_lock_dpll(u32 const base)
{
	struct dpll_regs *const dpll_regs = (struct dpll_regs *)base;

	clrsetbits_le32(&dpll_regs->cm_clkmode_dpll,
		      CM_CLKMODE_DPLL_DPLL_EN_MASK,
		      DPLL_EN_LOCK << CM_CLKMODE_DPLL_EN_SHIFT);
}

static inline void wait_for_lock(u32 const base)
{
	struct dpll_regs *const dpll_regs = (struct dpll_regs *)base;

	if (!wait_on_value(ST_DPLL_CLK_MASK, ST_DPLL_CLK_MASK,
		&dpll_regs->cm_idlest_dpll, LDELAY)) {
		printf("DPLL locking failed for %x\n", base);
		hang();
	}
}

inline u32 check_for_lock(u32 const base)
{
	struct dpll_regs *const dpll_regs = (struct dpll_regs *)base;
	u32 lock = readl(&dpll_regs->cm_idlest_dpll) & ST_DPLL_CLK_MASK;

	return lock;
}

const struct dpll_params *get_mpu_dpll_params(struct dplls const *dpll_data)
{
	u32 sysclk_ind = get_sys_clk_index();
	return &dpll_data->mpu[sysclk_ind];
}

const struct dpll_params *get_core_dpll_params(struct dplls const *dpll_data)
{
	u32 sysclk_ind = get_sys_clk_index();
	return &dpll_data->core[sysclk_ind];
}

const struct dpll_params *get_per_dpll_params(struct dplls const *dpll_data)
{
	u32 sysclk_ind = get_sys_clk_index();
	return &dpll_data->per[sysclk_ind];
}

const struct dpll_params *get_iva_dpll_params(struct dplls const *dpll_data)
{
	u32 sysclk_ind = get_sys_clk_index();
	return &dpll_data->iva[sysclk_ind];
}

const struct dpll_params *get_usb_dpll_params(struct dplls const *dpll_data)
{
	u32 sysclk_ind = get_sys_clk_index();
	return &dpll_data->usb[sysclk_ind];
}

const struct dpll_params *get_abe_dpll_params(struct dplls const *dpll_data)
{
#ifdef CONFIG_SYS_OMAP_ABE_SYSCK
	u32 sysclk_ind = get_sys_clk_index();
	return &dpll_data->abe[sysclk_ind];
#else
	return dpll_data->abe;
#endif
}

static const struct dpll_params *get_ddr_dpll_params
			(struct dplls const *dpll_data)
{
	u32 sysclk_ind = get_sys_clk_index();

	if (!dpll_data->ddr)
		return NULL;
	return &dpll_data->ddr[sysclk_ind];
}

#ifdef CONFIG_DRIVER_TI_CPSW
static const struct dpll_params *get_gmac_dpll_params
			(struct dplls const *dpll_data)
{
	u32 sysclk_ind = get_sys_clk_index();

	if (!dpll_data->gmac)
		return NULL;
	return &dpll_data->gmac[sysclk_ind];
}
#endif

static void do_setup_dpll(u32 const base, const struct dpll_params *params,
				u8 lock, char *dpll)
{
	u32 temp, M, N;
	struct dpll_regs *const dpll_regs = (struct dpll_regs *)base;

	if (!params)
		return;

	temp = readl(&dpll_regs->cm_clksel_dpll);

	if (check_for_lock(base)) {
		/*
		 * The Dpll has already been locked by rom code using CH.
		 * Check if M,N are matching with Ideal nominal opp values.
		 * If matches, skip the rest otherwise relock.
		 */
		M = (temp & CM_CLKSEL_DPLL_M_MASK) >> CM_CLKSEL_DPLL_M_SHIFT;
		N = (temp & CM_CLKSEL_DPLL_N_MASK) >> CM_CLKSEL_DPLL_N_SHIFT;
		if ((M != (params->m)) || (N != (params->n))) {
			debug("\n %s Dpll locked, but not for ideal M = %d,"
				"N = %d values, current values are M = %d,"
				"N= %d" , dpll, params->m, params->n,
				M, N);
		} else {
			/* Dpll locked with ideal values for nominal opps. */
			debug("\n %s Dpll already locked with ideal"
						"nominal opp values", dpll);

			bypass_dpll(base);
			goto setup_post_dividers;
		}
	}

	bypass_dpll(base);

	/* Set M & N */
	temp &= ~CM_CLKSEL_DPLL_M_MASK;
	temp |= (params->m << CM_CLKSEL_DPLL_M_SHIFT) & CM_CLKSEL_DPLL_M_MASK;

	temp &= ~CM_CLKSEL_DPLL_N_MASK;
	temp |= (params->n << CM_CLKSEL_DPLL_N_SHIFT) & CM_CLKSEL_DPLL_N_MASK;

	writel(temp, &dpll_regs->cm_clksel_dpll);

setup_post_dividers:
	setup_post_dividers(base, params);

	/* Lock */
	if (lock)
		do_lock_dpll(base);

	/* Wait till the DPLL locks */
	if (lock)
		wait_for_lock(base);
}

u32 omap_ddr_clk(void)
{
	u32 ddr_clk, sys_clk_khz, omap_rev, divider;
	const struct dpll_params *core_dpll_params;

	omap_rev = omap_revision();
	sys_clk_khz = get_sys_clk_freq() / 1000;

	core_dpll_params = get_core_dpll_params(*dplls_data);

	debug("sys_clk %d\n ", sys_clk_khz * 1000);

	/* Find Core DPLL locked frequency first */
	ddr_clk = sys_clk_khz * 2 * core_dpll_params->m /
			(core_dpll_params->n + 1);

	if (omap_rev < OMAP5430_ES1_0) {
		/*
		 * DDR frequency is PHY_ROOT_CLK/2
		 * PHY_ROOT_CLK = Fdpll/2/M2
		 */
		divider = 4;
	} else {
		/*
		 * DDR frequency is PHY_ROOT_CLK
		 * PHY_ROOT_CLK = Fdpll/2/M2
		 */
		divider = 2;
	}

	ddr_clk = ddr_clk / divider / core_dpll_params->m2;
	ddr_clk *= 1000;	/* convert to Hz */
	debug("ddr_clk %d\n ", ddr_clk);

	return ddr_clk;
}

/*
 * Lock MPU dpll
 *
 * Resulting MPU frequencies:
 * 4430 ES1.0	: 600 MHz
 * 4430 ES2.x	: 792 MHz (OPP Turbo)
 * 4460		: 920 MHz (OPP Turbo) - DCC disabled
 */
void configure_mpu_dpll(void)
{
	const struct dpll_params *params;
	struct dpll_regs *mpu_dpll_regs;
	u32 omap_rev;
	omap_rev = omap_revision();

	/*
	 * DCC and clock divider settings for 4460.
	 * DCC is required, if more than a certain frequency is required.
	 * For, 4460 > 1GHZ.
	 *     5430 > 1.4GHZ.
	 */
	if ((omap_rev >= OMAP4460_ES1_0) && (omap_rev < OMAP5430_ES1_0)) {
		mpu_dpll_regs =
			(struct dpll_regs *)((*prcm)->cm_clkmode_dpll_mpu);
		bypass_dpll((*prcm)->cm_clkmode_dpll_mpu);
		clrbits_le32((*prcm)->cm_mpu_mpu_clkctrl,
			MPU_CLKCTRL_CLKSEL_EMIF_DIV_MODE_MASK);
		setbits_le32((*prcm)->cm_mpu_mpu_clkctrl,
			MPU_CLKCTRL_CLKSEL_ABE_DIV_MODE_MASK);
		clrbits_le32(&mpu_dpll_regs->cm_clksel_dpll,
			CM_CLKSEL_DCC_EN_MASK);
	}

	params = get_mpu_dpll_params(*dplls_data);

	do_setup_dpll((*prcm)->cm_clkmode_dpll_mpu, params, DPLL_LOCK, "mpu");
	debug("MPU DPLL locked\n");
}

#if defined(CONFIG_USB_EHCI_OMAP) || defined(CONFIG_USB_XHCI_OMAP) || \
	defined(CONFIG_USB_MUSB_OMAP2PLUS)
static void setup_usb_dpll(void)
{
	const struct dpll_params *params;
	u32 sys_clk_khz, sd_div, num, den;

	sys_clk_khz = get_sys_clk_freq() / 1000;
	/*
	 * USB:
	 * USB dpll is J-type. Need to set DPLL_SD_DIV for jitter correction
	 * DPLL_SD_DIV = CEILING ([DPLL_MULT/(DPLL_DIV+1)]* CLKINP / 250)
	 *      - where CLKINP is sys_clk in MHz
	 * Use CLKINP in KHz and adjust the denominator accordingly so
	 * that we have enough accuracy and at the same time no overflow
	 */
	params = get_usb_dpll_params(*dplls_data);
	num = params->m * sys_clk_khz;
	den = (params->n + 1) * 250 * 1000;
	num += den - 1;
	sd_div = num / den;
	clrsetbits_le32((*prcm)->cm_clksel_dpll_usb,
			CM_CLKSEL_DPLL_DPLL_SD_DIV_MASK,
			sd_div << CM_CLKSEL_DPLL_DPLL_SD_DIV_SHIFT);

	/* Now setup the dpll with the regular function */
	do_setup_dpll((*prcm)->cm_clkmode_dpll_usb, params, DPLL_LOCK, "usb");
}
#endif

static void setup_dplls(void)
{
	u32 temp;
	const struct dpll_params *params;
	struct emif_reg_struct *emif = (struct emif_reg_struct *)EMIF1_BASE;

	debug("setup_dplls\n");

	/* CORE dpll */
	params = get_core_dpll_params(*dplls_data);	/* default - safest */
	/*
	 * Do not lock the core DPLL now. Just set it up.
	 * Core DPLL will be locked after setting up EMIF
	 * using the FREQ_UPDATE method(freq_update_core())
	 */
	if (emif_sdram_type(readl(&emif->emif_sdram_config)) ==
	    EMIF_SDRAM_TYPE_LPDDR2)
		do_setup_dpll((*prcm)->cm_clkmode_dpll_core, params,
							DPLL_NO_LOCK, "core");
	else
		do_setup_dpll((*prcm)->cm_clkmode_dpll_core, params,
							DPLL_LOCK, "core");
	/* Set the ratios for CORE_CLK, L3_CLK, L4_CLK */
	temp = (CLKSEL_CORE_X2_DIV_1 << CLKSEL_CORE_SHIFT) |
	    (CLKSEL_L3_CORE_DIV_2 << CLKSEL_L3_SHIFT) |
	    (CLKSEL_L4_L3_DIV_2 << CLKSEL_L4_SHIFT);
	writel(temp, (*prcm)->cm_clksel_core);
	debug("Core DPLL configured\n");

	/* lock PER dpll */
	params = get_per_dpll_params(*dplls_data);
	do_setup_dpll((*prcm)->cm_clkmode_dpll_per,
			params, DPLL_LOCK, "per");
	debug("PER DPLL locked\n");

	/* MPU dpll */
	configure_mpu_dpll();

#if defined(CONFIG_USB_EHCI_OMAP) || defined(CONFIG_USB_XHCI_OMAP) || \
	defined(CONFIG_USB_MUSB_OMAP2PLUS)
	setup_usb_dpll();
#endif
	params = get_ddr_dpll_params(*dplls_data);
	do_setup_dpll((*prcm)->cm_clkmode_dpll_ddrphy,
		      params, DPLL_LOCK, "ddr");

#ifdef CONFIG_DRIVER_TI_CPSW
	params = get_gmac_dpll_params(*dplls_data);
	do_setup_dpll((*prcm)->cm_clkmode_dpll_gmac, params,
		      DPLL_LOCK, "gmac");
#endif
}

u32 get_offset_code(u32 volt_offset, struct pmic_data *pmic)
{
	u32 offset_code;

	volt_offset -= pmic->base_offset;

	offset_code = (volt_offset + pmic->step - 1) / pmic->step;

	/*
	 * Offset codes 1-6 all give the base voltage in Palmas
	 * Offset code 0 switches OFF the SMPS
	 */
	return offset_code + pmic->start_code;
}

void do_scale_vcore(u32 vcore_reg, u32 volt_mv, struct pmic_data *pmic)
{
	u32 offset_code;
	u32 offset = volt_mv;
	int ret = 0;

	if (!volt_mv)
		return;

	pmic->pmic_bus_init();
	/* See if we can first get the GPIO if needed */
	if (pmic->gpio_en)
		ret = gpio_request(pmic->gpio, "PMIC_GPIO");

	if (ret < 0) {
		printf("%s: gpio %d request failed %d\n", __func__,
							pmic->gpio, ret);
		return;
	}

	/* Pull the GPIO low to select SET0 register, while we program SET1 */
	if (pmic->gpio_en)
		gpio_direction_output(pmic->gpio, 0);

	/* convert to uV for better accuracy in the calculations */
	offset *= 1000;

	offset_code = get_offset_code(offset, pmic);

	debug("do_scale_vcore: volt - %d offset_code - 0x%x\n", volt_mv,
		offset_code);

	if (pmic->pmic_write(pmic->i2c_slave_addr, vcore_reg, offset_code))
		printf("Scaling voltage failed for 0x%x\n", vcore_reg);
	if (pmic->gpio_en)
		gpio_direction_output(pmic->gpio, 1);
}

int __weak get_voltrail_opp(int rail_offset)
{
	/*
	 * By default return OPP_NOM for all voltage rails.
	 */
	return OPP_NOM;
}

static u32 optimize_vcore_voltage(struct volts const *v, int opp)
{
	u32 val;

	if (!v->value[opp])
		return 0;
	if (!v->efuse.reg[opp])
		return v->value[opp];

	switch (v->efuse.reg_bits) {
	case 16:
		val = readw(v->efuse.reg[opp]);
		break;
	case 32:
		val = readl(v->efuse.reg[opp]);
		break;
	default:
		printf("Error: efuse 0x%08x bits=%d unknown\n",
		       v->efuse.reg[opp], v->efuse.reg_bits);
		return v->value[opp];
	}

	if (!val) {
		printf("Error: efuse 0x%08x bits=%d val=0, using %d\n",
		       v->efuse.reg[opp], v->efuse.reg_bits, v->value[opp]);
		return v->value[opp];
	}

	debug("%s:efuse 0x%08x bits=%d Vnom=%d, using efuse value %d\n",
	      __func__, v->efuse.reg[opp], v->efuse.reg_bits, v->value[opp],
	      val);
	return val;
}

#ifdef CONFIG_IODELAY_RECALIBRATION
void __weak recalibrate_iodelay(void)
{
}
#endif

/*
 * Setup the voltages for the main SoC core power domains.
 * We start with the maximum voltages allowed here, as set in the corresponding
 * vcores_data struct, and then scale (usually down) to the fused values that
 * are retrieved from the SoC. The scaling happens only if the efuse.reg fields
 * are initialised.
 * Rail grouping is supported for the DRA7xx SoCs only, therefore the code is
 * compiled conditionally. Note that the new code writes the scaled (or zeroed)
 * values back to the vcores_data struct for eventual reuse. Zero values mean
 * that the corresponding rails are not controlled separately, and are not sent
 * to the PMIC.
 */
void scale_vcores(struct vcores_data const *vcores)
{
	int i, opp, j, ol;
	struct volts *pv = (struct volts *)vcores;
	struct volts *px;

	for (i=0; i<(sizeof(struct vcores_data)/sizeof(struct volts)); i++) {
		opp = get_voltrail_opp(i);
		debug("%d -> ", pv->value[opp]);

		if (pv->value[opp]) {
			/* Handle non-empty members only */
			pv->value[opp] = optimize_vcore_voltage(pv, opp);
			px = (struct volts *)vcores;
			j = 0;
			while (px < pv) {
				/*
				 * Scan already handled non-empty members to see
				 * if we have a group and find the max voltage,
				 * which is set to the first occurance of the
				 * particular SMPS; the other group voltages are
				 * zeroed.
				 */
				ol = get_voltrail_opp(j);
				if (px->value[ol] &&
				    (pv->pmic->i2c_slave_addr ==
				     px->pmic->i2c_slave_addr) &&
				    (pv->addr == px->addr)) {
					/* Same PMIC, same SMPS */
					if (pv->value[opp] > px->value[ol])
						px->value[ol] = pv->value[opp];

					pv->value[opp] = 0;
				}
				px++;
				j++;
			}
		}
		debug("%d\n", pv->value[opp]);
		pv++;
	}

	opp = get_voltrail_opp(VOLT_CORE);
	debug("cor: %d\n", vcores->core.value[opp]);
	do_scale_vcore(vcores->core.addr, vcores->core.value[opp],
		       vcores->core.pmic);
	/*
	 * IO delay recalibration should be done immediately after
	 * adjusting AVS voltages for VDD_CORE_L.
	 * Respective boards should call __recalibrate_iodelay()
	 * with proper mux, virtual and manual mode configurations.
	 */
#ifdef CONFIG_IODELAY_RECALIBRATION
	recalibrate_iodelay();
#endif

	opp = get_voltrail_opp(VOLT_MPU);
	debug("mpu: %d\n", vcores->mpu.value[opp]);
	do_scale_vcore(vcores->mpu.addr, vcores->mpu.value[opp],
		       vcores->mpu.pmic);
	/* Configure MPU ABB LDO after scale */
	abb_setup(vcores->mpu.efuse.reg[opp],
		  (*ctrl)->control_wkup_ldovbb_mpu_voltage_ctrl,
		  (*prcm)->prm_abbldo_mpu_setup,
		  (*prcm)->prm_abbldo_mpu_ctrl,
		  (*prcm)->prm_irqstatus_mpu_2,
		  vcores->mpu.abb_tx_done_mask,
		  OMAP_ABB_FAST_OPP);

	opp = get_voltrail_opp(VOLT_MM);
	debug("mm: %d\n", vcores->mm.value[opp]);
	do_scale_vcore(vcores->mm.addr, vcores->mm.value[opp],
		       vcores->mm.pmic);
	/* Configure MM ABB LDO after scale */
	abb_setup(vcores->mm.efuse.reg[opp],
		  (*ctrl)->control_wkup_ldovbb_mm_voltage_ctrl,
		  (*prcm)->prm_abbldo_mm_setup,
		  (*prcm)->prm_abbldo_mm_ctrl,
		  (*prcm)->prm_irqstatus_mpu,
		  vcores->mm.abb_tx_done_mask,
		  OMAP_ABB_FAST_OPP);

	opp = get_voltrail_opp(VOLT_GPU);
	debug("gpu: %d\n", vcores->gpu.value[opp]);
	do_scale_vcore(vcores->gpu.addr, vcores->gpu.value[opp],
		       vcores->gpu.pmic);
	/* Configure GPU ABB LDO after scale */
	abb_setup(vcores->gpu.efuse.reg[opp],
		  (*ctrl)->control_wkup_ldovbb_gpu_voltage_ctrl,
		  (*prcm)->prm_abbldo_gpu_setup,
		  (*prcm)->prm_abbldo_gpu_ctrl,
		  (*prcm)->prm_irqstatus_mpu,
		  vcores->gpu.abb_tx_done_mask,
		  OMAP_ABB_FAST_OPP);

	opp = get_voltrail_opp(VOLT_EVE);
	debug("eve: %d\n", vcores->eve.value[opp]);
	do_scale_vcore(vcores->eve.addr, vcores->eve.value[opp],
		       vcores->eve.pmic);
	/* Configure EVE ABB LDO after scale */
	abb_setup(vcores->eve.efuse.reg[opp],
		  (*ctrl)->control_wkup_ldovbb_eve_voltage_ctrl,
		  (*prcm)->prm_abbldo_eve_setup,
		  (*prcm)->prm_abbldo_eve_ctrl,
		  (*prcm)->prm_irqstatus_mpu,
		  vcores->eve.abb_tx_done_mask,
		  OMAP_ABB_FAST_OPP);

	opp = get_voltrail_opp(VOLT_IVA);
	debug("iva: %d\n", vcores->iva.value[opp]);
	do_scale_vcore(vcores->iva.addr, vcores->iva.value[opp],
		       vcores->iva.pmic);
	/* Configure IVA ABB LDO after scale */
	abb_setup(vcores->iva.efuse.reg[opp],
		  (*ctrl)->control_wkup_ldovbb_iva_voltage_ctrl,
		  (*prcm)->prm_abbldo_iva_setup,
		  (*prcm)->prm_abbldo_iva_ctrl,
		  (*prcm)->prm_irqstatus_mpu,
		  vcores->iva.abb_tx_done_mask,
		  OMAP_ABB_FAST_OPP);
}

static inline void enable_clock_domain(u32 const clkctrl_reg, u32 enable_mode)
{
	clrsetbits_le32(clkctrl_reg, CD_CLKCTRL_CLKTRCTRL_MASK,
			enable_mode << CD_CLKCTRL_CLKTRCTRL_SHIFT);
	debug("Enable clock domain - %x\n", clkctrl_reg);
}

static inline void disable_clock_domain(u32 const clkctrl_reg)
{
	clrsetbits_le32(clkctrl_reg, CD_CLKCTRL_CLKTRCTRL_MASK,
			CD_CLKCTRL_CLKTRCTRL_SW_SLEEP <<
			CD_CLKCTRL_CLKTRCTRL_SHIFT);
	debug("Disable clock domain - %x\n", clkctrl_reg);
}

static inline void wait_for_clk_enable(u32 clkctrl_addr)
{
	u32 clkctrl, idlest = MODULE_CLKCTRL_IDLEST_DISABLED;
	u32 bound = LDELAY;

	while ((idlest == MODULE_CLKCTRL_IDLEST_DISABLED) ||
		(idlest == MODULE_CLKCTRL_IDLEST_TRANSITIONING)) {

		clkctrl = readl(clkctrl_addr);
		idlest = (clkctrl & MODULE_CLKCTRL_IDLEST_MASK) >>
			 MODULE_CLKCTRL_IDLEST_SHIFT;
		if (--bound == 0) {
			printf("Clock enable failed for 0x%x idlest 0x%x\n",
				clkctrl_addr, clkctrl);
			return;
		}
	}
}

static inline void enable_clock_module(u32 const clkctrl_addr, u32 enable_mode,
				u32 wait_for_enable)
{
	clrsetbits_le32(clkctrl_addr, MODULE_CLKCTRL_MODULEMODE_MASK,
			enable_mode << MODULE_CLKCTRL_MODULEMODE_SHIFT);
	debug("Enable clock module - %x\n", clkctrl_addr);
	if (wait_for_enable)
		wait_for_clk_enable(clkctrl_addr);
}

static inline void wait_for_clk_disable(u32 clkctrl_addr)
{
	u32 clkctrl, idlest = MODULE_CLKCTRL_IDLEST_FULLY_FUNCTIONAL;
	u32 bound = LDELAY;

	while ((idlest != MODULE_CLKCTRL_IDLEST_DISABLED)) {
		clkctrl = readl(clkctrl_addr);
		idlest = (clkctrl & MODULE_CLKCTRL_IDLEST_MASK) >>
			 MODULE_CLKCTRL_IDLEST_SHIFT;
		if (--bound == 0) {
			printf("Clock disable failed for 0x%x idlest 0x%x\n",
			       clkctrl_addr, clkctrl);
			return;
		}
	}
}

static inline void disable_clock_module(u32 const clkctrl_addr,
					u32 wait_for_disable)
{
	clrsetbits_le32(clkctrl_addr, MODULE_CLKCTRL_MODULEMODE_MASK,
			MODULE_CLKCTRL_MODULEMODE_SW_DISABLE <<
			MODULE_CLKCTRL_MODULEMODE_SHIFT);
	debug("Disable clock module - %x\n", clkctrl_addr);
	if (wait_for_disable)
		wait_for_clk_disable(clkctrl_addr);
}

void freq_update_core(void)
{
	u32 freq_config1 = 0;
	const struct dpll_params *core_dpll_params;
	u32 omap_rev = omap_revision();

	core_dpll_params = get_core_dpll_params(*dplls_data);
	/* Put EMIF clock domain in sw wakeup mode */
	enable_clock_domain((*prcm)->cm_memif_clkstctrl,
				CD_CLKCTRL_CLKTRCTRL_SW_WKUP);
	wait_for_clk_enable((*prcm)->cm_memif_emif_1_clkctrl);
	wait_for_clk_enable((*prcm)->cm_memif_emif_2_clkctrl);

	freq_config1 = SHADOW_FREQ_CONFIG1_FREQ_UPDATE_MASK |
	    SHADOW_FREQ_CONFIG1_DLL_RESET_MASK;

	freq_config1 |= (DPLL_EN_LOCK << SHADOW_FREQ_CONFIG1_DPLL_EN_SHIFT) &
				SHADOW_FREQ_CONFIG1_DPLL_EN_MASK;

	freq_config1 |= (core_dpll_params->m2 <<
			SHADOW_FREQ_CONFIG1_M2_DIV_SHIFT) &
			SHADOW_FREQ_CONFIG1_M2_DIV_MASK;

	writel(freq_config1, (*prcm)->cm_shadow_freq_config1);
	if (!wait_on_value(SHADOW_FREQ_CONFIG1_FREQ_UPDATE_MASK, 0,
			(u32 *) (*prcm)->cm_shadow_freq_config1, LDELAY)) {
		puts("FREQ UPDATE procedure failed!!");
		hang();
	}

	/*
	 * Putting EMIF in HW_AUTO is seen to be causing issues with
	 * EMIF clocks and the master DLL. Keep EMIF in SW_WKUP
	 * in OMAP5430 ES1.0 silicon
	 */
	if (omap_rev != OMAP5430_ES1_0) {
		/* Put EMIF clock domain back in hw auto mode */
		enable_clock_domain((*prcm)->cm_memif_clkstctrl,
					CD_CLKCTRL_CLKTRCTRL_HW_AUTO);
		wait_for_clk_enable((*prcm)->cm_memif_emif_1_clkctrl);
		wait_for_clk_enable((*prcm)->cm_memif_emif_2_clkctrl);
	}
}

void bypass_dpll(u32 const base)
{
	do_bypass_dpll(base);
	wait_for_bypass(base);
}

void lock_dpll(u32 const base)
{
	do_lock_dpll(base);
	wait_for_lock(base);
}

static void setup_clocks_for_console(void)
{
	/* Do not add any spl_debug prints in this function */
	clrsetbits_le32((*prcm)->cm_l4per_clkstctrl, CD_CLKCTRL_CLKTRCTRL_MASK,
			CD_CLKCTRL_CLKTRCTRL_SW_WKUP <<
			CD_CLKCTRL_CLKTRCTRL_SHIFT);

	/* Enable all UARTs - console will be on one of them */
	clrsetbits_le32((*prcm)->cm_l4per_uart1_clkctrl,
			MODULE_CLKCTRL_MODULEMODE_MASK,
			MODULE_CLKCTRL_MODULEMODE_SW_EXPLICIT_EN <<
			MODULE_CLKCTRL_MODULEMODE_SHIFT);

	clrsetbits_le32((*prcm)->cm_l4per_uart2_clkctrl,
			MODULE_CLKCTRL_MODULEMODE_MASK,
			MODULE_CLKCTRL_MODULEMODE_SW_EXPLICIT_EN <<
			MODULE_CLKCTRL_MODULEMODE_SHIFT);

	clrsetbits_le32((*prcm)->cm_l4per_uart3_clkctrl,
			MODULE_CLKCTRL_MODULEMODE_MASK,
			MODULE_CLKCTRL_MODULEMODE_SW_EXPLICIT_EN <<
			MODULE_CLKCTRL_MODULEMODE_SHIFT);

	clrsetbits_le32((*prcm)->cm_l4per_uart4_clkctrl,
			MODULE_CLKCTRL_MODULEMODE_MASK,
			MODULE_CLKCTRL_MODULEMODE_SW_EXPLICIT_EN <<
			MODULE_CLKCTRL_MODULEMODE_SHIFT);

	clrsetbits_le32((*prcm)->cm_l4per_clkstctrl, CD_CLKCTRL_CLKTRCTRL_MASK,
			CD_CLKCTRL_CLKTRCTRL_HW_AUTO <<
			CD_CLKCTRL_CLKTRCTRL_SHIFT);
}

void do_enable_clocks(u32 const *clk_domains,
			    u32 const *clk_modules_hw_auto,
			    u32 const *clk_modules_explicit_en,
			    u8 wait_for_enable)
{
	u32 i, max = 100;

	/* Put the clock domains in SW_WKUP mode */
	for (i = 0; (i < max) && clk_domains && clk_domains[i]; i++) {
		enable_clock_domain(clk_domains[i],
				    CD_CLKCTRL_CLKTRCTRL_SW_WKUP);
	}

	/* Clock modules that need to be put in HW_AUTO */
	for (i = 0; (i < max) && clk_modules_hw_auto &&
		     clk_modules_hw_auto[i]; i++) {
		enable_clock_module(clk_modules_hw_auto[i],
				    MODULE_CLKCTRL_MODULEMODE_HW_AUTO,
				    wait_for_enable);
	};

	/* Clock modules that need to be put in SW_EXPLICIT_EN mode */
	for (i = 0; (i < max) && clk_modules_explicit_en &&
		     clk_modules_explicit_en[i]; i++) {
		enable_clock_module(clk_modules_explicit_en[i],
				    MODULE_CLKCTRL_MODULEMODE_SW_EXPLICIT_EN,
				    wait_for_enable);
	};

	/* Put the clock domains in HW_AUTO mode now */
	for (i = 0; (i < max) && clk_domains && clk_domains[i]; i++) {
		enable_clock_domain(clk_domains[i],
				    CD_CLKCTRL_CLKTRCTRL_HW_AUTO);
	}
}

void do_enable_ipu_clocks(u32 const *clk_domains,
			  u32 const *clk_modules_hw_auto,
			  u32 const *clk_modules_explicit_en,
			  u8 wait_for_enable)
{
	u32 i, max = 10;

	if (!IS_ENABLED(CONFIG_REMOTEPROC_TI_IPU))
		return;

	/* Put the clock domains in SW_WKUP mode */
	for (i = 0; (i < max) && clk_domains && clk_domains[i]; i++) {
		enable_clock_domain(clk_domains[i],
				    CD_CLKCTRL_CLKTRCTRL_SW_WKUP);
	}

	/* Clock modules that need to be put in HW_AUTO */
	for (i = 0; (i < max) && clk_modules_hw_auto &&
	     clk_modules_hw_auto[i]; i++) {
		enable_clock_module(clk_modules_hw_auto[i],
				    MODULE_CLKCTRL_MODULEMODE_HW_AUTO,
				    wait_for_enable);
	};

	/* Clock modules that need to be put in SW_EXPLICIT_EN mode */
	for (i = 0; (i < max) && clk_modules_explicit_en &&
	     clk_modules_explicit_en[i]; i++) {
		enable_clock_module(clk_modules_explicit_en[i],
				    MODULE_CLKCTRL_MODULEMODE_SW_EXPLICIT_EN,
				    wait_for_enable);
	};
}

void do_disable_clocks(u32 const *clk_domains,
			    u32 const *clk_modules_disable,
			    u8 wait_for_disable)
{
	u32 i, max = 100;


	/* Clock modules that need to be put in SW_DISABLE */
	for (i = 0; (i < max) && clk_modules_disable[i]; i++)
		disable_clock_module(clk_modules_disable[i],
				     wait_for_disable);

	/* Put the clock domains in SW_SLEEP mode */
	for (i = 0; (i < max) && clk_domains[i]; i++)
		disable_clock_domain(clk_domains[i]);
}

/**
 * setup_early_clocks() - Setup early clocks needed for SoC
 *
 * Setup clocks for console, SPL basic initialization clocks and initialize
 * the timer. This is invoked prior prcm_init.
 */
void setup_early_clocks(void)
{
	switch (omap_hw_init_context()) {
	case OMAP_INIT_CONTEXT_SPL:
	case OMAP_INIT_CONTEXT_UBOOT_FROM_NOR:
	case OMAP_INIT_CONTEXT_UBOOT_AFTER_CH:
		setup_clocks_for_console();
		enable_basic_clocks();
		timer_init();
		/* Fall through */
	}
}

void prcm_init(void)
{
	switch (omap_hw_init_context()) {
	case OMAP_INIT_CONTEXT_SPL:
	case OMAP_INIT_CONTEXT_UBOOT_FROM_NOR:
	case OMAP_INIT_CONTEXT_UBOOT_AFTER_CH:
		scale_vcores(*omap_vcores);
		setup_dplls();
		setup_warmreset_time();
		break;
	default:
		break;
	}

	if (OMAP_INIT_CONTEXT_SPL != omap_hw_init_context())
		enable_basic_uboot_clocks();
}

#if !CONFIG_IS_ENABLED(DM_I2C)
void gpi2c_init(void)
{
	static int gpi2c = 1;

	if (gpi2c) {
		i2c_init(CONFIG_SYS_I2C_SPEED,
			 CONFIG_SYS_I2C_SLAVE);
		gpi2c = 0;
	}
}
#endif
