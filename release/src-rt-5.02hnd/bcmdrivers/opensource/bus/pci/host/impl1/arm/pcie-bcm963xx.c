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
#include <pmc_pcie.h>
#include <pmc_drv.h>
#include <shared_utils.h>

#include <pcie_common.h>
#include <pcie-bcm963xx.h>

#if defined(GTAC5300)
#define CHANGE_PROBE_ORDER
#endif

#ifdef CHANGE_PROBE_ORDER
#if NUM_CORE != 3
#error "NUM_CORE != 3"
#endif
static int pid2core[NUM_CORE] = {1, 2, 0};
#endif

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
#define BCM963XX_ROOT_BUSNUM                       0x00
#define BCM963XX_MAX_BUSNUM                        0xFF

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

#ifdef CONFIG_OF
#ifndef INTERRUPT_ID_PCIE0
#define INTERRUPT_ID_PCIE0                         0
#define INTERRUPT_ID_PCIE1                         0
#define INTERRUPT_ID_PCIE2                         0
#define FAIL_ON_DT_ERROR
#endif /* !INTERRUPT_ID_PCIE0 */
#endif /* CONFIG_OF */

#define HCD_USE_DT_ENTRY(core)                     \
	((INTERRUPT_ID_PCIE0) ? FALSE : TRUE)

/* don't fail on DT error until DT is tested and ready working */
#ifdef FAIL_ON_DT_ERROR
#define HCD_FAIL_ON_DT_ERROR(res, err)             \
	if (err) {                                     \
	    HCD_WARN("No DT entry for %s\n",res);      \
	    return err;                                \
	}
#else
#define HCD_FAIL_ON_DT_ERROR(res, err)             \
	if (err)                                       \
	    HCD_WARN("No DT entry for %s, using defaults\n",res);
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
/***********
  * MISC
  ***********/
static void bcm963xx_misc_set_pcie_reset(
	struct platform_device *pdev, int core, bool enable);

/***********
  * MSI
  ***********/
static int bcm963xx_pcie_msi_alloc(struct bcm963xx_pcie_msi *msi);
static void bcm963xx_pcie_msi_free(struct bcm963xx_pcie_msi *msi,
	unsigned long irq);
static int bcm963xx_pcie_msi_setup(struct msi_controller *chip,
	struct pci_dev *pdev, struct msi_desc *desc);
static void bcm963xx_pcie_msi_teardown(struct msi_controller *chip,
	unsigned int irq);
static int bcm963xx_pcie_msi_map(struct irq_domain *domain,
	unsigned int irq, irq_hw_number_t hwirq);
static irqreturn_t bcm963xx_pcie_msi_isr(int irq, void *data);
static int bcm963xx_pcie_msi_enable(struct bcm963xx_pcie_hcd *pdrv);
static int bcm963xx_pcie_msi_disable(struct bcm963xx_pcie_hcd *pdrv);

static void __iomem *bcm963xx_pcie_map_bus(struct pci_bus *bus,
	unsigned int devfn, int where);
static int bcm963xx_pcie_map_irq(const struct pci_dev *pcidev, u8 slot,
	u8 pin);

static void bcm963xx_pcie_fixup_final_mps(struct pci_dev *dev);

uint16 bcm963xx_pcie_mdio_read (struct bcm963xx_pcie_hcd *pdrv,
	uint16 phyad, uint16 regad);
int bcm963xx_pcie_mdio_write (struct bcm963xx_pcie_hcd *pdrv,
	uint16 phyad, uint16 regad, uint16 wrdata);

static void bcm963xx_pcie_phy_config(struct bcm963xx_pcie_hcd *pdrv);
static int bcm963xx_pcie_core_set_speed(struct bcm963xx_pcie_hcd *pdrv);
static int bcm963xx_pcie_core_config(struct bcm963xx_pcie_hcd *pdrv);
static void bcm963xx_pcie_core_reset(struct bcm963xx_pcie_hcd *pdrv);


