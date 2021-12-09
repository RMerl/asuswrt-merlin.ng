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

#include "bcmtypes.h"

#ifndef NULL
	#define NULL ((void *)0)
#endif


typedef union
{
	struct {
#if defined(_BCM96836_) || defined(CONFIG_BCM96836) || defined(_BCM96846_) || defined(CONFIG_BCM96846) || defined(_BCM96856_) || defined(CONFIG_BCM96856)
        uint32  pmbAddr     : 12;   // [11:00]
        uint32  map_rev     : 4;    // [15:12] always non-zero for AVS3 devices (see CLASSIC_BPCM_ID_REG below)
        uint32  island      : 4;    // [19:16]
        uint32  devType     : 4;    // [23:20] see enum above
        uint32  hw_rev      : 8;    // [31:24]
#else
#ifdef PMC_LITTLE_ENDIAN
		uint32 pmb_Addr	: 8;
		uint32 hw_rev	: 8;
		uint32 sw_strap	: 16;
#else
		uint32 sw_strap	: 16;
		uint32 hw_rev	: 8;
		uint32 pmb_Addr	: 8;
#endif
#endif
	} Bits;
	uint32 Reg32;
} BPCM_ID_REG;

#if defined(_BCM96836_) || defined(CONFIG_BCM96836) || defined(_BCM96846_) || defined(CONFIG_BCM96846) || defined(_BCM96856_) || defined(CONFIG_BCM96856)

typedef union
{
    struct {
        uint32  pmb_Addr    : 8;    // [07:00]
        uint32  hw_rev      : 4;    // [11:08]
        uint32  map_rev     : 4;    // [15:12] always zero for classic devices (technically upper 4-bits of hw_rev)
        uint32  sw_strap    : 16;   // [31:16]
    } Bits;
    uint32 Reg32;
} CLASSIC_BPCM_ID_REG; 
#endif

// types of PMB devices
enum {
	kPMB_BPCM = 0,
	kPMB_MIPS_PLL = 1,
	kPMB_GEN_PLL = 2,
	kPMB_LC_PLL = 3,
	// 4..15 reserved
};

#if defined(_BCM96838_) || defined(CONFIG_BCM96838)
typedef union
{
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32 num_zones	: 8;
		uint32 reserved2	: 8;
		uint32 devType		: 4;	// see enum above
		uint32 reserved1	: 12;
#else
		uint32 reserved1	: 12;
		uint32 devType		: 4;	// see enum above
		uint32 reserved2	: 8;
		uint32 num_zones	: 8;
#endif
	} Bits;
	uint32 Reg32;
} BPCM_CAPABILITES_REG;
#else
typedef union
{
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32 num_zones	: 8;
		uint32 num_sr_bits	: 8;
		uint32 devType		: 4;	// see enum above
		uint32 reserved1	: 12;
#else
		uint32 reserved1	: 12;
		uint32 devType		: 4;	// see enum above
		uint32 num_sr_bits	: 8;
		uint32 num_zones	: 8;
#endif
	} Bits;
	uint32 Reg32;
} BPCM_CAPABILITES_REG;
#endif

typedef union
{
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32 pwd_alert	: 1;
		uint32 reserved		: 31;
#else
		uint32 reserved		: 31;
		uint32 pwd_alert	: 1;
#endif
	} Bits;
	uint32 Reg32;
} BPCM_STATUS_REG;

typedef union
{
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32 ro_en_s		: 1;
		uint32 ro_en_h		: 1;
		uint32 ectr_en_s	: 1;
		uint32 ectr_en_h	: 1;
		uint32 thresh_en_s	: 1;
		uint32 thresh_en_h	: 1;
		uint32 continuous_s	: 1;
		uint32 continuous_h	: 1;
		uint32 reserved		: 4;
		uint32 valid_s		: 1;
		uint32 alert_s		: 1;
		uint32 valid_h		: 1;
		uint32 alert_h		: 1;
		uint32 interval		: 16;
#else
		uint32 interval		: 16;
		uint32 alert_h		: 1;
		uint32 valid_h		: 1;
		uint32 alert_s		: 1;
		uint32 valid_s		: 1;
		uint32 reserved		: 4;
		uint32 continuous_h	: 1;
		uint32 continuous_s	: 1;
		uint32 thresh_en_h	: 1;
		uint32 thresh_en_s	: 1;
		uint32 ectr_en_h	: 1;
		uint32 ectr_en_s	: 1;
		uint32 ro_en_h		: 1;
		uint32 ro_en_s		: 1;
#endif
	} Bits;
	uint32 Reg32;
} BPCM_AVS_ROSC_CONTROL_REG;

typedef union
{
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32 thresh_lo	: 16;
		uint32 thresh_hi	: 16;
#else
		uint32 thresh_hi	: 16;
		uint32 thresh_lo	: 16;
#endif
	} Bits;
	uint32 Reg32;
} BPCM_AVS_ROSC_THRESHOLD;

typedef union
{
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32 count_s	: 16;
		uint32 count_h	: 16;
#else
		uint32 count_h	: 16;
		uint32 count_s	: 16;
#endif
	} Bits;
	uint32 Reg32;
} BPCM_AVS_ROSC_COUNT;

typedef union
{
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32 pwd_en		: 1;
		uint32 pwd_alert_sel	: 1;
		uint32 start		: 6;
		uint32 pwd_tm_en	: 1;
		uint32 reserved2	: 6;
		uint32 alert		: 1;
		uint32 ccfg		: 8;
		uint32 rsel		: 3;
		uint32 clr_cfg		: 3;
		uint32 reserved1	: 2;
#else
		uint32 reserved1	: 2;
		uint32 clr_cfg		: 3;
		uint32 rsel		: 3;
		uint32 ccfg		: 8;
		uint32 alert		: 1;
		uint32 reserved2	: 6;
		uint32 pwd_tm_en	: 1;
		uint32 start		: 6;
		uint32 pwd_alert_sel	: 1;
		uint32 pwd_en		: 1;
#endif
	} Bits;
	uint32 Reg32;
} BPCM_AVS_PWD_CONTROL;

#if !defined(_BCM96838_) && !defined(CONFIG_BCM96838)
typedef union
{
	struct {
		uint32 tbd		: 32;
	} Bits;
	uint32 Reg32;
} BPCM_PWD_ACCUM_CONTROL;
#endif

typedef union
{
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32 sr		: 8;
		uint32 gp		: 24;
#else
		uint32 gp		: 24;
		uint32 sr		: 8;
#endif
	} Bits;
#if defined (CONFIG_BCM96858) || defined(_BCM96858_)
	struct
	{
#ifdef PMC_LITTLE_ENDIAN
		uint32 wan_main_rst_n:1;
		uint32 wan_top_bb_rst_n:1;
		uint32 epon_core_rst_n:1;
		uint32 epon_rx_rclk16_sw_reset_n:1;
		uint32 epon_rx_rbc125_sw_reset_n:1;
		uint32 epon_tx_tclk16_sw_reset_n:1;
		uint32 epon_tx_clk125_sw_reset_n:1;
		uint32 gpon_main_rst_n:1;
		uint32 gpon_rx_rst_n:1;
		uint32 gpon_tx_rst_n:1;
		uint32 gpon_8khz_rst_n:1;
		uint32 ngpon_main_rst_n:1;
		uint32 ngpon_rx_rst_n:1;
		uint32 ngpon_tx_rst_n:1;
		uint32 ngpon_8khz_rst_n:1;
		uint32 gpon_nco_rst_n:1;
		uint32 apm_rst_n:1;
		uint32 reserved:15;
#else
		uint32 reserved:15;
		uint32 apm_rst_n:1;
		uint32 gpon_nco_rst_n:1;
		uint32 ngpon_8khz_rst_n:1;
		uint32 ngpon_tx_rst_n:1;
		uint32 ngpon_rx_rst_n:1;
		uint32 ngpon_main_rst_n:1;
		uint32 gpon_8khz_rst_n:1;
		uint32 gpon_tx_rst_n:1;
		uint32 gpon_rx_rst_n:1;
		uint32 gpon_main_rst_n:1;
		uint32 epon_tx_clk125_sw_reset_n:1;
		uint32 epon_tx_tclk16_sw_reset_n:1;
		uint32 epon_rx_rbc125_sw_reset_n:1;
		uint32 epon_rx_rclk16_sw_reset_n:1;
		uint32 epon_core_rst_n:1;
		uint32 wan_top_bb_rst_n:1;
		uint32 wan_main_rst_n:1;
#endif
	}Bits_Wantop;
#endif
#if defined (CONFIG_BCM963158) || defined(_BCM963158_)
	struct
	{
#ifdef PMC_LITTLE_ENDIAN
		uint32 wan_main_rst_n:1;
		uint32 wan_top_bb_rst_n:1;
		uint32 rbus_rst_n:1;
		uint32 reserved1:2;
		uint32 core_soft_rst_n:1;
		uint32 i_zn_sw_init:1;
		uint32 reserved2:1;
		uint32 epon_main_rst_n:1;
		uint32 epon_rx_rst_n:1;
		uint32 epon_tx_rst_n:1;
		uint32 epon_core_rst_n:1;
		uint32 ae_rx_rclk16_sw_reset_n:1;
		uint32 ae_rx_rbc125_sw_reset_n:1;  /* for B0 */
		uint32 ae_tx_tclk16_sw_reset_n:1;  /* for B0 */
		uint32 ae_tx_clk125_sw_reset_n:1;
		uint32 gpon_main_rst_n:1;
		uint32 gpon_rx_rst_n:1;
		uint32 gpon_tx_rst_n:1;
		uint32 gpon_8khz_rst_n:1;
		uint32 ngpon_main_rst_n:1;
		uint32 ngpon_rx_rst_n:1;
		uint32 ngpon_tx_rst_n:1;
		uint32 ngpon_8khz_rst_n:1;
		uint32 reserved3:2;
		uint32 gpon_nco_rst_n:1;
		uint32 epon_rx_rclk16_sw_reset_n:1; /* for B0 */
		uint32 epon_rx_rbc125_sw_reset_n:1; /* for B0 */
		uint32 epon_tx_tclk16_sw_reset_n:1; /* for B0 */
		uint32 epon_tx_clk125_sw_reset_n:1; /* for B0 */
		uint32 reserved4:1;
#else
		uint32 reserved4:1;
		uint32 epon_tx_clk125_sw_reset_n:1; /* for B0 */
		uint32 epon_tx_tclk16_sw_reset_n:1; /* for B0 */
		uint32 epon_rx_rbc125_sw_reset_n:1; /* for B0 */
		uint32 epon_rx_rclk16_sw_reset_n:1; /* for B0 */
		uint32 gpon_nco_rst_n:1;
		uint32 reserved3:2;
		uint32 ngpon_8khz_rst_n:1;
		uint32 ngpon_tx_rst_n:1;
		uint32 ngpon_rx_rst_n:1;
		uint32 ngpon_main_rst_n:1;
		uint32 gpon_8khz_rst_n:1;
		uint32 gpon_tx_rst_n:1;
		uint32 gpon_rx_rst_n:1;
		uint32 gpon_main_rst_n:1;
		uint32 ae_tx_clk125_sw_reset_n:1;
		uint32 ae_tx_tclk16_sw_reset_n:1;  /* for B0 */
		uint32 ae_rx_rbc125_sw_reset_n:1;  /* for B0 */
		uint32 ae_rx_rclk16_sw_reset_n:1;
		uint32 epon_core_rst_n:1;
		uint32 epon_tx_rst_n:1;
		uint32 epon_rx_rst_n:1;
		uint32 epon_main_rst_n:1;
		uint32 reserved2:1;
		uint32 i_zn_sw_init:1;
		uint32 core_soft_rst_n:1;
		uint32 reserved1:2;
		uint32 rbus_rst_n:1;
		uint32 wan_top_bb_rst_n:1;
		uint32 wan_main_rst_n:1;
#endif
	}Bits_Wantop;
