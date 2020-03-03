/* drivers/devfreq/exynos4210_memorybus.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *	MyungJoo Ham <myungjoo.ham@samsung.com>
 *
 * EXYNOS4 - Memory/Bus clock frequency scaling support in DEVFREQ framework
 *	This version supports EXYNOS4210 only. This changes bus frequencies
 *	and vddint voltages. Exynos4412/4212 should be able to be supported
 *	with minor modifications.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/io.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/suspend.h>
#include <linux/pm_opp.h>
#include <linux/devfreq.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/module.h>

#include <mach/map.h>

#include "exynos_ppmu.h"
#include "exynos4_bus.h"

#define MAX_SAFEVOLT	1200000 /* 1.2V */

enum exynos4_busf_type {
	TYPE_BUSF_EXYNOS4210,
	TYPE_BUSF_EXYNOS4x12,
};

/* Assume that the bus is saturated if the utilization is 40% */
#define BUS_SATURATION_RATIO	40

enum busclk_level_idx {
	LV_0 = 0,
	LV_1,
	LV_2,
	LV_3,
	LV_4,
	_LV_END
};

enum exynos_ppmu_idx {
	PPMU_DMC0,
	PPMU_DMC1,
	PPMU_END,
};

#define EX4210_LV_MAX	LV_2
#define EX4x12_LV_MAX	LV_4
#define EX4210_LV_NUM	(LV_2 + 1)
#define EX4x12_LV_NUM	(LV_4 + 1)

/**
 * struct busfreq_opp_info - opp information for bus
 * @rate:	Frequency in hertz
 * @volt:	Voltage in microvolts corresponding to this OPP
 */
struct busfreq_opp_info {
	unsigned long rate;
	unsigned long volt;
};

struct busfreq_data {
	enum exynos4_busf_type type;
	struct device *dev;
	struct devfreq *devfreq;
	bool disabled;
	struct regulator *vdd_int;
	struct regulator *vdd_mif; /* Exynos4412/4212 only */
	struct busfreq_opp_info curr_oppinfo;
	struct busfreq_ppmu_data ppmu_data;

	struct notifier_block pm_notifier;
	struct mutex lock;

	/* Dividers calculated at boot/probe-time */
	unsigned int dmc_divtable[_LV_END]; /* DMC0 */
	unsigned int top_divtable[_LV_END];
};

/* 4210 controls clock of mif and voltage of int */
static struct bus_opp_table exynos4210_busclk_table[] = {
	{LV_0, 400000, 1150000},
	{LV_1, 267000, 1050000},
	{LV_2, 133000, 1025000},
	{0, 0, 0},
};

/*
 * MIF is the main control knob clock for Exynos4x12 MIF/INT
 * clock and voltage of both mif/int are controlled.
 */
static struct bus_opp_table exynos4x12_mifclk_table[] = {
	{LV_0, 400000, 1100000},
	{LV_1, 267000, 1000000},
	{LV_2, 160000, 950000},
	{LV_3, 133000, 950000},
	{LV_4, 100000, 950000},
	{0, 0, 0},
};

/*
 * INT is not the control knob of 4x12. LV_x is not meant to represent
 * the current performance. (MIF does)
 */
static struct bus_opp_table exynos4x12_intclk_table[] = {
	{LV_0, 200000, 1000000},
	{LV_1, 160000, 950000},
	{LV_2, 133000, 925000},
	{LV_3, 100000, 900000},
	{0, 0, 0},
};

/* TODO: asv volt definitions are "__initdata"? */
/* Some chips have different operating voltages */
static unsigned int exynos4210_asv_volt[][EX4210_LV_NUM] = {
	{1150000, 1050000, 1050000},
	{1125000, 1025000, 1025000},
	{1100000, 1000000, 1000000},
	{1075000, 975000, 975000},
	{1050000, 950000, 950000},
};

static unsigned int exynos4x12_mif_step_50[][EX4x12_LV_NUM] = {
	/* 400      267     160     133     100 */
	{1050000, 950000, 900000, 900000, 900000}, /* ASV0 */
	{1050000, 950000, 900000, 900000, 900000}, /* ASV1 */
	{1050000, 950000, 900000, 900000, 900000}, /* ASV2 */
	{1050000, 900000, 900000, 900000, 900000}, /* ASV3 */
	{1050000, 900000, 900000, 900000, 850000}, /* ASV4 */
	{1050000, 900000, 900000, 850000, 850000}, /* ASV5 */
	{1050000, 900000, 850000, 850000, 850000}, /* ASV6 */
	{1050000, 900000, 850000, 850000, 850000}, /* ASV7 */
	{1050000, 900000, 850000, 850000, 850000}, /* ASV8 */
};

static unsigned int exynos4x12_int_volt[][EX4x12_LV_NUM] = {
	/* 200    160      133     100 */
	{1000000, 950000, 925000, 900000}, /* ASV0 */
	{975000,  925000, 925000, 900000}, /* ASV1 */
	{950000,  925000, 900000, 875000}, /* ASV2 */
	{950000,  900000, 900000, 875000}, /* ASV3 */
	{925000,  875000, 875000, 875000}, /* ASV4 */
	{900000,  850000, 850000, 850000}, /* ASV5 */
	{900000,  850000, 850000, 850000}, /* ASV6 */
	{900000,  850000, 850000, 850000}, /* ASV7 */
	{900000,  850000, 850000, 850000}, /* ASV8 */
};

