/***********************************************************************
 *
 *  Copyright (c) 2006-2010  Broadcom Corporation
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
#include "bcm_ulog.h"
#include "board.h"
#include "bcm_boardctl.h"
#include "xtmcfgdrv.h"
#include "devctl_xtm.h"


/* Globals. */
int g_nXtmFd = -1;


/***************************************************************************
 * Function Name: OpenBcmXtmCfg
 * Description  : Opens the bcmxtmcfg device.
 * Returns      : device handle if successsful or -1 if error
 ***************************************************************************/
static int OpenBcmXtmCfg( void )
{
    int nFd = open( "/dev/bcmxtmcfg0", O_RDWR );

    if( nFd == -1 )
        bcmuLog_error( "devCtl_xtmInitialize: open error %d\n", errno );

    return( nFd );
} /* OpenBcmXtmCfg */

/***************************************************************************
 * Function Name: xtmDrvErr2Str
 * Description  : Maps and error code to an error strings.
 * Returns      : device handle if successsful or -1 if error
 ***************************************************************************/
static char* xtmDrvErr2Str(BCMXTM_STATUS nRet)
{
    switch (nRet)
    {
    case XTMSTS_SUCCESS:
       return("XTM driver returns No Error\n");

    case XTMSTS_ERROR:
       return("XTM driver returns General Error\n");

    case XTMSTS_STATE_ERROR:
       return("XTM driver returns State Error\n");

    case XTMSTS_PARAMETER_ERROR:
       return("XTM driver returns Parameter Error\n");

    case XTMSTS_ALLOC_ERROR:
       return("XTM driver returns Memory Allocation Error\n");

    case XTMSTS_RESOURCE_ERROR:
       return("XTM driver returns Resource Error\n");

    case XTMSTS_IN_USE:
       return("XTM driver returns In Use Error\n");

    case XTMSTS_NOT_FOUND:
       return("XTM driver returns Not Found Error\n");

    case XTMSTS_NOT_SUPPORTED:
       return("XTM driver returns Not Supported Error\n");

    case XTMSTS_TIMEOUT:
       return("XTM driver returns Timeout Error\n");

    default:
       return("XTM driver returns error.\n" );
    } /* switch */
} /* xtmDrvErr2Str */

/***************************************************************************
 * Function Name: xtmReturnCmsCode
 * Description  : Returns the CMS return code for a given XTM return code.
 * Returns      : CmsRet return code
 ***************************************************************************/
static CmsRet xtmReturnCmsCode( BCMXTM_STATUS bxStatus )
{
    CmsRet Ret;

    if( bxStatus == XTMSTS_SUCCESS )
        Ret = CMSRET_SUCCESS;
    else
    {
        bcmuLog_debug( "%s\n", xtmDrvErr2Str(bxStatus) );
        if (bxStatus == XTMSTS_NOT_SUPPORTED) 
            Ret = CMSRET_METHOD_NOT_SUPPORTED;
	else
            Ret = CMSRET_INTERNAL_ERROR;
    }

    return( Ret );
} /* xtmReturnCmsCode */


/***************************************************************************
 * Function Name: devCtl_xtmInitialize
 * Description  : Initializes the object.
 * Returns      : CmsRet return code
 ***************************************************************************/
CmsRet devCtl_xtmInitialize( PXTM_INITIALIZATION_PARMS pInitParms )
{
    XTMCFGDRV_INITIALIZE Arg;

    devCtl_xtmInitializeTrace();

    /* set default values for the unspecified parameters */
    if (pInitParms->ulReceiveQueueSizes[0] == 0)
    {
        /* for small ram system like 8MB (48R), use smaller buffer */
        if (devCtl_getSdramSize() <= SZ_8MB)
            pInitParms->ulReceiveQueueSizes[0] = DEFAULT_SMALL_PACKET_Q_SIZE;
        else
            /* 16MB or 32MB boards */
            pInitParms->ulReceiveQueueSizes[0] = DEFAULT_PACKET_Q_SIZE;
    }

    if (pInitParms->ulReceiveQueueSizes[1] == 0)
        pInitParms->ulReceiveQueueSizes[1] = DEFAULT_CELL_Q_SIZE;

    if (pInitParms->bondConfig.sConfig.ptmBond != BC_PTM_BONDING_DISABLE &&
        pInitParms->bondConfig.sConfig.ptmBond != BC_PTM_BONDING_ENABLE)
        pInitParms->bondConfig.sConfig.ptmBond = BC_PTM_BONDING_DISABLE;

    if (pInitParms->bondConfig.sConfig.atmBond != BC_ATM_BONDING_DISABLE &&
        pInitParms->bondConfig.sConfig.atmBond != BC_ATM_BONDING_ENABLE)
        pInitParms->bondConfig.sConfig.atmBond = BC_ATM_BONDING_DISABLE;

    memcpy(&Arg.Init, pInitParms, sizeof(XTM_INITIALIZATION_PARMS));
    Arg.bxStatus = XTMSTS_ERROR;

    if( g_nXtmFd == -1  )
        g_nXtmFd = OpenBcmXtmCfg();

    if( g_nXtmFd != -1  )
        ioctl( g_nXtmFd, XTMCFGIOCTL_INITIALIZE, &Arg );
    else
        Arg.bxStatus = XTMSTS_STATE_ERROR;

    return( xtmReturnCmsCode(Arg.bxStatus) );
} /* devCtl_xtmInitialize */


