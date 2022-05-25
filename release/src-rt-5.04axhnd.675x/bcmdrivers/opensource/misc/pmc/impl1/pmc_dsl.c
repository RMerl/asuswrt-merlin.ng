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
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
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

#if !IS_BCMCHIP(63178)
static int pmc_wait_afe_pll_lock(void)
{
	int ret = 0;
#if IS_BCMCHIP(63146)
#define IS_AFE_PLL_LOCKED(reg) ((reg).Bits.lock)
	PLL_STAT_REG reg;
	int reg_offset = AFEPLLBPCMRegOffset(stat);
#else
#define IS_AFE_PLL_LOCKED(reg) ((reg).Bits.ssc_mode)
	PLL_LOOP1_REG reg;
	int reg_offset = PLLBPCMRegOffset(loop1);
#endif
	do {
		ret = ReadBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE,
				reg_offset, &reg.Reg32);
		if (ret)
			return ret;
	}
#ifdef CONFIG_BRCM_IKOS
	while (0); /* no AFE PLL in IKOS, skip the waiting */
#else
	while (!IS_AFE_PLL_LOCKED(reg));
#endif
	return ret;

}
#endif

int g_powered_up = 0;

int pmc_dsl_power_up(void)
{
	int ret = 0;
#if !IS_BCMCHIP(63178)
	PLL_CTRL_REG pll_ctrl;
#if IS_BCMCHIP(63146)
	int resets_offset = AFEPLLBPCMRegOffset(resets);	
#else
	int resets_offset = PLLBPCMRegOffset(resets);	
#endif
#endif

#if defined(CONFIG_DQM_TEST)
	int iii, jjj;
	uint32_t val;
	for (iii = 0; iii < 3; iii++) {
		for (jjj = 0; jjj < 4; jjj++) {
		ret = ReadZoneRegister(PMB_ADDR_VDSL3_CORE, iii, jjj, &val);
			printk("%s:%d:dev_id=%d, Zone#%d, reg_id = %d, val = 0x%08lx, ret = %d\n",
				__func__, __LINE__, PMB_ADDR_VDSL3_CORE, iii, jjj, val, ret);
		}
	}
#endif

	if (g_powered_up)
		return ret;

#ifdef CONFIG_BRCM_IKOS
	printk("%s: start the AFE PLL\n", __FUNCTION__);
#endif

#if !IS_BCMCHIP(63178)
	/* 1) start the AFE PLL */
	ret = ReadBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE,
			resets_offset, &pll_ctrl.Reg32);
	if (ret)
		return ret;
	pll_ctrl.Bits.resetb = 1;
#if !IS_BCMCHIP(63158) && !IS_BCMCHIP(63146)
	pll_ctrl.Bits.post_resetb = 1;
#endif
	ret = WriteBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE,
			resets_offset, pll_ctrl.Reg32);
	if (ret)
		return ret;

	if ((ret = pmc_wait_afe_pll_lock()))
		return ret;

#if IS_BCMCHIP(63158) || IS_BCMCHIP(63146)
	ret = ReadBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE,
			resets_offset, &pll_ctrl.Reg32);
	if (ret)
		return ret;
	pll_ctrl.Bits.post_resetb = 1;
	ret = WriteBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE,
			resets_offset, pll_ctrl.Reg32);
	if (ret)
		return ret;
#endif
#endif

	/* 2) AFE is locked, commence PowerOn */
#if IS_BCMCHIP(63158) || IS_BCMCHIP(63146)
	ret |= PowerOnDevice(PMB_ADDR_VDSL3_PMD);
#endif
	ret |= PowerOnDevice(PMB_ADDR_VDSL3_CORE);
	if (ret)
		return ret;

#if IS_BCMCHIP(63138) || IS_BCMCHIP(63148)
	/* 3) set the dsl clock */
	ret = pmc_dsl_clock_set(1);
	if (ret)
		return ret;
#endif
#if IS_BCMCHIP(63138) || IS_BCMCHIP(63148) || IS_BCMCHIP(63158)
	ret = PowerOnDevice(PMB_ADDR_VDSL3_MIPS);
#endif
	if (ret)
		return ret;

#ifndef CONFIG_BRCM_IKOS
	udelay(100);
#endif /* CONFIG_BRCM_IKOS */

#if IS_BCMCHIP(63158)
	apply_ubus_credit_each_master(UBUS_PORT_ID_DSLCPU);
	apply_ubus_credit_each_master(UBUS_PORT_ID_DSL);
