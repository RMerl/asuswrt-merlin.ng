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

#if IS_BCMCHIP(6765)
#include <BPCM_6765.h>
#elif IS_BCMCHIP(6766)
#include <BPCM_6766.h>
#elif IS_BCMCHIP(6764)
#include <BPCM_6764.h>
#else
#if !IS_BCMCHIP(6888)
#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef IS_BCMCHIP
#define IS_BCMCHIP(num) (defined(_BCM9##num##_)||defined(CONFIG_BCM9##num)||\
				defined(CONFIG_BCM##num))
#endif

typedef union {
	struct {
#if IS_BCMCHIP(6846) || IS_BCMCHIP(6856) || IS_BCMCHIP(63178) || IS_BCMCHIP(6878) || IS_BCMCHIP(63146)\
	|| IS_BCMCHIP(4912) || IS_BCMCHIP(6813) || IS_BCMCHIP(6855)
		uint32_t pmbAddr:12;	// [11:00]
		uint32_t map_rev:4;	// [15:12] always non-zero for AVS3 devices (see CLASSIC_BPCM_ID_REG below)
		uint32_t island:4;	// [19:16]
		uint32_t devType:4;	// [23:20] see enum above
		uint32_t hw_rev:8;	// [31:24]
#else
#ifdef PMC_LITTLE_ENDIAN
		uint32_t pmb_Addr:8;
		uint32_t hw_rev:8;
		uint32_t sw_strap:16;
#else
		uint32_t sw_strap:16;
		uint32_t hw_rev:8;
		uint32_t pmb_Addr:8;
#endif
#endif
	} Bits;
	uint32_t Reg32;
} BPCM_ID_REG;

#if IS_BCMCHIP(6846) || IS_BCMCHIP(6856)

typedef union {
	struct {
		uint32_t pmb_Addr:8;	// [07:00]
		uint32_t hw_rev:4;	// [11:08]
		uint32_t map_rev:4;	// [15:12] always zero for classic devices (technically upper 4-bits of hw_rev)
		uint32_t sw_strap:16;	// [31:16]
	} Bits;
	uint32_t Reg32;
} CLASSIC_BPCM_ID_REG;
#endif

#if !IS_BCMCHIP(6878) && !IS_BCMCHIP(6855)
// types of PMB devices
enum {
	kPMB_BPCM = 0,
	kPMB_MIPS_PLL = 1,
	kPMB_GEN_PLL = 2,
	kPMB_LC_PLL = 3,
	// 4..15 reserved
};
#else
enum {
	kPMB_NO_DEVICE = 0,
	kPMB_BPCM = 1,		// in AVS3, this structure does not contain ARS registers (except for "classic" BPCM devices) 
	kPMB_MIPS_PLL = 2,
	kPMB_GEN_PLL = 3,
	kPMB_LC_PLL = 4,
	kPMB_CLKRST = 5,
	kPMB_PVTMON = 6,	// used in in AVS3 when PVT is wrapped in a BPCM structure
	kPMB_TMON_INTERNAL = 7,	// ditto - used when TMON thermistor is on-die
	kPMB_TMON_EXTERNAL = 8,	// ditto - used when thermistor is off-die
	kPMB_ARS = 9,		// AVS Remote Sensors - remote oscillators and Power-Watchdog
	// 10..15 reserved
};
#endif

typedef union {
	struct {
#ifdef PMC_LITTLE_ENDIAN
#if IS_BCMCHIP(63146) || IS_BCMCHIP(4912) || IS_BCMCHIP(6813) 
		uint32_t num_zones:6;
		uint32_t reserved0:2;
#else
		uint32_t num_zones:8;
#endif
		uint32_t num_sr_bits:8;
		uint32_t devType:4;	// see enum above
		uint32_t reserved1:12;
#else
		uint32_t reserved1:12;
		uint32_t devType:4;	// see enum above
		uint32_t num_sr_bits:8;
#if IS_BCMCHIP(63146) || IS_BCMCHIP(4912) || IS_BCMCHIP(6813) 
		uint32_t reserved0:2;
		uint32_t num_zones:6;
#else
		uint32_t num_zones:8;
#endif
#endif
	} Bits;
	uint32_t Reg32;
} BPCM_CAPABILITES_REG;

typedef union {
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32_t pwd_alert:1;
		uint32_t reserved:31;
#else
		uint32_t reserved:31;
		uint32_t pwd_alert:1;
#endif
	} Bits;
	uint32_t Reg32;
} BPCM_STATUS_REG;

typedef union {
	struct {
#ifdef PMC_LITTLE_ENDIAN
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
#else
		uint32_t interval:16;
		uint32_t alert_h:1;
		uint32_t valid_h:1;
		uint32_t alert_s:1;
		uint32_t valid_s:1;
		uint32_t reserved:4;
		uint32_t continuous_h:1;
		uint32_t continuous_s:1;
		uint32_t thresh_en_h:1;
		uint32_t thresh_en_s:1;
		uint32_t ectr_en_h:1;
		uint32_t ectr_en_s:1;
		uint32_t ro_en_h:1;
		uint32_t ro_en_s:1;
#endif
	} Bits;
	uint32_t Reg32;
} BPCM_AVS_ROSC_CONTROL_REG;

typedef union {
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32_t thresh_lo:16;
		uint32_t thresh_hi:16;
#else
		uint32_t thresh_hi:16;
		uint32_t thresh_lo:16;
#endif
	} Bits;
	uint32_t Reg32;
} BPCM_AVS_ROSC_THRESHOLD;

typedef union {
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32_t count_s:16;
		uint32_t count_h:16;
#else
		uint32_t count_h:16;
		uint32_t count_s:16;
#endif
	} Bits;
	uint32_t Reg32;
} BPCM_AVS_ROSC_COUNT;

typedef union {
	struct {
#ifdef PMC_LITTLE_ENDIAN
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
#else
		uint32_t reserved1:2;
		uint32_t clr_cfg:3;
		uint32_t rsel:3;
		uint32_t ccfg:8;
		uint32_t alert:1;
		uint32_t reserved2:6;
		uint32_t pwd_tm_en:1;
		uint32_t start:6;
		uint32_t pwd_alert_sel:1;
		uint32_t pwd_en:1;
#endif
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
#ifdef PMC_LITTLE_ENDIAN
		uint32_t sr:8;
		uint32_t gp:24;
#else
		uint32_t gp:24;
		uint32_t sr:8;
#endif
	} Bits;
#if IS_BCMCHIP(6858)
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32_t wan_main_rst_n:1;
		uint32_t wan_top_bb_rst_n:1;
		uint32_t epon_core_rst_n:1;
		uint32_t epon_rx_rclk16_sw_reset_n:1;
		uint32_t epon_rx_rbc125_sw_reset_n:1;
		uint32_t epon_tx_tclk16_sw_reset_n:1;
		uint32_t epon_tx_clk125_sw_reset_n:1;
		uint32_t gpon_main_rst_n:1;
		uint32_t gpon_rx_rst_n:1;
		uint32_t gpon_tx_rst_n:1;
		uint32_t gpon_8khz_rst_n:1;
		uint32_t ngpon_main_rst_n:1;
		uint32_t ngpon_rx_rst_n:1;
		uint32_t ngpon_tx_rst_n:1;
		uint32_t ngpon_8khz_rst_n:1;
		uint32_t gpon_nco_rst_n:1;
		uint32_t apm_rst_n:1;
		uint32_t reserved:15;
#else
		uint32_t reserved:15;
		uint32_t apm_rst_n:1;
		uint32_t gpon_nco_rst_n:1;
		uint32_t ngpon_8khz_rst_n:1;
		uint32_t ngpon_tx_rst_n:1;
		uint32_t ngpon_rx_rst_n:1;
		uint32_t ngpon_main_rst_n:1;
		uint32_t gpon_8khz_rst_n:1;
		uint32_t gpon_tx_rst_n:1;
		uint32_t gpon_rx_rst_n:1;
		uint32_t gpon_main_rst_n:1;
		uint32_t epon_tx_clk125_sw_reset_n:1;
		uint32_t epon_tx_tclk16_sw_reset_n:1;
		uint32_t epon_rx_rbc125_sw_reset_n:1;
		uint32_t epon_rx_rclk16_sw_reset_n:1;
		uint32_t epon_core_rst_n:1;
		uint32_t wan_top_bb_rst_n:1;
		uint32_t wan_main_rst_n:1;
#endif
	} Bits_Wantop;
#endif
#if IS_BCMCHIP(63158)
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32_t wan_main_rst_n:1;
		uint32_t wan_top_bb_rst_n:1;
		uint32_t rbus_rst_n:1;
		uint32_t reserved1:2;
		uint32_t core_soft_rst_n:1;
		uint32_t i_zn_sw_init:1;
		uint32_t reserved2:1;
		uint32_t epon_main_rst_n:1;
		uint32_t epon_rx_rst_n:1;
		uint32_t epon_tx_rst_n:1;
		uint32_t epon_core_rst_n:1;
		uint32_t ae_rx_rclk16_sw_reset_n:1;
		uint32_t ae_rx_rbc125_sw_reset_n:1;	/* for B0 */
		uint32_t ae_tx_tclk16_sw_reset_n:1;	/* for B0 */
		uint32_t ae_tx_clk125_sw_reset_n:1;
		uint32_t gpon_main_rst_n:1;
		uint32_t gpon_rx_rst_n:1;
		uint32_t gpon_tx_rst_n:1;
		uint32_t gpon_8khz_rst_n:1;
		uint32_t ngpon_main_rst_n:1;
		uint32_t ngpon_rx_rst_n:1;
		uint32_t ngpon_tx_rst_n:1;
		uint32_t ngpon_8khz_rst_n:1;
		uint32_t reserved3:2;
		uint32_t gpon_nco_rst_n:1;
		uint32_t epon_rx_rclk16_sw_reset_n:1;	/* for B0 */
		uint32_t epon_rx_rbc125_sw_reset_n:1;	/* for B0 */
		uint32_t epon_tx_tclk16_sw_reset_n:1;	/* for B0 */
		uint32_t epon_tx_clk125_sw_reset_n:1;	/* for B0 */
		uint32_t reserved4:1;
#else
		uint32_t reserved4:1;
		uint32_t epon_tx_clk125_sw_reset_n:1;	/* for B0 */
		uint32_t epon_tx_tclk16_sw_reset_n:1;	/* for B0 */
		uint32_t epon_rx_rbc125_sw_reset_n:1;	/* for B0 */
		uint32_t epon_rx_rclk16_sw_reset_n:1;	/* for B0 */
		uint32_t gpon_nco_rst_n:1;
		uint32_t reserved3:2;
		uint32_t ngpon_8khz_rst_n:1;
		uint32_t ngpon_tx_rst_n:1;
		uint32_t ngpon_rx_rst_n:1;
		uint32_t ngpon_main_rst_n:1;
		uint32_t gpon_8khz_rst_n:1;
		uint32_t gpon_tx_rst_n:1;
		uint32_t gpon_rx_rst_n:1;
		uint32_t gpon_main_rst_n:1;
		uint32_t ae_tx_clk125_sw_reset_n:1;
		uint32_t ae_tx_tclk16_sw_reset_n:1;	/* for B0 */
		uint32_t ae_rx_rbc125_sw_reset_n:1;	/* for B0 */
		uint32_t ae_rx_rclk16_sw_reset_n:1;
		uint32_t epon_core_rst_n:1;
		uint32_t epon_tx_rst_n:1;
		uint32_t epon_rx_rst_n:1;
		uint32_t epon_main_rst_n:1;
		uint32_t reserved2:1;
		uint32_t i_zn_sw_init:1;
		uint32_t core_soft_rst_n:1;
		uint32_t reserved1:2;
		uint32_t rbus_rst_n:1;
		uint32_t wan_top_bb_rst_n:1;
		uint32_t wan_main_rst_n:1;
#endif
	} Bits_Wantop;
#endif
#if IS_BCMCHIP(63158) || IS_BCMCHIP(63178)
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32_t vdsl_bpcm_early_reset:1;
		uint32_t vdsl_bpcm_reset:1;
		uint32_t mips_ubus_soft_reset_bpcm_reset:1;
		uint32_t qproc_1_bpcm_reset:1;
		uint32_t qproc_2_bpcm_reset:1;
		uint32_t sar_bpcm_soft_reset:1;
		uint32_t vdsl_ubus_soft_bpcm_reset:1;
		uint32_t reserved1:1;
		uint32_t gp:24;
#else
		uint32_t gp:24;
		uint32_t reserved1:1;
		uint32_t vdsl_ubus_soft_bpcm_reset:1;
		uint32_t sar_bpcm_soft_reset:1;
		uint32_t qproc_2_bpcm_reset:1;
		uint32_t qproc_1_bpcm_reset:1;
		uint32_t mips_ubus_soft_reset_bpcm_reset:1;
		uint32_t vdsl_bpcm_reset:1;
		uint32_t vdsl_bpcm_early_reset:1;
