/***********************************************************************
 * <:copyright-BRCM:2006-2013:proprietary:standard
 * 
 *    Copyright (c) 2006-2013 Broadcom 
 *    All Rights Reserved
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
 * :> *
 * $Change: 121810 $
 ***********************************************************************/

#ifndef __DEVCTL_HOMEPLUG_H__
#define __DEVCTL_HOMEPLUG_H__

/*!\file devctl_homeplug.h
 * \brief Header file for the user mode HomePlug device control library API.
 *  This is in the devCtl library.
 *
 * These API are called by user applications to perform HomePlug driver operations.
 * These API send L2 Protocol primitives to BCM60321.
 *
 */

#include "cms.h"

#define kHomeplugVersionStringSize           16
#define kHomeplugStringSize                  64
#define kHomeplugMACAddressStringSize        32
#define kHomeplugSNRPerToneStringSize        27632 /* 4 digits per item, 3454 items * 2 paths */
#define kHomeplugFWBuildMsgSize              256
#define kHomeplugSNRPerToneMaxSize           (6908 + 8) /* 8 for possible uneven grouping excess */
#define kHomeplugFlashDescrMaxSize           64

/* HPAV interface status. */
typedef enum BcmHPAVStatus
{
   HPAV_DISABLED = 0,
   HPAV_ENABLED
} BCMHPAV_STATUS;


/* UKE command status. */
typedef enum UKEStatus
{
  UKE_STATUS_SUCCESS = 0,
  UKE_STATUS_FAILED,
  UKE_STATUS_TIMEOUT,
  UKE_STATUS_ONGOING,
  UKE_STATUS_NOT_STARTED,
} UKEStatus;

/* UKE command action */
typedef enum UKEAction
{
   UKE_ACTION_START = 0,
   UKE_ACTION_RANDOMIZE_KEY,
   UKE_ACTION_GET_STATUS,
} UKEAction;


/* IEEE 1905 FOB command action */
typedef enum IEEE1905FOBAction
{
   IEEE_1905_FOB_ACTION_INVALID = 0,
   IEEE_1905_FOB_ACTION_REGISTER = 1,
} IEEE1905FOBAction;

/* STDBY command action */
typedef enum STDBYAction
{
    STDBY_ACTION_BUTTON     = (1 << 0),
    STDBY_ACTION_GPHY_CABLE = (1 << 3),
} STDBYAction;

/*! \brief HPAV QoS rule id */
typedef enum
{
   CLASSIF_RULE_ETH_DA = 0, //!< Destination Address of the Ethernet frame.
   CLASSIF_RULE_ETH_SA, //!< Source Address of the Ethernet frame.
   CLASSIF_RULE_VLAN_PRIORITY, //!< VLAN User Priority
   CLASSIF_RULE_VLAN_ID, //!< VLAN ID
   CLASSIF_RULE_IPV4_TOS, //!< IPv4 Type of Service
   CLASSIF_RULE_IPV4_PROT, //!< IPv4 Protocol
   CLASSIF_RULE_IPV4_SA, //!< IPv4 Source Address
   CLASSIF_RULE_IPV4_DA, //!< IPv4 Destination Address
   CLASSIF_RULE_IPV6_TRAF_CLASS, //!< IPv6 Traffic Class
   CLASSIF_RULE_IPV6_FLOW_LABEL, //!< IPv6 Flow Label
   CLASSIF_RULE_IPV6_SA, //!< IPv6 Source Address
   CLASSIF_RULE_IPV6_DA, //!< IPv6 Destination Address
   CLASSIF_RULE_TCP_SPORT, //!< TCP Source Port
   CLASSIF_RULE_TCP_DPORT, //!< TCP Destination Port
   CLASSIF_RULE_UDP_SPORT, //!< UDP Source Port
   CLASSIF_RULE_UDP_DPORT, //!< UDP Destination Port
   CLASSIF_RULE_MCAST_ADDR_IPv4, //!< Multicast address IPv4
   CLASSIF_RULE_MCAST_ADDR_IPv6,  //!< Multicast address IPv4
   CLASSIF_RULE_IPV4_DSCP, //!< Differenciated services
   CLASSIF_RULE_MCAST_ADDR_IPv4_MANAGED, /**< Multicast MAC address IPV4 managed */
   CLASSIF_RULE_MCAST_ADDR_IPv6_MANAGED, /**< Multicast MAC address IPV6 managed */
   CLASSIF_RULE_UCAST_ADDR, /**< Unicast address */
   CLASSIF_RULE_MCAST_ADDR_IPv4_UNMANAGED, /**<  Multicast MAC address IPV4 unmanaged */
   CLASSIF_RULE_MCAST_ADDR_IPv6_UNMANAGED, /**<  Multicast MAC address IPV6 unmanaged */
   CLASSIF_RULE_IGMP, /**<  Multicast IGMP (mcast addr and IPV4 proto IGMP) */
   CLASSIF_RULE_INVALID = 0XFF   //!< Invalid / uninitialized
} HomePlugQoSId;

