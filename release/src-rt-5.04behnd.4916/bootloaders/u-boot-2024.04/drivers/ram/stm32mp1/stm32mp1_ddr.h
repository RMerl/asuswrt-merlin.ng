/* SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause */
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#ifndef _RAM_STM32MP1_DDR_H
#define _RAM_STM32MP1_DDR_H

enum stm32mp1_ddr_interact_step {
	STEP_DDR_RESET,
	STEP_CTL_INIT,
	STEP_PHY_INIT,
	STEP_DDR_READY,
	STEP_RUN,
};

/* DDR CTL and DDR PHY REGISTERS */
struct stm32mp1_ddrctl;
struct stm32mp1_ddrphy;

/**
 * struct ddr_info
 *
 * @dev: pointer for the device
 * @info: UCLASS RAM information
 * @ctl: DDR controleur base address
 * @clk: DDR clock
 * @phy: DDR PHY base address
 * @rcc: rcc base address
 */
struct ddr_info {
	struct udevice *dev;
	struct ram_info info;
	struct clk clk;
	struct stm32mp1_ddrctl *ctl;
	struct stm32mp1_ddrphy *phy;
	u32 rcc;
};

struct stm32mp1_ddrctrl_reg {
	u32 mstr;
	u32 mrctrl0;
	u32 mrctrl1;
	u32 derateen;
	u32 derateint;
	u32 pwrctl;
	u32 pwrtmg;
	u32 hwlpctl;
	u32 rfshctl0;
	u32 rfshctl3;
	u32 crcparctl0;
	u32 zqctl0;
	u32 dfitmg0;
	u32 dfitmg1;
	u32 dfilpcfg0;
	u32 dfiupd0;
	u32 dfiupd1;
	u32 dfiupd2;
	u32 dfiphymstr;
	u32 odtmap;
	u32 dbg0;
	u32 dbg1;
	u32 dbgcmd;
	u32 poisoncfg;
	u32 pccfg;

};

struct stm32mp1_ddrctrl_timing {
	u32 rfshtmg;
	u32 dramtmg0;
	u32 dramtmg1;
	u32 dramtmg2;
	u32 dramtmg3;
	u32 dramtmg4;
	u32 dramtmg5;
	u32 dramtmg6;
	u32 dramtmg7;
	u32 dramtmg8;
	u32 dramtmg14;
	u32 odtcfg;
};

struct stm32mp1_ddrctrl_map {
	u32 addrmap1;
	u32 addrmap2;
	u32 addrmap3;
	u32 addrmap4;
	u32 addrmap5;
	u32 addrmap6;
	u32 addrmap9;
	u32 addrmap10;
	u32 addrmap11;
};

struct stm32mp1_ddrctrl_perf {
	u32 sched;
	u32 sched1;
	u32 perfhpr1;
	u32 perflpr1;
	u32 perfwr1;
	u32 pcfgr_0;
	u32 pcfgw_0;
	u32 pcfgqos0_0;
	u32 pcfgqos1_0;
	u32 pcfgwqos0_0;
	u32 pcfgwqos1_0;
	u32 pcfgr_1;
	u32 pcfgw_1;
	u32 pcfgqos0_1;
	u32 pcfgqos1_1;
	u32 pcfgwqos0_1;
	u32 pcfgwqos1_1;
};

struct stm32mp1_ddrphy_reg {
	u32 pgcr;
	u32 aciocr;
	u32 dxccr;
	u32 dsgcr;
	u32 dcr;
	u32 odtcr;
	u32 zq0cr1;
	u32 dx0gcr;
	u32 dx1gcr;
	u32 dx2gcr;
	u32 dx3gcr;
};

struct stm32mp1_ddrphy_timing {
	u32 ptr0;
	u32 ptr1;
	u32 ptr2;
	u32 dtpr0;
	u32 dtpr1;
	u32 dtpr2;
	u32 mr0;
	u32 mr1;
	u32 mr2;
	u32 mr3;
};

struct stm32mp1_ddr_info {
	const char *name;
	u32 speed; /* in kHZ */
	u32 size;  /* memory size in byte = col * row * width */
};

struct stm32mp1_ddr_config {
	struct stm32mp1_ddr_info info;
	struct stm32mp1_ddrctrl_reg c_reg;
	struct stm32mp1_ddrctrl_timing c_timing;
	struct stm32mp1_ddrctrl_map c_map;
	struct stm32mp1_ddrctrl_perf c_perf;
	struct stm32mp1_ddrphy_reg p_reg;
	struct stm32mp1_ddrphy_timing p_timing;
};

int stm32mp1_ddr_clk_enable(struct ddr_info *priv, u32 mem_speed);

void stm32mp1_ddr_init(
	struct ddr_info *priv,
	const struct stm32mp1_ddr_config *config);

int stm32mp1_dump_reg(const struct ddr_info *priv,
		      const char *name);

void stm32mp1_edit_reg(const struct ddr_info *priv,
		       char *name,
		       char *string);

int stm32mp1_dump_param(const struct stm32mp1_ddr_config *config,
			const char *name);

void stm32mp1_edit_param(const struct stm32mp1_ddr_config *config,
			 char *name,
			 char *string);

bool stm32mp1_ddr_interactive(
	void *priv,
	enum stm32mp1_ddr_interact_step step,
	const struct stm32mp1_ddr_config *config);

#endif
