/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015 Atmel Corporation.
 *		      Wenyou Yang <wenyou.yang@atmel.com>
 */

#ifndef __ATMEL_PIO4_H
#define __ATMEL_PIO4_H

#ifndef __ASSEMBLY__

struct atmel_pio4_port {
	u32 mskr;		/* 0x00 PIO Mask Register */
	u32 cfgr;		/* 0x04 PIO Configuration Register */
	u32 pdsr;		/* 0x08 PIO Pin Data Status Register */
	u32 locksr;		/* 0x0C PIO Lock Status Register */
	u32 sodr;		/* 0x10 PIO Set Output Data Register */
	u32 codr;		/* 0x14 PIO Clear Output Data Register */
	u32 odsr;		/* 0x18 PIO Output Data Status Register */
	u32 reserved0;
	u32 ier;		/* 0x20 PIO Interrupt Enable Register */
	u32 idr;		/* 0x24 PIO Interrupt Disable Register */
	u32 imr;		/* 0x28 PIO Interrupt Mask Register */
	u32 isr;		/* 0x2C PIO Interrupt Status Register */
	u32 reserved1[3];
	u32 iofr;		/* 0x3C PIO I/O Freeze Register */
};

#endif

/*
 * PIO Configuration Register Fields
 */
#define ATMEL_PIO_CFGR_FUNC_MASK	GENMASK(2, 0)
#define ATMEL_PIO_CFGR_FUNC_GPIO	(0x0 << 0)
#define ATMEL_PIO_CFGR_FUNC_PERIPH_A	(0x1 << 0)
#define ATMEL_PIO_CFGR_FUNC_PERIPH_B	(0x2 << 0)
#define ATMEL_PIO_CFGR_FUNC_PERIPH_C	(0x3 << 0)
#define ATMEL_PIO_CFGR_FUNC_PERIPH_D	(0x4 << 0)
#define ATMEL_PIO_CFGR_FUNC_PERIPH_E	(0x5 << 0)
#define ATMEL_PIO_CFGR_FUNC_PERIPH_F	(0x6 << 0)
#define ATMEL_PIO_CFGR_FUNC_PERIPH_G	(0x7 << 0)
#define ATMEL_PIO_DIR_MASK		BIT(8)
#define ATMEL_PIO_PUEN_MASK		BIT(9)
#define ATMEL_PIO_PDEN_MASK		BIT(10)
#define ATMEL_PIO_IFEN_MASK		BIT(12)
#define ATMEL_PIO_IFSCEN_MASK		BIT(13)
#define ATMEL_PIO_OPD_MASK		BIT(14)
#define ATMEL_PIO_SCHMITT_MASK		BIT(15)
#define ATMEL_PIO_DRVSTR_MASK		GENMASK(17, 16)
#define ATMEL_PIO_DRVSTR_LO		(1 << 16)
#define ATMEL_PIO_DRVSTR_ME		(2 << 16)
#define ATMEL_PIO_DRVSTR_HI		(3 << 16)
#define ATMEL_PIO_CFGR_EVTSEL_MASK	GENMASK(26, 24)
#define ATMEL_PIO_CFGR_EVTSEL_FALLING	(0 << 24)
#define ATMEL_PIO_CFGR_EVTSEL_RISING	(1 << 24)
#define ATMEL_PIO_CFGR_EVTSEL_BOTH	(2 << 24)
#define ATMEL_PIO_CFGR_EVTSEL_LOW	(3 << 24)
#define ATMEL_PIO_CFGR_EVTSEL_HIGH	(4 << 24)

#define ATMEL_PIO_NPINS_PER_BANK	32
#define ATMEL_PIO_BANK(pin_id)		(pin_id / ATMEL_PIO_NPINS_PER_BANK)
#define ATMEL_PIO_LINE(pin_id)		(pin_id % ATMEL_PIO_NPINS_PER_BANK)
#define ATMEL_PIO_BANK_OFFSET		0x40

#define ATMEL_GET_PIN_NO(pinfunc)	((pinfunc) & 0xff)
#define ATMEL_GET_PIN_FUNC(pinfunc)	((pinfunc >> 16) & 0xf)
#define ATMEL_GET_PIN_IOSET(pinfunc)	((pinfunc >> 20) & 0xf)

#define AT91_PIO_PORTA		0x0
#define AT91_PIO_PORTB		0x1
#define AT91_PIO_PORTC		0x2
#define AT91_PIO_PORTD		0x3

int atmel_pio4_set_gpio(u32 port, u32 pin, u32 config);
int atmel_pio4_set_a_periph(u32 port, u32 pin, u32 config);
int atmel_pio4_set_b_periph(u32 port, u32 pin, u32 config);
int atmel_pio4_set_c_periph(u32 port, u32 pin, u32 config);
int atmel_pio4_set_d_periph(u32 port, u32 pin, u32 config);
int atmel_pio4_set_e_periph(u32 port, u32 pin, u32 config);
int atmel_pio4_set_f_periph(u32 port, u32 pin, u32 config);
int atmel_pio4_set_g_periph(u32 port, u32 pin, u32 config);
int atmel_pio4_set_pio_output(u32 port, u32 pin, u32 value);
int atmel_pio4_get_pio_input(u32 port, u32 pin);

#endif
