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
 * File Name  : xtmoamhandler.cpp (impl2)
 *
 * Description: This file contains the implementation for the XTM OAM handler
 *              class.  This class handles sending an ATM OAM F4 or OAM F5
 *              loopback request and waiting for the loopback response.
 *              It also handles responding to a received ATM OAM F4 or OAM F5
 *              loopback request.
 ***************************************************************************/

#include "xtmcfgimpl.h"

#define VCI_OAM_F4_SEGMENT                  3
#define VCI_OAM_F4_END_TO_END               4
#define OAM_TYPE_FUNCTION_BYTE_OFS          0
#define OAM_LB_INDICATION_BYTE_OFS          1
#define OAM_LB_CORRELATION_TAG_BYTE_OFS     2
#define OAM_LB_LOCATION_ID_BYTE_OFS         6
#define OAM_LB_SRC_ID_BYTE_OFS              22
#define OAM_LB_UNUSED_BYTE_OFS              38
#define OAM_LB_CRC_BYTE_OFS                 46
#define OAM_LB_CORRELATION_TAG_LEN          4
#define OAM_LB_LOCATION_ID_LEN              16
#define OAM_LB_SRC_ID_LEN                   16
#define OAM_LB_UNUSED_BYTE_LEN              8
#define OAM_LB_CRC_BYTE_LEN                 2
#define OAM_FAULT_MGMT_AIS                  0x10
#define OAM_FAULT_MGMT_RDI                  0x11
#define OAM_FAULT_MGMT_LB                   0x18
#define OAM_FAULT_MGMT_LB_REQUEST           1
#define OAM_FAULT_MGMT_LB_RESPONSE          0
#define OAM_LB_UNUSED_BYTE_DEFAULT          0x6a
#define OAM_RSP_RECEIVED                    0x80000000


/* Define static members */
XTM_PROCESSOR   *XTM_OAM_HANDLER::m_pXtmProcessor;  /* Pointer to parent XTM_PROCESSOR object */
    /* For the 63138 and 63148, implement a workaround to strip bytes and
       allow OAM traffic due to JIRA HW63138-12 */
#if defined (CONFIG_BCM963138) || defined (CONFIG_BCM963148)
    bool XTM_OAM_HANDLER::m_bStripByteWorkaroundActive;
    UINT32 XTM_OAM_HANDLER::m_pulSavedRxVcamRxRamReg[XP_MAX_CONNS];
    UINT32 XTM_OAM_HANDLER::m_ulStripByteWorkaroundTimeStart;
#endif

/***************************************************************************
 * Function Name: XTM_OAM_HANDLER
 * Description  : Constructor for the XTM processor class.
 * Returns      : None.
 ***************************************************************************/
XTM_OAM_HANDLER::XTM_OAM_HANDLER( void )
{
   XtmOsPrintf("%s:Enter\n",__FUNCTION__);
    m_pfnXtmrtReq = NULL;
    m_ulRegistered = 0;
    memset(m_PendingOamReqs, 0x00, sizeof(m_PendingOamReqs));
    memset(m_ulRspTimes, 0x00, sizeof(m_ulRspTimes));

    /* In 63138 and 63148, implement strip byte workaround */
#if defined (CONFIG_BCM963138) || defined (CONFIG_BCM963148)
    m_bStripByteWorkaroundActive = false;
    memset(m_pulSavedRxVcamRxRamReg, 0x00, sizeof(m_pulSavedRxVcamRxRamReg));
    m_ulStripByteWorkaroundTimeStart = 0;
#endif

} /* XTM_OAM_HANDLER */


/***************************************************************************
 * Function Name: ~XTM_OAM_HANDLER
 * Description  : Destructor for the XTM processor class.
 * Returns      : None.
 ***************************************************************************/
XTM_OAM_HANDLER::~XTM_OAM_HANDLER( void )
{
   XtmOsPrintf("%s:Enter\n",__FUNCTION__);
    Uninitialize();
} /* ~XTM_OAM_HANDLER */


