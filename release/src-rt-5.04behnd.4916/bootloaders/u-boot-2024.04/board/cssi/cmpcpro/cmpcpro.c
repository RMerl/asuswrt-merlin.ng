// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2006-2023  CS GROUP France
 */

#include <command.h>
#include <dm.h>
#include <env.h>
#include <env_internal.h>
#include <eeprom.h>
#include <fdt_support.h>
#include <hang.h>
#include <ioports.h>
#include <mpc83xx.h>
#include <netdev.h>
#include <spi.h>
#include <stdarg.h>
#include <stdlib.h>

#include <linux/delay.h>
#include <linux/immap_qe.h>
#include <linux/libfdt.h>
#include <linux/log2.h>
#include <linux/sizes.h>

#include <asm/io.h>
#include <asm/global_data.h>
#include <asm/mmu.h>

#include <u-boot/crc.h>

#include "../common/common.h"

DECLARE_GLOBAL_DATA_PTR;

#define ADDR_FPGA_BASE		((unsigned char __iomem *)CONFIG_CPLD_BASE)
#define ADDR_FPGA_RESET_G	(ADDR_FPGA_BASE + 0x40)
#define ADDR_FPGA_REG_ETAT	(ADDR_FPGA_BASE + 0x42)

#define R_ETAT_PRES_BASE	0x01
#define RESET_G_OK		0x08

/* SPI EEPROM parameters */
#define MAX_SPI_BYTES	0x28
#define EE_OFF_MAC1	0x10
#define EE_OFF_MAC2	0x16
#define EE_OFF_MAC3	0x1C

static uint upma_table[] = {
	/* Read Single-Beat (RSS) */
	0x00AC0C00, 0x00FC1C40, 0x30FCE045, 0xFFFF0C00,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	/* Read Burst (RBS) */
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	/* Write Single-Beat (WSS) */
	0x00A30C00, 0x00F31C40, 0x3FF3C045, 0xFFFF0C00,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	/* Write Burst (WBS) */
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	/* Refresh Timer (RTS) */
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	/* Exception Condition (EXS) */
	0xFFFF0C01, 0xFFFF0C01, 0xFFFF0C01, 0xFFFF0C01,
};

const qe_iop_conf_t qe_iop_conf_tab[] = {
	/* ETH3 */
	{1,  0, 1, 0, 1}, /* TxD0 */
	{1,  1, 1, 0, 1}, /* TxD1 */
	{1,  2, 1, 0, 1}, /* TxD2 */
	{1,  3, 1, 0, 1}, /* TxD3 */
	{1,  9, 1, 0, 1}, /* TxER */
	{1, 12, 1, 0, 1}, /* TxEN */
	{3, 24, 2, 0, 1}, /* TxCLK->CLK10 */

	{1,  4, 2, 0, 1}, /* RxD0 */
	{1,  5, 2, 0, 1}, /* RxD1 */
	{1,  6, 2, 0, 1}, /* RxD2 */
	{1,  7, 2, 0, 1}, /* RxD3 */
	{1,  8, 2, 0, 1}, /* RxER */
	{1, 10, 2, 0, 1}, /* RxDV */
	{0, 13, 2, 0, 1}, /* RxCLK->CLK9 */
	{1, 11, 2, 0, 1}, /* COL */
	{1, 13, 2, 0, 1}, /* CRS */

	/* ETH4 */
	{1, 18, 1, 0, 1}, /* TxD0 */
	{1, 19, 1, 0, 1}, /* TxD1 */
	{1, 20, 1, 0, 1}, /* TxD2 */
	{1, 21, 1, 0, 1}, /* TxD3 */
	{1, 27, 1, 0, 1}, /* TxER */
	{1, 30, 1, 0, 1}, /* TxEN */
	{3,  6, 2, 0, 1}, /* TxCLK->CLK8 */

	{1, 22, 2, 0, 1}, /* RxD0 */
	{1, 23, 2, 0, 1}, /* RxD1 */
	{1, 24, 2, 0, 1}, /* RxD2 */
	{1, 25, 2, 0, 1}, /* RxD3 */
	{1, 26, 1, 0, 1}, /* RxER */
	{1, 28, 2, 0, 1}, /* Rx_DV */
	{3, 31, 2, 0, 1}, /* RxCLK->CLK7 */
	{1, 29, 2, 0, 1}, /* COL */
	{1, 31, 2, 0, 1}, /* CRS */

	{3,  4, 3, 0, 2}, /* MDIO */
	{3,  5, 1, 0, 2}, /* MDC */

	{0,  0, 0, 0, QE_IOP_TAB_END}, /* END of table */
};

