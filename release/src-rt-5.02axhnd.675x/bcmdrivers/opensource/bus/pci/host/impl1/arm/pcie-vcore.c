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
#include <pmc_drv.h>
#include <shared_utils.h>

#include <pcie_hcd.h>
#include <pcie_common.h>
#include <pcie-bcm963xx.h>
#include <pcie-vcore.h>
#include <pcie-vdev.h>

#include <pmc_wlan.h>

/*
 * +-----------------------------------------------------
 *
 *  Defines
 *
 * +-----------------------------------------------------
 */

/*
 * +-----------------------------------------------------
 * Virtual device
 * +-----------------------------------------------------
 */


/*
 * +-----------------------------------------------------
 *
 *  Macros
 *
 * +-----------------------------------------------------
 */
#define PCIE_CORE2VCORE(core)                     \
	(((core) >= NUM_CORE) ? ((core) - NUM_CORE) : (core))

/*
 * +-----------------------------------------------------
 *
 *  Structures
 *
 * +-----------------------------------------------------
 */

/*
 * PCIe Vitual Device control block
 *
 * @cfg         : virtual device configuration
 * @owin        : bar window resource
 * @owin_inited : flag for window initialized or not
 * @irq         : device irq (if present)
 * @enabled     : flag to specify powered and enabled
 */
struct pcie_vdev {
	struct pcie_vdev_cfg cfg;
	struct resource      owin;
	bool                 owin_inited;
	bool                 enabled;
	int                  apon;
	int                  irq;
};

/*
 * PCIe Vitual Core control block
 *
 * @drv      : PCIe core driver control block
 * @num_dev  : number of virtual devices per core
 * @dev      : virual device control block
 */
struct pcie_vcore {
	struct bcm963xx_pcie_hcd drv;       /* This should be at the beginning */
	int                      num_dev;
	struct pcie_vdev         dev[MAX_NUM_VDEV];
};

/*
 * +-----------------------------------------------------
 *
 *  Local Function prototype
 *
 * +-----------------------------------------------------
 */
static int pcie_vcore_config_read(struct pci_bus *bus, unsigned int devfn,
	int where, int size, u32 *val);
static int pcie_vcore_config_write(struct pci_bus *bus, unsigned int devfn,
	int where, int size, u32 val);

static int pcie_vcore_setup_res(struct pcie_vcore *pvcore,
	struct list_head *resources);
static void pcie_vcore_unmap_res(struct pcie_vcore *pvcore);
static int pcie_vcore_parse_dt(struct pcie_vcore *pvcore);
static int pcie_vcore_init_dev(struct pcie_vcore *pvcore);

/*
 * +-----------------------------------------------------
 *
 *  external Function prototype
 *
 * +-----------------------------------------------------
 */
int bcm963xx_pcie_map_irq(const struct pci_dev *pcidev, u8 slot,
	u8 pin);

/*
 * +-----------------------------------------------------
 *
 *  Global variables
 *
 * +-----------------------------------------------------
 */

static struct pci_ops pcie_vcore_ops = {
	.read = pcie_vcore_config_read,
	.write = pcie_vcore_config_write,
};

/*
 * +-----------------------------------------------------
 *
 *  Local inline functions
 *
 * +-----------------------------------------------------
 */
static inline bool pcie_vcore_access_valid(struct pcie_vcore *pvcore,
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
	for (idx = 0; idx < pvcore->num_dev; idx++) {
	    if ((pvcore->dev[idx].enabled == TRUE) &&
	        (bus == pvcore->dev[idx].cfg.bus)) {
	        valid = (dev == pvcore->dev[idx].cfg.slot);
	    }
	    if (valid == TRUE) break;
	}

	/* Update the device index if available */
	if (pidx) *pidx = idx;

	HCD_FN_EXT();
	return valid;
}

static inline int pcie_vcore_pwrup_dev(struct pcie_vcore *pvcore, int devidx)
{
	int rc;

	pvcore->dev[devidx].enabled = FALSE;
	rc = pmc_wlan_power_up(pvcore->drv.core_id*pvcore->num_dev + devidx);
	if (rc == kPMC_NO_ERROR) {
	    pvcore->dev[devidx].enabled = TRUE;
	    HCD_INFO("vcore [%d] vdev [%d] powered up\n", pvcore->drv.core_id, devidx);
	} else {
	    HCD_ERROR("vcore [%d] vdev [%d] failed to power up rc [%d]\n",
	        pvcore->drv.core_id, devidx, rc);
	}

	return rc;
}