/***************************************************************************
 * Function Name: Initialize
 * Description  : Initializes the object.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_OAM_HANDLER::Initialize( FN_XTMRT_REQ pfnXtmrtReq, XTM_PROCESSOR *pXtmProcessor )
{
    BCMXTM_STATUS bxStatus = XTMSTS_SUCCESS;
    XTMRT_CELL_HDLR CellHdlr;
    //XtmOsPrintf("%s:Enter\n",__FUNCTION__);
    //Following Initialization is required as XTM_PROCESSOR object is not
    //created by new instead alloc
    m_pfnXtmrtReq = NULL;
    m_ulRegistered = 0;
    memset(m_PendingOamReqs, 0x00, sizeof(m_PendingOamReqs));
    memset(m_ulRspTimes, 0x00, sizeof(m_ulRspTimes));
#if defined (CONFIG_BCM963138) || defined (CONFIG_BCM963148)
    m_bStripByteWorkaroundActive = false;
    memset(m_pulSavedRxVcamRxRamReg, 0x00, sizeof(m_pulSavedRxVcamRxRamReg));
    m_ulStripByteWorkaroundTimeStart = 0;
#endif 
    m_pfnXtmrtReq = pfnXtmrtReq;
    m_pXtmProcessor = pXtmProcessor; /* Parent XTM_PROCESSOR object */

    if( m_ulRegistered == 0)
    {
        /* Register an OAM cell handler with the bcmxtmrt network driver. */
        CellHdlr.ulCellHandlerType = CELL_HDLR_OAM;
        CellHdlr.pfnCellHandler = XTM_OAM_HANDLER::ReceiveOamCb;
        CellHdlr.pContext = this;
        if( (*m_pfnXtmrtReq) (NULL, XTMRT_CMD_REGISTER_CELL_HANDLER,
            &CellHdlr) == 0 )
        {
            m_ulRegistered = 1;
        }
        else
            XtmOsPrintf("bcmxtmcfg: Error registering OAM handler\n");
    }
    else
        bxStatus = XTMSTS_STATE_ERROR;

    return( bxStatus );
} /* Initialize */


/***************************************************************************
 * Function Name: Uninitialize
 * Description  : Uninitializes the object.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_OAM_HANDLER::Uninitialize( void )
{
   XtmOsPrintf("%s:Enter\n",__FUNCTION__);
    if( m_ulRegistered )
    {
        XTMRT_CELL_HDLR CellHdlr;

        /* Unregister the OAM cell handler with the bcmxtmrt network driver. */
        CellHdlr.ulCellHandlerType = CELL_HDLR_OAM;
        CellHdlr.pfnCellHandler = XTM_OAM_HANDLER::ReceiveOamCb;
        CellHdlr.pContext = this;
        (*m_pfnXtmrtReq)(NULL, XTMRT_CMD_UNREGISTER_CELL_HANDLER, &CellHdlr);
        m_ulRegistered = 0;
    }

    return( XTMSTS_SUCCESS );
} /* Uninitialize */


/***************************************************************************
 * Function Name: SendCell
 * Description  : Sends an OAM cell.
 * Returns      : XTMSTS_SUCCESS if successful or error status.
 ***************************************************************************/