/*! \brief HPAV QoS rule priority */
typedef enum
{
   CAP_0 = 0,
   CAP_1,
   CAP_2,
   CAP_3
} HomePlugQoSPriority;

/* Authorize command status. */
typedef enum AuthStatus
{
   AUTH_STATUS_SUCCESS = 0,
   AUTH_STATUS_NO_RESPONSE,
   AUTH_STATUS_PROTOCOL_ABORTED,
   AUTH_STATUS_STARTED,
   AUTH_STATUS_BUSY,
   AUTH_STATUS_FAILED
} AuthStatus;


/*! \brief HomePlug device control and driver API version information
 */
typedef struct
{
   char devCtrl_APIVerStr[kHomeplugVersionStringSize];
   char L2APIVerStr[kHomeplugVersionStringSize];
} HomePlugVersionInfo, *PHomePlugVersionInfo;

/*! \brief HomePlug interface status information
 */
typedef struct
{
    UBOOL8    enable;	/**< Enable */
    char      alias[kHomeplugStringSize];	/**< Alias */
    UINT32    lastChange;	/**< LastChange */
    char      MACAddress[kHomeplugStringSize];	/**< MACAddress */
    char      logicalNetwork[kHomeplugStringSize];	/**< LogicalNetwork */
    char      version[kHomeplugVersionStringSize];	/**< Version */
    char      firmwareVersion[kHomeplugStringSize];	/**< FirmwareVersion */
    char      otherNetworksPresent[kHomeplugStringSize];	/**< OtherNetworksPresent */
    UINT32    maxBitRate;	/**< MaxBitRate */
    UINT32    associatedDeviceNumberOfEntries;	/**< AssociatedDeviceNumberOfEntries */
    char      X_BROADCOM_COM_PLCFwBuildMsg[kHomeplugFWBuildMsgSize]; /**< firmware build message */ 
    UINT8     X_BROADCOM_COM_PLCPHY_Temperature; /**< Chip Temperature */
} HomePlugInterfaceStatus, *PHomePlugInterfaceStatus;

/*! \brief HomePlug interface stats information
 */
typedef struct
{
   UINT64    bytesSent;	/**< BytesSent */
   UINT64    bytesReceived;	/**< BytesReceived */
   UINT64    packetsSent;	/**< PacketsSent */
   UINT64    packetsReceived;	/**< PacketsReceived */
   UINT32    errorsSent;	/**< ErrorsSent */
   UINT32    errorsReceived;	/**< ErrorsReceived */
   UINT64    unicastPacketsSent;	/**< UnicastPacketsSent */
   UINT64    unicastPacketsReceived;	/**< UnicastPacketsReceived */
   UINT32    discardPacketsSent;	/**< DiscardPacketsSent */
   UINT32    discardPacketsReceived;	/**< DiscardPacketsReceived */
   UINT64    multicastPacketsSent;	/**< MulticastPacketsSent */
   UINT64    multicastPacketsReceived;	/**< MulticastPacketsReceived */
   UINT64    broadcastPacketsSent;	/**< BroadcastPacketsSent */
   UINT64    broadcastPacketsReceived;	/**< BroadcastPacketsReceived */
   UINT32    unknownProtoPacketsReceived;	/**< UnknownProtoPacketsReceived */
   UINT64    MPDUTxAck;	/**< MPDUTxAck */
   UINT64    MPDUTxCol;	/**< MPDUTxCol */
   UINT64    MPDUTxFailed;	/**< MPDUTxFailed */
   UINT64    MPDURxAck;	/**< MPDURxAck */
   UINT64    MPDURxFailed;	/**< MPDURxFailed */
} HomePlugInterfaceStats, *PHomePlugInterfaceStats;

