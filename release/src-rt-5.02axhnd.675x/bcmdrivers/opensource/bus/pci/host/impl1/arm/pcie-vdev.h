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
#ifndef __PCIE_VDEV_H
#define __PCIE_VDEV_H

#ifdef __cplusplus
extern "C" {
#endif


/*
 * +-----------------------------------------------------
 *
 *  Defines
 *
 * +-----------------------------------------------------
 */

#define NUM_BAR_REGS                     6

#define PCI_CFG_REGS_SIZE                0x100
#define PCI_CFG_REGS_SIZEW               (PCI_CFG_REGS_SIZE/4)

#define VDEV_CFG_REGS_SIZE               0x20
#define VDEV_CFG_REGS_SIZEW              (VDEV_CFG_REGS_SIZE/4)

#define PCIE_VDEV_BAR_1_SIZE             0x100
#define PCIE_VDEV_BAR_2_SIZE             0x104
#define PCIE_VDEV_BAR_3_SIZE             0x108
#define PCIE_VDEV_BAR_4_SIZE             0x10C
#define PCIE_VDEV_BAR_5_SIZE             0x110
#define PCIE_VDEV_BAR_6_SIZE             0x114
#define PCIE_VDEV_BUS_NUM                0x118
#define PCIE_VDEV_SLOT_NUM               0x119

#define PCIE_VDEV_CFG_SIZE               (PCI_CFG_REGS_SIZE + VDEV_CFG_REGS_SIZE)
#define PCIE_VDEV_CFG_SIZEW              (PCIE_VDEV_CFG_SIZE/4)

#define PCIE_VDEV_CFG_LIST_SIZE          \
	(sizeof(pcie_vdev_cfg_list)/sizeof(struct pcie_vdev_cfg_info))

#define NUM_PCIE_VDEV_DEF_CFG            1
#define NUM_PCIE_VDEV_CFG                (PCIE_VDEV_CFG_LIST_SIZE - NUM_PCIE_VDEV_DEF_CFG)

/*
 * +-----------------------------------------------------
 *
 *  Macros
 *
 * +-----------------------------------------------------
 */

/*
 * 00: device_vendor_id    status_command          rev_id_class_code headertype_lat_cachelinesize
 * 10: bar_1               bar_2                   bar_3             bar_4
 * 20: bar_5               bar_6                   cardbus_cis       subsystem_id_vendor_id
 * 30: exp_rom_bar         cap_pointer             reserved          lat_min_grant_int_pin_int_line
 * 40: Reserved            reserved                pm_cap            pm_csr
 * 50: Reserved            reserved                msi_cap           msi_addr_l
 * 60: msi_addr_h          msi_data                vendor-specific   unknown
 * 70: unknown             unknown                 unknown           unknown
 * 80: unknown             unknown                 unknown           unknown
 * 90: unknown             unknown                 unknown           unknown
 * a0: msi_cap             msix_tbl_off_bir        msix_pba_bir_off  pcie_capability
 * b0: device_capability   device_status_control   link_capability   link_status_control
 * c0: slot_capability     slot_control_status     root_cap_control  root_status
 * d0: device_capability_2 device_status_control_2 link_capability_2 link_status_control_2
 * e0: reserved            reserved                reserved          reserved
 * f0: reserved            reserved                reserved          reserved
 * 100: bar_size_1          bar_size_2              bar_size_3        bar_size_4
 * 110: bar_size_5          bar_size_6              dev_bus           reserved
 */
#define PCIE_VDEV_FILL_CFG_REG(DeviceVend, ClassRevId, BAR_1_Addr, IntLinePin)              \
	DeviceVend, 0x00100000, ClassRevId, 0x00000000, /* 00 */                            \
	BAR_1_Addr, 0x00000000, 0x00000004, 0x00000000, /* 10 */                            \
	0x0000000c, 0x00000000, 0x00000000, DeviceVend, /* 20 */                            \
	0x00000000, 0x00000048, 0x00000000, IntLinePin, /* 30 */                            \
	0x00000000, 0x00000000, 0x06035801, 0x00004108, /* 40 */                            \
	0x00000000, 0x00000000, 0x008b6805, 0x00000000, /* 50 */                            \
	0x00000000, 0x00000000, 0x0000a009, 0x00000000, /* 60 */                            \
	0x28100000, 0x00000000, 0x00000000, 0x00000000, /* 70 */                            \
	0x28000000, 0x00000000, 0x00000080, 0x00000000, /* 80 */                            \
	0x00000000, 0x00000100, 0x00000000, 0x00000000, /* 90 */                            \
	0x003fac11, 0x00008000, 0x00008800, 0x00020010, /* a0 */                            \
	0x00008f82, 0x00102c10, 0x0046dc22, 0x10220000, /* b0 */                            \
	0x00000000, 0x00000000, 0x00000000, 0x00000000, /* c0 */                            \
	0x0008081f, 0x00000000, 0x00000006, 0x00000002, /* d0 */                            \
	0x00000000, 0x00000000, 0x00000000, 0x00000000, /* e0 */                            \
	0x00000000, 0x00000000, 0x00000000, 0x00000000  /* f0 */

#define PCIE_VDEV_FILL_CFG_EXT(BAR_1_Size, ModSlotBus)                                      \
	BAR_1_Size, 0x00000000, 0x00000000, 0x00000000, /* 100 */                           \
	0x00000000, 0x00000000, ModSlotBus, 0x00000000  /* 110 */

#define PCIE_VDEV_FILL_CFG(DeviceVend, ClassRevId, BAR_1_Addr, IntLinePin,                  \
	                       BAR_1_Size, ModSlotBus)                                      \
	PCIE_VDEV_FILL_CFG_REG(DeviceVend, ClassRevId, BAR_1_Addr, IntLinePin),             \
	PCIE_VDEV_FILL_CFG_EXT(BAR_1_Size, ModSlotBus)

