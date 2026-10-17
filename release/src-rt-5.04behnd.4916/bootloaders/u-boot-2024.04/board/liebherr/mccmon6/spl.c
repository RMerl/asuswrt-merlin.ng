// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014 Wandboard
 * Author: Tungyi Lin <tungyilin1127@gmail.com>
 *         Richard Hu <hakahu@gmail.com>
 */

#include <image.h>
#include <init.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <errno.h>
#include <asm/gpio.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/video.h>
#include <mmc.h>
#include <fsl_esdhc_imx.h>
#include <asm/arch/crm_regs.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <serial.h>
#include <spl.h>
#include <linux/delay.h>

#include <asm/arch/mx6-ddr.h>
/*
 * Driving strength:
 *   0x30 == 40 Ohm
 *   0x28 == 48 Ohm
 */

#define IMX6DQ_DRIVE_STRENGTH		0x30
#define IMX6SDL_DRIVE_STRENGTH		0x28

/* configure MX6Q/DUAL mmdc DDR io registers */
static struct mx6dq_iomux_ddr_regs mx6dq_ddr_ioregs = {
	.dram_sdclk_0 = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdclk_1 = IMX6DQ_DRIVE_STRENGTH,
	.dram_cas = IMX6DQ_DRIVE_STRENGTH,
	.dram_ras = IMX6DQ_DRIVE_STRENGTH,
	.dram_reset = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdcke0 = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdcke1 = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdba2 = 0x00000000,
	.dram_sdodt0 = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdodt1 = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdqs0 = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdqs1 = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdqs2 = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdqs3 = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdqs4 = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdqs5 = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdqs6 = IMX6DQ_DRIVE_STRENGTH,
	.dram_sdqs7 = IMX6DQ_DRIVE_STRENGTH,
	.dram_dqm0 = IMX6DQ_DRIVE_STRENGTH,
	.dram_dqm1 = IMX6DQ_DRIVE_STRENGTH,
	.dram_dqm2 = IMX6DQ_DRIVE_STRENGTH,
	.dram_dqm3 = IMX6DQ_DRIVE_STRENGTH,
	.dram_dqm4 = IMX6DQ_DRIVE_STRENGTH,
	.dram_dqm5 = IMX6DQ_DRIVE_STRENGTH,
	.dram_dqm6 = IMX6DQ_DRIVE_STRENGTH,
	.dram_dqm7 = IMX6DQ_DRIVE_STRENGTH,
};

/* configure MX6Q/DUAL mmdc GRP io registers */
static struct mx6dq_iomux_grp_regs mx6dq_grp_ioregs = {
	.grp_ddr_type = 0x000c0000,
	.grp_ddrmode_ctl = 0x00020000,
	.grp_ddrpke = 0x00000000,
	.grp_addds = IMX6DQ_DRIVE_STRENGTH,
	.grp_ctlds = IMX6DQ_DRIVE_STRENGTH,
	.grp_ddrmode = 0x00020000,
	.grp_b0ds = IMX6DQ_DRIVE_STRENGTH,
	.grp_b1ds = IMX6DQ_DRIVE_STRENGTH,
	.grp_b2ds = IMX6DQ_DRIVE_STRENGTH,
	.grp_b3ds = IMX6DQ_DRIVE_STRENGTH,
	.grp_b4ds = IMX6DQ_DRIVE_STRENGTH,
	.grp_b5ds = IMX6DQ_DRIVE_STRENGTH,
	.grp_b6ds = IMX6DQ_DRIVE_STRENGTH,
	.grp_b7ds = IMX6DQ_DRIVE_STRENGTH,
};

