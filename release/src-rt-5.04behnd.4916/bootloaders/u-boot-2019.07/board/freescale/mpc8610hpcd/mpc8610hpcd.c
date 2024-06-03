// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2007,2009-2011 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <command.h>
#include <pci.h>
#include <asm/processor.h>
#include <asm/immap_86xx.h>
#include <asm/fsl_pci.h>
#include <fsl_ddr_sdram.h>
#include <asm/fsl_serdes.h>
#include <i2c.h>
#include <asm/io.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <spd_sdram.h>
#include <netdev.h>

DECLARE_GLOBAL_DATA_PTR;

void sdram_init(void);
phys_size_t fixed_sdram(void);
int mpc8610hpcd_diu_init(void);


/* called before any console output */
int board_early_init_f(void)
{
	volatile immap_t *immap = (immap_t *)CONFIG_SYS_IMMR;
	volatile ccsr_gur_t *gur = &immap->im_gur;

	gur->gpiocr |= 0x88aa5500; /* DIU16, IR1, UART0, UART2 */

	return 0;
}

int misc_init_r(void)
{
	u8 tmp_val, version;
	u8 *pixis_base = (u8 *)PIXIS_BASE;

	/*Do not use 8259PIC*/
	tmp_val = in_8(pixis_base + PIXIS_BRDCFG0);
	out_8(pixis_base + PIXIS_BRDCFG0, tmp_val | 0x80);

	/*For FPGA V7 or higher, set the IRQMAPSEL to 0 to use MAP0 interrupt*/
	version = in_8(pixis_base + PIXIS_PVER);
	if(version >= 0x07) {
		tmp_val = in_8(pixis_base + PIXIS_BRDCFG0);
		out_8(pixis_base + PIXIS_BRDCFG0, tmp_val & 0xbf);
	}

	/* Using this for DIU init before the driver in linux takes over
	 *  Enable the TFP410 Encoder (I2C address 0x38)
	 */

	tmp_val = 0xBF;
	i2c_write(0x38, 0x08, 1, &tmp_val, sizeof(tmp_val));
	/* Verify if enabled */
	tmp_val = 0;
	i2c_read(0x38, 0x08, 1, &tmp_val, sizeof(tmp_val));
	debug("DVI Encoder Read: 0x%02x\n", tmp_val);

	tmp_val = 0x10;
	i2c_write(0x38, 0x0A, 1, &tmp_val, sizeof(tmp_val));
	/* Verify if enabled */
	tmp_val = 0;
	i2c_read(0x38, 0x0A, 1, &tmp_val, sizeof(tmp_val));
	debug("DVI Encoder Read: 0x%02x\n", tmp_val);

	return 0;
}

int checkboard(void)
{
	volatile immap_t *immap = (immap_t *)CONFIG_SYS_IMMR;
	volatile ccsr_local_mcm_t *mcm = &immap->im_local_mcm;
	u8 *pixis_base = (u8 *)PIXIS_BASE;

	printf ("Board: MPC8610HPCD, Sys ID: 0x%02x, "
		"Sys Ver: 0x%02x, FPGA Ver: 0x%02x, ",
		in_8(pixis_base + PIXIS_ID), in_8(pixis_base + PIXIS_VER),
		in_8(pixis_base + PIXIS_PVER));

	/*
	 * The MPC8610 HPCD workbook says that LBMAP=11 is the "normal" boot
	 * bank and LBMAP=00 is the alternate bank.  However, the pixis
	 * altbank code can only set bits, not clear them, so we treat 00 as
	 * the normal bank and 11 as the alternate.
	 */
	switch (in_8(pixis_base + PIXIS_VBOOT) & 0xC0) {
	case 0:
		puts("vBank: Standard\n");
		break;
	case 0x40:
		puts("Promjet\n");
		break;
	case 0x80:
		puts("NAND\n");
		break;
	case 0xC0:
		puts("vBank: Alternate\n");
		break;
	}

	mcm->abcr |= 0x00010000; /* 0 */
	mcm->hpmr3 = 0x80000008; /* 4c */
	mcm->hpmr0 = 0;
	mcm->hpmr1 = 0;
	mcm->hpmr2 = 0;
	mcm->hpmr4 = 0;
	mcm->hpmr5 = 0;

	return 0;
}


int dram_init(void)
{
	phys_size_t dram_size = 0;

#if defined(CONFIG_SPD_EEPROM)
	dram_size = fsl_ddr_sdram();
#else
	dram_size = fixed_sdram();
#endif

	setup_ddr_bat(dram_size);

	debug(" DDR: ");
	gd->ram_size = dram_size;

	return 0;
}


#if !defined(CONFIG_SPD_EEPROM)
/*
 * Fixed sdram init -- doesn't use serial presence detect.
 */

