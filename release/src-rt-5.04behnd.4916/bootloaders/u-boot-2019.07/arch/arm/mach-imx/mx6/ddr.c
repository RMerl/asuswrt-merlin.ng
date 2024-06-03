// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014 Gateworks Corporation
 * Author: Tim Harvey <tharvey@gateworks.com>
 */

#include <common.h>
#include <linux/types.h>
#include <asm/arch/clock.h>
#include <asm/arch/mx6-ddr.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <asm/types.h>
#include <wait_bit.h>

#if defined(CONFIG_MX6_DDRCAL)
static void reset_read_data_fifos(void)
{
	struct mmdc_p_regs *mmdc0 = (struct mmdc_p_regs *)MMDC_P0_BASE_ADDR;

	/* Reset data FIFOs twice. */
	setbits_le32(&mmdc0->mpdgctrl0, 1 << 31);
	wait_for_bit_le32(&mmdc0->mpdgctrl0, 1 << 31, 0, 100, 0);

	setbits_le32(&mmdc0->mpdgctrl0, 1 << 31);
	wait_for_bit_le32(&mmdc0->mpdgctrl0, 1 << 31, 0, 100, 0);
}

static void precharge_all(const bool cs0_enable, const bool cs1_enable)
{
	struct mmdc_p_regs *mmdc0 = (struct mmdc_p_regs *)MMDC_P0_BASE_ADDR;

	/*
	 * Issue the Precharge-All command to the DDR device for both
	 * chip selects. Note, CON_REQ bit should also remain set. If
	 * only using one chip select, then precharge only the desired
	 * chip select.
	 */
	if (cs0_enable) { /* CS0 */
		writel(0x04008050, &mmdc0->mdscr);
		wait_for_bit_le32(&mmdc0->mdscr, 1 << 14, 1, 100, 0);
	}

	if (cs1_enable) { /* CS1 */
		writel(0x04008058, &mmdc0->mdscr);
		wait_for_bit_le32(&mmdc0->mdscr, 1 << 14, 1, 100, 0);
	}
}

static void force_delay_measurement(int bus_size)
{
	struct mmdc_p_regs *mmdc0 = (struct mmdc_p_regs *)MMDC_P0_BASE_ADDR;
	struct mmdc_p_regs *mmdc1 = (struct mmdc_p_regs *)MMDC_P1_BASE_ADDR;

	writel(0x800, &mmdc0->mpmur0);
	if (bus_size == 0x2)
		writel(0x800, &mmdc1->mpmur0);
}

static void modify_dg_result(u32 *reg_st0, u32 *reg_st1, u32 *reg_ctrl)
{
	u32 dg_tmp_val, dg_dl_abs_offset, dg_hc_del, val_ctrl;

	/*
	 * DQS gating absolute offset should be modified from reflecting
	 * (HW_DG_LOWx + HW_DG_UPx)/2 to reflecting (HW_DG_UPx - 0x80)
	 */

	val_ctrl = readl(reg_ctrl);
	val_ctrl &= 0xf0000000;

	dg_tmp_val = ((readl(reg_st0) & 0x07ff0000) >> 16) - 0xc0;
	dg_dl_abs_offset = dg_tmp_val & 0x7f;
	dg_hc_del = (dg_tmp_val & 0x780) << 1;

	val_ctrl |= dg_dl_abs_offset + dg_hc_del;

	dg_tmp_val = ((readl(reg_st1) & 0x07ff0000) >> 16) - 0xc0;
	dg_dl_abs_offset = dg_tmp_val & 0x7f;
	dg_hc_del = (dg_tmp_val & 0x780) << 1;

	val_ctrl |= (dg_dl_abs_offset + dg_hc_del) << 16;

	writel(val_ctrl, reg_ctrl);
}

static void correct_mpwldectr_result(void *reg)
{
	/* Limit is 200/256 of CK, which is WL_HC_DELx | 0x48. */
	const unsigned int limit = 0x148;
	u32 val = readl(reg);
	u32 old = val;

	if ((val & 0x17f) > limit)
		val &= 0xffff << 16;

	if (((val >> 16) & 0x17f) > limit)
		val &= 0xffff;

	if (old != val)
		writel(val, reg);
}

int mmdc_do_write_level_calibration(struct mx6_ddr_sysinfo const *sysinfo)
{
	struct mmdc_p_regs *mmdc0 = (struct mmdc_p_regs *)MMDC_P0_BASE_ADDR;
	struct mmdc_p_regs *mmdc1 = (struct mmdc_p_regs *)MMDC_P1_BASE_ADDR;
	u32 esdmisc_val, zq_val;
	u32 errors = 0;
	u32 ldectrl[4] = {0};
	u32 ddr_mr1 = 0x4;
	u32 rwalat_max;

	/*
	 * Stash old values in case calibration fails,
	 * we need to restore them
	 */
	ldectrl[0] = readl(&mmdc0->mpwldectrl0);
	ldectrl[1] = readl(&mmdc0->mpwldectrl1);
	if (sysinfo->dsize == 2) {
		ldectrl[2] = readl(&mmdc1->mpwldectrl0);
		ldectrl[3] = readl(&mmdc1->mpwldectrl1);
	}

	/* disable DDR logic power down timer */
	clrbits_le32(&mmdc0->mdpdc, 0xff00);

	/* disable Adopt power down timer */
	setbits_le32(&mmdc0->mapsr, 0x1);

	debug("Starting write leveling calibration.\n");

	/*
	 * 2. disable auto refresh and ZQ calibration
	 * before proceeding with Write Leveling calibration
	 */
	esdmisc_val = readl(&mmdc0->mdref);
	writel(0x0000C000, &mmdc0->mdref);
	zq_val = readl(&mmdc0->mpzqhwctrl);
	writel(zq_val & ~0x3, &mmdc0->mpzqhwctrl);

	/* 3. increase walat and ralat to maximum */
	rwalat_max = (1 << 6) | (1 << 7) | (1 << 8) | (1 << 16) | (1 << 17);
	setbits_le32(&mmdc0->mdmisc, rwalat_max);
	if (sysinfo->dsize == 2)
		setbits_le32(&mmdc1->mdmisc, rwalat_max);
	/*
	 * 4 & 5. Configure the external DDR device to enter write-leveling
	 * mode through Load Mode Register command.
	 * Register setting:
	 * Bits[31:16] MR1 value (0x0080 write leveling enable)
	 * Bit[9] set WL_EN to enable MMDC DQS output
	 * Bits[6:4] set CMD bits for Load Mode Register programming
	 * Bits[2:0] set CMD_BA to 0x1 for DDR MR1 programming
	 */
	writel(0x00808231, &mmdc0->mdscr);

	/* 6. Activate automatic calibration by setting MPWLGCR[HW_WL_EN] */
	writel(0x00000001, &mmdc0->mpwlgcr);

	/*
	 * 7. Upon completion of this process the MMDC de-asserts
	 * the MPWLGCR[HW_WL_EN]
	 */
	wait_for_bit_le32(&mmdc0->mpwlgcr, 1 << 0, 0, 100, 0);

	/*
	 * 8. check for any errors: check both PHYs for x64 configuration,
	 * if x32, check only PHY0
	 */
	if (readl(&mmdc0->mpwlgcr) & 0x00000F00)
		errors |= 1;
	if (sysinfo->dsize == 2)
		if (readl(&mmdc1->mpwlgcr) & 0x00000F00)
			errors |= 2;

	debug("Ending write leveling calibration. Error mask: 0x%x\n", errors);

	/* check to see if cal failed */
	if ((readl(&mmdc0->mpwldectrl0) == 0x001F001F) &&
	    (readl(&mmdc0->mpwldectrl1) == 0x001F001F) &&
	    ((sysinfo->dsize < 2) ||
	     ((readl(&mmdc1->mpwldectrl0) == 0x001F001F) &&
	      (readl(&mmdc1->mpwldectrl1) == 0x001F001F)))) {
		debug("Cal seems to have soft-failed due to memory not supporting write leveling on all channels. Restoring original write leveling values.\n");
		writel(ldectrl[0], &mmdc0->mpwldectrl0);
		writel(ldectrl[1], &mmdc0->mpwldectrl1);
		if (sysinfo->dsize == 2) {
			writel(ldectrl[2], &mmdc1->mpwldectrl0);
			writel(ldectrl[3], &mmdc1->mpwldectrl1);
		}
		errors |= 4;
	}

	correct_mpwldectr_result(&mmdc0->mpwldectrl0);
	correct_mpwldectr_result(&mmdc0->mpwldectrl1);
	if (sysinfo->dsize == 2) {
		correct_mpwldectr_result(&mmdc1->mpwldectrl0);
		correct_mpwldectr_result(&mmdc1->mpwldectrl1);
	}

	/*
	 * User should issue MRS command to exit write leveling mode
	 * through Load Mode Register command
	 * Register setting:
	 * Bits[31:16] MR1 value "ddr_mr1" value from initialization
	 * Bit[9] clear WL_EN to disable MMDC DQS output
	 * Bits[6:4] set CMD bits for Load Mode Register programming
	 * Bits[2:0] set CMD_BA to 0x1 for DDR MR1 programming
	 */
	writel((ddr_mr1 << 16) + 0x8031, &mmdc0->mdscr);

	/* re-enable auto refresh and zq cal */
	writel(esdmisc_val, &mmdc0->mdref);
	writel(zq_val, &mmdc0->mpzqhwctrl);

	debug("\tMMDC_MPWLDECTRL0 after write level cal: 0x%08X\n",
	      readl(&mmdc0->mpwldectrl0));
	debug("\tMMDC_MPWLDECTRL1 after write level cal: 0x%08X\n",
	      readl(&mmdc0->mpwldectrl1));
	if (sysinfo->dsize == 2) {
		debug("\tMMDC_MPWLDECTRL0 after write level cal: 0x%08X\n",
		      readl(&mmdc1->mpwldectrl0));
		debug("\tMMDC_MPWLDECTRL1 after write level cal: 0x%08X\n",
		      readl(&mmdc1->mpwldectrl1));
	}

	/* We must force a readback of these values, to get them to stick */
	readl(&mmdc0->mpwldectrl0);
	readl(&mmdc0->mpwldectrl1);
	if (sysinfo->dsize == 2) {
		readl(&mmdc1->mpwldectrl0);
		readl(&mmdc1->mpwldectrl1);
	}

	/* enable DDR logic power down timer: */
	setbits_le32(&mmdc0->mdpdc, 0x00005500);

	/* Enable Adopt power down timer: */
	clrbits_le32(&mmdc0->mapsr, 0x1);

	/* Clear CON_REQ */
	writel(0, &mmdc0->mdscr);

	return errors;
}

