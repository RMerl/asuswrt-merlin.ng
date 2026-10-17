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

/*
 * +-----------------------------------------------------
 *  Defines
 * +-----------------------------------------------------
 */

/* Hardware registers */
#define BCM963XX_PCIE_PHYS_SIZE                    0x00010000
#define BCM963XX_PCIE_MEMS_SIZE                    0x10000000
#define BCM963XX_DDR_UBUS_ADDRESS_BASE             0x00000000

/* PCIe core register blocks */
#define PCIE_RC_CFG_VENDOR_REGS                    0x0180
#define PCIE_RC_CFG_L1SUB_REGS                     0x0240
#define PCIE_BLK_404_REGS                          0x0404
#define PCIE_BLK_428_REGS                          0x0428
#define PCIE_BLK_800_REGS                          0x0800
#define PCIE_BLK_1000_REGS                         0x1000
#define PCIE_BLK_1800_REGS                         0x1800
#define PCIE_MISC_REGS                             0x4000
#define PCIE_MISC_PERST_REGS                       0x4100
#define PCIE_MISC_HARD_REGS                        0x4200
#define PCIE_L2_INTR_CTRL_REGS                     0x4300
#define PCIE_MISC_EXT_REGS                         0x4400
#define PCIE_MSI_INTR2_REGS                        0x4500 /* rev >= 3.03 */
#define PCIE_OB_ERR_HNDL_REGS                      0x4600
#define PCIE_DMA_REGS                              0x5000
#define PCIE_EXT_CFG_DATA                          0x8000
#define PCIE_EXT_CFG_REGS                          0x9000
#define UBUS_L2_INTR_CTRL_REGS                     0x9100
#define PCIE_IPI_L2_INTR_CTRL_REGS                 0x9200
#define PCIE_PCIE_INTR1_REGS                       0x9300
#define PCIE_CPU_INTR1_REGS                        0x9400
#define PCIE_STAT_REGS                             0x9500
#define PCIE_SAT_TIMER_REGS                        0x9800
#define PCIE_G3_PLL_REGS                           0xA010

/* RC_CFG_TYPE1 */
#define RC_CFG_TYPE1_DEVICE_VENDOR_ID_REG          (0x0000)
#define RC_CFG_TYPE1_REV_ID_CLASS_CODE_REG         (0x0008)
#define RC_CFG_PCIE_DEVICE_STATUS_CONTROL_REG      (0x00B4)
#define RC_CFG_PCIE_LINK_CAPABILITY_REG            (0x00B8)
#define RC_CFG_PCIE_LINK_CAP_LINK_SPEED(cap)        ((cap) & 0xF)
#define RC_CFG_PCIE_LINK_CAP_LINK_WIDTH(cap)        ((cap & 0x3F0)>>4)
#define RC_CFG_PCIE_LINK_STATUS_CONTROL_REG        (0x00BC)
#define RC_CFG_PCIE_LINK_STAT_LINK_WIDTH(stat)      ((stat & 0x3F00000)>>20)
#define RC_CFG_PCIE_LINK_STAT_LINK_SPEED(stat)      ((stat & 0xF0000)>>16)
#define RC_CFG_PCIE_LINK_STATUS_CONTROL_2_REG      (0x00DC)
#define RC_CFG_PCIE_LINK_CTRL_TGT_LINK_SPEED_MASK   0x00000003

/* RC_CFG_PRIV1 */
#define RC_CFG_PRIV1_ID_VAL3_REG                   (0x043C)
#define RC_CFG_PRIV1_ID_VAL3_REVISION_ID_MASK       0xff000000
#define RC_CFG_PRIV1_DEVICE_CAPABILITY_REG         (0x04D4)
#define RC_CFG_PRIV1_LINK_CAPABILITY_REG           (0x04DC)
#define RC_CFG_PRIV1_LINK_CAP_MAX_LINK_WIDTH_SHIFT  4
#define RC_CFG_PRIV1_LINK_CAP_MAX_LINK_WIDTH_MASK   0x000001F0
#define RC_CFG_PRIV1_LINK_CAP_LINK_SPEED_MASK       0x0000000F
#define RC_CFG_PRIV1_ROOT_CAP_REG                  (0x04F8)
#define RC_CFG_PRIV1_ROOT_CAP_EXT2_CAP_ENA_MASK     0x000000F8
#define RC_CFG_PRIV1_ROOT_CAP_EXT2_CAP_ENA_L1SUB    0x00000008
#define RC_CFG_PRIV1_ROOT_CAP_EXT2_CAP_ENA_SEC_CAP  0x00000010
#define RC_CFG_PRIV1_ROOT_CAP_EXT2_CAP_ENA_PTM_CAP  0x00000020
#define RC_CFG_PRIV1_PCIER_SECPCE_EQ_CONTROL_0     (0x0638)

/* RC_TL */
#define RC_TL_FCIMM_NP_LIMIT_REG                   (PCIE_BLK_800_REGS+0x0210)
#define RC_TL_FCIMM_NP_LIMIT_ENA_FC_NP_UPD_10US     (0x02000000)
#define RC_TL_FCIMM_NP_LIMIT_FC_NP_USCNT_MASK       (0x01E00000)
#define RC_TL_FCIMM_NP_LIMIT_FC_NP_USCNT_SHIFT      (21)
#define RC_TL_FCIMM_NP_LIMIT_ENA_FC_NP_IMMEDIATE    (0x00100000)
#define RC_TL_FCIMM_NP_LIMIT_FC_NPH_IMM_LIMIT_MASK  (0x000FF000)
#define RC_TL_FCIMM_NP_LIMIT_FC_NPH_IMM_LIMIT_SHIFT (12)
#define RC_TL_FCIMM_NP_LIMIT_FC_NPD_IMM_LIMIT_MASK  (0x00000FFF)
#define RC_TL_FCIMM_NP_LIMIT_FC_NPD_IMM_LIMIT_SHIFT (0)
#define RC_TL_FCIMM_P_LIMIT_REG                    (PCIE_BLK_800_REGS+0x0214)
#define RC_TL_FCIMM_P_LIMIT_ENA_FC_P_UPD_10US       (0x02000000)
#define RC_TL_FCIMM_P_LIMIT_FC_P_USCNT_MASK         (0x01E00000)
#define RC_TL_FCIMM_P_LIMIT_FC_P_USCNT_SHIFT        (21)
#define RC_TL_FCIMM_P_LIMIT_ENA_FC_P_IMMEDIATE      (0x00100000)
#define RC_TL_FCIMM_P_LIMIT_FC_PH_IMM_LIMIT_MASK    (0x000FF000)
#define RC_TL_FCIMM_P_LIMIT_FC_PH_IMM_LIMIT_SHIFT   (12)
#define RC_TL_FCIMM_P_LIMIT_FC_PD_IMM_LIMIT_MASK    (0x00000FFF)
#define RC_TL_FCIMM_P_LIMIT_FC_PD_IMM_LIMIT_SHIFT   (0)

