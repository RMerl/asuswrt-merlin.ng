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

#if defined(CONFIG_BRCM_IKOS) || defined(CONFIG_BCM_PCIE_PMC_BRD_STUBS)
#define USE_PMC_BRD_STUBS
#else /* !CONFIG_BRCM_IKOS && !CONFIG_BCM_PCIE_PMC_BRD_STUBS */
#include <board.h>
#include <pmc_pcie.h>
#include <pmc_drv.h>
#endif /* !CONFIG_BRCM_IKOS && !CONFIG_BCM_PCIE_PMC_BRD_STUBS */
#include <shared_utils.h>

#include <pcie_hcd.h>
#include <pcie_common.h>
#include <pcie-bcm963xx.h>
#include <pcie-vcore.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 18, 0))
#include "../drivers/pci/pci.h"
#endif

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
 * PCI - PCIE
 * +-----------------------------------------------------
 */
#define PCI_EXP_DEVCAP_PAYLOAD_512B                2
#define PCI_EXP_DEVCTL_PAYLOAD_512B                (2 << 5)

/*
 * +-----------------------------------------------------
 * HCD Driver
 * +-----------------------------------------------------
 */
#define BCM963XX_ROOT_BUSNUM                       0x00
#define BCM963XX_MAX_BUSNUM                        0xFF
#define BCM963XX_MAX_LINK_WIDTH                    PCIE_LINK_WIDTH_2LANES


/*
 * +-----------------------------------------------------
 * PCIe MSI
 * +-----------------------------------------------------
 */
#define MISC_MSI_DATA_CONFIG_MATCH_MAGIC           0x0000BCA0
#define MSI_ISR_NAME_STR_LEN                       32

/*
 * +-----------------------------------------------------
 * PCIe PHY
 * +-----------------------------------------------------
 */
#define PHY_PCIE_BLK2_PWR_MGMT_REGS                4
#define PCIE_G3_PLL_LOCK_WAIT_TIMEOUT              100 /* milliseconds */

/*
 * +-----------------------------------------------------
 *
 *  Macros
 *
 * +-----------------------------------------------------
 */

/*
 * +-----------------------------------------------------
 * DT Binding
 * +-----------------------------------------------------
 */

#ifdef CONFIG_OF
#ifndef INTERRUPT_ID_PCIE0
#define INTERRUPT_ID_PCIE0                         0
#define INTERRUPT_ID_PCIE1                         0
#define INTERRUPT_ID_PCIE2                         0
#define INTERRUPT_ID_PCIE3                         0
#endif /* !INTERRUPT_ID_PCIE0 */
#endif /* CONFIG_OF */

#define HCD_USE_DT_ENTRY(core)                                     \
	((IS_ENABLED(CONFIG_OF)) ? TRUE : FALSE)

#define HCD_FAIL_ON_DT_ERROR(res, err)                             \
	if (err) {                                                 \
	    HCD_ERROR("No DT entry for %s\n", res);                \
	    return err;                                            \
	} else {                                                   \
	    HCD_INFO("Updating %s settings from DT entry\n", res); \
	}
#define HCD_WARN_ON_DT_ERROR(res, err)                             \
	if (err) {                                                 \
	    HCD_WARN("No DT entry for %s, using defaults\n", res); \
	} else {                                                   \
	    HCD_INFO("Updating %s settings from DT entry\n", res); \
	}

#define OWIN_RES_CONFIGURED(pdrv, win)             ((pdrv)->resources.owin[win].start != 0)

/*
 * +-----------------------------------------------------
 *
 *  Structures
 *
 * +-----------------------------------------------------
 */

/*
 * +-----------------------------------------------------
 *
 *  Local Function prototype
 *
 * +-----------------------------------------------------
 */
/*
 * +-----------------------------------------------------
 * MISC
 * +-----------------------------------------------------
 */
static void bcm963xx_misc_set_pcie_reset(
	struct platform_device *pdev, int core, bool enable);

/*
 * +-----------------------------------------------------
 * MSI
 * +-----------------------------------------------------
 */
static int bcm963xx_pcie_msi_alloc_region(struct bcm963xx_pcie_msi *msi, int nirqs);
static void bcm963xx_pcie_msi_free(struct bcm963xx_pcie_msi *msi,
	unsigned long irq);
static int bcm963xx_pcie_msi_setup_irq(struct msi_controller *chip,
	struct pci_dev *pdev, struct msi_desc *desc);
static int bcm963xx_pcie_msi_setup_irqs(struct msi_controller *chip,
	struct pci_dev *pdev, int nvec, int type);
static int bcm963xx_pcie_msi_setup_irq_range(struct msi_controller *chip,
	struct pci_dev *pdev, struct msi_desc *desc, int nvec);
static void bcm963xx_pcie_msi_teardown(struct msi_controller *chip,
	unsigned int irq);
static int bcm963xx_pcie_msi_map(struct irq_domain *domain,
	unsigned int irq, irq_hw_number_t hwirq);
static irqreturn_t bcm963xx_pcie_msi_isr(int irq, void *data);
static int bcm963xx_pcie_msi_enable(struct bcm963xx_pcie_hcd *pdrv);
static int bcm963xx_pcie_msi_disable(struct bcm963xx_pcie_hcd *pdrv);

static void __iomem *bcm963xx_pcie_map_bus(struct pci_bus *bus,
	unsigned int devfn, int where);
int bcm963xx_pcie_map_irq(const struct pci_dev *pcidev, u8 slot,
	u8 pin);

uint16 bcm963xx_pcie_mdio_read(struct bcm963xx_pcie_hcd *pdrv,
	uint16 phyad, uint16 regad);
int bcm963xx_pcie_mdio_write(struct bcm963xx_pcie_hcd *pdrv,
	uint16 phyad, uint16 regad, uint16 wrdata);

static void bcm963xx_pcie_phy_config(struct bcm963xx_pcie_hcd *pdrv);
static void bcm963xx_pcie_gen2_phy_config(struct bcm963xx_pcie_hcd *pdrv);
static void bcm963xx_pcie_gen3_phy_config(struct bcm963xx_pcie_hcd *pdrv);
static int bcm963xx_pcie_gen3_phy_enable_ssc(struct bcm963xx_pcie_hcd *pdrv,
	bool enable);
static void bcm963xx_pcie_gen3_phy_config_ssc(struct bcm963xx_pcie_hcd *pdrv);
static int bcm963xx_pcie_phy_config_pwrmode(struct bcm963xx_pcie_hcd *pdrv);
static void bcm963xx_pcie_core_set_reset(struct bcm963xx_pcie_hcd *pdrv,
	bool enable);
static int bcm963xx_pcie_core_set_speed(struct bcm963xx_pcie_hcd *pdrv);
static int bcm963xx_pcie_core_reset_config(struct bcm963xx_pcie_hcd *pdrv);
static int bcm963xx_pcie_core_config(struct bcm963xx_pcie_hcd *pdrv);
static void bcm963xx_pcie_core_reset(struct bcm963xx_pcie_hcd *pdrv);


static irqreturn_t bcm963xx_pcie_errlog_isr(int irq, void *data);
static int bcm963xx_pcie_errlog_enable(struct bcm963xx_pcie_hcd *pdrv);
static void bcm963xx_pcie_errlog_disable(struct bcm963xx_pcie_hcd *pdrv);

static int bcm963xx_pcie_setup_owin(struct bcm963xx_pcie_hcd *pdrv,
	struct list_head *resources);
static int bcm963xx_pcie_setup_regs(struct bcm963xx_pcie_hcd *pdrv);
static void bcm963xx_pcie_unmap_res(struct bcm963xx_pcie_hcd *pdrv);
static int bcm963xx_pcie_parse_dt(struct bcm963xx_pcie_hcd *pdrv);
static int bcm963xx_pcie_init_res(struct bcm963xx_pcie_hcd *pdrv);

static int  bcm963xx_pcie_probe(struct platform_device *pdev);
static int  bcm963xx_pcie_remove(struct platform_device *pdev);
static int __init bcm963xx_pcie_init(void);
static void __exit bcm963xx_pcie_exit(void);
static void bcm963xx_pcie_plt_dev_release(struct device *dev);
static int __init bcm963xx_pcie_plt_init(void);
static void __init bcm963xx_pcie_plt_deinit(void);


/*
 * +-----------------------------------------------------
 *
 *  external Function prototype
 *
 * +-----------------------------------------------------
 */
extern unsigned long getMemorySize(void);
extern int ubus_decode_pcie_wnd_cfg(uint32 base, uint32 size, uint32 core);

#if defined(USE_PMC_BRD_STUBS)
static inline void pcie_hcd_pmc_power_up(int unit) { }
static inline void pcie_hcd_pmc_power_down(int unit) { }
static inline int pcie_hcd_get_port_mode(int port) {
	/* Alwyas RC mode */
	return 1;
}
static inline int pcie_hcd_port_enabled(int port) {
	/* Port not present */
	return 0;
}
static inline int pmc_hcd_get_chip_id(void) {
	unsigned long chipid = 0;
	int res;
	char chipidstr[32];

	snprintf(chipidstr, sizeof(chipidstr), "0x%d", CONFIG_BCM_CHIP_NUMBER);

	res = kstrtoul(chipidstr, 0, &chipid);

	if (res != 0) {
	    HCD_ERROR("kstrtoul for 0x%d with [%s] returned res %d\r\n",
	        CONFIG_BCM_CHIP_NUMBER, chipidstr, res);
	}

	return chipid;
}
#else /* !USE_PMC_BRD_STUBS */
static inline void pcie_hcd_pmc_power_up(int unit) {
	return pmc_pcie_power_up(unit);
}
static inline void pcie_hcd_pmc_power_down(int unit) {
	return pmc_pcie_power_down(unit);
}
static inline int pcie_hcd_get_port_mode(int port) {
	return kerSysGetPciePortMode(port);
}
static inline int pcie_hcd_port_enabled(int port) {
	return kerSysGetPciePortEnable(port);
}
static inline int pmc_hcd_get_chip_id(void) {
	return kerSysGetChipId();
}
#endif /* !USE_PMC_BRD_STUBS */

static inline int pmc_hcd_get_chip_rev(void) {
	return UtilGetChipRev();
}

/*
 * +-----------------------------------------------------
 *
 *  Global variables
 *
 * +-----------------------------------------------------
 */
static struct platform_device bcm963xx_pcie_plt_dev[NUM_PCIE_CORES];

static struct pci_ops bcm963xx_pcie_ops = {
	.map_bus = bcm963xx_pcie_map_bus,
	.read = pci_generic_config_read,
	.write = pci_generic_config_write,
};

static const struct of_device_id bcm963xx_pcie_of_match[] = {
	{
	    .type = "pci",
	    .compatible = "brcm,bcm963xx-pcie",
	},
	{},
};
MODULE_DEVICE_TABLE(of, bcm963xx_pcie_of_match);

static struct platform_driver bcm963xx_pcie_driver = {
	.probe  = bcm963xx_pcie_probe,
	.driver = {
	    .name  = BCM963XX_PCIE_DRV_NAME,
	    .owner = THIS_MODULE,
	    .of_match_table = of_match_ptr(bcm963xx_pcie_of_match),
	},
	.remove = bcm963xx_pcie_remove,
};

static const struct of_device_id bcm963xx_vpcie_of_match[] = {
	{
	    .type = "vpci",
	    .compatible = "brcm,bcm963xx-vpcie",
	},
	{},
};
MODULE_DEVICE_TABLE(of, bcm963xx_vpcie_of_match);

static struct platform_driver pcie_vcore_driver = {
	.probe  = pcie_vcore_probe,
	.driver = {
	    .name  = PCIE_VCORE_DRV_NAME,
	    .owner = THIS_MODULE,
	    .of_match_table = of_match_ptr(bcm963xx_vpcie_of_match),
	},
	.remove = pcie_vcore_remove,
};

/*
 * +-----------------------------------------------------
 * MSI
 * +-----------------------------------------------------
 */
static char bcm963xx_pcie_msi_name[NUM_CORE][MSI_ISR_NAME_STR_LEN];

/*
 * +-----------------------------------------------------
 * errlog
 * +-----------------------------------------------------
 */
static char bcm963xx_pcie_ubus_intr_str[][32] = {
	"UBUS_LINKUP",
	"UBUS_LINKDOWN",
	"UBUS_INV_CMD_ERR",
	"UBUS_ADDR_RANGE_ERR",
	"UBUS_UBUS_TOUT_ERR",
	"UBUS_REG_ACC_ERR",
	"UBUS_REG_DLEN_ERR",
	"UBUS_MEM_ACC_ERR",
	"UBUS_MEM_WIN_CRSS_ERR",
	"UBUS_PCIE_TGT_RD_RPLY_ERR",
	"UBUS_PCIE_TGT_WR_RPLY_ERR",
	"UBUS_MEM_INV_SWAP_ERR",
	"UBUS_PCIE_INV_SWAP_ERR",
	"UBUS_UBUS_SPARSE_ERR",
	"UBUS_UBUS_SEC_ERR",
	"UBUS_UBUS_WRAP_ERR",
};
#define NUM_PCIE_UBUS_INTR_STR                 \
	(sizeof(bcm963xx_pcie_ubus_intr_str)/sizeof(bcm963xx_pcie_ubus_intr_str[0]))

/*
 * +-----------------------------------------------------
 *
 *  Local inline functions
 *
 * +-----------------------------------------------------
 */
/* read 32bit pcie register space */
static inline u32 hcd_readl(void __iomem *addr, unsigned offset)
{
	return readl(addr + offset);
}

/* write 32bit pcie register space */
static inline void hcd_writel(u32 data, void __iomem *addr, unsigned offset)
{
	writel(data, addr + offset);
}

/* Check if PCIe link up or not */
static inline bool hcd_is_pcie_link_up(struct bcm963xx_pcie_hcd *pdrv)
{
	u32    status = hcd_readl(pdrv->base, RC_DL_DL_STATUS_OFFSET);

	return (status&RC_DL_DL_STATUS_PHYLINKUP_MASK) ? TRUE : FALSE;
}

/* msi controller to msi */
static inline struct bcm963xx_pcie_msi *to_bcm963xx_pcie_msi(struct msi_controller *chip)
{
	return container_of(chip, struct bcm963xx_pcie_msi, chip);
}

/* configure spread spectrum clock (ssc) */
static inline void bcm963xx_pcie_config_ssc(struct bcm963xx_pcie_hcd *pdrv)
{
	/* Call ssc config functions based on core gen */
	if (pdrv->core_gen == PCIE_LINK_SPEED_GEN2) {
	    bcm963xx_pcie_gen2_phy_config_ssc(pdrv);
	} else if (pdrv->core_gen == PCIE_LINK_SPEED_GEN3) {
	    bcm963xx_pcie_gen3_phy_config_ssc(pdrv);
	}
	return;
}

/* enable/disable spread spectrum clock (ssc) */
static inline int bcm963xx_pcie_enable_ssc(struct bcm963xx_pcie_hcd *pdrv,
	bool enable)
{
	int ret = 0;

	/* Call ssc enable functions based on core gen */
	if (pdrv->core_gen == PCIE_LINK_SPEED_GEN2) {
	    ret = bcm963xx_pcie_gen2_phy_enable_ssc(pdrv, enable);
	} else if (pdrv->core_gen == PCIE_LINK_SPEED_GEN3) {
	    ret = bcm963xx_pcie_gen3_phy_enable_ssc(pdrv, enable);
	}
	return ret;
}


/*
 * +-----------------------------------------------------
 *
 *  Local Functions
 *
 * +-----------------------------------------------------
 */
/*
 * +-----------------------------------------------------
 * MISC block Local Functions
 * +-----------------------------------------------------
 */
/*
 *
 * Function bcm963xx_misc_set_pcie_reset (pdev, core, enable)
 *
 *
 *   Parameters:
 *    pdev ... pointer to platform device (Not used)
 *    core ... pcie core (0, 1, ..NUM_CORE)
 *    enable ... falg to enable reset
 *
 *   Description:
 *    Set the PCIe core reset state (enable/disable)
 *
 *  Return: None
 */
static void bcm963xx_misc_set_pcie_reset(
	struct platform_device *pdev, int core, bool enable)
{
#if defined(MISC_PCIE_CTRL_CORE_SOFT_RESET_MASK)
	volatile struct Misc *misc_regs = (struct Misc *)MISC_BASE;
	if (enable == TRUE)
	    /* Soft Reset the core */
	    misc_regs->miscPCIECtrl &= ~(1<<core);
	else
	    /* Bring the core out of Soft Reset */
	    misc_regs->miscPCIECtrl |= (1<<core);
#else /* !MISC_PCIE_CTRL_CORE_SOFT_RESET_MASK */
	HCD_ERROR("core [%d] skipped reset %sassert as misc registers are not defined\r\n",
	    core, (enable) ? "" : "de-");
#endif /* !MISC_PCIE_CTRL_CORE_SOFT_RESET_MASK */

