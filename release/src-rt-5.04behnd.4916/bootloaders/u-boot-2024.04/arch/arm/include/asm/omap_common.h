/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 */
#ifndef	_OMAP_COMMON_H_
#define	_OMAP_COMMON_H_

#ifndef __ASSEMBLY__

#include <linux/types.h>

#define NUM_SYS_CLKS	7
#define SYS_PTV		2	/* Divisor: 2^(PTV+1) => 8 */

struct bd_info;

struct prcm_regs {
	/* cm1.ckgen */
	u32 cm_clksel_core;
	u32 cm_clksel_abe;
	u32 cm_dll_ctrl;
	u32 cm_clkmode_dpll_core;
	u32 cm_idlest_dpll_core;
	u32 cm_autoidle_dpll_core;
	u32 cm_clksel_dpll_core;
	u32 cm_div_m2_dpll_core;
	u32 cm_div_m3_dpll_core;
	u32 cm_div_h11_dpll_core;
	u32 cm_div_h12_dpll_core;
	u32 cm_div_h13_dpll_core;
	u32 cm_div_h14_dpll_core;
	u32 cm_div_h21_dpll_core;
	u32 cm_div_h24_dpll_core;
	u32 cm_ssc_deltamstep_dpll_core;
	u32 cm_ssc_modfreqdiv_dpll_core;
	u32 cm_emu_override_dpll_core;
	u32 cm_div_h22_dpllcore;
	u32 cm_div_h23_dpll_core;
	u32 cm_clkmode_dpll_mpu;
	u32 cm_idlest_dpll_mpu;
	u32 cm_autoidle_dpll_mpu;
	u32 cm_clksel_dpll_mpu;
	u32 cm_div_m2_dpll_mpu;
	u32 cm_ssc_deltamstep_dpll_mpu;
	u32 cm_ssc_modfreqdiv_dpll_mpu;
	u32 cm_bypclk_dpll_mpu;
	u32 cm_clkmode_dpll_iva;
	u32 cm_idlest_dpll_iva;
	u32 cm_autoidle_dpll_iva;
	u32 cm_clksel_dpll_iva;
	u32 cm_div_h11_dpll_iva;
	u32 cm_div_h12_dpll_iva;
	u32 cm_ssc_deltamstep_dpll_iva;
	u32 cm_ssc_modfreqdiv_dpll_iva;
	u32 cm_bypclk_dpll_iva;
	u32 cm_clkmode_dpll_abe;
	u32 cm_idlest_dpll_abe;
	u32 cm_autoidle_dpll_abe;
	u32 cm_clksel_dpll_abe;
	u32 cm_div_m2_dpll_abe;
	u32 cm_div_m3_dpll_abe;
	u32 cm_ssc_deltamstep_dpll_abe;
	u32 cm_ssc_modfreqdiv_dpll_abe;
	u32 cm_clkmode_dpll_ddrphy;
	u32 cm_idlest_dpll_ddrphy;
	u32 cm_autoidle_dpll_ddrphy;
	u32 cm_clksel_dpll_ddrphy;
	u32 cm_div_m2_dpll_ddrphy;
	u32 cm_div_h11_dpll_ddrphy;
	u32 cm_div_h12_dpll_ddrphy;
	u32 cm_div_h13_dpll_ddrphy;
	u32 cm_ssc_deltamstep_dpll_ddrphy;
	u32 cm_clkmode_dpll_dsp;
	u32 cm_shadow_freq_config1;
	u32 cm_clkmode_dpll_gmac;
	u32 cm_mpu_mpu_clkctrl;

	/* cm1.dsp */
	u32 cm_dsp_clkstctrl;
	u32 cm_dsp_dsp_clkctrl;

	/* cm1.abe */
	u32 cm1_abe_clkstctrl;
	u32 cm1_abe_l4abe_clkctrl;
	u32 cm1_abe_aess_clkctrl;
	u32 cm1_abe_pdm_clkctrl;
	u32 cm1_abe_dmic_clkctrl;
	u32 cm1_abe_mcasp_clkctrl;
	u32 cm1_abe_mcbsp1_clkctrl;
	u32 cm1_abe_mcbsp2_clkctrl;
	u32 cm1_abe_mcbsp3_clkctrl;
	u32 cm1_abe_slimbus_clkctrl;
	u32 cm1_abe_timer5_clkctrl;
	u32 cm1_abe_timer6_clkctrl;
	u32 cm1_abe_timer7_clkctrl;
	u32 cm1_abe_timer8_clkctrl;
	u32 cm1_abe_wdt3_clkctrl;

