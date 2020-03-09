/*
 *	The PCI Library -- Example of use (simplistic lister of PCI devices)
 *
 *	Written by Martin Mares and put to public domain. You can do
 *	with it anything you want, but I don't give you any warranty.
 */

#include <stdio.h>

#include "lib/pci.h"

int main(void)
{
  struct pci_access *pacc;
  struct pci_dev *dev;
  unsigned int c;
  char namebuf[1024], *name;

  pacc = pci_alloc();		/* Get the pci_access structure */
  /* Set all options you want -- here we stick with the defaults */
  pci_init(pacc);		/* Initialize the PCI library */
  pci_scan_bus(pacc);		/* We want to get the list of devices */
  for (dev=pacc->devices; dev; dev=dev->next)	/* Iterate over all devices */
    {
      pci_fill_info(dev, PCI_FILL_IDENT | PCI_FILL_BASES | PCI_FILL_CLASS);	/* Fill in header info we need */
      c = pci_read_byte(dev, PCI_INTERRUPT_PIN);				/* Read config register directly */
      printf("%04x:%02x:%02x.%d vendor=%04x device=%04x class=%04x irq=%d (pin %d) base0=%lx",
	     dev->domain, dev->bus, dev->dev, dev->func, dev->vendor_id, dev->device_id,
	     dev->device_class, dev->irq, c, (long) dev->base_addr[0]);

      /* Look up and print the full name of the device */
      name = pci_lookup_name(pacc, namebuf, sizeof(namebuf), PCI_LOOKUP_DEVICE, dev->vendor_id, dev->device_id);
      printf(" (%s)\n", name);
    }
  pci_cleanup(pacc);		/* Close everything */
  return 0;
}
