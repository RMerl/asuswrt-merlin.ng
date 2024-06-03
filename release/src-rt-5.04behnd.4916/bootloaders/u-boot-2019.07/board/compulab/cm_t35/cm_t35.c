// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2011 - 2013 CompuLab, Ltd. <www.compulab.co.il>
 *
 * Authors: Mike Rapoport <mike@compulab.co.il>
 *	    Igor Grinberg <grinberg@compulab.co.il>
 *
 * Derived from omap3evm and Beagle Board by
 *	Manikandan Pillai <mani.pillai@ti.com>
 *	Richard Woodruff <r-woodruff2@ti.com>
 *	Syed Mohammed Khasim <x0khasim@ti.com>
 */

#include <common.h>
#include <environment.h>
#include <status_led.h>
#include <netdev.h>
#include <net.h>
#include <i2c.h>
#include <usb.h>
#include <mmc.h>
#include <splash.h>
#include <twl4030.h>
#include <linux/compiler.h>

#include <asm/io.h>
#include <linux/errno.h>
#include <asm/arch/mem.h>
#include <asm/arch/mux.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-types.h>
#include <asm/ehci-omap.h>
#include <asm/gpio.h>

#include "../common/common.h"
#include "../common/eeprom.h"

DECLARE_GLOBAL_DATA_PTR;

const omap3_sysinfo sysinfo = {
	DDR_DISCRETE,
	"CM-T3x board",
	"NAND",
};

#ifdef CONFIG_SPL_BUILD
/*
 * Routine: get_board_mem_timings
 * Description: If we use SPL then there is no x-loader nor config header
 * so we have to setup the DDR timings ourself on both banks.
 */
void get_board_mem_timings(struct board_sdrc_timings *timings)
{
	timings->mr = MICRON_V_MR_165;
	timings->mcfg = MICRON_V_MCFG_200(256 << 20); /* raswidth 14 needed */
	timings->ctrla = MICRON_V_ACTIMA_165;
	timings->ctrlb = MICRON_V_ACTIMB_165;
	timings->rfr_ctrl = SDP_3430_SDRC_RFR_CTRL_165MHz;
}
#endif

struct splash_location splash_locations[] = {
	{
		.name = "nand",
		.storage = SPLASH_STORAGE_NAND,
		.flags = SPLASH_STORAGE_RAW,
		.offset = 0x100000,
	},
};

int splash_screen_prepare(void)
{
	return splash_source_load(splash_locations,
				  ARRAY_SIZE(splash_locations));
}

/*
 * Routine: board_init
 * Description: hardware init.
 */
int board_init(void)
{
	gpmc_init(); /* in SRAM or SDRAM, finish GPMC */

	/* board id for Linux */
	if (get_cpu_family() == CPU_OMAP34XX)
		gd->bd->bi_arch_number = MACH_TYPE_CM_T35;
	else
		gd->bd->bi_arch_number = MACH_TYPE_CM_T3730;

	/* boot param addr */
	gd->bd->bi_boot_params = (OMAP34XX_SDRC_CS0 + 0x100);

#if defined(CONFIG_LED_STATUS) && defined(CONFIG_LED_STATUS_BOOT_ENABLE)
	status_led_set(CONFIG_LED_STATUS_BOOT, CONFIG_LED_STATUS_ON);
#endif

	return 0;
}

/*
 * Routine: get_board_rev
 * Description: read system revision
 */
u32 get_board_rev(void)
{
	return cl_eeprom_get_board_rev(CONFIG_SYS_I2C_EEPROM_BUS);
};

int misc_init_r(void)
{
	cl_print_pcb_info();
	omap_die_id_display();

	return 0;
}

/*
 * Routine: set_muxconf_regs
 * Description: Setting up the configuration Mux registers specific to the
 *		hardware. Many pins need to be moved from protect to primary
 *		mode.
 */
