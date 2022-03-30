/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Broadcom Ltd.
 */

#ifndef _63138_CPU_H
#define _63138_CPU_H

#define ARMGTIM_BASE        0x8001e200
#define AIP_BASE            0x80018000
#define ARMCFG_BASE         0x80020000

typedef struct ArmGTimer {
	uint32_t gtim_glob_low;	/* 0x00 */
	uint32_t gtim_glob_hi;	/* 0x04 */
	uint32_t gtim_glob_ctrl;	/* 0x08 */
#define ARM_GTIM_GLOB_CTRL_PRESCALE_SHIFT            8
#define ARM_GTIM_GLOB_CTRL_PRESCALE_MASK             (0xff<<ARM_GTIM_GLOB_CTRL_PRESCALE_SHIFT)
#define ARM_GTIM_GLOB_CTRL_AUTOINCR_EN               0x8
#define ARM_GTIM_GLOB_CTRL_IRQ_EN                    0x4
#define ARM_GTIM_GLOB_CTRL_COMP_EN                   0x2
#define ARM_GTIM_GLOB_CTRL_TIMER_EN                  0x1
	uint32_t gtim_glob_status;	/* 0x0c */
	uint32_t gtim_glob_comp_low;	/* 0x10 */
	uint32_t gtim_glob_comp_hi;	/* 0x14 */
	uint32_t gtim_glob_incr;	/* 0x18 */
} ArmGTimer;

#define ARMGTIM ((volatile ArmGTimer * const) ARMGTIM_BASE)

typedef struct ArmAipCtrl {
	uint32_t cfg;		/* 0x00 */
#define AIP_CTRL_CFG_WR_FAST_ACK_MASK 0x2
	uint32_t l2c_ctrl;	/* 0x04 */
	uint32_t arm_ctrl;	/* 0x08 */
	uint32_t unused0;
	uint32_t acp_ctrl[32];	/* 0x10-0xc */
#define AIPACP_WCACHE_SHIFT         0
#define AIPACP_WCACHE_MASK          0xf
#define AIPACP_WCACHE_GET(reg_val) ((reg_val >> AIPACP_WCACHE_SHIFT) & AIPACP_WCACHE_MASK)
#define AIPACP_WCACHE_SET(reg_val,new_val)   ((reg_val & ~(AIPACP_WCACHE_MASK << AIPACP_WCACHE_SHIFT)) | (new_val << AIPACP_WCACHE_SHIFT))
#define AIPACP_RCACHE_SHIFT         4
#define AIPACP_RCACHE_MASK          0xf
#define AIPACP_RCACHE_GET(reg_val)  ((reg_val >> AIPACP_RCACHE_SHIFT) & AIPACP_RCACHE_MASK)
#define AIPACP_RCACHE_SET(reg_val,new_val)   ((reg_val & ~(AIPACP_RCACHE_MASK << AIPACP_RCACHE_SHIFT)) | (new_val << AIPACP_RCACHE_SHIFT))
#define AIPACP_WUSER_SHIFT          8
#define AIPACP_WUSER_MASK           0x1f
#define AIPACP_WUSER_GET(reg_val)      ((reg_val >> AIPACP_WUSER_SHIFT) & AIPACP_WUSER_MASK)
#define AIPACP_WUSER_SET(reg_val,new_val) ((reg_val & ~(AIPACP_WUSER_MASK << AIPACP_WUSER_SHIFT)) | (new_val << AIPACP_WUSER_SHIFT))
#define AIPACP_RUSER_SHIFT          13
#define AIPACP_RUSER_MASK           0x1f
#define AIPACP_RUSER_GET(reg_val)   ((reg_val >> AIPACP_RUSER_SHIFT) & AIPACP_RUSER_MASK)
#define AIPACP_RUSER_SET(reg_val,new_val) ((reg_val & ~(AIPACP_RUSER_MASK << AIPACP_RUSER_SHIFT)) | (new_val << AIPACP_RUSER_SHIFT))

	uint32_t unused1[12];
	uint32_t debug_permission;	/* 0xc0 */
	uint32_t debug_en;	/* 0xc4 */
} ArmAipCtrl;
#define ARMAIPCTRL ((volatile ArmAipCtrl * const) AIP_BASE)

/*
 * ARM CFG
 */
