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
#ifndef BPCM_H
#define BPCM_H

#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef _TYPEDEFS_H_
#ifndef _UINT32_T_DECLARED
typedef unsigned int uint32_t;
#define _UINT32_T_DECLARED
#endif
#ifndef _UINT64_T_DECLARED
typedef unsigned long long uint64_t;
#define _UINT64_T_DECLARED
#endif
#endif

typedef union {
	struct {
		uint32_t pmbAddr:12;	// [11:00]
		uint32_t map_rev:4;	// [15:12] always non-zero for AVS3 devices (see CLASSIC_BPCM_ID_REG below)
		uint32_t island:4;	// [19:16]
		uint32_t devType:4;	// [23:20] see enum above
		uint32_t hw_rev:8;	// [31:24]
	} Bits;
	uint32_t Reg32;
} BPCM_ID_REG;

// types of PMB devices
enum {
	kPMB_BPCM = 0,
	kPMB_MIPS_PLL = 1,
	kPMB_GEN_PLL = 2,
	kPMB_LC_PLL = 3,
	// 4..15 reserved
};

typedef union {
	struct {
		uint32_t num_zones:6;
		uint32_t reserved0:2;
		uint32_t num_sr_bits:8;
		uint32_t devType:4;	// see enum above
		uint32_t reserved1:12;
	} Bits;
	uint32_t Reg32;
} BPCM_CAPABILITES_REG;

typedef union {
	struct {
		uint32_t pwd_alert:1;
		uint32_t reserved:31;
	} Bits;
	uint32_t Reg32;
} BPCM_STATUS_REG;

typedef union {
	struct {
		uint32_t ro_en_s:1;
		uint32_t ro_en_h:1;
		uint32_t ectr_en_s:1;
		uint32_t ectr_en_h:1;
		uint32_t thresh_en_s:1;
		uint32_t thresh_en_h:1;
		uint32_t continuous_s:1;
		uint32_t continuous_h:1;
		uint32_t reserved:4;
		uint32_t valid_s:1;
		uint32_t alert_s:1;
		uint32_t valid_h:1;
		uint32_t alert_h:1;
		uint32_t interval:16;
	} Bits;
	uint32_t Reg32;
} BPCM_AVS_ROSC_CONTROL_REG;

typedef union {
	struct {
		uint32_t thresh_lo:16;
		uint32_t thresh_hi:16;
	} Bits;
	uint32_t Reg32;
} BPCM_AVS_ROSC_THRESHOLD;

typedef union {
	struct {
		uint32_t count_s:16;
		uint32_t count_h:16;
	} Bits;
	uint32_t Reg32;
} BPCM_AVS_ROSC_COUNT;

typedef union {
	struct {
		uint32_t pwd_en:1;
		uint32_t pwd_alert_sel:1;
		uint32_t start:6;
		uint32_t pwd_tm_en:1;
		uint32_t reserved2:6;
		uint32_t alert:1;
		uint32_t ccfg:8;
		uint32_t rsel:3;
		uint32_t clr_cfg:3;
		uint32_t reserved1:2;
	} Bits;
	uint32_t Reg32;
} BPCM_AVS_PWD_CONTROL;

typedef union {
	struct {
		uint32_t tbd:32;
	} Bits;
	uint32_t Reg32;
} BPCM_PWD_ACCUM_CONTROL;

typedef union {
	struct {
		uint32_t sr:8;
		uint32_t gp:24;
	} Bits;
	uint32_t Reg32;
} BPCM_SR_CONTROL;

typedef union{
	struct {
		uint32_t tbd:32;
	} Bits;
	uint32_t Reg32;
	struct {
		uint32_t vdsl_arm_por_reset_n:1;
		uint32_t vdsl_arm_reset_n:1;
		uint32_t vdsl_arm_debug_reset_n:1;
		uint32_t vdsl_arm_l2_reset_n:1;
		uint32_t vdsl_arm_cdbgrstreq_en:1;
		uint32_t vdsl_arm_niden_a7_b0:1;
		uint32_t vdsl_arm_spniden_a7_b0:1;
		uint32_t vdsl_arm_nsocdbgreset_a7:1;
		uint32_t axi4_ubus4_pass_through_disable:1;
		uint32_t vdsl_arm_dbgen_a7_b0:1;
		uint32_t vdsl_arm_spiden_a7_b0:1;
		uint32_t vdsl_arm_scratch_reg:21;
	} Bits_vdsl;
} BPCM_VDSL_ARM_RST_CTL;

