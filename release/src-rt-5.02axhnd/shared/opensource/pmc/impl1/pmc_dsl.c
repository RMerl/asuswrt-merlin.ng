/*
<:copyright-BRCM:2013:DUAL/GPL:standard 

   Copyright (c) 2013 Broadcom 
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
#ifndef _CFE_
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#else
#include "lib_types.h"
#include "lib_printf.h"
#include "cfe_timer.h"
#define printk printf
#endif
#include "shared_utils.h"
#include "bcm_ubus4.h"

// FIXME!! the following defines are for debugging and verification purpose,
// once the module has been verified, they and the codes should be removed
//#define CONFIG_DSLLMEM_TEST
//#define CONFIG_DQM_TEST

#ifdef CONFIG_DSLLMEM_TEST
#include "bcm_map_part.h"
#endif
#include "pmc_drv.h"
#include "pmc_dsl.h"
#include "BPCM.h"
#include "pmc_wan.h"

int pmc_dsl_power_up(void)
{
	int ret;
	PLL_CTRL_REG pll_ctrl;
	PLL_LOOP1_REG pll_loop1;
#if defined(_BCM963381_) || defined(CONFIG_BCM963381)
	PLL_LOOP0_REG pll_loop0;
	PLL_CFG0_REG pll_cfg0;
	PLL_CFG1_REG pll_cfg1;
	BPCM_MISC_CONTROL misc_ctrl;
#endif

#if defined(CONFIG_DQM_TEST)
	int iii, jjj;
	uint32 val;
	for (iii = 0; iii < 3; iii++) {
		for (jjj = 0; jjj < 4; jjj++) {
		ret = ReadZoneRegister(PMB_ADDR_VDSL3_CORE, iii, jjj, &val);
			printk("%s:%d:dev_id=%d, Zone#%d, reg_id = %d, val = 0x%08lx, ret = %d\n",
				__func__, __LINE__, PMB_ADDR_VDSL3_CORE, iii, jjj, val, ret);
		}
	}
#endif

#if 0
	/* the following code was included in dv code to configure PLL based on speed */
	PLL_CHCFG_REG ch01_cfg, ch45_cfg;
	PLL_NDIV_REG pll_ndiv;
	if (vdsl_clk == 480MHz) {
		/* ch01_cfg default is 0x04040404 */
		ch01_cfg.Reg32 = 0;
		ch01_cfg.Bits.mdiv0 = 0x5;
		ch01_cfg.Bits.load_en_ch0 = 1;
		ch01_cfg.Bits.mdiv_override0 = 1;
		ch01_cfg.Bits.mdiv2 = 0x5;
		ch01_cfg.Bits.load_en_ch1 = 1;
		ch01_cfg.Bits.mdiv_override1 = 1;
		ret = WriteBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE,
				PLLBPCMRegOffset(ch01_cfg), ch01_cfg.Reg32);
		if (ret)
			return ret;
	} else if (vdsl_clk == 533MHz) {
		/* ndiv default is 48 */
		pll_ndiv.Reg32 = 64 | (1 << 31);

		/* ch01_cfg default is 0x04040404 */
		ch01_cfg.Reg32 = 0;
		ch01_cfg.Bits.mdiv0 = 0x6;
		ch01_cfg.Bits.load_en_ch0 = 1;
		ch01_cfg.Bits.mdiv_override0 = 1;
		ch01_cfg.Bits.mdiv2 = 0x6;
		ch01_cfg.Bits.load_en_ch1 = 1;
		ch01_cfg.Bits.mdiv_override1 = 1;

		/* ch45_cfg default is 0x04030500 */
		ch45_cfg.Reg32 = 0;
		ch45_cfg.Bits.enableb_ch0 = 1;
		ch45_cfg.Bits.load_en_ch0 = 1;
		ch45_cfg.Bits.mdiv2 = 0x4;
		ch45_cfg.Bits.load_en_ch1 = 1;
		ch45_cfg.Bits.mdiv_override1 = 1;

		ret = WriteBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE,
				PLLBPCMRegOffset(ndiv), pll_ndiv.Reg32);
		if (ret)
			return ret;

		ret = WriteBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE,
				PLLBPCMRegOffset(ch01_cfg), ch01_cfg.Reg32);
		if (ret)
			return ret;

		ret = WriteBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE,
				PLLBPCMRegOffset(ch45_cfg), ch45_cfg.Reg32);
		if (ret)
			return ret;
	}

