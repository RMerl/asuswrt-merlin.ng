// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <clk.h>
#include <ram.h>
#include <reset.h>
#include <timer.h>
#include <asm/io.h>
#include <asm/arch/ddr.h>
#include <linux/iopoll.h>
#include "stm32mp1_ddr.h"
#include "stm32mp1_ddr_regs.h"

#define RCC_DDRITFCR		0xD8

#define RCC_DDRITFCR_DDRCAPBRST		(BIT(14))
#define RCC_DDRITFCR_DDRCAXIRST		(BIT(15))
#define RCC_DDRITFCR_DDRCORERST		(BIT(16))
#define RCC_DDRITFCR_DPHYAPBRST		(BIT(17))
#define RCC_DDRITFCR_DPHYRST		(BIT(18))
#define RCC_DDRITFCR_DPHYCTLRST		(BIT(19))

struct reg_desc {
	const char *name;
	u16 offset;	/* offset for base address */
	u8 par_offset;	/* offset for parameter array */
};

#define INVALID_OFFSET	0xFF

#define DDRCTL_REG(x, y) \
	{#x,\
	 offsetof(struct stm32mp1_ddrctl, x),\
	 offsetof(struct y, x)}

#define DDRPHY_REG(x, y) \
	{#x,\
	 offsetof(struct stm32mp1_ddrphy, x),\
	 offsetof(struct y, x)}

#define DDR_REG_DYN(x) \
	{#x,\
	 offsetof(struct stm32mp1_ddrctl, x),\
	 INVALID_OFFSET}

#define DDRPHY_REG_DYN(x) \
	{#x,\
	 offsetof(struct stm32mp1_ddrphy, x),\
	 INVALID_OFFSET}

/***********************************************************
 * PARAMETERS: value get from device tree :
 *             size / order need to be aligned with binding
 *             modification NOT ALLOWED !!!
 ***********************************************************/
#define DDRCTL_REG_REG_SIZE	25	/* st,ctl-reg */
#define DDRCTL_REG_TIMING_SIZE	12	/* st,ctl-timing */
#define DDRCTL_REG_MAP_SIZE	9	/* st,ctl-map */
#define DDRCTL_REG_PERF_SIZE	17	/* st,ctl-perf */

#define DDRPHY_REG_REG_SIZE	11	/* st,phy-reg */
#define	DDRPHY_REG_TIMING_SIZE	10	/* st,phy-timing */
#define	DDRPHY_REG_CAL_SIZE	12	/* st,phy-cal */

#define DDRCTL_REG_REG(x)	DDRCTL_REG(x, stm32mp1_ddrctrl_reg)
static const struct reg_desc ddr_reg[DDRCTL_REG_REG_SIZE] = {
	DDRCTL_REG_REG(mstr),
	DDRCTL_REG_REG(mrctrl0),
	DDRCTL_REG_REG(mrctrl1),
	DDRCTL_REG_REG(derateen),
	DDRCTL_REG_REG(derateint),
	DDRCTL_REG_REG(pwrctl),
	DDRCTL_REG_REG(pwrtmg),
	DDRCTL_REG_REG(hwlpctl),
	DDRCTL_REG_REG(rfshctl0),
	DDRCTL_REG_REG(rfshctl3),
	DDRCTL_REG_REG(crcparctl0),
	DDRCTL_REG_REG(zqctl0),
	DDRCTL_REG_REG(dfitmg0),
	DDRCTL_REG_REG(dfitmg1),
	DDRCTL_REG_REG(dfilpcfg0),
	DDRCTL_REG_REG(dfiupd0),
	DDRCTL_REG_REG(dfiupd1),
	DDRCTL_REG_REG(dfiupd2),
	DDRCTL_REG_REG(dfiphymstr),
	DDRCTL_REG_REG(odtmap),
	DDRCTL_REG_REG(dbg0),
	DDRCTL_REG_REG(dbg1),
	DDRCTL_REG_REG(dbgcmd),
	DDRCTL_REG_REG(poisoncfg),
	DDRCTL_REG_REG(pccfg),
};

