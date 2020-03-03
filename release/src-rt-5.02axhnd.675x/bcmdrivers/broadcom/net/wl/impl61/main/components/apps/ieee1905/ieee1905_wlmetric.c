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
 * $Change$
 ***********************************************************************/
#if defined(WIRELESS)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include "i5ctl_wlcfg.h"
#include "ieee1905_wlmetric.h"
#include "ieee1905_wlcfg.h"
#include "ieee1905_trace.h"
#include "ieee1905_datamodel_priv.h"
#include "ieee1905_flowmanager.h"
#ifdef MULTIAP
#include "ieee1905_ethstat.h"
#endif /* MULTIAP */
#define I5_TRACE_MODULE i5TraceWlcfg
#define MAXCOUNT_MACLIST 128
#define I5_WL_BANDWIDTH_UPDATE_INTERVAL_MSEC 1000

static unsigned char wlBandwidthOverridesLeft = 0;
static unsigned int wlOverrideAvailBandwidthKbps = 0;
static unsigned int wlOverrideMacThroughputKbps = 0;

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  hexdump_ascii
 *  Description:  internal function to print char array contetn
 * =====================================================================================
 */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  wlm_1905_GetWlLinks
 *  Description:  API to retrieve wl sta/wds
 *   Parameters:  wllinks- pointer to return values
 *       return:
 * =====================================================================================
 */
WLM_1905_RET_STATUS wlm_1905_GetWlLinks(char const *ifname,unsigned char **wllinks)
{
    int length=MAXCOUNT_MACLIST*7+sizeof(unsigned int);
    *wllinks=malloc(length);
    memcpy(*wllinks,"1905_sta_list",14);
    if (wl_ioctl((char *)ifname, WLC_GET_VAR, *wllinks,length))
    {
        return METRIC_1905_FAILURE;
    }
    else
    {
        return METRIC_1905_SUCCESS;
    }
}

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  wlm_1905_GetWlLinkMetric
 *  Description:  API to get metric for macs in parameter macs.
 *   Parameters:
 *       return:
 * =====================================================================================
 */
WLM_1905_RET_STATUS wlm_1905_GetWlLinkMetric(char const *ifname,unsigned char *macs,unsigned char **wlmetric,int *len)
{
    WLM_1905_MACLIST *maclist=(WLM_1905_MACLIST *)macs;
    if(maclist->count>0)
    {
        char *cmd="1905_get_mtcs ";
        int length=maclist->count*sizeof(WLM_1905_LINK_INFO)+UINT_SIZE+14;
        *wlmetric=malloc(length);
        if(!*wlmetric) return -1;
        memcpy(*wlmetric,cmd,14);
        (*wlmetric)[13]='\0';
        memcpy(*wlmetric+14,macs,maclist->count*MAC_ADDR_LEN+UINT_SIZE);
        if(wl_ioctl((char *)ifname,WLC_GET_VAR,*wlmetric,length))
        {
            return METRIC_1905_FAILURE;
        }
        else
        {
            return METRIC_1905_SUCCESS;
        }
    }
    return METRIC_1905_FAILURE;
}

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  wlm_1905_print_metric
 *  Description:  sample fucntion on handling metric update from wireless driver, it is
 *  		  getting called from wlm_1905_Process_Message
 *   Parameters:
 *       return:
 * =====================================================================================
 */
