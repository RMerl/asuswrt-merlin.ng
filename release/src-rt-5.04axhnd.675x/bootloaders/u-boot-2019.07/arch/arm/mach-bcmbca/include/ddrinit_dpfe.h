/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#ifndef _DDRINIT_DPFE_H
#define _DDRINIT_DPFE_H

#define DPFE_OPTION_SAFEMODE			0x1
#define DPFE_OPTION_SEG_FIRST			0x2
#define DPFE_OPTION_SEG_LAST			0x4
#define DPFE_OPTION_SEG_MASK			(DPFE_OPTION_SEG_FIRST|DPFE_OPTION_SEG_LAST)

typedef struct _dpfe_seg_param {
	uint8_t* seg_buf;
	uint32_t buf_size;
	int seg_id;
	uint32_t mcb_sel;
}dpfe_seg_param;

typedef struct _dpfe_param {
	dpfe_seg_param* seg_param;
	uint32_t* mcb;
	uint32_t* seed;
	uint32_t* ddr_size;
	uint32_t dpfe_option;
	int      scramble_enable;
	uint64_t unscram_addr;
	uint64_t unscram_size;
}dpfe_param;

typedef int (*dpfe_func) (dpfe_param* params);

#endif