	return;
}

/*
 * +-----------------------------------------------------
 * MSI Local Functions
 * +-----------------------------------------------------
 */

/*
 *
 * Function bcm963xx_pcie_msi_alloc_region (msi, nirqs)
 *
 *
 *   Parameters:
 *     msi ... msi control block pointer
 *     nirqs...number of irqs to allocate
 *
 *
 *   Description:
 *     Allocate contiguous range of unused msi irq numbers for the client to use
 *
 *
 *  Return: msi irq number on success, -ve value on failure
 */
static int bcm963xx_pcie_msi_alloc_region(struct bcm963xx_pcie_msi *msi, int nirqs)
{
	int irq;

	HCD_FN_ENT();

	mutex_lock(&msi->lock);

	irq = bitmap_find_free_region(msi->used, msi->map_size,
	    order_base_2(nirqs));

	mutex_unlock(&msi->lock);

	if (irq >= 0) {
	    HCD_INFO("Allocated MSI Hw IRQ %d - %d\r\n", irq, (irq + nirqs - 1));
	}

	HCD_FN_EXT();

	return irq;
}

/*
 *
 * Function bcm963xx_pcie_msi_free (msi, irq)
 *
 *
 *   Parameters:
 *  msi ... msi control block pointer
 *  irq  ... msi irq number previously allocated
 *
 *
 *   Description:
 *     Free (unuse) a previously allocated msi interrupt number
 *
 *
 *  Return: None
 */
static void bcm963xx_pcie_msi_free(struct bcm963xx_pcie_msi *msi,
	unsigned long irq)
{

	HCD_FN_ENT();

	HCD_INFO("Freeing MSI Hw IRQ %lu\r\n", irq);

	mutex_lock(&msi->lock);

	if (!test_bit(irq, msi->used)) {
	    HCD_ERROR("trying to free unused MSI#%lu\n", irq);
	} else {
	    clear_bit(irq, msi->used);
	}

	mutex_unlock(&msi->lock);

	HCD_FN_EXT();
}

/*
 *
 * Function bcm963xx_pcie_msi_setup_irq_range (chip, pdev, desc, nvec)
 *
 *
 *   Parameters:
 *  chip ... msi chip control block pointer
 *  pdev  ... pci device pointer
 *  desc  ... msi irq descriptor pointer
 *  nvec  ... number of irqs
 *
 *   Description:
 *     Allocate nvec consecutive free MSI interrupts, Map MSI interrupts to system
 *     virtual interrupts, send an MSI message for the matching adress & data
 *
 *
 *  Return: 0 on success, -ve value on failure
 */
static int bcm963xx_pcie_msi_setup_irq_range(struct msi_controller *chip,
	struct pci_dev *pdev, struct msi_desc *desc, int nvec)
{
	struct bcm963xx_pcie_msi *msi = to_bcm963xx_pcie_msi(chip);
	struct msi_msg msg;
	unsigned int irq;
	int hwirq;
	int i;

	HCD_FN_ENT();

	hwirq = bcm963xx_pcie_msi_alloc_region(msi, nvec);
	if (hwirq < 0) {
	    HCD_ERROR("failed to allocate IRQ range %d: %d\n", nvec, hwirq);
	    return -ENOSPC;
	}

	/* Create IRQ mapping for all the vectors */
	for (i = 0; i < nvec; i++) {
	    irq = irq_create_mapping(msi->domain, hwirq+i);
	    if (!irq) {
	        HCD_ERROR("failed to create IRQ mapping for %d\n", hwirq+i);
	        bcm963xx_pcie_msi_free(msi, irq);
	        return -EINVAL;
	    }
	}

	/* make sure irq is able to find it */
	irq = irq_find_mapping(msi->domain, hwirq);
	if (!irq) {
	    HCD_ERROR("irq_find_mapping(0x%px, %d) returned %d\r\n", msi->domain, hwirq, irq);
	    return -ENOSPC;
	}

	/* Setup the msi desc parameters */
	for (i = 0; i < nvec; i++) {
	    irq_set_msi_desc_off(irq, i, desc);
	}
	desc->nvec_used = nvec;
	desc->msi_attrib.multiple = order_base_2(nvec);

	/* Configure the hardware for MSI */
	msg.address_lo = hcd_readl(msi->pdrv->base, MISC_MSI_BAR_CONFIG_LO_OFFSET);
	msg.address_lo &= MISC_MSI_BAR_CONFIG_LO_MATCH_ADDR_MASK;
	msg.address_hi = 0;
	msg.data = (MISC_MSI_DATA_CONFIG_MATCH_MAGIC | hwirq);

	pci_write_msi_msg(irq, &msg);

	HCD_INFO("MSI Msg lo_addr [0x%x] hi_addr [0x%x], data [0x%x]\r\n",
	    msg.address_lo, msg.address_hi, msg.data);

	HCD_FN_EXT();

	return 0;
}

/*
 *
 * Function bcm963xx_pcie_msi_setup_irq (chip, pdev, desc)
 *
 *
 *   Parameters:
 *  chip ... msi chip control block pointer
 *  pdev  ... pci device pointer
 *  desc  ... msi irq descriptior
 *
 *   Description:
 *     call setup_irq_range with 1 interrupt
 *
 *  Return: 0 on success, -ve value on failure
 */
static int bcm963xx_pcie_msi_setup_irq(struct msi_controller *chip,
	struct pci_dev *pdev, struct msi_desc *desc)
{
	int rc;

	HCD_FN_ENT();

	rc = bcm963xx_pcie_msi_setup_irq_range(chip, pdev, desc, 1);

	HCD_FN_EXT();

	return rc;
}

/*
 *
 * Function bcm963xx_pcie_msi_setup_irqs (chip, pdev, nvec, type)
 *
 *
 *   Parameters:
 *  chip ... msi chip control block pointer
 *  pdev  ... pci device pointer
 *  nvec  ... number of irqs
 *  type  ... type of msi interrupt
 *
 *   Description:
 *     Check the type and descriptor and call setup_irq_range()
 *
 *  Return: 0 on success, -ve value on failure
 */
static int bcm963xx_pcie_msi_setup_irqs(struct msi_controller *chip,
	struct pci_dev *pdev, int nvec, int type)
{
	struct msi_desc *desc = NULL;
	int rc;

	HCD_FN_ENT();

	/* MSI-X interrupts are not supported */
	if (type == PCI_CAP_ID_MSIX) {
	    HCD_ERROR("%s: MSIX is not supported \r\n", __FUNCTION__);
	    return -EINVAL;
	}

#if defined(CONFIG_PCI_MSI)
	/* Get the MSI descriptor pointer */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
	WARN_ON(!list_is_singular(&pdev->dev.msi_list));
	desc = list_entry(pdev->dev.msi_list.next, struct msi_desc, list);
#else /* LINUX_VERSION_CODE < KERNEL_VERSION(4, 4, 0) */
	WARN_ON(!list_is_singular(&pdev->msi_list));
	desc = list_entry(pdev->msi_list.next, struct msi_desc, list);
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(4, 4, 0) */
#endif /* CONFIG_PCI_MSI */

	if (desc == NULL) {
	    HCD_LOG("%s: msi desc is NULL\r\n", __FUNCTION__);
	    return -EINVAL;
	}

	rc = bcm963xx_pcie_msi_setup_irq_range(chip, pdev, desc, nvec);

	HCD_FN_EXT();

	return rc;
}

/*
 *
 * Function bcm963xx_pcie_msi_teardown (chip, irq)
 *
 *
 *   Parameters:
 *  chip ... msi chip control block pointer
 *  irq  ... virtual irq number
 *
 *   Description:
 *     unmap the system virtual interrupt, and unuse the msi irq number
 *
 *
 *  Return: None
 */
static void bcm963xx_pcie_msi_teardown(struct msi_controller *chip,
	unsigned int irq)
{
	struct bcm963xx_pcie_msi *msi = to_bcm963xx_pcie_msi(chip);
	struct irq_data *d = irq_get_irq_data(irq);
	irq_hw_number_t hwirq = irqd_to_hwirq(d);

	HCD_FN_ENT();

	irq_dispose_mapping(irq);
	bcm963xx_pcie_msi_free(msi, hwirq);

	HCD_FN_EXT();
}


/*
 *
 * Function bcm963xx_pcie_msi_map (domain, irq, hwirq)
 *
 *
 *   Parameters:
 *  domain ... msi chip control block pointer
 *  irq  ... virtual irq number
 *  hwirq ... msi interrupt number for this core
 *
 *   Description:
 *     setup the parameters for virtual irq number
 *
 *
 *  Return: 0 on success, -ve value on failure
 */
static int bcm963xx_pcie_msi_map(struct irq_domain *domain,
	unsigned int irq, irq_hw_number_t hwirq)
{
	struct bcm963xx_pcie_msi *msi = (struct bcm963xx_pcie_msi*)domain->host_data;

	HCD_FN_ENT();

	irq_set_chip_and_handler(irq, &msi->irq_ops, handle_simple_irq);
	irq_set_chip_data(irq, msi);

	HCD_FN_EXT();

	return 0;
}


/*
 *
 * Function bcm963xx_pcie_msi_isr (irq, data)
 *
 *
 *   Parameters:
 *  irq  ... PCIe core irq number
 *  data ... pointer to hcd (given through request_irq())
 *
 *   Description:
 *     Read the MSI interrupt status. Process all the set MSI interrupts
 *
 *
 *  Return: IRQ_HANDLED on success, IRQ_NONE on NO MSI interrupts
 */
static irqreturn_t bcm963xx_pcie_msi_isr(int irq, void *data)
{
	struct bcm963xx_pcie_msi *msi = (struct bcm963xx_pcie_msi *)data;
	struct bcm963xx_pcie_hcd *pdrv = msi->pdrv;
	unsigned int i;
	uint32 reg_val;

	HCD_FN_ENT();

	/* Get the MSI interrupt status */
	reg_val = hcd_readl(pdrv->base, msi->intr_status);
	reg_val &= (msi->intr_bitmask);

	/* clear the interrupts, as this is an edge triggered interrupt */
	hcd_writel(reg_val, pdrv->base, msi->intr_clear);

	/* Process all the available MSI interrupts */
	i = 0;

	while (reg_val != 0x00000000) {
	    if (reg_val & (1ul << (i+msi->intr_bitshift))) {
	        if (i < msi->map_size) {
	            irq = irq_find_mapping(msi->domain, i);
	            if (irq) {
	                if (test_bit(i, msi->used))
	                    generic_handle_irq(irq);
	                else
	                    HCD_INFO("unexpected MSI %d\n", i);
	            } else {
	                /* that's weird who triggered this? */
	                /* just clear it */
	                HCD_INFO("Un handled MSI %d\n", i);
	            }
	        }
	        reg_val &= (~(1ul << (i+msi->intr_bitshift)));
	    }
	    i++;
	}

	HCD_FN_EXT();

	return (i > 0) ? IRQ_HANDLED : IRQ_NONE;

}


/*
 *
 * Function bcm963xx_pcie_msi_enable (pdrv)
 *
 *
 *   Parameters:
 *  pdrv ... pointer to hcd
 *
 *   Description:
 *     Allocate msi control block, initialize msi control block parameters,
 *     setup PCIe core irq isr and configure the hardware to enable MSI
 *     functionality
 *
 *
 *  Return: 0 on success, -ve on failure
 */
static int bcm963xx_pcie_msi_enable(struct bcm963xx_pcie_hcd *pdrv)
{
	struct platform_device *pdev = pdrv->pdev;
	struct bcm963xx_pcie_msi *msi = NULL;
	int err;
	u32 reg_data;

	HCD_FN_ENT();

	msi = kzalloc(sizeof(*msi), GFP_KERNEL);
	if (!msi) {
	    HCD_ERROR("Unable to allocate memory for MSI\r\n");
	    return -ENOMEM;
	}

	HCD_INFO("Allocated [0x%px] hcd\r\n", msi);


	mutex_init(&msi->lock);
	snprintf(bcm963xx_pcie_msi_name[pdrv->core_id], MSI_ISR_NAME_STR_LEN,
	    "msi_pcie:%d", pdrv->core_id);

	/* Initialize all msi structure elements */
	pdrv->msi = msi;
	msi->pdrv = pdrv;
	msi->irq_ops.name = bcm963xx_pcie_msi_name[pdrv->core_id];
	msi->irq_ops.irq_enable = pci_msi_unmask_irq;
	msi->irq_ops.irq_disable = pci_msi_mask_irq;
	msi->irq_ops.irq_mask = pci_msi_mask_irq;
	msi->irq_ops.irq_unmask = pci_msi_unmask_irq;
	msi->domain_ops.map = bcm963xx_pcie_msi_map;

	if (pdrv->core_rev >= 0x0303) {
	    msi->intr_status = MSI_INTR2_CPU_STATUS_OFFSET;
	    msi->intr_clear = MSI_INTR2_CPU_CLEAR_OFFSET;
	    msi->intr_mask_set = MSI_INTR2_CPU_MASK_SET_OFFSET;
	    msi->intr_mask_clear = MSI_INTR2_CPU_MASK_CLEAR_OFFSET;
	    msi->intr_bitshift = MSI_INTR2_CPU_MSI_INTR_SHIFT;
	    msi->intr_bitmask = MSI_INTR2_CPU_MSI_INTR_MASK;
	    msi->cpu_intr_bitmask = CPU_INTR1_PCIE_MSI_INTR_CPU_INTR;
	} else {
	    msi->intr_status = INTR2_CPU_STATUS_OFFSET;
	    msi->intr_clear = INTR2_CPU_CLEAR_OFFSET;
	    msi->intr_mask_set = INTR2_CPU_MASK_SET_OFFSET;
	    msi->intr_mask_clear = INTR2_CPU_MASK_CLEAR_OFFSET;
	    msi->intr_bitshift = INTR2_CPU_MSI_INTR_SHIFT;
	    msi->intr_bitmask = INTR2_CPU_MSI_INTR_MASK;
	    msi->cpu_intr_bitmask = CPU_INTR1_PCIE_INTR_CPU_INTR;
	}
	msi->map_size = MSI_MAP_MAX_SIZE - msi->intr_bitshift;

	msi->chip.dev = &pdev->dev;
	msi->chip.setup_irq = bcm963xx_pcie_msi_setup_irq;
	msi->chip.setup_irqs = bcm963xx_pcie_msi_setup_irqs;
	msi->chip.teardown_irq = bcm963xx_pcie_msi_teardown;
	msi->domain = irq_domain_add_linear(pdev->dev.of_node, msi->map_size,
	    &msi->domain_ops, &msi->chip);
	if (!msi->domain) {
	    HCD_ERROR("failed to create IRQ domain\n");
	    return -ENOMEM;
	}

	if (HCD_USE_DT_ENTRY(pdrv->core_id) && (pdrv->resources.irq == 0)) {
	    err = platform_get_irq_byname(pdev, "msi");
	    if (err > 0) {
	        pdrv->resources.irq = err;
	    } else {
	        HCD_ERROR("failed to get msi intr from DT: %d\n", err);
	        goto err;
	    }
	}

	err = request_irq(pdrv->resources.irq, bcm963xx_pcie_msi_isr, IRQF_SHARED,
	    msi->irq_ops.name, msi);
	if (err < 0) {
	    HCD_ERROR("failed to request IRQ[%d]: %d\n", pdrv->resources.irq, err);
	    goto err;
	}


	HCD_INFO("Using irq=%d for PCIE-MSI interrupts\r\n", pdrv->resources.irq);

	/* Program the Root Complex Registers for matching address hi and low */
	/* The address should be unique with in the down stream/up stream BAR mapping */
	reg_data = (MISC_MSI_BAR_CONFIG_LO_MATCH_ADDR_MASK
	    | MISC_MSI_BAR_CONFIG_LO_ENABLE_MASK);

	hcd_writel(reg_data, pdrv->base, MISC_MSI_BAR_CONFIG_LO_OFFSET);
	hcd_writel(0, pdrv->base, MISC_MSI_BAR_CONFIG_HI_OFFSET);

	/* Program the RC registers for matching data pattern */
	reg_data = MISC_MSI_DATA_CONFIG_MATCH_MASK;
	reg_data &= ((~(msi->map_size-1))<<MISC_MSI_DATA_CONFIG_MATCH_SHIFT);
	reg_data |= MISC_MSI_DATA_CONFIG_MATCH_MAGIC;
	hcd_writel(reg_data, pdrv->base, MISC_MSI_DATA_CONFIG_OFFSET);

	/* Clear all MSI interrupts initially */
	reg_data = msi->intr_bitmask;
	hcd_writel(reg_data, pdrv->base, msi->intr_clear);


	/* enable all available MSI vectors */
	hcd_writel(reg_data, pdrv->base, msi->intr_mask_clear);

	/* Enable MSI interrupt at L1 Intr1 controller */
	reg_data = msi->cpu_intr_bitmask;
	hcd_writel(reg_data, pdrv->base, CPU_INTR1_INTR_MASK_CLEAR_OFFSET);

	/* Set the flag to specify MSI is enabled */
	msi->enabled = true;

	HCD_INFO("MSI Enabled\n");

	err = 0;

err:
	HCD_FN_EXT();
	return err;
}