typedef union {
	struct {
		uint32_t tbd:32;
	} Bits;
	uint32_t Reg32;
} BPCM_GLOBAL_CNTL;

typedef union {
	struct {
		uint32_t ctl;
	} Bits_sata_gp;
	struct {
		uint32_t iddq_bias:1;	/* 0 */
		uint32_t ext_pwr_down:4;	/* 1-4 */
		uint32_t force_dll_en:1;	/* 5 */
		uint32_t iddq_global_pwr:1;	/* 6 */
		uint32_t reserved:25;
	} Bits_switch_z1_qgphy;
	struct {
		uint32_t iddq_bias:1;	/* 0 */
		uint32_t ext_pwr_down:1;	/* 1 */
		uint32_t force_dll_en:1;	/* 2 */
		uint32_t iddq_global_pwd:1;	/* 3 */
		uint32_t ck25_dis:1;	/* 4 */
		uint32_t phy_reset:1;	/* 5 */
		uint32_t reserved0:2;
		uint32_t phy_ad:5;	/* 8-12 */
		uint32_t reserved1:18;
		uint32_t ctrl_en:1;	/* 31 */
	} Bits_egphy_1port;
	struct {
		uint32_t iddq_bias:1;	/* 0 */
		uint32_t ext_pwr_down:4;	/* 1-4 */
		uint32_t force_dll_en:1;	/* 5 */
		uint32_t iddq_global_pwd:1;	/* 6 */
		uint32_t ck25_dis:1;	/* 7 */
		uint32_t phy_reset:1;	/* 8 */
		uint32_t reserved0:3;
		uint32_t phy_ad:5;	/* 12-16 */
		uint32_t reserved1:14;
		uint32_t ctrl_en:1;	/* 31 */
	} Bits_egphy_4port;
	struct {
		uint32_t iddq_bias:1;	/* 0 */
		uint32_t ext_pwr_down:4;	/* 1-4 */
		uint32_t force_dll_en:1;	/* 5 */
		uint32_t iddq_global_pwr:1;	/* 6 */
		uint32_t reserved0:25;	/* 7-31 */
	} Bits_qgphy_cntl;
	struct {
		uint32_t ctl;
	} Bits_vdsl_phy;
	struct {
		uint32_t alt_bfc_vector:12;	/* 00-11 */
		uint32_t reserved0:3;
		uint32_t alt_bfc_en:1;	/* 15 */
		uint32_t reset_dly_cfg:2;	/* 16-17 */
		uint32_t reserved1:8;
		uint32_t ext_mclk_en_reset:1;	/* 26 */
		uint32_t ext_mclk_en:1;	/* 27 */
		uint32_t por_reset_n_ctl:1;	/* 28 */
		uint32_t reset_n_ctl:1;	/* 29 */
		uint32_t reserved2:1;
		uint32_t clken:1;	/* 31 */
	} Bits_vdsl_mips;
	uint32_t Reg32;
} BPCM_MISC_CONTROL;

typedef union {
	struct {
		uint32_t field;
	} Bits_qgphy_status;
	struct {
		uint32_t alt_bfc_vector:12;	/* 00-11 */
		uint32_t reserved0:3;
		uint32_t alt_bfc_en:1;	/* 15 */
		uint32_t reset_dly_cfg:2;	/* 16-17 */
		uint32_t reserved1:8;
		uint32_t ext_mclk_en_reset:1;	/* 26 */
		uint32_t ext_mclk_en:1;	/* 27 */
		uint32_t por_reset_n_ctl:1;	/* 28 */
		uint32_t reset_n_ctl:1;	/* 29 */
		uint32_t reserved2:1;
		uint32_t clken:1;	/* 31 */
	} Bits_vdsl_mips;	/* second PHY MIPS core */
	uint32_t Reg32;
} BPCM_MISC_CONTROL2;

typedef union {
	struct {
		uint32_t gphy_iddq_bias:1;	/* 00 */
		uint32_t gphy_ext_pwr_down:1;	/* 01 */
		uint32_t gphy_force_dll_en:1;	/* 02 */
		uint32_t gphy_iddq_global_pwr:1;	/* 03 */
		uint32_t serdes_iddq:1;	/* 04 */
		uint32_t serdes_pwrdwn:1;	/* 05 */
		uint32_t reserved0:2;	/* 07:06 */
		uint32_t serdes_refclk_sel:3;	/* 10:08 */
		uint32_t reserved1:5;	/* 15:11 */
		uint32_t pll_clk125_250_sel:1;	/* 16 */
		uint32_t pll_mux_clk_250_sel:1;	/* 17 */
		uint32_t reserved2:14;	/* 31:18 */
	} Bits;
	uint32_t Reg32;
} BPCM_SGPHY_CNTL;

