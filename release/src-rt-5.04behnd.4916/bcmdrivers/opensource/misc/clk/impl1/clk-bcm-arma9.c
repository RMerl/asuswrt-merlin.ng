/*
<:copyright-BRCM:2021:DUAL/GPL:standard 

   Copyright (c) 2021 Broadcom 
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

#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/stop_machine.h>
#include "clk-bcm63xx.h"

/*
 * ARM CFG
 */
typedef struct ArmProcClkMgr {
   u32 wr_access;    /* 0x00 */
#define ARM_PROC_CLK_WR_ACCESS_PASSWORD_SHIFT         8
#define ARM_PROC_CLK_WR_ACCESS_PASSWORD_MASK          (0xffff<<ARM_PROC_CLK_WR_ACCESS_PASSWORD_SHIFT)
#define ARM_PROC_CLK_WR_ACCESS_PASSWORD               0xA5A5
#define ARM_PROC_CLK_WR_ACCESS_CLKMGR_ACC_SHIFT       0
#define ARM_PROC_CLK_WR_ACCESS_CLKMGR_ACC_MASK        (0x1<<ARM_PROC_CLK_WR_ACCESS_CLKMGR_ACC_SHIFT)
#define ARM_PROC_CLK_WR_ACCESS_CLKMGR_ACC             0x1
   u32 unused0;
   u32 policy_freq;
#define ARM_PROC_CLK_POLICY3_FREQ_SHIFT               24
#define ARM_PROC_CLK_POLICY3_FREQ_MASK                (0x7<<ARM_PROC_CLK_POLICY3_FREQ_SHIFT)
#define ARM_PROC_CLK_POLICY2_FREQ_SHIFT               16
#define ARM_PROC_CLK_POLICY2_FREQ_MASK                (0x7<<ARM_PROC_CLK_POLICY2_FREQ_SHIFT)
#define ARM_PROC_CLK_POLICY1_FREQ_SHIFT               8
#define ARM_PROC_CLK_POLICY1_FREQ_MASK                (0x7<<ARM_PROC_CLK_POLICY1_FREQ_SHIFT)
#define ARM_PROC_CLK_POLICY0_FREQ_SHIFT               0
#define ARM_PROC_CLK_POLICY0_FREQ_MASK                (0x7<<ARM_PROC_CLK_POLICY0_FREQ_SHIFT)
#define ARM_PROC_CLK_POLICY_FREQ_MASK                 (ARM_PROC_CLK_POLICY0_FREQ_MASK|ARM_PROC_CLK_POLICY1_FREQ_MASK|ARM_PROC_CLK_POLICY2_FREQ_MASK|ARM_PROC_CLK_POLICY3_FREQ_MASK)
#define ARM_PROC_CLK_POLICY_FREQ_ALL(x)	              ((x << ARM_PROC_CLK_POLICY3_FREQ_SHIFT) | (x << ARM_PROC_CLK_POLICY2_FREQ_SHIFT) | \
                                                       (x << ARM_PROC_CLK_POLICY1_FREQ_SHIFT) | (x << ARM_PROC_CLK_POLICY0_FREQ_SHIFT))
#define ARM_PROC_CLK_POLICY_FREQ_CRYSTAL              0
#define ARM_PROC_CLK_POLICY_FREQ_SYSCLK               2
#define ARM_PROC_CLK_POLICY_FREQ_ARMPLL_SLOW          6
#define ARM_PROC_CLK_POLICY_FREQ_ARMPLL_FAST          7
   u32 policy_ctl;
#define ARM_PROC_CLK_POLICY_CTL_GO_ATL                0x4
#define ARM_PROC_CLK_POLICY_CTL_GO_AC                 0x2
#define ARM_PROC_CLK_POLICY_CTL_GO                    0x1
   u32 policy0_mask;    /* 0x10 */
   u32 policy1_mask;
   u32 policy2_mask;
   u32 policy3_mask;
   u32 int_en;       /* 0x20 */
   u32 int_stat;
   u32 unused1[3];
   u32 lvm_en;       /* 0x34 */
   u32 lvm0_3;
   u32 lvm4_7;
   u32 vlt0_3;       /* 0x40 */
   u32 vlt4_7;
   u32 unused2[46];
   u32 bus_quiesc;      /* 0x100 */
   u32 unused3[63];
   u32 core0_clkgate;      /* 0x200 */
