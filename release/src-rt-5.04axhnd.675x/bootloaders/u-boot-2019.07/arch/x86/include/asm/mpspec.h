/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 *
 * Adapted from coreboot src/arch/x86/include/arch/smp/mpspec.h
 */

#ifndef __ASM_MPSPEC_H
#define __ASM_MPSPEC_H

/*
 * Structure definitions for SMP machines following the
 * Intel MultiProcessor Specification 1.4
 */

#define MPSPEC_V14	4

#define MPF_SIGNATURE	"_MP_"

struct mp_floating_table {
	char mpf_signature[4];	/* "_MP_" */
	u32 mpf_physptr;	/* Configuration table address */
	u8 mpf_length;		/* Our length (paragraphs) */
	u8 mpf_spec;		/* Specification version */
	u8 mpf_checksum;	/* Checksum (makes sum 0) */
	u8 mpf_feature1;	/* Predefined or Unique configuration? */
	u8 mpf_feature2;	/* Bit7 set for IMCR/PIC */
	u8 mpf_feature3;	/* Unused (0) */
	u8 mpf_feature4;	/* Unused (0) */
	u8 mpf_feature5;	/* Unused (0) */
};

#define MPC_SIGNATURE	"PCMP"

struct mp_config_table {
	char mpc_signature[4];	/* "PCMP" */
	u16 mpc_length;		/* Size of table */
	u8 mpc_spec;		/* Specification version */
	u8 mpc_checksum;	/* Checksum (makes sum 0) */
	char mpc_oem[8];	/* OEM ID */
	char mpc_product[12];	/* Product ID */
	u32 mpc_oemptr;		/* OEM table address */
	u16 mpc_oemsize;	/* OEM table size */
	u16 mpc_entry_count;	/* Number of entries in the table */
	u32 mpc_lapic;		/* Local APIC address */
	u16 mpe_length;		/* Extended table size */
	u8 mpe_checksum;	/* Extended table checksum */
	u8 reserved;
};

/* Base MP configuration table entry types */

enum mp_base_config_entry_type {
	MP_PROCESSOR,
	MP_BUS,
	MP_IOAPIC,
	MP_INTSRC,
	MP_LINTSRC
};

#define MPC_CPU_EN	(1 << 0)
#define MPC_CPU_BP	(1 << 1)

struct mpc_config_processor {
	u8 mpc_type;
	u8 mpc_apicid;
	u8 mpc_apicver;
	u8 mpc_cpuflag;
	u32 mpc_cpusignature;
	u32 mpc_cpufeature;
	u32 mpc_reserved[2];
};

#define BUSTYPE_CBUS	"CBUS  "
#define BUSTYPE_CBUSII	"CBUSII"
#define BUSTYPE_EISA	"EISA  "
#define BUSTYPE_FUTURE	"FUTURE"
#define BUSTYPE_INTERN	"INTERN"
#define BUSTYPE_ISA	"ISA   "
#define BUSTYPE_MBI	"MBI   "
#define BUSTYPE_MBII	"MBII  "
#define BUSTYPE_MCA	"MCA   "
#define BUSTYPE_MPI	"MPI   "
#define BUSTYPE_MPSA	"MPSA  "
#define BUSTYPE_NUBUS	"NUBUS "
#define BUSTYPE_PCI	"PCI   "
#define BUSTYPE_PCMCIA	"PCMCIA"
#define BUSTYPE_TC	"TC    "
#define BUSTYPE_VL	"VL    "
#define BUSTYPE_VME	"VME   "
#define BUSTYPE_XPRESS	"XPRESS"

struct mpc_config_bus {
	u8 mpc_type;
	u8 mpc_busid;
	u8 mpc_bustype[6];
};

#define MPC_APIC_USABLE	(1 << 0)

struct mpc_config_ioapic {
	u8 mpc_type;
	u8 mpc_apicid;
	u8 mpc_apicver;
	u8 mpc_flags;
	u32 mpc_apicaddr;
};