/* configure MX6SOLO/DUALLITE mmdc DDR io registers */
struct mx6sdl_iomux_ddr_regs mx6sdl_ddr_ioregs = {
	.dram_sdclk_0 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdclk_1 = IMX6SDL_DRIVE_STRENGTH,
	.dram_cas = IMX6SDL_DRIVE_STRENGTH,
	.dram_ras = IMX6SDL_DRIVE_STRENGTH,
	.dram_reset = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdcke0 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdcke1 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdba2 = 0x00000000,
	.dram_sdodt0 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdodt1 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdqs0 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdqs1 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdqs2 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdqs3 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdqs4 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdqs5 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdqs6 = IMX6SDL_DRIVE_STRENGTH,
	.dram_sdqs7 = IMX6SDL_DRIVE_STRENGTH,
	.dram_dqm0 = IMX6SDL_DRIVE_STRENGTH,
	.dram_dqm1 = IMX6SDL_DRIVE_STRENGTH,
	.dram_dqm2 = IMX6SDL_DRIVE_STRENGTH,
	.dram_dqm3 = IMX6SDL_DRIVE_STRENGTH,
	.dram_dqm4 = IMX6SDL_DRIVE_STRENGTH,
	.dram_dqm5 = IMX6SDL_DRIVE_STRENGTH,
	.dram_dqm6 = IMX6SDL_DRIVE_STRENGTH,
	.dram_dqm7 = IMX6SDL_DRIVE_STRENGTH,
};

/* configure MX6SOLO/DUALLITE mmdc GRP io registers */
struct mx6sdl_iomux_grp_regs mx6sdl_grp_ioregs = {
	.grp_ddr_type = 0x000c0000,
	.grp_ddrmode_ctl = 0x00020000,
	.grp_ddrpke = 0x00000000,
	.grp_addds = IMX6SDL_DRIVE_STRENGTH,
	.grp_ctlds = IMX6SDL_DRIVE_STRENGTH,
	.grp_ddrmode = 0x00020000,
	.grp_b0ds = IMX6SDL_DRIVE_STRENGTH,
	.grp_b1ds = IMX6SDL_DRIVE_STRENGTH,
	.grp_b2ds = IMX6SDL_DRIVE_STRENGTH,
	.grp_b3ds = IMX6SDL_DRIVE_STRENGTH,
	.grp_b4ds = IMX6SDL_DRIVE_STRENGTH,
	.grp_b5ds = IMX6SDL_DRIVE_STRENGTH,
	.grp_b6ds = IMX6SDL_DRIVE_STRENGTH,
	.grp_b7ds = IMX6SDL_DRIVE_STRENGTH,
};

/* H5T04G63AFR-PB */
static struct mx6_ddr3_cfg h5t04g63afr = {
	.mem_speed = 1600,
	.density = 4,
	.width = 16,
	.banks = 8,
	.rowaddr = 15,
	.coladdr = 10,
	.pagesz = 2,
	.trcd = 1375,
	.trcmin = 4875,
	.trasmin = 3500,
};

