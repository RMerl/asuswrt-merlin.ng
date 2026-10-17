// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2010-2013, NVIDIA CORPORATION. All rights reserved.
 * Copyright (c) 2011 The Chromium OS Authors.
 */

#include <common.h>
#include <log.h>
#include <asm/io.h>
#include <asm/arch/pinmux.h>

/* return 1 if a pingrp is in range */
#define pmux_pingrp_isvalid(pin) (((pin) >= 0) && ((pin) < PMUX_PINGRP_COUNT))

/* return 1 if a pmux_func is in range */
#define pmux_func_isvalid(func) \
	(((func) >= 0) && ((func) < PMUX_FUNC_COUNT))

/* return 1 if a pin_pupd_is in range */
#define pmux_pin_pupd_isvalid(pupd) \
	(((pupd) >= PMUX_PULL_NORMAL) && ((pupd) <= PMUX_PULL_UP))

/* return 1 if a pin_tristate_is in range */
#define pmux_pin_tristate_isvalid(tristate) \
	(((tristate) >= PMUX_TRI_NORMAL) && ((tristate) <= PMUX_TRI_TRISTATE))

#ifdef TEGRA_PMX_PINS_HAVE_E_INPUT
/* return 1 if a pin_io_is in range */
#define pmux_pin_io_isvalid(io) \
	(((io) >= PMUX_PIN_OUTPUT) && ((io) <= PMUX_PIN_INPUT))
#endif

#ifdef TEGRA_PMX_PINS_HAVE_LOCK
/* return 1 if a pin_lock is in range */
#define pmux_pin_lock_isvalid(lock) \
	(((lock) >= PMUX_PIN_LOCK_DISABLE) && ((lock) <= PMUX_PIN_LOCK_ENABLE))
#endif

#ifdef TEGRA_PMX_PINS_HAVE_OD
/* return 1 if a pin_od is in range */
#define pmux_pin_od_isvalid(od) \
	(((od) >= PMUX_PIN_OD_DISABLE) && ((od) <= PMUX_PIN_OD_ENABLE))
#endif

#ifdef TEGRA_PMX_PINS_HAVE_IO_RESET
/* return 1 if a pin_ioreset_is in range */
#define pmux_pin_ioreset_isvalid(ioreset) \
	(((ioreset) >= PMUX_PIN_IO_RESET_DISABLE) && \
	 ((ioreset) <= PMUX_PIN_IO_RESET_ENABLE))
#endif

#ifdef TEGRA_PMX_PINS_HAVE_RCV_SEL
/* return 1 if a pin_rcv_sel_is in range */
#define pmux_pin_rcv_sel_isvalid(rcv_sel) \
	(((rcv_sel) >= PMUX_PIN_RCV_SEL_NORMAL) && \
	 ((rcv_sel) <= PMUX_PIN_RCV_SEL_HIGH))
#endif

#ifdef TEGRA_PMX_PINS_HAVE_E_IO_HV
/* return 1 if a pin_e_io_hv is in range */
#define pmux_pin_e_io_hv_isvalid(e_io_hv) \
	(((e_io_hv) >= PMUX_PIN_E_IO_HV_NORMAL) && \
	 ((e_io_hv) <= PMUX_PIN_E_IO_HV_HIGH))
#endif

#ifdef TEGRA_PMX_GRPS_HAVE_LPMD
#define pmux_lpmd_isvalid(lpm) \
	(((lpm) >= PMUX_LPMD_X8) && ((lpm) <= PMUX_LPMD_X))
#endif

#if defined(TEGRA_PMX_PINS_HAVE_SCHMT) || defined(TEGRA_PMX_GRPS_HAVE_SCHMT)
#define pmux_schmt_isvalid(schmt) \
	(((schmt) >= PMUX_SCHMT_DISABLE) && ((schmt) <= PMUX_SCHMT_ENABLE))
#endif

