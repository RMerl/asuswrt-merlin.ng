/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010-2014
 * NVIDIA Corporation <www.nvidia.com>
 */

#ifndef _TEGRA_PINMUX_H_
#define _TEGRA_PINMUX_H_

#include <linux/types.h>

#include <asm/arch/tegra.h>

/* The pullup/pulldown state of a pin group */
enum pmux_pull {
	PMUX_PULL_NORMAL = 0,
	PMUX_PULL_DOWN,
	PMUX_PULL_UP,
};

/* Defines whether a pin group is tristated or in normal operation */
enum pmux_tristate {
	PMUX_TRI_NORMAL = 0,
	PMUX_TRI_TRISTATE = 1,
};

#ifdef TEGRA_PMX_PINS_HAVE_E_INPUT
enum pmux_pin_io {
	PMUX_PIN_OUTPUT = 0,
	PMUX_PIN_INPUT = 1,
	PMUX_PIN_NONE,
};
#endif

#ifdef TEGRA_PMX_PINS_HAVE_LOCK
enum pmux_pin_lock {
	PMUX_PIN_LOCK_DEFAULT = 0,
	PMUX_PIN_LOCK_DISABLE,
	PMUX_PIN_LOCK_ENABLE,
};
#endif

#ifdef TEGRA_PMX_PINS_HAVE_OD
enum pmux_pin_od {
	PMUX_PIN_OD_DEFAULT = 0,
	PMUX_PIN_OD_DISABLE,
	PMUX_PIN_OD_ENABLE,
};
#endif

#ifdef TEGRA_PMX_PINS_HAVE_IO_RESET
enum pmux_pin_ioreset {
	PMUX_PIN_IO_RESET_DEFAULT = 0,
	PMUX_PIN_IO_RESET_DISABLE,
	PMUX_PIN_IO_RESET_ENABLE,
};
#endif

#ifdef TEGRA_PMX_PINS_HAVE_RCV_SEL
enum pmux_pin_rcv_sel {
	PMUX_PIN_RCV_SEL_DEFAULT = 0,
	PMUX_PIN_RCV_SEL_NORMAL,
	PMUX_PIN_RCV_SEL_HIGH,
};
#endif

#ifdef TEGRA_PMX_PINS_HAVE_E_IO_HV
enum pmux_pin_e_io_hv {
	PMUX_PIN_E_IO_HV_DEFAULT = 0,
	PMUX_PIN_E_IO_HV_NORMAL,
	PMUX_PIN_E_IO_HV_HIGH,
};
#endif

#ifdef TEGRA_PMX_GRPS_HAVE_LPMD
/* Defines a pin group cfg's low-power mode select */
enum pmux_lpmd {
	PMUX_LPMD_X8 = 0,
	PMUX_LPMD_X4,
	PMUX_LPMD_X2,
	PMUX_LPMD_X,
	PMUX_LPMD_NONE = -1,
};
#endif

#if defined(TEGRA_PMX_PINS_HAVE_SCHMT) || defined(TEGRA_PMX_GRPS_HAVE_SCHMT)
/* Defines whether a pin group cfg's schmidt is enabled or not */
enum pmux_schmt {
	PMUX_SCHMT_DISABLE = 0,
	PMUX_SCHMT_ENABLE = 1,
	PMUX_SCHMT_NONE = -1,
};
#endif

#if defined(TEGRA_PMX_PINS_HAVE_HSM) || defined(TEGRA_PMX_GRPS_HAVE_HSM)
/* Defines whether a pin group cfg's high-speed mode is enabled or not */
enum pmux_hsm {
	PMUX_HSM_DISABLE = 0,
	PMUX_HSM_ENABLE = 1,
	PMUX_HSM_NONE = -1,
};
#endif

/*
 * This defines the configuration for a pin, including the function assigned,
 * pull up/down settings and tristate settings. Having set up one of these
 * you can call pinmux_config_pingroup() to configure a pin in one step. Also
 * available is pinmux_config_table() to configure a list of pins.
 */
struct pmux_pingrp_config {
	u32 pingrp:16;		/* pin group PMUX_PINGRP_...        */
	u32 func:8;		/* function to assign PMUX_FUNC_... */
	u32 pull:2;		/* pull up/down/normal PMUX_PULL_...*/
	u32 tristate:2;		/* tristate or normal PMUX_TRI_...  */
#ifdef TEGRA_PMX_PINS_HAVE_E_INPUT
	u32 io:2;		/* input or output PMUX_PIN_...     */
#endif
#ifdef TEGRA_PMX_PINS_HAVE_LOCK
	u32 lock:2;		/* lock enable/disable PMUX_PIN...  */
#endif
#ifdef TEGRA_PMX_PINS_HAVE_OD
	u32 od:2;		/* open-drain or push-pull driver   */
#endif
#ifdef TEGRA_PMX_PINS_HAVE_IO_RESET
	u32 ioreset:2;		/* input/output reset PMUX_PIN...   */
#endif
#ifdef TEGRA_PMX_PINS_HAVE_RCV_SEL
	u32 rcv_sel:2;		/* select between High and Normal  */
				/* VIL/VIH receivers */
#endif
#ifdef TEGRA_PMX_PINS_HAVE_E_IO_HV
	u32 e_io_hv:2;		/* select 3.3v tolerant receivers */
#endif
#ifdef TEGRA_PMX_PINS_HAVE_SCHMT
	u32 schmt:2;		/* schmitt enable            */
#endif
#ifdef TEGRA_PMX_PINS_HAVE_HSM
	u32 hsm:2;		/* high-speed mode enable    */
#endif
};