BCMXTM_STATUS XTM_OAM_HANDLER::SendCell( PXTM_ADDR pConnAddr, PXTM_ADDR pInternalConnAddr,
    PXTM_OAM_CELL_INFO pOamCellInfo, XTM_CONNECTION_TABLE *pConnTable )
{
    BCMXTM_STATUS bxStatus = XTMSTS_PARAMETER_ERROR;
    XTM_CONNECTION *pConn = NULL;
    XTMRT_HANDLE hDev = INVALID_HANDLE;
    PXTM_ADDR pReqAddr = NULL;
#if defined(CONFIG_ARM64)
    UINT32 *pulRspTime = NULL;
#else
    unsigned long *pulRspTime = NULL;
#endif
    UINT32 i;
    UINT8 ucCircuitType = pOamCellInfo->ucCircuitType;
   XtmOsPrintf("%s:Enter\n",__FUNCTION__);

    /* Validate the OAM parameters and find the bcmxtmrt driver instance to
     * send the OAM cell to.
     */

    if( ((pInternalConnAddr->ulTrafficType & TRAFFIC_TYPE_ATM_MASK) ==
            TRAFFIC_TYPE_ATM) &&
        (ucCircuitType == CTYPE_OAM_F5_SEGMENT ||
         ucCircuitType == CTYPE_OAM_F5_END_TO_END) )
    {
        /* OAM F5 cell to send. Get the bcmxtmrt driver instance handle. */
        if( (pConn = pConnTable->GetForPortId( pInternalConnAddr )) != NULL )
        {
            if( (hDev = pConn->GetHandle()) != INVALID_HANDLE )
                bxStatus = XTMSTS_SUCCESS;
        }
    }
    else
    {
        if( ((pInternalConnAddr->ulTrafficType & TRAFFIC_TYPE_ATM_MASK) ==
                TRAFFIC_TYPE_ATM) &&
            ((ucCircuitType == CTYPE_OAM_F4_SEGMENT &&
              pInternalConnAddr->u.Vcc.usVci == VCI_OAM_F4_SEGMENT) ||
             (ucCircuitType == CTYPE_OAM_F4_END_TO_END &&
              pInternalConnAddr->u.Vcc.usVci == VCI_OAM_F4_END_TO_END)) )
        {
            /* OAM F4 cell.  Send it on the first connection that has a valid
             * bcmxtmrt driver instance handle and the port matches.
             */
            XTM_ADDR Addr;
            i = 0;

            XP_REGS->ulTxOamCfg &=
                ~(TXOAM_F4_SEG_VPI_MASK | TXOAM_F4_E2E_VPI_MASK);
            XP_REGS->ulRxOamCfg &=
                ~(RXOAM_F4_SEG_VPI_MASK | RXOAM_F4_E2E_VPI_MASK);
            XP_REGS->ulTxOamCfg |=
                (pInternalConnAddr->u.Vcc.usVpi << 8) | pInternalConnAddr->u.Vcc.usVpi;
            XP_REGS->ulRxOamCfg |=
                (pInternalConnAddr->u.Vcc.usVpi << 8) | pInternalConnAddr->u.Vcc.usVpi;

            while( (pConn = pConnTable->Enum( &i )) != NULL )
            {
                pConn->GetAddr( &Addr );
                Addr.u.Vcc.ulPortMask &= pInternalConnAddr->u.Vcc.ulPortMask;
                if( pInternalConnAddr->u.Vcc.ulPortMask == Addr.u.Vcc.ulPortMask &&
                    (hDev = pConn->GetHandle()) != INVALID_HANDLE )
                {
                    bxStatus = XTMSTS_SUCCESS;
                    break;
                }
            }
        }
    }

    if( bxStatus == XTMSTS_SUCCESS )
    {
        XTMRT_CELL Cell;
        UINT8 *pData = Cell.ucData;

        memcpy(&Cell.ConnAddr, pConnAddr, sizeof(XTM_ADDR));
        Cell.ucCircuitType = ucCircuitType;

        pData[OAM_TYPE_FUNCTION_BYTE_OFS] = OAM_FAULT_MGMT_LB;
        pData[OAM_LB_INDICATION_BYTE_OFS] = OAM_FAULT_MGMT_LB_REQUEST;
        memset( &pData[OAM_LB_CORRELATION_TAG_BYTE_OFS], 0xbc, sizeof(int) );
        if( ucCircuitType == CTYPE_OAM_F5_END_TO_END ||
            ucCircuitType == CTYPE_OAM_F4_END_TO_END )
        {
            /* 16 bytes location ID and 16 bytes source ID */
            memset( &pData[OAM_LB_LOCATION_ID_BYTE_OFS], 0xff,
                OAM_LB_LOCATION_ID_LEN + OAM_LB_SRC_ID_LEN ); 
        }
        else
        {
            /* for segment one, we need to have NODE ID  agreement between
             * our modem and the network nodes; for now, I just assume
             * we are node src_id_3-0 now and the other end of the segment is
             * locationId_3-0
             */
            memset(&pData[OAM_LB_LOCATION_ID_BYTE_OFS], 0xff, sizeof(int));
            memset(&pData[OAM_LB_LOCATION_ID_BYTE_OFS+4], 0xff, sizeof(int));
            memset(&pData[OAM_LB_LOCATION_ID_BYTE_OFS+8], 0xff, sizeof(int));
            memset(&pData[OAM_LB_LOCATION_ID_BYTE_OFS+12],0xff, sizeof(int));

            memset(&pData[OAM_LB_SRC_ID_BYTE_OFS], 0xff, sizeof(int));
            memset(&pData[OAM_LB_SRC_ID_BYTE_OFS+4], 0xff, sizeof(int));
            memset(&pData[OAM_LB_SRC_ID_BYTE_OFS+8], 0xff, sizeof(int));
            memset(&pData[OAM_LB_SRC_ID_BYTE_OFS+12], 0xff, sizeof(int));
        }

        memset( &pData[OAM_LB_UNUSED_BYTE_OFS], OAM_LB_UNUSED_BYTE_DEFAULT,
            OAM_LB_UNUSED_BYTE_LEN );
        memset( &pData[OAM_LB_CRC_BYTE_OFS], 0, OAM_LB_CRC_BYTE_LEN );

        /* Save OAM circuit type and VCC address to match with response.
         * Save the OAM circuit type in the "ulTrafficType" field.
         */
        for( i = 0; i < sizeof(m_PendingOamReqs) / sizeof(XTM_ADDR); i++ )
        {
            if( m_PendingOamReqs[i].ulTrafficType == 0 )
            {
                pReqAddr = &m_PendingOamReqs[i];
                pulRspTime = &m_ulRspTimes[i];
                break;
            }
        }

        if( pReqAddr )
        {
            UINT32 ulTotalTime = 0;
            UINT32 ulIterations = 0;
            UINT32 ulTimeout = (pOamCellInfo->ulTimeout) ?
                pOamCellInfo->ulTimeout : 1;

            pOamCellInfo->ulSent = pOamCellInfo->ulReceived = 0;
            pOamCellInfo->ulMinRspTime = ulTimeout;
            for( i = 0; i < pOamCellInfo->ulRepetition; i++ )
            {
                memcpy(pReqAddr, pConnAddr, sizeof(XTM_ADDR));
                pReqAddr->ulTrafficType = ucCircuitType;
                *pulRspTime = XtmOsTickGet();

#if defined (CONFIG_BCM963138) || defined (CONFIG_BCM963148)
                /* Enable byte strip workaround if it is in use */
                EnableStripByteWorkaround();
#endif


                /* Send to OAM cell to the bcmxtmrt driver to send. */
                if( (*m_pfnXtmrtReq) (hDev, XTMRT_CMD_SEND_CELL, &Cell) == 0 )
                {
                    /* Wait for the corresponding OAM response. */
                    const UINT32 ulOamIntervalMs = 100;

                    pOamCellInfo->ulSent++;
                    bxStatus = XTMSTS_TIMEOUT;
                    ulIterations = ulTimeout / ulOamIntervalMs;
                    while( ulIterations-- )
                    {
                        if( (pReqAddr->ulTrafficType & OAM_RSP_RECEIVED) != 0 )
                        {
                            pOamCellInfo->ulReceived++;
                            ulTotalTime += *pulRspTime;
                            if( *pulRspTime < pOamCellInfo->ulMinRspTime )
                                pOamCellInfo->ulMinRspTime = *pulRspTime;
                            if( *pulRspTime > pOamCellInfo->ulMaxRspTime )
                                pOamCellInfo->ulMaxRspTime = *pulRspTime;
                            bxStatus = XTMSTS_SUCCESS;
                            break;
                        }

                        XtmOsDelay( ulOamIntervalMs );
                    }
                    pReqAddr->ulTrafficType = 0;

                    if( bxStatus == XTMSTS_TIMEOUT )
                    {
                        XtmOsPrintf("bcmxtmcfg: OAM loopback response not "
                            "received on VCC %d.%d.%d\n",
                            pConnAddr->u.Vcc.ulPortMask,
                            pConnAddr->u.Vcc.usVpi, pConnAddr->u.Vcc.usVci);

#if defined (CONFIG_BCM963138) || defined (CONFIG_BCM963148)
                        /* Disable byte strip workaround if we timed out and
                           it is in use */
                        DisableStripByteWorkaround();
#endif
                    }
                }
                else
                {
                    if( pReqAddr )
                        pReqAddr->ulTrafficType = 0;
                    bxStatus = XTMSTS_ERROR;
                    XtmOsPrintf("bcmxtmcfg: OAM Send Cell Failed \n") ;
                }
            }

            if( pOamCellInfo->ulReceived )
            {
                bxStatus = XTMSTS_SUCCESS;
                pOamCellInfo->ulAvgRspTime =
                    ulTotalTime / pOamCellInfo->ulReceived;
            }
            else
            {
                bxStatus = XTMSTS_TIMEOUT;
                pOamCellInfo->ulAvgRspTime = 0;
                pOamCellInfo->ulMinRspTime = 0;
            }
        }
        else
            bxStatus = XTMSTS_RESOURCE_ERROR;
    }

    return( bxStatus );
} /* SendCell */


