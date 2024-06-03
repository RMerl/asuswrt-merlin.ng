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
#include <linux/regulator/consumer.h>
#include "../drivers/pci/pci.h"
#include <linux/irqchip/chained_irq.h>
#include <linux/irqdomain.h>
#include <linux/msi.h>

/* brcm bsp */
#if defined(CONFIG_BRCM_IKOS) || defined(CONFIG_BCM_PCIE_PMC_BRD_STUBS)
#define USE_PMC_BRD_STUBS
#else /* !CONFIG_BRCM_IKOS && !CONFIG_BCM_PCIE_PMC_BRD_STUBS */
#include <board.h>
#include <pmc_pcie.h>
#include <flash_common.h>
#endif /* !CONFIG_BRCM_IKOS && !CONFIG_BCM_PCIE_PMC_BRD_STUBS */
#include <shared_utils.h>
#include <bcm_intr.h>
#include <bcm_nvram.h>
#include <bcm_otp.h>
#include <bcm_strap_drv.h>


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

/* PCI Power ON to PERST# minumum time */
#define PCIE_PWR_2_PERST_DELAY              100 /* milli seconds */
#define PCIE_PWR_SETTLING_DELAY             25  /* milli seconds */

#define MSI_ISR_NAME_STR_LEN                32

#define APON_F_DEFAULT                      (1 << HCD_APON_DEFAULT)
#define APON_F_ON                           (1 << HCD_APON_ON)
#define APON_F_OFF                          (1 << HCD_APON_OFF)
#define APON_F_OFF_W_DOM                    (1 << HCD_APON_OFF_WITH_DOMAIN)

/* HCD Settings */
#define HCD_SETTING_VAL_INV                 0xFFFFFFFF
#define HCD_SETTING_VAL_STR_LEN             32

/* Local Error Codes */
#define EHCD_SCAN_DEFER                     EPROBE_DEFER

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
 * HCD Settings
 *
 * Types:
 *  - Common interdependant settings (cpci)
 *  - Core independant settings      (pcie@)
 *
 * Order:
 * - Driver default (Common and Core)
 * - LKM parameter  (Common and Core)
 * - Common DT      (Common)
 * - Kernel/env NV  (Common)
 * - Core DT        (Core)
 * - Kernel/env NV  (Core)
 */
#define HCD_DECLARE_CORE_SETTING(name, def)             \
	u32 nv_##name = HCD_SETTING_VAL_INV;                \
	u32 name = (def);                                   \
	module_param(name, int, S_IRUGO)

/* Common setting have no nvram backup */
#define HCD_DECLARE_CMMN_SETTING(name, def)             \
	u32 name = (def);                                   \
	module_param(name, int, S_IRUGO)