#endif
	uint32 Reg32;
} BPCM_SR_CONTROL;

#if !defined(_BCM96838_) && !defined(CONFIG_BCM96838)
#if (defined(_BCM963138_) || defined(CONFIG_BCM963138)) || defined(_BCM94908_) || defined(CONFIG_BCM94908) || defined(_BCM963158_) || defined(CONFIG_BCM963158)
typedef union
{
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32 z2_p_wan_phy_sel : 3;	/* 0-2 */
		uint32 reserved0	: 1;	/* 3 */
		uint32 z2_switch_p3_phy_sel: 3;	/* 4-6 */
		uint32 reserved1	: 1;	/* 7 */
		uint32 z2_switch_p4_phy_sel: 3;	/* 8-10 */
		uint32 reserved2	: 1;	/* 11 */
		uint32 z0_mux_sel	: 1;	/* 12 */
		uint32 z1_gphy_mux_sel	: 1;	/* 13 */
		uint32 z2_gphy_mux_sel	: 1;	/* 14 */
		uint32 z2_crossbar_mux_sel: 1;	/* 15 */
		uint32 reserved3	: 1;	/* 16 */
		uint32 z1_pda_en	: 1;	/* 17 */
		uint32 z1_ck250_clk_en	: 1;	/* 18 */
		uint32 z1_ck25_clk_dis	: 1;	/* 19 */
		uint32 reserved4	: 2;	/* 20-21 */
		uint32 z2_ck250_clk_en	: 1;	/* 22 */
		uint32 z2_ck25_clk_dis	: 1;	/* 23 */
		uint32 z2_serdes_clk_en	: 1;	/* 24 */
		uint32 z2_serdes_reset_mdioregs	: 1;	/* 25 */
		uint32 z2_sedes_reset_pll: 1;	/* 26 */
		uint32 z2_serdes_reset	: 1;	/* 27 */
		uint32 z2_serdes_mux_sel: 1;	/* 28 */
		uint32 reserved5	: 1;	/* 29 */
		uint32 z1_gphy_reset	: 1;	/* 30 */
		uint32 z2_gphy_reset	: 1;	/* 31 */
#else
		uint32 z2_gphy_reset	: 1;	/* 31 */
		uint32 z1_gphy_reset	: 1;	/* 30 */
		uint32 reserved5	: 1;	/* 29 */
		uint32 z2_serdes_mux_sel: 1;	/* 28 */
		uint32 z2_serdes_reset	: 1;	/* 27 */
		uint32 z2_sedes_reset_pll: 1;	/* 26 */
		uint32 z2_serdes_reset_mdioregs	: 1;	/* 25 */
		uint32 z2_serdes_clk_en	: 1;	/* 24 */
		uint32 z2_ck25_clk_dis	: 1;	/* 23 */
		uint32 z2_ck250_clk_en	: 1;	/* 22 */
		uint32 reserved4	: 2;	/* 20-21 */
		uint32 z1_ck25_clk_dis	: 1;	/* 19 */
		uint32 z1_ck250_clk_en	: 1;	/* 18 */
		uint32 z1_pda_en	: 1;	/* 17 */
		uint32 reserved3	: 1;	/* 16 */
		uint32 z2_crossbar_mux_sel: 1;	/* 15 */
		uint32 z2_gphy_mux_sel	: 1;	/* 14 */
		uint32 z1_gphy_mux_sel	: 1;	/* 13 */
		uint32 z0_mux_sel	: 1;	/* 12 */
		uint32 reserved2	: 1;	/* 11 */
		uint32 z2_switch_p4_phy_sel: 3;	/* 8-10 */
		uint32 reserved1	: 1;	/* 7 */
		uint32 z2_switch_p3_phy_sel: 3;	/* 4-6 */
		uint32 reserved0	: 1;	/* 3 */
		uint32 z2_p_wan_phy_sel : 3;	/* 0-2 */
#endif
	} Bits;
	uint32 Reg32;
} BPCM_GLOBAL_CNTL;
#elif defined(_BCM96848_) || defined(CONFIG_BCM96848) || defined(_BCM96858_) || defined(CONFIG_BCM96858) || defined(_BCM96836_) || defined(CONFIG_BCM96836) || defined(CONFIG_BCM96846) || defined(_BCM96846_) || defined(_BCM96856_) || defined(CONFIG_BCM96856)

typedef union
{
    struct {
        uint32 tbd      : 32;
    } Bits;
    uint32 Reg32;
} BPCM_GLOBAL_CNTL;
#else
typedef union
{
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32 z0_mux_sel	: 1;	/* 0 */
		uint32 reserved0	: 4;	/* 1-4 */
		uint32 z1_gphy_reset	: 1;	/* 5 */
		uint32 reserved1	: 3;	/* 6-8 */
		uint32 z1_ck25_clk_dis	: 1;	/* 9 */
		uint32 z1_ck250_clk_en	: 1;	/* 10 */
		uint32 reserved2	: 1;	/* 11 */
		uint32 z1_gphy_mux_sel	: 1;	/* 12 */
		uint32 z1_pda_en	: 1;	/* 13 */
		uint32 reserved3	: 3;	/* 14-16 */
		uint32 z2_gphy_reset	: 1;	/* 17 */
		uint32 z2_serdes_reset	: 1;	/* 18 */
		uint32 z2_serdes_reset_mdioregs	: 1;	/* 19 */
		uint32 z2_sedes_reset_pll	: 1;	/* 20 */
		uint32 z2_ck250_clk_en	: 1;	/* 21 */
		uint32 z2_ck25_clk_dis	: 1;	/* 22 */
		uint32 z2_serdes_clk_en	: 1;	/* 23 */
		uint32 reserved4	: 1;	/* 24 */
		uint32 z2_serdes_mux_sel	: 1;	/* 25 */
		uint32 z2_gphy_mux_sel	: 1;	/* 26 */
		uint32 z2_crossbar_mux_sel	: 1;	/* 27 */
		uint32 z2_p_wan_phy_sel	: 2;	/* 28-29 */
		uint32 z2_switch_phy_sel	: 2;	/* 30-31 */
#else
		uint32 z2_switch_phy_sel	: 2;	/* 30-31 */
		uint32 z2_p_wan_phy_sel	: 2;	/* 28-29 */
		uint32 z2_crossbar_mux_sel	: 1;	/* 27 */
		uint32 z2_gphy_mux_sel	: 1;	/* 26 */
		uint32 z2_serdes_mux_sel	: 1;	/* 25 */
		uint32 reserved4	: 1;	/* 24 */
		uint32 z2_serdes_clk_en	: 1;	/* 23 */
		uint32 z2_ck25_clk_dis	: 1;	/* 22 */
		uint32 z2_ck250_clk_en	: 1;	/* 21 */
		uint32 z2_sedes_reset_pll	: 1;	/* 20 */
		uint32 z2_serdes_reset_mdioregs	: 1;	/* 19 */
		uint32 z2_serdes_reset	: 1;	/* 18 */
		uint32 z2_gphy_reset	: 1;	/* 17 */
		uint32 reserved3	: 3;	/* 14-16 */
		uint32 z1_pda_en	: 1;	/* 13 */
		uint32 z1_gphy_mux_sel	: 1;	/* 12 */
		uint32 reserved2	: 1;	/* 11 */
		uint32 z1_ck250_clk_en	: 1;	/* 10 */
		uint32 z1_ck25_clk_dis	: 1;	/* 9 */
		uint32 reserved1	: 3;	/* 6-8 */
		uint32 z1_gphy_reset	: 1;	/* 5 */
		uint32 reserved0	: 4;	/* 1-4 */
		uint32 z0_mux_sel	: 1;	/* 0 */
#endif
	} Bits;
	uint32 Reg32;
} BPCM_GLOBAL_CNTL;
#endif

#if defined(_BCM963158_) || defined(CONFIG_BCM963158)
typedef union
{
    struct {

#ifdef PMC_LITTLE_ENDIAN
		uint32 z0_mux_sel       :1; //        = r_Z0_GLOBAL_CNTL[0];
		uint32 reserverd_1      :1; //        unused
		uint32 z3_pda_en        :1; //        = r_Z0_GLOBAL_CNTL[2];
		uint32 rx_sys_clk_en    :1; //        = r_Z0_GLOBAL_CNTL[3];
		uint32 tx_sys_clk_en    :1; //         = r_Z0_GLOBAL_CNTL[4];
		uint32 gmii_rx_clk_en   :1; //        = r_Z0_GLOBAL_CNTL[5];
		uint32 gmii_tx_clk_en   :1; //        = r_Z0_GLOBAL_CNTL[6];
		uint32 rsrvd            :25;
#else
		uint32 rsrvd            :25;
		uint32 gmii_tx_clk_en   :1; //        = r_Z0_GLOBAL_CNTL[6];
		uint32 gmii_rx_clk_en   :1; //        = r_Z0_GLOBAL_CNTL[5];
		uint32 tx_sys_clk_en    :1; //        = r_Z0_GLOBAL_CNTL[4];
		uint32 rx_sys_clk_en    :1; //        = r_Z0_GLOBAL_CNTL[3];
		uint32 z3_pda_en        :1; //        = r_Z0_GLOBAL_CNTL[2];
		uint32 reserverd_1      :1; //        unused
		uint32 z0_mux_sel       :1; //        = r_Z0_GLOBAL_CNTL[0];
#endif

    } Bits;
    uint32 Reg32;

} BPCM_GLOBAL_CNTL_0;