/***************************************************************************
 * Function Name: ReceiveOamCb
 * Description  : Processes a received OAM cell.
 * Returns      : 0 if successful, non-0 if not
 ***************************************************************************/
int XTM_OAM_HANDLER::ReceiveOamCb( XTMRT_HANDLE hDev, UINT32 ulCommand,
    void *pParam, void *pContext )
{
    const UINT32 ulAtmHdrSize = 4; /* no HEC */
    XTM_OAM_HANDLER *pThis = (XTM_OAM_HANDLER *) pContext;
    PXTMRT_CELL pCell = (PXTMRT_CELL) pParam;
    UINT8 *pData = pCell->ucData;
    PXTM_ADDR pConnAddr = &pCell->ConnAddr;
    UINT32 i;
    bool bValidOamRxd = true;  /* Flag when a valid OAM was received */
   XtmOsPrintf("%s:Enter\n",__FUNCTION__);

    /* Remove ATM header. */
    for( i = 0; i < CELL_PAYLOAD_SIZE; i++ )
        pCell->ucData[i] = pCell->ucData[i + ulAtmHdrSize];

    pConnAddr->ulTrafficType = pCell->ucCircuitType;
    switch( pData[OAM_TYPE_FUNCTION_BYTE_OFS] )
    {
    case OAM_FAULT_MGMT_LB:
        if( pData[OAM_LB_INDICATION_BYTE_OFS] == OAM_FAULT_MGMT_LB_RESPONSE )
        {
            /* An OAM loopback response has been received.  Try to find the
             * associated request.
             */
            UINT32 ulSize = sizeof(pThis->m_PendingOamReqs) / sizeof(XTM_ADDR);
            for( i = 0; i < ulSize; i++ )
            {
                if(XTM_ADDR_CMP(&pThis->m_PendingOamReqs[i], pConnAddr))
                {
                    /* Found. */
                    XtmOsPrintf ("bcmxtmcfg : OAM LB Response Received vpi %d vci %d \n",
                                  pConnAddr->u.Vcc.usVpi, pConnAddr->u.Vcc.usVci) ;
                    pThis->m_ulRspTimes[i] =
                        XtmOsTickGet() - pThis->m_ulRspTimes[i];
                    pThis->m_PendingOamReqs[i].ulTrafficType|=OAM_RSP_RECEIVED;
                    break;
                }
            }
        }
        else
        {
            /* Send an OAM loopback response. */
            pData[OAM_LB_INDICATION_BYTE_OFS] = OAM_FAULT_MGMT_LB_RESPONSE;
            memset( &pData[OAM_LB_CRC_BYTE_OFS], 0, OAM_LB_CRC_BYTE_LEN );

            if( pCell->ucCircuitType == CTYPE_OAM_F4_SEGMENT ||
                pCell->ucCircuitType == CTYPE_OAM_F4_END_TO_END )
            {
                XP_REGS->ulTxOamCfg &=
                    ~(TXOAM_F4_SEG_VPI_MASK | TXOAM_F4_E2E_VPI_MASK);
                XP_REGS->ulRxOamCfg &=
                    ~(RXOAM_F4_SEG_VPI_MASK | RXOAM_F4_E2E_VPI_MASK);
                XP_REGS->ulTxOamCfg |=
                    (pConnAddr->u.Vcc.usVpi << 8) | pConnAddr->u.Vcc.usVpi;
                XP_REGS->ulRxOamCfg |=
                    (pConnAddr->u.Vcc.usVpi << 8) | pConnAddr->u.Vcc.usVpi;
            }

            XtmOsPrintf ("bcmxtmcfg : OAM LB Response Sent vpi %d vci %d \n",
                                  pConnAddr->u.Vcc.usVpi, pConnAddr->u.Vcc.usVci) ;
            (*pThis->m_pfnXtmrtReq) (hDev, XTMRT_CMD_SEND_CELL, pCell);
        }
        break;

    case OAM_FAULT_MGMT_AIS:
        /* Send an OAM RDI. */
        pData[OAM_TYPE_FUNCTION_BYTE_OFS] = OAM_FAULT_MGMT_RDI;
        memset( &pData[OAM_LB_CRC_BYTE_OFS], 0, OAM_LB_CRC_BYTE_LEN );

        if( pCell->ucCircuitType == CTYPE_OAM_F4_SEGMENT ||
            pCell->ucCircuitType == CTYPE_OAM_F4_END_TO_END )
        {
            XP_REGS->ulTxOamCfg &=
                ~(TXOAM_F4_SEG_VPI_MASK | TXOAM_F4_E2E_VPI_MASK);
            XP_REGS->ulRxOamCfg &=
                ~(RXOAM_F4_SEG_VPI_MASK | RXOAM_F4_E2E_VPI_MASK);
            XP_REGS->ulTxOamCfg |=
                (pConnAddr->u.Vcc.usVpi << 8) | pConnAddr->u.Vcc.usVpi;
            XP_REGS->ulRxOamCfg |=
                (pConnAddr->u.Vcc.usVpi << 8) | pConnAddr->u.Vcc.usVpi;
        }

        (*pThis->m_pfnXtmrtReq) (hDev, XTMRT_CMD_SEND_CELL, pCell);
        break;

    default:
        bValidOamRxd = false;  /* invalid OAM was received */
        break;

    }

#if defined (CONFIG_BCM963138) || defined (CONFIG_BCM963148)
    /* Was this a valid OAM? */
    if(bValidOamRxd == true)
    {
        /* Since we RX'd a valid OAM, disable byte strip workaround
           if it is in use */
        DisableStripByteWorkaround();
    }
#endif

    return( 0 );
} /* ReceiveOamCb */


    /* For the 63138 and 63148, implement a workaround to strip bytes and
       allow OAM traffic due to JIRA HW63138-12 */
