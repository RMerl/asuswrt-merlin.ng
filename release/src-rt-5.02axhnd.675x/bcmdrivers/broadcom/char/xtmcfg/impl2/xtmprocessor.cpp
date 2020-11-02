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
 * File Name  : xtmprocessor.cpp (impl2)
 *
 * Description: This file contains the implementation for the XTM processor
 *              class.  This is the topmost class that contains or points to
 *              all objects needed to handle the processing of the XTM processor.
 ***************************************************************************/

#include "board.h"
#include "AdslMibDef.h"
#include "xtmcfgimpl.h"
#include "bcmadsl.h"

typedef int (*FN_ADSL_GET_OBJECT_VALUE) (unsigned char lineId, char *objId,
                                         int objIdLen, char *dataBuf, long *dataBufLen);

static adslMibInfo PhyMibInfo ;
static void *pBondTmrParm = NULL;

extern "C" {

extern FN_ADSL_GET_OBJECT_VALUE g_pfnAdslGetObjValue;
}

extern "C" {
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963381) || defined(CONFIG_BCM963148)
extern int pmc_sar_power_up(void);
extern int pmc_sar_power_down(void);
extern int pmc_sar_power_reset(void);
extern int pmc_sar_soft_reset(void);
#elif defined(CONFIG_BCM963158)
#include "pmc_wan.h"
#define POWER_OFF 0
#define POWER_ON  1
#if (CONFIG_BRCM_CHIP_REV==0x63158A0)
extern int pll_sar_set_divider(void);
#endif
#endif
}

typedef int (*FN_ADSL_SET_OBJECT_VALUE) (unsigned char lineId, char *objId,
                                         int objIdLen, char *dataBuf, long *dataBufLen);

extern "C" {

extern FN_ADSL_SET_OBJECT_VALUE g_pfnAdslSetObjValue;
}

#ifndef ULONG_MAX
#define ULONG_MAX (~0x0)
#endif

/* Declare static members of this class and local static variables */
static char dslSysCtlCommand [3] = {(char)kOidAdslPrivateSysCtl, (char)kOidAdslPrivateSysMediaCfg, 0} ;
XTM_ASM_HANDLER *XTM_PROCESSOR::m_pAsmHandler;

/***************************************************************************
 * Function Name: XTM_PROCESSOR
 * Description  : Constructor for the XTM processor class.
 * Returns      : None.
 ***************************************************************************/
#if 0
XTM_PROCESSOR::XTM_PROCESSOR( void )
{
   Cxtor();
}
#endif

int XTM_PROCESSOR::Cxtor( void )
{
   UINT32 ulPort ;
   m_pTrafficDescrEntries = NULL;
   m_ulNumTrafficDescrEntries = 0;
   memset(&m_InitParms, 0x00, sizeof(m_InitParms));
#if defined(CONFIG_BCM963158)
   m_ulConnSem = (unsigned long)XtmOsCreateLock();
#else
   m_ulConnSem = (UINT32)XtmOsCreateLock();
#endif
   m_ulRxPafLockupMonitorEvtValid = 0 ;
   m_ulRxBondPrevErrStatus = 0 ;
   m_ulRxBondPrevExpSid01 = 0 ;
   m_ulRxBondPrevExpSid23 = 0 ;
#if defined(CONFIG_BCM_55153_DPU)
   m_ulTrafficMonitorPort = PHY_PORTID_0;
#else
   m_ulTrafficMonitorPort = MAX_INTERFACES ;
#endif
   m_ulPrevRelatedUpEvtValid = 0 ;


   for (ulPort=PHY_PORTID_0; ulPort<MAX_BONDED_LINES; ulPort++) {
      m_ulPendingEvtValid[ulPort] = 0 ;
   }

   m_ulTxPafEnabled = 0 ;
   m_ulPortShapingConfig = m_ulQShapingConfig = PORT_Q_SHAPING_OFF; 

   memset (m_ulRxVpiVciCamShadow, 0x00, sizeof(m_ulRxVpiVciCamShadow));

   /* Set up private members */
   m_AsmHandler.initializeVars ();
   m_pAsmHandler = &m_AsmHandler;
   
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963381) || defined(CONFIG_BCM963148)
   /* power up the SAR block */
   pmc_sar_power_up();
#elif defined(CONFIG_BCM963158)
   pmc_wan_interface_power_control(WAN_INTF_XDSL, POWER_ON);
#endif
   
   return 0;
} /* XTM_PROCESSOR */


/***************************************************************************
 * Function Name: ~XTM_PROCESSOR
 * Description  : Destructor for the XTM processor class.
 * Returns      : None.
 ***************************************************************************/
#if 0
XTM_PROCESSOR::~XTM_PROCESSOR( void )
{
   Dxtor();
}
#endif

int XTM_PROCESSOR::Dxtor( void )
{
   UINT32 ulPort ;

   m_ulTrafficMonitorPort = MAX_INTERFACES ;
   m_ulPrevRelatedUpEvtValid = 0 ;
   for (ulPort=PHY_PORTID_0; ulPort<MAX_BONDED_LINES; ulPort++) {
      m_ulPendingEvtValid[ulPort] = 0 ;
   }

   m_ulRxPafLockupMonitorEvtValid = 0 ;
   m_ulRxBondPrevErrStatus = 0 ;
   m_ulRxBondPrevExpSid01 = 0 ;
   m_ulRxBondPrevExpSid23 = 0 ;

   XtmOsDeleteLock(m_ulConnSem);

   Uninitialize(bcmxtmrt_request);
   
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963381) || defined(CONFIG_BCM963148)
   /* power down the SAR block */
   pmc_sar_power_down();
#elif defined(CONFIG_BCM963158)
   pmc_wan_interface_power_control(WAN_INTF_XDSL, POWER_OFF);
#endif   
   
   return 0;
} /* ~XTM_PROCESSOR */

/***************************************************************************
 * Function Name: Initialize
 * Description  : Initializes the object.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_PROCESSOR::Initialize( PXTM_INITIALIZATION_PARMS pInitParms,
    FN_XTMRT_REQ pfnXtmrtReq )

{
   BCMXTM_STATUS bxStatus = XTMSTS_SUCCESS;
   UINT32 i, j, k, l;
   UINT32 *p;
   UINT8 ucInternalPort[MAX_INTERFACES];
   //Following Initialization is required as XTM_PROCESSOR is created using
   //alloc function instead of new.
   if((pInitParms != NULL) && (pfnXtmrtReq != NULL))
   {
      UINT32 ulPort ;
      //XtmOsPrintf("%s:Enter\n",__FUNCTION__);
      m_pTrafficDescrEntries = NULL;
      m_ulNumTrafficDescrEntries = 0;
      memset(&m_InitParms, 0x00, sizeof(m_InitParms));
      XtmOsPrintf("SAR Base:%pK\n",XP_REGS);
#if defined(CONFIG_ARM64)
      m_ulConnSem = (unsigned long)XtmOsCreateLock();
#else
      m_ulConnSem = (UINT32)XtmOsCreateLock();
#endif
      m_ulRxPafLockupMonitorEvtValid = 0 ;
      m_ulRxBondPrevErrStatus = 0 ;
      m_ulRxBondPrevExpSid01 = 0 ;
      m_ulRxBondPrevExpSid23 = 0 ;
#if defined(CONFIG_BCM_55153_DPU)
      m_ulTrafficMonitorPort = PHY_PORTID_0;
#else
      m_ulTrafficMonitorPort = MAX_INTERFACES ;
#endif
      m_ulPrevRelatedUpEvtValid = 0 ;
      for (ulPort=PHY_PORTID_0; ulPort<MAX_BONDED_LINES; ulPort++) {
         m_ulPendingEvtValid[ulPort] = 0 ;
      }
      m_ulTxPafEnabled = 0 ;
      m_ulPortShapingConfig = m_ulQShapingConfig = PORT_Q_SHAPING_OFF; 
      memset(m_ulRxVpiVciCamShadow, 0x00, sizeof(m_ulRxVpiVciCamShadow));
      /* power up the SAR block */
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963381) || defined(CONFIG_BCM963148)
      pmc_sar_power_up();
#endif

#ifdef CONFIG_BCM963158
#if (CONFIG_BRCM_CHIP_REV==0x63158A0)
      pll_sar_set_divider();
#endif
#endif
   }

   if( pInitParms ) {
      for( i = PHY_PORTID_0; i < MAX_INTERFACES; i++ ) {
         m_Interfaces[i].PreInit () ;
      }
      memcpy(&m_InitParms, pInitParms, sizeof(m_InitParms));
   }

   if( pfnXtmrtReq )
      m_pfnXtmrtReq = pfnXtmrtReq;

   /* Set up private members */
   m_pAsmHandler = &m_AsmHandler;

   /* Read the chip Id & Chip Versions for future use */
   m_ulChipIdRev  = PERF->RevID & REV_ID_MASK ;

   i = TXSARP_ENET_FCS_INSERT | TXSARP_CRC16_EN | TXSARP_PREEMPT |
      TXSAR_USE_ALT_FSTAT | TXSARP_SOF_WHILE_TX ;
   j = XP_REGS->ulRxSarCfg;

#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178)
   i |= TXSARP_ENABLE_TX_SPKT ;
#endif

#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148)
   XtmOsPrintf ("bcmxtmcfg: ChipId Rev-%x \n", m_ulChipIdRev) ;

/* No selection necessary for 63158. BBH is the only option */
   i |= TXSARP_SEL_TXBBH ;
   j |= RXSARA_SEL_RXBBH ;
#endif

   //Reset the error counters
   m_ulRxBufferPrevOverrunCounter = 0;
   m_ulRxBufferPrevUnderrunCounter = 0;
   m_ulRxBufferPrevErrorPacketCounter = 0;
   m_ulRxBufferPrevErrorFragmentCounter = 0;

   /* Strip the Ethernet CRC from the PTM packet (The PTM CRC has already
    * been stripped.)
    */
   XP_REGS->ulRxPafConfig &= ~RP_DROP_PKT_END_BYTE_MASK;
   XP_REGS->ulRxPafConfig |= (4 /* ENET CRC */ << RP_DROP_PKT_END_BYTE_SHIFT);

   /* set the VCId for RxPaf to 0 */
   XP_REGS->ulRxPafVcid = 0 ;
   for (l=0; l<XP_MAX_RXPAF_WR_CHANNELS; l++)
      XP_REGS->ulRxPafVcid |= (RXPAF_INVALID_VCID<<(l*RXPAF_VCID_SHIFT)) ;

#if defined(CONFIG_BCM_JUMBO_FRAME)
   /* set the RxPaf Packet Sizes, Min-64, Max-2070 bytes. */
   XP_REGS->ulRxPafPktSize = (RP_MIN_FRAG_PKT_SIZE << RP_MIN_PACKET_SIZE_SHIFT) | 
                             (RP_MAX_JUMBO_FRAG_PKT_SIZE << RP_MAX_PACKET_SIZE_SHIFT) ;
                          
   /* Set the packet buffer length to 9*0x100 bytes */
   XP_REGS->ulRxPktBufThreshold &= ~PBTHRESH_MAX_COUNT_MASK;
   XP_REGS->ulRxPktBufThreshold |= 0x9;
#else
   /* set the RxPaf Packet Sizes, Min-64, Max-1600 bytes. */
   XP_REGS->ulRxPafPktSize = (RP_MIN_FRAG_PKT_SIZE << RP_MIN_PACKET_SIZE_SHIFT) | 
                             (RP_MAX_FRAG_PKT_SIZE << RP_MAX_PACKET_SIZE_SHIFT) ;
#endif

   /* Set the packet overrun delay to disabled to handle bursty
    * iMIX traffic from Spirent test center, as well as to apply back pressure to PHY.
    */
   XP_REGS->ulRxPktBufFifoDelay[0] = 0xFFFFFFFF;
   XP_REGS->ulRxPktBufFifoDelay[1] = 0xFFFFFFFF;

   XP_REGS->ulTxSarCfg |= i;
   XP_REGS->ulRxSarCfg = j;

   m_ulTrafficType = TRAFFIC_TYPE_NOT_CONNECTED;
#if defined(CONFIG_BCM_55153_DPU)
   m_ulXTMLinkMode = XTM_LINK_MODE_GFAST ;
#else
   m_ulXTMLinkMode = XTM_LINK_MODE_UNKNOWN ;
#endif
   m_ulXTMNitroMode = XTM_MODE_NITRO_DISABLE ;

   XP_REGS->ulTxSchedCfg = TXSCH_SHAPER_RESET;

   for( i = 0; i < 1000; i++ )
      ;

   XP_REGS->ulTxLineRateTimer = 0 ;
   XP_REGS->ulTxSchedCfg = 0;

   XP_REGS->ulRxUtopiaCfg = 0 ;

   switch( (m_InitParms.ulPortConfig & PC_INTERNAL_EXTERNAL_MASK) )
   {
#if defined(CONFIG_BCM963268)
      case PC_INTERNAL_EXTERNAL:
         XtmOsPrintf ("bcmxtmcfg: Ports Internal & External \n") ;
         GPIO->GPIOBaseMode     |= GPIO_BASE_UTOPIA_OVERRIDE ; /* Only for Utopia */
         ucInternalPort [PHY_PORTID_0] = 0x01 ;
         ucInternalPort [PHY_PORTID_1] = 0x00 ;
         ucInternalPort [PHY_PORTID_2] = 0x00 ;  /* Not used */
         ucInternalPort [PHY_PORTID_3] = 0x00 ;  /* Not used */
         break;

      case PC_ALL_EXTERNAL:
         XtmOsPrintf ("bcmxtmcfg: ALL Ports External \n") ;
         GPIO->GPIOBaseMode     |= GPIO_BASE_UTOPIA_OVERRIDE ; /* Only for Utopia */
         memset(ucInternalPort, 0x00, sizeof(ucInternalPort));
         break;
#endif
      /* case PC_ALL_INTERNAL: */
      default:
#if defined(CONFIG_BCM963268)
         XP_REGS->ulRxUtopiaCfg |= RXUTO_INTERNAL_BUF0_EN;
#endif
         memset(ucInternalPort, 0x01, sizeof(ucInternalPort));
         break;
   }

   XP_REGS->ulRxAalMaxSdu = 0xffff;

   /* Initialize transmit and receive connection tables. */
   for( i = 0; i < XP_MAX_CONNS; i++ )
      XP_REGS->ulTxVpiVciTable[i] = 0;

   for( i = 0; i < XP_MAX_CONNS * 2; i++ )
      XP_REGS->ulRxVpiVciCam[i] = 0;

   /* Add headers. */
   UINT32 Hdrs[XP_MAX_TX_HDRS][XP_TX_HDR_WORDS] = PKT_HDRS;
   UINT32 ulHdrOffset = 0; /* header offset in packet */

   for( i = 0, k = PKT_HDRS_START_IDX; Hdrs[k][0] != PKT_HDRS_END; i++, k++ )
   {
      /* p[0] = header length, p[1]... = header contents */
      p = Hdrs[k];
      XP_REGS->ulTxHdrInsert[i] =
         ((p[0] << TXHDR_COUNT_SHIFT) & TXHDR_COUNT_MASK) |
         ((ulHdrOffset << TXHDR_OFFSET_SHIFT) & TXHDR_OFFSET_MASK);

      for( j = 0; j < p[0] / sizeof(UINT32); j++ )
         XP_REGS->ulTxHdrValues[i][j] = p[1 + j];

      if( p[0] % sizeof(UINT32) != 0 )
         XP_REGS->ulTxHdrValues[i][j] = p[1 + (p[0] / sizeof(UINT32))];
   }

   /* Read to clear MIB counters. Set "clear on read" as default. */
   XP_REGS->ulMibCtrl = 1;

   for(p = XP_REGS->ulTxPortPktOctCnt; p <= &XP_REGS->ulBondOutputCellCnt; p++)
      i = *(volatile UINT32 *)p;

   for(p = XP_REGS->ulTxVcPktOctCnt; p < XP_REGS->ulTxVcPktOctCnt+XP_MAX_CONNS; p++)
      i = *(volatile UINT32 *)p;

   XP_REGS->ulRxPktBufMibMatch = 0;
   for( i = 0; i < XP_RX_MIB_MATCH_ENTRIES; i++ )
   {
      j = XP_REGS->ulRxPktBufMibRxOctet;
      k = XP_REGS->ulRxPktBufMibRxPkt;
   }

   /* Configure Rx Filter Mib control register. */
   XP_REGS->ulRxPktBufMibCtrl = 0;

   /* Mask and clear interrupts. */
   XP_REGS->ulIrqMask = ~INTR_MASK;
   XP_REGS->ulIrqStatus = ~0;

   /* Initialize interfaces. */
   for( i = PHY_PORTID_0; i < MAX_INTERFACES; i++ ) {
      if (pInitParms && pfnXtmrtReq) {
         m_LogPhyPortmap[i].LogicalPortId = i;
         m_LogPhyPortmap[i].PhysicalPortId = PORT_PHY_INVALID;
      }
      if ((m_InitParms.bondConfig.sConfig.ptmBond == BC_PTM_BONDING_ENABLE) ||
            (m_InitParms.bondConfig.sConfig.atmBond == BC_ATM_BONDING_ENABLE)) {
         if (m_InitParms.bondConfig.sConfig.dualLat == BC_DUAL_LATENCY_ENABLE) {
            if (i < PHY_PORTID_2)
               m_Interfaces[i].Initialize(i, ucInternalPort[i], i+PHY_PORTID_2, 
                                          m_InitParms.bondConfig.sConfig.autoSenseAtm,
                                          m_pfnXtmrtReq);
            else
               m_Interfaces[i].Initialize(i, ucInternalPort[i], i-PHY_PORTID_2,
                                          m_InitParms.bondConfig.sConfig.autoSenseAtm,
                                          m_pfnXtmrtReq);
         }
         else {
            /* No dual latency. Hard code it for the interfaces as per chip
             * definitions
             */
            UINT32 ulBondingPort ;
            switch (i) {
               case PHY_PORTID_0  :
                  ulBondingPort = PHY_PORTID_1 ;
                  break ;
               case PHY_PORTID_1  :
                  ulBondingPort = PHY_PORTID_0 ;
                  break ;
               default    :
                  ulBondingPort = MAX_INTERFACES ;
                  break ;
            }
            m_Interfaces[i].Initialize(i, ucInternalPort[i], ulBondingPort,
                                          m_InitParms.bondConfig.sConfig.autoSenseAtm,
                                          m_pfnXtmrtReq);
         }
      }
      else {
         m_Interfaces[i].Initialize(i, ucInternalPort[i], MAX_INTERFACES,
                                          m_InitParms.bondConfig.sConfig.autoSenseAtm, m_pfnXtmrtReq);
      }
   } /* for (i, MAX_INTERFACES) */
   if(pInitParms != NULL && pfnXtmrtReq != NULL)
      m_ConnTable.Initialize(&m_ConnTable);

   if( pInitParms)
   {
      /* First time that this Initialize function has been called.  Call the
       * bcmxtmrt driver initialization function.
       */
      XTMRT_GLOBAL_INIT_PARMS GlobInit;
      memcpy(GlobInit.ulReceiveQueueSizes, m_InitParms.ulReceiveQueueSizes,
            sizeof(GlobInit.ulReceiveQueueSizes));
      GlobInit.bondConfig.uConfig = m_InitParms.bondConfig.uConfig;
      GlobInit.ulMibRxClrOnRead   = PBMIB_CLEAR;
      GlobInit.pulMibTxOctetCountBase =  XP_REGS->ulTxVcPktOctCnt;
      GlobInit.pulMibRxCtrl           = &XP_REGS->ulRxPktBufMibCtrl;
      GlobInit.pulMibRxMatch          = &XP_REGS->ulRxPktBufMibMatch;
      GlobInit.pulMibRxOctetCount     = &XP_REGS->ulRxPktBufMibRxOctet;
      GlobInit.pulMibRxPacketCount    = &XP_REGS->ulRxPktBufMibRxPkt;
      GlobInit.pulRxCamBase           = &XP_REGS->ulRxVpiVciCam[0];
      if(m_pfnXtmrtReq != NULL) {
          (*m_pfnXtmrtReq) (NULL, XTMRT_CMD_GLOBAL_INITIALIZATION, &GlobInit);
      }

      bxStatus = m_OamHandler.Initialize( m_pfnXtmrtReq, this );

      if (bxStatus != XTMSTS_SUCCESS) {
          /* OAM failed to initialize.  Quit initialization. */
          XtmOsPrintf ("bcmxtmcfg: ATM Modes Initialization Error. Can't initialize OAM Handler!  FATAL!!!! \n") ;
          return(bxStatus);
      }

      /* Init PAF params */

      /* Set the link list upper length based on processor type and revision */
#if defined(CONFIG_BCM963268)
      /* Are we in a 63268 processor of revision C0 or earlier? */
      if((PERF->RevID & REV_ID_MASK) == 0xC0)
          /* Yes.  Use a linked list limit with a safety margin. */
          m_ulRxPafLongLinkListThreshold = RXPAF_LONG_LINK_LIST_THRESHOLD_63268C0;
      else
          /* No.  Use a linked list limit that uses all available entires,
             since D0 and later revisions for the 268 do not have a problem. */
          m_ulRxPafLongLinkListThreshold = RXPAF_LONG_LINK_LIST_THRESHOLD;

#else
      /* Set the long linked list limit */
      m_ulRxPafLongLinkListThreshold = RXPAF_LONG_LINK_LIST_THRESHOLD;
#endif

      /* Set the short (minimum) linked list limit */
      m_ulRxPafShortLinkListThreshold = RXPAF_SHORT_LINK_LIST_THRESHOLD;
   } /* End pInitParms test */

   /* Uninitialize ASM handler to wipe an old configuration, then initialize it again. */
   m_AsmHandler.Uninitialize();

   /* Set up the ATM autosense object */
   bxStatus = m_XtmAutoSense.Initialize(pInitParms);
   m_XtmAutoSense.initializeVars (m_InitParms.bondConfig.sConfig.autoSenseAtm == BC_ATM_AUTO_SENSE_ENABLE);

   /* Did autosense initialize work? */
   if (bxStatus != XTMSTS_SUCCESS) {
      /* No - flag error and then try to uninitialize and deallocate */
      XtmOsPrintf ("bcmxtmcfg: ATM Bonding Initialization Error. Can't initialize Autosense!  FATAL!!!! \n") ;
      m_XtmAutoSense.Uninitialize() ;

      /* Quit initialization. */
      return(bxStatus);
   }

   /* Now we're ready to do a full initialization */
   m_AsmHandler.initializeVars ();
   bxStatus = m_AsmHandler.Initialize( m_InitParms.bondConfig,
           m_pfnXtmrtReq, &m_Interfaces[0], &m_ConnTable, &m_XtmAutoSense );

   /* Did initialize work? */
   if (bxStatus != XTMSTS_SUCCESS) {
      /* No - flag error and then try to uninitialize and deallocate */
      XtmOsPrintf ("bcmxtmcfg: ATM Bonding Initialization Error. Can't initialize ASM handler!  FATAL!!!! \n") ;
      m_AsmHandler.Uninitialize() ;
      m_XtmAutoSense.Uninitialize() ;
      m_OamHandler.Uninitialize() ;

      /* Quit initialization. */
      return(bxStatus);

   }

   XtmOsStopTimer ((void *) XtmMonitorTimerCb);

   /* Initialization is complete.  Launch timer task if it was required. */
   pBondTmrParm = (void *) this ;
   //XtmOsPrintf ("XTM: pBondTmrParm = %lx \n", pBondTmrParm) ;

   XtmOsStartTimer ((void *) XtmMonitorTimerCb,  0,
         XTM_BOND_DSL_MONITOR_TIMEOUT_MS) ;

   return( bxStatus );
} /* Initialize */