/* H5TQ2G63DFR-H9 */
static struct mx6_ddr3_cfg h5tq2g63dfr = {
	.mem_speed = 1333,
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

static struct mx6_mmdc_calibration mx6q_2g_mmdc_calib = {
	.p0_mpwldectrl0 = 0x001f001f,
	.p0_mpwldectrl1 = 0x001f001f,
	.p1_mpwldectrl0 = 0x001f001f,
	.p1_mpwldectrl1 = 0x001f001f,
	.p0_mpdgctrl0 = 0x4301030d,
	.p0_mpdgctrl1 = 0x03020277,
	.p1_mpdgctrl0 = 0x4300030a,
	.p1_mpdgctrl1 = 0x02780248,
	.p0_mprddlctl = 0x4536393b,
	.p1_mprddlctl = 0x36353441,
	.p0_mpwrdlctl = 0x41414743,
	.p1_mpwrdlctl = 0x462f453f,
};

/* DDR 64bit 2GB */
static struct mx6_ddr_sysinfo mem_q = {
	.dsize		= 2,
	.cs1_mirror	= 0,
	/* config for full 4GB range so that get_mem_size() works */
	.cs_density	= 32,
	.ncs		= 1,
	.bi_on		= 1,
	.rtt_nom	= 1,
	.rtt_wr		= 0,
	.ralat		= 5,
	.walat		= 0,
	.mif3_mode	= 3,
	.rst_to_cke	= 0x23,
	.sde_to_rst	= 0x10,
};

static struct mx6_mmdc_calibration mx6dl_1g_mmdc_calib = {
	.p0_mpwldectrl0 = 0x001f001f,
	.p0_mpwldectrl1 = 0x001f001f,
	.p1_mpwldectrl0 = 0x001f001f,
	.p1_mpwldectrl1 = 0x001f001f,
	.p0_mpdgctrl0 = 0x420e020e,
	.p0_mpdgctrl1 = 0x02000200,
	.p1_mpdgctrl0 = 0x42020202,
	.p1_mpdgctrl1 = 0x01720172,
	.p0_mprddlctl = 0x494c4f4c,
	.p1_mprddlctl = 0x4a4c4c49,
	.p0_mpwrdlctl = 0x3f3f3133,
	.p1_mpwrdlctl = 0x39373f2e,
};

static struct mx6_mmdc_calibration mx6s_512m_mmdc_calib = {
	.p0_mpwldectrl0 = 0x0040003c,
	.p0_mpwldectrl1 = 0x0032003e,
	.p0_mpdgctrl0 = 0x42350231,
	.p0_mpdgctrl1 = 0x021a0218,
	.p0_mprddlctl = 0x4b4b4e49,
	.p0_mpwrdlctl = 0x3f3f3035,
};

/* DDR 64bit 1GB */
static struct mx6_ddr_sysinfo mem_dl = {
	.dsize		= 2,
	.cs1_mirror	= 0,
	/* config for full 4GB range so that get_mem_size() works */
	.cs_density	= 32,
	.ncs		= 1,
	.bi_on		= 1,
	.rtt_nom	= 1,
	.rtt_wr		= 0,
	.ralat		= 5,
	.walat		= 0,
	.mif3_mode	= 3,
	.rst_to_cke	= 0x23,
	.sde_to_rst	= 0x10,
};

/* DDR 32bit 512MB */
static struct mx6_ddr_sysinfo mem_s = {
	.dsize		= 1,
	.cs1_mirror	= 0,
	/* config for full 4GB range so that get_mem_size() works */
	.cs_density	= 32,
	.ncs		= 1,
	.bi_on		= 1,
	.rtt_nom	= 1,
	.rtt_wr		= 0,
	.ralat		= 5,
	.walat		= 0,
	.mif3_mode	= 3,
	.rst_to_cke	= 0x23,
	.sde_to_rst	= 0x10,
};

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

static void spl_dram_init(void)
{
	if (is_cpu_type(MXC_CPU_MX6SOLO)) {
		mx6sdl_dram_iocfg(32, &mx6sdl_ddr_ioregs, &mx6sdl_grp_ioregs);
		mx6_dram_cfg(&mem_s, &mx6s_512m_mmdc_calib, &h5tq2g63dfr);
	} else if (is_cpu_type(MXC_CPU_MX6DL)) {
		mx6sdl_dram_iocfg(64, &mx6sdl_ddr_ioregs, &mx6sdl_grp_ioregs);
		mx6_dram_cfg(&mem_dl, &mx6dl_1g_mmdc_calib, &h5tq2g63dfr);
	} else if (is_cpu_type(MXC_CPU_MX6Q)) {
		mx6dq_dram_iocfg(64, &mx6dq_ddr_ioregs, &mx6dq_grp_ioregs);
		mx6_dram_cfg(&mem_q, &mx6q_2g_mmdc_calib, &h5t04g63afr);
	}

	udelay(100);
}

static void setup_spi(void)
{
	enable_spi_clk(true, 2);
}

#define UART_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

static iomux_v3_cfg_t const uart1_pads[] = {
	IOMUX_PADS(PAD_CSI0_DAT10__UART1_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_CSI0_DAT11__UART1_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
};

static void setup_iomux_uart(void)
{
	SETUP_IOMUX_PADS(uart1_pads);
}

void board_init_f(ulong dummy)
{
	ccgr_init();

	/* setup AIPS and disable watchdog */
	arch_cpu_init();

	gpr_init();

	/* iomux */
	setup_iomux_uart();

	/* setup GP timer */
	timer_init();

	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();

	/* enable ECSPI clocks */
	setup_spi();

	/* DDR initialization */
	spl_dram_init();
}

void board_boot_order(u32 *spl_boot_list)
{
	switch (spl_boot_device()) {
	case BOOT_DEVICE_MMC2:
	case BOOT_DEVICE_MMC1:
		spl_boot_list[0] = BOOT_DEVICE_MMC2;
		spl_boot_list[1] = BOOT_DEVICE_MMC1;
		break;

	case BOOT_DEVICE_NOR:
		spl_boot_list[0] = BOOT_DEVICE_NOR;
		break;
	}
}

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	return 0;
}
#endif

#ifdef CONFIG_SPL_OS_BOOT
int spl_start_uboot(void)
{
	char s[16];
	int ret;
	/*
	 * We use BOOT_DEVICE_MMC1, but SD card is connected
	 * to MMC2
	 *
	 * Correct "mapping" is delivered in board defined
	 * board_boot_order() function.
	 *
	 * SD card boot is regarded as a "development" one,
	 * hence we _always_ go through the u-boot.
	 *
	 */
	if (spl_boot_device() == BOOT_DEVICE_MMC1)
		return 1;

	/* break into full u-boot on 'c' */
	if (serial_tstc() && serial_getc() == 'c')
		return 1;

	env_init();
	ret = env_get_f("boot_os", s, sizeof(s));
	if ((ret != -1) && (strcmp(s, "no") == 0))
		return 1;

	/*
	 * Check if SWUpdate recovery needs to be started
	 *
	 * recovery_status = NULL (not set - ret == -1) -> normal operation
	 *
	 * recovery_status = progress or
	 * recovery_status = failed   or
	 * recovery_status = <any value> -> start SWUpdate
	 *
	 */
	ret = env_get_f("recovery_status", s, sizeof(s));
	if (ret != -1)
		return 1;

	return 0;
}
#endif /* CONFIG_SPL_OS_BOOT */

#define WEIM_NOR_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE |          \
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |               \
	PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST)