#define PCIE_VDEV_INIT_CFG(dev, rev, bar_1, int_line, bar_1_size, bus, slot)                \
	PCIE_VDEV_FILL_CFG_REG((((dev) << 16) | 0x14e4), (0x02800000|rev),                  \
	                       ((bar_1 | 0x04)), (0x100 | (int_line))),                     \
	PCIE_VDEV_FILL_CFG_EXT((bar_1_size), (((slot) << 8) | (bus)))

#define PCIE_VDEV_LIST_ELEM(chip, rev, elem)                                                \
	{.chip_id = chip, .rev_id = rev,                                                    \
	 .size = sizeof(elem)/PCIE_VDEV_CFG_SIZE, .pcfg = (struct pcie_vdev_cfg*)(&elem)}


/*
 * +-----------------------------------------------------
 *
 *  Structures
 *
 * +-----------------------------------------------------
 */
/* Note: Structure assumes Little Endian platform with sizeof(uint32) = 4 */
/* 48 (pm) -> 58 (msi) -> 68 (vendor) -> a0 (msix) -> ac (pcie) */
struct pcie_cfg_space_regs {
	/* 00 */    uint32 device_vendor_id;
	/* 04 */    uint32 status_command;
	/* 08 */    uint32 rev_id_class_code;
	            union {
	/* 0c */    uint32 headertype_lat_cachelinesize;
	                struct {
	                    uint8 cache_line_size;
	                    uint8 lat_timer;
	                    uint8 header_type;
	                    uint8 bist;
	                } cfg0c;
	            };
	/* 10 */    uint32 bar_1;
	/* 14 */    uint32 bar_2;
	/* 18 */    uint32 bar_3;
	/* 1c */    uint32 bar_4;
	/* 20 */    uint32 bar_5;
	/* 24 */    uint32 bar_6;
	/* 28 */    uint32 cardbus_cis;
	/* 2c */    uint32 subsystem_id_vendor_id;
	/* 30 */    uint32 exp_rom_bar;
	/* 34 */    uint32 cap_pointer; /* -> 0x48 */
	/* 38 */    uint32 reserved1;
	            union {
	/* 3c */        uint32 lat_min_grant_int_pin_int_line;
	                struct {
	                    uint8 int_line;
	                    uint8 int_pin;
	                    uint8 min_grant;
	                    uint8 max_lat;
	                } cfg3c;
	            };
	/* 40 */    uint32 reserved2[2];
	/* 48 */    uint32 pm_cap;     /* -> 0x58 */
	/* 4c */    uint32 pm_csr;
	/* 50-54 */ uint32 reserved3[2];
	/* 58 */    uint32 msi_cap;     /* -> 0x68 */
	/* 5c */    uint32 msi_addr_l;
	/* 60 */    uint32 msi_addr_h;
	/* 64 */    uint32 msi_data;
	/* 68-9c */ uint32 vendor_specific[14]; /* -> 0xa0 */
	/* a0 */    uint32 msix_cap;            /* -> 0xac */
	/* a4 */    uint32 msix_tbl_off_bir;
	/* a8 */    uint32 msix_pba_bir_off;
	/* ac */    uint32 pcie_capability;
	/* b0 */    uint32 device_capability;
	/* b4 */    uint32 device_status_control;
	/* b8 */    uint32 link_capability;
	/* bc */    uint32 link_status_control;
	/* c0 */    uint32 slot_capability;
	/* c4 */    uint32 slot_control_status;
	/* c8 */    uint32 root_cap_control;
	/* cc */    uint32 root_status;
	/* d0 */    uint32 device_capability_2;
	/* d4 */    uint32 device_status_control_2;
	/* d8 */    uint32 link_capability_2;
	/* dc */    uint32 link_status_control_2;
	/* e0-fc */ uint32 reserved4[8];
};

