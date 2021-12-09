#if defined(CONFIG_BCM_KF_ARM_BCM963XX)
/*
<:copyright-BRCM:2015:DUAL/GPL:standard

   Copyright (c) 2015 Broadcom
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include <linux/of_pci.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/delay.h>

#include <board.h>
#include <shared_utils.h>

#include <pcie_common.h>
#include <pcie-bcm947xx.h>

/**************************************
  *
  *  Defines
  *
  **************************************/
/***********
  * PCI - PCIE
  ***********/
#define PCI_EXP_DEVCAP_PAYLOAD_512B                2
#define PCI_EXP_DEVCTL_PAYLOAD_512B                (2 << 5)

/***********
  * HCD Driver
  ***********/
#define BCM947XX_ROOT_BUSNUM                       0x00
#define BCM947XX_MAX_BUSNUM                        0x01

/***********
  * PCIe MSI
  ***********/
#define MISC_MSI_DATA_CONFIG_MATCH_MAGIC           0x0000BCA0
#define MSI_ISR_NAME_STR_LEN                       32

/**************************************
 *
 *  Macros
 *
 **************************************/

/***********
  * DT Binding
  ***********/
/* don't fail on DT error until DT is tested and ready working */
#ifdef FAIL_ON_DT_ERROR
#define HCD_FAIL_ON_DT_ERROR(res, err)             \
	if (err) {                                     \
	    HCD_WARN("No DT etnry for %s\n",res);      \
	    return err;                                \
	}
#else
#define HCD_FAIL_ON_DT_ERROR(res, err)             \
	if (err)                                       \
	    HCD_WARN("No DT etnry for %s, using defaults\n",res);
#endif



/**************************************
 *
 *  Structures
 *
 **************************************/

/**************************************
 *
 *  Local Function prototype
 *
 **************************************/

static int bcm947xx_pcie_config_read(struct pci_bus *bus, unsigned int devfn,
                                   int where, int size, u32 *val);
static int bcm947xx_pcie_config_write(struct pci_bus *bus, unsigned int devfn,
                                    int where, int size, u32 val);
static int bcm947xx_pcie_map_irq(const struct pci_dev *pcidev, u8 slot,
	u8 pin);

static void bcm947xx_pcie_fixup_final_mps(struct pci_dev *dev);
static int bcm947xx_pcie_core_config(struct bcm947xx_pcie_hcd *pdrv);
static void __init noinline bcm947xx_pcie_bridge_init(struct bcm947xx_pcie_hcd *pdrv);
static int __init noinline bcm947xx_pcie_check_link(struct bcm947xx_pcie_hcd *pdrv, uint32 allow_gen2);
static void __init bcm947xx_pcie_map_init(struct bcm947xx_pcie_hcd *pdrv);
static bool __init bcm947xx_pcie_hw_init(struct bcm947xx_pcie_hcd *pdrv);
static bool __init bcm947xx_pcie_is_pcie_link_up(struct bcm947xx_pcie_hcd *pdrv);
static void bcm947xx_pcie_core_reset(struct bcm947xx_pcie_hcd *pdrv);


static int bcm947xx_pcie_setup_owin(struct bcm947xx_pcie_hcd *pdrv,
	struct list_head *resources);
static int bcm947xx_pcie_setup_regs(struct bcm947xx_pcie_hcd *pdrv);
static void bcm947xx_pcie_unmap_res(struct bcm947xx_pcie_hcd *pdrv);
static int bcm947xx_pcie_parse_dt(struct bcm947xx_pcie_hcd *pdrv);
static int bcm947xx_pcie_init_res(struct bcm947xx_pcie_hcd *pdrv);

static int __init bcm947xx_pcie_probe(struct platform_device *pdev);
static int bcm947xx_pcie_remove(struct platform_device *pdev);
static int __init bcm947xx_pcie_init(void);
static void bcm947xx_pcie_exit(void);
static int __init bcm947xx_pcie_plt_init(void);
static void __init bcm947xx_hndpci_init(void);


/**************************************
 *
 *  external Function prototype
 *
 **************************************/
extern unsigned long getMemorySize(void);

/* Global PCIE config space array for internal radio */
static si_pci_cfg_t si_pci_cfg[MAX_SOC_D11];

/**************************************
 *
 *  Global variables
 *
 **************************************/
static struct platform_device bcm947xx_pcie_plt_dev[NUM_CORE];

static struct pci_ops bcm947xx_pcie_ops = {
	.read = bcm947xx_pcie_config_read,
	.write = bcm947xx_pcie_config_write,
};

static const struct of_device_id bcm947xx_pcie_of_match[] = {
	{
	    .type = "pci",
	    .compatible = "brcm,bcm947xx_pcie",
	},
};
MODULE_DEVICE_TABLE(of, bcm947xx_pcie_of_match);

static struct platform_driver bcm947xx_pcie_driver = {
	.probe  = bcm947xx_pcie_probe,
	.driver = {
	    .name  = BCM947XX_PCIE_DRV_NAME,
	    .owner = THIS_MODULE,
	    .of_match_table = of_match_ptr(bcm947xx_pcie_of_match),
	},
	.remove = bcm947xx_pcie_remove,
};

/**************************************
 *
 *  Local inline functions
 *
 **************************************/
/* read 32bit pcie register space */
static inline u32 hcd_readl(void __iomem *addr, unsigned offset)
{
	writel(offset, addr + PCIE_GEN2_PCIE_EXT_CFG_ADDR);
	return readl(addr + PCIE_GEN2_PCIE_EXT_CFG_DATA);
}

/* write 32bit pcie register space */
static inline void hcd_writel(u32 data, void __iomem *addr, unsigned offset)
{
	writel(offset, addr + PCIE_GEN2_PCIE_EXT_CFG_ADDR);
	writel(data, addr + PCIE_GEN2_PCIE_EXT_CFG_DATA);
}

/**************************************
 *
 *  Local Functions
 *
 **************************************/

