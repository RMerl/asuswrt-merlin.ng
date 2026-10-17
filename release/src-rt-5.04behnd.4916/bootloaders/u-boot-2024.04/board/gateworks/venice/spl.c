// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 Gateworks Corporation
 */

#include <common.h>
#include <cpu_func.h>
#include <hang.h>
#include <i2c.h>
#include <init.h>
#include <spl.h>
#include <asm/mach-imx/gpio.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx8mm_pins.h>
#include <asm/arch/imx8mn_pins.h>
#include <asm/arch/imx8mp_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/arch/ddr.h>
#include <asm-generic/gpio.h>
#include <asm/sections.h>
#include <dm/uclass.h>
#include <dm/device.h>
#include <dm/pinctrl.h>
#include <linux/delay.h>
#include <power/bd71837.h>
#include <power/mp5416.h>
#include <power/pca9450.h>

#include "eeprom.h"
#include "lpddr4_timing.h"

#define PCIE_RSTN IMX_GPIO_NR(4, 6)

static void spl_dram_init(int size)
{
	struct dram_timing_info *dram_timing;

	switch (size) {
#ifdef CONFIG_IMX8MM
	case 512:
		dram_timing = &dram_timing_512mb;
		break;
	case 1024:
		dram_timing = &dram_timing_1gb;
		break;
	case 2048:
		dram_timing = &dram_timing_2gb;
		break;
	case 4096:
		dram_timing = &dram_timing_4gb;
		break;
	default:
		printf("Unknown DDR configuration: %d MiB\n", size);
		dram_timing = &dram_timing_1gb;
		size = 1024;
#elif CONFIG_IMX8MN
	case 1024:
		dram_timing = &dram_timing_1gb_single_die;
		break;
	case 2048:
		if (!strcmp(eeprom_get_model(), "GW7902-SP466-A") ||
		    !strcmp(eeprom_get_model(), "GW7902-SP466-B")) {
			dram_timing = &dram_timing_2gb_dual_die;
		} else {
			dram_timing = &dram_timing_2gb_single_die;
		}
		break;
	default:
		printf("Unknown DDR configuration: %d MiB\n", size);
		dram_timing = &dram_timing_2gb_dual_die;
		size = 2048;
#elif CONFIG_IMX8MP
	case 1024:
		dram_timing = &dram_timing_1gb_single_die;
		break;
	case 4096:
		dram_timing = &dram_timing_4gb_dual_die;
		break;
	default:
		printf("Unknown DDR configuration: %d GiB\n", size);
		dram_timing = &dram_timing_4gb_dual_die;
		size = 4096;
#endif
	}

	printf("DRAM    : LPDDR4 ");
	if (size > 512)
		printf("%d GiB", size / 1024);
	else
		printf("%d MiB", size);
	printf(" %dMT/s %dMHz\n",
	       dram_timing->fsp_msg[0].drate,
	       dram_timing->fsp_msg[0].drate / 2);
	ddr_init(dram_timing);
}

/*
 * Model specific PMIC adjustments necessary prior to DRAM init
 *
 * Note that we can not use pmic dm drivers here as we have a generic
 * venice dt that does not have board-specific pmic's defined.
 *
 * Instead we must use dm_i2c so we a helpers to give us
 * clrsetbit functions we would otherwise have if we could use PMIC dm
 * drivers.
 */
static int dm_i2c_clrsetbits(struct udevice *dev, uint reg, uint clr, uint set)
{
	int ret;
	u8 val;

	ret = dm_i2c_read(dev, reg, &val, 1);
	if (ret)
		return ret;
	val = (val & ~clr) | set;

	return dm_i2c_write(dev, reg, &val, 1);
}