typedef union
{
    struct {

#ifdef PMC_LITTLE_ENDIAN
		uint32 z1_pda_en                      :1;//= r_Z1_GLOBAL_CNTL[0];
		uint32 reserved                       :2;
		uint32 z1_ck250_clk_en                :1;//= r_Z1_GLOBAL_CNTL[3];
		uint32 z1_ref_clk_dis                 :1;//= r_Z1_GLOBAL_CNTL[4];
		uint32 z1_mux_sel                     :1;//= r_Z1_GLOBAL_CNTL[5];
		uint32 z1_gphy_reset                  :1;//= r_Z1_GLOBAL_CNTL[6];
		uint32 z1_gphy_iddq_global_pwr        :1;//= r_Z1_GLOBAL_CNTL[7];
		uint32 z1_gphy_force_dll_en           :1;//= r_Z1_GLOBAL_CNTL[8];
		uint32 z1_gphy_ext_pwr_down           :4;//= r_Z1_GLOBAL_CNTL[12:9];
		uint32 z1_gphy_iddq_bias              :1;//= r_Z1_GLOBAL_CNTL[13];
		uint32 z1_switch_p3_phy_sel           :1;//= r_Z1_GLOBAL_CNTL[14];
		uint32 z1_switch_p8_sel               :1;//= r_Z1_GLOBAL_CNTL[15];
		uint32 rsrvd                          :16;
#else
		uint32 rsrvd                          :16;
		uint32 z1_switch_p8_sel               :1;//= r_Z1_GLOBAL_CNTL[15];
		uint32 z1_switch_p3_phy_sel           :1;//= r_Z1_GLOBAL_CNTL[14];
		uint32 z1_gphy_iddq_bias              :1;//= r_Z1_GLOBAL_CNTL[13];
		uint32 z1_gphy_ext_pwr_down           :4;//= r_Z1_GLOBAL_CNTL[12:9];
		uint32 z1_gphy_force_dll_en           :1;//= r_Z1_GLOBAL_CNTL[8];
		uint32 z1_gphy_iddq_global_pwr        :1;//= r_Z1_GLOBAL_CNTL[7];
		uint32 z1_gphy_reset                  :1;//= r_Z1_GLOBAL_CNTL[6];
		uint32 z1_mux_sel                     :1;//= r_Z1_GLOBAL_CNTL[5];
		uint32 z1_ref_clk_dis                 :1;//= r_Z1_GLOBAL_CNTL[4];
		uint32 z1_ck250_clk_en                :1;//= r_Z1_GLOBAL_CNTL[3];
		uint32 reserved                       :2;
		uint32 z1_pda_en                      :1;//= r_Z1_GLOBAL_CNTL[0];
#endif

    } Bits;
    uint32 Reg32;

} BPCM_GLOBAL_CNTL_1;


typedef union
{
    struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32 z2_pda_en:1                ; //= r_Z2_GLOBAL_CNTL[0];
		uint32 reserved:1;
		uint32 z2_ck250_clk_en:1          ; //= r_Z2_GLOBAL_CNTL[2];
		uint32 z2_ref_clk_dis:1           ; //= r_Z2_GLOBAL_CNTL[3];
		uint32 z2_serdes_clk_en:1         ; //= r_Z2_GLOBAL_CNTL[4];
		uint32 z2_gphy_mux_sel:1          ; //= r_Z2_GLOBAL_CNTL[5];
		uint32 z2_gphy_reset:1            ; //= r_Z2_GLOBAL_CNTL[6];
		uint32 z2_gphy_iddq_global_pwr:1  ; //= r_Z2_GLOBAL_CNTL[7];
		uint32 z2_gphy_force_dll_en:1     ; //= r_Z2_GLOBAL_CNTL[8];
		uint32 z2_gphy_ext_pwr_down:1     ; //= r_Z2_GLOBAL_CNTL[9];
		uint32 z2_gphy_iddq_bias:1        ; //= r_Z2_GLOBAL_CNTL[10];
		uint32 z2_crossbar_mux_sel:1      ; //= r_Z2_GLOBAL_CNTL[11];
		uint32 z2_p_wan_phy_sel:2         ; //= r_Z2_GLOBAL_CNTL[13:12];
		uint32 z2_switch_p4_phy_sel:2     ; //= r_Z2_GLOBAL_CNTL[15:14];
		uint32 z2_switch_p6_phy_sel:2     ; //= r_Z2_GLOBAL_CNTL[17:16];
		uint32 z2_serdes_mux_sel:1        ; //= r_Z2_GLOBAL_CNTL[18];
		uint32 z2_serdes_iddq:1           ; //= r_Z2_GLOBAL_CNTL[19];
		uint32 z2_serdes_pwrdwn:1         ; //= r_Z2_GLOBAL_CNTL[20];
		uint32 z2_serdes_reset:1          ; //= r_Z2_GLOBAL_CNTL[21];
		uint32 z2_serdes_reset_mdioregs:1 ; //= r_Z2_GLOBAL_CNTL[22];
		uint32 z2_serdes_reset_pll:1      ; //= r_Z2_GLOBAL_CNTL[23];
		uint32 z2_serdes_refclk_sel:3     ; //= r_Z2_GLOBAL_CNTL[26:24];
		uint32 z2_pll_clk125_250_sel:1    ; //= r_Z2_GLOBAL_CNTL[27];
		uint32 z2_pll_mux_clk250_sel:1    ; //= r_Z2_GLOBAL_CNTL[28];
		uint32 rsrvd:3;
#else
		uint32 rsrvd:3;
		uint32 z2_pll_mux_clk250_sel:1    ; //= r_Z2_GLOBAL_CNTL[28];
		uint32 z2_pll_clk125_250_sel:1    ; //= r_Z2_GLOBAL_CNTL[27];
		uint32 z2_serdes_refclk_sel:3     ; //= r_Z2_GLOBAL_CNTL[26:24];
		uint32 z2_serdes_reset_pll:1      ; //= r_Z2_GLOBAL_CNTL[23];
		uint32 z2_serdes_reset_mdioregs:1 ; //= r_Z2_GLOBAL_CNTL[22];
		uint32 z2_serdes_reset:1          ; //= r_Z2_GLOBAL_CNTL[21];
		uint32 z2_serdes_pwrdwn:1         ; //= r_Z2_GLOBAL_CNTL[20];
		uint32 z2_serdes_iddq:1           ; //= r_Z2_GLOBAL_CNTL[19];
		uint32 z2_serdes_mux_sel:1        ; //= r_Z2_GLOBAL_CNTL[18];
		uint32 z2_switch_p6_phy_sel:2     ; //= r_Z2_GLOBAL_CNTL[17:16];
		uint32 z2_switch_p4_phy_sel:2     ; //= r_Z2_GLOBAL_CNTL[15:14];
		uint32 z2_p_wan_phy_sel:2         ; //= r_Z2_GLOBAL_CNTL[13:12];
		uint32 z2_crossbar_mux_sel:1      ; //= r_Z2_GLOBAL_CNTL[11];
		uint32 z2_gphy_iddq_bias:1        ; //= r_Z2_GLOBAL_CNTL[10];
		uint32 z2_gphy_ext_pwr_down:1     ; //= r_Z2_GLOBAL_CNTL[9];
		uint32 z2_gphy_force_dll_en:1     ; //= r_Z2_GLOBAL_CNTL[8];
		uint32 z2_gphy_iddq_global_pwr:1  ; //= r_Z2_GLOBAL_CNTL[7];
		uint32 z2_gphy_reset:1            ; //= r_Z2_GLOBAL_CNTL[6];
		uint32 z2_gphy_mux_sel:1          ; //= r_Z2_GLOBAL_CNTL[5];
		uint32 z2_serdes_clk_en:1         ; //= r_Z2_GLOBAL_CNTL[4];
		uint32 z2_ref_clk_dis:1           ; //= r_Z2_GLOBAL_CNTL[3];
		uint32 z2_ck250_clk_en:1          ; //= r_Z2_GLOBAL_CNTL[2];
		uint32 reserved:1;
		uint32 z2_pda_en:1                ; //= r_Z2_GLOBAL_CNTL[0];
#endif
	
    } Bits;
    uint32 Reg32;

} BPCM_GLOBAL_CNTL_2;
#endif


typedef union
{
	struct {
		uint32 ctl;
	} Bits_sata_gp;
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32 iddq_bias	: 1;	/* 0 */
		uint32 ext_pwr_down	: 4;	/* 1-4 */
		uint32 force_dll_en	: 1;	/* 5 */
		uint32 iddq_global_pwr	: 1;	/* 6 */
		uint32 reserved		: 25;
#else
		uint32 reserved		: 25;
		uint32 iddq_global_pwr	: 1;	/* 6 */
		uint32 force_dll_en	: 1;	/* 5 */
		uint32 ext_pwr_down	: 4;	/* 1-4 */
		uint32 iddq_bias	: 1;	/* 0 */
#endif
	} Bits_switch_z1_qgphy;
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32 iddq_bias	: 1;	/* 0 */
		uint32 ext_pwr_down	: 1;	/* 1 */
		uint32 force_dll_en	: 1;	/* 2 */
		uint32 iddq_global_pwd	: 1;	/* 3 */
		uint32 ck25_dis		: 1;	/* 4 */
		uint32 phy_reset	: 1;	/* 5 */
		uint32 reserved0	: 2;
		uint32 phy_ad		: 5;	/* 8-12 */
		uint32 reserved1	: 18;
		uint32 ctrl_en		: 1;	/* 31 */
#else
		uint32 ctrl_en		: 1;	/* 31 */
		uint32 reserved1	: 18;
		uint32 phy_ad		: 5;	/* 8-12 */
		uint32 reserved0	: 2;
		uint32 phy_reset	: 1;	/* 5 */
		uint32 ck25_dis		: 1;	/* 4 */
		uint32 iddq_global_pwd	: 1;	/* 3 */
		uint32 force_dll_en	: 1;	/* 2 */
		uint32 ext_pwr_down	: 1;	/* 1 */
		uint32 iddq_bias	: 1;	/* 0 */
#endif
	} Bits_egphy_1port;
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32 iddq_bias	: 1;	/* 0 */
		uint32 ext_pwr_down	: 4;	/* 1-4 */
		uint32 force_dll_en	: 1;	/* 5 */
		uint32 iddq_global_pwd	: 1;	/* 6 */
		uint32 ck25_dis		: 1;	/* 7 */
		uint32 phy_reset	: 1;	/* 8 */
		uint32 reserved0	: 3;
		uint32 phy_ad		: 5;	/* 12-16 */
		uint32 reserved1	: 14;
		uint32 ctrl_en		: 1;	/* 31 */
#else
		uint32 ctrl_en		: 1;	/* 31 */
		uint32 reserved1	: 14;
		uint32 phy_ad		: 5;	/* 12-16 */
		uint32 reserved0	: 3;
		uint32 phy_reset	: 1;	/* 8 */
		uint32 ck25_dis		: 1;	/* 7 */
		uint32 iddq_global_pwd	: 1;	/* 6 */
		uint32 force_dll_en	: 1;	/* 5 */
		uint32 ext_pwr_down	: 4;	/* 1-4 */
		uint32 iddq_bias	: 1;	/* 0 */
#endif
	} Bits_egphy_4port;
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32 iddq_bias	: 1;	/* 0 */
		uint32 ext_pwr_down	: 4;	/* 1-4 */
		uint32 force_dll_en	: 1;	/* 5 */
		uint32 iddq_global_pwr	: 1;	/* 6 */
		uint32 reserved0	: 25;	/* 7-31 */
#else
		uint32 reserved0	: 25;	/* 7-31 */
		uint32 iddq_global_pwr	: 1;	/* 6 */
		uint32 force_dll_en	: 1;	/* 5 */
		uint32 ext_pwr_down	: 4;	/* 1-4 */
		uint32 iddq_bias	: 1;	/* 0 */
#endif
	} Bits_qgphy_cntl;
	struct {
		uint32 ctl;
	} Bits_vdsl_phy;
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32 alt_bfc_vector	: 12;	/* 00-11 */
		uint32 reserved0	: 3;
		uint32 alt_bfc_en	: 1;	/* 15 */
		uint32 reset_dly_cfg	: 2;	/* 16-17 */
		uint32 reserved1	: 8;
		uint32 ext_mclk_en_reset: 1;	/* 26 */
		uint32 ext_mclk_en	: 1;	/* 27 */
		uint32 por_reset_n_ctl	: 1;	/* 28 */
		uint32 reset_n_ctl	: 1;	/* 29 */
		uint32 reserved2	: 1;
		uint32 clken		: 1;	/* 31 */
