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

#ifndef _IEEE1905_DATAMODEL_H_
#define _IEEE1905_DATAMODEL_H_

/*
 * IEEE1905 Data-Model
 */

#include "ieee1905_defines.h"
#include "ieee1905_linkedlist.h"
#include <bcmwifi_channels.h>

#define I5_DM_VERSION 0

typedef int (*i5MacAddressDeliveryFunc)(unsigned char const * macAddressList, unsigned char numMacs);

#ifdef MULTIAP

/* Bit flags used in i5_dm_clients_type structure */
#define I5_CLIENT_FLAG_QUERY_STATE    0x01  /* 0 means pending, 1 means done */
#define I5_CLIENT_FLAG_XXX            0x02  /* This is not used now. we can use it later */
#define I5_CLIENT_FLAG_BSTA           0x04  /* This is same as IEEE1905_MAP_FLAG_STA */

#define I5_CLIENT_IS_BSTA(client)    ((client)->flags & I5_CLIENT_FLAG_BSTA)

/* Bit flags used in i5_dm_bss_type structure */
#define I5_BSS_FLAG_QUERY_STATE    0x01  /* 0 means pending, 1 means done */

/* Default TX_RATE for ethernet agents, is currently 1 Gbps(1000 Mbps) */
#define I5_DM_DEFAULT_ETH_TX_RATE	1000

/* Bit flags for status book-keeping used in i5_dm_interface_type structure */
#define I5_FLAG_IFR_M1_SENT		0x1
#define I5_FLAG_IFR_M2_RECEIVED		0x2
#define I5_FLAG_IFR_CAC_PENDING           0x80  /* CAC is not completed */

#define I5_IS_M1_SENT(flags)		((flags) & I5_FLAG_IFR_M1_SENT)
#define I5_IS_M2_RECEIVED(flags)	((flags) & I5_FLAG_IFR_M2_RECEIVED)
#define I5_IS_IFR_CAC_PENDING(flags)            ((flags) & I5_FLAG_IFR_CAC_PENDING)

/* SSID Type */
typedef struct {
  unsigned char	SSID_len;
  unsigned char	SSID[IEEE1905_MAX_SSID_LEN + 1];
} ieee1905_ssid_type;

typedef struct {
  unsigned char include_bit_esp; /* Include bit for the Estimated Service Parameters Information.
                                 * Its of type IEEE1905_INCL_BIT_ESP_XX
                                 */
  unsigned char esp_ac_be[IEEE1905_ESP_LEN]; /* ESP Information field for AC=BE */
  unsigned char esp_ac_bk[IEEE1905_ESP_LEN]; /* ESP Information field for AC=BK */
  unsigned char esp_ac_vo[IEEE1905_ESP_LEN]; /* ESP Information field for AC=VO */
  unsigned char esp_ac_vi[IEEE1905_ESP_LEN]; /* ESP Information field for AC=VI */
} ieee1905_ap_metric;

/* VHT Capabilities */
typedef struct {
  unsigned short  TxMCSMap;	/* VHT Tx MCS map */
  unsigned short  RxMCSMap;	/* VHT Rx MCS map */
  unsigned char  CapsEx;	/* VHT extended capabilities */
  unsigned char  Caps;		/* VHT capabilities */
  unsigned char  Valid;		/* Wheter VHT caps are valid or not. */
} ieee1905_vht_caps_type;

/* Radio Capabilities */
typedef struct {
  unsigned char maxBSSSupported;  /* Maximum number of BSSs supported by this radio */
  unsigned char Len;	/* length of List */
  unsigned char Valid;	/* Whether radio caps are valid or not */
  unsigned char *List;	/* List of opclass */
  unsigned short ListSize;  /* Memory allocated to the List */
} ieee1905_radio_caps_type;

