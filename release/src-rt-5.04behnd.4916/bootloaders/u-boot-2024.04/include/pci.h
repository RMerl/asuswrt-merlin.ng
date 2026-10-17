/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2001 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Andreas Heppel <aheppel@sysgo.de>
 *
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 * Copyright (c) 2021  Maciej W. Rozycki <macro@orcam.me.uk>
 */

#ifndef _PCI_H
#define _PCI_H

#define PCI_CFG_SPACE_SIZE	256
#define PCI_CFG_SPACE_EXP_SIZE	4096

/*
 * Under PCI, each device has 256 bytes of configuration address space,
 * of which the first 64 bytes are standardized as follows:
 */
#define PCI_STD_HEADER_SIZEOF	64
#define PCI_VENDOR_ID		0x00	/* 16 bits */
#define PCI_DEVICE_ID		0x02	/* 16 bits */
#define PCI_COMMAND		0x04	/* 16 bits */
#define  PCI_COMMAND_IO		0x1	/* Enable response in I/O space */
#define  PCI_COMMAND_MEMORY	0x2	/* Enable response in Memory space */
#define  PCI_COMMAND_MASTER	0x4	/* Enable bus mastering */
#define  PCI_COMMAND_SPECIAL	0x8	/* Enable response to special cycles */
#define  PCI_COMMAND_INVALIDATE 0x10	/* Use memory write and invalidate */
#define  PCI_COMMAND_VGA_PALETTE 0x20	/* Enable palette snooping */
#define  PCI_COMMAND_PARITY	0x40	/* Enable parity checking */
#define  PCI_COMMAND_WAIT	0x80	/* Enable address/data stepping */
#define  PCI_COMMAND_SERR	0x100	/* Enable SERR */
#define  PCI_COMMAND_FAST_BACK	0x200	/* Enable back-to-back writes */

#define PCI_STATUS		0x06	/* 16 bits */
#define  PCI_STATUS_CAP_LIST	0x10	/* Support Capability List */
#define  PCI_STATUS_66MHZ	0x20	/* Support 66 Mhz PCI 2.1 bus */
#define  PCI_STATUS_UDF		0x40	/* Support User Definable Features [obsolete] */
#define  PCI_STATUS_FAST_BACK	0x80	/* Accept fast-back to back */
#define  PCI_STATUS_PARITY	0x100	/* Detected parity error */
#define  PCI_STATUS_DEVSEL_MASK 0x600	/* DEVSEL timing */
#define  PCI_STATUS_DEVSEL_FAST 0x000
#define  PCI_STATUS_DEVSEL_MEDIUM 0x200
#define  PCI_STATUS_DEVSEL_SLOW 0x400
#define  PCI_STATUS_SIG_TARGET_ABORT 0x800 /* Set on target abort */
#define  PCI_STATUS_REC_TARGET_ABORT 0x1000 /* Master ack of " */
#define  PCI_STATUS_REC_MASTER_ABORT 0x2000 /* Set on master abort */
#define  PCI_STATUS_SIG_SYSTEM_ERROR 0x4000 /* Set when we drive SERR */
#define  PCI_STATUS_DETECTED_PARITY 0x8000 /* Set on parity error */

#define PCI_CLASS_REVISION	0x08	/* High 24 bits are class, low 8
					   revision */
#define PCI_REVISION_ID		0x08	/* Revision ID */
#define PCI_CLASS_PROG		0x09	/* Reg. Level Programming Interface */
#define PCI_CLASS_DEVICE	0x0a	/* Device class */
#define PCI_CLASS_CODE		0x0b	/* Device class code */
#define PCI_CLASS_SUB_CODE	0x0a	/* Device sub-class code */

#define PCI_CACHE_LINE_SIZE	0x0c	/* 8 bits */
#define PCI_LATENCY_TIMER	0x0d	/* 8 bits */
#define PCI_HEADER_TYPE		0x0e	/* 8 bits */
#define  PCI_HEADER_TYPE_NORMAL 0
#define  PCI_HEADER_TYPE_BRIDGE 1
#define  PCI_HEADER_TYPE_CARDBUS 2

#define PCI_BIST		0x0f	/* 8 bits */
#define PCI_BIST_CODE_MASK	0x0f	/* Return result */
#define PCI_BIST_START		0x40	/* 1 to start BIST, 2 secs or less */
#define PCI_BIST_CAPABLE	0x80	/* 1 if BIST capable */

/*
 * Base addresses specify locations in memory or I/O space.
 * Decoded size can be determined by writing a value of
 * 0xffffffff to the register, and reading it back.  Only
 * 1 bits are decoded.
 */
#define PCI_BASE_ADDRESS_0	0x10	/* 32 bits */
#define PCI_BASE_ADDRESS_1	0x14	/* 32 bits [htype 0,1 only] */
#define PCI_BASE_ADDRESS_2	0x18	/* 32 bits [htype 0 only] */
#define PCI_BASE_ADDRESS_3	0x1c	/* 32 bits */
#define PCI_BASE_ADDRESS_4	0x20	/* 32 bits */
#define PCI_BASE_ADDRESS_5	0x24	/* 32 bits */
#define  PCI_BASE_ADDRESS_SPACE 0x01	/* 0 = memory, 1 = I/O */
#define  PCI_BASE_ADDRESS_SPACE_IO 0x01
#define  PCI_BASE_ADDRESS_SPACE_MEMORY 0x00
#define  PCI_BASE_ADDRESS_MEM_TYPE_MASK 0x06
#define  PCI_BASE_ADDRESS_MEM_TYPE_32	0x00	/* 32 bit address */
#define  PCI_BASE_ADDRESS_MEM_TYPE_1M	0x02	/* Below 1M [obsolete] */
#define  PCI_BASE_ADDRESS_MEM_TYPE_64	0x04	/* 64 bit address */
#define  PCI_BASE_ADDRESS_MEM_PREFETCH	0x08	/* prefetchable? */
#define  PCI_BASE_ADDRESS_MEM_MASK	(~0x0fULL)
#define  PCI_BASE_ADDRESS_IO_MASK	(~0x03ULL)
/* bit 1 is reserved if address_space = 1 */

/* Convert a regsister address (e.g. PCI_BASE_ADDRESS_1) to a bar # (e.g. 1) */
#define pci_offset_to_barnum(offset)	\
		(((offset) - PCI_BASE_ADDRESS_0) / sizeof(u32))

/* Header type 0 (normal devices) */
#define PCI_CARDBUS_CIS		0x28
#define PCI_SUBSYSTEM_VENDOR_ID 0x2c
#define PCI_SUBSYSTEM_ID	0x2e
#define PCI_ROM_ADDRESS		0x30	/* Bits 31..11 are address, 10..1 reserved */
#define  PCI_ROM_ADDRESS_ENABLE 0x01
#define PCI_ROM_ADDRESS_MASK	(~0x7ffULL)

#define PCI_CAPABILITY_LIST	0x34	/* Offset of first capability list entry */

/* 0x35-0x3b are reserved */
#define PCI_INTERRUPT_LINE	0x3c	/* 8 bits */
#define PCI_INTERRUPT_PIN	0x3d	/* 8 bits */
#define PCI_MIN_GNT		0x3e	/* 8 bits */
#define PCI_MAX_LAT		0x3f	/* 8 bits */

#define PCI_INTERRUPT_LINE_DISABLE	0xff

/* Header type 1 (PCI-to-PCI bridges) */
#define PCI_PRIMARY_BUS		0x18	/* Primary bus number */
#define PCI_SECONDARY_BUS	0x19	/* Secondary bus number */
#define PCI_SUBORDINATE_BUS	0x1a	/* Highest bus number behind the bridge */
#define PCI_SEC_LATENCY_TIMER	0x1b	/* Latency timer for secondary interface */
#define PCI_IO_BASE		0x1c	/* I/O range behind the bridge */
#define PCI_IO_LIMIT		0x1d
#define  PCI_IO_RANGE_TYPE_MASK 0x0f	/* I/O bridging type */
#define  PCI_IO_RANGE_TYPE_16	0x00
#define  PCI_IO_RANGE_TYPE_32	0x01
#define  PCI_IO_RANGE_MASK	~0x0f
#define PCI_SEC_STATUS		0x1e	/* Secondary status register, only bit 14 used */
#define PCI_MEMORY_BASE		0x20	/* Memory range behind */
#define PCI_MEMORY_LIMIT	0x22
#define  PCI_MEMORY_RANGE_TYPE_MASK 0x0f
#define  PCI_MEMORY_RANGE_MASK	~0x0f
#define PCI_PREF_MEMORY_BASE	0x24	/* Prefetchable memory range behind */
#define PCI_PREF_MEMORY_LIMIT	0x26
#define  PCI_PREF_RANGE_TYPE_MASK 0x0f
#define  PCI_PREF_RANGE_TYPE_32 0x00
#define  PCI_PREF_RANGE_TYPE_64 0x01
#define  PCI_PREF_RANGE_MASK	~0x0f
#define PCI_PREF_BASE_UPPER32	0x28	/* Upper half of prefetchable memory range */
#define PCI_PREF_LIMIT_UPPER32	0x2c
#define PCI_IO_BASE_UPPER16	0x30	/* Upper half of I/O addresses */
#define PCI_IO_LIMIT_UPPER16	0x32
/* 0x34 same as for htype 0 */
/* 0x35-0x3b is reserved */
#define PCI_ROM_ADDRESS1	0x38	/* Same as PCI_ROM_ADDRESS, but for htype 1 */
/* 0x3c-0x3d are same as for htype 0 */
#define PCI_BRIDGE_CONTROL	0x3e
#define  PCI_BRIDGE_CTL_PARITY	0x01	/* Enable parity detection on secondary interface */
#define  PCI_BRIDGE_CTL_SERR	0x02	/* The same for SERR forwarding */
#define  PCI_BRIDGE_CTL_NO_ISA	0x04	/* Disable bridging of ISA ports */
#define  PCI_BRIDGE_CTL_VGA	0x08	/* Forward VGA addresses */
#define  PCI_BRIDGE_CTL_MASTER_ABORT 0x20  /* Report master aborts */
#define  PCI_BRIDGE_CTL_BUS_RESET 0x40	/* Secondary bus reset */
#define  PCI_BRIDGE_CTL_FAST_BACK 0x80	/* Fast Back2Back enabled on secondary interface */