#define ARM_PROC_CLK_CORE0_CLKGATE_HYST_EN_SHIFT     9
#define ARM_PROC_CLK_CORE0_CLKGATE_HYST_EN_MASK      (0x1<<ARM_PROC_CLK_CORE0_CLKGATE_HYST_EN_SHIFT)
#define ARM_PROC_CLK_CORE0_CLKGATE_HYST_EN           1
#define ARM_PROC_CLK_CORE0_CLKGATE_HYST_DIS          0
#define ARM_PROC_CLK_CORE0_CLKGATE_HYST_VAL_SHIFT    8
#define ARM_PROC_CLK_CORE0_CLKGATE_HYST_VAL_MASK     (0x1<<ARM_PROC_CLK_CORE0_CLKGATE_HYST_VAL_SHIFT)
#define ARM_PROC_CLK_CORE0_CLKGATE_HYST_VAL_LOW      0x0
#define ARM_PROC_CLK_CORE0_CLKGATE_HYST_VAL_HIGH     0x1
#define ARM_PROC_CLK_CORE0_CLKGATE_GATING_SEL_SHIFT  1
#define ARM_PROC_CLK_CORE0_CLKGATE_GATING_SEL_MASK   (0x1<<ARM_PROC_CLK_CORE0_CLKGATE_GATING_SEL_SHIFT)
#define ARM_PROC_CLK_CORE0_CLKGATE_GATING_SEL_HW     0x0
#define ARM_PROC_CLK_CORE0_CLKGATE_GATING_SEL_SW     0x1
#define ARM_PROC_CLK_CORE0_CLKGATE_CLK_EN_SHIFT      0
#define ARM_PROC_CLK_CORE0_CLKGATE_CLK_EN_MASK       (0x1<<ARM_PROC_CLK_CORE0_CLKGATE_CLK_EN_SHIFT)
#define ARM_PROC_CLK_CORE0_CLKGATE_CLK_EN            1
#define ARM_PROC_CLK_CORE0_CLKGATE_CLK_DIS           0
   u32 core1_clkgate;
#define ARM_PROC_CLK_CORE1_CLKGATE_HYST_EN_SHIFT     9
#define ARM_PROC_CLK_CORE1_CLKGATE_HYST_EN_MASK      (0x1<<ARM_PROC_CLK_CORE0_CLKGATE_HYST_EN_SHIFT)
#define ARM_PROC_CLK_CORE1_CLKGATE_HYST_EN           1
#define ARM_PROC_CLK_CORE1_CLKGATE_HYST_DIS          0
#define ARM_PROC_CLK_CORE1_CLKGATE_HYST_VAL_SHIFT    8
#define ARM_PROC_CLK_CORE1_CLKGATE_HYST_VAL_MASK     (0x1<<ARM_PROC_CLK_CORE0_CLKGATE_HYST_VAL_SHIFT)
#define ARM_PROC_CLK_CORE1_CLKGATE_HYST_VAL_LOW      0x0
#define ARM_PROC_CLK_CORE1_CLKGATE_HYST_VAL_HIGH     0x1
#define ARM_PROC_CLK_CORE1_CLKGATE_GATING_SEL_SHIFT  1
#define ARM_PROC_CLK_CORE1_CLKGATE_GATING_SEL_MASK   (0x1<<ARM_PROC_CLK_CORE0_CLKGATE_GATING_SEL_SHIFT)
#define ARM_PROC_CLK_CORE1_CLKGATE_GATING_SEL_HW     0x0
#define ARM_PROC_CLK_CORE1_CLKGATE_GATING_SEL_SW     0x1
#define ARM_PROC_CLK_CORE1_CLKGATE_CLK_EN_SHIFT      0
#define ARM_PROC_CLK_CORE1_CLKGATE_CLK_EN_MASK       (0x1<<ARM_PROC_CLK_CORE0_CLKGATE_CLK_EN_SHIFT)
#define ARM_PROC_CLK_CORE1_CLKGATE_CLK_EN            1
#define ARM_PROC_CLK_CORE1_CLKGATE_CLK_DIS           0
   u32 unused4[2];
   u32 arm_switch_clkgate; /* 0x210 */
