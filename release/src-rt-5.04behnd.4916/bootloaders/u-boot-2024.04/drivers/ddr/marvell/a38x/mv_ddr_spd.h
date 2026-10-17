/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#ifndef _MV_DDR_SPD_H
#define _MV_DDR_SPD_H

#include "mv_ddr_topology.h"

/*
 * Based on JEDEC Standard No. 21-C, 4.1.2.L-4:
 * Serial Presence Detect (SPD) for DDR4 SDRAM Modules
 */

/* block 0: base configuration and dram parameters */
#define MV_DDR_SPD_DATA_BLOCK0_SIZE		128
/* block 1: module specific parameters sub-block */
#define MV_DDR_SPD_DATA_BLOCK1M_SIZE		64
/* block 1: hybrid memory parameters sub-block */
#define MV_DDR_SPD_DATA_BLOCK1H_SIZE		64
/* block 2: extended function parameter block */
#define MV_DDR_SPD_DATA_BLOCK2E_SIZE		64
/* block 2: manufacturing information */
#define MV_DDR_SPD_DATA_BLOCK2M_SIZE		64
/* block 3: end user programmable */
#define MV_DDR_SPD_DATA_BLOCK3_SIZE		128

#define MV_DDR_SPD_DEV_TYPE_DDR4		0xc
#define MV_DDR_SPD_MODULE_TYPE_UDIMM		0x2
#define MV_DDR_SPD_MODULE_TYPE_SO_DIMM		0x3
#define MV_DDR_SPD_MODULE_TYPE_MINI_UDIMM	0x6
#define MV_DDR_SPD_MODULE_TYPE_72BIT_SO_UDIMM	0x9
#define MV_DDR_SPD_MODULE_TYPE_16BIT_SO_DIMM	0xc
#define MV_DDR_SPD_MODULE_TYPE_32BIT_SO_DIMM	0xd

/*
 * TODO: For now, the struct contains block 0 & block 1 with module specific
 * parameters for unbuffered memory module types only.
 */
