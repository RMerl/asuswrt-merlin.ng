/***********************************************************************
 *
 *  Copyright (c) 2013  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2013:proprietary:standard
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
 * $Change: 116460 $
 ***********************************************************************/

/* Do not use ifdefs in this file -- this file should not be conditional on compile
   time configuration, as we only give one copy of the object (per platform) to the
   customer, and it is expected to work with any configuration.  */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ieee1905_datamodel_priv.h"
#include "ieee1905_utils.h"
#include "ieee1905_trace.h"
#include "ieee1905_socket.h"
#include "ieee1905_interface.h"
#include "ieee1905_wlcfg.h"
#include "ieee1905_glue.h"
#include "ieee1905_brutil.h"
#include "ieee1905_flowmanager.h"

/* Declarations of functions which define what is supported: */

extern int isFbctlSupported();
extern int isFcctlSupported();
extern int isAutoWdsSupported();

/* External functions which may not be linked in.  If they are not linked in, they should
   not be invoked */

int           i5WlCfgGetWdsMacFromName(char const *ifname, char *macString, int maxLen) __attribute__ ((weak));
char         *i5WlcfgGetWlParentInterface(char const * ifname, char * wlParentIf)  __attribute__ ((weak));
unsigned int  i5WlCfgAreMediaTypesCompatible(unsigned short mediaType1, unsigned short mediaType2) __attribute__ ((weak));
unsigned int  i5WlCfgIsApConfigured(char const * ifname) __attribute__ ((weak));

int fbCtlAddIf(unsigned int group, unsigned int ifIndex) __attribute__ ((weak));
int fbCtlDeleteIf(unsigned int groupIdx, unsigned int ifIndex) __attribute__ ((weak));
int fbCtlTokens(unsigned int groupindex, unsigned int ifindex,
                unsigned int tokens, unsigned int max_tokens) __attribute__ ((weak));
int fcCtlFlush(int arg) __attribute__ ((weak));

#define I5_FLOW_MANAGER_MAX_NEIGHBORS 16

#define I5_TRACE_MODULE i5TraceFlow

typedef struct {
   unsigned short bandwidthMbps;
   unsigned char stpCost;
} i5_flowManager_stpPathCost_type;

static const i5_flowManager_stpPathCost_type stpPathCosts[] =
  {{4   , 250},  /* standard value */
   {7   , 140},
   {10  , 100},  /* standard value */
   {13  ,  77},
   {16  ,  62},  /* standard value */
   {32  ,  40},
   {49  ,  30},
   {66  ,  25},
   {83  ,  22},
   {100 ,  19},  /* standard value */
   {150 ,  15},
   {200 ,  12},
   {1000,   4},  /* standard value */
   {2000,   2},  /* standard value */
   {10000,  1}}; /* standard value */

/*----------------------*
 | Hysteresis Algorithm |
 *----------------------*/
#define I5_FLOW_MANAGER_ITERATIONS_FOR_STP_UPDATE 5

/* When BW increases, only add a fraction of the BW gain that is reported ( 1 / (2^n) )*/
#define I5_FLOW_MANAGER_HYSTERESIS_CRAWL_UP_BITSHIFT 1
/* When storing BW values in the struct above, give them extra bits of accuracy,
 * otherwise the hysteresis algorithm will never reach the actual value */
#define I5_FLOW_MANAGER_HYSTERESIS_STORAGE_BW_BITSHIFT 4
#define I5_FLOW_MANAGER_HYSTERESIS_STORAGE_BW_ROUNDOFF (1 << (I5_FLOW_MANAGER_HYSTERESIS_STORAGE_BW_BITSHIFT - 1))

/*----------------*
 | End Hysteresis |
 *----------------*/

typedef struct i5IfIndexList_t{
  i5_ll_listitem     ll;
  unsigned int       ifindex;
} i5FlowInterfaceList;

typedef struct i5NeighConn_t{
  i5_ll_listitem        ll;
  unsigned char         neighborId[ETH_ALEN];
  int                   neighborIndex;
  char                  activated;
  i5FlowInterfaceList   ifindexList;
} i5FlowNeighConn;

static i5FlowNeighConn i5FlowConnectionList = {};
static int i5FlowCurrentNeighborIndex = 0;
static timer_elem_type *i5FlowRegisterWdsRetry_ptmr = NULL;

void i5FlowManagerProcessWirelessUp(void);

