// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2010 Freescale Semiconductor, Inc.
 * Copyright (C) 2010 Ilya Yanok, Emcraft Systems, yanok@emcraft.com
 */

#include <common.h>
#include <hwconfig.h>
#include <i2c.h>
#include <spi.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <pci.h>
#include <mpc83xx.h>
#include <vsc7385.h>
#include <netdev.h>
#include <fsl_esdhc.h>
#include <asm/io.h>
#include <asm/fsl_serdes.h>
#include <asm/fsl_mpc83xx_serdes.h>

/*
 * The following are used to control the SPI chip selects for the SPI command.
 */
#ifdef CONFIG_MPC8XXX_SPI

#define SPI_CS_MASK	0x00400000

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	return bus == 0 && cs == 0;
}

void spi_cs_activate(struct spi_slave *slave)
{
	immap_t *immr = (immap_t *)CONFIG_SYS_IMMR;

	/* active low */
	clrbits_be32(&immr->gpio[0].dat, SPI_CS_MASK);
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	immap_t *immr = (immap_t *)CONFIG_SYS_IMMR;

	/* inactive high */
	setbits_be32(&immr->gpio[0].dat, SPI_CS_MASK);
}
#endif /* CONFIG_MPC8XXX_SPI */

#ifdef CONFIG_FSL_ESDHC
int board_mmc_init(bd_t *bd)
{
	return fsl_esdhc_mmc_init(bd);
}
#endif

static u8 read_board_info(void)
{
	u8 val8;
	i2c_set_bus_num(0);

	if (i2c_read(CONFIG_SYS_I2C_PCF8574A_ADDR, 0, 0, &val8, 1) == 0)
		return val8;
	else
		return 0;
}

int checkboard(void)
{
	static const char * const rev_str[] = {
		"1.0",
		"<reserved>",
		"<reserved>",
		"<reserved>",
		"<unknown>",
	};
	u8 info;
	int i;

	info = read_board_info();
	i = (!info) ? 4 : info & 0x03;

	printf("Board: Freescale MPC8308RDB Rev %s\n", rev_str[i]);

	return 0;
}

static struct pci_region pcie_regions_0[] = {
	{
		.bus_start = CONFIG_SYS_PCIE1_MEM_BASE,
		.phys_start = CONFIG_SYS_PCIE1_MEM_PHYS,
		.size = CONFIG_SYS_PCIE1_MEM_SIZE,
		.flags = PCI_REGION_MEM,
	},
	{
		.bus_start = CONFIG_SYS_PCIE1_IO_BASE,
		.phys_start = CONFIG_SYS_PCIE1_IO_PHYS,
		.size = CONFIG_SYS_PCIE1_IO_SIZE,
		.flags = PCI_REGION_IO,
	},
};

void pci_init_board(void)
{
	immap_t *immr = (immap_t *)CONFIG_SYS_IMMR;
	sysconf83xx_t *sysconf = &immr->sysconf;
	law83xx_t *pcie_law = sysconf->pcielaw;
	struct pci_region *pcie_reg[] = { pcie_regions_0 };

	fsl_setup_serdes(CONFIG_FSL_SERDES1, FSL_SERDES_PROTO_PEX,
					FSL_SERDES_CLK_100, FSL_SERDES_VDD_1V);

	/* Deassert the resets in the control register */
	out_be32(&sysconf->pecr1, 0xE0008000);
	udelay(2000);

	/* Configure PCI Express Local Access Windows */
	out_be32(&pcie_law[0].bar, CONFIG_SYS_PCIE1_BASE & LAWBAR_BAR);
	out_be32(&pcie_law[0].ar, LBLAWAR_EN | LBLAWAR_512MB);

	mpc83xx_pcie_init(1, pcie_reg);
}
/*
 * Miscellaneous late-boot configurations
 *
 * If a VSC7385 microcode image is present, then upload it.
*/
int misc_init_r(void)
{
#ifdef CONFIG_MPC8XXX_SPI
	immap_t *immr = (immap_t *)CONFIG_SYS_IMMR;
	sysconf83xx_t *sysconf = &immr->sysconf;

	/*
	 * Set proper bits in SICRH to allow SPI on header J8
	 *
	 * NOTE: this breaks the TSEC2 interface, attached to the Vitesse
	 * switch. The pinmux configuration does not have a fine enough
	 * granularity to support both simultaneously.
	 */
	clrsetbits_be32(&sysconf->sicrh, SICRH_GPIO_A_TSEC2, SICRH_GPIO_A_GPIO);
	puts("WARNING: SPI enabled, TSEC2 support is broken\n");

	/* Set header J8 SPI chip select output, disabled */
	setbits_be32(&immr->gpio[0].dir, SPI_CS_MASK);
	setbits_be32(&immr->gpio[0].dat, SPI_CS_MASK);
#endif

#ifdef CONFIG_VSC7385_IMAGE
	if (vsc7385_upload_firmware((void *) CONFIG_VSC7385_IMAGE,
		CONFIG_VSC7385_IMAGE_SIZE)) {
		puts("Failure uploading VSC7385 microcode.\n");
		return 1;
	}
#endif

	return 0;
}
#if defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);
	fsl_fdt_fixup_dr_usb(blob, bd);
	fdt_fixup_esdhc(blob, bd);

	return 0;
}
#endif

int board_eth_init(bd_t *bis)
{
	int rv, num_if = 0;

	/* Initialize TSECs first */
	rv = cpu_eth_init(bis);
	if (rv >= 0)
		num_if += rv;
	else
		printf("ERROR: failed to initialize TSECs.\n");

	rv = pci_eth_init(bis);
	if (rv >= 0)
		num_if += rv;
	else
		printf("ERROR: failed to initialize PCI Ethernet.\n");

	return num_if;
}
