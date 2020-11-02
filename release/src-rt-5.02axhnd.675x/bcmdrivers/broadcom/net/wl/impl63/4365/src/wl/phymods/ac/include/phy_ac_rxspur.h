/*
 * ACPHY Rx Spur canceller module interface (to other PHY modules).
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
 * $Id$
 */

#ifndef _phy_ac_rxspur_h_
#define _phy_ac_rxspur_h_

#include <phy_api.h>
#include <phy_ac.h>
#include <phy_rxspur.h>

/* forward declaration */
typedef struct phy_ac_rxspur_info phy_ac_rxspur_info_t;
typedef struct acphy_dssf_values acphy_dssf_values_t;
typedef struct acphy_dssfB_values acphy_dssfB_values_t;
typedef struct acphy_spurcan_values acphy_spurcan_values_t;

/* register/unregister ACPHY specific implementations to/from common */
phy_ac_rxspur_info_t
*phy_ac_rxspur_register_impl(phy_info_t *pi, phy_ac_info_t *aci, phy_rxspur_info_t *cmn_info);
void phy_ac_rxspur_unregister_impl(phy_ac_rxspur_info_t *ac_info);

/* ************************************************************************* */

#define ACPHY_SPURWAR_NTONES	8 /* Numver of tones for spurwar */
/* Number of tones(spurwar+nvshp) to be written */
#define ACPHY_SPURWAR_NV_NTONES	32

/* ************************************************************************* */

extern void
phy_ac_spurwar(phy_ac_rxspur_info_t *rxspuri, uint8 noise_var[][ACPHY_SPURWAR_NV_NTONES],
                   int8 *tone_id, uint8 *core_sp);
extern void phy_ac_dssf(phy_ac_rxspur_info_t *rxspuri, bool on);
extern void phy_ac_dssfB(phy_ac_rxspur_info_t *rxspuri, bool on);
extern void phy_ac_spurcan(phy_ac_rxspur_info_t *rxspuri, bool enable);
extern void phy_ac_set_spurmode(phy_ac_rxspur_info_t *rxspuri, uint16 freq);
extern void phy_ac_get_spurmode(phy_ac_rxspur_info_t *rxspuri, uint16 freq);
#if defined(WLTEST)
extern void phy_ac_force_spurmode(phy_ac_rxspur_info_t *rxspuri, int16 int_val);
#endif /* WLTEST */
#endif /* _phy_ac_rxspur_h_ */
