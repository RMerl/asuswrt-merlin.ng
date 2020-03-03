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
 * File Name  : xtmasmhandler.cpp
 *
 * Description: This file contains the implementation for the XTM ASM handler
 *              class.  
 *              This class handles sending an ASM traffic for bonding.
 *              It also handles the ASMs being recieved for further processing.
 ***************************************************************************/

#include "xtmcfgimpl.h"

//#define SID_MODE_8BIT

/* Globals */
static UINT32 bondMgmtValid = 0;
static void *pAsmTimerParm = NULL;

#if 1
#define RX_ASM_PRINT_DEBUG    0xFFFFFFFF
#define TX_ASM_PRINT_DEBUG    0xFFFFFFFF
#else
#define RX_ASM_PRINT_DEBUG    0x1
#define TX_ASM_PRINT_DEBUG    0x1
#endif

#define BONDING_STATE(state)    ((state == BND_IDLE) ? "DATA_IDLE" : ((state == BND_FORCE_RESET) \
                                 ? "DATA_FORCE_RESET" : "DATA_ACTIVE"))

/***************************************************************************
 * Function Name: XTM_ASM_HANDLER
 * Description  : Constructor for the XTM processor class.
 * Returns      : None.
 ***************************************************************************/
XTM_ASM_HANDLER::XTM_ASM_HANDLER( void )
{
   m_pfnXtmrtReq = NULL;
   m_ulRegistered = 0;

    /* Flag that we have neither initialized or uninitialized at this point */
    m_bInitialized = false;
    m_bUninitialized = false;

} /* XTM_ASM_HANDLER */


void XTM_ASM_HANDLER::initializeVars(void)
{
   m_pfnXtmrtReq = NULL;
   m_ulRegistered = 0;

    /* Flag that we have neither initialized or uninitialized at this point */
    m_bInitialized = false;
    m_bUninitialized = false;
}

/***************************************************************************
 * Function Name: ~XTM_ASM_HANDLER
 * Description  : Destructor for the XTM processor class.
 * Returns      : None.
 ***************************************************************************/
XTM_ASM_HANDLER::~XTM_ASM_HANDLER( void )
{
    Uninitialize();
} /* ~XTM_ASM_HANDLER */

void XTM_ASM_HANDLER::resetBondingGroup ()
{
   int i ;

   XtmOsPrintf ("bcmxtmcfg: resetBondingGroup \n") ;
   m_BondMgmt.knownMapping = 0 ;
   m_BondMgmt.bondingId = BONDING_INVALID_GROUP_ID ;
   m_BondMgmt.asmLastRxId = 0 ;
   m_BondMgmt.numberOfLines = 0 ;
   m_BondMgmt.nextLineToPoll = MAX_BONDED_LINES ;
   m_BondMgmt.remoteRxState = 0 ;
   m_BondMgmt.remoteTxState = 0 ;
   m_BondMgmt.resetState = BND_IDLE ;
   XtmOsPrintf ("bcmxtmcfg: Bonding State is %s \n", BONDING_STATE(BND_IDLE)) ;
   m_BondMgmt.vt = XtmOsGetTimeStamp () ;
   m_BondMgmt.refLink = ~0 ; //* Currently no ref link */
   memset (&m_BondMgmt.asmCell, 0, sizeof (Asm)) ;
   m_BondMgmt.asmCell.msgLen = 0x28;
   m_BondMgmt.asmCell.rxAsmStatus[0] = 0xC0 ;
#ifdef SID_MODE_8BIT
   m_BondMgmt.asmCell.msgType = ATMBOND_ASM_MESSAGE_TYPE_8BITSID ;
#else
   m_BondMgmt.asmCell.msgType = ATMBOND_ASM_MESSAGE_TYPE_12BITSID ;
#endif

   for (i=0; i<MAX_BONDED_LINES; i++) {
      mappingReset (i, 0);
   }
}

void XTM_ASM_HANDLER::mappingReset(UINT32 ulLogicalPort, int timeout)
{
   LineMapping *mappingp = &m_BondMgmt.mapping[ulLogicalPort];
   int *remoteIdToLocalp = m_BondMgmt.remoteIdToLocal;
   Asm *asmp = &m_BondMgmt.asmCell;
   LineInfo *linep = &m_BondMgmt.lines[ulLogicalPort];
   UINT32 knownMapping = m_BondMgmt.knownMapping;
   UINT32 newKnownMapping;

   /* remove the line from set of known mapping */
   newKnownMapping = knownMapping & ~(1<<ulLogicalPort);
   m_BondMgmt.knownMapping = newKnownMapping;

   if ((newKnownMapping == 0) && (knownMapping != 0))
   {
      /* last line in the bonding => we reset complete bonding group */ 
      resetBondingGroup () ;
      return;
   }

   /* remote Tx cannot be active .... so remove it from the reoredering queue */

   XtmOsPrintf ("bcmxtmcfg: tx %d disabled \n", ulLogicalPort) ;
// m_pInterfaces [ulLogicalPort].SetLinkDataStatus (DATA_STATUS_DISABLED) ;
   /* ToDo -  should we reset rx */

   /* if line already associated with a remote idx, reset it. */
   if (mappingp->localIdToRemoteId != -1) {

      UINT16 newLocalTx =  asmp->txLinkStatus[0];
      UINT16 newLocalRx =  asmp->rxLinkStatus[0];
      UINT32 shiftArg = 14-(mappingp->localIdToRemoteId<<1);

      remoteIdToLocalp[mappingp->localIdToRemoteId] = -1;
      newLocalRx = (newLocalRx & ~(LK_ST_MASK << shiftArg)) | (LK_ST_SHOULD_NOT_USE<< shiftArg);
      newLocalTx = (newLocalTx & ~(LK_ST_MASK << shiftArg)) | (LK_ST_SHOULD_NOT_USE<< shiftArg);
      linep->inUseRx = linep->inUseTx = 0;
      linep->localRxState = 0;
      linep->localTxState = 0;
      linep->groupId = 0;
      asmp->txLinkStatus[0] = newLocalTx;
      asmp->rxLinkStatus[0] = newLocalRx;
   }

   mappingp->localIdToRemoteId = -1;
   mappingp->requestedDelay = 0;
   mappingp->lastAsmTimestamp = 0;
   mappingp->receivedAsm = 0;
   mappingp->transmittedAsm = 0;
   mappingp->stateChanges = 0;
   mappingp->localStateChanges = 0;
}

void XTM_ASM_HANDLER::LineInfo_init(LineInfo *objp)
{
  objp->inUseRx = objp->inUseTx = 0;
  objp->asmReady = 0;
  objp->lastAsmElapsed = 0;
  objp->usecTxAsm = ~0;
  objp->localRxState = 0;
  objp->localTxState = 0;
  objp->groupId = 0;
}

