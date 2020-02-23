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

#if defined(DMP_DEVICE2_IEEE1905BASELINE_1)
#include "cms.h"
#include "cms_util.h"
#include "cms_obj.h"
#include "cms_lck.h"
#include "cms_msg.h"
#include "cms_boardcmds.h"
#include "cms_boardioctl.h"
#include "ieee1905_datamodel_priv.h"
#include "ieee1905_trace.h"
#include "ieee1905_cmsmdm.h"
#include "ieee1905_utils.h"
#include "ieee1905_glue.h"
#include "mdm_validstrings.h"

#define I5_TRACE_MODULE    i5TraceCmsUtil

/*!\file ieee1905_cmsmdm.c
 * \brief This file handles IEEE1905 TR181 Data Model.
 *
 */
#define I5_CMS_MDM_LOCK_TIMEOUT      200   /* 200 milliseconds */
#define I5_CMS_MDM_LOCK_TIMEOUT_LONG 20000 /* 20 seconds */
#define I5_CMS_MDM_LOCK_RETRY        10000 /* 10 seconds */

#define I5_CMS_MDM_RETRY_SAVE_BASE             (1 << 0)
#define I5_CMS_MDM_RETRY_SAVE_NETWORK_TOPOLOGY (1 << 1)
#define I5_CMS_MDM_RETRY_NETWORK_TOPOLOGY      (1 << 2)
static timer_elem_type                      *i5CmsMdmLockTimer = NULL;
static int                                   i5CmsMdmTimerOp   = 0;
static t_I5_API_CONFIG_BASE                  i5CmsMdmBaseCfg;
static t_I5_API_CONFIG_SET_NETWORK_TOPOLOGY  i5CmsMdmNetTopCfg;

#define I5_CMSMDM_MAX_BRIDGING_TUPLE_ITEM_LENGTH 256
#define I5_CMSMDM_MAX_BRIDGING_TUPLE_TOTAL_LENGTH 2048

static const char i5CmsMdmMediaTypeStrings[][20] =
{
  MDMVS_IEEE_802_3U,
  MDMVS_IEEE_802_3AB,
  MDMVS_IEEE_802_11B,
  MDMVS_IEEE_802_11G,
  MDMVS_IEEE_802_11A,
  MDMVS_IEEE_802_11N_2_4,
  MDMVS_IEEE_802_11N_5_0,
  MDMVS_IEEE_802_11AC,
  MDMVS_IEEE_802_11AD,
  MDMVS_IEEE_802_11AF,
  MDMVS_IEEE_1901__WAVELET,
  MDMVS_IEEE_1901_FFT,
  MDMVS_MOCAV1_1
};

static const char* _i5CmsMdmGetMdmStringFromMediaType(const unsigned short mediaType)
{
   switch (mediaType) {
      case I5_MEDIA_TYPE_FAST_ETH:
         return i5CmsMdmMediaTypeStrings[0];
      case I5_MEDIA_TYPE_GIGA_ETH:
         return i5CmsMdmMediaTypeStrings[1];
      case I5_MEDIA_TYPE_WIFI_B:
         return i5CmsMdmMediaTypeStrings[2];
      case I5_MEDIA_TYPE_WIFI_G:
         return i5CmsMdmMediaTypeStrings[3];
      case I5_MEDIA_TYPE_WIFI_A:
         return i5CmsMdmMediaTypeStrings[4];
      case I5_MEDIA_TYPE_WIFI_N24:
         return i5CmsMdmMediaTypeStrings[5];
      case I5_MEDIA_TYPE_WIFI_N5:
         return i5CmsMdmMediaTypeStrings[6];
      case I5_MEDIA_TYPE_WIFI_AC:
         return i5CmsMdmMediaTypeStrings[7];
      case I5_MEDIA_TYPE_WIFI_AD:
         return i5CmsMdmMediaTypeStrings[8];
      case I5_MEDIA_TYPE_WIFI_AF:
         return i5CmsMdmMediaTypeStrings[9];
      case I5_MEDIA_TYPE_1901_WAVELET:
         return i5CmsMdmMediaTypeStrings[10];
      case I5_MEDIA_TYPE_1901_FFT:
         return i5CmsMdmMediaTypeStrings[11];
      case I5_MEDIA_TYPE_MOCA_V11:
         return i5CmsMdmMediaTypeStrings[12];
      default:
         return NULL;
   }
}

/** return the full path name of IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}.Interface.{i}. .
 *  @param pParentIidStack      (IN) MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE iidstack.
 *  @param pInterface           (IN) neighbor IEEE1905 device's interface mac address.
 *  @return full path. the caller have to free this string buffer.
 */
static char * _i5CmsMdmGetNetworkTopologyDevIfcFullPath(const InstanceIdStack *pParentIidStack, const unsigned char *intfId)
{
   CmsRet ret = CMSRET_SUCCESS;
   InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
   Dev2Ieee1905AlNetworkTopologyIeee1905DeviceIfcObject *ieee1905DevIfcObj = NULL;
   char macAddrStr[I5_MAC_STR_BUF_LEN] = {0};
   char *ieee1905IfcFullPath = NULL;
   MdmPathDescriptor ieee1905IfcPathDesc;

   snprintf(macAddrStr, I5_MAC_STR_BUF_LEN, I5_MAC_DELIM_FMT, I5_MAC_PRM(intfId));
   i5TraceInfo("InterfaceId=%s\n", macAddrStr);

   while((ret = cmsObj_getNextInSubTreeFlags(MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE_IFC,
                                             pParentIidStack, &iidStack,
                                             OGF_NO_VALUE_UPDATE, (void **) &ieee1905DevIfcObj)) == CMSRET_SUCCESS)
   {
      if(!cmsUtl_strcasecmp(macAddrStr, ieee1905DevIfcObj->interfaceId))
      {
         i5TraceInfo("Found a IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}.Interface.{i}. =%s\n", ieee1905DevIfcObj->interfaceId);
         break;
      }
      cmsObj_free((void **) &ieee1905DevIfcObj);
   }

   if(ret != CMSRET_SUCCESS)
   {
      i5TraceError("Cannot find IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}.Interface.{i}.\n");
      return NULL;
   }
   INIT_PATH_DESCRIPTOR(&ieee1905IfcPathDesc);
   ieee1905IfcPathDesc.oid = MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE_IFC;
   ieee1905IfcPathDesc.iidStack = iidStack;
   cmsMdm_pathDescriptorToFullPathNoEndDot(&ieee1905IfcPathDesc, &ieee1905IfcFullPath);

   cmsObj_free((void **) &ieee1905DevIfcObj);

   return ieee1905IfcFullPath;
}

static CmsRet _i5CmsMdmGetDeviceIidFromMac (const unsigned char *devMacAddr, InstanceIdStack *pParentIidStack,
                                        Dev2Ieee1905AlNetworkTopologyIeee1905DeviceObject **ieee1905DevObj)
{
   CmsRet ret = CMSRET_OBJECT_NOT_FOUND;
   char longVersion[18];

   snprintf(longVersion,18,I5_MAC_DELIM_FMT,I5_MAC_PRM(devMacAddr));

   while((ret = cmsObj_getNextFlags(MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE,
                                    pParentIidStack,
                                    OGF_NO_VALUE_UPDATE, (void **) ieee1905DevObj)) == CMSRET_SUCCESS)
   {
      i5TraceInfo("comparing input=%s found=%s\n", longVersion, (*ieee1905DevObj)->IEEE1905Id);

      if(!cmsUtl_strcasecmp(longVersion, (*ieee1905DevObj)->IEEE1905Id))
      {
         i5TraceInfo("Found a IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}. IEEE1905Id=%s\n", (*ieee1905DevObj)->IEEE1905Id);
         break;
      }
      cmsObj_free((void **) ieee1905DevObj);
   }

   return ret;
}

static CmsRet _i5CmsMdmRemoveNetworkTopologyDevIfcOfParent(const InstanceIdStack *pParentIidStack,
                                                          const i5_dm_interface_type *pInterface)
{
   CmsRet ret = CMSRET_SUCCESS;
   InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
   Dev2Ieee1905AlNetworkTopologyIeee1905DeviceIfcObject *ieee1905DevIfcObj = NULL;
   char macAddrStr[I5_MAC_STR_BUF_LEN] = {0};

   if(pInterface == NULL)
   {
      i5TraceError("pInterface is NULL\n");
      return CMSRET_INVALID_ARGUMENTS;
   }
   snprintf(macAddrStr, I5_MAC_STR_BUF_LEN, I5_MAC_DELIM_FMT, I5_MAC_PRM(pInterface->InterfaceId));
   i5TraceInfo("update ieee1905 device interface InterfaceId=%s\n", macAddrStr);

   while((ret = cmsObj_getNextInSubTreeFlags(MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE_IFC,
                                             pParentIidStack, &iidStack,
                                             OGF_NO_VALUE_UPDATE, (void **) &ieee1905DevIfcObj)) == CMSRET_SUCCESS)
   {
      if(!cmsUtl_strcasecmp(macAddrStr, ieee1905DevIfcObj->interfaceId))
      {
         i5TraceInfo("Found a IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}.Interface.{i}. = %s\n", ieee1905DevIfcObj->interfaceId);
         break;
      }
      cmsObj_free((void **) &ieee1905DevIfcObj);
   }

   /* new one IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}.Interface.{i}. */
   if(ret != CMSRET_SUCCESS)
   {
      i5TraceError("IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}.Interface.{i}. = %s does not exist\n", ieee1905DevIfcObj->interfaceId);
   }
   else
   {
      cmsObj_deleteInstance(MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE_IFC, &iidStack);
   }

   if(ieee1905DevIfcObj)
   {
      cmsObj_free((void **) &ieee1905DevIfcObj);
   }
   return ret;
}

/** Update IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}.Interface.{i}.
 *  This function is called by i5CmsMdmUpdateNetworkTopologyDev().
 *  @param pParentIidStack      (IN) MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE iidstack.
 *  @param pInterface           (IN) neighbor IEEE1905 device's interface.
 *  @return CmsRet enum.
 */
static CmsRet _i5CmsMdmUpdateNetworkTopologyDevIfcOfParent(const InstanceIdStack *pParentIidStack, const i5_dm_interface_type *pInterface)
{
   CmsRet ret = CMSRET_SUCCESS;
   InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
   Dev2Ieee1905AlNetworkTopologyIeee1905DeviceIfcObject *ieee1905DevIfcObj = NULL;
   char macAddrStr[I5_MAC_STR_BUF_LEN] = {0};

   if(pInterface == NULL)
   {
      i5TraceError("pInterface is NULL\n");
      return CMSRET_INVALID_ARGUMENTS;
   }
   snprintf(macAddrStr, I5_MAC_STR_BUF_LEN, I5_MAC_DELIM_FMT, I5_MAC_PRM(pInterface->InterfaceId));
   i5TraceInfo("update ieee1905 device interface InterfaceId=%s\n", macAddrStr);

   while((ret = cmsObj_getNextInSubTreeFlags(MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE_IFC,
                                             pParentIidStack, &iidStack,
                                             OGF_NO_VALUE_UPDATE, (void **) &ieee1905DevIfcObj)) == CMSRET_SUCCESS)
   {
      if(!cmsUtl_strcasecmp(macAddrStr, ieee1905DevIfcObj->interfaceId))
      {
         i5TraceInfo("Found a IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}.Interface.{i}. = %s\n", ieee1905DevIfcObj->interfaceId);
         break;
      }
      cmsObj_free((void **) &ieee1905DevIfcObj);
   }

   /* new one IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}.Interface.{i}. */
   if(ret != CMSRET_SUCCESS)
   {
      iidStack = *pParentIidStack;
      if((ret = cmsObj_addInstance(MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE_IFC, &iidStack)) != CMSRET_SUCCESS)
      {
         i5TraceError("Add IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}.Interface.{i}. fails, ret=%d\n", ret);
         goto error;
      }
      if((ret = cmsObj_get(MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE_IFC, &iidStack,
                  OGF_NO_VALUE_UPDATE, (void **) &ieee1905DevIfcObj)) != CMSRET_SUCCESS)
      {
         i5TraceError("Get IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}. fails, ret=%d\n", ret);
         goto error;
      }
   }
   CMSMEM_REPLACE_STRING(ieee1905DevIfcObj->interfaceId, macAddrStr);
   CMSMEM_REPLACE_STRING(ieee1905DevIfcObj->mediaType, _i5CmsMdmGetMdmStringFromMediaType(pInterface->MediaType));

   if((ret = cmsObj_setFlags(ieee1905DevIfcObj, &iidStack, OSF_NO_RCL_CALLBACK)))
   {
      i5TraceError("cmsObj_set IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}.Interface.{i}. fails, ret=%d\n", ret);
      goto error;
   }

error:
   if(ieee1905DevIfcObj)
      cmsObj_free((void **) &ieee1905DevIfcObj);
   return ret;

}