/* RC_DL */
#define RC_DL_PDL_CONTROL0_REG                     (PCIE_BLK_1000_REGS+0x0000)
#define RC_DL_PDL_CONTROL1_REG                     (PCIE_BLK_1000_REGS+0x0004)
#define RC_DL_PDL_CONTROL2_REG                     (PCIE_BLK_1000_REGS+0x0008)
#define RC_DL_PDL_CONTROL2_EN_ACK_LAT_TIMER         (1 << 3)
#define RC_DL_PDL_CONTROL3_REG                     (PCIE_BLK_1000_REGS+0x000C)
#define RC_DL_PDL_CONTROL4_REG                     (PCIE_BLK_1000_REGS+0x0010)
#define RC_DL_PDL_CONTROL4_NPH_FC_INIT_MASK         (0xFF000000)
#define RC_DL_PDL_CONTROL4_NPH_FC_INIT_SHIFT        (24)
#define RC_DL_PDL_CONTROL4_PD_FC_INIT_MASK          (0x00FFF000)
#define RC_DL_PDL_CONTROL4_PD_FC_INIT_SHIFT         (12)
#define RC_DL_PDL_CONTROL4_NPD_FC_INIT_MASK         (0x00000FFF)
#define RC_DL_PDL_CONTROL4_NPD_FC_INIT_SHIFT        (0)
#define RC_DL_PDL_CONTROL5_REG                     (PCIE_BLK_1000_REGS+0x0014)
#define RC_DL_PDL_CONTROL5_APSM_L1_GAP_MASK         (0x007F0000)
#define RC_DL_PDL_CONTROL5_APSM_L1_GAP_SHIFT        (16)
#define RC_DL_PDL_CONTROL5_GLOOPBACK                (1 << 9)
#define RC_DL_PDL_CONTROL5_DOWNSTREAM_PORT          (1 << 8)
#define RC_DL_PDL_CONTROL5_PH_INIT_MASK             (0x000000FF)
#define RC_DL_PDL_CONTROL5_PH_INIT_SHIFT            (0)
#define RC_DL_MDIO_ADDR_REG                        (PCIE_BLK_1000_REGS+0x0100)
#define RC_DL_MDIO_WR_DATA_REG                     (PCIE_BLK_1000_REGS+0x0104)
#define RC_DL_MDIO_RD_DATA_REG                     (PCIE_BLK_1000_REGS+0x0108)
#define RC_DL_DL_STATUS_REG                        (PCIE_BLK_1000_REGS+0x0048)
#define RC_DL_DL_STATUS_PHYLINKUP_MASK              (0x00002000)

/* RC_PL */
#define RC_PL_REG_PHY_CTL_1_REG                    (PCIE_BLK_1800_REGS+0x0004)
#define RC_PL_REG_PHY_CTL_1_PWRDN_ENA_NOSYNC        0x00000008

/* PCI-E Miscellaneous Registers */
#define MISC_CTRL_REG                              (PCIE_MISC_REGS+0x0008)
#define MISC_CTRL_MAX_BURST_SIZE_64B                (0 << 20)
#define MISC_CTRL_MAX_BURST_SIZE_128B               (1 << 20)
#define MISC_CTRL_MAX_BURST_SIZE_MASK               (3 << 20)
#define MISC_CTRL_BURST_ALIGN_MASK(rev)             \
	((rev >= 0x0310) ? (7 << 17) : (1 << 19))
#define MISC_CTRL_BURST_ALIGN(rev, b)               \
	((rev >= 0x0310) ? (b << 17) : (b << 19))
#define MISC_CTRL_CFG_READ_UR_MODE                  (1 << 13)
#define MISC_CTRL_PCIE_IN_WR_COMBINE                (1 << 11)
#define MISC_CTRL_PCIE_RCB_MPS_MODE                 (1 << 10)
#define MISC_CTRL_PCIE_RCB_64B_MODE                 (1 << 7)
#define MISC_CTRL_PCIE_IN_CPL_RO                    (1 << 5)
#define MISC_MSI_BAR_CONFIG_LO_REG                 (PCIE_MISC_REGS+0x0044)
#define MISC_MSI_BAR_CONFIG_LO_MATCH_ADDR_MASK      0xfffffffc
#define MISC_MSI_BAR_CONFIG_LO_ENABLE_MASK          0x00000001
#define MISC_MSI_BAR_CONFIG_HI_REG                 (PCIE_MISC_REGS+0x0048)
#define MISC_MSI_DATA_CONFIG_REG                   (PCIE_MISC_REGS+0x004C)
#define MISC_MSI_DATA_CONFIG_MATCH_MASK             0xffff0000
#define MISC_MSI_DATA_CONFIG_MATCH_SHIFT            16
#define MISC_RC_BAD_ADDR_LO_REG                    (PCIE_MISC_REGS+0x0050)
#define MISC_RC_BAD_ADDR_LO_CLEAR                   (0x00000001)
#define MISC_RC_BAD_ADDR_HI_REG                    (PCIE_MISC_REGS+0x0054)
#define MISC_RC_BAD_DATA_REG                       (PCIE_MISC_REGS+0x0058)
#define MISC_REVISION_REG                          (PCIE_MISC_REGS+0x006C)
#define MISC_CPU_2_PCIE_MEM_WIN0_BASE_LIMIT_REG    (PCIE_MISC_REGS+0x0070)
#define MISC_CPU_2_PCIE_MEM_WIN_BASE_LIMIT_REG(win) \
	(MISC_CPU_2_PCIE_MEM_WIN0_BASE_LIMIT_REG+(win)*4)