#endif

#if defined(CONFIG_BCM_UBUS4_DCM)
	ubus_cong_threshold_wr(UBUS_PORT_ID_DSLCPU, 0);
	ubus_cong_threshold_wr(UBUS_PORT_ID_DSL, 0);
#endif

#if IS_BCMCHIP(63158)
	pmc_wan_interface_power_control(WAN_INTF_XDSL, 1);
#endif

	g_powered_up = 1;

	return ret;
}

#if IS_BCMCHIP(63146)

#define DSLPHY_PHYS_BASE      0x80750000
#define DSLPHY_SIZE           0x20000
void pmc_dsl_block_shutdown(void)
{
	uint32_t val = 0;
	void __iomem *res = ioremap(DSLPHY_PHYS_BASE, DSLPHY_SIZE);
	if (IS_ERR(res))
	{
		printk("%s: Failed to map PHY regs @DSLPHY_PHYS_BASE=0x%xx\n",__FUNCTION__,DSLPHY_PHYS_BASE);
		return;
	}

	printk("%s: resetting PHY regs @DSLPHY_BASE=0x%px\n",__FUNCTION__,res);
	// FIXME!! Might need to convert these into named macros.
	// The offsets are taken from DSLSIM build environment.
	writel(val, res+0x05000); // RBUS_RSENC_A_BASE
	writel(val, res+0x06000); // RBUS_RSENC_B_BASE
	writel(val, res+0x07000); // RBUS_AHIF_BASE
	writel(val, res+0x07800); // RBUS_AHIF_TXPAF_BASE
	writel(val, res+0x07B00); // RBUS_AHIF_GFAST_BASE
	writel(val, res+0x09000); // RBUS_CMD_DMA_BASE
	writel(val, res+0x11000); // RBUS_RSDEC_A_BASE
	writel(val, res+0x12000); // RBUS_RSDEC_B_BASE

	//printk("%s: resetting PHY (CmdDisp) regs @DSLPHY_BASE=0x%px\n",__FUNCTION__,res);
	writel(val, res+0x06FF0); // RBUS_RSENC_B_CMD_DISP_BASE
	writel(val, res+0x05FF0); // RBUS_RSENC_A_CMD_DISP_BASE
	writel(val, res+0x07F90); // RBUS_AHIF_GFD0_CMD_DISP_BASE
	writel(val, res+0x07FA0); // RBUS_AHIF_GFD1_CMD_DISP_BASE
	writel(val, res+0x07FB0); // RBUS_AHIF_DS_CMD_DISP_BASE
	writel(val, res+0x07FC0); // RBUS_AHIF_US0_CMD_DISP_BASE
	writel(val, res+0x07FD0); // RBUS_AHIF_US1_CMD_DISP_BASE
	writel(val, res+0x07FE0); // RBUS_AHIF_US2_CMD_DISP_BASE
	writel(val, res+0x07FF0); // RBUS_AHIF_US3_CMD_DISP_BASE
	writel(val, res+0x09F40); // RBUS_CMD_DMA_XXX_CMD_DISP_BASE
	writel(val, res+0x09F50); // RBUS_CMD_DMA_XXX_CMD_DISP_BASE
	writel(val, res+0x09F60); // RBUS_CMD_DMA_XXX_CMD_DISP_BASE
	writel(val, res+0x09F70); // RBUS_CMD_DMA_XXX_CMD_DISP_BASE
	writel(val, res+0x09F80); // RBUS_CMD_DMA_XXX_CMD_DISP_BASE
	writel(val, res+0x09F90); // RBUS_CMD_DMA_XXX_CMD_DISP_BASE
	writel(val, res+0x09FA0); // RBUS_CMD_DMA_XXX_CMD_DISP_BASE
	writel(val, res+0x09FB0); // RBUS_CMD_DMA_XXX_CMD_DISP_BASE
	writel(val, res+0x09FC0); // RBUS_CMD_DMA_XXX_CMD_DISP_BASE
	writel(val, res+0x09FD0); // RBUS_CMD_DMA_XXX_CMD_DISP_BASE
	writel(val, res+0x09FE0); // RBUS_CMD_DMA_XXX_CMD_DISP_BASE
	writel(val, res+0x09FF0); // RBUS_CMD_DMA_XXX_CMD_DISP_BASE
	writel(val, res+0x11FF0); // RBUS_RSDEC_A_CMD_DISP_BASE
	writel(val, res+0x12FF0); // RBUS_RSDEC_B_CMD_DISP_BASE
    iounmap(res);
}
#endif

