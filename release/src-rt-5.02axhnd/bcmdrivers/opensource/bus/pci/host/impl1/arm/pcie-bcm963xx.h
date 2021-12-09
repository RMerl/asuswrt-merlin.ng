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
#ifndef __PCIE_BCM963XX_H
#define __PCIE_BCM963XX_H

#ifdef __cplusplus
extern "C" {
#endif
#include <linux/of_pci.h>
#include <linux/of_irq.h>

#include <bcm_map_part.h>
#include <bcm_intr.h>

/*
 * +-----------------------------------------------------
 *  Defines
 * +-----------------------------------------------------
 */

/*
 * +-----------------------------------------------------
 * Hardware registers
 * +-----------------------------------------------------
 */
#define BCM963XX_PCIE_PHYS_SIZE                    0x00010000
#define BCM963XX_PCIE_MEMS_SIZE                    0x10000000
#define BCM963XX_DDR_UBUS_ADDRESS_BASE             0x00000000

/*
 * +-----------------------------------------------------
 * PCIe core register blocks
 * +-----------------------------------------------------
 */
#define PCIE_RC_CFG_VENDOR_REGS_OFFSET             0x0180
#define PCIE_RC_CFG_L1SUB_REGS_OFFSET              0x0240
#define PCIE_BLK_404_REGS_OFFSET                   0x0404
#define PCIE_BLK_428_REGS_OFFSET                   0x0428
#define PCIE_BLK_800_REGS_OFFSET                   0x0800
#define PCIE_BLK_1000_REGS_OFFSET                  0x1000
#define PCIE_BLK_1800_REGS_OFFSET                  0x1800
#define PCIE_MISC_REGS_OFFSET                      0x4000
#define PCIE_MISC_HARD_REGS_OFFSET                 0x4200
#define PCIE_MISC_PERST_REGS_OFFSET                0x4100
#define PCIE_MISC_HARD_REGS_OFFSET                 0x4200
#define PCIE_L2_INTR_CTRL_REGS_OFFSET              0x4300
#define PCIE_DMA_REGS_OFFSET                       0x4400
#define PCIE_MSI_INTR2_REGS_OFFSET                 0x4500 /* rev >= 3.03 */
#define PCIE_EXT_CFG_DEV_OFFSET                    0x8000
#define PCIE_EXT_CFG_REGS_OFFSET                   0x9000
#define PCIE_UBUS_L2_INTR_CTRL_REGS_OFFSET         0x9100
#define PCIE_IPI_L2_INTR_CTRL_REGS_OFFSET          0x9200
#define PCIE_PCIE_INTR1_REGS_OFFSET                0x9300
#define PCIE_CPU_INTR1_REGS_OFFSET                 0x9400
#define PCIE_G3_PLL_REGS_OFFSET                    0xA010

/*
 * +-----------------------------------------------------
 * RC_CFG_TYPE1
 * +-----------------------------------------------------
 */
#define RC_CFG_TYPE1_DEVICE_VENDOR_ID_OFFSET       (0x0000)
#define RC_CFG_TYPE1_REV_ID_CLASS_CODE_OFFSET      (0x0008)
#define RC_CFG_PCIE_DEVICE_STATUS_CONTROL_OFFSET   (0x00B4)
#define RC_CFG_PCIE_LINK_CAPABILITY_OFFSET         (0x00B8)
#define RC_CFG_PCIE_LINK_CAP_LINK_SPEED(cap)       ((cap) & 0xF)
#define PCIE_LINK_WIDTH_1LANE                      (1)
#define PCIE_LINK_WIDTH_2LANES                     (2)
#define PCIE_LINK_WIDTH_4LANES                     (4)
#define PCIE_LINK_WIDTH_8LANES                     (8)
#define RC_CFG_PCIE_LINK_CAP_LINK_WIDTH(cap)       ((cap & 0x3F0)>>4)
#define RC_CFG_PCIE_LINK_STATUS_CONTROL_OFFSET     (0x00BC)
#define RC_CFG_PCIE_LINK_STAT_LINK_WIDTH(stat)     ((stat & 0x3F00000)>>20)
#define RC_CFG_PCIE_LINK_STAT_LINK_SPEED(stat)     ((stat & 0xF0000)>>16)
#define RC_CFG_PCIE_LINK_STATUS_CONTROL_2_OFFSET   (0x00DC)
#define RC_CFG_PCIE_LINK_CTRL_TGT_LINK_SPEED_MASK  0x00000003

/*
 * +-----------------------------------------------------
 * RC_CFG_PRIV1
 * +-----------------------------------------------------
 */
#define RC_CFG_PRIV1_ID_VAL3_OFFSET                (PCIE_BLK_428_REGS_OFFSET+0x0014)
#define RC_CFG_PRIV1_ID_VAL3_REVISION_ID_MASK      0xff000000
#define RC_CFG_PRIV1_DEVICE_CAPABILITY_OFFSET      (PCIE_BLK_428_REGS_OFFSET+0x00AC)
#define RC_CFG_PRIV1_LINK_CAPABILITY_OFFSET        (PCIE_BLK_428_REGS_OFFSET+0x00B4)
#define RC_CFG_PRIV1_LINK_CAP_LINK_SPEED_MASK      0x00000003

/*
 * +-----------------------------------------------------
 * RC_DL
 * +-----------------------------------------------------
 */
#define RC_DL_MDIO_ADDR_OFFSET                     (PCIE_BLK_1000_REGS_OFFSET+0x0100)
#define RC_DL_MDIO_WR_DATA_OFFSET                  (PCIE_BLK_1000_REGS_OFFSET+0x0104)
#define RC_DL_MDIO_RD_DATA_OFFSET                  (PCIE_BLK_1000_REGS_OFFSET+0x0108)
#define RC_DL_DL_STATUS_OFFSET                     (PCIE_BLK_1000_REGS_OFFSET+0x0048)
#define RC_DL_DL_STATUS_PHYLINKUP_MASK             0x00002000

/*
 * +-----------------------------------------------------
 * PCI-E Miscellaneous Registers
 * +-----------------------------------------------------
 */
#define MISC_CTRL_OFFSET                           (PCIE_MISC_REGS_OFFSET+0x0008)
#define MISC_CTRL_MAX_BURST_SIZE_64B               (0 << 20)
#define MISC_CTRL_MAX_BURST_SIZE_128B              (1 << 20)
#define MISC_CTRL_MAX_BURST_SIZE_MASK              (3 << 20)
#define MISC_CTRL_BURST_ALIGN_MASK(rev)            ((rev >= 0x0310) ? (7 << 17) : (1 << 19))
#define MISC_CTRL_BURST_ALIGN(rev, b)              ((rev >= 0x0310) ? (b << 17) : (b << 19))
#define MISC_CTRL_CFG_READ_UR_MODE                 (1 << 13)
#define MISC_CTRL_PCIE_IN_WR_COMBINE               (1 << 11)
#define MISC_CTRL_PCIE_RCB_MPS_MODE                (1 << 10)
#define MISC_CTRL_PCIE_RCB_64B_MODE                (1 << 7)
#define MISC_MSI_BAR_CONFIG_LO_OFFSET              (PCIE_MISC_REGS_OFFSET+0x0044)
#define MISC_MSI_BAR_CONFIG_LO_MATCH_ADDR_MASK     0xfffffffc
#define MISC_MSI_BAR_CONFIG_LO_ENABLE_MASK         0x00000001
#define MISC_MSI_BAR_CONFIG_HI_OFFSET              (PCIE_MISC_REGS_OFFSET+0x0048)
#define MISC_MSI_DATA_CONFIG_OFFSET                (PCIE_MISC_REGS_OFFSET+0x004C)
#define MISC_MSI_DATA_CONFIG_MATCH_MASK            0xffff0000
#define MISC_MSI_DATA_CONFIG_MATCH_SHIFT           16
#define MISC_REVISION_OFFSET                       (PCIE_MISC_REGS_OFFSET+0x006C)
#define MISC_CPU_2_PCIE_MEM_WIN0_BASE_LIMIT_OFFSET (PCIE_MISC_REGS_OFFSET+0x0070)
#define MISC_CPU_2_PCI_MEM_WIN_LO_BASE_LIMIT_LIMIT_MASK  0xfff00000
#define MISC_CPU_2_PCI_MEM_WIN_LO_BASE_LIMIT_LIMIT_SHIFT 20
#define MISC_CPU_2_PCI_MEM_WIN_LO_BASE_LIMIT_BASE_SHIFT  4
#define MISC_CPU_2_PCIE_MEM_WIN0_LO_OFFSET         (PCIE_MISC_REGS_OFFSET+0x000C)
#define MISC_CPU_2_PCI_MEM_WIN_LO_BASE_ADDR_MASK   0xfff00000
#define MISC_CPU_2_PCIE_MEM_WIN0_HI_OFFSET         (PCIE_MISC_REGS_OFFSET+0x0010)
#define MISC_RC_BAR1_CONFIG_LO_OFFSET              (PCIE_MISC_REGS_OFFSET+0x002C)
#define MISC_RC_BAR_CONFIG_LO_SIZE_MAX             0x14
#define MISC_RC_BAR_CONFIG_LO_MATCH_ADDRESS_MASK   0xfff00000
#define MISC_RC_BAR1_CONFIG_HI_OFFSET              (PCIE_MISC_REGS_OFFSET+0x0030)
#define MISC_PCIE_CTRL_OFFSET                      (PCIE_MISC_REGS_OFFSET+0x0064)
#define MISC_PCIE_CTRL_PCIE_PERSTB                 (1 << 2)
#define MISC_UBUS_CTRL_OFFSET                      (PCIE_MISC_REGS_OFFSET+0x00A4)
#define MISC_UBUS_CTRL_UBUS_WR_WITH_REPLY          (1 << 14)
#define MISC_UBUS_BAR1_CONFIG_REMAP_OFFSET         (PCIE_MISC_REGS_OFFSET+0x00AC)
#define MISC_UBUS_BAR_CONFIG_ACCESS_EN             (1 << 0)
#define MISC_HARD_DEBUG_OFFSET                     (PCIE_MISC_HARD_REGS_OFFSET+0x0004)
#define MISC_HARD_DEBUG_REFCLK_OVERRIDE_OUT        (1 << 20)
#define MISC_HARD_DEBUG_REFCLK_OVERRIDE_IN_MASK    (7 << 17)
#define MISC_HARD_DEBUG_REFCLK_OVERRIDE_IN(x)      (x << 17)
#define MISC_HARD_DEBUG_REFCLK_OVERRIDE            (1 << 16)
/*
 * +-----------------------------------------------------
 * PCIE EXTERNAL CFG Registers
 * +-----------------------------------------------------
 */
#define EXT_CFG_PCIE_EXT_CFG_INDEX_OFFSET          (PCIE_EXT_CFG_REGS_OFFSET+0x0000)
#define EXT_CFG_BUS_NUM_SHIFT                      20
#define EXT_CFG_DEV_NUM_SHIFT                      15
#define EXT_CFG_FUNC_NUM_SHIFT                     12

/*
 * +-----------------------------------------------------
 * INTR2: PCI-E L2 Interrupt Controller Registers
 * +-----------------------------------------------------
 */
#define INTR2_CPU_STATUS_OFFSET                    (PCIE_L2_INTR_CTRL_REGS_OFFSET+0x0000)
#define INTR2_CPU_CLEAR_OFFSET                     (PCIE_L2_INTR_CTRL_REGS_OFFSET+0x0008)
#define INTR2_CPU_MASK_STATUS_OFFSET               (PCIE_L2_INTR_CTRL_REGS_OFFSET+0x000C)
#define INTR2_CPU_MASK_SET_OFFSET                  (PCIE_L2_INTR_CTRL_REGS_OFFSET+0x0010)
#define INTR2_CPU_MASK_CLEAR_OFFSET                (PCIE_L2_INTR_CTRL_REGS_OFFSET+0x0014)
#define INTR2_CPU_PCIE_TGT_BAD_ADDR(rev)           (((rev) >= 0x303) ? (1 << 18) : (1 << 8))
#define INTR2_CPU_PCIE_TGT_BAD_ACCESS(rev)         (((rev) >= 0x303) ? (1 << 19) : (1 << 9))
#define INTR2_CPU_PCIE_MSTR_UR_ATTN(rev)           (((rev) >= 0x303) ? (1 << 22) : (1 << 12))
#define INTR2_CPU_PCIE_MSTR_CA_ATTN(rev)           (((rev) >= 0x303) ? (1 << 23) : (1 << 13))
#define INTR2_CPU_PCIE_MSTR_RETRY_TIMEOUT(rev)     (((rev) >= 0x303) ? (1 << 24) : (1 << 14))
#define INTR2_CPU_PCIE_MSTR_FWD_ERR(rev)           (((rev) >= 0x303) ? (1 << 25) : (1 << 15))
#define INTR2_CPU_PCIE_INTR_MASK(rev)              (                               \
	INTR2_CPU_PCIE_TGT_BAD_ADDR(rev)       | INTR2_CPU_PCIE_TGT_BAD_ACCESS(rev) |  \
	INTR2_CPU_PCIE_MSTR_UR_ATTN(rev)       | INTR2_CPU_PCIE_MSTR_CA_ATTN(rev)   |  \
	INTR2_CPU_PCIE_MSTR_RETRY_TIMEOUT(rev) | INTR2_CPU_PCIE_MSTR_FWD_ERR(rev))