#endif
	} Bits_vdsl;
#endif
	uint32_t Reg32;
} BPCM_SR_CONTROL;

typedef union{
	struct {
		uint32_t tbd:32;
	} Bits;
	uint32_t Reg32;
	struct {
#ifdef PMC_LITTLE_ENDIAN
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
#else
		uint32_t vdsl_arm_scratch_reg:21;
		uint32_t vdsl_arm_spiden_a7_b0:1;
		uint32_t vdsl_arm_dbgen_a7_b0:1;
		uint32_t axi4_ubus4_pass_through_disable:1;
		uint32_t vdsl_arm_nsocdbgreset_a7:1;
		uint32_t vdsl_arm_spniden_a7_b0:1;
		uint32_t vdsl_arm_niden_a7_b0:1;
		uint32_t vdsl_arm_cdbgrstreq_en:1;
		uint32_t vdsl_arm_l2_reset_n:1;
		uint32_t vdsl_arm_debug_reset_n:1;
		uint32_t vdsl_arm_reset_n:1;
		uint32_t vdsl_arm_por_reset_n:1;
#endif
	} Bits_vdsl;
} BPCM_VDSL_ARM_RST_CTL;

#if IS_BCMCHIP(63138) || IS_BCMCHIP(4908) || IS_BCMCHIP(63158)
typedef union {
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32_t z2_p_wan_phy_sel:3;	/* 0-2 */
		uint32_t reserved0:1;	/* 3 */
		uint32_t z2_switch_p3_phy_sel:3;	/* 4-6 */
		uint32_t reserved1:1;	/* 7 */
		uint32_t z2_switch_p4_phy_sel:3;	/* 8-10 */
		uint32_t reserved2:1;	/* 11 */
		uint32_t z0_mux_sel:1;	/* 12 */
		uint32_t z1_gphy_mux_sel:1;	/* 13 */
		uint32_t z2_gphy_mux_sel:1;	/* 14 */
		uint32_t z2_crossbar_mux_sel:1;	/* 15 */
		uint32_t reserved3:1;	/* 16 */
		uint32_t z1_pda_en:1;	/* 17 */
		uint32_t z1_ck250_clk_en:1;	/* 18 */
		uint32_t z1_ck25_clk_dis:1;	/* 19 */
		uint32_t reserved4:2;	/* 20-21 */
		uint32_t z2_ck250_clk_en:1;	/* 22 */
		uint32_t z2_ck25_clk_dis:1;	/* 23 */
		uint32_t z2_serdes_clk_en:1;	/* 24 */
		uint32_t z2_serdes_reset_mdioregs:1;	/* 25 */
		uint32_t z2_sedes_reset_pll:1;	/* 26 */
		uint32_t z2_serdes_reset:1;	/* 27 */
		uint32_t z2_serdes_mux_sel:1;	/* 28 */
		uint32_t reserved5:1;	/* 29 */
		uint32_t z1_gphy_reset:1;	/* 30 */
		uint32_t z2_gphy_reset:1;	/* 31 */
#else
		uint32_t z2_gphy_reset:1;	/* 31 */
		uint32_t z1_gphy_reset:1;	/* 30 */
		uint32_t reserved5:1;	/* 29 */
		uint32_t z2_serdes_mux_sel:1;	/* 28 */
		uint32_t z2_serdes_reset:1;	/* 27 */
		uint32_t z2_sedes_reset_pll:1;	/* 26 */
		uint32_t z2_serdes_reset_mdioregs:1;	/* 25 */
		uint32_t z2_serdes_clk_en:1;	/* 24 */
		uint32_t z2_ck25_clk_dis:1;	/* 23 */
		uint32_t z2_ck250_clk_en:1;	/* 22 */
		uint32_t reserved4:2;	/* 20-21 */
		uint32_t z1_ck25_clk_dis:1;	/* 19 */
		uint32_t z1_ck250_clk_en:1;	/* 18 */
		uint32_t z1_pda_en:1;	/* 17 */
		uint32_t reserved3:1;	/* 16 */
		uint32_t z2_crossbar_mux_sel:1;	/* 15 */
		uint32_t z2_gphy_mux_sel:1;	/* 14 */
		uint32_t z1_gphy_mux_sel:1;	/* 13 */
		uint32_t z0_mux_sel:1;	/* 12 */
		uint32_t reserved2:1;	/* 11 */
		uint32_t z2_switch_p4_phy_sel:3;	/* 8-10 */
		uint32_t reserved1:1;	/* 7 */
		uint32_t z2_switch_p3_phy_sel:3;	/* 4-6 */
		uint32_t reserved0:1;	/* 3 */
		uint32_t z2_p_wan_phy_sel:3;	/* 0-2 */
#endif
	} Bits;
	uint32_t Reg32;
} BPCM_GLOBAL_CNTL;

#elif IS_BCMCHIP(6858) || IS_BCMCHIP(6846) || IS_BCMCHIP(6856) || \
	IS_BCMCHIP(63178) || IS_BCMCHIP(47622) || IS_BCMCHIP(6878) || \
	IS_BCMCHIP(63146) || IS_BCMCHIP(6855)  || IS_BCMCHIP(4912) || \
	IS_BCMCHIP(6756) || IS_BCMCHIP(6813) 

typedef union {
	struct {
		uint32_t tbd:32;
	} Bits;
	uint32_t Reg32;
} BPCM_GLOBAL_CNTL;
#else
typedef union {
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32_t z0_mux_sel:1;	/* 0 */
		uint32_t reserved0:4;	/* 1-4 */
		uint32_t z1_gphy_reset:1;	/* 5 */
		uint32_t reserved1:3;	/* 6-8 */
		uint32_t z1_ck25_clk_dis:1;	/* 9 */
		uint32_t z1_ck250_clk_en:1;	/* 10 */
		uint32_t reserved2:1;	/* 11 */
		uint32_t z1_gphy_mux_sel:1;	/* 12 */
		uint32_t z1_pda_en:1;	/* 13 */
		uint32_t reserved3:3;	/* 14-16 */
		uint32_t z2_gphy_reset:1;	/* 17 */
		uint32_t z2_serdes_reset:1;	/* 18 */
		uint32_t z2_serdes_reset_mdioregs:1;	/* 19 */
		uint32_t z2_sedes_reset_pll:1;	/* 20 */
		uint32_t z2_ck250_clk_en:1;	/* 21 */
		uint32_t z2_ck25_clk_dis:1;	/* 22 */
		uint32_t z2_serdes_clk_en:1;	/* 23 */
		uint32_t reserved4:1;	/* 24 */
		uint32_t z2_serdes_mux_sel:1;	/* 25 */
		uint32_t z2_gphy_mux_sel:1;	/* 26 */
		uint32_t z2_crossbar_mux_sel:1;	/* 27 */
		uint32_t z2_p_wan_phy_sel:2;	/* 28-29 */
		uint32_t z2_switch_phy_sel:2;	/* 30-31 */
#else
		uint32_t z2_switch_phy_sel:2;	/* 30-31 */
		uint32_t z2_p_wan_phy_sel:2;	/* 28-29 */
		uint32_t z2_crossbar_mux_sel:1;	/* 27 */
		uint32_t z2_gphy_mux_sel:1;	/* 26 */
		uint32_t z2_serdes_mux_sel:1;	/* 25 */
		uint32_t reserved4:1;	/* 24 */
		uint32_t z2_serdes_clk_en:1;	/* 23 */
		uint32_t z2_ck25_clk_dis:1;	/* 22 */
		uint32_t z2_ck250_clk_en:1;	/* 21 */
		uint32_t z2_sedes_reset_pll:1;	/* 20 */
		uint32_t z2_serdes_reset_mdioregs:1;	/* 19 */
		uint32_t z2_serdes_reset:1;	/* 18 */
		uint32_t z2_gphy_reset:1;	/* 17 */
		uint32_t reserved3:3;	/* 14-16 */
		uint32_t z1_pda_en:1;	/* 13 */
		uint32_t z1_gphy_mux_sel:1;	/* 12 */
		uint32_t reserved2:1;	/* 11 */
		uint32_t z1_ck250_clk_en:1;	/* 10 */
		uint32_t z1_ck25_clk_dis:1;	/* 9 */
		uint32_t reserved1:3;	/* 6-8 */
		uint32_t z1_gphy_reset:1;	/* 5 */
		uint32_t reserved0:4;	/* 1-4 */
		uint32_t z0_mux_sel:1;	/* 0 */
#endif
	} Bits;
	uint32_t Reg32;
} BPCM_GLOBAL_CNTL;
#endif

#if IS_BCMCHIP(63158)
typedef union {
	struct {

#ifdef PMC_LITTLE_ENDIAN
		uint32_t z0_mux_sel:1;	//        = r_Z0_GLOBAL_CNTL[0];
		uint32_t reserverd_1:1;	//        unused
		uint32_t z3_pda_en:1;	//        = r_Z0_GLOBAL_CNTL[2];
		uint32_t rx_sys_clk_en:1;	//        = r_Z0_GLOBAL_CNTL[3];
		uint32_t tx_sys_clk_en:1;	//         = r_Z0_GLOBAL_CNTL[4];
		uint32_t gmii_rx_clk_en:1;	//        = r_Z0_GLOBAL_CNTL[5];
		uint32_t gmii_tx_clk_en:1;	//        = r_Z0_GLOBAL_CNTL[6];
		uint32_t rsrvd:25;
#else
		uint32_t rsrvd:25;
		uint32_t gmii_tx_clk_en:1;	//        = r_Z0_GLOBAL_CNTL[6];
		uint32_t gmii_rx_clk_en:1;	//        = r_Z0_GLOBAL_CNTL[5];
		uint32_t tx_sys_clk_en:1;	//        = r_Z0_GLOBAL_CNTL[4];
		uint32_t rx_sys_clk_en:1;	//        = r_Z0_GLOBAL_CNTL[3];
		uint32_t z3_pda_en:1;	//        = r_Z0_GLOBAL_CNTL[2];
		uint32_t reserverd_1:1;	//        unused
		uint32_t z0_mux_sel:1;	//        = r_Z0_GLOBAL_CNTL[0];
#endif

	} Bits;
	uint32_t Reg32;

} BPCM_GLOBAL_CNTL_0;

typedef union {
	struct {

#ifdef PMC_LITTLE_ENDIAN
		uint32_t z1_pda_en:1;	//= r_Z1_GLOBAL_CNTL[0];
		uint32_t reserved:2;
		uint32_t z1_ck250_clk_en:1;	//= r_Z1_GLOBAL_CNTL[3];
		uint32_t z1_ref_clk_dis:1;	//= r_Z1_GLOBAL_CNTL[4];
		uint32_t z1_mux_sel:1;	//= r_Z1_GLOBAL_CNTL[5];
		uint32_t z1_gphy_reset:1;	//= r_Z1_GLOBAL_CNTL[6];
		uint32_t z1_gphy_iddq_global_pwr:1;	//= r_Z1_GLOBAL_CNTL[7];
		uint32_t z1_gphy_force_dll_en:1;	//= r_Z1_GLOBAL_CNTL[8];
		uint32_t z1_gphy_ext_pwr_down:4;	//= r_Z1_GLOBAL_CNTL[12:9];
		uint32_t z1_gphy_iddq_bias:1;	//= r_Z1_GLOBAL_CNTL[13];
		uint32_t z1_switch_p3_phy_sel:1;	//= r_Z1_GLOBAL_CNTL[14];
		uint32_t z1_switch_p8_sel:1;	//= r_Z1_GLOBAL_CNTL[15];
		uint32_t rsrvd:16;
#else
		uint32_t rsrvd:16;
		uint32_t z1_switch_p8_sel:1;	//= r_Z1_GLOBAL_CNTL[15];
		uint32_t z1_switch_p3_phy_sel:1;	//= r_Z1_GLOBAL_CNTL[14];
		uint32_t z1_gphy_iddq_bias:1;	//= r_Z1_GLOBAL_CNTL[13];
		uint32_t z1_gphy_ext_pwr_down:4;	//= r_Z1_GLOBAL_CNTL[12:9];
		uint32_t z1_gphy_force_dll_en:1;	//= r_Z1_GLOBAL_CNTL[8];
		uint32_t z1_gphy_iddq_global_pwr:1;	//= r_Z1_GLOBAL_CNTL[7];
		uint32_t z1_gphy_reset:1;	//= r_Z1_GLOBAL_CNTL[6];
		uint32_t z1_mux_sel:1;	//= r_Z1_GLOBAL_CNTL[5];
		uint32_t z1_ref_clk_dis:1;	//= r_Z1_GLOBAL_CNTL[4];
		uint32_t z1_ck250_clk_en:1;	//= r_Z1_GLOBAL_CNTL[3];
		uint32_t reserved:2;
		uint32_t z1_pda_en:1;	//= r_Z1_GLOBAL_CNTL[0];
#endif

	} Bits;
	uint32_t Reg32;

} BPCM_GLOBAL_CNTL_1;

