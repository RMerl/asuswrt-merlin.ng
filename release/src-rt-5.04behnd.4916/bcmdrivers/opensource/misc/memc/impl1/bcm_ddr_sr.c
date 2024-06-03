/*
<:copyright-BRCM:2021:GPL/GPL:standard

	 Copyright (c) 2021 Broadcom
	 All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/export.h>
#include <linux/cpu.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <asm/io.h>
#include "bcm_memc.h"

// 63138, 63158, 63178, 47622, 6756, 6765, 63146, 4912, 6813
static unsigned int __iomem
	// PhyControl.*
	*mcr_phyctl_idle_pad_en0,
	*mcr_phyctl_idle_pad_en1,
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963178) || \
	defined(CONFIG_BCM947622) || defined(CONFIG_BCM963146) || \
	defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813) || defined(CONFIG_BCM96846) || \
	defined(CONFIG_BCM96878) || defined(CONFIG_BCM96855) || \
	defined(CONFIG_BCM96858) || defined(CONFIG_BCM96856) || \
	defined(CONFIG_BCM96765)
	// PhyByteLane0Control.*
	*mcr_phybl0_idle_pad_ctl,
	// PhyByteLane1Control.*
	*mcr_phybl1_idle_pad_ctl,
#if defined(CONFIG_BCM96846) || defined(CONFIG_BCM96855) || \
    defined(CONFIG_BCM96858) || defined(CONFIG_BCM96856)
	// PhyByteLane2Control.*
	*mcr_phybl2_idle_pad_ctl,
	// PhyByteLane3Control.*
	*mcr_phybl3_idle_pad_ctl,
#endif
#endif
#if defined(CONFIG_BCM963178) || defined(CONFIG_BCM947622)
	// PhyControl.*
	*mcr_phyctl_clock_idle,
#endif
#if defined(CONFIG_BCM963178) || defined(CONFIG_BCM947622) || \
	defined(CONFIG_BCM963146) || defined(CONFIG_BCM94912) || \
	defined(CONFIG_BCM96813) || defined(CONFIG_BCM96846) || \
	defined(CONFIG_BCM96878) || defined(CONFIG_BCM96855) || \
	defined(CONFIG_BCM96858) || defined(CONFIG_BCM96856) || \
	defined(CONFIG_BCM96765)
	// PhyByteLane0Control.*
	*mcr_phybl0_clock_idle,
	// PhyByteLane1Control.*
	*mcr_phybl1_clock_idle,
#if defined(CONFIG_BCM96846) || defined(CONFIG_BCM96855) || \
	defined(CONFIG_BCM96858) || defined(CONFIG_BCM96856)
	// PhyByteLane2Control.*
	*mcr_phybl2_clock_idle,
	// PhyByteLane3Control.*
	*mcr_phybl3_clock_idle,
#endif
#endif
	*mcr_phyctl_idle_pad_ctl; // PhyControl.*

MODULE_PARM_DESC(enable_self_refresh, "enable DDR self refresh");
static unsigned int enable_self_refresh = 0;

#ifndef CONFIG_BCM963138
// hw auto self refresh
#define MEMC_GLB_GCFG_SREF_SLOW_CLK_SHIFT	26
#define MEMC_GLB_GCFG_SREF_SLOW_CLK_MASK	(1<<MEMC_GLB_GCFG_SREF_SLOW_CLK_SHIFT)
#define MEMC_DDR_AUTO_SELFREFRESH_EN		(1 << 31)
#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178) || \
	defined(CONFIG_BCM947622) || defined(CONFIG_BCM96756) || \
	defined(CONFIG_BCM96846) || defined(CONFIG_BCM96855) || defined(CONFIG_BCM96878) || \
	defined(CONFIG_BCM96858) || defined(CONFIG_BCM96856)
#define MEMC_DDR_AUTO_SR_IDLE_CNT_MASK		(0x7FFFFFFF)
#elif defined(CONFIG_BCM963146) || defined(CONFIG_BCM94912) || \
	defined(CONFIG_BCM96813) || defined(CONFIG_BCM96765)
#define MEMC_DDR_AUTO_SR_IDLE_CNT_MASK		(0x3FFFFFFF)
#endif

static unsigned int mc_clk_mhz; 
static unsigned int __iomem
	*mcr_auto_self_refresh,
	*mcr_glb_gcfg;

MODULE_PARM_DESC(autosr_thresh_us,
	"idle threshold in microsecond before DDR enters self refresh");
static unsigned int autosr_thresh_us = 20;

#define param_check_autosr_thresh_us(atu, p_atu) \
	__param_check(atu, p_atu, unsigned int)

static int
param_set_autosr_thresh_us(const char *val, const struct kernel_param *kp)
{
	unsigned int thresh_new, thresh_cur, thresh_cur_us; 

	if (!mcr_auto_self_refresh) {
		pr_warn("Warning: undefined register auto_self_refresh\n");
		return 0;
	}

	thresh_cur_us = autosr_thresh_us;
	if (kstrtouint(val, 0, kp->arg)) {
		pr_err("Error: invalid parameter '%s' for autosr_thresh_us\n",
				val);
		return -1;
	}

	thresh_new = (autosr_thresh_us * mc_clk_mhz) &
		MEMC_DDR_AUTO_SR_IDLE_CNT_MASK;
        /* threshold needs a min value of 1 (MEMC tick) */
        if (!thresh_new)
            thresh_new = 1 & MEMC_DDR_AUTO_SR_IDLE_CNT_MASK;

	thresh_cur = *mcr_auto_self_refresh & MEMC_DDR_AUTO_SR_IDLE_CNT_MASK;
	if (thresh_new != thresh_cur){
		*mcr_auto_self_refresh &= ~MEMC_DDR_AUTO_SR_IDLE_CNT_MASK;
		*mcr_auto_self_refresh |= thresh_new;
		pr_info("changing DDR auto self refresh idle threshold from "
			"%u us (%u ticks) into %u us (%u ticks)\n",
			thresh_cur_us, thresh_cur, autosr_thresh_us, thresh_new);
	}
	return 0;
}

