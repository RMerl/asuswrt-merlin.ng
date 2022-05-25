/***********************************************************************
 *
 * Copyright (c) 2006-2007  Broadcom Corporation
 * All Rights Reserved
 *
 * <:label-BRCM:2006-2007:DUAL/GPL:standard
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 *
 ************************************************************************/

#ifndef __CMS_MSG_H__
#define __CMS_MSG_H__

#include "cms.h"
#include "cms_eid.h"
#include "mdm_params.h"
#include "omci_msg.h"


/*!\file cms_msg.h
 * \brief Public header file for messaging.
 * Code which need to handle messages must include this file.
 *
 * Here is a general description of how to use this interface.
 *
 * Early in application startup code, call cmsMsg_init() with a pointer
 * to void *.
 *
 * To receive a message, call cmsMsg_receive().  This returns a
 * pointer to a buffer that has a CmsMsgHeader at the beginning
 * and optional data after the header.  Free this buffer when you
 * are done with it by calling cmsMsg_free().
 *
 * To send a message, allocate a buffer big enough to hold a
 * CmsMsgHeader and any optional data you need to send with the
 * message.  Fill in the buffer (header and data portion), and
 * call cmsMsg_send().
 *
 * Before the application exits, call cmsMsg_cleanup().
 */


/*!\enum CmsMsgType
 * \brief  Enumeration of possible message types
 *
 * This section lists the CMS Message Ranges:
 * Broadcom Reserved Message Range                   0x10000250-0x1fffffff
 * -- system event messages                          0x10000250-0x100007ff
 * -- system request/response messages               0x10000800-0x10000fff
 * -- EPON event messages                            0x10001f00-0x10001f1f
 * -- EPON request/response messages                 0x10001f20-0x10001fff
 * -- Voice event messages                           0x10002000-0x100020ff
 * -- Voice request/response messages                0x10002100-0x100021ff
 * -- GPON OMCI request/response messages            0x10002200-0x100022ff
 * -- Modular Software messages                      0x10002500-0x100025ff
 * -- Publish/Subscribe messages                     0x10002700-0x1000273f
 * -- XMPP related messages                          0x10002800-0x100028FF
 * -- CellularApp related messages                   0x10003000-0x10003010
 *
 * Customer Reserved Message Range                   0x20000000-0x2fffffff
 *
 * All other message values are reserved for future use.
 *
 * Note that a message type does not specify whether it is a
 * request or response, that is indicated via the flags field of
 * CmsMsgHeader.
 */
