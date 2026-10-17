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
#include <linux/arm-smccc.h>
#include <linux/pci-ecam.h>

/* brcm bsp */
#if defined(CONFIG_BRCM_IKOS) || defined(CONFIG_BCM_PCIE_PMC_BRD_STUBS)
#define USE_PMC_BRD_STUBS
#else /* !CONFIG_BRCM_IKOS && !CONFIG_BCM_PCIE_PMC_BRD_STUBS */
#include <board.h>
#include <pmc_pcie.h>
#include <flash_common.h>
#endif /* !CONFIG_BRCM_IKOS && !CONFIG_BCM_PCIE_PMC_BRD_STUBS */
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

#define PCIE_LNKCAP                         \
	(PCI_EXP_CFG_OFFSET + PCI_EXP_LNKCAP)
#define PCIE_LNKCAP_LNK_WIDTH(cap)          \
	(((cap) & PCI_EXP_LNKCAP_MLW) >> PCI_EXP_LNKSTA_NLW_SHIFT)
#define PCIE_LNKCAP_LNK_SPEED(cap)          ((cap) & PCI_EXP_LNKCAP_SLS)
#define PCIE_LNKCTLSTA                      \
	(PCI_EXP_CFG_OFFSET + PCI_EXP_LNKCTL)
#define PCIE_LNKSTA_LNK_WIDTH(sta)          \
	(((sta) & PCI_EXP_LNKSTA_NLW) >> PCI_EXP_LNKSTA_NLW_SHIFT)
#define PCIE_LNKSTA_LNK_SPEED(sta)          ((sta) & PCI_EXP_LNKSTA_CLS)

/* PCI Power ON to PERST# minimum time */
#define PCIE_PWR_2_PERST_DELAY              100 /* milli seconds */
#define PCIE_PWR_SETTLING_DELAY             25  /* milli seconds */

#define MSI_ISR_NAME_STR_LEN                32

#define APON_F_DEFAULT                      (1 << HCD_APON_DEFAULT)
#define APON_F_ON                           (1 << HCD_APON_ON)
#define APON_F_OFF                          (1 << HCD_APON_OFF)
#define APON_F_OFF_W_DOM                    (1 << HCD_APON_OFF_WITH_DOMAIN)

/* HCD Settings */
#define HCD_SETTING_VAL_INV                 0xFFFFFFFF
#define HCD_SETTING_VAL_STR_LEN             64
#define HCD_CORE_ARRAY_SETTING_VALUES       2

/* Local Error Codes */
#define EHCD_SCAN_DEFER                     EPROBE_DEFER

/* Satellite Timer Section */
/* CTMR_CTRL registers */
#define CTMR_CTRL_GET_H_OFFSET              (0x000C)
#define CTMR_CTRL_STMR_EN_OFFSET            (0x0030)
#define CTMR_CTRL_GET_L0_OFFSET             (0x0080)
#define CTMR_CTRL_GET_L1_OFFSET             (0x0084)
#define CTMR_CTRL_GET_L2_OFFSET             (0x0088)
#define CTMR_CTRL_GET_L3_OFFSET             (0x008C)

/* CTMR Satellite Timer ID: 0-pcie0, 1-pcie1, 2-WiFi0, 3-WiFi1 */
#define CTMR_CTRL_STMR(core)                \
	(((core) < NUM_PCORES) ? (core) : ((core) - NUM_PCORES + 2))
#define CTMR_CTRL_GET_L_OFFSET(core)        \
	(CTMR_CTRL_GET_L0_OFFSET + CTMR_CTRL_STMR((core)) * 4)

/* SAT_TIMER: Satellite Timer Registers */
#define SAT_TIMER_CTRL_OFFSET                (0x0000)
#define SAT_TIMER_SET_L_OFFSET               (0x000C)
#define SAT_TIMER_SET_H_OFFSET               (0x0010)
#define SAT_TIMER_INC_L_OFFSET               (0x0014)
#define SAT_TIMER_INC_H_OFFSET               (0x0018)

#define PSCI_BRCM_SYSTEM_CONTROL             0x8400001F
#define CTMR_CTRL_STMR_WRITE_CMD             0x80000000

#define FASTPATH_ADDR_UNINITIALIZED          -1ULL

/*
 * +-----------------------------------------------------
 *  Macros
 * +-----------------------------------------------------
 */
#define PCI_EXP_LNK_WIDTH(width)            \
	(((width) & PCI_EXP_LNKSTA_NLW) >> PCI_EXP_LNKSTA_NLW_SHIFT)

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
 *  - Common interdependent settings (cpci)
 *  - Core independent settings      (pcie@)
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