static void __iomem *bcm947xx_pcie_cfg_base(struct pci_bus *bus, unsigned int devfn, int where)
{
	struct bcm947xx_pcie_hcd *pdrv = bus->sysdata;
	void __iomem *base;
	int offset;
	int busno = bus->number;
	int devno = PCI_SLOT(devfn);
	int fnno = PCI_FUNC(devfn);
	int type;
	u32 addr_reg;

	base = pdrv->base;

	/* If there is no link, just show the PCI bridge. */
	if (busno == 0) {
		if (devno>= 1) {
			return NULL;
		}
		writel(where & 0x1ffc, pdrv->base + PCIE_GEN2_PCIE_EXT_CFG_ADDR);
		offset = PCIE_GEN2_PCIE_EXT_CFG_DATA;
	} else {
		/* WAR for function num > 1 */
		if (fnno > 1) {
			return NULL;
		}
		type = 1;
		addr_reg = (busno & 0xff) << 20 |
			(devno << 15) |
			(fnno << 12) |
			(where & 0xffc) |
			(type & 0x3);

		writel(addr_reg, pdrv->base + PCIE_GEN2_PCIE_CFG_ADDR);
		offset = PCIE_GEN2_PCIE_CFG_DATA;
	}

	return base + offset;
}

#define sprom_control   dev_dep[0x88 - 0x40]

static void __init bcm947xx_hndpci_init(void)
{
	pci_config_regs *cfg;
	si_bar_cfg_t *bar;
	int i, j, d11idx;
	
	memset(si_pci_cfg, 0, sizeof(si_pci_cfg));
	
	for (i = 0; i < MAX_SOC_D11; i++) {
		cfg = &si_pci_cfg[i].emu;
		bar = &si_pci_cfg[i].bar;
		d11idx = (i == 0) ? D11_CORE0_IDX : D11_CORE1_IDX;
		cfg->vendor = VENDOR_BROADCOM;
		/* Device ID should be read from NVRAM.
		 * But this should not have any impact since wl
		 * driver should read the nvram and overwrite it.
		 */
		cfg->device = 0x43c8;
		cfg->rev_id = 0x0;
		/* programming interface should be 0 for
		 * other network controller
		 */
		cfg->prog_if = 0x0;
		cfg->sub_class = PCI_NET_OTHER;
		cfg->base_class = PCI_CLASS_NET;
		cfg->header_type = PCI_HEADER_NORMAL;
		cfg->base[0] = CHIPCOMMON_PHYS_BASE + (d11idx * SI_CORE_SIZE);
		/* Save core interrupt flag */
		cfg->int_pin = d11idx;
		/* Save core interrupt assignment */
		cfg->int_line = 0;
		/* Indicate there is no SROM */
		*((uint32 *)&cfg->sprom_control) = 0xffffffff;
		
		bar->n = 1;;
		bar->size[0] = SI_CORE_SIZE;

		/* Using addrspace 0 only */
		for (j = 1; j < PCI_BAR_MAX; j++) {
		    cfg->base[j] = 0;
		    bar->size[j] = 0;
		}
	}
}

static int
bcm947xx_si_pcid_read_config(struct pci_bus *bus, unsigned int devfn, int off, int len, u32 *buf)
{
	pci_config_regs *cfg;
	int slotno = PCI_SLOT(devfn);

	cfg = (slotno == D11_CORE0_IDX) ? &si_pci_cfg[0].emu : &si_pci_cfg[1].emu;

	if (len == 4)
	    *((uint32 *)buf) = (*((uint32 *)((ulong) cfg + off)));
	else if (len == 2)
	    *((uint16 *)buf) = (*((uint16 *)((ulong) cfg + off)));
	else if (len == 1)
	    *((uint8 *)buf) = *((uint8 *)((ulong) cfg + off));
	else {
	    *buf = ~0UL;
	}

	return PCIBIOS_SUCCESSFUL;
}
static int
bcm947xx_si_pcid_write_config(struct pci_bus *bus, unsigned int devfn, int off, int len, u32 *buf)
{
	pci_config_regs *cfg;
	si_bar_cfg_t *bar;
	int slotno = PCI_SLOT(devfn);

	cfg = (slotno == D11_CORE0_IDX) ? &si_pci_cfg[0].emu : &si_pci_cfg[1].emu;
	bar = (slotno == D11_CORE0_IDX) ? &si_pci_cfg[0].bar : &si_pci_cfg[1].bar;

	/* Emulate BAR sizing */
	if (off >= OFFSETOF(pci_config_regs, base[0]) &&
		off <= OFFSETOF(pci_config_regs, base[3]) &&
		len == 4 && *((uint32 *)buf) == ~0) {
	        /* Highest numbered address match register */
	        if (off == OFFSETOF(pci_config_regs, base[0]))
		   cfg->base[0] = ~(bar->size[0] - 1);
	        else if (off == OFFSETOF(pci_config_regs, base[1]) && bar->n >= 1) {
		   cfg->base[1] = ~(bar->size[1] - 1);
		} else if (off == OFFSETOF(pci_config_regs, base[2]) && bar->n >= 2) {
		   cfg->base[2] = ~(bar->size[2] - 1);
		} else if (off == OFFSETOF(pci_config_regs, base[3]) && bar->n >= 3)
		   cfg->base[3] = ~(bar->size[3] - 1);
	} else if (len == 4)
		*((uint32 *)((ulong) cfg + off)) = *((uint32 *)buf);
	else if (len == 2)
		*((uint16 *)((ulong) cfg + off)) = *((uint16 *)buf);
	else if (len == 1)
		*((uint8 *)((ulong) cfg + off)) = *((uint8 *)buf);
	else 
	    printk("[%s][%d] ERROR \n", __FUNCTION__, __LINE__);

	return PCIBIOS_SUCCESSFUL;
}