#define INTR2_CPU_MSI_INTR_MASK                    0xFF000000
#define INTR2_CPU_MSI_INTR_SHIFT                   24

/*
 * +-----------------------------------------------------
 * MSI_INTR2: PCI-E MSI L2 Interrupt Controller Registers
 * +-----------------------------------------------------
 */
/* MSI interrupts as a seperate block (rev >= 3.03) */
#define MSI_INTR2_CPU_STATUS_OFFSET                (PCIE_MSI_INTR2_REGS_OFFSET+0x0000)
#define MSI_INTR2_CPU_CLEAR_OFFSET                 (PCIE_MSI_INTR2_REGS_OFFSET+0x0008)
#define MSI_INTR2_CPU_MASK_SET_OFFSET              (PCIE_MSI_INTR2_REGS_OFFSET+0x0010)
#define MSI_INTR2_CPU_MASK_CLEAR_OFFSET            (PCIE_MSI_INTR2_REGS_OFFSET+0x0014)
#define MSI_INTR2_CPU_MSI_INTR_MASK                0xFFFFFFFF
#define MSI_INTR2_CPU_MSI_INTR_SHIFT               0

/*
 * +-----------------------------------------------------
 * UBUS_INTR2: PCI-E UBUS L2 Interrupt Controller Registers
 * +-----------------------------------------------------
 */