typedef union {
	struct {
		uint32_t field;
	} Bits;
	uint32_t Reg32;
} BPCM_SGPHY_STATUS;

typedef union {
	struct {
		uint32_t cpu_reset_n:8;	// 07:00    R/W
		uint32_t c0l2_reset:1;	// 08:08    R/W
		uint32_t c1l2_reset:1;	// 09:09    R/W
		uint32_t reserved0:6;	// 15:10    R/O
		uint32_t cpu_bpcm_init_on:8;	// 23:16    R/W
		uint32_t c0l2_bpcm_init_on:1;	// 24:24    R/W
		uint32_t c1l2_bpcm_init_on:1;	// 25:25    R/W
		uint32_t ubus_sr:1;	// 26:26    R/W
		uint32_t cci_sr:1;	// 27:27    R/W
		uint32_t webcores_sr:1;	// 28:28    R/W
		uint32_t hw_done:1;	// 29:29    R/O
		uint32_t sw_done:1;	// 30:30    R/W
		uint32_t start:1;	// 31:31    R/W
	} Bits;
	uint32_t Reg32;
} ARM_CONTROL_REG;

typedef union {
	struct {
		uint32_t mem_pwr_ok:1;	// 00:00    R/W
		uint32_t mem_pwr_on:1;	// 01:01    R/W
		uint32_t mem_clamp_on:1;	// 02:02    R/W
		uint32_t reserved2:1;	// 03:03    R/W
		uint32_t mem_pwr_ok_status:1;	// 04:04    R/O
		uint32_t mem_pwr_on_status:1;	// 05:05    R/O
		uint32_t reserved1:2;	// 07:06    R/W
		uint32_t mem_pda:4;	// 11:08    R/W only LS bit for CPU0/1, all four bits for neon_l2
		uint32_t reserved0:3;	// 14:12    R/W
		uint32_t clamp_on:1;	// 15:15    R/W
		uint32_t pwr_ok:4;	// 19:16    R/W ditto
		uint32_t pwr_on:4;	// 23:20    R/W ditto
		uint32_t pwr_ok_status:4;	// 27:24    R/O ditto
		uint32_t pwr_on_status:4;	// 31:28    R/O only LS 2-bits for CPU1, only LS 1 bit for neon_l2
	} Bits;
	uint32_t Reg32;
} ARM_CPUx_PWR_CTRL_REG;

typedef union {
	struct {
		uint32_t resetb:1;	// 00:00
		uint32_t post_resetb:1;	// 01:01
		uint32_t pwrdwn:1;	// 02:02
		uint32_t master_reset:1;	// 03:03
		uint32_t pwrdwn_ldo:1;	// 04:04
		uint32_t iso:1;	// 05:05 // only used in afepll
		uint32_t reserved0:2;	// 07:06
		uint32_t ldo_ctrl:6;	// 13:08
		uint32_t reserved1:1;	// 14:14
		uint32_t hold_ch_all:1;	// 15:15
		uint32_t reserved2:4;	// 16:19
		uint32_t byp_wait:1;	// 20:20 // only used in b15pll
		uint32_t reserved3:11;	// 21:31
	} Bits;
	uint32_t Reg32;
} PLL_CTRL_REG;

typedef union {
	struct {
		uint32_t fb_offset:12;	// 11:00
		uint32_t fb_phase_en:1;	// 12:12
		uint32_t _8phase_en:1;	// 13:13
		uint32_t sr:18;	// 31:14
	} Bits;
	uint32_t Reg32;
} PLL_PHASE_REG;

typedef union {
	struct {
		uint32_t ndiv_int:10;		// 09:00
		uint32_t reserved0:2;		// 11:10
		uint32_t fref_eff_info:8;	// 19:12
		uint32_t auto_cnfg_dsbl:8;	// 27:20
		uint32_t reserved1:3;		// 30:28
		uint32_t ndiv_override:1;	// 31
	} Bits;
	uint32_t Reg32;
} PLL_NDIV_REG;

typedef union {
	struct {
		uint32_t pdiv:4;	// 03:00
		uint32_t ndiv_frac:24;	// 27:04
		uint32_t reserved0:3;	// 30:28
		uint32_t ndiv_pdiv_override:1;	// 31:31
	} Bits;
	uint32_t Reg32;
} PLL_PDIV_REG;