/* HE Capabilities */
typedef struct {
  unsigned short TxBW80MCSMap;		/* HE 80Mhz Tx MCS map */
  unsigned short RxBW80MCSMap;		/* HE 80Mhz Tx MCS map */
  unsigned short TxBW160MCSMap;		/* HE 160Mhz Tx MCS map */
  unsigned short RxBW160MCSMap;		/* HE 160Mhz Tx MCS map */
  unsigned short TxBW80p80MCSMap;	/* HE 80p80Mhz Tx MCS map */
  unsigned short RxBW80p80MCSMap;	/* HE 80p80Mhz Tx MCS map */
  unsigned char  CapsEx;		/* HE extended capabilities */
  unsigned char  Caps;			/* HE capabilities */
  unsigned char Valid;			/* Whether HE caps are valid or not */
} ieee1905_he_caps_type;

/* AP capabilities */
typedef struct {
  unsigned char HTCaps;		/* HT caps. */
  ieee1905_vht_caps_type VHTCaps;	/* VHT caps */
  ieee1905_radio_caps_type RadioCaps;	/* Radio caps */
  ieee1905_he_caps_type HECaps;		/* HE caps */
} ieee1905_ap_caps_type;

typedef struct {
  struct timespec queried;  /* Time at which the link metric calculated */
  unsigned int delta; /* The time delta in ms between queried and report was sent */
  unsigned int downlink_rate; /* Estimated MAC Data Rate in downlink (in Mb/s) */
  unsigned int uplink_rate; /* Estimated MAC Data Rate in upnlink (in Mb/s) */
  unsigned char rcpi; /* Measured uplink RSSI for STA in RCPI */
} ieee1905_sta_link_metric;

typedef struct {
  unsigned int bytes_sent;  /* number of bytes sent to the associated STA */
  unsigned int bytes_recv;  /* number of bytes received from the associated STA */
  unsigned int packets_sent;  /* number of packets successfully sent to the associated STA */
  unsigned int packets_recv;  /* number of packets received from the associated STA */
  unsigned int tx_packet_err;  /* number of packets which could not be transmitted */
  unsigned int rx_packet_err;  /* number of packets which were received in error form */
  unsigned int retransmission_count;  /* number of packets sent with the retry flag set */
} ieee1905_sta_traffic_stats;

typedef struct {
  unsigned char regclass;
  unsigned char count;
  unsigned char channel[IEEE1905_MAX_RCCHANNELS];
  unsigned char pref;
  unsigned char reason;
} ieee1905_chan_pref_rc_map;

typedef struct {
  uint8 rc_count;
  ieee1905_chan_pref_rc_map *rc_map;
} ieee1905_chan_pref_rc_map_array;

/* Interfcae Metric */
typedef struct {
  unsigned char chan_util;  /* Channel Utilization */
} ieee1905_interface_metric;

/* Transmitter and reciever link metric */
typedef struct {
  struct timespec queried; /* Last queried time */
  unsigned int txPacketErrors; /* Estimated number of lost packets during the measurement period */
  unsigned int transmittedPackets; /* Estimated number of packets transmitted */
  unsigned short macThroughPutCapacity; /* The maximum MAC throughput in Mbps */
  unsigned short linkAvailability; /* average percentage of time that the link is available for
                                    * data transmission
                                    */
  unsigned short phyRate; /* PHY rate in Mbps */
  unsigned int receivedPackets; /* Number of packets received at the interface */
  unsigned int rxPacketErrors; /* Estimated number of lost packets during the measurement period */
  unsigned char rcpi; /* estimated RSSI in dB in RCPI */
  unsigned int prevRxBytes;
  unsigned int latestRxBytes;
} ieee1905_backhaul_link_metric;

typedef struct {
  unsigned int txPacketErrors; /* Estimated number of lost packets during the measurement period */
  unsigned int transmittedPackets; /* Estimated number of packets transmitted */
  unsigned int receivedPackets; /* Number of packets received at the interface */
  unsigned int rxPacketErrors; /* Estimated number of lost packets during the measurement period */
} ieee1905_old_backhaul_link_counter;

typedef struct {
  i5_ll_listitem  ll;
  unsigned char   mac[MAC_ADDR_LEN];
  struct timeval  assoc_tm;
  unsigned char *assoc_frame;
  unsigned short assoc_frame_len;
  ieee1905_sta_traffic_stats traffic_stats; /* Associated STA Traffic Stats */
  ieee1905_sta_traffic_stats old_traffic_stats; /* Old Associated STA Traffic Stats */
  ieee1905_sta_link_metric link_metric; /* Associated STA Link Metrics */

  void *vndr_data;    /* vendor specific data pointer which can be filled by the user */
  unsigned char flags;  /* Flags of type I5_CLIENT_FLAG_XXX */
} i5_dm_clients_type;

