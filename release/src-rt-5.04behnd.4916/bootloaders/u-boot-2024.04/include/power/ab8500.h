/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Based on include/linux/mfd/abx500/ab8500.h from Linux
 * Copyright (C) ST-Ericsson SA 2010
 * Author: Srinidhi Kasagar <srinidhi.kasagar@stericsson.com>
 */

#ifndef _PMIC_AB8500_H_
#define _PMIC_AB8500_H_

/*
 * AB IC versions
 *
 * AB8500_VERSION_AB8500 should be 0xFF but will never be read as need a
 * non-supported multi-byte I2C access via PRCMU. Set to 0x00 to ease the
 * print of version string.
 */
enum ab8500_version {
	AB8500_VERSION_AB8500 = 0x0,
	AB8500_VERSION_AB8505 = 0x1,
	AB8500_VERSION_AB9540 = 0x2,
	AB8500_VERSION_AB8540 = 0x4,
	AB8500_VERSION_UNDEFINED,
};

/* AB8500 CIDs*/
#define AB8500_CUTEARLY	0x00
#define AB8500_CUT1P0	0x10
#define AB8500_CUT1P1	0x11
#define AB8500_CUT1P2	0x12 /* Only valid for AB8540 */
#define AB8500_CUT2P0	0x20
#define AB8500_CUT3P0	0x30
#define AB8500_CUT3P3	0x33

/*
 * AB8500 bank addresses
 */
#define AB8500_BANK(bank, reg)		(((bank) << 8) | (reg))
#define AB8500_M_FSM_RANK(reg)		AB8500_BANK(0x0, reg)
#define AB8500_SYS_CTRL1_BLOCK(reg)	AB8500_BANK(0x1, reg)
#define AB8500_SYS_CTRL2_BLOCK(reg)	AB8500_BANK(0x2, reg)
#define AB8500_REGU_CTRL1(reg)		AB8500_BANK(0x3, reg)
#define AB8500_REGU_CTRL2(reg)		AB8500_BANK(0x4, reg)
#define AB8500_USB(reg)			AB8500_BANK(0x5, reg)
#define AB8500_TVOUT(reg)		AB8500_BANK(0x6, reg)
#define AB8500_DBI(reg)			AB8500_BANK(0x7, reg)
#define AB8500_ECI_AV_ACC(reg)		AB8500_BANK(0x8, reg)
#define AB8500_RESERVED(reg)		AB8500_BANK(0x9, reg)
#define AB8500_GPADC(reg)		AB8500_BANK(0xA, reg)
#define AB8500_CHARGER(reg)		AB8500_BANK(0xB, reg)
#define AB8500_GAS_GAUGE(reg)		AB8500_BANK(0xC, reg)
#define AB8500_AUDIO(reg)		AB8500_BANK(0xD, reg)
#define AB8500_INTERRUPT(reg)		AB8500_BANK(0xE, reg)
#define AB8500_RTC(reg)			AB8500_BANK(0xF, reg)
#define AB8500_GPIO(reg)		AB8500_BANK(0x10, reg)
#define AB8500_MISC(reg)		AB8500_BANK(0x10, reg)
#define AB8500_DEVELOPMENT(reg)		AB8500_BANK(0x11, reg)
#define AB8500_DEBUG(reg)		AB8500_BANK(0x12, reg)
#define AB8500_PROD_TEST(reg)		AB8500_BANK(0x13, reg)
#define AB8500_STE_TEST(reg)		AB8500_BANK(0x14, reg)
#define AB8500_OTP_EMUL(reg)		AB8500_BANK(0x15, reg)

#define AB8500_NUM_BANKS		0x16
#define AB8500_NUM_REGISTERS		AB8500_BANK(AB8500_NUM_BANKS, 0)

struct ab8500 {
	enum ab8500_version version;
	u8 chip_id;
};

static inline int is_ab8500(struct ab8500 *ab)
{
	return ab->version == AB8500_VERSION_AB8500;
}

static inline int is_ab8505(struct ab8500 *ab)
{
	return ab->version == AB8500_VERSION_AB8505;
}

/* exclude also ab8505, ab9540... */
static inline int is_ab8500_1p0_or_earlier(struct ab8500 *ab)
{
	return (is_ab8500(ab) && (ab->chip_id <= AB8500_CUT1P0));
}

/* exclude also ab8505, ab9540... */
static inline int is_ab8500_1p1_or_earlier(struct ab8500 *ab)
{
	return (is_ab8500(ab) && (ab->chip_id <= AB8500_CUT1P1));
}

/* exclude also ab8505, ab9540... */
static inline int is_ab8500_2p0_or_earlier(struct ab8500 *ab)
{
	return (is_ab8500(ab) && (ab->chip_id <= AB8500_CUT2P0));
}

static inline int is_ab8500_3p3_or_earlier(struct ab8500 *ab)
{
	return (is_ab8500(ab) && (ab->chip_id <= AB8500_CUT3P3));
}

/* exclude also ab8505, ab9540... */
static inline int is_ab8500_2p0(struct ab8500 *ab)
{
	return (is_ab8500(ab) && (ab->chip_id == AB8500_CUT2P0));
}

static inline int is_ab8505_1p0_or_earlier(struct ab8500 *ab)
{
	return (is_ab8505(ab) && (ab->chip_id <= AB8500_CUT1P0));
}

static inline int is_ab8505_2p0(struct ab8500 *ab)
{
	return (is_ab8505(ab) && (ab->chip_id == AB8500_CUT2P0));
}

static inline int is_ab8505_2p0_earlier(struct ab8500 *ab)
{
	return (is_ab8505(ab) && (ab->chip_id < AB8500_CUT2P0));
}

#endif