static CmsRet _i5CmsMdmRemoveNetworkTopologyDevNeighborOfParent(const InstanceIdStack *pParentIidStack,
                                                            const i5_dm_1905_neighbor_type *pNeighbor)
{
   CmsRet ret = CMSRET_SUCCESS;
   InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
   Dev2Ieee1905AlNetworkTopologyIeee1905DeviceIeee1905NeighborObject *ieee1905DevNeighborObj = NULL;
   char ieee1905MacAddrStr[I5_MAC_STR_BUF_LEN] = {0};
   char *intfFullpath = NULL;

   if(pNeighbor == NULL)
   {
      i5TraceError("pNeighbor is NULL\n");
      return CMSRET_INVALID_ARGUMENTS;
   }
   if((intfFullpath = _i5CmsMdmGetNetworkTopologyDevIfcFullPath(pParentIidStack, pNeighbor->LocalInterfaceId)) == NULL)
   {
      i5TraceError("cannot get interface full path\n");
      return CMSRET_INVALID_ARGUMENTS;
   }

   snprintf(ieee1905MacAddrStr, I5_MAC_STR_BUF_LEN, I5_MAC_DELIM_FMT, I5_MAC_PRM(pNeighbor->Ieee1905Id));
   i5TraceInfo("update ieee1905 device neighbor LocalInterface=%s, NeighborId=%s\n", intfFullpath, ieee1905MacAddrStr);

   while((ret = cmsObj_getNextInSubTreeFlags(MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE_IEEE1905_NEIGHBOR,
                                             pParentIidStack, &iidStack,
                                             OGF_NO_VALUE_UPDATE, (void **) &ieee1905DevNeighborObj)) == CMSRET_SUCCESS)
   {
      if(!cmsUtl_strcasecmp(intfFullpath, ieee1905DevNeighborObj->localInterface) &&
         !cmsUtl_strcasecmp(ieee1905MacAddrStr, ieee1905DevNeighborObj->neighborDeviceId))
      {
         i5TraceInfo("Found a IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}.IEEE1905Neighbor.{i}.\n");
         break;
      }
      cmsObj_free((void **) &ieee1905DevNeighborObj);
   }

   /* new one IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}.Interface.{i}. */
   if(ret != CMSRET_SUCCESS)
   {
      i5TraceError("IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}.IEEE1905Neighbor.{i}. does not exist\n");
   }
   else
   {
      cmsObj_deleteInstance(MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE_IEEE1905_NEIGHBOR, &iidStack);
   }

   if(intfFullpath)
   {
      CMSMEM_FREE_BUF_AND_NULL_PTR(intfFullpath);
   }
   if(ieee1905DevNeighborObj)
   {
      cmsObj_free((void **) &ieee1905DevNeighborObj);
   }
   return ret;
}

/** Update IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}.IEEE1905Neighbor.{i}.
 *  This function is called by i5CmsMdmUpdateNetworkTopologyDev().
 *  @param pParentIidStack      (IN) MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE iidstack.
 *  @param pNeighbor            (IN) neighbor IEEE1905 device's neighbor.
 *  @return CmsRet enum.
 *          CMSRET_SUCCESS - okay, new Neighbor created
 *          CMSRET_SUCCESS_OBJECT_UNCHANGED - okay, was update of old neighbor
 *          CMSRET_xxxxxx - failure
 */
static CmsRet _i5CmsMdmUpdateNetworkTopologyDevNeighborOfParent(const InstanceIdStack *pParentIidStack,
                                                    const i5_dm_1905_neighbor_type *pNeighbor)
{
   CmsRet ret = CMSRET_SUCCESS;
   char oldNeighbor = 0;
   InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
   InstanceIdStack iidStackMetric = EMPTY_INSTANCE_ID_STACK;
   Dev2Ieee1905AlNetworkTopologyIeee1905DeviceIeee1905NeighborObject *ieee1905DevNeighborObj = NULL;
   _Dev2Ieee1905AlNetworkTopologyIeee1905DeviceIeee1905NeighborMetricObject *metricObj = NULL;
   char ieee1905MacAddrStr[I5_MAC_STR_BUF_LEN] = {0};
   char *intfFullpath = NULL;
   char longVersionMac[I5_MAC_STR_BUF_LEN] = "";

   if(pNeighbor == NULL)
   {
      i5TraceError("pNeighbor is NULL\n");
      return CMSRET_INVALID_ARGUMENTS;
   }
   if((intfFullpath = _i5CmsMdmGetNetworkTopologyDevIfcFullPath(pParentIidStack, pNeighbor->LocalInterfaceId)) == NULL)
   {
      i5TraceError("cannot get interface full path\n");
      return CMSRET_INVALID_ARGUMENTS;
   }

   snprintf(ieee1905MacAddrStr, I5_MAC_STR_BUF_LEN, I5_MAC_DELIM_FMT, I5_MAC_PRM(pNeighbor->Ieee1905Id));

   while((ret = cmsObj_getNextInSubTreeFlags(MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE_IEEE1905_NEIGHBOR,
                                             pParentIidStack, &iidStack,
                                             OGF_NO_VALUE_UPDATE, (void **) &ieee1905DevNeighborObj)) == CMSRET_SUCCESS)
   {
      if(!cmsUtl_strcasecmp(intfFullpath, ieee1905DevNeighborObj->localInterface) &&
         !cmsUtl_strcasecmp(ieee1905MacAddrStr, ieee1905DevNeighborObj->neighborDeviceId))
      {
         oldNeighbor = 1;
         break;
      }
      cmsObj_free((void **) &ieee1905DevNeighborObj);
   }

   /* new one IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}.Interface.{i}. */
   if(ret != CMSRET_SUCCESS)
   {
      iidStack = *pParentIidStack;
      if((ret = cmsObj_addInstance(MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE_IEEE1905_NEIGHBOR, &iidStack)) != CMSRET_SUCCESS)
      {
         i5TraceError("Add IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}.IEEE1905Neighbor.{i}. fails, ret=%d\n", ret);
         goto error;
      }
      if((ret = cmsObj_get(MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE_IEEE1905_NEIGHBOR, &iidStack,
                  OGF_NO_VALUE_UPDATE, (void **) &ieee1905DevNeighborObj)) != CMSRET_SUCCESS)
      {
         i5TraceError("Get IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}.IEEE1905Neighbor.{i}., ret=%d\n", ret);
         goto error;
      }
   }
   CMSMEM_REPLACE_STRING(ieee1905DevNeighborObj->localInterface, intfFullpath);
   CMSMEM_REPLACE_STRING(ieee1905DevNeighborObj->neighborDeviceId, ieee1905MacAddrStr);

   if((ret = cmsObj_setFlags(ieee1905DevNeighborObj, &iidStack, OSF_NO_RCL_CALLBACK)))
   {
      i5TraceError("cmsObj_set IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}.IEEE1905Neighbor.{i}. fails, ret=%d\n", ret);
      goto error;
   }

   /* Fetch the Link Metric Object, or create it */
   if((ret = cmsObj_getNextInSubTreeFlags(MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE_IEEE1905_NEIGHBOR_METRIC,
               &iidStack,
               &iidStackMetric,
               OGF_NO_VALUE_UPDATE, (void **) &metricObj)) != CMSRET_SUCCESS)
   {
      if((ret = cmsObj_addInstance(MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE_IEEE1905_NEIGHBOR_METRIC, &iidStack)) != CMSRET_SUCCESS)
      {
         i5TraceError("Add IEEE1905.AL.Device.{i}.Neighbor{i}.Metric fails, ret=%d\n", ret);
         goto error;
      }
      if((ret = cmsObj_get(MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE_IEEE1905_NEIGHBOR_METRIC, &iidStack,
                  OGF_NO_VALUE_UPDATE, (void **) &metricObj)) != CMSRET_SUCCESS)
      {
         i5TraceError("Get IEEE1905.AL.Device.{i}.Neighbor{i}.Metric, ret=%d\n", ret);
         goto error;
      }
      iidStackMetric = iidStack;
   }

   /* Update/Set the remote Device's MAC interface Address in the Metric Object */
   snprintf(longVersionMac, I5_MAC_STR_BUF_LEN, I5_MAC_DELIM_FMT, I5_MAC_PRM(pNeighbor->NeighborInterfaceId));
   CMSMEM_REPLACE_STRING (metricObj->neighborMACAddress, longVersionMac);
   metricObj->IEEE802dot1Bridge = pNeighbor->IntermediateLegacyBridge;

   if((ret = cmsObj_setFlags(metricObj, &iidStackMetric, OSF_NO_RCL_CALLBACK)))
   {
      i5TraceError("cmsObj_set IEEE1905.AL.Device.{i}.Neighbor{i}.Metric fails, ret=%d\n", ret);
      goto error;
   }

error:
   if(intfFullpath)
      CMSMEM_FREE_BUF_AND_NULL_PTR(intfFullpath);
   if(ieee1905DevNeighborObj)
      cmsObj_free((void **) &ieee1905DevNeighborObj);
   if(metricObj)
      cmsObj_free((void **) &metricObj);

   if ((ret == CMSRET_SUCCESS) && (oldNeighbor))
   {
     return CMSRET_SUCCESS_OBJECT_UNCHANGED;
   }
   return ret;
}

static CmsRet _i5CmsMdmRemoveNetworkTopologyDevLegacyNeighborOfParent(const InstanceIdStack *pParentIidStack,
                                                                  const i5_dm_legacy_neighbor_type *pLegacy)
{
   CmsRet ret = CMSRET_SUCCESS;
   InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
   Dev2Ieee1905AlNetworkTopologyIeee1905DeviceNonIeee1905NeighborObject *ieee1905DevLegacyNeighborObj = NULL;
   char MacAddrStr[I5_MAC_STR_BUF_LEN] = {0};
   char *intfFullpath = NULL;

   if(pLegacy == NULL)
   {
      i5TraceError("pLegacy is NULL");
      return CMSRET_INVALID_ARGUMENTS;
   }
   if((intfFullpath = _i5CmsMdmGetNetworkTopologyDevIfcFullPath(pParentIidStack, pLegacy->LocalInterfaceId)) == NULL)
   {
      i5TraceError("cannot get interface full path");
      return CMSRET_INVALID_ARGUMENTS;
   }

   snprintf(MacAddrStr, I5_MAC_STR_BUF_LEN, I5_MAC_DELIM_FMT, I5_MAC_PRM(pLegacy->NeighborInterfaceId));
   i5TraceInfo("update ieee1905 device neighbor LocalInterface=%s, NeighborIntfId=%s", intfFullpath, MacAddrStr);

   while((ret = cmsObj_getNextInSubTreeFlags(MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE_NON_IEEE1905_NEIGHBOR,
                                             pParentIidStack, &iidStack,
                                             OGF_NO_VALUE_UPDATE, (void **) &ieee1905DevLegacyNeighborObj)) == CMSRET_SUCCESS)
   {
      if(!cmsUtl_strcasecmp(intfFullpath, ieee1905DevLegacyNeighborObj->localInterface) &&
         !cmsUtl_strcasecmp(MacAddrStr, ieee1905DevLegacyNeighborObj->neighborInterfaceId))
      {
         i5TraceInfo("Found a IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}.NonIEEE1905Neighbor.{i}.");
         break;
      }
      cmsObj_free((void **) &ieee1905DevLegacyNeighborObj);
   }

   /* new one IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}.NonIEEE1905Neighbor.{i}. */
   if(ret != CMSRET_SUCCESS)
   {
      i5TraceInfo("could not find that interface");
   }
   else
   {
      ret = cmsObj_deleteInstance (MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE_NON_IEEE1905_NEIGHBOR, &iidStack);
      if (ret != CMSRET_SUCCESS)
      {
         i5TraceInfo("deletion of interface failed.");
      }
   }

   if (ieee1905DevLegacyNeighborObj)
   {
      cmsObj_free((void **) &ieee1905DevLegacyNeighborObj);
   }
   return ret;

}

/** Update IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}.NonIEEE1905Neighbor.{i}.
 *  This function is called by i5CmsMdmUpdateNetworkTopologyDev().
 *  @param pParentIidStack      (IN) MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE iidstack.
 *  @param pLegacy              (IN) neighbor IEEE1905 device's legacy neighbor.
 *  @return CmsRet enum.
 */
