/* SPDX-License-Identifier: Intel */
/*
 * Copyright (C) 2013, Intel Corporation
 * Copyright (C) 2015 Google, Inc
 */

#ifndef __FSP_VPD_H
#define __FSP_VPD_H

struct memory_down_data {
	uint8_t enable_memory_down;
	uint8_t dram_speed;
	uint8_t dram_type;
	uint8_t dimm_0_enable;
	uint8_t dimm_1_enable;
	uint8_t dimm_width;
	uint8_t dimm_density;
	uint8_t dimm_bus_width;
	uint8_t dimm_sides;			/* Ranks Per dimm_ */
	uint8_t dimm_tcl;			/* tCL */
	/* tRP and tRCD in DRAM clk - 5:12.5ns, 6:15ns, etc. */
	uint8_t dimm_trpt_rcd;
	uint8_t dimm_twr;			/* tWR in DRAM clk  */
	uint8_t dimm_twtr;			/* tWTR in DRAM clk */
	uint8_t dimm_trrd;			/* tRRD in DRAM clk */
	uint8_t dimm_trtp;			/* tRTP in DRAM clk */
	uint8_t dimm_tfaw;			/* tFAW in DRAM clk */
};

struct __packed upd_region {
	uint64_t signature;			/* Offset 0x0000 */
	uint8_t reserved0[24];			/* Offset 0x0008 */
	uint16_t mrc_init_tseg_size;		/* Offset 0x0020 */
	uint16_t mrc_init_mmio_size;		/* Offset 0x0022 */
	uint8_t mrc_init_spd_addr1;		/* Offset 0x0024 */
	uint8_t mrc_init_spd_addr2;		/* Offset 0x0025 */
	uint8_t emmc_boot_mode;			/* Offset 0x0026 */
	uint8_t enable_sdio;			/* Offset 0x0027 */
	uint8_t enable_sdcard;			/* Offset 0x0028 */
	uint8_t enable_hsuart0;			/* Offset 0x0029 */
	uint8_t enable_hsuart1;			/* Offset 0x002a */
	uint8_t enable_spi;			/* Offset 0x002b */
	uint8_t reserved1;			/* Offset 0x002c */
	uint8_t enable_sata;			/* Offset 0x002d */
	uint8_t sata_mode;			/* Offset 0x002e */
	uint8_t enable_azalia;			/* Offset 0x002f */
	struct azalia_config *azalia_cfg_ptr;	/* Offset 0x0030 */
	uint8_t enable_xhci;			/* Offset 0x0034 */
	uint8_t lpe_mode;			/* Offset 0x0035 */
	uint8_t lpss_sio_mode;			/* Offset 0x0036 */
	uint8_t enable_dma0;			/* Offset 0x0037 */
	uint8_t enable_dma1;			/* Offset 0x0038 */
	uint8_t enable_i2_c0;			/* Offset 0x0039 */
	uint8_t enable_i2_c1;			/* Offset 0x003a */
	uint8_t enable_i2_c2;			/* Offset 0x003b */
	uint8_t enable_i2_c3;			/* Offset 0x003c */
	uint8_t enable_i2_c4;			/* Offset 0x003d */
	uint8_t enable_i2_c5;			/* Offset 0x003e */
	uint8_t enable_i2_c6;			/* Offset 0x003f */
	uint8_t enable_pwm0;			/* Offset 0x0040 */
	uint8_t enable_pwm1;			/* Offset 0x0041 */
	uint8_t enable_hsi;			/* Offset 0x0042 */
	uint8_t igd_dvmt50_pre_alloc;		/* Offset 0x0043 */
	uint8_t aperture_size;			/* Offset 0x0044 */
	uint8_t gtt_size;			/* Offset 0x0045 */
	uint8_t reserved2[5];			/* Offset 0x0046 */
	uint8_t mrc_debug_msg;			/* Offset 0x004b */
	uint8_t isp_enable;			/* Offset 0x004c */
	uint8_t scc_mode;			/* Offset 0x004d */
	uint8_t igd_render_standby;		/* Offset 0x004e */
	uint8_t txe_uma_enable;			/* Offset 0x004f */
	uint8_t os_selection;			/* Offset 0x0050 */
	uint8_t emmc45_ddr50_enabled;		/* Offset 0x0051 */
	uint8_t emmc45_hs200_enabled;		/* Offset 0x0052 */
	uint8_t emmc45_retune_timer_value;	/* Offset 0x0053 */
	uint8_t enable_igd;			/* Offset 0x0054 */
	uint8_t unused_upd_space1[155];		/* Offset 0x0055 */
	struct memory_down_data memory_params;	/* Offset 0x00f0 */
	uint16_t terminator;			/* Offset 0x0100 */
};

#define VPD_IMAGE_ID		0x3157454956594C56	/* 'VLYVIEW1' */

struct __packed vpd_region {
	uint64_t sign;				/* Offset 0x0000 */
	uint32_t img_rev;			/* Offset 0x0008 */
	uint32_t upd_offset;			/* Offset 0x000c */
	uint8_t unused[16];			/* Offset 0x0010 */
	uint32_t fsp_res_memlen;		/* Offset 0x0020 */
	uint8_t platform_type;			/* Offset 0x0024 */
	uint8_t enable_secure_boot;		/* Offset 0x0025 */
};
#endif
