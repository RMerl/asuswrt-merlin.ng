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

#ifndef _IEEE1905_SOCKET_H_
#define _IEEE1905_SOCKET_H_

/*
 * IEEE1905 Socket
 */

#include <stdio.h>
#include <sys/socket.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <netinet/in.h>
#include <sys/un.h>
#include "ieee1905_defines.h"
#include "i5api.h"
#include "ieee1905_linkedlist.h"
#include "ieee1905_timer.h"
#include "ieee1905.h"
#if defined(USEBCMUSCHED)
#include "bcm_usched.h"
#endif /* USEBCMUSCHED */

#if (FD_SETSIZE < 64)
#define MAX_I5_SOCKETS FD_SETSIZE
#else
#define MAX_I5_SOCKETS 64
#endif // endif

#define MAX_PATH_NAME 64
#define MAX_FREQ_BANDS 3

/* IEEE1905 definition of bands */
#define I5_FREQ_BAND_2G 0
#define I5_FREQ_BAND_5G 1
#define I5_FREQ_BAND_60 2

enum {
  i5_socket_type_ll = 0,
  i5_socket_type_bridge_ll,
  i5_socket_type_in_listen,
  i5_socket_type_json,
  i5_socket_type_netlink,
  i5_socket_type_udp,
  i5_socket_type_stream,
  i5_socket_type_raweth
};

/* _i5_socket_type flags */
#define I5_SOCKET_FLAG_LOOPBACK   0x01  /* Socket is for loopback interface */

/* Helper MACRO for _i5_socket_type flags */
#define I5_SOCKET_IS_LOOPBACK(flags)  ((flags) & I5_SOCKET_FLAG_LOOPBACK)

/* Loopback IP address */
#define I5_SOCKET_LOOPBACK_IP	"127.0.0.1"

typedef struct _i5_socket_type {
  i5_ll_listitem ll;
  int sd;
  void (*process)(struct _i5_socket_type *psock);
  int type;
  unsigned char flags;  /* Bit flags of type I5_SOCKET_FLAG_XXX */
  timer_elem_type *ptmr;
  union {
    struct {
      struct sockaddr_ll      sa;
      unsigned char           mac_address[ETH_ALEN];
      char                    ifname[I5_MAX_IFNAME];
      unsigned int            options;
      void (*notify)(struct _i5_socket_type *psock, unsigned char oper_status);
      timer_elem_type        *pnltmr;
      unsigned int            discoveryRetryPeriod;
      struct _i5_socket_type *pMetricSock;
      struct _i5_socket_type *pLldpProtoSock;
      unsigned short          hystBwMbps;
      unsigned char           hystBandwidthDataValid;
      void                   *pInterfaceCtx;
    } sll;
    struct {
      struct sockaddr_in sa;
    } sinl;
    struct {
      FILE *fp;
      int isGet;
    } sinr;
    struct {
      struct sockaddr_nl sa;
    } snl;
    struct {
      struct sockaddr_un sa;
    } sun;
  } u;
} i5_socket_type;

typedef struct {
  i5_ll_listitem  ll;
  unsigned int    freqBand;
  char            ifname[I5_MAX_IFNAME];
  int             renew;
  int             expectedMsg;
  unsigned int    callCounter;
  unsigned int    expiryTime;
  char            rxIfname[I5_MAX_IFNAME];
  unsigned char   registrarMac[MAC_ADDR_LEN];
} apSearchEntry;

typedef struct  {
  timer_elem_type  *timer;
  apSearchEntry    *activeSearchEntry;
  apSearchEntry     searchEntryList;
} apSearchSendStruct;

typedef struct {
  timer_elem_type               *timer;
  unsigned int                   count;
} secStatusStruct;

typedef struct  {
  timer_elem_type    *ptmr;
  i5_socket_type     *psock;
  union {
    struct {
      int           macAddrSet;
      unsigned char plcDevMac[MAC_ADDR_LEN];
    };
  };
} controlSockStruct;

typedef struct i5_wps_wscKey_ {
    unsigned char *pub_key;
    unsigned short pub_len;
    unsigned char *priv_key;
    unsigned short priv_len;
} i5_wps_wscKey;

typedef struct {
  i5_ll_listitem ll;
  unsigned char *m2;
  unsigned int m2_len;
} i5_wsc_m2_type;

/* List of VLAN interfaces created */
typedef struct {
  dll_t node;                 /* self referencial (next,prev) pointers of type dll_t */
  char ifname[I5_MAX_IFNAME]; /* Interface Name for which the VLAN is created */
  bool hwStpDisabled;         /* HW STP disabled for this interface */
} i5_vlan_ifr_node;

