/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011  Renesas Solutions Corp.
 */

#ifndef _ASM_CPU_SH7757_H_
#define _ASM_CPU_SH7757_H_

#define CCR		0xFF00001C
#define WTCNT		0xFFCC0000
#define CCR_CACHE_INIT	0x0000090b
#define CACHE_OC_NUM_WAYS	1

#ifndef __ASSEMBLY__		/* put C only stuff in this section */
/* MMU */
struct mmu_regs {
	unsigned int	reserved[4];
	unsigned int	mmucr;
};
#define MMU_BASE	((struct mmu_regs *)0xff000000)

/* Watchdog */
#define WTCSR0		0xffcc0002
#define WRSTCSR_R	0xffcc0003
#define WRSTCSR_W	0xffcc0002
#define WTCSR_PREFIX		0xa500
#define WRSTCSR_PREFIX		0x6900
#define WRSTCSR_WOVF_PREFIX	0x9600

/* SCIF */
#define SCIF0_BASE	0xfe4b0000	/* The real name is SCIF2 */
#define SCIF1_BASE	0xfe4c0000	/* The real name is SCIF3 */
#define SCIF2_BASE	0xfe4d0000	/* The real name is SCIF4 */

/* SerMux */
#define SMR0		0xfe470000

/* TMU0 */
#define TMU_BASE    0xFE430000

/* ETHER, GETHER MAC address */
struct ether_mac_regs {
	unsigned int	reserved[114];
	unsigned int	mahr;
	unsigned int	reserved2;
	unsigned int	malr;
};
#define GETHER0_MAC_BASE	((struct ether_mac_regs *)0xfee0400)
#define GETHER1_MAC_BASE	((struct ether_mac_regs *)0xfee0c00)
#define ETHER0_MAC_BASE		((struct ether_mac_regs *)0xfef0000)
#define ETHER1_MAC_BASE		((struct ether_mac_regs *)0xfef0800)

/* GETHER */
struct gether_control_regs {
	unsigned int	gbecont;
};
#define GETHER_CONTROL_BASE	((struct gether_control_regs *)0xffc10100)
#define GBECONT_RMII1		0x00020000
#define GBECONT_RMII0		0x00010000

/* USB0/1 */
struct usb_common_regs {
	unsigned short	reserved[129];
	unsigned short	suspmode;
};
#define USB0_COMMON_BASE	((struct usb_common_regs *)0xfe450000)
#define USB1_COMMON_BASE	((struct usb_common_regs *)0xfe4f0000)

struct usb0_phy_regs {
	unsigned short	reset;
	unsigned short	reserved[4];
	unsigned short	portsel;
};
#define USB0_PHY_BASE		((struct usb0_phy_regs *)0xfe5f0000)

struct usb1_port_regs {
	unsigned int	port1sel;
	unsigned int	reserved;
	unsigned int	usb1intsts;
};
#define USB1_PORT_BASE		((struct usb1_port_regs *)0xfe4f2000)

struct usb1_alignment_regs {
	unsigned int	ehcidatac;	/* 0xfe4fe018 */
	unsigned int	reserved[63];
	unsigned int	ohcidatac;
};
#define USB1_ALIGNMENT_BASE	((struct usb1_alignment_regs *)0xfe4fe018)

/* GCTRL, GRA */
struct gctrl_regs {
	unsigned int	wprotect;
	unsigned int	gplldiv;
	unsigned int	gracr2;		/* GRA */
	unsigned int	gracr3;		/* GRA */
	unsigned int	reserved[4];
	unsigned int	fcntcr1;
	unsigned int	fcntcr2;
	unsigned int	reserved2[2];
	unsigned int	gpll1div;
	unsigned int	vcompsel;
	unsigned int	reserved3[62];
	unsigned int	fdlmon;
	unsigned int	reserved4[2];
	unsigned int	flcrmon;
	unsigned int	reserved5[944];
	unsigned int	spibootcan;
};
#define GCTRL_BASE		((struct gctrl_regs *)0xffc10000)

/* PCIe setup */
struct pcie_setup_regs {
	unsigned int	pbictl0;
	unsigned int	gradevctl;
	unsigned int	reserved[2];
	unsigned int	bmcinf[6];
	unsigned int	reserved2[118];
	unsigned int	idset[2];
	unsigned int	subidset;
	unsigned int	reserved3[2];
	unsigned int	linkconfset[4];
	unsigned int	trsid;
	unsigned int	reserved4[6];
	unsigned int	toutset;
	unsigned int	reserved5[7];
	unsigned int	lad0;
	unsigned int	ladmsk0;
	unsigned int	lad1;
	unsigned int	ladmsk1;
	unsigned int	lad2;
	unsigned int	ladmsk2;
	unsigned int	lad3;
	unsigned int	ladmsk3;
	unsigned int	lad4;
	unsigned int	ladmsk4;
	unsigned int	lad5;
	unsigned int	ladmsk5;
	unsigned int	reserved6[94];
	unsigned int	vdmrxvid[2];
	unsigned int	reserved7;
	unsigned int	pbiintfr;
	unsigned int	pbiinten;
	unsigned int	msimap;
	unsigned int	barmap;
	unsigned int	baracsize;
	unsigned int	advserest;
	unsigned int	pbictl3;
	unsigned int	reserved8[8];
	unsigned int	pbictl1;
	unsigned int	scratch0;
	unsigned int	reserved9[6];
	unsigned int	pbictl2;
	unsigned int	reserved10;
	unsigned int	pbirev;
};
#define PCIE_SETUP_BASE		((struct pcie_setup_regs *)0xffca1000)

struct pcie_system_bus_regs {
	unsigned int	reserved[3];
	unsigned int	endictl0;
	unsigned int	endictl1;
};
#define PCIE_SYSTEM_BUS_BASE	((struct pcie_system_bus_regs *)0xffca1600)


/* PCIe-Bridge */
struct pciebrg_regs {
	unsigned short	ctrl_h8s;
	unsigned short	reserved[7];
	unsigned short	cp_addr;
	unsigned short	reserved2;
	unsigned short	cp_data;
	unsigned short	reserved3;
	unsigned short	cp_ctrl;
};
#define PCIEBRG_BASE		((struct pciebrg_regs *)0xffd60000)

/* CPU version */
#define CCN_PRR			0xff000044
#define prr_mask(_val)		((_val >> 4) & 0xff)
#define PRR_SH7757_B0		0x10
#define PRR_SH7757_C0		0x11

#define is_sh7757_b0(_val)						\
({									\
	int __ret = prr_mask(__raw_readl(CCN_PRR)) == PRR_SH7757_B0;	\
	__ret;								\
})
#endif	/* ifndef __ASSEMBLY__ */

#endif	/* _ASM_CPU_SH7757_H_ */
