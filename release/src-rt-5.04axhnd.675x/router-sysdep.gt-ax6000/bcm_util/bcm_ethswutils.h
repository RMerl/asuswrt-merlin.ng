/***********************************************************************
 *
 *  Copyright (c) 2020  Broadcom Ltd
 *  All Rights Reserved
 *
<:label-BRCM:2020:DUAL/GPL:standard

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
 *
 ************************************************************************/


#ifndef __BCM_ETHSWUTIL_H__
#define __BCM_ETHSWUTIL_H__

/*!\file bcm_ethswutil.h
 * \brief Header file for BCM ethsw util functions.
 *
 */

#include "bcm_retcodes.h"
#include "number_defs.h"



/** Get a list of LAN Only Eth interface names in the system.
 *  This function is deprecated, call bcm_enet_driver_getLANOnlyEthPortIfNameList() instead.
 *  TODO: Delete in 5.04L.03.
 *
 *  @param LANOnlyEthPortIfNameList (OUT) Pointer to char * of pLANOnlyPortList.
 *             This function will malloc() a buffer big enough to hold a list
 *             of all the LAN interface names in the system, separated by comma.
               e.g. eth0,eth1
 *             Caller is responsible for freeing the buffer with free(), NOT
 *             cmsMem_free
 *
 *  @return BcmRet code.
 */
BcmRet ethswUtil_getLANOnlyEthPortIfNameList(char **LANOnlyEthPortIfNameList);


/** Get a list of WAN Only Eth interface names in the system.
 *  This function is deprecated, call bcm_enet_drivergetWANOnlyEthPortIfNameList() instead.
 *  TODO: Delete in 5.04L.03.
 *
 *  @param WANOnlyEthPortIfNameList (OUT) Pointer to char * of pWANOnlyPortList.
 *             This function will malloc() a buffer big enough to hold a list
 *             of all WAN the interface names in the system, separated by comma.
 *             e.g. eth0,eth1
 *             Caller is responsible for freeing the buffer with free(), NOT
 *             cmsMem_free
 *
 *  @return BcmRet code.
 */
BcmRet ethswUtil_getWANOnlyEthPortIfNameList(char **WANOnlyEthPortIfNameList);


/** Get a list of Eth interfaces names in the system, a port in this list can
 *  be configured  as either LAN or WAN.
 *  This function is deprecated, call bcm_enet_driver_getLanWanPortIfNameList() instead.
 *  TODO: Delete in 5.04L.03.
 *
 *  @param LanWanPortIfNameList (OUT) Pointer to char * of pLanWanPortIfNameList.
 *              This function will malloc() a buffer big enough to hold a list
 *              of all the interface names in the system, separated by comma.
 *              e.g. eth0,eth1
 *              Caller is responsible for freeing the buffer with free(), NOT
 *              cmsMem_free
 *
 *  @return BcmRet code.
 */
BcmRet ethswUtil_getLanWanPortIfNameList(char **LanWanPortIfNameList);


/** Get number of LAN Only Eth interface names in the system.
 *
 *  @param prefix (IN) interface prefix or NULL.
 *  @param prefixLen (IN) interface prefix length if applicable.
 *  @param numP (OUT) Pointer to the counter.
 *
 *  @return BcmRet code.
 */
BcmRet ethswUtil_getLANOnlyEthPortNumByPrefix(const char *prefix,
                                              UINT32 prefixLen, UINT32 *numP);



#endif /* __BCM_ETHSWUTIL_H__ */
