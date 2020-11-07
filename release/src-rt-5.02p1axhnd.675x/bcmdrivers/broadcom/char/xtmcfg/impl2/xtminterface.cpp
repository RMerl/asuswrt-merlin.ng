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
 * File Name  : xtminterface.cpp (impl2)
 *
 * Description: This file contains the implementation for the XTM interface
 *              class.  This class handles the processing that is associated
 *              with an ATM/PTM port.
 ***************************************************************************/

#include "bcmtypes.h"
#include "xtmcfgimpl.h"
#include "bcmadsl.h"

typedef int (*FN_ADSL_GET_OBJECT_VALUE) (unsigned char lineId, char *objId,
		                                   int objIdLen, char *dataBuf, long *dataBufLen);

static adslMibInfo MibInfo ;

extern "C" {

extern FN_ADSL_GET_OBJECT_VALUE g_pfnAdslGetObjValue;
}

/***************************************************************************
 * Function Name: XTM_INTERFACE
 * Description  : Constructor for the XTM interface class.
 * Returns      : None.
 ***************************************************************************/
XTM_INTERFACE::XTM_INTERFACE( void )
{
    //XtmOsPrintf("%s:Enter\n",__FUNCTION__); 
    m_ulPhysPort = 0;
    m_ulPhysBondingPort = MAX_INTERFACES ;
    m_ulDataStatus = DATA_STATUS_DISABLED ;
    m_ulErrTickCount = 0 ;
    m_ulIfInPacketsPtm = 0;
    m_ulAutoSenseATM = BC_ATM_AUTO_SENSE_ENABLE ;
    memset(&m_Cfg, 0x00, sizeof(m_Cfg));
    memset( &m_LinkInfo, 0x00, sizeof(m_LinkInfo));
    memset( &m_LinkDelay, 0x00, sizeof(m_LinkDelay));
    memset( &m_Stats, 0x00, sizeof(m_Stats));
    m_LinkInfo.ulLinkState = LINK_DOWN;
    m_Cfg.ulIfAdminStatus = ADMSTS_DOWN;

    // Log that we not initialized at this point
    m_bInitialized = false;

} /* XTM_INTERFACE */

void XTM_INTERFACE::PreInit ( void )
{
    //XtmOsPrintf("%s:Enter\n",__FUNCTION__); 
    m_ulPhysPort = 0;
    m_ulPhysBondingPort = MAX_INTERFACES ;
    m_ulDataStatus = DATA_STATUS_DISABLED ;
    m_ulErrTickCount = 0 ;
    m_ulIfInPacketsPtm = 0;
    m_ulAutoSenseATM = BC_ATM_AUTO_SENSE_ENABLE ;
    memset(&m_Cfg, 0x00, sizeof(m_Cfg));
    memset( &m_LinkInfo, 0x00, sizeof(m_LinkInfo));
    memset( &m_LinkDelay, 0x00, sizeof(m_LinkDelay));
    memset( &m_Stats, 0x00, sizeof(m_Stats));
    m_LinkInfo.ulLinkState = LINK_DOWN;
    m_Cfg.ulIfAdminStatus = ADMSTS_DOWN;
    m_ulXTMLinkMode      = XTM_LINK_MODE_UNKNOWN;

#if defined(XTM_PORT_SHAPING)
    m_ulEnableShaping    = 0;
    m_ulShapeRate        = 0;
    m_usMbs              = 0;
    m_ulPortShapingRatio = 0;
#endif

    // Log that we not initialized at this point
    m_bInitialized = false;

} /* XTM_INTERFACE */

/***************************************************************************
 * Function Name: ~XTM_INTERFACE
 * Description  : Destructor for the XTM interface class.
 * Returns      : None.
 ***************************************************************************/
XTM_INTERFACE::~XTM_INTERFACE( void )
{
    //XtmOsPrintf("%s:Enter\n",__FUNCTION__); 
    Uninitialize();
} /* ~XTM_INTERFACE */


/***************************************************************************
 * Function Name: Initialize
 * Description  : Initializes the object.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_INTERFACE::Initialize( UINT32 ulPort, UINT32 ulInternalPort, UINT32 ulBondingPort,
                                         UINT32 autoSenseATM, FN_XTMRT_REQ pfnXtmrtReq)
{
    BCMXTM_STATUS bxStatus = XTMSTS_SUCCESS;
    //XtmOsPrintf("%s:Enter\n",__FUNCTION__); 
    // Initialize as needed
    if(m_bInitialized != true)
    {
        m_ulPhysPort = ulPort;
        m_ulInternalPort = ulInternalPort;
        m_ulPhysBondingPort = ulBondingPort;
        m_ulIfInPacketsPtm = 0;
        m_ulAutoSenseATM = autoSenseATM ;
        m_pfnXtmrtReq = pfnXtmrtReq;

        XP_REGS->ulTxSarCfg |= (1 << (m_ulPhysPort + TXSARA_CRC10_EN_SHIFT));
#if !defined(CONFIG_BCM963158)
        XP_REGS->ulRxSarCfg |= (1 << (m_ulPhysPort + RXSARA_CRC10_EN_SHIFT));
#else
        /* in 63158A0 case, all the incoming cells are marked in error when this
        ** is set. So, a temporary workaround is to keep it disabled.
	** Revert this for 63158B0.
        **/
#if (CONFIG_BRCM_CHIP_REV!=0x63158A0)
        XP_REGS->ulRxSarCfg |= (1 << (m_ulPhysPort + RXSARA_CRC10_EN_SHIFT));
#endif
#endif

        XP_REGS->ulRxAtmCfg[m_ulPhysPort] |= RX_DOE_MASK ;

        SetRxUtopiaLinkInfo (LINK_UP) ;

        XP_REGS->ulRxPafWriteChanFlush |= (0x11 << m_ulPhysPort) ;

        m_Cfg.ulIfLastChange = XtmOsTickGet() / 10;
    }

    // Log the sucessful initialization
    m_bInitialized = true;

    return( bxStatus );
} /* Initialize */

/***************************************************************************
 * Function Name: ReInitialize
 * Description  : ReInitializes the object in terms of updating the bonding
 * port member.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_INTERFACE::ReInitialize( UINT32 ulBondingPort )
{
    BCMXTM_STATUS bxStatus = XTMSTS_SUCCESS;
    //XtmOsPrintf("%s:Enter\n",__FUNCTION__); 
    
    m_ulPhysBondingPort = ulBondingPort;
    return( bxStatus );
} /* ReInitialize */