/***************************************************************************
 * Function Name: devCtl_xtmUninitialize
 * Description  : Clean up resources allocated during devCtl_xtmInitialize.
 * Returns      : CmsRet return code
 ***************************************************************************/
CmsRet devCtl_xtmUninitialize( void )
{
    XTMCFGDRV_STATUS_ONLY Arg = {XTMSTS_ERROR};

    devCtl_xtmUninitializeTrace();

    if( g_nXtmFd == -1  )
        g_nXtmFd = OpenBcmXtmCfg();

    if( g_nXtmFd != -1  )
    {
        ioctl( g_nXtmFd, XTMCFGIOCTL_UNINITIALIZE, &Arg );
        close( g_nXtmFd );
        g_nXtmFd = -1;
    }
    else
        Arg.bxStatus = XTMSTS_STATE_ERROR;

    return( xtmReturnCmsCode(Arg.bxStatus) );
} /* devCtl_xtmUninitialize */


/***************************************************************************
 * Function Name: devCtl_xtmReInitialize
 * Description  : Restarts the XTM driver wit its own configuration
 * (pre-existing) along with SAR HW. More like a debug tool.
 * Returns      : CmsRet return code
 ***************************************************************************/
CmsRet devCtl_xtmReInitialize( void )
{
    XTMCFGDRV_STATUS_ONLY Arg = {XTMSTS_ERROR};

    devCtl_xtmReInitializeTrace();

    if( g_nXtmFd == -1  )
        g_nXtmFd = OpenBcmXtmCfg();

    if( g_nXtmFd != -1  )
    {
        ioctl( g_nXtmFd, XTMCFGIOCTL_REINITIALIZE, &Arg );
        close( g_nXtmFd );
        g_nXtmFd = -1;
    }
    else
        Arg.bxStatus = XTMSTS_STATE_ERROR;

    return( xtmReturnCmsCode(Arg.bxStatus) );
} /* devCtl_xtmReInitialize */


/***************************************************************************
 * Function Name: devCtl_xtmGetTrafficDescrTable
 * Description  : Returns the Traffic Descriptor Table.
 * Returns      : CmsRet return code
 ***************************************************************************/
CmsRet devCtl_xtmGetTrafficDescrTable(
    PXTM_TRAFFIC_DESCR_PARM_ENTRY pTrafficDescrTable,
    UINT32 *pulTrafficDescrTableSize )
{
    XTMCFGDRV_TRAFFIC_DESCR_TABLE Arg = 
        {pTrafficDescrTable, *pulTrafficDescrTableSize, XTMSTS_ERROR};

    devCtl_xtmGetTrafficDescrTableTrace();

    if( g_nXtmFd == -1  )
        g_nXtmFd = OpenBcmXtmCfg();

    if( g_nXtmFd != -1  )
    {
        ioctl( g_nXtmFd, XTMCFGIOCTL_GET_TRAFFIC_DESCR_TABLE, &Arg );
        *pulTrafficDescrTableSize = Arg.ulTrafficDescrTableSize;
    }
    else
        Arg.bxStatus = XTMSTS_STATE_ERROR;

    return( xtmReturnCmsCode(Arg.bxStatus) );
} /* devCtl_xtmGetTrafficDescrTable */