void iop_setup_miae(void)
{
	immap_t __iomem *im = (immap_t *)CONFIG_SYS_IMMR;

	/* PORTA configuration */
	out_be32(&im->qepio.ioport[0].pdat, 0x00808000);
	out_be32(&im->qepio.ioport[0].podr, 0x00008000);
	out_be32(&im->qepio.ioport[0].dir1, 0x40800968);
	out_be32(&im->qepio.ioport[0].dir2, 0x650A0896);
	out_be32(&im->qepio.ioport[0].ppar1, 0x40400204);
	out_be32(&im->qepio.ioport[0].ppar2, 0x05050464);

	/* PORTB configuration */
	out_be32(&im->qepio.ioport[1].pdat, 0x00018000);
	out_be32(&im->qepio.ioport[1].podr, 0x00000000);
	out_be32(&im->qepio.ioport[1].dir1, 0x50A08949);
	out_be32(&im->qepio.ioport[1].dir2, 0x5C0C6890);
	out_be32(&im->qepio.ioport[1].ppar1, 0x50504644);
	out_be32(&im->qepio.ioport[1].ppar2, 0x080800A0);

	/* PORTC configuration */
	out_be32(&im->qepio.ioport[2].pdat, 0x3D000108);
	out_be32(&im->qepio.ioport[2].podr, 0x00000000);
	out_be32(&im->qepio.ioport[2].dir1, 0x45518000);
	out_be32(&im->qepio.ioport[2].dir2, 0xA8119561);
	out_be32(&im->qepio.ioport[2].ppar1, 0x80008000);
	out_be32(&im->qepio.ioport[2].ppar2, 0x00000000);

	/* PORTD configuration */
	out_be32(&im->qepio.ioport[3].pdat, 0x1000E000);
	out_be32(&im->qepio.ioport[3].podr, 0x0000E000);
	out_be32(&im->qepio.ioport[3].dir1, 0xFDD20800);
	out_be32(&im->qepio.ioport[3].dir2, 0x54155228);
	out_be32(&im->qepio.ioport[3].ppar1, 0x54A30C00);
	out_be32(&im->qepio.ioport[3].ppar2, 0x00000100);
}

void iop_setup_mcr(void)
{
	immap_t __iomem *im = (immap_t *)CONFIG_SYS_IMMR;

	/* PORTA configuration */
	out_be32(&im->qepio.ioport[0].pdat, 0x00808004);
	out_be32(&im->qepio.ioport[0].podr, 0x00000000);
	out_be32(&im->qepio.ioport[0].dir1, 0x40800A68);
	out_be32(&im->qepio.ioport[0].dir2, 0x650A0896);
	out_be32(&im->qepio.ioport[0].ppar1, 0x40400004);
	out_be32(&im->qepio.ioport[0].ppar2, 0x05050444);

	/* PORTB configuration */
	out_be32(&im->qepio.ioport[1].pdat, 0x00008000);
	out_be32(&im->qepio.ioport[1].podr, 0x00000004);
	out_be32(&im->qepio.ioport[1].dir1, 0x50A08A4A);
	out_be32(&im->qepio.ioport[1].dir2, 0x5C0C6890);
	out_be32(&im->qepio.ioport[1].ppar1, 0x50504444);
	out_be32(&im->qepio.ioport[1].ppar2, 0x08080080);

	/* PORTC configuration */
	out_be32(&im->qepio.ioport[2].pdat, 0x3D000018);
	out_be32(&im->qepio.ioport[2].podr, 0x00000400);
	out_be32(&im->qepio.ioport[2].dir1, 0x45518000);
	out_be32(&im->qepio.ioport[2].dir2, 0xA8129561);
	out_be32(&im->qepio.ioport[2].ppar1, 0x80008000);
	out_be32(&im->qepio.ioport[2].ppar2, 0x00000000);

	/* PORTD configuration */
	out_be32(&im->qepio.ioport[3].pdat, 0x1000E000);
	out_be32(&im->qepio.ioport[3].podr, 0x0000E000);
	out_be32(&im->qepio.ioport[3].dir1, 0xFDD20800);
	out_be32(&im->qepio.ioport[3].dir2, 0x54155228);
	out_be32(&im->qepio.ioport[3].ppar1, 0x54A30C00);
	out_be32(&im->qepio.ioport[3].ppar2, 0x00000100);
}

