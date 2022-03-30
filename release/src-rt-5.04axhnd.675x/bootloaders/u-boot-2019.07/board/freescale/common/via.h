#ifndef _MPC85xx_VIA_H
void mpc85xx_config_via(struct pci_controller* hose, pci_dev_t dev, struct pci_config_table *tab);

/* Function 1, IDE */
void mpc85xx_config_via_usbide(struct pci_controller* hose, pci_dev_t dev, struct pci_config_table *tab);

/* Function 2, USB ports 0-1 */
void mpc85xx_config_via_usb(struct pci_controller* hose, pci_dev_t dev, struct pci_config_table *tab);

/* Function 3, USB ports 2-3 */
void mpc85xx_config_via_usb2(struct pci_controller* hose, pci_dev_t dev, struct pci_config_table *tab);

/* Function 5, Power Management */
void mpc85xx_config_via_power(struct pci_controller* hose, pci_dev_t dev, struct pci_config_table *tab);

/* Function 6, AC97 Interface */
void mpc85xx_config_via_ac97(struct pci_controller* hose, pci_dev_t dev, struct pci_config_table *tab);
#endif /* _MPC85xx_VIA_H */
