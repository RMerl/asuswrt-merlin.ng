/*
<:copyright-BRCM:2018:DUAL/GPL:standard 

   Copyright (c) 2018 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:> 
*/
#include <arch_helpers.h>
#include <assert.h>
#include <debug.h>
#include <gicv2.h>
#include <platform_def.h>
#include <platform.h>
#include <psci.h>
#include <bl31.h>
#include <io.h>
#include <delay_timer.h>
#include <pmc_drv_special.h>

extern int pmc_cpu_core_power_up(int cpu);
extern int pmc_cpu_core_power_down(int cpu);
extern void pmc_initmode(void);
extern void udelay(uint32_t);
extern void sp_min_warm_entrypoint(void);
extern void gicv2_pcpu_distif_init(void);
extern void gicv2_cpuif_enable(void);
extern void a9_gic_secure_init(void);

/*
 * set specified CPU start address in the BIUCFG address space.
 */
#if defined (PLATFORM_FLAVOR_6858) && defined(BIUCFG_BASE)
#define BIUCFG_EP_LO(i)	((void*)BIUCFG_BASE + 0x120 + (i) * 4)
#else
#define BIUCFG_EP_LO(i)	((void*)BIUCFG_BASE + 0x120 + (i) * 8)
#define BIUCFG_EP_HI(i)	((void*)BIUCFG_BASE + 0x124 + (i) * 8)
#endif

static void biu_set_cpu_ep(int cpu, long ep)
{
#if defined(BIUCFG_BASE)
#if defined (PLATFORM_FLAVOR_6858)
	writel(((u32)ep) >> 8, BIUCFG_EP_LO(cpu));
#else
	writeq(ep, BIUCFG_EP_LO(cpu));
#endif
#elif defined(BIUCTRL_BASE)
	BIUCTRL->power_cfg |= 1 << (cpu + BIU_CPU_CTRL_PWR_CFG_CPU0_BPCM_INIT_ON_SHIFT);
	BIUCTRL->reset_cfg &= ~(0x1 << cpu);
#endif

#if (!defined (PLATFORM_FLAVOR_6858) && !defined (PLATFORM_FLAVOR_6856))
#if defined(BOOT_LUT) 
	BOOT_LUT->bootLutRst = (u32)ep;
#endif

#if defined(BOOTLUT_BASE)
	writel((u32)ep, (u32*)(BOOTLUT_BASE+0x20));
#endif
#endif
}

/*******************************************************************************
 * Platform handler called to check the validity of the power state
 * parameter. The power state parameter has to be a composite power state.
 ******************************************************************************/
static int brcm_validate_power_state(unsigned int power_state,
				psci_power_state_t *req_state)
{
	return PSCI_E_SUCCESS; 
}

/*******************************************************************************
 * Platform handler called to check the validity of the non secure
 * entrypoint.
 ******************************************************************************/
static int brcm_validate_ns_entrypoint(uintptr_t entrypoint)
{
	/*
	 * Check if the non secure entrypoint lies within the non
	 * secure DRAM.
	 */
    return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * Platform handler called when a CPU is about to enter standby.
 ******************************************************************************/
static void brcm_cpu_standby(plat_local_state_t cpu_state)
{

	assert(cpu_state == PLAT_LOCAL_STATE_RET);

	/*
	 * Enter standby state
	 * dsb is good practice before using wfi to enter low power states
	 */
	dsb();
	wfi();
}

/*******************************************************************************
 * Platform handler called when a power domain is about to be turned on. The
 * mpidr determines the CPU to be turned on.
 ******************************************************************************/
static int brcm_pwr_domain_on(u_register_t mpidr)
{
	int rc = PSCI_E_SUCCESS;

	/*
	 * here it is assumed that CFE didn't start the CPUs. Set the
	 * CPU entry point and power the CPU up.
	 */
#ifdef AARCH32
	biu_set_cpu_ep(mpidr & 0xff, (long)sp_min_warm_entrypoint);
#else
	biu_set_cpu_ep(mpidr & 0xff, (long)bl31_warm_entrypoint);
#endif
	if (pmc_cpu_core_power_up(mpidr & 0xff) < 0)
		rc = PSCI_E_INTERN_FAIL;
	return rc;
}

/*******************************************************************************
 * Platform handler called when a power domain is about to be turned off. The
 * target_state encodes the power state that each level should transition to.
 ******************************************************************************/
void brcm_pwr_domain_off(const psci_power_state_t *target_state)
{
#if defined (PLATFORM_FLAVOR_63138)
	assert(0);
#else
	gicv2_cpuif_disable();
#endif
}

/*******************************************************************************
 * Platform handler called when a power domain is about to be suspended. The
 * target_state encodes the power state that each level should transition to.
 ******************************************************************************/
void brcm_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	assert(0);
}

