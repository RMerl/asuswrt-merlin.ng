// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 Marek Vasut <marex@denx.de>
 */

#include <common.h>
#include <asm-generic/gpio.h>
#include <asm-generic/sections.h>
#include <asm/arch/clock.h>
#include <asm/arch/ddr.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/iomux-v3.h>
#include <dm/uclass.h>
#include <hang.h>
#include <i2c_eeprom.h>
#include <image.h>
#include <init.h>
#include <net.h>
#include <spl.h>

#include <dm/uclass.h>
#include <dm/device.h>
#include <dm/uclass-internal.h>
#include <dm/device-internal.h>

DECLARE_GLOBAL_DATA_PTR;

#define WDOG_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_ODE | PAD_CTL_PUE | PAD_CTL_PE)

#define DDRC_ECCCFG0_ECC_MODE_MASK	0x7

u8 dmo_get_memcfg(void)
{
	struct gpio_desc gpio[4];
	u8 memcfg = 0;
	ofnode node;
	int i, ret;

	node = ofnode_path("/config");
	if (!ofnode_valid(node)) {
		printf("%s: no /config node?\n", __func__);
		return BIT(2) | BIT(0);
	}

	ret = gpio_request_list_by_name_nodev(node,
					      "dmo,ram-coding-gpios",
					      gpio, ARRAY_SIZE(gpio),
					      GPIOD_IS_IN);
	for (i = 0; i < ret; i++)
		memcfg |= !!dm_gpio_get_value(&(gpio[i])) << i;

	gpio_free_list_nodev(gpio, ret);

	return memcfg;
}

int board_phys_sdram_size(phys_size_t *size)
{
	u8 memcfg = dmo_get_memcfg();
	u8 ecc = 0;

	*size = 4ULL >> ((memcfg >> 1) & 0x3);

	if (IS_ENABLED(CONFIG_IMX8M_DRAM_INLINE_ECC)) {
		/* 896 MiB, i.e. 1 GiB without 12.5% reserved for in-band ECC */
		ecc = readl(DDRC_ECCCFG0(0)) & DDRC_ECCCFG0_ECC_MODE_MASK;
	}

	*size *= SZ_1G - (ecc ? (SZ_1G / 8) : 0);

	return 0;
}

#ifdef CONFIG_SPL_BUILD
static void data_modul_imx_edm_sbc_early_init_f(const iomux_v3_cfg_t wdog_pad)
{
	struct wdog_regs *wdog = (struct wdog_regs *)WDOG1_BASE_ADDR;

	imx_iomux_v3_setup_pad(wdog_pad | MUX_PAD_CTRL(WDOG_PAD_CTRL));

	set_wdog_reset(wdog);
}

__weak int data_modul_imx_edm_sbc_board_power_init(void)
{
	return 0;
}

static void spl_dram_init(struct dram_timing_info *dram_timing_info[8])
{
	u8 memcfg = dmo_get_memcfg();
	int i;

	printf("DDR:   %d GiB x%d [0x%x]\n",
	       /* 0..4 GiB, 1..2 GiB, 0..1 GiB */
	       4 >> ((memcfg >> 1) & 0x3),
	       /* 0..x32, 1..x16 */
	       32 >> (memcfg & BIT(0)),
	       memcfg);

	if (!dram_timing_info[memcfg]) {
		printf("Unsupported DRAM strapping, trying lowest supported. MEMCFG=0x%x\n",
		       memcfg);
		for (i = 7; i >= 0; i--)
			if (dram_timing_info[i])	/* Configuration found */
				break;
	}

	ddr_init(dram_timing_info[memcfg]);

	if (IS_ENABLED(CONFIG_IMX8M_DRAM_INLINE_ECC)) {
		printf("DDR:   Inline ECC %sabled\n",
		       (readl(DDRC_ECCCFG0(0)) & DDRC_ECCCFG0_ECC_MODE_MASK) ?
		       "en" : "dis");
	}
}

void dmo_board_init_f(const iomux_v3_cfg_t wdog_pad,
		      struct dram_timing_info *dram_timing_info[8])
{
	struct udevice *dev;
	int ret;

	icache_enable();

	arch_cpu_init();

	init_uart_clk(2);

	data_modul_imx_edm_sbc_early_init_f(wdog_pad);

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	ret = spl_early_init();
	if (ret) {
		debug("spl_early_init() failed: %d\n", ret);
		hang();
	}

	preloader_console_init();

	ret = uclass_get_device_by_name(UCLASS_CLK,
					"clock-controller@30380000",
					&dev);
	if (ret < 0) {
		printf("Failed to find clock node. Check device tree\n");
		hang();
	}

	enable_tzc380();

	data_modul_imx_edm_sbc_board_power_init();

	/* DDR initialization */
	spl_dram_init(dram_timing_info);

	board_init_r(NULL, 0);
}
#else
void dmo_setup_boot_device(void)
{
	int boot_device = get_boot_device();
	char *devnum;

	devnum = env_get("devnum");
	if (devnum)	/* devnum is already set */
		return;

	if (boot_device == MMC3_BOOT)	/* eMMC */
		env_set_ulong("devnum", 0);
	else
		env_set_ulong("devnum", 1);
}

void dmo_setup_mac_address(void)
{
	unsigned char enetaddr[6];
	struct udevice *dev;
	int off, ret;

	ret = eth_env_get_enetaddr("ethaddr", enetaddr);
	if (ret)	/* ethaddr is already set */
		return;

	off = fdt_path_offset(gd->fdt_blob, "eeprom0");
	if (off < 0) {
		printf("%s: No eeprom0 path offset\n", __func__);
		return;
	}

	ret = uclass_get_device_by_of_offset(UCLASS_I2C_EEPROM, off, &dev);
	if (ret) {
		printf("Cannot find EEPROM!\n");
		return;
	}

	ret = i2c_eeprom_read(dev, 0xb0, enetaddr, 0x6);
	if (ret) {
		printf("Error reading configuration EEPROM!\n");
		return;
	}

	if (is_valid_ethaddr(enetaddr))
		eth_env_set_enetaddr("ethaddr", enetaddr);
}
#endif
