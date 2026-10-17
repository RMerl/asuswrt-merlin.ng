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
#include <linux/ratelimit.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
#include "../drivers/pci/pci.h"
#endif

#if defined(CONFIG_BRCM_IKOS) || defined(CONFIG_BCM_PCIE_PMC_BRD_STUBS)
#define USE_PMC_BRD_STUBS
#endif /* !CONFIG_BRCM_IKOS && !CONFIG_BCM_PCIE_PMC_BRD_STUBS */
#include <board.h>
#include <pmc_core_api.h>
#include <pmc_pcie.h>
#include <shared_utils.h>
#include <bcm_intr.h>
#include <bcm_ubus4.h>

#include <pcie_hcd.h>
#include <pcie_wlep.h>
#include <pcie-bcm963xx.h>

/*
 * +-----------------------------------------------------
 *  Defines
 * +-----------------------------------------------------
 */
#define NUM_PCIE_CORES                      (NUM_CORE)
#define BCM963XX_PCIE_HC_DEV_NAME           "bcm963xx-pcie"
#define BCM963XX_PCIE_HC_DRV_NAME           BCM963XX_PCIE_HC_DEV_NAME
#define BCM963XX_ROOT_BUSNUM                0x00
#define BCM963XX_MAX_BUSNUM                 0xFF
#define BCM963XX_MAX_LINK_WIDTH             PCIE_LINK_WIDTH_2LANES


/* PCIe MSI */
#define MSI_ISR_NAME_STR_LEN                32

/* PCIe PHY */
#define PHY_PCIE_BLK2_PWR_MGMT_REGS         4
#define PCIE_G3_PLL_LOCK_WAIT_TIMEOUT       100 /* milliseconds */

/* PCIe SERDES */
#define PHY_PROCESS_28HPM                   0
#define PHY_PROCESS_16FFC                   1
#define PHY_PROCESS_8NM                     2
#define PHY_PROCESS_MAX                     3

#define PHY_SERDES_VIPER                    0
#define PHY_SERDES_BLACKSHARK               1
#define PHY_SERDES_MERLIN8                  2
#define PHY_SERDES_MERLIN8_API              3
#define PHY_SERDES_MAX                      4
#define PHY_SERDES_INV                      0xFFFFFFFF

#define PHY_FREQ_REFCLK_50MHZ               0
#define PHY_FREQ_XTAL_50MHZ                 0
#define PHY_FREQ_REFCLK_80MHZ               1
#define PHY_FREQ_XTAL_80MHZ                 1
#define PHY_FREQ_MAX                        2

/* Base Address Register (BAR) assignments */
#define BAR1                                1
#define BAR2                                2
#define BAR3                                3
#define BAR4                                4
#define BAR5                                5
#define BAR6                                6
#define BAR_FIRST                           BAR1
#define BAR_LAST                            BAR6
#define BAR123_MASK                         ((1<<BAR1) | (1<<BAR2) | (1<<BAR3))
#define BAR456_MASK                         ((1<<BAR4) | (1<<BAR5) | (1<<BAR6))

#define SZ_2G                               0x80000000
#define SZ2KB(sz)                           ((sz) >> 10)
#define SZ2MB(sz)                           ((sz) >> 20)
#define SZKB_32G                            (SZ2KB(SZ_2G) << 4)

/*
 * BAR Configuration for the SoC address map
 *
 *  2GB: 0x00_0000_0000 - 0x00_7FFF_FFFF : Lower 2GB of DDR
 *  2GB: 0x00_8000_0000 - 0x00_FFFF_FFFF : Register Space
 *  4GB: 0x01_0000_0000 - 0x01_FFFF_FFFF : 4GB for 8GB DDR
 *  8GB: 0x02_0000_0000 - 0x03_FFFF_FFFF : 8GB for 16GB DDR
 * 16GB: 0x04_0000_0000 - 0x07_FFFF_FFFF : 16GB  for 32GB DDR
 * 32GB: 0x08_0000_0000 - 0x0F_FFFF_FFFF : 32GB  for 64GB DDR
 *  2GB: 0x01_0000_0000 - 0x01_7FFF_FFFF : Upper 2GB for 4GB DDR
 *       0x02_0000_0000 - 0x02_7FFF_FFFF : Upper 2GB for 8GB DDR
 *       0x04_0000_0000 - 0x04_7FFF_FFFF : Upper 2GB for 16GB DDR
 *       0x08_0000_0000 - 0x08_7FFF_FFFF : Upper 2GB for 32GB DDR
 *       0x10_0000_0000 - 0x10_7FFF_FFFF : Upper 2GB for 32GB DDR
 */
#define DDR_ADDR_2GB_BASE_HI                0x00000000
#define DDR_ADDR_U2GB_BASE_HI_4GBDDR        0x00000001
#define DDR_ADDR_U2GB_BASE_HI_8GBDDR        0x00000002
#define DDR_ADDR_U2GB_BASE_HI_16GBDDR       0x00000004
#define DDR_ADDR_U2GB_BASE_HI_32GBDDR       0x00000008
#define DDR_ADDR_U2GB_BASE_HI_64GBDDR       0x00000010
#define DDR_ADDR_4GB_BASE_HI                0x00000001
#define DDR_ADDR_8GB_BASE_HI                0x00000002
#define DDR_ADDR_16GB_BASE_HI               0x00000004
#define DDR_ADDR_32GB_BASE_HI               0x00000008

/*
 * MAX DDR Size supported by PCIe BAR with the current SoC address map
 * 64GB (32GB + 16 GB + 8GB + 4GB + 2GB + 2GB)
 */
#define DDR_RC_BAR_CONFIG_SIZE_MAX          (MISC_RC_BAR_CONFIG_LO_SIZE_MAX + 1)

#define RC_LINK_CAP_MAX_LNK_WIDTH_SHIFT     \
	RC_CFG_PRIV1_LINK_CAP_MAX_LINK_WIDTH_SHIFT
#define RC_MEM_WIN_LO_BASE_LIMIT_LIMIT_SHIFT\
	MISC_CPU_2_PCI_MEM_WIN_LO_BASE_LIMIT_LIMIT_SHIFT
#define RC_MEM_WIN_LO_BASE_LIMIT_LIMIT_MASK \
	MISC_CPU_2_PCI_MEM_WIN_LO_BASE_LIMIT_LIMIT_MASK

/*
 * Priority Queueing
 */
/*
 * -------------------------------------------------
 * | Traffic  | Traffic | PCIe  | UBUS | UBUS      |
 * | Type     | Class   | Queue | Flow | Priority  |
 * |----------|---------|-------|------|-----------|
 * | Mutex/BC |   3     | Trap  | Trap |  1        |
 * | ICC      |   2     |   2   |   2  |  1        |
 * | CT-DMA   |   1     |   1   |   1  |  1        |
 * | Default  |   0     |   0   |   0  |  0        |
 * -------------------------------------------------
 */

/* PCIe Bus Traffic Class (0-7) */
#define PCIE_TC0                            0 /* PCIe Traffic Class 0 */
#define PCIE_TC1                            1 /* PCIe Traffic Class 1 */
#define PCIE_TC2                            2 /* PCIe Traffic Class 2 */
#define PCIE_TC3                            3 /* PCIe Traffic Class 3 */
#define PCIE_TC4                            4 /* PCIe Traffic Class 4 */
#define PCIE_TC5                            5 /* PCIe Traffic Class 5 */
#define PCIE_TC6                            6 /* PCIe Traffic Class 6 */
#define PCIE_TC7                            7 /* PCIe Traffic Class 7 */

/* PCIe Queues for PCIe Traffic Class mapping */
#define PCIE_TC2Q_LOW                       MISC_EXT_PCIE_TC_QUEUE_LOW_PRIO
#define PCIE_TC2Q_MID                       MISC_EXT_PCIE_TC_QUEUE_MID_PRIO
#define PCIE_TC2Q_HIGH                      MISC_EXT_PCIE_TC_QUEUE_HIGH_PRIO
#define PCIE_TC2Q_RESV                      MISC_EXT_PCIE_TC_QUEUE_RESV_PRIO

/* UBUS Flows for PCIe Traffic Class mapping */
#define PCIE_TC2F_LOW                       MISC_EXT_PCIE_TC_FLOW_LOW_PRIO
#define PCIE_TC2F_MID                       MISC_EXT_PCIE_TC_FLOW_MID_PRIO
#define PCIE_TC2F_HIGH                      MISC_EXT_PCIE_TC_FLOW_HIGH_PRIO
#define PCIE_TC2F_HIGHEST                   MISC_EXT_PCIE_TC_FLOW_RESV_PRIO

/* UBUS Priorities for PCIe Traffic Class mapping */
#define PCIE_TC2P_LOW                       MISC_EXT_PCIE_TC_UBUS_LOW_PRIO
#define PCIE_TC2P_HIGH                      MISC_EXT_PCIE_TC_UBUS_HIGH_PRIO

/* Arbitration Weights */
#define ARB_WEIGHT_SP0                      0 /* Strict Priority, Lowest */
#define ARB_WEIGHT_SP1                      1 /* Strict Priority, Medium */
#define ARB_WEIGHT_SP2                      2 /* Strict Priority, Highest */
#define ARB_WEIGHT_XWRR1                    1 /* Round Robbin  1/32 */
#define ARB_WEIGHT_XWRR8                    8 /* Round Robbin 8/32 */

/* Arbitration Modes */
#define ARB_MODE_SP                         \
	MISC_EXT_PCIE_UBUS_READ_REQUEST_ARB_SP
#define ARB_MODE_WRR                        \
	MISC_EXT_PCIE_UBUS_READ_REQUEST_ARB_WRR
#define ARB_MODE_IWRR                       \
	MISC_EXT_PCIE_UBUS_READ_REQUEST_ARB_IWRR

/* mode+weight register value for given arbiter */
#define ARB_MODE_WEIGHT(arb, mode, weight)  \
	MISC_EXT_PCIE_UBUS_READ_REQUEST_ARB_WEIGHT((arb), (mode), (weight))

/*
 * +-----------------------------------------------------
 *  Macros
 * +-----------------------------------------------------
 */

/* DT Binding */
#define HCD_USE_DT_ENTRY(core)             \
	((IS_ENABLED(CONFIG_OF)) ? TRUE : FALSE)

#define PCIE_PHY_SERDES(phc)               \
	((struct bcm963xx_hc_cb *)(phc)->phc_cb)->phy.serdes
#define PCIE_PHY_PROCESS(phc)              \
	((struct bcm963xx_hc_cb *)(phc)->phc_cb)->phy.process
#define PCIE_PHY_FREQUENCY(phc)            \
	((struct bcm963xx_hc_cb *)(phc)->phc_cb)->phy.frequency

/* Register access */
#define hcd_readl                          debug_hcd_readl
#define hcd_writel                         debug_hcd_writel

/* #define PCIE_HCD_FORCE_LOG_REG_ACCESS */
#if defined(PCIE_HCD_FORCE_LOG_REG_ACCESS)
#define hcd_nolog_readl                      debug_hcd_readl
#define hcd_nolog_writel                     debug_hcd_writel
#else /* !PCIE_HCD_FORCE_LOG_REG_ACCESS */
#define hcd_nolog_readl                      nodebug_hcd_readl
#define hcd_nolog_writel                     nodebug_hcd_writel
#endif /* !PCIE_HCD_FORCE_LOG_REG_ACCESS */

#define BCM94912_CHIP(chipid)                \
	(((chipid) == 0x4912) || ((chipid) == 0x4915) || ((chipid) == 0x49418))
/*
 * +-----------------------------------------------------
 *  Structures
 * +-----------------------------------------------------
 */
/**
 * BCM963xx PCIe Host Controller control block
 *
 * @cpu_intr_bitmask: CPU interrupt bitmask for MSI
 * @intr_status: MSI Interrupt status register offset
 * @intr_clear: MSI Interrupt clear register offset
 * @intr_mask_set: MSI Interrupt mask set register offset
 * @intr_mask_clear: MSI Interrupt mask clear register offset
 * @intr_bitshift: MSI Interrupts bits position on the above registers
 * @intr_bitmask: MSI Interrupts bitmask
 */
struct bcm963xx_hc_msi {
	uint32 cpu_intr_bitmask;
	uint32 intr_status;
	uint32 intr_clear;
	uint32 intr_mask_set;
	uint32 intr_mask_clear;
	uint32 intr_bitshift;
	uint32 intr_bitmask;
};

/*
 * BCM963xx PCIe Host Controller Driver workarounds (wars)
 *
 * @g2defset:        GEN2 default settings workaround
 * @bifp1sysclk:     Bifurcation Port#1 System Clock workaround
 * @g3txclk:         GEN3 TX clock start workaround (MCS63158-ESX00-R)
 * @g3rxset:         GEN3 RX default settings workaround (MCS63158-ESX00-R)
 * @g3cmosdrvopclk:  GEN3 CMOS driver for output clock workaround
 *                   (CRRBBPCIE-177)
 * @g3refclkdetmode: GEN3 Reference clock detect mode workaround (CRRBBPCIE-175)
 * @g2pllcoupling:   GEN2 high Jitter due to PLL Coupling workaround
 *                   (MCS4915-ESX00-R)
 * @g3tcaerrata:     GEN3 Transmit Clock Alignment Errata (MCS4915-ESX00-R)
 * @rxsigdetflt:     RX Signal Detect Filter
 * @ptmcapadv:       PTM Capability advertisement
 * @phaseonlyckcmp:  Phase Only clock compensation (CRRBBPCIE-262)
 */
struct bcm963xx_hc_war
{
	uint32 g2defset:1;
	uint32 bifp1sysclk:1;
	uint32 g3txclk:1;
	uint32 g3rxset:1;
	uint32 g3cmosdrvopclk:1;
	uint32 g3refclkdetmode:1;
	uint32 g2pllcoupling:1;
	uint32 g3tcaerrata:1;
	uint32 rxsigdetflt:1;
	uint32 ptmcapadv:1;
	uint32 phaseonlyckcmp:1;
};

/*
 * BCM963xx PCIe Host Controller PHY PLL clock control block
 *
 * @idx:         Index to be used for the global ssc control block
 * @port_mask:   Mask of all ports sharing this clock
 * @ssc:         Combined 'ssc' enable configuration
 * @ssc_cfgd:    'ssc' configured status
 * @ssc_enbld:   'ssc' enabled status
 */
struct bcm963xx_hc_pll_clk
{
	int  idx;
	u32  port_mask;
	bool ssc;
	bool ssc_cfgd;
	bool ssc_enbld;
};

/*
 * BCM963xx PCIe PHY information
 *
 * @serdes:        Serdes in use (PHY_SERDES_XXXX)
 * @process:       Analog Serdes process (PHY_PROCESS_XXX)
 * @frequency:     clock frequency (PHY_FREQ_XXX)
 * @lane:          lane# for bifurcated ports
 * @clk:           PLL Clock control block
 */
struct bcm963xx_hc_phy
{
	int                         serdes;
	int                         process;
	int                         frequency;
	int                         lane;
	struct bcm963xx_hc_pll_clk* clk;
};

/*
 * BCM963xx PCIe Host Controller control block
 *
 * @misc_pdev: pointer to MISC block platform devices
 * @msi:       host controller msi register information
 * @wars:      host controller WAR's based on core revision
 */
struct bcm963xx_hc_cb
{
	struct platform_device    *misc_pdev;
	struct bcm963xx_hc_msi msi;
	struct bcm963xx_hc_war wars;
	struct bcm963xx_hc_phy phy;
};

/*
 * +-----------------------------------------------------
 *  Local Function prototype
 * +-----------------------------------------------------
 */
/* MSI Interrupt */
static u32 bcm963xx_hc_msi_isr(struct pcie_hc_core *phc);
static int bcm963xx_hc_msi_enable(struct pcie_hc_core *phc);
static void bcm963xx_hc_msi_disable(struct pcie_hc_core *phc);

/* Errlog Interrupt */
static irqreturn_t bcm963xx_hc_errlog_isr(int irq, void *data);
static int bcm963xx_hc_errlog_enable(struct pcie_hc_core *phc);
static void bcm963xx_hc_errlog_disable(struct pcie_hc_core *phc);

/* Core Access */
static int bcm963xx_hc_config_read(struct pci_bus *bus, unsigned int devfn,
	int where, int size, u32 *val);
static int bcm963xx_hc_config_write(struct pci_bus *bus, unsigned int devfn,
	int where, int size, u32 val);
static void __iomem *bcm963xx_hc_map_bus(struct pci_bus *bus,
	unsigned int devfn, int where);
int bcm963xx_hc_map_irq(const struct pci_dev *pcidev, u8 slot,
	u8 pin);

/* PHY Access */
uint16 bcm963xx_hc_mdio_read(struct pcie_hc_core *phc,
	uint16 phyad, uint16 regad);
int bcm963xx_hc_mdio_write(struct pcie_hc_core *phc,
	uint16 phyad, uint16 regad, uint16 wrdata);

/* PHY setup */
static void bcm963xx_hc_gen2_phy_config_ssc(struct pcie_hc_core *phc);
static int bcm963xx_hc_gen2_phy_enable_ssc(struct pcie_hc_core *phc,
	bool enable);
static void bcm963xx_hc_phy_config_rescal(struct pcie_hc_core *phc);
static void bcm963xx_hc_phy_config(struct pcie_hc_core *phc);
static void bcm963xx_hc_gen2_phy_config(struct pcie_hc_core *phc);
static void bcm963xx_hc_gen3_phy_config(struct pcie_hc_core *phc);
static int bcm963xx_hc_gen3_phy_enable_ssc(struct pcie_hc_core *phc,
	bool enable);
static void bcm963xx_hc_gen3_phy_config_ssc(struct pcie_hc_core *phc);
static int bcm963xx_hc_phy_assert_iddq(struct pcie_hc_core *phc, bool assert);
static int bcm963xx_hc_phy_config_pwrmode(struct pcie_hc_core *phc);
static int bcm963xx_hc_phy_config_eq_presets(struct pcie_hc_core *phc);
static int bcm963xx_hc_phy_config_tca(struct pcie_hc_core *phc);
static int bcm963xx_hc_phy_config_phyrxfilter(struct pcie_hc_core *phc,
	bool inreset);

/* Core setup */
static int bcm963xx_hc_core_config_ecam(struct pcie_hc_core *phc);
static int bcm963xx_hc_core_config_fastpath(struct pcie_hc_core *phc, u32 en);
static int bcm963xx_hc_core_config_prioqueue(struct pcie_hc_core *phc);
static void bcm963xx_hc_core_set_reset(struct pcie_hc_core *phc,
	bool enable);
static int bcm963xx_hc_core_set_speed(struct pcie_hc_core *phc, u8 speed);
static int bcm963xx_hc_core_get_speed(struct pcie_hc_core *phc);
static int bcm963xx_hc_core_set_link_width(struct pcie_hc_core *phc);
static int bcm963xx_hc_core_set_coalescing(struct pcie_hc_core *phc);
static int bcm963xx_hc_core_reset_config(struct pcie_hc_core *phc);
static int bcm963xx_hc_core_pre_config(struct pcie_hc_core *phc);
static int bcm963xx_hc_core_post_config(struct pcie_hc_core *phc);
static int bcm963xx_hc_core_unconfig(struct pcie_hc_core *phc);
static int bcm963xx_hc_core_reset(struct pcie_hc_core *phc);
static int bcm963xx_hc_core_power(struct pcie_hc_core *phc, unsigned int devidx,
	bool up, int type);

static int bcm963xx_hc_setup_rev(struct pcie_hc_core *phc);

static int bcm963xx_hc_probe(struct pcie_hc_core *phc);
static void bcm963xx_hc_remove(struct pcie_hc_core *phc);

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
static struct pci_ops bcm963xx_hc_ops_specific = {
	.map_bus = NULL,
	.read = bcm963xx_hc_config_read,
	.write = bcm963xx_hc_config_write,
};

