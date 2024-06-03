/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * MCF5274/5 Internal Memory Map
 *
 * Copyright (c) 2005 Arthur Shipkowski <art@videon-central.com>
 * Based on work Copyright (c) 2003 Josef Baumgartner
 *                                  <josef.baumgartner@telex.de>
 */

#ifndef __IMMAP_5275__
#define __IMMAP_5275__

#define MMAP_SCM	(CONFIG_SYS_MBAR + 0x00000000)
#define MMAP_SDRAM	(CONFIG_SYS_MBAR + 0x00000040)
#define MMAP_FBCS	(CONFIG_SYS_MBAR + 0x00000080)
#define MMAP_DMA0	(CONFIG_SYS_MBAR + 0x00000100)
#define MMAP_DMA1	(CONFIG_SYS_MBAR + 0x00000110)
#define MMAP_DMA2	(CONFIG_SYS_MBAR + 0x00000120)
#define MMAP_DMA3	(CONFIG_SYS_MBAR + 0x00000130)
#define MMAP_UART0	(CONFIG_SYS_MBAR + 0x00000200)
#define MMAP_UART1	(CONFIG_SYS_MBAR + 0x00000240)
#define MMAP_UART2	(CONFIG_SYS_MBAR + 0x00000280)
#define MMAP_I2C	(CONFIG_SYS_MBAR + 0x00000300)
#define MMAP_QSPI	(CONFIG_SYS_MBAR + 0x00000340)
#define MMAP_DTMR0	(CONFIG_SYS_MBAR + 0x00000400)
#define MMAP_DTMR1	(CONFIG_SYS_MBAR + 0x00000440)
#define MMAP_DTMR2	(CONFIG_SYS_MBAR + 0x00000480)
#define MMAP_DTMR3	(CONFIG_SYS_MBAR + 0x000004C0)
#define MMAP_INTC0	(CONFIG_SYS_MBAR + 0x00000C00)
#define MMAP_INTC1	(CONFIG_SYS_MBAR + 0x00000D00)
#define MMAP_INTCACK	(CONFIG_SYS_MBAR + 0x00000F00)
#define MMAP_FEC0	(CONFIG_SYS_MBAR + 0x00001000)
#define MMAP_FEC0FIFO	(CONFIG_SYS_MBAR + 0x00001400)
#define MMAP_FEC1	(CONFIG_SYS_MBAR + 0x00001800)
#define MMAP_FEC1FIFO	(CONFIG_SYS_MBAR + 0x00001C00)
#define MMAP_GPIO	(CONFIG_SYS_MBAR + 0x00100000)
#define MMAP_RCM	(CONFIG_SYS_MBAR + 0x00110000)
#define MMAP_CCM	(CONFIG_SYS_MBAR + 0x00110004)
#define MMAP_PLL	(CONFIG_SYS_MBAR + 0x00120000)
#define MMAP_EPORT	(CONFIG_SYS_MBAR + 0x00130000)
#define MMAP_WDOG	(CONFIG_SYS_MBAR + 0x00140000)
#define MMAP_PIT0	(CONFIG_SYS_MBAR + 0x00150000)
#define MMAP_PIT1	(CONFIG_SYS_MBAR + 0x00160000)
#define MMAP_PIT2	(CONFIG_SYS_MBAR + 0x00170000)
#define MMAP_PIT3	(CONFIG_SYS_MBAR + 0x00180000)
#define MMAP_MDHA	(CONFIG_SYS_MBAR + 0x00190000)
#define MMAP_RNG	(CONFIG_SYS_MBAR + 0x001A0000)
#define MMAP_SKHA	(CONFIG_SYS_MBAR + 0x001B0000)
#define MMAP_USB	(CONFIG_SYS_MBAR + 0x001C0000)
#define MMAP_PWM0	(CONFIG_SYS_MBAR + 0x001D0000)

#include <asm/coldfire/eport.h>
#include <asm/coldfire/flexbus.h>
#include <asm/coldfire/intctrl.h>
#include <asm/coldfire/mdha.h>
#include <asm/coldfire/pwm.h>
#include <asm/coldfire/qspi.h>
#include <asm/coldfire/rng.h>
#include <asm/coldfire/skha.h>

