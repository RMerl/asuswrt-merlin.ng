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
 * File Name  : xtmconnection.cpp (impl2)
 *
 * Description: This file contains the implementation for the XTM connection
 *              and connection table classes.
 ***************************************************************************/

#include "xtmcfgimpl.h"
#include "bcmadsl.h"

#if defined(ADSLMIBDEF_H_VER) && (ADSLMIBDEF_H_VER  >  0)
typedef int (*FN_ADSL_GET_OBJECT_VALUE) (unsigned char lineId, char *objId,
                                         int objIdLen, char *dataBuf, long *dataBufLen);
typedef void (*FN_ADSL_WAN_DEV_NOTIFY) (unsigned char bDevUp);


static adslMibInfo PhyMibInfo ;

extern "C" {

extern FN_ADSL_GET_OBJECT_VALUE g_pfnAdslGetObjValue ;
extern FN_ADSL_WAN_DEV_NOTIFY g_pfnAdslWanDevState ;
}
#endif

UINT32 XTM_CONNECTION::ms_ulSitUt   = ADSL_SIT_UNIT_OF_TIME;
UINT32 XTM_CONNECTION::ms_ulSitLoUt = ADSL_SIT_UNIT_OF_TIME * 32;
UINT32 XTM_CONNECTION::ms_ulTxQueuesInUse = 0;

/* CDVT for SCR token bucket in number of cells.
 * CDVT=8 should cover cell transmission delay caused by up to 8 shaped PVCs
 * of the same service class (priority).
 * i.e., the peak cell rate of multiple CBRs or UBRWPCRs, or the sustainable
 * cell rate of multiple VBRs can be maintained.
 */
#define CDVT 8   /* 8 cells */

/***************************************************************************
 * Function Name: XTM_CONNECTION
 * Description  : Constructor for the XTM connection class.
 * Returns      : None.
 ***************************************************************************/
int XTM_CONNECTION::Cxtor( FN_XTMRT_REQ pfnXtmrtReq,
    XTM_CONNECTION_TABLE *pConnTable, UINT32 ulConnSem,
    UINT32 *pulRxVpiVciCamShadow, XtmBondConfig bondConfig)
{
   //XtmOsPrintf("%s:Enter\n",__FUNCTION__);
    m_pfnXtmrtReq = pfnXtmrtReq;
    m_pConnTable = pConnTable;
    m_ulConnSem = ulConnSem;
    m_pulRxVpiVciCamShadow = pulRxVpiVciCamShadow;
    m_hDev = INVALID_HANDLE;
    m_ulCreated = 0;
    m_ulConnected = 0;
    m_ulRemovePending = 0;
    m_ulUpdatePending = 0;
    m_ulTxQIdsSize = 0;
    m_pTrafficDescrEntries = NULL;
    m_ulNumTrafficDescrEntries = 0;
    m_BondConfig.uConfig = bondConfig.uConfig ;
    memset(m_szNetDeviceName, 0x00, sizeof(m_szNetDeviceName));
    memset(&m_Addr, 0x00, sizeof(m_Addr));
    memset(&m_Cfg, 0x00, sizeof(m_Cfg));
    memset(m_ucVcids, INVALID_VCID, sizeof(m_ucVcids));
    memset(m_TxQIds, 0x00, sizeof(m_TxQIds));
    memset(m_VcidInfo, 0x00, sizeof(m_VcidInfo));
    for( UINT32 i = 0; i < MAX_VCIDS; i++ ) {
        m_VcidInfo[i].ulMpIdx = m_VcidInfo[i].ulQueueIdx = INVALID_INDEX;
        m_VcidInfo[i].ulBondingPort  = MAX_PHY_PORTS;
    }

    m_ulHwAddHdr = 1;
    m_ulHwRemoveHdr = 1;
    m_ulXTMLinkMode = XTM_LINK_MODE_UNKNOWN ;
    m_ulXTMNitroMode = XTM_MODE_NITRO_DISABLE ;

    return 0;
} /* XTM_CONNECTION */


/***************************************************************************
 * Function Name: ~XTM_CONNECTION
 * Description  : Destructor for the XTM connection class.
 * Returns      : None.
 ***************************************************************************/
int XTM_CONNECTION::Dxtor( void )
{
   //XtmOsPrintf("%s:Enter\n",__FUNCTION__);
    DeleteNetworkDevice();
    return 0;
} /* ~XTM_CONNECTION */


/***************************************************************************
 * Function Name: Initialize
 * Description  : Similar to Constructor for the XTM connection class.
 * Returns      : None.
 ***************************************************************************/
void XTM_CONNECTION::Initialize( FN_XTMRT_REQ pfnXtmrtReq,
    XTM_CONNECTION_TABLE *pConnTable, void *ulConnSem,
    UINT32 *pulRxVpiVciCamShadow, XtmBondConfig bondConfig)
{
   //XtmOsPrintf("%s:Enter\n",__FUNCTION__);
    m_pfnXtmrtReq = pfnXtmrtReq;
    m_pConnTable = pConnTable;
#if defined(CONFIG_ARM64)
    m_ulConnSem = (unsigned long)ulConnSem;
#else
    m_ulConnSem = (UINT32)ulConnSem;
#endif
    m_pulRxVpiVciCamShadow = pulRxVpiVciCamShadow;
    m_hDev = INVALID_HANDLE;
    m_ulCreated = 0;
    m_ulConnected = 0;
    m_ulRemovePending = 0;
    m_ulUpdatePending = 0;
    m_ulTxQIdsSize = 0;
    m_pTrafficDescrEntries = NULL;
    m_ulNumTrafficDescrEntries = 0;
    m_BondConfig.uConfig = bondConfig.uConfig ;
    memset(m_szNetDeviceName, 0x00, sizeof(m_szNetDeviceName));
    memset(&m_Addr, 0x00, sizeof(m_Addr));
    memset(&m_Cfg, 0x00, sizeof(m_Cfg));
    memset(m_ucVcids, INVALID_VCID, sizeof(m_ucVcids));
    memset(m_TxQIds, 0x00, sizeof(m_TxQIds));
    memset(m_VcidInfo, 0x00, sizeof(m_VcidInfo));
    for( UINT32 i = 0; i < MAX_VCIDS; i++ ) {
        m_VcidInfo[i].ulMpIdx = m_VcidInfo[i].ulQueueIdx = INVALID_INDEX;
        m_VcidInfo[i].ulBondingPort  = MAX_PHY_PORTS;
    }

    m_ulHwAddHdr = 1;
    m_ulHwRemoveHdr = 1;
    m_ulXTMLinkMode = XTM_LINK_MODE_UNKNOWN ;
    m_ulXTMNitroMode = XTM_MODE_NITRO_DISABLE ;
} /* Initialize */

/***************************************************************************
 * Function Name: GetAddr
 * Description  : Returns the connection address.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_CONNECTION::GetAddr( PXTM_ADDR pAddr )
{
    //XtmOsPrintf("%s:Enter\n",__FUNCTION__);
    memcpy(pAddr, &m_Addr, sizeof(m_Addr));
    return( XTMSTS_SUCCESS );
} /* GetAddr */


/***************************************************************************
 * Function Name: SetAddr
 * Description  : Sets the connection address.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_CONNECTION::SetAddr( PXTM_ADDR pAddr )
{
   //XtmOsPrintf("%s:Enter\n",__FUNCTION__);
    memcpy(&m_Addr, pAddr, sizeof(m_Addr));
    return( XTMSTS_SUCCESS );
} /* SetAddr */


/***************************************************************************
 * Function Name: GetCfg
 * Description  : Returns the connection's configuration.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_CONNECTION::GetCfg( PXTM_CONN_CFG pCfg )
{
   //XtmOsPrintf("%s:Enter\n",__FUNCTION__);
    memcpy(pCfg, &m_Cfg, sizeof(m_Cfg));
    pCfg->ulOperStatus = OPRSTS_DOWN;

    /* The operational status of the connection depends on the link status and
     * bcmxtmrt device state.
     */
    if( m_ulConnected && m_Cfg.ulAdminStatus == ADMSTS_UP )
    {
        UINT32 ulOpenState = XTMRT_DEV_CLOSED;
        //XtmOsPrintf("%s:calling XTMRT_CMD_GET_DEVICE_STATE\n");
        (*m_pfnXtmrtReq) (m_hDev, XTMRT_CMD_GET_DEVICE_STATE, &ulOpenState);
        if( ulOpenState == XTMRT_DEV_OPENED )
            pCfg->ulOperStatus = OPRSTS_UP;
    }

    return( XTMSTS_SUCCESS );
} /* GetCfg */


/***************************************************************************
 * Function Name: GetTxQSize
 * Description  : Maps the XTM link mode to the corresponding default Q threshold.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
UINT32 XTM_CONNECTION::GetTxQSize ( )
{
   UINT32 ulQSize = 0 ;

   switch (m_ulXTMLinkMode) {

      case XTM_LINK_MODE_ADSL      :
         ulQSize =  m_pConnTable->m_ulAdslThreshold ;
         break;
      case XTM_LINK_MODE_VDSL      :
         ulQSize =  m_pConnTable->m_ulVdslThreshold ;
         break;
      case XTM_LINK_MODE_VDSL_RTX  :
         ulQSize =  m_pConnTable->m_ulVdslRtxThreshold ;
         break;
      case XTM_LINK_MODE_GFAST     :
         ulQSize =  m_pConnTable->m_ulGfastThreshold ;
         break;
      default                      :
         ulQSize =  0 ;
         break;
   }

   return (ulQSize) ;
}

/***************************************************************************
 * Function Name: SetCfg
 * Description  : Sets the connection's configuration.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_CONNECTION::SetCfg( PXTM_CONN_CFG pCfg, UINT32 ulXTMLinkMode, UINT32 ulXTMNitroMode )
{
    BCMXTM_STATUS bxStatus = XTMSTS_SUCCESS;

   //XtmOsPrintf("%s:Enter\n",__FUNCTION__);
    /* A NULL configuration means the configuration is being deleted. */
    m_ulXTMLinkMode = ulXTMLinkMode ;
    m_ulXTMNitroMode = ulXTMNitroMode ;

    if( pCfg )
    {
        /* Validate transmit queue port id parameter. */
        /* Also set the queue size for 138/148 platforms to drop threshold */
        for( UINT32 i = 0; i < pCfg->ulTransmitQParmsSize; i++ )
        {
            UINT32 ulPid = pCfg->TransmitQParms[i].ulPortId;
            UINT32 ulPortMask = m_Addr.u.Conn.ulPortMask ;
            if( (ulPid & ulPortMask) == 0 )
            {
      XtmOsPrintf ("bcmxtmcfg: ulPid =%x, ulPortMask=%x \n", ulPid, ulPortMask) ;
                bxStatus = XTMSTS_PARAMETER_ERROR;
                break;
            }
#if defined(CONFIG_BCM963148) || defined(CONFIG_BCM963138) || defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178) 
            pCfg->TransmitQParms[i].usSize = GetTxQSize () ;
            MapXtmOsPrintf ("Q Size = %d \n", pCfg->TransmitQParms[i].usSize) ;
#endif
        }

        if (bxStatus == XTMSTS_SUCCESS)
        {
            if(( m_Addr.ulTrafficType == TRAFFIC_TYPE_PTM ) ||
               ( m_Addr.ulTrafficType == TRAFFIC_TYPE_PTM_BONDED))
            {
                /* Validate PTM priority parameter. */
                for( UINT32 i = 0; i < pCfg->ulTransmitQParmsSize; i++ )
                {
                    UINT32 ulPtmPri = pCfg->TransmitQParms[i].ulPtmPriority;
                    if( (ulPtmPri & m_Addr.u.Flow.ulPtmPriority) == 0 ||
                         ulPtmPri == (PTM_PRI_LOW | PTM_PRI_HIGH) )
                    {
                        bxStatus = XTMSTS_PARAMETER_ERROR;
                        break;
                    }
                }
            }
        }

        if( bxStatus == XTMSTS_SUCCESS )
        {
            UINT32 AdminStatus;
            /*If admin status is not set to a valid value, set it to enabled.*/
            if( pCfg->ulAdminStatus != ADMSTS_DOWN )
                pCfg->ulAdminStatus = ADMSTS_UP;

            if( m_Cfg.ulAdminStatus != ADMSTS_DOWN )
                m_Cfg.ulAdminStatus = ADMSTS_UP;

            if( m_hDev != INVALID_HANDLE &&
                pCfg->ulAdminStatus != m_Cfg.ulAdminStatus )
            {
                AdminStatus = pCfg->ulAdminStatus;
                (*m_pfnXtmrtReq) (m_hDev, XTMRT_CMD_SET_ADMIN_STATUS,
                    (UINT32 *) &AdminStatus);
                pCfg->ulLastChange = XtmOsTickGet() / 10;
            }

            memcpy(&m_Cfg, pCfg, sizeof(m_Cfg));
            m_ulRemovePending = 0;

            if( m_ulConnected ) 
                CheckTransmitQueues();
        }
    }
    else
        m_ulRemovePending = 1;

    return( bxStatus );

} /* SetCfg */


/***************************************************************************
 * Function Name: SetTdt
 * Description  : Saves the Traffic Descriptor Table pointer and reconfigures
 *                the connection's shaper(s) if it is connected.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_CONNECTION::SetTdt( PXTM_TRAFFIC_DESCR_PARM_ENTRY pTrafficDescrEntries,
                                      UINT32 ulNumTrafficDescrEntries )
{
    if (m_ulConnSem == NULL) {
        XtmOsPrintf ("bcmxtmcfg: STdt- ConnSem NULL\n");        
        return XTMSTS_STATE_ERROR;
    }

    XtmOsRequestLock(m_ulConnSem);

    m_pTrafficDescrEntries     = pTrafficDescrEntries;
    m_ulNumTrafficDescrEntries = ulNumTrafficDescrEntries;

    /* Queue shaper config would be called from ReserveOneTxQueue().
     * There should be no need to configure the queue shaper here.
     * Should clean up later.
     */
#if 0                   
    if( m_ulConnected )
    {
        UINT32 i;
        PXTMRT_TRANSMIT_QUEUE_ID pTxQId;
        UINT32 ulIsPtm = 
            ((m_Addr.ulTrafficType & TRAFFIC_TYPE_ATM_MASK) != TRAFFIC_TYPE_ATM)?
            1 : 0;

        for( i = 0, pTxQId = m_TxQIds; i < m_ulTxQIdsSize; i++, pTxQId++ )
        {
            if( ulIsPtm )
            {
            
                ConfigurePtmQueueShaper( pTxQId->ulQueueIndex,
                                         pTxQId->ulMinBitRate,
                                         pTxQId->ulShapingRate,
                                         pTxQId->usShapingBurstSize );
            }
            else
            {
                /* In ATM mode, if the VC has only one queue, we need to
                 * configure its shaper here.
                 * For multi-queue VC, we need to disable its shaper and
                 * use the MP group shaper instead.
                 */
                if( m_ulTxQIdsSize == 1)
                    ConfigureAtmShaper( QUEUE_SHAPER, pTxQId->ulQueueIndex );
                else
                    XP_REGS->ulSsteQueueGtsCfg[pTxQId->ulQueueIndex] = 0x0;
            }
        }
    }
