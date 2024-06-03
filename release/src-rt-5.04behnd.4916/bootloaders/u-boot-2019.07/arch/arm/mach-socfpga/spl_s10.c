// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2016-2018 Intel Corporation <www.intel.com>
 *
 */

#include <asm/io.h>
#include <asm/u-boot.h>
#include <asm/utils.h>
#include <common.h>
#include <debug_uart.h>
#include <image.h>
#include <spl.h>
#include <asm/arch/clock_manager.h>
#include <asm/arch/firewall_s10.h>
#include <asm/arch/mailbox_s10.h>
#include <asm/arch/reset_manager.h>
#include <asm/arch/system_manager.h>
#include <watchdog.h>
#include <dm/uclass.h>

DECLARE_GLOBAL_DATA_PTR;

static struct socfpga_system_manager *sysmgr_regs =
	(struct socfpga_system_manager *)SOCFPGA_SYSMGR_ADDRESS;

u32 spl_boot_device(void)
{
	/* TODO: Get from SDM or handoff */
	return BOOT_DEVICE_MMC1;
}

#ifdef CONFIG_SPL_MMC_SUPPORT
u32 spl_boot_mode(const u32 boot_device)
{
#if defined(CONFIG_SPL_FS_FAT) || defined(CONFIG_SPL_FS_EXT4)
	return MMCSD_MODE_FS;
#else
	return MMCSD_MODE_RAW;
#endif
}
#endif

void spl_disable_firewall_l4_per(void)
{
	const struct socfpga_firwall_l4_per *firwall_l4_per_base =
		(struct socfpga_firwall_l4_per *)SOCFPGA_FIREWALL_L4_PER;
	u32 i;
	const u32 *addr[] = {
			&firwall_l4_per_base->nand,
			&firwall_l4_per_base->nand_data,
			&firwall_l4_per_base->usb0,
			&firwall_l4_per_base->usb1,
			&firwall_l4_per_base->spim0,
			&firwall_l4_per_base->spim1,
			&firwall_l4_per_base->emac0,
			&firwall_l4_per_base->emac1,
			&firwall_l4_per_base->emac2,
			&firwall_l4_per_base->sdmmc,
			&firwall_l4_per_base->gpio0,
			&firwall_l4_per_base->gpio1,
			&firwall_l4_per_base->i2c0,
			&firwall_l4_per_base->i2c1,
			&firwall_l4_per_base->i2c2,
			&firwall_l4_per_base->i2c3,
			&firwall_l4_per_base->i2c4,
			&firwall_l4_per_base->timer0,
			&firwall_l4_per_base->timer1,
			&firwall_l4_per_base->uart0,
			&firwall_l4_per_base->uart1
			};

	/*
	 * The following lines of code will enable non-secure access
	 * to nand, usb, spi, emac, sdmmc, gpio, i2c, timers and uart. This
	 * is needed as most OS run in non-secure mode. Thus we need to
	 * enable non-secure access to these peripherals in order for the
	 * OS to use these peripherals.
	 */
	for (i = 0; i < ARRAY_SIZE(addr); i++)
		writel(FIREWALL_L4_DISABLE_ALL, addr[i]);
}

