// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2006, 2007, 2010-2011 Freescale Semiconductor.
 */

#include <common.h>
#include <pci.h>
#include <asm/processor.h>
#include <asm/immap_86xx.h>
#include <asm/fsl_pci.h>
#include <fsl_ddr_sdram.h>
#include <asm/fsl_serdes.h>
#include <asm/io.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <netdev.h>

DECLARE_GLOBAL_DATA_PTR;

phys_size_t fixed_sdram(void);

int checkboard(void)
{
	u8 vboot;
	u8 *pixis_base = (u8 *)PIXIS_BASE;

	printf ("Board: MPC8641HPCN, Sys ID: 0x%02x, "
		"Sys Ver: 0x%02x, FPGA Ver: 0x%02x, ",
		in_8(pixis_base + PIXIS_ID), in_8(pixis_base + PIXIS_VER),
		in_8(pixis_base + PIXIS_PVER));

	vboot = in_8(pixis_base + PIXIS_VBOOT);
	if (vboot & PIXIS_VBOOT_FMAP)
		printf ("vBank: %d\n", ((vboot & PIXIS_VBOOT_FBANK) >> 6));
	else
		puts ("Promjet\n");

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

	debug("    DDR: ");
	gd->ram_size = dram_size;

	return 0;
}


#if !defined(CONFIG_SPD_EEPROM)
/*
 * Fixed sdram init -- doesn't use serial presence detect.
 */
phys_size_t
fixed_sdram(void)
{
#if !defined(CONFIG_SYS_RAMBOOT)
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	struct ccsr_ddr __iomem *ddr = &immap->im_ddr1;

	ddr->cs0_bnds = CONFIG_SYS_DDR_CS0_BNDS;
	ddr->cs0_config = CONFIG_SYS_DDR_CS0_CONFIG;
	ddr->timing_cfg_3 = CONFIG_SYS_DDR_TIMING_3;
	ddr->timing_cfg_0 = CONFIG_SYS_DDR_TIMING_0;
	ddr->timing_cfg_1 = CONFIG_SYS_DDR_TIMING_1;
	ddr->timing_cfg_2 = CONFIG_SYS_DDR_TIMING_2;
	ddr->sdram_mode = CONFIG_SYS_DDR_MODE_1;
	ddr->sdram_mode_2 = CONFIG_SYS_DDR_MODE_2;
	ddr->sdram_interval = CONFIG_SYS_DDR_INTERVAL;
	ddr->sdram_data_init = CONFIG_SYS_DDR_DATA_INIT;
	ddr->sdram_clk_cntl = CONFIG_SYS_DDR_CLK_CTRL;
	ddr->sdram_ocd_cntl = CONFIG_SYS_DDR_OCD_CTRL;
	ddr->sdram_ocd_status = CONFIG_SYS_DDR_OCD_STATUS;

#if defined (CONFIG_DDR_ECC)
	ddr->err_disable = 0x0000008D;
	ddr->err_sbe = 0x00ff0000;
#endif
	asm("sync;isync");

	udelay(500);

#if defined (CONFIG_DDR_ECC)
	/* Enable ECC checking */
	ddr->sdram_cfg = (CONFIG_SYS_DDR_CONTROL | 0x20000000);
#else
	ddr->sdram_cfg = CONFIG_SYS_DDR_CONTROL;
	ddr->sdram_cfg_2 = CONFIG_SYS_DDR_CONTROL2;
#endif
	asm("sync; isync");

	udelay(500);
#endif
	return CONFIG_SYS_SDRAM_SIZE * 1024 * 1024;
}
#endif	/* !defined(CONFIG_SPD_EEPROM) */

void pci_init_board(void)
{
	fsl_pcie_init_board(0);

#ifdef CONFIG_PCIE1
		/*
		 * Activate ULI1575 legacy chip by performing a fake
		 * memory access.  Needed to make ULI RTC work.
		 */
		in_be32((unsigned *) ((char *)(CONFIG_SYS_PCIE1_MEM_VIRT
				       + CONFIG_SYS_PCIE1_MEM_SIZE - 0x1000000)));
#endif /* CONFIG_PCIE1 */
}


#if defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
	int off;
	u64 *tmp;
	int addrcells;

	ft_cpu_setup(blob, bd);

	FT_FSL_PCI_SETUP;

	/*
	 * Warn if it looks like the device tree doesn't match u-boot.
	 * This is just an estimation, based on the location of CCSR,
	 * which is defined by the "reg" property in the soc node.
	 */
	off = fdt_path_offset(blob, "/soc8641");
	addrcells = fdt_address_cells(blob, 0);
	tmp = (u64 *)fdt_getprop(blob, off, "reg", NULL);

	if (tmp) {
		u64 addr;

		if (addrcells == 1)
			addr = *(u32 *)tmp;
		else
			addr = *tmp;

		if (addr != CONFIG_SYS_CCSRBAR_PHYS)
			printf("WARNING: The CCSRBAR address in your .dts "
			       "does not match the address of the CCSR "
			       "in u-boot.  This means your .dts might "
			       "be old.\n");
	}

	return 0;
}
#endif


/*
 * get_board_sys_clk
 *      Reads the FPGA on board for CONFIG_SYS_CLK_FREQ
 */

unsigned long
get_board_sys_clk(ulong dummy)
{
	u8 i, go_bit, rd_clks;
	ulong val = 0;
	u8 *pixis_base = (u8 *)PIXIS_BASE;

	go_bit = in_8(pixis_base + PIXIS_VCTL);
	go_bit &= 0x01;

	rd_clks = in_8(pixis_base + PIXIS_VCFGEN0);
	rd_clks &= 0x1C;

	/*
	 * Only if both go bit and the SCLK bit in VCFGEN0 are set
	 * should we be using the AUX register. Remember, we also set the
	 * GO bit to boot from the alternate bank on the on-board flash
	 */

	if (go_bit) {
		if (rd_clks == 0x1c)
			i = in_8(pixis_base + PIXIS_AUX);
		else
			i = in_8(pixis_base + PIXIS_SPD);
	} else {
		i = in_8(pixis_base + PIXIS_SPD);
	}

	i &= 0x07;

	switch (i) {
	case 0:
		val = 33000000;
		break;
	case 1:
		val = 40000000;
		break;
	case 2:
		val = 50000000;
		break;
	case 3:
		val = 66000000;
		break;
	case 4:
		val = 83000000;
		break;
	case 5:
		val = 100000000;
		break;
	case 6:
		val = 134000000;
		break;
	case 7:
		val = 166000000;
		break;
	}

	return val;
}

int board_eth_init(bd_t *bis)
{
	/* Initialize TSECs */
	cpu_eth_init(bis);
	return pci_eth_init(bis);
}

void board_reset(void)
{
	u8 *pixis_base = (u8 *)PIXIS_BASE;

	out_8(pixis_base + PIXIS_RST, 0);

	while (1)
		;
}
