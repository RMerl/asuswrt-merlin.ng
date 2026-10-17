/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (C)  2021 Texas Instruments Incorporated - https://www.ti.com/
 *
 */

#ifndef __TI_SCI_STATIC_DATA_H
#define __TI_SCI_STATIC_DATA_H

struct ti_sci_resource_static_data {
	u32 dev_id;
	u16 range_start;
	u16 range_num;
	u8 subtype;
};

#if IS_ENABLED(CONFIG_K3_DM_FW)

#if IS_ENABLED(CONFIG_SOC_K3_J721E)
static struct ti_sci_resource_static_data rm_static_data[] = {
	/* Free rings */
	{
		.dev_id = 235,
		.subtype = 1,
		.range_start = 124,
		.range_num = 32,
	},
	/* TX channels */
	{
		.dev_id = 236,
		.subtype = 13,
		.range_start = 6,
		.range_num = 2,
	},
	/* RX channels */
	{
		.dev_id = 236,
		.subtype = 10,
		.range_start = 6,
		.range_num = 2,
	},
	/* RX Free flows */
	{
		.dev_id = 236,
		.subtype = 0,
		.range_start = 60,
		.range_num = 8,
	},
	{ },
};
#endif /* CONFIG_SOC_K3_J721E */

#if IS_ENABLED(CONFIG_SOC_K3_J721S2)
static struct ti_sci_resource_static_data rm_static_data[] = {
	/* Free rings */
	{
		.dev_id = 272,
		.subtype = 1,
		.range_start = 180,
		.range_num = 32,
	},
	/* TX channels */
	{
		.dev_id = 273,
		.subtype = 13,
		.range_start = 12,
		.range_num = 2,
	},
	/* RX channels */
	{
		.dev_id = 273,
		.subtype = 10,
		.range_start = 12,
		.range_num = 2,
	},
	/* RX Free flows */
	{
		.dev_id = 273,
		.subtype = 0,
		.range_start = 80,
		.range_num = 8,
	},
	{ },
};
#endif /* CONFIG_SOC_K3_J721S2 */

#if IS_ENABLED(CONFIG_SOC_K3_AM625) || IS_ENABLED(CONFIG_SOC_K3_AM62A7)
static struct ti_sci_resource_static_data rm_static_data[] = {
	/* BC channels */
	{
		.dev_id = 26,
		.subtype = 32,
		.range_start = 18,
		.range_num = 2,
	},
	{ },
};
#endif /* CONFIG_SOC_K3_AM625 || CONFIG_SOC_K3_AM62A7 */

#else
static struct ti_sci_resource_static_data rm_static_data[] = {
	{ },
};
#endif /* CONFIG_K3_DM_FW */
#endif /* __TI_SCI_STATIC_DATA_H */