static int bcm947xx_pcie_config_read(struct pci_bus *bus, unsigned int devfn,
	int where, int size, u32 *val)
{
	void __iomem *base;
	u32 data_reg;
	int slotno = PCI_SLOT(devfn);

	if (bus->number > BCM947XX_MAX_BUSNUM) {
		*val = ~0UL;
		return PCIBIOS_SUCCESSFUL;
	}

	if ((bus->number == 0) &&
	    ((slotno == D11_CORE0_IDX) || (slotno == D11_CORE1_IDX))) {
	    return bcm947xx_si_pcid_read_config(bus, devfn, where, size, val);
	}

	base = bcm947xx_pcie_cfg_base(bus, devfn, where);
	if (base == NULL) {
		*val = ~0UL;
		return PCIBIOS_SUCCESSFUL;
	}

	data_reg = readl(base);

	/* bcm947189: CLASS field is R/O, and set to wrong 0x200 value */
	if (bus->number == 0 && devfn == 0) {
		if ((where & 0xffc) == PCI_CLASS_REVISION) {
		/*
		 * RC's class is 0x0280, but Linux PCI driver needs 0x6040
		 * for a PCIe bridge. So we must fixup the class code
		 * to 0x60400 here.
		 */
			data_reg &= 0xff;
			data_reg |= 0x60400 << 8;
		}
	}

	if (size == 4) {
		*val = data_reg;
	} else if (size < 4) {
		u32 mask = (1 << (size * 8)) - 1;
		int shift = (where % 4) * 8;
		*val = (data_reg >> shift) & mask;
	}

	return PCIBIOS_SUCCESSFUL;
}

static int bcm947xx_pcie_config_write(struct pci_bus *bus, unsigned int devfn,
                                    int where, int size, u32 val)
{
	void __iomem *base;
	u32 data_reg;
	int slotno = PCI_SLOT(devfn);

	if (bus->number > BCM947XX_MAX_BUSNUM)
		return PCIBIOS_SUCCESSFUL;

	if ((bus->number == 0) &&
	    ((slotno == D11_CORE0_IDX) || (slotno == D11_CORE1_IDX))) {
	    return bcm947xx_si_pcid_write_config(bus, devfn, where, size, &val);
	}

	base = bcm947xx_pcie_cfg_base(bus, devfn, where);
	if (base == NULL) {
		return PCIBIOS_SUCCESSFUL;
	}

	if (size < 4) {
		u32 mask = (1 << (size * 8)) - 1;
		int shift = (where % 4) * 8;
		data_reg = readl(base);
		data_reg &= ~(mask << shift);
		data_reg |= (val & mask) << shift;
	} else {
		data_reg = val;
	}

	writel(data_reg, base);

	return PCIBIOS_SUCCESSFUL;
}


/*
  *
  * Function bcm947xx_pcie_map_irq (pcidev, slot, pin)
  *
  *
  *   Parameters:
  *    pcidev ... pointer to pci device data structure
  *    slot ... pci slot (not used)
  *    pin ... pin number (not used)
  *
  *   Description:
  * 	  Get the pcie core irq number.
  *
  *
  *  Return: pcie core irq number
  */
static int bcm947xx_pcie_map_irq(const struct pci_dev *pcidev,
	u8 slot,u8 pin)
{
	struct bcm947xx_pcie_hcd *pdrv = pcidev->bus->sysdata;

	if (pdrv->pdev->dev.of_node)
	    return of_irq_parse_and_map_pci(pcidev, slot, pin);
	else {
		uint32 irq;
		if (pcidev->device == 0x43c8)
			irq = (slot == D11_CORE0_IDX) ? INTERRUPT_ID_D11_0 : INTERRUPT_ID_D11_1;
		else
			irq = pdrv->resources.irq;

		return irq;
	}
}

/*
  *
  * Function bcm947xx_pcie_fixup_final_mps (dev)
  *
  *
  *   Parameters:
  *    dev ... pointer to pci device data structure
  *
  *   Description:
  * 	  Re-sync the MPS setting on all connected devices on the bus
  *     as Linux PCI bus driver does not sync the MPS setting during
  *     enumberation.
  *     fixup until it is implemented in Linux PCI bus driverr
  *
  *
  *  Return: None
  */
static void bcm947xx_pcie_fixup_final_mps(struct pci_dev *dev)
{
	/* sync-up mps. Changes allowed only in safe state */
	pcie_bus_config = PCIE_BUS_SAFE;
	if (dev->bus && dev->bus->self) {
	    pcie_bus_configure_settings(dev->bus);
	}
	return;
}
DECLARE_PCI_FIXUP_FINAL(PCI_VENDOR_ID_BROADCOM, PCI_ANY_ID, bcm947xx_pcie_fixup_final_mps);

/*
  *
  * Function bcm947xx_pcie_core_config (pdrv)
  *
  *
  *   Parameters:
  *    pdrv ... pointer to pcie core hcd data structure
  *
  *   Description:
  * 	  Setup pcie core legacy interrupts, outgoing memory window, bar1, pci class, UBUS
  *
  *  Return: 0 on success, -ve on failure
  */
static int bcm947xx_pcie_core_config(struct bcm947xx_pcie_hcd *pdrv)
{
	HCD_FN_ENT();

	HCD_FN_EXT();

	return 0;
}


static int __init allow_gen2_rc(struct bcm947xx_pcie_hcd *pdrv)
{
	uint32 vendorid, devid;
	uint32 val;
	int allow = 1;

	/* Read PCI vendor/device ID's */
	writel(0x0, pdrv->base + PCIE_GEN2_PCIE_CFG_ADDR);
	val = readl(pdrv->base + PCIE_GEN2_PCIE_CFG_DATA);
	vendorid = val & 0xffff;
	devid = val >> 16;

	if (vendorid == VENDOR_BROADCOM &&
	     (devid == BCM43217_CHIP_ID || devid == BCM43227_CHIP_ID)) {
		/* Only support GEN1 */
		return 0;
	}

	return (allow);
}

