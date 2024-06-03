// SPDX-License-Identifier: GPL-2.0
/*
 *  EHCI HCD (Host Controller Driver) for USB.
 *
 *  Copyright (C) 2013,2014 Renesas Electronics Corporation
 *  Copyright (C) 2014 Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/ehci-rmobile.h>
#include "ehci.h"

#if defined(CONFIG_R8A7740)
static u32 usb_base_address[] = {
	0xC6700000
};
#elif defined(CONFIG_R8A7790)
static u32 usb_base_address[] = {
	0xEE080000,	/* USB0 (EHCI) */
	0xEE0A0000,	/* USB1 */
	0xEE0C0000,	/* USB2 */
};
#elif defined(CONFIG_R8A7791) || defined(CONFIG_R8A7793) || \
	defined(CONFIG_R8A7794)
static u32 usb_base_address[] = {
	0xEE080000,	/* USB0 (EHCI) */
	0xEE0C0000,	/* USB1 */
};
#else
#error rmobile EHCI USB driver not supported on this platform
#endif

int ehci_hcd_stop(int index)
{
	int i;
	u32 base;
	struct ahbcom_pci_bridge *ahbcom_pci;

	base = usb_base_address[index];
	ahbcom_pci = (struct ahbcom_pci_bridge *)(base + AHBPCI_OFFSET);
	writel(0, &ahbcom_pci->ahb_bus_ctr);

	/* reset ehci */
	setbits_le32(base + EHCI_USBCMD, CMD_RESET);
	for (i = 100; i > 0; i--) {
		if (!(readl(base + EHCI_USBCMD) & CMD_RESET))
			break;
		udelay(100);
	}

	if (!i)
		printf("error : ehci(%d) reset failed.\n", index);

	if (index == (ARRAY_SIZE(usb_base_address) - 1))
		setbits_le32(SMSTPCR7, SMSTPCR703);

	return 0;
}

int ehci_hcd_init(int index, enum usb_init_type init,
	struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	u32 base;
	u32 phys_base;
	struct rmobile_ehci_reg *rehci;
	struct ahbcom_pci_bridge *ahbcom_pci;
	struct ahbconf_pci_bridge *ahbconf_pci;
	struct ahb_pciconf *ahb_pciconf_ohci;
	struct ahb_pciconf *ahb_pciconf_ehci;
	uint32_t cap_base;

	base = usb_base_address[index];
	phys_base = base;
	if (index == 0)
		clrbits_le32(SMSTPCR7, SMSTPCR703);

	rehci = (struct rmobile_ehci_reg *)(base + EHCI_OFFSET);
	ahbcom_pci = (struct ahbcom_pci_bridge *)(base + AHBPCI_OFFSET);
	ahbconf_pci =
		(struct ahbconf_pci_bridge *)(base + PCI_CONF_AHBPCI_OFFSET);
	ahb_pciconf_ohci = (struct ahb_pciconf *)(base + PCI_CONF_OHCI_OFFSET);
	ahb_pciconf_ehci = (struct ahb_pciconf *)(base + PCI_CONF_EHCI_OFFSET);

	/* Clock & Reset & Direct Power Down */
	clrsetbits_le32(&ahbcom_pci->usbctr,
			(DIRPD | PCICLK_MASK | USBH_RST), USBCTR_WIN_SIZE_1GB);
	clrbits_le32(&ahbcom_pci->usbctr, PLL_RST);

	/* AHB-PCI Bridge Communication Registers */
	writel(AHB_BUS_CTR_INIT, &ahbcom_pci->ahb_bus_ctr);
	writel((CONFIG_SYS_SDRAM_BASE & 0xf0000000) | PCIAHB_WIN_PREFETCH,
	       &ahbcom_pci->pciahb_win1_ctr);
	writel(0xf0000000 | PCIAHB_WIN_PREFETCH,
	       &ahbcom_pci->pciahb_win2_ctr);
	writel(phys_base | PCIWIN2_PCICMD, &ahbcom_pci->ahbpci_win2_ctr);

	setbits_le32(&ahbcom_pci->pci_arbiter_ctr,
		     PCIBP_MODE | PCIREQ1 | PCIREQ0);

	/* PCI Configuration Registers for AHBPCI */
	writel(PCIWIN1_PCICMD | AHB_CFG_AHBPCI,
	       &ahbcom_pci->ahbpci_win1_ctr);
	writel(phys_base + AHBPCI_OFFSET, &ahbconf_pci->basead);
	writel(CONFIG_SYS_SDRAM_BASE & 0xf0000000, &ahbconf_pci->win1_basead);
	writel(0xf0000000, &ahbconf_pci->win2_basead);
	writel(SERREN | PERREN | MASTEREN | MEMEN,
	       &ahbconf_pci->cmnd_sts);

	/* PCI Configuration Registers for EHCI */
	writel(PCIWIN1_PCICMD | AHB_CFG_HOST, &ahbcom_pci->ahbpci_win1_ctr);
	writel(phys_base + OHCI_OFFSET, &ahb_pciconf_ohci->basead);
	writel(phys_base + EHCI_OFFSET, &ahb_pciconf_ehci->basead);
	writel(SERREN | PERREN | MASTEREN | MEMEN,
	       &ahb_pciconf_ohci->cmnd_sts);
	writel(SERREN | PERREN | MASTEREN | MEMEN,
	       &ahb_pciconf_ehci->cmnd_sts);

	/* Enable PCI interrupt */
	setbits_le32(&ahbcom_pci->pci_int_enable,
		     USBH_PMEEN | USBH_INTBEN | USBH_INTAEN);

	*hccr = (struct ehci_hccr *)((uint32_t)&rehci->hciversion);
	cap_base = ehci_readl(&(*hccr)->cr_capbase);
	*hcor = (struct ehci_hcor *)((uint32_t)*hccr + HC_LENGTH(cap_base));

	return 0;
}