#endif

#if defined(_BCM963381_) || defined(CONFIG_BCM963381)
	/* assert AFE global power down */ 
	ret = ReadBPCMRegister(PMB_ADDR_VDSL3_CORE, 
			BPCMVDSLRegOffset(misc_control), &misc_ctrl.Reg32);
	if (ret)
		return ret;
	misc_ctrl.Bits_vdsl_phy.ctl |= (0x1 << 31);
	ret = WriteBPCMRegister(PMB_ADDR_VDSL3_CORE,
			BPCMVDSLRegOffset(misc_control), misc_ctrl.Reg32);
	if (ret)
		return ret;

	/* 63381 A1/A0 requires some special tuning for AFE pll to lock */
	if( UtilGetChipRev() == 0xA0 || UtilGetChipRev() == 0xA1 )
	{
		printk("Applying DSL AFEPLL tuning for 63381 A0/A1 chip\n");
		/* offset 5 cfg[0], turn on dco bias boot bit */
		ret = ReadBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE,
			PLLBPCMRegOffset(cfg0), &pll_cfg0.Reg32);
		if (ret)
			return ret;
		pll_cfg0.Bits.dco_bias_boost = 0x1;
		pll_cfg0.Bits.fdco_ctrl_bypass |= 0x1;
		ret = WriteBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE,
			PLLBPCMRegOffset(cfg0), pll_cfg0.Reg32);
		if (ret)
			return ret;	

		/* offset 6 cfg[1], modify port reset mode and pwm_ctrl */
		ret = ReadBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE,
			PLLBPCMRegOffset(cfg1), &pll_cfg1.Reg32);
		if (ret)
			return ret;
		pll_cfg1.Bits.port_reset_mode = 2;
		ret = WriteBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE,
			PLLBPCMRegOffset(cfg1), pll_cfg1.Reg32);
		if (ret)
			return ret;		

		/* set the ki/kp registers to 3/5. */
		ret = ReadBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE,
			PLLBPCMRegOffset(loop0), &pll_loop0.Reg32);
		if (ret)
			return ret;	
		pll_loop0.Bits.ki = 3;
		pll_loop0.Bits.kp = 5;
		ret = WriteBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE,
			PLLBPCMRegOffset(loop0), pll_loop0.Reg32);
		if (ret)
			return ret;
	}
#endif
#ifdef CONFIG_BRCM_IKOS
	printk("%s: start the AFE PLL\n", __FUNCTION__);
#endif
	/* 1) start the AFE PLL */
	ret = ReadBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE,
			PLLBPCMRegOffset(resets), &pll_ctrl.Reg32);
	if (ret)
		return ret;
	pll_ctrl.Bits.resetb = 1;
#if !defined(_BCM963158_) && !defined(CONFIG_BCM963158)
	pll_ctrl.Bits.post_resetb = 1;
#endif
	ret = WriteBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE,
			PLLBPCMRegOffset(resets), pll_ctrl.Reg32);
	if (ret)
		return ret;

	do {
		ret = ReadBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE,
				PLLBPCMRegOffset(loop1), &pll_loop1.Reg32);
		if (ret)
			return ret;
	}
#ifdef CONFIG_BRCM_IKOS
	while (0);
	printk("%s: Skip waiting for AFE PLL lock(pll_loop1.Bits.ssc_mode=%d)\n", __FUNCTION__, pll_loop1.Bits.ssc_mode);
#else
	while (!(pll_loop1.Bits.ssc_mode));
#endif