/***************************************************************************
 * Function Name: devCtl_xtmConfig
 * Description  : Pass new configuration parameters to the object.
 * Returns      : CmsRet return code
 ***************************************************************************/
CmsRet devCtl_xtmConfig( PXTM_CONFIGURATION_PARMS pConfigParms )
{
    XTMCFGDRV_CONFIGURE Arg;

    devCtl_xtmConfigTrace();

    memcpy(&Arg.Config, pConfigParms, sizeof(XTMCFGDRV_CONFIGURE));
    Arg.bxStatus = XTMSTS_ERROR;

    if( g_nXtmFd == -1  )
        g_nXtmFd = OpenBcmXtmCfg();

    if( g_nXtmFd != -1  )
        ioctl( g_nXtmFd, XTMCFGIOCTL_CONFIGURE, &Arg );
    else
        Arg.bxStatus = XTMSTS_STATE_ERROR;

    return( xtmReturnCmsCode(Arg.bxStatus) );

}

/***************************************************************************
 * Function Name: devCtl_xtmManageThreshold
 * Description  : Pass new threshold parameters to the object.
 * Returns      : CmsRet return code
 ***************************************************************************/
CmsRet devCtl_xtmManageThreshold( PXTM_THRESHOLD_PARMS pThresholdParms )
{
    XTMCFGDRV_THRESHOLD Arg;

    devCtl_xtmManageThresholdTrace();

    memcpy(&Arg.Threshold, pThresholdParms, sizeof(XTMCFGDRV_THRESHOLD));
    Arg.bxStatus = XTMSTS_ERROR;

    if( g_nXtmFd == -1  )
        g_nXtmFd = OpenBcmXtmCfg();

    if( g_nXtmFd != -1  )
        ioctl( g_nXtmFd, XTMCFGIOCTL_MANAGE_THRESHOLD, &Arg );
    else
        Arg.bxStatus = XTMSTS_STATE_ERROR;

    return( xtmReturnCmsCode(Arg.bxStatus) );

}

/***************************************************************************
 * Function Name: devCtl_xtmSetTrafficDescrTable
 * Description  : Saves the supplied Traffic Descriptor Table to a private
 *                data member.
 * Returns      : CmsRet return code
 ***************************************************************************/
CmsRet devCtl_xtmSetTrafficDescrTable(
    PXTM_TRAFFIC_DESCR_PARM_ENTRY pTrafficDescrTable,
    UINT32 ulTrafficDescrTableSize )
{
    XTMCFGDRV_TRAFFIC_DESCR_TABLE Arg = 
        {pTrafficDescrTable, ulTrafficDescrTableSize, XTMSTS_ERROR};

    devCtl_xtmSetTrafficDescrTableTrace();

    if( g_nXtmFd == -1  )
        g_nXtmFd = OpenBcmXtmCfg();

    if( g_nXtmFd != -1  )
        ioctl( g_nXtmFd, XTMCFGIOCTL_SET_TRAFFIC_DESCR_TABLE, &Arg );
    else
        Arg.bxStatus = XTMSTS_STATE_ERROR;

    return( xtmReturnCmsCode(Arg.bxStatus) );
} /* devCtl_xtmSetTrafficDescrTable */


/***************************************************************************
 * Function Name: devCtl_xtmGetInterfaceCfg
 * Description  : Calls the interface object for the specified interface id
 *                to return the interface configuration record.
 * Returns      : CmsRet return code
 ***************************************************************************/
CmsRet devCtl_xtmGetInterfaceCfg( UINT32 ulPortId,
    PXTM_INTERFACE_CFG pInterfaceCfg )
{
    XTMCFGDRV_INTERFACE_CFG Arg;

    devCtl_xtmGetInterfaceCfgTrace();

    Arg.ulPortId = ulPortId;
    memcpy(&Arg.InterfaceCfg, pInterfaceCfg, sizeof(XTM_INTERFACE_CFG));
    Arg.bxStatus = XTMSTS_ERROR;

    if( g_nXtmFd == -1  )
        g_nXtmFd = OpenBcmXtmCfg();

    if( g_nXtmFd != -1  )
    {
        ioctl( g_nXtmFd, XTMCFGIOCTL_GET_INTERFACE_CFG, &Arg );
        memcpy(pInterfaceCfg, &Arg.InterfaceCfg, sizeof(XTM_INTERFACE_CFG));
    }
    else
        Arg.bxStatus = XTMSTS_STATE_ERROR;

    return( xtmReturnCmsCode(Arg.bxStatus) );
} /* devCtl_xtmGetInterfaceCfg */


