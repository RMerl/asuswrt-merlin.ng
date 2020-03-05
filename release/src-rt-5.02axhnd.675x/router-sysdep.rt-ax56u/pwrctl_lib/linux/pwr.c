/***********************************************************************
 *
 *  Copyright (c) 2008-2010  Broadcom Corporation
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

/***************************************************************************
 * File Name  : pwrmngt.c
 *
 * Description: This file contains the the user mode wrapper to the kernel
 *              Power Management API.
 ***************************************************************************/

/* Includes. */
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>      /* open */ 
#include <unistd.h>     /* exit */
#include <sys/ioctl.h>  /* ioctl */
#include "cms_retcodes.h"
#include "bcmpwrmngtcfg.h"
#include "devctl_pwrmngt.h"
#include "bcmpwrmngtdrv.h"

/* Globals. */
int g_nPwrFd = -1 ;

/***************************************************************************
 * Function Name: OpenBcmPwrMngtCfg
 * Description  : Opens the power management device.
 * Returns      : device handle if successsful or -1 if error
 ***************************************************************************/
static int OpenBcmPwrMngtCfg( void )
{
    int nFd = open( "/dev/pwrmngt", O_RDWR ) ;

    return( nFd );
} /* OpenBcmPwrMngtCfg */

/***************************************************************************
 * Function Name: PwrMngtDrvErr2Str
 * Description  : Maps and error code to an error strings.
 * Returns      : device handle if successsful or -1 if error
 ***************************************************************************/
static char* PwrMngtDrvErr2Str(PWRMNGT_STATUS nRet)
{
    switch (nRet)
    {
    case PWRMNGTSTS_SUCCESS:
       return("Power Management driver returns Success\n");

    case PWRMNGTSTS_INIT_FAILED:
       return("Power Management driver returns Initialization Fail Error\n");

    case PWRMNGTSTS_ERROR:
       return("Power Management driver returns General Error\n");

    case PWRMNGTSTS_PARAMETER_ERROR:
       return("Power Management driver returns Parameter Error\n");

    case PWRMNGTSTS_ALLOC_ERROR:
       return("Power Management driverreturns Memory Allocation Error\n");

    case PWRMNGTSTS_NOT_SUPPORTED:
       return("Power Management driver returns Not Supported Error\n");

    case PWRMNGTSTS_TIMEOUT:
       return("Power Management driver returns Timeout Error\n");

    default:
       return("Power Management driver returns error.\n" );
    } /* switch */
} /* PwrMngtDrvErr2Str */

/***************************************************************************
 * Function Name: PwrMngtReturnCmsCode
 * Description  : Returns the CMS return code for a given PwrMngt return code.
 * Returns      : CmsRet return code
 ***************************************************************************/
static CmsRet PwrMngtReturnCmsCode( PWRMNGT_STATUS status)
{
    CmsRet Ret;

    if( status == PWRMNGTSTS_SUCCESS )
        Ret = CMSRET_SUCCESS;
    else
    {
        Ret = CMSRET_INTERNAL_ERROR;
    }

    return( Ret );
} /* PwrMngtReturnCmsCode */

/***************************************************************************
 * Function Name: PwrMngtCtl_Initialize
 * Description  : Initializes the driver instance.
 * Returns      : CmsRet return code
 ***************************************************************************/
CmsRet PwrMngtCtl_Initialize(PPWRMNGT_CONFIG_PARAMS pInitParms)
{
    PWRMNGT_DRV_INITIALIZE Arg ;

    memcpy ((UINT8*) &Arg.init, pInitParms, sizeof (PWRMNGT_CONFIG_PARAMS)) ;
    Arg.status = PWRMNGTSTS_ERROR ;

    if( g_nPwrFd == -1  )
        g_nPwrFd = OpenBcmPwrMngtCfg();

    if( g_nPwrFd != -1  ) {
        if (ioctl( g_nPwrFd, PWRMNGT_IOCTL_INITIALIZE, &Arg ) < 0) {
            Arg.status = PWRMNGTSTS_ERROR;
        }
    }
    else
        Arg.status = PWRMNGTSTS_ERROR;

    return( PwrMngtReturnCmsCode(Arg.status) );
} /* PwrMngtCtl_Initialize*/