static void wlm_1905_print_metric(unsigned char *msg)
{

    int i=0;
    WLM_1905_LINK_INFOS *pLinkInfos = (WLM_1905_LINK_INFOS *)msg;
    WLM_1905_LINK_INFO  *sta = &pLinkInfos->linkinfo[0];
    for(; i<pLinkInfos->count; i++)
    {
        printf("\t---------------------------------------------------\n");
        printf("\t phy rate:		%u kbps\n", (sta->tx_rate));
        printf("\t linkavailabilty:	%u%% \n", (sta->linkavailability));
        printf("\t mac rate:		%u kbps\n", (sta->macthroughput));
        printf("\t tx used bandwidth:	%u kbps \n", (sta->tx_used_total));
        printf("\t leftover bandwidth:	%u kbps \n", (sta->tx_available_bandwidth));
        printf("\t---------------------------------------------------\n");
        printf("\t tx pkts:	 	%u\n", (sta->tx_pkts));
        printf("\t tx failures:	 	%u\n", (sta->tx_failures));
        printf("\t rx ucast pkts:	 	%u\n", (sta->rx_ucast_pkts));
        printf("\t rx mcast/bcast pkts: 	%u\n", (sta->rx_mcast_pkts));
        printf("\t rate of last tx pkt: 	%u kbps\n", (sta->tx_rate));
        printf("\t rate of last rx pkt: 	%u kbps\n", (sta->rx_rate));
        printf("\t rx decrypt succeeds: 	%u\n", (sta->rx_decrypt_succeeds));
        printf("\t rx decrypt failures: 	%u\n", (sta->rx_decrypt_failures));
        printf("\t tx bandwidth:		%u kbps \n", (sta->tx_used_rates));
        printf("\t tx bytes:	 	%u\n", (sta->tx_bytes));
        printf("\t rx bytes:	 	%u\n", (sta->rx_bytes));
        printf("\t interval:	 	%u\n", (sta->interval));
        sta++;
    }
}

/* Debug function to override Available and Max Throughput bandwidths
 *
 * avail - in Mbps
 * maxThrough - in Mbps
 * numOverrides - 0 - turn off override
 *              - 255 - forever (or until turned off)
 *              - [1,254] - that many times
 */
void i5WlLinkMetricsOverrideBandwidth (unsigned int availMbps, unsigned int macThroughMbps, unsigned char numOverrides)
{
  wlBandwidthOverridesLeft = numOverrides;
  if (numOverrides) {
    wlOverrideAvailBandwidthKbps = availMbps << 10;
    wlOverrideMacThroughputKbps = macThroughMbps << 10;
  }
}

static void i5WlLinkMetricsProcessNotification (unsigned char *msg, int dataLength)
{
    int i=0;
    WLM_1905_LINK_INFOS *pLinkInfos = (WLM_1905_LINK_INFOS *)msg;
    WLM_1905_LINK_INFO  *sta = &pLinkInfos->linkinfo[0];

    int calculatedLength = pLinkInfos->count * sizeof(WLM_1905_LINK_INFO) + sizeof(pLinkInfos->count);

    if (dataLength != calculatedLength) {
      i5TraceError("FAILED. Rx'd len=%d bytes. Expected %d bytes because %d metrics (%zu each)\n", dataLength, calculatedLength, pLinkInfos->count, sizeof(WLM_1905_LINK_INFO));
      return;
    }

    i5TraceInfo("Mac Addresses Rx'd = %d\n", pLinkInfos->count);

    for( ; i < pLinkInfos->count; i++)
    {
        i5_dm_1905_neighbor_type *neighbor = i5Dm1905GetLocalNeighbor(sta->mac);
        if (neighbor) {
           i5TraceInfo("BW(kbps) Tot = %d Avail = %d rxbytes=%d\n", sta->macthroughput, sta->tx_available_bandwidth, sta->rx_bytes);
           if (wlBandwidthOverridesLeft) {
              sta->macthroughput = wlOverrideMacThroughputKbps;
              sta->tx_available_bandwidth = wlOverrideAvailBandwidthKbps;
              if (wlBandwidthOverridesLeft < 255) {
                wlBandwidthOverridesLeft --;
              }
              i5TraceInfo("BW(kbps) Tot = %d Avail = %d\n", sta->macthroughput, sta->tx_available_bandwidth);
           }
           i5Dm1905NeighborBandwidthUpdate(neighbor, sta->macthroughput >> 10, sta->tx_available_bandwidth >> 10, sta->rx_bytes, i5_config.i5_mac_address);
#if defined(SUPPORT_IEEE1905_FM)
           i5FlowManagerMetricUpdate(neighbor->localIfindex, ((sta->macthroughput) >> 10), (sta->tx_available_bandwidth >> 10));
#endif // endif
        }
        sta++;
    }
}