#endif

    XtmOsReleaseLock(m_ulConnSem);
    return( XTMSTS_SUCCESS );

} /* SetTdt */


/***************************************************************************
 * Function Name: SetLinkInfo
 * Description  : Processes a "link UP" or "link down" condition.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_CONNECTION::SetLinkInfo( UINT32 ulLinkMask,
                                           PXTM_INTERFACE_LINK_INFO pLinkInfo,
                                           UINT32 ulBondingPort,
                                           PXTM_INTERFACE_LINK_INFO pOtherLinkInfo,
                                           UINT32 ulXTMLinkMode, UINT32 ulXTMNitroMode)
{
   int    ret ;
   UINT32 ulPort=0 ;
   UINT32 connected = 0;
   UINT32 pendingConnected = 0;
   UINT32 pendingDisconnected = 0;
   UINT32 pendingConnectionUpdate = 0;
   UINT32 queueIdx;
   XTMRT_LINK_STATUS_CHANGE Lsc;

   BCMXTM_STATUS bxStatus = XTMSTS_SUCCESS;

   if (m_ulConnSem == NULL) {
      XtmOsPrintf ("bcmxtmcfg: SLI- ConnSem NULL\n");        
      return XTMSTS_STATE_ERROR;
   }

   //XtmOsPrintf("%s:Enter\n",__FUNCTION__);

   memset(&Lsc, 0x00, sizeof(Lsc));

   Lsc.ulLinkDataMask = ulLinkMask ;
   Lsc.ulLinkState    = pLinkInfo->ulLinkState;
   Lsc.ulTrafficType  = pLinkInfo->ulLinkTrafficType ;

   if (pOtherLinkInfo == NULL)
   {
      Lsc.ulLinkUsRate      = pLinkInfo->ulLinkUsRate;
      Lsc.ulLinkDsRate      = pLinkInfo->ulLinkDsRate;
      Lsc.ulOtherLinkUsRate = 0;
      Lsc.ulOtherLinkDsRate = 0;
   }
   else
   {
      if (ulBondingPort != PHY_PORTID_0)
      {
         Lsc.ulLinkUsRate      = pLinkInfo->ulLinkUsRate;
         Lsc.ulLinkDsRate      = pLinkInfo->ulLinkDsRate;
         Lsc.ulOtherLinkUsRate = pOtherLinkInfo->ulLinkUsRate;
         Lsc.ulOtherLinkDsRate = pOtherLinkInfo->ulLinkDsRate;
      }
      else
      {
         Lsc.ulLinkUsRate      = pOtherLinkInfo->ulLinkUsRate;
         Lsc.ulLinkDsRate      = pOtherLinkInfo->ulLinkDsRate;
         Lsc.ulOtherLinkUsRate = pLinkInfo->ulLinkUsRate;
         Lsc.ulOtherLinkDsRate = pLinkInfo->ulLinkDsRate;
      }
   }
   XtmOsRequestLock(m_ulConnSem);
   if(m_ulUpdatePending == 1)
   {
      XtmOsReleaseLock(m_ulConnSem);
      return XTMSTS_IN_USE;
   }
   m_ulUpdatePending = 1;

   m_ulXTMLinkMode = ulXTMLinkMode ;
   m_ulXTMNitroMode = ulXTMNitroMode ;

   if( pLinkInfo->ulLinkState == LINK_UP )
   {
      if (!m_ulConnected)
      {
         /* Link up. Reserve vcids and transmit queues. */
         m_ulTxQIdsSize = 0;
         memset(m_VcidInfo, 0x00, sizeof(m_VcidInfo));
         for( UINT32 i = 0; i < MAX_VCIDS; i++ )
         {
            m_VcidInfo[i].ulMpIdx    = INVALID_INDEX;
            m_VcidInfo[i].ulQueueIdx = INVALID_INDEX;
            m_VcidInfo[i].ulBondingPort  = MAX_PHY_PORTS;
         }

         bxStatus = ReserveVcidEntries();

         if( bxStatus == XTMSTS_SUCCESS )
         {
            UINT32 i;
            PXTM_TRANSMIT_QUEUE_PARMS pTxQParms;

            for( i = 0, pTxQParms = m_Cfg.TransmitQParms;
                  i < m_Cfg.ulTransmitQParmsSize && bxStatus == XTMSTS_SUCCESS;
                  i++, pTxQParms++ )
            {
               if (pTxQParms->usSize != 0)
                  bxStatus = ReserveOneTxQueue(pTxQParms);
               if (bxStatus != XTMSTS_SUCCESS)
                  break ;
            }
         }

         if( bxStatus == XTMSTS_SUCCESS )
         {
            Lsc.ulTransmitQueueIdsSize = m_ulTxQIdsSize;
            memcpy(Lsc.TransmitQueueIds, m_TxQIds, m_ulTxQIdsSize *
                  sizeof(XTMRT_TRANSMIT_QUEUE_ID));
            Lsc.ulRxVcidsSize = CopyVcids(Lsc.ucRxVcids);
            Lsc.ucTxVcid = Lsc.ucRxVcids[0];

            if( pLinkInfo->ulLinkTrafficType == TRAFFIC_TYPE_PTM_RAW )
               Lsc.ulLinkState |= LSC_RAW_ENET_MODE;

#if defined(ADSLMIBDEF_H_VER) && (ADSLMIBDEF_H_VER  >  0)
            XtmOsReleaseLock(m_ulConnSem);
            if( g_pfnAdslGetObjValue != NULL )
            {
               long int size = sizeof(PhyMibInfo);
               UINT32   ulRxPafCfg = XP_REGS->ulRxPafConfig;

               ulPort = (ulLinkMask & 0x1) ? PHY_PORTID_0 
                  : ((ulLinkMask & 0x2) ?  PHY_PORTID_1 : MAX_PHY_PORTS) ;

               ret = g_pfnAdslGetObjValue(ulPort, NULL, 0, (char *)&PhyMibInfo, &size);

               if (ret != 0)
                  XtmOsPrintf ("Error g_pfnAdslGetObjValue port - %d return value - %d \n", ulPort, ret) ;

               if( PhyMibInfo.IkanosCO4Detected )
               {
                  /* use ikanos ptm crc/enet fcs */
                  XtmOsPrintf ("CO4 DSLAM based operations for PTM CRC & ENET FCS. \n") ;
                  XP_REGS->ulTxSarCfg |= TXSARP_USE_ALT_PTM_CRC;
                  ulRxPafCfg &= ~RP_TC_CRC_ONEC_OUTPUT;
                  ulRxPafCfg |= (RP_TC_CRC_BITREV_OUTPUT | RP_TC_CRC_BITREV_INPUT);
                  ulRxPafCfg &= ~RP_ENET_CRC_DROP_PKT ;
               }
               else
               {
                  XtmOsPrintf ("NON-CO4 DSLAM based operations for PTM CRC & ENET FCS. \n") ;
                  XP_REGS->ulTxSarCfg &= ~TXSARP_USE_ALT_PTM_CRC;
                  ulRxPafCfg |= RP_TC_CRC_ONEC_OUTPUT;
                  ulRxPafCfg &= ~(RP_TC_CRC_BITREV_OUTPUT | RP_TC_CRC_BITREV_INPUT);
                  ulRxPafCfg |= RP_ENET_CRC_DROP_PKT ;
               }
               XP_REGS->ulRxPafConfig = ulRxPafCfg;
            }
            XtmOsRequestLock(m_ulConnSem);
#endif
            pendingConnected = 1;
         }
         else {
            UnreserveVcidsAndTxShapers();
            m_ulUpdatePending = 0;
         }
      }
      else
      {
         /* Connection is already connected */
         Lsc.ulTransmitQueueIdsSize = m_ulTxQIdsSize;
         memcpy(Lsc.TransmitQueueIds, m_TxQIds, m_ulTxQIdsSize * sizeof(XTMRT_TRANSMIT_QUEUE_ID));
         pendingConnectionUpdate = 1;
      }
   } /* if (LINK_UP) */
   else
   {
      if( m_ulConnected )
      {
         if (ulLinkMask == 0)
         {
            /* Link down. Unreserve vcids and transmit queues. */
            Lsc.ulTransmitQueueIdsSize = m_ulTxQIdsSize;
            memcpy(Lsc.TransmitQueueIds, m_TxQIds, m_ulTxQIdsSize *
                  sizeof(XTMRT_TRANSMIT_QUEUE_ID));
            Lsc.ulRxVcidsSize = CopyVcids(Lsc.ucRxVcids);
            Lsc.ucTxVcid = Lsc.ucRxVcids[0];
            pendingDisconnected = 1;
         }
         else
         {
            Lsc.ulTransmitQueueIdsSize = m_ulTxQIdsSize;
            memcpy(Lsc.TransmitQueueIds, m_TxQIds, m_ulTxQIdsSize * sizeof(XTMRT_TRANSMIT_QUEUE_ID));

            /* One or more links are UP in the group */
            pendingConnectionUpdate = 1;
         }

         bxStatus = XTMSTS_SUCCESS;
      }
      else
      {
         m_ulUpdatePending = 0;
         bxStatus = XTMSTS_STATE_ERROR;
      }
   } /* if (LINK_DOWN) */
   connected = m_ulConnected;
   XtmOsReleaseLock(m_ulConnSem);
   if( pendingConnected || pendingDisconnected || pendingConnectionUpdate)
   {
      if ((m_hDev != INVALID_HANDLE) && ((*m_pfnXtmrtReq) (m_hDev, XTMRT_CMD_LINK_STATUS_CHANGED, &Lsc) == 0 ) )
      {
         XtmOsRequestLock(m_ulConnSem);
         if(pendingConnected)
         {
            m_ulConnected = 1;
            pendingConnected = 0;
         }
         else if(pendingDisconnected)
         {
            XtmOsPrintf ("XTM W1B %u\n", XtmOsGetTimeStampMs()) ;
            for(queueIdx=0;queueIdx<m_ulTxQIdsSize;queueIdx++)
               WaitForTxComplete(m_TxQIds[queueIdx].ulQueueIndex) ;
            XtmOsPrintf ("XTM W1E %u\n", XtmOsGetTimeStampMs()) ;
            m_ulConnected = 0;
            UnreserveVcidsAndTxShapers();
            pendingDisconnected = 0;
         }
         else
         {
            pendingConnectionUpdate = 0;
         }
         bxStatus = XTMSTS_SUCCESS;
         connected = m_ulConnected;
         m_ulUpdatePending = 0;
         XtmOsReleaseLock(m_ulConnSem);
      }
      else
      {
         XtmOsRequestLock(m_ulConnSem);
         bxStatus = XTMSTS_ERROR;
         if(pendingConnected)
         {
            UnreserveVcidsAndTxShapers();
            m_ulConnected = 0 ;
            pendingConnected = 0;
         }
         else 
         {
            pendingDisconnected = 0;
            pendingConnectionUpdate = 0;
         }
         connected = m_ulConnected;
         m_ulUpdatePending = 0;
         XtmOsReleaseLock(m_ulConnSem);
      }
   } /* if( pendingConnected || pendingDisconnected || pendingConnectionUpdate) */

   if (connected)
      XtmOsPrintf ("bcmxtmcfg: Connection UP, LinkActiveStatus=0x%x, US=%d, DS=%d \n",
            ulLinkMask,
            (pOtherLinkInfo ? (pLinkInfo->ulLinkUsRate+pOtherLinkInfo->ulLinkUsRate) :
             pLinkInfo->ulLinkUsRate), 
            (pOtherLinkInfo?(pLinkInfo->ulLinkDsRate+pOtherLinkInfo->ulLinkDsRate) :
             pLinkInfo->ulLinkDsRate)) ;
   else
      XtmOsPrintf ("bcmxtmcfg: Connection DOWN, LinkActiveStatus=0x%x \n",
            ulLinkMask) ;

   return( bxStatus );

} /* SetLinkInfo */


/***************************************************************************
 * Function Name: CreateNetworkDevice
 * Description  : Calls the bcmxtmrt driver to create an operating system
 *                network device instance.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_CONNECTION::CreateNetworkDevice( char *pszNetDeviceName, UINT32 ulTxPafEnabled )
{
    BCMXTM_STATUS bxStatus;
   //XtmOsPrintf("%s:Enter\n",__FUNCTION__);

    if( !m_ulCreated )
    {
        XTMRT_CREATE_NETWORK_DEVICE Cni;

        memcpy(&Cni.ConnAddr, &m_Addr, sizeof(m_Addr));
//FIXME        strcpy(Cni.szNetworkDeviceName, pszNetDeviceName);
        memcpy(Cni.szNetworkDeviceName, pszNetDeviceName, NETWORK_DEVICE_NAME_SIZE);

        Cni.ulHeaderType = m_Cfg.ulHeaderType;
        Cni.ulFlags = 0;
        Cni.ulTxPafEnabled = ulTxPafEnabled ;
        if( m_ulHwAddHdr == 1 )
            Cni.ulFlags |= CNI_HW_ADD_HEADER;
        if( m_ulHwRemoveHdr == 1 )
            Cni.ulFlags |= CNI_HW_REMOVE_HEADER;
        if( (XP_REGS->ulTxSarCfg & TXSAR_USE_ALT_FSTAT) == TXSAR_USE_ALT_FSTAT )
            Cni.ulFlags |= CNI_USE_ALT_FSTAT;
        if( (XP_REGS->ulRxPafConfig & RP_DROP_PKT_END_BYTE_MASK) > 0 )
            Cni.ulFlags |= CNI_HW_REMOVE_TRAILER;
        if( m_ulXTMLinkMode == XTM_LINK_MODE_GFAST )
            Cni.ulFlags |= CNI_GFAST ;
        if( m_ulXTMNitroMode == XTM_MODE_NITRO_ENABLE )
            Cni.ulFlags |= CNI_NITRO ;

        if( (*m_pfnXtmrtReq) (NULL, XTMRT_CMD_CREATE_DEVICE, &Cni) == 0 )
        {
//FIXME            strcpy(m_szNetDeviceName, pszNetDeviceName);
            memcpy(m_szNetDeviceName, pszNetDeviceName, NETWORK_DEVICE_NAME_SIZE);
            m_hDev = Cni.hDev;
            m_ulCreated = 1;
            bxStatus = XTMSTS_SUCCESS;

            /* Inform the DSL driver if the "ptm0" interface goes up */

            if (!strcmp (m_szNetDeviceName, "ptm0")) {

               if ( g_pfnAdslWanDevState != NULL ) {
                  g_pfnAdslWanDevState (1) ; /* 1 for up */
               }
            }
        }
        else
            bxStatus = XTMSTS_RESOURCE_ERROR;
    }
    else
        bxStatus = XTMSTS_STATE_ERROR;

    return( bxStatus );

} /* CreateNetworkDevice */