typedef enum 
{
   CMS_MSG_SYSTEM_BOOT                          = 0x10000250, /**< system has booted, delivered to apps
                                                                 *   EIF_LAUNCH_ON_STARTUP set in their
                                                                 *   CmsEntityInfo.flags structure.
                                                                 */
   CMS_MSG_APP_LAUNCHED                         = 0x10000251, /**< Used by apps to confirm that launch succeeded.
                                                                 *   Sent from app to smd in cmsMsg_init.
                                                                 */
   CMS_MSG_WAN_LINK_UP                          = 0x10000252, /**< wan link is up (includes dsl, ethernet, etc) */
   CMS_MSG_WAN_LINK_DOWN                        = 0x10000253, /**< wan link is down */
   CMS_MSG_WAN_CONNECTION_UP                    = 0x10000254, /**< WAN connection is up (got IP address) */
   CMS_MSG_WAN_CONNECTION_DOWN                  = 0x10000255, /**< WAN connection is down (lost IP address) */
   CMS_MSG_ETH_LINK_UP                          = 0x10000256, /**< eth link is up (only if eth is used as LAN interface) */
   CMS_MSG_ETH_LINK_DOWN                        = 0x10000257, /**< eth link is down (only if eth is used as LAN interface) */
   CMS_MSG_USB_LINK_UP                          = 0x10000258, /**< usb link is up (only if eth is used as LAN interface) */
   CMS_MSG_USB_LINK_DOWN                        = 0x10000259, /**< usb link is down (only if eth is used as LAN interface) */
   CMS_MSG_ACS_CONFIG_CHANGED                   = 0x1000025A, /**< ACS configuration has changed. */
   CMS_MSG_TR69C_CONFIG_CHANGED                 = 0x1000025B, /**< to update loggingSOAP and connection request authentication */
   CMS_MSG_DELAYED_MSG                          = 0x1000025C, /**< This message is delivered to when delayed msg timer expires. */
   CMS_MSG_TR69_ACTIVE_NOTIFICATION             = 0x1000025D, /**< This message is sent to tr69c when one or more
                                                       *   parameters with active notification attribute
                                                       *   has had their value changed.
                                                       */
   CMS_MSG_WAN_ERRORSAMPLES_AVAILABLE           = 0x1000025E,/**< WAN connection has vectoring error samples available */
   CMS_MSG_WAN_ERRORSAMPLES_AVAILABLE_LINE1     = 0x1000025F,/**< WAN connection has vectoring error samples available for line 1, keep CMS_MSG_WAN_ERRORSAMPLES_AVAILABLE+1*/
   CMS_MSG_STUN_CONFIG_CHANGED                  = 0x10000260, /**< STUN configuration has changed. */
   CMS_MSG_GET_SHMID                            = 0x10000261, /**< Used by apps to get shmid from smd. */
   CMS_MSG_SHMID                                = 0x10000262, /**< Sent from ssk to smd when shmid is obtained. */
   CMS_MSG_MDM_INITIALIZED                      = 0x10000263, /**< Sent from ssk to smd when MDM has been initialized. */
   CMS_MSG_DHCPC_STATE_CHANGED                  = 0x10000264, /**< Sent from dhcp client when state changes, see also DhcpcStateChangeMsgBody */
   CMS_MSG_PPPOE_STATE_CHANGED                  = 0x10000265, /**< Sent from pppoe when state changes, see also PppoeStateChangeMsgBody */
   CMS_MSG_DHCP6C_STATE_CHANGED                 = 0x10000266, /**< Sent from dhcpv6 client when state changes, see also Dhcp6cStateChangeMsgBody */
   CMS_MSG_PING_STATE_CHANGED                   = 0x10000267, /**< Ping state changed (completed, or stopped) */
   CMS_MSG_DHCPD_RELOAD		                    = 0x10000268, /**< Sent to dhcpd to force it reload config file without restart */
   CMS_MSG_DHCPD_DENY_VENDOR_ID	                = 0x10000269, /**< Sent from dhcpd to notify a denied request with some vendor ID */
   CMS_MSG_DHCPD_HOST_INFO                      = 0x1000026A, /**< Sent from dhcpd to ssk to inform of lan host add/delete */
   CMS_MSG_TRACERT_STATE_CHANGED                = 0x1000026B, /**< Traceroute state changed (completed, or stopped) */
   CMS_MSG_MDM_INIT_DONE                        = 0x1000026C, /**< Sent from mdm_init to ssk at end of mdm_init. */
   CMS_MSG_DNSPROXY_RELOAD	                    = 0x10000270, /**< Sent to dnsproxy to force it reload config file without restart */
   CMS_MSG_TIME_STATE_CHANGED 	                = 0x10000271, /**< TIME state changed */
   CMS_MSG_DNSPROXY_IPV6_CHANGED                = 0x10000272, /**< Sent to dnsproxy to inform the DProxy IPv6 DNS server address */
   CMS_MSG_DNSPROXY_GET_STATS	                = 0x10000273, /**< Sent to dnsproxy to get DNS query error statistic */
   CMS_MSG_MCPD_RELOAD	                        = 0x10000276, /**< OBSOLETE: use mcpctl or its api instead. */
   CMS_MSG_MCPD_RESET	                        = 0x10000277, /**< OBSOLETE: use mcpctl or its api instead. */
   CMS_MSG_CONFIG_WRITTEN                       = 0x10000280, /**< Event sent when a config file is written. */
   CMS_MSG_CONFIG_UPLOAD_COMPLETE               = 0x10000281, /**< Event sent when a remote configuration cycle has ended. */
   CMS_MSG_DHCPC_GATEWAY_INFO                   = 0x10000282, /**< Sent from dhcp client when information associated with a connected Internet 
                                                                   Gateway Device is included in dhcp ack packet, see also DhcpcGatewayInfoMsgBody */
   CMS_MSG_DHCPC_REQ_OPTION_REPORT              = 0x10000283, /**< Sent from dhcp client to report request options, see also DhcpcReqOptionReportMsgBody */

   CMS_MSG_SET_PPP_UP                           = 0x10000290, /* Sent to ppp when set ppp up manually */
   CMS_MSG_SET_PPP_DOWN                         = 0x10000291, /* Sent to ppp when set ppp down manually */  

   CMS_MSG_LAN_CONNECTION_UP                    = 0x10000292, /* LAN UP - TBD: remove when proper LAN events are supported */
   CMS_MSG_LAN_CONNECTION_DOWN                  = 0x10000293, /* LAN DOWN - TBD: remove when proper LAN events are supported */

   CMS_MSG_DNSPROXY_DUMP_STATUS                 = 0x100002A1, /* Tell dnsproxy to dump its current status */
   CMS_MSG_DNSPROXY_DUMP_STATS                  = 0x100002A2, /* Tell dnsproxy to dump its statistics */
   CMS_MSG_RASTATUS6_INFO                       = 0x100002A3, /**< Sent from rastatus6 when RA is received, see also RAStatus6MsgBody */
   CMS_MSG_RASTATUS6_HOST6_RENEW	            = 0x100002A4, /**< Sent to rastatus6 to force it renew IPv6 host record */
   CMS_MSG_RASTATUS6_HOST6_INFO                 = 0x100002A5, /**< Sent from rastatus6 to ssk to inform of IPv6 host add/delete */
   CMS_MSG_GET_TR69C_SESSION_STATUS             = 0x100002B0, /* get TR69C session status (active[1], ended[0]) */
   CMS_MSG_AUTONOMOUS_TRANSFER_COMPLETE         = 0x100002B1, /* notify TR69C of an autonmous transfer (not initiated by ACS) */
   CMS_MSG_WAN_PORT_SET_OPSTATE                 = 0x100002B2, /* notify WAN auto detection result */
   CMS_MSG_FIREWALL_CTL                         = 0x100002B3, /**< See FirewallCtlMsgBody below */

   CMS_MSG_TCP_PURE_ACK_CONFIG_CHANGED          = 0x100002C0,  /**< (event) TCP Pure Ack queue config has changed, intfName follows hdr. */
   CMS_MSG_WLAN_CHANGED          	            = 0x10000300,  /**< Tell wlssk to restart the wlan daemons */

   CMS_MSG_SNMPD_CONFIG_CHANGED                 = 0x10000301, /**< ACS configuration has changed. */
   CMS_MSG_MANAGEABLE_DEVICE_NOTIFICATION_LIMIT_CHANGED
                                                = 0x10000302, /**< Notification Limit of number of management device. XXX Ugh, missing a 0 */
#if BRCM_PKTCBL_SUPPORT
   CMS_MSG_SNMPD_NOTIFICATION                   = 0x10000303, /**< SNMPD Notification request (CmsMsgSnmpdNotif) */
#endif /* BRCM_PKTCBL_SUPPORT */

   CMS_MSG_STORAGE_ADD_PHYSICAL_MEDIUM          = 0x10000310, /**< A physical storage medium has been added */
   CMS_MSG_STORAGE_REMOVE_PHYSICAL_MEDIUM       = 0x10000311, /**< A physical storage medium has been removed */
   CMS_MSG_STORAGE_ADD_LOGICAL_VOLUME           = 0x10000312, /**< A logical storage medium has been added */
   CMS_MSG_STORAGE_REMOVE_LOGICAL_VOLUME        = 0x10000313, /**< A logical storage medium has been removed */
   
   CMS_MSG_USB_DEVICE_STATE_CHANGE              = 0x10000315, /**< tell ssk usb device attach or detach */

   CMS_MSG_INTFSTACK_LOWERLAYERS_CHANGED        = 0x10000320, /**< tell ssk about change in LowerLayers param */
   CMS_MSG_INTFSTACK_ALIAS_CHANGED              = 0x10000321, /**< tell ssk about change in Alias param */
   CMS_MSG_INTFSTACK_OBJECT_DELETED             = 0x10000322, /**< tell ssk about object delete */
   CMS_MSG_INTFSTACK_STATIC_ADDRESS_CONFIG      = 0x10000323, /**< tell ssk about static address configuration */
   CMS_MSG_INTFSTACK_PROPAGATE_STATUS           = 0x10000324, /**< tell ssk about propagate interface stack due to ip interface enable status change */      
   CMS_MSG_INTFSTACK_PROPAGATE_STATUS_EX        = 0x10000325, /**< more flexible way to propagate status on interface stack */
   CMS_MSG_INTFSTACK_RESERVED_END               = 0x1000032F, /**< End of reserved range for interface stack messages */

   CMS_MSG_SET_BUS_MANAGER                      = 0x10000350, /**< Sender is the BCM Msg Bus manager */

   CMS_MSG_DIAG                                 = 0x10000790, /**< request diagnostic to be run (obsolete?  See START/STOP msg below) */
   CMS_MSG_TR69_GETRPCMETHODS_DIAG              = 0x10000791, /**< request tr69c send out a GetRpcMethods */
   CMS_MSG_DSL_LOOP_DIAG_COMPLETE               = 0x10000792, /**< dsl loop diagnostic completes */
   CMS_MSG_DSL_SELT_DIAG_COMPLETE               = 0x10000793, /**< dsl SELT diagnostic completes */
   CMS_MSG_ATM_OAM_DIAG_COMPLETE                = 0x10000795, /**< ATM OAM diagnostic completes */
   CMS_MSG_UPLOAD_DIAG_COMPLETE                 = 0x10000796, /**< IP upload diag is complete */
   CMS_MSG_DOWNLOAD_DIAG_COMPLETE               = 0x10000797, /**< IP download diag is complete */
   CMS_MSG_SPDSVC_DIAG_EVENT                    = 0x10000794, /**< speed service has an update (for websockd) */
   CMS_MSG_SPDSVC_DIAG_COMPLETE                 = 0x10000798, /**< speed service is done (for ssk) */

   CMS_MSG_START_PING_DIAG                      = 0x100007A0, /**< event msg to ssk to request start ping diag, no response */
   CMS_MSG_STOP_PING_DIAG                       = 0x100007A1, /**< event msg to ssk to request stop ping diag, no response */
   CMS_MSG_START_TRACERT_DIAG                   = 0x100007A2, /**< event msg to ssk to request start traceroute diag, no response */
   CMS_MSG_STOP_TRACERT_DIAG                    = 0x100007A3, /**< event msg to ssk to request stop traceroute diag, no response */
   CMS_MSG_START_UPLOAD_DIAG                    = 0x100007A4, /**< event msg to ssk to request start upload diag, no response */
   CMS_MSG_STOP_UPLOAD_DIAG                     = 0x100007A5, /**< event msg to ssk to request stop upload diag, no response */
   CMS_MSG_START_DOWNLOAD_DIAG                  = 0x100007A6, /**< event msg to ssk to request start download diag, no response */
   CMS_MSG_STOP_DOWNLOAD_DIAG                   = 0x100007A7, /**< event msg to ssk to request stop download diag, no response */
   CMS_MSG_START_UDPECHO                        = 0x100007A8, /**< event msg to ssk to request start udpecho service, no response */
   CMS_MSG_STOP_UDPECHO                         = 0x100007A9, /**< event msg to ssk to request stop udpecho service, no response */

   CMS_MSG_DIAG_RESERVED_END                    = 0x100007BF,

   CMS_MSG_INTERNAL_NOOP                        = 0x100007C0, /**< used internally by CMS MSG sub-system.  Apps which get this should just ignore it and free it. */

   CMS_MSG_REGISTER_DELAYED_MSG                 = 0x10000800, /**< request a message sometime in the future. */
   CMS_MSG_UNREGISTER_DELAYED_MSG               = 0x10000801, /**< cancel future message delivery. */
   CMS_MSG_REGISTER_EVENT_INTEREST              = 0x10000802, /**< request receipt of the specified event msg. */
   CMS_MSG_UNREGISTER_EVENT_INTEREST            = 0x10000803, /**< cancel receipt of the specified event msg. */

   CMS_MSG_START_APP                            = 0x10000807, /**< request smd to start an app; pid is returned in the wordData */
   CMS_MSG_RESTART_APP                          = 0x10000809, /**< request smd to stop and then start an app; pid is returned in the wordData */
   CMS_MSG_STOP_APP                             = 0x1000080A, /**< request smd to stop an app */
   CMS_MSG_IS_APP_RUNNING                       = 0x1000080B, /**< request to check if the the application is running or not */
   CMS_MSG_APP_TERMINATED                       = 0x1000080C, /**< register to smd for application termination info. */
   CMS_MSG_TERMINATE                            = 0x1000080D, /**< request app to terminate, a response means action has started. */
   CMS_MSG_IS_APP_ACTIVE                        = 0x1000080E, /**< request to check if the the application is active or not. (active means either LAUNCHED or RUNNING). */

   CMS_MSG_REBOOT_SYSTEM                        = 0x10000850,  /**< request smd to reboot, a response means reboot sequence has started. */

   CMS_MSG_DUMP_EID_INFO                        = 0x1000085A,  /**< request smd to dump its eid info DB */
   CMS_MSG_RESCAN_EID_INFO                      = 0x1000085B,  /**< request smd to rescan the eid info files (future) */
   CMS_MSG_ADD_EID_INFO                         = 0x1000085C,  /**< request smd to add given eid info to its DB (future) */
   CMS_MSG_APPLY_EID_INFO                       = 0x1000085D,  /**< request smd to apply EID settings (future) */

   CMS_MSG_SET_LOG_LEVEL                        = 0x10000860,  /**< request app to set its log level. */
   CMS_MSG_SET_LOG_DESTINATION                  = 0x10000861,  /**< request app to set its log destination. */
   CMS_MSG_SET_LOG_DESTINATION_MASK             = 0x10000862,  /**< request app to set its BCM ulog destination mask. */
   CMS_MSG_SET_LOG_LEVEL_EX                     = 0x10000863,  /**< request to set BCM ULog level by threadId or EID */
   CMS_MSG_SET_LOG_DESTINATION_MASK_EX          = 0x10000864,  /**< request to set BCM ULog dest mask by threadId or EID */

   CMS_MSG_MEM_DUMP_STATS                       = 0x1000086A,  /**< request app to dump its memstats */
   CMS_MSG_MEM_DUMP_TRACEALL                    = 0x1000086B,  /**< request app to dump all of its mem leak traces */
   CMS_MSG_MEM_DUMP_TRACE50                     = 0x1000086C,  /**< request app to its last 50 mem leak traces */
   CMS_MSG_MEM_DUMP_TRACECLONES                 = 0x1000086D,  /**< request app to dump mem leak traces with clones */

   CMS_MSG_LOAD_IMAGE_STARTING                  = 0x10000870,  /**< Obsolete: do not use anymore. */
   CMS_MSG_LOAD_IMAGE_DONE                      = 0x10000871,  /**< Obsolete: do not use anymore. */
   CMS_MSG_GET_CONFIG_FILE                      = 0x10000872,  /**< ask smd for a copy of the config file. */
   CMS_MSG_VALIDATE_CONFIG_FILE                 = 0x10000873,  /**< ask smd to validate the given config file. */
   CMS_MSG_WRITE_CONFIG_FILE                    = 0x10000874,  /**< ask smd to write the config file. */
   CMS_MSG_VENDOR_CONFIG_UPDATE                 = 0x10000875,  /**<  the config file. */

   CMS_MSG_GET_WAN_LINK_STATUS                  = 0x10000880,  /**< request current WAN LINK status. */
   CMS_MSG_GET_WAN_CONN_STATUS                  = 0x10000881,  /**< request current WAN Connection status. */
   CMS_MSG_GET_LAN_LINK_STATUS                  = 0x10000882,  /**< request current LAN LINK status. */
   CMS_MSG_GET_IF_LINK_STATUS                   = 0x10000883,  /**< request current Interface LINK status. */

   CMS_MSG_WATCH_WAN_CONNECTION                 = 0x10000890,  /**< request ssk to watch the dsl link status and then change the connectionStatus for bridge, static MER and ipoa */
   CMS_MSG_WATCH_DSL_LOOP_DIAG                  = 0x10000891,  /**< request ssk to watch the dsl loop diag and then update the stats */
   CMS_MSG_WATCH_DSL_SELT_DIAG                  = 0x10000892,  /**< request ssk to watch the dsl SELT diag and then update the stats */

   CMS_MSG_GET_LEASE_TIME_REMAINING             = 0x100008A0,  /**< ask dhcpd how much time remains on lease for particular LAN host */
   CMS_MSG_GET_DEVICE_INFO                      = 0x100008A1,  /**< request system/device's info */
   CMS_MSG_REQUEST_FOR_PPP_CHANGE               = 0x100008A2,  /**< request for disconnect/connect ppp  */
   CMS_MSG_EVENT_TIME_SYNC                      = 0x100008A3,  /**< NTP clients send sync delta value */
   CMS_MSG_VALIDATE_SESSION_KEY                 = 0x100008A4,  /**< ask httpd to validate the given session key. */

   CMS_MSG_QOS_DHCP_OPT60_COMMAND               = 0x100008C0, /**< QoS Vendor Class ID classification command */
   CMS_MSG_QOS_DHCP_OPT77_COMMAND               = 0x100008C1, /**< QoS User   Class ID classification command */

   CMS_MSG_EPONMAC_BOOT_COMPLETE                = 0x10001F00, /**< notification from eponapp that EPON mac has booted. */
   CMS_MSG_PORT_LOOP_DETECT                     = 0x10001F20, /**< EPON ONU port loop detect oam */
   CMS_MSG_PORT_DISABLE_LOOPED                  = 0x10001F21, /**< EPON ONU port disable looped oam */
   CMS_MSG_EPON_LINK_STATUS_CHANGED             = 0x10001F22, /**< EPON ONU link status changed */
   CMS_MSG_EPON_GET_LINK_STATUS                 = 0x10001F23, /**< EPON ONU link status */
   CMS_MSG_EPON_SET_ACS                         = 0x10001F26, /**< EPON ONU oam set acs config */
   CMS_MSG_EPON_GET_TRIPLECHURNING_STATE        = 0x10001F27,
   CMS_MSG_EPON_GET_WAN_CONFIG                  = 0x10001F28, /**< EPON ONU oam get wan intf confuration */
   CMS_MSG_EPON_SET_WAN_CONFIG                  = 0x10001F29, /**< EPON ONU oam set wan intf confuration */

   /* VOICE related messages */
   CMS_MSG_VOICE_INTERNAL                       = 0x10002000, /**< Internal Voice messages (CmsMsgVoiceInternalEvent) */
   CMS_MSG_VOICE_STOP                           = 0x10002001, /**< Voice stop request. */
   CMS_MSG_VOICE_START                          = 0x10002002, /**< Voice start request. */
   CMS_MSG_VOICE_RESTART                        = 0x10002003, /**< Voice restart request. */
   CMS_MSG_VOICE_CALLMGR_RESTART                = 0x10002004, /**< Call manager restart request. */
   CMS_MSG_VOICE_CONFIG_CHANGED                 = 0x10002005, /**< Voice Configuration parameter changed private event
                                                                 * msg. (CmsMsgVoiceConfigChanged) */
   CMS_MSG_VOICE_DEFAULT                        = 0x10002006, /**< Voice call manager set defaults request. */
   CMS_MSG_VOICE_DIAG                           = 0x10002007, /**< request voice diagnostic to be run (VoiceDiagMsgBody) */
   CMS_MSG_VOICE_OMCI_GET_RTP_STATS             = 0x10002008, /**< Voice get RTP PM stats msg (OMCI). */
   CMS_MSG_VOICE_OMCI_UPLD_COMPLETE             = 0x10002009, /**< OMCI configuration upload complete. */
   CMS_MSG_VOICE_STATISTICS_REQUEST             = 0x1000200A, /**< request for Voice call statistics */
   CMS_MSG_VOICE_STATISTICS_RESPONSE            = 0x1000200B, /**< response for Voice call statistics (CmsMsgVoiceStatisticsResponse) */
   CMS_MSG_VOICE_STATISTICS_RESET               = 0x1000200C, /**< request to reset Voice call statistics */
   CMS_MSG_VOICE_ANNOUNCE_DOWNLOAD              = 0x1000200D, /**< request to download voice announcement */
   CMS_MSG_VOICE_CODEC_PROFILE_DELETE           = 0x1000200E, /**< Notify that codec profile has changed */

   CMS_MSG_VOICE_PKTCBL_EVENT                   = 0x10002020, /**< PacketCable event (CmsMsgVoicePktcblEvent) */

   CMS_MSG_VOICE_BDK_SET_FIREWALL               = 0x10002030, /**< Set firewall rule in BDK.  Sent from voice app to voice cb thread.
                                                                   Req format:  VarData = firewall rule (BcmVoiceSysFirewallRuleType format).
                                                                   Resp format: no response */
   CMS_MSG_VOICE_BDK_CERT_REFRESH               = 0x10002031, /**< Refresh certificates in BDK. */

   CMS_MSG_VOICE_DECT_START                     = 0x100021A0, /**< request for number of DECT endpoints */
   CMS_MSG_VOICE_DECT_STOP                      = 0x100021A1, /**< request for number of DECT endpoints */
   CMS_MSG_VOICE_DECT_MEM_SET                   = 0x100021A2, /**< request for number of DECT endpoints */
   CMS_MSG_VOICE_DECT_MEM_GET                   = 0x100021A3, /**< request for number of DECT endpoints */
   CMS_MSG_VOICE_DECT_MODE_SET                  = 0x100021A4, /**< request for number of DECT endpoints */
   CMS_MSG_VOICE_DECT_MODE_GET                  = 0x100021A5, /**< request for number of DECT endpoints */
   CMS_MSG_VOICE_DECT_EVENT                     = 0x100021A6, /**< DECT EVENT raised by DECT endpoints */
   CMS_MSG_VOICE_DECT_READY                     = 0x100021A7, /**< DECT Module is ready */
   CMS_MSG_VOICE_DECT_CM_EVENT                  = 0x100021A8, /**< Call manager events or states */
   CMS_MSG_VOICE_DECT_CALL_CTL_CMD              = 0x100021A9, /**< DECT Call Control commands*/
   CMS_MSG_VOICE_DECT_LIST_UPDATE               = 0x100021AA, /**< DECT List update event */
   CMS_MSG_VOICE_DECT_DBG_START                 = 0x100021AB, /**< Start DECT debug task */
   CMS_MSG_VOICE_DECT_DBG_STOP                  = 0x100021AC, /**< Stop DECT debug task */

   CMS_MSG_VOICE_DECT_OPEN_REG_WND              = 0x100021F0, /**< request for opening DECT registration window */
   CMS_MSG_VOICE_DECT_CLOSE_REG_WND             = 0x100021F1, /**< request for closing DECT registration window */
   CMS_MSG_VOICE_DECT_INFO_REQ                  = 0x100021F2, /**< request for Voice DECT status information */
   CMS_MSG_VOICE_DECT_INFO_RSP                  = 0x100021F3, /**< response for Voice DECT status information */
   CMS_MSG_VOICE_DECT_AC_SET                    = 0x100021F4, /**< request for Voice DECT Access Code Set */
   CMS_MSG_VOICE_DECT_HS_INFO_REQ               = 0x100021F5, /**< request for Voice DECT handset status information */
   CMS_MSG_VOICE_DECT_HS_INFO_RSP               = 0x100021F6, /**< response for Voice DECT handset status information */
   CMS_MSG_VOICE_DECT_HS_DELETE                 = 0x100021F7, /**< request for deleting a handset from DECT module */
   CMS_MSG_VOICE_DECT_HS_PING                   = 0x100021F8, /**< request for pinging a handset from DECT module */
   CMS_MSG_VOICE_DECT_NUM_ENDPT                 = 0x100021F9, /**< request for number of DECT endpoints */
   CMS_MSG_VOICE_DECT_REGHSETLIST_UPDATE        = 0x100021FA, /**< Event indicating change in number of registered dect handset */
   CMS_MSG_VOICE_DECT_SYNC_DATE_TIME            = 0x100021FB, /**< request for date and time synchronization */

   CMS_MSG_GET_GPON_OMCI_STATS                  = 0x10002200, /**< request GPON OMCI statistics */
   CMS_MSG_OMCI_COMMAND_REQUEST                 = 0x10002201, /**< GPON OMCI command message request */
   CMS_MSG_OMCI_COMMAND_RESPONSE                = 0x10002202, /**< GPON OMCI command message response */
   CMS_MSG_OMCI_DEBUG_GET_REQUEST               = 0x10002203, /**< GPON OMCI debug get message request */
   CMS_MSG_OMCI_DEBUG_GET_RESPONSE              = 0x10002204, /**< GPON OMCI debug get message response */
   CMS_MSG_OMCI_DEBUG_SET_REQUEST               = 0x10002205, /**< GPON OMCI debug set message request */
   CMS_MSG_OMCI_DEBUG_SET_RESPONSE              = 0x10002206, /**< GPON OMCI debug set message response */
   CMS_MSG_OMCI_DEBUG_MKERR_SWDLERR1            = 0x10002207, /**< GPON OMCI debug drop next section to cause missing section error */
   CMS_MSG_OMCI_DEBUG_MKERR_SWDLERR2            = 0x10002208, /**< GPON OMCI debug drop final section of next window to cause no response on final window section error */
   CMS_MSG_OMCI_DEBUG_MKERR_SWDLERR3            = 0x10002209, /**< GPON OMCI debug corrupt next section to cause CRC error on SW DL image */
   CMS_MSG_OMCI_DUMP_INFO_REQ                   = 0x1000220A, /**< GPON OMCI debug dump internal info request */
   CMS_MSG_OMCI_DEBUG_OMCI_MSG_GEN              = 0x1000220B, /**< OMCI debug command to generate alarm, AVC, TCA etc. */


   CMS_MSG_OMCI_IGMP_ADMISSION_CONTROL          = 0x10002220, /**< mcpd request admission control from omcid */
   CMS_MSG_OMCI_MLD_ADMISSION_CONTROL           = 0x10002221, /**< mcpd request admission control from omcid */
   CMS_MSG_OMCI_CAPTURE_STATE_ON                = 0x10002230, /**< Start the capture of OMCI msgs from OLT */
   CMS_MSG_OMCI_CAPTURE_STATE_OFF               = 0x10002231, /**< Stop the capture of OMCI msgs from OLT */
   CMS_MSG_OMCI_CAPTURE_REPLAY_ON               = 0x10002232, /**< Start the playback of OMCI msgs */
   CMS_MSG_OMCI_CAPTURE_REPLAY_OFF              = 0x10002233, /**< Start the playback of OMCI msgs */
   CMS_MSG_OMCI_CAPTURE_VIEW                    = 0x10002234, /**< Start the display OMCI msgs from a file */
   CMS_MSG_OMCI_CAPTURE_DOWNLOAD                = 0x10002235, /**< Download internal OMCI msg capture file */
   CMS_MSG_OMCI_CAPTURE_UPLOAD                  = 0x10002236, /**< Upload a file of OMCI msgs to replace internal file */
   CMS_MSG_OMCI_PROMISC_SET_REQUEST             = 0x10002240, /**< GPON OMCI Promisc set message request */
   CMS_MSG_OMCI_PROMISC_SET_RESPONSE            = 0x10002241, /**< GPON OMCI Promisc set message response */
   CMS_MSG_OMCI_ETH_PORT_TYPE_SET_REQUEST       = 0x10002245, /**< GPON OMCI Eth Port Type set message request */
   CMS_MSG_OMCI_ETH_PORT_TYPE_GET_REQUEST       = 0x10002246, /**< GPON OMCI Eth Port Type get message request */
   CMS_MSG_OMCI_ETH_PORT_TYPE_SET_RESPONSE      = 0x10002247, /**< GPON OMCI Eth Port Type set message response */
   CMS_MSG_OMCI_GPON_WAN_SERVICE_STATUS_CHANGE  = 0x10002250, /**< OMCI-->RG - Wan service status change notification */
   CMS_MSG_OMCI_RG_WAN_SERVICE_STAUTS_CHANGE    = 0x10002251, /**< RG-->OMCI - WAN service status change notification */
   CMS_MSG_OMCI_MCPD_MIB_RESET                  = 0x10002252, /**< OBSOLET: use mcpctl or its api instead. */

   CMS_MSG_OMCIPMD_ALARM_SEQ_SET                = 0x10002301, /**< OMCIPMD command to set OMCI Alarm Sequence Number. */
   CMS_MSG_OMCIPMD_ALARM_SEQ_GET                = 0x10002302, /**< OMCIPMD command to get OMCI Alarm Sequence Number. */

   CMS_MSG_WAN_PORT_ENABLE                      = 0x10002442, /**< Enable MDM Wan port by name */

   CMS_MSG_LXC_STATUS_INFO                      = 0x100024FF, /**< LXC update container's status */
   CMS_MSG_MODSW_BEGIN                          = 0x10002500,  /**< start of Modular Software Message types */
   CMS_MSG_MODSW_RESERVIED                      = 0x10002501,  /**< This range is reserved for Modular Software, do not add your messages here! */
   CMS_MSG_MODSW_END                            = 0x100025FF,  /**< end of Modular Software Message types */

   CMS_MSG_BMU_BEGIN                    = 0x10002600,  /**< start of Battery Management Unit Message types */
   CMS_MSG_BMU_END                      = 0x1000261F,  /**< end of Battery Management Unit message types */

   CMS_MSG_WLAN_LINK_STATUS_CHANGED     = 0x10002655, /**<APSTA link event to ssk to update Wifi Wan status (obsolete) */
   CMS_MSG_WIFI_LINK_UP                 = 0x1000265A, /**< LAN side Wifi SSID link is up */
   CMS_MSG_WIFI_LINK_DOWN               = 0x1000265B, /**< LAN side Wifi SSID link is down */
   CMS_MSG_WIFI_UPDATE_ASSOCIATEDDEVICE = 0x1000265C, /**< update associated device status */

   CMS_MSG_SUBSCRIBE_EVENT              = 0x10002700,  /**< Subscribe to an event.  Current info will be returned in response msg.  See cms_msg_pubsub.h. */
   CMS_MSG_UNSUBSCRIBE_EVENT            = 0x10002701,  /**< Unsubscribe to an event.  See cms_msg_pubsub.h for details.*/
   CMS_MSG_GET_EVENT_STATUS             = 0x10002702,  /**< Get current status of event.  See cms_msg_pubsub.h for details.*/
   CMS_MSG_NOTIFY_EVENT                 = 0x10002703,  /**< Subscribers to an event will receive this when event is published (status changes).*/
   CMS_MSG_PUBLISH_EVENT                = 0x10002704,  /**< The owner/manager of the info publishes event to let others know of status change.*/
   CMS_MSG_UNPUBLISH_EVENT              = 0x10002705,  /**< The owner/manager of the info unpublishes this event. */
   CMS_MSG_QUERY_EVENT_STATUS           = 0x10002706,  /**< Query events in sysdir, the data payload must contain query string. */
   CMS_MSG_DUMP_SYSDIR                  = 0x10002730,  /**< Tell sys_directory to dump its data, wordData indicates type */
   CMS_MSG_PUBSUB_END                   = 0x1000273F,  /**< end of Pub/Sub Message types */

   CMS_MSG_XMPP_CONNECTION_ENABLE       = 0x10002800, /**< Enable/Disable XMPP Connection */
   CMS_MSG_XMPP_CONNECTION_DELETE       = 0x10002801, /**< Delete XMPP Connection */
   CMS_MSG_XMPP_CONNECTION_UPDATE       = 0x10002802, /**< Update XMPP Connection */
   CMS_MSG_XMPP_CONNECTION_UP           = 0x10002803, /**< XMPP Connection status up */

   CMS_MSG_XMPP_REQUEST_SEND_CONNREQUEST_EVENT = 0x10002805, /**< XMPP request TR69c to send Connection Request Event to ACS */

   CMS_MSG_PERIODICSTAT_START_SAMPLESET    = 0x10002900, /**< SampleSet -- start or restart */
   CMS_MSG_PERIODICSTAT_PAUSE_SAMPLESET    = 0x10002901, /**< SampleSet -- pause while disable */
   CMS_MSG_PERIODICSTAT_STOP_SAMPLESET     = 0x10002902, /**< SampleSet -- stop and delete */
   CMS_MSG_PERIODICSTAT_UPDATE_SAMPLESET   = 0x10002903, /**< SampleSet -- update interval and report samples */
   CMS_MSG_PERIODICSTAT_FORCE_SAMPLE       = 0x10002904, /**< Force to fetch a sample */
   CMS_MSG_PERIODICSTAT_ADD_PARAMETER      = 0x10002905, /**< Add a parameter into sample set*/
   CMS_MSG_PERIODICSTAT_DELETE_PARAMETER   = 0x10002906, /**< Delete a parameter from sample set */
   CMS_MSG_PERIODICSTAT_UPDATE_PARAMETER   = 0x10002907, /**< Update attributes of a parameter */
   CMS_MSG_PERIODICSTAT_STATUS_CHANGE      = 0x10002908, /**< Notify that sample set's status is changed from Enabled to Trigger */

   CMS_MSG_ETHLAG_LINK_UP                  = 0x10002950, /**< ethLag link is up */
   CMS_MSG_ETHLAG_LINK_DOWN                = 0x10002951, /**< ethLag link is down */

   CMS_MSG_IEEE1905_GET_CFG             = 0x10003570, /**< Get ieee parms FROM MDM */
   CMS_MSG_IEEE1905_SET_CFG             = 0x10003571, /**< Save ieee parms TO MDM */

   CMS_MSG_MDM_POST_ACTIVATING          = 0x10003700, /**< MDM Post Activating */

   CMS_MSG_DBUS_PROPERTIES_CHANGED       = 0x1000A000, /**< DBus properties changed message */
   CMS_MSG_DBUS_OBJECT_ADD               = 0x1000A001, /**< DBus object add message */
   CMS_MSG_DBUS_OBJECT_DELETE            = 0x1000A002, /**< DBus object delete message */
   CMS_MSG_DBUS_MULTI_PROPERTIES_CHANGED = 0x1000A003, /**< DBus Multiple properties changed message */
   CMS_MSG_ECMS_WIFI_CONN_FAILURE        = 0x1000A004, /**< ECMS WIFI Connection Failure message */
   CMS_MSG_ECMS_OMDIAG_RES_CPUMEM        = 0x1000A005, /**< ECMS exceed cpu memory alarm per resource */
   CMS_MSG_ECMS_WLAN_APRSSI_QUERY_RESP   = 0x1000A006, /**< ECMS HTTPD APRSSI Query response */
   CMS_MSG_ECMS_WLAN_STARSSI_QUERY_RESP  = 0x1000A007, /**< ECMS HTTPD STARSSI Query response */
   CMS_MSG_ECMS_WLAN_STA_BSSTRANS_RESP   = 0x1000A008, /**< ECMS HTTPD Sta BSS Transition response */
   CMS_MSG_ECMS_WLAN_STARSSI_LOW_ALARM   = 0x1000A009, /**< ECMS Sta Rssi lower threshold alarm */
   CMS_MSG_ECMS_SYS_VOICE_EVENTS         = 0x1000A00A, /**< ECMS VOICE Events message */
   CMS_MSG_ECMS_TO_WLCTDM                = 0x1000A00B, /**< ECMS to WLCTM*/

   CMS_MSG_REMOTE_OBJ_GET               = 0x1000A100, /**< request to remote_objd to get object. see cms_msg_remoteobj.h for details.*/
   CMS_MSG_REMOTE_OBJ_GETNEXT           = 0x1000A101, /**< request to remote_objd to get next object. see cms_msg_remoteObj.h for details.*/
   CMS_MSG_REMOTE_OBJ_GETANCESTOR       = 0x1000A102, /**< request to remote_objd to get the ancestor of an object. see cms_msg_remoteObj.h for details.*/
   CMS_MSG_REMOTE_OBJ_ADD_INSTANCE      = 0x1000A103, /**< request to remote_objd to add an object instance. see cms_msg_remoteObj.h for details.*/
   CMS_MSG_REMOTE_OBJ_DEL_INSTANCE      = 0x1000A104, /**< request to remote_objd to deete an object instance. see cms_msg_remoteObj.h for details.*/
   CMS_MSG_REMOTE_OBJ_SET               = 0x1000A105, /**< request to remote_objd to set object. see cms_msg_remoteObj.h for details.*/
   CMS_MSG_REMOTE_OBJ_GET_PARAMS        = 0x1000A106,
   CMS_MSG_REMOTE_OBJ_SET_PARAMS        = 0x1000A107,
   CMS_MSG_REMOTE_OBJ_GET_PARAMNAMES    = 0x1000A108,
   CMS_MSG_REMOTE_OBJ_GET_PARAMATTRS    = 0x1000A109,
   CMS_MSG_REMOTE_OBJ_SET_PARAMATTRS    = 0x1000A10A,
   CMS_MSG_REMOTE_OBJ_SAVE_CONFIG       = 0x1000A10B, /**< request to remote_objd to save running config to PSI */
   CMS_MSG_REMOTE_OBJ_INVALIDATE_CONFIG       = 0x1000A10C,
   CMS_MSG_REMOTE_OBJ_READ_CONFIG       = 0x1000A10D, /**< request to remote_objd to read PSI to a buffer */
   CMS_MSG_REMOTE_GET_CHANGED_PARAMS    = 0x1000A10E, /**< request to remote_objd to list of all changed parameters */
   CMS_MSG_REMOTE_CLEAR_ALL_PARAM_VALUE_CHANGES  = 0x1000A10F, /**< request to remote_objd to clear all param value changed recording */
   CMS_MSG_REMOTE_GET_NUM_PARAM_VALUE_CHANGES    = 0x1000A110, /**< request to remote_objd to total number of changed parameters */
   CMS_MSG_REMOTE_OBJ_DUMP_STATS        = 0x1000A111, /**< request to dump out statistics in remote_objd. see cms_msg_remoteObj.h for details.*/
   CMS_MSG_REMOTE_OBJ_READ_MDM          = 0x1000A112, /**< request to remote_objd to read MDM to a buffer */
   CMS_MSG_REMOTE_OBJ_ADD_VENDOR_CONFIG = 0x1000A120, /**< request to remote_objd to add a DeviceInfo.VendorConfigFile entry */
   CMS_MSG_REMOTE_OBJ_DEL_VENDOR_CONFIG = 0x1000A121, /**< request to remote_objd to delete the specified DeviceInfo.VendorConfigFile entry */
   CMS_MSG_REMOTE_OBJ_NS_LOOKUP         = 0x1000A122, /**< request namespace lookup of the fullpath in the body of the msg */

   CMS_MSG_ALT_NOTIFICATION             = 0x1000B100, /**< This message is sent to the related md, then converted to bus event to show  that
                                                       * parameters with altchange attribute has had their value changed*/

   CMS_MSG_CUSTOMER_RESERVED_BEGIN      = 0x20000000, /**< This range of messages are reserved for customer use */
   CMS_MSG_CUSTOMER_RESERVED_END        = 0x2FFFFFFF, /**< End of customer reserved range */

   CMS_MSG_FUTURE_RESERVED_BEGIN        = 0x30000000 /**< All messages from this point are reserved for future use */

} CmsMsgType;