int XTM_ASM_HANDLER::IsElapsedSinceLastAsm (LineMapping *mappingp)
{
  return (XtmOsGetTimeStamp () - mappingp->lastAsmTimestamp) ;
}

void XTM_ASM_HANDLER::PollLines ()
{
   PXTM_INTERFACE_LINK_INFO pLi ;

   int localId =  m_BondMgmt.nextLineToPoll ;
   if (localId != MAX_BONDED_LINES) {

        /* Is link interface pointer good? */
        if(m_pInterfaces == NULL)
        {
            XtmOsPrintf ("bcmxtmcfg: XTM_ASM_HANDLER::PollLines() failed due to uninialized m_pInterfaces[] array.\n") ;
            return;
        }

        /* Get link pointer from interface */
      pLi = m_pInterfaces[localId].GetLinkInfo() ;

      if (pLi->ulLinkState == LINK_UP) {

         /* in case the CPE link id is known, we monitor if we still receive
            ASM.
            In case no ASM has been received for a certain amount of time,
            we reset the mapping (which will unregister from the reordering queue)
            */

         if (m_BondMgmt.knownMapping &(1<<localId)) {

            if (IsElapsedSinceLastAsm(&m_BondMgmt.mapping[localId]) > ASM_DOWN_TIMEOUT) {

               m_BondMgmt.mapping[localId].localStateChanges++;
               /* reset bonding mapping */
               mappingReset (localId, 0);
            }
         }
      } /* if (pLi) */

      m_BondMgmt.nextLineToPoll++ ;

      if (m_BondMgmt.nextLineToPoll > MAX_BONDED_LINES)
         m_BondMgmt.nextLineToPoll = 0;
   } /* if (localId) */
}

/* return the tranmit ASM tx time */
void  XTM_ASM_HANDLER::LinkStateNotify (UINT32 ulLogicalPort)
{
   //UINT32    usecTx ;
   LineInfo  *objp ;
   LineMapping *mappingp ;
   PXTM_INTERFACE_LINK_INFO pLi;
   UINT32 usRate ;
   UINT16 trainingState ;


    /* Is link interface pointer good? */
    if(m_pInterfaces == NULL)
    {
        XtmOsPrintf ("bcmxtmcfg: XTM_ASM_HANDLER::PollLines() failed due to uninialized m_pInterfaces[] array.\n") ;
        return;
    }

    /* Use link pointer to get rates and state */
    pLi = m_pInterfaces [ulLogicalPort].GetLinkInfo() ;
   usRate = pLi->ulLinkUsRate ;
   trainingState = pLi->ulLinkState ;

   MapXtmOsPrintf ("bcmxtmcfg: LinkStateNotify %d  us %d  ds %d \n",
                ulLogicalPort, usRate, pLi->ulLinkDsRate) ;

   /* action to be done:
      => need a pointer to the global ASM in order
      to change it if required (can be changed at any time)
   */
   objp = &m_BondMgmt.lines [ulLogicalPort];
   mappingp = &m_BondMgmt.mapping[ulLogicalPort];
   mappingp->receivedAsm = 0 ;

   if (trainingState == LINK_DOWN)
   {
      /* in case there is no showtime, reset everything related to ASM */
      objp->usecTxAsm = ~0;
      objp->asmReady = 0;
      //XtmOsPrintf ("bcmxtmcfg: tx link %d disabled \n", ulLogicalPort) ;
      //m_pInterfaces[ulLogicalPort].SetLinkDataStatus (DATA_STATUS_DISABLED) ;
   }
   else
   {
      if (objp->usecTxAsm == (UINT32) ~0)
         objp->lastAsmElapsed = 0;
      //objp->usecTxAsm = MAX(100000,(((8*1050*150*1000)/*slow down asm rate *40*/ )/usRate)*53);
      //objp->usecTxAsm = MAX(100000,(((8*1050*40*1000)/*slow down asm rate *40*/ )/usRate)*53);
      //objp->usecTxAsm = 500000 ;
      objp->usecTxAsm = MAX(100000,(53*8*1050*150/*slow down asm rate 150 times of an ATM data*/ )/usRate);
      //usecTx = (53*8*1010*1000)/usRate ;
      //m_BondMgmt.timeInterval = MIN(usecTx, MAX_TIME_INTERVAL) ;
      //XtmOsPrintf ("bcmxtmcfg: tx link %d enabled \n", ulLogicalPort) ;
      //m_pInterfaces[ulLogicalPort].SetLinkDataStatus (DATA_STATUS_ENABLED) ;
   }
}

void XTM_ASM_HANDLER::TxAsm ()
{
   int i;
   UINT8 *payload ;
   PXTM_INTERFACE_LINK_INFO pLi ;

   i = MAX_BONDED_LINES ;

   /* Search line to be used for transmit .... */

   while (i--)
   {
      m_BondMgmt.lines[i].lastAsmElapsed += m_BondMgmt.timeInterval;
      if (m_BondMgmt.lines[i].lastAsmElapsed > m_BondMgmt.lines[i].usecTxAsm)
      {
         m_BondMgmt.lines[i].asmReady = 1;
         m_BondMgmt.lines[i].lastAsmElapsed -= m_BondMgmt.lines[i].usecTxAsm;
      }

      if (m_BondMgmt.lines[i].asmReady) {

         pLi = m_pInterfaces[i].GetLinkInfo () ;
         if (
             (pLi->ulLinkState == LINK_UP) && (pLi->ulLinkUsRate != 0)
               &&
             (m_BondMgmt.mapping[i].localIdToRemoteId != -1)
               &&
             (m_BondMgmt.resetState != BND_IDLE)
            ) {

            /* only send the ASM is the remote line Id is known */
            /* fill in the asm */

            m_BondMgmt.asmCell.asmId ++;
            m_BondMgmt.asmCell.txLinkId = m_BondMgmt.mapping[i].localIdToRemoteId;
            m_BondMgmt.asmCell.groupId = m_BondMgmt.bondingId;
            m_BondMgmt.asmCell.delay = 0; /* for timebeing do not yet request
                                             delay on the line (CPE will only do
                                             this for test purposes */
            m_BondMgmt.asmCell.appliedDelay = 0 ; /* We cannot support differential delay due to
                                                     HW bonding.
                                                     */
            {
               UINT32 ct;
               UINT32 dt;

               UINT8 tv1[TIMEOUT_VALUE_SIZE] ;
               XtmOsGetTimeOfDay(tv1,NULL);
               ct = XtmOsTimeInUsecs(tv1) ;
               dt = ct-m_BondMgmt.vt ;

               /* convert timestamp so that unit is 100usec */
               m_BondMgmt.asmCell.timestamp = dt/100;
            }

#ifdef SID_MODE_8BIT
            if (m_BondMgmt.asmCell.msgType == ATMBOND_ASM_MESSAGE_TYPE_12BITSID)
               m_BondMgmt.asmCell.msgType = ATMBOND_ASM_MESSAGE_TYPE_8BITSID ;
#endif
            /* CRC will be done by HW. */
            m_BondMgmt.asmCell.crc = 0 ;

            payload = (UINT8 *)&m_BondMgmt.asmCell ;

            if (!(m_BondMgmt.mapping[i].transmittedAsm % TX_ASM_PRINT_DEBUG) 
                  ||
                 (m_BondMgmt.printAsm == 0x1))
               printNewAsm ((Asm*) payload, "TxASM") ;

            if (SendAsmCell (i, payload) == XTMSTS_SUCCESS) {
               /* Increase transmitted cells counter */
               m_BondMgmt.mapping[i].transmittedAsm++;
            }

         } /* if remote mapping known */
         m_BondMgmt.lines[i].asmReady = 0;
      } /* if (asmready) */

   } /* (while i) */
}

