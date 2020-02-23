/*
    <:copyright-BRCM:2011:proprietary:standard
    
       Copyright (c) 2011 Broadcom 
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
/***************************************************************************
 * File Name  : xtmcfgmain.cpp (impl2)
 *
 * Description: This file contains the implementation for the XTM kernel entry
 *              point functions.
 ***************************************************************************/

/* Includes. */
#include "xtmcfgimpl.h"
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,2,0)
#include "bcm_intr.h"
#endif


/* Globals. */
static XTM_PROCESSOR *g_pXtmProcessor = NULL;


/* Prototypes. */
extern "C" {
BCMXTM_STATUS BcmXtm_ChipInitialize( PXTM_INITIALIZATION_PARMS pInitParms,
    UINT32 *pulBusSpeed );
}


/***************************************************************************
 * Function Name: BcmXtm_Initialize
 * Description  : ATM/PTM processor initialization entry point function.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
extern "C"
BCMXTM_STATUS BcmXtm_Initialize( PXTM_INITIALIZATION_PARMS pInitParms )
{
    BCMXTM_STATUS bxStatus;

    //XtmOsPrintf("%s:Enter\n",__FUNCTION__);
    if( g_pXtmProcessor == NULL )
    {
        XtmOsInitialize();
        g_pXtmProcessor = (XTM_PROCESSOR *)XtmOsAlloc(sizeof(XTM_PROCESSOR));

        if( g_pXtmProcessor != NULL )
        {
            UINT32 ulBusSpeed = 0;
            //g_pXtmProcessor->XTM_PROCESSOR();
            memset(g_pXtmProcessor, 0x00, sizeof(XTM_PROCESSOR));
            g_pXtmProcessor->Cxtor();

            bxStatus = BcmXtm_ChipInitialize( pInitParms, &ulBusSpeed );
            if( bxStatus == XTMSTS_SUCCESS )
            {
                if( (bxStatus = g_pXtmProcessor->Initialize( pInitParms,
                    bcmxtmrt_request )) != XTMSTS_SUCCESS )
                {
                    BcmXtm_Uninitialize();
                }
            }
        }
        else
            bxStatus = XTMSTS_ALLOC_ERROR;
    }
    else
#if defined(CONFIG_BCM_55153_DPU)
        //For DPU by default we have already initialized with default ptm
        //interface. So if there comes initialize ioctls from userspace return
        //success.
        bxStatus = XTMSTS_SUCCESS;
#else
        bxStatus = XTMSTS_STATE_ERROR;
#endif

    return( bxStatus );
} /* BcmXtm_Initialize */