/** This header must be at the beginning of every message.
 *
 * The header may then be followed by additional optional data, depending on
 * the message type.
 * Most of the fields should be self-explanatory.
 * Be very mindful of 32 vs 64 bit issues when adding or removing fields from
 * the header.  The compiler will add padding to the middle and end of the
 * structure to ensure proper alignment, but the positions of the fields and
 * the total size of the struct must be the same whether it is compiled 32 or
 * 64 bit.
 */
typedef struct cms_msg_header
{
   CmsMsgType  type;  /**< specifies what message this is. */
   CmsEntityId src;   /**< CmsEntityId of the sender; for apps that can have
                       *   multiple instances, use the MAKE_SPECIFI_EID macro. */
   CmsEntityId dst;   /**< CmsEntityId of the receiver; for apps that can have
                       *   multiple instances, use the MAKE_SPECIFI_EID macro. */
   union {
      UINT16 all;     /**< All 16 bits of the flags at once. */
      struct {
         UINT16 event:1;    /**< This is a event msg. */
         UINT16 request:1;  /**< This is a request msg. */
         UINT16 response:1; /**< This is a response msg. */
         UINT16 available1:1;  /**< was requeue, no longer used */
         UINT16 bounceIfNotRunning:1; /**< Do not launch the app to receive this message if
                                       *  it is not already running. */
         UINT16 unused:11;  /**< For future expansion. */
      } bits;
   } flags;  /**< Modifiers to the type of message. */
   UINT16 sequenceNumber;     /**< "Optional", but read the explanation below.
                               *
                               * Senders of request or event message types
                               * are free to set this to whatever
                               * they want, or leave it unitialized.  Senders
                               * are not required to increment the sequence
                               * number with every new message sent.
                               * However, response messages must 
                               * return the same sequence number as the
                               * request message.
                               * 
                               */
   UINT64 next;               /**< This is actually a cms_msg_header ptr.
                                   Allows CmsMsgHeaders to be chained. */
   UINT32 wordData;   /**< As an optimization, allow one word of user
                       *   data in msg hdr.
                       *
                       * For messages that have only one word of data,
                       * we can just put the data in this field.
                       * One good use is for response messages that just
                       * need to return a status code.  The message type
                       * determines whether this field is used or not.
                       */
   UINT32 dataLength; /**< Amount of data following the header.  0 if no additional data. */
} CmsMsgHeader;