/* System configuration registers
*/
typedef	struct sys_ctrl {
	u32 ipsbar;
	u32 res1;
	u32 rambar;
	u32 res2;
	u8 crsr;
	u8 cwcr;
	u8 lpicr;
	u8 cwsr;
	u8 res3[8];
	u32 mpark;
	u8 mpr;
	u8 res4[3];
	u8 pacr0;
	u8 pacr1;
	u8 pacr2;
	u8 pacr3;
	u8 pacr4;
	u8 res5;
	u8 pacr5;
	u8 pacr6;
	u8 pacr7;
	u8 res6;
	u8 pacr8;
	u8 res7;
	u8 gpacr;
	u8 res8[3];
} sysctrl_t;
/* SDRAM controller registers, offset: 0x040
 */
typedef struct sdram_ctrl {
	u32 sdmr;
	u32 sdcr;
	u32 sdcfg1;
	u32 sdcfg2;
	u32 sdbar0;
	u32 sdbmr0;
	u32 sdbar1;
	u32 sdbmr1;
} sdramctrl_t;

/* DMA module registers, offset 0x100
 */
typedef struct	dma_ctrl {
	u32 sar;
	u32 dar;
	u32 dsrbcr;
	u32 dcr;
} dma_t;

/* GPIO port registers
*/
typedef struct	gpio_ctrl {
	/* Port Output Data Registers */
	u8 podr_res1[4];
	u8 podr_busctl;
	u8 podr_addr;
	u8 podr_res2[2];
	u8 podr_cs;
	u8 podr_res3;
	u8 podr_fec0h;
	u8 podr_fec0l;
	u8 podr_feci2c;
	u8 podr_qspi;
	u8 podr_sdram;
	u8 podr_timerh;
	u8 podr_timerl;
	u8 podr_uartl;
	u8 podr_fec1h;
	u8 podr_fec1l;
	u8 podr_bs;
	u8 podr_res4;
	u8 podr_usbh;
	u8 podr_usbl;
	u8 podr_uarth;
	u8 podr_res5[3];
	/* Port Data Direction Registers */
	u8 pddr_res1[4];
	u8 pddr_busctl;
	u8 pddr_addr;
	u8 pddr_res2[2];
	u8 pddr_cs;
	u8 pddr_res3;
	u8 pddr_fec0h;
	u8 pddr_fec0l;
	u8 pddr_feci2c;
	u8 pddr_qspi;
	u8 pddr_sdram;
	u8 pddr_timerh;
	u8 pddr_timerl;
	u8 pddr_uartl;
	u8 pddr_fec1h;
	u8 pddr_fec1l;
	u8 pddr_bs;
	u8 pddr_res4;
	u8 pddr_usbh;
	u8 pddr_usbl;
	u8 pddr_uarth;
	u8 pddr_res5[3];
	/* Port Pin Data/Set Registers */
	u8 ppdsdr_res1[4];
	u8 ppdsdr_busctl;
	u8 ppdsdr_addr;
	u8 ppdsdr_res2[2];
	u8 ppdsdr_cs;
	u8 ppdsdr_res3;
	u8 ppdsdr_fec0h;
	u8 ppdsdr_fec0l;
	u8 ppdsdr_feci2c;
	u8 ppdsdr_qspi;
	u8 ppdsdr_sdram;
	u8 ppdsdr_timerh;
	u8 ppdsdr_timerl;
	u8 ppdsdr_uartl;
	u8 ppdsdr_fec1h;
	u8 ppdsdr_fec1l;
	u8 ppdsdr_bs;
	u8 ppdsdr_res4;
	u8 ppdsdr_usbh;
	u8 ppdsdr_usbl;
	u8 ppdsdr_uarth;
	u8 ppdsdr_res5[3];
	/* Port Clear Output Data Registers */
	u8 pclrr_res1[4];
	u8 pclrr_busctl;
	u8 pclrr_addr;
	u8 pclrr_res2[2];
	u8 pclrr_cs;
	u8 pclrr_res3;
	u8 pclrr_fec0h;
	u8 pclrr_fec0l;
	u8 pclrr_feci2c;
	u8 pclrr_qspi;
	u8 pclrr_sdram;
	u8 pclrr_timerh;
	u8 pclrr_timerl;
	u8 pclrr_uartl;
	u8 pclrr_fec1h;
	u8 pclrr_fec1l;
	u8 pclrr_bs;
	u8 pclrr_res4;
	u8 pclrr_usbh;
	u8 pclrr_usbl;
	u8 pclrr_uarth;
	u8 pclrr_res5[3];
	/* Pin Assignment Registers */
	u8 par_addr;
	u8 par_cs;
	u16 par_busctl;
	u8 par_res1[2];
	u16 par_usb;
	u8 par_fec0hl;
	u8 par_fec1hl;
	u16 par_timer;
	u16 par_uart;
	u16 par_qspi;
	u16 par_sdram;
	u16 par_feci2c;
	u8 par_bs;
	u8 par_res2[3];
} gpio_t;


