// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009 Stefan Roese <sr@denx.de>, DENX Software Engineering
 *
 * Copyright (C) 2006 Micronas GmbH
 */

#include <common.h>
#include <linux/errno.h>

#include "vct.h"

/*
 * List of statically defined buffers per SCC.
 * The first entry in the table is the number of fixed buffers
 * followed by the list of buffer IDs
 */
static u32 buffer_list_0[] = { 6, 120, 121, 122, 123, 139, 140 };
static u32 buffer_list_1[] = { 6, 120, 121, 122, 123, 139, 140 };
static u32 buffer_list_2[] = { 5, 124, 125, 126, 139, 140 };
static u32 buffer_list_3[] = { 5, 124, 125, 126, 139, 140 };
static u32 buffer_list_4[] = { 5, 124, 125, 126, 139, 140 };
static u32 buffer_list_5[] = { 3, 127, 139, 140 };
static u32 buffer_list_6[] = { 3, 127, 139, 140 };
static u32 buffer_list_7[] = { 6, 128, 129, 130, 131, 139, 140 };
static u32 buffer_list_8[] = { 6, 128, 129, 130, 131, 139, 140 };
static u32 buffer_list_9[] = { 5, 124, 125, 126, 139, 140 };
static u32 buffer_list_10[] = { 5, 124, 125, 126, 139, 140 };
static u32 buffer_list_11[] = { 5, 124, 125, 126, 139, 140 };
static u32 buffer_list_12[] = { 6, 132, 133, 134, 135, 139, 140 };
static u32 buffer_list_13[] = { 6, 132, 133, 134, 135, 139, 140 };
static u32 buffer_list_14[] = { 4, 137, 138, 139, 140 };
static u32 buffer_list_15[] = { 6, 136, 136, 137, 138, 139, 140 };

/** Issue#7674 (new) - DP/DVP buffer assignment */
static u32 buffer_list_16[] = { 6, 106, 108, 109, 107, 139, 140 };
static u32 buffer_list_17[] = { 6, 106, 110, 107, 111, 139, 140 };
static u32 buffer_list_18[] = { 6, 106, 113, 107, 114, 139, 140 };
static u32 buffer_list_19[] = { 3, 112, 139, 140 };
static u32 buffer_list_20[] = { 35, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
				13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
				79, 80, 81, 82, 83, 84, 85, 86, 139, 140 };
static u32 buffer_list_21[] = { 27, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
				13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
				139, 140 };
static u32 buffer_list_22[] = { 81, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
				13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
				25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
				37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
				49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60,
				61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72,
				73, 74, 75, 76, 77, 78, 139, 140 };
static u32 buffer_list_23[] = { 29, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
				13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
				88, 89, 139, 140 };
static u32 buffer_list_24[] = { 6, 90, 91, 92, 93, 139, 140 };
static u32 buffer_list_25[] = { 18, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99,
				100, 101, 102, 103, 104, 105, 139, 140 };
static u32 buffer_list_26[] = { 5, 94, 95, 96, 139, 140 };
static u32 buffer_list_27[] = { 5, 97, 98, 99, 139, 140 };
static u32 buffer_list_28[] = { 5, 100, 101, 102, 139, 140 };
static u32 buffer_list_29[] = { 5, 103, 104, 105, 139, 140 };
static u32 buffer_list_30[] = { 10, 108, 109, 110, 111, 113, 114, 116, 117,
				139, 140 };
static u32 buffer_list_31[] = { 13, 106, 107, 108, 109, 110, 111, 113, 114,
				115, 116, 117, 139, 140 };
static u32 buffer_list_32[] = { 13, 106, 107, 108, 109, 110, 111, 113, 114,
				115, 116, 117, 139, 140 };
static u32 buffer_list_33[] = { 27, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
				13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
				139, 140 };
static u32 buffer_list_34[] = { 27, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
				13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
				139, 140 };
static u32 buffer_list_35[] = { 28, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
				13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
				87, 139, 140 };
static u32 buffer_list_36[] = { 28, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
				13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
				87, 139, 140 };
static u32 buffer_list_37[] = { 27, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
				13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
				139, 140 };