/* Header type 2 (CardBus bridges) */
#define PCI_CB_CAPABILITY_LIST	0x14
/* 0x15 reserved */
#define PCI_CB_SEC_STATUS	0x16	/* Secondary status */
#define PCI_CB_PRIMARY_BUS	0x18	/* PCI bus number */
#define PCI_CB_CARD_BUS		0x19	/* CardBus bus number */
#define PCI_CB_SUBORDINATE_BUS	0x1a	/* Subordinate bus number */
#define PCI_CB_LATENCY_TIMER	0x1b	/* CardBus latency timer */
#define PCI_CB_MEMORY_BASE_0	0x1c
#define PCI_CB_MEMORY_LIMIT_0	0x20
#define PCI_CB_MEMORY_BASE_1	0x24
#define PCI_CB_MEMORY_LIMIT_1	0x28
#define PCI_CB_IO_BASE_0	0x2c
#define PCI_CB_IO_BASE_0_HI	0x2e
#define PCI_CB_IO_LIMIT_0	0x30
#define PCI_CB_IO_LIMIT_0_HI	0x32
#define PCI_CB_IO_BASE_1	0x34
#define PCI_CB_IO_BASE_1_HI	0x36
#define PCI_CB_IO_LIMIT_1	0x38
#define PCI_CB_IO_LIMIT_1_HI	0x3a
#define  PCI_CB_IO_RANGE_MASK	~0x03
/* 0x3c-0x3d are same as for htype 0 */
#define PCI_CB_BRIDGE_CONTROL	0x3e
#define  PCI_CB_BRIDGE_CTL_PARITY	0x01	/* Similar to standard bridge control register */
#define  PCI_CB_BRIDGE_CTL_SERR		0x02
#define  PCI_CB_BRIDGE_CTL_ISA		0x04
#define  PCI_CB_BRIDGE_CTL_VGA		0x08
#define  PCI_CB_BRIDGE_CTL_MASTER_ABORT 0x20
#define  PCI_CB_BRIDGE_CTL_CB_RESET	0x40	/* CardBus reset */
#define  PCI_CB_BRIDGE_CTL_16BIT_INT	0x80	/* Enable interrupt for 16-bit cards */
#define  PCI_CB_BRIDGE_CTL_PREFETCH_MEM0 0x100	/* Prefetch enable for both memory regions */
#define  PCI_CB_BRIDGE_CTL_PREFETCH_MEM1 0x200
#define  PCI_CB_BRIDGE_CTL_POST_WRITES	0x400
#define PCI_CB_SUBSYSTEM_VENDOR_ID 0x40
#define PCI_CB_SUBSYSTEM_ID	0x42
#define PCI_CB_LEGACY_MODE_BASE 0x44	/* 16-bit PC Card legacy mode base address (ExCa) */
/* 0x48-0x7f reserved */

/* Capability lists */

#define PCI_CAP_LIST_ID		0	/* Capability ID */
#define  PCI_CAP_ID_PM		0x01	/* Power Management */
#define  PCI_CAP_ID_AGP		0x02	/* Accelerated Graphics Port */
#define  PCI_CAP_ID_VPD		0x03	/* Vital Product Data */
#define  PCI_CAP_ID_SLOTID	0x04	/* Slot Identification */
#define  PCI_CAP_ID_MSI		0x05	/* Message Signalled Interrupts */
#define  PCI_CAP_ID_CHSWP	0x06	/* CompactPCI HotSwap */
#define  PCI_CAP_ID_PCIX	0x07	/* PCI-X */
#define  PCI_CAP_ID_HT		0x08	/* HyperTransport */
#define  PCI_CAP_ID_VNDR	0x09	/* Vendor-Specific */
#define  PCI_CAP_ID_DBG		0x0A	/* Debug port */
#define  PCI_CAP_ID_CCRC	0x0B	/* CompactPCI Central Resource Control */
#define  PCI_CAP_ID_SHPC	0x0C	/* PCI Standard Hot-Plug Controller */
#define  PCI_CAP_ID_SSVID	0x0D	/* Bridge subsystem vendor/device ID */
#define  PCI_CAP_ID_AGP3	0x0E	/* AGP Target PCI-PCI bridge */
#define  PCI_CAP_ID_SECDEV	0x0F	/* Secure Device */
#define  PCI_CAP_ID_EXP		0x10	/* PCI Express */
#define  PCI_CAP_ID_MSIX	0x11	/* MSI-X */
#define  PCI_CAP_ID_SATA	0x12	/* SATA Data/Index Conf. */
#define  PCI_CAP_ID_AF		0x13	/* PCI Advanced Features */
#define  PCI_CAP_ID_EA		0x14	/* PCI Enhanced Allocation */
#define  PCI_CAP_ID_MAX		PCI_CAP_ID_EA
#define PCI_CAP_LIST_NEXT	1	/* Next capability in the list */
#define PCI_CAP_FLAGS		2	/* Capability defined flags (16 bits) */
#define PCI_CAP_SIZEOF		4

/* Power Management Registers */

#define  PCI_PM_CAP_VER_MASK	0x0007	/* Version */
#define  PCI_PM_CAP_PME_CLOCK	0x0008	/* PME clock required */
#define  PCI_PM_CAP_AUX_POWER	0x0010	/* Auxilliary power support */
#define  PCI_PM_CAP_DSI		0x0020	/* Device specific initialization */
#define  PCI_PM_CAP_D1		0x0200	/* D1 power state support */
#define  PCI_PM_CAP_D2		0x0400	/* D2 power state support */
#define  PCI_PM_CAP_PME		0x0800	/* PME pin supported */
#define PCI_PM_CTRL		4	/* PM control and status register */
#define  PCI_PM_CTRL_STATE_MASK 0x0003	/* Current power state (D0 to D3) */
#define  PCI_PM_CTRL_PME_ENABLE 0x0100	/* PME pin enable */
#define  PCI_PM_CTRL_DATA_SEL_MASK	0x1e00	/* Data select (??) */
#define  PCI_PM_CTRL_DATA_SCALE_MASK	0x6000	/* Data scale (??) */
#define  PCI_PM_CTRL_PME_STATUS 0x8000	/* PME pin status */
#define PCI_PM_PPB_EXTENSIONS	6	/* PPB support extensions (??) */
#define  PCI_PM_PPB_B2_B3	0x40	/* Stop clock when in D3hot (??) */
#define  PCI_PM_BPCC_ENABLE	0x80	/* Bus power/clock control enable (??) */
#define PCI_PM_DATA_REGISTER	7	/* (??) */
#define PCI_PM_SIZEOF		8

/* AGP registers */

#define PCI_AGP_VERSION		2	/* BCD version number */
#define PCI_AGP_RFU		3	/* Rest of capability flags */
#define PCI_AGP_STATUS		4	/* Status register */
#define  PCI_AGP_STATUS_RQ_MASK 0xff000000	/* Maximum number of requests - 1 */
#define  PCI_AGP_STATUS_SBA	0x0200	/* Sideband addressing supported */
#define  PCI_AGP_STATUS_64BIT	0x0020	/* 64-bit addressing supported */
#define  PCI_AGP_STATUS_FW	0x0010	/* FW transfers supported */
#define  PCI_AGP_STATUS_RATE4	0x0004	/* 4x transfer rate supported */
#define  PCI_AGP_STATUS_RATE2	0x0002	/* 2x transfer rate supported */
#define  PCI_AGP_STATUS_RATE1	0x0001	/* 1x transfer rate supported */
#define PCI_AGP_COMMAND		8	/* Control register */
#define  PCI_AGP_COMMAND_RQ_MASK 0xff000000  /* Master: Maximum number of requests */
#define  PCI_AGP_COMMAND_SBA	0x0200	/* Sideband addressing enabled */
#define  PCI_AGP_COMMAND_AGP	0x0100	/* Allow processing of AGP transactions */
#define  PCI_AGP_COMMAND_64BIT	0x0020	/* Allow processing of 64-bit addresses */
#define  PCI_AGP_COMMAND_FW	0x0010	/* Force FW transfers */
#define  PCI_AGP_COMMAND_RATE4	0x0004	/* Use 4x rate */
#define  PCI_AGP_COMMAND_RATE2	0x0002	/* Use 4x rate */
#define  PCI_AGP_COMMAND_RATE1	0x0001	/* Use 4x rate */
#define PCI_AGP_SIZEOF		12

/* PCI-X registers */

#define  PCI_X_CMD_DPERR_E      0x0001  /* Data Parity Error Recovery Enable */
#define  PCI_X_CMD_ERO          0x0002  /* Enable Relaxed Ordering */
#define  PCI_X_CMD_MAX_READ     0x0000  /* Max Memory Read Byte Count */
#define  PCI_X_CMD_MAX_SPLIT    0x0030  /* Max Outstanding Split Transactions */
#define  PCI_X_CMD_VERSION(x)   (((x) >> 12) & 3) /* Version */


/* Slot Identification */

#define PCI_SID_ESR		2	/* Expansion Slot Register */
#define  PCI_SID_ESR_NSLOTS	0x1f	/* Number of expansion slots available */
#define  PCI_SID_ESR_FIC	0x20	/* First In Chassis Flag */
#define PCI_SID_CHASSIS_NR	3	/* Chassis Number */

/* Message Signalled Interrupts registers */

#define PCI_MSI_FLAGS		2	/* Various flags */
#define  PCI_MSI_FLAGS_64BIT	0x80	/* 64-bit addresses allowed */
#define  PCI_MSI_FLAGS_QSIZE	0x70	/* Message queue size configured */
#define  PCI_MSI_FLAGS_QMASK	0x0e	/* Maximum queue size available */
#define  PCI_MSI_FLAGS_ENABLE	0x01	/* MSI feature enabled */
#define  PCI_MSI_FLAGS_MASKBIT	0x0100	/* Per-vector masking capable */
#define PCI_MSI_RFU		3	/* Rest of capability flags */
#define PCI_MSI_ADDRESS_LO	4	/* Lower 32 bits */
#define PCI_MSI_ADDRESS_HI	8	/* Upper 32 bits (if PCI_MSI_FLAGS_64BIT set) */
#define PCI_MSI_DATA_32		8	/* 16 bits of data for 32-bit devices */
#define PCI_MSI_DATA_64		12	/* 16 bits of data for 64-bit devices */

#define PCI_MAX_PCI_DEVICES	32
#define PCI_MAX_PCI_FUNCTIONS	8

#define PCI_FIND_CAP_TTL 0x48
#define CAP_START_POS 0x40

/* Extended Capabilities (PCI-X 2.0 and Express) */
#define PCI_EXT_CAP_ID(header)		(header & 0x0000ffff)
#define PCI_EXT_CAP_VER(header)		((header >> 16) & 0xf)
#define PCI_EXT_CAP_NEXT(header)	((header >> 20) & 0xffc)