typedef struct {
  uint8 channel;
  uint8 count;
  uint8 center_chan_80mhz;
  uint8 center_chan_160mhz;
  uint8 regclass[I5_MAX_CHANRCS];
} i5_dm_chan_rc_map_type;

typedef struct {
  uint8 regclass;
  uint8 count;
  uint8 channel[IEEE1905_80211_CHAN_PER_REGCLASS];
} i5_dm_rc_chan_map_type;

typedef struct {
  i5_ll_listitem      ll;
  char ifname[I5_MAX_IFNAME];
  unsigned char mapFlags; /* Of Type IEEE1905_MAP_FLAG_XXX */
  unsigned char flags;    /* Flags of type I5_BSS_FLAG_XXX */
  unsigned char       BSSID[MAC_ADDR_LEN];
  ieee1905_ssid_type  ssid;
  unsigned short      ClientsNumberOfEntries;
  i5_dm_clients_type  client_list; /* List of client's associated to this BSS */
  ieee1905_ap_metric  APMetric; /* Ap Metric */

  void *vndr_data;    /* vendor specific data pointer which can be filled by the user */
} i5_dm_bss_type;

#endif /* MULTIAP */

typedef struct {
  i5_ll_listitem ll;
  unsigned char  state;
  unsigned char  InterfaceId[MAC_ADDR_LEN];
  char  ifname[I5_MAX_IFNAME];
  unsigned int   Status;
  unsigned int   SecurityStatus;
  unsigned int   LastChanged;
  unsigned short MediaType;       // 0xffff indicates a Generic Phy Device
  unsigned char  netTechOui[I5_PHY_INTERFACE_NETTECHOUI_SIZE];
  unsigned char  netTechVariant;
  unsigned char  netTechName[I5_PHY_INTERFACE_NETTECHNAME_SIZE];
  unsigned char  url[I5_PHY_INTERFACE_URL_MAX_SIZE];
  unsigned char  MediaSpecificInfo[I5_MEDIA_SPECIFIC_INFO_MAX_SIZE];
  unsigned int   MediaSpecificInfoSize;
  i5MacAddressDeliveryFunc i5MacAddrDeliver;
#ifdef MULTIAP
  unsigned int   BSSNumberOfEntries;
  chanspec_t chanspec;
  unsigned char opClass;
  unsigned char band; /* Of type BAND_XXX. For dual band it will have both 5GL and 5GH */
#endif /* MULTIAP */
  union {
    struct {
      char          wlParentName[I5_MAX_IFNAME];
      unsigned char isRenewPending;
      unsigned char confStatus;
      unsigned char credChanged;
      unsigned char isConfigured; /* enrollee only */
    };
  };
#ifdef MULTIAP
  i5_dm_bss_type           bss_list; /* List of BSS's operating in this interface */
  ieee1905_chan_pref_rc_map_array ChanPrefs;
  unsigned char	      TxPowerLimit;
  unsigned int        msg_stat_flag;
  ieee1905_ap_caps_type     ApCaps;   /* AP capabilities. */
  ieee1905_interface_metric ifrMetric;  /* Interface Metric */
  unsigned char bssid[MAC_ADDR_LEN];  /* BSSID in case of IEEE1905_MAP_FLAG_STA */
  unsigned char mapFlags; /* Of Type IEEE1905_MAP_FLAG_XXX */
  unsigned char flags;	/* Of Type I5_FLAG_IFR_XXX */
#endif /* MULTIAP */
  void *vndr_data;    /* vendor specific data pointer which can be filled by the user */
} i5_dm_interface_type;

typedef struct {
  i5_ll_listitem ll;
  unsigned char  state;
  unsigned char  LocalInterfaceId[MAC_ADDR_LEN];
  unsigned char  NeighborInterfaceId[MAC_ADDR_LEN];
} i5_dm_legacy_neighbor_type;