#else
		uint32 clken		: 1;	/* 31 */
		uint32 reserved2	: 1;
		uint32 reset_n_ctl	: 1;	/* 29 */
		uint32 por_reset_n_ctl	: 1;	/* 28 */
		uint32 ext_mclk_en	: 1;	/* 27 */
		uint32 ext_mclk_en_reset: 1;	/* 26 */
		uint32 reserved1	: 8;
		uint32 reset_dly_cfg	: 2;	/* 16-17 */
		uint32 alt_bfc_en	: 1;	/* 15 */
		uint32 reserved0	: 3;
		uint32 alt_bfc_vector	: 12;	/* 00-11 */
#endif
	} Bits_vdsl_mips;
	uint32 Reg32;
} BPCM_MISC_CONTROL;

typedef union
{
	struct {
		uint32 field;
	} Bits_qgphy_status;
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32 alt_bfc_vector	: 12;	/* 00-11 */
		uint32 reserved0	: 3;
		uint32 alt_bfc_en	: 1;	/* 15 */
		uint32 reset_dly_cfg	: 2;	/* 16-17 */
		uint32 reserved1	: 8;
		uint32 ext_mclk_en_reset: 1;	/* 26 */
		uint32 ext_mclk_en	: 1;	/* 27 */
		uint32 por_reset_n_ctl	: 1;	/* 28 */
		uint32 reset_n_ctl	: 1;	/* 29 */
		uint32 reserved2	: 1;
		uint32 clken		: 1;	/* 31 */
#else
		uint32 clken		: 1;	/* 31 */
		uint32 reserved2	: 1;
		uint32 reset_n_ctl	: 1;	/* 29 */
		uint32 por_reset_n_ctl	: 1;	/* 28 */
		uint32 ext_mclk_en	: 1;	/* 27 */
		uint32 ext_mclk_en_reset: 1;	/* 26 */
		uint32 reserved1	: 8;
		uint32 reset_dly_cfg	: 2;	/* 16-17 */
		uint32 alt_bfc_en	: 1;	/* 15 */
		uint32 reserved0	: 3;
		uint32 alt_bfc_vector	: 12;	/* 00-11 */
#endif
	} Bits_vdsl_mips;  /* second PHY MIPS core */
	uint32 Reg32;
} BPCM_MISC_CONTROL2;

typedef union
{
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32 gphy_iddq_bias		: 1;	/* 00 */
		uint32 gphy_ext_pwr_down	: 1;	/* 01 */
		uint32 gphy_force_dll_en	: 1;	/* 02 */
		uint32 gphy_iddq_global_pwr	: 1;	/* 03 */
		uint32 serdes_iddq		: 1;	/* 04 */
		uint32 serdes_pwrdwn		: 1;	/* 05 */
		uint32 reserved0		: 2;	/* 07:06 */
		uint32 serdes_refclk_sel	: 3;	/* 10:08 */
		uint32 reserved1		: 5;	/* 15:11 */
		uint32 pll_clk125_250_sel	: 1;	/* 16 */
		uint32 pll_mux_clk_250_sel	: 1;	/* 17 */
		uint32 reserved2		: 14;	/* 31:18 */
#else
		uint32 reserved2		: 14;	/* 31:18 */
		uint32 pll_mux_clk_250_sel	: 1;	/* 17 */
		uint32 pll_clk125_250_sel	: 1;	/* 16 */
		uint32 reserved1		: 5;	/* 15:11 */
		uint32 serdes_refclk_sel	: 3;	/* 10:08 */
		uint32 reserved0		: 2;	/* 07:06 */
		uint32 serdes_pwrdwn		: 1;	/* 05 */
		uint32 serdes_iddq		: 1;	/* 04 */
		uint32 gphy_iddq_global_pwr	: 1;	/* 03 */
		uint32 gphy_force_dll_en	: 1;	/* 02 */
		uint32 gphy_ext_pwr_down	: 1;	/* 01 */
		uint32 gphy_iddq_bias		: 1;	/* 00 */
#endif
	} Bits;
	uint32 Reg32;
} BPCM_SGPHY_CNTL;

typedef union
{
	struct {
		uint32 field;
	} Bits;
	uint32 Reg32;
} BPCM_SGPHY_STATUS;

typedef union
{
	struct {
#if defined(_BCM963138_) || defined(CONFIG_BCM963138)
#ifdef PMC_LITTLE_ENDIAN
		uint32 core_pwr_ctrl	:  2;	// 01:00    R/W
		uint32 reserved2	:  6;	// 07:02    R/W
		uint32 pll_pwr_on	:  1;	// 08:08    R/W
		uint32 pll_ldo_pwr_on	:  1;	// 09:09    R/W
		uint32 pll_clamp_on	:  1;	// 10:10    R/W
		uint32 reserved1	:  2;	// 12:11    R/O
		uint32 cpu0_reset_n	:  1;	// 13:13    R/W
		uint32 cpu1_reset_n	:  1;	// 14:14    R/W
		uint32 neon_reset_n	:  1;	// 15:15    R/W
		uint32 reserved0	: 12;	// 27:16    R/O
		uint32 pwr_ctrl_sts	:  2;	// 29:28    R/O
		uint32 power_down	:  2;	// 31:30    R/O
#else
		uint32 power_down	:  2;	// 31:30    R/O
		uint32 pwr_ctrl_sts	:  2;	// 29:28    R/O
		uint32 reserved0	: 12;	// 27:16    R/O
		uint32 neon_reset_n	:  1;	// 15:15    R/W
		uint32 cpu1_reset_n	:  1;	// 14:14    R/W
		uint32 cpu0_reset_n	:  1;	// 13:13    R/W
		uint32 reserved1	:  2;	// 12:11    R/O
		uint32 pll_clamp_on	:  1;	// 10:10    R/W
		uint32 pll_ldo_pwr_on	:  1;	// 09:09    R/W
		uint32 pll_pwr_on	:  1;	// 08:08    R/W
		uint32 reserved2	:  6;	// 07:02    R/W
		uint32 core_pwr_ctrl	:  2;	// 01:00    R/W
#endif
#elif defined(_BCM963148_) || defined(CONFIG_BCM963148)
#ifdef PMC_LITTLE_ENDIAN
		uint32	cpu0_reset_n	:  1;	// 00:00    R/W
		uint32	power_down	:  1;	// 01:01    R/W
		uint32	reserved	: 30;	// 31:02    R/O
#else
		uint32	reserved	: 30;	// 31:02    R/O
		uint32	power_down	:  1;	// 01:01    R/W
		uint32	cpu0_reset_n	:  1;	// 00:00    R/W
#endif
#elif defined(_BCM96858_) || defined(CONFIG_BCM96858) || defined(_BCM96836_) || defined(CONFIG_BCM96836) || defined(_BCM963158_) || defined(CONFIG_BCM93158) || defined(_BCM96846_) || defined(CONFIG_BCM96846) || defined(_BCM96856_) || defined(CONFIG_BCM96856) 
#ifdef PMC_LITTLE_ENDIAN
		uint32  cpu_reset           :  8;   // 07:00    R/W
		uint32  c0l2_reset          :  1;   // 08:08    R/W
		uint32  c1l2_reset          :  1;   // 09:09    R/W
		uint32  reserved0           :  6;   // 15:10    R/O
		uint32  cpu_bpcm_init_on    :  8;   // 23:16    R/W
		uint32  c0l2_bpcm_init_on   :  1;   // 24:24    R/W
		uint32  c1l2_bpcm_init_on   :  1;   // 25:25    R/W
		uint32  ubus_sr             :  1;   // 26:26    R/W
		uint32  cci_sr              :  1;   // 27:27    R/W
		uint32  webcores_sr         :  1;   // 28:28    R/W
		uint32  hw_done             :  1;   // 29:29    R/O
		uint32  sw_done             :  1;   // 30:30    R/W
		uint32  start               :  1;   // 31:31    R/W
#else
		uint32  start				:  1;   // 31:31    R/W
		uint32  sw_done				:  1;   // 30:30    R/W
		uint32  hw_done				:  1;   // 29:29    R/O
		uint32  webcores_sr			:  1;   // 28:28    R/W
		uint32  cci_sr				:  1;   // 27:27    R/W
		uint32  ubus_sr				:  1;   // 26:26    R/W
		uint32  c1l2_bpcm_init_on	:  1;   // 25:25    R/W
		uint32  c0l2_bpcm_init_on	:  1;   // 24:24    R/W
		uint32  cpu_bpcm_init_on	:  8;   // 23:16    R/W
		uint32  reserved0			:  6;   // 15:10    R/O
		uint32  c1l2_reset			:  1;   // 09:09    R/W
		uint32  c0l2_reset			:  1;   // 08:08    R/W
		uint32  cpu_reset			:  8;   // 07:00    R/W
#endif
#endif
	} Bits;
	uint32 Reg32;
} ARM_CONTROL_REG;

typedef union
{
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32 mem_pwr_ok	:  1;	// 00:00    R/W
		uint32 mem_pwr_on	:  1;	// 01:01    R/W
		uint32 mem_clamp_on	:  1;	// 02:02    R/W
		uint32 reserved2	:  1;	// 03:03    R/W
		uint32 mem_pwr_ok_status	:  1;	// 04:04    R/O
		uint32 mem_pwr_on_status	:  1;	// 05:05    R/O
		uint32 reserved1	:  2;	// 07:06    R/W
		uint32 mem_pda		:  4;	// 11:08    R/W only LS bit for CPU0/1, all four bits for neon_l2
		uint32 reserved0	:  3;	// 14:12    R/W
		uint32 clamp_on		:  1;	// 15:15    R/W
		uint32 pwr_ok		:  4;	// 19:16    R/W ditto
		uint32 pwr_on		:  4;	// 23:20    R/W ditto
		uint32 pwr_ok_status	:  4;	// 27:24    R/O ditto
		uint32 pwr_on_status	:  4;	// 31:28    R/O only LS 2-bits for CPU1, only LS 1 bit for neon_l2
#else
		uint32 pwr_on_status	:  4;	// 31:28    R/O only LS 2-bits for CPU1, only LS 1 bit for neon_l2
		uint32 pwr_ok_status	:  4;	// 27:24    R/O ditto
		uint32 pwr_on		:  4;	// 23:20    R/W ditto
		uint32 pwr_ok		:  4;	// 19:16    R/W ditto
		uint32 clamp_on		:  1;	// 15:15    R/W
		uint32 reserved0	:  3;	// 14:12    R/W
		uint32 mem_pda		:  4;	// 11:08    R/W only LS bit for CPU0/1, all four bits for neon_l2
		uint32 reserved1	:  2;	// 07:06    R/W
		uint32 mem_pwr_on_status	:  1;	// 05:05    R/O
		uint32 mem_pwr_ok_status	:  1;	// 04:04    R/O
		uint32 reserved2	:  1;	// 03:03    R/W
		uint32 mem_clamp_on	:  1;	// 02:02    R/W
		uint32 mem_pwr_on	:  1;	// 01:01    R/W
		uint32 mem_pwr_ok	:  1;	// 00:00    R/W
#endif
	} Bits;
	uint32 Reg32;
} ARM_CPUx_PWR_CTRL_REG;