/*! \brief visible HPAV station information
 */
typedef struct
{
   char      MACAddress[kHomeplugMACAddressStringSize]; /**< MACAddress */
   UINT8     bMACAddress[6]; /**< MACAddress in binary format */
   UINT32    txPhyRate;	/**< TxPhyRate */
   UINT8     txBeamformed; /**< tx_beamformed */
   UINT8     txMode;       /**< txMode */
   UINT8     txDevType;    /**< rx_devType */
   UINT32    rxPhyRate;	/**< RxPhyRate */
   UINT8     rxBeamformed; /**< rx_beamformed */
   UINT8     rxMode;       /**< rxMode */
   UINT8     rxDevType;    /**< rxdevType */

   
   char      SNRPerTone[kHomeplugSNRPerToneStringSize]; /**< SNRPerTone */
   UINT8     SNRPerToneBin[kHomeplugSNRPerToneMaxSize]; /**< binary SNRPerTone */
   UINT32    SNRNumTones; /**< number of snr tones */
   UINT32    avgAttenuation; /**< AvgAttenuation */
   char *    endStationMACs; /**< EndStationMACs */
   UBOOL8    active;	/**< Active */
   char      firmwareVersion[kHomeplugStringSize];	/**< X_BROADCOM_COM_FirmwareVersion */
   UINT32    uptime;	/**< X_BROADCOM_COM_Uptime */
   UINT8 *   overview; /**< X_BROADCOM_COM_Overview */
   UINT32    overview_length; /**< X_BROADCOM_COM_Overview len */
} HomePlugStation, *PHomePlugStation;

/*! \brief HomePlug interface assoc devices information
 */
typedef struct
{
   UINT8             NumSTAs; /**< Number of visible HPAV stations */
   HomePlugStation*  Stations; /**< first item in the list of visible HPAV stations */
} HomePlugInterfaceAssocDevices, *PHomePlugInterfaceAssocDevices;

/*! \brief Discovered PLC device
 */
typedef struct
{
  unsigned char MACAddress[6];  /**< MAC Address */
  char          HFID[65];       /**< HFID */
  unsigned char MMV;            /**< L2 Protocol Version supported */
} HomePlugDiscoveredLocalPLCDevice, *PHomePlugDiscoveredLocalPLCDevice, HomePlugDiscoveredPLCDevice;

/*! \brief Remote PLC devices discovered from the local one
 */
typedef struct
{
  UINT8 NumSTAs;  /**< Number of discovered HPAV stations */
  HomePlugDiscoveredPLCDevice  Stations[16]; /** list of discovered HPAV stations (max of 16) */
} HomePlugDiscoveredPLCDevicesInfo, *PHomePlugDiscoveredPLCDevicesInfo;

/*! \brief HPAV QoS rule set */
typedef struct
{
   unsigned char no_av_rules; //!< Number of chaining rules (a maximum of 4)
   HomePlugQoSId id[4]; //!< rule's ids

   unsigned char no_bytes_used; //!< Number of bytes used in the rule value buffer
   unsigned char value[16]; /*!< rules value buffer
                       (all the rules shared 16 bytes for their value) */
} HomePlugQoSRuleSet;

/*! \brief HPAV QoS rule */
typedef struct
{
   HomePlugQoSPriority CAP; //!< channel access priority.
   HomePlugQoSRuleSet classifier_rule_set; //!< classifier rules assoc.
} HomePlugQoSRule;

/*! \brief Number of QoS rules that can be set for each level */
typedef struct
{
  unsigned char classifier_rules_src_dst_mac; /** Number of classifying rules for src or dst MAC address */
  unsigned char classifier_rules_vlan; /** Number of classifying rules for VLAN tag or prio */
  unsigned char classifier_rules_ip_tos_prot; /** Number of classifying rules for IP ToS prot or IPV6 class */
  unsigned char classifier_rules_ip_dst_src_addr; /** Number of classifying rules for IP dest or src address */
  unsigned char classifier_rules_udp_tcp_port; /** Number of classifying rules for  UDP or TCP port */
} HomePlugQoSRuleLevel;

