// SPDX-License-Identifier: GPL-2.0+
/*
 * Board functions for BuR BRPPT2 board
 *
 * Copyright (C) 2019
 * B&R Industrial Automation GmbH - http://www.br-automation.com/
 *
 */
#include <common.h>
#include <cpu_func.h>
#include <hang.h>
#include <init.h>
#include <spl.h>
#include <dm.h>
#include <miiphy.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#ifdef CONFIG_SPL_BUILD
# include <asm/arch/mx6-ddr.h>
#endif
#include <asm/arch/clock.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/gpio.h>

#define USBHUB_RSTN	IMX_GPIO_NR(1, 16)
#define BKLT_EN		IMX_GPIO_NR(1, 15)
#define CAPT_INT	IMX_GPIO_NR(4, 9)
#define CAPT_RESETN	IMX_GPIO_NR(4, 11)
#define SW_INTN		IMX_GPIO_NR(3, 26)
#define VCCDISP_EN	IMX_GPIO_NR(5, 18)
#define EMMC_RSTN	IMX_GPIO_NR(6, 8)
#define PMIC_IRQN	IMX_GPIO_NR(5, 22)
#define TASTER		IMX_GPIO_NR(5, 23)

#define ETH0_LINK	IMX_GPIO_NR(1, 27)
#define ETH1_LINK	IMX_GPIO_NR(1, 28)

#define UART_PAD_CTRL		(PAD_CTL_PUS_47K_UP |			\
				PAD_CTL_SPEED_LOW | PAD_CTL_DSE_48ohm |	\
				PAD_CTL_SRE_SLOW  | PAD_CTL_HYS)

#define I2C_PAD_CTRL		(PAD_CTL_PUS_47K_UP |			\
				PAD_CTL_SPEED_LOW | PAD_CTL_DSE_48ohm |	\
				PAD_CTL_SRE_SLOW  | PAD_CTL_HYS)

#define ECSPI_PAD_CTRL		(PAD_CTL_PUS_100K_DOWN |		\
				PAD_CTL_SPEED_MED | PAD_CTL_DSE_48ohm |	\
				PAD_CTL_SRE_FAST  | PAD_CTL_HYS)
#define USDHC_PAD_CTRL		(PAD_CTL_PUS_47K_UP |			\
				PAD_CTL_SPEED_LOW | PAD_CTL_DSE_48ohm |	\
				PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define ENET_PAD_CTRL		(PAD_CTL_PUS_100K_UP |			\
				PAD_CTL_SPEED_MED | PAD_CTL_DSE_60ohm |	\
				PAD_CTL_SRE_SLOW  | PAD_CTL_HYS)

#define ENET_PAD_CTRL1		(PAD_CTL_PUS_100K_UP |			\
				PAD_CTL_SPEED_MED | PAD_CTL_DSE_34ohm |	\
				PAD_CTL_SRE_SLOW  | PAD_CTL_HYS)

#define ENET_PAD_CTRL_PU	(PAD_CTL_PUS_100K_UP |		\
				PAD_CTL_SPEED_MED | PAD_CTL_DSE_80ohm |	\
				PAD_CTL_SRE_SLOW  | PAD_CTL_HYS)

#define ENET_PAD_CTRL_CLK	((PAD_CTL_PUS_100K_UP & ~PAD_CTL_PKE) |	\
				PAD_CTL_SPEED_MED | PAD_CTL_DSE_60ohm |	\
				PAD_CTL_SRE_FAST)

#define GPIO_PAD_CTRL_PU	(PAD_CTL_PUS_100K_UP |			\
				PAD_CTL_SPEED_LOW | PAD_CTL_DSE_48ohm |	\
				PAD_CTL_SRE_SLOW  | PAD_CTL_HYS)