static CmsRet _i5CmsMdmUpdateNetworkTopologyDevLegacyNeighborOfParent(const InstanceIdStack *pParentIidStack,
                                                          const i5_dm_legacy_neighbor_type *pLegacy)
{
   CmsRet ret = CMSRET_SUCCESS;
   InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
   Dev2Ieee1905AlNetworkTopologyIeee1905DeviceNonIeee1905NeighborObject *ieee1905DevLegacyNeighborObj = NULL;
   char MacAddrStr[I5_MAC_STR_BUF_LEN] = {0};
   char *intfFullpath = NULL;

   if(pLegacy == NULL)
   {
      i5TraceError("pLegacy is NULL");
      return CMSRET_INVALID_ARGUMENTS;
   }
   if((intfFullpath = _i5CmsMdmGetNetworkTopologyDevIfcFullPath(pParentIidStack, pLegacy->LocalInterfaceId)) == NULL)
   {
      i5TraceError("cannot get interface full path");
      return CMSRET_INVALID_ARGUMENTS;
   }

   snprintf(MacAddrStr, I5_MAC_STR_BUF_LEN, I5_MAC_DELIM_FMT, I5_MAC_PRM(pLegacy->NeighborInterfaceId));
   i5TraceInfo("update ieee1905 device neighbor LocalInterface=%s, NeighborIntfId=%s", intfFullpath, MacAddrStr);

   while((ret = cmsObj_getNextInSubTreeFlags(MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE_NON_IEEE1905_NEIGHBOR,
                                             pParentIidStack, &iidStack,
                                             OGF_NO_VALUE_UPDATE, (void **) &ieee1905DevLegacyNeighborObj)) == CMSRET_SUCCESS)
   {
      if(!cmsUtl_strcasecmp(intfFullpath, ieee1905DevLegacyNeighborObj->localInterface) &&
         !cmsUtl_strcasecmp(MacAddrStr, ieee1905DevLegacyNeighborObj->neighborInterfaceId))
      {
         i5TraceInfo("Found a IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}.NonIEEE1905Neighbor.{i}.");
         break;
      }
      cmsObj_free((void **) &ieee1905DevLegacyNeighborObj);
   }

   /* new one IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}.NonIEEE1905Neighbor.{i}. */
   if(ret != CMSRET_SUCCESS)
   {
      iidStack = *pParentIidStack;
      if((ret = cmsObj_addInstance(MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE_NON_IEEE1905_NEIGHBOR, &iidStack)) != CMSRET_SUCCESS)
      {
         i5TraceError("Add IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}.NonIEEE1905Neighbor.{i}. fails, ret=%d", ret);
         goto error;
      }
      if((ret = cmsObj_get(MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE_NON_IEEE1905_NEIGHBOR, &iidStack,
                  OGF_NO_VALUE_UPDATE, (void **) &ieee1905DevLegacyNeighborObj)) != CMSRET_SUCCESS)
      {
         i5TraceError("Get IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}.NonIEEE1905Neighbor.{i}., ret=%d", ret);
         goto error;
      }
   }
   CMSMEM_REPLACE_STRING(ieee1905DevLegacyNeighborObj->localInterface, intfFullpath);
   CMSMEM_REPLACE_STRING(ieee1905DevLegacyNeighborObj->neighborInterfaceId, MacAddrStr);

   if((ret = cmsObj_setFlags(ieee1905DevLegacyNeighborObj, &iidStack, OSF_NO_RCL_CALLBACK)))
   {
      i5TraceError("cmsObj_set IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}.NonIEEE1905Neighbor.{i}. fails, ret=%d", ret);
      goto error;
   }

error:
   if(intfFullpath)
      CMSMEM_FREE_BUF_AND_NULL_PTR(intfFullpath);
   if(ieee1905DevLegacyNeighborObj)
      cmsObj_free((void **) &ieee1905DevLegacyNeighborObj);
   return ret;

}

static CmsRet _i5CmsMdmGetIidsForLocalNeighbor(const i5_dm_1905_neighbor_type * const localNeighbor,
                                                InstanceIdStack *ifcIidStack, InstanceIdStack *linkIidStack,
                                                char *intfMacAddrStr, char *ieee1905MacAddrStr,
                                                char *localIntfMacAddrStr,
                                                Dev2Ieee1905AlIfcLinkObject **ieee1905IfcLinkObj)
{
   Dev2Ieee1905AlIfcObject *ieee1905IfcObj = NULL;

   snprintf(intfMacAddrStr, I5_MAC_STR_BUF_LEN, I5_MAC_DELIM_FMT, I5_MAC_PRM(localNeighbor->NeighborInterfaceId));
   snprintf(ieee1905MacAddrStr, I5_MAC_STR_BUF_LEN, I5_MAC_DELIM_FMT, I5_MAC_PRM(localNeighbor->Ieee1905Id));
   snprintf(localIntfMacAddrStr, I5_MAC_STR_BUF_LEN, I5_MAC_DELIM_FMT, I5_MAC_PRM(localNeighbor->LocalInterfaceId));
   i5TraceInfo("checking local ieee1905 interface {%s} link, NeighInterfaceId=%s, NeighIeee1905Id=%s\n",
                     localNeighbor->localIfname, intfMacAddrStr, ieee1905MacAddrStr);

   while(cmsObj_getNextFlags(MDMOID_DEV2_IEEE1905_AL_IFC, ifcIidStack,
                             OGF_NO_VALUE_UPDATE, (void **) &ieee1905IfcObj) == CMSRET_SUCCESS)
   {
      if(!cmsUtl_strcasecmp(localIntfMacAddrStr, ieee1905IfcObj->interfaceId))
      {
         while(cmsObj_getNextInSubTreeFlags(MDMOID_DEV2_IEEE1905_AL_IFC_LINK, ifcIidStack, linkIidStack,
                                            OGF_NO_VALUE_UPDATE, (void **)ieee1905IfcLinkObj) == CMSRET_SUCCESS)
         {
            if(!cmsUtl_strcasecmp(intfMacAddrStr, (*ieee1905IfcLinkObj)->interfaceId) &&
               !cmsUtl_strcasecmp(ieee1905MacAddrStr, (*ieee1905IfcLinkObj)->IEEE1905Id))
            {
               i5TraceInfo("Found a IEEE1905.AL.Interface.{i}.Link.{i}\n");
               cmsObj_free((void **) &ieee1905IfcObj);
               return CMSRET_SUCCESS;
            }
            cmsObj_free((void **) ieee1905IfcLinkObj);
         }
         cmsObj_free((void **) &ieee1905IfcObj);
         return CMSRET_OBJECT_NOT_FOUND;
      }
      cmsObj_free((void **) &ieee1905IfcObj);
   }
   return CMSRET_INVALID_ARGUMENTS;
}

static char * _i5CmsMdmConvertBridgeTupleToString(const InstanceIdStack *pParentIidStack, i5_dm_bridging_tuple_info_type *bridge)
{
   int   index;
   char *outputBuf;
   int   objectLen;
   int   pathLen;
   char *fullPath;

   if ( bridge->forwardingInterfaceListNumEntries <= 0 )
   {
      return NULL;
   }

   outputBuf = malloc(bridge->forwardingInterfaceListNumEntries * (I5_CMSMDM_MAX_BRIDGING_TUPLE_ITEM_LENGTH + 2));
   if ( NULL == outputBuf )
   {
      return NULL;
   }

   objectLen = 0;
   for ( index = 0; index < bridge->forwardingInterfaceListNumEntries; index++) {
     fullPath = _i5CmsMdmGetNetworkTopologyDevIfcFullPath(pParentIidStack, &(bridge->ForwardingInterfaceList[index*MAC_ADDR_LEN]) );
     if (fullPath != NULL) {
       pathLen = strlen(fullPath);
       if ( pathLen > I5_CMSMDM_MAX_BRIDGING_TUPLE_ITEM_LENGTH )
       {
          continue;
       }

       if ( (objectLen + pathLen + 1) > I5_CMSMDM_MAX_BRIDGING_TUPLE_TOTAL_LENGTH )
       {
         break;
       }
       if ( objectLen != 0 )
       {
          outputBuf[objectLen] = ',';
          objectLen += 1;
       }
       strcpy(&outputBuf[objectLen], fullPath);
       objectLen += pathLen;
       outputBuf[objectLen] = '\0';
     }
   }
   return outputBuf;
}

/* Loop through bridging tuples until the index'th entry (zero-based!) */
/* Caller must already have CMS LOCK */
static CmsRet _i5CmsMdmRemoveBridgingTupleWithParent(InstanceIdStack *pParentIidStack, int index)
{
   CmsRet ret = CMSRET_SUCCESS;
   InstanceIdStack iidStackToBeRemoved = EMPTY_INSTANCE_ID_STACK;
   Dev2Ieee1905AlNetworkTopologyIeee1905DeviceBridgingTupleObject *ieee1905DevBridgingTupleObj = NULL;

   i5TraceInfo("Remove Bridging tuple\n");
   while ((ret = cmsObj_getNextInSubTreeFlags(MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE_BRIDGING_TUPLE,
                                              pParentIidStack, &iidStackToBeRemoved,
                                              OGF_NO_VALUE_UPDATE, (void **) &ieee1905DevBridgingTupleObj)) == CMSRET_SUCCESS)
   {
      cmsObj_free((void **) &ieee1905DevBridgingTupleObj);
      if ( index == 0 )
      {
         break;
      }
      index--;
   }

   i5TraceInfo("Found a Bridging tuple ... deleting \n");
   ret = cmsObj_deleteInstance (MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE_BRIDGING_TUPLE, &iidStackToBeRemoved);
   if (ret != CMSRET_SUCCESS)
   {
      i5TraceError("deletion of interface failed.");
   }

   return ret;
}

/* Loop through bridging tuples until the index'th entry (zero-based!) */
/* Caller must already have CMS LOCK */
static CmsRet _i5CmsMdmUpdateBridgingTupleWithParent(const InstanceIdStack *pParentIidStack, i5_dm_bridging_tuple_info_type *bridge, int index)
{
   CmsRet ret = CMSRET_SUCCESS;
   InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
   Dev2Ieee1905AlNetworkTopologyIeee1905DeviceBridgingTupleObject *ieee1905DevBridgingTupleObj = NULL;
   char *buf;

   i5TraceInfo("Update Bridging tuple\n");

   while ((ret = cmsObj_getNextInSubTreeFlags(MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE_BRIDGING_TUPLE,
                                       pParentIidStack, &iidStack,
                                       OGF_NO_VALUE_UPDATE, (void **) &ieee1905DevBridgingTupleObj)) == CMSRET_SUCCESS)
   {
      if ( 0 == index )
      {
         break;
      }
      cmsObj_free((void **)&ieee1905DevBridgingTupleObj);
      index--;
   }

   /* if matching object was not found then add it */
   if ( ret != CMSRET_SUCCESS )
   {
      iidStack = *pParentIidStack;
      if((ret = cmsObj_addInstance(MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE_BRIDGING_TUPLE, &iidStack)) != CMSRET_SUCCESS)
      {
         i5TraceError("Add IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}.BridgingTuple.{i}. fails, ret=%d", ret);
         return ret;
      }

      if((ret = cmsObj_get(MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE_BRIDGING_TUPLE, &iidStack,
                           OGF_NO_VALUE_UPDATE, (void **) &ieee1905DevBridgingTupleObj)) != CMSRET_SUCCESS)
      {
         i5TraceError("Get IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}.BridgingTuple.{i}., ret=%d", ret);
         return ret;
      }
   }

   buf = _i5CmsMdmConvertBridgeTupleToString(pParentIidStack, bridge);
   if ( buf )
   {
      CMSMEM_REPLACE_STRING_FLAGS(ieee1905DevBridgingTupleObj->interfaceList, buf, ALLOC_ZEROIZE);
      free(buf);

      ret = cmsObj_setFlags(ieee1905DevBridgingTupleObj, &iidStack, OSF_NO_RCL_CALLBACK);
      if(ret != CMSRET_SUCCESS)
      {
         i5TraceError("cmsObj_set IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}.BridgingTuple.{i}. fails, ret=%d", ret);
      }
   }
   cmsObj_free((void **) &ieee1905DevBridgingTupleObj);

   return ret;
}

/* Caller must already have CMS LOCK */
/* This function calls itself */
static void _i5CmsMdmBridgeTupleUpdateReentrant(const InstanceIdStack *pParentIidStack,
                                                i5_dm_bridging_tuple_info_type *bridge,
                                                int bridgeTupleCount,
                                                int index)
{
  i5_dm_bridging_tuple_info_type* nextBridge;

  i5TraceInfo("\n");

  if (bridge == NULL) {
    return;
  }
  nextBridge = (i5_dm_bridging_tuple_info_type*) bridge->ll.next;
  if (nextBridge != NULL) {
    /* Here we go again */
    _i5CmsMdmBridgeTupleUpdateReentrant (pParentIidStack, nextBridge, bridgeTupleCount, index+1);
  }

  _i5CmsMdmUpdateBridgingTupleWithParent(pParentIidStack, bridge, (bridgeTupleCount - index - 1));
}

static void _i5CmsMdmUpdateAllBridgingTuples(const i5_dm_device_type *pDev)
{
   CmsRet ret = CMSRET_SUCCESS;
   InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
   Dev2Ieee1905AlNetworkTopologyIeee1905DeviceObject *ieee1905DevObj = NULL;

   i5TraceInfo("\n");

   ret = _i5CmsMdmGetDeviceIidFromMac(pDev->DeviceId, &iidStack, &ieee1905DevObj);
   if(ret != CMSRET_SUCCESS)
   {
      i5TraceInfo("No such Device\n");
      return;
   }
   else {
      i5_dm_bridging_tuple_info_type* bridge = (i5_dm_bridging_tuple_info_type *)pDev->bridging_tuple_list.ll.next;
      _i5CmsMdmBridgeTupleUpdateReentrant(&iidStack, bridge, pDev->BridgingTuplesNumberOfEntries, 0);
      cmsObj_free((void**)&ieee1905DevObj);
   }
}

static int _i5CmsMdmUpdateNetworkTopologyDev(const i5_dm_device_type *pDev)
{
   CmsRet ret = CMSRET_SUCCESS;
   InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
   Dev2Ieee1905AlNetworkTopologyIeee1905DeviceObject *ieee1905DevObj = NULL;
   char macAddrStr[I5_MAC_STR_BUF_LEN] = {0};

   snprintf(macAddrStr, I5_MAC_STR_BUF_LEN, I5_MAC_DELIM_FMT, I5_MAC_PRM(pDev->DeviceId));
   i5TraceInfo("update ieee1905 device DeviceId=%s\n", macAddrStr);

   ret = _i5CmsMdmGetDeviceIidFromMac(pDev->DeviceId, &iidStack, &ieee1905DevObj);

   /* new one IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}. */
   if(ret != CMSRET_SUCCESS)
   {
      i5TraceInfo("Creating new device\n");
      INIT_INSTANCE_ID_STACK(&iidStack);
      if((ret = cmsObj_addInstance(MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE, &iidStack)) != CMSRET_SUCCESS)
      {
         i5TraceError("Add IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}. fails, ret=%d\n", ret);
      }
      else if((ret = cmsObj_get(MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE, &iidStack,
                  OGF_NO_VALUE_UPDATE, (void **) &ieee1905DevObj)) != CMSRET_SUCCESS)
      {
         i5TraceError("Get IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}. fails, ret=%d\n", ret);
      }
   }

   /* At this point, we should have either found the Device, or created a new one */
   if (ret == CMSRET_SUCCESS) {
     ieee1905DevObj->version = pDev->Version;
     CMSMEM_REPLACE_STRING(ieee1905DevObj->friendlyName, pDev->friendlyName);
     CMSMEM_REPLACE_STRING(ieee1905DevObj->IEEE1905Id, macAddrStr);

     if((ret = cmsObj_setFlags(ieee1905DevObj, &iidStack, OSF_NO_RCL_CALLBACK)) != CMSRET_SUCCESS)
     {
        i5TraceError("cmsObj_set IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}. fails, ret=%d\n", ret);
     }
   }

   if(ieee1905DevObj)
   {
      cmsObj_free((void **) &ieee1905DevObj);
   }

   return ret;
}