int mmdc_do_dqs_calibration(struct mx6_ddr_sysinfo const *sysinfo)
{
	struct mmdc_p_regs *mmdc0 = (struct mmdc_p_regs *)MMDC_P0_BASE_ADDR;
	struct mmdc_p_regs *mmdc1 = (struct mmdc_p_regs *)MMDC_P1_BASE_ADDR;
	struct mx6dq_iomux_ddr_regs *mx6_ddr_iomux =
		(struct mx6dq_iomux_ddr_regs *)MX6DQ_IOM_DDR_BASE;
	bool cs0_enable;
	bool cs1_enable;
	bool cs0_enable_initial;
	bool cs1_enable_initial;
	u32 esdmisc_val;
	u32 temp_ref;
	u32 pddword = 0x00ffff00; /* best so far, place into MPPDCMPR1 */
	u32 errors = 0;
	u32 initdelay = 0x40404040;

	/* check to see which chip selects are enabled */
	cs0_enable_initial = readl(&mmdc0->mdctl) & 0x80000000;
	cs1_enable_initial = readl(&mmdc0->mdctl) & 0x40000000;

	/* disable DDR logic power down timer: */
	clrbits_le32(&mmdc0->mdpdc, 0xff00);

	/* disable Adopt power down timer: */
	setbits_le32(&mmdc0->mapsr, 0x1);

	/* set DQS pull ups */
	setbits_le32(&mx6_ddr_iomux->dram_sdqs0, 0x7000);
	setbits_le32(&mx6_ddr_iomux->dram_sdqs1, 0x7000);
	setbits_le32(&mx6_ddr_iomux->dram_sdqs2, 0x7000);
	setbits_le32(&mx6_ddr_iomux->dram_sdqs3, 0x7000);
	setbits_le32(&mx6_ddr_iomux->dram_sdqs4, 0x7000);
	setbits_le32(&mx6_ddr_iomux->dram_sdqs5, 0x7000);
	setbits_le32(&mx6_ddr_iomux->dram_sdqs6, 0x7000);
	setbits_le32(&mx6_ddr_iomux->dram_sdqs7, 0x7000);

	/* Save old RALAT and WALAT values */
	esdmisc_val = readl(&mmdc0->mdmisc);

	setbits_le32(&mmdc0->mdmisc,
		     (1 << 6) | (1 << 7) | (1 << 8) | (1 << 16) | (1 << 17));

	/* Disable auto refresh before proceeding with calibration */
	temp_ref = readl(&mmdc0->mdref);
	writel(0x0000c000, &mmdc0->mdref);

	/*
	 * Per the ref manual, issue one refresh cycle MDSCR[CMD]= 0x2,
	 * this also sets the CON_REQ bit.
	 */
	if (cs0_enable_initial)
		writel(0x00008020, &mmdc0->mdscr);
	if (cs1_enable_initial)
		writel(0x00008028, &mmdc0->mdscr);

	/* poll to make sure the con_ack bit was asserted */
	wait_for_bit_le32(&mmdc0->mdscr, 1 << 14, 1, 100, 0);

	/*
	 * Check MDMISC register CALIB_PER_CS to see which CS calibration
	 * is targeted to (under normal cases, it should be cleared
	 * as this is the default value, indicating calibration is directed
	 * to CS0).
	 * Disable the other chip select not being target for calibration
	 * to avoid any potential issues.  This will get re-enabled at end
	 * of calibration.
	 */
	if ((readl(&mmdc0->mdmisc) & 0x00100000) == 0)
		clrbits_le32(&mmdc0->mdctl, 1 << 30);	/* clear SDE_1 */
	else
		clrbits_le32(&mmdc0->mdctl, 1 << 31);	/* clear SDE_0 */

	/*
	 * Check to see which chip selects are now enabled for
	 * the remainder of the calibration.
	 */
	cs0_enable = readl(&mmdc0->mdctl) & 0x80000000;
	cs1_enable = readl(&mmdc0->mdctl) & 0x40000000;

	precharge_all(cs0_enable, cs1_enable);

	/* Write the pre-defined value into MPPDCMPR1 */
	writel(pddword, &mmdc0->mppdcmpr1);

	/*
	 * Issue a write access to the external DDR device by setting
	 * the bit SW_DUMMY_WR (bit 0) in the MPSWDAR0 and then poll
	 * this bit until it clears to indicate completion of the write access.
	 */
	setbits_le32(&mmdc0->mpswdar0, 1);
	wait_for_bit_le32(&mmdc0->mpswdar0, 1 << 0, 0, 100, 0);

	/* Set the RD_DL_ABS# bits to their default values
	 * (will be calibrated later in the read delay-line calibration).
	 * Both PHYs for x64 configuration, if x32, do only PHY0.
	 */
	writel(initdelay, &mmdc0->mprddlctl);
	if (sysinfo->dsize == 0x2)
		writel(initdelay, &mmdc1->mprddlctl);

	/* Force a measurment, for previous delay setup to take effect. */
	force_delay_measurement(sysinfo->dsize);

	/*
	 * ***************************
	 * Read DQS Gating calibration
	 * ***************************
	 */
	debug("Starting Read DQS Gating calibration.\n");

	/*
	 * Reset the read data FIFOs (two resets); only need to issue reset
	 * to PHY0 since in x64 mode, the reset will also go to PHY1.
	 */
	reset_read_data_fifos();

	/*
	 * Start the automatic read DQS gating calibration process by
	 * asserting MPDGCTRL0[HW_DG_EN] and MPDGCTRL0[DG_CMP_CYC]
	 * and then poll MPDGCTRL0[HW_DG_EN]] until this bit clears
	 * to indicate completion.
	 * Also, ensure that MPDGCTRL0[HW_DG_ERR] is clear to indicate
	 * no errors were seen during calibration.
	 */

	/*
	 * Set bit 30: chooses option to wait 32 cycles instead of
	 * 16 before comparing read data.
	 */
	setbits_le32(&mmdc0->mpdgctrl0, 1 << 30);
	if (sysinfo->dsize == 2)
		setbits_le32(&mmdc1->mpdgctrl0, 1 << 30);

	/* Set bit 28 to start automatic read DQS gating calibration */
	setbits_le32(&mmdc0->mpdgctrl0, 5 << 28);

	/* Poll for completion.  MPDGCTRL0[HW_DG_EN] should be 0 */
	wait_for_bit_le32(&mmdc0->mpdgctrl0, 1 << 28, 0, 100, 0);

	/*
	 * Check to see if any errors were encountered during calibration
	 * (check MPDGCTRL0[HW_DG_ERR]).
	 * Check both PHYs for x64 configuration, if x32, check only PHY0.
	 */
	if (readl(&mmdc0->mpdgctrl0) & 0x00001000)
		errors |= 1;

	if ((sysinfo->dsize == 0x2) && (readl(&mmdc1->mpdgctrl0) & 0x00001000))
		errors |= 2;

	/* now disable mpdgctrl0[DG_CMP_CYC] */
	clrbits_le32(&mmdc0->mpdgctrl0, 1 << 30);
	if (sysinfo->dsize == 2)
		clrbits_le32(&mmdc1->mpdgctrl0, 1 << 30);

	/*
	 * DQS gating absolute offset should be modified from
	 * reflecting (HW_DG_LOWx + HW_DG_UPx)/2 to
	 * reflecting (HW_DG_UPx - 0x80)
	 */
	modify_dg_result(&mmdc0->mpdghwst0, &mmdc0->mpdghwst1,
			 &mmdc0->mpdgctrl0);
	modify_dg_result(&mmdc0->mpdghwst2, &mmdc0->mpdghwst3,
			 &mmdc0->mpdgctrl1);
	if (sysinfo->dsize == 0x2) {
		modify_dg_result(&mmdc1->mpdghwst0, &mmdc1->mpdghwst1,
				 &mmdc1->mpdgctrl0);
		modify_dg_result(&mmdc1->mpdghwst2, &mmdc1->mpdghwst3,
				 &mmdc1->mpdgctrl1);
	}
	debug("Ending Read DQS Gating calibration. Error mask: 0x%x\n", errors);

	/*
	 * **********************
	 * Read Delay calibration
	 * **********************
	 */
	debug("Starting Read Delay calibration.\n");

	reset_read_data_fifos();

	/*
	 * 4. Issue the Precharge-All command to the DDR device for both
	 * chip selects.  If only using one chip select, then precharge
	 * only the desired chip select.
	 */
	precharge_all(cs0_enable, cs1_enable);

	/*
	 * 9. Read delay-line calibration
	 * Start the automatic read calibration process by asserting
	 * MPRDDLHWCTL[HW_RD_DL_EN].
	 */
	writel(0x00000030, &mmdc0->mprddlhwctl);

	/*
	 * 10. poll for completion
	 * MMDC indicates that the write data calibration had finished by
	 * setting MPRDDLHWCTL[HW_RD_DL_EN] = 0.   Also, ensure that
	 * no error bits were set.
	 */
	wait_for_bit_le32(&mmdc0->mprddlhwctl, 1 << 4, 0, 100, 0);

	/* check both PHYs for x64 configuration, if x32, check only PHY0 */
	if (readl(&mmdc0->mprddlhwctl) & 0x0000000f)
		errors |= 4;

	if ((sysinfo->dsize == 0x2) &&
	    (readl(&mmdc1->mprddlhwctl) & 0x0000000f))
		errors |= 8;

	debug("Ending Read Delay calibration. Error mask: 0x%x\n", errors);

	/*
	 * ***********************
	 * Write Delay Calibration
	 * ***********************
	 */
	debug("Starting Write Delay calibration.\n");

	reset_read_data_fifos();

	/*
	 * 4. Issue the Precharge-All command to the DDR device for both
	 * chip selects. If only using one chip select, then precharge
	 * only the desired chip select.
	 */
	precharge_all(cs0_enable, cs1_enable);

	/*
	 * 8. Set the WR_DL_ABS# bits to their default values.
	 * Both PHYs for x64 configuration, if x32, do only PHY0.
	 */
	writel(initdelay, &mmdc0->mpwrdlctl);
	if (sysinfo->dsize == 0x2)
		writel(initdelay, &mmdc1->mpwrdlctl);

	/*
	 * XXX This isn't in the manual. Force a measurement,
	 * for previous delay setup to effect.
	 */
	force_delay_measurement(sysinfo->dsize);

	/*
	 * 9. 10. Start the automatic write calibration process
	 * by asserting MPWRDLHWCTL0[HW_WR_DL_EN].
	 */
	writel(0x00000030, &mmdc0->mpwrdlhwctl);

	/*
	 * Poll for completion.
	 * MMDC indicates that the write data calibration had finished
	 * by setting MPWRDLHWCTL[HW_WR_DL_EN] = 0.
	 * Also, ensure that no error bits were set.
	 */
	wait_for_bit_le32(&mmdc0->mpwrdlhwctl, 1 << 4, 0, 100, 0);

	/* Check both PHYs for x64 configuration, if x32, check only PHY0 */
	if (readl(&mmdc0->mpwrdlhwctl) & 0x0000000f)
		errors |= 16;

	if ((sysinfo->dsize == 0x2) &&
	    (readl(&mmdc1->mpwrdlhwctl) & 0x0000000f))
		errors |= 32;

	debug("Ending Write Delay calibration. Error mask: 0x%x\n", errors);

	reset_read_data_fifos();

	/* Enable DDR logic power down timer */
	setbits_le32(&mmdc0->mdpdc, 0x00005500);

	/* Enable Adopt power down timer */
	clrbits_le32(&mmdc0->mapsr, 0x1);

	/* Restore MDMISC value (RALAT, WALAT) to MMDCP1 */
	writel(esdmisc_val, &mmdc0->mdmisc);

	/* Clear DQS pull ups */
	clrbits_le32(&mx6_ddr_iomux->dram_sdqs0, 0x7000);
	clrbits_le32(&mx6_ddr_iomux->dram_sdqs1, 0x7000);
	clrbits_le32(&mx6_ddr_iomux->dram_sdqs2, 0x7000);
	clrbits_le32(&mx6_ddr_iomux->dram_sdqs3, 0x7000);
	clrbits_le32(&mx6_ddr_iomux->dram_sdqs4, 0x7000);
	clrbits_le32(&mx6_ddr_iomux->dram_sdqs5, 0x7000);
	clrbits_le32(&mx6_ddr_iomux->dram_sdqs6, 0x7000);
	clrbits_le32(&mx6_ddr_iomux->dram_sdqs7, 0x7000);

	/* Re-enable SDE (chip selects) if they were set initially */
	if (cs1_enable_initial)
		/* Set SDE_1 */
		setbits_le32(&mmdc0->mdctl, 1 << 30);

	if (cs0_enable_initial)
		/* Set SDE_0 */
		setbits_le32(&mmdc0->mdctl, 1 << 31);

	/* Re-enable to auto refresh */
	writel(temp_ref, &mmdc0->mdref);

	/* Clear the MDSCR (including the con_req bit) */
	writel(0x0, &mmdc0->mdscr);	/* CS0 */

	/* Poll to make sure the con_ack bit is clear */
	wait_for_bit_le32(&mmdc0->mdscr, 1 << 14, 0, 100, 0);

	/*
	 * Print out the registers that were updated as a result
	 * of the calibration process.
	 */
	debug("MMDC registers updated from calibration\n");
	debug("Read DQS gating calibration:\n");
	debug("\tMPDGCTRL0 PHY0 = 0x%08X\n", readl(&mmdc0->mpdgctrl0));
	debug("\tMPDGCTRL1 PHY0 = 0x%08X\n", readl(&mmdc0->mpdgctrl1));
	if (sysinfo->dsize == 2) {
		debug("\tMPDGCTRL0 PHY1 = 0x%08X\n", readl(&mmdc1->mpdgctrl0));
		debug("\tMPDGCTRL1 PHY1 = 0x%08X\n", readl(&mmdc1->mpdgctrl1));
	}
	debug("Read calibration:\n");
	debug("\tMPRDDLCTL PHY0 = 0x%08X\n", readl(&mmdc0->mprddlctl));
	if (sysinfo->dsize == 2)
		debug("\tMPRDDLCTL PHY1 = 0x%08X\n", readl(&mmdc1->mprddlctl));
	debug("Write calibration:\n");
	debug("\tMPWRDLCTL PHY0 = 0x%08X\n", readl(&mmdc0->mpwrdlctl));
	if (sysinfo->dsize == 2)
		debug("\tMPWRDLCTL PHY1 = 0x%08X\n", readl(&mmdc1->mpwrdlctl));

	/*
	 * Registers below are for debugging purposes.  These print out
	 * the upper and lower boundaries captured during
	 * read DQS gating calibration.
	 */
	debug("Status registers bounds for read DQS gating:\n");
	debug("\tMPDGHWST0 PHY0 = 0x%08x\n", readl(&mmdc0->mpdghwst0));
	debug("\tMPDGHWST1 PHY0 = 0x%08x\n", readl(&mmdc0->mpdghwst1));
	debug("\tMPDGHWST2 PHY0 = 0x%08x\n", readl(&mmdc0->mpdghwst2));
	debug("\tMPDGHWST3 PHY0 = 0x%08x\n", readl(&mmdc0->mpdghwst3));
	if (sysinfo->dsize == 2) {
		debug("\tMPDGHWST0 PHY1 = 0x%08x\n", readl(&mmdc1->mpdghwst0));
		debug("\tMPDGHWST1 PHY1 = 0x%08x\n", readl(&mmdc1->mpdghwst1));
		debug("\tMPDGHWST2 PHY1 = 0x%08x\n", readl(&mmdc1->mpdghwst2));
		debug("\tMPDGHWST3 PHY1 = 0x%08x\n", readl(&mmdc1->mpdghwst3));
	}

	debug("Final do_dqs_calibration error mask: 0x%x\n", errors);

	return errors;
}
#endif

