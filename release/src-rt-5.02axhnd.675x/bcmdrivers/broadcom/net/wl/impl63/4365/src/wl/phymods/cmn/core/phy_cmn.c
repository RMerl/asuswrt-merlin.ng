/*
 * PHYComMoN module implementation
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

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_cmn.h>
/*
struct phy_cmn_info {
	shared_phy_t	*sh;
	phy_info_t	*pi[MAX_RSDB_MAC_NUM];
	uint8		num_d11_cores;
	uint16		phymode;
};
*/
phy_cmn_info_t *
phy_cmn_attach(phy_info_t *pi)
{
	phy_cmn_info_t *cmn;
	shared_phy_t *sh;
	int ref_count = 0;
	sh = pi->sh;

	/* OBJECT REGISTRY: check if shared key has value already stored */
	cmn = (phy_cmn_info_t *)wlapi_obj_registry_get(sh->physhim, OBJR_PHY_CMN_INFO);
	if (cmn == NULL) {
		if ((cmn = phy_malloc(pi, sizeof(phy_cmn_info_t))) == NULL) {
			PHY_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n", sh->unit,
			          __FUNCTION__, MALLOCED(sh->osh)));
			return NULL;
		}
		cmn->sh = sh;

		/* OBJECT REGISTRY: We are the first instance, store value for key */
		wlapi_obj_registry_set(sh->physhim, OBJR_PHY_CMN_INFO, cmn);
	}

	/* OBJECT REGISTRY: Reference the stored value in both instances */
	ref_count = wlapi_obj_registry_ref(sh->physhim, OBJR_PHY_CMN_INFO);
	ASSERT(ref_count <= MAX_RSDB_MAC_NUM);

	cmn->phymode = wlapi_get_phymode(sh->physhim);

	cmn->pi[ref_count - 1] = pi;
	cmn->num_d11_cores++;

	PHY_INFORM(("\n*** wl%d: %s: cmn->pi[%d] = pi = %p | num_d11_cores = %d phymode %d ***\n\n",
		sh->unit, __FUNCTION__, ref_count - 1, pi, cmn->num_d11_cores, cmn->phymode));

	return cmn;
}

void
phy_cmn_detach(phy_cmn_info_t *cmn)
{
	shared_phy_t *sh;

	(void)sh;

	if (cmn == NULL)
		return;

	sh = cmn->sh;

	if (wlapi_obj_registry_unref(sh->physhim, OBJR_PHY_CMN_INFO) == 0) {
		wlapi_obj_registry_set(sh->physhim, OBJR_PHY_CMN_INFO, NULL);
		MFREE(sh->osh, cmn, sizeof(phy_cmn_info_t));
	}
}

int
phy_cmn_register_obj(phy_cmn_info_t *ci, phy_obj_ptr_t *obj, phy_obj_type_t type)
{
	return BCME_OK;
}

phy_obj_ptr_t *
phy_cmn_find_obj(phy_cmn_info_t *ci, phy_obj_type_t type)
{
	return NULL;
}

phy_info_t *
phy_get_other_pi(phy_info_t *pi)
{
	ASSERT(pi != NULL && pi->cmni != NULL);

	/* Check, out of bound array access */
	if (MAX_RSDB_MAC_NUM < 2)
		return pi;

	return (pi == pi->cmni->pi[PHY_RSBD_PI_IDX_CORE0])
		? pi->cmni->pi[PHY_RSBD_PI_IDX_CORE1]
		: pi->cmni->pi[PHY_RSBD_PI_IDX_CORE0];
}

void
phy_set_phymode(phy_info_t *pi, uint16 new_phymode)
{
	phy_info_t *other_pi;
	ASSERT(pi != NULL && pi->cmni != NULL);

	if (new_phymode == pi->cmni->phymode)
		return;

	pi->cmni->phymode = new_phymode;

	other_pi = phy_get_other_pi(pi);
	/* MAC driver has updated to a new phymode,
	 * set a flag to trigger phy init.
	 */
	other_pi->phyinit_pending = pi->phyinit_pending = TRUE;
}

uint8
phy_get_current_core(phy_info_t *pi)
{
	ASSERT(pi != NULL && pi->cmni != NULL);

	return (pi == pi->cmni->pi[PHY_RSBD_PI_IDX_CORE0])
		? PHY_RSBD_PI_IDX_CORE0
		: PHY_RSBD_PI_IDX_CORE1;
}

uint16
phy_get_phymode(const phy_info_t *pi)
{
	ASSERT(pi != NULL && pi->cmni != NULL);

	return pi->cmni->phymode;
}

phy_info_t *
phy_get_pi(const phy_info_t *pi, int idx)
{
	ASSERT(pi != NULL && pi->cmni != NULL);
	ASSERT(idx < MAX_RSDB_MAC_NUM);

	return pi->cmni->pi[idx];
}
