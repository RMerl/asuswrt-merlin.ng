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

#ifndef _I5API_H_
#define _I5API_H_

#include "ieee1905_defines.h"

#define I5_MESSAGE_BACKLOG 3
#define I5_SSID_MAX_LENGTH 32
#define I5_PASSWORD_MAX_LENGTH 32
#define I5_CMD_MAX_BUF_SIZE 128

short i5_controller_port;

/* ieee1905 api messages */
typedef enum t_i5_api_cmd_name
{
  I5_API_CMD_NONE = 0,
  I5_API_CMD_RETRIEVE_DM,
  I5_API_CMD_TRACE,
  I5_API_CMD_WLCFG,
  I5_API_CMD_PLC,
  I5_API_CMD_FLOWSHOW,
  I5_API_CMD_STOP,
  I5_API_CMD_START,
  I5_API_CMD_LINKUPDATE,
  I5_API_CMD_JSON_LEG,
  I5_API_CMD_PUSH_BUTTON,
  I5_API_CMD_SHOW_AL_MAC,
  I5_API_CMD_SEND_MESSAGE,
  I5_API_CMD_SET_LQ_INTERVAL,
  I5_API_CMD_SEND_BYTES,
  I5_API_CMD_SET_WIFI_PASS_SSID,
  I5_API_CMD_SET_WIFI_OVERRIDE_BW,
  I5_API_CMD_SET_WIFI_RELEASE_BW,
  I5_API_CMD_SET_WIFI_BOUNCE_BW,
  I5_API_CMD_SET_PLC_PASS_NMK,
  I5_API_CMD_SET_PLC_OVERRIDE_BW,
  I5_API_CMD_SET_PLC_RELEASE_BW,
  I5_API_CMD_SET_PLC_BOUNCE_BW,
  I5_API_CMD_SET_MOCA_PASS,
  I5_API_CMD_SHOW_MSGS,
  I5_API_CMD_SHOW_SOCKETS,
  I5_API_CMD_TRACE_TIME,
  I5_API_CMD_GET_CONFIG,
  I5_API_CMD_SET_CONFIG,
#ifdef MULTIAP
  I5_API_CMD_CLIENT_CAP,
  I5_API_CMD_SEND_HL_DATA,
  I5_API_CMD_SEND_STEER,
  I5_API_CMD_LIST_OPERATION,
  I5_API_CMD_STEER_CONFIG,
  I5_API_CMD_ASSOC_CNTRL,
  I5_API_CMD_SEND_FILE,
  I5_API_CMD_WPS,
  I5_API_CMD_RENEW_CONFIG,
  I5_API_CMD_SEND_BH_STEER,
  I5_API_CMD_METRIC_CONFIG,
  I5_API_CMD_SEND_AP_METRIC_QUERY,
  I5_API_CMD_SEND_ASSOC_STA_LINK_METRIC_QUERY,
  I5_API_CMD_SEND_UNASSOC_STA_LINK_METRIC_QUERY,
  I5_API_CMD_SEND_BEACON_METRIC_QUERY,
  I5_API_CMD_HLE_SEND_HL_DATA = 44, /* Do not modify this number as it is being used by customer. */
  I5_API_CMD_RETRIEVE_DM_EXT,
#endif /* MULTIAP */
} t_I5_API_CMD_NAME;

typedef struct t_i5_api_msg
{
  t_I5_API_CMD_NAME  cmd;
  unsigned int       len;
} t_I5_API_MSG;

typedef struct t_i5_api_trace_msg
{
  int                module_id;
  int                depth;
  unsigned int       ifindex;
  unsigned char      interfaceMac[6];
} t_I5_API_TRACE_MSG;

typedef struct t_i5_api_wlcfg_msg
{
  char               ifname[I5_MAX_IFNAME];
  int                subcmd;
  int                status;
} t_I5_API_WLCFG_MSG;

typedef enum t_i5_api_plc_subcmd
{
  I5_API_PLC_UKE_START     = 0,
  I5_API_PLC_UKE_RANDOMIZE,
} t_I5_API_PLC_SUBCMD;

typedef struct t_i5_api_plc_msg
{
  t_I5_API_PLC_SUBCMD subcmd;
} t_I5_API_PLC_MSG;

typedef enum t_i5_api_json_leg_subcmd
{
  I5_API_JSON_LEG_OFF     = 0,
  I5_API_JSON_LEG_ON,
} t_I5_API_JSON_LEG_SUBCMD;