static int power_init_board(void)
{
	const char *model = eeprom_get_model();
	struct udevice *bus;
	struct udevice *dev;
	int ret;

	if ((!strncmp(model, "GW71", 4)) ||
	    (!strncmp(model, "GW72", 4)) ||
	    (!strncmp(model, "GW73", 4)) ||
	    (!strncmp(model, "GW7905", 6))) {
		ret = uclass_get_device_by_seq(UCLASS_I2C, 0, &bus);
		if (ret) {
			printf("PMIC    : failed I2C1 probe: %d\n", ret);
			return ret;
		}
		ret = dm_i2c_probe(bus, 0x69, 0, &dev);
		if (ret) {
			printf("PMIC    : failed probe: %d\n", ret);
			return ret;
		}
#ifdef CONFIG_IMX8MM
		puts("PMIC    : MP5416 (IMX8MM)\n");

		/* set VDD_ARM SW3 to 0.92V for 1.6GHz */
		dm_i2c_reg_write(dev, MP5416_VSET_SW3,
				 BIT(7) | MP5416_VSET_SW3_SVAL(920000));
#elif CONFIG_IMX8MP
		puts("PMIC    : MP5416 (IMX8MP)\n");

		/* set VDD_ARM SW3 to 0.95V for 1.6GHz */
		dm_i2c_reg_write(dev, MP5416_VSET_SW3,
				 BIT(7) | MP5416_VSET_SW3_SVAL(950000));
		/* set VDD_SOC SW1 to 0.95V for 1.6GHz */
		dm_i2c_reg_write(dev, MP5416_VSET_SW1,
				 BIT(7) | MP5416_VSET_SW1_SVAL(950000));
#endif
	}

	else if (!strncmp(model, "GW74", 4)) {
		ret = uclass_get_device_by_seq(UCLASS_I2C, 2, &bus);
		if (ret) {
			printf("PMIC    : failed I2C3 probe: %d\n", ret);
			return ret;
		}
		ret = dm_i2c_probe(bus, 0x25, 0, &dev);
		if (ret) {
			printf("PMIC    : failed probe: %d\n", ret);
			return ret;
		}
		puts("PMIC    : PCA9450\n");

		/* BUCKxOUT_DVS0/1 control BUCK123 output */
		dm_i2c_reg_write(dev, PCA9450_BUCK123_DVS, 0x29);

		/* Buck 1 DVS control through PMIC_STBY_REQ */
		dm_i2c_reg_write(dev, PCA9450_BUCK1CTRL, 0x59);

		/* Set DVS1 to 0.85v for suspend */
		dm_i2c_reg_write(dev, PCA9450_BUCK1OUT_DVS1, 0x14);

		/* increase VDD_SOC to 0.95V before first DRAM access */
		dm_i2c_reg_write(dev, PCA9450_BUCK1OUT_DVS0, 0x1C);

		/* Kernel uses OD/OD freq for SOC */
		/* To avoid timing risk from SOC to ARM, increase VDD_ARM to OD voltage 0.95v */
		dm_i2c_reg_write(dev, PCA9450_BUCK2OUT_DVS0, 0x1C);
	}

	else if ((!strncmp(model, "GW7901", 6)) ||
		 (!strncmp(model, "GW7902", 6)) ||
		 (!strncmp(model, "GW7903", 6)) ||
		 (!strncmp(model, "GW7904", 6))) {
		if (!strncmp(model, "GW7902", 6))
			ret = uclass_get_device_by_seq(UCLASS_I2C, 0, &bus);
		else
			ret = uclass_get_device_by_seq(UCLASS_I2C, 1, &bus);
		if (ret) {
			printf("PMIC    : failed I2C2 probe: %d\n", ret);
			return ret;
		}
		ret = dm_i2c_probe(bus, 0x4b, 0, &dev);
		if (ret) {
			printf("PMIC    : failed probe: %d\n", ret);
			return ret;
		}
		puts("PMIC    : BD71847\n");

		/* unlock the PMIC regs */
		dm_i2c_reg_write(dev, BD718XX_REGLOCK, 0x1);

		/* set switchers to forced PWM mode */
		dm_i2c_clrsetbits(dev, BD718XX_BUCK1_CTRL, 0, 0x8);
		dm_i2c_clrsetbits(dev, BD718XX_BUCK2_CTRL, 0, 0x8);
		dm_i2c_clrsetbits(dev, BD718XX_1ST_NODVS_BUCK_CTRL, 0, 0x8);
		dm_i2c_clrsetbits(dev, BD718XX_2ND_NODVS_BUCK_CTRL, 0, 0x8);
		dm_i2c_clrsetbits(dev, BD718XX_3RD_NODVS_BUCK_CTRL, 0, 0x8);
		dm_i2c_clrsetbits(dev, BD718XX_4TH_NODVS_BUCK_CTRL, 0, 0x8);

		/* increase VDD_0P95 (VDD_GPU/VPU/DRAM) to 0.975v for 1.5Ghz DDR */
		dm_i2c_reg_write(dev, BD718XX_1ST_NODVS_BUCK_VOLT, 0x83);

		/* increase VDD_SOC to 0.85v before first DRAM access */
		dm_i2c_reg_write(dev, BD718XX_BUCK1_VOLT_RUN, 0x0f);

		/* increase VDD_ARM to 0.92v for 800 and 1600Mhz */
		dm_i2c_reg_write(dev, BD718XX_BUCK2_VOLT_RUN, 0x16);

		/* Lock the PMIC regs */
		dm_i2c_reg_write(dev, BD718XX_REGLOCK, 0x11);
	}

	return 0;
}

