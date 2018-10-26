/***********************************************************************
 *
 *  Copyright (c) 2016  Broadcom
 *  All Rights Reserved
 *
<:label-BRCM:2016:proprietary:standard

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

/*****************************************************************************
*    Description:
*
*      BEEP User Management Functions.
*
*****************************************************************************/
#ifndef BEEP_USERMANAGEMENT_H
#define BEEP_USERMANAGEMENT_H

#include <stdio.h>


/* ---- Function Prototypes ----------------------------------------------- */

/*****************************************************************************
*  FUNCTION:  beep_generateUniqueUsername
*  DESCRIPTION:
*     Generate a unique 32-byte username.
*  PARAMETERS:
*     username (OUT) username.
*  RETURNS:
*     0 - success;  -1 - fail
******************************************************************************
*/
int beep_generateUniqueUsername(char *username);


/*****************************************************************************
*  FUNCTION:  beep_add_user
*  DESCRIPTION:
*     Add a user to both passwd and group files.  If uid is -1, the username
*     will be added with the next available uid.  If a valid uid is specified,
*     the username will be added with the specified uid.
*  PARAMETERS:
*     username (IN) username.
*     info (IN) uid info.
*     uid (IN/OUT) uid of the username.
*  RETURNS:
*     0 - success;  -1 - fail
******************************************************************************
*/
int beep_add_user(const char *username, const char *info, int *uid);


/*****************************************************************************
*  FUNCTION:  beep_delete_user
*  DESCRIPTION:
*     Delete the uid from both passwd and group file.
*  PARAMETERS:
*     username (IN) username.
*  RETURNS:
*     0 - success;  -1 - fail
******************************************************************************
*/
int beep_delete_user(const char *username);


/*****************************************************************************
*  FUNCTION:  beep_get_uid_by_username
*  DESCRIPTION:
*     Get uid by username.
*  PARAMETERS:
*     username (IN) username.
*     uid (OUT) uid of username.
*  RETURNS:
*     0 - success;  -1 - fail
******************************************************************************
*/
int beep_get_uid_by_username(const char *username, int *uid);


/*****************************************************************************
*  FUNCTION:  beep_getUsernameByUid
*  DESCRIPTION:
*     Get username by uid.
*  PARAMETERS:
*     uid (IN) uid of username.
*     username (OUT) username.
*     nameLen (IN) username buffer length.
*  RETURNS:
*     0 - success;  -1 - fail
******************************************************************************
*/
void beep_getUsernameByUid(int uid, char *username, int nameLen);


#endif /* BEEP_USERMANAGEMENT_H */