int pmc_dsl_power_down(void)
{
	int ret = 0;

	if (!g_powered_up)
		return ret;

#if IS_BCMCHIP(63158)
   /* WAN interface power control interferes with traffic permanently. So no
   ** need to control. It is not an option for power saving.
   **/
	ret |= PowerOffDevice(PMB_ADDR_VDSL3_PMD, 0);
	ret |= PowerOffDevice(PMB_ADDR_VDSL3_CORE, 0);
	ret |= PowerOffDevice(PMB_ADDR_VDSL3_MIPS, 0);
#elif IS_BCMCHIP(63178)
	ret |= PowerOffDevice(PMB_ADDR_VDSL3_CORE, 0);
#elif IS_BCMCHIP(63146)
#if 1
#if CONFIG_BRCM_CHIP_REV==0x63146A0
	//printk("%s: calling pmc_dsl_block_shutdown\n", __FUNCTION__);
	pmc_dsl_block_shutdown();
#endif
	return ret;
#else
	ret |= PowerOffDevice(PMB_ADDR_VDSL3_PMD, 0);
	ret |= PowerOffDevice(PMB_ADDR_VDSL3_CORE, 0);
#endif
#endif

	g_powered_up = 0;

	return ret;
}

#if IS_BCMCHIP(63138) || IS_BCMCHIP(63148)
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
#endif /* IS_BCMCHIP(63138) || IS_BCMCHIP(63148) */

#if IS_BCMCHIP(63146)
__attribute__((unused))
static int pmc_dsl_armcore_reset(void)
{
	int ret;
	uint32_t reg;

	ret = ReadBPCMRegister(PMB_ADDR_VDSL3_CORE,
			BPCMVDSLRegOffset(vdsl_arm_sr), &reg);
	if (ret)
		return ret;

	reg |= 0x1;  // assert arm resets
	ret = WriteBPCMRegister(PMB_ADDR_VDSL3_CORE,
			BPCMVDSLRegOffset(vdsl_arm_sr), reg);
	if (ret)
		return ret;

	reg &= ~0x1;  // deassert arm resets
	ret = WriteBPCMRegister(PMB_ADDR_VDSL3_CORE,
			BPCMVDSLRegOffset(vdsl_arm_sr), reg);

	return ret;
}

