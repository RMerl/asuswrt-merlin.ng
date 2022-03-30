// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2004 Freescale Semiconductor.
 * Copyright (C) 2003 Motorola Inc.
 * Xianghua Xiao (x.xiao@motorola.com)
 */

/*
 * PCI Configuration space access support for MPC85xx PCI Bridge
 */
#include <common.h>
#include <asm/cpm_85xx.h>
#include <pci.h>

#if !defined(CONFIG_FSL_PCI_INIT) && !defined(CONFIG_DM_PCI)

#ifndef CONFIG_SYS_PCI1_MEM_BUS
#define CONFIG_SYS_PCI1_MEM_BUS CONFIG_SYS_PCI1_MEM_BASE
#endif

#ifndef CONFIG_SYS_PCI1_IO_BUS
#define CONFIG_SYS_PCI1_IO_BUS CONFIG_SYS_PCI1_IO_BASE
#endif

#ifndef CONFIG_SYS_PCI2_MEM_BUS
#define CONFIG_SYS_PCI2_MEM_BUS CONFIG_SYS_PCI2_MEM_BASE
#endif

#ifndef CONFIG_SYS_PCI2_IO_BUS
#define CONFIG_SYS_PCI2_IO_BUS CONFIG_SYS_PCI2_IO_BASE
#endif

static struct pci_controller *pci_hose;