#ifdef TEGRA_PMX_SOC_HAS_IO_CLAMPING
/* Set/clear the pinmux CLAMP_INPUTS_WHEN_TRISTATED bit */
void pinmux_set_tristate_input_clamping(void);
void pinmux_clear_tristate_input_clamping(void);
#endif

/* Set the mux function for a pin group */
void pinmux_set_func(enum pmux_pingrp pin, enum pmux_func func);

/* Set the pull up/down feature for a pin group */
void pinmux_set_pullupdown(enum pmux_pingrp pin, enum pmux_pull pupd);

/* Set a pin group to tristate */
void pinmux_tristate_enable(enum pmux_pingrp pin);

/* Set a pin group to normal (non tristate) */
void pinmux_tristate_disable(enum pmux_pingrp pin);

#ifdef TEGRA_PMX_PINS_HAVE_E_INPUT
/* Set a pin group as input or output */
void pinmux_set_io(enum pmux_pingrp pin, enum pmux_pin_io io);
#endif

/**
 * Configure a list of pin groups
 *
 * @param config	List of config items
 * @param len		Number of config items in list
 */
void pinmux_config_pingrp_table(const struct pmux_pingrp_config *config,
				int len);

struct pmux_pingrp_desc {
	u8 funcs[4];
#if defined(CONFIG_TEGRA20)
	u8 ctl_id;
	u8 pull_id;
#endif /* CONFIG_TEGRA20 */
};

extern const struct pmux_pingrp_desc *tegra_soc_pingroups;

#ifdef TEGRA_PMX_SOC_HAS_DRVGRPS

#define PMUX_SLWF_MIN	0
#define PMUX_SLWF_MAX	3
#define PMUX_SLWF_NONE	-1

#define PMUX_SLWR_MIN	0
#define PMUX_SLWR_MAX	3
#define PMUX_SLWR_NONE	-1

#define PMUX_DRVUP_MIN	0
#define PMUX_DRVUP_MAX	127
#define PMUX_DRVUP_NONE	-1

#define PMUX_DRVDN_MIN	0
#define PMUX_DRVDN_MAX	127
#define PMUX_DRVDN_NONE	-1

/*
 * This defines the configuration for a pin group's pad control config
 */
struct pmux_drvgrp_config {
	u32 drvgrp:16;	/* pin group PMUX_DRVGRP_x   */
	u32 slwf:3;		/* falling edge slew         */
	u32 slwr:3;		/* rising edge slew          */
	u32 drvup:8;		/* pull-up drive strength    */
	u32 drvdn:8;		/* pull-down drive strength  */
#ifdef TEGRA_PMX_GRPS_HAVE_LPMD
	u32 lpmd:3;		/* low-power mode selection  */
#endif
#ifdef TEGRA_PMX_GRPS_HAVE_SCHMT
	u32 schmt:2;		/* schmidt enable            */
#endif
#ifdef TEGRA_PMX_GRPS_HAVE_HSM
	u32 hsm:2;		/* high-speed mode enable    */
#endif
};

/**
 * Set the GP pad configs
 *
 * @param config	List of config items
 * @param len		Number of config items in list
 */
void pinmux_config_drvgrp_table(const struct pmux_drvgrp_config *config,
				int len);

#endif /* TEGRA_PMX_SOC_HAS_DRVGRPS */

#ifdef TEGRA_PMX_SOC_HAS_MIPI_PAD_CTRL_GRPS
struct pmux_mipipadctrlgrp_config {
	u32 grp:16;	/* pin group PMUX_MIPIPADCTRLGRP_x   */
	u32 func:8;	/* function to assign PMUX_FUNC_... */
};

void pinmux_config_mipipadctrlgrp_table(
	const struct pmux_mipipadctrlgrp_config *config, int len);

struct pmux_mipipadctrlgrp_desc {
	u8 funcs[2];
};

extern const struct pmux_mipipadctrlgrp_desc *tegra_soc_mipipadctrl_groups;
#endif /* TEGRA_PMX_SOC_HAS_MIPI_PAD_CTRL_GRPS */

#endif /* _TEGRA_PINMUX_H_ */