#define HCD_CORE_SETTING(name)                          \
	((nv_##name != HCD_SETTING_VAL_INV) ?  nv_##name : name)

#define HCD_CMMN_SETTING(name)               (name)

/*
 * Each core using 'N' u32 values with initialized to 'def' value
 */
#define HCD_DECLARE_CORE_ARRAY_SETTING(name, N, def)    \
	u32 nv_##name[MAX_NUM_PCIE_CORES * (N)] = {         \
	    [0 ... ((MAX_NUM_PCIE_CORES*(N))-1)] =          \
	        HCD_SETTING_VAL_INV};                       \
	u32 name[MAX_NUM_PCIE_CORES * (N)] = {              \
	    [0 ... ((MAX_NUM_PCIE_CORES*(N))-1)] = (def)};  \
	int nr_##name = (N);                                \
	module_param_array(name, int, NULL, S_IRUGO)

#define HCD_READ_CORE_ARRAY_SETTING(name, core)         \
	do {                                                \
	    u32 val[HCD_CORE_ARRAY_SETTING_VALUES] = {0};   \
	    if (pcie_hcd_nvram_get_u32_array(#name, core,   \
	        nr_##name, val) > 0) {                      \
	        memcpy(nv_##name + ((core) * nr_##name),    \
	            val, nr_##name * sizeof(u32));          \
	    }                                               \
	} while (0)

#define HCD_UPDATE_CORE_ARRAY_SETTING(name, core)       \
	do {                                                \
	    int idx = 0;                                    \
	    do {                                            \
	        u32 nv_val = nv_##name[(core)][idx];        \
	        if (nv_val != HCD_SETTING_VAL_INV)          \
	            name[(core)][idx] = nv_val;             \
	    } while (++idx < nr_##name);                    \
	} while (0)

#define HCD_GET_CORE_ARRAY_SETTING(name, core, idx)                           \
	(nv_##name[((core)*nr_##name)+(idx)] != HCD_SETTING_VAL_INV) ?            \
	    nv_##name[((core)*nr_##name)+(idx)] : name[((core)*nr_##name)+(idx)]

#define HCD_CORE_ARRAY_SETTING               HCD_GET_CORE_ARRAY_SETTING

/* Proc File system Core Status information */
#define foreachcore_seq_printf(m, c, sts, fmt, args...)   \
{                                                         \
    int id;                                               \
	struct pcie_hcd_procfs *pfs_cb = &pcie_hcd_procfs_cb; \
	if (sts) seq_printf(m, "%15s : ", sts);               \
	for (id = 0; id < MAX_NUM_PCIE_CORES; id++) {         \
	    c = pfs_cb->pdrv[id];                             \
	    if (c == NULL) continue;                          \
	    seq_printf(m, fmt, ##args);                       \
	}                                                     \
	seq_printf(m, "\n");                                  \
}

/*
 * +-----------------------------------------------------
 *  Structures
 * +-----------------------------------------------------
 */
/*
 * PCIe Host Controller Driver Procfs control block
 * @dir:      pcie_hcd procfs directory file pointer
 * @coreinfo: pcie_hcd coreinfo procfs file pointer
 * @status:   pcie_hcd status procfs file pointer
 * @pdrv:     Host Controller Driver CB array
 */
struct pcie_hcd_procfs {
	struct proc_dir_entry *dir;
	struct proc_dir_entry *coreinfo;
	struct proc_dir_entry *status;
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
 * @pdrv:          Pointer to PCIe driver cb
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
	DECLARE_BITMAP(used, MAX_MSI_MAP_SIZE);
	int                    irq;
#endif /* CONFIG_BCM_PCI_MSI */
	int                    nr;
	bool                   enabled;
	struct pcie_hcd        *pdrv;
};

/*
 * BCM PCIe Host Controller Driver Outgoing Window control block
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
 * BCM PCIe Host Controller Driver Error Log interrupt control block
 *
 * @inited:     Errorlog interrupts initialization status
 */
struct pcie_hcd_errlog
{
	bool   inited;
};

/*
 * BCM PCIe Host Controller Driver apon control block
 *
 * @flags:     power operation flags
 * @powered:   power status of device
 * @cfg:       power configuration of device
 */
struct pcie_hcd_apon
{
	u32    flags;
	bool   powered[MAX_NUM_CORE_DEVS];
	u8     cfg[MAX_NUM_CORE_DEVS];
};

/*
 * BCM PCIe Host Controller power supply control block
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
 * BCM PCIe Host Controller Driver control block
 *
 * @msi:             msi control block
 * @pfs:             pointer to PCIe HCD proc file system control block
 * @pwr_supplies:    external power supplies for the slot
 * @core:            host controller core control block
 * @errlog:          error log control block
 * @apon:            apon control block
 * @owin:            Outgoing window control block
 */
struct pcie_hcd {
	struct pcie_hcd_msi    *msi;
	struct pcie_hcd_procfs *pfs;
	struct pcie_hcd_pwrsup pwrsupplies;

	struct pcie_hc_core    core;
	struct pcie_hcd_errlog errlog;
	struct pcie_hcd_apon   apon;
	struct pcie_hcd_owin   owin[MAX_NUM_OUTGOING_WINDOWS];
	struct pci_config_window *ecam_cfg;
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
 * BCM PCIe HCD FastPath Address Management (fpam) control block
 *
 * @flags:          Configuration flags
 * @master:         Master core for Broadcast/Mutex
 * @latent_master:  Likely to be Master from auto selection
 * @in_pcie_addr:   Incoming fastpath PCIe address base
 * @in_ubus_addr:   Incoming fastpath UBUS address
 * @out_addr:       Outgoing fastpath address for Broadcast
 */
struct pcie_hcd_bus_fpam
{
	/* Configuration */
	u8  flags;
	u8  master;
	u8  latent_master;

	/* incoming addresses */
	u64 in_pcie_addr;
	u64 in_ubus_addr;

	/* outgoing addresses */
	u64 out_addr[FASTPATH_ID_MAX];
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
	struct pcie_hcd_bus_fpam    fpam;
};

/*
 * +-----------------------------------------------------
 *  Local Function prototype
 * +-----------------------------------------------------
 */
/* nvram */
static int pcie_hcd_nvram_get_u32_array_tokenized(char *key, int core, int max,
	u32 *pval);
static int pcie_hcd_nvram_get_u32_array(char *key, int core, int max,
	u32 *pval);
static int pcie_hcd_nvram_get_u32(char *key, u32 *pval);
static int pcie_hcd_get_boot_order_core(int index);
static int pcie_hcd_nvram_init(void);

/* procfs */
static int pcie_hcd_procfs_init(struct pcie_hcd *pdrv);
static void pcie_hcd_procfs_deinit(struct pcie_hcd *pdrv);

/* Fastpath */
static int pcie_hcd_lnkdn_fastpath(struct pcie_hcd *pdrv);
static int pcie_hcd_init_fastpath(u32 config);

/* deferred scan */
static int pcie_hcd_init_deferred_scan(void);

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
static int pcie_hc_config_core_final(struct pcie_hcd *pdrv);
static void pcie_hc_unconfig_core(struct pcie_hcd *pdrv);
static int pcie_hc_config_core(struct pcie_hcd *pdrv);
static int pcie_hc_reset_core(struct pcie_hcd *pdrv);
static int pcie_hc_restart_core(struct pcie_hcd *pdrv);
static int pcie_hc_setup_access(struct pcie_hcd *pdrv);
static void pcie_hc_remove_access(struct pcie_hcd *pdrv);
static int pcie_hc_init_core(struct platform_device *pdev,
	struct pcie_hcd *pdrv);
static void pcie_hc_free_core(struct pcie_hcd *pdrv);
static int pcie_hc_update_core_cfg(struct pcie_hcd *pdrv);
static void pcie_hc_dump_core_cfg(struct pcie_hcd *pdrv);

/* APON */
static int pcie_hc_apon_pwrup(struct pcie_hcd *pdrv);
static int pcie_hc_apon_pwrdn(struct pcie_hcd *pdrv);
static int pcie_hc_apon_pwrphy(struct pcie_hcd *pdrv);

/* Hc helper functions */
static int pcie_hcd_set_power(struct pcie_hc_core *phc, unsigned int devidx,
	bool up, int type);

/* Power */
static int pcie_hcd_set_regulators(struct pcie_hcd *pdrv, unsigned int devidx,
	bool up);
static int pcie_hcd_init_regulators(struct pcie_hcd *pdrv);
static void pcie_hcd_free_regulators(struct pcie_hcd *pdrv);

/* PCI fops */
static int pcie_hcd_config_read(struct pci_bus *bus, unsigned int devfn,
	int where, int size, u32 *val);
static int pcie_hcd_config_write(struct pci_bus *bus, unsigned int devfn,
	int where, int size, u32 val);
static void __iomem *pcie_hcd_map_bus(struct pci_bus *bus,
	unsigned int devfn, int where);
static void __iomem *pcie_hcd_map_bus_ecam(struct pci_bus *bus,
	unsigned int devfn, int where);
static int pcie_hcd_map_irq(const struct pci_dev *pcidev,
	u8 slot, u8 pin);

/* PCI resources */
static int pcie_hcd_setup_resources(struct pcie_hcd *pdrv,
	struct list_head *resources);
static void pcie_hcd_unmap_resources(struct pcie_hcd *pdrv);
static int pcie_hcd_parse_resources(struct pcie_hcd *pdrv,
	struct device_node *np);
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
static void __exit pcie_hcd_plt_deinit(void);

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

static struct pci_ecam_ops pcie_hcd_ops_ecam;

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
 * (if exists, Device Tree entry has higher preference than this setting)
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
 * (if exists, Device Tree entry has higher preference than this setting)
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
 * (if exists, Device Tree entry has higher preference than this setting)
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
 * (if exists, Device Tree entry has higher preference than this setting)
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
 * (if exists, Device Tree entry has higher preference than this setting)
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
 * PCIe fastpath Configuration
 * (if exists, Device Tree entry has higher preference than this setting)
 *
 *     Each 4bit's corresponds to a setting
 *     [31-28] [27-24] [23-20] [19-16] [15-12] [11-08]  [07-04]  [03-00]
 *     [                   Reserved          ] [Master] [    flags     ]
 *
 *     Flags:
 *     bit#0 - Enable incoming fastpath
 *     bit#1 - Enable outgoing fastpath
 *     bit#2 - Enable non-exclusive mutex clear
 *     bit#3 - Select given fastpath master (from bits#11:8)
 *
 *     Master: (if bit#3 is set to 1'b)
 *     bit#11-8 - master PCIe core id for fastpath (0, 1)
 */
HCD_DECLARE_CORE_SETTING(pcie_fastpath, FASTPATH_EN_MASK);

/*
 * config_speed values
 * (if exists, Device Tree entry has higher preference than this setting)
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
 * (if exists, Device Tree entry has higher preference than this setting)
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
 * (if exists, Device Tree entry has higher preference than this setting)
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
 * (if exists, Device Tree entry has higher preference than this setting)
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
 * (if exists, Device Tree entry has higher preference than this setting)
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
 * (if exists, Device Tree entry has higher preference than this setting)
 *
 *     0 - Auto lane detection
 *     1 - Force to single lane on a dual lane configuration (RC + EP)
 *
 *     Each 4bit's corresponds to a PCIe core id
 *     [31-28] [27-24][23-20] [19-16] [15-12] [11-08] [07-04] [03-00]
 *                                    [core4] [core3] [core2] [core1]
 */
HCD_DECLARE_CORE_SETTING(pcie_numlanes, 0x0000);

/*
 * PCIe ACK and Credit Coalescing
 * (if exists, Device Tree entry has higher preference than this setting)
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
 * (if exists, Device Tree entry has higher preference than this setting)
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
 * (if exists, Device Tree entry has higher preference than this setting)
 *
 *     0 - Disable MDIO Rx Filtering
 *     1 - Enable RC MDIO Rx Filtering
 *     2 - Enable EP MDIO Rx Filtering
 *     3 - Enable RC+EP MDIO Rx Filtering
 *
 *     Each 4bit's corresponds to a PCIe core
 *     [31-28] [27-24][23-20] [19-16] [15-12] [11-08] [07-04] [03-00]
 *                                    [core4] [core3] [core2] [core1]
 */
HCD_DECLARE_CORE_SETTING(pcie_phyrxfilter, 0x0000);

/*
 * End Point Power On (GPIO Regulator) after PERSTB asset
 * (if exists, Device Tree entry has higher preference than this setting)
 *
 *     0 - Regulator Power up during RC power Up
 *     1 - Regulator power up after PERSTB=1 (default)
 *
 *     Each 4bit's corresponds to a PCIe core
 *     [31-28] [27-24][23-20] [19-16] [15-12] [11-08] [07-04] [03-00]
 *                                    [core4] [core3] [core2] [core1]
 */
HCD_DECLARE_CORE_SETTING(pcie_epuperstb, 0x1111);

/*
 * Priority Queues
 * (if exists, Device Tree entry has higher preference than this setting)
 *
 *     0 - Do not use Priority Queues
 *     1 - Use Priority Queues
 *
 *     Each 4bit's corresponds to a PCIe core
 *     [31-28] [27-24][23-20] [19-16] [15-12] [11-08] [07-04] [03-00]
 *                                    [core4] [core3] [core2] [core1]
 */
HCD_DECLARE_CORE_SETTING(pcie_prioqueue, 0x1111);

/*
 * GEN3 link equalization presets for the lanes
 * (if exists, Device Tree entry has higher preference than this setting)
 *
 *     Each nibble represent a preset values of RC-TX/RX, EP-TX/RX in order with
 *     the following possible values
 *
 *     0-11: Tx Preset for EP/RC
 *     0-7:  Rx Preset Hint for EP/RC
 *
 *     [31.................16] [15...12] [11...08] [07...04] [03...00]
 *     [       Reserved      ] [ EP RX ] [ EP TX ] [ RC RX ] [ RC TX ]
 */
HCD_DECLARE_CORE_ARRAY_SETTING(pcie_gen3_eq_presets, HCD_PHYLANE_MAX, 0x0000);

/* APON */
const static char pcie_hcd_apon_name[HCD_APON_LAST+1][8] = {
	"AUTO", "ON", "OFF", "DOMAIN"
};

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
static inline int pcie_hcd_pmc_power_up(int unit, int is_dual_lane) {
	return 0;
}
static inline int pcie_hcd_pmc_power_down(int unit, int is_dual_lane) {
	return 0;
}
static inline int pcie_hcd_get_port_mode(int port, struct device_node *np) {
	/* Always RC mode */
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
	    HCD_ERROR("kstrtoul for 0x%d with [%s] returned res %d\n",
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
static inline int pcie_hcd_ubus_decode_pcie_wnd_cfg(u32 base, u32 size,
	u32 core) {
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

static inline int pcie_hcd_stmr_en(struct pcie_hcd *pdrv) {
	struct pcie_hc_core *phc = &pdrv->core;
	u32 stmr_id = CTMR_CTRL_STMR(phc->info.id);
	u32 reg_data;
	struct arm_smccc_res res;

	/*
	 * Clear the enable for satellite timer by writing a '0' in bit-x
	 * (but keep the other bits in range 7:0 as a '1')
	 * It is not necessary to write a '1' back into bit-x.
	 * It would be ignored.
	 */
	reg_data = readl(phc->info.ctmr + CTMR_CTRL_STMR_EN_OFFSET);
	reg_data &= 0xFF;
	reg_data |= 0xACCE5500;
	reg_data &= (~(1 << stmr_id));

	arm_smccc_smc(PSCI_BRCM_SYSTEM_CONTROL, CTMR_CTRL_STMR_WRITE_CMD, reg_data,
	        0, 0, 0, 0, 0, &res);

	return 0;
}

static inline u32 pcie_hcd_read_reg(struct pcie_hc_core *phc, u32 reg) {
	return phc->plt_cfg->read_reg(phc, reg);
}

static inline void pcie_hcd_write_reg(u32 data, struct pcie_hc_core *phc,
	u32 reg) {
	return phc->plt_cfg->write_reg(data, phc, reg);
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
 *    pdrv ... pointer to PCIe core HCD data structure
 *
 *   Description:
 *    Get root bus for the given driver
 *
 *   Return: Pointer to Root bus on success, NULL on failure
 */
struct pci_bus *pcie_hcd_get_rootbus(struct pcie_hcd *pdrv)
{
	struct pci_host_bridge *bridge = pci_host_bridge_from_priv(pdrv);
	return bridge->bus;
}

/*
 * Function pcie_hcd_get_hccore_from_bus (bus)
 *
 *   Parameters:
 *    bus ... pointer to PCIe device bus data structure
 *
 *   Description:
 *    Get HC core control block from bus sysdata
 *
 *   Return: Pointer to host contller control block on success, NULL on failure
 */
struct pcie_hc_core *pcie_hcd_get_hccore_from_bus(struct pci_bus *bus)
{
	struct pcie_hc_core* phc = NULL;

	if (bus->ops == &pcie_hcd_ops_ecam.pci_ops) {
	    struct pci_config_window *cfg = (struct pci_config_window*)bus->sysdata;
	    if (cfg)
	        phc = (struct pcie_hc_core*)cfg->priv;
	} else {
	    phc = (struct pcie_hc_core*)bus->sysdata;
	}

	return phc;
}

/*
 * Function pcie_hcd_sync_stmr (pdrv)
 *
 *   Parameters:
 *    pdrv ... pointer to PCIe core HCD data structure
 *    stmr ... pointer to mapped satellite timer sub-block registers
 *
 *   Description:
 *    Synchronize PCIe local timers with Central Timers. Applicable for both
 *    physical and virtual (WiFi) cores that support Central Timer block
 *
 *   Return: 0: on success, -ve on failure
 */
int pcie_hcd_sync_stmr(struct pcie_hcd *pdrv, void __iomem *stmr)
{
	struct pcie_hc_core *phc = &pdrv->core;
	u64 ctmr_cntr64;
	u32 reg_data;
	u32 l_offset = CTMR_CTRL_GET_L_OFFSET(phc->info.id);

	HCD_FN_ENT();

	if ((phc->info.ctmr == NULL) || (stmr == NULL)) {
	    /*
	     * SoC doesn't support CTMR, or (v)PCIe doesn't support Satellite
	     * Timers. Nothing to do.
	     */
	    HCD_FN_RET_VAL(0);
	}

	if (pcie_hcd_stmr_en(pdrv) == -EALREADY) {
	    /* Satellite timer already enabled, nothing to do */
	    HCD_FN_RET_VAL(0);
	}

	/* Read Central Timer counter (low first and then high) */
	ctmr_cntr64 = readl(phc->info.ctmr + l_offset);
	ctmr_cntr64 |=
	((u64)readl(phc->info.ctmr + CTMR_CTRL_GET_H_OFFSET) << 32);

	/* for perfect alignment between central and satellite timer */
	ctmr_cntr64 += 2;

	/*
	 * Configure PCIe satellite timer with central timer counter
	 * (low first and then high)
	 */
	reg_data = ctmr_cntr64 & 0xFFFFFFFF;
	writel(reg_data, stmr + SAT_TIMER_INC_L_OFFSET);
	reg_data = (ctmr_cntr64 >> 32) & 0xFFFFFFFF;
	writel(reg_data, stmr + SAT_TIMER_INC_H_OFFSET);

	HCD_LOG("Core [%d] Synchronized the Satellite Timer with 0x%llx\n",
	    phc->info.id, ctmr_cntr64);

	HCD_FN_RET_VAL(0);
}

/*
 * Function pcie_hcd_update_fastpath_in_addr (pdrv, pcie_addr, ubus_offset)
 *
 *   Parameters:
 *    pdrv      ... Pointer to PCIe core HCD data structure
 *    pcie_addr ... Incoming broadcast address base for PCIe bus (End Point)
 *    ubus_addr ... Incoming mutex address for UBUS (internal WiFi)
 *
 *   Description:
 *    Update Root Complex Mutex Master information master core id,
 *    incoming addresses for PCIe and UBUS buses
 *
 *   Return: 0: on fastpath master, -ve otherwise
 */
int pcie_hcd_update_fastpath_in_addr(struct pcie_hcd *pdrv, u64 pcie_addr,
	u32 ubus_offset)
{
	struct pcie_hc_core      *phc = &pdrv->core;
	struct pcie_hcd_bus_fpam *fpam = &(pcie_hcd_cpci_get()->fpam);
	int                      rc = -EPERM;

	HCD_FN_ENT();

	if (fpam->flags & FASTPATH_EN_MASK) {
	    fpam->in_pcie_addr = pcie_addr;
	    if ((fpam->latent_master != INV_CORE) && (fpam->master == INV_CORE)) {
	        /* Mutex master selection is enabled and not selected yet */
	        /* For auto selection, update Master core id with first core */
	        if (!(fpam->flags & FASTPATH_MASTER)) {
	            fpam->latent_master = phc->info.id;
	        }

	        /* Update incoming address for PCIe and UBUS */
	        if (fpam->latent_master == phc->info.id) {
	            HCD_LOG("Core [%d] fastpath master\n", phc->info.id);
	            fpam->master = fpam->latent_master;
	            fpam->in_ubus_addr = (u64)(phc->res.base.start + ubus_offset);
	            rc = 0;
	        }
	    }
	}

	if (rc == 0) {
	    HCD_INFO("Core [%d] Update fastpath Flags [0x%x] Master [0x%x] "
	        "Latent [0x%x] pcie_addr [0x%llx] ubus_addr [0x%llx]\n",
	        phc->info.id, fpam->flags, fpam->master, fpam->latent_master,
	        fpam->in_pcie_addr, fpam->in_ubus_addr);
	}

	HCD_FN_RET_VAL(rc);
}

/*
 * Function pcie_hcd_get_fastpath_in_addr (pdrv)
 *
 *   Parameters:
 *    pdrv      ... Pointer to PCIe core HCD data structure
 *
 *   Description:
 *    Return the incoming fastpath base address to be used by the RC
 *    for broadcast messages
 *
 *   Return: incoming fast path address
 */
u64 pcie_hcd_get_fastpath_in_addr(struct pcie_hcd *pdrv)
{
	struct pcie_hc_core      *phc = NULL;
	struct pcie_hcd_bus_fpam *fpam = &(pcie_hcd_cpci_get()->fpam);
	u64                      addr = 0;

	HCD_FN_ENT();

	if (pdrv) phc = &pdrv->core;

	if (fpam->flags & FASTPATH_EN_IN) {
	    if (phc && (PLTID2HC(phc->info.id) == PHC)) {
	        /* External WiFi uses address coming from PCIe bus */
	        if (fpam->in_pcie_addr) {
	            addr = fpam->in_pcie_addr;
	        }
	    } else if (fpam->master != INV_CORE) {
	        /* Internal WiFi uses address coming from UBUS */
	        if (fpam->in_ubus_addr) {
	            addr = fpam->in_ubus_addr;
	        }
	    }
	}

	HCD_INFO("Core [%d] Get fastpath flags [0x%x] Master [0x%x]"
	    " in_addr [0x%llx]\n",
	    (phc) ? phc->info.id : fpam->master, fpam->flags, fpam->master, addr);

	HCD_FN_RET_VAL(addr);
}

/*
 * Function pcie_hcd_update_fastpath_out_addr (pdrv, np)
 *
 *   Parameters:
 *    pdrv  ... Pointer to PCIe core HCD data structure
 *    np    ... vPCIe device tree node pointer
 *
 *   Description:
 *    Set the outgoing fastpath address for the given physical core (BAR2 space
 *    last 4 bytes) or virtual core (mlo register space last 4 bytes) used for
 *    broadcast messages
 *
 *   Return: outgoing fastpath address
 */
u64 pcie_hcd_update_fastpath_out_addr(struct pcie_hcd *pdrv,
	struct device_node *np)
{
	struct pcie_hc_core      *phc = NULL;
	struct pcie_hcd_bus_fpam *fpam = &(pcie_hcd_cpci_get()->fpam);
	u64                      addr = 0;
	int                      core;

	HCD_FN_ENT();

	if (!(fpam->flags & FASTPATH_EN_OUT)) {
	    /* Nothing to do */
	    HCD_FN_RET_VAL(addr);
	}

	/* For physical cores, use BAR2 space last 4 bytes */
	/* For virtual cores, use MLO register space last 4 bytes */
	if (pdrv) {
	    int ep_res = (FASTPATH_EP_IN_BCAST_BAR << 1);
	    struct pci_bus *root;
	    struct pci_bus *child;
	    struct pci_dev *pdev;
	    u32 size;

	    phc = &pdrv->core;
	    core = phc->info.id;

	    /* Gather information needed */
	    root = pcie_hcd_get_rootbus(pdrv);
	    /* resources are based on 32bit bars, (twice the 64bit bars) */
	    list_for_each_entry(child, &root->children, node) {
	        list_for_each_entry(pdev, &child->devices, bus_list) {
	            size = pci_resource_len(pdev, ep_res);
	            if (size > 0) {
	                addr = (u64)pci_resource_start(pdev, ep_res);
	                /* Use last 4Bytes of BAR2 address space for fastpath out */
	                addr += (size - 0x4);
	                break;
	            }
	        }
	    }
	} else if (of_property_read_u32(np, "brcm,coreid", &core) >= 0) {
	    struct resource mlo_reg;

	    core += NUM_PCORES;
	    /* WIFI MLO core register base (optional) */
	    if (of_address_to_resource(np, 1, &mlo_reg) == 0) {
	        addr = mlo_reg.start + FASTPATH_PCIE_OUT_BCAST;
	    }
	}

	if (addr)
	    fpam->out_addr[FASTPATH_COREID_TO_REQID(core)] = addr;

	HCD_INFO("Core [%d] Get fastpath flags [0x%x] Master [0x%x]"
	    " out_addr [0x%llx]\n",
	    (phc) ? phc->info.id : fpam->master, fpam->flags, fpam->master, addr);

	HCD_FN_RET_VAL(addr);
}

/*
 * Function pcie_hcd_get_fastpath_out_addr (pdrv, core)
 *
 *   Parameters:
 *    pdrv  ... Pointer to PCIe core HCD data structure
 *    core  ... PCIe core identifier
 *
 *   Description:
 *    Return the outgoing fastpath base address to be used by the RC
 *    for broadcast messages
 *
 *   Return: outgoing fastpath address
 */
u64 pcie_hcd_get_fastpath_out_addr(struct pcie_hcd *pdrv, int core)
{
	struct pcie_hc_core      *phc = NULL;
	struct pcie_hcd_bus_fpam *fpam = &(pcie_hcd_cpci_get()->fpam);
	u64                      addr = 0;
	int                      configure = 1;

	HCD_FN_ENT();

	if (pdrv) {
	    if (core < 0) configure = 0;
	    phc = &pdrv->core;
	    core = phc->info.id;
	} else {
	    /* valid 'core' will be supplied by the caller */
	    configure = 0;
	}

	if (fpam->flags & FASTPATH_EN_OUT) {
	    u64* p_addr = &(fpam->out_addr[FASTPATH_COREID_TO_REQID(core)]);

	    if ((*p_addr == FASTPATH_ADDR_UNINITIALIZED) && (configure)) {
	        *p_addr = 0;
	        /*
	         * Fast path address is not initialized yet.
	         * Initialize and enable fastpath out address if supported by hc
	         */
	        pcie_hc_config_core_final(pdrv);
	    }
	    addr = *p_addr;
	    /* Set to 0 if still not initialized */
	    if (addr == FASTPATH_ADDR_UNINITIALIZED) addr = 0;
	}


	HCD_INFO("Core [%d] Get fastpath flags [0x%x] Master [0x%x]"
	    " out_addr [0x%llx]\n",
	    (phc) ? phc->info.id : fpam->master, fpam->flags, fpam->master, addr);

	HCD_FN_RET_VAL(addr);
}

/*
 * Function pcie_hcd_get_fastpath_mutexaddr (pdrv)
 *
 *   Parameters:
 *    pdrv      ... Pointer to PCIe core HCD data structure
 *
 *   Description:
 *    Return the incoming mutex lock address to be used by the mutex master core
 *
 *   Return: 0: on success, -ve on failure
 */
u64 pcie_hcd_get_fastpath_mutexaddr(struct pcie_hcd *pdrv)
{
	struct pcie_hc_core      *phc = NULL;
	struct pcie_hcd_bus_fpam *fpam = &(pcie_hcd_cpci_get()->fpam);
	u64                      addr = 0;

	HCD_FN_ENT();

	if (pdrv) phc = &pdrv->core;

	if (fpam->flags & FASTPATH_EN_IN) {
	    if (phc && (PLTID2HC(phc->info.id) == PHC)) {
	        /* External WiFi uses address coming from PCIe bus */
	        if (fpam->in_pcie_addr) {
	            addr = fpam->in_pcie_addr + FASTPATH_PCIE_IN_MUTEX;
	        }
	    } else if (fpam->master != INV_CORE) {
	        /* Internal WiFi uses address coming from UBUS */
	        if (fpam->in_ubus_addr) {
	            addr = fpam->in_ubus_addr - FASTPATH_PCIE_IN_MUTEX;
	        }
	    }
	}

	addr |= (fpam->flags & FASTPATH_EN_IN);
	addr |= ((fpam->flags & FASTPATH_NONEXCLUSIVE) >> 1);

	HCD_INFO("Core [%d] Get fastpath flags [0x%x] Master [0x%x] Latent [0x%x]"
	    " pcie_addr [0x%llx] ubus_addr [0x%llx] mutex_addr [0x%llx]\n",
	    (phc) ? phc->info.id : fpam->master, fpam->flags, fpam->master,
	    fpam->latent_master, fpam->in_pcie_addr, fpam->in_ubus_addr, addr);

	HCD_FN_RET_VAL(addr);
}

/*
 * Function pcie_hcd_get_fastpath_mutexconfig (pdrv)
 *
 *   Parameters:
 *    pdrv      ... Pointer to PCIe core HCD data structure
 *
 *   Description:
 *    Return the mutex configuration for the given core
 *
 *   Return: 0: on success, -ve on failure
 */
u32 pcie_hcd_get_fastpath_mutexconfig(struct pcie_hcd *pdrv)
{
	struct pcie_hc_core      *phc = NULL;
	struct pcie_hcd_bus_fpam *fpam = &(pcie_hcd_cpci_get()->fpam);
	u32                      config = 0;

	HCD_FN_ENT();

	if (pdrv) phc = &pdrv->core;

	if (phc) {
	    u32 req_id = FASTPATH_COREID_TO_REQID(phc->info.id);

	    config = (fpam->flags & FASTPATH_EN_IN);
	    config |= ((fpam->flags & FASTPATH_NONEXCLUSIVE) >> 1);
	    config |= (req_id << 4);

	    HCD_INFO("Core [%d] Get fastpath flags [0x%x] Master [0x%x]"
	        " pcie_addr [0x%llx] ubus_addr [0x%llx] mutex_id [%d]\n",
	        (phc) ? phc->info.id : fpam->master, fpam->flags, fpam->master,
	        fpam->in_pcie_addr, fpam->in_ubus_addr, req_id);
	}

	HCD_FN_RET_VAL(config);
}

/*
 * Function pcie_hcd_lnkdn_fastpath (pdrv)
 *
 *   Parameters:
 *    pdrv      ... Pointer to PCIe core HCD data structure
 *
 *   Description:
 *    Process fastpath address management for link down core
 *
 *   Return: 0: on success, -ve on failure
 */
static int pcie_hcd_lnkdn_fastpath(struct pcie_hcd *pdrv)
{
	struct pcie_hc_core      *phc = &pdrv->core;
	struct pcie_hcd_bus_fpam *fpam = &(pcie_hcd_cpci_get()->fpam);
	int                      rc = -ENODEV;

	HCD_FN_ENT();

	/* Allow configuring the fastpath to determine the master */
	if (phc->info.id == fpam->latent_master) {
	    pcie_hc_config_core(pdrv);
	}

	/* Power down PHY and EP device if allowed and keep RC digital power ON */
	if (pdrv->apon.flags != APON_F_ON) {
	    if ((phc->info.id == fpam->master) ||
	        (pdrv->apon.flags == APON_F_OFF_W_DOM)) {

	        pdrv->apon.flags = APON_F_OFF_W_DOM;

	        pcie_hc_apon_pwrdn(pdrv);
	        rc = 0;
	        HCD_INFO("Core [%d] Keep Digital ON\n", phc->info.id);
	    }
	}

	HCD_FN_RET_VAL(rc);
}

/*
 * Function pcie_hcd_init_fastpath (config)
 *
 *   Parameters:
 *    config      ... fastpath configuration
 *
 *   Description:
 *    Initialize fastpath address manager with the given configuration.
 *    If enabled, select the fastpath master core
 *
 *   Return: 0: on success, -ve on failure
 */
static int pcie_hcd_init_fastpath(u32 config)
{
	struct pcie_hcd_bus_fpam *fpam = &(pcie_hcd_cpci_get()->fpam);
	int req_id;

	HCD_FN_ENT();

	/* Initialize with defaults */
	fpam->in_pcie_addr = 0;
	fpam->in_ubus_addr = 0;
	fpam->master = INV_CORE;
	fpam->latent_master = INV_CORE;
	for (req_id = 0; req_id < FASTPATH_ID_MAX; req_id++) {
	    fpam->out_addr[req_id] = -1ULL;
	}

	/* Update control block from input setting */
	fpam->flags = HCD_GET_CMMN_SETTING(config, 0);

	/*
	 * Select last available PCIe core as likely to be the fastpath master
	 */
	if (fpam->flags & FASTPATH_EN_MASK) {
	    u32                core = INV_CORE;
	    int                latent_master = INV_CORE;
	    struct device_node *np;
	    int                vcores = 0;

	    if (fpam->flags & FASTPATH_MASTER) {
	        fpam->latent_master = HCD_GET_CMMN_SETTING(config, 2);
	    }

	    /* Search all available "pci" device tree nodes */
	    for_each_node_by_type(np, "pci") {

	        /* On success, writes core value */
	        of_property_read_u32(np, "brcm,coreid", &core);

	        if (of_device_is_available(np) && (core != INV_CORE)) {
	            /* Update as potential future master */
	            latent_master = core;
	        } else if (core == fpam->latent_master) {
	            HCD_ERROR("Core [%d] fastpath master is not enabled, Switching"
	                " to auto master mode\n", core);
	            fpam->latent_master = INV_CORE;
	            fpam->flags &= ~FASTPATH_MASTER;
	        }
	    }

	    /* Search all available "vpci" device tree nodes */
	    for_each_node_by_type(np, "vpci") {
	        if (of_device_is_available(np)) {
	            pcie_hcd_update_fastpath_out_addr(NULL, np);
	            vcores++;
	        }
	    }

	    /* For auto selection, store the likely master information */
	    if ((vcores != 0) && (fpam->latent_master == INV_CORE)) {
	        if (latent_master != INV_CORE) {
	            fpam->latent_master = latent_master;
	            HCD_LOG("Core [%d] latent fastpath master\n", latent_master);
	        } else {
	            HCD_ERROR("Core [all] no fastpath master found\n");
	        }

	        /*
	         * latent_master will become master during core configuration if
	         * supports fastpath
	         */
	    }
	}

	HCD_INFO("Core [all] Init fastpath Flags [0x%x] Master [0x%x] Latent [0x%x]"
	    " pcie_addr [0x%llx] ubus_addr [0x%llx]\n", fpam->flags, fpam->master,
	    fpam->latent_master, fpam->in_pcie_addr, fpam->in_ubus_addr);

	HCD_FN_RET_VAL(0);
}

/*
 * +-----------------------------------------------------
 *  Local Functions
 * +-----------------------------------------------------
 */

/*
 * Function pcie_hcd_init_deferred_scan ()
 *
 *   Parameters:
 *
 *   Description:
 *    Initialize deferred scan order after filtering disabled cores in
 *    device-tree
 *
 *   Return: 0: on success, -ve on failure
 */
static int pcie_hcd_init_deferred_scan(void)
{
	struct device_node *np;
	u32    scan_order;
	u32    core;
	int    order, index;
	int    avail[MAX_NUM_PCIE_CORES] = {false};

	HCD_FN_ENT();

	/* scan order is not set, nothing to do */
	if (!pcie_scan_order) {
	    HCD_FN_RET_VAL(0);
	}

	/* Get all available physical cores */
	for_each_node_by_type(np, "pci") {
	    core = INV_CORE;
	    of_property_read_u32(np, "brcm,coreid", &core);
	    if (of_device_is_available(np) && (core < MAX_NUM_PHY_PCIE_CORES)) {
	        avail[core] = true;
	    }
	}

	/* Get all available virtual cores */
	for_each_node_by_type(np, "vpci") {
	    core = INV_CORE;
	    of_property_read_u32(np, "brcm,coreid", &core);
	    if (of_device_is_available(np) && (core < MAX_NUM_VIRT_PCIE_CORES)) {
	        avail[core + MAX_NUM_PHY_PCIE_CORES] = true;
	    }
	}

	/* Adjust scan order based on available cores */
	scan_order = 0x0;
	order = 0;
	for (index = 0; index < MAX_NUM_PCIE_CORES; index++) {
	    core = HCD_HC_CORE_CFG(pcie_scan_order, index);
	    if ((core < MAX_NUM_PCIE_CORES) && (avail[core] == true)) {
	        /* core DT active, add to new scan order */
	        HCD_HC_SET_CORE_CFG(scan_order, order, core);
	        order++;
	    } else {
	        /* core DT not available or in-active, filter out from the list */
	        HCD_INFO("Removing Core [%d] from deferred scan order\n", core);
	    }
	}

	/* Update the scan order with the filtered list */
	if (pcie_scan_order != scan_order) {
	    HCD_LOG("Adjusting [0x%x] deferred scan order to [0x%x]\n",
	        pcie_scan_order, scan_order);

	    pcie_scan_order = scan_order;
	}

	HCD_FN_RET_VAL(0);
}

/*
 * Function pcie_hcd_nvram_get_u32_array_tokenized (key, core, max, pval)
 *
 *   Parameters:
 *    key  ... nvram key string
 *    core ... Core index into the main setting
 *    max ...  Maximum number of values to read
 *    pval ... Pointer to u32 values to be returned
 *
 *   Description:
 *    Query the key value from eNVRAM/kNVRAM and convert to u32 values of the
 *    given key values for the following format.
 *    - Core values separated by ',' and separated by ' ' within the core
 *    "core0_val0 core0_val1, core1_val0 core1_val1, ......"
 *
 *   Return: On success - number of array elements returned on success
 *           On failure - -ve value
 */
int pcie_hcd_nvram_get_u32_array_tokenized(char *key, int core, int max,
	u32 *pval)
{
	char valstr[HCD_SETTING_VAL_STR_LEN] = {};
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
	    if (pvalstr != NULL) {
	        strncpy(valstr, pvalstr, sizeof(valstr)-1);
	        pvalstr = valstr;
	    }
	}
#endif /* CONFIG_BCM_NVRAM */
#endif /* !USE_PMC_BRD_STUBS */

	if (pvalstr != NULL) {
	    int cnt = 0;
	    char *core_token = valstr;

	    /* Get the desired core value(s) string (each core separated by ',') */
	    do {
	        core_token = strsep(&pvalstr, ",");

	        if (core_token && (core == cnt)) {
	            char *indx_token = NULL;

	            /* Extract the core setting values (separated by ' ') */
	            cnt = 0;
	            do {
	                indx_token = strsep(&core_token, " ");

	                if (indx_token) {
	                    if ((indx_token[0] == '\0') || (indx_token[0] == ' ') ||
	                        (indx_token[0] == ','))
	                        continue;

	                    if (sscanf(indx_token, "0x%x", &val) != 1) {
	                        val = 0;
	                    }
	                    if (pval) pval[cnt] = val;
	                    cnt++;
	                }
	            } while ((indx_token) && (cnt < max));

	            HCD_FN_RET_VAL(cnt);
	        }
	    } while ((core_token) && (cnt++ < core));
	}

	HCD_FN_RET_VAL(-EINVAL);
}

/*
 * Function pcie_hcd_nvram_get_u32_array (key, core, elem_sz, pval)
 *
 *   Parameters:
 *    key  ... nvram key string
 *    core ... Core id to be used as an array index in the setting
 *    max  ... Number of array elements for each core
 *    pval ... Pointer to u32 array where values to be returned
 *
 *   Description:
 *    Query the key value from eNVRAM/kNVRAM and convert to u32 array of the
 *    given key index
 *    value expected in hex format with a 0x prefix
 *
 *   Return: On success - number of array elements returned on success
 *           On failure - -ve value
 */
int pcie_hcd_nvram_get_u32_array(char *key, int core, int max, u32 *pval)
{
	int rc;
	char core_key[128];

	HCD_FN_ENT();

	/* Check if individual core based setting exists */
	/* Convert pcie_xxxx key  to pcie0_xxxx/pcie1_xxxx key */
	snprintf(core_key, sizeof(core_key), "pcie%d_%s", core, key+5);
	rc = pcie_hcd_nvram_get_u32_array_tokenized(core_key, 0, max, pval);

	if (rc <= 0) {
	    /* Check if global setting exists */
	    rc = pcie_hcd_nvram_get_u32_array_tokenized(key, core, max, pval);
	}

	HCD_FN_RET_VAL(rc);
}

/*
 * Function pcie_hcd_nvram_get_u32 (key, pval)
 *
 *   Parameters:
 *    key  ... nvram key string
 *    pval ... pointer to u32 value to be returned
 *
 *   Description:
 *    Query the key value from eNVRAM/kNVRAM and convert to u32 value
 *    value expected in hex format with a 0x prefix
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
	HCD_READ_CMMN_SETTING(pcie_fastpath);
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
	HCD_READ_CORE_SETTING(pcie_prioqueue);

	/* Updated during core probe
	HCD_READ_CORE_ARRAY_SETTING(pcie_gen3_eq_presets, -1);
	*/

	HCD_FN_RET_VAL(0);

#else /* !MODULE */
	HCD_FN_ENT();

	HCD_ERROR("nvram is not supported in built-in mode\n");

	HCD_FN_RET_VAL(-1);
#endif /* !MODULE */
}

/*
 * Function pcie_hcd_procfs_status_show (m, v)
 *
 *   Parameters:
 *
 *   Description:
 *    Read function for 'status' proc file
 *
 *   Return: length of information on success, 0 on failure/no information
 */
static int pcie_hcd_procfs_status_show(struct seq_file *m, void *v)
{
	struct pcie_hcd *pdrv;
	struct pcie_hcd_bus_fpam *fpam = &(pcie_hcd_cpci_get()->fpam);

	HCD_FN_ENT();

	/*
	 * Common to all Cores
	 */
	seq_printf(m, "\nCOMMON:\n");

	/* (e)nvram status */
	seq_printf(m, "   dt/(e)nvram  : %-15s 0x%-8x %-15s 0x%-8x %-15s 0x%-8x\n",
	    "pcie_apon", HCD_CORE_SETTING(pcie_apon),
	    "pcie_prioqueue", HCD_CORE_SETTING(pcie_prioqueue),
	    "pcie_fastpath", HCD_CMMN_SETTING(pcie_fastpath));

	/* fastpath status */
	seq_printf(m, "      fastpath  : %-15s 0x%-8x %-15s %-10d\n",
	    "Config", pcie_fastpath, "Master", fpam->master);

	/*
	 * Per Core information
	 * (each core information takes up 16 characters wide)
	 */
	seq_printf(m, "\nCORE:             ");
	foreachcore_seq_printf(m, pdrv, (char*)NULL, "Core#%d          ",
	    pdrv->core.info.id);

	foreachcore_seq_printf(m, pdrv, "Revision", "%d.%02d            ",
	    ((pdrv->core.info.rev >> 8) & 0xFF), (pdrv->core.info.rev & 0xFF));

	foreachcore_seq_printf(m, pdrv, "domain", "%04d            ",
	    pci_domain_nr(pcie_hcd_get_rootbus(pdrv)));

	foreachcore_seq_printf(m, pdrv, "base", "0x%08x      ",
	    (u32)pdrv->core.res.base.start);

	foreachcore_seq_printf(m, pdrv, "ecam", "0x%08x      ",
	    (u32)pdrv->core.res.ecam.start);

	foreachcore_seq_printf(m, pdrv, "BC Out Address", "0x%08x      ",
	    (u32)(pcie_hcd_get_fastpath_out_addr(pdrv, -1) & ~3ULL));

	foreachcore_seq_printf(m, pdrv, "BC In Address", "0x%08x      ",
	    (u32)(pcie_hcd_get_fastpath_in_addr(pdrv) & ~3ULL));

	foreachcore_seq_printf(m, pdrv, "Mutex Address", "0x%08x      ",
	    (u32)(pcie_hcd_get_fastpath_mutexaddr(pdrv) & ~3ULL));

	foreachcore_seq_printf(m, pdrv, "Mutex Config", "0x%08x      ",
	    pcie_hcd_get_fastpath_mutexconfig(pdrv));

	foreachcore_seq_printf(m, pdrv, "apon", "%-16s",
	    pcie_hcd_apon_name[ffs(pdrv->apon.flags)-1]);

	seq_printf(m, "\n");

	HCD_FN_RET_VAL(0);
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
	            pci_domain_nr(pcie_hcd_get_rootbus(pfs_cb->pdrv[core])));
	    }
	}

	HCD_FN_RET_VAL(0);
}

/*
 * Function pcie_hcd_procfs_init (pdrv)
 *
 *   Parameters:
 *    pdrv ... pointer to PCIe core HCD data structure
 *
 *   Description:
 *    PCIe HCD driver proc file system initialization
 *
 *   Return: 0 on success, -ve on failure
 */
int pcie_hcd_procfs_init(struct pcie_hcd *pdrv)
{
	struct pcie_hcd_procfs *pfs_cb = &pcie_hcd_procfs_cb;

	HCD_FN_ENT();

	if (pdrv) {
	    /* PCIe core addition to procfs CB */

	    pfs_cb->pdrv[pdrv->core.info.id] = pdrv;
	    pdrv->pfs = pfs_cb;

	    if (pdrv->pfs->dir) {
	        /* Already created, Nothing to do */
	        HCD_FN_RET_VAL(0);
	    }
	    HCD_ERROR("Core [%d] unexpected before procfs init\n",
	        pdrv->core.info.id);
	}

	/* make a directory in /proc if doesn't exist */
	pfs_cb->dir = proc_mkdir("pcie_hcd", NULL);

	if (pfs_cb->dir == NULL) {
	    HCD_ERROR("Unable to create pcie_hcd proc directory\n");
	    HCD_FN_RET_VAL(-1);
	}

	pfs_cb->coreinfo = proc_create_single("coreinfo", 0, pfs_cb->dir,
	    pcie_hcd_procfs_coreinfo_show);
	if (pfs_cb->coreinfo == NULL) {
	    HCD_ERROR("Unable to create pcie_hcd/coreinfo proc file\n");
	    HCD_FN_RET_VAL(-1);
	}

	pfs_cb->status = proc_create_single("status", 0, pfs_cb->dir,
	    pcie_hcd_procfs_status_show);
	if (pfs_cb->status == NULL) {
	    HCD_ERROR("Unable to create pcie_hcd/status proc file\n");
	    HCD_FN_RET_VAL(-1);
	}

	HCD_FN_RET_VAL(0);
}

/*
 * Function pcie_hcd_procfs_deinit (pdrv)
 *
 *   Parameters:
 *    pdrv ... pointer to PCIe core HCD data structure
 *
 *   Description:
 *    PCIe HCD driver proc file system de-initialization
 *
 *   Return: 0 on success, -ve on failure
 */
void pcie_hcd_procfs_deinit(struct pcie_hcd *pdrv)
{
	struct pcie_hcd_procfs *pfs_cb = NULL;
	int core;

	HCD_FN_ENT();

	if (pdrv) {
	    /* PCIe core remove from procfs CB */
	    pfs_cb = pdrv->pfs;

	    if (pfs_cb != NULL) {
	        pfs_cb->pdrv[pdrv->core.info.id] = NULL;
	    }

	    HCD_FN_RET();
	}

	/* cpci remove */
	pfs_cb = &pcie_hcd_procfs_cb;

	for (core = 0; core < MAX_NUM_PCIE_CORES; core++) {
	    if (pfs_cb->pdrv[core]) {
	        /* There are some cores still running */
	        HCD_ERROR("Core [%d] unexpected during procfs de-init\n",
	            core);
	    }
	}

	if (pfs_cb->status != NULL) {
	    remove_proc_entry("status", pfs_cb->dir);
	    pfs_cb->status = NULL;
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
 *    data ... pointer to msi control block
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
 *    irq_data  ... pointer to msi control block
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
 *    data    ... pointer to msi control block
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
 *    args    ... platform allocation arguments
 *
 *   Description:
 *    Allocate required number of msi interrupts from the domain
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

	if (nr_irqs > msi->nr) {
	    HCD_ERROR("requested irq's [%d] exceeds max [%d]\n", nr_irqs, msi->nr);
	    HCD_FN_RET_VAL(-EINVAL);
	}

	mutex_lock(&msi->lock);
	hwirq = bitmap_find_free_region(msi->used, msi->nr, order_base_2(nr_irqs));
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

	if (nr_irqs > msi->nr) {
	    HCD_ERROR("requested irq's [%d] exceeds max [%d]\n", nr_irqs, msi->nr);
	    HCD_FN_RET();
	}

	bitmap_release_region(msi->used, d->hwirq,  order_base_2(nr_irqs));
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

	HCD_INFO("created top_domain 0x%px\n", msi->top_domain);

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

	HCD_INFO("created domain 0x%px\n", msi->domain);

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
 *    data ... pointer to HCD (given through request_irq())
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
	    if (test_bit(bit, msi->used)) {
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

	HCD_INFO("Using irq=%d for PCIE-MSI interrupts\n", msi->irq);

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
	    HCD_ERROR("Unable to allocate memory for MSI\n");
	    HCD_FN_RET_VAL(-ENOMEM);
	}

	HCD_INFO("Allocated msi [0x%px] \n", msi);

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
	 * MSI should have been updated by HC with the number of interrupts
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
 *    pdrv ... pointer to PCIe core HCD data structure
 *
 *   Description:
 *    Setup PCIe core bus error logging
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
 *    pdrv ... pointer to PCIe core HCD data structure
 *
 *   Description:
 *    Setup PCIe core bus error logging
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

	HCD_LOG("Core [%d] Disabled PCIE/UBUS Error Interrupts\n",
	    pdrv->core.info.id);

	HCD_FN_RET();
}

/*
 * Function pcie_hc_link_up (pdrv)
 *
 *   Parameters:
 *    pdrv ... pointer to PCIe core HCD data structure
 *
 *   Description:
 *    Check PCIe link is UP or down
 *
 *   Return: 0 on linkup, -ve on link down/failure
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

	    link_status = pcie_hcd_read_reg(phc, PCIE_LNKCTLSTA);
	    link_status >>= 16;

	    HCD_LOG("Core [%d] Link UP - [%d] lanes, [GEN%d] speed\n",
	        pdrv->core.info.id, PCIE_LNKSTA_LNK_WIDTH(link_status),
	        PCIE_LNKSTA_LNK_SPEED(link_status));
	} else {
	    err = -ENODEV;
	}

	HCD_FN_RET_VAL(err);
}

/*
 * Function pcie_hc_config_core_final (pdrv)
 *
 *   Parameters:
 *    pdev ... pointer to PCIe platform device
 *    pdrv ... pointer to PCIe core HCD data structure
 *
 *   Description:
 *    Initialize the host controller post enumeration. Retrieve and configure
 *    End Point BAR4 to be used as fastpath out address.
 *
 *   Return: 0 on success, -ve on failure
 */
static int pcie_hc_config_core_final(struct pcie_hcd *pdrv)
{
	HCD_FN_ENT();

	if (pdrv->core.plt_cfg->config_core_post) {
	    pdrv->core.plt_cfg->config_core_post(&pdrv->core);
	}

	HCD_FN_RET_VAL(0);
}

/*
 * Function pcie_hc_unconfig_core (pdrv)
 *
 *   Parameters:
 *    pdrv ... pointer to PCIe core HCD data structure
 *
 *   Description:
 *    De-configure the HC core
 *
 *   Return: None
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
 *    pdrv ... pointer to PCIe core HCD data structure
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
 *    pdrv ... pointer to PCIe core HCD data structure
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
 *    pdrv ... pointer to PCIe core HCD data structure
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
 *    pdrv ... pointer to PCIe core HCD data structure
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
	        HCD_ERROR("Core [%d] reg base mapping fail: [%ld]\n",
	            phc->info.id, PTR_ERR(phc->info.base));
	        HCD_FN_RET_VAL(PTR_ERR(phc->info.base));
	    }
	}

	/* Initialize misc PCIe control base if present */
	phc->info.misc = pcie_hcd_cpci_get()->misc.base;

	HCD_INFO("Core [%d] mapped reg base [0x%px]\n", pdev->id,
	    phc->info.base);

	reg_data = pcie_hcd_read_reg(phc, PCI_VENDOR_ID);
	HCD_INFO("Core [%d] Vendor [0x%04x] device [0x%04x]\n",
	    pdev->id, (reg_data & 0xFFFF), ((reg_data >> 16) & 0xFFFF));

	if (reg_data == 0xdeaddead) {
	    HCD_ERROR("Core [%d] access returned invalid value 0x%x\n",
	        pdev->id, reg_data);
	    HCD_FN_RET_VAL(-ENODEV);
	}

	reg_data = pcie_hcd_read_reg(phc, PCI_CLASS_REVISION);
	HCD_INFO("Core [%d] Rev [0x%02x] Class [0x%06x]\n",
	    pdev->id, (reg_data & 0xFF), ((reg_data >> 8) & 0xFFFFFF));

	phc->info.rev = pcie_hcd_read_reg(phc, phc->cfg.core_rev_offset);
	phc->info.rev &= phc->cfg.core_rev_mask;

	/* Get the Link Width and speed gen configuration from the core */
	reg_data = pcie_hcd_read_reg(phc, PCIE_LNKCAP);
	phc->info.gen = PCIE_LNKCAP_LNK_SPEED(reg_data);
	link_width = PCIE_LNKCAP_LNK_WIDTH(reg_data);
	if (!phc->res.link_width) phc->res.link_width = link_width;

	HCD_LOG("found Port [%d] GEN%d Core Rev [%x.%02x] with %d Lanes\n",
	    pdev->id, phc->info.gen, ((phc->info.rev >> 8) & 0xFF),
	    (phc->info.rev & 0xFF), link_width);

	/* Set phy power state */
	pcie_hc_apon_pwrphy(pdrv);

	/* Setup PCIe core revision based initialization in the HC */
	if (phc->plt_cfg->setup_rev) {
	    int err;

	    if ((err = phc->plt_cfg->setup_rev(phc)) != 0) {
	        HCD_FN_RET_VAL(err);
	    }
	}

	HCD_FN_RET_VAL(0);
}

/*
 * Function pcie_hc_remove_access (pdrv)
 *
 *
 *   Parameters:
 *    pdrv ... pointer to PCIe core HCD data structure
 *
 *   Description:
 *    Remove/unmap HC core register access
 *
 *   Return: 0 on success, -ve on failure
 */
static void pcie_hc_remove_access(struct pcie_hcd *pdrv)
{
	struct device *dev;
	struct pcie_hc_core     *phc = NULL;

	HCD_FN_ENT();

	if (pdrv == NULL) {
	    HCD_FN_RET();
	}

	phc = &(pdrv->core);
	dev = &(phc->pdev->dev);

	if (pdrv->core.info.base && phc->res.base.start && phc->res.base.flags) {
	    devm_iounmap(dev, phc->info.base);
	    HCD_INFO("Core [%d] unmap reg base [0x%px]\n", phc->info.id,
	        phc->info.base);
	    phc->info.base = NULL;
	}

	HCD_FN_RET();
}

/*
 * Function pcie_hc_init_core (pdrv)
 *
 *   Parameters:
 *    pdev ... pointer to PCIe platform device
 *    pdrv ... pointer to PCIe core HCD data structure
 *
 *   Description:
 *    Initialize the HCD resource entries to default values. Currently supported
 *    resources
 *    - PCIe core base, memory window, PCI bus range
 *
 *   Return: 0 on success, -ve on failure
 */
static int pcie_hc_init_core(struct platform_device *pdev,
	struct pcie_hcd *pdrv)
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
	    HCD_ERROR("Core [%d] failed to initialize HC\n", pdev->id);
	    HCD_FN_RET_VAL(err);
	}

	/* Update configuration with default parameters */
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
	phc->cfg.prioqueue = HCD_HC_CORE_CFG(pcie_prioqueue, phc->info.id);

	if (PHC_CORE(phc->info.id)) {
	    int lane;
	    for (lane = 0; lane < HCD_PHYLANE_MAX; lane++) {
	        phc->cfg.g3_eq_presets[lane] = HCD_HC_CORE_ARRAY_CFG(
	            pcie_gen3_eq_presets, phc->info.id, lane);
	        if (phc->cfg.g3_eq_presets[lane] == 0x00000000) {
	            /* Use lane#0 setting for all, if only lane#0 is set */
	            phc->cfg.g3_eq_presets[lane] = phc->cfg.g3_eq_presets[0];
	        }
	    }
	}

	pcie_hc_dump_core_cfg(pdrv);

	HCD_FN_RET_VAL(err);
}

/*
 * Function pcie_hc_free_core (pdrv)
 *
 *   Parameters:
 *    pdrv ... pointer to PCIe core HCD data structure
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
 *    pdrv ... pointer to PCIe core HCD data structure
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

	/* Update core configuration from common nv parameters */
	phc->cfg.ssc = HCD_GET_CMMN_SETTING(pcie_ssc_cfg, core) ? true : false;
	phc->cfg.pllid = HCD_GET_CMMN_SETTING(pcie_pllclkid, core);
	phc->cfg.plladj = HCD_GET_CMMN_SETTING(pcie_pllclkadj, core);
	phc->cfg.barmask = pcie_barmask;
	phc->cfg.fastpath = pcie_fastpath & 0xFF;

	/* Get updated nvram parameters after module load */
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
	HCD_READ_CORE_SETTING(pcie_prioqueue);

	if (PHC_CORE(core)) {
	    HCD_READ_CORE_ARRAY_SETTING(pcie_gen3_eq_presets, core);
	}

	/* Update core configuration from core nv parameters */
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
	phc->cfg.linkupretries = HCD_GET_CORE_SETTING(pcie_linkupretries, core);
	phc->cfg.phyrxfilter = HCD_GET_CORE_SETTING(pcie_phyrxfilter, core);
	phc->cfg.epuperstb = HCD_GET_CORE_SETTING(pcie_epuperstb, core);
	phc->cfg.prioqueue = HCD_GET_CORE_SETTING(pcie_prioqueue, core);

	if (PHC_CORE(core)) {
	    u8 lane;
	    for (lane = 0; lane < HCD_PHYLANE_MAX; lane++) {
	        phc->cfg.g3_eq_presets[lane] = HCD_GET_CORE_ARRAY_SETTING(
	            pcie_gen3_eq_presets, core, lane);
	        if (phc->cfg.g3_eq_presets[lane] == 0x00000000) {
	            /* Use lane#0 setting for all, if only lane#0 is set */
	            phc->cfg.g3_eq_presets[lane] = phc->cfg.g3_eq_presets[0];
	        }
	    }
	}

	pcie_hc_dump_core_cfg(pdrv);

	return 0;
}

/*
 * Function pcie_hc_dump_core_cfg (pdrv)
 *
 *   Parameters:
 *    pdrv ... pointer to PCIe core HCD data structure
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

	HCD_INFO("    speed [%d] apon [%d] phypwrmode [%d] errlog [%d] msi [%d] "
	    "barmask [%d]\n",
	    phc->cfg.speed, phc->cfg.apon, phc->cfg.phypwrmode,
	    phc->cfg.errlog, phc->cfg.msi, phc->cfg.barmask);

	HCD_INFO("    lanes [%d] linkupretries [%d] phyrxfilter [%d] "
	    "epuperstb [%d] prioqueue [%d] fastpath [%d]\n",
	    phc->res.link_width, phc->cfg.linkupretries, phc->cfg.phyrxfilter,
	    phc->cfg.epuperstb, phc->cfg.prioqueue, phc->cfg.fastpath);

	return;
}

/*
 * Function pcie_hc_apon_pwrup (pdrv)
 *
 *   Parameters:
 *    pdrv ... pointer to PCIe core HCD data structure
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

	    if (apon != HCD_APON_OFF) {
	        /* Default or Always ON or OFF only PHY */
	        int rc;

	        rc = pdrv->core.plt_cfg->set_power(phc, devidx, true, HCD_DEV_BOTH);
	        if (rc == 0) {
	            pdrv->apon.powered[devidx] = true;
	            HCD_INFO("Core [%d] dev [%d] powered up\n",
	                pdrv->core.info.id, devidx);
	            devs++;
	        } else {
	            pdrv->apon.powered[devidx] = false;
	            HCD_ERROR("Core [%d] dev [%d] failed to powered up\n",
	                pdrv->core.info.id, devidx);
	            devs = -ENODEV;
	        }
	    } else {
	        /* Always off */
	        pdrv->core.plt_cfg->set_power(phc, devidx, false, HCD_DEV_BOTH);
	        HCD_LOG("Core [%d] dev [%d] powered down due to apon [%d]\n",
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
 *    pdrv ... pointer to PCIe core HCD data structure
 *
 *   Description:
 *    Power down the HC core devices if not configured for always on
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
	        struct pcie_hcd_bus_fpam *fpam = &(pcie_hcd_cpci_get()->fpam);
	        int pwr_devs = HCD_DEV_BOTH;

	        if ((pdrv->apon.cfg[devidx] == HCD_APON_OFF_WITH_DOMAIN) ||
	            (fpam->master == phc->info.id)) {
	            /* Only power down RC PHY and keep RC Digital ON */
	            pwr_devs &= (~HCD_DEV_RC);
	            pwr_devs |= (HCD_DEV_RC_PHY);
	            HCD_LOG("Core [%d] skip power down RC digital\n", phc->info.id);
	        }

	        phc->plt_cfg->set_power(phc, devidx, false, pwr_devs);
	        pdrv->apon.powered[devidx] = false;
	        HCD_LOG("Core [%d] dev [%d] powered down %s\n", phc->info.id,
	            devidx, (pwr_devs & HCD_DEV_RC) ? "Fully" : "Partially");
	    }
	}

	HCD_FN_RET_VAL(0);
}

/*
 * Function pcie_hc_apon_pwrphy (pdrv)
 *
 *
 *   Parameters:
 *    pdrv ... Pointer to PCIe core HCD data structure
 *
 *   Description:
 *    Power down the HC core devices if not configured for always on
 *
 *   Return: 0 on success, -ve on failure
 */
static int pcie_hc_apon_pwrphy(struct pcie_hcd *pdrv)
{
	struct pcie_hc_core *phc = &pdrv->core;
	u8 devidx;

	HCD_FN_ENT();

	for (devidx = 0; devidx < phc->info.devs; devidx++) {
	    if (pdrv->apon.cfg[devidx] == HCD_APON_OFF_WITH_DOMAIN) {
	        phc->plt_cfg->set_power(phc, devidx, false,
	            (HCD_DEV_RC_PHY | HCD_DEV_EP));
	        HCD_LOG("Core [%d] dev [%d] PHY powered down due to apon\r\n",
	            phc->info.id, devidx);
	    } else {
	        phc->plt_cfg->set_power(phc, devidx, true, HCD_DEV_RC_PHY);
	    }
	}

	HCD_FN_RET_VAL(0);
}

/*
 * Function pcie_hcd_set_regulators(pdrv, devidx, up)
 *
 *   Parameters:
 *    pdrv ... pointer to PCIe core HCD data structure
 *    devidx ... core device index (not used)
 *    up     ... flag to specify power up/down
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
	                /* Make sure enough delay before asserting PCIe reset */
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
 *    pdrv ... pointer to PCIe core HCD data structure
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
 *    pdrv ... pointer to PCIe core HCD data structure
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
 *    up     ... flag to specify power up/down
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
 *    bus   ... pointer to PCIe bus
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

	/* endpoint device is invalid access if powered down with domain */
	if ((phc->pdrv->apon.flags != APON_F_OFF_W_DOM) ||
	    (bus->number == 0)) {
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
 *    bus   ... pointer to PCIe bus
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

	/* endpoint device is invalid access if powered down with domain */
	if ((phc->pdrv->apon.flags != APON_F_OFF_W_DOM) ||
	    (bus->number == 0)) {
	    err = phc->plt_cfg->pci_fops->write(bus, devfn, where, size, val);
	}

	HCD_FN_RET_VAL(err);
}

/*
 * Function pcie_hcd_map_bus (bus, devfn, where)
 *
 *   Parameters:
 *    bus   ... pointer to PCIe bus
 *    devfn ... device and function number
 *    where ... offset from the base
 *
 *   Description:
 *    Map PCIe HC core config space address
 *
 *   Return: valid mapped memory on success, NULL on failure
 */
static void __iomem *pcie_hcd_map_bus(struct pci_bus *bus,
	unsigned int devfn, int where)
{
	struct pcie_hc_core *phc = pcie_hcd_get_hccore_from_bus(bus);
	void __iomem *addr = NULL;

	HCD_FN_ENT();

	/* invalid access if powered down with domain */
	if (phc->pdrv->apon.flags != APON_F_OFF) {
	    addr = phc->plt_cfg->pci_fops->map_bus(bus, devfn, where);
	}

	HCD_FN_RET_VAL(addr);
}

/*
 * Function pcie_hcd_map_bus_ecam (bus, devfn, where)
 *
 *   Parameters:
 *    bus   ... pointer to PCIe bus
 *    devfn ... device and function number
 *    where ... offset from the base
 *
 *   Description:
 *    Map PCIe HC core config space address
 *
 *   Return: valid mapped memory on success, NULL on failure
 */
static void __iomem *pcie_hcd_map_bus_ecam(struct pci_bus *bus,
	unsigned int devfn, int where)
{
	struct pci_config_window *cfg = (struct pci_config_window*)bus->sysdata;
	struct pcie_hc_core *phc = pcie_hcd_get_hccore_from_bus(bus);
	void __iomem *addr = NULL;

	HCD_FN_ENT();

	if (!cfg)  return NULL;

	/* invalid access if powered down with domain */
	if (phc->pdrv->apon.flags != APON_F_OFF) {
	    /* This check is still needed for the initial versions of ECAM SoC's */
	    if ((bus->number == PCIE_HCD_ROOT_BUSNUM) && (PCI_SLOT(devfn) != 0)) {
	        goto done;
	    }
	    /* Use Default ecam addressing scheme */
	    addr = pci_ecam_map_bus(bus, devfn, where);
	}

done:
	HCD_FN_RET_VAL(addr);
}

/*
 * Function pcie_hcd_map_irq (pcidev, slot, pin)
 *
 *   Parameters:
 *    pcidev ... pointer to PCI device data structure
 *    slot   ... PCI slot (not used)
 *    pin    ... pin number (not used)
 *
 *   Description:
 *    Get the PCIe core irq number.
 *
 *   Return: PCIe core irq number
 */
int pcie_hcd_map_irq(const struct pci_dev *pcidev, u8 slot, u8 pin)
{
	int irq = -1;
	int  (*hc_irq_map)(const struct pci_dev *, u8, u8);
	struct pcie_hc_core *phc = pcie_hcd_get_hccore_from_bus(pcidev->bus);

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
 *    pdrv      ... pointer to PCIe core HCD data structure
 *    resources ... PCIe core resources
 *
 *   Description:
 *    Dummy as window resources are mapped by pci stack
 *
 *   Return: 0 on success, -ve on failure
 */
static int pcie_hcd_setup_resources(struct pcie_hcd *pdrv,
	struct list_head *resources)
{
	HCD_FN_ENT();
	HCD_FN_RET_VAL(0);
}

/*
 * Function pcie_hcd_unmap_resources (pdrv)
 *
 *   Parameters:
 *    pdrv ... pointer to PCIe core HCD data structure
 *
 *   Description:
 *    Dummy as unmap resources are done by pci stack
 *
 *   Return: 0 on success, -ve on failure
 */
static void pcie_hcd_unmap_resources(struct pcie_hcd *pdrv)
{
	HCD_FN_ENT();
	HCD_FN_RET();
}

/*
 * Function pcie_hcd_parse_resources (pdrv, resources)
 *
 *   Parameters:
 *    pdrv ... Pointer to PCIe core HCD data structure
 *    np   ... Pointer to device tree node
 *
 *   Description:
 *    Get PCIe memory and bus resources either from DT or from already parsed
 *    PCIe bridge
 *
 *   Return: 0 on success, -ve on failure
 */
static int pcie_hcd_parse_resources(struct pcie_hcd *pdrv,
	struct device_node *np)
{
	int win = OWIN0;
	int err = 0;
	struct pcie_hc_core_res *pres = &pdrv->core.res;
	struct resource_entry *entry;
	struct pci_host_bridge *bridge = pci_host_bridge_from_priv(pdrv);

	HCD_FN_ENT();

	/*
	 * Outgoing memory window and bus range are already parsed by
	 * bridge, so get it from bridge windows
	 */
	resource_list_for_each_entry(entry, &bridge->windows) {
	    struct resource *res = entry->res;

	    switch (resource_type(res)) {
	        case IORESOURCE_BUS:
	            memcpy(&pres->bus_range, res, sizeof(struct resource));
	            break;

	        case IORESOURCE_MEM:
	            if (win >= MAX_NUM_OUTGOING_WINDOWS) {
	                HCD_ERROR("Core [%d] outgoing windows [%d] exceeds"
	                    " max [%d]\n", pdrv->core.info.id, win,
	                    MAX_NUM_OUTGOING_WINDOWS);
	                break;
	            }

	            memcpy(&pres->owin[win], res, sizeof(struct resource));
	            win++;
	            break;

	        default:
	            HCD_ERROR("Core [%d] unknown resource type [%ld]\n",
	                pdrv->core.info.id, resource_type(res));
	            break;
	    }
	}
	err = (win) ? 0 : -EINVAL;

	HCD_FN_RET_VAL(err);
}

/*
 * Function pcie_hcd_parse_dt (pdrv)
 *
 *   Parameters:
 *    pdrv ... pointer to PCIe core HCD data structure
 *
 *   Description:
 *    Parse PCIe core HCD device tree entries. Currently supported resources
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

	    /* PCIe core optional ECAM base */
	    err = of_address_to_resource(np, 1, &pres->ecam);
	    HCD_WARN_ON_DT_ERROR("reg (ecam base)", err);

	    /* Use generic ecam operations except mapping */
	    memcpy(&pcie_hcd_ops_ecam, &pci_generic_ecam_ops,
	        sizeof(struct pci_ecam_ops));
	    pcie_hcd_ops_ecam.pci_ops.map_bus = pcie_hcd_map_bus_ecam;

	    /* Parse and get outgoing memory windows and bus ranges */
	    err = pcie_hcd_parse_resources(pdrv, np);
	    if (err < 0) {
	        HCD_FN_RET_VAL(err);
	    }

	    if (HCD_USE_DT_ENTRY(pdev->id) && (pres->irq == 0)) {
	        pres->irq = irq_of_parse_and_map(np, 0);
	        err = (pres->irq > 0) ? 0 : 1;
	        HCD_FAIL_ON_DT_ERROR("interrupt", err);
	    }

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

	    /* PCIe phy serdes flags */
	    err = of_property_read_u32(np, "brcm,serdes", &dt_val);
	    if (err == 0) {
	        pdrv->core.cfg.serdes = dt_val;
	    }
	    HCD_WARN_ON_DT_ERROR("brcm,serdes", err);

	    /* PCIe Priority Queues Setting */
	    err = of_property_read_u32(np, "brcm,prioqueue", &dt_val);
	    if (err == 0) {
	        HCD_HC_SET_CORE_CFG(pcie_prioqueue, pdev->id, dt_val);
	    }
	    HCD_WARN_ON_DT_ERROR("brcm,prioqueue", err);

	    /*
	     * PCIe gen3 link equalization presets of the lanes with a
	     * minimum of 1 and maximum of 2 lanes
	     */
	    {
	        u16 dt_val_array[HCD_PHYLANE_MAX] = {0};
	        int lane;

	        err = of_property_read_variable_u16_array(np,
	            "brcm,gen3-eq-presets", dt_val_array, 1, HCD_PHYLANE_MAX);
	        if (err == 1) {
	            /* Single value for all lanes */
	            for (lane = 1; lane < HCD_PHYLANE_MAX; lane++) {
	                dt_val_array[lane] = dt_val_array[0];
	            }
	        }

	        /* Update core configuration with DT values */
	        if (err > 0) {
	            for (lane = 0; lane < HCD_PHYLANE_MAX; lane++) {
	                HCD_HC_SET_CORE_ARRAY_CFG(pcie_gen3_eq_presets, pdev->id,
	                    lane, dt_val_array[lane]);
	            }
	            err = 0;
	        }
	    }
	    HCD_WARN_ON_DT_ERROR("brcm,gen3-eq-presets", err);

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

	HCD_INFO("Core [%d] regs-base: %pR\n", pdev->id, &pres->base);
	if (hcd_resource_len(&pres->ecam))
	    HCD_LOG("Core [%d] ECAM: %pR\n", pdev->id, &pres->ecam);
	HCD_INFO("Core [%d] Interrupt [%d]\n", pdev->id, pres->irq);
	HCD_INFO("Core [%d] bus_range: %pR\n", pdev->id, &pres->bus_range);
	for (win = OWIN0; win < MAX_NUM_OUTGOING_WINDOWS; win++) {
	    if (hcd_resource_len(pres->owin + win)) {
	        HCD_INFO("Core [%d] owin[%d]: %pR\n", pdev->id, win,
	           (pres->owin + win));
	    }
	}

	pcie_hc_dump_core_cfg(pdrv);

	HCD_FN_RET_VAL(0);
}

/*
 * Function pcie_hcd_cpci_probe (pdev)
 *
 *   Parameters:
 *    pdev ... pointer to PCIe platform device data structure
 *
 *   Description:
 *    read and initialize the PCIe device tree entries common for all cores
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
	    /* Not a common PCIe device tree entry */
	    err = -1;
	    goto done;
	}

	memset(cpci, 0, sizeof(struct pcie_hcd_cpci));

	HCD_INFO("common core settings\n");

	/* MISC PCIe registers base */
	err = of_address_to_resource(np, 0, &cpci->misc.res);
	if ((cpci->misc.res.start != 0) && (cpci->misc.base == NULL)) {
	    cpci->misc.base = devm_ioremap_resource(&pdev->dev, &cpci->misc.res);
	    if (IS_ERR(cpci->misc.base)) {
	        HCD_ERROR("cpci MISC reg base mapping fail: [%ld]\n",
	            PTR_ERR(cpci->misc.base));
	        HCD_FN_RET_VAL(PTR_ERR(cpci->misc.base));
	    }
	}
	HCD_INFO("Mapped misc reg base [0x%px]\n", cpci->misc.base);

	/* read common PCIe device configuration */
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

	/* PCIe Fastpath configuration */
	err = of_property_read_u32(np, "brcm,fastpath", &dt_val);
	if (err == 0) {
	    pcie_fastpath = dt_val;
	}
	HCD_WARN_ON_DT_ERROR("brcm,fastpath", err);

	/* Update settings from NVRAM (kernel, environment) if valid */
	HCD_UPDATE_CMMN_SETTING(pcie_pllclkid);
	HCD_UPDATE_CMMN_SETTING(pcie_ssc_cfg);
	HCD_UPDATE_CMMN_SETTING(pcie_pllclkadj);
	HCD_UPDATE_CMMN_SETTING(pcie_barmask);
	HCD_UPDATE_CMMN_SETTING(pcie_scan_order);
	HCD_UPDATE_CMMN_SETTING(pcie_fastpath);

	pcie_hcd_init_deferred_scan();
	pcie_hcd_init_fastpath(pcie_fastpath);

	/* Disable SSC if PLL Clock frequency adjustment is enabled */
	if (pcie_pllclkadj && pcie_ssc_cfg) {
	    HCD_WARN("Force disable SSC on PLL Frequency Adjustment enable\n");
	    /* disable for port#1,2,3. Port#0 setting unchanged */
	    pcie_ssc_cfg &= 0x000F;
	}

	pcie_hcd_procfs_init(NULL);

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
 *    pdev ... pointer to PCIe platform device data structure
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

	pcie_hcd_procfs_deinit(NULL);

	if ((cpci->misc.res.start != 0) && (cpci->misc.base != NULL)) {
	    devm_iounmap(&pdev->dev, cpci->misc.base);
	}

	HCD_FN_RET_VAL(0);
}

