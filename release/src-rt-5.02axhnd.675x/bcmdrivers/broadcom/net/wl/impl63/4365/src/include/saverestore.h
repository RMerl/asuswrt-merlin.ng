/*
 * Header file for save-restore functionality in driver
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
 * $Id: saverestore.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef _SAVERESTORE_H
#define _SAVERESTORE_H

/* WL_ENAB_RUNTIME_CHECK may be set based upon the #define below (for ROM builds). It may also
 * be defined via makefiles (e.g. ROM auto abandon unoptimized compiles).
 */
#if defined(BCMROMBUILD)
	#ifndef WL_ENAB_RUNTIME_CHECK
		#define WL_ENAB_RUNTIME_CHECK
	#endif
#endif /* BCMROMBUILD */

#ifdef SR_ESSENTIALS /* minimal SR functionality to support srvsdb */
	extern bool _sr_essentials;
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define SR_ESSENTIALS_ENAB() (_sr_essentials)
	#elif defined(SR_ESSENTIALS_DISABLED)
		#define SR_ESSENTIALS_ENAB()	(0)
	#else
		#define SR_ESSENTIALS_ENAB()	(1)
	#endif
#else
	#define SR_ESSENTIALS_ENAB()		(0)
#endif /* SR_ESSENTIALS */

/* Power save related save/restore support */
#ifdef SAVERESTORE
	extern bool _sr;
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define SR_ENAB() (_sr)
	#elif defined(SAVERESTORE_DISABLED)
		#define SR_ENAB()	(0)
	#else
		#define SR_ENAB()	(1)
	#endif
#else
	#define SR_ENAB() 		(0)
#endif /* SAVERESTORE */

#define SRCTL43239_BANK_SIZE(sr_cntrl) ((sr_cntrl & 0x7F0) >> 4)
#define SRCTL43239_BANK_NUM(sr_cntrl) (sr_cntrl & 0xF)
#define SR_HOST 0
#define SR_ENGINE 1

/* BANK size is calculated in the units of 32bit WORDS */
#define SRCTL_BANK_SIZE(sr_cntrl) ((((sr_cntrl & 0x7F0) >> 4) + 1) << 8)
#define SRCTL_BANK_NUM(sr_cntrl) (sr_cntrl & 0xF)

/* Functions called regardless of SAVERESTORE_* or SR_ESSENTIALS_* preprocessor constants */
uint32 sr_chipcontrol(si_t *si_h, uint32 mask, uint32 val);
uint32 sr_chipcontrol2(si_t *si_h, uint32 mask, uint32 val);
uint32 sr_chipcontrol4(si_t *si_h, uint32 mask, uint32 val);
uint32 sr_chipcontrol5(si_t *si_h, uint32 mask, uint32 val);
uint32 sr_chipcontrol6(si_t *si_h, uint32 mask, uint32 val);
uint32 sr_regcontrol4(si_t *si_h, uint32 mask, uint32 val);
uint32 sr_get_cur_minresmask(si_t *sih);

/* Minimal functionality required to operate SR engine */
#if defined(SR_ESSENTIALS)
void sr_download_firmware(si_t *si_h);
int sr_engine_enable(si_t *si_h, bool oper, bool enable);
void sr_save_restore_init(si_t *si_h);
bool sr_cap(si_t *sih);
void sr_get_source_code_array(si_t *sih, uint32 **sr_source_code, uint32 *sr_source_codesz);
#else
#define sr_download_firmware(a)	do { } while (0)
#define sr_engine_enable(a, b, c)	(0)
#define sr_cap(a)	(FALSE)
#endif /* SR_ESSENTIALS */

/* Power save - SR functionality */
#ifdef SAVERESTORE
#if !defined(SR_ESSENTIALS)
error:define SR_ESSENTIALS as well!
#endif // endif
uint32 sr_mem_access(si_t *sih, int op, uint32 addr, uint32 data);
bool sr_isenab(si_t *sih);
void sr_engine_enable_post_dnld(si_t *sih, bool enable);
CONST uint32* sr_get_sr_params(si_t *sih, uint32 *arrsz, uint32 *offset);
#endif /* SAVERESTORE */

#endif /* _SAVERESTORE_H */
