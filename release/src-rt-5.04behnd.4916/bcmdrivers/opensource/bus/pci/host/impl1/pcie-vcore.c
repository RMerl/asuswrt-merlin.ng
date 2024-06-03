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
#include <pmc_pcie.h>
#include <shared_utils.h>

#include <pcie_hcd.h>
#include <pcie-vcore.h>
#include <pcie-vdev.h>

#include <pmc_wlan.h>
#include <pmc_core_api.h>

/*
 * +-----------------------------------------------------
 *  Defines
 * +-----------------------------------------------------
 */
#define BCMVPCIE_HC_REV                     0x00000100  /* 1.0 */
#define BCMVPCIECR                          BCMVPCIE_HC_REV
#define BCMVPCIE_HC_REV_MASK                0x0000FFFF
#define BCMVPCIE_HC_REV_OFFSET              0xb0

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
 * PCIe Vitual Device control block
 *
 * @cfg         : virtual device configuration
 * @active      : flag to specify device configuration exists
 * @enabled     : flag to specify powered and enabled
 * @irq         : device irq (if present)
 */
struct bcmvpcie_vdev
{
	struct bcmvpcie_vdev_cfg cfg;
	bool                 active;
	bool                 enabled;
	int                  irq[PCIE_VDEV_IRQ_IDX_MAX];
};

/*
 * PCIe Vitual Core control block
 *
 * @vid      : virtual core id
 * @num_dev  : number of virtual devices per core
 * @dev      : virual device control block
 */
struct bcmvpcie_hc_cb
{
	int                     vid;
	int                     num_dev;
	struct bcmvpcie_vdev    dev[MAX_NUM_VDEV];
};

/*
 * +-----------------------------------------------------
 *  Local Function prototype
 * +-----------------------------------------------------
 */
/* Config Space Access */
static int bcmvpcie_hc_config_read(struct pci_bus *bus, unsigned int devfn,
	int where, int size, u32 *val);
static int bcmvpcie_hc_config_write(struct pci_bus *bus, unsigned int devfn,
	int where, int size, u32 val);

/* Core Access */
static int bcmvpcie_hc_map_irq(const struct pci_dev *pcidev,
	u8 slot, u8 pin);

/* Core setup */
static int bcmvpcie_hc_core_config(struct pcie_hc_core *phc);

static int bcmvpcie_hc_parse_slot_dt(struct pcie_hc_core *phc,
	struct device_node *np, struct device_node *np_slot);
static int bcmvpcie_hc_parse_dt(struct pcie_hc_core *phc);
static int bcmvpcie_hc_init_dev(struct pcie_hc_core *phc);

static int bcmvpcie_hc_probe(struct pcie_hc_core *phc);
static void  bcmvpcie_hc_remove(struct pcie_hc_core *phc);

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

static struct pci_ops bcmvpcie_hc_ops = {
	.read = bcmvpcie_hc_config_read,
	.write = bcmvpcie_hc_config_write,
};