/* errlog */
static char bcm963xx_hc_ubus_intr_str[][32] = {
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
#define NUM_PCIE_UBUS_INTR_STR              \
	(sizeof(bcm963xx_hc_ubus_intr_str)/sizeof(bcm963xx_hc_ubus_intr_str[0]))
#define HCD_LOG_ERR_INT(fmt, args...)       \
	if (printk_ratelimit()) HCD_ERROR(fmt, ##args)

static char bcm963xx_hc_phy_serdes_name[PHY_SERDES_MAX][16] = {
	"Viper", "BlackShark", "Merlin8", "Merlin8-API"
};
static char bcm963xx_hc_phy_process_name[PHY_PROCESS_MAX][16] = {
	"28HPM", "16FFC", "8NM"
};
static char bcm963xx_hc_phy_freq_name[PHY_FREQ_MAX][16] = {
	"50MHz", "80MHz"
};

static struct bcm963xx_hc_pll_clk bcm963xx_pllclk_cb[NUM_PCIE_CORES] = {0};
static fp_set_power_t bcm963xx_fp_set_power = NULL;

/*
 * +-----------------------------------------------------
 *  Local inline functions
 * +-----------------------------------------------------
 */
#if defined(CONFIG_BRCM_IKOS) || defined(CONFIG_BCM_PCIE_PMC_BRD_STUBS)
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
#else /* !CONFIG_BRCM_IKOS && !CONFIG_BCM_PCIE_PMC_BRD_STUBS */
static inline int pmc_hcd_get_chip_id(void) {
	return kerSysGetChipId();
}
#endif /* !CONFIG_BRCM_IKOS && !CONFIG_BCM_PCIE_PMC_BRD_STUBS */

static inline int pmc_hcd_get_chip_rev(void) {
	return UtilGetChipRev();
}

/* read 32bit PCIe register space */
static inline u32 debug_hcd_readl(struct pcie_hc_core *phc, unsigned offset)
{
	u32 data = readl(phc->info.base + offset);

	HCD_LOG_REG("read_reg:  <0x%08x> <0x%08x>\n",
	    (u32)(phc->res.base.start + offset), data);

	return data;
}

/* write 32bit PCIe register space */
static inline void debug_hcd_writel(u32 data, struct pcie_hc_core *phc,
	unsigned offset)
{
	writel(data, phc->info.base + offset);

	HCD_LOG_REG("write_reg: <0x%08x> <0x%08x>\n",
	    (u32)(phc->res.base.start + offset), data);

	return;
}

/* read 32bit PCIe register space */
static inline u32 nodebug_hcd_readl(struct pcie_hc_core *phc, unsigned offset)
{
	return readl(phc->info.base + offset);
}

/* write 32bit PCIe register space */
static inline void nodebug_hcd_writel(u32 data, struct pcie_hc_core *phc,
	unsigned offset)
{
	writel(data, phc->info.base + offset);
}

/* find and return the ssc clock core cb pointer */
static inline struct bcm963xx_hc_pll_clk*
	bcm963xx_hc_get_pllclk(struct pcie_hc_core *phc)
{
	struct bcm963xx_hc_cb *phc_cb = (struct bcm963xx_hc_cb *)(phc->phc_cb);
	struct bcm963xx_hc_pll_clk *pllclk = NULL;

	pllclk = phc_cb->phy.clk;

	if (pllclk == NULL) {
	    pllclk = &bcm963xx_pllclk_cb[phc->cfg.pllid];

	    /* Initialize pll clk parameters if not done already */
	    if (pllclk->port_mask == 0x0) {
	        int core_idx, clk_idx, ssc;

	        pllclk->idx = phc->cfg.pllid;
	        for (core_idx = 0; core_idx < NUM_PCIE_CORES; core_idx++) {
	            clk_idx = HCD_HC_CORE_CFG(pcie_pllclkid, core_idx);
	            ssc = HCD_HC_CORE_CFG(pcie_ssc_cfg, core_idx);
	            if (clk_idx == phc->cfg.pllid) {
	                pllclk->port_mask |= (1 << core_idx);
	                pllclk->ssc |= (ssc ? true : false);
	            }
	        }

	        HCD_INFO("Core [%d] Initialized pllclk [%d] port_msk [0x%x] "
	            "ssc [%d]\n",
	            phc->info.id, phc->cfg.pllid, pllclk->port_mask, pllclk->ssc);
	    }
	    /* Set pll clock pointer */
	    phc_cb->phy.clk = pllclk;
	}

	HCD_INFO("Core [%d] Using pllclk [%d][0x%px] port_msk [0x%x] ssc [%d]\n",
	    phc->info.id, phc->cfg.pllid, pllclk, pllclk->port_mask, pllclk->ssc);

	return pllclk;
}

/* Check if PCIe link up or not */
static inline int bcm963xx_hc_pcie_link_up(struct pcie_hc_core *phc)
{
	u32 status = hcd_readl(phc, RC_DL_DL_STATUS_REG);
	return (status & RC_DL_DL_STATUS_PHYLINKUP_MASK) ? 1 : -ENODEV;
}

static inline int bcm963xx_hc_is_pcie_link_up(struct pcie_hc_core *phc)
{
	struct bcm963xx_hc_cb *phc_cb = (struct bcm963xx_hc_cb *)phc->phc_cb;
	int link_up = bcm963xx_hc_pcie_link_up(phc);

	if ((link_up == 1) && (phc_cb->wars.rxsigdetflt == 1)) {
	    /* Linkup state, configure (in reset = FALSE) */
	    if (bcm963xx_hc_phy_config_phyrxfilter(phc, FALSE) == 0) {
	        /* Check link state again */
	        link_up = bcm963xx_hc_pcie_link_up(phc);
	    } else {
	        link_up = -ENODEV;
	    }
	}

	if (link_up == 1) {
	    link_up = bcm963xx_hc_phy_config_tca(phc);
	}

	return link_up;
}

static inline uint16 mdio_get_read_blk_addr(struct pcie_hc_core *phc,
	uint16 blk, uint8 idx)
{
	uint16 addr;
	struct bcm963xx_hc_cb *phc_cb = (struct bcm963xx_hc_cb *)phc->phc_cb;

	if (!((phc_cb->phy.lane == HCD_PHYLANE_AUTO) ||
	    (phc_cb->phy.lane == HCD_PHYLANE_BC))) {
	    idx = phc_cb->phy.lane - 1;
	}

	/* Get Lane specific address */
	addr = blk +  idx * SERDES_LN_REG;

	return addr;
}

static inline uint16 mdio_get_write_blk_addr(struct pcie_hc_core *phc,
	uint16 blk)
{
	uint16 addr;
	int ln = ((struct bcm963xx_hc_cb *)(phc)->phc_cb)->phy.lane;

	if ((ln == HCD_PHYLANE_AUTO) || (ln == HCD_PHYLANE_BC)) {
	    /* Use Broadcast address */
	    addr = (blk >> 4) | 0xF000;
	} else {
	    addr = blk + (ln - 1) * SERDES_LN_REG;
	}

	return addr;
}

/* BAR Configuration for the given memory address and size */
void bcm963xx_hc_config_bar(struct pcie_hc_core *phc, int num, uint32 base_hi,
	uint32 base_lo, uint32 sizekb, int icm) {
	uint32 reg_data;
	uint32 reg_addr;
	uint32 barsz = 0;

	/* Calculate the size to be programmed (in terms of 64KB) */
	if (sizekb) {
	    for (barsz = 0; barsz < MISC_RC_BAR_CONFIG_LO_SIZE_MAX; barsz++) {
	        if ((64 * (1 << barsz)) >= sizekb) {
	            break;
	        }
	    }
	    barsz++;
	}

	/* program bar_lo */
	reg_addr = MISC_RC_BAR_CONFIG_REG(num);
	reg_data = base_lo & MISC_RC_BAR_CONFIG_LO_MATCH_ADDRESS_MASK;
	reg_data |= (barsz);
	hcd_writel(reg_data, phc, reg_addr);

	/* program bar_hi */
	hcd_writel(base_hi, phc, reg_addr + MISC_RC_BAR_CONFIG_HI_REG);

	/* program remap_lo */
	reg_addr = MISC_UBUS_BAR_CONFIG_REMAP_REG(num);
	reg_data = base_lo & MISC_UBUS_BAR_CONFIG_REMAP_OFFSET_MASK;
	if (sizekb) {
	    reg_data |= MISC_UBUS_BAR_CONFIG_REMAP_ACCESS_EN;
	}
	hcd_writel(reg_data, phc, reg_addr);

	/* program remap_hi */
	hcd_writel(base_hi, phc, reg_addr + MISC_RC_BAR_CONFIG_HI_REG);

	/*
	 * For Inter Core messaging, incoming read completion ordering is
	 * relaxed, and may be processed ahead of incoming write or message
	 * transactions
	 */
	if (icm) {
	    reg_addr = MISC_CTRL_REG;
	    reg_data = hcd_readl(phc, reg_addr);
	    reg_data |= MISC_CTRL_PCIE_IN_CPL_RO;
	    hcd_writel(reg_data, phc, reg_addr);
	}

	HCD_LOG("Core [%d] BAR%d " HCD_ADDR_SIZE_LOG_FMT "\n",
	    phc->info.id, num,
	    HCD_ADDR_SIZE_LOG((((u64)base_hi << 32) |  base_lo), (sizekb * SZ_1K)));

	return;
}

/* BAR Configuration for the given memory address and size */
void bcm963xx_hc_get_bar(struct pcie_hc_core *phc, int num,
	uint32 *base_hi, uint32 *base_lo, uint32 *sizekb) {
	uint32 reg_data;
	uint32 reg_addr;
	uint32 hi_offset;
	uint32 barsz;

	*sizekb = 0;

	/* Check if the bar is enabled ? */
	reg_addr = MISC_UBUS_BAR_CONFIG_REMAP_REG(num);
	reg_data = hcd_readl(phc, reg_addr);
	if ((reg_data & MISC_UBUS_BAR_CONFIG_REMAP_ACCESS_EN) == 0)
	    return;

	/* Read bar_lo */
	reg_addr = MISC_RC_BAR_CONFIG_REG(num);
	reg_data = hcd_readl(phc, reg_addr);
	*base_lo = reg_data & MISC_RC_BAR_CONFIG_LO_MATCH_ADDRESS_MASK;
	barsz = reg_data & (~MISC_RC_BAR_CONFIG_LO_MATCH_ADDRESS_MASK);

	/* Convert barsz to kb size */
	if (barsz--) {
	    *sizekb = 64;
	    while (barsz--) {
	        (*sizekb) <<= 1;
	    }
	}

	/* Read bar_hi */
	reg_data = hcd_readl(phc, reg_addr + MISC_RC_BAR_CONFIG_HI_REG);
	*base_hi = reg_data;

	if (barsz == MISC_RC_BAR_CONFIG_LO_SIZE_16GB)
	    hi_offset = 3;
	else if (barsz == MISC_RC_BAR_CONFIG_LO_SIZE_8GB)
	    hi_offset = 1;
	else
	   hi_offset = 0;

	HCD_INFO("Core [%d] BAR%d get 0x%02x_%08x - 0x%x_%08x\n", phc->info.id, num,
	    *base_hi, *base_lo, (*base_hi + hi_offset),
	    *base_lo + (*sizekb) * SZ_1K);

	return;
}

/*
 * Function bcm963xx_hc_phy_config_tca (phc)
 *
 *   Parameters:
 *    phc ... pointer to PCIe core HC data structure
 *
 *   Description:
 *    Configure Transmit Clock Adjustment if required
 *
 *   Return: Status of configuration
 *     0:       Success
 *     -EAGAIN: Try again
 *     -ENODEV: Failure
 */
static int bcm963xx_hc_phy_config_tca(struct pcie_hc_core *phc)
{
	struct bcm963xx_hc_cb *phc_cb = (struct bcm963xx_hc_cb *)(phc->phc_cb);
	int rc = 0;
	struct wlep_dev* wldev = NULL;
	int lane;
	bool tcabsign = true;
	u16 rddata;
	u32 buses = 0xFF000000;

	HCD_FN_ENT();

	if (phc_cb->wars.g3tcaerrata == 0) {
	    HCD_FN_EXT();
	    return rc;
	}

	HCD_LOG("Applying g3tcaerrata\n");

	/* Put TCA Fine tuning mode */
	{
	    /*
	     * Block:0x1000 (PCIE_BLK0), Register:0x07 (tx_seq_ctrl)
	     * bit[1]] 1 (tca_fine_tune_en)
	     *
	     * ' enable fine tune tca (bit-11)
	     */
	    bcm963xx_hc_mdio_write(phc, 0, 0x1f, SERDES_PCIE_BLK0_REGS);
	    rddata = bcm963xx_hc_mdio_read(phc, 0, 0x07);
	    if (!(rddata & (1<< 11))) {
	        rddata |= (1<< 11);
	        bcm963xx_hc_mdio_write(phc, 0, 0x07, rddata);
	        HCD_INFO("Enabled TCA fine_tune\n");
	    }
	}


	/* Check for TCA Bug Signature */
	for (lane = 0; lane < phc->res.link_width; lane++) {


	    bcm963xx_hc_mdio_write(phc, 0, 0x1f, 0xffd0);
	    bcm963xx_hc_mdio_write(phc, 0, 0x1e, 0x0800|lane);

	    /*
	     * Block:0xD210 (TXPMD_B), Register:0x06 (txpmd_b_ctrl_6)
	     * bit[0] rg_tx_snapshot_request default: 0'b
	     */
	    HCD_INFO("Toggling snapshot_request\n");
	    bcm963xx_hc_mdio_write(phc, 0, 0x1f, SERDES_TXPMD_B_REGS);
	    rddata = bcm963xx_hc_mdio_read(phc, 0, 0x06);
	    rddata &= 0xFFFE;    /* Clear rg_tx_snapshot_request */
	    bcm963xx_hc_mdio_write(phc, 0, 0x06, rddata);
	    rddata |= 0x0001;
	    bcm963xx_hc_mdio_write(phc, 0, 0x06, rddata);

	    /* Updated status register 7 or 8 indicates no TCA bug signature */

	    /*
	     * Block:0xD210 (TXPMD_B), Register:0x07 (txpmd_b_sts_7)
	     * bit[15:0] sts_tx_snapshot_15_0 default: 0x3801
	     */
	    rddata = bcm963xx_hc_mdio_read(phc, 0, 0x07);

	    if (rddata != 0x3801) {
	        tcabsign = false;
	        break;
	    }

	    /*
	     * Block:0xD210 (TXPMD_B), Register:0x08 (txpmd_b_sts_8)
	     * bit[15:0] sts_tx_snapshot_31_16 default: 0x7000
	     */
	    rddata = bcm963xx_hc_mdio_read(phc, 0, 0x08);

	    if (rddata != 0x7000) {
	        tcabsign = false;
	        break;
	    }
	}

	/* TCA Correction Algorithm */
	if (tcabsign == true) {
	    struct pci_bus *root_bus;

	    HCD_LOG("TCA bug signature found, applying correction\n");

	    /* Get root bus */
	    root_bus = pcie_hcd_get_rootbus(phc->pdrv);
	    if (!root_bus) {
	        HCD_ERROR("Failed to get root bus\n");
	        rc = -ENOMEM;
	        goto done;
	    }

	    /* Probe WLAN device */
	    wldev = wlep_probe_dev(root_bus, phc->res.owin[0].start, &rc);
	    if (wldev == NULL) {
	        HCD_ERROR("g3tcaerrata: probe device, err [%d]\n", rc);
	        goto done;
	    }

	    rc = wlep_config_dev(wldev);
	    if (rc != 0) {
	        HCD_ERROR("g3tcaerrata: configure device, err [%d]\n", rc);
	        goto done;
	    }
	    HCD_LOG("g3tcaerrata: Enumerated EP\n");

	    /* Place link in D3Hot (to avoid link errors during next step) */
	    if (wlep_set_power_state(wldev, PCI_D3hot) != 0) {
	        HCD_ERROR("g3tcaerrata: failed to set power state, err [%d]\n", rc);
	        rc = -EAGAIN;
	        goto done;
	    }

	    HCD_INFO("Restarting ... TCA\n");
	    /* Write SerDes MDIO register to restart TCA */
	    /*
	     * Block:0x1000 (PCIE_BLK0), Register:0x02 (ctrl3)
	     * bit[15] (tca_restart)
	     *
	     * ' tca_restart = 0'b
	     * ' tca_restart = 1'b
	     */
	    bcm963xx_hc_mdio_write(phc, 0, 0x1f, SERDES_PCIE_BLK0_REGS);
	    rddata = bcm963xx_hc_mdio_read(phc, 0, 0x02);
	    rddata &= ~(1 << 15);                        /* Clear tca_restart */
	    bcm963xx_hc_mdio_write(phc, 0, 0x02, rddata);
	    rddata |= (1 << 15);                         /* Set tca_restart */
	    bcm963xx_hc_mdio_write(phc, 0, 0x02, rddata);
	    udelay(10);                                  /* Delay 10 us */

	    /* Confirm done successfully */
	    /*
	     * Block:0x1000 (PCIE_BLK0), Register:0x08 (tx_seq_status)
	     * bit[4] (tca_seq_done)
	     * bit[3:0] )tx_seq_state
	     *
	     * ' tca_seq_done = 1'b, tx_seq_state = 1000'b
	     */
	    bcm963xx_hc_mdio_write(phc, 0, 0x1f, SERDES_PCIE_BLK0_REGS);
	    rddata = bcm963xx_hc_mdio_read(phc, 0, 0x08);
	    rddata &= 0x1F;
	    if (rddata != 0x18) {
	        HCD_ERROR("Failed to restart TCA\n");
	        rc = -EAGAIN;
	        goto done;
	    }

	    /* Place link in D0 */
	    if (wlep_set_power_state(wldev, PCI_D0) != 0) {
	        HCD_ERROR("g3tcaerrata: failed to set power state, err [%d]\n", rc);
	        rc = -EAGAIN;
	        goto done;
	    }
	}

done:
	/* Restore back the buses configuration */
	if (buses != 0xFF000000) {
	    hcd_writel(buses, phc, PCI_PRIMARY_BUS);
	}

	if (wldev) {
	    HCD_INFO("Freeing temporary EP device 0x%px\n", wldev);
	    wlep_free_dev(wldev);
	}

	HCD_FN_EXT();

	return rc;
}

/* configure spread spectrum clock (ssc) */
static inline void bcm963xx_hc_config_ssc(struct pcie_hc_core *phc)
{
	struct bcm963xx_hc_pll_clk *pllclk;

	/* Nothing to do, if
	 *  SSC is not supported
	 *  SSC enable is not configured
	 *  SSC is already configured by shared port
	 */
	pllclk = bcm963xx_hc_get_pllclk(phc);
	if (pllclk == NULL) {
	    HCD_ERROR("Core [%d] unable to get pllclk [%d] cb\n", phc->info.id,
	        phc->cfg.pllid);
	    return;
	}

	/* ssc not set or already configured, nothing to do */
	if ((pllclk->ssc == false) || (pllclk->ssc_cfgd == true)) {
	    HCD_INFO("Core [%d] ssc [%d] ssc_cfgd [%d] skip configure\n",
	        phc->info.id, pllclk->ssc, pllclk->ssc_cfgd);
	    return;
	}

	/* Call ssc configure functions based on core gen */
	if (phc->info.gen == PCIE_LINK_SPEED_GEN2) {
	    bcm963xx_hc_gen2_phy_config_ssc(phc);
	} else if (phc->info.gen == PCIE_LINK_SPEED_GEN3) {
	    bcm963xx_hc_gen3_phy_config_ssc(phc);
	}

	pllclk->ssc_cfgd = true;

	HCD_INFO("Port [%d] SSC Configured\n", phc->info.id);

	return;
}

/* enable/disable spread spectrum clock (ssc) */
static inline int bcm963xx_hc_enable_ssc(struct pcie_hc_core *phc,
	bool enable)
{
	int ret = 0;
	struct bcm963xx_hc_pll_clk *pllclk;

	/* Nothing to do, if
	 *  SSC is not supported
	 *  SSC enable is not configured
	 *  SSC is already enabled
	 */
	pllclk = bcm963xx_hc_get_pllclk(phc);
	if (pllclk == NULL) {
	    HCD_ERROR("Core [%d] unable to get pllclk [%d] cb\n", phc->info.id,
	        phc->cfg.pllid);
	    return 0;
	}

	/* ssc not set or already enabled, nothing to do */
	if ((pllclk->ssc == false) || (pllclk->ssc_enbld == true)) {
	    HCD_INFO("Core [%d] ssc [%d] ssc_enbld [%d] skip enable\n",
	        phc->info.id, pllclk->ssc, pllclk->ssc_enbld);
	    return 0;
	}

	/* Call ssc enable functions based on core gen */
	if (phc->info.gen == PCIE_LINK_SPEED_GEN2) {
	    ret = bcm963xx_hc_gen2_phy_enable_ssc(phc, enable);
	} else if (phc->info.gen == PCIE_LINK_SPEED_GEN3) {
	    ret = bcm963xx_hc_gen3_phy_enable_ssc(phc, enable);
	}

	if (ret == 0) {
	    HCD_LOG("Port [%d] SSC %s\n", phc->info.id,
	       (enable) ? "Enabled" : "Disabled");
	    pllclk->ssc_enbld = enable;
	} else {
	    HCD_ERROR("Core [%d] failed to %s SSC\n", phc->info.id,
	        (enable) ? "Enable" : "Disable");
	}

	return ret;
}

/*
 * +-----------------------------------------------------
 *  Local Functions
 * +-----------------------------------------------------
 */
/* MSI Local Functions */
/*
 * Function bcm963xx_hc_msi_isr (irq, data)
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
static u32 bcm963xx_hc_msi_isr(struct pcie_hc_core *phc)
{
	struct bcm963xx_hc_cb *phc_cb = (struct bcm963xx_hc_cb *)(phc->phc_cb);
	struct bcm963xx_hc_msi *msi = &phc_cb->msi;
	uint32 reg_val;

	HCD_FN_ENT();

	/* Get the MSI interrupt status */
	reg_val = hcd_nolog_readl(phc, msi->intr_status);
	reg_val &= (msi->intr_bitmask);

	/* clear the interrupts, as this is an edge triggered interrupt */
	hcd_nolog_writel(reg_val, phc, msi->intr_clear);

	/* Return the bitmask without need for shift in HCD */
	reg_val >>= msi->intr_bitshift;

	HCD_FN_EXT();

	return reg_val;
}

/*
 * Function bcm963xx_hc_msi_enable (phc)
 *
 *   Parameters:
 *    phc ... pointer to hcd
 *
 *   Description:
 *    setup PCIe core irq isr and configure the hardware to enable MSI
 *    functionality
 *
 *   Return: 0 on success, -ve on failure
 */
static int bcm963xx_hc_msi_enable(struct pcie_hc_core *phc)
{
	struct bcm963xx_hc_cb *phc_cb = (struct bcm963xx_hc_cb *)(phc->phc_cb);
	struct bcm963xx_hc_msi *msi = &phc_cb->msi;
	u32 reg_data;
	u64 msi_addr = MSI_MATCH_ADDR_MAGIC;
	int err = 0;

	if (phc->info.rev >= 0x0303) {
	    msi->intr_status = MSI_INTR2_CPU_STATUS_REG;
	    msi->intr_clear = MSI_INTR2_CPU_CLEAR_REG;
	    msi->intr_mask_set = MSI_INTR2_CPU_MASK_SET_REG;
	    msi->intr_mask_clear = MSI_INTR2_CPU_MASK_CLEAR_REG;
	    msi->intr_bitshift = MSI_INTR2_CPU_MSI_INTR_SHIFT;
	    msi->intr_bitmask = MSI_INTR2_CPU_MSI_INTR_MASK;
	    msi->cpu_intr_bitmask = CPU_INTR1_PCIE_MSI_INTR_CPU_INTR;
	} else {
	    msi->intr_status = INTR2_CPU_STATUS_REG;
	    msi->intr_clear = INTR2_CPU_CLEAR_REG;
	    msi->intr_mask_set = INTR2_CPU_MASK_SET_REG;
	    msi->intr_mask_clear = INTR2_CPU_MASK_CLEAR_REG;
	    msi->intr_bitshift = INTR2_CPU_MSI_INTR_SHIFT;
	    msi->intr_bitmask = INTR2_CPU_MSI_INTR_MASK;
	    msi->cpu_intr_bitmask = CPU_INTR1_PCIE_INTR_CPU_INTR;
	}

	/*
	 * Program the Root Complex Registers for matching address hi and low
	 * The address should be unique with in the down stream/up stream BAR
	 * mapping
	 */
	reg_data = (msi_addr & 0xFFFFFFFF);
	reg_data |= MISC_MSI_BAR_CONFIG_LO_ENABLE_MASK;
	hcd_writel(reg_data, phc, MISC_MSI_BAR_CONFIG_LO_REG);

	reg_data = (msi_addr >> 32);
	hcd_writel(reg_data, phc, MISC_MSI_BAR_CONFIG_HI_REG);

	/* Program the RC registers for matching data pattern */
	reg_data = MISC_MSI_DATA_CONFIG_MATCH_MASK;
	reg_data &= ((~(phc->cfg.msi_map_size-1))<<MISC_MSI_DATA_CONFIG_MATCH_SHIFT);
	reg_data |= MSI_MATCH_DATA_MAGIC;
	hcd_writel(reg_data, phc, MISC_MSI_DATA_CONFIG_REG);

	/* Clear all MSI interrupts initially */
	reg_data = msi->intr_bitmask;
	hcd_writel(reg_data, phc, msi->intr_clear);

	/* enable all available MSI vectors */
	hcd_writel(reg_data, phc, msi->intr_mask_clear);

	/* Enable MSI interrupt at L1 Intr1 controller */
	reg_data = msi->cpu_intr_bitmask;
	hcd_writel(reg_data, phc, CPU_INTR1_INTR_MASK_CLEAR_REG);

	return err;
}

/*
 * Function bcm963xx_hc_msi_disable (phc)
 *
 *   Parameters:
 *    phc ... pointer to hcd
 *
 *   Description:
 *    Disable MSI feature on the hardware, Free the PCIe core isr and
 *    unmap and free all the MSI interrupts
 *
 *   Return: 0 on success, -ve on failure
 */
static void bcm963xx_hc_msi_disable(struct pcie_hc_core *phc)
{
	struct bcm963xx_hc_cb *phc_cb = (struct bcm963xx_hc_cb *)(phc->phc_cb);
	struct bcm963xx_hc_msi *msi = NULL;
	u32 reg_data;

	HCD_FN_ENT();

	msi = &phc_cb->msi;
	if (msi) {
	    /* Disable MSI interrupt at L1 Intr1 controller */
	    reg_data = msi->cpu_intr_bitmask;
	    hcd_writel(reg_data, phc, CPU_INTR1_INTR_MASK_SET_REG);

	    /* Disable all available MSI vectors */
	    reg_data = msi->intr_bitmask;
	    hcd_writel(reg_data, phc, msi->intr_mask_set);
	}

	HCD_FN_EXT();

	return;
}

/*
 * Function bcm963xx_hc_map_bus (bus, devfn, where)
 *
 *   Parameters:
 *    bus   ... pointer to PCI bus data structure
 *    devfn ... PCI device, function mapping
 *    where ... offset from the device base
 *
 *   Description:
 *    Check the PCI bus/device for allowable combinations, find the device
 *    base offset, setup the hardware for access to device/function
 *
 *   Return: mapped configuration address on success, NULL on failure
 */
static void __iomem *bcm963xx_hc_map_bus(struct pci_bus *bus,
	unsigned int devfn, int where)
{
	struct pcie_hc_core *phc = pcie_hcd_get_hccore_from_bus(bus);
	uint32 offset;
	uint32 bus_no = bus->number;
	uint32 dev_no = PCI_SLOT(devfn);
	uint32 func_no = PCI_FUNC(devfn);
	bool valid = FALSE;

	HCD_FN_ENT();

	HCD_INFO("bus [0x%px] bus_no [%d] dev [%d] func [%d] where [%d]\r\n",
	    bus, bus_no, dev_no, func_no, where);

	/*
	 * RC configuration space is registers not memory, allow only valid bus/dev
	 * combinations
	 */
	if (bus_no <= (BCM963XX_ROOT_BUSNUM+1)) {
	    /* Root Complex bridge, first device or switch */
	    /* Allow only configuration space (dev#0) */
	    valid = (dev_no == 0);
	} else if (bus_no == (BCM963XX_ROOT_BUSNUM+2)) {
	    /* Switch UP stream port */
	    /* Allow access for all the DN ports */
	    valid =  TRUE;
	} else {
	    /* Switch down stream ports to devices */
	    /* Allow only configuration space (dev#0) */
	    /* otherwise will loop for the rest of the device */
	    valid = (dev_no == 0);
	}

	if (!valid) {
	    return NULL;
	}

	/* find the offset */
	offset = (bus_no) ? PCIE_EXT_CFG_DATA : 0;
	offset += where;

	/* Select the configuration */
	hcd_writel((bus_no<<EXT_CFG_BUS_NUM_SHIFT)
	    |(dev_no <<EXT_CFG_DEV_NUM_SHIFT)
	    |(func_no<<EXT_CFG_FUNC_NUM_SHIFT),
	    phc, EXT_CFG_PCIE_EXT_CFG_INDEX_REG);

	HCD_INFO("configuration space mapped address = [0x%px]\r\n",
	    phc->info.base + offset);

	HCD_FN_EXT();

	return (phc->info.base + offset);
}

static int bcm963xx_hc_config_read(struct pci_bus *bus, unsigned int devfn,
	int where, int size, u32 *val)
{
	struct pcie_hc_core *phc = pcie_hcd_get_hccore_from_bus(bus);
	void __iomem *addr;

	addr = bcm963xx_hc_map_bus(bus, devfn, where);
	if (!addr) {
	    *val = ~0;
	    return PCIBIOS_DEVICE_NOT_FOUND;
	}

	if (size == 1)
	    *val = readb(addr);
	else if (size == 2)
	    *val = readw(addr);
	else
	    *val = readl(addr);

	HCD_LOG_REG("read_reg%d:  <0x%08x> <0x%08x>\n", size,
	    (u32)(phc->res.base.start + (addr - phc->info.base)), *val);

	return PCIBIOS_SUCCESSFUL;
}

static int bcm963xx_hc_config_write(struct pci_bus *bus, unsigned int devfn,
	int where, int size, u32 val)
{
	struct pcie_hc_core *phc = pcie_hcd_get_hccore_from_bus(bus);
	void __iomem *addr;

	addr = bcm963xx_hc_map_bus(bus, devfn, where);
	if (!addr)
	    return PCIBIOS_DEVICE_NOT_FOUND;

	if (size == 1)
	    writeb(val, addr);
	else if (size == 2)
	    writew(val, addr);
	else
	    writel(val, addr);

	HCD_LOG_REG("write_reg%d:  <0x%08x> <0x%08x>\n", size,
	    (u32)(phc->res.base.start + (addr - phc->info.base)), val);

	return PCIBIOS_SUCCESSFUL;
}

/*
 * Function bcm963xx_hc_map_irq (pcidev, slot, pin)
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
int bcm963xx_hc_map_irq(const struct pci_dev *pcidev,
	u8 slot, u8 pin)
{
	int irq = -1;
	struct pcie_hc_core *phc = pcie_hcd_get_hccore_from_bus(pcidev->bus);

	HCD_FN_ENT();
	HCD_INFO("(dev [0x%px] slot [%d] pin [%d])\r\n",
	    pcidev, slot, pin);

	if (HCD_USE_DT_ENTRY(phc->info.id) && (phc->res.irq == 0)) {
	    irq = of_irq_parse_and_map_pci(pcidev, slot, pin);
	} else {
	    irq = phc->res.irq;
	}

	HCD_FN_EXT();
	return irq;
}

/*
 *  Function bcm963xx_hc_mdio_read (phc, phyad, regad)
 *
 *   Parameters:
 *    phc   ... pointer to PCIe core HC data structure
 *    phyad ... MDIO PHY address (typically 0!)
 *    regad ... Register address in range 0-0x1f
 *
 *   Description:
 *    Perform PCIE MDIO read on specified PHY (typically 0), and Register.
 *    Access is through an indirect command/status mechanism, and timeout
 *    is possible. If command is not immediately complete, which would
 *    be typically the case, one more attempt is made after a 1ms delay.
 *
 *   Return: 16-bit data item or 0xdead on MDIO timeout
 */
uint16 bcm963xx_hc_mdio_read(struct pcie_hc_core *phc,
	uint16 phyad, uint16 regad)
{
	int timeout;
	uint32 data;
	uint16 retval;

	HCD_FN_ENT();

	/*
	 * Bit-20=1 to initiate READ, bits 19:16 is the phyad, bits 4:0 is the
	 * regad
	 */
	data = 0x100000;
	data = data |((phyad & 0xf)<<16);
	data = data |(regad & 0x1F);

	hcd_writel(data, phc, RC_DL_MDIO_ADDR_REG);
	/* critical delay */
	udelay(1000);

	timeout = 2;
	while (timeout > 0) {
	    data = hcd_readl(phc, RC_DL_MDIO_RD_DATA_REG);
	    /* Bit-31=1 is DONE */
	    if (data & 0x80000000)
	        break;
	    timeout = timeout - 1;
	    udelay(1000);
	}

	if (timeout == 0) {
	    retval = 0xdead;
	    HCD_ERROR("mdio_read: <0x%04x> <0x%04x> timeout\n", regad, retval);
	} else {
	    /* Bits 15:0 is read data */
	    retval = (data & 0xffff);
	    HCD_LOG_MDIO("mdio_read: <0x%04x> <0x%04x>\n", regad, retval);
	}

	HCD_FN_EXT();

	return retval;
}

/*
 * Function bcm963xx_hc_mdio_write (phc, phyad, regad, wrdata)
 *
 *   Parameters:
 *    phc    ... pointer to PCIe core HC data structure
 *    phyad  ... MDIO PHY address (typically 0!)
 *    regad  ... Register address in range 0-0x1f
 *    wrdata ... 16-bit write data
 *
 *   Description:
 *    Perform PCIE MDIO write on specified PHY (typically 0), and Register.
 *    Access is through an indirect command/status mechanism, and timeout
 *    is possible. If command is not immediately complete, which would
 *    be typically the case, one more attempt is made after a 1ms delay.
 *
 *   Return: 1 on success, 0 on timeout
 */
int bcm963xx_hc_mdio_write(struct pcie_hc_core *phc,
	uint16 phyad, uint16 regad, uint16 wrdata)
{
	int timeout;
	uint32 data;

	HCD_FN_ENT();

	/* bits 19:16 is the phyad, bits 4:0 is the regad */
	data = ((phyad & 0xf) << 16);
	data = data | (regad & 0x1F);

	hcd_writel(data, phc, RC_DL_MDIO_ADDR_REG);
	udelay(1000);

	/* Bit-31=1 to initial the WRITE, bits 15:0 is the write data */
	data = 0x80000000;
	data = data | (wrdata & 0xFFFF);

	hcd_writel(data, phc, RC_DL_MDIO_WR_DATA_REG);
	udelay(1000);

	/* Bit-31=0 when DONE */
	timeout = 2;
	while (timeout > 0) {

	    data = hcd_readl(phc, RC_DL_MDIO_WR_DATA_REG);

	    /* CTRL1 Bit-31=1 is DONE */
	    if ((data & 0x80000000) == 0)
	        break;

	    timeout = timeout - 1;
	    udelay(1000);
	}

	HCD_FN_EXT();

	if (timeout == 0) {
	    HCD_ERROR("mdio_write: <0x%04x> <0x%04x> fail\n", regad, wrdata);
	    return 0;
	} else {
	    HCD_LOG_MDIO("mdio_write: <0x%04x> <0x%04x>\n", regad, wrdata);
	    return 1;
	}
}


/*
 * Function bcm963xx_hc_gen2_phy_config_ssc (phc)
 *
 *   Parameters:
 *    phc ... pointer to PCIe core HC data structure
 *
 *   Description:
 *    Configure PCIe SSC through MDIO interface. The settings
 *    largely comes from ASIC design team
 *
 *   Return: None
 */
void bcm963xx_hc_gen2_phy_config_ssc(struct pcie_hc_core *phc)
{
	HCD_FN_ENT();

	if (PCIE_PHY_PROCESS(phc) == PHY_PROCESS_16FFC) {
	    if (PCIE_PHY_FREQUENCY(phc) == PHY_FREQ_REFCLK_80MHZ) {
	        /*
	         * Settings from SoC Design team
	         *
	         * mdio write 0x1f 0x2300  ' Set frequency setting to 99.99 to start
	         * mdio write 0x00 0x643e  ' INT[9:0]=0x3e, FRAC[17:16]=0x1
	         * mdio write 0x01 0xf020  ' FRAC[15:0]=0xf020
	         *
	         * mdio write 0x1f 0x2600  ' point to SSC control set
	         * mdio write 0x01 0x1db2  ' write ssc_limit[15:0]
	         * mdio write 0x02 0x0001  ' write ssc_limit[18:16]
	         * mdio write 0x03 0x0120  ' write ssc_step[15:0]
	         */

	        bcm963xx_hc_mdio_write(phc, 0, 0x1f, SERDES_PLL_AFE3_REGS);
	        /* ctrl0: override_ana_reg     bits:15 */
	        /* ctrl0: override_val         bits:14 */
	        /* ctrl0: frac_mode_sel[01:00] bits:13-12 */
	        /* ctrl0: pll_ndiv_frac[17:16] bits:11-10 */
	        /* ctrl0: int_val[9:0]         bits:9-0 */
	        bcm963xx_hc_mdio_write(phc, 0, 0x00, 0x643e);
	        /* ctrl1: pll_ndiv_frac[15-00] bits:15-0 */
	        bcm963xx_hc_mdio_write(phc, 0, 0x01, 0xf020);

	        bcm963xx_hc_mdio_write(phc, 0, 0x1f, SERDES_PLL_SSC_CTRL_REGS);
	        /* ctrl1: i_ssc_limit[15-0] bits:15-0 */
	        bcm963xx_hc_mdio_write(phc, 0, 0x01, 0x1db2);
	        /* ctrl2: i_ssc_limit[18-16] bits: 2:0 */
	        bcm963xx_hc_mdio_write(phc, 0, 0x02, 0x0001);
	        /* ctrl3: i_ssc_step bits:15-0 */
	        bcm963xx_hc_mdio_write(phc, 0, 0x03, 0x0120);

	    } else  {
	        /*
	         * Settings from SoC Design team
	         *
	         * mdio write 0x1f 0x2200  ' Set frequency setting to 99.99 to start
	         * mdio write 0x00 0x5c63  ' INT[9:0]=0x63, FRAC[17:16]=0x3
	         * mdio write 0x01 0xffff  ' FRAC[15:0]=0xffff
	         *
	         * mdio write 0x1f 0x2600  ' point to SSC control set
	         * mdio write 0x01 0xf5c2  ' write ssc_limit[15:0]
	         * mdio write 0x02 0x0001  ' write ssc_limit[18:16]
	         * mdio write 0x03 0x0350  ' write ssc_step[15:0]
	         */

	        bcm963xx_hc_mdio_write(phc, 0, 0x1f, SERDES_PLL_AFE2_REGS);
	        /* ctrl0: override_ana_reg     bits:15 */
	        /* ctrl0: override_val         bits:14 */
	        /* ctrl0: frac_mode_sel[01:00] bits:13-12 */
	        /* ctrl0: pll_ndiv_frac[17:16] bits:11-10 */
	        /* ctrl0: int_val[9:0]         bits:9-0 */
	        bcm963xx_hc_mdio_write(phc, 0, 0x00, 0x5c63);
	        /* ctrl1: pll_ndiv_frac[15-00] bits:15-0 */
	        bcm963xx_hc_mdio_write(phc, 0, 0x01, 0xffff);

	        bcm963xx_hc_mdio_write(phc, 0, 0x1f, SERDES_PLL_SSC_CTRL_REGS);
	        /* ctrl1: i_ssc_limit[15-0] bits:15-0 */
	        bcm963xx_hc_mdio_write(phc, 0, 0x01, 0xf5c2);
	        /* ctrl2: i_ssc_limit[18-16] bits: 2:0 */
	        bcm963xx_hc_mdio_write(phc, 0, 0x02, 0x0001);
	        /* ctrl3: i_ssc_step bits:15-0 */
	        bcm963xx_hc_mdio_write(phc, 0, 0x03, 0x0350);
	    }
	} else {

	    /* set the SSC parameters
	     *
	     * SSC Parameters
	     * Workaround (for early gen2 cards):
	     * Block 0x1100, Register 0xA = 0xea3c
	     * Block 0x1100, Register 0xB = 0x04e7
	     * Block 0x1100, Register 0xC = 0x0039
	     * -Block 0x1100 fixed in 63148A0, 63381B0, 63138B0 but ok to write
	     *  anyway
	     */
	    if (phc->info.rev < 0x303) {
	        bcm963xx_hc_mdio_write(phc, 0, 0x1f, SERDES_PCIE_BLK1_REGS);
	        /* sscControl8: ssc_limit_50MHz_0 */
	        bcm963xx_hc_mdio_write(phc, 0, 0x0a, 0xea3c);
	        /* sscControl9: ssc_step_50MHz */
	        bcm963xx_hc_mdio_write(phc, 0, 0x0b, 0x04e7);
	        /* sscControl10 */
	        bcm963xx_hc_mdio_write(phc, 0, 0x0c, 0x0039);
	    }

	    /* set the SSC parameters
	     *
	     * SSC Parameters
	     * Block 0x2200, Register 5 = 0x5044
	     * // VCO parameters for fractional mode, -175ppm
	     * Block 0x2200, Register 6 = 0xfef1
	     * // VCO parameters for fractional mode, -175ppm
	     * Block 0x2200, Register 7 = 0xe818
	     * // VCO parameters for fractional mode, -175ppm
	     * Notes:
	     * -Only need to apply these fixes when enabling Spread Spectrum
	     *  Clocking (SSC), *   which would likely be a flash option
	     */

	    bcm963xx_hc_mdio_write(phc, 0, 0x1f, SERDES_PLL_AFE2_REGS);
	    /* ctrl5 pll_ctrl_95_80 */
	    bcm963xx_hc_mdio_write(phc, 0, 0x05, 0x5044);
	    /* ctrl6 pll_ctrl_111_96 */
	    bcm963xx_hc_mdio_write(phc, 0, 0x06, 0xfef1);
	    /* ctrl7 pll_ctrl_127_122 */
	    bcm963xx_hc_mdio_write(phc, 0, 0x07, 0xe818);
	}

	HCD_FN_EXT();

	return;
}

/*
 * Function bcm963xx_hc_gen2_phy_enable_ssc (phc,enable)
 *
 *   Parameters:
 *    phc   ... pointer to PCIe core HC data structure
 *    enable...flag to specify enable or disable SSC
 *
 *   Description:
 *    Enable/disable SSC for GEN2 cores. Assumed that SSC is configured before
 *    enabling the SSC
 *
 *   Return: 0:     on success or no action.
 *          -1:   on failure or timeout
 */
int bcm963xx_hc_gen2_phy_enable_ssc(struct pcie_hc_core *phc,
	bool enable)
{
	uint16 data = 0;
	int timeout = 40;
	int ret = 0;
	uint16 ssc_block;
	uint16 en_reg, en_mask;
	uint16 sts_reg, sts_mask;

	HCD_FN_ENT();

	/*
	 * SSC disabled when PCIe core comes out of reset to allow PLL sync to
	 * happen write sscControl0 register ssc_mode_enable_ovrd &
	 * ssc_mode_enable_ovrd_val
	 */
	if (PCIE_PHY_PROCESS(phc) == PHY_PROCESS_16FFC) {
	    /*
	     * Settings from SoC Design team
	     *
	     * mdio write 0x1f 0x2600  ' point to SSC control set
	     * mdio write 0x00 0x0442  ' enable
	     */
	    ssc_block = SERDES_PLL_SSC_CTRL_REGS;
	    en_reg = 0x0000;             /* ctrl0 */
	    en_mask = (1 << 1);          /* bit:1 i_ssc_mode */
	    sts_reg = 0x000F;            /* status */
	    sts_mask = (1<< 0);          /* bit:1 o_ssc_status */
	} else {
	    /* PCIE_SERDES_DEFAULT */
	    ssc_block = SERDES_PCIE_BLK1_REGS;
	    en_reg = 0x0002;             /* sscControl0 */
	    en_mask = (3 << 14);         /* bit:15 ssc_mode_enable_ovrd */
	                                 /* bit:14 ssc_mode_enable_overd_val */
	    sts_reg = 0x0001;            /* status */
	    sts_mask = (1 << 10);        /* bit:10 ssc_status */
	}

	bcm963xx_hc_mdio_write(phc, 0, 0x1f, ssc_block);
	data = bcm963xx_hc_mdio_read(phc, 0, en_reg);
	if (enable == TRUE)
	    data |= en_mask;
	else
	    data &= ~en_mask;
	bcm963xx_hc_mdio_write(phc, 0, en_reg, data);

	/* Check the status to see if SSC is set or not */
	while (timeout-- > 0) {
	    data = bcm963xx_hc_mdio_read(phc, 0, sts_reg);
	    /* Bit-31=1 is DONE */
	    if (((data & sts_mask) >> 10) == enable)
	        break;
	    timeout = timeout - 1;
	    udelay(1000);
	}

	if (timeout == 0) {
	    ret = -1;
	}

	HCD_FN_EXT();

	return ret;
}

/*
 * Function bcm963xx_hc_phy_config_rescal (phc)
 *
 *   Parameters:
 *    phc ... pointer to PCIe core HC data structure
 *
 *   Description:
 *    Configure PCIe PHY rescal through MDIO interface. The settings
 *    largely comes from ASIC design team
 *
 *   Return: None
 */
void bcm963xx_hc_phy_config_rescal(struct pcie_hc_core *phc)
{
	uint16 rddata, wrdata;
	int val = 0;

	HCD_FN_ENT();

	if (GetRCalSetting_1UM_VERT(&val) == kPMC_NO_ERROR)
	{
	    HCD_LOG("Core [%d] setting resistor calibration value to 0x%x\n",
	        phc->info.id, val);

	    if (phc->info.gen == PCIE_LINK_SPEED_GEN3) {
	        /*
	         *    ' Block 0x1000, Register 0, bit8=enable, bits 7:4=val
	         *    tmp = pcie_mdio_write (0, &h1f&, &hffd0&) ' AER block
	         *    tmp = pcie_mdio_write (0, &h1e&, &h0800&) ' PMA_PMD
	         *
	         *    tmp = pcie_mdio_write(0, &h1f&, &h1000&)
	         *    data = pcie_mdio_read(0, &h00&)
	         *    data = (data And Not(lshift(&h1f,4))) Or lshift(1,8)
	         *    Or lshift(val,4)
	         *    tmp = pcie_mdio_write(0, &h00&, data)
	         */

	        /*
	         * For Gen3, first Select the lane
	         *
	         * Block:0xffd0 (AER), Register:0x1e (AER)
	         * Value:0x09ff bit[15:11] 1(PMA_PMD) bit[10:0] 0x000 (lane0)
	         */
	        bcm963xx_hc_mdio_write(phc, 0, 0x1f, 0xffd0);
	        bcm963xx_hc_mdio_write(phc, 0, 0x1e, 0x0800);

	        /*
	         * Block:0x1000 (PCIE_BLK0), Register:0x00 (PCIE_BLK0_ctrl1)
	         * bit[7:4] val (rescal_force_val) bit[8] 1 (rescal_force)
	         */
	        bcm963xx_hc_mdio_write(phc, 0, 0x1f, 0x1000);
	        rddata = bcm963xx_hc_mdio_read(phc, 0, 0x00);
	        /* enable */
	        wrdata = ((rddata & 0xff0f) | ((val & 0xf) << 4) | (1 << 8));
	        bcm963xx_hc_mdio_write(phc, 0, 0x00, wrdata);
	    } else {
	        /*
	         * Rcal Calibration Timers
	         * Block 0x1000, Register 1, bit 4(enable), and 3:0 (value)
	         */
	        bcm963xx_hc_mdio_write(phc, 0, 0x1f, 0x1000);
	        rddata = bcm963xx_hc_mdio_read(phc, 0, 1);
	        wrdata = ((rddata & 0xffe0) | (val & 0xf) | (1 << 4)); /* enable */
	        bcm963xx_hc_mdio_write(phc, 0, 1, wrdata);
	    }
	}

	HCD_FN_EXT();

	return;
}

/*
 *
 * Function bcm963xx_hc_gen3_phy_config_ssc (phc)
 *
 *
 *   Parameters:
 *    phc ... pointer to PCIe core HC data structure
 *
 *   Description:
 *     Configure PCIe SSC through MDIO interface. The settings
 *     largely comes from ASIC design team
 *
 *   Return: None
 */
static void bcm963xx_hc_gen3_phy_config_ssc(struct pcie_hc_core *phc)
{
	uint32 rddata, wrdata;
	int waittime;

	HCD_FN_ENT();

	if (PCIE_PHY_PROCESS(phc) == PHY_PROCESS_16FFC) {
	    /*
	     * Settings from Design team
	     *
	     * Applied while PCIe reset is asserted (prior to link training)
	     * PCIE_3_G3_PLL.PLL_SSC_LIMIT_SSC_MODE.PLL_SSC_MODE=0
	     * PCIE_3_G3_PLL.PLL_SSC_LIMIT_SSC_MODE.PLL_SSC_DOWNSPREAD=1
	     *
	     * XTAL Freq 50MHz
	     *   PCIE_3_G3_PLL.PLL_SSC_LIMIT_SSC_MODE.PLL_SSC_LIMIT=&h9c00&
	     *   PCIE_3_G3_PLL.PLL_SSC_STEP.PLL_SSC_STEP=&h330&
	     *
	     * XTAL Freq 80MHz
	     *   PCIE_3_G3_PLL.PLL_SSC_LIMIT_SSC_MODE.PLL_SSC_LIMIT=&h6200&
	     *   PCIE_3_G3_PLL.PLL_SSC_STEP.PLL_SSC_STEP=&h13c&
	     *
	     */
	    if (PCIE_PHY_FREQUENCY(phc) == PHY_FREQ_REFCLK_80MHZ) {
	        wrdata = 0x6200; /* SSC Limit */
	    } else {
	        wrdata = 0x9c00; /* SSC Limit */
	    }
	    wrdata &=  ~G3_PLL_PLL_SSC_LIMIT_SSC_MODE_MODE;
	    wrdata |=  G3_PLL_PLL_SSC_LIMIT_SSC_MODE_DOWNSPREAD;
	    hcd_writel(wrdata, phc, G3_16FF_PLL_PLL_SSC_LIMIT_SSC_MODE_REG);

	    if (PCIE_PHY_FREQUENCY(phc) == PHY_FREQ_REFCLK_80MHZ) {
	        wrdata = 0x013c; /* SSC Step */
	    } else {
	        wrdata = 0x0330; /* SSC Step */
	    }
	    hcd_writel(wrdata, phc, G3_PLL_PLL_SSC_STEP_REG);

	    HCD_FN_EXT();

	    return;
	}

	/* 28HPM Serdes */
	/*
	 * Keep PLL2 in reset
	 * set PCIE_x_G3_PLL_PLL_RESETS (0xXXXXA010), RESETB and POST_RESETB
	 * (bit 1:0) to 2'b00
	 */
	rddata = hcd_readl(phc, G3_PLL_PLL_RESETS_REG);
	wrdata = rddata & (~G3_PLL_PLL_RESETS_RESETB | ~G3_PLL_PLL_RESETS_POST_RESETB);
	hcd_writel(wrdata, phc, G3_PLL_PLL_RESETS_REG);

	/*
	 * program PLL2 fractional divider
	 * set PCIE_x_G3_PLL_PLL_NDIV (0xXXXXA01C) to 0x1D
	 * set PCIE_x_G3_PLL_PLL_NDIV_FRAC_HOLDALL (0xXXXXA05C) to 0xECCCCD
	 */
	rddata = hcd_readl(phc, G3_PLL_PLL_NDIV_REG);
	wrdata = rddata & (~G3_PLL_PLL_NDIV_INT_MASK);
	wrdata |= G3_PLL_PLL_NDIV_INT(0x1D);
	hcd_writel(wrdata, phc, G3_PLL_PLL_NDIV_REG);

	rddata = hcd_readl(phc, G3_PLL_PLL_NDIV_FRAC_HOLDALL_REG);
	wrdata = rddata & (~G3_PLL_PLL_NDIV_FRAC_HOLDALL_FRAC_MASK);
	wrdata &= (~G3_PLL_PLL_NDIV_FRAC_HOLDALL_HOLA_ALL);
	wrdata |= G3_PLL_PLL_NDIV_FRAC_HOLDALL_NDIV_FRAC(0xECCCCD);
	hcd_writel(wrdata, phc, G3_PLL_PLL_NDIV_FRAC_HOLDALL_REG);

	/*
	 * bring PLL2 out of reset
	 * set PCIE_x_G3_PLL_PLL_RESETS (0xXXXXA010), RESETB and POST_RESETB
	 * (bit 1:0) to 2'b11
	 */
	rddata = hcd_readl(phc, G3_PLL_PLL_RESETS_REG);
	wrdata = rddata | G3_PLL_PLL_RESETS_RESETB|G3_PLL_PLL_RESETS_POST_RESETB;
	hcd_writel(wrdata, phc, G3_PLL_PLL_RESETS_REG);

	/*
	 * check PLL2 is locked
	 * check PCIE_x_G3_PLL_PLL_STAT (0xXXXXA03C), PLL_LOCK (bit 31) is 1
	 */
	waittime = PCIE_G3_PLL_LOCK_WAIT_TIMEOUT;
	rddata = 0;

	while (!(rddata & G3_PLL_PLL_STAT_PLL_LOCK) && waittime) {
	    rddata = hcd_readl(phc, G3_PLL_PLL_STAT_REG);
	    udelay(1000);
	    waittime--;
	}
	if (waittime == 0) {
	    HCD_ERROR("PCIe Core [%d] PLL LOCK timedout stat 0x%x\r\n",
	        phc->info.id, rddata);
	}

	HCD_FN_EXT();

	return;
}

/*
 *
 * Function bcm963xx_hc_gen3_phy_enable_ssc (phc,enable)
 *
 *   Parameters:
 *    phc   ... pointer to PCIe core HC data structure
 *    enable...flag to specify enable or disable SSC
 *
 *   Description:
 *    Enable/disable SSC. Assumed that SSC is configured before enabling the SSC
 *
 *   Return: 0:   on success or no action.
 *          -1:   on failure or timeout
 */
static int bcm963xx_hc_gen3_phy_enable_ssc(struct pcie_hc_core *phc,
	bool enable)
{
	uint32 rddata, wrdata;
	int waittime;

	if (PCIE_PHY_PROCESS(phc) == PHY_PROCESS_16FFC) {
	    rddata = hcd_readl(phc, G3_16FF_PLL_PLL_SSC_LIMIT_SSC_MODE_REG);
	    if (enable == TRUE) {
	        wrdata = rddata | G3_PLL_PLL_SSC_LIMIT_SSC_MODE_MODE;
	    } else {
	        wrdata = rddata & ~G3_PLL_PLL_SSC_LIMIT_SSC_MODE_MODE;
	    }
	    hcd_writel(wrdata, phc, G3_16FF_PLL_PLL_SSC_LIMIT_SSC_MODE_REG);

	    HCD_FN_EXT();

	    return 0;
	}

	/* 28HPM Serdes */
	if (enable == TRUE) {
	    /*
	     * enable SSC
	     * set PCIE_x_G3_PLL_PLL_SSC_STEP_VCOGAIN (0xXXXXA064) to 0x30068
	     * set PCIE_x_G3_PLL_PLL_SSC_LIMIT_SSC_MODE (0xXXXXA060) to 0x1001333
	     */
	    rddata = hcd_readl(phc, G3_PLL_PLL_SSC_STEP_VCOGAIN_REG);
	    wrdata = rddata & (~G3_PLL_PLL_SSC_STEP_VCOGAIN_VCO_GAIN_MASK);
	    wrdata |= G3_PLL_PLL_SSC_STEP_VCOGAIN_VCO_GAIN(3);
	    wrdata &= (~G3_PLL_PLL_SSC_STEP_VCOGAIN_SSC_STEP_MASK);
	    wrdata |= G3_PLL_PLL_SSC_STEP_VCOGAIN_SSC_STEP(0x0068);
	    hcd_writel(wrdata, phc, G3_PLL_PLL_SSC_STEP_VCOGAIN_REG);

	    rddata = hcd_readl(phc, G3_PLL_PLL_SSC_LIMIT_SSC_MODE_REG);
	    wrdata = rddata & (~G3_PLL_PLL_SSC_LIMIT_SSC_MODE_LIMIT_MASK);
	    wrdata |= G3_PLL_PLL_SSC_LIMIT_SSC_MODE_LIMIT(0x1333);
	    wrdata |= G3_PLL_PLL_SSC_LIMIT_SSC_MODE_MODE;
	    hcd_writel(wrdata, phc, G3_PLL_PLL_SSC_STEP_VCOGAIN_REG);

	    /*
	     * check that serdes PLL is locked
	     * set serdes MDIO_MMDSEL_AER_mdio_aer to 0x9FF
	     * check serdes PCIE_BLK1 (0x1100), status (reg 1), pll_lock (bit 11)
	     * is 1
	     */
	    /*
	     * Block:0xffd0 (AER), Register:0x1e (AER)
	     * Value:0x09ff bit[15:11] 1(PMA_PMD) bit[10:0] 0x1FF (broadcast)
	     *
	     *  tmp = pcie_mdio_write (0, &h1f&, &hffd0&) ' AER block
	     *  tmp = pcie_mdio_write (0, &h1e&, &h09ff&) ' PMA_PMD Broadcast
	     */
	    bcm963xx_hc_mdio_write(phc, 0, 0x1f, 0xffd0);
	    bcm963xx_hc_mdio_write(phc, 0, 0x1e, 0x09ff);

	    waittime = PCIE_G3_PLL_LOCK_WAIT_TIMEOUT;
	    rddata = 0;

	    /*
	     * Block:0x1100 (PCIE_BLK1), Register:0x01 (Status)
	     * Value:0x0800 bit[11] 1 (pll_lock)
	     */
	    bcm963xx_hc_mdio_write(phc, 0, 0x1f, 0x1100);
	    while (!(rddata & (1<<11)) && waittime) {
	    rddata = bcm963xx_hc_mdio_read(phc, 0, 0x01);
	        udelay(1000);
	        waittime--;
	    }

	    if (waittime == 0) {
	        HCD_ERROR("PCIe Core [%d] PLL LOCK timeout. Status [0x%x]\r\n",
	            phc->info.id, rddata);
	        HCD_FN_EXT();
	        return -1;
	    }
	} else {
	    rddata = hcd_readl(phc, G3_PLL_PLL_SSC_LIMIT_SSC_MODE_REG);
	    wrdata = rddata & ~G3_PLL_PLL_SSC_LIMIT_SSC_MODE_MODE;
	    hcd_writel(wrdata, phc, G3_PLL_PLL_SSC_STEP_VCOGAIN_REG);
	}

	HCD_FN_EXT();

	return 0;
}

/*
 *
 * Function bcm963xx_hc_phy_assert_iddq (phc, assert)
 *
 *
 *   Parameters:
 *    phc ... pointer to PCIe core HC data structure
 *    assert ... assert or de-assert
 *
 *   Description:
 *    Setup PCIe core power mode according to configuration
 *
 *   Return: 0 on success, -ve on failure
 */
static int bcm963xx_hc_phy_assert_iddq(struct pcie_hc_core *phc, bool assert)
{
	u32 reg_data;

	HCD_FN_ENT();

	if (phc->info.base == NULL) {
	    HCD_FN_RET_VAL(-EPERM);
	}

	reg_data = hcd_readl(phc, MISC_HARD_DEBUG_REG);
	if (assert == TRUE) {
	    /* Asserts IDDQ to SerDes for maximum power savings */
	    reg_data |= MISC_HARD_DEBUG_SERDES_IDDQ;
	} else {
	    /* de-asserts IDDQ to SerDes for power up */
	    reg_data &= ~MISC_HARD_DEBUG_SERDES_IDDQ;
	}
	hcd_writel(reg_data, phc, MISC_HARD_DEBUG_REG);

	HCD_INFO("Core [%d] %s assert serdes IDDQ\n", phc->info.id,
	    (assert == TRUE) ? "" : "De");

	HCD_FN_RET_VAL(0);
}

/*
 *
 * Function bcm963xx_hc_phy_config_pwrmode (phc)
 *
 *
 *   Parameters:
 *    phc ... pointer to PCIe core HC data structure
 *
 *   Description:
 *    Setup PCIe core power mode according to configuration
 *
 *   Return: 0 on success, -ve on failure
 */
static int bcm963xx_hc_phy_config_pwrmode(struct pcie_hc_core *phc)
{
	HCD_FN_ENT();

	/* Nothing to do, if configured to use default PHY power mode */
	if (phc->cfg.phypwrmode == 0)
	    return 0;

	/* Check if the host is configured for GEN1 speed */
	if (phc->cfg.speed == 1) {
	    HCD_LOG("[%d] Power mode [%d] is not supported in GEN1 Speeds",
	        phc->info.id, phc->cfg.phypwrmode);
	    return 0;
	}

	if (phc->cfg.phypwrmode == 1) {
	    int   lane;
	    uint16 block_addr;

	    /* Setting received from hardware team for  *** 63138B1 ***
	     * Also works for 63158, 4908, 47622 platforms
	     * For other platforms the values might need tuning
	     *
	     * Note:Below EP side changes also needed for Gen2-40nm Brcm SoC's
	     * pcieserdesreg 0x820, 0x16 0xa400
	     * pcieserdesreg 0x820, 0x17 0x05b7
	     * pcieserdesreg 0x801, 0x1a 0x4028
	     */
	    for (lane = 0; lane < phc->res.link_width; lane++) {
	        if (PCIE_PHY_PROCESS(phc) == PHY_PROCESS_16FFC) {
	            if (phc->info.gen == PCIE_LINK_SPEED_GEN3) {
	                /*
	                 * mdio write 0x1f,0xffd0
	                 * mdio write 0x1e, (0x800 + i)   // PMA_PMD Lane
	                 *
	                 * mdio write 0x1f, 0x5000  // TX DFE
	                 * mdio write 0x0b, 0x0ba1     // half-swing no pre-emphasis
	                 */
	                bcm963xx_hc_mdio_write(phc, 0, 0x1f, 0xffd0);
	                bcm963xx_hc_mdio_write(phc, 0, 0x1e, (0x0800 + lane));

	                block_addr = SERDES_TX_DFE0_LN0_REG;
	                bcm963xx_hc_mdio_write(phc, 0, 0x001f, block_addr);
	                bcm963xx_hc_mdio_write(phc, 0, 0x000b, 0x0ba1);
	            } else {
	                /*
	                 * mdio wirte 0x1f (0x5000 + i*0x10) ' point to per-lane
	                 * register set
	                 * mdio write 0x0d 0x1f00  ' set tx margin to half-swing,
	                 *                         ' no pre-emphasis
	                 */

	                block_addr = SERDES_TX_DFE0_LN0_REG;
	                block_addr += (lane * SERDES_TX_DFE0_LN_SIZE);
	                bcm963xx_hc_mdio_write(phc, 0, 0x001f, block_addr);
	                bcm963xx_hc_mdio_write(phc, 0, 0x000d, 0x1f00);
	            }
	        } else {
	            block_addr = SERDES_TX_CTR1_LN0_REG;
	            block_addr += (lane * SERDES_TX_CTR1_LN_SIZE);
	            bcm963xx_hc_mdio_write(phc, 0, 0x001f, block_addr);
	            bcm963xx_hc_mdio_write(phc, 0, 0x0001, 0x000b);
	            bcm963xx_hc_mdio_write(phc, 0, 0x0000, 0x0e20);

	            block_addr = SERDES_TX_DFE0_LN0_REG;
	            block_addr += (lane * SERDES_TX_DFE0_LN_SIZE);
	            bcm963xx_hc_mdio_write(phc, 0, 0x001f, block_addr);
	            bcm963xx_hc_mdio_write(phc, 0, 0x000d, 0x00f0);
	        }

	        /* Required to settle the power mode setting */
	        mdelay(10);
	    }

	    HCD_LOG("Port [%d] set to Low Power Mode (half swing)\n", phc->info.id);
	}

	HCD_FN_EXT();

	return 0;
}

/*
 *
 * Function bcm963xx_hc_phy_config_eq_presets (phc)
 *
 *
 *   Parameters:
 *    phc ... Pointer to PCIe core HC data structure
 *
 *   Description:
 *    Setup PCIe core GEN3 lanes Tx/Rx equalization presets
 *
 *   Return: 0 on success, -ve on failure
 */
static int bcm963xx_hc_phy_config_eq_presets(struct pcie_hc_core *phc)
{
	u32 reg_data;
	int lane;

	HCD_FN_ENT();

	/* Nothing to do, for GEN1/2 ports or configured to use default presets */
	if ((phc->info.gen != PCIE_LINK_SPEED_GEN3) ||
	    (phc->cfg.g3_eq_presets[0] == 0)) {
	    return 0;
	}

	/*
	 * Setting received from hardware team
	 *
	 * Program in RC register prior to link training, not used by HW until Gen3
	 * speed bringup
	 *
	 * PCIE_<port>_RC_CFG_PRIV1_PCIER_SECPCIE_EQ_CONTROL_0_ADDR (offset 0x638)
	 * Bits 14:12/30:28 Lane 0/1 EP Rx Preset Hint
	 * Bits 11:8/27:24 Lane 0/1  EP Tx Preset
	 * Bits 6:4/22:20 Lane 0/1 RC Rx Preset Hint
	 * Bits 3:0/19:16 Lane 0/1  RC Tx preset
	 */
	reg_data = hcd_readl(phc, RC_CFG_PRIV1_PCIER_SECPCE_EQ_CONTROL_0);
	for (lane = 0; lane < phc->res.link_width; lane++) {
	    if (phc->cfg.g3_eq_presets[lane]) {
	        u16 preset = (u16)phc->cfg.g3_eq_presets[lane];

	        /* Update with user settings */
	        reg_data &= (~(0xFFFF << (lane*16)));
	        reg_data |= (preset << (lane*16));
	    }
	}
	hcd_writel(reg_data, phc, RC_CFG_PRIV1_PCIER_SECPCE_EQ_CONTROL_0);

	HCD_LOG("Port [%d] Configured link equalization presets [0x%08x]\n",
	    phc->info.id, reg_data);

	HCD_FN_EXT();

	return 0;
}

/*
 *
 * Function bcm963xx_hc_gen2_phy_config (phc)
 *
 *
 *   Parameters:
 *    phc ... pointer to PCIe core HC data structure
 *
 *   Description:
 *     Configure PCIe PHY through MDIO interface for any workarounds or
 *     special features for Gen2 core based on Viper SerDes.
 *     The settings largely comes from ASIC design team
 *
 *   Return: None
 */
static void bcm963xx_hc_gen2_phy_config(struct pcie_hc_core *phc)
{
	struct bcm963xx_hc_cb *phc_cb = (struct bcm963xx_hc_cb *)(phc->phc_cb);

	HCD_FN_ENT();

	if (phc->info.gen != PCIE_LINK_SPEED_GEN2) {
	    /* Nothing to do for non gen2 cores */
	    HCD_FN_EXT();
	    return;
	}

	/*
	 * Populate the workarounds for Gen2 for the early Gen2 revisions
	 * All issues got fixed in revisions 3.03 and later
	 */
	if (phc_cb->wars.g2defset == 1) {

	    HCD_LOG("Port [%d] applying gen2 workarounds for core rev < 3.04\n",
	        phc->info.id);

	    /*
	     * VCO Calibration Timers
	     * Workaround:
	     * Block 0x3000, Register 0xB = 0x40
	     * Block 0x3000, Register 0xD = 7
	     * Notes:
	     * -Fixed in 63148A0,  63138B0 but ok to write anyway
	     */
	    bcm963xx_hc_mdio_write(phc, 0, 0x1f, 0x3000);
	    bcm963xx_hc_mdio_read (phc, 0, 0x1f);  /* just to exercise the read */
	    bcm963xx_hc_mdio_write(phc, 0, 0xB, 0x40);
	    bcm963xx_hc_mdio_write(phc, 0, 0xD, 7);

	    /*
	     * Reference clock output level
	     * Workaround:
	     * Block 0x2200, Register 3 = 0xaba4
	     * Note:
	     * -Fixed in 63148A0, 63138B0 but ok to write anyway
	     */
	    bcm963xx_hc_mdio_write(phc, 0, 0x1f, 0x2200);
	    bcm963xx_hc_mdio_write(phc, 0, 3, 0xaba4);

	    /*
	     * Tx Pre-emphasis
	     * Workaround:
	     * Block 0x4000, Register 0 = 0x1d20  // Gen1
	     * Block 0x4000, Register 1 = 0x12cd  // Gen1
	     * Block 0x4000, Register 3 = 0x0016  // Gen1, Gen2
	     * Block 0x4000, Register 4 = 0x5920  // Gen2
	     * Block 0x4000, Register 5 = 0x13cd  // Gen2
	     * Notes:
	     * -Fixed in 63148A0, 63138B0 but ok to write anyway
	     */
	    bcm963xx_hc_mdio_write(phc, 0, 0x1f, 0x4000);
	    bcm963xx_hc_mdio_write(phc, 0, 0, 0x1D20);
	    bcm963xx_hc_mdio_write(phc, 0, 1, 0x12CD);
	    bcm963xx_hc_mdio_write(phc, 0, 3, 0x0016);
	    bcm963xx_hc_mdio_write(phc, 0, 4, 0x5920);
	    bcm963xx_hc_mdio_write(phc, 0, 5, 0x13CD);

	    /*
	     * Rx Signal Detect
	     * Workaround:
	     * Block 0x6000, Register 5 = 0x2c0d
	     * Notes:
	     * -Fixed in 63148A0, 63138B0 but ok to write anyway
	     */
	    bcm963xx_hc_mdio_write(phc, 0, 0x1f, 0x6000);
	    bcm963xx_hc_mdio_write(phc, 0, 0x5, 0x2C0D);

	    /*
	     * Rx Jitter Tolerance
	     * Workaround:
	     * Block 0x7300, Register 3 = 0x190  // Gen1
	     * Block 0x7300, Register 9 = 0x194  // Gen2
	     * Notes:
	     * -Gen1 setting 63148A0, 63138B0 but ok to write anyway
	     * -Gen2 setting only in latest SerDes RTL  / future tapeouts
	     */
	    bcm963xx_hc_mdio_write(phc, 0, 0x1f, 0x7300);
	    bcm963xx_hc_mdio_write(phc, 0, 3, 0x190);
	    bcm963xx_hc_mdio_write(phc, 0, 9, 0x194);

	    /*
	     * Gen2 Rx Equalizer
	     * Workaround:
	     * Block 0x6000 Register 7 = 0xf0c8  // Gen2
	     * Notes:
	     * -New setting only in latest SerDes RTL / future tapeouts
	     */
	    bcm963xx_hc_mdio_write(phc, 0, 0x1f, 0x6000);
	    bcm963xx_hc_mdio_write(phc, 0, 7, 0xf0c8);


	    /*
	     * EP Mode PLL Bandwidth and Peaking
	     * Workaround:
	     * Block 0x2100, Register 0 = 0x5174
	     * Block 0x2100, Register 4 = 0x6023
	     * Notes:
	     * -Only needed for EP mode, but ok to write in RC mode too
	     * -New setting only in latest SerDes RTL / future tapeouts
	     */
	    bcm963xx_hc_mdio_write(phc, 0, 0x1f, 0x2100);
	    bcm963xx_hc_mdio_write(phc, 0, 0, 0x5174);
	    bcm963xx_hc_mdio_write(phc, 0, 4, 0x6023);
	}

	if (phc_cb->wars.g2pllcoupling == 1) {
	    /*
	     * Settings from SoC Design team
	     *
	     * This workaround nominally lower the frequency by -4766ppm
	     * allowing some margin compared to the -5000ppm limit
	     *
	     * mdio write 0x1f 0x2200  ' Set frequency setting to 99.5234 (-4700ppm)
	     * mdio write 0x00 0x5863  ' INT[9:0]=0x63, FRAC[17:16]=0x2
	     * mdio write 0x01 0x1800  ' FRAC[15:0]=0x1800
	     *
	     */
	    bcm963xx_hc_mdio_write(phc, 0, 0x1f, SERDES_PLL_AFE2_REGS);
	    /* ctrl0: override_ana_reg     bits:15 */
	    /* ctrl0: override_val         bits:14 */
	    /* ctrl0: frac_mode_sel[01:00] bits:13-12 */
	    /* ctrl0: pll_ndiv_frac[17:16] bits:11-10 */
	    /* ctrl0: int_val[9:0]         bits:9-0 */
	    bcm963xx_hc_mdio_write(phc, 0, 0x00, 0x5863);
	    /* ctrl1: pll_ndiv_frac[15-00] bits:15-0 */
	    bcm963xx_hc_mdio_write(phc, 0, 0x01, 0x1800);

	    HCD_LOG("Core [%d] Applied g2pllcoupling WAR\r\n", phc->info.id);
	}

	if (phc_cb->wars.phaseonlyckcmp == 1) {
	    /*
	     * Settings from SoC Design team
	     *
	     *  mdio write 0x1f, 0x7300 # RX_DFE3_LN0 register set
	     *  mdio write 0x0e, 0x2800 # mdio_ckcmp_phase_only_en bit-11=1,
	     *                          # value bit=13=1
	     *  mdio write 0x1f, 0x7310 # RX_DFE3_LN1 register set
	     *  mdio write 0x0e, 0x2800 # mdio_ckcmp_phase_only_en bit-11=1,
	     *                          # value bit=13=1
	     */
	    bcm963xx_hc_mdio_write(phc, 0, 0x1f, SERDES_RX_DFE3_REGS);
	    /* RX_DFE3_LN0_ctrlE.mdio_ckcmp_phase_only_en = 1 */
	    /* RX_DFE3_LN0_ctrlE.Reserved = 1 */
	    bcm963xx_hc_mdio_write(phc, 0, 0x0e, 0x2800);

	    bcm963xx_hc_mdio_write(phc, 0, 0x1f,
	        SERDES_RX_DFE3_REGS+SERDES_LN_REG);
	    /* RX_DFE3_LN0_ctrlE.mdio_ckcmp_phase_only_en = 1 */
	    /* RX_DFE3_LN0_ctrlE.Reserved = 1 */
	    bcm963xx_hc_mdio_write(phc, 0, 0x0e, 0x2800);

	    HCD_LOG("Core [%d] Applied phaseonlyckcmp WAR\r\n", phc->info.id);
	}

	HCD_FN_EXT();

	return;
}

/*
 *
 * Function bcm963xx_hc_gen3_phy_config (phc)
 *
 *
 *   Parameters:
 *    phc ... pointer to PCIe core HC data structure
 *
 *   Description:
 *     Configure PCIe PHY through MDIO interface for any workarounds or
 *     special features for GEN3 core based on Blackshark SerDes.
 *     The settings largely comes from ASIC design team
 *
 *   Return: None
 */
static void bcm963xx_hc_gen3_phy_config(struct pcie_hc_core *phc)
{
	struct bcm963xx_hc_cb *phc_cb = (struct bcm963xx_hc_cb *)(phc->phc_cb);
	uint16 rddata, wrdata;

	HCD_FN_ENT();

	if (phc->info.gen != PCIE_LINK_SPEED_GEN3) {

	    /* Nothing to do for gen3 or greater cores */
	    HCD_FN_EXT();
	    return;
	}

	if (phc_cb->wars.g3txclk == 1) {
	    int i;
	    uint16 pwrmgmt_reg_vals[PHY_PCIE_BLK2_PWR_MGMT_REGS];

	    HCD_LOG("Port [%d] applying GEN3 TxClock Start workaround\n",
	        phc->info.id);

	    /*
	     * For Gen3, first Select the lane
	     *
	     * Block:0xffd0 (AER), Register:0x1e (AER)
	     * Value:0x09ff bit[15:11] 1(PMA_PMD) bit[10:0] 0x000 (lane0)
	     *
	     *   tmp = pcie_mdio_write (0, &h1f&, &hffd0&) ' AER block
	     *   tmp = pcie_mdio_write (0, &h1e&, &h0800&) ' PMA_PMD Common
	     */
	    bcm963xx_hc_mdio_write(phc, 0, 0x1f, 0xffd0);
	    bcm963xx_hc_mdio_write(phc, 0, 0x1e, 0x0800);

	    /*
	     * Block:0x1000 (PCIE_BLK0), Register:0x07 (tx_seq_ctrl)
	     * bit[12] 1 (tca_one_shot_disable) bit[1]] 1 (tca_fine_tune_en)
	     *
	     * ' disable one shot (bit-12), enable fine tune tca (bit-11)
	     * ' which allows pclk to be active even if txclk is stuck
	     * tmp = pcie_mdio_write (0, &h1f&, &h1000&) ' PCIE_BLK0
	     * rddata = pcie_mdio_read (0, &h07&) ' tx_seq_ctrl
	     * wrdata = (rddata And Not(&h1800)) Or lshift(1,12) Or lshift(1,11)
	     * tmp = pcie_mdio_write (0, &h07&, wrdata)  ' tx_seq_ctrl
	     */
	    bcm963xx_hc_mdio_write(phc, 0, 0x1f, 0x1000);
	    rddata = bcm963xx_hc_mdio_read(phc, 0, 0x07);
	    wrdata = rddata | (1 << 12) | (1<< 11);
	    bcm963xx_hc_mdio_write(phc, 0, 0x07, wrdata);

	    /*
	     * ' need to deassert PHY reset for next step
	     * pcie_perst_reset(DEASSERT)
	     */
	    bcm963xx_hc_core_set_reset(phc, FALSE);

	    /*
	     * Block:0x1200 (PCIE_BLK2), Register:0x01,2,3,4
	     * (PCIE_BLK2_PwrMgmt0,1,2,3)
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
	    bcm963xx_hc_mdio_write(phc, 0, 0x1f, 0x1200);
	    for (i = 0; i < PHY_PCIE_BLK2_PWR_MGMT_REGS; i++) {
	        pwrmgmt_reg_vals[i] = bcm963xx_hc_mdio_read(phc, 0, i+1);
	        wrdata = 0x0808;
	        bcm963xx_hc_mdio_write(phc, 0, i+1, wrdata);
	        wrdata = 0x0000;
	        bcm963xx_hc_mdio_write(phc, 0, i+1, wrdata);
	    }

	    /*
	     * ' Assert PHY reset to get out of tx powerdown
	     *     pcie_perst_reset(ASSERT)
	     */
	    bcm963xx_hc_core_set_reset(phc, TRUE);

	    /*
	     * Block:0xffd0 (AER), Register:0x1e (AER)
	     * Value:0x09ff bit[15:11] 1(PMA_PMD) bit[10:0] 0x1FF (broadcast)
	     *
	     *     tmp = pcie_mdio_write (0, &h1f&, &hffd0&) ' AER block
	     *     tmp = pcie_mdio_write (0, &h1e&, &h09ff&) ' PMA_PMD Broadcast
	     */
	    bcm963xx_hc_mdio_write(phc, 0, 0x1f, 0xffd0);
	    bcm963xx_hc_mdio_write(phc, 0, 0x1e, 0x09ff);
	    /*
	     * Block: 0xd3b0 (AMS_TX) Register:0x01 (TX_CTRL_1)
	     * Value: clear dcc_en (bit 15)
	     *
	     *     wrdata = &h0718& ' clear DCC Enable bit-15
	     *     (default for register is &h8718&
	     *     tmp = pcie_mdio_write (0, &h1f&, &hd3b0&) ' AMS Tx
	     *     tmp = pcie_mdio_write (0, &h01&, wrdata)
	     *      ' clear register 1, bit-15 (default for register is &h8718&)
	     */
	    bcm963xx_hc_mdio_write(phc, 0, 0x1f, 0xd3b0);
	    rddata = bcm963xx_hc_mdio_read(phc, 0, 0x01);
	    wrdata = rddata & (~0x8000);
	    bcm963xx_hc_mdio_write(phc, 0, 0x01, wrdata);

	    /*
	     *     for lane = 0 to numlanes-1
	     *     tmp = pcie_mdio_write (0, &h1f&, &hffd0&) ' AER block
	     *     tmp = pcie_mdio_write (0, &h1e&, (&h800 Or lane))
	     *     ' PMA_PMD Broadcast
	     *     tmp = pcie_mdio_write (0, &h1f&, &hd3b0&) ' AMS Tx
	     *     rddata = pcie_mdio_read (0, &h01&)
	     *     if rddata <> wrdata then
	     *     print "Error in txclk_start_workaround, lane=" & lane & ":
	     *      expecting &h" & hex16(wrdata) & ", read back &h" & hex16(rddata)
	     *     status = 1
	     *     end if
	     *     next
	     */
	    for (i = 0; i < phc->res.link_width; i++) {
	        bcm963xx_hc_mdio_write(phc, 0, 0x1f, 0xffd0);
	        bcm963xx_hc_mdio_write(phc, 0, 0x1e, 0x0800|i);
	        bcm963xx_hc_mdio_write(phc, 0, 0x1f, 0xd3b0);
	        rddata = bcm963xx_hc_mdio_read(phc, 0, 0x01);
	        if (rddata != wrdata) {
	            HCD_ERROR("Core [%d] Lane [%d] Error in txclk_start_war,",
	                phc->info.id, i);
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
	    bcm963xx_hc_mdio_write(phc, 0, 0x1f, 0xffd0);
	    bcm963xx_hc_mdio_write(phc, 0, 0x1f, 0x0800);

	    /*
	     * Block:0x1000 (PCIE_BLK0), Register:0x07 (tx_seq_ctrl)
	     * bit[12] 1 (tca_one_shot_disable) bit[1]] 1 (tca_fine_tune_en)
	     *
	     *     ' re-enable one shot, clear the disable (bit-12),
	     *     disable fine tune tca (bit-11)
	     *     tmp = pcie_mdio_write (0, &h1f&, &h1000&) ' PCIE_BLK0
	     *     rddata = pcie_mdio_read (0, &h07&) ' tx_seq_ctrl
	     *     wrdata = (rddata And Not(&h1800)) Or lshift(0,12) Or lshift(0,11)
	     *     tmp = pcie_mdio_write (0, &h07&, wrdata)  ' tx_seq_ctrl
	     */
	    bcm963xx_hc_mdio_write(phc, 0, 0x1f, 0x1000);
	    rddata = bcm963xx_hc_mdio_read(phc, 0, 0x07);
	    wrdata = rddata | (1 << 12) | (1<< 11);
	    bcm963xx_hc_mdio_write(phc, 0, 0x07, wrdata);

	    /*
	     *     tmp = pcie_mdio_write (0, &h1f&, &h1200&) ' PCIE_BLK2
	     *     for i = 0 to 3
	     *     tmp = pcie_mdio_write (0, i+1, vals(i))
	     *     next
	     */
	    bcm963xx_hc_mdio_write(phc, 0, 0x1f, 0x1200);
	    for (i = 0; i < PHY_PCIE_BLK2_PWR_MGMT_REGS; i++) {
	        bcm963xx_hc_mdio_write(phc, 0, i+1, pwrmgmt_reg_vals[i]);
	    }
	}

	if (phc_cb->wars.g3rxset == 1) {

	   HCD_LOG("Port [%d] applying GEN3 Rx Default Settings workaround\n",
	       phc->info.id);

	    /*
	     * Block:0xffd0 (AER), Register:0x1e (AER)
	     * Value:0x09ff bit[15:11] 1(PMA_PMD) bit[10:0] 0x1FF (broadcast)
	     *
	     *  tmp = pcie_mdio_write (0, &h1f&, &hffd0&) ' AER block
	     *  tmp = pcie_mdio_write (0, &h1e&, &h09ff&) ' PMA_PMD Broadcast
	     */
	    bcm963xx_hc_mdio_write(phc, 0, 0x1f, 0xffd0);
	    bcm963xx_hc_mdio_write(phc, 0, 0x1e, 0x09ff);


	    /*
	     * ' ----------------------------------------------------------------
	     * ' The following flips DC offset loop polarity for all 3 PCIe gens
	     * ' ----------------------------------------------------------------
	     *      tmp = pcie_mdio_write (0, &h1f&, &hd240&)
	     *      tmp = pcie_mdio_write (0, &h00&, &h320c&) ' default &h120c&
	     *
	     *      tmp = pcie_mdio_write (0, &h1f&, &hd2a0&)
	     *      tmp = pcie_mdio_write (0, &h01&, &h000b&) ' default &h0003&
	     */

	    /*
	     * Block:0xd240 (TRNSUM_B), Register:0x00 (rx_trnsum_b_ctrl_0)
	     * Value:0x320c bit[13] 1 (rg_dc_offset_grad_inv)
	     */
	    bcm963xx_hc_mdio_write(phc, 0, 0x1f, 0xd240);
	    rddata = bcm963xx_hc_mdio_read(phc, 0, 0x00);
	    wrdata = rddata | (1 << 13);
	    bcm963xx_hc_mdio_write(phc, 0, 0x00, wrdata);

	    /*
	     * Block:0xd2a0 (AEQ), Register:0x01 (rx_aeq_ctrl_1)
	     * Value:0x000b bit[3] 1(rg_invert_os)
	     */
	    bcm963xx_hc_mdio_write(phc, 0, 0x1f, 0xd2a0);
	    rddata = bcm963xx_hc_mdio_read(phc, 0, 0x01);
	    wrdata = rddata | (1 << 3);
	    bcm963xx_hc_mdio_write(phc, 0, 0x01, wrdata);

	    /*
	     * ' ----------------------------------------------------------------
	     * ' The following adjust CDR behavior
	     * ' ----------------------------------------------------------------
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
	    bcm963xx_hc_mdio_write(phc, 0, 0x1f, 0xd2d0);

	    rddata = bcm963xx_hc_mdio_read(phc, 0, 0x06);
	    wrdata = (rddata & (~0x01c0)) | (3 << 6);
	    bcm963xx_hc_mdio_write(phc, 0, 0x06, wrdata);

	    /*
	     * Block:0xd2d0 (CDR_C), Register:0x07 (rx_cdr_c_CONTROL_7)
	     * Value:0x3653 bit[8:6] 1 (rg_acq_prop_bw_g2)
	     */
	    rddata = bcm963xx_hc_mdio_read(phc, 0, 0x07);
	    wrdata = (rddata & (~0x01c0)) | (1 << 6);
	    bcm963xx_hc_mdio_write(phc, 0, 0x07, wrdata);

	    /*
	     * Block:0xd2d0 (CDR_C), Register:0x08 (rx_cdr_c_CONTROL_8)
	     * Value:0x3653 bit[8:6] 1 (rg_acq_prop_bw_g2)
	     */
	    rddata = bcm963xx_hc_mdio_read(phc, 0, 0x08);
	    wrdata = (rddata & (~0x01c0)) | (1 << 6);
	    bcm963xx_hc_mdio_write(phc, 0, 0x08, wrdata);

	    /*
	     * Block:0xd2d0 (CDR_C), Register:0x09 (rx_cdr_c_CONTROL_9)
	     * Value:0x3653 bit[8:6] 1 (rg_acq_prop_bw_g2)
	     */
	    rddata = bcm963xx_hc_mdio_read(phc, 0, 0x09);
	    wrdata = (rddata & (~0x01c0)) | (1 << 6);
	    bcm963xx_hc_mdio_write(phc, 0, 0x09, wrdata);

	    /*
	     * Block:0xd2d0 (CDR_C), Register:0x02 (rx_cdr_c_CONTROL_2)
	     * Value:0x085e bit[13:5] 1 (rg_mon_offset_val_g0)
	     */
	    rddata = bcm963xx_hc_mdio_read(phc, 0, 0x02);
	    wrdata = (rddata & (~0x1fe0)) | (66 << 5);
	    bcm963xx_hc_mdio_write(phc, 0, 0x02, wrdata);

	    /*
	     * Block:0xd2d0 (CDR_C), Register:0x01 (rx_cdr_c_CONTROL_1)
	     * Value:0x085e bit[13:5] 1 (rg_mon_offset_val_g0)
	     */
	    rddata = bcm963xx_hc_mdio_read(phc, 0, 0x01);
	    wrdata = (rddata & (~0x1fe0)) | (66 << 5);
	    bcm963xx_hc_mdio_write(phc, 0, 0x01, wrdata);

	    /*
	     * Block:0xd2d0 (CDR_C), Register:0x00 (rx_cdr_c_CONTROL_0)
	     * Value:0x085e bit[13:5] 1 (rg_mon_offset_val_g0)
	     */
	    rddata = bcm963xx_hc_mdio_read(phc, 0, 0x00);
	    wrdata = (rddata & (~0x1fe0)) | (66 << 5);
	    bcm963xx_hc_mdio_write(phc, 0, 0x00, wrdata);

	    /*
	     * ' ----------------------------------------------------------------
	     * ' The following adjust VGA behavior
	     * ' ----------------------------------------------------------------
	     *
	     *        tmp = pcie_mdio_write (0, &h1f&, &hd240&)
	     *        tmp = pcie_mdio_write (0, &h04&, &hcc66&) ' default &hcc22&
	     */

	    /*
	     * Block:0xd240 (TRNSUM_B), Register:0x04 (rx_trnsum_b_ctrl_4)
	     * Value:0xcc66 bit[7:4] 6 (rg_vga_pattern) bit[3:0] 6
	     * (rg_vga_pattern_bit_en)
	     */
	    bcm963xx_hc_mdio_write(phc, 0, 0x1f, 0xd240);
	    rddata = bcm963xx_hc_mdio_read(phc, 0, 0x04);
	    wrdata = (rddata & (~0x00ff)) | (6 << 0) | (6 << 4);
	    bcm963xx_hc_mdio_write(phc, 0, 0x04, wrdata);

	    /*
	     * ' ----------------------------------------------------------------
	     * ' The following adjust DFE behavior
	     * ' ----------------------------------------------------------------
	     *
	     *        tmp = pcie_mdio_write (0, &h1f&, &hd240&)
	     *        tmp = pcie_mdio_write (0, &h05&, &h6a1c&) ' default &h681c&
	     *        tmp = pcie_mdio_write (0, &h06&, &h0a1c&) ' default &h081c&
	     */
	    bcm963xx_hc_mdio_write(phc, 0, 0x1f, 0xd240);

	    /*
	     * Block:0xd240 (TRNSUM_B), Register:0x05 (rx_trnsum_b_ctrl_5)
	     * Value:0x6a1c bit[9] 1 (rg_dfe_1_cmn_only)
	     */
	    rddata = bcm963xx_hc_mdio_read(phc, 0, 0x05);
	    wrdata = (rddata | (1 << 9));
	    bcm963xx_hc_mdio_write(phc, 0, 0x05, wrdata);

	    /*
	     * Block:0xd240 (TRNSUM_B), Register:0x06 (rx_trnsum_b_ctrl_6)
	     * Value:0x0a1c bit[9] 1 (rg_dfe_2_cmn_only)
	     */
	    rddata = bcm963xx_hc_mdio_read(phc, 0, 0x06);
	    wrdata = (rddata | (1 << 9));
	    bcm963xx_hc_mdio_write(phc, 0, 0x06, wrdata);
	}

	HCD_FN_EXT();

	return;
}

/*
 *
 * Function bcm963xx_hc_phy_config (phc)
 *
 *
 *   Parameters:
 *    phc ... pointer to PCIe core HC data structure
 *
 *   Description:
 *     Configure PCIe PHY through MDIO interface for any workarounds or
 *     special features. The settings largely comes from ASIC design team
 *
 *   Return: None
 */
static void bcm963xx_hc_phy_config(struct pcie_hc_core *phc)
{
	struct bcm963xx_hc_cb *phc_cb = (struct bcm963xx_hc_cb *)(phc->phc_cb);
	uint16 data = 0;

	HCD_FN_ENT();

	if (PCIE_PHY_SERDES(phc) >= PHY_SERDES_MERLIN8) {
	    HCD_LOG("%s not yet supported\n", __FUNCTION__);
	    return;
	}

	HCD_INFO("applying serdes parameters chipid [0x%x] chiprev [0x%x]\n",
	    pmc_hcd_get_chip_id(), pmc_hcd_get_chip_rev());

	/* configure Resistor calibration */
	bcm963xx_hc_phy_config_rescal(phc);

	if (phc_cb->wars.bifp1sysclk == 1) {

	    HCD_LOG("Port [%d] applying sys clock workaround part-2\n",
	        phc->info.id);

	    /*
	     * Workaround PART-2
	     * Set SerDes MDIO Block 0x1000, Register 1
	     * bit-12 = 1 (mdio_RC_refclk_sel)
	     * bit-13 = 1 (mdio_RC_refclk_val)
	     *
	     */
	    bcm963xx_hc_mdio_write(phc, 0, 0x1f, 0x1000);
	    data = bcm963xx_hc_mdio_read(phc, 0, 1);
	    data |= (3 << 12);
	    bcm963xx_hc_mdio_write(phc, 0, 1, data);
	}

	if (phc->info.gen == PCIE_LINK_SPEED_GEN2) {
	    /* Configure GEN2 PHY workarounds */
	    bcm963xx_hc_gen2_phy_config(phc);
	} else if (phc->info.gen == PCIE_LINK_SPEED_GEN3) {
	    /* Configure GEN3 PHY workarounds */
	    bcm963xx_hc_gen3_phy_config(phc);
	}

	/* Configure SSC */
	bcm963xx_hc_config_ssc(phc);

	/* Disable SSC, will be enabled after reset if link is up (enable= FALSE) */
	bcm963xx_hc_enable_ssc(phc, FALSE);

	/* Set PHY power mode if configured */
	bcm963xx_hc_phy_config_pwrmode(phc);

	/* Set PHY link equalization presets */
	bcm963xx_hc_phy_config_eq_presets(phc);

	/* Allow settling time */
	mdelay(10);

	HCD_FN_EXT();

	return;
}

/*
 * Function bcm963xx_hc_phy_config_phyrxfilter (phc, inreset)
 *
 *   Parameters:
 *    phc ... pointer to PCIe core HC data structure
 *    inreset  flag to specify SoC in Reset or not
 *
 *   Description:
 *    Configure Rx signal detection filter for RC and EP
 *
 *   Return: Status of configuration
 *     0:       Success
 *     -EAGAIN: Try again
 *     -ENODEV: Failure
 */
static int bcm963xx_hc_phy_config_phyrxfilter(struct pcie_hc_core *phc,
	bool inreset)
{
	int rc = 0;
	u16 rddata;
	u16 phyblk;
	int speed;
	u16 phyreg;

	HCD_FN_ENT();

	if (phc->cfg.phyrxfilter == 0) {
	    HCD_FN_EXT();
	    return rc;
	}

	speed = bcm963xx_hc_core_get_speed(phc);

	if (inreset == TRUE) {
	    if (phc->cfg.phyrxfilter & HCD_DEV_EP) {
	        if (speed != PCIE_LINK_SPEED_GEN1) {
	            /* Switch to Gen1 Sped */
	            bcm963xx_hc_core_set_speed(phc, PCIE_LINK_SPEED_GEN1);
	            if (!phc->cfg.speed) {
	                phc->cfg.speed = speed;
	            }
	            HCD_INFO("phyrxfilter: Switch GEN%d to GEN1\n", speed);
	       }
	    }

	    if (phc->cfg.phyrxfilter & HCD_DEV_RC) {

	        HCD_LOG("Core [%d] applying phyrxfilter for RC\n",
	            phc->info.id);

	        /*
	         * 28nm:
	         * =====
	         * Block:0xf600 (RX_AFE0_BC), Register:07 (ctrlg1)
	         * mdio write 0x1f 0xf600 ' Default 0xF0c8
	         *     Set filter to 0xff in bits 15:8 (70-80ns)
	         * mdio write 0x07 0xFFC8
	         *
	         * 16nm:
	         * =====
	         * Block:0xf600 (RX_AFE0_BC), Register:07 (ctrl1)
	         * mdio write 0x1f 0xf600 ' Default 0xF000
	         *     Set filter to 0xff in bits 15:8 (70-80ns)
	         * mdio write 0x01 0xFF00
	         */
	        phyblk = mdio_get_read_blk_addr(phc, SERDES_RX_AFE0_REGS, 0);
	        bcm963xx_hc_mdio_write(phc, 0, 0x1f, phyblk);
	        phyreg = 0x07; /* 28nm PHY ctrlg1 register */
	        rddata = bcm963xx_hc_mdio_read(phc, 0, phyreg);

	        if ((rddata & 0x00ff) == 0x0000) {
	            /* 16nm PHY returns 0 as reg#7 doesn't exist */
	            phyreg = 0x01;  /* use 16nm ctrl1 register */
	            rddata = bcm963xx_hc_mdio_read(phc, 0, phyreg);
	        }
	        rddata |= 0xff00;
	        phyblk = mdio_get_write_blk_addr(phc, SERDES_RX_AFE0_REGS);
	        bcm963xx_hc_mdio_write(phc, 0, 0x1f, phyblk);
	        bcm963xx_hc_mdio_write(phc, 0, phyreg, rddata);

	        /*
	         * Block:0xf700 (RX_DEF0_BC), Register:09 (control2)
	         * mdio write 0x1f 0xf700 ' Default 0x1800.
	         *     Clear bit mdio_FiltBypass bit-11 (enable filter) and
	         *     clear mdio_FiltInGen1 bit-10=0 (Gen 1 not enabled)
	         * mdio write 0x09 0x1000
	         */
	        phyblk = mdio_get_read_blk_addr(phc, SERDES_RX_DFE0_REGS, 0);
	        bcm963xx_hc_mdio_write(phc, 0, 0x1f, phyblk);
	        phyreg = 0x09;
	        rddata = bcm963xx_hc_mdio_read(phc, 0, phyreg);
	        rddata &= ~(1 << 11);
	        rddata &= ~(1 << 10);

	        phyblk = mdio_get_write_blk_addr(phc, SERDES_RX_DFE0_REGS);
	        bcm963xx_hc_mdio_write(phc, 0, 0x1f, phyblk);
	        bcm963xx_hc_mdio_write(phc, 0, phyreg, rddata);

	        HCD_INFO("phyrxfilter: Enabled for RC\n");
	    }

	    goto done;
	} else if (phc->cfg.phyrxfilter & HCD_DEV_EP) {
	    struct pci_bus* root_bus;
	    struct wlep_dev* wldev;

	    HCD_LOG("Core [%d] applying phyrxfilter for EP\n",
	        phc->info.id);

	    /* Setup BAR, OWIN .... */
	    rc = bcm963xx_hc_core_pre_config(phc);
	    if (rc) {
	        HCD_ERROR("phyrxfilter: setup RC hw, err [%d]\r\n", rc);
	        goto done;
	    }

	    /* Get root bus */
	    root_bus = pcie_hcd_get_rootbus(phc->pdrv);
	    if (!root_bus) {
	        HCD_ERROR("Failed to get root bus\n");
	        rc = -ENOMEM;
	        goto done;
	    }

	    /* Probe WLAN device */
	    wldev = wlep_probe_dev(root_bus, phc->res.owin[0].start, &rc);
	    if (wldev == NULL) {
	        HCD_ERROR("phyrxfilter: probe device, err [%d]\n", rc);
	        goto done;
	    }

	    rc = wlep_config_dev(wldev);
	    if (rc != 0) {
	        HCD_ERROR("phyrxfilter: configure device, err [%d]\n", rc);
	        goto done;
	    }
	    HCD_INFO("phyrxfilter: Enumerated EP\n");

	    /*
	     * 28nm:
	     * =====
	     * Block:0xf600 (RX_AFE0_BC), Register:07 (ctrlg1)
	     * mdio write 0x1f 0xf600 ' Default 0xF0c8
	     *     Set filter to 0xff in bits 15:8 (70-80ns)
	     * mdio write 0x07 0xFFC8
	     *
	     * 16nm:
	     * =====
	     * Block:0xf600 (RX_AFE0_BC), Register:07 (ctrl1)
	     * mdio write 0x1f 0xf600 ' Default 0xF000
	     *     Set filter to 0xff in bits 15:8 (70-80ns)
	     * mdio write 0x01 0xFF00
	     */
	    wlep_mdio_write(wldev, 0, 0x1f, SERDES_RX_AFE0_REGS);
	    phyreg = 0x07; /* 28nm PHY ctrlg1 register */
	    rddata = wlep_mdio_read(wldev, 0, phyreg);

	    if ((rddata & 0x00ff) == 0x0000) {
	        /* 16nm PHY returns 0 as reg#7 doesn't exist */
	        phyreg = 0x01;  /* use 16nm ctrl1 register */
	        rddata = wlep_mdio_read(wldev, 0, phyreg);
	    }
	    rddata |= 0xff00;

	    /* Broadcast write on all supported lanes */
	    wlep_mdio_write(wldev, 0, 0x1f, SERDES_RX_AFE0_BC_REGS);
	    wlep_mdio_write(wldev, 0, phyreg, rddata);

	    /*
	     * Block:0xf700 (RX_DEF0_BC), Register:09 (control2)
	     * mdio write 0x1f 0xf700 ' Default 0x1800.
	     *     Clear bit mdio_FiltBypass bit-11 (enable filter) and
	     *     clear mdio_FiltInGen1 bit-10=0 (Gen 1 not enabled)
	     * mdio write 0x09 0x1000
	     */
	    wlep_mdio_write(wldev, 0, 0x1f, SERDES_RX_DFE0_REGS);
	    phyreg = 0x09;
	    rddata = wlep_mdio_read(wldev, 0, phyreg);
	    rddata &= ~(1 << 11);
	    rddata &= ~(1 << 10);

	    /* Broadcast write on all supported lanes */
	    wlep_mdio_write(wldev, 0, 0x1f, SERDES_RX_DFE0_BC_REGS);
	    wlep_mdio_write(wldev, 0, phyreg, rddata);

	    HCD_INFO("phyrxfilter: Enabled for EP\n");

	    {
	        int ln;

	        HCD_INFO("phyrxfilter: Dump MDIO\n");
	        HCD_INFO("======================\n");

	        for (ln = 0; ln < phc->res.link_width; ln++) {
	            phyblk = mdio_get_read_blk_addr(phc, SERDES_RX_AFE0_REGS, ln);
	            phyreg = 0x07; /* 28nm PHY ctrlg1 register */
	            bcm963xx_hc_mdio_write(phc, 0, 0x1f, phyblk);
	            rddata = bcm963xx_hc_mdio_read(phc, 0, phyreg);
	            if ((rddata & 0x00ff) == 0x0000) {
	                /* 16nm PHY returns 0 as reg#7 doesn't exist */
	                phyreg = 0x01;  /* use 16nm ctrl1 register */
	                rddata = bcm963xx_hc_mdio_read(phc, 0, phyreg);
	            }
	            HCD_INFO("phyrxfilter: RC mdio_read 0x%04x, 0x%x = 0x%04x\n",
	                phyblk, phyreg, rddata);

	            phyblk = mdio_get_read_blk_addr(phc, SERDES_RX_DFE0_REGS, ln);
	            phyreg = 0x09;
	            bcm963xx_hc_mdio_write(phc, 0, 0x1f, phyblk);
	            rddata = bcm963xx_hc_mdio_read(phc, 0, phyreg);
	            HCD_INFO("phyrxfilter: RC mdio_read 0x%04x, 0x%x = 0x%04x\n",
	                phyblk, phyreg, rddata);
	        }

	        for (ln = 0; ln < wlep_get_mlw(wldev); ln++) {
	            phyblk = SERDES_RX_AFE0_REGS + (ln * SERDES_LN_REG);
	            phyreg = 0x07; /* 28nm PHY ctrlg1 register */
	            wlep_mdio_write(wldev, 0, 0x1f, phyblk);
	            rddata = wlep_mdio_read(wldev, 0, phyreg);
	            if ((rddata & 0x00ff) == 0x0000) {
	                /* 16nm PHY returns 0 as reg#7 doesn't exist */
	                phyreg = 0x01;  /* use 16nm ctrl1 register */
	                rddata = wlep_mdio_read(wldev, 0, phyreg);
	            }
	            HCD_INFO("phyrxfilter: EP mdio_read 0x%04x, 0x%x = 0x%04x\n",
	                phyblk, phyreg, rddata);

	            phyblk = SERDES_RX_DFE0_REGS + (ln * SERDES_LN_REG);
	            phyreg = 0x09;
	            wlep_mdio_write(wldev, 0, 0x1f, phyblk);
	            rddata = wlep_mdio_read(wldev, 0, phyreg);
	            HCD_INFO("phyrxfilter: EP mdio_read 0x%04x, 0x%x = 0x%04x\n",
	                phyblk, phyreg, rddata);
	        }
	        HCD_INFO("======================\n");
	    }

	    /* Clean up */
	    wlep_free_dev(wldev);

	    /* Switch to original Speed */
	    if (phc->cfg.speed) {
	        bcm963xx_hc_core_set_speed(phc, phc->cfg.speed);
	        HCD_INFO("phyrxfilter: Change GEN%d -> GEN%d\n", speed,
	            phc->cfg.speed);
	    }
	}

done:
	HCD_FN_EXT();
	return rc;
}

/*
 *
 * Function bcm963xx_hc_core_config_ecam (phc)
 *
 *
 * Parameters:
 *  phc ... pointer to PCIe core HC data structure
 *
 * Description:
 *   Configure PCIe Enhanced Configuration Access Method (ECAM)
 *
 *  Return: 0 on success, -ve on failure
 */
static int bcm963xx_hc_core_config_ecam(struct pcie_hc_core *phc)
{
	u32 reg_data;
	struct resource *ecam = &(phc->res.ecam);

	HCD_FN_ENT();

	/*
	 * 31:20 LIMIT = ECAM_End[31:20]
	 * 19:16 Reserved
	 * 15:04 BASE  = ECAM_Start[31:20]
	 * 03:00 Reserved
	 */
	reg_data = ecam->end & MISC_EXT_ECAM_BASE_LIMIT_LIMIT_MASK;
	reg_data |= ((ecam->start >> 16) &  MISC_EXT_ECAM_BASE_LIMIT_BASE_MASK);
	hcd_writel(reg_data, phc, MISC_EXT_ECAM_BASE_LIMIT_REG);

#ifdef CONFIG_PHYS_ADDR_T_64BIT
	reg_data = ((ecam->start >> 32) & MISC_EXT_ECAM_WIN_HI_BASE_MASK);
#else /* !CONFIG_PHYS_ADDR_T_64BIT */
	reg_data = 0x00;
#endif /* !CONFIG_PHYS_ADDR_T_64BIT */
	hcd_writel(reg_data, phc, MISC_EXT_ECAM_WIN_HI_REG);

	/* Allow returning 0xffffffff on bridge master interface errors */
	reg_data = OB_ERR_HNDL_ERR_TREATMENT_CFG_ERR_REPLACE;
	hcd_writel(reg_data, phc, OB_ERR_HNDL_ERR_TREATMENT_REG);

	HCD_LOG("Core [%d] configured ECAM %pR\n", phc->info.id, ecam);

	HCD_FN_RET_VAL(0);
}

/*
 *
 * Function bcm963xx_hc_core_config_fastpath (phc, en)
 *
 *
 * Parameters:
 *  phc ... pointer to PCIe core HC data structure
 *  en  ... enable flag for FASTPATH_IN or FASTPATH_OUT
 *
 * Description:
 *   Configure Mutex for the incoming lock traffic
 *
 *  Return: 0 on success, -ve on failure
 */
static int bcm963xx_hc_core_config_fastpath(struct pcie_hc_core *phc, u32 en)
{
	u32 reg_data;
	bool exclusive = true;
	u64 addr = 0;
#if defined(UBUS_PORT_ID_MLO) || defined(UBUS_PORT_ID_MLO1)
	int req_id;
#endif /* UBUS_PORT_ID_MLO || UBUS_PORT_ID_MLO1 */

	HCD_FN_ENT();

	en &= FASTPATH_EN_MASK;
	if ((phc->info.rev < 0x406) || !(phc->cfg.fastpath & en)) {
	    /* Not supported or Disabled, Nothing to do */
	    return 0;
	}

	if (en & FASTPATH_EN_IN) {
	    if (phc->cfg.fastpath & FASTPATH_NONEXCLUSIVE) {
	        exclusive = false;
	    }

	    addr = FASTPATH_PCIE_IN_BASE;

	    HCD_LOG("Core [%d] Configuring Incoming Fastpath\n", phc->info.id);

	    /*
	     * -----------------------
	     * EP <- PCIe Bus -> RC
	     * -----------------------
	     */
	    /* Configure incoming Mutex address */
	    reg_data = (addr & FASTPATH_IN_ADDR_LO_BASE_ADDRESS_MASK);
	    reg_data |= FASTPATH_IN_ADDR_MUTEX_VISIBILITY;
	    if (exclusive == true) {
	        reg_data |= FASTPATH_IN_ADDR_MUTEX_MODE;
	    }
	    reg_data |= FASTPATH_IN_ADDR_MESSAGE_LATBITS(0);
	    reg_data |= FASTPATH_IN_ADDR_ENABLE;
	    hcd_writel(reg_data, phc, MISC_EXT_FASTPATH_IN_ADDR_LO_REG);

	    reg_data = (addr >> 32);
	    hcd_writel(reg_data, phc, MISC_EXT_FASTPATH_IN_ADDR_HI_REG);

	    /*
	     * -----------------------
	     * RC/WiFi <- UBUS -> RC
	     * -----------------------
	     */
	    /* Configure Incoming address from Other RC's */
	    /* Nothing to do as it is fixed for now */
	    /* Configure Incoming address from WiFi */
#if defined(UBUS_PORT_ID_MLO)

	    req_id = FASTPATH_ID_WIFI0;
	    reg_data = hcd_readl(phc, MISC_EXT_UBUS_FASTPATH_PORT_CONFIG_REG);
	    reg_data &= (~(FASTPATH_PORT_CONFIG_ID_MASK << (8*req_id)));
	    reg_data |= FASTPATH_PORT_MUTEX_VISIBILITY(req_id);
	    if (exclusive == true) {
	        reg_data |= FASTPATH_PORT_MUTEX_MODE(req_id);
	    }
	    reg_data |= FASTPATH_PORT_MESSAGE_LATBITS(req_id, 0);
	    reg_data |= FASTPATH_PORT_ENABLE(req_id);

	    hcd_writel(reg_data, phc, MISC_EXT_UBUS_FASTPATH_PORT_CONFIG_REG);

	    reg_data = hcd_readl(phc, MISC_EXT_UBUS_FASTPATH_PORT_MAPPING_REG);
	    reg_data &= (~(FASTPATH_PORT_MAPPING_ID_MASK << (8*req_id)));
	    reg_data |= UBUS_FASTPATH_PORT_MAPPING(req_id, UBUS_PORT_ID_MLO);
	    hcd_writel(reg_data, phc, MISC_EXT_UBUS_FASTPATH_PORT_MAPPING_REG);
#endif /* UBUS_PORT_ID_MLO */

#if defined(UBUS_PORT_ID_MLO1)

	    req_id = FASTPATH_ID_WIFI1;
	    reg_data = hcd_readl(phc, MISC_EXT_UBUS_FASTPATH_PORT_CONFIG_REG);
	    reg_data &= (~(FASTPATH_PORT_CONFIG_ID_MASK << (8*req_id)));
	    reg_data |= FASTPATH_PORT_MUTEX_VISIBILITY(req_id);
	    if (exclusive == true) {
	        reg_data |= FASTPATH_PORT_MUTEX_MODE(req_id);
	    }
	    reg_data |= FASTPATH_PORT_MESSAGE_LATBITS(req_id, 0);
	    reg_data |= FASTPATH_PORT_ENABLE(req_id);
	    hcd_writel(reg_data, phc, MISC_EXT_UBUS_FASTPATH_PORT_CONFIG_REG);

	    reg_data = hcd_readl(phc, MISC_EXT_UBUS_FASTPATH_PORT_MAPPING_REG);
	    reg_data &= (~(FASTPATH_PORT_MAPPING_ID_MASK << (8*req_id)));
	    reg_data |= UBUS_FASTPATH_PORT_MAPPING(req_id, UBUS_PORT_ID_MLO1);
	    hcd_writel(reg_data, phc, MISC_EXT_UBUS_FASTPATH_PORT_MAPPING_REG);
#endif /* UBUS_PORT_ID_MLO1 */

	    /* Update the fastpath address information in HCD */
	    pcie_hcd_update_fastpath_in_addr(phc->pdrv, addr,
	        MISC_EXT_BROADCAST_OUT_MESSAGE_REG);
	}

	if (en & FASTPATH_EN_OUT) {

	    HCD_LOG("Core [%d] Configuring Outgoing Fastpath\n", phc->info.id);

	    /*
	     * -----------------------
	     * EP <- PCIe Bus -> RC
	     * -----------------------
	     */
	    /* Configure outgoing broadcast address */
	    addr = pcie_hcd_update_fastpath_out_addr(phc->pdrv, NULL);

	    reg_data = (addr & FASTPATH_OUT_ADDR_LO_BASE_ADDRESS_MASK);
	    reg_data |= FASTPATH_OUT_ADDR_ENABLE;
	    hcd_writel(reg_data, phc, MISC_EXT_FASTPATH_OUT_ADDR_LO_REG);

	    reg_data = (addr >> 32);
	    hcd_writel(reg_data, phc, MISC_EXT_FASTPATH_OUT_ADDR_HI_REG);

	    /* Fastpath out attributes for PCIe */
	    reg_data = PCIE_TC3;
	    hcd_writel(reg_data, phc, MISC_EXT_FASTPATH_OUT_ATTR_REG);

	    /*
	     * -----------------------
	     * RC/WiFi <- UBUS -> RC
	     * -----------------------
	     */
	    /* Configure Outgoing address to Other RC's */
	    /* Nothing to do as it is fixed for now */
	    /* Configure Outgoing address to WiFi */
#if defined(UBUS_PORT_ID_MLO)
	    req_id = FASTPATH_ID_WIFI0;

	    addr = pcie_hcd_get_fastpath_out_addr(NULL, req_id);
	    reg_data = (addr & UBUS_FASTPATH_OUT_ADDR_LO_BASE_ADDRESS_MASK);
	    reg_data |= UBUS_FASTPATH_OUT_ADDR_ENABLE;
	    hcd_writel(reg_data, phc, UBUS_FASTPATH_OUT_ADDR_LO_REG(req_id));

	    reg_data = (addr >> 32);
	    hcd_writel(reg_data, phc, UBUS_FASTPATH_OUT_ADDR_HI_REG(req_id));
#endif /* UBUS_PORT_ID_MLO */

#if defined(UBUS_PORT_ID_MLO1)
	    req_id = FASTPATH_ID_WIFI1;

	    addr = pcie_hcd_get_fastpath_out_addr(NULL, req_id);
	    reg_data = (addr & UBUS_FASTPATH_OUT_ADDR_LO_BASE_ADDRESS_MASK);
	    reg_data |= UBUS_FASTPATH_OUT_ADDR_ENABLE;
	    hcd_writel(reg_data, phc, UBUS_FASTPATH_OUT_ADDR_LO_REG(req_id));

	    reg_data = (addr >> 32);
	    hcd_writel(reg_data, phc, UBUS_FASTPATH_OUT_ADDR_HI_REG(req_id));
#endif /* UBUS_PORT_ID_MLO1 */

	    /* Fastpath out attributes for UBUS */
	    reg_data = UBUS_FASTPATH_OUT_ATTR_QUEUE(PCIE_TC2Q_HIGH);
	    reg_data |= UBUS_FASTPATH_OUT_ATTR_FLOW(PCIE_TC2F_HIGHEST);
	    reg_data |= UBUS_FASTPATH_OUT_ATTR_PRIORITY(PCIE_TC2P_HIGH);
	    hcd_writel(reg_data, phc, MISC_EXT_UBUS_FASTPATH_OUT_ATTR_REG);

	    /* Outgoing queue filter#3 used for Broadcast messages */
	    reg_data = MISC_EXT_PCIE_OUTGOING_Q_FILTER_ENABLE |
	        MISC_EXT_PCIE_OUTGOING_Q_FILTER_FLOW_ENABLE |
	        MISC_EXT_PCIE_OUTGOING_Q_FILTER_FLOW(PCIE_TC2F_HIGHEST) |
	        MISC_EXT_PCIE_OUTGOING_Q_FILTER_QUEUE(PCIE_TC2Q_HIGH) |
	        MISC_EXT_PCIE_OUTGOING_Q_FILTER_TC(PCIE_TC3);
	    hcd_writel(reg_data, phc, MISC_EXT_PCIE_OUTGOING_Q_FILTER_REG(3));
	}

	HCD_FN_RET_VAL(0);
}

/*
 *
 * Function bcm963xx_hc_core_config_prioqueue (phc)
 *
 *
 * Parameters:
 *  phc ... pointer to PCIe core HC data structure
 *
 * Description:
 *   Configure Priority Queues for the incoming and outgoing traffic
 *
 *  Return: 0 on success, -ve on failure
 */
static int bcm963xx_hc_core_config_prioqueue(struct pcie_hc_core *phc)
{
	u32 reg_data;

	HCD_FN_ENT();

	/*
	 * -----------------------
	 * Priority Queues Mapping
	 * -----------------------
	 */

	HCD_LOG("Core [%d] Configuring Priority Queue Mapping\n", phc->info.id);

	/* Ingress (PCIe to UBUS) Remapping */

	/*
	 * PCIe Incoming Traffic Class to Queue Mapping
	 * TC#0 => Low, TC#1 => Mid, TC#2 => High
	 */
	reg_data = MISC_EXT_PCIE_TC_QUEUE_TC2Q(PCIE_TC0, PCIE_TC2Q_LOW) |
	    MISC_EXT_PCIE_TC_QUEUE_TC2Q(PCIE_TC1, PCIE_TC2Q_MID) |
	    MISC_EXT_PCIE_TC_QUEUE_TC2Q(PCIE_TC2, PCIE_TC2Q_HIGH);
	hcd_writel(reg_data, phc, MISC_EXT_PCIE_TC_QUEUE_REG);

	/*
	 * PCIe Incoming Traffic Class to UBUS Flow Mapping
	 * TC#0 => Low, TC#1 => Mid, TC#2 => High
	 */
	reg_data = MISC_EXT_PCIE_TC_FLOW_TC2F(PCIE_TC0, PCIE_TC2F_LOW) |
	    MISC_EXT_PCIE_TC_FLOW_TC2F(PCIE_TC1, PCIE_TC2F_MID) |
	    MISC_EXT_PCIE_TC_FLOW_TC2F(PCIE_TC2, PCIE_TC2F_HIGH);
	hcd_writel(reg_data, phc, MISC_EXT_PCIE_TC_FLOW_REG);

	/*
	 * PCIe Incoming Traffic Class to UBUS Priority Mapping
	 * TC#0 => Low, TC#1 => High, TC#2 => High
	 * Only Low/High supported, no Mid
	 */
	reg_data = MISC_EXT_PCIE_TC_UBUS_TC2P(PCIE_TC0, PCIE_TC2P_LOW) |
	    MISC_EXT_PCIE_TC_UBUS_TC2P(PCIE_TC1, PCIE_TC2P_HIGH) |
	    MISC_EXT_PCIE_TC_UBUS_TC2P(PCIE_TC2, PCIE_TC2P_HIGH);
	hcd_writel(reg_data, phc, MISC_EXT_PCIE_TC_PRIORITY_REG);

	/* Egress (UBUS to PCIe ) Remapping */

	/*
	 * PCIe Outgoing Port to Queue Filter 0
	 *
	 * Used for Inter PCIe Core Communication (ICC) using 'flow'
	 * EP always uses TC#2 and UBUS uses corresponding Flow#2
	 */
	reg_data = MISC_EXT_PCIE_OUTGOING_Q_FILTER_ENABLE |
	    MISC_EXT_PCIE_OUTGOING_Q_FILTER_FLOW_ENABLE |
	    MISC_EXT_PCIE_OUTGOING_Q_FILTER_FLOW(PCIE_TC2F_HIGH) |
	    MISC_EXT_PCIE_OUTGOING_Q_FILTER_QUEUE(PCIE_TC2Q_MID) |
	    MISC_EXT_PCIE_OUTGOING_Q_FILTER_TC(PCIE_TC2);
	hcd_writel(reg_data, phc, MISC_EXT_PCIE_OUTGOING_Q_FILTER_REG(0));

#if defined(UBUS_PORT_ID_WIFI)
	/*
	 * PCIe Outgoing Port to Queue Filter 1
	 *
	 * Used for Inter PCIe Core Communication (ICC) using 'port_id' for
	 * Internal WiFi core#A
	 */
	reg_data = MISC_EXT_PCIE_OUTGOING_Q_FILTER_ENABLE |
	    MISC_EXT_PCIE_OUTGOING_Q_FILTER_PORT_ENABLE |
	    MISC_EXT_PCIE_OUTGOING_Q_FILTER_PORT(UBUS_PORT_ID_WIFI) |
	    MISC_EXT_PCIE_OUTGOING_Q_FILTER_QUEUE(PCIE_TC2Q_MID) |
	    MISC_EXT_PCIE_OUTGOING_Q_FILTER_TC(PCIE_TC2);
	hcd_writel(reg_data, phc, MISC_EXT_PCIE_OUTGOING_Q_FILTER_REG(1));
#endif /* UBUS_PORT_ID_WIFI */

#if defined(UBUS_PORT_ID_WIFI1)
	/*
	 * PCIe Outgoing Port to Queue Filter 2
	 *
	 * Used for Inter PCIe Core Communication (ICC) using 'port_id' for
	 * Internal WiFi core#B
	 */
	reg_data = MISC_EXT_PCIE_OUTGOING_Q_FILTER_ENABLE |
	    MISC_EXT_PCIE_OUTGOING_Q_FILTER_PORT_ENABLE |
	    MISC_EXT_PCIE_OUTGOING_Q_FILTER_PORT(UBUS_PORT_ID_WIFI1) |
	    MISC_EXT_PCIE_OUTGOING_Q_FILTER_QUEUE(PCIE_TC2Q_MID) |
	    MISC_EXT_PCIE_OUTGOING_Q_FILTER_TC(PCIE_TC2);
	hcd_writel(reg_data, phc, MISC_EXT_PCIE_OUTGOING_Q_FILTER_REG(2));
#endif /* UBUS_PORT_ID_WIFI1 */

	/*
	 * ---------------------------
	 * Priority Queues Arbitration
	 * ---------------------------
	 */

	HCD_LOG("Core [%d] Configuring Priority Queue Arbitration\n",
	    phc->info.id);

	/* PCIe to UBUS Arbitration */

	/*
	 * PCIe to UBUS Read Request Arbiter
	 *
	 * Arbiter#0: IWRR lowest weight
	 * Arbiter#1: IWRR medium weight
	 * Arbiter#2: SP lowest weight
	 */
	reg_data = ARB_MODE_WEIGHT(2, ARB_MODE_SP, ARB_WEIGHT_SP0) |
	    ARB_MODE_WEIGHT(1, ARB_MODE_IWRR, ARB_WEIGHT_XWRR8) |
	    ARB_MODE_WEIGHT(0, ARB_MODE_IWRR, ARB_WEIGHT_XWRR1);
	hcd_writel(reg_data, phc, MISC_EXT_PCIE_UBUS_READ_REQUEST_ARB_REG);

	/*
	 * PCIe to UBUS Write Request Arbiter
	 *
	 * Arbiter#0: IWRR lowest weight
	 * Arbiter#1: IWRR medium weight
	 * Arbiter#2: SP lowest weight
	 */
	reg_data = ARB_MODE_WEIGHT(2, ARB_MODE_SP, ARB_WEIGHT_SP0) |
	    ARB_MODE_WEIGHT(1, ARB_MODE_IWRR, ARB_WEIGHT_XWRR8) |
	    ARB_MODE_WEIGHT(0, ARB_MODE_IWRR, ARB_WEIGHT_XWRR1);
	hcd_writel(reg_data, phc, MISC_EXT_PCIE_UBUS_WRITE_REQUEST_ARB_REG);

	/*
	 * PCIe to UBUS Write Request Qx Arbiter (arbitrate per priority)
	 *
	 * Q2=DMA, Q1=WRITE, Q0=READ
	 *
	 * All arbiters with same mode and weight (IWRR + lowest weight 1/32)
	 */
	reg_data = ARB_MODE_WEIGHT(2, ARB_MODE_IWRR, ARB_WEIGHT_XWRR1) |
	    ARB_MODE_WEIGHT(1, ARB_MODE_IWRR, ARB_WEIGHT_XWRR1) |
	    ARB_MODE_WEIGHT(0, ARB_MODE_IWRR, ARB_WEIGHT_XWRR1);
	hcd_writel(reg_data, phc, MISC_EXT_UBUS_REQUEST_Q2_ARB_REG);
	hcd_writel(reg_data, phc, MISC_EXT_UBUS_REQUEST_Q1_ARB_REG);
	hcd_writel(reg_data, phc, MISC_EXT_UBUS_REQUEST_Q0_ARB_REG);

	/*
	 * PCIe to UBUS Final Request Arbiter
	 *
	 * final arbiter Q3=write broadcast, Q2:Q0=priorities 2:0
	 *
	 * Arbiter#0: IWRR lowest weight
	 * Arbiter#1: IWRR medium weight
	 * Arbiter#2: SP lowest weight
	 * Arbiter#3: SP medium weight
	 */
	reg_data = ARB_MODE_WEIGHT(3, ARB_MODE_SP, ARB_WEIGHT_SP1) |
	    ARB_MODE_WEIGHT(2, ARB_MODE_SP, ARB_WEIGHT_SP0) |
	    ARB_MODE_WEIGHT(1, ARB_MODE_IWRR, ARB_WEIGHT_XWRR8) |
	    ARB_MODE_WEIGHT(0, ARB_MODE_IWRR, ARB_WEIGHT_XWRR1);
	hcd_writel(reg_data, phc, MISC_EXT_UBUS_REQUEST_ARB_REG);

	/*
	 * PCIe to UBUS Reply Arbiter
	 *
	 * Arbiter#0: IWRR lowest weight
	 * Arbiter#1: IWRR medium weight
	 * Arbiter#2: SP lowest weight
	 */
	reg_data = ARB_MODE_WEIGHT(2, ARB_MODE_SP, ARB_WEIGHT_SP0) |
	    ARB_MODE_WEIGHT(1, ARB_MODE_IWRR, ARB_WEIGHT_XWRR8) |
	    ARB_MODE_WEIGHT(0, ARB_MODE_IWRR, ARB_WEIGHT_XWRR1);
	hcd_writel(reg_data, phc, MISC_EXT_UBUS_PCIE_REPLY_ARB_REG);

	/* UBUS to PCIe Arbitration */

	/*
	 * UBUS to PCIe Read/Write Arbiter (arbitrate read/write)
	 *
	 * Q2=mutex/broadcast, Q1=inter-chip, Q0=default
	 *
	 * Arbiter#0: SP lowest weight
	 * Arbiter#1: SP medium weight
	 * Arbiter#2: SP highest weight
	 */
	reg_data = ARB_MODE_WEIGHT(2, ARB_MODE_SP, ARB_WEIGHT_SP2) |
	    ARB_MODE_WEIGHT(1, ARB_MODE_SP, ARB_WEIGHT_SP1) |
	    ARB_MODE_WEIGHT(0, ARB_MODE_SP, ARB_WEIGHT_SP0);
	hcd_writel(reg_data, phc, MISC_EXT_UBUS_PCIE_READ_WRITE_ARB_REG);


	/*
	 * UBUS to PCIe Qx Arbiter (arbitrate per-priority)
	 *
	 * Q3=DMA write, Q2=DMA read, Q1=completion, Q0=UBUS read/write
	 *
	 * All arbiters with same mode and weight (IWRR + lowest weight 1/32)
	 */
	reg_data = ARB_MODE_WEIGHT(3, ARB_MODE_IWRR, ARB_WEIGHT_XWRR1) |
	    ARB_MODE_WEIGHT(2, ARB_MODE_IWRR, ARB_WEIGHT_XWRR1) |
	    ARB_MODE_WEIGHT(1, ARB_MODE_IWRR, ARB_WEIGHT_XWRR1) |
	    ARB_MODE_WEIGHT(0, ARB_MODE_IWRR, ARB_WEIGHT_XWRR1);
	hcd_writel(reg_data, phc, MISC_EXT_PCIE_REQUEST_Q2_ARB_REG);
	hcd_writel(reg_data, phc, MISC_EXT_PCIE_REQUEST_Q1_ARB_REG);
	hcd_writel(reg_data, phc, MISC_EXT_PCIE_REQUEST_Q0_ARB_REG);

	/*
	 * UBUS to PCIe Tx Arbiter
	 *
	 * PCIe Tx final arbitration among priorities
	 *
	 * Arbiter#0: IWRR lowest weight
	 * Arbiter#1: IWRR medium weight
	 * Arbiter#2: SP lowest weight
	 */
	reg_data = ARB_MODE_WEIGHT(2, ARB_MODE_SP, ARB_WEIGHT_SP0) |
	    ARB_MODE_WEIGHT(1, ARB_MODE_IWRR, ARB_WEIGHT_XWRR8) |
	    ARB_MODE_WEIGHT(0, ARB_MODE_IWRR, ARB_WEIGHT_XWRR1);
	hcd_writel(reg_data, phc, MISC_EXT_PCIE_TX_ARB_REG);

	/*
	 * UBUS to PCIe Reply Arbiter
	 *
	 * Arbiter#0: IWRR lowest weight
	 * Arbiter#1: IWRR medium weight
	 * Arbiter#2: SP lowest weight
	 */
	reg_data = ARB_MODE_WEIGHT(2, ARB_MODE_SP, ARB_WEIGHT_SP0) |
	    ARB_MODE_WEIGHT(1, ARB_MODE_IWRR, ARB_WEIGHT_XWRR8) |
	    ARB_MODE_WEIGHT(0, ARB_MODE_IWRR, ARB_WEIGHT_XWRR1);
	hcd_writel(reg_data, phc, MISC_EXT_PCIE_UBUS_REPLY_ARB_REG);

	HCD_FN_RET_VAL(0);
}

/*
 *
 * Function bcm963xx_hc_core_set_reset (phc, enable)
 *
 *
 *   Parameters:
 *    phc    ... pointer to PCIe core HC data structure
 *    enable ... falg to enable reset
 *
 *   Description:
 *    Set the PCIe core reset state (enable/disable)
 *
 *   Return: None
 */
static void bcm963xx_hc_core_set_reset(
	struct pcie_hc_core *phc, bool enable)
{
	uint32 reg_data;

	if (phc->info.rev >= 0x320) {
	    /* Use PCIe internal MISC Block to do reset */

	    reg_data = hcd_readl(phc, MISC_PCIE_CTRL_REG);

	    if (enable == TRUE)
	        /* Assert fundamental Soft Reset the core */
	        reg_data &= ~MISC_PCIE_CTRL_PCIE_PERSTB;
	    else
	        /* De-assert reset and bring the core out of Soft Reset */
	        reg_data |= MISC_PCIE_CTRL_PCIE_PERSTB;

	    hcd_writel(reg_data, phc, MISC_PCIE_CTRL_REG);
	} else if (phc->info.misc != NULL) {
	    /* Use external MISC Block to do reset */
	    reg_data = readl(phc->info.misc);
	    if (enable == TRUE)
	        /* Soft Reset the core */
	        reg_data &= ~(1 << phc->info.id);
	    else
	        /* Bring the core out of Soft Reset */
	        reg_data |= (1 << phc->info.id);

	    writel(reg_data, phc->info.misc);
	} else {
	    HCD_ERROR("Core [%d] reset method not defined\n", phc->info.id);
	}

	/* Allow reset change to settle down */
	mdelay(5);

	return;
}

/*
 *
 * Function bcm963xx_hc_core_set_speed (phc, speed)
 *
 *
 * Parameters:
 *  phc ... pointer to PCIe core HC data structure
 *  speed ... Speed value (0: default, 1: 2.5Gbps, 2 - 5Gbps, 3 - 8 Gbps
 *
 * Description:
 *  Setup PCIe core speed according to configuration
 *
 * Return: 0 on success, -ve on failure
 */
static int bcm963xx_hc_core_set_speed(struct pcie_hc_core *phc, u8 speed)
{
	u32 data;

	HCD_FN_ENT();

	/* Nothing to do, if configured to use default speed */
	if (speed == 0) {
	    HCD_FN_EXT();
	    return 0;
	}

	data = hcd_readl(phc, RC_CFG_PRIV1_LINK_CAPABILITY_REG);
	data &= ~RC_CFG_PRIV1_LINK_CAP_LINK_SPEED_MASK;
	data |= (speed & RC_CFG_PRIV1_LINK_CAP_LINK_SPEED_MASK);

	hcd_writel(data, phc, RC_CFG_PRIV1_LINK_CAPABILITY_REG);

	data = hcd_readl(phc, RC_CFG_PCIE_LINK_STATUS_CONTROL_2_REG);
	data &= ~RC_CFG_PCIE_LINK_CTRL_TGT_LINK_SPEED_MASK;
	data |= (speed & RC_CFG_PCIE_LINK_CTRL_TGT_LINK_SPEED_MASK);
	hcd_writel(data, phc, RC_CFG_PCIE_LINK_STATUS_CONTROL_2_REG);

	HCD_LOG("Core [%d] Link Speed set to GEN%d\n", phc->info.id, speed);

	/* Required to settle the speed setting */
	mdelay(10);

	HCD_FN_EXT();

	return 0;
}

/*
 *
 * Function bcm963xx_hc_core_get_speed (phc)
 *
 *
 * Parameters:
 *  phc ... pointer to PCIe core HC data structure
 *
 * Description:
 *  Get PCIe core speed according to configuration
 *
 * Return: link speed capability
 */
static int bcm963xx_hc_core_get_speed(struct pcie_hc_core *phc)
{
	int speed;
	u32 data;

	HCD_FN_ENT();

	data = hcd_readl(phc, RC_CFG_PRIV1_LINK_CAPABILITY_REG);
	speed = data & RC_CFG_PRIV1_LINK_CAP_LINK_SPEED_MASK;

	HCD_FN_EXT();

	return speed;
}

/*
 *
 * Function bcm963xx_hc_core_set_link_width (phc)
 *
 *
 * Parameters:
 *  phc ... pointer to PCIe core HC data structure
 *
 * Description:
 *   Setup PCIe core lanes according to configuration
 *
 *  Return: 0 on success, -ve on failure
 */
static int bcm963xx_hc_core_set_link_width(struct pcie_hc_core *phc)
{
	u32 reg_data;
	u32 max_width;

	HCD_FN_ENT();

	/* Nothing to do, if configured to use default link width */
	if (phc->res.link_width == 0)
	    return 0;

	reg_data = hcd_readl(phc, RC_CFG_PRIV1_LINK_CAPABILITY_REG);
	max_width = reg_data & RC_CFG_PRIV1_LINK_CAP_MAX_LINK_WIDTH_MASK;
	max_width >>= RC_LINK_CAP_MAX_LNK_WIDTH_SHIFT;

	if (max_width > phc->res.link_width) {
	    /* Downgrade max link width */
	    reg_data &= ~RC_CFG_PRIV1_LINK_CAP_MAX_LINK_WIDTH_MASK;
	    reg_data |= (phc->res.link_width << RC_LINK_CAP_MAX_LNK_WIDTH_SHIFT);

	    hcd_writel(reg_data, phc, RC_CFG_PRIV1_LINK_CAPABILITY_REG);

	    reg_data = hcd_readl(phc, RC_CFG_PRIV1_LINK_CAPABILITY_REG);

	    HCD_LOG("Core [%d] Link width changed from %d to %d\n", phc->info.id,
	        max_width, phc->res.link_width);

	    /*
	     * Enable the auxilliary powerdown of unused lanes in a multi-lane link
	     * RC_PL_REG_PHY_CTL_1.REG_P2_POWERDOWN_ENA_NOSYNC = 1'b
	     */
	    reg_data = hcd_readl(phc, RC_PL_REG_PHY_CTL_1_REG);
	    reg_data |= RC_PL_REG_PHY_CTL_1_PWRDN_ENA_NOSYNC;
	    hcd_writel(reg_data, phc, RC_PL_REG_PHY_CTL_1_REG);

	    HCD_INFO("Core [%d] aux pwrdn unused lanes, "
	        "RC_PL_REG_PHY_CTL_1 = 0x%x\n", phc->info.id,
	        hcd_readl(phc, RC_PL_REG_PHY_CTL_1_REG));

	    /* Required to settle the speed setting */
	    mdelay(10);
	}

	HCD_FN_EXT();

	return 0;
}

/*
 *
 * Function bcm963xx_hc_core_set_coalescing (phc)
 *
 *
 * Parameters:
 *  phc ... pointer to PCIe core HC data structure
 *
 * Description:
 *   Setup PCIe core ack and credit coalescing configuration
 *
 *  Return: 0 on success, -ve on failure
 */
static int bcm963xx_hc_core_set_coalescing(struct pcie_hc_core *phc)
{
	u32 reg_data;
	u32 coalesc;

	HCD_FN_ENT();

	/* Nothing to do, if configured to use disable acc */
	if (phc->cfg.acc == FALSE) {
	    HCD_FN_EXT();
	    return 0;
	}

	/*
	 * From Design team:
	 *
	 * 1) Turn on PCIe ACK coalescing (applies to reads and writes)
	 * PCIE_<port>_RC_DL_PDL_CONTROL_2.ENABLE_ACK_LAT_TIMER=1
	 *
	 * 2) Turn on PCIe Credit coalescing to 1/4 of the advertised credits
	 *    (separate settings for reads and writes)
	 *
	 * a) Query non-posted (read) advertised credits and set coalescing to
	 * NPH_INIT>>2 (1/4th)
	 * nph_init =  PCIE_<port>_RC_DL_PDL_CONTROL_4.NPH_FC_INIT
	 * nph_coal = nph_init>>2
	 * PCIE_<port>_RC_TL_TL_FCIMM_NP_LIMIT.REG_FC_NPH_IMM_LIMIT = nph_coal-1
	 * PCIE_<port>_RC_TL_TL_FCIMM_NP_LIMIT REG_ENA_FC_NP_IMMEDIATE = 0
	 *
	 * Note that nph_init ranges from 8 (e.g. 4908) to 32 (e.g. 4912 dual-lane)
	 *  and is unique per port.
	 *
	 * b) Query posted (write) advertised credits and coalescing to PH_INIT>>2
	 * (1/4th)
	 * ph_init = PCIE_<port>_RC_DL_PDL_CONTROL_5.PH_INIT
	 * ph_coal = ph_init>>2
	 * PCIE_<port>_RC_TL_TL_FCIMM_P_LIMIT.REG_FC_PH_IMM_LIMIT = ph_coal-1
	 * PCIE_<port>_RC_TL_TL_FCIMM_P_LIMIT. REG_ENA_FC_P_IMMEDIATE = 0
	 *
	 * Note that ph_init ranges from 8 (e.g. 4908) to 16 (e.g. 4912 dual-lane)
	 *  and is unique per port.
	 *
	 */

	/* Turn on PCIe ACK coalescing (applies to reads and writes) */
	reg_data = hcd_readl(phc, RC_DL_PDL_CONTROL2_REG);
	reg_data |= RC_DL_PDL_CONTROL2_EN_ACK_LAT_TIMER;
	hcd_writel(reg_data, phc, RC_DL_PDL_CONTROL2_REG);

	/* Query non-posted (read) advertised credits */
	reg_data = hcd_readl(phc, RC_DL_PDL_CONTROL4_REG);
	reg_data &= RC_DL_PDL_CONTROL4_NPH_FC_INIT_MASK;
	reg_data >>= RC_DL_PDL_CONTROL4_NPH_FC_INIT_SHIFT;

	/*  set coalescing to NPH_INIT>>2 (1/4th) */
	coalesc = (reg_data >> 2);
	reg_data = hcd_readl(phc, RC_TL_FCIMM_NP_LIMIT_REG);
	reg_data &= ~RC_TL_FCIMM_NP_LIMIT_FC_NPH_IMM_LIMIT_MASK;
	reg_data |= ((coalesc-1) << RC_TL_FCIMM_NP_LIMIT_FC_NPH_IMM_LIMIT_SHIFT);
	reg_data &= ~RC_TL_FCIMM_NP_LIMIT_ENA_FC_NP_IMMEDIATE;
	hcd_writel(reg_data, phc, RC_TL_FCIMM_NP_LIMIT_REG);

	/* Query posted (write) advertised credits */
	reg_data = hcd_readl(phc, RC_DL_PDL_CONTROL5_REG);
	reg_data &= RC_DL_PDL_CONTROL5_PH_INIT_MASK;
	reg_data >>= RC_DL_PDL_CONTROL5_PH_INIT_SHIFT;

	/* set coalescing to PH_INIT>>2 (1/4th) */
	coalesc = (reg_data >> 2);
	reg_data = hcd_readl(phc, RC_TL_FCIMM_P_LIMIT_REG);
	reg_data &= ~RC_TL_FCIMM_P_LIMIT_FC_PH_IMM_LIMIT_MASK;
	reg_data |= ((coalesc-1) << RC_TL_FCIMM_P_LIMIT_FC_PH_IMM_LIMIT_SHIFT);
	reg_data &= ~RC_TL_FCIMM_P_LIMIT_ENA_FC_P_IMMEDIATE;
	hcd_writel(reg_data, phc, RC_TL_FCIMM_P_LIMIT_REG);

	HCD_FN_EXT();

	return 0;
}

/*
 * Function bcm963xx_hc_core_reset_config (phc)
 *
 *   Parameters:
 *    phc ... pointer to PCIe core HC data structure
 *
 *   Description:
 *    Setup PCIe core parameters that needs to be done when reset is asserted
 *
 *   Return: 0 on success, -ve on failure
 */
static int bcm963xx_hc_core_reset_config(struct pcie_hc_core *phc)
{
	struct bcm963xx_hc_cb *phc_cb = (struct bcm963xx_hc_cb *)(phc->phc_cb);

	HCD_FN_ENT();

	/* Configure the link speed, if need */
	bcm963xx_hc_core_set_speed(phc, phc->cfg.speed);

	/* Configure the link width, if needed */
	bcm963xx_hc_core_set_link_width(phc);

	/* Configure PCIe ack and credit coalescing for Root Complex */
	bcm963xx_hc_core_set_coalescing(phc);

	/* sysclk war for bifurcated port (applicable only for some versions) */
	if (phc_cb->wars.bifp1sysclk == 1) {
	    uint32 hard_dbg;
	    /*
	     * Workaround PART-1
	     * In PCIE_1_MISC_HARD_PCIE_HARD_DEBUG register, set
	     * REFCLK_OVERRIDE = 1
	     * REFCLK_OVERRIDE_IN = 0 (Internal CML reference clock for the
	     * SerDes PLL)
	     * REFCLK_OVERRIDE_OUT = 0 (Disabled output clock)
	     */
	    hard_dbg = hcd_readl(phc, MISC_HARD_DEBUG_REG);
	    hard_dbg |= MISC_HARD_DEBUG_REFCLK_OVERRIDE;
	    hard_dbg &= ~MISC_HARD_DEBUG_REFCLK_OVERRIDE_OUT;
	    hard_dbg &= ~MISC_HARD_DEBUG_REFCLK_OVERRIDE_IN_MASK;

	    hcd_writel(hard_dbg, phc, MISC_HARD_DEBUG_REG);
	}

	if (phc_cb->wars.g3cmosdrvopclk == 1) {
	    HCD_LOG("Port [%d] Applying g3cmosdrvopclk WAR\r\n", phc->info.id);
	    /*
	     * CMOS driver for output clock - Jira CRRBBPCIE-177
	     * Applied while PCIe reset is asserted (prior to link training).
	     *
	     *  mdio write 0x1f, 0xffd0
	     *  mdio write 0x1e, 0x0800
	     *
	     *  mdio write 0x1f, 0xd3d0
	     *  mdio write 0x04, 0x0810
	     */
	    bcm963xx_hc_mdio_write(phc, 0, 0x1f, 0xffd0);
	    bcm963xx_hc_mdio_write(phc, 0, 0x1e, 0x0800);

	    bcm963xx_hc_mdio_write(phc, 0, 0x1f, 0xd3d0); /* AMS_PLL */
	    /* CTRL_4.CMOS_TPort_en=1, test_amp=01 (112.5%) */
	    bcm963xx_hc_mdio_write(phc, 0, 0x04, 0x0810);
	}

	if (phc_cb->wars.g3refclkdetmode == 1) {
	    HCD_LOG("Port [%d] Applying g3refclkdetmode WAR\r\n", phc->info.id);
	    /*
	     * Correct settings for reference clock detect mode - Jira CRRBBPCIE-175
	     * Applied while PCIe reset is asserted (prior to link training)
	     *
	     *  mdio write 0x1f, 0xffd0
	     *  mdio write 0x1e, 0x0400
	     *
	     *  mdio write 0x1f, 0x1200
	     *  mdio write 0x08, 0x0064
	     *  mdio write 0x09, 0x00c4
	     */
	    bcm963xx_hc_mdio_write(phc, 0, 0x1f, 0xffd0);
	    bcm963xx_hc_mdio_write(phc, 0, 0x1e, 0x0400);

	    bcm963xx_hc_mdio_write(phc, 0, 0x1f, 0x1200); /* PCIE_BLK2 */
	    /* RefclkSenseM.mdio_RefclkSense_M */
	    bcm963xx_hc_mdio_write(phc, 0, 0x08, 0x0064);
	    /* RefclkSenseN.mdio_RefclkSense_N */
	    bcm963xx_hc_mdio_write(phc, 0, 0x09, 0x00c4);
	}

	if (phc_cb->wars.rxsigdetflt == 1) {
	    /* Reset state, configure (inreset = TRUE) */
	    bcm963xx_hc_phy_config_phyrxfilter(phc, TRUE);
	}

	HCD_FN_EXT();

	return 0;
}

/*
 * Function bcm963xx_hc_core_unconfig (phc)
 *
 *   Parameters:
 *    phc ... pointer to PCIe core HC data structure
 *
 *   Description:
 *    de-configure PCIe core incoming bar's
 *
 *   Return: 0 on success, -ve on failure
 */
static int bcm963xx_hc_core_unconfig(struct pcie_hc_core *phc)
{
	int barnum;
	uint32 base_hi, base_lo, sizekb;

	HCD_FN_ENT();

	for (barnum = BAR_FIRST; barnum <= BAR_LAST; barnum++) {
	    if ((phc->cfg.barmask & (1 << barnum))) {
	        bcm963xx_hc_get_bar(phc, barnum, &base_hi, &base_lo, &sizekb);
	        if (sizekb) {
	            /* Un configure the BAR register */
	            bcm963xx_hc_config_bar(phc, barnum, 0x0, 0x0, 0x0, 0x0);
	        }
	    }
	}

	/* Assert Core Reset */
	bcm963xx_hc_core_set_reset(phc, TRUE);

	HCD_FN_EXT();

	return 0;
}

/*
 * Function bcm963xx_hc_core_post_config (phc)
 *
 *   Parameters:
 *    phc ... pointer to PCIe core HC data structure
 *
 *   Description:
 *    Setup PCIe core fastpath out message
 *    called as part of post enumberation.
 *
 *   Return: 0 on success, -ve on failure
 */
static int bcm963xx_hc_core_post_config(struct pcie_hc_core *phc)
{
	HCD_FN_ENT();

	/* Configure the Broadcast out messaging if supported and enabled */
	bcm963xx_hc_core_config_fastpath(phc, FASTPATH_EN_OUT);

	HCD_FN_EXT();

	return 0;
}

/*
 * Function bcm963xx_hc_core_pre_config (phc)
 *
 *   Parameters:
 *    phc ... pointer to PCIe core HC data structure
 *
 *   Description:
 *    Setup PCIe core legacy interrupts, outgoing memory window, bars
 *    PCI class, UBUS
 *
 *   Return: 0 on success, -ve on failure
 */
static int bcm963xx_hc_core_pre_config(struct pcie_hc_core *phc)
{
	struct bcm963xx_hc_cb *phc_cb = (struct bcm963xx_hc_cb *)(phc->phc_cb);
	uint32 reg_data;
	struct resource *owin;
	int win;

	HCD_FN_ENT();

	/* setup legacy out-of-band interrupts */
	reg_data = (CPU_INTR1_PCIE_INTD_CPU_INTR
	            | CPU_INTR1_PCIE_INTC_CPU_INTR
	            | CPU_INTR1_PCIE_INTB_CPU_INTR
	            | CPU_INTR1_PCIE_INTA_CPU_INTR);
	hcd_writel(reg_data, phc, CPU_INTR1_INTR_MASK_CLEAR_REG);


	for (win = OWIN0; win < MAX_NUM_OUTGOING_WINDOWS; win++) {
	    uint32 reg_offset;

	    if (!OWIN_RES_CONFIGURED(phc, win)) continue;

	    owin = &(phc->res.owin[win]);

	    /* setup outgoing memory resource window */
	    reg_data = ((owin->start >> RC_MEM_WIN_LO_BASE_LIMIT_LIMIT_SHIFT) <<
	        MISC_CPU_2_PCI_MEM_WIN_LO_BASE_LIMIT_BASE_SHIFT);
	    reg_data |= (owin->end & RC_MEM_WIN_LO_BASE_LIMIT_LIMIT_MASK);
	    reg_offset = MISC_CPU_2_PCIE_MEM_WIN_BASE_LIMIT_REG(win);
	    hcd_writel(reg_data, phc, reg_offset);

	    reg_data = (owin->start & MISC_CPU_2_PCI_MEM_WIN_LO_BASE_ADDR_MASK);
	    reg_offset = MISC_CPU_2_PCIE_MEM_WIN_LO_REG(win);
	    hcd_writel(reg_data, phc, reg_offset);
	    /* TODO: for 64bit ARM */
	    /* reg_offset = MISC_CPU_2_PCIE_MEM_WIN_HI_REG(win); */
	    /* hcd_writel(0, phc, reg_offset); */

	}

	/* setup incoming DDR memory BAR */
	{
	    int barnum;
	    uint32 ddr_sizekb;
	    uint32 sizekb;
	    uint32 base_lo;
	    uint32 base_hi;

	    /* Go through available BAR's and configure the DDR access */
	    barnum = BAR_FIRST;
	    ddr_sizekb = SZ2KB(kerSysGetMemorySize());
	    sizekb = SZ2KB(SZ_2G);
	    base_lo = 0x00000000;
	    base_hi = 0x00000000;

	    do {
	        if (!(phc->cfg.barmask & (1 << barnum))) {
	            /* This BAR number is not for DDR, skip to next BAR */
	            barnum++;
	            continue;
	        }

	        /* Is this the last memory segment ? */
	        if (ddr_sizekb <= sizekb) {
	            sizekb = ddr_sizekb;
	        }

	        /* Configure the BAR register */
	        bcm963xx_hc_config_bar(phc, barnum, base_hi, base_lo, sizekb, 0x0);

	        /* Prepare for next BAR and DDR segment */
	        ddr_sizekb -= sizekb;
	        sizekb <<= 1;
	        if (base_hi) base_hi <<= 1;
	        else base_hi = 0x00000001;
	        barnum++;
	    } while (barnum <= BAR_LAST && ddr_sizekb && sizekb <= SZKB_32G);

	    if (ddr_sizekb) {
	        HCD_ERROR("Core [%d] Can not use last %d %s of DDR memory\n",
	            phc->info.id,
	            SZ2MB(ddr_sizekb) ? SZ2MB(ddr_sizekb) : SZ2KB(ddr_sizekb),
	            SZ2MB(ddr_sizekb) ? "GB" : "MB");
	    }
	}

	/* set device bus/dev/func -no need */
	/* setup class code, as bridge */
	reg_data = hcd_readl(phc, RC_CFG_PRIV1_ID_VAL3_REG);
	reg_data &= RC_CFG_PRIV1_ID_VAL3_REVISION_ID_MASK;
	reg_data |= (PCI_CLASS_BRIDGE_PCI << 8);
	hcd_writel(reg_data, phc, RC_CFG_PRIV1_ID_VAL3_REG);
	/* disable bar0 size -no need */

	/* disable data bus error for enumeration */
	reg_data = hcd_readl(phc, MISC_CTRL_REG);
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
	reg_data &= ~(MISC_CTRL_BURST_ALIGN_MASK(phc->info.rev));
	if (phc->info.rev >= 0x320) {
	    reg_data |= MISC_CTRL_BURST_ALIGN(phc->info.rev, 4);
	} else if (phc->info.rev == 0x310) {
	    reg_data |= MISC_CTRL_BURST_ALIGN(phc->info.rev, 3);
	} else {
	    reg_data |= MISC_CTRL_BURST_ALIGN(phc->info.rev, 1);
	}

	if (phc->info.rev == 0x310) {
	    /* workaround for UBUS4 Logic Bug in this revision */
	    /* Limit the max burst to 64B */
	    reg_data &= ~MISC_CTRL_MAX_BURST_SIZE_MASK;
	    reg_data |= MISC_CTRL_MAX_BURST_SIZE_64B;
	}

	hcd_writel(reg_data, phc, MISC_CTRL_REG);

	if (phc->info.rev >= 0x320) {
	    /* wait for UBUS replay for burst writes */
	    reg_data = hcd_readl(phc, MISC_UBUS_CTRL_REG);
	    reg_data |= MISC_UBUS_CTRL_UBUS_WR_WITH_REPLY;
	    hcd_writel(reg_data, phc, MISC_UBUS_CTRL_REG);
	}

	/* If configured, enable PCIe SSC (enable = TRUE) */
	bcm963xx_hc_enable_ssc(phc, TRUE);

	/* Process PTM Capability Advertisement */
	if (phc_cb->wars.ptmcapadv == 1) {
	    reg_data = hcd_readl(phc, RC_CFG_PRIV1_ROOT_CAP_REG);
	    reg_data |= (RC_CFG_PRIV1_ROOT_CAP_EXT2_CAP_ENA_PTM_CAP);
	    hcd_writel(reg_data, phc, RC_CFG_PRIV1_ROOT_CAP_REG);
	    HCD_LOG("Core [%d] Enabled PTM Capability Advertisement\n",
	        phc->info.id);
	}

	/* Synchronize the Satellite Timer with the Router CPU Central Timer */
	if ((phc->info.rev >= 0x406) && (phc->info.ctmr != NULL)) {
	    pcie_hcd_sync_stmr(phc->pdrv,
	        (phc->info.base+PCIE_SAT_TIMER_REGS));
	}

	/* Configure the priority queues if supported and enabled */
	if ((phc->info.rev >= 0x406) && (phc->cfg.prioqueue)) {
	    bcm963xx_hc_core_config_prioqueue(phc);
	}

	/* Configure the Mutex Lock (incoming broadcast) if supported and enabled */
	bcm963xx_hc_core_config_fastpath(phc, FASTPATH_EN_IN);

	if ((phc->info.rev >= 0x409) && (hcd_resource_len(&phc->res.ecam))) {
	    bcm963xx_hc_core_config_ecam(phc);
	}
	HCD_FN_EXT();

	return 0;
}

/*
 * Function bcm963xx_hc_core_reset (phc)
 *
 *   Parameters:
 *    phc ... pointer to PCIe core HC data structure
 *
 *   Description:
 *    Reset PCIe core using misc driver API's. Configure the PHY parameters
 *    while the core is in reset.
 *
 *   Return: 0 on success, -ve on failure
 */
static int bcm963xx_hc_core_reset(struct pcie_hc_core *phc)
{
	HCD_FN_ENT();

	/* Soft Reset the core */
	bcm963xx_hc_core_set_reset(phc, TRUE);

	/* Configure the PHY when core is in reset */
	bcm963xx_hc_phy_config(phc);

	/* Configure the core in reset asset state */
	bcm963xx_hc_core_reset_config(phc);

	/* 6716B0 GPIO Regulator WAR */
	if (phc->cfg.epuperstb && bcm963xx_fp_set_power) {
	    uint32 hard_dbg;

	    HCD_LOG("Core [%d] Using epuperstb\n", phc->info.id);
	    /*
	     * 1. PCIE_MISC_HARD_PCIE_HARD_DEBUG.DOWNSTREAM_PERST_ASSERT = 1
	     *    delay 10 usec
	     *
	     * 2. MISC_PCIE_CTRL_PCIE_PERSTB = 1
	     *    delay 1 msec
	     *
	     * 3. Power Up EP device
	     *    delay 125 msec
	     *
	     * 4. PCIE_MISC_HARD_PCIE_HARD_DEBUG.DOWNSTREAM_PERST_ASSERT = 0
	     *    delay 210msec
	     *
	     * Using DS_PERST to gate external PERST# until clock is stable
	     *
	     * DS_PERST__|--------------------------------------------|____________
	     * PERSTB  _____|------------------------------------------------------
	     * EP_GPIO __________|-------------------------------------------------
	     *
	     * PERST# ________________________________________________|------------
	     * REFCLK XXXXX||||_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|
	     */

	    /* Down Stream PERST: DS_PERST_ASSERT=1 */
	    hard_dbg = hcd_readl(phc, MISC_HARD_DEBUG_REG);
	    hard_dbg |= MISC_HARD_DEBUG_DS_PERST_ASSERT;
	    hcd_writel(hard_dbg, phc, MISC_HARD_DEBUG_REG);
	    mdelay(1);

	    /* PCIE_PERSTB=1, also turns on REFCLK on the bus */
	    bcm963xx_hc_core_set_reset(phc, FALSE);

	    /* Power Up EP */
	    bcm963xx_fp_set_power(phc, 0, true, HCD_DEV_EP);

	    /* DS_PERST_ASSERT=0 */
	    hard_dbg &= (~MISC_HARD_DEBUG_DS_PERST_ASSERT);
	    hcd_writel(hard_dbg, phc, MISC_HARD_DEBUG_REG);
	} else {
	    /*
	     * DS_PERST____________________________________________________________
	     * PERSTB  ________________|-------------------------------------------
	     * EP_GPIO _____|------------------------------------------------------
	     *
	     * PERST# ______________________|--------------------------------------
	     * REFCLK XXXXXXXXXXXXXXXXXX|||||_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|
	     */
	    /* Bring the core out of Soft Reset */
	    bcm963xx_hc_core_set_reset(phc, FALSE);
	}

	/* this is a critical delay */
	mdelay(100);

	HCD_FN_EXT();

	return 0;
}

/*
 * Function bcm963xx_hc_core_power(phc, devidx, up, type)
 *
 *   Parameters:
 *    phc    ... pointer to core control block
 *    devidx ... core device index (not used)
 *    up     ... flag to specify power up/down
 *    type   ... mask to specify EP and/or RC devices
 *
 *   Description:
 *     power up/down the core wrapper function that skips powering up
 *     up end point if epuperstb is set, devidx ignored
 *
 *   Return: 0 success, -ve on failure
 */
static int bcm963xx_hc_core_power(struct pcie_hc_core *phc, unsigned int devidx,
	bool up, int type)
{
	int ret = 0;

	HCD_FN_ENT();

	if (bcm963xx_fp_set_power != NULL) {
	    if ((up == TRUE) && (phc->cfg.epuperstb != 0)) {
	        /* Power Up only RC, EP will be done after PERSTB=1 */
	        type &= (~HCD_DEV_EP);
	    }

	    if (type & HCD_DEV_RC_PHY) {
	        if (up == FALSE) {
	            /* Keep core in the reset state for power down */
	            bcm963xx_hc_core_set_reset(phc, TRUE);
	            /* Assert PHY IDDQ for power down */
	            bcm963xx_hc_phy_assert_iddq(phc, TRUE);
	        } else {
	            /* nothing to do for core due to regular core reset sequence */
	            /* de-assert for power up */
	            bcm963xx_hc_phy_assert_iddq(phc, FALSE);
	        }
	        type &= (~HCD_DEV_RC_PHY);
	    }

	    if (type) {
	        /* Call HCD level set power function */
	        ret = bcm963xx_fp_set_power(phc, devidx, up, type);
	    }
	} else {
	    HCD_ERROR("Error: %s called when bcm963xx_fp_set_power=0x%px\n",
	        __FUNCTION__, bcm963xx_fp_set_power);
	    ret = -EINVAL;
	}

	HCD_FN_RET_VAL(ret);
}

/*
 *
 * Function bcm963xx_hc_errlog_isr (irq, data)
 *
 *
 *   Parameters:
 *  irq  ... PCIe core irq number
 *  data ... pointer to HCD (given through request_irq())
 *
 *   Description:
 *     Read the Error interrupt status. Process all the enabled Error interrupts
 *
 *
 *   Return: IRQ_HANDLED on success, IRQ_NONE on NO Error interrupts
 */
static irqreturn_t bcm963xx_hc_errlog_isr(int irq, void *data)
{
	struct pcie_hc_core *phc = (struct pcie_hc_core*)data;
	unsigned int i;
	uint32 reg_val;
	uint32 int_status;

	HCD_FN_ENT();

	/* Get the UBUS interrupt status */
	int_status = hcd_nolog_readl(phc, CPU_INTR1_INTR_INTR_STATUS_REG);

	i = 0;

	if (int_status & CPU_INTR1_PCIE_UBUS_CPU_INTR) {
	    /* Get the UBUS interrupt status */
	    reg_val = hcd_nolog_readl(phc, UBUS_INTR2_CPU_STATUS_REG);
	    reg_val &= UBUS_INTR2_PCIE_INTR_MASK;

	    /* clear the interrupts, as this is an edge triggered interrupt */
	    hcd_nolog_writel(reg_val, phc, UBUS_INTR2_CPU_CLEAR_REG);

	    /* Process all the available UBUS interrupts */

	    while (reg_val != 0x00000000) {
	        if (reg_val & (1ul << (i))) {
	            if (i >= NUM_PCIE_UBUS_INTR_STR) {
	                HCD_LOG_ERR_INT("Core [%d] UBUS Intr [UnKnown_%d]\r\n",
	                    phc->info.id, i);
	            } else {
	                HCD_LOG_ERR_INT("Core [%d] UBUS Intr [%s]\r\n",
	                    phc->info.id, bcm963xx_hc_ubus_intr_str[i]);
	            }
	            reg_val &= (~(1ul << (i)));
	        }
	        i++;
	    }
	}

	if (int_status & CPU_INTR1_PCIE_INTR_CPU_INTR) {
	    u32 rev = phc->info.rev;

	    /* Get the PCIe Core interrupt status */
	    reg_val = hcd_nolog_readl(phc, INTR2_CPU_STATUS_REG);
	    reg_val &= INTR2_CPU_PCIE_INTR_MASK(rev);

	    /* clear the interrupts, as this is an edge triggered interrupt */
	    hcd_nolog_writel(reg_val, phc, INTR2_CPU_CLEAR_REG);

	    /* Process all the available PCIe interrupts */
	    while (reg_val != 0x00000000) {
	        if (reg_val & INTR2_CPU_PCIE_TGT_BAD_ADDR(rev)) {
	            HCD_LOG_ERR_INT("Core [%d] PCIe Intr [PCIE_TGT_BAD_ADDR]\n",
	                phc->info.id);
	            HCD_LOG_ERR_INT("          ADDR [0x%08x_%08x] DATA [0x%08x]\n",
	                hcd_nolog_readl(phc, MISC_RC_BAD_ADDR_HI_REG),
	                hcd_nolog_readl(phc, MISC_RC_BAD_ADDR_LO_REG),
	                hcd_nolog_readl(phc, MISC_RC_BAD_DATA_REG));

	            /* Clear the contents to capture next address, data */
	            hcd_nolog_writel(MISC_RC_BAD_ADDR_LO_CLEAR, phc,
	                MISC_RC_BAD_ADDR_LO_REG);

	            reg_val &= (~INTR2_CPU_PCIE_TGT_BAD_ADDR(rev));
	        } else if (reg_val & INTR2_CPU_PCIE_TGT_BAD_ACCESS(rev)) {
	            HCD_LOG_ERR_INT("Core [%d] PCIe Intr [PCIE_TGT_BAD_ACCESS]\n",
	                phc->info.id);
	            reg_val &= (~INTR2_CPU_PCIE_TGT_BAD_ACCESS(rev));
	        } else if (reg_val & INTR2_CPU_PCIE_MSTR_UR_ATTN(rev)) {
	            HCD_LOG_ERR_INT("Core [%d] PCIe Intr [PCIE_MSTR_UR_ATTN]\n",
	                phc->info.id);
	            reg_val &= (~INTR2_CPU_PCIE_MSTR_UR_ATTN(rev));
	        } else if (reg_val & INTR2_CPU_PCIE_MSTR_CA_ATTN(rev)) {
	            HCD_LOG_ERR_INT("Core [%d] PCIe Intr [PCIE_MSTR_CA_ATTN]\n",
	                phc->info.id);
	            reg_val &= (~INTR2_CPU_PCIE_MSTR_CA_ATTN(rev));
	        } else if (reg_val & INTR2_CPU_PCIE_MSTR_RETRY_TIMEOUT(rev)) {
	            HCD_LOG_ERR_INT("Core [%d] PCIe Intr [PCIE_MSTR_RETRY_TOUT]\n",
	                phc->info.id);
	            reg_val &= (~INTR2_CPU_PCIE_MSTR_RETRY_TIMEOUT(rev));
	        } else if (reg_val & INTR2_CPU_PCIE_MSTR_FWD_ERR(rev)) {
	            HCD_LOG_ERR_INT("Core [%d] PCIe Intr [PCIE_MSTR_FWD_ERR]\n",
	                phc->info.id);
	            reg_val &= (~INTR2_CPU_PCIE_MSTR_FWD_ERR(rev));
	        } else {
	            HCD_LOG_ERR_INT("Core [%d] PCIe Intr [Unknown_0x%x]\n",
	                phc->info.id, reg_val);
	            reg_val = 0x0;
	        }
	        i++;
	    }
	}

	HCD_FN_EXT();

	return (i > 0) ? IRQ_HANDLED : IRQ_NONE;
}

/*
 * Function bcm963xx_hc_errlog_enable (phc)
 *
 *   Parameters:
 *    phc ... pointer to PCIe core HC data structure
 *
 *   Description:
 *    Setup PCIe core bus error logging
 *
 *   Return: 0 on success, -ve on failure
 */
static int bcm963xx_hc_errlog_enable(struct pcie_hc_core *phc)
{
	uint32 reg_data;
	int err;

	HCD_FN_ENT();

	err = request_irq(phc->res.irq, bcm963xx_hc_errlog_isr, IRQF_SHARED,
	    phc->pdev->name, phc);
	if (err < 0) {
	    HCD_ERROR("failed to request IRQ[%d]: %d\n", phc->res.irq, err);
	    return err;
	}

	/* PCIe Core Interrupts (PCIE_INTR2) */
	/* Enable Bad address and bad access interrupts */
	reg_data = INTR2_CPU_PCIE_INTR_MASK(phc->info.rev);
	hcd_writel(reg_data, phc, INTR2_CPU_MASK_CLEAR_REG);

	/* setup PCIE Core CPU interrupts */
	reg_data = CPU_INTR1_PCIE_INTR_CPU_INTR;
	hcd_writel(reg_data, phc, CPU_INTR1_INTR_MASK_CLEAR_REG);

	/* UBUS Interrupts */
	/* Enable UBUS Intr2 controller interrupt */
	reg_data = UBUS_INTR2_PCIE_INTR_MASK;
	hcd_writel(reg_data, phc, UBUS_INTR2_CPU_MASK_CLEAR_REG);

	/* setup UBUS CPU interrupts */
	reg_data = CPU_INTR1_PCIE_UBUS_CPU_INTR;
	hcd_writel(reg_data, phc, CPU_INTR1_INTR_MASK_CLEAR_REG);

	HCD_LOG("Core [%d] Enabled PCIE/UBUS Error Interrupts\r\n", phc->info.id);

	HCD_FN_EXT();

	return err;
}

/*
 * Function bcm963xx_hc_errlog_disable (phc)
 *
 *   Parameters:
 *    phc ... pointer to PCIe core HC data structure
 *
 *   Description:
 *    Setup PCIe core bus error logging
 *
 *   Return: 0 on success, -ve on failure
 */
static void bcm963xx_hc_errlog_disable(struct pcie_hc_core *phc)
{
	HCD_FN_ENT();

	/* Disable PCIE Core CPU interrupts */
	hcd_writel(CPU_INTR1_PCIE_INTR_CPU_INTR, phc, CPU_INTR1_INTR_MASK_SET_REG);

	/* Disable UBUS CPU interrupts */
	hcd_writel(CPU_INTR1_PCIE_UBUS_CPU_INTR, phc, CPU_INTR1_INTR_MASK_SET_REG);

	if (phc->res.irq > 0)
	    free_irq(phc->res.irq, phc);

	HCD_FN_EXT();

	return;
}

/*
 * Function bcm963xx_hc_setup_rev (phc)
 *
 *   Parameters:
 *    phc ... pointer to PCIe core HC data structure
 *
 *   Description:
 *    Initialize HC core cb based on the core revision
 *
 *   Return: 0 on success, -ve on failure
 */
static int bcm963xx_hc_setup_rev(struct pcie_hc_core *phc)
{
	struct bcm963xx_hc_cb *phc_cb = (struct bcm963xx_hc_cb *)(phc->phc_cb);
	struct bcm963xx_hc_msi *msi = NULL;
	int    chip_id = pmc_hcd_get_chip_id();
	int    chip_rev = pmc_hcd_get_chip_rev();

	HCD_FN_ENT();

	/* Determine the serdes type based on revision */
	if (phc->cfg.serdes == PHY_SERDES_INV) {
	    /* All early cores are Viper gen2 */
	    phc_cb->phy.serdes = PHY_SERDES_VIPER;
	    if (phc->info.gen == PCIE_LINK_SPEED_GEN3) {
	        if ((phc->info.rev > 0x402) && (phc->info.rev <= 0x408)) {
	            /* Gen3 until 4.08 are Black shark */
	            phc_cb->phy.serdes = PHY_SERDES_BLACKSHARK;
	        } else {
	            /* 4.09 is Merlin 8 */
	            phc_cb->phy.serdes = PHY_SERDES_MERLIN8;
	        }
	    }
	} else {
	    /* User specified the serdes type */
	    phc_cb->phy.serdes = phc->cfg.serdes;
	}

	/* So far until 4.02, all SoC's are 28nm, and all are GEN2 ports */
	if (phc_cb->phy.serdes == PHY_SERDES_VIPER) {
	    if (phc->info.rev <= 0x402) {
	        phc_cb->phy.process = PHY_PROCESS_28HPM;
	        phc_cb->phy.frequency = PHY_FREQ_REFCLK_50MHZ;
	    } else {
	        phc_cb->phy.process = PHY_PROCESS_16FFC;

	        if (phc->info.rev < 0x408) {
	            phc_cb->phy.frequency = PHY_FREQ_REFCLK_50MHZ;
	        } else {
	            /* Currently: BCM96764, need to use chip_id/rev later */
	            phc_cb->phy.frequency = PHY_FREQ_REFCLK_80MHZ;
	        }
	    }
	} else if (phc_cb->phy.serdes == PHY_SERDES_BLACKSHARK) {
	    uint32 xtal_freq, rddata;

	    phc_cb->phy.process = PHY_PROCESS_16FFC;

	    /*
	     * xtal freq = (100 * PDIV * MDIV)/NDIV
	     *
	     * Where PDIV = 1, MDIV = 0x40, NDIV from PLL_LOOP_FREQ_NDIV_INT
	     */
	    rddata = hcd_readl(phc, G3_PLL_PLL_LOOP_FREQ_REG);
	    rddata &= G3_PLL_PLL_LOOP_FREQ_NDIV_INT_MASK;
	    xtal_freq = (0x40 * 100)/rddata;
	    if (xtal_freq == 80) {
	            phc_cb->phy.frequency = PHY_FREQ_XTAL_80MHZ;
	    } else {
	            phc_cb->phy.frequency = PHY_FREQ_XTAL_50MHZ;
	    }
	} else if ((phc_cb->phy.serdes == PHY_SERDES_MERLIN8) ||
	            (phc_cb->phy.serdes == PHY_SERDES_MERLIN8_API)) {
	    phc_cb->phy.process = PHY_PROCESS_8NM;
	    phc_cb->phy.frequency = PHY_FREQ_REFCLK_50MHZ; /* ?? */
	} else {
	    HCD_ERROR("Port [%d] unknown Serdes\n", phc->info.id);
	    return -ENODEV;
	}

	HCD_LOG("Port [%d] using %s-%s%s%s Serdes\n", phc->info.id,
	    bcm963xx_hc_phy_process_name[PCIE_PHY_PROCESS(phc)],
	    bcm963xx_hc_phy_serdes_name[PCIE_PHY_SERDES(phc)],
	    ((PCIE_PHY_SERDES(phc) == PHY_SERDES_VIPER) ? ", RefClk : " :
	    ((PCIE_PHY_SERDES(phc) == PHY_SERDES_BLACKSHARK) ? ", Xtal : " : "")),
	    (PCIE_PHY_SERDES(phc) >= PHY_SERDES_MERLIN8) ? "" :
	    bcm963xx_hc_phy_freq_name[PCIE_PHY_FREQUENCY(phc)]);

	if (phc->info.rev < 0x303) {
	    phc_cb->wars.g2defset = 1;
	    HCD_LOG("Core [%d] Enable GEN2 Default settings WAR\r\n", phc->info.id);
	} else {
	    phc_cb->wars.g2defset = 0;
	}

	/* These cores require reset through PERIPH MISC base */
	if ((phc->info.rev < 0x320) && (phc->info.misc == NULL)) {
	    HCD_ERROR("Core [%d] missing MISC PCIE CONTROL info\n", phc->info.id);
	    return -ENODEV;
	}

	if ((phc->info.rev == 0x320) && (phc->info.id == 1) &&
	    (((chip_id == 0x68360) && (chip_rev == 0xA0)) ||
	    ((chip_id == 0x6858) && (chip_rev == 0xB0)))) {
	    /* For 68360A0, 6858B0 enable Bifurcation System Clock WAR on port 1 */
	    phc_cb->wars.bifp1sysclk = 1;
	    HCD_LOG("Core [%d] Enable bifp1sysclk WAR\r\n", phc->info.id);
	} else {
	    phc_cb->wars.bifp1sysclk = 0;
	}

	if (phc->info.gen == PCIE_LINK_SPEED_GEN3) {

	    phc_cb->wars.g3txclk = 0;
	    phc_cb->wars.g3rxset = 0;

	    if ((phc->info.rev == 0x401) && BCM94912_CHIP(chip_id) &&
	        (chip_rev == 0xA0)) {
	        phc_cb->wars.g3cmosdrvopclk = 1;
	        phc_cb->wars.g3refclkdetmode = 1;
	        HCD_LOG("Core [%d] Enable g3cmosdrvopclk WAR\r\n", phc->info.id);
	        HCD_LOG("Core [%d] Enable g3refclkdetmode WAR\r\n", phc->info.id);
	    } else {
	        phc_cb->wars.g3cmosdrvopclk = 0;
	        phc_cb->wars.g3refclkdetmode = 0;
	    }
	}

	/*
	 * For 4912B0 16FFC viper based port#2, Adjust PLL Frequency
	 * by -5000ppm to reduce jitter due to PLL coupling
	 */
	if ((PCIE_PHY_PROCESS(phc) == PHY_PROCESS_16FFC) && (phc->cfg.plladj) &&
	    BCM94912_CHIP(chip_id) && (chip_rev == 0x1B0) && (phc->info.id == 2)) {
	    phc_cb->wars.g2pllcoupling = 1;
	    HCD_LOG("Core [%d] Enable g2pllcoupling WAR\r\n", phc->info.id);
	} else {
	    phc_cb->wars.g2pllcoupling = 0;
	}

	/*
	 * For 4912B0 GEN3 dual lane port#1, Transmit Clock Alignment
	 * for out-of phase between lanes
	 */
	if ((phc->info.gen == PCIE_LINK_SPEED_GEN3) && (phc->cfg.linkupretries) &&
	    BCM94912_CHIP(chip_id) && (chip_rev == 0x1B0) && (phc->info.id == 1)) {
	    phc_cb->wars.g3tcaerrata = 1;
	    HCD_LOG("Core [%d] Enable g3tcaerrata WAR\r\n", phc->info.id);
	} else {
	    phc_cb->wars.g3tcaerrata = 0;
	}

	/* Explicitly set mdio PHY lane#
	 * All SoC's with bifurcation support
	 *  bifurcated ports (0,1) +
	 * in single lane mode + no lane# setting, needs explicitly set the lane#
	 */
	if ((((chip_id == 0x6858) && (chip_rev >= 0xB0)) ||
	    ((chip_id == 0x63158) && (chip_rev >= 0xB0)) ||
	    (chip_id == 0x68560)) &&
	    (phc->info.id <= 1) && (phc->res.link_width == 1) &&
	    (phc_cb->phy.lane == 0)) {
	    phc_cb->phy.lane = (1 << phc->info.id);
	    HCD_LOG("Core [%d] Using Bifurcated Single Lane [%d]\n", phc->info.id,
	        phc_cb->phy.lane-1);
	}

	/* Tested only with 28HFC/GEN2 serdes Only */
	if ((phc->cfg.phyrxfilter) && (phc->info.gen == PCIE_LINK_SPEED_GEN2)) {
	    phc_cb->wars.rxsigdetflt = 1;
	    HCD_LOG("Core [%d] Enable rxsigdetflt WAR\r\n", phc->info.id);
	} else {
	    phc_cb->wars.rxsigdetflt = 0;
	}

	/*
	 * Enable PTM Capability Advertisement (supported internally) for
	 * 6765 A0/A1 SoC
	 */
	if ((chip_id == 0x6765) && (chip_rev <= 0xA1)) {
	    phc_cb->wars.ptmcapadv = 1;
	} else {
	    phc_cb->wars.ptmcapadv = 0;
	}

	/*
	 * Enable Phase only clock compensation WAR for 6764 A0/A1 SoC's
	 * that use rev 4.08
	 */
	if (phc->info.rev == 0x0408) {
	    phc_cb->wars.phaseonlyckcmp = 1;
	}

	msi = &phc_cb->msi;
	if (phc->info.rev >= 0x0303) {
	    msi->intr_status = MSI_INTR2_CPU_STATUS_REG;
	    msi->intr_clear = MSI_INTR2_CPU_CLEAR_REG;
	    msi->intr_mask_set = MSI_INTR2_CPU_MASK_SET_REG;
	    msi->intr_mask_clear = MSI_INTR2_CPU_MASK_CLEAR_REG;
	    msi->intr_bitshift = MSI_INTR2_CPU_MSI_INTR_SHIFT;
	    msi->intr_bitmask = MSI_INTR2_CPU_MSI_INTR_MASK;
	    msi->cpu_intr_bitmask = CPU_INTR1_PCIE_MSI_INTR_CPU_INTR;
	} else {
	    msi->intr_status = INTR2_CPU_STATUS_REG;
	    msi->intr_clear = INTR2_CPU_CLEAR_REG;
	    msi->intr_mask_set = INTR2_CPU_MASK_SET_REG;
	    msi->intr_mask_clear = INTR2_CPU_MASK_CLEAR_REG;
	    msi->intr_bitshift = INTR2_CPU_MSI_INTR_SHIFT;
	    msi->intr_bitmask = INTR2_CPU_MSI_INTR_MASK;
	    msi->cpu_intr_bitmask = CPU_INTR1_PCIE_INTR_CPU_INTR;
	}

	phc->cfg.msi_intr_bitshift = msi->intr_bitshift;
	phc->cfg.msi_map_size = MAX_MSI_MAP_SIZE - msi->intr_bitshift;

	/* Set bar mask configuration */
	if (phc->cfg.barmask == 0) {
	    phc->cfg.barmask = (BAR123_MASK | BAR456_MASK);
	}

	if (phc->info.rev < 0x0404) {
	    /* Supports BAR#1 -> BAR#3 */
	    phc->cfg.barmask &= BAR123_MASK;
	}

	HCD_FN_EXT();

	return 0;
}

/*
 * Function bcm963xx_hc_probe (phc)
 *
 *   Parameters:
 *    phc ... pointer to PCIe core data structure
 *
 *   Description:
 *    PCIe HC driver probe. Called for each instance of the PCIe core.
 *    Allocate control block and initialize the default resources
 *
 *   Return: 0 on success, -ve on failure
 */
static int bcm963xx_hc_probe(struct pcie_hc_core *phc)
{
	struct bcm963xx_hc_cb *phc_cb = NULL;
	int err = 0;

	HCD_FN_ENT();

	/* Allocate HC control block */
	phc_cb = (struct bcm963xx_hc_cb*) kzalloc(sizeof(*phc_cb), GFP_KERNEL);
	if (!phc_cb) {
	    HCD_ERROR("[%d] Unable to allocate memory for CB\r\n", phc->info.id);
	    err =  -ENOMEM;
	    goto done;
	}
	HCD_INFO("Core [%d] Allocated [0x%px] hc\r\n", phc->info.id, phc_cb);

	/* Initialize HC elements */
	phc->phc_cb = (void*)phc_cb;
	phc->cfg.core_rev_offset = MISC_REVISION_REG;
	phc->cfg.core_rev_mask = 0xFFFFFF;
	phc->cfg.serdes = PHY_SERDES_INV;

	/* only one device under the core */
	phc->info.devs = 1;

done:
	HCD_FN_EXT();

	return err;
}

/*
 * Function bcm963xx_hc_remove (phc)
 *
 *
 *   Parameters:
 *    phc ... pointer to PCIe core HC data structure
 *
 *   Description:
 *    PCIe HC driver remove - Free the allocated resources
 *
 *   Return: 0 on success, -ve on failure
 */
static void bcm963xx_hc_remove(struct pcie_hc_core *phc)
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
 * Function pcie_hc_plt_init (hc_cfg)
 *
 *
 *   Parameters:
 *    hc_cfg ... pointer to PCIe host controller configuration data structure
 *
 *   Description:
 *    fill the HC configuration
 *
 *   Return: 0 on success, -ve on failure
 */
int pcie_phc_plt_init(struct pcie_hc_plt_cfg *hc_cfg)
{
	int ret = 0;

	HCD_FN_ENT();

	/* Sanity Check */
	if (hc_cfg == NULL) {
	    HCD_ERROR("bcm963xx: NULL hc_cfg\n");
	    return -EINVAL;
	}

	/* fill the configuration items */
	hc_cfg->num_cores = NUM_PCIE_CORES;
	hc_cfg->flags = 0;

	/* setup the platform device names (for built-in, non-dt legacy) */
	if (hc_cfg->plt_dev) {
	    int core;
	    for (core = 0; core < NUM_PCIE_CORES; core++) {
	        hc_cfg->plt_dev[core].name = BCM963XX_PCIE_HC_DEV_NAME;
	    }
	}

	/* PCI operations */
	hc_cfg->pci_fops = &bcm963xx_hc_ops_specific;

	/* initialize bcm963xx fops */
	hc_cfg->init_core = bcm963xx_hc_probe;
	hc_cfg->free_core = bcm963xx_hc_remove;
	hc_cfg->setup_rev = bcm963xx_hc_setup_rev;
	hc_cfg->read_reg = hcd_readl;
	hc_cfg->write_reg = hcd_writel;
	hc_cfg->config_core = bcm963xx_hc_core_pre_config;
	hc_cfg->unconfig_core = bcm963xx_hc_core_unconfig;
	hc_cfg->config_core_post = bcm963xx_hc_core_post_config;

	hc_cfg->reset_core = bcm963xx_hc_core_reset;
	hc_cfg->is_linkup = bcm963xx_hc_is_pcie_link_up;
	hc_cfg->map_core_irq = bcm963xx_hc_map_irq;

	hc_cfg->setup_msi = bcm963xx_hc_msi_enable;
	hc_cfg->teardown_msi = bcm963xx_hc_msi_disable;
	hc_cfg->get_msi = bcm963xx_hc_msi_isr;

	hc_cfg->setup_errint = bcm963xx_hc_errlog_enable;
	hc_cfg->teardown_errint = bcm963xx_hc_errlog_disable;

	/* Use a wrapper on top of HCD set_power function */
	bcm963xx_fp_set_power = hc_cfg->set_power;
	hc_cfg->set_power = bcm963xx_hc_core_power;

	HCD_FN_EXT();

	return ret;
}