typedef union {
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32_t z2_pda_en:1;	//= r_Z2_GLOBAL_CNTL[0];
		uint32_t reserved:1;
		uint32_t z2_ck250_clk_en:1;	//= r_Z2_GLOBAL_CNTL[2];
		uint32_t z2_ref_clk_dis:1;	//= r_Z2_GLOBAL_CNTL[3];
		uint32_t z2_serdes_clk_en:1;	//= r_Z2_GLOBAL_CNTL[4];
		uint32_t z2_gphy_mux_sel:1;	//= r_Z2_GLOBAL_CNTL[5];
		uint32_t z2_gphy_reset:1;	//= r_Z2_GLOBAL_CNTL[6];
		uint32_t z2_gphy_iddq_global_pwr:1;	//= r_Z2_GLOBAL_CNTL[7];
		uint32_t z2_gphy_force_dll_en:1;	//= r_Z2_GLOBAL_CNTL[8];
		uint32_t z2_gphy_ext_pwr_down:1;	//= r_Z2_GLOBAL_CNTL[9];
		uint32_t z2_gphy_iddq_bias:1;	//= r_Z2_GLOBAL_CNTL[10];
		uint32_t z2_crossbar_mux_sel:1;	//= r_Z2_GLOBAL_CNTL[11];
		uint32_t z2_p_wan_phy_sel:2;	//= r_Z2_GLOBAL_CNTL[13:12];
		uint32_t z2_switch_p4_phy_sel:2;	//= r_Z2_GLOBAL_CNTL[15:14];
		uint32_t z2_switch_p6_phy_sel:2;	//= r_Z2_GLOBAL_CNTL[17:16];
		uint32_t z2_serdes_mux_sel:1;	//= r_Z2_GLOBAL_CNTL[18];
		uint32_t z2_serdes_iddq:1;	//= r_Z2_GLOBAL_CNTL[19];
		uint32_t z2_serdes_pwrdwn:1;	//= r_Z2_GLOBAL_CNTL[20];
		uint32_t z2_serdes_reset:1;	//= r_Z2_GLOBAL_CNTL[21];
		uint32_t z2_serdes_reset_mdioregs:1;	//= r_Z2_GLOBAL_CNTL[22];
		uint32_t z2_serdes_reset_pll:1;	//= r_Z2_GLOBAL_CNTL[23];
		uint32_t z2_serdes_refclk_sel:3;	//= r_Z2_GLOBAL_CNTL[26:24];
		uint32_t z2_pll_clk125_250_sel:1;	//= r_Z2_GLOBAL_CNTL[27];
		uint32_t z2_pll_mux_clk250_sel:1;	//= r_Z2_GLOBAL_CNTL[28];
		uint32_t rsrvd:3;
#else
		uint32_t rsrvd:3;
		uint32_t z2_pll_mux_clk250_sel:1;	//= r_Z2_GLOBAL_CNTL[28];
		uint32_t z2_pll_clk125_250_sel:1;	//= r_Z2_GLOBAL_CNTL[27];
		uint32_t z2_serdes_refclk_sel:3;	//= r_Z2_GLOBAL_CNTL[26:24];
		uint32_t z2_serdes_reset_pll:1;	//= r_Z2_GLOBAL_CNTL[23];
		uint32_t z2_serdes_reset_mdioregs:1;	//= r_Z2_GLOBAL_CNTL[22];
		uint32_t z2_serdes_reset:1;	//= r_Z2_GLOBAL_CNTL[21];
		uint32_t z2_serdes_pwrdwn:1;	//= r_Z2_GLOBAL_CNTL[20];
		uint32_t z2_serdes_iddq:1;	//= r_Z2_GLOBAL_CNTL[19];
		uint32_t z2_serdes_mux_sel:1;	//= r_Z2_GLOBAL_CNTL[18];
		uint32_t z2_switch_p6_phy_sel:2;	//= r_Z2_GLOBAL_CNTL[17:16];
		uint32_t z2_switch_p4_phy_sel:2;	//= r_Z2_GLOBAL_CNTL[15:14];
		uint32_t z2_p_wan_phy_sel:2;	//= r_Z2_GLOBAL_CNTL[13:12];
		uint32_t z2_crossbar_mux_sel:1;	//= r_Z2_GLOBAL_CNTL[11];
		uint32_t z2_gphy_iddq_bias:1;	//= r_Z2_GLOBAL_CNTL[10];
		uint32_t z2_gphy_ext_pwr_down:1;	//= r_Z2_GLOBAL_CNTL[9];
		uint32_t z2_gphy_force_dll_en:1;	//= r_Z2_GLOBAL_CNTL[8];
		uint32_t z2_gphy_iddq_global_pwr:1;	//= r_Z2_GLOBAL_CNTL[7];
		uint32_t z2_gphy_reset:1;	//= r_Z2_GLOBAL_CNTL[6];
		uint32_t z2_gphy_mux_sel:1;	//= r_Z2_GLOBAL_CNTL[5];
		uint32_t z2_serdes_clk_en:1;	//= r_Z2_GLOBAL_CNTL[4];
		uint32_t z2_ref_clk_dis:1;	//= r_Z2_GLOBAL_CNTL[3];
		uint32_t z2_ck250_clk_en:1;	//= r_Z2_GLOBAL_CNTL[2];
		uint32_t reserved:1;
		uint32_t z2_pda_en:1;	//= r_Z2_GLOBAL_CNTL[0];
#endif

	} Bits;
	uint32_t Reg32;

} BPCM_GLOBAL_CNTL_2;
#endif

typedef union {
	struct {
		uint32_t ctl;
	} Bits_sata_gp;
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32_t iddq_bias:1;	/* 0 */
		uint32_t ext_pwr_down:4;	/* 1-4 */
		uint32_t force_dll_en:1;	/* 5 */
		uint32_t iddq_global_pwr:1;	/* 6 */
		uint32_t reserved:25;
#else
		uint32_t reserved:25;
		uint32_t iddq_global_pwr:1;	/* 6 */
		uint32_t force_dll_en:1;	/* 5 */
		uint32_t ext_pwr_down:4;	/* 1-4 */
		uint32_t iddq_bias:1;	/* 0 */
#endif
	} Bits_switch_z1_qgphy;
	struct {
#ifdef PMC_LITTLE_ENDIAN
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
#else
		uint32_t ctrl_en:1;	/* 31 */
		uint32_t reserved1:18;
		uint32_t phy_ad:5;	/* 8-12 */
		uint32_t reserved0:2;
		uint32_t phy_reset:1;	/* 5 */
		uint32_t ck25_dis:1;	/* 4 */
		uint32_t iddq_global_pwd:1;	/* 3 */
		uint32_t force_dll_en:1;	/* 2 */
		uint32_t ext_pwr_down:1;	/* 1 */
		uint32_t iddq_bias:1;	/* 0 */
#endif
	} Bits_egphy_1port;
	struct {
#ifdef PMC_LITTLE_ENDIAN
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
#else
		uint32_t ctrl_en:1;	/* 31 */
		uint32_t reserved1:14;
		uint32_t phy_ad:5;	/* 12-16 */
		uint32_t reserved0:3;
		uint32_t phy_reset:1;	/* 8 */
		uint32_t ck25_dis:1;	/* 7 */
		uint32_t iddq_global_pwd:1;	/* 6 */
		uint32_t force_dll_en:1;	/* 5 */
		uint32_t ext_pwr_down:4;	/* 1-4 */
		uint32_t iddq_bias:1;	/* 0 */
#endif
	} Bits_egphy_4port;
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32_t iddq_bias:1;	/* 0 */
		uint32_t ext_pwr_down:4;	/* 1-4 */
		uint32_t force_dll_en:1;	/* 5 */
		uint32_t iddq_global_pwr:1;	/* 6 */
		uint32_t reserved0:25;	/* 7-31 */
#else
		uint32_t reserved0:25;	/* 7-31 */
		uint32_t iddq_global_pwr:1;	/* 6 */
		uint32_t force_dll_en:1;	/* 5 */
		uint32_t ext_pwr_down:4;	/* 1-4 */
		uint32_t iddq_bias:1;	/* 0 */
#endif
	} Bits_qgphy_cntl;
	struct {
		uint32_t ctl;
	} Bits_vdsl_phy;
	struct {
#ifdef PMC_LITTLE_ENDIAN
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
#else
		uint32_t clken:1;	/* 31 */
		uint32_t reserved2:1;
		uint32_t reset_n_ctl:1;	/* 29 */
		uint32_t por_reset_n_ctl:1;	/* 28 */
		uint32_t ext_mclk_en:1;	/* 27 */
		uint32_t ext_mclk_en_reset:1;	/* 26 */
		uint32_t reserved1:8;
		uint32_t reset_dly_cfg:2;	/* 16-17 */
		uint32_t alt_bfc_en:1;	/* 15 */
		uint32_t reserved0:3;
		uint32_t alt_bfc_vector:12;	/* 00-11 */
#endif
	} Bits_vdsl_mips;
	uint32_t Reg32;
} BPCM_MISC_CONTROL;

typedef union {
	struct {
		uint32_t field;
	} Bits_qgphy_status;
	struct {
#ifdef PMC_LITTLE_ENDIAN
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
#else
		uint32_t clken:1;	/* 31 */
		uint32_t reserved2:1;
		uint32_t reset_n_ctl:1;	/* 29 */
		uint32_t por_reset_n_ctl:1;	/* 28 */
		uint32_t ext_mclk_en:1;	/* 27 */
		uint32_t ext_mclk_en_reset:1;	/* 26 */
		uint32_t reserved1:8;
		uint32_t reset_dly_cfg:2;	/* 16-17 */
		uint32_t alt_bfc_en:1;	/* 15 */
		uint32_t reserved0:3;
		uint32_t alt_bfc_vector:12;	/* 00-11 */
#endif
	} Bits_vdsl_mips;	/* second PHY MIPS core */
	uint32_t Reg32;
} BPCM_MISC_CONTROL2;

typedef union {
	struct {
#ifdef PMC_LITTLE_ENDIAN
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
#else
		uint32_t reserved2:14;	/* 31:18 */
		uint32_t pll_mux_clk_250_sel:1;	/* 17 */
		uint32_t pll_clk125_250_sel:1;	/* 16 */
		uint32_t reserved1:5;	/* 15:11 */
		uint32_t serdes_refclk_sel:3;	/* 10:08 */
		uint32_t reserved0:2;	/* 07:06 */
		uint32_t serdes_pwrdwn:1;	/* 05 */
		uint32_t serdes_iddq:1;	/* 04 */
		uint32_t gphy_iddq_global_pwr:1;	/* 03 */
		uint32_t gphy_force_dll_en:1;	/* 02 */
		uint32_t gphy_ext_pwr_down:1;	/* 01 */
		uint32_t gphy_iddq_bias:1;	/* 00 */
#endif
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
#if IS_BCMCHIP(63138)
#ifdef PMC_LITTLE_ENDIAN
		uint32_t core_pwr_ctrl:2;	// 01:00    R/W
		uint32_t reserved2:6;	// 07:02    R/W
		uint32_t pll_pwr_on:1;	// 08:08    R/W
		uint32_t pll_ldo_pwr_on:1;	// 09:09    R/W
		uint32_t pll_clamp_on:1;	// 10:10    R/W
		uint32_t reserved1:2;	// 12:11    R/O
		uint32_t cpu0_reset_n:1;	// 13:13    R/W
		uint32_t cpu1_reset_n:1;	// 14:14    R/W
		uint32_t neon_reset_n:1;	// 15:15    R/W
		uint32_t reserved0:12;	// 27:16    R/O
		uint32_t pwr_ctrl_sts:2;	// 29:28    R/O
		uint32_t power_down:2;	// 31:30    R/O
#else
		uint32_t power_down:2;	// 31:30    R/O
		uint32_t pwr_ctrl_sts:2;	// 29:28    R/O
		uint32_t reserved0:12;	// 27:16    R/O
		uint32_t neon_reset_n:1;	// 15:15    R/W
		uint32_t cpu1_reset_n:1;	// 14:14    R/W
		uint32_t cpu0_reset_n:1;	// 13:13    R/W
		uint32_t reserved1:2;	// 12:11    R/O
		uint32_t pll_clamp_on:1;	// 10:10    R/W
		uint32_t pll_ldo_pwr_on:1;	// 09:09    R/W
		uint32_t pll_pwr_on:1;	// 08:08    R/W
		uint32_t reserved2:6;	// 07:02    R/W
		uint32_t core_pwr_ctrl:2;	// 01:00    R/W
#endif
#elif IS_BCMCHIP(63148)
#ifdef PMC_LITTLE_ENDIAN
		uint32_t cpu0_reset_n:1;	// 00:00    R/W
		uint32_t power_down:1;	// 01:01    R/W
		uint32_t reserved:30;	// 31:02    R/O
#else
		uint32_t reserved:30;	// 31:02    R/O
		uint32_t power_down:1;	// 01:01    R/W
		uint32_t cpu0_reset_n:1;	// 00:00    R/W
#endif
#elif IS_BCMCHIP(6858) || IS_BCMCHIP(63158) || IS_BCMCHIP(6846) || \
	IS_BCMCHIP(6856) || IS_BCMCHIP(63178) || IS_BCMCHIP(47622) || \
	IS_BCMCHIP(6878) || IS_BCMCHIP(63146) || IS_BCMCHIP(4912) || \
	IS_BCMCHIP(6855) || IS_BCMCHIP(6756) || IS_BCMCHIP(6813) 
#ifdef PMC_LITTLE_ENDIAN
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
#else
		uint32_t start:1;	// 31:31    R/W
		uint32_t sw_done:1;	// 30:30    R/W
		uint32_t hw_done:1;	// 29:29    R/O
		uint32_t webcores_sr:1;	// 28:28    R/W
		uint32_t cci_sr:1;	// 27:27    R/W
		uint32_t ubus_sr:1;	// 26:26    R/W
		uint32_t c1l2_bpcm_init_on:1;	// 25:25    R/W
		uint32_t c0l2_bpcm_init_on:1;	// 24:24    R/W
		uint32_t cpu_bpcm_init_on:8;	// 23:16    R/W
		uint32_t reserved0:6;	// 15:10    R/O
		uint32_t c1l2_reset:1;	// 09:09    R/W
		uint32_t c0l2_reset:1;	// 08:08    R/W
		uint32_t cpu_reset_n:8;	// 07:00    R/W
#endif
#endif
	} Bits;
	uint32_t Reg32;
} ARM_CONTROL_REG;