/*
 *
 * Function bcm963xx_pcie_msi_disable (pdrv)
 *
 *
 *   Parameters:
 *  pdrv ... pointer to hcd
 *
 *   Description:
 *       Disable MSI feature on the hardware, Free the PCIe core isr and
 *     unmap and free all the MSI interrupts
 *
 *
 *  Return: 0 on success, -ve on failure
 */
static int bcm963xx_pcie_msi_disable(struct bcm963xx_pcie_hcd *pdrv)
{
	struct bcm963xx_pcie_msi *msi = NULL;
	unsigned int i, irq;
	u32 reg_data;

	HCD_FN_ENT();

	if (!pdrv) {
	    HCD_FN_EXT();
	    return 0;
	}

	msi = pdrv->msi;
	if (msi) {
	    /* Disable MSI interrupt at L1 Intr1 controller */
	    reg_data = msi->cpu_intr_bitmask;
	    hcd_writel(reg_data, pdrv->base, CPU_INTR1_INTR_MASK_SET_OFFSET);

	    /* Disable all available MSI vectors */
	    reg_data = msi->intr_bitmask;
	    hcd_writel(reg_data, pdrv->base, msi->intr_mask_set);

	    /* Clear all mapped interrupts */
	    if (pdrv->resources.irq > 0)
	        free_irq(pdrv->resources.irq, msi);

	    for (i = 0; i < msi->map_size; i++) {
	        irq = irq_find_mapping(msi->domain, i);
	        if (irq > 0)
	            irq_dispose_mapping(irq);
	    }

	    irq_domain_remove(msi->domain);

	    /* Set the flag to specify MSI is disabled */
	    msi->enabled = false;

	    HCD_INFO("MSI Disabled\n");

	    pdrv->msi = NULL;
	    kfree(msi);
	}

	HCD_FN_EXT();
	return 0;
}

/*
 *
 * Function bcm963xx_pcie_map_bus (bus, devfn, where)
 *
 *
 *   Parameters:
 *    bus ... pointer to pci bus data structure
 *    devfn ... pci device, function mapping
 *    where ... offset from the device base
 *
 *   Description:
 *     Check the PCI bus/device for allowable combinations, find the device
 *     base offset, setup the hardware for access to device/functon
 *
 *
 *  Return: mapped configuration address on success, NULL on failure
 */
static void __iomem *bcm963xx_pcie_map_bus(struct pci_bus *bus,
	unsigned int devfn, int where)
{
	struct bcm963xx_pcie_hcd *pdrv = bus->sysdata;
	uint32 offset;
	uint32 bus_no = bus->number;
	uint32 dev_no = PCI_SLOT(devfn);
	uint32 func_no = PCI_FUNC(devfn);
	bool valid = FALSE;

	HCD_FN_ENT();

	HCD_INFO("bus [0x%px] bus_no [%d] dev [%d] func [%d] where [%d]\r\n",
	    bus, bus_no, dev_no, func_no, where);

	/* RC config space is registers not memory, allow only valid bus/dev combinations */
	if (pdrv->hc_cfg.apon == HCD_APON_OFF_WITH_DOMAIN) {
	    valid = FALSE;
	} else if (bus_no <= (BCM963XX_ROOT_BUSNUM+1)) {
	    /* Root Conplex bridge, first device or switch */
	    /* Allow only configuration space (dev#0) */
	    valid = (dev_no == 0);
	} else if (bus_no == (BCM963XX_ROOT_BUSNUM+2)) {
	    /* Switch UP stream port */
	    /* Allow access for all the DN ports */
	    valid =  TRUE;
	} else {
	    /* Switch down stream ports to devices */
	    /* Allow only configuration space (dev#0) */
	    valid = (dev_no == 0); /* otherwise will loop for the rest of the device */
	}

	if (!valid) {
	    return NULL;
	}

	/* find the offset */
	offset = (bus_no) ? PCIE_EXT_CFG_DEV_OFFSET : 0;
	offset += where;

	/* Select the configuration */
	hcd_writel((bus_no<<EXT_CFG_BUS_NUM_SHIFT)
	    |(dev_no <<EXT_CFG_DEV_NUM_SHIFT)
	    |(func_no<<EXT_CFG_FUNC_NUM_SHIFT),
	    pdrv->base, EXT_CFG_PCIE_EXT_CFG_INDEX_OFFSET);

	HCD_INFO("config space mapped address = [0x%px]\r\n", pdrv->base + offset);

	HCD_FN_EXT();
	return pdrv->base + offset;
}


/*
 *
 * Function bcm963xx_pcie_map_irq (pcidev, slot, pin)
 *
 *
 *   Parameters:
 *    pcidev ... pointer to pci device data structure
 *    slot ... pci slot (not used)
 *    pin ... pin number (not used)
 *
 *   Description:
 *       Get the pcie core irq number.
 *
 *
 *  Return: pcie core irq number
 */
int bcm963xx_pcie_map_irq(const struct pci_dev *pcidev,
	u8 slot, u8 pin)
{
	int irq = -1;

	struct bcm963xx_pcie_hcd *pdrv = pcidev->bus->sysdata;

	if (IS_PCIE_VCORE(pdrv->pdev->id)) {
	    irq = pcie_vcore_map_irq(pcidev, slot, pin);
	} else if (HCD_USE_DT_ENTRY(pdrv->core_id)) {
	    irq = of_irq_parse_and_map_pci(pcidev, slot, pin);
	} else {
	    irq = pdrv->resources.irq;
	}

	return irq;
}

/*
 *  Function bcm963xx_pcie_mdio_read (pdrv, phyad, regad)
 *
 *   Parameters:
 *    pdrv ... pointer to pcie core hcd data structure
 *	 phyad ... MDIO PHY address (typically 0!)
 *	 regad ... Register address in range 0-0x1f
 *
 *   Description:
 *      Perform PCIE MDIO read on specified PHY (typically 0), and Register.
 *      Access is through an indirect command/status mechanism, and timeout
 *	 is possible. If command is not immediately complete, which would
 *      be typically the case, one more attempt is made after a 1ms delay.
 *
 *   Return: 16-bit data item or 0xdead on MDIO timeout
 */
uint16 bcm963xx_pcie_mdio_read(struct bcm963xx_pcie_hcd *pdrv,
	uint16 phyad, uint16 regad)
{
	int timeout;
	uint32 data;
	uint16 retval;

	HCD_FN_ENT();

	/* Bit-20=1 to initiate READ, bits 19:16 is the phyad, bits 4:0 is the regad */
	data = 0x100000;
	data = data |((phyad & 0xf)<<16);
	data = data |(regad & 0x1F);

	hcd_writel(data, pdrv->base, RC_DL_MDIO_ADDR_OFFSET);
	/* critical delay */
	udelay(1000);

	timeout = 2;
	while (timeout > 0) {
	    data = hcd_readl(pdrv->base, RC_DL_MDIO_RD_DATA_OFFSET);
	    /* Bit-31=1 is DONE */
	    if (data & 0x80000000)
	        break;
	    timeout = timeout - 1;
	    udelay(1000);
	}

	if (timeout == 0) {
	    retval = 0xdead;
	} else {
	    /* Bits 15:0 is read data */
	    retval = (data&0xffff);
	}

	HCD_FN_EXT();

	return retval;
}

/*
 *
 * Function bcm963xx_pcie_mdio_write (pdrv, phyad, regad, wrdata)
 *
 *
 *   Parameters:
 *    pdrv ... pointer to pcie core hcd data structure
 *	 phyad ... MDIO PHY address (typically 0!)
 *	 regad  ... Register address in range 0-0x1f
 *	 wrdata ... 16-bit write data
 *
 *
 *   Description:
 *     Perform PCIE MDIO write on specified PHY (typically 0), and Register.
 *     Access is through an indirect command/status mechanism, and timeout
 *     is possible. If command is not immediately complete, which would
 *     be typically the case, one more attempt is made after a 1ms delay.
 *
 *
 *  Return: 1 on success, 0 on timeout
 */
int bcm963xx_pcie_mdio_write(struct bcm963xx_pcie_hcd *pdrv,
	uint16 phyad, uint16 regad, uint16 wrdata)
{
	int timeout;
	uint32 data;

	HCD_FN_ENT();

	/* bits 19:16 is the phyad, bits 4:0 is the regad */
	data = ((phyad & 0xf) << 16);
	data = data | (regad & 0x1F);

	hcd_writel(data, pdrv->base, RC_DL_MDIO_ADDR_OFFSET);
	udelay(1000);

	/* Bit-31=1 to initial the WRITE, bits 15:0 is the write data */
	data = 0x80000000;
	data = data | (wrdata & 0xFFFF);

	hcd_writel(data, pdrv->base, RC_DL_MDIO_WR_DATA_OFFSET);
	udelay(1000);

	/* Bit-31=0 when DONE */
	timeout = 2;
	while (timeout > 0) {

	    data = hcd_readl(pdrv->base, RC_DL_MDIO_WR_DATA_OFFSET);

	    /* CTRL1 Bit-31=1 is DONE */
	    if ((data & 0x80000000) == 0)
	        break;

	    timeout = timeout - 1;
	    udelay(1000);
	}

	HCD_FN_EXT();

	if (timeout == 0) {
	    return 0;
	} else
	    return 1;
}

/*
 *
 * Function bcm963xx_pcie_gen3_phy_config_ssc (pdrv)
 *
 *
 *   Parameters:
 *    pdrv ... pointer to pcie core hcd data structure
 *
 *   Description:
 *     Configure PCIe SSC through MDIO interface. The settings
 *     largely comes from ASIC design team
 *
 *  Return: None
 */
static void bcm963xx_pcie_gen3_phy_config_ssc(struct bcm963xx_pcie_hcd *pdrv)
{
	uint32 rddata, wrdata;
	int waittime;

	HCD_FN_ENT();

	/* Nothing to do, if SSC is not configured */
	if (pdrv->hc_cfg.ssc == FALSE) {
	    HCD_FN_EXT();
	    return;
	}

	/*
	 * Keep PLL2 in reset
	 * set PCIE_x_G3_PLL_PLL_RESETS (0xXXXXA010), RESETB and POST_RESETB (bit 1:0) to 2'b00
	 */
	rddata = hcd_readl(pdrv->base, G3_PLL_PLL_RESETS_OFFSET);
	wrdata = rddata & (~G3_PLL_PLL_RESETS_RESETB | ~G3_PLL_PLL_RESETS_POST_RESETB);
	hcd_writel(wrdata, pdrv->base, G3_PLL_PLL_RESETS_OFFSET);

	/*
	 * program PLL2 fractional divider
	 * set PCIE_x_G3_PLL_PLL_NDIV (0xXXXXA01C) to 0x1D
	 * set PCIE_x_G3_PLL_PLL_NDIV_FRAC_HOLDALL (0xXXXXA05C) to 0xECCCCD
	 */
	rddata = hcd_readl(pdrv->base, G3_PLL_PLL_NDIV_OFFSET);
	wrdata = rddata & (~G3_PLL_PLL_NDIV_INT_MASK);
	wrdata |= G3_PLL_PLL_NDIV_INT(0x1D);
	hcd_writel(wrdata, pdrv->base, G3_PLL_PLL_NDIV_OFFSET);

	rddata = hcd_readl(pdrv->base, G3_PLL_PLL_NDIV_FRAC_HOLDALL_OFFSET);
	wrdata = rddata & (~G3_PLL_PLL_NDIV_FRAC_HOLDALL_FRAC_MASK);
	wrdata &= (~G3_PLL_PLL_NDIV_FRAC_HOLDALL_HOLA_ALL);
	wrdata |= G3_PLL_PLL_NDIV_FRAC_HOLDALL_NDIV_FRAC(0xECCCCD);
	hcd_writel(wrdata, pdrv->base, G3_PLL_PLL_NDIV_FRAC_HOLDALL_OFFSET);

	/*
	 * bring PLL2 out of reset
	 * set PCIE_x_G3_PLL_PLL_RESETS (0xXXXXA010), RESETB and POST_RESETB (bit 1:0) to 2'b11
	 */
	rddata = hcd_readl(pdrv->base, G3_PLL_PLL_RESETS_OFFSET);
	wrdata = rddata | G3_PLL_PLL_RESETS_RESETB|G3_PLL_PLL_RESETS_POST_RESETB;
	hcd_writel(wrdata, pdrv->base, G3_PLL_PLL_RESETS_OFFSET);

	/*
	 * check PLL2 is locked
	 * check PCIE_x_G3_PLL_PLL_STAT (0xXXXXA03C), PLL_LOCK (bit 31) is 1
	 */
	waittime = PCIE_G3_PLL_LOCK_WAIT_TIMEOUT;
	rddata = 0;

	while (!(rddata & G3_PLL_PLL_STAT_PLL_LOCK) && waittime) {
	    rddata = hcd_readl(pdrv->base, G3_PLL_PLL_STAT_OFFSET);
	    udelay(1000);
		waittime--;
	}
	if (waittime == 0) {
	    HCD_ERROR("PCIe Core [%d] PLL LOCK timedout stat 0x%x\r\n",
	        pdrv->core_id, rddata);
	}

	HCD_FN_EXT();

	return;
}

/*
 *
 * Function bcm963xx_pcie_gen3_phy_enable_ssc (pdrv,enable)
 *
 *
 *	 Parameters:
 *	  pdrv ... pointer to pcie core hcd data structure
 *	  enable...flag to specify enable or disable SSC
 *
 *	 Description:
 *	 Enable/disable SSC. Assumed that SSC is configured before enabling the SSC
 *
 *	Return: 0:	   on success or no action.
 *				-1:   on failure or timeout
 */