/* Init PCIE RC host bridge */
static void __init noinline bcm947xx_pcie_bridge_init(struct bcm947xx_pcie_hcd *pdrv)
{
	u32 devfn = 0;
	u8 tmp8;
	u16 tmp16;

	/* Fake <bus> object */
	struct pci_bus bus = {
		.number = 0,
		.ops = &bcm947xx_pcie_ops ,
		.sysdata = pdrv,
	};

	pci_bus_write_config_byte(&bus, devfn, PCI_PRIMARY_BUS, 0);
	pci_bus_write_config_byte(&bus, devfn, PCI_SECONDARY_BUS, 1);
	pci_bus_write_config_byte(&bus, devfn, PCI_SUBORDINATE_BUS, 4);

	pci_bus_read_config_byte(&bus, devfn, PCI_PRIMARY_BUS, &tmp8);
	pci_bus_read_config_byte(&bus, devfn, PCI_SECONDARY_BUS, &tmp8);
	pci_bus_read_config_byte(&bus, devfn, PCI_SUBORDINATE_BUS, &tmp8);

	/* MEM_BASE, MEM_LIM require 1MB alignment */
	BUG_ON((pdrv->resources.owin.start >> 16) & 0xf);

	HCD_INFO("PCIE%d: membase 0x%llx memlimit 0x%llx\n", pdrv->core_id,
		(u64)pdrv->resources.owin.start, (u64)pdrv->resources.owin.end);

	pci_bus_write_config_word(&bus, devfn, PCI_MEMORY_BASE,
		(pdrv->resources.owin.start >> 16) & 0xfff0);

	BUG_ON(((pdrv->resources.owin.end + 1) >> 16) & 0xf);
	pci_bus_write_config_word(&bus, devfn, PCI_MEMORY_LIMIT,
		(pdrv->resources.owin.end >> 16) & 0xfff0);

	/* These registers are not supported on the NS */
	pci_bus_write_config_word(&bus, devfn, PCI_IO_BASE_UPPER16, 0);
	pci_bus_write_config_word(&bus, devfn, PCI_IO_LIMIT_UPPER16, 0);

	/* Force class to that of a Bridge */
	pci_bus_write_config_word(&bus, devfn, PCI_CLASS_DEVICE, PCI_CLASS_BRIDGE_PCI);

	pci_bus_read_config_word(&bus, devfn, PCI_CLASS_DEVICE, &tmp16);
	pci_bus_read_config_word(&bus, devfn, PCI_MEMORY_BASE, &tmp16);
	pci_bus_read_config_word(&bus, devfn, PCI_MEMORY_LIMIT, &tmp16);
}

/*
  *
  * Function bcm947xx_pcie_check_link(pdrv)
  *
  *
  *   Parameters:
  *    pdrv ... pointer to pcie core hcd data structure
  *
  *   Description:
  *       Check link status, return 0 if link is up in RC mode,
  *       otherwise return non-zero
  *
  *  Return: 0 on success, -ve on failure
  */
static int __init noinline bcm947xx_pcie_check_link(struct bcm947xx_pcie_hcd *pdrv, uint32 allow_gen2)
{
	u32 devfn = 0;
	u16 pos, tmp16, link = 0;
	u8 nlw, tmp8;
	u32 tmp32;
	int wait = 0;

	/* Fake <bus> object */
	struct pci_bus bus = {
		.number = 0,
		.ops = &bcm947xx_pcie_ops ,
		.sysdata = pdrv,
	};

	pci_bus_read_config_dword(&bus, devfn, 0xdc, &tmp32);
	tmp32 &= ~0xf;
	if (allow_gen2)
		tmp32 |= 2;
	else {
		/* force PCIE GEN1 */
		tmp32 |= 1;
	}
	pci_bus_write_config_dword(&bus, devfn, 0xdc, tmp32);

	/* Retrain link */
	pos = pci_bus_find_capability(&bus, devfn, PCI_CAP_ID_EXP);
	pci_bus_read_config_word(&bus, devfn, pos + PCI_EXP_LNKCTL, &tmp16);
	tmp16 |= PCI_EXP_LNKCTL_RL;
	pci_bus_write_config_word(&bus, devfn, pos + PCI_EXP_LNKCTL, tmp16);

	/* Wait for link training */
	do {
		pci_bus_read_config_word(&bus, devfn, pos + PCI_EXP_LNKSTA, &tmp16);
		if (!(tmp16 & PCI_EXP_LNKSTA_LT))
			break;
		mdelay(100);
	} while (wait++ < 10);

	if (tmp16 & PCI_EXP_LNKSTA_LT)
		HCD_INFO("PCIE [%d]: Retrain link failed\n", pdrv->core_id);

	/* See if the port is in EP mode, indicated by header type 00 */
	pci_bus_read_config_byte(&bus, devfn, PCI_HEADER_TYPE, &tmp8);
	if (tmp8 != PCI_HEADER_TYPE_BRIDGE) {
		HCD_INFO("PCIe core %d in End-Point mode - ignored\n", pdrv->core_id);
		return -ENODEV;
	}

	/* bcm947189 PAX only changes NLW field when card is present */
	pos = pci_bus_find_capability(&bus, devfn, PCI_CAP_ID_EXP);
	pci_bus_read_config_word(&bus, devfn, pos + PCI_EXP_LNKSTA, &tmp16);


	HCD_INFO("PCIE%d: LINKSTA reg %#x val %#x\n", pdrv->core_id,
		pos + PCI_EXP_LNKSTA, tmp16);

	nlw = (tmp16 & PCI_EXP_LNKSTA_NLW) >> PCI_EXP_LNKSTA_NLW_SHIFT;
	link = tmp16 & PCI_EXP_LNKSTA_DLLLA;

	if (nlw != 0)
		link = 1;

#ifdef	DEBUG
	for (; pos < 0x100; pos += 2) {
		pci_bus_read_config_word(&bus, devfn, pos, &tmp16);
		if (tmp16)
			HCD_INFO("reg[%#x]=%#x, ", pos, tmp16);
	}
#endif
	HCD_INFO("PCIE%d link=%d\n", pdrv->core_id, link);

	return (link ? 0 : -ENOSYS);
}

 
/*
  *
  * Function bcm947xx_pcie_map_init(pdrv)
  *
  *
  *   Parameters:
  *    pdrv ... pointer to pcie core hcd data structure
  *
  *   Description:
  * 	  init PCIE HCD map
  *
  *  Return: 0 on success, -ve on failure
  */