/***************************************************************************
 * Function Name: LinkModeDelay
 * Description  : Read the DSL Mib Statuses and determine the actual XTM link
 *                mode.
 * Returns      : void
 ***************************************************************************/
UINT32  XTM_PROCESSOR::LinkModeDelay (void)
{
   UINT32 ulDelay ;

   switch (m_ulXTMLinkMode) {

      case XTM_LINK_MODE_ADSL      :
         ulDelay = XTM_ADSL_MODE_BONDING_DELAY ;
         break ;
      case XTM_LINK_MODE_VDSL      :
         ulDelay = XTM_VDSL_MODE_BONDING_DELAY ;
         break ;
      case XTM_LINK_MODE_VDSL_RTX  :
         ulDelay = XTM_VDSL_RTX_BONDING_DELAY ;
         break ;
      case XTM_LINK_MODE_GFAST     :
         ulDelay = XTM_GFAST_BONDING_DELAY ;
         break ;
      default                             :
         ulDelay = XTM_UNKNOWN_MODE_BONDING_DELAY ;
         break ;
   }

   return (ulDelay) ;

} /* LinkModeDelay */


#if defined(CONFIG_BCM963268)
/***************************************************************************
 * Function Name: ReprogramRxPaf
 * Description  : Reprogramming RxPAF based on bonding status taking
 *                differential factors.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
void XTM_PROCESSOR::ReprogramRxPaf ( UINT32 ulTrafficType, UINT32 ulPort,
             PXTM_INTERFACE_LINK_INFO pLinkInfo, UINT32 ulBondingPort,
             PXTM_INTERFACE_LINK_INFO pOtherLinkInfo)
{
   UINT32 ulPortDiffDelay, ulBondingPortDiffDelay, ulOverHead ;
   UINT32 ulCellFullPort, ulCellFullBondingPort, ulCellFullWord, ulTimeOutWord, ulRxBondTimeOut ;
   PXTM_INTERFACE_LINK_DELAY pLinkDelay, pOtherLinkDelay ;

   XtmOsPrintf ("bcmxtmcfg: ReprogramRxPaf traffictype %d \n", ulTrafficType) ;

   if (ulTrafficType == TRAFFIC_TYPE_PTM_BONDED)
      ulOverHead = RXPAF_PTM_OVERHEAD ;
   else
      ulOverHead = RXPAF_ATM_OVERHEAD ;

   if ((pLinkInfo->ulLinkState == LINK_UP) && (pOtherLinkInfo->ulLinkState == LINK_UP)) {

      pLinkDelay      = m_Interfaces[ulPort].GetLinkDelay() ;
      pOtherLinkDelay = m_Interfaces[ulBondingPort].GetLinkDelay () ;

      ulPortDiffDelay = (pLinkDelay->ulLinkDsBondingDelay + RXPAF_JITTER) ;
      ulBondingPortDiffDelay = (pOtherLinkDelay->ulLinkDsBondingDelay + RXPAF_JITTER) ;

      ulCellFullPort        = ((ulPortDiffDelay * pLinkInfo->ulLinkDsRate)/1000)/
                       (((60 * 8 * (100-ulOverHead))/100)) ;
      ulCellFullBondingPort = ((ulBondingPortDiffDelay * pOtherLinkInfo->ulLinkDsRate)/1000)/
                       (((60 * 8 * (100-ulOverHead))/100)) ;

      /* Shorter link-list must be more than 10 cells.
         Longer link-list must be equal or less than 1082 cells */

      if (ulCellFullPort > m_ulRxPafLongLinkListThreshold)
         ulCellFullPort = m_ulRxPafLongLinkListThreshold ;
      else if (ulCellFullPort < m_ulRxPafShortLinkListThreshold)
         ulCellFullPort = m_ulRxPafShortLinkListThreshold ;

      if (ulCellFullBondingPort > m_ulRxPafLongLinkListThreshold)
         ulCellFullBondingPort = m_ulRxPafLongLinkListThreshold ;
      else if (ulCellFullBondingPort < m_ulRxPafShortLinkListThreshold)
         ulCellFullBondingPort = m_ulRxPafShortLinkListThreshold ;

      if (ulPort == PHY_PORTID_0) {
         ulCellFullWord = ulCellFullPort | (ulCellFullBondingPort << RXBOND_CELL_FULL_WR_CHAN_SHIFT) ;
         ulTimeOutWord = ((((ulPortDiffDelay*4)/5)+1)*RXBOND_TIMEOUT_1_25_MSECS) | 
                           (((((ulBondingPortDiffDelay*4)/5)+1)*RXBOND_TIMEOUT_1_25_MSECS) << RXBOND_TIMEOUT_WR_CHAN_SHIFT) ;
      }
      else {
         ulCellFullWord = ulCellFullBondingPort | (ulCellFullPort << RXBOND_CELL_FULL_WR_CHAN_SHIFT) ;
         ulTimeOutWord = ((((ulBondingPortDiffDelay*4)/5)+1)*RXBOND_TIMEOUT_1_25_MSECS) |
                           (((((ulPortDiffDelay*4)/5)+1)*RXBOND_TIMEOUT_1_25_MSECS) << RXBOND_TIMEOUT_WR_CHAN_SHIFT) ;
      }

      XP_REGS->ulRxBondCellFull [0] = ulCellFullWord;  /* Non preemptive 0, 1 */
      XP_REGS->ulRxBondCellFull [2] = ulCellFullWord;  /* Preemptive 4, 5 */

      /* Set the RxPaf Timeout Registers to 2 ms */
      ulRxBondTimeOut = XP_REGS->ulRxBondTimeout0 ;
      ulRxBondTimeOut >>= 16 ;
      ulRxBondTimeOut <<= 16 ;
      ulRxBondTimeOut |= ulTimeOutWord ;

      XP_REGS->ulRxBondTimeout0 = ulRxBondTimeOut ; /* Non-preemptive 0, 1 */
      XP_REGS->ulRxBondTimeout1 = ulRxBondTimeOut ; /* Preemptive 4, 5 */
   }
   else {
      /* Disable Cell Full handling for the single line mode */
      XP_REGS->ulRxBondCellFull [0] = 0x04440444;
      XP_REGS->ulRxBondCellFull [2] = 0x04440444;

      /* Set the RxPaf Timeout Registers to 2 ms */
      XP_REGS->ulRxBondTimeout0 = 0x08080808 ;
      XP_REGS->ulRxBondTimeout1 = 0x08080808 ;
   }
}

#else

/* 63138, 148, 158 platforms have increased SAR RxPAF buffering
 */
/***************************************************************************
 * Function Name: ReprogramRxPaf
 * Description  : Reprogramming RxPAF based on bonding status taking
 *                differential factors.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
void XTM_PROCESSOR::ReprogramRxPaf ( UINT32 ulTrafficType, UINT32 ulPort,
             PXTM_INTERFACE_LINK_INFO pLinkInfo, UINT32 ulBondingPort,
             PXTM_INTERFACE_LINK_INFO pOtherLinkInfo)
{
   UINT32 ulPortDiffDelay, ulBondingPortDiffDelay, ulOverHead ;
   UINT32 ulCellFullPort, ulCellFullBondingPort, ulCellFullWord, ulTimeOutWord, ulTimeOutWord1, ulRxBondTimeOut ;
   UINT32 ulBaseTimerUs ;

   XtmOsPrintf ("bcmxtmcfg: ReprogramRxPaf traffictype %d \n", ulTrafficType) ;

   /*
    It's based on 
    SAR_RXBOND_RXBOND_BASE_TIMER - RxPaf Bonding Base Timer
    Offset (byte space) = 32'h80130930
    23:00		TIMER	RxPaf Base Timer Register: 200MHz clock divider, used to trigger timeout counters
 
    register which is 0x15552. It means the counter is counting every  0x15552/200 = 436.89us = 0.437 ms

    Timeout counter per channel is a byte value, we can’t set it beyond 111.4 ms.
    */ 

   if ((pLinkInfo->ulLinkState == LINK_UP) && (pOtherLinkInfo->ulLinkState == LINK_UP)) {

      ulBaseTimerUs = XP_REGS->ulRxBondBaseTimer/RXBOND_CLK_DIVIDER ;
      ulBaseTimerUs = (ulBaseTimerUs != 0) ? ulBaseTimerUs : 1 ;

      ulPortDiffDelay = ulBondingPortDiffDelay = LinkModeDelay ();

      ulCellFullPort        = ((ulPortDiffDelay * pLinkInfo->ulLinkDsRate)/1000)/
                       (((60 * 8 * (100-ulOverHead))/100)) ; /* Convert to cells - Overhead */
      ulCellFullBondingPort = ((ulBondingPortDiffDelay * pOtherLinkInfo->ulLinkDsRate)/1000)/
                       (((60 * 8 * (100-ulOverHead))/100)) ; /* Convert to cells - overhead */

      /* Shorter link-list must be more than 10 cells.
         Longer link-list must be equal or less than 1082 cells */

      if (ulCellFullPort > m_ulRxPafLongLinkListThreshold)
         ulCellFullPort = m_ulRxPafLongLinkListThreshold ;
      else if (ulCellFullPort < m_ulRxPafShortLinkListThreshold)
         ulCellFullPort = m_ulRxPafShortLinkListThreshold ;

      if (ulCellFullBondingPort > m_ulRxPafLongLinkListThreshold)
         ulCellFullBondingPort = m_ulRxPafLongLinkListThreshold ;
      else if (ulCellFullBondingPort < m_ulRxPafShortLinkListThreshold)
         ulCellFullBondingPort = m_ulRxPafShortLinkListThreshold ;

      ulCellFullWord = (RXBOND_CELL_FULL_LIST_MAX | (RXBOND_CELL_FULL_LIST_MAX << RXBOND_CELL_FULL_WR_CHAN_SHIFT)) ;

      if (ulPort == PHY_PORTID_0) {
         ulTimeOutWord = ((ulPortDiffDelay*1000)/ulBaseTimerUs) ; /* MilliSec to us */
         ulTimeOutWord = (ulTimeOutWord > RXBOND_MAX_TIMEOUT_PER_WRCHN) ? RXBOND_MAX_TIMEOUT_PER_WRCHN : ulTimeOutWord ;

         ulTimeOutWord1 = ((ulBondingPortDiffDelay*1000)/ulBaseTimerUs) ;
         ulTimeOutWord1 = (ulTimeOutWord1 > RXBOND_MAX_TIMEOUT_PER_WRCHN) ? RXBOND_MAX_TIMEOUT_PER_WRCHN : ulTimeOutWord1 ;

         ulTimeOutWord  |= (ulTimeOutWord1 << RXBOND_TIMEOUT_WR_CHAN_SHIFT) ;
      }
      else {
         ulTimeOutWord = ((ulBondingPortDiffDelay*1000)/ulBaseTimerUs) ;
         ulTimeOutWord = (ulTimeOutWord > RXBOND_MAX_TIMEOUT_PER_WRCHN) ? RXBOND_MAX_TIMEOUT_PER_WRCHN : ulTimeOutWord ;

         ulTimeOutWord1 = ((ulPortDiffDelay*1000)/ulBaseTimerUs) ;
         ulTimeOutWord1 = (ulTimeOutWord1 > RXBOND_MAX_TIMEOUT_PER_WRCHN) ? RXBOND_MAX_TIMEOUT_PER_WRCHN : ulTimeOutWord1 ;

         ulTimeOutWord  |= (ulTimeOutWord1 << RXBOND_TIMEOUT_WR_CHAN_SHIFT) ;
      }

      /* Write channels 0, 2 are used. Other Wr channels are not used as only
      ** non-preemptive, single latency exists */

      XP_REGS->ulRxBondCellFull [0] = ulCellFullWord;  /* Non preemptive 0, 1 */
      XP_REGS->ulRxBondCellFull [2] = ulCellFullWord;  /* Preemptive 4, 5 */

      ulRxBondTimeOut = XP_REGS->ulRxBondTimeout0 ;
      ulRxBondTimeOut >>= 16 ;
      ulRxBondTimeOut <<= 16 ;
      ulRxBondTimeOut |= ulTimeOutWord ;

      XP_REGS->ulRxBondTimeout0 = ulRxBondTimeOut ; /* Non-preemptive 0, 1 */
      XP_REGS->ulRxBondTimeout1 = ulRxBondTimeOut ; /* Preemptive 4, 5 */
   }
   else {
      /* Disable Cell Full handling for the single line mode */
      XP_REGS->ulRxBondCellFull [0] = RXBOND_CELL_FULL_LIST_MAX | (RXBOND_CELL_FULL_LIST_MAX << RXBOND_CELL_FULL_WR_CHAN_SHIFT) ;
      XP_REGS->ulRxBondCellFull [2] = RXBOND_CELL_FULL_LIST_MAX | (RXBOND_CELL_FULL_LIST_MAX << RXBOND_CELL_FULL_WR_CHAN_SHIFT) ;

      /* Set the RxPaf Timeout Registers to 27 ms as there could be ReTx delay.
      ** All channels 0-7
      **/
      XP_REGS->ulRxBondTimeout0 = (RXBOND_DEF_TIMEOUT_PER_WRCHN) | (RXBOND_DEF_TIMEOUT_PER_WRCHN << 8) | 
                                  (RXBOND_DEF_TIMEOUT_PER_WRCHN << 16) | (RXBOND_DEF_TIMEOUT_PER_WRCHN << 24) ;
      XP_REGS->ulRxBondTimeout1 = (RXBOND_DEF_TIMEOUT_PER_WRCHN) | (RXBOND_DEF_TIMEOUT_PER_WRCHN << 8) | 
                                  (RXBOND_DEF_TIMEOUT_PER_WRCHN << 16) | (RXBOND_DEF_TIMEOUT_PER_WRCHN << 24) ;
   }
}
#endif

 

/***************************************************************************
 * Function Name: ReconfigureSAR
 * Description  : Reconfigures the 63268 SAR based on the traffic type.
 *                Bonding/Normal modes can change dynamically, atleast the SW
 *                provisions for the same. 
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
void XTM_PROCESSOR::ReconfigureSAR ( UINT32 ulPort, UINT32 ulTrafficType )
{
   UINT32   i,j,k,l,m ;
#if defined(CONFIG_BCM963268)
   UINT32 ulRam, ulCam ;
#endif
   BCMXTM_STATUS bxStatus = XTMSTS_SUCCESS ;

   XtmOsPrintf ("bcmxtmcfg: ReconfSAR P-%d, T-%d \n", ulPort, ulTrafficType) ;

   switch (ulTrafficType) {

      case   TRAFFIC_TYPE_PTM_BONDED   :

         if (m_ulTrafficType != ulTrafficType) {

            i = XP_REGS->ulTxSarCfg;
            i |= TXSARA_BOND_EN;

            /* Unconfigure any ATM/ ATM bonding related settings */
            j = XP_REGS->ulRxSarCfg;

            i &= ~(TXSARA_SID12_EN | TXSARA_BOND_DUAL_LATENCY);
            j &= ~(RXSARA_SID12_EN | RXSARA_BOND_DUAL_LATENCY);
            j |= RXSARA_BOND_EN;

            k = XP_REGS->ulTxUtopiaCfg;
#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178) 
            k |= TXUTO_CELL_FIFO_DEPTH_1; /* Set to default depth 1 */
#else
            k &= ~TXUTO_CELL_FIFO_DEPTH_1; /* Set to default depth 2 */
#endif

            l = (RP_BONDING_6465B_MIN_FRAG_SIZE << RP_MIN_FRAGMENT_SIZE_SHIFT) |
                (RP_BONDING_6465B_MAX_FRAG_SIZE << RP_MAX_FRAGMENT_SIZE_SHIFT) ;

            /* Configure PTM_BONDED_Port Pairs in SHPR */
            if (m_InitParms.bondConfig.sConfig.dualLat == BC_DUAL_LATENCY_ENABLE)
               m = (PHY_PORTID_0 << 2 | PHY_PORTID_2) |
                   (PHY_PORTID_1 << 18 | PHY_PORTID_3 << 16) ;
            else
               m = (PHY_PORTID_0 << 2 | PHY_PORTID_1) ;

            XP_REGS->ulTxSarCfg = i;
            XP_REGS->ulRxSarCfg = j;
            XP_REGS->ulTxUtopiaCfg = k;
            XP_REGS->ulRxPafFragSize = l;
            XP_REGS->ulSsPtmBondedPair = m ;

            /* Disable for bonding. RxPaf bonding controller will handle
             * drop/overflow for bonding, but we do need to set for higher rates
             * vdsl bonding at RxBondCellFull registers.
             * Setting it to disabled to utilize the buffering at SAR/PHY/Headend
             * CO.
             */
                XP_REGS->ulRxPktBufFifoDelay[0] = 0xFFFFFFFF;
                XP_REGS->ulRxPktBufFifoDelay[1] = 0xFFFFFFFF;

            XP_REGS->ulRxBondCellFull [0] = 0x04440444;
            XP_REGS->ulRxBondCellFull [2] = 0x04440444;

            XP_REGS->ulRxBondConfig &= ~RB_CELL_FULL_WAIT_MASK ;

            /* Set the RxPaf Timeout Registers to 2 ms */
            XP_REGS->ulRxBondTimeout0 = 0x08080808 ;
            XP_REGS->ulRxBondTimeout1 = 0x08080808 ;

#ifdef TESTING_D0
            i = XP_REGS->ulSsNewCfgD0_or_Reserved5 ;
            i |= SHPR_NEWCFGD0_EN24HDR ;
            XP_REGS->ulSsNewCfgD0_or_Reserved5 = i ;
#endif

            m_ulTrafficType = ulTrafficType ;

            /* RX PAF hangs in some situations in 63268, which we addressed in
             * future chipsets including 63138.
             * Making the below only for 63268 base.
             */
#if defined(CONFIG_BCM963268)
            XP_REGS->ulRxVpiVciCam[(MAX_VCIDS * 2) + 1] = 0 ;
            XP_REGS->ulRxVpiVciCam[MAX_VCIDS * 2]       = 0 ;
            m_ulRxPafLockupMonitorEvtValid = XTM_BOND_RX_PAF_LOCKUP_MONITOR_TIMEOUT_MS ;
            XP_REGS->ulRxBondSIDOutCnt = RXBOND_SIDOUT_CNT_DISABLE_UPDATE;
#else
            XP_REGS->ulRxBondSIDOutCnt = RXBOND_SIDOUT_CNT_ENABLE_UPDATE;
#endif
         }

         m_ulRxBondPrevErrStatus = XP_REGS->ulRxBondErrorStatus ;
         m_ulRxBondPrevExpSid01  = XP_REGS->ulRxBondExpectedSid0 ;
         m_ulRxBondPrevExpSid23  = XP_REGS->ulRxBondExpectedSid1 ;


         /* Set the OutputDelayCount/InputDelayCount for RxPAF to Non-Zero to benefit
          * GINP+SRA deployments with Noise (Both for Non-bonding & Bonding)
          *
          * Ref - RxPaf App Note in Commengine/bonding references.
          * Following values are IOP tested with ALU line cards.
          */

          XP_REGS->ulRxPafTestMode0 &= ~RP_TEST_MODE0_OUTPUT_DELAY_COUNT_MASK ;
          XP_REGS->ulRxPafTestMode0 &= ~RP_TEST_MODE0_INPUT_DELAY_COUNT_MASK ;

          XP_REGS->ulRxPafTestMode0 |= (0x0 << RP_TEST_MODE0_OUTPUT_DELAY_COUNT_SHIFT) ;
          XP_REGS->ulRxPafTestMode0 |= (0x0 << RP_TEST_MODE0_INPUT_DELAY_COUNT_SHIFT) ;

#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963158)
          /* Check TxPAF status */
          m_ulTxPafEnabled = TXPAF_REGS->ulControl & TXPAF_PROCESSOR_CONTROL_ENABLE_MASK ;
          XtmOsPrintf ("bcmxtmcfg: TxPAF Status = %s \n", (m_ulTxPafEnabled==1) ? "Enabled" : "Disabled") ;
#else
          m_ulTxPafEnabled = 0 ;