typedef union {
	struct {
		uint32_t mdiv0:9;	// 08:00  
		uint32_t enableb_ch0:1;	// 09:09
		uint32_t hold_ch0:1;	// 10:10
		uint32_t mdel0:1;	// 11:11
		uint32_t reserved0:3;	// 14:12
		uint32_t mdiv_override0:1;	// 15:15
		uint32_t mdiv1:9;	// 24:16
		uint32_t enableb_ch1:1;	// 25:25
		uint32_t hold_ch1:1;	// 26:26
		uint32_t mdel1:1;	// 27:27
		uint32_t reserved1:3;	// 30:28
		uint32_t mdiv_override1:1;	// 31:31
	} Bits;
	uint32_t Reg32;
} PLL_CHCFG_REG;

typedef union {
	struct {
		uint32_t reserved0:8;	// 07:00
		uint32_t stat_select:3;	// 10:08
		uint32_t pq_mode:1;	// 11:11
		uint32_t stat_mode:2;	// 13:12
		uint32_t stat_reset:1;	// 14:14  
		uint32_t stat_update:1;	// 15:15  
		uint32_t ssc_step:16;	// 31:16
	} Bits;
	uint32_t Reg32;
} PLL_LOOP0_REG;

typedef union {
	struct {
		uint32_t ssc_limit:22;	// 21:00
		uint32_t reserved0:2;	// 23:22
		uint32_t ssc_clkdiv:4;	// 27:24
		uint32_t ssc_status:1;	// 28:28
		uint32_t reserved1:2;	// 30:29
		uint32_t ssc_mode:1;	// 31:31
	} Bits;
	uint32_t Reg32;
} PLL_LOOP1_REG;

typedef union {
	struct {
		uint32_t fdco_ctrl_bypass:16;	// 15:00
		uint32_t fdco_bypass_en:1;	// 16:16
		uint32_t fdco_dac_sel:1;	// 17:17
		uint32_t state_reset:1;	// 18:18
		uint32_t state_mode:2;	// 20:19
		uint32_t state_sel:3;	// 23:21
		uint32_t state_update:1;	// 24:24
		uint32_t dco_en:1;	// 25:25
		uint32_t dco_div2_div4:1;	// 26:26
		uint32_t dco_bias_boost:1;	// 27:27
		uint32_t bb_en:1;	// 28:28
		uint32_t t2d_offset:3;	// 31:29
	} Bits;
	uint32_t Reg32;
} PLL_CFG0_REG;

typedef union {
	struct {
		uint32_t t2d_offset_msb:1;	// 00:00
		uint32_t t2d_clk_enable:1;	// 01:01
		uint32_t t2d_clk_sel:1;	// 02:02
		uint32_t kpp:4;	// 06:03
		uint32_t pwm_ctrl:2;	// 08:07
		uint32_t port_reset_mode:2;	// 10:09
		uint32_t byp2_en:1;	// 11:11
		uint32_t byp1_en:1;	// 12:12
		uint32_t ref_diff_sel:1;	// 13:13
		uint32_t ki_startlow:1;	// 14:14
		uint32_t en_500ohm:1;	// 15:15
		uint32_t refd2c_bias:3;	// 18:16
		uint32_t post_div2_div3:1;	// 19:19
		uint32_t ki_boost:1;	// 20:20
		uint32_t reserved0:11;	// 31:21
	} Bits;
	uint32_t Reg32;
} PLL_CFG1_REG;

typedef union {
	struct {
		uint32_t en_cml:3;	// 02:00
		uint32_t tri_en:1;	// 03:03
		uint32_t test_sel:3;	// 06:04
		uint32_t test_en:1;	// 07:07
		uint32_t reserved0:24;
	} Bits;
	uint32_t Reg32;
} PLL_OCTRL_REG;

typedef union {
	struct {
		uint32_t out:12;	// 11:00
		uint32_t reserved:19;	// 30:12
		uint32_t lock:1;	// 31:31
	} Bits;
	uint32_t Reg32;
} PLL_STAT_REG;

typedef union {
	struct {
		uint32_t ndiv_int:10;	// 9:0	  
		uint32_t ndiv_frac:22;	// 31:10
	} Bits;
	uint32_t Reg32;
} PLL_DECNDIV_REG;

typedef union {
	struct {
		uint32_t pdiv:4;	// 03:00
		uint32_t mdiv5:9;	// 12:04
		uint32_t mdiv0:9;	// 21:13
		uint32_t mdiv1:9;	// 30:22
		uint32_t reserved0:1;	// 31:31
	} Bits;
	uint32_t Reg32;
} PLL_DECPDIV_REG;

