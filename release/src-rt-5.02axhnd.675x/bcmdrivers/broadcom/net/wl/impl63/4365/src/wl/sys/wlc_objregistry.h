/*
 * WLC Object Registry API definition
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
 * $Id: wlc_objregistry.h 708017 2017-06-29 14:11:45Z $
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

#ifndef _wlc_objregistry_h_
#define _wlc_objregistry_h_

typedef enum obj_registry_key {
	OBJR_SELF,		/* Must be @ 0 */
	OBJR_MODULE_ID,		/* Stores wlc->modulecb */
	OBJR_BSSCFG_PTR,
	OBJR_CLM_PTR,
	OBJR_AMPDUTX_CONFIG,
	OBJR_AMPDURX_CONFIG,
	OBJR_WLC_CMN_INFO,	/* wlc shared structure */
	OBJR_WLC_PUB_CMN_INFO,	/* wlc pub_cmn_t structure */
	OBJR_PHY_CMN_SROM_INFO, /* sharing phycmn srom structure */
	OBJR_PHY_CMN_INFO,	/* phy shared structure	*/
	OBJR_PHY_CMN_RADAR_INFO, /* phy shared radar structure */
	OBJR_ACPHY_SROM_INFO,	/* sharing acphy srom structure	*/
	OBJR_WLC_BANDSTATE,	/* wlc->bandstate[] */
	OBJR_SCANPUBLIC_CMN, /* Common info for scan public */
	OBJR_SCANPRIV_CMN, /* Common info for scan private */
	OBJR_DEFAULT_BSS, /* Shared default BSS  */
	OBJR_P2P_DATA_PTR,	/* shared p2p data */
	OBJR_ASSOCIATION_INFO, /* Shared association info. */
	OBJR_BSSCFG_CUBBY,
	OBJR_SCB_CUBBY,
	OBJR_DFS_SCAN_INFO,	/* DFS scan information. */
	OBJR_MAX_KEYS
} obj_registry_key_t;

#ifdef WL_OBJ_REGISTRY

/* === Object Registry API === */

/* Create the registry only once in wl_xxx.c per port layer and pass it to wlc_attach()
 * Each wlc_attach() creates a new WLC instance that shares the same objr instance
 */
obj_registry_t* obj_registry_alloc(osl_t* osh, int entries);

/* Destroy the registry at the end, after all instances of WLC are freed-up */
void obj_registry_free(obj_registry_t* objr, osl_t *osh);

/* obj_registry_set() is used to setup the value for key.
 * It simply overwrites the existing value if any
 * returns, BCME_OK on success
 * returns, BCME_RANGE if key exceeds, max limit
 */

int obj_registry_set(obj_registry_t *objr, obj_registry_key_t key, void *value);

/* obj_registry_get() is used to get the value for key.
 * return of NULL, indicates key is unused / invalid
 *
 */
void * obj_registry_get(obj_registry_t *objr, obj_registry_key_t key);

/* Ref counting on registry objects is provided for users to keep track of ref counts */

/*
 * Typical call sequence will be as follows:
 * 	Step (1). check if the registry has a value for key 'KEY_X'
 * 	Step (2). if it has value, go to Step (4)
 * 	Step (3). registry has no value, so allocate and store value for 'KEY_X'
 *	Step (4). reference the stored value for 'KEY_X'
 */

/* obj_registy_ref() is used to increment ref_cnt associated with 'key'
 * If there is no value stored, reference is not incremented.
 */
int obj_registry_ref(obj_registry_t *objr, obj_registry_key_t key);

/* obj_registry_unref() is used to decrement ref_cnt associated with 'key'
 * Decrements the reference count for each call.
 * If there is no value stored, reference is not decremented.
 */
int obj_registry_unref(obj_registry_t *objr, obj_registry_key_t key);

int obj_registry_islast(obj_registry_t *objr);

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
/* Debug function to dump out all contents of the registry */
int wlc_dump_objr(obj_registry_t *objr, struct bcmstrbuf *b);
#endif // endif

#else /* WL_OBJ_REGISTRY */

#define obj_registry_alloc(o, c) (NULL)
#define obj_registry_free(d, o)
#define obj_registry_set(d, k, v)
#define obj_registry_get(d, k) (NULL)
#define obj_registry_ref(d, k) (1)
#define obj_registry_unref(d, k) (0)
#define obj_registry_islast(d) (1)

#endif /* WL_OBJ_REGISTRY */

/* Enumerated Keys used in WLC OBJECT REGISTRY */

#endif /* _wlc_objregistry_h_ */