/***************************************************************************
 * Function Name: GetStats
 * Description  : Returns statistics for this interface.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_INTERFACE::GetStats( PXTM_INTERFACE_STATS pStats,
    UINT32 ulReset )
{
    //XtmOsPrintf("%s:Enter\n",__FUNCTION__); 
    UINT32 ulCurrMibCtrl = XP_REGS->ulMibCtrl;

    XP_REGS->ulMibCtrl = (ulReset) ? 1 : 0;

    pStats->ulIfInOctets  += XP_REGS->ulRxPortPktOctCnt[m_ulPhysPort];
    pStats->ulIfOutOctets += XP_REGS->ulTxPortPktOctCnt[m_ulPhysPort];
    
    if( m_LinkInfo.ulLinkTrafficType == TRAFFIC_TYPE_ATM ||
        m_LinkInfo.ulLinkTrafficType == TRAFFIC_TYPE_ATM_BONDED )
    {
        pStats->ulIfInPackets += XP_REGS->ulRxPortPktCnt[m_ulPhysPort];
    }
    else
    {
        pStats->ulIfInPackets = m_ulIfInPacketsPtm;
        if (m_LinkInfo.ulLinkTrafficType == TRAFFIC_TYPE_PTM_BONDED)
        {
            pStats->ulIfInPackets += (XP_REGS->ulRxPafFragCount[m_ulPhysPort] +
                                      XP_REGS->ulRxPafFragCount[m_ulPhysPort+MAX_PHY_PORTS]);

            /* ulRxPafFragCount[] includes dropped frag. Therefore, we have to
            * deduct the dropped frag count.
            */
            pStats->ulIfInPackets -= (XP_REGS->ulRxPafDroppedFragCount[m_ulPhysPort] +
                                      XP_REGS->ulRxPafDroppedFragCount[m_ulPhysPort+MAX_PHY_PORTS]);
                                  
            /* For PTM bonding, ulRxBondDroppedFragCount and ulRxPafDroppedPktCount
            * need to be counted.
            */
            pStats->ulIfInPackets -= (XP_REGS->ulRxPafDroppedPktCount[m_ulPhysPort] +
                                      XP_REGS->ulRxPafDroppedPktCount[m_ulPhysPort+MAX_PHY_PORTS]);
            pStats->ulIfInPackets -= (XP_REGS->ulRxBondDroppedFragCount[m_ulPhysPort] +
                                      XP_REGS->ulRxBondDroppedFragCount[m_ulPhysPort+MAX_PHY_PORTS]);
        }
        else
        {
            pStats->ulIfInPackets += (XP_REGS->ulRxPafPktCount[m_ulPhysPort] +
                                      XP_REGS->ulRxPafPktCount[m_ulPhysPort+MAX_PHY_PORTS]);

            /* ulRxPafPktCount[] includes dropped packets. Therefore, we have to
            * deduct the dropped packet count.
            */
            pStats->ulIfInPackets -= (XP_REGS->ulRxPafDroppedPktCount[m_ulPhysPort] +
                                      XP_REGS->ulRxPafDroppedPktCount[m_ulPhysPort+MAX_PHY_PORTS]);
                                  
        }
        m_ulIfInPacketsPtm = (ulReset) ? 0 : pStats->ulIfInPackets;
    }
    
    pStats->ulIfOutPackets += XP_REGS->ulTxPortPktCnt[m_ulPhysPort];
    pStats->ulIfInOamRmCells += (XP_REGS->ulTxRxPortOamCellCnt[m_ulPhysPort] &
        OAM_RX_CELL_COUNT_MASK) >> OAM_RX_CELL_COUNT_SHIFT;
    pStats->ulIfOutOamRmCells += (XP_REGS->ulTxRxPortOamCellCnt[m_ulPhysPort] &
        OAM_TX_CELL_COUNT_MASK) >> OAM_TX_CELL_COUNT_SHIFT;
    pStats->ulIfInAsmCells += (XP_REGS->ulTxRxPortAsmCellCnt[m_ulPhysPort] &
        ASM_RX_CELL_COUNT_MASK) >> ASM_RX_CELL_COUNT_SHIFT;
    pStats->ulIfOutAsmCells += (XP_REGS->ulTxRxPortAsmCellCnt[m_ulPhysPort] &
        ASM_TX_CELL_COUNT_MASK) >> ASM_TX_CELL_COUNT_SHIFT;
    pStats->ulIfInCellErrors +=
        (XP_REGS->ulRxPortErrorPktCellCnt[m_ulPhysPort] &
        ERROR_RX_CELL_COUNT_MASK) >> ERROR_RX_CELL_COUNT_SHIFT;
    pStats->ulIfInPacketErrors +=
        (XP_REGS->ulRxPortErrorPktCellCnt[m_ulPhysPort] &
        ERROR_RX_PKT_COUNT_MASK) >> ERROR_RX_PKT_COUNT_SHIFT;

    XP_REGS->ulMibCtrl = ulCurrMibCtrl;

    return( XTMSTS_SUCCESS );
} /* GetStats */


/***************************************************************************
 * Function Name: SetRxUtopiaLinkInfo
 * Description  : Enabled/Disabled Utopia Links on rx side.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_INTERFACE::SetRxUtopiaLinkInfo ( UINT32 ulLinkState )
{
   //XtmOsPrintf("%s:Enter\n",__FUNCTION__); 
   UINT32 ulRxPortEnShift = 0, ulRxSlaveIntPortEnShift = 32 ;

   if( m_ulInternalPort ) {
      ulRxPortEnShift = RXUTO_INT_PORT_EN_SHIFT;
   }
#if defined(CONFIG_BCM963268)
   else {
      if ((GPIO->GPIOBaseMode & GPIO_BASE_UTOPIA_OVERRIDE) != 0) {

         if ((MISC->miscStrapBus & MISC_STRAP_BUS_UTOPIA_MASTER_N_SLAVE) != 0)
            ulRxPortEnShift = RXUTO_EXT_IN_PORT_EN_SHIFT;
         else {
            ulRxPortEnShift = RXUTO_EXT_OUT_PORT_EN_SHIFT;
            ulRxSlaveIntPortEnShift = RXUTO_INT_PORT_EN_SHIFT ;
         }
      }
   }
#endif

   if( ulLinkState == LINK_UP ) {
      XP_REGS->ulRxUtopiaCfg |= 1 << (m_ulPhysPort + ulRxPortEnShift);
      if (ulRxSlaveIntPortEnShift != 32)
         XP_REGS->ulRxUtopiaCfg |= 1 << (m_ulPhysPort + ulRxSlaveIntPortEnShift);
   }
   else {
      XP_REGS->ulRxUtopiaCfg &= ~(1 << (m_ulPhysPort + ulRxPortEnShift));
      if (ulRxSlaveIntPortEnShift != 32)
         XP_REGS->ulRxUtopiaCfg &= ~(1 << (m_ulPhysPort + ulRxSlaveIntPortEnShift));
   }

   return( XTMSTS_SUCCESS );
} /* SetRxUtopiaLinkInfo */


