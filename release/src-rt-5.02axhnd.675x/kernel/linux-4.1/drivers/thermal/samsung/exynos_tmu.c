/*
 * exynos_tmu.c - Samsung EXYNOS TMU (Thermal Management Unit)
 *
 *  Copyright (C) 2014 Samsung Electronics
 *  Bartlomiej Zolnierkiewicz <b.zolnierkie@samsung.com>
 *  Lukasz Majewski <l.majewski@samsung.com>
 *
 *  Copyright (C) 2011 Samsung Electronics
 *  Donggeun Kim <dg77.kim@samsung.com>
 *  Amit Daniel Kachhap <amit.kachhap@linaro.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <linux/clk.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>

#include "exynos_tmu.h"
#include "../thermal_core.h"

/* Exynos generic registers */
#define EXYNOS_TMU_REG_TRIMINFO		0x0
#define EXYNOS_TMU_REG_CONTROL		0x20
#define EXYNOS_TMU_REG_STATUS		0x28
#define EXYNOS_TMU_REG_CURRENT_TEMP	0x40
#define EXYNOS_TMU_REG_INTEN		0x70
#define EXYNOS_TMU_REG_INTSTAT		0x74
#define EXYNOS_TMU_REG_INTCLEAR		0x78

#define EXYNOS_TMU_TEMP_MASK		0xff
#define EXYNOS_TMU_REF_VOLTAGE_SHIFT	24
#define EXYNOS_TMU_REF_VOLTAGE_MASK	0x1f
#define EXYNOS_TMU_BUF_SLOPE_SEL_MASK	0xf
#define EXYNOS_TMU_BUF_SLOPE_SEL_SHIFT	8
#define EXYNOS_TMU_CORE_EN_SHIFT	0

/* Exynos3250 specific registers */
#define EXYNOS_TMU_TRIMINFO_CON1	0x10

/* Exynos4210 specific registers */
#define EXYNOS4210_TMU_REG_THRESHOLD_TEMP	0x44
#define EXYNOS4210_TMU_REG_TRIG_LEVEL0	0x50

/* Exynos5250, Exynos4412, Exynos3250 specific registers */
#define EXYNOS_TMU_TRIMINFO_CON2	0x14
#define EXYNOS_THD_TEMP_RISE		0x50
#define EXYNOS_THD_TEMP_FALL		0x54
#define EXYNOS_EMUL_CON		0x80

#define EXYNOS_TRIMINFO_RELOAD_ENABLE	1
#define EXYNOS_TRIMINFO_25_SHIFT	0
#define EXYNOS_TRIMINFO_85_SHIFT	8
#define EXYNOS_TMU_TRIP_MODE_SHIFT	13
#define EXYNOS_TMU_TRIP_MODE_MASK	0x7
#define EXYNOS_TMU_THERM_TRIP_EN_SHIFT	12

#define EXYNOS_TMU_INTEN_RISE0_SHIFT	0
#define EXYNOS_TMU_INTEN_RISE1_SHIFT	4
#define EXYNOS_TMU_INTEN_RISE2_SHIFT	8
#define EXYNOS_TMU_INTEN_RISE3_SHIFT	12
#define EXYNOS_TMU_INTEN_FALL0_SHIFT	16

#define EXYNOS_EMUL_TIME	0x57F0
#define EXYNOS_EMUL_TIME_MASK	0xffff
#define EXYNOS_EMUL_TIME_SHIFT	16
#define EXYNOS_EMUL_DATA_SHIFT	8
#define EXYNOS_EMUL_DATA_MASK	0xFF
#define EXYNOS_EMUL_ENABLE	0x1

/* Exynos5260 specific */
#define EXYNOS5260_TMU_REG_INTEN		0xC0
#define EXYNOS5260_TMU_REG_INTSTAT		0xC4
#define EXYNOS5260_TMU_REG_INTCLEAR		0xC8
#define EXYNOS5260_EMUL_CON			0x100

/* Exynos4412 specific */
#define EXYNOS4412_MUX_ADDR_VALUE          6
#define EXYNOS4412_MUX_ADDR_SHIFT          20

/*exynos5440 specific registers*/
#define EXYNOS5440_TMU_S0_7_TRIM		0x000
#define EXYNOS5440_TMU_S0_7_CTRL		0x020
#define EXYNOS5440_TMU_S0_7_DEBUG		0x040
#define EXYNOS5440_TMU_S0_7_TEMP		0x0f0
#define EXYNOS5440_TMU_S0_7_TH0			0x110
#define EXYNOS5440_TMU_S0_7_TH1			0x130
#define EXYNOS5440_TMU_S0_7_TH2			0x150
#define EXYNOS5440_TMU_S0_7_IRQEN		0x210
#define EXYNOS5440_TMU_S0_7_IRQ			0x230
/* exynos5440 common registers */
#define EXYNOS5440_TMU_IRQ_STATUS		0x000
#define EXYNOS5440_TMU_PMIN			0x004

#define EXYNOS5440_TMU_INTEN_RISE0_SHIFT	0
#define EXYNOS5440_TMU_INTEN_RISE1_SHIFT	1
#define EXYNOS5440_TMU_INTEN_RISE2_SHIFT	2
#define EXYNOS5440_TMU_INTEN_RISE3_SHIFT	3
#define EXYNOS5440_TMU_INTEN_FALL0_SHIFT	4
#define EXYNOS5440_TMU_TH_RISE4_SHIFT		24
#define EXYNOS5440_EFUSE_SWAP_OFFSET		8

/* Exynos7 specific registers */
#define EXYNOS7_THD_TEMP_RISE7_6		0x50
#define EXYNOS7_THD_TEMP_FALL7_6		0x60
#define EXYNOS7_TMU_REG_INTEN			0x110
#define EXYNOS7_TMU_REG_INTPEND			0x118
#define EXYNOS7_TMU_REG_EMUL_CON		0x160

#define EXYNOS7_TMU_TEMP_MASK			0x1ff
#define EXYNOS7_PD_DET_EN_SHIFT			23
#define EXYNOS7_TMU_INTEN_RISE0_SHIFT		0
#define EXYNOS7_TMU_INTEN_RISE1_SHIFT		1
#define EXYNOS7_TMU_INTEN_RISE2_SHIFT		2
#define EXYNOS7_TMU_INTEN_RISE3_SHIFT		3
#define EXYNOS7_TMU_INTEN_RISE4_SHIFT		4
#define EXYNOS7_TMU_INTEN_RISE5_SHIFT		5
#define EXYNOS7_TMU_INTEN_RISE6_SHIFT		6
#define EXYNOS7_TMU_INTEN_RISE7_SHIFT		7
#define EXYNOS7_EMUL_DATA_SHIFT			7
#define EXYNOS7_EMUL_DATA_MASK			0x1ff