#define MISC_CPU_2_PCIE_MEM_WIN1_BASE_LIMIT_REG    (PCIE_MISC_REGS+0x0074)
#define MISC_CPU_2_PCIE_MEM_WIN2_BASE_LIMIT_REG    (PCIE_MISC_REGS+0x0078)
#define MISC_CPU_2_PCIE_MEM_WIN3_BASE_LIMIT_REG    (PCIE_MISC_REGS+0x007C)
#define MISC_CPU_2_PCI_MEM_WIN_LO_BASE_LIMIT_LIMIT_MASK  0xfff00000
#define MISC_CPU_2_PCI_MEM_WIN_LO_BASE_LIMIT_LIMIT_SHIFT 20
#define MISC_CPU_2_PCI_MEM_WIN_LO_BASE_LIMIT_BASE_SHIFT  4
#define MISC_CPU_2_PCIE_MEM_WIN0_LO_REG            (PCIE_MISC_REGS+0x000C)
#define MISC_CPU_2_PCI_MEM_WIN_LO_BASE_ADDR_MASK    0xfff00000
#define MISC_CPU_2_PCIE_MEM_WIN_LO_REG(win)         \
	(MISC_CPU_2_PCIE_MEM_WIN0_LO_REG+(win)*8)
#define MISC_CPU_2_PCIE_MEM_WIN0_HI_REG            (PCIE_MISC_REGS+0x0010)
#define MISC_CPU_2_PCIE_MEM_WIN_HI_REG(win)         \
	(MISC_CPU_2_PCIE_MEM_WIN0_HI_REG+(win)*8)
#define MISC_CPU_2_PCIE_MEM_WIN1_LO_REG            (PCIE_MISC_REGS+0x0014)
#define MISC_CPU_2_PCIE_MEM_WIN1_HI_REG            (PCIE_MISC_REGS+0x0018)
#define MISC_CPU_2_PCIE_MEM_WIN2_LO_REG            (PCIE_MISC_REGS+0x001C)
#define MISC_CPU_2_PCIE_MEM_WIN2_HI_REG            (PCIE_MISC_REGS+0x0020)
#define MISC_CPU_2_PCIE_MEM_WIN3_LO_REG            (PCIE_MISC_REGS+0x0024)
#define MISC_CPU_2_PCIE_MEM_WIN3_HI_REG            (PCIE_MISC_REGS+0x0028)
#define MISC_RC_BAR1_CONFIG_LO_REG                 (PCIE_MISC_REGS+0x002C)
#define MISC_RC_BAR_CONFIG_LO_SIZE_MAX              0x14
#define MISC_RC_BAR_CONFIG_LO_SIZE_1GB              0x0F
#define MISC_RC_BAR_CONFIG_LO_SIZE_2GB              0x10
#define MISC_RC_BAR_CONFIG_LO_SIZE_4GB              0x11
#define MISC_RC_BAR_CONFIG_LO_SIZE_8GB              0x12
#define MISC_RC_BAR_CONFIG_LO_SIZE_16GB             0x13
#define MISC_RC_BAR_CONFIG_LO_SIZE_32GB             0x14
#define MISC_RC_BAR_CONFIG_LO_MATCH_ADDRESS_MASK    0xfffff000
#define MISC_RC_BAR1_CONFIG_HI_REG                 (PCIE_MISC_REGS+0x0030)
#define MISC_RC_BAR2_CONFIG_LO_REG                 (PCIE_MISC_REGS+0x0034)
#define MISC_RC_BAR2_CONFIG_HI_REG                 (PCIE_MISC_REGS+0x0038)
#define MISC_RC_BAR3_CONFIG_LO_REG                 (PCIE_MISC_REGS+0x003C)
#define MISC_RC_BAR3_CONFIG_HI_REG                 (PCIE_MISC_REGS+0x0040)
#define MISC_PCIE_CTRL_REG                         (PCIE_MISC_REGS+0x0064)
#define MISC_PCIE_CTRL_PCIE_PERSTB                  (1 << 2)
#define MISC_PCIE_STATUS_REG                       (PCIE_MISC_REGS+0x0068)
#define MISC_PCIE_STATUS_LINK_IN_L1                 (1 << 20)
#define MISC_PCIE_STATUS_LINK_IN_L0                 (1 << 21)
#define MISC_UBUS_CTRL_REG                         (PCIE_MISC_REGS+0x00A4)
#define MISC_UBUS_CTRL_UBUS_WR_WITH_REPLY           (1 << 14)
#define MISC_UBUS_BAR1_CONFIG_REMAP_REG            (PCIE_MISC_REGS+0x00AC)
#define MISC_UBUS_BAR_CONFIG_REMAP_ACCESS_EN        (1 << 0)
#define MISC_UBUS_BAR_CONFIG_REMAP_OFFSET_MASK      0xfffff000
#define MISC_UBUS_BAR1_CONFIG_REMAP_HI_REG         (PCIE_MISC_REGS+0x00B0)
#define MISC_UBUS_BAR2_CONFIG_REMAP_REG            (PCIE_MISC_REGS+0x00B4)
#define MISC_UBUS_BAR2_CONFIG_REMAP_HI_REG         (PCIE_MISC_REGS+0x00B8)
#define MISC_UBUS_BAR3_CONFIG_REMAP_REG            (PCIE_MISC_REGS+0x00BC)
#define MISC_UBUS_BAR3_CONFIG_REMAP_HI_REG         (PCIE_MISC_REGS+0x00C0)
#define MISC_RC_BAR4_CONFIG_LO_REG                 (PCIE_MISC_REGS+0x00C8)
#define MISC_RC_BAR4_CONFIG_HI_REG                 (PCIE_MISC_REGS+0x00CC)
#define MISC_RC_BAR5_CONFIG_LO_REG                 (PCIE_MISC_REGS+0x00D4)
#define MISC_RC_BAR5_CONFIG_HI_REG                 (PCIE_MISC_REGS+0x00D8)
#define MISC_RC_BAR6_CONFIG_LO_REG                 (PCIE_MISC_REGS+0x00DC)
#define MISC_RC_BAR6_CONFIG_HI_REG                 (PCIE_MISC_REGS+0x00E0)
#define MISC_UBUS_BAR4_CONFIG_REMAP_REG            (PCIE_MISC_REGS+0x00E4)
#define MISC_UBUS_BAR4_CONFIG_REMAP_HI_REG         (PCIE_MISC_REGS+0x00E8)
#define MISC_UBUS_BAR5_CONFIG_REMAP_REG            (PCIE_MISC_REGS+0x00EC)
#define MISC_UBUS_BAR5_CONFIG_REMAP_HI_REG         (PCIE_MISC_REGS+0x00F0)
#define MISC_UBUS_BAR6_CONFIG_REMAP_REG            (PCIE_MISC_REGS+0x00F4)
#define MISC_UBUS_BAR6_CONFIG_REMAP_HI_REG         (PCIE_MISC_REGS+0x00F8)
#define MISC_RC_BAR_CONFIG_HI_REG                   0x00000004
#define MISC_RC_BAR_CONFIG_REG(barnum)              \
	((barnum) <= 1) ? MISC_RC_BAR1_CONFIG_LO_REG    : \
	((barnum) == 2) ? MISC_RC_BAR2_CONFIG_LO_REG    : \
	((barnum) == 3) ? MISC_RC_BAR3_CONFIG_LO_REG    : \
	((barnum) == 4) ? MISC_RC_BAR4_CONFIG_LO_REG    : \
	((barnum) == 5) ? MISC_RC_BAR5_CONFIG_LO_REG    : \
	MISC_RC_BAR6_CONFIG_LO_REG