static int bcm963xx_pcie_gen3_phy_enable_ssc(struct bcm963xx_pcie_hcd *pdrv,
	bool enable)
{
	uint32 rddata, wrdata;
	int waittime;

	/* Nothing to do, if SSC is not configured */
	if (pdrv->hc_cfg.ssc == FALSE) {
	    HCD_FN_EXT();
	    return 0;
	}

	if (enable == TRUE) {
	    /*
	     * enable SSC
	     * set PCIE_x_G3_PLL_PLL_SSC_STEP_VCOGAIN (0xXXXXA064) to 0x30068
	     * set PCIE_x_G3_PLL_PLL_SSC_LIMIT_SSC_MODE (0xXXXXA060) to 0x1001333
	     */
	    rddata = hcd_readl(pdrv->base, G3_PLL_PLL_SSC_STEP_VCOGAIN_OFFSET);
	    wrdata = rddata & (~G3_PLL_PLL_SSC_STEP_VCOGAIN_VCO_GAIN_MASK);
	    wrdata |= G3_PLL_PLL_SSC_STEP_VCOGAIN_VCO_GAIN(3);
	    wrdata &= (~G3_PLL_PLL_SSC_STEP_VCOGAIN_SSC_STEP_MASK);
	    wrdata |= G3_PLL_PLL_SSC_STEP_VCOGAIN_SSC_STEP(0x0068);
	    hcd_writel(wrdata, pdrv->base, G3_PLL_PLL_SSC_STEP_VCOGAIN_OFFSET);

	    rddata = hcd_readl(pdrv->base, G3_PLL_PLL_SSC_LIMIT_SSC_MODE_OFFSET);
	    wrdata = rddata & (~G3_PLL_PLL_SSC_LIMIT_SSC_MODE_LIMIT_MASK);
	    wrdata |= G3_PLL_PLL_SSC_LIMIT_SSC_MODE_LIMIT(0x1333);
	    wrdata |= G3_PLL_PLL_SSC_LIMIT_SSC_MODE_MODE;
	    hcd_writel(wrdata, pdrv->base, G3_PLL_PLL_SSC_STEP_VCOGAIN_OFFSET);

	    /*
	     * check that serdes PLL is locked
	     * set serdes MDIO_MMDSEL_AER_mdio_aer to 0x9FF
	     * check serdes PCIE_BLK1 (0x1100), status (reg 1), pll_lock (bit 11) is 1
	     */
	    /*
	     * Block:0xffd0 (AER), Register:0x1e (AER)
	     * Value:0x09ff bit[15:11] 1(PMA_PMD) bit[10:0] 0x1FF (broadcast)
	     *
	     *	  tmp = pcie_mdio_write (0, &h1f&, &hffd0&) ' AER block
	     *	  tmp = pcie_mdio_write (0, &h1e&, &h09ff&) ' PMA_PMD Broadcast
	     */
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x1f, 0xffd0);
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x1e, 0x09ff);

	    waittime = PCIE_G3_PLL_LOCK_WAIT_TIMEOUT;
	    rddata = 0;

	    /*
	     * Block:0x1100 (PCIE_BLK1), Register:0x01 (Status)
	     * Value:0x0800 bit[11] 1 (pll_lock)
	     */
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x1f, 0x1100);
	    while (!(rddata & (1<<11)) && waittime) {
		rddata = bcm963xx_pcie_mdio_read(pdrv, 0, 0x01);
	        udelay(1000);
	        waittime--;
	    }

	    if (waittime == 0) {
	        HCD_ERROR("PCIe Core [%d] PLL LOCK timedout. Status [0x%x]\r\n",
	            pdrv->core_id, rddata);
	        HCD_FN_EXT();
	        return -1;
	    }
	} else {
	    rddata = hcd_readl(pdrv->base, G3_PLL_PLL_SSC_LIMIT_SSC_MODE_OFFSET);
	    wrdata = rddata & ~G3_PLL_PLL_SSC_LIMIT_SSC_MODE_MODE;
	    hcd_writel(wrdata, pdrv->base, G3_PLL_PLL_SSC_STEP_VCOGAIN_OFFSET);
	}

	HCD_FN_EXT();

	return 0;
}

/*
 *
 * Function bcm963xx_pcie_phy_config_pwrmode (pdrv)
 *
 *
 * Parameters:
 *    pdrv ... pointer to pcie core hcd data structure
 *
 *    Description:
 *        Setup pcie core power mode according to configuration
 *
 *  Return: 0 on success, -ve on failure
 */
static int bcm963xx_pcie_phy_config_pwrmode(struct bcm963xx_pcie_hcd *pdrv)
{
	HCD_FN_ENT();

	/* Nothing to do, if configured to use default phy powermode */
	if (pdrv->hc_cfg.phypwrmode == 0)
	    return 0;

	/* Check if the host is configured for GEN1 speed */
	if (pdrv->hc_cfg.speed == 1) {
	    HCD_LOG("[%d] Power mode [%d] is not supported in GEN1 Speeds",
	        pdrv->core_id, pdrv->hc_cfg.phypwrmode);
	    return 0;
	}

	if (pdrv->hc_cfg.phypwrmode == 1) {
	    int   lane;
	    uint16 block_addr;

	    /* Setting received from hardware team for  *** 63138B1 ***
	     * For other platforms the values might need tuning
	     *
	     * Note:Below EP side changes also needed for Gen2-40nm Brcm SoC's
	     *      pcieserdesreg 0x820, 0x16 0xa400
	     *      pcieserdesreg 0x820, 0x17 0x05b7
	     *      pcieserdesreg 0x801, 0x1a 0x4028
	     */
	    for (lane = 0; lane < pdrv->resources.link_width; lane++) {
	        block_addr = SERDES_TX_CTR1_LN0_OFFSET + lane * SERDES_TX_CTR1_LN_SIZE;
	        bcm963xx_pcie_mdio_write(pdrv, 0, 0x001f, block_addr);
	        bcm963xx_pcie_mdio_write(pdrv, 0, 0x0001, 0x000b);
	        bcm963xx_pcie_mdio_write(pdrv, 0, 0x0000, 0x0e20);

	        block_addr = SERDES_TX_DFE0_LN0_OFFSET + lane * SERDES_TX_DFE0_LN_SIZE;
	        bcm963xx_pcie_mdio_write(pdrv, 0, 0x001f, block_addr);
	        bcm963xx_pcie_mdio_write(pdrv, 0, 0x000d, 0x00f0);

	       /* Required to settle the power mode setting */
	       mdelay(10);
	    }

	    HCD_LOG("Core [%d] phy power mode set to [%d]\n", pdrv->core_id,
	        pdrv->hc_cfg.phypwrmode);
	}

	HCD_FN_EXT();

	return 0;
}

/*
 *
 * Function bcm963xx_pcie_gen2_phy_config (pdrv)
 *
 *
 *   Parameters:
 *    pdrv ... pointer to pcie core hcd data structure
 *
 *   Description:
 *     Configure PCIe PHY through MDIO interface for any workarounds or
 *     special features for Gen2 core based on Viper SerDes.
 *     The settings largely comes from ASIC design team
 *
 *  Return: None
 */
static void bcm963xx_pcie_gen2_phy_config(struct bcm963xx_pcie_hcd *pdrv)
{
	HCD_FN_ENT();

	if (pdrv->core_gen != PCIE_LINK_SPEED_GEN2) {
	    /* Nothing to do for non gen2 cores */
	    HCD_FN_EXT();
	    return;
	}

	/*
	 * Populate the workarounds for Gen2 for the early Gen2 revisions
	 * All issues got fixed in revisions 3.03 and later
	 */
	if (pdrv->wars.g2defset == 1) {

	    HCD_LOG("Core [%d] applying gen2 workarounds for core rev < 3.04\n",
	        pdrv->core_id);

	    /*
	     * VCO Calibration Timers
	     * Workaround:
	     * Block 0x3000, Register 0xB = 0x40
	     * Block 0x3000, Register 0xD = 7
	     * Notes:
	     * -Fixed in 63148A0, 63381B0, 63138B0 but ok to write anyway
	     */
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x1f, 0x3000);
	    bcm963xx_pcie_mdio_read (pdrv, 0, 0x1f);  /* just to exericise the read */
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0xB, 0x40);
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0xD, 7);

	    /*
	     * Reference clock output level
	     * Workaround:
	     * Block 0x2200, Register 3 = 0xaba4
	     * Note:
	     * -Fixed in 63148A0, 63381B0, 63138B0 but ok to write anyway
	     */
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x1f, 0x2200);
	    bcm963xx_pcie_mdio_write(pdrv, 0, 3, 0xaba4);

	    /*
	     * Tx Pre-emphasis
	     * Workaround:
	     * Block 0x4000, Register 0 = 0x1d20  // Gen1
	     * Block 0x4000, Register 1 = 0x12cd  // Gen1
	     * Block 0x4000, Register 3 = 0x0016  // Gen1, Gen2
	     * Block 0x4000, Register 4 = 0x5920  // Gen2
	     * Block 0x4000, Register 5 = 0x13cd  // Gen2
	     * Notes:
	     * -Fixed in 63148A0, 63381B0, 63138B0 but ok to write anyway
	     */
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x1f, 0x4000);
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0, 0x1D20);
	    bcm963xx_pcie_mdio_write(pdrv, 0, 1, 0x12CD);
	    bcm963xx_pcie_mdio_write(pdrv, 0, 3, 0x0016);
	    bcm963xx_pcie_mdio_write(pdrv, 0, 4, 0x5920);
	    bcm963xx_pcie_mdio_write(pdrv, 0, 5, 0x13CD);

	    /*
	     * Rx Signal Detect
	     * Workaround:
	     * Block 0x6000, Register 5 = 0x2c0d
	     * Notes:
	     * -Fixed in 63148A0, 63381B0, 63138B0 but ok to write anyway
	     */
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x1f, 0x6000);
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x5, 0x2C0D);

	    /*
	     * Rx Jitter Tolerance
	     * Workaround:
	     * Block 0x7300, Register 3 = 0x190  // Gen1
	     * Block 0x7300, Register 9 = 0x194  // Gen2
	     * Notes:
	     * -Gen1 setting 63148A0, 63381B0, 63138B0 but ok to write anyway
	     * -Gen2 setting only in latest SerDes RTL  / future tapeouts
	     */
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x1f, 0x7300);
	    bcm963xx_pcie_mdio_write(pdrv, 0, 3, 0x190);
	    bcm963xx_pcie_mdio_write(pdrv, 0, 9, 0x194);

	    /*
	     * Gen2 Rx Equalizer
	     * Workaround:
	     * Block 0x6000 Register 7 = 0xf0c8  // Gen2
	     * Notes:
	     * -New setting only in latest SerDes RTL / future tapeouts
	     */
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x1f, 0x6000);
	    bcm963xx_pcie_mdio_write(pdrv, 0, 7, 0xf0c8);


	    /*
	     * EP Mode PLL Bandwidth and Peaking
	     * Workaround:
	     * Block 0x2100, Register 0 = 0x5174
	     * Block 0x2100, Register 4 = 0x6023
	     * Notes:
	     * -Only needed for EP mode, but ok to write in RC mode too
	     * -New setting only in latest SerDes RTL / future tapeouts
	     */
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x1f, 0x2100);
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0, 0x5174);
	    bcm963xx_pcie_mdio_write(pdrv, 0, 4, 0x6023);
	}

	HCD_FN_EXT();

	return;
}

/*
 *
 * Function bcm963xx_pcie_gen3_phy_config (pdrv)
 *
 *
 *   Parameters:
 *    pdrv ... pointer to pcie core hcd data structure
 *
 *   Description:
 *     Configure PCIe PHY through MDIO interface for any workarounds or
 *     special features for GEN3 core based on Blackshark SerDes.
 *     The settings largely comes from ASIC design team
 *
 *  Return: None
 */