typedef struct ArmProcClkMgr {
	uint32_t wr_access;	/* 0x00 */
#define ARM_PROC_CLK_WR_ACCESS_PASSWORD_SHIFT         8
#define ARM_PROC_CLK_WR_ACCESS_PASSWORD_MASK          (0xffff<<ARM_PROC_CLK_WR_ACCESS_PASSWORD_SHIFT)
#define ARM_PROC_CLK_WR_ACCESS_PASSWORD               0xA5A5
#define ARM_PROC_CLK_WR_ACCESS_CLKMGR_ACC_SHIFT       0
#define ARM_PROC_CLK_WR_ACCESS_CLKMGR_ACC_MASK        (0x1<<ARM_PROC_CLK_WR_ACCESS_CLKMGR_ACC_SHIFT)
#define ARM_PROC_CLK_WR_ACCESS_CLKMGR_ACC             0x1
	uint32_t unused0;
	uint32_t policy_freq;
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
	uint32_t policy_ctl;
#define ARM_PROC_CLK_POLICY_CTL_GO_ATL                0x4
#define ARM_PROC_CLK_POLICY_CTL_GO_AC                 0x2
#define ARM_PROC_CLK_POLICY_CTL_GO                    0x1
	uint32_t policy0_mask;	/* 0x10 */
	uint32_t policy1_mask;
	uint32_t policy2_mask;
	uint32_t policy3_mask;
	uint32_t int_en;	/* 0x20 */
	uint32_t int_stat;
	uint32_t unused1[3];
	uint32_t lvm_en;	/* 0x34 */
	uint32_t lvm0_3;
	uint32_t lvm4_7;
	uint32_t vlt0_3;	/* 0x40 */
	uint32_t vlt4_7;
	uint32_t unused2[46];
	uint32_t bus_quiesc;	/* 0x100 */
	uint32_t unused3[63];
	uint32_t core0_clkgate;	/* 0x200 */
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
	uint32_t core1_clkgate;
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
	uint32_t unused4[2];
	uint32_t arm_switch_clkgate;	/* 0x210 */
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
	uint32_t unused5[59];
	uint32_t arm_periph_clkgate;	/* 0x300 */
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
	uint32_t unused6[63];
	uint32_t apb0_clkgate;	/* 0x400 */
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
	uint32_t unused7[383];
	uint32_t pl310_div;	/* 0xa00 */
	uint32_t pl310_trigger;
	uint32_t arm_switch_div;
	uint32_t arm_switch_trigger;
	uint32_t apb_div;	/* 0xa10 */
	uint32_t apb_trigger;
	uint32_t unused8[122];
	uint32_t pllarma;	/* 0xc00 */
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
	uint32_t pllarmb;
#define ARM_PROC_CLK_PLLARMB_NDIV_FRAC_SHIFT         0
#define ARM_PROC_CLK_PLLARMB_NDIV_FRAC_MASK         (0xfffff<<ARM_PROC_CLK_PLLARMB_NDIV_FRAC_SHIFT)
	uint32_t pllarmc;
#define ARM_PROC_CLK_PLLARMC_MDIV_SHIFT              0
#define ARM_PROC_CLK_PLLARMC_MDIV_MASK               (0xff<<ARM_PROC_CLK_PLLARMC_MDIV_SHIFT)
	uint32_t pllarmctrl0;
	uint32_t pllarmctrl1;	/* 0xc10 */
	uint32_t pllarmctrl2;
	uint32_t pllarmctrl3;
	uint32_t pllarmctrl4;
	uint32_t pllarmctrl5;	/* 0xc20 */
	uint32_t pllarm_offset;
	uint32_t unused9[118];
	uint32_t arm_div;	/* 0xe00 */
	uint32_t arm_seg_trg;
	uint32_t arm_seg_trg_override;
	uint32_t unused10;
	uint32_t pll_debug;	/* 0xe10 */
	uint32_t unused11[3];
	uint32_t activity_mon1;	/* 0xe20 */
	uint32_t activity_mon2;
	uint32_t unused12[6];
	uint32_t clkgate_dbg;	/* 0xe40 */
	uint32_t unused13;
	uint32_t apb_clkgate_dbg1;
	uint32_t unused14[6];
	uint32_t clkmon;	/* 0xe64 */
	uint32_t unused15[10];
	uint32_t kproc_ccu_prof_ctl;	/* 0xe90 */
	uint32_t kproc_cpu_proc_sel;
	uint32_t kproc_cpu_proc_cnt;
	uint32_t kproc_cpu_proc_dbg;
	uint32_t unused16[8];
	uint32_t policy_dbg;	/* 0xec0 */
	uint32_t tgtmask_dbg1;
} ArmProcClkMgr;

typedef struct ArmProcResetMgr {
	uint32_t wr_access;	/* 0x00 */
	uint32_t soft_rstn;
	uint32_t a9_core_soft_rstn;
} ArmProcResetMgr;

typedef struct ArmCfg {
	ArmProcClkMgr proc_clk;	/* 0x00 */
	uint32_t unused0[14];	/* 0xec8 - 0xeff */
	ArmProcResetMgr proc_reset;	/* 0xf00 */
	//uint32_t unused0[61];      /* 0xf0c - 0xfff */
} ArmCfg;
#define ARMCFG ((volatile ArmCfg * const) ARMCFG_BASE)

#endif
