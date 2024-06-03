// SPDX-License-Identifier: GPL-2.0+
/*
 *  (C) Copyright 2010,2011
 *  NVIDIA Corporation <www.nvidia.com>
 */

#include <common.h>
#include <dm.h>
#include <efi_loader.h>
#include <errno.h>
#include <ns16550.h>
#include <usb.h>
#include <asm/io.h>
#include <asm/arch-tegra/ap.h>
#include <asm/arch-tegra/board.h>
#include <asm/arch-tegra/cboot.h>
#include <asm/arch-tegra/clk_rst.h>
#include <asm/arch-tegra/pmc.h>
#include <asm/arch-tegra/pmu.h>
#include <asm/arch-tegra/sys_proto.h>
#include <asm/arch-tegra/uart.h>
#include <asm/arch-tegra/warmboot.h>
#include <asm/arch-tegra/gpu.h>
#include <asm/arch-tegra/usb.h>
#include <asm/arch-tegra/xusb-padctl.h>
#if IS_ENABLED(CONFIG_TEGRA_CLKRST)
#include <asm/arch/clock.h>
#endif
#if IS_ENABLED(CONFIG_TEGRA_PINCTRL)
#include <asm/arch/funcmux.h>
#include <asm/arch/pinmux.h>
#endif
#include <asm/arch/tegra.h>
#ifdef CONFIG_TEGRA_CLOCK_SCALING
#include <asm/arch/emc.h>
#endif
#include "emc.h"

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SPL_BUILD
/* TODO(sjg@chromium.org): Remove once SPL supports device tree */
U_BOOT_DEVICE(tegra_gpios) = {
	"gpio_tegra"
};
#endif

__weak void pinmux_init(void) {}
__weak void pin_mux_usb(void) {}
__weak void pin_mux_spi(void) {}
__weak void pin_mux_mmc(void) {}
__weak void gpio_early_init_uart(void) {}
__weak void pin_mux_display(void) {}
__weak void start_cpu_fan(void) {}
__weak void cboot_late_init(void) {}

#if defined(CONFIG_TEGRA_NAND)
__weak void pin_mux_nand(void)
{
	funcmux_select(PERIPH_ID_NDFLASH, FUNCMUX_DEFAULT);
}
#endif

/*
 * Routine: power_det_init
 * Description: turn off power detects
 */
static void power_det_init(void)
{
#if defined(CONFIG_TEGRA20)
	struct pmc_ctlr *const pmc = (struct pmc_ctlr *)NV_PA_PMC_BASE;

	/* turn off power detects */
	writel(0, &pmc->pmc_pwr_det_latch);
	writel(0, &pmc->pmc_pwr_det);
#endif
}

__weak int tegra_board_id(void)
{
	return -1;
}

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	int board_id = tegra_board_id();

	printf("Board: %s", CONFIG_TEGRA_BOARD_STRING);
	if (board_id != -1)
		printf(", ID: %d\n", board_id);
	printf("\n");

	return 0;
}
#endif	/* CONFIG_DISPLAY_BOARDINFO */

__weak int tegra_lcd_pmic_init(int board_it)
{
	return 0;
}

__weak int nvidia_board_init(void)
{
	return 0;
}

/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init(void)
{
	__maybe_unused int err;
	__maybe_unused int board_id;

	/* Do clocks and UART first so that printf() works */
#if IS_ENABLED(CONFIG_TEGRA_CLKRST)
	clock_init();
	clock_verify();
#endif

	tegra_gpu_config();

#ifdef CONFIG_TEGRA_SPI
	pin_mux_spi();
#endif

#ifdef CONFIG_MMC_SDHCI_TEGRA
	pin_mux_mmc();
#endif

	/* Init is handled automatically in the driver-model case */
#if defined(CONFIG_DM_VIDEO)
	pin_mux_display();
#endif
	/* boot param addr */
	gd->bd->bi_boot_params = (NV_PA_SDRAM_BASE + 0x100);

	power_det_init();

#ifdef CONFIG_SYS_I2C_TEGRA
# ifdef CONFIG_TEGRA_PMU
	if (pmu_set_nominal())
		debug("Failed to select nominal voltages\n");
#  ifdef CONFIG_TEGRA_CLOCK_SCALING
	err = board_emc_init();
	if (err)
		debug("Memory controller init failed: %d\n", err);
#  endif
# endif /* CONFIG_TEGRA_PMU */
#endif /* CONFIG_SYS_I2C_TEGRA */

#ifdef CONFIG_USB_EHCI_TEGRA
	pin_mux_usb();
#endif

#if defined(CONFIG_DM_VIDEO)
	board_id = tegra_board_id();
	err = tegra_lcd_pmic_init(board_id);
	if (err) {
		debug("Failed to set up LCD PMIC\n");
		return err;
	}
#endif

#ifdef CONFIG_TEGRA_NAND
	pin_mux_nand();
#endif

	tegra_xusb_padctl_init();

#ifdef CONFIG_TEGRA_LP0
	/* save Sdram params to PMC 2, 4, and 24 for WB0 */
	warmboot_save_sdram_params();

	/* prepare the WB code to LP0 location */
	warmboot_prepare_code(TEGRA_LP0_ADDR, TEGRA_LP0_SIZE);
#endif
	return nvidia_board_init();
}