#define PCI_EXT_CAP_ID_ERR	0x01	/* Advanced Error Reporting */
#define PCI_EXT_CAP_ID_VC	0x02	/* Virtual Channel Capability */
#define PCI_EXT_CAP_ID_DSN	0x03	/* Device Serial Number */
#define PCI_EXT_CAP_ID_PWR	0x04	/* Power Budgeting */
#define PCI_EXT_CAP_ID_RCLD	0x05	/* Root Complex Link Declaration */
#define PCI_EXT_CAP_ID_RCILC	0x06	/* Root Complex Internal Link Control */
#define PCI_EXT_CAP_ID_RCEC	0x07	/* Root Complex Event Collector */
#define PCI_EXT_CAP_ID_MFVC	0x08	/* Multi-Function VC Capability */
#define PCI_EXT_CAP_ID_VC9	0x09	/* same as _VC */
#define PCI_EXT_CAP_ID_RCRB	0x0A	/* Root Complex RB? */
#define PCI_EXT_CAP_ID_VNDR	0x0B	/* Vendor-Specific */
#define PCI_EXT_CAP_ID_CAC	0x0C	/* Config Access - obsolete */
#define PCI_EXT_CAP_ID_ACS	0x0D	/* Access Control Services */
#define PCI_EXT_CAP_ID_ARI	0x0E	/* Alternate Routing ID */
#define PCI_EXT_CAP_ID_ATS	0x0F	/* Address Translation Services */
#define PCI_EXT_CAP_ID_SRIOV	0x10	/* Single Root I/O Virtualization */
#define PCI_EXT_CAP_ID_MRIOV	0x11	/* Multi Root I/O Virtualization */
#define PCI_EXT_CAP_ID_MCAST	0x12	/* Multicast */
#define PCI_EXT_CAP_ID_PRI	0x13	/* Page Request Interface */
#define PCI_EXT_CAP_ID_AMD_XXX	0x14	/* Reserved for AMD */
#define PCI_EXT_CAP_ID_REBAR	0x15	/* Resizable BAR */
#define PCI_EXT_CAP_ID_DPA	0x16	/* Dynamic Power Allocation */
#define PCI_EXT_CAP_ID_TPH	0x17	/* TPH Requester */
#define PCI_EXT_CAP_ID_LTR	0x18	/* Latency Tolerance Reporting */
#define PCI_EXT_CAP_ID_SECPCI	0x19	/* Secondary PCIe Capability */
#define PCI_EXT_CAP_ID_PMUX	0x1A	/* Protocol Multiplexing */
#define PCI_EXT_CAP_ID_PASID	0x1B	/* Process Address Space ID */
#define PCI_EXT_CAP_ID_DPC	0x1D	/* Downstream Port Containment */
#define PCI_EXT_CAP_ID_L1SS	0x1E	/* L1 PM Substates */
#define PCI_EXT_CAP_ID_PTM	0x1F	/* Precision Time Measurement */
#define PCI_EXT_CAP_ID_MAX	PCI_EXT_CAP_ID_PTM

/* Enhanced Allocation Registers */
#define PCI_EA_NUM_ENT		2	/* Number of Capability Entries */
#define  PCI_EA_NUM_ENT_MASK	0x3f	/* Num Entries Mask */
#define PCI_EA_FIRST_ENT	4	/* First EA Entry in List */
#define  PCI_EA_ES		0x00000007 /* Entry Size */
#define  PCI_EA_BEI		0x000000f0 /* BAR Equivalent Indicator */
/* 9-14 map to VF BARs 0-5 respectively */
#define  PCI_EA_BEI_VF_BAR0	9
#define  PCI_EA_BEI_VF_BAR5	14
/* Base, MaxOffset registers */
/* bit 0 is reserved */
#define  PCI_EA_IS_64		0x00000002	/* 64-bit field flag */
#define  PCI_EA_FIELD_MASK	0xfffffffc	/* For Base & Max Offset */

/* PCI Express capabilities */
#define PCI_EXP_FLAGS		2	/* Capabilities register */
#define  PCI_EXP_FLAGS_VERS	0x000f	/* Capability Version */
#define  PCI_EXP_FLAGS_TYPE	0x00f0	/* Device/Port type */
#define   PCI_EXP_TYPE_ROOT_PORT   0x4	/* Root Port */
#define   PCI_EXP_TYPE_DOWNSTREAM  0x6	/* Downstream Port */
#define   PCI_EXP_TYPE_PCIE_BRIDGE 0x8	/* PCI/PCI-X to PCIe Bridge */
#define PCI_EXP_DEVCAP		4	/* Device capabilities */
#define  PCI_EXP_DEVCAP_FLR	0x10000000 /* Function Level Reset */
#define  PCI_EXP_DEVCAP_PAYLOAD 0x0007	/* Max payload size supported */
#define  PCI_EXP_DEVCAP_PAYLOAD_128B 0x0000 /* 128 Bytes */
#define  PCI_EXP_DEVCAP_PAYLOAD_256B 0x0001 /* 256 Bytes */
#define  PCI_EXP_DEVCAP_PAYLOAD_512B 0x0002 /* 512 Bytes */
#define  PCI_EXP_DEVCAP_PAYLOAD_1024B 0x0003 /* 1024 Bytes */
#define  PCI_EXP_DEVCAP_PAYLOAD_2048B 0x0004 /* 2048 Bytes */
#define  PCI_EXP_DEVCAP_PAYLOAD_4096B 0x0005 /* 4096 Bytes */
#define PCI_EXP_DEVCTL		8	/* Device Control */
#define  PCI_EXP_DEVCTL_PAYLOAD	0x00e0	/* Max_Payload_Size */
#define  PCI_EXP_DEVCTL_PAYLOAD_128B 0x0000 /* 128 Bytes */
#define  PCI_EXP_DEVCTL_PAYLOAD_256B 0x0020 /* 256 Bytes */
#define  PCI_EXP_DEVCTL_PAYLOAD_512B 0x0040 /* 512 Bytes */
#define  PCI_EXP_DEVCTL_PAYLOAD_1024B 0x0060 /* 1024 Bytes */
#define  PCI_EXP_DEVCTL_PAYLOAD_2048B 0x0080 /* 2048 Bytes */
#define  PCI_EXP_DEVCTL_PAYLOAD_4096B 0x00a0 /* 4096 Bytes */
#define  PCI_EXP_DEVCTL_RELAX_EN 0x0010 /* Enable relaxed ordering */
#define  PCI_EXP_DEVCTL_NOSNOOP_EN 0x0800  /* Enable No Snoop */
#define  PCI_EXP_DEVCTL_READRQ	0x7000	/* Max_Read_Request_Size */
#define  PCI_EXP_DEVCTL_READRQ_128B  0x0000 /* 128 Bytes */
#define  PCI_EXP_DEVCTL_READRQ_256B  0x1000 /* 256 Bytes */
#define  PCI_EXP_DEVCTL_READRQ_512B  0x2000 /* 512 Bytes */
#define  PCI_EXP_DEVCTL_READRQ_1024B 0x3000 /* 1024 Bytes */
#define  PCI_EXP_DEVCTL_READRQ_2048B 0x4000 /* 2048 Bytes */
#define  PCI_EXP_DEVCTL_READRQ_4096B 0x5000 /* 4096 Bytes */
#define  PCI_EXP_DEVCTL_BCR_FLR	0x8000  /* Bridge Configuration Retry / FLR */
#define PCI_EXP_LNKCAP		12	/* Link Capabilities */
#define  PCI_EXP_LNKCAP_SLS	0x0000000f /* Supported Link Speeds */
#define  PCI_EXP_LNKCAP_SLS_2_5GB 0x00000001 /* LNKCAP2 SLS Vector bit 0 */
#define  PCI_EXP_LNKCAP_SLS_5_0GB 0x00000002 /* LNKCAP2 SLS Vector bit 1 */
#define  PCI_EXP_LNKCAP_SLS_8_0GB 0x00000003 /* LNKCAP2 SLS Vector bit 2 */
#define  PCI_EXP_LNKCAP_MLW	0x000003f0 /* Maximum Link Width */
#define  PCI_EXP_LNKCAP_DLLLARC	0x00100000 /* Data Link Layer Link Active Reporting Capable */
#define PCI_EXP_LNKCTL		16	/* Link Control */
#define  PCI_EXP_LNKCTL_RL	0x0020	/* Retrain Link */
#define PCI_EXP_LNKSTA		18	/* Link Status */
#define  PCI_EXP_LNKSTA_CLS	0x000f	/* Current Link Speed */
#define  PCI_EXP_LNKSTA_CLS_2_5GB 0x0001 /* Current Link Speed 2.5GT/s */
#define  PCI_EXP_LNKSTA_CLS_5_0GB 0x0002 /* Current Link Speed 5.0GT/s */
#define  PCI_EXP_LNKSTA_CLS_8_0GB 0x0003 /* Current Link Speed 8.0GT/s */
#define  PCI_EXP_LNKSTA_NLW	0x03f0	/* Negotiated Link Width */
#define  PCI_EXP_LNKSTA_NLW_SHIFT 4	/* start of NLW mask in link status */
#define  PCI_EXP_LNKSTA_LT	0x0800	/* Link Training */
#define  PCI_EXP_LNKSTA_DLLLA	0x2000	/* Data Link Layer Link Active */
#define  PCI_EXP_LNKSTA_LBMS	0x4000	/* Link Bandwidth Management Status */
#define PCI_EXP_SLTCAP		20	/* Slot Capabilities */
#define  PCI_EXP_SLTCAP_PSN	0xfff80000 /* Physical Slot Number */
#define PCI_EXP_RTCTL		28	/* Root Control */
#define  PCI_EXP_RTCTL_CRSSVE	0x0010	/* CRS Software Visibility Enable */
#define PCI_EXP_RTCAP		30	/* Root Capabilities */
#define  PCI_EXP_RTCAP_CRSVIS	0x0001	/* CRS Software Visibility capability */
#define PCI_EXP_DEVCAP2		36	/* Device Capabilities 2 */
#define  PCI_EXP_DEVCAP2_ARI	0x00000020 /* ARI Forwarding Supported */
#define PCI_EXP_DEVCTL2		40	/* Device Control 2 */
#define  PCI_EXP_DEVCTL2_ARI	0x0020 /* Alternative Routing-ID */
#define PCI_EXP_LNKCAP2		44	/* Link Capability 2 */
#define  PCI_EXP_LNKCAP2_SLS	0x000000fe /* Supported Link Speeds Vector */
#define PCI_EXP_LNKCTL2		48	/* Link Control 2 */
#define  PCI_EXP_LNKCTL2_TLS	0x000f	/* Target Link Speed */
#define  PCI_EXP_LNKCTL2_TLS_2_5GT 0x0001 /* Target Link Speed 2.5GT/s */
#define  PCI_EXP_LNKCTL2_TLS_5_0GT 0x0002 /* Target Link Speed 5.0GT/s */
#define  PCI_EXP_LNKCTL2_TLS_8_0GT 0x0003 /* Target Link Speed 8.0GT/s */