static int
param_get_autosr_thresh_us(char *buffer, const struct kernel_param *kp)
{
	unsigned int thresh_cur, thresh_cur_us; 

	if (!mcr_auto_self_refresh) {
		pr_warn("Warning: undefined register auto_self_refresh\n");
		return 0;
	}

	thresh_cur = *mcr_auto_self_refresh & MEMC_DDR_AUTO_SR_IDLE_CNT_MASK;
	thresh_cur_us =  thresh_cur / mc_clk_mhz;
	autosr_thresh_us = thresh_cur_us;

	pr_info("DDR auto self refresh idle threshold: %u us (%u ticks)\n",
			thresh_cur_us, thresh_cur);

	return scnprintf(buffer, PAGE_SIZE, "%u\n", *((unsigned int *)kp->arg));
}

static const struct kernel_param_ops param_ops_autosr_thresh_us = {
	.set = param_set_autosr_thresh_us,
	.get = param_get_autosr_thresh_us,
};

module_param(autosr_thresh_us, autosr_thresh_us, 0644);

#else // #ifndef CONFIG_BCM963138
// sw self refresh
#define DRAM_CFG_DRAMSLEEP          (1<<11)
static unsigned int __iomem *mcr_chn_tim_dram_cfg;
static PWRMNGT_DDR_SR_CTRL ddrSrCtrl = {.word = 0};
static volatile PWRMNGT_DDR_SR_CTRL *pDdrSrCtrl = &ddrSrCtrl;

void BcmPwrMngtRegisterLmemAddr(PWRMNGT_DDR_SR_CTRL *pDdrSr)
{
	int cpu;

	// Initialize to busy status
	if (NULL != pDdrSr)
		pDdrSrCtrl = pDdrSr;
	else
		pDdrSrCtrl = &ddrSrCtrl;

	pDdrSrCtrl->word = 0;
	for_each_possible_cpu(cpu)
		pDdrSrCtrl->host |= 1<<cpu;
}
EXPORT_SYMBOL(BcmPwrMngtRegisterLmemAddr);

void bcm_memc_self_refresh_pre_wfi(int cpu_mask)
{
	if (!enable_self_refresh)
		return;

	// On 63138, it was found that accessing the memory assigned by the
	// DSL driver to pDdrSrCtrl is very slow. A local shadow copy
	// ddrSrCtrl is first updated and used before deciding to access
	// pDdrSrCtrl.  On non-DSL chips, the shadow and the pointer are
	// referring to the same memory
	ddrSrCtrl.host &= ~cpu_mask;
	if (!ddrSrCtrl.host) {
		// Let the PHY MIPS know that all cores on the host will be
		// executing wfi
		pDdrSrCtrl->host = 0;
		// Ensure the PHY MIPS is not active so we can enter SR
		if (!pDdrSrCtrl->phy && mcr_chn_tim_dram_cfg)
			*mcr_chn_tim_dram_cfg |= DRAM_CFG_DRAMSLEEP;
	}
}

void bcm_memc_self_refresh_post_wfi(int cpu_mask)
{
	if (enable_self_refresh)
		ddrSrCtrl.host |= cpu_mask;
}
#endif // #ifndef CONFIG_BCM963138

#define param_check_enable_self_refresh(esr, p_esr) \
	__param_check(esr, p_esr, unsigned int)

