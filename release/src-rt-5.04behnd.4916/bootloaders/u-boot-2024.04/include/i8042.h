/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2002 ELTEC Elektronik AG
 * Frank Gottschling <fgottschling@eltec.de>
 */

/* i8042.h - Intel 8042 keyboard driver header */

#ifndef _I8042_H_
#define _I8042_H_

/* defines */

#define I8042_DATA_REG	0x60	/* keyboard i/o buffer */
#define I8042_STS_REG	0x64	/* keyboard status read */
#define I8042_CMD_REG	0x64	/* keyboard ctrl write */

/* Status register bit defines */
#define STATUS_OBF	(1 << 0)
#define STATUS_IBF	(1 << 1)

/* Configuration byte bit defines */
#define CFG_KIRQ_EN	(1 << 0)
#define CFG_MIRQ_EN	(1 << 1)
#define CFG_SET_BIST	(1 << 2)
#define CFG_KCLK_DIS	(1 << 4)
#define CFG_MCLK_DIS	(1 << 5)
#define CFG_AT_TRANS	(1 << 6)

/* i8042 commands */
#define CMD_RD_CONFIG	0x20	/* read configuration byte */
#define CMD_WR_CONFIG	0x60	/* write configuration byte */
#define CMD_SELF_TEST	0xaa	/* controller self-test */
#define CMD_KBD_DIS	0xad	/* keyboard disable */
#define CMD_KBD_EN	0xae	/* keyboard enable */
#define CMD_SET_KBD_LED	0xed	/* set keyboard led */
#define CMD_DRAIN_OUTPUT 0xf4   /* drain output buffer */
#define CMD_RESET_KBD	0xff	/* reset keyboard */

/* i8042 command result */
#define KBC_TEST_OK	0x55
#define KBD_ACK		0xfa
#define KBD_POR		0xaa

/* keyboard scan codes */

#define KBD_US		0	/* default US layout */
#define KBD_GER		1	/* german layout */

#define KBD_TIMEOUT	1000	/* 1 sec */
#define KBD_RESET_TRIES	3

#define AS		0	/* normal character index */
#define SH		1	/* shift index */
#define CN		2	/* control index */
#define NM		3	/* numeric lock index */
#define AK		4	/* right alt key */
#define CP		5	/* capslock index */
#define ST		6	/* stop output index */
#define EX		7	/* extended code index */
#define ES		8	/* escape and extended code index */

#define NORMAL		0x0000	/* normal key */
#define STP		0x0001	/* scroll lock stop output*/
#define NUM		0x0002	/* numeric lock */
#define CAPS		0x0004	/* capslock */
#define SHIFT		0x0008	/* shift */
#define CTRL		0x0010	/* control*/
#define EXT		0x0020	/* extended scan code 0xe0 */
#define ESC		0x0040	/* escape key press */
#define E1		0x0080	/* extended scan code 0xe1 */
#define BRK		0x0100	/* make break flag for keyboard */
#define ALT		0x0200	/* right alt */

#endif /* _I8042_H_ */
