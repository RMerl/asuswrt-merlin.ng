/*
<:copyright-BRCM:2021:DUAL/GPL:standard

   Copyright (c) 2021 Broadcom
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
#include <linux/version.h>
#include <linux/pci.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/delay.h>
#include "../drivers/pci/pci.h"
#include <pcie_hcd.h>
#include <pcie_wlep.h>
#include <pcie-bcm963xx.h>

/*
 * +-----------------------------------------------------
 *  Defines
 * +-----------------------------------------------------
 */
#define WLEP_BAR0_OFFSET               0x00800000
#define WLEP_BAR0_SIZE                 0x00010000
#define WLEP_BAR0_PCIECORE_OFFSET      0x2000
#define WLEP_SBPCIEREG_MDIOCONTROL     (WLEP_BAR0_PCIECORE_OFFSET + 0x0128)
#define WLEP_SBPCIEREG_MDIOWRDATA      (WLEP_BAR0_PCIECORE_OFFSET + 0x012C)
#define WLEP_SBPCIEREG_MDIORDDATA      (WLEP_BAR0_PCIECORE_OFFSET + 0x0130)

#ifndef PCI_FIND_CAP_TTL
#define PCI_FIND_CAP_TTL               48
#endif

#ifndef PCI_PM_D3_WAIT
#define PCI_PM_D3_WAIT                 10
#endif

#define wlep_readl                     debug_wlep_readl
#define wlep_writel                    debug_wlep_writel
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
typedef struct wlep_dev {
	struct pci_bus bus;         /* EP bus */
	u32    owin0;               /* OWIN0 physical address */
	u32    bar0_phys;           /* BAR0 physical address */
	void __iomem *bar0_virt;    /* Mapped BAR0 address */
	u32    id;                  /* device vendor ID */
	u8     pm_pos;              /* PM capability position in EP config */
	u8     mlw;                 /* Maximum Link Width */
} wlep_dev_t;

/*
 * +-----------------------------------------------------
 *  Local Function prototype
 * +-----------------------------------------------------
 */

/*
 * +-----------------------------------------------------
 *  external Function prototype
 * +-----------------------------------------------------
 */

/*
 * +-----------------------------------------------------
 *  Global variables
 * +-----------------------------------------------------
 */
/*
 * +-----------------------------------------------------
 *  Local inline functions
 * +-----------------------------------------------------
 */
/* read 32bit pcie register space */
static inline u32 debug_wlep_readl(struct wlep_dev *dev, unsigned offset)
{
	u8 *base = (u8*)(dev->bar0_virt);
	u32 data = readl(base + offset);

	HCD_INFO("wlep_read_reg:  <0x%08x> <0x%08x>\n",
	    (u32)(dev->bar0_phys + offset), data);

	return data;
}

/* write 32bit pcie register space */
static inline void debug_wlep_writel(u32 data, struct wlep_dev *dev,
	unsigned offset)
{
	u8 *base = (u8*)(dev->bar0_virt);
	writel(data, base + offset);

	HCD_INFO("wlep_write_reg: <0x%08x> <0x%08x>\n",
	    (dev->bar0_phys + offset), data);

	return;
}

/*
 * +-----------------------------------------------------
 *  Local Functions
 * +-----------------------------------------------------
 */

/*
 * +-----------------------------------------------------
 *  Global Functions
 * +-----------------------------------------------------
 */
/*
 * Function wlep_mdio_read (dev, phyad, regad)
 *
 *
 *   Parameters:
 *    dev   ... pointer to WLAN EP deice data structure
 *    phyad ... MDIO PHY address (typically 0!)
 *    regad ... Register address in range 0-0x1f
 *
 *   Description:
 *  Description:
 *    Perform MDIO read on specified PHY (typically 0), and Register.
 *    Access is through an indirect command/status mechanism, and timeout
 *    is possible. Status completion is polled at most 3 times, 1ms interval
 *    Completion is expected on the first poll.
 *
 *   Return: 0 on success, -ve on failure
 *  Return: 16-bit data item or 0xdead on MDIO timeout
 */
