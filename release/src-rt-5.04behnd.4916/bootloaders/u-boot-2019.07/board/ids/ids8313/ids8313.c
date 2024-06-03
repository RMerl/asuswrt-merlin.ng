// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Based on:
 * Copyright (c) 2011 IDS GmbH, Germany
 * ids8313.c - ids8313 board support.
 *
 * Sergej Stepanov <ste@ids.de>
 * Based on board/freescale/mpc8313erdb/mpc8313erdb.c
 */

#include <common.h>
#include <mpc83xx.h>
#include <spi.h>
#include <linux/libfdt.h>

DECLARE_GLOBAL_DATA_PTR;
/** CPLD contains the info about:
 * - board type: *pCpld & 0xF0
 * - hw-revision: *pCpld & 0x0F
 * - cpld-revision: *pCpld+1
 */
int checkboard(void)
{
	char *pcpld = (char *)CONFIG_SYS_CPLD_BASE;
	u8 u8Vers = readb(pcpld);
	u8 u8Revs = readb(pcpld + 1);

	printf("Board: ");
	switch (u8Vers & 0xF0) {
	case '\x40':
		printf("CU73X");
		break;
	case '\x50':
		printf("CC73X");
		break;
	default:
		printf("unknown(0x%02X, 0x%02X)\n", u8Vers, u8Revs);
		return 0;
	}
	printf("\nInfo:  HW-Rev: %i, CPLD-Rev: %i\n",
	       u8Vers & 0x0F, u8Revs & 0xFF);
	return 0;
}

/*
 *  fixed sdram init
 */
int fixed_sdram(unsigned long config)
{
	immap_t *im = (immap_t *)CONFIG_SYS_IMMR;
	u32 msize = CONFIG_SYS_DDR_SIZE << 20;

#ifndef CONFIG_SYS_RAMBOOT
	u32 msize_log2 = __ilog2(msize);

	out_be32(&im->sysconf.ddrlaw[0].bar,
		 (CONFIG_SYS_SDRAM_BASE & 0xfffff000));
	out_be32(&im->sysconf.ddrlaw[0].ar, LBLAWAR_EN | (msize_log2 - 1));
	out_be32(&im->sysconf.ddrcdr, CONFIG_SYS_DDRCDR_VALUE);
	sync();

	/*
	 * Erratum DDR3 requires a 50ms delay after clearing DDRCDR[DDR_cfg],
	 * or the DDR2 controller may fail to initialize correctly.
	 */
	udelay(50000);

	out_be32(&im->ddr.csbnds[0].csbnds, (msize - 1) >> 24);
	out_be32(&im->ddr.cs_config[0], config);

	/* currently we use only one CS, so disable the other banks */
	out_be32(&im->ddr.cs_config[1], 0);
	out_be32(&im->ddr.cs_config[2], 0);
	out_be32(&im->ddr.cs_config[3], 0);

	out_be32(&im->ddr.timing_cfg_3, CONFIG_SYS_DDR_TIMING_3);
	out_be32(&im->ddr.timing_cfg_1, CONFIG_SYS_DDR_TIMING_1);
	out_be32(&im->ddr.timing_cfg_2, CONFIG_SYS_DDR_TIMING_2);
	out_be32(&im->ddr.timing_cfg_0, CONFIG_SYS_DDR_TIMING_0);

	out_be32(&im->ddr.sdram_cfg, CONFIG_SYS_SDRAM_CFG);
	out_be32(&im->ddr.sdram_cfg2, CONFIG_SYS_SDRAM_CFG2);

	out_be32(&im->ddr.sdram_mode, CONFIG_SYS_DDR_MODE);
	out_be32(&im->ddr.sdram_mode2, CONFIG_SYS_DDR_MODE_2);

	out_be32(&im->ddr.sdram_interval, CONFIG_SYS_DDR_INTERVAL);
	out_be32(&im->ddr.sdram_clk_cntl, CONFIG_SYS_DDR_CLK_CNTL);
	sync();
	udelay(300);

	/* enable DDR controller */
	setbits_be32(&im->ddr.sdram_cfg, SDRAM_CFG_MEM_EN);
	/* now check the real size */
	disable_addr_trans();
	msize = get_ram_size(CONFIG_SYS_SDRAM_BASE, msize);
	enable_addr_trans();
#endif
	return msize;
}