typedef union {
	struct {
#ifdef PMC_LITTLE_ENDIAN
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
#else
		uint32_t pwr_on_status:4;	// 31:28    R/O only LS 2-bits for CPU1, only LS 1 bit for neon_l2
		uint32_t pwr_ok_status:4;	// 27:24    R/O ditto
		uint32_t pwr_on:4;	// 23:20    R/W ditto
		uint32_t pwr_ok:4;	// 19:16    R/W ditto
		uint32_t clamp_on:1;	// 15:15    R/W
		uint32_t reserved0:3;	// 14:12    R/W
		uint32_t mem_pda:4;	// 11:08    R/W only LS bit for CPU0/1, all four bits for neon_l2
		uint32_t reserved1:2;	// 07:06    R/W
		uint32_t mem_pwr_on_status:1;	// 05:05    R/O
		uint32_t mem_pwr_ok_status:1;	// 04:04    R/O
		uint32_t reserved2:1;	// 03:03    R/W
		uint32_t mem_clamp_on:1;	// 02:02    R/W
		uint32_t mem_pwr_on:1;	// 01:01    R/W
		uint32_t mem_pwr_ok:1;	// 00:00    R/W
#endif
	} Bits;
	uint32_t Reg32;
} ARM_CPUx_PWR_CTRL_REG;

typedef union {
	struct {
#ifdef PMC_LITTLE_ENDIAN
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
#else
		uint32_t reserved3:11;	// 21:31
		uint32_t byp_wait:1;	// 20:20 // only used in b15pll
		uint32_t reserved2:4;	// 16:19
		uint32_t hold_ch_all:1;	// 15:15
		uint32_t reserved1:1;	// 14:14
		uint32_t ldo_ctrl:6;	// 13:08
		uint32_t reserved0:2;	// 07:06
		uint32_t iso:1;	// 05:05 // only used in afepll
		uint32_t pwrdwn_ldo:1;	// 04:04
		uint32_t master_reset:1;	// 03:03
		uint32_t pwrdwn:1;	// 02:02
		uint32_t post_resetb:1;	// 01:01
		uint32_t resetb:1;	// 00:00
#endif
	} Bits;
	uint32_t Reg32;
} PLL_CTRL_REG;

typedef union {
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32_t fb_offset:12;	// 11:00
		uint32_t fb_phase_en:1;	// 12:12
		uint32_t _8phase_en:1;	// 13:13
		uint32_t sr:18;	// 31:14
#else
		uint32_t sr:18;	// 31:14
		uint32_t _8phase_en:1;	// 13:13
		uint32_t fb_phase_en:1;	// 12:12
		uint32_t fb_offset:12;	// 11:00
#endif
	} Bits;
	uint32_t Reg32;
} PLL_PHASE_REG;

#if IS_BCMCHIP(63146) || IS_BCMCHIP(4912) || IS_BCMCHIP(6813) 
typedef union {
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32_t ndiv_int:10;		// 09:00
		uint32_t reserved0:2;		// 11:10
		uint32_t fref_eff_info:8;	// 19:12
		uint32_t auto_cnfg_dsbl:8;	// 27:20
		uint32_t reserved1:3;		// 30:28
		uint32_t ndiv_override:1;	// 31
#else
		uint32_t ndiv_override:1;	// 31
		uint32_t reserved1:3;		// 30:28
		uint32_t auto_cnfg_dsbl:8;	// 27:20  
		uint32_t fref_eff_info:8;	// 19:12
		uint32_t reserved0:2;		// 11:10
		uint32_t ndiv_int:10;		// 09:00
#endif
	} Bits;
	uint32_t Reg32;
} PLL_NDIV_REG;

typedef union {
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32_t pdiv:4;	// 03:00
		uint32_t ndiv_frac:24;	// 27:04
		uint32_t reserved0:3;	// 30:28
		uint32_t ndiv_pdiv_override:1;	// 31:31
#else
		uint32_t ndiv_pdiv_override:1;	// 31:31
		uint32_t reserved0:3;	// 30:28
		uint32_t ndiv_frac:24;	// 27:04
		uint32_t pdiv:4;	// 03:00
#endif
	} Bits;
	uint32_t Reg32;
} PLL_PDIV_REG;

typedef union {
	struct {
#ifdef PMC_LITTLE_ENDIAN
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
#else
		uint32_t mdiv_override1:1;	// 31:31
		uint32_t reserved1:3;	// 30:28
		uint32_t mdel1:1;	// 27:27
		uint32_t hold_ch1:1;	// 26:26
		uint32_t enableb_ch1:1;	// 25:25
		uint32_t mdiv1:9;	// 24:16
		uint32_t mdiv_override0:1;	// 15:15
		uint32_t reserved0:3;	// 14:12
		uint32_t mdel0:1;	// 11:11
		uint32_t hold_ch0:1;	// 10:10
		uint32_t enableb_ch0:1;	// 09:09
		uint32_t mdiv0:9;	// 08:00
#endif
	} Bits;
	uint32_t Reg32;
} PLL_CHCFG_REG;

typedef union {
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32_t reserved0:8;	// 07:00
		uint32_t stat_select:3;	// 10:08
		uint32_t pq_mode:1;	// 11:11
		uint32_t stat_mode:2;	// 13:12
		uint32_t stat_reset:1;	// 14:14  
		uint32_t stat_update:1;	// 15:15  
		uint32_t ssc_step:16;	// 31:16
#else
		uint32_t ssc_step:16;	// 31:16
		uint32_t stat_update:1;	// 15:15
		uint32_t stat_reset:1;	// 14:14
		uint32_t stat_mode:2;	// 13:12
		uint32_t pq_mode:1;	// 11:11
		uint32_t stat_select:3;	// 10:08
		uint32_t reserved0:8;	// 07:00
#endif
	} Bits;
	uint32_t Reg32;
} PLL_LOOP0_REG;

#else
typedef union {
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32_t ndiv_int:10;	// 09:00
		uint32_t ndiv_frac:20;	// 29:10
		uint32_t reserved0:1;	// 30
		uint32_t ndiv_override:1;	// 31
#else
		uint32_t ndiv_override:1;	// 31
		uint32_t reserved0:1;	// 30
		uint32_t ndiv_frac:20;	// 29:10
		uint32_t ndiv_int:10;	// 09:00
#endif
	} Bits;
	uint32_t Reg32;
} PLL_NDIV_REG;

typedef union {
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32_t pdiv:3;	// 02:00
		uint32_t reserved0:28;	// 30:03
		uint32_t ndiv_pdiv_override:1;	// 31:31
#else
		uint32_t ndiv_pdiv_override:1;	// 31:31
		uint32_t reserved0:28;	// 30:03
		uint32_t pdiv:3;	// 02:00
#endif
	} Bits;
	uint32_t Reg32;
} PLL_PDIV_REG;

typedef union {
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32_t mdiv0:8;	// 07:00
		uint32_t enableb_ch0:1;	// 08:08
		uint32_t hold_ch0:1;	// 09:09
		uint32_t load_en_ch0:1;	// 10:10
		uint32_t mdel0:1;	// 11:11
		uint32_t reserved0:3;	// 14:12
		uint32_t mdiv_override0:1;	// 15:15
		uint32_t mdiv1:8;	// 23:16
		uint32_t enableb_ch1:1;	// 24:24
		uint32_t hold_ch1:1;	// 25:25
		uint32_t load_en_ch1:1;	// 26:26
		uint32_t mdel1:1;	// 27:27
		uint32_t reserved1:3;	// 30:28
		uint32_t mdiv_override1:1;	// 31:31
#else
		uint32_t mdiv_override1:1;	// 31:31
		uint32_t reserved1:3;	// 30:28
		uint32_t mdel1:1;	// 27:27
		uint32_t load_en_ch1:1;	// 26:26
		uint32_t hold_ch1:1;	// 25:25
		uint32_t enableb_ch1:1;	// 24:24
		uint32_t mdiv1:8;	// 23:16
		uint32_t mdiv_override0:1;	// 15:15
		uint32_t reserved0:3;	// 14:12
		uint32_t mdel0:1;	// 11:11
		uint32_t load_en_ch0:1;	// 10:10
		uint32_t hold_ch0:1;	// 09:09
		uint32_t enableb_ch0:1;	// 08:08
		uint32_t mdiv0:8;	// 07:00
#endif
	} Bits;
	uint32_t Reg32;
} PLL_CHCFG_REG;

typedef union {
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32_t reserved0:4;	// 03:00
		uint32_t ka:3;	// 06:04
		uint32_t reserved1:1;	// 07:07
		uint32_t ki:3;	// 10:08
		uint32_t reserved2:1;	// 11:11
		uint32_t kp:4;	// 15:12
		uint32_t ssc_step:16;	// 31:16
#else
		uint32_t ssc_step:16;	// 31:16
		uint32_t kp:4;	// 15:12
		uint32_t reserved2:1;	// 11:11
		uint32_t ki:3;	// 10:08
		uint32_t reserved1:1;	// 07:07
		uint32_t ka:3;	// 06:04
		uint32_t reserved0:4;	// 03:00
#endif
	} Bits;
	uint32_t Reg32;
} PLL_LOOP0_REG;

#endif

typedef union {
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32_t ssc_limit:22;	// 21:00
		uint32_t reserved0:2;	// 23:22
		uint32_t ssc_clkdiv:4;	// 27:24
		uint32_t ssc_status:1;	// 28:28
		uint32_t reserved1:2;	// 30:29
		uint32_t ssc_mode:1;	// 31:31
#else
		uint32_t ssc_mode:1;	// 31:31
		uint32_t reserved1:2;	// 30:29
		uint32_t ssc_status:1;	// 28:28
		uint32_t ssc_clkdiv:4;	// 27:24
		uint32_t reserved0:2;	// 23:22
		uint32_t ssc_limit:22;	// 21:00
#endif
	} Bits;
	uint32_t Reg32;
} PLL_LOOP1_REG;

typedef union {
	struct {
#ifdef PMC_LITTLE_ENDIAN
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
#else
		uint32_t t2d_offset:3;	// 31:29
		uint32_t bb_en:1;	// 28:28
		uint32_t dco_bias_boost:1;	// 27:27
		uint32_t dco_div2_div4:1;	// 26:26
		uint32_t dco_en:1;	// 25:25
		uint32_t state_update:1;	// 24:24
		uint32_t state_sel:3;	// 23:21
		uint32_t state_mode:2;	// 20:19
		uint32_t state_reset:1;	// 18:18
		uint32_t fdco_dac_sel:1;	// 17:17
		uint32_t fdco_bypass_en:1;	// 16:16
		uint32_t fdco_ctrl_bypass:16;	// 15:00
#endif
	} Bits;
	uint32_t Reg32;
} PLL_CFG0_REG;

