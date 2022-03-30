// SPDX-License-Identifier: GPL-2.0+
/*
 * Keystone: PSC configuration module
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#include <common.h>
#include <linux/errno.h>
#include <asm/io.h>
#include <asm/processor.h>
#include <asm/arch/psc_defs.h>

/**
 * psc_delay() - delay for psc
 *
 * Return: 10
 */
int psc_delay(void)
{
	udelay(10);
	return 10;
}

/**
 * psc_wait() - Wait for end of transitional state
 * @domain_num: GPSC domain number
 *
 * Polls pstat for the selected domain and waits for transitions to be complete.
 * Since this is boot loader code it is *ASSUMED* that interrupts are disabled
 * and no other core is mucking around with the psc at the same time.
 *
 * Return: 0 when the domain is free. Returns -1 if a timeout occurred waiting
 * for the completion.
 */
int psc_wait(u32 domain_num)
{
	u32 retry;
	u32 ptstat;

	/*
	 * Do nothing if the power domain is in transition. This should never
	 * happen since the boot code is the only software accesses psc.
	 * It's still remotely possible that the hardware state machines
	 * initiate transitions.
	 * Don't trap if the domain (or a module in this domain) is
	 * stuck in transition.
	 */
	retry = 0;

	do {
		ptstat = __raw_readl(KS2_PSC_BASE + PSC_REG_PSTAT);
		ptstat = ptstat & (1 << domain_num);
	} while ((ptstat != 0) && ((retry += psc_delay()) <
		 PSC_PTSTAT_TIMEOUT_LIMIT));

	if (retry >= PSC_PTSTAT_TIMEOUT_LIMIT)
		return -1;

	return 0;
}

/**
 * psc_get_domain_num() - Get the domain number
 * @mod_num:	LPSC module number
 */
u32 psc_get_domain_num(u32 mod_num)
{
	u32 domain_num;

	/* Get the power domain associated with the module number */
	domain_num = __raw_readl(KS2_PSC_BASE + PSC_REG_MDCFG(mod_num));
	domain_num = PSC_REG_MDCFG_GET_PD(domain_num);

	return domain_num;
}

/**
 * psc_set_state() - powers up/down a module
 * @mod_num:	LPSC module number
 * @state:	1 to enable, 0 to disable.
 *
 * Powers up/down the requested module and the associated power domain if
 * required. No action is taken it the module is already powered up/down.
 * This only controls modules. The domain in which the module resides will
 * be left in the power on state. Multiple modules can exist in a power
 * domain, so powering down the domain based on a single module is not done.
 *
 * Return: 0 on success, -1 if the module can't be powered up, or if there is a
 * timeout waiting for the transition.
 */
int psc_set_state(u32 mod_num, u32 state)
{
	u32 domain_num;
	u32 pdctl;
	u32 mdctl;
	u32 ptcmd;
	u32 reset_iso;
	u32 v;

	/*
	 * Get the power domain associated with the module number, and reset
	 * isolation functionality
	 */
	v = __raw_readl(KS2_PSC_BASE + PSC_REG_MDCFG(mod_num));
	domain_num = PSC_REG_MDCFG_GET_PD(v);
	reset_iso  = PSC_REG_MDCFG_GET_RESET_ISO(v);

	/* Wait for the status of the domain/module to be non-transitional */
	if (psc_wait(domain_num) != 0)
		return -1;

	/*
	 * Perform configuration even if the current status matches the
	 * existing state
	 *
	 * Set the next state of the power domain to on. It's OK if the domain
	 * is always on. This code will not ever power down a domain, so no
	 * change is made if the new state is power down.
	 */
	if (state == PSC_REG_VAL_MDCTL_NEXT_ON) {
		pdctl = __raw_readl(KS2_PSC_BASE + PSC_REG_PDCTL(domain_num));
		pdctl = PSC_REG_PDCTL_SET_NEXT(pdctl,
					       PSC_REG_VAL_PDCTL_NEXT_ON);
		__raw_writel(pdctl, KS2_PSC_BASE + PSC_REG_PDCTL(domain_num));
	}

	/* Set the next state for the module to enabled/disabled */
	mdctl = __raw_readl(KS2_PSC_BASE + PSC_REG_MDCTL(mod_num));
	mdctl = PSC_REG_MDCTL_SET_NEXT(mdctl, state);
	mdctl = PSC_REG_MDCTL_SET_RESET_ISO(mdctl, reset_iso);
	__raw_writel(mdctl, KS2_PSC_BASE + PSC_REG_MDCTL(mod_num));

	/* Trigger the enable */
	ptcmd = __raw_readl(KS2_PSC_BASE + PSC_REG_PTCMD);
	ptcmd |= (u32)(1<<domain_num);
	__raw_writel(ptcmd, KS2_PSC_BASE + PSC_REG_PTCMD);

	/* Wait on the complete */
	return psc_wait(domain_num);
}

