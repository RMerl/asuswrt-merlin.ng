/***********************************************************************
 *
 *  Copyright (c) 2006-2007  Broadcom Corporation
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

#ifndef __DEVCTL_XTM_H__
#define __DEVCTL_XTM_H__


/*!\file devctl_xtm.h
 * \brief Header file for the user mode XTM device control library API.
 *
 * Functions are in the atmctl library.  The atmctl library will get 
 * renamed to the xtmctl library soon.
 * These API are called by user applications to perform XTM driver operations.
 * These API make Linux ioctl calls to XTM driver. 
 *
 */

#include "cms.h"
#include "bcmxtmcfg.h"

/* Values for dual latency port id */
#define PHY0_PATH0                     0
#define PHY0_PATH1                     1
#define PHY1_PATH0                     2
#define PHY1_PATH1                     3
#define PHY0_PATH0_PATH1               4
#define PHY1_PATH0_PATH1               5

#define PORTID_TO_PORTMASK(PORTID)     (UINT32)                             \
    (((PORTID) == PHY0_PATH0) ? PORT_PHY0_PATH0                           : \
     ((PORTID) == PHY0_PATH1) ? PORT_PHY0_PATH1                           : \
     ((PORTID) == PHY1_PATH0) ? PORT_PHY1_PATH0                           : \
     ((PORTID) == PHY1_PATH1) ? PORT_PHY1_PATH1                           : \
     ((PORTID) == PHY0_PATH0_PATH1) ? (PORT_PHY0_PATH0 | PORT_PHY0_PATH1) : \
     ((PORTID) == PHY1_PATH0_PATH1) ? (PORT_PHY1_PATH0 | PORT_PHY1_PATH1) : PORT_PHY0_PATH0)

#define PORTMASK_TO_PORTID(PORTMASK)   (UINT32)                              \
    (((PORTMASK) == PORT_PHY0_PATH0) ? PHY0_PATH0                          : \
     ((PORTMASK) == PORT_PHY0_PATH1) ? PHY0_PATH1                          : \
     ((PORTMASK) == PORT_PHY1_PATH0) ? PHY1_PATH0                          : \
     ((PORTMASK) == PORT_PHY1_PATH1) ? PHY1_PATH1                          : \
     ((PORTMASK) == (PORT_PHY0_PATH0 | PORT_PHY0_PATH1))? PHY0_PATH0_PATH1 : \
     ((PORTMASK) == (PORT_PHY1_PATH0 | PORT_PHY1_PATH1))? PHY1_PATH0_PATH1 : PHY0_PATH0)

/* Default receive packet queue size */
#define DEFAULT_SMALL_PACKET_Q_SIZE 60

#ifdef SUPPORT_DSL_BONDING
    /* To address differential delay between multiple lines, we need
     * more buffering on the Rx side.
     * For ex., at the rate 0f 10/50 (US/DS), for the case of 64 bytes,
     * it would amount to ~= 71000 pkts/sec in DS direction. In case of
     * bonding, all these packets can be considered as fragments, which
     * means, there is 710 fragments per ms durection. If we want to
     * support atleast 1 ms delay differential between line 0 and 1, we
     * may need to pend atleast 710 buffers on the worst case. Current
     * provisioning here is for 2ms delay differentials.
     */
#define DEFAULT_PACKET_Q_SIZE 1600
#else
#define DEFAULT_PACKET_Q_SIZE 400
#endif

/* Default receive cell queue size */
#if defined(CHIP_6338)
#define DEFAULT_CELL_Q_SIZE   500  /* 38 needs a bigger cell queue for soft sar (64 bytes each) */
#else
#define DEFAULT_CELL_Q_SIZE   100
#endif