#define MISC_UBUS_BAR_CONFIG_REMAP_REG(barnum)      \
	((barnum) <= 1) ? MISC_UBUS_BAR1_CONFIG_REMAP_REG    : \
	((barnum) == 2) ? MISC_UBUS_BAR2_CONFIG_REMAP_REG    : \
	((barnum) == 3) ? MISC_UBUS_BAR3_CONFIG_REMAP_REG    : \
	((barnum) == 4) ? MISC_UBUS_BAR4_CONFIG_REMAP_REG    : \
	((barnum) == 5) ? MISC_UBUS_BAR5_CONFIG_REMAP_REG    : \
	MISC_UBUS_BAR6_CONFIG_REMAP_REG

#define MISC_HARD_DEBUG_REG                        (PCIE_MISC_HARD_REGS+0x0004)
#define MISC_HARD_DEBUG_SERDES_IDDQ                 (1 << 27)
#define MISC_HARD_DEBUG_REFCLK_OVERRIDE_OUT         (1 << 20)
#define MISC_HARD_DEBUG_REFCLK_OVERRIDE_IN_MASK     (7 << 17)
#define MISC_HARD_DEBUG_REFCLK_OVERRIDE_IN(x)       (x << 17)
#define MISC_HARD_DEBUG_REFCLK_OVERRIDE             (1 << 16)
#define MISC_HARD_DEBUG_DS_PERST_ASSERT             (1 << 3)

/* PCIe Miscellaneous Extension Registers */
#define MISC_EXT_FASTPATH_IN_ADDR_LO_REG           (PCIE_MISC_EXT_REGS+0x0000)
#define FASTPATH_IN_ADDR_LO_BASE_ADDRESS_MASK       (0xFFFFFF00)
#define FASTPATH_IN_ADDR_MESSAGE_LATBITS(b)         (((b) & 0x7) << 4)
#define FASTPATH_IN_ADDR_MESSAGE_BE                 (1 << 3)
#define FASTPATH_IN_ADDR_MUTEX_VISIBILITY           (1 << 2)
#define FASTPATH_IN_ADDR_MUTEX_MODE                 (1 << 1)
#define FASTPATH_IN_ADDR_ENABLE                     (1 << 0)
#define MISC_EXT_FASTPATH_IN_ADDR_HI_REG           (PCIE_MISC_EXT_REGS+0x0004)
#define MISC_EXT_FASTPATH_OUT_ADDR_LO_REG          (PCIE_MISC_EXT_REGS+0x0008)
#define FASTPATH_OUT_ADDR_LO_BASE_ADDRESS_MASK      (0xFFFFFFFC)
#define FASTPATH_OUT_ADDR_ENABLE                    (1 << 0)
#define MISC_EXT_FASTPATH_OUT_ADDR_HI_REG          (PCIE_MISC_EXT_REGS+0x000C)
#define MISC_EXT_FASTPATH_OUT_ATTR_REG             (PCIE_MISC_EXT_REGS+0x0010)
#define FASTPATH_OUT_ATTR_RELAXED_ORDERING          (1 << 4)
#define FASTPATH_OUT_ATTR_NO_SNOOP                  (1 << 3)
#define FASTPATH_OUT_ATTR_TRAFFIC_CLASS(tc)         (((tc)&0x7) << 0)
#define MISC_EXT_MUTEX_0_REG                       (PCIE_MISC_EXT_REGS+0x0014)
#define MISC_EXT_BROADCAST_OUT_MESSAGE_REG         (PCIE_MISC_EXT_REGS+0x0018)
#define MISC_EXT_UBUS_FASTPATH_PORT_CONFIG_REG     (PCIE_MISC_EXT_REGS+0x001C)
#define FASTPATH_PORT_CONFIG_ID_MASK                0xFF
#define FASTPATH_PORT_MESSAGE_LATBITS(i, b)         (((b)&0x7) << (((i)*8)+4))
#define FASTPATH_PORT_MESSAGE_BE(i)                 (1 << (((i)*8)+3))
#define FASTPATH_PORT_MUTEX_VISIBILITY(i)           (1 << (((i)*8)+2))
#define FASTPATH_PORT_MUTEX_MODE(i)                 (1 << (((i)*8)+1))
#define FASTPATH_PORT_ENABLE(i)                     (1 << ((i)*8))
#define MISC_EXT_UBUS_FASTPATH_PORT_MAPPING_REG    (PCIE_MISC_EXT_REGS+0x0020)
#define FASTPATH_PORT_MAPPING_ID_MASK               0xFF
#define UBUS_FASTPATH_PORT_MAPPING(i, p)           \
	    (((p)&FASTPATH_PORT_MAPPING_ID_MASK) << ((i)*8))