/*** Clock Divider Data for Exynos4210 ***/
static unsigned int exynos4210_clkdiv_dmc0[][8] = {
	/*
	 * Clock divider value for following
	 * { DIVACP, DIVACP_PCLK, DIVDPHY, DIVDMC, DIVDMCD
	 *		DIVDMCP, DIVCOPY2, DIVCORE_TIMERS }
	 */

	/* DMC L0: 400MHz */
	{ 3, 1, 1, 1, 1, 1, 3, 1 },
	/* DMC L1: 266.7MHz */
	{ 4, 1, 1, 2, 1, 1, 3, 1 },
	/* DMC L2: 133MHz */
	{ 5, 1, 1, 5, 1, 1, 3, 1 },
};
static unsigned int exynos4210_clkdiv_top[][5] = {
	/*
	 * Clock divider value for following
	 * { DIVACLK200, DIVACLK100, DIVACLK160, DIVACLK133, DIVONENAND }
	 */
	/* ACLK200 L0: 200MHz */
	{ 3, 7, 4, 5, 1 },
	/* ACLK200 L1: 160MHz */
	{ 4, 7, 5, 6, 1 },
	/* ACLK200 L2: 133MHz */
	{ 5, 7, 7, 7, 1 },
};
static unsigned int exynos4210_clkdiv_lr_bus[][2] = {
	/*
	 * Clock divider value for following
	 * { DIVGDL/R, DIVGPL/R }
	 */
	/* ACLK_GDL/R L1: 200MHz */
	{ 3, 1 },
	/* ACLK_GDL/R L2: 160MHz */
	{ 4, 1 },
	/* ACLK_GDL/R L3: 133MHz */
	{ 5, 1 },
};

/*** Clock Divider Data for Exynos4212/4412 ***/
static unsigned int exynos4x12_clkdiv_dmc0[][6] = {
	/*
	 * Clock divider value for following
	 * { DIVACP, DIVACP_PCLK, DIVDPHY, DIVDMC, DIVDMCD
	 *              DIVDMCP}
	 */

	/* DMC L0: 400MHz */
	{3, 1, 1, 1, 1, 1},
	/* DMC L1: 266.7MHz */
	{4, 1, 1, 2, 1, 1},
	/* DMC L2: 160MHz */
	{5, 1, 1, 4, 1, 1},
	/* DMC L3: 133MHz */
	{5, 1, 1, 5, 1, 1},
	/* DMC L4: 100MHz */
	{7, 1, 1, 7, 1, 1},
};
static unsigned int exynos4x12_clkdiv_dmc1[][6] = {
	/*
	 * Clock divider value for following
	 * { G2DACP, DIVC2C, DIVC2C_ACLK }
	 */

	/* DMC L0: 400MHz */
	{3, 1, 1},
	/* DMC L1: 266.7MHz */
	{4, 2, 1},
	/* DMC L2: 160MHz */
	{5, 4, 1},
	/* DMC L3: 133MHz */
	{5, 5, 1},
	/* DMC L4: 100MHz */
	{7, 7, 1},
};
static unsigned int exynos4x12_clkdiv_top[][5] = {
	/*
	 * Clock divider value for following
	 * { DIVACLK266_GPS, DIVACLK100, DIVACLK160,
		DIVACLK133, DIVONENAND }
	 */

	/* ACLK_GDL/R L0: 200MHz */
	{2, 7, 4, 5, 1},
	/* ACLK_GDL/R L1: 200MHz */
	{2, 7, 4, 5, 1},
	/* ACLK_GDL/R L2: 160MHz */
	{4, 7, 5, 7, 1},
	/* ACLK_GDL/R L3: 133MHz */
	{4, 7, 5, 7, 1},
	/* ACLK_GDL/R L4: 100MHz */
	{7, 7, 7, 7, 1},
};
static unsigned int exynos4x12_clkdiv_lr_bus[][2] = {
	/*
	 * Clock divider value for following
	 * { DIVGDL/R, DIVGPL/R }
	 */

	/* ACLK_GDL/R L0: 200MHz */
	{3, 1},
	/* ACLK_GDL/R L1: 200MHz */
	{3, 1},
	/* ACLK_GDL/R L2: 160MHz */
	{4, 1},
	/* ACLK_GDL/R L3: 133MHz */
	{5, 1},
	/* ACLK_GDL/R L4: 100MHz */
	{7, 1},
};
static unsigned int exynos4x12_clkdiv_sclkip[][3] = {
	/*
	 * Clock divider value for following
	 * { DIVMFC, DIVJPEG, DIVFIMC0~3}
	 */

	/* SCLK_MFC: 200MHz */
	{3, 3, 4},
	/* SCLK_MFC: 200MHz */
	{3, 3, 4},
	/* SCLK_MFC: 160MHz */
	{4, 4, 5},
	/* SCLK_MFC: 133MHz */
	{5, 5, 5},
	/* SCLK_MFC: 100MHz */
	{7, 7, 7},
};