#define GPIO_PAD_CTRL_PD	(PAD_CTL_PUS_100K_DOWN |		\
				PAD_CTL_SPEED_LOW | PAD_CTL_DSE_48ohm |	\
				PAD_CTL_SRE_SLOW  | PAD_CTL_HYS)

#define LCDCMOS_PAD_CTRL	(PAD_CTL_PUS_100K_DOWN |		\
				PAD_CTL_SPEED_LOW | PAD_CTL_DSE_120ohm |\
				PAD_CTL_SRE_SLOW  | PAD_CTL_HYS)

#define MUXDESC(pad, ctrl)	IOMUX_PADS(pad | MUX_PAD_CTRL(ctrl))

#if !defined(CONFIG_SPL_BUILD)
static iomux_v3_cfg_t const eth_pads[] = {
	/*
	 * Gigabit Ethernet
	 */
	/* CLKs */
	MUXDESC(PAD_GPIO_16__ENET_REF_CLK,	ENET_PAD_CTRL_CLK),
	MUXDESC(PAD_ENET_REF_CLK__ENET_TX_CLK,	ENET_PAD_CTRL_CLK),
	/* MDIO */
	MUXDESC(PAD_ENET_MDIO__ENET_MDIO,	ENET_PAD_CTRL_PU),
	MUXDESC(PAD_ENET_MDC__ENET_MDC,		ENET_PAD_CTRL_PU),
	/* RGMII */
	MUXDESC(PAD_RGMII_TXC__RGMII_TXC,	ENET_PAD_CTRL1),
	MUXDESC(PAD_RGMII_TD0__RGMII_TD0,	ENET_PAD_CTRL),
	MUXDESC(PAD_RGMII_TD1__RGMII_TD1,	ENET_PAD_CTRL),
	MUXDESC(PAD_RGMII_TD2__RGMII_TD2,	ENET_PAD_CTRL),
	MUXDESC(PAD_RGMII_TD3__RGMII_TD3,	ENET_PAD_CTRL),
	MUXDESC(PAD_RGMII_TX_CTL__RGMII_TX_CTL,	ENET_PAD_CTRL),
	MUXDESC(PAD_RGMII_RXC__RGMII_RXC,	ENET_PAD_CTRL_PU),
	MUXDESC(PAD_RGMII_RD0__RGMII_RD0,	ENET_PAD_CTRL_PU),
	MUXDESC(PAD_RGMII_RD1__RGMII_RD1,	ENET_PAD_CTRL_PU),
	MUXDESC(PAD_RGMII_RD2__RGMII_RD2,	ENET_PAD_CTRL_PU),
	MUXDESC(PAD_RGMII_RD3__RGMII_RD3,	ENET_PAD_CTRL_PU),
	MUXDESC(PAD_RGMII_RX_CTL__RGMII_RX_CTL,	ENET_PAD_CTRL_PU),
	/* ETH0_LINK */
	MUXDESC(PAD_ENET_RXD0__GPIO1_IO27,	GPIO_PAD_CTRL_PD),
	/* ETH1_LINK */
	MUXDESC(PAD_ENET_TX_EN__GPIO1_IO28,	GPIO_PAD_CTRL_PD),
};

