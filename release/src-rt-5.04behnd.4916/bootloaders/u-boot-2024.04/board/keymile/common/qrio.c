// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013 Keymile AG
 * Valentin Longchamp <valentin.longchamp@keymile.com>
 */

#include <common.h>
#include <asm/io.h>
#include <linux/bitops.h>

#include "common.h"
#include "qrio.h"

/* QRIO ID register offset */
#define ID_REV_OFF		0x00

/* QRIO GPIO register offsets */
#define DIRECT_OFF		0x18
#define GPRT_OFF		0x1c

void show_qrio(void)
{
	void __iomem *qrio_base = (void *)CFG_SYS_QRIO_BASE;
	u16 id_rev = in_be16(qrio_base + ID_REV_OFF);

	printf("QRIO: id = %u, revision = %u\n",
	       (id_rev >> 8) & 0xff, id_rev & 0xff);
}

#define SLFTEST_OFF		0x06

bool qrio_get_selftest_pin(void)
{
	u8 slftest;

	void __iomem *qrio_base = (void *)CFG_SYS_QRIO_BASE;

	slftest = in_8(qrio_base + SLFTEST_OFF);

	return (slftest & 1) > 0;
}

#define BPRTH_OFF		0x04

bool qrio_get_pgy_pres_pin(void)
{
	u8 pgy_pres;

	void __iomem *qrio_base = (void *)CFG_SYS_QRIO_BASE;

	pgy_pres = in_8(qrio_base + BPRTH_OFF);

	return (pgy_pres & 0x80) > 0;
}

int qrio_get_gpio(u8 port_off, u8 gpio_nr)
{
	u32 gprt;

	void __iomem *qrio_base = (void *)CFG_SYS_QRIO_BASE;

	gprt = in_be32(qrio_base + port_off + GPRT_OFF);

	return (gprt >> gpio_nr) & 1U;
}

void qrio_set_gpio(u8 port_off, u8 gpio_nr, bool value)
{
	u32 gprt, mask;

	void __iomem *qrio_base = (void *)CFG_SYS_QRIO_BASE;

	mask = 1U << gpio_nr;

	gprt = in_be32(qrio_base + port_off + GPRT_OFF);
	if (value)
		gprt |= mask;
	else
		gprt &= ~mask;

	out_be32(qrio_base + port_off + GPRT_OFF, gprt);
}

void qrio_gpio_direction_output(u8 port_off, u8 gpio_nr, bool value)
{
	u32 direct, mask;

	void __iomem *qrio_base = (void *)CFG_SYS_QRIO_BASE;

	mask = 1U << gpio_nr;

	direct = in_be32(qrio_base + port_off + DIRECT_OFF);
	direct |= mask;
	out_be32(qrio_base + port_off + DIRECT_OFF, direct);

	qrio_set_gpio(port_off, gpio_nr, value);
}

void qrio_gpio_direction_input(u8 port_off, u8 gpio_nr)
{
	u32 direct, mask;

	void __iomem *qrio_base = (void *)CFG_SYS_QRIO_BASE;

	mask = 1U << gpio_nr;

	direct = in_be32(qrio_base + port_off + DIRECT_OFF);
	direct &= ~mask;
	out_be32(qrio_base + port_off + DIRECT_OFF, direct);
}

void qrio_set_opendrain_gpio(u8 port_off, u8 gpio_nr, u8 val)
{
	u32 direct, mask;

	void __iomem *qrio_base = (void *)CFG_SYS_QRIO_BASE;

	mask = 1U << gpio_nr;

	direct = in_be32(qrio_base + port_off + DIRECT_OFF);
	if (val == 0)
		/* set to output -> GPIO drives low */
		direct |= mask;
	else
		/* set to input -> GPIO floating */
		direct &= ~mask;

	out_be32(qrio_base + port_off + DIRECT_OFF, direct);
}

#define WDMASK_OFF	0x16