static void cm_t3x_set_common_muxconf(void)
{
	/* SDRC */
	MUX_VAL(CP(SDRC_D0),		(IEN  | PTD | DIS | M0)); /*SDRC_D0*/
	MUX_VAL(CP(SDRC_D1),		(IEN  | PTD | DIS | M0)); /*SDRC_D1*/
	MUX_VAL(CP(SDRC_D2),		(IEN  | PTD | DIS | M0)); /*SDRC_D2*/
	MUX_VAL(CP(SDRC_D3),		(IEN  | PTD | DIS | M0)); /*SDRC_D3*/
	MUX_VAL(CP(SDRC_D4),		(IEN  | PTD | DIS | M0)); /*SDRC_D4*/
	MUX_VAL(CP(SDRC_D5),		(IEN  | PTD | DIS | M0)); /*SDRC_D5*/
	MUX_VAL(CP(SDRC_D6),		(IEN  | PTD | DIS | M0)); /*SDRC_D6*/
	MUX_VAL(CP(SDRC_D7),		(IEN  | PTD | DIS | M0)); /*SDRC_D7*/
	MUX_VAL(CP(SDRC_D8),		(IEN  | PTD | DIS | M0)); /*SDRC_D8*/
	MUX_VAL(CP(SDRC_D9),		(IEN  | PTD | DIS | M0)); /*SDRC_D9*/
	MUX_VAL(CP(SDRC_D10),		(IEN  | PTD | DIS | M0)); /*SDRC_D10*/
	MUX_VAL(CP(SDRC_D11),		(IEN  | PTD | DIS | M0)); /*SDRC_D11*/
	MUX_VAL(CP(SDRC_D12),		(IEN  | PTD | DIS | M0)); /*SDRC_D12*/
	MUX_VAL(CP(SDRC_D13),		(IEN  | PTD | DIS | M0)); /*SDRC_D13*/
	MUX_VAL(CP(SDRC_D14),		(IEN  | PTD | DIS | M0)); /*SDRC_D14*/
	MUX_VAL(CP(SDRC_D15),		(IEN  | PTD | DIS | M0)); /*SDRC_D15*/
	MUX_VAL(CP(SDRC_D16),		(IEN  | PTD | DIS | M0)); /*SDRC_D16*/
	MUX_VAL(CP(SDRC_D17),		(IEN  | PTD | DIS | M0)); /*SDRC_D17*/
	MUX_VAL(CP(SDRC_D18),		(IEN  | PTD | DIS | M0)); /*SDRC_D18*/
	MUX_VAL(CP(SDRC_D19),		(IEN  | PTD | DIS | M0)); /*SDRC_D19*/
	MUX_VAL(CP(SDRC_D20),		(IEN  | PTD | DIS | M0)); /*SDRC_D20*/
	MUX_VAL(CP(SDRC_D21),		(IEN  | PTD | DIS | M0)); /*SDRC_D21*/
	MUX_VAL(CP(SDRC_D22),		(IEN  | PTD | DIS | M0)); /*SDRC_D22*/
	MUX_VAL(CP(SDRC_D23),		(IEN  | PTD | DIS | M0)); /*SDRC_D23*/
	MUX_VAL(CP(SDRC_D24),		(IEN  | PTD | DIS | M0)); /*SDRC_D24*/
	MUX_VAL(CP(SDRC_D25),		(IEN  | PTD | DIS | M0)); /*SDRC_D25*/
	MUX_VAL(CP(SDRC_D26),		(IEN  | PTD | DIS | M0)); /*SDRC_D26*/
	MUX_VAL(CP(SDRC_D27),		(IEN  | PTD | DIS | M0)); /*SDRC_D27*/
	MUX_VAL(CP(SDRC_D28),		(IEN  | PTD | DIS | M0)); /*SDRC_D28*/
	MUX_VAL(CP(SDRC_D29),		(IEN  | PTD | DIS | M0)); /*SDRC_D29*/
	MUX_VAL(CP(SDRC_D30),		(IEN  | PTD | DIS | M0)); /*SDRC_D30*/
	MUX_VAL(CP(SDRC_D31),		(IEN  | PTD | DIS | M0)); /*SDRC_D31*/
	MUX_VAL(CP(SDRC_CLK),		(IEN  | PTD | DIS | M0)); /*SDRC_CLK*/
	MUX_VAL(CP(SDRC_DQS0),		(IEN  | PTD | DIS | M0)); /*SDRC_DQS0*/
	MUX_VAL(CP(SDRC_DQS1),		(IEN  | PTD | DIS | M0)); /*SDRC_DQS1*/
	MUX_VAL(CP(SDRC_DQS2),		(IEN  | PTD | DIS | M0)); /*SDRC_DQS2*/
	MUX_VAL(CP(SDRC_DQS3),		(IEN  | PTD | DIS | M0)); /*SDRC_DQS3*/
	MUX_VAL(CP(SDRC_CKE0),		(IDIS | PTU | EN  | M0)); /*SDRC_CKE0*/
	MUX_VAL(CP(SDRC_CKE1),		(IDIS | PTD | DIS | M7)); /*SDRC_CKE1*/

	/* GPMC */
	MUX_VAL(CP(GPMC_A1),		(IDIS | PTU | EN  | M0)); /*GPMC_A1*/
	MUX_VAL(CP(GPMC_A2),		(IDIS | PTU | EN  | M0)); /*GPMC_A2*/
	MUX_VAL(CP(GPMC_A3),		(IDIS | PTU | EN  | M0)); /*GPMC_A3*/
	MUX_VAL(CP(GPMC_A4),		(IDIS | PTU | EN  | M0)); /*GPMC_A4*/
	MUX_VAL(CP(GPMC_A5),		(IDIS | PTU | EN  | M0)); /*GPMC_A5*/
	MUX_VAL(CP(GPMC_A6),		(IDIS | PTU | EN  | M0)); /*GPMC_A6*/
	MUX_VAL(CP(GPMC_A7),		(IDIS | PTU | EN  | M0)); /*GPMC_A7*/
	MUX_VAL(CP(GPMC_A8),		(IDIS | PTU | EN  | M0)); /*GPMC_A8*/
	MUX_VAL(CP(GPMC_A9),		(IDIS | PTU | EN  | M0)); /*GPMC_A9*/
	MUX_VAL(CP(GPMC_A10),		(IDIS | PTU | EN  | M0)); /*GPMC_A10*/
	MUX_VAL(CP(GPMC_D0),		(IEN  | PTU | EN  | M0)); /*GPMC_D0*/
	MUX_VAL(CP(GPMC_D1),		(IEN  | PTU | EN  | M0)); /*GPMC_D1*/
	MUX_VAL(CP(GPMC_D2),		(IEN  | PTU | EN  | M0)); /*GPMC_D2*/
	MUX_VAL(CP(GPMC_D3),		(IEN  | PTU | EN  | M0)); /*GPMC_D3*/
	MUX_VAL(CP(GPMC_D4),		(IEN  | PTU | EN  | M0)); /*GPMC_D4*/
	MUX_VAL(CP(GPMC_D5),		(IEN  | PTU | EN  | M0)); /*GPMC_D5*/
	MUX_VAL(CP(GPMC_D6),		(IEN  | PTU | EN  | M0)); /*GPMC_D6*/
	MUX_VAL(CP(GPMC_D7),		(IEN  | PTU | EN  | M0)); /*GPMC_D7*/
	MUX_VAL(CP(GPMC_D8),		(IEN  | PTU | EN  | M0)); /*GPMC_D8*/
	MUX_VAL(CP(GPMC_D9),		(IEN  | PTU | EN  | M0)); /*GPMC_D9*/
	MUX_VAL(CP(GPMC_D10),		(IEN  | PTU | EN  | M0)); /*GPMC_D10*/
	MUX_VAL(CP(GPMC_D11),		(IEN  | PTU | EN  | M0)); /*GPMC_D11*/
	MUX_VAL(CP(GPMC_D12),		(IEN  | PTU | EN  | M0)); /*GPMC_D12*/
	MUX_VAL(CP(GPMC_D13),		(IEN  | PTU | EN  | M0)); /*GPMC_D13*/
	MUX_VAL(CP(GPMC_D14),		(IEN  | PTU | EN  | M0)); /*GPMC_D14*/
	MUX_VAL(CP(GPMC_D15),		(IEN  | PTU | EN  | M0)); /*GPMC_D15*/
	MUX_VAL(CP(GPMC_NCS0),		(IDIS | PTU | EN  | M0)); /*GPMC_nCS0*/

	/* SB-T35 Ethernet */
	MUX_VAL(CP(GPMC_NCS4),		(IEN  | PTU | EN  | M0)); /*GPMC_nCS4*/

	/* DVI enable */
	MUX_VAL(CP(GPMC_NCS3),		(IDIS  | PTU | DIS  | M4));/*GPMC_nCS3*/

	/* DataImage backlight */
	MUX_VAL(CP(GPMC_NCS7),		(IDIS  | PTU | DIS  | M4));/*GPIO_58*/

	/* CM-T3x Ethernet */
	MUX_VAL(CP(GPMC_NCS5),		(IDIS | PTU | DIS | M0)); /*GPMC_nCS5*/
	MUX_VAL(CP(GPMC_CLK),		(IEN  | PTD | DIS | M4)); /*GPIO_59*/
	MUX_VAL(CP(GPMC_NADV_ALE),	(IDIS | PTD | DIS | M0)); /*nADV_ALE*/
	MUX_VAL(CP(GPMC_NOE),		(IDIS | PTD | DIS | M0)); /*nOE*/
	MUX_VAL(CP(GPMC_NWE),		(IDIS | PTD | DIS | M0)); /*nWE*/
	MUX_VAL(CP(GPMC_NBE0_CLE),	(IDIS | PTU | EN  | M0)); /*nBE0_CLE*/
	MUX_VAL(CP(GPMC_NBE1),		(IDIS | PTD | DIS | M4)); /*GPIO_61*/
	MUX_VAL(CP(GPMC_NWP),		(IEN  | PTD | DIS | M0)); /*nWP*/
	MUX_VAL(CP(GPMC_WAIT0),		(IEN  | PTU | EN  | M0)); /*WAIT0*/

	/* DSS */
	MUX_VAL(CP(DSS_PCLK),		(IDIS | PTD | DIS | M0)); /*DSS_PCLK*/
	MUX_VAL(CP(DSS_HSYNC),		(IDIS | PTD | DIS | M0)); /*DSS_HSYNC*/
	MUX_VAL(CP(DSS_VSYNC),		(IDIS | PTD | DIS | M0)); /*DSS_VSYNC*/
	MUX_VAL(CP(DSS_ACBIAS),		(IDIS | PTD | DIS | M0)); /*DSS_ACBIAS*/
	MUX_VAL(CP(DSS_DATA6),		(IDIS | PTD | DIS | M0)); /*DSS_DATA6*/
	MUX_VAL(CP(DSS_DATA7),		(IDIS | PTD | DIS | M0)); /*DSS_DATA7*/
	MUX_VAL(CP(DSS_DATA8),		(IDIS | PTD | DIS | M0)); /*DSS_DATA8*/
	MUX_VAL(CP(DSS_DATA9),		(IDIS | PTD | DIS | M0)); /*DSS_DATA9*/
	MUX_VAL(CP(DSS_DATA10),		(IDIS | PTD | DIS | M0)); /*DSS_DATA10*/
	MUX_VAL(CP(DSS_DATA11),		(IDIS | PTD | DIS | M0)); /*DSS_DATA11*/
	MUX_VAL(CP(DSS_DATA12),		(IDIS | PTD | DIS | M0)); /*DSS_DATA12*/
	MUX_VAL(CP(DSS_DATA13),		(IDIS | PTD | DIS | M0)); /*DSS_DATA13*/
	MUX_VAL(CP(DSS_DATA14),		(IDIS | PTD | DIS | M0)); /*DSS_DATA14*/
	MUX_VAL(CP(DSS_DATA15),		(IDIS | PTD | DIS | M0)); /*DSS_DATA15*/
	MUX_VAL(CP(DSS_DATA16),		(IDIS | PTD | DIS | M0)); /*DSS_DATA16*/
	MUX_VAL(CP(DSS_DATA17),		(IDIS | PTD | DIS | M0)); /*DSS_DATA17*/

	/* serial interface */
	MUX_VAL(CP(UART3_RX_IRRX),	(IEN  | PTD | DIS | M0)); /*UART3_RX*/
	MUX_VAL(CP(UART3_TX_IRTX),	(IDIS | PTD | DIS | M0)); /*UART3_TX*/

	/* mUSB */
	MUX_VAL(CP(HSUSB0_CLK),		(IEN  | PTD | DIS | M0)); /*HSUSB0_CLK*/
	MUX_VAL(CP(HSUSB0_STP),		(IDIS | PTU | EN  | M0)); /*HSUSB0_STP*/
	MUX_VAL(CP(HSUSB0_DIR),		(IEN  | PTD | DIS | M0)); /*HSUSB0_DIR*/
	MUX_VAL(CP(HSUSB0_NXT),		(IEN  | PTD | DIS | M0)); /*HSUSB0_NXT*/
	MUX_VAL(CP(HSUSB0_DATA0),	(IEN  | PTD | DIS | M0)); /*HSUSB0_DATA0*/
	MUX_VAL(CP(HSUSB0_DATA1),	(IEN  | PTD | DIS | M0)); /*HSUSB0_DATA1*/
	MUX_VAL(CP(HSUSB0_DATA2),	(IEN  | PTD | DIS | M0)); /*HSUSB0_DATA2*/
	MUX_VAL(CP(HSUSB0_DATA3),	(IEN  | PTD | DIS | M0)); /*HSUSB0_DATA3*/
	MUX_VAL(CP(HSUSB0_DATA4),	(IEN  | PTD | DIS | M0)); /*HSUSB0_DATA4*/
	MUX_VAL(CP(HSUSB0_DATA5),	(IEN  | PTD | DIS | M0)); /*HSUSB0_DATA5*/
	MUX_VAL(CP(HSUSB0_DATA6),	(IEN  | PTD | DIS | M0)); /*HSUSB0_DATA6*/
	MUX_VAL(CP(HSUSB0_DATA7),	(IEN  | PTD | DIS | M0)); /*HSUSB0_DATA7*/

	/* USB EHCI */
	MUX_VAL(CP(ETK_D0_ES2),		(IEN  | PTD | EN  | M3)); /*HSUSB1_DT0*/
	MUX_VAL(CP(ETK_D1_ES2),		(IEN  | PTD | EN  | M3)); /*HSUSB1_DT1*/
	MUX_VAL(CP(ETK_D2_ES2),		(IEN  | PTD | EN  | M3)); /*HSUSB1_DT2*/
	MUX_VAL(CP(ETK_D7_ES2),		(IEN  | PTD | EN  | M3)); /*HSUSB1_DT3*/
	MUX_VAL(CP(ETK_D4_ES2),		(IEN  | PTD | EN  | M3)); /*HSUSB1_DT4*/
	MUX_VAL(CP(ETK_D5_ES2),		(IEN  | PTD | EN  | M3)); /*HSUSB1_DT5*/
	MUX_VAL(CP(ETK_D6_ES2),		(IEN  | PTD | EN  | M3)); /*HSUSB1_DT6*/
	MUX_VAL(CP(ETK_D3_ES2),		(IEN  | PTD | EN  | M3)); /*HSUSB1_DT7*/
	MUX_VAL(CP(ETK_D8_ES2),		(IEN  | PTD | EN  | M3)); /*HSUSB1_DIR*/
	MUX_VAL(CP(ETK_D9_ES2),		(IEN  | PTD | EN  | M3)); /*HSUSB1_NXT*/
	MUX_VAL(CP(ETK_CTL_ES2),	(IDIS | PTD | DIS | M3)); /*HSUSB1_CLK*/
	MUX_VAL(CP(ETK_CLK_ES2),	(IDIS | PTU | DIS | M3)); /*HSUSB1_STP*/

	MUX_VAL(CP(ETK_D14_ES2),	(IEN  | PTD | EN  | M3)); /*HSUSB2_DT0*/
	MUX_VAL(CP(ETK_D15_ES2),	(IEN  | PTD | EN  | M3)); /*HSUSB2_DT1*/
	MUX_VAL(CP(MCSPI1_CS3),		(IEN  | PTD | EN  | M3)); /*HSUSB2_DT2*/
	MUX_VAL(CP(MCSPI2_CS1),		(IEN  | PTD | EN  | M3)); /*HSUSB2_DT3*/
	MUX_VAL(CP(MCSPI2_SIMO),	(IEN  | PTD | EN  | M3)); /*HSUSB2_DT4*/
	MUX_VAL(CP(MCSPI2_SOMI),	(IEN  | PTD | EN  | M3)); /*HSUSB2_DT5*/
	MUX_VAL(CP(MCSPI2_CS0),		(IEN  | PTD | EN  | M3)); /*HSUSB2_DT6*/
	MUX_VAL(CP(MCSPI2_CLK),		(IEN  | PTD | EN  | M3)); /*HSUSB2_DT7*/
	MUX_VAL(CP(ETK_D12_ES2),	(IEN  | PTD | EN  | M3)); /*HSUSB2_DIR*/
	MUX_VAL(CP(ETK_D13_ES2),	(IEN  | PTD | EN  | M3)); /*HSUSB2_NXT*/
	MUX_VAL(CP(ETK_D10_ES2),	(IDIS | PTD | DIS | M3)); /*HSUSB2_CLK*/
	MUX_VAL(CP(ETK_D11_ES2),	(IDIS | PTU | DIS | M3)); /*HSUSB2_STP*/

	/* SB_T35_USB_HUB_RESET_GPIO */
	MUX_VAL(CP(CAM_WEN),		(IDIS | PTD | DIS | M4)); /*GPIO_167*/

	/* I2C1 */
	MUX_VAL(CP(I2C1_SCL),		(IEN  | PTU | EN  | M0)); /*I2C1_SCL*/
	MUX_VAL(CP(I2C1_SDA),		(IEN  | PTU | EN  | M0)); /*I2C1_SDA*/
	/* I2C2 */
	MUX_VAL(CP(I2C2_SCL),		(IEN  | PTU | EN  | M0)); /*I2C2_SCL*/
	MUX_VAL(CP(I2C2_SDA),		(IEN  | PTU | EN  | M0)); /*I2C2_SDA*/
	/* I2C3 */
	MUX_VAL(CP(I2C3_SCL),		(IEN  | PTU | EN  | M0)); /*I2C3_SCL*/
	MUX_VAL(CP(I2C3_SDA),		(IEN  | PTU | EN  | M0)); /*I2C3_SDA*/

	/* control and debug */
	MUX_VAL(CP(SYS_32K),		(IEN  | PTD | DIS | M0)); /*SYS_32K*/
	MUX_VAL(CP(SYS_CLKREQ),		(IEN  | PTD | DIS | M0)); /*SYS_CLKREQ*/
	MUX_VAL(CP(SYS_NIRQ),		(IEN  | PTU | EN  | M0)); /*SYS_nIRQ*/
	MUX_VAL(CP(SYS_OFF_MODE),	(IEN  | PTD | DIS | M0)); /*OFF_MODE*/
	MUX_VAL(CP(SYS_CLKOUT1),	(IEN  | PTD | DIS | M0)); /*CLKOUT1*/
	MUX_VAL(CP(SYS_CLKOUT2),	(IDIS | PTU | DIS | M4)); /*green LED*/
	MUX_VAL(CP(JTAG_NTRST),		(IEN  | PTD | DIS | M0)); /*JTAG_NTRST*/
	MUX_VAL(CP(JTAG_TCK),		(IEN  | PTD | DIS | M0)); /*JTAG_TCK*/
	MUX_VAL(CP(JTAG_TMS),		(IEN  | PTD | DIS | M0)); /*JTAG_TMS*/
	MUX_VAL(CP(JTAG_TDI),		(IEN  | PTD | DIS | M0)); /*JTAG_TDI*/

	/* MMC1 */
	MUX_VAL(CP(MMC1_CLK),		(IDIS | PTU | EN  | M0)); /*MMC1_CLK*/
	MUX_VAL(CP(MMC1_CMD),		(IEN  | PTU | EN  | M0)); /*MMC1_CMD*/
	MUX_VAL(CP(MMC1_DAT0),		(IEN  | PTU | EN  | M0)); /*MMC1_DAT0*/
	MUX_VAL(CP(MMC1_DAT1),		(IEN  | PTU | EN  | M0)); /*MMC1_DAT1*/
	MUX_VAL(CP(MMC1_DAT2),		(IEN  | PTU | EN  | M0)); /*MMC1_DAT2*/
	MUX_VAL(CP(MMC1_DAT3),		(IEN  | PTU | EN  | M0)); /*MMC1_DAT3*/

	/* SPI */
	MUX_VAL(CP(MCBSP1_CLKR),	(IEN | PTD | DIS | M1)); /*MCSPI4_CLK*/
	MUX_VAL(CP(MCBSP1_DX),		(IEN | PTD | DIS | M1)); /*MCSPI4_SIMO*/
	MUX_VAL(CP(MCBSP1_DR),		(IEN | PTD | DIS | M1)); /*MCSPI4_SOMI*/
	MUX_VAL(CP(MCBSP1_FSX),		(IEN | PTU | EN  | M1)); /*MCSPI4_CS0*/

	/* display controls */
	MUX_VAL(CP(MCBSP1_FSR),		(IDIS | PTU | DIS | M4)); /*GPIO_157*/
}