/*! \brief HomePlug Standby settings
 */
typedef struct
{
   unsigned short powermgr_standby_timeout; /* Standby timeout */
   int powermgr_standby_enabled; /* Standby is enabled/disabled */
} HomePlugStandbySettings, *PHomePlugStandbySettings;

/*! \brief Flash information
 */
typedef struct
{
  UINT32  flashSize;  /**< flash size in KB */
  char    flashDescription[kHomeplugFlashDescrMaxSize]; /**< flash 
                                                             description */
} HomePlugFlashInfo;

/*! \brief Traffic generation info
 */
typedef struct
{
   UINT8 mac_address[6]; //!< Destination mac address
   UINT32 duration; //!< Time duration (in ms)
   UINT16 packet_length; //!< Packet length (in bytes). Max value 1518
   UINT32 packet_period; //!< Packet period (in ms)
   UINT8 is_bidir; //!< 0 indicates unidir, 1 indicates bidir
} HomePlugGenerateTraffic, *PHomePlugGenerateTraffic;


/** Initialize HomePlug device control and driver API
 */
void devCtrl_homeplugIni();

/** Terminate HomePlug device control and driver API
 */
void devCtrl_homeplugClose();

/** Discover local PLC device's MAC Address
 */
int devCtrl_homeplugDiscoverLocalPLCDevice(int secs, PHomePlugDiscoveredLocalPLCDevice localPLCDevice);

/** Get HomePlug device control and driver API version information
 *
 * @param PHomePlugVersionInfo (OUT)  A pointer to version data struct.
 */
void devCtrl_homeplugGetVersionInfo(PHomePlugVersionInfo homeplugVer);

/** Get HomePlug interface BCM60321's status
 *
 * This function is called to get BCM60321's status.
 *
 * @param status (OUT)   BCM60321's status.
 *
 * @return 0 - success, -1 - error
 */
int devCtl_homeplugGetInterfaceStatus(BCMHPAV_STATUS* status);

/** Set HomePlug interface BCM60321's status
 *
 * This function is called to set BCM60321's status.
 *
 * @param status (IN)   BCM60321's status.
 *
 * @return 0 - success, -1 - error
 */
int devCtl_homeplugSetInterfaceStatus(BCMHPAV_STATUS status);

/** Get HomePlug interface status data from BCM60321
 *
 * This function is called to get status data from BCM60321.
 *
 * @param PHomePlugInterfaceStatus (OUT)  A pointer to status data struct.
 *
 * @return 0 - success, -1 - error
 */
int devCtl_homeplugGetInterfaceStatusData(PHomePlugInterfaceStatus data);

/** Get HomePlug interface stats data from BCM60321
 *
 * This function is called to get stats data from BCM60321.
 *
 * @param PHomePlugInterfaceStats (OUT)  A pointer to stats data struct.
 *
 * @return 0 - success, -1 - error
 */
int devCtl_homeplugGetInterfaceStatsData(PHomePlugInterfaceStats data);

/** Get HomePlug interface basic associated device data from local PLC device
 *
 * This function is called to get basic associated device data like PHY Rates 
 * or MAC address from local PLC device.
 *
 * @param PHomePlugInterfaceAssocDevices (OUT) A pointer to assoc device data struct
 *
 * @return 0 - success, -1 - error
 */
int devCtl_homeplugGetInterfaceAssocDeviceData(PHomePlugInterfaceAssocDevices data);

/** Get HomePlug interface extended associated device data from local PLC device
 *
 * This function is called to get extended associated device data like SNR or 
 * EndStations from local PLC device.
 *
 * @param PHomePlugInterfaceAssocDevices (OUT) A pointer to assoc device data struct
 *
 * @return 0 - success, -1 - error
 */
int devCtl_homeplugGetInterfaceAssocDeviceDataExtended(PHomePlugInterfaceAssocDevices data);