#define MCELSIUS	1000
/**
 * struct exynos_tmu_data : A structure to hold the private data of the TMU
	driver
 * @id: identifier of the one instance of the TMU controller.
 * @pdata: pointer to the tmu platform/configuration data
 * @base: base address of the single instance of the TMU controller.
 * @base_second: base address of the common registers of the TMU controller.
 * @irq: irq number of the TMU controller.
 * @soc: id of the SOC type.
 * @irq_work: pointer to the irq work structure.
 * @lock: lock to implement synchronization.
 * @clk: pointer to the clock structure.
 * @clk_sec: pointer to the clock structure for accessing the base_second.
 * @sclk: pointer to the clock structure for accessing the tmu special clk.
 * @temp_error1: fused value of the first point trim.
 * @temp_error2: fused value of the second point trim.
 * @regulator: pointer to the TMU regulator structure.
 * @reg_conf: pointer to structure to register with core thermal.
 * @tmu_initialize: SoC specific TMU initialization method
 * @tmu_control: SoC specific TMU control method
 * @tmu_read: SoC specific TMU temperature read method
 * @tmu_set_emulation: SoC specific TMU emulation setting method
 * @tmu_clear_irqs: SoC specific TMU interrupts clearing method
 */
struct exynos_tmu_data {
	int id;
	struct exynos_tmu_platform_data *pdata;
	void __iomem *base;
	void __iomem *base_second;
	int irq;
	enum soc_type soc;
	struct work_struct irq_work;
	struct mutex lock;
	struct clk *clk, *clk_sec, *sclk;
	u16 temp_error1, temp_error2;
	struct regulator *regulator;
	struct thermal_zone_device *tzd;

	int (*tmu_initialize)(struct platform_device *pdev);
	void (*tmu_control)(struct platform_device *pdev, bool on);
	int (*tmu_read)(struct exynos_tmu_data *data);
	void (*tmu_set_emulation)(struct exynos_tmu_data *data,
				  unsigned long temp);
	void (*tmu_clear_irqs)(struct exynos_tmu_data *data);
};

static void exynos_report_trigger(struct exynos_tmu_data *p)
{
	char data[10], *envp[] = { data, NULL };
	struct thermal_zone_device *tz = p->tzd;
	unsigned long temp;
	unsigned int i;

	if (!tz) {
		pr_err("No thermal zone device defined\n");
		return;
	}

	thermal_zone_device_update(tz);

	mutex_lock(&tz->lock);
	/* Find the level for which trip happened */
	for (i = 0; i < of_thermal_get_ntrips(tz); i++) {
		tz->ops->get_trip_temp(tz, i, &temp);
		if (tz->last_temperature < temp)
			break;
	}

	snprintf(data, sizeof(data), "%u", i);
	kobject_uevent_env(&tz->device.kobj, KOBJ_CHANGE, envp);
	mutex_unlock(&tz->lock);
}

/*
 * TMU treats temperature as a mapped temperature code.
 * The temperature is converted differently depending on the calibration type.
 */
static int temp_to_code(struct exynos_tmu_data *data, u8 temp)
{
	struct exynos_tmu_platform_data *pdata = data->pdata;
	int temp_code;

	switch (pdata->cal_type) {
	case TYPE_TWO_POINT_TRIMMING:
		temp_code = (temp - pdata->first_point_trim) *
			(data->temp_error2 - data->temp_error1) /
			(pdata->second_point_trim - pdata->first_point_trim) +
			data->temp_error1;
		break;
	case TYPE_ONE_POINT_TRIMMING:
		temp_code = temp + data->temp_error1 - pdata->first_point_trim;
		break;
	default:
		temp_code = temp + pdata->default_temp_offset;
		break;
	}

	return temp_code;
}

/*
 * Calculate a temperature value from a temperature code.
 * The unit of the temperature is degree Celsius.
 */
static int code_to_temp(struct exynos_tmu_data *data, u16 temp_code)
{
	struct exynos_tmu_platform_data *pdata = data->pdata;
	int temp;

	switch (pdata->cal_type) {
	case TYPE_TWO_POINT_TRIMMING:
		temp = (temp_code - data->temp_error1) *
			(pdata->second_point_trim - pdata->first_point_trim) /
			(data->temp_error2 - data->temp_error1) +
			pdata->first_point_trim;
		break;
	case TYPE_ONE_POINT_TRIMMING:
		temp = temp_code - data->temp_error1 + pdata->first_point_trim;
		break;
	default:
		temp = temp_code - pdata->default_temp_offset;
		break;
	}

	return temp;
}

static void sanitize_temp_error(struct exynos_tmu_data *data, u32 trim_info)
{
	struct exynos_tmu_platform_data *pdata = data->pdata;

	data->temp_error1 = trim_info & EXYNOS_TMU_TEMP_MASK;
	data->temp_error2 = ((trim_info >> EXYNOS_TRIMINFO_85_SHIFT) &
				EXYNOS_TMU_TEMP_MASK);

	if (!data->temp_error1 ||
		(pdata->min_efuse_value > data->temp_error1) ||
		(data->temp_error1 > pdata->max_efuse_value))
		data->temp_error1 = pdata->efuse_value & EXYNOS_TMU_TEMP_MASK;

	if (!data->temp_error2)
		data->temp_error2 =
			(pdata->efuse_value >> EXYNOS_TRIMINFO_85_SHIFT) &
			EXYNOS_TMU_TEMP_MASK;
}

static u32 get_th_reg(struct exynos_tmu_data *data, u32 threshold, bool falling)
{
	struct thermal_zone_device *tz = data->tzd;
	const struct thermal_trip * const trips =
		of_thermal_get_trip_points(tz);
	unsigned long temp;
	int i;

	if (!trips) {
		pr_err("%s: Cannot get trip points from of-thermal.c!\n",
		       __func__);
		return 0;
	}

	for (i = 0; i < of_thermal_get_ntrips(tz); i++) {
		if (trips[i].type == THERMAL_TRIP_CRITICAL)
			continue;

		temp = trips[i].temperature / MCELSIUS;
		if (falling)
			temp -= (trips[i].hysteresis / MCELSIUS);
		else
			threshold &= ~(0xff << 8 * i);

		threshold |= temp_to_code(data, temp) << 8 * i;
	}

	return threshold;
}