#if defined(_BCM963158_) || defined(CONFIG_BCM963158)
	ret = ReadBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE,
			PLLBPCMRegOffset(resets), &pll_ctrl.Reg32);
	if (ret)
		return ret;
	pll_ctrl.Bits.post_resetb = 1;
	ret = WriteBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE,
				PLLBPCMRegOffset(resets), pll_ctrl.Reg32);
	if (ret)
		return ret;
#endif

#if defined(_BCM963381_) || defined(CONFIG_BCM963381)
	/* de-assert AFE global power down */ 
	ret = ReadBPCMRegister(PMB_ADDR_VDSL3_CORE, 
			BPCMVDSLRegOffset(misc_control), &misc_ctrl.Reg32);
	if (ret)
		return ret;
	misc_ctrl.Bits_vdsl_phy.ctl &= ~(0x1 << 31);
	ret = WriteBPCMRegister(PMB_ADDR_VDSL3_CORE,
			BPCMVDSLRegOffset(misc_control), misc_ctrl.Reg32);
	if (ret)
		return ret;
#endif

#if defined(_BCM963158_) || defined(CONFIG_BCM963158)
	/* 2) AFE is locked, commence PowerOn */
	/* 63158 VDSL CORE and MIPS are powered on by default. 
	   We just need to power on PMD */
#ifdef CONFIG_BRCM_IKOS
	ret = 0;
	printk("%s: Skip calling PowerOnDevice(PMB_ADDR_VDSL3_PMD)\n", __FUNCTION__);
#else
	ret = PowerOnDevice(PMB_ADDR_VDSL3_PMD);
#endif
#else
	/* 2) AFE is locked, commence PowerOn */
	ret = PowerOnDevice(PMB_ADDR_VDSL3_CORE);
#endif
	if (ret)
		return ret;

#if !defined(_BCM963158_) && !defined(CONFIG_BCM963158)
	/* 3) set the dsl clock */
	ret = pmc_dsl_clock_set(1);
	if (ret)
		return ret;

	/* 4) power on  PHY MIPS */
	ret = PowerOnDevice(PMB_ADDR_VDSL3_MIPS);
#endif

#ifndef CONFIG_BRCM_IKOS
#if defined(_CFE_) 
	cfe_usleep(100);
#else
	udelay(100);
#endif
#endif /* CONFIG_BRCM_IKOS */

#if defined(CONFIG_BCM963158)
	apply_ubus_credit_each_master(UBUS_PORT_ID_DSLCPU);
	apply_ubus_credit_each_master(UBUS_PORT_ID_DSL);
#endif

#if defined(CONFIG_BCM_UBUS4_DCM)
    ubus_cong_threshold_wr(UBUS_PORT_ID_DSLCPU, 0);
    ubus_cong_threshold_wr(UBUS_PORT_ID_DSL, 0);
#endif

	return ret;
}

int pmc_dsl_power_down(void)
{
#if defined(CONFIG_BCM963158)
	pmc_wan_interface_power_control(WAN_INTF_XDSL, 0);
#endif
	return 0;
}

