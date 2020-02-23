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

/*
 * IEEE1905 Eth Stats
 */

#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <linux/ethtool.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include "ieee1905_socket.h"
#include "ieee1905_trace.h"
#include "ieee1905_ethstat.h"
#include "ieee1905_datamodel_priv.h"

#define I5_TRACE_MODULE i5TraceEthStat

/* BRCM ethernet adapter speed */
#define I5_ETHSTAT_ETH_CAPACITY 1000  /* 1000 Mbps */
#define I5_ETHSTAT_TOTAL_FRAME_SIZE 1538  /* In Bytes */
#define I5_ETHSTAT_FRAME_PAYLOAD_SIZE 1500  /* In Bytes */

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif // endif

#ifndef SIOCETHTOOL
#define SIOCETHTOOL     0x8946
#endif // endif

#define min(a,b) \
  ({ __typeof__ (a) _a = (a); \
     __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

#define max(a,b) \
  ({ __typeof__ (a) _a = (a); \
     __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

enum {
    ET_TX_BYTES = 0,
    ET_TX_PACKETS,
    ET_TX_ERRORS,
    ET_TX_CAPACITY,
    ET_RX_BYTES,
    ET_RX_PACKETS,
    ET_RX_ERRORS,
    ET_MAX
} Bcm63xxEnetStats;

int gEthtoolStatIdx[ET_MAX] = {};
int gEthtoolStatIdxMax = ET_MAX;

typedef struct _IfCtx {
    char                name[I5_MAX_IFNAME];
    struct timespec     lastUpdateTime;     //in jiffies
    unsigned long long  lastReadBytes;      //rx bytes
    unsigned long long  lastSentBytes;      //tx bytes
    long                dataRate;           //in bytes per second
    long                capacity;           //in bytes per second
    int                 isDataRateValid;
    int                 isReadBytesValid;
    int                 fd;         /* socket suitable for ethtool ioctl */
    struct ifreq        ifr;        /* ifreq suitable for ethtool ioctl */
} IfCtx;

//----------------------- LOCAL FUNCTIONS -----------------------

#ifdef MULTIAP
/* Gets the stats from the "/sys/class/net/<ifr_name>/<stat_name>" file */
static unsigned int getEthInterfaceStat(char *ifname, char *stat_name)
{
  FILE *fp;
  unsigned int stat = 0;
  char file_path[125];

  snprintf(file_path, sizeof(file_path), "/sys/class/net/%s/%s", ifname, stat_name);

  if((fp = fopen(file_path, "r")) != NULL) {
    char val[20];

    if (fgets(val, sizeof(val), fp) != NULL) {
      stat = (unsigned int)strtoul(val, NULL, 0);
    }
    fclose(fp);
  }

  return ((unsigned int)stat);
}

/* Gets the stats from the "/sys/class/net/<ifr_name>/<stat_name>" file */
static unsigned long long getULLEthInterfaceStat(char *ifname, char *stat_name)
{
  FILE *fp;
  unsigned long long stat = 0;
  char file_path[125];

  snprintf(file_path, sizeof(file_path), "/sys/class/net/%s/%s", ifname, stat_name);

  if((fp = fopen(file_path, "r")) != NULL) {
    char val[20];

    if (fgets(val, sizeof(val), fp) != NULL) {
      stat = strtoull(val, NULL, 0);
    }
    fclose(fp);
  }

  return ((unsigned long long)stat);
}
#endif /* MULTIAP */

//----------------------- GLOBAL FUNCTIONS -----------------------

void i5EthStatInit()
{

    int idx;
    i5Trace("\n");

    // TBD: This is oversimplified, and assumes that Bcm63xxEnetStats matches
    // what is in bcmenet.c.  Later, do something that parses the strings
    for (idx = 0; idx < ET_MAX; idx++){
        gEthtoolStatIdx[idx] = idx;
    }
}

void i5EthStatFinish()
{
    i5Trace("\n");
}

void * i5EthStatGetCtx(const char *ifName)
{
    IfCtx *ifCtx;

    if (ifName == NULL)
        return NULL;
    ifCtx = (IfCtx *)calloc(1,sizeof(IfCtx));
    if (ifCtx == NULL)
        return NULL;
    strncpy(ifCtx->name, ifName, I5_MAX_IFNAME-1);
    ifCtx->name[I5_MAX_IFNAME-1] = '\0';
    strncpy(ifCtx->ifr.ifr_name, ifName, I5_MAX_IFNAME-1);
    ifCtx->ifr.ifr_name[I5_MAX_IFNAME-1] = '\0';
    ifCtx->fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (ifCtx->fd < 0) {
        i5TraceError( "Could not open control socket\n");
        free(ifCtx);
        return NULL;
    }
    i5Trace("ifName=%s, ctx = %p\n", ifName, ifCtx);

    return ifCtx;
}

void i5EthStatFreeCtx(void * ctx)
{
    IfCtx *ifCtx = (IfCtx *)ctx;
    i5Trace("ctx=%p\n", ctx);
    close(ifCtx->fd);
    // setting to 0 to make errors more obvious:
    memset(ifCtx, 0, sizeof(IfCtx));
    free(ifCtx);
}

#ifndef MULTIAP
int i5EthStatGet(void * ctx)
{
    IfCtx *ifCtx = (IfCtx *)ctx;

    struct timespec updateTime;
    long deltaTime = 0;
    int err;
    char  __attribute__ ((aligned (64)))statsBuff[ sizeof(struct ethtool_stats) + ET_MAX * sizeof(__u64) ] = {};
    struct ethtool_stats *stats = (void *)statsBuff;

    i5TraceInfo("ctx=%p\n", ctx);

    stats->cmd = ETHTOOL_GSTATS;
    stats->n_stats = gEthtoolStatIdxMax;

    ifCtx->ifr.ifr_data = stats;

    err = ioctl(ifCtx->fd, SIOCETHTOOL, &ifCtx->ifr);
    if (err < 0) {
        i5TraceError( "Eth Ioctl failed for %s (%d)\n", ifCtx->name, err );
        return -1;
    }

    if (1) {  // debug code...
        int i = 0;
        for (i = 0; i < ET_MAX; i++)
          i5TraceInfo("gEthtoolStatIdx[%d]=%d (%llu) (%lu)\n", i, gEthtoolStatIdx[i], stats->data[gEthtoolStatIdx[i]], ifCtx->lastReadBytes );
    }

    clock_gettime(CLOCK_REALTIME, &updateTime);
    if (ifCtx->isReadBytesValid)
        deltaTime = getDeltaTimeInMs(&ifCtx->lastUpdateTime, &updateTime);

    i5TraceInfo("ctx=%p (deltaTime=%d.%03ds)\n", ctx, (int)deltaTime/1000, (int)deltaTime % 1000);

    if (!ifCtx->isReadBytesValid || deltaTime > I5_ETHSTAT_MIN_DELTA_TIME)
    {
        unsigned long oldBytes=0;
        ifCtx->lastUpdateTime = updateTime;
        if (ifCtx->isReadBytesValid) {
             oldBytes = ifCtx->lastReadBytes;
        }
        ifCtx->lastReadBytes = stats->data[gEthtoolStatIdx[ET_TX_BYTES]];
        i5TraceInfo("ifCtx->lastReadBytes=%ld\n", ifCtx->lastReadBytes);
        ifCtx->capacity = stats->data[gEthtoolStatIdx[ET_TX_CAPACITY]];

        if (ifCtx->isReadBytesValid) {
            ifCtx->dataRate = min(((ifCtx->lastReadBytes - oldBytes)/deltaTime)*1000L, ifCtx->capacity);
            i5TraceInfo("ifCtx->dataRate=%ld (oldBytes=%ld, ifCtx->lastReadBytes=%ld, ifCtx->capacity=%ld, deltaTime=%ld)\n",
                ifCtx->dataRate, oldBytes, ifCtx->lastReadBytes, ifCtx->capacity, deltaTime );
            ifCtx->isDataRateValid = 1;
        }
        else {
            ifCtx->isReadBytesValid = 1;
        }
    }
    return 0;
}

int i5EthStatGetDataRateAndCapacity(void * ctx, long *dataRate, long *capacity)
{
    IfCtx *ifCtx = (IfCtx *)ctx;

    i5EthStatGet(ctx);

    if (! ifCtx->isDataRateValid )
        return -1;

    if (dataRate != NULL) {
        *dataRate = ifCtx->dataRate;
        i5TraceInfo("ifCtx->dataRate=%ld\n", ifCtx->dataRate);
    }
    if (capacity != NULL)
        *capacity = ifCtx->capacity;

    return 0;
}

#else /* MULTIAP */
/* Get both transmitter and reciever ethernet link metrics */
int i5EthStatGetBackhaulLinkMetric(void *ctx, ieee1905_backhaul_link_metric *metric)
{
  IfCtx *ifCtx = (IfCtx *)ctx;
  unsigned long speed = 0, maxFrameRate, linkThroughput;
  struct timespec updateTime;
  long deltaTime = 0;
  const unsigned long MBit = 1000000UL;

  clock_gettime(CLOCK_REALTIME, &updateTime);

  if (ifCtx->isReadBytesValid) {
    deltaTime = getDeltaTimeInMs(&ifCtx->lastUpdateTime, &updateTime);
  }

  i5TraceInfo("ctx=%p isReadBytesValid=%d (deltaTime=%d.%03ds)\n", ctx, ifCtx->isReadBytesValid,
    (int)deltaTime/1000, (int)deltaTime % 1000);

  if (!ifCtx->isReadBytesValid || deltaTime > I5_ETHSTAT_MIN_DELTA_TIME) {
    unsigned long long oldRxBytes=0, oldTxBytes=0;

    ifCtx->lastUpdateTime = updateTime;

    if (ifCtx->isReadBytesValid) {
      oldRxBytes = ifCtx->lastReadBytes;
      oldTxBytes = ifCtx->lastSentBytes;
    }

    ifCtx->lastReadBytes = getULLEthInterfaceStat(ifCtx->name, "statistics/rx_bytes");
    ifCtx->lastSentBytes = getULLEthInterfaceStat(ifCtx->name, "statistics/tx_bytes");
    i5TraceInfo("ifCtx->lastReadBytes=%llu ifCtx->lastSentBytes=%llu\n", ifCtx->lastReadBytes,
      ifCtx->lastSentBytes);
    metric->latestRxBytes = (unsigned int)ifCtx->lastReadBytes;

    metric->txPacketErrors = getEthInterfaceStat(ifCtx->name, "statistics/tx_errors");
    metric->transmittedPackets = getEthInterfaceStat(ifCtx->name, "statistics/tx_packets");

    /* get the capacity */
    speed = (unsigned short)getEthInterfaceStat(ifCtx->name, "speed");
    if (speed <= 0) {
      speed = I5_ETHSTAT_ETH_CAPACITY; /* 1 Gbps */
      i5TraceInfo("ifname[%s] Setting default speed[%lu]\n", ifCtx->name, speed);
    }
    metric->phyRate = (unsigned short)speed;
    speed = (speed * 1000000) / 8; /* Convert to Bytes per second */
    maxFrameRate = speed / I5_ETHSTAT_TOTAL_FRAME_SIZE;
    linkThroughput = maxFrameRate * I5_ETHSTAT_FRAME_PAYLOAD_SIZE;
    ifCtx->capacity = (long)linkThroughput;

    if (ifCtx->isReadBytesValid) {
      unsigned long long diffRx = 0, diffTx = 0;
      unsigned long drate;

      diffRx = ifCtx->lastReadBytes - oldRxBytes;
      diffTx = ifCtx->lastSentBytes - oldTxBytes;
      drate = (unsigned long)((float)(diffRx + diffTx)/(float)deltaTime);
      ifCtx->dataRate = min(drate * 1000UL, ifCtx->capacity);
      i5TraceInfo("ifCtx->dataRate=%ldBps (oldRxBytes=%llu, ifCtx->lastReadBytes=%llu, "
        "oldTxBytes=%llu, ifCtx->lastSentBytes=%llu, ifCtx->capacity=%ldBps, deltaTime=%ld)\n",
        ifCtx->dataRate, oldRxBytes, ifCtx->lastReadBytes, oldTxBytes, ifCtx->lastSentBytes,
        ifCtx->capacity, deltaTime );
        ifCtx->isDataRateValid = 1;
    } else {
      ifCtx->isReadBytesValid = 1;
    }

    metric->macThroughPutCapacity = (unsigned short)((ifCtx->capacity * 8)/ MBit);
    metric->linkAvailability = (unsigned short)(((float)(ifCtx->capacity - ifCtx->dataRate) / (float)ifCtx->capacity) * 100);
    metric->receivedPackets = getEthInterfaceStat(ifCtx->name, "statistics/rx_packets");
    metric->rxPacketErrors = getEthInterfaceStat(ifCtx->name, "statistics/rx_errors");
    metric->rcpi = (unsigned char)0xFF;
    i5TraceInfo("ifname[%s] txPacketErrors[%u] transmittedPackets[%u] phyRate[%d] "
      "macThroughPutCapacity[%d] linkAvailability[%d] receivedPackets[%u] rxPacketErrors[%u]\n",
      ifCtx->name, metric->txPacketErrors, metric->transmittedPackets, metric->phyRate,
      metric->macThroughPutCapacity, metric->linkAvailability, metric->receivedPackets,
      metric->rxPacketErrors);
  } else {
    return -1;
  }

  return 0;
}
#endif /* MULTIAP */

void i5EthstatGetParentInterface(char const *ifName, char *parentIfName)
{
  char *dotLocation;

  strcpy(parentIfName, ifName);
  dotLocation = strchr(parentIfName, '.');
  if (dotLocation != NULL) {
    dotLocation[0] = '\0';
  }
}

unsigned short i5EthStatFetchIfInfo(char const *ifname, unsigned char *pMediaInfo, int *pMediaLen,
                                    unsigned char *netTechOui, unsigned char *netTechVariant, unsigned char *netTechName, unsigned char *url, int sizeUrl)
{
  if ( pMediaInfo ) {
    *pMediaLen = 0;
  }
  return I5_MEDIA_TYPE_GIGA_ETH;
}