void XTM_ASM_HANDLER::MgmtTimerCb (void *ulParm)
{
   XTM_ASM_HANDLER *pAsmHandle = (XTM_ASM_HANDLER *) pAsmTimerParm ;

   //XtmOsPrintf ("XTM: pAsmHandle = %lx \n", pAsmHandle) ;
   if (bondMgmtValid) {

           if (pAsmHandle != NULL) {
               pAsmHandle->TxAsm () ;
               pAsmHandle->PollLines () ;
               XtmOsStartTimer ((void *) pAsmHandle->MgmtTimerCb, 0, XTM_BOND_MGMT_TIMEOUT_MS) ;
           }
   }
}

void XTM_ASM_HANDLER::StopTimerThread(void)
{
   XtmOsStopTimer((void *) XTM_ASM_HANDLER::MgmtTimerCb);
}

/***************************************************************************
 * Function Name: SendCell
 * Description  : Sends an ASM cell.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_ASM_HANDLER::SendAsmCell ( UINT32 ulPhysPort, UINT8 *pAsmData)
{
   UINT8       ucCircuitType ;
   BCMXTM_STATUS bxStatus = XTMSTS_SUCCESS;
   XTM_CONNECTION *pConn = NULL;
   XTMRT_HANDLE hDev = INVALID_HANDLE;

   MapXtmOsPrintf ("SendAsmCell %d \n", ulPhysPort) ;

   switch (ulPhysPort) {
      case PHY_PORTID_0    :
         ucCircuitType = CTYPE_ASM_P0 ;
         break ;
      case PHY_PORTID_1    :
         ucCircuitType = CTYPE_ASM_P1 ;
         break ;
      case PHY_PORTID_2    :
         ucCircuitType = CTYPE_ASM_P2 ;
         break ;
      case PHY_PORTID_3    :
         ucCircuitType = CTYPE_ASM_P3 ;
         break ;
      default              :
         XtmOsPrintf ("bcmxtmcfg: Tx'ed ASM Unknown Port %d \n", ulPhysPort) ;
         bxStatus = XTMSTS_PARAMETER_ERROR ;
         goto _End ;
         break ;
   } /* switch (ulPhysPort) */

   /* ASM traffic.  Send it on the first connection that has a valid
    * bcmxtmrt driver instance handle and the port matches.
    */
   {
      XTM_ADDR Addr;
      UINT32 i = 0;

      while( (pConn = m_pConnTable->Enum( &i )) != NULL ) {
         pConn->GetAddr( &Addr );
         if(((Addr.u.Vcc.ulPortMask & PORT_TO_PORTID(ulPhysPort)) == PORT_TO_PORTID(ulPhysPort) )
               &&
            ((hDev = pConn->GetHandle()) != INVALID_HANDLE ))
         {
            bxStatus = XTMSTS_SUCCESS;
            break;
         }
      } /* while (pConn) */

      if (pConn == NULL)
         bxStatus = XTMSTS_NOT_FOUND;
   }

   if( bxStatus == XTMSTS_SUCCESS ) {
      XTMRT_CELL Cell;

      Cell.ConnAddr.u.Conn.ulPortMask = 0x1 ;  /* ToDo */
      Cell.ucCircuitType = ucCircuitType;
      memcpy (Cell.ucData, pAsmData, sizeof (Asm)) ;

      byteSwapAsmCell((Asm *)Cell.ucData);
      
      /* Send to ASM cell to the bcmxtmrt driver to send. */
      if(m_pfnXtmrtReq != NULL) {
           if ((*m_pfnXtmrtReq) (hDev, XTMRT_CMD_SEND_CELL, &Cell) != 0)
            bxStatus = XTMSTS_ERROR ;
        }
   } /* if (bxStatus) */

_End  :
      return( bxStatus ) ;
} /* SendAsmCell */

void XTM_ASM_HANDLER::byteSwapAsmCell(Asm *pAsm)
{
#if defined(CONFIG_CPU_LITTLE_ENDIAN)
   pAsm->rxLinkStatus[0] = BYTESWAP16(pAsm->rxLinkStatus[0]);
   pAsm->rxLinkStatus[1] = BYTESWAP16(pAsm->rxLinkStatus[1]);
   pAsm->rxLinkStatus[2] = BYTESWAP16(pAsm->rxLinkStatus[2]);
   pAsm->rxLinkStatus[3] = BYTESWAP16(pAsm->rxLinkStatus[3]);
   pAsm->txLinkStatus[0] = BYTESWAP16(pAsm->txLinkStatus[0]);
   pAsm->txLinkStatus[1] = BYTESWAP16(pAsm->txLinkStatus[1]);
   pAsm->txLinkStatus[2] = BYTESWAP16(pAsm->txLinkStatus[2]);
   pAsm->txLinkStatus[3] = BYTESWAP16(pAsm->txLinkStatus[3]);
   pAsm->groupId         = BYTESWAP16(pAsm->groupId);
   pAsm->timestamp       = BYTESWAP32(pAsm->timestamp);
   pAsm->delay           = BYTESWAP16(pAsm->delay);
   pAsm->appliedDelay    = BYTESWAP16(pAsm->appliedDelay);
   pAsm->reservedC[0]    = BYTESWAP16(pAsm->reservedC[0]);
   pAsm->reservedC[1]    = BYTESWAP16(pAsm->reservedC[1]);
   pAsm->reservedC[2]    = BYTESWAP16(pAsm->reservedC[2]);
   pAsm->msgLen          = BYTESWAP16(pAsm->msgLen);
   pAsm->crc             = BYTESWAP32(pAsm->crc);
#endif   
}

