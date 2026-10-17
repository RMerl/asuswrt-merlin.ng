// SPDX-License-Identifier: GPL-2.0
/*
 * AM642: SoC specific initialization
 *
 * Copyright (C) 2020-2021 Texas Instruments Incorporated - https://www.ti.com/
 *	Keerthy <j-keerthy@ti.com>
 *	Dave Gerlach <d-gerlach@ti.com>
 */

#include <fdt_support.h>
#include <spl.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include "sysfw-loader.h"
#include "common.h"
#include <linux/soc/ti/ti_sci_protocol.h>
#include <dm.h>
#include <dm/uclass-internal.h>
#include <dm/pinctrl.h>
#include <mmc.h>
#include <dm/root.h>
#include <command.h>

#define CTRLMMR_MCU_RST_CTRL			0x04518170

#define CTRLMMR_MCU_RST_SRC                    (MCU_CTRL_MMR0_BASE + 0x18178)
#define COLD_BOOT                              0
#define SW_POR_MCU                             BIT(24)
#define SW_POR_MAIN                            BIT(25)

static void ctrl_mmr_unlock(void)
{
	/* Unlock all PADCFG_MMR1 module registers */
	mmr_unlock(PADCFG_MMR1_BASE, 1);

	/* Unlock all MCU_CTRL_MMR0 module registers */
	mmr_unlock(MCU_CTRL_MMR0_BASE, 0);
	mmr_unlock(MCU_CTRL_MMR0_BASE, 1);
	mmr_unlock(MCU_CTRL_MMR0_BASE, 2);
	mmr_unlock(MCU_CTRL_MMR0_BASE, 3);
	mmr_unlock(MCU_CTRL_MMR0_BASE, 4);
	mmr_unlock(MCU_CTRL_MMR0_BASE, 6);

	/* Unlock all CTRL_MMR0 module registers */
	mmr_unlock(CTRL_MMR0_BASE, 0);
	mmr_unlock(CTRL_MMR0_BASE, 1);
	mmr_unlock(CTRL_MMR0_BASE, 2);
	mmr_unlock(CTRL_MMR0_BASE, 3);
	mmr_unlock(CTRL_MMR0_BASE, 5);
	mmr_unlock(CTRL_MMR0_BASE, 6);

	/* Unlock all MCU_PADCFG_MMR1 module registers */
	mmr_unlock(MCU_PADCFG_MMR1_BASE, 1);
}

/*
 * This uninitialized global variable would normal end up in the .bss section,
 * but the .bss is cleared between writing and reading this variable, so move
 * it to the .data section.
 */
u32 bootindex __section(".data");
static struct rom_extended_boot_data bootdata __section(".data");

static void store_boot_info_from_rom(void)
{
	bootindex = *(u32 *)(CONFIG_SYS_K3_BOOT_PARAM_TABLE_INDEX);
	memcpy(&bootdata, (uintptr_t *)ROM_EXTENDED_BOOT_DATA_INFO,
	       sizeof(struct rom_extended_boot_data));
}

#if defined(CONFIG_K3_LOAD_SYSFW) && CONFIG_IS_ENABLED(DM_MMC)
void k3_mmc_stop_clock(void)
{
	if (spl_boot_device() == BOOT_DEVICE_MMC1) {
		struct mmc *mmc = find_mmc_device(0);

		if (!mmc)
			return;

		mmc->saved_clock = mmc->clock;
		mmc_set_clock(mmc, 0, true);
	}
}

void k3_mmc_restart_clock(void)
{
	if (spl_boot_device() == BOOT_DEVICE_MMC1) {
		struct mmc *mmc = find_mmc_device(0);

		if (!mmc)
			return;

		mmc_set_clock(mmc, mmc->saved_clock, false);
	}
}
#else
void k3_mmc_stop_clock(void) {}
void k3_mmc_restart_clock(void) {}
#endif

#ifdef CONFIG_SPL_OF_LIST
void do_dt_magic(void)
{
	int ret, rescan;

	/* Perform board detection */
	do_board_detect();

	/*
	 * Board detection has been done.
	 * Let us see if another dtb wouldn't be a better match
	 * for our board
	 */
	if (IS_ENABLED(CONFIG_CPU_V7R)) {
		ret = fdtdec_resetup(&rescan);
		if (!ret && rescan) {
			dm_uninit();
			dm_init_and_scan(true);
		}
	}
}
#endif

#if CONFIG_IS_ENABLED(USB_STORAGE)
static int fixup_usb_boot(const void *fdt_blob)
{
	int ret = 0;

	switch (spl_boot_device()) {
	case BOOT_DEVICE_USB:
		/*
		 * If the boot mode is host, fixup the dr_mode to host
		 * before cdns3 bind takes place
		 */
		ret = fdt_find_and_setprop((void *)fdt_blob,
					   "/bus@f4000/cdns-usb@f900000/usb@f400000",
					   "dr_mode", "host", 5, 0);
		if (ret)
			printf("%s: fdt_find_and_setprop() failed:%d\n",
			       __func__, ret);
		fallthrough;
	default:
		break;
	}

	return ret;
}