typedef union {
	struct {
#ifdef PMC_LITTLE_ENDIAN
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
#else
		uint32_t reserved0:11;	// 31:21
		uint32_t ki_boost:1;	// 20:20
		uint32_t post_div2_div3:1;	// 19:19
		uint32_t refd2c_bias:3;	// 18:16
		uint32_t en_500ohm:1;	// 15:15
		uint32_t ki_startlow:1;	// 14:14
		uint32_t ref_diff_sel:1;	// 13:13
		uint32_t byp1_en:1;	// 12:12
		uint32_t byp2_en:1;	// 11:11
		uint32_t port_reset_mode:2;	// 10:09
		uint32_t pwm_ctrl:2;	// 08:07
		uint32_t kpp:4;	// 06:03
		uint32_t t2d_clk_sel:1;	// 02:02
		uint32_t t2d_clk_enable:1;	// 01:01
		uint32_t t2d_offset_msb:1;	// 00:00
#endif
	} Bits;
	uint32_t Reg32;
} PLL_CFG1_REG;

typedef union {
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32_t en_cml:3;	// 02:00
		uint32_t tri_en:1;	// 03:03
		uint32_t test_sel:3;	// 06:04
		uint32_t test_en:1;	// 07:07
		uint32_t reserved0:24;
#else
		uint32_t reserved0:24;
		uint32_t test_en:1;	// 07:07
		uint32_t test_sel:3;	// 06:04
		uint32_t tri_en:1;	// 03:03
		uint32_t en_cml:3;	// 02:00
#endif
	} Bits;
	uint32_t Reg32;
} PLL_OCTRL_REG;

typedef union {
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32_t out:12;	// 11:00
		uint32_t reserved:19;	// 30:12
		uint32_t lock:1;	// 31:31
#else
		uint32_t lock:1;	// 31:31
		uint32_t reserved:19;	// 30:12
		uint32_t out:12;	// 11:00
#endif
	} Bits;
	uint32_t Reg32;
} PLL_STAT_REG;

#if IS_BCMCHIP(6858) || IS_BCMCHIP(63158) || IS_BCMCHIP(6846) || \
	IS_BCMCHIP(6856) || IS_BCMCHIP(63178) || IS_BCMCHIP(47622) || \
	IS_BCMCHIP(6878) || IS_BCMCHIP(6855) || IS_BCMCHIP(6756)
typedef union {
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32_t ndiv_int:10;	// 09:00
		uint32_t reserved0:2;	// 11:10
		uint32_t ndiv_frac:20;	// 31:12    
#else
		uint32_t ndiv_frac:20;	// 31:12
		uint32_t reserved0:2;	// 11:10
		uint32_t ndiv_int:10;	// 09:00
#endif
	} Bits;
	uint32_t Reg32;
} PLL_DECNDIV_REG;

typedef union {
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32_t pdiv:4;	// 03:00
		uint32_t reserved0:12;	// 15:04
		uint32_t mdiv0:8;	// 23:16
		uint32_t mdiv1:8;	// 31:24
#else
		uint32_t mdiv1:8;	// 31:24
		uint32_t mdiv0:8;	// 23:16
		uint32_t reserved0:12;	// 15:04
		uint32_t pdiv:4;	// 03:00
#endif
	} Bits;
	uint32_t Reg32;
} PLL_DECPDIV_REG;

typedef union {
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32_t mdiv2:8;	// 07:00
		uint32_t mdiv3:8;	// 15:08
		uint32_t mdiv4:8;	// 23:16
		uint32_t mdiv5:8;	// 31:24
#else
		uint32_t mdiv5:8;	// 31:24
		uint32_t mdiv4:8;	// 23:16
		uint32_t mdiv3:8;	// 15:08
		uint32_t mdiv2:8;	// 07:00
#endif
	} Bits;
	uint32_t Reg32;
} PLL_DECCH25_REG;
#elif IS_BCMCHIP(63146) || IS_BCMCHIP(4912) || IS_BCMCHIP(6813) 
typedef union {
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32_t ndiv_int:10;	// 9:0	  
		uint32_t ndiv_frac:22;	// 31:10

#else
		uint32_t ndiv_frac:22;	// 31:10	  
		uint32_t ndiv_int:10;	// 9:0
#endif
	} Bits;
	uint32_t Reg32;
} PLL_DECNDIV_REG;

typedef union {
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32_t pdiv:4;	// 03:00
		uint32_t mdiv5:9;	// 12:04
		uint32_t mdiv0:9;	// 21:13
		uint32_t mdiv1:9;	// 30:22
		uint32_t reserved0:1;	// 31:31
#else
		uint32_t reserved0:1;	// 31:31
		uint32_t mdiv1:9;	// 30:22
		uint32_t mdiv0:9;	// 21:13  
		uint32_t mdiv5:9;	// 12:04
		uint32_t pdiv:4;	// 03:00
#endif
	} Bits;
	uint32_t Reg32;
} PLL_DECPDIV_REG;

typedef union {
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32_t mdiv2:9;	// 08:00
		uint32_t mdiv3:9;	// 17:09
		uint32_t mdiv4:9;	// 26:18
		uint32_t reserved0:5;	// 31:27
#else
		uint32_t reserved0:5;	// 31:27
		uint32_t mdiv4:9;	// 26:18
		uint32_t mdiv3:9;	// 17:09
		uint32_t mdiv2:9;	// 08:00
#endif
	} Bits;
	uint32_t Reg32;
} PLL_DECCH25_REG;
#endif

typedef union {
	struct {
#ifdef PMC_LITTLE_ENDIAN
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
#else
		uint32_t reset_state:1;	// R/O 31:31
		uint32_t iso_state:1;	// R/O
		uint32_t mem_pwr_state:1;	// R/O
		uint32_t dpg_pwr_state:1;	// R/O
		uint32_t pwr_good:1;	// R/O
		uint32_t pwr_on_state:1;	// R/O
		uint32_t pwr_off_state:1;	// R/O
		uint32_t freq_scalar_dyn_sel:1;	// R/O
		uint32_t pwr_cntl_state:5;
		uint32_t reserved:5;
		uint32_t mem_stby:1;
		uint32_t blk_reset_assert:1;	//      12:12
		uint32_t mem_pwr_ctl_en:1;
		uint32_t pwr_up_req:1;
		uint32_t pwr_dn_req:1;
		uint32_t dpg_ctl_en:1;
		uint32_t manual_ctl:1;
		uint32_t manual_iso_ctl:1;
		uint32_t manual_mem_pwr:2;
		uint32_t dpg_capable:1;	// R/O
		uint32_t freq_scale_used:1;	// R/O
		uint32_t manual_reset_ctl:1;
		uint32_t manual_clk_en:1;
#endif
	} Bits;
	uint32_t Reg32;
} BPCM_PWR_ZONE_N_CONTROL;

typedef union {
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32_t pwr_ok_delay_sel:3;
		uint32_t pwk_ok_thresh:2;
		uint32_t reserved:3;
		uint32_t iso_on_delay:4;
		uint32_t iso_off_delay:4;
		uint32_t clock_on_delay:4;
		uint32_t clock_off_delay:4;
		uint32_t reset_on_delay:4;
		uint32_t reset_off_delay:4;
#else
		uint32_t reset_off_delay:4;	// 31:28
		uint32_t reset_on_delay:4;
		uint32_t clock_off_delay:4;
		uint32_t clock_on_delay:4;
		uint32_t iso_off_delay:4;
		uint32_t iso_on_delay:4;
		uint32_t reserved:3;
		uint32_t pwk_ok_thresh:2;
		uint32_t pwr_ok_delay_sel:3;
#endif
	} Bits;
	uint32_t Reg32;
} BPCM_PWR_ZONE_N_CONFIG1;

typedef union {
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32_t delay_prescale_sel:3;
		uint32_t slew_prescale_sel:3;
		uint32_t reserved:6;
		uint32_t dpgn_on_delay:4;
		uint32_t dpg1_on_delay:4;
		uint32_t dpg_off_delay:4;
		uint32_t mem_on_delay:4;
		uint32_t mem_off_delay:4;
#else
		uint32_t mem_off_delay:4;	// 31:28
		uint32_t mem_on_delay:4;
		uint32_t dpg_off_delay:4;
		uint32_t dpg1_on_delay:4;
		uint32_t dpgn_on_delay:4;
		uint32_t reserved:6;
		uint32_t slew_prescale_sel:3;
		uint32_t delay_prescale_sel:3;
#endif
	} Bits;
	uint32_t Reg32;
} BPCM_PWR_ZONE_N_CONFIG2;

typedef union {
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32_t fs_bypass_en:1;
		uint32_t gear_sel:1;
		uint32_t use_dyn_gear_sel:1;
		uint32_t reserved2:1;
		uint32_t low_gear_div:3;
		uint32_t high_gear_div:3;
		uint32_t reserved:22;
#else
		uint32_t reserved:22;	// 31:10
		uint32_t high_gear_div:3;
		uint32_t low_gear_div:3;
		uint32_t reserved2:1;
		uint32_t use_dyn_gear_sel:1;
		uint32_t gear_sel:1;
		uint32_t fs_bypass_en:1;
#endif
	} Bits;
	uint32_t Reg32;
} BPCM_ZONE_N_FREQ_SCALAR_CONTROL;

typedef struct {
	BPCM_PWR_ZONE_N_CONTROL control;
	BPCM_PWR_ZONE_N_CONFIG1 config1;
	BPCM_PWR_ZONE_N_CONFIG2 config2;
#if IS_BCMCHIP(63178) || IS_BCMCHIP(47622) || IS_BCMCHIP(6878) || IS_BCMCHIP(63146)\
   || IS_BCMCHIP(4912) || IS_BCMCHIP(6813) || IS_BCMCHIP(6855) || IS_BCMCHIP(6756)
	uint32_t reserved0;
	uint32_t timer_control;
	uint32_t timer_status;
	uint32_t reserved1[2];
#else
	BPCM_ZONE_N_FREQ_SCALAR_CONTROL freq_scalar_control;
#endif
#if IS_BCMCHIP(6846) || IS_BCMCHIP(6856)
	uint32_t reserved[4];
#endif
} BPCM_ZONE;

#define BPCMZoneOffset(reg)	offsetof(BPCM_ZONE,reg)
#define BPCMZoneRegOffset(reg)	(BPCMZoneOffset(reg) >> 2)

typedef union {
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32_t pmb_Addr:8;
		uint32_t hw_rev:8;
		uint32_t module_id:16;
#else
		uint32_t module_id:16;
		uint32_t hw_rev:8;
		uint32_t pmb_Addr:8;
#endif
	} Bits;
	uint32_t Reg32;
} BPCM_UBUS_ID_REG;

typedef union {
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32_t num_zones:8;
		uint32_t sr_reg_bits:8;
		uint32_t pllType:2;
		uint32_t reserved0:1;
		uint32_t ubus:1;
		uint32_t reserved1:12;
#else
		uint32_t reserved1:12;
		uint32_t ubus:1;
		uint32_t reserved0:1;
		uint32_t pllType:2;
		uint32_t sr_reg_bits:8;
		uint32_t num_zones:8;
#endif
	} Bits;
	uint32_t Reg32;
} BPCM_UBUS_CAPABILITES_REG;

typedef union {
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32_t ctrl_eswap:4;
		uint32_t reserved0:4;
		uint32_t ctrl_cd:4;
		uint32_t reserved1:4;
		uint32_t ctrl_seclev:8;
		uint32_t reqout_seclev:8;
#else
		uint32_t reqout_seclev:8;
		uint32_t ctrl_seclev:8;
		uint32_t reserved1:4;
		uint32_t ctrl_cd:4;
		uint32_t reserved0:4;
		uint32_t ctrl_eswap:4;
#endif
	} Bits;
	uint32_t Reg32;
} BPCM_UBUS_CTRL_REG;

typedef union {
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint64_t addr_in:24;
		uint64_t addr_out:24;
		uint64_t pid:8;
		uint64_t size:5;
		uint64_t cmddta:1;
		uint64_t en:2;
#else
		uint64_t en:2;
		uint64_t cmddta:1;
		uint64_t size:5;
		uint64_t pid:8;
		uint64_t addr_out:24;
		uint64_t addr_in:24;
#endif
	} Bits;
	struct {
		uint32_t word0;
		uint32_t word1;
	} Regs32;
	uint64_t Reg64;
} BPCM_UBUS_CFG_REG;