static int exynos4210_set_busclk(struct busfreq_data *data,
				 struct busfreq_opp_info *oppi)
{
	unsigned int index;
	unsigned int tmp;

	for (index = LV_0; index < EX4210_LV_NUM; index++)
		if (oppi->rate == exynos4210_busclk_table[index].clk)
			break;

	if (index == EX4210_LV_NUM)
		return -EINVAL;

	/* Change Divider - DMC0 */
	tmp = data->dmc_divtable[index];

	__raw_writel(tmp, EXYNOS4_CLKDIV_DMC0);

	do {
		tmp = __raw_readl(EXYNOS4_CLKDIV_STAT_DMC0);
	} while (tmp & 0x11111111);

	/* Change Divider - TOP */
	tmp = data->top_divtable[index];

	__raw_writel(tmp, EXYNOS4_CLKDIV_TOP);

	do {
		tmp = __raw_readl(EXYNOS4_CLKDIV_STAT_TOP);
	} while (tmp & 0x11111);

	/* Change Divider - LEFTBUS */
	tmp = __raw_readl(EXYNOS4_CLKDIV_LEFTBUS);

	tmp &= ~(EXYNOS4_CLKDIV_BUS_GDLR_MASK | EXYNOS4_CLKDIV_BUS_GPLR_MASK);

	tmp |= ((exynos4210_clkdiv_lr_bus[index][0] <<
				EXYNOS4_CLKDIV_BUS_GDLR_SHIFT) |
		(exynos4210_clkdiv_lr_bus[index][1] <<
				EXYNOS4_CLKDIV_BUS_GPLR_SHIFT));

	__raw_writel(tmp, EXYNOS4_CLKDIV_LEFTBUS);

	do {
		tmp = __raw_readl(EXYNOS4_CLKDIV_STAT_LEFTBUS);
	} while (tmp & 0x11);

	/* Change Divider - RIGHTBUS */
	tmp = __raw_readl(EXYNOS4_CLKDIV_RIGHTBUS);

	tmp &= ~(EXYNOS4_CLKDIV_BUS_GDLR_MASK | EXYNOS4_CLKDIV_BUS_GPLR_MASK);

	tmp |= ((exynos4210_clkdiv_lr_bus[index][0] <<
				EXYNOS4_CLKDIV_BUS_GDLR_SHIFT) |
		(exynos4210_clkdiv_lr_bus[index][1] <<
				EXYNOS4_CLKDIV_BUS_GPLR_SHIFT));

	__raw_writel(tmp, EXYNOS4_CLKDIV_RIGHTBUS);

	do {
		tmp = __raw_readl(EXYNOS4_CLKDIV_STAT_RIGHTBUS);
	} while (tmp & 0x11);

	return 0;
}

static int exynos4x12_set_busclk(struct busfreq_data *data,
				 struct busfreq_opp_info *oppi)
{
	unsigned int index;
	unsigned int tmp;

	for (index = LV_0; index < EX4x12_LV_NUM; index++)
		if (oppi->rate == exynos4x12_mifclk_table[index].clk)
			break;

	if (index == EX4x12_LV_NUM)
		return -EINVAL;

	/* Change Divider - DMC0 */
	tmp = data->dmc_divtable[index];

	__raw_writel(tmp, EXYNOS4_CLKDIV_DMC0);

	do {
		tmp = __raw_readl(EXYNOS4_CLKDIV_STAT_DMC0);
	} while (tmp & 0x11111111);

	/* Change Divider - DMC1 */
	tmp = __raw_readl(EXYNOS4_CLKDIV_DMC1);

	tmp &= ~(EXYNOS4_CLKDIV_DMC1_G2D_ACP_MASK |
		EXYNOS4_CLKDIV_DMC1_C2C_MASK |
		EXYNOS4_CLKDIV_DMC1_C2CACLK_MASK);

	tmp |= ((exynos4x12_clkdiv_dmc1[index][0] <<
				EXYNOS4_CLKDIV_DMC1_G2D_ACP_SHIFT) |
		(exynos4x12_clkdiv_dmc1[index][1] <<
				EXYNOS4_CLKDIV_DMC1_C2C_SHIFT) |
		(exynos4x12_clkdiv_dmc1[index][2] <<
				EXYNOS4_CLKDIV_DMC1_C2CACLK_SHIFT));

	__raw_writel(tmp, EXYNOS4_CLKDIV_DMC1);

	do {
		tmp = __raw_readl(EXYNOS4_CLKDIV_STAT_DMC1);
	} while (tmp & 0x111111);

	/* Change Divider - TOP */
	tmp = __raw_readl(EXYNOS4_CLKDIV_TOP);

	tmp &= ~(EXYNOS4_CLKDIV_TOP_ACLK266_GPS_MASK |
		EXYNOS4_CLKDIV_TOP_ACLK100_MASK |
		EXYNOS4_CLKDIV_TOP_ACLK160_MASK |
		EXYNOS4_CLKDIV_TOP_ACLK133_MASK |
		EXYNOS4_CLKDIV_TOP_ONENAND_MASK);