int fdtdec_board_setup(const void *fdt_blob)
{
	/* Can use the pointer from the function parameters */
	return fixup_usb_boot(fdt_blob);
}
#endif

#if defined(CONFIG_ESM_K3)
static void enable_mcu_esm_reset(void)
{
	/* Set CTRLMMR_MCU_RST_CTRL:MCU_ESM_ERROR_RST_EN_Z  to '0' (low active) */
	u32 stat = readl(CTRLMMR_MCU_RST_CTRL);

	stat &= 0xFFFDFFFF;
	writel(stat, CTRLMMR_MCU_RST_CTRL);
}
#endif

void board_init_f(ulong dummy)
{
#if defined(CONFIG_K3_LOAD_SYSFW) || defined(CONFIG_K3_AM64_DDRSS) || defined(CONFIG_ESM_K3)
	struct udevice *dev;
	int ret;
	int rst_src;
#endif

#if defined(CONFIG_CPU_V7R)
	setup_k3_mpu_regions();
#endif

	/*
	 * Cannot delay this further as there is a chance that
	 * K3_BOOT_PARAM_TABLE_INDEX can be over written by SPL MALLOC section.
	 */
	store_boot_info_from_rom();

	ctrl_mmr_unlock();

	/* Init DM early */
	spl_early_init();

	preloader_console_init();

#if defined(CONFIG_K3_LOAD_SYSFW)
	/*
	 * Process pinctrl for serial3 a.k.a. MAIN UART1 module and continue
	 * regardless of the result of pinctrl. Do this without probing the
	 * device, but instead by searching the device that would request the
	 * given sequence number if probed. The UART will be used by the system
	 * firmware (SYSFW) image for various purposes and SYSFW depends on us
	 * to initialize its pin settings.
	 */
	ret = uclass_find_device_by_seq(UCLASS_SERIAL, 3, &dev);
	if (!ret)
		pinctrl_select_state(dev, "default");

	/*
	 * Load, start up, and configure system controller firmware.
	 * This will determine whether or not ROM has already loaded
	 * system firmware and if so, will only perform needed config
	 * and not attempt to load firmware again.
	 */
	k3_sysfw_loader(is_rom_loaded_sysfw(&bootdata), k3_mmc_stop_clock,
			k3_mmc_restart_clock);
#endif

#if defined(CONFIG_CPU_V7R)
	/*
	 * Errata ID i2331 CPSW: A device lockup can occur during the second
	 * read of any CPSW subsystem register after any MAIN domain power on
	 * reset (POR). A MAIN domain POR occurs using the hardware MCU_PORz
	 * signal, or via software using CTRLMMR_RST_CTRL.SW_MAIN_POR or
	 * CTRLMMR_MCU_RST_CTRL.SW_MAIN_POR. After these resets, the processor
	 * and internal bus structures may get into a state which is only
	 * recoverable with full device reset using MCU_PORz.
	 * Workaround(s): To avoid the lockup, a warm reset should be issued
	 * after a MAIN domain POR and before any access to the CPSW registers.
	 * The warm reset realigns internal clocks and prevents the lockup from
	 * happening.
	 */
	ret = uclass_first_device_err(UCLASS_SYSRESET, &dev);
	if (ret)
		printf("\n%s:uclass device error [%d]\n",__func__,ret);

	rst_src = readl(CTRLMMR_MCU_RST_SRC);
	if (rst_src == COLD_BOOT || rst_src & (SW_POR_MCU | SW_POR_MAIN)) {
		printf("Resetting on cold boot to workaround ErrataID:i2331\n");
		printf("Please resend tiboot3.bin in case of UART/DFU boot\n");
		do_reset(NULL, 0, 0, NULL);
	}
#endif

	/* Output System Firmware version info */
	k3_sysfw_print_ver();

	do_dt_magic();

#if defined(CONFIG_ESM_K3)
	/* Probe/configure ESM0 */
	ret = uclass_get_device_by_name(UCLASS_MISC, "esm@420000", &dev);
	if (ret)
		printf("esm main init failed: %d\n", ret);

	/* Probe/configure MCUESM */
	ret = uclass_get_device_by_name(UCLASS_MISC, "esm@4100000", &dev);
	if (ret)
		printf("esm mcu init failed: %d\n", ret);

	enable_mcu_esm_reset();
#endif

#if defined(CONFIG_K3_AM64_DDRSS)
	ret = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (ret)
		panic("DRAM init failed: %d\n", ret);
#endif
	if (IS_ENABLED(CONFIG_SPL_ETH) && IS_ENABLED(CONFIG_TI_AM65_CPSW_NUSS) &&
	    spl_boot_device() == BOOT_DEVICE_ETHERNET) {
		struct udevice *cpswdev;

		if (uclass_get_device_by_driver(UCLASS_MISC, DM_DRIVER_GET(am65_cpsw_nuss), &cpswdev))
			printf("Failed to probe am65_cpsw_nuss driver\n");
	}
}