void
pci_mpc85xx_init(struct pci_controller *board_hose)
{
	u16 reg16;
	u32 dev;

	volatile ccsr_pcix_t *pcix = (void *)(CONFIG_SYS_MPC85xx_PCIX_ADDR);
#ifdef CONFIG_MPC85XX_PCI2
	volatile ccsr_pcix_t *pcix2 = (void *)(CONFIG_SYS_MPC85xx_PCIX2_ADDR);
#endif
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	struct pci_controller * hose;

	pci_hose = board_hose;

	hose = &pci_hose[0];

	hose->first_busno = 0;
	hose->last_busno = 0xff;

	pci_setup_indirect(hose,
			   (CONFIG_SYS_IMMR+0x8000),
			   (CONFIG_SYS_IMMR+0x8004));

	/*
	 * Hose scan.
	 */
	dev = PCI_BDF(hose->first_busno, 0, 0);
	pci_hose_read_config_word (hose, dev, PCI_COMMAND, &reg16);
	reg16 |= PCI_COMMAND_SERR | PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY;
	pci_hose_write_config_word(hose, dev, PCI_COMMAND, reg16);

	/*
	 * Clear non-reserved bits in status register.
	 */
	pci_hose_write_config_word(hose, dev, PCI_STATUS, 0xffff);

	if (!(gur->pordevsr & MPC85xx_PORDEVSR_PCI1)) {
		/* PCI-X init */
		if (CONFIG_SYS_CLK_FREQ < 66000000)
			printf("PCI-X will only work at 66 MHz\n");

		reg16 = PCI_X_CMD_MAX_SPLIT | PCI_X_CMD_MAX_READ
			| PCI_X_CMD_ERO | PCI_X_CMD_DPERR_E;
		pci_hose_write_config_word(hose, dev, PCIX_COMMAND, reg16);
	}

	pcix->potar1   = (CONFIG_SYS_PCI1_MEM_BUS >> 12) & 0x000fffff;
	pcix->potear1  = 0x00000000;
	pcix->powbar1  = (CONFIG_SYS_PCI1_MEM_PHYS >> 12) & 0x000fffff;
	pcix->powbear1 = 0x00000000;
	pcix->powar1 = (POWAR_EN | POWAR_MEM_READ |
			POWAR_MEM_WRITE | (__ilog2(CONFIG_SYS_PCI1_MEM_SIZE) - 1));

	pcix->potar2  = (CONFIG_SYS_PCI1_IO_BUS >> 12) & 0x000fffff;
	pcix->potear2  = 0x00000000;
	pcix->powbar2  = (CONFIG_SYS_PCI1_IO_PHYS >> 12) & 0x000fffff;
	pcix->powbear2 = 0x00000000;
	pcix->powar2 = (POWAR_EN | POWAR_IO_READ |
			POWAR_IO_WRITE | (__ilog2(CONFIG_SYS_PCI1_IO_SIZE) - 1));

	pcix->pitar1 = 0x00000000;
	pcix->piwbar1 = 0x00000000;
	pcix->piwar1 = (PIWAR_EN | PIWAR_PF | PIWAR_LOCAL |
			PIWAR_READ_SNOOP | PIWAR_WRITE_SNOOP | PIWAR_MEM_2G);

	pcix->powar3 = 0;
	pcix->powar4 = 0;
	pcix->piwar2 = 0;
	pcix->piwar3 = 0;

	pci_set_region(hose->regions + 0,
		       CONFIG_SYS_PCI1_MEM_BUS,
		       CONFIG_SYS_PCI1_MEM_PHYS,
		       CONFIG_SYS_PCI1_MEM_SIZE,
		       PCI_REGION_MEM);

	pci_set_region(hose->regions + 1,
		       CONFIG_SYS_PCI1_IO_BUS,
		       CONFIG_SYS_PCI1_IO_PHYS,
		       CONFIG_SYS_PCI1_IO_SIZE,
		       PCI_REGION_IO);

	hose->region_count = 2;

	pci_register_hose(hose);

#if defined(CONFIG_TARGET_MPC8555CDS) || defined(CONFIG_TARGET_MPC8541CDS)
	/*
	 * This is a SW workaround for an apparent HW problem
	 * in the PCI controller on the MPC85555/41 CDS boards.
	 * The first config cycle must be to a valid, known
	 * device on the PCI bus in order to trick the PCI
	 * controller state machine into a known valid state.
	 * Without this, the first config cycle has the chance
	 * of hanging the controller permanently, just leaving
	 * it in a semi-working state, or leaving it working.
	 *
	 * Pick on the Tundra, Device 17, to get it right.
	 */
	{
		u8 header_type;

		pci_hose_read_config_byte(hose,
					  PCI_BDF(0,BRIDGE_ID,0),
					  PCI_HEADER_TYPE,
					  &header_type);
	}
#endif

	hose->last_busno = pci_hose_scan(hose);

#ifdef CONFIG_MPC85XX_PCI2
	hose = &pci_hose[1];

	hose->first_busno = pci_hose[0].last_busno + 1;
	hose->last_busno = 0xff;

	pci_setup_indirect(hose,
			   (CONFIG_SYS_IMMR+0x9000),
			   (CONFIG_SYS_IMMR+0x9004));

	dev = PCI_BDF(hose->first_busno, 0, 0);
	pci_hose_read_config_word (hose, dev, PCI_COMMAND, &reg16);
	reg16 |= PCI_COMMAND_SERR | PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY;
	pci_hose_write_config_word(hose, dev, PCI_COMMAND, reg16);

	/*
	 * Clear non-reserved bits in status register.
	 */
	pci_hose_write_config_word(hose, dev, PCI_STATUS, 0xffff);

	pcix2->potar1   = (CONFIG_SYS_PCI2_MEM_BUS >> 12) & 0x000fffff;
	pcix2->potear1  = 0x00000000;
	pcix2->powbar1  = (CONFIG_SYS_PCI2_MEM_PHYS >> 12) & 0x000fffff;
	pcix2->powbear1 = 0x00000000;
	pcix2->powar1 = (POWAR_EN | POWAR_MEM_READ |
			POWAR_MEM_WRITE | (__ilog2(CONFIG_SYS_PCI2_MEM_SIZE) - 1));

	pcix2->potar2  = (CONFIG_SYS_PCI2_IO_BUS >> 12) & 0x000fffff;
	pcix2->potear2  = 0x00000000;
	pcix2->powbar2  = (CONFIG_SYS_PCI2_IO_PHYS >> 12) & 0x000fffff;
	pcix2->powbear2 = 0x00000000;
	pcix2->powar2 = (POWAR_EN | POWAR_IO_READ |
			POWAR_IO_WRITE | (__ilog2(CONFIG_SYS_PCI2_IO_SIZE) - 1));

	pcix2->pitar1 = 0x00000000;
	pcix2->piwbar1 = 0x00000000;
	pcix2->piwar1 = (PIWAR_EN | PIWAR_PF | PIWAR_LOCAL |
			PIWAR_READ_SNOOP | PIWAR_WRITE_SNOOP | PIWAR_MEM_2G);

	pcix2->powar3 = 0;
	pcix2->powar4 = 0;
	pcix2->piwar2 = 0;
	pcix2->piwar3 = 0;

	pci_set_region(hose->regions + 0,
		       CONFIG_SYS_PCI2_MEM_BUS,
		       CONFIG_SYS_PCI2_MEM_PHYS,
		       CONFIG_SYS_PCI2_MEM_SIZE,
		       PCI_REGION_MEM);

	pci_set_region(hose->regions + 1,
		       CONFIG_SYS_PCI2_IO_BUS,
		       CONFIG_SYS_PCI2_IO_PHYS,
		       CONFIG_SYS_PCI2_IO_SIZE,
		       PCI_REGION_IO);

	hose->region_count = 2;

	/*
	 * Hose scan.
	 */
	pci_register_hose(hose);

	hose->last_busno = pci_hose_scan(hose);
#endif
}
#endif /* !CONFIG_FSL_PCI_INIT */
