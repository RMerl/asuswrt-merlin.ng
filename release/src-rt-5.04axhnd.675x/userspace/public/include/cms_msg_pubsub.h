/***********************************************************************
 *
 * Copyright (c) 2019  Broadcom Corporation
 * All Rights Reserved
 *
 * <:label-BRCM:2019:DUAL/GPL:standard
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 * :>
 *
 ************************************************************************/

/*!\file cms_msg_pubsub.h
 * \brief This file contains the message bodies used in CMS Pub/Sub msg types.
 */

#ifndef CMS_MSG_PUBSUB_H
#define CMS_MSG_PUBSUB_H

#include "bdk.h"
#include "sysutil_net.h"  // for networking constants, build libsys_util first
#include "os_defs.h"      // for UBOOL8

/** PubSub events are divided into several types, which is indicated in the
  * wordData field of the CMS Message Header.
 */
typedef enum 
{
   PUBSUB_EVENT_KEY_VALUE=1,
   PUBSUB_EVENT_NAMESPACE=2,
   PUBSUB_EVENT_INTERFACE=3,
   PUBSUB_EVENT_MDM_OWNER=20,
} PubSubEventType;


/** Data body for CMS_MSG_SUBSCRIBE_EVENT, CMS_MSG_RECEIVE_EVENT,
 *  CMS_MSG_PUBLISH_EVENT, and CMS_MSG_GET_EVENT_STATUS messages when
 *  wordData == 1 (PUBSUB_EVENT_KEY_VALUE).
 */
#define PUBSUB_KEY_MAX_LEN  255

typedef struct
{
   char key[PUBSUB_KEY_MAX_LEN+1]; /**< Key can be any alpha numeric plus ., _, -, and :, case insensitive.  (never use = or *). */
   UINT32 valueLen;  /** Length of value string following this struct (including null termination). */
} PubSubKeyValueMsgBody; 

// Some commonly used keys in BDK system.
#define BDK_KEY_X_BROADCOM_COM_TM   "X_BROADCOM_COM_TM"
#define BDK_KEY_DEVICE_QOS_PARAMS   "Device.QoS.Params"

// Comma separated list of IPv4 and IPv6 static DNS servers.  If none are
// configured, then value will be empty string.
#define BDK_KEY_DEVICE_DNS_STATIC_SERVERS  "Device.DNS.StaticServers"

// Remote components publish an update to this key whenever the TCP Pure Ack
// queue config for the interface has changed (not including system startup).
// Key is prefix:intfName, e.g. TcpPureAck:ptm0
// Value is a timestamp
#define BDK_KEY_TCP_PURE_ACK_CONFIG_CHANGED_PREFIX  "TCP_PURE_ACK_CONFIG_CHANGED"

// WiFi component publishes an update to this key for devices association and
// dis-associattion events with a timeout.
// Key is prefix:<ifname> e.g. AP_CLIENT_ASSOC_CHANGED:wl0.1
// Value is a comma separated events which is formated as <id>:<op>:<MAC_ADDR>
// <id> is an unsigned integer of instance id, 0 means dis-associated device.
// <op> is operating command code defined in rut2_wifi.h
// <MAC_ADDR> is the MAC Address of associated/disassociated device.
// e.g. 1:1:00:22:44:55:66:26,1:2:00:22:44:55:66:26,...
#define BDK_KEY_AP_CLIENT_ASSOC_CHANGED_PREFIX "AP_CLIENT_ASSOC_CHANGED"

/** Data body for CMS_MSG_SUBSCRIBE_EVENT, CMS_MSG_RECEIVE_EVENT,
 *  CMS_MSG_PUBLISH_EVENT, and CMS_MSG_GET_EVENT_STATUS messages when
 *  wordData == 2 (PUBSUB_EVENT_NAMESPACE).
 */
#define PUBSUB_NAMESPACE_MAX_LEN  255

typedef struct
{
   char namespc[PUBSUB_NAMESPACE_MAX_LEN+1]; /**< E.g. Device.Voice. */
   char ownerCompName[BDK_COMP_NAME_LEN+1];  /**< component which owns this namespace. */
} PubSubNamespaceMsgBody; 


/** Data body for CMS_MSG_SUBSCRIBE_EVENT, CMS_MSG_RECEIVE_EVENT,
 *  CMS_MSG_PUBLISH_EVENT, and CMS_MSG_GET_EVENT_STATUS messages when
 *  wordData == 3 (PUBSUB_EVENT_INTERFACE).
 */