/***************************************************************************
 * Function Name: DeleteNetworkDevice
 * Description  : Calls the bcmxtmrt driver to delete an operating system
 *                network device.
 *                Also calls the DSL driver to notify WAN "ptm0" interface down
 *                condition.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_CONNECTION::DeleteNetworkDevice( void )
{
    BCMXTM_STATUS bxStatus;

   //XtmOsPrintf("%s:Enter\n",__FUNCTION__);
    while (m_ulUpdatePending ==1)
       XtmOsDelay (100) ;

    if( m_ulConnected )
    {
        XTM_INTERFACE_LINK_INFO LinkInfo;

        //XtmOsPrintf ("xtm: Conn DeleteNetworkDevice Connected \n") ;
        memset(&LinkInfo, 0x00, sizeof(LinkInfo));
        LinkInfo.ulLinkState = LINK_DOWN;
        bxStatus = SetLinkInfo( 0x0, &LinkInfo, 0, NULL, m_ulXTMLinkMode, m_ulXTMNitroMode );
        if(bxStatus != XTMSTS_SUCCESS)
           return bxStatus;
    }

    if( m_ulCreated )
    {
        //XtmOsPrintf ("xtm: Created. Delete_Device \n") ;
        (*m_pfnXtmrtReq) (m_hDev, XTMRT_CMD_DELETE_DEVICE, NULL);
         m_hDev = INVALID_HANDLE;
         m_ulCreated = 0;

        /* Inform the DSL driver if the "ptm0" interface goes down */

        if (!strcmp (m_szNetDeviceName, "ptm0")) {

           if ( g_pfnAdslWanDevState != NULL ) {
              g_pfnAdslWanDevState (0) ; /* 0 for down */
           }
        }

        m_szNetDeviceName[0] = '\0';
        bxStatus = XTMSTS_SUCCESS;
    }
    else
        bxStatus = XTMSTS_STATE_ERROR;

    return( bxStatus );

} /* DeleteNetworkDevice */


/***************************************************************************
 * Function Name: ReserveVcidEntries
 * Description  : Reserves Rx CAM and, for ATM, Tx Table entries.  The index
 *                into the Rx CAM and Tx Table is called a "vcid".  The same
 *                vcid indicies are used for both the Rx CAM and Tx Table.
 *
 *                The Rx CAM uses vcid for both ATM and PTM, and includes port
 *                in the CAM entry.  Tx Table only uses vcid for ATM and does
 *                not include port in a Tx Table entry.  Since the same vcids
 *                are used for both the Rx CAM and Tx Table, the Tx Table may
 *                have multiple entries with the same value (same VPI/VCI).
 *                This is redundant but does (should) not create problems.
 *
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_CONNECTION::ReserveVcidEntries( void )
{
   BCMXTM_STATUS bxStatus = XTMSTS_SUCCESS;
   UINT32 ulPort;
   UINT32 ulPortMask = m_Addr.u.Conn.ulPortMask ;


    for( ulPort = PHY_PORTID_0;
         ulPort < MAX_INTERFACES && bxStatus==XTMSTS_SUCCESS;
         ulPort++ )
   {
      MapXtmOsPrintf ("ulPort=%d, PortMask=%x \n", ulPort, ulPortMask) ;

      if( (ulPortMask & PORT_TO_PORTID(ulPort)) != 0 )
      {
         /* The connection uses this port.  Find an available vcid. */
         if( (m_Addr.ulTrafficType & TRAFFIC_TYPE_ATM_MASK) == TRAFFIC_TYPE_ATM )
         {
            if( m_ucVcids[ulPort][0] == INVALID_VCID )
            {
               if (m_Addr.ulTrafficType == TRAFFIC_TYPE_ATM_BONDED)
               {
                  /* For  bonded ports, we dont need duplicate vcid configuration
                   * for odd ports (bonded pairs...) on the Rx/Tx side. 
                   * HW does not work with this kind of configuration.
                   * Dual latency - todo */
                  if (ulPort == PHY_PORTID_0)
                     bxStatus = ReserveAtmVcidEntry( ulPort ) ;
               }
               else
               {
                  bxStatus = ReserveAtmVcidEntry( ulPort ) ;
               }
            }
         }
         else
         {
            /* TBD. Why only port 0? */
            /* If the port is 0 and the TEQ port is 0, then configure
             * receive for "raw" mode.
             */
#if 0
            if( ulPort == PHY_PORTID_0 &&
                  ((XP_REGS->ulRxUtopiaCfg & RXUTO_INT_TEQ_PORT_MASK) >>
                  RXUTO_INT_TEQ_PORT_SHIFT) == PORT_PHY0_PATH0 )
            {
               if( m_ucVcids[ulPort][0] ==  INVALID_VCID )
               {
                  bxStatus = ReserveRawVcidEntry( ulPort ); /* this does not work yet */
            }
            }
            else
#else
            {
               UINT32 ulPtmPri;

               for( ulPtmPri = PTM_PRI_LOW; ulPtmPri <= PTM_PRI_HIGH; ulPtmPri++ )
               {
                  if( (m_Addr.u.Flow.ulPtmPriority & ulPtmPri) != 0 &&
                      m_ucVcids[ulPort][ulPtmPri-PTM_PRI_LOW] ==  INVALID_VCID )
                  {
                     if (m_Addr.ulTrafficType == TRAFFIC_TYPE_PTM_BONDED) {

                         if (ulPort == PHY_PORTID_0)      /* Dual Latancy - toDo */
                              bxStatus = ReservePtmVcidEntry( ulPort, ulPtmPri, ulPort+1) ;
                      }
                      else  {
                         bxStatus = ReservePtmVcidEntry( ulPort, ulPtmPri, MAX_PHY_PORTS );
                      }
                  }
               } /* for (ulPtmPri) */
            }
#endif
         }/* else part of if (atm traffic type) */
      } /* if( (ulPortMask & PORT_TO_PORTID(ulPort)) != 0 ) */
   } /* for (ulPort) */

   return( bxStatus );

} /* ReserveVcidEntries */


/***************************************************************************
 * Function Name: ReserveAtmVcidEntry
 * Description  : Finds an available vcid and configures the Rx CAM and Tx
 *                Table entries.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_CONNECTION::ReserveAtmVcidEntry( UINT32 ulPort )
{
    BCMXTM_STATUS bxStatus = XTMSTS_RESOURCE_ERROR;
    UINT32 ulVcid, ulCam, ulRam;
    //XtmOsPrintf("%s:Enter\n",__FUNCTION__);

    for( ulVcid = 0; ulVcid < MAX_VCIDS; ulVcid++ )
    {
        if( (m_pulRxVpiVciCamShadow[ulVcid] & RXCAM_VALID) == 0 )
        {
            /* An available vcid is found. */
            ulCam = ulPort | (m_Addr.u.Vcc.usVci << RXCAMA_VCI_SHIFT) |
                (m_Addr.u.Vcc.usVpi << RXCAMA_VPI_SHIFT) | RXCAM_VALID;

            ulRam = ulVcid << RXCAM_VCID_SHIFT;

            switch( m_Cfg.ulAtmAalType )
            {
            case AAL_5:
                ulRam |= RXCAM_CT_AAL5;

                /* Strip RFC2684 header on receive. */
                /* Note: Enable RFC Stripping by hardware regardless strip byte
                 * count is zero or not. This is to fix the bug where pppoa/vcmux
                 * can not connect if it is configured after deleting a ipoa/LLC
                 * or pppoe/LLC connection, without reboot or resync dsl link.
                 * See SWBCACPE-9256.
                 */
                if( m_ulHwRemoveHdr == 1 )
                {
                    ulRam |= RXCAM_STRIP_EN |
                        (HT_LEN(m_Cfg.ulHeaderType) << RXCAM_STRIP_BYTE_SHIFT);
                }
                break;

            case AAL_0_PACKET:
                ulRam |= RXCAM_CT_AAL0_PKT;
                break;

            case AAL_0_CELL:
                ulRam |= RXCAM_CT_AAL0_CELL;
                break;
            }

            m_ucVcids[ulPort][0] = ulVcid;

            m_VcidInfo[ulVcid].pConnArb       = &m_Cfg.ConnArbs[ulPort][0];
            m_VcidInfo[ulVcid].ulPort         = ulPort;
            m_VcidInfo[ulVcid].ulQueueIdx     = INVALID_INDEX;
            m_VcidInfo[ulVcid].ulMpIdx        = INVALID_INDEX;
            m_VcidInfo[ulVcid].ulMpQueueCount = 0;
            m_VcidInfo[ulVcid].ulBondingPort  = MAX_PHY_PORTS;

            /* Write the values to the Rx VPI/VCI/Port CAM. */
            XP_REGS->ulRxVpiVciCam[(ulVcid * 2) + 1] = ulRam;
            XP_REGS->ulRxVpiVciCam[ulVcid * 2]       = ulCam;
            m_pulRxVpiVciCamShadow[ulVcid]           = ulCam;

            /* Also use the same vcid for the Tx Table. */
            XP_REGS->ulTxVpiVciTable[ulVcid] =
                (m_Addr.u.Vcc.usVci << TXTBL_VCI_SHIFT) |
                (m_Addr.u.Vcc.usVpi << TXTBL_VPI_SHIFT);

            /* Set F4 OAM VPI to this VCC.
             * Note: This will override any previous F4 OAM VPI value.
             *       Since there is only one register for F4 OAM VPI,
             *       we just have to set it to the last configured VCC.
             *       This patch only works well if the CPE has only one
             *       ATM path (VPI) to the ISP. Normally it is the case.
             */
            XP_REGS->ulTxOamCfg &=
                ~(TXOAM_F4_SEG_VPI_MASK | TXOAM_F4_E2E_VPI_MASK);
            XP_REGS->ulRxOamCfg &=
                ~(RXOAM_F4_SEG_VPI_MASK | RXOAM_F4_E2E_VPI_MASK);
            XP_REGS->ulTxOamCfg |=
                (m_Addr.u.Vcc.usVpi << TXOAM_F4_E2E_VPI_SHIFT) | m_Addr.u.Vcc.usVpi;
            XP_REGS->ulRxOamCfg |=
                (m_Addr.u.Vcc.usVpi << RXOAM_F4_E2E_VPI_SHIFT) | m_Addr.u.Vcc.usVpi;

            /* Weight must be at least one to get scheduled. */
            if( m_VcidInfo[ulVcid].pConnArb->ulWeightValue == 0 )
                m_VcidInfo[ulVcid].pConnArb->ulWeightValue = 1;

            bxStatus = XTMSTS_SUCCESS;
            break;
        }
    }

    return( bxStatus );

} /* ReserveAtmVcidEntry */


/***************************************************************************
 * Function Name: ReservePtmVcidEntry
 * Description  : Finds an available vcid and configures an Rx CAM entry for
 *                each PTM priority that is in the connection address.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_CONNECTION::ReservePtmVcidEntry( UINT32 ulPort,
    UINT32 ulPtmPri, UINT32 ulBondingPort)
{
    BCMXTM_STATUS bxStatus = XTMSTS_RESOURCE_ERROR;
    UINT32 ulVcid, ulCam, ulRam, ulPafVcId ;
    //XtmOsPrintf("%s:Enter\n",__FUNCTION__);

    for( ulVcid = 0; ulVcid < MAX_VCIDS; ulVcid++ )
    {
        if( (m_pulRxVpiVciCamShadow[ulVcid] & RXCAM_VALID) == 0 )
        {
            /* An available vcid is found. */
            ulCam = ulPort | RXCAM_VALID;

            ulRam = (ulVcid << RXCAM_VCID_SHIFT) | (RXCAM_CT_PTM << RXCAM_CT_SHIFT);
            m_ucVcids[ulPort][ulPtmPri - PTM_PRI_LOW] = ulVcid;

            m_VcidInfo[ulVcid].pConnArb = &m_Cfg.ConnArbs[ulPort][ulPtmPri - PTM_PRI_LOW];
            m_VcidInfo[ulVcid].ulPort         = ulPort;
            m_VcidInfo[ulVcid].ulQueueIdx     = INVALID_INDEX;
            m_VcidInfo[ulVcid].ulMpIdx        = INVALID_INDEX;
            m_VcidInfo[ulVcid].ulMpQueueCount = 0;
            m_VcidInfo[ulVcid].ulBondingPort  = ulBondingPort;
            m_VcidInfo[ulVcid].ulPtmPri       = ulPtmPri;

            /* Write the values to the Rx VPI/VCI/Port CAM. */
            XP_REGS->ulRxVpiVciCam[(ulVcid * 2) + 1] = ulRam;
            XP_REGS->ulRxVpiVciCam[ulVcid * 2] = ulCam;
            m_pulRxVpiVciCamShadow[ulVcid] = ulCam;

            /* Write RxPaf VCID regs */
            ulPafVcId  = XP_REGS->ulRxPafVcid;
            ulPafVcId &= ~((RXPAF_VCID_MASK<<(ulPort * 4))
                           <<((ulPtmPri-PTM_PRI_LOW) * RXPAF_VCID_PREEMPTION_SHIFT));
            ulPafVcId |= ((ulVcid<<(ulPort * 4))
                           <<((ulPtmPri-PTM_PRI_LOW) * RXPAF_VCID_PREEMPTION_SHIFT));
            XP_REGS->ulRxPafVcid = ulPafVcId ;

#if 0 /* Did not work. to be investigated. */
            /* Write VCID default destination queue for ptm pre-emptive flow */
            if (ulPtmPri == PTM_PRI_HIGH)
            {
               UINT32 ulVcidDesQ;

               ulVcidDesQ  = XP_REGS->ulRxPBufMuxVcidDefQueueId[0];
               ulVcidDesQ &= ~(RXPBUF_MUXVCID_DEF_QID_MASK<<(ulVcid * 2));
               ulVcidDesQ |= (0x1<<(ulVcid * 2));   /* set destination qid to 1 */
               XP_REGS->ulRxPBufMuxVcidDefQueueId[0] = ulVcidDesQ;
            }