static void bcm963xx_pcie_gen3_phy_config(struct bcm963xx_pcie_hcd *pdrv)
{
	uint16 rddata, wrdata;

	HCD_FN_ENT();

	if (pdrv->core_gen != PCIE_LINK_SPEED_GEN3) {

	    /* Nothing to do for gen3 or greater cores */
	    HCD_FN_EXT();
	    return;
	}

	if (pdrv->wars.g3txclk == 1) {
	    int i;
	    uint16 pwrmgmt_reg_vals[PHY_PCIE_BLK2_PWR_MGMT_REGS];

	    HCD_LOG("Core [%d] applying GEN3 TxClock Start workaround\n",
	        pdrv->core_id);

	    /*
	     * For Gen3, first Select the lane
	     *
	     * Block:0xffd0 (AER), Register:0x1e (AER)
	     * Value:0x09ff bit[15:11] 1(PMA_PMD) bit[10:0] 0x000 (lane0)
	     *
	     *   tmp = pcie_mdio_write (0, &h1f&, &hffd0&) ' AER block
	     *   tmp = pcie_mdio_write (0, &h1e&, &h0800&) ' PMA_PMD Common
	     */
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x1f, 0xffd0);
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x1e, 0x0800);


	    /*
	     * Block:0x1000 (PCIE_BLK0), Register:0x07 (tx_seq_ctrl)
	     * bit[12] 1 (tca_one_shot_disable) bit[1]] 1 (tca_fine_tune_en)
	     *
	     * ' disable one shot (bit-12), enable fine tune tca (bit-11)
	     * ' which allows pclk to be active even if txclk is stuck
	     *      tmp = pcie_mdio_write (0, &h1f&, &h1000&) ' PCIE_BLK0
	     *      rddata = pcie_mdio_read (0, &h07&) ' tx_seq_ctrl
	     *      wrdata = (rddata And Not(&h1800)) Or lshift(1,12) Or lshift(1,11)
	     *      tmp = pcie_mdio_write (0, &h07&, wrdata)  ' tx_seq_ctrl
	     */
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x1f, 0x1000);
	    rddata = bcm963xx_pcie_mdio_read(pdrv, 0, 0x07);
	    wrdata = rddata | (1 << 12) | (1<< 11);
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x07, wrdata);

	    /*
	     * ' need to deassert PHY reset for next step
	     *      pcie_perst_reset(DEASSERT)
	     */
	    bcm963xx_pcie_core_set_reset(pdrv, FALSE);
	    /*
	     * Block:0x1200 (PCIE_BLK2), Register:0x01,2,3,4 (PCIE_BLK2_PwrMgmt0,1,2,3)
	     * bit[3] 1 (tx0_pwrdn) bit[10] 1 (tx0_pwrdn)
	     * bit[3] 0 (tx0_pwrdn) bit[10] 0 (tx0_pwrdn)
	     *
	     * ' read all 4 power management control registers for restoration later
	     * ' while toggling bit-3 of the 8-bit power control (tx powerdown)
	     *     tmp = pcie_mdio_write (0, &h1f&, &h1200&) ' PCIE_BLK2
	     *     for i = 0 to 3
	     *     vals(i) = pcie_mdio_read (0, i+1)
	     *     tmp = pcie_mdio_write (0, i+1, &h0808&)
	     *     tmp = pcie_mdio_write (0, i+1, &h0000&)
	     *     next
	     */
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x1f, 0x1200);
	    for (i = 0; i < PHY_PCIE_BLK2_PWR_MGMT_REGS; i++) {
		    pwrmgmt_reg_vals[i] = bcm963xx_pcie_mdio_read(pdrv, 0, i+1);
			wrdata = 0x0808;
		    bcm963xx_pcie_mdio_write(pdrv, 0, i+1, wrdata);
			wrdata = 0x0000;
		    bcm963xx_pcie_mdio_write(pdrv, 0, i+1, wrdata);
	    }

	    /*
	     * ' Assert phy reset to get out of tx powerdown
	     *     pcie_perst_reset(ASSERT)
	     */
	    bcm963xx_pcie_core_set_reset(pdrv, TRUE);

	    /*
	     * Block:0xffd0 (AER), Register:0x1e (AER)
	     * Value:0x09ff bit[15:11] 1(PMA_PMD) bit[10:0] 0x1FF (broadcast)
	     *
	     *     tmp = pcie_mdio_write (0, &h1f&, &hffd0&) ' AER block
	     *     tmp = pcie_mdio_write (0, &h1e&, &h09ff&) ' PMA_PMD Broadcast
	     */
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x1f, 0xffd0);
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x1e, 0x09ff);
	    /*
		 * Block: 0xd3b0 (AMS_TX) Register:0x01 (TX_CTRL_1)
		 * Value: clear dcc_en (bit 15)
		 *
	     *     wrdata = &h0718& ' clear DCC Enable bit-15 (default for register is &h8718&
	     *     tmp = pcie_mdio_write (0, &h1f&, &hd3b0&) ' AMS Tx
	     *     tmp = pcie_mdio_write (0, &h01&, wrdata)
	     *                     ' clear register 1, bit-15 (default for register is &h8718&)
	     */
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x1f, 0xd3b0);
	    rddata = bcm963xx_pcie_mdio_read(pdrv, 0, 0x01);
	    wrdata = rddata & (~0x8000);
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x01, wrdata);

	    /*
	     *     for lane = 0 to numlanes-1
	     *     tmp = pcie_mdio_write (0, &h1f&, &hffd0&) ' AER block
	     *     tmp = pcie_mdio_write (0, &h1e&, (&h800 Or lane)) ' PMA_PMD Broadcast
	     *     tmp = pcie_mdio_write (0, &h1f&, &hd3b0&) ' AMS Tx
	     *     rddata = pcie_mdio_read (0, &h01&)
	     *     if rddata <> wrdata then
	     *     print "Error in txclk_start_workaround, lane=" & lane & ":
	     *            expecting &h" & hex16(wrdata) & ", read back &h" & hex16(rddata)
	     *     status = 1
	     *     end if
	     *     next
	     */
	    for (i = 0; i < pdrv->resources.link_width; i++) {
	        bcm963xx_pcie_mdio_write(pdrv, 0, 0x1f, 0xffd0);
	        bcm963xx_pcie_mdio_write(pdrv, 0, 0x1e, 0x0800|i);
		    bcm963xx_pcie_mdio_write(pdrv, 0, 0x1f, 0xd3b0);
		    rddata = bcm963xx_pcie_mdio_read(pdrv, 0, 0x01);
		    if (rddata != wrdata) {
	            HCD_ERROR("Core [%d] Lane [%d] Error in txclk_start_workaround,",
	                pdrv->core_id, i);
	            HCD_ERROR("expecting 0x%x, read back 0x%x\r\n",
	                wrdata, rddata);
	        }
	    }

	    /*
	     * Block:0xffd0 (AER), Register:0x1e (AER)
	     * Value:0x09ff bit[15:11] 1(PMA_PMD) bit[10:0] 0x000 (lane0)
	     *
	     *     tmp = pcie_mdio_write (0, &h1f&, &hffd0&) ' AER block
	     *     tmp = pcie_mdio_write (0, &h1e&, &h0800&) ' PMA_PMD Common
	     */
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x1f, 0xffd0);
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x1f, 0x0800);

	    /*
	     * Block:0x1000 (PCIE_BLK0), Register:0x07 (tx_seq_ctrl)
	     * bit[12] 1 (tca_one_shot_disable) bit[1]] 1 (tca_fine_tune_en)
	     *
	     *     ' re-enable one shot, clear the disable (bit-12), disable fine tune tca (bit-11)
	     *     tmp = pcie_mdio_write (0, &h1f&, &h1000&) ' PCIE_BLK0
	     *     rddata = pcie_mdio_read (0, &h07&) ' tx_seq_ctrl
	     *     wrdata = (rddata And Not(&h1800)) Or lshift(0,12) Or lshift(0,11)
	     *     tmp = pcie_mdio_write (0, &h07&, wrdata)  ' tx_seq_ctrl
	     */
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x1f, 0x1000);
	    rddata = bcm963xx_pcie_mdio_read(pdrv, 0, 0x07);
	    wrdata = rddata | (1 << 12) | (1<< 11);
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x07, wrdata);

	    /*
	     *     tmp = pcie_mdio_write (0, &h1f&, &h1200&) ' PCIE_BLK2
	     *     for i = 0 to 3
	     *     tmp = pcie_mdio_write (0, i+1, vals(i))
	     *     next
	     */
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x1f, 0x1200);
	    for (i = 0; i < PHY_PCIE_BLK2_PWR_MGMT_REGS; i++) {
		    bcm963xx_pcie_mdio_write(pdrv, 0, i+1, pwrmgmt_reg_vals[i]);
	    }
	}

	if (pdrv->wars.g3rxset == 1) {

	   HCD_LOG("Core [%d] applying GEN3 Rx Default Settings workaround\n",
	       pdrv->core_id);

	    /*
		 * Block:0xffd0 (AER), Register:0x1e (AER)
		 * Value:0x09ff bit[15:11] 1(PMA_PMD) bit[10:0] 0x1FF (broadcast)
		 *
		 *	  tmp = pcie_mdio_write (0, &h1f&, &hffd0&) ' AER block
		 *	  tmp = pcie_mdio_write (0, &h1e&, &h09ff&) ' PMA_PMD Broadcast
		 */
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x1f, 0xffd0);
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x1e, 0x09ff);


	    /*	      ' ----------------------------------------------------------------
	     *	      ' The following flips DC offset loop polarity for all 3 PCIe gens
	     *	      ' ----------------------------------------------------------------
	     *	      tmp = pcie_mdio_write (0, &h1f&, &hd240&)
	     *	      tmp = pcie_mdio_write (0, &h00&, &h320c&) ' default &h120c&
	     *
	     *	      tmp = pcie_mdio_write (0, &h1f&, &hd2a0&)
	     *	      tmp = pcie_mdio_write (0, &h01&, &h000b&) ' default &h0003&
	     */

	    /*
		 * Block:0xd240 (TRNSUM_B), Register:0x00 (rx_trnsum_b_ctrl_0)
		 * Value:0x320c bit[13] 1 (rg_dc_offset_grad_inv)
		 */
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x1f, 0xd240);
	    rddata = bcm963xx_pcie_mdio_read(pdrv, 0, 0x00);
	    wrdata = rddata | (1 << 13);
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x00, wrdata);

	    /*
		 * Block:0xd2a0 (AEQ), Register:0x01 (rx_aeq_ctrl_1)
		 * Value:0x000b bit[3] 1(rg_invert_os)
		 */
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x1f, 0xd2a0);
	    rddata = bcm963xx_pcie_mdio_read(pdrv, 0, 0x01);
	    wrdata = rddata | (1 << 3);
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x01, wrdata);

	    /*
	     *        ' ----------------------------------------------------------------
	     *        ' The following adjust CDR behavior
	     *        ' ----------------------------------------------------------------
	     *
	     *        tmp = pcie_mdio_write (0, &h1f&, &hd2d0&)
	     *
	     *        tmp = pcie_mdio_write (0, &h06&, &h36d3&) ' default &h3713&
	     *        tmp = pcie_mdio_write (0, &h07&, &h3653&) ' default &h3713&
	     *        tmp = pcie_mdio_write (0, &h08&, &h2452&) ' default &h24d2&
	     *        tmp = pcie_mdio_write (0, &h09&, &h2452&) ' default &h2452&
	     *
	     *        tmp = pcie_mdio_write (0, &h02&, &h085e&)
	     *        tmp = pcie_mdio_write (0, &h01&, &h085e&)
	     *        tmp = pcie_mdio_write (0, &h00&, &h085e&)
	     */


	    /*
	     * Block:0xd2d0 (CDR_C), Register:0x06 (rx_cdr_c_CONTROL_6)
		 * Value:0x36d3 bit[8:6] 3 (rg_acq_prop_bw_g2)
	     */
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x1f, 0xd2d0);

	    rddata = bcm963xx_pcie_mdio_read(pdrv, 0, 0x06);
	    wrdata = (rddata & (~0x01c0)) | (3 << 6);
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x06, wrdata);

	    /*
	     * Block:0xd2d0 (CDR_C), Register:0x07 (rx_cdr_c_CONTROL_7)
		 * Value:0x3653 bit[8:6] 1 (rg_acq_prop_bw_g2)
	     */
	    rddata = bcm963xx_pcie_mdio_read(pdrv, 0, 0x07);
	    wrdata = (rddata & (~0x01c0)) | (1 << 6);
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x07, wrdata);

	    /*
	     * Block:0xd2d0 (CDR_C), Register:0x08 (rx_cdr_c_CONTROL_8)
		 * Value:0x3653 bit[8:6] 1 (rg_acq_prop_bw_g2)
	     */
	    rddata = bcm963xx_pcie_mdio_read(pdrv, 0, 0x08);
	    wrdata = (rddata & (~0x01c0)) | (1 << 6);
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x08, wrdata);

	    /*
	     * Block:0xd2d0 (CDR_C), Register:0x09 (rx_cdr_c_CONTROL_9)
		 * Value:0x3653 bit[8:6] 1 (rg_acq_prop_bw_g2)
	     */
	    rddata = bcm963xx_pcie_mdio_read(pdrv, 0, 0x09);
	    wrdata = (rddata & (~0x01c0)) | (1 << 6);
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x09, wrdata);

	    /*
	     * Block:0xd2d0 (CDR_C), Register:0x02 (rx_cdr_c_CONTROL_2)
		 * Value:0x085e bit[13:5] 1 (rg_mon_offset_val_g0)
	     */
	    rddata = bcm963xx_pcie_mdio_read(pdrv, 0, 0x02);
	    wrdata = (rddata & (~0x1fe0)) | (66 << 5);
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x02, wrdata);

	    /*
	     * Block:0xd2d0 (CDR_C), Register:0x01 (rx_cdr_c_CONTROL_1)
		 * Value:0x085e bit[13:5] 1 (rg_mon_offset_val_g0)
	     */
	    rddata = bcm963xx_pcie_mdio_read(pdrv, 0, 0x01);
	    wrdata = (rddata & (~0x1fe0)) | (66 << 5);
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x01, wrdata);

	    /*
	     * Block:0xd2d0 (CDR_C), Register:0x00 (rx_cdr_c_CONTROL_0)
		 * Value:0x085e bit[13:5] 1 (rg_mon_offset_val_g0)
	     */
	    rddata = bcm963xx_pcie_mdio_read(pdrv, 0, 0x00);
	    wrdata = (rddata & (~0x1fe0)) | (66 << 5);
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x00, wrdata);

	    /*
	     *        ' ----------------------------------------------------------------
	     *        ' The following adjust VGA behavior
	     *        ' ----------------------------------------------------------------
	     *
	     *        tmp = pcie_mdio_write (0, &h1f&, &hd240&)
	     *        tmp = pcie_mdio_write (0, &h04&, &hcc66&) ' default &hcc22&
	     */

	    /*
	     * Block:0xd240 (TRNSUM_B), Register:0x04 (rx_trnsum_b_ctrl_4)
	     * Value:0xcc66 bit[7:4] 6 (rg_vga_pattern) bit[3:0] 6 (rg_vga_pattern_bit_en)
	     */
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x1f, 0xd240);
	    rddata = bcm963xx_pcie_mdio_read(pdrv, 0, 0x04);
	    wrdata = (rddata & (~0x00ff)) | (6 << 0) | (6 << 4);
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x04, wrdata);

	    /*
	     *        ' ----------------------------------------------------------------
	     *        ' The following adjust DFE behavior
	     *        ' ----------------------------------------------------------------
	     *
	     *        tmp = pcie_mdio_write (0, &h1f&, &hd240&)
	     *        tmp = pcie_mdio_write (0, &h05&, &h6a1c&) ' default &h681c&
	     *        tmp = pcie_mdio_write (0, &h06&, &h0a1c&) ' default &h081c&
	     */
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x1f, 0xd240);

	    /*
	     * Block:0xd240 (TRNSUM_B), Register:0x05 (rx_trnsum_b_ctrl_5)
	     * Value:0x6a1c bit[9] 1 (rg_dfe_1_cmn_only)
	     */
	    rddata = bcm963xx_pcie_mdio_read(pdrv, 0, 0x05);
	    wrdata = (rddata | (1 << 9));
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x05, wrdata);

	    /*
	     * Block:0xd240 (TRNSUM_B), Register:0x06 (rx_trnsum_b_ctrl_6)
	     * Value:0x0a1c bit[9] 1 (rg_dfe_2_cmn_only)
	     */
	    rddata = bcm963xx_pcie_mdio_read(pdrv, 0, 0x06);
	    wrdata = (rddata | (1 << 9));
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x06, wrdata);
	}

	HCD_FN_EXT();

	return;
}

/*
 *
 * Function bcm963xx_pcie_phy_config (pdrv)
 *
 *
 *   Parameters:
 *    pdrv ... pointer to pcie core hcd data structure
 *
 *   Description:
 *     Configure PCIe PHY through MDIO interface for any workarounds or
 *     special features. The settings largely comes from ASIC desig team
 *
 *  Return: None
 */
static void bcm963xx_pcie_phy_config(struct bcm963xx_pcie_hcd *pdrv)
{
	uint16 data = 0;

	HCD_FN_ENT();

	HCD_INFO("applying serdes parameters chipid [0x%x] chiprev [0x%x]\n",
	    pmc_hcd_get_chip_id(), pmc_hcd_get_chip_rev());

	/* configure Resistor calibration */
	bcm963xx_pcie_phy_config_rescal(pdrv);

	if (pdrv->wars.bifp1sysclk == 1) {

	    HCD_LOG("Core [%d] applying sys clock workaround part-2\n",
	        pdrv->core_id);

	    /*
	     * Workaround PART-2
	     * Set SerDes MDIO Block 0x1000, Register 1
	     * bit-12 = 1 (mdio_RC_refclk_sel)
	     * bit-13 = 1 (mdio_RC_refclk_val)
	     *
	     */
	    bcm963xx_pcie_mdio_write(pdrv, 0, 0x1f, 0x1000);
	    data = bcm963xx_pcie_mdio_read(pdrv, 0, 1);
	    data |= (3 << 12);
	    bcm963xx_pcie_mdio_write(pdrv, 0, 1, data);
	}

	if (pdrv->core_gen == PCIE_LINK_SPEED_GEN2) {
		/* Configure GEN2 PHY workarounds */
	    bcm963xx_pcie_gen2_phy_config(pdrv);
	} else if (pdrv->core_gen == PCIE_LINK_SPEED_GEN3) {
		/* Configure GEN3 PHY workarounds */
	    bcm963xx_pcie_gen3_phy_config(pdrv);
	}

	/* Configure SSC */
	bcm963xx_pcie_config_ssc(pdrv);

	/* Disable SSC, will be enabled after reset if link is up (enable= FALSE) */
	bcm963xx_pcie_enable_ssc(pdrv, FALSE);

	/* Set phy power mode if configured */
	bcm963xx_pcie_phy_config_pwrmode(pdrv);

	/* Allow settling time */
	mdelay(10);

	HCD_FN_EXT();

	return;
}

/*
 *
 * Function bcm963xx_pcie_core_set_reset (pdrv, enable)
 *
 *
 *   Parameters:
 *    pdrv ... pointer to pcie core hcd data structure
 *    enable ... falg to enable reset
 *
 *   Description:
 *    Set the PCIe core reset state (enable/disable)
 *
 *  Return: None
 */
static void bcm963xx_pcie_core_set_reset(
	struct bcm963xx_pcie_hcd *pdrv, bool enable)
{

	if (pdrv->core_rev >= 0x320) {
	    /* Use PCIe internal MISC Block to do reset */
	    uint32 reg_data;

	    reg_data = hcd_readl(pdrv->base, MISC_PCIE_CTRL_OFFSET);

	    if (enable == TRUE)
	        /* Assert fundamental Soft Reset the core	*/
	        reg_data &= ~MISC_PCIE_CTRL_PCIE_PERSTB;
	    else
	        /* De-assert reset and bring the core out of Soft Reset */
	        reg_data |= MISC_PCIE_CTRL_PCIE_PERSTB;

	    hcd_writel(reg_data, pdrv->base, MISC_PCIE_CTRL_OFFSET);
	} else {
	    /* Use external MISC Block to do reset */
	    bcm963xx_misc_set_pcie_reset(pdrv->misc_pdev, pdrv->core_id, enable);
	}

	/* Allow reset change to settle down */
	mdelay(10);

	return;
}

/*
 *
 * Function bcm963xx_pcie_core_set_speed (pdrv)
 *
 *
 *	Parameters:
 *	 pdrv ... pointer to pcie core hcd data structure
 *
 *	Description:
 *		Setup pcie core speed according to configuration
 *
 *  Return: 0 on success, -ve on failure
 */
static int bcm963xx_pcie_core_set_speed(struct bcm963xx_pcie_hcd *pdrv)
{
	u32 data;

	HCD_FN_ENT();

	/* Nothing to do, if configured to use default speed */
	if (pdrv->hc_cfg.speed == 0)
		return 0;

	data = hcd_readl(pdrv->base, RC_CFG_PRIV1_LINK_CAPABILITY_OFFSET);
	data &= ~RC_CFG_PRIV1_LINK_CAP_LINK_SPEED_MASK;
	data |= (pdrv->hc_cfg.speed & RC_CFG_PRIV1_LINK_CAP_LINK_SPEED_MASK);

	hcd_writel(data, pdrv->base, RC_CFG_PRIV1_LINK_CAPABILITY_OFFSET);

	data = hcd_readl(pdrv->base, RC_CFG_PCIE_LINK_STATUS_CONTROL_2_OFFSET);
	data &= ~RC_CFG_PCIE_LINK_CTRL_TGT_LINK_SPEED_MASK;
	data |= (pdrv->hc_cfg.speed & RC_CFG_PCIE_LINK_CTRL_TGT_LINK_SPEED_MASK);
	hcd_writel(data, pdrv->base, RC_CFG_PCIE_LINK_STATUS_CONTROL_2_OFFSET);

	HCD_LOG("[%d] Link Speed set to %d\n", pdrv->core_id, pdrv->hc_cfg.speed);

	/* Required to settle the speed setting */
	mdelay(10);

	HCD_FN_EXT();

	return 0;
}

/*
 *
 * Function bcm963xx_pcie_core_reset_config (pdrv)
 *
 *
 *   Parameters:
 *    pdrv ... pointer to pcie core hcd data structure
 *
 *   Description:
 *       Setup pcie core parameters that needs to be done when reset is asserted
 *
 *  Return: 0 on success, -ve on failure
 */