#if defined(XTM_PORT_SHAPING)

/***************************************************************************
 * Function Name: ConfigureMaxUTPortShaping
 * Description  : Enables Port Shaping on Utopia
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_INTERFACE::ConfigureMaxUTPortShaping ( UINT32 ulTrafficType)
{
   UINT32 i ;
   volatile UINT32 *Addr ;

   /* Apply for all the VDSL modes due to SAR clk/UT clk differences and Zero
   ** padding issue.
   ** Ref HW Jira - HW63158-290
   **/
   if ((m_ulXTMLinkMode != XTM_LINK_MODE_GFAST) &&
         ((ulTrafficType == TRAFFIC_TYPE_PTM) || (ulTrafficType == TRAFFIC_TYPE_PTM_RAW) ||
          (ulTrafficType == TRAFFIC_TYPE_PTM_BONDED))) {

      XtmOsPrintf ("bcmxtmcfg: Configure Max UT port shaping %d\n", m_ulPhysPort) ;

      XP_REGS->ulTxLineRateTimer = XTMCFG_LINE_RATE_TIMER ;


      XP_REGS->ulSstSitValue     = XTMCFG_SST_SIT_VALUE ;


      Addr = (UINT32 *) &XP_REGS->ulSsteUtoPortPcr[m_ulPhysPort] [0] ;

      i = *Addr ;
      i &= SSTE_UTOPCRMW_PCR_PLVLWT_MASK ;
      i |= (XTMCFG_UTO_PCR_INCR_SHIFT << SSTE_UTOPCRMW_PCR_INCR_SHIFT) ;
      *Addr = i;

      i = XP_REGS->ulSsteUtoPortPcr[m_ulPhysPort] [1] ;
      i &= SSTE_UTOPCRMW_PCR_CURACC_MASK ;
      i |= (XTMCFG_UTO_PCR_ACCLMT << SSTE_UTOPCRMW_PCR_ACCLMT_SHIFT) ;
      XP_REGS->ulSsteUtoPortPcr[m_ulPhysPort][1] = i;

      i = XP_REGS->ulSsteUtoPortPcr[m_ulPhysPort] [2] ;  
      i &= SSTE_UTOPCRMW_PCR_CURSIT_MASK ;
      i |= (XTMCFG_UTO_PCR_SITLMT << SSTE_UTOPCRMW_PCR_SITLMT_SHIFT) ;
      XP_REGS->ulSsteUtoPortPcr[m_ulPhysPort] [2] = i;

      i = XP_REGS->ulSsteUtoGtsCfg[m_ulPhysPort] ;  
      i &= ~SSTE_UTOGTSCFG_MASK ;
      i |= (0x1 << SSTE_UTOGTSCFG_SHIFT) ;
      XP_REGS->ulSsteUtoGtsCfg[m_ulPhysPort] = i;

   } /* if (!Gfast && VDSL PTM/PTM RAW) */

   else {

      /* ATM, ATM bonded, GFAST single/bonded modes dont need the PortShaping */
      /* Disable Port shaping */

      XtmOsPrintf ("bcmxtmcfg: UnConfigure Max UT port shaping %d\n", m_ulPhysPort) ;

      i = XP_REGS->ulSsteUtoGtsCfg[m_ulPhysPort] ;  
      i &= ~SSTE_UTOGTSCFG_MASK ;
      i &= ~(0x1 << SSTE_UTOGTSCFG_SHIFT) ;
      XP_REGS->ulSsteUtoGtsCfg[m_ulPhysPort] = i;
   }

   return( XTMSTS_SUCCESS );

} /* ConfigureMaxUTPortShaping */

#endif /* 158 */


void XTM_INTERFACE::UpdateLinkDelay()
{
	int ret ;
	long int size = sizeof (adslMibInfo) ;
    //XtmOsPrintf("%s:Enter\n",__FUNCTION__); 

	MibInfo.maxBondingDelay = 0 ;
	
	if (g_pfnAdslGetObjValue != NULL) {
		size = sizeof (MibInfo) ;
		ret  = g_pfnAdslGetObjValue (m_ulPhysPort, NULL, 0, (char *) &MibInfo, &size) ;
		if (ret != 0)
			XtmOsPrintf ("Error g_pfnAdslGetObjValue port - %d return value - %d \n", m_ulPhysPort, ret) ;
	}

	m_LinkDelay.ulLinkDsBondingDelay = MibInfo.maxBondingDelay ;
}

/***************************************************************************
 * Function Name: SetLinkInfo
 * Description  : Call when there is a change in link status.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_INTERFACE::SetLinkInfo( PXTM_INTERFACE_LINK_INFO pLinkInfo, UINT8 rxConfig,
		                                    UINT8 linkInfoConfig, UINT8 txConfig, UINT32 ulXTMLinkMode )
{
	int ret ;
   UINT32 ulTxPortEnShift=0, ulRxTeqPortShift=0, ulRxTeqPortMask=0, ulTxSlaveIntPortEnShift = 32 ;
   UINT32 ulTxRawPortEnShift ;
    //XtmOsPrintf("%s:Enter\n",__FUNCTION__); 

   if( m_ulInternalPort )
   {
      ulTxPortEnShift  = TXUTO_INT_PORT_EN_SHIFT;
      ulTxRawPortEnShift = TXUTO_INT_RAW_PID_EN_SHIFT;
      ulRxTeqPortShift = RXUTO_INT_TEQ_PORT_SHIFT;
      ulRxTeqPortMask  = RXUTO_INT_TEQ_PORT_MASK;
   }
#if defined(CONFIG_BCM963268)
   else {
      /* Utopia override option will be set only for "all external"/ "internal
       * external" port modes.
       */
      if ((GPIO->GPIOBaseMode & GPIO_BASE_UTOPIA_OVERRIDE) != 0) {

         if (MISC->miscStrapBus & MISC_STRAP_BUS_UTOPIA_MASTER_N_SLAVE)
            ulTxPortEnShift = TXUTO_EXT_PORT_EN_SHIFT;
         else {
            ulTxPortEnShift = TXUTO_SLV_PORT_EN_SHIFT;
            ulTxSlaveIntPortEnShift = TXUTO_INT_PORT_EN_SHIFT ;
         }

         ulRxTeqPortShift = RXUTO_EXT_TEQ_PORT_SHIFT;
         ulRxTeqPortMask  = RXUTO_EXT_TEQ_PORT_MASK;
      }
   }
