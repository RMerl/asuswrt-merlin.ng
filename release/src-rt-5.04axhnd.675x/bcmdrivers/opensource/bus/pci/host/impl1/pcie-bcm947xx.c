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
#include <bcm_intr.h>

#include <pcie_hcd.h>
#include <pcie-bcm947xx.h>

/*
 * +-----------------------------------------------------
 *  Defines
 * +-----------------------------------------------------
 */
/* HCD Driver */
#define BCM947XX_ROOT_BUSNUM                0x00
#define BCM947XX_MAX_BUSNUM                 0x01

/*
 * +-----------------------------------------------------
 *  Macros
 * +-----------------------------------------------------
 */

/*
 * +-----------------------------------------------------
 *  Structures
 * +-----------------------------------------------------
 */

/*
 * +-----------------------------------------------------
 *  Local Function prototype
 * +-----------------------------------------------------
 */
/* Internal WLAN Radio Virtul PCIe */
static void __init bcm947xx_hndpci_init(void);
static int bcm947xx_si_pcid_read_config(struct pci_bus *bus,
	unsigned int devfn, int off, int len, u32 *buf);
static int bcm947xx_si_pcid_write_config(struct pci_bus *bus,
	unsigned int devfn, int off, int len, u32 *buf);

/* Core Access */
static int bcm947xx_hc_config_read(struct pci_bus *bus, unsigned int devfn,
                                   int where, int size, u32 *val);
static int bcm947xx_hc_config_write(struct pci_bus *bus, unsigned int devfn,
                                    int where, int size, u32 val);
static int bcm947xx_hc_map_irq(const struct pci_dev *pcidev, u8 slot,
	u8 pin);
static int __init allow_gen2_rc(struct pcie_hc_core *phc);

/* Core setup */
static void __init noinline bcm947xx_hc_bridge_init(struct pcie_hc_core *phc);
static int __init noinline bcm947xx_hc_check_link(struct pcie_hc_core *phc, uint32 allow_gen2);
static void __init bcm947xx_hc_map_init(struct pcie_hc_core *phc);
static bool __init bcm947xx_hc_hw_init(struct pcie_hc_core *phc);
static bool __init bcm947xx_hc_is_pcie_link_up(struct pcie_hc_core *phc);

static int bcm947xx_hc_core_config(struct pcie_hc_core *phc);
static int bcm947xx_hc_core_reset(struct pcie_hc_core *phc);

static int bcm947xx_hc_setup_rev(struct pcie_hc_core *phc);
static int bcm947xx_hc_init_res(struct pcie_hc_core *phc);

static int bcm947xx_hc_post_init(struct pcie_hc_core *phc);

static int bcm947xx_hc_probe(struct pcie_hc_core *phc);
static void bcm947xx_hc_remove(struct pcie_hc_core *phc);

/*
 * +-----------------------------------------------------
 *  external Function prototype
 * +-----------------------------------------------------
 */
extern unsigned long getMemorySize(void);

/*
 * +-----------------------------------------------------
 *  Global variables
 * +-----------------------------------------------------
 */
/* Global PCIE config space array for internal radio */
static si_pci_cfg_t si_pci_cfg[MAX_SOC_D11];

static struct pci_ops bcm947xx_hc_ops = {
	.read = bcm947xx_hc_config_read,
	.write = bcm947xx_hc_config_write,
};

/*
 * +-----------------------------------------------------
 *  Local inline functions
 * +-----------------------------------------------------
 */
/* read 32bit pcie register space */
static inline u32 hcd_readl(struct pcie_hc_core *phc, unsigned offset)
{
	writel(offset, phc->info.base + PCIE_GEN2_PCIE_EXT_CFG_ADDR);
	return readl(phc->info.base + PCIE_GEN2_PCIE_EXT_CFG_DATA);
}

/* write 32bit pcie register space */
static inline void hcd_writel(u32 data, struct pcie_hc_core *phc, unsigned offset)
{
	writel(offset, phc->info.base + PCIE_GEN2_PCIE_EXT_CFG_ADDR);
	writel(data, phc->info.base + PCIE_GEN2_PCIE_EXT_CFG_DATA);
}

/*
 * +-----------------------------------------------------
 *  Local Functions
 * +-----------------------------------------------------
 */
