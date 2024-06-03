// SPDX-License-Identifier: GPL-2.0+
/*
 * SPL specific code for Compulab CM-FX6 board
 *
 * Copyright (C) 2014, Compulab Ltd - http://compulab.co.il/
 *
 * Author: Nikita Kiryanov <nikita@compulab.co.il>
 */

#include <common.h>
#include <spl.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/mx6-ddr.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/crm_regs.h>
#include <asm/mach-imx/iomux-v3.h>
#include <fsl_esdhc.h>
#include "common.h"

enum ddr_config {
	DDR_16BIT_256MB,
	DDR_32BIT_512MB,
	DDR_32BIT_1GB,
	DDR_64BIT_1GB,
	DDR_64BIT_2GB,
	DDR_64BIT_4GB,
	DDR_UNKNOWN,
};

/*
 * Below DRAM_RESET[DDR_SEL] = 0 which is incorrect according to
 * Freescale QRM, but this is exactly the value used by the automatic
 * calibration script and it works also in all our tests, so we leave
 * it as is at this point.
 */
#define CM_FX6_DDR_IOMUX_CFG \
	.dram_sdqs0	= 0x00000038, \
	.dram_sdqs1	= 0x00000038, \
	.dram_sdqs2	= 0x00000038, \
	.dram_sdqs3	= 0x00000038, \
	.dram_sdqs4	= 0x00000038, \
	.dram_sdqs5	= 0x00000038, \
	.dram_sdqs6	= 0x00000038, \
	.dram_sdqs7	= 0x00000038, \
	.dram_dqm0	= 0x00000038, \
	.dram_dqm1	= 0x00000038, \
	.dram_dqm2	= 0x00000038, \
	.dram_dqm3	= 0x00000038, \
	.dram_dqm4	= 0x00000038, \
	.dram_dqm5	= 0x00000038, \
	.dram_dqm6	= 0x00000038, \
	.dram_dqm7	= 0x00000038, \
	.dram_cas	= 0x00000038, \
	.dram_ras	= 0x00000038, \
	.dram_sdclk_0	= 0x00000038, \
	.dram_sdclk_1	= 0x00000038, \
	.dram_sdcke0	= 0x00003000, \
	.dram_sdcke1	= 0x00003000, \
	.dram_reset	= 0x00000038, \
	.dram_sdba2	= 0x00000000, \
	.dram_sdodt0	= 0x00000038, \
	.dram_sdodt1	= 0x00000038,

#define CM_FX6_GPR_IOMUX_CFG \
	.grp_b0ds	= 0x00000038, \
	.grp_b1ds	= 0x00000038, \
	.grp_b2ds	= 0x00000038, \
	.grp_b3ds	= 0x00000038, \
	.grp_b4ds	= 0x00000038, \
	.grp_b5ds	= 0x00000038, \
	.grp_b6ds	= 0x00000038, \
	.grp_b7ds	= 0x00000038, \
	.grp_addds	= 0x00000038, \
	.grp_ddrmode_ctl = 0x00020000, \
	.grp_ddrpke	= 0x00000000, \
	.grp_ddrmode	= 0x00020000, \
	.grp_ctlds	= 0x00000038, \
	.grp_ddr_type	= 0x000C0000,

static struct mx6sdl_iomux_ddr_regs ddr_iomux_s = { CM_FX6_DDR_IOMUX_CFG };
static struct mx6sdl_iomux_grp_regs grp_iomux_s = { CM_FX6_GPR_IOMUX_CFG };
static struct mx6dq_iomux_ddr_regs ddr_iomux_q = { CM_FX6_DDR_IOMUX_CFG };
static struct mx6dq_iomux_grp_regs grp_iomux_q = { CM_FX6_GPR_IOMUX_CFG };

static struct mx6_mmdc_calibration cm_fx6_calib_s = {
	.p0_mpwldectrl0	= 0x005B0061,
	.p0_mpwldectrl1	= 0x004F0055,
	.p0_mpdgctrl0	= 0x0314030C,
	.p0_mpdgctrl1	= 0x025C0268,
	.p0_mprddlctl	= 0x42464646,
	.p0_mpwrdlctl	= 0x36322C34,
};

static struct mx6_ddr_sysinfo cm_fx6_sysinfo_s = {
	.cs1_mirror	= 1,
	.cs_density	= 16,
	.bi_on		= 1,
	.rtt_nom	= 1,
	.rtt_wr		= 0,
	.ralat		= 5,
	.walat		= 1,
	.mif3_mode	= 3,
	.rst_to_cke	= 0x23,
	.sde_to_rst	= 0x10,
};

