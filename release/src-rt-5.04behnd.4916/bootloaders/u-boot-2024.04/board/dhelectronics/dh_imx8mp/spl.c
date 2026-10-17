// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 Marek Vasut <marex@denx.de>
 */

#include <common.h>
#include <hang.h>
#include <image.h>
#include <init.h>
#include <spl.h>
#include <asm/io.h>
#include <asm-generic/gpio.h>
#include <asm/arch/clock.h>
#include <asm/arch/ddr.h>
#include <asm/arch/imx8mp_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/arch/ddr.h>
#include <asm/sections.h>

#include <dm/uclass.h>
#include <dm/device.h>
#include <dm/uclass-internal.h>
#include <dm/device-internal.h>

#include <linux/bitfield.h>

#include <power/pmic.h>
#include <power/pca9450.h>

#include "lpddr4_timing.h"

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_FSEL1)
#define WDOG_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_ODE | PAD_CTL_PUE | PAD_CTL_PE)

static const iomux_v3_cfg_t uart_pads[] = {
	MX8MP_PAD_SAI2_RXFS__UART1_DCE_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX8MP_PAD_SAI2_RXC__UART1_DCE_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static const iomux_v3_cfg_t wdog_pads[] = {
	MX8MP_PAD_GPIO1_IO02__WDOG1_WDOG_B  | MUX_PAD_CTRL(WDOG_PAD_CTRL),
};

static bool dh_gigabit_eqos, dh_gigabit_fec;
static u8 dh_som_rev;

static void dh_imx8mp_early_init_f(void)
{
	struct wdog_regs *wdog = (struct wdog_regs *)WDOG1_BASE_ADDR;

	imx_iomux_v3_setup_multiple_pads(wdog_pads, ARRAY_SIZE(wdog_pads));

	set_wdog_reset(wdog);

	imx_iomux_v3_setup_multiple_pads(uart_pads, ARRAY_SIZE(uart_pads));
}

static int dh_imx8mp_board_power_init(void)
{
	struct udevice *dev;
	int ret;

	ret = pmic_get("pmic@25", &dev);
	if (ret == -ENODEV) {
		puts("Failed to get PMIC\n");
		return 0;
	}
	if (ret != 0)
		return ret;

	/* BUCKxOUT_DVS0/1 control BUCK123 output. */
	pmic_reg_write(dev, PCA9450_BUCK123_DVS, 0x29);

	/* Increase VDD_SOC to typical value 0.95V before first DRAM access. */
	if (IS_ENABLED(CONFIG_IMX8M_VDD_SOC_850MV))
		/* Set DVS0 to 0.85V for special case. */
		pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS0, 0x14);
	else
		pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS0, 0x1c);

	/* Set DVS1 to 0.85v for suspend. */
	pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS1, 0x14);

	/*
	 * Enable DVS control through PMIC_STBY_REQ and
	 * set B1_ENMODE=1 (ON by PMIC_ON_REQ=H).
	 */
	pmic_reg_write(dev, PCA9450_BUCK1CTRL, 0x59);

	/* Kernel uses OD/OD frequency for SoC. */

	/* To avoid timing risk from SoC to ARM, increase VDD_ARM to OD voltage 0.95V */
	pmic_reg_write(dev, PCA9450_BUCK2OUT_DVS0, 0x1c);

	/* DRAM Vdd1 always FPWM */
	pmic_reg_write(dev, PCA9450_BUCK5CTRL, 0x0d);
	/* DRAM Vdd2/Vddq always FPWM */
	pmic_reg_write(dev, PCA9450_BUCK6CTRL, 0x0d);

	/* Set LDO4 and CONFIG2 to enable the I2C level translator. */
	pmic_reg_write(dev, PCA9450_LDO4CTRL, 0x59);
	pmic_reg_write(dev, PCA9450_CONFIG2, 0x1);

	return 0;
}

static struct dram_timing_info *dram_timing_info[8] = {
	NULL,					/* 512 MiB */
	NULL,					/* 1024 MiB */
	NULL,					/* 1536 MiB */
	&dh_imx8mp_dhcom_dram_timing_16g_x32,	/* 2048 MiB */
	NULL,					/* 3072 MiB */
	&dh_imx8mp_dhcom_dram_timing_32g_x32,	/* 4096 MiB */
	NULL,					/* 6144 MiB */
	NULL,					/* 8192 MiB */
};

static void spl_dram_init(void)
{
	const u16 size[] = { 512, 1024, 1536, 2048, 3072, 4096, 6144, 8192 };
	u8 memcfg = dh_get_memcfg();
	int i;

	printf("DDR:   %d MiB [0x%x]\n", size[memcfg], memcfg);

	if (!dram_timing_info[memcfg]) {
		printf("Unsupported DRAM strapping, trying lowest supported. MEMCFG=0x%x\n",
		       memcfg);
		for (i = 0; i < ARRAY_SIZE(dram_timing_info); i++)
			if (dram_timing_info[i])	/* Configuration found */
				break;
	}

	ddr_init(dram_timing_info[memcfg]);

	printf("DDR:   Inline ECC %sabled\n",
	       (readl(DDRC_ECCCFG0(0)) & DDRC_ECCCFG0_ECC_MODE_MASK) ?
	       "en" : "dis");
}