#endif

          MapXtmOsPrintf ("bcmxtmcfg: PTM Bonding enabled \n");

         //m_Interfaces[ulPort].SetLinkDataStatus (DATA_STATUS_ENABLED) ;
         break;

      case   TRAFFIC_TYPE_ATM_BONDED   :

         if (m_ulTrafficType != ulTrafficType) {

                /* Uninitialize ASM handler to wipe an old configuration, then initialize it again. */
                m_AsmHandler.Uninitialize();
                m_AsmHandler.initializeVars ();
                bxStatus = m_AsmHandler.Initialize( m_InitParms.bondConfig,
                       m_pfnXtmrtReq, &m_Interfaces[0], &m_ConnTable, &m_XtmAutoSense );

                /* Did initialize work? */
                if (bxStatus != XTMSTS_SUCCESS) {
                   /* No - flag error and then try to uninitialize and deallocate */
                   XtmOsPrintf ("bcmxtmcfg: ATM Bonding Initialization Error. Can't initialize ASM handler!  FATAL!!!! \n") ;
                   m_AsmHandler.Uninitialize() ;
                   m_XtmAutoSense.Uninitialize() ;
                   return;
                }

            i = XP_REGS->ulTxSarCfg;
            j = XP_REGS->ulRxSarCfg;
            k = XP_REGS->ulTxUtopiaCfg;

            i |= TXSARA_BOND_EN;
            j |= RXSARA_BOND_EN;
            k |= TXUTO_CELL_FIFO_DEPTH_1;

            /* Configure for 12-bit SID by default */
            if (m_InitParms.bondConfig.sConfig.dualLat == BC_DUAL_LATENCY_ENABLE) {
               i |= TXSARA_BOND_DUAL_LATENCY;
               j |= RXSARA_BOND_DUAL_LATENCY;
            }
            else {
               i &= ~TXSARA_BOND_DUAL_LATENCY;
               j &= ~RXSARA_BOND_DUAL_LATENCY;
            }

            i &= ~TXSARA_ASM_CRC_DIS ;
            j &= ~RXSARA_ASM_CRC_DIS ;

            l = (RP_DEF_MIN_FRAG_SIZE << RP_MIN_FRAGMENT_SIZE_SHIFT) |
               (RP_DEF_MAX_FRAG_SIZE << RP_MAX_FRAGMENT_SIZE_SHIFT) ;

            XP_REGS->ulTxSarCfg = i;
            XP_REGS->ulRxSarCfg = j;
            XP_REGS->ulTxUtopiaCfg = k;
            XP_REGS->ulRxPafFragSize = l;

            /* Disable for bonding. RxPaf bonding controller will handle
             * drop/overflow for bonding, but we do need to set for higher rates
             * vdsl bonding at RxBondCellFull registers.
             * Setting it to disabled to utilize the buffering at SAR/PHY/Headend
             * CO.
             */
               XP_REGS->ulRxPktBufFifoDelay[0] = 0xFFFFFFFF;
               XP_REGS->ulRxPktBufFifoDelay[1] = 0xFFFFFFFF;

            XP_REGS->ulRxBondCellFull [0] = 0x04440444;
            XP_REGS->ulRxBondCellFull [2] = 0x04440444;

            XP_REGS->ulRxBondConfig &= ~RB_CELL_FULL_WAIT_MASK ;

            /* Set the RxPaf Timeout Registers to 2 ms */
            XP_REGS->ulRxBondTimeout0 = 0x08080808 ;
            XP_REGS->ulRxBondTimeout1 = 0x08080808 ;

#if defined(CONFIG_BCM963268)
            /* Set the reserve RxVCAM entry for idle cells IOP-Zhone issue */
            ulCam = PHY_PORTID_0 | (IDLE_CELL_VCI << RXCAMA_VCI_SHIFT) |
                (IDLE_CELL_VPI << RXCAMA_VPI_SHIFT) | RXCAM_VALID;
            ulRam = MAX_VCIDS << RXCAM_VCID_SHIFT ;
            ulRam |= RXCAM_CT_AAL0_CELL ;
            XP_REGS->ulRxVpiVciCam[(MAX_VCIDS * 2) + 1] = ulRam;
            XP_REGS->ulRxVpiVciCam[MAX_VCIDS * 2]       = ulCam;
#endif

            m_ulTrafficType = ulTrafficType ;

#if defined(CONFIG_BCM963268)
            m_ulRxPafLockupMonitorEvtValid = XTM_BOND_RX_PAF_LOCKUP_MONITOR_TIMEOUT_MS ;
            XP_REGS->ulRxBondSIDOutCnt = RXBOND_SIDOUT_CNT_DISABLE_UPDATE;
#else
            XP_REGS->ulRxBondSIDOutCnt = RXBOND_SIDOUT_CNT_ENABLE_UPDATE;
#endif
            m_ulRxBondPrevErrStatus = XP_REGS->ulRxBondErrorStatus ;
            m_ulRxBondPrevExpSid01  = XP_REGS->ulRxBondExpectedSid0 ;
            m_ulRxBondPrevExpSid23  = XP_REGS->ulRxBondExpectedSid1 ;

            XP_REGS->ulRxPafTestMode0 &= ~RP_TEST_MODE0_OUTPUT_DELAY_COUNT_MASK ;
            XP_REGS->ulRxPafTestMode0 &= ~RP_TEST_MODE0_INPUT_DELAY_COUNT_MASK ;

#if defined(CONFIG_BCM963268)
            /* Set the OutputDelayCount/InputDelayCount for RxPAF to Non-Zero to benefit
             * a workaround for ATM bonded modes to avoid DS packet loss
             * due to SAR Rx MUX issue when dealing with Bypassed/Normal packets in DS.
             *
             * Bypass traffic (ASM & Non-ASM traffic)
             * 
             * Ref - RxPaf App Note in Commengine/bonding references.
             */
            XP_REGS->ulRxPafTestMode0 |= (0xFF << RP_TEST_MODE0_OUTPUT_DELAY_COUNT_SHIFT) ;
            XP_REGS->ulRxPafTestMode0 |= (0xFF << RP_TEST_MODE0_INPUT_DELAY_COUNT_SHIFT) ;
#endif

            m_ulTxPafEnabled = 0 ;
       MapXtmOsPrintf ("bcmxtmcfg: ATM Bonding enabled \n");
    }
         break;

      default                        :
         /* Non bonding modes. */
         i = XP_REGS->ulTxSarCfg;
         i &= ~TXSARA_BOND_EN;
         XP_REGS->ulTxSarCfg = i;

         /* In case of ATM mode, Un-configure the following. */
         i = XP_REGS->ulTxSarCfg;
         j = XP_REGS->ulRxSarCfg;

         i &= ~(TXSARA_BOND_EN | TXSARA_SID12_EN | TXSARA_BOND_DUAL_LATENCY);
         j &= ~(RXSARA_BOND_EN | RXSARA_SID12_EN | RXSARA_BOND_DUAL_LATENCY);

         k = XP_REGS->ulTxUtopiaCfg;
#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178) 
         if(ulTrafficType == TRAFFIC_TYPE_PTM)
            k |= TXUTO_CELL_FIFO_DEPTH_1; /* Set to default depth 1 */
#else
         k &= ~TXUTO_CELL_FIFO_DEPTH_1; /* Set to default depth 2 */
#endif

         l = (RP_DEF_MIN_FRAG_SIZE << RP_MIN_FRAGMENT_SIZE_SHIFT) |
             (RP_DEF_MAX_FRAG_SIZE << RP_MAX_FRAGMENT_SIZE_SHIFT) ;
         
         XP_REGS->ulTxSarCfg = i ;
         XP_REGS->ulRxSarCfg = j ;
         XP_REGS->ulRxPafFragSize  = l ;
         XP_REGS->ulTxUtopiaCfg = k ;

         /* Enable for Non-bonding. Setting it to disabled to handle
          * bursty iMIX traffic from Spirent test center, as well as to utilize all the headend entity
          * (SAR/PHY/CO) buffering.
          */
             XP_REGS->ulRxPktBufFifoDelay[0] = 0xFFFFFFFF;
             XP_REGS->ulRxPktBufFifoDelay[1] = 0xFFFFFFFF;
         
         XP_REGS->ulRxBondCellFull [0] = 0x04440444;
         XP_REGS->ulRxBondCellFull [2] = 0x04440444;

         /* Set the RxPaf Timeout Registers to defaults 1.25 ms */
         XP_REGS->ulRxBondTimeout0 = 0x05050505 ;
         XP_REGS->ulRxBondTimeout1 = 0x05050505 ;

#if defined(CONFIG_BCM963268)
         XP_REGS->ulRxVpiVciCam[(MAX_VCIDS * 2) + 1] = 0 ;
         XP_REGS->ulRxVpiVciCam[MAX_VCIDS * 2]       = 0 ;
#endif

         m_ulTrafficType = ulTrafficType ;
         m_ulRxPafLockupMonitorEvtValid = 0 ;
         XP_REGS->ulRxBondSIDOutCnt = RXBOND_SIDOUT_CNT_DISABLE_UPDATE;
         m_ulRxBondPrevErrStatus = 0 ;
         m_ulRxBondPrevExpSid01  = 0 ;
         m_ulRxBondPrevExpSid23  = 0 ;

         XP_REGS->ulRxPafTestMode0 &= ~RP_TEST_MODE0_OUTPUT_DELAY_COUNT_MASK ;
         XP_REGS->ulRxPafTestMode0 &= ~RP_TEST_MODE0_INPUT_DELAY_COUNT_MASK ;

         /* Is this ATM or PTM traffic? */
         if((ulTrafficType & TRAFFIC_TYPE_ATM_MASK) == 0) {
             XP_REGS->ulRxPafTestMode0 |= (0x0 << RP_TEST_MODE0_OUTPUT_DELAY_COUNT_SHIFT) ;
             XP_REGS->ulRxPafTestMode0 |= (0x0 << RP_TEST_MODE0_INPUT_DELAY_COUNT_SHIFT) ;
         }
#if defined(CONFIG_BCM963268)
         else { 
         /* ATM traffic */

         /* Set the OutputDelayCount/InputDelayCount for RxPAF to Non-Zero to benefit
          * a workaround for ATM modes to avoid DS packet loss
          * due to SAR Rx MUX issue when dealing with Bypassed/Normal packets in DS.
          *
          * Bypass mode - ATM OAM F4 & other data traffic.
          * 
          * Ref - RxPaf App Note in Commengine/bonding references.
          * Following values are IOP tested with ALU line cards.
          */
             XP_REGS->ulRxPafTestMode0 |= (0xff << RP_TEST_MODE0_OUTPUT_DELAY_COUNT_SHIFT) ;
             XP_REGS->ulRxPafTestMode0 |= (0xff << RP_TEST_MODE0_INPUT_DELAY_COUNT_SHIFT) ;
         }
#endif

#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963158)
          /* Check TxPAF status */
          m_ulTxPafEnabled = TXPAF_REGS->ulControl & TXPAF_PROCESSOR_CONTROL_ENABLE_MASK ;
          XtmOsPrintf ("bcmxtmcfg: TxPAF Status = %s \n", (m_ulTxPafEnabled==1) ? "Enabled" : "Disabled") ;
#else
          m_ulTxPafEnabled = 0 ;
#endif

         MapXtmOsPrintf ("bcmxtmcfg: Normal(XTM/PTM) Mode enabled \n");
         break ;
   } /* switch (ulTrafficType) */

   /* Control the TC_CRC based on G.fast mode or not */
   if (m_ulXTMLinkMode == XTM_LINK_MODE_GFAST) {
      XP_REGS->ulRxPafConfig &= ~(RP_TC_CRC_EN | RP_TC_CRC_DROP_END_BYTE) ;
      XP_REGS->ulTxSarCfg    &= ~TXSARP_CRC16_EN ;
   }
   else {
      XP_REGS->ulRxPafConfig |= (RP_TC_CRC_EN | RP_TC_CRC_DROP_END_BYTE) ;
      XP_REGS->ulTxSarCfg    |= TXSARP_CRC16_EN ;
   }

}

/***************************************************************************
 * Function Name: Uninitialize
 * Description  : Undo Initialize.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_PROCESSOR::Uninitialize( FN_XTMRT_REQ pfnXtmrtReq )
{
    UINT32 i;
    XTM_CONNECTION *pConn;

    /* Set interface link state to down to disable SAR receive */
    for( i = PHY_PORTID_0; i < MAX_INTERFACES; i++ )
    {
        PXTM_INTERFACE_LINK_INFO pLi = m_Interfaces[i].GetLinkInfo();

        if (pLi->ulLinkState == LINK_UP)
        {
            XTM_INTERFACE_LINK_INFO linkInfo;

            linkInfo = *pLi;
            linkInfo.ulLinkState = LINK_DOWN;
            SetInterfaceLinkInfo( PORT_TO_PORTID(i), &linkInfo, 0 );
        }
    }

    /* Initialize() has started the timer, so stop it now */
    XtmOsStopTimer ((void *) XtmMonitorTimerCb);
    pBondTmrParm = NULL ;

    /* Delete all network device instances. */
    i = 0;
    while( (pConn = m_ConnTable.Enum( &i )) != NULL )
    {
        m_ConnTable.Remove( pConn );
        // delete pConn;
        pConn->Dxtor();
        XtmOsFree((char *)pConn);
        pConn = NULL;
    }

    /* Uninitialize interfaces. */
    for( i = PHY_PORTID_0; i < MAX_INTERFACES; i++ )
        m_Interfaces[i].Uninitialize();

    if( m_pTrafficDescrEntries )
    {
        XtmOsFree( (char *) m_pTrafficDescrEntries );
        m_pTrafficDescrEntries = NULL;
    }

    m_OamHandler.Uninitialize();
    m_AsmHandler.Uninitialize();
    m_pAsmHandler = NULL;

    m_pfnXtmrtReq = pfnXtmrtReq;
    if(m_pfnXtmrtReq != NULL) {
        (*m_pfnXtmrtReq) (NULL, XTMRT_CMD_GLOBAL_UNINITIALIZATION, NULL);
    }

    return( XTMSTS_SUCCESS );

} /* Uninitialize */

/***************************************************************************
 * Function Name: ReInitialize
 * Description  : ReInitializes the traffic Mode of XTM drivers (XtmCfg/XtmRt)
 *                based on the Updated init parameters for further data
 *                initialization.
 *                Called as currently we cannot support on the fly
 *                conversion from non-bonded to bonded and vice versa
 *                traffic types. This function enforces this condition.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_PROCESSOR::ReInitialize (UINT32 ulTrafficType)
{
   int i, ret ;
   BCMXTM_STATUS bxStatus = XTMSTS_SUCCESS ;

   switch (ulTrafficType) {
      case TRAFFIC_TYPE_PTM_BONDED     :
      case TRAFFIC_TYPE_ATM_BONDED     :
         m_InitParms.bondConfig.sConfig.ptmBond = BC_PTM_BONDING_ENABLE ;
         m_InitParms.bondConfig.sConfig.atmBond = BC_ATM_BONDING_ENABLE ;
         break ;
      case TRAFFIC_TYPE_PTM            :
      case TRAFFIC_TYPE_ATM            :
      case TRAFFIC_TYPE_PTM_RAW        :
         m_InitParms.bondConfig.sConfig.ptmBond = BC_PTM_BONDING_DISABLE ;
         m_InitParms.bondConfig.sConfig.atmBond = BC_ATM_BONDING_DISABLE ;
         break ;
      default                          :
         break ;
   }

   /* ReInitialize interfaces. */
   for( i = PHY_PORTID_0; i < MAX_INTERFACES; i++ ) {
      if ((m_InitParms.bondConfig.sConfig.ptmBond == BC_PTM_BONDING_ENABLE) ||
            (m_InitParms.bondConfig.sConfig.atmBond == BC_ATM_BONDING_ENABLE)) {
         if (m_InitParms.bondConfig.sConfig.dualLat == BC_DUAL_LATENCY_ENABLE) {
            if (i < PHY_PORTID_2)
               m_Interfaces[i].ReInitialize(i+PHY_PORTID_2) ;
            else
               m_Interfaces[i].ReInitialize(i-PHY_PORTID_2) ;
         }
         else {
            /* No dual latency. Hard code it for the interfaces as per chip
             * definitions
             */
            UINT32 ulBondingPort ;
            switch (i) {
               case PHY_PORTID_0  :
                  ulBondingPort = PHY_PORTID_1 ;
                  break ;
               case PHY_PORTID_1  :
                  ulBondingPort = PHY_PORTID_0 ;
                  break ;
               default    :
                  ulBondingPort = MAX_INTERFACES ;
                  break ;
            }
            m_Interfaces[i].ReInitialize(ulBondingPort) ;
         }
      }
      else {
         m_Interfaces[i].ReInitialize(MAX_INTERFACES);
      }
   }

   /* Call the bcmxtmrt driver ReInitialization function.*/

   XTMRT_GLOBAL_INIT_PARMS GlobInit;
   GlobInit.bondConfig.uConfig = m_InitParms.bondConfig.uConfig ;
    if(m_pfnXtmrtReq != NULL) {
        ret = (*m_pfnXtmrtReq) (NULL, XTMRT_CMD_GLOBAL_REINITIALIZATION, &GlobInit);
        if (ret != 0)
            bxStatus = XTMSTS_STATE_ERROR ;
    } else {
        bxStatus = XTMSTS_STATE_ERROR;
    }

   return (bxStatus) ;

} /* ReInitialize */


/***************************************************************************
 * Function Name: Configure
 * Description  : Set configurable parameters based on values passed from API.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_PROCESSOR::Configure (PXTM_CONFIGURATION_PARMS pConfigInfo)
{
   BCMXTM_STATUS bxStatus = XTMSTS_SUCCESS ;

    /* Are we setting traffic timeout parameter? */
    if(pConfigInfo->sParamsSelected.trafficParam==XTM_CONFIGURATION_PARM_SET)
    {
        /* New traffic timeout.  Be sure to convert from seconds to mS */
        UINT32 ulNewTrafficTimeoutMs = 
            (UINT32)(pConfigInfo->ulBondingTrafficTimeoutSeconds * (1000));

        /* Tell the user what we're doing */
        XtmOsPrintf ("  Setting traffic timeout parameter to %lu mS.  Was %lu mS.\n", 
                     ulNewTrafficTimeoutMs, m_XtmAutoSense.GetBondingTrafficTimeoutMs());

        /* Set traffic timeout. */
        m_XtmAutoSense.SetBondingTrafficTimeoutMs(ulNewTrafficTimeoutMs);
    }

    /* Are we dumping traffic timeout parameter? */
    if(pConfigInfo->sParamsSelected.trafficParam==XTM_CONFIGURATION_PARM_DUMP)
    {
        /* Tell the user the value of the parameter */
        XtmOsPrintf ("  Existing traffic timeout parameter is %lu mS.\n", 
                     m_XtmAutoSense.GetBondingTrafficTimeoutMs());
    }

    /* Are we setting single line timeout parameter? */
    if(pConfigInfo->sParamsSelected.singleLineParam==XTM_CONFIGURATION_PARM_SET)
    {
        /* New single line timeout.  Be sure to convert from seconds to mS */
        UINT32 ulNewSingleLineTimeoutMs = 
            (UINT32)(pConfigInfo->ulSingleLineTimeoutSeconds * (1000));

        /* Tell the user what we're doing */
        XtmOsPrintf ("  Setting single line timeout parameter to %lu mS.  Was %lu mS.\n", 
                     ulNewSingleLineTimeoutMs, m_XtmAutoSense.GetOneLineTimeoutMs());

        /* Set single line timeout. */
        m_XtmAutoSense.SetOneLineTimeoutMs(ulNewSingleLineTimeoutMs);
    }

    /* Are we dumping single line timeout parameter? */
    if(pConfigInfo->sParamsSelected.singleLineParam==XTM_CONFIGURATION_PARM_DUMP)
    {
        /* Tell the user the value of the parameter */
        XtmOsPrintf ("  Existing single line timeout parameter is %lu mS.\n", 
                     m_XtmAutoSense.GetOneLineTimeoutMs());
    }

   return (bxStatus) ;

} /* Configure */


/***************************************************************************
 * Function Name: validateThreshold
 * Description  : valiate threshold/mode parameters that is applicable for tx
 *                queues (get/set operations)
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_PROCESSOR::validateThreshold (UINT32 ulThreshold)
{
   BCMXTM_STATUS bxStatus = XTMSTS_SUCCESS ;

   if(ulThreshold < XTM_XDSL_MODE_MIN_THRESHOLD || ulThreshold > XTM_XDSL_MODE_MAX_THRESHOLD)
   {
      /* Tell the user what we're doing */
      XtmOsPrintf ("  Invalid XTM Mode Q threshold %d \n", ulThreshold) ;
      bxStatus = XTMSTS_PARAMETER_ERROR ;
   }

   return (bxStatus) ;

} /* validateThreshold */

