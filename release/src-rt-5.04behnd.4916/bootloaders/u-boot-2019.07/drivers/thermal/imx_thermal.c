// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 Freescale Semiconductor, Inc.
 * Author: Nitin Garg <nitin.garg@freescale.com>
 *             Ye Li <Ye.Li@freescale.com>
 */

#include <config.h>
#include <common.h>
#include <div64.h>
#include <fuse.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <linux/math64.h>
#include <thermal.h>
#include <imx_thermal.h>

/* board will busyloop until this many degrees C below CPU max temperature */
#define TEMPERATURE_HOT_DELTA   5 /* CPU maxT - 5C */
#define FACTOR0			10000000
#define FACTOR1			15423
#define FACTOR2			4148468
#define OFFSET			3580661
#define MEASURE_FREQ		327
#define TEMPERATURE_MIN         -40
#define TEMPERATURE_HOT         85
#define TEMPERATURE_MAX         125

#define TEMPSENSE0_TEMP_CNT_SHIFT	8
#define TEMPSENSE0_TEMP_CNT_MASK	(0xfff << TEMPSENSE0_TEMP_CNT_SHIFT)
#define TEMPSENSE0_FINISHED		(1 << 2)
#define TEMPSENSE0_MEASURE_TEMP		(1 << 1)
#define TEMPSENSE0_POWER_DOWN		(1 << 0)
#define MISC0_REFTOP_SELBIASOFF		(1 << 3)
#define TEMPSENSE1_MEASURE_FREQ		0xffff

struct thermal_data {
	unsigned int fuse;
	int critical;
	int minc;
	int maxc;
};

