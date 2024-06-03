// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015
 * Texas Instruments Incorporated, <www.ti.com>
 *
 * Lokesh Vutla <lokeshvutla@ti.com>
 */

#include <common.h>
#include <asm/utils.h>
#include <asm/arch/dra7xx_iodelay.h>
#include <asm/arch/omap.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/clock.h>
#include <asm/arch/mux_dra7xx.h>
#include <asm/omap_common.h>

static int isolate_io(u32 isolate)
{
	if (isolate) {
		clrsetbits_le32((*ctrl)->control_pbias, SDCARD_PWRDNZ,
				SDCARD_PWRDNZ);
		clrsetbits_le32((*ctrl)->control_pbias, SDCARD_BIAS_PWRDNZ,
				SDCARD_BIAS_PWRDNZ);
	}

	/* Override control on ISOCLKIN signal to IO pad ring. */
	clrsetbits_le32((*prcm)->prm_io_pmctrl, PMCTRL_ISOCLK_OVERRIDE_MASK,
			PMCTRL_ISOCLK_OVERRIDE_CTRL);
	if (!wait_on_value(PMCTRL_ISOCLK_STATUS_MASK, PMCTRL_ISOCLK_STATUS_MASK,
			   (u32 *)(*prcm)->prm_io_pmctrl, LDELAY))
		return ERR_DEISOLATE_IO << isolate;

	/* Isolate/Deisolate IO */
	clrsetbits_le32((*ctrl)->ctrl_core_sma_sw_0, CTRL_ISOLATE_MASK,
			isolate << CTRL_ISOLATE_SHIFT);
	/* Dummy read to add delay t > 10ns */
	readl((*ctrl)->ctrl_core_sma_sw_0);

	/* Return control on ISOCLKIN to hardware */
	clrsetbits_le32((*prcm)->prm_io_pmctrl, PMCTRL_ISOCLK_OVERRIDE_MASK,
			PMCTRL_ISOCLK_NOT_OVERRIDE_CTRL);
	if (!wait_on_value(PMCTRL_ISOCLK_STATUS_MASK,
			   0 << PMCTRL_ISOCLK_STATUS_SHIFT,
			   (u32 *)(*prcm)->prm_io_pmctrl, LDELAY))
		return ERR_DEISOLATE_IO << isolate;

	return 0;
}

static int calibrate_iodelay(u32 base)
{
	u32 reg;

	/* Configure REFCLK period */
	reg = readl(base + CFG_REG_2_OFFSET);
	reg &= ~CFG_REG_REFCLK_PERIOD_MASK;
	reg |= CFG_REG_REFCLK_PERIOD;
	writel(reg, base + CFG_REG_2_OFFSET);

	/* Initiate Calibration */
	clrsetbits_le32(base + CFG_REG_0_OFFSET, CFG_REG_CALIB_STRT_MASK,
			CFG_REG_CALIB_STRT << CFG_REG_CALIB_STRT_SHIFT);
	if (!wait_on_value(CFG_REG_CALIB_STRT_MASK, CFG_REG_CALIB_END,
			   (u32 *)(base + CFG_REG_0_OFFSET), LDELAY))
		return ERR_CALIBRATE_IODELAY;

	return 0;
}

static int update_delay_mechanism(u32 base)
{
	/* Initiate the reload of calibrated values. */
	clrsetbits_le32(base + CFG_REG_0_OFFSET, CFG_REG_ROM_READ_MASK,
			CFG_REG_ROM_READ_START);
	if (!wait_on_value(CFG_REG_ROM_READ_MASK, CFG_REG_ROM_READ_END,
			   (u32 *)(base + CFG_REG_0_OFFSET), LDELAY))
		return ERR_UPDATE_DELAY;

	return 0;
}

static u32 calculate_delay(u32 base, u16 offset, u16 den)
{
	u16 refclk_period, dly_cnt, ref_cnt;
	u32 reg, q, r;

	refclk_period = readl(base + CFG_REG_2_OFFSET) &
			      CFG_REG_REFCLK_PERIOD_MASK;

	reg = readl(base + offset);
	dly_cnt = (reg & CFG_REG_DLY_CNT_MASK) >> CFG_REG_DLY_CNT_SHIFT;
	ref_cnt = (reg & CFG_REG_REF_CNT_MASK) >> CFG_REG_REF_CNT_SHIFT;

	if (!dly_cnt || !den)
		return 0;

	/*
	 * To avoid overflow and integer truncation, delay value
	 * is calculated as quotient + remainder.
	 */
	q = 5 * ((ref_cnt * refclk_period) / (dly_cnt * den));
	r = (10 * ((ref_cnt * refclk_period) % (dly_cnt * den))) /
		(2 * dly_cnt * den);

	return q + r;
}

static u32 get_cfg_reg(u16 a_delay, u16 g_delay, u32 cpde, u32 fpde)
{
	u32 g_delay_coarse, g_delay_fine;
	u32 a_delay_coarse, a_delay_fine;
	u32 c_elements, f_elements;
	u32 total_delay, reg = 0;

	g_delay_coarse = g_delay / 920;
	g_delay_fine = ((g_delay % 920) * 10) / 60;

	a_delay_coarse = a_delay / cpde;
	a_delay_fine = ((a_delay % cpde) * 10) / fpde;

	c_elements = g_delay_coarse + a_delay_coarse;
	f_elements = (g_delay_fine + a_delay_fine) / 10;

	if (f_elements > 22) {
		total_delay = c_elements * cpde + f_elements * fpde;

		c_elements = total_delay / cpde;
		f_elements = (total_delay % cpde) / fpde;
	}

	reg = (c_elements << CFG_X_COARSE_DLY_SHIFT) & CFG_X_COARSE_DLY_MASK;
	reg |= (f_elements << CFG_X_FINE_DLY_SHIFT) & CFG_X_FINE_DLY_MASK;
	reg |= CFG_X_SIGNATURE << CFG_X_SIGNATURE_SHIFT;
	reg |= CFG_X_LOCK << CFG_X_LOCK_SHIFT;

	return reg;
}