/***************************************************************************
 * Function Name: devCtl_xtmSetInterfaceCfg
 * Description  : Calls the interface object for the specified interface id
 *                to save a new interface configuration record.
 * Returns      : CmsRet return code
 ***************************************************************************/
CmsRet devCtl_xtmSetInterfaceCfg( UINT32 ulPortId,
    PXTM_INTERFACE_CFG pInterfaceCfg )
{
    XTMCFGDRV_INTERFACE_CFG Arg;

    devCtl_xtmSetInterfaceCfgTrace();

    Arg.ulPortId = ulPortId;
    memcpy(&Arg.InterfaceCfg, pInterfaceCfg, sizeof(XTM_INTERFACE_CFG));
    Arg.bxStatus = XTMSTS_ERROR;


    if( g_nXtmFd == -1  )
        g_nXtmFd = OpenBcmXtmCfg();

    if( g_nXtmFd != -1  )
        ioctl( g_nXtmFd, XTMCFGIOCTL_SET_INTERFACE_CFG, &Arg );
    else
        Arg.bxStatus = XTMSTS_STATE_ERROR;

    return( xtmReturnCmsCode(Arg.bxStatus) );
} /* devCtl_xtmSetInterfaceCfg */


/***************************************************************************
 * Function Name: devCtl_xtmSetDsPtmBondingDeviation
 * Description  : Calls the interface object for the specified interface id
 *                to save a new interface configuration record.
 * Returns      : CmsRet return code
 ***************************************************************************/
CmsRet devCtl_xtmSetDsPtmBondingDeviation ( UINT32 ulDeviation )
{
    XTMCFGDRV_PTMBONDINGCFG Arg;

    devCtl_xtmSetDsPtmBondingDeviationTrace();

    Arg.ulDeviation = ulDeviation;
    Arg.bxStatus = XTMSTS_ERROR;

    if( g_nXtmFd == -1  )
        g_nXtmFd = OpenBcmXtmCfg();

    if( g_nXtmFd != -1  )
        ioctl( g_nXtmFd, XTMCFGIOCTL_SET_DS_PTMBONDING_DEVIATION, &Arg );
    else
        Arg.bxStatus = XTMSTS_STATE_ERROR;

    return( xtmReturnCmsCode(Arg.bxStatus) );
} /* devCtl_xtmSetDsPtmBondingDeviation */

/***************************************************************************
 * Function Name: devCtl_xtmGetConnCfg
 * Description  : Returns the connection configuration record for the specified
 *                connection address.
 * Returns      : CmsRet return code
 ***************************************************************************/
CmsRet devCtl_xtmGetConnCfg( PXTM_ADDR pConnAddr, PXTM_CONN_CFG pConnCfg )
{
    XTMCFGDRV_CONN_CFG Arg;

    devCtl_xtmGetConnCfgTrace();

    memcpy(&Arg.ConnAddr, pConnAddr, sizeof(XTM_ADDR));
    memcpy(&Arg.ConnCfg, pConnCfg, sizeof(XTM_CONN_CFG));
    Arg.bxStatus = XTMSTS_ERROR;

    if( g_nXtmFd == -1  )
        g_nXtmFd = OpenBcmXtmCfg();

    if( g_nXtmFd != -1  )
    {
        ioctl( g_nXtmFd, XTMCFGIOCTL_GET_CONN_CFG, &Arg );
        memcpy(pConnCfg, &Arg.ConnCfg, sizeof(XTM_CONN_CFG));
    }
    else
        Arg.bxStatus = XTMSTS_STATE_ERROR;

    return( xtmReturnCmsCode(Arg.bxStatus) );
} /* devCtl_xtmGetConnCfg */


/***************************************************************************
 * Function Name: devCtl_xtmSetConnCfg
 * Description  : Saves the connection configuration record for the specified
 *                connection address.
 * Returns      : CmsRet return code
 ***************************************************************************/