#if defined(CONFIG_MX6)
static int read_cpu_temperature(struct udevice *dev)
{
	int temperature;
	unsigned int reg, n_meas;
	const struct imx_thermal_plat *pdata = dev_get_platdata(dev);
	struct anatop_regs *anatop = (struct anatop_regs *)pdata->regs;
	struct thermal_data *priv = dev_get_priv(dev);
	u32 fuse = priv->fuse;
	int t1, n1;
	s64 c1, c2;
	s64 temp64;
	s32 rem;

	/*
	 * Sensor data layout:
	 *   [31:20] - sensor value @ 25C
	 * We use universal formula now and only need sensor value @ 25C
	 * slope = 0.4445388 - (0.0016549 * 25C fuse)
	 */
	n1 = fuse >> 20;
	t1 = 25; /* t1 always 25C */

	/*
	 * Derived from linear interpolation:
	 * slope = 0.4445388 - (0.0016549 * 25C fuse)
	 * slope = (FACTOR2 - FACTOR1 * n1) / FACTOR0
	 * offset = 3.580661
	 * offset = OFFSET / 1000000
	 * (Nmeas - n1) / (Tmeas - t1 - offset) = slope
	 * We want to reduce this down to the minimum computation necessary
	 * for each temperature read.  Also, we want Tmeas in millicelsius
	 * and we don't want to lose precision from integer division. So...
	 * Tmeas = (Nmeas - n1) / slope + t1 + offset
	 * milli_Tmeas = 1000000 * (Nmeas - n1) / slope + 1000000 * t1 + OFFSET
	 * milli_Tmeas = -1000000 * (n1 - Nmeas) / slope + 1000000 * t1 + OFFSET
	 * Let constant c1 = (-1000000 / slope)
	 * milli_Tmeas = (n1 - Nmeas) * c1 + 1000000 * t1 + OFFSET
	 * Let constant c2 = n1 *c1 + 1000000 * t1
	 * milli_Tmeas = (c2 - Nmeas * c1) + OFFSET
	 * Tmeas = ((c2 - Nmeas * c1) + OFFSET) / 1000000
	 */
	temp64 = FACTOR0;
	temp64 *= 1000000;
	temp64 = div_s64_rem(temp64, FACTOR1 * n1 - FACTOR2, &rem);
	c1 = temp64;
	c2 = n1 * c1 + 1000000 * t1;

	/*
	 * now we only use single measure, every time we read
	 * the temperature, we will power on/down anadig thermal
	 * module
	 */
	writel(TEMPSENSE0_POWER_DOWN, &anatop->tempsense0_clr);
	writel(MISC0_REFTOP_SELBIASOFF, &anatop->ana_misc0_set);

	/* setup measure freq */
	reg = readl(&anatop->tempsense1);
	reg &= ~TEMPSENSE1_MEASURE_FREQ;
	reg |= MEASURE_FREQ;
	writel(reg, &anatop->tempsense1);

	/* start the measurement process */
	writel(TEMPSENSE0_MEASURE_TEMP, &anatop->tempsense0_clr);
	writel(TEMPSENSE0_FINISHED, &anatop->tempsense0_clr);
	writel(TEMPSENSE0_MEASURE_TEMP, &anatop->tempsense0_set);

	/* make sure that the latest temp is valid */
	while ((readl(&anatop->tempsense0) &
		TEMPSENSE0_FINISHED) == 0)
		udelay(10000);

	/* read temperature count */
	reg = readl(&anatop->tempsense0);
	n_meas = (reg & TEMPSENSE0_TEMP_CNT_MASK)
		>> TEMPSENSE0_TEMP_CNT_SHIFT;
	writel(TEMPSENSE0_FINISHED, &anatop->tempsense0_clr);

	/* Tmeas = (c2 - Nmeas * c1 + OFFSET) / 1000000 */
	temperature = div_s64_rem(c2 - n_meas * c1 + OFFSET, 1000000, &rem);

	/* power down anatop thermal sensor */
	writel(TEMPSENSE0_POWER_DOWN, &anatop->tempsense0_set);
	writel(MISC0_REFTOP_SELBIASOFF, &anatop->ana_misc0_clr);

	return temperature;
}
#elif defined(CONFIG_MX7)
static int read_cpu_temperature(struct udevice *dev)
{
	unsigned int reg, tmp;
	unsigned int raw_25c, te1;
	int temperature;
	unsigned int *priv = dev_get_priv(dev);
	u32 fuse = *priv;
	struct mxc_ccm_anatop_reg *ccm_anatop = (struct mxc_ccm_anatop_reg *)
						 ANATOP_BASE_ADDR;
	/*
	 * fuse data layout:
	 * [31:21] sensor value @ 25C
	 * [20:18] hot temperature value
	 * [17:9] sensor value of room
	 * [8:0] sensor value of hot
	 */

	raw_25c = fuse >> 21;
	if (raw_25c == 0)
		raw_25c = 25;

	te1 = (fuse >> 9) & 0x1ff;

	/*
	 * now we only use single measure, every time we read
	 * the temperature, we will power on/down anadig thermal
	 * module
	 */
	writel(TEMPMON_HW_ANADIG_TEMPSENSE1_POWER_DOWN_MASK, &ccm_anatop->tempsense1_clr);
	writel(PMU_REF_REFTOP_SELFBIASOFF_MASK, &ccm_anatop->ref_set);

	/* write measure freq */
	reg = readl(&ccm_anatop->tempsense1);
	reg &= ~TEMPMON_HW_ANADIG_TEMPSENSE1_MEASURE_FREQ_MASK;
	reg |= TEMPMON_HW_ANADIG_TEMPSENSE1_MEASURE_FREQ(MEASURE_FREQ);
	writel(reg, &ccm_anatop->tempsense1);

	writel(TEMPMON_HW_ANADIG_TEMPSENSE1_MEASURE_TEMP_MASK, &ccm_anatop->tempsense1_clr);
	writel(TEMPMON_HW_ANADIG_TEMPSENSE1_FINISHED_MASK, &ccm_anatop->tempsense1_clr);
	writel(TEMPMON_HW_ANADIG_TEMPSENSE1_MEASURE_TEMP_MASK, &ccm_anatop->tempsense1_set);

	if (soc_rev() >= CHIP_REV_1_1) {
		while ((readl(&ccm_anatop->tempsense1) &
		       TEMPMON_HW_ANADIG_TEMPSENSE1_FINISHED_MASK) == 0)
			;
		reg = readl(&ccm_anatop->tempsense1);
		tmp = (reg & TEMPMON_HW_ANADIG_TEMPSENSE1_TEMP_VALUE_MASK)
		       >> TEMPMON_HW_ANADIG_TEMPSENSE1_TEMP_VALUE_SHIFT;
	} else {
		/*
		 * Since we can not rely on finish bit, use 10ms
		 * delay to get temperature. From RM, 17us is
		 * enough to get data, but to gurantee to get
		 * the data, delay 10ms here.
		 */
		udelay(10000);
		reg = readl(&ccm_anatop->tempsense1);
		tmp = (reg & TEMPMON_HW_ANADIG_TEMPSENSE1_TEMP_VALUE_MASK)
		       >> TEMPMON_HW_ANADIG_TEMPSENSE1_TEMP_VALUE_SHIFT;
	}

	writel(TEMPMON_HW_ANADIG_TEMPSENSE1_FINISHED_MASK, &ccm_anatop->tempsense1_clr);

	/* power down anatop thermal sensor */
	writel(TEMPMON_HW_ANADIG_TEMPSENSE1_POWER_DOWN_MASK, &ccm_anatop->tempsense1_set);
	writel(PMU_REF_REFTOP_SELFBIASOFF_MASK, &ccm_anatop->ref_clr);

	/* Single point */
	temperature = tmp - (te1 - raw_25c);

	return temperature;
}
#endif