#endif
            if (ulBondingPort != MAX_PHY_PORTS)
            {
               /* Write RxPaf VCID regs */
                ulPafVcId  = XP_REGS->ulRxPafVcid;
                ulPafVcId &= ~((RXPAF_VCID_MASK<<(ulBondingPort * 4))
                               <<((ulPtmPri-PTM_PRI_LOW) * RXPAF_VCID_PREEMPTION_SHIFT));
                ulPafVcId |= ((ulVcid<<(ulBondingPort * 4))
                               <<((ulPtmPri-PTM_PRI_LOW) * RXPAF_VCID_PREEMPTION_SHIFT));
                XP_REGS->ulRxPafVcid = ulPafVcId ;
            }

            /* Weight must be at least one to get scheduled. */
            if( m_VcidInfo[ulVcid].pConnArb->ulWeightValue == 0 )
                m_VcidInfo[ulVcid].pConnArb->ulWeightValue = 1;

            bxStatus = XTMSTS_SUCCESS;
            break;
        }
    }

    return( bxStatus );

} /* ReservePtmVcidEntry */


/***************************************************************************
 * Function Name: ReserveRawVcidEntry
 * Description  : Finds an available vcid and configures an Rx CAM entry for
 *                each PTM priority that is in the connection address.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_CONNECTION::ReserveRawVcidEntry( UINT32 ulPort )
{
    BCMXTM_STATUS bxStatus = XTMSTS_RESOURCE_ERROR;
    UINT32 ulVcid, ulCam, ulRam;
    //XtmOsPrintf("%s:Enter\n",__FUNCTION__);

    for( ulVcid = 0; ulVcid < MAX_VCIDS; ulVcid++ )
    {
        if( (m_pulRxVpiVciCamShadow[ulVcid] & RXCAM_VALID) == 0 )
        {
            /* An available vcid is found. */
            ulCam = ulPort | RXCAM_TEQ_CELL | RXCAM_VALID;
            ulRam = (ulVcid << RXCAM_VCID_SHIFT) | RXCAM_CT_TEQ;

            m_ucVcids[ulPort][0] = ulVcid;
            m_VcidInfo[ulVcid].pConnArb = &m_Cfg.ConnArbs[ulPort][0];
            m_VcidInfo[ulVcid].ulPort         = ulPort;
            m_VcidInfo[ulVcid].ulQueueIdx     = INVALID_INDEX;
            m_VcidInfo[ulVcid].ulMpIdx        = INVALID_INDEX;
            m_VcidInfo[ulVcid].ulMpQueueCount = 0;
            m_VcidInfo[ulVcid].ulBondingPort  = MAX_PHY_PORTS;

            /* Write the values to the Rx VPI/VCI/Port CAM. */
            XP_REGS->ulRxVpiVciCam[(ulVcid * 2) + 1] = ulRam;
            XP_REGS->ulRxVpiVciCam[ulVcid * 2] = ulCam;
            m_pulRxVpiVciCamShadow[ulVcid] = ulCam;

            /* Weight must be at least one to get scheduled. */
            if( m_VcidInfo[ulVcid].pConnArb->ulWeightValue == 0 )
                m_VcidInfo[ulVcid].pConnArb->ulWeightValue = 1;

            bxStatus = XTMSTS_SUCCESS;
                break;
        }
    }

    return( bxStatus );

} /* ReserveRawVcidEntry */

/***************************************************************************
 * Function Name: ReserveOneMpGroup
 * Description  : Reserves one Multi Priority group.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_CONNECTION::ReserveOneMpGroup( VcidInfo *pInfo )
{
    BCMXTM_STATUS bxStatus = XTMSTS_RESOURCE_ERROR;
    UINT32 ulMpAalCfg;
    UINT32 i, ulShift;
    //XtmOsPrintf("%s:Enter\n",__FUNCTION__);
    UINT32 ulIsPtm,ulMaxMpGrps;
    ulIsPtm = ((m_Addr.ulTrafficType & TRAFFIC_TYPE_ATM_MASK) != TRAFFIC_TYPE_ATM)? 1 : 0;
#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178)
    if(ulIsPtm) {
       ulMaxMpGrps = PTM_TXMP_NUM_GROUPS;
    }
    else {
       ulMaxMpGrps = ATM_TXMP_NUM_GROUPS;
    }
#else
    ulMaxMpGrps = TXMP_NUM_GROUPS;
#endif

    /* Find an available MP index. */
    for( i = 0, ulShift = 0, ulMpAalCfg = XP_REGS->ulTxMpCfg;
         i < ulMaxMpGrps;
         i++, ulShift += TXMP_GROUP_SIZE )
    {
        if( (ulMpAalCfg & (TXMP_GROUP_EN << ulShift)) == 0 )
        {

            /* Per ATM VC / PTM Flow cell based arbitration. */
            PXTM_CONN_ARB pArb = pInfo->pConnArb;

            pInfo->ulMpIdx = i;

            XP_REGS->ulTxMpCfg |= (TXMP_GROUP_EN << ulShift);

            if( ulIsPtm )
            {
               /* Configure SstUtopiaCfg */

               /* Non-bonded case:
                * 1) Path0 Latency-non-preemptive, Packets are sent to 1 PB  queue (e.g. PBQ0)
                * 2) Path0 Latency-pre-emptive, Packets are sent to 1 PB  queue (e.g. PBQ1)
                * 3) Path1 Latency-non-preemptive, Packets are sent to 1 PB  queue (e.g. PBQ2)
                * 4) Path1 Latency-pre-emptive, Packets are sent to 1 PB  queue (e.g. PBQ3)
                * Bonded case:
                * 1) Path0 Latency-non-preemptive : Packets are fragmented into 2 PB queues (e.g. PBQ0, PBQ1)
                * 2) Path0 Latency-pre-emptive: Packets are fragmented into 2 PB queues (e.g. PBQ2, PBQ3)
                * 3) Path1 Latency-non-preemptive : Packets are fragmented into 2 PB queues (e.g. PBQ4, PBQ5)
                * 4) Path1 Latency-pre-emptive: Packets are fragmented into 2 PB queues (e.g. PBQ6, PBQ7)
                */

               UINT32 ulPktBufQueue = (pInfo->ulBondingPort == MAX_PHY_PORTS)?
                                         pInfo->ulMpIdx : (pInfo->ulMpIdx * 2);

                  if (pInfo->ulPtmPri == PTM_PRI_LOW)
               {
                  /* clear bits */
                  XP_REGS->ulSstUtopiaCfg &=
                     ~(SST_UTOPIA_CFG_PKTBUFQINDEX_NON_PREEMPT_MASK << (pInfo->ulPort*8));
                  
                  /* set bits */
                  XP_REGS->ulSstUtopiaCfg |= (ulPktBufQueue << (pInfo->ulPort*8));
               }
               else if (pInfo->ulPtmPri == PTM_PRI_HIGH)
               {
                  /* clear bits */
                  XP_REGS->ulSstUtopiaCfg &=
                     ~((SST_UTOPIA_CFG_ENABLE_PREEMPTION | SST_UTOPIA_CFG_PKTBUFQINDEX_PREEMPT_MASK)
                          << (pInfo->ulPort*8));
                  
                  /* set bits */
                  XP_REGS->ulSstUtopiaCfg |=
                     (ulPktBufQueue << ((pInfo->ulPort*8)+3)) |
                     (SST_UTOPIA_CFG_ENABLE_PREEMPTION << (pInfo->ulPort*8));
               }

               if (pInfo->ulBondingPort != MAX_PHY_PORTS)
               {
                  /* configure for bonded path */

                  if (pInfo->ulPtmPri == PTM_PRI_LOW)
                  {
                     /* clear bits */
                     XP_REGS->ulSstUtopiaCfg &=
                        ~(SST_UTOPIA_CFG_PKTBUFQINDEX_NON_PREEMPT_MASK << (pInfo->ulBondingPort*8));

                     /* set bits */
                     XP_REGS->ulSstUtopiaCfg |= ((ulPktBufQueue+1) << (pInfo->ulBondingPort*8));
                  }
                  else if (pInfo->ulPtmPri == PTM_PRI_HIGH)
                  {
                     /* clear bits */
                     XP_REGS->ulSstUtopiaCfg &=
                        ~((SST_UTOPIA_CFG_ENABLE_PREEMPTION | SST_UTOPIA_CFG_PKTBUFQINDEX_PREEMPT_MASK)
                             << (pInfo->ulBondingPort*8));

                     /* set bits */
                     XP_REGS->ulSstUtopiaCfg |=
                        ((ulPktBufQueue+1) << ((pInfo->ulBondingPort*8)+3)) |
                        (SST_UTOPIA_CFG_ENABLE_PREEMPTION << (pInfo->ulBondingPort*8));
                  }
               }
            }

            XP_REGS->ulSsteMpVcArbWt[i] = 
                  (pArb->ulWeightValue << SSTE_SETARBWT_SHIFT) |
                  (pInfo->ulPort << SSTE_UTPT_SHIFT);
            XP_REGS->ulSsteMpScr[i][0] &=
                  ~(SSTE_SCR_PLVLWT_MASK << SSTE_SCR_PLVLWT_SHIFT);
            XP_REGS->ulSsteMpScr[i][0] = 
                  (pArb->ulSubPriority << SSTE_SCR_PLVLWT_SHIFT);

            if( ulIsPtm )
               bxStatus = XTMSTS_SUCCESS;
            else 
               bxStatus = ConfigureAtmShaper( MPAAL_SHAPER, pInfo->ulMpIdx );
            break;
        }
    }

    return( bxStatus );

} /* ReserveOneMpGroup */


/***************************************************************************
 * Function Name: ReserveOneTxQueue
 * Description  : Reserves and configures one transmit queue.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_CONNECTION::ReserveOneTxQueue( PXTM_TRANSMIT_QUEUE_PARMS pTxQParms )
{
    BCMXTM_STATUS bxStatus = XTMSTS_SUCCESS;
    PXTMRT_TRANSMIT_QUEUE_ID pTxQId;
    UINT32 i, ulTxQueueIdx;
    UINT32 ulVcid  = GetVcid(pTxQParms->ulPortId, pTxQParms->ulPtmPriority);  
    UINT32 ulIsPtm = 
        ((m_Addr.ulTrafficType & TRAFFIC_TYPE_ATM_MASK) != TRAFFIC_TYPE_ATM)
        ? 1 : 0;
    VcidInfo *pInfo;
    //XtmOsPrintf("%s:Enter\n",__FUNCTION__);

    if( ulVcid >= MAX_VCIDS )
    {
        XtmOsPrintf("ReserveOneTxQueue: tx queue has NO vcid %d. PortId=%x Prio=%d \n", ulVcid,
                    pTxQParms->ulPortId, pTxQParms->ulPtmPriority);
        return( XTMSTS_STATE_ERROR );
    }

    /* Find an unused Tx queue. */
    for(i = 0, ulTxQueueIdx = INVALID_INDEX; i < MAX_TRANSMIT_QUEUES; i++)
    {
        if( (ms_ulTxQueuesInUse & (1 << i)) == 0 )
        {
            ulTxQueueIdx = i;
            ms_ulTxQueuesInUse |= (1 << i);
            break;
        }
    }

    if( ulTxQueueIdx < MAX_TRANSMIT_QUEUES )
    {
        XtmOsPrintf("bcmxtmcfg: Reserve TxQueueIdx=%d for vcid %d\n", ulTxQueueIdx, ulVcid);
    }
    else
    {
        XtmOsPrintf("ReserveOneTxQueue: Failed to allocate tx queue.\n");
        return( XTMSTS_RESOURCE_ERROR );
    }

    pInfo = &m_VcidInfo[ulVcid];

    /* Reserve an MP group for this connection if
     *   - this is the second queue of a ATM connection.
     *   - this is the first queue of a PTM connection.
     */ 
    if( (!ulIsPtm && pInfo->ulMpQueueCount == 1) ||
        ( ulIsPtm && pInfo->ulMpQueueCount == 0) )
    {
        if( pInfo->ulMpIdx == INVALID_INDEX )
            bxStatus = ReserveOneMpGroup( pInfo );
        else
            bxStatus = XTMSTS_STATE_ERROR;

        if( bxStatus != XTMSTS_SUCCESS )
        {
            /* Unreserve tx queue and return with error. */
            ms_ulTxQueuesInUse &= ~(1 << ulTxQueueIdx);

            XtmOsPrintf("ReserveOneTxQueue: Reserve MP grp error 0x%x\n", bxStatus);
            return( bxStatus );
        }
    }

    pInfo->ulMpQueueCount++;

    /* Weight must be at least one to get scheduled. */
    if( pTxQParms->ulWeightValue == 0 )
        pTxQParms->ulWeightValue = 1;

    /* Configure the shaper for this queue */
    if( ulIsPtm )
    {
        ConfigurePtmQueueShaper( ulTxQueueIdx,
                                 pTxQParms->ulMinBitRate,
                                 pTxQParms->ulShapingRate,
                                 pTxQParms->usShapingBurstSize );
    }
    else
    {
        ConfigureAtmShaper( QUEUE_SHAPER, ulTxQueueIdx ); 
                          
        /* In ATM mode, if the queue is a member of an MP group, we need
         * to disable its shaper and use the MP group shaper instead.
         */
        if( pInfo->ulMpIdx != INVALID_INDEX )
            XP_REGS->ulSsteQueueGtsCfg[ulTxQueueIdx] = 0x0;
    }

    /* Configure the queue arbiter for this queue. Also add the queue
     * to the MP group if exists.
     */
    ConfigureArbitration( ulTxQueueIdx, pInfo->ulMpIdx, pInfo->ulPort,
                          pTxQParms->ucWeightAlg,
                          pTxQParms->ulWeightValue,
                          pTxQParms->ucSubPriority );

    if( pInfo->ulMpIdx == INVALID_INDEX )
    {
        /* There was no MP group reserved for this connection.
         * This is a single queue VC. Save this tx queue index.
         */
        pInfo->ulQueueIdx = ulTxQueueIdx;
    }
    else if( pInfo->ulQueueIdx != INVALID_INDEX )
    {
        /* This must be a new MP group. We also need to add the first tx queue
         * to it. The first tx queue should have been saved in m_TxQIds[].
         */
        for( i = 0, pTxQId = m_TxQIds; i < m_ulTxQIdsSize; i++, pTxQId++ )
        {
            if( pTxQId->ulQueueIndex == pInfo->ulQueueIdx )
            {
                /* Configure the shaper for this queue */
                if( ulIsPtm )
                {
                    ConfigurePtmQueueShaper( pTxQId->ulQueueIndex,
                                             pTxQId->ulMinBitRate,
                                             pTxQId->ulShapingRate,
                                             pTxQId->usShapingBurstSize );
                }
                else
                {
                    ConfigureAtmShaper( QUEUE_SHAPER, pTxQId->ulQueueIndex );
                     
                    /* In ATM mode, if the queue is a member of an MP group, we need
                     * to disable its shaper and use the MP group shaper instead.
                     */
                    XP_REGS->ulSsteQueueGtsCfg[pTxQId->ulQueueIndex] = 0x0;
                }

                ConfigureArbitration( pInfo->ulQueueIdx, pInfo->ulMpIdx, pInfo->ulPort,
                                      pTxQId->ucWeightAlg,
                                      pTxQId->ulWeightValue,
                                      pTxQId->ucSubPriority );

                pInfo->ulQueueIdx = INVALID_INDEX;
                break;
            }
        }
        if (pInfo->ulQueueIdx != INVALID_INDEX)
        {
            XtmOsPrintf("ReserveOneTxQueue: Could not find pInfo->ulQueueIdx %d\n",
                        pInfo->ulQueueIdx);
            pInfo->ulQueueIdx = INVALID_INDEX;
        }
    }

    /* Fill in Channel Index Table. */
    if( (m_Addr.ulTrafficType&TRAFFIC_TYPE_ATM_MASK) == TRAFFIC_TYPE_ATM )
    {
        i = ulVcid << TXCHA_VCID_SHIFT;

        switch( m_Cfg.ulAtmAalType )
        {
        case AAL_5:
            i |= TXCHA_CT_AAL5;
            if( HT_LEN(m_Cfg.ulHeaderType) != 0 && m_ulHwAddHdr == 1 )
                i |= TXCHA_HDR_EN |
                     (HT_TYPE(m_Cfg.ulHeaderType)-PKT_HDRS_START_IDX)<<TXCHA_HDR_IDX_SHIFT;
            break;

        case AAL_0_PACKET:
            i |= TXCHA_CT_AAL0_PKT;
            break;

        case AAL_0_CELL:
            i |= TXCHA_CT_AAL0_CELL;
            break;
        }
    }
    else /* PTM */ {
        if (m_ulXTMLinkMode != XTM_LINK_MODE_GFAST)
           i = TXCHP_FCS_EN | TXCHP_CRC_EN;
        else
           i = TXCHP_FCS_EN ;
    }

    XP_REGS->ulTxChannelCfg[ulTxQueueIdx] = i;

    /* Fill in the transmit queue id structure that is passed to the
     * bcmxtmrt driver.
     */
    pTxQParms->ulTxQueueIdx = ulTxQueueIdx;
    
    pTxQId = &m_TxQIds[m_ulTxQIdsSize++];
    pTxQId->ucQosQId        = pTxQParms->ucQosQId;
    pTxQId->ulPortId        = pTxQParms->ulPortId;
    pTxQId->ulPtmPriority   = pTxQParms->ulPtmPriority;
    pTxQId->ucWeightAlg     = pTxQParms->ucWeightAlg;
    pTxQId->ulWeightValue   = pTxQParms->ulWeightValue;
    pTxQId->ucSubPriority   = pTxQParms->ucSubPriority;
    pTxQId->usQueueSize     = pTxQParms->usSize;
    MapXtmOsPrintf ("Q Size2 = %d \n", pTxQId->usQueueSize) ;
    pTxQId->ucDropAlg       = pTxQParms->ucDropAlg;
    pTxQId->ucLoMinThresh   = pTxQParms->ucLoMinThresh;
    pTxQId->ucLoMaxThresh   = pTxQParms->ucLoMaxThresh;
    pTxQId->ucHiMinThresh   = pTxQParms->ucHiMinThresh;
    pTxQId->ucHiMaxThresh   = pTxQParms->ucHiMaxThresh;
    pTxQId->ulMinBitRate    = pTxQParms->ulMinBitRate;
    pTxQId->ulShapingRate   = pTxQParms->ulShapingRate;
    pTxQId->usShapingBurstSize = pTxQParms->usShapingBurstSize;
    pTxQId->ulQueueIndex    = ulTxQueueIdx;

    return( bxStatus );

} /* ReserveOneTxQueue */