static void cm_t35_set_muxconf(void)
{
	/* DSS */
	MUX_VAL(CP(DSS_DATA0),		(IDIS | PTD | DIS | M0)); /*DSS_DATA0*/
	MUX_VAL(CP(DSS_DATA1),		(IDIS | PTD | DIS | M0)); /*DSS_DATA1*/
	MUX_VAL(CP(DSS_DATA2),		(IDIS | PTD | DIS | M0)); /*DSS_DATA2*/
	MUX_VAL(CP(DSS_DATA3),		(IDIS | PTD | DIS | M0)); /*DSS_DATA3*/
	MUX_VAL(CP(DSS_DATA4),		(IDIS | PTD | DIS | M0)); /*DSS_DATA4*/
	MUX_VAL(CP(DSS_DATA5),		(IDIS | PTD | DIS | M0)); /*DSS_DATA5*/

	MUX_VAL(CP(DSS_DATA18),         (IDIS | PTD | DIS | M0)); /*DSS_DATA18*/
	MUX_VAL(CP(DSS_DATA19),         (IDIS | PTD | DIS | M0)); /*DSS_DATA19*/
	MUX_VAL(CP(DSS_DATA20),         (IDIS | PTD | DIS | M0)); /*DSS_DATA20*/
	MUX_VAL(CP(DSS_DATA21),         (IDIS | PTD | DIS | M0)); /*DSS_DATA21*/
	MUX_VAL(CP(DSS_DATA22),         (IDIS | PTD | DIS | M0)); /*DSS_DATA22*/
	MUX_VAL(CP(DSS_DATA23),         (IDIS | PTD | DIS | M0)); /*DSS_DATA23*/

	/* MMC1 */
	MUX_VAL(CP(MMC1_DAT4),		(IEN  | PTU | EN  | M0)); /*MMC1_DAT4*/
	MUX_VAL(CP(MMC1_DAT5),		(IEN  | PTU | EN  | M0)); /*MMC1_DAT5*/
	MUX_VAL(CP(MMC1_DAT6),		(IEN  | PTU | EN  | M0)); /*MMC1_DAT6*/
	MUX_VAL(CP(MMC1_DAT7),		(IEN  | PTU | EN  | M0)); /*MMC1_DAT7*/
}