typedef struct {
  unsigned int          flags;  /* Flags of type I5_CONFIG_FLAG_XXX */
  unsigned char         i5_mac_address[ETH_ALEN];
  char                  friendlyName[I5_DEVICE_FRIENDLY_NAME_LEN];
  unsigned char         socketRemovedFlag;
  i5_socket_type        i5_socket_list;
  int                   num_i5_sockets;
  unsigned short        last_message_identifier;
  controlSockStruct     plc_control_socket;
  controlSockStruct     wl_control_socket;
#ifndef MULTIAP
  apSearchSendStruct    apSearch;
#endif /* MULTIAP */
  char                  freqBandEnable[MAX_FREQ_BANDS];
  char                  jsonLegacyDisplayEnabled;
  secStatusStruct       ptmrSecStatus;
  unsigned int          deviceWatchdogTimeoutMsec;
  int                   running;
  int                   networkTopEnabled;
#ifdef MULTIAP
  unsigned char         isNewBssCreated;
  ieee1905_call_bks_t   cbs;  /* Callbacks needs to be called */
  ieee1905_policy_config policyConfig;	/* Config for different policies */
  unsigned char dwds_enabled;  /* Whether DWDS enabled or not */
  ieee1905_glist_t client_bssinfo_list; /* List of type ieee1905_client_bssinfo_type */
  unsigned char		*m1;
  int			m1_len;
  i5_wps_wscKey		*keys;
  i5_wsc_m2_type m2_list;
  unsigned char m2_count;
  timer_elem_type *ptmrApSearch;  /* Timer created for Auto Configuration Search */
  timer_elem_type *ptmrWSC; /* Timer created for AP Auto configuration M1/M2 */
  timer_elem_type *ptmrGetBSSID;  /* Timer for getting the bSTA's BSSID */
  timer_elem_type *ptmrRemoveStaleNeighbors;  /* Timer created for removing stale neighbors */
  unsigned char curWSCMac[MAC_ADDR_LEN]; /* Current wireless interface for which the M1 has sent */
  unsigned int discovery_timeout; /* Timeout for sending the topology/bridge discovery messages */
  unsigned char device_mode;  /* Tells whether device supports agent, controller or both. This
                               * is got from multiap_mode NVRAM. Use I5_CONFIG_FLAG_CONTROLLER
                               * and I5_CONFIG_FLAG_AGENT for checking
                               */
  unsigned short prim_vlan_id;    /* Primary VLAN ID */
  unsigned short sec_vlan_id;     /* Secondary VLAN ID */
  unsigned short vlan_ether_type; /* VLAN ether type */
  ieee1905_glist_t vlan_ifr_list; /* List of i5_vlan_ifr_node type objects */
#endif /* MULTIAP */
#if defined(USEBCMUSCHED)
  bcm_usched_handle	*usched_hdl;		/* Handle to Micro Scheduler Module */
#endif /* USEBCMUSCHED */

} i5_config_type;

extern i5_config_type i5_config;

i5_socket_type *i5SocketStreamNew(unsigned short port, void(* process)(i5_socket_type *));
void i5SocketDumpSockets(void);
i5_socket_type *i5SocketNew(int sd, int socket_type, void (*process)(i5_socket_type *));
void i5SocketNetlink(void);
int i5SocketClose(i5_socket_type *psock);
void i5SocketMain(void);
void i5SocketInit(int supServiceFlag);
void i5SocketDeinit(void);
i5_socket_type *i5SocketFindDevSocketByType( unsigned int socketType );
i5_socket_type *i5SocketFindDevSocketByIndex( unsigned int ifindex );
i5_socket_type *i5SocketFindDevSocketByName( const char *ifname );
i5_socket_type *i5SocketFindDevSocketByAddr( unsigned char *macAddr, i5_socket_type *pifstart );
i5_socket_type *i5SocketFindUdpSocketByProcess( void *process );

char *i5SocketGetIfName(i5_socket_type *psock);
int   i5SocketGetIfIndex(i5_socket_type *psock);
unsigned char *i5SocketGetAddress(i5_socket_type *psock);

/* Closes the socket */
void i5SocketCloseSockFD(int *sockfd);
/* Read "length" bytes of "data" from non-blocking socket */
unsigned int i5SocketRecvData(i5_socket_type *psock, unsigned char *data, unsigned int length);
/* Connects to the server given the IP address and port number */
int i5SocketConnectToServer(char* straddrs, unsigned int nport);
/* Sends the data to socket */
unsigned int i5SocketSendData(int sockfd, char *data, unsigned int len);

#endif // endif
