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
#include <linux/version.h>
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
#include <linux/seq_file.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
#include "../drivers/pci/pci.h"
#endif

/* brcm bsp */
#if defined(CONFIG_BRCM_IKOS) || defined(CONFIG_BCM_PCIE_PMC_BRD_STUBS)
#define USE_PMC_BRD_STUBS
#else /* !CONFIG_BRCM_IKOS && !CONFIG_BCM_PCIE_PMC_BRD_STUBS */
#include <board.h>
#include <pmc_pcie.h>
#endif /* !CONFIG_BRCM_IKOS && !CONFIG_BCM_PCIE_PMC_BRD_STUBS */
#include <shared_utils.h>
#include <bcm_intr.h>
#include <bcm_nvram.h>

/* pcie_hcd */
#include <pcie_hcd.h>
#include <pcie-vcore.h>

/*
 * +-----------------------------------------------------
 *  Defines
 * +-----------------------------------------------------
 */
#define PCIE_HCD_ROOT_BUSNUM                0x00
#define PCIE_HCD_MAX_BUSNUM                 0x0F

#define PCIE_HC_CORE_REV_OFFSET             0x0000406c
#define PCI_EXP_CFG_OFFSET                  0x000000AC

#define PCIE_LNKCAP                         (PCI_EXP_CFG_OFFSET + PCI_EXP_LNKCAP)
#define PCIE_LNKCAP_LNK_WIDTH(cap)          \
	    (((cap) & PCI_EXP_LNKCAP_MLW) >> PCI_EXP_LNKSTA_NLW_SHIFT)
#define PCIE_LNKCAP_LNK_SPEED(cap)          ((cap) & PCI_EXP_LNKCAP_SLS)
#define PCIE_LNKCTLSTA                      (PCI_EXP_CFG_OFFSET + PCI_EXP_LNKCTL)
#define PCIE_LNKSTA_LNK_WIDTH(sta)          \
	    (((sta) & PCI_EXP_LNKSTA_NLW) >> PCI_EXP_LNKSTA_NLW_SHIFT)
#define PCIE_LNKSTA_LNK_SPEED(sta)          ((sta) & PCI_EXP_LNKSTA_CLS)

#define MSI_ISR_NAME_STR_LEN                32

#define APON_F_DEFAULT                      (1 << HCD_APON_DEFAULT)
#define APON_F_ON                           (1 << HCD_APON_ON)
#define APON_F_OFF                          (1 << HCD_APON_OFF)
#define APON_F_OFF_W_DOM                    (1 << HCD_APON_OFF_WITH_DOMAIN)

/*
 * +-----------------------------------------------------
 *  Macros
 * +-----------------------------------------------------
 */
#define PCI_EXP_LNK_WIDTH(width)            \
	    (((width)&PCI_EXP_LNKSTA_NLW) >> PCI_EXP_LNKSTA_NLW_SHIFT)

#define HCD_USE_DT_ENTRY(core)              \
	((IS_ENABLED(CONFIG_OF)) ? true : false)

#define HCD_FAIL_ON_DT_ERROR(res, err)      \
	if (err) {                                                 \
	    HCD_ERROR("No DT entry for [%s]\n", res);                \
	    return err;                                            \
	} else {                                                   \
	    HCD_INFO("Updating [%s] settings from DT entry\n", res); \
	}

#define HCD_WARN_ON_DT_ERROR(res, err)      \
	if (err) {                                                 \
	    HCD_WARN("No DT entry for [%s] using defaults\n", res); \
	} else {                                                   \
	    HCD_INFO("Updating [%s] settings from DT entry\n", res); \
	}

/*
 * +-----------------------------------------------------
 *  Structures
 * +-----------------------------------------------------
 */
/**
 * PCIe Host Controller Driver Procfs control block
 * @dir:      pcie_hcd procfs directory file pointer
 * @coreinfo: pcie_hcd coreinfo procfs file pointer
 * @pdrv:     Host Controller Driver CB array
 */
struct pcie_hcd_procfs {
	struct proc_dir_entry *dir;
	struct proc_dir_entry *coreinfo;
	struct pcie_hcd       *pdrv[MAX_NUM_PCIE_CORES];
};

/*
 * BCM PCIe Host Controller device resources
 * @chip:         PCIe msi controller
 * @used:         bitmap of used msi interrupts
 * @domain:       pointer to msi irq domain
 * @irq_ops:      msi interrupt mask/unmask operations
 * @domain_ops:   msi interrupt domain operations
 * @pdrv:         Pointer to pcie driver cb
 * @map_size:     Number of msi interrupts
 * @mutex_lock:   lock for irq alloc/free
 * @enabled:      flag to specify msi is enabled or not
 */
struct pcie_hcd_msi {
	/* this should be the first element. don't move */
	struct msi_controller chip;
	DECLARE_BITMAP(used, MAX_MSI_MAP_SIZE);
	struct irq_domain     *domain;
	struct irq_chip       irq_ops;
	struct irq_domain_ops domain_ops;
	struct pcie_hcd       *pdrv;
	int    map_size;
	struct mutex lock;
	bool   enabled;
};

/*
 * BCM PCIe Host Contoller Driver Outgoing Window control block
 *
 * @pci_addr:   Physical address of PCI core to determine the Offset between CPU  and PCIe
 * @inited:     Outgoing memory Window resource initialization status
 * @need_remap: Need Outgoing memory window resource re-map
 */
struct pcie_hcd_owin
{
	u64    pci_addr;
	bool   need_remap;
	bool   inited;
};

/*
 * BCM PCIe Host Contoller Driver Error Log interrupt control block
 *
 * @inited:     Errorlog interrupts initialization status
 */
struct pcie_hcd_errlog
{
	bool   inited;
};

/*
 * BCM PCIe Host Contoller Driver apon control block
 *
 * @flags:     power operation flags
 * @powered:   power status of device
 * @cfg:       power configraton of device
 */
struct pcie_hcd_apon
{
	u32    flags;
	bool   powered[MAX_NUM_CORE_DEVS];
	u8     cfg[MAX_NUM_CORE_DEVS];
};


/*
 * BCM PCIe Host Contoller Driver control block
 *
 * @sys:             PCI sysdata (only arm32 platforms)
 * @msi:             msi control block
 * @bus:             pointer to root bus
 * @pfs:             pointer to pcie hcd proc file system control block
 * @core:            host controller core control block
 * @errlog:          error log control block
 * @apon:            apon control block
 * @owin:            Outgoing window control block
 */
struct pcie_hcd
{
	struct pcie_hcd_msi    *msi;
	struct pci_bus         *bus;
	struct pcie_hcd_procfs *pfs;

	struct pcie_hc_core    core;
	struct pcie_hcd_errlog errlog;
	struct pcie_hcd_apon   apon;
	struct pcie_hcd_owin   owin[MAX_NUM_OUTGOING_WINDOWS];
};

/*
 * +-----------------------------------------------------
 *  Local Function prototype
 * +-----------------------------------------------------
 */
/* nvram */
static int pcie_hcd_nvram_get_u32(char *key, u32 *pval);
static int pcie_hcd_get_boot_order_core(int index);
static int pcie_hcd_nvram_init(void);

/* procfs */
static int pcie_hcd_procfs_init(struct pcie_hcd *pdrv);
static void pcie_hcd_procfs_deinit(struct pcie_hcd *pdrv);

/* msi interrupt */
static int pcie_hcd_msi_alloc_region(struct pcie_hcd_msi *msi, int nirqs);
static void pcie_hcd_msi_free(struct pcie_hcd_msi *msi,
	unsigned long irq);
static int pcie_hcd_msi_setup_irq_range(struct msi_controller *chip,
	struct pci_dev *pdev, struct msi_desc *desc, int nvec);
static int pcie_hcd_msi_setup_irq(struct msi_controller *chip,
	struct pci_dev *pdev, struct msi_desc *desc);
static int pcie_hcd_msi_setup_irqs(struct msi_controller *chip,
	struct pci_dev *pdev, int nvec, int type);
static void pcie_hcd_msi_teardown(struct msi_controller *chip,
	unsigned int irq);
static int pcie_hcd_msi_map(struct irq_domain *domain,
	unsigned int irq, irq_hw_number_t hwirq);
static irqreturn_t pcie_hcd_msi_isr(int irq, void *data);
static int pcie_hcd_msi_enable(struct pcie_hcd *pdrv);
static int pcie_hcd_msi_setup(struct pcie_hcd *pdrv);
static int pcie_hcd_msi_disable(struct pcie_hcd *pdrv);

/* errlog interrupt */
static int pcie_hcd_errlog_enable(struct pcie_hcd *pdrv);
static void pcie_hcd_errlog_disable(struct pcie_hcd *pdrv);

/* HC core fops access */
static int pcie_hc_link_up(struct pcie_hcd *pdrv);
static int pcie_hc_config_core(struct pcie_hcd *pdrv);
static int pcie_hc_reset_core(struct pcie_hcd *pdrv);
static int pcie_hc_setup_access(struct pcie_hcd *pdrv);
static int pcie_hc_init_core(struct platform_device *pdev, struct pcie_hcd *pdrv);
static void pcie_hc_free_core(struct pcie_hcd *pdrv);

/* APON */
static int pcie_hc_apon_pwrup(struct pcie_hcd *pdrv);
static int pcie_hc_apon_pwrdn(struct pcie_hcd *pdrv);

/* Hc helper functions */
static int pcie_hcd_set_power(struct pcie_hc_core *phc, unsigned int devidx, bool up);

/* pci fops */
static int pcie_hcd_config_read(struct pci_bus *bus, unsigned int devfn,
	int where, int size, u32 *val);
static int pcie_hcd_config_write(struct pci_bus *bus, unsigned int devfn,
	int where, int size, u32 val);
static void __iomem *pcie_hcd_map_bus(struct pci_bus *bus,
	unsigned int devfn, int where);
static int pcie_hcd_map_irq(const struct pci_dev *pcidev,
	u8 slot, u8 pin);

/* pci resources */
static int pcie_hcd_setup_resources(struct pcie_hcd *pdrv,
	struct list_head *resources);
static void pcie_hcd_unmap_resources(struct pcie_hcd *pdrv);
static int pcie_hcd_parse_dt(struct pcie_hcd *pdrv);

/* driver probe */
static int pcie_hcd_probe(struct platform_device *pdev);
static int pcie_hcd_remove(struct platform_device *pdev);
static int pcie_hcd_cpci_probe(struct platform_device *pdev);
static int pcie_hcd_cpci_remove(struct platform_device *pdev);

/* module */
static int __init pcie_hcd_init(void);
static void __exit pcie_hcd_exit(void);

/* platform */
static void pcie_hcd_plt_dev_release(struct device *dev);
static int __init pcie_hcd_plt_init(void);
static void __init pcie_hcd_plt_deinit(void);

/*
 * +-----------------------------------------------------
 *  external Function prototype
 * +-----------------------------------------------------
 */
extern int ubus_decode_pcie_wnd_cfg(u32 base, u32 size, u32 core);


/*
 * +-----------------------------------------------------
 *  Global variables
 * +-----------------------------------------------------
 */

static struct pci_ops pcie_hcd_ops_generic = {
	.map_bus = pcie_hcd_map_bus,
	.read = pci_generic_config_read,
	.write = pci_generic_config_write,
};

static struct pci_ops pcie_hcd_ops_specific = {
	.read = pcie_hcd_config_read,
	.write = pcie_hcd_config_write,
};

static const struct of_device_id pcie_hcd_of_match[] = {
	{ .type = "cpci", .compatible = "brcm,bcm-pcie",       },
	{ .type = "pci",  .compatible = "brcm,bcm-pcie",       },
	{ .type = "vpci", .compatible = "brcm,bcm-pcie",       },
	{ .type = "pci",  .compatible = "brcm,bcm963xx-pcie",  },
	{ .type = "pci",  .compatible = "brcm,bcm947xx-pcie",  },
	{ .type = "vpci", .compatible = "brcm,bcm963xx-vpcie", },
	{},
};
MODULE_DEVICE_TABLE(of, pcie_hcd_of_match);

static struct platform_driver pcie_hcd_driver = {
	.probe  = pcie_hcd_probe,
	.driver = {
	    .name  = PCIE_HCD_DRV_NAME,
	    .owner = THIS_MODULE,
	    .of_match_table = of_match_ptr(pcie_hcd_of_match),
	},
	.remove = pcie_hcd_remove,
};

/* Virtual core first and then physical core */
static int pcie_hcd_hc_enum_order[MAX_HC_TYPE] = {
	VHC,
	PHC
};