#if defined(CONFIG_MX6SX)
/* Configure MX6SX mmdc iomux */
void mx6sx_dram_iocfg(unsigned width,
		      const struct mx6sx_iomux_ddr_regs *ddr,
		      const struct mx6sx_iomux_grp_regs *grp)
{
	struct mx6sx_iomux_ddr_regs *mx6_ddr_iomux;
	struct mx6sx_iomux_grp_regs *mx6_grp_iomux;

	mx6_ddr_iomux = (struct mx6sx_iomux_ddr_regs *)MX6SX_IOM_DDR_BASE;
	mx6_grp_iomux = (struct mx6sx_iomux_grp_regs *)MX6SX_IOM_GRP_BASE;

	/* DDR IO TYPE */
	writel(grp->grp_ddr_type, &mx6_grp_iomux->grp_ddr_type);
	writel(grp->grp_ddrpke, &mx6_grp_iomux->grp_ddrpke);

	/* CLOCK */
	writel(ddr->dram_sdclk_0, &mx6_ddr_iomux->dram_sdclk_0);

	/* ADDRESS */
	writel(ddr->dram_cas, &mx6_ddr_iomux->dram_cas);
	writel(ddr->dram_ras, &mx6_ddr_iomux->dram_ras);
	writel(grp->grp_addds, &mx6_grp_iomux->grp_addds);

	/* Control */
	writel(ddr->dram_reset, &mx6_ddr_iomux->dram_reset);
	writel(ddr->dram_sdba2, &mx6_ddr_iomux->dram_sdba2);
	writel(ddr->dram_sdcke0, &mx6_ddr_iomux->dram_sdcke0);
	writel(ddr->dram_sdcke1, &mx6_ddr_iomux->dram_sdcke1);
	writel(ddr->dram_odt0, &mx6_ddr_iomux->dram_odt0);
	writel(ddr->dram_odt1, &mx6_ddr_iomux->dram_odt1);
	writel(grp->grp_ctlds, &mx6_grp_iomux->grp_ctlds);

	/* Data Strobes */
	writel(grp->grp_ddrmode_ctl, &mx6_grp_iomux->grp_ddrmode_ctl);
	writel(ddr->dram_sdqs0, &mx6_ddr_iomux->dram_sdqs0);
	writel(ddr->dram_sdqs1, &mx6_ddr_iomux->dram_sdqs1);
	if (width >= 32) {
		writel(ddr->dram_sdqs2, &mx6_ddr_iomux->dram_sdqs2);
		writel(ddr->dram_sdqs3, &mx6_ddr_iomux->dram_sdqs3);
	}

	/* Data */
	writel(grp->grp_ddrmode, &mx6_grp_iomux->grp_ddrmode);
	writel(grp->grp_b0ds, &mx6_grp_iomux->grp_b0ds);
	writel(grp->grp_b1ds, &mx6_grp_iomux->grp_b1ds);
	if (width >= 32) {
		writel(grp->grp_b2ds, &mx6_grp_iomux->grp_b2ds);
		writel(grp->grp_b3ds, &mx6_grp_iomux->grp_b3ds);
	}
	writel(ddr->dram_dqm0, &mx6_ddr_iomux->dram_dqm0);
	writel(ddr->dram_dqm1, &mx6_ddr_iomux->dram_dqm1);
	if (width >= 32) {
		writel(ddr->dram_dqm2, &mx6_ddr_iomux->dram_dqm2);
		writel(ddr->dram_dqm3, &mx6_ddr_iomux->dram_dqm3);
	}
}
#endif