#define NOR_WP			IMX_GPIO_NR(1, 1)

static iomux_v3_cfg_t const eimnor_pads[] = {
	IOMUX_PADS(PAD_EIM_D16__EIM_DATA16 | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D17__EIM_DATA17 | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D18__EIM_DATA18 | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D19__EIM_DATA19 | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D20__EIM_DATA20 | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D21__EIM_DATA21 | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D22__EIM_DATA22 | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D23__EIM_DATA23 | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D24__EIM_DATA24 | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D25__EIM_DATA25 | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D26__EIM_DATA26 | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D27__EIM_DATA27 | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D28__EIM_DATA28 | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D29__EIM_DATA29 | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D30__EIM_DATA30 | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D31__EIM_DATA31 | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_DA0__EIM_AD00   | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_DA1__EIM_AD01   | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_DA2__EIM_AD02   | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_DA3__EIM_AD03   | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_DA4__EIM_AD04   | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_DA5__EIM_AD05   | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_DA6__EIM_AD06   | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_DA7__EIM_AD07   | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_DA8__EIM_AD08   | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_DA9__EIM_AD09   | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_DA10__EIM_AD10  | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_DA11__EIM_AD11  | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_DA12__EIM_AD12  | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_DA13__EIM_AD13  | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_DA14__EIM_AD14  | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_DA15__EIM_AD15  | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_A16__EIM_ADDR16 | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_A17__EIM_ADDR17 | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_A18__EIM_ADDR18 | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_A19__EIM_ADDR19 | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_A20__EIM_ADDR20 | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_A21__EIM_ADDR21 | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_A22__EIM_ADDR22 | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_A23__EIM_ADDR23 | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_A24__EIM_ADDR24 | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_A25__EIM_ADDR25 | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_OE__EIM_OE_B	| MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_RW__EIM_RW		| MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_CS0__EIM_CS0_B	| MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_GPIO_1__GPIO1_IO01	| MUX_PAD_CTRL(NO_PAD_CTRL)),
};