static int bcm963xx_pcie_core_reset_config(struct bcm963xx_pcie_hcd *pdrv)
{

	HCD_FN_ENT();

	/* Configure the link speed, if need */
	bcm963xx_pcie_core_set_speed(pdrv);

	/* sysclk war for bifurcated port (applicable only for some versions) */
	if (pdrv->wars.bifp1sysclk == 1) {
		uint32 hard_dbg;
		/*
		 * Workaround PART-1
		 * In PCIE_1_MISC_HARD_PCIE_HARD_DEBUG register, set
		 * REFCLK_OVERRIDE = 1
		 * REFCLK_OVERRIDE_IN = 0 (Internal CML reference clock for the SerDes PLL)
		 * REFCLK_OVERRIDE_OUT = 0 (Disabled output clock)
		 */
		hard_dbg = hcd_readl(pdrv->base, MISC_HARD_DEBUG_OFFSET);
		hard_dbg |= MISC_HARD_DEBUG_REFCLK_OVERRIDE;
		hard_dbg &= ~MISC_HARD_DEBUG_REFCLK_OVERRIDE_OUT;
		hard_dbg &= ~MISC_HARD_DEBUG_REFCLK_OVERRIDE_IN_MASK;

		hcd_writel(hard_dbg, pdrv->base, MISC_HARD_DEBUG_OFFSET);
	}

	HCD_FN_EXT();

	return 0;
}

/*
 *
 * Function bcm963xx_pcie_core_config (pdrv)
 *
 *
 *   Parameters:
 *    pdrv ... pointer to pcie core hcd data structure
 *
 *   Description:
 *       Setup pcie core legacy interrupts, outgoing memory window, bar1, pci class, UBUS
 *
 *  Return: 0 on success, -ve on failure
 */
static int bcm963xx_pcie_core_config(struct bcm963xx_pcie_hcd *pdrv)
{
	uint32	reg_data;
	struct resource *owin;
	int win;

	HCD_FN_ENT();

	/* setup lgacy outband interrupts */
	reg_data = (CPU_INTR1_PCIE_INTD_CPU_INTR
	            | CPU_INTR1_PCIE_INTC_CPU_INTR
	            | CPU_INTR1_PCIE_INTB_CPU_INTR
	            | CPU_INTR1_PCIE_INTA_CPU_INTR);
	hcd_writel(reg_data, pdrv->base, CPU_INTR1_INTR_MASK_CLEAR_OFFSET);


	for (win = OWIN0; win < NUM_OUTGOING_WINDOWS; win++) {
	    uint32 reg_offset;

	    if (!OWIN_RES_CONFIGURED(pdrv, win)) continue;

	    owin = &(pdrv->resources.owin[win]);

	    /* setup outgoing mem resource window */
	    reg_data = ((owin->start >> MISC_CPU_2_PCI_MEM_WIN_LO_BASE_LIMIT_LIMIT_SHIFT) <<
	        MISC_CPU_2_PCI_MEM_WIN_LO_BASE_LIMIT_BASE_SHIFT);
	    reg_data |= (owin->end & MISC_CPU_2_PCI_MEM_WIN_LO_BASE_LIMIT_LIMIT_MASK);
	    reg_offset = MISC_CPU_2_PCIE_MEM_WIN_BASE_LIMIT_OFFSET(win);
	    hcd_writel(reg_data, pdrv->base, reg_offset);

	    reg_data = (owin->start & MISC_CPU_2_PCI_MEM_WIN_LO_BASE_ADDR_MASK);
	    reg_offset = MISC_CPU_2_PCIE_MEM_WIN_LO_OFFSET(win);
	    hcd_writel(reg_data, pdrv->base, reg_offset);
	    /* TODO: for 64bit ARM */
	    /* reg_offset = MISC_CPU_2_PCIE_MEM_WIN_HI_OFFSET(win); */
	    /* hcd_writel(0, pdrv->base, reg_offset); */

	    if (pdrv->owin_need_remap[win]) {
	        int rc;

	        rc = ubus_decode_pcie_wnd_cfg(owin->start,
	            (owin->end - owin->start) + 1, pdrv->pdev->id);
	        if (rc != 0)
	            return rc;
	    }
	}

	/* setup incoming DDR memory BAR(1) */
	{
	    uint32 sizekb;
	    uint32 barsz;

	    /* system memory size in kB */
	    sizekb = ((getMemorySize()) >> 10);

	    /* Calculate the size to be programmed (in terms of 64KB) */
	    for (barsz = 0; barsz < MISC_RC_BAR_CONFIG_LO_SIZE_MAX; barsz++)
	    {
	        if ((64 * (1 << barsz)) >= sizekb) {
	            break;
	        }
	    }
	    reg_data = BCM963XX_DDR_UBUS_ADDRESS_BASE;
	    reg_data &= MISC_RC_BAR_CONFIG_LO_MATCH_ADDRESS_MASK;
	    reg_data |= (barsz+1);

	    hcd_writel(reg_data, pdrv->base, MISC_RC_BAR1_CONFIG_LO_OFFSET);
	    /* TODO: for 64bit ARM */
	    /* hcd_writel(0, pdrv->base, MISC_RC_BAR1_CONFIG_HI_OFFSET); */
	    reg_data = MISC_UBUS_BAR_CONFIG_ACCESS_EN;
	    hcd_writel(reg_data, pdrv->base, MISC_UBUS_BAR1_CONFIG_REMAP_OFFSET);
	}


	/* set device bus/func/func -no need */
	/* setup class code, as bridge */
	reg_data = hcd_readl(pdrv->base, RC_CFG_PRIV1_ID_VAL3_OFFSET);
	reg_data &= RC_CFG_PRIV1_ID_VAL3_REVISION_ID_MASK;
	reg_data |= (PCI_CLASS_BRIDGE_PCI << 8);
	hcd_writel(reg_data, pdrv->base, RC_CFG_PRIV1_ID_VAL3_OFFSET);
	/* disable bar0 size -no need */

	/* disable data bus error for enumeration */
	reg_data = hcd_readl(pdrv->base, MISC_CTRL_OFFSET);
	reg_data |= MISC_CTRL_CFG_READ_UR_MODE;
	/* Misc performance addition */
	reg_data |= (MISC_CTRL_MAX_BURST_SIZE_128B
	            |MISC_CTRL_PCIE_IN_WR_COMBINE
	            |MISC_CTRL_PCIE_RCB_MPS_MODE
	            |MISC_CTRL_PCIE_RCB_64B_MODE);

	/*
	 * BURST_ALIGN: 3.20 onwards aligned to 64B
	 *              3.10 aligned to 32B
	 *              others burst alignment enabled
	 */
	reg_data &= ~(MISC_CTRL_BURST_ALIGN_MASK(pdrv->core_rev));
	if (pdrv->core_rev >= 0x320) {
	    reg_data |=	MISC_CTRL_BURST_ALIGN(pdrv->core_rev, 4);
	} else if (pdrv->core_rev == 0x310) {
	    reg_data |=	MISC_CTRL_BURST_ALIGN(pdrv->core_rev, 3);
	} else {
	    reg_data |=	MISC_CTRL_BURST_ALIGN(pdrv->core_rev, 1);
	}

	if (pdrv->core_rev == 0x310) {
	    /* workaround for UBUS4 Logic Bug in this revision */
	    /* Limit the max burst to 64B */
	    reg_data &= ~MISC_CTRL_MAX_BURST_SIZE_MASK;
	    reg_data |= MISC_CTRL_MAX_BURST_SIZE_64B;
	}

	hcd_writel(reg_data, pdrv->base, MISC_CTRL_OFFSET);

	if (pdrv->core_rev >= 0x320) {
	    /* wait for UBUS replay for burst writes */
	    reg_data = hcd_readl(pdrv->base, MISC_UBUS_CTRL_OFFSET);
	    reg_data |= MISC_UBUS_CTRL_UBUS_WR_WITH_REPLY;
	    hcd_writel(reg_data, pdrv->base, MISC_UBUS_CTRL_OFFSET);
	}

	/* If configured, enable PCIe SSC (enable = TRUE) */
	bcm963xx_pcie_enable_ssc(pdrv, TRUE);

	HCD_FN_EXT();

	return 0;
}

/*
 *
 * Function bcm963xx_pcie_core_reset (pdrv)
 *
 *
 *   Parameters:
 *    pdrv ... pointer to pcie core hcd data structure
 *
 *   Description:
 *       Reset PCIe core using misc driver API's. Configure the phy parameters
 *     while the core is in reset.
 *
 *  Return: 0 on success, -ve on failure
 */
static void bcm963xx_pcie_core_reset(struct bcm963xx_pcie_hcd *pdrv)
{
	HCD_FN_ENT();

	/* Soft Reset the core */
	bcm963xx_pcie_core_set_reset(pdrv, TRUE);

	/* Configure the phy when core is in reset */
	bcm963xx_pcie_phy_config(pdrv);

	/* Configure the core in reset asset state */
	bcm963xx_pcie_core_reset_config(pdrv);

	/* Bring the core out of Soft Reset */
	bcm963xx_pcie_core_set_reset(pdrv, FALSE);

	/* this is a critical delay */
	mdelay(500);

	HCD_FN_EXT();
	return;
}


/*
 *
 * Function bcm963xx_pcie_errlog_isr (irq, data)
 *
 *
 *   Parameters:
 *  irq  ... PCIe core irq number
 *  data ... pointer to hcd (given through request_irq())
 *
 *   Description:
 *     Read the Error interrupt status. Process all the enabled Error interrupts
 *
 *
 *  Return: IRQ_HANDLED on success, IRQ_NONE on NO Error interrupts
 */
static irqreturn_t bcm963xx_pcie_errlog_isr(int irq, void *data)
{
	struct bcm963xx_pcie_hcd *pdrv = (struct bcm963xx_pcie_hcd*)data;
	unsigned int i;
	uint32 reg_val;
	uint32 int_status;

	HCD_FN_ENT();

	/* Get the UBUS interrupt status */
	int_status = hcd_readl(pdrv->base, CPU_INTR1_INTR_INTR_STATUS_OFFSET);

	i = 0;

	if (int_status & CPU_INTR1_PCIE_UBUS_CPU_INTR) {
	    /* Get the UBUS interrupt status */
	    reg_val = hcd_readl(pdrv->base, UBUS_INTR2_CPU_STATUS_OFFSET);
	    reg_val &= UBUS_INTR2_PCIE_INTR_MASK;

	    /* clear the interrupts, as this is an edge triggered interrupt */
	    hcd_writel(reg_val, pdrv->base, UBUS_INTR2_CPU_CLEAR_OFFSET);

	    /* Process all the available UBUS interrupts */

	    while (reg_val != 0x00000000) {
	        if (reg_val & (1ul << (i))) {
	            if (i >= NUM_PCIE_UBUS_INTR_STR) {
	                HCD_LOG("Core [%d] UBUS Intr [UnKnown_%d]\r\n", pdrv->core_id, i);
	            } else {
	                HCD_LOG("Core [%d] UBUS Intr [%s]\r\n", pdrv->core_id,
	                    bcm963xx_pcie_ubus_intr_str[i]);
	            }
	            reg_val &= (~(1ul << (i)));
	        }
	        i++;
	    }
	}

	if (int_status & CPU_INTR1_PCIE_INTR_CPU_INTR) {
	    /* Get the PCIe Core interrupt status */
	    reg_val = hcd_readl(pdrv->base, INTR2_CPU_STATUS_OFFSET);
	    reg_val &= INTR2_CPU_PCIE_INTR_MASK(pdrv->core_rev);

	    /* clear the interrupts, as this is an edge triggered interrupt */
	    hcd_writel(reg_val, pdrv->base, INTR2_CPU_CLEAR_OFFSET);

	    /* Process all the available PCIe interrupts */
	    while (reg_val != 0x00000000) {
	        if (reg_val & INTR2_CPU_PCIE_TGT_BAD_ADDR(pdrv->core_rev)) {
	            HCD_LOG("Core [%d] PCIe Intr [PCIE_TGT_BAD_ADDR] [0x%08x_%08x]\r\n",
	                pdrv->core_id, hcd_readl(pdrv->base, MISC_RC_BAD_ADDR_HI_OFFSET),
	                hcd_readl(pdrv->base, MISC_RC_BAD_ADDR_LO_OFFSET));
	            reg_val &= (~INTR2_CPU_PCIE_TGT_BAD_ADDR(pdrv->core_rev));
	        } else if (reg_val & INTR2_CPU_PCIE_TGT_BAD_ACCESS(pdrv->core_rev)) {
	            HCD_LOG("Core [%d] PCIe Intr [PCIE_TGT_BAD_ACCESS]\r\n", pdrv->core_id);
	            reg_val &= (~INTR2_CPU_PCIE_TGT_BAD_ACCESS(pdrv->core_rev));
	        } else if (reg_val & INTR2_CPU_PCIE_MSTR_UR_ATTN(pdrv->core_rev)) {
	            HCD_LOG("Core [%d] PCIe Intr [PCIE_MSTR_UR_ATTN]\r\n", pdrv->core_id);
	            reg_val &= (~INTR2_CPU_PCIE_MSTR_UR_ATTN(pdrv->core_rev));
	        } else if (reg_val & INTR2_CPU_PCIE_MSTR_CA_ATTN(pdrv->core_rev)) {
	            HCD_LOG("Core [%d] PCIe Intr [PCIE_MSTR_CA_ATTN]\r\n", pdrv->core_id);
	            reg_val &= (~INTR2_CPU_PCIE_MSTR_CA_ATTN(pdrv->core_rev));
	        } else if (reg_val & INTR2_CPU_PCIE_MSTR_RETRY_TIMEOUT(pdrv->core_rev)) {
	            HCD_LOG("Core [%d] PCIe Intr [PCIE_MSTR_RETRY_TIMEOUT]\r\n", pdrv->core_id);
	            reg_val &= (~INTR2_CPU_PCIE_MSTR_RETRY_TIMEOUT(pdrv->core_rev));
	        } else if (reg_val & INTR2_CPU_PCIE_MSTR_FWD_ERR(pdrv->core_rev)) {
	            HCD_LOG("Core [%d] PCIe Intr [PCIE_MSTR_FWD_ERR]\r\n", pdrv->core_id);
	            reg_val &= (~INTR2_CPU_PCIE_MSTR_FWD_ERR(pdrv->core_rev));
	        } else {
	            HCD_LOG("Core [%d] PCIe Intr [Unknown_0x%x]\r\n", pdrv->core_id, reg_val);
	            reg_val = 0x0;
	        }
	        i++;
	    }
	}

	HCD_FN_EXT();

	return (i > 0) ? IRQ_HANDLED : IRQ_NONE;
}

/*
 *
 * Function bcm963xx_pcie_errlog_enable (pdrv)
 *
 *
 *   Parameters:
 *    pdrv ... pointer to pcie core hcd data structure
 *
 *   Description:
 *       Setup pcie core bus error logging
 *
 *  Return: 0 on success, -ve on failure
 */
static int bcm963xx_pcie_errlog_enable(struct bcm963xx_pcie_hcd *pdrv)
{
	uint32 reg_data;
	int err;

	HCD_FN_ENT();

	if (pdrv->errlog_inited == TRUE) {
	    /* Error logging already initialized nothing to do */
	    return 0;
	}

	if (HCD_USE_DT_ENTRY(pdrv->core_id) && (pdrv->resources.irq == 0)) {
	    err = platform_get_irq_byname(pdrv->pdev, "intr");
	    if (err > 0) {
	        pdrv->resources.irq = err;
	    } else {
	        HCD_ERROR("failed to get msi intr from DT: %d\n", err);
	        return 0;
	    }
	}

	err = request_irq(pdrv->resources.irq, bcm963xx_pcie_errlog_isr, IRQF_SHARED,
	    pdrv->pdev->name, pdrv);
	if (err < 0) {
	    HCD_ERROR("failed to request IRQ[%d]: %d\n", pdrv->resources.irq, err);
	    return err;
	}

	/* PCIe Core Interrupts (PCIE_INTR2) */
	/* Enable Bad address and bad access interrupts */
	reg_data = INTR2_CPU_PCIE_INTR_MASK(pdrv->core_rev);
	hcd_writel(reg_data, pdrv->base, INTR2_CPU_MASK_CLEAR_OFFSET);

	/* setup PCIE Core CPU interrupts */
	reg_data = CPU_INTR1_PCIE_INTR_CPU_INTR;
	hcd_writel(reg_data, pdrv->base, CPU_INTR1_INTR_MASK_CLEAR_OFFSET);

	/* UBUS Interrupts */
	/* Enable UBUS Intr2 controller interrupt */
	reg_data = UBUS_INTR2_PCIE_INTR_MASK;
	hcd_writel(reg_data, pdrv->base, UBUS_INTR2_CPU_MASK_CLEAR_OFFSET);

	/* setup UBUS CPU interrupts */
	reg_data = CPU_INTR1_PCIE_UBUS_CPU_INTR;
	hcd_writel(reg_data, pdrv->base, CPU_INTR1_INTR_MASK_CLEAR_OFFSET);

	pdrv->errlog_inited = TRUE;

	HCD_LOG("Core [%d] Enabled PCIE/UBUS Error Interrupts\r\n", pdrv->core_id);

	HCD_FN_EXT();

	return err;
}