static void cm_t3730_set_muxconf(void)
{
	/* DSS */
	MUX_VAL(CP(DSS_DATA18),		(IDIS | PTD | DIS | M3)); /*DSS_DATA0*/
	MUX_VAL(CP(DSS_DATA19),		(IDIS | PTD | DIS | M3)); /*DSS_DATA1*/
	MUX_VAL(CP(DSS_DATA20),		(IDIS | PTD | DIS | M3)); /*DSS_DATA2*/
	MUX_VAL(CP(DSS_DATA21),		(IDIS | PTD | DIS | M3)); /*DSS_DATA3*/
	MUX_VAL(CP(DSS_DATA22),		(IDIS | PTD | DIS | M3)); /*DSS_DATA4*/
	MUX_VAL(CP(DSS_DATA23),		(IDIS | PTD | DIS | M3)); /*DSS_DATA5*/

	MUX_VAL(CP(SYS_BOOT0),		(IDIS | PTD | DIS | M3)); /*DSS_DATA18*/
	MUX_VAL(CP(SYS_BOOT1),		(IDIS | PTD | DIS | M3)); /*DSS_DATA19*/
	MUX_VAL(CP(SYS_BOOT3),		(IDIS | PTD | DIS | M3)); /*DSS_DATA20*/
	MUX_VAL(CP(SYS_BOOT4),		(IDIS | PTD | DIS | M3)); /*DSS_DATA21*/
	MUX_VAL(CP(SYS_BOOT5),		(IDIS | PTD | DIS | M3)); /*DSS_DATA22*/
	MUX_VAL(CP(SYS_BOOT6),		(IDIS | PTD | DIS | M3)); /*DSS_DATA23*/
}