#if defined(CONFIG_MX6UL) || defined(CONFIG_MX6ULL)
void mx6ul_dram_iocfg(unsigned width,
		      const struct mx6ul_iomux_ddr_regs *ddr,
		      const struct mx6ul_iomux_grp_regs *grp)
{
	struct mx6ul_iomux_ddr_regs *mx6_ddr_iomux;
	struct mx6ul_iomux_grp_regs *mx6_grp_iomux;

	mx6_ddr_iomux = (struct mx6ul_iomux_ddr_regs *)MX6UL_IOM_DDR_BASE;
	mx6_grp_iomux = (struct mx6ul_iomux_grp_regs *)MX6UL_IOM_GRP_BASE;

	/* DDR IO TYPE */
	writel(grp->grp_ddr_type, &mx6_grp_iomux->grp_ddr_type);
	writel(grp->grp_ddrpke, &mx6_grp_iomux->grp_ddrpke);

	/* CLOCK */
	writel(ddr->dram_sdclk_0, &mx6_ddr_iomux->dram_sdclk_0);

	/* ADDRESS */
	writel(ddr->dram_cas, &mx6_ddr_iomux->dram_cas);
	writel(ddr->dram_ras, &mx6_ddr_iomux->dram_ras);
	writel(grp->grp_addds, &mx6_grp_iomux->grp_addds);

	/* Control */
	writel(ddr->dram_reset, &mx6_ddr_iomux->dram_reset);
	writel(ddr->dram_sdba2, &mx6_ddr_iomux->dram_sdba2);
	writel(ddr->dram_odt0, &mx6_ddr_iomux->dram_odt0);
	writel(ddr->dram_odt1, &mx6_ddr_iomux->dram_odt1);
	writel(grp->grp_ctlds, &mx6_grp_iomux->grp_ctlds);

	/* Data Strobes */
	writel(grp->grp_ddrmode_ctl, &mx6_grp_iomux->grp_ddrmode_ctl);
	writel(ddr->dram_sdqs0, &mx6_ddr_iomux->dram_sdqs0);
	writel(ddr->dram_sdqs1, &mx6_ddr_iomux->dram_sdqs1);

	/* Data */
	writel(grp->grp_ddrmode, &mx6_grp_iomux->grp_ddrmode);
	writel(grp->grp_b0ds, &mx6_grp_iomux->grp_b0ds);
	writel(grp->grp_b1ds, &mx6_grp_iomux->grp_b1ds);
	writel(ddr->dram_dqm0, &mx6_ddr_iomux->dram_dqm0);
	writel(ddr->dram_dqm1, &mx6_ddr_iomux->dram_dqm1);
}
#endif

#if defined(CONFIG_MX6SL)
void mx6sl_dram_iocfg(unsigned width,
		      const struct mx6sl_iomux_ddr_regs *ddr,
		      const struct mx6sl_iomux_grp_regs *grp)
{
	struct mx6sl_iomux_ddr_regs *mx6_ddr_iomux;
	struct mx6sl_iomux_grp_regs *mx6_grp_iomux;

	mx6_ddr_iomux = (struct mx6sl_iomux_ddr_regs *)MX6SL_IOM_DDR_BASE;
	mx6_grp_iomux = (struct mx6sl_iomux_grp_regs *)MX6SL_IOM_GRP_BASE;

	/* DDR IO TYPE */
	mx6_grp_iomux->grp_ddr_type = grp->grp_ddr_type;
	mx6_grp_iomux->grp_ddrpke = grp->grp_ddrpke;

	/* CLOCK */
	mx6_ddr_iomux->dram_sdclk_0 = ddr->dram_sdclk_0;

	/* ADDRESS */
	mx6_ddr_iomux->dram_cas = ddr->dram_cas;
	mx6_ddr_iomux->dram_ras = ddr->dram_ras;
	mx6_grp_iomux->grp_addds = grp->grp_addds;

	/* Control */
	mx6_ddr_iomux->dram_reset = ddr->dram_reset;
	mx6_ddr_iomux->dram_sdba2 = ddr->dram_sdba2;
	mx6_grp_iomux->grp_ctlds = grp->grp_ctlds;

	/* Data Strobes */
	mx6_grp_iomux->grp_ddrmode_ctl = grp->grp_ddrmode_ctl;
	mx6_ddr_iomux->dram_sdqs0 = ddr->dram_sdqs0;
	mx6_ddr_iomux->dram_sdqs1 = ddr->dram_sdqs1;
	if (width >= 32) {
		mx6_ddr_iomux->dram_sdqs2 = ddr->dram_sdqs2;
		mx6_ddr_iomux->dram_sdqs3 = ddr->dram_sdqs3;
	}

	/* Data */
	mx6_grp_iomux->grp_ddrmode = grp->grp_ddrmode;
	mx6_grp_iomux->grp_b0ds = grp->grp_b0ds;
	mx6_grp_iomux->grp_b1ds = grp->grp_b1ds;
	if (width >= 32) {
		mx6_grp_iomux->grp_b2ds = grp->grp_b2ds;
		mx6_grp_iomux->grp_b3ds = grp->grp_b3ds;
	}

	mx6_ddr_iomux->dram_dqm0 = ddr->dram_dqm0;
	mx6_ddr_iomux->dram_dqm1 = ddr->dram_dqm1;
	if (width >= 32) {
		mx6_ddr_iomux->dram_dqm2 = ddr->dram_dqm2;
		mx6_ddr_iomux->dram_dqm3 = ddr->dram_dqm3;
	}
}
#endif