/***************************************************************************
 * Function Name: ManageThreshold
 * Description  : Manage threshold/mode parameters that is applicable for tx
 *                queues (get/set operations)
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_PROCESSOR::ManageThreshold (PXTM_THRESHOLD_PARMS pThreshold)
{
   BCMXTM_STATUS bxStatus = XTMSTS_SUCCESS ;

   /* Are we setting ADSL threshold parameter? */
   if (pThreshold->sParams.adslParam == XTM_THRESHOLD_PARM_SET) {

      /* New ADSL threshold. */
      UINT32 ulThreshold = (UINT32) pThreshold->adslThreshold;
      bxStatus = validateThreshold (ulThreshold) ;

      if (bxStatus == XTMSTS_SUCCESS) {

         XtmOsPrintf ("  Setting ADSL threshold to %d \n", ulThreshold) ;
         m_ConnTable.setThreshold (XTM_LINK_MODE_ADSL, ulThreshold) ;
      }
   }

   /* Are we getting ADSL threshold parameter */
   if(pThreshold->sParams.adslParam == XTM_THRESHOLD_PARM_GET)
   {
      /* Tell the user the value of the parameter */
      XtmOsPrintf ("Existing ADSL threshold parameter is %lu.\n",
            m_ConnTable.getThreshold (XTM_LINK_MODE_ADSL)) ;
   }

   /* Are we setting VDSL threshold parameter? */
   if (pThreshold->sParams.vdslParam == XTM_THRESHOLD_PARM_SET) {

      /* New VDSL threshold. */
      UINT32 ulThreshold = (UINT32) pThreshold->vdslThreshold;
      bxStatus = validateThreshold (ulThreshold) ;

      if (bxStatus == XTMSTS_SUCCESS) {

         XtmOsPrintf ("  Setting VDSL threshold to %d \n", ulThreshold) ;
         m_ConnTable.setThreshold (XTM_LINK_MODE_VDSL, ulThreshold) ;
      }
   }

   /* Are we getting VDSL threshold parameter */
   if(pThreshold->sParams.vdslParam == XTM_THRESHOLD_PARM_GET)
   {
      /* Tell the user the value of the parameter */
      XtmOsPrintf ("Existing VDSL threshold parameter is %lu.\n",
            m_ConnTable.getThreshold (XTM_LINK_MODE_VDSL)) ;
   }

   /* Are we setting VDSL Rtx threshold parameter? */
   if (pThreshold->sParams.vdslRtxParam == XTM_THRESHOLD_PARM_SET) {

      /* New VDSL threshold. */
      UINT32 ulThreshold = (UINT32) pThreshold->vdslRtxThreshold;
      bxStatus = validateThreshold (ulThreshold) ;

      if (bxStatus == XTMSTS_SUCCESS) {

         XtmOsPrintf ("  Setting VDSL RTX threshold to %d \n", ulThreshold) ;
         m_ConnTable.setThreshold (XTM_LINK_MODE_VDSL_RTX, ulThreshold) ;
      }
   }

   /* Are we getting VDSL RTX threshold parameter */
   if(pThreshold->sParams.vdslRtxParam == XTM_THRESHOLD_PARM_GET)
   {
      /* Tell the user the value of the parameter */
      XtmOsPrintf ("Existing VDSL RTX threshold parameter is %lu.\n",
                    m_ConnTable.getThreshold (XTM_LINK_MODE_VDSL_RTX)) ;
   }

   /* Are we setting GFAST threshold parameter? */
   if (pThreshold->sParams.gfastParam == XTM_THRESHOLD_PARM_SET) {

      /* New GFAST threshold. */
      UINT32 ulThreshold = (UINT32) pThreshold->gfastThreshold;
      bxStatus = validateThreshold (ulThreshold) ;

      if (bxStatus == XTMSTS_SUCCESS) {

         XtmOsPrintf ("  Setting GFAST threshold to %d \n", ulThreshold) ;
         m_ConnTable.setThreshold (XTM_LINK_MODE_GFAST, ulThreshold) ;
      }
   }

   /* Are we getting GFAST threshold parameter */
   if(pThreshold->sParams.gfastParam == XTM_THRESHOLD_PARM_GET)
   {
      /* Tell the user the value of the parameter */
      XtmOsPrintf ("Existing GFAST threshold parameter is %lu.\n",
            m_ConnTable.getThreshold (XTM_LINK_MODE_GFAST)) ;
   }

   return (bxStatus) ;

} /* ManageThreshold */


/***************************************************************************
 * Function Name: GetTrafficDescrTable
 * Description  : Returns the Traffic Descriptor Table.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_PROCESSOR::GetTrafficDescrTable(PXTM_TRAFFIC_DESCR_PARM_ENTRY
    pTrafficDescrTable, UINT32 *pulTrafficDescrTableSize)
{
    BCMXTM_STATUS bxStatus = XTMSTS_PARAMETER_ERROR;

    if( *pulTrafficDescrTableSize >= m_ulNumTrafficDescrEntries )
    {
        if( m_pTrafficDescrEntries && m_ulNumTrafficDescrEntries )
        {
            memcpy(pTrafficDescrTable, m_pTrafficDescrEntries,
              m_ulNumTrafficDescrEntries*sizeof(XTM_TRAFFIC_DESCR_PARM_ENTRY));
            bxStatus = XTMSTS_SUCCESS;
        }
    }

    *pulTrafficDescrTableSize = m_ulNumTrafficDescrEntries;

    return( bxStatus );
} /* GetTrafficDescrTable */


/***************************************************************************
 * Function Name: SetTrafficDescrTable
 * Description  : Saves the supplied Traffic Descriptor Table.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_PROCESSOR::SetTrafficDescrTable(PXTM_TRAFFIC_DESCR_PARM_ENTRY
    pTrafficDescrTable, UINT32  ulTrafficDescrTableSize)
{
    BCMXTM_STATUS bxStatus;

    if( ulTrafficDescrTableSize )
    {
        /* Free an existing table if it exists. */
        if( m_pTrafficDescrEntries )
        {
            XtmOsFree( (char *) m_pTrafficDescrEntries );
            m_pTrafficDescrEntries = NULL;
            m_ulNumTrafficDescrEntries = 0;
        }

        UINT32 ulSize =
            ulTrafficDescrTableSize * sizeof(XTM_TRAFFIC_DESCR_PARM_ENTRY);

        /* Allocate memory for the new table. */
        m_pTrafficDescrEntries =
            (PXTM_TRAFFIC_DESCR_PARM_ENTRY) XtmOsAlloc(ulSize);

        /* Copy the table. */
        if( m_pTrafficDescrEntries )
        {
            m_ulNumTrafficDescrEntries = ulTrafficDescrTableSize;
            memcpy( m_pTrafficDescrEntries, pTrafficDescrTable, ulSize );

            /* Update connections with new values. */
            UINT32 i = 0;
            XTM_CONNECTION *pConn;
            while( (pConn = m_ConnTable.Enum( &i )) != NULL )
            {
                pConn->SetTdt( m_pTrafficDescrEntries,
                    m_ulNumTrafficDescrEntries );
            }

            bxStatus = XTMSTS_SUCCESS;
        }
        else
            bxStatus = XTMSTS_ALLOC_ERROR;
    }
    else
        bxStatus = XTMSTS_PARAMETER_ERROR;

    return( bxStatus );
} /* SetTrafficDescrTable */


/***************************************************************************
 * Function Name: GetInterfaceCfg
 * Description  : Returns the interface configuration record for the specified
 *                port id.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_PROCESSOR::GetInterfaceCfg( UINT32 ulPortId,
    PXTM_INTERFACE_CFG pInterfaceCfg )
{
    BCMXTM_STATUS bxStatus;
    //XtmOsPrintf("%s:Entry\n",__FUNCTION__);

    UINT32 ulPort = PORTID_TO_PORT(ulPortId);
    if( ulPort < MAX_INTERFACES )
        bxStatus = m_Interfaces[ulPort].GetCfg( pInterfaceCfg, &m_ConnTable );
    else
        bxStatus = XTMSTS_PARAMETER_ERROR;
    return( bxStatus ) ;
} /* GetInterfaceCfg */


/***************************************************************************
 * Function Name: SetInterfaceCfg
 * Description  : Sets the interface configuration record for the specified
 *                port.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_PROCESSOR::SetInterfaceCfg( UINT32 ulPortId,
    PXTM_INTERFACE_CFG pInterfaceCfg )
{
    BCMXTM_STATUS bxStatus;
    UINT32 ulPort = PORTID_TO_PORT(ulPortId);
    UINT32 ulBondingPort ;

    if( ulPort < MAX_INTERFACES ) {
       bxStatus = m_Interfaces[ulPort].SetCfg( pInterfaceCfg );
       ulBondingPort = m_Interfaces[ulPort].GetBondingPortNum () ;
       if (ulBondingPort != MAX_INTERFACES)
          bxStatus = m_Interfaces[ulBondingPort].SetCfg( pInterfaceCfg );
#if defined(XTM_PORT_SHAPING)
       if(bxStatus == XTMSTS_SUCCESS)
       {
          m_ulPortShapingConfig = pInterfaceCfg->ulPortShaping;
          if(((m_ulTrafficType == TRAFFIC_TYPE_PTM) || (m_ulTrafficType == TRAFFIC_TYPE_PTM_RAW) 
                || (m_ulTrafficType == TRAFFIC_TYPE_PTM_BONDED)) &&
                (m_ulXTMLinkMode != XTM_LINK_MODE_GFAST))
          {
             UpdateSitSlr(ulPort) ;
          }
       }
#endif
    }
    else
        bxStatus = XTMSTS_PARAMETER_ERROR;

    return( bxStatus );
} /* SetInterfaceCfg */


/* The following routine updates (add/removal) the port mask for the connection
 * for bonding.
 * For ex., if a given port is part of the port mask, its bonding member will
 * also be updated to be part of the same port mask.
 * This happens transparent to the user layer.
 * It also updates (add/removal) the traffic type for the connection
 * for bonding.
 * For ex., if a given connection is PTM type and bonding is enabled, then
 * the traffic type is changed from/to PTM_BONDED type.
 * This happens transparent to the user layer.
 */
void XTM_PROCESSOR::UpdateConnAddrForBonding ( PXTM_ADDR pConnAddr, UINT32 operation )
{
   UINT32 ulPortId, ulPort, ulBondingPort ;
   UINT32 *pulPortMask = &pConnAddr->u.Conn.ulPortMask ;
   UINT32 *pulTrafficType = &pConnAddr->ulTrafficType ;

   if (( m_InitParms.bondConfig.sConfig.ptmBond == BC_PTM_BONDING_ENABLE) ||
       ( m_InitParms.bondConfig.sConfig.atmBond == BC_ATM_BONDING_ENABLE)) {
      for (ulPortId = PORT_PHY0_PATH0 ; ulPortId <= PORT_PHY1_PATH1 ; ulPortId <<= 0x1) {
         if (*pulPortMask & ulPortId) {
            ulPort = PORTID_TO_PORT (ulPortId) ;
            if ( ulPort >= MAX_INTERFACES )
               continue;
            ulBondingPort = m_Interfaces[ulPort].GetBondingPortNum () ;
            if (ulBondingPort != MAX_INTERFACES) {
               if (operation == XTM_ADD)
                  *pulPortMask |= PORT_TO_PORTID (ulBondingPort) ;
               else {
                  /*Check if the line we are about to remove from the mask is the active line
                  if so don't remove that mask make sure we are removing the line that is not
                  active */
                  if (m_ulTrafficMonitorPort != ulBondingPort) 
                     *pulPortMask &= ~PORT_TO_PORTID (ulBondingPort) ;
               }
            }
         }
      }

      if( (*pulTrafficType & TRAFFIC_TYPE_ATM_MASK) != TRAFFIC_TYPE_ATM ) {
         if (operation == XTM_ADD)
            *pulTrafficType = TRAFFIC_TYPE_PTM_BONDED ;
         else
            *pulTrafficType = TRAFFIC_TYPE_PTM ;
      }
      else {
         if (operation == XTM_ADD)
            *pulTrafficType = TRAFFIC_TYPE_ATM_BONDED ;
         else
            *pulTrafficType = TRAFFIC_TYPE_ATM ;
      }
   }
}

/* The following routine updates (add/removal) the bonding port ID for the
 * transmit Q Parameters section of the connection configuration.
 * For a given portId in the transmit Q Params information, the corresponding
 * bonding member also will be updated.
 * This happens transparent to the user layer.
 */
void XTM_PROCESSOR::UpdateTransmitQParamsForBonding ( XTM_TRANSMIT_QUEUE_PARMS *pTxQParms,
                                           UINT32 ulTxQParmsSize, UINT32 operation)
{
   UINT32 ulPort, ulBondingPort ;
   UINT32 txQIndex ;
   PXTM_INTERFACE_LINK_INFO pLiPort, pLiBondingPort ;

   for (txQIndex = 0 ; txQIndex < ulTxQParmsSize; txQIndex++) {

      ulPort = PORTID_TO_PORT (pTxQParms [txQIndex].ulPortId ) ;
      if ( ulPort >= MAX_INTERFACES )
         continue;
      ulBondingPort = m_Interfaces[ulPort].GetBondingPortNum () ;

      pLiPort =  m_Interfaces[ulPort].GetLinkInfo() ;

      if (ulBondingPort != MAX_INTERFACES) {

         pLiBondingPort =  m_Interfaces[ulBondingPort].GetLinkInfo();

         if ((pLiPort->ulLinkTrafficType == TRAFFIC_TYPE_PTM_BONDED) ||
             (pLiBondingPort->ulLinkTrafficType == TRAFFIC_TYPE_PTM_BONDED) ||
             (pLiPort->ulLinkTrafficType == TRAFFIC_TYPE_ATM_BONDED) ||
             (pLiBondingPort->ulLinkTrafficType == TRAFFIC_TYPE_ATM_BONDED)) {

            if (operation == XTM_ADD)
               pTxQParms [txQIndex].ulBondingPortId = PORT_TO_PORTID (ulBondingPort) ;
            else
               pTxQParms [txQIndex].ulBondingPortId = PORT_PHY_INVALID ;
         }
         else
            pTxQParms [txQIndex].ulBondingPortId = PORT_PHY_INVALID ;
      }
      else
         pTxQParms [txQIndex].ulBondingPortId = PORT_PHY_INVALID ;
   } /* txQIndex */
}

void XTM_PROCESSOR::PollLines ()
{
   int  ulInterfaceId ;
   PXTM_INTERFACE_LINK_INFO pLi ;
   UINT32 ulUsStatus, ulDsStatus ;

   for (ulInterfaceId = 0 ; ulInterfaceId < MAX_BONDED_LINES; ulInterfaceId++) {

      pLi = m_Interfaces[ulInterfaceId].GetLinkInfo() ;

      if (pLi->ulLinkState == LINK_UP) {

         if (m_Interfaces[ulInterfaceId].GetLinkErrorStatus (&ulUsStatus, &ulDsStatus) != XTMSTS_SUCCESS) {

            if (m_Interfaces[ulInterfaceId].GetPortDataStatus () == DATA_STATUS_ENABLED) {
               XTMRT_TOGGLE_PORT_DATA_STATUS_CHANGE Tpdsc;
               memset (&Tpdsc, 0, sizeof (Tpdsc)) ;
               Tpdsc.ulPortId         = ulInterfaceId ;
               Tpdsc.ulPortDataUsStatus = (ulUsStatus==XTMSTS_SUCCESS) ? XTMRT_CMD_PORT_DATA_STATUS_ENABLED :
                                                            XTMRT_CMD_PORT_DATA_STATUS_DISABLED ;
               Tpdsc.ulPortDataDsStatus = (ulDsStatus==XTMSTS_SUCCESS) ? XTMRT_CMD_PORT_DATA_STATUS_ENABLED :
                                                            XTMRT_CMD_PORT_DATA_STATUS_DISABLED ;

               (*m_pfnXtmrtReq) (NULL, XTMRT_CMD_TOGGLE_PORT_DATA_STATUS_CHANGE, &Tpdsc) ;
               m_Interfaces[ulInterfaceId].SetPortDataStatus (DATA_STATUS_DISABLED, XTM_NO_FLUSH) ;
            }
         }
         else {

            if (m_Interfaces[ulInterfaceId].GetPortDataStatus () == DATA_STATUS_DISABLED) {
               XTMRT_TOGGLE_PORT_DATA_STATUS_CHANGE Tpdsc ;
               memset (&Tpdsc, 0, sizeof (Tpdsc)) ;
               Tpdsc.ulPortId         = ulInterfaceId ;
               Tpdsc.ulPortDataUsStatus = XTMRT_CMD_PORT_DATA_STATUS_ENABLED ;
          Tpdsc.ulPortDataDsStatus = XTMRT_CMD_PORT_DATA_STATUS_ENABLED ;

               if(m_pfnXtmrtReq != NULL) {
                   (*m_pfnXtmrtReq) (NULL, XTMRT_CMD_TOGGLE_PORT_DATA_STATUS_CHANGE, &Tpdsc) ;
               }
               m_Interfaces[ulInterfaceId].SetPortDataStatus (DATA_STATUS_ENABLED, XTM_UNDO_FLUSH) ;
            }
         }
      } /* if (pLi->ulLinkState == LINK_UP) */
   } /* for (ulInterfaceId) */
}


void XTM_PROCESSOR::XtmMonitorTimerCb (void *ulParm)
{
   int i, j ;
   int ret ;
   UINT32 ulPort ;
   XTM_PROCESSOR *pXtmProcessor = (XTM_PROCESSOR *) pBondTmrParm ;
   UINT32 rxBondErrStatus ;

   //XtmOsPrintf ("XTM: pBondTmrParm = %lx \n", pXtmProcessor) ;

   if (pXtmProcessor->m_ulPrevRelatedUpEvtValid == 0) {
      for (ulPort=0; ulPort<MAX_BONDED_LINES; ulPort++) {
         if (pXtmProcessor->m_ulPendingEvtValid[ulPort]) {
            MapXtmOsPrintf ("bcmxtmcfg:Pend Evt Svc Port-%d, PortId-%d, LI-%d\n",
                  (int) ulPort, (int) pXtmProcessor->m_ulPendingEvtPortId[ulPort],
                  (int) pXtmProcessor->m_PendingEvtLinkInfo[ulPort].ulLinkState) ;
            pXtmProcessor->m_ulPendingEvtValid[ulPort] = 0 ;
            pXtmProcessor->SetInterfaceLinkInfo (pXtmProcessor->m_ulPendingEvtPortId[ulPort],
                  &pXtmProcessor->m_PendingEvtLinkInfo[ulPort], 1) ;
         }
      }
   }
   else
      pXtmProcessor->m_ulPrevRelatedUpEvtValid -= XTM_BOND_DSL_MONITOR_TIMEOUT_MS ;

   // Invoke OAM Handler timer
   XTM_OAM_HANDLER::TimerUpdate();

   // Invoke XtmAutosesnse timer
   pXtmProcessor->m_XtmAutoSense.TimerUpdate();

   if ((pXtmProcessor->m_ulTrafficMonitorPort != MAX_INTERFACES) && ((pXtmProcessor->m_ulTrafficType == TRAFFIC_TYPE_NOT_CONNECTED) || (pXtmProcessor->m_ulTrafficType & TRAFFIC_TYPE_ATM_MASK) )) {
      // Check if Autosense detected a change in PHY type
      if(pXtmProcessor->m_XtmAutoSense.PhyChangePending() && pXtmProcessor->m_XtmAutoSense.PhyStatus() != PHYSTATUS_UNKNOWN) {

         //ATM mode
         UINT32 ulTrafficType;         // For setting the desired traffic type
         UINT32 ulConnAddrAction;      // Action to pass to UpdateConnAddrs()
         UINT32 mode;

         // Set up variables so we can change traffic type, order the correct PHY to be loaded, etc.
         if(pXtmProcessor->m_XtmAutoSense.PhyStatus() == PHYSTATUS_ATM_SINGLELINE) {
            XtmOsPrintf ("\nbcmxtmcfg: Changing over to Single Line PHY\n") ;
            // We want Single Line PHY.
            ulTrafficType = TRAFFIC_TYPE_ATM;               // Set traffic type
            dslSysCtlCommand [2] = XTM_USE_NON_BONDED_PHY ; // Set command to send to PHY
            ulConnAddrAction = XTM_REMOVE;                  // Remove connections
         }
         else {
            // We want Bonding PHY.
            XtmOsPrintf ("\nbcmxtmcfg: Changing over to Bonded PHY\n") ;
            ulTrafficType = TRAFFIC_TYPE_ATM_BONDED;        // Set traffic type
            dslSysCtlCommand [2] = XTM_USE_BONDED_PHY ;     // Set command to send to PHY
            ulConnAddrAction = XTM_ADD;                    // Add connections
         }

         // Prepare for switching out to a new PHY
         XtmOsSendSysEvent (XTM_EVT_TRAFFIC_TYPE_MISMATCH) ;
         XtmOsPrintf ("\nbcmxtmcfg: Traffic incompatibility between CO & CPE. Reconfiguring the system. \n") ;

         // Update connection addresses
         pXtmProcessor->UpdateConnAddrs (ulConnAddrAction) ;

         // Set the traffic type
         if (pXtmProcessor->ReInitialize  (ulTrafficType) != XTMSTS_SUCCESS)
            XtmOsPrintf ("\nbcmxtmcfg: Fatal - ReInitialize XTM part due to Traffic type change has failed (traffic type=%lu) \n", ulTrafficType) ;
         pXtmProcessor->ReconfigureSAR  (pXtmProcessor->m_ulTrafficMonitorPort, ulTrafficType) ;
         pXtmProcessor->m_Interfaces[pXtmProcessor->m_ulTrafficMonitorPort].UpdateLinkInfo (ulTrafficType) ;

         // Check and update connection RX CAM and Tx table programming
         if ((pXtmProcessor->m_ulTrafficMonitorPort == PHY_PORTID_1) && (ulTrafficType == TRAFFIC_TYPE_ATM)) {
            UINT32 idx;
            XTM_CONNECTION *pConn;
            XTM_INTERFACE_LINK_INFO Li ;
            XTM_ADDR Addr;
            PXTM_INTERFACE_LINK_INFO pLi = pXtmProcessor->m_Interfaces[pXtmProcessor->m_ulTrafficMonitorPort].GetLinkInfo();
            idx = 0;
            memcpy(&Li,pLi,sizeof(XTM_INTERFACE_LINK_INFO));
            while( (pConn = pXtmProcessor->m_ConnTable.Enum( &idx )) != NULL ) {
               UINT32 ulPortMask = 0;           
               pConn->GetAddr( &Addr );
               ulPortMask = Addr.u.Conn.ulPortMask ;
               /* Now that the traffic sensing is completed, we need to
               ** reconfigure the connection and XTM/SAR states accordingly.
               **/
               XtmOsPrintf("(%s)Updating Connection Link Information for m_ulTrafficMonitorPort[%u]\n",__FUNCTION__,pXtmProcessor->m_ulTrafficMonitorPort);
               if (pConn->SetLinkInfo( (ulPortMask & PORT_TO_PORTID(pXtmProcessor->m_ulTrafficMonitorPort)),
                        &Li, MAX_INTERFACES, NULL, pXtmProcessor->m_ulXTMLinkMode, pXtmProcessor->m_ulXTMNitroMode) != XTMSTS_SUCCESS) {
                  XtmOsPrintf("(%s): Failed to reprogram the connection for this Non-Bonded mode on Port1\n",__FUNCTION__);
               }
            }
         }

         // Send PHY command
         if (g_pfnAdslSetObjValue != NULL) {
            ret = g_pfnAdslSetObjValue (pXtmProcessor->m_ulTrafficMonitorPort, dslSysCtlCommand, sizeof (dslSysCtlCommand),
                  NULL, 0) ;
            if (ret != 0) {
               XtmOsPrintf ("bcmxtmcfg: dslSysCtl command for type of the PHY Failed. \n") ;
            }
         }

         // Tell XTMRT datapath what SID setting to use
         if (dslSysCtlCommand [2] == XTM_USE_BONDED_PHY){
            mode = m_pAsmHandler->getSIDMode();
            (*pXtmProcessor->m_pfnXtmrtReq) (NULL, XTMRT_CMD_SET_ATMBOND_SID_MODE,
                  &mode) ; // Have to use pointer to get to ASM handler because this is a static method
         }
         else {
            mode = ATMBOND_ASM_MESSAGE_TYPE_NOSID;
            (*pXtmProcessor->m_pfnXtmrtReq) (NULL, XTMRT_CMD_SET_ATMBOND_SID_MODE,
                  &mode) ;
         }
      }
   } /* if (pXtmProcessor->m_ulTrafficMonitorPort) */


   if (pXtmProcessor->m_ulRxPafLockupMonitorEvtValid > 0) {
      /* Check to see if RxPaf Locked up */
      pXtmProcessor->m_ulRxPafLockupMonitorEvtValid -= XTM_BOND_DSL_MONITOR_TIMEOUT_MS ;

      if (pXtmProcessor->m_ulRxPafLockupMonitorEvtValid == 0) {

         /* Check for the RxBondErrorStatus  */
         rxBondErrStatus = XP_REGS->ulRxBondErrorStatus ;
         if (((rxBondErrStatus & RB_ERR_STATUS_PAF_BAD_FRAG) != 0) &&
               ((pXtmProcessor->m_ulRxBondPrevErrStatus & RB_ERR_STATUS_PAF_BAD_FRAG) != 0)) {

            /* Check for the SIDs in the stuck stage */
            if ((XP_REGS->ulRxBondExpectedSid0 == pXtmProcessor->m_ulRxBondPrevExpSid01) &&
                  (XP_REGS->ulRxBondExpectedSid1 == pXtmProcessor->m_ulRxBondPrevExpSid23)) {
               /* RxPaf Lockup. Channel Flush. Wait for recovery */
               XtmOsPrintf ("Flushing RxPaf Channels.... \n") ;
               if (pXtmProcessor->m_ulTrafficType == TRAFFIC_TYPE_PTM_BONDED) {
                  for( i = PHY_PORTID_0; i < MAX_INTERFACES; i++ ) {
                     pXtmProcessor->m_Interfaces[i].SetPortDataStatus (DATA_STATUS_DISABLED, XTM_FLUSH) ;
                     for (j = 0; j < 5; j++)
                        XtmOsPrintf ("!");
                     pXtmProcessor->m_Interfaces[i].SetPortDataStatus (DATA_STATUS_ENABLED, XTM_UNDO_FLUSH) ;
                  }
               }
               else {
                  UINT32 ulCurrChanFlush = XP_REGS->ulRxPafWriteChanFlush ;
                  XP_REGS->ulRxPafWriteChanFlush = 0xFF ; /* Reset all 8 WrChannels */
                  for (j = 0; j < 5; j++)
                     XtmOsPrintf (".");
                  XP_REGS->ulRxPafWriteChanFlush = ulCurrChanFlush ;
               }
               XtmOsPrintf ("\n");
            }

            pXtmProcessor->m_ulRxBondPrevErrStatus = XP_REGS->ulRxBondErrorStatus ;
         }
         else {
            if (rxBondErrStatus != 0)
               pXtmProcessor->m_ulRxBondPrevErrStatus = rxBondErrStatus ;
         }

         pXtmProcessor->m_ulRxBondPrevExpSid01  = XP_REGS->ulRxBondExpectedSid0 ;
         pXtmProcessor->m_ulRxBondPrevExpSid23  = XP_REGS->ulRxBondExpectedSid1 ;

         pXtmProcessor->m_ulRxPafLockupMonitorEvtValid = XTM_BOND_RX_PAF_LOCKUP_MONITOR_TIMEOUT_MS ;

      } /* if RxPaf Lockup monitor timer has expired */

   } /* if RxPaf Lockup Monitor timer is valid */

   if ((pXtmProcessor->m_InitParms.bondConfig.sConfig.ptmBond == BC_PTM_BONDING_ENABLE) ||
         (pXtmProcessor->m_InitParms.bondConfig.sConfig.atmBond == BC_ATM_BONDING_ENABLE)) {
      pXtmProcessor->PollLines () ;
   }

   if (XtmOsStartTimer ((void *) pXtmProcessor->XtmMonitorTimerCb, 0,
            XTM_BOND_DSL_MONITOR_TIMEOUT_MS) != 0) {
      XtmOsPrintf ("\n\n Alarm bcmxtmcfg ...... (Re)Start BondDslMonitorTimer Failed \n\n") ;
   }
}