u16 wlep_mdio_read(struct wlep_dev *dev, u16 phyad, u16 regad)
{
	int timeout;
	u32 data;
	u16 retval = 0xdead;

	HCD_FN_ENT();

	if (!dev || !dev->bar0_virt) {
	    HCD_ERROR("ep_mdio_read: <0x%04x> <0x%04x> Inv\n", regad, retval);
	    HCD_FN_EXT();
	    return 0xdead;
	}

	/* Bit 29: CMD(1), 27-24: port, 23-8: addr, 7-0: clock divider (2) */
	data = (1 << 29) | (0x2 << 0);
	data |= ((phyad & 0xf) << 24);
	data |= ((regad & 0xffff) << 8);
	wlep_writel(data, dev, WLEP_SBPCIEREG_MDIOCONTROL);

	/* critical delay */
	udelay(1000);

	timeout = 3;
	while (timeout > 0) {
	    data = wlep_readl(dev, WLEP_SBPCIEREG_MDIORDDATA);
	    /* Bit-31=1 is DONE */
	    if (data & 0x80000000)
	        break;
	    timeout = timeout - 1;
	    udelay(1000);
	}

	if (timeout == 0) {
	    HCD_ERROR("%s: <0x%04x> <0x%04x> timeout\n", __FUNCTION__, regad, retval);
	} else {
	    /* Bits 30:0 is read data */
	    retval = (data&0xffff);
	    HCD_LOG_MDIO("%s: <0x%04x> <0x%04x>\n", __FUNCTION__, regad, retval);
	}

	HCD_FN_EXT();

	return retval;
}

/*
 * Function wlep_mdio_write (dev, phyad, regad, wrdata)
 *
 *   Parameters:
 *    dev    ... pointer to WLAN EP deice data structure
 *    phyad  ... MDIO PHY address (typically 0!)
 *    regad  ... Register address in range 0-0x1f
 *    wrdata ... 16-bit write data
 *
 *   Description:
 *    Perform PCIE MDIO write on specified PHY (typically 0), and Register.
 *    Access is through an indirect command/status mechanism, and timeout
 *    is possible. If command is not immediately complete, which would
 *    be typically the case, one more attempt is made after a 1ms delay.
 *
 *   Return: 1 on success, 0 on timeout
 */
int wlep_mdio_write(struct wlep_dev *dev, u16 phyad, u16 regad, u16 wrdata)
{
	int timeout;
	u32 data;

	HCD_FN_ENT();

	if (!dev || !dev->bar0_virt) {
	    HCD_ERROR("%s: <0x%04x> <0x%04x> fail\n", __FUNCTION__, regad, wrdata);
	    return 0;
	}

	/* Bit 29: CMD(0), 27-24: port, 23-8: addr, 7-0: clock divider (2) */
	data = ((phyad & 0xf) << 24) | ((regad & 0xffff) << 8) | (0x2 << 0);
	wlep_writel(data, dev, WLEP_SBPCIEREG_MDIOCONTROL);
	udelay(1000);

	/* Bit-31=1 to initial the WRITE, bits 30:0 is the write data */
	data = (1 << 31) | (wrdata & 0x7fffffff);
	wlep_writel(data, dev, WLEP_SBPCIEREG_MDIOWRDATA);
	udelay(1000);

	/* Bit-31=0 when DONE */
	timeout = 3;
	while (timeout > 0) {

	    data = wlep_readl(dev, WLEP_SBPCIEREG_MDIOWRDATA);

	    /* CTRL1 Bit-31=1 is DONE */
	    if ((data & 0x80000000) == 0)
	        break;

	    timeout = timeout - 1;
	    udelay(1000);
	}

	HCD_FN_EXT();

	if (timeout == 0) {
	    HCD_ERROR("%s: <0x%04x> <0x%04x> fail\n", __FUNCTION__, regad, wrdata);
	    return 0;
	} else {
	    HCD_LOG_MDIO("%s: <0x%04x> <0x%04x>\n", __FUNCTION__, regad, wrdata);
	    return 1;
	}
}

/*
 * Function wlep_set_power_state (dev, state)
 *
 *   Parameters:
 *    dev ... pointer to WLAN EP deice data structure
 *    state   Power State (PCI_D3hot, PCI_D0)
 *
 *   Description:
 *    Set Device power to the required state
 *
 *   Return:
 *    On failure - -ve error code
 *    On success - 0
 */
int wlep_set_power_state(struct wlep_dev *dev, pci_power_t state)
{
	struct pci_bus *bus;
	int devfn = 0;
	u32 status_mask;

	HCD_FN_ENT();

	if (state == PCI_D0) {
	    status_mask = MISC_PCIE_STATUS_LINK_IN_L0;
	} else if (state == PCI_D3hot) {
	    status_mask = MISC_PCIE_STATUS_LINK_IN_L1;
	} else {
	    /* Other states are not supported */
	    HCD_FN_ENT();
	    return -EINVAL;
	}

	bus = &dev->bus;

	/* Place link in required power state */
	{
	    u16 pmcsr;
	    u8 ctl_pos = dev->pm_pos + PCI_PM_CTRL;

	    pci_bus_read_config_word(bus, devfn, ctl_pos, &pmcsr);
	    pmcsr &= ~PCI_PM_CTRL_STATE_MASK;
	    pmcsr |= state;
	    pci_bus_write_config_word(bus, devfn, ctl_pos, pmcsr);

	    mdelay(PCI_PM_D3_WAIT);
	}

	/* Delay 50 us (from Design team) */
	udelay(50);

	/* Confirm link in required state */
	{
	    u32 reg_val;
	    pci_bus_read_config_dword(bus->parent, devfn, MISC_PCIE_STATUS_OFFSET, &reg_val);
	    if (!(reg_val & status_mask)) {
	        HCD_ERROR("Failed to put PCIe Link in %d state\n", state);
	        HCD_FN_ENT();
	        return -EPERM;
	    }
	}

	return 0;
}

