/* SPDX-License-Identifier: Intel */
/*
 * Copyright (c) 2016, Intel Corporation. All rights reserved.
 * Copyright 2019 Google LLC
 */
#ifndef __ASM_ARCH_FSP_S_UDP_H
#define __ASM_ARCH_FSP_S_UDP_H

#ifndef __ASSEMBLY__
#include <asm/fsp2/fsp_api.h>

/**
 * struct fsp_s_config - FSP-S configuration
 *
 * Note that struct fsp_upd_header preceeds this and is 32 bytes long. The
 * hex offsets mentioned in this file are relative to the start of the header,
 * the same convention used in Intel's APL FSP header file.
 */
struct __packed fsp_s_config {
	/* 0x20 */
	u8	active_processor_cores;
	u8	disable_core1;
	u8	disable_core2;
	u8	disable_core3;
	u8	vmx_enable;
	u8	proc_trace_mem_size;
	u8	proc_trace_enable;
	u8	eist;
	u8	boot_p_state;
	u8	enable_cx;
	u8	c1e;
	u8	bi_proc_hot;
	u8	pkg_c_state_limit;
	u8	c_state_auto_demotion;
	u8	c_state_un_demotion;
	u8	max_core_c_state;

	/* 0x30 */
	u8	pkg_c_state_demotion;
	u8	pkg_c_state_un_demotion;
	u8	turbo_mode;
	u8	hda_verb_table_entry_num;
	u32	hda_verb_table_ptr;
	u8	p2sb_unhide;
	u8	ipu_en;
	u8	ipu_acpi_mode;
	u8	force_wake;
	u32	gtt_mm_adr;

	/* 0x40 */
	u32	gm_adr;
	u8	pavp_lock;
	u8	graphics_freq_modify;
	u8	graphics_freq_req;
	u8	graphics_video_freq;
	u8	pm_lock;
	u8	dop_clock_gating;
	u8	unsolicited_attack_override;
	u8	wopcm_support;
	u8	wopcm_size;
	u8	power_gating;
	u8	unit_level_clock_gating;
	u8	fast_boot;

	/* 0x50 */
	u8	dyn_sr;
	u8	sa_ipu_enable;
	u8	pm_support;
	u8	enable_render_standby;
	u32	logo_size;
	u32	logo_ptr;
	u32	graphics_config_ptr;

	/* 0x60 */
	u8	pavp_enable;
	u8	pavp_pr3;
	u8	cd_clock;
	u8	pei_graphics_peim_init;
	u8	write_protection_enable[5];
	u8	read_protection_enable[5];
	u16	protected_range_limit[5];
	u16	protected_range_base[5];
	u8	gmm;
	u8	clk_gating_pgcb_clk_trunk;
	u8	clk_gating_sb;
	u8	clk_gating_sb_clk_trunk;
	u8	clk_gating_sb_clk_partition;
	u8	clk_gating_core;
	u8	clk_gating_dma;
	u8	clk_gating_reg_access;
	u8	clk_gating_host;
	u8	clk_gating_partition;
	u8	clk_gating_trunk;
	u8	hda_enable;
	u8	dsp_enable;
	u8	pme;

	/* 0x90 */
	u8	hd_audio_io_buffer_ownership;
	u8	hd_audio_io_buffer_voltage;
	u8	hd_audio_vc_type;
	u8	hd_audio_link_frequency;
	u8	hd_audio_i_disp_link_frequency;
	u8	hd_audio_i_disp_link_tmode;
	u8	dsp_endpoint_dmic;
	u8	dsp_endpoint_bluetooth;
	u8	dsp_endpoint_i2s_skp;
	u8	dsp_endpoint_i2s_hp;
	u8	audio_ctl_pwr_gate;
	u8	audio_dsp_pwr_gate;
	u8	mmt;
	u8	hmt;
	u8	hd_audio_pwr_gate;
	u8	hd_audio_clk_gate;

	/* 0xa0 */
	u32	dsp_feature_mask;
	u32	dsp_pp_module_mask;
	u8	bios_cfg_lock_down;
	u8	hpet;
	u8	hpet_bdf_valid;
	u8	hpet_bus_number;
	u8	hpet_device_number;
	u8	hpet_function_number;
	u8	io_apic_bdf_valid;
	u8	io_apic_bus_number;

