// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2004-2007, 2012 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

/*
 * PCI Configuration space access support
 */
#include <common.h>
#include <pci.h>
#include <asm/io.h>
#include <asm/immap.h>

#if defined(CONFIG_PCI)
/* System RAM mapped over PCI */
#define CONFIG_SYS_PCI_SYS_MEM_BUS	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_PCI_SYS_MEM_PHYS	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_PCI_SYS_MEM_SIZE	(1024 * 1024 * 1024)

#define cfg_read(val, addr, type, op)		*val = op((type)(addr));
#define cfg_write(val, addr, type, op)		op((type *)(addr), (val));

#define PCI_OP(rw, size, type, op, mask)				\
int pci_##rw##_cfg_##size(struct pci_controller *hose,			\
	pci_dev_t dev, int offset, type val)				\
{									\
	u32 addr = 0;							\
	u16 cfg_type = 0;						\
	addr = ((offset & 0xfc) | cfg_type | (dev)  | 0x80000000);	\
	out_be32(hose->cfg_addr, addr);					\
	cfg_##rw(val, hose->cfg_data + (offset & mask), type, op);	\
	out_be32(hose->cfg_addr, addr & 0x7fffffff);			\
	return 0;							\
}

PCI_OP(read, byte, u8 *, in_8, 3)
PCI_OP(read, word, u16 *, in_le16, 2)
PCI_OP(read, dword, u32 *, in_le32, 0)
PCI_OP(write, byte, u8, out_8, 3)
PCI_OP(write, word, u16, out_le16, 2)
PCI_OP(write, dword, u32, out_le32, 0)