static u32 bcmvpcie_hc_core_reg[] = {
	/* 0x00 */ 0x000014e4, 0x00000000, 0x00000000, 0x00000000,
	/* 0x10 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	/* 0x20 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	/* 0x30 */ 0x00000000, 0x00000048, 0x00000000, 0x00000000,
	/* 0x40 */ 0x00000000, 0x00000000, 0x0000ac01, 0x00000000,
	/* 0x50 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	/* 0x60 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	/* 0x70 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	/* 0x80 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	/* 0x90 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	/* 0xa0 */ 0x00000000, 0x00000000, 0x00000000, 0x00420010,
	/* 0xb0 */ BCMVPCIECR, 0x00000000, 0x00000000, 0x00000000,
	/* 0xc0 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	/* 0xd0 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	/* 0xe0 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	/* 0xf0 */ 0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

/*
 * +-----------------------------------------------------
 *  Local inline functions
 * +-----------------------------------------------------
 */
static inline bool bcmvpcie_hc_access_valid(struct bcmvpcie_hc_cb *phc_cb,
	uint32 bus, uint32 dev, uint32 func, uint *pidx)
{
	int idx;
	bool valid = FALSE;

	HCD_FN_ENT();

	HCD_INFO("bus_no [%d] dev [%d] func [%d]\r\n", bus, dev, func);

	/*
	 * go through all available devices and check if bus, slot
	 * numbers matches for the enabled devices
	 */
	for (idx = 0; idx < phc_cb->num_dev; idx++) {
	    if ((phc_cb->dev[idx].enabled == TRUE) &&
	        (bus == phc_cb->dev[idx].cfg.bus)) {
	        valid = (dev == phc_cb->dev[idx].cfg.slot);
	    }
	    if (valid == TRUE) break;
	}

	/* Update the device index if available */
	if (pidx) *pidx = idx;

	HCD_FN_EXT();
	return valid;
}

static inline int bcmvpcie_hc_pwrup_dev(struct pcie_hc_core *phc, int devidx)
{
	int rc;
	struct bcmvpcie_hc_cb *phc_cb = (struct bcmvpcie_hc_cb*)(phc->phc_cb);

	phc_cb->dev[devidx].enabled = FALSE;
	rc = pmc_wlan_power_up(phc_cb->dev[devidx].cfg.slot);
	if (rc == kPMC_NO_ERROR) {
	    phc_cb->dev[devidx].enabled = TRUE;
	    HCD_INFO("vcore [%d] vdev [%d] powered up\n", phc_cb->vid, devidx);
	} else {
	    HCD_ERROR("vcore [%d] vdev [%d] failed to power up rc [%d]\n",
	        phc_cb->vid, devidx, rc);
	}

	return rc;
}

static inline void bcmvpcie_hc_pwrdn_dev(struct pcie_hc_core *phc, int devidx)
{
	struct bcmvpcie_hc_cb *phc_cb = (struct bcmvpcie_hc_cb*)(phc->phc_cb);

	pmc_wlan_power_down(phc_cb->dev[devidx].cfg.slot);

	phc_cb->dev[devidx].enabled = FALSE;
	HCD_INFO("vcore [%d] vdev [%d] powered down\n", phc_cb->vid, devidx);

	return;
}

/* read 32bit pcie register space */
static inline u32 bcmvpcie_hc_read(struct pcie_hc_core *phc, unsigned offset)
{
	u32 data;

	data = bcmvpcie_hc_core_reg[offset/sizeof(u32)];

	HCD_LOG_REG("read_reg [0x%px + 0x%04x] = 0x%08x\n", phc->info.base, offset, data);
	return data;
}

/* write 32bit pcie register space */
static inline void bcmvpcie_hc_write(u32 data, struct pcie_hc_core *phc, unsigned offset)
{
	bcmvpcie_hc_core_reg[offset/sizeof(u32)] = data;
	HCD_LOG_REG("write_reg [0x%px + 0x%04x] = 0x%08x\n", phc->info.base, offset, data);
}

/*
 * +-----------------------------------------------------
 *  Local Functions
 * +-----------------------------------------------------
 */

/*
 * Function bcmvpcie_hc_config_read (bus, devfn, where, size, val)
 *
 *   Parameters:
 *    bus   ... pointer to pci bus data structure
 *    devfn ... pci device, function mapping
 *    where ... offset from the device base
 *    size  ... access size
 *    val   ... pointer to read data to be written
 *
 *   Description:
 *    Check and write to virtual device first, if not, go back to external device
 *
 *   Return: PCIBIOS_DEVICE: SUCCESS or DEVICE_NOT_FOUND on failure
 */
static int bcmvpcie_hc_config_read(struct pci_bus *bus, unsigned int devfn,
	int where, int size, u32 *val)
{
	struct pcie_hc_core *phc = bus->sysdata;
	struct bcmvpcie_hc_cb *phc_cb = (struct bcmvpcie_hc_cb*)(phc->phc_cb);
	int slot = PCI_SLOT(devfn);
	int ret = PCIBIOS_DEVICE_NOT_FOUND;
	ulong addr = 0;
	uint devidx = 0;
	bool valid;

	HCD_FN_ENT();

	valid = bcmvpcie_hc_access_valid(phc_cb, bus->number, slot, PCI_FUNC(devfn), &devidx);
	if (valid == FALSE) {
	    HCD_INFO("%s: invalid access bus %d, slot %d, func %d where %d\r\n",
	        __FUNCTION__, bus->number, slot, PCI_FUNC(devfn), where);
	    goto done;
	}

	if (where >= PCI_CFG_REGS_SIZE) {
	    HCD_INFO("%s: offset [%d] exceeds config space size [%d]\r\n",
	        __FUNCTION__, where, PCI_CFG_REGS_SIZE);
	    ret = PCIBIOS_BAD_REGISTER_NUMBER;
	    goto done;
	}

	addr =  (ulong)(&phc_cb->dev[devidx].cfg) + where;

	if (size == 1)
	    *((uint8 *)val) = *((uint8 *)(addr));
	else if (size == 2)
	    *((uint16 *)val) = (*((uint16 *)(addr)));
	else {
	    *((uint32 *)val) = (*((uint32 *)(addr)));

	    /*
	     * Special handling for BAR's in Linux
	     *
	     * Base addresses specify locations in memory or I/O space.
	     * Decoded size can be determined by writing a value of
	     * 0xffffffff to the register, and reading it back.  Only
	     * 1 bits are decoded.
	     */
	    if ((*val == 0xffffffff) &&
	        ((where >= PCI_BASE_ADDRESS_0) && (where <= PCI_BASE_ADDRESS_5))) {
	        *val = ~(phc_cb->dev[devidx].cfg.bar_size[(where - PCI_BASE_ADDRESS_0)/4]-1);
	    } else if (where == PCI_ROM_ADDRESS) {
	        *val = 0xffffffff;
	    }
	}

	ret = PCIBIOS_SUCCESSFUL;

	HCD_INFO("read [%d] [%d] [%d] [%d] = 0x%x return %d\r\n",
	    bus->number, slot, PCI_FUNC(devfn), where, *val, ret);

done:
	HCD_FN_EXT();
	return ret;
}

/*
 * Function bcmvpcie_hc_config_write (bus, devfn, where, size, val)
 *
 *   Parameters:
 *    bus   ... pointer to pci bus data structure
 *    devfn ... pci device, function mapping
 *    where ... offset from the device base
 *    size  ... access size
 *    val   ... value to be written
 *
 *   Description:
 *    Check and write to virtual device first, if not, go back to external device
 *
 *   Return: PCIBIOS_DEVICE: SUCCESS or DEVICE_NOT_FOUND on failure
 */
static int bcmvpcie_hc_config_write(struct pci_bus *bus, unsigned int devfn,
	int where, int size, u32 val)
{
	struct pcie_hc_core *phc = bus->sysdata;
	struct bcmvpcie_hc_cb *phc_cb = (struct bcmvpcie_hc_cb*)(phc->phc_cb);
	int slot = PCI_SLOT(devfn);
	int ret = PCIBIOS_DEVICE_NOT_FOUND;
	uint8* addr;
	uint devidx = 0;
	bool valid;

	HCD_FN_ENT();

	valid = bcmvpcie_hc_access_valid(phc_cb, bus->number, slot, PCI_FUNC(devfn), &devidx);
	if (valid == FALSE) {
	    HCD_LOG("%s: invalid access bus %d, slot %d, func %d where %d\r\n",
	        __FUNCTION__, bus->number, slot, PCI_FUNC(devfn), where);
	    goto done;
	}

	if (where >= PCI_CFG_REGS_SIZE) {
	    HCD_ERROR("%s: offset [%d] exceeds config space size [%d]\r\n",
	        __FUNCTION__, where, PCI_CFG_REGS_SIZE);
	    ret = PCIBIOS_BAD_REGISTER_NUMBER;
	    goto done;
	}

	addr =  (uint8*)(&phc_cb->dev[devidx].cfg) + where;

	if (size == 1)
	    *((uint8 *)addr) = val;
	else if (size == 2)
	    *((uint16 *)addr) = val;
	else
	    *((uint32 *)addr) = val;

	ret = PCIBIOS_SUCCESSFUL;

	HCD_INFO("write [%d] [%d] [%d] [%d] = 0x%x return %d\r\n",
	    bus->number, slot, PCI_FUNC(devfn), where, val, ret);

	if (where >= 0x3c && where <= 0x3f) {
	    HCD_INFO("where [0x%x] set to 0x%x\r\n", where, val);
	}

done:
	HCD_FN_EXT();
	return ret;
}

/*
 * Function bcmvpcie_hc_map_irq (pcidev, slot, pin)
 *
 *   Parameters:
 *    pcidev ... pointer to pci device data structure
 *    slot   ... pci slot/device id
 *    pin    ... pin number (not used)
 *
 *   Description:
 *    Get the pcie core irq number.
 *
 *   Return: pcie core irq number
 */
int bcmvpcie_hc_map_irq(const struct pci_dev *pcidev,
	u8 slot, u8 pin)
{
	struct pcie_hc_core *phc = pcidev->bus->sysdata;
	struct bcmvpcie_hc_cb *phc_cb = (struct bcmvpcie_hc_cb*)(phc->phc_cb);
	int idx;
	int irq = -1;

	HCD_FN_ENT();

	HCD_INFO("dev [0x%px] slot [%d] pin [%d]\r\n",
	    pcidev, slot, pin);

	for (idx = 0; idx < phc_cb->num_dev; idx++) {
	    if (slot == phc_cb->dev[idx].cfg.slot) {
	        irq = phc_cb->dev[idx].irq[PCIE_VDEV_IRQ_IDX_D11MAC];
	        break;
	    }
	}

	HCD_FN_EXT();
	return irq;
}

/*
 * Function bcmvpcie_hc_set_power(phc, devidx, up, type)
 *
 *   Parameters:
 *    phc    ... pointer to core control block
 *    devidx ... core device index
 *    up     ... flag to sepcify power up/down
 *    type   ... mask to specify EP and/or RC device type
 *
 *   Description:
 *    power up/down the core device
 *
 *   Return: 0 success, -ve on failure
 */
int bcmvpcie_hc_set_power(struct pcie_hc_core *phc, unsigned int devidx,
	bool up, int type)
{
	struct bcmvpcie_hc_cb *phc_cb = (struct bcmvpcie_hc_cb*)(phc->phc_cb);
	int ret = 0;

	HCD_FN_ENT();

	if (phc_cb->dev[devidx].active == false)
	    return 0;

	if (up == true) {
	    ret = bcmvpcie_hc_pwrup_dev(phc, devidx);
	    if (ret == 0)
	        phc_cb->dev[devidx].enabled = true;
	    else
	        phc_cb->dev[devidx].enabled = false;
	} else {
	    bcmvpcie_hc_pwrdn_dev(phc, devidx);
	    phc_cb->dev[devidx].enabled = false;
	    /* Make resource not available */
	    phc->res.owin[devidx].start = 0x0;
	}

	HCD_FN_EXT();
	return ret;
}

/*
 * Function bcmvpcie_hc_parse_slot_dt (phc, np, np_slot)
 *
 *   Parameters:
 *    phc ... pointer to pcie core hcd data structure
 *    np ... Pointer to vpcie core device tree node
 *    np_slot ... Pointer to vpcie slot devie tree node
 *
 *   Description:
 *    Parse pcie slot device tree entries. Currently supported resources
 *    - PCIe core base, memory window, PCI bus range
 *
 *   Return: 0 on success, -ve on failure
 */
static int bcmvpcie_hc_parse_slot_dt(struct pcie_hc_core *phc,
	struct device_node *np, struct device_node *np_slot)
{
	struct bcmvpcie_hc_cb *phc_cb = (struct bcmvpcie_hc_cb*)(phc->phc_cb);
	int devidx;
	int slot;
	int win = OWIN0;
	struct resource      *owin;

	struct resource base;
	struct bcmvpcie_vdev *pvdev;
	struct pcie_cfg_space_regs *regs;
	u32 dt_val;
	int err;

	HCD_FN_ENT();

	devidx = phc_cb->num_dev;

	/* Update the WLAN device register base and size */
	slot = 0;

	err = of_property_read_u32(np_slot, "brcm,coreid", &dt_val);
	if (err == 0) {
	    HCD_INFO("Updating coreid from DT entry\n");
	    slot = dt_val & 0xF;
	} else {
	    HCD_WARN("No DT entry for WiFi Coreid, skipping\n");
	    return -EINVAL;
	}

	/* This slot status is disabled */
	if (!of_device_is_available(np_slot)) {
	    HCD_INFO("DT entry disabled for core [%d], skipping core\n", slot);
	    return -EINVAL;
	}

	/* Virtual PCIe device id */
	err = of_property_read_u32(np_slot, "brcm,devid", &dt_val);
	if (err != 0) {
	    HCD_WARN("No DT entry for devid, skipping core\n");
	    return -EINVAL;
	}

	HCD_INFO("Updating devid from DT entry\n");

	pvdev = &phc_cb->dev[devidx];
	pvdev->cfg.slot = slot;
	regs = &pvdev->cfg.regs;

	dt_val &= 0xFFFF;
	regs->device_vendor_id &= 0x0000FFFF;
	regs->device_vendor_id |= (dt_val << 16);
	regs->subsystem_id_vendor_id = regs->device_vendor_id;

	/* WIFI core register base */
	err = of_address_to_resource(np, devidx, &base);
	if (err != 0) {
	    HCD_ERROR("No reg DT entry for core [%d] , skipping core\n", slot);
	    return -EINVAL;
	}

	/* WIFI core interrupts */
	{
	    struct pcie_vdev_cap_vndr *vend_cap;
	    int irq_idx;

	    vend_cap = (struct pcie_vdev_cap_vndr*)(regs->vendor_specific);
	    vend_cap->cap_id = PCI_CAP_ID_VNDR;
	    vend_cap->version = 0x01; /* v0.1 */

	    for (irq_idx = 0; irq_idx < PCIE_VDEV_IRQ_IDX_MAX; irq_idx++) {
	        err = of_irq_get(np, irq_idx + devidx * PCIE_VDEV_IRQ_IDX_MAX);
	        if (err > 0) {
	            HCD_INFO("Updating irq_idx [%d] from DT entry\n", irq_idx);
	            pvdev->irq[irq_idx] = err;
	            vend_cap->wifi_irq[irq_idx] = err;
	            if (irq_idx == PCIE_VDEV_IRQ_IDX_D11MAC) {
	                regs->cfg3c.int_line = err;
	            }
	        } else {
	            vend_cap->wifi_irq[irq_idx] = 0;
	            HCD_INFO("No DT entry for core [%d] irq [%d], skip rest\n",
	                devidx, irq_idx);
	            break;
	        }
	    }
	    vend_cap->length = 1 + irq_idx * sizeof(uint16);
	}

	pvdev->active = true;

	pvdev->cfg.bar_size[0] = base.end - base.start + 1;
	regs->bar_1 = base.start | PCI_BASE_ADDRESS_MEM_TYPE_64;

	owin = &phc->res.owin[win];
	if (pvdev->cfg.bar_size[0]) {
	    owin->name = dev_name(&phc->pdev->dev);
	    owin->start = regs->bar_1 & PCI_BASE_ADDRESS_MEM_MASK;
	    owin->end = owin->start + pvdev->cfg.bar_size[0] - 1;
	    owin->flags = IORESOURCE_MEM;
	    win++;
	}

	/* Virtual PCIe device revision id */
	{
	    err = of_property_read_u32(np_slot, "brcm,revid", &dt_val);
	    if (err == 0) {
	        HCD_INFO("Updating revid from DT entry\n");

	        regs->rev_id_class_code &= 0xFFFFFF00;
	        regs->rev_id_class_code |= (dt_val & 0xFF);
	    } else {
	        HCD_WARN("No DT entry for revid, using defaults\n");
	    }
	}


	/* PCIe force power on setting */
	{
	    err = of_property_read_u32(np, "brcm,apon", &dt_val);
	    if (err == 0) {
	        HCD_INFO("Updating apon settings from DT entry\n");
	        phc->cfg.apon = dt_val & 0xFF;
	    } else {
	        HCD_WARN("No DT entry for apon, using defaults\n");
	    }
	}

	HCD_LOG("vcore [%d] found vdev [%d] \r\n", phc_cb->vid, devidx);

	HCD_INFO("  device_vendor [0x%x] d11mac [%d] m2mdma [%d]\r\n",
	        regs->device_vendor_id, pvdev->irq[PCIE_VDEV_IRQ_IDX_D11MAC],
	        pvdev->irq[PCIE_VDEV_IRQ_IDX_M2MDMA]);

	HCD_INFO("  owin: start [0x%llx] end [0x%llx] flags [0x%lx]\r\n",
	        (u64)owin->start, (u64)owin->end, owin->flags);

	HCD_FN_EXT();

	return 0;
}

/*
 * Function bcmvpcie_hc_parse_dt (phc)
 *
 *   Parameters:
 *    phc ... pointer to pcie core hcd data structure
 *
 *   Description:
 *    Parse pcie core hcd device tree entries. Currently supported resources
 *    - PCIe core base, memory window, PCI bus range
 *
 *   Return: 0 on success, -ve on failure
 */
static int bcmvpcie_hc_parse_dt(struct pcie_hc_core *phc)
{
	struct bcmvpcie_hc_cb *phc_cb = (struct bcmvpcie_hc_cb*)(phc->phc_cb);
	struct device_node *np = NULL;
	struct device_node *np_slot = NULL;

	HCD_FN_ENT();

	/* Initialize attributes from device tree if present */
	np = phc->pdev->dev.of_node;
	if (!np) {
	    HCD_ERROR("[%d] DT node not available\n", phc->info.id);
	        return -ENODEV;
	}

	HCD_LOG("[%d] DT node available: %s\n", phc->info.id, np->full_name);

	/* Check if DT entry status is enabled or okay to load */
	if (!of_device_is_available(np)) {
	    dev_err(&phc->pdev->dev, "DT status disabled\n");
	    return -ENODEV;
	}

	/* Check if parent node has the single device information */
	phc_cb->num_dev = 0;
	if (bcmvpcie_hc_parse_slot_dt(phc, np, np) == 0) {
	    /* Parent node has single device */
	    phc_cb->num_dev++;
	} else {
	    /* Go through each slot device tree nodes */
	    for_each_child_of_node(np, np_slot) {
	        bcmvpcie_hc_parse_slot_dt(phc, np, np_slot);
	        phc_cb->num_dev++;
	    }
	}

	if (phc_cb->num_dev) {
	    phc->info.devs = phc_cb->num_dev;
	}

	HCD_FN_EXT();

	return 0;
}

/*
 * Function bcmvpcie_hc_core_config (phc)
 *
 *   Parameters:
 *    phc ... pointer to pcie core hc data structure
 *
 *   Description:
 *    Dummy
 *
 *   Return: 0 on success, -ve on failure
 */
static int bcmvpcie_hc_core_config(struct pcie_hc_core *phc)
{
	struct bcmvpcie_hc_cb *phc_cb = (struct bcmvpcie_hc_cb*)(phc->phc_cb);
	HCD_FN_ENT();

	HCD_LOG("[%d] vcore [%d] initialized with [%d] device%s\r\n",
	    phc->info.id, phc_cb->vid, phc_cb->num_dev,
	    (phc_cb->num_dev > 1) ? "s" : "");

	HCD_FN_EXT();

	return 0;
}

/*
 * Function bcmvpcie_hc_init_dev (phc)
 *
 *   Parameters:
 *    phc ... pointer to pcie core hcd data structure
 *
 *   Description:
 *    Initialize the HCD resource entries to default values. Currently supported resources
 *    - PCIe virtual core base as memory window, PCI bus range
 *
 *   Return: 0 on success, -ve on failure
 */
static int bcmvpcie_hc_init_dev(struct pcie_hc_core *phc)
{
	struct bcmvpcie_hc_cb *phc_cb = (struct bcmvpcie_hc_cb*)(phc->phc_cb);
	struct bcmvpcie_vdev *pvdev = NULL;
	struct bcmvpcie_vdev_cfg *pvcfg;
	int vid;
	int devidx = 0;
	struct resource *owin;
	struct resource bus_range;

	HCD_FN_ENT();

	/* Fill the virtual device config space with default values */
	vid = phc_cb->vid;

	/* Get the virtual device configuration. Hierarchy
	 * - nvram full configruation (TODO: pcie_vdev%d_cfg_path)
	 * - Default built-in configuration
	 * - nvram specific configuration parameters
	 * - Device Tree
	 */
	/* Copy Default built-in configuration */
	pvcfg = (struct bcmvpcie_vdev_cfg*)pcie_wifidefdev_cfg;

	bus_range.start = 0xFF;
	bus_range.end = 0;
	phc->cfg.apon = 0x00;

	for (devidx = 0; devidx < MAX_NUM_VDEV; devidx++) {
	    pvdev = &phc_cb->dev[devidx];


	    if (pvcfg) {
	        memcpy(&pvdev->cfg, pvcfg, sizeof(struct bcmvpcie_vdev_cfg));
	        pvcfg++;
	    }
	    pvdev->active = false;
	    phc->cfg.apon |= (HCD_APON_OFF << (devidx * HCD_APON_SHIFT));


	    /* Initialize device id and revision from SoC */
	    {
	        uint32 dev_vend = PCI_VENDOR_ID_BROADCOM;
	        uint32 class_rev = (PCI_CLASS_NETWORK_OTHER << 16);

	        /* ChipId as device Id, vendor Broadcom */
	        dev_vend |= ((kerSysGetChipId() & 0xFFFF) << 16);
	        pvdev->cfg.regs.device_vendor_id = dev_vend;
	        pvdev->cfg.regs.subsystem_id_vendor_id = dev_vend;

	        /* Chip revision as device revision, class network */
	        class_rev |= (UtilGetChipRev() & 0xFF);
	        pvdev->cfg.regs.rev_id_class_code = class_rev;
	    }

	    /* TODO: Add nvram control for
	     * - device id       (pcie_vcore%d_dev_id )
	     * - revision id     (pcie_vcore%d_rev_id)
	     * - class code      (pcie_vcore%d_class_code)
	     * - bar_1 addr, size (pcie_vcore%d_bar1_addr), (pcie_vcore%d_bar1_size)
	     * - interrupt line   (pcie_vcore%d_irq)
	     * - slot/dev num     (pcie_vcore%d_slot_num)
	     * - or complete config space binary file path
	     *    pcie_vcore%d_cfg_path=/etc/pcie/vcore0/cfg.bin
	     */

	    owin = &phc->res.owin[devidx];
	    if (pvdev->cfg.bar_size[0]) {
	        owin->name = dev_name(&phc->pdev->dev);
	        owin->start = pvdev->cfg.regs.bar_1 & PCI_BASE_ADDRESS_MEM_MASK;
	        owin->end = owin->start + pvdev->cfg.bar_size[0] - 1;
	        owin->flags = IORESOURCE_MEM;
	    }

	    /* PCI_INTERRUPT_PIN */
	    pvdev->irq[PCIE_VDEV_IRQ_IDX_D11MAC] = pvdev->cfg.regs.cfg3c.int_line;

	    if (pvdev->cfg.bus > bus_range.end) {
	        bus_range.end = pvdev->cfg.bus;
	    }
	    if (pvdev->cfg.bus < bus_range.start) {
	        bus_range.start = pvdev->cfg.bus;
	    }

	    if (pvdev->cfg.bar_size[0]) {
	        HCD_INFO("vcore [%d] vdev [%d] pvdev [0x%px] pvcfg [0x%px]\r\n",
	            vid, devidx, pvdev, pvcfg);

	        HCD_INFO("  device_vendor [0x%x] int [%d]\r\n",
	            pvdev->cfg.regs.device_vendor_id,
	            pvdev->irq[PCIE_VDEV_IRQ_IDX_D11MAC]);

	        HCD_INFO("owin: start [0x%llx] end [0x%llx] flags [0x%lx]\r\n",
	            (u64)owin->start, (u64)owin->end, owin->flags);
	    }
	}

	phc->res.bus_range.start = bus_range.start;
	phc->res.bus_range.end = bus_range.end;

	HCD_FN_EXT();

	return 0;
}

/*
 * Function bcmvpcie_hc_probe (phc)
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
int bcmvpcie_hc_probe(struct pcie_hc_core *phc)
{
	struct bcmvpcie_hc_cb *phc_cb = NULL;
	int err = 0;
	int vid;

	HCD_FN_ENT();

	/* Convert the core index to virtual core index */
	vid = PLTID2CORE(phc->info.id);

	if (vid >= NUM_VCORE) {
	    HCD_ERROR("vcore [%d] exceeds max vcores [%d]\r\n",
	        vid, NUM_VCORE);
	    err = -ENODEV;
	    goto done;
	}

	/* Allocate HC control block */
	phc_cb = (struct bcmvpcie_hc_cb*) kzalloc(sizeof(*phc_cb), GFP_KERNEL);
	if (!phc_cb) {
	    HCD_ERROR("[%d] vcore [%d] Unable to allocate memory for CB\r\n", phc->info.id, vid);
	    err =  -ENOMEM;
	    goto done;
	}
	HCD_INFO("[%d] vcore [%d] Allocated [0x%px] hc CB\r\n", phc->info.id, vid, phc_cb);

	/* Initialize hc elements */
	phc->phc_cb = (void*)phc_cb;
	phc->cfg.core_rev_offset = BCMVPCIE_HC_REV_OFFSET;
	phc->cfg.core_rev_mask = BCMVPCIE_HC_REV_MASK;

	/* initialize HC CB information */
	phc_cb->vid = vid;
	phc->info.base = (void __iomem *)bcmvpcie_hc_core_reg;

	/* Initialize  core resource element values for no device tree based
	 * legacy drivers
	 */
	err = bcmvpcie_hc_init_dev(phc);

	if (err) {
	    HCD_ERROR("vcore [%d] failed to initialize devices\n", vid);
	    goto done;
	}

	/* Update  core resource element values for device tree based drivers
	 */
	err = bcmvpcie_hc_parse_dt(phc);
	if (err) {
	    HCD_ERROR("vcore [%d] failed to parse DT\n", vid);
	}

done:
	HCD_FN_EXT();

	return err;
}