#define UBUS_INTR2_CPU_STATUS_OFFSET               (PCIE_UBUS_L2_INTR_CTRL_REGS_OFFSET+0x0000)
#define UBUS_INTR2_PCIE_LINKUP_INTR                (1 << 0)
#define UBUS_INTR2_PCIE_LINKDOWN_INTR              (1 << 1)
#define UBUS_INTR2_PCIE_INV_CMD_ERR_INTR           (1 << 2)
#define UBUS_INTR2_PCIE_ADDR_RANGE_ERR_INTR        (1 << 3)
#define UBUS_INTR2_PCIE_UBUS_TOUT_ERR_INTR         (1 << 4)
#define UBUS_INTR2_PCIE_REG_ACC_ERR_INTR           (1 << 5)
#define UBUS_INTR2_PCIE_REG_DLEN_ERR_INTR          (1 << 6)
#define UBUS_INTR2_PCIE_MEM_ACC_ERR_INTR           (1 << 7)
#define UBUS_INTR2_PCIE_MEM_WIN_CRSS_ERR_INTR      (1 << 8)
#define UBUS_INTR2_PCIE_PCIE_TGT_RD_RPLY_ERR_INTR  (1 << 9)
#define UBUS_INTR2_PCIE_PCIE_TGT_WR_RPLY_ERR_INTR  (1 << 10)
#define UBUS_INTR2_PCIE_MEM_INV_SWAP_ERR_INTR      (1 << 11)
#define UBUS_INTR2_PCIE_PCIE_INV_SWAP_ERR_INTR     (1 << 12)
#define UBUS_INTR2_PCIE_UBUS_SPARSE_ERR_INTR       (1 << 13)
#define UBUS_INTR2_PCIE_UBUS_SEC_ERR_INTR          (1 << 14)
#define UBUS_INTR2_PCIE_UBUS_WRAP_ERR_INTR         (1 << 15)
#define UBUS_INTR2_PCIE_INTR_MASK                  (0x0000FFFF)
#define UBUS_INTR2_CPU_SET_OFFSET                  (PCIE_UBUS_L2_INTR_CTRL_REGS_OFFSET+0x0004)
#define UBUS_INTR2_CPU_CLEAR_OFFSET                (PCIE_UBUS_L2_INTR_CTRL_REGS_OFFSET+0x0008)
#define UBUS_INTR2_CPU_MASK_STATUS_OFFSET          (PCIE_UBUS_L2_INTR_CTRL_REGS_OFFSET+0x000C)
#define UBUS_INTR2_CPU_MASK_SET_OFFSET             (PCIE_UBUS_L2_INTR_CTRL_REGS_OFFSET+0x0010)
#define UBUS_INTR2_CPU_MASK_CLEAR_OFFSET           (PCIE_UBUS_L2_INTR_CTRL_REGS_OFFSET+0x0014)