#if IS_BCMCHIP(63178) 
typedef union {
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32_t counter:8;
		uint32_t reserved2:7;
		uint32_t enable:1;
		uint32_t reserved1:16;
#else
		uint32_t reserved1:16;
		uint32_t enable:1;
		uint32_t reserved2:7;
		uint32_t counter:8;
#endif
	} Bits;
	uint32_t Reg32;
} BPCM_CLKRST_VREG_CONTROL;
#endif
#if IS_BCMCHIP(63146)
typedef union {
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32_t ubus_soft_reset:1;
		uint32_t alt_ubus_clk_sel:1;
		uint32_t observe_clk_sw_init:1;
		uint32_t alt_emmc_clk_sel:1;
		uint32_t reserved:5;
		uint32_t enable:1;
		uint32_t counter:8;
		uint32_t reserved2:14;
#else
		uint32_t reserved2:14;
		uint32_t counter:8;
		uint32_t enable:1;
		uint32_t reserved:5;
		uint32_t alt_emmc_clk_sel:1;
		uint32_t observe_clk_sw_init:1;
		uint32_t alt_ubus_clk_sel:1;
		uint32_t ubus_soft_reset:1;

#endif
	} Bits;
	uint32_t Reg32;
} BPCM_CLKRST_VREG_CONTROL;
#endif


#if IS_BCMCHIP(47622) || IS_BCMCHIP(6756)
typedef union {
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32_t ubus_soft_rst:1;
		uint32_t alt_ubus_clk_sel:1;
		uint32_t obsv_clk_swinit:1;
		uint32_t reserved0:17;
		uint32_t wl0_rf_enable:1;
		uint32_t wl1_rf_enable:1;
		uint32_t reserved1:10;
#else
		uint32_t reserved1:10;
		uint32_t wl1_rf_enable:1;
		uint32_t wl0_rf_enable:1;
		uint32_t reserved0:17;
		uint32_t obsv_clk_swinit:1;
		uint32_t alt_ubus_clk_sel:1;
		uint32_t ubus_soft_rst:1;
#endif
	} Bits;
	uint32_t Reg32;
} BPCM_CLKRST_CONTROL;
#endif

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
#if IS_BCMCHIP(6846) || IS_BCMCHIP(6856) || IS_BCMCHIP(63178) || \
	IS_BCMCHIP(47622) || IS_BCMCHIP(6878) || IS_BCMCHIP(63146) || \
	IS_BCMCHIP(4912) || IS_BCMCHIP(6813) || IS_BCMCHIP(6855) || IS_BCMCHIP(6756)
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
#else
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

#if IS_BCMCHIP(63158)
	uint32_t reserved;	// offset = 0x2c, actual offset = 11
	BPCM_GLOBAL_CNTL_0 global_control;	// offset = 0x30, actual offset = 12
	BPCM_GLOBAL_CNTL_1 global_control_1;	// offset = 0x34, actual offset = 13
	BPCM_GLOBAL_CNTL_2 global_control_2;	// offset = 0x38, actual offset = 14
	uint32_t global_status;	// offset = 0x3c, actual offset = 15
#else
	BPCM_GLOBAL_CNTL global_control;	// offset = 0x2c, actual offset = 11
	BPCM_MISC_CONTROL misc_control;	// offset = 0x30, actual offset = 12
	BPCM_MISC_CONTROL2 misc_control2;	// offset = 0x34, actual offset = 13
	BPCM_SGPHY_CNTL sgphy_cntl;	// offset = 0x38, actual offset = 14
	BPCM_SGPHY_STATUS sgphy_status;	// offset = 0x3c, actual offset = 15
#endif
	BPCM_ZONE zones[1020];	// offset = 0x40..0x3FFC, actual offset = 16..4095 (1020 * 4 = 4080 + 16 = 4096)
#endif
} BPCM_REGS;			// total offset space = 4096

#define BPCM_OFFSET(reg) (offsetof(BPCM_REGS,reg)>>2)

#if !IS_BCMCHIP(6878) && !IS_BCMCHIP(6846) && !IS_BCMCHIP(6856) && !IS_BCMCHIP(6855)
typedef struct {
#if !IS_BCMCHIP(63178) && !IS_BCMCHIP(63146)
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
#else
	BPCM_ID_REG id_reg;	// offset = 0x00, actual offset = 0
	BPCM_CAPABILITES_REG capabilities;	// offset = 0x04, actual offset = 1
	uint32_t reserved0[2];	// offset = 0x08, actual offset = 2
	uint32_t cfg_control;	// offset = 0x10, actual offset = 4
	BPCM_SR_CONTROL sr_control;	// offset = 0x14, actual offset = 5
#if IS_BCMCHIP(63146)
	uint32_t reserved1[2];	// offset = 0x18, actual offset = 6
	uint32_t vdsl_arm_sr;	// offset = 0x20, actual offset = 8
	BPCM_VDSL_ARM_RST_CTL vdsl_arm_rst_control;	// offset = 0x24, actual offset = 9
	uint32_t reserved2[2];	// offset = 0x28, actual offset = 10
	uint32_t vdsl_afe_config0;	// offset = 0x30, actual offset = 12
	uint32_t vdsl_afe_config1;	// offset = 0x34, actual offset = 13
	uint32_t reserved3[18];	// offset = 0x38, actual offset = 14
#else
	uint32_t reserved1[10];	// offset = 0x18, actual offset = 6
	BPCM_MISC_CONTROL misc_control;	// offset = 0x40, actual offset = 16
	uint32_t vdsl_phy_ctl;	// offset = 0x44, actual offset = 17
	uint32_t vdsl_afe_ctl;	// offset = 0x48, actual offset = 18
	uint32_t reserved2[13];	// offset = 0x4c, actual offset = 19
#endif
	BPCM_ZONE zones;	// offset = 0x80, actual offset = 32
#endif
} BPCM_VDSL_REGS;		// total offset space = 4096

#define BPCMVDSLOffset(reg)	offsetof(BPCM_VDSL_REGS,reg)
#define BPCMVDSLRegOffset(reg)	(BPCMVDSLOffset(reg) >> 2)

#endif

#define BPCMOffset(reg)		offsetof(BPCM_REGS,reg)
#define BPCMRegOffset(reg)	(BPCMOffset(reg) >> 2)

#if IS_BCMCHIP(63138) || IS_BCMCHIP(63148) || IS_BCMCHIP(6858) || \
	IS_BCMCHIP(63158)
// ARM BPCM addresses as used by 63138/63148 and possibly others (28nm)
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
	uint32_t reserved;	// offset = 0x2c, actual offset = 11
	ARM_CONTROL_REG arm_control;	// offset = 0x30, actual offset = 12
#if IS_BCMCHIP(63138)
	ARM_CPUx_PWR_CTRL_REG arm_pwr_ctrl_0;	// offset = 0x34, actual offset = 13
	ARM_CPUx_PWR_CTRL_REG arm_pwr_ctrl_1;	// offset = 0x38, actual offset = 14
	ARM_CPUx_PWR_CTRL_REG arm_neon_l2;	// offset = 0x3c, actua; offset = 15
#elif IS_BCMCHIP(63148)
	uint32_t reserved1[3];	// offset = 0x34, actual offset = 13..15
#elif IS_BCMCHIP(6858)
	uint32_t biu_clk_control0;	// offset = 0x34, actual offset = 13
	uint32_t reserved1[2];	// offset = 0x38, actual offset = 13..14
#endif
	BPCM_ZONE zones[1020];	// offset = 0x40..0x3FFC, actual offset = 16..4095 (1020 * 4 = 4080 + 16 = 4096)
} ARM_BPCM_REGS;
#define ARMBPCMOffset(reg)  offsetof(ARM_BPCM_REGS,reg)
#define ARMBPCMRegOffset(reg)   (ARMBPCMOffset(reg) >> 2)

#elif IS_BCMCHIP(6846) || IS_BCMCHIP(6856)
typedef struct {
	CLASSIC_BPCM_ID_REG id_reg;	// offset = 0x00, actual offset = 0
	BPCM_CAPABILITES_REG capabilities;	// offset = 0x04, actual offset = 1
	uint32_t control;	// offset = 0x08, actual offset = 2
	BPCM_STATUS_REG status;	// offset = 0x0c, actual offset = 3
	BPCM_AVS_ROSC_CONTROL_REG rosc_control;	// offset = 0x10, actual offset = 4
	BPCM_AVS_ROSC_THRESHOLD rosc_thresh;	// offset = 0x14, actual offset = 5
	BPCM_AVS_ROSC_COUNT rosc_count;	// offset = 0x18, actual offset = 6
	BPCM_AVS_PWD_CONTROL pwd_control;	// offset = 0x1c, actual offset = 7
	BPCM_SR_CONTROL sr_control;	// offset = 0x20, actual offset = 8
	uint32_t reserved0[3];	// offset = 0x24, actual offset = 9
	ARM_CONTROL_REG arm_control;	// offset = 0x30, actual offset = 12
	uint32_t biu_clk_control0;	// offset = 0x34, actual offset = 13
	uint32_t reserved1[2];	// offset = 0x38, actual offset = 14
	BPCM_ZONE zones;	// offset = 0x40, actual offset = 16
} ARM_BPCM_REGS;

#define ARMBPCMOffset(reg)  offsetof(ARM_BPCM_REGS,reg)
#define ARMBPCMRegOffset(reg)   (ARMBPCMOffset(reg) >> 2)

#elif IS_BCMCHIP(63178) || IS_BCMCHIP(47622) || IS_BCMCHIP(6878) || IS_BCMCHIP(6855) || IS_BCMCHIP(6756)
typedef struct {
	BPCM_ID_REG id_reg;	// offset = 0x00, actual offset = 0
	BPCM_CAPABILITES_REG capabilities;	// offset = 0x04, actual offset = 1
	uint32_t reserved0[2];	// offset = 0x08, actual offset = 2
	uint32_t cfg_control;	// offset = 0x10, actual offset = 4
	BPCM_SR_CONTROL sr_control;	// offset = 0x14, actual offset = 5
	uint32_t reserved1[6];	// offset = 0x18, actual offset = 6
	ARM_CONTROL_REG arm_control;	// offset = 0x30, actual offset = 12
	uint32_t biu_clk_control0;	// offset = 0x34, actual offset = 13
	uint32_t tbd[18];	// offset = 0x38, actual offset = 14
	BPCM_ZONE zones;	// offset = 0x80, actual offset = 32
} ARM_BPCM_REGS;
#define ARMBPCMOffset(reg)  offsetof(ARM_BPCM_REGS,reg)
#define ARMBPCMRegOffset(reg)   (ARMBPCMOffset(reg) >> 2)

#elif IS_BCMCHIP(63146) || IS_BCMCHIP(4912) || IS_BCMCHIP(6813)
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

#endif

#if IS_BCMCHIP(6846) || IS_BCMCHIP(6856)
typedef struct {
	BPCM_ID_REG id_reg;	// offset = 0x00, actual offset = 0
	BPCM_CAPABILITES_REG capabilities;	// offset = 0x04, actual offset = 1
	uint32_t reserved0[2];	// offset = 0x08..0x0c, actual offset 2..3
	PLL_CTRL_REG resets;	// offset = 0x10, actual offset = 4
	PLL_CFG0_REG cfg0;	// offset = 0x14, actual offset = 5
	PLL_CFG1_REG cfg1;	// offset = 0x18, actual offset = 6
	PLL_NDIV_REG ndiv;	// offset = 0x1c, actual offset = 7
	PLL_PDIV_REG pdiv;	// offset = 0x20, actual offset = 8
	PLL_LOOP0_REG loop0;	// offset = 0x24, actual offset = 9
	PLL_LOOP1_REG loop1;	// offset = 0x28, actual offset = a
	PLL_CHCFG_REG ch01_cfg;	// offset = 0x2c, actual offset = b
	PLL_CHCFG_REG ch23_cfg;	// offset = 0x30, actual offset = c
	PLL_CHCFG_REG ch45_cfg;	// offset = 0x34, actual offset = d
	PLL_OCTRL_REG octrl;	// offset = 0x38, actual offset = e
	PLL_STAT_REG stat;	// offset = 0x3c, actual offset = f
	uint32_t strap;		// offset = 0x40, actual offset = 0x10
	PLL_DECNDIV_REG decndiv;	// offset = 0x44, actual offset = 0x11
	PLL_DECPDIV_REG decpdiv;	// offset = 0x48, actual offset = 0x12
	PLL_DECCH25_REG decch25;	// offset = 0x4c, actual offset = 0x13
} PLL_CLASSIC_BPCM_REGS;

#define PLLCLASSICBPCMOffset(reg)  offsetof(PLL_CLASSIC_BPCM_REGS,reg)
#define PLLCLASSICBPCMRegOffset(reg)   (PLLCLASSICBPCMOffset(reg) >> 2)