#if defined (CONFIG_BCM963138) || defined (CONFIG_BCM963148)

/***************************************************************************
 * Function Name: EnableStripByteWorkaround
 * Description  : Enable the strip byte OAM workaround to allow OAM traffic 
 *    despite ATM bonding hardware limitation documented in JIRA HW63138-12.
 * Returns      : 0 if successful, non-0 if not
 ***************************************************************************/
int XTM_OAM_HANDLER::EnableStripByteWorkaround( void )
{
    int i;          /* Ubiquitous counter variable */
    XTM_BOND_INFO BondInfo;

    //XtmOsPrintf("bcmxtmcfg: Enabling 63138 strip byte workaround for OAM traffic.\n"
                //"    Network traffic may be degraded of up to %d seconds.\n", 
                //(XTM_STRIPBYTEWORKAROUND_TIMEOUT_MS / 1000));

    /* Only enter workaround in ATM Bonded mode */
    m_pXtmProcessor->GetBondingInfo ( &BondInfo );
    if(BondInfo.ulTrafficType == TRAFFIC_TYPE_ATM_BONDED)
    {
        /* Set mode flag to indicate we're in workaround mode. */
        m_bStripByteWorkaroundActive = true;

        /* Set timeout so we'll turn off workaround if we don't get a response */
        m_ulStripByteWorkaroundTimeStart = XtmOsGetTimeStamp();

        /* Now perform the workroaund itself and modify SAR_RX_VCAM_RX_RAM0  
           through SAR_RX_VCAM_RX_RAM15 registers as needed.*/
        for(i=0; i<XP_MAX_CONNS; i++)
        {
           /* Working variable with SAR_RX_VCAM_RX_RAM[i] register contents */
           UINT32 ulTemp = XP_REGS->ulRxVpiVciCam[(i*2) + 1];

            /* Save away the SAR_RX_VCAM_RX_RAM0 through SAR_RX_VCAM_RX_RAM15
               registers to the m_pulSavedRxVcamRxRamReg[] array.*/
           m_pulSavedRxVcamRxRamReg[i] = ulTemp;

           /* Do we need to clear the STRP_EN and STRP_BYTE fields
              for this register? */
           if((ulTemp & (RXCAM_STRIP_BYTE_MASK | RXCAM_STRIP_EN)) != 0)
           {

               /* Clear bit fields */
               //XtmOsPrintf("    DEBUG: Setting SAR_RX_VCAM_RX_RAM%d reg: %8.8x ", i, (unsigned int)ulTemp);
               ulTemp = ulTemp & ~(RXCAM_STRIP_BYTE_MASK | RXCAM_STRIP_EN); 
               //XtmOsPrintf("to %8.8x\n", (unsigned int)ulTemp);

               /* Write stripped value back to register */
               XP_REGS->ulRxVpiVciCam[(i*2) + 1] = ulTemp;
           }
               //else XtmOsPrintf("DEBUG(%s): Leaving SAR_RX_VCAM_RX_RAM[%d] as is:  %8.8x\n", __func__, i, (unsigned int)ulTemp);
        }

        /* Send a system event */
        XtmOsSendSysEvent(XTM_OAM_STRIP_BYTE_WORKAROUND_ENABLED);
    }

    return(0);
}

