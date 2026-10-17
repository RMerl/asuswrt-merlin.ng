/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Writing IntelGraphicsMem table for ACPI
 *
 * Copyright 2019 Google LLC
 * Modified from coreboot src/soc/intel/gma/opregion.h
 */

#ifndef _ASM_INTEL_OPREGION_H_
#define _ASM_INTEL_OPREGION_H_

#define VBT_SIGNATURE		0x54425624

/* IGD PCI Configuration register */
#define ASLS		0xfc		/* OpRegion Base */
#define SWSCI		0xe8		/* SWSCI Register */
#define SWSMISCI	0xe0		/* SWSMISCI Register */
#define GSSCIE		BIT(0)		/* SCI Event trigger */
#define SMISCISEL	BIT(15)		/* Select SMI or SCI event source */

/* mailbox 0: header */
struct __packed opregion_header {
	u8	signature[16];	    /* Offset 0    OpRegion signature */
	u32	size;		    /* Offset 16   OpRegion size */
	u32	version;	    /* Offset 20   OpRegion structure version */
	u8	sbios_version[32];  /* Offset 24   System BIOS build version */
	u8	vbios_version[16];  /* Offset 56   Video BIOS build version */
	u8	driver_version[16]; /* Offset 72   Graphic drvr build version */
	u32	mailboxes;	    /* Offset 88   Mailboxes supported */
	u32	dmod;		    /* Offset 92   Driver Model */
	u32	pcon;		    /* Offset 96   Platform Capabilities */
	u16	dver[16];	    /* Offset 100  GOP Version */
	u8	reserved[124];	    /* Offset 132  Reserved */
};

#define IGD_OPREGION_SIGNATURE "IntelGraphicsMem"
#define IGD_OPREGION_VERSION  2

#define IGD_MBOX1	BIT(0)
#define IGD_MBOX2	BIT(1)
#define IGD_MBOX3	BIT(2)
#define IGD_MBOX4	BIT(3)
#define IGD_MBOX5	BIT(4)

#define MAILBOXES_MOBILE  (IGD_MBOX1 | IGD_MBOX2 | IGD_MBOX3 | \
			   IGD_MBOX4 | IGD_MBOX5)
#define MAILBOXES_DESKTOP (IGD_MBOX2 | IGD_MBOX4)

#define SBIOS_VERSION_SIZE 32