typedef union {
	struct {
		uint32_t mdiv2:9;	// 08:00
		uint32_t mdiv3:9;	// 17:09
		uint32_t mdiv4:9;	// 26:18
		uint32_t reserved0:5;	// 31:27
	} Bits;
	uint32_t Reg32;
} PLL_DECCH25_REG;

typedef union {
	struct {
		uint32_t manual_clk_en:1;
		uint32_t manual_reset_ctl:1;
		uint32_t freq_scale_used:1;	// R/O
		uint32_t dpg_capable:1;	// R/O
		uint32_t manual_mem_pwr:2;
		uint32_t manual_iso_ctl:1;
		uint32_t manual_ctl:1;
		uint32_t dpg_ctl_en:1;
		uint32_t pwr_dn_req:1;
		uint32_t pwr_up_req:1;
		uint32_t mem_pwr_ctl_en:1;
		uint32_t blk_reset_assert:1;
		uint32_t mem_stby:1;
		uint32_t reserved:5;
		uint32_t pwr_cntl_state:5;
		uint32_t freq_scalar_dyn_sel:1;	// R/O
		uint32_t pwr_off_state:1;	// R/O
		uint32_t pwr_on_state:1;	// R/O
		uint32_t pwr_good:1;	// R/O
		uint32_t dpg_pwr_state:1;	// R/O
		uint32_t mem_pwr_state:1;	// R/O
		uint32_t iso_state:1;	// R/O
		uint32_t reset_state:1;	// R/O
	} Bits;
	uint32_t Reg32;
} BPCM_PWR_ZONE_N_CONTROL;

typedef union {
	struct {
		uint32_t pwr_ok_delay_sel:3;
		uint32_t pwk_ok_thresh:2;
		uint32_t reserved:3;
		uint32_t iso_on_delay:4;
		uint32_t iso_off_delay:4;
		uint32_t clock_on_delay:4;
		uint32_t clock_off_delay:4;
		uint32_t reset_on_delay:4;
		uint32_t reset_off_delay:4;
	} Bits;
	uint32_t Reg32;
} BPCM_PWR_ZONE_N_CONFIG1;

typedef union {
	struct {
		uint32_t delay_prescale_sel:3;
		uint32_t slew_prescale_sel:3;
		uint32_t reserved:6;
		uint32_t dpgn_on_delay:4;
		uint32_t dpg1_on_delay:4;
		uint32_t dpg_off_delay:4;
		uint32_t mem_on_delay:4;
		uint32_t mem_off_delay:4;
	} Bits;
	uint32_t Reg32;
} BPCM_PWR_ZONE_N_CONFIG2;

typedef union {
	struct {
		uint32_t fs_bypass_en:1;
		uint32_t gear_sel:1;
		uint32_t use_dyn_gear_sel:1;
		uint32_t reserved2:1;
		uint32_t low_gear_div:3;
		uint32_t high_gear_div:3;
		uint32_t reserved:22;
	} Bits;
	uint32_t Reg32;
} BPCM_ZONE_N_FREQ_SCALAR_CONTROL;

typedef struct {
	BPCM_PWR_ZONE_N_CONTROL control;
	BPCM_PWR_ZONE_N_CONFIG1 config1;
	BPCM_PWR_ZONE_N_CONFIG2 config2;
	uint32_t reserved0;
	uint32_t timer_control;
	uint32_t timer_status;
	uint32_t reserved1[2];
} BPCM_ZONE;

#define BPCMZoneOffset(reg)	offsetof(BPCM_ZONE,reg)
#define BPCMZoneRegOffset(reg)	(BPCMZoneOffset(reg) >> 2)

typedef union {
	struct {
		uint32_t pmb_Addr:8;
		uint32_t hw_rev:8;
		uint32_t module_id:16;
	} Bits;
	uint32_t Reg32;
} BPCM_UBUS_ID_REG;

typedef union {
	struct {
		uint32_t num_zones:8;
		uint32_t sr_reg_bits:8;
		uint32_t pllType:2;
		uint32_t reserved0:1;
		uint32_t ubus:1;
		uint32_t reserved1:12;
	} Bits;
	uint32_t Reg32;
} BPCM_UBUS_CAPABILITES_REG;

typedef union {
	struct {
		uint32_t ctrl_eswap:4;
		uint32_t reserved0:4;
		uint32_t ctrl_cd:4;
		uint32_t reserved1:4;
		uint32_t ctrl_seclev:8;
		uint32_t reqout_seclev:8;
	} Bits;
	uint32_t Reg32;
} BPCM_UBUS_CTRL_REG;