union mv_ddr_spd_data {
	unsigned char all_bytes[MV_DDR_SPD_DATA_BLOCK0_SIZE +
				MV_DDR_SPD_DATA_BLOCK1M_SIZE +
				MV_DDR_SPD_DATA_BLOCK1H_SIZE +
				MV_DDR_SPD_DATA_BLOCK2E_SIZE +
				MV_DDR_SPD_DATA_BLOCK2M_SIZE];
	struct {
		/* block 0 */
		union { /* num of bytes used/num of bytes in spd device/crc coverage */
			unsigned char all_bits;
			struct {
				unsigned char spd_bytes_used:4,
					spd_bytes_total:3,
					reserved:1;
			} bit_fields;
		} byte_0;
		union { /* spd revision */
			unsigned char all_bits;
			struct {
				unsigned char addtions_level:4,
					encoding_level:4;
			} bit_fields;
		} byte_1;
		unsigned char  byte_2; /* key_byte/dram device type */
		union { /* key byte/module type */
			unsigned char all_bits;
			struct {
				unsigned char module_type:4,
					hybrid_media:3,
					hybrid:1;
			} bit_fields;
		} byte_3;
		union { /* sdram density & banks */
			unsigned char all_bits;
			struct {
				unsigned char die_capacity:4,
					bank_address:2,
					bank_group:2;
			} bit_fields;
		} byte_4;
		union { /* sdram addressing */
			unsigned char all_bits;
			struct {
				unsigned char col_address:3,
					row_address:3,
					reserved:2;
			} bit_fields;
		} byte_5;
		union { /* sdram package type */
			unsigned char all_bits;
			struct {
				unsigned char signal_loading:2,
					reserved:2,
					die_count:3,
					sdram_package_type:1;
			} bit_fields;
		} byte_6;
		union { /* sdram optional features */
			unsigned char all_bits;
			struct {
				unsigned char mac:4, /* max activate count */
					t_maw:2, /* max activate window */
					reserved:2; /* all 0s */
			} bit_fields;
		} byte_7;
		unsigned char byte_8; /* sdram thermal & refresh options; reserved; 0x00 */
		union { /* other sdram optional features */
			unsigned char all_bits;
			struct {
				unsigned char reserved:5, /* all 0s */
					soft_ppr:1,
					ppr:2; /* post package repair */
			} bit_fields;
		} byte_9;
		union { /* secondary sdram package type */
			unsigned char all_bits;
			struct {
				unsigned char signal_loading:2,
					density_ratio:2, /* dram density ratio */
					die_count:3,
					sdram_package_type:1;
			} bit_fields;
		} byte_10;
		union { /* module nominal voltage, vdd */
			unsigned char all_bits;
			struct {
				unsigned char operable:1,
					endurant:1,
					reserved:5; /* all 0s */
			} bit_fields;
		} byte_11;
		union { /* module organization*/
			unsigned char all_bits;
			struct {
				unsigned char device_width:3,
					dimm_pkg_ranks_num:3, /* package ranks per dimm number */
					rank_mix:1,
					reserved:1; /* 0 */
			} bit_fields;
		} byte_12;
		union { /* module memory bus width */
			unsigned char all_bits;
			struct {
				unsigned char primary_bus_width:3, /* in bits */
					bus_width_ext:2, /* in bits */
					reserved:3; /* all 0s */
			} bit_fields;
		} byte_13;
		union { /* module thernal sensor */
			unsigned char all_bits;
			struct {
				unsigned char reserved:7,
					thermal_sensor:1;
			} bit_fields;
		} byte_14;
		union { /* extended module type */
			unsigned char all_bits;
			struct {
				unsigned char ext_base_module_type:4,
					reserved:4; /* all 0s */
			} bit_fields;
		} byte_15;
		unsigned char byte_16; /* reserved; 0x00 */
		union { /* timebases */
			unsigned char all_bits;
			struct {
				unsigned char ftb:2, /* fine timebase */
					mtb:2, /* medium timebase */
					reserved:4; /* all 0s */
			} bit_fields;
		} byte_17;
		unsigned char byte_18; /* sdram min cycle time (t ck avg min), mtb */
		unsigned char byte_19; /* sdram max cycle time (t ck avg max), mtb */
		unsigned char byte_20; /* cas latencies supported, first byte */
		unsigned char byte_21; /* cas latencies supported, second byte */
		unsigned char byte_22; /* cas latencies supported, third byte */
		unsigned char byte_23; /* cas latencies supported, fourth byte */
		unsigned char byte_24; /* min cas latency time (t aa min), mtb */
		unsigned char byte_25; /* min ras to cas delay time (t rcd min), mtb */
		unsigned char byte_26; /* min row precharge delay time (t rp min), mtb */
		union { /* upper nibbles for t ras min & t rc min */
			unsigned char all_bits;
			struct {
				unsigned char t_ras_min_msn:4, /* t ras min most significant nibble */
					t_rc_min_msn:4; /* t rc min most significant nibble */
			} bit_fields;
		} byte_27;
		unsigned char byte_28; /* min active to precharge delay time (t ras min), l-s-byte, mtb */
		unsigned char byte_29; /* min active to active/refresh delay time (t rc min), l-s-byte, mtb */
		unsigned char byte_30; /* min refresh recovery delay time (t rfc1 min), l-s-byte, mtb */
		unsigned char byte_31; /* min refresh recovery delay time (t rfc1 min), m-s-byte, mtb */
		unsigned char byte_32; /* min refresh recovery delay time (t rfc2 min), l-s-byte, mtb */
		unsigned char byte_33; /* min refresh recovery delay time (t rfc2 min), m-s-byte, mtb */
		unsigned char byte_34; /* min refresh recovery delay time (t rfc4 min), l-s-byte, mtb */
		unsigned char byte_35; /* min refresh recovery delay time (t rfc4 min), m-s-byte, mtb */
		union { /* upper nibble for t faw */
			unsigned char all_bits;
			struct {
				unsigned char t_faw_min_msn:4, /* t faw min most significant nibble */
					reserved:4;
			} bit_fields;
		} byte_36;
		unsigned char byte_37; /* min four activate window delay time (t faw min), l-s-byte, mtb */
		/* byte 38: min activate to activate delay time (t rrd_s min), diff bank group, mtb */
		unsigned char byte_38;
		/* byte 39: min activate to activate delay time (t rrd_l min), same bank group, mtb */
		unsigned char byte_39;
		unsigned char byte_40; /* min cas to cas delay time (t ccd_l min), same bank group, mtb */
		union { /* upper nibble for t wr min */
			unsigned char all_bits;
			struct {
				unsigned char t_wr_min_msn:4, /* t wr min most significant nibble */
					reserved:4;
			} bit_fields;
		} byte_41;
		unsigned char byte_42; /* min write recovery time (t wr min) */
		union { /* upper nibbles for t wtr min */
			unsigned char all_bits;
			struct {
				unsigned char t_wtr_s_min_msn:4, /* t wtr s min most significant nibble */
					t_wtr_l_min_msn:4; /* t wtr l min most significant nibble */
			} bit_fields;
		} byte_43;
		unsigned char byte_44; /* min write to read time (t wtr s min), diff bank group, mtb */
		unsigned char byte_45; /* min write to read time (t wtr l min), same bank group, mtb */
		unsigned char bytes_46_59[14]; /* reserved; all 0s */
		unsigned char bytes_60_77[18]; /* TODO: connector to sdram bit mapping */
		unsigned char bytes_78_116[39]; /* reserved; all 0s */
		/* fine offset for min cas to cas delay time (t ccd_l min), same bank group, ftb */
		unsigned char byte_117;
		/* fine offset for min activate to activate delay time (t rrd_l min), same bank group, ftb */
		unsigned char byte_118;
		/* fine offset for min activate to activate delay time (t rrd_s min), diff bank group, ftb */
		unsigned char byte_119;
		/* fine offset for min active to active/refresh delay time (t rc min), ftb */
		unsigned char byte_120;
		unsigned char byte_121; /* fine offset for min row precharge delay time (t rp min), ftb */
		unsigned char byte_122; /* fine offset for min ras to cas delay time (t rcd min), ftb */
		unsigned char byte_123; /* fine offset for min cas latency time (t aa min), ftb */
		unsigned char byte_124; /* fine offset for sdram max cycle time (t ck avg max), ftb */
		unsigned char byte_125; /* fine offset for sdram min cycle time (t ck avg min), ftb */
		unsigned char byte_126; /* crc for base configuration section, l-s-byte */
		unsigned char byte_127; /* crc for base configuration section, m-s-byte */
		/*
		 * block 1: module specific parameters for unbuffered memory module types only
		 */
		union { /* (unbuffered) raw card extension, module nominal height */
			unsigned char all_bits;
			struct {
				unsigned char nom_height_max:5, /* in mm */
					raw_cad_ext:3;
			} bit_fields;
		} byte_128;
		union { /* (unbuffered) module maximum thickness */
			unsigned char all_bits;
			struct {
				unsigned char front_thickness_max:4, /* in mm */
					back_thickness_max:4; /* in mm */
			} bit_fields;
		} byte_129;
		union { /* (unbuffered) reference raw card used */
			unsigned char all_bits;
			struct {
				unsigned char ref_raw_card:5,
					ref_raw_card_rev:2,
					ref_raw_card_ext:1;
			} bit_fields;
		} byte_130;
		union { /* (unbuffered) address mapping from edge connector to dram */
			unsigned char all_bits;
			struct {
				unsigned char rank_1_mapping:1,
					reserved:7;
			} bit_fields;
		} byte_131;
		unsigned char bytes_132_191[60]; /* reserved; all 0s */
		unsigned char bytes_192_255[MV_DDR_SPD_DATA_BLOCK1H_SIZE];
		unsigned char bytes_256_319[MV_DDR_SPD_DATA_BLOCK2E_SIZE];
		unsigned char bytes_320_383[MV_DDR_SPD_DATA_BLOCK2M_SIZE];
	} byte_fields;
};

