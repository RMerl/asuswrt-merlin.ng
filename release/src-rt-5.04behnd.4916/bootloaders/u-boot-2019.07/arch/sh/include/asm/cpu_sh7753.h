/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2012  Renesas Solutions Corp.
 */

#ifndef _ASM_CPU_SH7753_H_
#define _ASM_CPU_SH7753_H_

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

/* TMU0 */
#define TMU_BASE	 0xFE430000

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

/* SerMux */
struct sermux_regs {
	unsigned char	smr0;
	unsigned char	smr1;
	unsigned char	smr2;
	unsigned char	smr3;
	unsigned char	smr4;
	unsigned char	smr5;
};
#define SERMUX_BASE	((struct sermux_regs *)0xfe470000)


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

/* GPIO */
struct gpio_regs {
	unsigned short	pacr;
	unsigned short	pbcr;
	unsigned short	pccr;
	unsigned short	pdcr;
	unsigned short	pecr;
	unsigned short	pfcr;
	unsigned short	pgcr;
	unsigned short	phcr;
	unsigned short	picr;
	unsigned short	pjcr;
	unsigned short	pkcr;
	unsigned short	plcr;
	unsigned short	pmcr;
	unsigned short	pncr;
	unsigned short	pocr;
	unsigned short	reserved;
	unsigned short	pqcr;
	unsigned short	prcr;
	unsigned short	pscr;
	unsigned short	ptcr;
	unsigned short	pucr;
	unsigned short	pvcr;
	unsigned short	pwcr;
	unsigned short	pxcr;
	unsigned short	pycr;
	unsigned short	pzcr;
	unsigned char	padr;
	unsigned char	reserved_a;
	unsigned char	pbdr;
	unsigned char	reserved_b;
	unsigned char	pcdr;
	unsigned char	reserved_c;
	unsigned char	pddr;
	unsigned char	reserved_d;
	unsigned char	pedr;
	unsigned char	reserved_e;
	unsigned char	pfdr;
	unsigned char	reserved_f;
	unsigned char	pgdr;
	unsigned char	reserved_g;
	unsigned char	phdr;
	unsigned char	reserved_h;
	unsigned char	pidr;
	unsigned char	reserved_i;
	unsigned char	pjdr;
	unsigned char	reserved_j;
	unsigned char	pkdr;
	unsigned char	reserved_k;
	unsigned char	pldr;
	unsigned char	reserved_l;
	unsigned char	pmdr;
	unsigned char	reserved_m;
	unsigned char	pndr;
	unsigned char	reserved_n;
	unsigned char	podr;
	unsigned char	reserved_o;
	unsigned char	ppdr;
	unsigned char	reserved_p;
	unsigned char	pqdr;
	unsigned char	reserved_q;
	unsigned char	prdr;
	unsigned char	reserved_r;
	unsigned char	psdr;
	unsigned char	reserved_s;
	unsigned char	ptdr;
	unsigned char	reserved_t;
	unsigned char	pudr;
	unsigned char	reserved_u;
	unsigned char	pvdr;
	unsigned char	reserved_v;
	unsigned char	pwdr;
	unsigned char	reserved_w;
	unsigned char	pxdr;
	unsigned char	reserved_x;
	unsigned char	pydr;
	unsigned char	reserved_y;
	unsigned char	pzdr;
	unsigned char	reserved_z;
	unsigned short	ncer;
	unsigned short	ncmcr;
	unsigned short	nccsr;
	unsigned char	reserved2[2];
	unsigned short	psel0;		/* +0x70 */
	unsigned short	psel1;
	unsigned short	psel2;
	unsigned short	psel3;
	unsigned short	psel4;
	unsigned short	psel5;
	unsigned short	psel6;
	unsigned short	reserved3[2];
	unsigned short	psel7;
};
#define GPIO_BASE	((struct gpio_regs *)0xffec0000)

#endif	/* ifndef __ASSEMBLY__ */
#endif	/* _ASM_CPU_SH7753_H_ */
