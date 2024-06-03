// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB <daniel@omicron.se>.
 */

/*
 * Based on sc520cdp.c from rolo 1.6:
 *----------------------------------------------------------------------
 * (C) Copyright 2000
 * Sysgo Real-Time Solutions GmbH
 * Klein-Winternheim, Germany
 *----------------------------------------------------------------------
 */

#include <config.h>

#include <common.h>
#include <asm/io.h>
#include <ali512x.h>


/* ALI M5123 Logical device numbers:
 * 0 FDC
 * 1 unused?
 * 2 unused?
 * 3 lpt
 * 4 UART1
 * 5 UART2
 * 6 RTC
 * 7 mouse/kbd
 * 8 CIO
 */

/*
 ************************************************************
 *  Some access primitives for the ALi chip:                *
 ************************************************************
 */

static void ali_write(u8 index, u8 value)
{
	/* write an arbirary register */
	outb(index, ALI_INDEX);
	outb(value, ALI_DATA);
}

#if 0
static int ali_read(u8 index)
{
	outb(index, ALI_INDEX);
	return inb(ALI_DATA);
}
#endif

#define ALI_OPEN() \
	outb(0x51, ALI_INDEX); \
	outb(0x23, ALI_INDEX)


#define ALI_CLOSE() \
	outb(0xbb, ALI_INDEX)

/* Select a logical device */
#define ALI_SELDEV(dev)	\
	ali_write(0x07, dev)


void ali512x_init(void)
{
	ALI_OPEN();

	ali_write(0x02, 0x01);	/* soft reset */
	ali_write(0x03, 0x03);	/* disable access to CIOs */
	ali_write(0x22, 0x00);	/* disable direct powerdown */
	ali_write(0x23, 0x00);	/* disable auto powerdown */
	ali_write(0x24, 0x00);	/* IR 8 is active hi, pin26 is PDIR */

	ALI_CLOSE();
}

void ali512x_set_fdc(int enabled, u16 io, u8 irq, u8 dma_channel)
{
	ALI_OPEN();
	ALI_SELDEV(0);

	ali_write(0x30, enabled?1:0);
	if (enabled) {
		ali_write(0x60, io >> 8);
		ali_write(0x61, io & 0xff);
		ali_write(0x70, irq);
		ali_write(0x74, dma_channel);

		/* AT mode, no drive swap */
		ali_write(0xf0, 0x08);
		ali_write(0xf1, 0x00);
		ali_write(0xf2, 0xff);
		ali_write(0xf4, 0x00);
	}
	ALI_CLOSE();
}


void ali512x_set_pp(int enabled, u16 io, u8 irq, u8 dma_channel)
{
	ALI_OPEN();
	ALI_SELDEV(3);

	ali_write(0x30, enabled?1:0);
	if (enabled) {
		ali_write(0x60, io >> 8);
		ali_write(0x61, io & 0xff);
		ali_write(0x70, irq);
		ali_write(0x74, dma_channel);

		/* mode: EPP 1.9, ECP FIFO threshold = 7, IRQ active low */
		ali_write(0xf0, 0xbc);
		/* 12 MHz, Burst DMA in ECP */
		ali_write(0xf1, 0x05);
	}
	ALI_CLOSE();

}

void ali512x_set_uart(int enabled, int index, u16 io, u8 irq)
{
	ALI_OPEN();
	ALI_SELDEV(index?5:4);

	ali_write(0x30, enabled?1:0);
	if (enabled) {
		ali_write(0x60, io >> 8);
		ali_write(0x61, io & 0xff);
		ali_write(0x70, irq);

		ali_write(0xf0, 0x00);
		ali_write(0xf1, 0x00);

		/* huh? write 0xf2 twice - a typo in rolo
		 * or some secret ali errata? Who knows?
		 */
		if (index) {
			ali_write(0xf2, 0x00);
		}
		ali_write(0xf2, 0x0c);
	}
	ALI_CLOSE();

}

void ali512x_set_uart2_irda(int enabled)
{
	ALI_OPEN();
	ALI_SELDEV(5);

	ali_write(0xf1, enabled?0x48:0x00); /* fullduplex IrDa */
	ALI_CLOSE();

}

void ali512x_set_rtc(int enabled, u16 io, u8 irq)
{
	ALI_OPEN();
	ALI_SELDEV(6);

	ali_write(0x30, enabled?1:0);
	if (enabled) {
		ali_write(0x60, io >> 8);
		ali_write(0x61, io & 0xff);
		ali_write(0x70, irq);

		ali_write(0xf0, 0x00);
	}
	ALI_CLOSE();
}

void ali512x_set_kbc(int enabled, u8 kbc_irq, u8 mouse_irq)
{
	ALI_OPEN();
	ALI_SELDEV(7);

	ali_write(0x30, enabled?1:0);
	if (enabled) {
		ali_write(0x70, kbc_irq);
		ali_write(0x72, mouse_irq);

		ali_write(0xf0, 0x00);
	}
	ALI_CLOSE();
}