#endif

   m_ulXTMLinkMode = ulXTMLinkMode ;

   if( pLinkInfo->ulLinkState == LINK_UP )
   {
      if( m_Cfg.ulIfAdminStatus == ADMSTS_UP )
      {
         if (linkInfoConfig == XTM_CONFIGURE) {
            memcpy( &m_LinkInfo, pLinkInfo, sizeof(m_LinkInfo));
            m_Cfg.usIfTrafficType = pLinkInfo->ulLinkTrafficType;
         }

         if (txConfig == XTM_CONFIGURE) {
            XP_REGS->ulTxSchedCfg |=
               1 << (m_ulPhysPort + TXSCH_PORT_EN_SHIFT);
            XP_REGS->ulTxUtopiaCfg |= 
               1 << (m_ulPhysPort + ulTxPortEnShift);
            if (ulTxSlaveIntPortEnShift != 32)
               XP_REGS->ulTxUtopiaCfg |= 1 << (m_ulPhysPort + ulTxSlaveIntPortEnShift);

            if (m_Cfg.usIfTrafficType == TRAFFIC_TYPE_PTM_RAW)
               XP_REGS->ulTxUtopiaCfg |=
                  1 << (m_ulPhysPort + ulTxRawPortEnShift);
         }

         if (rxConfig == XTM_CONFIGURE) {
            XP_REGS->ulRxAtmCfg[m_ulPhysPort] |= RX_PORT_EN;

            if (m_Cfg.usIfTrafficType == TRAFFIC_TYPE_ATM_BONDED)
               XP_REGS->ulRxAtmCfg[m_ulPhysPort] |= (RXA_BONDING_VP_MASK |
                     (m_ulPhysPort << RX_PORT_MASK_SHIFT) |
                     RXA_HEC_CRC_IGNORE |
                     RXA_GFC_ERROR_IGNORE) ;

            //XtmOsPrintf ("traffictype = %d \n", m_Cfg.usIfTrafficType) ;

            //XtmOsPrintf ("UtopiaCfg = %x \n", XP_REGS->ulRxUtopiaCfg) ;
            if( m_Cfg.usIfTrafficType == TRAFFIC_TYPE_PTM_RAW )
            {
               //m_Cfg.usIfTrafficType = TRAFFIC_TYPE_PTM;
               //XtmOsPrintf ("UtopiaTeqMask = %x \n", ulRxTeqPortMask) ;
               XP_REGS->ulRxUtopiaCfg &= ~ulRxTeqPortMask;
               XP_REGS->ulRxUtopiaCfg |= (1 << (m_ulPhysPort + ulRxTeqPortShift));
            }
            //XtmOsPrintf ("UtopiaCfg = %x \n", XP_REGS->ulRxUtopiaCfg) ;
         } /* rxConfig */
      }

      /* Read a current snapshot of the IF Error Counters */

      if (m_ulPhysBondingPort != MAX_INTERFACES) {
         long int size = sizeof (adslMibInfo) ;

         if (g_pfnAdslGetObjValue != NULL) {
				size = sizeof (MibInfo) ;
	         ret  = g_pfnAdslGetObjValue (m_ulPhysPort, NULL, 0, (char *) &MibInfo, &size) ;
				if (ret != 0)
					XtmOsPrintf ("g_pfnAdslGetObjValue port - %d return value - %d \n", m_ulPhysPort, ret) ;
			}

         m_LinkDelay.ulLinkDsBondingDelay = MibInfo.maxBondingDelay ;
         m_LinkDelay.ulLinkUsDelay = MibInfo.xdslInfo.dirInfo[1].lpInfo[0].delay ;

         m_PrevIfMonitorInfo.rx_loss_of_sync     = MibInfo.adslPerfData.perfTotal.adslLOSS ;
         m_PrevIfMonitorInfo.rx_SES_count_change = MibInfo.adslPerfData.perfTotal.adslSES ;
         m_PrevIfMonitorInfo.tx_SES_count_change = MibInfo.adslTxPerfTotal.adslSES ;
         m_PrevIfMonitorInfo.tx_LOS_change       = MibInfo.adslTxPerfTotal.adslLOSS ;
         m_PrevIfMonitorInfo.rx_uncorrected      = MibInfo.xdslStat[0].rcvStat.cntRSUncor ; /* Latency 0 */

         m_ulErrTickCount = 0 ;
      }

      while (XTM_IS_WRCHAN_FLUSH_BUSY)
         XtmOsDelay (100) ;

      XP_REGS->ulRxPafWriteChanFlush &= ~(0xFFFFFF11 << m_ulPhysPort) ;

      m_ulDataStatus = DATA_STATUS_ENABLED ;
   }
   else /* LINK_DOWN */
   {
      if (linkInfoConfig == XTM_CONFIGURE) {
         memcpy( &m_LinkInfo, pLinkInfo, sizeof(m_LinkInfo));
         m_Cfg.usIfTrafficType        = TRAFFIC_TYPE_NOT_CONNECTED;
         m_LinkInfo.ulLinkTrafficType = TRAFFIC_TYPE_NOT_CONNECTED;
         m_LinkDelay.ulLinkDsBondingDelay = 0 ;
         m_LinkDelay.ulLinkUsDelay = 0 ;
      }

      if (txConfig == XTM_CONFIGURE) {
         XP_REGS->ulTxUtopiaCfg &= 
            ~(1 << (m_ulPhysPort + ulTxPortEnShift));
         if (ulTxSlaveIntPortEnShift != 32)
            XP_REGS->ulTxUtopiaCfg &= ~(1 << (m_ulPhysPort + ulTxSlaveIntPortEnShift));
         XP_REGS->ulTxSchedCfg &=
            ~(1 << (m_ulPhysPort + TXSCH_PORT_EN_SHIFT));
         XP_REGS->ulTxUtopiaCfg &=
            ~(1 << (m_ulPhysPort + ulTxRawPortEnShift));
      }
      else {
         /* Workaround for 68 atm bonding. Keep the scheduler enabled and the
          * utopia disabled (as the other bonding link may still be up.)
          */
         if( m_Cfg.usIfTrafficType == TRAFFIC_TYPE_ATM_BONDED ) {
            XP_REGS->ulTxUtopiaCfg &= ~(1 << (m_ulPhysPort + ulTxPortEnShift));
            if (ulTxSlaveIntPortEnShift != 32)
               XP_REGS->ulTxUtopiaCfg &= ~(1 << (m_ulPhysPort + ulTxSlaveIntPortEnShift));
         }
      }

      if (rxConfig == XTM_CONFIGURE) {
         XP_REGS->ulRxUtopiaCfg &= ~ulRxTeqPortMask;
         XP_REGS->ulRxUtopiaCfg |= (XP_MAX_PORTS << ulRxTeqPortShift);
         XP_REGS->ulRxAtmCfg[m_ulPhysPort] &= ~RX_PORT_EN;
      }

      while (XTM_IS_WRCHAN_FLUSH_BUSY)
         XtmOsDelay (100) ;

      XP_REGS->ulRxPafWriteChanFlush |= (0x11 << m_ulPhysPort) ;

      m_ulDataStatus = DATA_STATUS_DISABLED ;
      m_ulErrTickCount = 0 ;
   }

   m_Cfg.ulIfLastChange = XtmOsTickGet() / 10;

   return( XTMSTS_SUCCESS );
} /* SetLinkInfo */