	/* 0xb0 */
	u8	io_apic_device_number;
	u8	io_apic_function_number;
	u8	io_apic_entry24_119;
	u8	io_apic_id;
	u8	io_apic_range_select;
	u8	ish_enable;
	u8	bios_interface;
	u8	bios_lock;
	u8	spi_eiss;
	u8	bios_lock_sw_smi_number;
	u8	lpss_s0ix_enable;
	u8	unused_upd_space0[1];
	u8	i2c_clk_gate_cfg[8];
	u8	hsuart_clk_gate_cfg[4];
	u8	spi_clk_gate_cfg[3];
	u8	i2c0_enable;
	u8	i2c1_enable;
	u8	i2c2_enable;
	u8	i2c3_enable;
	u8	i2c4_enable;

	/* 0xd0 */
	u8	i2c5_enable;
	u8	i2c6_enable;
	u8	i2c7_enable;
	u8	hsuart0_enable;
	u8	hsuart1_enable;
	u8	hsuart2_enable;
	u8	hsuart3_enable;
	u8	spi0_enable;
	u8	spi1_enable;
	u8	spi2_enable;
	u8	os_dbg_enable;
	u8	dci_en;
	u32	uart2_kernel_debug_base_address;

	/* 0xe0 */
	u8	pcie_clock_gating_disabled;
	u8	pcie_root_port8xh_decode;
	u8	pcie8xh_decode_port_index;
	u8	pcie_root_port_peer_memory_write_enable;
	u8	pcie_aspm_sw_smi_number;
	u8	unused_upd_space1[1];
	u8	pcie_root_port_en[6];
	u8	pcie_rp_hide[6];
	u8	pcie_rp_slot_implemented[6];
	u8	pcie_rp_hot_plug[6];
	u8	pcie_rp_pm_sci[6];
	u8	pcie_rp_ext_sync[6];
	u8	pcie_rp_transmitter_half_swing[6];

	/* 0x110 */
	u8	pcie_rp_acs_enabled[6];
	u8	pcie_rp_clk_req_supported[6];
	u8	pcie_rp_clk_req_number[6];
	u8	pcie_rp_clk_req_detect[6];
	u8	advanced_error_reporting[6];
	u8	pme_interrupt[6];
	u8	unsupported_request_report[6];
	u8	fatal_error_report[6];

	/* 0x140 */
	u8	no_fatal_error_report[6];
	u8	correctable_error_report[6];
	u8	system_error_on_fatal_error[6];
	u8	system_error_on_non_fatal_error[6];
	u8	system_error_on_correctable_error[6];
	u8	pcie_rp_speed[6];
	u8	physical_slot_number[6];
	u8	pcie_rp_completion_timeout[6];

	/* 0x170 */
	u8	ptm_enable[6];
	u8	pcie_rp_aspm[6];
	u8	pcie_rp_l1_substates[6];
	u8	pcie_rp_ltr_enable[6];
	u8	pcie_rp_ltr_config_lock[6];
	u8	pme_b0_s5_dis;
	u8	pci_clock_run;

	/* 0x190 */
	u8	timer8254_clk_setting;
	u8	enable_sata;
	u8	sata_mode;
	u8	sata_salp_support;
	u8	sata_pwr_opt_enable;
	u8	e_sata_speed_limit;
	u8	speed_limit;
	u8	unused_upd_space2[1];
	u8	sata_ports_enable[2];
	u8	sata_ports_dev_slp[2];
	u8	sata_ports_hot_plug[2];
	u8	sata_ports_interlock_sw[2];

	/* 0x1a0 */
	u8	sata_ports_external[2];
	u8	sata_ports_spin_up[2];
	u8	sata_ports_solid_state_drive[2];
	u8	sata_ports_enable_dito_config[2];
	u8	sata_ports_dm_val[2];
	u8	unused_upd_space3[2];
	u16	sata_ports_dito_val[2];