static int bcm963xx_pcie_setup_owin(struct bcm963xx_pcie_hcd *pdrv,
	struct list_head *resources);
static int bcm963xx_pcie_setup_regs(struct bcm963xx_pcie_hcd *pdrv);
static void bcm963xx_pcie_unmap_res(struct bcm963xx_pcie_hcd *pdrv);
static int bcm963xx_pcie_parse_dt(struct bcm963xx_pcie_hcd *pdrv);
static int bcm963xx_pcie_init_res(struct bcm963xx_pcie_hcd *pdrv);

static int __init bcm963xx_pcie_probe(struct platform_device *pdev);
static int __exit bcm963xx_pcie_remove(struct platform_device *pdev);
static int __init bcm963xx_pcie_init(void);
static void __exit bcm963xx_pcie_exit(void);
static int __init bcm963xx_pcie_plt_init(void);



/**************************************
 *
 *  external Function prototype
 *
 **************************************/
extern unsigned long getMemorySize(void);


/**************************************
 *
 *  Global variables
 *
 **************************************/
static struct platform_device bcm963xx_pcie_plt_dev[NUM_CORE];

static struct pci_ops bcm963xx_pcie_ops = {
	.map_bus = bcm963xx_pcie_map_bus,
	.read = pci_generic_config_read,
	.write = pci_generic_config_write,
};

static const struct of_device_id bcm963xx_pcie_of_match[] = {
	{
	    .type = "pci",
	    .compatible = "brcm,bcm963xx_pcie",
	},
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

/***********
  * MSI
  ***********/
static char bcm963xx_pcie_msi_name[NUM_CORE][MSI_ISR_NAME_STR_LEN];

/**************************************
 *
 *  Local inline functions
 *
 **************************************/
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


/**************************************
 *
 *  Local Functions
 *
 **************************************/
/**********************
 * MISC block Local Functions
 **********************/
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
	volatile struct Misc *misc_regs = (struct Misc *)MISC_BASE;

	if (enable == TRUE)
	    /* Soft Reset the core  */
	    misc_regs->miscPCIECtrl &= ~(1<<core);
	else
	    /* Bring the core out of Soft Reset  */
	    misc_regs->miscPCIECtrl |= (1<<core);

	return;
}

/*****************
 * MSI Local Functions
 *****************/

/*
  *
  * Function bcm963xx_pcie_msi_alloc (msi)
  *
  *
  *   Parameters:
  *     msi ... msi control block pointer
  *
  *
  *   Description:
  *     Allocate an unused msi irq number for the client to use
  *
  *
  *  Return: msi irq number on success, -ve value on failure
  */