static inline void pcie_vcore_pwrdn_dev(struct pcie_vcore *pvcore, int devidx)
{
	pmc_wlan_power_down(pvcore->drv.core_id*pvcore->num_dev + devidx);
	pvcore->dev[devidx].enabled = FALSE;
	HCD_LOG("vcore [%d] vdev [%d] powered down\n", pvcore->drv.core_id, devidx);

	return;
}

/*
 * +-----------------------------------------------------
 *
 *  Local Functions
 *
 * +-----------------------------------------------------
 */

/*
 *
 * Function pcie_vcore_config_read (bus, devfn, where, size, val)
 *
 *
 *   Parameters:
 *    bus ... pointer to pci bus data structure
 *    devfn ... pci device, function mapping
 *    where ... offset from the device base
 *    size .... access size
 *    val .... pointer to read data to be written
 *
 *   Description:
 *     Check and write to virtual device first, if not, go back to external device
 *
 *  Return: PCIBIOS_DEVICE: SUCCESS or DEVICE_NOT_FOUND on failure
 */
static int pcie_vcore_config_read(struct pci_bus *bus, unsigned int devfn,
	int where, int size, u32 *val)
{
	struct pcie_vcore *pvcore = bus->sysdata;
	int slot = PCI_SLOT(devfn);
	int ret = PCIBIOS_DEVICE_NOT_FOUND;
	ulong addr = 0;
	uint devidx = 0;

	HCD_FN_ENT();

	if (pcie_vcore_access_valid(pvcore, bus->number, slot, PCI_FUNC(devfn), &devidx) == FALSE) {
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

	addr =  (ulong)(&pvcore->dev[devidx].cfg) + where;

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
	        *val = ~(pvcore->dev[devidx].cfg.bar_size[(where - PCI_BASE_ADDRESS_0)/4]-1);
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
 *
 * Function pcie_vcore_config_write (bus, devfn, where, size, val)
 *
 *
 *   Parameters:
 *    bus ... pointer to pci bus data structure
 *    devfn ... pci device, function mapping
 *    where ... offset from the device base
 *    size .... access size
 *    val .... value to be written
 *
 *   Description:
 *     Check and write to virtual device first, if not, go back to external device
 *
 *  Return: PCIBIOS_DEVICE: SUCCESS or DEVICE_NOT_FOUND on failure
 */
static int pcie_vcore_config_write(struct pci_bus *bus, unsigned int devfn,
	int where, int size, u32 val)
{
	struct pcie_vcore *pvcore = bus->sysdata;
	int slot = PCI_SLOT(devfn);
	int ret = PCIBIOS_DEVICE_NOT_FOUND;
	uint8* addr;
	uint devidx = 0;

	HCD_FN_ENT();

	if (pcie_vcore_access_valid(pvcore, bus->number, slot, PCI_FUNC(devfn), &devidx) == FALSE) {
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

	addr =  (uint8*)(&pvcore->dev[devidx].cfg) + where;

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
 *
 * Function pcie_vcore_setup_res (pvcore, resources)
 *
 *
 *   Parameters:
 *	  pvcore    ... pointer to pcie core hcd data structure
 *    resources ... pcie core resources
 *
 *   Description:
 *	   Map window memory resources
 *
 *  Return: 0 on success, -ve on failure
 */
static int pcie_vcore_setup_res(struct pcie_vcore *pvcore,
	struct list_head *resources)
{
	struct pcie_vdev *pvdev;
	struct device *dev;
	int idx;
	int err = 0;

	HCD_FN_ENT();

	dev = &pvcore->drv.pdev->dev;

	for (idx = 0; idx < pvcore->num_dev; idx++) {
	    pvdev = &pvcore->dev[idx];
	    if (pvdev->enabled == FALSE) {
	        continue;
	    }
	    err =  devm_request_resource(dev, &iomem_resource, &pvdev->owin);
	    if (err) {
	        HCD_ERROR("vcore [%d] dev [%d] pcie failed to create owin resource: [%d]\r\n",
	            pvcore->drv.core_id, idx, err);
	        HCD_FN_EXT();
	        return err;
	    } else {
	        HCD_INFO("vcore [%d] dev [%d] mapped pcie owin base [0x%llx]\r\n",
	            pvcore->drv.core_id, idx, (u64)pvdev->owin.start);
	    }

	    pci_add_resource_offset(resources, &pvdev->owin, 0);

	    pvdev->owin_inited = TRUE;
	}

	HCD_FN_EXT();

	return err;
}

/*
 *
 * Function pcie_vcore_unmap_res (pvcore)
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
static void pcie_vcore_unmap_res(struct pcie_vcore *pvcore)
{
	struct pcie_vdev *pvdev;
	int idx;

	HCD_FN_ENT();

	if (pvcore == NULL)
	    return;

	for (idx = 0; idx < pvcore->num_dev; idx++) {
	    pvdev = &pvcore->dev[idx];

	    if (pvdev->owin_inited == TRUE) {
	        devm_release_resource(&(pvcore->drv.pdev->dev), &pvdev->owin);
	        HCD_INFO("vcore [%d] vdev [%d] release owin [0x%llx]\r\n",
	            pvcore->drv.core_id, idx, (u64)pvdev->owin.start);
	    }

	    pvdev->owin_inited = FALSE;
	}

	return;
}

/*
 *
 * Function pcie_vcore_parse_dt (pdrv)
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
static int pcie_vcore_parse_dt(struct pcie_vcore *pvcore)
{
	struct bcm963xx_pcie_hcd *pdrv = &pvcore->drv;
	struct device_node *np = NULL;
	int devidx;

	HCD_FN_ENT();

	/* Initialize attributes from device tree if present */
	np = pdrv->pdev->dev.of_node;
	if (np) {
	    HCD_LOG("[%d] DT node available: %s\n", pdrv->core_id, np->full_name);

	    /* Check if DT entry status is enabled or okay to load */
	    if (!of_device_is_available(np)) {
	        dev_err(&pdrv->pdev->dev, "DT status disabled\n");
	        return -ENODEV;
	    }

	    /* Update the WLAN device register base and size */
	    for (devidx = 0; devidx < MAX_NUM_VDEV; devidx++) {
	        struct resource base;
	        struct pcie_vdev *pvdev = &pvcore->dev[devidx];

	        /* WIFI core register base */
	        if (of_address_to_resource(np, devidx, &base))
	            break;

	        pvdev->cfg.bar_size[0] = base.end - base.start + 1;
	        pvdev->cfg.regs.bar_1 = base.start | PCI_BASE_ADDRESS_MEM_TYPE_64;

	        if (pvdev->cfg.bar_size[0]) {
	            pvdev->owin.name = dev_name(&pvcore->drv.pdev->dev);
	            pvdev->owin.start = pvdev->cfg.regs.bar_1 & PCI_BASE_ADDRESS_MEM_MASK;
	            pvdev->owin.end = pvdev->owin.start + pvdev->cfg.bar_size[0] - 1;
	            pvdev->owin.flags = IORESOURCE_MEM;
	        }

	        {
	            u8 dt_val;
	            int err = 0;

	            /* PCIe force power on setting */
	            err = of_property_read_u8(np, "brcm,apon", &dt_val);
	            if (err == 0) {
	                pdrv->hc_cfg.apon = dt_val;
	            }
	            if (err) {
	                HCD_WARN("No DT entry for apon, using defaults\n");
	            } else {
	                HCD_INFO("Updating apon settings from DT entry\n");
	            }
	        }

	        HCD_INFO("vcore [%d] vdev [%d] device_vendor [0x%x] int [%d]\r\n",
	            pvcore->drv.core_id, devidx, pvdev->cfg.regs.device_vendor_id, pvdev->irq);

	        HCD_INFO("vcore [%d] vdev [%d] owin: start [0x%llx] end [0x%llx] flags [0x%lx]\r\n",
	            pvcore->drv.core_id, devidx, (u64)pvdev->owin.start, (u64)pvdev->owin.end,
	            pvdev->owin.flags);
	    }
	}

	HCD_FN_EXT();

	return 0;
}


/*
 *
 * Function pcie_vcore_init_dev (pvcore)
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
static int pcie_vcore_init_dev(struct pcie_vcore *pvcore)
{
	struct pcie_vdev *pvdev = NULL;
	struct pcie_vdev_cfg *pvcfg;
	int id;
	int devidx = 0;

	HCD_FN_ENT();

	/* Fill the virtual device config space with default values */
	id = pvcore->drv.core_id;

	/* Get the virtual device configuration. Hierarchy
	 * - nvram full configruation (TODO: pcie_vdev%d_cfg_path)
	 * - built-in chip sepcific configuration
	 * - Default built-in configuration
	 * - nvram specific configuration parameters
	 */
	if (id < NUM_PCIE_VDEV_CFG) {
	     /* copy built-in chip sepcific configuration */
	    pvcfg = pcie_vdev_cfg_list[id].pcfg;
	    pvcore->num_dev = pcie_vdev_cfg_list[id].size;
	} else {
	    /* Chip specific info not available, Copy Default built-in configuration */
	    HCD_ERROR("vcore [%d] default configuration not available\r\n", id);
	    pvcfg = (struct pcie_vdev_cfg*)pcie_wifidefdev_cfg;
	    pvcore->num_dev = sizeof(pcie_wifidefdev_cfg)/sizeof(struct pcie_vdev_cfg);
	}

	HCD_INFO("vcore [%d] found with [%d] device(s)\r\n",
	    pvcore->drv.core_id, pvcore->num_dev);

	for (devidx = 0; devidx < pvcore->num_dev; devidx++) {
	    pvdev = &pvcore->dev[devidx];

	    HCD_INFO("vcore [%d] vdev [%d] pvdev [0x%px] pvcfg [0x%px]\r\n",
	        pvcore->drv.core_id, devidx, pvdev, pvcfg);

	    if (pvcfg) {
	        memcpy(&pvdev->cfg, pvcfg, sizeof(struct pcie_vdev_cfg));
	        pvcfg++;
	    }

	    if (pcie_vcore_pwrup_dev(pvcore, devidx) != 0) {
	        HCD_ERROR("vcore [%d] vdev [%d] failed to power up, skipping initialization\n",
	            pvcore->drv.core_id, devidx);
	        continue;
	    }

	    /* If bar0 is not specified, fill with default */
	    if (pvdev->cfg.bar_size[0] == 0) {
	        switch (devidx) {
	            case 0:
	                pvdev->cfg.bar_size[0] = WLAN0_SIZE;
	                pvdev->cfg.regs.bar_1 = WLAN0_PHYS_BASE | PCI_BASE_ADDRESS_MEM_TYPE_64;
	            break;

#if defined(WLAN1_PHYS_BASE)
	            case 1:
	                pvdev->cfg.bar_size[0] = WLAN1_SIZE;
	                pvdev->cfg.regs.bar_1 = WLAN1_PHYS_BASE | PCI_BASE_ADDRESS_MEM_TYPE_64;
	            break;
#endif /* WLAN1_PHYS_BASE */

	            default:
	                HCD_ERROR("vcore [%d] vdev [%d] not supported\n",
	                    pvcore->drv.core_id, devidx);
	                return -ENODEV;
	        }
	    }

	    /* If interrupt is not specified, fill with default */
	    if (pvdev->cfg.regs.cfg3c.int_line == 0) {
	        switch (pvcore->drv.core_id) {
	            case 0:
	                pvdev->cfg.regs.cfg3c.int_line = INTERRUPT_ID_WLAN0_D11MAC;
	            break;

#if defined(WLAN1_PHYS_BASE)
	            case 1:
	                pvdev->cfg.regs.cfg3c.int_line = INTERRUPT_ID_WLAN1_D11MAC;
	            break;
#endif /* WLAN1_PHYS_BASE */

	            default:
	                HCD_ERROR("vcore [%d] vdev [%d] not supported\n",
	                    pvcore->drv.core_id, devidx);
	                return -ENODEV;
	        }
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

	    if (pvdev->cfg.bar_size[0]) {
	        pvdev->owin.name = dev_name(&pvcore->drv.pdev->dev);
	        pvdev->owin.start = pvdev->cfg.regs.bar_1 & PCI_BASE_ADDRESS_MEM_MASK;
	        pvdev->owin.end = pvdev->owin.start + pvdev->cfg.bar_size[0] - 1;
	        pvdev->owin.flags = IORESOURCE_MEM;
	    }

	    /* PCI_INTERRUPT_PIN */
	    pvdev->irq = pvdev->cfg.regs.cfg3c.int_line;

	    HCD_INFO("vcore [%d] vdev [%d] device_vendor [0x%x] int [%d]\r\n",
	        pvcore->drv.core_id, devidx, pvdev->cfg.regs.device_vendor_id, pvdev->irq);

	    HCD_INFO("vcore [%d] vdev [%d] owin: start [0x%llx] end [0x%llx] flags [0x%lx]\r\n",
	        pvcore->drv.core_id, devidx, (u64)pvdev->owin.start, (u64)pvdev->owin.end,
	        pvdev->owin.flags);
	}

	HCD_FN_EXT();

	return 0;
}

/*
 * +-----------------------------------------------------
 *  Global Functions
 * +-----------------------------------------------------
 */
/*
 *
 * Function pcie_vcore_map_irq (pcidev, slot, pin)
 *
 *
 *   Parameters:
 *    pcidev ... pointer to pci device data structure
 *    slot ... pci slot/device id
 *    pin ... pin number (not used)
 *
 *   Description:
 *       Get the pcie core irq number.
 *
 *
 *  Return: pcie core irq number
 */
int pcie_vcore_map_irq(const struct pci_dev *pcidev,
	u8 slot, u8 pin)
{
	struct pcie_vcore *pvcore = pcidev->bus->sysdata;
	int idx;
	int irq = -1;

	HCD_FN_ENT();

	HCD_INFO("dev [0x%px] slot [%d] pin [%d]\r\n",
	    pcidev, slot, pin);

	if (IS_PCIE_VCORE(pvcore->drv.pdev->id)) {
	    for (idx = 0; idx < pvcore->num_dev; idx++) {
	        if (slot == pvcore->dev[idx].cfg.slot) {
	            irq = pvcore->dev[idx].irq;
	            break;
	        }
	    }
	}

	HCD_FN_EXT();
	return irq;
}

/*
 *
 * Function pcie_vcore_probe (pdev)
 *
 *
 *   Parameters:
 *    pdev ... pointer to platform device
 *
 *   Description:
 *    PCIe hcd driver virtual core probe. Called for each instance of the PCIe virtual core.
 *    Get and allocate resource, configure virtual hardware, start the PCI bus and
 *    enumerate PCI devices
 *
 *  Return: 0 on success, -ve on failure
 */
int pcie_vcore_probe(struct platform_device *pdev)
{
	struct pcie_vcore *pvcore = NULL;
	struct bcm963xx_pcie_hcd *pdrv = NULL;
	struct device_node *np = pdev->dev.of_node;
	uint32 core = (uint32)pdev->id;
	int	err = 0;
	int devidx;
	bool apon_off = TRUE;
	struct pci_bus *bus;
	LIST_HEAD(res);

	HCD_FN_ENT();

	/* If coming from device tree, use device tree entry to find the core id */
	if (np && (pdev->id <= PLATFORM_DEVID_NONE)) {
	    if (of_property_read_u32(np, "brcm,coreid", &core) < 0) {
	        HCD_ERROR("Unable to get coreid from device tree\r\n");
	        return -ENODEV;
	    }
		pdev->id = core;
	}

	HCD_INFO("core [%d] probe\r\n", core);

	/* Convert the core index to virtual core index */
	core = PCIE_CORE2VCORE(core);

	if (core >= NUM_VCORE) {
	    HCD_ERROR("vcore [%d] exceeds max vcores [%d]\r\n",
	        core, NUM_VCORE);
	    return -ENODEV;
	}

	HCD_LOG("found core [%d] vcore [%d] \r\n", pdev->id, core);

	/* Allocate HCD control block */
	pvcore = kzalloc(sizeof(*pvcore), GFP_KERNEL);
	if (!pvcore) {
	    HCD_ERROR("vcore [%d] Unable to allocate memory for CB\r\n", core);
	    err =  -ENOMEM;
	    goto error;
	}
	HCD_INFO("vcore [%d] Allocated [0x%px] hcd\r\n", core, pvcore);

	pdrv = &pvcore->drv;

	/* PCIe port configuration */
	/* Use the platform device id as core_id */
	pdrv->core_id = pdev->id;
	bcm963xx_pcie_init_hc_cfg(&pvcore->drv);
	pcie_hcd_procfs_init(&pvcore->drv);

	/* Initialize  hcd elements */
	pdrv->core_id = core;
	pdrv->pdev = pdev;
	platform_set_drvdata(pdev, pdrv);

	/* Initialize  core resource element values for no device tree based
	 * legacy drivers
	 */
	pcie_vcore_init_dev(pvcore);


	/* Update core resource elements  (DT based) */
	err = pcie_vcore_parse_dt(pvcore);
	if (err) {
	    HCD_ERROR("vcore [%d] failed to update dt entries\r\n", core);
	    err =  -EINVAL;
	    goto error;
	}

	/* if Configured, skip port enumeration and power off the port */
	for (devidx = 0; devidx < pvcore->num_dev; devidx++) {
	    pvcore->dev[devidx].apon = pvcore->drv.hc_cfg.apon;
	    pvcore->dev[devidx].apon >>= (devidx * HCD_APON_SHIFT);
	    pvcore->dev[devidx].apon &= HCD_APON_MASK;
	    if (pvcore->dev[devidx].apon != HCD_APON_OFF)
	        apon_off = FALSE;
	}

	if (apon_off == TRUE) {
	    HCD_LOG("Skip vcore [%d] due to apon setting [0x%x]\r\n",
	        core, pvcore->drv.hc_cfg.apon);
	    err = -ENODEV;
	    goto error;
	}

	/* if Configured, skip device enumeration and power off the port */
	/* Force power off the internal WiFi devices */
	for (devidx = 0; devidx < pvcore->num_dev; devidx++) {
	    if ((pvcore->dev[devidx].enabled == TRUE) &&
	        (pvcore->dev[devidx].apon >= HCD_APON_OFF)) {
	         /* OFF, OFF_WITH_DOMAIN */
	         pcie_vcore_pwrdn_dev(pvcore, devidx);
	         HCD_LOG("vcore [%d] vdev [%d] powered down due to apon setting [0x%x]\r\n",
	            core, devidx, pvcore->drv.hc_cfg.apon);
	    }
	}

	/* Setup PCIe core memory window */
	err = pcie_vcore_setup_res(pvcore, &res);
	if (err) {
	    HCD_ERROR("vcore [%d] failed to setup owin resource, err [%d]\r\n",
	        core, err);
	    err =  -ENOMEM;
	    goto error;
	}

	HCD_LOG("vcore [%d] initialized with [%d] device%s\r\n",
	    core, pvcore->num_dev, (pvcore->num_dev > 1) ? "s" : "");

	/* Now do the PCI setup,
	 * - create,scan bus
	 * - assign resources, irq
	 * - add connected devices
	 */
	bus = pci_create_root_bus(&pdev->dev, pvcore->dev[0].cfg.bus,
	    &pcie_vcore_ops, pdrv, &res);
	if (!bus) {
	    HCD_ERROR("vcore [%d] failed to create root bus: %d\r\n", core, err);
	    err =  -ENXIO;
	    goto error;
	}

	/* store the bus for proper remove */
	pdrv->bus = bus;

	pci_scan_child_bus(bus);

	pci_assign_unassigned_bus_resources(bus);

	pci_bus_add_devices(bus);

	err = 0;

error:

	if (err) {
	    pcie_vcore_remove(pdev);
	}

	pci_free_resource_list(&res);

	HCD_FN_EXT();

	return err;
}

/*
 *
 * Function pcie_vcore_remove (pdrv)
 *
 *
 *   Parameters:
 *    pdrv ... pointer to pcie core hcd data structure
 *
 *   Description:
 *    Free the virtual pcie core resources
 *
 *  Return: 0 on success, -ve on failure
 */
int  pcie_vcore_remove(struct platform_device *pdev)
{
	struct pcie_vcore *pvcore = platform_get_drvdata(pdev);
	int devidx;

	HCD_FN_ENT();

	if (!pvcore) {
	    HCD_FN_EXT();
	    return 0;
	}

	/* Release the bus */
	if (pvcore->drv.bus) {
	    pci_stop_root_bus(pvcore->drv.bus);
	    pci_remove_root_bus(pvcore->drv.bus);
	}

	/* Release the bar resources */
	pcie_vcore_unmap_res(pvcore);

	/* power down the internal WiFi devices */
	for (devidx = 0; devidx < pvcore->num_dev; devidx++) {
	    if ((pvcore->dev[devidx].enabled == TRUE) &&
	        (pvcore->drv.hc_cfg.apon != HCD_APON_ON)) {
	        pcie_vcore_pwrdn_dev(pvcore, devidx);
	    }
	}

	/* Put back the global core id */
	pvcore->drv.core_id = pvcore->drv.pdev->id;
	pcie_hcd_procfs_deinit(&pvcore->drv);

	/* Free the control block */
	kfree(pvcore);

	HCD_FN_EXT();

	return 0;
}
