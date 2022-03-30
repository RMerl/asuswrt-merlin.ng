/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Based on acpi.c from coreboot
 *
 * Copyright (C) 2015, Saket Sinha <saket.sinha89@gmail.com>
 * Copyright (C) 2016, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef __ASM_ACPI_TABLE_H__
#define __ASM_ACPI_TABLE_H__

#define RSDP_SIG		"RSD PTR "	/* RSDP pointer signature */
#define OEM_ID			"U-BOOT"	/* U-Boot */
#define OEM_TABLE_ID		"U-BOOTBL"	/* U-Boot Table */
#define ASLC_ID			"INTL"		/* Intel ASL Compiler */

#define ACPI_RSDP_REV_ACPI_1_0	0
#define ACPI_RSDP_REV_ACPI_2_0	2

/*
 * RSDP (Root System Description Pointer)
 * Note: ACPI 1.0 didn't have length, xsdt_address, and ext_checksum
 */
struct acpi_rsdp {
	char signature[8];	/* RSDP signature */
	u8 checksum;		/* Checksum of the first 20 bytes */
	char oem_id[6];		/* OEM ID */
	u8 revision;		/* 0 for ACPI 1.0, others 2 */
	u32 rsdt_address;	/* Physical address of RSDT (32 bits) */
	u32 length;		/* Total RSDP length (incl. extended part) */
	u64 xsdt_address;	/* Physical address of XSDT (64 bits) */
	u8 ext_checksum;	/* Checksum of the whole table */
	u8 reserved[3];
};

/* Generic ACPI header, provided by (almost) all tables */
struct acpi_table_header {
	char signature[4];	/* ACPI signature (4 ASCII characters) */
	u32 length;		/* Table length in bytes (incl. header) */
	u8 revision;		/* Table version (not ACPI version!) */
	volatile u8 checksum;	/* To make sum of entire table == 0 */
	char oem_id[6];		/* OEM identification */
	char oem_table_id[8];	/* OEM table identification */
	u32 oem_revision;	/* OEM revision number */
	char aslc_id[4];	/* ASL compiler vendor ID */
	u32 aslc_revision;	/* ASL compiler revision number */
};

/* A maximum number of 32 ACPI tables ought to be enough for now */
#define MAX_ACPI_TABLES		32

/* RSDT (Root System Description Table) */
struct acpi_rsdt {
	struct acpi_table_header header;
	u32 entry[MAX_ACPI_TABLES];
};

/* XSDT (Extended System Description Table) */
struct acpi_xsdt {
	struct acpi_table_header header;
	u64 entry[MAX_ACPI_TABLES];
};

/* FADT Preferred Power Management Profile */
enum acpi_pm_profile {
	ACPI_PM_UNSPECIFIED = 0,
	ACPI_PM_DESKTOP,
	ACPI_PM_MOBILE,
	ACPI_PM_WORKSTATION,
	ACPI_PM_ENTERPRISE_SERVER,
	ACPI_PM_SOHO_SERVER,
	ACPI_PM_APPLIANCE_PC,
	ACPI_PM_PERFORMANCE_SERVER,
	ACPI_PM_TABLET
};

/* FADT flags for p_lvl2_lat and p_lvl3_lat */
#define ACPI_FADT_C2_NOT_SUPPORTED	101
#define ACPI_FADT_C3_NOT_SUPPORTED	1001

/* FADT Boot Architecture Flags */
#define ACPI_FADT_LEGACY_FREE		0x00
#define ACPI_FADT_LEGACY_DEVICES	(1 << 0)
#define ACPI_FADT_8042			(1 << 1)
#define ACPI_FADT_VGA_NOT_PRESENT	(1 << 2)
#define ACPI_FADT_MSI_NOT_SUPPORTED	(1 << 3)
#define ACPI_FADT_NO_PCIE_ASPM_CONTROL	(1 << 4)