#define flags_event        flags.bits.event      /**< Convenience macro for accessing event bit in msg hdr */
#define flags_request      flags.bits.request    /**< Convenience macro for accessing request bit in msg hdr */
#define flags_response     flags.bits.response   /**< Convenience macro for accessing response bit in msg hdr */
#define flags_bounceIfNotRunning flags.bits.bounceIfNotRunning   /**< Convenience macro for accessing bounceIfNotRunning bit in msg hdr */

#define EMPTY_MSG_HEADER   {0, 0, 0, {0}, 0, 0, 0, 0} /**< Initialize msg header to empty */


#define BCM_MSG_BUS_NAME_LENGTH  512  /**< Max length of message bus name. */

/** List of BCM Message Bus names, one for each Distributed MDM component.
 *
 *  The sysmgmt component will use the SMD_MESSAGE_ADDR.  In CMS classic,
 *  the only message bus is the SMD_MESSAGE_ADDR bus.
 */
#define DEVINFO_MSG_BUS   "/tmp/devinfo_msg_bus"
#define DIAG_MSG_BUS      "/tmp/diag_msg_bus"
#define DSL_MSG_BUS       "/tmp/dsl_msg_bus"
#define GPON_MSG_BUS      "/tmp/gpon_msg_bus"
#define EPON_MSG_BUS      "/tmp/epon_msg_bus"
#define WIFI_MSG_BUS      "/tmp/wifi_msg_bus"
#define VOICE_MSG_BUS     "/tmp/voice_msg_bus"
#define STORAGE_MSG_BUS   "/tmp/storage_msg_bus"
#define TR69_MSG_BUS      "/tmp/tr69_msg_bus"
#define USP_MSG_BUS       "/tmp/usp_msg_bus"
#define OPENPLAT_MSG_BUS  "/tmp/openplat_msg_bus"
#define SYS_DIRECTORY_MSG_BUS "/tmp/sys_directory_msg_bus"
#define SYSMGMT_MSG_BUS   SMD_MESSAGE_ADDR


/** Number of connections that can be queued up waiting to be accepted.
 *
 *  This is an obscure Linux socket parameter which probably never needs to
 *  be changed.
 */
#define BCM_MSG_BUS_MESSAGE_BACKLOG  3


/** Useful macro used with select system call.  See cmsMsg_getEventHandle().
 */
#define UPDATE_MAXFD(f)  (maxFd = (f > maxFd) ? f : maxFd)



/** Data body for CMS_MSG_REGISTER_DELAYED_MSG.
 */
typedef struct
{
   UINT32  delayMs; /**< Number of milliseconds in the future to deliver this message. */

} RegisterDelayedMsgBody;


/** Data body for CMS_MSG_SET_LOG_LEVEL_EX and SET_LOG_DESTINATION_MASK_EX
 */
typedef struct
{
   SINT32 threadId;
   UINT32 eid;
} BcmUlogExMsgBody;


/** Data body for CMS_MSG_DHCPC_STATE_CHANGED message type.
 *
 */
typedef struct
{
   UBOOL8 addressAssigned; /**< Have we been assigned an IP address ? */
   UBOOL8 isExpired; /**< Is the lease time expired ? */
   char ip[BUFLEN_32];   /**< New IP address, if addressAssigned==TRUE */
   char mask[BUFLEN_32]; /**< New netmask, if addressAssigned==TRUE */
   char gateway[BUFLEN_32];    /**< New gateway, if addressAssigned==TRUE */
   char nameserver[BUFLEN_64]; /**< New nameserver, if addressAssigned==TRUE */
   char ntpserver[BUFLEN_64];  /**< New ntp server(s), if addressAssigned==TRUE */
   UINT8 ipv4MaskLen; /**< For 6rd parameters (option 212) */
   UINT8 ipv6PrefixLen;
   char prefix[BUFLEN_48];
   char brAddr[BUFLEN_32];
   char hostName[BUFLEN_32];
   char domain[BUFLEN_32];
   char acsURL[CMS_MAX_ACS_URL_LENGTH];    /**< dhcp server may provide this */
   char acsProvisioningCode[CMS_MAX_ACS_PROVISIONING_CODE_LENGTH];  /**< dhcp server may provide this */
   UINT32 cwmpRetryMinimumWaitInterval; /**< dhcp server may provide this */
   UINT32 cwmpRetryIntervalMultiplier; /**< dhcp server may provide this */
} DhcpcStateChangedMsgBody;

/** Data body for CMS_MSG_DHCPC_GATEWAY_INFO message type.
 *
 */
typedef struct
{
  char ManufacturerOUI[BUFLEN_8];  /**< Organizationally unique identifier of the associated Internet Gateway Device */
  char ProductClass[BUFLEN_64]; /**< Identifier of the product class of the associated Internet Gateway Device */
  char SerialNumber[BUFLEN_64]; /**< Serial number of the associated Internet Gateway Device */
} DhcpcGatewayInfoMsgBody; 

/** Data body for CMS_MSG_DHCPC_REQ_OPTION_REPORT message type.
 *
 */
