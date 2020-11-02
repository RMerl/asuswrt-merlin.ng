/*
 * ATE restruct.
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
 * $Id: wl_ate.h kshaha $
 */
#ifndef _wl_ate_h_
#define _wl_ate_h_

#include <siutils.h>
#include <rte_dev.h>

/* Buffer of size WLC_SAMPLECOLLECT_MAXLEN (=10240 for 4345a0 ACPHY)
 * gets copied to this, multiple times
 */
#define ATE_SAMPLE_COLLECT_BUFFER_SIZE	(128*1024)

#define ATE_OTP_REGVAL_SIZE     96

/* Externally used ATE  data structures */
typedef struct _ate_params {
	void*	wl;
	si_t*	sih;
	uint8	gpio_input;
	uint8	gpio_output;
	bool	cmd_proceed;
	uint16	cmd_idx;
	bool	ate_cmd_done;
} ate_params_t;

/* Buffer defn for storing various register values */
typedef struct {
	uint32 gmult_lpf;       /* RCCal lpf result */
	uint32 gmult_adc;       /* RCCal adc result */
	uint32 rccal_dacbuf;    /* RCCal dacbuf result */
	uint32 curr_radio_temp; /* The current radio temperature */
	uint32 rcal_value;	/* the rcal_cfg_north.rcal_value in radioreg offset 0x78 */
	uint32 minipmu_cal;     /* minipmu_cal */
} ate_buffer_regval_t;

extern ate_params_t ate_params;
extern char ate_buffer_sc[];
extern uint32 ate_buffer_sc_size;
extern ate_buffer_regval_t ate_buffer_regval;
extern void cpu_flush_cache_all(void);
extern uint16 otp_regval[ATE_OTP_REGVAL_SIZE];

/* Externally used ATE functions */
void wl_ate_cmd_proc(void);
void wl_ate_init(si_t *sih, hnd_dev_t *bcmwl);

#endif /* _wl_ate_h_ */