static iomux_v3_cfg_t const board_pads[] = {
	/*
	 * I2C #3, #4
	 */
	MUXDESC(PAD_GPIO_3__I2C3_SCL,		I2C_PAD_CTRL),
	MUXDESC(PAD_GPIO_6__I2C3_SDA,		I2C_PAD_CTRL),

	/*
	 * UART#4 PADS
	 * UART_Tasten
	 */
	MUXDESC(PAD_CSI0_DAT12__UART4_TX_DATA,	UART_PAD_CTRL),
	MUXDESC(PAD_CSI0_DAT13__UART4_RX_DATA,	UART_PAD_CTRL),
	MUXDESC(PAD_CSI0_DAT17__UART4_CTS_B,	UART_PAD_CTRL),
	MUXDESC(PAD_CSI0_DAT16__UART4_RTS_B,	UART_PAD_CTRL),
	/*
	 * ESCPI#1
	 * M25P32 NOR-Flash
	 */
	MUXDESC(PAD_EIM_D16__ECSPI1_SCLK,	ECSPI_PAD_CTRL),
	MUXDESC(PAD_EIM_D17__ECSPI1_MISO,	ECSPI_PAD_CTRL),
	MUXDESC(PAD_EIM_D18__ECSPI1_MOSI,	ECSPI_PAD_CTRL),
	MUXDESC(PAD_EIM_D19__GPIO3_IO19,	ECSPI_PAD_CTRL),
	/*
	 * ESCPI#2
	 * resTouch SPI ADC
	 */
	MUXDESC(PAD_CSI0_DAT8__ECSPI2_SCLK,	ECSPI_PAD_CTRL),
	MUXDESC(PAD_EIM_OE__ECSPI2_MISO,	ECSPI_PAD_CTRL),
	MUXDESC(PAD_CSI0_DAT9__ECSPI2_MOSI,	ECSPI_PAD_CTRL),
	MUXDESC(PAD_EIM_D24__GPIO3_IO24,	ECSPI_PAD_CTRL),
	/*
	 * USDHC#4
	 */
	MUXDESC(PAD_SD4_CLK__SD4_CLK,		USDHC_PAD_CTRL),
	MUXDESC(PAD_SD4_CMD__SD4_CMD,		USDHC_PAD_CTRL),
	MUXDESC(PAD_SD4_DAT0__SD4_DATA0,	USDHC_PAD_CTRL),
	MUXDESC(PAD_SD4_DAT1__SD4_DATA1,	USDHC_PAD_CTRL),
	MUXDESC(PAD_SD4_DAT2__SD4_DATA2,	USDHC_PAD_CTRL),
	MUXDESC(PAD_SD4_DAT3__SD4_DATA3,	USDHC_PAD_CTRL),
	MUXDESC(PAD_SD4_DAT4__SD4_DATA4,	USDHC_PAD_CTRL),
	MUXDESC(PAD_SD4_DAT5__SD4_DATA5,	USDHC_PAD_CTRL),
	MUXDESC(PAD_SD4_DAT6__SD4_DATA6,	USDHC_PAD_CTRL),
	MUXDESC(PAD_SD4_DAT7__SD4_DATA7,	USDHC_PAD_CTRL),
	/*
	 * USB OTG power & ID
	 */
	/* USB_OTG_5V_EN */
	MUXDESC(PAD_EIM_D22__GPIO3_IO22,	GPIO_PAD_CTRL_PD),
	MUXDESC(PAD_EIM_D31__GPIO3_IO31,	GPIO_PAD_CTRL_PD),
	/* USB_OTG_JUMPER */
	MUXDESC(PAD_ENET_RX_ER__USB_OTG_ID,	GPIO_PAD_CTRL_PD),
	/*
	 * PWM-Pins
	 */
	/* BKLT_CTL */
	MUXDESC(PAD_SD1_CMD__PWM4_OUT,		GPIO_PAD_CTRL_PD),
	/* SPEAKER */
	MUXDESC(PAD_SD1_DAT1__PWM3_OUT,		GPIO_PAD_CTRL_PD),
	/*
	 * GPIOs
	 */
	/* USB_HUB_nRESET */
	MUXDESC(PAD_SD1_DAT0__GPIO1_IO16,	GPIO_PAD_CTRL_PD),
	/* BKLT_EN */
	MUXDESC(PAD_SD2_DAT0__GPIO1_IO15,	GPIO_PAD_CTRL_PD),
	/* capTouch_INT */
	MUXDESC(PAD_KEY_ROW1__GPIO4_IO09,	GPIO_PAD_CTRL_PD),
	/* capTouch_nRESET */
	MUXDESC(PAD_KEY_ROW2__GPIO4_IO11,	GPIO_PAD_CTRL_PD),
	/* SW_nINT */
	MUXDESC(PAD_EIM_D26__GPIO3_IO26,	GPIO_PAD_CTRL_PU),
	/* VCC_DISP_EN */
	MUXDESC(PAD_CSI0_PIXCLK__GPIO5_IO18,	GPIO_PAD_CTRL_PD),
	/* eMMC_nRESET */
	MUXDESC(PAD_NANDF_ALE__GPIO6_IO08,	GPIO_PAD_CTRL_PD),
	/* HWID*/
	MUXDESC(PAD_NANDF_D0__GPIO2_IO00,	GPIO_PAD_CTRL_PU),
	MUXDESC(PAD_NANDF_D1__GPIO2_IO01,	GPIO_PAD_CTRL_PU),
	MUXDESC(PAD_NANDF_D2__GPIO2_IO02,	GPIO_PAD_CTRL_PU),
	MUXDESC(PAD_NANDF_D3__GPIO2_IO03,	GPIO_PAD_CTRL_PU),
	/* PMIC_nIRQ */
	MUXDESC(PAD_CSI0_DAT4__GPIO5_IO22,	GPIO_PAD_CTRL_PU),
	/* nTASTER */
	MUXDESC(PAD_CSI0_DAT5__GPIO5_IO23,	GPIO_PAD_CTRL_PU),
	/* RGB LCD Display */
	MUXDESC(PAD_DI0_DISP_CLK__IPU1_DI0_DISP_CLK,	LCDCMOS_PAD_CTRL),
	MUXDESC(PAD_DI0_PIN2__IPU1_DI0_PIN02,		LCDCMOS_PAD_CTRL),
	MUXDESC(PAD_DI0_PIN3__IPU1_DI0_PIN03,		LCDCMOS_PAD_CTRL),
	MUXDESC(PAD_DI0_PIN4__IPU1_DI0_PIN04,		LCDCMOS_PAD_CTRL),
	MUXDESC(PAD_DI0_PIN15__IPU1_DI0_PIN15,		LCDCMOS_PAD_CTRL),
	MUXDESC(PAD_DISP0_DAT0__IPU1_DISP0_DATA00,	LCDCMOS_PAD_CTRL),
	MUXDESC(PAD_DISP0_DAT1__IPU1_DISP0_DATA01,	LCDCMOS_PAD_CTRL),
	MUXDESC(PAD_DISP0_DAT2__IPU1_DISP0_DATA02,	LCDCMOS_PAD_CTRL),
	MUXDESC(PAD_DISP0_DAT3__IPU1_DISP0_DATA03,	LCDCMOS_PAD_CTRL),
	MUXDESC(PAD_DISP0_DAT4__IPU1_DISP0_DATA04,	LCDCMOS_PAD_CTRL),
	MUXDESC(PAD_DISP0_DAT5__IPU1_DISP0_DATA05,	LCDCMOS_PAD_CTRL),
	MUXDESC(PAD_DISP0_DAT6__IPU1_DISP0_DATA06,	LCDCMOS_PAD_CTRL),
	MUXDESC(PAD_DISP0_DAT7__IPU1_DISP0_DATA07,	LCDCMOS_PAD_CTRL),
	MUXDESC(PAD_DISP0_DAT8__IPU1_DISP0_DATA08,	LCDCMOS_PAD_CTRL),
	MUXDESC(PAD_DISP0_DAT9__IPU1_DISP0_DATA09,	LCDCMOS_PAD_CTRL),
	MUXDESC(PAD_DISP0_DAT10__IPU1_DISP0_DATA10,	LCDCMOS_PAD_CTRL),
	MUXDESC(PAD_DISP0_DAT11__IPU1_DISP0_DATA11,	LCDCMOS_PAD_CTRL),
	MUXDESC(PAD_DISP0_DAT12__IPU1_DISP0_DATA12,	LCDCMOS_PAD_CTRL),
	MUXDESC(PAD_DISP0_DAT13__IPU1_DISP0_DATA13,	LCDCMOS_PAD_CTRL),
	MUXDESC(PAD_DISP0_DAT14__IPU1_DISP0_DATA14,	LCDCMOS_PAD_CTRL),
	MUXDESC(PAD_DISP0_DAT15__IPU1_DISP0_DATA15,	LCDCMOS_PAD_CTRL),
	MUXDESC(PAD_DISP0_DAT16__IPU1_DISP0_DATA16,	LCDCMOS_PAD_CTRL),
	MUXDESC(PAD_DISP0_DAT17__IPU1_DISP0_DATA17,	LCDCMOS_PAD_CTRL),
	MUXDESC(PAD_DISP0_DAT18__IPU1_DISP0_DATA18,	LCDCMOS_PAD_CTRL),
	MUXDESC(PAD_DISP0_DAT19__IPU1_DISP0_DATA19,	LCDCMOS_PAD_CTRL),
	MUXDESC(PAD_DISP0_DAT20__IPU1_DISP0_DATA20,	LCDCMOS_PAD_CTRL),
	MUXDESC(PAD_DISP0_DAT21__IPU1_DISP0_DATA21,	LCDCMOS_PAD_CTRL),
	MUXDESC(PAD_DISP0_DAT22__IPU1_DISP0_DATA22,	LCDCMOS_PAD_CTRL),
	MUXDESC(PAD_DISP0_DAT23__IPU1_DISP0_DATA23,	LCDCMOS_PAD_CTRL),
};