typedef struct
{
  UINT32 leasedTime;  /**< The particular time in seconds is requested to be leased */
  char clientAddress[BUFLEN_32]; /**< The particular IP address is requested to be assigned */
  char serverAddress[BUFLEN_32]; /**< The destination address for any DHCP messages unicast to the DHCP server */
} DhcpcReqOptionReportMsgBody; 

/** Data body for CMS_MSG_DHCP6C_STATE_CHANGED message type.
 *
 */
typedef struct
{
   UBOOL8 prefixAssigned;  /**< Have we been assigned a site prefix ? */
   UBOOL8 addrAssigned;    /**< Have we been assigned an IPv6 address ? */
   UBOOL8 dnsAssigned;     /**< Have we been assigned dns server addresses ? */
   UBOOL8 domainNameAssigned;     /**< Have we been assigned dns server addresses ? */
   UBOOL8 aftrAssigned;     /**< Have we been assigned aftr name ? */
   UBOOL8 mapeAssigned;     /**< Have we been assigned mape config ? */
   UBOOL8 maptAssigned;     /**< Have we been assigned mapt config ? */
   char sitePrefix[BUFLEN_48];   /**< New site prefix, if prefixAssigned==TRUE */
   UINT32 prefixPltime;
   UINT32 prefixVltime;
   char sitePrefixOld[BUFLEN_48]; /**< add support for RFC7084 requirement L-13 */
   UINT32 prefixVltimeOld;
   UINT32 prefixCmd;
   char ifname[BUFLEN_32];
   char address[BUFLEN_48];      /**< New IPv6 address, if addrAssigned==TRUE */
   UINT32 addressPltime;
   UINT32 addressVltime;
   char pdIfAddress[BUFLEN_48];      /**< New IPv6 address of PD interface */
   UINT32 addrCmd;
   char nameserver[BUFLEN_128];  /**< New nameserver, if addressAssigned==TRUE */
   char domainName[BUFLEN_64];  /**< New domain Name, if addressAssigned==TRUE */
   char ntpserver[BUFLEN_128];  /**< New ntp server(s), dhcp server may provide this */
   char acsURL[CMS_MAX_ACS_URL_LENGTH];      /**< dhcp server may provide this */
   char acsProvisioningCode[CMS_MAX_ACS_PROVISIONING_CODE_LENGTH];  /** dhcp server may provide this */
   UINT32 cwmpRetryMinimumWaitInterval; /**< dhcp server may provide this */
   UINT32 cwmpRetryIntervalMultiplier; /**< dhcp server may provide this */
   char aftr[CMS_AFTR_NAME_LENGTH];      /**< dhcp server may provide this */
   char brIPv6Prefix[CMS_IPADDR_LENGTH]; /**< border relay address or prefix */
   char ruleIPv4Prefix[CMS_IPADDR_LENGTH];
   char ruleIPv6Prefix[CMS_IPADDR_LENGTH];
   UINT32 eaLen;
   UINT32 psidOffset;
   UINT32 psidLen;
   UINT32 psid;
   UBOOL8 isFMR;
} Dhcp6cStateChangedMsgBody;


/** Data body for CMS_MSG_RASTATUS6_INFO message type.
 *
 */
typedef struct
{
   char pio_prefix[CMS_IPADDR_LENGTH];  /**< prefix info in PIO, we only support one prefix in one RA message */
   UINT8 pio_prefixLen;
   UINT32 pio_plt;
   UINT32 pio_vlt;
   UBOOL8 pio_A_flag;
   UBOOL8 pio_L_flag;
   char dns_servers[CMS_IPADDR_LENGTH*2];  /**< RFC6106, we only support up to two DNS servers in one RA message */
   UINT32 dns_lifetime;
   char domainName[BUFLEN_32];  /**< RFC6106, we only support up to 32 characters in one RA message */
   UINT32 domainName_lifetime;
   char router[BUFLEN_40];  /**< source IP of the RA message */
   UINT32 router_lifetime;
   UBOOL8 router_M_flags;
   UBOOL8 router_O_flags;
   UBOOL8 router_P_flags;
   char ifName[BUFLEN_32];  /** < the interface which receives the RA */
} RAStatus6MsgBody;


/* ECMS Reverse Message Begin */
/* Data body for CMS_MSG_DBUS_PROPERTIES_CHANGED message type */
typedef struct
{
   char objectPath[BUFLEN_128];      /**< object path of service */
   char interfaceName[BUFLEN_64];    /**< interface name of object */
   char propName[BUFLEN_64];         /**< property name of interface */
   char propType[BUFLEN_32];         /**< property type of interface */
   char propValue[BUFLEN_1024];      /**< property value of interface */
} PropertiesChangedMsgBody;

/* Data body for CMS_MSG_DBUS_MULTI_PROPERTIES_CHANGED message type. 
 *
 * {'MemAlarm': <byte 0x46>, 'InternalPort': <uint32 20>, 'TxPkts': <uint64 0>}
 * {'Enable': <false>, 'Temperature': <0.0>, 'UserName':<'test123'>}
 *
 */
typedef struct
{
   char objectPath[BUFLEN_128];      /**< object path of service */
   char interfaceName[BUFLEN_64];    /**< interface name of object */
   char propertiesText[BUFLEN_1024]; /**< Multiple changed properties via text patten */
} MultiPropertiesChangedMsgBody;

/* Data body for CMS_MSG_DBUS_OBJECT_ADD message type */
typedef struct
{
   char objectPath[BUFLEN_128];       /**< object path of service */
   char toAddObjPath[BUFLEN_128];     /**< to added object path */
} ObjectAddMsgBody;

/* Data body for CMS_MSG_DBUS_OBJECT_DELETE message type */
typedef struct
{
   char objectPath[BUFLEN_128];       /**< object path of service */
   char toDeleteObjPath[BUFLEN_128];  /**< to deleted object path */
} ObjectDeleteMsgBody;

/*Data body of CMS_MSG_ECMS_WIFI_CONN_FAILURE message type */
typedef struct
{
   char ssid[BUFLEN_64];
   char bssid[BUFLEN_32];
   char reason[BUFLEN_128];
}WifiConnFailureMsgBody;

/*Data body of CMS_MSG_ECMS_SYS_VOICE_EVENTS message type*/
typedef struct
{
   char event[BUFLEN_64];
   char description[BUFLEN_128];
}SysVoiceEventsMsgBody;

/* Data body for CMS_MSG_ECMS_WLAN_APRSSI_QUERY_RESP message type */
#define MAX_AP_NUMS 16 
typedef struct
{
   char bssid[BUFLEN_32];
   int rssi;
} ApRssiResult;

typedef struct
{
   unsigned int result;
   char errdesc[BUFLEN_64];
   char mac[BUFLEN_32];
   ApRssiResult rssiResult[MAX_AP_NUMS];
   unsigned int RFBand;
   int nums;
} ApRssiMsgBody;

/* Data body for CMS_MSG_ECMS_WLAN_STARSSI_QUERY_RESP message type */
#define MAX_STA_NUMS 32 
typedef struct
{
   char mac[BUFLEN_32];
   int rssi;
} StaRssiResult;

typedef struct
{
   unsigned int result;
   char errdesc[BUFLEN_64];
   StaRssiResult rssiResult[MAX_STA_NUMS];
   unsigned int RFBand;
   int nums;
} StaRssiMsgBody;

/* Data body for CMS_MSG_ECMS_WLAN_STA_BSSTRANS_RESP message type */
typedef struct
{
   unsigned int result;
   char errdesc[BUFLEN_64];
   char mac[BUFLEN_32];
   unsigned int statusCode;
   unsigned int RFBand;
} StaBssTransResultMsgBody;

/* Data body for CMS_MSG_ECMS_WLAN_STARSSI_LOW_ALARM message type */
typedef struct
{
   char objectPath[BUFLEN_128]; /**< object path of STA */
   char mac[BUFLEN_32];
   int type;
   int rssi;
   unsigned int RFBand;
} StaRssiAlarmMsgBody;

/* ECMS Reverse Message End */

/*!\PPPOE state defines 
 * (was in syscall.h before)
 */

#define BCM_PPPOE_CLIENT_STATE_PADO          0   /* waiting for PADO */
#define BCM_PPPOE_CLIENT_STATE_PADS          1   /* got PADO, waiting for PADS */
#define BCM_PPPOE_CLIENT_STATE_CONFIRMED     2   /* got PADS, session ID confirmed */
#define BCM_PPPOE_CLIENT_STATE_DOWN          3   /* totally down */
#define BCM_PPPOE_CLIENT_STATE_UP            4   /* totally up */
#define BCM_PPPOE_SERVICE_AVAILABLE          5   /* ppp service is available on the remote */
#define BCM_PPPOE_AUTH_FAILED                7
#define BCM_PPPOE_RETRY_AUTH                 8
#define BCM_PPPOE_REPORT_LASTCONNECTERROR    9
#define BCM_PPPOE_CLIENT_STATE_UNCONFIGURED   10 
#define BCM_PPPOE_CLIENT_IPV6_STATE_UP   11
#define BCM_PPPOE_CLIENT_IPV6_STATE_DOWN   12

/** These values are returned in the wordData field of the response msg to
 *  CMS_MSG_GET_WAN_LINK_STATUS.
 *  See dslIntfStatusValues in cms-data-model.xml
 * There is a bit of confusion here.  These values are associated with the
 * WANDSLInterfaceConfig object, but usually, a caller is asking about
 * a WANDSLLinkConfig object. For now, the best thing to do is just check
 * for WAN_LINK_UP.  All other values basically mean the requested link is
 * not up.
 */
#define WAN_LINK_UP                   0
#define WAN_LINK_INITIALIZING         1
#define WAN_LINK_ESTABLISHINGLINK     2
#define WAN_LINK_NOSIGNAL             3
#define WAN_LINK_ERROR                4
#define WAN_LINK_DISABLED             5

#define LAN_LINK_UP                   0
#define LAN_LINK_DISABLED             1


/** Data body for CMS_MSG_PPPOE_STATE_CHANGED message type.
 *
 */
typedef struct
{
   UINT8 pppState;       /**< pppoe states */
   char ip[BUFLEN_32];   /**< New IP address, if pppState==BCM_PPPOE_CLIENT_STATE_UP */
   char ifname[BUFLEN_32];   /**< ppp interface name, if pppState==BCM_PPPOE_CLIENT_STATE_UP */
   char mask[BUFLEN_32]; /**< New netmask, if pppState==BCM_PPPOE_CLIENT_STATE_UP */
   char gateway[BUFLEN_32];    /**< New gateway, if pppState==BCM_PPPOE_CLIENT_STATE_UP */
   char nameserver[BUFLEN_64]; /**< New nameserver, if pppState==BCM_PPPOE_CLIENT_STATE_UP */
   char servicename[BUFLEN_256]; /**< service name, if pppState==BCM_PPPOE_CLIENT_STATE_UP */
   char ppplastconnecterror[PPP_CONNECT_ERROR_REASON_LEN];
   char localIntfId[CMS_IPADDR_LENGTH];   /**< Local interface identifier, if pppState==BCM_PPPOE_CLIENT_IPV6_STATE_UP */
   char remoteIntfId[CMS_IPADDR_LENGTH];   /**< Remote interface identifier, if pppState==BCM_PPPOE_CLIENT_IPV6_STATE_UP */
} PppoeStateChangeMsgBody;


/*!\enum NetworkAccessMode
 * \brief Different types of network access modes, returned by cmsDal_getNetworkAccessMode
 *        and also in the wordData field of CMS_MSG_GET_NETWORK_ACCESS_MODE
 */
typedef enum {
   NETWORK_ACCESS_DISABLED   = 0,  /**< access denied */
   NETWORK_ACCESS_LAN_SIDE   = 1,  /**< access from LAN */
   NETWORK_ACCESS_WAN_SIDE   = 2,  /**< access from WAN */
   NETWORK_ACCESS_CONSOLE    = 3   /**< access from serial console */
} NetworkAccessMode;


/** Data body for CMS_MSG_DHCPD_HOST_INFO message type.
 *
 */
typedef struct
{
   UBOOL8 deleteHost;  /**< TRUE if we are deleting a LAN host, otherwise we are adding or editing LAN host */
   SINT32 leaseTimeRemaining;      /** Number of seconds left in the lease, -1 means no expiration */
   char ifName[CMS_IFNAME_LENGTH]; /**< brx which this host is on */
   char ipAddr[BUFLEN_48];         /**< IP Address of the host */
   char macAddr[MAC_STR_LEN+1];    /**< mac address of the host */
   char addressSource[BUFLEN_32];  /** source of IP address assignment, same value as
                                     * LANDevice.{i}.Hosts.Host.{i}.addressSource */
   char interfaceType[BUFLEN_32];  /** type of interface used by LAN host, same values as 
                                     * LANDevice.{i}.Hosts.Host.{i}.InterfaceType */
   char hostName[BUFLEN_64];       /** Both dhcpd and data model specify hostname as 64 bytes */
   char oui[BUFLEN_8];             /** Host's manufacturing OUI */
   char serialNum[BUFLEN_64];      /** Host's serial number */
   char productClass[BUFLEN_64];   /** Host's product class */
} DhcpdHostInfoMsgBody;


