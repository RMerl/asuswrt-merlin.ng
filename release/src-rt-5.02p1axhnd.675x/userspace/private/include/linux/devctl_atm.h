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

#ifndef __DEVCTL_ATM_H__
#define __DEVCTL_ATM_H__


/*!\file devctl_atm.h
 * \brief Header file for the user mode ATM device control library API.
 *  This is in the devCtl library.
 *
 * These API are called by user applications to perform ATM driver operations.
 * These API make Linux ioctl calls to ATM driver. 
 *
 */


#include "cms.h"
#include "atmapidrv.h"


/**Initializes ATM driver
 *
 * This function is called to initialize ATM driver with ATM configuration parameters.
 *
 * @param pInitParms (IN)  A pointer to ATM_INITIALIZATION_PARMS.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_atmInitialize(PATM_INITIALIZATION_PARMS pInitParms);

/**Uninitializes ATM driver
 *
 * This function is called to uninitialize ATM driver and free resources allocated during initialization.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_atmUninitialize(void);

/** Get Interface ID of a physical port number
 *
 * This function is called to get interface id for a specified ATM port number.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_atmGetInterfaceId(UINT8 ucPhyPort, UINT32 *pulInterfaceId);

/** Get priority packet entries for a group
 *
 * This function is called get priority packet entries for a group.
 *
 * @param ulGroupNumber (IN)    Integer group number to get packet entries for.
 * @param pPriortyPackets (OUT) A pointer to PATM_PRIORITY_PACKET_ENTRY to store packet entry info.
 * @param pulPriorityPacketsSize (OUT) Pointer to integer to store size of priorityPackets returned. 
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_atmGetPriorityPacketGroup( UINT32 ulGroupNumber,
                                         PATM_PRIORITY_PACKET_ENTRY pPriorityPackets, 
                                         UINT32 *pulPriorityPacketsSize);

/** Set priority packet entries for a group
 *
 * This function is called set priority packet entries for a group.
 *
 * @param ulGroupNumber (IN)    Integer group number to set packet entries for.
 * @param pPriortyPackets (IN) A pointer to PATM_PRIORITY_PACKET_ENTRY packet entry info to set to group.
 * @param ulPriorityPacketsSize (IN) Integer containing size of priorityPackets.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_atmSetPriorityPacketGroup(UINT32 ulGroupNumber,
                                        PATM_PRIORITY_PACKET_ENTRY pPriorityPackets,
                                        UINT32 ulPriorityPacketsSize);


/** Get ATM traffic descriptor table size
 *
 * This function is called to get the number of entries in the ATM traffic descriptor table.
 *
 * @param  pulTrafficDescrTableSize(OUT)  Pointer to interger to store returned table size.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_atmGetTrafficDescrTableSize(UINT32 *pulTrafficDescrTableSize);

/** Get ATM traffic descriptor table
 *
 * This function is called to get ATM traffic descriptor table entries.
 *
 * @param  pTrafficDescrTable(OUT) Pointer to PATM_TRAFFIC_DESCR_PARM_ENTRY to store table entries.
 * @param  ulTrafficDescrTableSize(IN) Size of TrafficDescrTable.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_atmGetTrafficDescrTable(PATM_TRAFFIC_DESCR_PARM_ENTRY pTrafficDescrTable,
                                      UINT32 ulTrafficDescrTableSize);

/** Set ATM traffic descriptor table
 *
 * This function is called to set ATM traffic descriptor table entries.
 *
 * @param  pTrafficDescrTable(IN) Pointer to PATM_TRAFFIC_DESCR_PARM_ENTRY containing table entries.
 * @param  ulTrafficDescrTableSize(IN) Size of TrafficDescrTable.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_atmSetTrafficDescrTable(PATM_TRAFFIC_DESCR_PARM_ENTRY pTrafficDescrTable,
                                      UINT32 ulTrafficDescrTableSize);

/** Get ATM interface configuration record
 *
 * This function is called to get ATM interface configuration record of an interface ID.
 *
 * @param  ulInterfaceId(IN) Integer interface ID.
 * @param  pInterfaceCfg(OUT) Pointer to ATM_INTERFACE_CFG which stores interface configuration.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_atmGetInterfaceCfg( UINT32 ulInterfaceId,
                                  PATM_INTERFACE_CFG pInterfaceCfg );

/** Set ATM interface configuration record
 *
 * This function is called to set ATM interface configuration record of an interface ID.
 *
 * @param  ulInterfaceId(IN) Integer interface ID.
 * @param  pInterfaceCfg(IN) Pointer to ATM_INTERFACE_CFG which stores interface configuration to set.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_atmSetInterfaceCfg( UINT32 ulInterfaceId,
                                  PATM_INTERFACE_CFG pInterfaceCfg );

/** Get ATM VCC configuration record
 *
 * This function is called to get ATM VCC configuration record with a VCC address.
 *
 * @param  pVccAddr(IN) Pointer to ATM_VCC_ADDR whose configuration is to be retrieved.
 * @param  pVccCfg(OUT) Pointer to ATM_VCC_CFG which stores VCC configuration.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_atmGetVccCfg(PATM_VCC_ADDR pVccAddr, PATM_VCC_CFG pVccCfg);

/** Set ATM VCC configuration record
 *
 * This function is called to set ATM VCC configuration record with a VCC address.
 *
 * @param  pVccAddr(IN) Pointer to ATM_VCC_ADDR whose configuration is to be set.
 * @param  pVccCfg(IN) Pointer to ATM_VCC_CFG which stores VCC configuration.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_atmSetVccCfg(PATM_VCC_ADDR pVccAddr, PATM_VCC_CFG pVccCfg);

/** Get all ATM VCC addresses configured on an interface. 
 *
 * This function is called to get all ATM VCCs configured on an interface specified by interfaceId.
 *
 * @param  ulInterfaceId(IN) InterfaceId of interface
 * @param  pVccAddrs(OUT) Pointer to ATM_VCC_ADDR which stores all VCC addresses on interface
 * @param  ulNumVccs(IN) Number ATM_VCC_ADDR can stored by pVccAddrs
 * @param  pulNumReturned(OUT) Number ATM_VCC_ADDR actually returned by driver
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_atmGetVccAddrs(UINT32 ulInterfaceId, PATM_VCC_ADDR pVccAddrs,
                             UINT32 ulNumVccs, UINT32 *pulNumReturned);

/** Get ATM Interface Statistics
 *
 * This function is called to get (and) reset ATM interface statistics of an interface.
 *
 * @param  ulInterfaceId(IN) InterfaceId of interface
 * @param  pStatistics(OUT) Pointer to ATM_INTERFACE_STATS
 * @param  ulReset(IN)  nonzero if statistics are to be reset to 0 after copied to pStatistics.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_atmGetInterfaceStatistics (UINT32 ulInterfaceId,
                                         PATM_INTERFACE_STATS pStatistics, UINT32 ulReset);

/** Get ATM VCC Statistics
 *
 * This function is called to get (and) reset ATM VCC statistics for a specified VCC address.
 *
 * @param  pVccAddr(IN) Pointer to ATM_VCC_ADDR whose statistics caller wishes to get
 * @param  pVccStatistics(OUT) Pointer to ATM_VCC_STATS
 * @param  ulReset(IN)  nonzero if statistics are to be reset to 0 after copied to pVccStatistics.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_atmGetVccStatistics(PATM_VCC_ADDR pVccAddr,
                                  PATM_VCC_STATS pVccStatistics, UINT32 ulReset);


/** Set ATM Interface Link Info
 *
 * This function is called to set physical link information for the specified interface id.
 *
 * @param  ulInterfaceId(IN) Interger containing the interface ID.
 * @param  pLi(IN) Pointer to ATM_INTERFACE_LINK_INFO
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_atmSetInterfaceLinkInfo(UINT32 ulInterfaceId, PATM_INTERFACE_LINK_INFO pLi);

/** Set Notify Callback
 *
 * This function is called to add specified callback function to list of functions that are
 * called when an ATM notification event occurs.   Obsolete.
 *
 * @param  pFnNotifyCb(IN) 
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_atmSetNotifyCallback(FN_NOTIFY_CB pFnNotifyCb);

/** Reset Notify Callback
 *
 * This function is called to remove specified callback function to list of functions that are
 * called when an ATM notification event occurs.   Obsolete.
 *
 * @param  pFnNotifyCb(IN) 
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_atmResetNotifyCallback(FN_NOTIFY_CB pFnNotifyCb);

/** ATM Attach VCC
 *
 * This function is called to attach an application to a VCC.  Obsolete.
 *
 * @param  pVccAddr
 * @param  pAttachParms

 * @return CmsRet enum.
 */