enum mp_irq_source_types {
	MP_INT,
	MP_NMI,
	MP_SMI,
	MP_EXTINT
};

#define MP_IRQ_POLARITY_DEFAULT	0x0
#define MP_IRQ_POLARITY_HIGH	0x1
#define MP_IRQ_POLARITY_LOW	0x3
#define MP_IRQ_POLARITY_MASK	0x3
#define MP_IRQ_TRIGGER_DEFAULT	0x0
#define MP_IRQ_TRIGGER_EDGE	0x4
#define MP_IRQ_TRIGGER_LEVEL	0xc
#define MP_IRQ_TRIGGER_MASK	0xc

#define MP_APIC_ALL		0xff

struct mpc_config_intsrc {
	u8 mpc_type;
	u8 mpc_irqtype;
	u16 mpc_irqflag;
	u8 mpc_srcbus;
	u8 mpc_srcbusirq;
	u8 mpc_dstapic;
	u8 mpc_dstirq;
};

struct mpc_config_lintsrc {
	u8 mpc_type;
	u8 mpc_irqtype;
	u16 mpc_irqflag;
	u8 mpc_srcbusid;
	u8 mpc_srcbusirq;
	u8 mpc_destapic;
	u8 mpc_destlint;
};

/* Extended MP configuration table entry types */

enum mp_ext_config_entry_type {
	MPE_SYSTEM_ADDRESS_SPACE = 128,
	MPE_BUS_HIERARCHY,
	MPE_COMPAT_ADDRESS_SPACE
};

struct mp_ext_config {
	u8 mpe_type;
	u8 mpe_length;
};

#define ADDRESS_TYPE_IO		0
#define ADDRESS_TYPE_MEM	1
#define ADDRESS_TYPE_PREFETCH	2

struct mp_ext_system_address_space {
	u8 mpe_type;
	u8 mpe_length;
	u8 mpe_busid;
	u8 mpe_addr_type;
	u32 mpe_addr_base_low;
	u32 mpe_addr_base_high;
	u32 mpe_addr_length_low;
	u32 mpe_addr_length_high;
};

#define BUS_SUBTRACTIVE_DECODE	(1 << 0)

struct mp_ext_bus_hierarchy {
	u8 mpe_type;
	u8 mpe_length;
	u8 mpe_busid;
	u8 mpe_bus_info;
	u8 mpe_parent_busid;
	u8 reserved[3];
};

#define ADDRESS_RANGE_ADD	0
#define ADDRESS_RANGE_SUBTRACT	1

/*
 * X100 - X3FF
 * X500 - X7FF
 * X900 - XBFF
 * XD00 - XFFF
 */
#define RANGE_LIST_IO_ISA	0
/*
 * X3B0 - X3BB
 * X3C0 - X3DF
 * X7B0 - X7BB
 * X7C0 - X7DF
 * XBB0 - XBBB
 * XBC0 - XBDF
 * XFB0 - XFBB
 * XFC0 - XCDF
 */
#define RANGE_LIST_IO_VGA	1

struct mp_ext_compat_address_space {
	u8 mpe_type;
	u8 mpe_length;
	u8 mpe_busid;
	u8 mpe_addr_modifier;
	u32 mpe_range_list;
};

/**
 * mp_next_mpc_entry() - Compute MP configuration table end to be used as
 *                       next base table entry start address
 *
 * This computes the end address of current MP configuration table, without
 * counting any extended configuration table entry.
 *
 * @mc:		configuration table header address
 * @return:	configuration table end address
 */
static inline ulong mp_next_mpc_entry(struct mp_config_table *mc)
{
	return (ulong)mc + mc->mpc_length;
}

/**
 * mp_add_mpc_entry() - Add a base MP configuration table entry
 *
 * This adds the base MP configuration table entry size with
 * added base table entry length and increases entry count by 1.
 *
 * @mc:		configuration table header address
 * @length:	length of the added table entry
 */
