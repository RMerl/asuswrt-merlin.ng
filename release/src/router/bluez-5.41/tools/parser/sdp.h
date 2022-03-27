/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2001-2002  Ricky Yuen <ryuen@qualcomm.com>
 *  Copyright (C) 2003-2011  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef __SDP_H
#define __SDP_H

/* Bluetooth assigned UUIDs for protocols */
#define SDP_UUID_SDP                                   0x0001
#define SDP_UUID_UDP                                   0x0002
#define SDP_UUID_RFCOMM                                0x0003
#define SDP_UUID_TCP                                   0x0004
#define SDP_UUID_TCS_BIN                               0x0005
#define SDP_UUID_TCS_AT                                0x0006
#define SDP_UUID_OBEX                                  0x0008
#define SDP_UUID_IP                                    0x0009
#define SDP_UUID_FTP                                   0x000A
#define SDP_UUID_HTTP                                  0x000C
#define SDP_UUID_WSP                                   0x000E
#define SDP_UUID_BNEP                                  0x000F /* PAN */
#define SDP_UUID_HIDP                                  0x0011 /* HID */
#define SDP_UUID_HARDCOPY_CONTROL_CHANNEL              0x0012 /* HCRP */
#define SDP_UUID_HARDCOPY_DATA_CHANNEL                 0x0014 /* HCRP */
#define SDP_UUID_HARDCOPY_NOTIFICATION                 0x0016 /* HCRP */
#define SDP_UUID_AVCTP                                 0x0017 /* AVCTP */
#define SDP_UUID_AVDTP                                 0x0019 /* AVDTP */
#define SDP_UUID_CMTP                                  0x001B /* CIP */
#define SDP_UUID_UDI_C_PLANE                           0x001D /* UDI */
#define SDP_UUID_L2CAP                                 0x0100

/* Bluetooth assigned UUIDs for Service Classes */
#define SDP_UUID_SERVICE_DISCOVERY_SERVER              0x1000
#define SDP_UUID_BROWSE_GROUP_DESCRIPTOR               0x1001
#define SDP_UUID_PUBLIC_BROWSE_GROUP                   0x1002
#define SDP_UUID_SERIAL_PORT                           0x1101
#define SDP_UUID_LAN_ACCESS_PPP                        0x1102
#define SDP_UUID_DIALUP_NETWORKING                     0x1103
#define SDP_UUID_IR_MC_SYNC                            0x1104
#define SDP_UUID_OBEX_OBJECT_PUSH                      0x1105
#define SDP_UUID_OBEX_FILE_TRANSFER                    0x1106
#define SDP_UUID_IR_MC_SYNC_COMMAND                    0x1107
#define SDP_UUID_HEADSET                               0x1108
#define SDP_UUID_CORDLESS_TELEPHONY                    0x1109
#define SDP_UUID_AUDIO_SOURCE                          0x110a /* A2DP */
#define SDP_UUID_AUDIO_SINK                            0x110b /* A2DP */
#define SDP_UUID_AV_REMOTE_TARGET                      0x110c /* AVRCP */
#define SDP_UUID_ADVANCED_AUDIO                        0x110d /* A2DP */
#define SDP_UUID_AV_REMOTE                             0x110e /* AVRCP */
#define SDP_UUID_AV_REMOTE_CONTROLLER                  0x110f /* AVRCP */
#define SDP_UUID_INTERCOM                              0x1110
#define SDP_UUID_FAX                                   0x1111
#define SDP_UUID_HEADSET_AUDIO_GATEWAY                 0x1112
#define SDP_UUID_WAP                                   0x1113
#define SDP_UUID_WAP_CLIENT                            0x1114
#define SDP_UUID_PANU                                  0x1115 /* PAN */
#define SDP_UUID_NAP                                   0x1116 /* PAN */
#define SDP_UUID_GN                                    0x1117 /* PAN */
#define SDP_UUID_DIRECT_PRINTING                       0x1118 /* BPP */
#define SDP_UUID_REFERENCE_PRINTING                    0x1119 /* BPP */
#define SDP_UUID_IMAGING                               0x111a /* BIP */
#define SDP_UUID_IMAGING_RESPONDER                     0x111b /* BIP */
#define SDP_UUID_IMAGING_AUTOMATIC_ARCHIVE             0x111c /* BIP */
#define SDP_UUID_IMAGING_REFERENCED_OBJECTS            0x111d /* BIP */
#define SDP_UUID_HANDSFREE                             0x111e
#define SDP_UUID_HANDSFREE_AUDIO_GATEWAY               0x111f
#define SDP_UUID_DIRECT_PRINTING_REF_OBJS              0x1120 /* BPP */
#define SDP_UUID_DIRECT_PRINTING_REFERENCE_OBJECTS     0x1120 /* BPP */
#define SDP_UUID_REFLECTED_UI                          0x1121 /* BPP */
#define SDP_UUID_BASIC_PRINTING                        0x1122 /* BPP */
#define SDP_UUID_PRINTING_STATUS                       0x1123 /* BPP */
#define SDP_UUID_HUMAN_INTERFACE_DEVICE                0x1124 /* HID */
#define SDP_UUID_HARDCOPY_CABLE_REPLACE                0x1125 /* HCRP */
#define SDP_UUID_HCR_PRINT                             0x1126 /* HCRP */
#define SDP_UUID_HCR_SCAN                              0x1127 /* HCRP */
#define SDP_UUID_COMMON_ISDN_ACCESS                    0x1128 /* CIP */
#define SDP_UUID_UDI_MT                                0x112a /* UDI */
#define SDP_UUID_UDI_TA                                0x112b /* UDI */
#define SDP_UUID_AUDIO_VIDEO                           0x112c /* VCP */
#define SDP_UUID_SIM_ACCESS                            0x112d /* SAP */
#define SDP_UUID_PHONEBOOK_ACCESS_PCE                  0x112e /* PBAP */
#define SDP_UUID_PHONEBOOK_ACCESS_PSE                  0x112f /* PBAP */
#define SDP_UUID_PHONEBOOK_ACCESS                      0x1130 /* PBAP */
#define SDP_UUID_PNP_INFORMATION                       0x1200
#define SDP_UUID_GENERIC_NETWORKING                    0x1201
#define SDP_UUID_GENERIC_FILE_TRANSFER                 0x1202
#define SDP_UUID_GENERIC_AUDIO                         0x1203
#define SDP_UUID_GENERIC_TELEPHONY                     0x1204
#define SDP_UUID_UPNP_SERVICE                          0x1205 /* ESDP */
#define SDP_UUID_UPNP_IP_SERVICE                       0x1206 /* ESDP */
#define SDP_UUID_ESDP_UPNP_IP_PAN                      0x1300 /* ESDP */
#define SDP_UUID_ESDP_UPNP_IP_LAP                      0x1301 /* ESDP */
#define SDP_UUID_ESDP_UPNP_L2CAP                       0x1302 /* ESDP */
#define SDP_UUID_VIDEO_SOURCE                          0x1303 /* VDP */
#define SDP_UUID_VIDEO_SINK                            0x1304 /* VDP */
#define SDP_UUID_VIDEO_DISTRIBUTION                    0x1305 /* VDP */
#define SDP_UUID_APPLE_AGENT                           0x2112