/***************************************************************************
 * Function Name: GetConnCfg
 * Description  : Returns the connection configuration record for the
 *                specified connection address.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_PROCESSOR::GetConnCfg( PXTM_ADDR pConnAddr,
    PXTM_CONN_CFG pConnCfg )
{
    BCMXTM_STATUS bxStatus;
    XTM_CONNECTION *pConn ;
    
    UpdateConnAddrForBonding ( pConnAddr, XTM_ADD ) ;

    pConn = m_ConnTable.Get( pConnAddr );

    if( pConn )
    {
        UINT32 ulPortMask = pConnAddr->u.Conn.ulPortMask ;

        pConn->GetCfg( pConnCfg );

        /* The operational status of the connection is determined by checking
         * the link status.  A connection can use more than one link.  In order
         * for the operational status of the connection to be "up", atleast one
         * of the links it uses must be "up".
         * If all the links are "down", the connection will be operationally
         * down.
         */
        pConnCfg->ulOperStatus = OPRSTS_DOWN;
        for( UINT32 i = PHY_PORTID_0; i < MAX_INTERFACES; i++ )
        {
            if( (ulPortMask & PORT_TO_PORTID(i)) == PORT_TO_PORTID(i) )
            {
                PXTM_INTERFACE_LINK_INFO pLi = m_Interfaces[i].GetLinkInfo();
                if( pLi->ulLinkState == LINK_UP &&
                    ((pLi->ulLinkTrafficType == pConnAddr->ulTrafficType) ||
                     (pLi->ulLinkTrafficType == TRAFFIC_TYPE_PTM_RAW)))
                {
                    /* At least one link is UP. */
                    pConnCfg->ulOperStatus = OPRSTS_UP;
                    break;
                }
            }
        }

        bxStatus = XTMSTS_SUCCESS;
    }
    else
        bxStatus = XTMSTS_NOT_FOUND;

    UpdateConnAddrForBonding ( pConnAddr, XTM_REMOVE ) ;

    return( bxStatus );
} /* GetConnCfg */


/***************************************************************************
 * Function Name: SetConnCfg
 * Description  : Sets the connection configuration record for the specified
 *                connection address.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_PROCESSOR::SetConnCfg( PXTM_ADDR pConnAddr,
    PXTM_CONN_CFG pConnCfg )
{
    BCMXTM_STATUS bxStatus = XTMSTS_SUCCESS;
    XTM_CONNECTION *pConn ;
    UINT32 tqindex;
    UINT32 logicalPort;
    UINT32 physicalPort;
    UINT32 connIndex = 0;
    BOOL   bShapers = FALSE ;

    logicalPort = PORTMAP_TO_LOGICALPORT(pConnAddr->u.Conn.ulPortMask);
    //XtmOsPrintf("(%s)Logical portmask[%u] Logical PortId[%u]\n",__FUNCTION__,pConnAddr->u.Conn.ulPortMask,logicalPort);

    if ((pConnCfg) && (m_LogPhyPortmap[logicalPort].PhysicalPortId == PORT_PHY_INVALID)) {
       //Link-Up case & Connection creation
       //Here incoming portmask is based on logical port map. Convert this to physical port map now. This needs to be optimized 
       // when both PHY0 & PHY1 are enabled from the userspace.
       //XtmOsPrintf("(%s)mulTrafficMonitorPort[%u]\n",__FUNCTION__,m_ulTrafficMonitorPort);
       physicalPort = m_ulTrafficMonitorPort;
       //pConnAddr->u.Vcc.ulPortMask = PORT_TO_PORTID(physicalPort);
       pConnAddr->u.Conn.ulPortMask = PORT_TO_PORTID(physicalPort);
    }
    else {
       //Link-Down case or Connection Configuration update.
       physicalPort = m_LogPhyPortmap[logicalPort].PhysicalPortId;
       pConnAddr->u.Conn.ulPortMask = PORT_TO_PORTID(m_LogPhyPortmap[logicalPort].PhysicalPortId);
    } //if(pConnCfg)

    /* Make a port mask taking into account of bonding as well */
    UpdateConnAddrForBonding ( pConnAddr, XTM_ADD ) ;

    /* Make a port mask for xmit params of the conncfg taking into account of bonding as well */
    if (pConnCfg) {
       //This should be a for loop to update all the incoming transmit queue params.
       for (tqindex=0;tqindex<pConnCfg->ulTransmitQParmsSize;tqindex++) {

          pConnCfg->TransmitQParms[tqindex].ulPortId = PORT_TO_PORTID(physicalPort);

          /* Also check to see if QoS params are configured for shaping */
          if ((pConnCfg->TransmitQParms[tqindex].ulMinBitRate != 0) ||
              (pConnCfg->TransmitQParms[tqindex].ulShapingRate != 0)) {
             bShapers = TRUE ;
          }
       }//end for(tq...

       UpdateTransmitQParamsForBonding ( pConnCfg->TransmitQParms,
                                         pConnCfg->ulTransmitQParmsSize, XTM_ADD) ;

      m_ulQShapingConfig = (bShapers == TRUE) ? PORT_Q_SHAPING_ON : PORT_Q_SHAPING_OFF ;
    } //if(pConnCfg)

#if defined(XTM_PORT_SHAPING)
    if(((m_ulTrafficType == TRAFFIC_TYPE_PTM) || (m_ulTrafficType == TRAFFIC_TYPE_PTM_RAW) 
             || (m_ulTrafficType == TRAFFIC_TYPE_PTM_BONDED)) &&
          (m_ulXTMLinkMode != XTM_LINK_MODE_GFAST))
    {
       UpdateSitSlr(physicalPort) ;
    }
#endif

    pConn = m_ConnTable.Get( pConnAddr );

    if( pConn == NULL )
    {
        /* The connection does not exist. Create it and add it to the table. */
        pConn = (XTM_CONNECTION *)XtmOsAlloc(sizeof(XTM_CONNECTION));
        if( pConn != NULL )
        {
            XtmOsPrintf("(%s)Created new connection\n",__FUNCTION__);
            pConn->Initialize(m_pfnXtmrtReq, &m_ConnTable, (void *)m_ulConnSem, m_ulRxVpiVciCamShadow, m_InitParms.bondConfig);
            m_LogPhyPortmap[logicalPort].PhysicalPortId = physicalPort;
            pConn->SetTdt(m_pTrafficDescrEntries, m_ulNumTrafficDescrEntries);
            pConn->SetAddr( pConnAddr );
            if( (bxStatus = m_ConnTable.Add( pConn )) != XTMSTS_SUCCESS )
            {
                XtmOsPrintf("(%s)Failed to add this connection to the table\n",__FUNCTION__);
                // delete pConn;
                pConn->Dxtor();
                XtmOsFree((char *)pConn);
                m_LogPhyPortmap[logicalPort].PhysicalPortId = PORT_PHY_INVALID;
                pConn = NULL;
            }
        }
        else
            bxStatus = XTMSTS_ALLOC_ERROR;
    }

    if( pConn )
    {
        bxStatus = pConn->SetCfg( pConnCfg, m_ulXTMLinkMode, m_ulXTMNitroMode );
        if( pConnCfg == NULL )
        {
            /* A NULL configuration pointer means to remove the connection.
             * However, an active connection (and its configuration will stay
             * active until its network device instance is deleted.
             */
            if( !pConn->Connected() )
            {
                m_ConnTable.Remove( pConn );
                // delete pConn;
                pConn->Dxtor();
                XtmOsFree((char *)pConn);
                pConn = NULL;
                if(m_ConnTable.Enum(&connIndex) == NULL)
                   m_LogPhyPortmap[logicalPort].PhysicalPortId = PORT_PHY_INVALID;
            }
        }
    }

    if (pConnCfg)
      UpdateTransmitQParamsForBonding ( pConnCfg->TransmitQParms, 
                                        pConnCfg->ulTransmitQParmsSize, XTM_REMOVE) ;

    UpdateConnAddrForBonding ( pConnAddr, XTM_REMOVE ) ;
    return( bxStatus );
} /* SetConnCfg */


/***************************************************************************
 * Function Name: GetConnAddrs
 * Description  : Returns an array of all configured connection addresses.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_PROCESSOR::GetConnAddrs( PXTM_ADDR pConnAddrs,
    UINT32 *pulNumConns )
{
    BCMXTM_STATUS bxStatus = XTMSTS_SUCCESS;
    UINT32 ulInNum = *pulNumConns;
    UINT32 ulOutNum = 0;
    UINT32 i = 0;
    XTM_CONNECTION *pConn;
    XTM_ADDR Addr;

    while( (pConn = m_ConnTable.Enum( &i )) != NULL )
    {
        if( ulOutNum < ulInNum )
        {
            pConn->GetAddr( &Addr );
            UpdateConnAddrForBonding ( &Addr, XTM_REMOVE ) ;
            memcpy(&pConnAddrs[ulOutNum++], &Addr, sizeof(Addr));
        }
        else
        {
            bxStatus = XTMSTS_PARAMETER_ERROR;
            ulOutNum++;
        }
    }

    *pulNumConns = ulOutNum;
    return( bxStatus );
} /* GetConnAddrs */


/***************************************************************************
 * Function Name: UpdateConnAddrs
 * Description  : Updates an array of all configured connection addresses.
 *                Typically called, when there is a transition between
 *                bonded and non-bonded mode.
 * Parameters: ulConnAddrAction - Pass XTM_REMOVE to remove conenctions 
 *                (for bonded to nonbonded case) or  XTM_ADD (for
 *                nonbonded to bonded case).
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
void XTM_PROCESSOR::UpdateConnAddrs( UINT32 ulConnAddrAction )
{
    UINT32 i = 0;
    XTM_CONNECTION *pConn;
    XTM_ADDR Addr;

    while( (pConn = m_ConnTable.Enum( &i )) != NULL )
    {
       pConn->GetAddr( &Addr );
       UpdateConnAddrForBonding ( &Addr, ulConnAddrAction ) ;
       pConn->SetAddr( &Addr ) ;
    }

} /* UpdateConnAddrs */

/***************************************************************************
 * Function Name: GetInterfaceStatistics
 * Description  : Returns the interface statistics for the specified port.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_PROCESSOR::GetInterfaceStatistics( UINT32 ulPortId,
    PXTM_INTERFACE_STATS pIntfStats, UINT32 ulReset )
{
    BCMXTM_STATUS bxStatus;
    UINT32 ulPort = PORTID_TO_PORT(ulPortId);
    UINT32 ulBondingPort ;

    memset (pIntfStats, 0, sizeof (XTM_INTERFACE_STATS)) ;

    if( ulPort < MAX_INTERFACES ) {
        bxStatus = m_Interfaces[ulPort].GetStats( pIntfStats, ulReset );
       ulBondingPort = m_Interfaces[ulPort].GetBondingPortNum () ;
       if (ulBondingPort != MAX_INTERFACES)
          bxStatus = m_Interfaces[ulBondingPort].GetStats( pIntfStats, ulReset );
    }
    else
        bxStatus = XTMSTS_PARAMETER_ERROR;

    return( bxStatus );
} /* GetInterfaceStatistics */


/***************************************************************************
 * Function Name: GetErrorStatistics
 * Description  : Returns the error statistics for the device.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_PROCESSOR::GetErrorStatistics( PXTM_ERROR_STATS pErrStats )
{

    BCMXTM_STATUS bxStatus;
    UINT32 OverrunCounter;
    UINT32 UnderrunCounter;
    UINT32 ErrorPacketCounter;
    UINT32 ErrorFragmentCounter;
    UINT32 delta;

    OverrunCounter       = (XP_REGS->ulRxPktBufQueueFull);
    UnderrunCounter      = (XP_REGS->ulRxPktBufNoBuf);
    ErrorPacketCounter   = (XP_REGS->ulRxPktBufErrorPkt);
    ErrorFragmentCounter = (XP_REGS->ulRxPktBufErrorFrag);
    delta = 0;

    /* Do we have a good pointer? */
    if(pErrStats != NULL)
    {
        /* Zero out return structure */
        memset (pErrStats, 0, sizeof (XTM_ERROR_STATS)) ;

        /* Sum up drop fragment counts */
        for(int i=0; i<XP_MAX_RXPAF_WR_CHANNELS; i++) {
            pErrStats->ulPafLostFragments += XP_REGS->ulRxBondDroppedFragCount[i];
            pErrStats->ulPafLostFragments += XP_REGS->ulRxPafDroppedFragCount[i];
            pErrStats->ulPafErrs += XP_REGS->ulRxPafDroppedPktCount[i];
        }

        /* Get queue full (aka overrrun) errors */
        if(OverrunCounter >= m_ulRxBufferPrevOverrunCounter) {
           delta = OverrunCounter - m_ulRxBufferPrevOverrunCounter;
        }
        else {
           delta = (ULONG_MAX - m_ulRxBufferPrevOverrunCounter);
           delta += OverrunCounter;
        }
        pErrStats->ulOverflowErrorsRx = delta;
        m_ulRxBufferPrevOverrunCounter = OverrunCounter;

        /* Sum errors to get frame drop count */
        for(int i=0; i<XP_MAX_PORTS; i++) {
            pErrStats->ulFramesDropped += 
                     ((XP_REGS->ulRxPortErrorPktCellCnt[i] & ERROR_RX_PKT_COUNT_MASK)
                          >> ERROR_RX_PKT_COUNT_SHIFT) ;
        }

        /* Add all components of total PAF errors */
        if(ErrorPacketCounter >= m_ulRxBufferPrevErrorPacketCounter) {
           delta = ErrorPacketCounter - m_ulRxBufferPrevErrorPacketCounter;
        }
        else {
           delta = (ULONG_MAX - m_ulRxBufferPrevErrorPacketCounter);
           delta += ErrorPacketCounter;
        }
        pErrStats->ulPafErrs += delta;
        m_ulRxBufferPrevErrorPacketCounter = ErrorPacketCounter;

        if(ErrorFragmentCounter >= m_ulRxBufferPrevErrorFragmentCounter) {
           delta = ErrorFragmentCounter - m_ulRxBufferPrevErrorFragmentCounter;
        }
        else {
           delta = (ULONG_MAX - m_ulRxBufferPrevErrorFragmentCounter);
           delta += ErrorFragmentCounter;
        }
        pErrStats->ulPafLostFragments += delta;
        m_ulRxBufferPrevErrorFragmentCounter = ErrorFragmentCounter;
        m_ulRxBufferPrevUnderrunCounter = UnderrunCounter;

        /* Flag success */
        bxStatus = XTMSTS_SUCCESS;
    }
    else
    {
        /* Flag bad parameter */
        bxStatus = XTMSTS_PARAMETER_ERROR;
    }

    return( bxStatus );
} /* GetErrorStatistics */



/***************************************************************************
 * Function Name: UpdateLinkMode
 * Description  : Read the DSL Mib Statuses and determine the actual XTM link
 *                mode.
 * Returns      : void
 ***************************************************************************/
void XTM_PROCESSOR::UpdateLinkMode ( UINT32 ulDSLGinpMode,  UINT32 ulDSLMode )
{
   if (ulDSLMode < kVdslModVdsl2) {
      m_ulXTMLinkMode = XTM_LINK_MODE_ADSL ;
      XtmOsPrintf ("bcmxtmcfg: XTM Link Mode is ADSL. \n") ;
   }
   else if (ulDSLMode == kXdslModGfast) {
      m_ulXTMLinkMode = XTM_LINK_MODE_GFAST ;
      XtmOsPrintf ("bcmxtmcfg: XTM Link Mode is G.Fast \n") ;
   }
   else {
      /* It should be VDSL based link */
      if (ulDSLGinpMode) {
         m_ulXTMLinkMode =  XTM_LINK_MODE_VDSL_RTX ;
         XtmOsPrintf ("bcmxtmcfg: XTM Link Mode is VDSL G.inpRtx \n") ;
      }
      else {
         m_ulXTMLinkMode = XTM_LINK_MODE_VDSL ;
         XtmOsPrintf ("bcmxtmcfg: XTM Link Mode is VDSL \n") ;
      }
   }
} /* UpdateLinkMode */


/***************************************************************************
 * Function Name: UpdateLinkStatus
 * Description  : Read the link status to update certain datapath parameters for xtm processor.
 * Returns      : void
 ***************************************************************************/