/***************************************************************************
 * Function Name: UpdateLinkInfo
 * Description  : Call when there is a change in link traffic type.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_INTERFACE::UpdateLinkInfo( UINT32 ulTrafficType )
{
    //XtmOsPrintf("%s:Enter\n",__FUNCTION__); 
	m_LinkInfo.ulLinkTrafficType = ulTrafficType ;
   m_Cfg.usIfTrafficType = ulTrafficType;
   XtmOsPrintf ("bcmxtmcfg: Auto-Sensed XTM Link Information, port = %d, State = %s, Service Support = %s \n",
                m_ulPhysPort,
                (m_LinkInfo.ulLinkState == LINK_UP ? "UP" :
			         (m_LinkInfo.ulLinkState == LINK_DOWN ? "DOWN":
				       (m_LinkInfo.ulLinkState == LINK_START_TEQ_DATA ? "START_TEQ": "STOP_TEQ"))),
                (ulTrafficType == TRAFFIC_TYPE_ATM ? "ATM" :
                 (ulTrafficType == TRAFFIC_TYPE_PTM ? "PTM" :
                  (ulTrafficType == TRAFFIC_TYPE_PTM_BONDED ? "PTM_BONDED" :
                   (ulTrafficType == TRAFFIC_TYPE_ATM_BONDED ? "ATM_BONDED" : "RAW"))))) ;

   return( XTMSTS_SUCCESS );
} /* UpdateLinkInfo */

/***************************************************************************
 * Function Name: GetLinkErrorStatus (63268)
 * Description  : Call for detecting Link Errors. Only called for 63268 bonding
 * through run-time configurations.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_INTERFACE::GetLinkErrorStatus (UINT32 *pUsStatus, UINT32 *pDsStatus )
{
	int ret ;

   BCMXTM_STATUS nRet = XTMSTS_SUCCESS ;
    //XtmOsPrintf("%s:Enter\n",__FUNCTION__); 

   if (m_ulPhysBondingPort != MAX_INTERFACES) {

      if (m_ulErrTickCount == 0) {

         long int size = sizeof (adslMibInfo) ;
         IfMonitorErrorCounters CurrIfMonitorInfo ;

         m_ulUsStatus = m_ulDsStatus = XTMSTS_SUCCESS ;

         if (g_pfnAdslGetObjValue != NULL) {
				size = sizeof (MibInfo) ;
            ret = g_pfnAdslGetObjValue (m_ulPhysPort, NULL, 0, (char *) &MibInfo, &size) ;
				if (ret != 0)
					XtmOsPrintf ("g_pfnAdslGetObjValue port - %d return value - %d \n", m_ulPhysPort, ret) ;
			}

         CurrIfMonitorInfo.rx_loss_of_sync = MibInfo.adslPerfData.perfTotal.adslLOSS ;
         CurrIfMonitorInfo.rx_SES_count_change  = MibInfo.adslPerfData.perfTotal.adslSES ;
         CurrIfMonitorInfo.tx_SES_count_change = MibInfo.adslTxPerfTotal.adslSES ;
         CurrIfMonitorInfo.tx_LOS_change = MibInfo.adslTxPerfTotal.adslLOSS ;
	      CurrIfMonitorInfo.rx_uncorrected = MibInfo.xdslStat[0].rcvStat.cntRSUncor ; /* Latency 0 */

         if ((m_PrevIfMonitorInfo.rx_loss_of_sync != CurrIfMonitorInfo.rx_loss_of_sync) ||
             (m_PrevIfMonitorInfo.rx_SES_count_change != CurrIfMonitorInfo.rx_SES_count_change) ||
             (m_PrevIfMonitorInfo.rx_uncorrected != CurrIfMonitorInfo.rx_uncorrected)) {

				//XtmOsPrintf ("rxLos=%ul, rxSES=%ul, rxUnC=%ul \n",
						        //CurrIfMonitorInfo.rx_loss_of_sync, CurrIfMonitorInfo.rx_SES_count_change,
								  //CurrIfMonitorInfo.rx_uncorrected) ;
				nRet = XTMSTS_ERROR ;
				m_ulDsStatus = XTMSTS_ERROR ;
	 		}

         if ((m_PrevIfMonitorInfo.tx_SES_count_change != CurrIfMonitorInfo.tx_SES_count_change) ||
				 (m_PrevIfMonitorInfo.tx_LOS_change != CurrIfMonitorInfo.tx_LOS_change)) {

				//XtmOsPrintf ("txSES=%ul, txLos=%ul \n",
						        //CurrIfMonitorInfo.tx_SES_count_change, CurrIfMonitorInfo.tx_LOS_change) ;
				nRet = XTMSTS_ERROR ;
            m_ulUsStatus = XTMSTS_ERROR ;
			}

         if (nRet != XTMSTS_SUCCESS) {
            memcpy (&m_PrevIfMonitorInfo, &CurrIfMonitorInfo, sizeof (IfMonitorErrorCounters)) ;
            m_ulErrTickCount = XTM_BOND_DSL_ERR_DURATION_TIMEOUT_MS ;
         }
      }
      else {
         m_ulErrTickCount -= XTM_BOND_DSL_MONITOR_TIMEOUT_MS ;
         nRet = XTMSTS_ERROR ;
      }
   } /* if (m_ulPhysBondingPort != MAX_INTERFACES) */

   *pUsStatus = m_ulUsStatus ;
   *pDsStatus = m_ulDsStatus ;
   return (nRet) ;
}