#if defined(TEGRA_PMX_PINS_HAVE_HSM) || defined(TEGRA_PMX_GRPS_HAVE_HSM)
#define pmux_hsm_isvalid(hsm) \
	(((hsm) >= PMUX_HSM_DISABLE) && ((hsm) <= PMUX_HSM_ENABLE))
#endif

#define _R(offset)	(u32 *)((unsigned long)NV_PA_APB_MISC_BASE + (offset))

#if defined(CONFIG_TEGRA20)

#define MUX_REG(grp)	_R(0x80 + ((tegra_soc_pingroups[grp].ctl_id / 16) * 4))
#define MUX_SHIFT(grp)	((tegra_soc_pingroups[grp].ctl_id % 16) * 2)

#define PULL_REG(grp)	_R(0xa0 + ((tegra_soc_pingroups[grp].pull_id / 16) * 4))
#define PULL_SHIFT(grp)	((tegra_soc_pingroups[grp].pull_id % 16) * 2)

#define TRI_REG(grp)	_R(0x14 + (((grp) / 32) * 4))
#define TRI_SHIFT(grp)	((grp) % 32)

#else

#define REG(pin)	_R(0x3000 + ((pin) * 4))

#define MUX_REG(pin)	REG(pin)
#define MUX_SHIFT(pin)	0

#define PULL_REG(pin)	REG(pin)
#define PULL_SHIFT(pin)	2

#define TRI_REG(pin)	REG(pin)
#define TRI_SHIFT(pin)	4

#endif /* CONFIG_TEGRA20 */

#define DRV_REG(group)	_R(TEGRA_PMX_SOC_DRV_GROUP_BASE_REG + ((group) * 4))

#define MIPIPADCTRL_REG(group)	_R(TEGRA_PMX_SOC_MIPIPADCTRL_BASE_REG + ((group) * 4))

/*
 * We could force arch-tegraNN/pinmux.h to define all of these. However,
 * that's a lot of defines, and for now it's manageable to just put a
 * special case here. It's possible this decision will change with future
 * SoCs.
 */
#ifdef CONFIG_TEGRA210
#define IO_SHIFT	6
#define LOCK_SHIFT	7
#ifdef TEGRA_PMX_PINS_HAVE_HSM
#define HSM_SHIFT	9
#endif
#define E_IO_HV_SHIFT	10
#define OD_SHIFT	11
#ifdef TEGRA_PMX_PINS_HAVE_SCHMT
#define SCHMT_SHIFT	12
#endif
#else
#define IO_SHIFT	5
#define OD_SHIFT	6
#define LOCK_SHIFT	7
#define IO_RESET_SHIFT	8
#define RCV_SEL_SHIFT	9
#endif

#ifdef TEGRA_PMX_SOC_HAS_IO_CLAMPING
/* This register/field only exists on Tegra114 and later */
#define APB_MISC_PP_PINMUX_GLOBAL_0 0x40
#define CLAMP_INPUTS_WHEN_TRISTATED 1

void pinmux_set_tristate_input_clamping(void)
{
	u32 *reg = _R(APB_MISC_PP_PINMUX_GLOBAL_0);

	setbits_le32(reg, CLAMP_INPUTS_WHEN_TRISTATED);
}

void pinmux_clear_tristate_input_clamping(void)
{
	u32 *reg = _R(APB_MISC_PP_PINMUX_GLOBAL_0);

	clrbits_le32(reg, CLAMP_INPUTS_WHEN_TRISTATED);
}
#endif