/***************************************************************************
 * Function Name: UnreserveOneTxQueue
 * Description  : Unreserves and configures one transmit queue.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_CONNECTION::UnreserveOneTxQueue( PXTMRT_TRANSMIT_QUEUE_ID pTxQId )
{
    BCMXTM_STATUS bxStatus = XTMSTS_SUCCESS;
    UINT32 ulVcid = GetVcid(pTxQId->ulPortId, pTxQId->ulPtmPriority);
    //XtmOsPrintf("%s:Enter\n",__FUNCTION__);

    if( ulVcid >= MAX_VCIDS )
    {
        XtmOsPrintf("UnreserveOneTxQueue: tx queue has NO vcid %d. PortId=%x Prio=%d \n", ulVcid,
                    pTxQId->ulPortId, pTxQId->ulPtmPriority) ;
        return (XTMSTS_STATE_ERROR);
    }

    VcidInfo *pInfo = &m_VcidInfo[ulVcid];

    /* clear the arbitration weight for this queue */
    XP_REGS->ulSsteQueueArbWt[pTxQId->ulQueueIndex]  = 0;

    /* clear the subpriority for this queue */
    XP_REGS->ulSsteQueueScr[pTxQId->ulQueueIndex][0] &=
        ~(SSTE_SCR_PLVLWT_MASK << SSTE_SCR_PLVLWT_SHIFT);

    UnconfigureShaper( QUEUE_SHAPER, pTxQId->ulQueueIndex );

    if( pInfo->ulMpQueueCount > 0 )
        pInfo->ulMpQueueCount--;

    if( pInfo->ulMpIdx == INVALID_INDEX )
    {
        /* pInfo->ulQueueIdx must be pTxQId->ulQueueIndex.
         * Reset it to INVALID_INDEX.
         */
        pInfo->ulQueueIdx = INVALID_INDEX;
    }
    else
    {
        UINT32 i;
        UINT32 ulIdx = pInfo->ulMpIdx;
        UINT32 ulIsPtm = 
            ((m_Addr.ulTrafficType & TRAFFIC_TYPE_ATM_MASK) != TRAFFIC_TYPE_ATM)?
            1 : 0;

        /* Clear the corresponding bit in SstMpQueueMask register */
        XP_REGS->ulSstMpQueueMask[ulIdx / SST_MPG_PER_QUEUE_MASK_REG] &=  \
                                    ~((1 << pTxQId->ulQueueIndex) << (SST_SECOND_MPG_QUEUE_MASK_SHIFT * (ulIdx % SST_MPG_PER_QUEUE_MASK_REG)));

        /* Unreserve this MP group if
         *   - this ATM connection has only one queue left.
         *   - this PTM connection has no queue left.
         */
        if( (!ulIsPtm && pInfo->ulMpQueueCount == 1) ||
            ( ulIsPtm && pInfo->ulMpQueueCount == 0) )
        {
            UINT32 ulQMask = XP_REGS->ulSstMpQueueMask[ulIdx / SST_MPG_PER_QUEUE_MASK_REG];

            XP_REGS->ulTxMpCfg     &= ~(TXMP_GROUP_EN<<(ulIdx * TXMP_GROUP_SIZE));
            XP_REGS->ulSstMpPriArb[(ulIdx / NUM_MPG_PER_ARB_CONFIG_REG)] &= ~(SST_MP_ARB_ALG_PLVLN_MASK << \
                  (MPN_PLVL_ARB_MASK_SHIFT * (ulIdx % NUM_MPG_PER_ARB_CONFIG_REG)));
            XP_REGS->ulSstMpQueueMask[ulIdx / SST_MPG_PER_QUEUE_MASK_REG] &=  \
                                     ~((SST_MPG_QUEUE_MASK) << (SST_SECOND_MPG_QUEUE_MASK_SHIFT * (ulIdx % SST_MPG_PER_QUEUE_MASK_REG)));
            XP_REGS->ulSsteMpVcArbWt[ulIdx]  = 0;
            XP_REGS->ulSsteMpScr[ulIdx][0]  &=
                ~(SSTE_SCR_PLVLWT_MASK<<SSTE_SCR_PLVLWT_SHIFT);
            
            UnconfigureShaper( MPAAL_SHAPER, ulIdx );

            pInfo->ulMpIdx = INVALID_INDEX;

            if( ulIsPtm )
            {
               /* Unconfigure SstUtopiaCfg */
               if (pInfo->ulPtmPri == PTM_PRI_LOW)
               {
                  XP_REGS->ulSstUtopiaCfg &=
                     ~(SST_UTOPIA_CFG_PKTBUFQINDEX_NON_PREEMPT_MASK << (pInfo->ulPort*8));
               }
               else if (pInfo->ulPtmPri == PTM_PRI_HIGH)
               {
                  XP_REGS->ulSstUtopiaCfg &=
                     ~((SST_UTOPIA_CFG_ENABLE_PREEMPTION | SST_UTOPIA_CFG_PKTBUFQINDEX_PREEMPT_MASK)
                          << (pInfo->ulPort*8));
               }

               if (pInfo->ulBondingPort != MAX_PHY_PORTS)
               {
                  /* Unconfigure the bonded path */
                  if (pInfo->ulPtmPri == PTM_PRI_LOW)
                  {
                     XP_REGS->ulSstUtopiaCfg &=
                        ~(SST_UTOPIA_CFG_PKTBUFQINDEX_NON_PREEMPT_MASK << (pInfo->ulBondingPort*8));
                  }
                  else if (pInfo->ulPtmPri == PTM_PRI_HIGH)
                  {
                     XP_REGS->ulSstUtopiaCfg &=
                        ~((SST_UTOPIA_CFG_ENABLE_PREEMPTION | SST_UTOPIA_CFG_PKTBUFQINDEX_PREEMPT_MASK)
                             << (pInfo->ulBondingPort*8));
                  }
               }
            }
            else
            {
                for( i = 0; i < MAX_TRANSMIT_QUEUES; i++ )
                {
                    if( ulQMask & ((1 << i) * (SST_SECOND_MPG_QUEUE_MASK_SHIFT << (ulIdx % SST_MPG_PER_QUEUE_MASK_REG))) )
                    {
                        pInfo->ulQueueIdx = i;
                        ConfigureAtmShaper( QUEUE_SHAPER, pInfo->ulQueueIdx );
                        break;
                    }
                }
            }
        }
    }

    ms_ulTxQueuesInUse &= ~(1 << pTxQId->ulQueueIndex);

    return( bxStatus );

}  /* UnreserveOneTxQueue */


/***************************************************************************
 * Function Name: ConfigureAtmShaper
 * Description  : Reserves and configures available shapers.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_CONNECTION::ConfigureAtmShaper( UINT32 ulShaperType,
                                                  UINT32 ulShaperIdx )
{
    BCMXTM_STATUS bxStatus = XTMSTS_SUCCESS;
    UINT32 ulTdi = m_Cfg.ulTransmitTrafficDescrIndex;
    PXTM_TRAFFIC_DESCR_PARM_ENTRY pEntry = GetTdtEntry( ulTdi );
    UINT32 ulCategory = pEntry->ulServiceCategory;
    UINT32 ulPcr = 0;
    UINT32 ulScr = 0;
    UINT32 ulMbs = 1;
    UINT32 ulMcr = 0;
    BOOL   pcrSitLo = FALSE;
    BOOL   scrSitLo = FALSE;
    BOOL   mcrSitLo = FALSE;
    //XtmOsPrintf("%s:Enter\n",__FUNCTION__);

    /* Determine shaper register values from the traffic descriptor table
     * parameter entry.
     */
    switch( pEntry->ulTrafficDescrType )
    {
    case TDT_ATM_NO_TRAFFIC_DESCRIPTOR:
        break;

    case TDT_ATM_NO_CLP_NO_SCR:
    case TDT_ATM_NO_CLP_TAGGING_NO_SCR:
    case TDT_ATM_CLP_TRANSPARENT_NO_SCR:
        ulPcr = pEntry->ulPcr;
        break;

    case TDT_ATM_NO_CLP_SCR:
    case TDT_ATM_CLP_TRANSPARENT_SCR:
        ulPcr = pEntry->ulPcr;
        ulScr = pEntry->ulScr;
        ulMbs = pEntry->ulMbs;
        break;

    case TDT_ATM_CLP_NO_TAGGING_MCR:
        ulPcr = pEntry->ulPcr;
        break;

    case TDT_ATM_PTM_MAX_BIT_RATE_SCR:
        /* TBD. What is the configured units for MBR? Assume bits per second.*/
        ulScr = pEntry->ulScr;
        break;

    default:
        bxStatus = XTMSTS_PARAMETER_ERROR;
        break;
    }

    ulMcr = pEntry->ulMcr;
    