#define MISC_EXT_UBUS_FASTPATH_OUT_ADDR0_LO_REG    (PCIE_MISC_EXT_REGS+0x0024)
#define MISC_EXT_UBUS_FASTPATH_OUT_ADDR0_HI_REG    (PCIE_MISC_EXT_REGS+0x0028)
#define UBUS_FASTPATH_OUT_ADDR_LO_REG(i)        \
	(MISC_EXT_UBUS_FASTPATH_OUT_ADDR0_LO_REG+(i)*8)
#define UBUS_FASTPATH_OUT_ADDR_HI_REG(i)        \
	(MISC_EXT_UBUS_FASTPATH_OUT_ADDR0_HI_REG+(i)*8)
#define UBUS_FASTPATH_OUT_ADDR_LO_BASE_ADDRESS_MASK (0xFFFFFFFC)
#define UBUS_FASTPATH_OUT_ADDR_ENABLE               (1 << 0)
#define MISC_EXT_UBUS_FASTPATH_OUT_ATTR_REG        (PCIE_MISC_EXT_REGS+0x0044)
#define UBUS_FASTPATH_OUT_ATTR_PRIORITY(p)          (((p)&0x2) << 4)
#define UBUS_FASTPATH_OUT_ATTR_FLOW(f)              (((f)&0x2) << 2)
#define UBUS_FASTPATH_OUT_ATTR_QUEUE(q)             (((q)&0x2) << 0)
#define MISC_EXT_ECAM_BASE_LIMIT_REG               (PCIE_MISC_EXT_REGS+0x0048)
#define MISC_EXT_ECAM_BASE_LIMIT_LIMIT_MASK         (0xFFF00000)
#define MISC_EXT_ECAM_BASE_LIMIT_BASE_MASK          (0x0000FFF0)
#define MISC_EXT_ECAM_WIN_HI_REG                   (PCIE_MISC_EXT_REGS+0x004C)
#define MISC_EXT_ECAM_WIN_HI_BASE_MASK              (0x000000FF)
#define MISC_EXT_RC_BAR7_CONFIG_LO_REG             (PCIE_MISC_EXT_REGS+0x0050)
#define MISC_EXT_RC_BAR7_CONFIG_HI_REG             (PCIE_MISC_EXT_REGS+0x0054)
#define MISC_EXT_UBUS_BAR7_CONFIG_REMAP_REG        (PCIE_MISC_EXT_REGS+0x0058)
#define MISC_EXT_UBUS_BAR7_CONFIG_REMAP_HI_REG     (PCIE_MISC_EXT_REGS+0x005C)
#define MISC_EXT_RC_BAR8_CONFIG_LO_REG             (PCIE_MISC_EXT_REGS+0x0060)
#define MISC_EXT_RC_BAR8_CONFIG_HI_REG             (PCIE_MISC_EXT_REGS+0x0064)
#define MISC_EXT_UBUS_BAR8_CONFIG_REMAP_REG        (PCIE_MISC_EXT_REGS+0x0068)
#define MISC_EXT_UBUS_BAR8_CONFIG_REMAP_HI_REG     (PCIE_MISC_EXT_REGS+0x006C)

#define MISC_EXT_PCIE_TC_QUEUE_REG                 (PCIE_MISC_EXT_REGS+0x0070)
#define MISC_EXT_PCIE_TC_QUEUE_LOW_PRIO             0
#define MISC_EXT_PCIE_TC_QUEUE_MID_PRIO             1
#define MISC_EXT_PCIE_TC_QUEUE_HIGH_PRIO            2
#define MISC_EXT_PCIE_TC_QUEUE_RESV_PRIO            3 /* Reserved */
#define MISC_EXT_PCIE_TC_QUEUE_TC2Q(tc, q)          ((q) << ((tc)*4))

#define MISC_EXT_PCIE_TC_FLOW_REG                  (PCIE_MISC_EXT_REGS+0x0074)
#define MISC_EXT_PCIE_TC_FLOW_LOW_PRIO              0
#define MISC_EXT_PCIE_TC_FLOW_MID_PRIO              1
#define MISC_EXT_PCIE_TC_FLOW_HIGH_PRIO             2
#define MISC_EXT_PCIE_TC_FLOW_RESV_PRIO             3 /* Reserved */
#define MISC_EXT_PCIE_TC_FLOW_TC2F(tc, f)           ((f) << ((tc)*4))

#define MISC_EXT_PCIE_TC_PRIORITY_REG              (PCIE_MISC_EXT_REGS+0x0078)
#define MISC_EXT_PCIE_TC_UBUS_LOW_PRIO              0
#define MISC_EXT_PCIE_TC_UBUS_HIGH_PRIO             1
#define MISC_EXT_PCIE_TC_UBUS_TC2P(tc, p)           ((p) << ((tc)*4))

#define MISC_EXT_PCIE_OUTGOING_Q_FILTER_0_REG      (PCIE_MISC_EXT_REGS+0x007C)
#define MISC_EXT_PCIE_OUTGOING_Q_FILTER_1_REG      (PCIE_MISC_EXT_REGS+0x0080)
#define MISC_EXT_PCIE_OUTGOING_Q_FILTER_2_REG      (PCIE_MISC_EXT_REGS+0x0084)
#define MISC_EXT_PCIE_OUTGOING_Q_FILTER_3_REG      (PCIE_MISC_EXT_REGS+0x0088)
#define MISC_EXT_PCIE_OUTGOING_Q_FILTER_4_REG      (PCIE_MISC_EXT_REGS+0x008C)
#define MISC_EXT_PCIE_OUTGOING_Q_FILTER_5_REG      (PCIE_MISC_EXT_REGS+0x0090)
#define MISC_EXT_PCIE_OUTGOING_Q_FILTER_6_REG      (PCIE_MISC_EXT_REGS+0x0094)
#define MISC_EXT_PCIE_OUTGOING_Q_FILTER_7_REG      (PCIE_MISC_EXT_REGS+0x0098)
#define MISC_EXT_PCIE_OUTGOING_Q_FILTER_REG(f)      \
	(PCIE_MISC_EXT_REGS+0x007C+(f)*4)
