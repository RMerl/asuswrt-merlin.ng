/*
 * MU-MIMO private header file. Includes things that are needed by
 * both wlc_murx.h and wlc_mutx.h.
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
 * $Id: wlc_mumimo.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef _wlc_mumimo_h_
#define _wlc_mumimo_h_

#include <bcmutils.h>
#include <proto/802.11.h>

#define MU_CLIENT_INDEX_NONE  0xFFFF

#define MIMO_GROUP_NUM    (VHT_SIGA1_GID_MAX_GID + 1)

/* Maximum value for MU user position */
#define MU_USER_POS_MAX  3

/* MU group ID range. Groups 0 and 63 are reserved for SU. */
#define MU_GROUP_ID_MIN 1
#define MU_GROUP_ID_MAX (VHT_SIGA1_GID_MAX_GID - 1)

/* Can be used as an MU group ID wildcard. Same value as SU. */
#define MU_GROUP_ID_ANY 0

/* Maximum number of MU clients */
#define MUCLIENT_NUM  8

/* Minimum number of MU clients */
#define MUCLIENT_NUM_MIN  2
#define MUCLIENT_NUM_4  4
#define MUCLIENT_NUM_6  6

/* MU client scheduler duration */
#define MUCLIENT_SCHEDULER_DUR  60

/* Number of bytes in membership bit mask for a STA. */
#define MU_MEMBERSHIP_SIZE  (ROUNDUP(MIMO_GROUP_NUM, NBBY)/NBBY)

/* Number of bits used to identify a user position within a group. */
#define MU_POSITION_BIT_LEN  2

/* Number of bytes in user position bit string for a STA. */
#define MU_POSITION_SIZE  (ROUNDUP(MIMO_GROUP_NUM * MU_POSITION_BIT_LEN, NBBY)/NBBY)

#endif   /* _wlc_mumimo_h_ */