#define DDRCTL_REG_TIMING(x)	DDRCTL_REG(x, stm32mp1_ddrctrl_timing)
static const struct reg_desc ddr_timing[DDRCTL_REG_TIMING_SIZE] = {
	DDRCTL_REG_TIMING(rfshtmg),
	DDRCTL_REG_TIMING(dramtmg0),
	DDRCTL_REG_TIMING(dramtmg1),
	DDRCTL_REG_TIMING(dramtmg2),
	DDRCTL_REG_TIMING(dramtmg3),
	DDRCTL_REG_TIMING(dramtmg4),
	DDRCTL_REG_TIMING(dramtmg5),
	DDRCTL_REG_TIMING(dramtmg6),
	DDRCTL_REG_TIMING(dramtmg7),
	DDRCTL_REG_TIMING(dramtmg8),
	DDRCTL_REG_TIMING(dramtmg14),
	DDRCTL_REG_TIMING(odtcfg),
};

#define DDRCTL_REG_MAP(x)	DDRCTL_REG(x, stm32mp1_ddrctrl_map)
static const struct reg_desc ddr_map[DDRCTL_REG_MAP_SIZE] = {
	DDRCTL_REG_MAP(addrmap1),
	DDRCTL_REG_MAP(addrmap2),
	DDRCTL_REG_MAP(addrmap3),
	DDRCTL_REG_MAP(addrmap4),
	DDRCTL_REG_MAP(addrmap5),
	DDRCTL_REG_MAP(addrmap6),
	DDRCTL_REG_MAP(addrmap9),
	DDRCTL_REG_MAP(addrmap10),
	DDRCTL_REG_MAP(addrmap11),
};

#define DDRCTL_REG_PERF(x)	DDRCTL_REG(x, stm32mp1_ddrctrl_perf)
static const struct reg_desc ddr_perf[DDRCTL_REG_PERF_SIZE] = {
	DDRCTL_REG_PERF(sched),
	DDRCTL_REG_PERF(sched1),
	DDRCTL_REG_PERF(perfhpr1),
	DDRCTL_REG_PERF(perflpr1),
	DDRCTL_REG_PERF(perfwr1),
	DDRCTL_REG_PERF(pcfgr_0),
	DDRCTL_REG_PERF(pcfgw_0),
	DDRCTL_REG_PERF(pcfgqos0_0),
	DDRCTL_REG_PERF(pcfgqos1_0),
	DDRCTL_REG_PERF(pcfgwqos0_0),
	DDRCTL_REG_PERF(pcfgwqos1_0),
	DDRCTL_REG_PERF(pcfgr_1),
	DDRCTL_REG_PERF(pcfgw_1),
	DDRCTL_REG_PERF(pcfgqos0_1),
	DDRCTL_REG_PERF(pcfgqos1_1),
	DDRCTL_REG_PERF(pcfgwqos0_1),
	DDRCTL_REG_PERF(pcfgwqos1_1),
};

#define DDRPHY_REG_REG(x)	DDRPHY_REG(x, stm32mp1_ddrphy_reg)
static const struct reg_desc ddrphy_reg[DDRPHY_REG_REG_SIZE] = {
	DDRPHY_REG_REG(pgcr),
	DDRPHY_REG_REG(aciocr),
	DDRPHY_REG_REG(dxccr),
	DDRPHY_REG_REG(dsgcr),
	DDRPHY_REG_REG(dcr),
	DDRPHY_REG_REG(odtcr),
	DDRPHY_REG_REG(zq0cr1),
	DDRPHY_REG_REG(dx0gcr),
	DDRPHY_REG_REG(dx1gcr),
	DDRPHY_REG_REG(dx2gcr),
	DDRPHY_REG_REG(dx3gcr),
};

#define DDRPHY_REG_TIMING(x)	DDRPHY_REG(x, stm32mp1_ddrphy_timing)
static const struct reg_desc ddrphy_timing[DDRPHY_REG_TIMING_SIZE] = {
	DDRPHY_REG_TIMING(ptr0),
	DDRPHY_REG_TIMING(ptr1),
	DDRPHY_REG_TIMING(ptr2),
	DDRPHY_REG_TIMING(dtpr0),
	DDRPHY_REG_TIMING(dtpr1),
	DDRPHY_REG_TIMING(dtpr2),
	DDRPHY_REG_TIMING(mr0),
	DDRPHY_REG_TIMING(mr1),
	DDRPHY_REG_TIMING(mr2),
	DDRPHY_REG_TIMING(mr3),
};