static void __iomem *bcm947xx_hc_cfg_base(struct pci_bus *bus, unsigned int devfn, int where)
{
	struct pcie_hc_core *phc = bus->sysdata;
	void __iomem *base;
	int offset;
	int busno = bus->number;
	int devno = PCI_SLOT(devfn);
	int fnno = PCI_FUNC(devfn);
	int type;
	u32 addr_reg;

	base = phc->info.base;

	/* If there is no link, just show the PCI bridge. */
	if (busno == 0) {
	    if (devno >= 1) {
	        return NULL;
	    }
	    writel(where & 0x1ffc, phc->info.base + PCIE_GEN2_PCIE_EXT_CFG_ADDR);
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

	    writel(addr_reg, phc->info.base + PCIE_GEN2_PCIE_CFG_ADDR);
	    offset = PCIE_GEN2_PCIE_CFG_DATA;
	}

	return base + offset;
}

#define sprom_control   dev_dep[0x88 - 0x40]

/*
 * Function bcm947xx_hndpci_init ()
 *
 *   Parameters:
 *
 *   Description:
 *    initialize 947xx internal WLAN core configuration
 *
 *   Return: None
 */
static void __init bcm947xx_hndpci_init(void)
{
	pci_config_regs *cfg;
	si_bar_cfg_t *bar;
	int i, j, d11idx;

	HCD_FN_ENT();

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

	    bar->n = 1;
	    bar->size[0] = SI_CORE_SIZE;

	    /* Using addrspace 0 only */
	    for (j = 1; j < PCI_BAR_MAX; j++) {
	        cfg->base[j] = 0;
	        bar->size[j] = 0;
	    }
	}

	HCD_FN_EXT();

	return;
}

/*
 * Function bcm947xx_si_pcid_read_config (bus, devfn where, size, val)
 *
 *   Parameters:
 *    bus   ... pointer to pcie bus
 *    devfn ... device and function number
 *    where ... offset from the base
 *    size  ... access size
 *    val   ... pointer to read value update location
 *
 *   Description:
 *    Read 947xx internal WLAN core configuration space
 *
 *   Return: 0 on success, -ve on failure
 */
static int
bcm947xx_si_pcid_read_config(struct pci_bus *bus, unsigned int devfn, int off, int len, u32 *buf)
{
	pci_config_regs *cfg;
	int slotno = PCI_SLOT(devfn);

	HCD_FN_ENT();

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

	HCD_FN_EXT();

	return PCIBIOS_SUCCESSFUL;
}

/*
 * Function bcm947xx_si_pcid_write_config (bus, devfn, where, size, val)
 *
 *   Parameters:
 *    bus   ... pointer to pcie bus
 *    devfn ... device and function number
 *    where ... offset from the base
 *    size  ... access size
 *    val   ... value to be written
 *
 *   Description:
 *    Write 947xx internal WLAN core device config space
 *
 *   Return: 0 on success, -ve on failure
 */
static int
bcm947xx_si_pcid_write_config(struct pci_bus *bus, unsigned int devfn, int off, int len, u32 *buf)
{
	pci_config_regs *cfg;
	si_bar_cfg_t *bar;
	int slotno = PCI_SLOT(devfn);

	HCD_FN_ENT();

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

	HCD_FN_EXT();

	return PCIBIOS_SUCCESSFUL;
}

/*
 * Function bcm947xx_hc_config_read (bus, devfn where, size, val)
 *
 *   Parameters:
 *    bus   ... pointer to pcie bus
 *    devfn ... device and function number
 *    where ... offset from the base
 *    size  ... access size
 *    val   ... pointer to read value update location
 *
 *   Description:
 *    Read 947xx PCIe HC core configuration space
 *
 *   Return: 0 on success, -ve on failure
 */
static int bcm947xx_hc_config_read(struct pci_bus *bus, unsigned int devfn,
	int where, int size, u32 *val)
{
	void __iomem *base;
	u32 data_reg;
	int slotno = PCI_SLOT(devfn);

	HCD_FN_ENT();

	if (bus->number > BCM947XX_MAX_BUSNUM) {
	    *val = ~0UL;
	    return PCIBIOS_SUCCESSFUL;
	}

	if ((bus->number == 0) &&
	    ((slotno == D11_CORE0_IDX) || (slotno == D11_CORE1_IDX))) {
	    return bcm947xx_si_pcid_read_config(bus, devfn, where, size, val);
	}

	base = bcm947xx_hc_cfg_base(bus, devfn, where);
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

	HCD_FN_EXT();

	return PCIBIOS_SUCCESSFUL;
}