static int i5FlowManagerApproximateLogarithm(int bandwidth)
{
   int prevStpBw = 1;
   int prevStpCost = 255;
   int index = 0;

   for ( ; index < sizeof(stpPathCosts)/sizeof(i5_flowManager_stpPathCost_type); index ++) {
      int currStpBw = stpPathCosts[index].bandwidthMbps;
      int currStpCost = stpPathCosts[index].stpCost;

      if (currStpBw == bandwidth) {
         return (int) currStpCost;
      }
      else if (currStpBw < bandwidth) {
         prevStpBw = currStpBw;
         prevStpCost = currStpCost;
      }
      else {
         int numerator = bandwidth - prevStpBw;
         int denom = currStpBw - prevStpBw;
         int costRange = currStpCost - prevStpCost;
         return prevStpCost + (costRange * numerator) / denom;
      }
   }
   return 1;
}

static int i5FlowGetNeighborCount(void)
{
  int count = 0;
  i5FlowNeighConn* neighbor = (i5FlowNeighConn*)i5FlowConnectionList.ll.next;

  while (neighbor != NULL) {
    count ++;
    neighbor = (i5FlowNeighConn*)neighbor->ll.next;
  }
  return count;
}

static i5FlowNeighConn *i5FlowGetNeighbor(unsigned char const *neighborId)
{
  i5Trace("\n");
  if (neighborId == NULL) {
    i5Trace("NULL pointer\n");
    return NULL;
  }
  i5FlowNeighConn* neighbor = (i5FlowNeighConn*)&i5FlowConnectionList.ll.next;

  while (neighbor != NULL) {
    if (memcmp(neighbor->neighborId, neighborId, ETH_ALEN) == 0) {
      break;
    }
    neighbor = (i5FlowNeighConn*)neighbor->ll.next;
  }
  return neighbor;
}

static i5FlowNeighConn* i5FlowAddNeighborId(unsigned char const *neighborId)
{
  i5Trace("\n");
  if (neighborId == NULL) {
    i5Trace("NULL pointer\n");
    return NULL;
  }

  i5FlowNeighConn* newNeighbor = i5FlowGetNeighbor(neighborId);
  if (newNeighbor != NULL) {
    return newNeighbor;
  }

  newNeighbor = malloc(sizeof(i5FlowNeighConn));
  memcpy(newNeighbor->neighborId, neighborId, ETH_ALEN);
  newNeighbor->neighborIndex = i5FlowCurrentNeighborIndex;
  newNeighbor->activated = 0;
  i5FlowCurrentNeighborIndex++;
  newNeighbor->ifindexList.ll.next = NULL;

  i5LlItemAdd(NULL, &i5FlowConnectionList, newNeighbor);

  return newNeighbor;
}

static int i5FlowGetIfindexCount(i5FlowNeighConn *neighbor)
{
  int count = 0;
  i5FlowInterfaceList *interfaceItem = (i5FlowInterfaceList *)neighbor->ifindexList.ll.next;

  while(interfaceItem != NULL) {
    count++;
    interfaceItem = (i5FlowInterfaceList *)interfaceItem->ll.next;
  }
  return count;
}

static i5FlowInterfaceList *i5FlowFindIfIndexForNeighbor(i5FlowNeighConn *neighbor, unsigned int ifindex)
{
  i5FlowInterfaceList *matchingInterface = (i5FlowInterfaceList *)neighbor->ifindexList.ll.next;

  for (; matchingInterface != NULL; matchingInterface = (i5FlowInterfaceList *)matchingInterface->ll.next) {
    if ( matchingInterface->ifindex == ifindex) {
      return matchingInterface;
    }
  }
  return NULL;
}

static void i5FlowAddIfindexToNeighbor(i5FlowNeighConn *neighbor, unsigned int ifindex)
{
  i5FlowInterfaceList *newInterface = NULL;

  if (neighbor == NULL) {
    i5Trace("NULL pointer\n");
    return;
  }
  i5Trace("Neighbor: " I5_MAC_DELIM_FMT " ifindex:%d\n", I5_MAC_PRM(neighbor->neighborId),ifindex);
  if (i5FlowFindIfIndexForNeighbor(neighbor, ifindex) != NULL) {
    i5Trace("ifindex already exists\n");
    return;
  }

  newInterface = malloc(sizeof(i5FlowInterfaceList));
  newInterface->ifindex = ifindex;
  i5LlItemAdd(NULL, &neighbor->ifindexList, newInterface);
}

