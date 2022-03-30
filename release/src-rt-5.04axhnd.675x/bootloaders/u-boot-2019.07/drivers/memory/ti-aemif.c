// SPDX-License-Identifier: GPL-2.0+
/*
 * Keystone2: Asynchronous EMIF Configuration
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#include <common.h>
#include <asm/ti-common/ti-aemif.h>

#define AEMIF_WAITCYCLE_CONFIG		(CONFIG_AEMIF_CNTRL_BASE + 0x4)
#define AEMIF_NAND_CONTROL		(CONFIG_AEMIF_CNTRL_BASE + 0x60)
#define AEMIF_ONENAND_CONTROL		(CONFIG_AEMIF_CNTRL_BASE + 0x5c)
#define AEMIF_CONFIG(cs)		(CONFIG_AEMIF_CNTRL_BASE + 0x10 \
					 + (cs * 4))

#define AEMIF_CFG_SELECT_STROBE(v)	((v) ? 1 << 31 : 0)
#define AEMIF_CFG_EXTEND_WAIT(v)	((v) ? 1 << 30 : 0)
#define AEMIF_CFG_WR_SETUP(v)		(((v) & 0x0f) << 26)
#define AEMIF_CFG_WR_STROBE(v)		(((v) & 0x3f) << 20)
#define AEMIF_CFG_WR_HOLD(v)		(((v) & 0x07) << 17)
#define AEMIF_CFG_RD_SETUP(v)		(((v) & 0x0f) << 13)
#define AEMIF_CFG_RD_STROBE(v)		(((v) & 0x3f) << 7)
#define AEMIF_CFG_RD_HOLD(v)		(((v) & 0x07) << 4)
#define AEMIF_CFG_TURN_AROUND(v)	(((v) & 0x03) << 2)
#define AEMIF_CFG_WIDTH(v)		(((v) & 0x03) << 0)

#define set_config_field(reg, field, val)			\
	do {							\
		if (val != -1) {				\
			reg &= ~AEMIF_CFG_##field(0xffffffff);	\
			reg |=	AEMIF_CFG_##field(val);		\
		}						\
	} while (0)

static void aemif_configure(int cs, struct aemif_config *cfg)
{
	unsigned long tmp;

	if (cfg->mode == AEMIF_MODE_NAND) {
		tmp = __raw_readl(AEMIF_NAND_CONTROL);
		tmp |= (1 << cs);
		__raw_writel(tmp, AEMIF_NAND_CONTROL);

	} else if (cfg->mode == AEMIF_MODE_ONENAND) {
		tmp = __raw_readl(AEMIF_ONENAND_CONTROL);
		tmp |= (1 << cs);
		__raw_writel(tmp, AEMIF_ONENAND_CONTROL);
	}

	tmp = __raw_readl(AEMIF_CONFIG(cs));

	set_config_field(tmp, SELECT_STROBE,	cfg->select_strobe);
	set_config_field(tmp, EXTEND_WAIT,	cfg->extend_wait);
	set_config_field(tmp, WR_SETUP,		cfg->wr_setup);
	set_config_field(tmp, WR_STROBE,	cfg->wr_strobe);
	set_config_field(tmp, WR_HOLD,		cfg->wr_hold);
	set_config_field(tmp, RD_SETUP,		cfg->rd_setup);
	set_config_field(tmp, RD_STROBE,	cfg->rd_strobe);
	set_config_field(tmp, RD_HOLD,		cfg->rd_hold);
	set_config_field(tmp, TURN_AROUND,	cfg->turn_around);
	set_config_field(tmp, WIDTH,		cfg->width);

	__raw_writel(tmp, AEMIF_CONFIG(cs));
}

void aemif_init(int num_cs, struct aemif_config *config)
{
	int cs;

	if (num_cs > AEMIF_NUM_CS) {
		num_cs = AEMIF_NUM_CS;
		printf("AEMIF: csnum has to be <= 5");
	}

	for (cs = 0; cs < num_cs; cs++)
		aemif_configure(cs, config + cs);
}
