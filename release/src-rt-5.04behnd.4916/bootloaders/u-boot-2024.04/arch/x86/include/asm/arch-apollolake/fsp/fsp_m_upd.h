/* SPDX-License-Identifier: Intel */
/*
 * Copyright (c) 2019, Intel Corporation. All rights reserved.
 * Copyright 2019 Google LLC
 */

#ifndef	__ASM_ARCH_FSP_M_UDP_H
#define	__ASM_ARCH_FSP_M_UDP_H

#ifndef __ASSEMBLY__
#include <asm/fsp2/fsp_api.h>

#define FSP_DRAM_CHANNELS	4

struct __packed fspm_arch_upd {
	u8	revision;
	u8	reserved[3];
	void	*nvs_buffer_ptr;
	void	*stack_base;
	u32	stack_size;
	u32	boot_loader_tolum_size;
	u32	boot_mode;
	u8	reserved1[8];
};

struct __packed fsp_ram_channel {
	u8	rank_enable;
	u8	device_width;
	u8	dram_density;
	u8	option;
	u8	odt_config;
	u8	tristate_clk1;
	u8	mode2_n;
	u8	odt_levels;
};

/**
 * struct fsp_m_config - FSP-M configuration
 *
 * Note that headers precede this and are 64 bytes long. The hex offsets
 * mentioned in this file are relative to the start of the header, the same
 * convention used in Intel's APL FSP header file.
 */
struct __packed fsp_m_config {
	/* 0x40 */
	u32	serial_debug_port_address;
	u8	serial_debug_port_type;
	u8	serial_debug_port_device;
	u8	serial_debug_port_stride_size;
	u8	mrc_fast_boot;
	u8	igd;
	u8	igd_dvmt50_pre_alloc;
	u8	igd_aperture_size;
	u8	gtt_size;
	u8	primary_video_adaptor;
	u8	package;
	u8	profile;
	u8	memory_down;

	/* 0x50 */
	u8	ddr3_l_page_size;
	u8	ddr3_lasr;
	u8	scrambler_support;
	u8	interleaved_mode;
	u16	channel_hash_mask;
	u16	slice_hash_mask;
	u8	channels_slices_enable;
	u8	min_ref_rate2x_enable;
	u8	dual_rank_support_enable;
	u8	rmt_mode;
	u16	memory_size_limit;
	u16	low_memory_max_value;

	/* 0x60 */
	u16	high_memory_max_value;
	u8	disable_fast_boot;
	u8	dimm0_spd_address;
	u8	dimm1_spd_address;
	struct fsp_ram_channel chan[FSP_DRAM_CHANNELS];
	u8	rmt_check_run;
	u16	rmt_margin_check_scale_high_threshold;
	u8	ch_bit_swizzling[FSP_DRAM_CHANNELS][32];
	u32	msg_level_mask;
	u8	unused_upd_space0[4];

	/* 0x110 */
	u8	pre_mem_gpio_table_pin_num[4];
	u32	pre_mem_gpio_table_ptr;
	u8	pre_mem_gpio_table_entry_num;
	u8	enhance_port8xh_decoding;
	u8	spd_write_enable;
	u8	mrc_data_saving;
	u32	oem_loading_base;

	/* 0x120 */
	u8	oem_file_name[16];

	/* 0x130 */
	void	*mrc_boot_data_ptr;
	u8	e_mmc_trace_len;
	u8	skip_cse_rbp;
	u8	npk_en;
	u8	fw_trace_en;
	u8	fw_trace_destination;
	u8	recover_dump;
	u8	msc0_wrap;
	u8	msc1_wrap;
	u32	msc0_size;

	/* 0x140 */
	u32	msc1_size;
	u8	pti_mode;
	u8	pti_training;
	u8	pti_speed;
	u8	punit_mlvl;
	u8	pmc_mlvl;
	u8	sw_trace_en;
	u8	periodic_retraining_disable;
	u8	enable_reset_system;
	u8	enable_s3_heci2;
	u8	unused_upd_space1[3];

	/* 0x150 */
	void	*variable_nvs_buffer_ptr;
	u64	start_timer_ticker_of_pfet_assert;
	u8	rt_en;
	u8	skip_pcie_power_sequence;
	u8	reserved_fspm_upd[2];
};

/** FSP-M UPD Configuration */
struct __packed fspm_upd {
	struct fsp_upd_header header;
	struct fspm_arch_upd arch;
	struct fsp_m_config config;
	u8 unused_upd_space2[158];
	u16 upd_terminator;
};
#endif

