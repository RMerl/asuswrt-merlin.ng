// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Kontron Electronics GmbH
 */

#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/sections.h>
#include <fsl_esdhc_imx.h>
#include <init.h>
#include <linux/delay.h>
#include <linux/sizes.h>
#include <linux/errno.h>
#include <mmc.h>
#include <sl-mx6ul-common.h>

DECLARE_GLOBAL_DATA_PTR;

enum {
	BOARD_TYPE_KTN_SL_UL = 1,
	BOARD_TYPE_KTN_SL_ULL,
	BOARD_TYPE_MAX
};

#define UART_PAD_CTRL  (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_22K_UP  | PAD_CTL_SPEED_LOW |		\
	PAD_CTL_DSE_80ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_CD_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE |	\
	PAD_CTL_PUS_100K_DOWN  | PAD_CTL_SPEED_LOW |		\
	PAD_CTL_DSE_80ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define SPI_PAD_CTRL (PAD_CTL_HYS | PAD_CTL_SPEED_MED | \
	PAD_CTL_DSE_40ohm | PAD_CTL_SRE_FAST)

#include <spl.h>
#include <asm/arch/mx6-ddr.h>

static iomux_v3_cfg_t const usdhc1_pads[] = {
	MX6_PAD_SD1_CLK__USDHC1_CLK | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_CMD__USDHC1_CMD | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DATA0__USDHC1_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DATA1__USDHC1_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DATA2__USDHC1_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DATA3__USDHC1_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),

	/* CD */
	MX6_PAD_UART1_RTS_B__GPIO1_IO19 | MUX_PAD_CTRL(USDHC_CD_PAD_CTRL),
};

#define USDHC1_CD_GPIO	IMX_GPIO_NR(1, 19)

static iomux_v3_cfg_t const usdhc2_pads[] = {
	MX6_PAD_NAND_RE_B__USDHC2_CLK | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_NAND_WE_B__USDHC2_CMD | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_NAND_DATA00__USDHC2_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_NAND_DATA01__USDHC2_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_NAND_DATA02__USDHC2_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_NAND_DATA03__USDHC2_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	/* RST */
	MX6_PAD_NAND_ALE__GPIO4_IO10 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

#define USDHC2_PWR_GPIO	IMX_GPIO_NR(4, 10)

static struct fsl_esdhc_cfg usdhc_cfg[2] = {
	{USDHC1_BASE_ADDR, 0, 4},
	{USDHC2_BASE_ADDR, 0, 4},
};

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	switch (cfg->esdhc_base) {
	case USDHC1_BASE_ADDR:
		ret = !gpio_get_value(USDHC1_CD_GPIO);
		break;
	case USDHC2_BASE_ADDR:
		// This SDHC interface does not use a CD pin
		ret = 1;
		break;
	}

	return ret;
}

int board_mmc_init(struct bd_info *bis)
{
	int i, ret;

	/*
	 * According to the board_mmc_init() the following map is done:
	 * (U-Boot device node)    (Physical Port)
	 * mmc0                    USDHC1
	 * mmc1                    USDHC2
	 */
	for (i = 0; i < CFG_SYS_FSL_USDHC_NUM; i++) {
		switch (i) {
		case 0:
			imx_iomux_v3_setup_multiple_pads(usdhc1_pads, ARRAY_SIZE(usdhc1_pads));
			gpio_direction_input(USDHC1_CD_GPIO);
			usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
			break;
		case 1:
			imx_iomux_v3_setup_multiple_pads(usdhc2_pads, ARRAY_SIZE(usdhc2_pads));
			gpio_direction_output(USDHC2_PWR_GPIO, 0);
			udelay(500);
			gpio_direction_output(USDHC2_PWR_GPIO, 1);
			usdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
			break;
		default:
			printf("Warning: you configured more USDHC controllers (%d) than supported by the board\n",
			       i + 1);
			return -EINVAL;
			}

			ret = fsl_esdhc_initialize(bis, &usdhc_cfg[i]);
			if (ret) {
				printf("Warning: failed to initialize mmc dev %d\n", i);
				return ret;
			}
	}
	return 0;
}

iomux_v3_cfg_t const ecspi2_pads[] = {
	MX6_PAD_CSI_DATA00__ECSPI2_SCLK | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_CSI_DATA02__ECSPI2_MOSI | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_CSI_DATA03__ECSPI2_MISO | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_CSI_DATA01__GPIO4_IO22  | MUX_PAD_CTRL(NO_PAD_CTRL),
};