int pmc_dsl_armcore_enable(int flag)
{
	int ret;
	BPCM_VDSL_ARM_RST_CTL rst_ctl;

#if CONFIG_BRCM_CHIP_REV!=0x63146A0
	BPCM_VDSL_ARM_SR_CTL arm_sr_reg;
	if((ret = ReadBPCMRegister(PMB_ADDR_VDSL3_CORE, BPCMVDSLRegOffset(vdsl_arm_sr), &arm_sr_reg.Reg32)))
		return ret;
#endif

	ret = ReadBPCMRegister(PMB_ADDR_VDSL3_CORE,
			BPCMVDSLRegOffset(vdsl_arm_rst_control), &rst_ctl.Reg32);
	if (ret)
		return ret;

	if (flag) {
	    // de-assert arm resets
	    // rst_ctl.Reg32 |=  ((1<<0)|(1<<1)|(1<<2)|(1<<3)|(1<<7) | (1<<9)|(1<<10));
	    rst_ctl.Bits_vdsl.vdsl_arm_por_reset_n     = 1;
	    rst_ctl.Bits_vdsl.vdsl_arm_reset_n         = 1;
	    rst_ctl.Bits_vdsl.vdsl_arm_debug_reset_n   = 1;
	    rst_ctl.Bits_vdsl.vdsl_arm_l2_reset_n      = 1;
	    rst_ctl.Bits_vdsl.vdsl_arm_nsocdbgreset_a7 = 1;
	    rst_ctl.Bits_vdsl.vdsl_arm_dbgen_a7_b0     = 1;
	    rst_ctl.Bits_vdsl.vdsl_arm_spiden_a7_b0    = 1;
#if CONFIG_BRCM_CHIP_REV!=0x63146A0
		// de-asserting VDSL ARM soft reset
		// [0x420] &= ~1
		arm_sr_reg.Bits.sr = 0;
#endif
	}
	else {
	    // assert arm resets
	    // rst_ctl.Reg32 &=  ((1<<0)|(1<<2)|(1<<3)|(1<<7) | (1<<9)|(1<<10));
	    rst_ctl.Bits_vdsl.vdsl_arm_por_reset_n     = 0;
	    //rst_ctl.Bits_vdsl.vdsl_arm_reset_n         = 0;
	    rst_ctl.Bits_vdsl.vdsl_arm_debug_reset_n   = 0;
	    rst_ctl.Bits_vdsl.vdsl_arm_l2_reset_n      = 0;
	    rst_ctl.Bits_vdsl.vdsl_arm_nsocdbgreset_a7 = 0;
	    rst_ctl.Bits_vdsl.vdsl_arm_dbgen_a7_b0     = 0;
	    rst_ctl.Bits_vdsl.vdsl_arm_spiden_a7_b0    = 0;
#if CONFIG_BRCM_CHIP_REV!=0x63146A0
		// asserting VDSL ARM soft reset
		// [0x420] |= 1
		arm_sr_reg.Bits.sr = 1;
#endif
	}

#if CONFIG_BRCM_CHIP_REV==0x63146A0
	printk("%s: (flag=%d) bpcm reg[0x424] <= 0x%08x\n", __FUNCTION__, flag, rst_ctl.Reg32);
	ret = WriteBPCMRegister(PMB_ADDR_VDSL3_CORE,
			BPCMVDSLRegOffset(vdsl_arm_rst_control), rst_ctl.Reg32);
#else
	printk("%s: (flag=%d) bpcm reg[0x424] <= 0x%08x reg[0x420] <= 0x%08x\n", __FUNCTION__, flag, rst_ctl.Reg32, arm_sr_reg.Reg32);
	if (flag)
		ret = WriteBPCMRegister(PMB_ADDR_VDSL3_CORE,
		    BPCMVDSLRegOffset(vdsl_arm_sr), arm_sr_reg.Reg32);

	ret = WriteBPCMRegister(PMB_ADDR_VDSL3_CORE,
			BPCMVDSLRegOffset(vdsl_arm_rst_control), rst_ctl.Reg32);

	if (!flag)
		ret = WriteBPCMRegister(PMB_ADDR_VDSL3_CORE,
		    BPCMVDSLRegOffset(vdsl_arm_sr), arm_sr_reg.Reg32);
#endif
	return ret;
}

/* keep old API for backaward compatiblity */
int pmc_dsl_mipscore_enable(int flag, int core)
{
	return pmc_dsl_armcore_enable(flag);
}
#else // end of #if IS_BCMCHIP(63146)
int pmc_dsl_mipscore_enable(int flag, int core)
{
	int ret;
	BPCM_MISC_CONTROL misc_ctrl;
	int misc_offset = BPCMVDSLRegOffset(misc_control);


#if IS_BCMCHIP(63138)
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

#if IS_BCMCHIP(63158) || IS_BCMCHIP(63178)
	if( core ) {
		printk("pmc_dsl_mipscore_enable not supported for secondary mips thread, flag %d core %d\n", flag, core);
		return 1;
	}
#endif

	/* perform individual read/write of the reset bits per vdsl team suggestion */
	if (flag) {
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
	}

	return ret;
}
#endif // end of #if IS_BCMCHIP(63146) #else

