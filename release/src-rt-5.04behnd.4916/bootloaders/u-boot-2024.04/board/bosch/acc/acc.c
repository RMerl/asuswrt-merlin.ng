// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2017 DENX Software Engineering GmbH, Heiko Schocher <hs@denx.de>
 * Copyright (c) 2019 Bosch Thermotechnik GmbH
 * Copyright (c) 2022 DENX Software Engineering GmbH, Philip Oberfichtner <pro@denx.de>
 */

#include <common.h>
#include <cpu_func.h>
#include <bootstage.h>
#include <dm.h>
#include <dm/platform_data/serial_mxc.h>
#include <dm/device-internal.h>
#include <env.h>
#include <hang.h>
#include <init.h>
#include <linux/delay.h>
#include <mmc.h>

#include <asm/io.h>
#include <asm/gpio.h>
#include <linux/sizes.h>

#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/sections.h>
#include <usb.h>
#include <usb/ehci-ci.h>
#include <fuse.h>

#include <watchdog.h>

DECLARE_GLOBAL_DATA_PTR;

#define GPIO_ACC_PLAT_DETECT     IMX_GPIO_NR(5, 9)
#define GPIO_ACC_RAM_VOLT_DETECT IMX_GPIO_NR(5, 0)
#define GPIO_BUZZER              IMX_GPIO_NR(1, 18)
#define GPIO_LAN1_RESET          IMX_GPIO_NR(4, 27)
#define GPIO_LAN2_RESET          IMX_GPIO_NR(4, 19)
#define GPIO_LAN3_RESET          IMX_GPIO_NR(4, 18)
#define GPIO_USB_HUB_RESET       IMX_GPIO_NR(5, 5)
#define GPIO_EXP_RS485_RESET     IMX_GPIO_NR(4, 16)
#define GPIO_TOUCH_RESET         IMX_GPIO_NR(1, 20)

#define BOARD_INFO_MAGIC 0x19730517

struct board_info {
	int magic;
	int board;
	int rev;
};

static struct board_info *detect_board(void);

#define PFID_BOARD_ACC 0xe

static const char * const name_board[] = {
	[PFID_BOARD_ACC] = "ACC",
};

#define PFID_REV_22 0x8
#define PFID_REV_21 0x9
#define PFID_REV_20 0xa
#define PFID_REV_14 0xb
#define PFID_REV_13 0xc
#define PFID_REV_12 0xd
#define PFID_REV_11 0xe
#define PFID_REV_10 0xf

static const char * const name_revision[] = {
	[0 ... PFID_REV_10] = "Unknown",
	[PFID_REV_10] = "1.0",
	[PFID_REV_11] = "1.1",
	[PFID_REV_12] = "1.2",
	[PFID_REV_13] = "1.3",
	[PFID_REV_14] = "1.4",
	[PFID_REV_20] = "2.0",
	[PFID_REV_21] = "2.1",
	[PFID_REV_22] = "2.2",
};

/*
 * NXP Reset Default: 0x0001B0B0
 * - Schmitt trigger input (PAD_CTL_HYS)
 * - 100K Ohm Pull Up (PAD_CTL_PUS_100K_UP)
 * - Pull Enabled (PAD_CTL_PUE)
 * - Pull/Keeper Enabled (PAD_CTL_PKE)
 * - CMOS output (No PAD_CTL_ODE)
 * - Medium Speed (PAD_CTL_SPEED_MED)
 * - 40 Ohm drive strength (PAD_CTL_DSE_40ohm)
 * - Slow (PAD_CTL_SRE_SLOW)
 */

/* Input, no pull up/down: 0x0x000100B0 */
#define GPIN_PAD_CTRL (PAD_CTL_HYS \
		| PAD_CTL_SPEED_MED \
		| PAD_CTL_DSE_40ohm \
		| PAD_CTL_SRE_SLOW)

/* Input, pull up: 0x0x0001B0B0 */
#define GPIN_PU_PAD_CTRL (PAD_CTL_HYS \
		| PAD_CTL_PUS_100K_UP \
		| PAD_CTL_PUE \
		| PAD_CTL_PKE \
		| PAD_CTL_SPEED_MED \
		| PAD_CTL_DSE_40ohm \
		| PAD_CTL_SRE_SLOW)