#define DDRPHY_REG_CAL(x)	DDRPHY_REG(x, stm32mp1_ddrphy_cal)
static const struct reg_desc ddrphy_cal[DDRPHY_REG_CAL_SIZE] = {
	DDRPHY_REG_CAL(dx0dllcr),
	DDRPHY_REG_CAL(dx0dqtr),
	DDRPHY_REG_CAL(dx0dqstr),
	DDRPHY_REG_CAL(dx1dllcr),
	DDRPHY_REG_CAL(dx1dqtr),
	DDRPHY_REG_CAL(dx1dqstr),
	DDRPHY_REG_CAL(dx2dllcr),
	DDRPHY_REG_CAL(dx2dqtr),
	DDRPHY_REG_CAL(dx2dqstr),
	DDRPHY_REG_CAL(dx3dllcr),
	DDRPHY_REG_CAL(dx3dqtr),
	DDRPHY_REG_CAL(dx3dqstr),
};

/**************************************************************
 * DYNAMIC REGISTERS: only used for debug purpose (read/modify)
 **************************************************************/
#ifdef CONFIG_STM32MP1_DDR_INTERACTIVE
static const struct reg_desc ddr_dyn[] = {
	DDR_REG_DYN(stat),
	DDR_REG_DYN(init0),
	DDR_REG_DYN(dfimisc),
	DDR_REG_DYN(dfistat),
	DDR_REG_DYN(swctl),
	DDR_REG_DYN(swstat),
	DDR_REG_DYN(pctrl_0),
	DDR_REG_DYN(pctrl_1),
};

#define DDR_REG_DYN_SIZE	ARRAY_SIZE(ddr_dyn)

static const struct reg_desc ddrphy_dyn[] = {
	DDRPHY_REG_DYN(pir),
	DDRPHY_REG_DYN(pgsr),
	DDRPHY_REG_DYN(zq0sr0),
	DDRPHY_REG_DYN(zq0sr1),
	DDRPHY_REG_DYN(dx0gsr0),
	DDRPHY_REG_DYN(dx0gsr1),
	DDRPHY_REG_DYN(dx1gsr0),
	DDRPHY_REG_DYN(dx1gsr1),
	DDRPHY_REG_DYN(dx2gsr0),
	DDRPHY_REG_DYN(dx2gsr1),
	DDRPHY_REG_DYN(dx3gsr0),
	DDRPHY_REG_DYN(dx3gsr1),
};

#define DDRPHY_REG_DYN_SIZE	ARRAY_SIZE(ddrphy_dyn)

#endif

/*****************************************************************
 * REGISTERS ARRAY: used to parse device tree and interactive mode
 *****************************************************************/
enum reg_type {
	REG_REG,
	REG_TIMING,
	REG_PERF,
	REG_MAP,
	REGPHY_REG,
	REGPHY_TIMING,
	REGPHY_CAL,
#ifdef CONFIG_STM32MP1_DDR_INTERACTIVE
/* dynamic registers => managed in driver or not changed,
 * can be dumped in interactive mode
 */
	REG_DYN,
	REGPHY_DYN,
#endif
	REG_TYPE_NB
};

enum base_type {
	DDR_BASE,
	DDRPHY_BASE,
	NONE_BASE
};

struct ddr_reg_info {
	const char *name;
	const struct reg_desc *desc;
	u8 size;
	enum base_type base;
};

#define DDRPHY_REG_CAL(x)	DDRPHY_REG(x, stm32mp1_ddrphy_cal)

const struct ddr_reg_info ddr_registers[REG_TYPE_NB] = {
[REG_REG] = {
	"static", ddr_reg, DDRCTL_REG_REG_SIZE, DDR_BASE},
[REG_TIMING] = {
	"timing", ddr_timing, DDRCTL_REG_TIMING_SIZE, DDR_BASE},
[REG_PERF] = {
	"perf", ddr_perf, DDRCTL_REG_PERF_SIZE, DDR_BASE},
[REG_MAP] = {
	"map", ddr_map, DDRCTL_REG_MAP_SIZE, DDR_BASE},
[REGPHY_REG] = {
	"static", ddrphy_reg, DDRPHY_REG_REG_SIZE, DDRPHY_BASE},
[REGPHY_TIMING] = {
	"timing", ddrphy_timing, DDRPHY_REG_TIMING_SIZE, DDRPHY_BASE},
[REGPHY_CAL] = {
	"cal", ddrphy_cal, DDRPHY_REG_CAL_SIZE, DDRPHY_BASE},
#ifdef CONFIG_STM32MP1_DDR_INTERACTIVE
[REG_DYN] = {
	"dyn", ddr_dyn, DDR_REG_DYN_SIZE, DDR_BASE},
[REGPHY_DYN] = {
	"dyn", ddrphy_dyn, DDRPHY_REG_DYN_SIZE, DDRPHY_BASE},
#endif

};