/* FADT Feature Flags */
#define ACPI_FADT_WBINVD		(1 << 0)
#define ACPI_FADT_WBINVD_FLUSH		(1 << 1)
#define ACPI_FADT_C1_SUPPORTED		(1 << 2)
#define ACPI_FADT_C2_MP_SUPPORTED	(1 << 3)
#define ACPI_FADT_POWER_BUTTON		(1 << 4)
#define ACPI_FADT_SLEEP_BUTTON		(1 << 5)
#define ACPI_FADT_FIXED_RTC		(1 << 6)
#define ACPI_FADT_S4_RTC_WAKE		(1 << 7)
#define ACPI_FADT_32BIT_TIMER		(1 << 8)
#define ACPI_FADT_DOCKING_SUPPORTED	(1 << 9)
#define ACPI_FADT_RESET_REGISTER	(1 << 10)
#define ACPI_FADT_SEALED_CASE		(1 << 11)
#define ACPI_FADT_HEADLESS		(1 << 12)
#define ACPI_FADT_SLEEP_TYPE		(1 << 13)
#define ACPI_FADT_PCI_EXPRESS_WAKE	(1 << 14)
#define ACPI_FADT_PLATFORM_CLOCK	(1 << 15)
#define ACPI_FADT_S4_RTC_VALID		(1 << 16)
#define ACPI_FADT_REMOTE_POWER_ON	(1 << 17)
#define ACPI_FADT_APIC_CLUSTER		(1 << 18)
#define ACPI_FADT_APIC_PHYSICAL		(1 << 19)
#define ACPI_FADT_HW_REDUCED_ACPI	(1 << 20)
#define ACPI_FADT_LOW_PWR_IDLE_S0	(1 << 21)

enum acpi_address_space_type {
	ACPI_ADDRESS_SPACE_MEMORY = 0,	/* System memory */
	ACPI_ADDRESS_SPACE_IO,		/* System I/O */
	ACPI_ADDRESS_SPACE_PCI,		/* PCI config space */
	ACPI_ADDRESS_SPACE_EC,		/* Embedded controller */
	ACPI_ADDRESS_SPACE_SMBUS,	/* SMBus */
	ACPI_ADDRESS_SPACE_PCC = 0x0a,	/* Platform Comm. Channel */
	ACPI_ADDRESS_SPACE_FIXED = 0x7f	/* Functional fixed hardware */
};

enum acpi_address_space_size {
	ACPI_ACCESS_SIZE_UNDEFINED = 0,
	ACPI_ACCESS_SIZE_BYTE_ACCESS,
	ACPI_ACCESS_SIZE_WORD_ACCESS,
	ACPI_ACCESS_SIZE_DWORD_ACCESS,
	ACPI_ACCESS_SIZE_QWORD_ACCESS
};

struct acpi_gen_regaddr {
	u8 space_id;	/* Address space ID */
	u8 bit_width;	/* Register size in bits */
	u8 bit_offset;	/* Register bit offset */
	u8 access_size;	/* Access size */
	u32 addrl;	/* Register address, low 32 bits */
	u32 addrh;	/* Register address, high 32 bits */
};

/* FADT (Fixed ACPI Description Table) */
struct __packed acpi_fadt {
	struct acpi_table_header header;
	u32 firmware_ctrl;
	u32 dsdt;
	u8 res1;
	u8 preferred_pm_profile;
	u16 sci_int;
	u32 smi_cmd;
	u8 acpi_enable;
	u8 acpi_disable;
	u8 s4bios_req;
	u8 pstate_cnt;
	u32 pm1a_evt_blk;
	u32 pm1b_evt_blk;
	u32 pm1a_cnt_blk;
	u32 pm1b_cnt_blk;
	u32 pm2_cnt_blk;
	u32 pm_tmr_blk;
	u32 gpe0_blk;
	u32 gpe1_blk;
	u8 pm1_evt_len;
	u8 pm1_cnt_len;
	u8 pm2_cnt_len;
	u8 pm_tmr_len;
	u8 gpe0_blk_len;
	u8 gpe1_blk_len;
	u8 gpe1_base;
	u8 cst_cnt;
	u16 p_lvl2_lat;
	u16 p_lvl3_lat;
	u16 flush_size;
	u16 flush_stride;
	u8 duty_offset;
	u8 duty_width;
	u8 day_alrm;
	u8 mon_alrm;
	u8 century;
	u16 iapc_boot_arch;
	u8 res2;
	u32 flags;
	struct acpi_gen_regaddr reset_reg;
	u8 reset_value;
	u16 arm_boot_arch;
	u8 minor_revision;
	u32 x_firmware_ctl_l;
	u32 x_firmware_ctl_h;
	u32 x_dsdt_l;
	u32 x_dsdt_h;
	struct acpi_gen_regaddr x_pm1a_evt_blk;
	struct acpi_gen_regaddr x_pm1b_evt_blk;
	struct acpi_gen_regaddr x_pm1a_cnt_blk;
	struct acpi_gen_regaddr x_pm1b_cnt_blk;
	struct acpi_gen_regaddr x_pm2_cnt_blk;
	struct acpi_gen_regaddr x_pm_tmr_blk;
	struct acpi_gen_regaddr x_gpe0_blk;
	struct acpi_gen_regaddr x_gpe1_blk;
};