void XTM_PROCESSOR::UpdateLinkStatus ( UINT32 ulPhysPort,  PXTM_INTERFACE_LINK_INFO pLinkInfo )
{
   int ret;
   long size = sizeof (PhyMibInfo) ;
   UINT32 ulDSLGinpMode, ulDSLMode ;

   if ( (pLinkInfo->ulLinkState == LINK_UP) && (g_pfnAdslGetObjValue != NULL) ) {

      size = sizeof (PhyMibInfo) ;
      XtmOsPrintf ("bcmxtmcfg: ULS Port %d \n", ulPhysPort) ;
      ret  = g_pfnAdslGetObjValue (ulPhysPort, NULL, 0, (char *) &PhyMibInfo, &size) ;
      if (ret != 0)
         XtmOsPrintf ("bcmxtmcfg: SILI:Error g_pfnAdslGetObjValue port - %d return value - %d \n", ulPhysPort, ret) ;
      else {
	 /* Due to the restriction we have had in the past (not anymore with Dynamic port mapping), Phy always indicate status on Port 0 when in non-bonded mode, even if the
	  * actual port connected could be Port 1 physically.
	  * with the advent of Dynamic port mapping, we can work on any physical port with Log-Phy port mapping.
	  * Once DSL indicates the actual port, this check/reassignment can be removed 
	  */
         if (PhyMibInfo.adslTrainingState != kAdslTrainingConnected) {
            ulPhysPort = (ulPhysPort == 0) ? 1 : 0 ;
            ret  = g_pfnAdslGetObjValue (ulPhysPort, NULL, 0, (char *) &PhyMibInfo, &size) ;
            if (ret != 0) {
               XtmOsPrintf ("bcmxtmcfg: SILI:Error g_pfnAdslGetObjValue port - %d return value - %d \n", ulPhysPort, ret) ;
               return ;
            }
         }

         ulDSLGinpMode = (PhyMibInfo.xdslStat[PORTID_TO_PORT(PORT_PHY0_FAST)].ginpStat.status & kXdslGinpDsEnabled) ? 1 : 0 ; /* Latency 0 */
         ulDSLMode = PhyMibInfo.adslConnection.modType ;
         UpdateLinkMode (ulDSLGinpMode, ulDSLMode) ;
      }
   } /* Link Status & Mib Non-Null */
}


/***************************************************************************
 * Function Name: AutoSensePreProcess
 * Description  : Effect some preprocess procedures for ATM modes auto-sensing
 *                if applicable. PTM modes are skipped.
 * Returns      : void
 ***************************************************************************/
void XTM_PROCESSOR::AutoSensePreProcess ( UINT32 ulPhysPort, UINT32 ulTrafficType, 
    PXTM_INTERFACE_LINK_INFO pLinkInfo )
{
   int ret;
   long size = sizeof (PhyMibInfo) ;

   XtmOsPrintf ("bcmxtmcfg: AutoSensePreProcess Port=%d, Traffic type=%d State=%d \n", (UINT32) ulPhysPort, (UINT32) ulTrafficType, (UINT32) pLinkInfo->ulLinkState) ;
   switch(ulTrafficType) {
      /* Send ATM indications to Autosense state machine */
      case TRAFFIC_TYPE_ATM_BONDED:
         /* If PHY declares we're in ATM bonding, update Autosense state machine to match. */
         m_XtmAutoSense.BondingIndicationRx();
         break;

      case TRAFFIC_TYPE_NOT_CONNECTED:
         /* For not connected case, line activity still applies & we can
         ** continue to fall down for auto sensing */
      case TRAFFIC_TYPE_ATM:
      case TRAFFIC_TYPE_ATM_TEQ: {
         /* For all other ATM modes, update line link states */
         /* We can read the DSL MIB for this line to see if Nitro is enabled.
          * If Nitro is enabled, it is ATM non-bonded mode.
          */
         if (pLinkInfo->ulLinkState == LINK_DOWN) {
            m_XtmAutoSense.LineIdle(ulPhysPort);
         } else {

            if (g_pfnAdslGetObjValue != NULL) {
               size = sizeof (PhyMibInfo) ;
               ret  = g_pfnAdslGetObjValue (ulPhysPort, NULL, 0, (char *) &PhyMibInfo, &size) ;
               if (ret != 0)
                  XtmOsPrintf ("Xtm SILI:Error g_pfnAdslGetObjValue port - %d return value - %d \n", ulPhysPort, ret) ;
               else
                  m_ulXTMNitroMode = (PhyMibInfo.adslFramingMode & kAtmHeaderCompression) ?
                                      XTM_MODE_NITRO_ENABLE : XTM_MODE_NITRO_DISABLE ;
            }

            if (m_ulXTMNitroMode == XTM_MODE_NITRO_ENABLE) {
               XtmOsPrintf ("bcmxtmcfg: ATM Nitro Mode enabled. \n") ;
               m_XtmAutoSense.ForceSingleLinePhy() ;
            }
            else {
               XtmOsPrintf ("bcmxtmcfg: Port %d State = %d \n", (UINT32) ulPhysPort, (UINT32) pLinkInfo->ulLinkState) ;
               m_XtmAutoSense.LineActive(ulPhysPort);
            }
         }


         break;
      }

      default:
         /* For all other states, we must be in PTM mode.  Reset Autosense state machine */
         m_XtmAutoSense.NonAtmMode();
         break;
   }
}

#ifdef SAR_RX_CTL_LED_CFG
/***************************************************************************
 * Function Name: SetupWanDataLed
 * Description  : Called when an ADSL/VDSL connection has come up or gone down.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
void XTM_PROCESSOR::SetupWanDataLed (PXTM_INTERFACE_LINK_INFO pLinkInfo, PXTM_INTERFACE_LINK_INFO pOtherLinkInfo)
{
   /* If we have a SAR LED configuration register, set it up to program the INT_ACT LED */
   uint32  uiLedCfgReg = 0;                /* Temp variable for SAR LED CFG register */

#if !defined (CONFIG_BCM963138) && !defined (CONFIG_BCM963148) && !defined (CONFIG_BCM963381) && !defined(CONFIG_BCM963158)
   /* Set up default LED configuration */
   uiLedCfgReg = SARLEDCFG_LED_EN |        /* Enable LED */
      SARLEDCFG_BLINK_30MS |              /* Blink only 30mS ion traffic */
      SARLEDCFG_MODE_TRAFFICLINK |        /* Light on link and blink on traffic */
      SARLEDCFG_LED_EN;                   /* Blink 30mS on traffic */

   /* Is the link up? */
   if ((pLinkInfo->ulLinkState == LINK_UP) ||
         ((pOtherLinkInfo) && (pOtherLinkInfo->ulLinkState == LINK_UP))) {
      uiLedCfgReg = uiLedCfgReg | SARLEDCFG_LNK;
      kerSysLedCtrl (kLedWanData, kLedStateOn) ;
   }
   else {
      uiLedCfgReg = uiLedCfgReg & ~SARLEDCFG_LNK;
      kerSysLedCtrl (kLedWanData, kLedStateOff) ;
   }
#else
   uiLedCfgReg = SARLEDCFG_LED_EN |       /* Enable LED */
      SARLEDCFG_BLINK_30MS |              /* Blink only 30mS ion traffic */
      SARLEDCFG_MODE_TRAFFIC |            /* Light on link and blink on traffic */
      SARLEDCFG_INT_LED |                 /* Use internal logic */
      SARLEDCFG_LED_EN;                   /* Blink 30mS on traffic */

   /* Is the link up? */
   if ((pLinkInfo->ulLinkState == LINK_UP) ||
         ((pOtherLinkInfo) && (pOtherLinkInfo->ulLinkState == LINK_UP))) {
      uiLedCfgReg = uiLedCfgReg | SARLEDCFG_LNK;
      kerSysLedCtrl (kLedWanData, kLedStateOn) ;
   }
   else {
      uiLedCfgReg = uiLedCfgReg & ~SARLEDCFG_LNK;
      kerSysLedCtrl (kLedWanData, kLedStateOff) ;
   }
#endif

   //*SARRXCTLLEDCFG = uiLedCfgReg;           /* Write the SAR LED CFG register */
   XP_REGS->ulLedCtrl = uiLedCfgReg; 
}
#endif

#if defined(XTM_PORT_SHAPING)
void XTM_PROCESSOR::UpdateSitSlr(UINT32 ulPort)
{

   XtmOsPrintf ("bcmxtmcfg: Port-%d, PortShaping-%d, QShaping-%d, TxPAF-%d \n", ulPort,
                m_ulPortShapingConfig, m_ulQShapingConfig, m_ulTxPafEnabled) ;

   if( (m_ulPortShapingConfig == PORT_Q_SHAPING_ON) ||
         (m_ulQShapingConfig == PORT_Q_SHAPING_ON) ||
         (m_ulTxPafEnabled) ) {
      XP_REGS->ulTxLineRateTimer = m_ulSlrCnt;
      XP_REGS->ulSstSitValue     = m_ulSitCnt;
   }
   else
   {
      XtmOsPrintf ("bcmxtmcfg: ReConfigure Max UT port shaping %d\n", ulPort) ;
      XP_REGS->ulTxLineRateTimer = XTMCFG_LINE_RATE_TIMER ;
      XP_REGS->ulSstSitValue     = XTMCFG_SST_SIT_VALUE ;
   }
}

void XTM_PROCESSOR::ConfigureTxPortShapingRatios(UINT32 ulPhysPort, UINT32 ulTrafficType)
{
   UINT32 shapedPort;
   UINT32 isVdslLinkMode;
   UINT32 ulBondingPhysPort;
   PXTM_INTERFACE_LINK_INFO pLinkInfo  = NULL;
   PXTM_INTERFACE_LINK_INFO pOtherLinkInfo  = NULL;
   UINT32 ulPortShapingRatio = 0;
   shapedPort = MAX_INTERFACES;
   if((m_ulXTMLinkMode == XTM_LINK_MODE_VDSL) || (m_ulXTMLinkMode == XTM_LINK_MODE_VDSL_RTX))
      isVdslLinkMode = 1;
   else
      isVdslLinkMode = 0;
   pLinkInfo = m_Interfaces[ulPhysPort].GetLinkInfo() ;
   ulPortShapingRatio = XTM_PORT_SHAPING_RATIO_FULL;
   XtmOsPrintf("ulTrafficType:%u \n",ulTrafficType);
   if((ulTrafficType == TRAFFIC_TYPE_PTM_BONDED) && (isVdslLinkMode == 1)) {
      ulBondingPhysPort = m_Interfaces[ulPhysPort].GetBondingPortNum();
      pOtherLinkInfo = m_Interfaces[ulBondingPhysPort].GetLinkInfo();
      XtmOsPrintf("ulPhysPort:%u ulBondingPhysPort:%u \n",ulPhysPort,ulBondingPhysPort);
      XtmOsPrintf("ulPhysPort LS:%u ulBondingPhysPort LS:%u \n",pLinkInfo->ulLinkState
                                                               ,pOtherLinkInfo->ulLinkState);
      // In PTM bonding mode all traffic goes through the Port0
      // so configuring the TX port shaping only to PORT0.
      if((pLinkInfo->ulLinkState == LINK_UP) || (pOtherLinkInfo->ulLinkState == LINK_UP))
      {
         shapedPort = PHY_PORTID_0;
      }
      else
      {
         XtmOsPrintf("Both links are down disabling TX port shaping\n");
         m_Interfaces[PHY_PORTID_0].DisableTxPortShaping();
      }
   }
   else if((ulTrafficType == TRAFFIC_TYPE_PTM) && (isVdslLinkMode == 1)){
      // Non Bonding case
      if(pLinkInfo->ulLinkState == LINK_UP)
      {
         shapedPort = ulPhysPort;
      }
      else
      {
         m_Interfaces[ulPhysPort].DisableTxPortShaping();
      }
   }
   if(shapedPort != MAX_INTERFACES)
   {
      m_Interfaces[shapedPort].ConfigureShapingRatio(ulPortShapingRatio);
      m_ulPortShapingConfig |= m_Interfaces[shapedPort].ConfigureTxPortShaping(ulTrafficType) ;
   }
}
#endif