static int exynos_tmu_initialize(struct platform_device *pdev)
{
	struct exynos_tmu_data *data = platform_get_drvdata(pdev);
	int ret;

	mutex_lock(&data->lock);
	clk_enable(data->clk);
	if (!IS_ERR(data->clk_sec))
		clk_enable(data->clk_sec);
	ret = data->tmu_initialize(pdev);
	clk_disable(data->clk);
	mutex_unlock(&data->lock);
	if (!IS_ERR(data->clk_sec))
		clk_disable(data->clk_sec);

	return ret;
}

static u32 get_con_reg(struct exynos_tmu_data *data, u32 con)
{
	struct exynos_tmu_platform_data *pdata = data->pdata;

	if (data->soc == SOC_ARCH_EXYNOS4412 ||
	    data->soc == SOC_ARCH_EXYNOS3250)
		con |= (EXYNOS4412_MUX_ADDR_VALUE << EXYNOS4412_MUX_ADDR_SHIFT);

	con &= ~(EXYNOS_TMU_REF_VOLTAGE_MASK << EXYNOS_TMU_REF_VOLTAGE_SHIFT);
	con |= pdata->reference_voltage << EXYNOS_TMU_REF_VOLTAGE_SHIFT;

	con &= ~(EXYNOS_TMU_BUF_SLOPE_SEL_MASK << EXYNOS_TMU_BUF_SLOPE_SEL_SHIFT);
	con |= (pdata->gain << EXYNOS_TMU_BUF_SLOPE_SEL_SHIFT);

	if (pdata->noise_cancel_mode) {
		con &= ~(EXYNOS_TMU_TRIP_MODE_MASK << EXYNOS_TMU_TRIP_MODE_SHIFT);
		con |= (pdata->noise_cancel_mode << EXYNOS_TMU_TRIP_MODE_SHIFT);
	}

	return con;
}

static void exynos_tmu_control(struct platform_device *pdev, bool on)
{
	struct exynos_tmu_data *data = platform_get_drvdata(pdev);

	mutex_lock(&data->lock);
	clk_enable(data->clk);
	data->tmu_control(pdev, on);
	clk_disable(data->clk);
	mutex_unlock(&data->lock);
}

static int exynos4210_tmu_initialize(struct platform_device *pdev)
{
	struct exynos_tmu_data *data = platform_get_drvdata(pdev);
	struct thermal_zone_device *tz = data->tzd;
	const struct thermal_trip * const trips =
		of_thermal_get_trip_points(tz);
	int ret = 0, threshold_code, i;
	unsigned long reference, temp;
	unsigned int status;

	if (!trips) {
		pr_err("%s: Cannot get trip points from of-thermal.c!\n",
		       __func__);
		ret = -ENODEV;
		goto out;
	}

	status = readb(data->base + EXYNOS_TMU_REG_STATUS);
	if (!status) {
		ret = -EBUSY;
		goto out;
	}

	sanitize_temp_error(data, readl(data->base + EXYNOS_TMU_REG_TRIMINFO));

	/* Write temperature code for threshold */
	reference = trips[0].temperature / MCELSIUS;
	threshold_code = temp_to_code(data, reference);
	if (threshold_code < 0) {
		ret = threshold_code;
		goto out;
	}
	writeb(threshold_code, data->base + EXYNOS4210_TMU_REG_THRESHOLD_TEMP);

	for (i = 0; i < of_thermal_get_ntrips(tz); i++) {
		temp = trips[i].temperature / MCELSIUS;
		writeb(temp - reference, data->base +
		       EXYNOS4210_TMU_REG_TRIG_LEVEL0 + i * 4);
	}

	data->tmu_clear_irqs(data);
out:
	return ret;
}

static int exynos4412_tmu_initialize(struct platform_device *pdev)
{
	struct exynos_tmu_data *data = platform_get_drvdata(pdev);
	const struct thermal_trip * const trips =
		of_thermal_get_trip_points(data->tzd);
	unsigned int status, trim_info, con, ctrl, rising_threshold;
	int ret = 0, threshold_code, i;
	unsigned long crit_temp = 0;

	status = readb(data->base + EXYNOS_TMU_REG_STATUS);
	if (!status) {
		ret = -EBUSY;
		goto out;
	}

	if (data->soc == SOC_ARCH_EXYNOS3250 ||
	    data->soc == SOC_ARCH_EXYNOS4412 ||
	    data->soc == SOC_ARCH_EXYNOS5250) {
		if (data->soc == SOC_ARCH_EXYNOS3250) {
			ctrl = readl(data->base + EXYNOS_TMU_TRIMINFO_CON1);
			ctrl |= EXYNOS_TRIMINFO_RELOAD_ENABLE;
			writel(ctrl, data->base + EXYNOS_TMU_TRIMINFO_CON1);
		}
		ctrl = readl(data->base + EXYNOS_TMU_TRIMINFO_CON2);
		ctrl |= EXYNOS_TRIMINFO_RELOAD_ENABLE;
		writel(ctrl, data->base + EXYNOS_TMU_TRIMINFO_CON2);
	}

	/* On exynos5420 the triminfo register is in the shared space */
	if (data->soc == SOC_ARCH_EXYNOS5420_TRIMINFO)
		trim_info = readl(data->base_second + EXYNOS_TMU_REG_TRIMINFO);
	else
		trim_info = readl(data->base + EXYNOS_TMU_REG_TRIMINFO);

	sanitize_temp_error(data, trim_info);

	/* Write temperature code for rising and falling threshold */
	rising_threshold = readl(data->base + EXYNOS_THD_TEMP_RISE);
	rising_threshold = get_th_reg(data, rising_threshold, false);
	writel(rising_threshold, data->base + EXYNOS_THD_TEMP_RISE);
	writel(get_th_reg(data, 0, true), data->base + EXYNOS_THD_TEMP_FALL);

	data->tmu_clear_irqs(data);

	/* if last threshold limit is also present */
	for (i = 0; i < of_thermal_get_ntrips(data->tzd); i++) {
		if (trips[i].type == THERMAL_TRIP_CRITICAL) {
			crit_temp = trips[i].temperature;
			break;
		}
	}

	if (i == of_thermal_get_ntrips(data->tzd)) {
		pr_err("%s: No CRITICAL trip point defined at of-thermal.c!\n",
		       __func__);
		ret = -EINVAL;
		goto out;
	}

	threshold_code = temp_to_code(data, crit_temp / MCELSIUS);
	/* 1-4 level to be assigned in th0 reg */
	rising_threshold &= ~(0xff << 8 * i);
	rising_threshold |= threshold_code << 8 * i;
	writel(rising_threshold, data->base + EXYNOS_THD_TEMP_RISE);
	con = readl(data->base + EXYNOS_TMU_REG_CONTROL);
	con |= (1 << EXYNOS_TMU_THERM_TRIP_EN_SHIFT);
	writel(con, data->base + EXYNOS_TMU_REG_CONTROL);

out:
	return ret;
}