#if defined(CONFIG_BCM_55153_DPU)
/***************************************************************************
 * Function Name: BcmXtm_Initialize_DPU
 * Description  : Initialize the SAR for GFAST DPU mode and create static
                  device with ptm0.  
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
extern "C"
BCMXTM_STATUS BcmXtm_Initialize_DPU( PXTM_INITIALIZATION_PARMS pInitParms )
{
    BCMXTM_STATUS bxStatus;

    if( g_pXtmProcessor == NULL )
    {
        XtmOsInitialize();
        g_pXtmProcessor = (XTM_PROCESSOR *)XtmOsAlloc(sizeof(XTM_PROCESSOR));

        if( g_pXtmProcessor != NULL )
        {
            UINT32 ulBusSpeed = 0;
            memset(g_pXtmProcessor, 0x00, sizeof(XTM_PROCESSOR));
            g_pXtmProcessor->Cxtor();

            bxStatus = BcmXtm_ChipInitialize( pInitParms, &ulBusSpeed );
            if( bxStatus == XTMSTS_SUCCESS )
            {
                if( (bxStatus = g_pXtmProcessor->Initialize( pInitParms,
                    bcmxtmrt_request )) != XTMSTS_SUCCESS )
                {
                    BcmXtm_Uninitialize();
                }
                else
                {
                    // Create default connection configuration.
                    XTM_ADDR Addr;
                    XTM_CONN_CFG ConnCfg;
                    PXTM_TRANSMIT_QUEUE_PARMS pTxQ;
                    char DeviceName[NETWORK_DEVICE_NAME_SIZE];
                    XtmOsPrintf("DPU Profile Need to create PTM Interface here\n");
                    memset(DeviceName,0,sizeof(DeviceName));
                    memset(&Addr, 0x0, sizeof(Addr));
                    memset(&ConnCfg, 0x0, sizeof(ConnCfg));
                    Addr.ulTrafficType = TRAFFIC_TYPE_PTM;
                    DeviceName[0] = 'p';
                    DeviceName[1] = 't';
                    DeviceName[2] = 'm';
                    DeviceName[3] = '0';
                    //Addr.u.Flow.ulPortMask = PORTID_TO_PORTMASK(PHY0_PATH0);
                    Addr.u.Flow.ulPortMask = PORT_PHY0_FAST;
                    Addr.u.Flow.ulPtmPriority = PTM_PRI_HIGH | PTM_PRI_LOW;
                    ConnCfg.ulHeaderType = HT_LLC_SNAP_ETHERNET;
                    ConnCfg.ulAdminStatus = ADMSTS_UP;
                    ConnCfg.ulTransmitQParmsSize = 1;
                    ConnCfg.ConnArbs[0][0].ulWeightAlg = WA_CWRR;
                    ConnCfg.ConnArbs[0][0].ulWeightValue = 1;
                    ConnCfg.ConnArbs[0][0].ulSubPriority = 0;
                    pTxQ = &ConnCfg.TransmitQParms[0];
                    pTxQ->usSize         = 400;
                    pTxQ->ucWeightAlg    = WA_CWRR;
                    pTxQ->ulWeightValue  = 1;
                    pTxQ->ucSubPriority  = 0;
                    pTxQ->ucQosQId       = 0;
                    pTxQ->ucDropAlg      = WA_DT;
                    pTxQ->ucLoMinThresh  = 0;
                    pTxQ->ucLoMaxThresh  = 0;
                    pTxQ->ucHiMinThresh  = 0;
                    pTxQ->ucHiMaxThresh  = 0;
                    pTxQ->ulMinBitRate   = 0;
                    pTxQ->ulShapingRate  = 0;
                    pTxQ->ulPortId       = PORT_PHY0_PATH0;
                    pTxQ->ulPtmPriority  = PTM_PRI_LOW;
                    bxStatus = g_pXtmProcessor->SetConnCfg(&Addr,&ConnCfg);
                    // call create device.
                    if(bxStatus == XTMSTS_SUCCESS)
                    {
                       bxStatus = g_pXtmProcessor->CreateNetworkDevice(&Addr,DeviceName);
                    }
                }
            }
        }
        else
            bxStatus = XTMSTS_ALLOC_ERROR;
    }
    else
        bxStatus = XTMSTS_STATE_ERROR;

    return( bxStatus );
} /* BcmXtm_Initialize_DPU */
#endif   //CONFIG_BCM_55153_DPU

/***************************************************************************
 * Function Name: BcmXtm_Uninitialize
 * Description  : ATM/PTM processor uninitialization entry point function.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
extern "C"
BCMXTM_STATUS BcmXtm_Uninitialize( void )
{
    XTM_PROCESSOR *pXtmProc = g_pXtmProcessor;

    //XtmOsPrintf("%s:Enter\n",__FUNCTION__);
    g_pXtmProcessor = NULL;

    if( pXtmProc ) {
        //delete pXtmProc;
        pXtmProc->Dxtor();
        XtmOsFree( (char *)pXtmProc );
        pXtmProc = NULL;
    }

    return( XTMSTS_SUCCESS );
} /* BcmXtm_Uninitialize */

/***************************************************************************
 * Function Name: BcmXtm_Configure
 * Description  : ATM/PTM processor configuration entry point function.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
extern "C"
BCMXTM_STATUS BcmXtm_Configure( PXTM_CONFIGURATION_PARMS pConfigParms )
{
    BCMXTM_STATUS bxStatus;

    //XtmOsPrintf("%s:Enter\n",__FUNCTION__);
    if( g_pXtmProcessor != NULL )
    {
        /* Pass configuration data to XtmProcessor */
        bxStatus = g_pXtmProcessor->Configure(pConfigParms);
    }
    else
        bxStatus = XTMSTS_STATE_ERROR;

    return( bxStatus );
} /* BcmXtm_Configure */


