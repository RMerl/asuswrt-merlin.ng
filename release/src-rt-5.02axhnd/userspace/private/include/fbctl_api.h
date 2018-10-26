#ifndef __FBCTL_API_H_INCLUDED__
#define __FBCTL_API_H_INCLUDED__

/***********************************************************************
 *
 *  Copyright (c) 2013 Broadcom Corporation
 *  All Rights Reserved
 *
 *
<:label-BRCM:2013:proprietary:standard

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

/***************************************************************************
 * File Name  : fbctl_api.h
 * Description: APIs for library that controls the Broadcom Flow Bond.
 ***************************************************************************/

#include <fbond.h>

/*
 *------------------------------------------------------------------------------
 * Function Name: fbCtlStatus
 * Description  : Displays flow bond status.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int fbCtlStatus(void);

/*
 *------------------------------------------------------------------------------
 * Function Name: fbCtlEnable
 * Description  : Enables flow bond.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int fbCtlEnable(void);

/*
 *------------------------------------------------------------------------------
 * Function Name: fbCtlDisable
 * Description  : Disables flow bond.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int fbCtlDisable(void);

/*
 *------------------------------------------------------------------------------
 * Function Name: fbCtlInterval
 * Description  : Sets the interval in msec when flows inactive are cleared
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int fbCtlInterval(unsigned int interval);

/*
 *------------------------------------------------------------------------------
 * Function Name: fbCtlAddIf
 * Description  : Adds an interface to flow bond.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int fbCtlAddIf(unsigned int groupindex, unsigned int ifindex);

/*
 *------------------------------------------------------------------------------
 * Function Name: fbCtlDeleteIf
 * Description  : Deletes an interface from flow bond.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int fbCtlDeleteIf(unsigned int groupindex, unsigned int ifindex);

/*
 *------------------------------------------------------------------------------
 * Function Name: fbCtlTokens
 * Description  : Sets number of tokens for an interface in flow bond.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int fbCtlTokens(unsigned int groupindex, unsigned int ifindex,
                unsigned int tokens, unsigned int max_tokens);

/*
 *------------------------------------------------------------------------------
 * Function Name: fbCtlTest
 * Description  : Randomly assign flows and prints status
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int fbCtlTest(unsigned int num_flows);


#if defined(CC_CONFIG_FBOND_DEBUG)
/*
 *------------------------------------------------------------------------------
 * Function Name: fbCtlDebug
 * Description  : Sets the debug level for the layer in flow bond.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int  fbCtlDebug(int layer, int level);
#endif

#endif  /* defined(__FBCTL_API_H_INCLUDED__) */