static int exynos5440_tmu_initialize(struct platform_device *pdev)
{
	struct exynos_tmu_data *data = platform_get_drvdata(pdev);
	unsigned int trim_info = 0, con, rising_threshold;
	int ret = 0, threshold_code;
	unsigned long crit_temp = 0;

	/*
	 * For exynos5440 soc triminfo value is swapped between TMU0 and
	 * TMU2, so the below logic is needed.
	 */
	switch (data->id) {
	case 0:
		trim_info = readl(data->base + EXYNOS5440_EFUSE_SWAP_OFFSET +
				 EXYNOS5440_TMU_S0_7_TRIM);
		break;
	case 1:
		trim_info = readl(data->base + EXYNOS5440_TMU_S0_7_TRIM);
		break;
	case 2:
		trim_info = readl(data->base - EXYNOS5440_EFUSE_SWAP_OFFSET +
				  EXYNOS5440_TMU_S0_7_TRIM);
	}
	sanitize_temp_error(data, trim_info);

	/* Write temperature code for rising and falling threshold */
	rising_threshold = readl(data->base + EXYNOS5440_TMU_S0_7_TH0);
	rising_threshold = get_th_reg(data, rising_threshold, false);
	writel(rising_threshold, data->base + EXYNOS5440_TMU_S0_7_TH0);
	writel(0, data->base + EXYNOS5440_TMU_S0_7_TH1);

	data->tmu_clear_irqs(data);

	/* if last threshold limit is also present */
	if (!data->tzd->ops->get_crit_temp(data->tzd, &crit_temp)) {
		threshold_code = temp_to_code(data, crit_temp / MCELSIUS);
		/* 5th level to be assigned in th2 reg */
		rising_threshold =
			threshold_code << EXYNOS5440_TMU_TH_RISE4_SHIFT;
		writel(rising_threshold, data->base + EXYNOS5440_TMU_S0_7_TH2);
		con = readl(data->base + EXYNOS5440_TMU_S0_7_CTRL);
		con |= (1 << EXYNOS_TMU_THERM_TRIP_EN_SHIFT);
		writel(con, data->base + EXYNOS5440_TMU_S0_7_CTRL);
	}
	/* Clear the PMIN in the common TMU register */
	if (!data->id)
		writel(0, data->base_second + EXYNOS5440_TMU_PMIN);
	return ret;
}

static int exynos7_tmu_initialize(struct platform_device *pdev)
{
	struct exynos_tmu_data *data = platform_get_drvdata(pdev);
	struct thermal_zone_device *tz = data->tzd;
	struct exynos_tmu_platform_data *pdata = data->pdata;
	unsigned int status, trim_info;
	unsigned int rising_threshold = 0, falling_threshold = 0;
	int ret = 0, threshold_code, i;
	unsigned long temp, temp_hist;
	unsigned int reg_off, bit_off;

	status = readb(data->base + EXYNOS_TMU_REG_STATUS);
	if (!status) {
		ret = -EBUSY;
		goto out;
	}

	trim_info = readl(data->base + EXYNOS_TMU_REG_TRIMINFO);

	data->temp_error1 = trim_info & EXYNOS7_TMU_TEMP_MASK;
	if (!data->temp_error1 ||
	    (pdata->min_efuse_value > data->temp_error1) ||
	    (data->temp_error1 > pdata->max_efuse_value))
		data->temp_error1 = pdata->efuse_value & EXYNOS_TMU_TEMP_MASK;

	/* Write temperature code for rising and falling threshold */
	for (i = (of_thermal_get_ntrips(tz) - 1); i >= 0; i--) {
		/*
		 * On exynos7 there are 4 rising and 4 falling threshold
		 * registers (0x50-0x5c and 0x60-0x6c respectively). Each
		 * register holds the value of two threshold levels (at bit
		 * offsets 0 and 16). Based on the fact that there are atmost
		 * eight possible trigger levels, calculate the register and
		 * bit offsets where the threshold levels are to be written.
		 *
		 * e.g. EXYNOS7_THD_TEMP_RISE7_6 (0x50)
		 * [24:16] - Threshold level 7
		 * [8:0] - Threshold level 6
		 * e.g. EXYNOS7_THD_TEMP_RISE5_4 (0x54)
		 * [24:16] - Threshold level 5
		 * [8:0] - Threshold level 4
		 *
		 * and similarly for falling thresholds.
		 *
		 * Based on the above, calculate the register and bit offsets
		 * for rising/falling threshold levels and populate them.
		 */
		reg_off = ((7 - i) / 2) * 4;
		bit_off = ((8 - i) % 2);

		tz->ops->get_trip_temp(tz, i, &temp);
		temp /= MCELSIUS;

		tz->ops->get_trip_hyst(tz, i, &temp_hist);
		temp_hist = temp - (temp_hist / MCELSIUS);

		/* Set 9-bit temperature code for rising threshold levels */
		threshold_code = temp_to_code(data, temp);
		rising_threshold = readl(data->base +
			EXYNOS7_THD_TEMP_RISE7_6 + reg_off);
		rising_threshold &= ~(EXYNOS7_TMU_TEMP_MASK << (16 * bit_off));
		rising_threshold |= threshold_code << (16 * bit_off);
		writel(rising_threshold,
		       data->base + EXYNOS7_THD_TEMP_RISE7_6 + reg_off);

		/* Set 9-bit temperature code for falling threshold levels */
		threshold_code = temp_to_code(data, temp_hist);
		falling_threshold &= ~(EXYNOS7_TMU_TEMP_MASK << (16 * bit_off));
		falling_threshold |= threshold_code << (16 * bit_off);
		writel(falling_threshold,
		       data->base + EXYNOS7_THD_TEMP_FALL7_6 + reg_off);
	}

	data->tmu_clear_irqs(data);
out:
	return ret;
}