/* Advanced Error Reporting */
#define PCI_ERR_CAP		24	/* Advanced Error Capabilities */
#define  PCI_ERR_CAP_FEP(x)	((x) & 31)	/* First Error Pointer */
#define  PCI_ERR_CAP_ECRC_GENC	0x00000020	/* ECRC Generation Capable */
#define  PCI_ERR_CAP_ECRC_GENE	0x00000040	/* ECRC Generation Enable */
#define  PCI_ERR_CAP_ECRC_CHKC	0x00000080	/* ECRC Check Capable */
#define  PCI_ERR_CAP_ECRC_CHKE	0x00000100	/* ECRC Check Enable */

/* Single Root I/O Virtualization Registers */
#define PCI_SRIOV_CAP		0x04	/* SR-IOV Capabilities */
#define PCI_SRIOV_CTRL		0x08	/* SR-IOV Control */
#define  PCI_SRIOV_CTRL_VFE	0x01	/* VF Enable */
#define  PCI_SRIOV_CTRL_MSE	0x08	/* VF Memory Space Enable */
#define  PCI_SRIOV_CTRL_ARI	0x10	/* ARI Capable Hierarchy */
#define PCI_SRIOV_INITIAL_VF	0x0c	/* Initial VFs */
#define PCI_SRIOV_TOTAL_VF	0x0e	/* Total VFs */
#define PCI_SRIOV_NUM_VF	0x10	/* Number of VFs */
#define PCI_SRIOV_VF_OFFSET	0x14	/* First VF Offset */
#define PCI_SRIOV_VF_STRIDE	0x16	/* Following VF Stride */
#define PCI_SRIOV_VF_DID	0x1a	/* VF Device ID */

/* Include the ID list */

#include <pci_ids.h>

/*
 * Config Address for PCI Configuration Mechanism #1
 *
 * See PCI Local Bus Specification, Revision 3.0,
 * Section 3.2.2.3.2, Figure 3-2, p. 50.
 */

#define PCI_CONF1_BUS_SHIFT	16 /* Bus number */
#define PCI_CONF1_DEV_SHIFT	11 /* Device number */
#define PCI_CONF1_FUNC_SHIFT	8  /* Function number */

#define PCI_CONF1_BUS_MASK	0xff
#define PCI_CONF1_DEV_MASK	0x1f
#define PCI_CONF1_FUNC_MASK	0x7
#define PCI_CONF1_REG_MASK	0xfc /* Limit aligned offset to a maximum of 256B */

#define PCI_CONF1_ENABLE	BIT(31)
#define PCI_CONF1_BUS(x)	(((x) & PCI_CONF1_BUS_MASK) << PCI_CONF1_BUS_SHIFT)
#define PCI_CONF1_DEV(x)	(((x) & PCI_CONF1_DEV_MASK) << PCI_CONF1_DEV_SHIFT)
#define PCI_CONF1_FUNC(x)	(((x) & PCI_CONF1_FUNC_MASK) << PCI_CONF1_FUNC_SHIFT)
#define PCI_CONF1_REG(x)	((x) & PCI_CONF1_REG_MASK)

#define PCI_CONF1_ADDRESS(bus, dev, func, reg) \
	(PCI_CONF1_ENABLE | \
	 PCI_CONF1_BUS(bus) | \
	 PCI_CONF1_DEV(dev) | \
	 PCI_CONF1_FUNC(func) | \
	 PCI_CONF1_REG(reg))

/*
 * Extension of PCI Config Address for accessing extended PCIe registers
 *
 * No standardized specification, but used on lot of non-ECAM-compliant ARM SoCs
 * or on AMD Barcelona and new CPUs. Reserved bits [27:24] of PCI Config Address
 * are used for specifying additional 4 high bits of PCI Express register.
 */

#define PCI_CONF1_EXT_REG_SHIFT	16
#define PCI_CONF1_EXT_REG_MASK	0xf00
#define PCI_CONF1_EXT_REG(x)	(((x) & PCI_CONF1_EXT_REG_MASK) << PCI_CONF1_EXT_REG_SHIFT)

#define PCI_CONF1_EXT_ADDRESS(bus, dev, func, reg) \
	(PCI_CONF1_ADDRESS(bus, dev, func, reg) | \
	 PCI_CONF1_EXT_REG(reg))

/*
 * Enhanced Configuration Access Mechanism (ECAM)
 *
 * See PCI Express Base Specification, Revision 5.0, Version 1.0,
 * Section 7.2.2, Table 7-1, p. 677.
 */
#define PCIE_ECAM_BUS_SHIFT	20 /* Bus number */
#define PCIE_ECAM_DEV_SHIFT	15 /* Device number */
#define PCIE_ECAM_FUNC_SHIFT	12 /* Function number */

#define PCIE_ECAM_BUS_MASK	0xff
#define PCIE_ECAM_DEV_MASK	0x1f
#define PCIE_ECAM_FUNC_MASK	0x7
#define PCIE_ECAM_REG_MASK	0xfff /* Limit offset to a maximum of 4K */

#define PCIE_ECAM_BUS(x)	(((x) & PCIE_ECAM_BUS_MASK) << PCIE_ECAM_BUS_SHIFT)
#define PCIE_ECAM_DEV(x)	(((x) & PCIE_ECAM_DEV_MASK) << PCIE_ECAM_DEV_SHIFT)
#define PCIE_ECAM_FUNC(x)	(((x) & PCIE_ECAM_FUNC_MASK) << PCIE_ECAM_FUNC_SHIFT)
#define PCIE_ECAM_REG(x)	((x) & PCIE_ECAM_REG_MASK)

#define PCIE_ECAM_OFFSET(bus, dev, func, where) \
	(PCIE_ECAM_BUS(bus) | \
	 PCIE_ECAM_DEV(dev) | \
	 PCIE_ECAM_FUNC(func) | \
	 PCIE_ECAM_REG(where))

#ifndef __ASSEMBLY__

#include <dm/pci.h>

#ifdef CONFIG_SYS_PCI_64BIT
typedef u64 pci_addr_t;
typedef u64 pci_size_t;
#else
typedef unsigned long pci_addr_t;
typedef unsigned long pci_size_t;
#endif

struct pci_region {
	pci_addr_t bus_start;	/* Start on the bus */
	phys_addr_t phys_start;	/* Start in physical address space */
	pci_size_t size;	/* Size */
	unsigned long flags;	/* Resource flags */

	pci_addr_t bus_lower;
};

#define PCI_REGION_MEM		0x00000000	/* PCI memory space */
#define PCI_REGION_IO		0x00000001	/* PCI IO space */
#define PCI_REGION_TYPE		0x00000001
#define PCI_REGION_PREFETCH	0x00000008	/* prefetchable PCI memory */

#define PCI_REGION_SYS_MEMORY	0x00000100	/* System memory */
#define PCI_REGION_RO		0x00000200	/* Read-only memory */

static inline void pci_set_region(struct pci_region *reg,
				      pci_addr_t bus_start,
				      phys_addr_t phys_start,
				      pci_size_t size,
				      unsigned long flags) {
	reg->bus_start	= bus_start;
	reg->phys_start = phys_start;
	reg->size	= size;
	reg->flags	= flags;
}

typedef int pci_dev_t;

#define PCI_BUS(d)		(((d) >> 16) & 0xff)

/*
 * Please note the difference in DEVFN usage in U-Boot vs Linux. U-Boot
 * uses DEVFN in bits 15-8 but Linux instead expects DEVFN in bits 7-0.
 * Please see the Linux header include/uapi/linux/pci.h for more details.
 * This is relevant for the following macros:
 * PCI_DEV, PCI_FUNC, PCI_DEVFN
 * The U-Boot macro PCI_DEV is equivalent to the Linux PCI_SLOT version with
 * the remark from above (input is in bits 15-8 instead of 7-0.
 */
#define PCI_DEV(d)		(((d) >> 11) & 0x1f)
#define PCI_FUNC(d)		(((d) >> 8) & 0x7)
#define PCI_DEVFN(d, f)		((d) << 11 | (f) << 8)

#define PCI_MASK_BUS(bdf)	((bdf) & 0xffff)
#define PCI_ADD_BUS(bus, devfn)	(((bus) << 16) | (devfn))
#define PCI_BDF(b, d, f)	((b) << 16 | PCI_DEVFN(d, f))
#define PCI_ANY_ID		(~0)

/* Convert from Linux format to U-Boot format */
#define PCI_TO_BDF(val)		((val) << 8)

struct pci_device_id {
	unsigned int vendor, device;	/* Vendor and device ID or PCI_ANY_ID */
	unsigned int subvendor, subdevice; /* Subsystem ID's or PCI_ANY_ID */
	unsigned int class, class_mask;	/* (class,subclass,prog-if) triplet */
	unsigned long driver_data;	/* Data private to the driver */
};

struct pci_controller;

struct pci_config_table {
	unsigned int vendor, device;		/* Vendor and device ID or PCI_ANY_ID */
	unsigned int class;			/* Class ID, or  PCI_ANY_ID */
	unsigned int bus;			/* Bus number, or PCI_ANY_ID */
	unsigned int dev;			/* Device number, or PCI_ANY_ID */
	unsigned int func;			/* Function number, or PCI_ANY_ID */

	void (*config_device)(struct pci_controller* hose, pci_dev_t dev,
			      struct pci_config_table *);
	unsigned long priv[3];
};

extern void pci_cfgfunc_do_nothing(struct pci_controller* hose, pci_dev_t dev,
				   struct pci_config_table *);
extern void pci_cfgfunc_config_device(struct pci_controller* hose, pci_dev_t dev,
				      struct pci_config_table *);

#define INDIRECT_TYPE_NO_PCIE_LINK	1

/**
 * Structure of a PCI controller (host bridge)
 *
 * With driver model this is dev_get_uclass_priv(bus)
 *
 * @skip_auto_config_until_reloc: true to avoid auto-config until U-Boot has
 *	relocated. Normally if PCI is used before relocation, this happens
 *	before relocation also. Some platforms set up static configuration in
 *	TPL/SPL to reduce code size and boot time, since these phases only know
 *	about a small subset of PCI devices. This is normally false.
 */
struct pci_controller {
	struct udevice *bus;
	struct udevice *ctlr;
	bool skip_auto_config_until_reloc;

	int first_busno;
	int last_busno;

	volatile unsigned int *cfg_addr;
	volatile unsigned char *cfg_data;

	int indirect_type;

	/*
	 * TODO(sjg@chromium.org): With driver model we use struct
	 * pci_controller for both the controller and any bridge devices
	 * attached to it. But there is only one region list and it is in the
	 * top-level controller.
	 *
	 * This could be changed so that struct pci_controller is only used
	 * for PCI controllers and a separate UCLASS (or perhaps
	 * UCLASS_PCI_GENERIC) is used for bridges.
	 */
	struct pci_region *regions;
	int region_count;