int i5wlmMetricUpdateLinkMetrics(char *wlname, int numMacs, unsigned char const *macAddresses)
{
   WLM_1905_MACLIST *metricConfig;
   unsigned char    *pMsg;
   int               length;
   char             *cmd="1905_get_mtcs";

   length = (numMacs*sizeof(WLM_1905_LINK_INFO)) + sizeof(int) + strlen(cmd) + 1;
   pMsg = malloc(length);
   if ( pMsg == NULL ) {
     return -1;
   }

   memcpy(pMsg, cmd, strlen(cmd));
   pMsg[strlen(cmd)] = '\0';
   metricConfig = (WLM_1905_MACLIST *)&pMsg[strlen(cmd)+1];
   metricConfig->count = numMacs;
   memcpy(metricConfig->ea, macAddresses, (numMacs*MAC_ADDR_LEN));
   if(wl_ioctl(wlname,WLC_GET_VAR, pMsg, length)) {
     free(pMsg);
     return -1;
   }

   i5WlLinkMetricsProcessNotification(pMsg, length - (strlen(cmd) + 1) );
   free(pMsg);
   return 0;
}

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  i5ctlWlmetricHandler
 *  Description:  function handler to handle i5ctl command
 * =====================================================================================
 */
int wlm_1905_i5ctl_handler(void *ps, void *pM)
{

    t_I5_API_WLCFG_MSG *pMsg=(t_I5_API_WLCFG_MSG *)pM;
    int status=-1;
    unsigned char *msg=NULL;
    int len=0;

    i5Trace("sub command is %d\n", pMsg->subcmd);
    switch(pMsg->subcmd)
    {
        case I5_CTL_WLMETRIC_GETWLLINKS:
            status=wlm_1905_GetWlLinks(pMsg->ifname,&msg);
            if(METRIC_1905_SUCCESS == status) {
                int i=0,j=0;
                WLM_1905_WLLINKS *links=(WLM_1905_WLLINKS *)msg;
                WLM_1905_WLLINK link;
                for(; i<links->count; i++)
                {
                    link=links->links[i];
                    printf("mac address:");
                    for(j=0; j<6; j++) {
                        printf(":%02x",link.mac[j]);
                    }
                    printf(" \twds link:%s\n",link.type==WLM_1905_WLLINK_WDS?"YES":"NO");
                }
            }
            if ( msg ) {
                free(msg);
            }
            break;

        case I5_CTL_WLMETRIC_LINKMETRIC:
        {
            unsigned char *macs=(unsigned char *)((void *)pMsg+sizeof(t_I5_API_WLCFG_MSG));
            status=wlm_1905_GetWlLinkMetric(pMsg->ifname,macs,&msg,&len);
            if(METRIC_1905_SUCCESS == status) {
                wlm_1905_print_metric((unsigned char *)msg) ;
            }
            if ( msg ) {
                free(msg);
            }
            break;
        }

        default:
            break;
    }
    return status;
}

#ifdef MULTIAP
void i5WlmGetAssociatedSTALinkMetric(i5_dm_device_type *pDeviceController,
  i5_dm_clients_type *pdmclient, ieee1905_ifr_metricrpt *metricrpt, ieee1905_vendor_data *vndr_data)
{
  i5_dm_bss_type *pbss = (i5_dm_bss_type*)I5LL_PARENT(pdmclient);
  ieee1905_sta_link_metric link_metric;
  ieee1905_sta_traffic_stats traffic_stats;

  memset(&link_metric, 0, sizeof(link_metric));

  /* Get the Associated STA link metrics and STA traffic stats */
  if (i5_config.cbs.assoc_sta_metric) {
    i5_config.cbs.assoc_sta_metric(pbss->ifname, pbss->BSSID, pdmclient->mac, &link_metric,
      &traffic_stats, vndr_data);
    clock_gettime(CLOCK_REALTIME, &link_metric.queried);
    memcpy(&pdmclient->link_metric, &link_metric, sizeof(pdmclient->link_metric));
    memcpy(&pdmclient->traffic_stats, &traffic_stats, sizeof(pdmclient->traffic_stats));
  }
}