	/* cm2.ckgen */
	u32 cm_clksel_mpu_m3_iss_root;
	u32 cm_clksel_usb_60mhz;
	u32 cm_scale_fclk;
	u32 cm_core_dvfs_perf1;
	u32 cm_core_dvfs_perf2;
	u32 cm_core_dvfs_perf3;
	u32 cm_core_dvfs_perf4;
	u32 cm_core_dvfs_current;
	u32 cm_iva_dvfs_perf_tesla;
	u32 cm_iva_dvfs_perf_ivahd;
	u32 cm_iva_dvfs_perf_abe;
	u32 cm_iva_dvfs_current;
	u32 cm_clkmode_dpll_per;
	u32 cm_idlest_dpll_per;
	u32 cm_autoidle_dpll_per;
	u32 cm_clksel_dpll_per;
	u32 cm_div_m2_dpll_per;
	u32 cm_div_m3_dpll_per;
	u32 cm_div_h11_dpll_per;
	u32 cm_div_h12_dpll_per;
	u32 cm_div_h13_dpll_per;
	u32 cm_div_h14_dpll_per;
	u32 cm_ssc_deltamstep_dpll_per;
	u32 cm_ssc_modfreqdiv_dpll_per;
	u32 cm_emu_override_dpll_per;
	u32 cm_clkmode_dpll_usb;
	u32 cm_idlest_dpll_usb;
	u32 cm_autoidle_dpll_usb;
	u32 cm_clksel_dpll_usb;
	u32 cm_div_m2_dpll_usb;
	u32 cm_ssc_deltamstep_dpll_usb;
	u32 cm_ssc_modfreqdiv_dpll_usb;
	u32 cm_clkdcoldo_dpll_usb;
	u32 cm_clkmode_dpll_pcie_ref;
	u32 cm_clkmode_apll_pcie;
	u32 cm_idlest_apll_pcie;
	u32 cm_div_m2_apll_pcie;
	u32 cm_clkvcoldo_apll_pcie;
	u32 cm_clkmode_dpll_unipro;
	u32 cm_idlest_dpll_unipro;
	u32 cm_autoidle_dpll_unipro;
	u32 cm_clksel_dpll_unipro;
	u32 cm_div_m2_dpll_unipro;
	u32 cm_ssc_deltamstep_dpll_unipro;
	u32 cm_ssc_modfreqdiv_dpll_unipro;
	u32 cm_coreaon_usb_phy1_core_clkctrl;
	u32 cm_coreaon_usb_phy2_core_clkctrl;
	u32 cm_coreaon_usb_phy3_core_clkctrl;
	u32 cm_coreaon_l3init_60m_gfclk_clkctrl;

	/* cm2.core */
	u32 cm_coreaon_bandgap_clkctrl;
	u32 cm_coreaon_io_srcomp_clkctrl;
	u32 cm_l3_1_clkstctrl;
	u32 cm_l3_1_dynamicdep;
	u32 cm_l3_1_l3_1_clkctrl;
	u32 cm_l3_2_clkstctrl;
	u32 cm_l3_2_dynamicdep;
	u32 cm_l3_2_l3_2_clkctrl;
	u32 cm_l3_gpmc_clkctrl;
	u32 cm_l3_2_ocmc_ram_clkctrl;
	u32 cm_mpu_m3_clkstctrl;
	u32 cm_mpu_m3_staticdep;
	u32 cm_mpu_m3_dynamicdep;
	u32 cm_mpu_m3_mpu_m3_clkctrl;
	u32 cm_sdma_clkstctrl;
	u32 cm_sdma_staticdep;
	u32 cm_sdma_dynamicdep;
	u32 cm_sdma_sdma_clkctrl;
	u32 cm_memif_clkstctrl;
	u32 cm_memif_dmm_clkctrl;
	u32 cm_memif_emif_fw_clkctrl;
	u32 cm_memif_emif_1_clkctrl;
	u32 cm_memif_emif_2_clkctrl;
	u32 cm_memif_dll_clkctrl;
	u32 cm_memif_emif_h1_clkctrl;
	u32 cm_memif_emif_h2_clkctrl;
	u32 cm_memif_dll_h_clkctrl;
	u32 cm_c2c_clkstctrl;
	u32 cm_c2c_staticdep;
	u32 cm_c2c_dynamicdep;
	u32 cm_c2c_sad2d_clkctrl;
	u32 cm_c2c_modem_icr_clkctrl;
	u32 cm_c2c_sad2d_fw_clkctrl;
	u32 cm_l4cfg_clkstctrl;
	u32 cm_l4cfg_dynamicdep;
	u32 cm_l4cfg_l4_cfg_clkctrl;
	u32 cm_l4cfg_hw_sem_clkctrl;
	u32 cm_l4cfg_mailbox_clkctrl;
	u32 cm_l4cfg_sar_rom_clkctrl;
	u32 cm_l3instr_clkstctrl;
	u32 cm_l3instr_l3_3_clkctrl;
	u32 cm_l3instr_l3_instr_clkctrl;
	u32 cm_l3instr_intrconn_wp1_clkctrl;