#if defined(CONFIG_MX6QDL) || defined(CONFIG_MX6Q) || defined(CONFIG_MX6D)
/* Configure MX6DQ mmdc iomux */
void mx6dq_dram_iocfg(unsigned width,
		      const struct mx6dq_iomux_ddr_regs *ddr,
		      const struct mx6dq_iomux_grp_regs *grp)
{
	volatile struct mx6dq_iomux_ddr_regs *mx6_ddr_iomux;
	volatile struct mx6dq_iomux_grp_regs *mx6_grp_iomux;

	mx6_ddr_iomux = (struct mx6dq_iomux_ddr_regs *)MX6DQ_IOM_DDR_BASE;
	mx6_grp_iomux = (struct mx6dq_iomux_grp_regs *)MX6DQ_IOM_GRP_BASE;

	/* DDR IO Type */
	mx6_grp_iomux->grp_ddr_type = grp->grp_ddr_type;
	mx6_grp_iomux->grp_ddrpke = grp->grp_ddrpke;

	/* Clock */
	mx6_ddr_iomux->dram_sdclk_0 = ddr->dram_sdclk_0;
	mx6_ddr_iomux->dram_sdclk_1 = ddr->dram_sdclk_1;

	/* Address */
	mx6_ddr_iomux->dram_cas = ddr->dram_cas;
	mx6_ddr_iomux->dram_ras = ddr->dram_ras;
	mx6_grp_iomux->grp_addds = grp->grp_addds;

	/* Control */
	mx6_ddr_iomux->dram_reset = ddr->dram_reset;
	mx6_ddr_iomux->dram_sdcke0 = ddr->dram_sdcke0;
	mx6_ddr_iomux->dram_sdcke1 = ddr->dram_sdcke1;
	mx6_ddr_iomux->dram_sdba2 = ddr->dram_sdba2;
	mx6_ddr_iomux->dram_sdodt0 = ddr->dram_sdodt0;
	mx6_ddr_iomux->dram_sdodt1 = ddr->dram_sdodt1;
	mx6_grp_iomux->grp_ctlds = grp->grp_ctlds;

	/* Data Strobes */
	mx6_grp_iomux->grp_ddrmode_ctl = grp->grp_ddrmode_ctl;
	mx6_ddr_iomux->dram_sdqs0 = ddr->dram_sdqs0;
	mx6_ddr_iomux->dram_sdqs1 = ddr->dram_sdqs1;
	if (width >= 32) {
		mx6_ddr_iomux->dram_sdqs2 = ddr->dram_sdqs2;
		mx6_ddr_iomux->dram_sdqs3 = ddr->dram_sdqs3;
	}
	if (width >= 64) {
		mx6_ddr_iomux->dram_sdqs4 = ddr->dram_sdqs4;
		mx6_ddr_iomux->dram_sdqs5 = ddr->dram_sdqs5;
		mx6_ddr_iomux->dram_sdqs6 = ddr->dram_sdqs6;
		mx6_ddr_iomux->dram_sdqs7 = ddr->dram_sdqs7;
	}

	/* Data */
	mx6_grp_iomux->grp_ddrmode = grp->grp_ddrmode;
	mx6_grp_iomux->grp_b0ds = grp->grp_b0ds;
	mx6_grp_iomux->grp_b1ds = grp->grp_b1ds;
	if (width >= 32) {
		mx6_grp_iomux->grp_b2ds = grp->grp_b2ds;
		mx6_grp_iomux->grp_b3ds = grp->grp_b3ds;
	}
	if (width >= 64) {
		mx6_grp_iomux->grp_b4ds = grp->grp_b4ds;
		mx6_grp_iomux->grp_b5ds = grp->grp_b5ds;
		mx6_grp_iomux->grp_b6ds = grp->grp_b6ds;
		mx6_grp_iomux->grp_b7ds = grp->grp_b7ds;
	}
	mx6_ddr_iomux->dram_dqm0 = ddr->dram_dqm0;
	mx6_ddr_iomux->dram_dqm1 = ddr->dram_dqm1;
	if (width >= 32) {
		mx6_ddr_iomux->dram_dqm2 = ddr->dram_dqm2;
		mx6_ddr_iomux->dram_dqm3 = ddr->dram_dqm3;
	}
	if (width >= 64) {
		mx6_ddr_iomux->dram_dqm4 = ddr->dram_dqm4;
		mx6_ddr_iomux->dram_dqm5 = ddr->dram_dqm5;
		mx6_ddr_iomux->dram_dqm6 = ddr->dram_dqm6;
		mx6_ddr_iomux->dram_dqm7 = ddr->dram_dqm7;
	}
}
#endif

#if defined(CONFIG_MX6QDL) || defined(CONFIG_MX6DL) || defined(CONFIG_MX6S)
/* Configure MX6SDL mmdc iomux */
void mx6sdl_dram_iocfg(unsigned width,
		       const struct mx6sdl_iomux_ddr_regs *ddr,
		       const struct mx6sdl_iomux_grp_regs *grp)
{
	volatile struct mx6sdl_iomux_ddr_regs *mx6_ddr_iomux;
	volatile struct mx6sdl_iomux_grp_regs *mx6_grp_iomux;

	mx6_ddr_iomux = (struct mx6sdl_iomux_ddr_regs *)MX6SDL_IOM_DDR_BASE;
	mx6_grp_iomux = (struct mx6sdl_iomux_grp_regs *)MX6SDL_IOM_GRP_BASE;

	/* DDR IO Type */
	mx6_grp_iomux->grp_ddr_type = grp->grp_ddr_type;
	mx6_grp_iomux->grp_ddrpke = grp->grp_ddrpke;

	/* Clock */
	mx6_ddr_iomux->dram_sdclk_0 = ddr->dram_sdclk_0;
	mx6_ddr_iomux->dram_sdclk_1 = ddr->dram_sdclk_1;

	/* Address */
	mx6_ddr_iomux->dram_cas = ddr->dram_cas;
	mx6_ddr_iomux->dram_ras = ddr->dram_ras;
	mx6_grp_iomux->grp_addds = grp->grp_addds;

	/* Control */
	mx6_ddr_iomux->dram_reset = ddr->dram_reset;
	mx6_ddr_iomux->dram_sdcke0 = ddr->dram_sdcke0;
	mx6_ddr_iomux->dram_sdcke1 = ddr->dram_sdcke1;
	mx6_ddr_iomux->dram_sdba2 = ddr->dram_sdba2;
	mx6_ddr_iomux->dram_sdodt0 = ddr->dram_sdodt0;
	mx6_ddr_iomux->dram_sdodt1 = ddr->dram_sdodt1;
	mx6_grp_iomux->grp_ctlds = grp->grp_ctlds;

	/* Data Strobes */
	mx6_grp_iomux->grp_ddrmode_ctl = grp->grp_ddrmode_ctl;
	mx6_ddr_iomux->dram_sdqs0 = ddr->dram_sdqs0;
	mx6_ddr_iomux->dram_sdqs1 = ddr->dram_sdqs1;
	if (width >= 32) {
		mx6_ddr_iomux->dram_sdqs2 = ddr->dram_sdqs2;
		mx6_ddr_iomux->dram_sdqs3 = ddr->dram_sdqs3;
	}
	if (width >= 64) {
		mx6_ddr_iomux->dram_sdqs4 = ddr->dram_sdqs4;
		mx6_ddr_iomux->dram_sdqs5 = ddr->dram_sdqs5;
		mx6_ddr_iomux->dram_sdqs6 = ddr->dram_sdqs6;
		mx6_ddr_iomux->dram_sdqs7 = ddr->dram_sdqs7;
	}

	/* Data */
	mx6_grp_iomux->grp_ddrmode = grp->grp_ddrmode;
	mx6_grp_iomux->grp_b0ds = grp->grp_b0ds;
	mx6_grp_iomux->grp_b1ds = grp->grp_b1ds;
	if (width >= 32) {
		mx6_grp_iomux->grp_b2ds = grp->grp_b2ds;
		mx6_grp_iomux->grp_b3ds = grp->grp_b3ds;
	}
	if (width >= 64) {
		mx6_grp_iomux->grp_b4ds = grp->grp_b4ds;
		mx6_grp_iomux->grp_b5ds = grp->grp_b5ds;
		mx6_grp_iomux->grp_b6ds = grp->grp_b6ds;
		mx6_grp_iomux->grp_b7ds = grp->grp_b7ds;
	}
	mx6_ddr_iomux->dram_dqm0 = ddr->dram_dqm0;
	mx6_ddr_iomux->dram_dqm1 = ddr->dram_dqm1;
	if (width >= 32) {
		mx6_ddr_iomux->dram_dqm2 = ddr->dram_dqm2;
		mx6_ddr_iomux->dram_dqm3 = ddr->dram_dqm3;
	}
	if (width >= 64) {
		mx6_ddr_iomux->dram_dqm4 = ddr->dram_dqm4;
		mx6_ddr_iomux->dram_dqm5 = ddr->dram_dqm5;
		mx6_ddr_iomux->dram_dqm6 = ddr->dram_dqm6;
		mx6_ddr_iomux->dram_dqm7 = ddr->dram_dqm7;
	}
}
#endif

/*
 * Configure mx6 mmdc registers based on:
 *  - board-specific memory configuration
 *  - board-specific calibration data
 *  - ddr3/lpddr2 chip details
 *
 * The various calculations here are derived from the Freescale
 * 1. i.Mx6DQSDL DDR3 Script Aid spreadsheet (DOC-94917) designed to generate
 *    MMDC configuration registers based on memory system and memory chip
 *    parameters.
 *
 * 2. i.Mx6SL LPDDR2 Script Aid spreadsheet V0.04 designed to generate MMDC
 *    configuration registers based on memory system and memory chip
 *    parameters.
 *
 * The defaults here are those which were specified in the spreadsheet.
 * For details on each register, refer to the IMX6DQRM and/or IMX6SDLRM
 * and/or IMX6SLRM section titled MMDC initialization.
 */
#define MR(val, ba, cmd, cs1) \
	((val << 16) | (1 << 15) | (cmd << 4) | (cs1 << 3) | ba)
#define MMDC1(entry, value) do {					  \
	if (!is_mx6sx() && !is_mx6ul() && !is_mx6ull() && !is_mx6sl())	  \
		mmdc1->entry = value;					  \
	} while (0)

/*
 * According JESD209-2B-LPDDR2: Table 103
 * WL: write latency
 */
static int lpddr2_wl(uint32_t mem_speed)
{
	switch (mem_speed) {
	case 1066:
	case 933:
		return 4;
	case 800:
		return 3;
	case 677:
	case 533:
		return 2;
	case 400:
	case 333:
		return 1;
	default:
		puts("invalid memory speed\n");
		hang();
	}

	return 0;
}

/*
 * According JESD209-2B-LPDDR2: Table 103
 * RL: read latency
 */
static int lpddr2_rl(uint32_t mem_speed)
{
	switch (mem_speed) {
	case 1066:
		return 8;
	case 933:
		return 7;
	case 800:
		return 6;
	case 677:
		return 5;
	case 533:
		return 4;
	case 400:
	case 333:
		return 3;
	default:
		puts("invalid memory speed\n");
		hang();
	}

	return 0;
}