CmsRet devCtl_xtmSetConnCfg( PXTM_ADDR pConnAddr, PXTM_CONN_CFG pConnCfg )
{
    XTMCFGDRV_CONN_CFG Arg;

    devCtl_xtmSetConnCfgTrace();

    memcpy(&Arg.ConnAddr, pConnAddr, sizeof(XTM_ADDR));
    if( pConnCfg )
        memcpy(&Arg.ConnCfg, pConnCfg, sizeof(XTM_CONN_CFG));
    else
        Arg.ConnCfg.ulAtmAalType = XTMCFG_INVALID_FIELD;
    Arg.bxStatus = XTMSTS_ERROR;

    if( g_nXtmFd == -1  )
        g_nXtmFd = OpenBcmXtmCfg();

    if( g_nXtmFd != -1  )
        ioctl( g_nXtmFd, XTMCFGIOCTL_SET_CONN_CFG, &Arg );
    else
        Arg.bxStatus = XTMSTS_STATE_ERROR;

    return( xtmReturnCmsCode(Arg.bxStatus) );
} /* devCtl_xtmSetConnCfg */


/***************************************************************************
 * Function Name: devCtl_xtmGetConnAddrs
 * Description  : Returns the configured connection addresses for an interface. 
 * Returns      : CmsRet return code
 ***************************************************************************/
CmsRet devCtl_xtmGetConnAddrs( PXTM_ADDR pConnAddrs, UINT32 *pulNumConns )
{
    XTMCFGDRV_CONN_ADDRS Arg = {pConnAddrs, *pulNumConns, XTMSTS_ERROR};

    devCtl_xtmGetConnAddrsTrace();

    if( g_nXtmFd == -1  )
        g_nXtmFd = OpenBcmXtmCfg();

    if( g_nXtmFd != -1  )
    {
        ioctl( g_nXtmFd, XTMCFGIOCTL_GET_CONN_ADDRS, &Arg );
        *pulNumConns = Arg.ulNumConns;
    }
    else
        Arg.bxStatus = XTMSTS_STATE_ERROR;

    return( xtmReturnCmsCode(Arg.bxStatus) );
} /* devCtl_xtmGetConnAddrs */


/***************************************************************************
 * Function Name: devCtl_xtmGetInterfaceStatistics
 * Description  : Returns the statistics record for an interface.
 * Returns      : CmsRet return code
 ***************************************************************************/
CmsRet devCtl_xtmGetInterfaceStatistics( UINT32 ulPortId,
    PXTM_INTERFACE_STATS pStatistics, UINT32 ulReset )
{
    XTMCFGDRV_INTERFACE_STATISTICS Arg;

    devCtl_xtmGetInterfaceStatisticsTrace();

    Arg.ulPortId = ulPortId;
    memcpy(&Arg.Statistics, pStatistics, sizeof(XTM_INTERFACE_STATS));
    Arg.bxStatus = XTMSTS_ERROR;
    Arg.ulReset = ulReset;

    if( g_nXtmFd == -1  )
        g_nXtmFd = OpenBcmXtmCfg();

    if( g_nXtmFd != -1  )
    {
        ioctl( g_nXtmFd, XTMCFGIOCTL_GET_INTERFACE_STATISTICS, &Arg );
        memcpy(pStatistics, &Arg.Statistics, sizeof(XTM_INTERFACE_STATS));
    }
    else
        Arg.bxStatus = XTMSTS_STATE_ERROR;

    return( xtmReturnCmsCode(Arg.bxStatus) );
} /* devCtl_xtmGetInterfaceStatistics */



/***************************************************************************
 * Function Name: devCtl_xtmGetErrorStatistics
 * Description  : Returns error statistics.
 * Returns      : CmsRet return code
 ***************************************************************************/
CmsRet devCtl_xtmGetErrorStatistics( PXTM_ERROR_STATS pErrStats )
{
    XTMCFGDRV_ERROR_STATISTICS Arg;

    devCtl_xtmGetErrorStatisticsTrace();

    memcpy(&Arg.ErrorStatistics, pErrStats, sizeof(XTM_ERROR_STATS));
    Arg.bxStatus = XTMSTS_ERROR;

    if( g_nXtmFd == -1  )
        g_nXtmFd = OpenBcmXtmCfg();

    if( g_nXtmFd != -1  )
    {
        ioctl( g_nXtmFd, XTMCFGIOCTL_GET_ERROR_STATISTICS, &Arg );
        memcpy(pErrStats, &Arg.ErrorStatistics, sizeof(XTM_ERROR_STATS));
    }
    else
        Arg.bxStatus = XTMSTS_STATE_ERROR;

    return( xtmReturnCmsCode(Arg.bxStatus) );

} /* devCtl_xtmGetErrorStatistics */