/* Bluetooth assigned numbers for Attribute IDs */
#define SDP_ATTR_ID_SERVICE_RECORD_HANDLE              0x0000
#define SDP_ATTR_ID_SERVICE_CLASS_ID_LIST              0x0001
#define SDP_ATTR_ID_SERVICE_RECORD_STATE               0x0002
#define SDP_ATTR_ID_SERVICE_SERVICE_ID                 0x0003
#define SDP_ATTR_ID_PROTOCOL_DESCRIPTOR_LIST           0x0004
#define SDP_ATTR_ID_BROWSE_GROUP_LIST                  0x0005
#define SDP_ATTR_ID_LANGUAGE_BASE_ATTRIBUTE_ID_LIST    0x0006
#define SDP_ATTR_ID_SERVICE_INFO_TIME_TO_LIVE          0x0007
#define SDP_ATTR_ID_SERVICE_AVAILABILITY               0x0008
#define SDP_ATTR_ID_BLUETOOTH_PROFILE_DESCRIPTOR_LIST  0x0009
#define SDP_ATTR_ID_DOCUMENTATION_URL                  0x000A
#define SDP_ATTR_ID_CLIENT_EXECUTABLE_URL              0x000B
#define SDP_ATTR_ID_ICON_URL                           0x000C
#define SDP_ATTR_ID_ADDITIONAL_PROTOCOL_DESC_LISTS     0x000D
#define SDP_ATTR_ID_SERVICE_NAME                       0x0100
#define SDP_ATTR_ID_SERVICE_DESCRIPTION                0x0101
#define SDP_ATTR_ID_PROVIDER_NAME                      0x0102
#define SDP_ATTR_ID_VERSION_NUMBER_LIST                0x0200
#define SDP_ATTR_ID_GROUP_ID                           0x0200
#define SDP_ATTR_ID_SERVICE_DATABASE_STATE             0x0201
#define SDP_ATTR_ID_SERVICE_VERSION                    0x0300

#define SDP_ATTR_ID_EXTERNAL_NETWORK                   0x0301 /* Cordless Telephony */
#define SDP_ATTR_ID_SUPPORTED_DATA_STORES_LIST         0x0301 /* Synchronization */
#define SDP_ATTR_ID_REMOTE_AUDIO_VOLUME_CONTROL        0x0302 /* GAP */
#define SDP_ATTR_ID_SUPPORTED_FORMATS_LIST             0x0303 /* OBEX Object Push */
#define SDP_ATTR_ID_FAX_CLASS_1_SUPPORT                0x0302 /* Fax */
#define SDP_ATTR_ID_FAX_CLASS_2_0_SUPPORT              0x0303
#define SDP_ATTR_ID_FAX_CLASS_2_SUPPORT                0x0304
#define SDP_ATTR_ID_AUDIO_FEEDBACK_SUPPORT             0x0305
#define SDP_ATTR_ID_SECURITY_DESCRIPTION               0x030a /* PAN */
#define SDP_ATTR_ID_NET_ACCESS_TYPE                    0x030b /* PAN */
#define SDP_ATTR_ID_MAX_NET_ACCESS_RATE                0x030c /* PAN */
#define SDP_ATTR_ID_IPV4_SUBNET                        0x030d /* PAN */
#define SDP_ATTR_ID_IPV6_SUBNET                        0x030e /* PAN */

#define SDP_ATTR_ID_SUPPORTED_CAPABILITIES             0x0310 /* Imaging */
#define SDP_ATTR_ID_SUPPORTED_FEATURES                 0x0311 /* Imaging and Hansfree */
#define SDP_ATTR_ID_SUPPORTED_FUNCTIONS                0x0312 /* Imaging */
#define SDP_ATTR_ID_TOTAL_IMAGING_DATA_CAPACITY        0x0313 /* Imaging */
#define SDP_ATTR_ID_SUPPORTED_REPOSITORIES             0x0314 /* PBAP */

#endif /* __SDP_H */