/* keep the old API for compatiblity and enable/disable the first PHY MIPS core */
int pmc_dsl_mips_enable(int flag)
{
#if IS_BCMCHIP(63146)
	return pmc_dsl_armcore_enable(flag);
#else
	return pmc_dsl_mipscore_enable(flag, 0);
#endif
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
#if IS_BCMCHIP(63158)
	sr_ctrl.Bits_vdsl.vdsl_bpcm_early_reset = 1;
	sr_ctrl.Bits_vdsl.vdsl_bpcm_reset = 1;
#elif IS_BCMCHIP(63178)
	sr_ctrl.Bits_vdsl.vdsl_bpcm_early_reset = 1;
	sr_ctrl.Bits_vdsl.vdsl_bpcm_reset = 1;
	sr_ctrl.Bits_vdsl.qproc_1_bpcm_reset = 1;
	sr_ctrl.Bits_vdsl.sar_bpcm_soft_reset = 1;
#elif IS_BCMCHIP(63146)
#if CONFIG_BRCM_CHIP_REV!=0x63146A0
	sr_ctrl.Bits_vdsl.vdsl_bpcm_early_reset = 1;
	sr_ctrl.Bits_vdsl.vdsl_bpcm_soft_reset = 1;
	sr_ctrl.Bits_vdsl.arm_axi_bpcm_cpu_clk_soft_reset = 1;
	sr_ctrl.Bits_vdsl.arm_axi_bpcm_arm_clk_soft_reset = 1;
#endif
	pmc_dsl_armcore_enable(0);
#else
	sr_ctrl.Bits.sr &= ~0x3;
#endif
	sr_ctrl.Bits.gp = 0xffffff;

#if CONFIG_BRCM_CHIP_REV!=0x63146A0
	ret = WriteBPCMRegister(PMB_ADDR_VDSL3_CORE,
			BPCMVDSLRegOffset(sr_control), sr_ctrl.Reg32);
	if (ret)
		return ret;
#endif

#if IS_BCMCHIP(63146) && (CONFIG_BRCM_CHIP_REV==0x63146A0)
	//printk("%s: calling pmc_dsl_block_shutdown\n", __FUNCTION__);
	pmc_dsl_block_shutdown();
#endif

	/* de-assert VDSL core reset */
	ret = ReadBPCMRegister(PMB_ADDR_VDSL3_CORE,
			BPCMVDSLRegOffset(sr_control), &sr_ctrl.Reg32);
	if (ret)
		return ret;
	
#if IS_BCMCHIP(63158)
	sr_ctrl.Bits_vdsl.vdsl_bpcm_early_reset = 0;
	sr_ctrl.Bits_vdsl.vdsl_bpcm_reset = 0;
#elif IS_BCMCHIP(63178)
	sr_ctrl.Bits_vdsl.vdsl_bpcm_early_reset = 0;
	sr_ctrl.Bits_vdsl.vdsl_bpcm_reset = 0;
	sr_ctrl.Bits_vdsl.qproc_1_bpcm_reset = 0;
	sr_ctrl.Bits_vdsl.sar_bpcm_soft_reset = 0;
#elif IS_BCMCHIP(63146)
#if CONFIG_BRCM_CHIP_REV!=0x63146A0
	sr_ctrl.Bits_vdsl.vdsl_bpcm_early_reset = 0;
	sr_ctrl.Bits_vdsl.vdsl_bpcm_soft_reset = 0;
	sr_ctrl.Bits_vdsl.arm_axi_bpcm_cpu_clk_soft_reset = 0;
	sr_ctrl.Bits_vdsl.arm_axi_bpcm_arm_clk_soft_reset = 0;
#endif
#else
	sr_ctrl.Bits.sr |= 0x3;
#endif
	sr_ctrl.Bits.gp = 0xffffff;
	
#if CONFIG_BRCM_CHIP_REV!=0x63146A0
	ret = WriteBPCMRegister(PMB_ADDR_VDSL3_CORE,
			BPCMVDSLRegOffset(sr_control), sr_ctrl.Reg32);
#endif

#if IS_BCMCHIP(63158) || IS_BCMCHIP(63146)
	/* reset PMD through BPCM */
	ret = ReadBPCMRegister(PMB_ADDR_VDSL3_PMD,
			BPCMVDSLRegOffset(sr_control), &sr_ctrl.Reg32);
	if (ret)
		return ret;
	sr_ctrl.Bits.sr |= 0x1;
	ret = WriteBPCMRegister(PMB_ADDR_VDSL3_PMD,
			BPCMVDSLRegOffset(sr_control), sr_ctrl.Reg32);
	if (ret)
		return ret;

	ret = ReadBPCMRegister(PMB_ADDR_VDSL3_PMD,
			BPCMVDSLRegOffset(sr_control), &sr_ctrl.Reg32);
	if (ret)
		return ret;
	sr_ctrl.Bits.sr &= ~0x1;
	ret = WriteBPCMRegister(PMB_ADDR_VDSL3_PMD,
			BPCMVDSLRegOffset(sr_control), sr_ctrl.Reg32);
#endif

#if IS_BCMCHIP(63146)
   /* Reading WAN BBH CFG TCONT address to ensure no impact */
   if (g_powered_up)
      printk("=====>New %s %d 0x%8.8x\n", __FUNCTION__, __LINE__, *((uint32_t *) 0xffffff800a894010));
   else
      printk("=====>New %s %d dsl not powered up\n", __FUNCTION__, __LINE__);
#endif
	return ret;
}