int board_ehci_hcd_init(int port)
{
	gpio_direction_output(USBHUB_RSTN, 1);

	return 0;
}

int board_late_init(void)
{
	ulong b_mode = 4;

	if (gpio_get_value(TASTER) == 0)
		b_mode = 12;

	env_set_ulong("b_mode", b_mode);

	return 0;
}

int board_init(void)
{
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	if (gpio_request(BKLT_EN, "BKLT_EN"))
		printf("Warning: BKLT_EN setup failed\n");
	gpio_direction_output(BKLT_EN, 0);

	if (gpio_request(USBHUB_RSTN, "USBHUB_nRST"))
		printf("Warning: USBHUB_nRST setup failed\n");
	gpio_direction_output(USBHUB_RSTN, 0);

	if (gpio_request(TASTER, "TASTER"))
		printf("Warning: TASTER setup failed\n");
	gpio_direction_input(TASTER);

	return 0;
}

int board_early_init_f(void)
{
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;

	SETUP_IOMUX_PADS(board_pads);
	SETUP_IOMUX_PADS(eth_pads);

	/* set GPIO_16 as ENET_REF_CLK_OUT running at 25 MHz */
	setbits_le32(&iomux->gpr[1], IOMUXC_GPR1_ENET_CLK_SEL_MASK);
	enable_fec_anatop_clock(0, ENET_25MHZ);
	enable_enet_clk(1);

	return 0;
}

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();

	return 0;
}
#else
/* configure MX6SOLO/DUALLITE mmdc DDR io registers */
static struct mx6sdl_iomux_ddr_regs ddr_iomux_s = {
	/* SDCLK[0:1], CAS, RAS, Reset: Differential input, 40ohm */
	.dram_sdclk_0		= 0x00020030,
	.dram_sdclk_1		= 0x00020030,
	.dram_cas		= 0x00020030,
	.dram_ras		= 0x00020030,
	.dram_reset		= 0x00020030,
	/* SDCKE[0:1]: 100k pull-up */
	.dram_sdcke0		= 0x00003000,
	.dram_sdcke1		= 0x00003000,
	/* SDBA2: pull-up disabled */
	.dram_sdba2		= 0x00000000,
	/* SDODT[0:1]: 100k pull-up, 40 ohm */
	.dram_sdodt0		= 0x00003030,
	.dram_sdodt1		= 0x00003030,
	/* SDQS[0:7]: Differential input, 40 ohm */
	.dram_sdqs0		= 0x00000030,
	.dram_sdqs1		= 0x00000030,
	.dram_sdqs2		= 0x00000030,
	.dram_sdqs3		= 0x00000030,
	.dram_sdqs4		= 0x00000030,
	.dram_sdqs5		= 0x00000030,
	.dram_sdqs6		= 0x00000030,
	.dram_sdqs7		= 0x00000030,
	/* DQM[0:7]: Differential input, 40 ohm */
	.dram_dqm0		= 0x00020030,
	.dram_dqm1		= 0x00020030,
	.dram_dqm2		= 0x00020030,
	.dram_dqm3		= 0x00020030,
	.dram_dqm4		= 0x00020030,
	.dram_dqm5		= 0x00020030,
	.dram_dqm6		= 0x00020030,
	.dram_dqm7		= 0x00020030,
};