static int _i5CmsMdmUpdateNetworkTopologyDevIfc(const i5_dm_device_type *pDev,
                                                const i5_dm_interface_type *pInterface)
{
   CmsRet ret = CMSRET_SUCCESS;
   InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
   Dev2Ieee1905AlNetworkTopologyIeee1905DeviceObject *ieee1905DevObj = NULL;

   i5TraceInfo("Adding" I5_MAC_DELIM_FMT " to dev " I5_MAC_DELIM_FMT "\n", I5_MAC_PRM(pDev->DeviceId), I5_MAC_PRM(pInterface->InterfaceId));

   ret = _i5CmsMdmGetDeviceIidFromMac(pDev->DeviceId, &iidStack, &ieee1905DevObj);

   /* new one IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}. */
   if (ret != CMSRET_SUCCESS)
   {
      // There was no device object
      i5TraceError("IEEE1905 device does not exist.\n");
   }
   else
   {
      ret = _i5CmsMdmUpdateNetworkTopologyDevIfcOfParent (&iidStack, pInterface);
      if(ret != CMSRET_SUCCESS)
      {
         i5TraceError("update of IEEE1905 Interface failed.\n");
      }
      cmsObj_free((void **) &ieee1905DevObj);
   }
   return ret;
}

static int _i5CmsMdmUpdateNetworkTopologyDevNeighbor(const i5_dm_device_type *pDev,
                                                     const i5_dm_1905_neighbor_type *pNeighbor)
{
   CmsRet ret = CMSRET_SUCCESS;
   InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
   Dev2Ieee1905AlNetworkTopologyIeee1905DeviceObject *ieee1905DevObj = NULL;

   i5TraceError("Adding" I5_MAC_DELIM_FMT " to dev " I5_MAC_DELIM_FMT "\n", I5_MAC_PRM(pNeighbor->Ieee1905Id), I5_MAC_PRM(pDev->DeviceId));

   ret = _i5CmsMdmGetDeviceIidFromMac(pDev->DeviceId, &iidStack, &ieee1905DevObj);

   /* new one IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}. */
   if (ret != CMSRET_SUCCESS)
   {
      // There was no device object
      i5TraceError("IEEE1905 device does not exist.\n");
   }
   else
   {
      ret = _i5CmsMdmUpdateNetworkTopologyDevNeighborOfParent (&iidStack, pNeighbor);
      if(ret != CMSRET_SUCCESS)
      {
         i5TraceError("update of IEEE1905 Neighbor failed.\n");
      }
      cmsObj_free((void **) &ieee1905DevObj);
   }

   return ret;
}

static int _i5CmsMdmUpdateNetworkTopologyDevLegacyNeighbor(const i5_dm_device_type *pDev,
                                                           const i5_dm_legacy_neighbor_type *pLegacy)
{
   CmsRet ret = CMSRET_SUCCESS;
   InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
   Dev2Ieee1905AlNetworkTopologyIeee1905DeviceObject *ieee1905DevObj = NULL;

   i5TraceInfo("\n");

   ret = _i5CmsMdmGetDeviceIidFromMac(pDev->DeviceId, &iidStack, &ieee1905DevObj);

   /* new one IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}. */
   if (ret != CMSRET_SUCCESS)
   {
      // There was no object to delete
      i5TraceError("IEEE1905 device does not exist.\n");
      ret = 0;
   }
   else
   {
      ret = _i5CmsMdmUpdateNetworkTopologyDevLegacyNeighborOfParent (&iidStack, pLegacy);
      cmsObj_free((void**) &ieee1905DevObj);
   }

   return ret;
}

static CmsRet _i5CmsMdmLocalInterfaceUpdate(const i5_dm_interface_type * const localInterface)
{
   CmsRet ret = CMSRET_SUCCESS;
   InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
   Dev2Ieee1905AlIfcObject *ieee1905IfcObj = NULL;
   char macAddrStr[I5_MAC_STR_BUF_LEN] = {0};

   i5TraceInfo("\n");

   snprintf(macAddrStr, I5_MAC_STR_BUF_LEN, I5_MAC_DELIM_FMT, I5_MAC_PRM(localInterface->InterfaceId));
   i5TraceInfo("update local ieee1905 interface {%s}, InterfaceId=%s, mediatype=0x%x\n",
                       localInterface->wlParentName, macAddrStr, localInterface->MediaType);

   while((ret = cmsObj_getNextFlags(MDMOID_DEV2_IEEE1905_AL_IFC, &iidStack,
                                    OGF_NO_VALUE_UPDATE, (void **) &ieee1905IfcObj)) == CMSRET_SUCCESS)
   {
      if(!cmsUtl_strcasecmp(macAddrStr, ieee1905IfcObj->interfaceId))
      {
         i5TraceInfo("Found a IEEE1905.AL.Interface.{i}. with interfaceId=%s\n", ieee1905IfcObj->interfaceId);
         break;
      }
      cmsObj_free((void **) &ieee1905IfcObj);
   }

   /* new IEEE1905.AL.Interface.{i}. */
   if(ret != CMSRET_SUCCESS)
   {
      INIT_INSTANCE_ID_STACK(&iidStack);
      if((ret = cmsObj_addInstance(MDMOID_DEV2_IEEE1905_AL_IFC, &iidStack)) != CMSRET_SUCCESS)
      {
         i5TraceError("Add IEEE1905.AL.Interface.{i}. fails, ret=%d\n", ret);
         goto error;
      }
      if((ret = cmsObj_get(MDMOID_DEV2_IEEE1905_AL_IFC, &iidStack,
                  OGF_NO_VALUE_UPDATE, (void **) &ieee1905IfcObj)) != CMSRET_SUCCESS)
      {
         i5TraceError("Get IEEE1905.AL.Interface.{i}. fails, ret=%d\n", ret);
         goto error;
      }
   }

   CMSMEM_REPLACE_STRING(ieee1905IfcObj->interfaceId, macAddrStr);
   CMSMEM_REPLACE_STRING(ieee1905IfcObj->status, "Up");
   ieee1905IfcObj->lastChange = 0;
   CMSMEM_REPLACE_STRING(ieee1905IfcObj->mediaType, _i5CmsMdmGetMdmStringFromMediaType(localInterface->MediaType));
   if((ret = cmsObj_setFlags(ieee1905IfcObj, &iidStack, OSF_NO_RCL_CALLBACK)))
   {
      i5TraceError("cmsObj_set IEEE1905.AL.Interface.{i}. fails, ret=%d\n", ret);
      goto error;
   }

error:
   if(ieee1905IfcObj)
   {
      cmsObj_free((void **) &ieee1905IfcObj);
   }

   return ret;
}

static CmsRet _i5CmsMdmLocalNeighborUpdate(const i5_dm_1905_neighbor_type * const localNeighbor)
{
   CmsRet ret = CMSRET_SUCCESS;
   InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
   InstanceIdStack iidStackIfcLink = EMPTY_INSTANCE_ID_STACK;
   InstanceIdStack iidStackIfcLinkMetric = EMPTY_INSTANCE_ID_STACK;
   Dev2Ieee1905AlIfcLinkObject *ieee1905IfcLinkObj = NULL;
   Dev2Ieee1905AlIfcLinkMetricObject *ieee1905IfcLinkMetricObj = NULL;
   char intfMacAddrStr[I5_MAC_STR_BUF_LEN] = {0};
   char ieee1905MacAddrStr[I5_MAC_STR_BUF_LEN] = {0};
   char localIntfMacAddrStr[I5_MAC_STR_BUF_LEN] = {0};
   i5_dm_device_type const *neighborDevice = NULL;

   i5TraceInfo("\n");

   ret = _i5CmsMdmGetIidsForLocalNeighbor(localNeighbor, &iidStack, &iidStackIfcLink,
                                          intfMacAddrStr, ieee1905MacAddrStr, localIntfMacAddrStr,
                                          &ieee1905IfcLinkObj);

   /* new one IEEE1905.AL.Interface.{i}.Link.{i}. */
   if(ret == CMSRET_INVALID_ARGUMENTS)
   {
      i5TraceInfo("Interface not found.  Can not add Link.\n");
      goto error;
   }
   else if (ret == CMSRET_OBJECT_NOT_FOUND)
   {
      i5TraceInfo("New Link.{i} under Interface {%s}\n", localNeighbor->localIfname);
      if((ret = cmsObj_addInstance(MDMOID_DEV2_IEEE1905_AL_IFC_LINK, &iidStack)) != CMSRET_SUCCESS)
      {
         i5TraceError("Add IEEE1905.AL.Interface.{i}.Link{i}. fails, ret=%d\n", ret);
         goto error;
      }
      if((ret = cmsObj_get(MDMOID_DEV2_IEEE1905_AL_IFC_LINK, &iidStack,
                  OGF_NO_VALUE_UPDATE, (void **) &ieee1905IfcLinkObj)) != CMSRET_SUCCESS)
      {
         i5TraceError("Get IEEE1905.AL.Interface.{i}.Link{i}. fails, ret=%d\n", ret);
         goto error;
      }

      iidStackIfcLink = iidStack;
   }

   CMSMEM_REPLACE_STRING(ieee1905IfcLinkObj->interfaceId, intfMacAddrStr);
   CMSMEM_REPLACE_STRING(ieee1905IfcLinkObj->IEEE1905Id, ieee1905MacAddrStr);

   neighborDevice = i5DmDeviceFind(localNeighbor->Ieee1905Id);
   if (neighborDevice) {
      i5_dm_interface_type *remoteInterface= i5DmInterfaceFind(neighborDevice, localNeighbor->NeighborInterfaceId);
      if (remoteInterface) {
         CMSMEM_REPLACE_STRING(ieee1905IfcLinkObj->mediaType,
                               _i5CmsMdmGetMdmStringFromMediaType(remoteInterface->MediaType));
      }
   }
   if((ret = cmsObj_setFlags(ieee1905IfcLinkObj, &iidStackIfcLink, OSF_NO_RCL_CALLBACK)))
   {
      i5TraceError("cmsObj_set IEEE1905.AL.Interface.{i}.Link{i}. fails, ret=%d\n", ret);
      goto error;
   }

   if ( (ret = cmsObj_getNextInSubTreeFlags(MDMOID_DEV2_IEEE1905_AL_IFC_LINK_METRIC,
                                           &iidStackIfcLink, &iidStackIfcLinkMetric,
                                           OGF_NO_VALUE_UPDATE, (void **) &ieee1905IfcLinkMetricObj)) != CMSRET_SUCCESS)

   {
      if((ret = cmsObj_addInstance(MDMOID_DEV2_IEEE1905_AL_IFC_LINK_METRIC, &iidStackIfcLink)) != CMSRET_SUCCESS)
      {
         i5TraceError("Add IEEE1905.AL.Interface.{i}.Link{i}.Metric fails, ret=%d\n", ret);
         goto error;
      }
      if((ret = cmsObj_get(MDMOID_DEV2_IEEE1905_AL_IFC_LINK_METRIC, &iidStackIfcLink,
                           OGF_NO_VALUE_UPDATE, (void **) &ieee1905IfcLinkMetricObj)) != CMSRET_SUCCESS)
      {
         i5TraceError("Get IEEE1905.AL.Interface.{i}.Link{i}.Metric fails, ret=%d\n", ret);
         goto error;
      }
      iidStackIfcLinkMetric = iidStackIfcLink;
   }

   ieee1905IfcLinkMetricObj->IEEE802dot1Bridge = localNeighbor->IntermediateLegacyBridge;

   if((ret = cmsObj_setFlags(ieee1905IfcLinkMetricObj, &iidStackIfcLinkMetric, OSF_NO_RCL_CALLBACK)))
   {
      i5TraceError("cmsObj_set IEEE1905.AL.Interface.{i}.Link{i}. fails, ret=%d\n", ret);
   }

error:
   if(ieee1905IfcLinkObj)
   {
      cmsObj_free((void **) &ieee1905IfcLinkObj);
   }
   if (ieee1905IfcLinkMetricObj)
   {
      cmsObj_free((void **) &ieee1905IfcLinkMetricObj);
   }

   return ret;
}