/***************************************************************************
 * Function Name: BcmXtm_ManageThreshold
 * Description  : ATM/PTM processor Manage Q Threshold entry point function.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
extern "C"
BCMXTM_STATUS BcmXtm_ManageThreshold( PXTM_THRESHOLD_PARMS pThresholdParams )
{
    BCMXTM_STATUS bxStatus;

    //XtmOsPrintf("%s:Enter\n",__FUNCTION__);
    if( g_pXtmProcessor != NULL )
    {
        /* Pass threshold data to XtmProcessor */
        bxStatus = g_pXtmProcessor->ManageThreshold (pThresholdParams);
    }
    else
        bxStatus = XTMSTS_STATE_ERROR;

    return( bxStatus );

} /* BcmXtm_ManageThreshold */


/***************************************************************************
 * Function Name: BcmXtm_GetTrafficDescrTable
 * Description  : 
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
extern "C"
BCMXTM_STATUS BcmXtm_GetTrafficDescrTable( PXTM_TRAFFIC_DESCR_PARM_ENTRY
    pTrafficDescrTable, UINT32 *pulTrafficDescrTableSize )
{
    //XtmOsPrintf("%s:Enter\n",__FUNCTION__);
    return( (g_pXtmProcessor)
        ? g_pXtmProcessor->GetTrafficDescrTable( pTrafficDescrTable,
            pulTrafficDescrTableSize )
        : XTMSTS_STATE_ERROR );
} /* BcmXtm_GetTrafficDescrTable */


/***************************************************************************
 * Function Name: BcmXtm_SetTrafficDescrTable
 * Description  : 
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
extern "C"
BCMXTM_STATUS BcmXtm_SetTrafficDescrTable( PXTM_TRAFFIC_DESCR_PARM_ENTRY
    pTrafficDescrTable, UINT32  ulTrafficDescrTableSize )
{
    //XtmOsPrintf("%s:Enter\n",__FUNCTION__);
    return( (g_pXtmProcessor)
        ? g_pXtmProcessor->SetTrafficDescrTable( pTrafficDescrTable,
            ulTrafficDescrTableSize )
        : XTMSTS_STATE_ERROR );
} /* BcmXtm_SetTrafficDescrTable */


/***************************************************************************
 * Function Name: BcmXtm_GetInterfaceCfg
 * Description  : 
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
extern "C"
BCMXTM_STATUS BcmXtm_GetInterfaceCfg( UINT32 ulPortId, PXTM_INTERFACE_CFG
    pInterfaceCfg )
{
    //XtmOsPrintf("%s:Enter g_pXtmProcessor [%pk] \n",__FUNCTION__,g_pXtmProcessor);
#if (1) 
    return( (g_pXtmProcessor)
        ? g_pXtmProcessor->GetInterfaceCfg( ulPortId, pInterfaceCfg )
        : XTMSTS_STATE_ERROR );
#else
    return XTMSTS_STATE_ERROR;
#endif
} /* BcmXtm_GetInterfaceCfg */


/***************************************************************************
 * Function Name: BcmXtm_SetInterfaceCfg
 * Description  : 
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
extern "C"
BCMXTM_STATUS BcmXtm_SetInterfaceCfg( UINT32 ulPortId, PXTM_INTERFACE_CFG
    pInterfaceCfg )
{
    //XtmOsPrintf("%s:Enter\n",__FUNCTION__);
    return( (g_pXtmProcessor)
        ? g_pXtmProcessor->SetInterfaceCfg( ulPortId, pInterfaceCfg )
        : XTMSTS_STATE_ERROR );
} /* BcmXtm_SetInterfaceCfg */


/***************************************************************************
 * Function Name: BcmXtm_GetConnCfg
 * Description  : 
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
extern "C"
BCMXTM_STATUS BcmXtm_GetConnCfg( PXTM_ADDR pConnAddr, PXTM_CONN_CFG pConnCfg )
{
    //XtmOsPrintf("%s:Enter\n",__FUNCTION__);
    return( (g_pXtmProcessor)
        ? g_pXtmProcessor->GetConnCfg( pConnAddr, pConnCfg )
        : XTMSTS_STATE_ERROR );
} /* BcmXtm_GetConnCfg */