int mv_ddr_spd_timing_calc(union mv_ddr_spd_data *spd_data, unsigned int timing_data[]);
enum mv_ddr_dev_width mv_ddr_spd_dev_width_get(union mv_ddr_spd_data *spd_data);
enum mv_ddr_die_capacity mv_ddr_spd_die_capacity_get(union mv_ddr_spd_data *spd_data);
unsigned char mv_ddr_spd_mem_mirror_get(union mv_ddr_spd_data *spd_data);
unsigned char mv_ddr_spd_cs_bit_mask_get(union mv_ddr_spd_data *spd_data);
unsigned char mv_ddr_spd_dev_type_get(union mv_ddr_spd_data *spd_data);
unsigned char mv_ddr_spd_module_type_get(union mv_ddr_spd_data *spd_data);
int mv_ddr_spd_supported_cls_calc(union mv_ddr_spd_data *spd_data);
unsigned int mv_ddr_spd_supported_cl_get(unsigned int cl);
enum mv_ddr_pkg_rank mv_ddr_spd_pri_bus_width_get(union mv_ddr_spd_data *spd_data);
enum mv_ddr_pkg_rank mv_ddr_spd_bus_width_ext_get(union mv_ddr_spd_data *spd_data);

#endif /* _MV_DDR_SPD_H */
