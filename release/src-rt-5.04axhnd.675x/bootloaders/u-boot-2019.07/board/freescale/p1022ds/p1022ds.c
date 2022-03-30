// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2010-2012 Freescale Semiconductor, Inc.
 * Authors: Srikanth Srinivasan <srikanth.srinivasan@freescale.com>
 *          Timur Tabi <timur@freescale.com>
 */

#include <common.h>
#include <command.h>
#include <pci.h>
#include <asm/processor.h>
#include <asm/mmu.h>
#include <asm/cache.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_pci.h>
#include <fsl_ddr_sdram.h>
#include <asm/fsl_serdes.h>
#include <asm/io.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <fsl_mdio.h>
#include <tsec.h>
#include <asm/fsl_law.h>
#include <netdev.h>
#include <i2c.h>
#include <hwconfig.h>

#include "../common/ngpixis.h"

int board_early_init_f(void)
{
	ccsr_gur_t *gur = (void *)CONFIG_SYS_MPC85xx_GUTS_ADDR;

	/* Set pmuxcr to allow both i2c1 and i2c2 */
	setbits_be32(&gur->pmuxcr, 0x1000);
#ifdef CONFIG_SYS_RAMBOOT
	setbits_be32(&gur->pmuxcr,
		in_be32(&gur->pmuxcr) | MPC85xx_PMUXCR_SD_DATA);
#endif

	/* Read back the register to synchronize the write. */
	in_be32(&gur->pmuxcr);

	/* Set the pin muxing to enable ETSEC2. */
	clrbits_be32(&gur->pmuxcr2, 0x001F8000);

	/* Enable the SPI */
	clrsetbits_8(&pixis->brdcfg0, PIXIS_ELBC_SPI_MASK, PIXIS_SPI);

	return 0;
}

int checkboard(void)
{
	u8 sw;

	printf("Board: P1022DS Sys ID: 0x%02x, "
	       "Sys Ver: 0x%02x, FPGA Ver: 0x%02x, ",
		in_8(&pixis->id), in_8(&pixis->arch), in_8(&pixis->scver));

	sw = in_8(&PIXIS_SW(PIXIS_LBMAP_SWITCH));

	switch ((sw & PIXIS_LBMAP_MASK) >> 6) {
	case 0:
		printf ("vBank: %u\n", ((sw & 0x30) >> 4));
		break;
	case 1:
		printf ("NAND\n");
		break;
	case 2:
	case 3:
		puts ("Promjet\n");
		break;
	}

	return 0;
}

#define CONFIG_TFP410_I2C_ADDR	0x38

/* Masks for the SSI_TDM and AUDCLK bits of the ngPIXIS BRDCFG1 register. */
#define CONFIG_PIXIS_BRDCFG1_SSI_TDM_MASK	0x0c
#define CONFIG_PIXIS_BRDCFG1_AUDCLK_MASK	0x03

/* Route the I2C1 pins to the SSI port instead. */
#define CONFIG_PIXIS_BRDCFG1_SSI_TDM_SSI	0x08

/* Choose the 12.288Mhz codec reference clock */
#define CONFIG_PIXIS_BRDCFG1_AUDCLK_12		0x02

/* Choose the 11.2896Mhz codec reference clock */
#define CONFIG_PIXIS_BRDCFG1_AUDCLK_11		0x01

/* Connect to USB2 */
#define CONFIG_PIXIS_BRDCFG0_USB2		0x10
/* Connect to TFM bus */
#define CONFIG_PIXIS_BRDCFG1_TDM		0x0c
/* Connect to SPI */
#define CONFIG_PIXIS_BRDCFG0_SPI		0x80