static void exynos4210_tmu_control(struct platform_device *pdev, bool on)
{
	struct exynos_tmu_data *data = platform_get_drvdata(pdev);
	struct thermal_zone_device *tz = data->tzd;
	unsigned int con, interrupt_en;

	con = get_con_reg(data, readl(data->base + EXYNOS_TMU_REG_CONTROL));

	if (on) {
		con |= (1 << EXYNOS_TMU_CORE_EN_SHIFT);
		interrupt_en =
			(of_thermal_is_trip_valid(tz, 3)
			 << EXYNOS_TMU_INTEN_RISE3_SHIFT) |
			(of_thermal_is_trip_valid(tz, 2)
			 << EXYNOS_TMU_INTEN_RISE2_SHIFT) |
			(of_thermal_is_trip_valid(tz, 1)
			 << EXYNOS_TMU_INTEN_RISE1_SHIFT) |
			(of_thermal_is_trip_valid(tz, 0)
			 << EXYNOS_TMU_INTEN_RISE0_SHIFT);

		if (data->soc != SOC_ARCH_EXYNOS4210)
			interrupt_en |=
				interrupt_en << EXYNOS_TMU_INTEN_FALL0_SHIFT;
	} else {
		con &= ~(1 << EXYNOS_TMU_CORE_EN_SHIFT);
		interrupt_en = 0; /* Disable all interrupts */
	}
	writel(interrupt_en, data->base + EXYNOS_TMU_REG_INTEN);
	writel(con, data->base + EXYNOS_TMU_REG_CONTROL);
}

static void exynos5440_tmu_control(struct platform_device *pdev, bool on)
{
	struct exynos_tmu_data *data = platform_get_drvdata(pdev);
	struct thermal_zone_device *tz = data->tzd;
	unsigned int con, interrupt_en;

	con = get_con_reg(data, readl(data->base + EXYNOS5440_TMU_S0_7_CTRL));

	if (on) {
		con |= (1 << EXYNOS_TMU_CORE_EN_SHIFT);
		interrupt_en =
			(of_thermal_is_trip_valid(tz, 3)
			 << EXYNOS5440_TMU_INTEN_RISE3_SHIFT) |
			(of_thermal_is_trip_valid(tz, 2)
			 << EXYNOS5440_TMU_INTEN_RISE2_SHIFT) |
			(of_thermal_is_trip_valid(tz, 1)
			 << EXYNOS5440_TMU_INTEN_RISE1_SHIFT) |
			(of_thermal_is_trip_valid(tz, 0)
			 << EXYNOS5440_TMU_INTEN_RISE0_SHIFT);
		interrupt_en |=
			interrupt_en << EXYNOS5440_TMU_INTEN_FALL0_SHIFT;
	} else {
		con &= ~(1 << EXYNOS_TMU_CORE_EN_SHIFT);
		interrupt_en = 0; /* Disable all interrupts */
	}
	writel(interrupt_en, data->base + EXYNOS5440_TMU_S0_7_IRQEN);
	writel(con, data->base + EXYNOS5440_TMU_S0_7_CTRL);
}

static void exynos7_tmu_control(struct platform_device *pdev, bool on)
{
	struct exynos_tmu_data *data = platform_get_drvdata(pdev);
	struct thermal_zone_device *tz = data->tzd;
	unsigned int con, interrupt_en;

	con = get_con_reg(data, readl(data->base + EXYNOS_TMU_REG_CONTROL));

	if (on) {
		con |= (1 << EXYNOS_TMU_CORE_EN_SHIFT);
		con |= (1 << EXYNOS7_PD_DET_EN_SHIFT);
		interrupt_en =
			(of_thermal_is_trip_valid(tz, 7)
			<< EXYNOS7_TMU_INTEN_RISE7_SHIFT) |
			(of_thermal_is_trip_valid(tz, 6)
			<< EXYNOS7_TMU_INTEN_RISE6_SHIFT) |
			(of_thermal_is_trip_valid(tz, 5)
			<< EXYNOS7_TMU_INTEN_RISE5_SHIFT) |
			(of_thermal_is_trip_valid(tz, 4)
			<< EXYNOS7_TMU_INTEN_RISE4_SHIFT) |
			(of_thermal_is_trip_valid(tz, 3)
			<< EXYNOS7_TMU_INTEN_RISE3_SHIFT) |
			(of_thermal_is_trip_valid(tz, 2)
			<< EXYNOS7_TMU_INTEN_RISE2_SHIFT) |
			(of_thermal_is_trip_valid(tz, 1)
			<< EXYNOS7_TMU_INTEN_RISE1_SHIFT) |
			(of_thermal_is_trip_valid(tz, 0)
			<< EXYNOS7_TMU_INTEN_RISE0_SHIFT);

		interrupt_en |=
			interrupt_en << EXYNOS_TMU_INTEN_FALL0_SHIFT;
	} else {
		con &= ~(1 << EXYNOS_TMU_CORE_EN_SHIFT);
		con &= ~(1 << EXYNOS7_PD_DET_EN_SHIFT);
		interrupt_en = 0; /* Disable all interrupts */
	}

	writel(interrupt_en, data->base + EXYNOS7_TMU_REG_INTEN);
	writel(con, data->base + EXYNOS_TMU_REG_CONTROL);
}

static int exynos_get_temp(void *p, long *temp)
{
	struct exynos_tmu_data *data = p;

	if (!data || !data->tmu_read)
		return -EINVAL;

	mutex_lock(&data->lock);
	clk_enable(data->clk);

	*temp = code_to_temp(data, data->tmu_read(data)) * MCELSIUS;

	clk_disable(data->clk);
	mutex_unlock(&data->lock);

	return 0;
}

#ifdef CONFIG_THERMAL_EMULATION
static u32 get_emul_con_reg(struct exynos_tmu_data *data, unsigned int val,
			    unsigned long temp)
{
	if (temp) {
		temp /= MCELSIUS;

		if (data->soc != SOC_ARCH_EXYNOS5440) {
			val &= ~(EXYNOS_EMUL_TIME_MASK << EXYNOS_EMUL_TIME_SHIFT);
			val |= (EXYNOS_EMUL_TIME << EXYNOS_EMUL_TIME_SHIFT);
		}
		if (data->soc == SOC_ARCH_EXYNOS7) {
			val &= ~(EXYNOS7_EMUL_DATA_MASK <<
				EXYNOS7_EMUL_DATA_SHIFT);
			val |= (temp_to_code(data, temp) <<
				EXYNOS7_EMUL_DATA_SHIFT) |
				EXYNOS_EMUL_ENABLE;
		} else {
			val &= ~(EXYNOS_EMUL_DATA_MASK <<
				EXYNOS_EMUL_DATA_SHIFT);
			val |= (temp_to_code(data, temp) <<
				EXYNOS_EMUL_DATA_SHIFT) |
				EXYNOS_EMUL_ENABLE;
		}
	} else {
		val &= ~EXYNOS_EMUL_ENABLE;
	}

	return val;
}

