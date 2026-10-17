// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2013-2017 Altera Corporation <www.altera.com>
 */

#include <common.h>
#include <asm/arch/clock_manager.h>
#include <asm/arch/system_manager.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <command.h>
#include <init.h>
#include <wait_bit.h>

DECLARE_GLOBAL_DATA_PTR;

void cm_wait_for_lock(u32 mask)
{
	u32 inter_val;
	u32 retry = 0;
	do {
#if defined(CONFIG_TARGET_SOCFPGA_GEN5)
		inter_val = readl(socfpga_get_clkmgr_addr() +
				  CLKMGR_INTER) & mask;
#else
		inter_val = readl(socfpga_get_clkmgr_addr() +
				  CLKMGR_STAT) & mask;
#endif
		/* Wait for stable lock */
		if (inter_val == mask)
			retry++;
		else
			retry = 0;
		if (retry >= 10)
			break;
	} while (1);
}

/* function to poll in the fsm busy bit */
int cm_wait_for_fsm(void)
{
	return wait_for_bit_le32((const void *)(socfpga_get_clkmgr_addr() +
				 CLKMGR_STAT), CLKMGR_STAT_BUSY, false, 20000,
				 false);
}

int set_cpu_clk_info(void)
{
#if defined(CONFIG_TARGET_SOCFPGA_GEN5)
	/* Calculate the clock frequencies required for drivers */
	cm_get_l4_sp_clk_hz();
	cm_get_mmc_controller_clk_hz();
#endif

	gd->bd->bi_arm_freq = cm_get_mpu_clk_hz() / 1000000;
	gd->bd->bi_dsp_freq = 0;

#if defined(CONFIG_TARGET_SOCFPGA_GEN5)
	gd->bd->bi_ddr_freq = cm_get_sdram_clk_hz() / 1000000;
#else
	gd->bd->bi_ddr_freq = 0;
#endif

	return 0;
}

#if IS_ENABLED(CONFIG_TARGET_SOCFPGA_SOC64)
int cm_set_qspi_controller_clk_hz(u32 clk_hz)
{
	u32 reg;
	u32 clk_khz;

	/*
	 * Store QSPI ref clock and set into sysmgr boot register.
	 * Only clock freq in kHz degree is accepted due to limited bits[27:0]
	 * is reserved for storing the QSPI clock freq into boot scratch cold0
	 * register.
	 */
	if (clk_hz < 1000)
		return -EINVAL;

	clk_khz = clk_hz / 1000;
	printf("QSPI: Reference clock at %d kHz\n", clk_khz);

	reg = (readl(socfpga_get_sysmgr_addr() +
		     SYSMGR_SOC64_BOOT_SCRATCH_COLD0)) &
		     ~(SYSMGR_SCRATCH_REG_0_QSPI_REFCLK_MASK);

	writel((clk_khz & SYSMGR_SCRATCH_REG_0_QSPI_REFCLK_MASK) | reg,
	       socfpga_get_sysmgr_addr() + SYSMGR_SOC64_BOOT_SCRATCH_COLD0);

	return 0;
}

unsigned int cm_get_qspi_controller_clk_hz(void)
{
	return (readl(socfpga_get_sysmgr_addr() +
		     SYSMGR_SOC64_BOOT_SCRATCH_COLD0) &
		     SYSMGR_SCRATCH_REG_0_QSPI_REFCLK_MASK) * 1000;
}
#endif

#ifndef CONFIG_SPL_BUILD
static int do_showclocks(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	cm_print_clock_quick_summary();
	return 0;
}

U_BOOT_CMD(
	clocks,	CONFIG_SYS_MAXARGS, 1, do_showclocks,
	"display clocks",
	""
);
#endif