int BcmXdslGetAfePLLChOffset(unsigned int chanId)
{
	int chOffset = -1;
	
	switch(chanId)
	{
		case CH01_CFG:
#ifdef CONFIG_BCM963146
			chOffset = AFEPLLBPCMRegOffset(ch01_cfg);
#else
            chOffset = PLLBPCMRegOffset(ch01_cfg);
#endif
			break;
		case CH23_CFG:
#ifdef CONFIG_BCM963146
			chOffset = AFEPLLBPCMRegOffset(ch23_cfg);
#else
            chOffset = PLLBPCMRegOffset(ch23_cfg);
#endif
			break;
		case CH45_CFG:
#ifdef CONFIG_BCM963146
			chOffset = AFEPLLBPCMRegOffset(ch45_cfg);
#else
            chOffset = PLLBPCMRegOffset(ch45_cfg);
#endif
			break;
		default:
			break;
	}
	
	return chOffset;
}

int ReadVDSL3PLLChCfg(int chOffset, uint32_t *pChCfg)
{
    return ReadBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE, chOffset, pChCfg);
}

int WriteVDSL3PLLChCfg(int chOffset, uint32_t chCfg)
{
    return WriteBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE, chOffset, chCfg);
}

int ReadVDSL3PLLMdiv(unsigned int chanId, pll_ch_cfg_t *pChCfg)
{
    int ret;
    PLL_CHCFG_REG chCfg;
    int chOffset = BcmXdslGetAfePLLChOffset(chanId); 

    if (chOffset == -1)
        return kPMC_INVALID_PARAM;

    ret = ReadBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE, chOffset, &chCfg.Reg32);
    if (ret)
        return ret;

    pChCfg->Reg = chCfg.Reg32;
    pChCfg->mdiv0 = chCfg.Bits.mdiv0;
    pChCfg->mdiv_override0 = chCfg.Bits.mdiv_override0;
    pChCfg->mdel0 = chCfg.Bits.mdel0;
    pChCfg->mdiv1 = chCfg.Bits.mdiv1;
    pChCfg->mdiv_override1 = chCfg.Bits.mdiv_override1;
    pChCfg->mdel1 = chCfg.Bits.mdel1;

    return ret;
}

int ModifyVDSL3PLLMdiv(unsigned int chanId, pll_ch_cfg_t i_chCfg)
{
    int ret;
    PLL_CHCFG_REG chCfg;
    int chOffset = BcmXdslGetAfePLLChOffset(chanId); 

    if (chOffset == -1)
        return kPMC_INVALID_PARAM;

    ret = ReadBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE, chOffset, &chCfg.Reg32);
    if (ret)
        return ret;

    chCfg.Bits.mdiv0 = i_chCfg.mdiv0;
    chCfg.Bits.mdiv_override0 = i_chCfg.mdiv_override0;
    chCfg.Bits.mdel0 = i_chCfg.mdel0;

    chCfg.Bits.mdiv1 = i_chCfg.mdiv1;
    chCfg.Bits.mdiv_override1 = i_chCfg.mdiv_override1;
    chCfg.Bits.mdel1 = i_chCfg.mdel1;
    
    ret = WriteBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE, chOffset, chCfg.Reg32);
    return ret;
}

int ResetVDSL3PLL(void)
{
    int ret;
    PLL_CTRL_REG pll_ctrl;

    ret = ReadBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE, PLLBPCMRegOffset(resets), &pll_ctrl.Reg32);
    if (ret)
        return ret;

    pll_ctrl.Bits.resetb = 0;
    pll_ctrl.Bits.post_resetb = 0;
    ret = WriteBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE, PLLBPCMRegOffset(resets), pll_ctrl.Reg32);

    return ret;
}

int ReadVDSL3PLLNdiv(uint32_t *reg, int *ndiv)
{
    int ret;
    PLL_NDIV_REG pll_ndiv;

    ret = ReadBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE, PLLBPCMRegOffset(ndiv), &pll_ndiv.Reg32);
    if (ret)
        return ret;

    if (reg)
        *reg = pll_ndiv.Reg32;
    if (ndiv)
        *ndiv = pll_ndiv.Bits.ndiv_int;
    return ret;
}