void set_muxconf_regs(void)
{
	cm_t3x_set_common_muxconf();

	if (get_cpu_family() == CPU_OMAP34XX)
		cm_t35_set_muxconf();
	else
		cm_t3730_set_muxconf();
}

#if defined(CONFIG_MMC)
#define SB_T35_WP_GPIO 59

int board_mmc_getcd(struct mmc *mmc)
{
	u8 val;

	if (twl4030_i2c_read_u8(TWL4030_CHIP_GPIO, TWL4030_BASEADD_GPIO, &val))
		return -1;

	return !(val & 1);
}

int board_mmc_init(bd_t *bis)
{
	return omap_mmc_init(0, 0, 0, -1, SB_T35_WP_GPIO);
}
#endif

#if defined(CONFIG_MMC)
void board_mmc_power_init(void)
{
	twl4030_power_mmc_init(0);
}
#endif

#ifdef CONFIG_SYS_I2C_OMAP24XX
/*
 * Routine: reset_net_chip
 * Description: reset the Ethernet controller via TPS65930 GPIO
 */
static int cm_t3x_reset_net_chip(int gpio)
{
	/* Set GPIO1 of TPS65930 as output */
	twl4030_i2c_write_u8(TWL4030_CHIP_GPIO, TWL4030_BASEADD_GPIO + 0x03,
			     0x02);
	/* Send a pulse on the GPIO pin */
	twl4030_i2c_write_u8(TWL4030_CHIP_GPIO, TWL4030_BASEADD_GPIO + 0x0C,
			     0x02);
	udelay(1);
	twl4030_i2c_write_u8(TWL4030_CHIP_GPIO, TWL4030_BASEADD_GPIO + 0x09,
			     0x02);
	mdelay(40);
	twl4030_i2c_write_u8(TWL4030_CHIP_GPIO, TWL4030_BASEADD_GPIO + 0x0C,
			     0x02);
	mdelay(1);
	return 0;
}
#else
static inline int cm_t3x_reset_net_chip(int gpio) { return 0; }
#endif