static void i5FlowDeactivateFlowBond(i5FlowNeighConn *neighbor)
{
  if (isFbctlSupported())  {
    i5FlowInterfaceList *interfaceItem = (i5FlowInterfaceList *)neighbor->ifindexList.ll.next;
    i5Trace("Deactivating Neighbor: " I5_MAC_DELIM_FMT "\n", I5_MAC_PRM(neighbor->neighborId) );
    for (; interfaceItem != NULL; interfaceItem = (i5FlowInterfaceList *)interfaceItem->ll.next) {
      i5Trace("Telling Flowbond to delete %d %d\n",neighbor->neighborIndex, interfaceItem->ifindex);
      fbCtlDeleteIf (neighbor->neighborIndex, interfaceItem->ifindex);
    }
  }
  neighbor->activated = 0;
}

static void i5FlowRemoveIfindexFromNeighbor(i5FlowNeighConn *neighbor, unsigned int ifindex)
{
  i5FlowInterfaceList *interfaceItem = NULL;
  i5Trace("Neighbor: " I5_MAC_DELIM_FMT " ifindex:%d\n", I5_MAC_PRM(neighbor->neighborId),ifindex);

  interfaceItem = i5FlowFindIfIndexForNeighbor(neighbor, ifindex);
  if (interfaceItem != NULL) {
    i5LlItemFree(&neighbor->ifindexList, interfaceItem);
    if (neighbor->ifindexList.ll.next == NULL) {
      i5Trace("No ifindices, removing Neighbor\n");
      i5LlItemFree(&i5FlowConnectionList, neighbor);
    } else if ((i5FlowGetIfindexCount(neighbor) < 2) && (neighbor->activated == 1)) {
      i5FlowDeactivateFlowBond(neighbor);
    }
  }
  else {
    i5Trace("No such ifindex\n");
  }
}

void i5FlowManagerShow(void)
{
   i5FlowNeighConn *neighborList = (i5FlowNeighConn *)i5FlowConnectionList.ll.next;
   printf("Flow Manager Database\n");
   printf("---------------------\n");
   printf("Neighbor count: %d\n",i5FlowGetNeighborCount());

   while (neighborList != NULL) {
      char ifindexString[255] = "";
      printf("Neighbor: " I5_MAC_DELIM_FMT " %s \n", I5_MAC_PRM(neighborList->neighborId),
                                                                     neighborList->activated ? "ACTIVE" : "INACTIVE");
      i5FlowInterfaceList *interfaceItem = (i5FlowInterfaceList *)neighborList->ifindexList.ll.next;
      while (interfaceItem != NULL) {
         char shortString[24] = "";
         snprintf(shortString, sizeof(shortString), "%u ", interfaceItem->ifindex);
         strcat(ifindexString, shortString);
         interfaceItem = (i5FlowInterfaceList *) interfaceItem->ll.next;
      }
      printf("Ifindices: %s\n",ifindexString);
      neighborList = (i5FlowNeighConn *)neighborList->ll.next;
   }
}

static void i5FlowActivateFlowBond(i5FlowNeighConn *neighbor)
{
  if (isFbctlSupported()) {
    i5FlowInterfaceList *interfaceItem = (i5FlowInterfaceList *)neighbor->ifindexList.ll.next;
    i5Trace("Activating Neighbor: " I5_MAC_DELIM_FMT "\n", I5_MAC_PRM(neighbor->neighborId) );

    for (; interfaceItem != NULL; interfaceItem = (i5FlowInterfaceList *)interfaceItem->ll.next) {
      fbCtlAddIf(neighbor->neighborIndex, interfaceItem->ifindex);
    }
  }
  neighbor->activated = 1;
}

static void i5FlowDeactivateAll(void)
{
   i5FlowNeighConn *neighborList = (i5FlowNeighConn *)i5FlowConnectionList.ll.next;

   while (neighborList != NULL) {
      if (neighborList->activated == 1) {
        i5FlowDeactivateFlowBond(neighborList);
      }
      neighborList = (i5FlowNeighConn *)neighborList->ll.next;
   }

}

void i5FlowManagerAddConnectionIndex(unsigned char const *neighborId, unsigned int ifindex)
{
  int oldNeighborCount = i5FlowGetNeighborCount();
  int newNeighborCount = 0;

  i5Trace("adding ifindex %d\n", ifindex);
  if (neighborId == NULL) {
    i5Trace("NULL pointer\n");
    return;
  }
  i5FlowNeighConn *neighbor =  i5FlowAddNeighborId (neighborId);

  i5FlowAddIfindexToNeighbor(neighbor, ifindex);

  newNeighborCount = i5FlowGetNeighborCount();
  if (newNeighborCount == 1) {   /* This restriction will go away once Flow Bond can handle more */
    if (i5FlowGetIfindexCount(neighbor) > 1) {
      /* Invoke Flow Bond */
      i5FlowActivateFlowBond (neighbor);
    }
  }
  else if (oldNeighborCount == 1) {
    /* We've changed from 1 neighbor to something else, disable flowbond */
    i5FlowDeactivateAll();
  }
}