/*
 * Function bcm947xx_hc_config_write (bus, devfn, where, size, val)
 *
 *   Parameters:
 *    bus   ... pointer to pcie bus
 *    devfn ... device and function number
 *    where ... offset from the base
 *    size  ... access size
 *    val   ... value to be written
 *
 *   Description:
 *    Write 947xx HC core device config space
 *
 *   Return: 0 on success, -ve on failure
 */
static int bcm947xx_hc_config_write(struct pci_bus *bus, unsigned int devfn,
                                    int where, int size, u32 val)
{
	void __iomem *base;
	u32 data_reg;
	int slotno = PCI_SLOT(devfn);

	HCD_FN_ENT();

	if (bus->number > BCM947XX_MAX_BUSNUM)
	    return PCIBIOS_SUCCESSFUL;

	if ((bus->number == 0) &&
	    ((slotno == D11_CORE0_IDX) || (slotno == D11_CORE1_IDX))) {
	    return bcm947xx_si_pcid_write_config(bus, devfn, where, size, &val);
	}

	base = bcm947xx_hc_cfg_base(bus, devfn, where);
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

	HCD_FN_EXT();

	return PCIBIOS_SUCCESSFUL;
}

/*
 * Function bcm947xx_hc_map_irq (pcidev, slot, pin)
 *
 *   Parameters:
 *    pcidev ... pointer to pci device data structure
 *    slot   ... pci slot (not used)
 *    pin    ... pin number (not used)
 *
 *   Description:
 *    Get the pcie core irq number.
 *
 *   Return: pcie core irq number
 */
static int bcm947xx_hc_map_irq(const struct pci_dev *pcidev,
	u8 slot, u8 pin)
{
	struct pcie_hc_core *phc = pcidev->bus->sysdata;
	uint32 irq;

	HCD_FN_ENT();

	if (pcidev->device == 0x43c8)
	    irq = (slot == D11_CORE0_IDX) ? INTERRUPT_ID_D11_0 : INTERRUPT_ID_D11_1;
	else
	    irq = phc->res.irq;

	HCD_FN_EXT();

	return irq;
}

/*
 * Function allow_gen2_rc(phc)
 *
 *   Parameters:
 *    phc ... pointer to pcie core hc data structure
 *
 *   Description:
 *    Check if GEN2 RC is allowed or not
 *
 *   Return: 1 on allowed, 0 on not allowed
 */
static int __init allow_gen2_rc(struct pcie_hc_core *phc)
{
	uint32 vendorid, devid;
	uint32 val;
	int allow = 1;

	HCD_FN_ENT();

	/* Read PCI vendor/device ID's */
	writel(0x0, phc->info.base + PCIE_GEN2_PCIE_CFG_ADDR);
	val = readl(phc->info.base + PCIE_GEN2_PCIE_CFG_DATA);
	vendorid = val & 0xffff;
	devid = val >> 16;

	if (vendorid == VENDOR_BROADCOM &&
	     (devid == BCM43217_CHIP_ID || devid == BCM43227_CHIP_ID)) {
	    /* Only support GEN1 */
	    allow = 0;
	}

	HCD_FN_EXT();

	return (allow);
}

/* Init PCIE RC host bridge */
/*
 * Function bcm947xx_hc_bridge_init(phc)
 *
 *   Parameters:
 *    phc ... pointer to pcie core hc data structure
 *
 *   Description:
 *    Initialize core pcie bridge
 *
 *   Return: None
 */