/***************************************************************************
 * Function Name: devCtl_xtmSetInterfaceLinkInfo
 * Description  : Calls the interface object for the specified interface id
 *                to set physical link information.
 * Returns      : CmsRet return code
 ***************************************************************************/
CmsRet devCtl_xtmSetInterfaceLinkInfo( UINT32 ulPortId,
    PXTM_INTERFACE_LINK_INFO pLi )
{
    XTMCFGDRV_INTERFACE_LINK_INFO Arg;

    devCtl_xtmSetInterfaceLinkInfoTrace();

    Arg.ulPortId = ulPortId;
    memcpy(&Arg.LinkInfo, pLi, sizeof(XTM_INTERFACE_LINK_INFO));
    Arg.bxStatus = XTMSTS_ERROR;

    if( g_nXtmFd == -1  )
        g_nXtmFd = OpenBcmXtmCfg();

    if( g_nXtmFd != -1  )
        ioctl( g_nXtmFd, XTMCFGIOCTL_SET_INTERFACE_LINK_INFO, &Arg );
    else
        Arg.bxStatus = XTMSTS_STATE_ERROR;

    return( xtmReturnCmsCode(Arg.bxStatus) );
} /* devCtl_xtmSetInterfaceLinkInfo */


/***************************************************************************
 * Function Name: devCtl_xtmSendOamCell
 * Description  : Sends an OAM request packet.
 * Returns      : CmsRet return code
 ***************************************************************************/
CmsRet devCtl_xtmSendOamCell( PXTM_ADDR pConnAddr,
    PXTM_OAM_CELL_INFO pOamCellInfo )
{
    XTMCFGDRV_SEND_OAM_CELL Arg;

    devCtl_xtmSendOamCellTrace();

    memcpy(&Arg.ConnAddr, pConnAddr, sizeof(XTM_ADDR));
    memcpy(&Arg.OamCellInfo, pOamCellInfo, sizeof(XTM_OAM_CELL_INFO));
    Arg.bxStatus = XTMSTS_ERROR;

    if( g_nXtmFd == -1  )
        g_nXtmFd = OpenBcmXtmCfg();

    if( g_nXtmFd != -1  )
    {
        ioctl( g_nXtmFd, XTMCFGIOCTL_SEND_OAM_CELL, &Arg );
        memcpy(pOamCellInfo, &Arg.OamCellInfo, sizeof(XTM_OAM_CELL_INFO));
    }
    else
        Arg.bxStatus = XTMSTS_STATE_ERROR;

    return( xtmReturnCmsCode(Arg.bxStatus) );
} /* devCtl_xtmSendOamCell */


/***************************************************************************
 * Function Name: devCtl_xtmCreateNetworkDevice
 * Description  : Calls the bcmxtmcfg driver to create an bcmxtmrt Linux
 *                network device instance.
 * Returns      : CmsRet return code
 ***************************************************************************/
CmsRet devCtl_xtmCreateNetworkDevice( PXTM_ADDR pConnAddr,
    char *pszNetworkDeviceName )
{
    XTMCFGDRV_CREATE_NETWORK_DEVICE Arg;

    devCtl_xtmCreateNetworkDeviceTrace();

    memcpy(&Arg.ConnAddr, pConnAddr, sizeof(XTM_ADDR));
    memcpy(&Arg.szNetworkDeviceName, pszNetworkDeviceName,
        sizeof(Arg.szNetworkDeviceName));
    Arg.bxStatus = XTMSTS_ERROR;

    if( g_nXtmFd == -1  )
        g_nXtmFd = OpenBcmXtmCfg();

    if( g_nXtmFd != -1  )
        ioctl( g_nXtmFd, XTMCFGIOCTL_CREATE_NETWORK_DEVICE, &Arg );
    else
        Arg.bxStatus = XTMSTS_STATE_ERROR;

    return( xtmReturnCmsCode(Arg.bxStatus) );
} /* devCtl_xtmCreateNetworkDevice */


/***************************************************************************
 * Function Name: devCtl_xtmDeleteNetworkDevice
 * Description  : Calls the interface object for the specified interface id
 *                to set physical link information.
 * Returns      : CmsRet return code
 ***************************************************************************/