static void exynos4412_tmu_set_emulation(struct exynos_tmu_data *data,
					 unsigned long temp)
{
	unsigned int val;
	u32 emul_con;

	if (data->soc == SOC_ARCH_EXYNOS5260)
		emul_con = EXYNOS5260_EMUL_CON;
	else if (data->soc == SOC_ARCH_EXYNOS7)
		emul_con = EXYNOS7_TMU_REG_EMUL_CON;
	else
		emul_con = EXYNOS_EMUL_CON;

	val = readl(data->base + emul_con);
	val = get_emul_con_reg(data, val, temp);
	writel(val, data->base + emul_con);
}

static void exynos5440_tmu_set_emulation(struct exynos_tmu_data *data,
					 unsigned long temp)
{
	unsigned int val;

	val = readl(data->base + EXYNOS5440_TMU_S0_7_DEBUG);
	val = get_emul_con_reg(data, val, temp);
	writel(val, data->base + EXYNOS5440_TMU_S0_7_DEBUG);
}

static int exynos_tmu_set_emulation(void *drv_data, unsigned long temp)
{
	struct exynos_tmu_data *data = drv_data;
	int ret = -EINVAL;

	if (data->soc == SOC_ARCH_EXYNOS4210)
		goto out;

	if (temp && temp < MCELSIUS)
		goto out;

	mutex_lock(&data->lock);
	clk_enable(data->clk);
	data->tmu_set_emulation(data, temp);
	clk_disable(data->clk);
	mutex_unlock(&data->lock);
	return 0;
out:
	return ret;
}
#else
#define exynos4412_tmu_set_emulation NULL
#define exynos5440_tmu_set_emulation NULL
static int exynos_tmu_set_emulation(void *drv_data,	unsigned long temp)
	{ return -EINVAL; }
#endif /* CONFIG_THERMAL_EMULATION */

static int exynos4210_tmu_read(struct exynos_tmu_data *data)
{
	int ret = readb(data->base + EXYNOS_TMU_REG_CURRENT_TEMP);

	/* "temp_code" should range between 75 and 175 */
	return (ret < 75 || ret > 175) ? -ENODATA : ret;
}

static int exynos4412_tmu_read(struct exynos_tmu_data *data)
{
	return readb(data->base + EXYNOS_TMU_REG_CURRENT_TEMP);
}

static int exynos5440_tmu_read(struct exynos_tmu_data *data)
{
	return readb(data->base + EXYNOS5440_TMU_S0_7_TEMP);
}

static int exynos7_tmu_read(struct exynos_tmu_data *data)
{
	return readw(data->base + EXYNOS_TMU_REG_CURRENT_TEMP) &
		EXYNOS7_TMU_TEMP_MASK;
}

static void exynos_tmu_work(struct work_struct *work)
{
	struct exynos_tmu_data *data = container_of(work,
			struct exynos_tmu_data, irq_work);
	unsigned int val_type;

	if (!IS_ERR(data->clk_sec))
		clk_enable(data->clk_sec);
	/* Find which sensor generated this interrupt */
	if (data->soc == SOC_ARCH_EXYNOS5440) {
		val_type = readl(data->base_second + EXYNOS5440_TMU_IRQ_STATUS);
		if (!((val_type >> data->id) & 0x1))
			goto out;
	}
	if (!IS_ERR(data->clk_sec))
		clk_disable(data->clk_sec);

	exynos_report_trigger(data);
	mutex_lock(&data->lock);
	clk_enable(data->clk);

	/* TODO: take action based on particular interrupt */
	data->tmu_clear_irqs(data);

	clk_disable(data->clk);
	mutex_unlock(&data->lock);
out:
	enable_irq(data->irq);
}

static void exynos4210_tmu_clear_irqs(struct exynos_tmu_data *data)
{
	unsigned int val_irq;
	u32 tmu_intstat, tmu_intclear;

	if (data->soc == SOC_ARCH_EXYNOS5260) {
		tmu_intstat = EXYNOS5260_TMU_REG_INTSTAT;
		tmu_intclear = EXYNOS5260_TMU_REG_INTCLEAR;
	} else if (data->soc == SOC_ARCH_EXYNOS7) {
		tmu_intstat = EXYNOS7_TMU_REG_INTPEND;
		tmu_intclear = EXYNOS7_TMU_REG_INTPEND;
	} else {
		tmu_intstat = EXYNOS_TMU_REG_INTSTAT;
		tmu_intclear = EXYNOS_TMU_REG_INTCLEAR;
	}

	val_irq = readl(data->base + tmu_intstat);
	/*
	 * Clear the interrupts.  Please note that the documentation for
	 * Exynos3250, Exynos4412, Exynos5250 and Exynos5260 incorrectly
	 * states that INTCLEAR register has a different placing of bits
	 * responsible for FALL IRQs than INTSTAT register.  Exynos5420
	 * and Exynos5440 documentation is correct (Exynos4210 doesn't
	 * support FALL IRQs at all).
	 */
	writel(val_irq, data->base + tmu_intclear);
}

static void exynos5440_tmu_clear_irqs(struct exynos_tmu_data *data)
{
	unsigned int val_irq;

	val_irq = readl(data->base + EXYNOS5440_TMU_S0_7_IRQ);
	/* clear the interrupts */
	writel(val_irq, data->base + EXYNOS5440_TMU_S0_7_IRQ);
}

static irqreturn_t exynos_tmu_irq(int irq, void *id)
{
	struct exynos_tmu_data *data = id;

	disable_irq_nosync(irq);
	schedule_work(&data->irq_work);

	return IRQ_HANDLED;
}

static const struct of_device_id exynos_tmu_match[] = {
	{ .compatible = "samsung,exynos3250-tmu", },
	{ .compatible = "samsung,exynos4210-tmu", },
	{ .compatible = "samsung,exynos4412-tmu", },
	{ .compatible = "samsung,exynos5250-tmu", },
	{ .compatible = "samsung,exynos5260-tmu", },
	{ .compatible = "samsung,exynos5420-tmu", },
	{ .compatible = "samsung,exynos5420-tmu-ext-triminfo", },
	{ .compatible = "samsung,exynos5440-tmu", },
	{ .compatible = "samsung,exynos7-tmu", },
	{ /* sentinel */ },
};
MODULE_DEVICE_TABLE(of, exynos_tmu_match);