/***************************************************************************
 * Function Name: SetPortDataStatus (63x68)
 * Description  : Call for setting the port data status.
 *                Also effects the port status in the SAR Rx registers.
 *                This is necessary to avoid SAR lockup from undesired traffic
 *                in DS direction during line micro-interruption.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
void XTM_INTERFACE::SetPortDataStatus ( UINT32 status, UINT32 flush )
{
    //XtmOsPrintf("%s:Enter\n",__FUNCTION__); 
   /* if Phy does stop & start the flow, then we do not need this here in XTM.
    * Currently in experimentation phase
    */
   if ((m_Cfg.usIfTrafficType == TRAFFIC_TYPE_ATM_BONDED) ||
		 (m_Cfg.usIfTrafficType == TRAFFIC_TYPE_PTM_BONDED)) {
      /* For PTM Bonding, XTM Network driver will take care in SW for Tx Path. This is for
       * HW control.
       * We only effect Rx direction, as there may be pending traffic for
       * concerned port in tx direction in the DMA queues, which should not be
       * blocked.
       */

      if (XTM_IS_WRCHAN_FLUSH_BUSY) {
         goto _End ;
      }

      if (status == DATA_STATUS_ENABLED)
         XP_REGS->ulRxPafWriteChanFlush &= ~(0xFFFFFF11 << m_ulPhysPort) ;
      else {
         if (flush == XTM_FLUSH)
              XP_REGS->ulRxPafWriteChanFlush |= (0x11 << m_ulPhysPort) ;
      }
#if 0
		XtmOsPrintf ("Paf Fl, P%d %x \n", m_ulPhysPort, XP_REGS->ulRxPafWriteChanFlush) ;
#endif
   } /* if (bonded traffic) */

   m_ulDataStatus = status;

_End :
   return ;
}


/***************************************************************************
 * Function Name: Uninitialize
 * Description  : Undo Initialize.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_INTERFACE::Uninitialize( void )
{
    //XtmOsPrintf("%s:Enter\n",__FUNCTION__); 
    // Set link status as needed
    if(m_bInitialized)
        SetRxUtopiaLinkInfo (LINK_DOWN) ;

    m_pfnXtmrtReq = NULL ;
    // Log the sucessful uninitialization
    m_bInitialized = false;

    return( XTMSTS_SUCCESS );
} /* Uninitialize */


/***************************************************************************
 * Function Name: GetCfg
 * Description  : Returns the interface configuration record.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_INTERFACE::GetCfg( PXTM_INTERFACE_CFG pCfg,
    XTM_CONNECTION_TABLE *pConnTbl )
{
    BCMXTM_STATUS bxStatus = XTMSTS_SUCCESS;
    XTM_CONNECTION *pConn;
    XTM_ADDR Addr;

    //XtmOsPrintf ("%s: Entry\n",__FUNCTION__) ;
    memcpy(pCfg, &m_Cfg, sizeof(XTM_INTERFACE_CFG));

    pCfg->ulIfOperStatus = (m_LinkInfo.ulLinkState == LINK_UP &&
        IsInterfaceUp()) ? OPRSTS_UP : OPRSTS_DOWN;

#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963158)
    if (m_ulPhysPort != XTM_RX_TEQ_PHY_PORT) {
       pCfg->usIfSupportedTrafficTypes = (SUPPORT_TRAFFIC_TYPE_ATM |
             SUPPORT_TRAFFIC_TYPE_PTM | SUPPORT_TRAFFIC_TYPE_PTM_RAW | SUPPORT_TRAFFIC_TYPE_PTM_BONDED
             | SUPPORT_TRAFFIC_TYPE_TXPAF_PTM_BONDED) ;
    }
    else {
       pCfg->usIfSupportedTrafficTypes = (SUPPORT_TRAFFIC_TYPE_ATM |
             SUPPORT_TRAFFIC_TYPE_PTM | SUPPORT_TRAFFIC_TYPE_PTM_RAW | SUPPORT_TRAFFIC_TYPE_PTM_BONDED
             | SUPPORT_TRAFFIC_TYPE_TXPAF_PTM_BONDED | SUPPORT_TRAFFIC_TYPE_TEQ) ;
    }
#else
    if (m_ulPhysPort != XTM_RX_TEQ_PHY_PORT) {
       pCfg->usIfSupportedTrafficTypes = (SUPPORT_TRAFFIC_TYPE_ATM |
             SUPPORT_TRAFFIC_TYPE_PTM | SUPPORT_TRAFFIC_TYPE_PTM_RAW | SUPPORT_TRAFFIC_TYPE_PTM_BONDED) ;
    }
    else {
       pCfg->usIfSupportedTrafficTypes = (SUPPORT_TRAFFIC_TYPE_ATM |
             SUPPORT_TRAFFIC_TYPE_PTM | SUPPORT_TRAFFIC_TYPE_PTM_RAW | SUPPORT_TRAFFIC_TYPE_PTM_BONDED
             | SUPPORT_TRAFFIC_TYPE_TEQ) ;
    }
#endif

    if (m_ulAutoSenseATM == BC_ATM_AUTO_SENSE_ENABLE)
        pCfg->usIfSupportedTrafficTypes |= SUPPORT_TRAFFIC_TYPE_ATM_BONDED ;

    /* Calculate the number of configured VCCs for this interface. */
    pCfg->ulAtmInterfaceConfVccs = 0;
    UINT32 i = 0;
    while( (pConn = pConnTbl->Enum( &i )) != NULL )
    {
        pConn->GetAddr( &Addr );
        if( ((Addr.ulTrafficType&TRAFFIC_TYPE_ATM_MASK) == TRAFFIC_TYPE_ATM) &&
            (Addr.u.Vcc.ulPortMask & PORT_TO_PORTID(m_ulPhysPort)) ==
             PORT_TO_PORTID(m_ulPhysPort) )
        {
            pCfg->ulAtmInterfaceConfVccs++;
        }
    }
    //XtmOsPrintf ("%s: Exit\n",__FUNCTION__) ;

    return( bxStatus );
} /* GetCfg */