static u32 buffer_list_38[] = { 29, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
				13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
				118, 119, 139, 140 };
static u32 buffer_list_39[] = { 91, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
				13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
				25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
				37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
				49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60,
				61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72,
				73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84,
				85, 86, 118, 119, 139, 140 };
static u32 buffer_list_40[] = { 0 };

/*
 * List of statically defined vcid.csize values.
 * The first entry in the table is the number of possible csize values
 * followed by the list of data path values in bits.
 */
static u32 csize_list_0[] = { 2, 0, 1 };
static u32 csize_list_1[] = { 2, 0, 1 };
static u32 csize_list_2[] = { 1, 1 };
static u32 csize_list_3[] = { 1, 1 };
static u32 csize_list_4[] = { 1, 1 };
static u32 csize_list_5[] = { 1, 0 };
static u32 csize_list_6[] = { 1, 0 };
static u32 csize_list_7[] = { 1, 1 };
static u32 csize_list_8[] = { 1, 1 };
static u32 csize_list_9[] = { 1, 1 };
static u32 csize_list_10[] = { 1, 1 };
static u32 csize_list_11[] = { 1, 1 };
static u32 csize_list_12[] = { 1, 1 };
static u32 csize_list_13[] = { 1, 1 };
static u32 csize_list_14[] = { 1, 2 };
static u32 csize_list_15[] = { 1, 4 };
static u32 csize_list_16[] = { 3, 0, 1, 2 };
static u32 csize_list_17[] = { 3, 0, 1, 2 };
static u32 csize_list_18[] = { 3, 0, 1, 2 };
static u32 csize_list_19[] = { 1, 2 };
static u32 csize_list_20[] = { 1, 0 };
static u32 csize_list_21[] = { 1, 0 };
static u32 csize_list_22[] = { 1, 2 };
static u32 csize_list_23[] = { 1, 3 };
static u32 csize_list_24[] = { 1, 3 };
static u32 csize_list_25[] = { 1, 3 };
static u32 csize_list_26[] = { 1, 0 };
static u32 csize_list_27[] = { 1, 0 };
static u32 csize_list_28[] = { 1, 0 };
static u32 csize_list_29[] = { 1, 0 };
static u32 csize_list_30[] = { 1, 2 };
static u32 csize_list_31[] = { 1, 2 };
static u32 csize_list_32[] = { 1, 2 };
static u32 csize_list_33[] = { 1, 2 };
static u32 csize_list_34[] = { 1, 2 };
static u32 csize_list_35[] = { 1, 2 };
static u32 csize_list_36[] = { 1, 2 };
static u32 csize_list_37[] = { 2, 0, 1 };
static u32 csize_list_38[] = { 1, 2 };
static u32 csize_list_39[] = { 1, 3 };
static u32 csize_list_40[] = { 1, 3 };

/*
 * SCC_Configuration table
 */
