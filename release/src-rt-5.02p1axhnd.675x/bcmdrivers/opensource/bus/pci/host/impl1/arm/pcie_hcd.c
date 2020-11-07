#if defined(CONFIG_BCM_KF_ARM_BCM963XX)
/*
    Copyright (c) 2019 Broadcom
    All Rights Reserved

    <:label-BRCM:2019:DUAL/GPL:standard
    
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
#include <linux/proc_fs.h>

#include <pcie_common.h>
#include <pcie-bcm963xx.h>
#include <pcie-vcore.h>
#include <pcie_hcd.h>

/*
 * +-----------------------------------------------------
 *
 *  Defines
 *
 * +-----------------------------------------------------
 */
#define NUM_PCIE_CORES                             (NUM_CORE + NUM_VCORE)

/*
 * +-----------------------------------------------------
 *
 *  Macros
 *
 * +-----------------------------------------------------
 */

/*
 * +-----------------------------------------------------
 *
 *  Structures
 *
 * +-----------------------------------------------------
 */
/**
 * PCIe Host Controller Driver Procfs control block
 * @dir:      pcie_hcd procfs directory file pointer
 * @coreinfo: pcie_hcd coreinfo procfs file pointer
 * @pdev:     Host Controller Driver CB array
 */
struct pcie_hcd_procfs {
	struct proc_dir_entry      *dir;
	struct proc_dir_entry      *coreinfo;
	struct bcm963xx_pcie_hcd   *pdrv[NUM_PCIE_CORES];
};

/*
 * +-----------------------------------------------------
 *
 *  Local Function prototype
 *
 * +-----------------------------------------------------
 */
static ssize_t pcie_hcd_procfs_coreinfo_read(struct file *filep,
	char __user *page, size_t count, loff_t *data);

/*
 * +-----------------------------------------------------
 *
 *  external Function prototype
 *
 * +-----------------------------------------------------
 */

/*
 * +-----------------------------------------------------
 *
 *  Global variables
 *
 * +-----------------------------------------------------
 */

static const struct file_operations pcie_hcd_procfs_coreinfo_fops = {
	.owner  = THIS_MODULE,
	.read   = pcie_hcd_procfs_coreinfo_read,
};

static struct pcie_hcd_procfs pcie_hcd_procfs_cb;

/*
 * +-----------------------------------------------------
 *
 *  Local inline functions
 *
 * +-----------------------------------------------------
 */

/*
 * +-----------------------------------------------------
 *
 *  Local Functions
 *
 * +-----------------------------------------------------
 */


/*
 *
 * Function pcie_hcd_procfs_coreinfo_read (filep, page, count, data)
 *
 *
 *   Parameters:
 * filep ... pointer to pcie core hcd data structure
 * page  ...
 * count ...
 * data  ...
 *
 *   Description:
 *    Read function for 'coreinfo' proc file
 *
 *  Return: length of information on success, 0 on failure/no information
 */
static ssize_t pcie_hcd_procfs_coreinfo_read(struct file *filep,
	char __user *page, size_t count, loff_t *data)
{
	int core;
	struct pcie_hcd_procfs *pfs_cb = &pcie_hcd_procfs_cb;
	int len = 0;

	if (*data != 0)
	    return 0; //indicate EOF

	len += sprintf((page+len), "c: dmn \n");
	for (core = 0; core < NUM_PCIE_CORES; core++) {

	    if (pfs_cb->pdrv[core] != NULL) {
	        len += sprintf((page+len), "%d: %04d\n", core,
	            pci_domain_nr(pfs_cb->pdrv[core]->bus));
	    }
	}

	return len;
}

/*
 * +-----------------------------------------------------
 *  Global Functions
 * +-----------------------------------------------------
 */
/*
 *
 * Function pcie_hcd_procfs_init (pdrv)
 *
 *
 *   Parameters:
 * pdrv ... pointer to pcie core hcd data structure
 *
 *   Description:
 *    PCIe hcd driver proc file system initialization
 *
 *  Return: 0 on success, -ve on failure
 */
int pcie_hcd_procfs_init(struct bcm963xx_pcie_hcd *pdrv)
{
	struct pcie_hcd_procfs *pfs_cb = &pcie_hcd_procfs_cb;

	pfs_cb->pdrv[pdrv->core_id] = pdrv;
	pdrv->pfs = pfs_cb;

	if (pdrv->pfs->dir) {
	    /* Already created, Nothing to do */
	    return 0;
	}

	/* make a directory in /proc if doesn't exist */
	pfs_cb->dir = proc_mkdir("pcie_hcd", NULL);

	if (pfs_cb->dir == NULL) {
	    HCD_ERROR("Core [%d] Unable to create pcie_hcd proc directory\n", pdrv->core_id);
	    return -1;
	}

	pfs_cb->coreinfo = proc_create("coreinfo", 0644, pfs_cb->dir,
	    &pcie_hcd_procfs_coreinfo_fops);
	if (pfs_cb->coreinfo == NULL) {
	    HCD_ERROR("Core [%d] Unable to create pcie_hcd/coreinfo proc file\n", pdrv->core_id);
	    return -1;
	}

	return 0;
}

/*
 *
 * Function pcie_hcd_procfs_deinit (pdrv)
 *
 *
 *   Parameters:
 * pdrv ... pointer to pcie core hcd data structure
 *
 *   Description:
 *    PCIe hcd driver proc file system de-initialization
 *
 *  Return: 0 on success, -ve on failure
 */
void pcie_hcd_procfs_deinit(struct bcm963xx_pcie_hcd *pdrv)
{
	struct pcie_hcd_procfs *pfs_cb = NULL;
	int core;

	if (pdrv) pfs_cb = pdrv->pfs;

	if (pfs_cb == NULL) {
	    /* Nothing to do */
	    return;
	}

	pfs_cb->pdrv[pdrv->core_id] = NULL;

	for (core = 0; core < NUM_PCIE_CORES; core++) {
	    if (pfs_cb->pdrv[core]) {
	        /* There are some cores still running */
	        return;
	    }
	}

	if (pfs_cb->coreinfo != NULL) {
	    remove_proc_entry("coreinfo", pfs_cb->dir);
	    pfs_cb->coreinfo = NULL;
	}

	if (pfs_cb->dir != NULL) {
	    remove_proc_entry("pcie_hcd", NULL);
	    pfs_cb->dir = NULL;
	}

	return;
}
#endif /* CONFIG_BCM_KF_ARM_BCM963XX */
