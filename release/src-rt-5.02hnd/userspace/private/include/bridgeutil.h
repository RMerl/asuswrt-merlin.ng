/*
* <:copyright-BRCM:2015:proprietary:standard
* 
*    Copyright (c) 2015 Broadcom 
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

/***************************************************************************
 * File Name  : bridgeutil.h
 *
 * Description: API definitions for call bridge IOCTL.
 *
 ***************************************************************************/
#ifndef __BRIDGE_UTIL_H__
#define __BRIDGE_UTIL_H__

#ifndef DESKTOP_LINUX
#include <bcm_local_kernel_include/linux/if_bridge.h>
#else
#include <linux/if_bridge.h>
#endif /* DESKTOP_LINUX */

#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif
#ifndef MAX_BRIDGES
#define MAX_BRIDGES 16
#endif
#ifndef BRIDGE_MAX_IFS
#define BRIDGE_MAX_IFS 64
#endif

#define BR_UTIL_FDB_TYPE_DYNAMIC 0
#define BR_UTIL_FDB_TYPE_STATIC  1

#define BR_UTIL_LIMIT_TYPE_MIN   1
#define BR_UTIL_LIMIT_TYPE_MAX   2
#define BR_UTIL_LIMIT_TYPE_USED  3

/*
 *------------------------------------------------------------------------------
 * Function Name: br_util_is_bridge
 * Description  : check whether an given interface is a bridge
 * Returns      : 1 - device is a bridge, 0 - device is not a bridge
 *------------------------------------------------------------------------------
 */
int br_util_is_bridge(const char *brname);

/*
 *------------------------------------------------------------------------------
 * Function Name: br_util_get_port_number
 * Description  : retirieve the bridge port index for a given interface
 * Returns      : negative - error, positive - port number
 *------------------------------------------------------------------------------
 */
int br_util_get_port_number(const char *ifname);

/*
 *------------------------------------------------------------------------------
 * Function Name: br_util_get_port_number
 * Description  : retirieve the bridge port name for the given port number
 * Returns      : negative - error, 0 - success
 *------------------------------------------------------------------------------
 */
int br_util_get_port_name(const char *brname, int port_num, char *ifname, int maxlen);

/*
 *------------------------------------------------------------------------------
 * Function Name: br_util_get_bridges 
 * Description  : Retrieve a list of bridge interfaces 
 * Returns      : 0 - success, otherwise error 
 *                on success num is set to number of bridge interfaces found
 *                and ifindicies will contain the if index for each 
 *                interface found
 *------------------------------------------------------------------------------
 */
int br_util_get_bridges(int ifindicies[], unsigned int *num);

/*
 *------------------------------------------------------------------------------
 * Function Name: br_get_bridge_wan_interfaces 
 * Description  : Retrieve a list of wan interfaces for a give bridge
 * Returns      : 0 - success, otherwise error 
 *                on success num is set to number of wan interfaces found
 *                and ifindicies will contain the if index for each 
 *                interface
 *------------------------------------------------------------------------------
 */
int br_util_get_bridge_wan_interfaces(const char *brName, int ifindicies[], unsigned int *num);

/*
 *------------------------------------------------------------------------------
 * Function Name: br_get_bridge_port_interfaces 
 * Description  : Retrieve a list of wan interfaces for a give bridge
 * Returns      : 0 - success, otherwise error 
 *                on success num is set to number of interfaces found
 *                and ifindicies will contain the if index for each 
 *                interface
 *------------------------------------------------------------------------------
 */
int br_util_get_bridge_port_interfaces(const char *brName, int ifindicies[], unsigned int *num);

/*
 *------------------------------------------------------------------------------
 * Function Name: br_get_bridge_port_interfaces 
 * Description  : Retrieve num (or fewer) fdb entries from bridge ifname
 * Returns      : number of entries read
 *------------------------------------------------------------------------------
 */
int br_util_read_fdb(const char *ifname, struct __fdb_entry *fdbs, int offset, int num);

/*
 *------------------------------------------------------------------------------
 * Function Name: br_util_add_fdb_entry 
 * Description  : Add an fdb entry to the bridge for ifname
 *                Type can BR_UTIL_FDB_TYPE_DYNAMIC or BR_UTIL_FDB_TYPE_STATIC
 * Returns      : 0 - success, otherwise failure
 *------------------------------------------------------------------------------
 */
int br_util_add_fdb_entry(const char *ifname, const unsigned char *macaddr, int type);

/*
 *------------------------------------------------------------------------------
 * Function Name: br_util_del_fdb_entry 
 * Description  : Delete an fdb entry from the bridge for ifname
 *                ifname refers to the device then entry is for, The bridge
 *                is determined by ifname
 * Returns      : 0 - success, otherwise failure
 *------------------------------------------------------------------------------
 */
int br_util_del_fdb_entry(const char *ifname, const unsigned char *macaddr);

/*
 *------------------------------------------------------------------------------
 * Function Name: br_util_flush_fdb 
 * Description  : Flush all fdb entries of a given type for ifname in bridge
 *                brname. Type can BR_UTIL_FDB_TYPE_DYNAMIC or 
 *                BR_UTIL_FDB_TYPE_STATIC. if ifname is NULL all entries in
 *                brname will be deleted.
 * Returns      : 0 - success, otherwise failure
 *------------------------------------------------------------------------------
 */
int br_util_flush_fdb(const char *brname, const char *ifname, int type );

/*
 *------------------------------------------------------------------------------
 * Function Name: br_util_get_fdb_limit 
 * Description  : Retrieve fdb limit corresponding to type for ifname
 * Returns      : 0 - success, otherwise failure
 *------------------------------------------------------------------------------
 */
int br_util_get_fdb_limit(const char *ifname, unsigned int type);

/*
 *------------------------------------------------------------------------------
 * Function Name: br_util_set_fdb_limit 
 * Description  : Set fdb limit corresponding to type
 *                set for ifname if not NULL, brname otherwsie
 * Returns      : 0 - success, otherwise failure
 *------------------------------------------------------------------------------
 */
int br_util_set_fdb_limit(const char *brname, const char *ifname, unsigned int type, unsigned int limit);

#endif /* __BRIDGE_UTIL_H__ */

