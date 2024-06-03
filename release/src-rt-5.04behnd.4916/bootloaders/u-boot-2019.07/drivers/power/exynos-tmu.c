/*
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *      http://www.samsung.com
 * Akshay Saraswat <akshay.s@samsung.com>
 *
 * EXYNOS - Thermal Management Unit
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <errno.h>
#include <fdtdec.h>
#include <tmu.h>
#include <asm/arch/tmu.h>
#include <asm/arch/power.h>

#define TRIMINFO_RELOAD		1
#define CORE_EN			1
#define THERM_TRIP_EN		(1 << 12)

#define INTEN_RISE0		1
#define INTEN_RISE1		(1 << 4)
#define INTEN_RISE2		(1 << 8)
#define INTEN_FALL0		(1 << 16)
#define INTEN_FALL1		(1 << 20)
#define INTEN_FALL2		(1 << 24)

#define TRIM_INFO_MASK		0xff

#define INTCLEAR_RISE0		1
#define INTCLEAR_RISE1		(1 << 4)
#define INTCLEAR_RISE2		(1 << 8)
#define INTCLEAR_FALL0		(1 << 16)
#define INTCLEAR_FALL1		(1 << 20)
#define INTCLEAR_FALL2		(1 << 24)
#define INTCLEARALL		(INTCLEAR_RISE0 | INTCLEAR_RISE1 | \
				 INTCLEAR_RISE2 | INTCLEAR_FALL0 | \
				 INTCLEAR_FALL1 | INTCLEAR_FALL2)

/* Tmeperature threshold values for various thermal events */
struct temperature_params {
	/* minimum value in temperature code range */
	unsigned min_val;
	/* maximum value in temperature code range */
	unsigned max_val;
	/* temperature threshold to start warning */
	unsigned start_warning;
	/* temperature threshold CPU tripping */
	unsigned start_tripping;
	/* temperature threshold for HW tripping */
	unsigned hardware_tripping;
};

/* Pre-defined values and thresholds for calibration of current temperature */
struct tmu_data {
	/* pre-defined temperature thresholds */
	struct temperature_params ts;
	/* pre-defined efuse range minimum value */
	unsigned efuse_min_value;
	/* pre-defined efuse value for temperature calibration */
	unsigned efuse_value;
	/* pre-defined efuse range maximum value */
	unsigned efuse_max_value;
	/* current temperature sensing slope */
	unsigned slope;
};

/* TMU device specific details and status */
struct tmu_info {
	/* base Address for the TMU */
	struct exynos5_tmu_reg *tmu_base;
	/* mux Address for the TMU */
	int tmu_mux;
	/* pre-defined values for calibration and thresholds */
	struct tmu_data data;
	/* value required for triminfo_25 calibration */
	unsigned te1;
	/* value required for triminfo_85 calibration */
	unsigned te2;
	/* Value for measured data calibration */
	int dc_value;
	/* enum value indicating status of the TMU */
	int tmu_state;
};

/* Global struct tmu_info variable to store init values */
static struct tmu_info gbl_info;

/*
 * Get current temperature code from register,
 * then calculate and calibrate it's value
 * in degree celsius.
 *
 * @return	current temperature of the chip as sensed by TMU
 */
static int get_cur_temp(struct tmu_info *info)
{
	struct exynos5_tmu_reg *reg = info->tmu_base;
	ulong start;
	int cur_temp = 0;

	/*
	 * Temperature code range between min 25 and max 125.
	 * May run more than once for first call as initial sensing
	 * has not yet happened.
	 */
	if (info->tmu_state == TMU_STATUS_NORMAL) {
		start = get_timer(0);
		do {
			cur_temp = readl(&reg->current_temp) & 0xff;
		} while ((cur_temp == 0) || (get_timer(start) > 100));
	}

	if (cur_temp == 0)
		return cur_temp;

	/* Calibrate current temperature */
	cur_temp = cur_temp - info->te1 + info->dc_value;

	return cur_temp;
}