/*
 * +-----------------------------------------------------
 * CPU_INTR1: PCIE CPU L1 Interrupt Controller Registers
 * +-----------------------------------------------------
 */
#define CPU_INTR1_INTR_INTR_STATUS_OFFSET          (PCIE_CPU_INTR1_REGS_OFFSET+0x0000)
#define CPU_INTR1_INTR_MASK_STATUS_OFFSET          (PCIE_CPU_INTR1_REGS_OFFSET+0x0004)
#define CPU_INTR1_INTR_MASK_SET_OFFSET             (PCIE_CPU_INTR1_REGS_OFFSET+0x0008)
#define CPU_INTR1_INTR_MASK_CLEAR_OFFSET           (PCIE_CPU_INTR1_REGS_OFFSET+0x000C)
#define CPU_INTR1_PCIE_INTA_CPU_INTR               (1 << 1)
#define CPU_INTR1_PCIE_INTB_CPU_INTR               (1 << 2)
#define CPU_INTR1_PCIE_INTC_CPU_INTR               (1 << 3)
#define CPU_INTR1_PCIE_INTD_CPU_INTR               (1 << 4)
#define CPU_INTR1_PCIE_INTR_CPU_INTR               (1 << 5)
#define CPU_INTR1_PCIE_NMI_CPU_INTR                (1 << 6)
#define CPU_INTR1_PCIE_UBUS_CPU_INTR               (1 << 7)
#define CPU_INTR1_PCIE_MSI_INTR_CPU_INTR           (1 << 9)