int do_set_iodelay(u32 base, struct iodelay_cfg_entry const *array,
		   int niodelays)
{
	struct iodelay_cfg_entry *iodelay = (struct iodelay_cfg_entry *)array;
	u32 reg, cpde, fpde, i;

	if (!niodelays)
		return 0;

	cpde = calculate_delay((*ctrl)->iodelay_config_base, CFG_REG_3_OFFSET,
			       88);
	if (!cpde)
		return ERR_CPDE;

	fpde = calculate_delay((*ctrl)->iodelay_config_base, CFG_REG_4_OFFSET,
			       264);
	if (!fpde)
		return ERR_FPDE;

	for (i = 0; i < niodelays; i++, iodelay++) {
		reg = get_cfg_reg(iodelay->a_delay, iodelay->g_delay, cpde,
				  fpde);
		writel(reg, base + iodelay->offset);
	}

	return 0;
}

int __recalibrate_iodelay_start(void)
{
	int ret = 0;

	/* IO recalibration should be done only from SRAM */
	if (OMAP_INIT_CONTEXT_SPL != omap_hw_init_context()) {
		puts("IODELAY recalibration called from invalid context - use only from SPL in SRAM\n");
		return -1;
	}

	/* unlock IODELAY CONFIG registers */
	writel(CFG_IODELAY_UNLOCK_KEY, (*ctrl)->iodelay_config_base +
	       CFG_REG_8_OFFSET);

	ret = calibrate_iodelay((*ctrl)->iodelay_config_base);
	if (ret)
		goto err;

	ret = isolate_io(ISOLATE_IO);
	if (ret)
		goto err;

	ret = update_delay_mechanism((*ctrl)->iodelay_config_base);

err:
	return ret;
}

void __recalibrate_iodelay_end(int ret)
{

	/* IO recalibration should be done only from SRAM */
	if (OMAP_INIT_CONTEXT_SPL != omap_hw_init_context()) {
		puts("IODELAY recalibration called from invalid context - use only from SPL in SRAM\n");
		return;
	}

	if (!ret)
		ret = isolate_io(DEISOLATE_IO);

	/* lock IODELAY CONFIG registers */
	writel(CFG_IODELAY_LOCK_KEY, (*ctrl)->iodelay_config_base +
	       CFG_REG_8_OFFSET);

	/*
	 * UART cannot be used during IO recalibration sequence as IOs are in
	 * isolation. So error handling and debug prints are done after
	 * complete IO delay recalibration sequence
	 */
	switch (ret) {
	case ERR_CALIBRATE_IODELAY:
		puts("IODELAY: IO delay calibration sequence failed\n");
		break;
	case ERR_ISOLATE_IO:
		puts("IODELAY: Isolation of Device IOs failed\n");
		break;
	case ERR_UPDATE_DELAY:
		puts("IODELAY: Delay mechanism update with new calibrated values failed\n");
		break;
	case ERR_DEISOLATE_IO:
		puts("IODELAY: De-isolation of Device IOs failed\n");
		break;
	case ERR_CPDE:
		puts("IODELAY: CPDE calculation failed\n");
		break;
	case ERR_FPDE:
		puts("IODELAY: FPDE calculation failed\n");
		break;
	case -1:
		puts("IODELAY: Wrong Context call?\n");
		break;
	default:
		debug("IODELAY: IO delay recalibration successfully completed\n");
	}

	return;
}

void __recalibrate_iodelay(struct pad_conf_entry const *pad, int npads,
			   struct iodelay_cfg_entry const *iodelay,
			   int niodelays)
{
	int ret = 0;

	/* IO recalibration should be done only from SRAM */
	if (OMAP_INIT_CONTEXT_SPL != omap_hw_init_context()) {
		puts("IODELAY recalibration called from invalid context - use only from SPL in SRAM\n");
		return;
	}

	ret = __recalibrate_iodelay_start();
	if (ret)
		goto err;

	/* Configure Mux settings */
	do_set_mux32((*ctrl)->control_padconf_core_base, pad, npads);

	/* Configure Manual IO timing modes */
	ret = do_set_iodelay((*ctrl)->iodelay_config_base, iodelay, niodelays);
	if (ret)
		goto err;

err:
	__recalibrate_iodelay_end(ret);

}

void late_recalibrate_iodelay(struct pad_conf_entry const *pad, int npads,
			      struct iodelay_cfg_entry const *iodelay,
			      int niodelays)
{
	int ret = 0;

	/* unlock IODELAY CONFIG registers */
	writel(CFG_IODELAY_UNLOCK_KEY, (*ctrl)->iodelay_config_base +
	       CFG_REG_8_OFFSET);

	ret = calibrate_iodelay((*ctrl)->iodelay_config_base);
	if (ret)
		goto err;

	ret = update_delay_mechanism((*ctrl)->iodelay_config_base);

	/* Configure Mux settings */
	do_set_mux32((*ctrl)->control_padconf_core_base, pad, npads);

	/* Configure Manual IO timing modes */
	ret = do_set_iodelay((*ctrl)->iodelay_config_base, iodelay, niodelays);
	if (ret)
		goto err;

err:
	/* lock IODELAY CONFIG registers */
	writel(CFG_IODELAY_LOCK_KEY, (*ctrl)->iodelay_config_base +
	       CFG_REG_8_OFFSET);
}