u32 spl_mmc_boot_mode(struct mmc *mmc, const u32 boot_device)
{
	switch (boot_device) {
	case BOOT_DEVICE_MMC1:
		return MMCSD_MODE_EMMCBOOT;

	case BOOT_DEVICE_MMC2:
		return MMCSD_MODE_FS;

	default:
		return MMCSD_MODE_RAW;
	}
}

static u32 __get_backup_bootmedia(u32 main_devstat)
{
	u32 bkup_bootmode =
	    (main_devstat & MAIN_DEVSTAT_BACKUP_BOOTMODE_MASK) >>
	    MAIN_DEVSTAT_BACKUP_BOOTMODE_SHIFT;
	u32 bkup_bootmode_cfg =
	    (main_devstat & MAIN_DEVSTAT_BACKUP_BOOTMODE_CFG_MASK) >>
	    MAIN_DEVSTAT_BACKUP_BOOTMODE_CFG_SHIFT;

	switch (bkup_bootmode) {
	case BACKUP_BOOT_DEVICE_UART:
		return BOOT_DEVICE_UART;

	case BACKUP_BOOT_DEVICE_DFU:
		if (bkup_bootmode_cfg & MAIN_DEVSTAT_BACKUP_USB_MODE_MASK)
			return BOOT_DEVICE_USB;
		return BOOT_DEVICE_DFU;


	case BACKUP_BOOT_DEVICE_ETHERNET:
		return BOOT_DEVICE_ETHERNET;

	case BACKUP_BOOT_DEVICE_MMC:
		if (bkup_bootmode_cfg)
			return BOOT_DEVICE_MMC2;
		return BOOT_DEVICE_MMC1;

	case BACKUP_BOOT_DEVICE_SPI:
		return BOOT_DEVICE_SPI;

	case BACKUP_BOOT_DEVICE_I2C:
		return BOOT_DEVICE_I2C;
	};

	return BOOT_DEVICE_RAM;
}

static u32 __get_primary_bootmedia(u32 main_devstat)
{
	u32 bootmode = (main_devstat & MAIN_DEVSTAT_PRIMARY_BOOTMODE_MASK) >>
	    MAIN_DEVSTAT_PRIMARY_BOOTMODE_SHIFT;
	u32 bootmode_cfg =
	    (main_devstat & MAIN_DEVSTAT_PRIMARY_BOOTMODE_CFG_MASK) >>
	    MAIN_DEVSTAT_PRIMARY_BOOTMODE_CFG_SHIFT;

	switch (bootmode) {
	case BOOT_DEVICE_OSPI:
		fallthrough;
	case BOOT_DEVICE_QSPI:
		fallthrough;
	case BOOT_DEVICE_XSPI:
		fallthrough;
	case BOOT_DEVICE_SPI:
		return BOOT_DEVICE_SPI;

	case BOOT_DEVICE_ETHERNET_RGMII:
		fallthrough;
	case BOOT_DEVICE_ETHERNET_RMII:
		return BOOT_DEVICE_ETHERNET;

	case BOOT_DEVICE_EMMC:
		return BOOT_DEVICE_MMC1;

	case BOOT_DEVICE_NAND:
		return BOOT_DEVICE_NAND;

	case BOOT_DEVICE_MMC:
		if ((bootmode_cfg & MAIN_DEVSTAT_PRIMARY_MMC_PORT_MASK) >>
		     MAIN_DEVSTAT_PRIMARY_MMC_PORT_SHIFT)
			return BOOT_DEVICE_MMC2;
		return BOOT_DEVICE_MMC1;

	case BOOT_DEVICE_DFU:
		if ((bootmode_cfg & MAIN_DEVSTAT_PRIMARY_USB_MODE_MASK) >>
		    MAIN_DEVSTAT_PRIMARY_USB_MODE_SHIFT)
			return BOOT_DEVICE_USB;
		return BOOT_DEVICE_DFU;

	case BOOT_DEVICE_NOBOOT:
		return BOOT_DEVICE_RAM;
	}

	return bootmode;
}

u32 spl_boot_device(void)
{
	u32 devstat = readl(CTRLMMR_MAIN_DEVSTAT);

	if (bootindex == K3_PRIMARY_BOOTMODE)
		return __get_primary_bootmedia(devstat);
	else
		return __get_backup_bootmedia(devstat);
}