//    XtmOsPrintf("===>pcr=%u scr=%u mbs=%u mcr=%u SitUt=%u SitLoUt=%u\n",
//            ulPcr, ulScr, ulMbs, ulMcr, ms_ulSitUt, ms_ulSitLoUt);

    if( bxStatus == XTMSTS_SUCCESS )
    {
        volatile UINT32 *pulGts = NULL;
        volatile UINT32 *pulScr = NULL;
        volatile UINT32 *pulPcr = NULL;
        
        /* Calculate the number of SIT pulses per sec.
         * Note that ms_ulSitUt is the number of SAR clocks per SIT pulse.
         */
        UINT32 ulSitsPerSec   = SAR_CLOCK / ms_ulSitUt;
        UINT32 ulSitsPerSecLo = SAR_CLOCK / ms_ulSitLoUt;

        /* Calculate SITLMT for PCR, SCR and MCR.
         * SITLMT is the number of SIT pulses per cell transmit.
         * Note that ulPcr, ulScr and ulMcr are in cells per second.
         */
        UINT32 ulPcrSitLmt = 0;
        UINT32 ulScrSitLmt = 0;
        UINT32 ulMcrSitLmt = 0;

        if( ulPcr )
        {
            ulPcrSitLmt = ulSitsPerSec / ulPcr;
            if( ulPcrSitLmt == 0 )
            {
                XtmOsPrintf("ConfigureAtmShaper: PCR_SITLMT=0x0 is invalid. No shaping.\n");
                return XTMSTS_PARAMETER_ERROR;
            }
            else if( ulPcrSitLmt > 0x7FFF )
            {
//               XtmOsPrintf("ConfigureAtmShaper: PCR_SITLMT=0x%x\n", ulPcrSitLmt);
            
                /* Try using SIT_LO_CNT */
                ulPcrSitLmt = ulSitsPerSecLo / ulPcr;
                if( ulPcrSitLmt == 0 || ulPcrSitLmt > 0x7FFF )
                {
                    XtmOsPrintf("ConfigureAtmShaper: PCR_SITLMT=0x%x is invalid. No shaping.\n",
                                ulPcrSitLmt);
                    return XTMSTS_PARAMETER_ERROR;
                }
                pcrSitLo = TRUE;
            }
        }
        
        if( ulScr )
        {
            ulScrSitLmt = ulSitsPerSec / ulScr;
            if( ulScrSitLmt == 0 )
            {
                XtmOsPrintf("ConfigureAtmShaper: SCR_SITLMT=0x0 is invalid. No shaping.\n");
                return XTMSTS_PARAMETER_ERROR;
            }
            else if( ulScrSitLmt > 0x7FFF )
            {
//               XtmOsPrintf("ConfigureAtmShaper: SCR_SITLMT=0x%x\n", ulScrSitLmt);
            
                /* Try using SIT_LO_CNT */
                ulScrSitLmt = ulSitsPerSecLo / ulScr;
                if( ulScrSitLmt == 0 || ulScrSitLmt > 0x7FFF )
                {
                    XtmOsPrintf("ConfigureAtmShaper: SCR_SITLMT=0x%x is invalid. No shaping.\n",
                                ulScrSitLmt);
                    return XTMSTS_PARAMETER_ERROR;
                }
                scrSitLo = TRUE;
            }
        }
        
        if( ulMcr )
        {
            ulMcrSitLmt = ulSitsPerSec / ulMcr;
            if( ulMcrSitLmt == 0 )
            {
                XtmOsPrintf("ConfigureAtmShaper: MCR_SITLMT=0x0 is invalid. No shaping.\n");
                return XTMSTS_PARAMETER_ERROR;
            }
            else if( ulMcrSitLmt > 0x7FFF )
            {
//               XtmOsPrintf("ConfigureAtmShaper: MCR_SITLMT=0x%x\n", ulMcrSitLmt);
            
                /* Try using SIT_LO_CNT */
                ulMcrSitLmt = ulSitsPerSecLo / ulMcr;
                if( ulMcrSitLmt == 0 || ulMcrSitLmt > 0x7FFF )
                {
                    XtmOsPrintf("ConfigureAtmShaper: MCR_SITLMT=0x%x is invalid. No shaping.\n",
                                ulMcrSitLmt);
                    return XTMSTS_PARAMETER_ERROR;
                }
                mcrSitLo = TRUE;
            }
        }
        
        if( ulShaperType == QUEUE_SHAPER )
        {
            pulGts = &XP_REGS->ulSsteQueueGtsCfg[ulShaperIdx];
            pulScr = &XP_REGS->ulSsteQueueScr[ulShaperIdx][0];
            pulPcr = &XP_REGS->ulSsteQueuePcr[ulShaperIdx][0];
        }
        else if( ulShaperType == MPAAL_SHAPER )
        {
            pulGts = &XP_REGS->ulSsMpGtsCfg[ulShaperIdx];
            pulScr = &XP_REGS->ulSsteMpScr[ulShaperIdx][0];
            pulPcr = &XP_REGS->ulSsteMpPcr[ulShaperIdx][0];
        }

        if( pulGts )
        {
            *pulGts = 0;
            /* Preserve SCR priority which is used for arbitration. */
            pulScr[0] &= (SSTE_SCR_PLVLWT_MASK << SSTE_SCR_PLVLWT_SHIFT);
            pulScr[1] = pulScr[2] = 0;
            pulPcr[0] = pulPcr[1] = pulPcr[2] = 0;

            switch( ulCategory )
            {
            case SC_CBR:
                pulScr[0] |= (1 << SSTE_SCR_INCR_SHIFT);
                pulScr[1]  = (CDVT << SSTE_SCR_ACCLMT_SHIFT);
                pulScr[2]  = (ulPcrSitLmt << SSTE_SCR_SITLMT_SHIFT);
                *pulGts    = SST_GTS_SCR_EN;
                if( pcrSitLo )
                {
                    *pulGts |= SST_GTS_SITLO_SCR_EN;
                }
                break;

            case SC_UBR:
                if( ulPcrSitLmt )
                {
                    /* UBR with PCR */
                    pulScr[0] |= (1 << SSTE_SCR_INCR_SHIFT);
                    pulScr[1]  = (CDVT << SSTE_SCR_ACCLMT_SHIFT);
                    pulScr[2]  = (ulPcrSitLmt << SSTE_SCR_SITLMT_SHIFT);
                    *pulGts   |= SST_GTS_SCR_EN;
                    if( pcrSitLo )
                    {
                        *pulGts |= SST_GTS_SITLO_SCR_EN;
                    }
                }

                /* MCR is only supported with a UBR configuration. */
                if( ulMcrSitLmt )
                {
                    pulPcr[0] |= (1 << SSTE_PCR_INCR_SHIFT) |
                                 (MCR_PRIORITY << SSTE_PCR_PLVLWT_SHIFT);
                    pulPcr[1]  = (CDVT << SSTE_PCR_ACCLMT_SHIFT);
                    pulPcr[2]  = (ulMcrSitLmt << SSTE_PCR_SITLMT_SHIFT);
                    *pulGts   |= SST_GTS_PCR_PRI_LEVEL_EN;
                    if( mcrSitLo )
                    {
                        *pulGts |= SST_GTS_SITLO_PCR_EN;
                    }
                }
                break;

            case SC_RT_VBR:
                /* TBD: Set the arbitration type for this VBR VC priority
                 * level to "SP by Age"
                 */
                /* fall through */
                            
            case SC_NRT_VBR:
                pulScr[0] |= (1 << SSTE_SCR_INCR_SHIFT);
                pulScr[1]  = (CDVT << SSTE_SCR_ACCLMT_SHIFT);
                pulScr[2]  = (ulScrSitLmt << SSTE_SCR_SITLMT_SHIFT);
                pulPcr[0]  = (1 << SSTE_PCR_INCR_SHIFT);
                pulPcr[1]  = (ulMbs << SSTE_PCR_ACCLMT_SHIFT);
                pulPcr[2]  = (ulPcrSitLmt << SSTE_PCR_SITLMT_SHIFT);
                *pulGts    = SST_GTS_PCR_EN | SST_GTS_SCR_EN | SST_GTS_VBR_MODE_EN;
                if( pcrSitLo )
                {
                    *pulGts |= SST_GTS_SITLO_PCR_EN;
                }
                if( scrSitLo )
                {
                    *pulGts |= SST_GTS_SITLO_SCR_EN;
                }
                break;
            }
        }
    }

    return( bxStatus );

} /* ConfigureATMShaper */


#define  GFAST_SCHED_DIV_FACTOR        1333

/***************************************************************************
 * Function Name: ConfigurePtmQueueShaper
 * Description  : Reserves and configures available shapers.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
UINT16 XTM_CONNECTION::CalculateMbs( UINT32 ulMinBitRate,
                                     UINT16 usMbs )
{
   UINT16 calcMbs ;
   //XtmOsPrintf("%s:Enter\n",__FUNCTION__);

   if (m_ulXTMLinkMode == XTM_LINK_MODE_GFAST) {
      /* new Mbs = (minrate)/8/(10^9/750000). Explanation that this is suitable
      ** for asymmetric Gfast is in SWBCACPE-23733 */
      calcMbs = (ulMinBitRate/8)/GFAST_SCHED_DIV_FACTOR ;
   }
   else {
      calcMbs = usMbs ;
   }
   return (calcMbs) ;
}

/***************************************************************************
 * Function Name: ConfigurePtmQueueShaper
 * Description  : Reserves and configures available shapers.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_CONNECTION::ConfigurePtmQueueShaper( UINT32 ulShaperIdx,
                                                       UINT32 ulMinBitRate,
                                                       UINT32 ulShapeRate,
                                                       UINT16 usMbs )
{
    volatile UINT32 *pulGts = &XP_REGS->ulSsteQueueGtsCfg[ulShaperIdx];
    volatile UINT32 *pulScr = &XP_REGS->ulSsteQueueScr[ulShaperIdx][0];
    volatile UINT32 *pulPcr = &XP_REGS->ulSsteQueuePcr[ulShaperIdx][0];
    UINT32 ulGts = 0;
    //XtmOsPrintf("%s:Enter\n",__FUNCTION__);
    
    /* Calculate the number of SIT pulses per sec.
     * Note that ms_ulSitUt is the number of SAR clocks per SIT pulse.
     */
    UINT32 ulSitsPerSec   = SAR_CLOCK / ms_ulSitUt;
    UINT32 ulSitsPerSecLo = SAR_CLOCK / ms_ulSitLoUt;

    usMbs = CalculateMbs (ulMinBitRate, usMbs) ;

//    XtmOsPrintf("===>ConfigurePtmQueueShaper: ShaperIdx=%u MinRate=%u ShapeRate=%u Mbs=%u SitUt=%u SitLoUT=%u\n",
//            ulShaperIdx, ulMinBitRate, ulShapeRate, usMbs, ms_ulSitUt, ms_ulSitLoUt);

    if (ulShapeRate && usMbs)
    {
        /* Calculate SCR.
         * ulScr is the number of token bucket refills per second to support the
         * the shaping rate in bytes per second.  ulShapeRate is in bits per sec.
         * The number of tokens per refill is PTM_TOKEN_BUCKET_INCREMENT and is
         * set to SCR_INCR.
         */ 
//        UINT32 ulScr = ulShapeRate / (PTM_TOKEN_BUCKET_INCREMENT * 8);

        /* Calculate SITLMT for SCR.
         * SITLMT is the number of SIT pulses between token bucket refills.
         */
//        UINT32 ulScrSitLmt = ulSitsPerSec / ulScr;
        UINT32 ulScrSitLmt = (ulSitsPerSec * PTM_TOKEN_BUCKET_INCREMENT / ulShapeRate) * 8;

        if (ulScrSitLmt == 0)
        {
           XtmOsPrintf("ConfigurePtmQueueShaper: SCR_SITLMT=0x0 is invalid. No shaping.\n");
           return XTMSTS_PARAMETER_ERROR;
        }
        else if (ulScrSitLmt > 0x7FFF)
        {
//           XtmOsPrintf("ConfigurePtmQueueShaper: SCR_SITLMT=0x%x\n", ulScrSitLmt);
           
           /* Try using SIT_LO_CNT */
//           ulScrSitLmt  = ulSitsPerSecLo / ulScr;
           ulScrSitLmt = (ulSitsPerSecLo * PTM_TOKEN_BUCKET_INCREMENT / ulShapeRate) * 8;
           if (ulScrSitLmt == 0 || ulScrSitLmt > 0x7FFF)
           {
              XtmOsPrintf("ConfigurePtmQueueShaper: SCR_SITLMT=0x%x is invalid. No shaping.\n",
                          ulScrSitLmt);
              return XTMSTS_PARAMETER_ERROR;
           }
           ulGts |= SST_GTS_SCR_EN | SST_GTS_SITLO_SCR_EN | SST_GTS_PKT_MODE_SHAPING_EN;
        }
        else
        {
           ulGts |= SST_GTS_SCR_EN | SST_GTS_PKT_MODE_SHAPING_EN;
        }
        
        /* Preserve SCR priority which is used for arbitration. */
        pulScr[0] &= (SSTE_SCR_PLVLWT_MASK << SSTE_SCR_PLVLWT_SHIFT);
        pulScr[0] |= (PTM_TOKEN_BUCKET_INCREMENT << SSTE_SCR_INCR_SHIFT);
        pulScr[1]  = (usMbs << SSTE_SCR_ACCLMT_SHIFT);
        pulScr[2]  = (ulScrSitLmt << SSTE_SCR_SITLMT_SHIFT);
    }

    if (ulMinBitRate && usMbs)
    {
        /* Calculate PCR.
         * ulPcr is the number of token bucket refills per second to support the
         * the shaping rate in bytes per second.  ulShapeRate is in bits per sec.
         * The number of tokens per refill is PTM_TOKEN_BUCKET_INCREMENT and is
         * set to PCR_INCR.
         */ 
//        UINT32 ulPcr = ulMinBitRate / (PTM_TOKEN_BUCKET_INCREMENT * 8);

        /* Calculate SITLMT for PCR.
         * SITLMT is the number of SIT pulses between token bucket refills.
         */
//        UINT32 ulPcrSitLmt = ulSitsPerSec / ulPcr;
        UINT32 ulPcrSitLmt = (ulSitsPerSec * PTM_TOKEN_BUCKET_INCREMENT / ulMinBitRate) * 8;

        if (ulPcrSitLmt == 0)
        {
           XtmOsPrintf("ConfigurePtmQueueShaper: PCR_SITLMT=0x0 is invalid. No shaping.\n");
           return XTMSTS_PARAMETER_ERROR;
        }
        else if (ulPcrSitLmt > 0x7FFF)
        {
//           XtmOsPrintf("ConfigurePtmQueueShaper: PCR_SITLMT=0x%x\n", ulPcrSitLmt);
           
           /* Try using SIT_LO_CNT */
//           ulPcrSitLmt  = ulSitsPerSecLo / ulPcr;
           ulPcrSitLmt = (ulSitsPerSecLo * PTM_TOKEN_BUCKET_INCREMENT / ulMinBitRate) * 8;
           if (ulPcrSitLmt == 0 || ulPcrSitLmt > 0x7FFF)
           {
              XtmOsPrintf("ConfigurePtmQueueShaper: PCR_SITLMT=0x%x is invalid. No shaping.\n",
                          ulPcrSitLmt);
              return XTMSTS_PARAMETER_ERROR;
           }
           ulGts |= SST_GTS_SITLO_PCR_EN | SST_GTS_PCR_PRI_LEVEL_EN | SST_GTS_PKT_MODE_SHAPING_EN;
        }
        else
        {
           ulGts |= SST_GTS_PCR_PRI_LEVEL_EN | SST_GTS_PKT_MODE_SHAPING_EN;
        }
        
        pulPcr[0] = (PTM_TOKEN_BUCKET_INCREMENT << SSTE_PCR_INCR_SHIFT) |
                    (MCR_PRIORITY << SSTE_PCR_PLVLWT_SHIFT);
        pulPcr[1] = (usMbs << SSTE_PCR_ACCLMT_SHIFT);
        pulPcr[2] = (ulPcrSitLmt << SSTE_PCR_SITLMT_SHIFT);
    }
    *pulGts = ulGts;

    return XTMSTS_SUCCESS;

} /* ConfigurePtmQueueShaper */


/***************************************************************************
 * Function Name: UnconfigureShaper
 * Description  : Reserves and configures available shapers.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_CONNECTION::UnconfigureShaper( UINT32 ulShaperType,
                                                 UINT32 ulShaperIdx )
{
    BCMXTM_STATUS bxStatus  = XTMSTS_SUCCESS;
    volatile UINT32 *pulScr = NULL;
    volatile UINT32 *pulPcr = NULL;
    //XtmOsPrintf("%s:Enter\n",__FUNCTION__);

    if( ulShaperType == QUEUE_SHAPER )
    {
        XP_REGS->ulSsteQueueGtsCfg[ulShaperIdx] = 0;
        pulScr = &XP_REGS->ulSsteQueueScr[ulShaperIdx][0];
        pulPcr = &XP_REGS->ulSsteQueuePcr[ulShaperIdx][0];
    }
    else if( ulShaperType == MPAAL_SHAPER )
    {
        XP_REGS->ulSsMpGtsCfg[ulShaperIdx] = 0;
        pulScr = &XP_REGS->ulSsteMpScr[ulShaperIdx][0];
        pulPcr = &XP_REGS->ulSsteMpPcr[ulShaperIdx][0];
    }

    if( pulScr )
    {
        /* Preserve SCR priority which is used for arbitration. */
        pulScr[0] &= (SSTE_SCR_PLVLWT_MASK << SSTE_SCR_PLVLWT_SHIFT);
        pulScr[1]  = pulScr[2] = 0;
        pulPcr[0]  = pulPcr[1] = pulPcr[2] = 0;
    }

    return( bxStatus );

} /* UnconfigureShaper */