	/* cm2.ivahd */
	u32 cm_ivahd_clkstctrl;
	u32 cm_ivahd_ivahd_clkctrl;
	u32 cm_ivahd_sl2_clkctrl;

	/* cm2.cam */
	u32 cm_cam_clkstctrl;
	u32 cm_cam_iss_clkctrl;
	u32 cm_cam_fdif_clkctrl;
	u32 cm_cam_vip1_clkctrl;
	u32 cm_cam_vip2_clkctrl;
	u32 cm_cam_vip3_clkctrl;
	u32 cm_cam_lvdsrx_clkctrl;
	u32 cm_cam_csi1_clkctrl;
	u32 cm_cam_csi2_clkctrl;

	/* cm2.dss */
	u32 cm_dss_clkstctrl;
	u32 cm_dss_dss_clkctrl;

	/* cm2.sgx */
	u32 cm_sgx_clkstctrl;
	u32 cm_sgx_sgx_clkctrl;

	/* cm2.l3init */
	u32 cm_l3init_clkstctrl;

	/* cm2.l3init */
	u32 cm_l3init_hsmmc1_clkctrl;
	u32 cm_l3init_hsmmc2_clkctrl;
	u32 cm_l3init_hsi_clkctrl;
	u32 cm_l3init_hsusbhost_clkctrl;
	u32 cm_l3init_hsusbotg_clkctrl;
	u32 cm_l3init_hsusbtll_clkctrl;
	u32 cm_l3init_p1500_clkctrl;
	u32 cm_l3init_sata_clkctrl;
	u32 cm_l3init_fsusb_clkctrl;
	u32 cm_l3init_ocp2scp1_clkctrl;
	u32 cm_l3init_ocp2scp3_clkctrl;
	u32 cm_l3init_usb_otg_ss1_clkctrl;
	u32 cm_l3init_usb_otg_ss2_clkctrl;

	u32 prm_irqstatus_mpu;
	u32 prm_irqstatus_mpu_2;

	/* cm2.l4per */
	u32 cm_l4per_clkstctrl;
	u32 cm_l4per_dynamicdep;
	u32 cm_l4per_adc_clkctrl;
	u32 cm_l4per_gptimer10_clkctrl;
	u32 cm_l4per_gptimer11_clkctrl;
	u32 cm_l4per_gptimer2_clkctrl;
	u32 cm_l4per_gptimer3_clkctrl;
	u32 cm_l4per_gptimer4_clkctrl;
	u32 cm_l4per_gptimer9_clkctrl;
	u32 cm_l4per_elm_clkctrl;
	u32 cm_l4per_gpio2_clkctrl;
	u32 cm_l4per_gpio3_clkctrl;
	u32 cm_l4per_gpio4_clkctrl;
	u32 cm_l4per_gpio5_clkctrl;
	u32 cm_l4per_gpio6_clkctrl;
	u32 cm_l4per_hdq1w_clkctrl;
	u32 cm_l4per_hecc1_clkctrl;
	u32 cm_l4per_hecc2_clkctrl;
	u32 cm_l4per_i2c1_clkctrl;
	u32 cm_l4per_i2c2_clkctrl;
	u32 cm_l4per_i2c3_clkctrl;
	u32 cm_l4per_i2c4_clkctrl;
	u32 cm_l4per_l4per_clkctrl;
	u32 cm_l4per_mcasp2_clkctrl;
	u32 cm_l4per_mcasp3_clkctrl;
	u32 cm_l4per_mgate_clkctrl;
	u32 cm_l4per_mcspi1_clkctrl;
	u32 cm_l4per_mcspi2_clkctrl;
	u32 cm_l4per_mcspi3_clkctrl;
	u32 cm_l4per_mcspi4_clkctrl;
	u32 cm_l4per_gpio7_clkctrl;
	u32 cm_l4per_gpio8_clkctrl;
	u32 cm_l4per_mmcsd3_clkctrl;
	u32 cm_l4per_mmcsd4_clkctrl;
	u32 cm_l4per_msprohg_clkctrl;
	u32 cm_l4per_slimbus2_clkctrl;
	u32 cm_l4per_qspi_clkctrl;
	u32 cm_l4per_uart1_clkctrl;
	u32 cm_l4per_uart2_clkctrl;
	u32 cm_l4per_uart3_clkctrl;
	u32 cm_l4per_uart4_clkctrl;
	u32 cm_l4per_mmcsd5_clkctrl;
	u32 cm_l4per_i2c5_clkctrl;
	u32 cm_l4per_uart5_clkctrl;
	u32 cm_l4per_uart6_clkctrl;
	u32 cm_l4sec_clkstctrl;
	u32 cm_l4sec_staticdep;
	u32 cm_l4sec_dynamicdep;
	u32 cm_l4sec_aes1_clkctrl;
	u32 cm_l4sec_aes2_clkctrl;
	u32 cm_l4sec_des3des_clkctrl;
	u32 cm_l4sec_pkaeip29_clkctrl;
	u32 cm_l4sec_rng_clkctrl;
	u32 cm_l4sec_sha2md51_clkctrl;
	u32 cm_l4sec_cryptodma_clkctrl;