/*
 * Function bcmvpcie_hc_remove (phc)
 *
 *   Parameters:
 *    phc ... pointer to pcie core hc data structure
 *
 *   Description:
 *    PCIe hc driver remove - Free the allocated resources
 *
 *   Return: 0 on success, -ve on failure
 */
static void bcmvpcie_hc_remove(struct pcie_hc_core *phc)
{
	HCD_FN_ENT();

	if (phc->phc_cb) {
	    /* Free the control block */
	    kfree(phc->phc_cb);
	    phc->phc_cb = NULL;
	}

	HCD_FN_EXT();
	return;
}

/*
 * +-----------------------------------------------------
 *  Global Functions
 * +-----------------------------------------------------
 */
/*
 * Function pcie_vhc_plt_init (hc_cfg)
 *
 *   Parameters:
 *    hc_cfg ... pointer to pcie host controller configuration data structure
 *
 *   Description:
 *    fill the hc configuration
 *
 *   Return: 0 on success, -ve on failure
 */
int pcie_vhc_plt_init(struct pcie_hc_plt_cfg *hc_cfg)
{
	int ret = 0;

	HCD_FN_ENT();

	/* Sanity Check */
	if (hc_cfg == NULL) {
	    HCD_ERROR("bcmvpcie: NULL hc_cfg\n");
	    return -EINVAL;
	}

	/* fill the configuration items */
	hc_cfg->num_cores = NUM_VCORE;
	hc_cfg->flags = (HC_CFG_SKP_DT);

	/* setup the platform device names (for built-in, non-dt legacy) */
	if (hc_cfg->plt_dev) {
	    int core;
	    for (core = 0; core < NUM_VCORE; core++) {
	        hc_cfg->plt_dev[core].name = BCMVPCIE_HC_DEV_NAME;
	    }
	}

	/* PCI operations */
	hc_cfg->pci_fops = &bcmvpcie_hc_ops;

	/* initialize bcm963xx fops */
	hc_cfg->init_core = bcmvpcie_hc_probe;
	hc_cfg->free_core = bcmvpcie_hc_remove;
	hc_cfg->setup_rev = NULL;
	hc_cfg->read_reg = bcmvpcie_hc_read;
	hc_cfg->write_reg = bcmvpcie_hc_write;
	hc_cfg->config_core = bcmvpcie_hc_core_config;
	hc_cfg->unconfig_core = NULL;
	hc_cfg->set_power = bcmvpcie_hc_set_power;

	hc_cfg->reset_core = NULL;
	hc_cfg->is_linkup = NULL;
	hc_cfg->map_core_irq = bcmvpcie_hc_map_irq;

	hc_cfg->setup_msi = NULL;
	hc_cfg->teardown_msi = NULL;
	hc_cfg->get_msi = NULL;

	hc_cfg->setup_errint = NULL;
	hc_cfg->teardown_errint = NULL;


	HCD_FN_EXT();

	return ret;
}