void XTM_ASM_HANDLER::printNewAsm (Asm *pAsm, const char* pMsg)
{
#if 0
   XtmOsPrintf ("------------------%s-------------------\n",pMsg) ;
   XtmOsPrintf ("ASM Message \n") ;
   XtmOsPrintf ("msgType        = 0x%x \n", pAsm->msgType) ;
   XtmOsPrintf ("asmId          = %d \n", pAsm->asmId) ;
   XtmOsPrintf ("txLinkId       = %d \n", pAsm->txLinkId) ;
   XtmOsPrintf ("txLinkNumber   = %d \n", pAsm->txLinkNumber) ;
   XtmOsPrintf ("rxLinkStatus   = 0x%x 0x%x \n", pAsm->rxLinkStatus[0], pAsm->rxLinkStatus[1]);
   XtmOsPrintf ("txLinkStatus   = 0x%x 0x%x \n", pAsm->txLinkStatus[0], pAsm->txLinkStatus[1]);
   XtmOsPrintf ("groupId        = %d \n", pAsm->groupId) ;
   XtmOsPrintf ("rxAsmStatus    = 0x%x 0x%x \n", pAsm->rxAsmStatus [0], pAsm->rxAsmStatus [1]) ;
   XtmOsPrintf ("groupLostCell  = %d \n", pAsm->groupLostCell) ;
   XtmOsPrintf ("timestamp      = 0x%x \n", pAsm->timestamp) ;
   XtmOsPrintf ("delay          = %d \n", pAsm->delay) ;
   XtmOsPrintf ("appliedDelay   = %d \n", pAsm->appliedDelay) ;
   XtmOsPrintf ("msgLen         = %d \n", pAsm->msgLen) ;
   XtmOsPrintf ("crc            = %x \n", pAsm->crc) ;
   XtmOsPrintf ("---------------------------------------\n") ;
#else
   XtmOsPrintf ("%s\n",pMsg) ;
   XtmOsPrintf ("mt = 0x%x \n", pAsm->msgType) ;
   XtmOsPrintf ("id = %d \n", pAsm->txLinkId) ;
   XtmOsPrintf ("rx = 0x%x 0x%x \n", pAsm->rxLinkStatus[0], pAsm->rxLinkStatus[1]);
   XtmOsPrintf ("tx = 0x%x 0x%x \n", pAsm->txLinkStatus[0], pAsm->txLinkStatus[1]);
   XtmOsPrintf ("groupId        = %d \n", pAsm->groupId) ;
#endif
}

UINT32 XTM_ASM_HANDLER::LineInfo_setGroupId(UINT32 ulLogicalPort, UINT32 groupId)
{
  LineInfo *objp = &m_BondMgmt.lines[ulLogicalPort] ;
  objp->groupId = groupId;
  return 0;
}

UINT32 XTM_ASM_HANDLER::LineInfo_getGroupId(UINT32 ulLogicalPort)
{
  LineInfo *objp = &m_BondMgmt.lines[ulLogicalPort] ;
  return objp->groupId;
}

UINT32 XTM_ASM_HANDLER::LineInfo_getUpdateLocalRxState (UINT32 ulLogicalPort, UINT32 rTx)
{
  int inUse = 0;
  UINT32 lRx, rRx;
  LineInfo *objp = &m_BondMgmt.lines[ulLogicalPort] ;
  PXTM_INTERFACE_LINK_INFO pLi ;
  
  /* check toggling local rx state .... only impacted by our local status.
   * The rx state may only be changed if the differential delay is small enough  !!!
   */

  if(m_pInterfaces == NULL)
  {
    lRx = LK_ST_SHOULD_NOT_USE;
  }
  else
  {
      pLi = m_pInterfaces [ulLogicalPort].GetLinkInfo() ;

      if (pLi->ulLinkState == LINK_DOWN || pLi->ulLinkDsRate == 0)
      {
        lRx = LK_ST_SHOULD_NOT_USE;
      }
      else
      {
        lRx = rTx;
         rRx = m_BondMgmt.remoteRxState ;
        if (rRx == LK_ST_SELECTED) {
            inUse = 1;
        }
      }
  }
  
  objp->inUseRx = inUse;
  objp->localRxState = lRx;
  return lRx;
    
}

UINT32 XTM_ASM_HANDLER::LineInfo_getUpdateLocalTxState (UINT32 ulLogicalPort, UINT32 rRx)
{
  int inUse = 0;
  int txState;
  LineInfo *objp = &m_BondMgmt.lines[ulLogicalPort] ;
  PXTM_INTERFACE_LINK_INFO pLi ;

  txState = LK_ST_SHOULD_NOT_USE;

  if(m_pInterfaces != NULL)
  {
      pLi = m_pInterfaces [ulLogicalPort].GetLinkInfo() ;
      /* modify the Tx state if necessary */
      if (pLi->ulLinkState == LINK_UP && pLi->ulLinkUsRate != 0)
      {
        if (rRx != LK_ST_SHOULD_NOT_USE)
        {
          txState = LK_ST_SELECTED;
          if (rRx == LK_ST_SELECTED)
          {
            inUse = 1;
          }
        }
        else
          txState = LK_ST_ACCEPTABLE;
      }
  }
  objp->inUseTx = inUse;
  objp->localTxState = txState;
  return txState;
}


/* Following function verify whether the complete mapping is known
   For this purpose, we need to know the total line numbers
*/
int XTM_ASM_HANDLER::mappingKnown ()
{
  int mappingKnown = 0 ;

  if (m_BondMgmt.numberOfLines != 0) {

     int i;

     if (m_BondingFallback == XTM_BONDING_FALLBACK_DISABLED) {
        for (i=0;i<m_BondMgmt.numberOfLines;i++) {

           if (m_BondMgmt.remoteIdToLocal[i] == -1) {
              goto _End ;
           }
        }
        mappingKnown = 1 ;
     }
     else {
        for (i=0;i<m_BondMgmt.numberOfLines;i++) {

           if (m_BondMgmt.remoteIdToLocal[i] != -1) {
              mappingKnown = 1 ;
              break ;
           }
        }
     }
  } /* Number of lines != 0 */
  else
     XtmOsPrintf ("bcmxtmcfg: atm bond mgmt number of lines is 0 \n") ;

_End :
  return mappingKnown ;
}

/*
  Reset FSM 
     wrong check   - xxxx  - xxxxx  => go error & reset
     correct check -  0xFF - active => reset fsm,  go idle
     correct check - !0xFF - active => no change
            
     correct check -  OxFF - error  => reset FSM, go idle
     correct check - !0xFF - error  time>timeout => go idle
     correct check - !0xFF - error  time<timeout => no change

     correct check -  0xFF - idle    => no change
     correct check - !0xFF - idle (+all mapping known) => go running
     correct check - !0xFF - idle (not all mapping known) => no change
*/