	struct pci_config_table *config_table;

	void (*fixup_irq)(struct pci_controller *, pci_dev_t);

	/* Used by auto config */
	struct pci_region *pci_mem, *pci_io, *pci_prefetch;
};

#if defined(CONFIG_DM_PCI_COMPAT)
extern phys_addr_t pci_hose_bus_to_phys(struct pci_controller* hose,
					pci_addr_t addr, unsigned long flags);
extern pci_addr_t pci_hose_phys_to_bus(struct pci_controller* hose,
					phys_addr_t addr, unsigned long flags);

#define pci_phys_to_bus(dev, addr, flags) \
	pci_hose_phys_to_bus(pci_bus_to_hose(PCI_BUS(dev)), (addr), (flags))
#define pci_bus_to_phys(dev, addr, flags) \
	pci_hose_bus_to_phys(pci_bus_to_hose(PCI_BUS(dev)), (addr), (flags))

#define pci_virt_to_bus(dev, addr, flags) \
	pci_hose_phys_to_bus(pci_bus_to_hose(PCI_BUS(dev)), \
			     (virt_to_phys(addr)), (flags))
#define pci_bus_to_virt(dev, addr, flags, len, map_flags) \
	map_physmem(pci_hose_bus_to_phys(pci_bus_to_hose(PCI_BUS(dev)), \
					 (addr), (flags)), \
		    (len), (map_flags))

#define pci_phys_to_mem(dev, addr) \
	pci_phys_to_bus((dev), (addr), PCI_REGION_MEM)
#define pci_mem_to_phys(dev, addr) \
	pci_bus_to_phys((dev), (addr), PCI_REGION_MEM)
#define pci_phys_to_io(dev, addr)  pci_phys_to_bus((dev), (addr), PCI_REGION_IO)
#define pci_io_to_phys(dev, addr)  pci_bus_to_phys((dev), (addr), PCI_REGION_IO)

#define pci_virt_to_mem(dev, addr) \
	pci_virt_to_bus((dev), (addr), PCI_REGION_MEM)
#define pci_mem_to_virt(dev, addr, len, map_flags) \
	pci_bus_to_virt((dev), (addr), PCI_REGION_MEM, (len), (map_flags))
#define pci_virt_to_io(dev, addr) \
	pci_virt_to_bus((dev), (addr), PCI_REGION_IO)
#define pci_io_to_virt(dev, addr, len, map_flags) \
	pci_bus_to_virt((dev), (addr), PCI_REGION_IO, (len), (map_flags))

/* For driver model these are defined in macros in pci_compat.c */
extern int pci_hose_read_config_byte(struct pci_controller *hose,
				     pci_dev_t dev, int where, u8 *val);
extern int pci_hose_read_config_word(struct pci_controller *hose,
				     pci_dev_t dev, int where, u16 *val);
extern int pci_hose_read_config_dword(struct pci_controller *hose,
				      pci_dev_t dev, int where, u32 *val);
extern int pci_hose_write_config_byte(struct pci_controller *hose,
				      pci_dev_t dev, int where, u8 val);
extern int pci_hose_write_config_word(struct pci_controller *hose,
				      pci_dev_t dev, int where, u16 val);
extern int pci_hose_write_config_dword(struct pci_controller *hose,
				       pci_dev_t dev, int where, u32 val);
#endif

void pciauto_region_init(struct pci_region *res);
void pciauto_region_align(struct pci_region *res, pci_size_t size);
void pciauto_config_init(struct pci_controller *hose);

/**
 * pciauto_region_allocate() - Allocate resources from a PCI resource region
 *
 * Allocates @size bytes from the PCI resource @res. If @supports_64bit is
 * false, the result will be guaranteed to fit in 32 bits.
 *
 * @res:		PCI region to allocate from
 * @size:		Amount of bytes to allocate
 * @bar:		Returns the PCI bus address of the allocated resource
 * @supports_64bit:	Whether to allow allocations above the 32-bit boundary
 * Return: 0 if successful, -1 on failure
 */
int pciauto_region_allocate(struct pci_region *res, pci_size_t size,
			    pci_addr_t *bar, bool supports_64bit);
int pci_skip_dev(struct pci_controller *hose, pci_dev_t dev);

#if defined(CONFIG_DM_PCI_COMPAT)
extern int pci_hose_read_config_byte_via_dword(struct pci_controller *hose,
					       pci_dev_t dev, int where, u8 *val);
extern int pci_hose_read_config_word_via_dword(struct pci_controller *hose,
					       pci_dev_t dev, int where, u16 *val);
extern int pci_hose_write_config_byte_via_dword(struct pci_controller *hose,
						pci_dev_t dev, int where, u8 val);
extern int pci_hose_write_config_word_via_dword(struct pci_controller *hose,
						pci_dev_t dev, int where, u16 val);

extern void *pci_map_bar(pci_dev_t pdev, int bar, int flags);
extern void pci_register_hose(struct pci_controller* hose);
extern struct pci_controller* pci_bus_to_hose(int bus);
extern struct pci_controller *find_hose_by_cfg_addr(void *cfg_addr);
extern struct pci_controller *pci_get_hose_head(void);

extern int pci_hose_scan(struct pci_controller *hose);
extern int pci_hose_scan_bus(struct pci_controller *hose, int bus);

extern void pciauto_setup_device(struct pci_controller *hose,
				 pci_dev_t dev, int bars_num,
				 struct pci_region *mem,
				 struct pci_region *prefetch,
				 struct pci_region *io);
extern void pciauto_prescan_setup_bridge(struct pci_controller *hose,
				 pci_dev_t dev, int sub_bus);
extern void pciauto_postscan_setup_bridge(struct pci_controller *hose,
				 pci_dev_t dev, int sub_bus);
extern int pciauto_config_device(struct pci_controller *hose, pci_dev_t dev);

extern pci_dev_t pci_find_device (unsigned int vendor, unsigned int device, int index);
extern pci_dev_t pci_find_devices (struct pci_device_id *ids, int index);
pci_dev_t pci_find_class(unsigned int find_class, int index);

extern int pci_hose_find_capability(struct pci_controller *hose, pci_dev_t dev,
				    int cap);
extern int pci_hose_find_cap_start(struct pci_controller *hose, pci_dev_t dev,
				   u8 hdr_type);
extern int pci_find_cap(struct pci_controller *hose, pci_dev_t dev, int pos,
			int cap);

int pci_find_next_ext_capability(struct pci_controller *hose,
				 pci_dev_t dev, int start, int cap);
int pci_hose_find_ext_capability(struct pci_controller *hose,
				 pci_dev_t dev, int cap);

#endif /* defined(CONFIG_DM_PCI_COMPAT) */

const char * pci_class_str(u8 class);
int pci_last_busno(void);

#ifdef CONFIG_MPC85xx
extern void pci_mpc85xx_init (struct pci_controller *hose);
#endif

/**
 * pci_write_bar32() - Write the address of a BAR including control bits
 *
 * This writes a raw address (with control bits) to a bar. This can be used
 * with devices which require hard-coded addresses, not part of the normal
 * PCI enumeration process.
 *
 * This is only available if CONFIG_DM_PCI_COMPAT is enabled
 *
 * @hose:	PCI hose to use
 * @dev:	PCI device to update
 * @barnum:	BAR number (0-5)
 * @addr:	BAR address with control bits
 */
void pci_write_bar32(struct pci_controller *hose, pci_dev_t dev, int barnum,
		     u32 addr);

/**
 * pci_read_bar32() - read the address of a bar
 *
 * This is only available if CONFIG_DM_PCI_COMPAT is enabled
 *
 * @hose:	PCI hose to use
 * @dev:	PCI device to inspect
 * @barnum:	BAR number (0-5)
 * Return: address of the bar, masking out any control bits
 * */
u32 pci_read_bar32(struct pci_controller *hose, pci_dev_t dev, int barnum);

/**
 * pci_hose_find_devices() - Find devices by vendor/device ID
 *
 * This is only available if CONFIG_DM_PCI_COMPAT is enabled
 *
 * @hose:	PCI hose to search
 * @busnum:	Bus number to search
 * @ids:	PCI vendor/device IDs to look for, terminated by 0, 0 record
 * @indexp:	Pointer to device index to find. To find the first matching
 *		device, pass 0; to find the second, pass 1, etc. This
 *		parameter is decremented for each non-matching device so
 *		can be called repeatedly.
 */
pci_dev_t pci_hose_find_devices(struct pci_controller *hose, int busnum,
				struct pci_device_id *ids, int *indexp);

/* Access sizes for PCI reads and writes */
enum pci_size_t {
	PCI_SIZE_8,
	PCI_SIZE_16,
	PCI_SIZE_32,
};

struct udevice;

/**
 * struct pci_child_plat - information stored about each PCI device
 *
 * Every device on a PCI bus has this per-child data.
 *
 * It can be accessed using dev_get_parent_plat(dev) if dev->parent is a
 * PCI bus (i.e. UCLASS_PCI)
 *
 * @devfn:	Encoded device and function index - see PCI_DEVFN()
 * @vendor:	PCI vendor ID (see pci_ids.h)
 * @device:	PCI device ID (see pci_ids.h)
 * @class:	PCI class, 3 bytes: (base, sub, prog-if)
 * @is_virtfn:	True for Virtual Function device
 * @pfdev:	Handle to Physical Function device
 * @virtid:	Virtual Function Index
 */
struct pci_child_plat {
	int devfn;
	unsigned short vendor;
	unsigned short device;
	unsigned int class;

	/* Variables for CONFIG_PCI_SRIOV */
	bool is_virtfn;
	struct udevice *pfdev;
	int virtid;
};

/* PCI bus operations */
struct dm_pci_ops {
	/**
	 * read_config() - Read a PCI configuration value
	 *
	 * PCI buses must support reading and writing configuration values
	 * so that the bus can be scanned and its devices configured.
	 *
	 * Normally PCI_BUS(@bdf) is the same as @dev_seq(bus), but not always.
	 * If bridges exist it is possible to use the top-level bus to
	 * access a sub-bus. In that case @bus will be the top-level bus
	 * and PCI_BUS(bdf) will be a different (higher) value
	 *
	 * @bus:	Bus to read from
	 * @bdf:	Bus, device and function to read
	 * @offset:	Byte offset within the device's configuration space
	 * @valuep:	Place to put the returned value
	 * @size:	Access size
	 * @return 0 if OK, -ve on error
	 */
	int (*read_config)(const struct udevice *bus, pci_dev_t bdf,
			   uint offset, ulong *valuep, enum pci_size_t size);
	/**
	 * write_config() - Write a PCI configuration value
	 *
	 * @bus:	Bus to write to
	 * @bdf:	Bus, device and function to write
	 * @offset:	Byte offset within the device's configuration space
	 * @value:	Value to write
	 * @size:	Access size
	 * @return 0 if OK, -ve on error
	 */
	int (*write_config)(struct udevice *bus, pci_dev_t bdf, uint offset,
			    ulong value, enum pci_size_t size);
};