/* FACS flags */
#define ACPI_FACS_S4BIOS_F	(1 << 0)
#define ACPI_FACS_64BIT_WAKE_F	(1 << 1)

/* FACS (Firmware ACPI Control Structure) */
struct acpi_facs {
	char signature[4];		/* "FACS" */
	u32 length;			/* Length in bytes (>= 64) */
	u32 hardware_signature;		/* Hardware signature */
	u32 firmware_waking_vector;	/* Firmware waking vector */
	u32 global_lock;		/* Global lock */
	u32 flags;			/* FACS flags */
	u32 x_firmware_waking_vector_l;	/* X FW waking vector, low */
	u32 x_firmware_waking_vector_h;	/* X FW waking vector, high */
	u8 version;			/* Version 2 */
	u8 res1[3];
	u32 ospm_flags;			/* OSPM enabled flags */
	u8 res2[24];
};

/* MADT flags */
#define ACPI_MADT_PCAT_COMPAT	(1 << 0)

/* MADT (Multiple APIC Description Table) */
struct acpi_madt {
	struct acpi_table_header header;
	u32 lapic_addr;			/* Local APIC address */
	u32 flags;			/* Multiple APIC flags */
};

/* MADT: APIC Structure Type*/
enum acpi_apic_types {
	ACPI_APIC_LAPIC	= 0,		/* Processor local APIC */
	ACPI_APIC_IOAPIC,		/* I/O APIC */
	ACPI_APIC_IRQ_SRC_OVERRIDE,	/* Interrupt source override */
	ACPI_APIC_NMI_SRC,		/* NMI source */
	ACPI_APIC_LAPIC_NMI,		/* Local APIC NMI */
	ACPI_APIC_LAPIC_ADDR_OVERRIDE,	/* Local APIC address override */
	ACPI_APIC_IOSAPIC,		/* I/O SAPIC */
	ACPI_APIC_LSAPIC,		/* Local SAPIC */
	ACPI_APIC_PLATFORM_IRQ_SRC,	/* Platform interrupt sources */
	ACPI_APIC_LX2APIC,		/* Processor local x2APIC */
	ACPI_APIC_LX2APIC_NMI,		/* Local x2APIC NMI */
};

/* MADT: Processor Local APIC Structure */

#define LOCAL_APIC_FLAG_ENABLED	(1 << 0)

struct acpi_madt_lapic {
	u8 type;		/* Type (0) */
	u8 length;		/* Length in bytes (8) */
	u8 processor_id;	/* ACPI processor ID */
	u8 apic_id;		/* Local APIC ID */
	u32 flags;		/* Local APIC flags */
};

/* MADT: I/O APIC Structure */
struct acpi_madt_ioapic {
	u8 type;		/* Type (1) */
	u8 length;		/* Length in bytes (12) */
	u8 ioapic_id;		/* I/O APIC ID */
	u8 reserved;
	u32 ioapic_addr;	/* I/O APIC address */
	u32 gsi_base;		/* Global system interrupt base */
};

/* MADT: Interrupt Source Override Structure */
struct __packed acpi_madt_irqoverride {
	u8 type;		/* Type (2) */
	u8 length;		/* Length in bytes (10) */
	u8 bus;			/* ISA (0) */
	u8 source;		/* Bus-relative int. source (IRQ) */
	u32 gsirq;		/* Global system interrupt */
	u16 flags;		/* MPS INTI flags */
};