/*
 * Monitors status of the TMU device and exynos temperature
 *
 * @param temp	pointer to the current temperature value
 * @return	enum tmu_status_t value, code indicating event to execute
 */
enum tmu_status_t tmu_monitor(int *temp)
{
	int cur_temp;
	struct tmu_data *data = &gbl_info.data;

	if (gbl_info.tmu_state == TMU_STATUS_INIT)
		return TMU_STATUS_INIT;

	/* Read current temperature of the SOC */
	cur_temp = get_cur_temp(&gbl_info);

	if (!cur_temp)
		goto out;

	*temp = cur_temp;

	/* Temperature code lies between min 25 and max 125 */
	if ((cur_temp >= data->ts.start_tripping) &&
	    (cur_temp <= data->ts.max_val))
		return TMU_STATUS_TRIPPED;

	if (cur_temp >= data->ts.start_warning)
		return TMU_STATUS_WARNING;

	if ((cur_temp < data->ts.start_warning) &&
	    (cur_temp >= data->ts.min_val))
		return TMU_STATUS_NORMAL;

 out:
	/* Temperature code does not lie between min 25 and max 125 */
	gbl_info.tmu_state = TMU_STATUS_INIT;
	debug("EXYNOS_TMU: Thermal reading failed\n");
	return TMU_STATUS_INIT;
}

/*
 * Get TMU specific pre-defined values from FDT
 *
 * @param info	pointer to the tmu_info struct
 * @param blob  FDT blob
 * @return	int value, 0 for success
 */
static int get_tmu_fdt_values(struct tmu_info *info, const void *blob)
{
#if CONFIG_IS_ENABLED(OF_CONTROL)
	fdt_addr_t addr;
	int node;
	int error = 0;

	/* Get the node from FDT for TMU */
	node = fdtdec_next_compatible(blob, 0,
				      COMPAT_SAMSUNG_EXYNOS_TMU);
	if (node < 0) {
		debug("EXYNOS_TMU: No node for tmu in device tree\n");
		return -ENODEV;
	}

	/*
	 * Get the pre-defined TMU specific values from FDT.
	 * All of these are expected to be correct otherwise
	 * miscalculation of register values in tmu_setup_parameters
	 * may result in misleading current temperature.
	 */
	addr = fdtdec_get_addr(blob, node, "reg");
	if (addr == FDT_ADDR_T_NONE) {
		debug("%s: Missing tmu-base\n", __func__);
		return -ENODEV;
	}
	info->tmu_base = (struct exynos5_tmu_reg *)addr;

	/* Optional field. */
	info->tmu_mux = fdtdec_get_int(blob,
				node, "samsung,mux", -1);
	/* Take default value as per the user manual b(110) */
	if (info->tmu_mux == -1)
		info->tmu_mux = 0x6;

	info->data.ts.min_val = fdtdec_get_int(blob,
				node, "samsung,min-temp", -1);
	error |= (info->data.ts.min_val == -1);
	info->data.ts.max_val = fdtdec_get_int(blob,
				node, "samsung,max-temp", -1);
	error |= (info->data.ts.max_val == -1);
	info->data.ts.start_warning = fdtdec_get_int(blob,
				node, "samsung,start-warning", -1);
	error |= (info->data.ts.start_warning == -1);
	info->data.ts.start_tripping = fdtdec_get_int(blob,
				node, "samsung,start-tripping", -1);
	error |= (info->data.ts.start_tripping == -1);
	info->data.ts.hardware_tripping = fdtdec_get_int(blob,
				node, "samsung,hw-tripping", -1);
	error |= (info->data.ts.hardware_tripping == -1);
	info->data.efuse_min_value = fdtdec_get_int(blob,
				node, "samsung,efuse-min-value", -1);
	error |= (info->data.efuse_min_value == -1);
	info->data.efuse_value = fdtdec_get_int(blob,
				node, "samsung,efuse-value", -1);
	error |= (info->data.efuse_value == -1);
	info->data.efuse_max_value = fdtdec_get_int(blob,
				node, "samsung,efuse-max-value", -1);
	error |= (info->data.efuse_max_value == -1);
	info->data.slope = fdtdec_get_int(blob,
				node, "samsung,slope", -1);
	error |= (info->data.slope == -1);
	info->dc_value = fdtdec_get_int(blob,
				node, "samsung,dc-value", -1);
	error |= (info->dc_value == -1);

	if (error) {
		debug("fail to get tmu node properties\n");
		return -EINVAL;
	}
#else
	/* Non DT support may never be added. Just in case  */
	return -ENODEV;
#endif

	return 0;
}