#define ARM_PROC_CLK_SWITCH_CLKGATE_HYST_EN_SHIFT    9
#define ARM_PROC_CLK_SWITCH_CLKGATE_HYST_EN_MASK     (0x1<<ARM_PROC_CLK_CORE0_CLKGATE_HYST_EN_SHIFT)
#define ARM_PROC_CLK_SWITCH_CLKGATE_HYST_EN          1
#define ARM_PROC_CLK_SWITCH_CLKGATE_HYST_DIS         0
#define ARM_PROC_CLK_SWITCH_CLKGATE_HYST_VAL_SHIFT   8
#define ARM_PROC_CLK_SWITCH_CLKGATE_HYST_VAL_MASK    (0x1<<ARM_PROC_CLK_CORE0_CLKGATE_HYST_VAL_SHIFT)
#define ARM_PROC_CLK_SWITCH_CLKGATE_HYST_VAL_LOW     0x0
#define ARM_PROC_CLK_SWITCH_CLKGATE_HYST_VAL_HIGH    0x1
#define ARM_PROC_CLK_SWITCH_CLKGATE_GATING_SEL_SHIFT 1
#define ARM_PROC_CLK_SWITCH_CLKGATE_GATING_SEL_MASK  (0x1<<ARM_PROC_CLK_CORE0_CLKGATE_GATING_SEL_SHIFT)
#define ARM_PROC_CLK_SWITCH_CLKGATE_GATING_SEL_HW    0x0
#define ARM_PROC_CLK_SWITCH_CLKGATE_GATING_SEL_SW    0x1
#define ARM_PROC_CLK_SWITCH_CLKGATE_CLK_EN_SHIFT     0
#define ARM_PROC_CLK_SWITCH_CLKGATE_CLK_EN_MASK      (0x1<<ARM_PROC_CLK_CORE0_CLKGATE_CLK_EN_SHIFT)
#define ARM_PROC_CLK_SWITCH_CLKGATE_CLK_EN           1
#define ARM_PROC_CLK_SWITCH_CLKGATE_CLK_DIS          0
   u32 unused5[59];
   u32 arm_periph_clkgate; /* 0x300 */
#define ARM_PROC_CLK_PERIPH_CLKGATE_HYST_EN_SHIFT    9
#define ARM_PROC_CLK_PERIPH_CLKGATE_HYST_EN_MASK     (0x1<<ARM_PROC_CLK_CORE0_CLKGATE_HYST_EN_SHIFT)
#define ARM_PROC_CLK_PERIPH_CLKGATE_HYST_EN          1
#define ARM_PROC_CLK_PERIPH_CLKGATE_HYST_DIS         0
#define ARM_PROC_CLK_PERIPH_CLKGATE_HYST_VAL_SHIFT   8
#define ARM_PROC_CLK_PERIPH_CLKGATE_HYST_VAL_MASK    (0x1<<ARM_PROC_CLK_CORE0_CLKGATE_HYST_VAL_SHIFT)
#define ARM_PROC_CLK_PERIPH_CLKGATE_HYST_VAL_LOW     0x0
#define ARM_PROC_CLK_PERIPH_CLKGATE_HYST_VAL_HIGH    0x1
#define ARM_PROC_CLK_PERIPH_CLKGATE_GATING_SEL_SHIFT 1
#define ARM_PROC_CLK_PERIPH_CLKGATE_GATING_SEL_MASK  (0x1<<ARM_PROC_CLK_CORE0_CLKGATE_GATING_SEL_SHIFT)
#define ARM_PROC_CLK_PERIPH_CLKGATE_GATING_SEL_HW    0x0
#define ARM_PROC_CLK_PERIPH_CLKGATE_GATING_SEL_SW    0x1
#define ARM_PROC_CLK_PERIPH_CLKGATE_CLK_EN_SHIFT     0
#define ARM_PROC_CLK_PERIPH_CLKGATE_CLK_EN_MASK      (0x1<<ARM_PROC_CLK_CORE0_CLKGATE_CLK_EN_SHIFT)
#define ARM_PROC_CLK_PERIPH_CLKGATE_CLK_EN           1
#define ARM_PROC_CLK_PERIPH_CLKGATE_CLK_DIS          0
   u32 unused6[63];
   u32 apb0_clkgate;    /* 0x400 */