static void __init bcm947xx_pcie_map_init(struct bcm947xx_pcie_hcd *pdrv)
{
	unsigned size, i;
	u32 addr;

	/*
	 * NOTE:
	 * All PCI-to-CPU address mapping are 1:1 for simplicity
	 */

	/* Outbound address translation setup */
	size = resource_size(&pdrv->resources.owin);
	addr = pdrv->resources.owin.start;
	BUG_ON(!addr);
	BUG_ON(addr & ((1 << 25) - 1));	/* 64MB alignment */

	for (i = 0; i < 3; i++) {
		const unsigned win_size = SZ_64M;
		/* 64-bit LE regs, write low word, high is 0 at reset */
		writel(addr, pdrv->base + PCIE_GEN2_OMAP(i));
		writel(addr|0x1, pdrv->base + PCIE_GEN2_OARR(i));
		addr += win_size;
		if (size >= win_size)
			size -= win_size;
		if (size == 0)
			break;
	}
	WARN_ON(size > 0);

	size = min(getMemorySize(), (ulong)SZ_128M);
	addr = PHYS_OFFSET;

	size >>= 20;	/* In MB */
	size &= 0xff;	/* Size is an 8-bit field */

	WARN_ON(size == 0);

	/* 64-bit LE regs, write low word, high is 0 at reset */
	writel(addr | 0x1, pdrv->base + PCIE_GEN2_IMAP1(0));
	writel(addr | size, pdrv->base + PCIE_GEN2_IARR(1));
	if (getMemorySize() <= SZ_128M) {
		return;
	}
	
	/* DDR memory size > 128MB */
	addr = PHYS_OFFSET + SZ_128M;

	size = min(getMemorySize() - SZ_128M, (ulong)SZ_128M);
	size >>= 20;	/* In MB */
	size &= 0xff;	/* Size is an 8-bit field */

	writel(addr | 0x1, pdrv->base + PCIE_GEN2_IMAP2(0));
	writel(addr | size, pdrv->base + PCIE_GEN2_IARR(2));
}

/*
  *
  * Function bcm947xx_pcie_hw_init(pdrv)
  *
  *
  *   Parameters:
  *    pdrv ... pointer to pcie core hcd data structure
  *
  *   Description:
  * 	  Check if PCIe link up or not 
  *
  *  Return: 0 on success, -ve on failure
  */
static bool __init bcm947xx_pcie_hw_init(struct bcm947xx_pcie_hcd *pdrv)
{
	u32 devfn = 0;
	u32 tmp32;
	u16 tmp16;

	/* Fake <bus> object */
	struct pci_bus bus = {
		.number = 0,
		.ops = &bcm947xx_pcie_ops ,
		.sysdata = pdrv,
	};

	HCD_FN_ENT();

	/* Change MPS and MRRS to 512 */
	pci_bus_read_config_word(&bus, devfn, 0x4d4, &tmp16);
	tmp16 &= ~7;
	tmp16 |= 2;
	pci_bus_write_config_word(&bus, devfn, 0x4d4, tmp16);

	pci_bus_read_config_dword(&bus, devfn, 0xb4, &tmp32);
	tmp32 &= ~((7 << 12) | (7 << 5));
	tmp32 |= (2 << 12) | (2 << 5);
	pci_bus_write_config_dword(&bus, devfn, 0xb4, tmp32);

	/* Turn-on Root-Complex (RC) mode, from reset default of EP */

	/* The mode is set by straps, can be overwritten via DMU
	   register <cru_straps_control> bit 5, "1" means RC
	 */

	/* Send a downstream reset */
	writel(0x3, pdrv->base + PCIE_GEN2_CLK_CONTROL);
	udelay(250);
	writel(0x1, pdrv->base + PCIE_GEN2_CLK_CONTROL);
	mdelay(250);

	HCD_FN_EXT();

	return TRUE;
}

/*
  *
  * Function bcm947xx_pcie_is_pcie_link_up (pdrv)
  *
  *
  *   Parameters:
  *    pdrv ... pointer to pcie core hcd data structure
  *
  *   Description:
  * 	  Check if PCIe link up or not 
  *
  *  Return: 0 on success, -ve on failure
  */
static bool __init bcm947xx_pcie_is_pcie_link_up(struct bcm947xx_pcie_hcd *pdrv)
{
	int allow_gen2, linkfail;

	HCD_FN_ENT();
	for (allow_gen2 = 0; allow_gen2 <= 1; allow_gen2++) {
		bcm947xx_pcie_hw_init(pdrv);
		bcm947xx_pcie_map_init(pdrv);
		/*
		 * Skip inactive ports -
		 * will need to change this for hot-plugging
		 */
		linkfail = bcm947xx_pcie_check_link(pdrv, allow_gen2);
		if (linkfail)
			break;
		bcm947xx_pcie_bridge_init(pdrv);

		if (allow_gen2 == 0) {
			if (allow_gen2_rc(pdrv) == 0)
				break;
			HCD_INFO("PCIE%d switching to GEN2\n", pdrv->core_id);
		}
	}

	HCD_FN_EXT();
	return (linkfail) ? FALSE: TRUE;
}

/*
  *
  * Function bcm947xx_pcie_core_reset (pdrv)
  *
  *
  *   Parameters:
  *    pdrv ... pointer to pcie core hcd data structure
  *
  *   Description:
  * 	  Reset PCIe core using misc driver API's. Configure the phy parameters
  *     while the core is in reset.
  *
  *  Return: 0 on success, -ve on failure
  */
static void bcm947xx_pcie_core_reset(struct bcm947xx_pcie_hcd *pdrv)
{
	uint32 clk_control;

	HCD_FN_ENT();

	/* this is a critical delay */
	mdelay(500);

	clk_control = readl(pdrv->base + PCIE_GEN2_CLK_CONTROL);

	/* clk_control bit 0 is PCIE_RC_PCIE_RESET. If Still under
	 * reset state, we need to trigger a pulse to reset device
	 */
	if (clk_control & 0x1) {
		clk_control &= ~0x1;
		writel(clk_control, pdrv->base + PCIE_GEN2_CLK_CONTROL);
		mdelay(100);
		/* read back to take effect */
		clk_control = readl(PCIEGEN2_BASE + PCIE_GEN2_CLK_CONTROL);
	}

	HCD_FN_EXT();
	return;
}




/*
  *
  * Function bcm947xx_pcie_setup_regs (pdrv)
  *
  *
  *   Parameters:
  *    pdrv ... pointer to pcie core hcd data structure
  *
  *   Description:
  * 	  Map  pcie core registers resource
  *
  *  Return: 0 on success, -ve on failure
  */