int SetVDSL3Ndiv(int ndiv)
{
    int ret;
    PLL_NDIV_REG pll_ndiv;

    ret = ReadBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE, PLLBPCMRegOffset(ndiv), &pll_ndiv.Reg32);
    if (ret)
        return ret;
    pll_ndiv.Bits.ndiv_int = ndiv;	/* ndiv_int[9:0] */
    pll_ndiv.Bits.ndiv_override = 1;					/* Set NDIV Override bit 31*/
    ret = WriteBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE, PLLBPCMRegOffset(ndiv), pll_ndiv.Reg32);

    return ret;
}

int ReleaseVDSL3PLLResetb(void)
{
    int ret;
    PLL_CTRL_REG pll_ctrl;

    ret = ReadBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE, PLLBPCMRegOffset(resets), &pll_ctrl.Reg32);
    if (ret)
        return ret;
    pll_ctrl.Bits.resetb = 1;
    ret = WriteBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE, PLLBPCMRegOffset(resets), pll_ctrl.Reg32);
    
    return ret;
}

int ReleaseVDSL3PLLPostResetb(void)
{
    int ret;
    PLL_CTRL_REG pll_ctrl;

    ret = ReadBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE, PLLBPCMRegOffset(resets), &pll_ctrl.Reg32);
    if (ret)
        return ret;
    pll_ctrl.Bits.post_resetb = 1;
    ret = WriteBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE, PLLBPCMRegOffset(resets), pll_ctrl.Reg32);
    
    return ret;
}

int WaitForLockVDSL3PLL(void)
{
    int ret;
    PLL_LOOP1_REG pll_loop1;
    do 
    {
        ret = ReadBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE,
            PLLBPCMRegOffset(loop1), &pll_loop1.Reg32);
        if (ret)
            return ret;
    } while (!(pll_loop1.Bits.ssc_mode));

    return ret;
}

int SetVDSL3HoldCh1(unsigned int chanId, int value)
{
    int ret;
    PLL_CHCFG_REG pllch_cfg;
    int offset =BcmXdslGetAfePLLChOffset(chanId); 

    if (offset == -1)
        return kPMC_INVALID_PARAM;

    ret = ReadBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE, offset, &pllch_cfg.Reg32);
    if (ret)
        return ret;
    pllch_cfg.Bits.hold_ch1 = value;
    
    ret = WriteBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE, offset, pllch_cfg.Reg32);
    return ret;
}

int SetVDSL3EnablCh1(unsigned int chanId, int value)
{
    int ret;
    int offset =BcmXdslGetAfePLLChOffset(chanId); 
    PLL_CHCFG_REG pllch_cfg;

    if (offset == -1)
        return kPMC_INVALID_PARAM;

    ret = ReadBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE, offset, &pllch_cfg.Reg32);
    if (ret)
        return ret;
    pllch_cfg.Bits.enableb_ch1 = value;
    
    ret = WriteBPCMRegister(AFEPLL_PMB_ADDR_VDSL3_CORE, offset, pllch_cfg.Reg32);
    return ret;
}
EXPORT_SYMBOL(BcmXdslGetAfePLLChOffset);
EXPORT_SYMBOL(ReadVDSL3PLLChCfg);
EXPORT_SYMBOL(WriteVDSL3PLLChCfg);
EXPORT_SYMBOL(ReadVDSL3PLLMdiv);
EXPORT_SYMBOL(ModifyVDSL3PLLMdiv);
EXPORT_SYMBOL(ResetVDSL3PLL);
EXPORT_SYMBOL(SetVDSL3Ndiv);
EXPORT_SYMBOL(ReadVDSL3PLLNdiv);
EXPORT_SYMBOL(ReleaseVDSL3PLLResetb);
EXPORT_SYMBOL(ReleaseVDSL3PLLPostResetb);
EXPORT_SYMBOL(WaitForLockVDSL3PLL);
EXPORT_SYMBOL(SetVDSL3HoldCh1);
EXPORT_SYMBOL(SetVDSL3EnablCh1);

#ifdef CONFIG_DSLLMEM_TEST
#define DSL_LMEM	((volatile uint32_t *)(DSLLMEM_BASE))
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

EXPORT_SYMBOL(pmc_dsl_power_up);
EXPORT_SYMBOL(pmc_dsl_power_down);
#if IS_BCMCHIP(63138) || IS_BCMCHIP(63148)
EXPORT_SYMBOL(pmc_dsl_clock_set);
#endif 
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