#if IS_ENABLED(CONFIG_IMX8M_DRAM_INLINE_ECC)
static const scrub_func_t dram_scrub_fn[8] = {
	NULL,					/* 512 MiB */
	NULL,					/* 1024 MiB */
	NULL,					/* 1536 MiB */
	dh_imx8mp_dhcom_dram_scrub_16g_x32,	/* 2048 MiB */
	NULL,					/* 3072 MiB */
	dh_imx8mp_dhcom_dram_scrub_32g_x32,	/* 4096 MiB */
	NULL,					/* 6144 MiB */
	NULL,					/* 8192 MiB */
};

void board_dram_ecc_scrub(void)
{
	u8 memcfg = dh_get_memcfg();

	if (!dram_scrub_fn[memcfg])
		return;

	dram_scrub_fn[memcfg]();
}
#endif

void spl_board_init(void)
{
	/*
	 * Set GIC clock to 500 MHz for OD VDD_SOC. Kernel driver does not
	 * allow to change it. Should set the clock after PMIC setting done.
	 * Default is 400 MHz (system_pll1_800m with div = 2) set by ROM for
	 * ND VDD_SOC.
	 */
	clock_enable(CCGR_GIC, 0);
	clock_set_target_val(GIC_CLK_ROOT, CLK_ROOT_ON | CLK_ROOT_SOURCE_SEL(5));
	clock_enable(CCGR_GIC, 1);
}

int spl_board_boot_device(enum boot_device boot_dev_spl)
{
	return BOOT_DEVICE_BOOTROM;
}

int board_spl_fit_append_fdt_skip(const char *name)
{
	if (!dh_gigabit_eqos) {		/* 1x or 2x RMII PHY SoM */
		if (dh_gigabit_fec) {	/* 1x RMII PHY SoM */
			if (!strcmp(name, "fdt-dto-imx8mp-dhcom-som-overlay-eth1xfast"))
				return 0;
		} else {		/* 2x RMII PHY SoM */
			if (!strcmp(name, "fdt-dto-imx8mp-dhcom-som-overlay-eth2xfast"))
				return 0;
			if (!strcmp(name, "fdt-dto-imx8mp-dhcom-pdk-overlay-eth2xfast")) {
				/* 2x RMII PHY SoM on PDK2 or PDK3 */
				if (of_machine_is_compatible("dh,imx8mp-dhcom-pdk2") ||
				    of_machine_is_compatible("dh,imx8mp-dhcom-pdk3"))
					return 0;
			}
		}
	}

	if (dh_som_rev == 0x0) { /* Prototype SoM rev.100 */
		if (!strcmp(name, "fdt-dto-imx8mp-dhcom-som-overlay-rev100"))
			return 0;

		if (!strcmp(name, "fdt-dto-imx8mp-dhcom-pdk3-overlay-rev100") &&
		    of_machine_is_compatible("dh,imx8mp-dhcom-pdk3"))
			return 0;
	}

	return 1;	/* Skip this DTO */
}

static void dh_imx8mp_board_cache_config(void)
{
	const void __iomem *mux_base = (void __iomem *)IOMUXC_BASE_ADDR;
	const u32 mux_sion[] = {
		FIELD_GET(MUX_CTRL_OFS_MASK, MX8MP_PAD_ENET_RX_CTL__GPIO1_IO24),
		FIELD_GET(MUX_CTRL_OFS_MASK, MX8MP_PAD_SAI1_TXFS__GPIO4_IO10),
		FIELD_GET(MUX_CTRL_OFS_MASK, MX8MP_PAD_NAND_DQS__GPIO3_IO14),
		FIELD_GET(MUX_CTRL_OFS_MASK, MX8MP_PAD_SAI1_TXD7__GPIO4_IO19),
		FIELD_GET(MUX_CTRL_OFS_MASK, MX8MP_PAD_SAI5_MCLK__GPIO3_IO25),
	};
	int i;

	for (i = 0; i < ARRAY_SIZE(mux_sion); i++)
		setbits_le32(mux_base + mux_sion[i], IOMUX_CONFIG_SION);

	dh_gigabit_eqos = !(readl(GPIO1_BASE_ADDR) & BIT(24));
	dh_gigabit_fec = !(readl(GPIO4_BASE_ADDR) & BIT(10));
	dh_som_rev = !!(readl(GPIO3_BASE_ADDR) & BIT(14));
	dh_som_rev |= !!(readl(GPIO4_BASE_ADDR) & BIT(19)) << 1;
	dh_som_rev |= !!(readl(GPIO3_BASE_ADDR) & BIT(25)) << 2;

	for (i = 0; i < ARRAY_SIZE(mux_sion); i++)
		clrbits_le32(mux_base + mux_sion[i], IOMUX_CONFIG_SION);
}

void board_init_f(ulong dummy)
{
	struct udevice *dev;
	int ret;

	arch_cpu_init();

	init_uart_clk(0);

	dh_imx8mp_early_init_f();

	preloader_console_init();

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	ret = spl_early_init();
	if (ret) {
		debug("spl_early_init() failed: %d\n", ret);
		hang();
	}

	ret = uclass_get_device_by_name(UCLASS_CLK,
					"clock-controller@30380000",
					&dev);
	if (ret < 0) {
		printf("Failed to find clock node. Check device tree\n");
		hang();
	}

	enable_tzc380();

	dh_imx8mp_board_power_init();

	/* DDR initialization */
	spl_dram_init();

	dh_imx8mp_board_cache_config();

	board_init_r(NULL, 0);
}