#define SERIAL_DEBUG_PORT_TYPE_NONE 0
#define SERIAL_DEBUG_PORT_TYPE_IO 1
#define SERIAL_DEBUG_PORT_TYPE_MMIO 2

#define SERIAL_DEBUG_PORT_DEVICE_UART0 0
#define SERIAL_DEBUG_PORT_DEVICE_UART1 1
#define SERIAL_DEBUG_PORT_DEVICE_UART2 2
#define SERIAL_DEBUG_PORT_DEVICE_EXTERNAL 3

#define SERIAL_DEBUG_PORT_STRIDE_SIZE_1 0
#define SERIAL_DEBUG_PORT_STRIDE_SIZE_4 2

#define IGD_DVMT_50_PRE_ALLOC_64M 0x02
#define IGD_DVMT_50_PRE_ALLOC_96M 0x03
#define IGD_DVMT_50_PRE_ALLOC_128M 0x04
#define IGD_DVMT_50_PRE_ALLOC_160M 0x05
#define IGD_DVMT_50_PRE_ALLOC_192M 0x06
#define IGD_DVMT_50_PRE_ALLOC_224M 0x07
#define IGD_DVMT_50_PRE_ALLOC_256M 0x08
#define IGD_DVMT_50_PRE_ALLOC_288M 0x09
#define IGD_DVMT_50_PRE_ALLOC_320M 0x0a
#define IGD_DVMT_50_PRE_ALLOC_352M 0x0b
#define IGD_DVMT_50_PRE_ALLOC_384M 0x0c
#define IGD_DVMT_50_PRE_ALLOC_416M 0x0d
#define IGD_DVMT_50_PRE_ALLOC_448M 0x0e
#define IGD_DVMT_50_PRE_ALLOC_480M 0x0f
#define IGD_DVMT_50_PRE_ALLOC_512M 0x10

#define IGD_APERTURE_SIZE_128M 0x1
#define IGD_APERTURE_SIZE_256M 0x2
#define IGD_APERTURE_SIZE_512M 0x3

#define GTT_SIZE_2M 1
#define GTT_SIZE_4M 2
#define GTT_SIZE_8M 3

#define PRIMARY_VIDEO_ADAPTER_AUTO 0
#define PRIMARY_VIDEO_ADAPTER_IGD 2
#define PRIMARY_VIDEO_ADAPTER_PCI 3

#define PACKAGE_SODIMM 0
#define PACKAGE_BGA 1
#define PACKAGE_BGA_MIRRORED 2
#define PACKAGE_SODIMM_UDIMM_RANK_MIRRORED 3

#define PROFILE_WIO2_800_7_8_8 0x1
#define PROFILE_WIO2_1066_9_10_10 0x2
#define PROFILE_LPDDR3_1066_8_10_10 0x3
#define PROFILE_LPDDR3_1333_10_12_12 0x4
#define PROFILE_LPDDR3_1600_12_15_15 0x5
#define PROFILE_LPDDR3_1866_14_17_17 0x6
#define PROFILE_LPDDR3_2133_16_20_20 0x7
#define PROFILE_LPDDR4_1066_10_10_10 0x8
#define PROFILE_LPDDR4_1600_14_15_15 0x9
#define PROFILE_LPDDR4_2133_20_20_20 0xa
#define PROFILE_LPDDR4_2400_24_22_22 0xb
#define PROFILE_LPDDR4_2666_24_24_24 0xc
#define PROFILE_LPDDR4_2933_28_27_27 0xd
#define PROFILE_LPDDR4_3200_28_29_29 0xe
#define PROFILE_DDR3_1066_6_6_6 0xf
#define PROFILE_DDR3_1066_7_7_7 0x10
#define PROFILE_DDR3_1066_8_8_8 0x11
#define PROFILE_DDR3_1333_7_7_7 0x12
#define PROFILE_DDR3_1333_8_8_8 0x13
#define PROFILE_DDR3_1333_9_9_9 0x14
#define PROFILE_DDR3_1333_10_10_10 0x15
#define PROFILE_DDR3_1600_8_8_8 0x16
#define PROFILE_DDR3_1600_9_9_9 0x17
#define PROFILE_DDR3_1600_10_10_10 0x18
#define PROFILE_DDR3_1600_11_11_11 0x19
#define PROFILE_DDR3_1866_10_10_10 0x1a
#define PROFILE_DDR3_1866_11_11_11 0x1b
#define PROFILE_DDR3_1866_12_12_12 0x1c
#define PROFILE_DDR3_1866_13_13_13 0x1d
#define PROFILE_DDR3_2133_11_11_11 0x1e
#define PROFILE_DDR3_2133_12_12_12 0x1f
#define PROFILE_DDR3_2133_13_13_13 0x20
#define PROFILE_DDR3_2133_14_14_14 0x21
#define PROFILE_DDR4_1333_10_10_10 0x22
#define PROFILE_DDR4_1600_10_10_10 0x23
#define PROFILE_DDR4_1600_11_11_11 0x24
#define PROFILE_DDR4_1600_12_12_12 0x25
#define PROFILE_DDR4_1866_12_12_12 0x26
#define PROFILE_DDR4_1866_13_13_13 0x27
#define PROFILE_DDR4_1866_14_14_14 0x28
#define PROFILE_DDR4_2133_14_14_14 0x29
#define PROFILE_DDR4_2133_15_15_15 0x2a
#define PROFILE_DDR4_2133_16_16_16 0x2b
#define PROFILE_DDR4_2400_15_15_15 0x2c
#define PROFILE_DDR4_2400_16_16_16 0x2d
#define PROFILE_DDR4_2400_17_17_17 0x2e
#define PROFILE_DDR4_2400_18_18_18 0x2f