static int exynos_of_get_soc_type(struct device_node *np)
{
	if (of_device_is_compatible(np, "samsung,exynos3250-tmu"))
		return SOC_ARCH_EXYNOS3250;
	else if (of_device_is_compatible(np, "samsung,exynos4210-tmu"))
		return SOC_ARCH_EXYNOS4210;
	else if (of_device_is_compatible(np, "samsung,exynos4412-tmu"))
		return SOC_ARCH_EXYNOS4412;
	else if (of_device_is_compatible(np, "samsung,exynos5250-tmu"))
		return SOC_ARCH_EXYNOS5250;
	else if (of_device_is_compatible(np, "samsung,exynos5260-tmu"))
		return SOC_ARCH_EXYNOS5260;
	else if (of_device_is_compatible(np, "samsung,exynos5420-tmu"))
		return SOC_ARCH_EXYNOS5420;
	else if (of_device_is_compatible(np,
					 "samsung,exynos5420-tmu-ext-triminfo"))
		return SOC_ARCH_EXYNOS5420_TRIMINFO;
	else if (of_device_is_compatible(np, "samsung,exynos5440-tmu"))
		return SOC_ARCH_EXYNOS5440;
	else if (of_device_is_compatible(np, "samsung,exynos7-tmu"))
		return SOC_ARCH_EXYNOS7;

	return -EINVAL;
}

static int exynos_of_sensor_conf(struct device_node *np,
				 struct exynos_tmu_platform_data *pdata)
{
	u32 value;
	int ret;

	of_node_get(np);

	ret = of_property_read_u32(np, "samsung,tmu_gain", &value);
	pdata->gain = (u8)value;
	of_property_read_u32(np, "samsung,tmu_reference_voltage", &value);
	pdata->reference_voltage = (u8)value;
	of_property_read_u32(np, "samsung,tmu_noise_cancel_mode", &value);
	pdata->noise_cancel_mode = (u8)value;

	of_property_read_u32(np, "samsung,tmu_efuse_value",
			     &pdata->efuse_value);
	of_property_read_u32(np, "samsung,tmu_min_efuse_value",
			     &pdata->min_efuse_value);
	of_property_read_u32(np, "samsung,tmu_max_efuse_value",
			     &pdata->max_efuse_value);

	of_property_read_u32(np, "samsung,tmu_first_point_trim", &value);
	pdata->first_point_trim = (u8)value;
	of_property_read_u32(np, "samsung,tmu_second_point_trim", &value);
	pdata->second_point_trim = (u8)value;
	of_property_read_u32(np, "samsung,tmu_default_temp_offset", &value);
	pdata->default_temp_offset = (u8)value;

	of_property_read_u32(np, "samsung,tmu_cal_type", &pdata->cal_type);
	of_property_read_u32(np, "samsung,tmu_cal_mode", &pdata->cal_mode);

	of_node_put(np);
	return 0;
}

static int exynos_map_dt_data(struct platform_device *pdev)
{
	struct exynos_tmu_data *data = platform_get_drvdata(pdev);
	struct exynos_tmu_platform_data *pdata;
	struct resource res;
	int ret;

	if (!data || !pdev->dev.of_node)
		return -ENODEV;

	/*
	 * Try enabling the regulator if found
	 * TODO: Add regulator as an SOC feature, so that regulator enable
	 * is a compulsory call.
	 */
	data->regulator = devm_regulator_get(&pdev->dev, "vtmu");
	if (!IS_ERR(data->regulator)) {
		ret = regulator_enable(data->regulator);
		if (ret) {
			dev_err(&pdev->dev, "failed to enable vtmu\n");
			return ret;
		}
	} else {
		dev_info(&pdev->dev, "Regulator node (vtmu) not found\n");
	}

	data->id = of_alias_get_id(pdev->dev.of_node, "tmuctrl");
	if (data->id < 0)
		data->id = 0;

	data->irq = irq_of_parse_and_map(pdev->dev.of_node, 0);
	if (data->irq <= 0) {
		dev_err(&pdev->dev, "failed to get IRQ\n");
		return -ENODEV;
	}

	if (of_address_to_resource(pdev->dev.of_node, 0, &res)) {
		dev_err(&pdev->dev, "failed to get Resource 0\n");
		return -ENODEV;
	}

	data->base = devm_ioremap(&pdev->dev, res.start, resource_size(&res));
	if (!data->base) {
		dev_err(&pdev->dev, "Failed to ioremap memory\n");
		return -EADDRNOTAVAIL;
	}

	pdata = devm_kzalloc(&pdev->dev,
			     sizeof(struct exynos_tmu_platform_data),
			     GFP_KERNEL);
	if (!pdata)
		return -ENOMEM;

	exynos_of_sensor_conf(pdev->dev.of_node, pdata);
	data->pdata = pdata;
	data->soc = exynos_of_get_soc_type(pdev->dev.of_node);

	switch (data->soc) {
	case SOC_ARCH_EXYNOS4210:
		data->tmu_initialize = exynos4210_tmu_initialize;
		data->tmu_control = exynos4210_tmu_control;
		data->tmu_read = exynos4210_tmu_read;
		data->tmu_clear_irqs = exynos4210_tmu_clear_irqs;
		break;
	case SOC_ARCH_EXYNOS3250:
	case SOC_ARCH_EXYNOS4412:
	case SOC_ARCH_EXYNOS5250:
	case SOC_ARCH_EXYNOS5260:
	case SOC_ARCH_EXYNOS5420:
	case SOC_ARCH_EXYNOS5420_TRIMINFO:
		data->tmu_initialize = exynos4412_tmu_initialize;
		data->tmu_control = exynos4210_tmu_control;
		data->tmu_read = exynos4412_tmu_read;
		data->tmu_set_emulation = exynos4412_tmu_set_emulation;
		data->tmu_clear_irqs = exynos4210_tmu_clear_irqs;
		break;
	case SOC_ARCH_EXYNOS5440:
		data->tmu_initialize = exynos5440_tmu_initialize;
		data->tmu_control = exynos5440_tmu_control;
		data->tmu_read = exynos5440_tmu_read;
		data->tmu_set_emulation = exynos5440_tmu_set_emulation;
		data->tmu_clear_irqs = exynos5440_tmu_clear_irqs;
		break;
	case SOC_ARCH_EXYNOS7:
		data->tmu_initialize = exynos7_tmu_initialize;
		data->tmu_control = exynos7_tmu_control;
		data->tmu_read = exynos7_tmu_read;
		data->tmu_set_emulation = exynos4412_tmu_set_emulation;
		data->tmu_clear_irqs = exynos4210_tmu_clear_irqs;
		break;
	default:
		dev_err(&pdev->dev, "Platform not supported\n");
		return -EINVAL;
	}

	/*
	 * Check if the TMU shares some registers and then try to map the
	 * memory of common registers.
	 */
	if (data->soc != SOC_ARCH_EXYNOS5420_TRIMINFO &&
	    data->soc != SOC_ARCH_EXYNOS5440)
		return 0;

	if (of_address_to_resource(pdev->dev.of_node, 1, &res)) {
		dev_err(&pdev->dev, "failed to get Resource 1\n");
		return -ENODEV;
	}

	data->base_second = devm_ioremap(&pdev->dev, res.start,
					resource_size(&res));
	if (!data->base_second) {
		dev_err(&pdev->dev, "Failed to ioremap memory\n");
		return -ENOMEM;
	}

	return 0;
}