/* MADT: Local APIC NMI Structure */
struct __packed acpi_madt_lapic_nmi {
	u8 type;		/* Type (4) */
	u8 length;		/* Length in bytes (6) */
	u8 processor_id;	/* ACPI processor ID */
	u16 flags;		/* MPS INTI flags */
	u8 lint;		/* Local APIC LINT# */
};

/* MCFG (PCI Express MMIO config space BAR description table) */
struct acpi_mcfg {
	struct acpi_table_header header;
	u8 reserved[8];
};

struct acpi_mcfg_mmconfig {
	u32 base_address_l;
	u32 base_address_h;
	u16 pci_segment_group_number;
	u8 start_bus_number;
	u8 end_bus_number;
	u8 reserved[4];
};

/* PM1_CNT bit defines */
#define PM1_CNT_SCI_EN		(1 << 0)

/* ACPI global NVS structure */
struct acpi_global_nvs;

/* DBG2 definitions are partially used for SPCR interface_type */

/* Types for port_type field */

#define ACPI_DBG2_SERIAL_PORT		0x8000
#define ACPI_DBG2_1394_PORT		0x8001
#define ACPI_DBG2_USB_PORT		0x8002
#define ACPI_DBG2_NET_PORT		0x8003

/* Subtypes for port_subtype field */

#define ACPI_DBG2_16550_COMPATIBLE	0x0000
#define ACPI_DBG2_16550_SUBSET		0x0001
#define ACPI_DBG2_ARM_PL011		0x0003
#define ACPI_DBG2_ARM_SBSA_32BIT	0x000D
#define ACPI_DBG2_ARM_SBSA_GENERIC	0x000E
#define ACPI_DBG2_ARM_DCC		0x000F
#define ACPI_DBG2_BCM2835		0x0010

#define ACPI_DBG2_1394_STANDARD		0x0000

#define ACPI_DBG2_USB_XHCI		0x0000
#define ACPI_DBG2_USB_EHCI		0x0001

#define ACPI_DBG2_UNKNOWN		0x00FF

/* SPCR (Serial Port Console Redirection table) */
struct __packed acpi_spcr {
	struct acpi_table_header header;
	u8 interface_type;
	u8 reserved[3];
	struct acpi_gen_regaddr serial_port;
	u8 interrupt_type;
	u8 pc_interrupt;
	u32 interrupt;		/* Global system interrupt */
	u8 baud_rate;
	u8 parity;
	u8 stop_bits;
	u8 flow_control;
	u8 terminal_type;
	u8 reserved1;
	u16 pci_device_id;	/* Must be 0xffff if not PCI device */
	u16 pci_vendor_id;	/* Must be 0xffff if not PCI device */
	u8 pci_bus;
	u8 pci_device;
	u8 pci_function;
	u32 pci_flags;
	u8 pci_segment;
	u32 reserved2;
};

/* These can be used by the target port */

void acpi_fill_header(struct acpi_table_header *header, char *signature);
void acpi_create_fadt(struct acpi_fadt *fadt, struct acpi_facs *facs,
		      void *dsdt);
int acpi_create_madt_lapics(u32 current);
int acpi_create_madt_ioapic(struct acpi_madt_ioapic *ioapic, u8 id,
			    u32 addr, u32 gsi_base);
int acpi_create_madt_irqoverride(struct acpi_madt_irqoverride *irqoverride,
				 u8 bus, u8 source, u32 gsirq, u16 flags);
int acpi_create_madt_lapic_nmi(struct acpi_madt_lapic_nmi *lapic_nmi,
			       u8 cpu, u16 flags, u8 lint);
u32 acpi_fill_madt(u32 current);
int acpi_create_mcfg_mmconfig(struct acpi_mcfg_mmconfig *mmconfig, u32 base,
			      u16 seg_nr, u8 start, u8 end);
u32 acpi_fill_mcfg(u32 current);
void acpi_create_gnvs(struct acpi_global_nvs *gnvs);
ulong write_acpi_tables(ulong start);

/**
 * acpi_get_rsdp_addr() - get ACPI RSDP table address
 *
 * This routine returns the ACPI RSDP table address in the system memory.
 *
 * @return:	ACPI RSDP table address
 */
ulong acpi_get_rsdp_addr(void);

#endif /* __ASM_ACPI_TABLE_H__ */