typedef struct t_i5_api_json_leg_msg
{
  t_I5_API_JSON_LEG_SUBCMD subcmd;
} t_I5_API_JSON_LEG_MSG;

typedef struct t_i5_api_golden_node_send_msg
{
  unsigned char       macAddr[6];
  unsigned int        messageId;
} t_I5_API_GOLDEN_NODE_SEND_MSG;

typedef struct t_i5_api_link_metric_interval
{
  unsigned int        intervalMsec;
} t_I5_API_LINK_METRIC_INTERVAL;

typedef struct t_i5_api_golden_node_send_msg_bytes
{
  unsigned char       macAddr[6];
  unsigned char       message[1];
} t_I5_API_GOLDEN_NODE_SEND_MSG_BYTES;

typedef struct t_i5_api_password_ssid_msg
{
  unsigned char       password[I5_PASSWORD_MAX_LENGTH+1];
  unsigned char       ssid[I5_SSID_MAX_LENGTH+1];
  char                ifname[I5_MAX_IFNAME];
} t_I5_API_PASSWORD_SSID_MSG;

typedef struct t_i5_api_override_bw_msg
{
  unsigned char       overrideCount;
  unsigned int        availBwMbps;
  unsigned int        macThroughBwMbps;
} t_I5_API_OVERRIDE_BW_MSG;

typedef enum t_i5_api_config_subcmd
{
  I5_API_CONFIG_BASE     = 0,

  /* the following commands are applicable
     when DMP_DEVICE2_IEEE1905BASELINE_1 is defined */
  I5_API_CONFIG_SET_NETWORK_TOPOLOGY,
  I5_API_CONFIG_GET_LINK_METRICS,
} t_I5_API_CONFIG_SUBCMD;

typedef struct
{
    t_I5_API_CONFIG_SUBCMD  subcmd;
    unsigned int            isEnabled;
    char                    deviceFriendlyName[I5_DEVICE_FRIENDLY_NAME_LEN];
    unsigned int            isRegistrar;
    unsigned int            apFreqBand24En;
    unsigned int            apFreqBand5En;
} t_I5_API_CONFIG_BASE;

#if defined(BRCM_CMS_BUILD) && defined(DMP_DEVICE2_IEEE1905BASELINE_1)
typedef struct
{
    t_I5_API_CONFIG_SUBCMD  subcmd;
    unsigned int            isEnabled;
} t_I5_API_CONFIG_SET_NETWORK_TOPOLOGY;

typedef struct
{
    t_I5_API_CONFIG_SUBCMD  subcmd;
    unsigned char           ieee1905Id[6];
    unsigned char           remoteInterfaceId[6];
} t_I5_API_CONFIG_GET_LINK_METRICS;

typedef struct
{
    unsigned int    packetErrors;
    unsigned int    packetErrorsReceived;
    unsigned int    transmittedPackets;
    unsigned int    packetsReceived;
    unsigned int    MacThroughputCapacity;
    unsigned int    linkAvailability;
    unsigned int    phyRate;
    unsigned int    rcpi;
} t_I5_API_CONFIG_GET_LINK_METRICS_REPLY;
#endif // endif

#ifdef MULTIAP
typedef struct t_i5_api_client_cap_query
{
  t_I5_API_GOLDEN_NODE_SEND_MSG msgInfo;
  unsigned char                 BSSID[6];
  unsigned char                 clientMAC[6];
} t_I5_API_CLIENT_CAP_QUERY;

typedef struct t_i5_api_higher_layer_data
{
  t_I5_API_GOLDEN_NODE_SEND_MSG msgInfo;
  unsigned char                 protocol;
  unsigned int                  hdrLen; /* Length of ethernet + ieee1905 message hader */
  unsigned int                  data_len;
  unsigned char                 data[1];
} t_I5_API_HIGHER_LAYER_DATA;

typedef struct {
  t_I5_API_GOLDEN_NODE_SEND_MSG msgInfo;
  unsigned int                  data_len;
  unsigned char                 data[1];
} t_I5_API_SEND_MESSAGE;

typedef struct t_i5_api_steer
{
  t_I5_API_GOLDEN_NODE_SEND_MSG msgInfo;
  unsigned char                 bssid[6];
  unsigned char                 request_mode;
  unsigned char                 disassoc_imminent;
  unsigned char                 abridged;
  unsigned short                opportunity_window;
  unsigned short                disassoc_timer;
  unsigned char                 sta_mac[6];
  unsigned char                 trgt_bssid[6];
  unsigned char                 operating_class;
  unsigned char                 channel;
} t_I5_API_STEER;

