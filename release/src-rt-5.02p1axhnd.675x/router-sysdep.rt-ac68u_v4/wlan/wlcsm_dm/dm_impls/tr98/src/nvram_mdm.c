/*
<:copyright-BRCM:2016:proprietary:standard

   Copyright (c) 2016 Broadcom
   All Rights Reserved

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
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <netinet/if_ether.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "cms.h"
#include "board.h"

#include "cms_util.h"
#include "mdm.h"
#include "mdm_private.h"
#include "odl.h"
#include "cms_boardcmds.h"
#include "cms_boardioctl.h"

#include "cms_core.h"

#include "cms_dal.h"
#include <bcmnvram.h>

//#define NVRAM_MDM_DBG

/* Const Definition*/
#define MDM_STRCPY(x, y)    if ( (y) != NULL ) \
		    CMSMEM_REPLACE_STRING_FLAGS( (x), (y), mdmLibCtx.allocFlags )

/*MDM Lock Waiting time 3s*/
#define LOCK_WAIT	3000

CmsRet wlWriteNvram(char *str)
{
    MdmPathDescriptor pathDesc;
    CmsRet ret;
    _WlanNvramObject      *nvramObj =NULL;

    INIT_PATH_DESCRIPTOR(&pathDesc);
    if ((ret = cmsObj_get(MDMOID_WLAN_NVRAM, &(pathDesc.iidStack), 0, (void **) &nvramObj)) != \
            CMSRET_SUCCESS) {

        return ret;
    }

    MDM_STRCPY(nvramObj->wlanNvram, str );

    ret = cmsObj_set(nvramObj, &(pathDesc.iidStack));
    cmsObj_free((void **) &nvramObj);

    if ( ret  != CMSRET_SUCCESS ) 	{
        printf("Failure to Set nvram Obj, ret=%d\n", ret );
    }

    return ret;
}



CmsRet wlReadNvram(char *str, int len)
{
    InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
    CmsRet ret;
    WlanNvramObject *nvramObj=NULL;

    if ((ret = cmsObj_get(MDMOID_WLAN_NVRAM, &iidStack, 0, (void *) &nvramObj)) != CMSRET_SUCCESS) {

        return ret;
    }

    if ( nvramObj->wlanNvram != NULL ) {
#ifdef NVRAM_MDM_DBG
        printf(" nvramObj->wlanNvram=[%s]___________\n", nvramObj->wlanNvram);
#endif
        strncpy(str, nvramObj->wlanNvram, len);
    }

    cmsObj_free((void **) &nvramObj);
    return ret;
}