#elif IS_BCMCHIP(6878)
typedef struct {
	BPCM_ID_REG id_reg;	// offset = 0x00
	BPCM_CAPABILITES_REG capabilities;	// offset = 0x04
	uint32_t reserved0[6];	// offset = 0x08
	PLL_CTRL_REG resets;	// offset = 0x20
	PLL_CFG0_REG cfg0;	// offset = 0x24
	PLL_CFG1_REG cfg1;	// offset = 0x28
	PLL_NDIV_REG ndiv;	// offset = 0x2c
	PLL_PDIV_REG pdiv;	// offset = 0x30
	PLL_LOOP0_REG loop0;	// offset = 0x34
	uint32_t reserved1;	// offset = 0x38
	PLL_LOOP1_REG loop1;	// offset = 0x3c
	PLL_CHCFG_REG ch01_cfg;	// offset = 0x40
	PLL_CHCFG_REG ch23_cfg;	// offset = 0x44
	PLL_CHCFG_REG ch45_cfg;	// offset = 0x48
	PLL_STAT_REG stat;	// offset = 0x4c
	uint32_t strap;		// offset = 0x50
	PLL_DECNDIV_REG decndiv;	// offset = 0x54
	PLL_DECPDIV_REG decpdiv;	// offset = 0x58
	PLL_DECCH25_REG decch25;	// offset = 0x5c
} PLL_CLASSIC_BPCM_REGS;

#define PLLCLASSICBPCMOffset(reg)  offsetof(PLL_CLASSIC_BPCM_REGS,reg)
#define PLLCLASSICBPCMRegOffset(reg)   (PLLCLASSICBPCMOffset(reg) >> 2)

#endif

typedef struct {
#if IS_BCMCHIP(6846) || IS_BCMCHIP(6856)
	BPCM_ID_REG id_reg;	// offset = 0x00
	BPCM_CAPABILITES_REG capabilities;	// offset = 0x04
	uint32_t reserved0[6];	// offset = 0x08
	PLL_CTRL_REG resets;	// offset = 0x20
	PLL_CFG0_REG cfg0;	// offset = 0x24
	PLL_CFG1_REG cfg1;	// offset = 0x28
	PLL_NDIV_REG ndiv;	// offset = 0x2c
	PLL_PDIV_REG pdiv;	// offset = 0x30
	PLL_LOOP0_REG loop0;	// offset = 0x34
	uint32_t reserved1;	// offset = 0x38
	PLL_LOOP1_REG loop1;	// offset = 0x3c
	PLL_CHCFG_REG ch01_cfg;	// offset = 0x40
	PLL_CHCFG_REG ch23_cfg;	// offset = 0x44
	PLL_CHCFG_REG ch45_cfg;	// offset = 0x48
	PLL_STAT_REG stat;	// offset = 0x4c
	uint32_t strap;		// offset = 0x50
	PLL_DECNDIV_REG decndiv;	// offset = 0x54
	PLL_DECPDIV_REG decpdiv;	// offset = 0x58
	PLL_DECCH25_REG decch25;	// offset = 0x5c
#elif IS_BCMCHIP(63146) || IS_BCMCHIP(4912) || IS_BCMCHIP(6813)
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
#else
	BPCM_ID_REG id_reg;	// offset = 0x00, actual offset = 0
	BPCM_CAPABILITES_REG capabilities;	// offset = 0x04, actual offset = 1
	uint32_t reserved0[2];	// offset = 0x08..0x0c, actual offset 2..3
	PLL_CTRL_REG resets;	// offset = 0x10, actual offset = 4
	PLL_CFG0_REG cfg0;	// offset = 0x14, actual offset = 5
	PLL_CFG1_REG cfg1;	// offset = 0x18, actual offset = 6
	PLL_NDIV_REG ndiv;	// offset = 0x1c, actual offset = 7
	PLL_PDIV_REG pdiv;	// offset = 0x20, actual offset = 8
	PLL_LOOP0_REG loop0;	// offset = 0x24, actual offset = 9
	PLL_LOOP1_REG loop1;	// offset = 0x28, actual offset = a
	PLL_CHCFG_REG ch01_cfg;	// offset = 0x2c, actual offset = b
	PLL_CHCFG_REG ch23_cfg;	// offset = 0x30, actual offset = c
	PLL_CHCFG_REG ch45_cfg;	// offset = 0x34, actual offset = d
	PLL_OCTRL_REG octrl;	// offset = 0x38, actual offset = e
	PLL_STAT_REG stat;	// offset = 0x3c, actual offset = f
	uint32_t strap;		// offset = 0x40, actual offset = 0x10
#if IS_BCMCHIP(6858) || IS_BCMCHIP(63178) || IS_BCMCHIP(47622) || \
	IS_BCMCHIP(6878) || IS_BCMCHIP(6855) || IS_BCMCHIP(6756)
	PLL_DECNDIV_REG decndiv;	// offset = 0x44, actual offset = 0x11
	PLL_DECPDIV_REG decpdiv;	// offset = 0x48, actual offset = 0x12
	PLL_DECCH25_REG decch25;	// offset = 0x4c, actual offset = 0x13
#else
	uint32_t decndiv;	// offset = 0x44, actual offset = 0x11
	uint32_t decpdiv;	// offset = 0x48, actual offset = 0x12
	uint32_t decch25;	// offset = 0x4c, actual offset = 0x13
#endif
#endif
} PLL_BPCM_REGS;

#define PLLBPCMOffset(reg)	offsetof(PLL_BPCM_REGS,reg)
#define PLLBPCMRegOffset(reg)	(PLLBPCMOffset(reg) >> 2)

#if IS_BCMCHIP(63146)
typedef struct {
	BPCM_ID_REG id_reg;	// offset = 0x00, actual offset = 0
	BPCM_CAPABILITES_REG capabilities;	// offset = 0x04, actual offset = 1
	uint32_t reserved0[2];	// offset = 0x08..0x0c, actual offset 2..3
	PLL_CTRL_REG resets;	// offset = 0x10, actual offset = 4
	uint32_t reserved1[5];	// offset = 0x14, actual offset = 5
	PLL_NDIV_REG ndiv;	// offset = 0x28, actual offset = a
	PLL_PDIV_REG pdiv;	// offset = 0x2c, actual offset = b
	PLL_CHCFG_REG ch01_cfg;	// offset = 0x30, actual offset = c
	PLL_CHCFG_REG ch23_cfg;	// offset = 0x34, actual offset = d
	PLL_CHCFG_REG ch45_cfg;	// offset = 0x38, actual offset = e
	PLL_LOOP0_REG loop0;	// offset = 0x3c, actual offset = f
	PLL_LOOP1_REG loop1;	// offset = 0x40, actual offset = 0x10
	PLL_STAT_REG stat;	// offset = 0x44, actual offset = 0x11
	uint32_t strap;		// offset = 0x48, actual offset = 0x12
	PLL_DECNDIV_REG decndiv;// offset = 0x4c, actual offset = 0x13
	PLL_DECPDIV_REG decpdiv;// offset = 0x50, actual offset = 0x14
	PLL_DECCH25_REG decch25;// offset = 0x54, actual offset = 0x15
} AFEPLL_BPCM_REGS;

#define AFEPLLBPCMOffset(reg)		offsetof(AFEPLL_BPCM_REGS,reg)
#define AFEPLLBPCMRegOffset(reg)	(AFEPLLBPCMOffset(reg) >> 2)
#endif

#if IS_BCMCHIP(63146) || IS_BCMCHIP(4912) || IS_BCMCHIP(6813)
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

#endif

#if IS_BCMCHIP(6878) || IS_BCMCHIP(6855)
typedef union {
	struct {
		uint32_t dac_data:10;	// [09:00]
		uint32_t vavs_minb0:1;	// [10:10] - R/O iVDDC <= Vmin0
		uint32_t vavs_minb1:1;	// [11:11] - R/O iVDDC <= Vmin1
		uint32_t vavs_warnb0:1;	// [12:12] - R/O iVDDC <= Vwarn0
		uint32_t vavs_warnb1:1;	// [13:13] - R/O iVDDC <= Vwarn1
		uint32_t vavs_maxb0:1;	// [14:14] - R/O iVDDC <= Vmax0
		uint32_t vavs_maxb1:1;	// [15:15] - R/O iVDDC <= Vmax1
		uint32_t adc_data:10;	// [25:16] - R/O ADC output data in offset binary format
		uint32_t adc_data_valid:1;	// [26:26] - R/O
		uint32_t reserved:5;	// [31:27] - R/O 
	} Bits;
	uint32_t Reg32;
} APVTMON_DATA_REG;

typedef union {
	// little endian - from page 5 of "ANA_VTMON_TS16FF_S0 & ANA_VTMON_PAD_TS16FF_Sx Module Specification"
	// defaut value = 0x00000001
	struct {
		uint32_t bg_adj:3;	// [02:00] - default = 1
		uint32_t vtest_sel:4;	// [06:03] - VTest = i_VDCC * (<value>+1)/20, default = 0
		uint32_t rmon_sel:3;	// [09:07]
		uint32_t mode:3;	// [12:10]
		uint32_t adc_insel:2;	// [14:13] - only used in expert mode (mode = 0b111)
		uint32_t dac_en:1;	// [15:15] - only used in expert mode (mode = 0b111)
		uint32_t con_pad:1;	// [16:16] - only used in expert mode (mode = 0b111)
		uint32_t burnin_en:1;	// [17:17] - only used in expert mode (mode = 0b111)
		uint32_t reserved:1;	// [18:18]
		uint32_t vdccmon_refadj_max1:1;	// [19:19]
		uint32_t vdccmon_refadj_min0:4;	// [23:20]
		uint32_t vdccmon_refadj_min1:3;	// [26:24]
		uint32_t dac_reset:1;	// [27:27]
		uint32_t dac_set:1;	// [28:28]
		uint32_t vdccmon_refadj_max0:3;	// [31:29]
	} Bits;
	uint32_t Reg32;
} APVTMON_CONTROL_REG;

typedef union {
	struct {
		uint32_t rstb:1;	// [00:00] - low active.  default = 0 (i.e. in reset)
		uint32_t pwr_dn:1;	// [01:01] - high-active.  default = 1 (i.e. powered down)
		uint32_t clk_en:1;	// [02:02]
		uint32_t reserved0:1;	// [03:03]
		uint32_t sel:3;	// [06:04] - see enum below - reset value = 0
		uint32_t reserved1:1;	// [07:07]
		uint32_t clk_div:5;	// [12:08] - value needed to divide pm_clk by (2*clk_div) to generate a 5MHz clock
		uint32_t reserved2:19;	// [31:13]
	} Bits;
	uint32_t Reg32;
} APVTMON_CONFIG_STATUS_REG;

typedef union {
	struct {
		uint32_t accum_en:1;	// [00:00]
		uint32_t round_en:1;	// [01:01] defaults to 1 (rounding enabled)
		uint32_t reserved1:6;	// [07:02]
		uint32_t skip_len:4;	// [11:08] how many samples to skip prior to starting averaging, default = 3
		uint32_t reserved0:20;	// [31:12]
	} Bits;
	uint32_t Reg32;
} APVTMON_ACQ_CONFIG_REG;

typedef union {
	struct {
		uint32_t warn_threshold:10;	// [09:00] - in ADC counts
		uint32_t warn_en:1;	// [10:10]
		uint32_t reserved0:3;	// [13:11]
		uint32_t clear_warn:1;	// [14:14] - Write only
		uint32_t warn:1;	// [15:15] - Read only
		uint32_t reset_threshold:10;	// [25:16] - in ADC counts
		uint32_t reset_en:1;	// [26:26]
		uint32_t reserved1:3;	// [29:27]
		uint32_t clear_reset:1;	// [30:30] - Write only
		uint32_t reset:1;	// [31:31] - Read only
	} Bits;
	uint32_t Reg32;
} APVTMON_TEMP_WARN_RESET_REG;

typedef union {
	struct {
		uint32_t reset_value:10;	// [09:00]
		uint32_t reserved:22;	// [31:10]
	} Bits;
	uint32_t Reg32;
} APVTMON_RESET_TEMP_REG;

typedef union {
	struct {
		uint32_t value:10;	// [09:00] - there are <meas_len> fractional bits
		uint32_t reserved0:8;	// [17:10]
		uint32_t valid:1;	// [18:18]
		uint32_t busy:1;	// [19:19]
		uint32_t reserved1:4;	// [23:20]
		uint32_t meas_len:3;	// [26:24]  #samples = 2^<value>
		uint32_t reserved2:4;	// [30:27]
		uint32_t enable:1;	// [31:31]
	} Bits;
	uint32_t Reg32;
} APVTMON_ACCUM_REG;

typedef union {
	struct {
		uint32_t sel:6;	// [05:00] - ring oscillator select (0..35)
		uint32_t reserved2:2;	// [07:06]
		uint32_t srm_ind_en:1;	// [08:08]
		uint32_t srm_ind_od:1;	// [09:09]
		uint32_t srm_ind_sel:2;	// [11:10]
		uint32_t reserved1:4;	// [15:12]
		uint32_t out:1;	// [16:16]
		uint32_t all_idl_low_oscs:1;	// [17:17]
		uint32_t all_idl_hi_oscs:1;	// [18:18]
		uint32_t reserved0:13;	// [31:19]
	} Bits;
	uint32_t Reg32;
} ROSC_CTRL_STS_REG;

