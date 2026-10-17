/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2008
 * Benjamin Warren, biggerbadderben@gmail.com
 */

/*
 * netdev.h - definitions an prototypes for network devices
 */

#ifndef _NETDEV_H_
#define _NETDEV_H_
#include <phy_interface.h>

struct udevice;

/*
 * Board and CPU-specific initialization functions
 * board_eth_init() has highest priority.  cpu_eth_init() only
 * gets called if board_eth_init() isn't instantiated or fails.
 * Return values:
 *      0: success
 *     -1: failure
 */

int board_eth_init(struct bd_info *bis);
int board_interface_eth_init(struct udevice *dev,
			     phy_interface_t interface_type);
int cpu_eth_init(struct bd_info *bis);

/* Driver initialization prototypes */
int ax88180_initialize(struct bd_info *bis);
int bcm_sf2_eth_register(struct bd_info *bis, u8 dev_num);
int bfin_EMAC_initialize(struct bd_info *bis);
int cs8900_initialize(u8 dev_num, int base_addr);
int dc21x4x_initialize(struct bd_info *bis);
int designware_initialize(ulong base_addr, u32 interface);
int dm9000_initialize(struct bd_info *bis);
int dnet_eth_initialize(int id, void *regs, unsigned int phy_addr);
int e1000_initialize(struct bd_info *bis);
int eepro100_initialize(struct bd_info *bis);
int ep93xx_eth_initialize(u8 dev_num, int base_addr);
int eth_3com_initialize (struct bd_info * bis);
int ethoc_initialize(u8 dev_num, int base_addr);
int fecmxc_initialize(struct bd_info *bis);
int fecmxc_initialize_multi(struct bd_info *bis, int dev_id, int phy_id,
			    uint32_t addr);
int ftmac100_initialize(struct bd_info *bits);
int ftmac110_initialize(struct bd_info *bits);
void gt6426x_eth_initialize(struct bd_info *bis);
int ks8851_mll_initialize(u8 dev_num, int base_addr);
int lan91c96_initialize(u8 dev_num, int base_addr);
int lpc32xx_eth_initialize(struct bd_info *bis);
int macb_eth_initialize(int id, void *regs, unsigned int phy_addr);
int mcdmafec_initialize(struct bd_info *bis);
int mcffec_initialize(struct bd_info *bis);
int mvgbe_initialize(struct bd_info *bis);
int mvneta_initialize(struct bd_info *bis, int base_addr, int devnum,
		      int phy_addr);
int natsemi_initialize(struct bd_info *bis);
int ne2k_register(void);
int npe_initialize(struct bd_info *bis);
int ns8382x_initialize(struct bd_info *bis);
int pcnet_initialize(struct bd_info *bis);
int ppc_4xx_eth_initialize (struct bd_info *bis);
int rtl8139_initialize(struct bd_info *bis);
int rtl8169_initialize(struct bd_info *bis);
int scc_initialize(struct bd_info *bis);
int sh_eth_initialize(struct bd_info *bis);
int skge_initialize(struct bd_info *bis);
int smc91111_initialize(u8 dev_num, phys_addr_t base_addr);
int smc911x_initialize(u8 dev_num, phys_addr_t base_addr);
int uli526x_initialize(struct bd_info *bis);

/* Boards with PCI network controllers can call this from their board_eth_init()
 * function to initialize whatever's on board.
 * Return value is total # of devices found */

static inline int pci_eth_init(struct bd_info *bis)
{
	int num = 0;

#ifdef CONFIG_PCI

#ifdef CONFIG_EEPRO100
	num += eepro100_initialize(bis);
#endif
#ifdef CONFIG_TULIP
	num += dc21x4x_initialize(bis);
#endif
#ifdef CONFIG_E1000
	num += e1000_initialize(bis);
#endif
#ifdef CONFIG_PCNET
	num += pcnet_initialize(bis);
#endif
#ifdef CONFIG_NATSEMI
	num += natsemi_initialize(bis);
#endif
#ifdef CONFIG_NS8382X
	num += ns8382x_initialize(bis);
#endif
#if defined(CONFIG_RTL8139)
	num += rtl8139_initialize(bis);
#endif
#if defined(CONFIG_RTL8169)
	num += rtl8169_initialize(bis);
#endif
#if defined(CONFIG_ULI526X)
	num += uli526x_initialize(bis);
#endif

#endif  /* CONFIG_PCI */
	return num;
}

struct mii_dev *fec_get_miibus(ulong base_addr, int dev_id);

#ifdef CONFIG_PHYLIB
struct phy_device;
int fec_probe(struct bd_info *bd, int dev_id, uint32_t base_addr,
		struct mii_dev *bus, struct phy_device *phydev);
#else
/*
 * Allow FEC to fine-tune MII configuration on boards which require this.
 */
struct eth_device;
int fecmxc_register_mii_postcall(struct eth_device *dev, int (*cb)(int));
#endif

#endif /* _NETDEV_H_ */