/*
 * Function pcie_hcd_scan_bus (pdrv)
 *
 *   Parameters:
 *    pdrv ... pointer to PCIe driver HCD data structure
 *
 *   Description:
 *    PCIe HCD driver bus scan. Called for each instance of the PCIe core.
 *    start the PCI bus and enumerate PCI devices
 *
 *   Return: 0 on success, -ve on failure
 */
static int pcie_hcd_scan_bus(struct pcie_hcd *pdrv)
{
	struct pci_host_bridge *bridge = pci_host_bridge_from_priv(pdrv);
	struct pcie_hc_core_res *pres = NULL;
	int err;


	pres = &pdrv->core.res;
	if (hcd_resource_len(&pres->ecam) > 0) {
	    pdrv->ecam_cfg = pci_ecam_create(&(pdrv->core.pdev->dev), &pres->ecam,
	        &pres->bus_range, &pcie_hcd_ops_ecam);

	    if (IS_ERR(pdrv->ecam_cfg)) {
	        HCD_ERROR("Core [%d] failed to create ECAM, rc (%ld) \n",
	            pdrv->core.info.id, PTR_ERR(pdrv->ecam_cfg));
	    }
	}

	if (pdrv->ecam_cfg) {
	    HCD_LOG("Core [%d] created ECAM %pR\n", pdrv->core.info.id,
	        &pres->ecam);

	    pdrv->ecam_cfg->priv = (void*)&(pdrv->core);
	    bridge->ops = (struct pci_ops *)&pcie_hcd_ops_ecam.pci_ops;
	    bridge->sysdata = pdrv->ecam_cfg;
	} else {
	    bridge->sysdata = &pdrv->core;
	    if (pdrv->core.plt_cfg->pci_fops->map_bus) {
	        bridge->ops = &pcie_hcd_ops_generic;
	    } else {
	        bridge->ops = &pcie_hcd_ops_specific;
	    }
	}

	bridge->map_irq = pcie_hcd_map_irq;
	err = pci_host_probe(bridge);
	if (err < 0) {
	    HCD_ERROR("Core [%d] failed to probe host, rc (%d)\n",
	        pdrv->core.info.id, err);
	    pcie_hcd_remove(pdrv->core.pdev);
	}

	return err;
}