typedef union {
	struct {
		uint64_t addr_in:24;
		uint64_t addr_out:24;
		uint64_t pid:8;
		uint64_t size:5;
		uint64_t cmddta:1;
		uint64_t en:2;
	} Bits;
	struct {
		uint32_t word0;
		uint32_t word1;
	} Regs32;
	uint64_t Reg64;
} BPCM_UBUS_CFG_REG;

typedef union {
	struct {
		uint32_t ubus_soft_reset:1;
		uint32_t alt_ubus_clk_sel:1;
		uint32_t observe_clk_sw_init:1;
		uint32_t alt_emmc_clk_sel:1;
		uint32_t reserved:5;
		uint32_t enable:1;
		uint32_t counter:8;
		uint32_t reserved2:14;
	} Bits;
	uint32_t Reg32;
} BPCM_CLKRST_VREG_CONTROL;

// There is a 20-bit address used to access any given BPCM register.  The upper 8-bits
// is the device address and the lower 12-bits is used to represent the BPCM register
// set for that device.  32-bit registers are allocated on 4-byte boundaries
// (i.e. 0, 1, 2, 3...) rather than on byte boundaries (0x00, 0x04, 0x08, 0x0c...)
// Thus, to get the actual address of any given register within the device's address
// space, I'll use the "C" offsetof macro and divide the result by 4
// e.g.:
// int regOffset = offsetof(BPCM_REGS,BPCM_AVS_PWD_CONTROL);    // yields the byte offset of the target register
// int regAddress = regOffset/4;                                // yields the 32-bit word offset of the target register
// The ReadBPCMReg and WriteBPCMReg functions will always take a device address
// (address of the BPCM device) and register offset (like regOffset above).  The offset
// will be divided by 4 and used as the lower 12-bits of the actual target address, while the
// device address will serve as the upper 8-bits of the actual address.
typedef struct {
	// PMB-slave:
	BPCM_ID_REG id_reg;	// offset 0x00, PMB reg index 0
	BPCM_CAPABILITES_REG capabilities;	// offset 0x04, PMB reg index 1
	uint32_t reserved0[2];	// offset 0x08, PMB reg index 2/3
	// BPCM
	uint32_t control;	// offset 0x10, PMB reg index 4
	BPCM_SR_CONTROL sr_control;	// offset 0x14, PMB reg index 5
	uint32_t reserved1[2];	// offset 0x18, PMB reg index 6/7
	// Client-specific registers
	uint32_t client_specific[24];	// offset 0x20, PMB reg index 8..31
	// Zones
	BPCM_ZONE zones[];	// offset 0x80..(0x20 + MAX_ZONES*32)), PMB reg index 32..(32+(MAX_ZONES*8-1))
} BPCM_REGS;			// total offset space = 4096

#define BPCMOffset(reg)		offsetof(BPCM_REGS,reg)
#define BPCMRegOffset(reg)	(BPCMOffset(reg) >> 2)

typedef struct {
	BPCM_ID_REG id_reg;	// offset = 0x00, actual offset = 0
	BPCM_CAPABILITES_REG capabilities;	// offset = 0x04, actual offset = 1
	uint32_t control;	// offset = 0x08, actual offset = 2
	BPCM_STATUS_REG status;	// offset = 0x0c, actual offset = 3
	BPCM_AVS_ROSC_CONTROL_REG rosc_control;	// offset = 0x10, actual offset = 4
	BPCM_AVS_ROSC_THRESHOLD rosc_thresh_h;	// offset = 0x14, actual offset = 5
	BPCM_AVS_ROSC_THRESHOLD rosc_thresh_s;	// offset = 0x18, actual offset = 6
	BPCM_AVS_ROSC_COUNT rosc_count;	// offset = 0x1c, actual offset = 7
	BPCM_AVS_PWD_CONTROL pwd_control;	// offset = 0x20, actual offset = 8
	BPCM_PWD_ACCUM_CONTROL pwd_accum_control;	// offset = 0x24, actual offset = 9
	BPCM_SR_CONTROL sr_control;	// offset = 0x28, actual offset = 10
	BPCM_GLOBAL_CNTL global_control;	// offset = 0x2c, actual offset = 11
	BPCM_MISC_CONTROL misc_control;	// offset = 0x30, actual offset = 12
	BPCM_MISC_CONTROL2 misc_control2;	// offset = 0x34, actual offset = 13
	uint32_t rvrsd[2];
	BPCM_ZONE zones[1020];	// offset = 0x40..0x3FFC, actual offset = 16..4095 (1020 * 4 = 4080 + 16 = 4096)
} BPCM_VDSL_REGS;		// total offset space = 4096