/***************************************************************************
 * Function Name: PwrMngtCtl_Uninitialize
 * Description  : Clean up resources allocated during PwrMngtCtl_Initialize.
 * Returns      : CmsRet return code
 ***************************************************************************/
CmsRet PwrMngtCtl_Uninitialize( void )
{
    PWRMNGT_DRV_STATUS_ONLY Arg = {PWRMNGTSTS_ERROR};

    if( g_nPwrFd == -1  )
        g_nPwrFd = OpenBcmPwrMngtCfg();

    if( g_nPwrFd != -1  )
    {
        if (ioctl( g_nPwrFd, PWRMNGT_IOCTL_UNINITIALIZE, &Arg ) < 0) {
            Arg.status = PWRMNGTSTS_ERROR;
        }
        close( g_nPwrFd );
        g_nPwrFd = -1;
    }
    else
        Arg.status = PWRMNGTSTS_ERROR;

    return( PwrMngtReturnCmsCode(Arg.status) );
} /* PwrMngtCtl_Uninitialize */

/***************************************************************************
 * Function Name: PwrMngtCtl_GetConfig
 * Description  : Returns the PwrMngt configuration parameters.
 * Returns      : CmsRet return code
 ***************************************************************************/
CmsRet PwrMngtCtl_GetConfig(
    PPWRMNGT_CONFIG_PARAMS pCfgParams, UINT32 configMask)
{
    PWRMNGT_DRV_CONFIG_PARAMS Arg ;

    memcpy(&Arg.configParams, pCfgParams, sizeof(PWRMNGT_CONFIG_PARAMS));
    Arg.configMask = configMask ;
    Arg.status = PWRMNGTSTS_ERROR ;

    if( g_nPwrFd == -1  )
        g_nPwrFd = OpenBcmPwrMngtCfg();

    if( g_nPwrFd != -1  )
    {
        if (ioctl( g_nPwrFd, PWRMNGT_IOCTL_GET_CONFIG, &Arg ) < 0) {
            Arg.status = PWRMNGTSTS_ERROR;
        }
        else {
            memcpy(pCfgParams, &Arg.configParams, sizeof(PWRMNGT_CONFIG_PARAMS));
        }
    }
    else
        Arg.status = PWRMNGTSTS_ERROR;

    return( PwrMngtReturnCmsCode(Arg.status) );
} /* PwrMngtCtl_GetConfig*/


/***************************************************************************
 * Function Name: PwrMngtCtl_SetConfig
 * Description  : calls the set global configuration API to effect the new
 *                configuration.
 * Returns      : CmsRet return code
 ***************************************************************************/
CmsRet PwrMngtCtl_SetConfig(
    PPWRMNGT_CONFIG_PARAMS pCfgParams, UINT32 configMask,
    void *mh __attribute__((unused)))
{
    PWRMNGT_DRV_CONFIG_PARAMS Arg ;

    memcpy(&Arg.configParams, pCfgParams, sizeof(PWRMNGT_CONFIG_PARAMS));
    Arg.configMask = configMask ;
    Arg.status = PWRMNGTSTS_ERROR ;
	
    /* Now, do the kernel work */
    if( g_nPwrFd == -1  )
        g_nPwrFd = OpenBcmPwrMngtCfg();

    if( g_nPwrFd != -1  )
    {
        if (ioctl( g_nPwrFd, PWRMNGT_IOCTL_SET_CONFIG, &Arg ) < 0) {
            Arg.status = PWRMNGTSTS_ERROR;
        }            
    }
    else
        Arg.status = PWRMNGTSTS_ERROR;

    return( PwrMngtReturnCmsCode(Arg.status) );
} /* PwrMngtReturnCmsCode */