int board_spi_cs_gpio(unsigned int bus, unsigned int cs)
{
	return (bus == CONFIG_SF_DEFAULT_BUS && cs == CONFIG_SF_DEFAULT_CS)
		? (IMX_GPIO_NR(4, 22)) : -1;
}

static void setup_spi(void)
{
	gpio_request(IMX_GPIO_NR(4, 22), "spi2_cs0");
	gpio_direction_output(IMX_GPIO_NR(4, 22), 1);
	imx_iomux_v3_setup_multiple_pads(ecspi2_pads, ARRAY_SIZE(ecspi2_pads));

	enable_spi_clk(true, 1);
}

static iomux_v3_cfg_t const uart4_pads[] = {
	MX6_PAD_UART4_TX_DATA__UART4_DCE_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_UART4_RX_DATA__UART4_DCE_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static void setup_iomux_uart(void)
{
	imx_iomux_v3_setup_multiple_pads(uart4_pads, ARRAY_SIZE(uart4_pads));
}

// DDR 256MB (Hynix H5TQ2G63DFR)
static struct mx6_ddr3_cfg mem_256M_ddr = {
	.mem_speed = 800,
	.density = 2,
	.width = 16,
	.banks = 8,
	.rowaddr = 14,
	.coladdr = 10,
	.pagesz = 2,
	.trcd = 1350,
	.trcmin = 4950,
	.trasmin = 3600,
};

static struct mx6_mmdc_calibration mx6_mmcd_256M_calib = {
	.p0_mpwldectrl0 = 0x00000000,
	.p0_mpdgctrl0 = 0x01340134,
	.p0_mprddlctl = 0x40405052,
	.p0_mpwrdlctl = 0x40404E48,
};

// DDR 512MB (Hynix H5TQ4G63DFR)
static struct mx6_ddr3_cfg mem_512M_ddr = {
	.mem_speed = 800,
	.density = 4,
	.width = 16,
	.banks = 8,
	.rowaddr = 15,
	.coladdr = 10,
	.pagesz = 2,
	.trcd = 1350,
	.trcmin = 4950,
	.trasmin = 3600,
};

static struct mx6_mmdc_calibration mx6_mmcd_512M_calib = {
	.p0_mpwldectrl0 = 0x00000000,
	.p0_mpdgctrl0 = 0X01440144,
	.p0_mprddlctl = 0x40405454,
	.p0_mpwrdlctl = 0x40404E4C,
};

// Common DDR parameters (256MB and 512MB)
static struct mx6ul_iomux_grp_regs mx6_grp_ioregs = {
	.grp_addds = 0x00000028,
	.grp_ddrmode_ctl = 0x00020000,
	.grp_b0ds = 0x00000028,
	.grp_ctlds = 0x00000028,
	.grp_b1ds = 0x00000028,
	.grp_ddrpke = 0x00000000,
	.grp_ddrmode = 0x00020000,
	.grp_ddr_type = 0x000c0000,
};

static struct mx6ul_iomux_ddr_regs mx6_ddr_ioregs = {
	.dram_dqm0 = 0x00000028,
	.dram_dqm1 = 0x00000028,
	.dram_ras = 0x00000028,
	.dram_cas = 0x00000028,
	.dram_odt0 = 0x00000028,
	.dram_odt1 = 0x00000028,
	.dram_sdba2 = 0x00000000,
	.dram_sdclk_0 = 0x00000028,
	.dram_sdqs0 = 0x00000028,
	.dram_sdqs1 = 0x00000028,
	.dram_reset = 0x00000028,
};

struct mx6_ddr_sysinfo ddr_sysinfo = {
	.dsize = 0,
	.cs_density = 20,
	.ncs = 1,
	.cs1_mirror = 0,
	.rtt_wr = 2,
	.rtt_nom = 1,		/* RTT_Nom = RZQ/2 */
	.walat = 1,		/* Write additional latency */
	.ralat = 5,		/* Read additional latency */
	.mif3_mode = 3,		/* Command prediction working mode */
	.bi_on = 1,		/* Bank interleaving enabled */
	.sde_to_rst = 0x10,	/* 14 cycles, 200us (JEDEC default) */
	.rst_to_cke = 0x23,	/* 33 cycles, 500us (JEDEC default) */
	.ddr_type = DDR_TYPE_DDR3,
	.refsel = 0,	/* Refresh cycles at 64KHz */
	.refr = 1,	/* 2 refresh commands per refresh cycle */
};

static void ccgr_init(void)
{
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	writel(0xFFFFFFFF, &ccm->CCGR0);
	writel(0xFFFFFFFF, &ccm->CCGR1);
	writel(0xFFFFFFFF, &ccm->CCGR2);
	writel(0xFFFFFFFF, &ccm->CCGR3);
	writel(0xFFFFFFFF, &ccm->CCGR4);
	writel(0xFFFFFFFF, &ccm->CCGR5);
	writel(0xFFFFFFFF, &ccm->CCGR6);
	writel(0xFFFFFFFF, &ccm->CCGR7);
}

static void spl_dram_init(void)
{
	unsigned int size;

	// DDR RAM connection is always 16 bit wide. Init IOs.
	mx6ul_dram_iocfg(16, &mx6_ddr_ioregs, &mx6_grp_ioregs);

	// Try to detect the 512MB RAM chip first.
	mx6_dram_cfg(&ddr_sysinfo, &mx6_mmcd_512M_calib, &mem_512M_ddr);

	// Get the available RAM size
	size = get_ram_size((void *)PHYS_SDRAM, SZ_512M);

	gd->ram_size = size;

	if (size == SZ_512M) {
		// 512MB RAM was detected
		return;
	} else if (size == SZ_256M) {
		// 256MB RAM was detected, use correct config and calibration
		mx6_dram_cfg(&ddr_sysinfo, &mx6_mmcd_256M_calib, &mem_256M_ddr);
	} else {
		printf("Invalid DDR RAM size detected: %x\n", size);
	}
}

static int do_board_detect(void)
{
	if (is_mx6ul())
		gd->board_type = BOARD_TYPE_KTN_SL_UL;
	else if (is_mx6ull())
		gd->board_type = BOARD_TYPE_KTN_SL_ULL;

	printf("Kontron SL i.MX6UL%s (N6%s1x) module, %lu MB RAM detected\n",
	       is_mx6ull() ? "L" : "", is_mx6ull() ? "4" : "3", gd->ram_size / SZ_1M);

	return 0;
}

void board_init_f(ulong dummy)
{
	ccgr_init();

	/* setup AIPS and disable watchdog */
	arch_cpu_init();

	/* iomux and setup of UART and SPI */
	board_early_init_f();

	/* setup GP timer */
	timer_init();

	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();

	/* DDR initialization */
	spl_dram_init();

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	/* Detect the board type */
	do_board_detect();

	/* load/boot image from boot device */
	board_init_r(NULL, 0);
}

void board_boot_order(u32 *spl_boot_list)
{
	u32 bootdev = spl_boot_device();

	/*
	 * The default boot fuse settings use the SD card (MMC1) as primary
	 * boot device, but allow SPI NOR as a fallback boot device. There
	 * is no proper way to detect if the fallback was used. Therefore
	 * we read the ECSPI2_CONREG register and see if it differs from the
	 * reset value 0x0. If that's the case we can assume that the BootROM
	 * has successfully probed the SPI NOR.
	 */
	switch (bootdev) {
	case BOOT_DEVICE_MMC1:
	case BOOT_DEVICE_MMC2:
		if (sl_mx6ul_is_spi_nor_boot()) {
			spl_boot_list[0] = BOOT_DEVICE_SPI;
			return;
		}
		break;
	}

	spl_boot_list[0] = bootdev;
}

int board_early_init_f(void)
{
	setup_iomux_uart();
	if (sl_mx6ul_is_spi_nor_boot())
		setup_spi();

	return 0;
}

int board_fit_config_name_match(const char *name)
{
	if (gd->board_type == BOARD_TYPE_KTN_SL_UL && is_mx6ul() &&
	    (!strcmp(name, "imx6ul-kontron-n631x-s") || !strcmp(name, "imx6ul-kontron-bl")))
		return 0;

	if (gd->board_type == BOARD_TYPE_KTN_SL_ULL && is_mx6ull() &&
	    (!strcmp(name, "imx6ull-kontron-n641x-s") || !strcmp(name, "imx6ull-kontron-bl")))
		return 0;

	return -1;
}
