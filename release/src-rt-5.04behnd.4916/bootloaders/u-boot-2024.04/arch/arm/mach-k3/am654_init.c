// SPDX-License-Identifier: GPL-2.0+
/*
 * AM6: SoC specific initialization
 *
 * Copyright (C) 2017-2018 Texas Instruments Incorporated - https://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 */

#include <fdt_support.h>
#include <init.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <spl.h>
#include <asm/arch/hardware.h>
#include "sysfw-loader.h"
#include "common.h"
#include <dm.h>
#include <dm/uclass-internal.h>
#include <dm/pinctrl.h>
#include <linux/soc/ti/ti_sci_protocol.h>
#include <log.h>
#include <mmc.h>
#include <stdlib.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_K3_LOAD_SYSFW
struct fwl_data main_cbass_fwls[] = {
	{ "MMCSD1_CFG", 2057, 1 },
	{ "MMCSD0_CFG", 2058, 1 },
	{ "USB3SS0_SLV0", 2176, 2 },
	{ "PCIE0_SLV", 2336, 8 },
	{ "PCIE1_SLV", 2337, 8 },
	{ "PCIE0_CFG", 2688, 1 },
	{ "PCIE1_CFG", 2689, 1 },
}, mcu_cbass_fwls[] = {
	{ "MCU_ARMSS0_CORE0_SLV", 1024, 1 },
	{ "MCU_ARMSS0_CORE1_SLV", 1028, 1 },
	{ "MCU_FSS0_S1", 1033, 8 },
	{ "MCU_FSS0_S0", 1036, 8 },
	{ "MCU_CPSW0", 1220, 1 },
};
#endif

static void ctrl_mmr_unlock(void)
{
	/* Unlock all WKUP_CTRL_MMR0 module registers */
	mmr_unlock(WKUP_CTRL_MMR0_BASE, 0);
	mmr_unlock(WKUP_CTRL_MMR0_BASE, 1);
	mmr_unlock(WKUP_CTRL_MMR0_BASE, 2);
	mmr_unlock(WKUP_CTRL_MMR0_BASE, 3);
	mmr_unlock(WKUP_CTRL_MMR0_BASE, 6);
	mmr_unlock(WKUP_CTRL_MMR0_BASE, 7);

	/* Unlock all MCU_CTRL_MMR0 module registers */
	mmr_unlock(MCU_CTRL_MMR0_BASE, 0);
	mmr_unlock(MCU_CTRL_MMR0_BASE, 1);
	mmr_unlock(MCU_CTRL_MMR0_BASE, 2);
	mmr_unlock(MCU_CTRL_MMR0_BASE, 6);

	/* Unlock all CTRL_MMR0 module registers */
	mmr_unlock(CTRL_MMR0_BASE, 0);
	mmr_unlock(CTRL_MMR0_BASE, 1);
	mmr_unlock(CTRL_MMR0_BASE, 2);
	mmr_unlock(CTRL_MMR0_BASE, 3);
	mmr_unlock(CTRL_MMR0_BASE, 6);
	mmr_unlock(CTRL_MMR0_BASE, 7);
}

/*
 * This uninitialized global variable would normal end up in the .bss section,
 * but the .bss is cleared between writing and reading this variable, so move
 * it to the .data section.
 */
u32 bootindex __section(".data");