static inline void mp_add_mpc_entry(struct mp_config_table *mc, uint length)
{
	mc->mpc_length += length;
	mc->mpc_entry_count++;
}

/**
 * mp_next_mpe_entry() - Compute MP configuration table end to be used as
 *                       next extended table entry start address
 *
 * This computes the end address of current MP configuration table,
 * including any extended configuration table entry.
 *
 * @mc:		configuration table header address
 * @return:	configuration table end address
 */
static inline ulong mp_next_mpe_entry(struct mp_config_table *mc)
{
	return (ulong)mc + mc->mpc_length + mc->mpe_length;
}

/**
 * mp_add_mpe_entry() - Add an extended MP configuration table entry
 *
 * This adds the extended MP configuration table entry size with
 * added extended table entry length.
 *
 * @mc:		configuration table header address
 * @mpe:	extended table entry base address
 */
static inline void mp_add_mpe_entry(struct mp_config_table *mc,
				    struct mp_ext_config *mpe)
{
	mc->mpe_length += mpe->mpe_length;
}

/**
 * mp_write_floating_table() - Write the MP floating table
 *
 * This writes the MP floating table, and points MP configuration table
 * to its end address so that MP configuration table follows immediately
 * after the floating table.
 *
 * @mf:		MP floating table base address
 * @return:	MP configuration table header address
 */
struct mp_config_table *mp_write_floating_table(struct mp_floating_table *mf);

/**
 * mp_config_table_init() - Initialize the MP configuration table header
 *
 * This populates the MP configuration table header with valid bits.
 *
 * @mc:		MP configuration table header address
 */
void mp_config_table_init(struct mp_config_table *mc);

/**
 * mp_write_processor() - Write a processor entry
 *
 * This writes a processor entry to the configuration table.
 *
 * @mc:		MP configuration table header address
 */
void mp_write_processor(struct mp_config_table *mc);

/**
 * mp_write_bus() - Write a bus entry
 *
 * This writes a bus entry to the configuration table.
 *
 * @mc:		MP configuration table header address
 * @id:		bus id
 * @bustype:	bus type name
 */
void mp_write_bus(struct mp_config_table *mc, int id, const char *bustype);

/**
 * mp_write_ioapic() - Write an I/O APIC entry
 *
 * This writes an I/O APIC entry to the configuration table.
 *
 * @mc:		MP configuration table header address
 * @id:		I/O APIC id
 * @ver:	I/O APIC version
 * @apicaddr:	I/O APIC address
 */
void mp_write_ioapic(struct mp_config_table *mc, int id, int ver, u32 apicaddr);

/**
 * mp_write_intsrc() - Write an I/O interrupt assignment entry
 *
 * This writes an I/O interrupt assignment entry to the configuration table.
 *
 * @mc:		MP configuration table header address
 * @irqtype:	IRQ type (INT/NMI/SMI/ExtINT)
 * @irqflag:	IRQ flag (level/trigger)
 * @srcbus:	source bus id where the interrupt comes from
 * @srcbusirq:	IRQ number mapped on the source bus
 * @dstapic:	destination I/O APIC id where the interrupt goes to
 * @dstirq:	destination I/O APIC pin where the interrupt goes to
 */
void mp_write_intsrc(struct mp_config_table *mc, int irqtype, int irqflag,
		     int srcbus, int srcbusirq, int dstapic, int dstirq);

/**
 * mp_write_pci_intsrc() - Write a PCI interrupt assignment entry
 *
 * This writes a PCI interrupt assignment entry to the configuration table.
 *
 * @mc:		MP configuration table header address
 * @irqtype:	IRQ type (INT/NMI/SMI/ExtINT)
 * @srcbus:	PCI bus number where the interrupt comes from
 * @dev:	device number on the PCI bus
 * @pin:	PCI interrupt pin (INT A/B/C/D)
 * @dstapic:	destination I/O APIC id where the interrupt goes to
 * @dstirq:	destination I/O APIC pin where the interrupt goes to
 */