	/* 0x1b0 */
	u16	sub_system_vendor_id;
	u16	sub_system_id;
	u8	crid_settings;
	u8	reset_select;
	u8	sdcard_enabled;
	u8	e_mmc_enabled;
	u8	e_mmc_host_max_speed;
	u8	ufs_enabled;
	u8	sdio_enabled;
	u8	gpp_lock;
	u8	sirq_enable;
	u8	sirq_mode;
	u8	start_frame_pulse;
	u8	smbus_enable;

	/* 0x1c0 */
	u8	arp_enable;
	u8	unused_upd_space4;
	u16	num_rsvd_smbus_addresses;
	u8	rsvd_smbus_address_table[128];
	u8	disable_compliance_mode;
	u8	usb_per_port_ctl;
	u8	usb30_mode;
	u8	unused_upd_space5[1];
	u8	port_usb20_enable[8];

	/* 0x250 */
	u8	port_us20b_over_current_pin[8];
	u8	usb_otg;
	u8	hsic_support_enable;
	u8	port_usb30_enable[6];

	/* 0x260 */
	u8	port_us30b_over_current_pin[6];
	u8	ssic_port_enable[2];
	u16	dlane_pwr_gating;
	u8	vtd_enable;
	u8	lock_down_global_smi;
	u16	reset_wait_timer;
	u8	rtc_lock;
	u8	sata_test_mode;

	/* 0x270 */
	u8	ssic_rate[2];
	u16	dynamic_power_gating;
	u16	pcie_rp_ltr_max_snoop_latency[6];

	/* 0x280 */
	u8	pcie_rp_snoop_latency_override_mode[6];
	u8	unused_upd_space6[2];
	u16	pcie_rp_snoop_latency_override_value[6];
	u8	pcie_rp_snoop_latency_override_multiplier[6];
	u8	skip_mp_init;
	u8	dci_auto_detect;
	u16	pcie_rp_ltr_max_non_snoop_latency[6];
	u8	pcie_rp_non_snoop_latency_override_mode[6];
	u8	tco_timer_halt_lock;
	u8	pwr_btn_override_period;

	/* 0x2b0 */
	u16	pcie_rp_non_snoop_latency_override_value[6];
	u8	pcie_rp_non_snoop_latency_override_multiplier[6];
	u8	pcie_rp_slot_power_limit_scale[6];
	u8	pcie_rp_slot_power_limit_value[6];
	u8	disable_native_power_button;
	u8	power_butter_debounce_mode;

	/* 0x2d0 */
	u32	sdio_tx_cmd_cntl;
	u32	sdio_tx_data_cntl1;
	u32	sdio_tx_data_cntl2;
	u32	sdio_rx_cmd_data_cntl1;

	/* 0x2e0 */
	u32	sdio_rx_cmd_data_cntl2;
	u32	sdcard_tx_cmd_cntl;
	u32	sdcard_tx_data_cntl1;
	u32	sdcard_tx_data_cntl2;

	/* 0x2f0 */
	u32	sdcard_rx_cmd_data_cntl1;
	u32	sdcard_rx_strobe_cntl;
	u32	sdcard_rx_cmd_data_cntl2;
	u32	emmc_tx_cmd_cntl;

	/* 0x300 */
	u32	emmc_tx_data_cntl1;
	u32	emmc_tx_data_cntl2;
	u32	emmc_rx_cmd_data_cntl1;
	u32	emmc_rx_strobe_cntl;

	/* 0x310 */
	u32	emmc_rx_cmd_data_cntl2;
	u32	emmc_master_sw_cntl;
	u8	pcie_rp_selectable_deemphasis[6];
	u8	monitor_mwait_enable;
	u8	hd_audio_dsp_uaa_compliance;

	/* 0x320 */
	u32	ipc[4];

	/* 0x330 */
	u8	sata_ports_disable_dynamic_pg[2];
	u8	init_s3_cpu;
	u8	skip_punit_init;
	u8	unused_upd_space7[4];
	u8	port_usb20_per_port_tx_pe_half[8];

	/* 0x340 */
	u8	port_usb20_per_port_pe_txi_set[8];
	u8	port_usb20_per_port_txi_set[8];