static void eimnor_cs_setup(void)
{
	struct weim *weim_regs = (struct weim *)WEIM_BASE_ADDR;

	/* NOR configuration */
	writel(0x00620181, &weim_regs->cs0gcr1);
	writel(0x00000001, &weim_regs->cs0gcr2);
	writel(0x0b020000, &weim_regs->cs0rcr1);
	writel(0x0000b000, &weim_regs->cs0rcr2);
	writel(0x0804a240, &weim_regs->cs0wcr1);
	writel(0x00000000, &weim_regs->cs0wcr2);

	writel(0x00000120, &weim_regs->wcr);
	writel(0x00000010, &weim_regs->wiar);
	writel(0x00000000, &weim_regs->ear);

	set_chipselect_size(CS0_128);
}

static void setup_eimnor(void)
{
	SETUP_IOMUX_PADS(eimnor_pads);
	gpio_direction_output(NOR_WP, 1);

	enable_eim_clk(1);
	eimnor_cs_setup();
}

#define USDHC_PAD_CTRL (PAD_CTL_PUS_47K_UP |			\
	PAD_CTL_SPEED_LOW | PAD_CTL_DSE_80ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC2_CD_GPIO		IMX_GPIO_NR(1, 4)

static iomux_v3_cfg_t const usdhc2_pads[] = {
	IOMUX_PADS(PAD_SD2_CLK__SD2_CLK    | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_CMD__SD2_CMD    | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_DAT0__SD2_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_DAT1__SD2_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_DAT2__SD2_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_DAT3__SD2_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	/* Carrier MicroSD Card Detect */
	IOMUX_PADS(PAD_GPIO_4__GPIO1_IO04  | MUX_PAD_CTRL(NO_PAD_CTRL)),
};

static iomux_v3_cfg_t const usdhc3_pads[] = {
	IOMUX_PADS(PAD_SD3_CLK__SD3_CLK    | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_CMD__SD3_CMD    | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT0__SD3_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT1__SD3_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT2__SD3_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT3__SD3_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT4__SD3_DATA4 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT5__SD3_DATA5 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT6__SD3_DATA6 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT7__SD3_DATA7 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_RST__SD3_RESET  | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
};

static struct fsl_esdhc_cfg usdhc_cfg[2] = {
	{USDHC3_BASE_ADDR},
	{USDHC2_BASE_ADDR},
};

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	switch (cfg->esdhc_base) {
	case USDHC2_BASE_ADDR:
		ret = !gpio_get_value(USDHC2_CD_GPIO);
		break;
	case USDHC3_BASE_ADDR:
		/*
		 * eMMC don't have card detect pin - since it is soldered to the
		 * PCB board
		 */
		ret = 1;
		break;
	}
	return ret;
}

int board_mmc_init(struct bd_info *bis)
{
	int ret;
	u32 index = 0;

	/*
	 * MMC MAP
	 * (U-Boot device node)    (Physical Port)
	 * mmc0                    Soldered on board eMMC device
	 * mmc1                    MicroSD card
	 */
	for (index = 0; index < CFG_SYS_FSL_USDHC_NUM; ++index) {
		switch (index) {
		case 0:
			SETUP_IOMUX_PADS(usdhc3_pads);
			usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
			usdhc_cfg[0].max_bus_width = 8;
			break;
		case 1:
			SETUP_IOMUX_PADS(usdhc2_pads);
			usdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
			usdhc_cfg[1].max_bus_width = 4;
			gpio_direction_input(USDHC2_CD_GPIO);
			break;
		default:
			printf("Warning: More USDHC controllers (%d) than supported (%d)\n",
			       index + 1, CFG_SYS_FSL_USDHC_NUM);
			return -EINVAL;
		}

		ret = fsl_esdhc_initialize(bis, &usdhc_cfg[index]);
		if (ret)
			return ret;
	}

	return 0;
}

#ifdef CONFIG_SPL_BOARD_INIT
#define DISPLAY_EN		IMX_GPIO_NR(1, 2)
void spl_board_init(void)
{
	setup_eimnor();

	gpio_direction_output(DISPLAY_EN, 1);
}
#endif /* CONFIG_SPL_BOARD_INIT */