/***************************************************************************
 * Function Name: DisableStripByteWorkaround
 * Description  : Exit the strip byte OAM workaround mode set by 
 *     EnableStripByteWorkaround().
 * Returns      : 0 if successful, non-0 if not
 ***************************************************************************/
int XTM_OAM_HANDLER::DisableStripByteWorkaround( void )
{
    int i;          /* Ubiquitous counter variable */

    //XtmOsPrintf("bcmxtmcfg: Disabling 63138 strip byte workaround for OAM traffic\n"
                //"    because OAM received or timeout passed. Network traffic restored.\n");

    /* Are we in workaround mode? */
    if(m_bStripByteWorkaroundActive)
    {
        /* We are in workaround mode.  Restore the STRP_EN and STRP_BYTE
           fields in the SAR_RX_VCAM_RX_RAM0 through SAR_RX_VCAM_RX_RAM15
           registers by using saved values from the m_pulSavedRxVcamRxRamReg[]
           array.*/
        for(i=0; i<XP_MAX_CONNS; i++)
        {
            /* Restore register.*/
            XP_REGS->ulRxVpiVciCam[(i*2) + 1] = m_pulSavedRxVcamRxRamReg[i];
        }

        /* Send a system event */
        XtmOsSendSysEvent(XTM_OAM_STRIP_BYTE_WORKAROUND_DISABLED);
    }

    /* Exit workaround mode by clearing flag and zeroing timeout. */
    m_bStripByteWorkaroundActive = false;
    m_ulStripByteWorkaroundTimeStart = 0;

    return(0);
}