/*
 * Calibrate and calculate threshold values and
 * enable interrupt levels
 *
 * @param	info pointer to the tmu_info struct
 */
static void tmu_setup_parameters(struct tmu_info *info)
{
	unsigned te_code, con;
	unsigned warning_code, trip_code, hwtrip_code;
	unsigned cooling_temp;
	unsigned rising_value;
	struct tmu_data *data = &info->data;
	struct exynos5_tmu_reg *reg = info->tmu_base;

	/* Must reload for reading efuse value from triminfo register */
	writel(TRIMINFO_RELOAD, &reg->triminfo_control);

	/* Get the compensation parameter */
	te_code = readl(&reg->triminfo);
	info->te1 = te_code & TRIM_INFO_MASK;
	info->te2 = ((te_code >> 8) & TRIM_INFO_MASK);

	if ((data->efuse_min_value > info->te1) ||
			(info->te1 > data->efuse_max_value)
			||  (info->te2 != 0))
		info->te1 = data->efuse_value;

	/* Get RISING & FALLING Threshold value */
	warning_code = data->ts.start_warning
			+ info->te1 - info->dc_value;
	trip_code = data->ts.start_tripping
			+ info->te1 - info->dc_value;
	hwtrip_code = data->ts.hardware_tripping
			+ info->te1 - info->dc_value;

	cooling_temp = 0;

	rising_value = ((warning_code << 8) |
			(trip_code << 16) |
			(hwtrip_code << 24));

	/* Set interrupt level */
	writel(rising_value, &reg->threshold_temp_rise);
	writel(cooling_temp, &reg->threshold_temp_fall);

	/*
	 * Init TMU control tuning parameters
	 * [28:24] VREF - Voltage reference
	 * [15:13] THERM_TRIP_MODE - Tripping mode
	 * [12] THERM_TRIP_EN - Thermal tripping enable
	 * [11:8] BUF_SLOPE_SEL - Gain of amplifier
	 * [6] THERM_TRIP_BY_TQ_EN - Tripping by TQ pin
	 */
	writel(data->slope, &reg->tmu_control);

	writel(INTCLEARALL, &reg->intclear);

	/* TMU core enable */
	con = readl(&reg->tmu_control);
	con |= THERM_TRIP_EN | CORE_EN | (info->tmu_mux << 20);

	writel(con, &reg->tmu_control);

	/* Enable HW thermal trip */
	set_hw_thermal_trip();

	/* LEV1 LEV2 interrupt enable */
	writel(INTEN_RISE1 | INTEN_RISE2, &reg->inten);
}

/*
 * Initialize TMU device
 *
 * @param blob  FDT blob
 * @return	int value, 0 for success
 */
int tmu_init(const void *blob)
{
	gbl_info.tmu_state = TMU_STATUS_INIT;
	if (get_tmu_fdt_values(&gbl_info, blob) < 0)
		goto ret;

	tmu_setup_parameters(&gbl_info);
	gbl_info.tmu_state = TMU_STATUS_NORMAL;
ret:
	return gbl_info.tmu_state;
}
