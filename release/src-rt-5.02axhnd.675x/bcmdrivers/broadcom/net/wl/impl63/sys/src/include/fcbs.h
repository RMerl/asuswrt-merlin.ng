/*
 * ULP specific FCBS interface
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
 * $Id: fcbs.h 688113 2017-03-03 12:32:45Z $
 */

#ifndef	_ULP_FCBS_H_
#define	_ULP_FCBS_H_

#include <fcbsutils.h>
#include <d11regs.h>
#include <osl.h>
#include <siutils.h>

#define	FCBS_ROM_SEQ_DISABLE	0xABCDABCD
#define	FCBS_DYN_SEQ_SKIP		0xDEADDEAD

typedef struct fcbs_info fcbs_info_t;
typedef struct fcbs_info * fcbs_info_t_p;

enum {
	FCBS_DS0,	/* 0 */
	FCBS_DS1,	/* 1 */
	FCBS_DS_MAX,
	};

typedef struct fcbs_out {
	uint32 cmd_start;
	uint32 dat_start;
} fcbs_out_t;

enum {
	FCBS_SEQ_IN_ROM = 1,
	FCBS_SEQ_IN_RAM = 2
};

/* Reference:
 * http://confluence.broadcom.com/display/WLAN/BCM43012+Driver+UCODE+Interface
 */
enum {
#ifdef BCMULP
	FCBS_DS1_MAC_INIT_BLOCK,	/* 0 */
	FCBS_DS1_PHY_RADIO_BLOCK,	/* 1 */
	FCBS_DS1_RADIO_PD_BLOCK,	/* 2 */
	FCBS_DS1_EXIT_BLOCK,		/* 3 */
	FCBS_DS0_RADIO_PD_BLOCK,	/* 4 */
	FCBS_DS0_RADIO_PU_BLOCK,	/* 5 */
#else
	FCBS_DS0_RADIO_PD_BLOCK,	/* 0 */
	FCBS_DS0_RADIO_PU_BLOCK,	/* 1 */
#endif /* BCMULP */
};

#define M_FCBS_DS1_MAC_INIT_BLOCK_SZ	8
#define M_FCBS_DS1_PHY_RADIO_BLOCK_SZ	16
#define M_FCBS_DS1_RADIO_PD_BLOCK_SZ	4
#define M_FCBS_DS1_EXIT_BLOCK_SZ	4
#define M_FCBS_DS0_RADIO_PU_BLOCK_SZ	2
#define M_FCBS_DS0_RADIO_PD_BLOCK_SZ	2

#define SHM_ENTRY_MASK			0xFFFF
#define SHM_ENTRY_SIZE			2

#define FCBS_SHM_SEQ_SZ			(6 * 2)		/* each shm = 2 bytes */

enum {
	SHM_FCBS_SEQ_CMD_PTR_INX,	/* 0 */
	SHM_FCBS_SEQ_DAT_PTR_INX,	/* 1 */
	SHM_FCBS_SEQ_CTL_WRD_INX,	/* 2 */
	SHM_FCBS_SEQ_WT_TIME_INX,	/* 3 */
	SHM_FCBS_SEQ_CTRL_ST_INX,	/* 4 */
	SHM_FCBS_SEQ_ACT_TM_INX,	/* 5 */
};

/* sequence id's used by FCBS Blocks */
enum {
	/* ************************************** */
	/* ROM Data                               */
	/* Idx in fcbs_input_ctrl$fcbs_input_data */
	/* ************************************** */

	/* host interrupt */
	TRIGGER_HOST_INTERRUPT	= 0,
	/* init seqs */
	INIT_PHY_REGS		= 1,
	INIT_PHY_TBLS		= 2,
	/* Misc phy functions */
	FUNC_DEAF		= 3,
	FUNC_RETURN_FROM_DEAF	= 4,
	FUNC_TBL_SETUP		= 5,
	FUNC_TBL_CLEANUP	= 6,
	FUNC_PHY_RESET		= 7,
	FUNC_NAPPING		= 8,
	/* pd seqs */
	PWR_DN_RADIO		= 9,

	/* initvals */
	FCBSROM_ULP_INITVALS	= 10,
	FCBSROM_ULP_BSINITVALS	= 11,
	FCBSROM_PAPD_WAVEFORM	= 12,

	/* ************************************** */
	/* RAM Data                               */
	/* Idx in ulp_fcbs$fcbs_ds1_phy_radio_blk */
	/* ************************************** */

	/* Dynamic seqs */
	CHANSPEC_PHY_RADIO	= 5,
	CALCACHE_PHY_RADIO	= 6,

	/* Exec seq */
	DS1_EXEC_MINIPMU_PU	= 9,
	DS1_EXEC_PLL_PU		= 10,
	DS1_EXEC_CHAN_TUNE	= 11,
	EXEC_NAPPING		= 4,

	/* DS0 exec seq */
	DS0_EXEC_MINIPMU_PU	= 0,
	DS0_EXEC_PLL_PU		= 1,
	DS0_EXEC_CHAN_TUNE	= 2,
	DS0_EXEC_RADIO_PD	= 0
};

/* FCBS_DS1_MAC_INIT_BLOCK */
#define FCBS_DYN_MAC_INIT_BLK_AMT_SEQ	2

/* FCBS_DS1_EXIT_BLOCK */
#define FCBS_DS1_EXIT_BLOCK_SEQ0	0

/* === Chip spedicific data: For Chip:43012 Rev: A0 [END] === */

/* The following arrays are generated by FCBS ROM generator tool */
extern uint8 fcbs_metadata[];
extern uint8 fcbs_ram_data[];

extern int fcbs_init(fcbs_info_t *fi, int ds_inx);

/*
 * This function adds/updates an FCBS dynamic sequence for a give block and
 * sequence number.
 *
 * fid: FCBS input data structure
 * blk_num: Block number
 * seq_num: Execution sequence number
 */
extern void fcbs_add_dynamic_seq(fcbs_input_data_t *fid,
		int blk_num, int seq_num);

/*
 * This function updates the FCBS ROM sequence number for a give block and
 * sequence number.
 *
 * blk_num: Block number
 * seq_num: Execution sequence number
 * rom_seq_num:
 *      - FCBS ROM sequence number
 *      - Pass FCBS_ROM_SEQ_DISABLE to disable this sequence
 */
extern void ulp_fcbs_update_rom_seq(int blk_num, int seq_num, int rom_seq_num);

/* If the FCBS sequence is in FCBS ROM then cmd_ptr and dat_ptr points to the addresses in
 * FCBS ROM. In case, if the sequence got abandoned, then cmd_ptr and dat_ptr points to
 * the addresses of the abandoned data present in ARM RAM.
 */
extern int fcbs_rom_metadata(int rom_seq_num, fcbs_tuples_t *ft);

/* attach and detach infra */
extern void BCMATTACHFN(fcbs_detach)(fcbs_info_t* fi);
extern fcbs_info_t_p BCMATTACHFN(fcbs_attach)(osl_t *osh, si_t *sih, d11regs_t *regs, uint8 core);

/* fcbs populate core function
 * INPUT ARGS:
 * 1. Formatted raw data in for a given stage (formatted ==> fcbs_input_data_t)
 * 2. num of sequences for that stage
 */
extern fcbs_out_t fcbs_populate(fcbs_info_t* fi, fcbs_input_data_t *input, int num_tuples,
	int ds_inx);
extern int fcbs_reset_cmd_dat_ptrs(void* ctx, int ds_inx, uint32 cmd_ptr, uint32 data_ptr);

#endif	/* _ULP_FCBS_H_ */