int pmc_dsl_clock_set(int flag)
{
	int ret;
	BPCM_SR_CONTROL sr_ctrl;
	BPCM_MISC_CONTROL misc_ctrl;

	ret = ReadBPCMRegister(PMB_ADDR_VDSL3_CORE,
			BPCMVDSLRegOffset(sr_control), &sr_ctrl.Reg32);
	if (ret)
		return ret;
	sr_ctrl.Bits.sr |= 0x1;
	sr_ctrl.Bits.gp = 0xffffff;

	ret = WriteBPCMRegister(PMB_ADDR_VDSL3_CORE,
			BPCMVDSLRegOffset(sr_control), sr_ctrl.Reg32);
	if (ret)
		return ret;

	ret = ReadBPCMRegister(PMB_ADDR_VDSL3_CORE, 
			BPCMVDSLRegOffset(misc_control), &misc_ctrl.Reg32);
	if (ret)
		return ret;

	if (flag)
		misc_ctrl.Bits_vdsl_phy.ctl |= 0x3;
	else
		misc_ctrl.Bits_vdsl_phy.ctl &= ~0x3;

	ret = WriteBPCMRegister(PMB_ADDR_VDSL3_CORE,
			BPCMVDSLRegOffset(misc_control), misc_ctrl.Reg32);
	if (ret)
		return ret;

	ret = ReadBPCMRegister(PMB_ADDR_VDSL3_CORE,
			BPCMVDSLRegOffset(sr_control), &sr_ctrl.Reg32);
	if (ret)
		return ret;
	sr_ctrl.Bits.sr |= 0x3;
	sr_ctrl.Bits.gp = 0xffffff;

	ret = WriteBPCMRegister(PMB_ADDR_VDSL3_CORE,
			BPCMVDSLRegOffset(sr_control), sr_ctrl.Reg32);
	return ret;
}

int pmc_dsl_mipscore_enable(int flag, int core)
{
	int ret;
	BPCM_MISC_CONTROL misc_ctrl;
	int misc_offset = BPCMVDSLRegOffset(misc_control);


#if defined(_BCM963138_) || defined(CONFIG_BCM963138)
        int val;

        val = 0xa040001;
        WriteBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE, BPCMRegOffset(rosc_thresh_h), val);
        val = 0x300;
        WriteBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE, BPCMRegOffset(rosc_thresh_s), val);
        val = 0x6400;
        WriteBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE, BPCMRegOffset(pwd_accum_control), val);

	/* 63138 B0 has two PHY MIPS core, the misc_ctrl for second core is next to the first one,
	   the Bits_vdsl_mips definition is same in both control and control2 structure so we can
	   just share the same misc_ctrl variable */
	if ((UtilGetChipRev() != 0xA0) && core )
		misc_offset = BPCMVDSLRegOffset(misc_control2);
#endif

#if defined(_BCM963158_) || defined(CONFIG_BCM963158)
	if( core ) {
		printk("pmc_dsl_mipscore_enable not supported for secondary mips thread, flag %d core %d\n", flag, core);
		return 1;
	}