static struct mx6_ddr3_cfg cm_fx6_ddr3_cfg_s = {
	.mem_speed	= 800,
	.density	= 4,
	.rowaddr	= 14,
	.coladdr	= 10,
	.pagesz		= 2,
	.trcd		= 1800,
	.trcmin		= 5200,
	.trasmin	= 3600,
	.SRT		= 0,
};

static void spl_mx6s_dram_init(enum ddr_config dram_config, bool reset)
{
	if (reset)
		((struct mmdc_p_regs *)MX6_MMDC_P0_MDCTL)->mdmisc = 2;

	switch (dram_config) {
	case DDR_16BIT_256MB:
		cm_fx6_sysinfo_s.dsize = 0;
		cm_fx6_sysinfo_s.ncs = 1;
		break;
	case DDR_32BIT_512MB:
		cm_fx6_sysinfo_s.dsize = 1;
		cm_fx6_sysinfo_s.ncs = 1;
		break;
	case DDR_32BIT_1GB:
		cm_fx6_sysinfo_s.dsize = 1;
		cm_fx6_sysinfo_s.ncs = 2;
		break;
	default:
		puts("Tried to setup invalid DDR configuration\n");
		hang();
	}

	mx6_dram_cfg(&cm_fx6_sysinfo_s, &cm_fx6_calib_s, &cm_fx6_ddr3_cfg_s);
	udelay(100);
}

static struct mx6_mmdc_calibration cm_fx6_calib_q = {
	.p0_mpwldectrl0	= 0x00630068,
	.p0_mpwldectrl1	= 0x0068005D,
	.p0_mpdgctrl0	= 0x04140428,
	.p0_mpdgctrl1	= 0x037C037C,
	.p0_mprddlctl	= 0x3C30303A,
	.p0_mpwrdlctl	= 0x3A344038,
	.p1_mpwldectrl0	= 0x0035004C,
	.p1_mpwldectrl1	= 0x00170026,
	.p1_mpdgctrl0	= 0x0374037C,
	.p1_mpdgctrl1	= 0x0350032C,
	.p1_mprddlctl	= 0x30322A3C,
	.p1_mpwrdlctl	= 0x48304A3E,
};

static struct mx6_ddr_sysinfo cm_fx6_sysinfo_q = {
	.cs_density	= 16,
	.cs1_mirror	= 1,
	.bi_on		= 1,
	.rtt_nom	= 1,
	.rtt_wr		= 0,
	.ralat		= 5,
	.walat		= 1,
	.mif3_mode	= 3,
	.rst_to_cke	= 0x23,
	.sde_to_rst	= 0x10,
	.refsel = 1,		/* Refresh cycles at 32KHz */
	.refr = 7,		/* 8 refresh commands per refresh cycle */
};

static struct mx6_ddr3_cfg cm_fx6_ddr3_cfg_q = {
	.mem_speed	= 1066,
	.density	= 4,
	.rowaddr	= 14,
	.coladdr	= 10,
	.pagesz		= 2,
	.trcd		= 1324,
	.trcmin		= 59500,
	.trasmin	= 9750,
	.SRT		= 0,
};

static void spl_mx6q_dram_init(enum ddr_config dram_config, bool reset)
{
	if (reset)
		((struct mmdc_p_regs *)MX6_MMDC_P0_MDCTL)->mdmisc = 2;

	cm_fx6_ddr3_cfg_q.rowaddr = 14;
	switch (dram_config) {
	case DDR_16BIT_256MB:
		cm_fx6_sysinfo_q.dsize = 0;
		cm_fx6_sysinfo_q.ncs = 1;
		break;
	case DDR_32BIT_512MB:
		cm_fx6_sysinfo_q.dsize = 1;
		cm_fx6_sysinfo_q.ncs = 1;
		break;
	case DDR_64BIT_1GB:
		cm_fx6_sysinfo_q.dsize = 2;
		cm_fx6_sysinfo_q.ncs = 1;
		break;
	case DDR_64BIT_2GB:
		cm_fx6_sysinfo_q.dsize = 2;
		cm_fx6_sysinfo_q.ncs = 2;
		break;
	case DDR_64BIT_4GB:
		cm_fx6_sysinfo_q.dsize = 2;
		cm_fx6_sysinfo_q.ncs = 2;
		cm_fx6_ddr3_cfg_q.rowaddr = 15;
		break;
	default:
		puts("Tried to setup invalid DDR configuration\n");
		hang();
	}

	mx6_dram_cfg(&cm_fx6_sysinfo_q, &cm_fx6_calib_q, &cm_fx6_ddr3_cfg_q);
	udelay(100);
}