#define MISC_EXT_PCIE_OUTGOING_Q_FILTER_TC(tc)      ((tc) << 0)
#define MISC_EXT_PCIE_OUTGOING_Q_FILTER_QUEUE(q)    ((q) << 5)
#define MISC_EXT_PCIE_OUTGOING_Q_FILTER_FLOW(f)     ((f) << 7)
#define MISC_EXT_PCIE_OUTGOING_Q_FILTER_FLOW_ENABLE (1 << 9)
#define MISC_EXT_PCIE_OUTGOING_Q_FILTER_PORT(p)     ((p) << 12)
#define MISC_EXT_PCIE_OUTGOING_Q_FILTER_PORT_ENABLE (1 << 20)
#define MISC_EXT_PCIE_OUTGOING_Q_FILTER_ENABLE      (1 << 31)

#define MISC_EXT_PCIE_UBUS_READ_REQUEST_ARB_REG    (PCIE_MISC_EXT_REGS+0x00C0)
#define MISC_EXT_PCIE_UBUS_READ_REQUEST_ARB_SP       0
#define MISC_EXT_PCIE_UBUS_READ_REQUEST_ARB_WRR      1
#define MISC_EXT_PCIE_UBUS_READ_REQUEST_ARB_IWRR     2
#define MISC_EXT_PCIE_UBUS_READ_REQUEST_ARB_WEIGHT(a, m, w) \
	(((w) << ((a)*8)) | ((m) << ((a)*8 + 6)))

#define MISC_EXT_PCIE_UBUS_WRITE_REQUEST_ARB_REG   (PCIE_MISC_EXT_REGS+0x00C4)
#define MISC_EXT_UBUS_REQUEST_Q0_ARB_REG           (PCIE_MISC_EXT_REGS+0x00C8)
#define MISC_EXT_UBUS_REQUEST_Q1_ARB_REG           (PCIE_MISC_EXT_REGS+0x00CC)
#define MISC_EXT_UBUS_REQUEST_Q2_ARB_REG           (PCIE_MISC_EXT_REGS+0x00D0)
#define MISC_EXT_UBUS_REQUEST_ARB_REG              (PCIE_MISC_EXT_REGS+0x00D4)
#define MISC_EXT_UBUS_PCIE_REPLY_ARB_REG           (PCIE_MISC_EXT_REGS+0x00D8)
#define MISC_EXT_UBUS_PCIE_READ_WRITE_ARB_REG      (PCIE_MISC_EXT_REGS+0x00DC)
#define MISC_EXT_PCIE_REQUEST_Q0_ARB_REG           (PCIE_MISC_EXT_REGS+0x00E0)
#define MISC_EXT_PCIE_REQUEST_Q1_ARB_REG           (PCIE_MISC_EXT_REGS+0x00E4)
#define MISC_EXT_PCIE_REQUEST_Q2_ARB_REG           (PCIE_MISC_EXT_REGS+0x00E8)
#define MISC_EXT_PCIE_TX_ARB_REG                   (PCIE_MISC_EXT_REGS+0x00EC)
#define MISC_EXT_PCIE_UBUS_REPLY_ARB_REG           (PCIE_MISC_EXT_REGS+0x00F0)


/* PCIE EXTERNAL CFG Registers */
#define EXT_CFG_PCIE_EXT_CFG_INDEX_REG             (PCIE_EXT_CFG_REGS+0x0000)
#define EXT_CFG_BUS_NUM_SHIFT                       20
#define EXT_CFG_DEV_NUM_SHIFT                       15
#define EXT_CFG_FUNC_NUM_SHIFT                      12

/* INTR2: PCI-E L2 Interrupt Controller Registers */
#define INTR2_CPU_STATUS_REG                       (PCIE_L2_INTR_CTRL_REGS+0x00)
#define INTR2_CPU_CLEAR_REG                        (PCIE_L2_INTR_CTRL_REGS+0x08)
#define INTR2_CPU_MASK_STATUS_REG                  (PCIE_L2_INTR_CTRL_REGS+0x0C)
#define INTR2_CPU_MASK_SET_REG                     (PCIE_L2_INTR_CTRL_REGS+0x10)
#define INTR2_CPU_MASK_CLEAR_REG                   (PCIE_L2_INTR_CTRL_REGS+0x14)
#define INTR2_CPU_PCIE_TGT_BAD_ADDR(rev)            \
	(((rev) >= 0x303) ? (1 << 18) : (1 << 8))
#define INTR2_CPU_PCIE_TGT_BAD_ACCESS(rev)          \
	(((rev) >= 0x303) ? (1 << 19) : (1 << 9))
#define INTR2_CPU_PCIE_MSTR_UR_ATTN(rev)            \
	(((rev) >= 0x303) ? (1 << 22) : (1 << 12))
#define INTR2_CPU_PCIE_MSTR_CA_ATTN(rev)            \
	(((rev) >= 0x303) ? (1 << 23) : (1 << 13))
#define INTR2_CPU_PCIE_MSTR_RETRY_TIMEOUT(rev)      \
	(((rev) >= 0x303) ? (1 << 24) : (1 << 14))
#define INTR2_CPU_PCIE_MSTR_FWD_ERR(rev)            \
	(((rev) >= 0x303) ? (1 << 25) : (1 << 15))
#define INTR2_CPU_PCIE_INTR_MASK(rev)              (\
	INTR2_CPU_PCIE_TGT_BAD_ADDR(rev) | INTR2_CPU_PCIE_TGT_BAD_ACCESS(rev) |  \
	INTR2_CPU_PCIE_MSTR_UR_ATTN(rev) | INTR2_CPU_PCIE_MSTR_CA_ATTN(rev)   |  \
	INTR2_CPU_PCIE_MSTR_RETRY_TIMEOUT(rev) | INTR2_CPU_PCIE_MSTR_FWD_ERR(rev))
#define INTR2_CPU_MSI_INTR_MASK                     0xFF000000
#define INTR2_CPU_MSI_INTR_SHIFT                    24