#define ARM_PROC_CLK_APB0_CLKGATE_HYST_EN_SHIFT      9
#define ARM_PROC_CLK_APB0_CLKGATE_HYST_EN_MASK       (0x1<<ARM_PROC_CLK_CORE0_CLKGATE_HYST_EN_SHIFT)
#define ARM_PROC_CLK_APB0_CLKGATE_HYST_EN            1
#define ARM_PROC_CLK_APB0_CLKGATE_HYST_DIS           0
#define ARM_PROC_CLK_APB0_CLKGATE_HYST_VAL_SHIFT     8
#define ARM_PROC_CLK_APB0_CLKGATE_HYST_VAL_MASK      (0x1<<ARM_PROC_CLK_CORE0_CLKGATE_HYST_VAL_SHIFT)
#define ARM_PROC_CLK_APB0_CLKGATE_HYST_VAL_LOW       0x0
#define ARM_PROC_CLK_APB0_CLKGATE_HYST_VAL_HIGH      0x1
#define ARM_PROC_CLK_APB0_CLKGATE_GATING_SEL_SHIFT   1
#define ARM_PROC_CLK_APB0_CLKGATE_GATING_SEL_MASK    (0x1<<ARM_PROC_CLK_CORE0_CLKGATE_GATING_SEL_SHIFT)
#define ARM_PROC_CLK_APB0_CLKGATE_GATING_SEL_HW      0x0
#define ARM_PROC_CLK_APB0_CLKGATE_GATING_SEL_SW      0x1
#define ARM_PROC_CLK_APB0_CLKGATE_CLK_EN_SHIFT       0
#define ARM_PROC_CLK_APB0_CLKGATE_CLK_EN_MASK        (0x1<<ARM_PROC_CLK_CORE0_CLKGATE_CLK_EN_SHIFT)
#define ARM_PROC_CLK_APB0_CLKGATE_CLK_EN             1
#define ARM_PROC_CLK_APB0_CLKGATE_CLK_DIS            0
   u32 unused7[383];
   u32 pl310_div;    /* 0xa00 */
   u32 pl310_trigger;
   u32 arm_switch_div;
   u32 arm_switch_trigger;
   u32 apb_div;         /* 0xa10 */
   u32 apb_trigger;
   u32 unused8[122];
   u32 pllarma;         /* 0xc00 */
#define ARM_PROC_CLK_PLLARMA_PLL_LOCK                (0x1<<28)
#define ARM_PROC_CLK_PLLARMA_PDIV_SHIFT              24
#define ARM_PROC_CLK_PLLARMA_PDIV_MASK               (0x7<<ARM_PROC_CLK_PLLARMA_PDIV_SHIFT)
#define ARM_PROC_CLK_PLLARMA_NDIV_SHIFT              8
#define ARM_PROC_CLK_PLLARMA_NDIV_MASK               (0x3ff<<ARM_PROC_CLK_PLLARMA_NDIV_SHIFT)
#define ARM_PROC_CLK_PLLARMA_PLL_LOCK_RAW            (0x1<<7)
#define ARM_PROC_CLK_PLLARMA_PWRDWN_SWOVRRIDE_SHIFT  4
#define ARM_PROC_CLK_PLLARMA_PWRDWN_SWOVRRIDE_MASK   (0x1<<ARM_PROC_CLK_PLLARMA_PWRDWN_SWOVRRIDE_SHIFT)
#define ARM_PROC_CLK_PLLARMA_SOFT_POST_RESETB_N      0x2          
#define ARM_PROC_CLK_PLLARMA_SOFT_RESETB_N           0x1
   u32 pllarmb;
#define ARM_PROC_CLK_PLLARMB_NDIV_FRAC_SHIFT         0
#define ARM_PROC_CLK_PLLARMB_NDIV_FRAC_MASK         (0xfffff<<ARM_PROC_CLK_PLLARMB_NDIV_FRAC_SHIFT)
   u32 pllarmc;
#define ARM_PROC_CLK_PLLARMC_MDIV_SHIFT              0
#define ARM_PROC_CLK_PLLARMC_MDIV_MASK               (0xff<<ARM_PROC_CLK_PLLARMC_MDIV_SHIFT)
   u32 pllarmctrl0;
   u32 pllarmctrl1;     /* 0xc10 */
   u32 pllarmctrl2;
   u32 pllarmctrl3;
   u32 pllarmctrl4;
   u32 pllarmctrl5;     /* 0xc20 */
   u32 pllarm_offset;
   u32 unused9[118];
   u32 arm_div;         /* 0xe00 */
   u32 arm_seg_trg;
   u32 arm_seg_trg_override;
   u32 unused10;
   u32 pll_debug;    /* 0xe10 */
   u32 unused11[3];
   u32 activity_mon1;      /* 0xe20 */
   u32 activity_mon2;
   u32 unused12[6];
   u32 clkgate_dbg;     /* 0xe40 */
   u32 unused13;
   u32 apb_clkgate_dbg1;
   u32 unused14[6];
   u32 clkmon;       /* 0xe64 */
   u32 unused15[10];
   u32 kproc_ccu_prof_ctl; /* 0xe90 */
   u32 kproc_cpu_proc_sel;
   u32 kproc_cpu_proc_cnt;
   u32 kproc_cpu_proc_dbg;
   u32 unused16[8];
   u32 policy_dbg;      /* 0xec0 */
   u32 tgtmask_dbg1;
} ArmProcClkMgr;

