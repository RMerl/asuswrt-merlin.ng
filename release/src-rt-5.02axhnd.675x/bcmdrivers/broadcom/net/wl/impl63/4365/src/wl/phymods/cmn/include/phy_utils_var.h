/*
 * PHY utils - nvram access functions.
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

#ifndef _phy_utils_var_h_
#define _phy_utils_var_h_

#include <typedefs.h>
#include <bcmdefs.h>

#include <wlc_phy_int.h>

/*
 * Search the name=value vars for a specific one and return its
 * value.  Returns NULL if not found.  This version of getvar uses a
 * phy specific instance of the vars.  The phy specific instance of
 * the get var routines guarantee that they are only used during
 * the execution of phy attach.  Any usage after this time will
 * assert/fail.  This is done so the Linux hybrid, where the top
 * of the driver released in source form and the bottom is released
 * as a linkable object file, protects against simple modification
 * of the vars string which might potentially affect regulatory
 * controlled aspects.  Linux hybrid builds also don't search NVRAM
 * if a name is not found in SROM.
 *
 * As an aid in locating any post wlc_phy_attach usage of
 * getvar/getintvar, a BCMDBG build passes the calling function
 * for output.
 */

char * phy_utils_getvar_internal(phy_info_t *pi, const char *name);
int phy_utils_getintvar_default(phy_info_t *pi, const char *name, int default_value);
#ifdef BCMDBG
char * phy_utils_getvar(phy_info_t *pi, const char *name, const char *function);
char * phy_utils_getvar_fabid(phy_info_t *pi, const char *name, const char *function);
char * phy_utils_getvar_fabid_internal(phy_info_t *pi, const char *name, const char *function);
int phy_utils_getintvar(phy_info_t *pi, const char *name, const char *function);
int phy_utils_getintvararray(phy_info_t *pi, const char *name, int idx, const char *function);
int phy_utils_getintvararray_default(phy_info_t *pi, const char *name, int idx, int default_value,
	const char *function);
int phy_utils_getintvararray_default_internal(phy_info_t *pi, const char *name, int idx,
	int default_value, const char *function);
#define PHY_GETVAR(pi, name) phy_utils_getvar_fabid(pi, name, __FUNCTION__)
/* Search the vars for a specific one and return its value as an integer. Returns 0 if not found */
#define PHY_GETINTVAR(pi, name) phy_utils_getintvar(pi, name, __FUNCTION__)
#define PHY_GETINTVAR_DEFAULT(pi, name, default_value) \
	phy_utils_getintvar_default(pi, name, default_value)
#define PHY_GETINTVAR_ARRAY(pi, name, idx) \
	phy_utils_getintvararray(pi, name, idx, __FUNCTION__)
#define PHY_GETINTVAR_ARRAY_DEFAULT(pi, name, idx, default_value) \
	phy_utils_getintvararray_default(pi, name, idx, default_value, __FUNCTION__)
#else
char * phy_utils_getvar(phy_info_t *pi, const char *name);
char * phy_utils_getvar_fabid(phy_info_t *pi, const char *name);
char * phy_utils_getvar_fabid_internal(phy_info_t *pi, const char *name);
int phy_utils_getintvar(phy_info_t *pi, const char *name);
int phy_utils_getintvararray(phy_info_t *pi, const char *name, int idx);
int phy_utils_getintvararray_default(phy_info_t *pi, const char *name, int idx, int default_value);
int phy_utils_getintvararray_default_internal(phy_info_t *pi, const char *name, int idx,
	int default_value);
#define PHY_GETVAR(pi, name)	phy_utils_getvar_fabid(pi, name)
#define PHY_GETINTVAR(pi, name)	phy_utils_getintvar(pi, name)
#define PHY_GETINTVAR_DEFAULT(pi, name, default_value) \
	phy_utils_getintvar_default(pi, name, default_value)
#define PHY_GETINTVAR_ARRAY(pi, name, idx) \
	phy_utils_getintvararray(pi, name, idx)
#define PHY_GETINTVAR_ARRAY_DEFAULT(pi, name, idx, default_value) \
	phy_utils_getintvararray_default(pi, name, idx, default_value)
#endif /* BCMDBG */

#endif /* _phy_utils_var_h_ */