const char *base_name[] = {
	[DDR_BASE] = "ctl",
	[DDRPHY_BASE] = "phy",
};

static u32 get_base_addr(const struct ddr_info *priv, enum base_type base)
{
	if (base == DDRPHY_BASE)
		return (u32)priv->phy;
	else
		return (u32)priv->ctl;
}

static void set_reg(const struct ddr_info *priv,
		    enum reg_type type,
		    const void *param)
{
	unsigned int i;
	unsigned int *ptr, value;
	enum base_type base = ddr_registers[type].base;
	u32 base_addr = get_base_addr(priv, base);
	const struct reg_desc *desc = ddr_registers[type].desc;

	debug("init %s\n", ddr_registers[type].name);
	for (i = 0; i < ddr_registers[type].size; i++) {
		ptr = (unsigned int *)(base_addr + desc[i].offset);
		if (desc[i].par_offset == INVALID_OFFSET) {
			pr_err("invalid parameter offset for %s", desc[i].name);
		} else {
			value = *((u32 *)((u32)param +
					       desc[i].par_offset));
			writel(value, ptr);
			debug("[0x%x] %s= 0x%08x\n",
			      (u32)ptr, desc[i].name, value);
		}
	}
}

#ifdef CONFIG_STM32MP1_DDR_INTERACTIVE
static void stm32mp1_dump_reg_desc(u32 base_addr, const struct reg_desc *desc)
{
	unsigned int *ptr;

	ptr = (unsigned int *)(base_addr + desc->offset);
	printf("%s= 0x%08x\n", desc->name, readl(ptr));
}

static void stm32mp1_dump_param_desc(u32 par_addr, const struct reg_desc *desc)
{
	unsigned int *ptr;

	ptr = (unsigned int *)(par_addr + desc->par_offset);
	printf("%s= 0x%08x\n", desc->name, readl(ptr));
}

static const struct reg_desc *found_reg(const char *name, enum reg_type *type)
{
	unsigned int i, j;
	const struct reg_desc *desc;

	for (i = 0; i < ARRAY_SIZE(ddr_registers); i++) {
		desc = ddr_registers[i].desc;
		for (j = 0; j < ddr_registers[i].size; j++) {
			if (strcmp(name, desc[j].name) == 0) {
				*type = i;
				return &desc[j];
			}
		}
	}
	*type = REG_TYPE_NB;
	return NULL;
}

int stm32mp1_dump_reg(const struct ddr_info *priv,
		      const char *name)
{
	unsigned int i, j;
	const struct reg_desc *desc;
	u32 base_addr;
	enum base_type p_base;
	enum reg_type type;
	const char *p_name;
	enum base_type filter = NONE_BASE;
	int result = -1;

	if (name) {
		if (strcmp(name, base_name[DDR_BASE]) == 0)
			filter = DDR_BASE;
		else if (strcmp(name, base_name[DDRPHY_BASE]) == 0)
			filter = DDRPHY_BASE;
	}

	for (i = 0; i < ARRAY_SIZE(ddr_registers); i++) {
		p_base = ddr_registers[i].base;
		p_name = ddr_registers[i].name;
		if (!name || (filter == p_base || !strcmp(name, p_name))) {
			result = 0;
			desc = ddr_registers[i].desc;
			base_addr = get_base_addr(priv, p_base);
			printf("==%s.%s==\n", base_name[p_base], p_name);
			for (j = 0; j < ddr_registers[i].size; j++)
				stm32mp1_dump_reg_desc(base_addr, &desc[j]);
		}
	}
	if (result) {
		desc = found_reg(name, &type);
		if (desc) {
			p_base = ddr_registers[type].base;
			base_addr = get_base_addr(priv, p_base);
			stm32mp1_dump_reg_desc(base_addr, desc);
			result = 0;
		}
	}
	return result;
}