/*
 *
 * Function bcm963xx_pcie_errlog_disable (pdrv)
 *
 *
 *   Parameters:
 *    pdrv ... pointer to pcie core hcd data structure
 *
 *   Description:
 *       Setup pcie core bus error logging
 *
 *  Return: 0 on success, -ve on failure
 */
static void bcm963xx_pcie_errlog_disable(struct bcm963xx_pcie_hcd *pdrv)
{
	HCD_FN_ENT();

	if (pdrv->errlog_inited == FALSE) {
	    /* Error logging is not initialized, nothing to do */
	    return;
	}


	/* Disable PCIE Core CPU interrupts */
	hcd_writel(CPU_INTR1_PCIE_INTR_CPU_INTR, pdrv->base, CPU_INTR1_INTR_MASK_SET_OFFSET);

	/* Disable UBUS CPU interrupts */
	hcd_writel(CPU_INTR1_PCIE_UBUS_CPU_INTR, pdrv->base, CPU_INTR1_INTR_MASK_SET_OFFSET);

	if (pdrv->resources.irq > 0)
	    free_irq(pdrv->resources.irq, pdrv);

	pdrv->errlog_inited = FALSE;

	HCD_LOG("Core [%d] Disabled PCIE/UBUS Error Interrupts\r\n", pdrv->core_id);

	HCD_FN_EXT();

	return;
}


/*
 *
 * Function bcm963xx_pcie_setup_regs (pdrv)
 *
 *
 *   Parameters:
 *    pdrv ... pointer to pcie core hcd data structure
 *
 *   Description:
 *       Map  pcie core registers resource
 *
 *  Return: 0 on success, -ve on failure
 */
static int bcm963xx_pcie_setup_regs(struct bcm963xx_pcie_hcd *pdrv)
{

	struct bcm963xx_pcie_hc_res *pres = NULL;
	struct platform_device *pdev = NULL;
	struct device *dev;
	uint32 reg_data;
	int    chip_id = pmc_hcd_get_chip_id();
	int    chip_rev = pmc_hcd_get_chip_rev();

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

	HCD_INFO("pcie core [%d] mapped reg base [0x%px]\r\n", pdev->id,
	    pdrv->base);

	reg_data = hcd_readl(pdrv->base, RC_CFG_TYPE1_DEVICE_VENDOR_ID_OFFSET);
	HCD_INFO("[%d] Vendor [0x%4x] device [0x%4x]\r\n",
	    pdev->id, (reg_data&0xFFFF), ((reg_data >> 16)&0xFFFF));

	if (reg_data == 0xdeaddead) {
	    HCD_ERROR("pcie core [%d] access returned invalid value 0x%x\r\n",
	        pdev->id, reg_data);
	    return -ENODEV;
	}

	reg_data = hcd_readl(pdrv->base, RC_CFG_TYPE1_REV_ID_CLASS_CODE_OFFSET);
	HCD_INFO("[%d] Rev [0x%2x] Class [0x%6x]\r\n",
	    pdev->id, (reg_data&0xFF), ((reg_data >> 8)&0xFFFFFF));

	pdrv->core_rev = hcd_readl(pdrv->base, MISC_REVISION_OFFSET);

	/* Get the Link Width and speed gen configuration from the core */
	reg_data = hcd_readl(pdrv->base, RC_CFG_PCIE_LINK_CAPABILITY_OFFSET);
	pres->link_width = RC_CFG_PCIE_LINK_CAP_LINK_WIDTH(reg_data);
	pdrv->core_gen = RC_CFG_PCIE_LINK_CAP_LINK_SPEED(reg_data);

	HCD_LOG("found port [%d] GEN%d Core Rev [%x.%02x] with %d Lanes\r\n",
	    pdev->id, pdrv->core_gen, ((pdrv->core_rev >> 8)&0xFF),
	    (pdrv->core_rev&0xFF), pres->link_width);

	if (pdrv->core_rev < 0x303) {
	    pdrv->wars.g2defset = 1;
	    HCD_LOG("Enable GEN2 Default settings WAR on Core %d\r\n", pdrv->core_id);
	} else {
	    pdrv->wars.g2defset = 0;
	}

	if ((pdrv->core_rev == 0x320) && (pdrv->core_id == 1) &&
	    (((chip_id == 0x68360) && (chip_rev == 0xA0)) ||
	    ((chip_id == 0x6858) && (chip_rev == 0xB0)))) {
	    /* For 68360A0, 6858B0 enable Bifurcation System Clock WAR on port 1 */
	    pdrv->wars.bifp1sysclk = 1;
	    HCD_LOG("Enable bifp1sysclk WAR on Core %d\r\n", pdrv->core_id);
	} else {
	    pdrv->wars.bifp1sysclk = 0;
	}

	if (pdrv->core_gen == PCIE_LINK_SPEED_GEN3) {
	    if ((pdrv->core_rev == 0x320) && (chip_id == 0x63158) && (chip_rev == 0xA0)) {
	        pdrv->wars.g3txclk = 1;
	        pdrv->wars.g3rxset = 1;
	    } else {
	        pdrv->wars.g3txclk = 0;
	        pdrv->wars.g3rxset = 0;
	    }
	}

	HCD_FN_EXT();

	return 0;
}


/*
 *
 * Function bcm963xx_pcie_setup_owin (pdrv, resources)
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
static int bcm963xx_pcie_setup_owin(struct bcm963xx_pcie_hcd *pdrv,
	struct list_head *resources)
{
	struct bcm963xx_pcie_hc_res *pres = NULL;
	struct platform_device *pdev = NULL;
	struct device *dev;
	int err = 0;
	int win;

	HCD_FN_ENT();

	pdev = pdrv->pdev;
	dev = &pdev->dev;
	pres = &pdrv->resources;

	for (win = OWIN0; win < NUM_OUTGOING_WINDOWS; win++) {

	    if (!OWIN_RES_CONFIGURED(pdrv, win)) continue;

	    err =  devm_request_resource(dev, &iomem_resource, &pres->owin[win]);
	    if (err) {
	        HCD_ERROR("[%d] pcie failed to create own resource: [%d]\r\n",
	            pdev->id, err);

	        HCD_FN_EXT();
	        return err;
	    }
	    else
	        HCD_INFO("[%d] mapped pcie owin base [0x%llx]\r\n", pdev->id,
	            (u64)pres->owin[win].start);


	    pci_add_resource_offset(resources,
	                &pres->owin[win],
	                pres->owin[win].start - pres->pci_addr[win]);

	    pdrv->owin_inited[win] = TRUE;
	}

	HCD_FN_EXT();

	return err;
}

/*
 *
 * Function bcm963xx_pcie_unmap_res (pdrv)
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
static void bcm963xx_pcie_unmap_res(struct bcm963xx_pcie_hcd *pdrv)
{
	struct bcm963xx_pcie_hc_res *pres = NULL;
	struct platform_device *pdev = NULL;
	struct device *dev;
	int win;

	HCD_FN_ENT();

	if (pdrv == NULL)
	    return;

	pdev = pdrv->pdev;
	dev = &pdev->dev;
	pres = &pdrv->resources;

	for (win = OWIN0; win < NUM_OUTGOING_WINDOWS; win++) {
	    if (pdrv->owin_inited[win] == TRUE) {
	        devm_release_resource(dev, &pres->owin[win]);
	        HCD_INFO("release owin[%d] [0x%llx]\r\n", win, (u64)pres->owin[win].start);
	        pdrv->owin_inited[win] = FALSE;
	    }
	}
	if (pdrv->base) {
	    devm_iounmap(dev, pdrv->base);
	    HCD_INFO("unmap reg base [0x%px]\r\n", pdrv->base);
	    pdrv->base = NULL;
	}

	return;
}

/*
 *
 * Function bcm963xx_pcie_parse_dt (pdrv)
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
static int bcm963xx_pcie_parse_dt(struct bcm963xx_pcie_hcd *pdrv)
{

	struct bcm963xx_pcie_hc_res *pres = NULL;
	struct platform_device *pdev = NULL;
	struct device_node *np = NULL;
	struct device *dev = NULL;
	int win;
	int err = 0;
	struct of_pci_range_parser parser;
	struct of_pci_range range;

	HCD_FN_ENT();

	pdev = pdrv->pdev;
	pres = &pdrv->resources;

	/* Initialize attributes from device tree if present */
	np = pdev->dev.of_node;
	dev = &pdev->dev;
	if (np) {
	    HCD_LOG("[%d] DT node available: %s\n", pdev->id, np->full_name);

	    /* Check if DT entry status is enabled or okay to load */
	    if (!of_device_is_available(np)) {
	        dev_err(dev, "DT status disabled\n");
	        return -ENODEV;
	    }

	    /* PCIe core registers base */
	    err = of_address_to_resource(np, 0, &pres->base);
	    HCD_FAIL_ON_DT_ERROR("reg (core base)", err);

	    /* PCIe Outgoing window Memory ranges */
	    err = of_pci_range_parser_init(&parser, np);
	    HCD_FAIL_ON_DT_ERROR("ranges (outgoing window)", err);

	    win = OWIN0;
	    /* Parse the ranges and add the resources found to the list */
	    for_each_of_pci_range(&parser, &range) {

	        if (win >= NUM_OUTGOING_WINDOWS) {
	            HCD_ERROR("[%d] ougoing windows [%d] exceeds max [%d]\n",
	                pdev->id, win, NUM_OUTGOING_WINDOWS);
	            return -EINVAL;
	        }

	        if (win == OWIN0) {
	            /*
	             * If specified address base is different from default,
	             * or size is different from base, we might need to remap
	             * the outgoing window in the UBUS
	             */
	            if ((pres->owin[win].start != range.cpu_addr) ||
	                (pres->owin[win].end != (range.cpu_addr + range.size - 1))) {
	                pdrv->owin_need_remap[win] = 1;
	            }
	        } else {
	            pdrv->owin_need_remap[win] = 1;
	        }

	        pres->owin[win].flags = range.flags;
	        pres->owin[win].parent = pres->owin[win].child = pres->owin[win].sibling = NULL;
	        pres->owin[win].name = np->full_name;
	        pres->owin[win].start = range.cpu_addr;
	        pres->owin[win].end = range.cpu_addr + range.size - 1;
	        pres->pci_addr[win] = range.pci_addr;

	        win++;
	    }

	    pres->irq = irq_of_parse_and_map(np, 0);
	    err = (pres->irq) ? 0 : 1;
	    HCD_FAIL_ON_DT_ERROR("interrupt", err);

	    /* PCI bus range */
	    err = of_pci_parse_bus_range(np, &pres->bus_range);
	    HCD_WARN_ON_DT_ERROR("busnumber", err);
	    if (pres->bus_range.end > BCM963XX_MAX_BUSNUM)
	        pres->bus_range.end = BCM963XX_MAX_BUSNUM;

	    /* PCIe Link width */
	    err = of_property_read_u32(np, "brcm,num-lanes", &pres->link_width);
	    HCD_WARN_ON_DT_ERROR("brcm,num-lanes", err);
	    if (pres->link_width > BCM963XX_MAX_LINK_WIDTH)
	        pres->link_width = BCM963XX_MAX_LINK_WIDTH;

	    /* PCIe port speed */
	    err = of_property_read_u8(np, "brcm,speed", &pdrv->hc_cfg.speed);
	    HCD_WARN_ON_DT_ERROR("brcm,speed", err);

	    /* PCIe port power mode */
	    err = of_property_read_u8(np, "brcm,phypwrmode", &pdrv->hc_cfg.phypwrmode);
	    HCD_WARN_ON_DT_ERROR("brcm,phypwrmode", err);

	    /* PCIe port error logging */
	    err = of_property_read_u8(np, "brcm,errlog", &pdrv->hc_cfg.errlog);
	    HCD_WARN_ON_DT_ERROR("brcm,errlog", err);

	    {
	        u8 dt_val;

	        /* PCIe ssc configuration */
	        err = of_property_read_u8(np, "brcm,ssc", &dt_val);
	        if (err == 0) {
	            pdrv->hc_cfg.ssc = (dt_val) ? TRUE : FALSE;
	        }
	        HCD_WARN_ON_DT_ERROR("brcm,ssc", err);

	        /* PCIe force power on setting */
	        err = of_property_read_u8(np, "brcm,apon", &dt_val);
	        if (err == 0) {
	            pdrv->hc_cfg.apon = (dt_val > HCD_APON_LAST) ? HCD_APON_DEFAULT : dt_val;
	        }
	        HCD_WARN_ON_DT_ERROR("brcm,apon", err);
	    }
	}

	HCD_INFO("[%d] regs: start [0x%llx] end [0x%llx] flags [0x%lx]\r\n",
	    pdev->id, (u64)pres->base.start, (u64)pres->base.end,
	    pres->base.flags);
	HCD_INFO("[%d] Interrupt [%d]\r\n", pdev->id, pres->irq);
	HCD_INFO("[%d] bus_range: start [0x%llx] end [0x%llx] flags [0x%lx]\r\n",
	    pdev->id, (u64)pres->bus_range.start, (u64)pres->bus_range.end,
	    pres->bus_range.flags);
	for (win = OWIN0; win < NUM_OUTGOING_WINDOWS; win++) {
	    if (pres->owin[win].start) {
	        HCD_INFO("[%d] owin[%d]: start [0x%llx] end [0x%llx] flags [0x%lx]\r\n",
	            pdev->id, win, (u64)pres->owin[win].start, (u64)pres->owin[win].end,
	            pres->owin[win].flags);
	    }
	}
	HCD_INFO("[%d] lanes: [%d] speed: [%d] ssc: [%d] phypwrmode: [%d]\r\n", pdev->id,
	    pres->link_width, pdrv->hc_cfg.speed, pdrv->hc_cfg.ssc, pdrv->hc_cfg.phypwrmode);

	HCD_FN_EXT();

	return 0;
}


/*
 *
 * Function bcm963xx_pcie_init_res (pdrv)
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
static int bcm963xx_pcie_init_res(struct bcm963xx_pcie_hcd *pdrv)
{

	struct bcm963xx_pcie_hc_res *pres = NULL;
	struct platform_device *pdev = NULL;
	int win;

	HCD_FN_ENT();

	pdev = pdrv->pdev;
	pres = &pdrv->resources;

	for (win = OWIN0; win < NUM_OUTGOING_WINDOWS; win++) {
	    pres->owin[win].start = pres->owin[win].end = 0;
	    pres->owin[win].flags = pres->pci_addr[win] = 0;
	    pdrv->owin_need_remap[win] = 0;
	}

	/* Initialize attributes with default values */
	switch (pdrv->core_id) {
	    case 0:
	        pres->base.start = PCIE0_PHYS_BASE;
	        pres->owin[OWIN0].name = dev_name(&pdev->dev);
	        pres->owin[OWIN0].start = PCIE0_MEM_PHYS_BASE;
	        pres->owin[OWIN0].end = pres->owin[OWIN0].start + BCM963XX_PCIE_MEMS_SIZE - 1;
	        pres->irq = INTERRUPT_ID_PCIE0;
	        break;
#if defined(PCIE1_PHYS_BASE)
	    case 1:
	        pres->base.start = PCIE1_PHYS_BASE;
	        pres->owin[OWIN0].name = dev_name(&pdev->dev);
	        pres->owin[OWIN0].start = PCIE1_MEM_PHYS_BASE;
	        pres->owin[OWIN0].end = pres->owin[OWIN0].start + BCM963XX_PCIE_MEMS_SIZE - 1;
	        pres->irq = INTERRUPT_ID_PCIE1;
	        break;
#endif /* PCIE1_PHYS_BASE */
#if defined(PCIE2_PHYS_BASE)
	    case 2:
	        pres->base.start = PCIE2_PHYS_BASE;
	        pres->owin[OWIN0].name = dev_name(&pdev->dev);
	        pres->owin[OWIN0].start = PCIE2_MEM_PHYS_BASE;
	        pres->owin[OWIN0].end = pres->owin[OWIN0].start + BCM963XX_PCIE_MEMS_SIZE - 1;
	        pres->irq = INTERRUPT_ID_PCIE2;
	        break;
#endif /* PCIE2_PHYS_BASE */
#if defined(PCIE3_PHYS_BASE)
	    case 3:
	        pres->base.start = PCIE3_PHYS_BASE;
	        pres->owin[OWIN0].name = dev_name(&pdev->dev);
	        pres->owin[OWIN0].start = PCIE3_MEM_PHYS_BASE;
	        pres->owin[OWIN0].end = pres->owin[OWIN0].start + BCM963XX_PCIE_MEMS_SIZE - 1;
	        pres->irq = INTERRUPT_ID_PCIE3;
	        break;
#endif /* PCIE3_PHYS_BASE */
	    default:
	        return -1;
	}

	pres->base.end = pres->base.start + BCM963XX_PCIE_PHYS_SIZE - 1;
	pres->base.flags = IORESOURCE_MEM;

	pres->owin[OWIN0].flags = IORESOURCE_MEM;
	pres->pci_addr[OWIN0] = pres->owin[OWIN0].start;

	pres->bus_range.start = BCM963XX_ROOT_BUSNUM;
	pres->bus_range.end = BCM963XX_MAX_BUSNUM;
	pres->bus_range.flags = IORESOURCE_BUS;


	pres->domain = pdrv->core_id;

	pres->link_width = PCIE_LINK_WIDTH_1LANE;

	/* PCIe port configuration */
	bcm963xx_pcie_init_hc_cfg(pdrv);

	HCD_INFO("[%d] regs: start [0x%llx] end [0x%llx] flags [0x%lx]\r\n",
	    pdev->id, (u64)pres->base.start, (u64)pres->base.end,
	    pres->base.flags);
	HCD_INFO("[%d] Interrupt [%d]\r\n", pdev->id, pres->irq);
	HCD_INFO("[%d] bus_range: start [0x%llx] end [0x%llx] flags [0x%lx]\r\n",
	    pdev->id, (u64)pres->bus_range.start, (u64)pres->bus_range.end,
	    pres->bus_range.flags);
	HCD_INFO("[%d] owin[0]: start [0x%llx] end [0x%llx] flags [0x%lx]\r\n",
	    pdev->id, (u64)pres->owin[OWIN0].start, (u64)pres->owin[OWIN0].end,
	    pres->owin[OWIN0].flags);

	HCD_FN_EXT();

	return 0;
}