static const struct scc_descriptor scc_descriptor_table[] = {
/* scn  scc_name  profile  SCC  scc_id  mci_id  rd  wr   m   p fh  si cfg sta */
	{"fe_", "fe_3dcomb_wr", STRM_P, SCC0_BASE, 0, 0, 0, 4, 1, 1, 0, 0, 0, 1,
	 buffer_list_0, csize_list_0},
	{"fe_", "fe_3dcomb_rd", STRM_P, SCC1_BASE, 1, 18, 4, 0, 1, 1, 0, 1, 0,
	 1, buffer_list_1, csize_list_1},
	{"di_", "di_tnr_wr", STRM_P, SCC2_BASE, 2, 1, 0, 3, 1, 1, 0, 2, 0, 1,
	 buffer_list_2, csize_list_2},
	{"di_", "di_tnr_field_rd", STRM_P, SCC3_BASE, 3, 19, 3, 0, 1, 1, 0, 3,
	 0, 1, buffer_list_3, csize_list_3},
	{"di_", "di_tnr_frame_rd", STRM_P, SCC4_BASE, 4, 20, 3, 0, 1, 1, 0, 4,
	 0, 1, buffer_list_4, csize_list_4},
	{"di_", "di_mval_wr", STRM_P, SCC5_BASE, 5, 2, 0, 1, 1, 1, 0, 5, 0, 1,
	 buffer_list_5, csize_list_5},
	{"di_", "di_mval_rd", STRM_P, SCC6_BASE, 6, 21, 1, 0, 1, 1, 0, 6, 0, 1,
	 buffer_list_6, csize_list_6},
	{"rc_", "rc_frame_wr", STRM_P, SCC7_BASE, 7, 3, 0, 4, 1, 1, 0, 7, 0, 1,
	 buffer_list_7, csize_list_7},
	{"rc_", "rc_frame0_rd", STRM_P, SCC8_BASE, 8, 22, 4, 0, 1, 1, 0, 8, 0,
	 1, buffer_list_8, csize_list_8},
	{"opt", "opt_field0_rd", STRM_P, SCC9_BASE, 9, 23, 3, 0, 1, 1, 0, 9, 0,
	 1, buffer_list_9, csize_list_9},
	{"opt", "opt_field1_rd", STRM_P, SCC10_BASE, 10, 24, 3, 0, 1, 1, 0, 10,
	 0, 1, buffer_list_10, csize_list_10},
	{"opt", "opt_field2_rd", STRM_P, SCC11_BASE, 11, 25, 3, 0, 1, 1, 0, 11,
	 0, 1, buffer_list_11, csize_list_11},
	{"pip", "pip_frame_wr", STRM_P, SCC12_BASE, 12, 4, 0, 4, 1, 1, 0, 12, 0,
	 1, buffer_list_12, csize_list_12},
	{"pip", "pip_frame_rd", STRM_P, SCC13_BASE, 13, 26, 4, 0, 1, 1, 0, 13,
	 0, 1, buffer_list_13, csize_list_13},
	{"dp_", "dp_agpu_rd", STRM_P, SCC14_BASE, 14, 27, 2, 0, 2, 1, 0, 14, 0,
	 1, buffer_list_14, csize_list_14},
	{"ewa", "ewarp_rw", SRMD, SCC15_BASE, 15, 11, 1, 1, 0, 0, 0, -1, 0, 0,
	 buffer_list_15, csize_list_15},
	{"dp_", "dp_osd_rd", STRM_P, SCC16_BASE, 16, 28, 3, 0, 2, 1, 0, 15, 0,
	 1, buffer_list_16, csize_list_16},
	{"dp_", "dp_graphic_rd", STRM_P, SCC17_BASE, 17, 29, 3, 0, 2, 1, 0, 16,
	 0, 1, buffer_list_17, csize_list_17},
	{"dvp", "dvp_osd_rd", STRM_P, SCC18_BASE, 18, 30, 2, 0, 2, 1, 0, 17, 0,
	 1, buffer_list_18, csize_list_18},
	{"dvp", "dvp_vbi_rd", STRM_D, SCC19_BASE, 19, 31, 1, 0, 0, 1, 0, -1, 0,
	 0, buffer_list_19, csize_list_19},
	{"tsi", "tsio_wr", STRM_P, SCC20_BASE, 20, 5, 0, 8, 2, 1, 1, -1, 0, 0,
	 buffer_list_20, csize_list_20},
	{"tsi", "tsio_rd", STRM_P, SCC21_BASE, 21, 32, 4, 0, 2, 1, 1, -1, 0, 0,
	 buffer_list_21, csize_list_21},
	{"tsd", "tsd_wr", SRMD, SCC22_BASE, 22, 6, 0, 64, 0, 0, 1, -1, 0, 0,
	 buffer_list_22, csize_list_22},
	{"vd_", "vd_ud_st_rw", SRMD, SCC23_BASE, 23, 12, 2, 2, 0, 0, 1, -1, 0,
	 0, buffer_list_23, csize_list_23},
	{"vd_", "vd_frr_rd", SRMD, SCC24_BASE, 24, 33, 4, 0, 0, 0, 0, -1, 0, 0,
	 buffer_list_24, csize_list_24},
	{"vd_", "vd_frw_disp_wr", SRMD, SCC25_BASE, 25, 7, 0, 16, 0, 0, 0, -1,
	 0, 0, buffer_list_25, csize_list_25},
	{"mr_", "mr_vd_m_y_rd", STRM_P, SCC26_BASE, 26, 34, 3, 0, 2, 1, 0, 18,
	 0, 1, buffer_list_26, csize_list_26},
	{"mr_", "mr_vd_m_c_rd", STRM_P, SCC27_BASE, 27, 35, 3, 0, 2, 1, 0, 19,
	 0, 1, buffer_list_27, csize_list_27},
	{"mr_", "mr_vd_s_y_rd", STRM_P, SCC28_BASE, 28, 36, 3, 0, 2, 1, 0, 20,
	 0, 1, buffer_list_28, csize_list_28},
	{"mr_", "mr_vd_s_c_rd", STRM_P, SCC29_BASE, 29, 37, 3, 0, 2, 1, 0, 21,
	 0, 1, buffer_list_29, csize_list_29},
	{"ga_", "ga_wr", STRM_P, SCC30_BASE, 30, 8, 0, 1, 1, 1, 0, -1, 1, 1,
	 buffer_list_30, csize_list_30},
	{"ga_", "ga_src1_rd", STRM_P, SCC31_BASE, 31, 38, 1, 0, 1, 1, 0, -1, 1,
	 1, buffer_list_31, csize_list_31},
	{"ga_", "ga_src2_rd", STRM_P, SCC32_BASE, 32, 39, 1, 0, 1, 1, 0, -1, 1,
	 1, buffer_list_32, csize_list_32},
	{"ad_", "ad_rd", STRM_D, SCC33_BASE, 33, 40, 2, 0, 0, 1, 1, -1, 0, 0,
	 buffer_list_33, csize_list_33},
	{"ad_", "ad_wr", STRM_D, SCC34_BASE, 34, 9, 0, 3, 0, 1, 1, -1, 0, 0,
	 buffer_list_34, csize_list_34},
	{"abp", "abp_rd", STRM_D, SCC35_BASE, 35, 41, 5, 0, 0, 1, 1, -1, 0, 0,
	 buffer_list_35, csize_list_35},
	{"abp", "abp_wr", STRM_D, SCC36_BASE, 36, 10, 0, 3, 0, 1, 1, -1, 0, 0,
	 buffer_list_36, csize_list_36},
	{"ebi", "ebi_rw", STRM_P, SCC37_BASE, 37, 13, 4, 4, 2, 1, 1, -1, 0, 0,
	 buffer_list_37, csize_list_37},
	{"usb", "usb_rw", SRMD, SCC38_BASE, 38, 14, 1, 1, 0, 0, 1, -1, 0, 0,
	 buffer_list_38, csize_list_38},
	{"cpu", "cpu1_spdma_rw", SRMD, SCC39_BASE, 39, 15, 1, 1, 0, 0, 1, -1, 0,
	 0, buffer_list_39, csize_list_39},
	{"cpu", "cpu1_bridge_rw", SRMD, SCC40_BASE, 40, 16, 0, 0, 0, 0, 0, -1,
	 0, 0, buffer_list_40, csize_list_40},
};