/*
 * +-----------------------------------------------------
 * UBUS_G3_PLL: PCIE GEN3 PLL Control Registers
 * +-----------------------------------------------------
 */
#define G3_PLL_PLL_RESETS_OFFSET                   (PCIE_G3_PLL_REGS_OFFSET+0x0000)
#define G3_PLL_PLL_RESETS_RESETB                   (1 << 0)
#define G3_PLL_PLL_RESETS_POST_RESETB              (1 << 1)
#define G3_PLL_PLL_NDIV_OFFSET                     (PCIE_G3_PLL_REGS_OFFSET+0x000c)
#define G3_PLL_PLL_NDIV_INT_MASK                   (0x3FF)
#define G3_PLL_PLL_NDIV_INT(d)                     ((d) & G3_PLL_PLL_NDIV_INT_MASK)
#define G3_PLL_PLL_STAT_OFFSET                     (PCIE_G3_PLL_REGS_OFFSET+0x002c)
#define G3_PLL_PLL_STAT_PLL_LOCK                   (1 << 31)
#define G3_PLL_PLL_NDIV_FRAC_HOLDALL_OFFSET        (PCIE_G3_PLL_REGS_OFFSET+0x004c)
#define G3_PLL_PLL_NDIV_FRAC_HOLDALL_HOLA_ALL      (1 << 24)
#define G3_PLL_PLL_NDIV_FRAC_HOLDALL_FRAC_MASK     (0xFFFFFF)
#define G3_PLL_PLL_NDIV_FRAC_HOLDALL_NDIV_FRAC(f)  ((f) & 0xFFFFFF)
#define G3_PLL_PLL_SSC_LIMIT_SSC_MODE_OFFSET       (PCIE_G3_PLL_REGS_OFFSET+0x0050)
#define G3_PLL_PLL_SSC_LIMIT_SSC_MODE_MODE         (1 << 24)
#define G3_PLL_PLL_SSC_LIMIT_SSC_MODE_LIMIT_MASK   (0x3FFFFF)
#define G3_PLL_PLL_SSC_LIMIT_SSC_MODE_LIMIT(l)     ((l) & 0x3FFFFF)
#define G3_PLL_PLL_SSC_STEP_VCOGAIN_OFFSET         (PCIE_G3_PLL_REGS_OFFSET+0x0054)
#define G3_PLL_PLL_SSC_STEP_VCOGAIN_VCO_GAIN_MASK  (0x000F0000)
#define G3_PLL_PLL_SSC_STEP_VCOGAIN_VCO_GAIN(g)    (((g) & 0xF) << 16)
#define G3_PLL_PLL_SSC_STEP_VCOGAIN_SSC_STEP_MASK  (0xFFFF)
#define G3_PLL_PLL_SSC_STEP_VCOGAIN_SSC_STEP(s)    ((s) & 0xFFFF)