void qrio_wdmask(u8 bit, bool wden)
{
	u16 wdmask;
	void __iomem *qrio_base = (void *)CFG_SYS_QRIO_BASE;

	wdmask = in_be16(qrio_base + WDMASK_OFF);

	if (wden)
		wdmask |= (1 << bit);
	else
		wdmask &= ~(1 << bit);

	out_be16(qrio_base + WDMASK_OFF, wdmask);
}

#define PRST_OFF	0x1a

void qrio_prst(u8 bit, bool en, bool wden)
{
	u16 prst;
	void __iomem *qrio_base = (void *)CFG_SYS_QRIO_BASE;

	qrio_wdmask(bit, wden);

	prst = in_be16(qrio_base + PRST_OFF);

	if (en)
		prst &= ~(1 << bit);
	else
		prst |= (1 << bit);

	out_be16(qrio_base + PRST_OFF, prst);
}

#define PRSTCFG_OFF	0x1c

void qrio_prstcfg(u8 bit, u8 mode)
{
	unsigned long prstcfg;
	u8 i;
	void __iomem *qrio_base = (void *)CFG_SYS_QRIO_BASE;

	prstcfg = in_be32(qrio_base + PRSTCFG_OFF);

	for (i = 0; i < 2; i++) {
		if (mode & (1 << i))
			__set_bit(2 * bit + i, &prstcfg);
		else
			__clear_bit(2 * bit + i, &prstcfg);
	}

	out_be32(qrio_base + PRSTCFG_OFF, prstcfg);
}

#define CTRLH_OFF		0x02
#define CTRLH_WRL_BOOT		0x01
#define CTRLH_WRL_UNITRUN	0x02

void qrio_set_leds(void)
{
	u8 ctrlh;
	void __iomem *qrio_base = (void *)CFG_SYS_QRIO_BASE;

	/* set UNIT LED to RED and BOOT LED to ON */
	ctrlh = in_8(qrio_base + CTRLH_OFF);
	ctrlh |= (CTRLH_WRL_BOOT | CTRLH_WRL_UNITRUN);
	out_8(qrio_base + CTRLH_OFF, ctrlh);
}

#define CTRLL_OFF		0x03
#define CTRLL_WRB_BUFENA	0x20

void qrio_enable_app_buffer(void)
{
	u8 ctrll;
	void __iomem *qrio_base = (void *)CFG_SYS_QRIO_BASE;

	/* enable application buffer */
	ctrll = in_8(qrio_base + CTRLL_OFF);
	ctrll |= (CTRLL_WRB_BUFENA);
	out_8(qrio_base + CTRLL_OFF, ctrll);
}

#define REASON1_OFF	0x12
#define REASON1_CPUWD	0x01

void qrio_cpuwd_flag(bool flag)
{
	u8 reason1;
	void __iomem *qrio_base = (void *)CFG_SYS_QRIO_BASE;

	reason1 = in_8(qrio_base + REASON1_OFF);
	if (flag)
		reason1 |= REASON1_CPUWD;
	else
		reason1 &= ~REASON1_CPUWD;
	out_8(qrio_base + REASON1_OFF, reason1);
}

#define REASON0_OFF	0x13
#define REASON0_SWURST	0x80
#define REASON0_CPURST	0x40
#define REASON0_BPRST	0x20
#define REASON0_COPRST	0x10
#define REASON0_SWCRST	0x08
#define REASON0_WDRST	0x04
#define REASON0_KBRST	0x02
#define REASON0_POWUP	0x01
#define UNIT_RESET\
	(REASON0_POWUP | REASON0_COPRST | REASON0_KBRST |\
	 REASON0_BPRST | REASON0_SWURST | REASON0_WDRST)
#define CORE_RESET      ((REASON1_CPUWD << 8) | REASON0_SWCRST)

bool qrio_reason_unitrst(void)
{
	u16 reason;
	void __iomem *qrio_base = (void *)CFG_SYS_QRIO_BASE;

	reason = in_be16(qrio_base + REASON1_OFF);

	return (reason & UNIT_RESET) > 0;
}