/* configure MX6SOLO/DUALLITE mmdc GRP io registers */
static struct mx6sdl_iomux_grp_regs grp_iomux_s = {
	/* DDR3 */
	.grp_ddr_type		= 0x000c0000,
	.grp_ddrmode_ctl	= 0x00020000,
	/* disable DDR pullups */
	.grp_ddrpke		= 0x00000000,
	/* ADDR[00:16], SDBA[0:1]: 40 ohm */
	.grp_addds		= 0x00000030,
	/* CS0/CS1/SDBA2/CKE0/CKE1/SDWE: 40 ohm */
	.grp_ctlds		= 0x00000030,
	/* DATA[00:63]: Differential input, 40 ohm */
	.grp_ddrmode		= 0x00020000,
	.grp_b0ds		= 0x00000030,
	.grp_b1ds		= 0x00000030,
	.grp_b2ds		= 0x00000030,
	.grp_b3ds		= 0x00000030,
	.grp_b4ds		= 0x00000030,
	.grp_b5ds		= 0x00000030,
	.grp_b6ds		= 0x00000030,
	.grp_b7ds		= 0x00000030,
};

/*
 * DDR3 desriptions - these are the memory chips we support
 */

/* NT5CC128M16FP-DII */
static struct mx6_ddr3_cfg cfg_nt5cc128m16fp_dii = {
	.mem_speed      = 1600,
	.density        = 2,
	.width          = 16,
	.banks          = 8,
	.rowaddr        = 14,
	.coladdr        = 10,
	.pagesz         = 2,
	.trcd           = 1375,
	.trcmin         = 4875,
	.trasmin        = 3500,
};