#endif

/***************************************************************************
 * Function Name: TimerUpdate
 * Description  : Perform periodic tasks for OAM handler.
 * Returns      : 0 if successful, non-0 if not
 ***************************************************************************/
int XTM_OAM_HANDLER::TimerUpdate( void )
{
   //XtmOsPrintf("%s:Enter\n",__FUNCTION__);
   /* For the 63138 and 63148, implement a workaround to strip bytes and
       allow OAM traffic due to JIRA HW63138-12 */
#if defined (CONFIG_BCM963138) || defined (CONFIG_BCM963148)

    UINT32 ulCurrTimestampUs;  // Current time for timeouts

    /*
    Check the MIB counters for received OAM cells.  If the MIB counter has 
    incremented since last pass (as indicated from a static variable) 
    and the bStripByteWorkaroundActive flag is set to false, we know 
    that a corrupted OAM was received.  In that case, run 
    enableStripByteWorkaround() to activate the workaround. 
    */
   if(m_bStripByteWorkaroundActive == false)
   {
       static INT32 lSavedOamRxCount = 0; /* Last received total number of OAMs */
       static INT32 lSavedOamTxCount = 0; /* Last transmitted total number of OAMs */
       INT32 lCurrOamRxCount = 0;         /* Current received total number of OAMs */
       INT32 lCurrOamTxCount = 0;         /* Current transmitted total number of OAMs */
       int i;

       /* Sum OAM cell RX counts for each port. */
       for(i=0; i<MAX_INTERFACES; i++)
       {
           XTM_INTERFACE_STATS sIntfStats;

           /* Get stats on interface */
           m_pXtmProcessor->GetInterfaceStatistics( i, &sIntfStats, 0 );

           /* Sum RX and TX counts */
           lCurrOamRxCount = lCurrOamRxCount + sIntfStats.ulIfInOamRmCells;
           lCurrOamTxCount = lCurrOamTxCount + sIntfStats.ulIfOutOamRmCells;
       }

       /* Compare to last timer pass:  Have tehre been any RX'd OAMs that we have
          not responded to (with a TX OAM)?*/
       if((lCurrOamRxCount - lCurrOamTxCount) > (lSavedOamRxCount - lSavedOamTxCount))
       {
           EnableStripByteWorkaround();
       }

       /* Save away the number of RX'd OAMs */
       lSavedOamRxCount = lCurrOamRxCount;
       lSavedOamTxCount = lCurrOamTxCount;
   }

   {
       /* Capture the current time */
       ulCurrTimestampUs = XtmOsGetTimeStamp();

       /* Are we waiting on timeout for traffic on the strip byte workaround? */
       if((m_bStripByteWorkaroundActive == true) && 
          (((m_ulStripByteWorkaroundTimeStart + (XTM_STRIPBYTEWORKAROUND_TIMEOUT_MS * 1000)) < ulCurrTimestampUs) 
                    ||
           (m_ulStripByteWorkaroundTimeStart > ulCurrTimestampUs) /* If WrapAround occurred */
          ))
       {
           /* We have timed out without recieving an OAM.  Disable workaround. */
           DisableStripByteWorkaround();
       }
   }


#endif

    return(0);
}

