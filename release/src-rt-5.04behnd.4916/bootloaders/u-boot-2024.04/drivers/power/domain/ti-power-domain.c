// SPDX-License-Identifier: GPL-2.0+
/*
 * Texas Instruments power domain driver
 *
 * Copyright (C) 2020-2021 Texas Instruments Incorporated - https://www.ti.com/
 *	Tero Kristo <t-kristo@ti.com>
 */

#include <asm/io.h>
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <power-domain-uclass.h>
#include <soc.h>
#include <k3-dev.h>
#include <linux/iopoll.h>

#define PSC_PTCMD		0x120
#define PSC_PTCMD_H		0x124
#define PSC_PTSTAT		0x128
#define PSC_PTSTAT_H		0x12C
#define PSC_PDSTAT		0x200
#define PSC_PDCTL		0x300
#define PSC_MDSTAT		0x800
#define PSC_MDCTL		0xa00

#define PDCTL_STATE_MASK		0x1
#define PDCTL_STATE_OFF			0x0
#define PDCTL_STATE_ON			0x1

#define MDSTAT_STATE_MASK		0x3f
#define MDSTAT_BUSY_MASK		0x30
#define MDSTAT_STATE_SWRSTDISABLE	0x0
#define MDSTAT_STATE_ENABLE		0x3

#define LPSC_TIMEOUT		1000
#define PD_TIMEOUT		1000

static u32 psc_read(struct ti_psc *psc, u32 reg)
{
	u32 val;

	val = readl(psc->base + reg);
	debug("%s: 0x%x from %p\n", __func__, val, psc->base + reg);
	return val;
}

static void psc_write(u32 val, struct ti_psc *psc, u32 reg)
{
	debug("%s: 0x%x to %p\n", __func__, val, psc->base + reg);
	writel(val, psc->base + reg);
}

static u32 pd_read(struct ti_pd *pd, u32 reg)
{
	return psc_read(pd->psc, reg + 4 * pd->id);
}

static void pd_write(u32 val, struct ti_pd *pd, u32 reg)
{
	psc_write(val, pd->psc, reg + 4 * pd->id);
}

static u32 lpsc_read(struct ti_lpsc *lpsc, u32 reg)
{
	return psc_read(lpsc->psc, reg + 4 * lpsc->id);
}

static void lpsc_write(u32 val, struct ti_lpsc *lpsc, u32 reg)
{
	psc_write(val, lpsc->psc, reg + 4 * lpsc->id);
}

static const struct soc_attr ti_k3_soc_pd_data[] = {
#if IS_ENABLED(CONFIG_SOC_K3_J721E)
	{
		.family = "J721E",
		.data = &j721e_pd_platdata,
	},
	{
		.family = "J7200",
		.data = &j7200_pd_platdata,
	},
#elif CONFIG_SOC_K3_J721S2
	{
		.family = "J721S2",
		.data = &j721s2_pd_platdata,
	},
#endif
#ifdef CONFIG_SOC_K3_AM625
	{
		.family = "AM62X",
		.data = &am62x_pd_platdata,
	},
#endif
#ifdef CONFIG_SOC_K3_AM62A7
	{
		.family = "AM62AX",
		.data = &am62ax_pd_platdata,
	},
#endif
	{ /* sentinel */ }
};

static int ti_power_domain_probe(struct udevice *dev)
{
	struct ti_k3_pd_platdata *data = dev_get_priv(dev);
	const struct soc_attr *soc_match_data;
	const struct ti_k3_pd_platdata *pdata;

	printf("%s(dev=%p)\n", __func__, dev);

	if (!data)
		return -ENOMEM;

	soc_match_data = soc_device_match(ti_k3_soc_pd_data);
	if (!soc_match_data)
		return -ENODEV;

	pdata = (const struct ti_k3_pd_platdata *)soc_match_data->data;

	data->psc = pdata->psc;
	data->pd = pdata->pd;
	data->lpsc = pdata->lpsc;
	data->devs = pdata->devs;
	data->num_psc = pdata->num_psc;
	data->num_pd = pdata->num_pd;
	data->num_lpsc = pdata->num_lpsc;
	data->num_devs = pdata->num_devs;

	return 0;
}