/* Watchdog registers
 */
typedef struct wdog_ctrl {
	u16 wcr;
	u16 wmr;
	u16 wcntr;
	u16 wsr;
	u8 res4[114];
} wdog_t;

/* USB module registers
*/
typedef struct usb {
	u16 res1;
	u16 fnr;
	u16 res2;
	u16 fnmr;
	u16 res3;
	u16 rfmr;
	u16 res4;
	u16 rfmmr;
	u8 res5[3];
	u8 far;
	u32 asr;
	u32 drr1;
	u32 drr2;
	u16 res6;
	u16 specr;
	u16 res7;
	u16 ep0sr;
	u32 iep0cfg;
	u32 oep0cfg;
	u32 ep1cfg;
	u32 ep2cfg;
	u32 ep3cfg;
	u32 ep4cfg;
	u32 ep5cfg;
	u32 ep6cfg;
	u32 ep7cfg;
	u32 ep0ctl;
	u16 res8;
	u16 ep1ctl;
	u16 res9;
	u16 ep2ctl;
	u16 res10;
	u16 ep3ctl;
	u16 res11;
	u16 ep4ctl;
	u16 res12;
	u16 ep5ctl;
	u16 res13;
	u16 ep6ctl;
	u16 res14;
	u16 ep7ctl;
	u32 ep0isr;
	u16 res15;
	u16 ep1isr;
	u16 res16;
	u16 ep2isr;
	u16 res17;
	u16 ep3isr;
	u16 res18;
	u16 ep4isr;
	u16 res19;
	u16 ep5isr;
	u16 res20;
	u16 ep6isr;
	u16 res21;
	u16 ep7isr;
	u32 ep0imr;
	u16 res22;
	u16 ep1imr;
	u16 res23;
	u16 ep2imr;
	u16 res24;
	u16 ep3imr;
	u16 res25;
	u16 ep4imr;
	u16 res26;
	u16 ep5imr;
	u16 res27;
	u16 ep6imr;
	u16 res28;
	u16 ep7imr;
	u32 ep0dr;
	u32 ep1dr;
	u32 ep2dr;
	u32 ep3dr;
	u32 ep4dr;
	u32 ep5dr;
	u32 ep6dr;
	u32 ep7dr;
	u16 res29;
	u16 ep0dpr;
	u16 res30;
	u16 ep1dpr;
	u16 res31;
	u16 ep2dpr;
	u16 res32;
	u16 ep3dpr;
	u16 res33;
	u16 ep4dpr;
	u16 res34;
	u16 ep5dpr;
	u16 res35;
	u16 ep6dpr;
	u16 res36;
	u16 ep7dpr;
	u8 res37[788];
	u8 cfgram[1024];
} usb_t;

/* PLL module registers
 */
typedef struct pll_ctrl {
	u32 syncr;
	u32 synsr;
} pll_t;

typedef struct rcm {
	u8 rcr;
	u8 rsr;
} rcm_t;

#endif /* __IMMAP_5275__ */
