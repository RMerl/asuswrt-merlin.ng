/*
 * FCBS utils interface
 * (common utility API's)
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: fcbsutils.h 688113 2017-03-03 12:32:45Z $
 */

#ifndef	_FCBSUTILS_H_
#define	_FCBSUTILS_H_

#include <typedefs.h>

/* prints */
#define FCBS_INFO(args)
#define FCBS_DBG(args)

#ifdef BCMDBG_ERR
#define	FCBS_ERR(args)	printf args
#else
#define	FCBS_ERR(args)	printf args
#endif	/* BCMDBG_ERR */

#define FCBS_MAX_ROM_SEQ	100
#define FCBS_MAX_ROM_SUB_SEQ	100

#define INITVALS_END_MARKER	0xFFFFFFFF
#define FCBS_DELAY_TPL		0xFFFFFFFE

#define FCBS_MAX_LEN		128

#define ADDR_HI(addr)		(((addr) >> 16) & 0xFFFF)
#define	ADDR_LO(addr)		((addr)  & 0xFFFF)

#define DATA_HI(addr)		(((addr) >> 16) & 0xFFFF)
#define	DATA_LO(addr)		((addr)  & 0xFFFF)

#define FCBS_TPL(val)		((val) & FCBS_TUPLE_MASK)
#define FCBS_DELAY_TUPLE(val)	{FCBS_DELAY_TPL, 0, (val)}

#define IS_WORD_ALIGN(num)	(((num) & 0x3) == 0)
#define WORD_ALIGN(num)		((num) = (((num) + 4) & (~0x3)))

#define TXE_BMC_MARKER		0xaabbccdd
#define TXFIFO_SZ(val)		((val >> 3) & 0x3ff) * 2048

#define BMC_TPL_IDX		7
#define BMC_BUFSIZE_256BLOCK	0	/* 0 indicates 256, 1 indicates 512 */
#define BMC_MAXBUFS		1024
#define TPL_SIZE		5 * BMC_MAXBUFS
#define BMCBUFSZ		1 << (BMC_BUFSIZE_256BLOCK + 8)
#define TPLBUF			TPL_SIZE/BMCBUFSZ

#define TXE_BMC_LINK		0x18001000 + 0x400 + (0xa0*2)
#define TXE_BMC_CONFIG		0x18001000 + 0x400 + (0xa1*2)
#define TXE_BMC_CMD		0x18001000 + 0x400 + (0xa4*2)
#define TXE_BMC_MAXBUFFERS	0x18001000 + 0x400 + (0xa5*2)
#define TXE_BMC_MINBUFFERS	0x18001000 + 0x400 + (0xa6*2)
#define TXE_BMC_CMD_VAL		(1 << 4) | BMC_TPL_IDX
#define TXE_BMC_LINK_VAL	(BMC_BUFSIZE_256BLOCK << 2) | (1 << 0)

/* tuple select */
typedef enum {
	CMD_TUPLE = 0,
	DAT_TUPLE = 1,
	} fcbs_tpl_type_t;

/* fcbs input data classification */
#define FCBS_DATA_SORTED	0x00000001
#define FCBS_DATA_ORDERED	0x00000002
#define FCBS_DATA_COMPRESS	0x00000004
#define FCBS_DATA_TUPLE		0x00000008
#define FCBS_DATA_INITVALS	0x00000010
#define FCBS_DATA_BP_SZ		0x00000010

/* PHY and radio dynamic data input classification */
#define FCBS_PHY_RADIO_DYNAMIC	0x00000020

/* Macros for header creation */
#define FCBS_TYPE_MASK			0xF
#define FCBS_TYPE_SHIFT			0

#define FCBS_LEN_MASK			0x7F
#define FCBS_LEN_SHIFT			4

#define FCBS_CDSIZE_MASK		0x3F
#define FCBS_CDSIZE_SHIFT		11

/* Misc macros */
#define FCBS_TUPLE_NUM_BYTES		2
#define FCBS_TUPLE_NUM_BITS		(FCBS_TUPLE_NUM_BYTES * 8)

#define FCBS_TUPLE_MASK			0xFFFF

#define FCBS_DELAY_VAL_SHIFT		4
#define FCBS_DELAY_VAL_MASK		0xFFF0