static int _i5CmsMdmFlushNetworkTopology(void)
{
   CmsRet ret = CMSRET_OBJECT_NOT_FOUND;
   InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
   Dev2Ieee1905AlNetworkTopologyIeee1905DeviceObject *ieee1905DevObj = NULL;

   i5TraceInfo("\n");

   while((ret = cmsObj_getNextFlags(MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE,
                                    &iidStack,
                                    OGF_NO_VALUE_UPDATE, (void **) &ieee1905DevObj)) == CMSRET_SUCCESS)
   {
      ret = cmsObj_deleteInstance(MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE, &iidStack);
      if (ret != CMSRET_SUCCESS)
      {
         i5TraceError("deletion of IEEE1905 device failed.\n");
      }
      cmsObj_free((void **) &ieee1905DevObj);
      INIT_INSTANCE_ID_STACK(&iidStack);
   }

   return 0;
}

static void _i5CmsMdmNetworkTopologyAddObjects(void)
{
   i5_dm_device_type *pDev = NULL;
   CmsRet ret;

   i5Trace("\n");

   pDev = (i5_dm_device_type *)i5_dm_network_topology.device_list.ll.next;
   while(pDev != NULL)
   {
      i5_dm_interface_type *pInterface = pDev->interface_list.ll.next;
      i5_dm_1905_neighbor_type *pNeigh = pDev->neighbor1905_list.ll.next;
      i5_dm_legacy_neighbor_type *pLeg = pDev->legacy_list.ll.next;

      ret = _i5CmsMdmUpdateNetworkTopologyDev(pDev);
      if( ret != CMSRET_SUCCESS)
      {
         i5Trace("i5CmsMdmUpdateNetworkTopologyDev fail, ret=%d\n", ret);
      }
      else
      {
         /* The following are sub-objects of the device, so we only try to create them if the device creation worked */
         while (pInterface != NULL)
         {
            _i5CmsMdmUpdateNetworkTopologyDevIfc(pDev, pInterface);
            pInterface = pInterface->ll.next;
         }
         while (pNeigh != NULL)
         {
            _i5CmsMdmUpdateNetworkTopologyDevNeighbor(pDev, pNeigh);
            pNeigh = pNeigh->ll.next;
         }
         while (pLeg != NULL)
         {
            _i5CmsMdmUpdateNetworkTopologyDevLegacyNeighbor(pDev, pLeg);
            pLeg = pLeg->ll.next;
         }
         _i5CmsMdmUpdateAllBridgingTuples(pDev);
      }
      pDev = pDev->ll.next;
   }

   return;
}

static CmsRet _i5CmsMdmGetInterfaceMacFromPathDescriptor(const char *fullPath, unsigned char *interfaceMac)
{
   CmsRet            ret;
   int               rc;
   MdmPathDescriptor pathDesc=EMPTY_PATH_DESCRIPTOR;
   Dev2Ieee1905AlNetworkTopologyIeee1905DeviceIfcObject *ieee1905DevIfcObj = NULL;

   ret = cmsMdm_fullPathToPathDescriptor(fullPath, &pathDesc);
   if (ret != CMSRET_SUCCESS)
   {
      return ret;
   }

   /* get snooping obj */
   ret = cmsObj_get(pathDesc.oid, &pathDesc.iidStack, OGF_NO_VALUE_UPDATE, (void **)&ieee1905DevIfcObj);
   if (ret != CMSRET_SUCCESS)
   {
      return ret;
   }

   rc = sscanf(ieee1905DevIfcObj->interfaceId, I5_MAC_SCANF, I5_MAC_SCANF_PRM(interfaceMac));
   cmsObj_free( (void **)&ieee1905DevIfcObj);
   if ( rc != MAC_ADDR_LEN )
   {
      return CMSRET_INVALID_PARAM_VALUE;
   }
   return CMSRET_SUCCESS;
}

static void _i5CmsMdmAlAddObjects(void)
{
   i5_dm_device_type *pDev = NULL;
   CmsRet ret;

   i5Trace("\n");

   pDev = i5DmGetSelfDevice();
   if (pDev != NULL)
   {
      i5_dm_interface_type *pInterface = pDev->interface_list.ll.next;
      i5_dm_1905_neighbor_type *pNeigh = pDev->neighbor1905_list.ll.next;

      while (pInterface != NULL)
      {
         ret = _i5CmsMdmLocalInterfaceUpdate(pInterface);
         if ( ret !=CMSRET_SUCCESS )
         {
            i5TraceError("Unable to update local interface\n");
         }
         pInterface = pInterface->ll.next;
      }
      while (pNeigh != NULL)
      {
         ret = _i5CmsMdmLocalNeighborUpdate(pNeigh);
         if ( ret !=CMSRET_SUCCESS )
         {
            i5TraceError("Unable to update local interface\n");
         }
         pNeigh = pNeigh->ll.next;
      }
   }

   return;
}

static void _i5CmsMdmAlRemoveDeletedObjects(void)
{
   CmsRet ret;
   int    rc;
   InstanceIdStack iidStackIfc = EMPTY_INSTANCE_ID_STACK;
   InstanceIdStack iidStackIfc2 = EMPTY_INSTANCE_ID_STACK;
   InstanceIdStack iidStackSub = EMPTY_INSTANCE_ID_STACK;
   InstanceIdStack iidStackSub2 = EMPTY_INSTANCE_ID_STACK;
   unsigned char id[MAC_ADDR_LEN];
   unsigned char id2[MAC_ADDR_LEN];
   Dev2Ieee1905AlIfcObject *ieee1905AlIfcObj = NULL;
   Dev2Ieee1905AlIfcLinkObject *ieee1905AlIfcLinkObj = NULL;
   i5_dm_device_type *pdevice = i5DmGetSelfDevice();

   if ( pdevice == NULL )
   {
      return;
   }

   while((ret = cmsObj_getNextInSubTreeFlags(MDMOID_DEV2_IEEE1905_AL_IFC,
                                             &iidStackIfc, &iidStackSub,
                                             OGF_NO_VALUE_UPDATE, (void **)&ieee1905AlIfcObj)) == CMSRET_SUCCESS)
   {

      rc = sscanf(ieee1905AlIfcObj->interfaceId, I5_MAC_SCANF, I5_MAC_SCANF_PRM(id));
      if ( MAC_ADDR_LEN == rc )
      {
         i5_dm_interface_type *pinterface = i5DmInterfaceFind(pdevice, id);
         if (pinterface == NULL)
         {
            ret = cmsObj_deleteInstance(MDMOID_DEV2_IEEE1905_AL_IFC, &iidStackSub);
            if ( ret != CMSRET_SUCCESS)
            {
               i5TraceError("deletion of IEEE1905 device failed.\n");
            }
            iidStackIfc = iidStackIfc2;
         }
         else
         {
            iidStackIfc2 = iidStackIfc;
            while((ret = cmsObj_getNextInSubTreeFlags(MDMOID_DEV2_IEEE1905_AL_IFC_LINK,
                                                      &iidStackIfc, &iidStackSub,
                                                      OGF_NO_VALUE_UPDATE, (void **)&ieee1905AlIfcLinkObj)) == CMSRET_SUCCESS)
            {
               rc  = sscanf(ieee1905AlIfcLinkObj->IEEE1905Id, I5_MAC_SCANF, I5_MAC_SCANF_PRM(id2));
               if (MAC_ADDR_LEN == rc)
               {
                  i5_dm_1905_neighbor_type *pneighbor = i5Dm1905NeighborFind(pdevice, id, id2);
                  if (pneighbor == NULL)
                  {
                     cmsObj_deleteInstance(MDMOID_DEV2_IEEE1905_AL_IFC_LINK, &iidStackSub);
                     iidStackSub = iidStackSub2;
                  }
                  else
                  {
                     iidStackSub2 = iidStackSub;
                  }
               }
               cmsObj_free((void **) &ieee1905AlIfcLinkObj);
            }
         }
      }
      cmsObj_free((void **)&ieee1905AlIfcObj);
   }
}

static void _i5CmsMdmNetworkTopologyRemoveDeletedObjects(void)
{
   CmsRet ret;
   InstanceIdStack iidStackDev = EMPTY_INSTANCE_ID_STACK;
   InstanceIdStack iidStackDev2 = EMPTY_INSTANCE_ID_STACK;
   InstanceIdStack iidStackSub = EMPTY_INSTANCE_ID_STACK;
   InstanceIdStack iidStackSub2 = EMPTY_INSTANCE_ID_STACK;
   unsigned char id[MAC_ADDR_LEN];
   unsigned char id2[MAC_ADDR_LEN];
   Dev2Ieee1905AlNetworkTopologyIeee1905DeviceObject *ieee1905DevObj;

   i5Trace("\n");

   while((ret = cmsObj_getNextFlags(MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE,
                                    &iidStackDev,
                                    OGF_NO_VALUE_UPDATE, (void **)&ieee1905DevObj)) == CMSRET_SUCCESS)
   {
      int rc = sscanf(ieee1905DevObj->IEEE1905Id, I5_MAC_SCANF, I5_MAC_SCANF_PRM(id));
      if ( MAC_ADDR_LEN == rc )
      {
         i5_dm_device_type *pdevice = i5DmDeviceFind(id);
         if ( NULL == pdevice )
         {
            /* remove device from CMS data model */
            ret = cmsObj_deleteInstance(MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE, &iidStackDev);
            if(ret != CMSRET_SUCCESS)
            {
               i5TraceError("deletion of IEEE1905 device failed.\n");
            }
            iidStackDev = iidStackDev2;
         }
         else
         {
            /* device is okay - need to check interfaces, and neighbors */
            Dev2Ieee1905AlNetworkTopologyIeee1905DeviceIfcObject *ieee1905DevIfcObj = NULL;
            Dev2Ieee1905AlNetworkTopologyIeee1905DeviceIeee1905NeighborObject *ieee1905DevNeighborObj = NULL;
            Dev2Ieee1905AlNetworkTopologyIeee1905DeviceNonIeee1905NeighborObject *ieee1905DevLegacyNeighborObj = NULL;
            Dev2Ieee1905AlNetworkTopologyIeee1905DeviceBridgingTupleObject *ieee1905DevBridgeObj = NULL;
            int brCount;

            iidStackDev2 = iidStackDev;
            while((ret = cmsObj_getNextInSubTreeFlags(MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE_IFC,
                                                      &iidStackDev, &iidStackSub,
                                                      OGF_NO_VALUE_UPDATE, (void **)&ieee1905DevIfcObj)) == CMSRET_SUCCESS)
            {

               rc = sscanf(ieee1905DevIfcObj->interfaceId, I5_MAC_SCANF, I5_MAC_SCANF_PRM(id));
               if ( MAC_ADDR_LEN == rc )
               {
                   i5_dm_interface_type *pinterface = i5DmInterfaceFind(pdevice, id);
                   if (pinterface == NULL)
                   {
                      ret = cmsObj_deleteInstance(MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE_IFC, &iidStackSub);
                      if (ret != CMSRET_SUCCESS)
                      {
                         i5TraceError("deletion of IEEE1905 interface failed.\n");
                      }
                      iidStackSub = iidStackSub2;
                   }
                   else
                   {
                      iidStackSub2 = iidStackSub;
                   }
               }
               cmsObj_free((void **) &ieee1905DevIfcObj);
            }

            INIT_INSTANCE_ID_STACK(&iidStackSub);
            INIT_INSTANCE_ID_STACK(&iidStackSub2);
            while((ret = cmsObj_getNextInSubTreeFlags(MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE_IEEE1905_NEIGHBOR,
                                                      &iidStackDev, &iidStackSub,
                                                      OGF_NO_VALUE_UPDATE, (void **)&ieee1905DevNeighborObj)) == CMSRET_SUCCESS)
            {
               ret = _i5CmsMdmGetInterfaceMacFromPathDescriptor(ieee1905DevNeighborObj->localInterface, id);
               rc  = sscanf(ieee1905DevNeighborObj->neighborDeviceId, I5_MAC_SCANF, I5_MAC_SCANF_PRM(id2));
               if ( (MAC_ADDR_LEN == rc) && (CMSRET_SUCCESS == ret) )
               {
                   i5_dm_1905_neighbor_type *pneighbor = i5Dm1905NeighborFind(pdevice, id, id2);
                   if (pneighbor == NULL)
                   {
                      ret = cmsObj_deleteInstance(MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE_IEEE1905_NEIGHBOR, &iidStackSub);
                      if (ret != CMSRET_SUCCESS)
                      {
                         i5TraceError("deletion of IEEE1905 Neighbor failed.\n");
                      }
                      iidStackSub = iidStackSub2;
                   }
                   else
                   {
                      iidStackSub2 = iidStackSub;
                   }
               }
               cmsObj_free((void **) &ieee1905DevNeighborObj);
            }

            INIT_INSTANCE_ID_STACK(&iidStackSub);
            INIT_INSTANCE_ID_STACK(&iidStackSub2);
            while((ret = cmsObj_getNextInSubTreeFlags(MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE_NON_IEEE1905_NEIGHBOR,
                                                      &iidStackDev, &iidStackSub,
                                                      OGF_NO_VALUE_UPDATE, (void **)&ieee1905DevLegacyNeighborObj)) == CMSRET_SUCCESS)
            {
               ret = _i5CmsMdmGetInterfaceMacFromPathDescriptor(ieee1905DevLegacyNeighborObj->localInterface, id);
               rc  = sscanf(ieee1905DevLegacyNeighborObj->neighborInterfaceId, I5_MAC_SCANF, I5_MAC_SCANF_PRM(id2));
               if ( (MAC_ADDR_LEN == rc) && (CMSRET_SUCCESS == ret) )
               {
                   i5_dm_legacy_neighbor_type *pneighbor = i5DmLegacyNeighborFind(pdevice, id, id2);
                   if (pneighbor == NULL)
                   {
                      ret = cmsObj_deleteInstance(MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE_NON_IEEE1905_NEIGHBOR, &iidStackSub);
                      if (ret != CMSRET_SUCCESS)
                      {
                         i5TraceError("deletion of NON IEEE1905 Neighbor failed.\n");
                      }
                      iidStackSub = iidStackSub2;
                   }
                   else
                   {
                      iidStackSub2 = iidStackSub;
                   }
               }
               cmsObj_free((void **) &ieee1905DevLegacyNeighborObj);
            }

            INIT_INSTANCE_ID_STACK(&iidStackSub);
            INIT_INSTANCE_ID_STACK(&iidStackSub2);
            brCount = 0;
            while((ret = cmsObj_getNextInSubTreeFlags(MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE_BRIDGING_TUPLE,
                                                      &iidStackDev, &iidStackSub,
                                                      OGF_NO_VALUE_UPDATE, (void **)&ieee1905DevBridgeObj)) == CMSRET_SUCCESS)
            {

               brCount++;
               if ( brCount > pdevice->BridgingTuplesNumberOfEntries )
               {
                  ret = cmsObj_deleteInstance(MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE_BRIDGING_TUPLE, &iidStackSub);
                  if (ret != CMSRET_SUCCESS)
                  {
                     i5TraceError("deletion of NON IEEE1905 Neighbor failed.\n");
                  }
                  iidStackSub = iidStackSub2;
               }
               else
               {
                  iidStackSub2 = iidStackSub;
               }
               cmsObj_free((void **) &ieee1905DevBridgeObj);
            }
         }
      }
      cmsObj_free((void**)&ieee1905DevObj);
   }

   return;
}