static void iop_setup_cmpcpro(void)
{
	immap_t __iomem *im = (immap_t *)CONFIG_SYS_IMMR;

	/* PORTA configuration */
	out_be32(&im->qepio.ioport[0].pdat, 0x00000000);
	out_be32(&im->qepio.ioport[0].podr, 0x00000000);
	out_be32(&im->qepio.ioport[0].dir1, 0x50A84020);
	out_be32(&im->qepio.ioport[0].dir2, 0x00000000);
	out_be32(&im->qepio.ioport[0].ppar1, 0xF0FCC000);
	out_be32(&im->qepio.ioport[0].ppar2, 0x00000000);

	/* PORTB configuration */
	out_be32(&im->qepio.ioport[1].pdat, 0x00000000);
	out_be32(&im->qepio.ioport[1].podr, 0x00000000);
	out_be32(&im->qepio.ioport[1].dir1, 0x00000000);
	out_be32(&im->qepio.ioport[1].dir2, 0x00006800);
	out_be32(&im->qepio.ioport[1].ppar1, 0x00000000);
	out_be32(&im->qepio.ioport[1].ppar2, 0x00000000);

	/* PORTC configuration */
	out_be32(&im->qepio.ioport[2].pdat, 0x19000000);
	out_be32(&im->qepio.ioport[2].podr, 0x00000000);
	out_be32(&im->qepio.ioport[2].dir1, 0x01410000);
	out_be32(&im->qepio.ioport[2].dir2, 0xA8009400);
	out_be32(&im->qepio.ioport[2].ppar1, 0x00000000);
	out_be32(&im->qepio.ioport[2].ppar2, 0x00000000);

	/* PORTD configuration */
	out_be32(&im->qepio.ioport[3].pdat, 0x1000E000);
	out_be32(&im->qepio.ioport[3].podr, 0x0000E000);
	out_be32(&im->qepio.ioport[3].dir1, 0xFD020000);
	out_be32(&im->qepio.ioport[3].dir2, 0x54055000);
	out_be32(&im->qepio.ioport[3].ppar1, 0x54030000);
	out_be32(&im->qepio.ioport[3].ppar2, 0x00000000);
}

int board_early_init_r(void)
{
	immap_t __iomem *im = (immap_t *)CONFIG_SYS_IMMR;
	fsl_lbc_t *lbus = &im->im_lbc;

	upmconfig(UPMA, upma_table, ARRAY_SIZE(upma_table));

	out_be32(&lbus->mamr, 0x00044440);

	/* configure LBCR register */
	out_be32(&lbus->lbcr, 0x00000500);
	sync();

	if (in_8(ADDR_FPGA_REG_ETAT) & R_ETAT_PRES_BASE) {
		int i;

		/* Initialize signal PROG_FPGA_FIRMWARE */
		setbits_be32(&im->qepio.ioport[0].pdat, 0x00008000);
		setbits_be32(&im->qepio.ioport[0].dir2, 0x60000002);
		setbits_be32(&im->qepio.ioport[0].podr, 0x00008000);

		mdelay(1);

		/* Now read CPDATA[31] to check if FPGA is loaded */
		if (!in_be32(&im->qepio.ioport[0].pdat) & 0x00000001) {
			printf("Reloading FPGA firmware.\n");

			clrbits_be32(&im->qepio.ioport[0].pdat, 0x00008000);
			udelay(1);
			setbits_be32(&im->qepio.ioport[0].pdat, 0x00008000);

			/* Wait 200 msec and check DONE_FPGA_FIRMWARE */
			mdelay(200);
			if (!(in_be32(&im->qepio.ioport[0].pdat) & 0x00000001)) {
				for (;;) {
					printf("error loading firmware.\n");
					mdelay(500);
				}
			}

			/* Send a reset signal and wait for 20 msec */
			out_8(ADDR_FPGA_RESET_G, in_8(ADDR_FPGA_RESET_G) | RESET_G_OK);
			mdelay(20);
			out_8(ADDR_FPGA_RESET_G, in_8(ADDR_FPGA_RESET_G) & ~RESET_G_OK);
		}

		/* Wait 300 msec and check the reset state */
		mdelay(300);
		for (i = 0; !(in_8(ADDR_FPGA_REG_ETAT) & RESET_G_OK); i++) {
			for (;;) {
				printf("Could not reset FPGA.\n");
				mdelay(500);
			}
		}

		iop_setup_common();

		/* clocks configuration */
		out_be32(&qe_immr->qmx.cmxsi1cr_l, 0x00040004);
		out_be32(&qe_immr->qmx.cmxsi1syr, 0x00000000);
	} else {
		iop_setup_cmpcpro();
	}

	return 0;
}