	/* l4 wkup regs */
	u32 cm_abe_pll_ref_clksel;
	u32 cm_sys_clksel;
	u32 cm_abe_pll_sys_clksel;
	u32 cm_wkup_clkstctrl;
	u32 cm_wkup_l4wkup_clkctrl;
	u32 cm_wkup_wdtimer1_clkctrl;
	u32 cm_wkup_wdtimer2_clkctrl;
	u32 cm_wkup_gpio1_clkctrl;
	u32 cm_wkup_gptimer1_clkctrl;
	u32 cm_wkup_gptimer12_clkctrl;
	u32 cm_wkup_synctimer_clkctrl;
	u32 cm_wkup_usim_clkctrl;
	u32 cm_wkup_sarram_clkctrl;
	u32 cm_wkup_keyboard_clkctrl;
	u32 cm_wkup_rtc_clkctrl;
	u32 cm_wkup_bandgap_clkctrl;
	u32 cm_wkupaon_scrm_clkctrl;
	u32 cm_wkupaon_io_srcomp_clkctrl;
	u32 prm_rstctrl;
	u32 prm_rstst;
	u32 prm_rsttime;
	u32 prm_io_pmctrl;
	u32 prm_vc_val_bypass;
	u32 prm_vc_cfg_i2c_mode;
	u32 prm_vc_cfg_i2c_clk;
	u32 prm_abbldo_mpu_setup;
	u32 prm_abbldo_mpu_ctrl;
	u32 prm_abbldo_mm_setup;
	u32 prm_abbldo_mm_ctrl;
	u32 prm_abbldo_iva_setup;
	u32 prm_abbldo_iva_ctrl;
	u32 prm_abbldo_eve_setup;
	u32 prm_abbldo_eve_ctrl;
	u32 prm_abbldo_gpu_setup;
	u32 prm_abbldo_gpu_ctrl;

	u32 cm_div_m4_dpll_core;
	u32 cm_div_m5_dpll_core;
	u32 cm_div_m6_dpll_core;
	u32 cm_div_m7_dpll_core;
	u32 cm_div_m4_dpll_iva;
	u32 cm_div_m5_dpll_iva;
	u32 cm_div_m4_dpll_ddrphy;
	u32 cm_div_m5_dpll_ddrphy;
	u32 cm_div_m6_dpll_ddrphy;
	u32 cm_div_m4_dpll_per;
	u32 cm_div_m5_dpll_per;
	u32 cm_div_m6_dpll_per;
	u32 cm_div_m7_dpll_per;
	u32 cm_l3instr_intrconn_wp1_clkct;
	u32 cm_l3init_usbphy_clkctrl;
	u32 cm_l4per_mcbsp4_clkctrl;
	u32 prm_vc_cfg_channel;

	/* SCRM stuff, used by some boards */
	u32 scrm_auxclk0;
	u32 scrm_auxclk1;

	/* GMAC Clk Ctrl */
	u32 cm_gmac_gmac_clkctrl;
	u32 cm_gmac_clkstctrl;

	/* IPU */
	u32 cm_ipu_clkstctrl;
	u32 cm_ipu_i2c5_clkctrl;
	u32 cm_ipu1_clkstctrl;
	u32 cm_ipu1_ipu1_clkctrl;
	u32 cm_ipu2_clkstctrl;
	u32 cm_ipu2_ipu2_clkctrl;

	/*l3main1 edma*/
	u32 cm_l3main1_tptc1_clkctrl;
	u32 cm_l3main1_tptc2_clkctrl;
};

