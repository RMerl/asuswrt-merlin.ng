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

/*
 * +-----------------------------------------------------
 *
 *  Defines
 *
 * +-----------------------------------------------------
 */
#define PCI_NUM_BAR_RESOURCES                       (PCI_STD_RESOURCE_END+1)
#define GEPD_NUM_SUP_DEV                            \
	(sizeof(pcie_gepd_sup_dev)/sizeof(unsigned short))
#define GEPD_DEBUG
#define GEPD_DRV_NAME                               "pcie-gepd"

/*
 * +-----------------------------------------------------
 *
 *  Macros
 *
 * +-----------------------------------------------------
 */
#if defined(GEPD_DEBUG)
#define GEPD_LOG(fmt, args...)                      \
	    printk(" %s: " fmt, GEPD_DRV_NAME, ##args)
#else
#define GEPD_LOG(fmt, args...)                      do {} while (0)
#endif

/*
 * +-----------------------------------------------------
 *
 *  Structures
 *
 * +-----------------------------------------------------
 */

typedef struct pcie_gepd_bar_res {
	phys_addr_t	phys;              /* Physical Address */
	unsigned long size;            /* bar size */
	volatile char *virt;           /* Virtual address */
} pcie_gepd_bar_res_t;

typedef struct pcie_gepd {
	pcie_gepd_bar_res_t bar[PCI_NUM_BAR_RESOURCES];
	int irq;
} pcie_gepd_t;

/*
 * +-----------------------------------------------------
 *
 *  Local Function prototype
 *
 * +-----------------------------------------------------
 */
static irqreturn_t pcie_gepd_isr(int irq, void *arg);
static int pcie_gepd_probe(struct pci_dev *pdev, const struct pci_device_id *ent);
static void pcie_gepd_remove(struct pci_dev *pdev);
static int __init pcie_gepd_init(void);
static void __exit pcie_gepd_exit(void);

/*
 * +-----------------------------------------------------
 *
 *  Global variables
 *
 * +-----------------------------------------------------
 */
static struct pci_device_id bcm_pcie_ep_devid[] = {
	{
	    vendor:      PCI_VENDOR_ID_BROADCOM,
	    device:      PCI_ANY_ID,
	    subvendor:   PCI_ANY_ID,
	    subdevice:   PCI_ANY_ID,
	    class:       PCI_CLASS_NETWORK_OTHER << 8,
	    class_mask:  0xffff00,
	    driver_data: 0
	},
	{ 0 }
};


static struct pci_driver pcie_gepdriver = {
	node:    {},
	name:    GEPD_DRV_NAME,
	id_table: bcm_pcie_ep_devid,
	probe:    pcie_gepd_probe,
	remove:   pcie_gepd_remove,
	suspend:  NULL,
	resume:   NULL,
};


static unsigned short pcie_gepd_sup_dev[] = {
	0x6315, /* 63158 EP */
	0x6317,	/* 63178 EP */
	0xf6ca, /* 63178 Internal WiFi 11ax 2x2 */
};

static struct pcie_gepd pcie_gepdriver_cb;


/*
 * +-----------------------------------------------------
 *
 *  Local Function prototype
 *
 * +-----------------------------------------------------
 */
/*
 *
 * Function pcie_gepd_isr (irq, arg)
 *
 *
 *   Parameters:
 *    irq ... interrupt number
 *    arg ... gepd control block
 *
 *   Description:
 *    This is dummy and not implemented yet
 *
 *  Return: IRQ_HANDLED
 */
static irqreturn_t pcie_gepd_isr(int irq, void *arg)
{
	/* Nothing to do, Just return success */
	return IRQ_HANDLED;
}

/*
 *
 * Function pcie_gepd_probe (pdev, ent)
 *
 *
 *   Parameters:
 *    pdev ... pointer to pcie device data structure
 *    ent  ... pcie device id table of driver
 *
 *   Description:
 *    Check if the device is supported by the driver. Allocate all BAR
 *    resources corresponding to the device
 *
 *  Return: 0 on success, -ve on failure
 */
static int pcie_gepd_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	int ret = 0;
	int res, i;
	int found = 0;
	struct pcie_gepd *epd = &pcie_gepdriver_cb;

	GEPD_LOG("%s\r\n", __FUNCTION__);

	/* Check if the pcie device is supported by the driver */
	for (i = 0; i < GEPD_NUM_SUP_DEV; i++) {
	    if (pcie_gepd_sup_dev[i] == pdev->device) {
	        found++;
	        GEPD_LOG("found supported device 0x%x\r\n", pdev->device);
	    }
	}

	/* Not supported */
	if (found == 0) return -ENODEV;

	/* Allocate HCD control block */
	epd = kzalloc(sizeof(*epd), GFP_KERNEL);
	if (!epd) {
	    GEPD_LOG("Unable to allocate memory for CB\r\n");
	    return -ENOMEM;
	}
	GEPD_LOG("Allocated [0x%p]\r\n", epd);

	/* Initialize the pcie data strucutre */
	pci_set_drvdata(pdev, epd);

	/* Allocate all available BAR resources */
	for (res = 0; res < PCI_NUM_BAR_RESOURCES; res++) {
	    epd->bar[res].phys = pci_resource_start(pdev, res);
	    epd->bar[res].size = pci_resource_len(pdev, res);

	    if (epd->bar[res].phys && epd->bar[res].size) {
	        GEPD_LOG("Found bar [%d] size 0x%lx\r\n", res, epd->bar[res].size);
	        epd->bar[res].virt = (volatile char*)ioremap_nocache(
	            (unsigned long)epd->bar[res].phys,
	            (unsigned long)epd->bar[res].size);
	        if (epd->bar[res].virt == NULL) {
	            GEPD_LOG("bar [%d] mapping failed\r\n", res);
	            ret = -ENODEV;
	            break;
	        }
	        GEPD_LOG("bar %d, phys 0x%08x size 0x%lx, virt 0x%p\r\n",
	            res, epd->bar[res].phys, epd->bar[res].size, epd->bar[res].virt);
	    }
	}

	if (request_irq(pdev->irq, pcie_gepd_isr, IRQF_SHARED, GEPD_DRV_NAME, epd) < 0) {
	    GEPD_LOG("request_irq(%d) failed\n", pdev->irq);
	} else {
	    epd->irq = pdev->irq;
	}

	/* Enable device */
	pci_restore_state(pdev);
	ret = pci_enable_device(pdev);
	if (!ret) pci_set_master(pdev);

	return ret;
}