/* MSI_INTR2: PCI-E MSI L2 Interrupt Controller Registers */
/* MSI interrupts as a separate block (rev >= 3.03) */
#define MSI_INTR2_CPU_STATUS_REG                   (PCIE_MSI_INTR2_REGS+0x0000)
#define MSI_INTR2_CPU_CLEAR_REG                    (PCIE_MSI_INTR2_REGS+0x0008)
#define MSI_INTR2_CPU_MASK_SET_REG                 (PCIE_MSI_INTR2_REGS+0x0010)
#define MSI_INTR2_CPU_MASK_CLEAR_REG               (PCIE_MSI_INTR2_REGS+0x0014)
#define MSI_INTR2_CPU_MSI_INTR_MASK                 0xFFFFFFFF
#define MSI_INTR2_CPU_MSI_INTR_SHIFT                0

/* OB_ERR_HNDL: PCIe Outbound Error Handling Registers */
#define OB_ERR_HNDL_ERR_TREATMENT_REG              (PCIE_OB_ERR_HNDL_REGS+0x00)
#define OB_ERR_HNDL_ERR_TREATMENT_CFG_ERR_REPLACE   (1 << 0)
#define OB_ERR_HNDL_ERR_TREATMENT_MEM_ERR_REPLACE   (1 << 1)

/* UBUS_INTR2: PCI-E UBUS L2 Interrupt Controller Registers */
#define UBUS_INTR2_CPU_STATUS_REG                  (UBUS_L2_INTR_CTRL_REGS+0x00)
#define UBUS_INTR2_PCIE_LINKUP_INTR                 (1 << 0)
#define UBUS_INTR2_PCIE_LINKDOWN_INTR               (1 << 1)
#define UBUS_INTR2_PCIE_INV_CMD_ERR_INTR            (1 << 2)
#define UBUS_INTR2_PCIE_ADDR_RANGE_ERR_INTR         (1 << 3)
#define UBUS_INTR2_UBUS_TOUT_ERR_INTR               (1 << 4)
#define UBUS_INTR2_PCIE_REG_ACC_ERR_INTR            (1 << 5)
#define UBUS_INTR2_PCIE_REG_DLEN_ERR_INTR           (1 << 6)
#define UBUS_INTR2_PCIE_MEM_ACC_ERR_INTR            (1 << 7)
#define UBUS_INTR2_PCIE_MEM_WIN_CRSS_ERR_INTR       (1 << 8)
#define UBUS_INTR2_PCIE_PCIE_TGT_RD_RPLY_ERR_INTR   (1 << 9)
#define UBUS_INTR2_PCIE_PCIE_TGT_WR_RPLY_ERR_INTR   (1 << 10)
#define UBUS_INTR2_PCIE_MEM_INV_SWAP_ERR_INTR       (1 << 11)
#define UBUS_INTR2_PCIE_PCIE_INV_SWAP_ERR_INTR      (1 << 12)
#define UBUS_INTR2_UBUS_SPARSE_ERR_INTR             (1 << 13)
#define UBUS_INTR2_UBUS_SEC_ERR_INTR                (1 << 14)
#define UBUS_INTR2_UBUS_WRAP_ERR_INTR               (1 << 15)
#define UBUS_INTR2_PCIE_INTR_MASK                   (0x0000FFFF)
#define UBUS_INTR2_CPU_SET_REG                     (UBUS_L2_INTR_CTRL_REGS+0x04)
#define UBUS_INTR2_CPU_CLEAR_REG                   (UBUS_L2_INTR_CTRL_REGS+0x08)
#define UBUS_INTR2_CPU_MASK_STATUS_REG             (UBUS_L2_INTR_CTRL_REGS+0x0C)
#define UBUS_INTR2_CPU_MASK_SET_REG                (UBUS_L2_INTR_CTRL_REGS+0x10)
#define UBUS_INTR2_CPU_MASK_CLEAR_REG              (UBUS_L2_INTR_CTRL_REGS+0x14)

/* CPU_INTR1: PCIE CPU L1 Interrupt Controller Registers */
#define CPU_INTR1_INTR_INTR_STATUS_REG             (PCIE_CPU_INTR1_REGS+0x0000)
#define CPU_INTR1_INTR_MASK_STATUS_REG             (PCIE_CPU_INTR1_REGS+0x0004)
#define CPU_INTR1_INTR_MASK_SET_REG                (PCIE_CPU_INTR1_REGS+0x0008)
#define CPU_INTR1_INTR_MASK_CLEAR_REG              (PCIE_CPU_INTR1_REGS+0x000C)
#define CPU_INTR1_PCIE_INTA_CPU_INTR                (1 << 1)
#define CPU_INTR1_PCIE_INTB_CPU_INTR                (1 << 2)
#define CPU_INTR1_PCIE_INTC_CPU_INTR                (1 << 3)
#define CPU_INTR1_PCIE_INTD_CPU_INTR                (1 << 4)
#define CPU_INTR1_PCIE_INTR_CPU_INTR                (1 << 5)
#define CPU_INTR1_PCIE_NMI_CPU_INTR                 (1 << 6)
#define CPU_INTR1_PCIE_UBUS_CPU_INTR                (1 << 7)
#define CPU_INTR1_PCIE_MSI_INTR_CPU_INTR            (1 << 9)

/* SAT_TIMER: PCIe Precision Timing Management Satellite Timer Registers */
#define SAT_TIMER_CTRL_REG                         (PCIE_SAT_TIMER_REGS+0x0000)
#define SAT_TIMER_SET_L_REG                        (PCIE_SAT_TIMER_REGS+0x000C)
#define SAT_TIMER_SET_H_REG                        (PCIE_SAT_TIMER_REGS+0x0010)
#define SAT_TIMER_INC_L_REG                        (PCIE_SAT_TIMER_REGS+0x0014)
#define SAT_TIMER_INC_H_REG                        (PCIE_SAT_TIMER_REGS+0x0018)