/* mailbox 1: public ACPI methods */
struct __packed opregion_mailbox1 {
	u32	drdy;	/* Offset 0    Driver readiness */
	u32	csts;	/* Offset 4    Status */
	u32	cevt;	/* Offset 8    Current event */
	u8	reserved[20];	/* Offset 12   Reserved */
	u32	didl;	/* Offset 32   Supported display device 1 */
	u32	ddl2;	/* Offset 36   Supported display device 2 */
	u32	ddl3;	/* Offset 40   Supported display device 3 */
	u32	ddl4;	/* Offset 44   Supported display device 4 */
	u32	ddl5;	/* Offset 48   Supported display device 5 */
	u32	ddl6;	/* Offset 52   Supported display device 6 */
	u32	ddl7;	/* Offset 56   Supported display device 7 */
	u32	ddl8;	/* Offset 60   Supported display device 8 */
	u32	cpdl;	/* Offset 64   Currently present display device 1 */
	u32	cpl2;	/* Offset 68   Currently present display device 2 */
	u32	cpl3;	/* Offset 72   Currently present display device 3 */
	u32	cpl4;	/* Offset 76   Currently present display device 4 */
	u32	cpl5;	/* Offset 80   Currently present display device 5 */
	u32	cpl6;	/* Offset 84   Currently present display device 6 */
	u32	cpl7;	/* Offset 88   Currently present display device 7 */
	u32	cpl8;	/* Offset 92   Currently present display device 8 */
	u32	cadl;	/* Offset 96   Currently active display device 1 */
	u32	cal2;	/* Offset 100  Currently active display device 2 */
	u32	cal3;	/* Offset 104  Currently active display device 3 */
	u32	cal4;	/* Offset 108  Currently active display device 4 */
	u32	cal5;	/* Offset 112  Currently active display device 5 */
	u32	cal6;	/* Offset 116  Currently active display device 6 */
	u32	cal7;	/* Offset 120  Currently active display device 7 */
	u32	cal8;	/* Offset 124  Currently active display device 8 */
	u32	nadl;	/* Offset 128  Next active device 1 */
	u32	ndl2;	/* Offset 132  Next active device 2 */
	u32	ndl3;	/* Offset 136  Next active device 3 */
	u32	ndl4;	/* Offset 140  Next active device 4 */
	u32	ndl5;	/* Offset 144  Next active device 5 */
	u32	ndl6;	/* Offset 148  Next active device 6 */
	u32	ndl7;	/* Offset 152  Next active device 7 */
	u32	ndl8;	/* Offset 156  Next active device 8 */
	u32	aslp;	/* Offset 160  ASL sleep timeout */
	u32	tidx;	/* Offset 164  Toggle table index */
	u32	chpd;	/* Offset 168  Current hot plug enable indicator */
	u32	clid;	/* Offset 172  Current lid state indicator */
	u32	cdck;	/* Offset 176  Current docking state indicator */
	u32	sxsw;	/* Offset 180  Display Switch notification on Sx State
			 * resume
			 */
	u32	evts;	/* Offset 184  Events supported by ASL */
	u32	cnot;	/* Offset 188  Current OS Notification */
	u32	nrdy;	/* Offset 192  Reasons for DRDY = 0 */
	u32	ddl9;	/* Offset 196  Extended Supported display device 1 */
	u32	dd10;	/* Offset 200  Extended Supported display device 2 */
	u32	dd11;	/* Offset 204  Extended Supported display device 3 */
	u32	dd12;	/* Offset 208  Extended Supported display device 4 */
	u32	dd13;	/* Offset 212  Extended Supported display device 5 */
	u32	dd14;	/* Offset 216  Extended Supported display device 6 */
	u32	dd15;	/* Offset 220  Extended Supported display device 7 */
	u32	cpl9;	/* Offset 224  Extended Currently present device 1 */
	u32	cp10;	/* Offset 228  Extended Currently present device 2 */
	u32	cp11;	/* Offset 232  Extended Currently present device 3 */
	u32	cp12;	/* Offset 236  Extended Currently present device 4 */
	u32	cp13;	/* Offset 240  Extended Currently present device 5 */
	u32	cp14;	/* Offset 244  Extended Currently present device 6 */
	u32	cp15;	/* Offset 248  Extended Currently present device 7 */
	u8	reserved2[4];	/* Offset 252  Reserved 4 bytes */
};

/* mailbox 2: software sci interface */
struct __packed opregion_mailbox2 {
	u32	scic;	/* Offset 0  Software SCI function number parameters */
	u32	parm;	/* Offset 4  Software SCI function number parameters */
	u32	dslp;	/* Offset 8  Driver sleep timeout */
	u8	reserved[244];	/* Offset 12   Reserved */
};

