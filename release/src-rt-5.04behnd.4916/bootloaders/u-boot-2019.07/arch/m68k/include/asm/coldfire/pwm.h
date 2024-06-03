/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Pulse Width Modulation Memory Map
 *
 * Copyright (C) 2004-2008 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#ifndef __ATA_H__
#define __ATA_H__

/* Pulse Width Modulation (PWM) */
typedef struct pwm_ctrl {
#ifdef CONFIG_M5272
	u8 cr0;
	u8 res1[3];
	u8 cr1;
	u8 res2[3];
	u8 cr2;
	u8 res3[7];
	u8 pwr0;
	u8 res4[3];
	u8 pwr1;
	u8 res5[3];
	u8 pwr2;
	u8 res6[7];
#else
	u8 en;			/* 0x00 PWM Enable */
	u8 pol;			/* 0x01 Polarity */
	u8 clk;			/* 0x02 Clock Select */
	u8 prclk;		/* 0x03 Prescale Clock Select */
	u8 cae;			/* 0x04 Center Align Enable */
	u8 ctl;			/* 0x05 Control */
	u16 res1;		/* 0x06 - 0x07 */
	u8 scla;		/* 0x08 Scale A */
	u8 sclb;		/* 0x09 Scale B */
	u16 res2;		/* 0x0A - 0x0B */
#ifdef CONFIG_M5275
	u8 cnt[4];		/* 0x0C Channel n Counter */
	u16 res3;		/* 0x10 - 0x11 */
	u8 per[4];		/* 0x14 Channel n Period */
	u16 res4;		/* 0x16 - 0x17 */
	u8 dty[4];		/* 0x18 Channel n Duty */
#else
	u8 cnt[8];		/* 0x0C Channel n Counter */
	u8 per[8];		/* 0x14 Channel n Period */
	u8 dty[8];		/* 0x1C Channel n Duty */
	u8 sdn;			/* 0x24 Shutdown */
	u8 res3[3];		/* 0x25 - 0x27 */
#endif				/* CONFIG_M5275 */
#endif				/* CONFIG_M5272 */
} pwm_t;

#ifdef CONFIG_M5272

#define PWM_CR_EN			(0x80)
#define PWM_CR_FRC1			(0x40)
#define PWM_CR_LVL			(0x20)
#define PWM_CR_CLKSEL(x)		((x) & 0x0F)
#define PWM_CR_CLKSEL_MASK		(0xF0)

#else

#define PWM_EN_PWMEn(x)			(1 << ((x) & 0x07))
#define PWM_EN_PWMEn_MASK		(0xF0)

#define PWM_POL_PPOLn(x)		(1 << ((x) & 0x07))
#define PWM_POL_PPOLn_MASK		(0xF0)

#define PWM_CLK_PCLKn(x)		(1 << ((x) & 0x07))
#define PWM_CLK_PCLKn_MASK		(0xF0)

#define PWM_PRCLK_PCKB(x)		(((x) & 0x07) << 4)
#define PWM_PRCLK_PCKB_MASK		(0x8F)
#define PWM_PRCLK_PCKA(x)		((x) & 0x07)
#define PWM_PRCLK_PCKA_MASK		(0xF8)

#define PWM_CLK_PCLKn(x)		(1 << ((x) & 0x07))
#define PWM_CLK_PCLKn_MASK		(0xF0)

#define PWM_CTL_CON67			(0x80)
#define PWM_CTL_CON45			(0x40)
#define PWM_CTL_CON23			(0x20)
#define PWM_CTL_CON01			(0x10)
#define PWM_CTL_PSWAR			(0x08)
#define PWM_CTL_PFRZ			(0x04)

#define PWM_SDN_IF			(0x80)
#define PWM_SDN_IE			(0x40)
#define PWM_SDN_RESTART			(0x20)
#define PWM_SDN_LVL			(0x10)
#define PWM_SDN_PWM7IN			(0x04)
#define PWM_SDN_PWM7IL			(0x02)
#define PWM_SDN_SDNEN			(0x01)

#endif				/* CONFIG_M5272 */

#endif				/* __ATA_H__ */