	tmp |= ((exynos4x12_clkdiv_top[index][0] <<
				EXYNOS4_CLKDIV_TOP_ACLK266_GPS_SHIFT) |
		(exynos4x12_clkdiv_top[index][1] <<
				EXYNOS4_CLKDIV_TOP_ACLK100_SHIFT) |
		(exynos4x12_clkdiv_top[index][2] <<
				EXYNOS4_CLKDIV_TOP_ACLK160_SHIFT) |
		(exynos4x12_clkdiv_top[index][3] <<
				EXYNOS4_CLKDIV_TOP_ACLK133_SHIFT) |
		(exynos4x12_clkdiv_top[index][4] <<
				EXYNOS4_CLKDIV_TOP_ONENAND_SHIFT));

	__raw_writel(tmp, EXYNOS4_CLKDIV_TOP);

	do {
		tmp = __raw_readl(EXYNOS4_CLKDIV_STAT_TOP);
	} while (tmp & 0x11111);

	/* Change Divider - LEFTBUS */
	tmp = __raw_readl(EXYNOS4_CLKDIV_LEFTBUS);

	tmp &= ~(EXYNOS4_CLKDIV_BUS_GDLR_MASK | EXYNOS4_CLKDIV_BUS_GPLR_MASK);

	tmp |= ((exynos4x12_clkdiv_lr_bus[index][0] <<
				EXYNOS4_CLKDIV_BUS_GDLR_SHIFT) |
		(exynos4x12_clkdiv_lr_bus[index][1] <<
				EXYNOS4_CLKDIV_BUS_GPLR_SHIFT));

	__raw_writel(tmp, EXYNOS4_CLKDIV_LEFTBUS);

	do {
		tmp = __raw_readl(EXYNOS4_CLKDIV_STAT_LEFTBUS);
	} while (tmp & 0x11);

	/* Change Divider - RIGHTBUS */
	tmp = __raw_readl(EXYNOS4_CLKDIV_RIGHTBUS);

	tmp &= ~(EXYNOS4_CLKDIV_BUS_GDLR_MASK | EXYNOS4_CLKDIV_BUS_GPLR_MASK);

	tmp |= ((exynos4x12_clkdiv_lr_bus[index][0] <<
				EXYNOS4_CLKDIV_BUS_GDLR_SHIFT) |
		(exynos4x12_clkdiv_lr_bus[index][1] <<
				EXYNOS4_CLKDIV_BUS_GPLR_SHIFT));

	__raw_writel(tmp, EXYNOS4_CLKDIV_RIGHTBUS);

	do {
		tmp = __raw_readl(EXYNOS4_CLKDIV_STAT_RIGHTBUS);
	} while (tmp & 0x11);

	/* Change Divider - MFC */
	tmp = __raw_readl(EXYNOS4_CLKDIV_MFC);

	tmp &= ~(EXYNOS4_CLKDIV_MFC_MASK);

	tmp |= ((exynos4x12_clkdiv_sclkip[index][0] <<
				EXYNOS4_CLKDIV_MFC_SHIFT));

	__raw_writel(tmp, EXYNOS4_CLKDIV_MFC);

	do {
		tmp = __raw_readl(EXYNOS4_CLKDIV_STAT_MFC);
	} while (tmp & 0x1);

	/* Change Divider - JPEG */
	tmp = __raw_readl(EXYNOS4_CLKDIV_CAM1);

	tmp &= ~(EXYNOS4_CLKDIV_CAM1_JPEG_MASK);

	tmp |= ((exynos4x12_clkdiv_sclkip[index][1] <<
				EXYNOS4_CLKDIV_CAM1_JPEG_SHIFT));

	__raw_writel(tmp, EXYNOS4_CLKDIV_CAM1);

	do {
		tmp = __raw_readl(EXYNOS4_CLKDIV_STAT_CAM1);
	} while (tmp & 0x1);

	/* Change Divider - FIMC0~3 */
	tmp = __raw_readl(EXYNOS4_CLKDIV_CAM);

	tmp &= ~(EXYNOS4_CLKDIV_CAM_FIMC0_MASK | EXYNOS4_CLKDIV_CAM_FIMC1_MASK |
		EXYNOS4_CLKDIV_CAM_FIMC2_MASK | EXYNOS4_CLKDIV_CAM_FIMC3_MASK);

	tmp |= ((exynos4x12_clkdiv_sclkip[index][2] <<
				EXYNOS4_CLKDIV_CAM_FIMC0_SHIFT) |
		(exynos4x12_clkdiv_sclkip[index][2] <<
				EXYNOS4_CLKDIV_CAM_FIMC1_SHIFT) |
		(exynos4x12_clkdiv_sclkip[index][2] <<
				EXYNOS4_CLKDIV_CAM_FIMC2_SHIFT) |
		(exynos4x12_clkdiv_sclkip[index][2] <<
				EXYNOS4_CLKDIV_CAM_FIMC3_SHIFT));

	__raw_writel(tmp, EXYNOS4_CLKDIV_CAM);

	do {
		tmp = __raw_readl(EXYNOS4_CLKDIV_STAT_CAM1);
	} while (tmp & 0x1111);