static int cm_fx6_spl_dram_init(void)
{
	unsigned long bank1_size, bank2_size;

	switch (get_cpu_type()) {
	case MXC_CPU_MX6SOLO:
		mx6sdl_dram_iocfg(64, &ddr_iomux_s, &grp_iomux_s);

		spl_mx6s_dram_init(DDR_32BIT_1GB, false);
		bank1_size = get_ram_size((long int *)PHYS_SDRAM_1, 0x80000000);
		bank2_size = get_ram_size((long int *)PHYS_SDRAM_2, 0x80000000);
		if (bank1_size == 0x20000000) {
			if (bank2_size == 0x20000000)
				return 0;

			spl_mx6s_dram_init(DDR_32BIT_512MB, true);
			return 0;
		}

		spl_mx6s_dram_init(DDR_16BIT_256MB, true);
		bank1_size = get_ram_size((long int *)PHYS_SDRAM_1, 0x80000000);
		if (bank1_size == 0x10000000)
			return 0;

		break;
	case MXC_CPU_MX6D:
	case MXC_CPU_MX6Q:
		mx6dq_dram_iocfg(64, &ddr_iomux_q, &grp_iomux_q);

		spl_mx6q_dram_init(DDR_64BIT_4GB, false);
		bank1_size = get_ram_size((long int *)PHYS_SDRAM_1, 0x80000000);
		if (bank1_size == 0x80000000)
			return 0;

		if (bank1_size == 0x40000000) {
			bank2_size = get_ram_size((long int *)PHYS_SDRAM_2,
								0x80000000);
			if (bank2_size == 0x40000000) {
				/* Don't do a full reset here */
				spl_mx6q_dram_init(DDR_64BIT_2GB, false);
			} else {
				spl_mx6q_dram_init(DDR_64BIT_1GB, true);
			}

			return 0;
		}

		spl_mx6q_dram_init(DDR_32BIT_512MB, true);
		bank1_size = get_ram_size((long int *)PHYS_SDRAM_1, 0x80000000);
		if (bank1_size == 0x20000000)
			return 0;

		spl_mx6q_dram_init(DDR_16BIT_256MB, true);
		bank1_size = get_ram_size((long int *)PHYS_SDRAM_1, 0x80000000);
		if (bank1_size == 0x10000000)
			return 0;

		break;
	}

	return -1;
}

static iomux_v3_cfg_t const uart4_pads[] = {
	IOMUX_PADS(PAD_KEY_COL0__UART4_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_KEY_ROW0__UART4_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
};

static void cm_fx6_setup_uart(void)
{
	SETUP_IOMUX_PADS(uart4_pads);
	enable_uart_clk(1);
}

#ifdef CONFIG_SPL_SPI_SUPPORT
static void cm_fx6_setup_ecspi(void)
{
	cm_fx6_set_ecspi_iomux();
	enable_spi_clk(1, 0);
}
#else
static void cm_fx6_setup_ecspi(void) { }
#endif

void board_init_f(ulong dummy)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	/*
	 * We don't use DMA in SPL, but we do need it in U-Boot. U-Boot
	 * initializes DMA very early (before all board code), so the only
	 * opportunity we have to initialize APBHDMA clocks is in SPL.
	 */
	setbits_le32(&mxc_ccm->CCGR0, MXC_CCM_CCGR0_APBHDMA_MASK);
	enable_usdhc_clk(1, 2);

	arch_cpu_init();
	timer_init();
	cm_fx6_setup_ecspi();
	cm_fx6_setup_uart();
	get_clocks();
	preloader_console_init();
	gpio_direction_output(CM_FX6_GREEN_LED, 1);
	if (cm_fx6_spl_dram_init()) {
		puts("!!!ERROR!!! DRAM detection failed!!!\n");
		hang();
	}
}

void board_boot_order(u32 *spl_boot_list)
{
	spl_boot_list[0] = spl_boot_device();
	switch (spl_boot_list[0]) {
	case BOOT_DEVICE_SPI:
		spl_boot_list[1] = BOOT_DEVICE_MMC1;
		break;
	case BOOT_DEVICE_MMC1:
		spl_boot_list[1] = BOOT_DEVICE_SPI;
		break;
	}
}

#ifdef CONFIG_SPL_MMC_SUPPORT
static struct fsl_esdhc_cfg usdhc_cfg = {
	.esdhc_base = USDHC3_BASE_ADDR,
	.max_bus_width = 4,
};

int board_mmc_init(bd_t *bis)
{
	cm_fx6_set_usdhc_iomux();

	usdhc_cfg.sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);

	return fsl_esdhc_initialize(bis, &usdhc_cfg);
}
#endif