typedef union
{
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32 resetb		: 1;	// 00:00
		uint32 post_resetb	: 1;	// 01:01
		uint32 pwrdwn		: 1;	// 02:02
		uint32 master_reset	: 1;	// 03:03
		uint32 pwrdwn_ldo	: 1;	// 04:04
		uint32 iso		: 1;	// 05:05 // only used in afepll
		uint32 reserved0	: 2;	// 07:06
		uint32 ldo_ctrl		: 6;	// 13:08
		uint32 reserved1	: 1;	// 14:14
		uint32 hold_ch_all	: 1;	// 15:15
		uint32 reserved2	: 4;	// 16:19
		uint32 byp_wait		: 1;	// 20:20 // only used in b15pll
		uint32 reserved3	: 11;	// 21:31
#else
		uint32 reserved3	: 11;	// 21:31
		uint32 byp_wait		: 1;	// 20:20 // only used in b15pll
		uint32 reserved2	: 4;	// 16:19
		uint32 hold_ch_all	: 1;	// 15:15
		uint32 reserved1	: 1;	// 14:14
		uint32 ldo_ctrl		: 6;	// 13:08
		uint32 reserved0	: 2;	// 07:06
		uint32 iso		: 1;	// 05:05 // only used in afepll
		uint32 pwrdwn_ldo	: 1;	// 04:04
		uint32 master_reset	: 1;	// 03:03
		uint32 pwrdwn		: 1;	// 02:02
		uint32 post_resetb	: 1;	// 01:01
		uint32 resetb		: 1;	// 00:00
#endif
	} Bits;
	uint32 Reg32;
} PLL_CTRL_REG;

typedef union
{
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32 fb_offset	: 12;	// 11:00
		uint32 fb_phase_en	: 1;	// 12:12
		uint32 _8phase_en	: 1;	// 13:13
		uint32 sr		: 18;	// 31:14
#else
		uint32 sr		: 18;	// 31:14
		uint32 _8phase_en	: 1;	// 13:13
		uint32 fb_phase_en	: 1;	// 12:12
		uint32 fb_offset	: 12;	// 11:00
#endif
	} Bits;
	uint32 Reg32;
} PLL_PHASE_REG;

typedef union
{
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32 ndiv_int		: 10;	// 09:00
		uint32 ndiv_frac	: 20;	// 29:10
		uint32 reserved0	: 2;
#else
		uint32 reserved0	: 2;
		uint32 ndiv_frac	: 20;	// 29:10
		uint32 ndiv_int		: 10;	// 09:00
#endif
	} Bits;
	uint32 Reg32;
} PLL_NDIV_REG;

typedef union
{
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32 pdiv		: 3;	// 02:00
		uint32 reserved0	: 28;	// 30:03
		uint32 ndiv_pdiv_override : 1;	// 31:31
#else
		uint32 ndiv_pdiv_override : 1;	// 31:31
		uint32 reserved0	: 28;	// 30:03
		uint32 pdiv		: 3;	// 02:00
#endif
	} Bits;
	uint32 Reg32;
} PLL_PDIV_REG;

typedef union
{
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32 reserved0	: 4;	// 03:00
		uint32 ka		: 3;	// 06:04
		uint32 reserved1	: 1;	// 07:07
		uint32 ki		: 3;	// 10:08
		uint32 reserved2	: 1;	// 11:11
		uint32 kp		: 4;	// 15:12
		uint32 ssc_step		: 16;	// 31:16
#else
		uint32 ssc_step		: 16;	// 31:16
		uint32 kp		: 4;	// 15:12
		uint32 reserved2	: 1;	// 11:11
		uint32 ki		: 3;	// 10:08
		uint32 reserved1	: 1;	// 07:07
		uint32 ka		: 3;	// 06:04
		uint32 reserved0	: 4;	// 03:00
#endif
	} Bits;
	uint32 Reg32;
} PLL_LOOP0_REG;

typedef union
{
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32 ssc_limit	: 22;	// 21:00
		uint32 reserved0	: 2;	// 23:22
		uint32 ssc_clkdiv	: 4;	// 27:24
		uint32 ssc_status	: 1;	// 28:28
		uint32 reserved1	: 2;	// 30:29
		uint32 ssc_mode		: 1;	// 31:31
#else
		uint32 ssc_mode		: 1;	// 31:31
		uint32 reserved1	: 2;	// 30:29
		uint32 ssc_status	: 1;	// 28:28
		uint32 ssc_clkdiv	: 4;	// 27:24
		uint32 reserved0	: 2;	// 23:22
		uint32 ssc_limit	: 22;	// 21:00
#endif
	} Bits;
	uint32 Reg32;
} PLL_LOOP1_REG;

typedef union
{
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32 mdiv0		: 8;	// 07:00
		uint32 enableb_ch0	: 1;	// 08:08
		uint32 hold_ch0		: 1;	// 09:09
		uint32 load_en_ch0	: 1;	// 10:10
		uint32 mdel0		: 1;	// 11:11
		uint32 reserved0	: 3;	// 14:12
		uint32 mdiv_override0	: 1;	// 15:15
		uint32 mdiv1		: 8;	// 23:16
		uint32 enableb_ch1	: 1;	// 24:24
		uint32 hold_ch1		: 1;	// 25:25
		uint32 load_en_ch1	: 1;	// 26:26
		uint32 mdel1		: 1;	// 27:27
		uint32 reserved1	: 3;	// 30:28
		uint32 mdiv_override1	: 1;	// 31:31
#else
		uint32 mdiv_override1	: 1;	// 31:31
		uint32 reserved1	: 3;	// 30:28
		uint32 mdel1		: 1;	// 27:27
		uint32 load_en_ch1	: 1;	// 26:26
		uint32 hold_ch1		: 1;	// 25:25
		uint32 enableb_ch1	: 1;	// 24:24
		uint32 mdiv1		: 8;	// 23:16
		uint32 mdiv_override0	: 1;	// 15:15
		uint32 reserved0	: 3;	// 14:12
		uint32 mdel0		: 1;	// 11:11
		uint32 load_en_ch0	: 1;	// 10:10
		uint32 hold_ch0		: 1;	// 09:09
		uint32 enableb_ch0	: 1;	// 08:08
		uint32 mdiv0		: 8;	// 07:00
#endif
	} Bits;
	uint32 Reg32;
} PLL_CHCFG_REG;

typedef union
{
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32 fdco_ctrl_bypass : 16;	// 15:00
		uint32 fdco_bypass_en	: 1;	// 16:16
		uint32 fdco_dac_sel	: 1;	// 17:17
		uint32 state_reset	: 1;	// 18:18
		uint32 state_mode 	: 2;	// 20:19
		uint32 state_sel	: 3;	// 23:21
		uint32 state_update 	: 1;	// 24:24
		uint32 dco_en 		: 1;	// 25:25
		uint32 dco_div2_div4	: 1;	// 26:26
		uint32 dco_bias_boost	: 1;	// 27:27
		uint32 bb_en		: 1;	// 28:28
		uint32 t2d_offset	: 3;    // 31:29
#else
		uint32 t2d_offset	: 3;    // 31:29
		uint32 bb_en		: 1;	// 28:28
		uint32 dco_bias_boost	: 1;	// 27:27
		uint32 dco_div2_div4	: 1;	// 26:26
		uint32 dco_en 		: 1;	// 25:25
		uint32 state_update 	: 1;	// 24:24
		uint32 state_sel	: 3;	// 23:21
		uint32 state_mode 	: 2;	// 20:19
		uint32 state_reset	: 1;	// 18:18
		uint32 fdco_dac_sel	: 1;	// 17:17
		uint32 fdco_bypass_en	: 1;	// 16:16
		uint32 fdco_ctrl_bypass : 16;	// 15:00
#endif
	} Bits;
	uint32 Reg32;
} PLL_CFG0_REG;

typedef union
{
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32 t2d_offset_msb 	: 1;	// 00:00
		uint32 t2d_clk_enable 	: 1;	// 01:01
		uint32 t2d_clk_sel 	: 1;	// 02:02
		uint32 kpp      	: 4;	// 06:03
		uint32 pwm_ctrl  	: 2;	// 08:07
		uint32 port_reset_mode	: 2;	// 10:09
		uint32 byp2_en 	 	: 1;	// 11:11
		uint32 byp1_en   	: 1;	// 12:12
		uint32 ref_diff_sel 	: 1;	// 13:13
		uint32 ki_startlow	: 1;	// 14:14
		uint32 en_500ohm	: 1;	// 15:15
		uint32 refd2c_bias	: 3;	// 18:16
		uint32 post_div2_div3	: 1;	// 19:19
		uint32 ki_boost		: 1;	// 20:20
		uint32 reserved0	: 11;   // 31:21
#else
		uint32 reserved0	: 11;   // 31:21
		uint32 ki_boost		: 1;	// 20:20
		uint32 post_div2_div3	: 1;	// 19:19
		uint32 refd2c_bias	: 3;	// 18:16
		uint32 en_500ohm	: 1;	// 15:15
		uint32 ki_startlow	: 1;	// 14:14
		uint32 ref_diff_sel 	: 1;	// 13:13
		uint32 byp1_en   	: 1;	// 12:12
		uint32 byp2_en 	 	: 1;	// 11:11
		uint32 port_reset_mode	: 2;	// 10:09
		uint32 pwm_ctrl  	: 2;	// 08:07
		uint32 kpp      	: 4;	// 06:03
		uint32 t2d_clk_sel 	: 1;	// 02:02
		uint32 t2d_clk_enable 	: 1;	// 01:01
		uint32 t2d_offset_msb 	: 1;	// 00:00
#endif
	} Bits;
	uint32 Reg32;
} PLL_CFG1_REG;

typedef union
{
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32 en_cml		: 3;	// 02:00
		uint32 tri_en		: 1;	// 03:03
		uint32 test_sel		: 3;	// 06:04
		uint32 test_en		: 1;	// 07:07
		uint32 reserved0	: 24;
#else
		uint32 reserved0	: 24;
		uint32 test_en		: 1;	// 07:07
		uint32 test_sel		: 3;	// 06:04
		uint32 tri_en		: 1;	// 03:03
		uint32 en_cml		: 3;	// 02:00
#endif
	} Bits;
	uint32 Reg32;
} PLL_OCTRL_REG;

typedef union
{
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32 out		: 12;	// 11:00
		uint32 reserved		: 19;	// 30:12
		uint32 lock		: 1;	// 31:31
#else
		uint32 lock		: 1;	// 31:31
		uint32 reserved		: 19;	// 30:12
		uint32 out		: 12;	// 11:00
#endif
	} Bits;
	uint32 Reg32;
} PLL_STAT_REG;

#if defined(_BCM96848_)      || defined(CONFIG_BCM96848) || defined(_BCM96858_) || defined(CONFIG_BCM96858) || defined(_BCM96836_) || \
    defined(CONFIG_BCM96836) || defined(_BCM963158_) || defined(CONFIG_BCM963158) || defined(_BCM96846_) || \
    defined(CONFIG_BCM96846) || defined(_BCM96856_) || defined(CONFIG_BCM96856) 