#endif

	/* perform individual read/write of the reset bits per vdsl team suggestion */
	if (flag) {
#if defined(_BCM963381_) || defined(CONFIG_BCM963381)
		/* 63381 B0 or later chip have additional bits to control phy mips reset */
		if ( UtilGetChipRev() != 0xA0 && UtilGetChipRev() != 0xA1 )
		{
			ret = ReadBPCMRegister(PMB_ADDR_VDSL3_MIPS,
				misc_offset, &misc_ctrl.Reg32);
			if (ret)
				return ret;
			misc_ctrl.Bits_vdsl_mips.ext_mclk_en = 1;	/* 1 << 27 */
			ret = WriteBPCMRegister(PMB_ADDR_VDSL3_MIPS,
				misc_offset, misc_ctrl.Reg32);
			if (ret)
				return ret;

			ret = ReadBPCMRegister(PMB_ADDR_VDSL3_MIPS,
				misc_offset, &misc_ctrl.Reg32);
			if (ret)
				return ret;
			misc_ctrl.Bits_vdsl_mips.ext_mclk_en_reset = 1;	/* 1 << 26 */
			ret = WriteBPCMRegister(PMB_ADDR_VDSL3_MIPS,
				misc_offset, misc_ctrl.Reg32);
			if (ret)
				return ret;
		}
#endif

		/* de-assert mips resets */
		ret = ReadBPCMRegister(PMB_ADDR_VDSL3_MIPS,
			misc_offset, &misc_ctrl.Reg32);
		if (ret)
			return ret;
		misc_ctrl.Bits_vdsl_mips.por_reset_n_ctl = 1;	/* 1 << 28 */
		ret = WriteBPCMRegister(PMB_ADDR_VDSL3_MIPS,
			misc_offset, misc_ctrl.Reg32);
		if (ret)
			return ret;

		ret = ReadBPCMRegister(PMB_ADDR_VDSL3_MIPS,
			misc_offset, &misc_ctrl.Reg32);
		if (ret)
			return ret;
		misc_ctrl.Bits_vdsl_mips.reset_n_ctl = 1;	/* 1 << 29 */
		ret = WriteBPCMRegister(PMB_ADDR_VDSL3_MIPS,
			misc_offset, misc_ctrl.Reg32);
		if (ret)
			return ret;

	} else {
		/* assert mips resets */
		ret = ReadBPCMRegister(PMB_ADDR_VDSL3_MIPS,
			misc_offset, &misc_ctrl.Reg32);
		if (ret)
			return ret;
		misc_ctrl.Bits_vdsl_mips.reset_n_ctl = 0;	/* 1 << 29 */
		ret = WriteBPCMRegister(PMB_ADDR_VDSL3_MIPS,
			misc_offset, misc_ctrl.Reg32);
		if (ret)
			return ret;

		ret = ReadBPCMRegister(PMB_ADDR_VDSL3_MIPS,
			misc_offset, &misc_ctrl.Reg32);
		if (ret)
			return ret;
		misc_ctrl.Bits_vdsl_mips.por_reset_n_ctl = 0;	/* 1 << 28 */
		ret = WriteBPCMRegister(PMB_ADDR_VDSL3_MIPS,
			misc_offset, misc_ctrl.Reg32);
		if (ret)
			return ret;

#if defined(_BCM963381_) || defined(CONFIG_BCM963381)
		/* 63381 B0 or later chip have additional bits to control phy mips reset */
		if( UtilGetChipRev() != 0xA0 && UtilGetChipRev() != 0xA1 )
		{
			ret = ReadBPCMRegister(PMB_ADDR_VDSL3_MIPS,
				misc_offset, &misc_ctrl.Reg32);
			if (ret)
				return ret;
			misc_ctrl.Bits_vdsl_mips.ext_mclk_en = 0;	/* 1 << 27 */
			ret = WriteBPCMRegister(PMB_ADDR_VDSL3_MIPS,
				misc_offset, misc_ctrl.Reg32);
			if (ret)
				return ret;

			ret = ReadBPCMRegister(PMB_ADDR_VDSL3_MIPS,
				misc_offset, &misc_ctrl.Reg32);
			if (ret)
				return ret;
			misc_ctrl.Bits_vdsl_mips.ext_mclk_en_reset = 0;	/* 1 << 26 */
			ret = WriteBPCMRegister(PMB_ADDR_VDSL3_MIPS,
				misc_offset, misc_ctrl.Reg32);
			if (ret)
				return ret;
		}
#endif

	}

	return ret;
}

/* keep the old API for compatiblity and enable/disable the first PHY MIPS core */
int pmc_dsl_mips_enable(int flag)
{
	return pmc_dsl_mipscore_enable(flag, 0);
}

int pmc_dsl_core_reset(void)
{
	int ret;
	BPCM_SR_CONTROL sr_ctrl;

	ret = ReadBPCMRegister(PMB_ADDR_VDSL3_CORE,
			BPCMVDSLRegOffset(sr_control), &sr_ctrl.Reg32);
	if (ret)
		return ret;

	/* assert VDSL core reset */
#if defined(_BCM963158_) || defined(CONFIG_BCM963158)
	sr_ctrl.Bits.sr |= 0x3;
#else
	sr_ctrl.Bits.sr &= ~0x3;
#endif
	sr_ctrl.Bits.gp = 0xffffff;

	ret = WriteBPCMRegister(PMB_ADDR_VDSL3_CORE,
			BPCMVDSLRegOffset(sr_control), sr_ctrl.Reg32);
	if (ret)
		return ret;

	ret = ReadBPCMRegister(PMB_ADDR_VDSL3_CORE,
			BPCMVDSLRegOffset(sr_control), &sr_ctrl.Reg32);
	if (ret)
		return ret;

	/* de-assert VDSL core reset */
#if defined(_BCM963158_) || defined(CONFIG_BCM963158)
	sr_ctrl.Bits.sr &= ~0x3;
#else
	sr_ctrl.Bits.sr |= 0x3;
#endif
	sr_ctrl.Bits.gp = 0xffffff;

	ret = WriteBPCMRegister(PMB_ADDR_VDSL3_CORE,
			BPCMVDSLRegOffset(sr_control), sr_ctrl.Reg32);
	return ret;
}

