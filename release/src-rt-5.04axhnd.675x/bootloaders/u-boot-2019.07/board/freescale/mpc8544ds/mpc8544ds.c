// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2007,2009-2010 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <command.h>
#include <pci.h>
#include <asm/processor.h>
#include <asm/mmu.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_pci.h>
#include <fsl_ddr_sdram.h>
#include <asm/fsl_serdes.h>
#include <asm/io.h>
#include <miiphy.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <fsl_mdio.h>
#include <tsec.h>
#include <netdev.h>

#include "../common/sgmii_riser.h"

int checkboard (void)
{
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	volatile fsl_lbc_t *lbc = LBC_BASE_ADDR;
	volatile ccsr_local_ecm_t *ecm = (void *)(CONFIG_SYS_MPC85xx_ECM_ADDR);
	u8 vboot;
	u8 *pixis_base = (u8 *)PIXIS_BASE;

	if ((uint)&gur->porpllsr != 0xe00e0000) {
		printf("immap size error %lx\n",(ulong)&gur->porpllsr);
	}
	printf ("Board: MPC8544DS, Sys ID: 0x%02x, "
		"Sys Ver: 0x%02x, FPGA Ver: 0x%02x, ",
		in_8(pixis_base + PIXIS_ID), in_8(pixis_base + PIXIS_VER),
		in_8(pixis_base + PIXIS_PVER));

	vboot = in_8(pixis_base + PIXIS_VBOOT);
	if (vboot & PIXIS_VBOOT_FMAP)
		printf ("vBank: %d\n", ((vboot & PIXIS_VBOOT_FBANK) >> 6));
	else
		puts ("Promjet\n");

	lbc->ltesr = 0xffffffff;	/* Clear LBC error interrupts */
	lbc->lteir = 0xffffffff;	/* Enable LBC error interrupts */
	ecm->eedr = 0xffffffff;		/* Clear ecm errors */
	ecm->eeer = 0xffffffff;		/* Enable ecm errors */

	return 0;
}

#ifdef CONFIG_PCI1
static struct pci_controller pci1_hose;
#endif

#ifdef CONFIG_PCIE3
static struct pci_controller pcie3_hose;
#endif

void pci_init_board(void)
{
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	struct fsl_pci_info pci_info;
	u32 devdisr, pordevsr, io_sel;
	u32 porpllsr, pci_agent, pci_speed, pci_32, pci_arb, pci_clk_sel;
	int first_free_busno = 0;

	int pcie_ep, pcie_configured;

	devdisr = in_be32(&gur->devdisr);
	pordevsr = in_be32(&gur->pordevsr);
	porpllsr = in_be32(&gur->porpllsr);
	io_sel = (pordevsr & MPC85xx_PORDEVSR_IO_SEL) >> 19;

	debug ("   pci_init_board: devdisr=%x, io_sel=%x\n", devdisr, io_sel);

	puts("\n");

#ifdef CONFIG_PCIE3
	pcie_configured = is_serdes_configured(PCIE3);

	if (pcie_configured && !(devdisr & MPC85xx_DEVDISR_PCIE3)){
		/* contains both PCIE3 MEM & IO space */
		set_next_law(CONFIG_SYS_PCIE3_MEM_PHYS, LAW_SIZE_4M,
				LAW_TRGT_IF_PCIE_3);
		SET_STD_PCIE_INFO(pci_info, 3);
		pcie_ep = fsl_setup_hose(&pcie3_hose, pci_info.regs);

		/* outbound memory */
		pci_set_region(&pcie3_hose.regions[0],
			       CONFIG_SYS_PCIE3_MEM_BUS2,
			       CONFIG_SYS_PCIE3_MEM_PHYS2,
			       CONFIG_SYS_PCIE3_MEM_SIZE2,
			       PCI_REGION_MEM);

		pcie3_hose.region_count = 1;

		printf("PCIE3: connected to ULI as %s (base addr %lx)\n",
			pcie_ep ? "Endpoint" : "Root Complex",
			pci_info.regs);
		first_free_busno = fsl_pci_init_port(&pci_info,
					&pcie3_hose, first_free_busno);

		/*
		 * Activate ULI1575 legacy chip by performing a fake
		 * memory access.  Needed to make ULI RTC work.
		 */
		in_be32((u32 *)CONFIG_SYS_PCIE3_MEM_BUS);
	} else {
		printf("PCIE3: disabled\n");
	}
	puts("\n");
#else
	setbits_be32(&gur->devdisr, MPC85xx_DEVDISR_PCIE3); /* disable */
#endif

#ifdef CONFIG_PCIE1
	SET_STD_PCIE_INFO(pci_info, 1);
	first_free_busno = fsl_pcie_init_ctrl(first_free_busno, devdisr, PCIE1, &pci_info);
#else
	setbits_be32(&gur->devdisr, _DEVDISR_PCIE1); /* disable */
#endif

#ifdef CONFIG_PCIE2
	SET_STD_PCIE_INFO(pci_info, 2);
	first_free_busno = fsl_pcie_init_ctrl(first_free_busno, devdisr, PCIE2, &pci_info);
#else
	setbits_be32(&gur->devdisr, _DEVDISR_PCIE2); /* disable */
#endif

#ifdef CONFIG_PCI1
	pci_speed = 66666000;
	pci_32 = 1;
	pci_arb = pordevsr & MPC85xx_PORDEVSR_PCI1_ARB;
	pci_clk_sel = porpllsr & MPC85xx_PORDEVSR_PCI1_SPD;

	if (!(devdisr & MPC85xx_DEVDISR_PCI1)) {
		SET_STD_PCI_INFO(pci_info, 1);
		set_next_law(pci_info.mem_phys,
			law_size_bits(pci_info.mem_size), pci_info.law);
		set_next_law(pci_info.io_phys,
			law_size_bits(pci_info.io_size), pci_info.law);

		pci_agent = fsl_setup_hose(&pci1_hose, pci_info.regs);
		printf("PCI: %d bit, %s MHz, %s, %s, %s (base address %lx)\n",
			(pci_32) ? 32 : 64,
			(pci_speed == 33333000) ? "33" :
			(pci_speed == 66666000) ? "66" : "unknown",
			pci_clk_sel ? "sync" : "async",
			pci_agent ? "agent" : "host",
			pci_arb ? "arbiter" : "external-arbiter",
			pci_info.regs);

		first_free_busno = fsl_pci_init_port(&pci_info,
					&pci1_hose, first_free_busno);
	} else {
		printf("PCI: disabled\n");
	}

	puts("\n");
#else
	setbits_be32(&gur->devdisr, MPC85xx_DEVDISR_PCI1); /* disable */
#endif
}

