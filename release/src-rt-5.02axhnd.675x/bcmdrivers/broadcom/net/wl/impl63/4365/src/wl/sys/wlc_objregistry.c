/*
 * WLC Object Registry API Implementation
 * Broadcom 802.11abg Networking Device Driver
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
 * $Id: wlc_objregistry.c 708017 2017-06-29 14:11:45Z $
 */

/**
 * @file
 * @brief
 * With the rise of RSDB (Real Simultaneous Dual Band), the need arose to support two 'wlc'
 * structures, with the requirement to share data between the two. To meet that requirement, a
 * simple 'key=value' mechanism is introduced.
 *
 * WLC Object Registry provides mechanisms to share data across WLC instances in RSDB
 * The key-value pairs (enums/void *ptrs) to be stored in the "registry" are decided at design time.
 * Even the Non_RSDB (single instance) goes thru the Registry calls to have a unified interface.
 * But the Non_RSDB functions call have dummy/place-holder implementation managed using MACROS.
 *
 * The registry stores key=value in a simple array which is index-ed by 'key'
 * The registry also maintains a reference counter, which helps the caller in freeing the
 * 'value' associated with a 'key'
 * The registry stores objects as pointers represented by "void *" and hence a NULL value
 * indicates unused key
 *
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */

#ifdef WL_OBJ_REGISTRY

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <wlioctl.h>
#include <wl_dbg.h>
#include <wlc_types.h>
#include <d11.h>
#include <wlc_rate.h>
#include <siutils.h>
#include <wlc_pub.h>
#include <wlc_key.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_objregistry.h>

#ifdef BCMDBG
#define WL_OBJR_DBG(x) printf x
#else
#define WL_OBJR_DBG(x)
#endif // endif

struct obj_registry {
	int count;
	void **value;
	uint8 *ref;
};

/* WLC Object Registry Public APIs */
obj_registry_t* obj_registry_alloc(osl_t *osh, int count)
{
	obj_registry_t *objr = NULL;
	if ((objr = MALLOCZ(osh, sizeof(obj_registry_t))) == NULL) {
			WL_ERROR(("wl %s: out of memory, malloced %d bytes\n",
				__FUNCTION__, MALLOCED(osh)));
	} else {
		objr->count = count;

		if ((objr->value =
			MALLOCZ(osh, sizeof(void*) * count)) == NULL) {
			WL_ERROR(("wl %s: out of memory, malloced %d bytes\n",
				__FUNCTION__, MALLOCED(osh)));
			obj_registry_free(objr, osh);
			return NULL;
		}

		if ((objr->ref =
			MALLOCZ(osh, sizeof(uint8) * count)) == NULL) {
			WL_ERROR(("wl %s: out of memory, malloced %d bytes\n",
				__FUNCTION__, MALLOCED(osh)));
			obj_registry_free(objr, osh);
			return NULL;
		}
	}
	WL_OBJR_DBG(("WLC_OBJR CREATED : %p\n", objr));

	return objr;
}

void obj_registry_free(obj_registry_t *objr, osl_t *osh)
{
	if (objr) {
#ifdef BCMDBG
		if (objr->value && objr->ref) {
			/* Check if some stale refs are still present */
			int i = 0;
			for (i = 0; i < objr->count; i++) {
				if (objr->ref[i] || objr->value[i]) {
					WL_ERROR(("key:%d ref:%d value:%p\n",
						i, objr->ref[i], objr->value[i]));
					ASSERT(0);
				}
			}
		}
#endif /* BCMDBG */
		if (objr->value)
			MFREE(osh, objr->value, sizeof(void*) * objr->count);
		if (objr->ref)
			MFREE(osh, objr->ref, sizeof(uint8) * objr->count);
		MFREE(osh, objr, sizeof(obj_registry_t));
	}
}

int obj_registry_set(obj_registry_t *objr, obj_registry_key_t key, void *value)
{
	if (objr) {
		WL_OBJR_DBG(("%s:%d:key[%d]=value[%p]\n", __FUNCTION__, __LINE__, (int)key, value));
		if ((key < 0) || (key >= objr->count)) return BCME_RANGE;
		objr->value[(int)key] = value;
		return BCME_OK;
	}
	return BCME_ERROR;
}

void* obj_registry_get(obj_registry_t *objr, obj_registry_key_t key)
{
	void *value = NULL;
	if (objr) {
		if ((key < 0) || (key >= objr->count)) return NULL;
		value = objr->value[(int)key];
		WL_OBJR_DBG(("%s:%d:key[%d]=value[%p]\n", __FUNCTION__, __LINE__, (int)key, value));
	}
	return value;
}

int obj_registry_ref(obj_registry_t *objr, obj_registry_key_t key)
{
	int ref = 0;
	if (objr && (key >= 0) && (key < objr->count)) {
#ifdef BCMDBG
		void *value = NULL;
		value = objr->value[(int)key];
#endif // endif
		ref = ++(objr->ref[(int)key]);
		WL_OBJR_DBG(("%s:%d: key[%d]=value[%p]/REF[%d]\n",
			__FUNCTION__, __LINE__, (int)key, value, ref));
	}
	return ref;
}

int obj_registry_unref(obj_registry_t *objr, obj_registry_key_t key)
{
	int ref = 0;
	if (objr && (key >= 0) && (key < objr->count)) {
#ifdef BCMDBG
		void *value = NULL;
		value = objr->value[(int)key];
#endif // endif
		ref = (objr->ref[(int)key]);
		if (ref > 0) {
			ref = --(objr->ref[(int)key]);
			WL_OBJR_DBG(("%s:%d: key[%d]=value[%p]/REF[%d]\n",
				__FUNCTION__, __LINE__, (int)key, value, ref));
		}
	}
	return ref;
}

/* A special helper function to identify if we are cleaning up for the finale WLC */
int obj_registry_islast(obj_registry_t *objr)
{
	if (objr && (objr->value[OBJR_SELF])) {
		return ((objr->ref[OBJR_SELF] <= 1) ? 1 : 0);
	}
	return 1;
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
int
wlc_dump_objr(obj_registry_t *objr, struct bcmstrbuf *b)
{
	int i = 0;
	WL_OBJR_DBG(("%s: %d \n", __FUNCTION__, __LINE__));
	if (objr) {
		if (b) {
			bcm_bprintf(b, "\nDumping Object Registry\n");
			bcm_bprintf(b, "Key\tValue\t\tRef\n");
			for (i = 0; i < objr->count; i++) {
				bcm_bprintf(b, "%d\t%p\t%d\n",
					i, objr->value[(int)i], objr->ref[(int)i]);
			}
		} else {
			WL_OBJR_DBG(("\nkey\tvalue\tref\n"));
			for (i = 0; i < objr->count; i++) {
				WL_OBJR_DBG(("%d\t%p\t%d\n",
					i, objr->value[(int)i], objr->ref[(int)i]));
			}
		}
	}
	else {
		bcm_bprintf(b, "\nObject Registry is not present\n");
	}

	return BCME_OK;
}
#endif /* BCMDBG */

#endif /* WL_OBJ_REGISTRY */