typedef struct t_i5_api_list_opr
{
  unsigned int			operation;
  unsigned int			list_id;
  unsigned char			MAC[6];
} t_I5_LIST_OPR;

typedef struct t_i5_api_steer_policy_config
{
  unsigned int			operation;
  unsigned char			MAC[6];
  unsigned char			steer_policy;
  unsigned char			bss_load_thld;
  char				rssi_thld;
} t_I5_STEER_POLICY_CONFIG;

typedef struct t_i5_api_assoc_cntrl
{
  t_I5_API_GOLDEN_NODE_SEND_MSG msgInfo;
  unsigned char                 bssid[6];
  unsigned char                 assocCntrl;
  unsigned short                validity;
  unsigned char                 sta_mac[6];
} t_I5_API_ASSOC_CNTRL;

typedef struct t_i5_api_file
{
  t_I5_API_GOLDEN_NODE_SEND_MSG msgInfo;
  unsigned int                  data_len;
  unsigned char                 data[1];
} t_I5_API_FILE;

typedef struct t_i5_api_wps_cntrl
{
  char				rfband[32];
  int				mode;
} t_I5_API_WPS_CNTRL;

typedef struct t_i5_api_renew_ap_config
{
  t_I5_API_GOLDEN_NODE_SEND_MSG msgInfo;
  int				rfband;
  int				role;
} t_I5_API_RENEW_AP_CONFIG;

typedef struct t_i5_api_bh_steer
{
  t_I5_API_GOLDEN_NODE_SEND_MSG msgInfo;
  unsigned char                 bh_sta_mac[6];
  unsigned char                 trgt_bssid[6];
  unsigned char                 opclass;
  unsigned char                 channel;
} t_I5_API_BH_STEER;

typedef struct t_i5_api_metric_rpt_policy_config
{
  unsigned int			operation;
  unsigned char			ap_rpt_intvl;
  unsigned char			MAC[6];
  unsigned char			sta_mtrc_rssi_thld;
  unsigned char			sta_mtrc_rssi_hyst;
  unsigned char			ap_mtrc_chan_util;
  unsigned char			sta_mtrc_policy_flag;
} t_I5_METRIC_RPT_POLICY_CONFIG;

typedef struct t_i5_api_ap_metric_query
{
  t_I5_API_GOLDEN_NODE_SEND_MSG msgInfo;
  unsigned char                 bssCount; /* Number of BSSIDs */
  unsigned char                 bssids[1]; /* each 6 octet BSSIDs will be in linear array */
} t_I5_API_AP_METRIC_QUERY;

typedef struct t_i5_api_assoc_sta_link_metric_query
{
  t_I5_API_GOLDEN_NODE_SEND_MSG msgInfo;
  unsigned char                 clientMAC[6];
} t_I5_API_ASSOC_STA_LINK_METRIC_QUERY;

typedef struct t_i5_api_unassoc_sta_link_metric_query
{
  t_I5_API_GOLDEN_NODE_SEND_MSG msgInfo;
  unsigned char                 clientMAC[6];
  unsigned char                 opclass;
  unsigned char                 channel;
} t_I5_API_UNASSOC_STA_LINK_METRIC_QUERY;

typedef struct t_i5_api_beacon_metric_query
{
  t_I5_API_GOLDEN_NODE_SEND_MSG msgInfo;
  unsigned char                 sta_mac[6];
  unsigned char                 opclass;
  unsigned char                 channel;
  unsigned char                 bssid[6];
  unsigned char                 subelements_len;
  unsigned char                 subelements[1];
} t_I5_API_BEACON_METRIC_QUERY;
#endif /* MULTIAP */

/* create session to ieee1905 daemon -- return socket */
int i5apiOpen(void);

/* prepend header to user cmd/data and send message -- return amount sent */
int i5apiSendMessage(int sd, int cmd, void *data, size_t datalen);

/* wait until response ready */
int i5apiWait(int sd, int waitsec);

/* receive response -- returns -1 on error, else payload length */
int i5apiRecvResponse(int sd, void **data, size_t datalen);

/* close session to ieee1905 daemon */
int i5apiClose(int sd);

/* perform transaction -- return -1 on error, else length of reply */
int i5apiTransaction(int cmd, void *reqdata, size_t reqlen, void **repdata, size_t replen);

#endif /* _I5API_H_ */