/*******************************************************************************
 * Platform handler called when a power domain has just been powered on after
 * being turned off earlier. The target_state encodes the low power state that
 * each level has woken up from.
 ******************************************************************************/
void brcm_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	assert(target_state->pwr_domain_state[MPIDR_AFFLVL0] ==
					PLAT_LOCAL_STATE_OFF);

#if defined (PLATFORM_FLAVOR_63138)
	a9_gic_secure_init();
#else
	/* TODO: This setup is needed only after a cold boot */
	gicv2_pcpu_distif_init();

	/* Enable the gic cpu interface */
	gicv2_cpuif_enable();
#endif
}

/*******************************************************************************
 * Platform handler called when a power domain has just been powered on after
 * having been suspended earlier. The target_state encodes the low power state
 * that each level has woken up from.
 ******************************************************************************/
void brcm_pwr_domain_suspend_finish(const psci_power_state_t *target_state)
{
	assert(0);
}

/*******************************************************************************
 * Platform handler called when a power domain is about to be turned off. The
 * target_state encodes the power state that each level should transition to.
 ******************************************************************************/
__dead2 void brcm_pwr_domain_pwr_down_wfi(const psci_power_state_t *target_state)
{
	int idx = plat_my_core_pos();

	pmc_cpu_core_power_down(idx);
	psci_power_down_wfi();

	panic();
}

/*******************************************************************************
 * Platform handlers to shutdown/reboot the system
 ******************************************************************************/
static void __dead2 brcm_system_off(void)
{
	ERROR("BRCM System Off: operation not handled.\n");
	panic();
}

static void __dead2 brcm_system_reset(void)
{
#if defined(WDTIMR0_BASE)
	INFO("performing system reset from PSCI.\n");

#if defined(AARCH32) && ! defined(PLATFORM_FLAVOR_6846)
	WDTIMER0->WDTimerCtl = SOFT_RESET;
#else
	WDTIMER0->SoftRst = SOFT_RESET;
#endif
	/*
	 * delay 1 second before assuming reset has failed.
	 */
	udelay(1000 * 1000);
	ERROR("watchdog system reset failed.");
	panic();
#elif defined(TIMR_BASE)
	((volatile Timer * const) (TIMR_BASE + TIMR_OFFSET))->SoftRst |=  SOFT_RESET;
#endif
	while(1);
}

static const plat_psci_ops_t plat_brcm_psci_pm_ops = {
	.cpu_standby = brcm_cpu_standby,
	.pwr_domain_on = brcm_pwr_domain_on,
	.pwr_domain_off = brcm_pwr_domain_off,
	.pwr_domain_suspend = brcm_pwr_domain_suspend,
	.pwr_domain_on_finish = brcm_pwr_domain_on_finish,
	.pwr_domain_suspend_finish = brcm_pwr_domain_suspend_finish,
	.pwr_domain_pwr_down_wfi = brcm_pwr_domain_pwr_down_wfi,
	.system_off = brcm_system_off,
	.system_reset = brcm_system_reset,
	.validate_power_state = brcm_validate_power_state,
	.validate_ns_entrypoint = brcm_validate_ns_entrypoint
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
#if defined(PLATFORM_FLAVOR_63138) || defined (PMB_ADDR_ORION_CPU0)
	pmc_initmode();
#endif

	*psci_ops = &plat_brcm_psci_pm_ops;

	return 0;
}