UINT32 XTM_ASM_HANDLER::UpdateResetFsm (int wrongCheck, int newMsgType, UINT32 ulLogicalPort)
{
  int resetState = m_BondMgmt.resetState ;

  MapXtmOsPrintf ("bcmxtmcfg: update Reset FSM newMsgType = %x \n", newMsgType) ;

  if (wrongCheck) {

    /* perform reset */
    resetBondingGroup () ;
    XtmOsGetTimeOfDay (m_BondMgmt.resetTimeout,NULL) ;
    XtmOsAddTimeOfDay (m_BondMgmt.resetTimeout, 5) ; /* stays at max 5 seconds in reset state */
    m_BondMgmt.asmCell.msgType = 0xFF ;
    resetState = BND_FORCE_RESET ;
  }
  else {
    if (resetState == BND_RUNNING) {
      if (newMsgType == 0xFF) {
        /* we receive an 0xFF, we just need to re-initialise, no need to ask
         * the other party to reinit */
        /* reset FSM go idle */
        /* perform reset */

        XtmOsPrintf ("bcmxtmcfg: ASM New MsgType=%d\n", newMsgType) ;
        resetBondingGroup ();
        resetState = BND_IDLE;
      }
    }
    else if  (resetState == BND_FORCE_RESET) {
      /* get ASM arrival time ... */
      UINT8 tv0[TIMEOUT_VALUE_SIZE] ;
      
      int timeoutPassed;
      XtmOsGetTimeOfDay(tv0,NULL);
      timeoutPassed = XtmOsIsTimerGreater (tv0,m_BondMgmt.resetTimeout);

      if ((newMsgType == 0xFF) || timeoutPassed) {
        /* note that timeout is a protection against misbehaving CO that do not
           send 0xFF when they get a "0xFF" */
        /* go idle */
        XtmOsPrintf ("bcmxtmcfg: ASM New MsgType=%d, TimeOutPassed=%d \n", newMsgType,
                     timeoutPassed) ;
        resetState = BND_IDLE; /* no need to reset as the reset take place
                                * when we enter force reset */
        /* normqlly in IDLE, we do not send asm. Just set correctly msgType in
         * case this feature is disabled */
#ifndef SID_MODE_8BIT
        m_BondMgmt.asmCell.msgType = ATMBOND_ASM_MESSAGE_TYPE_12BITSID ;
#else
        m_BondMgmt.asmCell.msgType = ATMBOND_ASM_MESSAGE_TYPE_8BITSID ;
#endif
      }
    }
    else if  (resetState == BND_IDLE) {

      if (mappingKnown() && (newMsgType != 0xFF)) {
        MapXtmOsPrintf ("bcmxtmcfg: State is %s ASM New MsgType=%d \n", BONDING_STATE(resetState), newMsgType);

        resetState = BND_RUNNING ;
        //XtmOsPrintf ("bcmxtmcfg: Bonding State is %s \n", BONDING_STATE(resetState)) ;
      }
    }
  } /* if good asm */

  if (resetState == BND_RUNNING) {

     if (newMsgType != m_BondMgmt.asmCell.msgType)
        XtmOsPrintf ("Port:%lu, CO SID type = %d, CPE SID type = %lu \n", ulLogicalPort, newMsgType, m_BondMgmt.asmCell.msgType) ;
     
#ifndef SID_MODE_8BIT
    UINT32 i,j ;

    /* Verify the Rx SID mapping and Tx SID mapping !!! */
    m_BondMgmt.asmCell.msgType = newMsgType ;

    /* Check type of sid and adapt */
    i = XP_REGS->ulTxSarCfg ;
    j = XP_REGS->ulRxSarCfg ;

    /* Check the message type and accordingly set the sid mask(s). */
    if ((UINT32) newMsgType != m_BondMgmt.sidType) {
       if (newMsgType == ATMBOND_ASM_MESSAGE_TYPE_8BITSID) {
          XtmOsPrintf ("bcmxtmcfg: SID MODE SET to 8 BIT MODE \n") ;
          i &= ~TXSARA_SID12_EN ;
          j &= ~RXSARA_SID12_EN ;
       }
       else {
          XtmOsPrintf ("bcmxtmcfg: SID MODE SET to 12 BIT MODE \n") ;
          i |= TXSARA_SID12_EN ;
          j |= RXSARA_SID12_EN ;
       }

       XP_REGS->ulTxSarCfg = i ;
       XP_REGS->ulRxSarCfg = j ;
       m_BondMgmt.sidType = newMsgType ;
    }
    else {
    }
#else 
     m_BondMgmt.asmCell.msgType = ATMBOND_ASM_MESSAGE_TYPE_8BITSID ;
#endif
  }

  if (m_BondMgmt.resetState != resetState)
     XtmOsPrintf ("bcmxtmcfg: Bonding State is %s \n", BONDING_STATE(resetState)) ;
  m_BondMgmt.resetState = resetState ;
  return resetState ;
}

void XTM_ASM_HANDLER::UpdateLineState (UINT32 ulLogicalPort)
{
  LineInfo *objp = &m_BondMgmt.lines[ulLogicalPort] ;

  if (!objp->inUseTx) {

    /* remove line from the active lines */
    XtmOsPrintf ("bcmxtmcfg: tx %d disabled \n", ulLogicalPort) ;
    //m_pInterfaces [ulLogicalPort].SetLinkDataStatus (DATA_STATUS_DISABLED) ;
  }
  else {
    XtmOsPrintf ("bcmxtmcfg: tx %d enabled \n", ulLogicalPort) ;
    //m_pInterfaces [ulLogicalPort].SetLinkDataStatus (DATA_STATUS_ENABLED) ;
  }
}

