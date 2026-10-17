// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 * Author: Fabio Estevam <fabio.estevam@freescale.com>
 *
 * Copyright (C) 2013, 2014 TQ-Systems (ported SabreSD to TQMa6x)
 * Author: Markus Niebel <markus.niebel@tq-group.com>
 */

#include <init.h>
#include <asm/arch/clock.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/sys_proto.h>
#include <env.h>
#include <fdt_support.h>
#include <asm/global_data.h>
#include <linux/errno.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/mach-imx/spi.h>
#include <common.h>
#include <fsl_esdhc_imx.h>
#include <linux/libfdt.h>
#include <i2c.h>
#include <mmc.h>
#include <power/pfuze100_pmic.h>
#include <power/pmic.h>
#include <spi_flash.h>

#include "tqma6_bb.h"

DECLARE_GLOBAL_DATA_PTR;

#define USDHC_CLK_PAD_CTRL (PAD_CTL_PUS_47K_UP  | PAD_CTL_SPEED_LOW | \
	PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PUS_47K_UP  | PAD_CTL_SPEED_LOW | \
	PAD_CTL_DSE_80ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define GPIO_OUT_PAD_CTRL  (PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_LOW | \
	PAD_CTL_DSE_40ohm   | PAD_CTL_HYS)

#define GPIO_IN_PAD_CTRL  (PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_LOW | \
	PAD_CTL_DSE_40ohm   | PAD_CTL_HYS)

#define SPI_PAD_CTRL (PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | \
	PAD_CTL_DSE_80ohm | PAD_CTL_SRE_FAST | PAD_CTL_HYS)

#define I2C_PAD_CTRL	(PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | \
	PAD_CTL_DSE_80ohm | PAD_CTL_HYS |			\
	PAD_CTL_ODE | PAD_CTL_SRE_FAST)

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();

	return 0;
}

static const uint16_t tqma6_emmc_dsr = 0x0100;

#ifndef CONFIG_DM_MMC
/* eMMC on USDHCI3 always present */
static iomux_v3_cfg_t const tqma6_usdhc3_pads[] = {
	NEW_PAD_CTRL(MX6_PAD_SD3_CLK__SD3_CLK,		USDHC_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_SD3_CMD__SD3_CMD,		USDHC_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_SD3_DAT0__SD3_DATA0,	USDHC_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_SD3_DAT1__SD3_DATA1,	USDHC_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_SD3_DAT2__SD3_DATA2,	USDHC_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_SD3_DAT3__SD3_DATA3,	USDHC_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_SD3_DAT4__SD3_DATA4,	USDHC_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_SD3_DAT5__SD3_DATA5,	USDHC_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_SD3_DAT6__SD3_DATA6,	USDHC_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_SD3_DAT7__SD3_DATA7,	USDHC_PAD_CTRL),
	/* eMMC reset */
	NEW_PAD_CTRL(MX6_PAD_SD3_RST__SD3_RESET,	GPIO_OUT_PAD_CTRL),
};

/*
 * According to board_mmc_init() the following map is done:
 * (U-Boot device node)    (Physical Port)
 * mmc0                    eMMC (SD3) on TQMa6
 * mmc1 .. n               optional slots used on baseboard
 */
struct fsl_esdhc_cfg tqma6_usdhc_cfg = {
	.esdhc_base = USDHC3_BASE_ADDR,
	.max_bus_width = 8,
};

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	if (cfg->esdhc_base == USDHC3_BASE_ADDR)
		/* eMMC/uSDHC3 is always present */
		ret = 1;
	else
		ret = tqma6_bb_board_mmc_getcd(mmc);

	return ret;
}

int board_mmc_getwp(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	if (cfg->esdhc_base == USDHC3_BASE_ADDR)
		/* eMMC/uSDHC3 is always present */
		ret = 0;
	else
		ret = tqma6_bb_board_mmc_getwp(mmc);

	return ret;
}

int board_mmc_init(struct bd_info *bis)
{
	imx_iomux_v3_setup_multiple_pads(tqma6_usdhc3_pads,
					 ARRAY_SIZE(tqma6_usdhc3_pads));
	tqma6_usdhc_cfg.sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
	if (fsl_esdhc_initialize(bis, &tqma6_usdhc_cfg)) {
		puts("Warning: failed to initialize eMMC dev\n");
	} else {
		struct mmc *mmc = find_mmc_device(0);
		if (mmc)
			mmc_set_dsr(mmc, tqma6_emmc_dsr);
	}

	tqma6_bb_board_mmc_init(bis);

	return 0;
}
#endif

#ifndef CONFIG_DM_SPI
static iomux_v3_cfg_t const tqma6_ecspi1_pads[] = {
	/* SS1 */
	NEW_PAD_CTRL(MX6_PAD_EIM_D19__GPIO3_IO19, SPI_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_D16__ECSPI1_SCLK, SPI_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_D17__ECSPI1_MISO, SPI_PAD_CTRL),
	NEW_PAD_CTRL(MX6_PAD_EIM_D18__ECSPI1_MOSI, SPI_PAD_CTRL),
};