/*
 * config_ssc values
 * (if exists, Device Tree entry has higer preference than this setting)
 *
 *     0 - disable
 *     1 - Enable
 *
 *     Each 4bit's corresponds to a PCIe core
 *     [31-28] [27-24][23-20] [19-16] [15-12] [11-08] [07-04] [03-00]
 *                                    [core4] [core3] [core2] [core1]
 */
u32 pcie_ssc_cfg = 0x0000;
module_param(pcie_ssc_cfg, int, S_IRUGO);

/*
 * config_speed values
 * (if exists, Device Tree entry has higer preference than this setting)
 *
 *     0 - default (keep reset value)
 *     1 - 2.5Gbps
 *     2 - 5Gbps
 *     3 - 8 Gbps
 *
 *     Each 4bit's corresponds to a PCIe core
 *     [31-28] [27-24][23-20] [19-16] [15-12] [11-08] [07-04] [03-00]
 *                                    [core4] [core3] [core2] [core1]
 */
u32 pcie_speed_cfg = 0x0000;
module_param(pcie_speed_cfg, int, S_IRUGO);

/*
 * core bring up order (right -> left)
 * (if exists, Device Tree entry has higer preference than this setting)
 *
 *     0x000 - default ( 0, 1, 2, ..... )
 *     0x210 - boot order 0, 1, 2
 *     0x012 - boot order 2, 1, 0
 *
 *     Each 4bit's corresponds to a PCIe core id
 *     [31-28] [27-24][23-20] [19-16][15-12] [11-08] [07-04] [03-00]
 *                                           [third] [second][first]
 */
u32 pcie_boot_order = 0x0000;
module_param(pcie_boot_order, int, S_IRUGO);

/*
 * Always power on
 * (if exists, Device Tree entry has higer preference than this setting)
 *
 *     0 - Power Down PCIe core if no Link (default)
 *     1 - Keep PCIe core Powered up even if there is no Link
 *     2 - Keep PCIe core Powered Off even if there is Link
 *     3 - Keep PCIe core Powered Off with domain even if there is Link
 *
 *     Each 4bit's corresponds to a PCIe core id
 *     [31-28] [27-24][23-20] [19-16] [15-12] [11-08] [07-04] [03-00]
 *                                    [core4] [core3] [core2] [core1]
 *
 *     For Virtual Core the nibble is divided as below
 *     [03-02] [01-00]
 *     [dev2]  [dev1]
 */
u32 pcie_apon = 0x0000;
module_param(pcie_apon, int, S_IRUGO);

/*
 * phy power mode values
 * (if exists, Device Tree entry has higer preference than this setting)
 *
 *     0 - default (normal power)
 *     1 - low power
 *
 *     Each 4bit's corresponds to a PCIe core
 *     [31-28] [27-24][23-20] [19-16] [15-12] [11-08] [07-04] [03-00]
 *                                    [core4] [core3] [core2] [core1]
 */
u32 pcie_phy_pwrmode = 0x0000;
module_param(pcie_phy_pwrmode, int, S_IRUGO);

/*
 * PCIe bus error logging
 * (if exists, Device Tree entry has higer preference than this setting)
 *
 *     0 - No PCIe bus error logging
 *     1 - enable PCIe bus error logging
 *
 *     Each 4bit's corresponds to a PCIe core id
 *     [31-28] [27-24][23-20] [19-16] [15-12] [11-08] [07-04] [03-00]
 *                                    [core4] [core3] [core2] [core1]
 */
u32 pcie_errlog = 0x1111;
module_param(pcie_errlog, int, S_IRUGO);

/*
 * PCIe enable MSI
 * (if exists, Device Tree entry has higer preference than this setting)
 *
 *     0 - Don't use PCIe MSI interrupts
 *     1 - Use PCIe MSI interrupts (default)
 *
 *     Each 4bit's corresponds to a PCIe core id
 *     [31-28] [27-24][23-20] [19-16] [15-12] [11-08] [07-04] [03-00]
 *                                    [core4] [core3] [core2] [core1]
 */
#if defined(CONFIG_BCM_PCI_MSI)
u32 pcie_msi = 0x1111;
#else
u32 pcie_msi = 0x0;
#endif
module_param(pcie_msi, int, S_IRUGO);

/*
 * PCIe force Single Lane
 * (if exists, Device Tree entry has higer preference than this setting)
 *
 *     0 - Auto lane detection
 *     1 - Force to single lane on a dual lane confiuration (RC + EP)
 *
 *     Each 4bit's corresponds to a PCIe core id
 *     [31-28] [27-24][23-20] [19-16] [15-12] [11-08] [07-04] [03-00]
 *                                    [core4] [core3] [core2] [core1]
 */
u32 pcie_numlanes = 0x0;
module_param(pcie_numlanes, int, S_IRUGO);

/*
 * PCIe ACK and Credit Coalescing
 * (if exists, Device Tree entry has higer preference than this setting)
 *
 *     0 - disable Ack and Credit Coalescing
 *     1 - enable Ack and Credit Coalescing
 *
 *     Each 4bit's corresponds to a PCIe core id
 *     [31-28] [27-24][23-20] [19-16] [15-12] [11-08] [07-04] [03-00]
 *                                    [core4] [core3] [core2] [core1]
 */
u32 pcie_acc = 0x1111;
module_param(pcie_acc, int, S_IRUGO);

/*
 * phy clock PLL id values
 * (if exists, Device Tree entry has higer preference than this setting)
 *
 *     [id] - core id of the clock PLL to be used
 *
 *     Each 4bit's corresponds to a PCIe core
 *     [31-28] [27-24][23-20] [19-16] [15-12] [11-08] [07-04] [03-00]
 *                                    [core4] [core3] [core2] [core1]
 */
u32 pcie_pllclkid = 0x3210;

/*
 * phy PLL clock Adjust
 * (if exists, Device Tree entry has higer preference than this setting)
 *
 *     [id] - core id of the clock PLL to be used
 *     0 - No PLL Clock Adjustment
 *     1 - Adjust PLL Clock frequency by -5000ppm
 *
 *     Each 4bit's corresponds to a PCIe core
 *     [31-28] [27-24][23-20] [19-16] [15-12] [11-08] [07-04] [03-00]
 *                                    [core4] [core3] [core2] [core1]
 */
u32 pcie_pllclkadj = 0x0000;

/* HCD Logging */
#ifdef HCD_DEBUG
int hcd_log_level = HCD_LOG_LVL_ERROR;
#endif

/* procfs */
static struct pcie_hcd_procfs pcie_hcd_procfs_cb;

/* msi interrupts */
static char pcie_hcd_msi_name[MAX_NUM_PCIE_CORES][MSI_ISR_NAME_STR_LEN];

/* platform */
static struct platform_device pcie_hcd_plt_dev[MAX_NUM_PCIE_CORES];
struct pcie_hc_plt_cfg hc_plt_cfg[MAX_HC_TYPE] = {
	{
	    .num_cores = -1,
	    .plt_dev   = pcie_hcd_plt_dev,
	    .plt_drv   = &pcie_hcd_driver,
	    .pci_fops  = &pcie_hcd_ops_generic,
	    .set_power = pcie_hcd_set_power,
	},
	{
	    .num_cores = -1,
	    .plt_dev   = pcie_hcd_plt_dev,
	    .plt_drv   = &pcie_hcd_driver,
	    .pci_fops  = &pcie_hcd_ops_generic,
	    .set_power = pcie_hcd_set_power,
	}
};

/*
 * +-----------------------------------------------------
 *  Local inline functions
 * +-----------------------------------------------------
 */
/* msi controller to msi */
static inline struct pcie_hcd_msi *to_pcie_hcd_msi(struct msi_controller *chip)
{
	return container_of(chip, struct pcie_hcd_msi, chip);
}