/***************************************************************************
 * Function Name: BcmXtm_SetConnCfg
 * Description  : 
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
extern "C"
BCMXTM_STATUS BcmXtm_SetConnCfg( PXTM_ADDR pConnAddr, PXTM_CONN_CFG pConnCfg )
{
    //XtmOsPrintf("%s:Enter\n",__FUNCTION__);
    return( (g_pXtmProcessor)
        ? g_pXtmProcessor->SetConnCfg( pConnAddr, pConnCfg )
        : XTMSTS_STATE_ERROR );
} /* BcmXtm_SetConnCfg */


/***************************************************************************
 * Function Name: BcmXtm_GetConnAddrs
 * Description  : 
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
extern "C"
BCMXTM_STATUS BcmXtm_GetConnAddrs( PXTM_ADDR pConnAddrs, UINT32 *pulNumConns )
{
    //XtmOsPrintf("%s:Enter\n",__FUNCTION__);
    return( (g_pXtmProcessor)
        ? g_pXtmProcessor->GetConnAddrs( pConnAddrs, pulNumConns )
        : XTMSTS_STATE_ERROR );
} /* BcmXtm_GetConnAddrs */


/***************************************************************************
 * Function Name: BcmXtm_GetInterfaceStatistics
 * Description  : 
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
extern "C"
BCMXTM_STATUS BcmXtm_GetInterfaceStatistics( UINT32 ulPortId,
    PXTM_INTERFACE_STATS pStatistics, UINT32 ulReset )
{
    //XtmOsPrintf("%s:Enter\n",__FUNCTION__);
    return( (g_pXtmProcessor)
        ? g_pXtmProcessor->GetInterfaceStatistics(ulPortId, pStatistics, ulReset)
        : XTMSTS_STATE_ERROR );
} /* BcmXtm_GetInterfaceStatistics */


/***************************************************************************
 * Function Name: BcmXtm_GetErrorStatistics
 * Description  : 
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
extern "C"
BCMXTM_STATUS BcmXtm_GetErrorStatistics( PXTM_ERROR_STATS pStatistics )
{
    //XtmOsPrintf("%s:Enter\n",__FUNCTION__);
    return( (g_pXtmProcessor)
        ? g_pXtmProcessor->GetErrorStatistics(pStatistics)
        : XTMSTS_STATE_ERROR );
} /* BcmXtm_GetInterfaceStatistics */

/***************************************************************************
 * Function Name: BcmXtm_SetInterfaceLinkInfo
 * Description  : 
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
extern "C"
BCMXTM_STATUS BcmXtm_SetInterfaceLinkInfo( UINT32 ulPortId,
    PXTM_INTERFACE_LINK_INFO pLinkInfo )
{
    BCMXTM_STATUS bxStatus = XTMSTS_STATE_ERROR;

    //XtmOsPrintf("%s:Enter\n",__FUNCTION__);
    if( g_pXtmProcessor )
    {
        bxStatus = g_pXtmProcessor->SetInterfaceLinkInfo(ulPortId, pLinkInfo, 0); 
        /* If all the available link(s) are down, the SAR will have to be reset
         * to accommodate possible different traffic types.
         */
        g_pXtmProcessor->CheckAndResetSAR(ulPortId, pLinkInfo);
    }

    return( bxStatus );
} /* BcmXtm_SetInterfaceLinkInfo */


/***************************************************************************
 * Function Name: BcmAtm_SetInterfaceLinkInfo
 * Description  : Backward compatibility function.
 * Returns      : STS_SUCCESS if successful or error status.
 ***************************************************************************/

typedef struct AtmInterfaceLinkInfo
{
    UINT32 ulStructureId;
    UINT32 ulLinkState;
    UINT32 ulLineRate;
    UINT32 ulReserved[2];
} ATM_INTERFACE_LINK_INFO, *PATM_INTERFACE_LINK_INFO;

