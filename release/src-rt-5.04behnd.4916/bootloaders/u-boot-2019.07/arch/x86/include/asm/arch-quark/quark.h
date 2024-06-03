/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef _QUARK_H_
#define _QUARK_H_

/* Message Bus Ports */
#define MSG_PORT_MEM_ARBITER	0x00
#define MSG_PORT_HOST_BRIDGE	0x03
#define MSG_PORT_RMU		0x04
#define MSG_PORT_MEM_MGR	0x05
#define MSG_PORT_USB_AFE	0x14
#define MSG_PORT_PCIE_AFE	0x16
#define MSG_PORT_SOC_UNIT	0x31

/* Port 0x00: Memory Arbiter Message Port Registers */

/* Enhanced Configuration Space */
#define AEC_CTRL		0x00

/* Port 0x03: Host Bridge Message Port Registers */

/* Host Miscellaneous Controls 2 */
#define HMISC2			0x03

#define HMISC2_SEGE		0x00000002
#define HMISC2_SEGF		0x00000004
#define HMISC2_SEGAB		0x00000010

/* Host Memory I/O Boundary */
#define HM_BOUND		0x08
#define HM_BOUND_LOCK		0x00000001

/* Extended Configuration Space */
#define HEC_REG			0x09

/* MTRR Registers */
#define MTRR_CAP		0x40
#define MTRR_DEF_TYPE		0x41

#define MTRR_FIX_64K_00000	0x42
#define MTRR_FIX_64K_40000	0x43
#define MTRR_FIX_16K_80000	0x44
#define MTRR_FIX_16K_90000	0x45
#define MTRR_FIX_16K_A0000	0x46
#define MTRR_FIX_16K_B0000	0x47
#define MTRR_FIX_4K_C0000	0x48
#define MTRR_FIX_4K_C4000	0x49
#define MTRR_FIX_4K_C8000	0x4a
#define MTRR_FIX_4K_CC000	0x4b
#define MTRR_FIX_4K_D0000	0x4c
#define MTRR_FIX_4K_D4000	0x4d
#define MTRR_FIX_4K_D8000	0x4e
#define MTRR_FIX_4K_DC000	0x4f
#define MTRR_FIX_4K_E0000	0x50
#define MTRR_FIX_4K_E4000	0x51
#define MTRR_FIX_4K_E8000	0x52
#define MTRR_FIX_4K_EC000	0x53
#define MTRR_FIX_4K_F0000	0x54
#define MTRR_FIX_4K_F4000	0x55
#define MTRR_FIX_4K_F8000	0x56
#define MTRR_FIX_4K_FC000	0x57

#define MTRR_SMRR_PHYBASE	0x58
#define MTRR_SMRR_PHYMASK	0x59

#define MTRR_VAR_PHYBASE(n)	(0x5a + 2 * (n))
#define MTRR_VAR_PHYMASK(n)	(0x5b + 2 * (n))

#ifndef __ASSEMBLY__

/* variable range MTRR usage */
enum {
	MTRR_VAR_ROM,
	MTRR_VAR_ESRAM,
	MTRR_VAR_RAM
};

#endif /* __ASSEMBLY__ */

/* Port 0x04: Remote Management Unit Message Port Registers */

/* ACPI PBLK Base Address Register */
#define PBLK_BA			0x70

/* Control Register */
#define RMU_CTRL		0x71

/* SPI DMA Base Address Register */
#define SPI_DMA_BA		0x7a

/* Thermal Sensor Register */
#define TS_MODE			0xb0
#define TS_TEMP			0xb1
#define TS_TRIP			0xb2

/* Port 0x05: Memory Manager Message Port Registers */

/* eSRAM Block Page Control */
#define ESRAM_BLK_CTRL		0x82
#define ESRAM_BLOCK_MODE	0x10000000

/* Port 0x14: USB2 AFE Unit Port Registers */

#define USB2_GLOBAL_PORT	0x4001
#define USB2_PLL1		0x7f02
#define USB2_PLL2		0x7f03
#define USB2_COMPBG		0x7f04

/* Port 0x16: PCIe AFE Unit Port Registers */

#define PCIE_RXPICTRL0_L0	0x2080
#define PCIE_RXPICTRL0_L1	0x2180

/* Port 0x31: SoC Unit Port Registers */

/* Thermal Sensor Config */
#define TS_CFG1			0x31
#define TS_CFG2			0x32
#define TS_CFG3			0x33
#define TS_CFG4			0x34