void i5FlowManagerRemoveConnectionIndex(unsigned char const *neighborId, unsigned int ifindex)
{
  i5FlowNeighConn *neighbor =  i5FlowGetNeighbor(neighborId);

  i5Trace("removing ifindex %d\n", ifindex);

  if (neighbor == NULL) {
    i5Trace("Neighbor ID not in database\n");
    return;
  }

  i5FlowRemoveIfindexFromNeighbor(neighbor, ifindex);
}

void i5FlowManagerActivateInterface(i5_socket_type *pif)
{

  unsigned short mediaType = i5GlueInterfaceGetMediaInfoFromName(pif->u.sll.ifname, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL);
  if(isAutoWdsSupported()) {
     /* If this is the wl interface, refresh the wds list */

     i5Trace("%s up, ifindex %d\n", pif->u.sll.ifname, pif->u.sll.sa.sll_ifindex);

     if ( i5DmIsInterfaceWireless(mediaType) ) {
       if ( 0 != strncmp(I5_GLUE_WLCFG_WDS_NAME_STRING, pif->u.sll.ifname, strlen(I5_GLUE_WLCFG_WDS_NAME_STRING)) ) {
         i5FlowManagerProcessWirelessUp();
       }
       else {
         char macString[24] = "";
         int success;

         success = i5WlCfgGetWdsMacFromName(pif->u.sll.ifname, &macString[0], 24);
         if (success == 0) {
           i5_dm_device_type *pdevice = i5DmGetSelfDevice();
           if ( (pdevice != NULL) && (pdevice->BridgingTuplesNumberOfEntries > 0)) {
             i5Trace("Setting static MAC in bridge for %s to %s.\n", pif->u.sll.ifname, macString);
             i5BrUtilAddStaticFdbEntry(pif->u.sll.ifname, macString);
           }
           else {
             i5TraceError("Unable to set static MAC - no bridge\n");
           }
         }
       }
     }
  }
  if (isFbctlSupported() && (!isAutoWdsSupported() || !i5DmIsInterfaceWireless(mediaType))) {
    i5FlowNeighConn *neighborList = (i5FlowNeighConn *)i5FlowConnectionList.ll.next;
    /* If the ifindex of the pif is in our active list, tell flow bond about it */
    /* Check through all active neighbors */
    while (neighborList != NULL) {
      if (neighborList->activated) {
        i5FlowInterfaceList *interfaceItem = (i5FlowInterfaceList *)neighborList->ifindexList.ll.next;
        while (interfaceItem != NULL) {
          if (interfaceItem->ifindex == pif->u.sll.sa.sll_ifindex) {
            i5Trace("Re-adding ifindex: %d\n", interfaceItem->ifindex);
            fbCtlAddIf(neighborList->neighborIndex, interfaceItem->ifindex);
            break;
          }
          interfaceItem = (i5FlowInterfaceList *) interfaceItem->ll.next;
        }
      }
      neighborList = (i5FlowNeighConn *)neighborList->ll.next;
    }
  }
}

static void i5FlowManagerFlushMacs (char *ifname)
{
  if (ifname) {
    i5_dm_device_type *pdevice = i5DmGetSelfDevice();
    i5_dm_bridging_tuple_info_type *pdmbr;

    if ( (pdevice != NULL) && (pdevice->BridgingTuplesNumberOfEntries > 0)) {
      pdmbr = pdevice->bridging_tuple_list.ll.next;
      i5Trace("Flush dynamic MACs from %s for %s\n", pdmbr->ifname, ifname);
      i5BrUtilFlushFdbs(pdmbr->ifname, ifname);
    }
    else {
      i5TraceInfo("Unable to flush dynamic MACs for %s, no bridge\n", ifname);
    }
  }
}

static void i5FlowClearWdsTimer(void)
{
  I5_WARN_ON_ONCE(!isAutoWdsSupported());
  if (i5FlowRegisterWdsRetry_ptmr) {
    i5TimerFree(i5FlowRegisterWdsRetry_ptmr);
    i5FlowRegisterWdsRetry_ptmr = NULL;
  }
}