/***************************************************************************
 * Function Name: ConfigureArbitration
 * Description  : Configures arbitration weight and priority values.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
void XTM_CONNECTION::ConfigureArbitration( UINT32 ulQueueIdx,
                                           UINT32 ulMpIdx,
                                           UINT32 ulPort,
                                           UINT32 ulWeightAlg,
                                           UINT32 ulWeightValue,
                                           UINT32 ulSubPriority )
{
    //XtmOsPrintf("%s:Enter\n",__FUNCTION__);
    if( ulQueueIdx != INVALID_INDEX )
    {
        /* Set the weight for this queue. */
        XP_REGS->ulSsteQueueArbWt[ulQueueIdx] =
            (ulWeightValue<<SSTE_SETARBWT_SHIFT) | (ulPort<<SSTE_UTPT_SHIFT);
        /* Set the subpriority for this queue. */
        XP_REGS->ulSsteQueueScr[ulQueueIdx][0] &=
            ~(SSTE_SCR_PLVLWT_MASK << SSTE_SCR_PLVLWT_SHIFT);
        XP_REGS->ulSsteQueueScr[ulQueueIdx][0] |=
            (ulSubPriority<<SSTE_SCR_PLVLWT_SHIFT);

        if( ulMpIdx != INVALID_INDEX )
        {
            /* Put this queue in the MP group */
            XP_REGS->ulSstMpQueueMask[ulMpIdx / SST_MPG_PER_QUEUE_MASK_REG] |=  \
                                     (1 << ulQueueIdx) << (SST_SECOND_MPG_QUEUE_MASK_SHIFT * (ulMpIdx % SST_MPG_PER_QUEUE_MASK_REG));

            /* Set the weight alg for this subpriority level within the MP group */
            if( ulWeightAlg == WA_WFQ ) {
                XP_REGS->ulSstMpPriArb[(ulMpIdx / NUM_MPG_PER_ARB_CONFIG_REG)] |= ((1 << ulSubPriority) << \
                  (MPN_PLVL_ARB_MASK_SHIFT * (ulMpIdx % NUM_MPG_PER_ARB_CONFIG_REG)));
            }
            else { /* WA_CWRR or WA_DISABLED */
                XP_REGS->ulSstMpPriArb[(ulMpIdx / NUM_MPG_PER_ARB_CONFIG_REG)] &= ~((1 << ulSubPriority) << \
                  (MPN_PLVL_ARB_MASK_SHIFT * (ulMpIdx % NUM_MPG_PER_ARB_CONFIG_REG)));
            }
        }
    }
} /* ConfigureArbitration */


/***************************************************************************
 * Function Name: UnreserveVcidsAndTxShapers
 * Description  : Clear VCID from Tx Table and Rx CAM and clear shapers used
 *                by this connection.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_CONNECTION::UnreserveVcidsAndTxShapers( void )
{
    UINT32 i, j;
    volatile UINT32 *pulScr, *pulPcr;
    UINT32 ulPafVcId;
    UINT8 ucVcid;
    //XtmOsPrintf("%s:Enter\n",__FUNCTION__);


    /* Reset queue arbitration and shaping registers. */
    for( i = 0; i < m_ulTxQIdsSize; i++ )
    {
        j = m_TxQIds[i].ulQueueIndex;
        ms_ulTxQueuesInUse &= ~(1 << j);
        XP_REGS->ulTxChannelCfg[j]    = 0;
        XP_REGS->ulSsteQueueArbWt[j]  = 0;
        XP_REGS->ulSsteQueueGtsCfg[j] = 0;
        pulScr = &XP_REGS->ulSsteQueueScr[j][0];
        pulPcr = &XP_REGS->ulSsteQueuePcr[j][0];
        pulScr[0] = pulScr[1] = pulScr[2] = 0;
        pulPcr[0] = pulPcr[1] = pulPcr[2] = 0;
    }

    m_ulTxQIdsSize = 0;
    memset(m_TxQIds, 0x00, sizeof(m_TxQIds));

    /* Reset MP group arbitration and shaping registers. */
    VcidInfo *pInfo;
    for( i = 0, pInfo = m_VcidInfo; i < MAX_VCIDS; i++, pInfo++ )
    {
        pInfo->ulMpQueueCount = 0;
        pInfo->ulQueueIdx     = INVALID_INDEX;
        pInfo->pConnArb       = NULL;

        if( pInfo->ulMpIdx != INVALID_INDEX )
        {
            XP_REGS->ulSsteMpVcArbWt[pInfo->ulMpIdx] = 0;
            XP_REGS->ulTxMpCfg &=
                ~(TXMP_GROUP_EN << pInfo->ulMpIdx * TXMP_GROUP_SIZE);
            XP_REGS->ulSstMpQueueMask[pInfo->ulMpIdx / SST_MPG_PER_QUEUE_MASK_REG] &=  \
                                     ~((SST_MPG_QUEUE_MASK) << (SST_SECOND_MPG_QUEUE_MASK_SHIFT * (pInfo->ulMpIdx % SST_MPG_PER_QUEUE_MASK_REG)));
            XP_REGS->ulSsMpGtsCfg[pInfo->ulMpIdx]     = 0;
            pulScr = &XP_REGS->ulSsteMpScr[pInfo->ulMpIdx][0];
            pulPcr = &XP_REGS->ulSsteMpPcr[pInfo->ulMpIdx][0];
            pulScr[0] = pulScr[1] = pulScr[2] = 0;
            pulPcr[0] = pulPcr[1] = pulPcr[2] = 0;

            pInfo->ulMpIdx = INVALID_INDEX;
        }
    }
    
    /* Unconfigure SstUtopiaCfg */
    XP_REGS->ulSstUtopiaCfg = 0;

    /* Reset Tx / Rx VC mapping registers. */
    for( i = 0; i < MAX_INTERFACES; i++ )
        for( j = 0; j < MAX_PTM_PRIORITIES; j++ )
        {
            if( (ucVcid = m_ucVcids[i][j]) != INVALID_VCID )
            {
                XP_REGS->ulRxVpiVciCam[(ucVcid * 2) + 1] = 0;
                XP_REGS->ulRxVpiVciCam[ucVcid * 2] = 0;
                XP_REGS->ulTxVpiVciTable[ucVcid] = 0;
                m_ucVcids[i][j] = INVALID_VCID;
                m_pulRxVpiVciCamShadow[ucVcid] = 0;

                ulPafVcId = XP_REGS->ulRxPafVcid;
                if (((ulPafVcId>>(i * 4)) & RXPAF_VCID_MASK) == ucVcid)
                {
                    XP_REGS->ulRxPafVcid &= ~(RXPAF_VCID_MASK<<(i * RXPAF_VCID_SHIFT));
                    XP_REGS->ulRxPafVcid |= (RXPAF_INVALID_VCID << (i*RXPAF_VCID_SHIFT)) ;
                }
                ulPafVcId >>= RXPAF_VCID_PREEMPTION_SHIFT;
                if (((ulPafVcId>>(i * 4)) & RXPAF_VCID_MASK) == ucVcid)
                {
                    XP_REGS->ulRxPafVcid &=
                        ~(RXPAF_VCID_MASK<<((i * RXPAF_VCID_SHIFT)+RXPAF_VCID_PREEMPTION_SHIFT));
                    XP_REGS->ulRxPafVcid |= (RXPAF_INVALID_VCID << 
                                             ((i*RXPAF_VCID_SHIFT)+RXPAF_VCID_PREEMPTION_SHIFT)) ;
                    XP_REGS->ulRxPBufMuxVcidDefQueueId[0] &=
                        ~(RXPBUF_MUXVCID_DEF_QID_MASK<<(ucVcid * 2));
                }
            }
        }

    return( XTMSTS_SUCCESS );

} /* UnreserveVcidsAndTxShapers */


/***************************************************************************
 * Function Name: WaitForTxComplete
 * Description  : After flushing action on the tx queues, this function
 *                waits for all the data to be completely drained and
 *                nothing is pending to be transmitted out.
 * Returns      : XTMSTS_SUCCESS
 ***************************************************************************/
void XTM_CONNECTION:: WaitForTxComplete (UINT32 ulQueueIdx)
{
   unsigned int loopCount, waitCnt ;
   volatile UINT32 pktOctCountPrev = 0, pktOctCountCurr;

   loopCount = 0 ;

   while (1) {

      pktOctCountCurr = XP_REGS->ulTxVcPktOctCnt[ulQueueIdx];
      if (pktOctCountCurr == pktOctCountPrev)
         loopCount++ ;
      else {
         loopCount = ((loopCount!=0) ? loopCount-1 : loopCount) ;
         pktOctCountPrev = pktOctCountCurr ;
      }

      if (loopCount == XTM_WAIT_TRIAL_DATA_COUNT_SYNC)
         break ;

      for (waitCnt=XTM_WAIT_TRIAL_DATA_BUSY; waitCnt>0; waitCnt--)
         asm ("nop") ;

   } /* while (1) */
}

/***************************************************************************
 * Function Name: CheckTransmitQueues
 * Description  : Adds and deletes transmit queues from the current
 *                configuration in order to match the new configuration.
 * Returns      : XTMSTS_SUCCESS
 ***************************************************************************/
void XTM_CONNECTION::CheckTransmitQueues( void )
{
    BCMXTM_STATUS bxStatus = XTMSTS_SUCCESS;
    XTMRT_TRANSMIT_QUEUE_ID TxQIdsSave[MAX_TRANSMIT_QUEUES];
    UINT32 i, j, k, ulFound, ulTxQIdsSizeSave = 0;
    PXTM_TRANSMIT_QUEUE_PARMS pTxQParms;
    PXTMRT_TRANSMIT_QUEUE_ID pTxQId;
    //XtmOsPrintf("%s:Enter\n",__FUNCTION__);

    if (m_ulConnSem == NULL) {
        XtmOsPrintf ("bcmxtmcfg: CTQ- ConnSem NULL\n");        
    }

    XtmOsRequestLock(m_ulConnSem);

    if( m_hDev != INVALID_HANDLE )
    {
       ((*m_pfnXtmrtReq) (m_hDev, XTMRT_CMD_STOP_ALL_TX_QUEUE, NULL)) ;

       /* Check transmit queues that have been removed.*/
       for( i = 0, pTxQId = m_TxQIds; i < m_ulTxQIdsSize; i++, pTxQId++ )
       {
          for( j = 0, pTxQParms = m_Cfg.TransmitQParms, ulFound = 0;
                j < m_Cfg.ulTransmitQParmsSize;
                j++, pTxQParms++ )
          {
             if( pTxQId->ucQosQId      == pTxQParms->ucQosQId &&
                   pTxQId->ulPortId      == pTxQParms->ulPortId &&
                   pTxQId->ucSubPriority == pTxQParms->ucSubPriority &&
                   pTxQId->ulPtmPriority == pTxQParms->ulPtmPriority &&
                   pTxQId->ucWeightAlg   == pTxQParms->ucWeightAlg &&
                   pTxQId->ulWeightValue == pTxQParms->ulWeightValue )
             {
                memcpy(&TxQIdsSave[ulTxQIdsSizeSave++], pTxQId,
                      sizeof(XTMRT_TRANSMIT_QUEUE_ID));
                ulFound = 1;
                break;
             }
          }

          if (!ulFound) {

             /* Delete the transmit queue. */

             /* TBD. The DMA needs to be stopped for each Tx queue in the
              *      MP group while this block of code executes.
              * Make sure to stop the xmit entities to SAR before effecting SAR configurations.
              */
             if (((*m_pfnXtmrtReq) (m_hDev, XTMRT_CMD_UNSET_TX_QUEUE, pTxQId)) == 0) {

                XtmOsPrintf ("XTM W2B %u\n", XtmOsGetTimeStampMs()) ;
                WaitForTxComplete(pTxQId->ulQueueIndex) ;
                XtmOsPrintf ("XTM W2E %u\n", XtmOsGetTimeStampMs()) ;

                bxStatus = UnreserveOneTxQueue(pTxQId);

                for (k=0x7ff; k>0; k--)
                   asm ("nop") ; 
             }
             else
                bxStatus = XTMSTS_STATE_ERROR ;
          }
       } /* for i */

       m_ulTxQIdsSize = ulTxQIdsSizeSave;
       memcpy(m_TxQIds, TxQIdsSave, m_ulTxQIdsSize * sizeof(XTMRT_TRANSMIT_QUEUE_ID));

       /* Check transmit queues that have been added. */
       for( j = 0, pTxQParms = m_Cfg.TransmitQParms;
             j < m_Cfg.ulTransmitQParmsSize;
             j++, pTxQParms++ )
       {
          for( i = 0, pTxQId = m_TxQIds, ulFound=0;
                i < m_ulTxQIdsSize;
                i++, pTxQId++ )
          {
             if( pTxQId->ucQosQId      == pTxQParms->ucQosQId &&
                 pTxQId->ulPortId      == pTxQParms->ulPortId &&
                 pTxQId->ucSubPriority == pTxQParms->ucSubPriority &&
                 pTxQId->ulPtmPriority == pTxQParms->ulPtmPriority &&
                 pTxQId->ucWeightAlg   == pTxQParms->ucWeightAlg &&
                 pTxQId->ulWeightValue == pTxQParms->ulWeightValue )
             {
                ulFound = 1;
                break;
             }
          }

          if (!ulFound) {

             /* Add the transmit queue.
              * Before setting up the transmit entities/DMA, we need to
              * ensure SAR Tx is configured.
              */
             if (pTxQParms->usSize != 0)
                bxStatus = ReserveOneTxQueue(pTxQParms);

             for (k=0x7ffff; k>0; k--)
                asm ("nop") ; 

             if( bxStatus == XTMSTS_SUCCESS )
             {
                (*m_pfnXtmrtReq) (m_hDev, XTMRT_CMD_SET_TX_QUEUE,
                      &m_TxQIds[m_ulTxQIdsSize - 1]);
             }
             else
                XtmOsPrintf("bcmxtmcfg: CTQ tx queue reservation failed.\n");
          }
       } /* for j */

       ((*m_pfnXtmrtReq) (m_hDev, XTMRT_CMD_START_ALL_TX_QUEUE, NULL)) ;

    } /* if( m_hDev != INVALID_HANDLE ) */
    else
       XtmOsPrintf ("bcmxtmcfg: CTQ Inv Dev Hdl\n");


    XtmOsReleaseLock(m_ulConnSem);

} /* CheckTransmitQueues */