void stm32mp1_edit_reg(const struct ddr_info *priv,
		       char *name, char *string)
{
	unsigned long *ptr, value;
	enum reg_type type;
	enum base_type base;
	const struct reg_desc *desc;
	u32 base_addr;

	desc = found_reg(name, &type);

	if (!desc) {
		printf("%s not found\n", name);
		return;
	}
	if (strict_strtoul(string, 16, &value) < 0) {
		printf("invalid value %s\n", string);
		return;
	}
	base = ddr_registers[type].base;
	base_addr = get_base_addr(priv, base);
	ptr = (unsigned long *)(base_addr + desc->offset);
	writel(value, ptr);
	printf("%s= 0x%08x\n", desc->name, readl(ptr));
}

static u32 get_par_addr(const struct stm32mp1_ddr_config *config,
			enum reg_type type)
{
	u32 par_addr = 0x0;

	switch (type) {
	case REG_REG:
		par_addr = (u32)&config->c_reg;
		break;
	case REG_TIMING:
		par_addr = (u32)&config->c_timing;
		break;
	case REG_PERF:
		par_addr = (u32)&config->c_perf;
		break;
	case REG_MAP:
		par_addr = (u32)&config->c_map;
		break;
	case REGPHY_REG:
		par_addr = (u32)&config->p_reg;
		break;
	case REGPHY_TIMING:
		par_addr = (u32)&config->p_timing;
		break;
	case REGPHY_CAL:
		par_addr = (u32)&config->p_cal;
		break;
	case REG_DYN:
	case REGPHY_DYN:
	case REG_TYPE_NB:
		par_addr = (u32)NULL;
		break;
	}

	return par_addr;
}

int stm32mp1_dump_param(const struct stm32mp1_ddr_config *config,
			const char *name)
{
	unsigned int i, j;
	const struct reg_desc *desc;
	u32 par_addr;
	enum base_type p_base;
	enum reg_type type;
	const char *p_name;
	enum base_type filter = NONE_BASE;
	int result = -EINVAL;

	if (name) {
		if (strcmp(name, base_name[DDR_BASE]) == 0)
			filter = DDR_BASE;
		else if (strcmp(name, base_name[DDRPHY_BASE]) == 0)
			filter = DDRPHY_BASE;
	}

	for (i = 0; i < ARRAY_SIZE(ddr_registers); i++) {
		par_addr = get_par_addr(config, i);
		if (!par_addr)
			continue;
		p_base = ddr_registers[i].base;
		p_name = ddr_registers[i].name;
		if (!name || (filter == p_base || !strcmp(name, p_name))) {
			result = 0;
			desc = ddr_registers[i].desc;
			printf("==%s.%s==\n", base_name[p_base], p_name);
			for (j = 0; j < ddr_registers[i].size; j++)
				stm32mp1_dump_param_desc(par_addr, &desc[j]);
		}
	}
	if (result) {
		desc = found_reg(name, &type);
		if (desc) {
			par_addr = get_par_addr(config, type);
			if (par_addr) {
				stm32mp1_dump_param_desc(par_addr, desc);
				result = 0;
			}
		}
	}
	return result;
}

void stm32mp1_edit_param(const struct stm32mp1_ddr_config *config,
			 char *name, char *string)
{
	unsigned long *ptr, value;
	enum reg_type type;
	const struct reg_desc *desc;
	u32 par_addr;

	desc = found_reg(name, &type);
	if (!desc) {
		printf("%s not found\n", name);
		return;
	}
	if (strict_strtoul(string, 16, &value) < 0) {
		printf("invalid value %s\n", string);
		return;
	}
	par_addr = get_par_addr(config, type);
	if (!par_addr) {
		printf("no parameter %s\n", name);
		return;
	}
	ptr = (unsigned long *)(par_addr + desc->par_offset);
	writel(value, ptr);
	printf("%s= 0x%08x\n", desc->name, readl(ptr));
}
#endif

__weak bool stm32mp1_ddr_interactive(void *priv,
				     enum stm32mp1_ddr_interact_step step,
				     const struct stm32mp1_ddr_config *config)
{
	return false;
}

#define INTERACTIVE(step)\
	stm32mp1_ddr_interactive(priv, step, config)

