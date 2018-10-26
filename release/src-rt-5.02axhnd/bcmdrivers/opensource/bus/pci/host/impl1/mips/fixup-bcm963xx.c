#if defined(CONFIG_BCM_KF_PCI_FIXUP)
/* 
* <:copyright-BRCM:2011:GPL/GPL:standard
* 
*    Copyright (c) 2011 Broadcom 
*    All Rights Reserved
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, version 2, as published by
* the Free Software Foundation (the "GPL").
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* 
* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
* 
* :>
*/
#include <linux/init.h>
#include <linux/types.h>
#include <linux/pci.h>

#include <bcm_intr.h>
#include <bcm_map_part.h>
#include <bcmpci.h>

#if defined(CONFIG_BCM963268) ||         defined(CONFIG_USB)
static char irq_tab_bcm63xx[] __initdata = {
#if defined(CONFIG_BCM963268)
    [WLAN_ONCHIP_DEV_SLOT] = INTERRUPT_ID_WLAN,
#endif
#if defined(CONFIG_USB)
    [USB_HOST_SLOT] = INTERRUPT_ID_USBH,
    [USB20_HOST_SLOT] = INTERRUPT_ID_USBH20
#endif
};
#endif

static void bcm63xx_fixup_header(struct pci_dev *dev)
{
    uint32 memaddr;
    uint32 size;

    memaddr = pci_resource_start(dev, 0);
    size = pci_resource_len(dev, 0);

    if (dev->bus->number == BCM_BUS_PCI) {
        switch (PCI_SLOT(dev->devfn)) {
#if defined(CONFIG_USB)
            case USB_HOST_SLOT:
                dev->resource[0].flags |= IORESOURCE_PCI_FIXED; // prevent linux from reallocating resources
                break;
    
            case USB20_HOST_SLOT:
               dev->resource[0].flags |= IORESOURCE_PCI_FIXED; // prevent linux from reallocating resources
               break;
#endif
#if defined(WLAN_CHIPC_BASE)
            case WLAN_ONCHIP_DEV_SLOT:
               dev->resource[0].flags |= IORESOURCE_PCI_FIXED; // prevent linux from reallocating resources
               break;
#endif   
        }
    }
}
DECLARE_PCI_FIXUP_HEADER(PCI_ANY_ID, PCI_ANY_ID, bcm63xx_fixup_header);


static void bcm63xx_fixup_final(struct pci_dev *dev)
{
    uint32 memaddr;
    uint32 size;

    memaddr = pci_resource_start(dev, 0);
    size = pci_resource_len(dev, 0);

    if (dev->bus->number == BCM_BUS_PCI) {
        switch (PCI_SLOT(dev->devfn)) {
         }
    } 
}
DECLARE_PCI_FIXUP_FINAL(PCI_ANY_ID, PCI_ANY_ID, bcm63xx_fixup_final);



int pcibios_plat_dev_init(struct pci_dev *dev)
{
    return 0;
}

int __init pcibios_map_irq(const struct pci_dev *dev, u8 slot, u8 pin)
{

#if defined(PCIEH)
    if (pci_is_pcie((struct pci_dev*)dev)) {

#if defined(PCIEH_1)
        if ((dev->bus->number >= BCM_BUS_PCIE1_ROOT)&& (dev->bus->number <= BCM_BUS_PCIE1_DEVICE))
            return INTERRUPT_ID_PCIE1_RC;
        
        if ((dev->bus->number >= BCM_BUS_PCIE_ROOT) && (dev->bus->number <= BCM_BUS_PCIE_DEVICE))
            return INTERRUPT_ID_PCIE_RC;
#endif
        /* single RC */
        return INTERRUPT_ID_PCIE_RC;
    }
#endif /* PCIEH */
    
#if defined(PCI_CFG) ||  defined(WLAN_CHIPC_BASE) ||   defined(CONFIG_USB)
    return irq_tab_bcm63xx[slot];
#else
    return 0;    
#endif

}

#endif