struct omap_sys_ctrl_regs {
	u32 control_status;
	u32 control_core_mac_id_0_lo;
	u32 control_core_mac_id_0_hi;
	u32 control_core_mac_id_1_lo;
	u32 control_core_mac_id_1_hi;
	u32 control_phy_power_usb;
	u32 control_core_mmr_lock1;
	u32 control_core_mmr_lock2;
	u32 control_core_mmr_lock3;
	u32 control_core_mmr_lock4;
	u32 control_core_mmr_lock5;
	u32 control_core_control_io1;
	u32 control_core_control_io2;
	u32 control_id_code;
	u32 control_std_fuse_die_id_0;
	u32 control_std_fuse_die_id_1;
	u32 control_std_fuse_die_id_2;
	u32 control_std_fuse_die_id_3;
	u32 control_std_fuse_opp_bgap;
	u32 control_ldosram_iva_voltage_ctrl;
	u32 control_ldosram_mpu_voltage_ctrl;
	u32 control_ldosram_core_voltage_ctrl;
	u32 control_usbotghs_ctrl;
	u32 control_phy_power_sata;
	u32 control_padconf_core_base;
	u32 control_paconf_global;
	u32 control_paconf_mode;
	u32 control_smart1io_padconf_0;
	u32 control_smart1io_padconf_1;
	u32 control_smart1io_padconf_2;
	u32 control_smart2io_padconf_0;
	u32 control_smart2io_padconf_1;
	u32 control_smart2io_padconf_2;
	u32 control_smart3io_padconf_0;
	u32 control_smart3io_padconf_1;
	u32 control_pbias;
	u32 control_i2c_0;
	u32 control_camera_rx;
	u32 control_hdmi_tx_phy;
	u32 control_uniportm;
	u32 control_dsiphy;
	u32 control_mcbsplp;
	u32 control_usb2phycore;
	u32 control_hdmi_1;
	u32 control_hsi;
	u32 control_ddr3ch1_0;
	u32 control_ddr3ch2_0;
	u32 control_ddrch1_0;
	u32 control_ddrch1_1;
	u32 control_ddrch2_0;
	u32 control_ddrch2_1;
	u32 control_lpddr2ch1_0;
	u32 control_lpddr2ch1_1;
	u32 control_ddrio_0;
	u32 control_ddrio_1;
	u32 control_ddrio_2;
	u32 control_ddr_control_ext_0;
	u32 control_lpddr2io1_0;
	u32 control_lpddr2io1_1;
	u32 control_lpddr2io1_2;
	u32 control_lpddr2io1_3;
	u32 control_lpddr2io2_0;
	u32 control_lpddr2io2_1;
	u32 control_lpddr2io2_2;
	u32 control_lpddr2io2_3;
	u32 control_hyst_1;
	u32 control_usbb_hsic_control;
	u32 control_c2c;
	u32 control_core_control_spare_rw;
	u32 control_core_control_spare_r;
	u32 control_core_control_spare_r_c0;
	u32 control_srcomp_north_side;
	u32 control_srcomp_south_side;
	u32 control_srcomp_east_side;
	u32 control_srcomp_west_side;
	u32 control_srcomp_code_latch;
	u32 control_pbiaslite;
	u32 control_port_emif1_sdram_config;
	u32 control_port_emif1_lpddr2_nvm_config;
	u32 control_port_emif2_sdram_config;
	u32 control_emif1_sdram_config_ext;
	u32 control_emif2_sdram_config_ext;
	u32 control_wkup_ldovbb_mpu_voltage_ctrl;
	u32 control_wkup_ldovbb_mm_voltage_ctrl;
	u32 control_wkup_ldovbb_iva_voltage_ctrl;
	u32 control_wkup_ldovbb_eve_voltage_ctrl;
	u32 control_wkup_ldovbb_gpu_voltage_ctrl;
	u32 control_smart1nopmio_padconf_0;
	u32 control_smart1nopmio_padconf_1;
	u32 control_padconf_mode;
	u32 control_xtal_oscillator;
	u32 control_i2c_2;
	u32 control_ckobuffer;
	u32 control_wkup_control_spare_rw;
	u32 control_wkup_control_spare_r;
	u32 control_wkup_control_spare_r_c0;
	u32 control_srcomp_east_side_wkup;
	u32 control_efuse_1;
	u32 control_efuse_2;
	u32 control_efuse_3;
	u32 control_efuse_4;
	u32 control_efuse_5;
	u32 control_efuse_6;
	u32 control_efuse_7;
	u32 control_efuse_8;
	u32 control_efuse_9;
	u32 control_efuse_10;
	u32 control_efuse_11;
	u32 control_efuse_12;
	u32 control_efuse_13;
	u32 control_padconf_wkup_base;
	u32 iodelay_config_base;
	u32 ctrl_core_sma_sw_0;
	u32 ctrl_core_sma_sw_1;
};