typedef union
{
    struct {
#ifdef PMC_LITTLE_ENDIAN
	uint32 ndiv_int		: 10;	// 09:00
	uint32 reserved0	: 2;	// 11:10
	uint32 ndiv_frac	: 20;	// 31:12    
#else
	uint32 ndiv_frac	: 20;	// 31:12
	uint32 reserved0	: 2;	// 11:10
	uint32 ndiv_int		: 10;	// 09:00
#endif
	} Bits;
	uint32 Reg32;
} PLL_DECNDIV_REG;

typedef union
{
    struct {
#ifdef PMC_LITTLE_ENDIAN
	uint32 pdiv			: 4;	// 03:00
	uint32 reserved0	: 12;	// 15:04
	uint32 mdiv0		: 8;	// 23:16
	uint32 mdiv1		: 8;	// 31:24
#else
	uint32 mdiv1		: 8;	// 31:24
	uint32 mdiv0		: 8;	// 23:16
	uint32 reserved0	: 12;	// 15:04
	uint32 pdiv			: 4;	// 03:00
#endif
	} Bits;
	uint32 Reg32;
} PLL_DECPDIV_REG;

typedef union
{
    struct {
#ifdef PMC_LITTLE_ENDIAN
	uint32 mdiv2		: 8;	// 07:00
	uint32 mdiv3		: 8;	// 15:08
	uint32 mdiv4		: 8;	// 23:16
	uint32 mdiv5		: 8;	// 31:24
#else
	uint32 mdiv5		: 8;	// 31:24
	uint32 mdiv4		: 8;	// 23:16
	uint32 mdiv3		: 8;	// 15:08
	uint32 mdiv2		: 8;	// 07:00
#endif
	} Bits;
	uint32 Reg32;
} PLL_DECCH25_REG;
#endif
#endif

typedef union
{
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32 manual_clk_en		: 1;
		uint32 manual_reset_ctl		: 1;
		uint32 freq_scale_used		: 1;	// R/O
		uint32 dpg_capable		: 1;	// R/O
		uint32 manual_mem_pwr		: 2;
		uint32 manual_iso_ctl		: 1;
		uint32 manual_ctl		: 1;
		uint32 dpg_ctl_en		: 1;
		uint32 pwr_dn_req		: 1;
		uint32 pwr_up_req		: 1;
		uint32 mem_pwr_ctl_en		: 1;
		uint32 blk_reset_assert		: 1;
		uint32 mem_stby			: 1;
		uint32 reserved			: 5;
		uint32 pwr_cntl_state		: 5;
		uint32 freq_scalar_dyn_sel	: 1;	// R/O
		uint32 pwr_off_state		: 1;	// R/O
		uint32 pwr_on_state		: 1;	// R/O
		uint32 pwr_good			: 1;	// R/O
		uint32 dpg_pwr_state		: 1;	// R/O
		uint32 mem_pwr_state		: 1;	// R/O
		uint32 iso_state		: 1;	// R/O
		uint32 reset_state		: 1;	// R/O
#else
		uint32 reset_state		: 1;	// R/O 31:31
		uint32 iso_state		: 1;	// R/O
		uint32 mem_pwr_state		: 1;	// R/O
		uint32 dpg_pwr_state		: 1;	// R/O
		uint32 pwr_good			: 1;	// R/O
		uint32 pwr_on_state		: 1;	// R/O
		uint32 pwr_off_state		: 1;	// R/O
		uint32 freq_scalar_dyn_sel	: 1;	// R/O
		uint32 pwr_cntl_state		: 5;
		uint32 reserved			: 5;
		uint32 mem_stby			: 1;
		uint32 blk_reset_assert		: 1;	//      12:12
		uint32 mem_pwr_ctl_en		: 1;
		uint32 pwr_up_req		: 1;
		uint32 pwr_dn_req		: 1;
		uint32 dpg_ctl_en		: 1;
		uint32 manual_ctl		: 1;
		uint32 manual_iso_ctl		: 1;
		uint32 manual_mem_pwr		: 2;
		uint32 dpg_capable		: 1;	// R/O
		uint32 freq_scale_used		: 1;	// R/O
		uint32 manual_reset_ctl		: 1;
		uint32 manual_clk_en		: 1;
#endif
	} Bits;
	uint32 Reg32;
} BPCM_PWR_ZONE_N_CONTROL;

typedef union
{
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32 pwr_ok_delay_sel	: 3;
		uint32 pwk_ok_thresh	: 2;
		uint32 reserved		: 3;
		uint32 iso_on_delay	: 4;
		uint32 iso_off_delay	: 4;
		uint32 clock_on_delay	: 4;
		uint32 clock_off_delay	: 4;
		uint32 reset_on_delay	: 4;
		uint32 reset_off_delay	: 4;
#else
		uint32 reset_off_delay	: 4;		// 31:28
		uint32 reset_on_delay	: 4;
		uint32 clock_off_delay	: 4;
		uint32 clock_on_delay	: 4;
		uint32 iso_off_delay	: 4;
		uint32 iso_on_delay	: 4;
		uint32 reserved		: 3;
		uint32 pwk_ok_thresh	: 2;
		uint32 pwr_ok_delay_sel	: 3;
#endif
	} Bits;
	uint32 Reg32;
} BPCM_PWR_ZONE_N_CONFIG1;

typedef union
{
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32 delay_prescale_sel	: 3;
		uint32 slew_prescale_sel	: 3;
		uint32 reserved			: 6;
		uint32 dpgn_on_delay		: 4;
		uint32 dpg1_on_delay		: 4;
		uint32 dpg_off_delay		: 4;
		uint32 mem_on_delay		: 4;
		uint32 mem_off_delay		: 4;
#else
		uint32 mem_off_delay		: 4;	// 31:28
		uint32 mem_on_delay		: 4;
		uint32 dpg_off_delay		: 4;
		uint32 dpg1_on_delay		: 4;
		uint32 dpgn_on_delay		: 4;
		uint32 reserved			: 6;
		uint32 slew_prescale_sel	: 3;
		uint32 delay_prescale_sel	: 3;
#endif
	} Bits;
	uint32 Reg32;
} BPCM_PWR_ZONE_N_CONFIG2;

typedef union
{
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32 fs_bypass_en	: 1;
		uint32 gear_sel		: 1;
		uint32 use_dyn_gear_sel	: 1;
		uint32 reserved2	: 1;
		uint32 low_gear_div	: 3;
		uint32 high_gear_div	: 3;
		uint32 reserved		: 22;
#else
		uint32 reserved		: 22;		// 31:10
		uint32 high_gear_div	: 3;
		uint32 low_gear_div	: 3;
		uint32 reserved2	: 1;
		uint32 use_dyn_gear_sel	: 1;
		uint32 gear_sel		: 1;
		uint32 fs_bypass_en	: 1;
#endif
	} Bits;
	uint32 Reg32;
} BPCM_ZONE_N_FREQ_SCALAR_CONTROL;

typedef struct
{
	BPCM_PWR_ZONE_N_CONTROL		control;
	BPCM_PWR_ZONE_N_CONFIG1		config1;
	BPCM_PWR_ZONE_N_CONFIG2		config2;
	BPCM_ZONE_N_FREQ_SCALAR_CONTROL	freq_scalar_control;
#if defined (_BCM96836_) || defined(CONFIG_BCM96836) || \
    defined (_BCM96846_) || defined(CONFIG_BCM96846) || \
    defined (_BCM96856_) || defined(CONFIG_BCM96856) 
    uint32 reserved[4];
#endif
} BPCM_ZONE;

#define BPCMZoneOffset(reg)	offsetof(BPCM_ZONE,reg)
#define BPCMZoneRegOffset(reg)	(BPCMZoneOffset(reg) >> 2)

typedef union
{
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32 pmb_Addr		: 8;
		uint32 hw_rev		: 8;
		uint32 module_id	: 16;
#else
		uint32 module_id	: 16;
		uint32 hw_rev		: 8;
		uint32 pmb_Addr		: 8;
#endif
	} Bits;
	uint32 Reg32;
} BPCM_UBUS_ID_REG;

typedef union
{
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32 num_zones	: 8;
		uint32 sr_reg_bits	: 8;
		uint32 pllType		: 2;
		uint32 reserved0	: 1;
		uint32 ubus		: 1;
		uint32 reserved1	: 12;
#else
		uint32 reserved1	: 12;
		uint32 ubus		: 1;
		uint32 reserved0	: 1;
		uint32 pllType		: 2;
		uint32 sr_reg_bits	: 8;
		uint32 num_zones	: 8;
#endif
	} Bits;
	uint32 Reg32;
} BPCM_UBUS_CAPABILITES_REG;

typedef union
{
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint32 ctrl_eswap	: 4;
		uint32 reserved0	: 4;
		uint32 ctrl_cd		: 4;
		uint32 reserved1	: 4;
		uint32 ctrl_seclev	: 8;
		uint32 reqout_seclev	: 8;
#else
		uint32 reqout_seclev	: 8;
		uint32 ctrl_seclev	: 8;
		uint32 reserved1	: 4;
		uint32 ctrl_cd		: 4;
		uint32 reserved0	: 4;
		uint32 ctrl_eswap	: 4;
#endif
	} Bits;	
	uint32 Reg32;
} BPCM_UBUS_CTRL_REG;

typedef union
{
	struct {
#ifdef PMC_LITTLE_ENDIAN
		uint64 addr_in		: 24;
		uint64 addr_out		: 24;
		uint64 pid		: 8;
		uint64 size		: 5;
		uint64 cmddta		: 1;
		uint64 en		: 2;
#else
		uint64 en		: 2;
		uint64 cmddta		: 1;
		uint64 size		: 5;
		uint64 pid		: 8;
		uint64 addr_out		: 24;
		uint64 addr_in		: 24;
#endif
	} Bits;	
	struct {
		uint32 word0;
		uint32 word1;
	} Regs32;
	uint64 Reg64;
} BPCM_UBUS_CFG_REG;