static int bcm963xx_pcie_msi_alloc(struct bcm963xx_pcie_msi *msi)
{
	int irq;

	HCD_FN_ENT();

	mutex_lock(&msi->lock);

	irq = find_first_zero_bit(msi->used, msi->map_size);
	if (irq < msi->map_size)
	    set_bit(irq, msi->used);
	else
	    irq = -ENOSPC;

	mutex_unlock(&msi->lock);

	HCD_INFO("Allocated MSI Hw IRQ %d\r\n",irq);

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

	HCD_INFO("Freeing MSI Hw IRQ %lu\r\n",irq);

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
  * Function bcm963xx_pcie_msi_setup (chip, pdev, desc)
  *
  *
  *   Parameters:
  *  chip ... msi chip control block pointer
  *  pdev  ... pci device pointer
  *  desc  ... msi irq descriptior
  *
  *   Description:
  *     Allocate one free MSI interrupt, Map MSI interrupt to system
  *     virtual interrupt, send an MSI message for the matching adress & data
  *
  *
  *  Return: 0 on success, -ve value on failure
  */
static int bcm963xx_pcie_msi_setup(struct msi_controller *chip,
	struct pci_dev *pdev, struct msi_desc *desc)
{
	struct bcm963xx_pcie_msi *msi = to_bcm963xx_pcie_msi(chip);
	struct msi_msg msg;
	unsigned int irq;
	int hwirq;

	HCD_FN_ENT();

	hwirq = bcm963xx_pcie_msi_alloc(msi);

	if (hwirq < 0) {
	    HCD_ERROR("failed to allocate IRQ: %d\n", hwirq);
	    return hwirq;
	}

	irq = irq_create_mapping(msi->domain, hwirq);
	if (!irq) {
	    HCD_ERROR("failed to create IRQ mapping for %d\n", hwirq);
	    bcm963xx_pcie_msi_free(msi, irq);
	    return -EINVAL;
	}

	irq_set_msi_desc(irq, desc);

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
	set_irq_flags(irq, IRQF_VALID);

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
	struct bcm963xx_pcie_hcd *pdrv = (struct bcm963xx_pcie_hcd*)data;
	struct bcm963xx_pcie_msi *msi = pdrv->msi;
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
	    if ( reg_val & ( 1ul << (i+msi->intr_bitshift))) {
	        if (i < msi->map_size) {
	            irq = irq_find_mapping(msi->domain, i);
	            if (irq) {
	                if (test_bit(i, msi->used))
	                    generic_handle_irq(irq);
	                else
	                    HCD_INFO("unexpected MSI %d\n",i);
	            } else {
	                /* that's weird who triggered this?*/
	                /* just clear it*/
	                HCD_INFO("Un handled MSI %d\n", i);
	            }
	        }
	        reg_val &= (~( 1ul << (i+msi->intr_bitshift)));
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
  * 	  Allocate msi control block, initialize msi control block parameters,
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

	HCD_INFO("Allocated [0x%p] hcd\r\n",msi);


	mutex_init(&msi->lock);
	snprintf(bcm963xx_pcie_msi_name[pdrv->core_id], MSI_ISR_NAME_STR_LEN,
	    "msi_pcie:%d",pdrv->core_id);

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
	} else {
	    msi->intr_status = INTR2_CPU_STATUS_OFFSET;
	    msi->intr_clear = INTR2_CPU_CLEAR_OFFSET;
	    msi->intr_mask_set = INTR2_CPU_MASK_SET_OFFSET;
	    msi->intr_mask_clear = INTR2_CPU_MASK_CLEAR_OFFSET;
	    msi->intr_bitshift = INTR2_CPU_MSI_INTR_SHIFT;
	    msi->intr_bitmask = INTR2_CPU_MSI_INTR_MASK;
	}
	msi->map_size = MSI_MAP_MAX_SIZE - msi->intr_bitshift;

	msi->chip.dev = &pdev->dev;
	msi->chip.setup_irq = bcm963xx_pcie_msi_setup;
	msi->chip.teardown_irq = bcm963xx_pcie_msi_teardown;
	msi->domain = irq_domain_add_linear(pdev->dev.of_node, msi->map_size,
	    &msi->domain_ops, &msi->chip);
	if (!msi->domain) {
	    HCD_ERROR("failed to create IRQ domain\n");
	    return -ENOMEM;
	}

	err = request_irq(pdrv->resources.irq, bcm963xx_pcie_msi_isr, IRQF_SHARED,
	    msi->irq_ops.name, pdrv);
	if (err < 0) {
	    HCD_ERROR("failed to request IRQ: %d\n", err);
	    goto err;
	}


	HCD_INFO("Using irq=%d for PCIE-MSI interrupts\r\n",pdrv->resources.irq);

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
	hcd_writel(reg_data, pdrv->base,MISC_MSI_DATA_CONFIG_OFFSET);

	/* Clear all MSI interrupts initially */
	reg_data = msi->intr_bitmask;
	hcd_writel(reg_data, pdrv->base, msi->intr_clear);


	/* enable all available MSI vectors */
	hcd_writel(reg_data, pdrv->base, msi->intr_mask_clear);

	/* Enable L2 Intr2 controller interrupt */
	reg_data = hcd_readl(pdrv->base, CPU_INTR1_INTR_MASK_CLEAR_OFFSET);
	reg_data |= CPU_INTR1_PCIE_INTR_CPU_INTR;
	hcd_writel(reg_data, pdrv->base, CPU_INTR1_INTR_MASK_CLEAR_OFFSET);

	set_irq_flags(pdrv->resources.irq, IRQF_VALID);

	/* Set the flag to specify MSI is enabled */
	msi->enabled = true;

	HCD_INFO("MSI Enabled\n");

	HCD_FN_EXT();

	return 0;

err:
	irq_domain_remove(msi->domain);

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
  * 	  Disable MSI feature on the hardware, Free the PCIe core isr and
  *     unmap and free all the MSI interrupts
  *
  *
  *  Return: 0 on success, -ve on failure
  */
static int bcm963xx_pcie_msi_disable(struct bcm963xx_pcie_hcd *pdrv)
{
	struct bcm963xx_pcie_msi *msi = pdrv->msi;
	unsigned int i, irq;
	u32 reg_data;

	HCD_FN_ENT();

	if (msi) {
	    /* Disable L2 Intr2 controller interrupt */
	    reg_data = CPU_INTR1_PCIE_INTR_CPU_INTR;
	    hcd_writel(reg_data, pdrv->base, CPU_INTR1_INTR_MASK_SET_OFFSET);

	    /* Disable all available MSI vectors */
	    reg_data = msi->intr_bitmask;
	    hcd_writel(reg_data, pdrv->base, msi->intr_mask_set);

	    /* Clear all mapped interrupts */
	    if (pdrv->resources.irq > 0)
	        free_irq(pdrv->resources.irq, pdrv);

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

	HCD_INFO("bus [0x%p] bus_no [%d] dev [%d] func [%d] where [%d]\r\n",
	    bus, bus_no, dev_no, func_no, where);

	/* RC config space is regisers not memory, allow only valid bus/dev combinations */
	if (bus_no <= (BCM963XX_ROOT_BUSNUM+1)) {
	    /* Root Conplex bridge, first device or switch */
	    /* Allow only configuration space (dev#0) */
	    valid = (dev_no == 0);
	} else if (bus_no == (BCM963XX_ROOT_BUSNUM+2)){
	    /* Switch UP stream port */
	    /* Allow access for all the DN ports */
	    valid =  TRUE;
	} else {
	    /* Switch down stream ports to devices */
	    /* Allow only configuration space (dev#0) */
	    valid = (dev_no == 0); /*otherwise will loop for the rest of the device*/
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


	HCD_INFO("config space mapped address = [0x%p]\r\n",pdrv->base + offset);

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
  * 	  Get the pcie core irq number.
  *
  *
  *  Return: pcie core irq number
  */
static int bcm963xx_pcie_map_irq(const struct pci_dev *pcidev,
	u8 slot,u8 pin)
{
	struct bcm963xx_pcie_hcd *pdrv = pcidev->bus->sysdata;

	if (pdrv->pdev->dev.of_node)
	    return of_irq_parse_and_map_pci(pcidev, slot, pin);
	else
	    return pdrv->resources.irq;
}

/*
  *
  * Function bcm963xx_pcie_fixup_final_mps (dev)
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
static void bcm963xx_pcie_fixup_final_mps(struct pci_dev *dev)
{
	/* sync-up mps. Changes allowed only in safe state */
	pcie_bus_config = PCIE_BUS_SAFE;
	if (dev->bus && dev->bus->self) {
	    pcie_bus_configure_settings(dev->bus);
	}
	return;
}
DECLARE_PCI_FIXUP_FINAL(PCI_VENDOR_ID_BROADCOM, PCI_ANY_ID, bcm963xx_pcie_fixup_final_mps);

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
uint16 bcm963xx_pcie_mdio_read (struct bcm963xx_pcie_hcd *pdrv,
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
	while (timeout-- > 0) {
	    data = hcd_readl(pdrv->base, RC_DL_MDIO_RD_DATA_OFFSET);
	    /* Bit-31=1 is DONE */
	    if (data & 0x80000000)
	        break;
	    timeout = timeout - 1;
	    udelay(1000);
	}

	if (timeout == 0) {
	    retval = 0xdead;
	} else
	    /* Bits 15:0 is read data*/
	    retval = (data&0xffff);

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
int bcm963xx_pcie_mdio_write (struct bcm963xx_pcie_hcd *pdrv,
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
	while (timeout-- > 0) {

	    data = hcd_readl(pdrv->base, RC_DL_MDIO_WR_DATA_OFFSET);

	    /* CTRL1 Bit-31=1 is DONE */
	    if ((data & 0x80000000) == 0 )
	        break;

	    timeout = timeout - 1;
	    udelay(1000);
	}

	HCD_FN_EXT();

	if (timeout == 0){
	    return 0;
	} else
	    return 1;
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
  *     Configure PCIe PHY through MDIO interface. The settings
  *     largely comes from ASIC desig team
  *
  *  Return: None
  */
static void bcm963xx_pcie_phy_config(struct bcm963xx_pcie_hcd *pdrv)
{
	HCD_FN_ENT();

#if defined(RCAL_1UM_VERT)
	/*
	 * Rcal Calibration Timers
	 *   Block 0x1000, Register 1, bit 4(enable), and 3:0 (value)
	 */
	{
	    int val = 0;
	    uint16 data = 0;
	    if(GetRCalSetting(RCAL_1UM_VERT, &val)== kPMC_NO_ERROR) {
	        HCD_INFO("setting resistor calibration value to 0x%x\n",
	            val);
	        bcm963xx_pcie_mdio_write(pdrv, 0, 0x1f , 0x1000);
	        data = bcm963xx_pcie_mdio_read (pdrv, 0, 1);
	        data = ((data & 0xffe0) | (val & 0xf) | (1 << 4)); /*enable*/
	        bcm963xx_pcie_mdio_write(pdrv, 0, 1, data);
	    }
	}
#endif

	HCD_INFO("applying serdes parameters chipid [0x%x] chiprev [0x%x]\n",
	    kerSysGetChipId(), UtilGetChipRev());

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

	/* Disable SSC, will be enabled after reset if link is up (enable= FALSE)*/
	bcm963xx_pcie_phy_config_ssc(pdrv);
	bcm963xx_pcie_phy_enable_ssc(pdrv, FALSE);


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

	/* Allow settling time */
	mdelay(10);

	HCD_FN_EXT();

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

	data = hcd_readl(pdrv->base,RC_CFG_PCIE_LINK_STATUS_CONTROL_2_OFFSET);
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
  * Function bcm963xx_pcie_core_config (pdrv)
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
static int bcm963xx_pcie_core_config(struct bcm963xx_pcie_hcd *pdrv)
{
	uint32	reg_data;
	struct resource *owin;

	HCD_FN_ENT();

	owin = &(pdrv->resources.owin);


	/* setup lgacy outband interrupts */
	reg_data = (CPU_INTR1_PCIE_INTD_CPU_INTR
	            | CPU_INTR1_PCIE_INTC_CPU_INTR
	            | CPU_INTR1_PCIE_INTB_CPU_INTR
	            | CPU_INTR1_PCIE_INTA_CPU_INTR);
	hcd_writel(reg_data, pdrv->base, CPU_INTR1_INTR_MASK_CLEAR_OFFSET);


	/* setup outgoing mem resource window */
	reg_data = ((owin->start >> MISC_CPU_2_PCI_MEM_WIN_LO_BASE_LIMIT_LIMIT_SHIFT) << MISC_CPU_2_PCI_MEM_WIN_LO_BASE_LIMIT_BASE_SHIFT);
	reg_data |= (owin->end & MISC_CPU_2_PCI_MEM_WIN_LO_BASE_LIMIT_LIMIT_MASK);
	hcd_writel(reg_data, pdrv->base, MISC_CPU_2_PCIE_MEM_WIN0_BASE_LIMIT_OFFSET);

	reg_data = (owin->start & MISC_CPU_2_PCI_MEM_WIN_LO_BASE_ADDR_MASK);
	hcd_writel(reg_data, pdrv->base, MISC_CPU_2_PCIE_MEM_WIN0_LO_OFFSET);
	/* TODO: for 64bit ARM */
	//hcd_writel(0, pdrv->base, MISC_CPU_2_PCIE_MEM_WIN0_HI_OFFSET);


	/* setup incoming DDR memory BAR(1) */
	{
	    uint32 sizekb;
	    uint32 barsz;

	    /* system memory size in kB */
	    sizekb = ((getMemorySize()) >> 10);

	    /* Calculate the size to be programmed (in terms of 64KB) */
	    for ( barsz = 0; barsz < MISC_RC_BAR_CONFIG_LO_SIZE_MAX; barsz++)
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
	    //hcd_writel(0, pdrv->base, MISC_RC_BAR1_CONFIG_HI_OFFSET);
	    reg_data = MISC_UBUS_BAR_CONFIG_ACCESS_EN;
	    hcd_writel(reg_data, pdrv->base, MISC_UBUS_BAR1_CONFIG_REMAP_OFFSET);
	}


	/* set device bus/func/func -no need*/
	/* setup class code, as bridge */
	reg_data = hcd_readl(pdrv->base, RC_CFG_PRIV1_ID_VAL3_OFFSET);
	reg_data &= RC_CFG_PRIV1_ID_VAL3_REVISION_ID_MASK;
	reg_data |= (PCI_CLASS_BRIDGE_PCI << 8);
	hcd_writel(reg_data, pdrv->base, RC_CFG_PRIV1_ID_VAL3_OFFSET);
	/* disable bar0 size -no need*/

	/* disable data bus error for enumeration */
	reg_data = hcd_readl(pdrv->base, MISC_CTRL_OFFSET);
	reg_data |= MISC_CTRL_CFG_READ_UR_MODE;
	/* Misc performance addition */
	reg_data |= (MISC_CTRL_MAX_BURST_SIZE_128B
	            |MISC_CTRL_PCIE_IN_WR_COMBINE
	            |MISC_CTRL_PCIE_RCB_MPS_MODE
	            |MISC_CTRL_PCIE_RCB_64B_MODE);

	if (pdrv->core_rev >= 0x310)
	    reg_data |=	MISC_CTRL_BURST_ALIGN(pdrv->core_rev, 3);
	else
	    reg_data |=	MISC_CTRL_BURST_ALIGN(pdrv->core_rev, 1);

	if (pdrv->core_rev == 0x310) {
	    /* workaround for UBUS4 Logic Bug in this revision */
	    /* Limit the max burst to 64B */
	    reg_data &= ~MISC_CTRL_MAX_BURST_SIZE_MASK;
	    reg_data |= MISC_CTRL_MAX_BURST_SIZE_64B;
	}
	
	hcd_writel(reg_data, pdrv->base, MISC_CTRL_OFFSET);

	/* If configured, enable PCIe SSC (enable = TRUE) */
	bcm963xx_pcie_phy_enable_ssc(pdrv, TRUE);

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
  * 	  Reset PCIe core using misc driver API's. Configure the phy parameters
  *     while the core is in reset.
  *
  *  Return: 0 on success, -ve on failure
  */
static void bcm963xx_pcie_core_reset(struct bcm963xx_pcie_hcd *pdrv)
{
	HCD_FN_ENT();

	/* Soft Reset the core  */
	bcm963xx_misc_set_pcie_reset(pdrv->misc_pdev, pdrv->core_id, TRUE);
	mdelay(10);

	/* Configure the phy when core is in reset */
	bcm963xx_pcie_phy_config(pdrv);

	/* Configure the link speed, if need */
	bcm963xx_pcie_core_set_speed(pdrv);

	/* Bring the core out of Soft Reset  */
	bcm963xx_misc_set_pcie_reset(pdrv->misc_pdev, pdrv->core_id, FALSE);
	mdelay(10);

	/* this is a critical delay */
	mdelay(500);

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
  * 	  Map  pcie core registers resource
  *
  *  Return: 0 on success, -ve on failure
  */
static int bcm963xx_pcie_setup_regs(struct bcm963xx_pcie_hcd *pdrv)
{

	struct bcm963xx_pcie_hc_res *pres = NULL;
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
	HCD_INFO("[%d] Rev [0x%2x] Class [0x%6x]\r\n",
	    pdev->id, (reg_data&0xFF), ((reg_data >> 8)&0xFFFFFF));

	pdrv->core_rev = hcd_readl(pdrv->base, MISC_REVISION_OFFSET);

	HCD_LOG("found core [%d] Rev [%2x.%2x]\r\n", pdev->id,
	    ((pdrv->core_rev >> 8)&0xFF), (pdrv->core_rev&0xFF));

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
	    if (pres->bus_range.end > BCM963XX_MAX_BUSNUM)
	        pres->bus_range.end = BCM963XX_MAX_BUSNUM;
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

	HCD_FN_ENT();

	pdev = pdrv->pdev;
	pres = &pdrv->resources;

	/* Initialize attributes with default values */
	switch (pdrv->core_id) {
	    case 0:
	        pres->base.start = PCIE0_PHYS_BASE;
	        pres->owin.start = PCIE0_MEM_PHYS_BASE;
	        pres->irq = INTERRUPT_ID_PCIE0;
	        break;
#if defined(PCIE1_PHYS_BASE)
	    case 1:
	        pres->base.start = PCIE1_PHYS_BASE;
	        pres->owin.start = PCIE1_MEM_PHYS_BASE;
	        pres->irq = INTERRUPT_ID_PCIE1;
	        break;
#endif
#if defined(PCIE2_PHYS_BASE)
	    case 2:
	        pres->base.start = PCIE2_PHYS_BASE;
	        pres->owin.start = PCIE2_MEM_PHYS_BASE;
	        pres->irq = INTERRUPT_ID_PCIE2;
	        break;
#endif
	    default:
	        return -1;
	}

	pres->base.end = pres->base.start + BCM963XX_PCIE_PHYS_SIZE - 1;
	pres->base.flags = IORESOURCE_MEM;

	pres->owin.end = pres->owin.start + BCM963XX_PCIE_MEMS_SIZE - 1;
	pres->owin.flags = IORESOURCE_MEM;

	pres->bus_range.start = BCM963XX_ROOT_BUSNUM;
	pres->bus_range.end = BCM963XX_MAX_BUSNUM;
	pres->bus_range.flags = IORESOURCE_BUS;

	pres->pci_addr = pres->owin.start;

	pres->domain = pdrv->core_id;

	/* PCIe port configuration */
	bcm963xx_pcie_hcd_init_hc_cfg(pdrv);

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
static int __init bcm963xx_pcie_probe(struct platform_device *pdev)
{
	struct bcm963xx_pcie_hcd *pdrv = NULL;
#if 0
	struct device_node *np = pdev->dev.of_node;
#endif
	uint32 core = (uint32)pdev->id;
	int	err = 0;
	struct pci_bus *bus;
	LIST_HEAD(res);

	HCD_FN_ENT();

#if 0
	/* If coming from device tree, use device tree entry to find the core id */
	if (np && (pdev->id <= PLATFORM_DEVID_NONE)) {
	    if (of_property_read_u32(np,"coreid", &core) < 0) {
	        HCD_ERROR("Unable to get coreid from device tree\r\n");
	        return -ENODEV;
	    }
		pdev->id = core;
	}
#endif
#ifdef CHANGE_PROBE_ORDER
	core = (uint32)pid2core[pdev->id];
#endif

	HCD_INFO("core [%d] probe\r\n",core);

	if (core >= NUM_CORE)
	    return -ENODEV;

	if (kerSysGetPciePortEnable(core)) {

	    pmc_pcie_power_up(core);
	    HCD_INFO("core [%d] powered-up\n", core);

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
	    bcm963xx_pcie_init_res(pdrv);

	    /* Update core resource elements  (DT based) */
	    err = bcm963xx_pcie_parse_dt(pdrv);
	    if (err) {
	        HCD_ERROR("failed to update core [%d] dt entries\r\n", core);
	        err =  -EINVAL;
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

	    /* lets talk to PCIe core, reset the core */
	    bcm963xx_pcie_core_reset(pdrv);

	    /* Check if PCIe link is up (for any device connected on the link) */
	    if (!(hcd_is_pcie_link_up(pdrv))) {
	        /* No device connected to PCIe core */
	        HCD_ERROR("failed to bring up core [%d] link\r\n",core);
	        err =  -ENODEV;
	        goto error;
	    }
	    HCD_INFO("core [%d] Link UP !!!!!!!\r\n",core);

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
	    bus = pci_scan_root_bus(&pdev->dev, BCM963XX_ROOT_BUSNUM,
	        &bcm963xx_pcie_ops, pdrv, &res);
	    if (!bus) {
	        HCD_ERROR("core [%d] failed to setup hw: %d\r\n", core, err);
	        err =  -ENXIO;
	        goto error;
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
#endif
#endif
	        bus->msi = (struct msi_controller*)pdrv->msi;
	    }

	    pci_assign_unassigned_bus_resources(bus);

	    if (!HCD_USE_DT_ENTRY(core)) {
			/* irq is a static mapping, not from DT. Apply irq fixup */
	        pci_fixup_irqs(pci_common_swizzle, bcm963xx_pcie_map_irq);
	    }

	    pci_bus_add_devices(bus);
	    err = 0;
	} else {
	    HCD_ERROR("core [%d] disabled\n", core);
	    err = -ENODEV;
	}

error:

	if (err) {
	    if (IS_ENABLED(CONFIG_PCI_MSI)) {
	        bcm963xx_pcie_msi_disable(pdrv);
	    }
	    bcm963xx_pcie_unmap_res(pdrv);
	    kfree(pdrv);
	    if (kerSysGetPciePortEnable(core)) {
	        pmc_pcie_power_down(core);
	        HCD_INFO("core [%d] powered-down\n", core);
	    }
	}

	pci_free_resource_list(&res);

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
static int __exit bcm963xx_pcie_remove(struct platform_device *pdev)
{
	struct bcm963xx_pcie_hcd *pdrv = platform_get_drvdata(pdev);
#ifdef CHANGE_PROBE_ORDER
	int core = pid2core[pdev->id];
#else
	int core = pdev->id;
#endif

	HCD_FN_ENT();

	if (!pdrv)
	    return 0;


	if (kerSysGetPciePortEnable(core)) {
	    if (IS_ENABLED(CONFIG_PCI_MSI)) {
	        bcm963xx_pcie_msi_disable(pdrv);
	    }

	    bcm963xx_pcie_unmap_res(pdrv);
	    kfree(pdrv);

	    /* power off ports without link */
	    pmc_pcie_power_down(core);
	}

	HCD_FN_EXT();

	return 0;
}


/**************************************
 *  Global Functions
 **************************************/
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

	printk("PCIe HCD (impl%d)\r\n",CONFIG_BCM_PCIE_HCD_IMPL);

	ret = platform_driver_register(&bcm963xx_pcie_driver);

	HCD_FN_EXT();

	return ret;
}

/*
  *
  * Function bcm963xx_pcie_init ()
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

	HCD_FN_EXT();

	return;
}

module_init(bcm963xx_pcie_init);
module_exit(bcm963xx_pcie_exit);

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
	int core;
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
	for (core = 0; core < NUM_CORE; core++) {
	    pdev = &bcm963xx_pcie_plt_dev[core];
	    pdev->name = BCM963XX_PCIE_DEV_NAME;
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
subsys_initcall(bcm963xx_pcie_plt_init);
#endif /* CONFIG_BCM_KF_ARM_BCM963XX */