	return 0;
}

static int exynos4x12_get_intspec(unsigned long mifclk)
{
	int i = 0;

	while (exynos4x12_intclk_table[i].clk) {
		if (exynos4x12_intclk_table[i].clk <= mifclk)
			return i;
		i++;
	}

	return -EINVAL;
}

static int exynos4_bus_setvolt(struct busfreq_data *data,
			       struct busfreq_opp_info *oppi,
			       struct busfreq_opp_info *oldoppi)
{
	int err = 0, tmp;
	unsigned long volt = oppi->volt;

	switch (data->type) {
	case TYPE_BUSF_EXYNOS4210:
		/* OPP represents DMC clock + INT voltage */
		err = regulator_set_voltage(data->vdd_int, volt,
					    MAX_SAFEVOLT);
		break;
	case TYPE_BUSF_EXYNOS4x12:
		/* OPP represents MIF clock + MIF voltage */
		err = regulator_set_voltage(data->vdd_mif, volt,
					    MAX_SAFEVOLT);
		if (err)
			break;

		tmp = exynos4x12_get_intspec(oppi->rate);
		if (tmp < 0) {
			err = tmp;
			regulator_set_voltage(data->vdd_mif,
					      oldoppi->volt,
					      MAX_SAFEVOLT);
			break;
		}
		err = regulator_set_voltage(data->vdd_int,
					    exynos4x12_intclk_table[tmp].volt,
					    MAX_SAFEVOLT);
		/*  Try to recover */
		if (err)
			regulator_set_voltage(data->vdd_mif,
					      oldoppi->volt,
					      MAX_SAFEVOLT);
		break;
	default:
		err = -EINVAL;
	}

	return err;
}

static int exynos4_bus_target(struct device *dev, unsigned long *_freq,
			      u32 flags)
{
	int err = 0;
	struct platform_device *pdev = container_of(dev, struct platform_device,
						    dev);
	struct busfreq_data *data = platform_get_drvdata(pdev);
	struct dev_pm_opp *opp;
	unsigned long freq;
	unsigned long old_freq = data->curr_oppinfo.rate;
	struct busfreq_opp_info	new_oppinfo;

	rcu_read_lock();
	opp = devfreq_recommended_opp(dev, _freq, flags);
	if (IS_ERR(opp)) {
		rcu_read_unlock();
		return PTR_ERR(opp);
	}
	new_oppinfo.rate = dev_pm_opp_get_freq(opp);
	new_oppinfo.volt = dev_pm_opp_get_voltage(opp);
	rcu_read_unlock();
	freq = new_oppinfo.rate;

	if (old_freq == freq)
		return 0;

	dev_dbg(dev, "targeting %lukHz %luuV\n", freq, new_oppinfo.volt);

	mutex_lock(&data->lock);

	if (data->disabled)
		goto out;

	if (old_freq < freq)
		err = exynos4_bus_setvolt(data, &new_oppinfo,
					  &data->curr_oppinfo);
	if (err)
		goto out;

	if (old_freq != freq) {
		switch (data->type) {
		case TYPE_BUSF_EXYNOS4210:
			err = exynos4210_set_busclk(data, &new_oppinfo);
			break;
		case TYPE_BUSF_EXYNOS4x12:
			err = exynos4x12_set_busclk(data, &new_oppinfo);
			break;
		default:
			err = -EINVAL;
		}
	}
	if (err)
		goto out;

	if (old_freq > freq)
		err = exynos4_bus_setvolt(data, &new_oppinfo,
					  &data->curr_oppinfo);
	if (err)
		goto out;

	data->curr_oppinfo = new_oppinfo;
out:
	mutex_unlock(&data->lock);
	return err;
}

static int exynos4_bus_get_dev_status(struct device *dev,
				      struct devfreq_dev_status *stat)
{
	struct busfreq_data *data = dev_get_drvdata(dev);
	struct busfreq_ppmu_data *ppmu_data = &data->ppmu_data;
	int busier;

	exynos_read_ppmu(ppmu_data);
	busier = exynos_get_busier_ppmu(ppmu_data);
	stat->current_frequency = data->curr_oppinfo.rate;

	/* Number of cycles spent on memory access */
	stat->busy_time = ppmu_data->ppmu[busier].count[PPMU_PMNCNT3];
	stat->busy_time *= 100 / BUS_SATURATION_RATIO;
	stat->total_time = ppmu_data->ppmu[busier].ccnt;

	/* If the counters have overflown, retry */
	if (ppmu_data->ppmu[busier].ccnt_overflow ||
	    ppmu_data->ppmu[busier].count_overflow[0])
		return -EAGAIN;

	return 0;
}

static struct devfreq_dev_profile exynos4_devfreq_profile = {
	.initial_freq	= 400000,
	.polling_ms	= 50,
	.target		= exynos4_bus_target,
	.get_dev_status	= exynos4_bus_get_dev_status,
};