int misc_init_r(void)
{
	u8 temp;
	const char *audclk;
	size_t arglen;
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);

	/* For DVI, enable the TFP410 Encoder. */

	temp = 0xBF;
	if (i2c_write(CONFIG_TFP410_I2C_ADDR, 0x08, 1, &temp, sizeof(temp)) < 0)
		return -1;
	if (i2c_read(CONFIG_TFP410_I2C_ADDR, 0x08, 1, &temp, sizeof(temp)) < 0)
		return -1;
	debug("DVI Encoder Read: 0x%02x\n", temp);

	temp = 0x10;
	if (i2c_write(CONFIG_TFP410_I2C_ADDR, 0x0A, 1, &temp, sizeof(temp)) < 0)
		return -1;
	if (i2c_read(CONFIG_TFP410_I2C_ADDR, 0x0A, 1, &temp, sizeof(temp)) < 0)
		return -1;
	debug("DVI Encoder Read: 0x%02x\n",temp);

	/* Enable the USB2 in PMUXCR2 and FGPA */
	if (hwconfig("usb2")) {
		clrsetbits_be32(&gur->pmuxcr2, MPC85xx_PMUXCR2_ETSECUSB_MASK,
			MPC85xx_PMUXCR2_USB);
		setbits_8(&pixis->brdcfg0, CONFIG_PIXIS_BRDCFG0_USB2);
	}

	/* tdm and audio can not enable simultaneous*/
	if (hwconfig("tdm") && hwconfig("audclk")){
		printf("WARNING: TDM and AUDIO can not be enabled simultaneous !\n");
		return -1;
	}

	/* Enable the TDM in PMUXCR and FGPA */
	if (hwconfig("tdm")) {
		clrsetbits_be32(&gur->pmuxcr, MPC85xx_PMUXCR_TDM_MASK,
			MPC85xx_PMUXCR_TDM);
		setbits_8(&pixis->brdcfg1, CONFIG_PIXIS_BRDCFG1_TDM);
		/* TDM need some configration option by SPI */
		clrsetbits_be32(&gur->pmuxcr, MPC85xx_PMUXCR_SPI_MASK,
			MPC85xx_PMUXCR_SPI);
		setbits_8(&pixis->brdcfg0, CONFIG_PIXIS_BRDCFG0_SPI);
	}

	/*
	 * Enable the reference clock for the WM8776 codec, and route the MUX
	 * pins for SSI. The default is the 12.288 MHz clock
	 */

	if (hwconfig("audclk")) {
		temp = in_8(&pixis->brdcfg1) & ~(CONFIG_PIXIS_BRDCFG1_SSI_TDM_MASK |
			CONFIG_PIXIS_BRDCFG1_AUDCLK_MASK);
		temp |= CONFIG_PIXIS_BRDCFG1_SSI_TDM_SSI;

		audclk = hwconfig_arg("audclk", &arglen);
		/* Check the first two chars only */
		if (audclk && (strncmp(audclk, "11", 2) == 0))
			temp |= CONFIG_PIXIS_BRDCFG1_AUDCLK_11;
		else
			temp |= CONFIG_PIXIS_BRDCFG1_AUDCLK_12;
		setbits_8(&pixis->brdcfg1, temp);
	}

	return 0;
}

/*
 * A list of PCI and SATA slots
 */
enum slot_id {
	SLOT_PCIE1 = 1,
	SLOT_PCIE2,
	SLOT_PCIE3,
	SLOT_PCIE4,
	SLOT_PCIE5,
	SLOT_SATA1,
	SLOT_SATA2
};

/*
 * This array maps the slot identifiers to their names on the P1022DS board.
 */
static const char *slot_names[] = {
	[SLOT_PCIE1] = "Slot 1",
	[SLOT_PCIE2] = "Slot 2",
	[SLOT_PCIE3] = "Slot 3",
	[SLOT_PCIE4] = "Slot 4",
	[SLOT_PCIE5] = "Mini-PCIe",
	[SLOT_SATA1] = "SATA 1",
	[SLOT_SATA2] = "SATA 2",
};

/*
 * This array maps a given SERDES configuration and SERDES device to the PCI or
 * SATA slot that it connects to.  This mapping is hard-coded in the FPGA.
 */
static u8 serdes_dev_slot[][SATA2 + 1] = {
	[0x01] = { [PCIE3] = SLOT_PCIE4, [PCIE2] = SLOT_PCIE5 },
	[0x02] = { [SATA1] = SLOT_SATA1, [SATA2] = SLOT_SATA2 },
	[0x09] = { [PCIE1] = SLOT_PCIE1, [PCIE3] = SLOT_PCIE4,
		   [PCIE2] = SLOT_PCIE5 },
	[0x16] = { [PCIE1] = SLOT_PCIE1, [PCIE3] = SLOT_PCIE2,
		   [PCIE2] = SLOT_PCIE3,
		   [SATA1] = SLOT_SATA1, [SATA2] = SLOT_SATA2 },
	[0x17] = { [PCIE1] = SLOT_PCIE1, [PCIE3] = SLOT_PCIE2,
		   [PCIE2] = SLOT_PCIE3 },
	[0x1a] = { [PCIE1] = SLOT_PCIE1, [PCIE2] = SLOT_PCIE3,
		   [PCIE2] = SLOT_PCIE3,
		   [SATA1] = SLOT_SATA1, [SATA2] = SLOT_SATA2 },
	[0x1c] = { [PCIE1] = SLOT_PCIE1,
		   [SATA1] = SLOT_SATA1, [SATA2] = SLOT_SATA2 },
	[0x1e] = { [PCIE1] = SLOT_PCIE1, [PCIE3] = SLOT_PCIE3 },
	[0x1f] = { [PCIE1] = SLOT_PCIE1 },
};