	/* 0x350 */
	u8	port_usb20_hs_skew_sel[8];
	u8	port_usb20_i_usb_tx_emphasis_en[8];

	/* 0x360 */
	u8	port_usb20_per_port_rxi_set[8];
	u8	port_usb20_hs_npre_drv_sel[8];

	/* 0x370 */
	u8	os_selection;
	u8	dptf_enabled;
	u8	pwm_enabled;
	u8	reserved_fsps_upd[13];
};

/** struct fsps_upd - FSP-S Configuration */
struct __packed fsps_upd {
	struct fsp_upd_header header;
	struct fsp_s_config config;
	u8 unused_upd_space2[46];
	u16 upd_terminator;
};
#endif

#define PROC_TRACE_MEM_SIZE_DISABLE 0xff

#define BOOT_P_STATE_HFM 0
#define BOOT_P_STATE_LFM 1

#define PKG_C_STATE_LIMIT_C0_C1 0
#define PKG_C_STATE_LIMIT_C2 1
#define PKG_C_STATE_LIMIT_C3 2
#define PKG_C_STATE_LIMIT_C6 3
#define PKG_C_STATE_LIMIT_C7 4
#define PKG_C_STATE_LIMIT_C7S 5
#define PKG_C_STATE_LIMIT_C8 6
#define PKG_C_STATE_LIMIT_C9 7
#define PKG_C_STATE_LIMIT_C10 8
#define PKG_C_STATE_LIMIT_CMAX 9
#define PKG_C_STATE_LIMIT_CPU_DEFAULT 254
#define PKG_C_STATE_LIMIT_AUTO 255

#define C_STATE_AUTO_DEMOTION_DISABLE_C1_C3 0
#define C_STATE_AUTO_DEMOTION_ENABLE_C3_C6_C7_TO_C1 1
#define C_STATE_AUTO_DEMOTION_ENABLE_C6_C7_TO_C3 2
#define C_STATE_AUTO_DEMOTION_ENABLE_C6_C7_TO_C1_C3 3

#define C_STATE_UN_DEMOTION_DISABLE_C1_C3 0
#define C_STATE_UN_DEMOTION_ENABLE_C1 1
#define C_STATE_UN_DEMOTION_ENABLE_C3 2
#define C_STATE_UN_DEMOTION_ENABLE_C1_C3 3

#define MAX_CORE_C_STATE_UNLIMITED 0
#define MAX_CORE_C_STATE_C1 1
#define MAX_CORE_C_STATE_C3 2
#define MAX_CORE_C_STATE_C6 3
#define MAX_CORE_C_STATE_C7 4
#define MAX_CORE_C_STATE_C8 5
#define MAX_CORE_C_STATE_C9 6
#define MAX_CORE_C_STATE_C10 7
#define MAX_CORE_C_STATE_CCX 8

#define IPU_ACPI_MODE_DISABLE 0
#define IPU_ACPI_MODE_IGFX_CHILD_DEVICE 1
#define IPU_ACPI_MODE_ACPI_DEVICE 1

#define CD_CLOCK_FREQ_144MHZ 0
#define CD_CLOCK_FREQ_288MHZ 1
#define CD_CLOCK_FREQ_384MHZ 2
#define CD_CLOCK_FREQ_576MHZ 3
#define CD_CLOCK_FREQ_624MHZ 4

#define HDA_IO_BUFFER_OWNERSHIP_HDA_ALL_IO 0
#define HDA_IO_BUFFER_OWNERSHIP_HDA_I2S_SPLIT 1
#define HDA_IO_BUFFER_OWNERSHIP_I2S_ALL_IO 2

#define HDA_IO_BUFFER_VOLTAGE_3V3 0
#define HDA_IO_BUFFER_VOLTAGE_1V8 1

#define HDA_VC_TYPE_VC0 0
#define HDA_VC_TYPE_VC1 1

#define HDA_LINK_FREQ_6MHZ 0
#define HDA_LINK_FREQ_12MHZ 1
#define HDA_LINK_FREQ_24MHZ 2
#define HDA_LINK_FREQ_48MHZ 3
#define HDA_LINK_FREQ_96MHZ 4
#define HDA_LINK_FREQ_INVALID 5