/***************************************************************************
 * Function Name: GetTdtEntry
 * Description  : Returns a traffic descriptor table entry for the specified
 *                index.
 * Returns      : Traffic descriptor table entry.
 ***************************************************************************/
PXTM_TRAFFIC_DESCR_PARM_ENTRY XTM_CONNECTION::GetTdtEntry( UINT32 ulTdtIdx )
{
    static XTM_TRAFFIC_DESCR_PARM_ENTRY Entry =
        {0, TDT_ATM_NO_TRAFFIC_DESCRIPTOR, 0, 0, 0, 0, SC_UBR};
    PXTM_TRAFFIC_DESCR_PARM_ENTRY pFound = &Entry;
    PXTM_TRAFFIC_DESCR_PARM_ENTRY pEntry;
    UINT32 i;
    //XtmOsPrintf("%s:Enter\n",__FUNCTION__);

    for( i = 0, pEntry = m_pTrafficDescrEntries;
         i < m_ulNumTrafficDescrEntries;
         i++, pEntry++ )
    {
        if( pEntry->ulTrafficDescrIndex == ulTdtIdx )
        {
            pFound = pEntry;
            break;
        }
    }

    return( pFound );

} /* GetTdtEntry */


/***************************************************************************
 * Function Name: GetVcid
 * Description  : Returns the vcid of a connection
 * Returns      : vcid.
 ***************************************************************************/
UINT32 XTM_CONNECTION::GetVcid( UINT32 ulPortId, UINT32 ulPtmPriority )
{
    UINT32 ulPort, ulPtmPriIdx, ulVcid;
    //XtmOsPrintf("%s:Enter\n",__FUNCTION__);

    ulPort = PORTID_TO_PORT(ulPortId);
    ulPtmPriIdx = ulPtmPriority? (ulPtmPriority - PTM_PRI_LOW) : 0;
    ulVcid = m_ucVcids[ulPort][ulPtmPriIdx];

    return( ulVcid );

} /* GetVcid */


/***************************************************************************
 * Function Name: CopyVcids
 * Description  : Copies VCIDs for this connection to an array.
 * Returns      : number of VCIDs copied
 ***************************************************************************/
UINT32 XTM_CONNECTION::CopyVcids( UINT8 *pucVcids )
{
    UINT32 ulNum = 0;
    //XtmOsPrintf("%s:Enter\n",__FUNCTION__);

    for( UINT32 i = 0; i < MAX_INTERFACES; i++ )
        for( UINT32 j = 0; j < MAX_PTM_PRIORITIES; j++ )
            if( m_ucVcids[i][j] != INVALID_VCID)
                pucVcids[ulNum++] = m_ucVcids[i][j];

    return( ulNum );
}


/***************************************************************************
 * XTM_CONNECTION_TABLE Class
 ***************************************************************************/


/***************************************************************************
 * Function Name: XTM_CONNECTION_TABLE
 * Description  : Constructor for the XTM connection table class.
 * Returns      : None.
 ***************************************************************************/
XTM_CONNECTION_TABLE::XTM_CONNECTION_TABLE( void )
{
   //XtmOsPrintf("%s:Enter\n",__FUNCTION__);
#if defined(CONFIG_ARM64)
    m_ulMuSem = (unsigned long)XtmOsCreateSem(1);
#else
    m_ulMuSem = (UINT32)XtmOsCreateSem(1);
#endif
    memset (m_pConnTable, 0x00, sizeof(m_pConnTable)) ;

} /* XTM_CONNECTION_TABLE */


/***************************************************************************
 * Function Name: ~XTM_CONNECTION_TABLE
 * Description  : Destructor for the XTM connection table class.
 * Returns      : None.
 ***************************************************************************/
XTM_CONNECTION_TABLE::~XTM_CONNECTION_TABLE( void )
{
   //XtmOsPrintf("%s:Enter\n",__FUNCTION__);
    XtmOsDeleteSem(m_ulMuSem);
} /* ~XTM_CONNECTION_TABLE */

/***************************************************************************
 * Function Name: Initialize 
 * Description  : Constructor for the XTM connection table class.
 * Returns      : None.
 ***************************************************************************/
void XTM_CONNECTION_TABLE::Initialize( XTM_CONNECTION_TABLE *pConnTable )
{
   //XtmOsPrintf("%s:Enter\n",__FUNCTION__);

   memset(pConnTable, 0x00, sizeof(XTM_CONNECTION_TABLE));

#if defined(CONFIG_ARM64)
   m_ulMuSem = (unsigned long)XtmOsCreateSem(1);
#else
   m_ulMuSem = (UINT32)XtmOsCreateSem(1);
#endif
   pConnTable->m_ulAdslThreshold = XTM_ADSL_QUEUE_DROP_THRESHOLD ;
   pConnTable->m_ulVdslThreshold = XTM_VDSL_QUEUE_DROP_THRESHOLD ;
   pConnTable->m_ulVdslRtxThreshold = XTM_VDSL_RTX_QUEUE_DROP_THRESHOLD ;
   pConnTable->m_ulGfastThreshold = XTM_GFAST_QUEUE_DROP_THRESHOLD ;

} /* XTM_CONNECTION_TABLE */


void XTM_CONNECTION_TABLE::setThreshold( UINT32 mode, UINT32 ulThreshold)
{
   if(m_ulMuSem == NULL)
      return;

   XtmOsRequestSem(m_ulMuSem, MAX_TIMEOUT);

   switch (mode) {

      case XTM_LINK_MODE_ADSL      :
         m_ulAdslThreshold = ulThreshold ;
         break;
      case XTM_LINK_MODE_VDSL      :
         m_ulVdslThreshold = ulThreshold ;
         break;
      case XTM_LINK_MODE_VDSL_RTX  :
         m_ulVdslRtxThreshold = ulThreshold ;
         break;
      case XTM_LINK_MODE_GFAST     :
         m_ulGfastThreshold = ulThreshold ;
         break;
      default                      :
         break;
   }

   XtmOsReleaseSem(m_ulMuSem);
   return ;
}

UINT32 XTM_CONNECTION_TABLE::getThreshold( UINT32 mode )
{
   UINT32 ulThreshold = 0 ;

   if(m_ulMuSem == NULL)
      return ulThreshold ;

   XtmOsRequestSem(m_ulMuSem, MAX_TIMEOUT);

   switch (mode) {

      case XTM_LINK_MODE_ADSL      :
         ulThreshold = m_ulAdslThreshold ;
         break;
      case XTM_LINK_MODE_VDSL      :
         ulThreshold = m_ulVdslThreshold ;
         break;
      case XTM_LINK_MODE_VDSL_RTX  :
         ulThreshold = m_ulVdslRtxThreshold ;
         break;
      case XTM_LINK_MODE_GFAST     :
         ulThreshold = m_ulGfastThreshold ;
         break;
      default                      :
         break;
   }

   XtmOsReleaseSem(m_ulMuSem);
   return (ulThreshold) ;
}

/***************************************************************************
 * Function Name: Add
 * Description  : Adds a connection object to the connection table.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_CONNECTION_TABLE::Add( XTM_CONNECTION *pConn )
{
    BCMXTM_STATUS bxStatus = XTMSTS_NOT_FOUND;
    XTM_ADDR ConnAddr;
    UINT32 i, ulPm;
    UINT32 ulUnused = (UINT32) -1;

   //XtmOsPrintf("%s:Enter\n",__FUNCTION__);
    if(m_ulMuSem == NULL)
       return bxStatus;
    XtmOsRequestSem(m_ulMuSem, MAX_TIMEOUT);

    pConn->GetAddr( &ConnAddr );
    ulPm = ConnAddr.u.Conn.ulPortMask ;

    /* Validate the portmask. */
    if( ulPm < PORT_PHY0_FAST || ulPm > (PORT_PHY0_FAST |
        PORT_PHY0_INTERLEAVED | PORT_PHY1_FAST | PORT_PHY1_INTERLEAVED) )
    {
        XtmOsPrintf ("xtmcfg: ulPm = %x \n", ulPm) ;
        bxStatus = XTMSTS_PARAMETER_ERROR;
    }
    else
    {
        /* Check if the connection is already in the table. */
        for( i = 0; i < MAX_CONN_TABLE_ENTRIES; i++ )
        {
            if( m_pConnTable[i] )
            {
                if( pConn == m_pConnTable[i] )
                {
                    bxStatus = XTMSTS_SUCCESS;
                    break;
                }
                else
                {
                    XTM_ADDR TblAddr;
                    m_pConnTable[i]->GetAddr( &TblAddr );
                    UINT32 ulPortMatch, ulAddrEqual;

                    /* Mask the port bits.  If the ATM/PTM addresses then
                     * match, this is an invalid entry because it conflicts
                     * with an existing ATM/PTM address.  It has the same
                     * VPI/VCI or PTM flow and at least one of the ports match.
                     */
                      if( (TblAddr.ulTrafficType & TRAFFIC_TYPE_ATM_MASK) == TRAFFIC_TYPE_ATM )
                    {
                        ulPortMatch = (TblAddr.u.Vcc.ulPortMask &
                            ConnAddr.u.Vcc.ulPortMask);
                        ulAddrEqual =
                            (TblAddr.u.Vcc.usVpi == ConnAddr.u.Vcc.usVpi) &&
                            (TblAddr.u.Vcc.usVci == ConnAddr.u.Vcc.usVci);
                    }
                    else
                    {
                        ulPortMatch = (TblAddr.u.Flow.ulPortMask &
                            ConnAddr.u.Flow.ulPortMask);
                        ulAddrEqual =
                            (TblAddr.u.Flow.ulPtmPriority ==
                             ConnAddr.u.Flow.ulPtmPriority);
                    }
                    if( ulPortMatch && ulAddrEqual )
                    {
                        /* Address conflict. */
                        bxStatus = XTMSTS_PARAMETER_ERROR;
                        break;
                    }
                }
            }
            else
                if( ulUnused == (UINT32) -1 )
                    ulUnused = i;
        }

        /* Add the connection to the table if it is not already in the table and
         * there is space.
         */
        if( bxStatus == XTMSTS_NOT_FOUND && ulUnused != (UINT32) -1 )
        {
            m_pConnTable[ulUnused] = pConn;
            bxStatus = XTMSTS_SUCCESS;
        }
    }

    XtmOsReleaseSem(m_ulMuSem);

    //XtmOsPrintf ("xtmcfg: bxStatus = %x \n", bxStatus) ;
    return( bxStatus );
} /* Add */


/***************************************************************************
 * Function Name: Remove
 * Description  : Removes a connection object from the connection table.
 * Returns      : None.
 ***************************************************************************/
void XTM_CONNECTION_TABLE::Remove( XTM_CONNECTION *pConn )
{
    UINT32 i;

    //XtmOsPrintf("%s:Enter\n",__FUNCTION__);
    if(m_ulMuSem == NULL)
       return;
    XtmOsRequestSem(m_ulMuSem, MAX_TIMEOUT);

    for( i = 0; i < MAX_CONN_TABLE_ENTRIES; i++ )
    {
        if( pConn == m_pConnTable[i] )
        {
            m_pConnTable[i] = NULL;
            break;
        }
    }

    XtmOsReleaseSem(m_ulMuSem);
} /* Remove */


/***************************************************************************
 * Function Name: Get
 * Description  : Returns a connection object from the connection table.
 * Returns      : pointer to an XTM_CONNECTION object or NULL
 ***************************************************************************/
XTM_CONNECTION *XTM_CONNECTION_TABLE::Get( PXTM_ADDR pAddr )
{
    XTM_CONNECTION *pConn = NULL;
    XTM_ADDR Addr;
    UINT32 i;

   //XtmOsPrintf("%s:Enter\n",__FUNCTION__);
    if(m_ulMuSem == NULL)
       return NULL;
    XtmOsRequestSem(m_ulMuSem, MAX_TIMEOUT);

    for( i = 0; i < MAX_CONN_TABLE_ENTRIES; i++ )
    {
        if( m_pConnTable[i] )
        {
            m_pConnTable[i]->GetAddr( &Addr );
            if( XTM_ADDR_CMP(pAddr, &Addr) )
            {
                /* Connection found. */
                pConn = m_pConnTable[i];
                break;
            }
        }
    }

    XtmOsReleaseSem(m_ulMuSem);

    return( pConn );
} /* Get */


/***************************************************************************
 * Function Name: GetForPortId
 * Description  : Returns a connection object from the connection table.
 *                The ulPortMask in the supplied parameter can contain
 *                a subset of ports.
 * Returns      : pointer to an XTM_CONNECTION object or NULL
 ***************************************************************************/
XTM_CONNECTION *XTM_CONNECTION_TABLE::GetForPortId( PXTM_ADDR pAddr )
{
    XTM_CONNECTION *pConn = NULL;
    XTM_ADDR Addr;
    UINT32 i;

    //XtmOsPrintf("%s:Enter\n",__FUNCTION__);
    if(m_ulMuSem == NULL)
       return NULL;
    XtmOsRequestSem(m_ulMuSem, MAX_TIMEOUT);

    for( i = 0; i < MAX_CONN_TABLE_ENTRIES; i++ )
    {
        if( m_pConnTable[i] )
        {
            m_pConnTable[i]->GetAddr( &Addr );
            Addr.u.Conn.ulPortMask &= pAddr->u.Conn.ulPortMask;
            if( XTM_ADDR_CMP(pAddr, &Addr) )
            {
                /* Connection found. */
                pConn = m_pConnTable[i];
                break;
            }
        }
    }

    XtmOsReleaseSem(m_ulMuSem);

    return( pConn );
} /* GetForPortId */


/***************************************************************************
 * Function Name: Enum
 * Description  : Returns a connection object from the connection table for
 *                the specified index.
 * Returns      : pointer to an XTM_CONNECTION object or NULL
 ***************************************************************************/
XTM_CONNECTION *XTM_CONNECTION_TABLE::Enum( UINT32 *pulIdx )
{
    XTM_CONNECTION *pConn = NULL;
    UINT32 i;
    //XtmOsPrintf("%s:m_ulMuSem[%pk]\n",__FUNCTION__,m_ulMuSem);
    if(m_ulMuSem == NULL)
       return NULL;
    XtmOsRequestSem(m_ulMuSem, MAX_TIMEOUT);

    for( i = *pulIdx; i < MAX_CONN_TABLE_ENTRIES; i++ )
    {
        if( m_pConnTable[i] )
        {
            pConn = m_pConnTable[i];
            break;
        }
    }

    *pulIdx = i + 1;

    XtmOsReleaseSem(m_ulMuSem);

    return( pConn );
} /* Enum */