CmsRet devCtl_xtmDeleteNetworkDevice( PXTM_ADDR pConnAddr )
{
    XTMCFGDRV_DELETE_NETWORK_DEVICE Arg;

    devCtl_xtmDeleteNetworkDeviceTrace();

    memcpy(&Arg.ConnAddr, pConnAddr, sizeof(XTM_ADDR));
    Arg.bxStatus = XTMSTS_ERROR;

    if( g_nXtmFd == -1  )
        g_nXtmFd = OpenBcmXtmCfg();

    if( g_nXtmFd != -1  )
        ioctl( g_nXtmFd, XTMCFGIOCTL_DELETE_NETWORK_DEVICE, &Arg );
    else
        Arg.bxStatus = XTMSTS_STATE_ERROR;

    return( xtmReturnCmsCode(Arg.bxStatus) );
} /* devCtl_xtmDeleteNetworkDevice */

/***************************************************************************
 * Function Name: devCtl_xtmGetBondingInfo
 * Description  : Calls & Retrieves XTM traffic status with respect to bonding
 *                non-bonding operations are involved.
 * Returns      : CmsRet return code
 ***************************************************************************/
CmsRet devCtl_xtmGetBondingInfo ( PXTM_BOND_INFO pBondInfo )
{
    XTMCFGDRV_BONDING_INFO Arg;

    devCtl_xtmGetBondingInfoTrace();

    memcpy(&Arg.info, pBondInfo, sizeof(XTM_BOND_INFO));
    Arg.bxStatus = XTMSTS_ERROR;

    if( g_nXtmFd == -1  )
        g_nXtmFd = OpenBcmXtmCfg();

    if( g_nXtmFd != -1  )
    {
        ioctl( g_nXtmFd, XTMCFGIOCTL_GET_BONDING_INFO, &Arg );
        memcpy(pBondInfo, &Arg.info, sizeof(XTM_BOND_INFO));
    }
    else
        Arg.bxStatus = XTMSTS_STATE_ERROR;

    return( xtmReturnCmsCode(Arg.bxStatus) );
} /* devCtl_xtmGetBondingInfo */



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
CmsRet devCtl_atmSendOamLoopbackTest(UINT32 type, PATM_ADDR pVccAddr,
    UINT32 repetition, UINT32 timeout, ATMDRV_OAM_LOOPBACK *results)
{
    CmsRet Ret;
    UINT8 ucCts[] = {CTYPE_OAM_F5_SEGMENT, CTYPE_OAM_F5_END_TO_END,
        CTYPE_OAM_F4_SEGMENT, CTYPE_OAM_F4_END_TO_END};
    XTM_OAM_CELL_INFO XtmOamCellInfo;
    XTM_ADDR Addr;

    devCtl_atmSendOamLoopbackTestTrace();

    memset((UINT8 *) &XtmOamCellInfo, 0x00, sizeof(XtmOamCellInfo));
    memset((UINT8 *) &Addr, 0x00, sizeof(Addr));

    Addr.ulTrafficType = TRAFFIC_TYPE_ATM;
    Addr.u.Vcc.ulPortMask = PORT_TO_PORTID(pVccAddr->ulPortMask);
    Addr.u.Vcc.usVpi = pVccAddr->usVpi;
    Addr.u.Vcc.usVci = pVccAddr->usVci;

    XtmOamCellInfo.ucCircuitType = ucCts[type];
    XtmOamCellInfo.ulTimeout = timeout;
    XtmOamCellInfo.ulRepetition = repetition;

    Ret = devCtl_xtmSendOamCell( &Addr, &XtmOamCellInfo );

    memcpy(&results->VccAddr, pVccAddr, sizeof(ATM_ADDR));
    results->type = type;
    results->baStatus = (Ret == CMSRET_SUCCESS) ? 0 : 1;
    results->repetition = repetition;
    results->timeout = timeout;
    results->sent = XtmOamCellInfo.ulSent;
    results->received = XtmOamCellInfo.ulReceived;
    results->minResponseTime = XtmOamCellInfo.ulMinRspTime;
    results->maxResponseTime = XtmOamCellInfo.ulMaxRspTime;
    results->avgResponseTime = XtmOamCellInfo.ulAvgRspTime;

    return( Ret );
} /* devCtl_atmSendOamLoopbackTest */