typedef struct
{
  char intfName[SYSUTL_IFNAME_LEN];  /**< interface name or ANY_WAN */
  UBOOL8 isUpstream;  /**< upStream is the more general way to say WAN */
  UBOOL8 isLayer3;    /**< is this a layer 3 interface (where does PPP fall into?  Do we need another category?) */
  UBOOL8 isLayer2;    /**< is this a layer 2 interface */
  UBOOL8 isIpv4Enabled; /**< For layer 3 interface, is IPv4 enabled? */
  UBOOL8 isIpv4Up;      /**< for layer 3 interface, is IPv4 up? */
  UBOOL8 isIpv6Enabled; /**< For layer 3 interface, is IPv6 enabled? */
  UBOOL8 isIpv6Up;      /**< for layer 3 interface, is IPv6 up? (IPv6 may have more complicted notion of up link-local/wan) */
  UBOOL8 isLinkUp;    /**< for layer 2 interface, is link up? */
  char ipv4Addr[SYSUTL_IPV4_ADDR_LEN]; /**< CIDR notation, e.g. 192.168.100.201 */
  char ipv4Netmask[SYSUTL_IPV4_ADDR_LEN]; /**< CIDR notation, e.g. 255.255.255.0 */
  char ipv4Gateway[SYSUTL_IPV4_ADDR_LEN]; /**< CIDR notation, e.g. 192.168.100.1 */
  char ipv6GlobalAddr[SYSUTL_IPV6_ADDR_LEN]; /**< DCIDR notation, e.g. 2002::5/64 */
  char ipv6NextHop[SYSUTL_IPV6_ADDR_LEN];  /**< next-hop or default route in IPv6 */
  char dnsServers[SYSUTL_IPVX_ADDR_LEN*4]; /**< Up to 4 DNS servers received on this connection separated by commas. Servers could have come from ipv4 or v6 connection. */
  char childrenIntfNames[512]; /**< special case if intfName is brx: comma separated list of interfaces under this bridge. */
  char wlBrName[SYSUTL_IFNAME_LEN];  /**<  for wifi to request to be put under this bridge */
  char pubCompName[BDK_COMP_NAME_LEN+1];  /**< component name of publisher. */
  char fullpath[256];  /**< TR181 fullpath of this interface (layer 2 interfaces in remote components must set this, optional for others). */
  char tags[256];  /**< tags associated with this intf, e.g. Default_LAN, Default_WAN */
  UINT32 additionalDataLen;  /**< length of optional, additional data (e.g. dhcpOptions) following this struct; must be a string and include null termination. 0 if none. */
} PubSubInterfaceMsgBody;

#define PUBSUB_KEY_MDM_NOTIFICATION_PREFIX       "MDM_ACTIVE_NOTIFICATION"
#define PUBSUB_KEY_MDM_ALT_NOTIFICATION_PREFIX   "MDM_ALT_NOTIFICATION"

#define PUBSUB_KEY_ATM_OAM_DIAG_COMPLETE    "ATM_OAM_DIAG_COMPLETE"
#define PUBSUB_KEY_DSL_LOOP_DIAG_COMPLETE   "DSL_LOOP_DIAG_COMPLETE"
#define PUBSUB_KEY_DSL_SELT_DIAG_COMPLETE   "DSL_SELT_DIAG_COMPLETE"
#define PUBSUB_KEY_PING_DIAG_COMPLETE       "PING_DIAG_COMPLETE"
#define PUBSUB_KEY_TRACERT_DIAG_COMPLETE    "TRACERT_DIAG_COMPLETE"
#define PUBSUB_KEY_UPLOAD_DIAG_COMPLETE     "UPLOAD_DIAG_COMPLETE"
#define PUBSUB_KEY_DOWNLOAD_DIAG_COMPLETE   "DOWNLOAD_DIAG_COMPLETE"
#define PUBSUB_KEY_SPDSVC_DIAG_EVENT        "SPDSVC_DIAG_EVENT"
#define PUBSUB_KEY_SPDSVC_DIAG_COMPLETE     "SPDSVC_DIAG_COMPLETE"


#define PUBSUB_INTF_TAG_DEFAULT_LAN         "Default_LAN"
#define PUBSUB_INTF_TAG_DEFAULT_WAN         "Default_WAN"



#endif /* CMS_MSG_PUBSUB_H */
