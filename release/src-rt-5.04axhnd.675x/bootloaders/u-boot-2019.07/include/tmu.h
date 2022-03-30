/*
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *      http://www.samsung.com
 * Akshay Saraswat <akshay.s@samsung.com>
 *
 * Thermal Management Unit
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

#ifndef _TMU_H
#define _TMU_H

enum tmu_status_t {
	TMU_STATUS_INIT = -1,
	TMU_STATUS_NORMAL = 0,
	TMU_STATUS_WARNING,
	TMU_STATUS_TRIPPED,
};

/*
 * Monitors status of the TMU device and exynos temperature
 *
 * @param temp	pointer to the current temperature value
 * @return	enum tmu_status_t value, code indicating event to execute
 *		and -1 on error
 */
enum tmu_status_t tmu_monitor(int *temp);

/*
 * Initialize TMU device
 *
 * @param blob  FDT blob
 * @return	int value, 0 for success
 */
int tmu_init(const void *blob);
#endif	/* _THERMAL_H_ */