static int bcm947xx_pcie_setup_regs(struct bcm947xx_pcie_hcd *pdrv)
{

	struct bcm947xx_pcie_hc_res *pres = NULL;
	struct platform_device *pdev = NULL;
	struct device *dev;
	uint32 reg_data;

	HCD_FN_ENT();

	pdev = pdrv->pdev;
	dev = &pdev->dev;
	pres = &pdrv->resources;

	pdrv->base = devm_ioremap_resource(dev, &pres->base);
	if (IS_ERR(pdrv->base)) {
	    HCD_ERROR("pcie core [%d] reg base mapping fail: [%ld]\r\n",
	        pdrv->core_id, PTR_ERR(pdrv->base));
	    return PTR_ERR(pdrv->base);
	}

	HCD_INFO("pcie core [%d] mapped reg base [0x%p]\r\n", pdev->id,
	    pdrv->base);

	reg_data = hcd_readl(pdrv->base, RC_CFG_TYPE1_DEVICE_VENDOR_ID_OFFSET);
	HCD_INFO("[%d] Vendor [0x%4x] device [0x%4x]\r\n",
	    pdev->id, (reg_data&0xFFFF), ((reg_data >> 16)&0xFFFF));

	reg_data = hcd_readl(pdrv->base, RC_CFG_TYPE1_REV_ID_CLASS_CODE_OFFSET);
	/*
	 * RC's class is 0x0280, but Linux PCI driver needs 0x60400
	 * for a PCIe bridge. So we must fixup the class code
	 * to 0x60400 here.
	 */
	reg_data &= 0xff;
	reg_data |= 0x60400 << 8;
	HCD_INFO("[%d] Rev [0x%2x] Class [0x%6x]\r\n",
	    pdev->id, (reg_data&0xFF), ((reg_data >> 8)&0xFFFFFF));

	pdrv->core_rev = reg_data & 0xFF;

	HCD_LOG("found core [%d] Rev [%2x.%2x]\r\n", pdev->id,
	    ((pdrv->core_rev >> 8)&0xFF), (pdrv->core_rev&0xFF));

	HCD_FN_EXT();

	return 0;
}


/*
  *
  * Function bcm947xx_pcie_setup_owin (pdrv, resources)
  *
  *
  *   Parameters:
  *	pdrv ... pointer to pcie core hcd data structure
  *   resources ... pcie core resources
  *
  *   Description:
  *	   Map window memory resources
  *
  *  Return: 0 on success, -ve on failure
  */
static int bcm947xx_pcie_setup_owin(struct bcm947xx_pcie_hcd *pdrv,
	struct list_head *resources)
{
	struct bcm947xx_pcie_hc_res *pres = NULL;
	struct platform_device *pdev = NULL;
	struct device *dev;
	int err = 0;

	HCD_FN_ENT();

	pdev = pdrv->pdev;
	dev = &pdev->dev;
	pres = &pdrv->resources;

	err =  devm_request_resource(dev, &iomem_resource, &pres->owin);
	if (err) {
	    HCD_ERROR("[%d] pcie failed to create own resource: [%d]\r\n",
	        pdev->id, err);

	    HCD_FN_EXT();
	    return err;
	}
	else
	    HCD_INFO("[%d] mapped pcie owin base [0x%llx]\r\n", pdev->id,
	        (u64)pres->owin.start);


	pci_add_resource_offset(resources,
	                &pdrv->resources.owin,
	                (pdrv->resources.owin.start - pdrv->resources.pci_addr));

	pdrv->owin_inited = TRUE;

	HCD_FN_EXT();

	return err;
}

/*
  *
  * Function bcm947xx_pcie_unmap_res (pdrv)
  *
  *
  *   Parameters:
  *	pdrv ... pointer to pcie core hcd data structure
  *
  *   Description:
  *	   Unmap previous allocated resources window memory and register base resources
  *
  *  Return: 0 on success, -ve on failure
  */
static void bcm947xx_pcie_unmap_res(struct bcm947xx_pcie_hcd *pdrv)
{
	struct bcm947xx_pcie_hc_res *pres = NULL;
	struct platform_device *pdev = NULL;
	struct device *dev;

	HCD_FN_ENT();

	if (pdrv == NULL)
	    return;

	pdev = pdrv->pdev;
	dev = &pdev->dev;
	pres = &pdrv->resources;

	if (pdrv->owin_inited == TRUE) {
	    devm_release_resource(dev, &pres->owin);
	    HCD_INFO("release owin [0x%llx]\r\n", (u64)pres->owin.start);
	    pdrv->owin_inited = FALSE;
	}
	if (pdrv->base) {
	    devm_iounmap(dev, pdrv->base);
	    HCD_INFO("unmap reg base [0x%p]\r\n",pdrv->base);
	    pdrv->base = NULL;
	}

	return;
}

/*
  *
  * Function bcm947xx_pcie_parse_dt (pdrv)
  *
  *
  *   Parameters:
  *	pdrv ... pointer to pcie core hcd data structure
  *
  *   Description:
  *	   Parse pcie core hcd device tree entries. Currently supported resources
  *       - PCIe core base, memory window, PCI bus range
  *
  *  Return: 0 on success, -ve on failure
  */