#define HDA_I_DISP_LINK_FREQ_6MHZ 0
#define HDA_I_DISP_LINK_FREQ_12MHZ 1
#define HDA_I_DISP_LINK_FREQ_24MHZ 2
#define HDA_I_DISP_LINK_FREQ_48MHZ 3
#define HDA_I_DISP_LINK_FREQ_96MHZ 4
#define HDA_I_DISP_LINK_FREQ_INVALID 5

#define HDA_I_DISP_LINK_T_MODE_2T 0
#define HDA_I_DISP_LINK_T_MODE_1T 1

#define HDA_DISP_DMIC_DISABLE 0
#define HDA_DISP_DMIC_2CH_ARRAY 1
#define HDA_DISP_DMIC_4CH_ARRAY 2

#define HDA_CSE_MEM_TRANSFERS_VC0 0
#define HDA_CSE_MEM_TRANSFERS_VC2 1

#define HDA_HOST_MEM_TRANSFERS_VC0 0
#define HDA_HOST_MEM_TRANSFERS_VC2 1

#define HDA_DSP_FEATURE_MASK_WOV 0x1
#define HDA_DSP_FEATURE_MASK_BT_SIDEBAND 0x2
#define HDA_DSP_FEATURE_MASK_CODEC_VAD 0x4
#define HDA_DSP_FEATURE_MASK_BT_INTEL_HFP 0x20
#define HDA_DSP_FEATURE_MASK_BT_INTEL_A2DP 0x40
#define HDA_DSP_FEATURE_MASK_DSP_BASED_PRE_PROC_DISABLE 0x80

#define HDA_DSP_PP_MODULE_MASK_WOV 0x1
#define HDA_DSP_PP_MODULE_MASK_BT_SIDEBAND 0x2
#define HDA_DSP_PP_MODULE_MASK_CODEC_VAD 0x4
#define HDA_DSP_PP_MODULE_MASK_BT_INTEL_HFP 0x20
#define HDA_DSP_PP_MODULE_MASK_BT_INTEL_A2DP 0x40
#define HDA_DSP_PP_MODULE_MASK_DSP_BASED_PRE_PROC_DISABLE 0x80

#define I2CX_ENABLE_DISABLED 0
#define I2CX_ENABLE_PCI_MODE 1
#define I2CX_ENABLE_ACPI_MODE 2

#define HSUARTX_ENABLE_DISABLED 0
#define HSUARTX_ENABLE_PCI_MODE 1
#define HSUARTX_ENABLE_ACPI_MODE 2

#define SPIX_ENABLE_DISABLED 0
#define SPIX_ENABLE_PCI_MODE 1
#define SPIX_ENABLE_ACPI_MODE 2

#define PCIE_RP_SPEED_AUTO 0
#define PCIE_RP_SPEED_GEN1 1
#define PCIE_RP_SPEED_GEN2 2
#define PCIE_RP_SPEED_GEN3 3

#define PCIE_RP_ASPM_DISABLE 0
#define PCIE_RP_ASPM_L0S 1
#define PCIE_RP_ASPM_L1 2
#define PCIE_RP_ASPM_L0S_L1 3
#define PCIE_RP_ASPM_AUTO 4

#define PCIE_RP_L1_SUBSTATES_DISABLE 0
#define PCIE_RP_L1_SUBSTATES_L1_1 1
#define PCIE_RP_L1_SUBSTATES_L1_2 2
#define PCIE_RP_L1_SUBSTATES_L1_1_L1_2 3

#define SATA_MODE_AHCI 0
#define SATA_MODE_RAID 1

#define SATA_SPEED_LIMIT_SC_SATA_SPEED 0
#define SATA_SPEED_LIMIT_1_5GBS 1
#define SATA_SPEED_LIMIT_3GBS 2
#define SATA_SPEED_LIMIT_6GBS 3

#define SATA_PORT_SOLID_STATE_DRIVE_HARD_DISK_DRIVE 0
#define SATA_PORT_SOLID_STATE_DRIVE_SOLID_STATE_DRIVE 1

