/*
 * ***************************************************************************
 * *  Mediatek Inc.
 * * 4F, No. 2 Technology 5th Rd.
 * * Science-based Industrial Park
 * * Hsin-chu, Taiwan, R.O.C.
 * *
 * * (c) Copyright 2002-2018, Mediatek, Inc.
 * *
 * * All rights reserved. Mediatek's source code is an unpublished work and the
 * * use of a copyright notice does not imply otherwise. This source code
 * * contains confidential trade secret material of Ralink Tech. Any attemp
 * * or participation in deciphering, decoding, reverse engineering or in any
 * * way altering the source code is stricitly prohibited, unless the prior
 * * written consent of Mediatek, Inc. is obtained.
 * ***************************************************************************
 *
 *  Module Name:
 *  1905 interface
 *
 *  Abstract:
 *  1905 interface
 *
 *  Revision History:
 *  Who         When          What
 *  --------    ----------    -----------------------------------------
 *  Kapil.Gupta 2018/05/02     First implementation of the 1905 interface Module
 * */

#define MIN_TLVS_LENGTH    38   /*46 - 8(cmdu header size) */
#define MAX_TLVS_LENGTH    1492     /*1500 -8(cmdu header size)*/
#define CMDU_HLEN          8

/*p1905.1 message version*/
#define MESSAGE_VERSION 0


/*p1905.1 message type*/
#define TOPOLOGY_DISCOVERY      0x0000
#define TOPOLOGY_NOTIFICATION   0x0001
#define TOPOLOGY_QUERY          0x0002
#define TOPOLOGY_RESPONSE       0x0003
#define VENDOR_SPECIFIC         0x0004
#define LINK_METRICS_QUERY      0x0005
#define LINK_METRICS_RESPONSE   0x0006
#define AP_AUTOCONFIG_SEARCH    0x0007
#define AP_AUTOCONFIG_RESPONSE  0x0008
#define AP_AUTOCONFIG_WSC       0x0009
#define AP_AUTOCONFIG_RENEW     0x000A
#define P1905_PB_EVENT_NOTIFY   0x000B
#define P1905_PB_JOIN_NOTIFY    0x000C

#define SCLIENT_CAPABILITY_QUERY 0x000D
#define ICHANNLE_SELECTION_REQUEST 0x000E
#define Ack_1905 0x8000
#define AP_CAPABILITY_QUERY 0x8001
#define AP_CAPABILITY_REPORT 0x8002
#define MAP_POLICY_CONFIG_REQUEST 0x8003
#define CHANNLE_PREFERENCE_QUERY 0x8004
#define CHANNLE_PREFERENCE_REPORT 0x8005
#define CHANNLE_SELECTION_REQUEST 0x8006
#define CHANNLE_SELECTION_RESPONSE 0x8007
#define OPERATING_CHANNEL_REPORT 0x8008
#define CLIENT_CAPABILITY_QUERY 0x8009
#define CLIENT_CAPABILITY_REPORT 0x800A
#define AP_LINK_METRICS_QUERY 0x800B
#define AP_LINK_METRICS_RESPONSE 0x800C
#define ASSOC_STA_LINK_METRICS_QUERY 0x800D
#define ASSOC_STA_LINK_METRICS_RESPONSE 0x800E
#define UNASSOC_STA_LINK_METRICS_QUERY 0x800F
#define UNASSOC_STA_LINK_METRICS_RESPONSE 0x8010
#define BEACON_METRICS_QUERY 0x8011
#define BEACON_METRICS_RESPONSE 0x8012
#define COMBINED_INFRASTRUCTURE_METRICS 0x8013
#define CLIENT_STEERING_REQUEST 0x8014
#define CLIENT_STEERING_BTM_REPORT 0x8015
#define CLIENT_ASSOC_CONTROL_REQUEST 0x8016
#define CLIENT_STEERING_COMPLETED 0x8017
#define BACKHAUL_STEERING_REQUEST 0x8019
#define BACKHAUL_STEERING_RESPONSE 0x801A

#define DEV_SEND_1905_REQUEST 0x9000

typedef struct
{
    unsigned char message_version;
    unsigned char reserved_field_0;
    __be16 message_type;
    __be16 message_id;
    unsigned char fragment_id;
    unsigned char reserve_field_1:6;
    unsigned char relay_indicator:1;
    unsigned char last_fragment_indicator:1;
} __attribute__ ((__packed__)) cmdu_message_header;

int _1905_open_connection(const char *ctrl_path, struct mapd_global *global);
#ifdef SUPPORT_MULTI_AP
int _1905_poll_devices_in_network(struct own_1905_device *dev, struct bh_link_entry *bh_entry);
#endif
int _1905_close_connection(struct mapd_global *global);
int _1905_if_send_ap_metric_rsp(struct own_1905_device *own_dev
#if defined(MAP_R2) || defined(CENT_STR)
, unsigned char periodic
#endif
);