static void store_boot_index_from_rom(void)
{
	bootindex = *(u32 *)(CONFIG_SYS_K3_BOOT_PARAM_TABLE_INDEX);
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
#if CONFIG_IS_ENABLED(DFU) || CONFIG_IS_ENABLED(USB_STORAGE)
#define CTRLMMR_SERDES0_CTRL	0x00104080
#define PCIE_LANE0		0x1
static int fixup_usb_boot(void)
{
	int ret;

	switch (spl_boot_device()) {
	case BOOT_DEVICE_USB:
		/*
		 * If bootmode is Host bootmode, fixup the dr_mode to host
		 * before the dwc3 bind takes place
		 */
		ret = fdt_find_and_setprop((void *)gd->fdt_blob,
				"/bus@100000/dwc3@4000000/usb@10000",
				"dr_mode", "host", 5, 0);
		if (ret)
			printf("%s: fdt_find_and_setprop() failed:%d\n", __func__,
			       ret);
		fallthrough;
	case BOOT_DEVICE_DFU:
		/*
		 * The serdes mux between PCIe and USB3 needs to be set to PCIe for
		 * accessing the interface at USB 2.0
		 */
		writel(PCIE_LANE0, CTRLMMR_SERDES0_CTRL);
	default:
		break;
	}

	return 0;
}

int fdtdec_board_setup(const void *fdt_blob)
{
	return fixup_usb_boot();
}
#endif

static void setup_am654_navss_northbridge(void)
{
	/*
	 * NB0 is bridge to SRAM and NB1 is bridge to DDR.
	 * To ensure that SRAM transfers are not stalled due to
	 * delays during DDR refreshes, SRAM traffic should be higher
	 * priority (threadmap=2) than DDR traffic (threadmap=0).
	 */
	writel(0x2, NAVSS0_NBSS_NB0_CFG_BASE + NAVSS_NBSS_THREADMAP);
	writel(0x0, NAVSS0_NBSS_NB1_CFG_BASE + NAVSS_NBSS_THREADMAP);
}

void board_init_f(ulong dummy)
{
#if defined(CONFIG_K3_LOAD_SYSFW) || defined(CONFIG_K3_AM654_DDRSS)
	struct udevice *dev;
	size_t pool_size;
	void *pool_addr;
	int ret;
#endif
	/*
	 * Cannot delay this further as there is a chance that
	 * K3_BOOT_PARAM_TABLE_INDEX can be over written by SPL MALLOC section.
	 */
	store_boot_index_from_rom();

	/* Make all control module registers accessible */
	ctrl_mmr_unlock();

	setup_am654_navss_northbridge();

#ifdef CONFIG_CPU_V7R
	disable_linefill_optimization();
	setup_k3_mpu_regions();
#endif

	/* Init DM early in-order to invoke system controller */
	spl_early_init();

#ifdef CONFIG_K3_EARLY_CONS
	/*
	 * Allow establishing an early console as required for example when
	 * doing a UART-based boot. Note that this console may not "survive"
	 * through a SYSFW PM-init step and will need a re-init in some way
	 * due to changing module clock frequencies.
	 */
	early_console_init();
#endif

#ifdef CONFIG_K3_LOAD_SYSFW
	/*
	 * Initialize an early full malloc environment. Do so by allocating a
	 * new malloc area inside the currently active pre-relocation "first"
	 * malloc pool of which we use all that's left.
	 */
	pool_size = CONFIG_VAL(SYS_MALLOC_F_LEN) - gd->malloc_ptr;
	pool_addr = malloc(pool_size);
	if (!pool_addr)
		panic("ERROR: Can't allocate full malloc pool!\n");

	mem_malloc_init((ulong)pool_addr, (ulong)pool_size);
	gd->flags |= GD_FLG_FULL_MALLOC_INIT;
	debug("%s: initialized an early full malloc pool at 0x%08lx of 0x%lx bytes\n",
	      __func__, (unsigned long)pool_addr, (unsigned long)pool_size);
	/*
	 * Process pinctrl for the serial0 a.k.a. WKUP_UART0 module and continue
	 * regardless of the result of pinctrl. Do this without probing the
	 * device, but instead by searching the device that would request the
	 * given sequence number if probed. The UART will be used by the system
	 * firmware (SYSFW) image for various purposes and SYSFW depends on us
	 * to initialize its pin settings.
	 */
	ret = uclass_find_device_by_seq(UCLASS_SERIAL, 0, &dev);
	if (!ret)
		pinctrl_select_state(dev, "default");

	/*
	 * Load, start up, and configure system controller firmware while
	 * also populating the SYSFW post-PM configuration callback hook.
	 */
	k3_sysfw_loader(false, k3_mmc_stop_clock, k3_mmc_restart_clock);

	/* Prepare console output */
	preloader_console_init();

	/* Disable ROM configured firewalls right after loading sysfw */
	remove_fwl_configs(main_cbass_fwls, ARRAY_SIZE(main_cbass_fwls));
	remove_fwl_configs(mcu_cbass_fwls, ARRAY_SIZE(mcu_cbass_fwls));
#else
	/* Prepare console output */
	preloader_console_init();
#endif

	/* Output System Firmware version info */
	k3_sysfw_print_ver();

	/* Perform board detection */
	do_board_detect();

#if defined(CONFIG_CPU_V7R) && defined(CONFIG_K3_AVS0)
	ret = uclass_get_device_by_driver(UCLASS_MISC, DM_DRIVER_GET(k3_avs),
					  &dev);
	if (ret)
		printf("AVS init failed: %d\n", ret);
#endif

#ifdef CONFIG_K3_AM654_DDRSS
	ret = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (ret)
		panic("DRAM init failed: %d\n", ret);
#endif
	spl_enable_cache();
}

u32 spl_mmc_boot_mode(struct mmc *mmc, const u32 boot_device)
{
#if defined(CONFIG_SUPPORT_EMMC_BOOT)
	u32 devstat = readl(CTRLMMR_MAIN_DEVSTAT);

	u32 bootmode = (devstat & CTRLMMR_MAIN_DEVSTAT_BOOTMODE_MASK) >>
			CTRLMMR_MAIN_DEVSTAT_BOOTMODE_SHIFT;

	/* eMMC boot0 mode is only supported for primary boot */
	if (bootindex == K3_PRIMARY_BOOTMODE &&
	    bootmode == BOOT_DEVICE_MMC1)
		return MMCSD_MODE_EMMCBOOT;
#endif

	/* Everything else use filesystem if available */
#if defined(CONFIG_SPL_FS_FAT) || defined(CONFIG_SPL_FS_EXT4)
	return MMCSD_MODE_FS;
#else
	return MMCSD_MODE_RAW;
#endif
}

static u32 __get_backup_bootmedia(u32 devstat)
{
	u32 bkup_boot = (devstat & CTRLMMR_MAIN_DEVSTAT_BKUP_BOOTMODE_MASK) >>
			CTRLMMR_MAIN_DEVSTAT_BKUP_BOOTMODE_SHIFT;

	switch (bkup_boot) {
	case BACKUP_BOOT_DEVICE_USB:
		return BOOT_DEVICE_USB;
	case BACKUP_BOOT_DEVICE_UART:
		return BOOT_DEVICE_UART;
	case BACKUP_BOOT_DEVICE_ETHERNET:
		return BOOT_DEVICE_ETHERNET;
	case BACKUP_BOOT_DEVICE_MMC2:
	{
		u32 port = (devstat & CTRLMMR_MAIN_DEVSTAT_BKUP_MMC_PORT_MASK) >>
			    CTRLMMR_MAIN_DEVSTAT_BKUP_MMC_PORT_SHIFT;
		if (port == 0x0)
			return BOOT_DEVICE_MMC1;
		return BOOT_DEVICE_MMC2;
	}
	case BACKUP_BOOT_DEVICE_SPI:
		return BOOT_DEVICE_SPI;
	case BACKUP_BOOT_DEVICE_HYPERFLASH:
		return BOOT_DEVICE_HYPERFLASH;
	case BACKUP_BOOT_DEVICE_I2C:
		return BOOT_DEVICE_I2C;
	};

	return BOOT_DEVICE_RAM;
}

static u32 __get_primary_bootmedia(u32 devstat)
{
	u32 bootmode = (devstat & CTRLMMR_MAIN_DEVSTAT_BOOTMODE_MASK) >>
			CTRLMMR_MAIN_DEVSTAT_BOOTMODE_SHIFT;

	if (bootmode == BOOT_DEVICE_OSPI || bootmode ==	BOOT_DEVICE_QSPI)
		bootmode = BOOT_DEVICE_SPI;

	if (bootmode == BOOT_DEVICE_MMC2) {
		u32 port = (devstat & CTRLMMR_MAIN_DEVSTAT_MMC_PORT_MASK) >>
			    CTRLMMR_MAIN_DEVSTAT_MMC_PORT_SHIFT;
		if (port == 0x0)
			bootmode = BOOT_DEVICE_MMC1;
	} else if (bootmode == BOOT_DEVICE_MMC1) {
		u32 port = (devstat & CTRLMMR_MAIN_DEVSTAT_EMMC_PORT_MASK) >>
			    CTRLMMR_MAIN_DEVSTAT_EMMC_PORT_SHIFT;
		if (port == 0x1)
			bootmode = BOOT_DEVICE_MMC2;
	} else if (bootmode == BOOT_DEVICE_DFU) {
		u32 mode = (devstat & CTRLMMR_MAIN_DEVSTAT_USB_MODE_MASK) >>
			    CTRLMMR_MAIN_DEVSTAT_USB_MODE_SHIFT;
		if (mode == 0x2)
			bootmode = BOOT_DEVICE_USB;
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