int XTM_ASM_HANDLER::ProcessReceivedAsm (Asm *pAsm, UINT32 ulLogicalPort)
{
   int txLinkId = pAsm->txLinkId & 0x1F ;
   int checkFailed = 0 ;

   MapXtmOsPrintf ("bcmxtmcfg: Rx ASM port : %d \n", ulLogicalPort) ;

   if (!(m_BondMgmt.mapping[ulLogicalPort].receivedAsm % RX_ASM_PRINT_DEBUG)
                ||
        (m_BondMgmt.printAsm == 0x1))
      printNewAsm (pAsm, "RxASM") ;

   if ((pAsm->msgType != 0xFF) && ((pAsm->msgType & ~1) != 0)) {
      /* unknown message type just discard the ASM */
      XtmOsPrintf ("bcmxtmcfg: ASM msgType %d received \n", pAsm->msgType) ;
      m_BondMgmt.mapping[ulLogicalPort].receivedUnKnowns++ ;
      *((UINT32 *) &m_BondMgmt.asmCell.rxAsmStatus [0]) |= (0x1<<(31-ulLogicalPort)) ;
      return 0;
   }

   /* Avoid line reset because no asm received on a line */
   m_BondMgmt.mapping[ulLogicalPort].lastAsmTimestamp = XtmOsGetTimeStamp() ;

   /* Increase ASM cell counter */
   m_BondMgmt.mapping[ulLogicalPort].receivedAsm++;

   *((UINT32 *) &m_BondMgmt.asmCell.rxAsmStatus [0]) &= ~(0x1<<(31-ulLogicalPort)) ;

   /* handle reception of msg type -> update the potential reset

      In case an old ASM with 0xFF is received. It will reset the bonding
      state. We think this is ok as it does not harm it just slow down the
      startup.

      Actions to be done:
      - learn bonding info
      + learn group id
      + learn link id

      - handle resetFSM
      + take into account a wrong check of groupid and link id
      (depending on the current state, it will lead to a reset or to a
      request to the CO to force a reset)
      + new msg type which could lead to a reset (just reset and go back
      to idle. In this case of starting up. When we are out of idle,
      it means that we have received an non 0xFF on each link. And
      consequently we should not oscillate between running and idle.

      - if new state is not "active", return and do not further handle the
      link states.
      */

   if ((m_BondMgmt.knownMapping & (1<<ulLogicalPort)) == 0)
   {
      /* if this is the first line to be mapped, we have to reset the bonding
       * group (reordering queue and tx sid) */

      if (m_BondMgmt.remoteIdToLocal[txLinkId] != -1) {

         XtmOsPrintf ("bcmxtmcfg: Asm Check Failed \n") ;
         checkFailed = 1;
      }
      else
      {
         if(m_BondMgmt.numberOfLines != 0)
         {
            //First time of second line of the bonding group.
            //Check the incoming group Id and compare it with
            //line0 of bonding group Id.
            if(m_BondMgmt.bondingId != pAsm->groupId)
            {
               //Different group Id
               XtmOsSendSysEvent (XTM_EVT_BONDING_GROUP_ID_MISMATCH) ;
               MapXtmOsPrintf("Bonding Group Id mismatch m_BondMgmt.bondingId:[%u] Port[%u] bondingId:[%u]\n",m_BondMgmt.bondingId,
                                                                                                           ulLogicalPort,
                                                                                                           pAsm->groupId);
               return 0;
            }
         }
         m_BondMgmt.knownMapping |= 0x1<<ulLogicalPort ;

         /* check range txLinkId */
         /* we receive the reset message => we learn the external id */
         m_BondMgmt.mapping [ulLogicalPort].localIdToRemoteId = txLinkId ;
         m_BondMgmt.remoteIdToLocal [txLinkId] = ulLogicalPort;
         LineInfo_setGroupId(ulLogicalPort,pAsm->groupId);

         /* first line passing here, will set the default tx/rx state in LINK_ID_KNOWN */
         if (m_BondMgmt.numberOfLines == 0)
         {
            UINT32 defaultLinkState ;

            m_BondMgmt.numberOfLines = pAsm->txLinkNumber;
            m_BondMgmt.nextLineToPoll = ulLogicalPort ;
            m_BondMgmt.bondingId = pAsm->groupId;
            m_BondMgmt.asmCell.txLinkNumber = pAsm->txLinkNumber;
            m_BondMgmt.asmCell.asmId = 0x0;
            /* reset local status */
            defaultLinkState = 0x5555 << ((8-m_BondMgmt.numberOfLines)<<1);
            m_BondMgmt.asmCell.rxLinkStatus[0] = defaultLinkState;
            m_BondMgmt.asmCell.txLinkStatus[0] = defaultLinkState;
            m_BondMgmt.asmLastRxId = pAsm->asmId;
         }
      }
   } /* if ((m_BondMgmt.knownMapping & (1<<ulLogicalPort)) == 0) */

   {
      /* keep in mind the requested delay */
      m_BondMgmt.mapping [ulLogicalPort].requestedDelay = pAsm->delay;
   }

   {
      int newResetFsmState;

      if ((m_BondMgmt.mapping [ulLogicalPort].localIdToRemoteId != txLinkId)
            || (m_BondMgmt.bondingId != pAsm->groupId)) {
         XtmOsPrintf ("bcmxtmcfg: checkFailed 2 \n") ;
         checkFailed = 1;
      }

      newResetFsmState = UpdateResetFsm (checkFailed, pAsm->msgType, ulLogicalPort) ;

      // In NonBonding mode or if the bonding mode is still being calculated, just log that an ASM was recieved but do no more
      if((m_pXtmAutoSense->PhyStatus() == PHYSTATUS_UNKNOWN) || (m_pXtmAutoSense->PhyStatus() == PHYSTATUS_ATM_SINGLELINE)) {
         // Log that an ASM has been recieved to the XTM_AUTOSENSE service
         m_pXtmAutoSense->BondingIndicationRx();

         // Discard ASM
         return 0;
      }

      if (newResetFsmState != BND_RUNNING) {
         MapXtmOsPrintf ("bcmxtmcfg: newResetFsmState is %d \n", newResetFsmState) ;
         return 0; /* no further processing */
      }
   }

   /* do not further handle out of sequence ASM  */
   if ((((UINT32)(pAsm->asmId-m_BondMgmt.asmLastRxId))&0xFF)>128)
   {
      MapXtmOsPrintf ("bcmxtmcfg: outofSequence ASM id %d last asm id %d\n",
                   pAsm->asmId, m_BondMgmt.asmLastRxId) ;
      return 1;
   }

   m_BondMgmt.asmLastRxId = pAsm->asmId ;

   /*
      - for each known line
      - handle remote rx state (check if our local tx state should be adapted)
      - for each active line, change local Tx state

      - handle remote tx state
      - adapt last seq Id
      (remote tx to disable => lastSeqId in resequencing should be ~0)
      */

   {
      int i;
      int linkNumber = m_BondMgmt.numberOfLines;
      int currentShift=(8-linkNumber)<<1;

      UINT32 remoteRxState = pAsm->rxLinkStatus[0];
      UINT32 remoteTxState = pAsm->txLinkStatus[0];
      UINT32 newLocalTx = 0;
      UINT32 newLocalRx = 0;
      UINT32 localRx = 0;
      UINT32 localTx = 0;

      /* keep remote tx state for debugging purposes */
      m_BondMgmt.remoteTxState = remoteTxState;
      m_BondMgmt.remoteRxState = remoteRxState;
      localRx = m_BondMgmt.asmCell.rxLinkStatus[0];
      localTx = m_BondMgmt.asmCell.txLinkStatus[0];

      remoteRxState >>=currentShift;
      remoteTxState >>=currentShift;
      localRx >>= currentShift;
      localTx >>= currentShift;

      for (i=linkNumber;i--;)
      {
         int localId = m_BondMgmt.remoteIdToLocal[i];
         if (localId != -1)
         {
            /* a link is only useable if at least some asm have been received on that link */
            UINT32 ITx, lRx;
            /* compute new local rx/tx state */
            ITx = LineInfo_getUpdateLocalTxState(localId, remoteRxState&LK_ST_MASK);
            newLocalTx |= ITx <<currentShift;
            if ((localTx & LK_ST_MASK) != (remoteRxState & LK_ST_MASK))
            {
               m_BondMgmt.mapping[localId].stateChanges++;
            }

            lRx = LineInfo_getUpdateLocalRxState(localId,remoteTxState&LK_ST_MASK); 
            newLocalRx |= lRx <<currentShift;
            if ((localRx & LK_ST_MASK) != (remoteTxState & LK_ST_MASK))
            {
               m_BondMgmt.mapping[localId].stateChanges++;
            }

            /* if line is not yet active, activate it/deactivate it */
            //UpdateLineState (localId) ;
         }
         else
         {
            newLocalTx |= LK_ST_SHOULD_NOT_USE<<currentShift;
            newLocalRx |= LK_ST_SHOULD_NOT_USE<<currentShift;
         }
         currentShift += 2;
         remoteRxState>>=2;
         remoteTxState>>=2;
         localRx >>= 2;
         localTx >>= 2;
      } /* for i .. LinkNumber */

      m_BondMgmt.asmCell.txLinkStatus[0] = newLocalTx;
      m_BondMgmt.asmCell.rxLinkStatus[0] = newLocalRx;
   }

   return 0 ;
}