/* measured on board TSERIES_ARM/1 V_LVDS_DL64 */
static struct mx6_mmdc_calibration cal_nt5cc128m16fp_dii_128x64_s = {
	/* write leveling calibration determine, MR1-value = 0x0002 */
	.p0_mpwldectrl0 = 0x003F003E,
	.p0_mpwldectrl1 = 0x003A003A,
	.p1_mpwldectrl0 = 0x001B001C,
	.p1_mpwldectrl1 = 0x00190031,
	/* Read DQS Gating calibration */
	.p0_mpdgctrl0   = 0x02640264,
	.p0_mpdgctrl1   = 0x02440250,
	.p1_mpdgctrl0   = 0x02400250,
	.p1_mpdgctrl1   = 0x0238023C,
	/* Read Calibration: DQS delay relative to DQ read access */
	.p0_mprddlctl   = 0x40464644,
	.p1_mprddlctl   = 0x464A4842,
	/* Write Calibration: DQ/DM delay relative to DQS write access */
	.p0_mpwrdlctl   = 0x38343034,
	.p1_mpwrdlctl   = 0x36323830,
};

/* measured on board TSERIES_ARM/1 V_LVDS_S32 */
static struct mx6_mmdc_calibration cal_nt5cc128m16fp_dii_128x32_s = {
	/* write leveling calibration determine, MR1-value = 0x0002 */
	.p0_mpwldectrl0 = 0x00410043,
	.p0_mpwldectrl1 = 0x003A003C,
	/* Read DQS Gating calibration */
	.p0_mpdgctrl0   = 0x023C0244,
	.p0_mpdgctrl1   = 0x02240230,
	/* Read Calibration: DQS delay relative to DQ read access */
	.p0_mprddlctl   = 0x484C4A48,
	/* Write Calibration: DQ/DM delay relative to DQS write access */
	.p0_mpwrdlctl   = 0x3C363434,
};