/** Set HomePlug interface alias
 *
 * This function is called to set Alias in BCM60321.
 *
 * @param alias (IN) data to set in BCM60321
 *
 * @return 0 - success, -1 - error
 */
int devCtl_homeplugSetInterfaceAlias(char* alias);

/** Set HomePlug interface logical network
 *
 * This function is called to set Logical Network in BCM60321.
 *
 * @param logical_network (IN) data to set in BCM60321
 *
 * @return 0 - success, -1 - error
 */
int devCtl_homeplugSetInterfaceLogicalNetwork(char* logical_network);

/** Set HomePlug interface network password
 *
 * This function is called to set Network Password in BCM60321.
 *
 * @param network_password (IN) data to set in BCM60321
 *
 * @return 0 - success, -1 - error
 */
int devCtl_homeplugSetInterfaceNetworkPassword(char* network_password);

/** Set HomePlug interface Universal Key Exchange (UKE) action
 *
 * This function is called to start UKE, randomize the network key
 * 
 * and check the status of the UKE protocol.
 *
 * @param action to be performed (start protocol/randomize key).
 *
 * @return 0 - success, -1 - error
 */
int devCtl_homeplugUKEAction(UKEAction action, UKEStatus * status);

/** Set HomePlug interface Stansby action
 *
 * This function is called to put the device in standby mode,
 *
 * wakeable by GPHY link detection or by a button press.
 *
 * @param action type of standby (GPHY cable / butotn)
 *
 * @return 0 - success, -1 - error
 */
int devCtl_homeplugSTDBYAction(STDBYAction action);

/** Send ieee1905 FOB action to the PLC,
*
* This function is called to send a ieee1905 fob action to 
* 
* the PLC subsystem.
*
* @param action to be performed.
*
* @param data pointer to the data to be passed.
*
* @param len length of the passed data.
*
* @return 0 - success, -1 - error
*/
int devCtl_homeplugIeee1905FOBAction(IEEE1905FOBAction action, char * data, unsigned int len);


/** Set MAC addr to use for communication with local PLC device
*
* This function is called to set the MAC addresse that will be used to  
* 
* communicate with the local PLC device
*
* @param macAddr to be used
*
* @return 0 - success, -1 - error
*/
int devCtrl_homeplugSetLocalPlcDeviceMac(unsigned char *macAddr);

/** Get remote discovered devices from local PLC device
 *
 * This function is called to obtain remote devices from local PLC device
 *
 * @param secs (IN) Seconds while the function is discovering devices
 * @param PHomePlugDiscoveredPLCDevicesInfo (OUT) A pointer to discovered devices data struct
 *
 * @return 0 - success, -1 - error
 */
int devCtl_homeplugGetDiscoveredPLCDevicesData(int secs, PHomePlugDiscoveredPLCDevicesInfo data);

/** Gets  local device firmeare version (sets value into global descirptor).
 *
 * @param fw_version (OUT) char str representing the fw version.
 *
 * @return 0 - success, -1 - error
 */
int devCtl_homeplugGetLocalDeviceVersion(char * fw_version);
/** Read HPAV QoS rules parameter from an existing paramconfig image file and
 *  save the content into a buffer.
 *
 * @param path      (IN)    full path to the paramconfig image file
 * @param size      (IN)    buffer size
 * @param buffer    (OUT)   buffer containing QoS information
 *
 * @return 0 - success, -1 - error
 */
int devCtrl_homeplugReadQoSParameter(
  const char* path,
  const int size,
  unsigned char* buffer);

/** Write buffer containing QoS rules into HPAV QoS rules parameter within an
 *  existing paramconfig image file.
 *
 * @param path      (IN)    full path to the paramconfig image file
 * @param size      (IN)    buffer size
 * @param buffer    (IN)    buffer containing QoS information
 *
 * @return 0 - success, -1 - error
 */
int devCtrl_homeplugWriteQoSParameter(
  const char* path,
  const int size,
  const unsigned char* buffer);

/** Deserialize QoS rules data.
 *
 * @param size      (IN)   buffer size
 * @param buffer    (IN)   buffer containing QoS information
 * @param no_rules  (OUT)  number of QoS rules contained in the buffer
 * @param qos_rules (OUT)  array of HomePlugQoSRule structs with QoS data from the input buffer
 *
 * @return 0 - success, -1 - error
 */