/**Initializes XTM driver
 *
 * This function is called to initialize the XTM driver with configuration
 * parameters.
 *
 * @param pInitParms (IN) - A pointer to XTM_INITIALIZATION_PARMS.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_xtmInitialize( PXTM_INITIALIZATION_PARMS pInitParms );

/**Uninitializes XTM driver
 *
 * This function is called to uninitialize XTM driver and free resources
 * allocated during initialization.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_xtmUninitialize(void);

/**ReInitializes XTM driver
 *
 * This function is called to ReInitialize XTM driver and the SAR HW 
 * with its own pre-configuration. More useful in debugging situations
 * when there is an ambiguity with SAR/PHY lockup with traffic
 * situations to isolate blocks.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_xtmReInitialize(void);


/**Configures XTM driver
 *
 * This function is called to pass configuration parameters to 
 * the XTM driver. 
 *
 * @param pConfigParms (IN) - A pointer to 
 *                  XTM_CONFIGURATION_PARMS.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_xtmConfig( PXTM_CONFIGURATION_PARMS pConfigParms );

/**Manage thresholds/mode in XTM driver
 *
 * This function is called to pass thresholds used per DSL mode of operation in 
 * the XTM driver. 
 *
 * @param pThresholdParms (IN) - A pointer to 
 *                  XTM_THRESHOLD_PARMS.
 *
 * @return CmsRet enum.
 */

CmsRet devCtl_xtmManageThreshold( PXTM_THRESHOLD_PARMS pThresholdParms );

/** Get XTM traffic descriptor table
 *
 * This function is called to get XTM traffic descriptor table entries.
 *
 * @param  pTrafficDescrTable(OUT) - Pointer to PXTM_TRAFFIC_DESCR_PARM_ENTRY
 *         to store table entries.
 * @param  pulTrafficDescrTableSize(IN/OUT) - Pointer to the number of elements
 *         in the TrafficDescrTable on entry and and the number of elements
 *         returned on exit.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_xtmGetTrafficDescrTable( PXTM_TRAFFIC_DESCR_PARM_ENTRY
    pTrafficDescrTable, UINT32 *pulTrafficDescrTableSize );


/** Set XTM traffic descriptor table
 *
 * This function is called to set XTM traffic descriptor table entries.
 *
 * @param  pTrafficDescrTable(IN) - Pointer to PXTM_TRAFFIC_DESCR_PARM_ENTRY
 *         containing table entries.
 * @param  ulTrafficDescrTableSize(IN) - Number of elements in the
 *         TrafficDescrTable.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_xtmSetTrafficDescrTable( PXTM_TRAFFIC_DESCR_PARM_ENTRY
    pTrafficDescrTable, UINT32 ulTrafficDescrTableSize );

/** Get XTM interface configuration record
 *
 * This function is called to get the XTM interface configuration record for
 * a specified port.
 *
 * @param  ulPortId(IN) - Identifies the physical port.
 * @param  pInterfaceCfg(OUT) - Pointer to a XTM_INTERFACE_CFG structure where
 *         the interface configuration is returned.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_xtmGetInterfaceCfg( UINT32 ulPortId, PXTM_INTERFACE_CFG
    pInterfaceCfg );

/** Set XTM interface configuration record
 *
 * This function is called to set the XTM interface configuration record for
 * a specified port.
 *
 * @param  ulPortId(IN) - Identifies the physical port.
 * @param  pInterfaceCfg(IN) - Pointer to XTM_INTERFACE_CFG structure which
 *         contains the interface configuration to set.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_xtmSetInterfaceCfg( UINT32 ulPortId, PXTM_INTERFACE_CFG
    pInterfaceCfg );

/** Set XTM DS PTM bonding deviation.
 *
 * @param  ulDeviation (OUT) - Identifies the deviation.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_xtmSetDsPtmBondingDeviation ( UINT32 ulDeviation );

/** Get XTM connection configuration record
 *
 * This function is called to get an XTM connection configuration record.  A
 * connection is an ATM VCC or PTM flow.
 *
 * @param  pConnAddr(IN) - Pointer to a XTM_CONN_ADDR address whose
 *         configuration is to be retrieved.
 * @param  pConnCfg(OUT) - Pointer to an XTM_CONN_CFG structure where the
 *         connection configuration is returned.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_xtmGetConnCfg( PXTM_ADDR pConnAddr, PXTM_CONN_CFG pConnCfg );

/** Set XTM connection configuration record
 *
 * This function is called to set an XTM connection configuration record.  A
 * connection is an ATM VCC or PTM flow.
 *
 * @param  pConnAddr(IN) - Pointer to an XTM_CONN_ADDR address whose
 *         configuration is to be set.
 * @param  pConnCfg(IN) - Pointer to an XTM_CONN_CFG structure which contains
 *         the connection configuration to set.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_xtmSetConnCfg( PXTM_ADDR pConnAddr, PXTM_CONN_CFG pConnCfg );

/** Get all configured XTM connection addresses. 
 *
 * This function is called to get all XTM connections.
 *
 * @param  pConnAddrs(OUT) - Pointer to an array XTM_CONN_ADDR addresses
 *         where the connection addresses will be returned.
 * @param  ulNumConns(IN/OUT) - Pointer to the number of elements in the
 *         pConnAddrs array on entry and the number of connection addresses
 *         filled in on exit.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_xtmGetConnAddrs( PXTM_ADDR pConnAddrs, UINT32 *pulNumConns );

/** Get XTM Interface Statistics
 *
 * This function is called to get and, optionally, reset XTM interface
 * statistics for an interface.
 *
 * @param  ulPortId(IN) - Identifies the physical port.
 * @param  pStatistics(OUT) - Pointer to a XTM_INTERFACE_STATS structure where
 *         the interface statistics are returned.
 * @param  ulReset(IN) - Flag set to a nonzero value to reset the statistics
 *         fields after they are copied to pStatistics.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_xtmGetInterfaceStatistics( UINT32 ulPortId, PXTM_INTERFACE_STATS
    pStatistics, UINT32 ulReset );




/** Get XTM Error Statistics
 *
 * This function is called to get XTM interface error statistics
 * for an interface. 
 *
 * @param pErrStats(OUT) - Pointer to a PXTM_ERROR_STATS 
 *                 struture to hold error statistics.
 *
 * @return CmsRet enum.
 */


