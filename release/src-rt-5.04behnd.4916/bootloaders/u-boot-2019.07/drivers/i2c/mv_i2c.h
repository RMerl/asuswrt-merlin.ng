/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2011
 * Marvell Inc, <www.marvell.com>
 */

#ifndef _MV_I2C_H_
#define _MV_I2C_H_
extern void i2c_clk_enable(void);

/* Shall the current transfer have a start/stop condition? */
#define I2C_COND_NORMAL		0
#define I2C_COND_START		1
#define I2C_COND_STOP		2

/* Shall the current transfer be ack/nacked or being waited for it? */
#define I2C_ACKNAK_WAITACK	1
#define I2C_ACKNAK_SENDACK	2
#define I2C_ACKNAK_SENDNAK	4

/* Specify who shall transfer the data (master or slave) */
#define I2C_READ		0
#define I2C_WRITE		1

#define I2C_ICR_INIT	(ICR_BEIE | ICR_IRFIE | ICR_ITEIE | ICR_GCD | ICR_SCLE)

#define I2C_ISR_INIT		0x7FF
/* ----- Control register bits ---------------------------------------- */

#define ICR_START	0x1		/* start bit */
#define ICR_STOP	0x2		/* stop bit */
#define ICR_ACKNAK	0x4		/* send ACK(0) or NAK(1) */
#define ICR_TB		0x8		/* transfer byte bit */
#define ICR_MA		0x10		/* master abort */
#define ICR_SCLE	0x20		/* master clock enable, mona SCLEA */
#define ICR_IUE		0x40		/* unit enable */
#define ICR_GCD		0x80		/* general call disable */
#define ICR_ITEIE	0x100		/* enable tx interrupts */
#define ICR_IRFIE	0x200		/* enable rx interrupts, mona: DRFIE */
#define ICR_BEIE	0x400		/* enable bus error ints */
#define ICR_SSDIE	0x800		/* slave STOP detected int enable */
#define ICR_ALDIE	0x1000		/* enable arbitration interrupt */
#define ICR_SADIE	0x2000		/* slave address detected int enable */
#define ICR_UR		0x4000		/* unit reset */
#ifdef CONFIG_ARMADA_3700
#define ICR_SM		0x00000		/* Standard Mode */
#define ICR_FM		0x10000		/* Fast Mode */
#define ICR_MODE_MASK	0x30000		/* Mode mask */
#else
#define ICR_SM		0x00000		/* Standard Mode */
#define ICR_FM		0x08000		/* Fast Mode */
#define ICR_MODE_MASK	0x18000		/* Mode mask */
#endif

/* ----- Status register bits ----------------------------------------- */

#define ISR_RWM		0x1		/* read/write mode */
#define ISR_ACKNAK	0x2		/* ack/nak status */
#define ISR_UB		0x4		/* unit busy */
#define ISR_IBB		0x8		/* bus busy */
#define ISR_SSD		0x10		/* slave stop detected */
#define ISR_ALD		0x20		/* arbitration loss detected */
#define ISR_ITE		0x40		/* tx buffer empty */
#define ISR_IRF		0x80		/* rx buffer full */
#define ISR_GCAD	0x100		/* general call address detected */
#define ISR_SAD		0x200		/* slave address detected */
#define ISR_BED		0x400		/* bus error no ACK/NAK */

#endif
