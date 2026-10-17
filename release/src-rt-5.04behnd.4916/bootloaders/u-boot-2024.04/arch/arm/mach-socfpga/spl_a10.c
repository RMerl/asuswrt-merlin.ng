// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2012-2021 Altera Corporation <www.altera.com>
 */

#include <common.h>
#include <cpu_func.h>
#include <hang.h>
#include <init.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/pl310.h>
#include <asm/u-boot.h>
#include <asm/utils.h>
#include <image.h>
#include <asm/arch/reset_manager.h>
#include <spl.h>
#include <asm/arch/system_manager.h>
#include <asm/arch/freeze_controller.h>
#include <asm/arch/clock_manager.h>
#include <asm/arch/scan_manager.h>
#include <asm/arch/sdram.h>
#include <asm/arch/scu.h>
#include <asm/arch/misc.h>
#include <asm/arch/nic301.h>
#include <asm/sections.h>
#include <fdtdec.h>
#include <watchdog.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/fpga_manager.h>
#include <mmc.h>
#include <memalign.h>
#include <linux/delay.h>

#define FPGA_BUFSIZ	16 * 1024
#define FSBL_IMAGE_IS_VALID	0x49535756

#define FSBL_IMAGE_IS_INVALID	0x0
#define BOOTROM_CONFIGURES_IO_PINMUX	0x3

DECLARE_GLOBAL_DATA_PTR;

#define BOOTROM_SHARED_MEM_SIZE		0x800	/* 2KB */
#define BOOTROM_SHARED_MEM_ADDR		(CFG_SYS_INIT_RAM_ADDR + \
					 SOCFPGA_PHYS_OCRAM_SIZE - \
					 BOOTROM_SHARED_MEM_SIZE)
#define RST_STATUS_SHARED_ADDR		(BOOTROM_SHARED_MEM_ADDR + 0x438)
static u32 rst_mgr_status __section(".data");

/*
 * Bootrom will clear the status register in reset manager and stores the
 * reset status value in shared memory. Bootrom stores shared data at last
 * 2KB of onchip RAM.
 * This function save reset status provided by BootROM to rst_mgr_status.
 * More information about reset status register value can be found in reset
 * manager register description.
 * When running in debugger without Bootrom, r0 to r3 are random values.
 * So, skip save the value when r0 is not BootROM shared data address.
 *
 * r0 - Contains the pointer to the shared memory block. The shared
 *	memory block is located in the top 2 KB of on-chip RAM.
 * r1 - contains the length of the shared memory.
 * r2 - unused and set to 0x0.
 * r3 - points to the version block.
 */
void save_boot_params(unsigned long r0, unsigned long r1, unsigned long r2,
		      unsigned long r3)
{
	if (r0 == BOOTROM_SHARED_MEM_ADDR)
		rst_mgr_status = readl(RST_STATUS_SHARED_ADDR);

	save_boot_params_ret();
}

u32 spl_boot_device(void)
{
	const u32 bsel = readl(socfpga_get_sysmgr_addr() + SYSMGR_A10_BOOTINFO);

	switch (SYSMGR_GET_BOOTINFO_BSEL(bsel)) {
	case 0x1:	/* FPGA (HPS2FPGA Bridge) */
		return BOOT_DEVICE_RAM;
	case 0x2:	/* NAND Flash (1.8V) */
	case 0x3:	/* NAND Flash (3.0V) */
		socfpga_per_reset(SOCFPGA_RESET(NAND), 0);
		return BOOT_DEVICE_NAND;
	case 0x4:	/* SD/MMC External Transceiver (1.8V) */
	case 0x5:	/* SD/MMC Internal Transceiver (3.0V) */
		socfpga_per_reset(SOCFPGA_RESET(SDMMC), 0);
		socfpga_per_reset(SOCFPGA_RESET(DMA), 0);
		return BOOT_DEVICE_MMC1;
	case 0x6:	/* QSPI Flash (1.8V) */
	case 0x7:	/* QSPI Flash (3.0V) */
		socfpga_per_reset(SOCFPGA_RESET(QSPI), 0);
		return BOOT_DEVICE_SPI;
	default:
		printf("Invalid boot device (bsel=%08x)!\n", bsel);
		hang();
	}
}

#ifdef CONFIG_SPL_MMC
u32 spl_mmc_boot_mode(struct mmc *mmc, const u32 boot_device)
{
#if defined(CONFIG_SPL_FS_FAT) || defined(CONFIG_SPL_FS_EXT4)
	return MMCSD_MODE_FS;
#else
	return MMCSD_MODE_RAW;
#endif
}
#endif