typedef struct {
  i5_ll_listitem   ll;
  unsigned char    state;
  unsigned char    LocalInterfaceId[MAC_ADDR_LEN];
  unsigned char    Ieee1905Id[MAC_ADDR_LEN];
  unsigned char    NeighborInterfaceId[MAC_ADDR_LEN];
  unsigned char    IntermediateLegacyBridge;
  unsigned short   MacThroughputCapacity;             /* in Mbit/s */
  unsigned short   availableThroughputCapacity;       /* in Mbit/s */
#ifdef MULTIAP
  ieee1905_backhaul_link_metric metric; /* Backhaul Link Metrics for neighbor */
  ieee1905_old_backhaul_link_counter old_metric; /* Old backhaul metric counters */
#endif /* MULTIAP */
  unsigned int     prevRxBytes;
  unsigned int     latestRxBytes;
  unsigned char    ignoreLinkMetricsCountdown;
  char             localIfname[I5_MAX_IFNAME];
  unsigned int     localIfindex;
  void            *bridgeDiscoveryTimer;

  void *vndr_data;    /* vendor specific data pointer which can be filled by the user */
} i5_dm_1905_neighbor_type;

typedef struct {
  i5_ll_listitem ll;
  unsigned char  state;
  char           ifname[I5_MAX_IFNAME];
  unsigned int   forwardingInterfaceListNumEntries;
  unsigned char  ForwardingInterfaceList[FWD_IF_LIST_LEN];
} i5_dm_bridging_tuple_info_type;

typedef struct i5_dm_device_type_t_{
  i5_ll_listitem ll;
  unsigned char  state;
  unsigned char  queryState;
  unsigned char  validated;
  unsigned char  numTopQueryFailures;
  unsigned char  nodeVersion;
  void          *nodeVersionTimer;
  unsigned int   hasChanged;
  void          *watchdogTimer;
  unsigned int   Version;
  unsigned char  DeviceId[MAC_ADDR_LEN];
  unsigned int   Status;
  unsigned int   LastChanged;
  unsigned int   InterfaceNumberOfEntries;
  unsigned int   LegacyNeighborNumberOfEntries;
  unsigned int   Ieee1905NeighborNumberOfEntries;
  unsigned int   BridgingTuplesNumberOfEntries;
  void          *psock;  /* Store the psock information on which the packet is recieved for this */
  unsigned int   flags;	/* Flags of type I5_CONFIG_FLAG_XXX */
#ifdef MULTIAP
  void          *steerOpportunityTimer; /* Timer to be created after sending steering
                                           * opportunity request for a device
                                           */
  unsigned char BasicCaps;	/* AP Basic caps. of Type IEEE1905_AP_CAPS_FLAGS_XXXX */
  unsigned short macThroughPutCapacity; /* MAC Throughput Capacity from this to the parent device */
#endif /* MULTIAP */
  char           friendlyName[I5_DEVICE_FRIENDLY_NAME_LEN];

  void *vndr_data;    /* vendor specific data pointer which can be filled by the user */

  i5_dm_interface_type           interface_list;
  i5_dm_legacy_neighbor_type     legacy_list;
  i5_dm_1905_neighbor_type       neighbor1905_list;
  i5_dm_bridging_tuple_info_type bridging_tuple_list;
  struct i5_dm_device_type_t_ *parentDevice;  /* Pointer to the parent device */
  time_t active_time;
} i5_dm_device_type;

typedef struct {
  void            *dmLinkMetricTimer;
  char            dmLinkMetricIntervalValid;
  unsigned int    dmLinkMetricIntervalMsec;
  void            *dmLinkMetricActivatedTimer;
} i5_dm_link_metric_autoquery_type;

typedef struct {
  i5_dm_device_type device_list;
  i5_dm_device_type *selfDevice; /* Pointer to the self device for easy access */
  unsigned int   DevicesNumberOfEntries;
  unsigned char  updateStpNeeded;
  i5_dm_link_metric_autoquery_type linkMetricAuto;
  void          *pLinkMetricTimer;
  void          *pPerAPLinkMetricTimer;
} i5_dm_network_topology_type;

#endif // endif