#ifdef CONFIG_SMC911X
/*
 * Routine: handle_mac_address
 * Description: prepare MAC address for on-board Ethernet.
 */
static int handle_mac_address(void)
{
	unsigned char enetaddr[6];
	int rc;

	rc = eth_env_get_enetaddr("ethaddr", enetaddr);
	if (rc)
		return 0;

	rc = cl_eeprom_read_mac_addr(enetaddr, CONFIG_SYS_I2C_EEPROM_BUS);
	if (rc)
		return rc;

	if (!is_valid_ethaddr(enetaddr))
		return -1;

	return eth_env_set_enetaddr("ethaddr", enetaddr);
}

/*
 * Routine: board_eth_init
 * Description: initialize module and base-board Ethernet chips
 */
#define SB_T35_SMC911X_BASE	(CONFIG_SMC911X_BASE + SZ_16M)
int board_eth_init(bd_t *bis)
{
	int rc = 0, rc1 = 0;

	rc1 = handle_mac_address();
	if (rc1)
		printf("No MAC address found! ");

	rc1 = cl_omap3_smc911x_init(0, 5, CONFIG_SMC911X_BASE,
				    cm_t3x_reset_net_chip, -EINVAL);
	if (rc1 > 0)
		rc++;

	rc1 = cl_omap3_smc911x_init(1, 4, SB_T35_SMC911X_BASE, NULL, -EINVAL);
	if (rc1 > 0)
		rc++;

	return rc;
}
#endif

