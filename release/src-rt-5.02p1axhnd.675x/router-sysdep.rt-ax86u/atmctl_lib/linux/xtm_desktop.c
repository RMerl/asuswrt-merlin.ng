/***********************************************************************
 *
 *  Copyright (c) 2007  Broadcom Corporation
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
 * File Name  : xtm.c
 *
 * Description: This file contains the the user mode wrapper to the kernel
 *              bcmxtmcfg driver API.
 ***************************************************************************/

/* Includes. */
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>      /* open */ 
#include <unistd.h>     /* exit */
#include <sys/ioctl.h>  /* ioctl */
#include "cms.h"
#include "cms_log.h"
#include "xtmcfgdrv.h"


/* Globals. */
int g_nXtmFd = -1;


/***************************************************************************
 * Function Name: devCtl_xtmInitialize
 * Description  : Initializes the object.
 * Returns      : CmsRet return code
 ***************************************************************************/
CmsRet devCtl_xtmInitialize( PXTM_INITIALIZATION_PARMS pInitParms __attribute__((unused)))
{

   return CMSRET_SUCCESS;

}


/***************************************************************************
 * Function Name: devCtl_xtmUninitialize
 * Description  : Clean up resources allocated during devCtl_xtmInitialize.
 * Returns      : CmsRet return code
 ***************************************************************************/
CmsRet devCtl_xtmUninitialize( void )
{

   return CMSRET_SUCCESS;

}


/***************************************************************************
 * Function Name: devCtl_xtmReInitialize
 * Description  : Restarts the SAR HW and SAR driver with
 * pre-configuration. Debug tool.
 * Returns      : CmsRet return code
 ***************************************************************************/
CmsRet devCtl_xtmReInitialize( void )
{
   return CMSRET_SUCCESS;
}

/***************************************************************************
 * Function Name: devCtl_xtmGetTrafficDescrTable
 * Description  : Returns the Traffic Descriptor Table.
 * Returns      : CmsRet return code
 ***************************************************************************/
CmsRet devCtl_xtmGetTrafficDescrTable(
    PXTM_TRAFFIC_DESCR_PARM_ENTRY pTrafficDescrTable __attribute__((unused)),
    UINT32 *pulTrafficDescrTableSize __attribute__((unused)))
{

   return CMSRET_SUCCESS;

}


/***************************************************************************
 * Function Name: devCtl_xtmSetTrafficDescrTable
 * Description  : Saves the supplied Traffic Descriptor Table to a private
 *                data member.
 * Returns      : CmsRet return code
 ***************************************************************************/
CmsRet devCtl_xtmSetTrafficDescrTable(
    PXTM_TRAFFIC_DESCR_PARM_ENTRY pTrafficDescrTable __attribute__((unused)),
    UINT32 ulTrafficDescrTableSize __attribute__((unused)) )
{

   return CMSRET_SUCCESS;

}


/***************************************************************************
 * Function Name: devCtl_xtmGetInterfaceCfg
 * Description  : Calls the interface object for the specified interface id
 *                to return the interface configuration record.
 * Returns      : CmsRet return code
 ***************************************************************************/
CmsRet devCtl_xtmGetInterfaceCfg( UINT32 ulPortId __attribute__((unused)),
    PXTM_INTERFACE_CFG pInterfaceCfg __attribute__((unused)))
{

   return CMSRET_SUCCESS;
}


/***************************************************************************
 * Function Name: devCtl_xtmSetInterfaceCfg
 * Description  : Calls the interface object for the specified interface id
 *                to save a new interface configuration record.
 * Returns      : CmsRet return code
 ***************************************************************************/
CmsRet devCtl_xtmSetInterfaceCfg( UINT32 ulPortId __attribute__((unused)),
    PXTM_INTERFACE_CFG pInterfaceCfg __attribute__((unused)))
{

   return CMSRET_SUCCESS;
}


/***************************************************************************
 * Function Name: devCtl_xtmGetConnCfg
 * Description  : Returns the connection configuration record for the specified
 *                connection address.
 * Returns      : CmsRet return code
 ***************************************************************************/
CmsRet devCtl_xtmGetConnCfg( PXTM_ADDR pConnAddr __attribute__((unused)),
                             PXTM_CONN_CFG pConnCfg __attribute__((unused)))
{
   return CMSRET_SUCCESS;
}


/***************************************************************************
 * Function Name: devCtl_xtmSetConnCfg
 * Description  : Saves the connection configuration record for the specified
 *                connection address.
 * Returns      : CmsRet return code
 ***************************************************************************/
CmsRet devCtl_xtmSetConnCfg( PXTM_ADDR pConnAddr __attribute__((unused)),
                             PXTM_CONN_CFG pConnCfg __attribute__((unused)))
{
   return CMSRET_SUCCESS;
} 


