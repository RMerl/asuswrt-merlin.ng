/***********************************************************************
 *
 *  Copyright (c) 2012  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2012:proprietary:standard

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
 *
 ************************************************************************/

#ifndef _QDM_DIAG_H_
#define _QDM_DIAG_H_


/*!\file qdm_diag.h
 * \brief This file contains declarations diagnostics object query functions
 *
 */

#include "cms.h"

/** Get the UDP Echo configuration 
 *
 * @param enable  (OUT)  A pointer to boolean variable to store the mode (enable=1, disable = 0)
 * @param interface  (OUT)  A buffer of at least MDM_SINGLE_FULLPATH_BUFLEN bytes long.
 * @param port  (OUT)  A pointer to an UINT32 to store the port number
 *
 * @return CmsRet enum.
 */

CmsRet qdmDiag_getUdpEchoCfg(UBOOL8 *enable, char *interface, UINT32 *udpPort);
CmsRet qdmDiag_getUdpEchoCfg_igd(UBOOL8 *enable, char *interface, UINT32 *udpPort);
CmsRet qdmDiag_getUdpEchoCfg_dev2(UBOOL8 *enable, char *interface, UINT32 *udpPort);


#if defined(SUPPORT_DM_LEGACY98)
#define qdmDiag_getUdpEchoCfg(a,b,c)   qdmDiag_getUdpEchoCfg_igd((a),(b),(c))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmDiag_getUdpEchoCfg(a,b,c)   qdmDiag_getUdpEchoCfg_igd((a),(b),(c))
#elif defined(SUPPORT_DM_PURE181)
#define qdmDiag_getUdpEchoCfg(a,b,c)   qdmDiag_getUdpEchoCfg_dev2((a),(b),(c))
#elif defined(SUPPORT_DM_DETECT)
#define qdmDiag_getUdpEchoCfg(a,b,c)       (cmsMdm_isDataModelDevice2() ? \
                                            qdmDiag_getUdpEchoCfg_dev2((a),(b),(c)) : \
                                            qdmDiag_getUdpEchoCfg_igd((a),(b),(c)))
#endif



#endif /* _QDM_DIAG_H_ */