/*
 * Function wlep_get_mlw (dev)
 *
 *   Parameters:
 *    dev ... pointer to WLAN EP deice data structure
 *
 *   Description:
 *    get WLAN EP device Maximum Link Width
 *
 *   Return:
 *    On failure - 0
 *    On success - 1, 2 (lane width)
 */
int wlep_get_mlw(struct wlep_dev *dev)
{
	return (dev) ? dev->mlw : 0;
}

/*
 * Function wlep_config_dev (dev)
 *
 *   Parameters:
 *    dev ... pointer to WLAN EP deice data structure
 *
 *   Description:
 *    Configure Root Complex and WLAN device for Core register access
 *
 *   Return:
 *    On failure - error code
 *    On success - 0
 */
int wlep_config_dev(struct wlep_dev *dev)
{
	struct pci_bus *bus;
	struct pci_bus *root_bus;
	u32    data;

	HCD_FN_ENT();

	if (!dev || !dev->bar0_virt) {
	    HCD_ERROR("WLEP no dev or bar0 memory\n");
	    HCD_FN_EXT();
	    return -ENOMEM;
	}

	/* Initialize variables */
	bus = &dev->bus;
	root_bus = bus->parent;

	/* Disable Interrupts */
	pci_bus_write_config_word(root_bus, 0, PCI_COMMAND,
	    PCI_COMMAND_INTX_DISABLE);
	pci_bus_write_config_word(bus, 0, PCI_COMMAND, PCI_COMMAND_INTX_DISABLE);

	/* Program BAR0, BAR2 */
	pci_bus_write_config_byte(root_bus, 0, PCI_SUBORDINATE_BUS, 0x01);
	data = dev->owin0 | PCI_BASE_ADDRESS_MEM_TYPE_64;
	pci_bus_write_config_dword(bus, 0, PCI_BASE_ADDRESS_2, data);
	data = dev->owin0 + WLEP_BAR0_OFFSET;
	data |= (PCI_BASE_ADDRESS_MEM_TYPE_64);
	pci_bus_write_config_dword(bus, 0, PCI_BASE_ADDRESS_0, data);

	/* Enable EP memory access */
	data = (PCI_COMMAND_MEMORY| PCI_COMMAND_MASTER);
	pci_bus_write_config_word(root_bus, 0, PCI_COMMAND, data);
	pci_bus_write_config_word(bus, 0, PCI_COMMAND, data);

	HCD_FN_EXT();

	return 0;
}

/*
 * Function wlep_probe_dev (root_bus, owin0, prc)
 *
 *   Parameters:
 *    root_bus ... pointer to root bus
 *    owin0    ... OWIN0 physical address
 *    prc      ... pointer to return code
 *
 *   Description:
 *    Probe WLAN device, create a bus, initialize resources needed to access
 *    device
 *
 *   Return:
 *    On failure - NULL with updated prc
 *    On success - valid WLAN EP device pointer
 */
struct wlep_dev* wlep_probe_dev(struct pci_bus *root_bus, u32 owin0, int *prc)
{
	struct wlep_dev *dev;
	struct pci_bus *bus;
	int rc = 0;
	int devfn = 0;
	u8 exp_pos = 0;