/* Get access to a PCI bus' operations */
#define pci_get_ops(dev)	((struct dm_pci_ops *)(dev)->driver->ops)

/**
 * dm_pci_get_bdf() - Get the BDF value for a device
 *
 * @dev:	Device to check
 * Return: bus/device/function value (see PCI_BDF())
 */
pci_dev_t dm_pci_get_bdf(const struct udevice *dev);

/**
 * pci_bind_bus_devices() - scan a PCI bus and bind devices
 *
 * Scan a PCI bus looking for devices. Bind each one that is found. If
 * devices are already bound that match the scanned devices, just update the
 * child data so that the device can be used correctly (this happens when
 * the device tree describes devices we expect to see on the bus).
 *
 * Devices that are bound in this way will use a generic PCI driver which
 * does nothing. The device can still be accessed but will not provide any
 * driver interface.
 *
 * @bus:	Bus containing devices to bind
 * Return: 0 if OK, -ve on error
 */
int pci_bind_bus_devices(struct udevice *bus);

/**
 * pci_auto_config_devices() - configure bus devices ready for use
 *
 * This works through all devices on a bus by scanning the driver model
 * data structures (normally these have been set up by pci_bind_bus_devices()
 * earlier).
 *
 * Space is allocated for each PCI base address register (BAR) so that the
 * devices are mapped into memory and I/O space ready for use.
 *
 * @bus:	Bus containing devices to bind
 * Return: 0 if OK, -ve on error
 */
int pci_auto_config_devices(struct udevice *bus);

/**
 * dm_pci_bus_find_bdf() - Find a device given its PCI bus address
 *
 * @bdf:	PCI device address: bus, device and function -see PCI_BDF()
 * @devp:	Returns the device for this address, if found
 * Return: 0 if OK, -ENODEV if not found
 */
int dm_pci_bus_find_bdf(pci_dev_t bdf, struct udevice **devp);

/**
 * pci_bus_find_devfn() - Find a device on a bus
 *
 * @find_devfn:		PCI device address (device and function only)
 * @devp:	Returns the device for this address, if found
 * Return: 0 if OK, -ENODEV if not found
 */
int pci_bus_find_devfn(const struct udevice *bus, pci_dev_t find_devfn,
		       struct udevice **devp);

/**
 * pci_find_first_device() - return the first available PCI device
 *
 * This function and pci_find_next_device() allow iteration through all
 * available PCI devices on all buses. Assuming there are any, this will
 * return the first one.
 *
 * @devp:	Set to the first available device, or NULL if no more are left
 *		or we got an error
 * Return: 0 if all is OK, -ve on error (e.g. a bus/bridge failed to probe)
 */
int pci_find_first_device(struct udevice **devp);

/**
 * pci_find_next_device() - return the next available PCI device
 *
 * Finds the next available PCI device after the one supplied, or sets @devp
 * to NULL if there are no more.
 *
 * @devp:	On entry, the last device returned. Set to the next available
 *		device, or NULL if no more are left or we got an error
 * Return: 0 if all is OK, -ve on error (e.g. a bus/bridge failed to probe)
 */
int pci_find_next_device(struct udevice **devp);

/**
 * pci_get_ff() - Returns a mask for the given access size
 *
 * @size:	Access size
 * Return: 0xff for PCI_SIZE_8, 0xffff for PCI_SIZE_16, 0xffffffff for
 * PCI_SIZE_32
 */
int pci_get_ff(enum pci_size_t size);

/**
 * pci_bus_find_devices () - Find devices on a bus
 *
 * @bus:	Bus to search
 * @ids:	PCI vendor/device IDs to look for, terminated by 0, 0 record
 * @indexp:	Pointer to device index to find. To find the first matching
 *		device, pass 0; to find the second, pass 1, etc. This
 *		parameter is decremented for each non-matching device so
 *		can be called repeatedly.
 * @devp:	Returns matching device if found
 * Return: 0 if found, -ENODEV if not
 */
int pci_bus_find_devices(struct udevice *bus, const struct pci_device_id *ids,
			 int *indexp, struct udevice **devp);

/**
 * pci_find_device_id() - Find a device on any bus
 *
 * @ids:	PCI vendor/device IDs to look for, terminated by 0, 0 record
 * @index:	Index number of device to find, 0 for the first match, 1 for
 *		the second, etc.
 * @devp:	Returns matching device if found
 * Return: 0 if found, -ENODEV if not
 */
int pci_find_device_id(const struct pci_device_id *ids, int index,
		       struct udevice **devp);

/**
 * dm_pci_hose_probe_bus() - probe a subordinate bus, scanning it for devices
 *
 * This probes the given bus which causes it to be scanned for devices. The
 * devices will be bound but not probed.
 *
 * @hose specifies the PCI hose that will be used for the scan. This is
 * always a top-level bus with uclass UCLASS_PCI. The bus to scan is
 * in @bdf, and is a subordinate bus reachable from @hose.
 *
 * @hose:	PCI hose to scan
 * @bdf:	PCI bus address to scan (PCI_BUS(bdf) is the bus number)
 * Return: 0 if OK, -ve on error
 */
int dm_pci_hose_probe_bus(struct udevice *bus);

/**
 * pci_bus_read_config() - Read a configuration value from a device
 *
 * TODO(sjg@chromium.org): We should be able to pass just a device and have
 * it do the right thing. It would be good to have that function also.
 *
 * @bus:	Bus to read from
 * @bdf:	PCI device address: bus, device and function -see PCI_BDF()
 * @offset:	Register offset to read
 * @valuep:	Place to put the returned value
 * @size:	Access size
 * Return: 0 if OK, -ve on error
 */
int pci_bus_read_config(const struct udevice *bus, pci_dev_t bdf, int offset,
			unsigned long *valuep, enum pci_size_t size);

/**
 * pci_bus_write_config() - Write a configuration value to a device
 *
 * @bus:	Bus to write from
 * @bdf:	PCI device address: bus, device and function -see PCI_BDF()
 * @offset:	Register offset to write
 * @value:	Value to write
 * @size:	Access size
 * Return: 0 if OK, -ve on error
 */
int pci_bus_write_config(struct udevice *bus, pci_dev_t bdf, int offset,
			 unsigned long value, enum pci_size_t size);

/**
 * pci_bus_clrset_config32() - Update a configuration value for a device
 *
 * The register at @offset is updated to (oldvalue & ~clr) | set.
 *
 * @bus:	Bus to access
 * @bdf:	PCI device address: bus, device and function -see PCI_BDF()
 * @offset:	Register offset to update
 * @clr:	Bits to clear
 * @set:	Bits to set
 * Return: 0 if OK, -ve on error
 */
int pci_bus_clrset_config32(struct udevice *bus, pci_dev_t bdf, int offset,
			    u32 clr, u32 set);

/**
 * Driver model PCI config access functions. Use these in preference to others
 * when you have a valid device
 */
int dm_pci_read_config(const struct udevice *dev, int offset,
		       unsigned long *valuep, enum pci_size_t size);

int dm_pci_read_config8(const struct udevice *dev, int offset, u8 *valuep);
int dm_pci_read_config16(const struct udevice *dev, int offset, u16 *valuep);
int dm_pci_read_config32(const struct udevice *dev, int offset, u32 *valuep);

int dm_pci_write_config(struct udevice *dev, int offset, unsigned long value,
			enum pci_size_t size);

int dm_pci_write_config8(struct udevice *dev, int offset, u8 value);
int dm_pci_write_config16(struct udevice *dev, int offset, u16 value);
int dm_pci_write_config32(struct udevice *dev, int offset, u32 value);

/**
 * These permit convenient read/modify/write on PCI configuration. The
 * register is updated to (oldvalue & ~clr) | set.
 */
int dm_pci_clrset_config8(struct udevice *dev, int offset, u32 clr, u32 set);
int dm_pci_clrset_config16(struct udevice *dev, int offset, u32 clr, u32 set);
int dm_pci_clrset_config32(struct udevice *dev, int offset, u32 clr, u32 set);

/*
 * The following functions provide access to the above without needing the
 * size parameter. We are trying to encourage the use of the 8/16/32-style
 * functions, rather than byte/word/dword. But both are supported.
 */
int pci_write_config32(pci_dev_t pcidev, int offset, u32 value);
int pci_write_config16(pci_dev_t pcidev, int offset, u16 value);
int pci_write_config8(pci_dev_t pcidev, int offset, u8 value);
int pci_read_config32(pci_dev_t pcidev, int offset, u32 *valuep);
int pci_read_config16(pci_dev_t pcidev, int offset, u16 *valuep);
int pci_read_config8(pci_dev_t pcidev, int offset, u8 *valuep);

/**
 * pci_generic_mmap_write_config() - Generic helper for writing to
 * memory-mapped PCI configuration space.
 * @bus: Pointer to the PCI bus
 * @addr_f: Callback for calculating the config space address
 * @bdf: Identifies the PCI device to access
 * @offset: The offset into the device's configuration space
 * @value: The value to write
 * @size: Indicates the size of access to perform
 *
 * Write the value @value of size @size from offset @offset within the
 * configuration space of the device identified by the bus, device & function
 * numbers in @bdf on the PCI bus @bus. The callback function @addr_f is
 * responsible for calculating the CPU address of the respective configuration
 * space offset.
 *
 * Return: 0 on success, else -EINVAL
 */
int pci_generic_mmap_write_config(
	const struct udevice *bus,
	int (*addr_f)(const struct udevice *bus, pci_dev_t bdf, uint offset,
		      void **addrp),
	pci_dev_t bdf,
	uint offset,
	ulong value,
	enum pci_size_t size);

/**
 * pci_generic_mmap_read_config() - Generic helper for reading from
 * memory-mapped PCI configuration space.
 * @bus: Pointer to the PCI bus
 * @addr_f: Callback for calculating the config space address
 * @bdf: Identifies the PCI device to access
 * @offset: The offset into the device's configuration space
 * @valuep: A pointer at which to store the read value
 * @size: Indicates the size of access to perform
 *
 * Read a value of size @size from offset @offset within the configuration
 * space of the device identified by the bus, device & function numbers in @bdf
 * on the PCI bus @bus. The callback function @addr_f is responsible for
 * calculating the CPU address of the respective configuration space offset.
 *
 * Return: 0 on success, else -EINVAL
 */