/* PCIe Controller Config */
#define PCIE_CFG		0x36
#define PCIE_CTLR_PRI_RST	0x00010000
#define PCIE_PHY_SB_RST		0x00020000
#define PCIE_CTLR_SB_RST	0x00040000
#define PCIE_PHY_LANE_RST	0x00090000
#define PCIE_CTLR_MAIN_RST	0x00100000

/* DRAM */
#define DRAM_BASE		0x00000000
#define DRAM_MAX_SIZE		0x80000000

/* eSRAM */
#define ESRAM_SIZE		0x80000

/* Memory BAR Enable */
#define MEM_BAR_EN		0x00000001

/* I/O BAR Enable */
#define IO_BAR_EN		0x80000000

/* 64KiB of RMU binary in flash */
#define RMU_BINARY_SIZE		0x10000

/* PCIe Root Port Configuration Registers */

#define PCIE_RP_CCFG		0xd0
#define CCFG_UPRS		(1 << 14)
#define CCFG_UNRS		(1 << 15)
#define CCFG_UNSD		(1 << 23)
#define CCFG_UPSD		(1 << 24)

#define PCIE_RP_MPC2		0xd4
#define MPC2_IPF		(1 << 11)

#define PCIE_RP_MBC		0xf4
#define MBC_SBIC		(3 << 16)

/* Legacy Bridge PCI Configuration Registers */
#define LB_GBA			0x44
#define LB_PM1BLK		0x48
#define LB_GPE0BLK		0x4c
#define LB_ACTL			0x58
#define LB_PABCDRC		0x60
#define LB_PEFGHRC		0x64
#define LB_WDTBA		0x84
#define LB_BCE			0xd4
#define LB_BC			0xd8
#define LB_RCBA			0xf0

/* USB EHCI memory-mapped registers */
#define EHCI_INSNREG01		0x94

/* USB device memory-mapped registers */
#define USBD_INT_MASK		0x410
#define USBD_EP_INT_STS		0x414
#define USBD_EP_INT_MASK	0x418

#ifndef __ASSEMBLY__

/* Root Complex Register Block */
struct quark_rcba {
	u32	rctl;
	u32	esd;
	u32	rsvd1[3150];
	u16	rmu_ir;
	u16	d23_ir;
	u16	core_ir;
	u16	d20d21_ir;
};

#include <asm/io.h>
#include <asm/pci.h>

/**
 * qrk_pci_read_config_dword() - Read a configuration value
 *
 * @dev:	PCI device address: bus, device and function
 * @offset:	Dword offset within the device's configuration space
 * @valuep:	Place to put the returned value
 *
 * Note: This routine is inlined to provide better performance on Quark
 */
static inline void qrk_pci_read_config_dword(pci_dev_t dev, int offset,
					     u32 *valuep)
{
	outl(dev | offset | PCI_CFG_EN, PCI_REG_ADDR);
	*valuep = inl(PCI_REG_DATA);
}

/**
 * qrk_pci_write_config_dword() - Write a PCI configuration value
 *
 * @dev:	PCI device address: bus, device and function
 * @offset:	Dword offset within the device's configuration space
 * @value:	Value to write
 *
 * Note: This routine is inlined to provide better performance on Quark
 */
static inline void qrk_pci_write_config_dword(pci_dev_t dev, int offset,
					      u32 value)
{
	outl(dev | offset | PCI_CFG_EN, PCI_REG_ADDR);
	outl(value, PCI_REG_DATA);
}

/**
 * board_assert_perst() - Assert the PERST# pin
 *
 * The CPU interface to the PERST# signal on Quark is platform dependent.
 * Board-specific codes need supply this routine to assert PCIe slot reset.
 *
 * The tricky part in this routine is that any APIs that may trigger PCI
 * enumeration process are strictly forbidden, as any access to PCIe root
 * port's configuration registers will cause system hang while it is held
 * in reset.
 */
void board_assert_perst(void);

/**
 * board_deassert_perst() - De-assert the PERST# pin
 *
 * The CPU interface to the PERST# signal on Quark is platform dependent.
 * Board-specific codes need supply this routine to de-assert PCIe slot reset.
 *
 * The tricky part in this routine is that any APIs that may trigger PCI
 * enumeration process are strictly forbidden, as any access to PCIe root
 * port's configuration registers will cause system hang while it is held
 * in reset.
 */
void board_deassert_perst(void);

#endif /* __ASSEMBLY__ */

#endif /* _QUARK_H_ */