/*
 *
 * Function pcie_gepd_remove (pdev)
 *
 *
 *   Parameters:
 *    pdev ... pointer to pci device  data structure
 *
 *   Description:
 *    disable the pcie device and release all resources corresponding to the device
 *
 *  Return: 0 on success, -ve on failure
 */
static void pcie_gepd_remove(struct pci_dev *pdev)
{
	struct pcie_gepd *epd;
	int res;

	GEPD_LOG("%s\r\n", __FUNCTION__);

	epd = pci_get_drvdata(pdev);

	if (epd) {
	    pci_disable_device(pdev);

	    /* Free BAR resources */
	    for (res = 0; res < PCI_NUM_BAR_RESOURCES; res++) {
	        if (epd->bar[res].virt) {
	            iounmap(epd->bar[res].virt);
	        }
	    }

	    /* free irq */
	    if (epd->irq != 0) {
	        free_irq(epd->irq, epd);
	        epd->irq = 0;
	    }
	    kfree(epd);
	}

	return;
}


/*
 * +-----------------------------------------------------
 *  Global Functions
 * +-----------------------------------------------------
 */
/*
 *
 * Function pcie_gepd_init ()
 *
 *
 *   Parameters:
 *
 *   Description:
 *    Generic PCIe End Point driver init, register the driver to PCI stack
 *    This inturn should call our probe()
 *
 *  Return: 0 on success, -ve on failure
 */
static int __init pcie_gepd_init(void)
{
	int ret = -1;

	GEPD_LOG("%s\r\n", __FUNCTION__);

	ret = pci_register_driver(&pcie_gepdriver);

	GEPD_LOG("register_driver returned %d\r\n", ret);

	return ret;
}

/*
 *
 * Function pcie_gepd_exit ()
 *
 *
 *   Parameters:
 *
 *   Description:
 *    Generic PCIe End Point driver exit, unregister the driver from  pci core
 *    This inturn should call our remove()
 *
 *  Return: None
 */
static void __exit pcie_gepd_exit(void)
{
	GEPD_LOG("%s\r\n", __FUNCTION__);

	pci_unregister_driver(&pcie_gepdriver);

	return;
}

module_init(pcie_gepd_init);
module_exit(pcie_gepd_exit);
MODULE_LICENSE("GPL");