void spl_disable_firewall_l4_sys(void)
{
	const struct socfpga_firwall_l4_sys *firwall_l4_sys_base =
		(struct socfpga_firwall_l4_sys *)SOCFPGA_FIREWALL_L4_SYS;
	u32 i;
	const u32 *addr[] = {
			&firwall_l4_sys_base->dma_ecc,
			&firwall_l4_sys_base->emac0rx_ecc,
			&firwall_l4_sys_base->emac0tx_ecc,
			&firwall_l4_sys_base->emac1rx_ecc,
			&firwall_l4_sys_base->emac1tx_ecc,
			&firwall_l4_sys_base->emac2rx_ecc,
			&firwall_l4_sys_base->emac2tx_ecc,
			&firwall_l4_sys_base->nand_ecc,
			&firwall_l4_sys_base->nand_read_ecc,
			&firwall_l4_sys_base->nand_write_ecc,
			&firwall_l4_sys_base->ocram_ecc,
			&firwall_l4_sys_base->sdmmc_ecc,
			&firwall_l4_sys_base->usb0_ecc,
			&firwall_l4_sys_base->usb1_ecc,
			&firwall_l4_sys_base->clock_manager,
			&firwall_l4_sys_base->io_manager,
			&firwall_l4_sys_base->reset_manager,
			&firwall_l4_sys_base->system_manager,
			&firwall_l4_sys_base->watchdog0,
			&firwall_l4_sys_base->watchdog1,
			&firwall_l4_sys_base->watchdog2,
			&firwall_l4_sys_base->watchdog3
		};

	for (i = 0; i < ARRAY_SIZE(addr); i++)
		writel(FIREWALL_L4_DISABLE_ALL, addr[i]);
}

void board_init_f(ulong dummy)
{
	const struct cm_config *cm_default_cfg = cm_get_default_config();
	int ret;

#ifdef CONFIG_HW_WATCHDOG
	/* Ensure watchdog is paused when debugging is happening */
	writel(SYSMGR_WDDBG_PAUSE_ALL_CPU, &sysmgr_regs->wddbg);

	/* Enable watchdog before initializing the HW */
	socfpga_per_reset(SOCFPGA_RESET(L4WD0), 1);
	socfpga_per_reset(SOCFPGA_RESET(L4WD0), 0);
	hw_watchdog_init();
#endif

	/* ensure all processors are not released prior Linux boot */
	writeq(0, CPU_RELEASE_ADDR);

	socfpga_per_reset(SOCFPGA_RESET(OSC1TIMER0), 0);
	timer_init();

	sysmgr_pinmux_init();

	/* configuring the HPS clocks */
	cm_basic_init(cm_default_cfg);

#ifdef CONFIG_DEBUG_UART
	socfpga_per_reset(SOCFPGA_RESET(UART0), 0);
	debug_uart_init();
#endif
	ret = spl_early_init();
	if (ret) {
		debug("spl_early_init() failed: %d\n", ret);
		hang();
	}

	preloader_console_init();
	cm_print_clock_quick_summary();

	/* enable non-secure interface to DMA330 DMA and peripherals */
	writel(SYSMGR_DMA_IRQ_NS | SYSMGR_DMA_MGR_NS, &sysmgr_regs->dma);
	writel(SYSMGR_DMAPERIPH_ALL_NS, &sysmgr_regs->dma_periph);

	spl_disable_firewall_l4_per();

	spl_disable_firewall_l4_sys();

	/* disable lwsocf2fpga and soc2fpga bridge security */
	writel(FIREWALL_BRIDGE_DISABLE_ALL, SOCFPGA_FIREWALL_SOC2FPGA);
	writel(FIREWALL_BRIDGE_DISABLE_ALL, SOCFPGA_FIREWALL_LWSOC2FPGA);

	/* disable SMMU security */
	writel(FIREWALL_L4_DISABLE_ALL, SOCFPGA_FIREWALL_TCU);

	/* disable ocram security at CCU for non secure access */
	clrbits_le32(CCU_REG_ADDR(CCU_CPU0_MPRT_ADMASK_MEM_RAM0),
		     CCU_ADMASK_P_MASK | CCU_ADMASK_NS_MASK);
	clrbits_le32(CCU_REG_ADDR(CCU_IOM_MPRT_ADMASK_MEM_RAM0),
		     CCU_ADMASK_P_MASK | CCU_ADMASK_NS_MASK);

#if CONFIG_IS_ENABLED(ALTERA_SDRAM)
		struct udevice *dev;

		ret = uclass_get_device(UCLASS_RAM, 0, &dev);
		if (ret) {
			debug("DRAM init failed: %d\n", ret);
			hang();
		}
#endif

	mbox_init();

#ifdef CONFIG_CADENCE_QSPI
	mbox_qspi_open();
#endif
}
