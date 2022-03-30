/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * MCF5272 Internal Memory Map
 *
 * Copyright (c) 2003 Josef Baumgartner <josef.baumgartner@telex.de>
 */

#ifndef __IMMAP_5272__
#define __IMMAP_5272__

#define MMAP_CFG	(CONFIG_SYS_MBAR + 0x00000000)
#define MMAP_INTC	(CONFIG_SYS_MBAR + 0x00000020)
#define MMAP_FBCS	(CONFIG_SYS_MBAR + 0x00000040)
#define MMAP_GPIO	(CONFIG_SYS_MBAR + 0x00000080)
#define MMAP_QSPI	(CONFIG_SYS_MBAR + 0x000000A0)
#define MMAP_PWM	(CONFIG_SYS_MBAR + 0x000000C0)
#define MMAP_DMA0	(CONFIG_SYS_MBAR + 0x000000E0)
#define MMAP_UART0	(CONFIG_SYS_MBAR + 0x00000100)
#define MMAP_UART1	(CONFIG_SYS_MBAR + 0x00000140)
#define MMAP_SDRAM	(CONFIG_SYS_MBAR + 0x00000180)
#define MMAP_TMR0	(CONFIG_SYS_MBAR + 0x00000200)
#define MMAP_TMR1	(CONFIG_SYS_MBAR + 0x00000220)
#define MMAP_TMR2	(CONFIG_SYS_MBAR + 0x00000240)
#define MMAP_TMR3	(CONFIG_SYS_MBAR + 0x00000260)
#define MMAP_WDOG	(CONFIG_SYS_MBAR + 0x00000280)
#define MMAP_PLIC	(CONFIG_SYS_MBAR + 0x00000300)
#define MMAP_FEC	(CONFIG_SYS_MBAR + 0x00000840)
#define MMAP_USB	(CONFIG_SYS_MBAR + 0x00001000)

#include <asm/coldfire/pwm.h>

/* System configuration registers */
typedef struct sys_ctrl {
	uint sc_mbar;
	ushort sc_scr;
	ushort sc_spr;
	uint sc_pmr;
	char res1[2];
	ushort sc_alpr;
	uint sc_dir;
	char res2[12];
} sysctrl_t;

/* Interrupt module registers */
typedef struct int_ctrl {
	uint int_icr1;
	uint int_icr2;
	uint int_icr3;
	uint int_icr4;
	uint int_isr;
	uint int_pitr;
	uint int_piwr;
	uchar res1[3];
	uchar int_pivr;
} intctrl_t;

/* Chip select module registers */
typedef struct cs_ctlr {
	uint cs_br0;
	uint cs_or0;
	uint cs_br1;
	uint cs_or1;
	uint cs_br2;
	uint cs_or2;
	uint cs_br3;
	uint cs_or3;
	uint cs_br4;
	uint cs_or4;
	uint cs_br5;
	uint cs_or5;
	uint cs_br6;
	uint cs_or6;
	uint cs_br7;
	uint cs_or7;
} csctrl_t;

/* GPIO port registers */
typedef struct gpio_ctrl {
	uint gpio_pacnt;
	ushort gpio_paddr;
	ushort gpio_padat;
	uint gpio_pbcnt;
	ushort gpio_pbddr;
	ushort gpio_pbdat;
	uchar res1[4];
	ushort gpio_pcddr;
	ushort gpio_pcdat;
	uint gpio_pdcnt;
	uchar res2[4];
} gpio_t;

/* DMA module registers */
typedef struct dma_ctrl {
	ulong dma_dmr;
	uchar res1[2];
	ushort dma_dir;
	ulong dma_dbcr;
	ulong dma_dsar;
	ulong dma_ddar;
	uchar res2[12];
} dma_t;

/* SDRAM controller registers, offset: 0x180 */
typedef struct sdram_ctrl {
	uchar res1[2];
	ushort sdram_sdcr;
	uchar res2[2];
	ushort sdram_sdtr;
	uchar res3[120];
} sdramctrl_t;

/* Watchdog registers */
typedef struct wdog_ctrl {
	ushort wdog_wrrr;
	ushort res1;
	ushort wdog_wirr;
	ushort res2;
	ushort wdog_wcr;
	ushort res3;
	ushort wdog_wer;
	uchar res4[114];
} wdog_t;