CmsRet devCtl_atmAttachVcc(PATM_VCC_ADDR pVccAddr, PATM_VCC_ATTACH_PARMS pAttachParms);

/** ATM Attach Management Cell.
 *
 * This function is called to attach an application to send and receive
 * ATM management cells on any VCC.  Obsolete.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_atmAttachMgmtCells(UINT32 ulInterfaceId, PATM_VCC_ATTACH_PARMS pAttachParms);

/** ATM Attach Transparent.
 *
 * This function is called to attach an application to send and receive
 * transparent ATM cells.  Obsolete.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_atmAttachTransparent(UINT32 ulInterfaceId, PATM_VCC_ATTACH_PARMS pAttachParms);


/** ATM Detach
 *
 * This function is called to detach an application from a VCC, management cell or 
 * transparent.  Obsolete.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_atmDetach(UINT32 ulHandle);

/** ATM Set AAL2 Channel IDs
 *
 * This function is called to specify a list of AAL2 channel ids that an application
 * wants to send and receive data for an AAL2 VCC.  Obsolete.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_atmSetAal2ChannelIds(UINT32 ulHandle,
                                   PATM_VCC_AAL2_CHANNEL_ID_PARMS pChannelIdParms,
                                   UINT32 ulNumChannelIdParms);

/** ATM Send VCC Data
 *
 * This function is called to send data on a VCC.  Obsolete
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_atmSendVccData(UINT32 ulHandle, PATM_VCC_DATA_PARMS pDataParms);

/** ATM Send Management Data
 *
 * This function is called to send management on a VCC.  Obsolete
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_atmSendMgmtData(UINT32 ulHandle, PATM_VCC_ADDR pVccAddr, PATM_VCC_DATA_PARMS pDataParms);


/** ATM Send Transparent Data
 *
 * This function is called to send transparent data on a VCC.  Obsolete
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_atmSendTransparentData(UINT32 ulHandle, UINT32 ulInterfaceId,
                                     PATM_VCC_DATA_PARMS pDataParms);

/** ATM Send OAM Loopback Test
 *
 * This function is called to send OAM F4/F5 loopback test.
 *
 * @param  type(IN) Type of test: OAM_LB_SEGMENT_TYPE, OAM_LB_END_TO_END_TYPE,
 *                                OAM_F4_LB_SEGMENT_TYPE, OAM_F4_LB_SEGMENT_TYPE
 * @param  pVccAddr(IN) Pointer to ATM_VCC_ADDR of VCC to do test on
 * @param  repetition(IN) Number of OAM loopback cell is to be sent/received.
 * @param  timeout(IN) Timeout value in ms; when timeout expires and response is not received, test fails.
 * @param  timeout(IN) result structure ATMDRV_OAM_LOOPBACK.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_atmSendOamLoopbackTest(UINT32 type, PATM_VCC_ADDR pVccAddr, UINT32 repetition, UINT32 timeout,
                                     ATMDRV_OAM_LOOPBACK *result);

/** ATM Set Bonding Bypass
 *
 * This function is called enable or disable ATM bonding.
 *
 * @param  bonding_bypass(IN) 0 for disable, nonzero for enable
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_atmSetBondingBypass(UINT32 bonding_bypass);

/** ATM get Bonding Connection Info.
 *
 * Returns the ADSL connection info for the non-bonded (or) bonded pair.
 *
 * @param pAdslInfo (OUT) pointer to ADSL_CONNECTION_INFO structure which
 *                        will be filled in by this function.
 *
 * @return CmsRet enum.
 */
CmsRet devCtl_atmGetBondingConnectionInfo(ADSL_CONNECTION_INFO *pAdslInfo);

#endif /* __DEVCTL_ATM_H__ */