#if defined(USE_PMC_BRD_STUBS)
static inline void pcie_hcd_pmc_power_up(int unit, int is_dual_lane) { }
static inline void pcie_hcd_pmc_power_down(int unit, int is_dual_lane) { }
static inline int pcie_hcd_get_port_mode(int port) {
	/* Alwyas RC mode */
	return 1;
}
static inline int pcie_hcd_port_enabled(int port) {
	if (PLTID2HC(port) == VHC) {
	    /* Virtual port always present */
	    return 1;
	}

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
static inline int pcie_hcd_ubus_decode_pcie_wnd_cfg(u32 base, u32 size, u32 core) {
	/* Not supported */
	return -1;
}
#else /* !USE_PMC_BRD_STUBS */
static inline void pcie_hcd_pmc_power_up(int unit, int is_dual_lane) {
	if (PLTID2HC(unit) == VHC) {
	    /* Virtual port: Nothing to do */
	    return;
	}
	return pmc_pcie_power_up(unit, is_dual_lane);
}
static inline void pcie_hcd_pmc_power_down(int unit, int is_dual_lane) {
	if (PLTID2HC(unit) == VHC) {
	    /* Virtual port: Nothing to do */
	    return;
	}
	return pmc_pcie_power_down(unit, is_dual_lane);
}
static inline int pcie_hcd_get_port_mode(int port) {
	if (PLTID2HC(port) == VHC) {
	    /* Virtual port always RC */
	    return 1;
	}
	return kerSysGetPciePortMode(port);
}
static inline int pcie_hcd_port_enabled(int port) {
	if (PLTID2HC(port) == VHC) {
	    /* Virtual port always present */
	    return 1;
	}
	return kerSysGetPciePortEnable(port);
}
static inline int pmc_hcd_get_chip_id(void) {
	return kerSysGetChipId();
}
static inline int pcie_hcd_ubus_decode_pcie_wnd_cfg(u32 base, u32 size, u32 core) {
	return ubus_decode_pcie_wnd_cfg(base, size, core);
}
#endif /* !USE_PMC_BRD_STUBS */

/*
 * +-----------------------------------------------------
 *  Local Functions
 * +-----------------------------------------------------
 */

/*
 * Function pcie_hcd_nvram_get_u32 (key, pval)
 *
 *   Parameters:
 *    key  ... nvram key string
 *    pval ... pointer to u32 value to be returned
 *
 *   Description:
 *    Query the key value from NVRAM and convert to u32 value
 *    value exptected in hex format with a 0xprefix
 *
 *   Return: 0 on success, -ve on failure
 */
int pcie_hcd_nvram_get_u32(char *key, u32 *pval)
{
#if !defined(CONFIG_BRCM_QEMU) && (defined(CONFIG_BCM_NVRAM) || defined(CONFIG_BCM_NVRAM_MODULE))
	char *valstr;
	u32 val;

	if ((valstr = nvram_k_get(key)) != NULL) {
	    if (sscanf(valstr, "0x%x", &val) == 1) {
	        if (pval) *pval = val;
	        HCD_FN_RET_VAL(0);
	    }
	    HCD_FN_RET_VAL(-EINVAL);
	}
#endif /* CONFIG_BCM_NVRAM */

	HCD_FN_RET_VAL(-ENOENT);
}

/*
 * Function pcie_hcd_get_boot_order_core (order)
 *
 *   Parameters:
 *    order ... index of boot order
 *
 *   Description:
 *    Initialize the port core.cfg parameters from global storage area
 *
 *   Return: None
 */
int pcie_hcd_get_boot_order_core(int index)
{
	int core;

	HCD_FN_ENT();

	if (pcie_boot_order)
	    core = ((pcie_boot_order >> (index*4)) &  0xF);
	else
	    core = index;

	HCD_FN_RET_VAL(core);
}

/*
 * Function pcie_hcd_nvram_init (void)
 *
 *    Parameters:
 *      None
 *
 *    Description:
 *     Query the PCIe NVRAM parameters and update the global configuration
 *     parameters
 *
 *   Return: 0 on success, -ve on failure
 */
int pcie_hcd_nvram_init(void)
{
#if defined(MODULE)
	u32 val;

	HCD_FN_ENT();

	if (pcie_hcd_nvram_get_u32("pcie_ssc_cfg", &val) == 0) {
	    pcie_ssc_cfg = val;
	}

	if (pcie_hcd_nvram_get_u32("pcie_speed_cfg", &val) == 0) {
	    pcie_speed_cfg = val;
	}

	if (pcie_hcd_nvram_get_u32("pcie_boot_order", &val) == 0) {
	    pcie_boot_order = val;
	}

	if (pcie_hcd_nvram_get_u32("pcie_apon", &val) == 0) {
	    pcie_apon = val;
	}

	if (pcie_hcd_nvram_get_u32("pcie_phy_pwrmode", &val) == 0) {
	    pcie_phy_pwrmode = val;
	}

	if (pcie_hcd_nvram_get_u32("pcie_errlog", &val) == 0) {
	    pcie_errlog = val;
	}

	if (pcie_hcd_nvram_get_u32("pcie_msi", &val) == 0) {
	    pcie_msi = val;
	}

	if (pcie_hcd_nvram_get_u32("pcie_numlanes", &val) == 0) {
	    pcie_numlanes = val;
	}

	if (pcie_hcd_nvram_get_u32("pcie_acc", &val) == 0) {
	    pcie_acc = val;
	}

	if (pcie_hcd_nvram_get_u32("pcie_pllclkadj", &val) == 0) {
	    pcie_pllclkadj = val;
	}

#ifdef HCD_DEBUG
	if (pcie_hcd_nvram_get_u32("pcie_log_level", &val) == 0) {
	    hcd_log_level = val;
	}
#endif /* HCD_DEBUG */

	HCD_FN_RET_VAL(0);

#else /* !MODULE */
	HCD_FN_ENT();

	HCD_ERROR("nvram is not supported in built-in mode\r\n");

	HCD_FN_RET_VAL(-1);
#endif /* !MODULE */
}

/*
 * Function pcie_hcd_procfs_coreinfo_show ()
 *
 *   Parameters:
 *
 *   Description:
 *    Read function for 'coreinfo' proc file
 *
 *   Return: length of information on success, 0 on failure/no information
 */
static int pcie_hcd_procfs_coreinfo_show(struct seq_file *m, void *v)
{
	int core;
	struct pcie_hcd_procfs *pfs_cb = &pcie_hcd_procfs_cb;

	HCD_FN_ENT();

	seq_printf(m, "c: dmn \n");
	for (core = 0; core < MAX_NUM_PCIE_CORES; core++) {

	    if (pfs_cb->pdrv[core] != NULL) {
	        seq_printf(m, "%d: %04d\n", core,
	            pci_domain_nr(pfs_cb->pdrv[core]->bus));
	    }
	}

	HCD_FN_RET_VAL(0);
}

/*
 * Function pcie_hcd_procfs_init (pdrv)
 *
 *   Parameters:
 *    pdrv ... pointer to pcie core hcd data structure
 *
 *   Description:
 *    PCIe hcd driver proc file system initialization
 *
 *   Return: 0 on success, -ve on failure
 */
int pcie_hcd_procfs_init(struct pcie_hcd *pdrv)
{
	struct pcie_hcd_procfs *pfs_cb = &pcie_hcd_procfs_cb;

	HCD_FN_ENT();

	pfs_cb->pdrv[pdrv->core.info.id] = pdrv;
	pdrv->pfs = pfs_cb;

	if (pdrv->pfs->dir) {
	    /* Already created, Nothing to do */
	    HCD_FN_RET_VAL(0);
	}

	/* make a directory in /proc if doesn't exist */
	pfs_cb->dir = proc_mkdir("pcie_hcd", NULL);

	if (pfs_cb->dir == NULL) {
	    HCD_ERROR("Core [%d] Unable to create pcie_hcd proc directory\n", pdrv->core.info.id);
	    HCD_FN_RET_VAL(-1);
	}

	pfs_cb->coreinfo = proc_create_single("coreinfo", 0, pfs_cb->dir,
	    pcie_hcd_procfs_coreinfo_show);
	if (pfs_cb->coreinfo == NULL) {
	    HCD_ERROR("Core [%d] Unable to create pcie_hcd/coreinfo proc file\n",
	        pdrv->core.info.id);
	    HCD_FN_RET_VAL(-1);
	}

	HCD_FN_RET_VAL(0);
}

/*
 * Function pcie_hcd_procfs_deinit (pdrv)
 *
 *   Parameters:
 *    pdrv ... pointer to pcie core hcd data structure
 *
 *   Description:
 *    PCIe hcd driver proc file system de-initialization
 *
 *   Return: 0 on success, -ve on failure
 */
void pcie_hcd_procfs_deinit(struct pcie_hcd *pdrv)
{
	struct pcie_hcd_procfs *pfs_cb = NULL;
	int core;

	HCD_FN_ENT();

	if (pdrv) pfs_cb = pdrv->pfs;

	if (pfs_cb == NULL) {
	    /* Nothing to do */
	    HCD_FN_RET();
	}

	pfs_cb->pdrv[pdrv->core.info.id] = NULL;

	for (core = 0; core < MAX_NUM_PCIE_CORES; core++) {
	    if (pfs_cb->pdrv[core]) {
	        /* There are some cores still running */
	        HCD_FN_RET();
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

	HCD_FN_RET();
}

/*
 * +-----------------------------------------------------
 * MSI Local Functions
 * +-----------------------------------------------------
 */

/*
 * Function pcie_hcd_msi_alloc_region (msi, nirqs)
 *
 *   Parameters:
 *    msi  ... msi control block pointer
 *    nirqs...number of irqs to allocate
 *
 *   Description:
 *    Allocate contiguous range of unused msi irq numbers for the client to use
 *
 *   Return: msi irq number on success, -ve value on failure
 */
static int pcie_hcd_msi_alloc_region(struct pcie_hcd_msi *msi, int nirqs)
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

	HCD_FN_RET_VAL(irq);
}

/*
 * Function pcie_hcd_msi_free (msi, irq)
 *
 *   Parameters:
 *    msi  ... msi control block pointer
 *    irq  ... msi irq number previously allocated
 *
 *   Description:
 *    Free (unuse) a previously allocated msi interrupt number
 *
 *   Return: None
 */
static void pcie_hcd_msi_free(struct pcie_hcd_msi *msi,
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

	HCD_FN_RET();
}

/*
 * Function pcie_hcd_msi_setup_irq_range (chip, pdev, desc, nvec)
 *
 *   Parameters:
 *    chip  ... msi chip control block pointer
 *    pdev  ... pci device pointer
 *    desc  ... msi irq descriptor pointer
 *    nvec  ... number of irqs
 *
 *   Description:
 *    Allocate nvec consecutive free MSI interrupts, Map MSI interrupts to system
 *    virtual interrupts, send an MSI message for the matching adress & data
 *
 *   Return: 0 on success, -ve value on failure
 */
static int pcie_hcd_msi_setup_irq_range(struct msi_controller *chip,
	struct pci_dev *pdev, struct msi_desc *desc, int nvec)
{
	struct pcie_hcd_msi *msi = to_pcie_hcd_msi(chip);
	struct msi_msg msg;
	unsigned int irq;
	int hwirq;
	int i;

	HCD_FN_ENT();

	if (msi->enabled == false) {
	    int err = pcie_hcd_msi_enable(msi->pdrv);
	    if (err < 0) {
	        HCD_ERROR("failed to configure msi 0x%p\n", msi);
	        HCD_FN_RET_VAL(err);
	    }
	}

	hwirq = pcie_hcd_msi_alloc_region(msi, nvec);
	if (hwirq < 0) {
	    HCD_ERROR("failed to allocate IRQ range %d: %d\n", nvec, hwirq);
	    HCD_FN_RET_VAL(-ENOSPC);
	}

	/* Create IRQ mapping for all the vectors */
	for (i = 0; i < nvec; i++) {
	    irq = irq_create_mapping(msi->domain, hwirq+i);
	    if (!irq) {
	        HCD_ERROR("failed to create IRQ mapping for %d\n", hwirq+i);
	        pcie_hcd_msi_free(msi, irq);
	        HCD_FN_RET_VAL(-EINVAL);
	    }
	}

	/* make sure irq is able to find it */
	irq = irq_find_mapping(msi->domain, hwirq);
	if (!irq) {
	    HCD_ERROR("irq_find_mapping(0x%px, %d) returned %d\r\n", msi->domain, hwirq, irq);
	    HCD_FN_RET_VAL(-ENOSPC);
	}

	/* Setup the msi desc parameters */
	for (i = 0; i < nvec; i++) {
	    BcmHalSetMsiIrqDescOff(irq, i, desc);
	}
	desc->nvec_used = nvec;
	desc->msi_attrib.multiple = order_base_2(nvec);

	/* Configure the hardware for MSI */
	msg.address_lo = MSI_MATCH_ADDR_MAGIC;
	msg.address_hi = 0;
	msg.data = (MSI_MATCH_DATA_MAGIC | hwirq);

	pci_write_msi_msg(irq, &msg);

	HCD_INFO("MSI Msg lo_addr [0x%x] hi_addr [0x%x], data [0x%x]\r\n",
	    msg.address_lo, msg.address_hi, msg.data);

	HCD_FN_RET_VAL(0);
}

/*
 * Function pcie_hcd_msi_setup_irq (chip, pdev, desc)
 *
 *   Parameters:
 *    chip  ... msi chip control block pointer
 *    pdev  ... pci device pointer
 *    desc  ... msi irq descriptior
 *
 *   Description:
 *    call setup_irq_range with 1 interrupt
 *
 *   Return: 0 on success, -ve value on failure
 */
static int pcie_hcd_msi_setup_irq(struct msi_controller *chip,
	struct pci_dev *pdev, struct msi_desc *desc)
{
	int rc;

	HCD_FN_ENT();

	rc = pcie_hcd_msi_setup_irq_range(chip, pdev, desc, 1);

	HCD_FN_RET_VAL(rc);
}

/*
 * Function pcie_hcd_msi_setup_irqs (chip, pdev, nvec, type)
 *
 *   Parameters:
 *    chip  ... msi chip control block pointer
 *    pdev  ... pci device pointer
 *    nvec  ... number of irqs
 *    type  ... type of msi interrupt
 *
 *   Description:
 *    Check the type and descriptor and call setup_irq_range()
 *
 *   Return: 0 on success, -ve value on failure
 */
static int pcie_hcd_msi_setup_irqs(struct msi_controller *chip,
	struct pci_dev *pdev, int nvec, int type)
{
	struct msi_desc *desc = NULL;
	int rc;

	HCD_FN_ENT();

	/* MSI-X interrupts are not supported */
	if (type == PCI_CAP_ID_MSIX) {
	    HCD_ERROR("%s: MSIX is not supported \r\n", __FUNCTION__);
	    HCD_FN_RET_VAL(-EINVAL);
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
	    HCD_FN_RET_VAL(-EINVAL);
	}

	rc = pcie_hcd_msi_setup_irq_range(chip, pdev, desc, nvec);

	HCD_FN_RET_VAL(rc);
}

/*
 * Function pcie_hcd_msi_teardown (chip, irq)
 *
 *   Parameters:
 *    chip ... msi chip control block pointer
 *    irq  ... virtual irq number
 *
 *   Description:
 *    unmap the system virtual interrupt, and unuse the msi irq number
 *
 *   Return: None
 */
static void pcie_hcd_msi_teardown(struct msi_controller *chip,
	unsigned int irq)
{
	struct pcie_hcd_msi *msi = to_pcie_hcd_msi(chip);
	struct irq_data *d = irq_get_irq_data(irq);
	irq_hw_number_t hwirq = irqd_to_hwirq(d);

	HCD_FN_ENT();

	irq_dispose_mapping(irq);
	pcie_hcd_msi_free(msi, hwirq);

	HCD_FN_RET();
}


/*
 * Function pcie_hcd_msi_map (domain, irq, hwirq)
 *
 *   Parameters:
 *    domain ... msi chip control block pointer
 *    irq    ... virtual irq number
 *    hwirq  ... msi interrupt number for this core
 *
 *   Description:
 *    setup the parameters for virtual irq number
 *
 *   Return: 0 on success, -ve value on failure
 */
static int pcie_hcd_msi_map(struct irq_domain *domain,
	unsigned int irq, irq_hw_number_t hwirq)
{
	struct pcie_hcd_msi *msi = (struct pcie_hcd_msi*)domain->host_data;

	HCD_FN_ENT();

	irq_set_chip_and_handler(irq, &msi->irq_ops, handle_simple_irq);
	irq_set_chip_data(irq, msi);

	HCD_FN_RET_VAL(0);
}

/*
 * Function pcie_hcd_msi_isr (irq, data)
 *
 *   Parameters:
 *    irq  ... PCIe core irq number
 *    data ... pointer to hcd (given through request_irq())
 *
 *   Description:
 *    Read the MSI interrupt status. Process all the set MSI interrupts
 *
 *   Return: IRQ_HANDLED on success, IRQ_NONE on NO MSI interrupts
 */
static irqreturn_t pcie_hcd_msi_isr(int irq, void *data)
{
	struct pcie_hcd_msi *msi = (struct pcie_hcd_msi *)data;
	struct pcie_hcd *pdrv = msi->pdrv;
	unsigned int i;
	u32 reg_val;

	HCD_FN_ENT();

	reg_val = pdrv->core.plt_cfg->get_msi(&pdrv->core);

	/* Process all the available MSI interrupts */
	i = 0;

	while (reg_val != 0x00000000) {
	    if (reg_val & (1ul << (i+pdrv->core.cfg.msi_intr_bitshift))) {
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
	        reg_val &= (~(1ul << (i+pdrv->core.cfg.msi_intr_bitshift)));
	    }
	    i++;
	}

	HCD_FN_RET_VAL((i > 0) ? IRQ_HANDLED : IRQ_NONE);
}

/*
 * Function pcie_hcd_msi_enable (pdrv)
 *
 *   Parameters:
 *    pdrv ... pointer to hcd
 *
 *   Description:
 *    setup PCIe core irq isr and configure the hardware to enable MSI
 *    functionality
 *
 *   Return: 0 on success, -ve on failure
 */
static int pcie_hcd_msi_enable(struct pcie_hcd *pdrv)
{
	struct pcie_hcd_msi *msi = pdrv->msi;
	int err = 0;

	/* Make sure msi structure was allocated earlier */
	if (!msi)
	    HCD_FN_RET_VAL(-ENODEV);

	/* Already enabled? */
	if (msi->enabled == true)
	    HCD_FN_RET_VAL(err);

	err = request_irq(pdrv->core.res.irq, pcie_hcd_msi_isr, IRQF_SHARED,
	   msi->irq_ops.name, msi);
	if (err < 0) {
	    HCD_ERROR("failed to request IRQ[%d]: %d\n", pdrv->core.res.irq, err);
	    HCD_FN_RET_VAL(err);
	}


	HCD_INFO("Using irq=%d for PCIE-MSI interrupts\r\n", pdrv->core.res.irq);

	err = pdrv->core.plt_cfg->setup_msi(&pdrv->core);
	if (err == 0) {
	    /* Set the flag to specify MSI is enabled */
	    msi->enabled = true;

	    HCD_INFO("MSI Enabled\n");
	} else {
	    free_irq(pdrv->core.res.irq, msi);
	}

	HCD_FN_RET_VAL(err);
}

/*
 * Function pcie_hcd_msi_setup (pdrv)
 *
 *   Parameters:
 *    pdrv ... pointer to hcd
 *
 *   Description:
 *    Allocate msi control block, initialize msi control block parameters,
 *
 *   Return: 0 on success, -ve on failure
 */
static int pcie_hcd_msi_setup(struct pcie_hcd *pdrv)
{
	struct platform_device *pdev = pdrv->core.pdev;
	struct pcie_hcd_msi *msi = NULL;
	int err;

	HCD_FN_ENT();

	if (!(pdrv->core.cfg.msi) || (pdrv->core.plt_cfg->setup_msi == NULL)) {
	    HCD_INFO("MSI is not supported or enabled\n");
	    HCD_FN_RET_VAL(0);
	}

	msi = kzalloc(sizeof(*msi), GFP_KERNEL);
	if (!msi) {
	    HCD_ERROR("Unable to allocate memory for MSI\r\n");
	    HCD_FN_RET_VAL(-ENOMEM);
	}

	HCD_INFO("Allocated [0x%px] hcd\r\n", msi);


	mutex_init(&msi->lock);
	snprintf(pcie_hcd_msi_name[pdrv->core.info.id], MSI_ISR_NAME_STR_LEN,
	    "msi_pcie:%d", pdrv->core.info.id);

	/* Initialize all msi structure elements */
	pdrv->msi = msi;
	msi->pdrv = pdrv;
	msi->irq_ops.name = pcie_hcd_msi_name[pdrv->core.info.id];
	msi->irq_ops.irq_enable = pci_msi_unmask_irq;
	msi->irq_ops.irq_disable = pci_msi_mask_irq;
	msi->irq_ops.irq_mask = pci_msi_mask_irq;
	msi->irq_ops.irq_unmask = pci_msi_unmask_irq;
	msi->domain_ops.map = pcie_hcd_msi_map;

	/* MSI should have been updated by hc with the number of interrupts supported */
	msi->map_size = pdrv->core.cfg.msi_map_size;

	msi->chip.dev = &pdev->dev;
	msi->chip.setup_irq = pcie_hcd_msi_setup_irq;
	msi->chip.setup_irqs = pcie_hcd_msi_setup_irqs;
	msi->chip.teardown_irq = pcie_hcd_msi_teardown;
	msi->domain = irq_domain_add_linear(pdev->dev.of_node, msi->map_size,
	    &msi->domain_ops, &msi->chip);
	if (!msi->domain) {
	    HCD_ERROR("failed to create IRQ domain\n");
	    HCD_FN_RET_VAL(-ENOMEM);
	}

	if (HCD_USE_DT_ENTRY(pdrv->core.info.id) && (pdrv->core.res.irq == 0)) {
	    err = platform_get_irq_byname(pdev, "msi");
	    if (err > 0) {
	        pdrv->core.res.irq = err;
	    } else {
	        HCD_ERROR("failed to get msi intr from DT: %d\n", err);
	        goto err;
	    }
	}

	err = 0;

err:
	HCD_FN_RET_VAL(err);
}

/*
 * Function pcie_hcd_msi_disable (pdrv)
 *
 *   Parameters:
 *    pdrv ... pointer to hcd
 *
 *   Description:
 *    Disable MSI feature on the hardware, Free the PCIe core isr and
 *    unmap and free all the MSI interrupts
 *
 *   Return: 0 on success, -ve on failure
 */
static int pcie_hcd_msi_disable(struct pcie_hcd *pdrv)
{
	struct pcie_hcd_msi *msi = NULL;
	unsigned int i, irq;

	HCD_FN_ENT();

	if (!pdrv) {
	    HCD_FN_RET_VAL(0);
	}

	msi = pdrv->msi;
	if (msi) {
	    if (pdrv->core.plt_cfg->teardown_msi)
	        pdrv->core.plt_cfg->teardown_msi(&pdrv->core);

	    /* Clear all mapped interrupts */
	    if (msi->enabled == true) {
	        free_irq(pdrv->core.res.irq, msi);
	    }

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

	HCD_FN_RET_VAL(0);
}

/*
 * Function pcie_hcd_errlog_enable (pdrv)
 *
 *   Parameters:
 *    pdrv ... pointer to pcie core hcd data structure
 *
 *   Description:
 *    Setup pcie core bus error logging
 *
 *   Return: 0 on success, -ve on failure
 */
static int pcie_hcd_errlog_enable(struct pcie_hcd *pdrv)
{
	int err;

	HCD_FN_ENT();

	if ((pdrv->errlog.inited == true) || (pdrv->core.plt_cfg->setup_errint == NULL)) {
	    /* Error logging already initialized or not supported, nothing to do */
	    HCD_FN_RET_VAL(0);
	}

	if (HCD_USE_DT_ENTRY(pdrv->core.info.id) && (pdrv->core.res.irq == 0)) {
	    err = platform_get_irq_byname(pdrv->core.pdev, "intr");
	    if (err > 0) {
	        pdrv->core.res.irq = err;
	    } else {
	        HCD_ERROR("failed to get errlog intr from DT: %d\n", err);
	        HCD_FN_RET_VAL(0);
	    }
	}

	err = pdrv->core.plt_cfg->setup_errint(&pdrv->core);

	if (err == 0) {
	    pdrv->errlog.inited = true;

	}

	HCD_FN_RET_VAL(err);
}

/*
 * Function pcie_hcd_errlog_disable (pdrv)
 *
 *   Parameters:
 *    pdrv ... pointer to pcie core hcd data structure
 *
 *   Description:
 *    Setup pcie core bus error logging
 *
 *   Return: 0 on success, -ve on failure
 */
static void pcie_hcd_errlog_disable(struct pcie_hcd *pdrv)
{
	HCD_FN_ENT();

	if ((pdrv->errlog.inited == false) || (pdrv->core.plt_cfg->teardown_errint == NULL)) {
	    /* Error logging is not initialized, nothing to do */
	    HCD_FN_RET();
	}

	pdrv->core.plt_cfg->teardown_errint(&pdrv->core);

	pdrv->errlog.inited = false;

	HCD_LOG("Core [%d] Disabled PCIE/UBUS Error Interrupts\r\n", pdrv->core.info.id);

	HCD_FN_RET();
}

/*
 * Function pcie_hc_link_up (pdrv)
 *
 *   Parameters:
 *    pdrv ... pointer to pcie core hcd data structure
 *
 *   Description:
 *    Check PCIe link is UP or down
 *
 *   Return: 0 on linkup, -ve on linkdown
 */
static int pcie_hc_link_up(struct pcie_hcd *pdrv)
{
	bool (*isup)(struct pcie_hc_core *);
	int err = 0;

	HCD_FN_ENT();

	/* Check if PCIe link is up (for any device connected on the link) */
	isup = pdrv->core.plt_cfg->is_linkup;
	if (isup) {
	    if (pdrv->core.plt_cfg->is_linkup(&pdrv->core) == true) {
	        u32 link_status;

	        link_status = pdrv->core.plt_cfg->read_reg(&pdrv->core, PCIE_LNKCTLSTA);
	        link_status >>= 16;

	        HCD_LOG("Core [%d] Link UP - [%d] lanes, [GEN%d] speed\r\n",
	            pdrv->core.info.id, PCIE_LNKSTA_LNK_WIDTH(link_status),
	            PCIE_LNKSTA_LNK_SPEED(link_status));
	    } else {
	        err = -ENODEV;
	    }
	}

	HCD_FN_RET_VAL(err);
}

/*
 * Function pcie_hc_config_core (pdrv)
 *
 *   Parameters:
 *    pdrv ... pointer to pcie core hcd data structure
 *
 *   Description:
 *    Configure HC core
 *
 *   Return: 0 on success, -ve on failure
 */
static int pcie_hc_config_core(struct pcie_hcd *pdrv)
{
	HCD_FN_ENT();

	pdrv->core.plt_cfg->config_core(&pdrv->core);

	HCD_FN_RET_VAL(0);
}

/*
 * Function pcie_hc_reset_core (pdrv)
 *
 *   Parameters:
 *    pdrv ... pointer to pcie core hcd data structure
 *
 *   Description:
 *    Reset PCIe core devices
 *
 *   Return: 0 on success, -ve on failure
 */
static int pcie_hc_reset_core(struct pcie_hcd *pdrv)
{
	int  (*reset)(struct pcie_hc_core *);

	HCD_FN_ENT();

	reset = pdrv->core.plt_cfg->reset_core;

	if (reset) {
	    reset(&pdrv->core);
	}

	HCD_FN_RET_VAL(0);
}

/*
 * Function pcie_hc_setup_access (pdrv)
 *
 *
 *   Parameters:
 *    pdrv ... pointer to pcie core hcd data structure
 *
 *   Description:
 *    Setup HC core for access
 *    Read core information
 *
 *   Return: 0 on success, -ve on failure
 */
static int pcie_hc_setup_access(struct pcie_hcd *pdrv)
{
	struct pcie_hc_core     *phc = NULL;
	struct platform_device  *pdev = NULL;
	struct device           *dev;
	u32    reg_data;
	u32    link_width;

	HCD_FN_ENT();

	phc = &pdrv->core;
	pdev = phc->pdev;
	dev = &pdev->dev;

	if (phc->info.base == NULL) {
	    phc->info.base = devm_ioremap_resource(dev, &phc->res.base);
	    if (IS_ERR(phc->info.base)) {
	        HCD_ERROR("Core [%d] reg base mapping fail: [%ld]\r\n",
	            phc->info.id, PTR_ERR(phc->info.base));
	        HCD_FN_RET_VAL(PTR_ERR(phc->info.base));
	    }
	}

	HCD_INFO("Core [%d] mapped reg base [0x%px]\r\n", pdev->id,
	    phc->info.base);

	reg_data = phc->plt_cfg->read_reg(phc, PCI_VENDOR_ID);
	HCD_INFO("Core [%d] Vendor [0x%04x] device [0x%04x]\r\n",
	    pdev->id, (reg_data&0xFFFF), ((reg_data >> 16)&0xFFFF));

	if (reg_data == 0xdeaddead) {
	    HCD_ERROR("Core [%d] access returned invalid value 0x%x\r\n",
	        pdev->id, reg_data);
	    HCD_FN_RET_VAL(-ENODEV);
	}

	reg_data = phc->plt_cfg->read_reg(phc, PCI_CLASS_REVISION);
	HCD_INFO("Core [%d] Rev [0x%02x] Class [0x%06x]\r\n",
	    pdev->id, (reg_data&0xFF), ((reg_data >> 8)&0xFFFFFF));

	phc->info.rev = phc->plt_cfg->read_reg(phc, phc->cfg.core_rev_offset);
	phc->info.rev &= phc->cfg.core_rev_mask;

	/* Get the Link Width and speed gen configuration from the core */
	reg_data = phc->plt_cfg->read_reg(phc, PCIE_LNKCAP);
	phc->info.gen = PCIE_LNKCAP_LNK_SPEED(reg_data);
	link_width = PCIE_LNKCAP_LNK_WIDTH(reg_data);
	if (!phc->res.link_width) phc->res.link_width = link_width;

	HCD_LOG("found Port [%d] GEN%d Core Rev [%x.%02x] with %d Lanes\r\n",
	    pdev->id, phc->info.gen, ((phc->info.rev >> 8)&0xFF),
	    (phc->info.rev&0xFF), link_width);

	/* Setup pcie core revision based initialization in the HC */
	if (phc->plt_cfg->setup_rev) {
	    int err;

	    if ((err = phc->plt_cfg->setup_rev(phc)) != 0) {
	        HCD_FN_RET_VAL(err);
	    }
	}

	HCD_FN_RET_VAL(0);
}

/*
 * Function pcie_hc_init_core (pdrv)
 *
 *   Parameters:
 *    pdev ... pointer to pcie platform device
 *    pdrv ... pointer to pcie core hcd data structure
 *
 *   Description:
 *    Initialize the HCD resource entries to default values. Currently supported resources
 *    - PCIe core base, memory window, PCI bus range
 *
 *   Return: 0 on success, -ve on failure
 */
static int pcie_hc_init_core(struct platform_device *pdev, struct pcie_hcd *pdrv)
{
	struct pcie_hc_core     *phc = NULL;
	struct pcie_hc_core_res *pres = NULL;
	int win;
	int err = -EINVAL;

	HCD_FN_ENT();

	phc = &pdrv->core;
	pres = &phc->res;

	/* initialize HC core information */
	phc->pdev = pdev;
	phc->pdrv = pdrv;
	phc->info.id = pdev->id;
	phc->phc_cb = NULL;
	phc->plt_cfg = &hc_plt_cfg[PLTID2HC(pdev->id)];

	/* Initialize HC configuration with HCD defaults */
	for (win = OWIN0; win < MAX_NUM_OUTGOING_WINDOWS; win++) {
	    pres->owin[win].start = pres->owin[win].end = 0;
	    pres->owin[win].flags = pdrv->owin[win].pci_addr = 0;
	    pdrv->owin[win].need_remap = 0;
	    pdrv->owin[win].pci_addr = pres->owin[win].start;
	}

	pres->bus_range.start = PCIE_HCD_ROOT_BUSNUM;
	pres->bus_range.end = PCIE_HCD_MAX_BUSNUM;
	pres->bus_range.flags = IORESOURCE_BUS;
	pres->domain = phc->info.id;
	pres->link_width = 0;

	/* Update HC configuration with HC defaults */
	if (phc->plt_cfg->init_core) {
	    err = phc->plt_cfg->init_core(phc);
	}

	if (err != 0) {
	    HCD_ERROR("Core [%d] failed to initialize HC\r\n", pdev->id);
	    HCD_FN_RET_VAL(err);
	}

	/* Update configurtion with nvram parameters */
	phc->cfg.ssc = HCD_HC_CORE_CFG(pcie_ssc_cfg, phc->info.id) ? true : false;
	phc->cfg.speed = HCD_HC_CORE_CFG(pcie_speed_cfg, phc->info.id);
	phc->cfg.apon = HCD_HC_CORE_CFG(pcie_apon, phc->info.id);
	phc->cfg.phypwrmode = HCD_HC_CORE_CFG(pcie_phy_pwrmode, phc->info.id);
	phc->cfg.errlog = HCD_HC_CORE_CFG(pcie_errlog, phc->info.id);
	phc->cfg.msi = HCD_HC_CORE_CFG(pcie_msi, phc->info.id);
	pres->link_width = HCD_HC_CORE_CFG(pcie_numlanes, phc->info.id);
	phc->cfg.acc = HCD_HC_CORE_CFG(pcie_acc, phc->info.id);
	phc->cfg.pllid = HCD_HC_CORE_CFG(pcie_pllclkid, phc->info.id);
	phc->cfg.plladj = HCD_HC_CORE_CFG(pcie_pllclkadj, phc->info.id);

	HCD_INFO("Core [%d] regs: start [0x%llx] end [0x%llx] flags [0x%lx]\r\n",
	    pdev->id, (u64)pres->base.start, (u64)pres->base.end,
	    pres->base.flags);
	HCD_INFO("Core [%d] Interrupt [%d]\r\n", pdev->id, pres->irq);
	HCD_INFO("Core [%d] bus_range: start [0x%llx] end [0x%llx] flags [0x%lx]\r\n",
	    pdev->id, (u64)pres->bus_range.start, (u64)pres->bus_range.end,
	    pres->bus_range.flags);
	for (win = OWIN0; win < MAX_NUM_OUTGOING_WINDOWS; win++) {
	    if (pres->owin[win].start) {
	        HCD_INFO("Core [%d] owin[%d]: start [0x%llx] end [0x%llx] flags [0x%lx]\r\n",
	            pdev->id, win, (u64)pres->owin[win].start, (u64)pres->owin[win].end,
	            pres->owin[win].flags);
	    }
	}

	HCD_INFO("Core [%d] ssc [%d] pllid [%d] plladj [%d]\n", pdev->id,
	    phc->cfg.ssc, phc->cfg.pllid, phc->cfg.plladj);
	HCD_INFO("Core [%d] speed [%d] apon [%d] phypwrmode [%d] errlog [%d] msi [%d]\n",
	    pdev->id, phc->cfg.speed, phc->cfg.apon, phc->cfg.phypwrmode,
	    phc->cfg.errlog, phc->cfg.msi);

	HCD_FN_RET_VAL(err);
}

/*
 * Function pcie_hc_free_core (pdrv)
 *
 *   Parameters:
 *    pdrv ... pointer to pcie core hcd data structure
 *
 *   Description:
 *    Free the HC core control block
 *
 *   Return: 0 on success, -ve on failure
 */
static void pcie_hc_free_core(struct pcie_hcd *pdrv)
{
	HCD_FN_ENT();

	if (pdrv->core.plt_cfg->free_core) {
	    pdrv->core.plt_cfg->free_core(&pdrv->core);
	}

	HCD_FN_RET();
}

/*
 * Function pcie_hc_apon_pwrup (pdrv)
 *
 *   Parameters:
 *    pdrv ... pointer to pcie core hcd data structure
 *
 *   Description:
 *    Set the apon configuration flags
 *    Power up the core devices
 *
 *   Return: Number of active devices on success,
 *           -ENODEV                  on all devices OFF
 */
static int pcie_hc_apon_pwrup(struct pcie_hcd *pdrv)
{
	struct pcie_hc_core *phc = &pdrv->core;
	u8 devidx;
	u8 apon;
	int devs = 0;

	HCD_FN_ENT();

	pdrv->apon.flags = 0;

	for (devidx = 0; devidx < phc->info.devs; devidx++) {
	    apon = phc->cfg.apon;
	    apon >>= (devidx * HCD_APON_SHIFT);
	    apon &= HCD_APON_MASK;
	    pdrv->apon.flags |= (1 << apon);
	    pdrv->apon.cfg[devidx] = apon;

	    if (apon <= HCD_APON_ON) {
	        /* Default or Always ON */
	        if (pdrv->core.plt_cfg->set_power(phc, devidx, true) == 0) {
	            pdrv->apon.powered[devidx] = true;
	            HCD_INFO("Core [%d] dev [%d] powered up\r\n", pdrv->core.info.id, devidx);
		    devs++;
	        } else {
	            pdrv->apon.powered[devidx] = false;
	            HCD_ERROR("Core [%d] dev [%d] failed to powered up\r\n",
	                pdrv->core.info.id, devidx);
	        }
	    } else {
	        /* Always off (with/without domain) */
	        pdrv->core.plt_cfg->set_power(phc, devidx, false);
	        HCD_LOG("Core [%d] dev [%d] powered down due to apon [%d]\r\n",
	            pdrv->core.info.id, devidx, apon);
	    }
	}

	HCD_FN_RET_VAL((pdrv->apon.flags == APON_F_OFF) ? -ENODEV : devs);
}

/*
 * Function pcie_hc_apon_pwrdn (pdrv)
 *
 *
 *   Parameters:
 *    pdrv ... pointer to pcie core hcd data structure
 *
 *   Description:
 *    Power down the hc core devices if not configured for always on
 *
 *   Return: 0 on success, -ve on failure
 */
static int pcie_hc_apon_pwrdn(struct pcie_hcd *pdrv)
{
	struct pcie_hc_core *phc = &pdrv->core;
	u8 devidx;

	HCD_FN_ENT();

	for (devidx = 0; devidx < phc->info.devs; devidx++) {
	    if (pdrv->apon.cfg[devidx] != HCD_APON_ON) {
		phc->plt_cfg->set_power(phc, devidx, false);
	        pdrv->apon.powered[devidx] = false;
	        HCD_LOG("Core [%d] dev [%d] powered down\r\n", phc->info.id, devidx);
	    }
	}

	HCD_FN_RET_VAL(0);
}

/*
 * Function pcie_hcd_set_power(phc, devidx, up)
 *
 *   Parameters:
 *    phc    ... pointer to core control block
 *    devidx ... core device index (not used)
 *    up     ... flag to sepcify power up/down
 *
 *   Description:
 *     power up/down the core, devidx ignored
 *
 *   Return: 0 success, -ve on failure
 */
int pcie_hcd_set_power(struct pcie_hc_core *phc, unsigned int devidx, bool up)
{
	HCD_FN_ENT();

	if (up == true) {
	    pcie_hcd_pmc_power_up(phc->info.id, phc->cfg.is_dual_lane);
	} else {
	    pcie_hcd_pmc_power_down(phc->info.id, phc->cfg.is_dual_lane);
	}

	HCD_FN_RET_VAL(0);
}

/*
 * Function pcie_hcd_config_read (bus, devfn where, size, val)
 *
 *   Parameters:
 *    bus   ... pointer to pcie bus
 *    devfn ... device and function number
 *    where ... offset from the base
 *    size  ... access size
 *    val   ... pointer to read value update location
 *
 *   Description:
 *    Read HC core configuration space
 *
 *   Return: 0 on success, -ve on failure
 */
static int pcie_hcd_config_read(struct pci_bus *bus, unsigned int devfn,
	int where, int size, u32 *val)
{
	struct pcie_hc_core *phc = (struct pcie_hc_core*)bus->sysdata;
	int err = PCIBIOS_SUCCESSFUL;

	HCD_FN_ENT();

	/* invalid access if powered down with domain */
	if (phc->pdrv->apon.flags != APON_F_OFF_W_DOM) {
	    err = phc->plt_cfg->pci_fops->read(bus, devfn, where, size, val);
	} else {
	    *val = 0xFFFFFFFF;
	}

	HCD_FN_RET_VAL(err);
}

/*
 * Function pcie_hcd_config_write (bus, devfn, where, size, val)
 *
 *   Parameters:
 *    bus   ... pointer to pcie bus
 *    devfn ... device and function number
 *    where ... offset from the base
 *    size  ... access size
 *    val   ... value to be written
 *
 *   Description:
 *    Write Hc core device config space
 *
 *   Return: 0 on success, -ve on failure
 */
static int pcie_hcd_config_write(struct pci_bus *bus, unsigned int devfn,
	int where, int size, u32 val)
{
	struct pcie_hc_core *phc = (struct pcie_hc_core*)bus->sysdata;
	int err = PCIBIOS_SUCCESSFUL;

	HCD_FN_ENT();

	/* invalid access if powered down with domain */
	if (phc->pdrv->apon.flags != APON_F_OFF_W_DOM) {
	    err = phc->plt_cfg->pci_fops->write(bus, devfn, where, size, val);
	}

	HCD_FN_RET_VAL(err);
}

/*
 * Function pcie_hcd_map_bus (bus, devfn, where)
 *
 *   Parameters:
 *    bus   ... pointer to pcie bus
 *    devfn ... device and function number
 *    where ... offset from the base
 *
 *   Description:
 *    Map pcie hc core config space address
 *
 *   Return: valid mapped memory on success, NULL on failure
 */
static void __iomem *pcie_hcd_map_bus(struct pci_bus *bus,
	unsigned int devfn, int where)
{
	struct pcie_hc_core *phc = (struct pcie_hc_core*)bus->sysdata;
	void __iomem *addr = NULL;

	HCD_FN_ENT();

	/* invalid access if powered down with domain */
	if (phc->pdrv->apon.flags != APON_F_OFF_W_DOM) {
	    addr = phc->plt_cfg->pci_fops->map_bus(bus, devfn, where);
	}

	HCD_FN_RET_VAL(addr);
}

/*
 * Function pcie_hcd_map_irq (pcidev, slot, pin)
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
int pcie_hcd_map_irq(const struct pci_dev *pcidev,
	u8 slot, u8 pin)
{
	int irq = -1;
	int  (*hc_irq_map)(const struct pci_dev *, u8, u8);
	struct pcie_hc_core *phc = pcidev->bus->sysdata;

	HCD_FN_ENT();

	hc_irq_map = hc_plt_cfg[PLTID2HC(phc->info.id)].map_core_irq;

	if (hc_irq_map) {
	    irq = hc_irq_map(pcidev, slot, pin);
	}

	HCD_FN_RET_VAL(irq);
}

/*
 * Function pcie_hcd_setup_resources (pdrv, resources)
 *
 *   Parameters:
 *    pdrv      ... pointer to pcie core hcd data structure
 *    resources ... pcie core resources
 *
 *   Description:
 *    Map window memory resources
 *
 *   Return: 0 on success, -ve on failure
 */
static int pcie_hcd_setup_resources(struct pcie_hcd *pdrv,
	struct list_head *resources)
{
	struct pcie_hc_core_res *pres = NULL;
	struct platform_device *pdev = NULL;
	struct resource *owin;
	struct device *dev;
	int err = 0;
	int win;

	HCD_FN_ENT();

	pdev = pdrv->core.pdev;
	dev = &pdev->dev;
	pres = &pdrv->core.res;

	for (win = OWIN0; win < MAX_NUM_OUTGOING_WINDOWS; win++) {

	    if (!OWIN_RES_CONFIGURED(&pdrv->core, win)) continue;

	    owin = &(pres->owin[win]);

	    if (pdrv->owin[win].need_remap) {

	        err = pcie_hcd_ubus_decode_pcie_wnd_cfg(owin->start,
	            (owin->end - owin->start) + 1, pdev->id);
	        if (err != 0) {
	            HCD_ERROR("Core [%d] pcie failed to config ubus decode window: [%d]\r\n",
	            pdev->id, err);
	            HCD_FN_RET_VAL(err);
	        }
	    }

	    err =  devm_request_resource(dev, &iomem_resource, owin);
	    if (err) {
	        HCD_ERROR("Core [%d] pcie failed to create own resource: [%d]\r\n",
	            pdev->id, err);

	        HCD_FN_RET_VAL(err);
	    }
	    else
	        HCD_INFO("Core [%d] mapped pcie owin[%d] base [0x%llx]\r\n", pdev->id,
	            win, (u64)owin->start);


	    pci_add_resource_offset(resources, owin, owin->start - pdrv->owin[win].pci_addr);

	    pdrv->owin[win].inited = true;
	}


	/* Setup PCIe core bus numbers */
	pci_add_resource(resources, &pres->bus_range);

	HCD_FN_RET_VAL(err);
}

/*
 * Function pcie_hcd_unmap_resources (pdrv)
 *
 *   Parameters:
 *    pdrv ... pointer to pcie core hcd data structure
 *
 *   Description:
 *    Unmap previous allocated resources window memory and register base resources
 *
 *   Return: 0 on success, -ve on failure
 */
static void pcie_hcd_unmap_resources(struct pcie_hcd *pdrv)
{
	struct pcie_hc_core_res *pres = NULL;
	struct platform_device *pdev = NULL;
	struct device *dev;
	int win;

	HCD_FN_ENT();

	if (pdrv == NULL) {
	    HCD_FN_RET();
	}

	pdev = pdrv->core.pdev;
	dev = &pdev->dev;
	pres = &pdrv->core.res;

	for (win = OWIN0; win < MAX_NUM_OUTGOING_WINDOWS; win++) {
	    if (pdrv->owin[win].inited == true) {
	        devm_release_resource(dev, &pres->owin[win]);
	        HCD_INFO("Core [%d] release owin[%d] [0x%llx]\r\n", pdrv->core.info.id,
	           win, (u64)pres->owin[win].start);
	        pdrv->owin[win].inited = false;
	    }
	}
	if (pdrv->core.info.base && pdrv->core.res.base.start) {
	    devm_iounmap(dev, pdrv->core.info.base);
	    HCD_INFO("Core [%d] unmap reg base [0x%px]\r\n", pdrv->core.info.id,
	        pdrv->core.info.base);
	    pdrv->core.info.base = NULL;
	}

	HCD_FN_RET();
}

/*
 * Function pcie_hcd_parse_dt (pdrv)
 *
 *   Parameters:
 *    pdrv ... pointer to pcie core hcd data structure
 *
 *   Description:
 *    Parse pcie core hcd device tree entries. Currently supported resources
 *       - PCIe core base, memory window, PCI bus range
 *
 *   Return: 0 on success, -ve on failure
 */
static int pcie_hcd_parse_dt(struct pcie_hcd *pdrv)
{

	struct pcie_hc_core_res *pres = NULL;
	struct platform_device *pdev = NULL;
	struct device_node *np = NULL;
	struct device *dev = NULL;
	int win;
	int err = 0;
	struct of_pci_range_parser parser;
	struct of_pci_range range;
	unsigned int skip_dt = 0;

	HCD_FN_ENT();

	pdev = pdrv->core.pdev;
	pres = &pdrv->core.res;

	skip_dt = hc_plt_cfg[PLTID2HC(pdrv->core.info.id)].flags & HC_CFG_SKP_DT;

	/* Initialize attributes from device tree if present */
	np = pdev->dev.of_node;
	dev = &pdev->dev;
	if (np && (skip_dt == 0)) {
	    HCD_LOG("Core [%d] DT node available: %s\n", pdev->id, np->full_name);

	    /* Check if DT entry status is enabled or okay to load */
	    if (!of_device_is_available(np)) {
	        dev_err(dev, "DT status disabled\n");
	        HCD_FN_RET_VAL(-ENODEV);
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

	        if (win >= MAX_NUM_OUTGOING_WINDOWS) {
	            HCD_ERROR("Core [%d] ougoing windows [%d] exceeds max [%d]\n",
	                pdev->id, win, MAX_NUM_OUTGOING_WINDOWS);
	            HCD_FN_RET_VAL(-EINVAL);
	        }

	        if (win == OWIN0) {
	            /*
	             * If specified address base is different from default,
	             * or size is different from base, we might need to remap
	             * the outgoing window in the UBUS
	             */
	            if ((pres->owin[win].start != range.cpu_addr) ||
	                (pres->owin[win].end != (range.cpu_addr + range.size - 1))) {
	                pdrv->owin[win].need_remap = 1;
	            }
	        } else {
	            pdrv->owin[win].need_remap = 1;
	        }

	        pres->owin[win].flags = range.flags;
	        pres->owin[win].parent = pres->owin[win].child = pres->owin[win].sibling = NULL;
	        pres->owin[win].name = np->full_name;
	        pres->owin[win].start = range.cpu_addr;
	        pres->owin[win].end = range.cpu_addr + range.size - 1;
	        pdrv->owin[win].pci_addr = range.pci_addr;

	        win++;
	    }

	    if (HCD_USE_DT_ENTRY(pdev->id) && (pres->irq == 0)) {
	        pres->irq = irq_of_parse_and_map(np, 0);
	        err = (pres->irq > 0) ? 0 : 1;
	        HCD_FAIL_ON_DT_ERROR("interrupt", err);
	    }

	    /* PCI bus range */
	    err = of_pci_parse_bus_range(np, &pres->bus_range);
	    HCD_WARN_ON_DT_ERROR("busnumber", err);
	    if (pres->bus_range.end > PCIE_HCD_MAX_BUSNUM)
	        pres->bus_range.end = PCIE_HCD_MAX_BUSNUM;

	    /* PCIe Link width */
	    err = of_property_read_u32(np, "brcm,num-lanes", &pres->link_width);
	    HCD_WARN_ON_DT_ERROR("brcm,num-lanes", err);
	    if (pres->link_width > PCIE_LINK_WIDTH_MAX)
	        pres->link_width = PCIE_LINK_WIDTH_MAX;

	    /* PCIe port speed */
	    err = of_property_read_u8(np, "brcm,speed", &pdrv->core.cfg.speed);
	    HCD_WARN_ON_DT_ERROR("brcm,speed", err);

	    /* PCIe port power mode */
	    err = of_property_read_u8(np, "brcm,phypwrmode", &pdrv->core.cfg.phypwrmode);
	    HCD_WARN_ON_DT_ERROR("brcm,phypwrmode", err);

	    if (of_property_read_bool(np, "brcm,dual-lane"))
	        pdrv->core.cfg.is_dual_lane = 1;
	    else
	        pdrv->core.cfg.is_dual_lane = 0;

	    /* PCIe port error logging */
	    err = of_property_read_u8(np, "brcm,errlog", &pdrv->core.cfg.errlog);
	    HCD_WARN_ON_DT_ERROR("brcm,errlog", err);

	    {
	        u8 dt_val;

	        /* PCIe force power on setting */
	        err = of_property_read_u8(np, "brcm,apon", &dt_val);
	        if (err == 0) {
	            pdrv->core.cfg.apon = dt_val;
	        }
	        HCD_WARN_ON_DT_ERROR("brcm,apon", err);

	        /* PCIe ssc configuration */
	        err = of_property_read_u8(np, "brcm,acc", &dt_val);
	        if (err == 0) {
	            pdrv->core.cfg.acc = (dt_val) ? true : false;
	        }
	        HCD_WARN_ON_DT_ERROR("brcm,acc", err);
	    }
	} else {
	    for (win = OWIN0; win < MAX_NUM_OUTGOING_WINDOWS; win++) {
	        pdrv->owin[win].pci_addr = pres->owin[win].start;
	    }
	}

	HCD_INFO("Core [%d] regs: start [0x%llx] end [0x%llx] flags [0x%lx]\r\n",
	    pdev->id, (u64)pres->base.start, (u64)pres->base.end,
	    pres->base.flags);
	HCD_INFO("Core [%d] Interrupt [%d]\r\n", pdev->id, pres->irq);
	HCD_INFO("Core [%d] bus_range: start [0x%llx] end [0x%llx] flags [0x%lx]\r\n",
	    pdev->id, (u64)pres->bus_range.start, (u64)pres->bus_range.end,
	    pres->bus_range.flags);
	for (win = OWIN0; win < MAX_NUM_OUTGOING_WINDOWS; win++) {
	    if (pres->owin[win].start) {
	        HCD_INFO("Core [%d] owin[%d]: start [0x%llx] end [0x%llx] flags [0x%lx]\r\n",
	            pdev->id, win, (u64)pres->owin[win].start, (u64)pres->owin[win].end,
	            pres->owin[win].flags);
	    }
	}
	HCD_INFO("Core [%d] lanes: [%d] speed: [%d] ssc: [%d] phypwrmode: [%d]\r\n", pdev->id,
	    pres->link_width, pdrv->core.cfg.speed, pdrv->core.cfg.ssc, pdrv->core.cfg.phypwrmode);

	HCD_FN_RET_VAL(0);
}

/*
 * Function pcie_hcd_cpci_probe (pdev)
 *
 *   Parameters:
 *    pdev ... pointer to pcie platform device data structure
 *
 *   Description:
 *    read and initialize the pcie device tree entries common for all cores
 *
 *   Return: 0 on processed, -ve on not processed
 */
static int pcie_hcd_cpci_probe(struct platform_device *pdev)
{
	int err = -1;
	struct device_node *np = pdev->dev.of_node;
	const char *type = NULL;
	u32 dt_val;

	HCD_FN_ENT();

	if (!np) {
	    /* No device tree entry */
	    goto done;
	}

	err = of_property_read_string(np, "device_type", &type);
	if ((err != 0) || (type == NULL) || (strcmp(type, "cpci") != 0)) {
	    /* Not a common pcie device tree entry */
	    err = -1;
	    goto done;
	}

	HCD_INFO("common core settings\r\n");

	/* read common pcie device configuration */
	err = of_property_read_u32(np, "brcm,pllclkid", &dt_val);
	if (err == 0) {
	    pcie_pllclkid = dt_val;
	}
	HCD_WARN_ON_DT_ERROR("brcm,pllclkid", err);

	/* PCIe ssc configuration */
	err = of_property_read_u32(np, "brcm,ssc", &dt_val);
	if (err == 0) {
	    pcie_ssc_cfg = dt_val;
	}
	HCD_WARN_ON_DT_ERROR("brcm,ssc", err);

	/* PCIe PLL Clock Adjustment */
	err = of_property_read_u32(np, "brcm,pllclkadj", &dt_val);
	if (err == 0) {
	    pcie_pllclkadj = dt_val;
	}
	HCD_WARN_ON_DT_ERROR("brcm,pllclkadj", err);

	/* Disable SSC if PLL Clock frequency adjustment is enabled */
	if (pcie_pllclkadj && pcie_ssc_cfg) {
	    HCD_WARN("Force disable SSC on PLL Frequency Adjustment enable\n");
	    /* disable for port#1,2,3. Port#0 setting unchanged */
	    pcie_ssc_cfg &= 0x000F;
	}

	platform_set_drvdata(pdev, NULL);
	err = 0;

done:
	HCD_FN_RET_VAL(err);
}

/*
 * Function pcie_hcd_cpci_remove (pdev)
 *
 *
 *   Parameters:
 *    pdev ... pointer to pcie platform device data structure
 *
 *   Description:
 *    Currently a place holder
 *
 *   Return: 0 on success, -ve on failure
 */
static int  pcie_hcd_cpci_remove(struct platform_device *pdev)
{
	HCD_FN_ENT();

	HCD_FN_RET_VAL(0);
}

/*
 * Function pcie_hcd_probe (pdev)
 *
 *   Parameters:
 *    pdev ... pointer to pcie platform device data structure
 *
 *   Description:
 *    PCIe hcd driver probe. Called for each instance of the PCIe core.
 *    Get and allocate resource, configure hardware, start the PCI bus and
 *    enumerate PCI devices
 *
 *   Return: 0 on success, -ve on failure
 */
static int pcie_hcd_probe(struct platform_device *pdev)
{
	struct pcie_hcd *pdrv = NULL;
	struct device_node *np = pdev->dev.of_node;
	u32 core = (u32)pdev->id;
	int	err = 0;
	struct pci_bus *bus;
	struct pci_bus *child;
	struct pci_ops *pci_fops = NULL;
	int active;
	LIST_HEAD(res);

	HCD_FN_ENT();

	if (np && pcie_hcd_cpci_probe(pdev) == 0) {
	    HCD_FN_RET_VAL(0);
	}

	/* If coming from device tree, use device tree entry to find the core id */
	if (np && (pdev->id <= PLATFORM_DEVID_NONE)) {
	    if (of_property_read_u32(np, "brcm,coreid", &core) < 0) {
	        HCD_ERROR("Unable to get coreid from device tree\r\n");
	        HCD_FN_RET_VAL(-ENODEV);
	    }
	    pdev->id = core;
	}

	HCD_INFO("Core [%d] probe\r\n", core);

	if (core >= NUM_CORES) {
	    HCD_FN_RET_VAL(-ENODEV);
	}

	if (pcie_hcd_port_enabled(core)) {

	    if (pcie_hcd_get_port_mode(core) == 0) {
	        HCD_ERROR("Core [%d] Not in RC Mode\r\n", core);
	        err = -ENODEV;
	        goto error;
	    }

	    /* Allocate HCD control block */
	    pdrv = kzalloc(sizeof(*pdrv), GFP_KERNEL);
	    if (!pdrv) {
	        HCD_ERROR("Core [%d] Unable to allocate memory for CB\r\n", core);
	        err =  -ENOMEM;
	        goto error;
	    }
	    HCD_INFO("Core [%d] Allocated [0x%px] hcd\r\n", core, pdrv);

	    /* Initialize  hcd elements */
	    platform_set_drvdata(pdev, pdrv);

	    /* Initialize  core resource element values for no device tree based
	     * legacy drivers
	     */
	    err = pcie_hc_init_core(pdev, pdrv);
	    if (err < 0) {
	        HCD_ERROR("Core [%d] failed to initialize HC\r\n", core);
	        err =  -EINVAL;
	        goto error;
	    }

	    /* Update core resource elements  (DT based) */
	    err = pcie_hcd_parse_dt(pdrv);
	    if (err < 0) {
	        HCD_ERROR("Core [%d] failed to update dt entries\r\n", core);
	        err =  -EINVAL;
	        goto error;
	    }

	    /*
	     * Power on the port devices,
	     * if Configured, skip port enumeration and power off the port device
	     */
	    active = pcie_hc_apon_pwrup(pdrv);
	    if (active < 0) {
	        HCD_LOG("Core [%d] skip due to apon setting [%d]\r\n", core, pdrv->core.cfg.apon);
		err = active;
	        goto error;
	    }

	    /* Setup PCIe core memory window */
	    err = pcie_hcd_setup_resources(pdrv, &res);
	    if (err < 0) {
	        HCD_ERROR("Core [%d] failed to setup owin resource, err [%d]\r\n",
	            core, err);
	        err =  -ENOMEM;
	        goto error;
	    }

	    if (pdrv->core.plt_cfg->pci_fops->map_bus)
	        pci_fops = &pcie_hcd_ops_generic;
	    else
	        pci_fops = &pcie_hcd_ops_specific;

	    /* No more active devices, skip enumeration, create bus and exit */
	    bus = pci_create_root_bus(&pdev->dev, pdrv->core.res.bus_range.start,
	        pci_fops, &pdrv->core, &res);
	    if (!bus) {
	        HCD_ERROR("Core [%d] failed to create root bus: %d\r\n", core, err);
	        err =  -ENXIO;
	        goto error;
	    }
	    pdrv->bus = bus;

	    if (active == 0) {
	        pcie_hcd_procfs_init(pdrv);
	        HCD_LOG("Core [%d] skip due to apon setting [%d]\r\n", core, pdrv->core.cfg.apon);
	        goto done;
	    }

	    /* setup pcie Core registers for access to PCIe core */
	    err = pcie_hc_setup_access(pdrv);
	    if (err < 0) {
	        HCD_ERROR("Core [%d] failed to setup regs, err [%d]\r\n", core,
	            err);
	        err =  -ENOMEM;
	        goto error;
	    }

	    /* lets talk to PCIe core, reset the core */
	    pcie_hc_reset_core(pdrv);

	    /* Check if PCIe link is up (for any device connected on the link) */
	    err = pcie_hc_link_up(pdrv);
	    if (err < 0) {
	        /* No device connected to PCIe core */
	        HCD_ERROR("Core [%d] link is DOWN\r\n", core);
	        goto error;
	    }

	    /* Got all core resources, Now configure the PCIe core */
	    err = pcie_hc_config_core(pdrv);
	    if (err < 0) {
	        HCD_ERROR("Core [%d] failed to setup hw, err [%d]\r\n", core, err);
	        err =  -ENODEV;
	        goto error;
	    }

	    /* Now initialize the PCIe core error logging */
	    if (pdrv->core.cfg.errlog) {
	        err = pcie_hcd_errlog_enable(pdrv);
	        if (err < 0) {
	            HCD_ERROR("Core [%d] failed to setup error logging, err [%d]\r\n", core, err);
	            err =  -ENODEV;
	            goto error;
	        }
	    }

	    if (IS_ENABLED(CONFIG_PCI_MSI) && pdrv->core.cfg.msi) {
	        err = pcie_hcd_msi_setup(pdrv);
	        if (err < 0) {
	            HCD_ERROR("failed to enable MSI support: %d\n", err);
	            goto error;
	        }
	    }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
	    {
	        struct pci_host_bridge *bridge = to_pci_host_bridge(bus->bridge);

#ifdef CONFIG_PCI_MSI
	        bridge->msi = (struct msi_controller*)pdrv->msi;
	        bus->msi = bridge->msi;
#endif /* CONFIG_PCI_MSI */
	        bridge->map_irq = pcie_hcd_map_irq;
	        bridge->swizzle_irq = pci_common_swizzle;

	        pci_scan_child_bus(bus);

	        pci_bus_size_bridges(bus);
	        pci_bus_assign_resources(bus);
	   }
#else /* LINUX_VERSION < 4.19.0 */

	    /* Now do the PCI setup,
	     * - create,scan bus
	     * - assign resources, irq
	     * - add connected devices
	     */
	    if (IS_ENABLED(CONFIG_PCI_MSI)) {
#ifdef CONFIG_ARM
#ifdef CONFIG_PCI_MSI
	        pdrv->sys.msi_ctrl = (struct msi_controller*)pdrv->msi;
#endif /* CONFIG_PCI_MSI */
#endif /* CONFIG_ARM */
	        bus->msi = (struct msi_controller*)pdrv->msi;
	    }

	    pci_scan_child_bus(bus);

	    pci_assign_unassigned_bus_resources(bus);
#ifdef CONFIG_ARM
	    /* Apply irq fixup */
	    pci_fixup_irqs(pci_common_swizzle, pcie_hcd_map_irq);
#endif /* CONFIG_ARM */
#endif /* LINUX_VERSION < 4.19.0 */

	    pci_bus_add_devices(bus);

	    /* Configure PCI Express settings */
	    list_for_each_entry(child, &bus->children, node)
	        pcie_bus_configure_settings(child);

	    pcie_hcd_procfs_init(pdrv);
	    err = 0;
	} else {
	    HCD_ERROR("Core [%d] disabled\n", core);
	    err = -ENODEV;
	}

error:

	if (err) {
	    pcie_hcd_remove(pdev);
	}

done:
	pci_free_resource_list(&res);

	HCD_FN_RET_VAL(err);
}

/*
 * Function pcie_hcd_remove (pdev)
 *
 *
 *   Parameters:
 *    pdev ... pointer to pcie platform device data structure
 *
 *   Description:
 *    PCIe hcd driver remove Free the resources and power down the PCIe core
 *
 *   Return: 0 on success, -ve on failure
 */
static int  pcie_hcd_remove(struct platform_device *pdev)
{
	struct pcie_hcd *pdrv = platform_get_drvdata(pdev);
	int core = pdev->id;

	HCD_FN_ENT();

	if (!pdrv) {
	    pcie_hcd_cpci_remove(pdev);
	    HCD_FN_RET_VAL(0);
	}

	if (pcie_hcd_port_enabled(core)) {

	    pcie_hcd_procfs_deinit(pdrv);

	    if (pdrv->bus) {
	        pci_stop_root_bus(pdrv->bus);
	        pci_remove_root_bus(pdrv->bus);
	    }

	    if (IS_ENABLED(CONFIG_PCI_MSI)) {
	        pcie_hcd_msi_disable(pdrv);
	    }

	    pcie_hcd_errlog_disable(pdrv);

	    pcie_hcd_unmap_resources(pdrv);

	    /* power off ports based on apon setting */
	    pcie_hc_apon_pwrdn(pdrv);

	    pcie_hc_free_core(pdrv);

	    kfree(pdrv);
	}

	HCD_FN_RET_VAL(0);
}

/*
 * +-----------------------------------------------------
 *  Global Functions
 * +-----------------------------------------------------
 */
/*
 * Function pcie_hcd_init ()
 *
 *   Parameters:
 *
 *   Description:
 *    PCIe hcd driver init, register the driver from platform list including misc driver
 *    This inturn should call our probe()
 *
 *   Return: 0 on success, -ve on failure
 */
static int __init pcie_hcd_init(void)
{
	int ret;
	int index;
	struct platform_driver *phcd_drv = &pcie_hcd_driver;

	HCD_FN_ENT();

	printk("Broadcom PCIe Host Controller Driver (impl%d)\r\n", CONFIG_BCM_PCIE_HCD_IMPL);

#if defined(MODULE)
	if ((ret = pcie_hcd_nvram_init()) != 0) {
	    HCD_ERROR("Failed to initialize nvram parameters\r\n");
	    goto done;
	}

	/* Need to initialize platform dev for loadable kernel module */
	if ((ret = pcie_hcd_plt_init()) != 0) {
	    HCD_ERROR("Failed to initialize platform device\r\n");
	    goto done;
	}
#endif /* MODULE */

	/* Register common hcd driver */
	HCD_INFO("register [%s] driver with platform driver\r\n",
	    phcd_drv->driver.name);
	ret = platform_driver_register(phcd_drv);
	if (ret != 0) {
	    HCD_ERROR("Failed to register%s driver with platform driver\r\n",
	        phcd_drv->driver.name);
	    goto done;
	}

	/* Register individual HC drivers, if different from hcd driver */
	for (index = 0; index < MAX_HC_TYPE; index++) {
	    int hc = pcie_hcd_hc_enum_order[index];
	    struct platform_driver *phc_drv = hc_plt_cfg[hc].plt_drv;

	    if ((hc_plt_cfg[hc].num_cores) && (phc_drv != phcd_drv)) {

	        HCD_INFO("register [%s] driver with platform driver\r\n", phc_drv->driver.name);
	        ret = platform_driver_register(phc_drv);
	        if (ret != 0) {
	            HCD_ERROR("Failed to register [%s] driver with platform driver\r\n",
	                phc_drv->driver.name);
	            goto done;
	        }
	    }
	}

done:
	HCD_FN_RET_VAL(ret);
}

/*
 * Function pcie_hcd_exit ()
 *
 *   Parameters:
 *
 *   Description:
 *    PCIe hcd driver exit, unregister the driver from platform list including misc driver
 *    This inturn should call our remove()
 *
 *   Return: None
 */
static void __exit pcie_hcd_exit(void)
{
	struct platform_driver *phcd_drv = &pcie_hcd_driver;
	int    index;

	HCD_FN_ENT();

	/* Un-register individual HC drivers, if different from hcd driver */
	for (index = MAX_HC_TYPE-1; index >= 0; index--) {
	    int hc = pcie_hcd_hc_enum_order[index];
	    struct platform_driver *phc_drv = hc_plt_cfg[hc].plt_drv;

	    if ((hc_plt_cfg[hc].num_cores) && (phc_drv != phcd_drv)) {

	        HCD_INFO("Un register [%s] driver with platform driver\r\n",
	            phc_drv->driver.name);
	        platform_driver_unregister(phc_drv);
	    }
	}

	/* Un-register pcie hcd driver */
	HCD_INFO("Un register [%s] driver with platform driver\r\n",
	    phcd_drv->driver.name);
	platform_driver_unregister(phcd_drv);


#if defined(MODULE)
#if defined(CONFIG_BCM_KF_PCI_RESET_DOMAIN_NR)
	pci_reset_domain_nr();
#endif /* CONFIG_BCM_KF_PCI_RESET_DOMAIN_NR */
	pcie_hcd_plt_deinit();
#endif /* MODULE */

	HCD_FN_RET();
}

module_init(pcie_hcd_init);
module_exit(pcie_hcd_exit);
MODULE_LICENSE("GPL");

/*
 * Function pcie_hcd_plt_dev_release (dev)
 *
 *   Parameters:
 *    pdrv ... pointer to pcie platform device data structure
 *
 *   Description:
 *    PCIe platform device release callback
 *
 *   Return: None
 */
static void pcie_hcd_plt_dev_release(struct device *dev)
{
	HCD_FN_ENT();
	/* Nothing to do */
	HCD_FN_RET();
}

/*
 * Function pcie_hcd_plt_init ()
 *
 *   Parameters:
 *
 *   Description:
 *    PCIe platform setup. Add PCIe cores to the platform devices
 *    Generally this is done outside, but since there is no support outside,
 *    it is done here.
 *
 *   Return: 0 on success, -ve value on failure
 */
static int __init pcie_hcd_plt_init(void)
{
	int i, core, hc;
	int ret = 0;
	struct platform_device	*pdev = NULL;

	HCD_FN_ENT();

	if (NUM_CORES < 0) {
	    NUM_VCORES = 0;
	    NUM_PCORES = 0;

	    hc_plt_cfg[PHC].plt_dev = &pcie_hcd_plt_dev[NUM_CORES];
	    ret = pcie_phc_plt_init(&hc_plt_cfg[PHC]);
	    if (ret < 0) {
	        HCD_ERROR("Failed to initialize HC\r\n");
	        HCD_FN_RET_VAL(ret);
	    }

#if defined(CONFIG_BCM_PCIE_VCD)
	    hc_plt_cfg[VHC].plt_dev = &pcie_hcd_plt_dev[NUM_CORES];
	    ret = pcie_vhc_plt_init(&hc_plt_cfg[VHC]);
	    if (ret < 0) {
	        HCD_ERROR("Failed to initialize VHC\r\n");
	        HCD_FN_RET_VAL(ret);
	    }
#endif /* CONFIG_BCM_PCIE_VCD */

	    for (hc = PHC; hc < MAX_HC_TYPE; hc++) {
	        if ((hc_plt_cfg[hc].num_cores &&
	            ((hc_plt_cfg[hc].plt_drv->probe == pcie_hcd_driver.probe) ||
	             (hc_plt_cfg[hc].plt_drv->remove == pcie_hcd_driver.remove))) &&
	            ((hc_plt_cfg[hc].init_core == NULL) || (hc_plt_cfg[hc].config_core == NULL) ||
	             (hc_plt_cfg[hc].read_reg == NULL) || (hc_plt_cfg[hc].write_reg == NULL))) {

	            HCD_ERROR("Failed to initialize platform driver fops\r\n");
	            HCD_ERROR("PCIe Platform HC[%d] Configuration:\n", hc);
	            HCD_ERROR("num_cores           :%d\n", hc_plt_cfg[hc].num_cores);
	            HCD_INFO("flags               :0x%08x\n", hc_plt_cfg[hc].flags);
	            HCD_ERROR("plt_dev             :0x%px\n", hc_plt_cfg[hc].plt_dev);
	            HCD_ERROR("plt_drv             :0x%px\n", hc_plt_cfg[hc].plt_drv);
	            HCD_ERROR("pci_fops            :0x%px\n", hc_plt_cfg[hc].pci_fops);
	            HCD_ERROR("init_core           :0x%px\n", hc_plt_cfg[hc].init_core);
	            HCD_ERROR("free_core           :0x%px\n", hc_plt_cfg[hc].free_core);
	            HCD_ERROR("read_reg            :0x%px\n", hc_plt_cfg[hc].read_reg);
	            HCD_ERROR("write_reg           :0x%px\n", hc_plt_cfg[hc].write_reg);
	            HCD_ERROR("config_core         :0x%px\n", hc_plt_cfg[hc].config_core);
	            HCD_ERROR("reset_core          :0x%px\n", hc_plt_cfg[hc].reset_core);
	            HCD_ERROR("is_linkup           :0x%px\n", hc_plt_cfg[hc].is_linkup);
	            HCD_ERROR("map_core_irq        :0x%px\n", hc_plt_cfg[hc].map_core_irq);
	            HCD_ERROR("setup_msi           :0x%px\n", hc_plt_cfg[hc].setup_msi);
	            HCD_ERROR("teardown_msi        :0x%px\n", hc_plt_cfg[hc].teardown_msi);
	            HCD_ERROR("setup_errint        :0x%px\n", hc_plt_cfg[hc].setup_errint);
	            HCD_ERROR("teardown_errint     :0x%px\n", hc_plt_cfg[hc].teardown_errint);
	            HCD_FN_RET_VAL(-EINVAL);
	        } else {
	            HCD_INFO("PCIe Platform HC[%d] Configuration:\n", hc);
	            HCD_INFO("num_cores           :%d\n", hc_plt_cfg[hc].num_cores);
	            HCD_INFO("flags               :0x%08x\n", hc_plt_cfg[hc].flags);
	            HCD_INFO("plt_dev             :0x%px\n", hc_plt_cfg[hc].plt_dev);
	            HCD_INFO("plt_drv             :0x%px\n", hc_plt_cfg[hc].plt_drv);
	            HCD_INFO("pci_fops            :0x%px\n", hc_plt_cfg[hc].pci_fops);
	            HCD_INFO("init_core           :0x%px\n", hc_plt_cfg[hc].init_core);
	            HCD_INFO("free_core           :0x%px\n", hc_plt_cfg[hc].free_core);
	            HCD_INFO("read_reg            :0x%px\n", hc_plt_cfg[hc].read_reg);
	            HCD_INFO("write_reg           :0x%px\n", hc_plt_cfg[hc].write_reg);
	            HCD_INFO("config_core         :0x%px\n", hc_plt_cfg[hc].config_core);
	            HCD_INFO("reset_core          :0x%px\n", hc_plt_cfg[hc].reset_core);
	            HCD_INFO("is_linkup           :0x%px\n", hc_plt_cfg[hc].is_linkup);
	            HCD_INFO("map_core_irq        :0x%px\n", hc_plt_cfg[hc].map_core_irq);
	            HCD_INFO("setup_msi           :0x%px\n", hc_plt_cfg[hc].setup_msi);
	            HCD_INFO("teardown_msi        :0x%px\n", hc_plt_cfg[hc].teardown_msi);
	            HCD_INFO("setup_errint        :0x%px\n", hc_plt_cfg[hc].setup_errint);
	            HCD_INFO("teardown_errint     :0x%px\n", hc_plt_cfg[hc].teardown_errint);
		}
	    }

	    HCD_LOG("DEF CFG: ssc 0x%x, speed  0x%x, bootorder 0x%x, apon 0x%x\n",
	        pcie_ssc_cfg, pcie_speed_cfg, pcie_boot_order, pcie_apon);
	    HCD_LOG("       : pwr 0x%x, errlog 0x%x, msi 0x%x, acc  0x%x, pllclkid 0x%x\n",
	        pcie_phy_pwrmode, pcie_errlog, pcie_msi, pcie_acc, pcie_pllclkid);
	    HCD_LOG("       : loglevel 0x%x pllclkadj 0x%x\n",
	        hcd_log_level, pcie_pllclkadj);
	}

	if (HCD_USE_DT_ENTRY(0)) {
	    /* Nothing to do, DT entries will populate the devices */
	    HCD_INFO("Using PCIe DT to probe\r\n");
	    HCD_FN_RET_VAL(ret);
	}

	/* Register All Pcie cores as platform devices */
	for (i = 0; i < NUM_CORES; i++) {
	    core = pcie_hcd_get_boot_order_core(i);
	    if (core >= NUM_CORES) {
	        HCD_LOG("Core [%d] not in range (> %d)\r\n", core, NUM_CORES);
	        continue;
	    }

	    pdev = &pcie_hcd_plt_dev[core];
	    if (pdev->name == NULL)
	        pdev->name = PCIE_HCD_DEV_NAME;
	    pdev->id = core;
	    pdev->dev.release = pcie_hcd_plt_dev_release;

	    ret = platform_device_register(pdev);
	    if (ret) {
	        HCD_ERROR("Core [%d] unable to register device: ret = %d\n",
	            core, ret);
	    } else {
	        HCD_INFO("Core [%d] registered device successfully\r\n", core);
	    }
	}

	HCD_FN_RET_VAL(ret);
}

/*
 * Function pcie_hcd_plt_deinit ()
 *
 *    Parameters:
 *
 *    Description:
 *     PCIe platform de-initialization. Unregister PCIe cores to the platform devices
 *     Generally this is done outside, but since there is no support outside,
 *     it is done here.
 *
 *   Return: None
 */
static void __init pcie_hcd_plt_deinit(void)
{
	int i, core;
	struct platform_device	*pdev = NULL;

	HCD_FN_ENT();

	if (NUM_CORES < 0) {
	    HCD_INFO("HC configuration not initialized\r\n");
	    HCD_FN_RET();
	}

	if (HCD_USE_DT_ENTRY(0)) {
	    /* Nothing to do, DT entries will populate the devices */
	    HCD_INFO("Using PCIe DT to probe\r\n");
	    HCD_FN_RET();
	}

	/* Un register All Pcie cores as platform devices */
	for (i = 0; i < NUM_CORES; i++) {
	    core = pcie_hcd_get_boot_order_core(i);
	    if (core >= NUM_CORES) {
	        HCD_LOG("Core [%d] not in range (> %d)\r\n", core, NUM_CORES);
	            continue;
	    }

	    pdev = &pcie_hcd_plt_dev[core];
	    if (pdev->name == NULL)
	        pdev->name = PCIE_HCD_DEV_NAME;
	    pdev->id = core;

	    platform_device_unregister(pdev);
	    HCD_INFO("Core [%d] unregister device successfully\r\n", core);
	}

	HCD_FN_RET();
}

#if !defined(MODULE)
/* Only for built-in drivers */
subsys_initcall(pcie_hcd_plt_init);
#endif /* !MODULE */