#define RSTCFG_OFF	0x11

void qrio_uprstreq(u8 mode)
{
	u32 rstcfg;
	void __iomem *qrio_base = (void *)CFG_SYS_QRIO_BASE;

	rstcfg = in_8(qrio_base + RSTCFG_OFF);

	if (mode & UPREQ_CORE_RST)
		rstcfg |= UPREQ_CORE_RST;
	else
		rstcfg &= ~UPREQ_CORE_RST;

	out_8(qrio_base + RSTCFG_OFF, rstcfg);
}

/* Early bootcount memory area is avilable starting from QRIO3 Rev.2 */
#define QRIO3_ID		0x71
#define QRIO3_REV		0x02
#define EBOOTCNT_OFF		0x28

ulong early_bootcount_load(void)
{
	void __iomem *qrio_base = (void *)CFG_SYS_QRIO_BASE;
	u16 id_rev = in_be16(qrio_base + ID_REV_OFF);
	u8 id = (id_rev >> 8) & 0xff;
	u8 rev = id_rev & 0xff;
	u32 ebootcount = 0;

	if (id == QRIO3_ID && rev >= QRIO3_REV) {
		ebootcount = in_be32(qrio_base + EBOOTCNT_OFF);
	} else {
		printf("QRIO: warning: early bootcount not supported, ");
		printf("id = %u, rev = %u\n", id, rev);
	}

	return ebootcount;
}

void early_bootcount_store(ulong ebootcount)
{
	void __iomem *qrio_base = (void *)CFG_SYS_QRIO_BASE;
	u16 id_rev = in_be16(qrio_base + ID_REV_OFF);
	u8 id = (id_rev >> 8) & 0xff;
	u8 rev = id_rev & 0xff;

	if (id == QRIO3_ID && rev >= QRIO3_REV) {
		out_be32(qrio_base + EBOOTCNT_OFF, ebootcount);
	} else {
		printf("QRIO: warning: early bootcount not supported, ");
		printf("id = %u, rev = %u\n", id, rev);
	}
}

/* I2C deblocking uses the algorithm defined in board/keymile/common/common.c
 * 2 dedicated QRIO GPIOs externally pull the SCL and SDA lines
 * For I2C only the low state is activly driven and high state is pulled-up
 * by a resistor. Therefore the deblock GPIOs are used
 *  -> as an active output to drive a low state
 *  -> as an open-drain input to have a pulled-up high state
 */

/* By default deblock GPIOs are floating */
void i2c_deblock_gpio_cfg(void)
{
	/* set I2C bus 1 deblocking GPIOs input, but 0 value for open drain */
	qrio_gpio_direction_input(KM_I2C_DEBLOCK_PORT,
				  KM_I2C_DEBLOCK_SCL);
	qrio_gpio_direction_input(KM_I2C_DEBLOCK_PORT,
				  KM_I2C_DEBLOCK_SDA);

	qrio_set_gpio(KM_I2C_DEBLOCK_PORT,
		      KM_I2C_DEBLOCK_SCL, 0);
	qrio_set_gpio(KM_I2C_DEBLOCK_PORT,
		      KM_I2C_DEBLOCK_SDA, 0);
}

void set_sda(int state)
{
	qrio_set_opendrain_gpio(KM_I2C_DEBLOCK_PORT,
				KM_I2C_DEBLOCK_SDA, state);
}

void set_scl(int state)
{
	qrio_set_opendrain_gpio(KM_I2C_DEBLOCK_PORT,
				KM_I2C_DEBLOCK_SCL, state);
}

int get_sda(void)
{
	return qrio_get_gpio(KM_I2C_DEBLOCK_PORT,
			     KM_I2C_DEBLOCK_SDA);
}

int get_scl(void)
{
	return qrio_get_gpio(KM_I2C_DEBLOCK_PORT,
			     KM_I2C_DEBLOCK_SCL);
}
