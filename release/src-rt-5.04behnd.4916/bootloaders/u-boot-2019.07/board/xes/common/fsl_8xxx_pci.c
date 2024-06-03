// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2008 Extreme Engineering Solutions, Inc.
 * Copyright 2007-2008 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <pci.h>
#include <asm/fsl_pci.h>
#include <asm/fsl_serdes.h>
#include <asm/io.h>
#include <linux/compiler.h>
#include <linux/libfdt.h>
#include <fdt_support.h>


#ifdef CONFIG_PCI1
static struct pci_controller pci1_hose;
#endif

void pci_init_board(void)
{
	int first_free_busno = 0;

#ifdef CONFIG_PCI1
	int pcie_ep;
	struct fsl_pci_info pci_info;
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u32 devdisr = in_be32(&gur->devdisr);
	uint pci_spd_norm = in_be32(&gur->pordevsr) & MPC85xx_PORDEVSR_PCI1_SPD;
	uint pci_32 = in_be32(&gur->pordevsr) & MPC85xx_PORDEVSR_PCI1_PCI32;
	uint pci_arb = in_be32(&gur->pordevsr) & MPC85xx_PORDEVSR_PCI1_ARB;
	uint pcix = in_be32(&gur->pordevsr) & MPC85xx_PORDEVSR_PCI1;
	uint freq = CONFIG_SYS_CLK_FREQ / 1000 / 1000;

	if (!(devdisr & MPC85xx_DEVDISR_PCI1)) {
		SET_STD_PCI_INFO(pci_info, 1);
		set_next_law(pci_info.mem_phys,
			law_size_bits(pci_info.mem_size), pci_info.law);
		set_next_law(pci_info.io_phys,
			law_size_bits(pci_info.io_size), pci_info.law);

		pcie_ep = fsl_setup_hose(&pci1_hose, pci_info.regs);
		printf("PCI1: %d bit %s, %s %d MHz, %s, %s\n",
			pci_32 ? 32 : 64,
			pcix ? "PCIX" : "PCI",
			pci_spd_norm ? ">=" : "<=",
			pcix ? freq * 2 : freq,
			pcie_ep ? "agent" : "host",
			pci_arb ? "arbiter" : "external-arbiter");

		first_free_busno = fsl_pci_init_port(&pci_info,
					&pci1_hose, first_free_busno);
	} else {
		printf("PCI1: disabled\n");
	}
#elif defined CONFIG_ARCH_MPC8548
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	/* PCI1 not present on MPC8572 */
	setbits_be32(&gur->devdisr, MPC85xx_DEVDISR_PCI1);
#endif

	fsl_pcie_init_board(first_free_busno);
}

#if defined(CONFIG_OF_BOARD_SETUP)
void ft_board_pci_setup(void *blob, bd_t *bd)
{
	FT_FSL_PCI_SETUP;
}
#endif /* CONFIG_OF_BOARD_SETUP */