void pci_mcf5445x_init(struct pci_controller *hose)
{
	pci_t *pci = (pci_t *)MMAP_PCI;
	pciarb_t *pciarb = (pciarb_t *)MMAP_PCIARB;
	gpio_t *gpio = (gpio_t *) MMAP_GPIO;
	u32 barEn = 0;

	out_be32(&pciarb->acr, 0x001f001f);

	/* Set PCIGNT1, PCIREQ1, PCIREQ0/PCIGNTIN, PCIGNT0/PCIREQOUT,
	   PCIREQ2, PCIGNT2 */
	out_be16(&gpio->par_pci,
		GPIO_PAR_PCI_GNT3_GNT3 | GPIO_PAR_PCI_GNT2 |
		GPIO_PAR_PCI_GNT1 | GPIO_PAR_PCI_GNT0 |
		GPIO_PAR_PCI_REQ3_REQ3 | GPIO_PAR_PCI_REQ2 |
		GPIO_PAR_PCI_REQ1 | GPIO_PAR_PCI_REQ0);

	/* Assert reset bit */
	setbits_be32(&pci->gscr, PCI_GSCR_PR);

	setbits_be32(&pci->tcr1, PCI_TCR1_P);

	/* Initiator windows */
	out_be32(&pci->iw0btar,
		CONFIG_SYS_PCI_MEM_PHYS | (CONFIG_SYS_PCI_MEM_PHYS >> 16));
	out_be32(&pci->iw1btar,
		CONFIG_SYS_PCI_IO_PHYS | (CONFIG_SYS_PCI_IO_PHYS >> 16));
	out_be32(&pci->iw2btar,
		CONFIG_SYS_PCI_CFG_PHYS | (CONFIG_SYS_PCI_CFG_PHYS >> 16));

	out_be32(&pci->iwcr,
		PCI_IWCR_W0C_EN | PCI_IWCR_W1C_EN | PCI_IWCR_W1C_IO |
		PCI_IWCR_W2C_EN | PCI_IWCR_W2C_IO);

	out_be32(&pci->icr, 0);

	/* Enable bus master and mem access */
	out_be32(&pci->scr, PCI_SCR_B | PCI_SCR_M);

	/* Cache line size and master latency */
	out_be32(&pci->cr1, PCI_CR1_CLS(8) | PCI_CR1_LTMR(0xF8));
	out_be32(&pci->cr2, 0);

#ifdef CONFIG_SYS_PCI_BAR0
	out_be32(&pci->bar0, PCI_BAR_BAR0(CONFIG_SYS_PCI_BAR0));
	out_be32(&pci->tbatr0, CONFIG_SYS_PCI_TBATR0 | PCI_TBATR_EN);
	barEn |= PCI_TCR2_B0E;
#endif
#ifdef CONFIG_SYS_PCI_BAR1
	out_be32(&pci->bar1, PCI_BAR_BAR1(CONFIG_SYS_PCI_BAR1));
	out_be32(&pci->tbatr1, CONFIG_SYS_PCI_TBATR1 | PCI_TBATR_EN);
	barEn |= PCI_TCR2_B1E;
#endif
#ifdef CONFIG_SYS_PCI_BAR2
	out_be32(&pci->bar2, PCI_BAR_BAR2(CONFIG_SYS_PCI_BAR2));
	out_be32(&pci->tbatr2, CONFIG_SYS_PCI_TBATR2 | PCI_TBATR_EN);
	barEn |= PCI_TCR2_B2E;
#endif
#ifdef CONFIG_SYS_PCI_BAR3
	out_be32(&pci->bar3, PCI_BAR_BAR3(CONFIG_SYS_PCI_BAR3));
	out_be32(&pci->tbatr3, CONFIG_SYS_PCI_TBATR3 | PCI_TBATR_EN);
	barEn |= PCI_TCR2_B3E;
#endif
#ifdef CONFIG_SYS_PCI_BAR4
	out_be32(&pci->bar4, PCI_BAR_BAR4(CONFIG_SYS_PCI_BAR4));
	out_be32(&pci->tbatr4, CONFIG_SYS_PCI_TBATR4 | PCI_TBATR_EN);
	barEn |= PCI_TCR2_B4E;
#endif
#ifdef CONFIG_SYS_PCI_BAR5
	out_be32(&pci->bar5, PCI_BAR_BAR5(CONFIG_SYS_PCI_BAR5));
	out_be32(&pci->tbatr5, CONFIG_SYS_PCI_TBATR5 | PCI_TBATR_EN);
	barEn |= PCI_TCR2_B5E;
#endif

	out_be32(&pci->tcr2, barEn);

	/* Deassert reset bit */
	clrbits_be32(&pci->gscr, PCI_GSCR_PR);
	udelay(1000);

	/* Enable PCI bus master support */
	hose->first_busno = 0;
	hose->last_busno = 0xff;

	pci_set_region(hose->regions + 0, CONFIG_SYS_PCI_MEM_BUS, CONFIG_SYS_PCI_MEM_PHYS,
		       CONFIG_SYS_PCI_MEM_SIZE, PCI_REGION_MEM);

	pci_set_region(hose->regions + 1, CONFIG_SYS_PCI_IO_BUS, CONFIG_SYS_PCI_IO_PHYS,
		       CONFIG_SYS_PCI_IO_SIZE, PCI_REGION_IO);

	pci_set_region(hose->regions + 2, CONFIG_SYS_PCI_SYS_MEM_BUS,
		       CONFIG_SYS_PCI_SYS_MEM_PHYS, CONFIG_SYS_PCI_SYS_MEM_SIZE,
		       PCI_REGION_MEM | PCI_REGION_SYS_MEMORY);

	hose->region_count = 3;

	hose->cfg_addr = &(pci->car);
	hose->cfg_data = (volatile unsigned char *)CONFIG_SYS_PCI_CFG_BUS;

	pci_set_ops(hose, pci_read_cfg_byte, pci_read_cfg_word,
		    pci_read_cfg_dword, pci_write_cfg_byte, pci_write_cfg_word,
		    pci_write_cfg_dword);

	/* Hose scan */
	pci_register_hose(hose);
	hose->last_busno = pci_hose_scan(hose);
}
#endif				/* CONFIG_PCI */