/***************************************************************************
 * Function Name: devCtl_xtmGetConnAddrs
 * Description  : Returns the configured connection addresses for an interface. 
 * Returns      : CmsRet return code
 ***************************************************************************/
CmsRet devCtl_xtmGetConnAddrs( PXTM_ADDR pConnAddrs __attribute__((unused)),
                               UINT32 *pulNumConns __attribute__((unused)))
{
   return CMSRET_SUCCESS;
} 


/***************************************************************************
 * Function Name: devCtl_xtmGetInterfaceStatistics
 * Description  : Returns the statistics record for an interface.
 * Returns      : CmsRet return code
 ***************************************************************************/
CmsRet devCtl_xtmGetInterfaceStatistics( UINT32 ulPortId __attribute__((unused)),
    PXTM_INTERFACE_STATS pStatistics __attribute__((unused)),
    UINT32 ulReset __attribute__((unused)))
{
   return CMSRET_SUCCESS;
}


/***************************************************************************
 * Function Name: devCtl_xtmSetInterfaceLinkInfo
 * Description  : Calls the interface object for the specified interface id
 *                to set physical link information.
 * Returns      : CmsRet return code
 ***************************************************************************/
CmsRet devCtl_xtmSetInterfaceLinkInfo( UINT32 ulPortId __attribute__((unused)),
    PXTM_INTERFACE_LINK_INFO pLi __attribute__((unused)))
{
   return CMSRET_SUCCESS;
}


/***************************************************************************
 * Function Name: devCtl_xtmSendOamCell
 * Description  : Sends an OAM request packet.
 * Returns      : CmsRet return code
 ***************************************************************************/
CmsRet devCtl_xtmSendOamCell( PXTM_ADDR pConnAddr __attribute__((unused)),
    PXTM_OAM_CELL_INFO pOamCellInfo __attribute__((unused)))
{
   return CMSRET_SUCCESS;
}


/***************************************************************************
 * Function Name: devCtl_xtmCreateNetworkDevice
 * Description  : Calls the bcmxtmcfg driver to create an bcmxtmrt Linux
 *                network device instance.
 * Returns      : CmsRet return code
 ***************************************************************************/
CmsRet devCtl_xtmCreateNetworkDevice( PXTM_ADDR pConnAddr __attribute__((unused)),
    char *pszNetworkDeviceName __attribute__((unused)))
{
   return CMSRET_SUCCESS;
}


/***************************************************************************
 * Function Name: devCtl_xtmDeleteNetworkDevice
 * Description  : Calls the interface object for the specified interface id
 *                to set physical link information.
 * Returns      : CmsRet return code
 ***************************************************************************/
CmsRet devCtl_xtmDeleteNetworkDevice( PXTM_ADDR pConnAddr __attribute__((unused)))
{
   return CMSRET_SUCCESS;
}


/* Backward compatibility function. */
#define OAM_LB_SEGMENT_TYPE                 0
#define OAM_LB_END_TO_END_TYPE              1
#define OAM_F4_LB_SEGMENT_TYPE              2
#define OAM_F4_LB_END_TO_END_TYPE           3

typedef struct
{
    ATM_ADDR VccAddr;
    UINT32 type;
    BCMXTM_STATUS baStatus;
    UINT32 repetition;
    UINT32 timeout;
    UINT32 sent;
    UINT32 received;
    UINT32 minResponseTime;
    UINT32 maxResponseTime;
    UINT32 avgResponseTime;
} ATMDRV_OAM_LOOPBACK, *PATMDRV_OAM_LOOPBACK;

/***************************************************************************
 * Function Name: devCtl_atmSendOamLoopbackTest
 * Description  : Sends an OAM request packet.
 * Returns      : CmsRet return code
 ***************************************************************************/
CmsRet devCtl_atmSendOamLoopbackTest(UINT32 type __attribute__((unused)),
    PATM_ADDR pVccAddr __attribute__((unused)),
    UINT32 repetition __attribute__((unused)),
    UINT32 timeout __attribute__((unused)),
    ATMDRV_OAM_LOOPBACK *results __attribute__((unused)))
{
   return CMSRET_SUCCESS;
} /* devCtl_atmSendOamLoopbackTest */

CmsRet devCtl_xtmGetBondingInfo ( PXTM_BOND_INFO pBondInfo __attribute__((unused)))
{
   return CMSRET_SUCCESS;
} 

CmsRet devCtl_xtmGetErrorStatistics( PXTM_ERROR_STATS pErrStats __attribute__((unused)))
{
   return CMSRET_SUCCESS;
} 

CmsRet devCtl_xtmManageThreshold( PXTM_THRESHOLD_PARMS pThresholdParms __attribute__((unused)))
{
   return CMSRET_SUCCESS;
} 

CmsRet devCtl_xtmConfig( PXTM_CONFIGURATION_PARMS pConfigParms __attribute__((unused)))
{
   return CMSRET_SUCCESS;
} 