void board_init_f(ulong dummy)
{
	struct udevice *bus, *dev;
	int i, ret;
	int dram_sz;

	arch_cpu_init();

	init_uart_clk(1);

	timer_init();

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	ret = spl_early_init();
	if (ret) {
		debug("spl_early_init() failed: %d\n", ret);
		hang();
	}

	preloader_console_init();

	enable_tzc380();

	/* need to hold PCIe switch in reset otherwise it can lock i2c bus EEPROM is on */
	gpio_request(PCIE_RSTN, "perst#");
	gpio_direction_output(PCIE_RSTN, 0);

	/*
	 * probe GSC device
	 *
	 * On a board with a missing/depleted backup battery for GSC, the
	 * board may be ready to probe the GSC before its firmware is
	 * running. Wait here for 50ms for the GSC firmware to let go of
	 * the SCL/SDA lines to avoid the i2c driver spamming
	 * 'Arbitration lost' I2C errors
	 */
	if (!uclass_get_device_by_seq(UCLASS_I2C, 0, &bus)) {
		if (!pinctrl_select_state(bus, "gpio")) {
			struct mxc_i2c_bus *i2c_bus = dev_get_priv(bus);
			struct gpio_desc *scl_gpio = &i2c_bus->scl_gpio;
			struct gpio_desc *sda_gpio = &i2c_bus->sda_gpio;

			dm_gpio_set_dir_flags(scl_gpio, GPIOD_IS_IN);
			dm_gpio_set_dir_flags(sda_gpio, GPIOD_IS_IN);
			for (i = 0; i < 5; i++) {
				if (dm_gpio_get_value(scl_gpio) &&
				    dm_gpio_get_value(sda_gpio))
					break;
				mdelay(10);
			}
			pinctrl_select_state(bus, "default");
		}
	}
	/* Wait indefiniately until the GSC probes */
	while (1) {
		if (!uclass_get_device_by_driver(UCLASS_MISC, DM_DRIVER_GET(gsc), &dev))
			break;
		mdelay(1);
	}
	dram_sz = venice_eeprom_init(0);

	/* PMIC */
	power_init_board();

	/* DDR initialization */
	spl_dram_init(dram_sz);

	board_init_r(NULL, 0);
}

/* determine prioritized order of boot devices to load U-Boot from */
void board_boot_order(u32 *spl_boot_list)
{
	int i = 0;

	/*
	 * If the SPL was loaded via serial loader, we try to get
	 * U-Boot proper via USB SDP.
	 */
	if (spl_boot_device() == BOOT_DEVICE_BOARD) {
#ifdef CONFIG_IMX8MM
		spl_boot_list[i++] = BOOT_DEVICE_BOARD;
#else
		spl_boot_list[i++] = BOOT_DEVICE_BOOTROM;
#endif
	}

	/* we have only eMMC in default venice dt */
	spl_boot_list[i++] = BOOT_DEVICE_MMC1;
}

/* return boot device based on where the SPL was loaded from */
int spl_board_boot_device(enum boot_device boot_dev_spl)
{
	switch (boot_dev_spl) {
	case USB_BOOT:
		return BOOT_DEVICE_BOARD;
	/* SDHC2 */
	case SD2_BOOT:
	case MMC2_BOOT:
		return BOOT_DEVICE_MMC1;
	/* SDHC3 */
	case SD3_BOOT:
	case MMC3_BOOT:
		return BOOT_DEVICE_MMC2;
	default:
		return BOOT_DEVICE_NONE;
	}
}

unsigned long board_spl_mmc_get_uboot_raw_sector(struct mmc *mmc, unsigned long raw_sect)
{
	if (!IS_SD(mmc)) {
		switch (EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config)) {
		case 1:
		case 2:
			if (IS_ENABLED(CONFIG_IMX8MN) || IS_ENABLED(CONFIG_IMX8MP))
				raw_sect -= 32 * 2;
			break;
		}
	}

	return raw_sect;
}

const char *spl_board_loader_name(u32 boot_device)
{
	switch (boot_device) {
	/* SDHC2 */
	case BOOT_DEVICE_MMC1:
		return "eMMC";
	/* SDHC3 */
	case BOOT_DEVICE_MMC2:
		return "SD card";
	default:
		return NULL;
	}
}

void spl_board_init(void)
{
	arch_misc_init();
}