static void ddrphy_idone_wait(struct stm32mp1_ddrphy *phy)
{
	u32 pgsr;
	int ret;

	ret = readl_poll_timeout(&phy->pgsr, pgsr,
				 pgsr & (DDRPHYC_PGSR_IDONE |
					 DDRPHYC_PGSR_DTERR |
					 DDRPHYC_PGSR_DTIERR |
					 DDRPHYC_PGSR_DFTERR |
					 DDRPHYC_PGSR_RVERR |
					 DDRPHYC_PGSR_RVEIRR),
				1000000);
	debug("\n[0x%08x] pgsr = 0x%08x ret=%d\n",
	      (u32)&phy->pgsr, pgsr, ret);
}

void stm32mp1_ddrphy_init(struct stm32mp1_ddrphy *phy, u32 pir)
{
	pir |= DDRPHYC_PIR_INIT;
	writel(pir, &phy->pir);
	debug("[0x%08x] pir = 0x%08x -> 0x%08x\n",
	      (u32)&phy->pir, pir, readl(&phy->pir));

	/* need to wait 10 configuration clock before start polling */
	udelay(10);

	/* Wait DRAM initialization and Gate Training Evaluation complete */
	ddrphy_idone_wait(phy);
}

/* start quasi dynamic register update */
static void start_sw_done(struct stm32mp1_ddrctl *ctl)
{
	clrbits_le32(&ctl->swctl, DDRCTRL_SWCTL_SW_DONE);
}

/* wait quasi dynamic register update */
static void wait_sw_done_ack(struct stm32mp1_ddrctl *ctl)
{
	int ret;
	u32 swstat;

	setbits_le32(&ctl->swctl, DDRCTRL_SWCTL_SW_DONE);

	ret = readl_poll_timeout(&ctl->swstat, swstat,
				 swstat & DDRCTRL_SWSTAT_SW_DONE_ACK,
				 1000000);
	if (ret)
		panic("Timeout initialising DRAM : DDR->swstat = %x\n",
		      swstat);

	debug("[0x%08x] swstat = 0x%08x\n", (u32)&ctl->swstat, swstat);
}

/* wait quasi dynamic register update */
static void wait_operating_mode(struct ddr_info *priv, int mode)
{
	u32 stat, val, mask, val2 = 0, mask2 = 0;
	int ret;

	mask = DDRCTRL_STAT_OPERATING_MODE_MASK;
	val = mode;
	/* self-refresh due to software => check also STAT.selfref_type */
	if (mode == DDRCTRL_STAT_OPERATING_MODE_SR) {
		mask |= DDRCTRL_STAT_SELFREF_TYPE_MASK;
		val |= DDRCTRL_STAT_SELFREF_TYPE_SR;
	} else if (mode == DDRCTRL_STAT_OPERATING_MODE_NORMAL) {
		/* normal mode: handle also automatic self refresh */
		mask2 = DDRCTRL_STAT_OPERATING_MODE_MASK |
			DDRCTRL_STAT_SELFREF_TYPE_MASK;
		val2 = DDRCTRL_STAT_OPERATING_MODE_SR |
		       DDRCTRL_STAT_SELFREF_TYPE_ASR;
	}

	ret = readl_poll_timeout(&priv->ctl->stat, stat,
				 ((stat & mask) == val) ||
				 (mask2 && ((stat & mask2) == val2)),
				 1000000);

	if (ret)
		panic("Timeout DRAM : DDR->stat = %x\n", stat);

	debug("[0x%08x] stat = 0x%08x\n", (u32)&priv->ctl->stat, stat);
}

void stm32mp1_refresh_disable(struct stm32mp1_ddrctl *ctl)
{
	start_sw_done(ctl);
	/* quasi-dynamic register update*/
	setbits_le32(&ctl->rfshctl3, DDRCTRL_RFSHCTL3_DIS_AUTO_REFRESH);
	clrbits_le32(&ctl->pwrctl, DDRCTRL_PWRCTL_POWERDOWN_EN);
	clrbits_le32(&ctl->dfimisc, DDRCTRL_DFIMISC_DFI_INIT_COMPLETE_EN);
	wait_sw_done_ack(ctl);
}