/* Common I/O
 *
 * (This descripotsion is base on several incompete sources
 *  since I have not been able to obtain any datasheet for the device
 *  there may be some mis-understandings burried in here.
 *  -- Daniel daniel@omicron.se)
 *
 * There are 22 CIO pins numbered
 * 10-17
 * 20-25
 * 30-37
 *
 * 20-24 are dedicated CIO pins, the other 17 are muliplexed with
 * other functions.
 *
 *           Secondary
 * CIO Pin   Function    Decription
 * =======================================================
 * CIO10     IRQIN1      Interrupt input 1?
 * CIO11     IRQIN2      Interrupt input 2?
 * CIO12     IRRX        IrDa Receive
 * CIO13     IRTX        IrDa Transmit
 * CIO14     P21         KBC P21 fucntion
 * CIO15     P20         KBC P21 fucntion
 * CIO16     I2C_CLK     I2C Clock
 * CIO17     I2C_DAT     I2C Data
 *
 * CIO20     -
 * CIO21     -
 * CIO22     -
 * CIO23     -
 * CIO24     -
 * CIO25     LOCK        Keylock
 *
 * CIO30     KBC_CLK     Keybaord Clock
 * CIO31     CS0J        General Chip Select decoder CS0J
 * CIO32     CS1J        General Chip Select decoder CS1J
 * CIO33     ALT_KCLK    Alternative Keyboard Clock
 * CIO34     ALT_KDAT    Alternative Keyboard Data
 * CIO35     ALT_MCLK    Alternative Mouse Clock
 * CIO36     ALT_MDAT    Alternative Mouse Data
 * CIO37     ALT_KBC     Alternative KBC select
 *
 * The CIO use an indirect address scheme.
 *
 * Reigster 3 in the SIO is used to select the index and data
 * port addresses where the CIO I/O registers show up.
 * The function selection registers are accessible under
 * function SIO 8.
 *
 * SIO reigster 3 (CIO Address Selection) bit definitions:
 * bit 7   CIO index and data registers enabled
 * bit 1-0 CIO indirect registers port address select
 *	 0  index = 0xE0 data = 0xE1
 *       1  index = 0xE2 data = 0xE3
 *       2  index = 0xE4 data = 0xE5
 *       3  index = 0xEA data = 0xEB
 *
 * There are three CIO I/O register accessed via CIO index port and CIO data port
 * 0x01     CIO 10-17 data
 * 0x02     CIO 20-25 data (bits 7-6 unused)
 * 0x03     CIO 30-37 data
 *
 *
 * The pin function is accessed through normal
 * SIO registers, each register have the same format:
 *
 * Bit   Function                     Value
 * 0     Input/output                 1=input
 * 1     Polarity of signal           1=inverted
 * 2     Unused                       ??
 * 3     Function (normal or special) 1=special
 * 7-4   Unused
 *
 * SIO REG
 * 0xe0     CIO 10 Config
 * 0xe1     CIO 11 Config
 * 0xe2     CIO 12 Config
 * 0xe3     CIO 13 Config
 * 0xe4     CIO 14 Config
 * 0xe5     CIO 15 Config
 * 0xe6     CIO 16 Config
 * 0xe7     CIO 16 Config
 *
 * 0xe8     CIO 20 Config
 * 0xe9     CIO 21 Config
 * 0xea     CIO 22 Config
 * 0xeb     CIO 23 Config
 * 0xec     CIO 24 Config
 * 0xed     CIO 25 Config
 *
 * 0xf5     CIO 30 Config
 * 0xf6     CIO 31 Config
 * 0xf7     CIO 32 Config
 * 0xf8     CIO 33 Config
 * 0xf9     CIO 34 Config
 * 0xfa     CIO 35 Config
 * 0xfb     CIO 36 Config
 * 0xfc     CIO 37 Config
 *
 */

#define ALI_CIO_PORT_SEL 0x83
#define ALI_CIO_INDEX    0xea
#define ALI_CIO_DATA     0xeb

void ali512x_set_cio(int enabled)
{
	int i;

	ALI_OPEN();

	if (enabled) {
		ali_write(0x3, ALI_CIO_PORT_SEL);    /* Enable CIO data register */
	} else {
		ali_write(0x3, ALI_CIO_PORT_SEL & ~0x80);
	}

	ALI_SELDEV(8);

	ali_write(0x30, enabled?1:0);

	/* set all pins to input to start with */
	for (i=0xe0;i<0xee;i++) {
		ali_write(i, 1);
	}

	for (i=0xf5;i<0xfe;i++) {
		ali_write(i, 1);
	}

	ALI_CLOSE();
}


void ali512x_cio_function(int pin, int special, int inv, int input)
{
	u8 data;
	u8 addr;

	/* valid pins are 10-17, 20-25 and 30-37 */
	if (pin >= 10 && pin <= 17) {
		addr = 0xe0+(pin&7);
	} else if (pin >= 20 && pin <= 25) {
		addr = 0xe8+(pin&7);
	} else if (pin >= 30 && pin <= 37) {
		addr = 0xf5+(pin&7);
	} else {
		return;
	}

	ALI_OPEN();

	ALI_SELDEV(8);


	data=0xf4;
	if (special) {
		data |= 0x08;
	} else {
		if (inv) {
			data |= 0x02;
		}
		if (input) {
			data |= 0x01;
		}
	}

	ali_write(addr, data);

	ALI_CLOSE();
}

void ali512x_cio_out(int pin, int value)
{
	u8 reg;
	u8 data;
	u8 bit;

	reg = pin/10;
	bit = 1 << (pin%10);


	outb(reg, ALI_CIO_INDEX);     /* select I/O register */
	data = inb(ALI_CIO_DATA);
	if (value) {
		data |= bit;
	} else {
		data &= ~bit;
	}
	outb(data, ALI_CIO_DATA);
}

int ali512x_cio_in(int pin)
{
	u8 reg;
	u8 data;
	u8 bit;

	/* valid pins are 10-17, 20-25 and 30-37 */
	reg = pin/10;
	bit = 1 << (pin%10);


	outb(reg, ALI_CIO_INDEX);     /* select I/O register */
	data = inb(ALI_CIO_DATA);

	return data & bit;
}