	/* Allocate downstream bus for the connected device */
	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev) {
	    HCD_ERROR("Failed to allocate memory for wlep\n");
	    rc = -ENOMEM;
	    goto done;
	}
	HCD_INFO("Allocated memory for wlep device [0x%px]\n", dev);

	/* Initialize dev parameters */
	dev->owin0 = owin0;
	dev->pm_pos = 0;

	/* Set bus parameters required for access */
	bus = &dev->bus;
	bus->parent = root_bus;
	bus->ops = root_bus->ops;
	bus->sysdata = root_bus->sysdata;
	bus->bus_flags = root_bus->bus_flags;
	bus->number = PCIE_HCD_ROOT_BUSNUM + 1;

	dev->bar0_phys = owin0 + WLEP_BAR0_OFFSET;
	dev->bar0_virt = ioremap(dev->bar0_phys, WLEP_BAR0_SIZE);
	if (!dev->bar0_virt) {
	    HCD_ERROR("Failed to map OWIN0 \n");
	    rc = -EAGAIN;
	    goto done;
	}

	HCD_INFO("WLEP BAR0 [0x%08x] mapped to [0x%px]\n",
	    dev->bar0_phys, dev->bar0_virt);

	/* Allow access to device config space */
	{
	    u32 buses;

	    pci_bus_read_config_dword(root_bus, devfn, PCI_PRIMARY_BUS, &buses);
	    buses &= ~0xFFFFFF;
	    buses |= 0x0F0100;
	    pci_bus_write_config_dword(root_bus, 0, PCI_PRIMARY_BUS, buses);
	}

	/* Is there a device present on devfn=0 ? */
	devfn = 0;
	if (!pci_bus_read_dev_vendor_id(bus, devfn, &dev->id, 60*1000)) {
	    HCD_ERROR("can not get vendor_id\n");
	    rc = -EAGAIN;
	    goto done;
	}
	HCD_INFO("WLEP device id [0x%08x]\n", dev->id);

	/* Check if device supports Capability List */
	{
	    u16 status;
	    pci_bus_read_config_word(bus, devfn, PCI_STATUS, &status);
	    if (!(status & PCI_STATUS_CAP_LIST)) {
	        HCD_ERROR("device [0x%08x] does not have Capability List\n",
	            dev->id);
	        rc = -EAGAIN;
	        goto done;
	    }
	}

	/* Get Device PM and PCIe Capability Position */
	{
	    u8 id;
	    u16 ent;
	    int ttl = PCI_FIND_CAP_TTL;
	    u8 pos = PCI_CAPABILITY_LIST;

	    pci_bus_read_config_byte(bus, devfn, pos, &pos);

	    while ((ttl)--) {
	        if (pos < 0x40)
	            break;

	        pos &= ~3;
	        pci_bus_read_config_word(bus, devfn, pos, &ent);


	        id = ent & 0xff;
	        if (id == 0xff)
	            break;

	        if (id == PCI_CAP_ID_PM) {
	            dev->pm_pos = pos;
	        }
	        if (id == PCI_CAP_ID_EXP) {
	            exp_pos = pos;
	        }

	        pos = (ent >> 8);
	    }
	}

	/* Check if Device Supports PCIe */
	if (exp_pos <= PCI_CAPABILITY_LIST) {
	    HCD_ERROR("can not find PCIE Capability\n");
	    rc = -EAGAIN;
	    goto done;
	}
	HCD_INFO("WLEP device exp pos [%d]\n", exp_pos);

	/* Check if Device Supports PM Capability */
	if (dev->pm_pos <= PCI_CAPABILITY_LIST) {
	    HCD_ERROR("can not find PM Capability\n");
	    rc = -EAGAIN;
	    goto done;
	}
	HCD_INFO("WLEP device pm pos [%d]\n", dev->pm_pos);

	dev->mlw = 0;
	{
	    u8 pos = exp_pos + PCI_EXP_LNKCAP;
	    u32 lnkcap;

	    pci_bus_read_config_dword(bus, devfn, pos, &lnkcap);

	    dev->mlw = RC_CFG_PCIE_LINK_CAP_LINK_WIDTH(lnkcap);
	    HCD_INFO("WLEP device %d lanes\n", dev->mlw);
	}

done:
	if (rc != 0) {
	    wlep_free_dev(dev);
	    if (prc) *prc = rc;
	    dev = NULL;
	}

	HCD_FN_EXT();

	return dev;
}

/*
 * Function wlep_free_dev (dev)
 *
 *   Parameters:
 *    dev ... pointer to WLAN EP deice data structure
 *
 *   Description:
 *    Free up allocated resources for WLAN EP device
 *
 *   Return: None
 */
void wlep_free_dev(struct wlep_dev *dev)
{
	HCD_FN_ENT();

	if (dev) {
	    struct pci_bus *bus = &dev->bus;

	    /* Reset the buses configuration */
	    pci_bus_write_config_dword(bus->parent, 0, PCI_PRIMARY_BUS, 0);

	    /* unmap bar0 memory */
	    if (dev->bar0_virt) {
	        iounmap(dev->bar0_virt);
	    }

	    /* Free the device memory */
	    kfree(dev);

	    HCD_INFO("Freed wlep device 0x%px\n", dev);
	}

	HCD_FN_EXT();

	return;
}