int pci_generic_mmap_read_config(
	const struct udevice *bus,
	int (*addr_f)(const struct udevice *bus, pci_dev_t bdf, uint offset,
		      void **addrp),
	pci_dev_t bdf,
	uint offset,
	ulong *valuep,
	enum pci_size_t size);

#if defined(CONFIG_PCI_SRIOV)
/**
 * pci_sriov_init() - Scan Virtual Function devices
 *
 * @pdev:	Physical Function udevice handle
 * @vf_en:	Number of Virtual Function devices to enable
 * Return: 0 on success, -ve on error
 */
int pci_sriov_init(struct udevice *pdev, int vf_en);

/**
 * pci_sriov_get_totalvfs() - Get total available Virtual Function devices
 *
 * @pdev:	Physical Function udevice handle
 * Return: count on success, -ve on error
 */
int pci_sriov_get_totalvfs(struct udevice *pdev);
#endif

#ifdef CONFIG_DM_PCI_COMPAT
/* Compatibility with old naming */
static inline int pci_write_config_dword(pci_dev_t pcidev, int offset,
					 u32 value)
{
	return pci_write_config32(pcidev, offset, value);
}

/* Compatibility with old naming */
static inline int pci_write_config_word(pci_dev_t pcidev, int offset,
					u16 value)
{
	return pci_write_config16(pcidev, offset, value);
}

/* Compatibility with old naming */
static inline int pci_write_config_byte(pci_dev_t pcidev, int offset,
					u8 value)
{
	return pci_write_config8(pcidev, offset, value);
}

/* Compatibility with old naming */
static inline int pci_read_config_dword(pci_dev_t pcidev, int offset,
					u32 *valuep)
{
	return pci_read_config32(pcidev, offset, valuep);
}

/* Compatibility with old naming */
static inline int pci_read_config_word(pci_dev_t pcidev, int offset,
				       u16 *valuep)
{
	return pci_read_config16(pcidev, offset, valuep);
}

/* Compatibility with old naming */
static inline int pci_read_config_byte(pci_dev_t pcidev, int offset,
				       u8 *valuep)
{
	return pci_read_config8(pcidev, offset, valuep);
}
#endif /* CONFIG_DM_PCI_COMPAT */

/**
 * dm_pciauto_config_device() - configure a device ready for use
 *
 * Space is allocated for each PCI base address register (BAR) so that the
 * devices are mapped into memory and I/O space ready for use.
 *
 * @dev:	Device to configure
 * Return: 0 if OK, -ve on error
 */
int dm_pciauto_config_device(struct udevice *dev);

/**
 * pci_conv_32_to_size() - convert a 32-bit read value to the given size
 *
 * Some PCI buses must always perform 32-bit reads. The data must then be
 * shifted and masked to reflect the required access size and offset. This
 * function performs this transformation.
 *
 * @value:	Value to transform (32-bit value read from @offset & ~3)
 * @offset:	Register offset that was read
 * @size:	Required size of the result
 * Return: the value that would have been obtained if the read had been
 * performed at the given offset with the correct size
 */
ulong pci_conv_32_to_size(ulong value, uint offset, enum pci_size_t size);

/**
 * pci_conv_size_to_32() - update a 32-bit value to prepare for a write
 *
 * Some PCI buses must always perform 32-bit writes. To emulate a smaller
 * write the old 32-bit data must be read, updated with the required new data
 * and written back as a 32-bit value. This function performs the
 * transformation from the old value to the new value.
 *
 * @value:	Value to transform (32-bit value read from @offset & ~3)
 * @offset:	Register offset that should be written
 * @size:	Required size of the write
 * Return: the value that should be written as a 32-bit access to @offset & ~3.
 */
ulong pci_conv_size_to_32(ulong old, ulong value, uint offset,
			  enum pci_size_t size);

/**
 * pci_get_controller() - obtain the controller to use for a bus
 *
 * @dev:	Device to check
 * Return: pointer to the controller device for this bus
 */
struct udevice *pci_get_controller(struct udevice *dev);

/**
 * pci_get_regions() - obtain pointers to all the region types
 *
 * @dev:	Device to check
 * @iop:	Returns a pointer to the I/O region, or NULL if none
 * @memp:	Returns a pointer to the memory region, or NULL if none
 * @prefp:	Returns a pointer to the pre-fetch region, or NULL if none
 * Return: the number of non-NULL regions returned, normally 3
 */
int pci_get_regions(struct udevice *dev, struct pci_region **iop,
		    struct pci_region **memp, struct pci_region **prefp);
int
pci_get_dma_regions(struct udevice *dev, struct pci_region *memp, int index);
/**
 * dm_pci_write_bar32() - Write the address of a BAR
 *
 * This writes a raw address to a bar
 *
 * @dev:	PCI device to update
 * @barnum:	BAR number (0-5)
 * @addr:	BAR address
 */
void dm_pci_write_bar32(struct udevice *dev, int barnum, u32 addr);

/**
 * dm_pci_read_bar32() - read a base address register from a device
 *
 * @dev:	Device to check
 * @barnum:	Bar number to read (numbered from 0)
 * @return: value of BAR
 */
u32 dm_pci_read_bar32(const struct udevice *dev, int barnum);

/**
 * dm_pci_bus_to_phys() - convert a PCI bus address range to a physical address
 *
 * @dev:	Device containing the PCI address
 * @addr:	PCI address to convert
 * @len:	Length of the address range
 * @mask:	Mask to match flags for the region type
 * @flags:	Flags for the region type (PCI_REGION_...)
 * Return: physical address corresponding to that PCI bus address
 */
phys_addr_t dm_pci_bus_to_phys(struct udevice *dev, pci_addr_t addr, size_t len,
			       unsigned long mask, unsigned long flags);

/**
 * dm_pci_phys_to_bus() - convert a physical address to a PCI bus address
 *
 * @dev:	Device containing the bus address
 * @addr:	Physical address to convert
 * @len:	Length of the address range
 * @mask:	Mask to match flags for the region type
 * @flags:	Flags for the region type (PCI_REGION_...)
 * Return: PCI bus address corresponding to that physical address
 */
pci_addr_t dm_pci_phys_to_bus(struct udevice *dev, phys_addr_t addr, size_t len,
			      unsigned long mask, unsigned long flags);

/**
 * dm_pci_map_bar() - get a virtual address associated with a BAR region
 *
 * Looks up a base address register and finds the physical memory address
 * that corresponds to it.
 * Can be used for 32b BARs 0-5 on type 0 functions and for 32b BARs 0-1 on
 * type 1 functions.
 * Can also be used on type 0 functions that support Enhanced Allocation for
 * 32b/64b BARs.  Note that duplicate BEI entries are not supported.
 * Can also be used on 64b bars on type 0 functions.
 *
 * @dev:	Device to check
 * @bar:	Bar register offset (PCI_BASE_ADDRESS_...)
 * @offset:     Offset from the base to map
 * @len:        Length to map
 * @mask:       Mask to match flags for the region type
 * @flags:	Flags for the region type (PCI_REGION_...)
 * @return: pointer to the virtual address to use or 0 on error
 */
void *dm_pci_map_bar(struct udevice *dev, int bar, size_t offset, size_t len,
		     unsigned long mask, unsigned long flags);

/**
 * dm_pci_find_next_capability() - find a capability starting from an offset
 *
 * Tell if a device supports a given PCI capability. Returns the
 * address of the requested capability structure within the device's
 * PCI configuration space or 0 in case the device does not support it.
 *
 * Possible values for @cap:
 *
 *  %PCI_CAP_ID_MSI	Message Signalled Interrupts
 *  %PCI_CAP_ID_PCIX	PCI-X
 *  %PCI_CAP_ID_EXP	PCI Express
 *  %PCI_CAP_ID_MSIX	MSI-X
 *
 * See PCI_CAP_ID_xxx for the complete capability ID codes.
 *
 * @dev:	PCI device to query
 * @start:	offset to start from
 * @cap:	capability code
 * @return:	capability address or 0 if not supported
 */
int dm_pci_find_next_capability(struct udevice *dev, u8 start, int cap);

/**
 * dm_pci_find_capability() - find a capability
 *
 * Tell if a device supports a given PCI capability. Returns the
 * address of the requested capability structure within the device's
 * PCI configuration space or 0 in case the device does not support it.
 *
 * Possible values for @cap:
 *
 *  %PCI_CAP_ID_MSI	Message Signalled Interrupts
 *  %PCI_CAP_ID_PCIX	PCI-X
 *  %PCI_CAP_ID_EXP	PCI Express
 *  %PCI_CAP_ID_MSIX	MSI-X
 *
 * See PCI_CAP_ID_xxx for the complete capability ID codes.
 *
 * @dev:	PCI device to query
 * @cap:	capability code
 * @return:	capability address or 0 if not supported
 */
int dm_pci_find_capability(struct udevice *dev, int cap);

/**
 * dm_pci_find_next_ext_capability() - find an extended capability
 *				       starting from an offset
 *
 * Tell if a device supports a given PCI express extended capability.
 * Returns the address of the requested extended capability structure
 * within the device's PCI configuration space or 0 in case the device
 * does not support it.
 *
 * Possible values for @cap:
 *
 *  %PCI_EXT_CAP_ID_ERR	Advanced Error Reporting
 *  %PCI_EXT_CAP_ID_VC	Virtual Channel
 *  %PCI_EXT_CAP_ID_DSN	Device Serial Number
 *  %PCI_EXT_CAP_ID_PWR	Power Budgeting
 *
 * See PCI_EXT_CAP_ID_xxx for the complete extended capability ID codes.
 *
 * @dev:	PCI device to query
 * @start:	offset to start from
 * @cap:	extended capability code
 * @return:	extended capability address or 0 if not supported
 */
int dm_pci_find_next_ext_capability(struct udevice *dev, int start, int cap);

/**
 * dm_pci_find_ext_capability() - find an extended capability
 *
 * Tell if a device supports a given PCI express extended capability.
 * Returns the address of the requested extended capability structure
 * within the device's PCI configuration space or 0 in case the device
 * does not support it.
 *
 * Possible values for @cap:
 *
 *  %PCI_EXT_CAP_ID_ERR	Advanced Error Reporting
 *  %PCI_EXT_CAP_ID_VC	Virtual Channel
 *  %PCI_EXT_CAP_ID_DSN	Device Serial Number
 *  %PCI_EXT_CAP_ID_PWR	Power Budgeting
 *
 * See PCI_EXT_CAP_ID_xxx for the complete extended capability ID codes.
 *
 * @dev:	PCI device to query
 * @cap:	extended capability code
 * @return:	extended capability address or 0 if not supported
 */
int dm_pci_find_ext_capability(struct udevice *dev, int cap);