static void __init noinline bcm947xx_hc_bridge_init(struct pcie_hc_core *phc)
{
	u32 devfn = 0;
	u8 tmp8;
	u16 tmp16;

	/* Fake <bus> object */
	struct pci_bus bus = {
	    .number = 0,
	    .ops = &bcm947xx_hc_ops,
	    .sysdata = phc,
	};

	HCD_FN_ENT();

	pci_bus_write_config_byte(&bus, devfn, PCI_PRIMARY_BUS, 0);
	pci_bus_write_config_byte(&bus, devfn, PCI_SECONDARY_BUS, 1);
	pci_bus_write_config_byte(&bus, devfn, PCI_SUBORDINATE_BUS, 4);

	pci_bus_read_config_byte(&bus, devfn, PCI_PRIMARY_BUS, &tmp8);
	pci_bus_read_config_byte(&bus, devfn, PCI_SECONDARY_BUS, &tmp8);
	pci_bus_read_config_byte(&bus, devfn, PCI_SUBORDINATE_BUS, &tmp8);

	/* MEM_BASE, MEM_LIM require 1MB alignment */
	BUG_ON((phc->res.owin[OWIN0].start >> 16) & 0xf);

	HCD_INFO("PCIE%d: membase 0x%llx memlimit 0x%llx\n", phc->info.id,
	    (u64)phc->res.owin[OWIN0].start, (u64)phc->res.owin[OWIN0].end);

	pci_bus_write_config_word(&bus, devfn, PCI_MEMORY_BASE,
	    (phc->res.owin[OWIN0].start >> 16) & 0xfff0);

	BUG_ON(((phc->res.owin[OWIN0].end + 1) >> 16) & 0xf);
	pci_bus_write_config_word(&bus, devfn, PCI_MEMORY_LIMIT,
	    (phc->res.owin[OWIN0].end >> 16) & 0xfff0);

	/* These registers are not supported on the NS */
	pci_bus_write_config_word(&bus, devfn, PCI_IO_BASE_UPPER16, 0);
	pci_bus_write_config_word(&bus, devfn, PCI_IO_LIMIT_UPPER16, 0);

	/* Force class to that of a Bridge */
	pci_bus_write_config_word(&bus, devfn, PCI_CLASS_DEVICE, PCI_CLASS_BRIDGE_PCI);

	pci_bus_read_config_word(&bus, devfn, PCI_CLASS_DEVICE, &tmp16);
	pci_bus_read_config_word(&bus, devfn, PCI_MEMORY_BASE, &tmp16);
	pci_bus_read_config_word(&bus, devfn, PCI_MEMORY_LIMIT, &tmp16);

	HCD_FN_EXT();

	return;
}

/*
 * Function bcm947xx_hc_check_link(phc, allow_gen2)
 *
 *   Parameters:
 *    phc        ... pointer to pcie core hc data structure
 *    allow_gen2 ... flag to specify GEN2 allow or not
 *
 *   Description:
 *    Check link status, return 0 if link is up in RC mode,
 *    otherwise return non-zero
 *
 *   Return: 0 on success, -ve on failure
 */