static int bcm947xx_pcie_parse_dt(struct bcm947xx_pcie_hcd *pdrv)
{

	struct bcm947xx_pcie_hc_res *pres = NULL;
	struct platform_device *pdev = NULL;
	struct device_node *np = NULL;
	struct device *dev = NULL;
	int err = 0;

	HCD_FN_ENT();

	pdev = pdrv->pdev;
	pres = &pdrv->resources;

	/* Initialize attributes from device tree if present */
	np = pdev->dev.of_node;
	dev = &pdev->dev;
	if (np) {
	    HCD_INFO("[%d] DT node available: %s\n", pdev->id, np->full_name);

	    /* Check if DT entry status is enabled or okay to load */
	    if (!of_device_is_available(np)) {
	        dev_err(dev, "DT status disabled\n");
	        return -ENODEV;
	    }

	    /* PCIe core registers base */
	    err = of_address_to_resource(np, 0, &pres->base);
	    HCD_FAIL_ON_DT_ERROR("base registers", err);

	    /* PCIe Mems base */
	    err = of_address_to_resource(np, 1, &pres->owin);
	    HCD_FAIL_ON_DT_ERROR("Window memory", err);

	    /* PCI bus range */
	    err = of_pci_parse_bus_range(np, &pres->bus_range);
	    HCD_FAIL_ON_DT_ERROR("busnumber", err);
	    if (pres->bus_range.end > BCM947XX_MAX_BUSNUM)
	        pres->bus_range.end = BCM947XX_MAX_BUSNUM;
	}

	HCD_INFO("[%d] regs: start [0x%llx] end [0x%llx] flags [0x%lx]\r\n",
	    pdev->id, (u64)pres->base.start, (u64)pres->base.end,
	    pres->base.flags);
	HCD_INFO("[%d] bus_range: start [0x%llx] end [0x%llx] flags [0x%lx]\r\n",
	    pdev->id, (u64)pres->bus_range.start, (u64)pres->bus_range.end,
	    pres->bus_range.flags);
	HCD_INFO("[%d] owin: start [0x%llx] end [0x%llx] flags [0x%lx]\r\n",
	    pdev->id, (u64)pres->owin.start, (u64)pres->owin.end,
	    pres->owin.flags);
	HCD_INFO("[%d] pci_addr: [0x%llx]\r\n", pdev->id, pres->pci_addr);

	HCD_FN_EXT();

	return 0;
}


/*
  *
  * Function bcm947xx_pcie_init_res (pdrv)
  *
  *
  *   Parameters:
  *	pdrv ... pointer to pcie core hcd data structure
  *
  *   Description:
  *	   Initialize the HCD resource entries to default values. Currently supported resources
  *      - PCIe core base, memory window, PCI bus range
  *
  *  Return: 0 on success, -ve on failure
  */
static int bcm947xx_pcie_init_res(struct bcm947xx_pcie_hcd *pdrv)
{

	struct bcm947xx_pcie_hc_res *pres = NULL;
	struct platform_device *pdev = NULL;

	HCD_FN_ENT();

	pdev = pdrv->pdev;
	pres = &pdrv->resources;

	/* Initialize attributes with default values */
	pres->base.start = PCIE_GEN2_PHYS_BASE;
	pres->owin.start = PCIE_GEN2_OWIN_PHYS_BASE;
	pres->irq = INTERRUPT_ID_PCIE_GEN2;
	pres->base.end = pres->base.start + SI_CORE_SIZE - 1;
	pres->base.flags = IORESOURCE_MEM;

	pres->owin.name = "PCIe Outbound Window, Port 0";
	pres->owin.end = pres->owin.start + PCIE_GEN2_OWIN_PHYS_SIZE - 1;
	pres->owin.flags = IORESOURCE_MEM;

	pres->bus_range.start = BCM947XX_ROOT_BUSNUM;
	pres->bus_range.end = BCM947XX_MAX_BUSNUM;
	pres->bus_range.flags = IORESOURCE_BUS;

	pres->pci_addr = pres->owin.start;

	pres->domain = pdrv->core_id;

	/* PCIe port configuration */
	bcm947xx_pcie_hcd_init_hc_cfg(pdrv);

	HCD_INFO("[%d] regs: start [0x%llx] end [0x%llx] flags [0x%lx]\r\n",
	    pdev->id, (u64)pres->base.start, (u64)pres->base.end,
	    pres->base.flags);
	HCD_INFO("[%d] bus_range: start [0x%llx] end [0x%llx] flags [0x%lx]\r\n",
	    pdev->id, (u64)pres->bus_range.start, (u64)pres->bus_range.end,
	    pres->bus_range.flags);
	HCD_INFO("[%d] owin: start [0x%llx] end [0x%llx] flags [0x%lx]\r\n",
	    pdev->id, (u64)pres->owin.start, (u64)pres->owin.end,
	    pres->owin.flags);
	HCD_INFO("[%d] pci_addr: [0x%llx]\r\n",pdev->id, pres->pci_addr);

	HCD_FN_EXT();

	return 0;
}



/*
  *
  * Function bcm947xx_pcie_probe (pdrv)
  *
  *
  *   Parameters:
  * pdrv ... pointer to pcie core hcd data structure
  *
  *   Description:
  *    PCIe hcd driver probe. Called for each instance of the PCIe core.
  *    Get and allocate resource, configure hardware, start the PCI bus and
  *    enumerate PCI devices
  *
  *  Return: 0 on success, -ve on failure
  */