#ifdef CONFIG_USB_EHCI_OMAP
struct omap_usbhs_board_data usbhs_bdata = {
	.port_mode[0] = OMAP_EHCI_PORT_MODE_PHY,
	.port_mode[1] = OMAP_EHCI_PORT_MODE_PHY,
	.port_mode[2] = OMAP_USBHS_PORT_MODE_UNUSED,
};

#define SB_T35_USB_HUB_RESET_GPIO	167
int ehci_hcd_init(int index, enum usb_init_type init,
		  struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	u8 val;
	int offset;

	cl_usb_hub_init(SB_T35_USB_HUB_RESET_GPIO, "sb-t35 hub rst");

	offset = TWL4030_BASEADD_GPIO + TWL4030_GPIO_GPIODATADIR1;
	twl4030_i2c_read_u8(TWL4030_CHIP_GPIO, offset, &val);
	/* Set GPIO6 and GPIO7 of TPS65930 as output */
	val |= 0xC0;
	twl4030_i2c_write_u8(TWL4030_CHIP_GPIO, offset, val);
	offset = TWL4030_BASEADD_GPIO + TWL4030_GPIO_SETGPIODATAOUT1;
	/* Take both PHYs out of reset */
	twl4030_i2c_write_u8(TWL4030_CHIP_GPIO, offset, 0xC0);
	udelay(1);

	return omap_ehci_hcd_init(index, &usbhs_bdata, hccr, hcor);
}

int ehci_hcd_stop(void)
{
	cl_usb_hub_deinit(SB_T35_USB_HUB_RESET_GPIO);
	return omap_ehci_hcd_stop();
}
#endif /* CONFIG_USB_EHCI_OMAP */