static CmsRet _i5CmsMdmSaveConfig(t_I5_API_CONFIG_BASE *pCfg)
{
   CmsRet ret;
   Dev2Ieee1905AlObject *ieee1905AlObj = NULL;
   InstanceIdStack       iidStack = EMPTY_INSTANCE_ID_STACK;

   i5Trace("dfname: %s, enabled:%d  registrar:%d bandEn %d, %d\n",
        pCfg->deviceFriendlyName, pCfg->isEnabled, pCfg->isRegistrar,
        pCfg->apFreqBand24En, pCfg->apFreqBand5En);

   ret = cmsObj_get(MDMOID_DEV2_IEEE1905_AL, &iidStack, OGF_NO_VALUE_UPDATE, (void **)&ieee1905AlObj);
   if(ret != CMSRET_SUCCESS)
   {
      i5TraceError("Get MDMOID_DEV2_IEEE1905_AL object fails, ret=%d", ret);
      cmsLck_releaseLock();
      return -1;
   }

   CMSMEM_REPLACE_STRING(ieee1905AlObj->status, pCfg->isEnabled ? MDMVS_ENABLED : MDMVS_DISABLED);
   CMSMEM_REPLACE_STRING(ieee1905AlObj->deviceFriendlyName, pCfg->deviceFriendlyName);
   ieee1905AlObj->enable = pCfg->isEnabled;
   ieee1905AlObj->isRegistrar = pCfg->isRegistrar;
   ieee1905AlObj->APFreqBand24Enable = pCfg->apFreqBand24En;
   ieee1905AlObj->APFreqBand5Enable = pCfg->apFreqBand5En;
   if ( pCfg->isRegistrar )
   {
      char buf[BUFLEN_32];
      int count = 0;
      if ( pCfg->apFreqBand24En )
      {
         count = snprintf(buf, BUFLEN_32, "802.11 2.4 GHz, " );
         if ( count < 0 )
         {
            i5TraceError("unable to print to buffer\n");
            count = 0;
         }
      }
      if ( pCfg->apFreqBand5En )
      {
         count = snprintf(&buf[count-1], (BUFLEN_32 - count), "802.11 5 GHz");
         if ( count < 0 )
         {
            i5TraceError("unable to print to buffer\n");
            count = 0;
         }
      }
      if ( count > 0 )
      {
         CMSMEM_REPLACE_STRING(ieee1905AlObj->registrarFreqBand, buf);
      }
      else
      {
         CMSMEM_REPLACE_STRING(ieee1905AlObj->registrarFreqBand, "");
      }
   }
   else
   {
      CMSMEM_REPLACE_STRING(ieee1905AlObj->registrarFreqBand, "");
   }

   ret = cmsObj_setFlags(ieee1905AlObj, &iidStack, OSF_NO_RCL_CALLBACK);
   if(ret != CMSRET_SUCCESS)
   {
      i5TraceError("could not set MDMOID_DEV2_IEEE1905_AL - ret = %d\n", ret);
   }

   cmsObj_free((void **)&ieee1905AlObj);

   return ret;
}

static CmsRet _i5CmsMdmSaveNetTopConfig(t_I5_API_CONFIG_SET_NETWORK_TOPOLOGY *pCfg)
{
   CmsRet ret;
   Dev2Ieee1905AlNetworkTopologyObject *ieee1905AlNTObj = NULL;
   InstanceIdStack       iidStack = EMPTY_INSTANCE_ID_STACK;

   i5Trace("TopEn %d\n", pCfg->isEnabled);

   ret = cmsObj_get(MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY, &iidStack, OGF_NO_VALUE_UPDATE, (void **)&ieee1905AlNTObj);
   if(ret != CMSRET_SUCCESS)
   {
      i5TraceError("Get MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY object fails, ret=%d", ret);
      cmsLck_releaseLock();
      return -1;
   }
   ieee1905AlNTObj->enable = pCfg->isEnabled;

   ret = cmsObj_setFlags(ieee1905AlNTObj, &iidStack, OSF_NO_RCL_CALLBACK);
   if(ret != CMSRET_SUCCESS)
   {
      i5TraceError("could not set MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY - ret = %d\n", ret);
   }

   cmsObj_free((void **)&ieee1905AlNTObj);

   return ret;
}

static void _i5CmsMdmRetryHandler( void *arg )
{
   int ret;

   i5TimerFree(i5CmsMdmLockTimer);
   i5CmsMdmLockTimer = NULL;

   if ( i5CmsMdmTimerOp & I5_CMS_MDM_RETRY_SAVE_BASE )
   {
      i5TraceInfo("Save base configuration\n");
      ret = _i5CmsMdmSaveConfig(&i5CmsMdmBaseCfg);
      if ( ret != 0 )
      {
         return;
      }
      i5CmsMdmTimerOp &= ~I5_CMS_MDM_RETRY_SAVE_BASE;
   }

   if ( i5CmsMdmTimerOp & I5_CMS_MDM_RETRY_SAVE_NETWORK_TOPOLOGY)
   {
      i5TraceInfo("Save network topology configuration\n");
      ret = _i5CmsMdmSaveNetTopConfig(&i5CmsMdmNetTopCfg);
      if ( ret != 0 )
      {
         return;
      }
      i5CmsMdmTimerOp &= ~I5_CMS_MDM_RETRY_SAVE_NETWORK_TOPOLOGY;
   }

   if ( i5CmsMdmTimerOp & I5_CMS_MDM_RETRY_NETWORK_TOPOLOGY)
   {
      i5TraceInfo("Update AL and Network Topology objects\n");
      _i5CmsMdmAlAddObjects();
      _i5CmsMdmAlRemoveDeletedObjects();

      if (1 == i5_config.networkTopEnabled)
      {
         _i5CmsMdmNetworkTopologyAddObjects();
         _i5CmsMdmNetworkTopologyRemoveDeletedObjects();
      }

      i5CmsMdmTimerOp &= ~I5_CMS_MDM_RETRY_NETWORK_TOPOLOGY;
   }
}

static CmsRet _i5CmsMdmAcquireMDMLock( int retryTime, int op )
{
   int timeout;
   CmsRet ret;

   if ( i5_config.running )
   {
      timeout = I5_CMS_MDM_LOCK_TIMEOUT;

   }
   else
   {
      timeout = I5_CMS_MDM_LOCK_TIMEOUT_LONG;
   }

   ret = cmsLck_acquireLockWithTimeout(timeout);
   if (ret != CMSRET_SUCCESS)
   {
      /* timeout or other error */
      i5CmsMdmTimerOp |= op;
      if ( NULL == i5CmsMdmLockTimer )
      {
         i5CmsMdmLockTimer  = i5TimerNew(retryTime, _i5CmsMdmRetryHandler, NULL);
      }
   }
   return ret;
}

static int _i5CmsMdmEntry( int op )
{
   CmsRet ret;

   ret = _i5CmsMdmAcquireMDMLock(I5_CMS_MDM_LOCK_RETRY, op);
   if (ret != CMSRET_SUCCESS)
   {
      /* indicate that the lock could not be held */
      return -1;
   }

   if ( i5CmsMdmLockTimer != NULL )
   {
      i5TimerFree(i5CmsMdmLockTimer);
      i5CmsMdmLockTimer = NULL;

      _i5CmsMdmRetryHandler(NULL);

      cmsLck_releaseLock();

      /* indicate that there was a rety timer and that
         all operations have been taken care of */
      return 1;
   }

   /* indicate that the lock was acquired */
   return 0;
}

/*--------------------------*
 |   Interface Functions    |
 *--------------------------*/

int i5CmsMdmLocalInterfaceUpdate(const i5_dm_interface_type * const localInterface)
{
   CmsRet ret = CMSRET_SUCCESS;

   if(localInterface == NULL)
   {
      i5TraceError("localInterface is NULL\n");
      return -1;
   }

   if ( _i5CmsMdmEntry(I5_CMS_MDM_RETRY_NETWORK_TOPOLOGY) != 0 )
   {
      return 0;
   }

   ret = _i5CmsMdmLocalInterfaceUpdate(localInterface);

   cmsLck_releaseLock();
   return ( ret == CMSRET_SUCCESS ) ? 0 : -1;
}

int i5CmsMdmLocalNeighborRemove(const i5_dm_1905_neighbor_type * const localNeighbor)
{
   CmsRet ret = CMSRET_SUCCESS;
   InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
   InstanceIdStack iidStack2 = EMPTY_INSTANCE_ID_STACK;
   Dev2Ieee1905AlIfcLinkObject *ieee1905IfcLinkObj = NULL;
   char intfMacAddrStr[I5_MAC_STR_BUF_LEN] = {0};
   char ieee1905MacAddrStr[I5_MAC_STR_BUF_LEN] = {0};
   char localIntfMacAddrStr[I5_MAC_STR_BUF_LEN] = {0};

   if(localNeighbor == NULL)
   {
      i5TraceError("localNeighbor is NULL\n");
      return -1;
   }

   if ( _i5CmsMdmEntry(I5_CMS_MDM_RETRY_NETWORK_TOPOLOGY) != 0 )
   {
      return 0;
   }

   i5TraceInfo("\n");

   ret = _i5CmsMdmGetIidsForLocalNeighbor(localNeighbor, &iidStack, &iidStack2,
                                          intfMacAddrStr, ieee1905MacAddrStr, localIntfMacAddrStr,
                                          &ieee1905IfcLinkObj);
   if(ret == CMSRET_SUCCESS)
   {
      ret = cmsObj_deleteInstance(MDMOID_DEV2_IEEE1905_AL_IFC_LINK, &iidStack2);
      if (ret != CMSRET_SUCCESS)
      {
         i5TraceError("IEEE1905.AL.Interface.{i}.Link{i}. failed to delete, ret=%d\n", ret);
      }
   }
   else
   {
      i5TraceError("Can't delete link since it doesn't exist\n");
      ret = CMSRET_SUCCESS;
   }

   if(ieee1905IfcLinkObj)
   {
      cmsObj_free((void **) &ieee1905IfcLinkObj);
   }
   cmsLck_releaseLock();

   return (ret == CMSRET_SUCCESS) ? 0 : -1;
}

int i5CmsMdmLocalNeighborUpdate(const i5_dm_1905_neighbor_type * const localNeighbor)
{
   CmsRet ret = CMSRET_SUCCESS;

   if(localNeighbor == NULL)
   {
      i5TraceError("localNeighbor is NULL\n");
      return -1;
   }

   if ( _i5CmsMdmEntry(I5_CMS_MDM_RETRY_NETWORK_TOPOLOGY) != 0 )
   {
      return 0;
   }

   ret = _i5CmsMdmLocalNeighborUpdate(localNeighbor);

   cmsLck_releaseLock();

   return (ret == CMSRET_SUCCESS) ? 0 : -1;
}

int i5CmsMdmRemoveNetworkTopologyDevIfc(const i5_dm_device_type *pDev,
                                        const i5_dm_interface_type *pInterface)
{
   CmsRet ret = CMSRET_SUCCESS;
   InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
   Dev2Ieee1905AlNetworkTopologyIeee1905DeviceObject *ieee1905DevObj = NULL;

   if (0 == i5_config.networkTopEnabled)
   {
     return 0;
   }

   if(pDev == NULL)
   {
      i5TraceError("device is NULL\n");
      return -1;
   }

   if ( _i5CmsMdmEntry(I5_CMS_MDM_RETRY_NETWORK_TOPOLOGY) != 0 )
   {
      return 0;
   }

   i5TraceInfo("\n");

   ret = _i5CmsMdmGetDeviceIidFromMac(pDev->DeviceId, &iidStack, &ieee1905DevObj);

   /* new one IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}. */
   if(ret != CMSRET_SUCCESS)
   {
      // There was no object to delete
      i5TraceError("IEEE1905 device does not exist.\n");
   }
   else
   {
     ret = _i5CmsMdmRemoveNetworkTopologyDevIfcOfParent(&iidStack, pInterface);
     if (ret != CMSRET_SUCCESS)
     {
        i5TraceError("deletion of IEEE1905 neighbor failed.\n");
     }
     cmsObj_free((void **) &ieee1905DevObj);
   }

   cmsLck_releaseLock();

   return (ret == CMSRET_SUCCESS) ? 0 : -1;
}