int last_stage_init(void)
{
	return 0;
}


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
		val = 33333333;
		break;
	case 1:
		val = 40000000;
		break;
	case 2:
		val = 50000000;
		break;
	case 3:
		val = 66666666;
		break;
	case 4:
		val = 83000000;
		break;
	case 5:
		val = 100000000;
		break;
	case 6:
		val = 133333333;
		break;
	case 7:
		val = 166666666;
		break;
	}

	return val;
}


#define MIIM_CIS8204_SLED_CON		0x1b
#define MIIM_CIS8204_SLEDCON_INIT	0x1115
/*
 * Hack to write all 4 PHYs with the LED values
 */
int board_phy_config(struct phy_device *phydev)
{
	static int do_once;
	uint phyid;
	struct mii_dev *bus = phydev->bus;

	if (phydev->drv->config)
		phydev->drv->config(phydev);
	if (do_once)
		return 0;

	for (phyid = 0; phyid < 4; phyid++)
		bus->write(bus, phyid, MDIO_DEVAD_NONE, MIIM_CIS8204_SLED_CON,
				MIIM_CIS8204_SLEDCON_INIT);

	do_once = 1;

	return 0;
}


int board_eth_init(bd_t *bis)
{
#ifdef CONFIG_TSEC_ENET
	struct fsl_pq_mdio_info mdio_info;
	struct tsec_info_struct tsec_info[2];
	int num = 0;

#ifdef CONFIG_TSEC1
	SET_STD_TSEC_INFO(tsec_info[num], 1);
	if (is_serdes_configured(SGMII_TSEC1)) {
		puts("eTSEC1 is in sgmii mode.\n");
		tsec_info[num].flags |= TSEC_SGMII;
	}
	num++;
#endif
#ifdef CONFIG_TSEC3
	SET_STD_TSEC_INFO(tsec_info[num], 3);
	if (is_serdes_configured(SGMII_TSEC3)) {
		puts("eTSEC3 is in sgmii mode.\n");
		tsec_info[num].flags |= TSEC_SGMII;
	}
	num++;
#endif

	if (!num) {
		printf("No TSECs initialized\n");

		return 0;
	}

	if (is_serdes_configured(SGMII_TSEC1) ||
	    is_serdes_configured(SGMII_TSEC3)) {
		fsl_sgmii_riser_init(tsec_info, num);
	}

	mdio_info.regs = (struct tsec_mii_mng *)CONFIG_SYS_MDIO_BASE_ADDR;
	mdio_info.name = DEFAULT_MII_NAME;
	fsl_pq_mdio_init(bis, &mdio_info);

	tsec_eth_init(bis, tsec_info, num);
#endif
	return pci_eth_init(bis);
}

#if defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);

	FT_FSL_PCI_SETUP;

#ifdef CONFIG_FSL_SGMII_RISER
	fsl_sgmii_riser_fdt_fixup(blob);
#endif

	return 0;
}
#endif