#define BPCMVDSLOffset(reg)	offsetof(BPCM_VDSL_REGS,reg)
#define BPCMVDSLRegOffset(reg)	(BPCMVDSLOffset(reg) >> 2)

typedef struct {
	BPCM_ID_REG id_reg;	// offset = 0x00, actual offset = 0
	BPCM_CAPABILITES_REG capabilities;	// offset = 0x04, actual offset = 1
	uint32_t reserved0[2];	// offset = 0x08, actual offset = 2
	uint32_t cfg_control;	// offset = 0x10, actual offset = 4
	BPCM_SR_CONTROL sr_control;	// offset = 0x14, actual offset = 5
	uint32_t reserved1[2];	// offset = 0x18, actual offset = 6
	BPCM_ZONE zones;	// offset = 0x20, actual offset = 8
	ARM_CONTROL_REG arm_control;	// offset = 0x40, actual offset = 16
} ARM_BPCM_REGS;
#define ARMBPCMOffset(reg)  offsetof(ARM_BPCM_REGS,reg)
#define ARMBPCMRegOffset(reg)   (ARMBPCMOffset(reg) >> 2)

typedef struct {
	/* BIU PLL BCPM definition */  
	BPCM_ID_REG id_reg;	// offset = 0x00 
	BPCM_CAPABILITES_REG capabilities;	// offset = 0x04
	uint32_t reserved0[2];	// offset = 0x08..0x0c
	PLL_CTRL_REG resets;	// offset = 0x10
	uint32_t reserved1[2];	// offset = 0x14
	PLL_NDIV_REG ndiv;	// offset = 0x1c
	PLL_PDIV_REG pdiv;	// offset = 0x20
	PLL_LOOP0_REG loop0;	// offset = 0x24
	PLL_LOOP1_REG loop1;	// offset = 0x28
	PLL_CHCFG_REG ch01_cfg;	// offset = 0x2c
	PLL_CHCFG_REG ch23_cfg;	// offset = 0x30
	PLL_CHCFG_REG ch45_cfg;	// offset = 0x34
	uint32_t reserved2;	// offset = 0x38
	PLL_STAT_REG stat;	// offset = 0x3c
	uint32_t strap;		// offset = 0x40
	PLL_DECNDIV_REG decndiv;// offset = 0x44
	PLL_DECPDIV_REG decpdiv;// offset = 0x48
	PLL_DECCH25_REG decch25;// offset = 0x4c
} PLL_BPCM_REGS;

#define PLLBPCMOffset(reg)	offsetof(PLL_BPCM_REGS,reg)
#define PLLBPCMRegOffset(reg)	(PLLBPCMOffset(reg) >> 2)

typedef struct {
	BPCM_ID_REG id_reg;	// offset = 0x00
	BPCM_CAPABILITES_REG capabilities;	// offset = 0x04
	uint32_t reserved0[2];	// offset = 0x08..0x0c
	PLL_CTRL_REG resets;	// offset = 0x10
	uint32_t reserved1[5];	// offset = 0x14
	PLL_NDIV_REG ndiv;	// offset = 0x28
	PLL_PDIV_REG pdiv;	// offset = 0x2c
	PLL_CHCFG_REG ch01_cfg;	// offset = 0x30
	PLL_CHCFG_REG ch23_cfg;	// offset = 0x34
	PLL_CHCFG_REG ch45_cfg;	// offset = 0x38
	PLL_STAT_REG stat;	// offset = 0x3c
	uint32_t strap;		// offset = 0x40
	PLL_DECNDIV_REG decndiv;// offset = 0x44
	PLL_DECPDIV_REG decpdiv;// offset = 0x48
	PLL_DECCH25_REG decch25;// offset = 0x4c
} RDPPLL_BPCM_REGS;

#define RDPPLLBPCMOffset(reg)		offsetof(RDPPLL_BPCM_REGS,reg)
#define RDPPLLBPCMRegOffset(reg)	(RDPPLLBPCMOffset(reg) >> 2)