void pinmux_set_func(enum pmux_pingrp pin, enum pmux_func func)
{
	u32 *reg = MUX_REG(pin);
	int i, mux = -1;
	u32 val;

	if (func == PMUX_FUNC_DEFAULT)
		return;

	/* Error check on pin and func */
	assert(pmux_pingrp_isvalid(pin));
	assert(pmux_func_isvalid(func));

	if (func >= PMUX_FUNC_RSVD1) {
		mux = (func - PMUX_FUNC_RSVD1) & 3;
	} else {
		/* Search for the appropriate function */
		for (i = 0; i < 4; i++) {
			if (tegra_soc_pingroups[pin].funcs[i] == func) {
				mux = i;
				break;
			}
		}
	}
	assert(mux != -1);

	val = readl(reg);
	val &= ~(3 << MUX_SHIFT(pin));
	val |= (mux << MUX_SHIFT(pin));
	writel(val, reg);
}

void pinmux_set_pullupdown(enum pmux_pingrp pin, enum pmux_pull pupd)
{
	u32 *reg = PULL_REG(pin);
	u32 val;

	/* Error check on pin and pupd */
	assert(pmux_pingrp_isvalid(pin));
	assert(pmux_pin_pupd_isvalid(pupd));

	val = readl(reg);
	val &= ~(3 << PULL_SHIFT(pin));
	val |= (pupd << PULL_SHIFT(pin));
	writel(val, reg);
}

static void pinmux_set_tristate(enum pmux_pingrp pin, int tri)
{
	u32 *reg = TRI_REG(pin);
	u32 val;

	/* Error check on pin */
	assert(pmux_pingrp_isvalid(pin));
	assert(pmux_pin_tristate_isvalid(tri));

	val = readl(reg);
	if (tri == PMUX_TRI_TRISTATE)
		val |= (1 << TRI_SHIFT(pin));
	else
		val &= ~(1 << TRI_SHIFT(pin));
	writel(val, reg);
}

void pinmux_tristate_enable(enum pmux_pingrp pin)
{
	pinmux_set_tristate(pin, PMUX_TRI_TRISTATE);
}

void pinmux_tristate_disable(enum pmux_pingrp pin)
{
	pinmux_set_tristate(pin, PMUX_TRI_NORMAL);
}

#ifdef TEGRA_PMX_PINS_HAVE_E_INPUT
void pinmux_set_io(enum pmux_pingrp pin, enum pmux_pin_io io)
{
	u32 *reg = REG(pin);
	u32 val;

	if (io == PMUX_PIN_NONE)
		return;

	/* Error check on pin and io */
	assert(pmux_pingrp_isvalid(pin));
	assert(pmux_pin_io_isvalid(io));

	val = readl(reg);
	if (io == PMUX_PIN_INPUT)
		val |= (io & 1) << IO_SHIFT;
	else
		val &= ~(1 << IO_SHIFT);
	writel(val, reg);
}
#endif

#ifdef TEGRA_PMX_PINS_HAVE_LOCK
static void pinmux_set_lock(enum pmux_pingrp pin, enum pmux_pin_lock lock)
{
	u32 *reg = REG(pin);
	u32 val;

	if (lock == PMUX_PIN_LOCK_DEFAULT)
		return;

	/* Error check on pin and lock */
	assert(pmux_pingrp_isvalid(pin));
	assert(pmux_pin_lock_isvalid(lock));

	val = readl(reg);
	if (lock == PMUX_PIN_LOCK_ENABLE) {
		val |= (1 << LOCK_SHIFT);
	} else {
		if (val & (1 << LOCK_SHIFT))
			printf("%s: Cannot clear LOCK bit!\n", __func__);
		val &= ~(1 << LOCK_SHIFT);
	}
	writel(val, reg);

	return;
}
#endif

#ifdef TEGRA_PMX_PINS_HAVE_OD
static void pinmux_set_od(enum pmux_pingrp pin, enum pmux_pin_od od)
{
	u32 *reg = REG(pin);
	u32 val;

	if (od == PMUX_PIN_OD_DEFAULT)
		return;

	/* Error check on pin and od */
	assert(pmux_pingrp_isvalid(pin));
	assert(pmux_pin_od_isvalid(od));

	val = readl(reg);
	if (od == PMUX_PIN_OD_ENABLE)
		val |= (1 << OD_SHIFT);
	else
		val &= ~(1 << OD_SHIFT);
	writel(val, reg);

	return;
}
#endif