static void spl_dram_init(void)
{
	struct gpio_regs *gpio = (struct gpio_regs *)GPIO2_BASE_ADDR;
	u32 val, dram_strap = 0;
	struct mx6_ddr3_cfg *mem = NULL;
	struct mx6_mmdc_calibration *calib = NULL;
	struct mx6_ddr_sysinfo sysinfo = {
		/* width of data bus:0=16,1=32,2=64 */
		.dsize		= -1,	/* CPU type specific (overwritten) */
		/* config for full 4GB range so that get_mem_size() works */
		.cs_density	= 32,	/* 32Gb per CS */
		.ncs		= 1,	/* single chip select */
		.cs1_mirror	= 0,
		.rtt_wr		= 1,	/* DDR3_RTT_60_OHM, RTT_Wr = RZQ/4 */
		.rtt_nom	= 1,	/* DDR3_RTT_60_OHM, RTT_Nom = RZQ/4 */
		.walat		= 1,	/* Write additional latency */
		.ralat		= 5,	/* Read additional latency */
		.mif3_mode	= 3,	/* Command prediction working mode */
		.bi_on		= 1,	/* Bank interleaving enabled */
		.sde_to_rst	= 0x10,	/* 14 cycles, 200us (JEDEC default) */
		.rst_to_cke	= 0x23,	/* 33 cycles, 500us (JEDEC default) */
		.ddr_type	= 0,	/* DDR3 */
	};

	/*
	 * MMDC Calibration requires the following data:
	 *  mx6_mmdc_calibration - board-specific calibration (routing delays)
	 *     these calibration values depend on board routing, SoC, and DDR
	 *  mx6_ddr_sysinfo - board-specific memory architecture (width/cs/etc)
	 *  mx6_ddr_cfg - chip specific timing/layout details
	 */

	/* setup HWID3-2 to input */
	val = readl(&gpio->gpio_dir);
	val &= ~(0x1 << 0 | 0x1 << 1);
	writel(val, &gpio->gpio_dir);

	/* read DRAM strapping from HWID3/2 (bit 1 and bit 0) */
	dram_strap = readl(&gpio->gpio_psr) & 0x3;

	switch (dram_strap) {
	/* 1 GiB, 64 bit, 4 NT5CC128M16FP chips */
	case 0:
		puts("DRAM strap 00\n");
		mem = &cfg_nt5cc128m16fp_dii;
		sysinfo.dsize = 2;
		calib = &cal_nt5cc128m16fp_dii_128x64_s;
		break;
	/* 512 MiB, 32 bit, 2 NT5CC128M16FP chips */
	case 1:
		puts("DRAM strap 01\n");
		mem = &cfg_nt5cc128m16fp_dii;
		sysinfo.dsize = 1;
		calib = &cal_nt5cc128m16fp_dii_128x32_s;
		break;
	default:
		printf("DRAM strap 0x%x (invalid)\n", dram_strap);
		break;
	}

	if (!mem) {
		puts("Error: Invalid Memory Configuration\n");
		hang();
	}
	if (!calib) {
		puts("Error: Invalid Board Calibration Configuration\n");
		hang();
	}

	mx6sdl_dram_iocfg(16 << sysinfo.dsize,
			  &ddr_iomux_s,
			  &grp_iomux_s);

	mx6_dram_cfg(&sysinfo, calib, mem);
}