int i5CmsMdmUpdateNetworkTopologyDevIfc(const i5_dm_device_type *pDev,
                                        const i5_dm_interface_type *pInterface)
{
   CmsRet ret = CMSRET_SUCCESS;
   InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
   Dev2Ieee1905AlNetworkTopologyIeee1905DeviceObject *ieee1905DevObj = NULL;

   if (0 == i5_config.networkTopEnabled)
   {
     return 0;
   }

   if((pDev == NULL) || (pInterface == NULL))
   {
      i5TraceError("ieee1905 device is NULL\n");
      return -1;
   }

   if ( _i5CmsMdmEntry(I5_CMS_MDM_RETRY_NETWORK_TOPOLOGY) != 0 )
   {
      return 0;
   }

   i5TraceInfo("Adding" I5_MAC_DELIM_FMT " to dev " I5_MAC_DELIM_FMT "\n", I5_MAC_PRM(pDev->DeviceId), I5_MAC_PRM(pInterface->InterfaceId));

   ret = _i5CmsMdmGetDeviceIidFromMac(pDev->DeviceId, &iidStack, &ieee1905DevObj);

   /* new one IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}. */
   if (ret != CMSRET_SUCCESS)
   {
      // There was no device object
      i5TraceError("IEEE1905 device does not exist.\n");
   }
   else
   {
      ret = _i5CmsMdmUpdateNetworkTopologyDevIfcOfParent (&iidStack, pInterface);
      if(ret == CMSRET_SUCCESS)
      {
         i5TraceError("update of IEEE1905 Interface failed.\n");
      }
      cmsObj_free((void **) &ieee1905DevObj);
   }
   cmsLck_releaseLock();

   return (ret == CMSRET_SUCCESS) ? 0 : -1;
}

int i5CmsMdmRemoveNetworkTopologyDevNeighbor(const i5_dm_device_type *pDev,
                                             const i5_dm_1905_neighbor_type *pNeighbor)
{
   CmsRet ret = CMSRET_SUCCESS;
   InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
   Dev2Ieee1905AlNetworkTopologyIeee1905DeviceObject *ieee1905DevObj = NULL;

   if (0 == i5_config.networkTopEnabled)
   {
      return 0;
   }

   if(pDev == NULL)
   {
      i5TraceError("device is NULL\n");
      return -1;
   }

   if ( _i5CmsMdmEntry(I5_CMS_MDM_RETRY_NETWORK_TOPOLOGY) != 0 )
   {
      return 0;
   }

   i5TraceInfo("\n");

   ret = _i5CmsMdmGetDeviceIidFromMac(pDev->DeviceId, &iidStack, &ieee1905DevObj);

   /* new one IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}. */
   if(ret != CMSRET_SUCCESS)
   {
      // There was no object to delete
      i5TraceError("IEEE1905 device does not exist.\n");
   }
   else
   {
     ret = _i5CmsMdmRemoveNetworkTopologyDevNeighborOfParent(&iidStack, pNeighbor);
     if (ret != CMSRET_SUCCESS)
     {
        i5TraceError("deletion of IEEE1905 neighbor failed.\n");
     }
     cmsObj_free((void **) &ieee1905DevObj);
   }
   cmsLck_releaseLock();

   return (ret == CMSRET_SUCCESS) ? 0 : -1;
}

int i5CmsMdmUpdateNetworkTopologyDevNeighbor(const i5_dm_device_type *pDev,
                                             const i5_dm_1905_neighbor_type *pNeighbor)
{
   CmsRet ret = CMSRET_SUCCESS;
   InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
   Dev2Ieee1905AlNetworkTopologyIeee1905DeviceObject *ieee1905DevObj = NULL;

   if (0 == i5_config.networkTopEnabled)
   {
      return 0;
   }

   if((pDev == NULL) || (pNeighbor == NULL))
   {
      i5TraceError("ieee1905 device is NULL\n");
      return -1;
   }

   if ( _i5CmsMdmEntry(I5_CMS_MDM_RETRY_NETWORK_TOPOLOGY) != 0 )
   {
      return 0;
   }

   i5TraceInfo("Adding" I5_MAC_DELIM_FMT " to dev " I5_MAC_DELIM_FMT "\n", I5_MAC_PRM(pNeighbor->Ieee1905Id), I5_MAC_PRM(pDev->DeviceId));

   ret = _i5CmsMdmGetDeviceIidFromMac(pDev->DeviceId, &iidStack, &ieee1905DevObj);

   /* new one IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}. */
   if (ret != CMSRET_SUCCESS)
   {
      // There was no device object
      i5TraceError("IEEE1905 device does not exist.\n");
   }
   else
   {
      ret = _i5CmsMdmUpdateNetworkTopologyDevNeighborOfParent (&iidStack, pNeighbor);
      if(ret == CMSRET_SUCCESS)
      {
         i5TraceError("update of IEEE1905 Neighbor failed.\n");
      }
      cmsObj_free((void **) &ieee1905DevObj);
   }
   cmsLck_releaseLock();

   return (ret == CMSRET_SUCCESS) ? 0 : -1;
}

int i5CmsMdmRemoveNetworkTopologyDevLegacyNeighbor(const i5_dm_device_type *pDev,
                                                   const i5_dm_legacy_neighbor_type *pLegacy)
{
   CmsRet ret = CMSRET_SUCCESS;
   InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
   Dev2Ieee1905AlNetworkTopologyIeee1905DeviceObject *ieee1905DevObj = NULL;

   if (0 == i5_config.networkTopEnabled)
   {
      return 0;
   }

   if(pDev == NULL)
   {
      i5TraceError("ieee1905 Device is NULL\n");
      return -1;
   }

   if ( _i5CmsMdmEntry(I5_CMS_MDM_RETRY_NETWORK_TOPOLOGY) != 0 )
   {
      return 0;
   }

   i5TraceInfo("\n");

   ret = _i5CmsMdmGetDeviceIidFromMac(pDev->DeviceId, &iidStack, &ieee1905DevObj);

   /* new one IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}. */
   if(ret != CMSRET_SUCCESS)
   {
      // There was no object to delete
      i5TraceError("IEEE1905 device does not exist.\n");
      cmsLck_releaseLock();
      return 0;
   }

   ret = _i5CmsMdmRemoveNetworkTopologyDevLegacyNeighborOfParent(&iidStack, pLegacy);
   if(ret != CMSRET_SUCCESS)
   {
      i5TraceError("deletion of IEEE1905 Legacy neighbor failed.\n");
   }
   cmsObj_free((void **) &ieee1905DevObj);

   cmsLck_releaseLock();
   return (ret == CMSRET_SUCCESS) ? 0 : -1;
}

int i5CmsMdmUpdateNetworkTopologyDevLegacyNeighbor(const i5_dm_device_type *pDev,
                                                   const i5_dm_legacy_neighbor_type *pLegacy)
{
   CmsRet ret = CMSRET_SUCCESS;
   InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
   Dev2Ieee1905AlNetworkTopologyIeee1905DeviceObject *ieee1905DevObj = NULL;

   if (0 == i5_config.networkTopEnabled)
   {
      return 0;
   }

   if(pDev == NULL)
   {
      i5TraceError("ieee1905 device is NULL\n");
      return -1;
   }

   if ( _i5CmsMdmEntry(I5_CMS_MDM_RETRY_NETWORK_TOPOLOGY) != 0 )
   {
      return 0;
   }

   i5TraceInfo("\n");

   ret = _i5CmsMdmGetDeviceIidFromMac(pDev->DeviceId, &iidStack, &ieee1905DevObj);

   /* new one IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}. */
   if (ret != CMSRET_SUCCESS)
   {
      // There was no object to delete
      i5TraceError("IEEE1905 device does not exist.\n");
      ret = CMSRET_SUCCESS;
   }
   else
   {
      ret = _i5CmsMdmUpdateNetworkTopologyDevLegacyNeighborOfParent (&iidStack, pLegacy);
      cmsObj_free((void**) &ieee1905DevObj);
   }

   cmsLck_releaseLock();
   return (ret == CMSRET_SUCCESS) ? 0 : -1;
}

/* Call when a 1905 Device is deleted
 */
int i5CmsMdmRemoveNetworkTopologyDev(unsigned char* ieee1905Id)
{
   CmsRet ret = CMSRET_SUCCESS;
   InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
   Dev2Ieee1905AlNetworkTopologyIeee1905DeviceObject *ieee1905DevObj = NULL;

   if (0 == i5_config.networkTopEnabled)
   {
      return 0;
   }

   if(ieee1905Id == NULL)
   {
      i5TraceError("ieee1905Id is NULL\n");
      return -1;
   }

   if(i5DmDeviceIsSelf(ieee1905Id))
   {
      i5TraceError("deletion of local IEEE1905 device is forbidden.\n");
      return -1;
   }

   if ( _i5CmsMdmEntry(I5_CMS_MDM_RETRY_NETWORK_TOPOLOGY) != 0 )
   {
      return 0;
   }

   i5TraceInfo("Removing:" I5_MAC_DELIM_FMT "\n", I5_MAC_PRM(ieee1905Id));

   ret = _i5CmsMdmGetDeviceIidFromMac(ieee1905Id, &iidStack, &ieee1905DevObj);

   /* new one IEEE1905.AL.NetworkTopology.IEEE1905Device.{i}. */
   if(ret != CMSRET_SUCCESS)
   {
      // There was no object to delete
      i5TraceError("IEEE1905 device does not exist.\n");
      cmsLck_releaseLock();
      return 0;
   }

   ret = cmsObj_deleteInstance(MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY_IEEE1905_DEVICE, &iidStack);
   if(ret != CMSRET_SUCCESS)
   {
      i5TraceError("deletion of IEEE1905 device failed.\n");
   }
   cmsObj_free((void**)&ieee1905DevObj);

   cmsLck_releaseLock();
   return (ret == CMSRET_SUCCESS) ? 0 : -1;
}

/* Call when a 1905 Device is added or changed
 *
 * Will create the device in CMS if necessary
 */
int i5CmsMdmUpdateNetworkTopologyDev(const i5_dm_device_type *pDev)
{
   CmsRet ret = CMSRET_SUCCESS;

   if (0 == i5_config.networkTopEnabled)
   {
     i5TraceInfo("Ignoring while topology is disabled\n");
     return 0;
   }

   if(pDev == NULL)
   {
      i5TraceError("pDev is NULL\n");
      return -1;
   }

   if ( _i5CmsMdmEntry(I5_CMS_MDM_RETRY_NETWORK_TOPOLOGY) != 0 )
   {
      return 0;
   }

   i5TraceInfo("\n");

   ret = _i5CmsMdmUpdateNetworkTopologyDev(pDev);

   cmsLck_releaseLock();

   return (ret == CMSRET_SUCCESS) ? 0 : -1;
}

/* Note that index coming in must be zero-based, and based on the order in which
 * the bridges were created
 */
void i5CmsMdmDeleteBridgingTuple(const i5_dm_device_type *pDev, i5_dm_bridging_tuple_info_type *pBrTuple)
{
   CmsRet ret = CMSRET_SUCCESS;
   InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
   Dev2Ieee1905AlNetworkTopologyIeee1905DeviceObject *ieee1905DevObj = NULL;
   int index = 0;

   if (0 == i5_config.networkTopEnabled)
   {
     return;
   }

   if((pDev == NULL) || (pBrTuple == NULL))
   {
      i5TraceError(" NULL device or tuple\n");
      return;
   }

   if ( _i5CmsMdmEntry(I5_CMS_MDM_RETRY_NETWORK_TOPOLOGY) != 0 )
   {
      return;
   }

   i5TraceInfo("\n");

   ret = _i5CmsMdmGetDeviceIidFromMac(pDev->DeviceId, &iidStack, &ieee1905DevObj);
   if(ret != CMSRET_SUCCESS)
   {
      i5TraceInfo("No such Device\n");
   }
   else {
      i5_dm_bridging_tuple_info_type *pBridgeTuple = pDev->bridging_tuple_list.ll.next;
      while ( pBridgeTuple != NULL )
      {
         if ( pBrTuple == pBridgeTuple )
         {
            break;
         }
         index++;
         pBridgeTuple = pBridgeTuple->ll.next;
      }

      if ( pBridgeTuple )
      {
         /* 1905 and CMS lists are in different order */
         index = pDev->BridgingTuplesNumberOfEntries - index - 1;
         _i5CmsMdmRemoveBridgingTupleWithParent(&iidStack, index);
      }
      cmsObj_free((void**) &ieee1905DevObj);
   }
   cmsLck_releaseLock();
}

/* Note that index coming in must be zero-based, and based on the order in which
 * the bridges were created
 */