int imx_thermal_get_temp(struct udevice *dev, int *temp)
{
	struct thermal_data *priv = dev_get_priv(dev);
	int cpu_tmp = 0;

	cpu_tmp = read_cpu_temperature(dev);

	while (cpu_tmp >= priv->critical) {
		printf("CPU Temperature (%dC) too close to max (%dC)",
		       cpu_tmp, priv->maxc);
		puts(" waiting...\n");
		udelay(5000000);
		cpu_tmp = read_cpu_temperature(dev);
	}

	*temp = cpu_tmp;

	return 0;
}

static const struct dm_thermal_ops imx_thermal_ops = {
	.get_temp	= imx_thermal_get_temp,
};

static int imx_thermal_probe(struct udevice *dev)
{
	unsigned int fuse = ~0;

	const struct imx_thermal_plat *pdata = dev_get_platdata(dev);
	struct thermal_data *priv = dev_get_priv(dev);

	/* Read Temperature calibration data fuse */
	fuse_read(pdata->fuse_bank, pdata->fuse_word, &fuse);

	if (is_soc_type(MXC_SOC_MX6)) {
		/* Check for valid fuse */
		if (fuse == 0 || fuse == ~0) {
			debug("CPU:   Thermal invalid data, fuse: 0x%x\n",
				fuse);
			return -EPERM;
		}
	} else if (is_soc_type(MXC_SOC_MX7)) {
		/* No Calibration data in FUSE? */
		if ((fuse & 0x3ffff) == 0)
			return -EPERM;
		/* We do not support 105C TE2 */
		if (((fuse & 0x1c0000) >> 18) == 0x6)
			return -EPERM;
	}

	/* set critical cooling temp */
	get_cpu_temp_grade(&priv->minc, &priv->maxc);
	priv->critical = priv->maxc - TEMPERATURE_HOT_DELTA;
	priv->fuse = fuse;

	enable_thermal_clk();

	return 0;
}

U_BOOT_DRIVER(imx_thermal) = {
	.name	= "imx_thermal",
	.id	= UCLASS_THERMAL,
	.ops	= &imx_thermal_ops,
	.probe	= imx_thermal_probe,
	.priv_auto_alloc_size = sizeof(struct thermal_data),
	.flags  = DM_FLAG_PRE_RELOC,
};
