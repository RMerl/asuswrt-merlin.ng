/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (C) 2016-2019 Intel Corporation <www.intel.com>
 *
 */

#ifndef _CLOCK_MANAGER_SOC64_
#define _CLOCK_MANAGER_SOC64_

const unsigned int cm_get_osc_clk_hz(void);
const unsigned int cm_get_f2s_per_ref_clk_hz(void);
const unsigned int cm_get_f2s_sdr_ref_clk_hz(void);
const unsigned int cm_get_intosc_clk_hz(void);
const unsigned int cm_get_fpga_clk_hz(void);

#define CLKMGR_INTOSC_HZ	400000000

/* Clock configuration accessors */
const struct cm_config * const cm_get_default_config(void);

#endif /* _CLOCK_MANAGER_SOC64_ */
