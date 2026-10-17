// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <asm/io.h>
#include <asm/spl.h>
#include <asm/arch-fsl-layerscape/fsl_serdes.h>
#include <asm/arch-fsl-layerscape/soc.h>
#include <spi_flash.h>

#include "sl28.h"

#define DCFG_RCWSR25 0x160
#define GPINFO_HW_VARIANT_MASK 0xff

#define SERDES_LNDGCR0		0x1ea08c0
#define   LNDGCR0_PROTS_MASK	GENMASK(11, 7)
#define   LNDGCR0_PROTS_SATA	(0x2 << 7)
#define SERDES_LNDGCR1		0x1ea08c4
#define   LNDGCR1_RDAT_INV	BIT(31)

/*
 * On this board the SMARC PCIe lane D might be switched to SATA mode. This
 * makes sense if this lane is connected to a Mini PCI slot and a mSATA card
 * is plugged in. In this case, the RX pair is swapped and we need to invert
 * the received data.
 */
static void fixup_sata_rx_polarity(void)
{
	u32 prot = in_le32(SERDES_LNDGCR0) & LNDGCR0_PROTS_MASK;
	u32 tmp;

	if (prot == LNDGCR0_PROTS_SATA) {
		tmp = in_le32(SERDES_LNDGCR1);
		tmp |= LNDGCR1_RDAT_INV;
		out_le32(SERDES_LNDGCR1, tmp);
	}
}

int sl28_variant(void)
{
	return in_le32(DCFG_BASE + DCFG_RCWSR25) & GPINFO_HW_VARIANT_MASK;
}

int board_fit_config_name_match(const char *name)
{
	int variant = sl28_variant();

	switch (variant) {
	case 1:
		return strcmp(name, "fsl-ls1028a-kontron-sl28-var1");
	case 2:
		return strcmp(name, "fsl-ls1028a-kontron-sl28-var2");
	case 3:
		return strcmp(name, "fsl-ls1028a-kontron-sl28-var3");
	case 4:
		return strcmp(name, "fsl-ls1028a-kontron-sl28-var4");
	default:
		return strcmp(name, "fsl-ls1028a-kontron-sl28");
	}
}

void board_boot_order(u32 *spl_boot_list)
{
	enum boot_source src = sl28_boot_source();

	switch (src) {
	case BOOT_SOURCE_SDHC:
		spl_boot_list[0] = BOOT_DEVICE_MMC2;
		break;
	case BOOT_SOURCE_SPI:
	case BOOT_SOURCE_I2C:
		spl_boot_list[0] = BOOT_DEVICE_SPI;
		break;
	case BOOT_SOURCE_MMC:
		spl_boot_list[0] = BOOT_DEVICE_MMC1;
		break;
	default:
		panic("unexpected bootsource (%d)\n", src);
		break;
	}
}

unsigned int spl_spi_get_uboot_offs(struct spi_flash *flash)
{
	enum boot_source src = sl28_boot_source();

	switch (src) {
	case BOOT_SOURCE_SPI:
		return 0x000000;
	case BOOT_SOURCE_I2C:
		return 0x230000;
	default:
		panic("unexpected bootsource (%d)\n", src);
		break;
	}
}

const char *spl_board_loader_name(u32 boot_device)
{
	enum boot_source src = sl28_boot_source();

	switch (src) {
	case BOOT_SOURCE_SDHC:
		return "SD card (Test mode)";
	case BOOT_SOURCE_SPI:
		return "Failsafe SPI flash";
	case BOOT_SOURCE_I2C:
		return "SPI flash";
	case BOOT_SOURCE_MMC:
		return "eMMC";
	default:
		return "(unknown)";
	}
}

int board_early_init_f(void)
{
	fixup_sata_rx_polarity();
	fsl_lsch3_early_init_f();

	return 0;
}