typedef union {
	struct {
		uint32_t count:16;	// [15:00]
		uint32_t valid:1;	// [16:16]
		uint32_t too_lo:1;	// [17:17] - count <= thresh_lo (only when THRESH_EN == 1)
		uint32_t too_hi:1;	// [18:18] - count <= thresh_hi (only when THRESH_EN == 1)
		uint32_t reserved0:5;	// [23:19]
		uint32_t continuous:1;	// [24:24]
		uint32_t thresh_en:1;	// [25:25] - enable threshold detection
		uint32_t ectr_en:1;	// [26:26] - enable counter
		uint32_t src_en:1;	// [27:27] - enable event source (may not do anything???)
		uint32_t meas_len:4;	// [31:28] - interval = 2^(<meas_len>+1)
	} Bits;
	uint32_t Reg32;
} ECTR_CTRL_STS_REG;

typedef union {
	struct {
		uint32_t thresh_lo:16;	// [15:00]
		uint32_t thresh_hi:16;	// [31:16]
	} Bits;
	uint32_t Reg32;
} ECTR_THRESH_REG;

typedef struct {
	ECTR_CTRL_STS_REG count_reg;
	ECTR_THRESH_REG thresh_reg;
} ROSC_REGS;

typedef struct {
	BPCM_ID_REG id_reg;	// offset 0x00, PMB reg index 0
	BPCM_CAPABILITES_REG capabilities;	// offset 0x04, PMB reg index 1
	uint32_t reserved0[2];	// offset 0x08, PMB reg index 2/3
	uint32_t reserved1[12];	// offset 0x10, PMB reg index 4-15 (future proofing )
	APVTMON_CONTROL_REG control;	// offset 0x40, PMB reg index 16
	APVTMON_CONFIG_STATUS_REG config;	// offset 0x44, PMB reg index 17
	APVTMON_DATA_REG adc_data;	// offset 0x48, PMB reg index 18
	uint32_t reserved2;	// offset 0x4c, PMB reg index 19
	APVTMON_ACQ_CONFIG_REG accum_config;	// offset 0x50, PMB reg index 20
	APVTMON_TEMP_WARN_RESET_REG warn_rst;	// offset 0x54, PMB reg index 21
	uint32_t reserved3[2];	// offset 0x58, PMB reg index 23
	APVTMON_ACCUM_REG acq_accum_regs[8];	// offset 0x60, PMB reg index 24-31
	ROSC_CTRL_STS_REG rosc_ctrl_sts;	// offset 0x80, PMB reg index 32
	uint32_t rosc_en_lo;	// offset 0x84, PMB reg index 33
	uint32_t rosc_en_hi;	// offset 0x88, PMB reg index 34
	uint32_t rosc_idle_lo;	// offset 0x8c, PMB reg index 35
	uint32_t rosc_idle_hi;	// offset 0x90, PMB reg index 36
	uint32_t reserved4[3];	// offset 0x94, PMB reg index 37-39
	ROSC_REGS ectr_regs;	// offset 0xa0, PMB reg index 40/41
} PVTMON_REGS;
// retrieves the BYTE offset of a PVTMON register:
#define PVTMON_OFFSET(reg) (offsetof(PVTMON_REGS,reg)>>2)

typedef struct {
// PMB-slave
	BPCM_ID_REG id_reg;	// offset 0x00, PMB reg index 0
	BPCM_CAPABILITES_REG capabilities;	// offset 0x04, PMB reg index 1
	uint32_t reserved0[6];	// offset 0x08, PMB reg index 2-7
	// ROSC registers
	BPCM_AVS_ROSC_CONTROL_REG rosc_control;	// offset 0x20, PMB reg index 8
	BPCM_AVS_ROSC_THRESHOLD rosc_thresh_h;	// offset 0x24, PMB reg index 9
	BPCM_AVS_ROSC_THRESHOLD rosc_thresh_s;	// offset 0x28, PMB reg index 10
	BPCM_AVS_ROSC_COUNT rosc_count;	// offset 0x2c, PMB reg index 11
	BPCM_AVS_PWD_CONTROL pwd_ctrl;	// offset 0x30, PMB reg index 12
	BPCM_PWD_ACCUM_CONTROL pwd_accum;	// offset 0x34, PMB reg index 13
} ARS_REGS;
// retrieves the BYTE offset of an ARS register:
#define ARS_OFFSET(reg) (offsetof(ARS_REGS, reg)>>2)

#endif

typedef struct {
	BPCM_UBUS_ID_REG id_reg;	/* offset = 0x00, actual offset = 0 */
	BPCM_UBUS_CAPABILITES_REG capabilities;	/* offset = 0x04, actual offset = 1 */
	uint32_t reserved0;	/* offset = 0x08, actual offset = 2 */
	BPCM_UBUS_CTRL_REG ctrl;	/* offset = 0x0c, actual offset = 3 */
	BPCM_UBUS_CFG_REG cfg[4];	/* offset = 0x10..0x2c, actual offset = 4..11 */
} BPCM_UBUS_REG;

#define UBUSBPCMOffset(reg)	offsetof(BPCM_UBUS_REG,reg)
#define UBUSBPCMRegOffset(reg)	(UBUSBPCMOffset(reg) >> 2)

#if IS_BCMCHIP(63178) || IS_BCMCHIP(47622) || IS_BCMCHIP(6756)
typedef struct {
	// PMB-slave:
	BPCM_ID_REG id_reg;	// offset 0x00, PMB reg index 0
	BPCM_CAPABILITES_REG capabilities;	// offset 0x04, PMB reg index 1
	uint32_t reserved0[7];	// offset 0x08-0x20, PMB reg index 2-8
	uint32_t control;	// offset 0x24, PMB reg index 9
#if IS_BCMCHIP(47622) || IS_BCMCHIP(6756)
	uint32_t observe_cntrl;	// offset 0x28, PMB reg index 10
	uint32_t observe_div;	// offset 0x2c, PMB reg index 11
	uint32_t observe_enable;	// offset 0x30, PMB reg index 12
	BPCM_CLKRST_CONTROL clkrst_control;	// offset 0x34, PMB reg index 13
#elif IS_BCMCHIP(63146)
	uint32_t unused[3];	// offset 0x28, PMB reg index 10
	BPCM_CLKRST_VREG_CONTROL vreg_control;	// offset 0x34, PMB reg index ??
#elif IS_BCMCHIP(63178)
	uint32_t unused[10];	// offset 0x28, PMB reg index 10
	BPCM_CLKRST_VREG_CONTROL vreg_control;	// offset 0x50, PMB reg index 19
#endif
} BPCM_CLKRST_REGS;
#else
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
#if IS_BCMCHIP(6878)
	uint32_t bpcm_ctrl;	// offset = 0x24, actual offset = 9
	uint32_t clkrst_control;	// offset = 0x28, actual offset = 10
	uint32_t ext_observe_ctrl;	// offset = 0x2c, actual offset = 11
	uint32_t reserved0[2];	// offset = 0x30-0x34, actual offset 12-13
	uint32_t xtal_control;	// offset = 0x38, actual offset = 14
	uint32_t clkrst_stat;	// offset = 0x3c, actual offset = 15
	uint32_t reserved1;	// offset = 0x40, actual offset = 16
	uint32_t clkrst_ena_clk_31_0;	// offset = 0x44, actual offset = 17
	uint32_t clkrst_ena_clk_63_32;	// offset = 0x48, actual offset = 18
	uint32_t clkrst_ena_clk_95_64;	// offset = 0x4c, actual offset = 19
	uint32_t reserved2[11];	// offset = 0x50-0x78, actual offset = 20-30
	uint32_t clkrst_ena_reset_31_0;	// offset = 0x7c, actual offset = 31
	uint32_t clkrst_ena_reset_63_32;	// offset = 0x80, actual offset = 32
	uint32_t clkrst_pll_observe_clk;	// offset = 0x84, actual offset = 33
	uint32_t clkrst_ref_cnt_thresh;	// offset = 0x88, actual offset = 34
	uint32_t clkrst_pll_clk_low_th;	// offset = 0x8c, actual offset = 35
	uint32_t clkrst_pll_clk_hi_th;	// offset = 0x90, actual offset = 36
	uint32_t clkrst_pll_clk_stat;	// offset = 0x94, actual offset = 37
	uint32_t clkrst_sticky_bit_stat;	// offset = 0x98, actual offset = 38
	uint32_t clkrst_clk250_src_sel;	// offset = 0x9c, actual offset = 39
	uint32_t clkrst_ena_force;	// offset = 0xa0, actual offset = 40
	uint32_t reserved3;	// offset = 0xa4, actual offset = 41
	uint32_t pmd_xtal_cntl;	// offset = 0xa8, actual offset = 42
	uint32_t pmd_xtal_cntl2;	// offset = 0xac, actual offset = 43
#else
	uint32_t reserved0;	// offset = 0x24, actual offset = 9
	BPCM_SR_CONTROL sr_control;	// offset = 0x28, actual offset = 10
	uint32_t reserved1;	// offset = 0x2c, actual offset = 11
	uint32_t clkrst_cfg;	// offset = 0x30, actual offset = 12
#if IS_BCMCHIP(63146)
	BPCM_CLKRST_VREG_CONTROL vreg_control;	// offset 0x50, PMB reg index 19
#else
	uint32_t clkrst_control;	// offset = 0x34, actual offset = 13
#endif
	uint32_t xtal_control;	// offset = 0x38, actual offset = 14
	uint32_t clkrst_stat;	// offset = 0x3c, actual offset = 15
#endif
} BPCM_CLKRST_REGS;
#endif

#define CLKRSTBPCMOffset(reg)  offsetof(BPCM_CLKRST_REGS, reg)
#define CLKRSTBPCMRegOffset(reg)   (CLKRSTBPCMOffset(reg) >> 2)

#if IS_BCMCHIP(47622) || IS_BCMCHIP(6756)
typedef struct {
	BPCM_ID_REG id_reg;	// offset 0x00, PMB reg index 0
	BPCM_CAPABILITES_REG capabilities;	// offset 0x04, PMB reg index 1
	uint32_t reserved0[2];	// offset 0x08, PMB reg index 2/3
	// BPCM
	uint32_t control;	// offset 0x10, PMB reg index 4
	BPCM_SR_CONTROL sr_control;	// offset 0x14, PMB reg index 5

	uint32_t z0_pm_cntl;	// offset 0x18
	uint32_t z0_pm_status;	// offset 0x1c
	uint32_t z1_pm_cntl;	// offset 0x20
	uint32_t z2_pm_cntl;	// offset 0x24
	uint32_t reserved1[22];	// reserved from 0x28 to 0x7F
	BPCM_ZONE zones[];

} BPCM_SYSPORT_REGS;

#define SYSPOffset(reg)		offsetof(BPCM_SYSPORT_REGS,reg)
#define SYSPRegOffset(reg)	(SYSPOffset(reg) >> 2)

#endif

#if IS_BCMCHIP(63146) || IS_BCMCHIP(4912) || IS_BCMCHIP(6813)
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
#if IS_BCMCHIP(63146)
	uint32_t qphy_cntrl;	// offset 0x20
	uint32_t qphy_status;	// offset 0x24
	uint32_t sphy_cntrl;	// offset 0x28
	uint32_t sphy_status;	// offset 0x2c
	uint32_t serdes0_cntrl;	// offset 0x30
	uint32_t serdes0_status;// offset 0x34
	uint32_t serdes0_an_st; // offset 0x38
	uint32_t serdes1_cntrl;	// offset 0x3c
	uint32_t serdes1_status;// offset 0x40
	uint32_t serdes1_an_st; // offset 0x44
	uint32_t reserved48[3];
	uint32_t rgmii_cntrl;   // offset 0x54
	uint32_t reserved58[2];
	uint32_t serdes0_st1;   // offset 0x60
	uint32_t serdes1_st1;   // offset 0x64
	uint32_t reserved68;
	uint32_t serdes0_pwr_cntrl;// offset 0x6c
	uint32_t serdes1_pwr_cntrl;// offset 0x70
#endif
#if IS_BCMCHIP(4912) || IS_BCMCHIP(6813)
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
#endif
} BPCM_ETH_REGS;

#define BPCMETHOffset(reg)		offsetof(BPCM_ETH_REGS,reg)
#define BPCMETHRegOffset(reg)	(BPCMETHOffset(reg) >> 2)

#endif

// *************************** macros ******************************
#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((unsigned long) &((TYPE *)0)->MEMBER)
#endif

#endif
#endif
#endif