#ifdef CONFIG_BOARD_EARLY_INIT_F
static void __gpio_early_init(void)
{
}

void gpio_early_init(void) __attribute__((weak, alias("__gpio_early_init")));

int board_early_init_f(void)
{
#if IS_ENABLED(CONFIG_TEGRA_CLKRST)
	if (!clock_early_init_done())
		clock_early_init();
#endif

#if defined(CONFIG_TEGRA_DISCONNECT_UDC_ON_BOOT)
#define USBCMD_FS2 (1 << 15)
	{
		struct usb_ctlr *usbctlr = (struct usb_ctlr *)0x7d000000;
		writel(USBCMD_FS2, &usbctlr->usb_cmd);
	}
#endif

	/* Do any special system timer/TSC setup */
#if IS_ENABLED(CONFIG_TEGRA_CLKRST)
#  if defined(CONFIG_TEGRA_SUPPORT_NON_SECURE)
	if (!tegra_cpu_is_non_secure())
#  endif
		arch_timer_init();
#endif

	pinmux_init();
	board_init_uart_f();

	/* Initialize periph GPIOs */
	gpio_early_init();
	gpio_early_init_uart();

	return 0;
}
#endif	/* EARLY_INIT */

int board_late_init(void)
{
#if CONFIG_IS_ENABLED(EFI_LOADER)
	if (gd->bd->bi_dram[1].start) {
		/*
		 * Only bank 0 is below board_get_usable_ram_top(), so all of
		 * bank 1 is not mapped by the U-Boot MMU configuration, and so
		 * we must prevent EFI from using it.
		 */
		efi_add_memory_map(gd->bd->bi_dram[1].start,
				   gd->bd->bi_dram[1].size >> EFI_PAGE_SHIFT,
				   EFI_BOOT_SERVICES_DATA, false);
	}
#endif

#if defined(CONFIG_TEGRA_SUPPORT_NON_SECURE)
	if (tegra_cpu_is_non_secure()) {
		printf("CPU is in NS mode\n");
		env_set("cpu_ns_mode", "1");
	} else {
		env_set("cpu_ns_mode", "");
	}
#endif
	start_cpu_fan();
	cboot_late_init();

	return 0;
}

/*
 * In some SW environments, a memory carve-out exists to house a secure
 * monitor, a trusted OS, and/or various statically allocated media buffers.
 *
 * This carveout exists at the highest possible address that is within a
 * 32-bit physical address space.
 *
 * This function returns the total size of this carve-out. At present, the
 * returned value is hard-coded for simplicity. In the future, it may be
 * possible to determine the carve-out size:
 * - By querying some run-time information source, such as:
 *   - A structure passed to U-Boot by earlier boot software.
 *   - SoC registers.
 *   - A call into the secure monitor.
 * - In the per-board U-Boot configuration header, based on knowledge of the
 *   SW environment that U-Boot is being built for.
 *
 * For now, we support two configurations in U-Boot:
 * - 32-bit ports without any form of carve-out.
 * - 64 bit ports which are assumed to use a carve-out of a conservatively
 *   hard-coded size.
 */