/** Data body for CMS_MSG_GET_LEASE_TIME_REMAINING message type.
 *
 * The lease time remaing is returned in the wordData field of the
 * response message.  A -1 means the lease does not expire.
 * A 0 could mean the lease is expired, or that dhcpd has not record
 * of the mac address that was given.
 *
 */
typedef struct
{
   char ifName[CMS_IFNAME_LENGTH]; /**< brx which this host is on */
   char macAddr[MAC_STR_LEN+1];    /**< mac address of the host */
} GetLeaseTimeRemainingMsgBody;



typedef enum 
{
   VOICE_DIAG_CFG_SHOW           = 0,
   VOICE_DIAG_EPTCMD             = 1,
   VOICE_DIAG_STUNLKUP           = 2,
   VOICE_DIAG_STATS_SHOW         = 3,
   VOICE_DIAG_PROFILE            = 4,
   VOICE_DIAG_EPTAPP_HELP        = 5,
   VOICE_DIAG_EPTAPP_SHOW        = 6,
   VOICE_DIAG_EPTAPP_CREATECNX   = 7,
   VOICE_DIAG_EPTAPP_DELETECNX   = 8,
   VOICE_DIAG_EPTAPP_MODIFYCNX   = 9,
   VOICE_DIAG_EPTAPP_EPTSIG      = 10,
   VOICE_DIAG_EPTAPP_SET         = 11,
   VOICE_DIAG_EPTPROV            = 12,
   VOICE_DIAG_EPTAPP_DECTTEST    = 13,
   VOICE_DIAG_DECT_MEM_SET       = 14,
   VOICE_DIAG_DECT_MEM_GET       = 15,
   VOICE_DIAG_DECT_MODE_SET      = 16,
   VOICE_DIAG_DECT_MODE_GET      = 17,
   VOICE_DIAG_EPTPROBE           = 18,
   VOICE_DIAG_EPTAPP_VAS         = 19,
   VOICE_DIAG_EPTAPP_VRS         = 20,
   VOICE_DIAG_LINE_TEST_CMD      = 21,
} VoiceDiagType;

/** Data body for Voice diagonistic message */
typedef struct
{
  VoiceDiagType type;
  char cmdLine[BUFLEN_128];  
} VoiceDiagMsgBody;

/** Data body for CMS_MSG_PING_DATA message type.
 *
 */
typedef struct
{
   char diagnosticsState[BUFLEN_32];  /**< Ping state: requested, none, completed... */
   char interface[BUFLEN_32];   /**< interface on which ICMP request is sent */
   char host[BUFLEN_32]; /**< Host -- either IP address form or hostName to send ICMP request to */
   UINT32 numberOfRepetitions; /**< Number of ICMP requests to send */
   UINT32    timeout;	/**< Timeout in seconds */
   UINT32    dataBlockSize;	/**< DataBlockSize  */
   UINT32    DSCP;	/**< DSCP */
   UINT32    successCount;	/**< SuccessCount */
   UINT32    failureCount;	/**< FailureCount */
   UINT32    averageResponseTime;	/**< AverageResponseTime */
   UINT32    minimumResponseTime;	/**< MinimumResponseTime */
   UINT32    maximumResponseTime;	/**< MaximumResponseTime */
   CmsEntityId requesterId;
}PingDataMsgBody;


/** Data body for CMS_MSG_TRACERT_STATE_CHANGED
 *
 */
typedef struct
{
   char diagnosticsState[BUFLEN_32];    /**< Traceroute state: requested, comleted, error... */
   char hostAddrOfRouteHop[BUFLEN_64];  /**< Host address of route hop */
   char hostOfRouteHop[BUFLEN_64];      /**< Host name of route hop */
   char rtTimes[BUFLEN_32];             /**< route trip times in milliseconds (one for each repetition) for this hop. */
   UINT32 routeHopIndex;                /**< The index of hop when trace routing */
   UINT32 sampleCount;                  /**< The number of success sample for recent hop */
   UINT32 errorCode;                    /**< Error code returned from the ICMP CODE field */
   UINT32 averageResponseTime;	       /**< AverageResponseTime */
} TracertDataMsgBody;


/** Data body for the CMS_MSG_WATCH_WAN_CONNECTION message type.
 *
 */
 typedef struct
{
   MdmObjectId oid;              /**< Object Identifier */
   InstanceIdStack iidStack;     /**< Instance Id Stack. for the ip/ppp Conn Object */
   UBOOL8 isAdd;                 /**< add  wan connection to ssk list if TRUE.  */
   UBOOL8 isStatic;              /**< If TRUE, it is is bridge, static IPoE and IPoA, FALSE: pppox or dynamic IPoE */   
   UBOOL8 isDeleted;             /**< Used for auto detect feature only.  If TRUE, the wan interface is removed.*/  
   UBOOL8 isAutoDetectChange;    /**< Used for auto detect feature only.  If TRUE, there is a change on the layer 2 auto detect flag */  
} WatchedWanConnection;


/** Data Body for CMS_MSG_FIREWALL_CTL.
 *  This is currently only used in BDK by the voice app to request firewall
 *  hole to firewalld, but in theory could be used any any app in any
 *  component.
 */
#define FIREWALL_CTL_STRMAXSIZE   128

typedef struct
{
   UINT32   enable;                          /* Enable this firewall rule (filter) */
   char     name[FIREWALL_CTL_STRMAXSIZE];   /* name of this firewall rule (filter) */
   char     intfName[CMS_IFNAME_LENGTH];  /* name of interface to apply this. */
   char     protocol[FIREWALL_CTL_STRMAXSIZE];   /* either "TCP" or "UDP".  Not supported?: "TCP or UDP" */
   UBOOL8   isIpv6;                      /* by default, ipv4 */
   UINT32   sourcePort;
   UINT32   destinationPort;
   char     sourceIPAddress[FIREWALL_CTL_STRMAXSIZE];
   char     sourceNetMask[FIREWALL_CTL_STRMAXSIZE];
   char     destinationIPAddress[FIREWALL_CTL_STRMAXSIZE];
   char     destinationNetMask[FIREWALL_CTL_STRMAXSIZE];
} FirewallCtlMsgBody;


/** Data body for the CMS_MSG_INTFSTACK_LOWERLAYERS_CHANGED message type.
 *
 */
typedef struct
{
  MdmObjectId oid;              /**< OID of the higher layer interface */
  InstanceIdStack iidStack;     /**< iidStack of the higher layer interface */
  char deltaLowerLayers[MDM_MULTI_FULLPATH_BUFLEN+32];  /**< a specially
                    formated comma separated string of LowerLayers as created
                    by cmsUtl_diffFullPathInCSL_dev2 and understood by ssk
                    intfstack code. */
} IntfStackLowerLayersChangedMsgBody;


/** Data body for the CMS_MSG_INTFSTACK_ALIAS_CHANGED message type.
 *
 */
typedef struct
{
  MdmObjectId oid;              /**< OID of the Alias that changed */
  InstanceIdStack iidStack;     /**< iidStack of the Alias that changed */
} IntfStackAliasChangedMsgBody;


/** Data body for the CMS_MSG_INTFSTACK_OBJECT_DELETED message type.
 *
 */
typedef struct
{
  MdmObjectId oid;              /**< OID of deleted object */
  InstanceIdStack iidStack;     /**< iidStack of the deleted object */
} IntfStackObjectDeletedMsgBody;


/** Data body for the CMS_MSG_INTFSTACK_STATIC_ADDRESS_CONFIG message type.
 *
 */
typedef struct
{
  char ifName[CMS_IFNAME_LENGTH]; /**< The interface that static address is configured */
  UBOOL8 isIPv4;                  /**< is it an IPv4 address (else IPv6) */
  UBOOL8 isAdd;                   /**< new static address added */
  UBOOL8 isMod;                   /**< existing static address modified */
  UBOOL8 isDel;                   /**< static address deleted */
} IntfStackStaticAddressConfig;



/** Data body for the CMS_MSG_INTFSTACK_PROPAGATE_STATUS message type.
 *
 */
typedef struct
{
  char ipLowerLayerFullPath[MDM_SINGLE_FULLPATH_BUFLEN]; /**< The lowerlayer fullpath of the ip interface */
} IntfStackPropagateStaus;


/** Data body for the CMS_MSG_INTFSTACK_PROPAGATE_STATUS_EX message type.
 *  This is a more flexible version of CMS_MSG_INTFSTACK_PROPAGATE_STATUS.
 */
#define PROPAGATE_FLAG_LOWERLAYER_STATUS    0x0001

typedef struct
{
  UINT32 flags;   /**< See the PROPAGATE_FLAG_XXX above */
  char higherLayerFullPath[MDM_SINGLE_FULLPATH_BUFLEN]; /**< meaning depends on PROPAGATE_FLAG_XXX */
  char lowerLayerFullPath[MDM_SINGLE_FULLPATH_BUFLEN];  /**< meaning depends on PROPAGATE_FLAG_XXX */
  char data[128];               /**< meaning depends on PROPAGATE_FLAG_XXX */
} IntfStackPropagateStatusExMsgBody;


/*
 * Data body for the CMS_MSG_DHCPD_DENY_VENDOR_ID message type
 */
typedef struct
{
   unsigned char chaddr[16]; /* Usually the MAC address */
   char vendor_id[BUFLEN_256]; /**< max length in RFC 2132 is 255 bytes, add 1 for null terminator */
   char ifName[CMS_IFNAME_LENGTH];  /**< The interface that dhcpd got this msg on */
}DHCPDenyVendorID;

/*
 * Data body for CMS_MSG_GET_DEVICE_INFO message type.
 */
typedef struct
{
   char oui[BUFLEN_8];              /**< manufacturer OUI of device */
   char serialNum[BUFLEN_64];       /**< serial number of device */
   char productClass[BUFLEN_64];    /**< product class of device */
} GetDeviceInfoMsgBody;


typedef enum 
{
   USER_REQUEST_CONNECTION_DOWN  = 0,
   USER_REQUEST_CONNECTION_UP    = 1
} UserConnectionRequstType;


/*
 * Data body for CMS_MSG_WATCH_DSL_LOOP_DIAG message type.
 */
typedef struct
{
   InstanceIdStack iidStack;
} dslDiagMsgBody;

/** Data body for CMS_MSG_VENDOR_CONFIG_UPDATE message type.
 *
 */
typedef struct
{
   char name[BUFLEN_64];              /**< name of configuration file */
   char version[BUFLEN_16];           /**< version of configuration file */
   char date[BUFLEN_64];              /**< date when config is updated */
   char description[BUFLEN_256];      /**< description of config file */
} vendorConfigUpdateMsgBody;

typedef enum
{
    MCAST_INTERFACE_ADD = 0,
    MCAST_INTERFACE_DELETE
} McastInterfaceAction;

typedef enum
{
   LinkStatusLinkDisabled = 0,
   LinkStatusLinkInitializing,
   LinkStatusLinkEstablishingLink,
   LinkStatusLinkNoSignal,
   LinkStatusLinkError,
   LinkStatusLinkUp
} LinkStatus_T;

/** Data body for message types :
 *  CMS_MSG_EPON_LINK_STATUS_CHANGED
 *  CMS_MSG_EPON_GET_LINK_STATUS
 * Enum value : 
 * Up = 1
 * Down = 2
 * 
 */
typedef enum
{
    EPON_LINK_STATUS_START = 0,
    EPON_LINK_STATUS_UP = 1,
    EPON_LINK_STATUS_DOWN = 2,
} EponLinkStatusType;

#ifndef EPON_LINK_MAX_ADD_DATA_LEN
#define EPON_LINK_MAX_ADD_DATA_LEN     128
#endif

typedef struct {
   char  l2Ifname[CMS_IFNAME_LENGTH];
   EponLinkStatusType linkStatus;
   SINT32 vlanId;
   SINT32 pbits;
   UINT32 link;
   UINT32 additionalDataLen;
} EponLinkStatusMsgBody;

/** Data body for CMS_MSG_APP_TERMINATED message type.
 *
 */
typedef struct
{
   CmsEntityId eid;      /**< Entity id of the exit process */
   SINT32 sigNum;        /**< signal number */   
   SINT32 exitCode;      /**< process exit code */   
} appTermiatedMsgBody;

/** Data body for the reply of CMS_MSG_DNSPROXY_GET_STATS
 *
 */
 typedef struct
{
   UINT32 dnsErrors; 	      /**< dns query error counter  */
} DnsGetStatsMsgBody;


/** Data body for the CMS_MSG_XMPP_CONNECTION_ENABLE message type.
 *
 */
