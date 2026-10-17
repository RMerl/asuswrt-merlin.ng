// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Kontron Electronics GmbH
 */

#include <asm/arch/clock.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/crm_regs.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/sections.h>
#include <init.h>
#include <spl.h>
#include <fsl_esdhc_imx.h>
#include <mmc.h>

#define SPI_PAD_CTRL (PAD_CTL_HYS | PAD_CTL_SPEED_MED | 	\
	PAD_CTL_DSE_40ohm | PAD_CTL_SRE_FAST)

#define UART_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |			\
	PAD_CTL_SRE_FAST | PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_22K_UP | PAD_CTL_SPEED_LOW |		\
	PAD_CTL_DSE_80ohm | PAD_CTL_SRE_FAST | PAD_CTL_HYS)

#define GPIO_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_100K_DOWN | PAD_CTL_SPEED_LOW |		\
	PAD_CTL_DSE_80ohm | PAD_CTL_SRE_FAST | PAD_CTL_HYS)

static const iomux_v3_cfg_t ecspi2_pads[] = {
	MX6_PAD_CSI0_DAT10__ECSPI2_MISO | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_CSI0_DAT9__ECSPI2_MOSI | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_CSI0_DAT8__ECSPI2_SCLK | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_CSI0_DAT11__GPIO5_IO29  | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static const iomux_v3_cfg_t uart2_pads[] = {
	MX6_PAD_SD4_DAT7__UART2_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_SD4_DAT4__UART2_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static const iomux_v3_cfg_t usdhc3_pads[] = {
	MX6_PAD_SD3_CLK__SD3_CLK | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_CMD__SD3_CMD | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT0__SD3_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT1__SD3_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT2__SD3_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT3__SD3_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),

	/* CD */
	MX6_PAD_GPIO_4__GPIO1_IO04 | MUX_PAD_CTRL(GPIO_PAD_CTRL),
};

#define USDHC3_CD_GPIO	IMX_GPIO_NR(1, 4)
#define SPI2_CS_GPIO	IMX_GPIO_NR(5, 29)

static struct fsl_esdhc_cfg usdhc_cfg[1] = {
	{USDHC3_BASE_ADDR, 0, 4},
};

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	switch (cfg->esdhc_base) {
	case USDHC3_BASE_ADDR:
		ret = !gpio_get_value(USDHC3_CD_GPIO);
		break;
	}

	return ret;
}