/* DMA state structures for read and write channels for each SCC */

static struct scc_dma_state scc_state_rd_0[] = { {-1} };
static struct scc_dma_state scc_state_wr_0[] = { {0}, {0}, {0}, {0} };
static struct scc_dma_state scc_state_rd_1[] = { {0}, {0}, {0}, {0} };
static struct scc_dma_state scc_state_wr_1[] = { {-1} };
static struct scc_dma_state scc_state_rd_2[] = { {-1} };
static struct scc_dma_state scc_state_wr_2[] = { {0}, {0}, {0} };
static struct scc_dma_state scc_state_rd_3[] = { {0}, {0}, {0} };
static struct scc_dma_state scc_state_wr_3[] = { {-1} };
static struct scc_dma_state scc_state_rd_4[] = { {0}, {0}, {0} };
static struct scc_dma_state scc_state_wr_4[] = { {-1} };
static struct scc_dma_state scc_state_rd_5[] = { {-1} };
static struct scc_dma_state scc_state_wr_5[] = { {0} };
static struct scc_dma_state scc_state_rd_6[] = { {0} };
static struct scc_dma_state scc_state_wr_6[] = { {-1} };
static struct scc_dma_state scc_state_rd_7[] = { {-1} };
static struct scc_dma_state scc_state_wr_7[] = { {0}, {0}, {0}, {0} };
static struct scc_dma_state scc_state_rd_8[] = { {0}, {0}, {0}, {0} };
static struct scc_dma_state scc_state_wr_8[] = { {-1} };
static struct scc_dma_state scc_state_rd_9[] = { {0}, {0}, {0}, };
static struct scc_dma_state scc_state_wr_9[] = { {-1} };
static struct scc_dma_state scc_state_rd_10[] = { {0}, {0}, {0} };
static struct scc_dma_state scc_state_wr_10[] = { {-1} };
static struct scc_dma_state scc_state_rd_11[] = { {0}, {0}, {0} };
static struct scc_dma_state scc_state_wr_11[] = { {-1} };
static struct scc_dma_state scc_state_rd_12[] = { {-1} };
static struct scc_dma_state scc_state_wr_12[] = { {0}, {0}, {0}, {0} };
static struct scc_dma_state scc_state_rd_13[] = { {0}, {0}, {0}, {0} };
static struct scc_dma_state scc_state_wr_13[] = { {-1} };
static struct scc_dma_state scc_state_rd_14[] = { {0}, {0} };
static struct scc_dma_state scc_state_wr_14[] = { {-1} };
static struct scc_dma_state scc_state_rd_15[] = { {0} };
static struct scc_dma_state scc_state_wr_15[] = { {0} };
static struct scc_dma_state scc_state_rd_16[] = { {0}, {0}, {0} };
static struct scc_dma_state scc_state_wr_16[] = { {-1} };
static struct scc_dma_state scc_state_rd_17[] = { {0}, {0}, {0} };
static struct scc_dma_state scc_state_wr_17[] = { {-1} };
static struct scc_dma_state scc_state_rd_18[] = { {0}, {0} };
static struct scc_dma_state scc_state_wr_18[] = { {-1} };
static struct scc_dma_state scc_state_rd_19[] = { {0} };
static struct scc_dma_state scc_state_wr_19[] = { {-1} };
static struct scc_dma_state scc_state_rd_20[] = { {-1} };
static struct scc_dma_state scc_state_wr_20[] = {
	{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0} };