UINT32 XTM_ASM_HANDLER::getSIDMode ( void)
{
   return (m_BondMgmt.sidType) ;
}

void XTM_ASM_HANDLER::setSIDMode ( void)
{
   UINT32 i, j ;

   i = XP_REGS->ulTxSarCfg ;
   j = XP_REGS->ulRxSarCfg ;

   if (m_BondMgmt.sidType == ATMBOND_ASM_MESSAGE_TYPE_12BITSID) {
      XtmOsPrintf ("bcmxtmcfg: SID MODE SET to 12 BIT MODE \n") ;
      i |= TXSARA_SID12_EN ;
      j |= RXSARA_SID12_EN ;
   }
   else  {
      XtmOsPrintf ("bcmxtmcfg: SID MODE SET to 8 BIT MODE \n") ;
      i &= ~TXSARA_SID12_EN ;
      j &= ~RXSARA_SID12_EN ;
   }

   XP_REGS->ulTxSarCfg = i ;
   XP_REGS->ulRxSarCfg = j ;
}

BOOL XTM_ASM_HANDLER::CheckASMActivity ( void)
{
   BOOL   ret = FALSE ;
   UINT32 i ;

   for (i=0;i<MAX_BONDED_LINES;i++) {
      XtmOsPrintf ("ASM: Line %d RxAsm %u \n", i, m_BondMgmt.mapping[i].receivedAsm) ;
      if (m_BondMgmt.mapping[i].receivedAsm > 0) {
         ret = TRUE ;
         break ;
      }
   }

   return (ret) ;
}

/***************************************************************************
 * Function Name: Initialize
 * Description  : Initializes the object.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_ASM_HANDLER::Initialize( XtmBondConfig bondConfig,
                                          FN_XTMRT_REQ pfnXtmrtReq,
                                 XTM_INTERFACE *pInterfaces,
                                 XTM_CONNECTION_TABLE *pConnTable,
                                 XTM_AUTOSENSE *pXtmAutoSense)
{
   int k ;
   BCMXTM_STATUS bxStatus = XTMSTS_SUCCESS;
   XTMRT_CELL_HDLR CellHdlr;


    /* Flag that uninitialization is no longer valid */
    m_bUninitialized = false;

    /* Do we need to initialize or has it been done already? */
    if(m_bInitialized) {
      XtmOsPrintf ("bcmxtmcfg: Out of sequence call to XTM_ASM_HANDLER::Initialize().  Recovering.\n") ;
        return( XTMSTS_SUCCESS ); /* Quit since there's nothing to do */
    }


   if( !m_ulRegistered )
   {
      m_pfnXtmrtReq = pfnXtmrtReq;
      m_BondCfg.uConfig = bondConfig.uConfig;
      m_BondingFallback = XTM_BONDING_FALLBACK_ENABLED ;
      m_pInterfaces = pInterfaces ;
      m_pConnTable = pConnTable ;
        m_pXtmAutoSense = pXtmAutoSense;

      XtmOsPrintf ("bcmxtmcfg: ATM Bonding configured in system. Fallback mode = %s \n",
                   (m_BondingFallback == XTM_BONDING_FALLBACK_ENABLED) ? "Enabled" : "Disabled");

      /* Initialize bondMgmt structure field definitions */
      memset (&m_BondMgmt, 0, sizeof (XtmBondMgmt));

      XtmOsPrintf ("bcmxtmcfg: Bonding State is %s \n", BONDING_STATE(BND_IDLE)) ;

      for (k=0;k<MAX_BONDED_LINES;k++) {
         
         /* Set the data tx statuses for the interfaces so no data can be
          * sent, until both the ends converge through the ASM messages.
          */
         MapXtmOsPrintf ("bcmxtmcfg: tx %d disabled \n", k) ;
         //m_pInterfaces[k].SetLinkDataStatus (DATA_STATUS_DISABLED) ;

         /* reset line context */
         LineInfo_init (&m_BondMgmt.lines[k]) ;

         /* preinit mapping information */
         m_BondMgmt.mapping [k].localIdToRemoteId = -1 ;
         m_BondMgmt.remoteIdToLocal [k] = -1 ;
      }

      m_BondMgmt.nextLineToPoll = MAX_BONDED_LINES ;
      //m_BondMgmt.timeInterval = MAX_TIME_INTERVAL ;
      m_BondMgmt.timeInterval = XTM_BOND_MGMT_TIMEOUT_MS*1000 ;

      m_BondMgmt.asmCell.msgLen = 0x28;
      m_BondMgmt.asmCell.rxAsmStatus[0] = 0xC0 ;

#ifndef SID_MODE_8BIT
      m_BondMgmt.sidType = ATMBOND_ASM_MESSAGE_TYPE_12BITSID;
#else
      m_BondMgmt.sidType = ATMBOND_ASM_MESSAGE_TYPE_8BITSID;
#endif      

      setSIDMode() ;

      /* Register an ASM cell handler with the bcmxtmrt network driver. */
      CellHdlr.ulCellHandlerType = CELL_HDLR_ASM;
      CellHdlr.pfnCellHandler = XTM_ASM_HANDLER::ReceiveAsmCb;
      CellHdlr.pContext = this;
      if( (*m_pfnXtmrtReq) (NULL, XTMRT_CMD_REGISTER_CELL_HANDLER,
               &CellHdlr) == 0 )
      {
         m_ulRegistered = 1;
      }
      else {
         XtmOsPrintf("bcmxtmcfg: Error registering ASM handler\n");
         bxStatus = XTMSTS_ERROR ;
      }

      pAsmTimerParm = (void *) this ;
      //XtmOsPrintf ("XTM: pAsmHandle = %lx \n", pAsmTimerParm) ;
      XtmOsStartTimer((void *) XTM_ASM_HANDLER::MgmtTimerCb, 0,
            XTM_BOND_MGMT_TIMEOUT_MS) ;
      bondMgmtValid = 1 ;
      m_BondMgmt.printAsm = 0 ;
      XtmOsPrintf ("bcmxtmcfg: ATM Bonding Mgmt Log Area = %pK \n", &m_BondMgmt.printAsm) ;
   }
   else
      bxStatus = XTMSTS_STATE_ERROR ;

    /* Flag that we're done initializing */
    m_bInitialized = true;

   return( bxStatus );
} /* Initialize */