#define	FCBS_DELAY_MAX_US		512
#define FCBS_DELAY_DECL			0x00000001

#define FCBS_PHY_TBL_ID_MASK		0xFFF
#define FCBS_PHY_TBL_ID_SHIFT		0

#define FCBS_PHY_TBL_WIDTH_MASK		0xF
#define FCBS_PHY_TBL_WIDTH_SHIFT	12

#define ROUNDUP(x, y)           ((((x) + ((y) - 1)) / (y)) * (y))

enum fcbs_type {
	FCBS_NULL = 0,
	FCBS_RADIO_REG = 1,
	FCBS_PHY_REG = 2,
	FCBS_PHY_TBL = 3,
	FCBS_DELAY = 4,
	FCBS_BP_ACCESS = 5,
	FCBS_RAW_DATA = 6,
	FCBS_DELAY_ULP = 7,
	FCBS_TYPE_MAX = 8
};

/* Backplane Accessisble addresses */
#define fcbs_data_bp(fid, inx, data_sz, flags, data) \
	fcbs_populate_input_data(fid, inx, FCBS_BP_ACCESS, data_sz, flags, data)

/* FCBS input data end marker */
#define fcbs_data_end(fid, inx) \
	fcbs_populate_input_data(fid, inx, FCBS_TYPE_MAX, 0, 0, NULL)

/* FCBS input data: FCBS tuples */
#define fcbs_data_tuples(fid, inx, data) \
	fcbs_populate_input_data(fid, inx, FCBS_NULL, 0, FCBS_DATA_TUPLE, data)

typedef struct fcbs_stage_metadata {
	uint rom_cmd_ptr;
	uint rom_dat_ptr;
	uint ram_cmd_ptr;
	uint ram_cmd_sz;
	uint ram_dat_ptr;
	uint ram_dat_sz;
} fcbs_stage_md_t;

/* ucode and inits structure */
typedef struct d11axiinitval {
	uint32  addr;
	uint32  size;
	uint32  value;
} d11axiiv_t, fcbs_ad_sz_t;

/* For PHY and Radio regs */
typedef struct fcbs_addr_data {
	int addr;
	int data;
} fcbs_input_ad_t, fcbs_ad_t, adp_t;

/* For PHY and Radio dynamic regs */
typedef struct fcbs_phy_rad_dyn_addr_data {
	uint16 addr;
	uint16 mask;
	uint16 static_data;
	uint16 data;
} fcbs_input_phy_rad_dyn_ad_t, phy_rad_dyn_adp_t;

/* For BP Access */
typedef struct fcbs_input_bp {
	int addr_hi;
	fcbs_ad_t *data;
} fcbs_input_bp_t;

/* For PHY Tables */
typedef struct fcbs_phy_tbl_data {
	uint16 id;
	uint16 len;
	uint16 offset;
	uint8 width;
	void *data;
} fcbs_input_phy_tbl_t, phytbl_t;

/* common for all types */
typedef struct fcbs_input_data {
	enum fcbs_type type;
	uint16 data_size;
	uint16 flags;
	void *data;
} fcbs_input_data_t;

typedef struct fcbs_tuples {
	uint16 *cmd_ptr;
	uint16 *dat_ptr;
	uint cmd_size;
	uint dat_size;
	uint cmd_buf_size; /* Used for freeing allocated memory */
	uint dat_buf_size; /* Used for freeing allocated memory */
} fcbs_tuples_t;

extern fcbs_tuples_t *fcbs_create_tuples(void *osh, fcbs_input_data_t *data);
extern void fcbs_free_tuples(void *osh, fcbs_tuples_t *ft);
extern void fcbs_print_tuples(fcbs_tuples_t *ft);
extern void fcbs_populate_input_data(fcbs_input_data_t * fid, uint8 inx, uint8 type,
		uint16 data_size, uint16 flags, void *data);
extern void fcbs_create_addr_data_pairs(fcbs_input_bp_t *p, int i, int addr, int data);
extern fcbs_tuples_t *fcbs_populate_tuples(void *osh, void *cmd, void *dat, int cmd_sz, int dat_sz);
#endif	/* _FCBSUTILS_H_ */