static int setup_sdram(void)
{
	u32 msize = CONFIG_SYS_DDR_SIZE << 20;
	long int size_01, size_02;

	size_01 = fixed_sdram(CONFIG_SYS_DDR_CONFIG);
	size_02 = fixed_sdram(CONFIG_SYS_DDR_CONFIG_256);

	if (size_01 > size_02)
		msize = fixed_sdram(CONFIG_SYS_DDR_CONFIG);
	else
		msize = size_02;

	return msize;
}

int dram_init(void)
{
	immap_t *im = (immap_t *)CONFIG_SYS_IMMR;
	fsl_lbc_t *lbc = &im->im_lbc;
	u32 msize = 0;

	if ((in_be32(&im->sysconf.immrbar) & IMMRBAR_BASE_ADDR) != (u32)im)
		return -ENXIO;

	msize = setup_sdram();

	out_be32(&lbc->lbcr, (0x00040000 | (0xFF << LBCR_BMT_SHIFT) | 0xF));
	out_be32(&lbc->mrtpr, 0x20000000);
	sync();

	gd->ram_size = msize;

	return 0;
}

#if defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);

	return 0;
}
#endif

/* gpio mask for spi_cs */
#define IDSCPLD_SPI_CS_MASK		0x00000001
/* spi_cs multiplexed through cpld */
#define IDSCPLD_SPI_CS_BASE		(CONFIG_SYS_CPLD_BASE + 0xf)

#if defined(CONFIG_MISC_INIT_R)
/* srp umcr mask for rts */
#define IDSUMCR_RTS_MASK 0x04
int misc_init_r(void)
{
	/*srp*/
	duart83xx_t *uart1 = &((immap_t *)CONFIG_SYS_IMMR)->duart[0];
	duart83xx_t *uart2 = &((immap_t *)CONFIG_SYS_IMMR)->duart[1];

	gpio83xx_t *iopd = &((immap_t *)CONFIG_SYS_IMMR)->gpio[0];
	u8 *spi_base = (u8 *)IDSCPLD_SPI_CS_BASE;

	/* deactivate spi_cs channels */
	out_8(spi_base, 0);
	/* deactivate the spi_cs */
	setbits_be32(&iopd->dir, IDSCPLD_SPI_CS_MASK);
	/*srp - deactivate rts*/
	out_8(&uart1->umcr, IDSUMCR_RTS_MASK);
	out_8(&uart2->umcr, IDSUMCR_RTS_MASK);


	gd->fdt_blob = (void *)CONFIG_SYS_FLASH_BASE;
	return 0;
}
#endif

#ifdef CONFIG_MPC8XXX_SPI
/*
 * The following are used to control the SPI chip selects
 */
int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	return bus == 0 && ((cs >= 0) && (cs <= 2));
}

void spi_cs_activate(struct spi_slave *slave)
{
	gpio83xx_t *iopd = &((immap_t *)CONFIG_SYS_IMMR)->gpio[0];
	u8 *spi_base = (u8 *)IDSCPLD_SPI_CS_BASE;

	/* select the spi_cs channel */
	out_8(spi_base, 1 << slave->cs);
	/* activate the spi_cs */
	clrbits_be32(&iopd->dat, IDSCPLD_SPI_CS_MASK);
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	gpio83xx_t *iopd = &((immap_t *)CONFIG_SYS_IMMR)->gpio[0];
	u8 *spi_base = (u8 *)IDSCPLD_SPI_CS_BASE;

	/* select the spi_cs channel */
	out_8(spi_base, 1 << slave->cs);
	/* deactivate the spi_cs */
	setbits_be32(&iopd->dat, IDSCPLD_SPI_CS_MASK);
}
#endif