int board_mmc_init(struct bd_info *bis)
{
	int i, ret;

	/*
	 * According to the board_mmc_init() the following map is done:
	 * (U-boot device node)    (Physical Port)
	 * mmc0                    USDHC1
	 * mmc1                    USDHC2
	 */
	for (i = 0; i < CFG_SYS_FSL_USDHC_NUM; i++) {
		switch (i) {
		case 0:
			imx_iomux_v3_setup_multiple_pads(usdhc3_pads,
							 ARRAY_SIZE(usdhc3_pads));
			gpio_direction_input(USDHC3_CD_GPIO);
			usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
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

static void ccgr_init(void)
{
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	writel(0x00C03F3F, &ccm->CCGR0);
	writel(0x0030FC03, &ccm->CCGR1);
	writel(0x0FFFC000, &ccm->CCGR2);
	writel(0x3FF00000, &ccm->CCGR3);
	writel(0x00FFF300, &ccm->CCGR4);
	writel(0x0F0000C3, &ccm->CCGR5);
	writel(0x000003FF, &ccm->CCGR6);
}

static int mx6ssielaff_dcd_table[] = {
	0x020e0774, 0x000C0000,
	0x020e0754, 0x00000000,
	0x020e04ac, 0x00000030,
	0x020e04b0, 0x00000030,
	0x020e0464, 0x00000030,
	0x020e0490, 0x00000030,
	0x020e074c, 0x00000030,
	0x020e0494, 0x00000030,
	0x020e04a0, 0x00000000,
	0x020e04b4, 0x00000030,
	0x020e04b8, 0x00000030,
	0x020e076c, 0x00000030,
	0x020e0750, 0x00020000,
	0x020e04bc, 0x00000030,
	0x020e04c0, 0x00000030,
	0x020e04c4, 0x00000030,
	0x020e04c8, 0x00000030,
	0x020e0760, 0x00020000,
	0x020e0764, 0x00000030,
	0x020e0770, 0x00000030,
	0x020e0778, 0x00000030,
	0x020e077c, 0x00000030,
	0x020e0470, 0x00000030,
	0x020e0474, 0x00000030,
	0x020e0478, 0x00000030,
	0x020e047c, 0x00000030,
	0x021b001c, 0x00008000,
	0x021b0800, 0xA1390003,
	0x021b080c, 0x00350035,
	0x021b0810, 0x002A0032,
	0x021b083c, 0x02340234,
	0x021b0840, 0x02200220,
	0x021b0848, 0x4650504E,
	0x021b0850, 0x3A342E34,
	0x021b081c, 0x33333333,
	0x021b0820, 0x33333333,
	0x021b0824, 0x33333333,
	0x021b0828, 0x33333333,
	0x021b08b8, 0x00000800,
	0x021b0004, 0x0002002D,
	0x021b0008, 0x00333040,
	0x021b000c, 0x676B52F3,
	0x021b0010, 0xB66D8B63,
	0x021b0014, 0x01FF00DB,
	0x021b0018, 0x00011740,
	0x021b001c, 0x00008000,
	0x021b002c, 0x000026D2,
	0x021b0030, 0x006B1023,
	0x021b0040, 0x00000027,
	0x021b0000, 0x84190000,
	0x021b001c, 0x02008032,
	0x021b001c, 0x00008033,
	0x021b001c, 0x00048031,
	0x021b001c, 0x15208030,
	0x021b001c, 0x04008040,
	0x021b0020, 0x00007800,
	0x021b0818, 0x00022227,
	0x021b0004, 0x0002556D,
	0x021b0404, 0x00011006,
	0x021b001c, 0x00000000,
	0x020c4068, 0x00C03F3F,
	0x020c406c, 0x0030FC03,
	0x020c4070, 0x0FFFC000,
	0x020c4074, 0x3FF00000,
	0x020c4078, 0xFFFFF300,
	0x020c407c, 0x0F0000C3,
	0x020c4080, 0x000003FF,
	0x020e0010, 0xF00000CF,
	0x020e0018, 0x007F007F,
	0x020e001c, 0x007F007F,
};

static void ddr_init(int *table, int size)
{
	int i;

	for (i = 0; i < size / 2 ; i++)
		writel(table[2 * i + 1], table[2 * i]);
}

static void spl_dram_init(void)
{
	ddr_init(mx6ssielaff_dcd_table, ARRAY_SIZE(mx6ssielaff_dcd_table));
}

int board_spi_cs_gpio(unsigned int bus, unsigned int cs)
{
	return (bus == CONFIG_SF_DEFAULT_BUS && cs == CONFIG_SF_DEFAULT_CS)
		? SPI2_CS_GPIO : -1;
}

static void setup_spi(void)
{
	imx_iomux_v3_setup_multiple_pads(ecspi2_pads, ARRAY_SIZE(ecspi2_pads));
	gpio_request(SPI2_CS_GPIO, "spi2_cs0");
	gpio_direction_output(SPI2_CS_GPIO, 1);
	enable_spi_clk(true, 1);
}

void board_init_f(ulong dummy)
{
	/* setup AIPS and disable watchdog */
	arch_cpu_init();

	ccgr_init();
	gpr_init();

	/* IOMUX UART */
	imx_iomux_v3_setup_multiple_pads(uart2_pads, ARRAY_SIZE(uart2_pads));

	/* setup GP timer */
	timer_init();

	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();

	/* DDR initialization */
	spl_dram_init();

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	/* SPI */
	setup_spi();

	/* load/boot image from boot device */
	board_init_r(NULL, 0);
}

void board_boot_order(u32 *spl_boot_list)
{
	u32 bootdev = spl_boot_device();

	/*
	 * The default boot fuse settings use the SD card (MMC1) as primary
	 * boot device, but allow SPI NOR as a fallback boot device.
	 * We can't detect the fallback case and spl_boot_device() will return
	 * BOOT_DEVICE_MMC1 despite the actual boot device being SPI NOR.
	 * Therefore we try to load U-Boot proper vom SPI NOR after loading
	 * from MMC has failed.
	 */
	spl_boot_list[0] = bootdev;

	switch (bootdev) {
	case BOOT_DEVICE_MMC1:
	case BOOT_DEVICE_MMC2:
		spl_boot_list[1] = BOOT_DEVICE_SPI;
		break;
	}
}