typedef struct ArmProcResetMgr {
   u32 wr_access;    /* 0x00 */
   u32 soft_rstn;
   u32 a9_core_soft_rstn;
} ArmProcResetMgr;

typedef struct ArmCfg {
   ArmProcClkMgr proc_clk;       /* 0x00 */
   u32 unused0[14];              /* 0xec8 - 0xeff */
   ArmProcResetMgr proc_reset;   /* 0xf00 */
} ArmCfg;


static int core_set_freq(void* p)
{
	unsigned int mdiv;
	struct bcm63xx_cpuclk *cpuclk = (struct bcm63xx_cpuclk *)p;
	volatile ArmCfg __iomem *ARMCFG = (ArmCfg __iomem *)cpuclk->reg;

	mdiv = cpuclk->mdiv;
	ARMCFG->proc_clk.pllarmc = (ARMCFG->proc_clk.pllarmc & ~ARM_PROC_CLK_PLLARMC_MDIV_MASK) | mdiv;

	return 0;
}

int get_arm_core_ratio(struct bcm63xx_cpuclk *cpuclk, int *ratio, int *ratio_base, int *mdiv)
{
	volatile ArmCfg __iomem *ARMCFG = (ArmCfg __iomem *)cpuclk->reg;
	
	/* A9 arm core does not have addition cpu clock ratio.
	 * Use only mdiv for pll output channel
	 */
	*ratio = 1;
	*ratio_base = 1;
	*mdiv = (ARMCFG->proc_clk.pllarmc & ARM_PROC_CLK_PLLARMC_MDIV_MASK) >> ARM_PROC_CLK_PLLARMC_MDIV_SHIFT;

	return 0;
}

long round_arm_core_rate(struct bcm63xx_cpuclk *cpuclk, unsigned long rate)
{
	int mdiv;
	/* valid mdiv is 2 to 10 */
	mdiv = cpuclk->pllclk /rate;

	if (mdiv < 2 )
		mdiv = 2;
	else if (mdiv > 10)
		mdiv = 10;

	return cpuclk->pllclk/mdiv;
}

int set_arm_core_clock(struct bcm63xx_cpuclk *cpuclk, unsigned long parent_rate, unsigned long rate)
{
	const struct cpumask *cpus;

	cpuclk->mdiv = cpuclk->pllclk/rate;

	/* tie up cores to change frequency */
	cpus = cpumask_of(smp_processor_id());
	/* interrupts disabled in stop_machine */
	stop_machine(core_set_freq, cpuclk, cpus);

	return 0;
}

int init_arm_core_pll(struct bcm63xx_cpuclk *cpuclk)
{
	u32 pll = ARM_PROC_CLK_POLICY_FREQ_ALL(ARM_PROC_CLK_POLICY_FREQ_ARMPLL_SLOW);
	volatile ArmCfg __iomem *ARMCFG = (ArmCfg __iomem *)cpuclk->reg;	
	u32 policy = ARMCFG->proc_clk.policy_freq;
	const int mdiv_en = 1 << 11;

	/* arm pll at 2GHz and cpu base freq at 1GHz */
	cpuclk->pllclk = FREQ_MHZ(2000);
	cpuclk->mdiv = 2;

	//if its setup for nosmp mode, assume it has to run at a lower frequency 
	//instead of doing this if(strstr(boot_command_line, "nosmp ") != NULL)
	// we can just check the exported variable
	if(setup_max_cpus == 0)
	{
		cpuclk->mdiv = 3;
	}

	/* change policy to use ARMPLL_SLOW in case cfe isn't up-to-date */
	if ((policy & ARM_PROC_CLK_POLICY_FREQ_MASK) != pll) {

		pr_warn("%s update arm clk policy from 0x%x to 0x%x mdiv %d\n", __func__, policy, pll, cpuclk->mdiv);

		ARMCFG->proc_clk.pllarmc = (ARMCFG->proc_clk.pllarmc & ~ARM_PROC_CLK_PLLARMC_MDIV_MASK) | mdiv_en | cpuclk->mdiv;
		ARMCFG->proc_clk.policy_freq = (policy & ~ARM_PROC_CLK_POLICY_FREQ_MASK) | pll;

		/* enable policy and wait for policy to be activated */
		ARMCFG->proc_clk.policy_ctl |= ARM_PROC_CLK_POLICY_CTL_GO_AC|ARM_PROC_CLK_POLICY_CTL_GO;
		while (ARMCFG->proc_clk.policy_ctl & ARM_PROC_CLK_POLICY_CTL_GO);
	}

	return 0;
}