// There is a 20-bit address used to access any given BPCM register.  The upper 8-bits
// is the device address and the lower 12-bits is used to represent the BPCM register
// set for that device.  32-bit registers are allocated on 4-byte boundaries
// (i.e. 0, 1, 2, 3...) rather than on byte boundaries (0x00, 0x04, 0x08, 0x0c...)
// Thus, to get the actual address of any given register within the device's address
// space, I'll use the "C" offsetof macro and divide the result by 4
// e.g.:
// int regOffset = offsetof(BPCM_REGS,BPCM_AVS_PWD_CONTROL);	// yields the byte offset of the target register
// int regAddress = regOffset/4;				// yields the 32-bit word offset of the target register
// The ReadBPCMReg and WriteBPCMReg functions will always take a device address
// (address of the BPCM device) and register offset (like regOffset above).  The offset
// will be divided by 4 and used as the lower 12-bits of the actual target address, while the
// device address will serve as the upper 8-bits of the actual address.
typedef struct
{
#if defined(_BCM96838_) || defined(CONFIG_BCM96838)
	BPCM_ID_REG			id_reg;		// offset = 0x00, actual offset = 0
	BPCM_CAPABILITES_REG		capabilities;	// offset = 0x04, actual offset = 1
	uint32				control;	// offset = 0x08, actual offset = 2
	BPCM_STATUS_REG			status;		// offset = 0x0c, actual offset = 3
	BPCM_AVS_ROSC_CONTROL_REG	rosc_control;	// offset = 0x10, actual offset = 4
	BPCM_AVS_ROSC_THRESHOLD		rosc_threshold;	// offset = 0x14, actual offset = 5
	BPCM_AVS_ROSC_COUNT		rosc_count;	// offset = 0x18, actual offset = 6
	BPCM_AVS_PWD_CONTROL		pwd_control;	// offset = 0x1c, actual offset = 7
	BPCM_SR_CONTROL			sr_control;	/* offset = 0x20, actual offset = 8 */
	uint32				reserved[7];	// offset = 0x24, actual offset = 9..15
	BPCM_ZONE			zones[1020];	// offset = 0x40..0x3FFC, actual offset = 16..4095 (1020 * 4 = 4080 + 16 = 4096)
#elif defined(_BCM96836_) || defined(CONFIG_BCM96836) || defined(_BCM96846_) || defined(CONFIG_BCM96846) || defined(_BCM96856_) || defined(CONFIG_BCM96856)

    // PMB-slave:
    BPCM_ID_REG                 id_reg;             // offset 0x00, PMB reg index 0
    BPCM_CAPABILITES_REG        capabilities;       // offset 0x04, PMB reg index 1
    uint32                      reserved0[2];       // offset 0x08, PMB reg index 2/3
    // BPCM
    uint32                      control;            // offset 0x10, PMB reg index 4
    BPCM_SR_CONTROL             sr_control;         // offset 0x14, PMB reg index 5
    uint32                      reserved1[2];       // offset 0x18, PMB reg index 6/7
    // Client-specific registers
    uint32                      client_specific[24];// offset 0x20, PMB reg index 8..31
    // Zones
    BPCM_ZONE                   zones[];            // offset 0x20..(0x20 + MAX_ZONES*32)), PMB reg index 32..(32+(MAX_ZONES*8-1))
#else	
	BPCM_ID_REG			id_reg;		// offset = 0x00, actual offset = 0
	BPCM_CAPABILITES_REG		capabilities;	// offset = 0x04, actual offset = 1
	uint32				control;	// offset = 0x08, actual offset = 2
	BPCM_STATUS_REG			status;		// offset = 0x0c, actual offset = 3
	BPCM_AVS_ROSC_CONTROL_REG	rosc_control;	// offset = 0x10, actual offset = 4
	BPCM_AVS_ROSC_THRESHOLD		rosc_thresh_h;	// offset = 0x14, actual offset = 5
	BPCM_AVS_ROSC_THRESHOLD		rosc_thresh_s;	// offset = 0x18, actual offset = 6
	BPCM_AVS_ROSC_COUNT		rosc_count;	// offset = 0x1c, actual offset = 7
	BPCM_AVS_PWD_CONTROL		pwd_control;	// offset = 0x20, actual offset = 8
	BPCM_PWD_ACCUM_CONTROL		pwd_accum_control; // offset = 0x24, actual offset = 9
	BPCM_SR_CONTROL			sr_control;	// offset = 0x28, actual offset = 10

#if defined(_BCM963158_) || defined(CONFIG_BCM963158)
	uint32                      reserved;            // offset = 0x2c, actual offset = 11
	BPCM_GLOBAL_CNTL_0          global_control;      // offset = 0x30, actual offset = 12
	BPCM_GLOBAL_CNTL_1          global_control_1;    // offset = 0x34, actual offset = 13
	BPCM_GLOBAL_CNTL_2          global_control_2;    // offset = 0x38, actual offset = 14
	uint32			    global_status;       // offset = 0x3c, actual offset = 15
#else
	BPCM_GLOBAL_CNTL		global_control; // offset = 0x2c, actual offset = 11
	BPCM_MISC_CONTROL		misc_control;	// offset = 0x30, actual offset = 12
	BPCM_MISC_CONTROL2		misc_control2;	// offset = 0x34, actual offset = 13
	BPCM_SGPHY_CNTL			sgphy_cntl;	// offset = 0x38, actual offset = 14
	BPCM_SGPHY_STATUS		sgphy_status;	// offset = 0x3c, actual offset = 15
#endif
	BPCM_ZONE			zones[1020];	// offset = 0x40..0x3FFC, actual offset = 16..4095 (1020 * 4 = 4080 + 16 = 4096)
#endif
} BPCM_REGS;						// total offset space = 4096

#if !defined(_BCM96838_) && !defined(CONFIG_BCM96838) && !defined(_BCM96836_) && \
    !defined(CONFIG_BCM96836) && !defined(_BCM96846_) && !defined(CONFIG_BCM96846) && !defined(_BCM96856_) && !defined(CONFIG_BCM96856)
typedef struct
{
	BPCM_ID_REG			id_reg;		// offset = 0x00, actual offset = 0
	BPCM_CAPABILITES_REG		capabilities;	// offset = 0x04, actual offset = 1
	uint32				control;	// offset = 0x08, actual offset = 2
	BPCM_STATUS_REG			status;		// offset = 0x0c, actual offset = 3
	BPCM_AVS_ROSC_CONTROL_REG	rosc_control;	// offset = 0x10, actual offset = 4
	BPCM_AVS_ROSC_THRESHOLD		rosc_thresh_h;	// offset = 0x14, actual offset = 5
	BPCM_AVS_ROSC_THRESHOLD		rosc_thresh_s;	// offset = 0x18, actual offset = 6
	BPCM_AVS_ROSC_COUNT		rosc_count;	// offset = 0x1c, actual offset = 7
	BPCM_AVS_PWD_CONTROL		pwd_control;	// offset = 0x20, actual offset = 8
	BPCM_PWD_ACCUM_CONTROL		pwd_accum_control; // offset = 0x24, actual offset = 9
	BPCM_SR_CONTROL			sr_control;	// offset = 0x28, actual offset = 10
	BPCM_GLOBAL_CNTL		global_control; // offset = 0x2c, actual offset = 11
	BPCM_MISC_CONTROL		misc_control;	// offset = 0x30, actual offset = 12
	BPCM_MISC_CONTROL2		misc_control2;	// offset = 0x34, actual offset = 13
	uint32			        rvrsd[2];
	BPCM_ZONE			zones[1020];	// offset = 0x40..0x3FFC, actual offset = 16..4095 (1020 * 4 = 4080 + 16 = 4096)
} BPCM_VDSL_REGS;						// total offset space = 4096

#define BPCMVDSLOffset(reg)	offsetof(BPCM_VDSL_REGS,reg)
#define BPCMVDSLRegOffset(reg)	(BPCMVDSLOffset(reg) >> 2)

#endif

#define BPCMOffset(reg)		offsetof(BPCM_REGS,reg)
#define BPCMRegOffset(reg)	(BPCMOffset(reg) >> 2)

#if defined(_BCM96836_) || defined(CONFIG_BCM96836)
typedef struct
{
    CLASSIC_BPCM_ID_REG         id_reg;     // offset = 0x00, actual offset = 0
    BPCM_CAPABILITES_REG        capabilities;   // offset = 0x04, actual offset = 1
    uint32              control;    // offset = 0x08, actual offset = 2
    BPCM_STATUS_REG         status;     // offset = 0x0c, actual offset = 3
    BPCM_AVS_ROSC_CONTROL_REG   rosc_control;   // offset = 0x10, actual offset = 4
    BPCM_AVS_ROSC_THRESHOLD     rosc_thresh_h;  // offset = 0x14, actual offset = 5
    BPCM_AVS_ROSC_THRESHOLD     rosc_thresh_s;  // offset = 0x18, actual offset = 6
    BPCM_AVS_ROSC_COUNT     rosc_count; // offset = 0x1c, actual offset = 7
    BPCM_AVS_PWD_CONTROL        pwd_control;    // offset = 0x20, actual offset = 8
    BPCM_PWD_ACCUM_CONTROL      pwd_accum_control; // offset = 0x24, actual offset = 9
    BPCM_SR_CONTROL         sr_control; // offset = 0x28, actual offset = 10
    BPCM_GLOBAL_CNTL        global_control; // offset = 0x2c, actual offset = 11
    BPCM_MISC_CONTROL       misc_control;   // offset = 0x30, actual offset = 12
    BPCM_MISC_CONTROL2      misc_control2;  // offset = 0x34, actual offset = 13
    BPCM_SGPHY_CNTL         sgphy_cntl; // offset = 0x38, actual offset = 14
    BPCM_SGPHY_STATUS       sgphy_status;   // offset = 0x3c, actual offset = 15
    BPCM_ZONE           zones[1020];    // offset = 0x40..0x3FFC, actual offset = 16..4095 (1020 * 4 = 4080 + 16 = 4096)
} CLASSIC_BPCM_REGS;

#define ClassicBPCMOffset(reg)     offsetof(CLASSIC_BPCM_REGS,reg)
#define ClassicBPCMRegOffset(reg)  (ClassicBPCMOffset(reg) >> 2)
#endif

#if defined(_BCM963138_) || defined(CONFIG_BCM963138) || defined(_BCM963148_) || defined(CONFIG_BCM963148) || defined(_BCM96858_) || defined(CONFIG_BCM96858) || defined(_BCM963158_) || defined(CONFIG_BCM963158)
// ARM BPCM addresses as used by 63138/63148 and possibly others (28nm)
typedef struct
{
	BPCM_ID_REG			id_reg;		// offset = 0x00, actual offset = 0
	BPCM_CAPABILITES_REG		capabilities;	// offset = 0x04, actual offset = 1
	uint32				control;	// offset = 0x08, actual offset = 2
	BPCM_STATUS_REG			status;		// offset = 0x0c, actual offset = 3
	BPCM_AVS_ROSC_CONTROL_REG	rosc_control;	// offset = 0x10, actual offset = 4
	BPCM_AVS_ROSC_THRESHOLD		rosc_thresh_h;	// offset = 0x14, actual offset = 5
	BPCM_AVS_ROSC_THRESHOLD		rosc_thresh_s;	// offset = 0x18, actual offset = 6
	BPCM_AVS_ROSC_COUNT		rosc_count;	// offset = 0x1c, actual offset = 7
	BPCM_AVS_PWD_CONTROL		pwd_control;	// offset = 0x20, actual offset = 8
	BPCM_PWD_ACCUM_CONTROL		pwd_accum_control; // offset = 0x24, actual offset = 9
	BPCM_SR_CONTROL			sr_control;	// offset = 0x28, actual offset = 10
	uint32				reserved;	// offset = 0x2c, actual offset = 11
	ARM_CONTROL_REG			arm_control;	// offset = 0x30, actual offset = 12
#if defined(_BCM963138_) || defined(CONFIG_BCM963138)
	ARM_CPUx_PWR_CTRL_REG		arm_pwr_ctrl_0;	// offset = 0x34, actual offset = 13
	ARM_CPUx_PWR_CTRL_REG		arm_pwr_ctrl_1;	// offset = 0x38, actual offset = 14
	ARM_CPUx_PWR_CTRL_REG		arm_neon_l2;	// offset = 0x3c, actua; offset = 15
#elif defined(_BCM963148_) || defined(CONFIG_BCM963148)
	uint32				reserved1[3];	// offset = 0x34, actual offset = 13..15
#elif defined(_BCM96858_) || defined(CONFIG_BCM96858)
	uint32				biu_clk_control0;	// offset = 0x34, actual offset = 13
	uint32				reserved1[2];		// offset = 0x38, actual offset = 13..14
#endif
	BPCM_ZONE			zones[1020];	// offset = 0x40..0x3FFC, actual offset = 16..4095 (1020 * 4 = 4080 + 16 = 4096)
} ARM_BPCM_REGS;
#define ARMBPCMOffset(reg)  offsetof(ARM_BPCM_REGS,reg)
#define ARMBPCMRegOffset(reg)   (ARMBPCMOffset(reg) >> 2)