#define TQMA6_SF_CS_GPIO IMX_GPIO_NR(3, 19)

static unsigned const tqma6_ecspi1_cs[] = {
	TQMA6_SF_CS_GPIO,
};

__weak void tqma6_iomuxc_spi(void)
{
	unsigned i;

	for (i = 0; i < ARRAY_SIZE(tqma6_ecspi1_cs); ++i)
		gpio_direction_output(tqma6_ecspi1_cs[i], 1);
	imx_iomux_v3_setup_multiple_pads(tqma6_ecspi1_pads,
					 ARRAY_SIZE(tqma6_ecspi1_pads));
}

#if defined(CONFIG_SF_DEFAULT_BUS) && defined(CONFIG_SF_DEFAULT_CS)
int board_spi_cs_gpio(unsigned bus, unsigned cs)
{
	return ((bus == CONFIG_SF_DEFAULT_BUS) &&
		(cs == CONFIG_SF_DEFAULT_CS)) ? TQMA6_SF_CS_GPIO : -1;
}
#endif
#endif

#if CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
static struct i2c_pads_info tqma6_i2c3_pads = {
	/* I2C3: on board LM75, M24C64,  */
	.scl = {
		.i2c_mode = NEW_PAD_CTRL(MX6_PAD_GPIO_5__I2C3_SCL,
					 I2C_PAD_CTRL),
		.gpio_mode = NEW_PAD_CTRL(MX6_PAD_GPIO_5__GPIO1_IO05,
					  I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(1, 5)
	},
	.sda = {
		.i2c_mode = NEW_PAD_CTRL(MX6_PAD_GPIO_6__I2C3_SDA,
					 I2C_PAD_CTRL),
		.gpio_mode = NEW_PAD_CTRL(MX6_PAD_GPIO_6__GPIO1_IO06,
					  I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(1, 6)
	}
};

static void tqma6_setup_i2c(void)
{
	int ret;
	/*
	 * use logical index for bus, e.g. I2C1 -> 0
	 * warn on error
	 */
	ret = setup_i2c(2, CONFIG_SYS_I2C_SPEED, 0x7f, &tqma6_i2c3_pads);
	if (ret)
		printf("setup I2C3 failed: %d\n", ret);
}
#endif

int board_early_init_f(void)
{
	return tqma6_bb_board_early_init_f();
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

#ifndef CONFIG_DM_SPI
	tqma6_iomuxc_spi();
#endif
#if CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
	tqma6_setup_i2c();
#endif

	tqma6_bb_board_init();

	return 0;
}

static const char *tqma6_get_boardname(void)
{
	u32 cpurev = get_cpu_rev();

	switch ((cpurev & 0xFF000) >> 12) {
	case MXC_CPU_MX6SOLO:
		return "TQMa6S";
		break;
	case MXC_CPU_MX6DL:
		return "TQMa6DL";
		break;
	case MXC_CPU_MX6D:
		return "TQMa6D";
		break;
	case MXC_CPU_MX6Q:
		return "TQMa6Q";
		break;
	default:
		return "??";
	};
}

#if CONFIG_IS_ENABLED(POWER_LEGACY)
/* setup board specific PMIC */
int power_init_board(void)
{
	struct pmic *p;
	u32 reg, rev;

	power_pfuze100_init(TQMA6_PFUZE100_I2C_BUS);
	p = pmic_get("PFUZE100");
	if (p && !pmic_probe(p)) {
		pmic_reg_read(p, PFUZE100_DEVICEID, &reg);
		pmic_reg_read(p, PFUZE100_REVID, &rev);
		printf("PMIC: PFUZE100 ID=0x%02x REV=0x%02x\n", reg, rev);
	}

	return 0;
}
#endif

int board_late_init(void)
{
	env_set("board_name", tqma6_get_boardname());

	tqma6_bb_board_late_init();

	return 0;
}

int checkboard(void)
{
	printf("Board: %s on a %s\n", tqma6_get_boardname(),
	       tqma6_bb_get_boardname());
	return 0;
}

/*
 * Device Tree Support
 */
#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT)
#define MODELSTRLEN 32u
int ft_board_setup(void *blob, struct bd_info *bd)
{
	char modelstr[MODELSTRLEN];

	snprintf(modelstr, MODELSTRLEN, "TQ %s on %s", tqma6_get_boardname(),
		 tqma6_bb_get_boardname());
	do_fixup_by_path_string(blob, "/", "model", modelstr);
	fdt_fixup_memory(blob, (u64)PHYS_SDRAM, (u64)gd->ram_size);
	/* bring in eMMC dsr settings */
	do_fixup_by_path_u32(blob,
			     "/soc/aips-bus@02100000/usdhc@02198000",
			     "dsr", tqma6_emmc_dsr, 2);
	tqma6_bb_ft_board_setup(blob, bd);

	return 0;
}
#endif /* defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT) */