phys_size_t fixed_sdram(void)
{
#if !defined(CONFIG_SYS_RAMBOOT)
	volatile immap_t *immap = (immap_t *)CONFIG_SYS_IMMR;
	struct ccsr_ddr __iomem *ddr = &immap->im_ddr1;
	uint d_init;

	ddr->cs0_bnds = 0x0000001f;
	ddr->cs0_config = 0x80010202;

	ddr->timing_cfg_3 = 0x00000000;
	ddr->timing_cfg_0 = 0x00260802;
	ddr->timing_cfg_1 = 0x3935d322;
	ddr->timing_cfg_2 = 0x14904cc8;
	ddr->sdram_mode = 0x00480432;
	ddr->sdram_mode_2 = 0x00000000;
	ddr->sdram_interval = 0x06180fff; /* 0x06180100; */
	ddr->sdram_data_init = 0xDEADBEEF;
	ddr->sdram_clk_cntl = 0x03800000;
	ddr->sdram_cfg_2 = 0x04400010;

#if defined(CONFIG_DDR_ECC)
	ddr->err_int_en = 0x0000000d;
	ddr->err_disable = 0x00000000;
	ddr->err_sbe = 0x00010000;
#endif
	asm("sync;isync");

	udelay(500);

	ddr->sdram_cfg = 0xc3000000; /* 0xe3008000;*/


#if defined(CONFIG_ECC_INIT_VIA_DDRCONTROLLER)
	d_init = 1;
	debug("DDR - 1st controller: memory initializing\n");
	/*
	 * Poll until memory is initialized.
	 * 512 Meg at 400 might hit this 200 times or so.
	 */
	while ((ddr->sdram_cfg_2 & (d_init << 4)) != 0)
		udelay(1000);

	debug("DDR: memory initialized\n\n");
	asm("sync; isync");
	udelay(500);
#endif

	return 512 * 1024 * 1024;
#endif
	return CONFIG_SYS_SDRAM_SIZE * 1024 * 1024;
}

#endif

#if defined(CONFIG_PCI)
/*
 * Initialize PCI Devices, report devices found.
 */

#ifndef CONFIG_PCI_PNP
static struct pci_config_table pci_fsl86xxads_config_table[] = {
	{PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID,
	 PCI_IDSEL_NUMBER, PCI_ANY_ID,
	 pci_cfgfunc_config_device, {PCI_ENET0_IOADDR,
				 PCI_ENET0_MEMADDR,
				 PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER} },
	{}
};
#endif


static struct pci_controller pci1_hose;
#endif /* CONFIG_PCI */

void pci_init_board(void)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_CCSRBAR;
	volatile ccsr_gur_t *gur = &immap->im_gur;
	struct fsl_pci_info pci_info;
	u32 devdisr;
	int first_free_busno;
	int pci_agent;

	devdisr = in_be32(&gur->devdisr);

	first_free_busno = fsl_pcie_init_board(0);

#ifdef CONFIG_PCI1
	if (!(devdisr & MPC86xx_DEVDISR_PCI1)) {
		SET_STD_PCI_INFO(pci_info, 1);
		set_next_law(pci_info.mem_phys,
			law_size_bits(pci_info.mem_size), pci_info.law);
		set_next_law(pci_info.io_phys,
			law_size_bits(pci_info.io_size), pci_info.law);

		pci_agent = fsl_setup_hose(&pci1_hose, pci_info.regs);
		printf("PCI: connected to PCI slots as %s" \
			" (base address %lx)\n",
			pci_agent ? "Agent" : "Host",
			pci_info.regs);
#ifndef CONFIG_PCI_PNP
		pci1_hose.config_table = pci_mpc86xxcts_config_table;
#endif
		first_free_busno = fsl_pci_init_port(&pci_info,
					&pci1_hose, first_free_busno);
	} else {
		printf("PCI: disabled\n");
	}

	puts("\n");
#else
	setbits_be32(&gur->devdisr, MPC86xx_DEVDISR_PCI1); /* disable */
#endif

	fsl_pcie_init_board(first_free_busno);
}

#if defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);

	FT_FSL_PCI_SETUP;

	return 0;
}
#endif

/*
 * get_board_sys_clk
 * Reads the FPGA on board for CONFIG_SYS_CLK_FREQ
 */

unsigned long
get_board_sys_clk(ulong dummy)
{
	u8 i;
	ulong val = 0;
	u8 *pixis_base = (u8 *)PIXIS_BASE;

	i = in_8(pixis_base + PIXIS_SPD);
	i &= 0x07;

	switch (i) {
	case 0:
		val = 33333000;
		break;
	case 1:
		val = 39999600;
		break;
	case 2:
		val = 49999500;
		break;
	case 3:
		val = 66666000;
		break;
	case 4:
		val = 83332500;
		break;
	case 5:
		val = 99999000;
		break;
	case 6:
		val = 133332000;
		break;
	case 7:
		val = 166665000;
		break;
	}

	return val;
}

int board_eth_init(bd_t *bis)
{
	return pci_eth_init(bis);
}

void board_reset(void)
{
	u8 *pixis_base = (u8 *)PIXIS_BASE;

	out_8(pixis_base + PIXIS_RST, 0);

	while (1)
		;
}