static int __init noinline bcm947xx_hc_check_link(struct pcie_hc_core *phc, uint32 allow_gen2)
{
	u32 devfn = 0;
	u16 pos, tmp16, link = 0;
	u8 nlw, tmp8;
	u32 tmp32;
	int wait = 0;

	/* Fake <bus> object */
	struct pci_bus bus = {
	    .number = 0,
	    .ops = &bcm947xx_hc_ops,
	    .sysdata = phc,
	};

	HCD_FN_ENT();

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
	    HCD_INFO("PCIE [%d]: Retrain link failed\n", phc->info.id);

	/* See if the port is in EP mode, indicated by header type 00 */
	pci_bus_read_config_byte(&bus, devfn, PCI_HEADER_TYPE, &tmp8);
	if (tmp8 != PCI_HEADER_TYPE_BRIDGE) {
	    HCD_INFO("PCIe core %d in End-Point mode - ignored\n", phc->info.id);
		HCD_FN_EXT();
	    return -ENODEV;
	}

	/* bcm947189 PAX only changes NLW field when card is present */
	pos = pci_bus_find_capability(&bus, devfn, PCI_CAP_ID_EXP);
	pci_bus_read_config_word(&bus, devfn, pos + PCI_EXP_LNKSTA, &tmp16);


	HCD_INFO("PCIE%d: LINKSTA reg %#x val %#x\n", phc->info.id,
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
	HCD_INFO("PCIE%d link=%d\n", phc->info.id, link);

	HCD_FN_EXT();

	return (link ? 0 : -ENOSYS);
}

/*
 * Function bcm947xx_hc_map_init(phc)
 *
 *   Parameters:
 *    phc ... pointer to pcie core hc data structure
 *
 *   Description:
 *    init PCIE HCD map
 *
 *   Return: 0 on success, -ve on failure
 */
static void __init bcm947xx_hc_map_init(struct pcie_hc_core *phc)
{
	unsigned size, i;
	u32 addr;

	/*
	 * NOTE:
	 * All PCI-to-CPU address mapping are 1:1 for simplicity
	 */

	HCD_FN_ENT();

	/* Outbound address translation setup */
	size = resource_size(&phc->res.owin[OWIN0]);
	addr = phc->res.owin[OWIN0].start;
	BUG_ON(!addr);
	BUG_ON(addr & ((1 << 25) - 1));	/* 64MB alignment */

	for (i = 0; i < 3; i++) {
	    const unsigned win_size = SZ_64M;
	    /* 64-bit LE regs, write low word, high is 0 at reset */
	    writel(addr, phc->info.base + PCIE_GEN2_OMAP(i));
	    writel(addr|0x1, phc->info.base + PCIE_GEN2_OARR(i));
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
	writel(addr | 0x1, phc->info.base + PCIE_GEN2_IMAP1(0));
	writel(addr | size, phc->info.base + PCIE_GEN2_IARR(1));
	if (getMemorySize() <= SZ_128M) {
	    HCD_FN_EXT();
	    return;
	}

	/* DDR memory size > 128MB */
	addr = PHYS_OFFSET + SZ_128M;

	size = min(getMemorySize() - SZ_128M, (ulong)SZ_128M);
	size >>= 20;	/* In MB */
	size &= 0xff;	/* Size is an 8-bit field */

	writel(addr | 0x1, phc->info.base + PCIE_GEN2_IMAP2(0));
	writel(addr | size, phc->info.base + PCIE_GEN2_IARR(2));

	HCD_FN_EXT();

	return;
}

/*
 * Function bcm947xx_hc_hw_init(phc)
 *
 *   Parameters:
 *    phc ... pointer to pcie core hc data structure
 *
 *   Description:
 *    Check if PCIe link up or not
 *
 *   Return: 0 on success, -ve on failure
 */
static bool __init bcm947xx_hc_hw_init(struct pcie_hc_core *phc)
{
	u32 devfn = 0;
	u32 tmp32;
	u16 tmp16;

	/* Fake <bus> object */
	struct pci_bus bus = {
	    .number = 0,
	    .ops = &bcm947xx_hc_ops,
	    .sysdata = phc,
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
	writel(0x3, phc->info.base + PCIE_GEN2_CLK_CONTROL);
	udelay(250);
	writel(0x1, phc->info.base + PCIE_GEN2_CLK_CONTROL);
	mdelay(250);

	HCD_FN_EXT();

	return TRUE;
}

/*
 * Function bcm947xx_hc_is_pcie_link_up (phc)
 *
 *   Parameters:
 *    phc ... pointer to pcie core hc data structure
 *
 *   Description:
 *    Check if PCIe link up or not
 *
 *   Return: 0 on success, -ve on failure
 */
static bool __init bcm947xx_hc_is_pcie_link_up(struct pcie_hc_core *phc)
{
	int allow_gen2, linkfail;

	HCD_FN_ENT();

	for (allow_gen2 = 0; allow_gen2 <= 1; allow_gen2++) {
	    bcm947xx_hc_hw_init(phc);
	    bcm947xx_hc_map_init(phc);
	    /*
	     * Skip inactive ports -
	     * will need to change this for hot-plugging
	     */
	    linkfail = bcm947xx_hc_check_link(phc, allow_gen2);
	    if (linkfail)
	        break;
	    bcm947xx_hc_bridge_init(phc);

	    if (allow_gen2 == 0) {
	        if (allow_gen2_rc(phc) == 0)
	            break;
	        HCD_INFO("PCIE%d switching to GEN2\n", phc->info.id);
	    }
	}

	HCD_FN_EXT();
	return (linkfail) ? FALSE: TRUE;
}

/*
 * Function bcm947xx_hc_core_config (phc)
 *
 *   Parameters:
 *    phc ... pointer to pcie core hc data structure
 *
 *   Description:
 *    Setup pcie core legacy interrupts, outgoing memory window, bar1, pci class, UBUS
 *
 *   Return: 0 on success, -ve on failure
 */
static int bcm947xx_hc_core_config(struct pcie_hc_core *phc)
{
	int err;

	HCD_FN_ENT();

	err = bcm947xx_hc_post_init(phc);

	HCD_FN_EXT();

	return err;
}

/*
 * Function bcm947xx_hc_core_reset (phc)
 *
 *   Parameters:
 *    phc ... pointer to pcie core hc data structure
 *
 *   Description:
 *    Reset PCIe core using misc driver API's. Configure the phy parameters
 *    while the core is in reset.
 *
 *   Return: 0 on success, -ve on failure
 */
static int bcm947xx_hc_core_reset(struct pcie_hc_core *phc)
{
	uint32 clk_control;

	HCD_FN_ENT();

	/* this is a critical delay */
	mdelay(500);

	clk_control = readl(phc->info.base + PCIE_GEN2_CLK_CONTROL);

	/* clk_control bit 0 is PCIE_RC_PCIE_RESET. If Still under
	 * reset state, we need to trigger a pulse to reset device
	 */
	if (clk_control & 0x1) {
	    clk_control &= ~0x1;
	    writel(clk_control, phc->info.base + PCIE_GEN2_CLK_CONTROL);
	    mdelay(100);
	    /* read back to take effect */
	    clk_control = readl(PCIEGEN2_BASE + PCIE_GEN2_CLK_CONTROL);
	}

	HCD_FN_EXT();

	return 0;
}

/*
 * Function bcm947xx_hc_setup_rev (phc)
 *
 *   Parameters:
 *    phc ... pointer to pcie core hc data structure
 *
 *   Description:
 *    Initialize HC core cb based on the core revision
 *
 *   Return: 0 on success, -ve on failure
 */
static int bcm947xx_hc_setup_rev(struct pcie_hc_core *phc)
{
	HCD_FN_ENT();

	HCD_FN_EXT();

	return 0;
}

/*
 * Function bcm947xx_hc_init_res (phc)
 *
 *   Parameters:
 *    phc ... pointer to pcie core hc data structure
 *
 *   Description:
 *    Initialize the HCD resource entries to default values. Currently supported resources
 *    - PCIe core base, memory window, PCI bus range
 *
 *   Return: 0 on success, -ve on failure
 */
static int bcm947xx_hc_init_res(struct pcie_hc_core *phc)
{

	struct pcie_hc_core_res *pres = NULL;
	struct platform_device *pdev = NULL;

	HCD_FN_ENT();

	pdev = phc->pdev;
	pres = &phc->res;

	/* Initialize attributes with default values */
	switch (phc->info.id) {
	    case 0:
	        pres->base.start = PCIE_GEN2_PHYS_BASE;
	        pres->base.end = pres->base.start + SI_CORE_SIZE - 1;
	        pres->base.flags = IORESOURCE_MEM;
	        pres->owin[OWIN0].name = "PCIe Outbound Window, Port 0";
	        pres->owin[OWIN0].start = PCIE_GEN2_OWIN_PHYS_BASE;
	        pres->owin[OWIN0].end = pres->owin[OWIN0].start + PCIE_GEN2_OWIN_PHYS_SIZE - 1;
	        pres->owin[OWIN0].flags = IORESOURCE_MEM;
	        pres->irq = INTERRUPT_ID_PCIE_GEN2;
	        break;
	    default:
	        return -1;
	}

	HCD_FN_EXT();

	return 0;
}

/*
 * Function bcm947xx_hc_post_init (phc)
 *
 *   Parameters:
 *    phc ... pointer to pcie core hc data structure
 *
 *   Description:
 *    PCIe hcd driver probe. Called for each instance of the PCIe core.
 *    Get and allocate resource, configure hardware, start the PCI bus and
 *    enumerate PCI devices
 *
 *   Return: 0 on success, -ve on failure
 */
static int bcm947xx_hc_post_init(struct pcie_hc_core *phc)
{
	HCD_FN_ENT();

	/* Setup virtual-wire interrupts */
	writel(0xf, phc->info.base + PCIE_GEN2_PCIE_SYS_RC_INTX_EN);
	/* Enable memory and bus master */
	writel(0x6, phc->info.base + PCIE_GEN2_PCIE_HDR_OFF + 4);

	HCD_FN_EXT();

	return 0;
}

/*
 * Function bcm947xx_hc_probe (phc)
 *
 *   Parameters:
 *    phc ... pointer to pcie core data structure
 *
 *   Description:
 *    PCIe hc driver probe. Called for each instance of the PCIe core.
 *    Allocate control block and initialize the default resources
 *
 *   Return: 0 on success, -ve on failure
 */
static int bcm947xx_hc_probe(struct pcie_hc_core *phc)
{
	HCD_FN_ENT();

	/* Initialize hc elements */
	phc->phc_cb = NULL;
	phc->cfg.core_rev_offset = RC_CFG_TYPE1_REV_ID_CLASS_CODE_OFFSET;
	phc->cfg.core_rev_mask = 0xFF;

	/* Initialize  core resource element values for no device tree based
	 * legacy drivers
	 */
	bcm947xx_hc_init_res(phc);

	/* init D11 cores that needs to be enumerated through PCIE */
	bcm947xx_hndpci_init();

	/* only one device under the core */
	phc->info.devs = 1;

	HCD_FN_EXT();

	return 0;
}

/*
 * Function bcm947xx_hc_remove (phc)
 *
 *   Parameters:
 *    phc ... pointer to pcie core hc data structure
 *
 *   Description:
 *    PCIe hc driver remove - Free the allocated resources
 *
 *   Return: 0 on success, -ve on failure
 */
static void bcm947xx_hc_remove(struct pcie_hc_core *phc)
{
	HCD_FN_ENT();

	HCD_FN_EXT();

	return;
}

/*
 * +-----------------------------------------------------
 *  Global Functions
 * +-----------------------------------------------------
 */
/*
 * Function pmc_pcie_power_up (unit)
 *
 *   Parameters:
 *    unit ... pcie core number
 *
 *   Description:
 *    stub for 47xx platform as bsp has no implementation
 *
 *   Return: None
 */
void pmc_pcie_power_up(int unit, int is_dual_lane)
{
}

/*
 * Function pmc_pcie_power_down (unit)
 *
 *   Parameters:
 *    unit ... pcie core number
 *
 *   Description:
 *    stub for 47xx platform as bsp has no implementation
 *
 *   Return: None
 */
void pmc_pcie_power_down(int unit, int is_dual_lane)
{
}

/*
 * Function ubus_decode_pcie_wnd_cfg (base, size, core)
 *
 *   Parameters:
 *    base ... customized pcie window base address 
 *    size ... size of customized pcie window
 *    core ... pcie core index
 *
 *   Description:
 *    stub for 47xx platform as bsp has no implementation
 *
 *   Return: 0 on success, -ve on failure
 */
int ubus_decode_pcie_wnd_cfg(u32 base, u32 size, u32 core)
{
	return 0;
}

/*
 * Function pcie_hc_plt_init (hc_cfg)
 *
 *   Parameters:
 *    hc_cfg ... pointer to pcie host controller configuration data structure
 *
 *   Description:
 *    fill the hc configuration
 *
 *   Return: 0 on success, -ve on failure
 */
int pcie_phc_plt_init(struct pcie_hc_plt_cfg *hc_cfg)
{
	int ret = 0;

	HCD_FN_ENT();

	/* Sanity Check */
	if (hc_cfg == NULL) {
	    HCD_ERROR("bcm947xx: NULL hc_cfg\n");
	    return -EINVAL;
	}

	/* fill the configuration items */
	hc_cfg->num_cores = NUM_CORE;

	/* setup the platform device names (for built-in, non-dt legacy) */
	if (hc_cfg->plt_dev) {
	    int core;
	    for (core = 0; core < NUM_CORE; core++) {
	            hc_cfg->plt_dev[core].name = BCM947XX_PCIE_DEV_NAME;
	    }
	}

	/* PCI operations */
	hc_cfg->pci_fops = &bcm947xx_hc_ops;

	/* initialize bcm963xx fops */
	hc_cfg->init_core = bcm947xx_hc_probe;
	hc_cfg->free_core = bcm947xx_hc_remove;
	hc_cfg->setup_rev = bcm947xx_hc_setup_rev;
	hc_cfg->read_reg = hcd_readl;
	hc_cfg->write_reg = hcd_writel;
	hc_cfg->config_core = bcm947xx_hc_core_config;
	hc_cfg->reset_core = bcm947xx_hc_core_reset;
	hc_cfg->is_linkup = bcm947xx_hc_is_pcie_link_up;
	hc_cfg->map_core_irq = bcm947xx_hc_map_irq;
	hc_cfg->setup_msi = NULL;
	hc_cfg->teardown_msi = NULL;
	hc_cfg->get_msi = NULL;
	hc_cfg->setup_errint = NULL;
	hc_cfg->teardown_errint = NULL;

	HCD_FN_EXT();

	return ret;
}