/***************************************************************************
 * Function Name: SetCfg
 * Description  : Sets the configuration record.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_INTERFACE::SetCfg( PXTM_INTERFACE_CFG pCfg )
{
    MapXtmOsPrintf("%s:Enter\n",__FUNCTION__); 
    m_Cfg.ulIfAdminStatus = pCfg->ulIfAdminStatus;
#if defined(XTM_PORT_SHAPING)
    //Port Shaping
    if((pCfg->ulShapeRate != 0) && (pCfg->usMbs != 0) && (pCfg->ulPortShaping == PORT_Q_SHAPING_ON))
    {
       m_ulEnableShaping = PORT_Q_SHAPING_ON;
       m_ulShapeRate  = pCfg->ulShapeRate;
       m_usMbs        = pCfg->usMbs;
       MapXtmOsPrintf("%s:m_ulShapeRate[%u bits] m_usMbs[%u bits]\n",__FUNCTION__, m_ulShapeRate, m_usMbs);
       ConfigureTxPortShaping (m_LinkInfo.ulLinkTrafficType) ;
    }
    else if(pCfg->ulPortShaping == PORT_Q_SHAPING_OFF)
    {
       m_ulEnableShaping = PORT_Q_SHAPING_OFF;
       m_ulShapeRate  = 0;
       m_usMbs        = 0;
       DisableTxPortShaping();
    }
    else {
       XtmOsPrintf("%s:m_ulShapeRate[%u bits] m_usMbs[%u]\n",__FUNCTION__, m_ulShapeRate, m_usMbs);
       return XTMSTS_PARAMETER_ERROR;
    }

#endif
    return( XTMSTS_SUCCESS );
} /* SetCfg */


#if defined(XTM_PORT_SHAPING)

void XTM_INTERFACE::ResetHWTxPortShaping (void)
{
   volatile UINT32 *pulUtoPortScr = &XP_REGS->ulSsteUtoPortScr[m_ulPhysPort][0];
   volatile UINT32 *pulUtoPortPcr = &XP_REGS->ulSsteUtoPortPcr[m_ulPhysPort][0];
   volatile UINT32 *pulUtoPortGts = &XP_REGS->ulSsteUtoGtsCfg[m_ulPhysPort];
   pulUtoPortScr[0] = 0;
   pulUtoPortScr[1] = 0;
   pulUtoPortScr[2] = 0;
   pulUtoPortPcr[0] = 0;
   pulUtoPortPcr[1] = 0;
   pulUtoPortPcr[2] = 0;
   *pulUtoPortGts = 0;
   return;
}

#define  GFAST_SCHED_DIV_FACTOR        1333