void i5FlowManagerDeactivateInterface(i5_socket_type *pif)
{
  i5Trace("%s down ifindex %d\n", pif->u.sll.ifname, pif->u.sll.sa.sll_ifindex);

  if (isAutoWdsSupported()) {
    i5FlowClearWdsTimer();
  }

  /* Check through all active neighbors */
  i5FlowNeighConn *neighborList = (i5FlowNeighConn *)i5FlowConnectionList.ll.next;
  while (neighborList != NULL) {
    if (neighborList->activated) {
      i5FlowInterfaceList *interfaceItem = (i5FlowInterfaceList *)neighborList->ifindexList.ll.next;
      while (interfaceItem != NULL) {
        if (interfaceItem->ifindex == pif->u.sll.sa.sll_ifindex) {
          i5Trace("Deleting ifindex: %d\n", interfaceItem->ifindex);
          i5FlowManagerRemoveConnectionIndex(neighborList->neighborId, interfaceItem->ifindex);
          break;
        }
        interfaceItem = (i5FlowInterfaceList *) interfaceItem->ll.next;
      }
    }
    neighborList = (i5FlowNeighConn *)neighborList->ll.next;
  }
}

/* This is an unpleasant function we're using to get the PLC link cost, if we have a PLC link
 * The goal is to use this to make the Wi-Fi cost higher so PLC is always favoured
 *
 * returns - 1 - a PLC link was found
 *           0 - no PLC link was found
 */
static int i5FlowManagerGetPlcCost(unsigned int *plcCost)
{
  i5_socket_type *pif = i5_config.i5_socket_list.ll.next;

  while ( pif ) {
    if ( pif->type == i5_socket_type_ll ) {
      unsigned char netTechOui[I5_PHY_INTERFACE_NETTECHOUI_SIZE];
      unsigned short mediaType = i5GlueInterfaceGetMediaInfoFromName(pif->u.sll.ifname, NULL, NULL, netTechOui, NULL, NULL, NULL, 0, NULL);
      if (i5DmIsInterfacePlc(mediaType, netTechOui)) {
        unsigned short bandwidthRoundedOff = pif->u.sll.hystBwMbps + I5_FLOW_MANAGER_HYSTERESIS_STORAGE_BW_ROUNDOFF;
        *plcCost = i5FlowManagerApproximateLogarithm(bandwidthRoundedOff >> I5_FLOW_MANAGER_HYSTERESIS_STORAGE_BW_BITSHIFT);
        i5TraceInfo("found a PLC cost of %d\n", *plcCost);
        return 1;
      }
    }
    pif = pif->ll.next;
  }

  return 0;
}

/* When an interface that has been flagged as "new" gets solid link metrics data, *
 * this function should be called to carry out STP on ALL available interfaces    */
static void i5FlowManagerUpdateStp(i5_socket_type *pFailedSock)
{
  i5_socket_type *pif;
  unsigned int plcCost = 0;
  int plcLinkExists = 0;
  unsigned char netTechOui[I5_PHY_INTERFACE_NETTECHOUI_SIZE];
  unsigned short mediaType;

  i5TraceInfo("\n");

  if( pFailedSock != NULL ) {
    mediaType = i5GlueInterfaceGetMediaInfoFromName(pFailedSock->u.sll.ifname, NULL, NULL, netTechOui, NULL, NULL, NULL, 0, NULL);
  }

  /* If no failed interface was given, or the failed interface is NOT PLC, then check for PLC links */
  if ((pFailedSock == NULL) || !i5DmIsInterfacePlc(mediaType, netTechOui) ) {
    plcLinkExists = i5FlowManagerGetPlcCost(&plcCost);
    i5TraceInfo("PLC link exists at cost %d\n", plcCost);
  }

  pif = (i5_socket_type *)i5_config.i5_socket_list.ll.next;
  while (pif != NULL) {
    if (pif->type == i5_socket_type_ll) {
      i5_socket_type *pbrsock = i5SocketFindDevSocketByType(i5_socket_type_bridge_ll);
      if ( pbrsock ) {
        char cmdStr[256];
        unsigned short mediaType = i5GlueInterfaceGetMediaInfoFromName(pif->u.sll.ifname, NULL, NULL, netTechOui, NULL, NULL, NULL, 0, NULL);
        if (plcLinkExists) {
           /* Use plcCost for the PLC link, add 1 to the PLC cost for everyone else */
           snprintf(cmdStr, sizeof(cmdStr), "brctl setpathcost %s %s %d",
                                            pbrsock->u.sll.ifname, pif->u.sll.ifname,
                                            plcCost + (i5DmIsInterfacePlc(mediaType, netTechOui) ? 0 : 1) );
        }
        else {
           /* If there's no PLC link, use the actual values */
           unsigned short bandwidthRoundedOff = pif->u.sll.hystBwMbps + I5_FLOW_MANAGER_HYSTERESIS_STORAGE_BW_ROUNDOFF;
           snprintf(cmdStr, sizeof(cmdStr), "brctl setpathcost %s %s %d",
                                            pbrsock->u.sll.ifname, pif->u.sll.ifname,
                                            i5FlowManagerApproximateLogarithm(bandwidthRoundedOff >> I5_FLOW_MANAGER_HYSTERESIS_STORAGE_BW_BITSHIFT));
        }
        i5TraceInfo("%s\n",cmdStr);
        system(cmdStr);
      }
      else {
         i5TraceInfo("Unable to set path cost for %s, no bridge\n", pif->u.sll.ifname);
      }
    }
    pif = pif->ll.next;
  }
}

