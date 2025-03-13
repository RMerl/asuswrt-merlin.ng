/***********************************************************************
 *
 *  Copyright (c) 2020  Broadcom Ltd
 *  All Rights Reserved
 *
<:label-BRCM:2020:DUAL/GPL:standard

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

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