#ifdef TEGRA_PMX_PINS_HAVE_IO_RESET
static void pinmux_set_ioreset(enum pmux_pingrp pin,
				enum pmux_pin_ioreset ioreset)
{
	u32 *reg = REG(pin);
	u32 val;

	if (ioreset == PMUX_PIN_IO_RESET_DEFAULT)
		return;

	/* Error check on pin and ioreset */
	assert(pmux_pingrp_isvalid(pin));
	assert(pmux_pin_ioreset_isvalid(ioreset));

	val = readl(reg);
	if (ioreset == PMUX_PIN_IO_RESET_ENABLE)
		val |= (1 << IO_RESET_SHIFT);
	else
		val &= ~(1 << IO_RESET_SHIFT);
	writel(val, reg);

	return;
}
#endif

#ifdef TEGRA_PMX_PINS_HAVE_RCV_SEL
static void pinmux_set_rcv_sel(enum pmux_pingrp pin,
				enum pmux_pin_rcv_sel rcv_sel)
{
	u32 *reg = REG(pin);
	u32 val;

	if (rcv_sel == PMUX_PIN_RCV_SEL_DEFAULT)
		return;

	/* Error check on pin and rcv_sel */
	assert(pmux_pingrp_isvalid(pin));
	assert(pmux_pin_rcv_sel_isvalid(rcv_sel));

	val = readl(reg);
	if (rcv_sel == PMUX_PIN_RCV_SEL_HIGH)
		val |= (1 << RCV_SEL_SHIFT);
	else
		val &= ~(1 << RCV_SEL_SHIFT);
	writel(val, reg);

	return;
}
#endif

#ifdef TEGRA_PMX_PINS_HAVE_E_IO_HV
static void pinmux_set_e_io_hv(enum pmux_pingrp pin,
				enum pmux_pin_e_io_hv e_io_hv)
{
	u32 *reg = REG(pin);
	u32 val;

	if (e_io_hv == PMUX_PIN_E_IO_HV_DEFAULT)
		return;

	/* Error check on pin and e_io_hv */
	assert(pmux_pingrp_isvalid(pin));
	assert(pmux_pin_e_io_hv_isvalid(e_io_hv));

	val = readl(reg);
	if (e_io_hv == PMUX_PIN_E_IO_HV_HIGH)
		val |= (1 << E_IO_HV_SHIFT);
	else
		val &= ~(1 << E_IO_HV_SHIFT);
	writel(val, reg);

	return;
}
#endif

#ifdef TEGRA_PMX_PINS_HAVE_SCHMT
static void pinmux_set_schmt(enum pmux_pingrp pin, enum pmux_schmt schmt)
{
	u32 *reg = REG(grp);
	u32 val;

	/* NONE means unspecified/do not change/use POR value */
	if (schmt == PMUX_SCHMT_NONE)
		return;

	/* Error check pad */
	assert(pmux_pingrp_isvalid(pin));
	assert(pmux_schmt_isvalid(schmt));

	val = readl(reg);
	if (schmt == PMUX_SCHMT_ENABLE)
		val |= (1 << SCHMT_SHIFT);
	else
		val &= ~(1 << SCHMT_SHIFT);
	writel(val, reg);

	return;
}
#endif

#ifdef TEGRA_PMX_PINS_HAVE_HSM
static void pinmux_set_hsm(enum pmux_pingrp pin, enum pmux_hsm hsm)
{
	u32 *reg = REG(grp);
	u32 val;

	/* NONE means unspecified/do not change/use POR value */
	if (hsm == PMUX_HSM_NONE)
		return;

	/* Error check pad */
	assert(pmux_pingrp_isvalid(pin));
	assert(pmux_hsm_isvalid(hsm));

	val = readl(reg);
	if (hsm == PMUX_HSM_ENABLE)
		val |= (1 << HSM_SHIFT);
	else
		val &= ~(1 << HSM_SHIFT);
	writel(val, reg);

	return;
}
#endif