static int exynos4210_init_tables(struct busfreq_data *data)
{
	u32 tmp;
	int mgrp;
	int i, err = 0;

	tmp = __raw_readl(EXYNOS4_CLKDIV_DMC0);
	for (i = LV_0; i < EX4210_LV_NUM; i++) {
		tmp &= ~(EXYNOS4_CLKDIV_DMC0_ACP_MASK |
			EXYNOS4_CLKDIV_DMC0_ACPPCLK_MASK |
			EXYNOS4_CLKDIV_DMC0_DPHY_MASK |
			EXYNOS4_CLKDIV_DMC0_DMC_MASK |
			EXYNOS4_CLKDIV_DMC0_DMCD_MASK |
			EXYNOS4_CLKDIV_DMC0_DMCP_MASK |
			EXYNOS4_CLKDIV_DMC0_COPY2_MASK |
			EXYNOS4_CLKDIV_DMC0_CORETI_MASK);

		tmp |= ((exynos4210_clkdiv_dmc0[i][0] <<
					EXYNOS4_CLKDIV_DMC0_ACP_SHIFT) |
			(exynos4210_clkdiv_dmc0[i][1] <<
					EXYNOS4_CLKDIV_DMC0_ACPPCLK_SHIFT) |
			(exynos4210_clkdiv_dmc0[i][2] <<
					EXYNOS4_CLKDIV_DMC0_DPHY_SHIFT) |
			(exynos4210_clkdiv_dmc0[i][3] <<
					EXYNOS4_CLKDIV_DMC0_DMC_SHIFT) |
			(exynos4210_clkdiv_dmc0[i][4] <<
					EXYNOS4_CLKDIV_DMC0_DMCD_SHIFT) |
			(exynos4210_clkdiv_dmc0[i][5] <<
					EXYNOS4_CLKDIV_DMC0_DMCP_SHIFT) |
			(exynos4210_clkdiv_dmc0[i][6] <<
					EXYNOS4_CLKDIV_DMC0_COPY2_SHIFT) |
			(exynos4210_clkdiv_dmc0[i][7] <<
					EXYNOS4_CLKDIV_DMC0_CORETI_SHIFT));

		data->dmc_divtable[i] = tmp;
	}

	tmp = __raw_readl(EXYNOS4_CLKDIV_TOP);
	for (i = LV_0; i <  EX4210_LV_NUM; i++) {
		tmp &= ~(EXYNOS4_CLKDIV_TOP_ACLK200_MASK |
			EXYNOS4_CLKDIV_TOP_ACLK100_MASK |
			EXYNOS4_CLKDIV_TOP_ACLK160_MASK |
			EXYNOS4_CLKDIV_TOP_ACLK133_MASK |
			EXYNOS4_CLKDIV_TOP_ONENAND_MASK);

		tmp |= ((exynos4210_clkdiv_top[i][0] <<
					EXYNOS4_CLKDIV_TOP_ACLK200_SHIFT) |
			(exynos4210_clkdiv_top[i][1] <<
					EXYNOS4_CLKDIV_TOP_ACLK100_SHIFT) |
			(exynos4210_clkdiv_top[i][2] <<
					EXYNOS4_CLKDIV_TOP_ACLK160_SHIFT) |
			(exynos4210_clkdiv_top[i][3] <<
					EXYNOS4_CLKDIV_TOP_ACLK133_SHIFT) |
			(exynos4210_clkdiv_top[i][4] <<
					EXYNOS4_CLKDIV_TOP_ONENAND_SHIFT));

		data->top_divtable[i] = tmp;
	}

	/*
	 * TODO: init tmp based on busfreq_data
	 * (device-tree or platform-data)
	 */
	tmp = 0; /* Max voltages for the reliability of the unknown */

	pr_debug("ASV Group of Exynos4 is %d\n", tmp);
	/* Use merged grouping for voltage */
	switch (tmp) {
	case 0:
		mgrp = 0;
		break;
	case 1:
	case 2:
		mgrp = 1;
		break;
	case 3:
	case 4:
		mgrp = 2;
		break;
	case 5:
	case 6:
		mgrp = 3;
		break;
	case 7:
		mgrp = 4;
		break;
	default:
		pr_warn("Unknown ASV Group. Use max voltage.\n");
		mgrp = 0;
	}

	for (i = LV_0; i < EX4210_LV_NUM; i++)
		exynos4210_busclk_table[i].volt = exynos4210_asv_volt[mgrp][i];

	for (i = LV_0; i < EX4210_LV_NUM; i++) {
		err = dev_pm_opp_add(data->dev, exynos4210_busclk_table[i].clk,
			      exynos4210_busclk_table[i].volt);
		if (err) {
			dev_err(data->dev, "Cannot add opp entries.\n");
			return err;
		}
	}


	return 0;
}