static struct scc_dma_state scc_state_rd_21[] = { {0}, {0}, {0}, {0} };
static struct scc_dma_state scc_state_wr_21[] = { {-1} };
static struct scc_dma_state scc_state_rd_22[] = { {-1} };
static struct scc_dma_state scc_state_wr_22[] = {
	{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
	{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
	{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
	{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
	{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0} };
static struct scc_dma_state scc_state_rd_23[] = { {0}, {0} };
static struct scc_dma_state scc_state_wr_23[] = { {0}, {0} };
static struct scc_dma_state scc_state_rd_24[] = { {0}, {0}, {0}, {0} };
static struct scc_dma_state scc_state_wr_24[] = { {-1} };
static struct scc_dma_state scc_state_rd_25[] = { {-1} };
static struct scc_dma_state scc_state_wr_25[] = {
	{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
	{0}, {0} };
static struct scc_dma_state scc_state_rd_26[] = { {0}, {0}, {0} };
static struct scc_dma_state scc_state_wr_26[] = { {-1} };
static struct scc_dma_state scc_state_rd_27[] = { {0}, {0}, {0} };
static struct scc_dma_state scc_state_wr_27[] = { {-1} };
static struct scc_dma_state scc_state_rd_28[] = { {0}, {0}, {0} };
static struct scc_dma_state scc_state_wr_28[] = { {-1} };
static struct scc_dma_state scc_state_rd_29[] = { {0}, {0}, {0} };
static struct scc_dma_state scc_state_wr_29[] = { {-1} };
static struct scc_dma_state scc_state_rd_30[] = { {-1} };
static struct scc_dma_state scc_state_wr_30[] = { {0} };
static struct scc_dma_state scc_state_rd_31[] = { {0} };
static struct scc_dma_state scc_state_wr_31[] = { {-1} };
static struct scc_dma_state scc_state_rd_32[] = { {0} };
static struct scc_dma_state scc_state_wr_32[] = { {-1} };
static struct scc_dma_state scc_state_rd_33[] = { {0}, {0} };
static struct scc_dma_state scc_state_wr_33[] = { {-1} };
static struct scc_dma_state scc_state_rd_34[] = { {-1} };
static struct scc_dma_state scc_state_wr_34[] = { {0}, {0}, {0} };
static struct scc_dma_state scc_state_rd_35[] = { {0}, {0}, {0}, {0}, {0} };
static struct scc_dma_state scc_state_wr_35[] = { {-1} };
static struct scc_dma_state scc_state_rd_36[] = { {-1} };
static struct scc_dma_state scc_state_wr_36[] = { {0}, {0}, {0} };
static struct scc_dma_state scc_state_rd_37[] = { {0}, {0}, {0}, {0} };
static struct scc_dma_state scc_state_wr_37[] = { {0}, {0}, {0}, {0} };
static struct scc_dma_state scc_state_rd_38[] = { {0} };
static struct scc_dma_state scc_state_wr_38[] = { {0} };
static struct scc_dma_state scc_state_rd_39[] = { {0} };
static struct scc_dma_state scc_state_wr_39[] = { {0} };
static struct scc_dma_state scc_state_rd_40[] = { {-1} };
static struct scc_dma_state scc_state_wr_40[] = { {-1} };

/* DMA state references to access from the driver */
static struct scc_dma_state *scc_state_rd[] = {
	scc_state_rd_0,
	scc_state_rd_1,
	scc_state_rd_2,
	scc_state_rd_3,
	scc_state_rd_4,
	scc_state_rd_5,
	scc_state_rd_6,
	scc_state_rd_7,
	scc_state_rd_8,
	scc_state_rd_9,
	scc_state_rd_10,
	scc_state_rd_11,
	scc_state_rd_12,
	scc_state_rd_13,
	scc_state_rd_14,
	scc_state_rd_15,
	scc_state_rd_16,
	scc_state_rd_17,
	scc_state_rd_18,
	scc_state_rd_19,
	scc_state_rd_20,
	scc_state_rd_21,
	scc_state_rd_22,
	scc_state_rd_23,
	scc_state_rd_24,
	scc_state_rd_25,
	scc_state_rd_26,
	scc_state_rd_27,
	scc_state_rd_28,
	scc_state_rd_29,
	scc_state_rd_30,
	scc_state_rd_31,
	scc_state_rd_32,
	scc_state_rd_33,
	scc_state_rd_34,
	scc_state_rd_35,
	scc_state_rd_36,
	scc_state_rd_37,
	scc_state_rd_38,
	scc_state_rd_39,
	scc_state_rd_40,
};

static struct scc_dma_state *scc_state_wr[] = {
	scc_state_wr_0,
	scc_state_wr_1,
	scc_state_wr_2,
	scc_state_wr_3,
	scc_state_wr_4,
	scc_state_wr_5,
	scc_state_wr_6,
	scc_state_wr_7,
	scc_state_wr_8,
	scc_state_wr_9,
	scc_state_wr_10,
	scc_state_wr_11,
	scc_state_wr_12,
	scc_state_wr_13,
	scc_state_wr_14,
	scc_state_wr_15,
	scc_state_wr_16,
	scc_state_wr_17,
	scc_state_wr_18,
	scc_state_wr_19,
	scc_state_wr_20,
	scc_state_wr_21,
	scc_state_wr_22,
	scc_state_wr_23,
	scc_state_wr_24,
	scc_state_wr_25,
	scc_state_wr_26,
	scc_state_wr_27,
	scc_state_wr_28,
	scc_state_wr_29,
	scc_state_wr_30,
	scc_state_wr_31,
	scc_state_wr_32,
	scc_state_wr_33,
	scc_state_wr_34,
	scc_state_wr_35,
	scc_state_wr_36,
	scc_state_wr_37,
	scc_state_wr_38,
	scc_state_wr_39,
	scc_state_wr_40,
};

static u32 scc_takeover_mode = SCC_TO_IMMEDIATE;

/* Change mode of the SPDMA for given direction */
static u32 scc_agu_mode_sp = AGU_BYPASS;

/* Change mode of the USB for given direction */
static u32 scc_agu_mode_usb = AGU_BYPASS;

static union scc_softwareconfiguration scc_software_configuration[SCC_MAX];

static u32 dma_fsm[4][4] = {
	/* DMA_CMD_RESET  DMA_CMD_SETUP    DMA_CMD_START    DMA_CMD_STOP */
	/* DMA_STATE_RESET */
	{DMA_STATE_RESET, DMA_STATE_SETUP, DMA_STATE_ERROR, DMA_STATE_ERROR},
	/* DMA_STATE_SETUP */
	{DMA_STATE_RESET, DMA_STATE_SETUP, DMA_STATE_START, DMA_STATE_SETUP},
	/* DMA_STATE_START */
	{DMA_STATE_RESET, DMA_STATE_ERROR, DMA_STATE_START, DMA_STATE_SETUP},
	/* DMA_STATE_ERROR */
	{DMA_STATE_RESET, DMA_STATE_ERROR, DMA_STATE_ERROR, DMA_STATE_ERROR},
};

static void dma_state_process(struct scc_dma_state *dma_state, u32 cmd)
{
	dma_state->dma_status = dma_fsm[dma_state->dma_status][cmd];
	dma_state->dma_cmd = cmd;
}

static void dma_state_process_dma_command(struct scc_dma_state *dma_state,
					  u32 dma_cmd)
{
	dma_state->dma_cmd = dma_cmd;
	switch (dma_cmd) {
	case DMA_START:
	case DMA_START_FH_RESET:
		dma_state_process(dma_state, DMA_CMD_START);
		break;
	case DMA_STOP:
		dma_state_process(dma_state, DMA_CMD_STOP);
		break;
	default:
		break;
	}
}

static void scc_takeover_dma(enum scc_id id, u32 dma_id, u32 drs)
{
	union scc_cmd dma_cmd;

	dma_cmd.reg = 0;

	/* Prepare the takeover for the DMA channel */
	dma_cmd.bits.action = DMA_TAKEOVER;
	dma_cmd.bits.id = dma_id;
	dma_cmd.bits.rid = TO_DMA_CFG;	/* this is DMA_CFG register takeover */
	if (drs == DMA_WRITE)
		dma_cmd.bits.drs = DMA_WRITE;

	reg_write(SCC_CMD(scc_descriptor_table[id].base_address), dma_cmd.reg);
}

int scc_dma_cmd(enum scc_id id, u32 cmd, u32 dma_id, u32 drs)
{
	union scc_cmd dma_cmd;
	struct scc_dma_state *dma_state;

	if ((id >= SCC_MAX) || (id < 0))
		return -EINVAL;

	dma_cmd.reg = 0;

	/* Prepare the takeover for the DMA channel */
	dma_cmd.bits.action = cmd;
	dma_cmd.bits.id = dma_id;
	if (drs == DMA_WRITE) {
		dma_cmd.bits.drs = DMA_WRITE;
		dma_state = &scc_state_wr[id][dma_id];
	} else {
		dma_state = &scc_state_rd[id][dma_id];
	}

	dma_state->scc_id = id;
	dma_state->dma_id = dma_id;
	dma_state_process_dma_command(dma_state, cmd);

	reg_write(SCC_CMD(scc_descriptor_table[id].base_address), dma_cmd.reg);

	return 0;
}

int scc_set_usb_address_generation_mode(u32 agu_mode)
{
	if (AGU_ACTIVE == agu_mode) {
		/* Ensure both DMAs are stopped */
		scc_dma_cmd(SCC_USB_RW, DMA_STOP, 0, DMA_WRITE);
		scc_dma_cmd(SCC_USB_RW, DMA_STOP, 0, DMA_READ);
	} else {
		agu_mode = AGU_BYPASS;
	}

	scc_agu_mode_usb = agu_mode;

	return 0;
}

int scc_setup_dma(enum scc_id id, u32 buffer_tag,
		  u32 type, u32 fh_mode, u32 drs, u32 dma_id)
{
	struct scc_dma_state *dma_state;
	int return_value = 0;
	union scc_dma_cfg dma_cfg;
	u32 *buffer_tag_list;
	u32 tag_count, t, t_valid;

	if ((id >= SCC_MAX) || (id < 0))
		return -EINVAL;

	buffer_tag_list = scc_descriptor_table[id].buffer_tag_list;

	/* if the register is only configured by hw, cannot write! */
	if (1 == scc_descriptor_table[id].hw_dma_cfg)
		return -EACCES;

	if (DMA_WRITE == drs) {
		if (dma_id >= scc_descriptor_table[id].p_dma_channels_wr)
			return -EINVAL;
		dma_state = &scc_state_wr[id][dma_id];
	} else {
		if (dma_id >= scc_descriptor_table[id].p_dma_channels_rd)
			return -EINVAL;
		dma_state = &scc_state_rd[id][dma_id];
	}

	/* Compose the DMA configuration register */
	tag_count = buffer_tag_list[0];
	t_valid = 0;
	for (t = 1; t <= tag_count; t++) {
		if (buffer_tag == buffer_tag_list[t]) {
			/* Tag found - validate */
			t_valid = 1;
			break;
		}
	}

	if (!t_valid)
		return -EACCES;

	/*
	 * Read the register first -- two functions write into the register
	 * it does not make sense to read the DMA config back, because there
	 * are two register configuration sets (drs)
	 */
	dma_cfg.reg = 0;
	dma_cfg.bits.buffer_id = buffer_tag;
	dma_state_process(dma_state, DMA_CMD_SETUP);

	/*
	 * This is Packet CFG set select - usable for TSIO, EBI and those SCCs
	 * which habe 2 packet configs
	 */
	dma_cfg.bits.packet_cfg_id =
		scc_software_configuration[id].bits.packet_select;

	if (type == DMA_CYCLIC)
		dma_cfg.bits.buffer_type = 1;
	else
		dma_cfg.bits.buffer_type = 0;

	if (fh_mode == USE_FH)
		dma_cfg.bits.fh_mode = 1;
	else
		dma_cfg.bits.fh_mode = 0;

	if (id == SCC_CPU1_SPDMA_RW)
		dma_cfg.bits.agu_mode = scc_agu_mode_sp;

	if (id == SCC_USB_RW)
		dma_cfg.bits.agu_mode = scc_agu_mode_usb;

	reg_write(SCC_DMA_CFG(scc_descriptor_table[id].base_address),
		  dma_cfg.reg);

	/* The DMA_CFG needs a takeover! */
	if (SCC_TO_IMMEDIATE == scc_takeover_mode)
		scc_takeover_dma(id, dma_id, drs);

	/* if (buffer_tag is not used) */
	dma_state->buffer_tag = buffer_tag;

	dma_state->scc_id = id;
	dma_state->dma_id = dma_id;

	return return_value;
}

int scc_enable(enum scc_id id, u32 value)
{
	if ((id >= SCC_MAX) || (id < 0))
		return -EINVAL;

	if (value == 0) {
		scc_software_configuration[id].bits.enable_status = 0;
	} else {
		value = 1;
		scc_software_configuration[id].bits.enable_status = 1;
	}
	reg_write(SCC_ENABLE(scc_descriptor_table[id].base_address), value);

	return 0;
}

static inline void ehb(void)
{
	__asm__ __volatile__(
		"	.set	mips32r2	\n"
		"	ehb			\n"
		"	.set	mips0		\n");
}

int scc_reset(enum scc_id id, u32 value)
{
	if ((id >= SCC_MAX) || (id < 0))
		return -EINVAL;

	/* Invert value to the strait logic from the negative hardware logic */
	if (value == 0)
		value = 1;
	else
		value = 0;

	/* Write the value to the register */
	reg_write(SCC_RESET(scc_descriptor_table[id].base_address), value);

	/* sync flush */
	asm("sync");	/* request bus write queue flush */
	ehb();		/* wait until previous bus commit instr has finished */
	asm("nop");	/* wait for flush to occur */
	asm("nop");	/* wait for flush to occur */

	udelay(100);

	return 0;
}
