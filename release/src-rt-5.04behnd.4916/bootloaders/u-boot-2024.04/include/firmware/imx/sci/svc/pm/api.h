/* SPDX-License-Identifier:     GPL-2.0+ */
/*
 * Copyright 2018 NXP
 */

#ifndef SC_PM_API_H
#define SC_PM_API_H

#include <firmware/imx/sci/types.h>
/* Defines for type widths */
#define SC_PM_POWER_MODE_W      2U      /* Width of sc_pm_power_mode_t */
#define SC_PM_CLOCK_MODE_W      3U      /* Width of sc_pm_clock_mode_t */
#define SC_PM_RESET_TYPE_W      2U      /* Width of sc_pm_reset_type_t */
#define SC_PM_RESET_REASON_W    4U      /* Width of sc_pm_reset_reason_t */
/* Defines for ALL parameters */
#define SC_PM_CLK_ALL   ((sc_pm_clk_t)UINT8_MAX)   /* All clocks */
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

/* Defines for sc_pm_clk_parent_t */
#define SC_PM_PARENT_XTAL              0U    /*!< Parent is XTAL. */
#define SC_PM_PARENT_PLL0              1U    /*!< Parent is PLL0 */
#define SC_PM_PARENT_PLL1              2U    /*!< Parent is PLL1 or PLL0/2 */
#define SC_PM_PARENT_PLL2              3U    /*!< Parent in PLL2 or PLL0/4 */
#define SC_PM_PARENT_BYPS              4U    /*!< Parent is a bypass clock. */

/* Defines for sc_pm_reset_type_t */
#define SC_PM_RESET_TYPE_COLD          0U    /* Cold reset */
#define SC_PM_RESET_TYPE_WARM          1U    /* Warm reset */
#define SC_PM_RESET_TYPE_BOARD         2U    /* Board reset */

/* Defines for sc_pm_reset_reason_t */
#define SC_PM_RESET_REASON_POR         0U    /* Power on reset */
#define SC_PM_RESET_REASON_JTAG        1U    /* JTAG reset */
#define SC_PM_RESET_REASON_SW          2U    /* Software reset */
#define SC_PM_RESET_REASON_WDOG        3U    /* Partition watchdog reset */
#define SC_PM_RESET_REASON_LOCKUP      4U    /* SCU lockup reset */
#define SC_PM_RESET_REASON_SNVS        5U    /* SNVS reset */
#define SC_PM_RESET_REASON_TEMP        6U    /* Temp panic reset */
#define SC_PM_RESET_REASON_MSI         7U    /* MSI reset */
#define SC_PM_RESET_REASON_UECC        8U    /* ECC reset */
#define SC_PM_RESET_REASON_SCFW_WDOG   9U    /* SCFW watchdog reset */
#define SC_PM_RESET_REASON_ROM_WDOG    10U   /* SCU ROM watchdog reset */
#define SC_PM_RESET_REASON_SECO        11U   /* SECO reset */
#define SC_PM_RESET_REASON_SCFW_FAULT  12U   /* SCFW fault reset */

/* Defines for sc_pm_sys_if_t */
#define SC_PM_SYS_IF_INTERCONNECT       0U   /* System interconnect */
#define SC_PM_SYS_IF_MU                 1U   /* AP -> SCU message units */
#define SC_PM_SYS_IF_OCMEM              2U   /* On-chip memory (ROM/OCRAM) */
#define SC_PM_SYS_IF_DDR                3U   /* DDR memory */

/* Defines for sc_pm_wake_src_t */
/* No wake source, used for self-kill */
#define SC_PM_WAKE_SRC_NONE             0U
/* Wakeup from SCU to resume CPU (IRQSTEER & GIC powered down) */
#define SC_PM_WAKE_SRC_SCU              1U
/* Wakeup from IRQSTEER to resume CPU (GIC powered down) */
#define SC_PM_WAKE_SRC_IRQSTEER         2U
/* Wakeup from IRQSTEER+GIC to wake CPU (GIC clock gated) */
#define SC_PM_WAKE_SRC_IRQSTEER_GIC     3U
/* Wakeup from GIC to wake CPU */
#define SC_PM_WAKE_SRC_GIC              4U
/* Types */

/*
 * This type is used to declare a power mode. Note resources only use
 * SC_PM_PW_MODE_OFF and SC_PM_PW_MODE_ON. The other modes are used only
 * as system power modes.
 */
typedef u8 sc_pm_power_mode_t;

/*
 * This type is used to declare a clock.
 */
typedef u8 sc_pm_clk_t;

/*
 * This type is used to declare a clock mode.
 */
typedef u8 sc_pm_clk_mode_t;

/*
 * This type is used to declare the clock parent.
 */
typedef u8 sc_pm_clk_parent_t;

/*
 * This type is used to declare clock rates.
 */
typedef u32 sc_pm_clock_rate_t;

/*
 * This type is used to declare a desired reset type.
 */
typedef u8 sc_pm_reset_type_t;

/*
 * This type is used to declare a reason for a reset.
 */
typedef u8 sc_pm_reset_reason_t;

/*
 * This type is used to specify a system-level interface to be power managed.
 */
typedef u8 sc_pm_sys_if_t;

/*
 * This type is used to specify a wake source for CPU resources.
 */
typedef u8 sc_pm_wake_src_t;
#endif /* SC_PM_API_H */