static int exynos4x12_init_tables(struct busfreq_data *data)
{
	unsigned int i;
	unsigned int tmp;
	int ret;

	/* Enable pause function for DREX2 DVFS */
	tmp = __raw_readl(EXYNOS4_DMC_PAUSE_CTRL);
	tmp |= EXYNOS4_DMC_PAUSE_ENABLE;
	__raw_writel(tmp, EXYNOS4_DMC_PAUSE_CTRL);

	tmp = __raw_readl(EXYNOS4_CLKDIV_DMC0);

	for (i = 0; i <  EX4x12_LV_NUM; i++) {
		tmp &= ~(EXYNOS4_CLKDIV_DMC0_ACP_MASK |
			EXYNOS4_CLKDIV_DMC0_ACPPCLK_MASK |
			EXYNOS4_CLKDIV_DMC0_DPHY_MASK |
			EXYNOS4_CLKDIV_DMC0_DMC_MASK |
			EXYNOS4_CLKDIV_DMC0_DMCD_MASK |
			EXYNOS4_CLKDIV_DMC0_DMCP_MASK);

		tmp |= ((exynos4x12_clkdiv_dmc0[i][0] <<
					EXYNOS4_CLKDIV_DMC0_ACP_SHIFT) |
			(exynos4x12_clkdiv_dmc0[i][1] <<
					EXYNOS4_CLKDIV_DMC0_ACPPCLK_SHIFT) |
			(exynos4x12_clkdiv_dmc0[i][2] <<
					EXYNOS4_CLKDIV_DMC0_DPHY_SHIFT) |
			(exynos4x12_clkdiv_dmc0[i][3] <<
					EXYNOS4_CLKDIV_DMC0_DMC_SHIFT) |
			(exynos4x12_clkdiv_dmc0[i][4] <<
					EXYNOS4_CLKDIV_DMC0_DMCD_SHIFT) |
			(exynos4x12_clkdiv_dmc0[i][5] <<
					EXYNOS4_CLKDIV_DMC0_DMCP_SHIFT));

		data->dmc_divtable[i] = tmp;
	}

	tmp = 0; /* Max voltages for the reliability of the unknown */

	if (tmp > 8)
		tmp = 0;
	pr_debug("ASV Group of Exynos4x12 is %d\n", tmp);

	for (i = 0; i < EX4x12_LV_NUM; i++) {
		exynos4x12_mifclk_table[i].volt =
			exynos4x12_mif_step_50[tmp][i];
		exynos4x12_intclk_table[i].volt =
			exynos4x12_int_volt[tmp][i];
	}

	for (i = 0; i < EX4x12_LV_NUM; i++) {
		ret = dev_pm_opp_add(data->dev, exynos4x12_mifclk_table[i].clk,
			      exynos4x12_mifclk_table[i].volt);
		if (ret) {
			dev_err(data->dev, "Fail to add opp entries.\n");
			return ret;
		}
	}

	return 0;
}

static int exynos4_busfreq_pm_notifier_event(struct notifier_block *this,
		unsigned long event, void *ptr)
{
	struct busfreq_data *data = container_of(this, struct busfreq_data,
						 pm_notifier);
	struct dev_pm_opp *opp;
	struct busfreq_opp_info	new_oppinfo;
	unsigned long maxfreq = ULONG_MAX;
	int err = 0;

	switch (event) {
	case PM_SUSPEND_PREPARE:
		/* Set Fastest and Deactivate DVFS */
		mutex_lock(&data->lock);

		data->disabled = true;

		rcu_read_lock();
		opp = dev_pm_opp_find_freq_floor(data->dev, &maxfreq);
		if (IS_ERR(opp)) {
			rcu_read_unlock();
			dev_err(data->dev, "%s: unable to find a min freq\n",
				__func__);
			mutex_unlock(&data->lock);
			return PTR_ERR(opp);
		}
		new_oppinfo.rate = dev_pm_opp_get_freq(opp);
		new_oppinfo.volt = dev_pm_opp_get_voltage(opp);
		rcu_read_unlock();

		err = exynos4_bus_setvolt(data, &new_oppinfo,
					  &data->curr_oppinfo);
		if (err)
			goto unlock;

		switch (data->type) {
		case TYPE_BUSF_EXYNOS4210:
			err = exynos4210_set_busclk(data, &new_oppinfo);
			break;
		case TYPE_BUSF_EXYNOS4x12:
			err = exynos4x12_set_busclk(data, &new_oppinfo);
			break;
		default:
			err = -EINVAL;
		}
		if (err)
			goto unlock;

		data->curr_oppinfo = new_oppinfo;
unlock:
		mutex_unlock(&data->lock);
		if (err)
			return err;
		return NOTIFY_OK;
	case PM_POST_RESTORE:
	case PM_POST_SUSPEND:
		/* Reactivate */
		mutex_lock(&data->lock);
		data->disabled = false;
		mutex_unlock(&data->lock);
		return NOTIFY_OK;
	}

	return NOTIFY_DONE;
}