void stm32mp1_refresh_restore(struct stm32mp1_ddrctl *ctl,
			      u32 rfshctl3, u32 pwrctl)
{
	start_sw_done(ctl);
	if (!(rfshctl3 & DDRCTRL_RFSHCTL3_DIS_AUTO_REFRESH))
		clrbits_le32(&ctl->rfshctl3, DDRCTRL_RFSHCTL3_DIS_AUTO_REFRESH);
	if (pwrctl & DDRCTRL_PWRCTL_POWERDOWN_EN)
		setbits_le32(&ctl->pwrctl, DDRCTRL_PWRCTL_POWERDOWN_EN);
	setbits_le32(&ctl->dfimisc, DDRCTRL_DFIMISC_DFI_INIT_COMPLETE_EN);
	wait_sw_done_ack(ctl);
}

/* board-specific DDR power initializations. */
__weak int board_ddr_power_init(enum ddr_type ddr_type)
{
	return 0;
}

__maybe_unused
void stm32mp1_ddr_init(struct ddr_info *priv,
		       const struct stm32mp1_ddr_config *config)
{
	u32 pir;
	int ret = -EINVAL;

	if (config->c_reg.mstr & DDRCTRL_MSTR_DDR3)
		ret = board_ddr_power_init(STM32MP_DDR3);
	else if (config->c_reg.mstr & DDRCTRL_MSTR_LPDDR2)
		ret = board_ddr_power_init(STM32MP_LPDDR2);
	else if (config->c_reg.mstr & DDRCTRL_MSTR_LPDDR3)
		ret = board_ddr_power_init(STM32MP_LPDDR3);

	if (ret)
		panic("ddr power init failed\n");

start:
	debug("name = %s\n", config->info.name);
	debug("speed = %d kHz\n", config->info.speed);
	debug("size  = 0x%x\n", config->info.size);
/*
 * 1. Program the DWC_ddr_umctl2 registers
 * 1.1 RESETS: presetn, core_ddrc_rstn, aresetn
 */
	/* Assert All DDR part */
	setbits_le32(priv->rcc + RCC_DDRITFCR, RCC_DDRITFCR_DDRCAPBRST);
	setbits_le32(priv->rcc + RCC_DDRITFCR, RCC_DDRITFCR_DDRCAXIRST);
	setbits_le32(priv->rcc + RCC_DDRITFCR, RCC_DDRITFCR_DDRCORERST);
	setbits_le32(priv->rcc + RCC_DDRITFCR, RCC_DDRITFCR_DPHYAPBRST);
	setbits_le32(priv->rcc + RCC_DDRITFCR, RCC_DDRITFCR_DPHYRST);
	setbits_le32(priv->rcc + RCC_DDRITFCR, RCC_DDRITFCR_DPHYCTLRST);

/* 1.2. start CLOCK */
	if (stm32mp1_ddr_clk_enable(priv, config->info.speed))
		panic("invalid DRAM clock : %d kHz\n",
		      config->info.speed);

/* 1.3. deassert reset */
	/* de-assert PHY rstn and ctl_rstn via DPHYRST and DPHYCTLRST */
	clrbits_le32(priv->rcc + RCC_DDRITFCR, RCC_DDRITFCR_DPHYRST);
	clrbits_le32(priv->rcc + RCC_DDRITFCR, RCC_DDRITFCR_DPHYCTLRST);
	/* De-assert presetn once the clocks are active
	 * and stable via DDRCAPBRST bit
	 */
	clrbits_le32(priv->rcc + RCC_DDRITFCR, RCC_DDRITFCR_DDRCAPBRST);

/* 1.4. wait 128 cycles to permit initialization of end logic */
	udelay(2);
	/* for PCLK = 133MHz => 1 us is enough, 2 to allow lower frequency */

	if (INTERACTIVE(STEP_DDR_RESET))
		goto start;

/* 1.5. initialize registers ddr_umctl2 */
	/* Stop uMCTL2 before PHY is ready */
	clrbits_le32(&priv->ctl->dfimisc, DDRCTRL_DFIMISC_DFI_INIT_COMPLETE_EN);
	debug("[0x%08x] dfimisc = 0x%08x\n",
	      (u32)&priv->ctl->dfimisc, readl(&priv->ctl->dfimisc));

	set_reg(priv, REG_REG, &config->c_reg);
	set_reg(priv, REG_TIMING, &config->c_timing);
	set_reg(priv, REG_MAP, &config->c_map);

	/* skip CTRL init, SDRAM init is done by PHY PUBL */
	clrsetbits_le32(&priv->ctl->init0,
			DDRCTRL_INIT0_SKIP_DRAM_INIT_MASK,
			DDRCTRL_INIT0_SKIP_DRAM_INIT_NORMAL);

	set_reg(priv, REG_PERF, &config->c_perf);

	if (INTERACTIVE(STEP_CTL_INIT))
		goto start;

/*  2. deassert reset signal core_ddrc_rstn, aresetn and presetn */
	clrbits_le32(priv->rcc + RCC_DDRITFCR, RCC_DDRITFCR_DDRCORERST);
	clrbits_le32(priv->rcc + RCC_DDRITFCR, RCC_DDRITFCR_DDRCAXIRST);
	clrbits_le32(priv->rcc + RCC_DDRITFCR, RCC_DDRITFCR_DPHYAPBRST);

/*  3. start PHY init by accessing relevant PUBL registers
 *    (DXGCR, DCR, PTR*, MR*, DTPR*)
 */
	set_reg(priv, REGPHY_REG, &config->p_reg);
	set_reg(priv, REGPHY_TIMING, &config->p_timing);
	set_reg(priv, REGPHY_CAL, &config->p_cal);

	if (INTERACTIVE(STEP_PHY_INIT))
		goto start;

/*  4. Monitor PHY init status by polling PUBL register PGSR.IDONE
 *     Perform DDR PHY DRAM initialization and Gate Training Evaluation
 */
	ddrphy_idone_wait(priv->phy);

/*  5. Indicate to PUBL that controller performs SDRAM initialization
 *     by setting PIR.INIT and PIR CTLDINIT and pool PGSR.IDONE
 *     DRAM init is done by PHY, init0.skip_dram.init = 1
 */
	pir = DDRPHYC_PIR_DLLSRST | DDRPHYC_PIR_DLLLOCK | DDRPHYC_PIR_ZCAL |
	      DDRPHYC_PIR_ITMSRST | DDRPHYC_PIR_DRAMINIT | DDRPHYC_PIR_ICPC;

	if (config->c_reg.mstr & DDRCTRL_MSTR_DDR3)
		pir |= DDRPHYC_PIR_DRAMRST; /* only for DDR3 */

	stm32mp1_ddrphy_init(priv->phy, pir);

/*  6. SET DFIMISC.dfi_init_complete_en to 1 */
	/* Enable quasi-dynamic register programming*/
	start_sw_done(priv->ctl);
	setbits_le32(&priv->ctl->dfimisc, DDRCTRL_DFIMISC_DFI_INIT_COMPLETE_EN);
	wait_sw_done_ack(priv->ctl);

/*  7. Wait for DWC_ddr_umctl2 to move to normal operation mode
 *     by monitoring STAT.operating_mode signal
 */
	/* wait uMCTL2 ready */

	wait_operating_mode(priv, DDRCTRL_STAT_OPERATING_MODE_NORMAL);

	debug("DDR DQS training : ");
/*  8. Disable Auto refresh and power down by setting
 *    - RFSHCTL3.dis_au_refresh = 1
 *    - PWRCTL.powerdown_en = 0
 *    - DFIMISC.dfiinit_complete_en = 0
 */
	stm32mp1_refresh_disable(priv->ctl);

/*  9. Program PUBL PGCR to enable refresh during training and rank to train
 *     not done => keep the programed value in PGCR
 */

/* 10. configure PUBL PIR register to specify which training step to run */
	/* warning : RVTRN  is not supported by this PUBL */
	stm32mp1_ddrphy_init(priv->phy, DDRPHYC_PIR_QSTRN);

/* 11. monitor PUB PGSR.IDONE to poll cpmpletion of training sequence */
	ddrphy_idone_wait(priv->phy);

/* 12. set back registers in step 8 to the orginal values if desidered */
	stm32mp1_refresh_restore(priv->ctl, config->c_reg.rfshctl3,
				 config->c_reg.pwrctl);

	/* enable uMCTL2 AXI port 0 and 1 */
	setbits_le32(&priv->ctl->pctrl_0, DDRCTRL_PCTRL_N_PORT_EN);
	setbits_le32(&priv->ctl->pctrl_1, DDRCTRL_PCTRL_N_PORT_EN);

	if (INTERACTIVE(STEP_DDR_READY))
		goto start;
}