/* This raises a flag to indicate a new interface.
 * When that interface actually shows up with new metrics data, STP calls will be made. */
void i5FlowManagerProcessNewNeighbor(i5_dm_1905_neighbor_type *newNeighbor)
{
   newNeighbor->ignoreLinkMetricsCountdown = I5_FLOW_MANAGER_ITERATIONS_FOR_STP_UPDATE;
   i5Trace("\n");
}

/*  A neighbor lost means that STP calls will be made. */
void i5FlowManagerProcessNeighborRemoved(i5_dm_1905_neighbor_type *deletedNeighbor)
{
   i5_socket_type *pif = i5SocketFindDevSocketByIndex(deletedNeighbor->localIfindex);

   i5Trace("%s %d\n", deletedNeighbor->localIfname, deletedNeighbor->localIfindex);
   if (pif == NULL) {
      i5Trace("This neighbor is not local to this device.\n");
      return;
   }

   /* TBD - single interface for plc may have multiple neighbours */
   i5FlowManagerFlushMacs(deletedNeighbor->localIfname);

   pif->u.sll.hystBwMbps = 1;
   pif->u.sll.hystBandwidthDataValid = 1;

   if (isFbctlSupported())
     fbCtlTokens (0, pif->u.sll.sa.sll_ifindex, 0 /*available is zero*/ , 0 /* max = 0 means flush the flows */);
   if (isFcctlSupported())
     fcCtlFlush(pif->u.sll.sa.sll_ifindex);
   i5FlowManagerUpdateStp(pif);
}

void i5FlowManagerCheckNeighborForOverload(i5_dm_1905_neighbor_type *neighbor)
{
  i5_dm_device_type *selfDevice = neighbor->ll.parent;
  unsigned int neighborCount = 0;
  i5_dm_1905_neighbor_type *neighborList[I5_FLOW_MANAGER_MAX_NEIGHBORS] = {0};
  int firstOverloadIndex = -1;
  i5_dm_1905_neighbor_type *currNeighbor = selfDevice->neighbor1905_list.ll.next;

  I5_WARN_ON_ONCE(!isFbctlSupported());
  if (neighbor->ll.parent != i5DmGetSelfDevice()) {
    return;
  }

  /* get all neighbor structures of the local device that go to that neighbor*/
  /* TDB - use I5_FLOW_MANAGER_MAX_NEIGHBORS instead of 3 */
  while ((currNeighbor != NULL) && (neighborCount < 3)) {
    if (memcmp(neighbor->Ieee1905Id, currNeighbor->Ieee1905Id, MAC_ADDR_LEN) == 0) {
       neighborList[neighborCount] = currNeighbor;
       neighborCount ++;
    }
    currNeighbor = (i5_dm_1905_neighbor_type *)currNeighbor->ll.next;
  }

  /* Nothing to do if there's only one way to get to this device */
  if (neighborCount > 1) {
    unsigned int neighborIndex = 0;
    unsigned int unusedBandwidth = 0; /* Track the Bandwidth available on non-overloaded interfaces */
    /* Search for overloaded neighbors */
    while (neighborList[neighborIndex] != NULL && (neighborIndex < I5_FLOW_MANAGER_MAX_NEIGHBORS)) {
      i5TraceInfo("Comparing avail:%d total:%d\n",neighborList[neighborIndex]->availableThroughputCapacity, neighborList[neighborIndex]->MacThroughputCapacity);
      /* An interface is overloaded if it has less than 1/16 of its capacity available, or it has less than 2 Mbps available */
      if ( (neighborList[neighborIndex]->availableThroughputCapacity < (neighborList[neighborIndex]->MacThroughputCapacity >> 4)) ||
           (neighborList[neighborIndex]->MacThroughputCapacity == 0)) {
         /* declare overload */
         i5TraceInfo("Declaring overload on neighbor %d\n",neighborIndex);
         if (firstOverloadIndex == -1) {
            firstOverloadIndex = neighborIndex;
         }
      }
      else {
         /* not a problem, so be part of the solution */
         unusedBandwidth += neighborList[neighborIndex]->availableThroughputCapacity;
      }
      neighborIndex ++;
    }
    /* Do a flowbond Switch one interface is overloaded and the other interfaces have enough available BW to handle the load */
    if (firstOverloadIndex != -1) {
      if (unusedBandwidth > (neighborList[firstOverloadIndex]->MacThroughputCapacity - neighborList[firstOverloadIndex]->availableThroughputCapacity)) {
        /* kill the interface related to the bad neighbor */
        i5_socket_type *pif = i5SocketFindDevSocketByIndex(neighborList[firstOverloadIndex]->localIfindex);
        if ( pif ) {
          i5TraceInfo("Dumping flows from %s(%d) because other interfaces have %d Mbps\n",
                     neighborList[firstOverloadIndex]->localIfname, pif->u.sll.sa.sll_ifindex, unusedBandwidth);
          /* Use (0,0) to tell the driver that this link is overloaded */
          fbCtlTokens (0, pif->u.sll.sa.sll_ifindex, 0 , 0 /* max = 0 means overloaded */);
          /* Update driver with accurate information */
          fbCtlTokens (0, pif->u.sll.sa.sll_ifindex,
                       neighborList[firstOverloadIndex]->availableThroughputCapacity,
                       neighborList[firstOverloadIndex]->MacThroughputCapacity);
          if (isFcctlSupported()) {
            fcCtlFlush(pif->u.sll.sa.sll_ifindex);
          }
        }
      }
    }
  }
}