#ifdef CONFIG_DSLLMEM_TEST
#define DSL_LMEM	((volatile uint32 *)(DSLLMEM_BASE))
static int dsl_lmem_test(void)
{
	int i;
	printk("%s:%d:start test!\n", __func__, __LINE__);

	printk("%s:%d:start writing to 0x%08lx\n", __func__, __LINE__, (unsigned long)DSLLMEM_BASE);
	for (i = 0; i < 20; i++)
		DSL_LMEM[i] = 0xdeadbeef + i;

	for (i = 80; i < 100; i++)
		DSL_LMEM[i] = 0xdeadbeef + i;

	for (i = 8000; i < 8100; i++)
		DSL_LMEM[i] = 0xdeadbeef + i;

	for (i = 10000; i < 11000; i++)
		DSL_LMEM[i] = 0xdeadbeef + i;

	printk("%s:%d:start reading and comparing\n", __func__, __LINE__);
	for (i = 0; i < 20; i++) {
		if (DSL_LMEM[i] != (0xdeadbeef + i))
			printk("%s:%d:DSL_LMEM[%d] = 0x%08lx != 0x%08lx\n",
					__func__, __LINE__, i,
					(unsigned long)DSL_LMEM[i],
					(unsigned long)(0xdeadbeef + i));
	}
	for (i = 80; i < 100; i++) {
		if (DSL_LMEM[i] != (0xdeadbeef + i))
			printk("%s:%d:DSL_LMEM[%d] = 0x%08lx != 0x%08lx\n",
					__func__, __LINE__, i,
					(unsigned long)DSL_LMEM[i],
					(unsigned long)(0xdeadbeef + i));
	}
	for (i = 8000; i < 8100; i++) {
		if (DSL_LMEM[i] != (0xdeadbeef + i))
			printk("%s:%d:DSL_LMEM[%d] = 0x%08lx != 0x%08lx\n",
					__func__, __LINE__, i,
					(unsigned long)DSL_LMEM[i],
					(unsigned long)(0xdeadbeef + i));
	}
	for (i = 10000; i < 11000; i++) {
		if (DSL_LMEM[i] != (0xdeadbeef + i))
			printk("%s:%d:DSL_LMEM[%d] = 0x%08lx != 0x%08lx\n",
					__func__, __LINE__, i,
					(unsigned long)DSL_LMEM[i],
					(unsigned long)(0xdeadbeef + i));
	}
	printk("%s:%d:done test\n", __func__, __LINE__);
	return 0;
}
#endif

#ifndef _CFE_
EXPORT_SYMBOL(pmc_dsl_power_up);
EXPORT_SYMBOL(pmc_dsl_power_down);
EXPORT_SYMBOL(pmc_dsl_clock_set);
EXPORT_SYMBOL(pmc_dsl_mips_enable);
EXPORT_SYMBOL(pmc_dsl_mipscore_enable);
EXPORT_SYMBOL(pmc_dsl_core_reset);
int pmc_dsl_init(void)
{
	// FIXME!! disable it for now, once we have DSL implementation
	// ready, then we go ahead and enable this
#ifdef CONFIG_DSLLMEM_TEST
	int ret;

	ret = pmc_dsl_power_up();
	if (ret != 0)
		printk("%s:%d:initialization fails! ret = %d\n", __func__, __LINE__, ret);

	dsl_lmem_test();
#endif
	return 0;
}
arch_initcall(pmc_dsl_init);
#endif