/* UBUS_G3_PLL: PCIE GEN3 PLL Control Registers */
#define G3_PLL_PLL_RESETS_REG                      (PCIE_G3_PLL_REGS+0x0000)
#define G3_PLL_PLL_RESETS_RESETB                    (1 << 0)
#define G3_PLL_PLL_RESETS_POST_RESETB               (1 << 1)
#define G3_PLL_PLL_LOOP_FREQ_REG                   (PCIE_G3_PLL_REGS+0x0004)
#define G3_PLL_PLL_LOOP_FREQ_NDIV_INT_MASK          (0x3FF)
#define G3_PLL_PLL_NDIV_REG                        (PCIE_G3_PLL_REGS+0x000c)
#define G3_PLL_PLL_NDIV_INT_MASK                    (0x3FF)
#define G3_PLL_PLL_NDIV_INT(d)                      \
	((d) & G3_PLL_PLL_NDIV_INT_MASK)
#define G3_PLL_PLL_STAT_REG                        (PCIE_G3_PLL_REGS+0x002c)
#define G3_PLL_PLL_STAT_PLL_LOCK                    (1 << 31)
#define G3_16FF_PLL_PLL_SSC_LIMIT_SSC_MODE_REG     (PCIE_G3_PLL_REGS+0x0030)
#define G3_PLL_PLL_SSC_LIMIT_SSC_MODE_DOWNSPREAD    (1 << 25)
#define G3_PLL_PLL_SSC_STEP_REG                    (PCIE_G3_PLL_REGS+0x0034)
#define G3_PLL_PLL_SSC_STEP_PLL_SSC_STEP_MASK       (0xFFFF)
#define G3_PLL_PLL_NDIV_FRAC_HOLDALL_REG           (PCIE_G3_PLL_REGS+0x004c)
#define G3_PLL_PLL_NDIV_FRAC_HOLDALL_HOLA_ALL       (1 << 24)
#define G3_PLL_PLL_NDIV_FRAC_HOLDALL_FRAC_MASK      (0xFFFFFF)
#define G3_PLL_PLL_NDIV_FRAC_HOLDALL_NDIV_FRAC(f)   ((f) & 0xFFFFFF)
#define G3_PLL_PLL_SSC_LIMIT_SSC_MODE_REG          (PCIE_G3_PLL_REGS+0x0050)
#define G3_PLL_PLL_SSC_LIMIT_SSC_MODE_MODE          (1 << 24)
#define G3_PLL_PLL_SSC_LIMIT_SSC_MODE_LIMIT_MASK    (0x3FFFFF)
#define G3_PLL_PLL_SSC_LIMIT_SSC_MODE_LIMIT(l)      ((l) & 0x3FFFFF)
#define G3_PLL_PLL_SSC_STEP_VCOGAIN_REG            (PCIE_G3_PLL_REGS+0x0054)
#define G3_PLL_PLL_SSC_STEP_VCOGAIN_VCO_GAIN_MASK   (0x000F0000)
#define G3_PLL_PLL_SSC_STEP_VCOGAIN_VCO_GAIN(g)     (((g) & 0xF) << 16)
#define G3_PLL_PLL_SSC_STEP_VCOGAIN_SSC_STEP_MASK   (0xFFFF)
#define G3_PLL_PLL_SSC_STEP_VCOGAIN_SSC_STEP(s)     ((s) & 0xFFFF)

/*
 * +-----------------------------------------------------
 * SERDES: GEN2 Registers
 * +-----------------------------------------------------
 */
#define SERDES_PCIE_BLK0_REGS                      (0x1000)
#define SERDES_PCIE_BLK_SIZE                        (0x0100)
#define SERDES_PCIE_BLK1_REGS                      (0x1100)
#define SERDES_PCIE_BLK2_REGS                      (0x1200)
#define SERDES_PCIE_BLK3_REGS                      (0x1300)
#define SERDES_PCIE_BLK4_REGS                      (0x1400)
#define SERDES_PCIE_BLK5_REGS                      (0x1500)
#define SERDES_PCIE_BLK6_REGS                      (0x1600) /* 16ff viper */
#define SERDES_PLL_AFE0_REGS                       (0x2000)
#define SERDES_PLL_AFE_SIZE                         (0x0100)
#define SERDES_PLL_AFE1_REGS                       (0x2100)
#define SERDES_PLL_AFE2_REGS                       (0x2200)
#define SERDES_PLL_AFE3_REGS                       (0x2300)
#define SERDES_PLL_STABLE_TMR_CTRL_REGS            (0x2400)
#define SERDES_PLL_PLL_CTRL_REGS                   (0x2500)
#define SERDES_PLL_SSC_CTRL_REGS                   (0x2600)
#define SERDES_TX_AFE_BLOCK_REGS                   (0x4000)
#define SERDES_TX_CTR1_LN0_REG                      (SERDES_TX_AFE_BLOCK_REGS)
#define SERDES_TX_CTR1_LN_SIZE                      0x0010
#define SERDES_TX_DFE_BLOCK_REGS                   (0x5000)
#define SERDES_TX_DFE0_LN0_REG                      (SERDES_TX_DFE_BLOCK_REGS)
#define SERDES_TX_DFE0_LN_SIZE                      0x0010
#define SERDES_RX_AFE0_REGS                        0x6000
#define SERDES_RX_DFE0_REGS                        0x7000
#define SERDES_RX_DFE1_REGS                        0x7100
#define SERDES_RX_DFE2_REGS                        0x7200
#define SERDES_RX_DFE3_REGS                        0x7300
#define SERDES_LN_REG                              0x0010
#define SERDES_TXPMD_B_REGS                        0xD210
#define SERDES_RX_AFE0_BC_REGS                     0xF600
#define SERDES_RX_DFE0_BC_REGS                     0xF700
#define SERDES_RX_DFE0_BC_CTRL2_FILTBYPASS          0x0800
#define SERDES_RX_DFE1_BC_REGS                     0xF710
#define SERDES_RX_DFE2_BC_REGS                     0xF720
#define SERDES_RX_DFE3_BC_REGS                     0xF730
#define NUM_CORE                                   MAX_NUM_PHY_PCIE_CORES

/*
 * +-----------------------------------------------------
 *  Structures
 * +-----------------------------------------------------
 */

#ifdef __cplusplus
}
#endif

#endif /* __PCIE_BCM963XX_H */