/**
 * dm_pci_flr() - Perform FLR if the device suppoorts it
 *
 * @dev:	PCI device to reset
 * @return:	0 if OK, -ENOENT if FLR is not supported by dev
 */
int dm_pci_flr(struct udevice *dev);

#define dm_pci_virt_to_bus(dev, addr, flags) \
	dm_pci_phys_to_bus(dev, (virt_to_phys(addr)), 0, PCI_REGION_TYPE, (flags))
#define dm_pci_bus_to_virt(dev, addr, len, mask, flags, map_flags)	\
({									\
	size_t _len = (len);						\
	phys_addr_t phys_addr = dm_pci_bus_to_phys((dev), (addr), _len,	\
						   (mask), (flags));	\
	map_physmem(phys_addr, _len, (map_flags));			\
})

#define dm_pci_phys_to_mem(dev, addr) \
	dm_pci_phys_to_bus((dev), (addr), 0, PCI_REGION_TYPE, PCI_REGION_MEM)
#define dm_pci_mem_to_phys(dev, addr) \
	dm_pci_bus_to_phys((dev), (addr), 0, PCI_REGION_TYPE, PCI_REGION_MEM)
#define dm_pci_phys_to_io(dev, addr) \
	dm_pci_phys_to_bus((dev), (addr), 0, PCI_REGION_TYPE, PCI_REGION_IO)
#define dm_pci_io_to_phys(dev, addr) \
	dm_pci_bus_to_phys((dev), (addr), 0, PCI_REGION_TYPE, PCI_REGION_IO)

#define dm_pci_virt_to_mem(dev, addr) \
	dm_pci_virt_to_bus((dev), (addr), PCI_REGION_MEM)
#define dm_pci_mem_to_virt(dev, addr, len, map_flags) \
	dm_pci_bus_to_virt((dev), (addr), (len), PCI_REGION_TYPE, \
			   PCI_REGION_MEM, (map_flags))
#define dm_pci_virt_to_io(dev, addr) \
	dm_pci_virt_to_bus((dev), (addr), PCI_REGION_IO)
#define dm_pci_io_to_virt(dev, addr, len, map_flags) \
	dm_pci_bus_to_virt((dev), (addr), (len), PCI_REGION_TYPE, \
			   PCI_REGION_IO, (map_flags))

/**
 * dm_pci_find_device() - find a device by vendor/device ID
 *
 * @vendor:	Vendor ID
 * @device:	Device ID
 * @index:	0 to find the first match, 1 for second, etc.
 * @devp:	Returns pointer to the device, if found
 * Return: 0 if found, -ve on error
 */
int dm_pci_find_device(unsigned int vendor, unsigned int device, int index,
		       struct udevice **devp);

/**
 * dm_pci_find_class() - find a device by class
 *
 * @find_class: 3-byte (24-bit) class value to find
 * @index:	0 to find the first match, 1 for second, etc.
 * @devp:	Returns pointer to the device, if found
 * Return: 0 if found, -ve on error
 */
int dm_pci_find_class(uint find_class, int index, struct udevice **devp);

/**
 * struct pci_emul_uc_priv - holds info about an emulator device
 *
 * There is always at most one emulator per client
 *
 * @client: Client device if any, else NULL
 */
struct pci_emul_uc_priv {
	struct udevice *client;
};

/**
 * struct dm_pci_emul_ops - PCI device emulator operations
 */
struct dm_pci_emul_ops {
	/**
	 * read_config() - Read a PCI configuration value
	 *
	 * @dev:	Emulated device to read from
	 * @offset:	Byte offset within the device's configuration space
	 * @valuep:	Place to put the returned value
	 * @size:	Access size
	 * @return 0 if OK, -ve on error
	 */
	int (*read_config)(const struct udevice *dev, uint offset,
			   ulong *valuep, enum pci_size_t size);
	/**
	 * write_config() - Write a PCI configuration value
	 *
	 * @dev:	Emulated device to write to
	 * @offset:	Byte offset within the device's configuration space
	 * @value:	Value to write
	 * @size:	Access size
	 * @return 0 if OK, -ve on error
	 */
	int (*write_config)(struct udevice *dev, uint offset, ulong value,
			    enum pci_size_t size);
	/**
	 * read_io() - Read a PCI I/O value
	 *
	 * @dev:	Emulated device to read from
	 * @addr:	I/O address to read
	 * @valuep:	Place to put the returned value
	 * @size:	Access size
	 * @return 0 if OK, -ENOENT if @addr is not mapped by this device,
	 *		other -ve value on error
	 */
	int (*read_io)(struct udevice *dev, unsigned int addr, ulong *valuep,
		       enum pci_size_t size);
	/**
	 * write_io() - Write a PCI I/O value
	 *
	 * @dev:	Emulated device to write from
	 * @addr:	I/O address to write
	 * @value:	Value to write
	 * @size:	Access size
	 * @return 0 if OK, -ENOENT if @addr is not mapped by this device,
	 *		other -ve value on error
	 */
	int (*write_io)(struct udevice *dev, unsigned int addr,
			ulong value, enum pci_size_t size);
	/**
	 * map_physmem() - Map a device into sandbox memory
	 *
	 * @dev:	Emulated device to map
	 * @addr:	Memory address, normally corresponding to a PCI BAR.
	 *		The device should have been configured to have a BAR
	 *		at this address.
	 * @lenp:	On entry, the size of the area to map, On exit it is
	 *		updated to the size actually mapped, which may be less
	 *		if the device has less space
	 * @ptrp:	Returns a pointer to the mapped address. The device's
	 *		space can be accessed as @lenp bytes starting here
	 * @return 0 if OK, -ENOENT if @addr is not mapped by this device,
	 *		other -ve value on error
	 */
	int (*map_physmem)(struct udevice *dev, phys_addr_t addr,
			   unsigned long *lenp, void **ptrp);
	/**
	 * unmap_physmem() - undo a memory mapping
	 *
	 * This must be called after map_physmem() to undo the mapping.
	 * Some devices can use this to check what has been written into
	 * their mapped memory and perform an operations they require on it.
	 * In this way, map/unmap can be used as a sort of handshake between
	 * the emulated device and its users.
	 *
	 * @dev:	Emuated device to unmap
	 * @vaddr:	Mapped memory address, as passed to map_physmem()
	 * @len:	Size of area mapped, as returned by map_physmem()
	 * @return 0 if OK, -ve on error
	 */
	int (*unmap_physmem)(struct udevice *dev, const void *vaddr,
			     unsigned long len);
};

/* Get access to a PCI device emulator's operations */
#define pci_get_emul_ops(dev)	((struct dm_pci_emul_ops *)(dev)->driver->ops)

/**
 * sandbox_pci_get_emul() - Get the emulation device for a PCI device
 *
 * Searches for a suitable emulator for the given PCI bus device
 *
 * @bus:	PCI bus to search
 * @find_devfn:	PCI device and function address (PCI_DEVFN())
 * @containerp:	Returns container device if found
 * @emulp:	Returns emulated device if found
 * Return: 0 if found, -ENODEV if not found
 */
int sandbox_pci_get_emul(const struct udevice *bus, pci_dev_t find_devfn,
			 struct udevice **containerp, struct udevice **emulp);

/**
 * sandbox_pci_get_client() - Find the client for an emulation device
 *
 * @emul:	Emulation device to check
 * @devp:	Returns the client device emulated by this device
 * Return: 0 if OK, -ENOENT if the device has no client yet
 */
int sandbox_pci_get_client(struct udevice *emul, struct udevice **devp);

/**
 * board_pci_fixup_dev() - Board callback for PCI device fixups
 *
 * @bus:	PCI bus
 * @dev:	PCI device
 */
extern void board_pci_fixup_dev(struct udevice *bus, struct udevice *dev);

/**
 * PCI_DEVICE - macro used to describe a specific pci device
 * @vend: the 16 bit PCI Vendor ID
 * @dev: the 16 bit PCI Device ID
 *
 * This macro is used to create a struct pci_device_id that matches a
 * specific device.  The subvendor and subdevice fields will be set to
 * PCI_ANY_ID.
 */
#define PCI_DEVICE(vend, dev) \
	.vendor = (vend), .device = (dev), \
	.subvendor = PCI_ANY_ID, .subdevice = PCI_ANY_ID

/**
 * PCI_DEVICE_SUB - macro used to describe a specific pci device with subsystem
 * @vend: the 16 bit PCI Vendor ID
 * @dev: the 16 bit PCI Device ID
 * @subvend: the 16 bit PCI Subvendor ID
 * @subdev: the 16 bit PCI Subdevice ID
 *
 * This macro is used to create a struct pci_device_id that matches a
 * specific device with subsystem information.
 */
#define PCI_DEVICE_SUB(vend, dev, subvend, subdev) \
	.vendor = (vend), .device = (dev), \
	.subvendor = (subvend), .subdevice = (subdev)

/**
 * PCI_DEVICE_CLASS - macro used to describe a specific pci device class
 * @dev_class: the class, subclass, prog-if triple for this device
 * @dev_class_mask: the class mask for this device
 *
 * This macro is used to create a struct pci_device_id that matches a
 * specific PCI class.  The vendor, device, subvendor, and subdevice
 * fields will be set to PCI_ANY_ID.
 */
#define PCI_DEVICE_CLASS(dev_class, dev_class_mask) \
	.class = (dev_class), .class_mask = (dev_class_mask), \
	.vendor = PCI_ANY_ID, .device = PCI_ANY_ID, \
	.subvendor = PCI_ANY_ID, .subdevice = PCI_ANY_ID

/**
 * PCI_VDEVICE - macro used to describe a specific pci device in short form
 * @vend: the vendor name
 * @dev: the 16 bit PCI Device ID
 *
 * This macro is used to create a struct pci_device_id that matches a
 * specific PCI device.  The subvendor, and subdevice fields will be set
 * to PCI_ANY_ID. The macro allows the next field to follow as the device
 * private data.
 */

#define PCI_VDEVICE(vend, dev) \
	.vendor = PCI_VENDOR_ID_##vend, .device = (dev), \
	.subvendor = PCI_ANY_ID, .subdevice = PCI_ANY_ID, 0, 0

/**
 * struct pci_driver_entry - Matches a driver to its pci_device_id list
 * @driver: Driver to use
 * @match: List of match records for this driver, terminated by {}
 */
struct pci_driver_entry {
	struct driver *driver;
	const struct pci_device_id *match;
};

#define U_BOOT_PCI_DEVICE(__name, __match)				\
	ll_entry_declare(struct pci_driver_entry, __name, pci_driver_entry) = {\
		.driver = llsym(struct driver, __name, driver), \
		.match = __match, \
		}

#endif /* __ASSEMBLY__ */
#endif /* _PCI_H */