#define MEMORY_DOWN_NO 0
#define MEMORY_DOWN_YES 1
#define MEMORY_DOWN_MD_SODIMM 2
#define MEMORY_DOWN_LPDDR4 3

#define DDR3L_PAGE_SIZE_1KB 1
#define DDR3L_PAGE_SIZE_2KB 2

#define INTERLEAVED_MODE_DISABLE 0
#define INTERLEAVED_MODE_ENABLE 2

#define RMT_MODE_DISABLE 0
#define RMT_MODE_ENABLE 3

#define CHX_DEVICE_WIDTH_X8 0
#define CHX_DEVICE_WIDTH_X16 1
#define CHX_DEVICE_WIDTH_X32 2
#define CHX_DEVICE_WIDTH_X64 3

#define CHX_DEVICE_DENSITY_4GB 0
#define CHX_DEVICE_DENSITY_6GB 1
#define CHX_DEVICE_DENSITY_8GB 2
#define CHX_DEVICE_DENSITY_12GB 3
#define CHX_DEVICE_DENSITY_16GB 4
#define CHX_DEVICE_DENSITY_2GB 5

#define CHX_OPTION_RANK_INTERLEAVING 0x1
#define CHX_OPTION_BANK_ADDRESS_HASHING_ENABLE 0x2
#define CHX_OPTION_CH1_CLK_DISABLE 0x4
#define CHX_OPTION_ADDRESS_MAP_2KB 0x10

#define CHX_ODT_CONFIG_DDR3_RX_ODT 0x1
#define CHX_ODT_CONFIG_DDR4_CA_ODT 0x2
#define CHX_ODT_CONFIG_DDR3L_TX_ODT 0x10

#define CHX_MODE2N_AUTO 0
#define CHX_MODE2N_FORCE 1

#define CHX_ODT_LEVELS_CONNECTED_TO_SOC 0x0
#define CHX_ODT_LEVELS_HELD_HIGH 0x1

#define NPK_EN_DISABLE 0
#define NPK_EN_ENABLE 1
#define NPK_EN_DEBUGGER 2
#define NPK_EN_AUTO 3

#define FW_TRACE_DESTINATION_NPK_TRACE_TO_MEMORY 1
#define FW_TRACE_DESTINATION_NPK_TRACE_TO_DCI 2
#define FW_TRACE_DESTINATION_NPK_NPK_TRACE_TO_BSSB 3
#define FW_TRACE_DESTINATION_NPK_TRACE_TO_PTI 4

#define MSC_X_WRAP_0 0
#define MSC_X_WRAP_1 1

#define MSC_X_SIZE_0M 0
#define MSC_X_SIZE_1M 1
#define MSC_X_SIZE_8M 2
#define MSC_X_SIZE_64M 3
#define MSC_X_SIZE_128M 4
#define MSC_X_SIZE_256M 5
#define MSC_X_SIZE_512M 6
#define MSC_X_SIZE_1GB 7

#define PTI_MODE_0 0
#define PTI_MODE_x4 1
#define PTI_MODE_x8 2
#define PTI_MODE_x12 3
#define PTI_MODE_x16 4

#define PTI_SPEED_FULL 0
#define PTI_SPEED_HALF 1
#define PTI_SPEED_QUARTER 2

#endif