#if defined(CONFIG_OMAP44XX) || defined(CONFIG_OMAP54XX)
struct dpll_params {
	u32 m;
	u32 n;
	s8 m2;
	s8 m3;
	s8 m4_h11;
	s8 m5_h12;
	s8 m6_h13;
	s8 m7_h14;
	s8 h21;
	s8 h22;
	s8 h23;
	s8 h24;
};

struct dpll_regs {
	u32 cm_clkmode_dpll;
	u32 cm_idlest_dpll;
	u32 cm_autoidle_dpll;
	u32 cm_clksel_dpll;
	u32 cm_div_m2_dpll;
	u32 cm_div_m3_dpll;
	u32 cm_div_m4_h11_dpll;
	u32 cm_div_m5_h12_dpll;
	u32 cm_div_m6_h13_dpll;
	u32 cm_div_m7_h14_dpll;
	u32 reserved[2];
	u32 cm_div_h21_dpll;
	u32 cm_div_h22_dpll;
	u32 cm_div_h23_dpll;
	u32 cm_div_h24_dpll;
};
#endif /* CONFIG_OMAP44XX || CONFIG_OMAP54XX */

struct dplls {
	const struct dpll_params *mpu;
	const struct dpll_params *core;
	const struct dpll_params *per;
	const struct dpll_params *abe;
	const struct dpll_params *iva;
	const struct dpll_params *usb;
	const struct dpll_params *ddr;
	const struct dpll_params *gmac;
};

struct pmic_data {
	u32 base_offset;
	u32 step;
	u32 start_code;
	unsigned gpio;
	int gpio_en;
	u32 i2c_slave_addr;
	void (*pmic_bus_init)(void);
	int (*pmic_write)(u8 sa, u8 reg_addr, u8 reg_data);
};

#if defined(CONFIG_OMAP44XX) || defined(CONFIG_OMAP54XX)
enum {
	OPP_LOW,
	OPP_NOM,
	OPP_OD,
	OPP_HIGH,
	NUM_OPPS,
};

/**
 * struct volts_efuse_data - efuse definition for voltage
 * @reg:	register address for efuse
 * @reg_bits:	Number of bits in a register address, mandatory.
 */
struct volts_efuse_data {
	u32 reg[NUM_OPPS];
	u8 reg_bits;
};

struct volts {
	u32 value[NUM_OPPS];
	u32 addr;
	struct volts_efuse_data efuse;
	struct pmic_data *pmic;

	u32 abb_tx_done_mask;
};

enum {
	VOLT_MPU,
	VOLT_CORE,
	VOLT_MM,
	VOLT_GPU,
	VOLT_EVE,
	VOLT_IVA,
	NUM_VOLT_RAILS,
};

struct vcores_data {
	struct volts mpu;
	struct volts core;
	struct volts mm;
	struct volts gpu;
	struct volts eve;
	struct volts iva;
};
#endif /* CONFIG_OMAP44XX || CONFIG_OMAP54XX */

extern struct prcm_regs const **prcm;
extern struct prcm_regs const omap5_es1_prcm;
extern struct prcm_regs const omap5_es2_prcm;
extern struct prcm_regs const omap4_prcm;
extern struct prcm_regs const dra7xx_prcm;
extern struct dplls const **dplls_data;
extern struct dplls dra7xx_dplls;
extern struct dplls dra72x_dplls;
extern struct dplls dra76x_dplls;
extern struct vcores_data const **omap_vcores;
extern const u32 sys_clk_array[8];
extern struct omap_sys_ctrl_regs const **ctrl;
extern struct omap_sys_ctrl_regs const am33xx_ctrl;
extern struct omap_sys_ctrl_regs const omap3_ctrl;
extern struct omap_sys_ctrl_regs const omap4_ctrl;
extern struct omap_sys_ctrl_regs const omap5_ctrl;
extern struct omap_sys_ctrl_regs const dra7xx_ctrl;

extern struct pmic_data tps659038;
extern struct pmic_data lp8733;
extern struct pmic_data lp87565;

void hw_data_init(void);

const struct dpll_params *get_mpu_dpll_params(struct dplls const *);
const struct dpll_params *get_core_dpll_params(struct dplls const *);
const struct dpll_params *get_per_dpll_params(struct dplls const *);
const struct dpll_params *get_iva_dpll_params(struct dplls const *);
const struct dpll_params *get_usb_dpll_params(struct dplls const *);
const struct dpll_params *get_abe_dpll_params(struct dplls const *);

#if defined(CONFIG_OMAP44XX) || defined(CONFIG_OMAP54XX)
void do_enable_clocks(u32 const *clk_domains,
		      u32 const *clk_modules_hw_auto,
		      u32 const *clk_modules_explicit_en,
		      u8 wait_for_enable);