/* mailbox 3: power conservation */
struct __packed opregion_mailbox3 {
	u32	ardy;		/* Offset 0   Driver readiness */
	u32	aslc;		/* Offset 4   ASLE interrupt command / status */
	u32	tche;		/* Offset 8   Technology enabled indicator */
	u32	alsi;		/* Offset 12  Current ALS illuminance reading */
	u32	bclp;		/* Offset 16  Backlight britness to set */
	u32	pfit;		/* Offset 20  Panel fitting Request */
	u32	cblv;		/* Offset 24  Brightness Current State */
	/* Offset 28  Backlight Brightness Level Duty Cycle Mapping Table */
	u16	bclm[20];
	u32	cpfm;		/* Offset 68  Panel Fitting Current Mode */
	u32	epfm;		/* Offset 72  Enabled Panel Fitting Modes */
	u8	plut[74];	/* Offset 76  Panel Look Up Table */
	/* Offset 150 PWM Frequency and Minimum Brightness */
	u32	pfmb;
	u32	ccdv;		/* Offset 154 Color Correction Default Values */
	u32	pcft;		/* Offset 158 Power Conservation Features */
	u32	srot;		/* Offset 162 Supported Rotation angle */
	u32	iuer;		/* Offset 166 Intel Ultrabook Event Register */
	u64	fdsp;		/* Offset 170 FFS Display Physical address */
	u32	fdss;		/* Offset 178 FFS Display Size */
	u32	stat;		/* Offset 182 State Indicator */
	/*
	 * Offset 186 (Igd opregion offset 0x3BAh)
	 * Physical address of Raw VBT data
	 */
	u64	rvda;
	/* Offset 194 (Igd opregion offset 0x3C2h) Size of Raw VBT data */
	u32	rvds;
	u8	reserved[58];	/* Offset 198 Reserved */
};

#define IGD_BACKLIGHT_BRIGHTNESS 0xff
#define IGD_INITIAL_BRIGHTNESS 0x64

#define IGD_FIELD_VALID		BIT(31)
#define IGD_WORD_FIELD_VALID	BIT(15)
#define IGD_PFIT_STRETCH 6

/* mailbox 4: vbt */
struct __packed opregion_vbt {
	u8 gvd1[6 << 10];
};

/* Mailbox 5: BIOS to Driver Notification Extension */
struct __packed opregion_mailbox5 {
	u32	phed;		/* Offset 7168 Panel Header */
	u8	bddc[256];	/* Offset 7172 Panel EDID */
	u8	reserved[764];	/* Offset 7428 764 bytes */
};

/* IGD OpRegion */
struct __packed igd_opregion {
	struct opregion_header header;
	struct opregion_mailbox1 mailbox1;
	struct opregion_mailbox2 mailbox2;
	struct opregion_mailbox3 mailbox3;
	struct opregion_vbt vbt;
	struct opregion_mailbox5 mailbox5;
};

/* Intel Video BIOS (Option ROM) */
struct __packed optionrom_header {
	u16	signature;
	u8	size;
	u8	reserved[21];
	u16	pcir_offset;
	u16	vbt_offset;
};

#define OPROM_SIGNATURE 0xaa55

struct __packed optionrom_pcir {
	u32	signature;
	u16	vendor;
	u16	device;
	u16	reserved1;
	u16	length;
	u8	revision;
	u8	classcode[3];
	u16	imagelength;
	u16	coderevision;
	u8	codetype;
	u8	indicator;
	u16	reserved2;
};

struct __packed optionrom_vbt {
	u8	hdr_signature[20];
	u16	hdr_version;
	u16	hdr_size;
	u16	hdr_vbt_size;
	u8	hdr_vbt_checksum;
	u8	hdr_reserved;
	u32	hdr_vbt_datablock;
	u32	hdr_aim[4];
	u8	datahdr_signature[16];
	u16	datahdr_version;
	u16	datahdr_size;
	u16	datahdr_datablocksize;
	u8	coreblock_id;
	u16	coreblock_size;
	u16	coreblock_biossize;
	u8	coreblock_biostype;
	u8	coreblock_releasestatus;
	u8	coreblock_hwsupported;
	u8	coreblock_integratedhw;
	u8	coreblock_biosbuild[4];
	u8	coreblock_biossignon[155];
};

/**
 * intel_gma_init_igd_opregion() - Initialise IGD OpRegion
 *
 * This is called from ACPI code and OS drivers
 *
 * Return: 0 if OK, -ve on error
 */
int intel_gma_init_igd_opregion(struct udevice *dev,
				struct igd_opregion *opregion);

#endif /* _ASM_INTEL_OPREGION_H_ */