#elif defined(_BCM96836_) || defined(CONFIG_BCM96836) || defined(_BCM96846_) || defined(CONFIG_BCM96846) || defined(_BCM96856_) || defined(CONFIG_BCM96856)
typedef struct
{
    CLASSIC_BPCM_ID_REG           id_reg;          // offset = 0x00, actual offset = 0
    BPCM_CAPABILITES_REG          capabilities;    // offset = 0x04, actual offset = 1
    uint32                        control;         // offset = 0x08, actual offset = 2
    BPCM_STATUS_REG               status;          // offset = 0x0c, actual offset = 3
    BPCM_AVS_ROSC_CONTROL_REG     rosc_control;    // offset = 0x10, actual offset = 4
    BPCM_AVS_ROSC_THRESHOLD       rosc_thresh;     // offset = 0x14, actual offset = 5
    BPCM_AVS_ROSC_COUNT           rosc_count;      // offset = 0x18, actual offset = 6
    BPCM_AVS_PWD_CONTROL          pwd_control;     // offset = 0x1c, actual offset = 7
    BPCM_SR_CONTROL               sr_control;      // offset = 0x20, actual offset = 8
    uint32                        reserved0[3];    // offset = 0x24, actual offset = 9
    ARM_CONTROL_REG               arm_control;     // offset = 0x30, actual offset = 12
    uint32                        biu_clk_control0;// offset = 0x34, actual offset = 13
    uint32                        reserved1[2];    // offset = 0x38, actual offset = 14
    BPCM_ZONE                     zones;           // offset = 0x40, actual offset = 16
} ARM_BPCM_REGS;

#define ARMBPCMOffset(reg)  offsetof(ARM_BPCM_REGS,reg)
#define ARMBPCMRegOffset(reg)   (ARMBPCMOffset(reg) >> 2)

#endif

#if defined(_BCM96836_) || defined(CONFIG_BCM96836) || defined(_BCM96846_) || defined(CONFIG_BCM96846) || defined(_BCM96856_) || defined(CONFIG_BCM96856)
typedef struct
{
    BPCM_ID_REG         id_reg;     // offset = 0x00, actual offset = 0
    BPCM_CAPABILITES_REG        capabilities;   // offset = 0x04, actual offset = 1
    uint32              reserved0[2];   // offset = 0x08..0x0c, actual offset 2..3
    PLL_CTRL_REG            resets;     // offset = 0x10, actual offset = 4
    PLL_CFG0_REG            cfg0;       // offset = 0x14, actual offset = 5
    PLL_CFG1_REG            cfg1;       // offset = 0x18, actual offset = 6
    PLL_NDIV_REG            ndiv;       // offset = 0x1c, actual offset = 7
    PLL_PDIV_REG            pdiv;       // offset = 0x20, actual offset = 8
    PLL_LOOP0_REG           loop0;      // offset = 0x24, actual offset = 9
    PLL_LOOP1_REG           loop1;      // offset = 0x28, actual offset = a
    PLL_CHCFG_REG           ch01_cfg;   // offset = 0x2c, actual offset = b
    PLL_CHCFG_REG           ch23_cfg;   // offset = 0x30, actual offset = c
    PLL_CHCFG_REG           ch45_cfg;   // offset = 0x34, actual offset = d
    PLL_OCTRL_REG           octrl;      // offset = 0x38, actual offset = e
    PLL_STAT_REG            stat;       // offset = 0x3c, actual offset = f
    uint32              strap;      // offset = 0x40, actual offset = 0x10
    PLL_DECNDIV_REG         decndiv;    // offset = 0x44, actual offset = 0x11
    PLL_DECPDIV_REG         decpdiv;    // offset = 0x48, actual offset = 0x12
    PLL_DECCH25_REG         decch25;    // offset = 0x4c, actual offset = 0x13
} PLL_CLASSIC_BPCM_REGS;

#define PLLCLASSICBPCMOffset(reg)  offsetof(PLL_CLASSIC_BPCM_REGS,reg)
#define PLLCLASSICBPCMRegOffset(reg)   (PLLCLASSICBPCMOffset(reg) >> 2)

#endif

#if !defined(_BCM96838_) && !defined(CONFIG_BCM96838)
typedef struct
{
#if defined(_BCM96836_) || defined(CONFIG_BCM96836) || defined(_BCM96846_) || defined(CONFIG_BCM96846) || defined(_BCM96856_) || defined(CONFIG_BCM96856)
    BPCM_ID_REG              id_reg;         // offset = 0x00
    BPCM_CAPABILITES_REG     capabilities;   // offset = 0x04
    uint32                   reserved0[6];   // offset = 0x08
    PLL_CTRL_REG             resets;         // offset = 0x20
    PLL_CFG0_REG             cfg0;           // offset = 0x24
    PLL_CFG1_REG             cfg1;           // offset = 0x28
    PLL_NDIV_REG             ndiv;           // offset = 0x2c
    PLL_PDIV_REG             pdiv;           // offset = 0x30
    PLL_LOOP0_REG            loop0;          // offset = 0x34
    uint32                   reserved1;      // offset = 0x38
    PLL_LOOP1_REG            loop1;          // offset = 0x3c
    PLL_CHCFG_REG            ch01_cfg;       // offset = 0x40
    PLL_CHCFG_REG            ch23_cfg;       // offset = 0x44
    PLL_CHCFG_REG            ch45_cfg;       // offset = 0x48
    PLL_STAT_REG             stat;           // offset = 0x4c
    uint32                   strap;          // offset = 0x50
    PLL_DECNDIV_REG          decndiv;        // offset = 0x54
    PLL_DECPDIV_REG          decpdiv;        // offset = 0x58
    PLL_DECCH25_REG          decch25;        // offset = 0x5c
#else
	BPCM_ID_REG			id_reg;		// offset = 0x00, actual offset = 0
	BPCM_CAPABILITES_REG		capabilities;	// offset = 0x04, actual offset = 1
	uint32				reserved0[2];	// offset = 0x08..0x0c, actual offset 2..3
	PLL_CTRL_REG			resets;		// offset = 0x10, actual offset = 4
	PLL_CFG0_REG			cfg0;		// offset = 0x14, actual offset = 5
	PLL_CFG1_REG			cfg1;		// offset = 0x18, actual offset = 6
	PLL_NDIV_REG			ndiv;		// offset = 0x1c, actual offset = 7
	PLL_PDIV_REG			pdiv;		// offset = 0x20, actual offset = 8
	PLL_LOOP0_REG			loop0;		// offset = 0x24, actual offset = 9
	PLL_LOOP1_REG			loop1;		// offset = 0x28, actual offset = a
	PLL_CHCFG_REG			ch01_cfg;	// offset = 0x2c, actual offset = b
	PLL_CHCFG_REG			ch23_cfg;	// offset = 0x30, actual offset = c
	PLL_CHCFG_REG			ch45_cfg;	// offset = 0x34, actual offset = d
	PLL_OCTRL_REG			octrl;		// offset = 0x38, actual offset = e
	PLL_STAT_REG			stat;		// offset = 0x3c, actual offset = f
	uint32				strap;		// offset = 0x40, actual offset = 0x10
#if defined(_BCM96848_) || defined(CONFIG_BCM96848) || defined(_BCM96858_) || defined(CONFIG_BCM96858)
	PLL_DECNDIV_REG			decndiv;	// offset = 0x44, actual offset = 0x11
	PLL_DECPDIV_REG			decpdiv;	// offset = 0x48, actual offset = 0x12
	PLL_DECCH25_REG			decch25;	// offset = 0x4c, actual offset = 0x13
#else
	uint32				decndiv;	// offset = 0x44, actual offset = 0x11
	uint32				decpdiv;	// offset = 0x48, actual offset = 0x12
	uint32				decch25;	// offset = 0x4c, actual offset = 0x13
#endif
#endif
} PLL_BPCM_REGS;

#define PLLBPCMOffset(reg)	offsetof(PLL_BPCM_REGS,reg)
#define PLLBPCMRegOffset(reg)	(PLLBPCMOffset(reg) >> 2)
#endif

typedef struct
{
	BPCM_UBUS_ID_REG		id_reg;		/* offset = 0x00, actual offset = 0 */
	BPCM_UBUS_CAPABILITES_REG	capabilities;	/* offset = 0x04, actual offset = 1 */
	uint32				reserved0;	/* offset = 0x08, actual offset = 2 */
	BPCM_UBUS_CTRL_REG		ctrl;		/* offset = 0x0c, actual offset = 3 */
	BPCM_UBUS_CFG_REG		cfg[4];		/* offset = 0x10..0x2c, actual offset = 4..11 */
} BPCM_UBUS_REG;

#define UBUSBPCMOffset(reg)	offsetof(BPCM_UBUS_REG,reg)
#define UBUSBPCMRegOffset(reg)	(UBUSBPCMOffset(reg) >> 2)

typedef struct
{
	BPCM_ID_REG					id_reg;			// offset = 0x00, actual offset = 0
	BPCM_CAPABILITES_REG		capabilities;	// offset = 0x04, actual offset = 1
	uint32						control;		// offset = 0x08, actual offset = 2
	BPCM_STATUS_REG				status;			// offset = 0x0c, actual offset = 3
	BPCM_AVS_ROSC_CONTROL_REG	rosc_control;	// offset = 0x10, actual offset = 4
	BPCM_AVS_ROSC_THRESHOLD		rosc_thresh_h;	// offset = 0x14, actual offset = 5
	BPCM_AVS_ROSC_THRESHOLD		rosc_thresh_s;	// offset = 0x18, actual offset = 6
	BPCM_AVS_ROSC_COUNT			rosc_count;		// offset = 0x1c, actual offset = 7
	BPCM_AVS_PWD_CONTROL		pwd_control;	// offset = 0x20, actual offset = 8
	uint32						reserved0;		// offset = 0x24, actual offset = 9
	BPCM_SR_CONTROL				sr_control;		// offset = 0x28, actual offset = 10
	uint32						reserved1;		// offset = 0x2c, actual offset = 11
	uint32						clkrst_cfg;		// offset = 0x30, actual offset = 12
	uint32						clkrst_control;	// offset = 0x34, actual offset = 13
	uint32						xtal_control;	// offset = 0x38, actual offset = 14
	uint32						clkrst_stat;	// offset = 0x3c, actual offset = 15
} BPCM_CLKRST_REGS;

#define CLKRSTBPCMOffset(reg)  offsetof(BPCM_CLKRST_REGS, reg)
#define CLKRSTBPCMRegOffset(reg)   (CLKRSTBPCMOffset(reg) >> 2)

// *************************** macros ******************************
#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((unsigned long) &((TYPE *)0)->MEMBER)
#endif

#endif