/***************************************************************************
 * Function Name: SetInterfaceLinkInfo
 * Description  : Called when an ADSL/VDSL connection has come up or gone down.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_PROCESSOR::SetInterfaceLinkInfo( UINT32 ulPortId,
    PXTM_INTERFACE_LINK_INFO pLinkInfo, UINT32 timerExpired )
{
   UINT32 ulSRAEvent = 0, timerStopped = 1 ;
   BCMXTM_STATUS bxStatus = XTMSTS_SUCCESS;
   UINT32 ulPhysPort = PORTID_TO_PORT(ulPortId);
   UINT32 ulBondingPhysPort;
   UINT32 ulTrafficType = pLinkInfo->ulLinkTrafficType ;
   PXTM_INTERFACE_LINK_INFO pOtherLinkInfo = NULL ;
   PXTM_INTERFACE_LINK_INFO pCurrLinkInfo ;

   MapXtmOsPrintf ("bcmxtmcfg: P=%d,S=%d,T=%d\n",  ulPhysPort, pLinkInfo->ulLinkState, ulTrafficType) ;

   if ( ulPhysPort >= MAX_INTERFACES )
      return XTMSTS_PARAMETER_ERROR;

   if (!timerExpired) {
       if (XtmOsStopTimer ((void *) XtmMonitorTimerCb) <= 0)
          timerStopped = 0 ;
   }

   UpdateLinkStatus (ulPhysPort, pLinkInfo) ;

   /* Are we using Autosense for determining bonding/nonbonding PHY? */
   if(m_XtmAutoSense.PhyStatus() != PHYSTATUS_AUTOSENSE_DISABLED)
      AutoSensePreProcess (ulPhysPort, ulTrafficType, pLinkInfo) ;

   if((pLinkInfo->ulLinkState != LINK_UP) && (pLinkInfo->ulLinkState != LINK_DOWN))
      goto _End;

   /* Check if the usermode operations (which are currently checked is the
    * creation of Network devices) are on right now, then defer this event and
    * come back later.
    */
   if (pLinkInfo->ulLinkState == LINK_UP) {
      if ((timerStopped) && (m_ulPrevRelatedUpEvtValid == 0)) {
         m_ulPrevRelatedUpEvtValid = XTM_BOND_DSL_PEND_EVT_DURATION_TIMEOUT_MS ;
         /* Fall through to process this event */
      }
      else /*if((timerStopped) && (m_ulPrevRelatedUpEvtValid != 0))*/{
         /* If prev related event is valid, (concurrent events happening), Save
          * this and come back later.
          */
         if (m_ulPrevRelatedUpEvtValid == 0)
              m_ulPrevRelatedUpEvtValid = XTM_BOND_DSL_PEND_EVT_DURATION_TIMEOUT_MS ;
         m_ulPendingEvtValid[ulPhysPort] = 1 ;
         m_ulPendingEvtPortId[ulPhysPort] = ulPortId ;
         memcpy (&m_PendingEvtLinkInfo[ulPhysPort], pLinkInfo, sizeof (XTM_INTERFACE_LINK_INFO)) ;
         XtmOsPrintf ("bcmxtmcfg: PendEvt1 LI, P=%d, S=%d \n", ulPhysPort, pLinkInfo->ulLinkState) ;
         goto _End ;
      }
   }
   else {
      if ((!timerStopped) || (m_ulPendingEvtValid[ulPhysPort] == 1)) {
         m_ulPendingEvtPortId[ulPhysPort] = ulPortId ;
         if (m_ulPendingEvtValid[ulPhysPort] != 1) {
             m_ulPrevRelatedUpEvtValid = XTM_BOND_DSL_PEND_EVT_DURATION_TIMEOUT_MS ;
             m_ulPendingEvtValid[ulPhysPort] = 1;
         }
         memcpy (&m_PendingEvtLinkInfo[ulPhysPort], pLinkInfo, sizeof (XTM_INTERFACE_LINK_INFO)) ;
         XtmOsPrintf ("bcmxtmcfg: PendEvt2 LI, P=%d, S=%d \n", ulPhysPort, pLinkInfo->ulLinkState) ;
         goto _End ;
      }
      else {
         m_ulPrevRelatedUpEvtValid = 0;
      }
   }

	/* This is to aid in sensing the traffic from xtm perspective. */
   ulBondingPhysPort = m_Interfaces[ulPhysPort].GetBondingPortNum () ;
   if (ulBondingPhysPort != MAX_INTERFACES)
      pOtherLinkInfo = m_Interfaces[ulBondingPhysPort].GetLinkInfo() ;

   if (pLinkInfo->ulLinkState == LINK_DOWN) {

        pLinkInfo->ulLinkUsRate = pLinkInfo->ulLinkDsRate = 0 ;

        if ((m_InitParms.bondConfig.sConfig.autoSenseAtm == BC_ATM_AUTO_SENSE_ENABLE)
              &&
            (m_XtmAutoSense.PhyStatus() == PHYSTATUS_ATM_SINGLELINE)) {

           /* Check for if the other link is active. The only case the
           ** other link is active under the previous checks would be if
           ** it is ATM non-bonded with both the lines connected state.
           ** If so, if the line that's link state is down is not the
           ** primary line (line 0), we need to set the interface state
           ** of the line, plus we send out the available link state
           ** flow through the function to take it effect.
           **/
           if (ulPhysPort == PHY_PORTID_1) {
               XtmOsPrintf ("bcmxtmcfg: Link down on port %d \n", ulPhysPort) ;
               bxStatus = m_Interfaces[ulPhysPort].SetLinkInfo( pLinkInfo, XTM_CONFIGURE, XTM_CONFIGURE,
                     XTM_CONFIGURE, m_ulXTMLinkMode);
               bxStatus = m_Interfaces[PHY_PORTID_0].UpdateLinkInfo (ulTrafficType) ;
               goto _End ;
           }
            }
   }

    XtmOsPrintf ("bcmxtmcfg:ulPhysPort[%x] m_ulTrafficMonitorPort[%x]\n",ulPhysPort,m_ulTrafficMonitorPort);
    /* Is this ATM traffic? */
    if ((ulTrafficType == TRAFFIC_TYPE_ATM) || (ulTrafficType == TRAFFIC_TYPE_ATM_BONDED)) {
        /* Is autosense enabled/ */
        if (m_InitParms.bondConfig.sConfig.autoSenseAtm == BC_ATM_AUTO_SENSE_ENABLE) {
            // Check for max ports and update state machine to reflect bonding
            if (m_ulTrafficMonitorPort == MAX_INTERFACES) {
                // Correct the port number to the physical port
                m_ulTrafficMonitorPort = ulPhysPort ;

                // Now update link type
                ulTrafficType = pLinkInfo->ulLinkTrafficType = TRAFFIC_TYPE_ATM_BONDED ;

            }
            else if ((m_XtmAutoSense.PhyStatus() == PHYSTATUS_ATM_BONDING) ||
                       ((pOtherLinkInfo) && (pOtherLinkInfo->ulLinkState == LINK_UP))) {
                // Set to bonding
                ulTrafficType = pLinkInfo->ulLinkTrafficType = TRAFFIC_TYPE_ATM_BONDED ;
            }
            else {
         
                // Set to single line
                ulTrafficType = pLinkInfo->ulLinkTrafficType = TRAFFIC_TYPE_ATM ;
            }  
        } /* if auto sense atm enable */
        else {
           XtmOsPrintf ("bcmxtmcfg: AutoSenseATM is not enabled. Link Traffic type from PHY=%lu \n",
                      ulTrafficType) ;
           // Correct the port number to the physical port
           m_ulTrafficMonitorPort = ulPhysPort;
        }
    } /* if (atm types) */
    else if((ulTrafficType == TRAFFIC_TYPE_PTM) /*|| (ulTrafficType == TRAFFIC_TYPE_PTM_BONDED)*/) {
       m_ulTrafficMonitorPort = ulPhysPort;
    }
    else {
       m_ulTrafficMonitorPort = PHY_PORTID_0;
    }

   XtmOsPrintf ("bcmxtmcfg: XTM LinkInfo. P-%d, S-%s(%d), Service =%s(%d) \n",
                ulPhysPort,
                ((pLinkInfo->ulLinkState == LINK_UP) ? "UP" :
                  ((pLinkInfo->ulLinkState == LINK_DOWN) ? "DOWN":
                   ((pLinkInfo->ulLinkState == LINK_START_TEQ_DATA) ? "START_TEQ":
                    ((pLinkInfo->ulLinkState == LINK_STOP_TEQ_DATA) ? "STOP_TEQ" : "Not Connected")))),
                pLinkInfo->ulLinkState,
                (ulTrafficType == TRAFFIC_TYPE_ATM ? "ATM" :
                 (ulTrafficType == TRAFFIC_TYPE_PTM ? "PTM" :
                  (ulTrafficType == TRAFFIC_TYPE_PTM_BONDED ? "PTMBND" :
                   (ulTrafficType == TRAFFIC_TYPE_ATM_BONDED ? "ATMBND" : 
                    (ulTrafficType == TRAFFIC_TYPE_PTM_RAW ? "PTMRAW" : "None"))))),
                ulTrafficType) ;

   pCurrLinkInfo = m_Interfaces[ulPhysPort].GetLinkInfo() ;

   if ((pCurrLinkInfo) && (pCurrLinkInfo->ulLinkState == pLinkInfo->ulLinkState)) /* duplicate */ {
      if (pLinkInfo->ulLinkState == LINK_UP) {
         ulSRAEvent = 1 ;
         MapXtmOsPrintf ("bcmxtmcfg: XTM Link Information, port = %d, Line UP SRA Event from PHY \n", ulPhysPort) ;
      }
      else {
         MapXtmOsPrintf ("bcmxtmcfg: XTM Link Information, port = %d, Duplicate Event from PHY \n", ulPhysPort) ;
         goto _End ;
      }
   }

   /* Reconfigure SAR based on the line traffic type.
    * We support Bonding modes (PTM-ATM) change on the fly.
    * Bonding to Normal mode requires reconfiguration. This is done only when
    * the first available link is UP.
    */
   if ((pLinkInfo->ulLinkState == LINK_UP) &&
       (!pOtherLinkInfo || (pOtherLinkInfo->ulLinkState == LINK_DOWN))) {

      if (CheckTrafficCompatibility (ulTrafficType) != XTMSTS_SUCCESS) {
         XtmOsSendSysEvent (XTM_EVT_TRAFFIC_TYPE_MISMATCH) ;
         XtmOsPrintf ("\nbcmxtmcfg: Traffic incompatibility between CO & CPE. Reconfiguring the system. \n") ;
         if (ReInitialize  (ulTrafficType) != XTMSTS_SUCCESS)
            XtmOsPrintf ("\nbcmxtmcfg: Fatal - ReInitialize XTM part due to Traffic type change has failed \n") ;
      }

		/* After traffic compatibility, redo the bond pair identification */

		ulBondingPhysPort = m_Interfaces[ulPhysPort].GetBondingPortNum () ;
		if (ulBondingPhysPort != MAX_INTERFACES)
				  pOtherLinkInfo = m_Interfaces[ulBondingPhysPort].GetLinkInfo() ;

		ReconfigureSAR  (ulPhysPort, ulTrafficType) ;
	}

   switch( pLinkInfo->ulLinkState )
   {
      case LINK_UP:
      case LINK_DOWN:

         {
            UINT32 ulSlrCnt;
            UINT32 ulSitCnt;
            UINT32 ulSitLoCnt;
            UINT32 ulLinkRateUnitsPerSec;

            if ((ulTrafficType & TRAFFIC_TYPE_ATM_MASK) == TRAFFIC_TYPE_ATM)
            {
               const UINT32 ulBitsPerCell = (53 * 8);

               ulLinkRateUnitsPerSec = (pOtherLinkInfo) ? 
                  (pLinkInfo->ulLinkUsRate+pOtherLinkInfo->ulLinkUsRate)/ulBitsPerCell :
                  pLinkInfo->ulLinkUsRate/ulBitsPerCell;

#ifdef CONFIG_BCM963268
               if (m_ulChipIdRev < 0xC0) {  /* Future Proof */
                  if (pOtherLinkInfo && (pOtherLinkInfo->ulLinkUsRate == 0))
                     ulLinkRateUnitsPerSec *= 2 ;
               }
#endif

               /* SLR_CNT is the number of SAR clocks per cell */
                    ulSlrCnt = (ulLinkRateUnitsPerSec != 0) ? (SAR_CLOCK/ulLinkRateUnitsPerSec) : 0;

               /* Scale SLR_CNT for a faster rate */
               ulSlrCnt = ulSlrCnt * 8 / 10;
            }
            else
            {
               const UINT32 ulBitsPerPacket = (64 * 8);  /* use the smallest packet size */

               ulLinkRateUnitsPerSec = (pOtherLinkInfo) ? 
                  (pLinkInfo->ulLinkUsRate+pOtherLinkInfo->ulLinkUsRate)/ulBitsPerPacket :
                  pLinkInfo->ulLinkUsRate/ulBitsPerPacket;

               ulLinkRateUnitsPerSec *= 8 ;  /* workaround - 8 times faster than the actual rates. */

#ifdef CONFIG_BCM963268
               if (m_ulChipIdRev < 0xC0) {  /* Future Proof */
                  if (pOtherLinkInfo && (pOtherLinkInfo->ulLinkUsRate == 0))
                     ulLinkRateUnitsPerSec *= 2 ;
               }
#endif

               /* SLR_CNT is the number of SAR clocks per packet */
               ulSlrCnt = (ulLinkRateUnitsPerSec != 0) ? (SAR_CLOCK/ulLinkRateUnitsPerSec) : 0;
               
               /* Scale SLR_CNT for a faster rate */
               ulSlrCnt = ulSlrCnt / 4;
#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178) 
               if((ulTrafficType == TRAFFIC_TYPE_PTM) || (ulTrafficType == TRAFFIC_TYPE_PTM_BONDED))
                  ulSlrCnt = 0x20;
#endif
            }

               if (ulSlrCnt < 0x20) {
                  /* Line rate timer is NOT stable under these values */
                  ulSlrCnt = 0x20 ;
               }
            
            /* SIT_CNT is the number of SAR clocks per SIT pulse.
             * Set it to 2 us.
             */
            ulSitCnt = (SAR_CLOCK / 1000000L) * 2;

            /* Set SIT_LO_CNT to 64 us. */
            ulSitLoCnt = ulSitCnt * 32;

            /* Save SIT_CNT and SIT_LO_CNT */
            XTM_CONNECTION::SetSitUt( ulSitCnt, ulSitLoCnt );
#if defined(XTM_PORT_SHAPING)
            m_Interfaces[ulPhysPort].SetSitUt( ulSitCnt, ulSitLoCnt );
#endif

            if( pLinkInfo->ulLinkState == LINK_UP )
            {

#ifdef CONFIG_BCM963268
               if ((m_ulChipIdRev >= 0xC0) || (XP_REGS->ulTxLineRateTimer == 0)) 
#endif
               {

                  XP_REGS->ulTxLineRateTimer = ulSlrCnt;
                  XP_REGS->ulSstSitValue     = ulSitCnt;
                  XP_REGS->ulSstSitSlowValue = ulSitLoCnt;
                  m_ulSlrCnt = ulSlrCnt;
                  m_ulSitCnt = ulSitCnt;
                  m_ulSitLoCnt = ulSitLoCnt;
#if defined(CONFIG_BCM963158) 
#if (CONFIG_BRCM_CHIP_REV != 0x63158A0)
                  if (m_ulXTMLinkMode == XTM_LINK_MODE_GFAST) {
                     XP_REGS->ulTxSchedCfg |= TXSCH_SIT_COUNT_EN | TXSCH_SITLO_COUNT_EN |
                        TXSCH_SLR_COUNT_EN | TXSCH_SLR_EN | TXSCH_SOFWT_PRIORITY_EN |
                        TXSCH_VC_GTS_EN | TXSCH_MP_GTS_EN | 
                        (TXSCH_GFAST_MPAAL_SEL_DEFAULT << TXSCH_GFAST_MPAAL_SEL_SHIFT);
                  }
                  else
#endif
#endif
                  XP_REGS->ulTxSchedCfg |= TXSCH_SIT_COUNT_EN | TXSCH_SITLO_COUNT_EN |
                     TXSCH_SLR_COUNT_EN | TXSCH_SLR_EN | TXSCH_SOFWT_PRIORITY_EN |
                     TXSCH_VC_GTS_EN | TXSCH_MP_GTS_EN;
               }

               if( (ulTrafficType & TRAFFIC_TYPE_ATM_MASK) == TRAFFIC_TYPE_ATM )
               {
                  XP_REGS->ulTxSarCfg &= ~TXSAR_MODE_PTM;
                  XP_REGS->ulRxSarCfg &= ~RXSAR_MODE_PTM;
                  XP_REGS->ulTxSarCfg &= ~TXSARP_ENABLE_RAW_PTM_MODE ;
                  XP_REGS->ulTxSarCfg |= TXSAR_MODE_ATM;
                  XP_REGS->ulRxSarCfg |= RXSAR_MODE_ATM;
               }
               else
               {
                  XP_REGS->ulTxSarCfg &= ~TXSAR_MODE_ATM;
                  XP_REGS->ulRxSarCfg &= ~RXSAR_MODE_ATM;
                  XP_REGS->ulTxSarCfg &= ~TXSARP_ENABLE_RAW_PTM_MODE ;
                  XP_REGS->ulTxSarCfg |= TXSAR_MODE_PTM | TXSARP_SOF_WHILE_TX ;
                  XP_REGS->ulRxSarCfg |= RXSAR_MODE_PTM;

                  if( ulTrafficType == TRAFFIC_TYPE_PTM_RAW ) {
                     XP_REGS->ulTxSarCfg |= TXSARP_ENABLE_RAW_PTM_MODE ;
                     XP_REGS->ulTxSarCfg &= ~TXSARP_SOF_WHILE_TX ;
                     XP_REGS->ulRxSarCfg &= ~RXSAR_MODE_ATM;
                     XP_REGS->ulTxSarCfg |= TXSAR_MODE_PTM;
                     XP_REGS->ulRxSarCfg |= RXSAR_MODE_PTM;
                  }
               }
            }
            else {
               /* Set to the fast pace temporarily to make draining of data faster across the tx queues, which will start
                * in this course as the one of the availale link(s) is going
                * down. This will be set to a correct value towards the end
                * based on the available link states.
                */
               XP_REGS->ulTxLineRateTimer = 0x20;
            }

            if( ulPhysPort < MAX_INTERFACES )
            {
               UINT32 i,idx;
               UINT32 ulPortMask, ulLinkMask;
               XTM_ADDR Addr;

               /* Create a "link up" bit mask of connected interfaces. */
               for( i = PHY_PORTID_0, ulLinkMask = 0; i < MAX_INTERFACES; i++ )
               {
                  PXTM_INTERFACE_LINK_INFO pLi = m_Interfaces[i].GetLinkInfo();
                  if( pLi->ulLinkState == LINK_UP )
                     ulLinkMask |= PORT_TO_PORTID(i);
               }

               /* If link down, subtract the port id of the interface that just
                * changed.  If link up, add the port id of the interface.
                */
               if( pLinkInfo->ulLinkState == LINK_DOWN )
                  ulLinkMask &= ~ulPortId;
               else
                  ulLinkMask |= ulPortId;


               /* Call connections that are affected by the link change. A connection
                * is affected if it is connected and none of its interfaces are
                * currently up or it is not connected and atleast one of its
                * interfaces is currently up.
                * In addition, if the link status changes either way as long as
                * the link is part of the port mask of the connection.
                */
               i = 0;
               XTM_CONNECTION *pConn;
               XTM_CONN_CFG   ConnCfg;
               while( (pConn = m_ConnTable.Enum( &i )) != NULL )
               {
                  UINT32 ulDoConnSetLinkInfo = 0;

                  pConn->GetAddr( &Addr );
                  ulPortMask = Addr.u.Conn.ulPortMask ;

                  if( ulBondingPhysPort != MAX_INTERFACES )
                     ulDoConnSetLinkInfo = ((ulPortId & ulPortMask) && pConn->Connected())? 1 : (PORTID_TO_PORT(ulPortId) == PHY_PORTID_0)? 1 : 0;
                  else 
                     ulDoConnSetLinkInfo = (((ulPortMask & ulLinkMask) == 0 &&
                              pConn->Connected()) ||
                           (((ulTrafficType == Addr.ulTrafficType) || (ulTrafficType == TRAFFIC_TYPE_PTM_RAW)) &&
                            (ulPortMask & ulLinkMask) == ulPortMask &&
                            !pConn->Connected())) ? 1 : 0;

                  if (ulDoConnSetLinkInfo)
                  {
                     //check if TqParams needs to be updated.
                     bxStatus = pConn->GetCfg(&ConnCfg);
                     if((bxStatus == XTMSTS_SUCCESS) && (ulPhysPort == PHY_PORTID_0) 
                     && (pLinkInfo->ulLinkState == LINK_UP) && (ulBondingPhysPort != MAX_INTERFACES)) {
                        m_ulTrafficMonitorPort = PHY_PORTID_0;
                        m_LogPhyPortmap[0].PhysicalPortId = PHY_PORTID_0;
                        for(idx=0;idx<ConnCfg.ulTransmitQParmsSize;idx++) {
                           ConnCfg.TransmitQParms[idx].ulPortId = PORT_TO_PORTID(ulPhysPort);
                        }
                        if((bxStatus = pConn->SetCfg(&ConnCfg, m_ulXTMLinkMode, m_ulXTMNitroMode))!= XTMSTS_SUCCESS)
                           XtmOsPrintf("bcmxtmcfg: Failed to reconfigure the connection configuration for bonded case and PORT 0 LINK up status=%d \n", bxStatus);
                     }
                     //XtmOsPrintf("Calling SetLinkInfo 1\n");
                     bxStatus = pConn->SetLinkInfo( (ulLinkMask & ulPortMask),
                                    pLinkInfo, ulBondingPhysPort, pOtherLinkInfo, 
                                    m_ulXTMLinkMode, m_ulXTMNitroMode);

                     if( bxStatus == XTMSTS_IN_USE)
                     {
                         if (m_ulPrevRelatedUpEvtValid == 0)
                              m_ulPrevRelatedUpEvtValid = XTM_BOND_DSL_PEND_EVT_DURATION_TIMEOUT_MS ;
                         m_ulPendingEvtValid[ulPhysPort] = 1 ;
                         m_ulPendingEvtPortId[ulPhysPort] = ulPortId ;
                         memcpy (&m_PendingEvtLinkInfo[ulPhysPort], pLinkInfo, sizeof (XTM_INTERFACE_LINK_INFO)) ;
                         XtmOsPrintf ("bcmxtmcfg: PendEvt3 LI, P=%d, S=%d \n", ulPhysPort, pLinkInfo->ulLinkState) ;
                         goto _End ;
                     }
                     else if( bxStatus != XTMSTS_SUCCESS )
                        break;
                  }
               }

               /* Call the interface that there is a link change. */
               CheckAndSetIfLinkInfo (ulTrafficType, ulPhysPort, pLinkInfo, ulBondingPhysPort,
                     pOtherLinkInfo) ;

               if (m_InitParms.bondConfig.sConfig.atmBond == BC_ATM_BONDING_ENABLE)
                  m_AsmHandler.LinkStateNotify(ulPhysPort) ;

#if defined(XTM_PORT_SHAPING)
               MapXtmOsPrintf("%s:Calling Port Shaping \n",__FUNCTION__);
               ConfigureTxPortShapingRatios(ulPhysPort,ulTrafficType);
#endif
               if( pLinkInfo->ulLinkState == LINK_DOWN )
               {
                  /* Call bcmxtmrt driver with NULL device context to stop the
                   * receive DMA.
                   */
                  if(m_pfnXtmrtReq != NULL) {
                            (*m_pfnXtmrtReq) (NULL, XTMRT_CMD_LINK_STATUS_CHANGED,
                                &ulPortId);
                        }

                  if (!GetAvailLinkStatus ()) { /* No links are available/UP */

                     XP_REGS->ulTxLineRateTimer = 0;
                     ReconfigureSAR  (ulPhysPort, TRAFFIC_TYPE_NOT_CONNECTED);
                     m_ulTrafficMonitorPort = MAX_INTERFACES ;

                  }
                  else {

#ifdef CONFIG_BCM963268
                     if ((m_ulChipIdRev >= 0xC0) || (XP_REGS->ulTxLineRateTimer == 0)) 
#endif
                     {
                        XP_REGS->ulTxLineRateTimer = ulSlrCnt;
                        XP_REGS->ulSstSitValue     = ulSitCnt;
                     }
                  }
               } /* (if Link is DOWN) */

#if defined(XTM_PORT_SHAPING)
               /* UT port shaping at the highest SIT configuration is to be
               ** enabled only under the following circumstances, as it
               ** interferes with the Port rate limit/Q shaping features.
               */
               if( (m_ulPortShapingConfig == PORT_Q_SHAPING_OFF) &&
                   (m_ulQShapingConfig == PORT_Q_SHAPING_OFF) &&
                   (!m_ulTxPafEnabled) ) {

                   m_Interfaces[ulPhysPort].ConfigureMaxUTPortShaping(ulTrafficType) ;
               }
#endif
            } /* if (valid interface) */
            else
               bxStatus = XTMSTS_PARAMETER_ERROR;
         }

#ifdef SAR_RX_CTL_LED_CFG
         SetupWanDataLed (pLinkInfo, pOtherLinkInfo) ;
#endif

         break;

      case LINK_START_TEQ_DATA:
         //XP_REGS->ulRxAtmCfg[ulPhysPort] = RX_PORT_EN |
            //RXA_VCI_MASK | RXA_VC_BIT_MASK;
         XP_REGS->ulRxAtmCfg[ulPhysPort] = RX_PORT_EN ;
         XP_REGS->ulRxUtopiaCfg &= ~RXUTO_INT_TEQ_PORT_MASK;
         XP_REGS->ulRxUtopiaCfg |= (0x1 << (ulPhysPort+RXUTO_INT_TEQ_PORT_SHIFT)) ;
         XP_REGS->ulRxVpiVciCam[(TEQ_DATA_VCID * 2) + 1] =
            RXCAM_CT_TEQ | (TEQ_DATA_VCID << RXCAM_VCID_SHIFT);
         XP_REGS->ulRxVpiVciCam[TEQ_DATA_VCID * 2] =
            ulPhysPort | RXCAM_TEQ_CELL | RXCAM_VALID;
         XP_REGS->ulRxPafVcid &= ~(RXPAF_INVALID_VCID<<(ulPhysPort*RXPAF_VCID_SHIFT)) ;
         XP_REGS->ulRxPafVcid |= (TEQ_DATA_VCID<<(ulPhysPort*RXPAF_VCID_SHIFT)) ;
         XP_REGS->ulRxPafWriteChanFlush &= ~(0xFFFFFF11 << ulPhysPort) ;

         /* Inform XtmRt */
         if(m_pfnXtmrtReq != NULL) {
             (*m_pfnXtmrtReq) (NULL, XTMRT_CMD_SET_TEQ_DEVCTX,
                NULL);
         }
         break;

      case LINK_STOP_TEQ_DATA:
         XP_REGS->ulRxAtmCfg[ulPhysPort] = 0;
         XP_REGS->ulRxUtopiaCfg &= ~RXUTO_INT_TEQ_PORT_MASK ;
         XP_REGS->ulRxVpiVciCam[(TEQ_DATA_VCID * 2) + 1] = 0;
         XP_REGS->ulRxVpiVciCam[TEQ_DATA_VCID * 2] = 0;
         XP_REGS->ulRxPafVcid &= ~(RXPAF_INVALID_VCID<<(ulPhysPort*RXPAF_VCID_SHIFT)) ;
         XP_REGS->ulRxPafVcid |= (RXPAF_INVALID_VCID<<(ulPhysPort*RXPAF_VCID_SHIFT)) ;
         XP_REGS->ulRxPafWriteChanFlush |= (0x11 << ulPhysPort) ;
         break;
   }

_End :
   /* Initialization is complete.  Launch timer task if it was required. */
   if ((timerStopped) &&  (!timerExpired))
       XtmOsStartTimer ((void *) XtmMonitorTimerCb, 0, XTM_BOND_DSL_MONITOR_TIMEOUT_MS) ;

   //XtmOsPrintf ("SILI done port=%d \n", ulPhysPort) ;

   return( bxStatus );
} /* SetInterfaceLinkInfo */


/***************************************************************************
 * Function Name: SendOamCell
 * Description  : Sends an OAM loopback cell.  For request cells, it waits for
 *                the OAM loopback response.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_PROCESSOR::SendOamCell( PXTM_ADDR pConnAddr,
    PXTM_OAM_CELL_INFO pOamCellInfo )
{
    BCMXTM_STATUS bxStatus = XTMSTS_STATE_ERROR;
    UINT32 ulPort = PORTID_TO_PORT(pConnAddr->u.Vcc.ulPortMask);
    PXTM_INTERFACE_LINK_INFO pLi = m_Interfaces[ulPort].GetLinkInfo();
    XTM_ADDR internalConnAddr;

    memcpy (&internalConnAddr, pConnAddr, sizeof (XTM_ADDR)) ;
    UpdateConnAddrForBonding ( &internalConnAddr, XTM_ADD ) ;

    if( ulPort < MAX_INTERFACES && pLi->ulLinkState == LINK_UP )
        bxStatus = m_OamHandler.SendCell(pConnAddr, &internalConnAddr, pOamCellInfo, &m_ConnTable);

    UpdateConnAddrForBonding ( &internalConnAddr, XTM_REMOVE ) ;

    return( bxStatus );
} /* SendOamCell */