UINT32 XTM_INTERFACE::ConfigureTxPortShaping( UINT32 ulTrafficType)
{
#if !defined(CONFIG_BCM_XRDP)
   UINT16 usMbs;
   UINT32 ulShapeRate;
   //UINT32 ulMbs;
   volatile UINT32 *pulUtoPortScr = &XP_REGS->ulSsteUtoPortScr[m_ulPhysPort][0];
   volatile UINT32 *pulUtoPortPcr = &XP_REGS->ulSsteUtoPortPcr[m_ulPhysPort][0];
   volatile UINT32 *pulUtoPortGts = &XP_REGS->ulSsteUtoGtsCfg[m_ulPhysPort];
   UINT32 ulScr = 0;
   UINT32 ulGts = 0;
   UINT32 ulScrSitLmt = 0;
#else
   XTMRT_PORT_SHAPER_INFO   portShaperInfo ;
#endif
   if((m_ulEnableShaping == 0)) {
      //No Shaping
      return PORT_Q_SHAPING_OFF;
   }

#if defined(CONFIG_BCM_XRDP)
   /* Send a message to XTMRT to control port shaping to enable mode across
   ** all the xtm channels corresponding to the port.
   ** First let us disable at SAR end to be sure.
   */
   ResetHWTxPortShaping() ;
   portShaperInfo.ulShapingRate      = m_ulShapeRate ;
   portShaperInfo.usShapingBurstSize = m_usMbs ;
   if (m_pfnXtmrtReq != NULL) {
      (*m_pfnXtmrtReq)(NULL, XTMRT_CMD_SET_TX_PORT_SHAPER_INFO, &portShaperInfo);
   }
   else {
      XtmOsPrintf("ConfigureTxPortShaping: Reference XTMRT Null \n") ;
   }
#else
   //Now apply the port shaping ratio.
   //First divide the value by 100 and then multiply by the factor so that we
   //may not overflow. 
   ulShapeRate  = (m_ulShapeRate  / XTM_PORT_SHAPING_RATIO_FULL) * m_ulPortShapingRatio;
   XtmOsPrintf("m_ulPortShapingRatio[%u] PhysPort[%u] ShapeRate[%u bits] Mbs[%u] \n",m_ulPortShapingRatio,
                                                                    m_ulPhysPort,
                                                                    ulShapeRate,
                                                                    m_usMbs);
   
   /* Calculate the number of SIT pulses per sec.
    * Note that ms_ulSitUt is the number of SAR clocks per SIT pulse.
   */
   UINT32 ulSitsPerSec   = SAR_CLOCK / ms_ulSitUt;
   UINT32 ulSitsPerSecLo = SAR_CLOCK / ms_ulSitLoUt;

   if((ulTrafficType & TRAFFIC_TYPE_ATM_MASK) == TRAFFIC_TYPE_ATM) {
      //ATM mode UBR with PCR
      usMbs = 8;  // 8 Cells
      ulScr = (ulShapeRate/(53*8));
      ulScrSitLmt = ulSitsPerSec / ulScr;
      XtmOsPrintf("Scr[%u] ScrSitLmt[%u] \n",ulScr,ulScrSitLmt);
      if( ulScrSitLmt == 0 )
      {
         XtmOsPrintf("%s: SCR_SITLMT=0x0 is invalid. No shaping.\n",__FUNCTION__);
         return PORT_Q_SHAPING_OFF;
      }
      else if( ulScrSitLmt > 0x7FFF )
      {
         /* Try using SIT_LO_CNT */
         ulScrSitLmt = ulSitsPerSecLo / ulScr;
         if( ulScrSitLmt == 0 || ulScrSitLmt > 0x7FFF )
         {
            XtmOsPrintf("%s: SCR_SITLMT=0x%x is invalid. No shaping.\n",__FUNCTION__,
                        ulScrSitLmt);
            return PORT_Q_SHAPING_OFF;
         }
         ulGts |= SST_GTS_SCR_EN | SST_GTS_SITLO_SCR_EN;
      }
      else
      {
         ulGts |= SST_GTS_SCR_EN;
      }
      *pulUtoPortGts = 0;
      /* Preserve SCR priority which is used for arbitration. */
      pulUtoPortScr[0] &= (SSTE_SCR_PLVLWT_MASK << SSTE_SCR_PLVLWT_SHIFT);
      pulUtoPortScr[0] |= (1 << SSTE_SCR_INCR_SHIFT);
      pulUtoPortScr[1]  = (8 << SSTE_SCR_ACCLMT_SHIFT);
      pulUtoPortScr[2]  = (ulScrSitLmt << SSTE_SCR_SITLMT_SHIFT);
      *pulUtoPortGts = ulGts;
   }
   else
   {
      //UBR with PBR Configuration
      
      if (m_ulXTMLinkMode == XTM_LINK_MODE_GFAST) {
         /* new Mbs = (minrate)/8/(10^9/750000). Explanation that this is suitable
          ** for asymmetric Gfast is in SWBCACPE-23733 */
         usMbs = (ulShapeRate/8)/GFAST_SCHED_DIV_FACTOR ;
      }
      else {
         usMbs = m_usMbs ;
      }

      if (ulShapeRate && usMbs)
      {
         /* Calculate SCR.
         * ulScr is the number of token bucket refills per second to support the
         * the shaping rate in bytes per second.  ulShapeRate is in bits per sec.
         * The number of tokens per refill is PTM_TOKEN_BUCKET_INCREMENT and is
         * set to SCR_INCR.
         */ 
         //UINT32 ulScr = ulShapeRate / (PTM_TOKEN_BUCKET_INCREMENT * 8);

         /* Calculate SITLMT for SCR.
         * SITLMT is the number of SIT pulses between token bucket refills.
         */
         //UINT32 ulScrSitLmt = ulSitsPerSec / ulScr;
         ulScrSitLmt = (ulSitsPerSec * PTM_TOKEN_BUCKET_INCREMENT / ulShapeRate) * 8;

         if (ulScrSitLmt == 0)
         {
            XtmOsPrintf("ConfigurePtmQueueShaper: SCR_SITLMT=0x0 is invalid. No shaping.\n");
            return PORT_Q_SHAPING_OFF;
         }
         else if (ulScrSitLmt > 0x7FFF)
         {
           
           /* Try using SIT_LO_CNT */
             ulScrSitLmt = (ulSitsPerSecLo * PTM_TOKEN_BUCKET_INCREMENT / ulShapeRate) * 8;
             if (ulScrSitLmt == 0 || ulScrSitLmt > 0x7FFF)
             {
                XtmOsPrintf("ConfigurePtmQueueShaper: SCR_SITLMT=0x%x is invalid. No shaping.\n",
                          ulScrSitLmt);
                return PORT_Q_SHAPING_OFF;
             }
             ulGts |= SST_GTS_SCR_EN | SST_GTS_SITLO_SCR_EN | SST_GTS_PKT_MODE_SHAPING_EN;
         }
         else
         {
            ulGts |= SST_GTS_SCR_EN | SST_GTS_PKT_MODE_SHAPING_EN;
         }
        
         /* Preserve SCR priority which is used for arbitration. */
         pulUtoPortScr[0] &= (SSTE_SCR_PLVLWT_MASK << SSTE_SCR_PLVLWT_SHIFT);
         pulUtoPortScr[0] |= (PTM_TOKEN_BUCKET_INCREMENT << SSTE_SCR_INCR_SHIFT);
         //pulUtoPortScr[0] |= (2286 << SSTE_SCR_INCR_SHIFT);
         pulUtoPortScr[1]  = (usMbs << SSTE_SCR_ACCLMT_SHIFT);
         pulUtoPortScr[2]  = (ulScrSitLmt << SSTE_SCR_SITLMT_SHIFT);
      }

      *pulUtoPortGts = ulGts;
   }
   MapXtmOsPrintf("pUtoPortScr[%08x] pUtoPortScr[%08x] \n",pulUtoPortScr[0],pulUtoPortScr[1]);
   MapXtmOsPrintf("pUtoPortScr[%08x] pUtoPortGts[%08x] \n",pulUtoPortScr[2],*pulUtoPortGts);
#endif
   return PORT_Q_SHAPING_ON;
}

void XTM_INTERFACE::DisableTxPortShaping()
{
#if defined(CONFIG_BCM_XRDP)
   XTMRT_PORT_SHAPER_INFO   portShaperInfo ;
#endif

   ResetHWTxPortShaping() ;

#if defined(CONFIG_BCM_XRDP)
   /* Send a message to XTMRT to control port shaping to disable mode across
   ** all the xtm channels corresponding to the port.
   ** The above lines are to make sure it is controlled at the SAR end.
   ** Values of shaping should be 0 set in previous routines.
   */
   portShaperInfo.ulShapingRate      = m_ulShapeRate ;
   portShaperInfo.usShapingBurstSize = m_usMbs ;
   if (m_pfnXtmrtReq != NULL) {
      (*m_pfnXtmrtReq)(NULL, XTMRT_CMD_SET_TX_PORT_SHAPER_INFO, &portShaperInfo);
   }
   else {
      XtmOsPrintf("ConfigureTxPortShaping: Reference XTMRT Null \n") ;
   }
#endif
}
#endif