static int ti_pd_wait(struct ti_pd *pd)
{
	u32 ptstat;
	u32 pdoffset = 0;
	u32 ptstatreg = PSC_PTSTAT;
	int ret;

	if (pd->id > 31) {
		pdoffset = 32;
		ptstatreg = PSC_PTSTAT_H;
	}

	ret = readl_poll_timeout(pd->psc->base + ptstatreg, ptstat,
				 !(ptstat & BIT(pd->id - pdoffset)), PD_TIMEOUT);

	if (ret)
		printf("%s: psc%d, pd%d failed to transition.\n", __func__,
		       pd->psc->id, pd->id);

	return ret;
}

static void ti_pd_transition(struct ti_pd *pd)
{
	u32 pdoffset = 0;
	u32 ptcmdreg = PSC_PTCMD;

	if (pd->id > 31) {
		pdoffset = 32;
		ptcmdreg = PSC_PTCMD_H;
	}

	psc_write(BIT(pd->id - pdoffset), pd->psc, ptcmdreg);
}

u8 ti_pd_state(struct ti_pd *pd)
{
	return pd_read(pd, PSC_PDCTL) & PDCTL_STATE_MASK;
}

static int ti_pd_get(struct ti_pd *pd)
{
	u32 pdctl;
	int ret;

	pd->usecount++;

	if (pd->usecount > 1)
		return 0;

	if (pd->depend) {
		ret = ti_pd_get(pd->depend);
		if (ret)
			return ret;
		ti_pd_transition(pd->depend);
		ret = ti_pd_wait(pd->depend);
		if (ret)
			return ret;
	}

	pdctl = pd_read(pd, PSC_PDCTL);

	if ((pdctl & PDCTL_STATE_MASK) == PDCTL_STATE_ON)
		return 0;

	debug("%s: enabling psc:%d, pd:%d\n", __func__, pd->psc->id, pd->id);

	pdctl &= ~PDCTL_STATE_MASK;
	pdctl |= PDCTL_STATE_ON;

	pd_write(pdctl, pd, PSC_PDCTL);

	return 0;
}

static int ti_pd_put(struct ti_pd *pd)
{
	u32 pdctl;
	int ret;

	pd->usecount--;

	if (pd->usecount > 0)
		return 0;

	pdctl = pd_read(pd, PSC_PDCTL);
	if ((pdctl & PDCTL_STATE_MASK) == PDCTL_STATE_OFF)
		return 0;

	pdctl &= ~PDCTL_STATE_MASK;
	pdctl |= PDCTL_STATE_OFF;

	debug("%s: disabling psc:%d, pd:%d\n", __func__, pd->psc->id, pd->id);

	pd_write(pdctl, pd, PSC_PDCTL);

	if (pd->depend) {
		ti_pd_transition(pd);
		ret = ti_pd_wait(pd);
		if (ret)
			return ret;

		ret = ti_pd_put(pd->depend);
		if (ret)
			return ret;
		ti_pd_transition(pd->depend);
		ret = ti_pd_wait(pd->depend);
		if (ret)
			return ret;
	}

	return 0;
}

static int ti_lpsc_wait(struct ti_lpsc *lpsc)
{
	u32 mdstat;
	int ret;

	ret = readl_poll_timeout(lpsc->psc->base + PSC_MDSTAT + lpsc->id * 4,
				 mdstat,
				 !(mdstat & MDSTAT_BUSY_MASK), LPSC_TIMEOUT);

	if (ret)
		printf("%s: module %d failed to transition.\n", __func__,
		       lpsc->id);

	return ret;
}