typedef struct
{
   UBOOL8 enable;  /**< Enable/Disable XMPP Connection */
   char jabberID[BUFLEN_1024]; /**< XMPP Connection jabberID */
   char password[BUFLEN_256]; /**< XMPP Connection Password */
   char serverAddress[BUFLEN_256]; /**< XMPP server address */
   UINT32 serverPort;              /**< XMPP server port */
   UBOOL8 useTLS;  /**< use TLS or not*/
   SINT32 keepAlive;
} XmppConnMsgBody;


/*!\TIME state defines 
 */
#define TIME_STATE_DISABLED                0   
#define TIME_STATE_UNSYNCHRONIZED          1   
#define TIME_STATE_SYNCHRONIZED            2
#define TIME_STATE_FAIL_TO_SYNCHRONIZE     3
#define TIME_STATE_ERROR                   4

/** Data body for CMS_MSG_AUTONOMOUS_TRANSFER_COMPLETE message type.
 *
 */
typedef struct
{
   UBOOL8 isDownload;
   UINT8 fileType;
   UINT32 fileSize;
   UINT32 faultCode;
   char faultStr[BUFLEN_64]; 
   UINT32 startTime;
   UINT32 completeTime;
} AutonomousTransferCompleteMsgBody;

/** Data body for CMS_MSG_EPON_SET_ACS message type.
 *
 */
typedef struct
{
   char acsURL[CMS_MAX_ACS_URL_LENGTH];      /**< acs url */
   char acsProvisioningCode[CMS_MAX_ACS_PROVISIONING_CODE_LENGTH];
   UINT32 cwmpRetryMinimumWaitInterval;
   UINT32 cwmpRetryIntervalMultiplier;
} EponSetAcsMsgBody;

/** Data body for CMS_MSG_EPON_GET_WAN_CONFIG/CMS_MSG_EPON_SET_WAN_CONFIG message type.
 *
 */
typedef struct
{
   char ifName[CMS_IFNAME_LENGTH];
   char alias[CMS_IFNAME_LENGTH];
   int isBridge;
   int IPv4Enable;
   int IPv6Enable;
} EponeWanIntfCfgMsgBody;

/*
 * Data body for CMS_MSG_WAN_PORT_SET_OPSTATE message type.
 */

typedef enum
{
    WANCONF_PHY_TYPE_AE = 0,
    WANCONF_PHY_TYPE_GPON,
    WANCONF_PHY_TYPE_EPON,
    WANCONF_PHY_TYPE_MAX
} WanConfPhyType;

typedef struct
{
   WanConfPhyType phyType;
   UBOOL8 opState;
} WanConfPhyOpStateMsgBody;

typedef enum
{
    MDM_POST_ACT_TYPE_FILTER = 0,
    MDM_POST_ACT_TYPE_MAX
} MdmPostActSubType;

/** Data body for the CMS_MSG_MDM_POST_ACTIVATING message type.
 *
 */
typedef struct
{
   MdmPostActSubType subType;              /**< SubType to point MDM node */
} MdmPostActNodeInfo;

typedef enum
{
    LXC_STATUS_STOPPED = 0,
    LXC_STATUS_STARTING,
    LXC_STATUS_RUNNING,
    LXC_STATUS_STOPPING,
    LXC_STATUS_ABORTING,
    LXC_STATUS_FREEZING,
    LXC_STATUS_FROZEN,
    LXC_STATUS_THAWED,
    LXC_STATUS_MAX
} LxcContainerState;

/** Data body for CMS_MSG_LXC_STATUS_INFO message type.
 *
 */
typedef struct
{
   char name[BUFLEN_32];
   LxcContainerState state;
} LxcContainerStatusMsgBody;

typedef enum {
   CMSMSGVOICEINTERNAL_IF_CHECK,               /* Check interface */
   CMSMSGVOICEINTERNAL_PROV_COMPLETE,          /* Provisioning complete */
#ifdef BRCM_PKTCBL_SUPPORT
   CMSMSGVOICEINTERNAL_PCSCFDISCOVERY_COMPLETE,/* PCSCF Discovery complete */
#endif /* BRCM_PKTCBL_SUPPORT */
   CMSMSGVOICEINTERNAL_CMGR_START_SUCCESS,     /* Call manager start success */
   CMSMSGVOICEINTERNAL_CMGR_START_FAIL,        /* Call manager start failure */
   CMSMSGVOICEINTERNAL_CMGR_STOP_COMPLETE,     /* Call manager stop completed */
} CmsMsgVoiceInternalEvent;

typedef enum {
   CMSMSGVOICECONFIGCHANGED_GENERAL,           /* General configuration change */
   CMSMSGVOICECONFIGCHANGED_BOUNDIF,           /* Bound IF configuration change */
   CMSMSGVOICECONFIGCHANGED_MGTPROT,           /* Management protocol configuration change */
   CMSMSGVOICECONFIGCHANGED_GLOBLOGLEVEL,      /* Global Log level configuration change */
   CMSMSGVOICECONFIGCHANGED_MODLOGLEVEL,       /* Module Log level configuration change */
   CMSMSGVOICECONFIGCHANGED_ROUTING,           /* Routing related configuration change */
   CMSMSGVOICECONFIGCHANGED_REGSTATUS,         /* Registration related status change */
   CMSMSGVOICECONFIGCHANGED_CLEARMWI,          /* Clear MWI set, data = netInst */
   CMSMSGVOICECONFIGCHANGED_SETMWI,            /* Set MWI set, data = netInst */
   CMSMSGVOICECONFIGCHANGED_SVCRESTART_MWI,    /* Restart MWI service, data = netInst */
} CmsMsgVoiceConfigChanged;

typedef struct
{
    UINT32    packetsSent;  /**< PacketsSent */
    UINT32    packetsReceived;  /**< PacketsReceived */
    UINT32    bytesSent;    /**< BytesSent */
    UINT32    bytesReceived;    /**< BytesReceived */
    UINT32    packetsLost;  /**< PacketsLost */
    UINT32    receiveInterarrivalJitter;    /**< ReceiveInterarrivalJitter */
    UINT32    roundTripDelay;   /**< RoundTripDelay */
    UINT32    averageReceiveInterarrivalJitter; /**< AverageReceiveInterarrivalJitter */
    UINT32    peakReceiveInterarrivalJitter; /**< PeakReceiveInterarrivalJitter */
    UINT32    incomingCallsReceived;    /**< IncomingCallsReceived */
    UINT32    incomingCallsAnswered;    /**< IncomingCallsAnswered */
    UINT32    incomingCallsConnected;   /**< IncomingCallsConnected */
    UINT32    incomingCallsFailed;  /**< IncomingCallsFailed */
    UINT32    outgoingCallsAttempted;   /**< OutgoingCallsAttempted */
    UINT32    outgoingCallsAnswered;    /**< OutgoingCallsAnswered */
    UINT32    outgoingCallsConnected;   /**< OutgoingCallsConnected */
    UINT32    outgoingCallsFailed;  /**< OutgoingCallsFailed */
    UINT32    incomingCallsDropped;
    UINT32    incomingTotalCallTime;  /* In seconds */
    UINT32    outgoingCallsDropped;
    UINT32    outgoingTotalCallTime;  /* In seconds */
    UINT32    jbUnderruns;            /* Jitter buffer underrun count */
    UINT32    jbOverruns;             /* Jitter buffer overrun count */
} CmsMsgVoiceStatisticsResponse;

#ifdef BRCM_PKTCBL_SUPPORT
typedef enum {
   CMSMSGSNMPDNOTIF_PKTCMTADEVPROVISIONINGENROLLMENT,  /* pktcMtaDevProvisioningEnrollment */
   CMSMSGSNMPDNOTIF_PKTCMTADEVPROVISIONINGSTATUS,      /* pktcMtaDevProvisioningStatus */
   CMSMSGSNMPDNOTIF_PKTCDEVEVINFORM,                   /* pktcDevEvInform */
   CMSMSGSNMPDNOTIF_PKTCDEVEVTRAP,                     /* pktcDevEvTrap */
} CmsMsgSnmpdNotif;

typedef enum {
   CMSMSGVOICEPKTCBLEVENT_RESETEVENTLOGTABLE,          /* pktcDevEvControl - resetEventLogTable */
   CMSMSGVOICEPKTCBLEVENT_RESETEVENTDESCRTABLE,        /* pktcDevEvControl - resetEventDescrTable */
} CmsMsgVoicePktcblEvent;
#endif /* BRCM_PKTCBL_SUPPORT */

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize messaging system.  This is the preferred initialization call
 * because it is more efficient.
 *
 * This function should be called early in startup.
 *
 * @param eid       (IN)  Entity id of the calling process.
 * @param flags     (IN)  Only EIF_MULTIPLE_INSTANCES is detected, other bits are ignored.
 * @param msgHandle (OUT) On successful return, this will point
 *                        to a msg_handle which should be used in subsequent messaging calls.
 *                        The caller is responsible for freeing the msg_handle by calling
 *                        cmsMsg_cleanup().
 *
 * @return CmsRet enum.
 */
CmsRet cmsMsg_initWithFlags(CmsEntityId eid, UINT32 flags, void **msgHandle);


/** Initialize messaging system.  If possible, use cmsMsg_initWithFlags().
 *
 * This function should be called early in startup.
 * 
 * @param eid       (IN)  Entity id of the calling process.
 * @param msgHandle (OUT) On successful return, this will point
 *                        to a msg_handle which should be used in subsequent messaging calls.
 *                        The caller is responsible for freeing the msg_handle by calling
 *                        cmsMsg_cleanup().
 *
 * @return CmsRet enum.
 */
CmsRet cmsMsg_init(CmsEntityId eid, void **msgHandle);


/** Initialize messaging system on the specified message bus.  Must use this
 *  to initialize connection to a message bus in a Distributed MDM component.
 *
 * @param eid       (IN)  Entity id of the calling process.
 * @param flags     (IN)  Only EIF_MULTIPLE_INSTANCES is detected, other bits are ignored.
 * @param busName   (IN)  SMD_MESSAGE_ADDR or *_MSG_BUS
 * @param msgHandle (OUT) On successful return, this will point
 *                        to a msg_handle which should be used in subsequent messaging calls.
 *                        The caller is responsible for freeing the msg_handle by calling
 *                        cmsMsg_cleanup().
 *
 * @return CmsRet enum.
 */
CmsRet cmsMsg_initOnBus(CmsEntityId eid, UINT32 flags, const char *busName,
                        void **msgHandle);


/** Clean up messaging system.
 *
 * This function should be called before the application exits.
 * @param msgHandle (IN) This was the msg_handle that was
 *                       created by cmsMsg_init().
 */
void cmsMsg_cleanup(void **msgHandle);


/** Send a message (blocking).
 *
 * This call is potentially blocking if the communcation channel is
 * clogged up, but in practice, it will not block.  If blocking becomes
 * a real problem, we can create a non-blocking version of this function.
 *
 * @param msgHandle (IN) This was the msgHandle created by
 *                       cmsMsg_init().
 * @param buf       (IN) This buf contains a CmsMsgHeader and possibly
 *                       more data depending on the message type.
 *                       The caller must fill in all the fields.
 *                       The total length of the message is the length
 *                       of the message header plus any additional data
 *                       as specified in the dataLength field in the CmsMsgHeader.
 * @return CmsRet enum.
 */
CmsRet cmsMsg_send(void *msgHandle, const CmsMsgHeader *buf);


/** Send a reply/response message to the given request message.
 *
 * Same notes about blocking from cmsMsg_send() apply.
 * Note that only a message header will be sent by this
 * function.  If the initial request message contains additional
 * data, this function will not send that data back.
 *
 * @param msgHandle (IN) This was the msgHandle created by
 *                       cmsMsg_init().
 * @param msg       (IN) The request message that we want to send
 *                       a response to.  This function does not modify
 *                       or free this message.  Caller is still required
 *                       to deal with it appropriately.
 * @param retCode   (IN) The return code to put into the wordData
 *                       field of the return message.
 * @return CmsRet enum.
 */
CmsRet cmsMsg_sendReply(void *msgHandle, const CmsMsgHeader *msg, CmsRet retCode);


/** Send a message and wait for a simple response.
 *
 * This function starts out by calling cmsMsg_send().
 * Then it waits for a response.  The result of the response is expected in
 * the wordData field of the response message.  The value in the wordData is
 * returned to the caller.  The response message must not have any additional
 * data after the header.  The response message is freed by this function.
 *
 * @param msgHandle (IN) This was the msgHandle created by
 *                       cmsMsg_init().
 * @param buf       (IN) This buf contains a CmsMsgHeader and possibly
 *                       more data depending on the message type.
 *                       The caller must fill in all the fields.
 *                       The total length of the message is the length
 *                       of the message header plus any additional data
 *                       as specified in the dataLength field in the CmsMsgHeader.
 *
 * @return CmsRet enum, which is either the response code from the recipient of the msg,
 *                      or a local error.
 */