static void pinmux_config_pingrp(const struct pmux_pingrp_config *config)
{
	enum pmux_pingrp pin = config->pingrp;

	pinmux_set_func(pin, config->func);
	pinmux_set_pullupdown(pin, config->pull);
	pinmux_set_tristate(pin, config->tristate);
#ifdef TEGRA_PMX_PINS_HAVE_E_INPUT
	pinmux_set_io(pin, config->io);
#endif
#ifdef TEGRA_PMX_PINS_HAVE_LOCK
	pinmux_set_lock(pin, config->lock);
#endif
#ifdef TEGRA_PMX_PINS_HAVE_OD
	pinmux_set_od(pin, config->od);
#endif
#ifdef TEGRA_PMX_PINS_HAVE_IO_RESET
	pinmux_set_ioreset(pin, config->ioreset);
#endif
#ifdef TEGRA_PMX_PINS_HAVE_RCV_SEL
	pinmux_set_rcv_sel(pin, config->rcv_sel);
#endif
#ifdef TEGRA_PMX_PINS_HAVE_E_IO_HV
	pinmux_set_e_io_hv(pin, config->e_io_hv);
#endif
#ifdef TEGRA_PMX_PINS_HAVE_SCHMT
	pinmux_set_schmt(pin, config->schmt);
#endif
#ifdef TEGRA_PMX_PINS_HAVE_HSM
	pinmux_set_hsm(pin, config->hsm);
#endif
}

void pinmux_config_pingrp_table(const struct pmux_pingrp_config *config,
				int len)
{
	int i;

	for (i = 0; i < len; i++)
		pinmux_config_pingrp(&config[i]);
}

#ifdef TEGRA_PMX_SOC_HAS_DRVGRPS

#define pmux_drvgrp_isvalid(pd) (((pd) >= 0) && ((pd) < PMUX_DRVGRP_COUNT))

#define pmux_slw_isvalid(slw) \
	(((slw) >= PMUX_SLWF_MIN) && ((slw) <= PMUX_SLWF_MAX))

#define pmux_drv_isvalid(drv) \
	(((drv) >= PMUX_DRVUP_MIN) && ((drv) <= PMUX_DRVUP_MAX))

#ifdef TEGRA_PMX_GRPS_HAVE_HSM
#define HSM_SHIFT	2
#endif
#ifdef TEGRA_PMX_GRPS_HAVE_SCHMT
#define SCHMT_SHIFT	3
#endif
#ifdef TEGRA_PMX_GRPS_HAVE_LPMD
#define LPMD_SHIFT	4
#define LPMD_MASK	(3 << LPMD_SHIFT)
#endif
/*
 * Note that the following DRV* and SLW* defines are accurate for many drive
 * groups on many SoCs. We really need a per-group data structure to solve
 * this, since the fields are in different positions/sizes in different
 * registers (for different groups).
 *
 * On Tegra30/114/124, the DRV*_SHIFT values vary.
 * On Tegra30, the SLW*_SHIFT values vary.
 * On Tegra30/114/124/210, the DRV*_MASK values vary, although the values
 *   below are wide enough to cover the widest fields, and hopefully don't
 *   interfere with any other fields.
 * On Tegra30, the SLW*_MASK values vary, but we can't use a value that's
 *   wide enough to cover all cases, since that would cause the field to
 *   overlap with other fields in the narrower cases.
 */
#define DRVDN_SHIFT	12
#define DRVDN_MASK	(0x7F << DRVDN_SHIFT)
#define DRVUP_SHIFT	20
#define DRVUP_MASK	(0x7F << DRVUP_SHIFT)
#define SLWR_SHIFT	28
#define SLWR_MASK	(3 << SLWR_SHIFT)
#define SLWF_SHIFT	30
#define SLWF_MASK	(3 << SLWF_SHIFT)