void i5CmsMdmUpdateBridgingTuple(const i5_dm_device_type *pDev, i5_dm_bridging_tuple_info_type *pBrTuple)
{
   CmsRet ret = CMSRET_SUCCESS;
   InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
   Dev2Ieee1905AlNetworkTopologyIeee1905DeviceObject *ieee1905DevObj = NULL;
   int index = 0;

   if (0 == i5_config.networkTopEnabled)
   {
     return;
   }

   if((pDev == NULL) || (pBrTuple == NULL))
   {
      i5TraceError(" NULL device or tuple\n");
      return;
   }

   if ( _i5CmsMdmEntry(I5_CMS_MDM_RETRY_NETWORK_TOPOLOGY) != 0 )
   {
      return;
   }

   i5TraceInfo("\n");

   ret = _i5CmsMdmGetDeviceIidFromMac(pDev->DeviceId, &iidStack, &ieee1905DevObj);
   if(ret != CMSRET_SUCCESS)
   {
      i5TraceInfo("No such Device\n");
   }
   else {
      i5_dm_bridging_tuple_info_type *pBridgeTuple = pDev->bridging_tuple_list.ll.next;
      while ( pBridgeTuple != NULL )
      {
         if ( pBrTuple == pBridgeTuple )
         {
            break;
         }
         index++;
         pBridgeTuple = pBridgeTuple->ll.next;
      }

      if ( pBridgeTuple )
      {
         /* 1905 and CMS lists are in different order */
         index = pDev->BridgingTuplesNumberOfEntries - index - 1;
         _i5CmsMdmUpdateBridgingTupleWithParent(&iidStack, pBrTuple, index);
      }
      cmsObj_free((void**) &ieee1905DevObj);
   }
   cmsLck_releaseLock();
}

/* for disabled->enabled, flush and reprogram topology
 * for enabled->disabled, do nothing
 */
void i5CmsMdmProcessNetworkTopologyConfigChange(void)
{
   if (i5_config.networkTopEnabled == 1)
   {

      CmsRet ret = _i5CmsMdmAcquireMDMLock(I5_CMS_MDM_LOCK_RETRY, I5_CMS_MDM_RETRY_NETWORK_TOPOLOGY);
      if (ret != CMSRET_SUCCESS)
      {
         return;
      }
      _i5CmsMdmFlushNetworkTopology();
      _i5CmsMdmNetworkTopologyAddObjects();
      cmsLck_releaseLock();
   }
}

int i5CmsMdmLoadConfig(t_I5_API_CONFIG_BASE *pCfg)
{
   CmsRet                ret;
   Dev2Ieee1905AlObject *ieee1905AlObj = NULL;
   InstanceIdStack       iidStack = EMPTY_INSTANCE_ID_STACK;

   ret = cmsLck_acquireLockWithTimeout(I5_CMS_MDM_LOCK_TIMEOUT_LONG);
   if (ret != CMSRET_SUCCESS)
   {
      i5TraceError("Could not get CMS lock, ret=%d", ret);
      return -1;
   }

   ret = cmsObj_get(MDMOID_DEV2_IEEE1905_AL, &iidStack, OGF_NO_VALUE_UPDATE, (void **)&ieee1905AlObj);
   if(ret != CMSRET_SUCCESS)
   {
      i5TraceError("Get MDMOID_DEV2_IEEE1905_AL object fails, ret=%d", ret);
      cmsLck_releaseLock();
      return -1;
   }

   pCfg->isEnabled = ieee1905AlObj->enable;
   strncpy(pCfg->deviceFriendlyName, ieee1905AlObj->deviceFriendlyName, I5_DEVICE_FRIENDLY_NAME_LEN-1);
   pCfg->deviceFriendlyName[I5_DEVICE_FRIENDLY_NAME_LEN-1] = '\0';
   pCfg->isRegistrar = ieee1905AlObj->isRegistrar;
   pCfg->apFreqBand24En = ieee1905AlObj->APFreqBand24Enable;
   pCfg->apFreqBand5En = ieee1905AlObj->APFreqBand5Enable;

   cmsObj_free((void **)&ieee1905AlObj);
   cmsLck_releaseLock();

   i5Trace("dfname:%s,  enabled:%d  registrar:%d bandEn %d, %d\n",
        pCfg->deviceFriendlyName, pCfg->isEnabled, pCfg->isRegistrar,
        pCfg->apFreqBand24En, pCfg->apFreqBand5En);

   return 0;
}

int i5CmsMdmLoadNetTopConfig(t_I5_API_CONFIG_SET_NETWORK_TOPOLOGY *pCfg)
{
   CmsRet                ret;
   Dev2Ieee1905AlNetworkTopologyObject *ieee1905AlNTObj = NULL;
   InstanceIdStack       iidStack = EMPTY_INSTANCE_ID_STACK;

   ret = cmsLck_acquireLockWithTimeout(I5_CMS_MDM_LOCK_TIMEOUT_LONG);
   if (ret != CMSRET_SUCCESS)
   {
      i5TraceError("Could not get CMS lock, ret=%d", ret);
      return -1;
   }

   ret = cmsObj_get(MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY, &iidStack, OGF_NO_VALUE_UPDATE, (void **)&ieee1905AlNTObj);
   if(ret != CMSRET_SUCCESS)
   {
      i5TraceError("Get MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY object fails, ret=%d", ret);
      cmsLck_releaseLock();
      return -1;
   }
   pCfg->isEnabled = ieee1905AlNTObj->enable;

   cmsObj_free((void **)&ieee1905AlNTObj);
   cmsLck_releaseLock();

   i5Trace("TopEn %d\n", pCfg->isEnabled);

   return 0;
}

int i5CmsMdmSaveConfig(t_I5_API_CONFIG_BASE *pCfg)
{
   CmsRet ret;
   int    rc;

   rc = _i5CmsMdmEntry(I5_CMS_MDM_RETRY_SAVE_BASE);
   if ( rc != 0 )
   {
      if ( ret < 0 )
      {
         memcpy(&i5CmsMdmBaseCfg, pCfg, sizeof(t_I5_API_CONFIG_BASE));
      }
      return 0;
   }

   ret = _i5CmsMdmSaveConfig(pCfg);

   cmsLck_releaseLock();

   return ((ret != CMSRET_SUCCESS) ? -1 : 0);
}

int i5CmsMdmSaveNetTopConfig(t_I5_API_CONFIG_SET_NETWORK_TOPOLOGY *pCfg)
{
   CmsRet ret;
   int rc;

   rc = _i5CmsMdmEntry(I5_CMS_MDM_RETRY_SAVE_NETWORK_TOPOLOGY);
   if ( rc != 0 )
   {
      if ( ret < 0 )
      {
         memcpy(&i5CmsMdmBaseCfg, pCfg, sizeof(t_I5_API_CONFIG_BASE));
      }
      return 0;
   }

   ret = _i5CmsMdmSaveNetTopConfig(pCfg);

   cmsLck_releaseLock();

   return ((ret != CMSRET_SUCCESS) ? -1 : 0);
}

int i5CmsMdmInit( )
{
   Dev2Ieee1905Object   *ieee1905Obj = NULL;
   Dev2Ieee1905AlObject *ieee1905AlObj = NULL;
   Dev2Ieee1905AlNetworkTopologyObject *ieee1905AlNTObj = NULL;
   Dev2Ieee1905AlSecurityObject *ieee1905AlSecObj = NULL;
   CmsRet                ret;
   InstanceIdStack       iidStack = EMPTY_INSTANCE_ID_STACK;
   char                  macAddrStr[I5_MAC_STR_BUF_LEN]; /* xx:xx:xx:xx:xx:xx */
   char                  deviceFriendlyName[BUFLEN_32] = {0};
   int                   initObjects = 0;

   i5Trace("\n");

   ret = cmsLck_acquireLockWithTimeout(I5_CMS_MDM_LOCK_TIMEOUT_LONG);
   if (ret != CMSRET_SUCCESS)
   {
      i5TraceError("Could not acquire CMS lock\n");
      return -1;
   }

   do
   {
      ret = cmsObj_get(MDMOID_DEV2_IEEE1905, &iidStack, OGF_NO_VALUE_UPDATE, (void **)&ieee1905Obj);
      if(ret != CMSRET_SUCCESS)
      {
         i5TraceError("Get MDMOID_DEV2_IEEE1905 object fails, ret=%d", ret);
         break;
      }

      ieee1905Obj->version = 0; /* 1905 version 0 */
      ret = cmsObj_setFlags(ieee1905Obj, &iidStack, OSF_NO_RCL_CALLBACK);
      if(ret != CMSRET_SUCCESS)
      {
         i5TraceError("could not set MDMOID_DEV2_IEEE1905\n");
         break;
      }

      ret = cmsObj_get(MDMOID_DEV2_IEEE1905_AL, &iidStack, OGF_NO_VALUE_UPDATE, (void **)&ieee1905AlObj);
      if(ret != CMSRET_SUCCESS)
      {
         i5TraceError("Get MDMOID_DEV2_IEEE1905_AL object fails, ret=%d", ret);
         break;
      }

      /* if friendlyName is NULL or empty then flag that default settings should be
         filled in */
      if ( (NULL == ieee1905AlObj->deviceFriendlyName) ||
           (0 == strlen(ieee1905AlObj->deviceFriendlyName)) )
      {
         i5Trace("Default configuration required - %s\n", ieee1905AlObj->deviceFriendlyName);

         initObjects = 1;

         /* fill in custom fields */
         ieee1905AlObj->enable = 1;
#if defined(SUPPORT_IEEE1905_REGISTRAR)
         ieee1905AlObj->isRegistrar = 1;
#else
         ieee1905AlObj->isRegistrar = 0;
#endif // endif
         ieee1905AlObj->APFreqBand24Enable = 1;
         ieee1905AlObj->APFreqBand5Enable = 1;

         if ( i5GlueAssignFriendlyName(i5_config.i5_mac_address, deviceFriendlyName, sizeof(deviceFriendlyName)) < 0 )
         {
            i5TraceError("Unable to get Friendly name for device\n");
            break;
         }
         ieee1905AlObj->lastChange = 0;
         /* leave lowerLayers as NULL */
         CMSMEM_REPLACE_STRING(ieee1905AlObj->deviceFriendlyName, deviceFriendlyName);
      }

      /* these items should always be initialized */
      CMSMEM_REPLACE_STRING(ieee1905AlObj->registrarFreqBand, "");
      CMSMEM_REPLACE_STRING(ieee1905AlObj->status, MDMVS_ENABLED);
      snprintf(macAddrStr, sizeof(macAddrStr), "%02X:%02X:%02X:%02X:%02X:%02X",
               i5_config.i5_mac_address[0], i5_config.i5_mac_address[1], i5_config.i5_mac_address[2],
               i5_config.i5_mac_address[3], i5_config.i5_mac_address[4], i5_config.i5_mac_address[5]);
      CMSMEM_REPLACE_STRING(ieee1905AlObj->IEEE1905Id, macAddrStr);

      ret = cmsObj_setFlags(ieee1905AlObj, &iidStack, OSF_NO_RCL_CALLBACK);
      if(ret != CMSRET_SUCCESS)
      {
         i5TraceError("could not set MDMOID_DEV2_IEEE1905_AL - ret = %d\n", ret);
         break;
      }

      if ( 1 == initObjects )
      {
         ret = cmsObj_get(MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY, &iidStack, OGF_NO_VALUE_UPDATE, (void **)&ieee1905AlNTObj);
         if(ret != CMSRET_SUCCESS)
         {
            i5TraceError("Get MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY object fails, ret=%d", ret);
            break;
         }
         ieee1905AlNTObj->enable = 0;
         CMSMEM_REPLACE_STRING( ieee1905AlNTObj->status, MDMVS_INCOMPLETE);
         ieee1905AlNTObj->maxChangeLogEntries = 1;
         /* leave ieee1905AlNTObj->lastChange as NULL */
         ieee1905AlNTObj->changeLogNumberOfEntries = 0;

         ret = cmsObj_setFlags(ieee1905AlNTObj, &iidStack, OSF_NO_RCL_CALLBACK);
         if(ret != CMSRET_SUCCESS)
         {
            i5TraceError("could not set MDMOID_DEV2_IEEE1905_AL_NETWORK_TOPOLOGY - ret = %d\n", ret);
            break;
         }

         ret = cmsObj_get(MDMOID_DEV2_IEEE1905_AL_SECURITY, &iidStack, OGF_NO_VALUE_UPDATE, (void **)&ieee1905AlSecObj);
         if(ret != CMSRET_SUCCESS)
         {
            i5TraceError("Get MDMOID_DEV2_IEEE1905_AL_SECURITY object fails, ret=%d", ret);
            break;
         }
         CMSMEM_REPLACE_STRING(ieee1905AlSecObj->setupMethod ,"PBC");
         CMSMEM_REPLACE_STRING(ieee1905AlSecObj->password, "");

         ret = cmsObj_setFlags(ieee1905AlSecObj, &iidStack, OSF_NO_RCL_CALLBACK);
         if(ret != CMSRET_SUCCESS)
         {
            i5TraceError("could not set MDMOID_DEV2_IEEE1905_AL_SECURITY - ret = %d\n", ret);
            break;
         }
      }
   }
   while ( 0 );

   if ( ieee1905Obj )
   {
      cmsObj_free((void **) &ieee1905Obj);
   }

   if ( ieee1905AlObj )
   {
      cmsObj_free((void **) &ieee1905AlObj);
   }

   if ( ieee1905AlNTObj )
   {
      cmsObj_free((void **) &ieee1905AlNTObj);
   }

   if ( ieee1905AlSecObj )
   {
      cmsObj_free((void **) &ieee1905AlSecObj);
   }

   cmsLck_releaseLock();
   return ((ret != CMSRET_SUCCESS) ? -1 : 0);
}
#endif /* DMP_DEVICE2_IEEE1905BASELINE_1 */
