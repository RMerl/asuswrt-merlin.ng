// SPDX-License-Identifier: GPL-2.0+
/*
 * boot-common.c
 *
 * Common bootmode functions for omap based boards
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - https://www.ti.com/
 */

#include <common.h>
#include <ahci.h>
#include <log.h>
#include <dm/uclass.h>
#include <fs_loader.h>
#include <spl.h>
#include <asm/global_data.h>
#include <asm/omap_common.h>
#include <asm/omap_sec_common.h>
#include <asm/arch/omap.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/sys_proto.h>
#include <watchdog.h>
#include <scsi.h>
#include <i2c.h>
#include <remoteproc.h>
#include <image.h>

DECLARE_GLOBAL_DATA_PTR;

#define IPU1_LOAD_ADDR         (0xa17ff000)
#define MAX_REMOTECORE_BIN_SIZE (8 * 0x100000)
#define IPU2_LOAD_ADDR         (IPU1_LOAD_ADDR + MAX_REMOTECORE_BIN_SIZE)

__weak u32 omap_sys_boot_device(void)
{
	return BOOT_DEVICE_NONE;
}

void save_omap_boot_params(void)
{
	u32 boot_params = *((u32 *)OMAP_SRAM_SCRATCH_BOOT_PARAMS);
	struct omap_boot_parameters *omap_boot_params;
	int sys_boot_device = 0;
	u32 boot_device;
	u32 boot_mode;

	if ((boot_params < NON_SECURE_SRAM_START) ||
	    (boot_params > NON_SECURE_SRAM_END))
		return;

	omap_boot_params = (struct omap_boot_parameters *)boot_params;

	boot_device = omap_boot_params->boot_device;
	boot_mode = MMCSD_MODE_UNDEFINED;

	/* Boot device */

#ifdef BOOT_DEVICE_NAND_I2C
	/*
	 * Re-map NAND&I2C boot-device to the "normal" NAND boot-device.
	 * Otherwise the SPL boot IF can't handle this device correctly.
	 * Somehow booting with Hynix 4GBit NAND H27U4G8 on Siemens
	 * Draco leads to this boot-device passed to SPL from the BootROM.
	 */
	if (boot_device == BOOT_DEVICE_NAND_I2C)
		boot_device = BOOT_DEVICE_NAND;
#endif
#ifdef BOOT_DEVICE_QSPI_4
	/*
	 * We get different values for QSPI_1 and QSPI_4 being used, but
	 * don't actually care about this difference.  Rather than
	 * mangle the later code, if we're coming in as QSPI_4 just
	 * change to the QSPI_1 value.
	 */
	if (boot_device == BOOT_DEVICE_QSPI_4)
		boot_device = BOOT_DEVICE_SPI;
#endif
	/*
	 * When booting from peripheral booting, the boot device is not usable
	 * as-is (unless there is support for it), so the boot device is instead
	 * figured out using the SYS_BOOT pins.
	 */
	switch (boot_device) {
#if defined(BOOT_DEVICE_UART) && !defined(CONFIG_SPL_YMODEM_SUPPORT)
		case BOOT_DEVICE_UART:
			sys_boot_device = 1;
			break;
#endif
#if defined(BOOT_DEVICE_USB) && !defined(CONFIG_SPL_USB_STORAGE)
		case BOOT_DEVICE_USB:
			sys_boot_device = 1;
			break;
#endif
#if defined(BOOT_DEVICE_USBETH) && !defined(CONFIG_SPL_USB_ETHER)
		case BOOT_DEVICE_USBETH:
			sys_boot_device = 1;
			break;
#endif
#if defined(BOOT_DEVICE_CPGMAC) && !defined(CONFIG_SPL_ETH)
		case BOOT_DEVICE_CPGMAC:
			sys_boot_device = 1;
			break;
#endif
#if defined(BOOT_DEVICE_DFU) && !defined(CONFIG_SPL_DFU)
		case BOOT_DEVICE_DFU:
			sys_boot_device = 1;
			break;
#endif
	}

	if (sys_boot_device) {
		boot_device = omap_sys_boot_device();

		/* MMC raw mode will fallback to FS mode. */
		if ((boot_device >= MMC_BOOT_DEVICES_START) &&
		    (boot_device <= MMC_BOOT_DEVICES_END))
			boot_mode = MMCSD_MODE_RAW;
	}

	gd->arch.omap_boot_device = boot_device;

	/* Boot mode */

#ifdef CONFIG_OMAP34XX
	if ((boot_device >= MMC_BOOT_DEVICES_START) &&
	    (boot_device <= MMC_BOOT_DEVICES_END)) {
		switch (boot_device) {
		case BOOT_DEVICE_MMC1:
			boot_mode = MMCSD_MODE_FS;
			break;
		case BOOT_DEVICE_MMC2:
			boot_mode = MMCSD_MODE_RAW;
			break;
		}
	}
#else
	/*
	 * If the boot device was dynamically changed and doesn't match what
	 * the bootrom initially booted, we cannot use the boot device
	 * descriptor to figure out the boot mode.
	 */
	if ((boot_device == omap_boot_params->boot_device) &&
	    (boot_device >= MMC_BOOT_DEVICES_START) &&
	    (boot_device <= MMC_BOOT_DEVICES_END)) {
		boot_params = omap_boot_params->boot_device_descriptor;
		if ((boot_params < NON_SECURE_SRAM_START) ||
		    (boot_params > NON_SECURE_SRAM_END))
			return;

		boot_params = *((u32 *)(boot_params + DEVICE_DATA_OFFSET));
		if ((boot_params < NON_SECURE_SRAM_START) ||
		    (boot_params > NON_SECURE_SRAM_END))
			return;

		boot_mode = *((u32 *)(boot_params + BOOT_MODE_OFFSET));

		if (boot_mode != MMCSD_MODE_FS &&
		    boot_mode != MMCSD_MODE_RAW)
#ifdef CONFIG_SUPPORT_EMMC_BOOT
			boot_mode = MMCSD_MODE_EMMCBOOT;
#else
			boot_mode = MMCSD_MODE_UNDEFINED;
#endif
	}
#endif

	gd->arch.omap_boot_mode = boot_mode;

#if !defined(CONFIG_AM33XX) && !defined(CONFIG_AM43XX)

	/* CH flags */

	gd->arch.omap_ch_flags = omap_boot_params->ch_flags;
#endif
}