/**
 * psc_enable_module() - power up a module
 * @mod_num:	LPSC module number
 *
 * Powers up the requested module and the associated power domain
 * if required. No action is taken it the module is already powered up.
 *
 * Return: 0 on success, -1 if the module can't be powered up, or
 * if there is a timeout waiting for the transition.
 *
 */
int psc_enable_module(u32 mod_num)
{
	u32 mdctl;

	/* Set the bit to apply reset */
	mdctl = __raw_readl(KS2_PSC_BASE + PSC_REG_MDCTL(mod_num));
	if ((mdctl & 0x3f) == PSC_REG_VAL_MDSTAT_STATE_ON)
		return 0;

	return psc_set_state(mod_num, PSC_REG_VAL_MDCTL_NEXT_ON);
}

/**
 * psc_disable_module() - Power down a module
 * @mod_num:	LPSC module number
 *
 * Return: 0 on success, -1 on failure or timeout.
 */
int psc_disable_module(u32 mod_num)
{
	u32 mdctl;

	/* Set the bit to apply reset */
	mdctl = __raw_readl(KS2_PSC_BASE + PSC_REG_MDCTL(mod_num));
	if ((mdctl & 0x3f) == 0)
		return 0;
	mdctl = PSC_REG_MDCTL_SET_LRSTZ(mdctl, 0);
	__raw_writel(mdctl, KS2_PSC_BASE + PSC_REG_MDCTL(mod_num));

	return psc_set_state(mod_num, PSC_REG_VAL_MDCTL_NEXT_SWRSTDISABLE);
}

/**
 * psc_set_reset_iso() - Set the reset isolation bit in mdctl
 * @mod_num:	LPSC module number
 *
 * The reset isolation enable bit is set. The state of the module is not
 * changed.
 *
 * Return: 0 if the module config showed that reset isolation is supported.
 * Returns 1 otherwise. This is not an error, but setting the bit in mdctl
 * has no effect.
 */
int psc_set_reset_iso(u32 mod_num)
{
	u32 v;
	u32 mdctl;

	/* Set the reset isolation bit */
	mdctl = __raw_readl(KS2_PSC_BASE + PSC_REG_MDCTL(mod_num));
	mdctl = PSC_REG_MDCTL_SET_RESET_ISO(mdctl, 1);
	__raw_writel(mdctl, KS2_PSC_BASE + PSC_REG_MDCTL(mod_num));

	v = __raw_readl(KS2_PSC_BASE + PSC_REG_MDCFG(mod_num));
	if (PSC_REG_MDCFG_GET_RESET_ISO(v) == 1)
		return 0;

	return 1;
}

/**
 * psc_disable_domain() - Disable a power domain
 * @domain_num: GPSC domain number
 */
int psc_disable_domain(u32 domain_num)
{
	u32 pdctl;
	u32 ptcmd;

	pdctl = __raw_readl(KS2_PSC_BASE + PSC_REG_PDCTL(domain_num));
	pdctl = PSC_REG_PDCTL_SET_NEXT(pdctl, PSC_REG_VAL_PDCTL_NEXT_OFF);
	pdctl = PSC_REG_PDCTL_SET_PDMODE(pdctl, PSC_REG_VAL_PDCTL_PDMODE_SLEEP);
	__raw_writel(pdctl, KS2_PSC_BASE + PSC_REG_PDCTL(domain_num));

	ptcmd = __raw_readl(KS2_PSC_BASE + PSC_REG_PTCMD);
	ptcmd |= (u32)(1 << domain_num);
	__raw_writel(ptcmd, KS2_PSC_BASE + PSC_REG_PTCMD);

	return psc_wait(domain_num);
}

/**
 * psc_module_keep_in_reset_enabled() - Keep module in enabled,in-reset state
 * @mod_num:	LPSC module number
 * @gate_clocks: Can the clocks be gated on this module?
 *
 * Enable the module, but do not release the module from local reset. This is
 * necessary for many processor systems on keystone SoCs to allow for system
 * initialization from a master processor prior to releasing the processor
 * from reset.
 */