void mx6_lpddr2_cfg(const struct mx6_ddr_sysinfo *sysinfo,
		    const struct mx6_mmdc_calibration *calib,
		    const struct mx6_lpddr2_cfg *lpddr2_cfg)
{
	volatile struct mmdc_p_regs *mmdc0;
	u32 val;
	u8 tcke, tcksrx, tcksre, trrd;
	u8 twl, txp, tfaw, tcl;
	u16 tras, twr, tmrd, trtp, twtr, trfc, txsr;
	u16 trcd_lp, trppb_lp, trpab_lp, trc_lp;
	u16 cs0_end;
	u8 coladdr;
	int clkper; /* clock period in picoseconds */
	int clock;  /* clock freq in mHz */
	int cs;

	/* only support 16/32 bits */
	if (sysinfo->dsize > 1)
		hang();

	mmdc0 = (struct mmdc_p_regs *)MMDC_P0_BASE_ADDR;

	clock = mxc_get_clock(MXC_DDR_CLK) / 1000000U;
	clkper = (1000 * 1000) / clock; /* pico seconds */

	twl = lpddr2_wl(lpddr2_cfg->mem_speed) - 1;

	/* LPDDR2-S2 and LPDDR2-S4 have the same tRFC value. */
	switch (lpddr2_cfg->density) {
	case 1:
	case 2:
	case 4:
		trfc = DIV_ROUND_UP(130000, clkper) - 1;
		txsr = DIV_ROUND_UP(140000, clkper) - 1;
		break;
	case 8:
		trfc = DIV_ROUND_UP(210000, clkper) - 1;
		txsr = DIV_ROUND_UP(220000, clkper) - 1;
		break;
	default:
		/*
		 * 64Mb, 128Mb, 256Mb, 512Mb are not supported currently.
		 */
		hang();
		break;
	}
	/*
	 * txpdll, txpr, taonpd and taofpd are not relevant in LPDDR2 mode,
	 * set them to 0. */
	txp = DIV_ROUND_UP(7500, clkper) - 1;
	tcke = 3;
	if (lpddr2_cfg->mem_speed == 333)
		tfaw = DIV_ROUND_UP(60000, clkper) - 1;
	else
		tfaw = DIV_ROUND_UP(50000, clkper) - 1;
	trrd = DIV_ROUND_UP(10000, clkper) - 1;

	/* tckesr for LPDDR2 */
	tcksre = DIV_ROUND_UP(15000, clkper);
	tcksrx = tcksre;
	twr  = DIV_ROUND_UP(15000, clkper) - 1;
	/*
	 * tMRR: 2, tMRW: 5
	 * tMRD should be set to max(tMRR, tMRW)
	 */
	tmrd = 5;
	tras = DIV_ROUND_UP(lpddr2_cfg->trasmin, clkper / 10) - 1;
	/* LPDDR2 mode use tRCD_LP filed in MDCFG3. */
	trcd_lp = DIV_ROUND_UP(lpddr2_cfg->trcd_lp, clkper / 10) - 1;
	trc_lp = DIV_ROUND_UP(lpddr2_cfg->trasmin + lpddr2_cfg->trppb_lp,
			      clkper / 10) - 1;
	trppb_lp = DIV_ROUND_UP(lpddr2_cfg->trppb_lp, clkper / 10) - 1;
	trpab_lp = DIV_ROUND_UP(lpddr2_cfg->trpab_lp, clkper / 10) - 1;
	/* To LPDDR2, CL in MDCFG0 refers to RL */
	tcl = lpddr2_rl(lpddr2_cfg->mem_speed) - 3;
	twtr = DIV_ROUND_UP(7500, clkper) - 1;
	trtp = DIV_ROUND_UP(7500, clkper) - 1;

	cs0_end = 4 * sysinfo->cs_density - 1;

	debug("density:%d Gb (%d Gb per chip)\n",
	      sysinfo->cs_density, lpddr2_cfg->density);
	debug("clock: %dMHz (%d ps)\n", clock, clkper);
	debug("memspd:%d\n", lpddr2_cfg->mem_speed);
	debug("trcd_lp=%d\n", trcd_lp);
	debug("trppb_lp=%d\n", trppb_lp);
	debug("trpab_lp=%d\n", trpab_lp);
	debug("trc_lp=%d\n", trc_lp);
	debug("tcke=%d\n", tcke);
	debug("tcksrx=%d\n", tcksrx);
	debug("tcksre=%d\n", tcksre);
	debug("trfc=%d\n", trfc);
	debug("txsr=%d\n", txsr);
	debug("txp=%d\n", txp);
	debug("tfaw=%d\n", tfaw);
	debug("tcl=%d\n", tcl);
	debug("tras=%d\n", tras);
	debug("twr=%d\n", twr);
	debug("tmrd=%d\n", tmrd);
	debug("twl=%d\n", twl);
	debug("trtp=%d\n", trtp);
	debug("twtr=%d\n", twtr);
	debug("trrd=%d\n", trrd);
	debug("cs0_end=%d\n", cs0_end);
	debug("ncs=%d\n", sysinfo->ncs);

	/*
	 * board-specific configuration:
	 *  These values are determined empirically and vary per board layout
	 */
	mmdc0->mpwldectrl0 = calib->p0_mpwldectrl0;
	mmdc0->mpwldectrl1 = calib->p0_mpwldectrl1;
	mmdc0->mpdgctrl0 = calib->p0_mpdgctrl0;
	mmdc0->mpdgctrl1 = calib->p0_mpdgctrl1;
	mmdc0->mprddlctl = calib->p0_mprddlctl;
	mmdc0->mpwrdlctl = calib->p0_mpwrdlctl;
	mmdc0->mpzqlp2ctl = calib->mpzqlp2ctl;

	/* Read data DQ Byte0-3 delay */
	mmdc0->mprddqby0dl = 0x33333333;
	mmdc0->mprddqby1dl = 0x33333333;
	if (sysinfo->dsize > 0) {
		mmdc0->mprddqby2dl = 0x33333333;
		mmdc0->mprddqby3dl = 0x33333333;
	}

	/* Write data DQ Byte0-3 delay */
	mmdc0->mpwrdqby0dl = 0xf3333333;
	mmdc0->mpwrdqby1dl = 0xf3333333;
	if (sysinfo->dsize > 0) {
		mmdc0->mpwrdqby2dl = 0xf3333333;
		mmdc0->mpwrdqby3dl = 0xf3333333;
	}

	/*
	 * In LPDDR2 mode this register should be cleared,
	 * so no termination will be activated.
	 */
	mmdc0->mpodtctrl = 0;

	/* complete calibration */
	val = (1 << 11); /* Force measurement on delay-lines */
	mmdc0->mpmur0 = val;

	/* Step 1: configuration request */
	mmdc0->mdscr = (u32)(1 << 15); /* config request */

	/* Step 2: Timing configuration */
	mmdc0->mdcfg0 = (trfc << 24) | (txsr << 16) | (txp << 13) |
			(tfaw << 4) | tcl;
	mmdc0->mdcfg1 = (tras << 16) | (twr << 9) | (tmrd << 5) | twl;
	mmdc0->mdcfg2 = (trtp << 6) | (twtr << 3) | trrd;
	mmdc0->mdcfg3lp = (trc_lp << 16) | (trcd_lp << 8) |
			  (trppb_lp << 4) | trpab_lp;
	mmdc0->mdotc = 0;

	mmdc0->mdasp = cs0_end; /* CS addressing */

	/* Step 3: Configure DDR type */
	mmdc0->mdmisc = (sysinfo->cs1_mirror << 19) | (sysinfo->walat << 16) |
			(sysinfo->bi_on << 12) | (sysinfo->mif3_mode << 9) |
			(sysinfo->ralat << 6) | (1 << 3);

	/* Step 4: Configure delay while leaving reset */
	mmdc0->mdor = (sysinfo->sde_to_rst << 8) |
		      (sysinfo->rst_to_cke << 0);

	/* Step 5: Configure DDR physical parameters (density and burst len) */
	coladdr = lpddr2_cfg->coladdr;
	if (lpddr2_cfg->coladdr == 8)		/* 8-bit COL is 0x3 */
		coladdr += 4;
	else if (lpddr2_cfg->coladdr == 12)	/* 12-bit COL is 0x4 */
		coladdr += 1;
	mmdc0->mdctl =  (lpddr2_cfg->rowaddr - 11) << 24 |	/* ROW */
			(coladdr - 9) << 20 |			/* COL */
			(0 << 19) |	/* Burst Length = 4 for LPDDR2 */
			(sysinfo->dsize << 16);	/* DDR data bus size */

	/* Step 6: Perform ZQ calibration */
	val = 0xa1390003; /* one-time HW ZQ calib */
	mmdc0->mpzqhwctrl = val;

	/* Step 7: Enable MMDC with desired chip select */
	mmdc0->mdctl |= (1 << 31) |			     /* SDE_0 for CS0 */
			((sysinfo->ncs == 2) ? 1 : 0) << 30; /* SDE_1 for CS1 */

	/* Step 8: Write Mode Registers to Init LPDDR2 devices */
	for (cs = 0; cs < sysinfo->ncs; cs++) {
		/* MR63: reset */
		mmdc0->mdscr = MR(63, 0, 3, cs);
		/* MR10: calibration,
		 * 0xff is calibration command after intilization.
		 */
		val = 0xA | (0xff << 8);
		mmdc0->mdscr = MR(val, 0, 3, cs);
		/* MR1 */
		val = 0x1 | (0x82 << 8);
		mmdc0->mdscr = MR(val, 0, 3, cs);
		/* MR2 */
		val = 0x2 | (0x04 << 8);
		mmdc0->mdscr = MR(val, 0, 3, cs);
		/* MR3 */
		val = 0x3 | (0x02 << 8);
		mmdc0->mdscr = MR(val, 0, 3, cs);
	}

	/* Step 10: Power down control and self-refresh */
	mmdc0->mdpdc = (tcke & 0x7) << 16 |
			5            << 12 |  /* PWDT_1: 256 cycles */
			5            <<  8 |  /* PWDT_0: 256 cycles */
			1            <<  6 |  /* BOTH_CS_PD */
			(tcksrx & 0x7) << 3 |
			(tcksre & 0x7);
	mmdc0->mapsr = 0x00001006; /* ADOPT power down enabled */

	/* Step 11: Configure ZQ calibration: one-time and periodic 1ms */
	val = 0xa1310003;
	mmdc0->mpzqhwctrl = val;

	/* Step 12: Configure and activate periodic refresh */
	mmdc0->mdref = (sysinfo->refsel << 14) | (sysinfo->refr << 11);

	/* Step 13: Deassert config request - init complete */
	mmdc0->mdscr = 0x00000000;

	/* wait for auto-ZQ calibration to complete */
	mdelay(1);
}

