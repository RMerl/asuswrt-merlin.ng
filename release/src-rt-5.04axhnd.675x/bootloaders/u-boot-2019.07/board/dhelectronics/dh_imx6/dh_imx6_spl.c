// SPDX-License-Identifier: GPL-2.0+
/*
 * DHCOM DH-iMX6 PDK SPL support
 *
 * Copyright (C) 2017 Marek Vasut <marex@denx.de>
 */

#include <common.h>
#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-ddr.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/io.h>
#include <errno.h>
#include <fuse.h>
#include <fsl_esdhc.h>
#include <i2c.h>
#include <mmc.h>
#include <spl.h>

#define ENET_PAD_CTRL							\
	(PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |	\
	 PAD_CTL_HYS)

#define GPIO_PAD_CTRL							\
	(PAD_CTL_HYS | PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm)

#define SPI_PAD_CTRL							\
	(PAD_CTL_HYS | PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |		\
	PAD_CTL_SRE_FAST)

#define UART_PAD_CTRL							\
	(PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |	\
	 PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_PAD_CTRL							\
	(PAD_CTL_PUS_47K_UP | PAD_CTL_SPEED_LOW | PAD_CTL_DSE_80ohm |	\
	 PAD_CTL_SRE_FAST | PAD_CTL_HYS)

static const struct mx6dq_iomux_ddr_regs dhcom6dq_ddr_ioregs = {
	.dram_sdclk_0	= 0x00020030,
	.dram_sdclk_1	= 0x00020030,
	.dram_cas	= 0x00020030,
	.dram_ras	= 0x00020030,
	.dram_reset	= 0x00020030,
	.dram_sdcke0	= 0x00003000,
	.dram_sdcke1	= 0x00003000,
	.dram_sdba2	= 0x00000000,
	.dram_sdodt0	= 0x00003030,
	.dram_sdodt1	= 0x00003030,
	.dram_sdqs0	= 0x00000030,
	.dram_sdqs1	= 0x00000030,
	.dram_sdqs2	= 0x00000030,
	.dram_sdqs3	= 0x00000030,
	.dram_sdqs4	= 0x00000030,
	.dram_sdqs5	= 0x00000030,
	.dram_sdqs6	= 0x00000030,
	.dram_sdqs7	= 0x00000030,
	.dram_dqm0	= 0x00020030,
	.dram_dqm1	= 0x00020030,
	.dram_dqm2	= 0x00020030,
	.dram_dqm3	= 0x00020030,
	.dram_dqm4	= 0x00020030,
	.dram_dqm5	= 0x00020030,
	.dram_dqm6	= 0x00020030,
	.dram_dqm7	= 0x00020030,
};

static const struct mx6dq_iomux_grp_regs dhcom6dq_grp_ioregs = {
	.grp_ddr_type	= 0x000C0000,
	.grp_ddrmode_ctl = 0x00020000,
	.grp_ddrpke	= 0x00000000,
	.grp_addds	= 0x00000030,
	.grp_ctlds	= 0x00000030,
	.grp_ddrmode	= 0x00020000,
	.grp_b0ds	= 0x00000030,
	.grp_b1ds	= 0x00000030,
	.grp_b2ds	= 0x00000030,
	.grp_b3ds	= 0x00000030,
	.grp_b4ds	= 0x00000030,
	.grp_b5ds	= 0x00000030,
	.grp_b6ds	= 0x00000030,
	.grp_b7ds	= 0x00000030,
};

static const struct mx6sdl_iomux_ddr_regs dhcom6sdl_ddr_ioregs = {
	.dram_sdclk_0	= 0x00020030,
	.dram_sdclk_1	= 0x00020030,
	.dram_cas	= 0x00020030,
	.dram_ras	= 0x00020030,
	.dram_reset	= 0x00020030,
	.dram_sdcke0	= 0x00003000,
	.dram_sdcke1	= 0x00003000,
	.dram_sdba2	= 0x00000000,
	.dram_sdodt0	= 0x00003030,
	.dram_sdodt1	= 0x00003030,
	.dram_sdqs0	= 0x00000030,
	.dram_sdqs1	= 0x00000030,
	.dram_sdqs2	= 0x00000030,
	.dram_sdqs3	= 0x00000030,
	.dram_sdqs4	= 0x00000030,
	.dram_sdqs5	= 0x00000030,
	.dram_sdqs6	= 0x00000030,
	.dram_sdqs7	= 0x00000030,
	.dram_dqm0	= 0x00020030,
	.dram_dqm1	= 0x00020030,
	.dram_dqm2	= 0x00020030,
	.dram_dqm3	= 0x00020030,
	.dram_dqm4	= 0x00020030,
	.dram_dqm5	= 0x00020030,
	.dram_dqm6	= 0x00020030,
	.dram_dqm7	= 0x00020030,
};

static const struct mx6sdl_iomux_grp_regs dhcom6sdl_grp_ioregs = {
	.grp_ddr_type	= 0x000C0000,
	.grp_ddrmode_ctl = 0x00020000,
	.grp_ddrpke	= 0x00000000,
	.grp_addds	= 0x00000030,
	.grp_ctlds	= 0x00000030,
	.grp_ddrmode	= 0x00020000,
	.grp_b0ds	= 0x00000030,
	.grp_b1ds	= 0x00000030,
	.grp_b2ds	= 0x00000030,
	.grp_b3ds	= 0x00000030,
	.grp_b4ds	= 0x00000030,
	.grp_b5ds	= 0x00000030,
	.grp_b6ds	= 0x00000030,
	.grp_b7ds	= 0x00000030,
};

static const struct mx6_mmdc_calibration dhcom_mmdc_calib_4x4g_1066 = {
	.p0_mpwldectrl0	= 0x00150019,
	.p0_mpwldectrl1	= 0x001C000B,
	.p1_mpwldectrl0	= 0x00020018,
	.p1_mpwldectrl1	= 0x0002000C,
	.p0_mpdgctrl0	= 0x43140320,
	.p0_mpdgctrl1	= 0x03080304,
	.p1_mpdgctrl0	= 0x43180320,
	.p1_mpdgctrl1	= 0x03100254,
	.p0_mprddlctl	= 0x4830383C,
	.p1_mprddlctl	= 0x3836323E,
	.p0_mpwrdlctl	= 0x3E444642,
	.p1_mpwrdlctl	= 0x42344442,
};

static const struct mx6_mmdc_calibration dhcom_mmdc_calib_2x4g_800 = {
	.p0_mpwldectrl0	= 0x0040003C,
	.p0_mpwldectrl1	= 0x0032003E,
	.p0_mpdgctrl0	= 0x42350231,
	.p0_mpdgctrl1	= 0x021A0218,
	.p0_mprddlctl	= 0x4B4B4E49,
	.p0_mpwrdlctl	= 0x3F3F3035,
};

static const struct mx6_mmdc_calibration dhcom_mmdc_calib_4x2g_1066 = {
	.p0_mpwldectrl0	= 0x001a001a,
	.p0_mpwldectrl1	= 0x00260015,
	.p0_mpdgctrl0	= 0x030c0320,
	.p0_mpdgctrl1	= 0x03100304,
	.p0_mprddlctl	= 0x432e3538,
	.p0_mpwrdlctl	= 0x363f423d,
	.p1_mpwldectrl0	= 0x0006001e,
	.p1_mpwldectrl1	= 0x00050015,
	.p1_mpdgctrl0	= 0x031c0324,
	.p1_mpdgctrl1	= 0x030c0258,
	.p1_mprddlctl	= 0x3834313f,
	.p1_mpwrdlctl	= 0x47374a42,
};

static const struct mx6_mmdc_calibration dhcom_mmdc_calib_4x2g_800 = {
	.p0_mpwldectrl0	= 0x003A003A,
	.p0_mpwldectrl1	= 0x0030002F,
	.p1_mpwldectrl0	= 0x002F0038,
	.p1_mpwldectrl1	= 0x00270039,
	.p0_mpdgctrl0	= 0x420F020F,
	.p0_mpdgctrl1	= 0x01760175,
	.p1_mpdgctrl0	= 0x41640171,
	.p1_mpdgctrl1	= 0x015E0160,
	.p0_mprddlctl	= 0x45464B4A,
	.p1_mprddlctl	= 0x49484A46,
	.p0_mpwrdlctl	= 0x40402E32,
	.p1_mpwrdlctl	= 0x3A3A3231,
};

static const struct mx6_mmdc_calibration dhcom_mmdc_calib_2x2g_800 = {
	.p0_mpwldectrl0	= 0x0040003C,
	.p0_mpwldectrl1	= 0x0032003E,
	.p0_mpdgctrl0	= 0x42350231,
	.p0_mpdgctrl1	= 0x021A0218,
	.p0_mprddlctl	= 0x4B4B4E49,
	.p0_mpwrdlctl	= 0x3F3F3035,
};

/*
 * 2 Gbit DDR3 memory
 *   - NANYA #NT5CC128M16IP-DII
 *   - NANYA #NT5CB128M16FP-DII
 */
static const struct mx6_ddr3_cfg dhcom_mem_ddr_2g = {
	.mem_speed	= 1600,
	.density	= 2,
	.width		= 16,
	.banks		= 8,
	.rowaddr	= 14,
	.coladdr	= 10,
	.pagesz		= 2,
	.trcd		= 1375,
	.trcmin		= 5863,
	.trasmin	= 3750,
};

/*
 * 4 Gbit DDR3 memory
 *   - Intelligent Memory #IM4G16D3EABG-125I
 */
static const struct mx6_ddr3_cfg dhcom_mem_ddr_4g = {
	.mem_speed	= 1600,
	.density	= 4,
	.width		= 16,
	.banks		= 8,
	.rowaddr	= 15,
	.coladdr	= 10,
	.pagesz		= 2,
	.trcd		= 1375,
	.trcmin		= 4875,
	.trasmin	= 3500,
};

/* DDR3 64bit */
static const struct mx6_ddr_sysinfo dhcom_ddr_64bit = {
	/* width of data bus:0=16,1=32,2=64 */
	.dsize		= 2,
	.cs_density	= 32,
	.ncs		= 1,	/* single chip select */
	.cs1_mirror	= 1,
	.rtt_wr		= 1,	/* DDR3_RTT_60_OHM, RTT_Wr = RZQ/4 */
	.rtt_nom	= 1,	/* DDR3_RTT_60_OHM, RTT_Nom = RZQ/4 */
	.walat		= 1,	/* Write additional latency */
	.ralat		= 5,	/* Read additional latency */
	.mif3_mode	= 3,	/* Command prediction working mode */
	.bi_on		= 1,	/* Bank interleaving enabled */
	.sde_to_rst	= 0x10,	/* 14 cycles, 200us (JEDEC default) */
	.rst_to_cke	= 0x23,	/* 33 cycles, 500us (JEDEC default) */
	.refsel		= 1,	/* Refresh cycles at 32KHz */
	.refr		= 3,	/* 4 refresh commands per refresh cycle */
};

/* DDR3 32bit */
static const struct mx6_ddr_sysinfo dhcom_ddr_32bit = {
	/* width of data bus:0=16,1=32,2=64 */
	.dsize		= 1,
	.cs_density	= 32,
	.ncs		= 1,	/* single chip select */
	.cs1_mirror	= 1,
	.rtt_wr		= 1,	/* DDR3_RTT_60_OHM, RTT_Wr = RZQ/4 */
	.rtt_nom	= 1,	/* DDR3_RTT_60_OHM, RTT_Nom = RZQ/4 */
	.walat		= 1,	/* Write additional latency */
	.ralat		= 5,	/* Read additional latency */
	.mif3_mode	= 3,	/* Command prediction working mode */
	.bi_on		= 1,	/* Bank interleaving enabled */
	.sde_to_rst	= 0x10,	/* 14 cycles, 200us (JEDEC default) */
	.rst_to_cke	= 0x23,	/* 33 cycles, 500us (JEDEC default) */
	.refsel		= 1,	/* Refresh cycles at 32KHz */
	.refr		= 3,	/* 4 refresh commands per refresh cycle */
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

/* Board ID */
static iomux_v3_cfg_t const hwcode_pads[] = {
	IOMUX_PADS(PAD_EIM_A19__GPIO2_IO19	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_A23__GPIO6_IO06	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_A22__GPIO2_IO16	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
};

static void setup_iomux_boardid(void)
{
	/* HW code pins: Setup alternate function and configure pads */
	SETUP_IOMUX_PADS(hwcode_pads);
}

/* DDR Code */
static iomux_v3_cfg_t const ddrcode_pads[] = {
	IOMUX_PADS(PAD_EIM_A16__GPIO2_IO22	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_A17__GPIO2_IO21	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
};

static void setup_iomux_ddrcode(void)
{
	/* ddr code pins */
	SETUP_IOMUX_PADS(ddrcode_pads);
}

enum dhcom_ddr3_code {
	DH_DDR3_SIZE_256MIB = 0x00,
	DH_DDR3_SIZE_512MIB = 0x01,
	DH_DDR3_SIZE_1GIB   = 0x02,
	DH_DDR3_SIZE_2GIB   = 0x03
};

#define DDR3_CODE_BIT_0   IMX_GPIO_NR(2, 22)
#define DDR3_CODE_BIT_1   IMX_GPIO_NR(2, 21)

enum dhcom_ddr3_code dhcom_get_ddr3_code(void)
{
	enum dhcom_ddr3_code ddr3_code;

	gpio_request(DDR3_CODE_BIT_0, "DDR3_CODE_BIT_0");
	gpio_request(DDR3_CODE_BIT_1, "DDR3_CODE_BIT_1");

	gpio_direction_input(DDR3_CODE_BIT_0);
	gpio_direction_input(DDR3_CODE_BIT_1);

	/* 256MB = 0b00; 512MB = 0b01; 1GB = 0b10; 2GB = 0b11 */
	ddr3_code = (!!gpio_get_value(DDR3_CODE_BIT_1) << 1)
	     | (!!gpio_get_value(DDR3_CODE_BIT_0));

	return ddr3_code;
}

/* GPIO */
static iomux_v3_cfg_t const gpio_pads[] = {
	IOMUX_PADS(PAD_GPIO_2__GPIO1_IO02	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_GPIO_4__GPIO1_IO04	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_GPIO_5__GPIO1_IO05	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_CSI0_DAT17__GPIO6_IO03	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_GPIO_19__GPIO4_IO05	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_DI0_PIN4__GPIO4_IO20	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D27__GPIO3_IO27	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_KEY_ROW0__GPIO4_IO07	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_KEY_COL1__GPIO4_IO08	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_CS1__GPIO6_IO14	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_CS2__GPIO6_IO15	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_KEY_ROW1__GPIO4_IO09	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT5__GPIO7_IO00	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT4__GPIO7_IO01	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_CSI0_VSYNC__GPIO5_IO21	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_GPIO_18__GPIO7_IO13	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_SD1_CMD__GPIO1_IO18	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_SD1_DAT0__GPIO1_IO16	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_SD1_DAT1__GPIO1_IO17	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_SD1_DAT2__GPIO1_IO19	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_SD1_CLK__GPIO1_IO20	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_CSI0_PIXCLK__GPIO5_IO18	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_CSI0_MCLK__GPIO5_IO19	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
};

static void setup_iomux_gpio(void)
{
	SETUP_IOMUX_PADS(gpio_pads);
}

/* Ethernet */
static iomux_v3_cfg_t const enet_pads[] = {
	IOMUX_PADS(PAD_ENET_MDIO__ENET_MDIO	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_MDC__ENET_MDC	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_TX_EN__ENET_TX_EN	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_TXD0__ENET_TX_DATA0	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_TXD1__ENET_TX_DATA1	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_GPIO_16__ENET_REF_CLK	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_RX_ER__ENET_RX_ER	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_RXD0__ENET_RX_DATA0	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_RXD1__ENET_RX_DATA1	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_CRS_DV__ENET_RX_EN	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	/* SMSC PHY Reset */
	IOMUX_PADS(PAD_EIM_WAIT__GPIO5_IO00	| MUX_PAD_CTRL(NO_PAD_CTRL)),
	/* ENET_VIO_GPIO */
	IOMUX_PADS(PAD_GPIO_7__GPIO1_IO07	| MUX_PAD_CTRL(NO_PAD_CTRL)),
	/* ENET_Interrupt - (not used) */
	IOMUX_PADS(PAD_RGMII_RD0__GPIO6_IO25	| MUX_PAD_CTRL(NO_PAD_CTRL)),
};

static void setup_iomux_enet(void)
{
	SETUP_IOMUX_PADS(enet_pads);
}

/* SD interface */
static iomux_v3_cfg_t const usdhc2_pads[] = {
	IOMUX_PADS(PAD_SD2_DAT0__SD2_DATA0	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_DAT1__SD2_DATA1	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_DAT2__SD2_DATA2	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_DAT3__SD2_DATA3	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_CLK__SD2_CLK		| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_CMD__SD2_CMD		| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_CS3__GPIO6_IO16	| MUX_PAD_CTRL(NO_PAD_CTRL)), /* CD */
};

/* onboard microSD */
static iomux_v3_cfg_t const usdhc3_pads[] = {
	IOMUX_PADS(PAD_SD3_DAT0__SD3_DATA0	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT1__SD3_DATA1	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT2__SD3_DATA2	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT3__SD3_DATA3	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_CLK__SD3_CLK		| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_CMD__SD3_CMD		| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_RST__GPIO7_IO08	| MUX_PAD_CTRL(NO_PAD_CTRL)), /* CD */
};

/* eMMC */
static iomux_v3_cfg_t const usdhc4_pads[] = {
	IOMUX_PADS(PAD_SD4_DAT0__SD4_DATA0	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT1__SD4_DATA1	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT2__SD4_DATA2	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT3__SD4_DATA3	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT4__SD4_DATA4	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT5__SD4_DATA5	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT6__SD4_DATA6	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT7__SD4_DATA7	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_CLK__SD4_CLK		| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_CMD__SD4_CMD		| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
};

/* SD */
static void setup_iomux_sd(void)
{
	SETUP_IOMUX_PADS(usdhc2_pads);
	SETUP_IOMUX_PADS(usdhc3_pads);
	SETUP_IOMUX_PADS(usdhc4_pads);
}

/* SPI */
static iomux_v3_cfg_t const ecspi1_pads[] = {
	/* SS0 */
	IOMUX_PADS(PAD_EIM_EB2__GPIO2_IO30	| MUX_PAD_CTRL(SPI_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D17__ECSPI1_MISO	| MUX_PAD_CTRL(SPI_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D18__ECSPI1_MOSI	| MUX_PAD_CTRL(SPI_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D16__ECSPI1_SCLK	| MUX_PAD_CTRL(SPI_PAD_CTRL)),
};

static void setup_iomux_spi(void)
{
	SETUP_IOMUX_PADS(ecspi1_pads);
}

int board_spi_cs_gpio(unsigned bus, unsigned cs)
{
	if (bus == 0 && cs == 0)
		return IMX_GPIO_NR(2, 30);
	else
		return -1;
}

/* UART */
static iomux_v3_cfg_t const uart1_pads[] = {
	IOMUX_PADS(PAD_SD3_DAT7__UART1_TX_DATA	| MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT6__UART1_RX_DATA	| MUX_PAD_CTRL(UART_PAD_CTRL)),
};

static void setup_iomux_uart(void)
{
	SETUP_IOMUX_PADS(uart1_pads);
}

/* USB */
static iomux_v3_cfg_t const usb_pads[] = {
	IOMUX_PADS(PAD_GPIO_1__USB_OTG_ID	| MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D31__GPIO3_IO31	| MUX_PAD_CTRL(NO_PAD_CTRL)),
};

static void setup_iomux_usb(void)
{
	SETUP_IOMUX_PADS(usb_pads);
}

/* Perform DDR DRAM calibration */
static int spl_dram_perform_cal(struct mx6_ddr_sysinfo const *sysinfo)
{
	int ret = 0;

#ifdef CONFIG_MX6_DDRCAL
	udelay(100);
	ret = mmdc_do_write_level_calibration(sysinfo);
	if (ret) {
		printf("DDR3: Write level calibration error [%d]\n", ret);
		return ret;
	}

	ret = mmdc_do_dqs_calibration(sysinfo);
	if (ret) {
		printf("DDR3: DQS calibration error [%d]\n", ret);
		return ret;
	}
#endif /* CONFIG_MX6_DDRCAL */

	return ret;
}


/* DRAM */
static void dhcom_spl_dram_init(void)
{
	enum dhcom_ddr3_code ddr3_code = dhcom_get_ddr3_code();

	if (is_mx6dq()) {
		mx6dq_dram_iocfg(64, &dhcom6dq_ddr_ioregs,
					&dhcom6dq_grp_ioregs);
		switch (ddr3_code) {
		default:
			printf("imx6qd: unsupported ddr3 code %d\n", ddr3_code);
			printf("        choosing 1024 MB\n");
			/* fall through */
		case DH_DDR3_SIZE_1GIB:
			mx6_dram_cfg(&dhcom_ddr_64bit,
				     &dhcom_mmdc_calib_4x2g_1066,
				     &dhcom_mem_ddr_2g);
			break;
		case DH_DDR3_SIZE_2GIB:
			mx6_dram_cfg(&dhcom_ddr_64bit,
				     &dhcom_mmdc_calib_4x4g_1066,
				     &dhcom_mem_ddr_4g);
			break;
		}

		/* Perform DDR DRAM calibration */
		spl_dram_perform_cal(&dhcom_ddr_64bit);

	} else if (is_cpu_type(MXC_CPU_MX6DL)) {
		mx6sdl_dram_iocfg(64, &dhcom6sdl_ddr_ioregs,
					  &dhcom6sdl_grp_ioregs);
		switch (ddr3_code) {
		default:
			printf("imx6dl: unsupported ddr3 code %d\n", ddr3_code);
			printf("        choosing 1024 MB\n");
			/* fall through */
		case DH_DDR3_SIZE_1GIB:
			mx6_dram_cfg(&dhcom_ddr_64bit,
				     &dhcom_mmdc_calib_4x2g_800,
				     &dhcom_mem_ddr_2g);
			break;
		}

		/* Perform DDR DRAM calibration */
		spl_dram_perform_cal(&dhcom_ddr_64bit);

	} else if (is_cpu_type(MXC_CPU_MX6SOLO)) {
		mx6sdl_dram_iocfg(32, &dhcom6sdl_ddr_ioregs,
					  &dhcom6sdl_grp_ioregs);
		switch (ddr3_code) {
		default:
			printf("imx6s: unsupported ddr3 code %d\n", ddr3_code);
			printf("       choosing 512 MB\n");
			/* fall through */
		case DH_DDR3_SIZE_512MIB:
			mx6_dram_cfg(&dhcom_ddr_32bit,
				     &dhcom_mmdc_calib_2x2g_800,
				     &dhcom_mem_ddr_2g);
			break;
		case DH_DDR3_SIZE_1GIB:
			mx6_dram_cfg(&dhcom_ddr_32bit,
				     &dhcom_mmdc_calib_2x4g_800,
				     &dhcom_mem_ddr_4g);
			break;
		}

		/* Perform DDR DRAM calibration */
		spl_dram_perform_cal(&dhcom_ddr_32bit);
	}
}

void board_init_f(ulong dummy)
{
	/* setup AIPS and disable watchdog */
	arch_cpu_init();

	ccgr_init();
	gpr_init();

	/* setup GP timer */
	timer_init();

	setup_iomux_boardid();
	setup_iomux_ddrcode();
	setup_iomux_gpio();
	setup_iomux_enet();
	setup_iomux_sd();
	setup_iomux_spi();
	setup_iomux_uart();
	setup_iomux_usb();

	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();

	/* DDR3 initialization */
	dhcom_spl_dram_init();

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	/* load/boot image from boot device */
	board_init_r(NULL, 0);
}