/*
 * Function pcie_hcd_probe (pdev)
 *
 *   Parameters:
 *    pdev ... pointer to PCIe platform device data structure
 *
 *   Description:
 *    PCIe HCD driver probe. Called for each instance of the PCIe core.
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
	struct pci_host_bridge *bridge;
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
	        HCD_ERROR("Unable to get coreid from device tree\n");
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

	HCD_INFO("Core [%d] probe\n", core);

	if (core >= NUM_CORES) {
	    HCD_ERROR("Core [%d] exceeds max Cores [%d]\n", core, NUM_CORES);
	    HCD_FN_RET_VAL(-ENODEV);
	}

	if (pcie_hcd_deferred_scan_active(pdev)) {
	    /* Add this device to the probed device list */
	    pcie_hcd_deferred_scan_add(pdev);
	}

	if (!pcie_hcd_port_enabled(core, np)) {
	    HCD_ERROR("Core [%d] not enabled\n", core);
	    err = -ENODEV;
	    goto error;
	}

	if (pcie_hcd_get_port_mode(core, np) == 0) {
	    HCD_ERROR("Core [%d] Not in RC Mode\n", core);
	    err = -ENODEV;
	    goto error;
	}

	/* Allocate bridge with HCD control block */
	bridge = devm_pci_alloc_host_bridge(&pdev->dev, sizeof(*pdrv));
	if (!bridge) {
	    HCD_ERROR("Core [%d] failed to allocate bridge\n", core);
	    err = -ENOMEM;
	    goto error;
	}

	pdrv = pci_host_bridge_priv(bridge);
	HCD_INFO("Core [%d] Allocated [0x%px] hcd\n", core, pdrv);

	/* Initialize  HCD elements */
	platform_set_drvdata(pdev, pdrv);
	INIT_LIST_HEAD(&pdrv->pwrsupplies.node);

	/* Initialize  core resource element values for no device tree based
	 * legacy drivers
	 */
	err = pcie_hc_init_core(pdev, pdrv);
	if (err < 0) {
	    HCD_ERROR("Core [%d] failed to initialize HC\n", core);
	    err =  -EINVAL;
	    goto error;
	}

	/* Update core resource elements  (DT based) */
	err = pcie_hcd_parse_dt(pdrv);
	if (err < 0) {
	    HCD_ERROR("Core [%d] failed to update dt entries\n", core);
	    err =  -EINVAL;
	    goto error;
	}

	/* Update core setting from valid nvram settings */
	err = pcie_hc_update_core_cfg(pdrv);
	if (err < 0) {
	    HCD_ERROR("Core [%d] failed to update dt entries\n", core);
	    err =  -EINVAL;
	    goto error;
	}

	/*
	 * Power on the port devices,
	 * if Configured, skip port enumeration and power off the port device
	 */
	active = pcie_hc_apon_pwrup(pdrv);
	if (active < 0) {
	    HCD_LOG("Core [%d] skip due to apon setting [%d]\n", core,
	        pdrv->core.cfg.apon);
	    err = active;
	    goto error;
	}

	/* Setup PCIe core memory window */
	err = pcie_hcd_setup_resources(pdrv, &res);
	if (err < 0) {
	    HCD_ERROR("Core [%d] failed to setup owin resource, err [%d]\n",
	        core, err);
	    err =  -ENOMEM;
	    goto error;
	}

	/* setup PCIe Core registers for access to PCIe core */
	err = pcie_hc_setup_access(pdrv);
	if (err < 0) {
	    HCD_ERROR("Core [%d] failed to setup regs, err [%d]\n", core,
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
	    HCD_ERROR("Core [%d] link is DOWN\n", core);
	    /* Fastpath master processing for link down cores */
	    err = pcie_hcd_lnkdn_fastpath(pdrv);
	    if (err < 0) {
	        goto error;
	    }
	    /* Continue to initialize even though no link */
	}

	/* Got all core resources, Now configure the PCIe core */
	err = pcie_hc_config_core(pdrv);
	if (err < 0) {
	    HCD_ERROR("Core [%d] failed to setup hw, err [%d]\n", core, err);
	    err =  -ENODEV;
	    goto error;
	}

	/* Now initialize the PCIe core error logging */
	if (pdrv->core.cfg.errlog) {
	    err = pcie_hcd_errlog_enable(pdrv);
	    if (err < 0) {
	        HCD_ERROR("Core [%d] failed to setup error logging, err [%d]\n",
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

	{
	    bool scan = ((err == 0) && (active)) ? true : false;

	    if (pcie_hcd_deferred_scan_active(pdev)) {
	        /* trigger deferred scan if enabled */
	        pcie_hcd_deferred_scan_upd(pdev,
	            (scan == true) ? -EHCD_SCAN_DEFER: -ENODEV);
	        pcie_hcd_deferred_scan_trigger(pdev);
	    } else if (scan == true) {
	        err = pcie_hcd_scan_bus(pdrv);
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
 *    pdev ... pointer to PCIe platform device data structure
 *
 *   Description:
 *    PCIe HCD driver remove Free the resources and power down the PCIe core
 *
 *   Return: 0 on success, -ve on failure
 */
static int  pcie_hcd_remove(struct platform_device *pdev)
{
	struct pcie_hcd *pdrv = platform_get_drvdata(pdev);
	struct pci_host_bridge *bridge;

	HCD_FN_ENT();

	if (!pdrv) {
	    pcie_hcd_cpci_remove(pdev);
	    HCD_FN_RET_VAL(0);
	}

	bridge = pci_host_bridge_from_priv(pdrv);
	if (bridge->bus) {
	    pci_stop_root_bus(bridge->bus);
	    pci_remove_root_bus(bridge->bus);
	}

	if (pdrv->ecam_cfg) {
	    pci_ecam_free(pdrv->ecam_cfg);
	}

	pcie_hcd_procfs_deinit(pdrv);

	if (IS_ENABLED(CONFIG_BCM_PCI_MSI)) {
	    pcie_hcd_msi_disable(pdrv);
	}

	pcie_hcd_errlog_disable(pdrv);

	pcie_hc_unconfig_core(pdrv);

	/* power off ports based on apon setting */
	pcie_hc_apon_pwrdn(pdrv);

	pcie_hcd_free_regulators(pdrv);

	pcie_hcd_unmap_resources(pdrv);

	pcie_hc_remove_access(pdrv);

	pcie_hc_free_core(pdrv);

	platform_set_drvdata(pdev, NULL);

	HCD_FN_RET_VAL(0);
}

extern bool xport_init_done;

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
 *    PCIe HCD driver init, register the driver from platform list including
 *    misc driver
 *    This in turn should call our probe()
 *
 *   Return: 0 on success, -ve on failure
 */
static int __init pcie_hcd_init(void)
{
	int ret;
	int index;
	struct platform_driver *phcd_drv = &pcie_hcd_driver;

	HCD_FN_ENT();

#ifdef CONFIG_BCM968880
	while (!xport_init_done) {
	    msleep(100);
	}
	msleep(100);
#endif

	printk("Broadcom PCIe Host Controller Driver (impl%d)\n",
	    CONFIG_BCM_PCIE_HCD_IMPL);

#if defined(MODULE)
	if ((ret = pcie_hcd_nvram_init()) != 0) {
	    HCD_ERROR("Failed to initialize nvram parameters\n");
	    goto done;
	}

	/* Need to initialize platform dev for loadable kernel module */
	if ((ret = pcie_hcd_plt_init()) != 0) {
	    HCD_ERROR("Failed to initialize platform device\n");
	    goto done;
	}
#endif /* MODULE */

	/* Register common HCD driver */
	HCD_INFO("register [%s] driver with platform driver\n",
	    phcd_drv->driver.name);
	ret = platform_driver_register(phcd_drv);
	if (ret != 0) {
	    HCD_ERROR("Failed to register%s driver with platform driver\n",
	        phcd_drv->driver.name);
	    goto done;
	}

	/* Register individual HC drivers, if different from HCD driver */
	for (index = 0; index < MAX_HC_TYPE; index++) {
	    int hc = pcie_hcd_hc_enum_order[index];
	    struct platform_driver *phc_drv = hc_plt_cfg[hc].plt_drv;

	    if ((hc_plt_cfg[hc].num_cores) && (phc_drv != phcd_drv)) {

	        HCD_INFO("register [%s] driver with platform driver\n",
	            phc_drv->driver.name);
	        ret = platform_driver_register(phc_drv);
	        if (ret != 0) {
	            HCD_ERROR("Failed to register [%s] with platform driver\n",
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
 *    PCIe HCD driver exit, de register the driver from platform list including
 *    misc driver
 *    This in turn should call our remove()
 *
 *   Return: None
 */
static void __exit pcie_hcd_exit(void)
{
	struct platform_driver *phcd_drv = &pcie_hcd_driver;
	int    index;

	HCD_FN_ENT();

	/* Disable fastpath to completely power down the master core */
	pcie_hcd_init_fastpath(FASTPATH_OFF);

	/* Un-register individual HC drivers, if different from HCD driver */
	for (index = MAX_HC_TYPE-1; index >= 0; index--) {
	    int hc = pcie_hcd_hc_enum_order[index];
	    struct platform_driver *phc_drv = hc_plt_cfg[hc].plt_drv;

	    if ((hc_plt_cfg[hc].num_cores) && (phc_drv != phcd_drv)) {

	        HCD_INFO("De-register [%s] driver with platform driver\n",
	            phc_drv->driver.name);
	        platform_driver_unregister(phc_drv);
	    }
	}

	/* Un-register PCIe HCD driver */
	HCD_INFO("De-register [%s] driver with platform driver\n",
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
 *    pdrv ... pointer to PCIe platform device data structure
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
	struct pcie_hc_plt_cfg  *plt_cfg = NULL;

	HCD_FN_ENT();

	if (NUM_CORES < 0) {
	    NUM_VCORES = 0;
	    NUM_PCORES = 0;

	    hc_plt_cfg[PHC].plt_dev = &pcie_hcd_plt_dev[NUM_CORES];
	    ret = pcie_phc_plt_init(&hc_plt_cfg[PHC]);
	    if (ret < 0) {
	        HCD_ERROR("Failed to initialize HC\n");
	        HCD_FN_RET_VAL(ret);
	    }

#if defined(CONFIG_BCM_PCIE_VCD)
	    hc_plt_cfg[VHC].plt_dev = &pcie_hcd_plt_dev[NUM_CORES];
	    ret = pcie_vhc_plt_init(&hc_plt_cfg[VHC]);
	    if (ret < 0) {
	        HCD_ERROR("Failed to initialize VHC\n");
	        HCD_FN_RET_VAL(ret);
	    }
#endif /* CONFIG_BCM_PCIE_VCD */

	    for (hc = PHC; hc < MAX_HC_TYPE; hc++) {
	        plt_cfg = &hc_plt_cfg[hc];
	        if ((plt_cfg->num_cores &&
	            ((plt_cfg->plt_drv->probe == pcie_hcd_driver.probe) ||
	            (plt_cfg->plt_drv->remove == pcie_hcd_driver.remove))) &&
	            ((!plt_cfg->init_core) || (!plt_cfg->config_core) ||
	            (!plt_cfg->read_reg) || (!plt_cfg->write_reg))) {

	            HCD_ERROR("Failed to initialize platform driver fops\n");
	            HCD_ERROR("PCIe Platform HC[%d] Configuration:\n", hc);
	            HCD_ERROR("num_cores       :%d\n", plt_cfg->num_cores);
	            HCD_INFO("flags            :0x%08x\n", plt_cfg->flags);
	            HCD_ERROR("plt_dev         :0x%px\n", plt_cfg->plt_dev);
	            HCD_ERROR("plt_drv         :0x%px\n", plt_cfg->plt_drv);
	            HCD_ERROR("pci_fops        :0x%px\n", plt_cfg->pci_fops);
	            HCD_ERROR("init_core       :0x%px\n", plt_cfg->init_core);
	            HCD_ERROR("free_core       :0x%px\n", plt_cfg->free_core);
	            HCD_ERROR("read_reg        :0x%px\n", plt_cfg->read_reg);
	            HCD_ERROR("write_reg       :0x%px\n", plt_cfg->write_reg);
	            HCD_ERROR("config_core     :0x%px\n", plt_cfg->config_core);
	            HCD_ERROR("reset_core      :0x%px\n", plt_cfg->reset_core);
	            HCD_ERROR("is_linkup       :0x%px\n", plt_cfg->is_linkup);
	            HCD_ERROR("map_core_irq    :0x%px\n", plt_cfg->map_core_irq);
	            HCD_ERROR("setup_msi       :0x%px\n", plt_cfg->setup_msi);
	            HCD_ERROR("teardown_msi    :0x%px\n", plt_cfg->teardown_msi);
	            HCD_ERROR("setup_errint    :0x%px\n", plt_cfg->setup_errint);
	            HCD_ERROR("teardown_errint :0x%px\n", plt_cfg->teardown_errint);
	            HCD_FN_RET_VAL(-EINVAL);
	        } else {
	            HCD_INFO("PCIe Platform HC[%d] Configuration:\n", hc);
	            HCD_INFO("num_cores        :%d\n", plt_cfg->num_cores);
	            HCD_INFO("flags            :0x%08x\n", plt_cfg->flags);
	            HCD_INFO("plt_dev          :0x%px\n", plt_cfg->plt_dev);
	            HCD_INFO("plt_drv          :0x%px\n", plt_cfg->plt_drv);
	            HCD_INFO("pci_fops         :0x%px\n", plt_cfg->pci_fops);
	            HCD_INFO("init_core        :0x%px\n", plt_cfg->init_core);
	            HCD_INFO("free_core        :0x%px\n", plt_cfg->free_core);
	            HCD_INFO("read_reg         :0x%px\n", plt_cfg->read_reg);
	            HCD_INFO("write_reg        :0x%px\n", plt_cfg->write_reg);
	            HCD_INFO("config_core      :0x%px\n", plt_cfg->config_core);
	            HCD_INFO("reset_core       :0x%px\n", plt_cfg->reset_core);
	            HCD_INFO("is_linkup        :0x%px\n", plt_cfg->is_linkup);
	            HCD_INFO("map_core_irq     :0x%px\n", plt_cfg->map_core_irq);
	            HCD_INFO("setup_msi        :0x%px\n", plt_cfg->setup_msi);
	            HCD_INFO("teardown_msi     :0x%px\n", plt_cfg->teardown_msi);
	            HCD_INFO("setup_errint     :0x%px\n", plt_cfg->setup_errint);
	            HCD_INFO("teardown_errint  :0x%px\n", plt_cfg->teardown_errint);
	        }
	    }

	    HCD_INFO("DEF CFG: ssc 0x%x, speed  0x%x, scanorder 0x%x, apon 0x%x "
	        "barmask 0x%x\n",
	        pcie_ssc_cfg, pcie_speed_cfg, pcie_scan_order, pcie_apon,
	        pcie_barmask);
	    HCD_INFO("       : pwr 0x%x, errlog 0x%x, msi 0x%x, acc  0x%x, "
	        "pllclkid 0x%x\n",
	        pcie_phy_pwrmode, pcie_errlog, pcie_msi, pcie_acc, pcie_pllclkid);
	    HCD_INFO("       : loglevel 0x%x pllclkadj 0x%x linkupretries 0x%x "
	        "phyrxfilter 0x%x epuperstb 0x%x prioqueue 0x%x fastpath 0x%x\n",
	        pcie_log_level, pcie_pllclkadj, pcie_linkupretries,
	        pcie_phyrxfilter, pcie_epuperstb, pcie_prioqueue, pcie_fastpath);
	}

	if (HCD_USE_DT_ENTRY(0)) {
	    /* Nothing to do, DT entries will populate the devices */
	    HCD_INFO("Using PCIe DT to probe\n");
	    HCD_FN_RET_VAL(ret);
	}

	/* Register All PCIe cores as platform devices */
	for (i = 0; i < NUM_CORES; i++) {
	    core = pcie_hcd_get_boot_order_core(i);
	    if (core >= NUM_CORES) {
	        HCD_LOG("Core [%d] not in range (> %d)\n", core, NUM_CORES);
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
	        HCD_INFO("Core [%d] registered device successfully\n", core);
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
 *     PCIe platform de-initialization. De-register PCIe cores to the platform
 *     devices
 *     Generally this is done outside, but since there is no support outside,
 *     it is done here.
 *
 *   Return: None
 */
static void __exit pcie_hcd_plt_deinit(void)
{
	int i, core;
	struct platform_device  *pdev = NULL;

	HCD_FN_ENT();

	if (NUM_CORES < 0) {
	    HCD_INFO("HC configuration not initialized\n");
	    HCD_FN_RET();
	}

	if (HCD_USE_DT_ENTRY(0)) {
	    /* Nothing to do, DT entries will populate the devices */
	    HCD_INFO("Using PCIe DT to probe\n");
	    HCD_FN_RET();
	}

	/* Un register All PCIe cores as platform devices */
	for (i = 0; i < NUM_CORES; i++) {
	    core = pcie_hcd_get_boot_order_core(i);
	    if (core >= NUM_CORES) {
	        HCD_LOG("Core [%d] not in range (> %d)\n", core, NUM_CORES);
	        continue;
	    }

	    pdev = &pcie_hcd_plt_dev[core];
	    if (pdev->name == NULL)
	        pdev->name = PCIE_HCD_DEV_NAME;
	    pdev->id = core;

	    platform_device_unregister(pdev);
	    HCD_INFO("Core [%d] de-register device successfully\n", core);
	}

	HCD_FN_RET();
}

#if !defined(MODULE)
/* Only for built-in drivers */
subsys_initcall(pcie_hcd_plt_init);
#endif /* !MODULE */