/* Input, pull down: 0x0x000130B0 */
#define GPIN_PD_PAD_CTRL (PAD_CTL_HYS \
		| PAD_CTL_PUS_100K_DOWN \
		| PAD_CTL_PUE \
		| PAD_CTL_PKE \
		| PAD_CTL_SPEED_MED \
		| PAD_CTL_DSE_40ohm \
		| PAD_CTL_SRE_SLOW)

static const iomux_v3_cfg_t board_detect_pads[] = {
	/* Platform detect */
	IOMUX_PADS(PAD_DISP0_DAT15__GPIO5_IO09 | MUX_PAD_CTRL(GPIN_PAD_CTRL)),
	/* RAM Volt detect */
	IOMUX_PADS(PAD_EIM_WAIT__GPIO5_IO00 | MUX_PAD_CTRL(GPIN_PAD_CTRL)),
	/* PFID 0..9 */
	IOMUX_PADS(PAD_NANDF_D0__GPIO2_IO00 | MUX_PAD_CTRL(GPIN_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D1__GPIO2_IO01 | MUX_PAD_CTRL(GPIN_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D2__GPIO2_IO02 | MUX_PAD_CTRL(GPIN_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D3__GPIO2_IO03 | MUX_PAD_CTRL(GPIN_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D4__GPIO2_IO04 | MUX_PAD_CTRL(GPIN_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D5__GPIO2_IO05 | MUX_PAD_CTRL(GPIN_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D6__GPIO2_IO06 | MUX_PAD_CTRL(GPIN_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D7__GPIO2_IO07 | MUX_PAD_CTRL(GPIN_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_CS3__GPIO6_IO16 | MUX_PAD_CTRL(GPIN_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_CS1__GPIO6_IO14 | MUX_PAD_CTRL(GPIN_PAD_CTRL)),
	/* Manufacturer */
	IOMUX_PADS(PAD_EIM_A24__GPIO5_IO04 | MUX_PAD_CTRL(GPIN_PAD_CTRL)),
	/* Redundant */
	IOMUX_PADS(PAD_NANDF_CS2__GPIO6_IO15 | MUX_PAD_CTRL(GPIN_PU_PAD_CTRL))
};

static int gpio_acc_pfid[] = {
	IMX_GPIO_NR(2, 0),
	IMX_GPIO_NR(2, 1),
	IMX_GPIO_NR(2, 2),
	IMX_GPIO_NR(2, 3),
	IMX_GPIO_NR(2, 4),
	IMX_GPIO_NR(6, 14),
	IMX_GPIO_NR(6, 15),
	IMX_GPIO_NR(2, 5),
	IMX_GPIO_NR(2, 6),
	IMX_GPIO_NR(2, 7),
	IMX_GPIO_NR(6, 16),
	IMX_GPIO_NR(5, 4),
};

static int init_gpio(int nr)
{
	int ret;

	ret = gpio_request(nr, "");
	if (ret != 0) {
		printf("Could not request gpio nr: %d\n", nr);
		hang();
	}
	ret = gpio_direction_input(nr);
	if (ret != 0) {
		printf("Could not set gpio nr: %d to input\n", nr);
		hang();
	}
	return 0;
}

/*
 * We want to detect the board type only once in SPL,
 * so we store the board_info struct at beginning in IRAM.
 *
 * U-Boot itself can read it also, and do not need again
 * to detect board type.
 *
 */
static struct board_info *detect_board(void)
{
	struct board_info *binfo = (struct board_info *)IRAM_BASE_ADDR;
	int i;

	if (binfo->magic == BOARD_INFO_MAGIC)
		return binfo;

	puts("Board: ");
	SETUP_IOMUX_PADS(board_detect_pads);
	init_gpio(GPIO_ACC_PLAT_DETECT);
	if (gpio_get_value(GPIO_ACC_PLAT_DETECT)) {
		puts("not supported");
		hang();
	} else {
		puts("Bosch ");
	}

	for (i = 0; i < sizeof(gpio_acc_pfid) / sizeof(int); i++)
		init_gpio(gpio_acc_pfid[i]);

	binfo->board = gpio_get_value(gpio_acc_pfid[0]) << 0 |
	    gpio_get_value(gpio_acc_pfid[1]) << 1 |
	    gpio_get_value(gpio_acc_pfid[2]) << 2 |
	    gpio_get_value(gpio_acc_pfid[11]) << 3;
	printf("%s ", name_board[binfo->board]);

	binfo->rev = gpio_get_value(gpio_acc_pfid[7]) << 0 |
	    gpio_get_value(gpio_acc_pfid[8]) << 1 |
	    gpio_get_value(gpio_acc_pfid[9]) << 2 |
	    gpio_get_value(gpio_acc_pfid[10]) << 3;
	printf("rev: %s\n", name_revision[binfo->rev]);

	binfo->magic = BOARD_INFO_MAGIC;

	return binfo;
}

static void unset_early_gpio(void)
{
	init_gpio(GPIO_LAN1_RESET);
	init_gpio(GPIO_LAN2_RESET);
	init_gpio(GPIO_LAN3_RESET);
	init_gpio(GPIO_USB_HUB_RESET);
	init_gpio(GPIO_EXP_RS485_RESET);
	init_gpio(GPIO_TOUCH_RESET);

	gpio_set_value(GPIO_LAN1_RESET, 1);
	gpio_set_value(GPIO_LAN2_RESET, 1);
	gpio_set_value(GPIO_LAN3_RESET, 1);
	gpio_set_value(GPIO_USB_HUB_RESET, 1);
	gpio_set_value(GPIO_EXP_RS485_RESET, 1);
	gpio_set_value(GPIO_TOUCH_RESET, 1);
}

int board_late_init(void)
{
	struct board_info *binfo = detect_board();

	switch (binfo->board) {
	case PFID_BOARD_ACC:
		env_set("bootconf", "conf-imx6q-bosch-acc.dtb");
		break;
	default:
		printf("Unknown board %d\n", binfo->board);
		break;
	}

	unset_early_gpio();

	return 0;
}

int board_init(void)
{
	/* Address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	return 0;
}

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();

	return 0;
}

#if IS_ENABLED(CONFIG_SPL_BUILD)
#include <asm/arch/crm_regs.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-ddr.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <spl.h>

/* Early
 * - Buzzer -> GPIO IN, Pull-Down (PWM enabled by Kernel later-on, lacks of an
 *   external pull-down resistor)
 * - Touch clean reset on every boot
 * - Ethernet(s), USB Hub, Expansion RS485 -> Clean reset on each u-boot init
 */
static const iomux_v3_cfg_t early_pads[] = {
	IOMUX_PADS(PAD_SD1_CMD__GPIO1_IO18 | MUX_PAD_CTRL(GPIN_PD_PAD_CTRL)), /* Buzzer PWM */
	IOMUX_PADS(PAD_DISP0_DAT6__GPIO4_IO27 | MUX_PAD_CTRL(GPIN_PD_PAD_CTRL)), /* #FEC_RESET_B */
	IOMUX_PADS(PAD_DI0_PIN2__GPIO4_IO18 | MUX_PAD_CTRL(GPIN_PD_PAD_CTRL)), /* #ETH1_RESET */
	IOMUX_PADS(PAD_DI0_PIN3__GPIO4_IO19 | MUX_PAD_CTRL(GPIN_PD_PAD_CTRL)), /* #ETH2_RESET */
	IOMUX_PADS(PAD_DISP0_DAT11__GPIO5_IO05 | MUX_PAD_CTRL(GPIN_PD_PAD_CTRL)), /* #USB Reset */
	IOMUX_PADS(PAD_DI0_DISP_CLK__GPIO4_IO16 | MUX_PAD_CTRL(GPIN_PD_PAD_CTRL)), /* #UART_RESET */
	IOMUX_PADS(PAD_SD1_CLK__GPIO1_IO20 | MUX_PAD_CTRL(GPIN_PD_PAD_CTRL)), /* #CTOUCH_RESET */
};

static void setup_iomux_early(void)
{
	SETUP_IOMUX_PADS(early_pads);
}

static void set_early_gpio(void)
{
	init_gpio(GPIO_BUZZER);
	init_gpio(GPIO_LAN1_RESET);
	init_gpio(GPIO_LAN2_RESET);
	init_gpio(GPIO_LAN3_RESET);
	init_gpio(GPIO_USB_HUB_RESET);
	init_gpio(GPIO_EXP_RS485_RESET);
	init_gpio(GPIO_TOUCH_RESET);

	/* Reset signals are active low */
	gpio_set_value(GPIO_BUZZER, 0);
	gpio_set_value(GPIO_LAN1_RESET, 0);
	gpio_set_value(GPIO_LAN2_RESET, 0);
	gpio_set_value(GPIO_LAN3_RESET, 0);
	gpio_set_value(GPIO_USB_HUB_RESET, 0);
	gpio_set_value(GPIO_EXP_RS485_RESET, 0);
	gpio_set_value(GPIO_TOUCH_RESET, 0);
}

/* UART */
#define UART_PAD_CTRL \
		(PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | \
		PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#undef UART_PAD_CTRL
#define UART_PAD_CTRL 0x1b0b1
static const iomux_v3_cfg_t uart2_pads[] = {
	IOMUX_PADS(PAD_SD3_DAT4__UART2_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT5__UART2_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_CMD__UART2_CTS_B | MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_CLK__UART2_RTS_B | MUX_PAD_CTRL(UART_PAD_CTRL)),
};

static void setup_iomux_uart(void)
{
	SETUP_IOMUX_PADS(uart2_pads);
}

void spl_board_init(void)
{
}

static const struct mx6dq_iomux_ddr_regs acc_mx6d_ddr_ioregs = {
	.dram_sdclk_0 = 0x00008038,
	.dram_sdclk_1 = 0x00008038,
	.dram_cas = 0x00008028,
	.dram_ras = 0x00008028,
	.dram_reset = 0x00000028,
	.dram_sdcke0 = 0x00003000,
	.dram_sdcke1 = 0x00003000,
	.dram_sdba2 = 0x00008000,
	.dram_sdodt0 = 0x00000028,
	.dram_sdodt1 = 0x00000028,
	.dram_sdqs0 = 0x00008038,
	.dram_sdqs1 = 0x00008038,
	.dram_sdqs2 = 0x00008038,
	.dram_sdqs3 = 0x00008038,
	.dram_sdqs4 = 0x00008038,
	.dram_sdqs5 = 0x00008038,
	.dram_sdqs6 = 0x00008038,
	.dram_sdqs7 = 0x00008038,
	.dram_dqm0 = 0x00008038,
	.dram_dqm1 = 0x00008038,
	.dram_dqm2 = 0x00008038,
	.dram_dqm3 = 0x00008038,
	.dram_dqm4 = 0x00008038,
	.dram_dqm5 = 0x00008038,
	.dram_dqm6 = 0x00008038,
	.dram_dqm7 = 0x00008038,
};

static const struct mx6dq_iomux_grp_regs acc_mx6d_grp_ioregs = {
	.grp_ddr_type = 0x000C0000,
	.grp_ddrmode_ctl = 0x00020000,
	.grp_ddrpke = 0x00000000,
	.grp_addds = 0x00000030,
	.grp_ctlds = 0x00000028,
	.grp_ddrmode = 0x00020000,
	.grp_b0ds = 0x00000038,
	.grp_b1ds = 0x00000038,
	.grp_b2ds = 0x00000038,
	.grp_b3ds = 0x00000038,
	.grp_b4ds = 0x00000038,
	.grp_b5ds = 0x00000038,
	.grp_b6ds = 0x00000038,
	.grp_b7ds = 0x00000038,
};

static const struct mx6_mmdc_calibration acc_mx6d_mmdc_calib = {
	.p0_mpwldectrl0 = 0x0020001F,
	.p0_mpwldectrl1 = 0x00280021,
	.p1_mpwldectrl0 = 0x00120028,
	.p1_mpwldectrl1 = 0x000D001F,
	.p0_mpdgctrl0 = 0x43340342,
	.p0_mpdgctrl1 = 0x03300325,
	.p1_mpdgctrl0 = 0x4334033E,
	.p1_mpdgctrl1 = 0x03280270,
	.p0_mprddlctl = 0x46373B3E,
	.p1_mprddlctl = 0x3B383544,
	.p0_mpwrdlctl = 0x36383E40,
	.p1_mpwrdlctl = 0x4030433A,
};

/* Micron MT41K128M16JT-125 (standard - 1600,CL=11)
 * !!! i.MX6 does NOT support data rates higher than DDR3-1066 !!!
 * So this setting is actually invalid!
 *
static const struct mx6_ddr3_cfg acc_mx6d_mem_ddr3_1600 = {
	.mem_speed = 1600,
	.density = 2,
	.width = 16,
	.banks = 8,
	.rowaddr = 14,
	.coladdr = 10,
	.pagesz = 2,
	.trcd = 1375,
	.trcmin = 4875,
	.trasmin = 3500,
	.SRT = 0,
};
 */

/* Micron MT41K128M16JT-125 is backward-compatible with 1333,CL=9 (-15E) and 1066,CL=7 (-187E)
 * Lowering to 1066 saves on ACC ~0.25 Watt at DC In with negligible performance loss
 * width set to 64, as four chips are used on acc (4 * 16 = 64)
 */
static const struct mx6_ddr3_cfg acc_mx6d_mem_ddr3_1066 = {
	.mem_speed = 1066,
	.density = 2,
	.width = 64,
	.banks = 8,
	.rowaddr = 14,
	.coladdr = 10,
	.pagesz = 2,
	.trcd = 1313, // 13.125ns
	.trcmin = 5063, // 50.625ns
	.trasmin = 3750, // 37.5ns
	.SRT = 0, // Set to 1 for temperatures above 85°C
};

static const struct mx6_ddr_sysinfo acc_mx6d_ddr_info = {
	.ddr_type = DDR_TYPE_DDR3,
	/* width of data bus:0=16,1=32,2=64 */
	.dsize = 2,
	.cs_density = 32,	/* 32Gb per CS */
	.ncs = 1,		/* single chip select */
	.cs1_mirror = 0,
	.rtt_wr = 1,		/* DDR3_RTT_60_OHM, RTT_Wr = RZQ/4 */
	.rtt_nom = 1,		/* DDR3_RTT_60_OHM, RTT_Nom = RZQ/4 */
	.walat = 0,		/* Write additional latency */
	.ralat = 5,		/* Read additional latency */
	.mif3_mode = 3,		/* Command prediction working mode */
	.bi_on = 1,		/* Bank interleaving enabled */
	.sde_to_rst = 0x33,	/* 14 cycles, 200us (JEDEC default) */
	.rst_to_cke = 0x33,	/* 33 cycles, 500us (JEDEC default) */
};

#define ACC_SPREAD_SPECTRUM_STOP	0x0fa
#define ACC_SPREAD_SPECTRUM_STEP	0x001
#define ACC_SPREAD_SPECTRUM_DENOM	0x190

static void ccgr_init(void)
{
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	/* Turn clocks on/off */
	writel(0x00C0000F, &ccm->CCGR0);
	writel(0x0030FC00, &ccm->CCGR1);
	writel(0x03FF0033, &ccm->CCGR2);
	writel(0x3FF3300F, &ccm->CCGR3);
	writel(0x0003C300, &ccm->CCGR4);
	writel(0x0F3000C3, &ccm->CCGR5);
	writel(0x00000FFF, &ccm->CCGR6);

	/* Enable spread spectrum */
	writel(BM_ANADIG_PLL_528_SS_ENABLE |
	       BF_ANADIG_PLL_528_SS_STOP(ACC_SPREAD_SPECTRUM_STOP) |
	       BF_ANADIG_PLL_528_SS_STEP(ACC_SPREAD_SPECTRUM_STEP),
	       &ccm->analog_pll_528_ss);

	writel(BF_ANADIG_PLL_528_DENOM_B(ACC_SPREAD_SPECTRUM_DENOM),
	       &ccm->analog_pll_528_denom);
}

/* MMC board initialization is needed till adding DM support in SPL */
#if IS_ENABLED(CONFIG_FSL_ESDHC_IMX) && !IS_ENABLED(CONFIG_DM_MMC)
#include <mmc.h>
#include <fsl_esdhc_imx.h>

static const iomux_v3_cfg_t usdhc2_pads[] = {
	IOMUX_PADS(PAD_SD2_CMD__SD2_CMD | MUX_PAD_CTRL(0x00017069)),
	IOMUX_PADS(PAD_SD2_CLK__SD2_CLK | MUX_PAD_CTRL(0x00010038)),
	IOMUX_PADS(PAD_SD2_DAT0__SD2_DATA0 | MUX_PAD_CTRL(0x00017069)),
	IOMUX_PADS(PAD_SD2_DAT1__SD2_DATA1 | MUX_PAD_CTRL(0x00017069)),
	IOMUX_PADS(PAD_SD2_DAT2__SD2_DATA2 | MUX_PAD_CTRL(0x00017069)),
	IOMUX_PADS(PAD_SD2_DAT3__SD2_DATA3 | MUX_PAD_CTRL(0x00017069)),
	IOMUX_PADS(PAD_GPIO_4__GPIO1_IO04 | MUX_PAD_CTRL(0x0001B0B0)),	/* CD */
};

static const iomux_v3_cfg_t usdhc4_pads[] = {
	IOMUX_PADS(PAD_SD4_CMD__SD4_CMD | MUX_PAD_CTRL(0x00017059)),
	IOMUX_PADS(PAD_SD4_CLK__SD4_CLK | MUX_PAD_CTRL(0x00010059)),
	IOMUX_PADS(PAD_SD4_DAT0__SD4_DATA0 | MUX_PAD_CTRL(0x00017059)),
	IOMUX_PADS(PAD_SD4_DAT1__SD4_DATA1 | MUX_PAD_CTRL(0x00017059)),
	IOMUX_PADS(PAD_SD4_DAT2__SD4_DATA2 | MUX_PAD_CTRL(0x00017059)),
	IOMUX_PADS(PAD_SD4_DAT3__SD4_DATA3 | MUX_PAD_CTRL(0x00017059)),
	IOMUX_PADS(PAD_SD4_DAT4__SD4_DATA4 | MUX_PAD_CTRL(0x00017059)),
	IOMUX_PADS(PAD_SD4_DAT5__SD4_DATA5 | MUX_PAD_CTRL(0x00017059)),
	IOMUX_PADS(PAD_SD4_DAT6__SD4_DATA6 | MUX_PAD_CTRL(0x00017059)),
	IOMUX_PADS(PAD_SD4_DAT7__SD4_DATA7 | MUX_PAD_CTRL(0x00017059)),
};

struct fsl_esdhc_cfg usdhc_cfg[2] = {
	{USDHC2_BASE_ADDR, 1, 4},
	{USDHC4_BASE_ADDR, 1, 8},
};

#define USDHC2_CD_GPIO	IMX_GPIO_NR(1, 4)

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	detect_board();

	switch (cfg->esdhc_base) {
	case USDHC2_BASE_ADDR:
		return !gpio_get_value(USDHC2_CD_GPIO);
	case USDHC4_BASE_ADDR:
		return 1;	/* eMMC always present */
	}

	return ret;
}

int board_mmc_init(struct bd_info *bis)
{
	int i, ret;

	gpio_direction_input(USDHC2_CD_GPIO);
	/*
	 * According to the board_mmc_init() the following map is done:
	 * (U-Boot device node) (Physical Port)
	 * mmc0 USDHC2
	 * mmc1 USDHC4
	 */
	for (i = 0; i < CFG_SYS_FSL_USDHC_NUM; i++) {
		switch (i) {
		case 0:
			SETUP_IOMUX_PADS(usdhc2_pads);
			usdhc_cfg[i].sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
			break;
		case 1:
			SETUP_IOMUX_PADS(usdhc4_pads);
			usdhc_cfg[i].sdhc_clk = mxc_get_clock(MXC_ESDHC4_CLK);
			break;
		default:
			printf("Warning - USDHC%d controller not supporting\n",
			       i + 1);
			return 0;
		}

		ret = fsl_esdhc_initialize(bis, &usdhc_cfg[i]);
		if (ret) {
			printf("Warning: failed to initialize mmc dev %d\n", i);
			return ret;
		}
	}

	return 0;
}
#endif

void board_boot_order(u32 *spl_boot_list)
{
	u32 bmode = imx6_src_get_boot_mode();
	u8 boot_dev = BOOT_DEVICE_MMC1;

	detect_board();

	switch ((bmode & IMX6_BMODE_MASK) >> IMX6_BMODE_SHIFT) {
	case IMX6_BMODE_SD:
	case IMX6_BMODE_ESD:
		/* SD/eSD - BOOT_DEVICE_MMC1 */
		if (IS_ENABLED(CONFIG_SYS_BOOT_EMMC)) {
			/*
			 * boot from SD is not allowed, if boot from eMMC is
			 * configured.
			 */
			puts("SD boot not allowed\n");
			spl_boot_list[0] = BOOT_DEVICE_NONE;
			return;
		}

		boot_dev = BOOT_DEVICE_MMC1;
		break;

	case IMX6_BMODE_MMC:
	case IMX6_BMODE_EMMC:
		/* MMC/eMMC */
		boot_dev = BOOT_DEVICE_MMC2;
		break;
	default:
		/* Default - BOOT_DEVICE_MMC1 */
		printf("Wrong board boot order\n");
		break;
	}

	spl_boot_list[0] = boot_dev;
}

static void setup_ddr(void)
{
	struct board_info *binfo = detect_board();

	switch (binfo->rev) {
	case PFID_REV_20:
	case PFID_REV_21:
	case PFID_REV_22:
	default:
		/* Rev 2 board has i.MX6 Dual with 64-bit RAM */
		mx6dq_dram_iocfg(acc_mx6d_mem_ddr3_1066.width,
				 &acc_mx6d_ddr_ioregs,
				 &acc_mx6d_grp_ioregs);
		mx6_dram_cfg(&acc_mx6d_ddr_info, &acc_mx6d_mmdc_calib,
			     &acc_mx6d_mem_ddr3_1066);
		/* Perform DDR DRAM calibration */
		udelay(100);
		mmdc_do_write_level_calibration(&acc_mx6d_ddr_info);
		mmdc_do_dqs_calibration(&acc_mx6d_ddr_info);
		break;
	}
}

void board_init_f(ulong dummy)
{
	/* setup AIPS and disable watchdog power-down counter (only enabled after reset) */
	arch_cpu_init();

	ccgr_init();
	gpr_init();

	/* setup GP timer */
	timer_init();

	/* Enable device tree and early DM support*/
	spl_early_init();

	/* Setup early required pinmuxes */
	setup_iomux_early();
	set_early_gpio();

	/* Setup UART pinmux */
	setup_iomux_uart();

	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();

	setup_ddr();

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	/* load/boot image from boot device */
	board_init_r(NULL, 0);
}
#endif

#if IS_ENABLED(CONFIG_USB_EHCI_MX6)
#define USB_OTHERREGS_OFFSET	0x800
#define UCTRL_PWR_POL		BIT(9)

int board_usb_phy_mode(int port)
{
	if (port == 1)
		return USB_INIT_HOST;
	else
		return usb_phy_mode(port);
}

int board_ehci_hcd_init(int port)
{
	u32 *usbnc_usb_ctrl;

	if (port > 1)
		return -EINVAL;

	usbnc_usb_ctrl = (u32 *)(USB_BASE_ADDR + USB_OTHERREGS_OFFSET +
				  port * 4);

	/* Set Power polarity */
	setbits_le32(usbnc_usb_ctrl, UCTRL_PWR_POL);

	return 0;
}
#endif

int board_fit_config_name_match(const char *name)
{
	if (!strcmp(name, "imx6q-bosch-acc"))
		return 0;
	return -1;
}

void reset_cpu(void)
{
	puts("Hanging CPU for watchdog reset!\n");
	hang();
}

#if CONFIG_IS_ENABLED(SHOW_BOOT_PROGRESS)
void show_boot_progress(int val)
{
	u32 fuseval;
	int ret;

	if (val < 0)
		val *= -1;

	switch (val) {
	case BOOTSTAGE_ID_ENTER_CLI_LOOP:
		printf("autoboot failed, check fuse\n");
		ret = fuse_read(0, 6, &fuseval);
		if (ret == 0 && (fuseval & 0x2) == 0x0) {
			printf("Enter cmdline, as device not closed\n");
			return;
		}
		ret = fuse_read(5, 7, &fuseval);
		if (ret == 0 && fuseval == 0x0) {
			printf("Enter cmdline, as it is a Development device\n");
			return;
		}
		panic("do not enter cmdline");
		break;
	}
}
#endif