typedef struct {
	BPCM_UBUS_ID_REG id_reg;	/* offset = 0x00, actual offset = 0 */
	BPCM_UBUS_CAPABILITES_REG capabilities;	/* offset = 0x04, actual offset = 1 */
	uint32_t reserved0;	/* offset = 0x08, actual offset = 2 */
	BPCM_UBUS_CTRL_REG ctrl;	/* offset = 0x0c, actual offset = 3 */
	BPCM_UBUS_CFG_REG cfg[4];	/* offset = 0x10..0x2c, actual offset = 4..11 */
} BPCM_UBUS_REG;

#define UBUSBPCMOffset(reg)	offsetof(BPCM_UBUS_REG,reg)
#define UBUSBPCMRegOffset(reg)	(UBUSBPCMOffset(reg) >> 2)

typedef struct {
	BPCM_ID_REG id_reg;	// offset = 0x00, actual offset = 0
	BPCM_CAPABILITES_REG capabilities;	// offset = 0x04, actual offset = 1
	uint32_t control;	// offset = 0x08, actual offset = 2
	BPCM_STATUS_REG status;	// offset = 0x0c, actual offset = 3
	BPCM_AVS_ROSC_CONTROL_REG rosc_control;	// offset = 0x10, actual offset = 4
	BPCM_AVS_ROSC_THRESHOLD rosc_thresh_h;	// offset = 0x14, actual offset = 5
	BPCM_AVS_ROSC_THRESHOLD rosc_thresh_s;	// offset = 0x18, actual offset = 6
	BPCM_AVS_ROSC_COUNT rosc_count;	// offset = 0x1c, actual offset = 7
	BPCM_AVS_PWD_CONTROL pwd_control;	// offset = 0x20, actual offset = 8
	uint32_t reserved0;	// offset = 0x24, actual offset = 9
	BPCM_SR_CONTROL sr_control;	// offset = 0x28, actual offset = 10
	uint32_t reserved1;	// offset = 0x2c, actual offset = 11
	uint32_t clkrst_cfg;	// offset = 0x30, actual offset = 12
	BPCM_CLKRST_VREG_CONTROL vreg_control;  // offset = 0x34, actual offset = 13
	uint32_t xtal_control;	// offset = 0x38, actual offset = 14
	uint32_t clkrst_stat;	// offset = 0x3c, actual offset = 15
} BPCM_CLKRST_REGS;

#define CLKRSTBPCMOffset(reg)  offsetof(BPCM_CLKRST_REGS, reg)
#define CLKRSTBPCMRegOffset(reg)   (CLKRSTBPCMOffset(reg) >> 2)

typedef struct {
    // ETH_PMB
	BPCM_ID_REG id_reg;	    // offset 0x00, PMB reg index 0
	BPCM_CAPABILITES_REG capabilities;	// offset 0x04, PMB reg index 1
	uint32_t reserved8[2];	// offset 0x08, PMB reg index 2/3
	// ETH_CFG
	uint32_t dpg_zones;	    // offset 0x10, PMB reg index 4
	uint32_t soft_rst;;	    // offset 0x14, PMB reg index 5
	uint32_t reserved18[2];	// offset 0x18, PMB reg index 6/7
    // ETH_CORE
	uint32_t qphy_cntrl;	// offset 0x20
	uint32_t qphy_status;	// offset 0x24
	uint32_t reserved28[2];
	uint32_t serdes0_cntrl;	// offset 0x30
	uint32_t serdes0_status;// offset 0x34
	uint32_t serdes0_an_st; // offset 0x38
	uint32_t serdes1_cntrl;	// offset 0x3c
	uint32_t serdes1_status;// offset 0x40
	uint32_t serdes1_an_st; // offset 0x44
	uint32_t serdes2_cntrl;	// offset 0x48
	uint32_t serdes2_status;// offset 0x4c
	uint32_t serdes2_an_st; // offset 0x50
	uint32_t rgmii_cntrl;   // offset 0x54
	uint32_t xport0_cntrl;  // offset 0x58
	uint32_t xport1_cntrl;  // offset 0x5c
	uint32_t serdes0_st1;   // offset 0x60
	uint32_t serdes1_st1;   // offset 0x64
	uint32_t serdes2_st1;   // offset 0x68
	uint32_t serdes0_pwr_cntrl;// offset 0x6c
	uint32_t serdes1_pwr_cntrl;// offset 0x70
	uint32_t serdes2_pwr_cntrl;// offset 0x74
} BPCM_ETH_REGS;

#define BPCMETHOffset(reg)		offsetof(BPCM_ETH_REGS,reg)
#define BPCMETHRegOffset(reg)	(BPCMETHOffset(reg) >> 2)


// *************************** macros ******************************
#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((unsigned long) &((TYPE *)0)->MEMBER)
#endif

#endif
