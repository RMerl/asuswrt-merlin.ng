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
#ifndef __PCIE_HCD_H
#define __PCIE_HCD_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * +-----------------------------------------------------
 *  Defines
 * +-----------------------------------------------------
 */

#define HC_CFG_SKP_PWRUP                    (1 << 0)
#define HC_CFG_SKP_PWRDN                    (1 << 1)
#define HC_CFG_SKP_DT                       (1 << 2)

/* Maximum number of PCIe ports supported by HCD */
#define MAX_NUM_PCIE_CORES                  4
#define MAX_NUM_CORE_DEVS                   2
#define MAX_MSI_MAP_SIZE                    32
#define MAX_NUM_OUTGOING_WINDOWS            4
#define OWIN0                               0
#define OWIN1                               1
#define OWIN2                               2
#define OWIN3                               3
#define PHC                                 0
#define VHC                                 1
#define MAX_HC_TYPE                         2

/* PCIe Gen (speed) values */
#define PCIE_LINK_SPEED_GEN1                1
#define PCIE_LINK_SPEED_GEN2                2
#define PCIE_LINK_SPEED_GEN3                3

/* PCIe link width values */
#define PCIE_LINK_WIDTH_1LANE               1
#define PCIE_LINK_WIDTH_2LANE               2
#define PCIE_LINK_WIDTH_4LANE               4
#define PCIE_LINK_WIDTH_8LANE               8
#define PCIE_LINK_WIDTH_MAX                 2

/* PCIe HCD APON (always power on) values */
#define HCD_APON_DEFAULT                    0
#define HCD_APON_ON                         1
#define HCD_APON_OFF                        2
#define HCD_APON_OFF_WITH_DOMAIN            3
#define HCD_APON_LAST                       (HCD_APON_OFF_WITH_DOMAIN)
#define HCD_APON_MASK                       3
#define HCD_APON_SHIFT                      2 /* 2 bits */

#define PCIE_HCD_DEV_NAME                   "bcm-pcie"
#define PCIE_HCD_DRV_NAME                   PCIE_HCD_DEV_NAME
#define VPCIE_HCD_DEV_NAME                  "bcm-vpcie"
#define VPCIE_HCD_DRV_NAME                  VPCIE_HCD_DEV_NAME

#define MSI_MATCH_ADDR_MAGIC                0xFFFFFFFC
#define MSI_MATCH_DATA_MAGIC                0x0000BCA0

/*
 * +-----------------------------------------------------
 *  Macros
 * +-----------------------------------------------------
 */
/* resources */
#define OWIN_RES_CONFIGURED(phc, win)       ((phc)->res.owin[(win)].start != 0)

/* Core config */
#define HCD_HC_CORE_CFG(cfg, core)          \
	(((0xF << ((core)*4)) & (cfg)) >> ((core)*4))

#define HCD_HC_SET_CORE_CFG(cfg, core, val) \
	do {                                    \
	    (cfg) &= (~(0xF << ((core)*4)));    \
	    (cfg) |= (val <<  ((core)*4));      \
	} while (false)

/* Enable HCD_DEBUG if debug logs are needed */
#define HCD_DEBUG

#ifdef HCD_DEBUG

#define HCD_LOG_LVL_ERROR                   0x0001
#define HCD_LOG_LVL_WARN                    0x0002
#define HCD_LOG_LVL_INFO                    0x0004
#define HCD_LOG_LVL_FN                      0x0008
#define HCD_LOG_LVL_REG                     0x0010
#define HCD_LOG_LVL_MDIO                    0x0020
#define HCD_LOG_LVL_ALL                     \
	    (HCD_LOG_LVL_FN | HCD_LOG_LVL_ERROR | HCD_LOG_LVL_WARN \
	     | HCD_LOG_LVL_INFO | HCD_LOG_LVL_REG | HCD_LOG_LVL_MDIO)

#define HCD_KERN_ERR
#define HCD_KERN_INFO
#define HCD_KERN_WARNING

#define HCD_FN_ENT()                        \
	if (hcd_log_level & HCD_LOG_LVL_FN) printk(">>%s\n", __FUNCTION__)

#define HCD_FN_EXT()                        \
	if (hcd_log_level & HCD_LOG_LVL_FN) printk("<<%s\n", __FUNCTION__)

#define HCD_FN_RET()                        do { HCD_FN_EXT(); return;} while (0)

#define HCD_FN_RET_VAL(r)                   do { HCD_FN_EXT(); return (r); } while (0)