static int
param_set_enable_self_refresh(const char *val, const struct kernel_param *kp)
{
	unsigned int enable_sr_cur = enable_self_refresh;

	if (kstrtouint(val, 0, kp->arg)) {
		pr_err("Error: invalid parameter '%s' for enable_self_refresh\n",
				val);
		return -1;
	}

	enable_self_refresh = !!enable_self_refresh;
#ifndef CONFIG_BCM963138
	if (!mcr_auto_self_refresh) {
		pr_warn("Warning: undefined register auto_self_refresh\n");
		return 0;
	}

	enable_sr_cur = !!(*mcr_auto_self_refresh &
			MEMC_DDR_AUTO_SELFREFRESH_EN);
#endif

	if (enable_sr_cur != enable_self_refresh) {
#ifndef CONFIG_BCM963138
		if (enable_self_refresh)
			*mcr_auto_self_refresh |= MEMC_DDR_AUTO_SELFREFRESH_EN;
		else
			*mcr_auto_self_refresh &= ~MEMC_DDR_AUTO_SELFREFRESH_EN;

#endif
		pr_debug("changing DDR self refresh into %sabled\n",
			enable_self_refresh ? "en" : "dis");
	}

	return 0;
}							

static int
param_get_enable_self_refresh(char *buffer, const struct kernel_param *kp)
{
#ifndef CONFIG_BCM963138
	if (!mcr_auto_self_refresh) {
		pr_warn("Warning: undefined register auto_self_refresh\n");
		return 0;
	}
	enable_self_refresh = !!(*mcr_auto_self_refresh &
			MEMC_DDR_AUTO_SELFREFRESH_EN);
#endif
	pr_debug("DDR self refresh is %sabled\n",
			enable_self_refresh ? "en" : "dis");
	return scnprintf(buffer, PAGE_SIZE, "%u\n", *((unsigned int *)kp->arg));
}

static const struct kernel_param_ops param_ops_enable_self_refresh = {
	.set = param_set_enable_self_refresh,
	.get = param_get_enable_self_refresh,
};

module_param(enable_self_refresh, enable_self_refresh, 0644);

struct bcm_memc_reg_addr
{
	const char *name;
	struct resource *res;
	unsigned int __iomem **paddr;
};
#define REG_ADDR(reg)				\
	{ .name = #reg, .paddr = &mcr_##reg, }
static int __init bcm_memc_map_sr_reg_addr(struct platform_device *pdev)
{
	int i, ret;
	struct bcm_memc_reg_addr mra[] = {
		REG_ADDR(phyctl_idle_pad_ctl),
		REG_ADDR(phyctl_idle_pad_en0),
		REG_ADDR(phyctl_idle_pad_en1),
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963178) || \
	defined(CONFIG_BCM947622) || defined(CONFIG_BCM963146) || \
	defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813) || defined(CONFIG_BCM96846) || \
	defined(CONFIG_BCM96878) || defined(CONFIG_BCM96855) || \
	defined(CONFIG_BCM96858) || defined(CONFIG_BCM96856) || \
	defined(CONFIG_BCM96765)
		REG_ADDR(phybl0_idle_pad_ctl),
		REG_ADDR(phybl1_idle_pad_ctl),
#if defined(CONFIG_BCM96846) || defined(CONFIG_BCM96855) || \
	defined(CONFIG_BCM96858) || defined(CONFIG_BCM96856)
		REG_ADDR(phybl2_idle_pad_ctl),
		REG_ADDR(phybl3_idle_pad_ctl),
#endif
#endif
#if defined(CONFIG_BCM963178) || defined(CONFIG_BCM947622)
		REG_ADDR(phyctl_clock_idle),
#endif
#if defined(CONFIG_BCM963178) || defined(CONFIG_BCM947622) || \
	defined(CONFIG_BCM963146) || defined(CONFIG_BCM94912) || \
	defined(CONFIG_BCM96813) || defined(CONFIG_BCM96846) || \
	defined(CONFIG_BCM96878) || defined(CONFIG_BCM96855) || \
	defined(CONFIG_BCM96858) || defined(CONFIG_BCM96856) || \
	defined(CONFIG_BCM96765)
		REG_ADDR(phybl0_clock_idle),
		REG_ADDR(phybl1_clock_idle),
#if defined(CONFIG_BCM96846) || defined(CONFIG_BCM96855) || \
	defined(CONFIG_BCM96858) || defined(CONFIG_BCM96856)
		REG_ADDR(phybl2_clock_idle),
		REG_ADDR(phybl3_clock_idle),
#endif
#endif
#ifndef CONFIG_BCM963138
		REG_ADDR(auto_self_refresh),
		REG_ADDR(glb_gcfg),
#else
		REG_ADDR(chn_tim_dram_cfg),
#endif
	};

	for (i = 0; i < ARRAY_SIZE(mra); i++) {
		mra[i].res = platform_get_resource_byname(pdev,
				IORESOURCE_MEM, mra[i].name);
		if (!mra[i].res) {
			pr_err("Error: failed to get reg %s\n", mra[i].name);
			ret = -ENOENT;
			goto err_out;
		}

		*(mra[i].paddr) = ioremap(mra[i].res->start,
				resource_size(mra[i].res));
		if (!*(mra[i].paddr)) {
			pr_err("Error: failed to ioremap reg %s\n", mra[i].name);
			ret = -ENXIO;
			goto err_out;
		}

		pr_info("ioremapped reg %s <0x%llx 0x%llx> to %px\n",
			mra[i].name, (unsigned long long)mra[i].res->start,
			(unsigned long long)resource_size(mra[i].res),
			*(mra[i].paddr));
	}

	return 0;