/***************************************************************************
 * Function Name: devCtl_xtmGetErrorStatistics
 * Description  : Returns error statistics.
 * Returns      : CmsRet return code
 ***************************************************************************/
CmsRet devCtl_xtmGetErrorStatistics( PXTM_ERROR_STATS pErrStats );



/** Set XTM Interface Link Info
 *
 * This function is called when a physical link is connected or disconnected.
 *
 * @param  ulPortId(IN) - Identifies the physical port.
 * @param  pLi(OUT) - Pointer to a XTM_INTERFACE_LINK_INFO structure where
 *         the interface link information is returned.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_xtmSetInterfaceLinkInfo( UINT32 ulPortId,
    PXTM_INTERFACE_LINK_INFO pLi );

/** XTM Send OAM Loopback Test
 *
 * This function is called to send OAM F4/F5 loopback test.
 *
 * @param  pConnAddr(IN) - Pointer to an XTM_CONN_ADDR address.  For OAM F4
 *         cells, only the VPI field is used.  The VCI is either 3 or 4.
 * @param  pOamCellInfo(IN/OUT)  - Pointer to an XTM_OAM_CELL_INFO structure
 *         that contains the OAM test type (F4/F5), timeout and repition on
 *         entry and response results and statistics on exit.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_xtmSendOamCell( PXTM_ADDR pConnAddr, PXTM_OAM_CELL_INFO
    pOamCellInfo );

/** Create network device
 *
 * This function is called to create an operating system (Linux) network device
 * instance.
 *
 * @param  pConnAddr(IN) - Pointer to an XTM_CONN_ADDR address.
 * @param  pszNetworkDeviceName(IN)  - String name of the network device.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_xtmCreateNetworkDevice( PXTM_ADDR pConnAddr,
    char *pszNetworkDeviceName );


/** Delete network device
 *
 * This function is called to delete an operating system (Linux) network device
 * instance.
 *
 * @param  pConnAddr(IN) - Pointer to an XTM_CONN_ADDR address.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_xtmDeleteNetworkDevice( PXTM_ADDR pConnAddr );

/** Get Bonding Info
 *
 * This function is called to retrieve XTM traffic status information
 * for bonding (mainly) as well as non-bonding.
 *
 * @param  pBondInfo(IN/OUT) - Pointer to an XTM_BOND_INFO structure.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_xtmGetBondingInfo ( PXTM_BOND_INFO pBondInfo ) ;

#endif /* __DEVCTL_XTM_H__ */

