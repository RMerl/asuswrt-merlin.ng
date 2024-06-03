// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Logic PD, Inc.
 *
 * Author: Adam Ford <aford173@gmail.com>
 *
 * Based on SabreSD by Fabio Estevam <fabio.estevam@nxp.com>
 * and updates by Jagan Teki <jagan@amarulasolutions.com>
 */

#include <common.h>
#include <miiphy.h>
#include <input.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <linux/sizes.h>
#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mxc_hdmi.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/iomux-v3.h>

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL  (PAD_CTL_PKE | PAD_CTL_PUE |            \
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |               \
	PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define NAND_PAD_CTRL  (PAD_CTL_PKE | PAD_CTL_PUE |            \
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED   |             \
	PAD_CTL_DSE_40ohm   | PAD_CTL_HYS)

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();
	return 0;
}

static iomux_v3_cfg_t const uart1_pads[] = {
	MX6_PAD_SD3_DAT7__UART1_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_SD3_DAT6__UART1_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static iomux_v3_cfg_t const uart2_pads[] = {
	MX6_PAD_SD4_DAT4__UART2_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_SD4_DAT5__UART2_RTS_B | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_SD4_DAT6__UART2_CTS_B | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_SD4_DAT7__UART2_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static iomux_v3_cfg_t const uart3_pads[] = {
	MX6_PAD_EIM_D23__UART3_CTS_B | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_EIM_D24__UART3_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_EIM_D25__UART3_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_EIM_EB3__UART3_RTS_B | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static void setup_iomux_uart(void)
{
	imx_iomux_v3_setup_multiple_pads(uart1_pads, ARRAY_SIZE(uart1_pads));
	imx_iomux_v3_setup_multiple_pads(uart2_pads, ARRAY_SIZE(uart2_pads));
	imx_iomux_v3_setup_multiple_pads(uart3_pads, ARRAY_SIZE(uart3_pads));
}

static iomux_v3_cfg_t const nand_pads[] = {
	MX6_PAD_NANDF_CS0__NAND_CE0_B | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_NANDF_ALE__NAND_ALE  | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_NANDF_CLE__NAND_CLE  | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_NANDF_WP_B__NAND_WP_B  | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_NANDF_RB0__NAND_READY_B   | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_NANDF_D0__NAND_DATA00    | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_NANDF_D1__NAND_DATA01    | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_NANDF_D2__NAND_DATA02    | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_NANDF_D3__NAND_DATA03    | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_NANDF_D4__NAND_DATA04    | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_NANDF_D5__NAND_DATA05    | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_NANDF_D6__NAND_DATA06    | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_NANDF_D7__NAND_DATA07    | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_SD4_CLK__NAND_WE_B    | MUX_PAD_CTRL(NAND_PAD_CTRL),
	MX6_PAD_SD4_CMD__NAND_RE_B    | MUX_PAD_CTRL(NAND_PAD_CTRL),
};

static void setup_nand_pins(void)
{
	imx_iomux_v3_setup_multiple_pads(nand_pads, ARRAY_SIZE(nand_pads));
}

static int ar8031_phy_fixup(struct phy_device *phydev)
{
	unsigned short val;

	/* To enable AR8031 output a 125MHz clk from CLK_25M */
	phy_write(phydev, MDIO_DEVAD_NONE, 0xd, 0x7);
	phy_write(phydev, MDIO_DEVAD_NONE, 0xe, 0x8016);
	phy_write(phydev, MDIO_DEVAD_NONE, 0xd, 0x4007);

	val = phy_read(phydev, MDIO_DEVAD_NONE, 0xe);
	val &= 0xffe3;
	val |= 0x18;
	phy_write(phydev, MDIO_DEVAD_NONE, 0xe, val);

	/* introduce tx clock delay */
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x5);
	val = phy_read(phydev, MDIO_DEVAD_NONE, 0x1e);
	val |= 0x0100;
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, val);

	return 0;
}

int board_phy_config(struct phy_device *phydev)
{
	ar8031_phy_fixup(phydev);

	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

/*
 * Do not overwrite the console
 * Use always serial for U-Boot console
 */
int overwrite_console(void)
{
	return 1;
}

int board_early_init_f(void)
{
	setup_iomux_uart();
	setup_nand_pins();
	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;
	return 0;
}

int board_late_init(void)
{
	env_set("board_name", "imx6logic");

	if (is_mx6dq()) {
		env_set("board_rev", "MX6DQ");
		if (!env_get("fdt_file"))
			env_set("fdt_file", "imx6q-logicpd.dtb");
	}

	return 0;
}

#ifdef CONFIG_SPL_BUILD
#include <asm/arch/mx6-ddr.h>
#include <asm/arch/mx6q-ddr.h>
#include <spl.h>
#include <linux/libfdt.h>

#ifdef CONFIG_SPL_OS_BOOT
int spl_start_uboot(void)
{
	/* break into full u-boot on 'c' */
	if (serial_tstc() && serial_getc() == 'c')
		return 1;

	return 0;
}
#endif

/* SD interface */
#define USDHC_PAD_CTRL							\
	(PAD_CTL_PUS_47K_UP | PAD_CTL_SPEED_LOW | PAD_CTL_DSE_80ohm |	\
	 PAD_CTL_SRE_FAST | PAD_CTL_HYS)

static iomux_v3_cfg_t const usdhc1_pads[] = {
	MX6_PAD_SD1_CLK__SD1_CLK   | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_CMD__SD1_CMD   | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DAT0__SD1_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DAT1__SD1_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DAT2__SD1_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DAT3__SD1_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
};

static iomux_v3_cfg_t const usdhc2_pads[] = {
	MX6_PAD_SD2_DAT0__SD2_DATA0	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD2_DAT1__SD2_DATA1	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD2_DAT2__SD2_DATA2	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD2_DAT3__SD2_DATA3	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD2_CLK__SD2_CLK	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD2_CMD__SD2_CMD	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_GPIO_4__GPIO1_IO04	| MUX_PAD_CTRL(NO_PAD_CTRL), /* CD */
};

#ifdef CONFIG_FSL_ESDHC
struct fsl_esdhc_cfg usdhc_cfg[] = {
	{USDHC1_BASE_ADDR}, /* SOM */
	{USDHC2_BASE_ADDR}  /* Baseboard */
};

int board_mmc_init(bd_t *bis)
{
	struct src *psrc = (struct src *)SRC_BASE_ADDR;
	unsigned int reg = readl(&psrc->sbmr1) >> 11;
	/*
	 * Upon reading BOOT_CFG register the following map is done:
	 * Bit 11 and 12 of BOOT_CFG register can determine the current
	 * mmc port
	 * 0x1                  SD1-SOM
	 * 0x2                  SD2-Baseboard
	 */

	reg &= 0x3; /* Only care about bottom 2 bits */

	switch (reg) {
	case 0:
		SETUP_IOMUX_PADS(usdhc1_pads);
		usdhc_cfg[0].esdhc_base = USDHC1_BASE_ADDR;
		usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
		gd->arch.sdhc_clk = usdhc_cfg[0].sdhc_clk;
		break;
	case 1:
		SETUP_IOMUX_PADS(usdhc2_pads);
		usdhc_cfg[1].esdhc_base = USDHC2_BASE_ADDR;
		usdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
		gd->arch.sdhc_clk = usdhc_cfg[1].sdhc_clk;
		break;
	}

	return fsl_esdhc_initialize(bis, &usdhc_cfg[reg]);
}

int board_mmc_getcd(struct mmc *mmc)
{
	return 1;
}
#endif

static void ccgr_init(void)
{
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	writel(0x00C03F3F, &ccm->CCGR0);
	writel(0x0030FC03, &ccm->CCGR1);
	writel(0x0FFFC000, &ccm->CCGR2);
	writel(0x3FF00000, &ccm->CCGR3);
	writel(0xFFFFF300, &ccm->CCGR4);
	writel(0x0F0000F3, &ccm->CCGR5);
	writel(0x00000FFF, &ccm->CCGR6);
}

static int mx6q_dcd_table[] = {
	MX6_IOM_GRP_DDR_TYPE, 0x000C0000,
	MX6_IOM_GRP_DDRPKE, 0x00000000,
	MX6_IOM_DRAM_SDCLK_0, 0x00000030,
	MX6_IOM_DRAM_SDCLK_1, 0x00000030,
	MX6_IOM_DRAM_CAS, 0x00000030,
	MX6_IOM_DRAM_RAS, 0x00000030,
	MX6_IOM_GRP_ADDDS, 0x00000030,
	MX6_IOM_DRAM_RESET, 0x00000030,
	MX6_IOM_DRAM_SDBA2, 0x00000000,
	MX6_IOM_DRAM_SDODT0, 0x00000030,
	MX6_IOM_DRAM_SDODT1, 0x00000030,
	MX6_IOM_GRP_CTLDS, 0x00000030,
	MX6_IOM_DDRMODE_CTL, 0x00020000,
	MX6_IOM_DRAM_SDQS0, 0x00000030,
	MX6_IOM_DRAM_SDQS1, 0x00000030,
	MX6_IOM_DRAM_SDQS2, 0x00000030,
	MX6_IOM_DRAM_SDQS3, 0x00000030,
	MX6_IOM_GRP_DDRMODE, 0x00020000,
	MX6_IOM_GRP_B0DS, 0x00000030,
	MX6_IOM_GRP_B1DS, 0x00000030,
	MX6_IOM_GRP_B2DS, 0x00000030,
	MX6_IOM_GRP_B3DS, 0x00000030,
	MX6_IOM_DRAM_DQM0, 0x00000030,
	MX6_IOM_DRAM_DQM1, 0x00000030,
	MX6_IOM_DRAM_DQM2, 0x00000030,
	MX6_IOM_DRAM_DQM3, 0x00000030,
	MX6_MMDC_P0_MDSCR, 0x00008000,
	MX6_MMDC_P0_MPZQHWCTRL, 0xA1390003,
	MX6_MMDC_P0_MPWLDECTRL0, 0x002D003A,
	MX6_MMDC_P0_MPWLDECTRL1, 0x0038002B,
	MX6_MMDC_P0_MPDGCTRL0, 0x03340338,
	MX6_MMDC_P0_MPDGCTRL1, 0x0334032C,
	MX6_MMDC_P0_MPRDDLCTL, 0x4036383C,
	MX6_MMDC_P0_MPWRDLCTL, 0x2E384038,
	MX6_MMDC_P0_MPRDDQBY0DL, 0x33333333,
	MX6_MMDC_P0_MPRDDQBY1DL, 0x33333333,
	MX6_MMDC_P0_MPRDDQBY2DL, 0x33333333,
	MX6_MMDC_P0_MPRDDQBY3DL, 0x33333333,
	MX6_MMDC_P0_MPMUR0, 0x00000800,
	MX6_MMDC_P0_MDPDC, 0x00020036,
	MX6_MMDC_P0_MDOTC, 0x09444040,
	MX6_MMDC_P0_MDCFG0, 0xB8BE7955,
	MX6_MMDC_P0_MDCFG1, 0xFF328F64,
	MX6_MMDC_P0_MDCFG2, 0x01FF00DB,
	MX6_MMDC_P0_MDMISC, 0x00011740,
	MX6_MMDC_P0_MDSCR, 0x00008000,
	MX6_MMDC_P0_MDRWD, 0x000026D2,
	MX6_MMDC_P0_MDOR, 0x00BE1023,
	MX6_MMDC_P0_MDASP, 0x00000047,
	MX6_MMDC_P0_MDCTL, 0x85190000,
	MX6_MMDC_P0_MDSCR, 0x00888032,
	MX6_MMDC_P0_MDSCR, 0x00008033,
	MX6_MMDC_P0_MDSCR, 0x00008031,
	MX6_MMDC_P0_MDSCR, 0x19408030,
	MX6_MMDC_P0_MDSCR, 0x04008040,
	MX6_MMDC_P0_MDREF, 0x00007800,
	MX6_MMDC_P0_MPODTCTRL, 0x00000007,
	MX6_MMDC_P0_MDPDC, 0x00025576,
	MX6_MMDC_P0_MAPSR, 0x00011006,
	MX6_MMDC_P0_MDSCR, 0x00000000,
	/* enable AXI cache for VDOA/VPU/IPU */

	MX6_IOMUXC_GPR4, 0xF00000CF,
	/* set IPU AXI-id0 Qos=0xf(bypass) AXI-id1 Qos=0x7 */
	MX6_IOMUXC_GPR6, 0x007F007F,
	MX6_IOMUXC_GPR7, 0x007F007F,
};

static void ddr_init(int *table, int size)
{
	int i;

	for (i = 0; i < size / 2 ; i++)
		writel(table[2 * i + 1], table[2 * i]);
}

static void spl_dram_init(void)
{
	if (is_mx6dq())
		ddr_init(mx6q_dcd_table, ARRAY_SIZE(mx6q_dcd_table));
}

void board_init_f(ulong dummy)
{
	/* DDR initialization */
	spl_dram_init();

	/* setup AIPS and disable watchdog */
	arch_cpu_init();

	ccgr_init();
	gpr_init();

	/* iomux and setup of uart and NAND pins */
	board_early_init_f();

	/* setup GP timer */
	timer_init();

	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	/* load/boot image from boot device */
	board_init_r(NULL, 0);
}
#endif
