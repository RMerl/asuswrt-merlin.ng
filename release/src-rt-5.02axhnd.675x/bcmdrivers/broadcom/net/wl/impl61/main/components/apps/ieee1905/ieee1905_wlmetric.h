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
 * $Change: 111969 $
 ***********************************************************************/

#ifndef _IEEE1905_WLMETRIC_H_
#define _IEEE1905_WLMETRIC_H_

#if defined(WIRELESS)
#include "ieee1905_wlcfg.h"
#include "typedefs.h"

#define UINT_SIZE (sizeof(uint32))

typedef struct maclist {
   uint          count;            /*   number of MAC addresses */
   unsigned char ea[1];            /*   variable length array of MAC addresses */
} WLM_1905_MACLIST;

typedef enum {
	/*  link metric */
	METRIC_1905_FAILURE,
	METRIC_1905_SUCCESS
} WLM_1905_RET_STATUS;

typedef struct
{
    unsigned char           mac[6];             /*  Station address */
    uint32                  tx_pkts;        /*  # of packets transmitted */
    uint32                  tx_failures;    /*  # of packets failed */
    uint32                  rx_ucast_pkts;  /*  # of unicast packets received */
    uint32                  rx_mcast_pkts;  /*  # of multicast packets received */
    uint32                  tx_rate;        /*  Rate of last successful tx frame */
    uint32                  rx_rate;        /*  Rate of last successful rx frame */
    uint32                  rx_decrypt_succeeds;    /*  # of packet decrypted successfully */
    uint32                  rx_decrypt_failures;    /*  # of packet decrypted unsuccessfully */
    uint32                  linkavailability;
    uint32                  macthroughput;
    uint32                  tx_used_rates; /* throughput of this sta */
    uint32                  tx_used_total; /* throughput of all the STAs */
    uint32                  tx_bytes;
    uint32                  rx_bytes;
    int32                   tx_available_bandwidth; /* tx available bandwidth */
    int8                    knoise;
    uint32                  interval;
} WLM_1905_LINK_INFO;

typedef struct
{
    uint32 count;
    WLM_1905_LINK_INFO linkinfo[1];
} WLM_1905_LINK_INFOS;

typedef enum wlm_1905_wllink_type
{
    WLM_1905_WLLINK_STA,
    WLM_1905_WLLINK_WDS,
    WLM_1905_WLLINK_PSTA
} WLM_1905_WLLINK_TYPE;

typedef struct wlm_1905_wllink
{
    unsigned char mac[6];
    unsigned char type;
} WLM_1905_WLLINK;

typedef struct wlm_1905_wllinks
{
    unsigned int count;
    WLM_1905_WLLINK links[1];
} WLM_1905_WLLINKS;

/*  Linux network driver ioctl encoding */
typedef struct wl_ioctl
{
    uint cmd;       /*  common ioctl definition */
    void *buf;      /*  pointer to user buffer */
    uint len;       /*  length of user buffer */
    uint8 set;              /*  1=set IOCTL; 0=query IOCTL */
    uint used;      /*  bytes read or written (optional) */
    uint needed;    /*  bytes needed (optional) */
} wl_ioctl_t;

#define WLC_GET_VAR (262)
#define WLC_SET_VAR (263)

WLM_1905_RET_STATUS wlm_1905_GetWlLinkMetric(char const *ifname,unsigned char *macs,unsigned char **wlmetric,int *len);
WLM_1905_RET_STATUS wlm_1905_GetWlLinks(char const *ifname,unsigned char **wllinks);
int wlm_1905_i5ctl_handler(void *psock, void *pMsg);
void i5WlLinkMetricsOverrideBandwidth (unsigned int availMbps, unsigned int macThroughMbps, unsigned char numOverrides);
int i5wlmMetricUpdateLinkMetrics(char *wlname, int numMacs, unsigned char const *macAddresses);

#ifdef MULTIAP
void i5WlmUpdateMAPMetrics(void *arg);
#endif /* MULTIAP */
#endif /* WIRELESS */
#endif // endif