/*
 *
 * Function bcm963xx_pcie_probe (pdrv)
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
static int bcm963xx_pcie_probe(struct platform_device *pdev)
{
	struct bcm963xx_pcie_hcd *pdrv = NULL;
	struct device_node *np = pdev->dev.of_node;
	uint32 core = (uint32)pdev->id;
	int	err = 0;
	struct pci_bus *bus;
	struct pci_bus *child;
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

	if (core >= NUM_CORE)
	    return -ENODEV;

	if (pcie_hcd_port_enabled(core)) {
	    if (pcie_hcd_get_port_mode(core) == 0) {
	        HCD_ERROR("[%d] Not in RC Mode\r\n", core);
	        err = -ENODEV;
	        goto error;
	    }

	    /* Port is enabled and is in RC mode */
	    pcie_hcd_pmc_power_up(core);
	    HCD_INFO("core [%d] powered-up\n", core);

	    /* Allocate HCD control block */
	    pdrv = kzalloc(sizeof(*pdrv), GFP_KERNEL);
	    if (!pdrv) {
	        HCD_ERROR("[%d] Unable to allocate memory for CB\r\n", core);
	        err =  -ENOMEM;
	        goto error;
	    }
	    HCD_INFO("[%d] Allocated [0x%px] hcd\r\n", core, pdrv);

	    /* Initialize  hcd elements */
	    pdrv->core_id = core;
	    pdrv->pdev = pdev;
	    platform_set_drvdata(pdev, pdrv);
	    pcie_hcd_procfs_init(pdrv);

	    /* Initialize  core resource element values for no device tree based
	     * legacy drivers
	     */
	    bcm963xx_pcie_init_res(pdrv);

	    /* Update core resource elements  (DT based) */
	    err = bcm963xx_pcie_parse_dt(pdrv);
	    if (err) {
	        HCD_ERROR("failed to update core [%d] dt entries\r\n", core);
	        err =  -EINVAL;
	        goto error;
	    }

	    /* if Configured, skip port enumeration and power off the port */
	    if (pdrv->hc_cfg.apon == HCD_APON_OFF) {
	        HCD_LOG("Skip core [%d] due to apon setting [%d]\r\n", core, pdrv->hc_cfg.apon);
	        err = -ENODEV;
	        goto error;
	    }

	    /* setup pcie Core registers for access to PCIe core */
	    err = bcm963xx_pcie_setup_regs(pdrv);
	    if (err) {
	        HCD_ERROR("failed to setup core[%d] regs, err [%d]\r\n", core,
	            err);
	        err =  -ENOMEM;
	        goto error;
	    }

	    if (pdrv->resources.link_width == 0) {
	        HCD_ERROR("core [%d] zero lanes, skip bring up\r\n", core);
	        err =  -ENODEV;
	        goto error;
	    }

	    /* lets talk to PCIe core, reset the core */
	    bcm963xx_pcie_core_reset(pdrv);

	    /* Check if PCIe link is up (for any device connected on the link) */
	    if (!(hcd_is_pcie_link_up(pdrv))) {
	        /* No device connected to PCIe core */
	        HCD_ERROR("core [%d] link is DOWN\r\n", core);
	        err =  -ENODEV;
	        goto error;
	    } else {
	        uint32 link_status;

	        link_status = hcd_readl(pdrv->base, RC_CFG_PCIE_LINK_STATUS_CONTROL_OFFSET);

	        HCD_LOG("core [%d] Link UP - [%d] lanes, [GEN%d] speed\r\n",
	            core, RC_CFG_PCIE_LINK_STAT_LINK_WIDTH(link_status),
	            RC_CFG_PCIE_LINK_STAT_LINK_SPEED(link_status));
	    }

	    /* Setup PCIe core memory window */
	    err = bcm963xx_pcie_setup_owin(pdrv, &res);
	    if (err) {
	        HCD_ERROR("core [%d] failed to setup owin resource, err [%d]\r\n",
	            core, err);
	        err =  -ENOMEM;
	        goto error;
	    }

	    /* Setup PCIe core bus numbers */
	    pci_add_resource(&res, &pdrv->resources.bus_range);

	    /* Got all driver resources. Now configure the PCIe core */
	    err = bcm963xx_pcie_core_config(pdrv);
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
	    bus = pci_create_root_bus(&pdev->dev, pdrv->resources.bus_range.start,
	        &bcm963xx_pcie_ops, pdrv, &res);
	    if (!bus) {
	        HCD_ERROR("core [%d] failed to create root bus: %d\r\n", core, err);
	        err =  -ENXIO;
	        goto error;
	    }

	    /* store the bus for proper remove */
	    pdrv->bus = bus;

	    /* if Configured, skip device enumeration and power off the port */
	    if (pdrv->hc_cfg.apon == HCD_APON_OFF_WITH_DOMAIN) {
	        HCD_LOG("Skip core [%d] initialization due to apon setting [%d]\r\n",
	            core, pdrv->hc_cfg.apon);

	        /* Force power off port */
	        pcie_hcd_pmc_power_down(pdev->id);
	        HCD_LOG("core [%d] powered down\r\n", pdrv->core_id);
	        goto done;
	    }

	    /* Now initialize the PCIe core error logging */
	    if (pdrv->hc_cfg.errlog) {
	        err = bcm963xx_pcie_errlog_enable(pdrv);
	        if (err) {
	            HCD_ERROR("core [%d] failed to setup error logging, err [%d]\r\n", core, err);
	            err =  -ENODEV;
	            goto error;
	        }
	    }

	    if (IS_ENABLED(CONFIG_PCI_MSI)) {
	        err = bcm963xx_pcie_msi_enable(pdrv);
	        if (err < 0) {
	            HCD_ERROR("failed to enable MSI support: %d\n", err);
	            goto error;
	        }
#ifdef CONFIG_ARM
#ifdef CONFIG_PCI_MSI
	        pdrv->sys.msi_ctrl = (struct msi_controller*)pdrv->msi;
#endif /* CONFIG_PCI_MSI */
#endif /* CONFIG_ARM */
	        bus->msi = (struct msi_controller*)pdrv->msi;
	    }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 13, 0))
	    {
	        struct pci_host_bridge *bridge = pci_host_bridge_from_priv(pdrv);
#ifdef CONFIG_PCI_MSI
	        bridge->msi = (struct msi_controller*)pdrv->msi;
#endif /* CONFIG_PCI_MSI */
	        bridge->map_irq = bcm963xx_pcie_map_irq;
	        bridge->swizzle_irq = pci_common_swizzle;
	    }
#endif /* LINUX_VERSION >= 4.13.0 */

	    pci_scan_child_bus(bus);

	    pci_assign_unassigned_bus_resources(bus);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 13, 0))
#ifdef CONFIG_ARM
	    /* Apply irq fixup */
	    pci_fixup_irqs(pci_common_swizzle, bcm963xx_pcie_map_irq);
#endif /* CONFIG_ARM */
#endif /* < 4.13.0 */

	    pci_bus_add_devices(bus);

	    /* Configure PCI Express settings */
	    list_for_each_entry(child, &bus->children, node)
	        pcie_bus_configure_settings(child);

	    err = 0;
	} else {
	    HCD_ERROR("core [%d] disabled\n", core);
	    err = -ENODEV;
	}

error:

	if (err) {
	    bcm963xx_pcie_remove(pdev);
	}

	pci_free_resource_list(&res);

done:
	HCD_FN_EXT();

	return err;
}


/*
 *
 * Function bcm963xx_pcie_remove (pdrv)
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
static int  bcm963xx_pcie_remove(struct platform_device *pdev)
{
	struct bcm963xx_pcie_hcd *pdrv = platform_get_drvdata(pdev);
	int core = pdev->id;

	HCD_FN_ENT();

	if (!pdrv) {
	    HCD_FN_EXT();
	    return 0;
	}

	if (pcie_hcd_port_enabled(core)) {
	    if (pdrv->bus) {
	        pci_stop_root_bus(pdrv->bus);
	        pci_remove_root_bus(pdrv->bus);
	    }

	    if (IS_ENABLED(CONFIG_PCI_MSI)) {
	        bcm963xx_pcie_msi_disable(pdrv);
	    }

	    bcm963xx_pcie_errlog_disable(pdrv);

	    bcm963xx_pcie_unmap_res(pdrv);

	    if (pdrv->hc_cfg.apon != HCD_APON_ON) {
	        /* power off ports without link */
	        pcie_hcd_pmc_power_down(core);
	        HCD_LOG("core [%d] powered down\r\n", pdrv->core_id);
	    }

	    pcie_hcd_procfs_deinit(pdrv);

	    kfree(pdrv);
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
 * Function bcm963xx_pcie_init ()
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
static int __init bcm963xx_pcie_init(void)
{
	int ret;

	HCD_FN_ENT();

	printk("PCIe HCD (impl%d)\r\n", CONFIG_BCM_PCIE_HCD_IMPL);

#if defined(MODULE)
	if ((ret = bcm963xx_pcie_common_init()) != 0) {
	    HCD_ERROR("Failed to initialize nvram parameters\r\n");
		goto done;
	}

	/* Need to initialize platform dev for loadable kernel module */
	if ((ret = bcm963xx_pcie_plt_init()) != 0) {
	    HCD_ERROR("Failed to initialize platform device\r\n");
		goto done;
	}
#endif /* MODULE */

	if ((ret = platform_driver_register(&pcie_vcore_driver)) != 0) {
	    HCD_ERROR("Failed to register virtual pcie driver with platform driver\r\n");
	    goto done;
	}

	if ((ret = platform_driver_register(&bcm963xx_pcie_driver)) != 0) {
	    HCD_ERROR("Failed to register pcie driver with platform driver\r\n");
	}

done:
	HCD_FN_EXT();

	return ret;
}

/*
 *
 * Function bcm963xx_pcie_exit ()
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
static void __exit bcm963xx_pcie_exit(void)
{
	HCD_FN_ENT();

	platform_driver_unregister(&bcm963xx_pcie_driver);

	platform_driver_unregister(&pcie_vcore_driver);

#if defined(MODULE)
#if defined(CONFIG_BCM_KF_PCI_RESET_DOMAIN_NR)
	pci_reset_domain_nr();
#endif /* CONFIG_BCM_KF_PCI_RESET_DOMAIN_NR */
	bcm963xx_pcie_plt_deinit();
#endif /* MODULE */

	HCD_FN_EXT();

	return;
}

module_init(bcm963xx_pcie_init);
module_exit(bcm963xx_pcie_exit);
MODULE_LICENSE("GPL");

/*
 *
 * Function bcm963xx_pcie_plt_dev_release (dev)
 *
 *
 *	 Parameters:
 * pdrv ... pointer to pcie platform device data structure
 *
 *	 Description:
 *	  PCIe platform device release callback
 *
 *	Return: None
 */
static void bcm963xx_pcie_plt_dev_release(struct device *dev)
{
	HCD_FN_ENT();
	/* Nothing to do */
	HCD_FN_EXT();
}


/*
 *
 * Function bcm963xx_pcie_plt_init ()
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
static int __init bcm963xx_pcie_plt_init(void)
{
	int i, core;
	int ret = 0;
	struct platform_device	*pdev = NULL;

	HCD_FN_ENT();

	if (HCD_USE_DT_ENTRY(0)) {
	    /* Nothing to do, DT entries will populate the devices */
	    HCD_INFO("Using PCIe DT to probe\r\n");
	    HCD_FN_EXT();
	    return ret;
	}

	/* Register All Pcie cores as platform devices */
	for (i = 0; i < NUM_PCIE_CORES; i++) {
	    core = bcm963xx_pcie_get_boot_order_core(i);
	    if (core >= NUM_PCIE_CORES) {
	        HCD_LOG("Core [%d] not in range (> %d)\r\n", core, NUM_PCIE_CORES);
	        continue;
	    }

	    pdev = &bcm963xx_pcie_plt_dev[core];
	    if (IS_PCIE_VCORE(core)) {
	        pdev->name = PCIE_VCORE_DEV_NAME;
	    } else {
	        pdev->name = BCM963XX_PCIE_DEV_NAME;
	    }
	    pdev->id = core;
	    pdev->dev.release = bcm963xx_pcie_plt_dev_release;

	    ret = platform_device_register(pdev);
	    if (ret) {
	        HCD_ERROR("unable to register PCIe core [%d] device: ret = %d\n",
	            core, ret);
	    } else {
	        HCD_INFO("registered PCIe core [%d] device successfully\r\n", core);
	    }
	}

	HCD_FN_EXT();

	return ret;
}
/*
 *
 * Function bcm963xx_pcie_plt_deinit ()
 *
 *
 *	 Parameters:
 *
 *	 Description:
 *	  PCIe platform de-initialization. Unregister PCIe cores to the platform devices
 *	  Generally this is done outside, but since there is no support outside,
 *	  it is done here.
 *
 *	Return: None
 */
static void __init bcm963xx_pcie_plt_deinit(void)
{
	int i, core;
	struct platform_device	*pdev = NULL;

	HCD_FN_ENT();

	if (HCD_USE_DT_ENTRY(0)) {
	    /* Nothing to do, DT entries will populate the devices */
	    HCD_INFO("Using PCIe DT to probe\r\n");
	    HCD_FN_EXT();
	    return;
	}

	/* Un register All Pcie cores as platform devices */
	for (i = 0; i < NUM_PCIE_CORES; i++) {
	    core = bcm963xx_pcie_get_boot_order_core(i);
	    if (core >= NUM_PCIE_CORES) {
	        HCD_LOG("Core [%d] not in range (> %d)\r\n", core, NUM_PCIE_CORES);
	            continue;
	    }

	    pdev = &bcm963xx_pcie_plt_dev[core];
	    if (IS_PCIE_VCORE(core)) {
	        pdev->name = PCIE_VCORE_DEV_NAME;
	    } else {
	        pdev->name = BCM963XX_PCIE_DEV_NAME;
	    }
	    pdev->id = core;

	    platform_device_unregister(pdev);
	    HCD_INFO("unregister PCIe core [%d] device successfully\r\n", core);
	}

	HCD_FN_EXT();

	return;
}

#if !defined(MODULE)
/* Only for built-in drivers */
subsys_initcall(bcm963xx_pcie_plt_init);
#endif /* !MODULE */
#endif /* CONFIG_BCM_KF_ARM_BCM963XX */