static void pinmux_set_drvup_slwf(enum pmux_drvgrp grp, int slwf)
{
	u32 *reg = DRV_REG(grp);
	u32 val;

	/* NONE means unspecified/do not change/use POR value */
	if (slwf == PMUX_SLWF_NONE)
		return;

	/* Error check on pad and slwf */
	assert(pmux_drvgrp_isvalid(grp));
	assert(pmux_slw_isvalid(slwf));

	val = readl(reg);
	val &= ~SLWF_MASK;
	val |= (slwf << SLWF_SHIFT);
	writel(val, reg);

	return;
}

static void pinmux_set_drvdn_slwr(enum pmux_drvgrp grp, int slwr)
{
	u32 *reg = DRV_REG(grp);
	u32 val;

	/* NONE means unspecified/do not change/use POR value */
	if (slwr == PMUX_SLWR_NONE)
		return;

	/* Error check on pad and slwr */
	assert(pmux_drvgrp_isvalid(grp));
	assert(pmux_slw_isvalid(slwr));

	val = readl(reg);
	val &= ~SLWR_MASK;
	val |= (slwr << SLWR_SHIFT);
	writel(val, reg);

	return;
}

static void pinmux_set_drvup(enum pmux_drvgrp grp, int drvup)
{
	u32 *reg = DRV_REG(grp);
	u32 val;

	/* NONE means unspecified/do not change/use POR value */
	if (drvup == PMUX_DRVUP_NONE)
		return;

	/* Error check on pad and drvup */
	assert(pmux_drvgrp_isvalid(grp));
	assert(pmux_drv_isvalid(drvup));

	val = readl(reg);
	val &= ~DRVUP_MASK;
	val |= (drvup << DRVUP_SHIFT);
	writel(val, reg);

	return;
}

static void pinmux_set_drvdn(enum pmux_drvgrp grp, int drvdn)
{
	u32 *reg = DRV_REG(grp);
	u32 val;

	/* NONE means unspecified/do not change/use POR value */
	if (drvdn == PMUX_DRVDN_NONE)
		return;

	/* Error check on pad and drvdn */
	assert(pmux_drvgrp_isvalid(grp));
	assert(pmux_drv_isvalid(drvdn));

	val = readl(reg);
	val &= ~DRVDN_MASK;
	val |= (drvdn << DRVDN_SHIFT);
	writel(val, reg);

	return;
}

#ifdef TEGRA_PMX_GRPS_HAVE_LPMD
static void pinmux_set_lpmd(enum pmux_drvgrp grp, enum pmux_lpmd lpmd)
{
	u32 *reg = DRV_REG(grp);
	u32 val;

	/* NONE means unspecified/do not change/use POR value */
	if (lpmd == PMUX_LPMD_NONE)
		return;

	/* Error check pad and lpmd value */
	assert(pmux_drvgrp_isvalid(grp));
	assert(pmux_lpmd_isvalid(lpmd));

	val = readl(reg);
	val &= ~LPMD_MASK;
	val |= (lpmd << LPMD_SHIFT);
	writel(val, reg);

	return;
}
#endif

#ifdef TEGRA_PMX_GRPS_HAVE_SCHMT
static void pinmux_set_schmt(enum pmux_drvgrp grp, enum pmux_schmt schmt)
{
	u32 *reg = DRV_REG(grp);
	u32 val;

	/* NONE means unspecified/do not change/use POR value */
	if (schmt == PMUX_SCHMT_NONE)
		return;

	/* Error check pad */
	assert(pmux_drvgrp_isvalid(grp));
	assert(pmux_schmt_isvalid(schmt));

	val = readl(reg);
	if (schmt == PMUX_SCHMT_ENABLE)
		val |= (1 << SCHMT_SHIFT);
	else
		val &= ~(1 << SCHMT_SHIFT);
	writel(val, reg);

	return;
}
#endif