/***************************************************************************
 * Function Name: CreateNetworkDevice
 * Description  : Call the bcmxtmrt driver to create a network device instance.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_PROCESSOR::CreateNetworkDevice( PXTM_ADDR pConnAddr,
                                                  char *pszNetworkDeviceName )
{
    BCMXTM_STATUS bxStatus;
    XTM_CONNECTION *pConn ;
    XTM_INTERFACE_LINK_INFO Li ;
    PXTM_INTERFACE_LINK_INFO pOtherLi = NULL ;
    UINT32 ulPort, ulBondingPort = MAX_INTERFACES;
    UINT32 ulPortMask;
    UINT32 ulLogicalPort;
    ulLogicalPort = PORTMAP_TO_LOGICALPORT(pConnAddr->u.Conn.ulPortMask);
    //XtmOsPrintf("(%s)portmask[%u] ulLogicalPort[%u]\n",__FUNCTION__,pConnAddr->u.Conn.ulPortMask,ulLogicalPort);
    //Here incoming portmask is based on logical port map. Convert this to physical port map now.
    //pConnAddr->u.Vcc.ulPortMask = PORT_TO_PORTID(m_LogPhyPortmap[ulLogicalPort].PhysicalPortId);
    pConnAddr->u.Conn.ulPortMask = PORT_TO_PORTID(m_LogPhyPortmap[ulLogicalPort].PhysicalPortId);
    //XtmOsPrintf("(%s)m_LogPhyPortmap[%u] updated portmask[%u]\n",__FUNCTION__,(m_LogPhyPortmap[ulLogicalPort].PhysicalPortId),
    //                                                                          pConnAddr->u.Conn.ulPortMask); 

    UpdateConnAddrForBonding ( pConnAddr, XTM_ADD ) ;
    pConn = m_ConnTable.Get( pConnAddr );

    if( pConn )
    {
        UINT32 ulLinkMask = 0 ;
        
        pConn->CreateNetworkDevice( pszNetworkDeviceName, m_ulTxPafEnabled );

        /* Call connection to indicate the link is up where appropriate. */
        memset (&Li, 0, sizeof (XTM_INTERFACE_LINK_INFO)) ;
        
        ulPortMask = pConnAddr->u.Conn.ulPortMask ;
        
        for( ulPort = PHY_PORTID_0; ulPort < MAX_INTERFACES; ulPort++ )
        {
            if( ulPortMask & PORT_TO_PORTID(ulPort) )
            {
                PXTM_INTERFACE_LINK_INFO pLi = m_Interfaces[ulPort].GetLinkInfo();
                ulBondingPort = m_Interfaces[ulPort].GetBondingPortNum() ;
                if( pLi->ulLinkState == LINK_UP &&
                    ((pLi->ulLinkTrafficType == pConnAddr->ulTrafficType) ||
                     (pLi->ulLinkTrafficType == TRAFFIC_TYPE_PTM_RAW)) )
                {
                   ulLinkMask |= PORT_TO_PORTID(ulPort) ;
                   Li.ulLinkUsRate += pLi->ulLinkUsRate ;
                   Li.ulLinkDsRate += pLi->ulLinkDsRate ;
                   Li.ulLinkTrafficType = pLi->ulLinkTrafficType ;

                   if (ulBondingPort != MAX_INTERFACES)
                   {
                      pOtherLi = m_Interfaces[ulBondingPort].GetLinkInfo();
                      if (pOtherLi->ulLinkState == LINK_UP)
                         ulLinkMask |= PORT_TO_PORTID(ulBondingPort) ;
                      break ;   /* More than 2 ports in bonding not supported, so exiting */
                   }
                }
            }
        }

        if (ulLinkMask != 0)
        {
           Li.ulLinkState = LINK_UP ;
           //XtmOsPrintf("Calling SetLinkInfo 2\n");
           pConn->SetLinkInfo( ulLinkMask, &Li, ulBondingPort, pOtherLi, m_ulXTMLinkMode, m_ulXTMNitroMode) ;
        }
        bxStatus = XTMSTS_SUCCESS;
    }
    else
        bxStatus = XTMSTS_NOT_FOUND;

    UpdateConnAddrForBonding ( pConnAddr, XTM_REMOVE ) ;

    {
       /* Compile Out for Loopback */
       UINT32 ulLinkState = LINK_UP;
       ulPort = PORTID_TO_PORT(pConnAddr->u.Vcc.ulPortMask);

       if( ulPort < MAX_INTERFACES )
       {
          PXTM_INTERFACE_LINK_INFO pLi = m_Interfaces[ulPort].GetLinkInfo();
          ulLinkState = pLi->ulLinkState;
       }

       /* All external ports */
       if(bxStatus == XTMSTS_SUCCESS && ulLinkState == LINK_DOWN &&
          (((XP_REGS->ulTxUtopiaCfg & TXUTO_EXT_PORT_EN_MASK) == TXUTO_EXT_PORT_EN_MASK) ||
           ((XP_REGS->ulTxUtopiaCfg & TXUTO_SLV_PORT_EN_MASK) == TXUTO_SLV_PORT_EN_MASK)))
       {
          XTM_INTERFACE_LINK_INFO Li;

          Li.ulLinkState  = LINK_UP;
          Li.ulLinkUsRate = 54000000;
          Li.ulLinkDsRate = 99000000;
          Li.ulLinkTrafficType = pConnAddr->ulTrafficType ;

          ulPortMask = pConnAddr->u.Conn.ulPortMask ;

          for( ulPort = PHY_PORTID_0; ulPort < MAX_INTERFACES; ulPort++ )
          {
             if( ulPortMask & PORT_TO_PORTID(ulPort) )
             {
                XtmOsPrintf("CreateNetworkDevice: Force link up, port=%lu, traffictype=%lu\n",
                ulPort, Li.ulLinkTrafficType);
                BcmXtm_SetInterfaceLinkInfo( (ulPortMask & PORT_TO_PORTID(ulPort)), &Li );
             }
          }

          if( m_InitParms.bondConfig.sConfig.ptmBond == BC_PTM_BONDING_ENABLE) {

             Li.ulLinkState  = LINK_UP;
             Li.ulLinkUsRate = 54000000;
             Li.ulLinkDsRate = 99000000;
             Li.ulLinkTrafficType = pConnAddr->ulTrafficType ;

             XtmOsPrintf("CreateNetworkDevice: Force link up, port=%lu, traffictype=%lu\n",
                   ulPort, Li.ulLinkTrafficType);
             BcmXtm_SetInterfaceLinkInfo( 0x2, &Li );
          }
       }
    }

    return( bxStatus );
} /* CreateNetworkDevice */


/***************************************************************************
 * Function Name: DeleteNetworkDevice
 * Description  : Remove a bcmxtmrt network device.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_PROCESSOR::DeleteNetworkDevice( PXTM_ADDR pConnAddr )
{
    BCMXTM_STATUS bxStatus;
    XTM_CONNECTION *pConn ;

    //XtmOsPrintf ("xtm: DeleteNetworkDevice....\n") ;
    //XtmOsPrintf("(%s)portmask[%u]\n",__FUNCTION__,pConnAddr->u.Conn.ulPortMask);
    //Here incoming portmask is based on logical port map. Convert this to physical port map now.
    pConnAddr->u.Vcc.ulPortMask = PORT_TO_PORTID(m_LogPhyPortmap[PORTMAP_TO_LOGICALPORT(pConnAddr->u.Conn.ulPortMask)].PhysicalPortId);
    //XtmOsPrintf("(%s)m_LogPhyPortmap[%u] updated portmask[%u]\n",__FUNCTION__,(m_LogPhyPortmap[PORTID_TO_PORT(pConnAddr->u.Conn.ulPortMask)].PhysicalPortId),
    //                                                                          pConnAddr->u.Vcc.ulPortMask);  

    UpdateConnAddrForBonding ( pConnAddr, XTM_ADD ) ;

    pConn = m_ConnTable.Get( pConnAddr );

    if( pConn )
    {
        //XtmOsPrintf ("xtm: conn->DeleteNetworkDevice....\n") ;

        pConn->DeleteNetworkDevice();

        /* The connection's configuration was previously removed. */
        if( pConn->RemovePending() )
        {
            m_ConnTable.Remove( pConn );
            // delete pConn;
            pConn->Dxtor();
            XtmOsFree((char *)pConn);
            pConn = NULL;
        }
        bxStatus = XTMSTS_SUCCESS;
    }
    else
        bxStatus = XTMSTS_NOT_FOUND;

    UpdateConnAddrForBonding ( pConnAddr, XTM_REMOVE ) ;
    //XtmOsPrintf ("xtm: DeleteNetworkDevice Done....\n") ;
    return( bxStatus );
} /* DeleteNetworkDevice */

/***************************************************************************
 * Function Name: GetBondingInfo
 * Description  : Returns the bonding information for the system, if applicable.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_PROCESSOR::GetBondingInfo ( PXTM_BOND_INFO pBondInfo )
{
    BCMXTM_STATUS bxStatus;
    PXTM_INTERFACE_LINK_INFO pLinkInfo [MAX_BOND_PORTS] ;
    PXTM_INTERFACE_LINK_DELAY pLinkDelay [MAX_BOND_PORTS] ;
    UINT32 portSet[MAX_BOND_PORTS], ulPort, ulBondingPort, portIndex ;
    UINT32 aggregate;

    /* Currently, single latency - single group information is supported and
     * returned, if available.
     * BACP for PTM bonding is not supported yet.
     */
    memset (pBondInfo, 0, sizeof (XTM_BOND_INFO)) ;

    pBondInfo->u8MajorVersion = XTM_MAJ_VER ;
    pBondInfo->u8MinorVersion = XTM_MIN_VER ;
    pBondInfo->u8BuildVersion = XTM_BUILD_VER ;
    pBondInfo->ulTxPafEnabled = (m_ulTxPafEnabled==1 ? 1 : 0) ;

    portSet[0] = 0x0 ;
    ulPort = portSet[0] ;

    if ((m_InitParms.bondConfig.sConfig.ptmBond == BC_PTM_BONDING_ENABLE) ||
        (m_InitParms.bondConfig.sConfig.atmBond == BC_ATM_BONDING_ENABLE)) {

        portSet[1] = m_Interfaces[ulPort].GetBondingPortNum () ;
        ulBondingPort = portSet[1] ;

        pLinkInfo[0] = m_Interfaces [ulPort].GetLinkInfo () ;
        pLinkInfo[1] = m_Interfaces [ulBondingPort].GetLinkInfo () ;

        pLinkDelay[0] = m_Interfaces [ulPort].GetLinkDelay () ;
        pLinkDelay[1] = m_Interfaces [ulBondingPort].GetLinkDelay () ;

        if ((pLinkInfo[0]->ulLinkState == LINK_UP) ||
            (pLinkInfo[1]->ulLinkState == LINK_UP)) {
           pBondInfo->ulTrafficType = (pLinkInfo[0]->ulLinkTrafficType != TRAFFIC_TYPE_NOT_CONNECTED) ?
                                      pLinkInfo[0]->ulLinkTrafficType :
                                      pLinkInfo[1]->ulLinkTrafficType ;
           pBondInfo->ulBondProto   = (pBondInfo->ulTrafficType == TRAFFIC_TYPE_PTM_BONDED) ?
                                      BC_BOND_PROTO_G994_AGGR :
                                      BC_BOND_PROTO_ASM ;
           pBondInfo->ulNumGroups   = 0x1 ;
           /* Fill the group 1 details */
           pBondInfo->grpInfo[0].ulGroupId     = (pBondInfo->ulBondProto == BC_BOND_PROTO_ASM) ?
               m_AsmHandler.GetGroupId () : 0 ;
           for (portIndex = 0; portIndex < MAX_BOND_PORTS; portIndex++) {
              aggregate = 0;
              if(pBondInfo->ulBondProto == BC_BOND_PROTO_ASM)
              {
                 //ATM Bonding Get LineInfo of each lines.
                 m_AsmHandler.GetLineInfo(portIndex,
                       &pBondInfo->grpInfo[0].portInfo[portIndex].rcvdGrpId,
                       &pBondInfo->grpInfo[0].portInfo[portIndex].localTxLineSt,
                       &pBondInfo->grpInfo[0].portInfo[portIndex].localRxLineSt);
                 if(pBondInfo->grpInfo[0].ulGroupId == pBondInfo->grpInfo[0].portInfo[portIndex].rcvdGrpId)
                    aggregate = 1;
              }
              pBondInfo->grpInfo[0].portInfo [portIndex].ulInterfaceId = portSet[portIndex] ;

              pBondInfo->grpInfo[0].portInfo [portIndex].linkState     = pLinkInfo[portIndex]->ulLinkState ;

              pBondInfo->grpInfo[0].portInfo [portIndex].usRate        = pLinkInfo[portIndex]->ulLinkUsRate ;

              pBondInfo->grpInfo[0].portInfo [portIndex].dsRate        = pLinkInfo[portIndex]->ulLinkDsRate ;
              if(pBondInfo->ulBondProto == BC_BOND_PROTO_ASM) {
                 if(aggregate == 1)
                 {
                    pBondInfo->grpInfo[0].aggrUSRate     += pLinkInfo[portIndex]->ulLinkUsRate ;
                    pBondInfo->grpInfo[0].aggrDSRate     += pLinkInfo[portIndex]->ulLinkDsRate ;
                 }
              }
              else {
                 pBondInfo->grpInfo[0].aggrUSRate     += pLinkInfo[portIndex]->ulLinkUsRate ;
                 pBondInfo->grpInfo[0].aggrDSRate     += pLinkInfo[portIndex]->ulLinkDsRate ;
              }

              pBondInfo->grpInfo[0].portInfo [portIndex].usDelay       = pLinkDelay[portIndex]->ulLinkUsDelay ;
              if (pBondInfo->grpInfo[0].diffUSDelay > pLinkDelay[portIndex]->ulLinkUsDelay)
                 pBondInfo->grpInfo[0].diffUSDelay -= pLinkDelay[portIndex]->ulLinkUsDelay ;
              else
                 pBondInfo->grpInfo[0].diffUSDelay = pLinkDelay[portIndex]->ulLinkUsDelay -
                                                     pBondInfo->grpInfo[0].diffUSDelay ;


              pBondInfo->grpInfo[0].portInfo [portIndex].dsBondingDelay = pLinkDelay[portIndex]->ulLinkDsBondingDelay ;
           }

           if (pBondInfo->ulBondProto == BC_BOND_PROTO_ASM) {
               pBondInfo->grpInfo[0].dataStatus   = m_AsmHandler.GetDataStatus () ;
           }
           else
              pBondInfo->grpInfo[0].dataStatus   = m_Interfaces [ulPort].GetPortDataStatus() |
                                                   m_Interfaces [ulBondingPort].GetPortDataStatus() ;
        }
        else {
           /* Both the links are down */
           pBondInfo->ulTrafficType         = TRAFFIC_TYPE_NOT_CONNECTED ;
           pBondInfo->ulBondProto           = BC_BOND_PROTO_NONE ;
           pBondInfo->grpInfo[0].ulGroupId  = BONDING_INVALID_GROUP_ID ;
           pBondInfo->grpInfo[0].dataStatus = DATA_STATUS_DISABLED ;
        }
        bxStatus = XTMSTS_SUCCESS ;
    }
    else {
      UINT32 portmap = GetAvailLinkStatus();
      switch(portmap & 0xF)
      {
         case (1 << PHY_PORTID_3):
            ulPort = PHY_PORTID_3;
            pLinkInfo[0] = m_Interfaces [PHY_PORTID_3].GetLinkInfo () ;
            break;
         case (1 << PHY_PORTID_2):
            ulPort = PHY_PORTID_2;
            pLinkInfo[0] = m_Interfaces [PHY_PORTID_2].GetLinkInfo () ;
            break;
         case (1 << PHY_PORTID_1):
            ulPort = PHY_PORTID_1;
            pLinkInfo[0] = m_Interfaces [PHY_PORTID_1].GetLinkInfo () ;
            break;
         case (1 << PHY_PORTID_0):
         default:
            ulPort = PHY_PORTID_0;
            pLinkInfo[0] = m_Interfaces [PHY_PORTID_0].GetLinkInfo () ;
            break;
      }
      pBondInfo->ulBondProto = BC_BOND_PROTO_NONE;
      pBondInfo->ulNumGroups = 0;
      pBondInfo->grpInfo[0].portInfo[0].ulInterfaceId = ulPort;
      pBondInfo->grpInfo[0].portInfo[0].linkState = pLinkInfo[0]->ulLinkState;
      if (pLinkInfo[0]->ulLinkState == LINK_UP) {
           pBondInfo->ulTrafficType = pLinkInfo[0]->ulLinkTrafficType ;
         pBondInfo->grpInfo[0].portInfo[0].usRate = pLinkInfo[0]->ulLinkUsRate;
         pBondInfo->grpInfo[0].portInfo[0].dsRate = pLinkInfo[0]->ulLinkDsRate;
         pBondInfo->grpInfo[0].dataStatus = DATA_STATUS_ENABLED;
        }
        else {
           pBondInfo->ulTrafficType = TRAFFIC_TYPE_NOT_CONNECTED ;
         pBondInfo->grpInfo[0].portInfo[0].usRate = 0;
         pBondInfo->grpInfo[0].portInfo[0].dsRate = 0;
         pBondInfo->grpInfo[0].dataStatus = DATA_STATUS_DISABLED;
        }
        bxStatus = XTMSTS_NOT_SUPPORTED ;
    }

    return( bxStatus ) ;
} /* GetBondingInfo */

/***************************************************************************
 * Function Name: CheckTrafficCompatibility
 * Description  : Called to check if the traffic mode given is matching with
 *                the traffic mode configured or enabled.
 *                Trafifc Modes are Bonding (Dual line PTM/ATM) & Non-bonding 
 *                (Single line PTM/ATM).
 * Returns      : None.
 ***************************************************************************/
BCMXTM_STATUS XTM_PROCESSOR::CheckTrafficCompatibility ( UINT32 ulTrafficType )
{
   BCMXTM_STATUS bxStatus = XTMSTS_SUCCESS ;

   switch (ulTrafficType) {
      case TRAFFIC_TYPE_PTM_BONDED     :
         if (m_InitParms.bondConfig.sConfig.ptmBond != BC_PTM_BONDING_ENABLE)
            bxStatus = XTMSTS_PROTO_ERROR ;
         break ;
      case TRAFFIC_TYPE_ATM_BONDED     :
         if ( m_InitParms.bondConfig.sConfig.atmBond != BC_ATM_BONDING_ENABLE)
            bxStatus = XTMSTS_PROTO_ERROR ;
         break ;
      case TRAFFIC_TYPE_PTM            :
      case TRAFFIC_TYPE_PTM_RAW        :
         if ( m_InitParms.bondConfig.sConfig.ptmBond == BC_PTM_BONDING_ENABLE)
            bxStatus = XTMSTS_PROTO_ERROR ;
         break ;
      case TRAFFIC_TYPE_ATM            :
         if ( m_InitParms.bondConfig.sConfig.atmBond == BC_PTM_BONDING_ENABLE)
            bxStatus = XTMSTS_PROTO_ERROR ;
         break ;
      default                          :
         break ;
   }

   return (bxStatus) ;
} /* CheckTrafficCompatibility */


/***************************************************************************
 * Function Name: CheckAndSetIfLinkInfo
 * Description  : Called for 63268 as bonding requires rx/tx side to be configured 
 *                intelligently based on the other bonding pair's activeness.
 *                Where port 0 goes down, the entire traffic stops as the scheduler gets disabled on the
 *        port 0. We cannot disable the tx scheduler for atm/ptm bonding
 *        operating port until all the ports are down.
 *            
 * Returns      : None.
 ***************************************************************************/
BCMXTM_STATUS XTM_PROCESSOR::CheckAndSetIfLinkInfo ( UINT32 ulTrafficType, UINT32 ulPort,
             PXTM_INTERFACE_LINK_INFO pLinkInfo, UINT32 ulBondingPort,
             PXTM_INTERFACE_LINK_INFO pOtherLinkInfo)
{
   BCMXTM_STATUS bxStatus ;

   XtmOsPrintf ("bcmxtmcfg: ChkSILI Port = %x, BondingPort = %x \n", ulPort, ulBondingPort) ;

   if (ulBondingPort != MAX_INTERFACES) {

      if( pLinkInfo->ulLinkState == LINK_UP ) {

         bxStatus = m_Interfaces[ulPort].SetLinkInfo( pLinkInfo, XTM_CONFIGURE, 
               XTM_CONFIGURE, XTM_CONFIGURE, m_ulXTMLinkMode );

         /* Use the same link information (spoof for tx side config) */
         if ((ulPort != PHY_PORTID_0) && (pOtherLinkInfo->ulLinkState == LINK_DOWN))
            bxStatus = m_Interfaces[ulBondingPort].SetLinkInfo( pLinkInfo, XTM_NO_CONFIGURE, 
                  XTM_NO_CONFIGURE, XTM_CONFIGURE, m_ulXTMLinkMode );

      }
      else {
         /* Link down */
         /* Only for single latency bonding.
          * For dual latency bonding, we need to pair 0 & 2 for a group and
          * 1 & 3 for another group.
          * For single latency, 0 & 1 are paired. These are derived from
          * HW design/assumptions.
          */
         if ((ulPort == PHY_PORTID_0) && (pOtherLinkInfo->ulLinkState == LINK_UP)) {
            bxStatus = m_Interfaces[ulPort].SetLinkInfo( pLinkInfo, XTM_CONFIGURE, XTM_CONFIGURE, 
                  XTM_NO_CONFIGURE, m_ulXTMLinkMode );
         }
         else {
            bxStatus = m_Interfaces[ulPort].SetLinkInfo( pLinkInfo, XTM_CONFIGURE, XTM_CONFIGURE,
                  XTM_CONFIGURE, m_ulXTMLinkMode);
         }

      }

      if( pOtherLinkInfo->ulLinkState == LINK_UP )
         m_Interfaces[ulBondingPort].UpdateLinkDelay () ; 

      ReprogramRxPaf (ulTrafficType, ulPort, pLinkInfo, ulBondingPort, pOtherLinkInfo) ;
   }
   else {
           bxStatus = m_Interfaces[ulPort].SetLinkInfo( pLinkInfo, XTM_CONFIGURE, XTM_CONFIGURE, XTM_CONFIGURE, m_ulXTMLinkMode );
   }
      //XtmOsPrintf ("%s:Exit \n",__FUNCTION__) ;

   return (bxStatus) ;
} /* CheckAndSetIfLinkInfo */

/***************************************************************************
 * Function Name: GetAvailLinkStatus
 * Description  : Check and all the possible links (out of 2 or 4) for UP
 *                condition and return 0 if no links are UP and the link mask
 *                if the links are up.
 * Returns      : None.
 ***************************************************************************/
UINT32 XTM_PROCESSOR::GetAvailLinkStatus()
{
   int i ;
   UINT32                     ulLinkAvailStatus = 0x0 ;
   PXTM_INTERFACE_LINK_INFO   pOtherLinkInfo ;

   for( i = PHY_PORTID_0; i < MAX_INTERFACES; i++ ) {

      pOtherLinkInfo = m_Interfaces [i].GetLinkInfo () ;
      if (pOtherLinkInfo->ulLinkState == LINK_UP)
         ulLinkAvailStatus |= (0x1 << i) ;
   }

   return (ulLinkAvailStatus) ;
}

/***************************************************************************
 * Function Name: CheckAndResetSAR
 * Description  : Called when all the available link(s) are DOWN to reset SAR
 *                block so that it is ready and accommodate differen traffic
 *                types when the available link(s) come UP.
 * Returns      : None.
 ***************************************************************************/
void XTM_PROCESSOR::CheckAndResetSAR( UINT32 ulPortId,
    PXTM_INTERFACE_LINK_INFO pLinkInfo )
{
   int i ;

   if( pLinkInfo->ulLinkState == LINK_DOWN )
   {

      /* Check if any other links are UP, in which case, we should not do any
       * SAR reset operation & initalize.
       */
      if (!GetAvailLinkStatus ()) { /* No links are available/UP */
         m_AsmHandler.StopTimerThread();
         XtmOsStopTimer ((void *) XtmMonitorTimerCb);
#if defined(CONFIG_BCM963268)
         PERF->softResetB &= ~SOFT_RST_SAR;
         PERF->softResetB |= SOFT_RST_SAR;
#elif defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963381)
         /* Applicable for 63381, 631XX chipsets */
         /* power down the SAR block */
         //XtmOsDelay (500) ;
         //pmc_sar_power_reset(); //This does NOT work. HW Jira is filed.
         pmc_sar_soft_reset();
#elif defined(CONFIG_BCM963158)
         XP_REGS->ulSoftReset |= (RX_SOFT_RST | TX_SOFT_RST);
         XtmOsDelay(500);
         XP_REGS->ulSoftReset &= ~(RX_SOFT_RST | TX_SOFT_RST);
#endif          
         /* Uninitialize all interfaces. */
         for( i = PHY_PORTID_0; i < MAX_INTERFACES; i++ )
            m_Interfaces[i].Uninitialize();

         Initialize( NULL, NULL );
      }
   }

   return ;
}  /* CheckAndResetSAR */