#define HCD_ERROR(fmt, args...)             \
	if (hcd_log_level & HCD_LOG_LVL_ERROR)  \
	    printk(HCD_KERN_ERR "  %s: " fmt, PCIE_HCD_DRV_NAME, ##args)

#define HCD_INFO(fmt, args...)              \
	if (hcd_log_level & HCD_LOG_LVL_INFO)   \
	    printk(HCD_KERN_INFO "  %s: " fmt, PCIE_HCD_DRV_NAME, ##args)

#define HCD_WARN(fmt, args...)              \
	if (hcd_log_level & HCD_LOG_LVL_WARN)   \
	    printk(HCD_KERN_WARNING "  %s: " fmt, PCIE_HCD_DRV_NAME, ##args)

#define HCD_LOG_MDIO(fmt, args...)          \
	if (hcd_log_level & HCD_LOG_LVL_MDIO)   \
	    printk(HCD_KERN_WARNING "  %s: " fmt, PCIE_HCD_DRV_NAME, ##args)

#define HCD_LOG_REG(fmt, args...)          \
	if (hcd_log_level & HCD_LOG_LVL_REG)   \
	    printk(HCD_KERN_WARNING "  %s: " fmt, PCIE_HCD_DRV_NAME, ##args)

#else
#define HCD_KERN_ERR                        KERN_ERR
#define HCD_KERN_INFO                       KERN_INFO
#define HCD_KERN_WARNING                    KERN_WARNING

#define HCD_FN_ENT()                        do {} while (0)
#define HCD_FN_EXT()                        do {} while (0)
#define HCD_FN_RET()                        return
#define HCD_FN_RET_VAL(r)                   return (r)
#define HCD_ERROR(fmt, args...)             do {} while (0)
#define HCD_INFO(fmt, args...)              do {} while (0)
#define HCD_WARN(fmt, args...)              do {} while (0)
#define HCD_LOG_REGP(fmt, args...)          do {} while (0)
#define HCD_LOG_REGC(fmt, args...)          do {} while (0)
#endif /* HCD_DEBUG */
#define HCD_LOG(fmt, args...)               \
	    printk("  %s: " fmt, PCIE_HCD_DRV_NAME, ##args)

#define NUM_PCORES                          (hc_plt_cfg[PHC].num_cores)
#define NUM_VCORES                          (hc_plt_cfg[VHC].num_cores)
#define NUM_CORES                           (NUM_PCORES + NUM_VCORES)
#define PLTID2HC(id)                        (((id) < NUM_PCORES) ? PHC : VHC)
#define PLTID2CORE(id)                      \
	(((id) >= NUM_PCORES) ? ((id) - NUM_PCORES) : (id))

/*
 * +-----------------------------------------------------
 *  Structures
 * +-----------------------------------------------------
 */

/* Forward declerations */
struct pcie_hcd;


/*
 * BCM PCIe Host Controller device resources
 * @base: PCIe core registers Physical address
 * @owin: PCIe core Outgoing Window physical address
 * @bus_range: PCIe core bus number range
 * @domain: PCI domain
 * @irq: interrupt ID
 * @link_width: number of lanes
 */
struct pcie_hc_core_res
{
	struct resource base;
	struct resource owin[MAX_NUM_OUTGOING_WINDOWS];
	struct resource bus_range;
	u32    domain;
	u32    irq;
	u32    link_width;
};

/**
 * BCM PCIe Host Controller configuration
 *
 * @NVRAM/MODULE PARAM/DT Section
 *
 * @ssc:               flag to specify PCIe SSC to be enabled or not
 * @speed:             flag to specify PCIe to force run at 2.5Gbps
 * @apon:              Keep the PCIe core powered on always
 * @phypwrmode:        power mode normal or low power mode
 * @errlog:            Bus Error logging
 * @msi:               MSI interrupts
 * @acc:               Ack and credit coalescing
 * @pllid:             core id of phy clock
 * @plladj:            Adjust PLL clock frequency by -4700ppm
 *
 * @CORE Section
 *
 * @core_rev_offset:   core revision register offset
 * @core_rev_mask:     core revision register mask
 *
 * @MSI Section (optional)
 *
 * @msi_map_size:      Number of MSI interrupts supported
 * @msi_intr_bitshift: Interrupt bit position start
 */
struct pcie_hc_core_cfg
{
	bool ssc;
	u8   speed;
	u8   apon;
	u8   phypwrmode;
	u8   errlog;
	u8   msi;
	u8   acc;
	u8   pllid;
	u8   plladj;

	u32  core_rev_offset;
	u32  core_rev_mask;

	int  msi_map_size;
	u32  msi_intr_bitshift;
	u8   is_dual_lane;
};

/**
 * BCM PCIe Host Controller core information
 *
 * @id:    Core port number
 * @gen:   Core speed (gen)
 * @devs:  number of devices under core
 * @rev:   Core revision
 * @base:  mapped core register base
 */
struct pcie_hc_core_info
{
	u8   id;
	u8   gen;
	u8   devs;
	u32  rev;
	void __iomem *base;
};


/**
 * BCM PCIe Host Controller core control block
 *
 * @pdev:    pointer to platform device
 * @pdrv:    pointer to HCD control block
 * @plt_cfg: pointer to HC platform configuration
 * @cfg:     core HCD configuration
 * @res:     core PCI resources
 * @info:    core information
 * @phc_cb:  pointer to host controller private control block
 */
struct pcie_hc_core
{
#ifdef CONFIG_ARM
	/* Unused, temporary to satisfy ARM arch code
	  * This element should be first in this structure. do not move
	  */
	struct pci_sys_data sys;
#endif
	struct platform_device   *pdev;
	struct pcie_hcd          *pdrv;
	struct pcie_hc_plt_cfg   *plt_cfg;
	struct pcie_hc_core_cfg  cfg;
	struct pcie_hc_core_res  res;
	struct pcie_hc_core_info info;
	void                     *phc_cb;
};

/**
 * host controller platform configuration
 *
 * @num_cores:        number of physical and virtual cores
 * @flags:            configuration flags
 * @plt_dev:          pointer to platform device
 * @plt_drv:          pointer to platform driver
 * @pci_fops:         pci file operations
 *
 * @init_core:        function pointer to initialize HC core control block
 * @free_core:        function pointer to free HC core control block
 * @setup_rev:        function pointer to setup HC core rev based info
 * @read_reg:         function pointer to read core registers
 * @write_reg:        function pointer to write core registers
 * @config_core:      function pointer to configure pcie core
 * @reset_core:       function pointer to reset pcie core
 * @is_linkup:        function pointer to check if linkup
 * @map_core_irq:     function pointer to map pcie interrupt
 * @setup_msi:        function pointer to configure pcie msi interrupts
 * @teardown_msi:     function pointer to de-configure msi interrupts
 * @get_msi:          function pointer to get msi interrupt status
 * @setup_errint:     function pointer to configure pcie error interrupt
 * @teardown_errint:  function pointer to de-configure pcie error interrupt
 * @get_errint:       function pointer to get error interrupt status
 * @set_power:        function pointer to set power up/down pcie core
 *
 */
struct pcie_hc_plt_cfg
{
	int                    num_cores;
	unsigned int           flags;
	struct platform_device *plt_dev;
	struct platform_driver *plt_drv;
	struct pci_ops         *pci_fops;

	int  (*init_core)(struct pcie_hc_core *);
	void (*free_core)(struct pcie_hc_core *);
	int  (*setup_rev)(struct pcie_hc_core *);
	u32  (*read_reg)(struct pcie_hc_core *, unsigned);
	void (*write_reg)(u32, struct pcie_hc_core *, unsigned);
	int  (*config_core)(struct pcie_hc_core *);
	int  (*reset_core)(struct pcie_hc_core *);
	bool (*is_linkup)(struct pcie_hc_core *);
	int  (*map_core_irq)(const struct pci_dev *, u8, u8);
	int  (*setup_msi)(struct pcie_hc_core *);
	void (*teardown_msi)(struct pcie_hc_core *);
	u32  (*get_msi)(struct pcie_hc_core *);
	int  (*setup_errint)(struct pcie_hc_core *);
	void (*teardown_errint)(struct pcie_hc_core *);
	u32  (*get_errint)(struct pcie_hc_core *);
	int  (*set_power)(struct pcie_hc_core *, unsigned, bool);
};

/*
 * +-----------------------------------------------------
 *  Function declerations
 * +-----------------------------------------------------
 */
/* To HCD */
extern int pcie_phc_plt_init(struct pcie_hc_plt_cfg *hc_cfg);
extern int pcie_vhc_plt_init(struct pcie_hc_plt_cfg *hc_cfg);

/* From HCD */

/*
 * +-----------------------------------------------------
 *  Variable declerations
 * +-----------------------------------------------------
 */
extern int hcd_log_level;
extern struct pcie_hc_plt_cfg hc_plt_cfg[];

extern u32 pcie_ssc_cfg;
extern u32 pcie_pllclkid;

#ifdef __cplusplus
}
#endif

#endif /* __PCIE_HCD_H */
