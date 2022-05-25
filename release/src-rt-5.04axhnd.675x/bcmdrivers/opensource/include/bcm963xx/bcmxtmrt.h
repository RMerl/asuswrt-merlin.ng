/*
<:copyright-BRCM:2007:DUAL/GPL:standard

   Copyright (c) 2007 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/

/***************************************************************************
 * File Name  : bcmxtmrt.h
 *
 * Description: This file contains the definitions, structures and function
 *              prototypes for the Broadcom Asynchronous/Packet Transfer Mode
 *              (XTM) Runtime driver.
 ***************************************************************************/

#if !defined(_BCMXTMRT_H_)
#define _BCMXTMRT_H_
#include "bcmxtmcfg.h"

#define CNI_HW_ADD_HEADER                   0x01
#define CNI_HW_REMOVE_HEADER                0x02
#define CNI_USE_ALT_FSTAT                   0x04
#define CNI_HW_REMOVE_TRAILER               0x08
#define CNI_GFAST                           0x10
#define CNI_NITRO                           0x20
#define CNI_HW_PTM_BONDING                  0x40
#define CNI_HW_PAD                          0x80
#define LSC_RAW_ENET_MODE                   0x80000000
#define CELL_PAYLOAD_SIZE                   48
#define CELL_SIZE                           (5 + CELL_PAYLOAD_SIZE)
#define MAX_VCIDS                           16

#define XTMRT_DEV_OPENED                    1
#define XTMRT_DEV_CLOSED                    2

#define CELL_HDLR_OAM                       1
#define CELL_HDLR_ASM                       2

#define XTMRT_CMD_GLOBAL_INITIALIZATION            1
#define XTMRT_CMD_CREATE_DEVICE                    2
#define XTMRT_CMD_GET_DEVICE_STATE                 3
#define XTMRT_CMD_SET_ADMIN_STATUS                 4
#define XTMRT_CMD_REGISTER_CELL_HANDLER            5
#define XTMRT_CMD_UNREGISTER_CELL_HANDLER          6
#define XTMRT_CMD_LINK_STATUS_CHANGED              7
#define XTMRT_CMD_SEND_CELL                        8
#define XTMRT_CMD_DELETE_DEVICE                    9
#define XTMRT_CMD_SET_TX_QUEUE                     10
#define XTMRT_CMD_UNSET_TX_QUEUE                   11
#define XTMRT_CMD_GET_NETDEV_TXCHANNEL             12
#define XTMRT_CMD_GLOBAL_UNINITIALIZATION          13
#define XTMRT_CMD_TOGGLE_PORT_DATA_STATUS_CHANGE   14
#define XTMRT_CMD_SET_TEQ_DEVCTX                   15
#define XTMRT_CMD_SET_DS_SEQ_DEVIATION 	           16
#define XTMRT_CMD_GLOBAL_REINITIALIZATION          17
#define XTMRT_CMD_SET_ATMBOND_SID_MODE             18
#define XTMRT_CMD_STOP_ALL_TX_QUEUE                19
#define XTMRT_CMD_START_ALL_TX_QUEUE               20
#define XTMRT_CMD_SET_TX_PORT_SHAPER_INFO          21

#define XTMRT_CMD_PORT_DATA_STATUS_DISABLED  0
#define XTMRT_CMD_PORT_DATA_STATUS_ENABLED   1

#define XTMRTCB_CMD_CELL_RECEIVED           3

/****************************************************************************
   Logging Defines for both XTMCFG & XTMRT driver.
****************************************************************************/
//These definitions are used mainly in XTMCFG drivers.
typedef enum {
   XTM_LOG_ERR = 0,              //Error
   XTM_LOG_NOT,                  //Notice
   XTM_LOG_INF,                  //Info
   XTM_LOG_DBG,                  //Debug
   XTM_LOG_MAX
}Xtm_LogLevel;

#define BCM_XTM_LOG      
#define BCM_XTM_RX_LOG  
#define BCM_XTM_TX_LOG   
#define BCM_XTM_LINK_LOG
extern unsigned int rx_path_debug;
extern unsigned int tx_path_debug;
extern unsigned int xtm_link_debug;
#define RX_PATH_DBG_BITMASK  0x0001
#define TX_PATH_DBG_BITMASK  0x0002
#define RX_DATA_DUMP_BITMASK 0x0004
#define TX_DATA_DUMP_BITMASK 0x0008
#define XTM_LINK_DBG_BITMASK 0x0010

#if defined(BCM_XTM_LOG)
#define BCM_XTM_DEBUG(fmt, arg...)  BCM_LOG_DEBUG(BCM_LOG_ID_XTM, fmt, ##arg)
#define BCM_XTM_INFO(fmt, arg...)   BCM_LOG_INFO(BCM_LOG_ID_XTM, fmt, ##arg)
#define BCM_XTM_NOTICE(fmt, arg...) BCM_LOG_NOTICE(BCM_LOG_ID_XTM, fmt, ##arg)
#define BCM_XTM_ERROR(fmt, arg...)  BCM_LOG_ERROR(BCM_LOG_ID_XTM, fmt, ##arg)
#else
#define BCM_XTM_DEBUG(fmt, arg...)
#define BCM_XTM_INFO(fmt, arg...)
#define BCM_XTM_NOTICE(fmt, arg...)
#define BCM_XTM_ERROR(fmt, arg...)
#endif

