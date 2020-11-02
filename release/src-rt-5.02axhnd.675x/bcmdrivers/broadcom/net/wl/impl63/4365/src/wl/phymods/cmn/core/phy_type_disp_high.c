/*
 * PHY Core module implementation - register all PHY type specific implementations'
 * iovar tables/handlers to IOCV module - used by high driver
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

#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_cfg.h>
#include <phy_dbg.h>
#include "phy_type_disp_high.h"

#include <wlc_iocv_types.h>

#if NCONF
#include "phy_type_n_iovt_high.h"
#include "phy_type_n_ioct_high.h"
#endif // endif
#if HTCONF
#include "phy_type_ht_iovt_high.h"
#include "phy_type_ht_ioct_high.h"
#endif // endif
#if LCNCONF
#include "phy_type_lcn_iovt_high.h"
#include "phy_type_lcn_ioct_high.h"
#endif // endif
#if LCN40CONF
#include "phy_type_lcn40_iovt_high.h"
#include "phy_type_lcn40_ioct_high.h"
#endif // endif
#if ACCONF || ACCONF2
#include "phy_type_ac_iovt_high.h"
#include "phy_type_ac_ioct_high.h"
#endif // endif

/* ============= PHY type implementation dispatch table ============= */

typedef struct {
	uint8 phy_type;
	int (*reg_iovt_all)(wlc_iocv_info_t *ii);
	int (*reg_ioct_all)(wlc_iocv_info_t *ii);
} phy_type_reg_tbl_t;

static phy_type_reg_tbl_t BCMATTACHDATA(phy_type_reg_tbl)[] = {
#if NCONF
	{PHY_TYPE_N, phy_n_high_register_iovt, phy_n_high_register_ioct},
#endif // endif
#if HTCONF
	{PHY_TYPE_HT, phy_ht_high_register_iovt, phy_ht_high_register_ioct},
#endif // endif
#if LCNCONF
	{PHY_TYPE_LCN, phy_lcn_high_register_iovt, phy_lcn_high_register_ioct},
#endif // endif
#if LCN40CONF
	{PHY_TYPE_LCN40, phy_lcn40_high_register_iovt, phy_lcn40_high_register_ioct},
#endif // endif
#if ACCONF || ACCONF2
	{PHY_TYPE_AC, phy_ac_high_register_iovt, phy_ac_high_register_ioct},
#endif // endif
	/* *** ADD NEW PHY TYPE IMPLEMENTATION ENTRIES HERE *** */
};

/* register PHY type specific implementation iovar tables/handlers */
int
BCMATTACHFN(phy_type_high_register_iovt)(uint phytype, wlc_iocv_info_t *ii)
{
	uint i;

	/* Unregister PHY type specific implementation with common */
	for (i = 0; i < ARRAYSIZE(phy_type_reg_tbl); i ++) {
		if (phytype == phy_type_reg_tbl[i].phy_type) {
			if (phy_type_reg_tbl[i].reg_iovt_all != NULL)
				return (phy_type_reg_tbl[i].reg_iovt_all)(ii);
			return BCME_OK;
		}
	}

	return BCME_NOTFOUND;
}

/* register PHY type specific implementation ioctl tables/handlers */
int
BCMATTACHFN(phy_type_high_register_ioct)(uint phytype, wlc_iocv_info_t *ii)
{
	uint i;

	/* Unregister PHY type specific implementation with common */
	for (i = 0; i < ARRAYSIZE(phy_type_reg_tbl); i ++) {
		if (phytype == phy_type_reg_tbl[i].phy_type) {
			if (phy_type_reg_tbl[i].reg_ioct_all != NULL)
				return (phy_type_reg_tbl[i].reg_ioct_all)(ii);
			return BCME_OK;
		}
	}

	return BCME_NOTFOUND;
}