/* This function updates internal variables in case they are needed for STP updates */
void i5FlowManagerMetricUpdate(int ifindex, unsigned short totalBandwidthMbps, unsigned short availBandwidthMbps)
{
   unsigned short bandwidthToBeRecorded = availBandwidthMbps << I5_FLOW_MANAGER_HYSTERESIS_STORAGE_BW_BITSHIFT;
   i5_socket_type *pif = i5SocketFindDevSocketByIndex(ifindex);

   i5TraceInfo("called with %d %d/%d\n", ifindex, availBandwidthMbps, totalBandwidthMbps);

   if ( pif == NULL ) {
      return;
   }

   if ((bandwidthToBeRecorded <= pif->u.sll.hystBwMbps) || (!pif->u.sll.hystBandwidthDataValid)) {
      /* If the bandwidth goes down, it goes down right away */
      /* If this is the first time the interface has rx'd bandwidth data, use it immediately */
      pif->u.sll.hystBandwidthDataValid = 1;
      pif->u.sll.hystBwMbps = bandwidthToBeRecorded;
   } else {
      /* If the bandwidth is going up, let it go up slowly, 1/8 of the distance it actually moved */
      pif->u.sll.hystBwMbps += (bandwidthToBeRecorded - pif->u.sll.hystBwMbps)
                                    >> I5_FLOW_MANAGER_HYSTERESIS_CRAWL_UP_BITSHIFT;
   }

   i5TraceInfo("Updating %s as having BW of %d.%x\n",
                pif->u.sll.ifname,
                pif->u.sll.hystBwMbps >> I5_FLOW_MANAGER_HYSTERESIS_STORAGE_BW_BITSHIFT,
                pif->u.sll.hystBwMbps & 0x0f);
   if (isFbctlSupported()) {
      fbCtlTokens (0, pif->u.sll.sa.sll_ifindex,
                   ((unsigned int)pif->u.sll.hystBwMbps + I5_FLOW_MANAGER_HYSTERESIS_STORAGE_BW_ROUNDOFF) >> I5_FLOW_MANAGER_HYSTERESIS_STORAGE_BW_BITSHIFT,
                   totalBandwidthMbps);
   }

   if (i5_dm_network_topology.updateStpNeeded) {
      i5_dm_network_topology.updateStpNeeded = 0;
      i5FlowManagerUpdateStp(NULL);
   }
}

/* make sure that a WDS link for the given interface is in the NVRAM */
static void i5FmSetWdsAddress(char *ifname, unsigned char *interfaceId)
{
  I5_WARN_ON_ONCE(!isAutoWdsSupported());

  char macString[24] = "";
  char cmdStr[256] = "";
  snprintf(macString,sizeof(macString),I5_MAC_DELIM_FMT,I5_MAC_PRM(interfaceId));
  if (!i5InterfaceSearchFileForString("/tmp/wlwds",macString)) {
    i5Trace("Link is wireless ... registering.\n");
    snprintf(cmdStr, sizeof(cmdStr), "wl -i %s wds %s", ifname, macString);
    system(cmdStr);
  }
  else {
    i5Trace("Link already in WDS list, nothing to do.\n");
  }
}