static ulong carveout_size(void)
{
#ifdef CONFIG_ARM64
	return SZ_512M;
#elif defined(CONFIG_ARMV7_SECURE_RESERVE_SIZE)
	// BASE+SIZE might not == 4GB. If so, we want the carveout to cover
	// from BASE to 4GB, not BASE to BASE+SIZE.
	return (0 - CONFIG_ARMV7_SECURE_BASE) & ~(SZ_2M - 1);
#else
	return 0;
#endif
}

/*
 * Determine the amount of usable RAM below 4GiB, taking into account any
 * carve-out that may be assigned.
 */
static ulong usable_ram_size_below_4g(void)
{
	ulong total_size_below_4g;
	ulong usable_size_below_4g;

	/*
	 * The total size of RAM below 4GiB is the lesser address of:
	 * (a) 2GiB itself (RAM starts at 2GiB, and 4GiB - 2GiB == 2GiB).
	 * (b) The size RAM physically present in the system.
	 */
	if (gd->ram_size < SZ_2G)
		total_size_below_4g = gd->ram_size;
	else
		total_size_below_4g = SZ_2G;

	/* Calculate usable RAM by subtracting out any carve-out size */
	usable_size_below_4g = total_size_below_4g - carveout_size();

	return usable_size_below_4g;
}

/*
 * Represent all available RAM in either one or two banks.
 *
 * The first bank describes any usable RAM below 4GiB.
 * The second bank describes any RAM above 4GiB.
 *
 * This split is driven by the following requirements:
 * - The NVIDIA L4T kernel requires separate entries in the DT /memory/reg
 *   property for memory below and above the 4GiB boundary. The layout of that
 *   DT property is directly driven by the entries in the U-Boot bank array.
 * - The potential existence of a carve-out at the end of RAM below 4GiB can
 *   only be represented using multiple banks.
 *
 * Explicitly removing the carve-out RAM from the bank entries makes the RAM
 * layout a bit more obvious, e.g. when running "bdinfo" at the U-Boot
 * command-line.
 *
 * This does mean that the DT U-Boot passes to the Linux kernel will not
 * include this RAM in /memory/reg at all. An alternative would be to include
 * all RAM in the U-Boot banks (and hence DT), and add a /memreserve/ node
 * into DT to stop the kernel from using the RAM. IIUC, I don't /think/ the
 * Linux kernel will ever need to access any RAM in* the carve-out via a CPU
 * mapping, so either way is acceptable.
 *
 * On 32-bit systems, we never define a bank for RAM above 4GiB, since the
 * start address of that bank cannot be represented in the 32-bit .size
 * field.
 */
int dram_init_banksize(void)
{
	int err;

	/* try to compute DRAM bank size based on cboot DTB first */
	err = cboot_dram_init_banksize();
	if (err == 0)
		return err;

	/* fall back to default DRAM bank size computation */

	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = usable_ram_size_below_4g();

#ifdef CONFIG_PCI
	gd->pci_ram_top = gd->bd->bi_dram[0].start + gd->bd->bi_dram[0].size;
#endif

#ifdef CONFIG_PHYS_64BIT
	if (gd->ram_size > SZ_2G) {
		gd->bd->bi_dram[1].start = 0x100000000;
		gd->bd->bi_dram[1].size = gd->ram_size - SZ_2G;
	} else
#endif
	{
		gd->bd->bi_dram[1].start = 0;
		gd->bd->bi_dram[1].size = 0;
	}

	return 0;
}

/*
 * Most hardware on 64-bit Tegra is still restricted to DMA to the lower
 * 32-bits of the physical address space. Cap the maximum usable RAM area
 * at 4 GiB to avoid DMA buffers from being allocated beyond the 32-bit
 * boundary that most devices can address. Also, don't let U-Boot use any
 * carve-out, as mentioned above.
 *
 * This function is called before dram_init_banksize(), so we can't simply
 * return gd->bd->bi_dram[1].start + gd->bd->bi_dram[1].size.
 */
ulong board_get_usable_ram_top(ulong total_size)
{
	ulong ram_top;

	/* try to get top of usable RAM based on cboot DTB first */
	ram_top = cboot_get_usable_ram_top(total_size);
	if (ram_top > 0)
		return ram_top;

	/* fall back to default usable RAM computation */

	return CONFIG_SYS_SDRAM_BASE + usable_ram_size_below_4g();
}