#if defined(PCIE0_PHYS_BASE) && defined(PCIE1_PHYS_BASE) && defined(PCIE2_PHYS_BASE) && \
	defined(PCIE3_PHYS_BASE)
#define NUM_CORE                    4
#elif defined(PCIE0_PHYS_BASE) && defined(PCIE1_PHYS_BASE) && defined(PCIE2_PHYS_BASE)
#define NUM_CORE                    3
#elif defined(PCIE0_PHYS_BASE) && defined(PCIE1_PHYS_BASE)
#define NUM_CORE                    2
#else
#define NUM_CORE                    1
#endif

/*
 * +-----------------------------------------------------
 * MSI
 * +-----------------------------------------------------
 */
#define MSI_MAP_MAX_SIZE                           32

/*
 * +-----------------------------------------------------
 *
 *  Structures
 *
 * +-----------------------------------------------------
 */
/*
 * +-----------------------------------------------------
 * MSI
 * +-----------------------------------------------------
 */
/*
 * BCM963xx PCIe Host Controller device resources
 * @chip: PCIe msi controller
 * @used:
 * @domain:
 * @lock:
 * @cpu_intr_bitmask: CPU interrupt bitsmask for MSI
 * @intr_status: MSI Interrupt status register offset
 * @intr_clear: MSI Interrupt clear register offset
 * @intr_mask_set: MSI Interrupt mask set register offset
 * @intr_mask_clear: MSI Interrupt mask clear register offset
 * @intr_bitshift: MSI Interrupts bits poisition on the above registers
 * @intr_bitmask: MSI Interrupts bitsmask
 */