int psc_module_keep_in_reset_enabled(u32 mod_num, bool gate_clocks)
{
	u32 mdctl, ptcmd, mdstat;
	u32 next_state;
	int domain_num = psc_get_domain_num(mod_num);
	int timeout = 100000;

	/* Wait for any previous transitions to complete */
	psc_wait(domain_num);
	mdctl = __raw_readl(KS2_PSC_BASE + PSC_REG_MDCTL(mod_num));
	/* Should be set 0 to assert Local reset */
	if ((mdctl & PSC_REG_MDCTL_SET_LRSTZ(mdctl, 1))) {
		mdctl = PSC_REG_MDCTL_SET_LRSTZ(mdctl, 0);
		__raw_writel(mdctl, KS2_PSC_BASE + PSC_REG_MDCTL(mod_num));
		/* Wait for transition to take place */
		psc_wait(domain_num);
	}

	/* Clear Module reset */
	mdctl = __raw_readl(KS2_PSC_BASE + PSC_REG_MDCTL(mod_num));
	next_state = gate_clocks ? PSC_REG_VAL_MDCTL_NEXT_OFF :
			PSC_REG_VAL_MDCTL_NEXT_ON;
	mdctl = PSC_REG_MDCTL_SET_NEXT(mdctl, next_state);
	__raw_writel(mdctl, KS2_PSC_BASE + PSC_REG_MDCTL(mod_num));
	/* Trigger PD transition */
	ptcmd = __raw_readl(KS2_PSC_BASE + PSC_REG_PTCMD);
	ptcmd |= (u32)(1 << domain_num);
	__raw_writel(ptcmd, KS2_PSC_BASE + PSC_REG_PTCMD);
	psc_wait(domain_num);

	mdstat = __raw_readl(KS2_PSC_BASE + PSC_REG_MDSTAT(mod_num));
	while (timeout) {
		mdstat = __raw_readl(KS2_PSC_BASE + PSC_REG_MDSTAT(mod_num));

		if (!(PSC_REG_MDSTAT_GET_STATUS(mdstat) & 0x30) &&
		    PSC_REG_MDSTAT_GET_MRSTDONE(mdstat) &&
		    PSC_REG_MDSTAT_GET_LRSTDONE(mdstat))
			break;
		timeout--;
	}

	if (!timeout) {
		printf("%s: Timedout waiting for mdstat(0x%08x) to change\n",
		       __func__, mdstat);
		return -ETIMEDOUT;
	}
	return 0;
}

/**
 * psc_module_release_from_reset() - Release the module from reset
 * @mod_num:	LPSC module number
 *
 * This is the follow through for the command 'psc_module_keep_in_reset_enabled'
 * Allowing the module to be released from reset once all required inits are
 * complete for the module. Typically, this allows the processor module to start
 * execution.
 */
int psc_module_release_from_reset(u32 mod_num)
{
	u32 mdctl, mdstat;
	int domain_num = psc_get_domain_num(mod_num);
	int timeout = 100000;

	/* Wait for any previous transitions to complete */
	psc_wait(domain_num);
	mdctl = __raw_readl(KS2_PSC_BASE + PSC_REG_MDCTL(mod_num));
	/* Should be set to 1 to de-assert Local reset */
	if ((mdctl & PSC_REG_MDCTL_SET_LRSTZ(mdctl, 0))) {
		mdctl = PSC_REG_MDCTL_SET_LRSTZ(mdctl, 1);
		__raw_writel(mdctl, KS2_PSC_BASE + PSC_REG_MDCTL(mod_num));
		/* Wait for transition to take place */
		psc_wait(domain_num);
	}
	mdstat = __raw_readl(KS2_PSC_BASE + PSC_REG_MDSTAT(mod_num));
	while (timeout) {
		mdstat = __raw_readl(KS2_PSC_BASE + PSC_REG_MDSTAT(mod_num));

		if (!(PSC_REG_MDSTAT_GET_STATUS(mdstat) & 0x30) &&
		    PSC_REG_MDSTAT_GET_MRSTDONE(mdstat) &&
		    PSC_REG_MDSTAT_GET_LRSTDONE(mdstat))
			break;
		timeout--;
	}

	if (!timeout) {
		printf("%s: Timedout waiting for mdstat(0x%08x) to change\n",
		       __func__, mdstat);
		return -ETIMEDOUT;
	}

	return 0;
}