void mp_write_pci_intsrc(struct mp_config_table *mc, int irqtype,
			 int srcbus, int dev, int pin, int dstapic, int dstirq);

/**
 * mp_write_lintsrc() - Write a local interrupt assignment entry
 *
 * This writes a local interrupt assignment entry to the configuration table.
 *
 * @mc:		MP configuration table header address
 * @irqtype:	IRQ type (INT/NMI/SMI/ExtINT)
 * @irqflag:	IRQ flag (level/trigger)
 * @srcbus:	PCI bus number where the interrupt comes from
 * @srcbusirq:	IRQ number mapped on the source bus
 * @dstapic:	destination local APIC id where the interrupt goes to
 * @destlint:	destination local APIC pin where the interrupt goes to
 */
void mp_write_lintsrc(struct mp_config_table *mc, int irqtype, int irqflag,
		      int srcbus, int srcbusirq, int destapic, int destlint);


/**
 * mp_write_address_space() - Write a system address space entry
 *
 * This writes a system address space entry to the configuration table.
 *
 * @mc:			MP configuration table header address
 * @busid:		bus id for the bus where system address space is mapped
 * @addr_type:		system address type
 * @addr_base_low:	starting address low
 * @addr_base_high:	starting address high
 * @addr_length_low:	address length low
 * @addr_length_high:	address length high
 */
void mp_write_address_space(struct mp_config_table *mc,
			    int busid, int addr_type,
			    u32 addr_base_low, u32 addr_base_high,
			    u32 addr_length_low, u32 addr_length_high);

/**
 * mp_write_bus_hierarchy() - Write a bus hierarchy descriptor entry
 *
 * This writes a bus hierarchy descriptor entry to the configuration table.
 *
 * @mc:			MP configuration table header address
 * @busid:		bus id
 * @bus_info:		bit0 indicates if the bus is a subtractive decode bus
 * @parent_busid:	parent bus id
 */
void mp_write_bus_hierarchy(struct mp_config_table *mc,
			    int busid, int bus_info, int parent_busid);

/**
 * mp_write_compat_address_space() - Write a compat bus address space entry
 *
 * This writes a compatibility bus address space modifier entry to the
 * configuration table.
 *
 * @mc:			MP configuration table header address
 * @busid:		bus id
 * @addr_modifier:	add or subtract to predefined address range list
 * @range_list:		list of predefined address space ranges
 */
void mp_write_compat_address_space(struct mp_config_table *mc, int busid,
				   int addr_modifier, u32 range_list);

/**
 * mptable_finalize() - Finalize the MP table
 *
 * This finalizes the MP table by calculating required checksums.
 *
 * @mc:		MP configuration table header address
 * @return:	MP table end address
 */
u32 mptable_finalize(struct mp_config_table *mc);

/**
 * mp_determine_pci_dstirq() - Determine PCI device's int pin on the I/O APIC
 *
 * This determines a PCI device's interrupt pin number on the I/O APIC.
 *
 * This can be implemented by platform codes to handle specifal cases, which
 * do not conform to the normal chipset/board design where PIRQ[A-H] are mapped
 * directly to I/O APIC INTPIN#16-23.
 *
 * @bus:	bus number of the pci device
 * @dev:	device number of the pci device
 * @func:	function number of the pci device
 * @pirq:	PIRQ number the PCI device's interrupt pin is routed to
 * @return:	interrupt pin number on the I/O APIC
 */
int mp_determine_pci_dstirq(int bus, int dev, int func, int pirq);

/**
 * write_mp_table() - Write MP table
 *
 * This writes MP table at a given address.
 *
 * @addr:	start address to write MP table
 * @return:	end address of MP table
 */
ulong write_mp_table(ulong addr);

#endif /* __ASM_MPSPEC_H */