int devCtrl_homeplugDeserializeQoSData(
  const int size,
  const unsigned char* buffer,
  unsigned char* no_rules,
  HomePlugQoSRule* qos_rules);

/** Serialize QoS rules data.
 *
 * @param no_rules  (IN)   number of entries in qos_rules
 * @param qos_rules (IN)   array of HomePlugQoSRule structs with QoS rules
 * @param size      (IN)   buffer size
 * @param buffer    (OUT)  buffer that will contain QoS information from qos_rules array
 *
 * @return 0 - success, -1 - error
 */
int devCtrl_homeplugSerializeQoSData(
   const unsigned char no_rules,
   const HomePlugQoSRule *qos_rules,
   const int size,
   unsigned char* buffer);

/** Read HPAV QoS "rules for level" information from an existing paramconfig
 *  image file and save the content in a HomePlugQoSRuleLevel struct.
 *
 * @param path        (IN)    full path to the paramconfig image file
 * @param rules_level (OUT)   struct which will contain QoS "rules for level"
 *                            information
 *
 * @return 0 - success, -1 - error
 */
int devCtrl_homeplugGetQoSRuleLevels(
  const char* path,
  HomePlugQoSRuleLevel* rules_level);

/** Write HPAV QoS "rules for level" information to an existing paramconfig
 *  image file.
 *
 * @param path        (IN)    full path to the paramconfig image file
 * @param rules_level (IN)    struct containing QoS "rules for level" information
 *
 * @return 0 - success, -1 - error
 */
int devCtrl_homeplugSetQoSRuleLevels(
  const char* path,
  HomePlugQoSRuleLevel* rules_level);

/** Get HomePlug interface PLC-PHY counters (BCM60500/BCM60333)
 *
 * This function is called to get  PLC-PHY counters (BCM60500/BCM60333)
 *
 * @param counters (OUT)  A pointer to array of counters
 * @param ncounters (OUT) Number of counters in the array
 *
 * @return 0 - success, -1 - error
 */
int devCtl_homeplugGetPLCPHYCounters(unsigned int** counters, unsigned int* ncounters);


/** Read Standby settings from an existing paramconfig image file.
 *
 * @param PHomePlugStandbySettings    (OUT)   Standby Settings
 *
 * @return 0 - success, -1 - error
 */
int devCtrl_homeplugReadStandbySettingsFromParamconfig(
  PHomePlugStandbySettings standbySettings);

/** Write Standby settings to an existing paramconfig image file.
 *
 * @param PHomePlugStandbySettings    (IN)   Standby Settings
 *
 * @return 0 - success, -1 - error
 */
int devCtrl_homeplugWriteStandbySettingsToParamconfig(
  PHomePlugStandbySettings standbySettings);

/** Generate traffic
 * 
 * @param mac_address Destination mac address of the traffic 
 * @param duration Time of traffic generation (in ms)
 * @param packet_length Ethernet packet length (in bytes)
 * @param inter_packet_gap Inter packet gap time (in ms)
 * @param is_bidir Generate unidir or bidir traffic (0 unidir, 1 bidir)
 *
 * @return 0 - success, -1 error (if input parmeters out of ranges )
 */
int devCtl_homeplugGenerateTraffic( PHomePlugGenerateTraffic data );

/** Get flash information.
 *
 * @param flashInfo (OUT) pointer to FlashInfo
 *
 * @return 0 - success, -1 - error
 */
int devCtl_homeplugGetFlashInfo(HomePlugFlashInfo* flashInfo);

/** Instruct a STA to authorize another STA to join its AVLN using DAK-based 
 *  distribution of the NMK.
 *
 * @param RemotePLCDevice (IN)  MAC address of STA to authenticate, if known
 * @param DevicePassword (IN)   DAK to be used for NMK Provisioning
 * @param status (OUT)          Operation result    
 *
 * @return 0 - success, -1 - error
 */
int devCtl_homeplugAuthorize(
  const char* RemotePLCDevice, const char* DevicePassword,  AuthStatus * status);

#endif /* __DEVCTL_HOMEPLUG_H__ */