err_out:
	for (i = 0; i < ARRAY_SIZE(mra) && *(mra[i].paddr); i++) {
		iounmap(*(mra[i].paddr));
		pr_info("iounmapped reg %s <0x%llx 0x%llx> from %px\n",
			mra[i].name, (unsigned long long)mra[i].res->start,
			(unsigned long long)resource_size(mra[i].res),
			*(mra[i].paddr));
		*(mra[i].paddr) = NULL;
	}
	return ret;
}

int __init bcm_memc_init_self_refresh(void *pdev)
{
	if (bcm_memc_map_sr_reg_addr(pdev))
		return -1;

	// PhyControl.*
	*mcr_phyctl_idle_pad_ctl = 0xe;
	*mcr_phyctl_idle_pad_en0 = 0x6df;
	*mcr_phyctl_idle_pad_en1 = 0x3fffff;
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963178) || \
	defined(CONFIG_BCM947622) || defined(CONFIG_BCM963146) || \
	defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813) || defined(CONFIG_BCM96846) || \
	defined(CONFIG_BCM96878) || defined(CONFIG_BCM96855) || \
	defined(CONFIG_BCM96858) || defined(CONFIG_BCM96856)

	// PhyByteLane0Control.*
	*mcr_phybl0_idle_pad_ctl = 0xfffe;
	// PhyByteLane1Control.*
	*mcr_phybl1_idle_pad_ctl = 0xfffe;

#if defined(CONFIG_BCM96846) || defined(CONFIG_BCM96855) || \
	defined(CONFIG_BCM96858) || defined(CONFIG_BCM96856) || \
	defined(CONFIG_BCM96765)
	// PhyByteLane2Control.*
	*mcr_phybl2_idle_pad_ctl = 0xfffe;
	// PhyByteLane3Control.*
	*mcr_phybl3_idle_pad_ctl = 0xfffe;
#endif

#endif
#if defined(CONFIG_BCM963178)
	// PhyControl.*
	*mcr_phyctl_clock_idle = 0x1e;
#elif defined(CONFIG_BCM947622)
	// PhyControl.*
	*mcr_phyctl_clock_idle = 0x1a;
#endif
#if defined(CONFIG_BCM963178) || defined(CONFIG_BCM947622) || \
	defined(CONFIG_BCM963146) || defined(CONFIG_BCM94912) || \
	defined(CONFIG_BCM96813) || defined(CONFIG_BCM96846) || \
	defined(CONFIG_BCM96878) || defined(CONFIG_BCM96855) || \
	defined(CONFIG_BCM96858) || defined(CONFIG_BCM96856) || \
	defined(CONFIG_BCM96765)
	// PhyByteLane0Control.*
	*mcr_phybl0_clock_idle = 0x7;
	// PhyByteLane1Control.*
	*mcr_phybl1_clock_idle = 0x7;

#if defined(CONFIG_BCM96846) || defined(CONFIG_BCM96855) || \
	defined(CONFIG_BCM96858) || defined(CONFIG_BCM96856)
	// PhyByteLane2Control.*
	*mcr_phybl2_clock_idle = 0x7;
	// PhyByteLane3Control.*
	*mcr_phybl3_clock_idle = 0x7;
#endif
#endif

#ifndef CONFIG_BCM963138
	if (bcm_memc_get_spd_mhz(&mc_clk_mhz))
		mc_clk_mhz = 533;

	// set up number of mclk cycles the memc has to be idle before it
	// automatically enters self-refresh, if enabled; and enable MEMC Slow
	// Clock when DDR Self-Refresh is activated
	*mcr_auto_self_refresh &= ~MEMC_DDR_AUTO_SR_IDLE_CNT_MASK;
	*mcr_auto_self_refresh |= (autosr_thresh_us * mc_clk_mhz) &
			MEMC_DDR_AUTO_SR_IDLE_CNT_MASK;
	*mcr_glb_gcfg |= MEMC_GLB_GCFG_SREF_SLOW_CLK_MASK;
#endif // #ifndef CONFIG_BCM963138

	return 0;
}