static int exynos4_busfreq_probe(struct platform_device *pdev)
{
	struct busfreq_data *data;
	struct busfreq_ppmu_data *ppmu_data;
	struct dev_pm_opp *opp;
	struct device *dev = &pdev->dev;
	int err = 0;

	data = devm_kzalloc(&pdev->dev, sizeof(struct busfreq_data), GFP_KERNEL);
	if (data == NULL) {
		dev_err(dev, "Cannot allocate memory.\n");
		return -ENOMEM;
	}

	ppmu_data = &data->ppmu_data;
	ppmu_data->ppmu_end = PPMU_END;
	ppmu_data->ppmu = devm_kzalloc(dev,
				       sizeof(struct exynos_ppmu) * PPMU_END,
				       GFP_KERNEL);
	if (!ppmu_data->ppmu) {
		dev_err(dev, "Failed to allocate memory for exynos_ppmu\n");
		return -ENOMEM;
	}

	data->type = pdev->id_entry->driver_data;
	ppmu_data->ppmu[PPMU_DMC0].hw_base = S5P_VA_DMC0;
	ppmu_data->ppmu[PPMU_DMC1].hw_base = S5P_VA_DMC1;
	data->pm_notifier.notifier_call = exynos4_busfreq_pm_notifier_event;
	data->dev = dev;
	mutex_init(&data->lock);

	switch (data->type) {
	case TYPE_BUSF_EXYNOS4210:
		err = exynos4210_init_tables(data);
		break;
	case TYPE_BUSF_EXYNOS4x12:
		err = exynos4x12_init_tables(data);
		break;
	default:
		dev_err(dev, "Cannot determine the device id %d\n", data->type);
		err = -EINVAL;
	}
	if (err) {
		dev_err(dev, "Cannot initialize busfreq table %d\n",
			     data->type);
		return err;
	}

	data->vdd_int = devm_regulator_get(dev, "vdd_int");
	if (IS_ERR(data->vdd_int)) {
		dev_err(dev, "Cannot get the regulator \"vdd_int\"\n");
		return PTR_ERR(data->vdd_int);
	}
	if (data->type == TYPE_BUSF_EXYNOS4x12) {
		data->vdd_mif = devm_regulator_get(dev, "vdd_mif");
		if (IS_ERR(data->vdd_mif)) {
			dev_err(dev, "Cannot get the regulator \"vdd_mif\"\n");
			return PTR_ERR(data->vdd_mif);
		}
	}

	rcu_read_lock();
	opp = dev_pm_opp_find_freq_floor(dev,
					 &exynos4_devfreq_profile.initial_freq);
	if (IS_ERR(opp)) {
		rcu_read_unlock();
		dev_err(dev, "Invalid initial frequency %lu kHz.\n",
			exynos4_devfreq_profile.initial_freq);
		return PTR_ERR(opp);
	}
	data->curr_oppinfo.rate = dev_pm_opp_get_freq(opp);
	data->curr_oppinfo.volt = dev_pm_opp_get_voltage(opp);
	rcu_read_unlock();

	platform_set_drvdata(pdev, data);

	data->devfreq = devm_devfreq_add_device(dev, &exynos4_devfreq_profile,
					   "simple_ondemand", NULL);
	if (IS_ERR(data->devfreq))
		return PTR_ERR(data->devfreq);

	/*
	 * Start PPMU (Performance Profiling Monitoring Unit) to check
	 * utilization of each IP in the Exynos4 SoC.
	 */
	busfreq_mon_reset(ppmu_data);

	/* Register opp_notifier for Exynos4 busfreq */
	err = devm_devfreq_register_opp_notifier(dev, data->devfreq);
	if (err < 0) {
		dev_err(dev, "Failed to register opp notifier\n");
		return err;
	}

	/* Register pm_notifier for Exynos4 busfreq */
	err = register_pm_notifier(&data->pm_notifier);
	if (err) {
		dev_err(dev, "Failed to setup pm notifier\n");
		return err;
	}

	return 0;
}

static int exynos4_busfreq_remove(struct platform_device *pdev)
{
	struct busfreq_data *data = platform_get_drvdata(pdev);

	/* Unregister all of notifier chain */
	unregister_pm_notifier(&data->pm_notifier);

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int exynos4_busfreq_resume(struct device *dev)
{
	struct busfreq_data *data = dev_get_drvdata(dev);
	struct busfreq_ppmu_data *ppmu_data = &data->ppmu_data;

	busfreq_mon_reset(ppmu_data);
	return 0;
}
#endif

static SIMPLE_DEV_PM_OPS(exynos4_busfreq_pm_ops, NULL, exynos4_busfreq_resume);

static const struct platform_device_id exynos4_busfreq_id[] = {
	{ "exynos4210-busfreq", TYPE_BUSF_EXYNOS4210 },
	{ "exynos4412-busfreq", TYPE_BUSF_EXYNOS4x12 },
	{ "exynos4212-busfreq", TYPE_BUSF_EXYNOS4x12 },
	{ },
};

static struct platform_driver exynos4_busfreq_driver = {
	.probe	= exynos4_busfreq_probe,
	.remove	= exynos4_busfreq_remove,
	.id_table = exynos4_busfreq_id,
	.driver = {
		.name	= "exynos4-busfreq",
		.pm	= &exynos4_busfreq_pm_ops,
	},
};

static int __init exynos4_busfreq_init(void)
{
	return platform_driver_register(&exynos4_busfreq_driver);
}
late_initcall(exynos4_busfreq_init);

static void __exit exynos4_busfreq_exit(void)
{
	platform_driver_unregister(&exynos4_busfreq_driver);
}
module_exit(exynos4_busfreq_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("EXYNOS4 busfreq driver with devfreq framework");
MODULE_AUTHOR("MyungJoo Ham <myungjoo.ham@samsung.com>");