static int __init bcm947xx_pcie_probe(struct platform_device *pdev)
{
	struct bcm947xx_pcie_hcd *pdrv = NULL;
	int core = pdev->id;
	int	err = 0;
	struct pci_bus *bus;
	LIST_HEAD(res);

	HCD_FN_ENT();

	HCD_INFO("core [%d] probe\r\n",core);

	if (core >= NUM_CORE)
	    return -ENODEV;

	if (kerSysGetPciePortEnable(core)) {
	    HCD_INFO("core [%d] powered-up\n", core);

	    /* init D11 cores that needs to be enumerated through PCIE */
	    bcm947xx_hndpci_init();

	    /* Allocate HCD control block */
	    pdrv = kzalloc(sizeof(*pdrv), GFP_KERNEL);
	    if (!pdrv) {
	        HCD_ERROR("[%d] Unable to allocate memory for CB\r\n", core);
	        err =  -ENOMEM;
	        goto error;
	    }
	    HCD_INFO("[%d] Allocated [0x%p] hcd\r\n",core, pdrv);

	    /* Initialize  hcd elements */
	    pdrv->core_id = core;
	    pdrv->pdev = pdev;
	    platform_set_drvdata(pdev, pdrv);

	    /* Initialize  core resource element values for no device tree based
	         * legacy drivers
	         */
	    bcm947xx_pcie_init_res(pdrv);

	    /* Update core resource elements  (DT based) */
	    err = bcm947xx_pcie_parse_dt(pdrv);
	    if (err) {
	        HCD_ERROR("failed to update core [%d] dt entries\r\n", core);
	        err =  -EINVAL;
	        goto error;
	    }

	    /* setup pcie Core registers for access to PCIe core */
	    err = bcm947xx_pcie_setup_regs(pdrv);
	    if (err) {
	        HCD_ERROR("failed to setup core[%d] regs, err [%d]\r\n", core,
	            err);
	        err =  -ENOMEM;
	        goto error;
	    }

	    /* lets talk to PCIe core, reset the core */
	    bcm947xx_pcie_core_reset(pdrv);

	    /* Check if PCIe link is up (for any device connected on the link) */
	    if (!(bcm947xx_pcie_is_pcie_link_up(pdrv))) {
	        /* No device connected to PCIe core */
	        HCD_ERROR("failed to bring up core [%d] link\r\n",core);
	        err =  -ENODEV;
	        goto error;
	    }
	    HCD_INFO("core [%d] Link UP !!!!!!!\r\n",core);

	    /* Setup PCIe core memory window */
	    err = bcm947xx_pcie_setup_owin(pdrv, &res);
	    if (err) {
	        HCD_ERROR("core [%d] failed to setup owin resource, err [%d]\r\n",
	            core, err);
	        err =  -ENOMEM;
	        goto error;
	    }

	    /* Setup PCIe core bus numbers */
	    pci_add_resource(&res, &pdrv->resources.bus_range);

	    /* Got all driver resources. Now configure the PCIe core */
	    err = bcm947xx_pcie_core_config(pdrv);
	    if (err) {
	        HCD_ERROR("core [%d] failed to setup hw, err [%d]\r\n", core, err);
	        err =  -ENODEV;
	        goto error;
	    }

	    /* Now do the PCI setup,
	         * - create,scan bus
	         * - assign resources, irq
	         * - add connected devices
	         */
	    bus = pci_scan_root_bus(&pdev->dev, BCM947XX_ROOT_BUSNUM,
	        &bcm947xx_pcie_ops, pdrv, &res);
	    if (!bus) {
	        HCD_ERROR("core [%d] failed to setup hw: %d\r\n", core, err);
	        err =  -ENXIO;
	        goto error;
	    }

	    pci_assign_unassigned_bus_resources(bus);
	    pci_fixup_irqs(pci_common_swizzle, bcm947xx_pcie_map_irq);
	    pci_bus_add_devices(bus);

	    /* Setup virtual-wire interrupts */
	    writel(0xf, pdrv->base + PCIE_GEN2_PCIE_SYS_RC_INTX_EN);
	    /* Enable memory and bus master */
	    writel(0x6, pdrv->base + PCIE_GEN2_PCIE_HDR_OFF + 4);

	    err = 0;
	} else {
	    HCD_ERROR("core [%d] disabled\n", core);
	    err = -ENODEV;
	}

error:

	if (err) {
	    bcm947xx_pcie_unmap_res(pdrv);
	    kfree(pdrv);
	    if (kerSysGetPciePortEnable(core)) {
	        HCD_INFO("core [%d] powered-down\n", core);
	    }
	}

	pci_free_resource_list(&res);

	HCD_FN_EXT();

	return err;
}


/*
  *
  * Function bcm947xx_pcie_remove (pdrv)
  *
  *
  *   Parameters:
  * pdrv ... pointer to pcie core hcd data structure
  *
  *   Description:
  *    PCIe hcd driver remove Free the resources and power down the PCIe core
  *
  *  Return: 0 on success, -ve on failure
  */
static int bcm947xx_pcie_remove(struct platform_device *pdev)
{
	struct bcm947xx_pcie_hcd *pdrv = platform_get_drvdata(pdev);
	int core = pdev->id;

	HCD_FN_ENT();

	if (!pdrv)
	    return 0;


	if (kerSysGetPciePortEnable(core)) {
	    bcm947xx_pcie_unmap_res(pdrv);
	    kfree(pdrv);
	}

	HCD_FN_EXT();

	return 0;
}


/**************************************
 *  Global Functions
 **************************************/
/*
  *
  * Function bcm947xx_pcie_init ()
  *
  *
  *   Parameters:
  *
  *   Description:
  *    PCIe hcd driver init, register the driver from platform list including misc driver
  *    This inturn should call our probe()
  *
  *  Return: 0 on success, -ve on failure
  */
static int __init bcm947xx_pcie_init(void)
{
	int ret;

	HCD_FN_ENT();

	printk("PCIe HCD (impl%d)\r\n",CONFIG_BCM_PCIE_HCD_IMPL);

	ret = platform_driver_register(&bcm947xx_pcie_driver);

	HCD_FN_EXT();

	return ret;
}

/*
  *
  * Function bcm947xx_pcie_init ()
  *
  *
  *   Parameters:
  *
  *   Description:
  *    PCIe hcd driver exit, unregister the driver from platform list including misc driver
  *    This inturn should call our remove()
  *
  *  Return: None
  */
static void bcm947xx_pcie_exit(void)
{
	HCD_FN_ENT();

	platform_driver_unregister(&bcm947xx_pcie_driver);

	HCD_FN_EXT();

	return;
}

module_init(bcm947xx_pcie_init);
module_exit(bcm947xx_pcie_exit);

/*
  *
  * Function bcm947xx_pcie_plt_init ()
  *
  *
  *   Parameters:
  *
  *   Description:
  *    PCIe platform setup. Add PCIe cores to the platform devices
  *    Generally this is done outside, but since there is no support outside,
  *    it is done here.
  *
  *  Return: 0 on success, -ve value on failure
  */
static int __init bcm947xx_pcie_plt_init(void)
{
	int core;
	int ret = 0;
	struct platform_device	*pdev = NULL;

	HCD_FN_ENT();

	/* Register All Pcie cores as platform devices */
	for (core = 0; core < NUM_CORE; core++) {
	    pdev = &bcm947xx_pcie_plt_dev[core];
	    pdev->name = BCM947XX_PCIE_DEV_NAME;
	    pdev->id = core;

	    ret = platform_device_register(pdev);
	    if (ret) {
	        HCD_ERROR("unable to register PCIe core [%d] device: ret = %d\n",
	            core, ret);
	    } else {
	        HCD_INFO("registered PCIe core [%d] device successfully\r\n",core);
	    }
	}

	HCD_FN_EXT();

	return ret;
}
subsys_initcall(bcm947xx_pcie_plt_init);
#endif /* CONFIG_BCM_KF_ARM_BCM963XX */