#define HCD_READ_CMMN_SETTING(name)                     \
	do {                                                \
	    u32 val;                                        \
	    if (pcie_hcd_nvram_get_u32(#name, &val) == 0) { \
	        name = val;                                 \
	    }                                               \
	} while (0)

#define HCD_READ_CORE_SETTING(name)                     \
	do {                                                \
	    u32 val;                                        \
	    if (pcie_hcd_nvram_get_u32(#name, &val) == 0) { \
	        nv_##name = val;                            \
	    }                                               \
	} while (0)

#define HCD_UPDATE_CMMN_SETTING(name)                   \
	HCD_READ_CMMN_SETTING(name)

#define HCD_UPDATE_CORE_SETTING(name)                   \
	if (nv_##name != HCD_SETTING_VAL_INV)  name = nv_##name

#define HCD_GET_CMMN_SETTING(name, core)                \
	    HCD_HC_CORE_CFG(name, core)

#define HCD_GET_CORE_SETTING(name, core)                \
	(nv_##name != HCD_SETTING_VAL_INV) ?                \
	    HCD_HC_CORE_CFG(nv_##name, core) :              \
	    HCD_HC_CORE_CFG(name, core)

/*
 * +-----------------------------------------------------
 *  Structures
 * +-----------------------------------------------------
 */
/*
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
 * @top_domain:    pointer to msi parent irq domain
 * @top_irq_ops:   parent msi domain irq operations
 * @domain:        pointer to msi irq domain
 * @irq_ops:       msi domain irq operations
 * @domain_ops:    msi interrupt domain operations
 * @domain_info:   msi interrupt domain information
 * @lock:          lock for irq alloc/free
 * @used:          MSI interrupts used bit mask
 * @irq:           MSI interrupt number
 * @nr:            Number of msi interrupts
 * @enabled:       flag to specify msi is enabled or not
 * @pdrv:          Pointer to pcie driver cb
 */
struct pcie_hcd_msi {
#ifdef CONFIG_BCM_PCI_MSI
	struct irq_domain      *top_domain;
	struct irq_chip        top_irq_ops;
	struct irq_domain      *domain;
	struct irq_chip        irq_ops;
	struct irq_domain_ops  domain_ops;
	struct msi_domain_info domain_info;
	struct mutex           lock;
	unsigned long          used;
	int                    irq;
#endif /* CONFIG_BCM_PCI_MSI */
	int                    nr;
	bool                   enabled;
	struct pcie_hcd        *pdrv;
};

/*
 * BCM PCIe Host Contoller Driver Outgoing Window control block
 *
 * @pci_addr:   Physical address of PCI core to determine the Offset between
 *              CPU  and PCIe
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
 * BCM PCIe Host Contoller power supply control block
 *
 * @node:      Linked list node
 * @name:      name of the power supply
 * @regulator: regulator control block
 */
struct pcie_hcd_pwrsup {
	struct list_head node;
	char name[32];
	struct regulator *regulator;
	int usage;
};

/*
 * BCM PCIe Host Contoller Driver control block
 *
 * @sys:             PCI sysdata (only arm32 platforms)
 * @msi:             msi control block
 * @bus:             pointer to root bus
 * @pfs:             pointer to pcie hcd proc file system control block
 * @pwr_supplies:    external power supplies for the slot
 * @core:            host controller core control block
 * @errlog:          error log control block
 * @apon:            apon control block
 * @owin:            Outgoing window control block
 */
struct pcie_hcd {
	struct pcie_hcd_msi    *msi;
	struct pci_bus         *bus;
	struct pcie_hcd_procfs *pfs;
	struct pcie_hcd_pwrsup pwrsupplies;

	struct pcie_hc_core    core;
	struct pcie_hcd_errlog errlog;
	struct pcie_hcd_apon   apon;
	struct pcie_hcd_owin   owin[MAX_NUM_OUTGOING_WINDOWS];
};

/*
 * BCM PCIe Periph MISC block
 *
 * @res:            MISC core PCIe control register resource
 * @base:           MISC core PCIe control register mapped address
 */
struct pcie_hcd_periph_misc
{
	struct resource res;
	void __iomem   *base;
};

/*
 * BCM PCIe HCD deferred root bus scan control block
 *
 * @probe_count:     Probed cores count
 * @probe_core:      probed core platform device pointer array
 * @probe_status:    core probe status array
 */
struct pcie_hcd_bus_dsc
{
	int                      probe_count;
	struct platform_device   *probe_pdev[MAX_NUM_PCIE_CORES];
	int                      probe_status[MAX_NUM_PCIE_CORES];
};

/*
 * BCM PCIe Common control block
 *
 * @periph_misc:    MISC core PCIe control register
*  @dsc:            Deferred Scan control block
 */
struct pcie_hcd_cpci
{
	struct pcie_hcd_periph_misc misc;
	struct pcie_hcd_bus_dsc     dsc;
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

#ifdef CONFIG_BCM_PCI_MSI
/* msi interrupt */
static void pcie_hcd_msi_compose_msi_msg(struct irq_data *data,
	struct msi_msg *msg);
static int pcie_hcd_msi_set_affinity(struct irq_data *irq_data,
	const struct cpumask *mask, bool force);
static void pcie_hcd_msi_top_irq_ack(struct irq_data *data);
static int pcie_hcd_msi_irq_domain_alloc(struct irq_domain *domain,
	unsigned int virq, unsigned int nr_irqs, void *args);
static void pcie_hcd_msi_irq_domain_free(struct irq_domain *domain,
	unsigned int virq, unsigned int nr_irqs);
static int pcie_hcd_msi_allocate_domains(struct pcie_hcd_msi *msi);
static void pcie_hcd_msi_free_domains(struct pcie_hcd_msi *msi);
static irqreturn_t pcie_hcd_msi_isr(int irq, void *data);
static int pcie_hcd_msi_enable(struct pcie_hcd *pdrv);
static int pcie_hcd_msi_setup(struct pcie_hcd *pdrv);
static int pcie_hcd_msi_disable(struct pcie_hcd *pdrv);
#else /* !CONFIG_BCM_PCI_MSI */
static inline int pcie_hcd_msi_setup(struct pcie_hcd *pdrv) { return 0; }
static inline int pcie_hcd_msi_disable(struct pcie_hcd *pdrv) { return 0; }
#endif /* ! CONFIG_BCM_PCI_MSI */

/* errlog interrupt */
static int pcie_hcd_errlog_enable(struct pcie_hcd *pdrv);
static void pcie_hcd_errlog_disable(struct pcie_hcd *pdrv);

/* HC core fops access */
static int pcie_hc_link_up(struct pcie_hcd *pdrv);
static void pcie_hc_unconfig_core(struct pcie_hcd *pdrv);
static int pcie_hc_config_core(struct pcie_hcd *pdrv);
static int pcie_hc_reset_core(struct pcie_hcd *pdrv);
static int pcie_hc_restart_core(struct pcie_hcd *pdrv);
static int pcie_hc_setup_access(struct pcie_hcd *pdrv);
static int pcie_hc_init_core(struct platform_device *pdev,
	struct pcie_hcd *pdrv);
static void pcie_hc_free_core(struct pcie_hcd *pdrv);
static int pcie_hc_update_core_cfg(struct pcie_hcd *pdrv);
static void pcie_hc_dump_core_cfg(struct pcie_hcd *pdrv);

/* APON */
static int pcie_hc_apon_pwrup(struct pcie_hcd *pdrv);
static int pcie_hc_apon_pwrdn(struct pcie_hcd *pdrv);

/* Hc helper functions */
static int pcie_hcd_set_power(struct pcie_hc_core *phc, unsigned int devidx,
	bool up, int type);

/* Power */
static int pcie_hcd_set_regulators(struct pcie_hcd *pdrv, unsigned int devidx,
	bool up);
static int pcie_hcd_init_regulators(struct pcie_hcd *pdrv);
static void pcie_hcd_free_regulators(struct pcie_hcd *pdrv);

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
static int pcie_hcd_scan_bus(struct pcie_hcd *pdrv);
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
	    .of_match_table = of_match_ptr(pcie_hcd_of_match)
	},
	.remove = pcie_hcd_remove
};

/* Virtual core first and then physical core */
static int pcie_hcd_hc_enum_order[MAX_HC_TYPE] = {
	VHC,
	PHC
};

/* Common control block */
static struct pcie_hcd_cpci pcie_hcd_cpci_cb;

/*
 * config_ssc values
 * (if exists, Device Tree entry has higher preference than this setting)
 *
 *     0 - disable
 *     1 - Enable
 *
 *     Each 4bit's corresponds to a PCIe core
 *     [31-28] [27-24][23-20] [19-16] [15-12] [11-08] [07-04] [03-00]
 *                                    [core4] [core3] [core2] [core1]
 */
HCD_DECLARE_CMMN_SETTING(pcie_ssc_cfg, 0x0000);

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
HCD_DECLARE_CMMN_SETTING(pcie_pllclkid, 0x3210);

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
HCD_DECLARE_CMMN_SETTING(pcie_pllclkadj, 0x0000);

/* HCD Logging */
#ifdef HCD_DEBUG
HCD_DECLARE_CMMN_SETTING(pcie_log_level, HCD_LOG_LVL_ERROR);
#endif

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
HCD_DECLARE_CMMN_SETTING(pcie_boot_order, 0x0000);

/*
 * PCIe BAR mask Configuration for DDR memory
 * (if exists, Device Tree entry has higer preference than this setting)
 *
 *   [use] Can be used to configure the BAR for the DDR memory
 *     0 - Do not use BAR# for DDR memory access
 *     1 - Use BAR# for DDR memory access
 *
 *     Each bit corresponds to the bar number
 *     [31-07] [ 06 ] [ 05 ] [ 04 ] [ 03 ] [ 02 ] [ 01 ] [ 00 ]
 *     [RSVD ] [BAR6] [BAR5] [BAR4] [BAR3] [BAR2] [BAR1] [RSVD]
 */
HCD_DECLARE_CMMN_SETTING(pcie_barmask, 0x0000);

/*
 * core device scan order (right -> left)
 * (if exists, Device Tree entry has higer preference than this setting)
 *
 *     0x0000 - default ( 0, 1, 2, 3 ..... )
 *     0x3210 - scan order 0, 1, 2, 3
 *     0x0012 - scan order 2, 1, 0
 *     0x0540 - scan order 0, 4, 5
 *
 *     Each 4bit's corresponds to a PCIe core id
 *     [31-28] [27-24] [23-20] [19-16] [15-12] [11-08] [07-04] [03-00]
 *                     [ 6th ] [ 5th ] [ 4th ] [ 3rd ] [ 2nd ] [ 1st ]
 *
 *     Core#4,5 are virtual PCIe core#0,1 respectively
 */
HCD_DECLARE_CMMN_SETTING(pcie_scan_order, 0x0000);

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
HCD_DECLARE_CORE_SETTING(pcie_speed_cfg, 0x0000);

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
HCD_DECLARE_CORE_SETTING(pcie_apon, 0x0000);

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
HCD_DECLARE_CORE_SETTING(pcie_phy_pwrmode, 0x0000);

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
HCD_DECLARE_CORE_SETTING(pcie_errlog, 0x1111);

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
HCD_DECLARE_CORE_SETTING(pcie_msi, 0x1111);
#else
HCD_DECLARE_CORE_SETTING(pcie_msi, 0x0000);
#endif

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
HCD_DECLARE_CORE_SETTING(pcie_numlanes, 0x0000);

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
HCD_DECLARE_CORE_SETTING(pcie_acc, 0x1111);

/*
 * Link Up Retries
 * (if exists, Device Tree entry has higer preference than this setting)
 *
 *     0 - No Link up retries
 *     [1 - 15] Number of Link up retries
 *
 *     Each 4bit's corresponds to a PCIe core
 *     [31-28] [27-24][23-20] [19-16] [15-12] [11-08] [07-04] [03-00]
 *                                    [core4] [core3] [core2] [core1]
 */
HCD_DECLARE_CORE_SETTING(pcie_linkupretries, 0x0000);

/*
 * phy rx filter
 * (if exists, Device Tree entry has higer preference than this setting)
 *
 *     0 - Disable MDIO Rx Filtering
 *     1 - Enanle RC MDIO Rx Filtering
 *     2 - Enanle EP MDIO Rx Filtering
 *     3 - Enanle RC+EP MDIO Rx Filtering
 *
 *     Each 4bit's corresponds to a PCIe core
 *     [31-28] [27-24][23-20] [19-16] [15-12] [11-08] [07-04] [03-00]
 *                                    [core4] [core3] [core2] [core1]
 */
HCD_DECLARE_CORE_SETTING(pcie_phyrxfilter, 0x0000);

/*
 * End Point Power On (GPIO Regulator) after PERSTB asset
 * (if exists, Device Tree entry has higer preference than this setting)
 *
 *     0 - Regulator Power up during RC power Up
 *     1 - Regulator power up after PERSTB=1 (default)
 *
 *     Each 4bit's corresponds to a PCIe core
 *     [31-28] [27-24][23-20] [19-16] [15-12] [11-08] [07-04] [03-00]
 *                                    [core4] [core3] [core2] [core1]
 */
HCD_DECLARE_CORE_SETTING(pcie_epuperstb, 0x1111);


/* procfs */
static struct pcie_hcd_procfs pcie_hcd_procfs_cb;

#ifdef CONFIG_BCM_PCI_MSI
/* msi interrupts */
static char pcie_hcd_msi_name[MAX_NUM_PCIE_CORES][MSI_ISR_NAME_STR_LEN];
#endif /* CONFIG_BCM_PCI_MSI */

/* platform */
static struct platform_device pcie_hcd_plt_dev[MAX_NUM_PCIE_CORES];
struct pcie_hc_plt_cfg hc_plt_cfg[MAX_HC_TYPE] = {
	{
	    .num_cores = -1,
	    .plt_dev   = pcie_hcd_plt_dev,
	    .plt_drv   = &pcie_hcd_driver,
	    .pci_fops  = &pcie_hcd_ops_generic,
	    .set_power = pcie_hcd_set_power
	},
	{
	    .num_cores = -1,
	    .plt_dev   = pcie_hcd_plt_dev,
	    .plt_drv   = &pcie_hcd_driver,
	    .pci_fops  = &pcie_hcd_ops_generic,
	    .set_power = pcie_hcd_set_power
	}
};

/*
 * +-----------------------------------------------------
 *  Local inline functions
 * +-----------------------------------------------------
 */
#if defined(USE_PMC_BRD_STUBS)
static inline int pcie_hcd_pmc_power_up(int unit, int is_dual_lane) { return 0; }
static inline int pcie_hcd_pmc_power_down(int unit, int is_dual_lane) { return 0; }
static inline int pcie_hcd_get_port_mode(int port, struct device_node *np) {
	/* Alwyas RC mode */
	return 1;
}

static inline int pcie_hcd_port_enabled(int port, struct device_node *np) {
	if (PLTID2HC(port) == VHC)
	{
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
static inline int pcie_hcd_ubus_decode_pcie_wnd_cfg(u32 base, u32 size,
	u32 core) {
	/* Not supported */
	return -1;
}

#else /* !USE_PMC_BRD_STUBS */
static inline int pcie_hcd_pmc_power_up(int unit, int is_dual_lane) {
	if (PLTID2HC(unit) == VHC) {
	    /* Virtual port: Nothing to do */
	    return 0;
	}
	return pmc_pcie_power_up(unit, is_dual_lane);
}
static inline int pcie_hcd_pmc_power_down(int unit, int is_dual_lane) {
	if (PLTID2HC(unit) == VHC) {
	    /* Virtual port: Nothing to do */
	    return 0;
	}
	return pmc_pcie_power_down(unit, is_dual_lane);
}
static inline int pcie_hcd_get_port_mode(int port, struct device_node *np) {
	int ret = 1;

	if (PLTID2HC(port) == VHC) {
	    /* Virtual port always RC */
	    goto exit;
	}
	if (bcm_strap_parse_and_test(np, "brcm,strap-pcie-rc-mode") == 0)
	    ret = 0;

exit:
	return ret;
}
static inline int pmc_hcd_get_chip_id(void) {
	return kerSysGetChipId();
}
static inline int pcie_hcd_ubus_decode_pcie_wnd_cfg(u32 base, u32 size, u32 core) {
	return ubus_decode_pcie_wnd_cfg(base, size, core);
}

static inline int pcie_hcd_port_enabled(int port, struct device_node *np)
{
	unsigned int otp_value = 0;
	int is_port_enabled = 1;

	if (PLTID2HC(port) == VHC)
	{
	    /* Virtual port always present */
	    goto exit;
	}

	if ((bcm_otp_is_pcie_port_disabled(port, &otp_value) == 0) && otp_value)
	{
	    is_port_enabled = 0;
	    goto exit;
	}

	/*
	 * The only condition to disable PCIe core based on strap
	 * if "brcm,strap-pcie-sata" exist in current core device
	 * tree node and strap is set to 0
	 */
	if (bcm_strap_parse_and_test(np, "brcm,strap-pcie-sata") == 0)
	{
	     is_port_enabled = 0;
	}

exit:
	return is_port_enabled;
}
#endif /* !USE_PMC_BRD_STUBS */

static inline struct pcie_hcd_cpci* pcie_hcd_cpci_get(void) {
	return &pcie_hcd_cpci_cb;
}

/*
 * Deferred root bus scanning
 *
 * Used for changing the Linux enumeration order of connected end point devices
 *
 * During core probe, each PCIe core gets powered up, initialized, linked up
 * along with root bus creation. The remaining scan for endpoint devices
 * can be deferred based on the user specified order.
 *
 * - Deferred scanning is only applicable for probing during PCIe driver loading
 * probes as a result of (unbind/bind) will not use this mechanism
 *
 * - No sanity check done for user scan order. If core specified in scan_order
 * is not probed by device tree (not present or disabled), then all subsequent
 * core's buses will not be scanned
 *
 * - All cores that were not part of scan order will be scanned at the end of
 * user specified scan order
 *
 */
static inline int pcie_hcd_deferred_scan_active(struct platform_device *pdev) {
	return pcie_scan_order;
}

/*
 * Add a PCIe core to the deferred scan list
 */
static inline int pcie_hcd_deferred_scan_add(struct platform_device *pdev) {
	struct pcie_hcd_bus_dsc *dsc = &(pcie_hcd_cpci_get()->dsc);

	if (!pcie_hcd_deferred_scan_active(pdev))
	    return 0;

	/* Initialize scan list for this core */
	dsc->probe_pdev[pdev->id] = pdev;
	dsc->probe_status[pdev->id] = 0;
	dsc->probe_count++;
	HCD_INFO("Core [%d] added to DSC list, count %d\n", pdev->id,
	    dsc->probe_count);

	return 0;
}

/*
 * Update PCIe core information (probe status) in the deferred scan list
 */
static inline int pcie_hcd_deferred_scan_upd(struct platform_device *pdev,
	int probe_status) {
	struct pcie_hcd_bus_dsc *dsc = &(pcie_hcd_cpci_get()->dsc);

	if (!pcie_hcd_deferred_scan_active(pdev))
	    return 0;

	/* Initialize scan list for this core */
	dsc->probe_pdev[pdev->id] = pdev;
	dsc->probe_status[pdev->id] = probe_status;
	HCD_INFO("Core [%d] updated status in  DSC list, status %d\n", pdev->id,
	    probe_status);

	return 0;
}

/*
 * Delete a PCIe core from the deferred scan list
 */
static inline int pcie_hcd_deferred_scan_del(struct platform_device *pdev) {
	struct pcie_hcd_bus_dsc *dsc = &(pcie_hcd_cpci_get()->dsc);

	if (!dsc->probe_count)
	    return 0;

	/* Initialize scan list for this core */
	dsc->probe_pdev[pdev->id] = NULL;
	dsc->probe_status[pdev->id] = 0;
	dsc->probe_count--;
	HCD_INFO("Core [%d] deleted from DSC list, count %d\n", pdev->id,
	    dsc->probe_count);

	return 0;
}

/*
 * Process deferred root bus scanning
 *
 * Called during each core probing. Starts the root bus scanning for all the
 * available probed cores that meet the user scan order
 */
static inline int pcie_hcd_deferred_scan_trigger(struct platform_device *pdev) {
	struct pcie_hcd_cpci* cpci = pcie_hcd_cpci_get();
	struct pcie_hcd_bus_dsc* dsc = &cpci->dsc;
	int core;
	struct platform_device *dsc_pdev = NULL;
	int dsc_status;

	if (!pcie_hcd_deferred_scan_active(pdev))
	    return 0;

	HCD_INFO("Core [%d] start deferred scan\n", pdev->id);

	/* Scan the root bus based on user specified order */
	while (dsc->probe_count) {
	    core = pcie_scan_order & 0xF;

	    dsc_pdev = dsc->probe_pdev[core];
	    dsc_status = dsc->probe_status[core];

	    if ((!dsc_pdev) || (dsc_status == 0)) {
	        if (dsc->probe_status[pdev->id] == -EHCD_SCAN_DEFER) {
	            HCD_LOG("core [%d] deferring due to scan order [0x%x]\n",
	                pdev->id, pcie_scan_order);
	        }
	        break;
	    }

	    if (dsc_status == -EHCD_SCAN_DEFER) {
	        HCD_LOG("Core [%d] Scanning root bus for devices\n", core);
	        pcie_hcd_scan_bus(platform_get_drvdata(dsc_pdev));
	    }

	    pcie_hcd_deferred_scan_del(dsc_pdev);
	    pcie_scan_order >>= 4;
	};

	/* Scan the root bus if any additional cores probed not in the user order */
	if ((!pcie_scan_order) && dsc->probe_count) {
	    for (core = 0; core < NUM_CORES; core++) {
	        dsc_pdev = dsc->probe_pdev[core];
	        dsc_status = dsc->probe_status[core];

	        if (dsc_pdev) {
	            if (dsc_status == -EHCD_SCAN_DEFER) {
	                HCD_LOG("Core [%d] (!pcie_scan_oder) Scan for devices\n",
	                    core);
	                pcie_hcd_scan_bus(platform_get_drvdata(dsc_pdev));
	            }
	            pcie_hcd_deferred_scan_del(dsc_pdev);
	        }
	    }
	}

	return 0;
}

/*
 * +-----------------------------------------------------
 *  External exported Functions
 * +-----------------------------------------------------
 */
/*
 * Function pcie_hcd_get_rootbus (pdrv)
 *
 *   Parameters:
 *    pdrv ... pointer to pcie core hcd data structure
 *
 *   Description:
 *    Get root bus for the given driver
 *
 *   Return: Pointer to Root bus on success, NULL on failure
 */
struct pci_bus *pcie_hcd_get_rootbus(struct pcie_hcd *pdrv)
{
	return pdrv->bus;
}

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
 *    Query the key value from eNVRAM/kNVRAM and convert to u32 value
 *    value exptected in hex format with a 0xprefix
 *
 *   Return: 0 on success, -ve on failure
 */
int pcie_hcd_nvram_get_u32(char *key, u32 *pval)
{
#if !defined(USE_PMC_BRD_STUBS)
	char valstr[HCD_SETTING_VAL_STR_LEN];
#endif /* !USE_PMC_BRD_STUBS */
	char *pvalstr = NULL;
	u32 val;

	HCD_FN_ENT();

#if !defined(USE_PMC_BRD_STUBS)
	/* uboot environment first */
	if (eNvramGet(key, valstr, HCD_SETTING_VAL_STR_LEN) > 0) {
	    pvalstr = valstr;
	}
#if defined(CONFIG_BCM_NVRAM) || defined(CONFIG_BCM_NVRAM_MODULE)
	else
	{
	    /* Check kernel nvram */
	    pvalstr = nvram_k_get(key);
	}
#endif /* CONFIG_BCM_NVRAM */
#endif /* !USE_PMC_BRD_STUBS */

	if (pvalstr != NULL) {
	    if (sscanf(pvalstr, "0x%x", &val) == 1) {
	        if (pval) *pval = val;
	        HCD_FN_RET_VAL(0);
	    }
	}

	HCD_FN_RET_VAL(-EINVAL);
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
	HCD_FN_ENT();

	/* Read from nvram and update Common settings */
	HCD_READ_CMMN_SETTING(pcie_ssc_cfg);
	HCD_READ_CMMN_SETTING(pcie_boot_order);
	HCD_READ_CMMN_SETTING(pcie_scan_order);
	HCD_READ_CMMN_SETTING(pcie_pllclkadj);
	HCD_READ_CMMN_SETTING(pcie_barmask);
#ifdef HCD_DEBUG
	HCD_READ_CMMN_SETTING(pcie_log_level);
#endif /* HCD_DEBUG */

	/* Read from nvram and update Core shadow settings */
	HCD_READ_CORE_SETTING(pcie_speed_cfg);
	HCD_READ_CORE_SETTING(pcie_apon);
	HCD_READ_CORE_SETTING(pcie_phy_pwrmode);
	HCD_READ_CORE_SETTING(pcie_errlog);
	HCD_READ_CORE_SETTING(pcie_msi);
	HCD_READ_CORE_SETTING(pcie_numlanes);
	HCD_READ_CORE_SETTING(pcie_acc);
	HCD_READ_CORE_SETTING(pcie_linkupretries);
	HCD_READ_CORE_SETTING(pcie_phyrxfilter);
	HCD_READ_CORE_SETTING(pcie_epuperstb);

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
	    HCD_ERROR("Core [%d] Unable to create pcie_hcd proc directory\n",
	        pdrv->core.info.id);
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

#ifdef CONFIG_BCM_PCI_MSI
/*
 * Function pcie_hcd_msi_compose_msi_msg (data, msg)
 *
 *   Parameters:
 *    data ... pointer to msi conrol block
 *    msg  ... pointer to message
 *
 *   Description:
 *    Fill msi message for the given msi interrupt
 *
 *   Return: None
 */
static void pcie_hcd_msi_compose_msi_msg(struct irq_data *data,
	struct msi_msg *msg)
{
	HCD_FN_ENT();

	HCD_INFO("%s - irq_data 0x%px, hwirq 0x%lx\n", __FUNCTION__,
	    data, data->hwirq);

	msg->address_lo = MSI_MATCH_ADDR_MAGIC;
	msg->address_hi = 0;
	msg->data = (MSI_MATCH_DATA_MAGIC | data->hwirq);

	HCD_FN_RET();
}

/*
 * Function pcie_hcd_msi_set_affinity (irq_data, mask, force)
 *
 *   Parameters:
 *    irq_data  ... pointer to msi conrol block
 *    mask      ... affnity mask
 *    force     ... flag to enforce setting
 *
 *   Description:
 *    dummy
 *
 *   Return: Invalid
 */
static int pcie_hcd_msi_set_affinity(struct irq_data *irq_data,
	const struct cpumask *mask, bool force)
{
	HCD_FN_ENT();

	HCD_FN_RET_VAL(-EINVAL);
}

/*
 * Function pcie_hcd_msi_top_irq_ack (data)
 *
 *   Parameters:
 *    data    ... pointer to msi conrol block
 *
 *   Description:
 *    Dummy as acknowledge is done during pcie_hcd_msi_isr itself
 *
 *   Return: None
 */
static void pcie_hcd_msi_top_irq_ack(struct irq_data *data)
{
	HCD_FN_ENT();

	HCD_FN_RET();
}

/*
 * Function pcie_hcd_msi_irq_domain_alloc (domain, virq, nr_irqs, args)
 *
 *   Parameters:
 *    domain  ... pointer to msi irq domain
 *    virq    ... virtual irq number
 *    nr_irqs ... number of irqs
 *    args    ... platform allocation agruments
 *
 *   Description:
 *    Allocate required number of msi interrups from the domain
 *
 *   Return: 0 on success, -ve on failure
 */
static int pcie_hcd_msi_irq_domain_alloc(struct irq_domain *domain,
	unsigned int virq, unsigned int nr_irqs, void *args)
{
	struct pcie_hcd_msi *msi = domain->host_data;
	struct pcie_hcd *pdrv = msi->pdrv;
	int hwirq;
	int i;

	HCD_FN_ENT();

	if (msi->enabled == false) {
	    int err = pcie_hcd_msi_enable(pdrv);
	    if (err < 0) {
	        HCD_ERROR("failed to configure msi 0x%px\n", msi);
	        HCD_FN_RET_VAL(err);
	    }
	}

	mutex_lock(&msi->lock);
	hwirq = bitmap_find_free_region(&msi->used, msi->nr, order_base_2(nr_irqs));
	mutex_unlock(&msi->lock);

	if (hwirq < 0) {
	    HCD_FN_RET_VAL(hwirq);
	}

	HCD_INFO("Created MSI hwirq %d, virq %d, nr_irqs %d\n",
	    hwirq, virq, nr_irqs);

	msi->irq_ops.name = pcie_hcd_msi_name[pdrv->core.info.id];
	msi->irq_ops.irq_compose_msi_msg = pcie_hcd_msi_compose_msi_msg;
	msi->irq_ops.irq_set_affinity = pcie_hcd_msi_set_affinity;

	for (i = 0; i < nr_irqs; i++) {
	    irq_domain_set_info(domain, virq + i, hwirq + i,
	        &msi->irq_ops, domain->host_data,
	        handle_edge_irq, NULL, NULL);
	}

	HCD_FN_RET_VAL(0);
}

/*
 * Function pcie_hcd_msi_irq_domain_free (domain, virq, nr_irqs)
 *
 *   Parameters:
 *    domain  ... pointer to msi irq domain
 *    virq    ... virtual irq number
 *    nr_irqs ... number of irqs
 *
 *   Description:
 *    Free up allocated msi interrupts from the domain
 *
 *   Return: None
 */
static void pcie_hcd_msi_irq_domain_free(struct irq_domain *domain,
	unsigned int virq, unsigned int nr_irqs)
{
	struct irq_data *d = irq_domain_get_irq_data(domain, virq);
	struct pcie_hcd_msi *msi = irq_data_get_irq_chip_data(d);

	HCD_FN_ENT();

	mutex_lock(&msi->lock);
	bitmap_release_region(&msi->used, d->hwirq,  order_base_2(nr_irqs));
	mutex_unlock(&msi->lock);

	HCD_FN_RET();
}

/*
 * Function pcie_hcd_msi_allocate_domains (msi)
 *
 *   Parameters:
 *    msi ... pointer to msi control block
 *
 *   Description:
 *    Allocate required msi domains
 *
 *   Return: 0 on success, -ve on failure
 */
static int pcie_hcd_msi_allocate_domains(struct pcie_hcd_msi *msi)
{
	struct pcie_hcd *pdrv = msi->pdrv;
	struct fwnode_handle *fwnode = dev_fwnode(&pdrv->core.pdev->dev);

	HCD_FN_ENT();

	msi->domain_ops.alloc = pcie_hcd_msi_irq_domain_alloc;
	msi->domain_ops.free  = pcie_hcd_msi_irq_domain_free;

	msi->top_domain = irq_domain_create_linear(fwnode, msi->nr,
	    &msi->domain_ops, msi);
	if (!msi->top_domain) {
	    HCD_ERROR("Core [%d] failed to create IRQ domain\n",
	        pdrv->core.info.id);
	    HCD_FN_RET_VAL(-ENOMEM);
	}

	HCD_INFO("craated top_domain 0x%px\n", msi->top_domain);

	msi->top_irq_ops.name = pcie_hcd_msi_name[pdrv->core.info.id];
	msi->top_irq_ops.irq_ack = pcie_hcd_msi_top_irq_ack;
	msi->domain_info.flags = MSI_FLAG_USE_DEF_DOM_OPS;
	msi->domain_info.flags |= MSI_FLAG_USE_DEF_CHIP_OPS;
	msi->domain_info.flags |= MSI_FLAG_MULTI_PCI_MSI;
	msi->domain_info.chip  = &msi->top_irq_ops;

	msi->domain = pci_msi_create_irq_domain(fwnode, &msi->domain_info,
	    msi->top_domain);
	if (!msi->domain) {
	    HCD_ERROR("Core [%d] failed to create MSI domain\n",
	        pdrv->core.info.id);
	    irq_domain_remove(msi->top_domain);
	    HCD_FN_RET_VAL(-ENOMEM);
	}

	HCD_INFO("craated domain 0x%px\n", msi->domain);

	HCD_FN_RET_VAL(0);
}

/*
 * Function pcie_hcd_msi_free_domains (msi)
 *
 *   Parameters:
 *    msi ... pointer to msi control block
 *
 *   Description:
 *    Free up allocated msi domains
 *
 *   Return: None
 */
static void pcie_hcd_msi_free_domains(struct pcie_hcd_msi *msi)
{
	HCD_FN_ENT();

	irq_domain_remove(msi->domain);
	irq_domain_remove(msi->top_domain);

	HCD_FN_RET();
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
	unsigned long status;
	u32 bit;
	int ret;


	HCD_FN_ENT();

	status = pdrv->core.plt_cfg->get_msi(&pdrv->core);

	/* Process all the available MSI interrupts */
	i = 0;

	for_each_set_bit(bit, &status, msi->nr) {
	    ret = -EINVAL;
	    if (test_bit(bit, &msi->used)) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
	        ret = generic_handle_domain_irq(msi->top_domain, bit);
#else
	        irq = irq_find_mapping(msi->top_domain, bit);
	        if (irq) {
	            ret = generic_handle_irq(irq);
	        } else {
	            /* that's weird who triggered this? */
	            /* just clear it */
	            HCD_INFO("Core [%d] Un handled MSI %d\n", pdrv->core.info.id,
	                bit);
	        }
#endif
	    }

	    if (ret)
	        HCD_ERROR("Core [%d] unexpected MSI %d\n", pdrv->core.info.id, bit);
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

	HCD_FN_ENT();

	/* Make sure msi structure was allocated earlier */
	if (!msi)
	    HCD_FN_RET_VAL(-ENODEV);

	/* Already enabled? */
	if (msi->enabled == true)
	    HCD_FN_RET_VAL(err);

	err = request_irq(msi->irq, pcie_hcd_msi_isr, IRQF_SHARED,
	    msi->top_irq_ops.name, msi);
	if (err < 0) {
	    HCD_ERROR("failed to request IRQ[%d]: %d\n", pdrv->core.res.irq, err);
	    HCD_FN_RET_VAL(err);
	}

	HCD_INFO("Using irq=%d for PCIE-MSI interrupts\r\n", msi->irq);

	err = pdrv->core.plt_cfg->setup_msi(&pdrv->core);
	if (err == 0) {
	    /* Set the flag to specify MSI is enabled */
	    msi->enabled = true;

	    HCD_INFO("MSI Enabled\n");
	} else {
	    free_irq(msi->irq, msi);
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

	HCD_INFO("Allocated msi [0x%px] \r\n", msi);

	mutex_init(&msi->lock);
	snprintf(pcie_hcd_msi_name[pdrv->core.info.id], MSI_ISR_NAME_STR_LEN,
	    "msi_pcie:%d", pdrv->core.info.id);

	/* Initialize msi structure elements */
	pdrv->msi = msi;
	msi->pdrv = pdrv;

	if (HCD_USE_DT_ENTRY(pdrv->core.info.id) && (pdrv->core.res.irq == 0)) {
	    err = platform_get_irq_byname(pdev, "msi");
	    if (err > 0) {
	        pdrv->core.res.irq = err;
	    } else {
	        HCD_ERROR("failed to get msi intr from DT: %d\n", err);
	        goto err;
	    }
	}

	msi->irq = pdrv->core.res.irq;
	/*
	 * MSI should have been updated by hc with the number of interrupts
	 * supported
	 */
	msi->nr = pdrv->core.cfg.msi_map_size;

	err = pcie_hcd_msi_allocate_domains(msi);

	if (err < 0) {
	    HCD_ERROR("failed to create IRQ domain\n");
	    HCD_FN_RET_VAL(err);
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

	HCD_FN_ENT();

	if (!pdrv) {
	    HCD_FN_RET_VAL(0);
	}

	msi = pdrv->msi;
	if (msi) {
	    if (pdrv->core.plt_cfg->teardown_msi)
	        pdrv->core.plt_cfg->teardown_msi(&pdrv->core);

	    if (msi->enabled == true) {
	        free_irq(msi->irq, msi);
	    }

	    pcie_hcd_msi_free_domains(msi);

	    /* Set the flag to specify MSI is disabled */
	    msi->enabled = false;

	    HCD_INFO("MSI Disabled\n");

	    pdrv->msi = NULL;
	    kfree(msi);
	}

	HCD_FN_RET_VAL(0);
}
#endif /* CONFIG_BCM_PCI_MSI */

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

	if ((pdrv->errlog.inited == true) ||
	    (pdrv->core.plt_cfg->setup_errint == NULL)) {
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

	if ((pdrv->errlog.inited == false) ||
	    (pdrv->core.plt_cfg->teardown_errint == NULL)) {
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
 *   Return: 0 on linkup, -ve on linkdown/failure
 */
static int pcie_hc_link_up(struct pcie_hcd *pdrv)
{
	struct pcie_hc_core    *phc = NULL;
	int err = -EAGAIN;
	int retries;

	HCD_FN_ENT();

	phc = &pdrv->core;

	if (!phc->plt_cfg->is_linkup) {
	    err = 0;
	    HCD_FN_RET_VAL(err);
	}

	/* Check if PCIe link is up (for any device connected on the link) */
	retries = phc->cfg.linkupretries;
	do {
	    err = phc->plt_cfg->is_linkup(&pdrv->core);

	    if ((err == -EAGAIN) && retries) {
	        /* Restart Core */
	        HCD_INFO("Link up failed, retry %d/%d\n",
	            phc->cfg.linkupretries - retries+1,
	            phc->cfg.linkupretries);
	        pcie_hc_restart_core(pdrv);
	    }
	} while ((err == -EAGAIN) && retries--);

	/* Check if we can do link train successfully with 1 Lane */
	if ((phc->cfg.linkupretries > 0) && (err == -EAGAIN) &&
	    (phc->res.link_width > PCIE_LINK_WIDTH_1LANE)) {
	    phc->res.link_width = PCIE_LINK_WIDTH_1LANE;
	    HCD_INFO("Link training retry in Single lane mode\n");

	    /* Restart Core in single lane */
	    pcie_hc_restart_core(pdrv);

	    err = phc->plt_cfg->is_linkup(&pdrv->core);
	}

	if (err == 0) {
	    u32 link_status;

	    link_status = phc->plt_cfg->read_reg(&pdrv->core, PCIE_LNKCTLSTA);
	    link_status >>= 16;

	    HCD_LOG("Core [%d] Link UP - [%d] lanes, [GEN%d] speed\r\n",
	        pdrv->core.info.id, PCIE_LNKSTA_LNK_WIDTH(link_status),
	        PCIE_LNKSTA_LNK_SPEED(link_status));
	} else {
	    err = -ENODEV;
	}

	HCD_FN_RET_VAL(err);
}

/*
 * Function pcie_hc_unconfig_core (pdrv)
 *
 *   Parameters:
 *    pdrv ... pointer to pcie core hcd data structure
 *
 *   Description:
 *    De-configure the HC core
 *
 *   Return: 0 on success, -ve on failure
 */
static void pcie_hc_unconfig_core(struct pcie_hcd *pdrv)
{
	HCD_FN_ENT();

	if (pdrv->core.plt_cfg->unconfig_core) {
	    pdrv->core.plt_cfg->unconfig_core(&pdrv->core);
	}

	HCD_FN_RET();
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
 * Function pcie_hc_restart_core (pdrv)
 *
 *   Parameters:
 *    pdrv ... pointer to pcie core hcd data structure
 *
 *   Description:
 *    Restart PCIe core (power down, power up, reset)
 *    Do not check apon setting
 *
 *   Return: 0 on success, -ve on failure
 */
static int pcie_hc_restart_core(struct pcie_hcd *pdrv)
{
	int devidx;
	struct pcie_hc_core    *phc = &pdrv->core;

	HCD_FN_ENT();

	HCD_INFO("Restarting Core %d\n", phc->info.id);

	for (devidx = 0; devidx < phc->info.devs; devidx++) {
	    /* Power Down */
	    phc->plt_cfg->set_power(phc, devidx, false, HCD_DEV_BOTH);

	    /* Power Up */
	    phc->plt_cfg->set_power(phc, devidx, true, HCD_DEV_BOTH);
	}

	/* Reset */
	pcie_hc_reset_core(pdrv);

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

	/* Initialize misc pcie control base if present */
	phc->info.misc = pcie_hcd_cpci_get()->misc.base;

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

	/* Update configurtion with default parameters */
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
	phc->cfg.linkupretries = HCD_HC_CORE_CFG(pcie_linkupretries, phc->info.id);
	phc->cfg.phyrxfilter = HCD_HC_CORE_CFG(pcie_phyrxfilter, phc->info.id);
	phc->cfg.epuperstb = HCD_HC_CORE_CFG(pcie_epuperstb, phc->info.id);

	pcie_hc_dump_core_cfg(pdrv);

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
 * Function pcie_hc_update_core_cfg (pdrv)
 *
 *   Parameters:
 *    pdrv ... pointer to pcie core hcd data structure
 *
 *   Description:
 *    update core specific settings from valid nvram settings
 *
 *   Return: 0 on success, -ve on failure
 */
static int pcie_hc_update_core_cfg(struct pcie_hcd *pdrv)
{
	struct pcie_hc_core *phc = &pdrv->core;
	struct pcie_hc_core_res *pres = &phc->res;
	u8 core = phc->info.id;

	/* Update core configurtion from common nv parameters */
	phc->cfg.ssc = HCD_GET_CMMN_SETTING(pcie_ssc_cfg, core) ? true : false;
	phc->cfg.pllid = HCD_GET_CMMN_SETTING(pcie_pllclkid, core);
	phc->cfg.plladj = HCD_GET_CMMN_SETTING(pcie_pllclkadj, core);
	phc->cfg.barmask = pcie_barmask;

	/* Get updated nvram params after module load */
	HCD_READ_CORE_SETTING(pcie_speed_cfg);
	HCD_READ_CORE_SETTING(pcie_apon);
	HCD_READ_CORE_SETTING(pcie_phy_pwrmode);
	HCD_READ_CORE_SETTING(pcie_errlog);
	HCD_READ_CORE_SETTING(pcie_msi);
	HCD_READ_CORE_SETTING(pcie_numlanes);
	HCD_READ_CORE_SETTING(pcie_acc);
	HCD_READ_CORE_SETTING(pcie_linkupretries);
	HCD_READ_CORE_SETTING(pcie_phyrxfilter);
	HCD_READ_CORE_SETTING(pcie_epuperstb);

	/* Update core configurtion from core nv parameters */
	phc->cfg.speed = HCD_GET_CORE_SETTING(pcie_speed_cfg, core);
	phc->cfg.apon = HCD_GET_CORE_SETTING(pcie_apon, core);
	/*
	 * Backward compatibility for Virtual core APON setting
	 * if new setting is empty and old setting is non-empty
	 * use old setting
	 */
	if ((PLTID2HC(core) == VHC) && (PLTID2CORE(core) == 1)) {
	    u8 bwc_apon = (HCD_GET_CORE_SETTING(pcie_apon, core-1)) >> 2;
	    if ((phc->cfg.apon == 0) && (bwc_apon != 0)) {
	        HCD_LOG("Core [%d] Using Old format apon setting\n", core);
	        phc->cfg.apon = bwc_apon;
	    }
	}
	phc->cfg.phypwrmode = HCD_GET_CORE_SETTING(pcie_phy_pwrmode, core);
	phc->cfg.errlog = HCD_GET_CORE_SETTING(pcie_errlog, core);
	phc->cfg.msi = HCD_GET_CORE_SETTING(pcie_msi, core);
	pres->link_width = HCD_GET_CORE_SETTING(pcie_numlanes, core);
	phc->cfg.acc = HCD_GET_CORE_SETTING(pcie_acc, core);
	phc->cfg.linkupretries = HCD_GET_CORE_SETTING(pcie_linkupretries,
	    phc->info.id);
	phc->cfg.phyrxfilter = HCD_GET_CORE_SETTING(pcie_phyrxfilter,
	    phc->info.id);
	phc->cfg.epuperstb = HCD_GET_CORE_SETTING(pcie_epuperstb,
	    phc->info.id);

	pcie_hc_dump_core_cfg(pdrv);

	return 0;
}

/*
 * Function pcie_hc_dump_core_cfg (pdrv)
 *
 *   Parameters:
 *    pdrv ... pointer to pcie core hcd data structure
 *
 *   Description:
 *    dump core specific configuration settings
 *
 *   Return: 0 on success, -ve on failure
 */
static void pcie_hc_dump_core_cfg(struct pcie_hcd *pdrv)
{
	struct pcie_hc_core *phc = &pdrv->core;
	u8 core = phc->info.id;

	HCD_INFO("Core [%d] configuration dump:\n", core);

	HCD_INFO("    ssc [%d] pllid [%d] plladj [%d]\n",
	    phc->cfg.ssc, phc->cfg.pllid, phc->cfg.plladj);

	HCD_INFO("    speed [%d] apon [%d] phypwrmode [%d] errlog [%d] msi [%d] barmask [%d]\n",
	    phc->cfg.speed, phc->cfg.apon, phc->cfg.phypwrmode,
	    phc->cfg.errlog, phc->cfg.msi, phc->cfg.barmask);

	HCD_INFO("    lanes [%d] linkupretries [%d] phyrxfilter [%d] "
	    "epuperstb [%d]\n",
	    phc->res.link_width, phc->cfg.linkupretries, phc->cfg.phyrxfilter,
	    phc->cfg.epuperstb);

	return;
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
	        int rc;

	        rc = pdrv->core.plt_cfg->set_power(phc, devidx, true, HCD_DEV_BOTH);
	        if (rc == 0) {
	            pdrv->apon.powered[devidx] = true;
	            HCD_INFO("Core [%d] dev [%d] powered up\r\n",
	                pdrv->core.info.id, devidx);
	            devs++;
	        } else {
	            pdrv->apon.powered[devidx] = false;
	            HCD_ERROR("Core [%d] dev [%d] failed to powered up\r\n",
	                pdrv->core.info.id, devidx);
	            devs = -ENODEV;
	        }
	    } else {
	        /* Always off (with/without domain) */
	        pdrv->core.plt_cfg->set_power(phc, devidx, false, HCD_DEV_BOTH);
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
	        phc->plt_cfg->set_power(phc, devidx, false, HCD_DEV_BOTH);
	        pdrv->apon.powered[devidx] = false;
	        HCD_LOG("Core [%d] dev [%d] powered down\r\n", phc->info.id, devidx);
	    }
	}

	HCD_FN_RET_VAL(0);
}

/*
 * Function pcie_hcd_set_regulators(pdrv, devidx, up)
 *
 *   Parameters:
 *    pdrv ... pointer to pcie core hcd data structure
 *    devidx ... core device index (not used)
 *    up     ... flag to sepcify power up/down
 *
 *   Description:
 *     power up/down the core, devidx ignored
 *
 *   Return: 0 success, -ve on failure
 */
int pcie_hcd_set_regulators(struct pcie_hcd *pdrv, unsigned int devidx, bool up)
{
	int ret = 0;
	struct pcie_hcd_pwrsup *supply;
	struct list_head *pos;

	HCD_FN_ENT();

	/* Go through all power supplies and perform the up/down operation */
	list_for_each(pos, &pdrv->pwrsupplies.node) {
	    supply = list_entry(pos, struct pcie_hcd_pwrsup, node);

	    if (up) {
	        if (supply->usage == 0) {
	            ret = regulator_enable(supply->regulator);
	            HCD_LOG("Core [%d] %senable %s regulator.\n",
	                pdrv->core.info.id, ret ? "failed to " : "", supply->name);
	            if (ret == 0) {
	                supply->usage++;
	                /* Make sure enough delay before asserting pcie reset */
	                mdelay(PCIE_PWR_SETTLING_DELAY);
	                mdelay(PCIE_PWR_2_PERST_DELAY);
	            }
	        }
	    } else if (supply->usage) {
	        ret = regulator_disable(supply->regulator);
	        HCD_LOG("Core [%d] %sdisable %s regulator.\n", pdrv->core.info.id,
	            ret ? "failed to " : "", supply->name);
	        if (ret == 0)
	            supply->usage--;
	    }
	}

	HCD_FN_RET_VAL(ret);
}

/*
 * Function pcie_hcd_init_regulators(pdrv)
 *
 *   Parameters:
 *    pdrv ... pointer to pcie core hcd data structure
 *
 *   Description:
 *     Parse device tree for power supplies and initialize corresponding
 *     regulators for use later
 *
 *   Return: 0 success, -ENOMEM on failure
 */
int pcie_hcd_init_regulators(struct pcie_hcd *pdrv)
{
	struct device_node *np = NULL;
	struct device *dev = NULL;

	int supplies;
	int i;
	struct pcie_hcd_pwrsup *supply;
	const char *name;

	HCD_FN_ENT();

	dev = &pdrv->core.pdev->dev;
	np = dev->of_node;

	INIT_LIST_HEAD(&pdrv->pwrsupplies.node);
	supplies = of_property_count_strings(np, "brcm,supply-names");
	if (supplies <= 0) {
	    supplies = 0;
	}
	HCD_LOG("Core [%d] has %d regulators\n", pdrv->core.info.id, supplies);

	for (i = 0; i < supplies; i++) {
	    if (of_property_read_string_index(np, "brcm,supply-names", i, &name)) {
	        continue;
	    }

	    HCD_INFO("Core [%d] regulator [%d] %s\n", pdrv->core.info.id, i, name);
	    supply = devm_kzalloc(dev, sizeof(*supply), GFP_KERNEL);
	    if (!supply) {
	        HCD_FN_RET_VAL(-ENOMEM);
	    }

	    strncpy(supply->name, name, sizeof(supply->name));
	    supply->name[sizeof(supply->name) - 1] = '\0';

	    supply->regulator = devm_regulator_get(dev, name);
	    if (IS_ERR(supply->regulator)) {
	        HCD_ERROR("Unable to get %s supply, err=%d\n", name,
	            (int)PTR_ERR(supply->regulator));
	        continue;
	    }
	    list_add_tail(&supply->node, &pdrv->pwrsupplies.node);
	}

	HCD_FN_RET_VAL(0);
}

/*
 * Function pcie_hcd_free_regulators(pdrv)
 *
 *   Parameters:
 *    pdrv ... pointer to pcie core hcd data structure
 *
 *   Description:
 *     freeup initialized regulators if any
 *
 *   Return: 0 success, -ve on failure
 */
void pcie_hcd_free_regulators(struct pcie_hcd *pdrv)
{
	HCD_FN_ENT();

	/* Place holder, nothing to do as devm_xxx memory is automatically freed */

	HCD_FN_RET();
}

/*
 * Function pcie_hcd_set_power(phc, devidx, up, type)
 *
 *   Parameters:
 *    phc    ... pointer to core control block
 *    devidx ... core device index (not used)
 *    up     ... flag to sepcify power up/down
 *    type   ... mask to specify EP and/or RC device type
 *
 *   Description:
 *     power up/down the core, devidx ignored
 *
 *   Return: 0 success, -ve on failure
 */
int pcie_hcd_set_power(struct pcie_hc_core *phc, unsigned int devidx, bool up,
	int type)
{
	int ret = 0;

	HCD_FN_ENT();

	if (type & HCD_DEV_RC) {
	    if (up == true) {
	        ret = pcie_hcd_pmc_power_up(phc->info.id, phc->cfg.is_dual_lane);
	    } else {
	        ret = pcie_hcd_pmc_power_down(phc->info.id, phc->cfg.is_dual_lane);
	    }
	}

	if ((!ret) && (type & HCD_DEV_EP))
	    ret = pcie_hcd_set_regulators(phc->pdrv, devidx, up);

	HCD_FN_RET_VAL(ret);
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
	u32 dt_val;

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

	        if (range.size & OWIN_REMAP_NEEDED) {
	            pdrv->owin[win].need_remap = 1;
	            range.size &= (~OWIN_REMAP_NEEDED);
	            HCD_INFO("Core [%d] outgoing window [%d] need address remap\n",
	                pdev->id, win);
	        }

	        pres->owin[win].flags = range.flags;
	        pres->owin[win].parent = NULL;
	        pres->owin[win].child = pres->owin[win].sibling = NULL;
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
	    err = of_property_read_u32(np, "brcm,num-lanes", &dt_val);
	    HCD_WARN_ON_DT_ERROR("brcm,num-lanes", err);
	    if (err == 0) {
	        if (dt_val > PCIE_LINK_WIDTH_MAX)
	            dt_val = PCIE_LINK_WIDTH_MAX;
	        HCD_HC_SET_CORE_CFG(pcie_numlanes, pdev->id, dt_val);
	    }

	    /* PCIe port speed */
	    err = of_property_read_u32(np, "brcm,speed", &dt_val);
	    if (err == 0) {
	        HCD_HC_SET_CORE_CFG(pcie_speed_cfg, pdev->id, dt_val);
	    }
	    HCD_WARN_ON_DT_ERROR("brcm,speed", err);

	    /* PCIe port power mode */
	    err = of_property_read_u32(np, "brcm,phypwrmode", &dt_val);
	    if (err == 0) {
	        HCD_HC_SET_CORE_CFG(pcie_phy_pwrmode, pdev->id, dt_val);
	    }
	    HCD_WARN_ON_DT_ERROR("brcm,phypwrmode", err);

	    if (of_property_read_bool(np, "brcm,dual-lane"))
	        pdrv->core.cfg.is_dual_lane = 1;
	    else
	        pdrv->core.cfg.is_dual_lane = 0;

	    /* PCIe port error logging */
	    err = of_property_read_u32(np, "brcm,errlog", &dt_val);
	    if (err == 0) {
	        HCD_HC_SET_CORE_CFG(pcie_errlog, pdev->id, dt_val);
	    }
	    HCD_WARN_ON_DT_ERROR("brcm,errlog", err);

	    /* PCIe port Link Training retries */
	    err = of_property_read_u32(np, "brcm,linkupretries", &dt_val);
	    HCD_WARN_ON_DT_ERROR("brcm,linkupretries", err);
	    if (err == 0) {
	        if (dt_val > PCIE_LINKUPRETREIES_MAX)
	            dt_val = PCIE_LINKUPRETREIES_MAX;
	        HCD_HC_SET_CORE_CFG(pcie_linkupretries, pdev->id, dt_val);
	    }

	    /* PCIe force power on setting */
	    err = of_property_read_u32(np, "brcm,apon", &dt_val);
	    if (err == 0) {
	        HCD_HC_SET_CORE_CFG(pcie_apon, pdev->id, dt_val);
	    }
	    HCD_WARN_ON_DT_ERROR("brcm,apon", err);

	    /* PCIe acc configuration */
	    err = of_property_read_u32(np, "brcm,acc", &dt_val);
	    if (err == 0) {
	        HCD_HC_SET_CORE_CFG(pcie_acc, pdev->id, dt_val);
	    }
	    HCD_WARN_ON_DT_ERROR("brcm,acc", err);

	    /* PCIe PHY RX Filter */
	    err = of_property_read_u32(np, "brcm,phyrxfilter", &dt_val);
	    if (err == 0) {
	        HCD_HC_SET_CORE_CFG(pcie_phyrxfilter, pdev->id, dt_val);
	    }
	    HCD_WARN_ON_DT_ERROR("brcm,phyrxfilter", err);

	    /* PCIe Regulator Power Up Setting */
	    err = of_property_read_u32(np, "brcm,epuperstb", &dt_val);
	    if (err == 0) {
	        HCD_HC_SET_CORE_CFG(pcie_epuperstb, pdev->id, dt_val);
	    }
	    HCD_WARN_ON_DT_ERROR("brcm,epuperstb", err);

	} else {
	    for (win = OWIN0; win < MAX_NUM_OUTGOING_WINDOWS; win++) {
	        pdrv->owin[win].pci_addr = pres->owin[win].start;
	    }
	}

	/* Parse external power supplies to be used with regulators */
	err = pcie_hcd_init_regulators(pdrv);
	if (err != 0) {
	    HCD_ERROR("Core [%d] init_regulators returned error %d\n", pdev->id,
	        err);
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

	pcie_hc_dump_core_cfg(pdrv);

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
	struct pcie_hcd_cpci *cpci = pcie_hcd_cpci_get();

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

	memset(cpci, 0, sizeof(struct pcie_hcd_cpci));

	HCD_INFO("common core settings\r\n");

	/* MISC PCIe registers base */
	err = of_address_to_resource(np, 0, &cpci->misc.res);
	if ((cpci->misc.res.start != 0) && (cpci->misc.base == NULL)) {
	    cpci->misc.base = devm_ioremap_resource(&pdev->dev, &cpci->misc.res);
	    if (IS_ERR(cpci->misc.base)) {
	        HCD_ERROR("cpci MISC reg base mapping fail: [%ld]\r\n",
	            PTR_ERR(cpci->misc.base));
	        HCD_FN_RET_VAL(PTR_ERR(cpci->misc.base));
	    }
	}
	HCD_INFO("Mapped misc reg base [0x%px]\r\n", cpci->misc.base);

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

	/* PCIe BAR configuration for DDR memory */
	err = of_property_read_u32(np, "brcm,barmask", &dt_val);
	if (err == 0) {
	    pcie_barmask = dt_val;
	}
	HCD_WARN_ON_DT_ERROR("brcm,barmask", err);

	/* PCIe boot order configuration */
	err = of_property_read_u32(np, "brcm,scan_order", &dt_val);
	if (err == 0) {
	    pcie_scan_order = dt_val;
	}
	HCD_WARN_ON_DT_ERROR("brcm,scan_order", err);

	/* Update settings from NVRAM (kernel, environment) if valid */
	HCD_UPDATE_CMMN_SETTING(pcie_pllclkid);
	HCD_UPDATE_CMMN_SETTING(pcie_ssc_cfg);
	HCD_UPDATE_CMMN_SETTING(pcie_pllclkadj);
	HCD_UPDATE_CMMN_SETTING(pcie_barmask);
	HCD_UPDATE_CMMN_SETTING(pcie_scan_order);

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
	struct pcie_hcd_cpci *cpci = pcie_hcd_cpci_get();
	HCD_FN_ENT();

	if ((cpci->misc.res.start != 0) && (cpci->misc.base != NULL)) {
	    devm_iounmap(&pdev->dev, cpci->misc.base);
	}

	HCD_FN_RET_VAL(0);
}

/*
 * Function pcie_hcd_scan_bus (pdrv)
 *
 *   Parameters:
 *    pdrv ... pointer to pcie driver hcd data structure
 *
 *   Description:
 *    PCIe hcd driver bus scan. Called for each instance of the PCIe core.
 *    start the PCI bus and enumerate PCI devices
 *
 *   Return: 0 on success, -ve on failure
 */
static int pcie_hcd_scan_bus(struct pcie_hcd *pdrv)
{
	struct pci_bus *bus = pdrv->bus;
	struct pci_host_bridge *bridge = to_pci_host_bridge(bus->bridge);
	struct pci_bus *child;

	bridge->map_irq = pcie_hcd_map_irq;
	bridge->swizzle_irq = pci_common_swizzle;

	pci_scan_child_bus(bus);

	pci_bus_size_bridges(bus);
	pci_bus_assign_resources(bus);

	/* Configure PCI Express settings */
	list_for_each_entry(child, &bus->children, node)
	    pcie_bus_configure_settings(child);

	pci_bus_add_devices(bus);

	return 0;
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
	int err = 0;
	struct pci_bus *bus;
	struct pci_ops *pci_fops = NULL;
	int active;
	LIST_HEAD(res);

	HCD_FN_ENT();

	if (np && pcie_hcd_cpci_probe(pdev) == 0) {
	    HCD_FN_RET_VAL(0);
	}

	/* If coming from device tree, use device tree entry to find the core id */
	if (np && (pdev->id <= PLATFORM_DEVID_NONE)) {
	    const char *type = NULL;

	    if (of_property_read_u32(np, "brcm,coreid", &core) < 0) {
	        HCD_ERROR("Unable to get coreid from device tree\r\n");
	        HCD_FN_RET_VAL(-ENODEV);
	    }

	    if (of_property_read_string(np, "device_type", &type) == 0) {
	        if (type && (strcmp(type, "vpci") == 0)) {
	            /* Virtual PCIe device, Adjust the core */
	            core += NUM_PCORES;
	        }
	    }
	    pdev->id = core;
	}

	HCD_INFO("Core [%d] probe\r\n", core);

	if (core >= NUM_CORES) {
	    HCD_ERROR("Core [%d] exceeds max Cores [%d]\r\n", core, NUM_CORES);
	    HCD_FN_RET_VAL(-ENODEV);
	}

	if (pcie_hcd_deferred_scan_active(pdev)) {
	    /* Add this device to the probed device list */
	    pcie_hcd_deferred_scan_add(pdev);
	}

	if (!pcie_hcd_port_enabled(core, np))
	{
	    HCD_ERROR("Core [%d] not enabled\r\n", core);
	    err = -ENODEV;
	    goto error;
	}

	if (pcie_hcd_get_port_mode(core, np) == 0)
	{
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
	INIT_LIST_HEAD(&pdrv->pwrsupplies.node);

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

	/* Update core setting from valid nvram settings */
	err = pcie_hc_update_core_cfg(pdrv);
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
	    HCD_LOG("Core [%d] skip due to apon setting [%d]\r\n", core,
	        pdrv->core.cfg.apon);
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
	    HCD_LOG("Core [%d] skip due to apon setting [%d]\r\n", core,
	        pdrv->core.cfg.apon);
	    goto scan_trigger;
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
	        HCD_ERROR("Core [%d] failed to setup error logging, err [%d]\r\n",
	            core, err);
	        err =  -ENODEV;
	        goto error;
	    }
	}

	if (IS_ENABLED(CONFIG_BCM_PCI_MSI) && pdrv->core.cfg.msi) {
	    err = pcie_hcd_msi_setup(pdrv);
	    if (err < 0) {
	        HCD_ERROR("failed to enable MSI support: %d\n", err);
	        goto error;
	    }
	}

	pcie_hcd_procfs_init(pdrv);

	err = 0;

error:

	if (err && pdrv) {
	    pcie_hcd_remove(pdev);
	}

scan_trigger:
	{
	    bool scan = ((err == 0) && (active)) ? true : false;

	    if (pcie_hcd_deferred_scan_active(pdev)) {
	        /* trigger deferred scan if enabled */
	        pcie_hcd_deferred_scan_upd(pdev,
	            (scan == true) ? -EHCD_SCAN_DEFER: -ENODEV);
	        pcie_hcd_deferred_scan_trigger(pdev);
	    } else if (scan == true) {
	        pcie_hcd_scan_bus(pdrv);
	    }
	}

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

	HCD_FN_ENT();

	if (!pdrv) {
	    pcie_hcd_cpci_remove(pdev);
	    HCD_FN_RET_VAL(0);
	}

	if (pdrv->bus) {
	    pci_stop_root_bus(pdrv->bus);
	    pci_remove_root_bus(pdrv->bus);
	}

	pcie_hcd_procfs_deinit(pdrv);

	if (IS_ENABLED(CONFIG_BCM_PCI_MSI)) {
	    pcie_hcd_msi_disable(pdrv);
	}

	pcie_hcd_errlog_disable(pdrv);

	pcie_hc_unconfig_core(pdrv);

	pcie_hcd_unmap_resources(pdrv);

	/* power off ports based on apon setting */
	pcie_hc_apon_pwrdn(pdrv);

	pcie_hcd_free_regulators(pdrv);

	pcie_hc_free_core(pdrv);

	kfree(pdrv);

	platform_set_drvdata(pdev, NULL);

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
	struct platform_device  *pdev = NULL;

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

	    HCD_LOG("DEF CFG: ssc 0x%x, speed  0x%x, scanorder 0x%x, apon 0x%x barmask 0x%x\n",
	        pcie_ssc_cfg, pcie_speed_cfg, pcie_scan_order, pcie_apon,
	            pcie_barmask);
	    HCD_LOG("       : pwr 0x%x, errlog 0x%x, msi 0x%x, acc  0x%x, pllclkid 0x%x\n",
	        pcie_phy_pwrmode, pcie_errlog, pcie_msi, pcie_acc, pcie_pllclkid);
	    HCD_LOG("       : loglevel 0x%x pllclkadj 0x%x linkupretries 0x%x "
	        "phyrxfilter 0x%x epuperstb 0x%x\n",
	        pcie_log_level, pcie_pllclkadj, pcie_linkupretries,
	        pcie_phyrxfilter, pcie_epuperstb);
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
	struct platform_device  *pdev = NULL;

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