static struct thermal_zone_of_device_ops exynos_sensor_ops = {
	.get_temp = exynos_get_temp,
	.set_emul_temp = exynos_tmu_set_emulation,
};

static int exynos_tmu_probe(struct platform_device *pdev)
{
	struct exynos_tmu_platform_data *pdata;
	struct exynos_tmu_data *data;
	int ret;

	data = devm_kzalloc(&pdev->dev, sizeof(struct exynos_tmu_data),
					GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	platform_set_drvdata(pdev, data);
	mutex_init(&data->lock);

	data->tzd = thermal_zone_of_sensor_register(&pdev->dev, 0, data,
						    &exynos_sensor_ops);
	if (IS_ERR(data->tzd)) {
		pr_err("thermal: tz: %p ERROR\n", data->tzd);
		return PTR_ERR(data->tzd);
	}
	ret = exynos_map_dt_data(pdev);
	if (ret)
		goto err_sensor;

	pdata = data->pdata;

	INIT_WORK(&data->irq_work, exynos_tmu_work);

	data->clk = devm_clk_get(&pdev->dev, "tmu_apbif");
	if (IS_ERR(data->clk)) {
		dev_err(&pdev->dev, "Failed to get clock\n");
		ret = PTR_ERR(data->clk);
		goto err_sensor;
	}

	data->clk_sec = devm_clk_get(&pdev->dev, "tmu_triminfo_apbif");
	if (IS_ERR(data->clk_sec)) {
		if (data->soc == SOC_ARCH_EXYNOS5420_TRIMINFO) {
			dev_err(&pdev->dev, "Failed to get triminfo clock\n");
			ret = PTR_ERR(data->clk_sec);
			goto err_sensor;
		}
	} else {
		ret = clk_prepare(data->clk_sec);
		if (ret) {
			dev_err(&pdev->dev, "Failed to get clock\n");
			goto err_sensor;
		}
	}

	ret = clk_prepare(data->clk);
	if (ret) {
		dev_err(&pdev->dev, "Failed to get clock\n");
		goto err_clk_sec;
	}

	if (data->soc == SOC_ARCH_EXYNOS7) {
		data->sclk = devm_clk_get(&pdev->dev, "tmu_sclk");
		if (IS_ERR(data->sclk)) {
			dev_err(&pdev->dev, "Failed to get sclk\n");
			goto err_clk;
		} else {
			ret = clk_prepare_enable(data->sclk);
			if (ret) {
				dev_err(&pdev->dev, "Failed to enable sclk\n");
				goto err_clk;
			}
		}
	}

	ret = exynos_tmu_initialize(pdev);
	if (ret) {
		dev_err(&pdev->dev, "Failed to initialize TMU\n");
		goto err_sclk;
	}

	ret = devm_request_irq(&pdev->dev, data->irq, exynos_tmu_irq,
		IRQF_TRIGGER_RISING | IRQF_SHARED, dev_name(&pdev->dev), data);
	if (ret) {
		dev_err(&pdev->dev, "Failed to request irq: %d\n", data->irq);
		goto err_sclk;
	}

	exynos_tmu_control(pdev, true);
	return 0;
err_sclk:
	clk_disable_unprepare(data->sclk);
err_clk:
	clk_unprepare(data->clk);
err_clk_sec:
	if (!IS_ERR(data->clk_sec))
		clk_unprepare(data->clk_sec);
err_sensor:
	if (!IS_ERR_OR_NULL(data->regulator))
		regulator_disable(data->regulator);
	thermal_zone_of_sensor_unregister(&pdev->dev, data->tzd);

	return ret;
}

static int exynos_tmu_remove(struct platform_device *pdev)
{
	struct exynos_tmu_data *data = platform_get_drvdata(pdev);
	struct thermal_zone_device *tzd = data->tzd;

	thermal_zone_of_sensor_unregister(&pdev->dev, tzd);
	exynos_tmu_control(pdev, false);

	clk_disable_unprepare(data->sclk);
	clk_unprepare(data->clk);
	if (!IS_ERR(data->clk_sec))
		clk_unprepare(data->clk_sec);

	if (!IS_ERR(data->regulator))
		regulator_disable(data->regulator);

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int exynos_tmu_suspend(struct device *dev)
{
	exynos_tmu_control(to_platform_device(dev), false);

	return 0;
}

static int exynos_tmu_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);

	exynos_tmu_initialize(pdev);
	exynos_tmu_control(pdev, true);

	return 0;
}

static SIMPLE_DEV_PM_OPS(exynos_tmu_pm,
			 exynos_tmu_suspend, exynos_tmu_resume);
#define EXYNOS_TMU_PM	(&exynos_tmu_pm)
#else
#define EXYNOS_TMU_PM	NULL
#endif

static struct platform_driver exynos_tmu_driver = {
	.driver = {
		.name   = "exynos-tmu",
		.pm     = EXYNOS_TMU_PM,
		.of_match_table = exynos_tmu_match,
	},
	.probe = exynos_tmu_probe,
	.remove	= exynos_tmu_remove,
};

module_platform_driver(exynos_tmu_driver);

MODULE_DESCRIPTION("EXYNOS TMU Driver");
MODULE_AUTHOR("Donggeun Kim <dg77.kim@samsung.com>");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:exynos-tmu");
