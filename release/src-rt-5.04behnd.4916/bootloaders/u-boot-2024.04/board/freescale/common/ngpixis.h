/* SPDX-License-Identifier: GPL-2.0+ */
/**
 * Copyright 2010-2011 Freescale Semiconductor
 * Author: Timur Tabi <timur@freescale.com>
 *
 * This file provides support for the ngPIXIS, a board-specific FPGA used on
 * some Freescale reference boards.
 */

/* ngPIXIS register set. Hopefully, this won't change too much over time.
 * Feel free to add board-specific #ifdefs where necessary.
 */
typedef struct ngpixis {
	u8 id;
	u8 arch;
	u8 scver;
	u8 csr;
	u8 rst;
	u8 serclk;
	u8 aux;
	u8 spd;
	u8 brdcfg0;
	u8 brdcfg1;	/* On some boards, this register is called 'dma' */
	u8 addr;
	u8 brdcfg2;
	u8 gpiodir;
	u8 data;
	u8 led;
	u8 tag;
	u8 vctl;
	u8 vstat;
	u8 vcfgen0;
	u8 res4;
	u8 ocmcsr;
	u8 ocmmsg;
	u8 gmdbg;
	u8 res5[2];
	u8 sclk[3];
	u8 dclk[3];
	u8 watch;
	struct {
		u8 sw;
		u8 en;
	} s[9];		/* s[0]..s[7] is SW1..SW8, and s[8] is SW11 */
} __attribute__ ((packed)) ngpixis_t;

/* Pointer to the PIXIS register set */
#define pixis ((ngpixis_t *)PIXIS_BASE)

/* The PIXIS SW register that corresponds to board switch X, where x >= 1 */
#define PIXIS_SW(x)		(pixis->s[(x) - 1].sw)

/* The PIXIS EN register that corresponds to board switch X, where x >= 1 */
#define PIXIS_EN(x)		(pixis->s[(x) - 1].en)

u8 pixis_read(unsigned int reg);
void pixis_write(unsigned int reg, u8 value);

#define PIXIS_READ(reg) pixis_read(offsetof(ngpixis_t, reg))
#define PIXIS_WRITE(reg, value) pixis_write(offsetof(ngpixis_t, reg), value)