/*
 * Returns the name of the slot to which the PCIe or SATA controller is
 * connected
 */
const char *board_serdes_name(enum srds_prtcl device)
{
	ccsr_gur_t *gur = (void *)CONFIG_SYS_MPC85xx_GUTS_ADDR;
	u32 pordevsr = in_be32(&gur->pordevsr);
	unsigned int srds_cfg = (pordevsr & MPC85xx_PORDEVSR_IO_SEL) >>
				MPC85xx_PORDEVSR_IO_SEL_SHIFT;
	enum slot_id slot = serdes_dev_slot[srds_cfg][device];
	const char *name = slot_names[slot];

	if (name)
		return name;
	else
		return "Nothing";
}

#ifdef CONFIG_PCI
void pci_init_board(void)
{
	fsl_pcie_init_board(0);
}
#endif

int board_early_init_r(void)
{
	const unsigned int flashbase = CONFIG_SYS_FLASH_BASE;
	int flash_esel = find_tlb_idx((void *)flashbase, 1);

	/*
	 * Remap Boot flash + PROMJET region to caching-inhibited
	 * so that flash can be erased properly.
	 */

	/* Flush d-cache and invalidate i-cache of any FLASH data */
	flush_dcache();
	invalidate_icache();

	if (flash_esel == -1) {
		/* very unlikely unless something is messed up */
		puts("Error: Could not find TLB for FLASH BASE\n");
		flash_esel = 2;	/* give our best effort to continue */
	} else {
		/* invalidate existing TLB entry for flash + promjet */
		disable_tlb(flash_esel);
	}

	set_tlb(1, flashbase, CONFIG_SYS_FLASH_BASE_PHYS,
			MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
			0, flash_esel, BOOKE_PAGESZ_256M, 1);

	return 0;
}

/*
 * Initialize on-board and/or PCI Ethernet devices
 *
 * Returns:
 *      <0, error
 *       0, no ethernet devices found
 *      >0, number of ethernet devices initialized
 */
int board_eth_init(bd_t *bis)
{
	struct fsl_pq_mdio_info mdio_info;
	struct tsec_info_struct tsec_info[2];
	unsigned int num = 0;

#ifdef CONFIG_TSEC1
	SET_STD_TSEC_INFO(tsec_info[num], 1);
	num++;
#endif
#ifdef CONFIG_TSEC2
	SET_STD_TSEC_INFO(tsec_info[num], 2);
	num++;
#endif

	mdio_info.regs = (struct tsec_mii_mng *)CONFIG_SYS_MDIO_BASE_ADDR;
	mdio_info.name = DEFAULT_MII_NAME;
	fsl_pq_mdio_init(bis, &mdio_info);

	return tsec_eth_init(bis, tsec_info, num) + pci_eth_init(bis);
}

#ifdef CONFIG_OF_BOARD_SETUP
/**
 * ft_codec_setup - fix up the clock-frequency property of the codec node
 *
 * Update the clock-frequency property based on the value of the 'audclk'
 * hwconfig option.  If audclk is not specified, then don't write anything
 * to the device tree, because it means that the codec clock is disabled.
 */
static void ft_codec_setup(void *blob, const char *compatible)
{
	const char *audclk;
	size_t arglen;
	u32 freq;

	audclk = hwconfig_arg("audclk", &arglen);
	if (audclk) {
		if (strncmp(audclk, "11", 2) == 0)
			freq = 11289600;
		else
			freq = 12288000;

		do_fixup_by_compat_u32(blob, compatible, "clock-frequency",
				       freq, 1);
	}
}

int ft_board_setup(void *blob, bd_t *bd)
{
	phys_addr_t base;
	phys_size_t size;

	ft_cpu_setup(blob, bd);

	base = env_get_bootm_low();
	size = env_get_bootm_size();

	fdt_fixup_memory(blob, (u64)base, (u64)size);

#ifdef CONFIG_HAS_FSL_DR_USB
	fsl_fdt_fixup_dr_usb(blob, bd);
#endif

	FT_FSL_PCI_SETUP;

#ifdef CONFIG_FSL_SGMII_RISER
	fsl_sgmii_riser_fdt_fixup(blob);
#endif

	/* Update the WM8776 node's clock frequency property */
	ft_codec_setup(blob, "wlf,wm8776");

	return 0;
}
#endif