/* Update all MAP related link metrics */
void i5WlmUpdateMAPMetrics(void *arg)
{
  int isCreateTimer = 0;
  i5_dm_device_type *pDevice;

  i5TraceInfo("\n");

  if (i5_dm_network_topology.pLinkMetricTimer) {
    i5TimerFree(i5_dm_network_topology.pLinkMetricTimer);
    i5_dm_network_topology.pLinkMetricTimer = NULL;
    isCreateTimer = 1;
  }

  pDevice = i5DmGetSelfDevice();
  if ( NULL != pDevice ) {
    i5_dm_interface_type *pInterface = pDevice->interface_list.ll.next;
    while ( pInterface != NULL ) {
      /* update the metrics for each Ethernet interface and then update
         the corresponding neighbor entries */
      if ( i5DmIsInterfaceEthernet(pInterface->MediaType) ) {
        i5_socket_type *pif = i5SocketFindDevSocketByAddr(pInterface->InterfaceId, NULL);
        while ( pif != NULL ) {
          i5TraceModuleInfo(i5TraceEthStat, "Update stats for %s (%d)\n", i5SocketGetIfName(pif),
            i5SocketGetIfIndex(pif));
          if ( pif->u.sll.pInterfaceCtx != NULL ) {
            int rt = 0;
            ieee1905_backhaul_link_metric metric;

            memset(&metric, 0, sizeof(metric));
            rt = i5EthStatGetBackhaulLinkMetric(pif->u.sll.pInterfaceCtx, &metric);
            if (rt >= 0) {
              i5_dm_1905_neighbor_type *pNeighbor = pDevice->neighbor1905_list.ll.next;
              while (pNeighbor != NULL) {
                if ( pNeighbor->localIfindex == i5SocketGetIfIndex(pif)) {
                  i5Dm1905NeighborLinkMetricUpdate(pNeighbor, &metric, 0,
                    (I5_DM_LINK_METRIC_UPDATE_TX | I5_DM_LINK_METRIC_UPDATE_RX | I5_DM_LINK_METRIC_UPDATE_LOCAL));
                }
                pNeighbor = pNeighbor->ll.next;
              }
            }
          }
          pif = i5SocketFindDevSocketByAddr(pInterface->InterfaceId, pif);
        }
      }

#if defined(WIRELESS)
      /* collect WL neigbour devices and query WL for metrics*/
      if ( i5DmIsInterfaceWireless(pInterface->MediaType) ) {
        ieee1905_backhaul_link_metric metric;
        i5_dm_1905_neighbor_type *pNeighbor = pDevice->neighbor1905_list.ll.next;

        while (pNeighbor != NULL) {
          if (0 == memcmp(pNeighbor->LocalInterfaceId, pInterface->InterfaceId, MAC_ADDR_LEN)) {
            memset(&metric, 0, sizeof(metric));
            if (i5_config.cbs.backhaul_link_metric) {
              i5_config.cbs.backhaul_link_metric(pInterface->ifname, pNeighbor->NeighborInterfaceId,
                &metric);
            }
            metric.phyRate = 0xFFFF; /* For wireless phyrate should be reported as 0xFFFF */
            i5Dm1905NeighborLinkMetricUpdate(pNeighbor, &metric, 0,
              (I5_DM_LINK_METRIC_UPDATE_TX | I5_DM_LINK_METRIC_UPDATE_RX | I5_DM_LINK_METRIC_UPDATE_LOCAL));
          }
          pNeighbor = pNeighbor->ll.next;
        }
      }
#endif // endif
      pInterface = pInterface->ll.next;
    }
  }

  if (isCreateTimer) {
    i5_dm_network_topology.pLinkMetricTimer = i5TimerNew(I5_DM_LINK_METRICS_GET_INTERVAL_MSEC,
      i5WlmUpdateMAPMetrics, NULL);
  }
}
#endif /* MULTIAP */
#endif // endif