static iomux_v3_cfg_t const board_pads_spl[] = {
	/* UART#1 PADS */
	MUXDESC(PAD_CSI0_DAT10__UART1_TX_DATA,	UART_PAD_CTRL),
	MUXDESC(PAD_CSI0_DAT11__UART1_RX_DATA,	UART_PAD_CTRL),
	/* ESCPI#1 PADS */
	MUXDESC(PAD_EIM_D16__ECSPI1_SCLK,	ECSPI_PAD_CTRL),
	MUXDESC(PAD_EIM_D17__ECSPI1_MISO,	ECSPI_PAD_CTRL),
	MUXDESC(PAD_EIM_D18__ECSPI1_MOSI,	ECSPI_PAD_CTRL),
	MUXDESC(PAD_EIM_D19__GPIO3_IO19,	ECSPI_PAD_CTRL),
	/* USDHC#4 PADS */
	MUXDESC(PAD_SD4_CLK__SD4_CLK,		USDHC_PAD_CTRL),
	MUXDESC(PAD_SD4_CMD__SD4_CMD,		USDHC_PAD_CTRL),
	MUXDESC(PAD_SD4_DAT0__SD4_DATA0,	USDHC_PAD_CTRL),
	MUXDESC(PAD_SD4_DAT1__SD4_DATA1,	USDHC_PAD_CTRL),
	MUXDESC(PAD_SD4_DAT2__SD4_DATA2,	USDHC_PAD_CTRL),
	MUXDESC(PAD_SD4_DAT3__SD4_DATA3,	USDHC_PAD_CTRL),
	MUXDESC(PAD_SD4_DAT4__SD4_DATA4,	USDHC_PAD_CTRL),
	MUXDESC(PAD_SD4_DAT5__SD4_DATA5,	USDHC_PAD_CTRL),
	MUXDESC(PAD_SD4_DAT6__SD4_DATA6,	USDHC_PAD_CTRL),
	MUXDESC(PAD_SD4_DAT7__SD4_DATA7,	USDHC_PAD_CTRL),
	/* HWID*/
	MUXDESC(PAD_NANDF_D0__GPIO2_IO00,	GPIO_PAD_CTRL_PU),
	MUXDESC(PAD_NANDF_D1__GPIO2_IO01,	GPIO_PAD_CTRL_PU),
	MUXDESC(PAD_NANDF_D2__GPIO2_IO02,	GPIO_PAD_CTRL_PU),
	MUXDESC(PAD_NANDF_D3__GPIO2_IO03,	GPIO_PAD_CTRL_PU),
};

void spl_board_init(void)
{
	preloader_console_init();
}

static void ccgr_init(void)
{
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	/*
	 * We don't use DMA in SPL, but we do need it in U-Boot. U-Boot
	 * initializes DMA very early (before all board code), so the only
	 * opportunity we have to initialize APBHDMA clocks is in SPL.
	 * setbits_le32(&ccm->CCGR0, MXC_CCM_CCGR0_APBHDMA_MASK);
	 */

	writel(0x00C03F3F, &ccm->CCGR0);
	writel(0x00F0FC03, &ccm->CCGR1);
	writel(0x0FFFF000, &ccm->CCGR2);
	writel(0x3FF00000, &ccm->CCGR3);
	writel(0x00FFF300, &ccm->CCGR4);
	writel(0x0F0030C3, &ccm->CCGR5);
	writel(0x000003F0, &ccm->CCGR6);
}

void board_init_f(ulong dummy)
{
	ccgr_init();
	arch_cpu_init();
	timer_init();
	gpr_init();

	SETUP_IOMUX_PADS(board_pads_spl);
	spl_dram_init();
}

void reset_cpu(void)
{
}
#endif /* CONFIG_SPL_BUILD */