#define CRID_SETTING_DISABLE 0
#define CRID_SETTING_CRID_1 1
#define CRID_SETTING_CRID_2 2
#define CRID_SETTING_CRID_3 3

#define RESET_SELECT_WARM_RESET 0x6
#define RESET_SELECT_COLD_RESET 0xe

#define EMMC_HOST_SPEED_MAX_HS400 0
#define EMMC_HOST_SPEED_MAX_HS200 1
#define EMMC_HOST_SPEED_MAX_DDR50 2

#define SERIAL_IRQ_MODE_QUIET_MODE 0
#define SERIAL_IRQ_MODE_CONTINUOUS_MODE 1

#define START_FRAME_PULSE_WIDTH_SCSFPW4CLK 0
#define START_FRAME_PULSE_WIDTH_SCSFPW6CLK 1
#define START_FRAME_PULSE_WIDTH_SCSFPW8CLK 1

#define USB30_MODE_DISABLE 0
#define USB30_MODE_ENABLE 1
#define USB30_MODE_AUTO 2

#define USB_OTG_DISABLE 0
#define USB_OTG_PCI_MODE 1
#define USB_OTG_ACPI_MODE 2

#define SSIC_RATE_A_SERIES 1
#define SSIC_RATE_B_SERIES 2

#define PCIE_RP_SNOOP_LATENCY_OVERRIDE_MODE_DISABLE 0
#define PCIE_RP_SNOOP_LATENCY_OVERRIDE_MODE_ENABLE 1
#define PCIE_RP_SNOOP_LATENCY_OVERRIDE_MODE_AUTO 2

#define PCIE_RP_SNOOP_LATENCY_OVERRIDE_MULTIPLIER_1NS 0
#define PCIE_RP_SNOOP_LATENCY_OVERRIDE_MULTIPLIER_32NS 1
#define PCIE_RP_SNOOP_LATENCY_OVERRIDE_MULTIPLIER_1024NS 2
#define PCIE_RP_SNOOP_LATENCY_OVERRIDE_MULTIPLIER_32768NS 3
#define PCIE_RP_SNOOP_LATENCY_OVERRIDE_MULTIPLIER_1048576NS 4
#define PCIE_RP_SNOOP_LATENCY_OVERRIDE_MULTIPLIER_33554432NS 5

#define PCIE_RP_NON_SNOOP_LATENCY_OVERRIDE_MODE_DISABLE 0
#define PCIE_RP_NON_SNOOP_LATENCY_OVERRIDE_MODE_ENABLE 1
#define PCIE_RP_NON_SNOOP_LATENCY_OVERRIDE_MODE_AUTO 2

#define PWR_BTN_OVERRIDE_PERIOD_4S 0
#define PWR_BTN_OVERRIDE_PERIOD_6S 1
#define PWR_BTN_OVERRIDE_PERIOD_8S 2
#define PWR_BTN_OVERRIDE_PERIOD_10S 3
#define PWR_BTN_OVERRIDE_PERIOD_12S 4
#define PWR_BTN_OVERRIDE_PERIOD_14S 5

#define PCIE_RP_NON_SNOOP_LATENCY_OVERRIDE_MULTIPLIER_1NS 0
#define PCIE_RP_NON_SNOOP_LATENCY_OVERRIDE_MULTIPLIER_32NS 1
#define PCIE_RP_NON_SNOOP_LATENCY_OVERRIDE_MULTIPLIER_1024NS 2
#define PCIE_RP_NON_SNOOP_LATENCY_OVERRIDE_MULTIPLIER_32768NS 3
#define PCIE_RP_NON_SNOOP_LATENCY_OVERRIDE_MULTIPLIER_1048576NS 4
#define PCIE_RP_NON_SNOOP_LATENCY_OVERRIDE_MULTIPLIER_33554432NS 5

#define PCIE_RP_SELECTABLE_DEEMPHASIS_6_DB 0
#define PCIE_RP_SELECTABLE_DEEMPHASIS_3_5_DB 1

#define OS_SELECTION_WINDOWS 0
#define OS_SELECTION_ANDROID 1
#define OS_SELECTION_LINUX 3

#endif