CmsRet cmsMsg_sendAndGetReply(void *msgHandle, const CmsMsgHeader *buf);


/** Send a message and wait up to a timeout time for a simple response.
 *
 * This function is the same as cmsMsg_sendAndGetReply() except there
 * is a limit, expressed as a timeout, on how long this function will
 * wait for a response.
 *
 * @param msgHandle (IN) This was the msgHandle created by
 *                       cmsMsg_init().
 * @param buf       (IN) This buf contains a CmsMsgHeader and possibly
 *                       more data depending on the message type.
 *                       The caller must fill in all the fields.
 *                       The total length of the message is the length
 *                       of the message header plus any additional data
 *                       as specified in the dataLength field in the CmsMsgHeader.
 * @param timeoutMilliSeconds (IN) Timeout in milliseconds.
 *
 * @return CmsRet enum, which is either the response code from the recipient of the msg,
 *                      or a local error.
 */
CmsRet cmsMsg_sendAndGetReplyWithTimeout(void *msgHandle,
                                         const CmsMsgHeader *buf,
                                         UINT32 timeoutMilliSeconds);


/** Send a message and wait up to a timeout time for a response that can
 *	have a data section.
 *
 * This function is the same as cmsMsg_sendAndGetReply() except this
 * returns a cmsMsgHeader and a data section if applicable.
 *
 * @param msgHandle (IN) This was the msgHandle created by
 *                       cmsMsg_init().
 * @param buf       (IN) This buf contains a CmsMsgHeader and possibly
 *                       more data depending on the message type.
 *                       The caller must fill in all the fields.
 *                       The total length of the message is the length
 *                       of the message header plus any additional data
 *                       as specified in the dataLength field in the CmsMsgHeader.
 * @param replyBuf (IN/OUT) On entry, replyBuf can either be the address of a pointer to
 *                       a buffer that will hold the reply message.
 *                       The caller must allocate enough space in the replyBuf
 *                       to hold the message header and any data that might
 *                       come back in the reply message.  (This is a dangerous
 *                       interface!  This function does not verify that the
 *                       caller has allocated enough space to hold the reply
 *                       message.  Memory corruption will occur if the reply
 *                       message contains more data than the caller has
 *                       allocated.  Note there is also no reason for this
 *                       parameter to be address of pointer, a simple pointer
 *                       to replyBuf would have been sufficient.)
 *                       replyBuf can also be the address of a NULL pointer. In
 *                       this case, this API will allocate buffer and puts its
 *                       address to the pointer pointed to by replyBuf. The caller
 *                       is responsible to free the buffer afterwards.
 *                       On successful return, replyBuf will point to a
 *                       CmsMsgHeader possibly followed by more data if the
 *                       reply message contains a data section.
 *
 * @return CmsRet enum, which is either the response code from the recipient of the msg,
 *                      or a local error.
 */
CmsRet cmsMsg_sendAndGetReplyBuf(void *msgHandle,
                                 const CmsMsgHeader *buf,
                                 CmsMsgHeader **replyBuf);


/** Send a message and wait up to a timeout time for a response that can
 *	have a data section.
 *
 * This function is the same as cmsMsg_sendAndGetReplyBuf() except there
 * is a limit, expressed as a timeout, on how long this function will
 * wait for a response.
 *
 * @param msgHandle (IN) This was the msgHandle created by
 *                       cmsMsg_init().
 * @param buf       (IN) This buf contains a CmsMsgHeader and possibly
 *                       more data depending on the message type.
 *                       The caller must fill in all the fields.
 *                       The total length of the message is the length
 *                       of the message header plus any additional data
 *                       as specified in the dataLength field in the CmsMsgHeader.
 * @param replyBuf (IN/OUT) On entry, replyBuf is the address of a pointer to
 *                       a buffer that will hold the reply message.
 *                       The caller must allocate enough space in the replyBuf
 *                       to hold the message header and any data that might
 *                       come back in the reply message.  (This is a dangerous
 *                       interface!  This function does not verify that the
 *                       caller has allocated enough space to hold the reply
 *                       message.  Memory corruption will occur if the reply
 *                       message contains more data than the caller has
 *                       allocated.  Note there is also no reason for this
 *                       parameter to be address of pointer, a simple pointer
 *                       to replyBuf would have been sufficient.)
 *                       replyBuf can also be the address of a NULL pointer. In
 *                       this case, this API will allocate buffer and puts its
 *                       address to the pointer pointed to by replyBuf. The caller
 *                       is responsible to free the buffer afterwards.
 *                       On successful return, replyBuf will point to a
 *                       CmsMsgHeader possibly followed by more data if the
 *                       reply message contains a data section.
 *
 * @param timeoutMilliSeconds (IN) Timeout in milliseconds.
 *
 * @return CmsRet enum, which is either the response code from the recipient of the msg,
 *                      or a local error.
 */
CmsRet cmsMsg_sendAndGetReplyBufWithTimeout(void *msgHandle,
                                            const CmsMsgHeader *buf,
                                            CmsMsgHeader **replyBuf,
                                            UINT32 timeoutMilliSeconds);


/**This api should only be called after the requset CMS message has been sent.
 * it is intended to provide higher level of control for the calling process.
 * for example, if the calling process wants to enforce that the sequenceNumber
 * in replyBuf should match the sequenceNumber in request.
 * Since "sequenceNumber" is an optional field in CmsMsgHeader, the check for
 * its match is done outside of cms msg library itself.
 *
 * @param msgHandle (IN) This was the msgHandle created by
 *                       cmsMsg_init().
 * @param buf       (IN) This buf contains a CmsMsgHeader and possibly
 *                       more data depending on the message type.
 *                       The caller must fill in all the fields.
 *                       The total length of the message is the length
 *                       of the message header plus any additional data
 *                       as specified in the dataLength field in the CmsMsgHeader.
 * @param replyBuf (IN/OUT) On entry, replyBuf is the address of a pointer to
 *                       a buffer that will hold the reply message.
 *                       The caller must allocate enough space in the replyBuf
 *                       to hold the message header and any data that might
 *                       come back in the reply message.  (This is a dangerous
 *                       interface!  This function does not verify that the
 *                       caller has allocated enough space to hold the reply
 *                       message.  Memory corruption will occur if the reply
 *                       message contains more data than the caller has
 *                       allocated.  Note there is also no reason for this
 *                       parameter to be address of pointer, a simple pointer
 *                       to replyBuf would have been sufficient.)
 *                       replyBuf can also be the address of a NULL pointer. In
 *                       this case, this API will allocate buffer and puts its
 *                       address to the pointer pointed to by replyBuf. The caller
 *                       is responsible to free the buffer afterwards.
 *                       On successful return, replyBuf will point to a
 *                       CmsMsgHeader possibly followed by more data if the
 *                       reply message contains a data section.
 *
 * @param timeoutMilliSeconds (IN) Timeout in milliseconds.
 *
 * @return CmsRet enum, which is either the response code from the recipient of the msg,
 *                      or a local error.
 */
CmsRet cmsMsg_getReplyBufWithTimeout(void *msgHandle, const CmsMsgHeader *buf,
                          CmsMsgHeader **replyBuf, UINT32 timeoutMilliSeconds);

/** Receive a message (blocking).
 *
 * This call will block until a message is received.
 * @param msgHandle (IN) This was the msgHandle created by cmsMsg_init().
 * @param buf      (OUT) On successful return, buf will point to a CmsMsgHeader
 *                       and possibly followed by more data depending on msg type.
 *                       The caller is responsible for freeing the message by calling
 *                       cmsMsg_free().
 * @return CmsRet enum.
 */
CmsRet cmsMsg_receive(void *msgHandle, CmsMsgHeader **buf);


/** Receive a message with timeout.
 *
 * This call will block until a message is received or until the timeout is reached.
 *
 * @param msgHandle (IN) This was the msgHandle created by cmsMsg_init().
 * @param buf      (OUT) On successful return, buf will point to a CmsMsgHeader
 *                       and possibly followed by more data depending on msg type.
 *                       The caller is responsible for freeing the message by calling
 *                       cmsMsg_free().
 * @param timeoutMilliSeconds (IN) Timeout in milliseconds.  0 means do not block,
 *                       otherwise, block for the specified number of milliseconds.
 * @return CmsRet enum.
 */
CmsRet cmsMsg_receiveWithTimeout(void *msgHandle,
                                 CmsMsgHeader **buf,
                                 UINT32 timeoutMilliSeconds);


/** Put a received message back into a temporary "put-back" queue.
 *
 * Since the RCL calls cmsMsg_receive, it may get an asynchronous event
 * message that is intended for the higher level application.  So it needs
 * to preserve the message in the msgHandle so the higher level application
 * can detect and receive it.  This happens in two steps: first the message
 * is put in a temporary "put-back" queue in the msgHandle (this function),
 * and then CMS_MSG_INTERNAL_NOOP is sent to the app (by cmsMsg_sendNoop).
 * The CMS_MSG_INTERNAL_NOOP will cause select to wake the app up, but
 * when the app calls one of the receive API's to receive a message,
 * the messages from the putback queue will be returned first.  This prevents
 * out of order packet delivery.
 *
 * @param msgHandle (IN) This was the msgHandle created by cmsMsg_init().
 * @param buf       (IN) The message to put back.
 */
void cmsMsg_putBack(void *msgHandle, CmsMsgHeader **buf);


/** Send an internal NOOP message.  Used in conjunction with cmsMsg_putBack.
 *
 * This message will cause the receiving app to wake up from select.
 * Applications which receive this message can simply ignore it and free it.
 */
void cmsMsg_sendNoop(void *msgHandle);


/** Make a copy of the specified message, including any additional data beyond the header.
 *
 * @param  buf      (IN) The message to copy.
 * @return duplicate of the specified message.
 */
CmsMsgHeader *cmsMsg_duplicate(const CmsMsgHeader *buf);



/** Get operating system dependent handle to detect available message to receive.
 *
 * This allows the application to get the operating system dependent handle
 * to detect a message that is available to be received so it can wait on the handle along
 * with other private event handles that the application manages.
 * In UNIX like operating systems, this will return a file descriptor
 * which the application can then use in select.
 * 
 * @param msgHandle    (IN) This was the msgHandle created by cmsMsg_init().
 * @param eventHandle (OUT) This is the OS dependent event handle.  For LINUX,
 *                          eventHandle is the file descriptor number.
 * @return CmsRet enum.
 */
CmsRet cmsMsg_getEventHandle(const void *msgHandle, void *eventHandle);


/** Get the eid of the creator of this message handle.
 * 
 * This function is used by the CMS libraries which are given a message handle
 * but needs to find out who the message handle belongs to.
 * 
 * @param msgHandle    (IN) This was the msgHandle created by cmsMsg_init().
 * 
 * @return CmsEntityId of the creator of the msgHandle.
 */
CmsEntityId cmsMsg_getHandleEid(const void *msgHandle);

/** Get the busName of this message handle.
 * 
 * This function is used by the CMS libraries which are given a message handle
 * but needs to find out the busName.
 * 
 * @param msgHandle    (IN) This was the msgHandle created by cmsMsg_init().
 * 
 * @return a const char * which points to the busName.
 */
const char *cmsMsg_getBusName(const void *msgHandle);


/** Check if CMS message service is ready
 *
 * This function is used by applications which might launch before
 * smd (which implements message routing).  They will need to wait for
 * CMS message service to become ready before calling cmsMsg_initWithFlags
 * or cmsMsg_init.  This function is not needed by apps launched by smd
 * in the normal CMS way.
 *
 * @return TRUE if message service is ready.  FALSE otherwise.
 */
UBOOL8 cmsMsg_isServiceReady(void);


/** Create the main unix domain socket for a message bus (used by BDK).
 *
 * @param msgBusName (IN) SMD_MESSAGE_ADDR or *_MSG_BUS
 * @param backlog    (IN) *_MESSAGE_BACKLOG
 *
 * @returns the fd on success, -1 on failure.
 */
SINT32 cmsMsg_initUnixDomainServerSocket(const char *msgBusName, SINT32 backlog);


/** Given the name of a Distributed MDM component, return the BCM MSG BUS name.
 *
 * @param componentName (IN) e.g. devinfo, dsl, gpon, wifi.
 *
 * @return BCM_MSG_BUS name, or NULL if the component name is not recognized or
 *         supported.
 */
const char *cmsMsg_componentNameToBusName(const char *compName);

/** Given the name of a CMS message bus, return the name of distributed MDM component.
 *
 * @param busName (IN) e.g. DSL_MSG_BUS
 *
 * @return component name, or NULL if the bus name is not recognized or
 *         supported.
 */
const char *cmsMsg_busNameToComponentName(const char *busName);

#ifdef __cplusplus
} /* end of extern "C" */
#endif

#endif // __CMS_MSG_H__
