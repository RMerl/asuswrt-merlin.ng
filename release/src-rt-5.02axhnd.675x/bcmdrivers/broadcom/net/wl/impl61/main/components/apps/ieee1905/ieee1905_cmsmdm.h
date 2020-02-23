/***********************************************************************
 *
 *  Copyright (c) 2014  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2014:proprietary:standard
 *
 *  This program is the proprietary software of Broadcom and/or its
 *  licensors, and may only be used, duplicated, modified or distributed pursuant
 *  to the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied), right
 *  to use, or waiver of any kind with respect to the Software, and Broadcom
 *  expressly reserves all rights in and to the Software and all intellectual
 *  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 *  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 *  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1. This program, including its structure, sequence and organization,
 *     constitutes the valuable trade secrets of Broadcom, and you shall use
 *     all reasonable efforts to protect the confidentiality thereof, and to
 *     use this information only in connection with your use of Broadcom
 *     integrated circuit products.
 *
 *  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
 *     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
 *     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
 *     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
 *     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
 *     PERFORMANCE OF THE SOFTWARE.
 *
 *  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
 *     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
 *     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 *     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
 *     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
 *     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
 *     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
 *     LIMITED REMEDY.
 * :>
 *
 * $Change: $
 ***********************************************************************/

#ifndef _IEEE1905_CMSMDM_H_
#define _IEEE1905_CMSMDM_H_

int i5CmsMdmInit( );
int i5CmsMdmLoadConfig(t_I5_API_CONFIG_BASE *pCfg);
int i5CmsMdmSaveConfig(t_I5_API_CONFIG_BASE *pCfg);

#if defined(DMP_DEVICE2_IEEE1905BASELINE_1)
int i5CmsMdmLoadNetTopConfig(t_I5_API_CONFIG_SET_NETWORK_TOPOLOGY *pCfg);
int i5CmsMdmSaveNetTopConfig(t_I5_API_CONFIG_SET_NETWORK_TOPOLOGY *pCfg);
#endif // endif

int i5CmsMdmRemoveNetworkTopologyDevIfc(const i5_dm_device_type *pDev,
                                        const i5_dm_interface_type *pInterface);

int i5CmsMdmUpdateNetworkTopologyDevIfc(const i5_dm_device_type *pDev,
                                        const i5_dm_interface_type *pInterface);

int i5CmsMdmRemoveNetworkTopologyDevNeighbor(const i5_dm_device_type *pDev,
                                             const i5_dm_1905_neighbor_type *pNeighbor);

int i5CmsMdmUpdateNetworkTopologyDevNeighbor(const i5_dm_device_type *pDev,
                                             const i5_dm_1905_neighbor_type *pNeighbor);

int i5CmsMdmRemoveNetworkTopologyDevLegacyNeighbor(const i5_dm_device_type *pDev,
                                                   const i5_dm_legacy_neighbor_type *pLegacy);

int i5CmsMdmUpdateNetworkTopologyDevLegacyNeighbor(const i5_dm_device_type *pDev,
                                                   const i5_dm_legacy_neighbor_type *pLegacy);

int i5CmsMdmRemoveNetworkTopologyDev(unsigned char* ieee1905Id);

int i5CmsMdmUpdateNetworkTopologyDev(const i5_dm_device_type *pDev);

int i5CmsMdmLocalInterfaceUpdate(const i5_dm_interface_type * const localInterface);

int i5CmsMdmLocalNeighborRemove(const i5_dm_1905_neighbor_type * const localNeighbor);

int i5CmsMdmLocalNeighborUpdate(const i5_dm_1905_neighbor_type * const localNeighbor);

int i5CmsMdmRemoveNetworkTopologyDevLegacyNeighbor(const i5_dm_device_type *pDev,
                                                   const i5_dm_legacy_neighbor_type *pLegacy);

int i5CmsMdmUpdateNetworkTopologyDevLegacyNeighbor(const i5_dm_device_type *pDev,
                                                   const i5_dm_legacy_neighbor_type *pLegacy);

void i5CmsMdmUpdateBridgingTuple(const i5_dm_device_type * pDev, i5_dm_bridging_tuple_info_type * pBrTuple);

void i5CmsMdmDeleteBridgingTuple(const i5_dm_device_type *pDev, i5_dm_bridging_tuple_info_type *pBrTuple);

int i5CmsMdmReinitNetworkTopology(void);

void i5CmsMdmProcessNetworkTopologyConfigChange(void);

#endif // endif