#ifdef TEGRA_PMX_GRPS_HAVE_HSM
static void pinmux_set_hsm(enum pmux_drvgrp grp, enum pmux_hsm hsm)
{
	u32 *reg = DRV_REG(grp);
	u32 val;

	/* NONE means unspecified/do not change/use POR value */
	if (hsm == PMUX_HSM_NONE)
		return;

	/* Error check pad */
	assert(pmux_drvgrp_isvalid(grp));
	assert(pmux_hsm_isvalid(hsm));

	val = readl(reg);
	if (hsm == PMUX_HSM_ENABLE)
		val |= (1 << HSM_SHIFT);
	else
		val &= ~(1 << HSM_SHIFT);
	writel(val, reg);

	return;
}
#endif

static void pinmux_config_drvgrp(const struct pmux_drvgrp_config *config)
{
	enum pmux_drvgrp grp = config->drvgrp;

	pinmux_set_drvup_slwf(grp, config->slwf);
	pinmux_set_drvdn_slwr(grp, config->slwr);
	pinmux_set_drvup(grp, config->drvup);
	pinmux_set_drvdn(grp, config->drvdn);
#ifdef TEGRA_PMX_GRPS_HAVE_LPMD
	pinmux_set_lpmd(grp, config->lpmd);
#endif
#ifdef TEGRA_PMX_GRPS_HAVE_SCHMT
	pinmux_set_schmt(grp, config->schmt);
#endif
#ifdef TEGRA_PMX_GRPS_HAVE_HSM
	pinmux_set_hsm(grp, config->hsm);
#endif
}

void pinmux_config_drvgrp_table(const struct pmux_drvgrp_config *config,
				int len)
{
	int i;

	for (i = 0; i < len; i++)
		pinmux_config_drvgrp(&config[i]);
}
#endif /* TEGRA_PMX_SOC_HAS_DRVGRPS */

#ifdef TEGRA_PMX_SOC_HAS_MIPI_PAD_CTRL_GRPS

#define pmux_mipipadctrlgrp_isvalid(pd) (((pd) >= 0) && ((pd) < PMUX_MIPIPADCTRLGRP_COUNT))

static void pinmux_mipipadctrl_set_func(enum pmux_mipipadctrlgrp grp,
	enum pmux_func func)
{
	u32 *reg = MIPIPADCTRL_REG(grp);
	int i, mux = -1;
	u32 val;

	if (func == PMUX_FUNC_DEFAULT)
		return;

	/* Error check grp and func */
	assert(pmux_mipipadctrlgrp_isvalid(grp));
	assert(pmux_func_isvalid(func));

	if (func >= PMUX_FUNC_RSVD1) {
		mux = (func - PMUX_FUNC_RSVD1) & 1;
	} else {
		/* Search for the appropriate function */
		for (i = 0; i < 2; i++) {
			if (tegra_soc_mipipadctrl_groups[grp].funcs[i]
			    == func) {
				mux = i;
				break;
			}
		}
	}
	assert(mux != -1);

	val = readl(reg);
	val &= ~(1 << 1);
	val |= (mux << 1);
	writel(val, reg);
}

static void pinmux_config_mipipadctrlgrp(const struct pmux_mipipadctrlgrp_config *config)
{
	enum pmux_mipipadctrlgrp grp = config->grp;

	pinmux_mipipadctrl_set_func(grp, config->func);
}

void pinmux_config_mipipadctrlgrp_table(
	const struct pmux_mipipadctrlgrp_config *config, int len)
{
	int i;

	for (i = 0; i < len; i++)
		pinmux_config_mipipadctrlgrp(&config[i]);
}
#endif /* TEGRA_PMX_SOC_HAS_MIPI_PAD_CTRL_GRPS */