#ifdef CONFIG_SPL_BUILD
u32 spl_boot_device(void)
{
	return gd->arch.omap_boot_device;
}

u32 spl_mmc_boot_mode(struct mmc *mmc, const u32 boot_device)
{
	return gd->arch.omap_boot_mode;
}

int load_firmware(char *name_fw, u32 *loadaddr)
{
	struct udevice *fsdev;
	int size = 0;

	if (!IS_ENABLED(CONFIG_FS_LOADER))
		return 0;

	if (!*loadaddr)
		return 0;

	if (!get_fs_loader(&fsdev)) {
		size = request_firmware_into_buf(fsdev, name_fw,
						 (void *)*loadaddr, 0, 0);
	}

	return size;
}

void spl_boot_ipu(void)
{
	int ret, size;
	u32 loadaddr = IPU1_LOAD_ADDR;

	if (!IS_ENABLED(CONFIG_SPL_BUILD) ||
	    !IS_ENABLED(CONFIG_REMOTEPROC_TI_IPU))
		return;

	size = load_firmware("dra7-ipu1-fw.xem4", &loadaddr);
	if (size <= 0) {
		pr_err("Firmware loading failed\n");
		goto skip_ipu1;
	}

	enable_ipu1_clocks();
	ret = rproc_dev_init(0);
	if (ret) {
		debug("%s: IPU1 failed to initialize on rproc (%d)\n",
		      __func__, ret);
		goto skip_ipu1;
	}

	ret = rproc_load(0, IPU1_LOAD_ADDR, 0x2000000);
	if (ret) {
		debug("%s: IPU1 failed to load on rproc (%d)\n", __func__,
		      ret);
		goto skip_ipu1;
	}

	debug("Starting IPU1...\n");

	ret = rproc_start(0);
	if (ret)
		debug("%s: IPU1 failed to start (%d)\n", __func__, ret);

skip_ipu1:
	loadaddr = IPU2_LOAD_ADDR;
	size = load_firmware("dra7-ipu2-fw.xem4", &loadaddr);
	if (size <= 0) {
		pr_err("Firmware loading failed for ipu2\n");
		return;
	}

	enable_ipu2_clocks();
	ret = rproc_dev_init(1);
	if (ret) {
		debug("%s: IPU2 failed to initialize on rproc (%d)\n", __func__,
		      ret);
		return;
	}

	ret = rproc_load(1, IPU2_LOAD_ADDR, 0x2000000);
	if (ret) {
		debug("%s: IPU2 failed to load on rproc (%d)\n", __func__,
		      ret);
		return;
	}

	debug("Starting IPU2...\n");

	ret = rproc_start(1);
	if (ret)
		debug("%s: IPU2 failed to start (%d)\n", __func__, ret);
}

void spl_board_init(void)
{
	/* Prepare console output */
	preloader_console_init();

#if defined(CONFIG_SPL_NAND_SUPPORT) || defined(CONFIG_SPL_ONENAND_SUPPORT)
	gpmc_init();
#endif
#if defined(CONFIG_SPL_I2C) && !CONFIG_IS_ENABLED(DM_I2C)
	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
#endif
#if defined(CONFIG_AM33XX) && defined(CONFIG_SPL_MUSB_NEW)
	arch_misc_init();
#endif
#if defined(CONFIG_HW_WATCHDOG) || defined(CONFIG_WATCHDOG)
	hw_watchdog_init();
#endif
#ifdef CONFIG_AM33XX
	am33xx_spl_board_init();
#endif
	if (IS_ENABLED(CONFIG_SPL_BUILD) &&
	    IS_ENABLED(CONFIG_REMOTEPROC_TI_IPU))
		spl_boot_ipu();
}

void __noreturn jump_to_image_no_args(struct spl_image_info *spl_image)
{
	typedef void __noreturn (*image_entry_noargs_t)(u32 *);
	image_entry_noargs_t image_entry =
			(image_entry_noargs_t) spl_image->entry_point;

	u32 boot_params = *((u32 *)OMAP_SRAM_SCRATCH_BOOT_PARAMS);

	debug("image entry point: 0x%lX\n", spl_image->entry_point);
	/* Pass the saved boot_params from rom code */
	image_entry((u32 *)boot_params);
}
#endif

#ifdef CONFIG_TI_SECURE_DEVICE
void board_fit_image_post_process(const void *fit, int node, void **p_image,
				  size_t *p_size)
{
	secure_boot_verify_image(p_image, p_size);
}

static void tee_image_process(ulong tee_image, size_t tee_size)
{
	secure_tee_install((u32)tee_image);
}
U_BOOT_FIT_LOADABLE_HANDLER(IH_TYPE_TEE, tee_image_process);
#endif