void spl_board_init(void)
{
	int ret;

	ALLOC_CACHE_ALIGN_BUFFER(char, buf, FPGA_BUFSIZ);

	/* enable console uart printing */
	preloader_console_init();
	schedule();

	arch_early_init_r();

	/* If the full FPGA is already loaded, ie.from EPCQ, config fpga pins */
	if (is_fpgamgr_user_mode()) {
		ret = config_pins(gd->fdt_blob, "shared");
		if (ret)
			return;

		ret = config_pins(gd->fdt_blob, "fpga");
		if (ret)
			return;
	} else if (!is_fpgamgr_early_user_mode()) {
		/* Program IOSSM(early IO release) or full FPGA */
		fpgamgr_program(buf, FPGA_BUFSIZ, 0);

		/* Skipping double program for combined RBF */
		if (!is_fpgamgr_user_mode()) {
			/*
			 * Expect FPGA entered early user mode, so
			 * the flag is set to re-program IOSSM
			 */
			force_periph_program(true);

			/* Re-program IOSSM to stabilize IO system */
			fpgamgr_program(buf, FPGA_BUFSIZ, 0);

			force_periph_program(false);
		}
	}

	/* If the IOSSM/full FPGA is already loaded, start DDR */
	if (is_fpgamgr_early_user_mode() || is_fpgamgr_user_mode()) {
		if (!is_regular_boot_valid()) {
			/*
			 * Ensure all signals in stable state before triggering
			 * warm reset. This value is recommended from stress
			 * test.
			 */
			mdelay(10);

#if IS_ENABLED(CONFIG_CADENCE_QSPI)
			/*
			 * Trigger software reset to QSPI flash.
			 * On some boards, the QSPI flash reset may not be
			 * connected to the HPS warm reset.
			 */
			qspi_flash_software_reset();
#endif

			ret = readl(socfpga_get_rstmgr_addr() +
				    RSTMGR_A10_SYSWARMMASK);
			/*
			 * Masking s2f & FPGA manager module reset from warm
			 * reset
			 */
			writel(ret & (~(ALT_RSTMGR_SYSWARMMASK_S2F_SET_MSK |
			       ALT_RSTMGR_FPGAMGRWARMMASK_S2F_SET_MSK)),
			       socfpga_get_rstmgr_addr() +
			       RSTMGR_A10_SYSWARMMASK);

			/*
			 * BootROM will configure both IO and pin mux after a
			 * warm reset
			 */
			ret = readl(socfpga_get_sysmgr_addr() +
				    SYSMGR_A10_ROMCODE_CTRL);
			writel(ret | BOOTROM_CONFIGURES_IO_PINMUX,
			       socfpga_get_sysmgr_addr() +
			       SYSMGR_A10_ROMCODE_CTRL);

			/*
			 * Up to here, image is considered valid and should be
			 * set as valid before warm reset is triggered
			 */
			writel(FSBL_IMAGE_IS_VALID, socfpga_get_sysmgr_addr() +
			       SYSMGR_A10_ROMCODE_INITSWSTATE);

			/*
			 * Set this flag to scratch register, so that a proper
			 * boot progress before / after warm reset can be
			 * tracked by FSBL
			 */
			set_regular_boot(true);

			schedule();

			reset_cpu();
		}

		/*
		 * Reset this flag to scratch register, so that a proper
		 * boot progress before / after warm reset can be
		 * tracked by FSBL
		 */
		set_regular_boot(false);

		ret = readl(socfpga_get_rstmgr_addr() +
			    RSTMGR_A10_SYSWARMMASK);

		/*
		 * Unmasking s2f & FPGA manager module reset from warm
		 * reset
		 */
		writel(ret | ALT_RSTMGR_SYSWARMMASK_S2F_SET_MSK |
			ALT_RSTMGR_FPGAMGRWARMMASK_S2F_SET_MSK,
			socfpga_get_rstmgr_addr() + RSTMGR_A10_SYSWARMMASK);

		/*
		 * Up to here, MPFE hang workaround is considered done and
		 * should be reset as invalid until FSBL successfully loading
		 * SSBL, and prepare jumping to SSBL, then only setting as
		 * valid
		 */
		writel(FSBL_IMAGE_IS_INVALID, socfpga_get_sysmgr_addr() +
		       SYSMGR_A10_ROMCODE_INITSWSTATE);

		ddr_calibration_sequence();
	}

	if (!is_fpgamgr_user_mode())
		fpgamgr_program(buf, FPGA_BUFSIZ, 0);
}

void board_init_f(ulong dummy)
{
	if (spl_early_init())
		hang();

	socfpga_get_managers_addr();

	dcache_disable();

	socfpga_init_security_policies();
	socfpga_sdram_remap_zero();
	socfpga_pl310_clear();

	/* Assert reset to all except L4WD0 and L4TIMER0 */
	socfpga_per_reset_all();
	socfpga_watchdog_disable();

	/* Configure the clock based on handoff */
	cm_basic_init(gd->fdt_blob);

#ifdef CONFIG_HW_WATCHDOG
	/* release osc1 watchdog timer 0 from reset */
	socfpga_reset_deassert_osc1wd0();

	/* reconfigure and enable the watchdog */
	hw_watchdog_init();
	schedule();
#endif /* CONFIG_HW_WATCHDOG */

	config_dedicated_pins(gd->fdt_blob);
	schedule();
}

/* board specific function prior loading SSBL / U-Boot proper */
void spl_board_prepare_for_boot(void)
{
	writel(FSBL_IMAGE_IS_VALID, socfpga_get_sysmgr_addr() +
	       SYSMGR_A10_ROMCODE_INITSWSTATE);
}