void do_disable_clocks(u32 const *clk_domains,
		       u32 const *clk_modules_disable,
		       u8 wait_for_disable);
#endif /* CONFIG_OMAP44XX || CONFIG_OMAP54XX */

void do_enable_ipu_clocks(u32 const *clk_domains,
			  u32 const *clk_modules_hw_auto,
			  u32 const *clk_modules_explicit_en,
			  u8 wait_for_enable);
void enable_ipu1_clocks(void);
void enable_ipu2_clocks(void);
void setup_post_dividers(u32 const base,
			const struct dpll_params *params);
u32 omap_ddr_clk(void);
u32 get_sys_clk_index(void);
void enable_basic_clocks(void);
void enable_basic_uboot_clocks(void);

void enable_usb_clocks(int index);
void disable_usb_clocks(int index);

#if defined(CONFIG_OMAP44XX) || defined(CONFIG_OMAP54XX)
void scale_vcores(struct vcores_data const *);
#endif /* CONFIG_OMAP44XX || CONFIG_OMAP54XX */
int get_voltrail_opp(int rail_offset);
u32 get_offset_code(u32 volt_offset, struct pmic_data *pmic);
void do_scale_vcore(u32 vcore_reg, u32 volt_mv, struct pmic_data *pmic);
void abb_setup(u32 fuse, u32 ldovbb, u32 setup, u32 control,
	       u32 txdone, u32 txdone_mask, u32 opp);
s8 abb_setup_ldovbb(u32 fuse, u32 ldovbb);

struct tag_serialnr;

void omap_die_id_serial(void);
void omap_die_id_get_board_serial(struct tag_serialnr *serialnr);
void omap_die_id_usbethaddr(void);
void omap_die_id_display(void);

#ifdef CONFIG_FASTBOOT_FLASH
void omap_set_fastboot_vars(void);
#else
static inline void omap_set_fastboot_vars(void) { }
#endif

void recalibrate_iodelay(void);

void omap_smc1(u32 service, u32 val);

/*
 * Low-level helper function used when performing secure ROM calls on high-
 * security (HS) device variants by doing a specially-formed smc entry.
 */
u32 omap_smc_sec(u32 service, u32 proc_id, u32 flag, u32 *params);
u32 omap_smc_sec_cpu1(u32 service, u32 proc_id, u32 flag, u32 *params);

void enable_edma3_clocks(void);
void disable_edma3_clocks(void);

void omap_die_id(unsigned int *die_id);

/* Initialize general purpose I2C(0) on the SoC */
void gpi2c_init(void);

/* Common FDT Fixups */
int ft_hs_disable_rng(void *fdt, struct bd_info *bd);
int ft_hs_fixup_dram(void *fdt, struct bd_info *bd);
int ft_hs_add_tee(void *fdt, struct bd_info *bd);

/* ABB */
#define OMAP_ABB_NOMINAL_OPP		0
#define OMAP_ABB_FAST_OPP		1
#define OMAP_ABB_SLOW_OPP		3
#define OMAP_ABB_CONTROL_FAST_OPP_SEL_MASK		(0x1 << 0)
#define OMAP_ABB_CONTROL_SLOW_OPP_SEL_MASK		(0x1 << 1)
#define OMAP_ABB_CONTROL_OPP_CHANGE_MASK		(0x1 << 2)
#define OMAP_ABB_CONTROL_SR2_IN_TRANSITION_MASK		(0x1 << 6)
#define OMAP_ABB_SETUP_SR2EN_MASK			(0x1 << 0)
#define OMAP_ABB_SETUP_ACTIVE_FBB_SEL_MASK		(0x1 << 2)
#define OMAP_ABB_SETUP_ACTIVE_RBB_SEL_MASK		(0x1 << 1)
#define OMAP_ABB_SETUP_SR2_WTCNT_VALUE_MASK		(0xff << 8)

static inline u32 omap_revision(void)
{
	extern u32 *const omap_si_rev;
	return *omap_si_rev;
}

#define OMAP44xx	0x44000000

static inline u8 is_omap44xx(void)
{
	extern u32 *const omap_si_rev;
	return (*omap_si_rev & 0xFF000000) == OMAP44xx;
};

#define OMAP54xx	0x54000000

static inline u8 is_omap54xx(void)
{
	extern u32 *const omap_si_rev;
	return ((*omap_si_rev & 0xFF000000) == OMAP54xx);
}

#define DRA7XX		0x07000000
#define DRA72X		0x07200000
#define DRA76X		0x07600000

static inline u8 is_dra7xx(void)
{
	extern u32 *const omap_si_rev;
	return ((*omap_si_rev & 0xFF000000) == DRA7XX);
}