struct bcm963xx_pcie_msi {
	/* this should be the first element. don't move */
	struct msi_controller chip;
	DECLARE_BITMAP(used, MSI_MAP_MAX_SIZE);
	struct irq_domain *domain;
	struct irq_chip irq_ops;
	struct irq_domain_ops domain_ops;
	struct bcm963xx_pcie_hcd *pdrv;
	int map_size;
	struct mutex lock;
	bool enabled;
	uint32 cpu_intr_bitmask;
	uint32 intr_status;
	uint32 intr_clear;
	uint32 intr_mask_set;
	uint32 intr_mask_clear;
	uint32 intr_bitshift;
	uint32 intr_bitmask;
};

/*
 * +-----------------------------------------------------
 * HCD driver
 * +-----------------------------------------------------
 */

/*
 * BCM963xx PCIe Host Controller device resources
 * @base: PCIe core registers Physical address
 * @owin: PCIe core Outgoing Window physical address
 * @bus_range: PCIe core bus number range
 * @pci_addr: Physical address of PCI core to determine the Offset between CPU  and PCIe
 * @domain: PCI domain
 * @irq: interrupt ID
 * @link_width: number of lanes
 */
struct bcm963xx_pcie_hc_res
{
	struct resource base;
	struct resource owin;
	struct resource bus_range;
	u64 pci_addr;
	uint32 domain;
	uint32 irq;
	uint32 link_width;
};

/*
 * BCM963xx PCIe Host Contoller Driver workarounds (wars)
 *
 * @g2defset:    GEN2 default settings workaround
 * @bifp1sysclk: Bifurcation Port1 System Clock workaround
 * @g3txclk:     GEN3 txclock start workaround (MCS63158-ESX00-R)
 * @g3rxset:     GEN3 rx default settings workaround (MCS63158-ESX00-R)
 */
struct bcm963xx_pcie_hcd_war
{
	uint32 g2defset:1;
	uint32 bifp1sysclk:1;
	uint32 g3txclk:1;
	uint32 g3rxset:1;
};

/*
 * BCM963xx PCIe Host Contoller Driver control block
 * @sys: PCI sysdata (only arm32 platforms)
 * @resources: PCIe controller resources
 * @pdev: pointer to platform device
 * @misc_pdev: pointer to misc platform device
 * @core_id: Host Controller core number
 * @core_gen: PCIe core GEN (1,2,3)
 * @core_rev: PCIe core ip revision
 * @wars: PCIe core specific workarounds
 * @base: PCie registers mapped base (virtual)
 * @owin_inited: Outgoing memory Window resource initialization status
 * @cfg: hc configuration parameters
 * @msi: msi control block
 * @errlog_inited: error log configuration status
 */
struct bcm963xx_pcie_hcd
{
#ifdef CONFIG_ARM
	/* Unused, temporary to satisfy ARM arch code
	  * This element should be first in this structure. do not move
	  */
	struct pci_sys_data sys;
#endif
	struct bcm963xx_pcie_hc_res resources;
	struct platform_device *pdev;
	struct platform_device *misc_pdev;
	uint8 core_id;
	uint8 core_gen;
	uint32 core_rev;
	struct bcm963xx_pcie_hcd_war wars;
	void __iomem *base;
	bool owin_inited;
	struct bcm963xx_pcie_hc_cfg hc_cfg;
	struct bcm963xx_pcie_msi *msi;
	struct pci_bus  *bus;
	bool errlog_inited;
};


#ifdef __cplusplus
}
#endif

#endif /* __PCIE_BCM963XX_H */
#endif /* CONFIG_BCM_KF_ARM_BCM963XX */