#if defined(BCM_XTM_RX_LOG)
#define BCM_XTM_RX_DEBUG(fmt, arg...)  if(unlikely(rx_path_debug == 1)) \
                                          BCM_LOG_DEBUG(BCM_LOG_ID_XTM, fmt, ##arg)
#define BCM_XTM_RX_INFO(fmt, arg...)   if(unlikely(rx_path_debug == 1)) \
                                          BCM_LOG_INFO(BCM_LOG_ID_XTM, fmt, ##arg)
#define BCM_XTM_RX_NOTICE(fmt, arg...) if(unlikely(rx_path_debug == 1)) \
                                          BCM_LOG_NOTICE(BCM_LOG_ID_XTM, fmt, ##arg)
#define BCM_XTM_RX_ERROR(fmt, arg...)  if(unlikely(rx_path_debug == 1)) \
                                          BCM_LOG_ERROR(BCM_LOG_ID_XTM, fmt, ##arg)
#else
#define BCM_XTM_RX_DEBUG(fmt, arg...)
#define BCM_XTM_RX_INFO(fmt, arg...)
#define BCM_XTM_RX_NOTICE(fmt, arg...)
#define BCM_XTM_RX_ERROR(fmt, arg...)
#endif

#if defined(BCM_XTM_TX_LOG)
#define BCM_XTM_TX_DEBUG(fmt, arg...)  if(unlikely(tx_path_debug == 1)) \
                                          BCM_LOG_DEBUG(BCM_LOG_ID_XTM, fmt, ##arg)
#define BCM_XTM_TX_INFO(fmt, arg...)   if(unlikely(tx_path_debug == 1)) \
                                          BCM_LOG_INFO(BCM_LOG_ID_XTM, fmt, ##arg)
#define BCM_XTM_TX_NOTICE(fmt, arg...) if(unlikely(tx_path_debug == 1)) \
                                          BCM_LOG_NOTICE(BCM_LOG_ID_XTM, fmt, ##arg)
#define BCM_XTM_TX_ERROR(fmt, arg...)  if(unlikely(tx_path_debug == 1)) \
                                          BCM_LOG_ERROR(BCM_LOG_ID_XTM, fmt, ##arg)
#else
#define BCM_XTM_TX_DEBUG(fmt, arg...)
#define BCM_XTM_TX_INFO(fmt, arg...)
#define BCM_XTM_TX_NOTICE(fmt, arg...)
#define BCM_XTM_TX_ERROR(fmt, arg...)
#endif

#if defined(BCM_XTM_LINK_LOG)
#define BCM_XTM_LINK_DEBUG(fmt, arg...)  if(unlikely(xtm_link_debug == 1)) \
                                            BCM_LOG_DEBUG(BCM_LOG_ID_XTM, fmt, ##arg)
#define BCM_XTM_LINK_INFO(fmt, arg...)   if(unlikely(xtm_link_debug == 1)) \
                                            BCM_LOG_INFO(BCM_LOG_ID_XTM, fmt, ##arg)
#define BCM_XTM_LINK_NOTICE(fmt, arg...) if(unlikely(xtm_link_debug == 1)) \
                                            BCM_LOG_NOTICE(BCM_LOG_ID_XTM, fmt, ##arg)
#define BCM_XTM_LINK_ERROR(fmt, arg...)  if(unlikely(xtm_link_debug == 1)) \
                                            BCM_LOG_ERROR(BCM_LOG_ID_XTM, fmt, ##arg)
#else
#define BCM_XTM_LINK_DEBUG(fmt, arg...)
#define BCM_XTM_LINK_INFO(fmt, arg...)
#define BCM_XTM_LINK_NOTICE(fmt, arg...)
#define BCM_XTM_LINK_ERROR(fmt, arg...)
#endif

#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM963146)
typedef unsigned long XTMRT_HANDLE;
#else
typedef UINT32 XTMRT_HANDLE;
#endif

typedef int (*FN_XTMRT_REQ) (XTMRT_HANDLE hDev, UINT32 ulCmd, void *pParm);

typedef struct XtmrtGlobalInitParms
{
    UINT32 ulReceiveQueueSizes[MAX_RECEIVE_QUEUES];
    XtmBondConfig bondConfig;

    UINT32 ulMibRxClrOnRead;
    volatile UINT32 *pulMibTxOctetCountBase;
    volatile UINT32 *pulMibRxCtrl;
    volatile UINT32 *pulMibRxMatch;
    volatile UINT32 *pulMibRxOctetCount;
    volatile UINT32 *pulMibRxPacketCount;
    volatile UINT32 *pulRxCamBase;

} XTMRT_GLOBAL_INIT_PARMS, *PXTMRT_GLOBAL_INIT_PARMS;

typedef struct XtmrtCreateNetworkDevice
{
    XTM_ADDR ConnAddr;
    char szNetworkDeviceName[NETWORK_DEVICE_NAME_SIZE];
    UINT32 ulHeaderType;
    UINT32 ulFlags;
    UINT32 ulTxPafEnabled;
    XTMRT_HANDLE hDev;
} XTMRT_CREATE_NETWORK_DEVICE, *PXTMRT_CREATE_NETWORK_DEVICE;

typedef struct XtmrtCell
{
    XTM_ADDR ConnAddr;
    UINT8 ucCircuitType;
    UINT8 ucData[CELL_SIZE];
} XTMRT_CELL, *PXTMRT_CELL;

typedef int (*XTMRT_CALLBACK) (XTMRT_HANDLE hDev, UINT32 ulCommand, void *pParam,
    void *pContext);

typedef struct XtmrtCellHdlr
{
    UINT32 ulCellHandlerType;
    XTMRT_CALLBACK pfnCellHandler;
    void *pContext;
} XTMRT_CELL_HDLR, *PXTMRT_CELL_HDLR;

typedef struct XtmrtPortShaperInfo
{
    UINT32 ulShapingRate;        /* 0 indicates no shaping */
    UINT16 usShapingBurstSize;
} XTMRT_PORT_SHAPER_INFO, *PXTMRT_PORT_SHAPER_INFO;

#define XTM_HIGH_SUB_PRIORITY       MAX_SUB_PRIORITIES-1
#define XTM_LOW_SUB_PRIORITY        0

typedef struct XtmrtTransmitQueueId
{
    UINT32 ulPortId;
    UINT32 ulBondingPortId;      /* Read-only */
    UINT32 ulPtmPriority;
    UINT32 ulWeightValue;
    UINT8  ucQosQId;
    UINT8  ucWeightAlg;
    UINT8  ucSubPriority;
    UINT8  ucDropAlg;            /* DT or RED or WRED */
    UINT8  ucLoMinThresh;        /* RED/WRED Low Class min threshold in % of queue size */
    UINT8  ucLoMaxThresh;        /* RED/WRED Low Class max threshold in % of queue size */
    UINT8  ucHiMinThresh;        /* WRED High Class min threshold in % of queue size */
    UINT8  ucHiMaxThresh;        /* WRED High Class max threshold in % of queue size */
    UINT32 ulMinBitRate;         /* 0 indicates no shaping */
    UINT32 ulShapingRate;        /* 0 indicates no shaping */
    UINT16 usShapingBurstSize;
    UINT16 usQueueSize;
    UINT32 ulMBR;
    UINT32 ulQueueIndex;
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    UINT32 ulLoThresh;
    UINT32 ulHiThresh;
    UINT32 ulDropped;
#endif
} XTMRT_TRANSMIT_QUEUE_ID, *PXTMRT_TRANSMIT_QUEUE_ID;

typedef struct XtmrtLinkStatusChange
{
    UINT32 ulLinkState;
    UINT32 ulLinkUsRate;
    UINT32 ulLinkDsRate;
    UINT32 ulLinkDataMask ;
    UINT32 ulOtherLinkUsRate ;
    UINT32 ulOtherLinkDsRate ;
    UINT32 ulDsSeqDeviation ;   /* Only for bonding */
    UINT32 ulTrafficType ;
    UINT32 ulTransmitQueueIdsSize;
    XTMRT_TRANSMIT_QUEUE_ID TransmitQueueIds[MAX_TRANSMIT_QUEUES];
    UINT8 ucTxVcid ;
    UINT32 ulRxVcidsSize ;
    UINT8 ucRxVcids [MAX_VCIDS];
} XTMRT_LINK_STATUS_CHANGE, *PXTMRT_LINK_STATUS_CHANGE;

typedef struct XtmrtNetdevTxchannel
{
    UINT32 txChannel;
    void * pDev;
} XTMRT_NETDEV_TXCHANNEL, *PXTMRT_NETDEV_TXCHANNEL;

typedef struct XtmrtTogglePortDataStatusChange
{
    UINT32 ulPortId;
    UINT32 ulPortDataUsStatus ;
    UINT32 ulPortDataDsStatus ;
} XTMRT_TOGGLE_PORT_DATA_STATUS_CHANGE, *PXTMRT_TOGGLE_PORT_DATA_STATUS_CHANGE;

#if defined(__cplusplus)
extern "C" {
#endif

int bcmxtmrt_request(XTMRT_HANDLE hDev, UINT32 ulCmd, void *pParm);

#if defined(__cplusplus)
}
#endif

#endif /* _BCMXTMRT_H_ */

