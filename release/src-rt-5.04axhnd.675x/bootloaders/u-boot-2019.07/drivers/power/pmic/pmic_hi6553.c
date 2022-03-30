// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2015 Linaro
 *  Peter Griffin <peter.griffin@linaro.org>
 */
#include <asm/io.h>
#include <common.h>
#include <power/pmic.h>
#include <power/max8997_muic.h>
#include <power/hi6553_pmic.h>
#include <errno.h>

u8 *pmussi_base;

uint8_t hi6553_readb(u32 offset)
{
	return readb(pmussi_base + (offset << 2));
}

void hi6553_writeb(u32 offset, uint8_t value)
{
	writeb(value, pmussi_base + (offset << 2));
}

int pmic_reg_write(struct pmic *p, u32 reg, u32 val)
{
	if (check_reg(p, reg))
		return -EINVAL;

	hi6553_writeb(reg, (uint8_t)val);

	return 0;
}

int pmic_reg_read(struct pmic *p, u32 reg, u32 *val)
{
	if (check_reg(p, reg))
		return -EINVAL;

	*val = (u32)hi6553_readb(reg);

	return 0;
}

static void hi6553_init(void)
{
	int data;

	hi6553_writeb(HI6553_PERI_EN_MARK, 0x1e);
	hi6553_writeb(HI6553_NP_REG_ADJ1, 0);
	data = HI6553_DISABLE6_XO_CLK_CONN | HI6553_DISABLE6_XO_CLK_NFC |
		HI6553_DISABLE6_XO_CLK_RF1 | HI6553_DISABLE6_XO_CLK_RF2;
	hi6553_writeb(HI6553_DISABLE6_XO_CLK, data);

	/* configure BUCK0 & BUCK1 */
	hi6553_writeb(HI6553_BUCK01_CTRL2, 0x5e);
	hi6553_writeb(HI6553_BUCK0_CTRL7, 0x10);
	hi6553_writeb(HI6553_BUCK1_CTRL7, 0x10);
	hi6553_writeb(HI6553_BUCK0_CTRL5, 0x1e);
	hi6553_writeb(HI6553_BUCK1_CTRL5, 0x1e);
	hi6553_writeb(HI6553_BUCK0_CTRL1, 0xfc);
	hi6553_writeb(HI6553_BUCK1_CTRL1, 0xfc);

	/* configure BUCK2 */
	hi6553_writeb(HI6553_BUCK2_REG1, 0x4f);
	hi6553_writeb(HI6553_BUCK2_REG5, 0x99);
	hi6553_writeb(HI6553_BUCK2_REG6, 0x45);
	mdelay(1);
	hi6553_writeb(HI6553_VSET_BUCK2_ADJ, 0x22);
	mdelay(1);

	/* configure BUCK3 */
	hi6553_writeb(HI6553_BUCK3_REG3, 0x02);
	hi6553_writeb(HI6553_BUCK3_REG5, 0x99);
	hi6553_writeb(HI6553_BUCK3_REG6, 0x41);
	hi6553_writeb(HI6553_VSET_BUCK3_ADJ, 0x02);
	mdelay(1);

	/* configure BUCK4 */
	hi6553_writeb(HI6553_BUCK4_REG2, 0x9a);
	hi6553_writeb(HI6553_BUCK4_REG5, 0x99);
	hi6553_writeb(HI6553_BUCK4_REG6, 0x45);

	/* configure LDO20 */
	hi6553_writeb(HI6553_LDO20_REG_ADJ, 0x50);

	hi6553_writeb(HI6553_NP_REG_CHG, 0x0f);
	hi6553_writeb(HI6553_CLK_TOP0, 0x06);
	hi6553_writeb(HI6553_CLK_TOP3, 0xc0);
	hi6553_writeb(HI6553_CLK_TOP4, 0x00);

	/* configure LDO7 & LDO10 for SD slot */
	data = hi6553_readb(HI6553_LDO7_REG_ADJ);
	data = (data & 0xf8) | 0x2;
	hi6553_writeb(HI6553_LDO7_REG_ADJ, data);
	mdelay(5);
	/* enable LDO7 */
	hi6553_writeb(HI6553_ENABLE2_LDO1_8, 1 << 6);
	mdelay(5);
	data = hi6553_readb(HI6553_LDO10_REG_ADJ);
	data = (data & 0xf8) | 0x5;
	hi6553_writeb(HI6553_LDO10_REG_ADJ, data);
	mdelay(5);
	/* enable LDO10 */
	hi6553_writeb(HI6553_ENABLE3_LDO9_16, 1 << 1);
	mdelay(5);

	/* select 32.764KHz */
	hi6553_writeb(HI6553_CLK19M2_600_586_EN, 0x01);
}

int power_hi6553_init(u8 *base)
{
	static const char name[] = "HI6553 PMIC";
	struct pmic *p = pmic_alloc();

	if (!p) {
		printf("%s: POWER allocation error!\n", __func__);
		return -ENOMEM;
	}

	p->name = name;
	p->interface = PMIC_NONE;
	p->number_of_regs = 44;
	pmussi_base = base;

	hi6553_init();

	puts("HI6553 PMIC init\n");

	return 0;
}