static inline u8 is_dra72x(void)
{
	extern u32 *const omap_si_rev;
	return (*omap_si_rev & 0xFFF00000) == DRA72X;
}

static inline u8 is_dra76x(void)
{
	extern u32 *const omap_si_rev;
	return (*omap_si_rev & 0xFFF00000) == DRA76X;
}

static inline u8 is_dra76x_abz(void)
{
	extern u32 *const omap_si_rev;
	return (*omap_si_rev & 0xF) == 2;
}

static inline u8 is_dra76x_acd(void)
{
	extern u32 *const omap_si_rev;
	return (*omap_si_rev & 0xF) == 3;
}
#endif

/*
 * silicon revisions.
 * Moving this to common, so that most of code can be moved to common,
 * directories.
 */

/* omap4 */
#define OMAP4430_SILICON_ID_INVALID	0xFFFFFFFF
#define OMAP4430_ES1_0	0x44300100
#define OMAP4430_ES2_0	0x44300200
#define OMAP4430_ES2_1	0x44300210
#define OMAP4430_ES2_2	0x44300220
#define OMAP4430_ES2_3	0x44300230
#define OMAP4460_ES1_0	0x44600100
#define OMAP4460_ES1_1	0x44600110
#define OMAP4470_ES1_0	0x44700100

/* omap5 */
#define OMAP5430_SILICON_ID_INVALID	0
#define OMAP5430_ES1_0	0x54300100
#define OMAP5432_ES1_0	0x54320100
#define OMAP5430_ES2_0  0x54300200
#define OMAP5432_ES2_0  0x54320200

/* DRA7XX */
#define DRA762_ES1_0	0x07620100
#define DRA752_ES1_0	0x07520100
#define DRA752_ES1_1	0x07520110
#define DRA752_ES2_0	0x07520200
#define DRA722_ES1_0	0x07220100
#define DRA722_ES2_0	0x07220200
#define DRA722_ES2_1	0x07220210

#define DRA762_ABZ_ES1_0	0x07620102
#define DRA762_ACD_ES1_0	0x07620103
/*
 * silicon device type
 * Moving to common from cpu.h, since it is shared by various omap devices
 */
#define TST_DEVICE          0x0
#define EMU_DEVICE          0x1
#define HS_DEVICE           0x2
#define GP_DEVICE           0x3


/*
 * SRAM scratch space entries
 */
#define OMAP_SRAM_SCRATCH_OMAP_REV	SRAM_SCRATCH_SPACE_ADDR
#define OMAP_SRAM_SCRATCH_EMIF_SIZE	(SRAM_SCRATCH_SPACE_ADDR + 0x4)
#define OMAP_SRAM_SCRATCH_EMIF_T_NUM	(SRAM_SCRATCH_SPACE_ADDR + 0xC)
#define OMAP_SRAM_SCRATCH_EMIF_T_DEN	(SRAM_SCRATCH_SPACE_ADDR + 0x10)
#define OMAP_SRAM_SCRATCH_PRCM_PTR      (SRAM_SCRATCH_SPACE_ADDR + 0x14)
#define OMAP_SRAM_SCRATCH_DPLLS_PTR     (SRAM_SCRATCH_SPACE_ADDR + 0x18)
#define OMAP_SRAM_SCRATCH_VCORES_PTR    (SRAM_SCRATCH_SPACE_ADDR + 0x1C)
#define OMAP_SRAM_SCRATCH_SYS_CTRL	(SRAM_SCRATCH_SPACE_ADDR + 0x20)
#define OMAP_SRAM_SCRATCH_BOOT_PARAMS	(SRAM_SCRATCH_SPACE_ADDR + 0x24)
#ifndef TI_SRAM_SCRATCH_BOARD_EEPROM_START
#define TI_SRAM_SCRATCH_BOARD_EEPROM_START (SRAM_SCRATCH_SPACE_ADDR + 0x28)
#define TI_SRAM_SCRATCH_BOARD_EEPROM_END (SRAM_SCRATCH_SPACE_ADDR + 0x200)
#endif
#define OMAP_SRAM_SCRATCH_SPACE_END	(TI_SRAM_SCRATCH_BOARD_EEPROM_END)

/* Boot parameters */
#define DEVICE_DATA_OFFSET	0x18
#define BOOT_MODE_OFFSET	0x8

#define CH_FLAGS_CHSETTINGS	(1 << 0)
#define CH_FLAGS_CHRAM		(1 << 1)
#define CH_FLAGS_CHFLASH	(1 << 2)
#define CH_FLAGS_CHMMCSD	(1 << 3)

#ifndef __ASSEMBLY__
u32 omap_sys_boot_device(void);
#endif

#endif /* _OMAP_COMMON_H_ */
