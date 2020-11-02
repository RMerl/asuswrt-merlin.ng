/*
 * Copyright 1998 Epigram, Inc.
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
 * $Id: smean.h 708017 2017-06-29 14:11:45Z $
 *
 */
#ifndef _SMEAN_H_
#define _SMEAN_H_ 1

/*
 * A small package for keeping smoothed mean values and
 * smoothed mean deviations, based loosely on the
 * floating point examples of TCP round trip time functions.
 *
 * All integer arithmetic.
 * Sample values are integers.
 * Frac_Bits is the number of bits to the right of the decimal point.
 * Div_Bits is the log-base-2 of the divisor used for smoothing. It corresponds
 *    loosely to the number of samples included in the smoothed estimate.
 * The get_smean() and get_sdev() functions return integers.
 *
 * Note: There are no overflow checks at present.
 */

/*
 * A structure for use with general purpose subroutines.
 */
typedef struct smeandev2_struct
{
	unsigned char	div_bits;	/* power of 2 */
	unsigned char	frac_bits;	/* extra bits instead of real rounding */
	int	smean;		/* smoothed mean * 2^frac_bits */
	int	smdev;		/* smoothed mean deviation * 2^frac_bits */
} smeandev2_t;

/*
 * A structure for use with parameterized macros.
 */
typedef struct smeandev_struct
{
	int	smean;		/* smoothed mean * 2^frac_bits */
	int	smdev;		/* smoothed mean deviation * 2^frac_bits */
} smeandev_t;

void
init_smean_sdev(unsigned int frac_bits, unsigned int div_bits, smeandev2_t *p, int startval);

void
update_smean_sdev(smeandev2_t *p, int nextval);

int
get_smean(smeandev2_t *p);

int
get_smdev(smeandev2_t *p);

#define INIT_SMEAN_SDEV(frac_bits, div_bits, p, startval)		\
		do {							\
			(p)->smean = (startval) << (frac_bits);		\
			(p)->smdev = 0;					\
		} while (0)

#define UPDATE_SMEAN_SDEV(frac_bits, div_bits, p, nextval)		\
		do {							\
			int	err;					\
									\
			err = ((nextval) << (frac_bits)) - (p)->smean;	\
			(p)->smean += err >> (div_bits);		\
			if (err < 0) {					\
				err = -err;				\
			}						\
			err -= (p)->smdev;				\
			(p)->smdev += err >> (div_bits);		\
		} while (0)

#define GET_SMEAN(frac_bits, div_bits, p) \
		(((p)->smean + ((1 << (frac_bits)) >> 1)) >> (frac_bits))

#define GET_SMDEV(frac_bits, div_bits, p) \
		(((p)->smdev + ((1 << (frac_bits)) >> 1)) >> (frac_bits))

#endif	/* _SMEAN_H_ */