extern "C"
BCMXTM_STATUS BcmAtm_SetInterfaceLinkInfo( UINT32 ulInterfaceId,
    PATM_INTERFACE_LINK_INFO pInterfaceLinkInfo )
{
    XTM_INTERFACE_LINK_INFO LinkInfo;

    XtmOsPrintf("%s:Enter\n",__FUNCTION__);
    LinkInfo.ulLinkState = pInterfaceLinkInfo->ulLinkState;
    LinkInfo.ulLinkUsRate = pInterfaceLinkInfo->ulLineRate;
    LinkInfo.ulLinkDsRate = pInterfaceLinkInfo->ulLineRate;
    LinkInfo.ulLinkTrafficType = TRAFFIC_TYPE_ATM;

    return( BcmXtm_SetInterfaceLinkInfo( PORT_TO_PORTID(ulInterfaceId),
        &LinkInfo ) );
} /* BcmAtm_SetInterfaceLinkInfo */

extern "C"
BCMXTM_STATUS BcmAtm_GetInterfaceId( UINT8 ucPhyPort, UINT32 *pulInterfaceId )
{
    //XtmOsPrintf("%s:Enter\n",__FUNCTION__);
    *pulInterfaceId = 0;
    return( XTMSTS_SUCCESS );
} // BcmAtm_GetInterfaceId

/***************************************************************************
 * Function Name: BcmXtm_SendOamCell
 * Description  : 
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
extern "C"
BCMXTM_STATUS BcmXtm_SendOamCell( PXTM_ADDR pConnAddr,
    PXTM_OAM_CELL_INFO pOamCellInfo)
{
    //XtmOsPrintf("%s:Enter\n",__FUNCTION__);
    return( (g_pXtmProcessor)
        ? g_pXtmProcessor->SendOamCell( pConnAddr, pOamCellInfo )
        : XTMSTS_STATE_ERROR );
} /* BcmXtm_SendOamCell */


/***************************************************************************
 * Function Name: BcmXtm_CreateNetworkDevice
 * Description  : 
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
extern "C"
BCMXTM_STATUS BcmXtm_CreateNetworkDevice( PXTM_ADDR pConnAddr,
    char *pszNetworkDeviceName )
{
    //XtmOsPrintf("%s:Enter\n",__FUNCTION__);
    return( (g_pXtmProcessor)
        ? g_pXtmProcessor->CreateNetworkDevice( pConnAddr,
            pszNetworkDeviceName )
        : XTMSTS_STATE_ERROR );
} /* BcmXtm_CreateNetworkDevice */


/***************************************************************************
 * Function Name: BcmXtm_DeleteNetworkDevice
 * Description  : 
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
extern "C"
BCMXTM_STATUS BcmXtm_DeleteNetworkDevice( PXTM_ADDR pConnAddr )
{
    //XtmOsPrintf("%s:Enter\n",__FUNCTION__);
    return( (g_pXtmProcessor)
        ? g_pXtmProcessor->DeleteNetworkDevice( pConnAddr )
        : XTMSTS_STATE_ERROR );
} /* BcmXtm_DeleteNetworkDevice */

/***************************************************************************
 * Function Name: BcmXtm_GetBondingInfo
 * Description  : If bonding enabled, return success with info.
 *                If non-bonding, return Non-Success message.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
extern "C"
BCMXTM_STATUS BcmXtm_GetBondingInfo ( PXTM_BOND_INFO pBondingInfo)
{
    //XtmOsPrintf("%s:Enter\n",__FUNCTION__);
    return( (g_pXtmProcessor)
        ? g_pXtmProcessor->GetBondingInfo( pBondingInfo )
        : XTMSTS_STATE_ERROR ) ;
} /* BcmXtm_GetBondingInfo */


/***************************************************************************
 * Function Name: BcmXtm_ChipInitialization (63268/6318/63138/63381/63148/63158)
 * Description  : Chip specific initialization.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
extern "C"
BCMXTM_STATUS BcmXtm_ChipInitialize( PXTM_INITIALIZATION_PARMS pInitParms,
    UINT32 *pulBusSpeed )
{
    //XtmOsPrintf("%s:Enter\n",__FUNCTION__);
    return( XTMSTS_SUCCESS );
}  /* BcmXtm_ChipInitialize */