/*  Search through its neighbors and create WDS links to any of their wireless interfaces
 *  - called when the local node realizes it is AP configured (either manually or via AP Autoconfiguration)
 */
static void i5FlowManagerRegisterAllWds(i5_dm_interface_type *plocalInterface)
{
  I5_WARN_ON_ONCE(!isAutoWdsSupported());

  char cmdStr[256] = "";
  i5_dm_device_type *device = i5_dm_network_topology.device_list.ll.next;

  i5Trace("\n");

  if (plocalInterface->MediaType == I5_MEDIA_TYPE_UNKNOWN) {
    i5Trace("No WDS links possible when media type is UNKNOWN.\n");
    return;
  }

  snprintf(cmdStr, sizeof(cmdStr), "wl -i %s wds > /tmp/wlwds", plocalInterface->wlParentName);
  system(cmdStr);

  /* loop through all devices */
  while (device != NULL) {
    /* don't make WDS links to myself */
    if ( !i5DmDeviceIsSelf(device->DeviceId) ) {
      i5TraceInfo("Device: " I5_MAC_DELIM_FMT "\n", I5_MAC_PRM(device->DeviceId) );
      /* loop through the interfaces of this neighbor */
      i5_dm_interface_type *currInterface = (i5_dm_interface_type *)device->interface_list.ll.next;
      while (currInterface != NULL) {
        i5TraceInfo("Interface: " I5_MAC_DELIM_FMT "\n", I5_MAC_PRM(currInterface->InterfaceId) );
        if (i5WlCfgAreMediaTypesCompatible(plocalInterface->MediaType, currInterface->MediaType)) {
          i5Trace("Setting WDS Address " I5_MAC_DELIM_FMT "\n", I5_MAC_PRM(currInterface->InterfaceId));
          i5FmSetWdsAddress(plocalInterface->wlParentName, currInterface->InterfaceId);
        }
        else {
          i5TraceInfo("Does not match media type.\n");
        }
        currInterface = (i5_dm_interface_type *)currInterface->ll.next;
      }
    }
    device = (i5_dm_device_type *)device->ll.next;
  }
}

static void i5FlowManagerRegisterAllWdsTimeout(void *arg)
{
  i5_dm_device_type *pdevice = i5DmGetSelfDevice();
  i5_dm_interface_type *pinterface;
  int timerRequired = 0;

  I5_WARN_ON_ONCE(!isAutoWdsSupported());
  i5Trace("\n");

  i5FlowClearWdsTimer();

  if ( NULL == pdevice ) {
    return;
  }

  pinterface = pdevice->interface_list.ll.next;
  while ( pinterface ) {
    if ( i5DmIsInterfaceWireless(pinterface->MediaType) )
    {
      timerRequired++;
      if ( (1 == i5WlCfgIsApConfigured(pinterface->wlParentName)) &&
           (1 == i5BrUtilGetPortStpState(pinterface->wlParentName)) )
      {
         timerRequired--;
         i5FlowManagerRegisterAllWds(pinterface);
      }
      else
      {
         i5Trace("Wireless interface %s is not ready\n", pinterface->wlParentName);
      }
    }
    pinterface = pinterface->ll.next;
  }
  if (timerRequired != 0 ) {
    i5FlowRegisterWdsRetry_ptmr = i5TimerNew(2000, i5FlowManagerRegisterAllWdsTimeout, NULL);
  }
}

void i5FlowManagerProcessLocalWirelessDown(void)
{
  I5_WARN_ON_ONCE(!isAutoWdsSupported());
  i5Trace("\n");
  i5FlowClearWdsTimer();
}

void i5FlowManagerProcessWirelessUp(void)
{
  I5_WARN_ON_ONCE(!isAutoWdsSupported());
  i5Trace("\n");
  if (i5DmAnyWirelessInterfaceUp(i5DmGetSelfDevice()) ) {
    if (i5FlowRegisterWdsRetry_ptmr) {
      return;
    }
    else {
      i5FlowManagerRegisterAllWdsTimeout(NULL);
    }
  }
}

void i5FlowManagerDeinit(void)
{
  if (i5FlowRegisterWdsRetry_ptmr) {
    i5TimerFree(i5FlowRegisterWdsRetry_ptmr);
  }
  while (i5FlowConnectionList.ll.next != NULL) {
    i5FlowNeighConn *pEntry = (i5FlowNeighConn *)i5FlowConnectionList.ll.next;
    while ( pEntry->ifindexList.ll.next != NULL ) {
      i5LlItemFree(&pEntry->ifindexList, pEntry->ifindexList.ll.next);
    }
    i5LlItemFree(&i5FlowConnectionList, pEntry);
  }
}
