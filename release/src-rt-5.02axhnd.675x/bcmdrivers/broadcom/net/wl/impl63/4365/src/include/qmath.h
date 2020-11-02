/*
 * qmath functions used in arithmatic and DSP operations where
 * fractional operations, saturation support is needed.
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
 * $Id: qmath.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef __QMATH_H__
#define __QMATH_H__

#include <typedefs.h>

int16
qm_sat32(int32 op);

int32
qm_mul321616(int16 op1, int16 op2);

int16
qm_mul16(int16 op1, int16 op2);

int32
qm_muls321616(int16 op1, int16 op2);

uint16
qm_mulu16(uint16 op1, uint16 op2);

int16
qm_muls16(int16 op1, int16 op2);

int32
qm_add32(int32 op1, int32 op2);

int16
qm_add16(int16 op1, int16 op2);

int16
qm_sub16(int16 op1, int16 op2);

int32
qm_sub32(int32 op1, int32 op2);

int32
qm_mac321616(int32 acc, int16 op1, int16 op2);

int32
qm_shl32(int32 op, int shift);

int32
qm_shr32(int32 op, int shift);

int16
qm_shl16(int16 op, int shift);

int16
qm_shr16(int16 op, int shift);

int16
qm_norm16(int16 op);

int16
qm_norm32(int32 op);

int16
qm_div_s(int16 num, int16 denom);

int16
qm_abs16(int16 op);

int16
qm_div16(int16 num, int16 denom, int16 *qQuotient);

int32
qm_abs32(int32 op);

int16
qm_div163232(int32 num, int32 denom, int16 *qquotient);

int32
qm_mul323216(int32 op1, int16 op2);

int32
qm_mulsu321616(int16 op1, uint16 op2);

int32
qm_muls323216(int32 op1, int16 op2);

int32
qm_mul32(int32 a, int32 b);

int32
qm_muls32(int32 a, int32 b);

void
qm_log10(int32 N, int16 qN, int16 *log10N, int16 *qLog10N);

void
qm_1byN(int32 N, int16 qN, int32 *result, int16 *qResult);

#endif /* #ifndef __QMATH_H__ */