void mx6_ddr3_cfg(const struct mx6_ddr_sysinfo *sysinfo,
		  const struct mx6_mmdc_calibration *calib,
		  const struct mx6_ddr3_cfg *ddr3_cfg)
{
	volatile struct mmdc_p_regs *mmdc0;
	volatile struct mmdc_p_regs *mmdc1;
	u32 val;
	u8 tcke, tcksrx, tcksre, txpdll, taofpd, taonpd, trrd;
	u8 todtlon, taxpd, tanpd, tcwl, txp, tfaw, tcl;
	u8 todt_idle_off = 0x4; /* from DDR3 Script Aid spreadsheet */
	u16 trcd, trc, tras, twr, tmrd, trtp, trp, twtr, trfc, txs, txpr;
	u16 cs0_end;
	u16 tdllk = 0x1ff; /* DLL locking time: 512 cycles (JEDEC DDR3) */
	u8 coladdr;
	int clkper; /* clock period in picoseconds */
	int clock; /* clock freq in MHz */
	int cs;
	u16 mem_speed = ddr3_cfg->mem_speed;

	mmdc0 = (struct mmdc_p_regs *)MMDC_P0_BASE_ADDR;
	if (!is_mx6sx() && !is_mx6ul() && !is_mx6ull() && !is_mx6sl())
		mmdc1 = (struct mmdc_p_regs *)MMDC_P1_BASE_ADDR;

	/* Limit mem_speed for MX6D/MX6Q */
	if (is_mx6dq() || is_mx6dqp()) {
		if (mem_speed > 1066)
			mem_speed = 1066; /* 1066 MT/s */

		tcwl = 4;
	}
	/* Limit mem_speed for MX6S/MX6DL */
	else {
		if (mem_speed > 800)
			mem_speed = 800;  /* 800 MT/s */

		tcwl = 3;
	}

	clock = mem_speed / 2;
	/*
	 * Data rate of 1066 MT/s requires 533 MHz DDR3 clock, but MX6D/Q supports
	 * up to 528 MHz, so reduce the clock to fit chip specs
	 */
	if (is_mx6dq() || is_mx6dqp()) {
		if (clock > 528)
			clock = 528; /* 528 MHz */
	}

	clkper = (1000 * 1000) / clock; /* pico seconds */
	todtlon = tcwl;
	taxpd = tcwl;
	tanpd = tcwl;

	switch (ddr3_cfg->density) {
	case 1: /* 1Gb per chip */
		trfc = DIV_ROUND_UP(110000, clkper) - 1;
		txs = DIV_ROUND_UP(120000, clkper) - 1;
		break;
	case 2: /* 2Gb per chip */
		trfc = DIV_ROUND_UP(160000, clkper) - 1;
		txs = DIV_ROUND_UP(170000, clkper) - 1;
		break;
	case 4: /* 4Gb per chip */
		trfc = DIV_ROUND_UP(260000, clkper) - 1;
		txs = DIV_ROUND_UP(270000, clkper) - 1;
		break;
	case 8: /* 8Gb per chip */
		trfc = DIV_ROUND_UP(350000, clkper) - 1;
		txs = DIV_ROUND_UP(360000, clkper) - 1;
		break;
	default:
		/* invalid density */
		puts("invalid chip density\n");
		hang();
		break;
	}
	txpr = txs;

	switch (mem_speed) {
	case 800:
		txp = DIV_ROUND_UP(max(3 * clkper, 7500), clkper) - 1;
		tcke = DIV_ROUND_UP(max(3 * clkper, 7500), clkper) - 1;
		if (ddr3_cfg->pagesz == 1) {
			tfaw = DIV_ROUND_UP(40000, clkper) - 1;
			trrd = DIV_ROUND_UP(max(4 * clkper, 10000), clkper) - 1;
		} else {
			tfaw = DIV_ROUND_UP(50000, clkper) - 1;
			trrd = DIV_ROUND_UP(max(4 * clkper, 10000), clkper) - 1;
		}
		break;
	case 1066:
		txp = DIV_ROUND_UP(max(3 * clkper, 7500), clkper) - 1;
		tcke = DIV_ROUND_UP(max(3 * clkper, 5625), clkper) - 1;
		if (ddr3_cfg->pagesz == 1) {
			tfaw = DIV_ROUND_UP(37500, clkper) - 1;
			trrd = DIV_ROUND_UP(max(4 * clkper, 7500), clkper) - 1;
		} else {
			tfaw = DIV_ROUND_UP(50000, clkper) - 1;
			trrd = DIV_ROUND_UP(max(4 * clkper, 10000), clkper) - 1;
		}
		break;
	default:
		puts("invalid memory speed\n");
		hang();
		break;
	}
	txpdll = DIV_ROUND_UP(max(10 * clkper, 24000), clkper) - 1;
	tcksre = DIV_ROUND_UP(max(5 * clkper, 10000), clkper);
	taonpd = DIV_ROUND_UP(2000, clkper) - 1;
	tcksrx = tcksre;
	taofpd = taonpd;
	twr  = DIV_ROUND_UP(15000, clkper) - 1;
	tmrd = DIV_ROUND_UP(max(12 * clkper, 15000), clkper) - 1;
	trc  = DIV_ROUND_UP(ddr3_cfg->trcmin, clkper / 10) - 1;
	tras = DIV_ROUND_UP(ddr3_cfg->trasmin, clkper / 10) - 1;
	tcl  = DIV_ROUND_UP(ddr3_cfg->trcd, clkper / 10) - 3;
	trp  = DIV_ROUND_UP(ddr3_cfg->trcd, clkper / 10) - 1;
	twtr = ROUND(max(4 * clkper, 7500) / clkper, 1) - 1;
	trcd = trp;
	trtp = twtr;
	cs0_end = 4 * sysinfo->cs_density - 1;

	debug("density:%d Gb (%d Gb per chip)\n",
	      sysinfo->cs_density, ddr3_cfg->density);
	debug("clock: %dMHz (%d ps)\n", clock, clkper);
	debug("memspd:%d\n", mem_speed);
	debug("tcke=%d\n", tcke);
	debug("tcksrx=%d\n", tcksrx);
	debug("tcksre=%d\n", tcksre);
	debug("taofpd=%d\n", taofpd);
	debug("taonpd=%d\n", taonpd);
	debug("todtlon=%d\n", todtlon);
	debug("tanpd=%d\n", tanpd);
	debug("taxpd=%d\n", taxpd);
	debug("trfc=%d\n", trfc);
	debug("txs=%d\n", txs);
	debug("txp=%d\n", txp);
	debug("txpdll=%d\n", txpdll);
	debug("tfaw=%d\n", tfaw);
	debug("tcl=%d\n", tcl);
	debug("trcd=%d\n", trcd);
	debug("trp=%d\n", trp);
	debug("trc=%d\n", trc);
	debug("tras=%d\n", tras);
	debug("twr=%d\n", twr);
	debug("tmrd=%d\n", tmrd);
	debug("tcwl=%d\n", tcwl);
	debug("tdllk=%d\n", tdllk);
	debug("trtp=%d\n", trtp);
	debug("twtr=%d\n", twtr);
	debug("trrd=%d\n", trrd);
	debug("txpr=%d\n", txpr);
	debug("cs0_end=%d\n", cs0_end);
	debug("ncs=%d\n", sysinfo->ncs);
	debug("Rtt_wr=%d\n", sysinfo->rtt_wr);
	debug("Rtt_nom=%d\n", sysinfo->rtt_nom);
	debug("SRT=%d\n", ddr3_cfg->SRT);
	debug("twr=%d\n", twr);

	/*
	 * board-specific configuration:
	 *  These values are determined empirically and vary per board layout
	 *  see:
	 *   appnote, ddr3 spreadsheet
	 */
	mmdc0->mpwldectrl0 = calib->p0_mpwldectrl0;
	mmdc0->mpwldectrl1 = calib->p0_mpwldectrl1;
	mmdc0->mpdgctrl0 = calib->p0_mpdgctrl0;
	mmdc0->mpdgctrl1 = calib->p0_mpdgctrl1;
	mmdc0->mprddlctl = calib->p0_mprddlctl;
	mmdc0->mpwrdlctl = calib->p0_mpwrdlctl;
	if (sysinfo->dsize > 1) {
		MMDC1(mpwldectrl0, calib->p1_mpwldectrl0);
		MMDC1(mpwldectrl1, calib->p1_mpwldectrl1);
		MMDC1(mpdgctrl0, calib->p1_mpdgctrl0);
		MMDC1(mpdgctrl1, calib->p1_mpdgctrl1);
		MMDC1(mprddlctl, calib->p1_mprddlctl);
		MMDC1(mpwrdlctl, calib->p1_mpwrdlctl);
	}

	/* Read data DQ Byte0-3 delay */
	mmdc0->mprddqby0dl = 0x33333333;
	mmdc0->mprddqby1dl = 0x33333333;
	if (sysinfo->dsize > 0) {
		mmdc0->mprddqby2dl = 0x33333333;
		mmdc0->mprddqby3dl = 0x33333333;
	}

	if (sysinfo->dsize > 1) {
		MMDC1(mprddqby0dl, 0x33333333);
		MMDC1(mprddqby1dl, 0x33333333);
		MMDC1(mprddqby2dl, 0x33333333);
		MMDC1(mprddqby3dl, 0x33333333);
	}

	/* MMDC Termination: rtt_nom:2 RZQ/2(120ohm), rtt_nom:1 RZQ/4(60ohm) */
	val = (sysinfo->rtt_nom == 2) ? 0x00011117 : 0x00022227;
	mmdc0->mpodtctrl = val;
	if (sysinfo->dsize > 1)
		MMDC1(mpodtctrl, val);

	/* complete calibration */
	val = (1 << 11); /* Force measurement on delay-lines */
	mmdc0->mpmur0 = val;
	if (sysinfo->dsize > 1)
		MMDC1(mpmur0, val);

	/* Step 1: configuration request */
	mmdc0->mdscr = (u32)(1 << 15); /* config request */

	/* Step 2: Timing configuration */
	mmdc0->mdcfg0 = (trfc << 24) | (txs << 16) | (txp << 13) |
			(txpdll << 9) | (tfaw << 4) | tcl;
	mmdc0->mdcfg1 = (trcd << 29) | (trp << 26) | (trc << 21) |
			(tras << 16) | (1 << 15) /* trpa */ |
			(twr << 9) | (tmrd << 5) | tcwl;
	mmdc0->mdcfg2 = (tdllk << 16) | (trtp << 6) | (twtr << 3) | trrd;
	mmdc0->mdotc = (taofpd << 27) | (taonpd << 24) | (tanpd << 20) |
		       (taxpd << 16) | (todtlon << 12) | (todt_idle_off << 4);
	mmdc0->mdasp = cs0_end; /* CS addressing */

	/* Step 3: Configure DDR type */
	mmdc0->mdmisc = (sysinfo->cs1_mirror << 19) | (sysinfo->walat << 16) |
			(sysinfo->bi_on << 12) | (sysinfo->mif3_mode << 9) |
			(sysinfo->ralat << 6);

	/* Step 4: Configure delay while leaving reset */
	mmdc0->mdor = (txpr << 16) | (sysinfo->sde_to_rst << 8) |
		      (sysinfo->rst_to_cke << 0);

	/* Step 5: Configure DDR physical parameters (density and burst len) */
	coladdr = ddr3_cfg->coladdr;
	if (ddr3_cfg->coladdr == 8)		/* 8-bit COL is 0x3 */
		coladdr += 4;
	else if (ddr3_cfg->coladdr == 12)	/* 12-bit COL is 0x4 */
		coladdr += 1;
	mmdc0->mdctl =  (ddr3_cfg->rowaddr - 11) << 24 |	/* ROW */
			(coladdr - 9) << 20 |			/* COL */
			(1 << 19) |		/* Burst Length = 8 for DDR3 */
			(sysinfo->dsize << 16);		/* DDR data bus size */

	/* Step 6: Perform ZQ calibration */
	val = 0xa1390001; /* one-time HW ZQ calib */
	mmdc0->mpzqhwctrl = val;
	if (sysinfo->dsize > 1)
		MMDC1(mpzqhwctrl, val);

	/* Step 7: Enable MMDC with desired chip select */
	mmdc0->mdctl |= (1 << 31) |			     /* SDE_0 for CS0 */
			((sysinfo->ncs == 2) ? 1 : 0) << 30; /* SDE_1 for CS1 */

	/* Step 8: Write Mode Registers to Init DDR3 devices */
	for (cs = 0; cs < sysinfo->ncs; cs++) {
		/* MR2 */
		val = (sysinfo->rtt_wr & 3) << 9 | (ddr3_cfg->SRT & 1) << 7 |
		      ((tcwl - 3) & 3) << 3;
		debug("MR2 CS%d: 0x%08x\n", cs, (u32)MR(val, 2, 3, cs));
		mmdc0->mdscr = MR(val, 2, 3, cs);
		/* MR3 */
		debug("MR3 CS%d: 0x%08x\n", cs, (u32)MR(0, 3, 3, cs));
		mmdc0->mdscr = MR(0, 3, 3, cs);
		/* MR1 */
		val = ((sysinfo->rtt_nom & 1) ? 1 : 0) << 2 |
		      ((sysinfo->rtt_nom & 2) ? 1 : 0) << 6;
		debug("MR1 CS%d: 0x%08x\n", cs, (u32)MR(val, 1, 3, cs));
		mmdc0->mdscr = MR(val, 1, 3, cs);
		/* MR0 */
		val = ((tcl - 1) << 4) |	/* CAS */
		      (1 << 8)   |		/* DLL Reset */
		      ((twr - 3) << 9) |	/* Write Recovery */
		      (sysinfo->pd_fast_exit << 12); /* Precharge PD PLL on */
		debug("MR0 CS%d: 0x%08x\n", cs, (u32)MR(val, 0, 3, cs));
		mmdc0->mdscr = MR(val, 0, 3, cs);
		/* ZQ calibration */
		val = (1 << 10);
		mmdc0->mdscr = MR(val, 0, 4, cs);
	}

	/* Step 10: Power down control and self-refresh */
	mmdc0->mdpdc = (tcke & 0x7) << 16 |
			5            << 12 |  /* PWDT_1: 256 cycles */
			5            <<  8 |  /* PWDT_0: 256 cycles */
			1            <<  6 |  /* BOTH_CS_PD */
			(tcksrx & 0x7) << 3 |
			(tcksre & 0x7);
	if (!sysinfo->pd_fast_exit)
		mmdc0->mdpdc |= (1 << 7); /* SLOW_PD */
	mmdc0->mapsr = 0x00001006; /* ADOPT power down enabled */

	/* Step 11: Configure ZQ calibration: one-time and periodic 1ms */
	val = 0xa1390003;
	mmdc0->mpzqhwctrl = val;
	if (sysinfo->dsize > 1)
		MMDC1(mpzqhwctrl, val);

	/* Step 12: Configure and activate periodic refresh */
	mmdc0->mdref = (sysinfo->refsel << 14) | (sysinfo->refr << 11);

	/* Step 13: Deassert config request - init complete */
	mmdc0->mdscr = 0x00000000;

	/* wait for auto-ZQ calibration to complete */
	mdelay(1);
}