u8 lpsc_get_state(struct ti_lpsc *lpsc)
{
	return lpsc_read(lpsc, PSC_MDCTL) & MDSTAT_STATE_MASK;
}

int ti_lpsc_transition(struct ti_lpsc *lpsc, u8 state)
{
	struct ti_pd *psc_pd;
	int ret;
	u32 mdctl;

	psc_pd = lpsc->pd;

	if (state == MDSTAT_STATE_ENABLE) {
		lpsc->usecount++;
		if (lpsc->usecount > 1)
			return 0;
	} else {
		lpsc->usecount--;
		if (lpsc->usecount >= 1)
			return 0;
	}

	debug("%s: transitioning psc:%d, lpsc:%d to %x\n", __func__,
	      lpsc->psc->id, lpsc->id, state);

	if (lpsc->depend)
		ti_lpsc_transition(lpsc->depend, state);

	mdctl = lpsc_read(lpsc, PSC_MDCTL);
	if ((mdctl & MDSTAT_STATE_MASK) == state)
		return 0;

	if (state == MDSTAT_STATE_ENABLE)
		ti_pd_get(psc_pd);
	else
		ti_pd_put(psc_pd);

	mdctl &= ~MDSTAT_STATE_MASK;
	mdctl |= state;

	lpsc_write(mdctl, lpsc, PSC_MDCTL);

	ti_pd_transition(psc_pd);
	ret = ti_pd_wait(psc_pd);
	if (ret)
		return ret;

	return ti_lpsc_wait(lpsc);
}

static int ti_power_domain_transition(struct power_domain *pd, u8 state)
{
	struct ti_lpsc *lpsc = pd->priv;

	return ti_lpsc_transition(lpsc, state);
}

static int ti_power_domain_on(struct power_domain *pd)
{
	debug("%s(pd=%p, id=%lu)\n", __func__, pd, pd->id);

	return ti_power_domain_transition(pd, MDSTAT_STATE_ENABLE);
}

static int ti_power_domain_off(struct power_domain *pd)
{
	debug("%s(pd=%p, id=%lu)\n", __func__, pd, pd->id);

	return ti_power_domain_transition(pd, MDSTAT_STATE_SWRSTDISABLE);
}

static struct ti_lpsc *lpsc_lookup(struct ti_k3_pd_platdata *data, int id)
{
	int idx;

	for (idx = 0; idx < data->num_devs; idx++)
		if (data->devs[idx].id == id)
			return data->devs[idx].lpsc;

	return NULL;
}

static int ti_power_domain_of_xlate(struct power_domain *pd,
				    struct ofnode_phandle_args *args)
{
	struct ti_k3_pd_platdata *data = dev_get_priv(pd->dev);
	struct ti_lpsc *lpsc;

	debug("%s(power_domain=%p, id=%d)\n", __func__, pd, args->args[0]);

	if (args->args_count < 1) {
		printf("Invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	lpsc = lpsc_lookup(data, args->args[0]);
	if (!lpsc) {
		printf("%s: invalid dev-id: %d\n", __func__, args->args[0]);
		return -ENOENT;
	}

	pd->id = lpsc->id;
	pd->priv = lpsc;

	return 0;
}
static const struct udevice_id ti_power_domain_of_match[] = {
	{ .compatible = "ti,sci-pm-domain" },
	{ /* sentinel */ }
};

static struct power_domain_ops ti_power_domain_ops = {
	.on = ti_power_domain_on,
	.off = ti_power_domain_off,
	.of_xlate = ti_power_domain_of_xlate,
};

U_BOOT_DRIVER(ti_pm_domains) = {
	.name = "ti-pm-domains",
	.id = UCLASS_POWER_DOMAIN,
	.of_match = ti_power_domain_of_match,
	.probe = ti_power_domain_probe,
	.priv_auto = sizeof(struct ti_k3_pd_platdata),
	.ops = &ti_power_domain_ops,
};