int dram_init(int board_type)
{
	immap_t __iomem *im = (immap_t __iomem *)CONFIG_SYS_IMMR;

	out_be32(&im->sysconf.ddrlaw[0].bar, CFG_SYS_DDR_SDRAM_BASE & LAWBAR_BAR);
	out_be32(&im->sysconf.ddrlaw[0].ar, LAWAR_EN | ((ilog2(SZ_512M) - 1) & LAWAR_SIZE));

	out_be32(&im->ddr.sdram_clk_cntl, CFG_SYS_DDR_CLK_CNTL);
	out_be32(&im->ddr.csbnds[0].csbnds, CFG_SYS_DDR_CS0_BNDS);
	out_be32(&im->ddr.cs_config[0], CFG_SYS_DDR_CS0_CONFIG);

	out_be32(&im->ddr.timing_cfg_0, CFG_SYS_DDR_TIMING_0);
	out_be32(&im->ddr.timing_cfg_1, CFG_SYS_DDR_TIMING_1);
	out_be32(&im->ddr.timing_cfg_2, CFG_SYS_DDR_TIMING_2);
	out_be32(&im->ddr.timing_cfg_3, CFG_SYS_DDR_TIMING_3);
	out_be32(&im->ddr.sdram_cfg, CFG_SYS_DDR_SDRAM_CFG);
	out_be32(&im->ddr.sdram_cfg2, CFG_SYS_DDR_SDRAM_CFG2);
	out_be32(&im->ddr.sdram_mode, CFG_SYS_DDR_MODE);
	out_be32(&im->ddr.sdram_mode2, CFG_SYS_DDR_MODE2);
	out_be32(&im->ddr.sdram_interval, CFG_SYS_DDR_INTERVAL);
	udelay(200);

	setbits_be32(&im->ddr.sdram_cfg, SDRAM_CFG_MEM_EN);

	gd->ram_size = SZ_512M;

	return 0;
}

int checkboard(void)
{
	printf("Board: ");

	/* Is a motherboard present ? */
	if (in_8(ADDR_FPGA_REG_ETAT) & R_ETAT_PRES_BASE)
		return checkboard_common();

	printf("CMPCPRO (CS GROUP)\n");

	return 0;
}

/* Reads MAC addresses from SPI EEPROM */
static int setup_mac(void)
{
	uchar din[MAX_SPI_BYTES];
	int ret;
	unsigned long ident = 0x08005120;

	ret = read_eeprom(din, sizeof(din));
	if (ret)
		return ret;

	if (memcmp(din + EE_OFF_MAC1, &ident, sizeof(ident)) == 0) {
		eth_env_set_enetaddr("ethaddr", din + EE_OFF_MAC1);
		eth_env_set_enetaddr("eth3addr", din + EE_OFF_MAC1);
	}

	if (memcmp(din + EE_OFF_MAC2, &ident, sizeof(ident)) == 0)
		eth_env_set_enetaddr("eth1addr", din + EE_OFF_MAC2);

	if (memcmp(din + EE_OFF_MAC3, &ident, sizeof(ident)) == 0)
		eth_env_set_enetaddr("eth2addr", din + EE_OFF_MAC3);

	return 0;
}

int misc_init_r(void)
{
	/* we do not modify environment variable area if CRC is false */
	/* Verify if mother board is present */
	if (in_8(ADDR_FPGA_REG_ETAT) & R_ETAT_PRES_BASE) {
		misc_init_r_common();
	} else {
		env_set("config", CFG_BOARD_CMPCXXX);
		env_set("hostname", CFG_BOARD_CMPCXXX);
	}

	if (setup_mac())
		printf("Error retrieving mac addresses\n");

	return 0;
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
	ft_cpu_setup(blob, bd);

	/* MIAE only */
	if (!(in_8(ADDR_FPGA_REG_ETAT) & R_ETAT_PRES_BASE))
		return 0;

	return ft_board_setup_common(blob);
}

void ft_board_setup_phy3(void)
{
	/* switch to phy3 with gpio, we'll only use phy3 */
	immap_t *immr = (immap_t *)CONFIG_SYS_IMMR;

	setbits_be32(&immr->qepio.ioport[2].pdat, 0x00000400);
}