/*
 * PCIe Vitual Device configuration
 *
 * @regs    : Config space registers
 * @bar_size: bar memory size
 * @bus     : bus number this device to be enumerated as
 * @slot    : slot number this device to be enuerated as
 *          : <domain>:<bus>:<slot>.0
 * @rsvd    : padding to make it 32 byte aligned
 * @rsvd1   : padding to make it 32 byte aligned
 */
struct pcie_vdev_cfg {
	struct pcie_cfg_space_regs regs;
	uint32 bar_size[NUM_BAR_REGS];
	uint8  bus;
	uint8  slot;
	uint16 rsvd;
	uint32 rsvd1;
};

/*
 * PCIe Vitual Device configuration information
 *
 * @chip_id : BCA Chip Identifier
 * @rev_id  : BCA Chip Revision
 * @size    : Number of configuraitons per core
 * @pcfg    : pointer to virtual device configuration
 */
struct pcie_vdev_cfg_info {
	uint32               chip_id;
	uint32               rev_id;
	uint32               size;
	struct pcie_vdev_cfg *pcfg;
};

/*
 * +-----------------------------------------------------
 *
 *  Global variables
 *
 * +-----------------------------------------------------
 */

/*
 * Default virtual device configuration
 *
 * If chip specific configuration is not available, this will be used
 * - device 0xf6ca, bar_1: 0x0, size: 0x0, bus: 1, slot: 0
 */
const uint32 pcie_wifidefdev_cfg[PCIE_VDEV_CFG_SIZEW] = {
	/* 1 -  WIFI device, 1 - Virtual Core */
	/* 0:1:0.0 */ PCIE_VDEV_INIT_CFG(0xf6ca, 0x2, 0x00000000, 0x00, 0x00000000, 1, 0)
};

/*
 * Chip specific WIFI device configurations
 */

#if defined(CONFIG_BCM963178)
const uint32 pcie_wifi0dev_cfg[PCIE_VDEV_CFG_SIZEW] = {
	/* 1 -  WIFI device, 1 - Virtual Core */
	/* 0:1:0.0 */ PCIE_VDEV_INIT_CFG(0xf6ca, 0x2, WLAN0_PHYS_BASE, 0, WLAN0_SIZE, 1, 0)
};
#endif /* CONFIG_BCM963178 */

#if defined(CONFIG_BCM947622)
const uint32 pcie_wifi01dev_cfg[2*PCIE_VDEV_CFG_SIZEW] = {
	/* 2 -  WIFI devices, 1 - Virtual Core */
	/* 0:1:0.0 */ PCIE_VDEV_INIT_CFG(0xf6ca, 0x2, WLAN0_PHYS_BASE, 0, WLAN0_SIZE, 1, 0),
	/* 0:1:1.0 */ PCIE_VDEV_INIT_CFG(0xf6ca, 0x2, WLAN1_PHYS_BASE, 0, WLAN1_SIZE, 1, 1)
};
#endif /* CONFIG_BCM947622 */

#if defined(CONFIG_BCM96878)
const uint32 pcie_wifi_6878_dev_cfg[PCIE_VDEV_CFG_SIZEW] = {
	/* 1 -  WIFI device, 1 - Virtual Core */
	/* 0:1:0.0 */ PCIE_VDEV_INIT_CFG(0x6878, 0x2, WLAN0_PHYS_BASE, 0, WLAN0_SIZE, 1, 0)
};
#endif /* CONFIG_BCM96878 */

/*
 * List of available wifi device configurations
 *
 * - Add SoC Specific virtual device to the list based on the number of cores
 * - Last one is default that can be applied to any SoC
 */
struct pcie_vdev_cfg_info pcie_vdev_cfg_list[] = {
#if defined(CONFIG_BCM963178)
	PCIE_VDEV_LIST_ELEM(0x63178, 0xA0, pcie_wifi0dev_cfg),
#endif /* CONFIG_BCM963178 */
#if defined(CONFIG_BCM947622)
	PCIE_VDEV_LIST_ELEM(0x47622, 0xA0, pcie_wifi01dev_cfg),
#endif /* CONFIG_BCM947622 */
#if defined(CONFIG_BCM96878)
	PCIE_VDEV_LIST_ELEM(0x6878, 0xA0, pcie_wifi_6878_dev_cfg),
#endif /* CONFIG_BCM96878 */
	PCIE_VDEV_LIST_ELEM(0x0, 0x0, pcie_wifidefdev_cfg) /* Delimiter, default */
};

#ifdef __cplusplus
}
#endif
#endif /* __PCIE_VDEV_H */