/***************************************************************************
 * Function Name: Uninitialize
 * Description  : Uninitializes the object.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_ASM_HANDLER::Uninitialize( void )
{
   int k = 0;
   bondMgmtValid =  0 ;
   pAsmTimerParm = NULL ;

    /* We only need to uninitialize if we've been initialized but not uninitialized */
    if(!m_bInitialized || m_bUninitialized) {
      XtmOsPrintf ("bcmxtmcfg: Out of sequence call to XTM_ASM_HANDLER::Uninitialize().  Recovering.\n") ;
        return( XTMSTS_SUCCESS ); /* Quit since there's nothing to do */
    }

    /* Flag that initialization is no longer valid */
    m_bInitialized = false;

   if( m_ulRegistered )
   {
      XTMRT_CELL_HDLR CellHdlr;

      for (k=0;k<MAX_BONDED_LINES;k++) {

         /* Set the data tx statuses for the interfaces so no data can be
          * sent, until both the ends converge through the ASM messages.
          */

         MapXtmOsPrintf ("bcmxtmcfg: tx %d disabled \n", k) ;
         //m_pInterfaces[k].SetLinkDataStatus (DATA_STATUS_DISABLED) ;
      }

      /* Unregister the ASM cell handler with the bcmxtmrt network driver. */
      CellHdlr.ulCellHandlerType = CELL_HDLR_ASM;
      CellHdlr.pfnCellHandler = XTM_ASM_HANDLER::ReceiveAsmCb;
      CellHdlr.pContext = this;

        if(m_pfnXtmrtReq != NULL) {
            (*m_pfnXtmrtReq)(NULL, XTMRT_CMD_UNREGISTER_CELL_HANDLER, &CellHdlr);
        }

        /* Stop any pending timer events */
        XtmOsStopTimer((void *) XTM_ASM_HANDLER::MgmtTimerCb);

      m_pConnTable = NULL ;
      m_pInterfaces = NULL ;
      m_pfnXtmrtReq = NULL ;
      m_BondingFallback = XTM_BONDING_FALLBACK_DISABLED ;
      m_ulRegistered = 0;
   }

    /* Flag that we're done uninitializing */
    m_bUninitialized = true;

   return( XTMSTS_SUCCESS );
} /* Uninitialize */

/***************************************************************************
 * Function Name: GetGroupId
 * Description  : Retrieves the bonding group ID information.
 * Returns      : Group ID.
 ***************************************************************************/
UINT32 XTM_ASM_HANDLER::GetGroupId ( void )
{
    return (m_BondMgmt.bondingId) ;
} /* GetGroupId */

/***************************************************************************
 * Function Name: GetDataStatus
 * Description  : Retrieves the bonding group's data tx/rx status. 
 * Returns      : Data Status Information
 ***************************************************************************/
UINT32 XTM_ASM_HANDLER::GetDataStatus ( void )
{
    return (m_BondMgmt.resetState) ;
} /* GetDataStatus */

UINT32 XTM_ASM_HANDLER::GetLineInfo(UINT32 ulLogicalPort, UINT32 *groupId, int *txState, int *rxState)
{
   LineInfo *linep = &m_BondMgmt.lines[ulLogicalPort];
   *groupId = linep->groupId;
   *txState = linep->localTxState;
   *rxState = linep->localRxState;
   return 0;
}
/***************************************************************************
 * Function Name: ReceiveAsmCb
 * Description  : Processes a received ASM cell.
 * Returns      : 0 if successful, non-0 if not
 ***************************************************************************/
int XTM_ASM_HANDLER::ReceiveAsmCb( XTMRT_HANDLE hDev, UINT32 ulCommand,
    void *pParam, void *pContext )
{
   const UINT32 ulAtmHdrSize = 4; /* no HEC */
   XTM_ASM_HANDLER *pThis = (XTM_ASM_HANDLER *) pContext;
   PXTMRT_CELL pCell = (PXTMRT_CELL) pParam;
   UINT32 i, ulPhysPort;
   int status = 0 ;

   /* Remove ATM header. */
   for( i = 0; i < CELL_PAYLOAD_SIZE; i++ )
      pCell->ucData[i] = pCell->ucData[i + ulAtmHdrSize];

   switch (pCell->ucCircuitType) {
      case CTYPE_ASM_P0    :
         ulPhysPort = PHY_PORTID_0 ;
         break ;
      case CTYPE_ASM_P1    :
         ulPhysPort = PHY_PORTID_1 ;
         break ;
      case CTYPE_ASM_P2    :
         ulPhysPort = PHY_PORTID_2 ;
         break ;
      case CTYPE_ASM_P3    :
         ulPhysPort = PHY_PORTID_3 ;
         break ;
      default              :
         XtmOsPrintf ("bcmxtmcfg: Received ASM Unknown %d \n", pCell->ucCircuitType) ;
         status = -1 ;
         break ;
   }

   if (status == 0)
   {
      byteSwapAsmCell((Asm *)pCell->ucData);
      
      pThis->ProcessReceivedAsm ((Asm *) pCell->ucData, ulPhysPort) ;
   }

   return( status );
} /* ReceiveAsmCb */
