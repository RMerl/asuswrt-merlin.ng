/* SPDX-License-Identifier:     GPL-2.0+ */
/*
 * Copyright 2018 NXP
 */

#ifndef SC_PM_API_H
#define SC_PM_API_H

/* Defines for sc_pm_power_mode_t */
#define SC_PM_PW_MODE_OFF	0U /* Power off */
#define SC_PM_PW_MODE_STBY	1U /* Power in standby */
#define SC_PM_PW_MODE_LP	2U /* Power in low-power */
#define SC_PM_PW_MODE_ON	3U /* Power on */

/* Defines for sc_pm_clk_t */
#define SC_PM_CLK_SLV_BUS	0U /* Slave bus clock */
#define SC_PM_CLK_MST_BUS	1U /* Master bus clock */
#define SC_PM_CLK_PER		2U /* Peripheral clock */
#define SC_PM_CLK_PHY		3U /* Phy clock */
#define SC_PM_CLK_MISC		4U /* Misc clock */
#define SC_PM_CLK_MISC0		0U /* Misc 0 clock */
#define SC_PM_CLK_MISC1		1U /* Misc 1 clock */
#define SC_PM_CLK_MISC2		2U /* Misc 2 clock */
#define SC_PM_CLK_MISC3		3U /* Misc 3 clock */
#define SC_PM_CLK_MISC4		4U /* Misc 4 clock */
#define SC_PM_CLK_CPU		2U /* CPU clock */
#define SC_PM_CLK_PLL		4U /* PLL */
#define SC_PM_CLK_BYPASS	4U /* Bypass clock */

/* Defines for sc_pm_clk_mode_t */
#define SC_PM_CLK_MODE_ROM_INIT		0U /* Clock is initialized by ROM. */
#define SC_PM_CLK_MODE_OFF		1U /* Clock is disabled */
#define SC_PM_CLK_MODE_ON		2U /* Clock is enabled. */
#define SC_PM_CLK_MODE_AUTOGATE_SW	3U /* Clock is in SW autogate mode */
#define SC_PM_CLK_MODE_AUTOGATE_HW	4U /* Clock is in HW autogate mode */
#define SC_PM_CLK_MODE_AUTOGATE_SW_HW	5U /* Clock is in SW-HW autogate mode */

typedef u8 sc_pm_power_mode_t;
typedef u8 sc_pm_clk_t;
typedef u8 sc_pm_clk_mode_t;
typedef u8 sc_pm_clk_parent_t;
typedef u32 sc_pm_clock_rate_t;

#endif /* SC_PM_API_H */
