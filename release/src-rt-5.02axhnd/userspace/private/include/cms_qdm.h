/*
* <:copyright-BRCM:2012:proprietary:standard
* 
*    Copyright (c) 2012 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
:>
*/

#ifndef __CMS_QDM_H__
#define __CMS_QDM_H__


/*!\file cms_qdm.h
 * \brief Header file for the Query Data Model (QDM) library.
 *
 * The purpose of this library is to provide a generic and clean API
 * for rcl/stl/rut functions which need to query the MDM for some info
 * and for the various upper layer codes, such has httpd, DAL, TR69
 * to do the same queries.  Prior to 4.14, upper layer codes would call
 * "rut" functions directly, which violated the layered architecture of CMS.
 * The QDM library resolves this problem by providing a place for the
 * query code needed by both the rcl/stl/rut functions and also the
 * upper layer codes.
 * 
 * Rules for functions in the QDM library:
 * 1. Functions must not modify the MDM.  It should only query, search, and
 *    return results.
 * 2. Functions must not accept entire MDM objects as arguments.  Individual
 *    pieces of info needed for the queries must be passed in as separate
 *    arguments.
 * 3. Functions must not return entire MDM objects as results.  The desired
 *    result must be returned as basic types, such as strings, UBOOL8, int, etc.
 * 4. Functions must not have any dependence on TR98 or TR181 data model.
 *    Functions should use #ifdefs or separate files to implement TR98,
 *    hybrid, or TR181 versions of the same function.  By hiding the
 *    data model details inside the function, callers will not have to change
 *    based on which data model is in use.
 * 5. Callers of the function must acquire the CMS lock.  The functions in
 *    the QDM do not acquire the lock (because from inside the rcl/stl/rut
 *    functions, the lock has already been acquired).  To reenforce this point,
 *    all function names in the QDM library must end with "Locked", e.g.
 *    qdmModsw_getExecEnvPathLocked(const char *name);
 */


#include "qdm_modsw_ee.h"
#include "qdm_tr69c.h"
#include "qdm_intf.h"
#include "qdm_lan.h"
#include "qdm_dhcpv4.h"
#include "qdm_route.h"
#include "qdm_dns.h"
#include "qdm_multicast.h"
#include "qdm_qos.h"
#include "qdm_vlan.h"
#include "qdm_dsl.h"
#include "qdm_ipintf.h"
#include "qdm_ipv6.h"
#include "qdm_xtm.h"
#include "qdm_ethernet.h"
#include "qdm_diag.h"
#include "qdm_xmppc.h"
#include "qdm_system.h"
#include "qdm_wifi.h"

#endif /* __CMS_QDM_H__ */