void mmdc_read_calibration(struct mx6_ddr_sysinfo const *sysinfo,
                           struct mx6_mmdc_calibration *calib)
{
	struct mmdc_p_regs *mmdc0 = (struct mmdc_p_regs *)MMDC_P0_BASE_ADDR;
	struct mmdc_p_regs *mmdc1 = (struct mmdc_p_regs *)MMDC_P1_BASE_ADDR;

	calib->p0_mpwldectrl0 = readl(&mmdc0->mpwldectrl0);
	calib->p0_mpwldectrl1 = readl(&mmdc0->mpwldectrl1);
	calib->p0_mpdgctrl0 = readl(&mmdc0->mpdgctrl0);
	calib->p0_mpdgctrl1 = readl(&mmdc0->mpdgctrl1);
	calib->p0_mprddlctl = readl(&mmdc0->mprddlctl);
	calib->p0_mpwrdlctl = readl(&mmdc0->mpwrdlctl);

	if (sysinfo->dsize == 2) {
		calib->p1_mpwldectrl0 = readl(&mmdc1->mpwldectrl0);
		calib->p1_mpwldectrl1 = readl(&mmdc1->mpwldectrl1);
		calib->p1_mpdgctrl0 = readl(&mmdc1->mpdgctrl0);
		calib->p1_mpdgctrl1 = readl(&mmdc1->mpdgctrl1);
		calib->p1_mprddlctl = readl(&mmdc1->mprddlctl);
		calib->p1_mpwrdlctl = readl(&mmdc1->mpwrdlctl);
	}
}

void mx6_dram_cfg(const struct mx6_ddr_sysinfo *sysinfo,
		  const struct mx6_mmdc_calibration *calib,
		  const void *ddr_cfg)
{
	if (sysinfo->ddr_type == DDR_TYPE_DDR3) {
		mx6_ddr3_cfg(sysinfo, calib, ddr_cfg);
	} else if (sysinfo->ddr_type == DDR_TYPE_LPDDR2) {
		mx6_lpddr2_cfg(sysinfo, calib, ddr_cfg);
	} else {
		puts("Unsupported ddr type\n");
		hang();
	}
}