/* PLIC module registers */
typedef struct plic_ctrl {
	ulong plic_p0b1rr;
	ulong plic_p1b1rr;
	ulong plic_p2b1rr;
	ulong plic_p3b1rr;
	ulong plic_p0b2rr;
	ulong plic_p1b2rr;
	ulong plic_p2b2rr;
	ulong plic_p3b2rr;
	uchar plic_p0drr;
	uchar plic_p1drr;
	uchar plic_p2drr;
	uchar plic_p3drr;
	uchar res1[4];
	ulong plic_p0b1tr;
	ulong plic_p1b1tr;
	ulong plic_p2b1tr;
	ulong plic_p3b1tr;
	ulong plic_p0b2tr;
	ulong plic_p1b2tr;
	ulong plic_p2b2tr;
	ulong plic_p3b2tr;
	uchar plic_p0dtr;
	uchar plic_p1dtr;
	uchar plic_p2dtr;
	uchar plic_p3dtr;
	uchar res2[4];
	ushort plic_p0cr;
	ushort plic_p1cr;
	ushort plic_p2cr;
	ushort plic_p3cr;
	ushort plic_p0icr;
	ushort plic_p1icr;
	ushort plic_p2icr;
	ushort plic_p3icr;
	ushort plic_p0gmr;
	ushort plic_p1gmr;
	ushort plic_p2gmr;
	ushort plic_p3gmr;
	ushort plic_p0gmt;
	ushort plic_p1gmt;
	ushort plic_p2gmt;
	ushort plic_p3gmt;
	uchar res3;
	uchar plic_pgmts;
	uchar plic_pgmta;
	uchar res4;
	uchar plic_p0gcir;
	uchar plic_p1gcir;
	uchar plic_p2gcir;
	uchar plic_p3gcir;
	uchar plic_p0gcit;
	uchar plic_p1gcit;
	uchar plic_p2gcit;
	uchar plic_p3gcit;
	uchar res5[3];
	uchar plic_pgcitsr;
	uchar res6[3];
	uchar plic_pdcsr;
	ushort plic_p0psr;
	ushort plic_p1psr;
	ushort plic_p2psr;
	ushort plic_p3psr;
	ushort plic_pasr;
	uchar res7;
	uchar plic_plcr;
	ushort res8;
	ushort plic_pdrqr;
	ushort plic_p0sdr;
	ushort plic_p1sdr;
	ushort plic_p2sdr;
	ushort plic_p3sdr;
	ushort res9;
	ushort plic_pcsr;
	uchar res10[1184];
} plic_t;

/* USB module registers */
typedef struct usb {
	ushort res1;
	ushort usb_fnr;
	ushort res2;
	ushort usb_fnmr;
	ushort res3;
	ushort usb_rfmr;
	ushort res4;
	ushort usb_rfmmr;
	uchar res5[3];
	uchar usb_far;
	ulong usb_asr;
	ulong usb_drr1;
	ulong usb_drr2;
	ushort res6;
	ushort usb_specr;
	ushort res7;
	ushort usb_ep0sr;
	ulong usb_iep0cfg;
	ulong usb_oep0cfg;
	ulong usb_ep1cfg;
	ulong usb_ep2cfg;
	ulong usb_ep3cfg;
	ulong usb_ep4cfg;
	ulong usb_ep5cfg;
	ulong usb_ep6cfg;
	ulong usb_ep7cfg;
	ulong usb_ep0ctl;
	ushort res8;
	ushort usb_ep1ctl;
	ushort res9;
	ushort usb_ep2ctl;
	ushort res10;
	ushort usb_ep3ctl;
	ushort res11;
	ushort usb_ep4ctl;
	ushort res12;
	ushort usb_ep5ctl;
	ushort res13;
	ushort usb_ep6ctl;
	ushort res14;
	ushort usb_ep7ctl;
	ulong usb_ep0isr;
	ushort res15;
	ushort usb_ep1isr;
	ushort res16;
	ushort usb_ep2isr;
	ushort res17;
	ushort usb_ep3isr;
	ushort res18;
	ushort usb_ep4isr;
	ushort res19;
	ushort usb_ep5isr;
	ushort res20;
	ushort usb_ep6isr;
	ushort res21;
	ushort usb_ep7isr;
	ulong usb_ep0imr;
	ushort res22;
	ushort usb_ep1imr;
	ushort res23;
	ushort usb_ep2imr;
	ushort res24;
	ushort usb_ep3imr;
	ushort res25;
	ushort usb_ep4imr;
	ushort res26;
	ushort usb_ep5imr;
	ushort res27;
	ushort usb_ep6imr;
	ushort res28;
	ushort usb_ep7imr;
	ulong usb_ep0dr;
	ulong usb_ep1dr;
	ulong usb_ep2dr;
	ulong usb_ep3dr;
	ulong usb_ep4dr;
	ulong usb_ep5dr;
	ulong usb_ep6dr;
	ulong usb_ep7dr;
	ushort res29;
	ushort usb_ep0dpr;
	ushort res30;
	ushort usb_ep1dpr;
	ushort res31;
	ushort usb_ep2dpr;
	ushort res32;
	ushort usb_ep3dpr;
	ushort res33;
	ushort usb_ep4dpr;
	ushort res34;
	ushort usb_ep5dpr;
	ushort res35;
	ushort usb_ep6dpr;
	ushort res36;
	ushort usb_ep7dpr;
	uchar res37[788];
	uchar usb_cfgram[1024];
} usb_t;

#endif				/* __IMMAP_5272__ */
