/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2016 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

/**********************************************************************

    module: wifi_hal.h

        For CCSP Component:  Wifi_Provisioning_and_management

    ---------------------------------------------------------------

    description:

        This header file gives the function call prototypes and 
        structure definitions used for the RDK-Broadband 
        Wifi radio hardware abstraction layer

    ---------------------------------------------------------------

    environment:

        This HAL layer is intended to support Wifi drivers 
        through an open API.  

    ---------------------------------------------------------------

    HAL version:

        The version of the Wifi HAL is specified in #defines below.

    --------------------------------------------------------------- 

    author:

        zhicheng_qiu@comcast.com
        Charles Moreman, moremac@cisco.com
        Paul White, paul@plumewifi.com
    ---------------------------------------------------------------

	Notes:

	What is new for 2.2.0
	  1. Add Country Code support
	  2. Add more DCS function
	  3. Move RadiusSecret from struct wifi_radius_setting_t to wifi_getApSecurityRadiusServer function
	  4. Add wifi_getApSecuritySecondaryRadiusServer
	What is new for 2.2.1
	  1. Add wifi_setRadioTrafficStatsMeasure, wifi_setRadioTrafficStatsRadioStatisticsEnable
	What is new for 2.2.2
	  1. Add Band Steering HAL
	What is new for 2.3.0
	  1. Add AP Beacon Rate control HAL
	  2. Add Dynamic Channel Selection (phase 2) HAL
	  3. Add Air Time Management HAL
	What is new for 2.4.0
	  1. Add data structure and HAL for mesh
	What is new for 2.5.0
	  1. Add the Channel switch HAL for mesh
	What is new for 2.6.0
	  1. Add the Band steering HAL for mesh
	What is new for 2.7.0
	  1. Add HAL for Wifi telemetry
	What is new for 2.8.0
	  1. Add HAL for 11w
    What is new for 2.9.0
      1. Add HAL function definitions for 802.11r Fast Transition
    What is new for 2.10.0
      1. Add HAL function definitions for 802.11v BSS Transition Management
    What is new for 2.11.0
      1. Add HAL function definitions for 802.11k Neighbor Request and Response definitions
    What is new for 2.12.0
      1. Add HAL function definitions for 802.11k Beacon Request and Response definitions
    What is new for 2.13.0
      1. Add HAL function definitions for DPP
    What is new for 2.14.0
      1. Add HAL function definitions for steering effectiveness telemetry
    What is new for 2.15.0
      1. Add HAL function definitions for 802.11ax
      2. Add HAL definitions for dfs channel state
      3. Add HAL function definitions for EAP parameters
    What is new for 2.16.0
      1. Modified HAL structure definition for VAP Telemetry
    What is new for 2.17.0
      1. Add HAL function definition for Single Client reporting feature
    What is new for 2.18.0
      1. Add HAL Fuction Definition for Absolute TX-Power retreival
    What is new for 2.19.0
      1. Added zerowait DFS status support
      2. Modified HAL definitions for EAP parameters
      3. Updated comments for ChannelUtilization, ActivityFactor, CarrierSenseThreshold_Exceeded
         and RetransmissionMetirc radio metrics
    What is new for 2.20.0
      1. Add HAL definitions for Multi PSK support
**********************************************************************/
/**
* @file wifi_hal.h
* @author zhicheng_qiu@cable.comcast.com
* @brief For CCSP Component:  Wifi_Provisioning_and_management
*
* @brief Wifi subsystem level APIs that are common to Client and Access Point devices. This HAL layer is intended to support Wifi drivers through an open API. This sample implementation file gives the function call prototypes and structure definitions used for the RDK-Broadband Wifi hardware abstraction layer.
* This header file gives the function call prototypes and structure definitions used for the RDK-Broadband Wifi radio hardware abstraction layer.
*/

/**
 * @defgroup WIFI_HAL  Wi-Fi HAL
 * Wi-Fi Access Point HAL provides an interface (data structures and API) to create, secure and delete the Access point
 * and also provides APIs to establish the client to connect to the Access point.
 *
 * @defgroup WIFI_HAL_TYPES  WIFI HAL Data Types
 * @ingroup  WIFI_HAL
 *
 * @defgroup WIFI_HAL_APIS WIFI HAL APIs
 * @ingroup WIFI_HAL
 *
 */

#ifndef __WIFI_HAL_H__
#define __WIFI_HAL_H__

#ifdef __cplusplus
extern "C"{
#endif


#ifndef ULLONG
#define ULLONG unsigned long long
#endif

#ifndef ULONG
#define ULONG unsigned long
#endif

#ifndef USHORT
#define USHORT unsigned short
#endif

#ifndef BOOL
#define BOOL  unsigned char
#endif

#ifndef CHAR
#define CHAR  char
#endif

#ifndef UCHAR
#define UCHAR unsigned char
#endif

#ifndef INT
#define INT   int
#endif

#ifndef UINT
#define UINT  unsigned int
#endif

#ifndef TRUE
#define TRUE     1
#endif

#ifndef FALSE
#define FALSE    0
#endif

#ifndef ENABLE
#define ENABLE   1
#endif

#ifndef RETURN_OK
#define RETURN_OK   0
#endif

#ifndef RETURN_ERR
#define RETURN_ERR   -1
#endif


#ifndef RADIO_INDEX_1
#define RADIO_INDEX_1 1
#define RADIO_INDEX_2 2
#define AP_INDEX_1 1
#define AP_INDEX_2 2
#define AP_INDEX_3 3
#define AP_INDEX_4 4
#define AP_INDEX_5 5
#define AP_INDEX_6 6
#define AP_INDEX_7 7
#define AP_INDEX_8 8
#define AP_INDEX_9 9
#define AP_INDEX_10 10
#define AP_INDEX_11 11
#define AP_INDEX_12 12
#define AP_INDEX_13 13
#define AP_INDEX_14 14
#define AP_INDEX_15 15
#define AP_INDEX_16 16
#endif

/**
 * @addtogroup WIFI_HAL_TYPES
 * @{
 */
//defines for HAL version 2.19.0
#define WIFI_HAL_MAJOR_VERSION 2   /**< This is the major verion of this HAL. */
#define WIFI_HAL_MINOR_VERSION 19   /**< This is the minor verson of the HAL. */
#define WIFI_HAL_MAINTENANCE_VERSION 0   /**< This is the maintenance version of the HAL. */

/**********************************************************************
                STRUCTURE DEFINITIONS
**********************************************************************/
typedef unsigned char mac_address_t[6];
typedef char   r1_key_holder_t[13];
typedef char   nas_id_t[49];
typedef unsigned char   r0r1_key_t[16];
typedef char r0r1_key_str_t[33];
typedef char            mac_addr_str_t[18];
typedef mac_address_t   bssid_t;
typedef char            ssid_t[32];
typedef unsigned int u_int32_t;
typedef struct _wifi_basicTrafficStats
{
     ULONG wifi_BytesSent;
     ULONG wifi_BytesReceived;
     ULONG wifi_PacketsSent;
     ULONG wifi_PacketsReceived;
     ULONG wifi_Associations; 	
} wifi_basicTrafficStats_t;

typedef struct _wifi_trafficStats
{
     ULONG wifi_ErrorsSent;
     ULONG wifi_ErrorsReceived;
     ULONG wifi_UnicastPacketsSent;
     ULONG wifi_UnicastPacketsReceived;
     ULONG wifi_DiscardedPacketsSent;
     ULONG wifi_DiscardedPacketsReceived;
     ULONG wifi_MulticastPacketsSent;
     ULONG wifi_MulticastPacketsReceived;
     ULONG wifi_BroadcastPacketsSent;
     ULONG wifi_BroadcastPacketsRecevied;
     ULONG wifi_UnknownPacketsReceived;
} wifi_trafficStats_t;

typedef struct _wifi_radioTrafficStats
{
	 ULONG wifi_ErrorsSent;	
     ULONG wifi_ErrorsReceived;   
     ULONG wifi_DiscardPacketsSent; 
     ULONG wifi_DiscardPacketsReceived; 
	 ULONG wifi_PLCPErrorCount;	
	 ULONG wifi_FCSErrorCount;	
	 ULONG wifi_InvalidMACCount;	
	 ULONG wifi_PacketsOtherReceived;	
	 INT   wifi_Noise; 	
	 
} wifi_radioTrafficStats_t;	

typedef struct _wifi_ssidTrafficStats
{
     ULONG wifi_RetransCount;	
     ULONG wifi_FailedRetransCount; 	
     ULONG wifi_RetryCount;  
     ULONG wifi_MultipleRetryCount; 	
     ULONG wifi_ACKFailureCount;  	
     ULONG wifi_AggregatedPacketCount; 	
	 
} wifi_ssidTrafficStats_t;  

typedef struct _wifi_neighbor_ap
{
	 CHAR  ap_Radio[64];	
	 CHAR  ap_SSID[64];	
	 CHAR  ap_BSSID[64];	
	 CHAR  ap_Mode[64];	
	 UINT  ap_Channel;
	 INT   ap_SignalStrength;
	 CHAR  ap_SecurityModeEnabled[64];	
	 CHAR  ap_EncryptionMode[64];	
	 CHAR  ap_OperatingFrequencyBand[16];	
	 CHAR  ap_SupportedStandards[64];	
	 CHAR  ap_OperatingStandards[16];	
	 CHAR  ap_OperatingChannelBandwidth[16];	
	 UINT  ap_BeaconPeriod;	
	 INT   ap_Noise;	
	 CHAR  ap_BasicDataTransferRates[256];	
	 CHAR  ap_SupportedDataTransferRates[256];
	 UINT  ap_DTIMPeriod;
	 UINT  ap_ChannelUtilization;	
	 
} wifi_neighbor_ap_t;
//<<

typedef struct _wifi_radioTrafficStats2
{
     ULONG radio_BytesSent;	/**< The total number of bytes transmitted out of the interface, including framing characters. */
     ULONG radio_BytesReceived;	/**< The total number of bytes received on the interface, including framing characters. */
     ULONG radio_PacketsSent;	/**< The total number of packets transmitted out of the interface. */
     ULONG radio_PacketsReceived; /**< The total number of packets received on the interface. */

	 ULONG radio_ErrorsSent;	/**< The total number of outbound packets that could not be transmitted because of errors. */
     ULONG radio_ErrorsReceived;    /**< The total number of inbound packets that contained errors preventing them from being delivered to a higher-layer protocol. */
     ULONG radio_DiscardPacketsSent; /**< The total number of outbound packets which were chosen to be discarded even though no errors had been detected to prevent their being transmitted. One possible reason for discarding such a packet could be to free up buffer space. */
     ULONG radio_DiscardPacketsReceived; /**< The total number of inbound packets which were chosen to be discarded even though no errors had been detected to prevent their being delivered. One possible reason for discarding such a packet could be to free up buffer space. */
	 ULONG radio_PLCPErrorCount;	/**< The number of packets that were received with a detected Physical Layer Convergence Protocol (PLCP) header error.	*/
	 ULONG radio_FCSErrorCount;	/**< The number of packets that were received with a detected FCS error. This parameter is based on dot11FCSErrorCount from [Annex C/802.11-2012]. */
	 ULONG radio_InvalidMACCount;	/**< The number of packets that were received with a detected invalid MAC header error. */
	 ULONG radio_PacketsOtherReceived;	/**< The number of packets that were received, but which were destined for a MAC address that is not associated with this interface. */
	 INT   radio_NoiseFloor; 	/**< The noise floor for this radio channel where a recoverable signal can be obtained. Expressed as a signed integer in the range (-110:0).  Measurement should capture all energy (in dBm) from sources other than Wi-Fi devices as well as interference from Wi-Fi devices too weak to be decoded. Measured in dBm */
	 ULONG radio_ChannelUtilization; /**< Percentage of time the channel was occupied by the radio's own activity (Activity Factor) or the activity of other radios.  Channel utilization MUST cover all user traffic, management traffic, and time the radio was unavailable for CSMA activities, including DIFS intervals, etc.  The metric is calculated and updated in this parameter and if this metric is queried, it MUST return real-time value.  Units in Percentage */
	 INT   radio_ActivityFactor; /**< Percentage of time that the radio was transmitting or receiving Wi-Fi packets to/from associated clients. Activity factor MUST include all traffic that deals with communication between the radio and clients associated to the radio as well as management overhead for the radio, including NAV timers, beacons, probe responses,time for receiving devices to send an ACK, SIFC intervals, etc.  The metric is calculated and updated in this parameter.  If this metric is queried, it MUST return real-time value.  Units in Percentage */
	 INT   radio_CarrierSenseThreshold_Exceeded; /**< Percentage of time that the radio was unable to transmit or receive Wi-Fi packets to/from associated clients due to energy detection (ED) on the channel or clear channel assessment (CCA). The metric is calculated and updated in this Parameter. If this metric is queried, it MUST return real-time value.  Units in Percentage */
	 INT   radio_RetransmissionMetirc; /**< Percentage of packets that had to be re-transmitted. Multiple re-transmissions of the same packet count as one.  The metric is calculated and updated in this parameter.  If this metric is queried, it MUST return real-time value.  Units  in percentage */
	 INT   radio_MaximumNoiseFloorOnChannel; /**< Maximum Noise on the channel during the measuring interval.  The metric is updated in this parameter at the end of the interval defined by "Radio Statistics Measuring Interval".  The calculation of this metric MUST only use the data collected in the just completed interval.  If this metric is queried before it has been updated with an initial calculation, it MUST return -1.  Units in dBm */
	 INT   radio_MinimumNoiseFloorOnChannel; /**< Minimum Noise on the channel. The metric is updated in this Parameter at the end of the interval defined by "Radio Statistics Measuring Interval".  The calculation of this metric MUST only use the data collected in the just completed interval.  If this metric is queried before it has been updated with an initial calculation, it MUST return -1. Units in dBm */
	 INT   radio_MedianNoiseFloorOnChannel; /**< Median Noise on the channel during the measuring interval.   The metric is updated in this parameter at the end of the interval defined by "Radio Statistics Measuring Interval".  The calculation of this metric MUST only use the data collected in the just completed interval.  If this metric is queried before it has been updated with an initial calculation, it MUST return -1. Units in dBm */
	 ULONG radio_StatisticsStartTime; 	 /**< The date and time at which the collection of the current set of statistics started.  This time must be updated whenever the radio statistics are reset. */
	
} wifi_radioTrafficStats2_t;	//for radio only

typedef struct _wifi_radioTrafficStatsMeasure
{
	 INT   radio_RadioStatisticsMeasuringRate; /**< Input //"The rate at which radio related statistics are periodically collected.  Only statistics that explicitly indicate the use of this parameter MUST use the rate set in this parameter  Other parameter's are assumed to collect data in real-time or nearly real-time. Default value is 30 seconds.  This parameter MUST be persistent across reboots. If this parameter is changed,  then use of the new rate MUST be deferred until the start of the next interval and all metrics using this rate MUST return -1 until the completion of the next full interval Units in Seconds" */
	 INT   radio_RadioStatisticsMeasuringInterval; /**< Input //The interval for which radio data MUST be retained in order and at the end of which appropriate calculations are executed and reflected in the associated radio object's.  Only statistics that explicitly indicate the use of this parameter MUST use the interval set in this parameter  Default value is 30 minutes.  This parameter MUST be persistent across reboots.   If this item is modified, then all metrics leveraging this interval as well as the metrics "Total number 802.11 packet of TX" and "Total number 802.11 packet of RX" MUST be re-initialized immediately.  Additionally, the "Statistics Start Time" must be reset to the current time. Units in Seconds */
} wifi_radioTrafficStatsMeasure_t;	//for radio only


typedef struct _wifi_ssidTrafficStats2
{
     ULONG ssid_BytesSent;	/**< The total number of bytes transmitted out of the interface, including framing characters. */
     ULONG ssid_BytesReceived;	/**< The total number of bytes received on the interface, including framing characters. */
     ULONG ssid_PacketsSent;	/**< The total number of packets transmitted out of the interface. */
     ULONG ssid_PacketsReceived; /**< The total number of packets received on the interface. */

     ULONG ssid_RetransCount;	/**< The total number of transmitted packets which were retransmissions. Two retransmissions of the same packet results in this counter incrementing by two. */
     ULONG ssid_FailedRetransCount; /**< The number of packets that were not transmitted successfully due to the number of retransmission attempts exceeding an 802.11 retry limit. This parameter is based on dot11FailedCount from [802.11-2012]. */
     ULONG ssid_RetryCount;  /**< The number of packets that were successfully transmitted after one or more retransmissions. This parameter is based on dot11RetryCount from [802.11-2012]. */
     ULONG ssid_MultipleRetryCount; /**< The number of packets that were successfully transmitted after more than one retransmission. This parameter is based on dot11MultipleRetryCount from [802.11-2012]. */
     ULONG ssid_ACKFailureCount;  /**< The number of expected ACKs that were never received. This parameter is based on dot11ACKFailureCount from [802.11-2012]. */	
     ULONG ssid_AggregatedPacketCount; /**< The number of aggregated packets that were transmitted. This applies only to 802.11n and 802.11ac. */
	 
	 ULONG ssid_ErrorsSent;	/**< The total number of outbound packets that could not be transmitted because of errors. */
     ULONG ssid_ErrorsReceived;    /**< The total number of inbound packets that contained errors preventing them from being delivered to a higher-layer protocol. */
     ULONG ssid_UnicastPacketsSent;	/**< The total number of inbound packets that contained errors preventing them from being delivered to a higher-layer protocol. */
     ULONG ssid_UnicastPacketsReceived;  /**< The total number of received packets, delivered by this layer to a higher layer, which were not addressed to a multicast or broadcast address at this layer. */
     ULONG ssid_DiscardedPacketsSent; /**< The total number of outbound packets which were chosen to be discarded even though no errors had been detected to prevent their being transmitted. One possible reason for discarding such a packet could be to free up buffer space. */
     ULONG ssid_DiscardedPacketsReceived; /**< The total number of inbound packets which were chosen to be discarded even though no errors had been detected to prevent their being delivered. One possible reason for discarding such a packet could be to free up buffer space. */
     ULONG ssid_MulticastPacketsSent; /**< The total number of packets that higher-level protocols requested for transmission and which were addressed to a multicast address at this layer, including those that were discarded or not sent. */
     ULONG ssid_MulticastPacketsReceived; /**< The total number of received packets, delivered by this layer to a higher layer, which were addressed to a multicast address at this layer.  */
     ULONG ssid_BroadcastPacketsSent;  /**< The total number of packets that higher-level protocols requested for transmission and which were addressed to a broadcast address at this layer, including those that were discarded or not sent. */
     ULONG ssid_BroadcastPacketsRecevied; /**< The total number of packets that higher-level protocols requested for transmission and which were addressed to a broadcast address at this layer, including those that were discarded or not sent. */
     ULONG ssid_UnknownPacketsReceived;  /**< The total number of packets received via the interface which were discarded because of an unknown or unsupported protocol. */

} wifi_ssidTrafficStats2_t;  //for ssid only


//Please do not edit the elements for this data structure 
typedef struct _wifi_neighbor_ap2
{
	 //CHAR  ap_Radio[64];	//The value MUST be the path name of a row in theDevice.WiFi.Radiotable. The Radio that detected the neighboring WiFi SSID.  
	 CHAR  ap_SSID[64];	/**< The current service set identifier in use by the neighboring WiFi SSID. The value MAY be empty for hidden SSIDs. */
	 CHAR  ap_BSSID[64];	/**< [MACAddress] The BSSID used for the neighboring WiFi SSID. */
	 CHAR  ap_Mode[64];	/**< The mode the neighboring WiFi radio is operating in. Enumeration of: AdHoc, Infrastructure */
	 UINT  ap_Channel;	/**< The current radio channel used by the neighboring WiFi radio. */
	 INT   ap_SignalStrength;	/**< An indicator of radio signal strength (RSSI) of the neighboring WiFi radio measured indBm, as an average of the last 100 packets received. */
	 CHAR  ap_SecurityModeEnabled[64];	/**< The type of encryption the neighboring WiFi SSID advertises. Enumeration of:None, WPA-WPA2 etc. */
	 CHAR  ap_EncryptionMode[64];	/**< Comma-separated list of strings. The type of encryption the neighboring WiFi SSID advertises. Each list item is an enumeration of: TKIP, AES */
	 CHAR  ap_OperatingFrequencyBand[16];	/**< Indicates the frequency band at which the radio this SSID instance is operating. Enumeration of:2.4GHz, 5GHz */
	 CHAR  ap_SupportedStandards[64];	/**< Comma-separated list of strings. List items indicate which IEEE 802.11 standards thisResultinstance can support simultaneously, in the frequency band specified byOperatingFrequencyBand. Each list item is an enumeration of: */
	 CHAR  ap_OperatingStandards[16];	/**< Comma-separated list of strings. Each list item MUST be a member of the list reported by theSupportedStandardsparameter. List items indicate which IEEE 802.11 standard that is detected for thisResult. */
	 CHAR  ap_OperatingChannelBandwidth[16];	/**< Indicates the bandwidth at which the channel is operating. Enumeration of: */
	 UINT  ap_BeaconPeriod;	/**< Time interval (inms) between transmitting beacons. */
	 INT   ap_Noise;	/**< Indicator of average noise strength (indBm) received from the neighboring WiFi radio. */
	 CHAR  ap_BasicDataTransferRates[256];	/**< Comma-separated list (maximum list length 256) of strings. Basic data transmit rates (in Mbps) for the SSID. For example, ifBasicDataTransferRatesis "1,2", this indicates that the SSID is operating with basic rates of 1 Mbps and 2 Mbps. */
	 CHAR  ap_SupportedDataTransferRates[256];	/**< Comma-separated list (maximum list length 256) of strings. Data transmit rates (in Mbps) for unicast frames at which the SSID will permit a station to connect. For example, ifSupportedDataTransferRatesis "1,2,5.5", this indicates that the SSID will only permit connections at 1 Mbps, 2 Mbps and 5.5 Mbps. */
	 UINT  ap_DTIMPeriod;	/**< The number of beacon intervals that elapse between transmission of Beacon frames containing a TIM element whose DTIM count field is 0. This value is transmitted in the DTIM Period field of beacon frames. [802.11-2012] */
	 UINT  ap_ChannelUtilization;	/**< Indicates the fraction of the time AP senses that the channel is in use by the neighboring AP for transmissions. */
	 
} wifi_neighbor_ap2_t;	//COSA_DML_NEIGHTBOURING_WIFI_RESULT

typedef struct _wifi_diag_ipping_setting
{
	 CHAR  ipping_Interface[256];	/**< The value MUST be the path name of a row in the IP.Interface table. The IP-layer interface over which the test is to be performed. This identifies the source IP address to use when performing the test. Example: Device.IP.Interface.1. If an empty string is specified, the CPE MUST use the interface as directed by its routing policy (Forwarding table entries) to determine the appropriate interface. */
	 CHAR  ipping_Host[256];	/**< Host name or address of the host to ping. In the case where Host is specified by name, and the name resolves to more than one address, it is up to the device implementation to choose which address to use. */
	 UINT  ipping_NumberOfRepetitions;	/**< Number of repetitions of the ping test to perform before reporting the results. */
	 UINT  ipping_Timeout;	/**< Timeout in milliseconds for the ping test.	*/
	 UINT  ipping_DataBlockSize;	/**< Size of the data block in bytes to be sent for each ping. */
	 UINT  ipping_DSCP;	/**< DiffServ codepoint to be used for the test packets. By default the CPE SHOULD set this value to zero. */

} wifi_diag_ipping_setting_t;	

typedef struct _wifi_diag_ipping_result
{
	 CHAR  ipping_DiagnosticsState[64];	/**< Indicates availability of diagnostic data. Enumeration of:	Complete, Error_CannotResolveHostName, 	Error_Internal, Error_Other */
	 UINT  ipping_SuccessCount;	/**< Result parameter indicating the number of successful pings (those in which a successful response was received prior to the timeout) in the most recent ping test.	*/
	 UINT  ipping_FailureCount;	/**< Result parameter indicating the number of failed pings in the most recent ping test. */
	 UINT  ipping_AverageResponseTime;	/**< Result parameter indicating the average response time in milliseconds over all repetitions with successful responses of the most recent ping test. If there were no successful responses, this value MUST be zero. */
	 UINT  ipping_MinimumResponseTime;	/**< Result parameter indicating the minimum response time in milliseconds over all repetitions with successful responses of the most recent ping test. If there were no successful responses, this value MUST be zero. */
	 UINT  ipping_MaximumResponseTime;	/**< Result parameter indicating the maximum response time in milliseconds over all repetitions with successful responses of the most recent ping test. If there were no successful responses, this value MUST be zero. */
	 
} wifi_diag_ipping_result_t;

//----------------ENVIRONMENT-------------------------------------------
typedef struct _wifi_channelStats {
	INT  ch_number;						/**< each channel is only 20MHz bandwidth */
	BOOL ch_in_pool; 	    			/**< If ch_in_pool is false, driver do not need to scan this channel */
	INT  ch_noise;		    			/**< this is used to return the average noise floor in dbm */
	BOOL ch_radar_noise;				/**< if ch_number is in DFS channel, this is used to return if radar signal is present on DFS channel (5G only) */
	INT  ch_max_80211_rssi;    			/**< max RSSI from the neighbor AP in dbm on this channel. */
	INT  ch_non_80211_noise;			/**< average non 802.11 noise */
	INT  ch_utilization;				/**< this is used to return the 802.11 utilization in percent */
	ULLONG ch_utilization_total; /**<  Total time radio spent receiveing or transmitting on that channel (ch_utilization_active) */
	ULLONG ch_utilization_busy; /**<  Time radio detected that channel was busy (Busy = Rx + Tx + Interference) */
	ULLONG ch_utilization_busy_tx; /**<  Time time radio spent transmitting on channel */
	ULLONG ch_utilization_busy_rx; /**<  Time radio spent receiving on channel (Rx = Rx_obss + Rx_self + Rx_errr (self and obss errors) */
	ULLONG ch_utilization_busy_self; /**< Time radio spend receiving on channel from its own connected clients */
	ULLONG ch_utilization_busy_ext; /**< Time radio detected that extended channel was busy (40MHz extention channel busy */
} wifi_channelStats_t;					//<!This data structure is for each channel

typedef struct _wifi_channelStats2 { 
	UINT 	ch_Frequency; 					/**< Current primary channel centre frequency */
	INT 	ch_NoiseFloor; 					/**< Current noise floor on channel */
	INT 	ch_Non80211Noise; 				/**< Current non 802.11 noise on channel */
	INT 	ch_Max80211Rssi; 				/**< Max RSSI from the neighbor AP in dbm on this channel */
	UINT 	ch_ObssUtil; 					/**< Other bss utilization for last interval */
	UINT 	ch_SelfBssUtil; 				/**< Self bss utilization for last interval */
} wifi_channelStats2_t;

//----------------ASSO. DEV-------------------------------------------
//>> Deprecated: used for old RDKB code. 
typedef struct _wifi_device
{
     UCHAR wifi_devMacAddress[6];
     CHAR wifi_devIPAddress[64];
     BOOL wifi_devAssociatedDeviceAuthentiationState;
     INT  wifi_devSignalStrength;
     INT  wifi_devTxRate;
     INT  wifi_devRxRate;
} wifi_device_t;
//<<

//Please do not edit the elements for this data structure
 /**
 * @brief Client information
 *
 * Structure which holds the device information associated with a particular  wifi access point.
 */

typedef struct _wifi_associated_dev
{
	UCHAR cli_MACAddress[6];		/**< The MAC address of an associated device. */
	CHAR  cli_IPAddress[64];		/**< IP of the associated device */
	BOOL  cli_AuthenticationState; /**< Whether an associated device has authenticated (true) or not (false). */
	UINT  cli_LastDataDownlinkRate; /**< The data transmit rate in kbps that was most recently used for transmission from the access point to the associated device. */
	UINT  cli_LastDataUplinkRate; 	/**< The data transmit rate in kbps that was most recently used for transmission from the associated device to the access point. */
	INT   cli_SignalStrength; 		/**< An indicator of radio signal strength of the uplink from the associated device to the access point, measured in dBm, as an average of the last 100 packets received from the device. */
	UINT  cli_Retransmissions; 	/**< The number of packets that had to be re-transmitted, from the last 100 packets sent to the associated device. Multiple re-transmissions of the same packet count as one. */
	BOOL  cli_Active; 				/**<	boolean	-	Whether or not this node is currently present in the WiFi AccessPoint network. */

	CHAR  cli_OperatingStandard[64];	/**< Radio standard the associated Wi-Fi client device is operating under. Enumeration of: */
	CHAR  cli_OperatingChannelBandwidth[64];	/**< The operating channel bandwidth of the associated device. The channel bandwidth (applicable to 802.11n and 802.11ac specifications only). Enumeration of: */
	INT   cli_SNR;		/**< A signal-to-noise ratio (SNR) compares the level of the Wi-Fi signal to the level of background noise. Sources of noise can include microwave ovens, cordless phone, bluetooth devices, wireless video cameras, wireless game controllers, fluorescent lights and more. It is measured in decibels (dB). */
	CHAR  cli_InterferenceSources[64]; /**< Wi-Fi operates in two frequency ranges (2.4 Ghz and 5 Ghz) which may become crowded other radio products which operate in the same ranges. This parameter reports the probable interference sources that this Wi-Fi access point may be observing. The value of this parameter is a comma seperated list of the following possible sources: eg: MicrowaveOven,CordlessPhone,BluetoothDevices,FluorescentLights,ContinuousWaves,Others */
	ULONG cli_DataFramesSentAck;	/**< The DataFramesSentAck parameter indicates the total number of MSDU frames marked as duplicates and non duplicates acknowledged. The value of this counter may be reset to zero when the CPE is rebooted. Refer section A.2.3.14 of CableLabs Wi-Fi MGMT Specification. */
	ULONG cli_DataFramesSentNoAck;	/**< The DataFramesSentNoAck parameter indicates the total number of MSDU frames retransmitted out of the interface (i.e., marked as duplicate and non-duplicate) and not acknowledged, but does not exclude those defined in the DataFramesLost parameter. The value of this counter may be reset to zero when the CPE is rebooted. Refer section A.2.3.14 of CableLabs Wi-Fi MGMT Specification. */
	ULONG cli_BytesSent;	/**< The total number of bytes transmitted to the client device, including framing characters. */
	ULONG cli_BytesReceived;	/**< The total number of bytes received from the client device, including framing characters. */
	INT   cli_RSSI;	/**< The Received Signal Strength Indicator, RSSI, parameter is the energy observed at the antenna receiver for transmissions from the device averaged over past 100 packets recevied from the device. */
	INT   cli_MinRSSI;	/**< The Minimum Received Signal Strength Indicator, RSSI, parameter is the minimum energy observed at the antenna receiver for past transmissions (100 packets). */
	INT   cli_MaxRSSI;	/**< The Maximum Received Signal Strength Indicator, RSSI, parameter is the energy observed at the antenna receiver for past transmissions (100 packets). */
	UINT  cli_Disassociations;	/**< This parameter  represents the total number of client disassociations. Reset the parameter evey 24hrs or reboot */
	UINT  cli_AuthenticationFailures;	/**< This parameter indicates the total number of authentication failures.  Reset the parameter evey 24hrs or reboot */

} wifi_associated_dev_t;	//~COSA_DML_WIFI_AP_ASSOC_DEVICE

typedef struct _wifi_associated_dev2
{
	mac_address_t cli_MACAddress;		/**< The MAC address of an associated device. */
	CHAR  cli_IPAddress[64];		/**< IP of the associated device */
	BOOL  cli_AuthenticationState; /**< Whether an associated device has authenticated (true) or not (false). */
	UINT  cli_LastDataDownlinkRate; /**< The data transmit rate in kbps that was most recently used for transmission from the access point to the associated device. */
	UINT  cli_LastDataUplinkRate; 	/**< The data transmit rate in kbps that was most recently used for transmission from the associated device to the access point. */
	INT   cli_SignalStrength; 		/**< An indicator of radio signal strength of the uplink from the associated device to the access point, measured in dBm, as an average of the last 100 packets received from the device. */
	UINT  cli_Retransmissions; 	/**< The number of packets that had to be re-transmitted, from the last 100 packets sent to the associated device. Multiple re-transmissions of the same packet count as one. */
	BOOL  cli_Active; 				/**<	boolean	-	Whether or not this node is currently present in the WiFi AccessPoint network. */

	CHAR  cli_OperatingStandard[64];	/**< Radio standard the associated Wi-Fi client device is operating under. Enumeration of: */
	CHAR  cli_OperatingChannelBandwidth[64];	/**< The operating channel bandwidth of the associated device. The channel bandwidth (applicable to 802.11n and 802.11ac specifications only). Enumeration of: */
	INT   cli_SNR;		/**< A signal-to-noise ratio (SNR) compares the level of the Wi-Fi signal to the level of background noise. Sources of noise can include microwave ovens, cordless phone, bluetooth devices, wireless video cameras, wireless game controllers, fluorescent lights and more. It is measured in decibels (dB). */
	CHAR  cli_InterferenceSources[64]; /**< Wi-Fi operates in two frequency ranges (2.4 Ghz and 5 Ghz) which may become crowded other radio products which operate in the same ranges. This parameter reports the probable interference sources that this Wi-Fi access point may be observing. The value of this parameter is a comma seperated list of the following possible sources: eg: MicrowaveOven,CordlessPhone,BluetoothDevices,FluorescentLights,ContinuousWaves,Others */
	ULONG cli_DataFramesSentAck;	/**< The DataFramesSentAck parameter indicates the total number of MSDU frames marked as duplicates and non duplicates acknowledged. The value of this counter may be reset to zero when the CPE is rebooted. Refer section A.2.3.14 of CableLabs Wi-Fi MGMT Specification. */
	ULONG cli_DataFramesSentNoAck;	/**< The DataFramesSentNoAck parameter indicates the total number of MSDU frames retransmitted out of the interface (i.e., marked as duplicate and non-duplicate) and not acknowledged, but does not exclude those defined in the DataFramesLost parameter. The value of this counter may be reset to zero when the CPE is rebooted. Refer section A.2.3.14 of CableLabs Wi-Fi MGMT Specification. */
	ULONG cli_BytesSent;	/**< The total number of bytes transmitted to the client device, including framing characters. */
	ULONG cli_BytesReceived;	/**< The total number of bytes received from the client device, including framing characters. */
	INT   cli_RSSI;	/**< The Received Signal Strength Indicator, RSSI, parameter is the energy observed at the antenna receiver for transmissions from the device averaged over past 100 packets recevied from the device. */
	INT   cli_MinRSSI;	/**< The Minimum Received Signal Strength Indicator, RSSI, parameter is the minimum energy observed at the antenna receiver for past transmissions (100 packets). */
	INT   cli_MaxRSSI;	/**< The Maximum Received Signal Strength Indicator, RSSI, parameter is the energy observed at the antenna receiver for past transmissions (100 packets). */
	UINT  cli_Disassociations;	/**< This parameter  represents the total number of client disassociations. Reset the parameter evey 24hrs or reboot */
	UINT  cli_AuthenticationFailures;	/**< This parameter indicates the total number of authentication failures.  Reset the parameter evey 24hrs or reboot */

	ULLONG   cli_Associations;	/**<  Stats handle used to determine reconnects; increases for every association (stat delta calcualtion) */
} wifi_associated_dev2_t;

/* 802.11ax HAL structure definitions */

#define     MAX_RU_ALLOCATIONS  74
#define     MAX_BSR 32

typedef enum {
	wifi_twt_agreement_type_individual,
	wifi_twt_agreement_type_broadcast,
} wifi_twt_agreement_type_t;

typedef struct {
	BOOL	implicit;
	BOOL	announced;
	BOOL	trigger_enabled;
} wifi_twt_operation_t;

typedef struct {
	UINT    wake_time;
    UINT    wake_interval;
    UINT    min_wake_duration;
    UINT    channel;
} wifi_twt_individual_params_t;

typedef struct {
	UINT    traget_beacon;
    UINT    listen_interval;
} wifi_twt_broadcast_params_t;

typedef struct {
	wifi_twt_agreement_type_t	agreement;
	wifi_twt_operation_t	operation;
	union {
		wifi_twt_individual_params_t	individual;
		wifi_twt_broadcast_params_t	broadcast;
	} patams;
} wifi_twt_params_t;


typedef struct {
	wifi_twt_params_t	twt_params;
} wifi_80211ax_params_t;

typedef enum {
    wifi_guard_interval_400,
    wifi_guard_interval_800,
    wifi_guard_interval_1600,
    wifi_guard_interval_3200,
    wifi_guard_interval_auto,
} wifi_guard_interval_t;

typedef enum {
    wifi_dl_data_ack_immediate,
    wifi_dl_data_block_ack_immediate,
    wifi_dl_data_block_ack_deferred,
} wifi_dl_data_ack_type_t;

typedef enum {
    WIFI_DL_MU_TYPE_NONE,
    WIFI_DL_MU_TYPE_HE,
    WIFI_DL_MU_TYPE_MIMO,
    WIFI_DL_MU_TYPE_HE_MIMO
} wifi_dl_mu_type_t;

typedef enum {
    WIFI_UL_MU_TYPE_NONE,
    WIFI_UL_MU_TYPE_HE,
} wifi_ul_mu_type_t;

typedef enum {
    WIFI_RU_TYPE_26,
    WIFI_RU_TYPE_52,
    WIFI_RU_TYPE_106,
    WIFI_RU_TYPE_242,
    WIFI_RU_TYPE_484,
    WIFI_RU_TYPE_996,
    WIFI_RU_TYPE_1024,
} wifi_ru_type_t;

typedef enum {
    wifi_access_category_background,
    wifi_access_category_best_effort,
    wifi_access_category_video,
    wifi_access_category_voice,
} wifi_access_category_t;

typedef struct {
    wifi_access_category_t  access_category;
    UINT        queue_size;
} wifi_bsr_t;


typedef struct {
    UCHAR   subchannels;
    wifi_ru_type_t  type;
} wifi_ru_allocation_t;

typedef struct {
    wifi_dl_mu_type_t   cli_DownlinkMuType;
    wifi_bsr_t              cli_BufferStatus[MAX_BSR];
    UCHAR                   cli_AllocatedDownlinkRuNum;
    wifi_ru_allocation_t    cli_DownlinkRuAllocations[MAX_RU_ALLOCATIONS];
} wifi_dl_mu_stats_t;

typedef struct {
    wifi_ul_mu_type_t   cli_UpinkMuType;
    UCHAR                   cli_ChannelStateInformation;
    wifi_bsr_t              cli_BufferStatus[MAX_BSR];
    UCHAR                   cli_AllocatedUplinkRuNum;
    wifi_ru_allocation_t    cli_UplinkRuAllocations[MAX_RU_ALLOCATIONS];
} wifi_ul_mu_stats_t;

#define MAX_NR                  8
#define MAX_NC                  4
#define MAX_SUB_CARRIERS        256
#define MAX_PILOTS              26

/* RSSI in each of received streams of the received frame */
typedef INT	wifi_streams_rssi_t	[MAX_NR];
/* CSI data for each subcarrier over Nc and Nr */
typedef UINT	wifi_carrier_data_t	[MAX_NR][MAX_NC];
/* CSI data over 80MHz BW */
typedef wifi_carrier_data_t		wifi_csi_matrix_t [MAX_SUB_CARRIERS];

typedef UCHAR	wifi_evm_data_t	[MAX_NC][MAX_NR];
typedef wifi_evm_data_t	wifi_evm_matrix_t[MAX_PILOTS];

/**
 * @brief This structure hold the information about the wifi interface.
 */
typedef struct _wifi_frame_info
{
	UCHAR	bw_mode;			/* Bit 0-3: 0:20MHz; 1:40MHz; 2:80MHz; 3:160MHz */
								/* Bit 4: 80+80MHz */
								/* Bit 4-7: 0:11n; 1:11ac */
	UCHAR	mcs;				/* Encoded as 11ac numbering */	
	UCHAR	Nr;					/* Number of antennas used to receive the frame */
	UCHAR	Nc;					/* Number of streams used to transmit the frame */
	wifi_streams_rssi_t	nr_rssi;	/* RSSI on each of Nr */
	USHORT	valid_mask;			/* Bit mask that determines which regions of CSI capture (tones) are valid. One bit represents 20MHz chunk. */
	USHORT	phy_bw;				/* VAP BW at the time of capture, indicated as 20, 40, 80, 160 */
	USHORT	cap_bw;				/* Frame BW at the time of capture */
	UINT	num_sc;				/* Number of subcarriers in the payload so that information can be used in conjunction with the number of streams to fully decode valid regions */
	UCHAR	decimation;			/* Value to indicate degree to which CSI matrix is decimated in terms of number of subcarriers present.*/
	UINT	channel;			/* Primary Channel of received frame */
	ULLONG	time_stamp;			/* PHY timestamp of CSI capture with at minimum millisecond	*/
								/* resolution. Ideally this can be resolved to a standard epoch */
								/* format with millisecond resolution. */
} wifi_frame_info_t;

/**
 * @brief This structure hold the information about the wifi interface.
 */
typedef struct _wifi_csi_data
{
	wifi_frame_info_t	frame_info;	/* as defined above */
	wifi_csi_matrix_t	csi_matrix;		/* The NC value representing the number of non-zero columns 
									in the H matrix is equal to the number of spatial streams in the 
									packet. The NR value representing the number of rows in the H matrix 
									is equal to the number of antennas at the receiver. 
									Irrespective of the NC and NR values, the output H matrix is always 
									of size 4x4. For example, if the frame uses 2 spatial streams 
									and the receiver has 3 antennas, NC=2, NR=3. 
									However, the H matrix will be of size 4x4 with a 3x2 sub-matrix 
									with non-zero values. Rest of the values of the matrix will be zero. */
	wifi_evm_matrix_t	evm_matrix;	/* Similar scheme to the CSI matrix, Nc represents the number of non-zero columns and Nr represents the number of nonzero rows. There are 16 elements to accommodate the full number of pilots in a 160 MHz capture. Each element is an EVM value for a pilot expressed in dB. */
} wifi_csi_data_t;

/**
 * @brief This structure hold the information about the wifi interface.
 */
typedef struct _wifi_associated_dev3
{
        mac_address_t cli_MACAddress;           /**< The MAC address of an associated device. */
        CHAR  cli_IPAddress[64];                /**< IP of the associated device  (deprecated, keep it empty) */
        BOOL  cli_AuthenticationState; /**< Whether an associated device has authenticated (true) or not (false). */
        UINT  cli_LastDataDownlinkRate; /**< The median PHY rate in Mbps of the most recent 16 unicast data frame transmissions from the access point to the associated device. */
        UINT  cli_LastDataUplinkRate;   /**< The median PHY rate in Mbps of the most recent 16 unicast data frame transmissions from the associated device to the access point. */
        INT   cli_SignalStrength;               /**< An indicator of radio signal strength of the uplink from the associated device to the access point, measured in dBm, as an average of the last 100 packets received from the device. */
        UINT  cli_Retransmissions;      /**< The number of packets that had to be re-transmitted, from the last 100 packets sent to the associated device. Multiple re-transmissions of the same packet count as one. */
        BOOL  cli_Active;                               /**<      boolean -       Whether or not this node is currently present in the WiFi AccessPoint network. */

        CHAR  cli_OperatingStandard[64];        /**< Radio standard the associated Wi-Fi client device is operating under. Enumeration of: */
        CHAR  cli_OperatingChannelBandwidth[64];        /**< The operating channel bandwidth of the associated device. The channel bandwidth (applicable to 802.11n and 802.11ac specifications only). Enumeration of: */
        INT   cli_SNR;          /**< A signal-to-noise ratio (SNR) compares the level of the Wi-Fi signal to the level of background noise. Sources of noise can include microwave ovens, cordless phone, bluetooth devices, wireless video cameras, wireless game controllers, fluorescent lights and more. It is measured in decibels (dB). */
        CHAR  cli_InterferenceSources[64]; /**< Wi-Fi operates in two frequency ranges (2.4 Ghz and 5 Ghz) which may become crowded other radio products which operate in the same ranges. This parameter reports the probable interference sources that this Wi-Fi access point may be observing. The value of this parameter is a comma seperated list of the following possible sources: eg: MicrowaveOven,CordlessPhone,BluetoothDevices,FluorescentLights,ContinuousWaves,Others */
        ULONG cli_DataFramesSentAck;    /**< The DataFramesSentAck parameter indicates the total number of MSDU frames marked as duplicates and non duplicates acknowledged. The value of this counter may be reset to zero when the CPE is rebooted. Refer section A.2.3.14 of CableLabs Wi-Fi MGMT Specification. */
        ULONG cli_DataFramesSentNoAck;  /**< The DataFramesSentNoAck parameter indicates the total number of MSDU frames retransmitted out of the interface (i.e., marked as duplicate and non-duplicate) and not acknowledged, but does not exclude those defined in the DataFramesLost parameter. The value of this counter may be reset to zero when the CPE is rebooted. Refer section A.2.3.14 of CableLabs Wi-Fi MGMT Specification. */
        ULONG cli_BytesSent;    /**< The total number of bytes transmitted to the client device, including framing characters. */
        ULONG cli_BytesReceived;        /**< The total number of bytes received from the client device, including framing characters. */
        INT   cli_RSSI; /**< The Received Signal Strength Indicator, RSSI, parameter is the energy observed at the antenna receiver for transmissions from the device averaged over past 100 packets recevied from the device. */
        INT   cli_MinRSSI;      /**< The Minimum Received Signal Strength Indicator, RSSI, parameter is the minimum energy observed at the antenna receiver for past transmissions (100 packets). */
        INT   cli_MaxRSSI;      /**< The Maximum Received Signal Strength Indicator, RSSI, parameter is the energy observed at the antenna receiver for past transmissions (100 packets). */
        UINT  cli_Disassociations;      /**< This parameter  represents the total number of client disassociations. Reset the parameter evey 24hrs or reboot */
        UINT  cli_AuthenticationFailures;       /**< This parameter indicates the total number of authentication failures.  Reset the parameter evey 24hrs or reboot */

        ULLONG   cli_Associations;      /**<  Stats handle used to determine reconnects; increases for every association (stat delta calcualtion) */

        ULONG cli_PacketsSent; /**< The total number of packets transmitted to the Associated Device. */
        ULONG cli_PacketsReceived; /**< The total number of packets received from the Associated Device. */
        ULONG cli_ErrorsSent; /**< The total number of outbound packets that could not be transmitted because of errors. These might be due to the number of retransmissions exceeding the retry limit, or from other causes. */
        ULONG cli_RetransCount; /**< The total number of transmitted packets which were retransmissions for each client on the vAP. Two retransmissions of the same packet results in this counter incrementing by two. Three retransmissions of the same packet results in this counter incrementing by three.... */
        ULONG cli_FailedRetransCount;  /**< The number of packets that were not transmitted successfully due to the number of retransmission attempts exceeding an 802.11 retry limit. */
        ULONG cli_RetryCount;  /**< The number of packets that were successfully transmitted after one or more retransmissions */
        ULONG cli_MultipleRetryCount; /**< The number of packets that were successfully transmitted after more than one retransmission. */

       UINT  cli_MaxDownlinkRate; /**< The Max data transmit rate in Mbps for the access point to the associated device. */
       UINT  cli_MaxUplinkRate;   /**<  The Max data transmit rate in Mbps for the associated device to the access point. */
       wifi_ul_mu_stats_t  cli_DownlinkMuStats;
       wifi_dl_mu_stats_t  cli_UplinkMuStats;
	   wifi_twt_params_t	cli_TwtParams;

	   /* To facilitate retrieval of CSI data for specific associated client, an existing RDK-B Wi-Fi HAL 
		function is being extended. In current implementation wifi_getApAssociatedDeviceDiagnosticResult3 
		retrieves variety of statistics and state specific information for associated clients. 
		The wifi_associated_dev3_t data structure is filled by native WLAN drivers for each associated client 
		as and when the function is called by RDK-B application/process. A new component structure 
		wifi_csi_data_t is being defined that is part of wifi_associated_dev3_t structure and needs to be 
		allocated and filled for specific client or list of clients when 
		wifi_getApAssociatedDeviceDiagnosticResult3 API is called by RDK-B application/process. In cases when 
		application needs CSI data, the RDK-B application will call 
		INT wifi_getApAssociatedDeviceDiagnosticResult3(INT apIndex, wifi_associated_dev3_t **associated_dev_array, UINT *output_array_size) by allocating the associated_dev_array memory for output_array_size number of client
		devices. In other words output_array_size will specify the number of client devices in the array for 
		which CSI data needs to filled by driver. The cli_MACAddress will specify the client devices in each
		of wifi_associated_dev3_t. Wi-Fi HAL implementation in such case MUST allocate memory for cli_CSIData
		fill in required fields. The called in such cases is reposnsible for deallocation of memory. 
		The wifi_csi_data_t is defined above */

	 	wifi_csi_data_t  *cli_CsiData; 
} wifi_associated_dev3_t;

/**
 * @brief RADIUS Server information.
 *
 * Structure which holds the the RADIUS server settings.
 */
typedef struct _wifi_radius_setting_t
{
	 INT  RadiusServerRetries; 			/**< Number of retries for Radius requests. */
	 INT  RadiusServerRequestTimeout; 	/**< Radius request timeout in seconds after which the request must be retransmitted for the # of retries available.	 */
	 INT  PMKLifetime; 					/**< Default time in seconds after which a Wi-Fi client is forced to ReAuthenticate (def 8 hrs).	 */
	 BOOL PMKCaching; 					/**< Enable or disable caching of PMK.	 */
	 INT  PMKCacheInterval; 			/**< Time interval in seconds after which the PMKSA (Pairwise Master Key Security Association) cache is purged (def 5 minutes).	 */
	 INT  MaxAuthenticationAttempts; 	/**< Indicates the # of time, a client can attempt to login with incorrect credentials. When this limit is reached, the client is blacklisted and not allowed to attempt loging into the network. Settings this parameter to 0 (zero) disables the blacklisting feature. */
	 INT  BlacklistTableTimeout; 		/**< Time interval in seconds for which a client will continue to be blacklisted once it is marked so.	*/
	 INT  IdentityRequestRetryInterval; /**< Time Interval in seconds between identity requests retries. A value of 0 (zero) disables it.	*/
	 INT  QuietPeriodAfterFailedAuthentication;  /**< The enforced quiet period (time interval) in seconds following failed authentication. A value of 0 (zero) disables it. */
	 //UCHAR RadiusSecret[64];			//<! The secret used for handshaking with the RADIUS server [RFC2865]. When read, this parameter returns an empty string, regardless of the actual value.
		 
} wifi_radius_setting_t;
/* MCS/NSS/BW rate table and indexes that shoul be used for supported rates
----------------------------------------------
| type | bw         | nss        |  mcs     
----------------------------------------------
| OFDM | 0 (20Mhz)  | 0 (legacy) |  0 - 6M 
|      |            |            |  1 - 9M 
|      |            |            |  2 - 12M 
|      |            |            |  3 - 18M 
|      |            |            |  4 - 24M 
|      |            |            |  5 - 36M 
|      |            |            |  6 - 48M 
|      |            |            |  7 - 54M
----------------------------------------------
| CCK  | 0 (20Mhz)  | 0 (legacy) |  8 - L1M 
|      |            |            |  9 - L2M 
|      |            |            | 10 - L5.5M
|      |            |            | 11 - L11M 
|      |            |            | 12 - S2M 
|      |            |            | 13 - S5.5M
|      |            |            | 14 - S11M"
----------------------------------------------
| VHT  | 0 (20Mhz)  | 1 (chain1) |  1 - HT/VHT
|      | 1 (40Mhz)  | ...        |  2 - HT/VHT
|      | 2 (80MHz)  | 8 (chain8) |  3 - HT/VHT
|      | 2 (160MHz) |            |  4 - HT/VHT
|      |            |            |  5 - HT/VHT
|      |            |            |  6 - HT/VHT
|      |            |            |  7 - HT/VHT
|      |            |            |  8 - VHT 
|      |            |            |  9 - VHT 
----------------------------------------------
NOTE: The size of this table on 4x4 can be big - we could send only non zero elements!
*/
typedef struct _wifi_associated_dev_rate_info_rx_stats {
        // rate table index see table above
	UCHAR nss; 					/**< 0 equals legacy protocolss (OFDM, CCK) 1 - n spatial stream (HT, VHT) */
	UCHAR mcs;						/**< 0 - 7 (HT) - 9 (VHT) */
	USHORT bw; 					/**< 20, 40, 80, 160 ... (to be considered 5 , 10, 80+80) ... */
	ULLONG flags;  				/**< Flag indicating data validation that HAS_BYTES, HAS_MSDUS, HAS_MPDUS, HAS_PPDUS, HAS_BW_80P80, HAS_RSSI_COMB, HAS_RSSI_ARRAY */
	ULLONG bytes;					/**< number of bytes received for given rate */
	ULLONG msdus;					/**< number of MSDUs received for given rate */
	ULLONG mpdus;					/**< number of MPDUs received for given rate */
	ULLONG ppdus;					/**< number of PPDUs received for given rate */
	ULLONG retries;				/**< number of retries received for given rate */
	UCHAR rssi_combined;			/**< Last RSSI received on give rate */
	/* Per antenna RSSI (above noise floor) for all widths (primary,secondary) 
		-----------------------------------------------
		| chain_num |  20MHz [pri20                   ]
		|           |  40MHZ [pri20,sec20             ] 
		|           |  80MHz [pri20,sec20,sec40,      ]
		|           | 160MHz [pri20,sec20,sec40,sec80 ]
		-----------------------------------------------
		|  1        |  rssi  [pri20,sec20,sec40,sec80 ]
		|  ...      |  ...
		|  8        |  rssi  [pri20,sec20,sec40,sec80 ]
		-----------------------------------------------	*/
	UCHAR rssi_array[8][4]; 		//<! 8=antennas, 4=20+20+40+80 extension rssi
} wifi_associated_dev_rate_info_rx_stats_t;

typedef struct _wifi_associated_dev_rate_info_tx_stats {
        // rate table index see table above
	UCHAR nss; 					/**< 0 equals legacy protocolss (OFDM, CCK) 1 - n spatial stream (HT, VHT) */
	UCHAR mcs;						/**< 0 - 7 (HT) - 9 (VHT) */
	USHORT bw; 					/**< 20, 40, 80, 160 ... (to be considered 5 , 10, 80+80) ... */
	ULLONG flags;  				/**< Flag indicating data validation that HAS_BYTES, HAS_MSDUS, HAS_MPDUS, HAS_PPDUS, HAS_BW_80P80, HAS_RSSI_COMB, HAS_RSSI_ARRAY */
	ULLONG bytes;					/**< number of bytes transmitted for given rate */
	ULLONG msdus;					/**< number of MSDUs transmitted for given rate */
	ULLONG mpdus;					/**< number of MPDUs transmitted for given rate */
	ULLONG ppdus;					/**< number of PPDUs transmitted for given rate */
	ULLONG retries;				/**< number of transmittion retries for given rate */
	ULLONG attempts;				/**< number of attempts trying transmitt on given rate */
} wifi_associated_dev_rate_info_tx_stats_t;

typedef struct athstat_results{
    u_int32_t    rtx_total;
    u_int32_t    tx_total;
    u_int32_t    rx_total;
    u_int32_t    rx_own;
    u_int32_t    d_rx_t; // delta rx_total
    u_int32_t    d_rx_o; // delta rx_own
} athstat_results_t;
/* AC/TID rate table
 ----------------------
|    TID   |    AC    |
-----------------------
|  0  | 8  |    BE    |  
|  1  | 9  |    BK    |
|  2  | 10 |    BK    |
|  3  | 11 |    BE    |
|  4  | 12 |    VI    |
|  5  | 13 |    VI    |
|  6  | 14 |    VO    |
|  7  | 15 |    VO    |
-----------------------
*/

typedef enum
{
    WIFI_RADIO_QUEUE_TYPE_VI = 0,
    WIFI_RADIO_QUEUE_TYPE_VO,
    WIFI_RADIO_QUEUE_TYPE_BE,
    WIFI_RADIO_QUEUE_TYPE_BK,
    WIFI_RADIO_QUEUE_TYPE_CAB,
    WIFI_RADIO_QUEUE_TYPE_BCN,
    WIFI_RADIO_QUEUE_MAX_QTY,
    WIFI_RADIO_QUEUE_TYPE_NONE = -1
} wifi_radioQueueType_t;

typedef enum
{
    WIFI_CSA_DEAUTH_MODE_NONE = 0,
    WIFI_CSA_DEAUTH_MODE_UCAST,
    WIFI_CSA_DEAUTH_MODE_BCAST
} wifi_csaDeauthMode_t;

typedef enum
{
    WIFI_SCANFILTER_MODE_DISABLED = 0,
    WIFI_SCANFILTER_MODE_ENABLED,
    WIFI_SCANFILTER_MODE_FIRST
} wifi_scanFilterMode_t;

typedef enum
{
    WIFI_MAC_ACL_MODE_DISABLED  = 0,
    WIFI_MAC_ACL_MODE_WHITELIST = 1,
    WIFI_MAC_ACL_MODE_BLACKLIST = 2
} wifi_macAclMode_t;

typedef struct wifi_associated_dev_tid_entry
{
    UCHAR  ac;						/**< BE, BK. VI, VO (wifi_radioQueueType_t) */
    UCHAR  tid;                       			/**< 0 - 16 */
    ULLONG ewma_time_ms;					/**< Moving average value based on last couple of transmitted msdus */
    ULLONG sum_time_ms;					/**< Delta of cumulative msdus times over interval */
    ULLONG num_msdus;					/**< Number of msdus in given interval */
} wifi_associated_dev_tid_entry_t;

typedef struct wifi_associated_dev_tid_stats
{
    wifi_associated_dev_tid_entry_t tid_array[16];
} wifi_associated_dev_tid_stats_t;

/*    Explanation:
                             these are actually 3 host-endian integers
                            in this example they are big-endian because
                             the piranha's host cpu is big-endian MIPS
                                    _____________|____________
                                   /             |            \
                                  |              |            |
                             _____|______    ____|____    ____|_____
                            |            |  |         |  |          |
     ap1       glastackrssi:75  74  73  77  2  3  68  1  0  0  0  136
                            ^^^^^^^^^^^^^^  ^^^^^^^^^^^  ^^^^^^^^^^^^
                                  |              |            |
                         last 4 rssi values      |      sample counter
                                                 |
                                         last 4 rssi's age
    
                                the "77" rssi is 1 second old
                                         ______|______
                                        /             \
                                        |             |
     ap1       glastackrssi:75  74  73  77  2  3  68  1  0  0  0  136
                                     |             |
                                     \____________/
                                           |
                                 the 2nd most recent rssi of "73"
                                 is 68 seconds old *in relation*
                                 to the 1st ("77") therefore it is
                                 68 + 1 seconds old *now*   */
typedef struct _wifi_rssi_snapshot {
	UCHAR  rssi[4];                       		/**< Last 4 RSSI frames received */
	UCHAR  time_s[4];                                  /**< Time of when last 4 RSSI were received */
	USHORT count;                                      /**< Sequence numer of received managemant (bcn, ack) frames  */
} wifi_rssi_snapshot_t;

typedef struct _wifi_associated_dev_stats {
	ULLONG 	cli_rx_bytes;				/**< The total number of bytes received from the client device, including framing characters. */
	ULLONG 	cli_tx_bytes;				/**< The total number of bytes transmitted to the client device, including framing characters. */
	ULLONG 	cli_rx_frames;				/**< The total number of frames received from the client */
	ULLONG 	cli_tx_frames;				/**< The total number of frames transmitted to the client */
	ULLONG 	cli_rx_retries;				/**< Number of rx retries */
	ULLONG 	cli_tx_retries;				/**< Number of tx retries. cli_Retransmissions */
	ULLONG 	cli_rx_errors;				/**< Number of numer of rx error */
	ULLONG 	cli_tx_errors;				/**< Number of tx errors */
	double 	cli_rx_rate;					/**< average rx data rate used */
	double 	cli_tx_rate;					/**< average tx data rate used} wifi_associated_dev_t; */
	wifi_rssi_snapshot_t cli_rssi_bcn;      /**< RSSI from last 4 beacons received (STA) */
	wifi_rssi_snapshot_t cli_rssi_ack;      /**< RSSI from last 4 ack received     (AP) */
} wifi_associated_dev_stats_t;	

/** @} */  //END OF GROUP WIFI_HAL_TYPES 

/**
 * @addtogroup WIFI_HAL_APIS
 * @{
 */

//SURVEY CHANNEL
/* wifi_getWifiChannelStats() function */
/**
* @brief Get the channels utilization status.
*
* @param[in]      radioIndex                      The index of the radio
* @param[in, out] input_output_channelStats_array The array initially filled with requested channel numbers.
*                                                 The same array is used as an output with channel statistics
*                                                 details. Data for each channel must be written to the corresponding
*                                                 element of the array. When array_size = 0, the API returns ONCHAN
*                                                 stats in a single wifi_channelStats_t element.
* @param[out]     array_size                      The length of the output array
*
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Get the basic Radio channel traffic static info
INT wifi_getRadioChannelStats(INT radioIndex, wifi_channelStats_t *input_output_channelStats_array, INT array_size);

INT wifi_getRadioChannelStats2(INT radioIndex, wifi_channelStats2_t *outputChannelStats2);


/* wifi_getApAssociatedDeviceRxStatsResult() function */
/**
* @brief Get the associated client per rate receive status.
*
* @param [in]  radioIndex          The index of radio array.
* @param [in]  clientMacAddress    Client mac address UCHAR[6]
* @param [out] stats_array         Client receive status
* @param [out] output_array_size   The length of output array
* @param [out] handle              Status validation handle used to determine reconnects;
*                                  increases for every association.
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getApAssociatedDeviceRxStatsResult(INT radioIndex, mac_address_t *clientMacAddress, wifi_associated_dev_rate_info_rx_stats_t **stats_array, UINT *output_array_size, ULLONG *handle);

/**
* @brief Get the associated client per rate transmission status.
*
* @param [in]  radioIndex          The index of radio array.
* @param [in]  clientMacAddress    Client mac address UCHAR[6]
* @param [out] stats_array         Client transmission status
* @param [out] output_array_size   The length of output array
* @param [out] handle              Status validation handle used to determine reconnects;
*                                  increases for every association.
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/

INT wifi_getApAssociatedDeviceTxStatsResult(INT radioIndex, mac_address_t *clientMacAddress, wifi_associated_dev_rate_info_tx_stats_t **stats_array, UINT *output_array_size, ULLONG *handle);

/* wifi_getApAssociatedDeviceTidStatsResult() function */
/**
* @brief Get the associated client per rate transmission status.
*
* @param [in]  radioIndex         The index of radio array
* @param [in]  clientMacAddress   client mac address UCHAR[6]
* @param [out] stats              wifi_associated_dev_tid_stats_t *stats, client status
* @param [in]  handle             Status validation handle used to determine reconnects
*                                 incremented for every association
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getApAssociatedDeviceTidStatsResult(INT radioIndex,  mac_address_t *clientMacAddress, wifi_associated_dev_tid_stats_t *tid_stats,  ULLONG *handle);

/**
* @brief Get the associated device  status.
*
* @param [in]  apIndex               The index of access point array
* @param [in]  clientMacAddress      client mac address UCHAR[6]
* @param [out] associated_dev_stats  Associated device status
* @param [in]  handle                Status validation handle used to determine reconnects;
*                                    increases for every association
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getApAssociatedDeviceStats(INT apIndex, mac_address_t *clientMacAddress, wifi_associated_dev_stats_t *associated_dev_stats, ULLONG *handle);	


/******************************************************************/
/******************************************************************/


//---------------------------------------------------------------------------------------------------
/* wifi_getHalVersion() function */
/**
* @brief Get the wifi hal version in string.
*
* Eg "2.0.0". WIFI_HAL_MAJOR_VERSION.WIFI_HAL_MINOR_VERSION.WIFI_HAL_MAINTENANCE_VERSION
*
* @param[out] output_string  WiFi Hal version to be returned.
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @sideeffect None
*/
//Wifi system api
//Get the wifi hal version in string, eg "2.0.0".  WIFI_HAL_MAJOR_VERSION.WIFI_HAL_MINOR_VERSION.WIFI_HAL_MAINTENANCE_VERSION
INT wifi_getHalVersion(CHAR *output_string);   //RDKB   

//---------------------------------------------------------------------------------------------------
//
// Wifi subsystem level APIs that are common to Client and Access Point devices.
//
//---------------------------------------------------------------------------------------------------

/* wifi_factoryReset() function */
/**
* @brief Clears internal variables to implement a factory reset of the Wi-Fi subsystem.
*
* A Specific implementation may dictate some functionalities since different hardware implementations
* may have different requirements.
*
* @param None
*
* @return The status of the operation.
* @retval RETURN_OK if successful.
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//clears internal variables to implement a factory reset of the Wi-Fi subsystem
INT wifi_factoryReset();	//RDKB

/* wifi_factoryResetRadios() function */
/**
* @brief Restore all radio parameters without touching access point parameters.
*
* A Specific implementation may dictate some functionalities since different hardware implementations
* may have different requirements.
*
* @param None
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
*
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Restore all radio parameters without touch access point parameters
INT wifi_factoryResetRadios(); //RDKB

/* wifi_factoryResetRadio() function */
/**
* @brief Restore selected radio parameters without touching access point parameters.
*
* @param radioIndex  Index of Wi-Fi Radio channel
*
* @return The status of the operation.
* @retval RETURN_OK if successful.
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous.
* @sideeffect None.
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Restore selected radio parameters without touch access point parameters
INT wifi_factoryResetRadio(int radioIndex); 	//RDKB

/* wifi_setLED() function */
/**
* @brief Set the system LED status
*
* @param radioIndex  Index of Wi-Fi Radio channel
* @param enable      LED status
*
* @return The status of the operation.
* @retval RETURN_OK if successful.
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous.
* @sideeffect None.
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Set the system LED status
INT wifi_setLED(INT radioIndex, BOOL enable);	//RDKB

/* wifi_init() function */
/**
* @brief This function call initializes all Wi-Fi radios.
*
* A specific implementation  may dictate some functionality since different hardware implementations
* may have different initilization requirements.
*
* @param None
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
// Initializes the wifi subsystem (all radios)
INT wifi_init();                              //RDKB

/* wifi_reset() function */
/**
* @brief Resets the Wifi subsystem.
* This includes reset of all Access Point variables.
*
* Implementation specifics may dictate what is actualy reset since different hardware
* implementations may have different requirements.
*
* @param None
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
// resets the wifi subsystem, deletes all APs
INT wifi_reset();                            //RDKB

/* wifi_down() function */
/**
* @brief Turns off transmit power for the entire Wifi subsystem, for all radios.
*
* Implementation specifics may dictate some functionality since
* different hardware implementations may have different requirements.
*
* @param None
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
// turns off transmit power for the entire Wifi subsystem, for all radios
INT wifi_down();                       		//RDKB

/* wifi_createInitialConfigFiles() function */
/**
* @brief This function creates wifi configuration files.
*
* The format and content of these files are implementation dependent.  This function call is
* used to trigger this task if necessary. Some implementations may not need this
* function. If an implementation does not need to create config files the function call can
* do nothing and return RETURN_OK.
*
* @param None
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_createInitialConfigFiles();                                                                                    

/* wifi_getRadioCountryCode() function */
/**
* @brief Outputs the country code to a max 64 character string
*
* @param[in]  radioIndex      Index of Wi-Fi radio channel
* @param[out] output_string   Country code to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getRadioCountryCode(INT radioIndex, CHAR *output_string);

/* wifi_setRadioCountryCode() function */
/**
* @brief Set the country code for selected Wi-Fi radio channel.
*
* @param[in] radioIndex    Index of Wi-Fi radio channel
* @param[in] CountryCode   Country code
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setRadioCountryCode(INT radioIndex, CHAR *CountryCode);       

/* wifi_pushCountryCode() function */
/**
* @brief Set the country code for both wifi radios and apply them. wifi reset automatically if necessary.
*        The function need to return immediately.
*
* @param[in] CountryCode   Country code
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_pushCountryCode(CHAR *CountryCode);

//---------------------------------------------------------------------------------------------------
//Wifi Tr181 API

//Device.WiFi.

//---------------------------------------------------------------------------------------------------
// Air Time Management HAL.
//---------------------------------------------------------------------------------------------------
/**
* @brief Get the ATM(Air Time Management) Capable.
*
* Device.WiFi.X_RDKCENTRAL-COM_ATM_Capable	boolean	R
*
* @param[out] output_bool  Indication as to whether  Air Time Management is supported.
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getATMCapable(BOOL *output_bool);

/**
* @brief Set ATM Enable.
*
* The type of algorithm to apply across the configured Access Points and/or clients;
* Device.WiFi.X_RDKCENTRAL-COM_ATM_Enable	uint	W
*
* @param[in]  enable  Boolean value to set/unset ATM.
*                     False indicates  Disabled
                      True  indicates  Dynamic (Sharing of unused Airtime Between AP Groups allowed)
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setATMEnable(BOOL enable); 

/**
* @brief Get ATM Enable status.
*
* Device.WiFi.X_RDKCENTRAL-COM_ATM_Enable	uint	W
*
* @param[out] output_enable  Returns the ATM enable status.
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getATMEnable(BOOL *output_enable); 

//Device.WiFi.X_RDKCENTRAL-COM_ATM_NumberAPGroups	uint	R	default to 8

//Device.WiFi.X_RDKCENTRAL-COM_ATM_APGroup.{i}.	objectA grouping of Access Points and the percentage of Airtime Assigned to them.
//Device.WiFi.X_RDKCENTRAL-COM_ATM_APGroup.{i}.APList 			String	W	Comma Separated List of AP Indexes assigned to this group. apList= "1,2"  ap index is start from 0
//Device.WiFi.X_RDKCENTRAL-COM_ATM_APGroup.{i}.AirTimePercent	uint	W The Percentage of Available Airtime assigned to this ATM AP Group (5%-100%) The sum of all percentages assigned to all groups must be <= 100%"
/**
* @brief Set Access Point Air Time Percent.
*
* Device.WiFi.X_RDKCENTRAL-COM_ATM_APGroup.{i}.AirTimePercent	uint	W
*
* @param[in]  apIndex             Index of Access Point array.
* @param[in]  ap_AirTimePercent   The Percentage of Available Airtime assigned to this ATM Access Point Group (5%-100%)
*                                 The sum of all percentages assigned to all groups must be <= 100%"
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setApATMAirTimePercent(INT apIndex, UINT ap_AirTimePercent); 

/**
* @brief Get Ap Air Time Percent.
*
* Device.WiFi.X_RDKCENTRAL-COM_ATM_APGroup.{i}.AirTimePercent	uint	W
*
* @param[in]   apIndex                    The index  of Access Point array.
* @param[out]  output_ap_AirTimePercent   The Percentage of Available Airtime assigned to this
*                                         ATM Access Point Group (5%-100%)
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getApATMAirTimePercent(INT apIndex, UINT *output_ap_AirTimePercent);

//Device.WiFi.X_RDKCENTRAL-COM_ATM_APGroup.{i}.NumberSta		uint	R The number of assured throughput Clients configured for ATM
/**
* @brief Get the list for Air Time Percent for each Station.
*
* @param[in]  apIndex                   The index of Access Point array.
* @param[out] output_sta_MAC_ATM_array  Caller allocated buffer.
*										output_sta_MAC_ATM_array contains the atm array in format
*									    of "$MAC $ATM_percent|$MAC $ATM_percent|$MAC $ATM_percent"
* @param[out] buf_size                  The size for output_sta_MAC_ATM_array
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getApATMSta(INT apIndex, UCHAR *output_sta_MAC_ATM_array, UINT  buf_size);  //output_sta_MAC_ATM_array contains the atm array in format of "$MAC $ATM_percent|$MAC $ATM_percent|$MAC $ATM_percent"
																				//buf_size is the size for output_sta_MAC_ATM_array

//Device.WiFi.X_RDKCENTRAL-COM_ATM_APGroup.{i}.Sta.{i}	object		
//Device.WiFi.X_RDKCENTRAL-COM_ATM_APGroup.{i}.Sta.{i}.MAC	string(18)	W	[MACAddress] The MAC Address to which the Following Configuration Applies
//Device.WiFi.X_RDKCENTRAL-COM_ATM_APGroup.{i}.Sta.{i}.AirTimePercent	uint	W	The Percentage of Available Airtime assigned to this ATM within an AP Group for this client.
/**
* @brief Set Air Time Percent for each Station.
* 
* @param[in] apIndex          The index of Access Point array
* @param[in] sta_MAC          If sta_MAC is new, HAL need to add this new record into ATM table for this AP; 
*                             if sta_MAC is not new, HAL need to change sta_AirTimePercent for this MAC in ATM table for this AP.
* @param sta_AirTimePercent   if sta_AirTimePercent is 0, HAL needd to remove this recordC from the ATM table
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setApATMSta(INT apIndex, UCHAR *sta_MAC, UINT sta_AirTimePercent);  	//if sta_MAC is new, HAL need to add this new record into ATM table for this AP
																			//if sta_MAC is not new, HAL need to change sta_AirTimePercent for this MAC in ATM table for this AP
																			//if sta_AirTimePercent is 0, HAL needd to remove this recordC from the ATM table
//Air Time Management HAL end
																			
/* wifi_getRadioNumberOfEntries() function */
/**
* @brief Get the total number of radios in this wifi subsystem.
*
* @param[out] output  Total number of radios to be returned.
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.RadioNumberOfEntries
//Get the total number of radios in this wifi subsystem
INT wifi_getRadioNumberOfEntries(ULONG *output); //Tr181

/* wifi_getSSIDNumberOfEntries() function */
/**
* @brief Get the total number of SSID entries in this wifi subsystem.
*
* @param[out] output Total number of SSID entries to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.SSIDNumberOfEntries
//Get the total number of SSID entries in this wifi subsystem 
INT wifi_getSSIDNumberOfEntries(ULONG *output); //Tr181

//Device.WiFi.AccessPointNumberOfEntries

//Device.WiFi.EndPointNumberOfEntries
//End points are managed by RDKB
//INT wifi_getEndPointNumberOfEntries(INT radioIndex, ULONG *output); //Tr181

//---------------------------------------------------------------------------------------------------
//
// Wifi radio level APIs that are common to Client and Access Point devices
//
//---------------------------------------------------------------------------------------------------

//Device.WiFi.Radio.

/* wifi_getRadioEnable() function */
/**
* @brief Get the Radio enable config parameter.
*
* @param[in]  radioIndex   Index of Wi-Fi radio channel
* @param[out] output_bool  Radio Enable status, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.Radio.{i}.Enable
//Get the Radio enable config parameter
INT wifi_getRadioEnable(INT radioIndex, BOOL *output_bool);	//RDKB

/* wifi_setRadioEnable() function */
/**
* @brief Set the Radio enable config parameter.
*
* @param[in]  radioIndex   Index of Wi-Fi radio channel
* @param[in]  enable       Set the selected radio's status as Enable/Disable
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Set the Radio enable config parameter
INT wifi_setRadioEnable(INT radioIndex, BOOL enable);		//RDKB

/* wifi_getRadioStatus() function */
/**
* @brief Get the Radio enable status.
*
* @param[in]   radioIndex    Index of Wi-Fi radio channel
* @param[out]  output_bool   Selected radio's enable status, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.Radio.{i}.Status
//Get the Radio enable status
INT wifi_getRadioStatus(INT radioIndex, BOOL *output_bool);	//RDKB

/* wifi_getRadioIfName() function */
/**
* @brief Get the Radio Interface name from platform, eg "wifi0".
*
* @param radioIndex      Index of Wi-Fi radio channel
* @param output_string   Interface name, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.Radio.{i}.Alias

//Device.WiFi.Radio.{i}.Name
//Get the Radio Interface name from platform, eg "wifi0"
INT wifi_getRadioIfName(INT radioIndex, CHAR *output_string); //Tr181

//Device.WiFi.Radio.{i}.LastChange

//Device.WiFi.Radio.{i}.LowerLayers

//Device.WiFi.Radio.{i}.Upstream

/* wifi_getRadioMaxBitRate() function */
/**
* @brief Get the maximum PHY bit rate supported by this interface. eg: "216.7 Mb/s", "1.3 Gb/s".
*
* The output_string is a max length 64 octet string that is allocated by the RDKB code.
* Implementations must ensure that strings are not longer than this.
*
* @param[in]   radioIndex     Index of Wi-Fi radio channel.
* @param[out]  output_string  Maximum bit rate supported, to be returned.
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.Radio.{i}.MaxBitRate
//Get the maximum PHY bit rate supported by this interface. eg: "216.7 Mb/s", "1.3 Gb/s"
//The output_string is a max length 64 octet string that is allocated by the RDKB code.  Implementations must ensure that strings are not longer than this.
INT wifi_getRadioMaxBitRate(INT radioIndex, CHAR *output_string);	//RDKB

/* wifi_getRadioSupportedFrequencyBands() function */
/**
* @brief Get Supported frequency bands at which the radio can operate. eg: "2.4GHz,5GHz".
*
* The output_string is a max length 64 octet string that is allocated by the RDKB code.  
* Implementations must ensure that strings are not longer than this.
* 
* @param[in]  radioIndex     Index of Wi-Fi radio channel
* @param[out] output_string  Supported frequency bands, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.Radio.{i}.SupportedFrequencyBands
//Get Supported frequency bands at which the radio can operate. eg: "2.4GHz,5GHz"
//The output_string is a max length 64 octet string that is allocated by the RDKB code.  Implementations must ensure that strings are not longer than this.
INT wifi_getRadioSupportedFrequencyBands(INT radioIndex, CHAR *output_string);	//RDKB

/* wifi_getRadioOperatingFrequencyBand() function */
/**
* @brief Get the frequency band at which the radio is operating, eg: "2.4GHz".
*
* The output_string is a max length 64 octet string that is allocated by the RDKB code.
* Implementations must ensure that strings are not longer than this.
*
* @param[in]  radioIndex      Index of Wi-Fi radio channel.
* @param[out] output_string   Operating frequency band, to be returned.
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.Radio.{i}.OperatingFrequencyBand
//Get the frequency band at which the radio is operating, eg: "2.4GHz"
//The output_string is a max length 64 octet string that is allocated by the RDKB code.  Implementations must ensure that strings are not longer than this.
INT wifi_getRadioOperatingFrequencyBand(INT radioIndex, CHAR *output_string); //Tr181

/* wifi_getRadioSupportedStandards() function */
/**
* @brief Get the Supported Radio Mode. eg: "b,g,n"; "n,ac"; "ax"; "a,n,ac,ax".
*
* The output_string is a max length 64 octet string that is allocated by the RDKB code.
* Implementations must ensure that strings are not longer than this.
*
* @param[in]  radioIndex      Index of Wi-Fi radio channel.
* @param[out] output_string   Supported radio mode, to be returned.
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.Radio.{i}.SupportedStandards
//Get the Supported Radio Mode. eg: "b,g,n"; "n,ac"; "ax"; "a,n,ac,ax"
//The output_string is a max length 64 octet string that is allocated by the RDKB code.  Implementations must ensure that strings are not longer than this.
INT wifi_getRadioSupportedStandards(INT radioIndex, CHAR *output_string); //Tr181

/** Deprecated: used for old RDKB code. **/
/* wifi_getRadioStandard() function */
/**
* @brief Get the radio operating mode, and pure mode flag. eg: "ac".
*
* The output_string is a max length 64 octet string that is allocated by the RDKB code.
* Implementations must ensure that strings are not longer than this.
*
* @param[in]  radioIndex      Index of Wi-Fi radio channel
* @param[out] output_string   Radio operating mode, to be returned
* @param[out] gOnly           Boolean pointer variable need to be updated based on the "output_string"
* @param[out] nOnly           Boolean pointer variable need to be updated based on the "output_string"
* @param[out] acOnly          Boolean pointer variable need to be updated based on the "output_string"
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.Radio.{i}.OperatingStandards
//Get the radio operating mode, and pure mode flag. eg: "ac"
//The output_string is a max length 64 octet string that is allocated by the RDKB code.  Implementations must ensure that strings are not longer than this.
INT wifi_getRadioStandard(INT radioIndex, CHAR *output_string, BOOL *gOnly, BOOL *nOnly, BOOL *acOnly);	//RDKB

/* wifi_getRadioMode() function */
/**
* @brief Get the radio operating mode, and pure mode flag. eg: "ac".
*
* The output_string is a max length 64 octet string that is allocated by the RDKB code.
* Implementations must ensure that strings are not longer than this.
*
* @param[in]  radioIndex      Index of Wi-Fi radio channel
* @param[out] output_string   Radio operating mode, to be returned
* @param[out] pureMode        Pointer to pure mode bit map starting from LSB b only, g only, a only, 
* 						  	  n only, ac only, ax only, e.g. n only will be 8, ax only will be 32
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.Radio.{i}.OperatingStandards
//Get the radio operating mode, and pure mode flag. eg: "ac"
//The output_string is a max length 64 octet string that is allocated by the RDKB code.  Implementations must ensure that strings are not longer than this.
INT wifi_getRadioMode(INT radioIndex, CHAR *output_string, UINT *pureMode);	//RDKB

/** Deprecated: used for old RDKB code. **/
/* wifi_setRadioChannelMode() function */
/**
* @brief Set the radio operating mode, and pure mode flag.
*
* @param[in] radioIndex   Index of Wi-Fi radio channel
* @param[in] channelMode  Pass the channelMode for specified radio index
* @param[in] gOnlyFlag    Pass operating mode flag for setting pure mode flag
* @param[in] nOnlyFlag    Pass operating mode flag for setting pure mode flag
* @param[in] acOnlyFlag   Pass operating mode flag for setting pure mode flag
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Set the radio operating mode, and pure mode flag.
INT wifi_setRadioChannelMode(INT radioIndex, CHAR *channelMode, BOOL gOnlyFlag, BOOL nOnlyFlag, BOOL acOnlyFlag);	//RDKB

/* wifi_setRadioMode() function */
/**
* @brief Set the radio operating mode, and pure mode flag.
*
* @param[in] radioIndex   Index of Wi-Fi radio channel
* @param[in] channelMode  Pass the channelMode for specified radio index
* @param[in] pureMode     Pass flag for setting pure mode bit map starting from LSB b only, g only, a only, 
* 						  n only, ac only, ax only, e.g. n only will be 8, ax only will be 32
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Set the radio operating mode, and pure mode flag.
INT wifi_setRadioMode(INT radioIndex, CHAR *channelMode, UINT pureMode);	//RDKB

/* wifi_getRadioPossibleChannels() function */
/**
* @brief Get the list of supported channel. eg: "1-11".
*
* The output_string is a max length 64 octet string that is allocated by the RDKB code.
* Implementations must ensure that strings are not longer than this.
*
* @param[in]  radioIndex      Index of Wi-Fi radio channel
* @param[out] output_string   List of supported radio channels, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.Radio.{i}.PossibleChannels
//Get the list of supported channel. eg: "1-11"
//The output_string is a max length 128 octet string that is allocated by the RDKB code.  Implementations must ensure that strings are not longer than this.
INT wifi_getRadioPossibleChannels(INT radioIndex, CHAR *output_string);	//RDKB

/* wifi_getRadioChannelsInUse() function */
/**
* @brief Get the list of supported channel. eg: "1-11".
*
* The output_string is a max length 64 octet string that is allocated by the RDKB code.
* Implementations must ensure that strings are not longer than this.
*
* @param[in]  radioIndex     Index of Wi-Fi radio channel
* @param[out] output_string  List of supported radio channels, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.Radio.{i}.ChannelsInUse
//Get the list for used channel. eg: "1,6,9,11"
//The output_string is a max length 256 octet string that is allocated by the RDKB code.  Implementations must ensure that strings are not longer than this.
INT wifi_getRadioChannelsInUse(INT radioIndex, CHAR *output_string);	//RDKB

/* wifi_getRadioChannel() function */
/**
* @brief Get the running channel number.
*
* @param[in]  radioIndex     Index of Wi-Fi radio channel
* @param[out] output_ulong   Running channel number, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.Radio.{i}.Channel
INT wifi_getRadioChannel(INT radioIndex,ULONG *output_ulong);	//RDKB

/* wifi_setRadioChannel() function */
/**
* @brief Set the running channel number.
*
* @param[in] radioIndex  Index of Wi-Fi radio channel
* @param[in] channel     Channel number to be set as running wifi radio channel
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setRadioChannel(INT radioIndex, ULONG channel);	//RDKB	//AP only

/* wifi_setRadioAutoChannelEnable() function */
/**
* @brief Enables or disables a driver level variable to indicate if auto channel selection is enabled on this radio.
*
* This "auto channel" means the auto channel selection when radio is up.
* (which is different from the dynamic channel/frequency selection (DFC/DCS))
*
* @param[in] radioIndex  Index of Wi-Fi radio channel
* @param[in] enable      Enable/Disable selected radio channel as auto channel radio
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Enables or disables a driver level variable to indicate if auto channel selection is enabled on this radio
//This "auto channel" means the auto channel selection when radio is up. (which is different from the dynamic channel/frequency selection (DFC/DCS))
INT wifi_setRadioAutoChannelEnable(INT radioIndex, BOOL enable); //RDKB

/* wifi_getRadioAutoChannelSupported() function */
/**
* @brief Check if the driver support the AutoChannel.
*
* Device.WiFi.Radio.{i}.AutoChannelSupported
*
* @param[in]  radioIndex   Index of Wi-Fi radio channel
* @param[out] output_bool  Value of Auto Channel Supported, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.Radio.{i}.AutoChannelSupported
//Check if the driver support the AutoChannel
INT wifi_getRadioAutoChannelSupported(INT radioIndex, BOOL *output_bool); //Tr181

/* wifi_getRadioAutoChannelEnable() function */
/**
* @brief Get the AutoChannel enable status.
*
* @param[in]  radioIndex   Index of Wi-Fi radio channel
* @param[out] output_bool  Auto Channel Enabled status, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Get the AutoChannel enable status
INT wifi_getRadioAutoChannelEnable(INT radioIndex, BOOL *output_bool);	//Tr181

/* wifi_setRadioAutoChannelEnable() function */
/**
* @brief Enables or disables a driver level variable to indicate if auto channel selection is enabled on this radio.
*
* This "auto channel" means the auto channel selection when radio is up.
* (which is different from the dynamic channel/frequency selection (DFC/DCS))
*
* @param[in] radioIndex  Index of Wi-Fi radio channel.
* @param[in] enable      Enable/Disable selected radio channel as auto channel radio
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Set the AutoChannel enable status
INT wifi_setRadioAutoChannelEnable(INT radioIndex, BOOL enable);	//Tr181

/* wifi_getRadioDCSSupported() function */
/**
* @brief Check if the driver support the DCS.
*
* Device.WiFi.Radio.{i}.X_COMCAST-COM_DCSSupported.
*
* @param[in]  radioIndex   Index of Wi-Fi radio channel
* @param[out] output_bool  DCS Supported flag for the radio index, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.Radio.{i}.X_COMCAST-COM_DCSSupported
//Check if the driver support the DCS
INT wifi_getRadioDCSSupported(INT radioIndex, BOOL *output_bool); 	//RDKB

/* wifi_getRadioDCSEnable() function */
/**
* @brief Get DCS of the selected wifi radio channel's enable/disable status.
*
* Device.WiFi.Radio.{i}.X_COMCAST-COM_DCSEnable
*
* @param[in]  radioIndex   Index of Wi-Fi radio channel
* @param[out] output_bool  DCS Enable flag for the selected radio index, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.Radio.{i}.X_COMCAST-COM_DCSEnable
INT wifi_getRadioDCSEnable(INT radioIndex, BOOL *output_bool);		//RDKB

/* wifi_setRadioDCSEnable() function */
/**
* @brief Enable/Disable selected wifi radio channel's DCS.
*
* Device.WiFi.Radio.{i}.X_COMCAST-COM_DCSEnable
*
* @param[in]  radioIndex  Index of Wi-Fi radio channel
* @param[out] enable      Set the value of DCS Enable flag for the selected radio index
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setRadioDCSEnable(INT radioIndex, BOOL enable);			//RDKB

/* wifi_getRadioDCSChannelPool() function */
/**
* @brief Get radio DCS channel pool.
*
* The output_string is a max length 256 octet string that is allocated by the RDKB code.
* Implementations must ensure that strings are not longer than this.
* The value of this parameter is a comma seperated list of channel number.
*
* @param[in]  radioIndex   Index of Wi-Fi radio channel
* @param[out] output_pool  DCS channel pool for the selected radio index,to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//The output_string is a max length 256 octet string that is allocated by the RDKB code.  Implementations must ensure that strings are not longer than this.
//The value of this parameter is a comma seperated list of channel number
INT wifi_getRadioDCSChannelPool(INT radioIndex, CHAR *output_pool);			//RDKB

/* wifi_setRadioDCSChannelPool() function */
/**
* @brief Set radio DCS channel pool.
*
* The output_string is a max length 256 octet string that is allocated by the RDKB code.  Implementations must ensure that strings are not longer than this.
* The value of this parameter is a comma seperated list of channel number.
*
* @param[in] radioIndex Index of Wi-Fi radio channel
* @param[in] pool       Set DCS channel pool for the selected radio index
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setRadioDCSChannelPool(INT radioIndex, CHAR *pool);			//RDKB

/* wifi_getRadioDCSScanTime() function */
/**
* @brief Get radio DCS scan time.
*
* @param[in]  radioIndex                 Index of Wi-Fi radio channel
* @param[out] output_interval_seconds    Get the interval time in seconds
* @param[out] output_dwell_milliseconds  Get the dwell time in milliseconds
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getRadioDCSScanTime(INT radioIndex, INT *output_interval_seconds, INT *output_dwell_milliseconds);

/* wifi_setRadioDCSScanTime() function */
/**
* @brief Set radio DCS scan time.
*
* @param[in]  radioIndex          Index of Wi-Fi radio channel
* @param[in]  interval_seconds    Set the interval time in seconds
* @param[in]  dwell_milliseconds  Set the dwell time in milliseconds
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setRadioDCSScanTime(INT radioIndex, INT interval_seconds, INT dwell_milliseconds);

/** @} */  //END OF GROUP WIFI_HAL_APIS

/**
 * @addtogroup WIFI_HAL_TYPES
 * @{
 */

//---------------------------------------------------------------------------------------------------
// Dynamic Channel Selection (phase 2) HAL.
//---------------------------------------------------------------------------------------------------
typedef struct _wifi_apRssi {  
	CHAR  ap_BSSID[6];    		/**< BSSID */
	UINT  ap_channelWidth;   	/**< The channel width; 1 for 20Mhz, 2 for 40 MHz, 4 for 80 MHz, 8 for 160 MHz, 10 for 80+80Mhz */
	INT   ap_rssi;       		/**< RSSI of the neighboring AP in dBm. */
} wifi_apRssi_t;

typedef struct _wifi_channelMetrics {
	INT  channel_number;		/**< Each channel is only 20MHz bandwidth */
	BOOL channel_in_pool; 	    /**< If channel_in_pool is false, driver do not need to scan this channel */
	INT  channel_noise;		    /**< This is used to return the average noise floor in dbm */
	BOOL channel_radar_noise;	/**< If channel_number is in DFS channel, this is used to return if radar signal is present on DFS channel (5G only) */
	INT  channel_non_80211_noise;			/**< Average non 802.11 noise */
	INT  channel_utilization;	/**< This is used to return the 802.11 utilization in percent */
	INT  channel_txpower;		/**< This is used to return the current txpower in dbm on this channel	*/

	wifi_apRssi_t channel_rssi_list[64];	/**< RSSI list from the neighbor AP on this channel. The list should be sorted descendly based on ap_rssi. If there are more than 64 AP on this channel, return first 64.  */
	UINT channel_rssi_count; 	/**< RSSI counter in channel_rssi_list */
} wifi_channelMetrics_t;

/** @} */  //END OF GROUP WIFI_HAL_TYPES  
 
//Device.WiFi.Radio.i.X_RDKCENTRAL-COM_DCSEnable	boolean	W	
//Indication as to whether DCS is enabled
//INT wifi_setRadioDcsScanning(INT radioIndex, BOOL enable_background_scanning);
//INT wifi_getRadioDcsScanning(INT radioIndex, BOOL *output_enable_background_scanning);


/**
 * @addtogroup WIFI_HAL_APIS
 * @{
 */

/**
* @brief Set radio Dcs Dwell time.
*
* Device.WiFi.Radio.{i}.X_RDKCENTRAL-COM_DCSDwelltime	integer	W	
*
* @param[in]  radioIndex   Index of Wi-Fi radio 
* @param[in]  millisecond  Dwell time on each channel
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setRadioDcsDwelltime(INT radioIndex, INT millisecond); 

/**
* @brief Get radio Dcs Dwell time.
*
* Device.WiFi.Radio.{i}.X_RDKCENTRAL-COM_DCSDwelltime	integer	W	
*
* @param[in] radioIndex           Index of Wi-Fi radio 
* @param[in] output_millisecond   Dwell time on each channel
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getRadioDcsDwelltime(INT radioIndex, INT *output_millisecond);

/* wifi_setRadioDcsScanning() function */
/**
* @brief Enable/Disable selected wifi radio channel's DCS.
*
* Device.WiFi.Radio.{i}.X_RDKCENTRAL_COM_DCSEnable
*
* @param[in] radioIndex  Index of Wi-Fi radio channel
* @param[in] enable      Set the value of DCS Enable flag for the selected radio index
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setRadioDcsScanning(INT radioIndex, BOOL enable);                        //RDKB

/* wifi_getRadioDcsScanning() function */
/**
* @brief Get DCS of the selected wifi radio channel's enable/disable status.
*
* Device.WiFi.Radio.{i}.X_RDKCENTRAL_COM_DCSEnable
*
* @param[in]  radioIndex   Index of Wi-Fi radio channel
* @param[out] output_bool  DCS Enable flag for the selected radio index, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getRadioDcsScanning(INT radioIndex, BOOL *output_bool);          //RDKB


//Device.WiFi.Radio.i.X_RDKCENTRAL-COM_DCSHighChannelUsageThreshold	integer	W

/**
* @brief Get radio Channel Metrics.
*
* @param[in]  radioIndex                          Index of Wi-Fi radio 
* @param[out] input_output_channelMetrics_array   caller allocated buffer
* @param[out] array_size                          The count for wifi_channelMetrics_t that caller allocated
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and should not invoke any blocking system
* calls. This is blocking call.
*
*/
INT wifi_getRadioDcsChannelMetrics(INT radioIndex, wifi_channelMetrics_t *input_output_channelMetrics_array, INT array_size);

/**
* @brief Instantlly change the radio Channel.
*
* Use Channels Switch Announcements (CSAs) (in 802.11h) to notify the client, 
* and channel change instantly. Do not save wifi config (channel change is not 
* persistent over wifi reboot).
*
* @param[in] radioIndex  Index of Wi-Fi radio 
* @param[in] channel     net channel
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and should not invoke any blocking system 
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_pushRadioChannel(INT radioIndex, UINT channel);
//Dynamic Channel Selection (phase 2) HAL END

/**
* @brief  This HAL API is used to change the channel to destination channel, with destination bandwidth.
*
* @param[in] radioIndex          Index of Wi-Fi radio 
* @param[in] channel             net channel
* @param[in] channel_width_MHz   channel frequency
* @param[in] csa_beacon_count    Specifies how long CSA need to be announced.
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and should not invoke any blocking system 
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_pushRadioChannel2(INT radioIndex, UINT channel, UINT channel_width_MHz, UINT csa_beacon_count);

/* wifi_getRadioDfsSupport() function */
/**
* @brief Get radio DFS support.
*
* Device.WiFi.Radio.{i}.X_COMCAST-COM_DfsSupported
*
* @param[in]  radioIndex   Index of Wi-Fi radio channel
* @param[out] output_bool  Get DFS support for the selected radio index in the pre-allocated buffer
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.Radio.{i}.X_COMCAST-COM_DfsSupported
//Get radio DFS support
INT wifi_getRadioDfsSupport(INT radioIndex, BOOL *output_bool);		//RDKB

/* wifi_getRadioDfsEnable() function */
/**
* @brief Get the Dfs enable status.
*
* Data model parameter used to check the DFS enable status is,
* Device.WiFi.Radio.{i}.X_COMCAST-COM_DfsEnable
*
* @param[in]  radioIndex   Index of Wi-Fi radio channel
* @param[out] output_bool  Get DFS Enable status of the selected radio channel
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.Radio.{i}.X_COMCAST-COM_DfsEnable					
INT wifi_getRadioDfsEnable(INT radioIndex, BOOL *output_bool);		//RDKB				

/* wifi_setRadioDfsEnable() function */
/**
* @brief Set the Dfs enable status.
*
* Data model parameter used to check the DFS enable status is "Device.WiFi.Radio.{i}.X_COMCAST-COM_DfsEnable".
*
* @param[in] radioIndex  Index of Wi-Fi radio channel
* @param[in] enable      Set DFS Enable status of the selected radio channel
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setRadioDfsEnable(INT radioIndex, BOOL enabled);			//RDKB				

/* wifi_getRadioAutoChannelRefreshPeriodSupported() function */
/**
* @brief Check if the driver support the AutoChannelRefreshPeriod.
*
* @param[in]  radioIndex   Index of Wi-Fi radio channel
* @param[out] output_bool  Get auto channel refresh period support for the selected radio channel
*                          in the pre-allocated bool buffer.
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.Radio.{i}.AutoChannelRefreshPeriod
INT wifi_getRadioAutoChannelRefreshPeriodSupported(INT radioIndex, BOOL *output_bool); //Tr181

/* wifi_getRadioAutoChannelRefreshPeriod() function */
/**
* @brief Get the DCS refresh period in seconds.
*
* @param[in]  radioIndex    Index of Wi-Fi radio channel
* @param[out] output_ulong  The refresh period.
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getRadioAutoChannelRefreshPeriod(INT radioIndex, ULONG *output_ulong); //Tr181

/* wifi_setRadioAutoChannelRefreshPeriod() function */
/**
* @brief Set the DCS refresh period in seconds.
*
* @param[in] radioIndex  Index of Wi-Fi radio channel
* @param[in] seconds     Set auto channel refresh period in seconds support for the selected radio channel.
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setRadioAutoChannelRefreshPeriod(INT radioIndex, ULONG seconds); //Tr181

/* wifi_getRadioOperatingChannelBandwidth() function */
/**
* @brief Get the Operating Channel Bandwidth. eg "20MHz", "40MHz", "80MHz", "80+80", "160".
*
* The output_string is a max length 64 octet string that is allocated by the RDKB code.
* Implementations must ensure that strings are not longer than this.
*
* @param[in]  radioIndex      Index of Wi-Fi radio channel
* @param[out] output_string   Get operating channel bandwidth for the selected radio channel in the pre-allocated char buffer.
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.Radio.{i}.OperatingChannelBandwidth
INT wifi_getRadioOperatingChannelBandwidth(INT radioIndex, CHAR *output_string); //Tr181

/* wifi_setRadioOperatingChannelBandwidth() function */
/**
* @brief Set the Operating Channel Bandwidth. eg "20MHz", "40MHz", "80MHz", "80+80", "160".
*
* @param[in] radioIndex  Index of Wi-Fi radio channel
* @param[in] bandwidth   Set operating channel bandwidth for the selected radio channel
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setRadioOperatingChannelBandwidth(INT radioIndex, CHAR *bandwidth); //Tr181	//AP only

/* wifi_getRadioExtChannel() function */
/**
* @brief Get the secondary extension channel position.
*
* "AboveControlChannel" or "BelowControlChannel". (this is for 40MHz and 80MHz bandwith only).
* The output_string is a max length 64 octet string that is allocated by the RDKB code.
* Implementations must ensure that strings are not longer than this.
* Device.WiFi.Radio.{i}.ExtensionChannel
*
* @param[in]  radioIndex     Index of Wi-Fi radio channel
* @param[out] output_string  Secondary extension channel position, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getRadioExtChannel(INT radioIndex, CHAR *output_string); //Tr181

/* wifi_setRadioExtChannel() function */
/**
* @brief Set the secondary extension channel position.
*
* "AboveControlChannel" or "BelowControlChannel". (this is for 40MHz and 80MHz bandwith only).
* Device.WiFi.Radio.{i}.ExtensionChannel
*
* @param[in] radioIndex  Index of Wi-Fi radio channel
* @param[in] string      Secondary extension channel position
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setRadioExtChannel(INT radioIndex, CHAR *string); //Tr181	//AP only

/* wifi_getRadioGuardInterval() function */
/**
* @brief Get the guard interval value. eg "400nsec" or "800nsec".
*
* The output_string is a max length 64 octet string that is allocated by the RDKB code. 
* Implementations must ensure that strings are not longer than this.
* Device.WiFi.Radio.{i}.GuardInterval
*
* @param[in]  radioIndex     Index of Wi-Fi radio channel
* @param[out] output_string  Guard interval value, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.Radio.{i}.GuardInterval
INT wifi_getRadioGuardInterval(INT radioIndex, CHAR *output_string);	//Tr181

/* wifi_setRadioGuardInterval() function */
/**
* @brief Set the guard interval value. eg "400nsec" or "800nsec".
*
* Device.WiFi.Radio.{i}.GuardInterval
*
* @param[in] radioIndex  Index of Wi-Fi radio channel
* @param[in] string      Guard interval value
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setRadioGuardInterval(INT radioIndex, CHAR *string);	//Tr181

/* wifi_getRadioMCS() function */
/**
* @brief Get the Modulation Coding Scheme index, eg: "-1", "1", "15".
*
* Device.WiFi.Radio.{i}.MCS
*
* @param[in]  radioIndex  Index of Wi-Fi radio channel
* @param[out] output_INT  Modulation Coding Scheme index, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.Radio.{i}.MCS
INT wifi_getRadioMCS(INT radioIndex, INT *output_INT); //Tr181

/* wifi_setRadioMCS() function */
/**
* @brief Set the Modulation Coding Scheme index, eg: "-1", "1", "15".
*
* Device.WiFi.Radio.{i}.MCS
*
* @param[in] radioIndex  Index of Wi-Fi radio channel
* @param[in] MCS         Modulation Coding Scheme index value
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setRadioMCS(INT radioIndex, INT MCS); //Tr181

/* wifi_getRadioTransmitPowerSupported() function */
/**
* @brief Get supported Transmit Power list, eg : "0,25,50,75,100".
*
* The output_list is a max length 64 octet string that is allocated by the RDKB code.
* Implementations must ensure that strings are not longer than this.
* Device.WiFi.Radio.{i}.TransmitPowerSupported
*
* @param[in]  radioIndex   Index of Wi-Fi radio channel
* @param[out] output_list  Transmit power list, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.Radio.{i}.TransmitPowerSupported
INT wifi_getRadioTransmitPowerSupported(INT radioIndex, CHAR *output_list); //Tr181

/* wifi_getRadioTransmitPower() function */
/**
* @brief Get current Transmit Power in dBm units.
*
* The transmit power value is in dBm units of full power for this radio.
* Device.WiFi.Radio.{i}.TransmitPower
*
* @param[in]  radioIndex   Index of Wi-Fi radio channel
* @param[out] output_ulong Current Transmit power value, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getRadioTransmitPower(INT radioIndex, ULONG *output_ulong);	//RDKB

/* wifi_getRadioPercentageTransmitPower() function E.g : "75" "100"*/
/**
* @brief Get current Transmit Power level in units of full power.
*
* The transmit power is a percentage value of full power for this radio.
*
* @param[in]  radioIndex   Index of Wi-Fi radio channel
* @param[out] output_ulong Current Transmit power percentage value, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getRadioPercentageTransmitPower(INT radioIndex, ULONG *output_ulong);    //RDKB

/* wifi_setRadioTransmitPower() function */
/**
* @brief Set current Transmit Power, eg "75", "100".
*
* The transmit power level is in units of full power for this radio.
* Device.WiFi.Radio.{i}.TransmitPower
*
* @param[in] radioIndex      Index of Wi-Fi radio channel
* @param[in] TransmitPower   Transmit power value
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setRadioTransmitPower(INT radioIndex, ULONG TransmitPower);	//RDKB

/* wifi_getRadioIEEE80211hSupported() function */
/**
* @brief Get 80211h Supported. 
* 
* 80211h solves interference with satellites and radar using the same 5 GHz frequency band.
* Device.WiFi.Radio.{i}.IEEE80211hSupported
*
* @param[in]  radioIndex  Index of Wi-Fi radio channel
* @param[out] Supported   80211h Supported, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getRadioIEEE80211hSupported(INT radioIndex, BOOL *Supported);  //Tr181

/* wifi_getRadioIEEE80211hEnabled() function */
/**
* @brief Get 80211h feature enable.
*
* Device.WiFi.Radio.{i}.IEEE80211hEnabled
*
* @param[in]  radioIndex  Index of Wi-Fi radio channel
* @param[out] enable      80211h feature enable, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getRadioIEEE80211hEnabled(INT radioIndex, BOOL *enable);  //Tr181

/* wifi_setRadioIEEE80211hEnabled() function */
/**
* @brief Set 80211h feature enable.
*
* Device.WiFi.Radio.{i}.IEEE80211hEnabled
*
* @param[in] radioIndex  Index of Wi-Fi radio channel
* @param[in] enable      80211h feature enable
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setRadioIEEE80211hEnabled(INT radioIndex, BOOL enable);  //Tr181

/* wifi_getRadioCarrierSenseThresholdRange() function */
/**
* @brief Indicates the Carrier Sense ranges supported by the radio.
*
* It is measured in dBm. Refer section A.2.3.2 of CableLabs Wi-Fi MGMT Specification.
* Device.WiFi.Radio.{i}.RegulatoryDomain
* Device.WiFi.Radio.{i}.X_COMCAST-COM_CarrierSenseThresholdRange		
*
* @param[in]  radioIndex  Index of Wi-Fi radio channel
* @param[out] output      Carrier sense threshold range, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getRadioCarrierSenseThresholdRange(INT radioIndex, INT *output);  //P3

/* wifi_getRadioCarrierSenseThresholdInUse() function */
/**
* @brief The RSSI signal level at which CS/CCA detects a busy condition.
* 
* This attribute enables Access Points to increase minimum sensitivity to avoid detecting busy condition
* from multiple/weak Wi-Fi sources in dense Wi-Fi environments.
* It is measured in dBm. Refer section A.2.3.2 of CableLabs Wi-Fi MGMT Specification.
* Device.WiFi.Radio.{i}.X_COMCAST-COM_CarrierSenseThresholdInUse
*
* @param[in]  radioIndex  Index of Wi-Fi radio channel
* @param[out] output      Carrier sense threshold in use, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getRadioCarrierSenseThresholdInUse(INT radioIndex, INT *output);	//P3

/* wifi_setRadioCarrierSenseThresholdInUse() function */
/**
* @brief Set Carrier sense threshold in use for the selected radio index.
*
* The RSSI signal level at which CS/CCA detects a busy condition.
* This attribute enables Access Point to increase minimum sensitivity to avoid detecting busy condition
* from multiple/weak Wi-Fi sources in dense Wi-Fi environments. It is measured in dBm.
* Device.WiFi.Radio.{i}.X_COMCAST-COM_CarrierSenseThresholdInUse
*
* @param[in] radioIndex  Index of Wi-Fi radio channel
* @param[in] threshold   Carrier sense threshold, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setRadioCarrierSenseThresholdInUse(INT radioIndex, INT threshold);	//P3

//Device.WiFi.Radio.{i}.X_COMCAST-COM_ChannelSwitchingCount
//This parameter indicates the total number of Channel Changes.  Reset the parameter every 24 hrs or reboot
//INT wifi_getRadioChannelSwitchingCount(INT radioIndex, INT *output); 	//P3


/* wifi_getRadioBeaconPeriod() function */
/**
* @brief Gets the time interval between transmitting beacons (expressed in milliseconds).
*
* This parameter is based ondot11BeaconPeriod from [802.11-2012].
* Device.WiFi.Radio.{i}.BeaconPeriod
*
* @param[in]  radioIndex  Index of Wi-Fi radio channel
* @param[out] output      Radio Beacon period, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.Radio.{i}.BeaconPeriod
INT wifi_getRadioBeaconPeriod(INT radioIndex, UINT *output);

/* wifi_setRadioBeaconPeriod() function */
/**
* @brief Sets the time interval between transmitting beacons (expressed in milliseconds).
*
* This parameter is based ondot11BeaconPeriod from [802.11-2012].
* Device.WiFi.Radio.{i}.BeaconPeriod
*
* @param[in] radioIndex    Index of Wi-Fi radio channel
* @param[in] BeaconPeriod  Radio Beacon period
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setRadioBeaconPeriod(INT radioIndex, UINT BeaconPeriod); 


/* wifi_getRadioBasicDataTransmitRates() function */
/**
* @brief Get the set of data rates, in Mbps.
*
* This has to be supported by all stations that desire to join this BSS.
* The stations have to be able to receive and transmit at each of the data rates listed inBasicDataTransmitRates.
* For example, a value of "1,2", indicates that stations support 1 Mbps and 2 Mbps.
* Most control packets use a data rate in BasicDataTransmitRates.
* Device.WiFi.Radio.{i}.BasicDataTransmitRates
*
* @param[in]  radioIndex  Index of Wi-Fi radio channel
* @param[out] output      Comma-separated list of strings, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.Radio.{i}.BasicDataTransmitRates
//Comma-separated list of strings. The set of data rates, in Mbps, that have to be supported by all stations that desire to join this BSS. The stations have to be able to receive and transmit at each of the data rates listed inBasicDataTransmitRates. For example, a value of "1,2", indicates that stations support 1 Mbps and 2 Mbps. Most control packets use a data rate in BasicDataTransmitRates.	
INT wifi_getRadioBasicDataTransmitRates(INT radioIndex, CHAR *output);

/* wifi_setRadioBasicDataTransmitRates() function */
/**
* @brief Set the data rates, in Mbps.
*
* This have to be supported by all stations that desire to join this BSS.
* The stations have to be able to receive and transmit at each of the data rates listed inBasicDataTransmitRates. 
* For example, a value of "1,2", indicates that stations support 1 Mbps and 2 Mbps.
* Most control packets use a data rate in BasicDataTransmitRates.
* Device.WiFi.Radio.{i}.BasicDataTransmitRates
*
* @param[in] radioIndex     Index of Wi-Fi radio channel
* @param[in] TransmitRates  Comma-separated list of strings
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setRadioBasicDataTransmitRates(INT radioIndex, CHAR *TransmitRates);

/* wifi_getRadioSupportedDataTransmitRates() function */
/**
* @brief Get the supported data transmit rates in Mbps.
*
* That have to be supported by all stations that desire to join this BSS.
* The stations have to be able to receive and transmit at each of the data rates listed in SupportedDataTransmitRates.
* For example, a value of "1,2", indicates that stations support 1 Mbps and 2 Mbps.
* Most control packets use a data rate in SupportedDataTransmitRates
* Device.WiFi.Radio.{i}.SupportedDataTransmitRates
*
* @param[in]  radioIndex    Index of Wi-Fi radio channel
* @param[out] output_rates  Comma-separated list of strings, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Contains a comma separated string of supported rates supported by the radio instance. Must include all rates in the acceptance criteria belonging to the corresponding radio configuration.	
INT wifi_getRadioSupportedDataTransmitRates(INT radioIndex, char *output_rates);

/* getRadioOperationalDataTransmitRates() function */
/**
* @brief Get the set of data rates, in Mbps, that have to be supported by all stations that desire to join this BSS.
*
* The stations have to be able to receive and transmit at each of the data rates listed inOperationalDataTransmitRates.
* For example, a value of "1,2", indicates that stations support 1 Mbps and 2 Mbps.
* Most control packets use a data rate in OperationalDataTransmitRates.
* Device.WiFi.Radio.{i}.OperationalDataTransmitRates
*
* @param[in]  radioIndex    Index of Wi-Fi radio channel
* @param[out] output_rates  Comma-separated list of strings, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.Radio.{i}.OperationalDataTransmitRates
//Contains a comman separated string of operational rates supported by the radio instance. Is either equal to the set of supported rates or a super set.	
INT wifi_getRadioOperationalDataTransmitRates(INT radioIndex, char *output_rates);		


/* wifi_setRadioOperationalDataTransmitRates() function */
/**
 * @brief Set the data rates, in Mbps, that have to be supported by all stations that desire to join this BSS.
 *
 * The stations should be able to receive and transmit at each of the data rates listed in OperationalDataTransmitRates.
 * For example, a value of "1,2", indicates that stations support 1 Mbps and 2 Mbps.
 * Most control packets use a data rate in OperationalDataTransmitRates.
 * Device.WiFi.Radio.{i}.OperationalDataTransmitRates
 *
 * @param[in]  radioIndex  Index of Wi-Fi radio channel
 * @param[out] rates       Comma-separated list of strings
 *
 * @return The status of the operation
 * @retval RETURN_OK if successful
 * @retval RETURN_ERR if any error is detected
 *
 * @execution Synchronous
 * @sideeffect None
 *
 * @note This function must not suspend and must not invoke any blocking system
 * calls. It should probably just send a message to a driver event handler task.
 *
 */
INT wifi_setRadioOperationalDataTransmitRates(INT radioIndex, char *rates);



//---------------------------------------------------------------------------------------------------
//Device.WiFi.Radio.{i}.Stats.

//Device.WiFi.Radio.{i}.Stats.BytesSent
//Device.WiFi.Radio.{i}.Stats.BytesReceived
//Device.WiFi.Radio.{i}.Stats.PacketsSent
//Device.WiFi.Radio.{i}.Stats.PacketsReceived
//Device.WiFi.Radio.{i}.Stats.ErrorsSent
//Device.WiFi.Radio.{i}.Stats.ErrorsReceived
//Device.WiFi.Radio.{i}.Stats.DiscardPacketsSent
//Device.WiFi.Radio.{i}.Stats.DiscardPacketsReceived
//Device.WiFi.Radio.{i}.Stats.PLCPErrorCount
//Device.WiFi.Radio.{i}.Stats.FCSErrorCount
//Device.WiFi.Radio.{i}.Stats.InvalidMACCount
//Device.WiFi.Radio.{i}.Stats.PacketsOtherReceived
//Device.WiFi.Radio.{i}.Stats.X_COMCAST-COM_NoiseFloor
//Device.WiFi.Radio.{i}.Stats.X_COMCAST-COM_ChannelUtilization
//Device.WiFi.Radio.{i}.Stats.X_COMCAST-COM_ActivityFactor
//Device.WiFi.Radio.{i}.Stats.X_COMCAST-COM_CarrierSenseThreshold_Exceeded
//Device.WiFi.Radio.{i}.Stats.X_COMCAST-COM_RetransmissionMetirc
//Device.WiFi.Radio.{i}.Stats.X_COMCAST-COM_MaximumNoiseFloorOnChannel
//Device.WiFi.Radio.{i}.Stats.X_COMCAST-COM_MinimumNoiseFloorOnChannel
//Device.WiFi.Radio.{i}.Stats.X_COMCAST-COM_MedianNoiseFloorOnChannel
//Device.WiFi.Radio.{i}.Stats.X_COMCAST-COM_StatisticsStartTime

/* wifi_getRadioTrafficStats2() function */
/**
* @brief Get detail radio traffic static info.
*
* @param[in]  radioIndex      Index of Wi-Fi radio channel
* @param[out] output_struct   wifi_radioTrafficStats2_t *output_struct, all traffic stats info to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getRadioTrafficStats2(INT radioIndex, wifi_radioTrafficStats2_t *output_struct); //Tr181

/* wifi_setRadioTrafficStatsMeasure() function */
/**
* @brief Set radio traffic static Measuring rules.
*
* Device.WiFi.Radio.{i}.Stats.X_COMCAST-COM_RadioStatisticsMeasuringRate
* Device.WiFi.Radio.{i}.Stats.X_COMCAST-COM_RadioStatisticsMeasuringInterval
*
* @param[in] radioIndex    Index of Wi-Fi radio channel
* @param[in] input_struct  wifi_radioTrafficStatsMeasure_t *input_struct, traffic stats measure info
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.Radio.{i}.Stats.X_COMCAST-COM_RadioStatisticsMeasuringRate
//Device.WiFi.Radio.{i}.Stats.X_COMCAST-COM_RadioStatisticsMeasuringInterval
INT wifi_setRadioTrafficStatsMeasure(INT radioIndex, wifi_radioTrafficStatsMeasure_t *input_struct); //Tr181

/* wifi_setRadioTrafficStatsRadioStatisticsEnable() function */
/**
* @brief Set radio traffic statistics enable.
*
* Device.WiFi.Radio.{i}.Stats.X_COMCAST-COM_RadioStatisticsEnable bool writable
*
* @param[in]  radioIndex  Index of Wi-Fi radio channel
* @param[out] enable      Enable/disable, traffic stats statistics
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setRadioTrafficStatsRadioStatisticsEnable(INT radioIndex, BOOL enable);

//-----------------------------------------------------------------------------------------------
/* wifi_getRadioStatsReceivedSignalLevel() function */
/**
* @brief Clients associated with the AP over a specific interval.
*
* The histogram MUST have a range from -110to 0 dBm and MUST be divided in bins of 3 dBM, with bins aligning on the -110 dBm 
* end of the range.
* Received signal levels equal to or greater than the smaller boundary of a bin and less than the larger boundary are included
* in the respective bin.
* The bin associated with the clients current received signal level MUST be incremented when a client associates with the AP.
* Additionally, the respective bins associated with each connected clients current received signal level MUST be incremented at
* the interval defined by "Radio Statistics Measuring Rate".
* The histogram  bins MUST NOT be incremented at any other time.
* The histogram data collected during the interval MUST be published to the parameter only at the end of the interval defined by 
* "Radio Statistics Measuring Interval".
* The underlying histogram data MUST be cleared at the start of each interval defined by "Radio Statistics Measuring Interval".
* If any of the parameter's representing this histogram is queried before the histogram has been updated with an initial set of
* data, it MUST return -1.
* Units dBm.
* Device.WiFi.Radio.{i}.Stats.X_COMCAST-COM_ReceivedSignalLevel.{i}.ReceivedSignalLevel
*
* @param[in] radioIndex   Index of Wi-Fi radio channel
* @param[in] signalIndex  Signal index
* @param[out]SignalLevel  Signal level, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.Radio.{i}.Stats.X_COMCAST-COM_ReceivedSignalLevel.{i}.

INT wifi_getRadioStatsReceivedSignalLevel(INT radioIndex, INT signalIndex, INT *SignalLevel); //Tr181

//-----------------------------------------------------------------------------------------------------
/* wifi_applyRadioSettings() function */
/**
* @brief This API is used to apply (push) all previously set radio level variables and make these settings active in the hardware.
*
* Not all implementations may need this function.
* If not needed for a particular implementation simply return no-error (0).
*
* @param[in] radioIndex  Index of Wi-Fi radio channel
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_applyRadioSettings(INT radioIndex);  


/* wifi_getRadioResetCount() function */
/**
* @brief Get the radio reset count.
*
* @param[in]  radioIndex  Index of Wi-Fi radio channel
* @param[out] output_int  Reset count, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getRadioResetCount(INT radioIndex, ULONG *output_int);

//---------------------------------------------------------------------------------------------------
//
// Wifi SSID level APIs common to Client and Access Point devices.
//
//---------------------------------------------------------------------------------------------------

//Device.WiFi.SSID.{i}.

/* wifi_getSSIDRadioIndex() function */
/**
* @brief Get the radio index associated with the SSID entry.
*
* @param[in]  ssidIndex   SSID index
* @param[out] radioIndex  Radio index, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getSSIDRadioIndex(INT ssidIndex, INT *radioIndex);

/* wifi_getSSIDEnable() function */
/**
* @brief Get SSID enable configuration parameters (not the SSID enable status).
*
* Device.WiFi.SSID.{i}.Enable
*
* @param[in]  ssidIndex   SSID index
* @param[out] output_bool SSID enable, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getSSIDEnable(INT ssidIndex, BOOL *output_bool); //Tr181

/* wifi_setSSIDEnable() function */
/**
* @brief Set SSID enable configuration parameters.
*
* Device.WiFi.SSID.{i}.Enable
*
* @param[in] ssidIndex  SSID index
* @param[in] enable     SSID enable value
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setSSIDEnable(INT ssidIndex, BOOL enable); //Tr181

/* wifi_getSSIDStatus() function */
/**
* @brief Get SSID enable status.
* Device.WiFi.SSID.{i}.Status
*
* @param[in]  ssidIndex     SSID index
* @param[out] output_string SSID enable status, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getSSIDStatus(INT ssidIndex, CHAR *output_string); //Tr181

/* wifi_getSSIDName() function */
/**
* @brief Get SSID Name associated with the Access Point index.
*
* Outputs a 32 byte or less string indicating the SSID name.
* Sring buffer must be preallocated by the caller.
* Device.WiFi.SSID.{i}.Name
* Device.WiFi.SSID.{i}.Alias
*
* @param[in]  apIndex         Access Point index
* @param[out] output_string   SSID name, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.SSID.{i}.Alias

//Device.WiFi.SSID.{i}.Name
INT wifi_getSSIDName(INT apIndex, CHAR *output_string);        

/**
* @brief To read the run time ssid name.
*
* @param[in]  apIndex         Access Point index
* @param[out] output_string   SSID name, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getSSIDNameStatus(INT apIndex, CHAR *output_string);


/* wifi_setSSIDName() function */
/**
* @brief Set SSID Name associated with the Access Point index.
*
* Device.WiFi.SSID.{i}.Name
* Device.WiFi.SSID.{i}.Alias
* Accepts a max 32 byte string and sets an internal variable to the SSID name.
*
* @param[in]  apIndex      Access Point index
* @param[out] ssid_string  SSID Name
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setSSIDName(INT apIndex, CHAR *ssid_string);
// push the ssid name to the hardware //repleaced by wifi_applySSIDSettings
//INT wifi_pushSSIDName(INT apIndex, CHAR *ssid);                         


/* wifi_getBaseBSSID() function */
/**
* @brief Get the BSSID.
*
* Device.WiFi.SSID.{i}.BSSID
*
* @param[in]  ssidIndex      SSID index
* @param[out] output_string  Base BSSID, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.SSID.{i}.LastChange

//Device.WiFi.SSID.{i}.LowerLayers

//Device.WiFi.SSID.{i}.BSSID
INT wifi_getBaseBSSID(INT ssidIndex, CHAR *output_string);	//RDKB

/* wifi_getSSIDMACAddress() function */
/**
* @brief Get the MAC address associated with this Wifi SSID.
*
* Device.WiFi.SSID.{i}.MACAddress
*
* @param[in]  ssidIndex      SSID index
* @param[out] output_string  MAC Address, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.SSID.{i}.MACAddress
INT wifi_getSSIDMACAddress(INT ssidIndex, CHAR *output_string); //Tr181

//Device.WiFi.SSID.{i}.SSID

//-----------------------------------------------------------------------------------------------
//Device.WiFi.SSID.{i}.Stats.
//Device.WiFi.SSID.{i}.Stats.BytesSent
//Device.WiFi.SSID.{i}.Stats.BytesReceived
//Device.WiFi.SSID.{i}.Stats.PacketsSent
//Device.WiFi.SSID.{i}.Stats.PacketsReceived

//Device.WiFi.SSID.{i}.Stats.RetransCount		
//Device.WiFi.SSID.{i}.Stats.FailedRetransCount	
//Device.WiFi.SSID.{i}.Stats.RetryCount	
//Device.WiFi.SSID.{i}.Stats.MultipleRetryCount	
//Device.WiFi.SSID.{i}.Stats.ACKFailureCount	
//Device.WiFi.SSID.{i}.Stats.AggregatedPacketCount	
	 
//Device.WiFi.SSID.{i}.Stats.ErrorsSent
//Device.WiFi.SSID.{i}.Stats.ErrorsReceived
//Device.WiFi.SSID.{i}.Stats.UnicastPacketsSent
//Device.WiFi.SSID.{i}.Stats.UnicastPacketsReceived
//Device.WiFi.SSID.{i}.Stats.DiscardPacketsSent
//Device.WiFi.SSID.{i}.Stats.DiscardPacketsReceived
//Device.WiFi.SSID.{i}.Stats.MulticastPacketsSent
//Device.WiFi.SSID.{i}.Stats.MulticastPacketsReceived
//Device.WiFi.SSID.{i}.Stats.BroadcastPacketsSent
//Device.WiFi.SSID.{i}.Stats.BroadcastPacketsReceived
//Device.WiFi.SSID.{i}.Stats.UnknownProtoPacketsReceived	

/* wifi_getSSIDTrafficStats2() function */
/**
* @brief Get the basic SSID traffic static info.
*
* @param[in]  ssidIndex      SSID index
* @param[out] output_struct  SSID traffic status.
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getSSIDTrafficStats2(INT ssidIndex, wifi_ssidTrafficStats2_t *output_struct); //Tr181

/* wifi_applySSIDSettings() function */
/**
* @brief Apply SSID and AP (in the case of Acess Point devices) to the hardware.
*
* Not all implementations may need this function.
* If not needed for a particular implementation simply return no-error (0).
*
* @param[in] ssidIndex  SSID index
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_applySSIDSettings(INT ssidIndex);


//-----------------------------------------------------------------------------------------------
//Device.WiFi.NeighboringWiFiDiagnostic.	
//Device.WiFi.NeighboringWiFiDiagnostic.DiagnosticsState
//Device.WiFi.NeighboringWiFiDiagnostic.ResultNumberOfEntries

//-----------------------------------------------------------------------------------------------
//Device.WiFi.NeighboringWiFiDiagnostic.Result.{i}.	
//Device.WiFi.NeighboringWiFiDiagnostic.Result.{i}.Radio
//Device.WiFi.NeighboringWiFiDiagnostic.Result.{i}.SSID
//Device.WiFi.NeighboringWiFiDiagnostic.Result.{i}.BSSID
//Device.WiFi.NeighboringWiFiDiagnostic.Result.{i}.Mode						
//Device.WiFi.NeighboringWiFiDiagnostic.Result.{i}.Channel
//Device.WiFi.NeighboringWiFiDiagnostic.Result.{i}.SignalStrength
//Device.WiFi.NeighboringWiFiDiagnostic.Result.{i}.SecurityModeEnabled
//Device.WiFi.NeighboringWiFiDiagnostic.Result.{i}.EncryptionMode	
//Device.WiFi.NeighboringWiFiDiagnostic.Result.{i}.OperatingFrequencyBand
//Device.WiFi.NeighboringWiFiDiagnostic.Result.{i}.SupportedStandards		
//Device.WiFi.NeighboringWiFiDiagnostic.Result.{i}.OperatingStandards
//Device.WiFi.NeighboringWiFiDiagnostic.Result.{i}.OperatingChannelBandwidth
//Device.WiFi.NeighboringWiFiDiagnostic.Result.{i}.BeaconPeriod	
//Device.WiFi.NeighboringWiFiDiagnostic.Result.{i}.Noise
//Device.WiFi.NeighboringWiFiDiagnostic.Result.{i}.BasicDataTransferRates
//Device.WiFi.NeighboringWiFiDiagnostic.Result.{i}.SupportedDataTransferRates
//Device.WiFi.NeighboringWiFiDiagnostic.Result.{i}.DTIMPeriod
//Device.WiFi.NeighboringWiFiDiagnostic.Result.{i}.X_COMCAST-COM_ChannelUtilization

/* wifi_getNeighboringWiFiDiagnosticResult2() function */
/**
* @brief Start the wifi scan and get the result into output buffer for RDKB to parser.
*
* The result will be used to manage endpoint list. HAL functon should allocate an data structure array,
* and return to caller with "neighbor_ap_array". 
*
* @param[in]  radioIndex           Radio index
* @param[out] neighbor_ap_array    Neighbour access point info to be returned
* @param[out] output_array_size    Length of the output array.
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getNeighboringWiFiDiagnosticResult2(INT radioIndex, wifi_neighbor_ap2_t **neighbor_ap_array, UINT *output_array_size); //Tr181	

/**
* @brief Returns the Wifi scan status.
*
* @param[in]  radioIndex           Radio index
* @param[out] neighbor_ap_array    Neighbour access point info to be returned
* @param[out] output_array_size    Length of the output array.
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getNeighboringWiFiStatus(INT radioIndex, wifi_neighbor_ap2_t **neighbor_ap_array, UINT *output_array_size); //Mesh

/** @} */  //END OF GROUP WIFI_HAL_APIS

/**
 * @addtogroup WIFI_HAL_TYPES
 * @{
 */

/**
 * @brief Represents the wifi scan modes.
 */
typedef enum
{
    WIFI_RADIO_SCAN_MODE_NONE = 0,
    WIFI_RADIO_SCAN_MODE_FULL,
    WIFI_RADIO_SCAN_MODE_ONCHAN,
    WIFI_RADIO_SCAN_MODE_OFFCHAN,
    WIFI_RADIO_SCAN_MODE_SURVEY
} wifi_neighborScanMode_t;
/** @} */  //END OF GROUP WIFI_HAL_TYPES 

/**
 * @addtogroup WIFI_HAL_APIS
 * @{
 */

/**
* @brief This API initates the scanning.
*
* @param[in]  apIndex       The index of access point array.
* @param[out] scan_mode     Scan modes.
* @param[out] dwell_time    Amount of time spent on each channel in the hopping sequence.
* @param[out] chan_num      The channel number.
* @param[out] chan_list     List of channels.
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_startNeighborScan(INT apIndex, wifi_neighborScanMode_t scan_mode, INT dwell_time, UINT chan_num, UINT *chan_list);

//>> Deprecated: used for old RDKB code. 
/** Deprecated: used for old RDKB code. */
INT wifi_getSSIDTrafficStats(INT ssidIndex, wifi_ssidTrafficStats_t *output_struct); //Tr181
/** Deprecated: used for old RDKB code. */
INT wifi_getBasicTrafficStats(INT apIndex, wifi_basicTrafficStats_t *output_struct);  //!< Outputs basic traffic stats per AP
/** Deprecated: used for old RDKB code. */
INT wifi_getWifiTrafficStats(INT apIndex, wifi_trafficStats_t *output_struct); //!< Outputs more detailed traffic stats per AP
/** Deprecated: used for old RDKB code. */
INT wifi_getNeighboringWiFiDiagnosticResult(wifi_neighbor_ap_t **neighbor_ap_array, UINT *output_array_size); //Tr181
/**  Deprecated: used for old RDKB code. */
INT wifi_getAllAssociatedDeviceDetail(INT apIndex, ULONG *output_ulong, wifi_device_t **output_struct); //RDKB
//<<

//>> -------------------- wifi_ap_hal -----------------------------------
//---------------------------------------------------------------------------------------------------
//
// Additional Wifi radio level APIs used for RDKB Access Point devices
//
//---------------------------------------------------------------------------------------------------

/* wifi_getBandSteeringCapability() function */
/**
* @brief To get Band Steering Capability.
*
* Device.WiFi.X_RDKCENTRAL-COM_BandSteering object
* Device.WiFi.X_RDKCENTRAL-COM_BandSteering.Capability bool r/o
*
* @param[out] support Band Steering Capability support, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.X_RDKCENTRAL-COM_BandSteering object
//Device.WiFi.X_RDKCENTRAL-COM_BandSteering.Capability bool r/o
INT wifi_getBandSteeringCapability(BOOL *support); 

/* wifi_getBandSteeringEnable() function */
/**
* @brief To get Band Steering enable status.
*
* Device.WiFi.X_RDKCENTRAL-COM_BandSteering.Enable bool r/w
*
* @param[out] enable  Band Steering enable status, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getBandSteeringEnable(BOOL *enable);

/* wifi_setBandSteeringEnable() function */
/**
* @brief To turn on/off Band steering.
*
* @param[in] enable  Band Steering enable status
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setBandSteeringEnable(BOOL enable);

/* wifi_getBandSteeringApGroup() function */
/**
* @brief To get Band Steering Access Point group.
*
* @param[out] output_ApGroup Band Steering Access point group
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getBandSteeringApGroup(char *output_ApGroup);	

/* wifi_setBandSteeringApGroup() function */
/**
* @brief To set Band Steering Access Point group. 
*
* @param[in] ApGroup - Band Steering Access Point  group
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//To set Band Steering AP group 
//ApGroup contains AP index(start from 1) pair array, in following format "$index_2.4G,$index_5G;$index_2.4G,$index_5G"
//Example "1,2;3,4;7,8" for Private, XH, LnF pairs. 
//ApGroup have to contain at least one AP pair, such as "1,2"
INT wifi_setBandSteeringApGroup(char *ApGroup); 

/* wifi_getRadioBandUtilization() function */
/**
* @brief To read the radio band utilization.
*
* @param[in]  radioIndex         Radio Index
* @param[out] output_percentage  Radio band utilization percentage, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getRadioBandUtilization (INT radioIndex, INT *output_percentage);

/* wifi_getApAssociatedDevice() function */
/**
* @brief Gets the ApAssociatedDevice list for client MAC addresses
*
* @param[in]  apIndex          Access Point index
* @param[out] output_buf       List for client MAC, to be returned
* @param[out] output_buf_size  Buffer length
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
*
*/
INT wifi_getApAssociatedDevice(INT ap_index, CHAR *output_buf, INT output_buf_size);

/* wifi_getApAssociatedDevice() function */
/**
* @brief Gets the RSSI value associated with the access point.
*
* @param[in]  apIndex      Access Point index
* @param[out] MAC          Client MAC in upcase format
* @param[out] output_RSSI  RSSI is in dbm
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
*
*/
INT wifi_getApDeviceRSSI(INT ap_index, CHAR *MAC, INT *output_RSSI);

/* wifi_getApAssociatedDevice() function */
/**
* @brief Gets the ApAssociatedDevice Rx Rate
*
* @param[in] apIndex       Access Point index
* @param[in] MAC           Client MAC in upcase format
* @param[ut] output_RxMb   Rx Rate in Mb
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
*
*/
INT wifi_getApDeviceRxrate (INT ap_index, CHAR *MAC, INT *output_RxMb);

/* wifi_getApAssociatedDevice() function */
/**
* @brief Gets the ApAssociatedDevice Tx Rate
*
* @param[in]  apIndex       Access Point index
* @param[in]  MAC           Client MAC in upcase format
* @param[out] output_TxMb   Tx Rate in Mb
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
*
*/
INT wifi_getApDeviceTxrate (INT ap_index, CHAR *MAC, INT *output_TxMb);

/* wifi_getBandSteeringBandUtilizationThreshold() function */
/**
* @brief To set and read the band steering BandUtilizationThreshold parameters.
*
* Device.WiFi.X_RDKCENTRAL-COM_BandSteering.BandSetting.{i}.UtilizationThreshold int r/w
*
* @param[in]  radioIndex     Radio Index
* @param[out] pBuThreshold   Steering bane utilization threshold, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getBandSteeringBandUtilizationThreshold (INT radioIndex, INT *pBuThreshold);

/* wifi_setBandSteeringBandUtilizationThreshold() function */
/**
* @brief To set the band steering BandUtilizationThreshold parameters.
*
* Device.WiFi.X_RDKCENTRAL-COM_BandSteering.BandSetting.{i}.UtilizationThreshold int r/w
*
* @param[in] radioIndex    Radio Index
* @param[in] buThreshold   Steering bane utilization threshold
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setBandSteeringBandUtilizationThreshold (INT radioIndex, INT buThreshold);

/* wifi_getBandSteeringRSSIThreshold() function */
/**
* @brief To read the band steering RSSIThreshold parameters.
*
* Device.WiFi.X_RDKCENTRAL-COM_BandSteering.BandSetting.{i}.RSSIThreshold int r/w
*
* @param[in] radioIndex       Radio Index
* @param[in] pRssiThreshold   Band steering RSSIThreshold value, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getBandSteeringRSSIThreshold (INT radioIndex, INT *pRssiThreshold);

/* wifi_setBandSteeringRSSIThreshold() function */
/**
* @brief To set the band steering RSSIThreshold parameters.
*
* For 2.4G, the expectation is if the 2G rssi is below the set value steer to 2G
* For 5G, if the set value is greater than the set threshold value then steer to 5
* Device.WiFi.X_RDKCENTRAL-COM_BandSteering.BandSetting.{i}.RSSIThreshold int r/w
*
* @param[in] radioIndex     Radio Index
* @param[in] rssiThreshold  Band steering RSSIThreshold value
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setBandSteeringRSSIThreshold (INT radioIndex, INT rssiThreshold);

/* wifi_getBandSteeringPhyRateThreshold() function */
/**
* @brief To read the band steering physical modulation rate threshold parameters.
*
* Device.WiFi.X_RDKCENTRAL-COM_BandSteering.BandSetting.{i}.PhyRateThreshold int r/w
*
* @param[in]  radioIndex    Radio Index
* @param[out] pPrThreshold  Physical modulation rate threshold value, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getBandSteeringPhyRateThreshold (INT radioIndex, INT *pPrThreshold); //If chip is not support, return -1

/* wifi_setBandSteeringPhyRateThreshold() function */
/**
* @brief To set the band steering physical modulation rate threshold parameters.
*
* Device.WiFi.X_RDKCENTRAL-COM_BandSteering.BandSetting.{i}.PhyRateThreshold int r/w
*
* @param[in] radioIndex   Radio Index
* @param[in] prThreshold  Physical modulation rate threshold value
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setBandSteeringPhyRateThreshold (INT radioIndex, INT prThreshold); //If chip is not support, return -1

/* wifi_getBandSteeringOverloadInactiveTime() function */
/**
* @brief To read the inactivity time (in seconds) for steering under overload condition 
*
* Device.WiFi.X_RDKCENTRAL-COM_BandSteering.BandSetting.{i}.OverloadInactiveTime int r/w 
*
* @param[in]  radioIndex             Radio Index
* @param[out] overloadInactiveTime   inactivity time (in seconds) for steering under overload condition
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getBandSteeringOverloadInactiveTime (INT radioIndex, INT *overloadInactiveTime); //If chip is not support, return -1

/* wifi_setBandSteeringOverloadInactiveTime() function */
/**
* @brief To set the inactivity time (in seconds) for steering under overload condition.
*
* Device.WiFi.X_RDKCENTRAL-COM_BandSteering.BandSetting.{i}.OverloadInactiveTime int r/w 
*
* @param[in] radioIndex             Radio Index
* @param[in] overloadInactiveTime   Inactivity time (in seconds) for steering under overload condition
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setBandSteeringOverloadInactiveTime (INT radioIndex, INT overloadInactiveTime); //If chip is not support, return -1

/* wifi_getBandSteeringIdleInactiveTime() function */
/**
* @brief To read the inactivity time (in seconds) for steering under Idle condition.
* 
* Device.WiFi.X_RDKCENTRAL-COM_BandSteering.BandSetting.{i}.IdleInactiveTime int r/w
*
* @param[in]  radioIndex         Radio Index
* @param[out] idleInactiveTime   Inactivity time (in seconds) for steering under Idle condition
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getBandSteeringIdleInactiveTime (INT radioIndex, INT *idleInactiveTime); //If chip is not support, return -1

/* wifi_getBandSteeringIdleInactiveTime() function */
/**
* @brief To set the inactivity time (in seconds) for steering under Idle condition.
*
* Device.WiFi.X_RDKCENTRAL-COM_BandSteering.BandSetting.{i}.IdleInactiveTime int r/w
*
* @param[in]  radioIndex         Radio Index
* @param[in]  idleInactiveTime   Inactivity time (in seconds) for steering under Idle condition
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setBandSteeringIdleInactiveTime (INT radioIndex, INT idleInactiveTime); //If chip is not support, return -1


/* wifi_getBandSteeringLog() function */
/**
* @brief To get the band steering log.
*
* If no steering or record_index is out of boundary, return -1.
* Device.WiFi.X_RDKCENTRAL-COM_BandSteering.History string r/o
*
* @param[in]  record_index       Record index
* @param[out] pSteeringTime      Returns the UTC time in seconds
* @param[in]  pClientMAC         pClientMAC is pre allocated as 64bytes
* @param[in]  pSourceSSIDIndex   Source SSID index
* @param[in]  pDestSSIDIndex     Destination SSID index
* @param[out] pSteeringReason    Returns the predefined steering trigger reason
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.X_RDKCENTRAL-COM_BandSteering.History string r/o
INT wifi_getBandSteeringLog(INT record_index, ULONG *pSteeringTime, CHAR *pClientMAC, INT *pSourceSSIDIndex, INT *pDestSSIDIndex, INT *pSteeringReason); 
	
/* wifi_factoryResetAP() function */
/**
* @brief Restore Access point paramters to default without change other AP nor Radio parameters (No need to reboot wifi)
*
* @param[in] apIndex  Access Point index
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_factoryResetAP(int apIndex); 	

/* wifi_setRadioCtsProtectionEnable() function */
/**
* @brief  Enables CTS protection for the radio used by this Access Point.
*
* @param[in] apIndex  Access Point index
* @param[in] enable   CTS protection enable value
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setRadioCtsProtectionEnable(INT apIndex, BOOL enable);          //P3 

/* wifi_setRadioObssCoexistenceEnable() function */
/**
* @brief Enables OBSS Coexistence - fall back to 20MHz if necessary for the radio used by this AP.
*
* @param[in] apIndex   Access Point index
* @param[in] enable    OBSS Coexistence enable value
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setRadioObssCoexistenceEnable(INT apIndex, BOOL enable);      

/* wifi_setRadioFragmentationThreshold() function */
/**
* @brief Sets the fragmentation threshold in bytes for the radio used by this Access Point.
*
* @param[in] apIndex    Access Point index
* @param[in] threshold  Fragmentation Threshold value
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setRadioFragmentationThreshold(INT apIndex, UINT threshold);    //P3 

/* wifi_setRadioSTBCEnable() function */
/**
* @brief Enable STBC mode in the hardware.
* 0 == not enabled, 1 == enabled.
*
* @param[in]  radioIndex   Radio index
* @param[in]  STBC_Enable  STBC mode enable value
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setRadioSTBCEnable(INT radioIndex, BOOL STBC_Enable);

/* wifi_getRadioAMSDUEnable() function */
/**
* @brief Outputs A-MSDU enable status, 0 == not enabled, 1 == enabled.
*
* @param[in]  radioIndex   Radio index
* @param[out] output_bool  A-MSDU enable status, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getRadioAMSDUEnable(INT radioIndex, BOOL *output_bool);

/* wifi_setRadioAMSDUEnable() function */
/**
* @brief Enables A-MSDU in the hardware, 0 == not enabled, 1 == enabled.
*
* @param[in]  radioIndex    Radio index
* @param[out] amsduEnable   A-MSDU enable status value
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setRadioAMSDUEnable(INT radioIndex, BOOL amsduEnable);

/* wifi_getRadioTxChainMask() function */
/**
* @brief Outputs the number of Tx streams.
*
* @param[in]  radioIndex  Radio index
* @param[out] output_int  Number of Tx streams, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getRadioTxChainMask(INT radioIndex, INT *output_int);           //P2

/* wifi_setRadioTxChainMask() function */
/**
* @brief Sets the number of Tx streams to an environment variable.
*
* @param[in] radioIndex   Radio index
* @param[in] numStreams   Number of Tx streams
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setRadioTxChainMask(INT radioIndex, INT numStreams);            //P2

/* wifi_getRadioRxChainMask() function */
/**
* @brief Outputs the number of Rx streams.
*
* @param[in]  radioIndex  Radio index
* @param[out] output_int  Number of Rx streams, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getRadioRxChainMask(INT radioIndex, INT *output_int);           //P2  // outputs the number of Rx streams

/* wifi_setRadioRxChainMask() function */
/**
* @brief Sets the number of Rx streams to an environment variable.
*
* @param[in] radioIndex   Radio index
* @param[in]  numStreams  Number of Rx streams
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setRadioRxChainMask(INT radioIndex, INT numStreams);            //P2  // sets the number of Rx streams to an environment variable

//>> Deprecated: 
/**<Deprecated */
INT wifi_pushBridgeInfo(INT apIndex); 									 //P2  //!< Push the BridgeInfo environment variables to the hardware
/**< Deprecated */
//INT wifi_pushRadioChannel(INT radioIndex, UINT channel);                 //P2  //!<  push the channel number setting to the hardware  //Applying changes with wifi_applyRadioSettings().
/**< Deprecated */
INT wifi_pushRadioChannelMode(INT radioIndex);                           //P2  //!< push the channel mode environment variable that is set by "wifi_setChannelMode()" to the hardware  //Applying changes with wifi_applyRadioSettings().
/**< Deprecated */
INT wifi_pushRadioTxChainMask(INT radioIndex);                           //P2  //!<  push the environment varible that is set by "wifi_setTxChainMask()" to the hardware //Applying changes with wifi_applyRadioSettings().
/**< Deprecated */
INT wifi_pushRadioRxChainMask(INT radioIndex);                           //P2  //!< push the environment varible that is set by "wifi_setRxChainMask()" to the hardware //Applying changes with wifi_applyRadioSettings().
//<<

/* wifi_pushSSID() function */
/**
* @brief Push the environment variable that is set by "wifi_setSsidName" to the hardware.
*
* @param[in] apIndex  Access Point index
* @param[in] ssid     WiFi SSID value
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_pushSSID(INT apIndex, CHAR *ssid); 							 // push the environment varible that is set by "wifi_setSsidName" to the hardware    

/* wifi_pushSsidAdvertisementEnable() function */
/**
* @brief Push the environment variable that is set by "wifi_setApSsidAdvertisementEnable" to the hardware.
*
* @param[in] apIndex  Access Point index
* @param[in] enable   SSID Advertisement value
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_pushSsidAdvertisementEnable(INT apIndex, BOOL enable);			 // push the environment varible that is set by "wifi_setApSsidAdvertisementEnable" to the hardware	

/* wifi_getRadioUpTime() function */
/**
* @brief Get the number of seconds elapsed since radio is started.
*
* @param[in] radioIndex  Radio index
* @param[in] uptime      Wifi uptime, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getRadioUpTime(INT radioIndex, ULONG *uptime);					 // get the number of seconds elapsed since radio is started	 


/* wifi_getRadioReverseDirectionGrantSupported() function */
/**
* @brief Get radio RDG enable Support.
*
* @param[in]  radioIndex   Radio index
* @param[out] output_bool  RDG enable support value, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getRadioReverseDirectionGrantSupported(INT radioIndex, BOOL *output_bool);    //Get radio RDG enable Support

/* wifi_getRadioReverseDirectionGrantEnable() function */
/**
* @brief Get radio RDG enable setting.
*
* @param[in]  radioIndex   Radio index
* @param[out] output_bool  RDG enable setting value, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getRadioReverseDirectionGrantEnable(INT radioIndex, BOOL *output_bool);    //Get radio RDG enable setting

/* wifi_setRadioReverseDirectionGrantEnable() function */
/**
* @brief Set radio RDG enable setting.
*
* @param[in] radioIndex  Radio index
* @param[in] enable      RDG enable setting value
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setRadioReverseDirectionGrantEnable(INT radioIndex, BOOL enable);			//Set radio RDG enable setting

/* wifi_getRadioDeclineBARequestEnable() function */
/**
* @brief Get radio ADDBA (ADD Block Acknowledgement) enable setting.
*
* @param[in]  radioIndex   Radio index
* @param[out] output_bool  Radio ADDBA enable setting value, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getRadioDeclineBARequestEnable(INT radioIndex, BOOL *output_bool);			//Get radio ADDBA enable setting

/* wifi_setRadioDeclineBARequestEnable() function */
/**
* @brief Set radio ADDBA (ADD Block Acknowledgement) enable setting.
*
* @param[in] radioIndex  Radio index
* @param[in] enable      Radio ADDBA enable setting value
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setRadioDeclineBARequestEnable(INT radioIndex, BOOL enable);				//Set radio ADDBA enable setting

/* wifi_getRadioAutoBlockAckEnable() function */
/**
* @brief Get radio auto block ack enable setting.
*
* @param[in]  radioIndex   Radio index
* @param[out] output_bool  Auto block ack enable setting value, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getRadioAutoBlockAckEnable(INT radioIndex, BOOL *output_bool);				//Get radio auto block ack enable setting

/* wifi_setRadioAutoBlockAckEnable() function */
/**
* @brief Set radio auto block ack enable setting.
*
* @param[in] radioIndex  Radio index
* @param[in] enable      Auto block ack enable setting value
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setRadioAutoBlockAckEnable(INT radioIndex, BOOL enable);					//Set radio auto block ack enable setting

/* wifi_getRadio11nGreenfieldSupported() function */
/**
* @brief Get radio 11n pure mode enable support.
*
* @param[in]  radioIndex   Radio index
* @param[out] output_bool  Radio 11n pure mode enable support value, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getRadio11nGreenfieldSupported(INT radioIndex, BOOL *output_bool);			//Get radio 11n pure mode enable Support

/* wifi_getRadio11nGreenfieldEnable() function */
/**
* @brief Get radio 11n pure mode enable setting.
*
* @param[in]  radioIndex   Radio index
* @param[out] output_bool  Radio 11n pure mode enable setting, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getRadio11nGreenfieldEnable(INT radioIndex, BOOL *output_bool);			//Get radio 11n pure mode enable setting

/* wifi_setRadio11nGreenfieldEnable() function */
/**
* @brief Set radio 11n pure mode enable setting.
*
* @param[in] radioIndex  Radio index
* @param[in] enable      Radio 11n pure mode enable setting
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setRadio11nGreenfieldEnable(INT radioIndex, BOOL enable);					//Set radio 11n pure mode enable setting

/* wifi_getRadioIGMPSnoopingEnable() function */
/**
* @brief Get radio IGMP snooping enable setting.
*
* @param[in]  radioIndex   Radio index
* @param[out] output_bool  Radio IGMP snooping enable setting, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getRadioIGMPSnoopingEnable(INT radioIndex, BOOL *output_bool);				//Get radio IGMP snooping enable setting

/* wifi_setRadioIGMPSnoopingEnable() function */
/**
* @brief Set radio IGMP snooping enable setting.
*
* @param[in]  radioIndex  Radio index
* @param[out] enable      Radio IGMP snooping enable setting
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setRadioIGMPSnoopingEnable(INT radioIndex, BOOL enable);					//Set radio IGMP snooping enable setting
//---------------------------------------------------------------------------------------------------
//
// Additional Wifi AP level APIs used for Access Point devices
//
//---------------------------------------------------------------------------------------------------


//AP HAL

/* wifi_createAp() function */
/**
* @brief Creates a new access point and pushes these parameters to the hardware.
*
* @param[in] apIndex     Access Point index
* @param[in] radioIndex  Radio index
* @param[in] essid       SSID Name
* @param[in] hideSsid    True/False, to SSID advertisement enable value
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_createAp(INT apIndex, INT radioIndex, CHAR *essid, BOOL hideSsid);  // creates a new ap and pushes these parameters to the hardware

/* wifi_deleteAp() function */
/**
* @brief Deletes this access point entry on the hardware, clears all internal variables associated with this access point.
*
* @param[in] apIndex  Access Point index
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_deleteAp(INT apIndex);                                     // deletes this ap entry on the hardware, clears all internal variables associaated with this ap

/* wifi_getApName() function */
/**
* @brief Outputs a 16 byte or less name associated with the Access Point.
* String buffer must be pre-allocated by the caller.
*
* @param[in]  apIndex         Access Point index
* @param[out] output_string   Access Point name, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getApName(INT apIndex, CHAR *output_string);                 // Outputs a 16 byte or less name assocated with the AP.  String buffer must be pre-allocated by the caller

/* wifi_getApIndexFromName() function */
/**
* @brief Outputs the index number in that corresponds to the SSID string.
*
* @param[in]  inputSsidString  WiFi SSID Name
* @param[out] ouput_int        Access Point index, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getApIndexFromName(CHAR *inputSsidString, INT *ouput_int);	 // Outputs the index number in that corresponds to the SSID string

/* wifi_getApBeaconType() function */
/**
* @brief Outputs a 32 byte or less string indicating the beacon type as "None", "Basic", "WPA", "11i", "WPAand11i".
*
* @param[in]  apIndex        Access Point index
* @param[out] output_string  Beacon type, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getApBeaconType(INT apIndex, CHAR *output_string);           // Outputs a 32 byte or less string indicating the beacon type as "None", "Basic", "WPA", "11i", "WPAand11i"

/* wifi_setApBeaconType() function */
/**
* @brief Sets the beacon type environment variable. Allowed input strings are "None", "Basic", "WPA, "11i", "WPAand11i".
*
* @param[in] apIndex            Access Point index
* @param[in] beaconTypeString   Beacon type 
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setApBeaconType(INT apIndex, CHAR *beaconTypeString);        // Sets the beacon type environment variable. Allowed input strings are "None", "Basic", "WPA, "11i", "WPAand11i"

/* wifi_setApBeaconInterval() function */
/**
* @brief Sets the beacon interval on the hardware for this Access Point.
*
* @param[in] apIndex          Access Point index
* @param[in] beaconInterval   Beacon interval 
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setApBeaconInterval(INT apIndex, INT beaconInterval);        // sets the beacon interval on the hardware for this AP

/* wifi_setApDTIMInterval() function */
/**
* @brief Sets the DTIM interval for this Access Point.
*
* DTIM (Delivery Traffic Indication Message)
*
* @param[in] apIndex        Access Point index
* @param[in] dtimInterval   DTIM interval 
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setApDTIMInterval(INT apIndex, INT dtimInterval);			  // Sets the DTIM interval for this AP	

/* wifi_getApRtsThresholdSupported() function */
/**
* @brief Get the packet size threshold supported.
*
* @param[in]  apIndex       Access Point index
* @param[out] output_bool   Packet size threshold supported, to be returned 
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getApRtsThresholdSupported(INT apIndex, BOOL *output_bool);  // Get the packet size threshold supported. 

/* wifi_setApRtsThreshold() function */
/**
* @brief Sets the packet size threshold in bytes to apply RTS/CTS backoff rules.
*
* @param[in] apIndex    Access Point index
* @param[in] threshold  Packet size threshold 
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setApRtsThreshold(INT apIndex, UINT threshold);              // sets the packet size threshold in bytes to apply RTS/CTS backoff rules. 

/* wifi_getApWpaEncryptionMode() function */
/**
* @brief Ouputs up to a 32 byte string as either "TKIPEncryption", "AESEncryption", or "TKIPandAESEncryption".
*
* @param[in]  apIndex        Access Point index
* @param[out] output_string  WPA Encryption mode, to be returned 
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getApWpaEncryptionMode(INT apIndex, CHAR *output_string);    // ouputs up to a 32 byte string as either "TKIPEncryption", "AESEncryption", or "TKIPandAESEncryption"

/* wifi_setApWpaEncryptionMode() function */
/**
* @brief Sets the encyption mode environment variable.
*
* Valid string format are:
*
* - TKIPEncryption
* - AESEncryption
* - TKIPandAESEncryption
*
* @param[in] apIndex  Access Point index
* @param[in] encMode  WPA Encryption mode
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setApWpaEncryptionMode(INT apIndex, CHAR *encMode);          // sets the encyption mode environment variable.  Valid string format is "TKIPEncryption", "AESEncryption", or "TKIPandAESEncryption"

/* wifi_removeApSecVaribles() function */
/**
* @brief Deletes internal security variable settings for this access point.
*
* @param[in] apIndex  Access Point index
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_removeApSecVaribles(INT apIndex);                            // deletes internal security varable settings for this ap

/* wifi_disableApEncryption() function */
/**
* @brief Changes the hardware settings to disable encryption on this access point.
*
* @param[in] apIndex  Access Point index
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_disableApEncryption(INT apIndex);                            // changes the hardware settings to disable encryption on this ap

/* wifi_setApAuthMode() function */
/**
* @brief Set the authorization mode on this access point.
*
* Mode mapping as:
* -1 open
* -2 shared
* -4 auto
*
* @param[in] apIndex Access Point index
* @param[in] mode    Authorization mode
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setApAuthMode(INT apIndex, INT mode);                        // set the authorization mode on this ap. mode mapping as: 1: open, 2: shared, 4:auto

/* wifi_setApBasicAuthenticationMode() function */
/**
* @brief Sets an environment variable for the authMode.
*
* Valid strings are "None", "EAPAuthentication" or "SharedAuthentication".
*
* @param[in] apIndex   Access Point index
* @param[in] authMode  Authentication mode
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setApBasicAuthenticationMode(INT apIndex, CHAR *authMode);   // sets an environment variable for the authMode. Valid strings are "None", "EAPAuthentication" or "SharedAuthentication"

/* wifi_getApNumDevicesAssociated() function */
/**
* @brief Outputs the number of stations associated per Access Point.
*
* @param[in]  apIndex       Access Point index
* @param[out] output_ulong  Number of stations, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getApNumDevicesAssociated(INT apIndex, ULONG *output_ulong); // Outputs the number of stations associated per AP

/* wifi_kickApAssociatedDevice() function */
/**
* @brief Manually removes any active wi-fi association with the device specified on this access point.
*
* @param[in] apIndex     Access Point index
* @param[in] client_mac  Client device MAC address
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_kickApAssociatedDevice(INT apIndex, CHAR *client_mac);  	// manually removes any active wi-fi association with the device specified on this ap

/* wifi_getApRadioIndex() function */
/**
* @brief Outputs the radio index for the specified access point.
*
* @param[in]  apIndex     Access Point index
* @param[out] output_int  Radio index, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getApRadioIndex(INT apIndex, INT *output_int);                // outputs the radio index for the specified ap

/* wifi_setApRadioIndex() function */
/**
* @brief Sets the radio index for the specific access point.
*
* @param[in] apIndex     Access Point index
* @param[in] radioIndex  Radio index
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setApRadioIndex(INT apIndex, INT radioIndex);                // sets the radio index for the specific ap

/* wifi_getApAclDevices() function */
/**
* @brief Get the ACL MAC list per Access Point.
*
* @param[in]  apIndex   Access Point index
* @param[out] macArray  Mac Array list, to be returned // in formate as "11:22:33:44:55:66\n11:22:33:44:55:67\n"
* @param[out] buf_size  Buffer size for the mac array list
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getApAclDevices(INT apIndex, CHAR *macArray, UINT buf_size);	// Get the ACL MAC list per AP

/* wifi_addApAclDevice() function */
/**
* @brief Adds the mac address to the filter list.
*
* @param[in] apIndex            Access Point index
* @param[in] DeviceMacAddress   Mac Address of a device
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_addApAclDevice(INT apIndex, CHAR *DeviceMacAddress);         // adds the mac address to the filter list

/* wifi_delApAclDevice() function */
/**
* @brief Deletes the Device MAC address from the Access control filter list.
*
* @param[in]  apIndex           Access Point index
* @param[in]  DeviceMacAddress  Mac Address of a device
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_delApAclDevice(INT apIndex, CHAR *DeviceMacAddress);


/**
* @brief Deletes all Device MAC address from the Access control filter list.
*
* @param[in]  apIndex           Access Point index
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
*/
INT wifi_delApAclDevices(INT apINdex);


/* wifi_getApAclDeviceNum() function */
/**
* @brief Outputs the number of devices in the filter list.
*
* @param[in]  apIndex      Access Point index
* @param[out] output_uint  Number of devices, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getApAclDeviceNum(INT apIndex, UINT *output_uint);           // outputs the number of devices in the filter list

/* wifi_kickApAclAssociatedDevices() function */
/**
* @brief Enable kick for devices on acl black list.
*
* @param[in] apIndex  Access Point index
* @param[in] enable   Enable/disable kick for devices on acl black list
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_kickApAclAssociatedDevices(INT apIndex,BOOL enable);         // enable kick for devices on acl black list

/**
* @brief This function is to enable or disable
grey list Access Control on all applicable VAP
*
* @param[in]  enable             enable access control if true, disable if false
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not
invoke any blocking system
* calls.
*
*/
INT wifi_enableGreylistAccessControl(BOOL enable);
/* wifi_setApMacAddressControlMode() function */
/**
* @brief Sets the mac address filter control mode.
*
* - 0 : filter as  disabled
* - 1 : filter as whitelist
* - 2 : filter as blacklist.
*
* @param[in] apIndex      Access Point index
* @param[in] filterMode   Mac Address filter control mode
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setApMacAddressControlMode(INT apIndex, INT filterMode);     // sets the mac address filter control mode.  0 == filter disabled, 1 == filter as whitelist, 2 == filter as blacklist

/**
* @brief This function is to read the ACL mode.
*
* @param[in]  apIndex             Access Point index
* @param[out] output_filterMode   Mac Address control mode
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getApMacAddressControlMode(INT apIndex, INT *output_filterMode);


/* wifi_setApVlanEnable() function */
/**
* @brief Enables internal gateway VLAN mode.
*
* In this mode a Vlan tag is added to upstream (received) data packets before exiting the Wifi driver.
* VLAN tags in downstream data are stripped from data packets before transmission.  Default is FALSE.
*
* @param[in] apIndex      Access Point index
* @param[in] VlanEnabled  Internal gateway VLAN mode
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setApVlanEnable(INT apIndex, BOOL VlanEnabled);              // enables internal gateway VLAN mode.  In this mode a Vlan tag is added to upstream (received) data packets before exiting the Wifi driver.  VLAN tags in downstream data are stripped from data packets before transmission.  Default is FALSE. 

/* wifi_setApVlanID() function */
/**
* @brief Sets the vlan ID for this access point to an internal environment variable.
*
* @param[in] apIndex Access Point index
* @param[in] vlanId  VLAN ID
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setApVlanID(INT apIndex, INT vlanId);                        // sets the vlan ID for this ap to an internal environment variable

/* wifi_getApBridgeInfo() function */
/**
* @brief Gets bridgeName, IP address and Subnet.
*
* BridgeName is a maximum of 32 characters.
*
* @param[in]  index       Access Point index
* @param[out] bridgeName  Bridge name, to be returned
* @param[out] IP          IP Address, to be returned
* @param[out] subnet      Subnet, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getApBridgeInfo(INT index, CHAR *bridgeName, CHAR *IP, CHAR *subnet);	// gets bridgeName, IP address and Subnet.

/* wifi_setApBridgeInfo() function */
/**
* @brief Sets bridgeName, IP address and Subnet to internal environment variables.
*
* BridgeName is a maximum of 32 characters.
*
* @param[in] apIndex     Access Point index
* @param[in] bridgeName  Bridge name
* @param[in] IP          IP Address
* @param[in] subnet      Subnet
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setApBridgeInfo(INT apIndex, CHAR *bridgeName, CHAR *IP, CHAR *subnet);   //sets bridgeName, IP address and Subnet to internal environment variables. bridgeName is a maximum of 32 characters, 
//INT wifi_pushApBridgeInfo(INT apIndex);                               // push the BridgeInfo environment variables to the hardware //Applying changes with wifi_applyRadioSettings()

/* wifi_resetApVlanCfg() function */
/**
* @brief Reset the vlan configuration for this access point.
*
* @param[in] apIndex  Access Point index
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_resetApVlanCfg(INT apIndex);                                 // reset the vlan configuration for this ap
//INT wifi_setApBridging(INT apIndex, BOOL bridgeEnable);             // set the environment variables to control briding.  If isolation is requried then disable bridging.  //use wifi_setApIsolationEnable instead
//INT wifi_getApRouterEnable(INT apIndex, BOOL *output_bool);           //P4 // Outputs a bool that indicates if router is enabled for this ap
//INT wifi_setApRouterEnable(INT apIndex, BOOL routerEnabled);          //P4 // sets the routerEnabled variable for this ap

/* wifi_createHostApdConfig() function */
/**
* @brief Creates configuration variables needed for WPA/WPS.
*
* These variables are implementation dependent and in some implementations these variables are used by hostapd when it is started.
* Specific variables that are needed are dependent on the hostapd implementation.
* These variables are set by WPA/WPS security functions in this wifi HAL.
* If not needed for a particular implementation this function may simply return no error.
*
* @param[in] apIndex       Access Point index
* @param[in] createWpsCfg  Enable/Disable WPS Configuration creation
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_createHostApdConfig(INT apIndex, BOOL createWpsCfg);       // creates configuration variables needed for WPA/WPS.  These variables are implementation dependent and in some implementations these variables are used by hostapd when it is started.  Specific variables that are needed are dependent on the hostapd implementation. These variables are set by WPA/WPS security functions in this wifi HAL.  If not needed for a particular implementation this function may simply return no error.

/* wifi_startHostApd() function */
/**
* @brief Starts hostapd.
*
* Uses the variables in the hostapd config with format compatible with the specific hostapd implementation.
*
* @param None
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_startHostApd();                                            // starts hostapd, uses the variables in the hostapd config with format compatible with the specific hostapd implementation

/* wifi_stopHostApd() function */
/**
* @brief Stops hostapd
*
* @param None
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_stopHostApd();                                             // stops hostapd

//-----------------------------------------------------------------------------------------------

/* wifi_setApEnable() function */
/**
* @brief Sets the Access Point enable status variable for the specified access point.
*
* @param[in] apIndex  Access Point index
* @param[in] enable   Enable/Disable AP enable status variable
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.AccessPoint.{i}.	
//Device.WiFi.AccessPoint.{i}.Enable
INT wifi_setApEnable(INT apIndex, BOOL enable);                       // sets the AP enable status variable for the specified ap.

/**
* @brief Dynamically enable/disable access point.
*
* @param[in] apIndex  Access Point index
* @param[in] enable   Enable/Disable AP enable status variable
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_pushApEnable(INT apIndex, BOOL Enable);


/* wifi_getApEnable() function */
/**
* @brief Outputs the setting of the internal variable that is set by wifi_setEnable().
*
* @param[in]  apIndex      Access Point index
* @param[out] output_bool  AP enable status, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getApEnable(INT apIndex, BOOL *output_bool);                 // Outputs the setting of the internal variable that is set by wifi_setEnable().  


/* wifi_getApStatus() function */
/**
* @brief Outputs the AP "Enabled" "Disabled" status from driver.
*
* Device.WiFi.AccessPoint.{i}.Status
*
* @param[in]  apIndex        Access Point index
* @param[out] output_string  AP status, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.AccessPoint.{i}.Status
INT wifi_getApStatus(INT apIndex, CHAR *output_string);  				// Outputs the AP "Enabled" "Disabled" status from driver 

/* wifi_getApSsidAdvertisementEnable() function */
/**
* @brief Indicates whether or not beacons include the SSID name.
*
* Outputs 1 if SSID on the AP is enabled, else ouputs 0.
* Device.WiFi.AccessPoint.{i}.SSIDAdvertisementEnabled
*
* @param[in]  apIndex      Access Point index
* @param[out] output_bool  SSID Advertisement enabled, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.AccessPoint.{i}.SSIDAdvertisementEnabled		
//Indicates whether or not beacons include the SSID name.
INT wifi_getApSsidAdvertisementEnable(INT apIndex, BOOL *output_bool);// outputs a 1 if SSID on the AP is enabled, else ouputs 0

/* wifi_setApSsidAdvertisementEnable() function */
/**
* @brief Sets an internal variable for ssid advertisement.
*
* Set to 1 to enable, set to 0 to disable.
* Device.WiFi.AccessPoint.{i}.SSIDAdvertisementEnabled
*
* @param[in] apIndex  Access Point index
* @param[in] enable   SSID Advertisement enable value
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setApSsidAdvertisementEnable(INT apIndex, BOOL enable);      // sets an internal variable for ssid advertisement.  Set to 1 to enable, set to 0 to disable

/* wifi_pushApSsidAdvertisementEnable() function */
/**
* @brief Push the ssid advertisement enable variable to the hardware.
*
* Applying changs with wifi_applyRadioSettings().
*
* @param[in] apIndex  Access Point index
* @param[in] enable   SSID Advertisement enable value
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_pushApSsidAdvertisementEnable(INT apIndex, BOOL enable);     // push the ssid advertisement enable variable to the hardware //Applying changs with wifi_applyRadioSettings()

/* wifi_getApRetryLimit() function */
/**
* @brief Get the maximum number of retransmission for a packet.
*
* This corresponds to IEEE 802.11 parameter dot11ShortRetryLimit.
* Device.WiFi.AccessPoint.{i}.RetryLimit
*
* @param[in]  apIndex  Access Point index
* @param[out] output   Maximum number of retransmission for a packet, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.AccessPoint.{i}.RetryLimit		
//The maximum number of retransmission for a packet. This corresponds to IEEE 802.11 parameter dot11ShortRetryLimit.
INT wifi_getApRetryLimit(INT apIndex, UINT *output); 

/* wifi_setApRetryLimit() function */
/**
* @brief Set the maximum number of retransmission for a packet.
*
* This corresponds to IEEE 802.11 parameter dot11ShortRetryLimit.
* Device.WiFi.AccessPoint.{i}.RetryLimit
*
* @param[in] apIndex  Access Point index
* @param[in] number   Maximum number of retransmission for a packet
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setApRetryLimit(INT apIndex, UINT number); 

/* wifi_getApWMMCapability() function */
/**
* @brief Indicates whether this access point supports WiFi Multimedia (WMM) Access Categories (AC).
* Device.WiFi.AccessPoint.{i}.WMMCapability
*
* @param[in]  apIndex  Access Point index
* @param[out] output   WMM capability, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getApWMMCapability(INT apIndex, BOOL *output); 

/* wifi_getApUAPSDCapability() function */
/**
* @brief Indicates whether this access point supports WMM Unscheduled Automatic Power Save Delivery (U-APSD).
*
* U-APSD support implies WMM support. Device.WiFi.AccessPoint.{i}.UAPSDCapability
*
* @param[in]  apIndex  Access Point index
* @param[out] output   U-APSD capability, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.AccessPoint.{i}.UAPSDCapability		
//Indicates whether this access point supports WMM Unscheduled Automatic Power Save Delivery (U-APSD). Note: U-APSD support implies WMM support.
INT wifi_getApUAPSDCapability(INT apIndex, BOOL *output); 			
			
/* wifi_getApWmmEnable() function */
/**
* @brief Indicates whether WMM support is currently enabled.
*
* When enabled, this is indicated in beacon frames.
* Device.WiFi.AccessPoint.{i}.WMMEnable
*
* @param[in]  apIndex  Access Point index
* @param[out] output   WMM support enabled status, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.AccessPoint.{i}.WMMEnable		
//Whether WMM support is currently enabled. When enabled, this is indicated in beacon frames.
INT wifi_getApWmmEnable(INT apIndex, BOOL *output);                   

/* wifi_setApWmmEnable() function */
/**
* @brief Enables/disables WMM on the hardwawre for this AP.  enable==1, disable == 0.
*
* @param[in] apIndex  Access Point index
* @param[in] enable   WMM support enabled status
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setApWmmEnable(INT apIndex, BOOL enable);                    // enables/disables WMM on the hardwawre for this AP.  enable==1, disable == 0

/* wifi_getApWmmUapsdEnable() function */
/**
* @brief Indicates whether U-APSD support is currently enabled.
*
* When enabled, this is indicated in beacon frames.
* U-APSD can only be enabled if WMM is also enabled. Device.WiFi.AccessPoint.{i}.UAPSDEnable
*
* @param[in]  apIndex  Access Point index
* @param[out] output   U-APSD support enabled status, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.AccessPoint.{i}.UAPSDEnable		
//Whether U-APSD support is currently enabled. When enabled, this is indicated in beacon frames. Note: U-APSD can only be enabled if WMM is also enabled.
INT wifi_getApWmmUapsdEnable(INT apIndex, BOOL *output);               

/* wifi_setApWmmUapsdEnable() function */
/**
* @brief Enables/disables Automatic Power Save Delivery on the hardwarwe for this Access Point.
*
* @param[in] apIndex  Access Point index
* @param[in] enable   U-APSD enable/disable value
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setApWmmUapsdEnable(INT apIndex, BOOL enable);               // enables/disables Automatic Power Save Delivery on the hardwarwe for this AP

/* wifi_setApWmmOgAckPolicy() function */
/**
* @brief Sets the WMM ACK policy on the hardware.
*
* AckPolicy false means do not acknowledge, true means acknowledge.
*
* @param[in] apIndex    Access Point index
* @param[in] class
* @param[in] ackPolicy  Acknowledge policy
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
// Sets the WMM ACK polity on the hardware. AckPolicy false means do not acknowledge, true means acknowledge
INT wifi_setApWmmOgAckPolicy(INT apIndex, INT cla, BOOL ackPolicy);  //RDKB
			
/* wifi_getApIsolationEnable() function */
/**
* @brief Get Access Point isolation value.
*
* A value of true means that the devices connected to the Access Point are isolated
* from all other devices within the home network (as is typically the case for a Wireless Hotspot).
* Device.WiFi.AccessPoint.{i}.IsolationEnable
*
* @param[in]  apIndex  Access Point index
* @param[out] output   Access Point Isolation enable, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.AccessPoint.{i}.IsolationEnable	
//Enables or disables device isolation.	A value of true means that the devices connected to the Access Point are isolated from all other devices within the home network (as is typically the case for a Wireless Hotspot).	
INT wifi_getApIsolationEnable(INT apIndex, BOOL *output); //Tr181		

/* wifi_setApIsolationEnable() function */
/**
* @brief Enables or disables device isolation.
*
* A value of true means that the devices connected to the Access Point are isolated
* from all other devices within the home network (as is typically the case for a Wireless Hotspot).
* Device.WiFi.AccessPoint.{i}.IsolationEnable
*
* @param[in] apIndex  Access Point index
* @param[in] enable   Access Point Isolation enable value 
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setApIsolationEnable(INT apIndex, BOOL enable); //Tr181			


/**
* @brief Set Access Point Beacon TX rate.
*
* Device.WiFi.AccessPoint.{i}.X_RDKCENTRAL-COM_BeaconRate
*
* @param[in] apIndex      Access point index will be 0, 2,4,6,8 10, 12, 14(for 2.4G) only;
* @param[in] sBeaconRate  sBeaconRate could be "1Mbps"; "5.5Mbps"; "6Mbps"; "2Mbps"; "11Mbps"; "12Mbps"; "24Mbps"
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setApBeaconRate(INT apIndex, char *sBeaconRate);

/**
* @brief Get Access Point  Beacon TX rate.
*
* Device.WiFi.AccessPoint.{i}.X_RDKCENTRAL-COM_BeaconRate
*
* @param[in]  apIndex             Index of Wi-Fi Access Point
* @param[out] output_BeaconRate   Beacon rate output
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getApBeaconRate(INT apIndex, char *output_BeaconRate);
	

/* wifi_getApMaxAssociatedDevices() function */
/**
* @brief Get maximum associated devices with the Access Point index.
*
* The maximum number of devices that can simultaneously be connected to the access point.
* A value of 0 means that there is no specific limit.
* Device.WiFi.AccessPoint.{i}.MaxAssociatedDevices
*
* @param[in]  apIndex  Access Point index
* @param[out] output   Maximum associated devices, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.AccessPoint.{i}.MaxAssociatedDevices	
//The maximum number of devices that can simultaneously be connected to the access point. A value of 0 means that there is no specific limit.			
INT wifi_getApMaxAssociatedDevices(INT apIndex, UINT *output); //Tr181		

/* wifi_setApMaxAssociatedDevices() function */
/**
* @brief Set maximum associated devices with the Access Point index.
*
* The maximum number of devices that can simultaneously be connected to the access point.
* A value of 0 means that there is no specific limit.
* Device.WiFi.AccessPoint.{i}.MaxAssociatedDevices
*
* @param[in] apIndex  Access Point index
* @param[in] number   Maximum associated devices
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setApMaxAssociatedDevices(INT apIndex, UINT number); //Tr181					
					
/* wifi_getApAssociatedDevicesHighWatermarkThreshold() function */
/**
* @brief Get the HighWatermarkThreshold value, that is lesser than or equal to MaxAssociatedDevices.
*
* Setting this parameter does not actually limit the number of clients that can associate with this access point
* as that is controlled by MaxAssociatedDevices.
* MaxAssociatedDevices or 50.
* The default value of this parameter should be equal to MaxAssociatedDevices.
* In case MaxAssociatedDevices is 0 (zero), the default value of this parameter should be 50.
* A value of 0 means that there is no specific limit and Watermark calculation algorithm should be turned off.
* Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_AssociatedDevicesHighWatermarkThreshold
*
* @param[in]  apIndex  Access Point index
* @param[out] output   HighWatermarkThreshold value, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getApAssociatedDevicesHighWatermarkThreshold(INT apIndex, UINT *output); //Tr181	//P3

/* wifi_setApAssociatedDevicesHighWatermarkThreshold() function */
/**
* @brief Set the HighWatermarkThreshold value, that is lesser than or equal to MaxAssociatedDevices.
*
* Setting this parameter does not actually limit the number of clients that can associate with this access point
* as that is controlled by MaxAssociatedDevices.
* MaxAssociatedDevices or 50.
* The default value of this parameter should be equal to MaxAssociatedDevices.
* In case MaxAssociatedDevices is 0 (zero), the default value of this parameter should be 50. 
* A value of 0 means that there is no specific limit and Watermark calculation algorithm should be turned off.
* Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_AssociatedDevicesHighWatermarkThreshold
*
* @param[in] apIndex    Access Point index
* @param[in] Threshold  HighWatermarkThreshold value 
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setApAssociatedDevicesHighWatermarkThreshold(INT apIndex, UINT Threshold); //Tr181		//P3			

/* wifi_getApAssociatedDevicesHighWatermarkThresholdReached() function */
/**
* @brief Get the number of times the current total number of associated device has reached the HighWatermarkThreshold value.
*
* This calculation can be based on the parameter AssociatedDeviceNumberOfEntries as well.
* Implementation specifics about this parameter are left to the product group and the device vendors.
* It can be updated whenever there is a new client association request to the access point.
* Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_AssociatedDevicesHighWatermarkThresholdReached
*
* @param[in]  apIndex Access Point index
* @param[out] output  Number of times the current total number of associated device has reached
*                     the HighWatermarkThreshold value, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getApAssociatedDevicesHighWatermarkThresholdReached(INT apIndex, UINT *output); //Tr181 //P3

/* wifi_getApAssociatedDevicesHighWatermark() function */
/**
* @brief Maximum number of associated devices that have ever associated with the access point concurrently
* since the last reset of the device or WiFi module.
* Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_AssociatedDevicesHighWatermark
*
* @param[in]  apIndex  Access Point index
* @param[out] output   Maximum number of associated devices that have ever associated with the access point concurrently,
*                      to be returned 
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getApAssociatedDevicesHighWatermark(INT apIndex, UINT *output); //Tr181	//P3

/* wifi_getApAssociatedDevicesHighWatermarkDate() function */
/**
* @brief Get Date and Time at which the maximum number of associated devices ever associated with the access point
* concurrently since the last reset of the device or WiFi module.
*
* In short when was X_COMCAST-COM_AssociatedDevicesHighWatermark updated
* This dateTime value is in UTC.
* Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_AssociatedDevicesHighWatermarkDate
*
* @param[in]   apIndex             Access Point index
* @param[out]  output_in_seconds   Date and Time at which the maximum number of associated 
*                                  devices ever associated with the access point
*                                  concurrenlty, to be returned.
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getApAssociatedDevicesHighWatermarkDate(INT apIndex, ULONG *output_in_seconds); //Tr181	//P3

					
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_InterworkingServiceCapability	boolean	R	
//When true, indicates whether the access point supports interworking with external networks.	

//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_InterworkingServiceEnable	boolean	W	
//Enables or disables capability of the access point to intework with external network. When enabled, the access point includes Interworking IE in the beacon frames.	

//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_PasspointCapability	boolean	R	
//Indicates whether this access point supports Passpoint (aka Hotspot 2.0). The Passpoint enabled AccessPoint must use WPA2-Enterprise security and WPS must not be enabled.	

//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_PasspointEnable	boolean	W	
//Whether Passpoint (aka Hotspot 2.0) support is currently enabled. When enabled, Passpoint specific information elemenets are indicated in beacon frames.	

//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_MAC_FilteringMode	string	R	
//"The current operational state of the MAC Filtering Mode, Enumeration of:    Allow-ALL, Allow, Deny			

//-----------------------------------------------------------------------------------------------				  
//Device.WiFi.AccessPoint.{i}.Security.	

/* wifi_getApSecurityModesSupported() function */
/**
* @brief Indicates which security modes this AccessPoint instance is capable of supporting.
*
* Each list item is an enumeration of:
* - None
* - WEP-64
* - WEP-128
* - WPA-Personal
* - WPA2-Personal
* - WPA-WPA2-Personal
* - WPA-Enterprise
* - WPA2-Enterprise
* - WPA-WPA2-Enterprise.
* Device.WiFi.AccessPoint.{i}.Security.ModesSupported
*
* @param[in]  apIndex  Access Point index
* @param[out] output   Comma-separated list of security modes, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getApSecurityModesSupported(INT apIndex, CHAR *output); 			
			
/* wifi_getApSecurityModeEnabled() function */
/**
* @brief Get the Security modes supported.
*
* The value MUST be a member of the list reported by the ModesSupported parameter.
* Indicates which security mode is enabled.
* Device.WiFi.AccessPoint.{i}.Security.ModeEnabled	string	W
*
* @param[in]  apIndex  Access Point index
* @param[out] output   Enabled security mode, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getApSecurityModeEnabled(INT apIndex, CHAR *output);    

/* wifi_setApSecurityModeEnabled() function */
/**
* @brief Enable supported security mode.
*
* The value MUST be a member of the list reported by the ModesSupported parameter. Indicates which security mode is enabled.
* Device.WiFi.AccessPoint.{i}.Security.ModeEnabled	string	W
*
* @param[in] apIndex  Access Point index
* @param[in] encMode  Supported security mode
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setApSecurityModeEnabled(INT apIndex, CHAR *encMode);        

//Device.WiFi.AccessPoint.{i}.Security.WEPKey	
//A WEP key expressed as a hexadecimal string.

/* wifi_getApSecurityPreSharedKey() function */
/**
* @brief Get PreSharedKey associated with a Access Point.
*
* A literal PreSharedKey (PSK) expressed as a hexadecimal string.
* Device.WiFi.AccessPoint.{i}.Security.PreSharedKey
*
* @param[in]  apIndex        Access Point index
* @param[out] output_string  PreSharedKey, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.AccessPoint.{i}.Security.PreSharedKey		
//A literal PreSharedKey (PSK) expressed as a hexadecimal string.
INT wifi_getApSecurityPreSharedKey(INT apIndex, CHAR *output_string);         // output_string must be pre-allocated as 64 character string by caller

/* wifi_setApSecurityPreSharedKey() function */
/**
* @brief Set PreSharedKey associated with a Access Point.
*
* A literal PreSharedKey (PSK) expressed as a hexadecimal string.
* Device.WiFi.AccessPoint.{i}.Security.PreSharedKey
*
* @param[in] apIndex       Access Point index
* @param[in] preSharedKey  PreSharedKey
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setApSecurityPreSharedKey(INT apIndex, CHAR *preSharedKey);          // sets an environment variable for the psk. Input string preSharedKey must be a maximum of 64 characters

/* wifi_getApSecurityKeyPassphrase() function */
/**
* @brief Get a passphrase from which the PreSharedKey is to be generated for WPA-Personal or WPA2-Personal or
*
* WPA-WPA2-Personal security modes.
*
* Device.WiFi.AccessPoint.{i}.Security.KeyPassphrase string-(63)	W
*
* @param[in]  apIndex         Access Point index
* @param[out] output_string   Security key passphrase, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getApSecurityKeyPassphrase(INT apIndex, CHAR *output_string);

/* wifi_setApSecurityKeyPassphrase() function */
/**
* @brief Set a passphrase from which the PreSharedKey is to be generated, for WPA-Personal or WPA2-Personal or
* WPA-WPA2-Personal security modes.
*
* Device.WiFi.AccessPoint.{i}.Security.KeyPassphrase string-(63)	W
*
* @param[in] apIndex      Access Point index
* @param[in] passPhrase   Security key passphrase
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setApSecurityKeyPassphrase(INT apIndex, CHAR *passPhrase);           // sets the passphrase environment variable, max 63 characters

//Device.WiFi.AccessPoint.{i}.Security.RekeyingInterval	unsignedInt	W	
//The interval (expressed in seconds) in which the keys are re-generated.
//INT wifi_getApSecurityWpaRekeyInterval(INT apIndex, INT *output_int);         // outputs the rekey interval
//INT wifi_setApSecurityWpaRekeyInterval(INT apIndex, INT rekeyInterval);       // sets the internal variable for the rekey interval

/* wifi_setApSecurityReset() function */
/**
* @brief When set to true, this AccessPoint instance's WiFi security settings are reset to their factory default values.
*
* The affected settings include ModeEnabled, WEPKey, PreSharedKey and KeyPassphrase.
* Device.WiFi.AccessPoint.{i}.Security.Reset
*
* @param[in] apIndex  Access Point index
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.AccessPoint.{i}.Security.Reset	
//When set to true, this AccessPoint instance's WiFi security settings are reset to their factory default values. The affected settings include ModeEnabled, WEPKey, PreSharedKey and KeyPassphrase.
INT wifi_setApSecurityReset(INT apIndex);

//Device.WiFi.AccessPoint.{i}.Security.X_COMCAST-COM_KeyPassphrase	string(63)	RW	
//A passphrase from which the PreSharedKey is to be generated, for WPA-Personal or WPA2-Personal or WPA-WPA2-Personal security modes.	If KeyPassphrase is written, then PreSharedKey is immediately generated. The ACS SHOULD NOT set both the KeyPassphrase and the PreSharedKey directly (the result of doing this is undefined). The key is generated as specified by WPA, which uses PBKDF2 from PKCS #5: Password-based Cryptography Specification Version 2.0 ([RFC2898]).	This custom parameter is defined to enable reading the Passphrase via TR-069 /ACS. When read it should return the actual passphrase			
//INT wifi_getApKeyPassphrase(INT apIndex, CHAR *output); //Tr181	
//INT wifi_setApKeyPassphrase(INT apIndex, CHAR *passphase); //Tr181	

//Device.WiFi.AccessPoint.{i}.Security.X_COMCAST-COM_WEPKey	string	RW	
//A WEP key expressed as a hexadecimal string.	WEPKey is used only if ModeEnabled is set to WEP-64 or WEP-128.	A 5 byte WEPKey corresponds to security mode WEP-64 and a 13 byte WEPKey corresponds to security mode WEP-128.	This custom parameter is defined to enable reading the WEPKey via TR-069/ACS. When read it should return the actual WEPKey.	If User enters 10 or 26 Hexadecimal characters, it should return keys as Hexadecimal characters.	If user enters 5 or 13 ASCII character key it should return key as ASCII characters.			

/* wifi_getApSecurityMFPConfig() function */
/**
* @brief To retrive the MFPConfig for each VAP
*
* The affected settings Device.WiFi.AccessPoint.{i}.Security.Reset
*
* @param[in] apIndex  Access Point index
* @param[out] output_string. Preallocated buffer for 64bytes. Allowed output string are "Disabled", "Optional", "Required"
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getApSecurityMFPConfig(INT apIndex, CHAR *output_string);

/* wifi_setApSecurityMFPConfig() function */
/**
* @brief the hal is used to set the MFP config for each VAP.
*        1. mfpconfig need to be saved into wifi config in persistent way (so that it could be automatically applied after the wifi or vap restart)
*        2. mfpconfig need to be applied right away.
*
* The affected settings include Device.WiFi.AccessPoint.{i}.Security.Reset
*
* @param[in] apIndex  Access Point index
* @param[in] MfpConfig,  The allowed string for MFPConfig are "Disabled", "Optional", "Required"
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setApSecurityMFPConfig(INT apIndex, CHAR *MfpConfig);


//-----------------------------------------------------------------------------------------------

/* wifi_getApSecurityRadiusServer() function */
/**
* @brief Get the IP Address and port number of the RADIUS server, which are used for WLAN security.
*
* RadiusServerIPAddr is only applicable when ModeEnabled is an Enterprise type
* (i.e. WPA-Enterprise, WPA2-Enterprise or WPA-WPA2-Enterprise).
*  String is 64 bytes max.
* Device.WiFi.AccessPoint.{i}.Security.RadiusServerIPAddr
* Device.WiFi.AccessPoint.{i}.Security.RadiusServerPort
* Device.WiFi.AccessPoint.{i}.Security.RadiusSecret
*
* @param[in]  Index                 Access Point index
* @param[out] IP_output             IP Address, to be returned
* @param[out] Port_output           Port output, to be returned
* @param[out] RadiusSecret_output   Radius Secret output, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.AccessPoint.{i}.Security.RadiusServerIPAddr	
//Device.WiFi.AccessPoint.{i}.Security.RadiusServerPort	
//Device.WiFi.AccessPoint.{i}.Security.RadiusSecret
//The IP Address and port number of the RADIUS server used for WLAN security. RadiusServerIPAddr is only applicable when ModeEnabled is an Enterprise type (i.e. WPA-Enterprise, WPA2-Enterprise or WPA-WPA2-Enterprise).  String is 64 bytes max
INT wifi_getApSecurityRadiusServer(INT apIndex, CHAR *IP_output, UINT *Port_output, CHAR *RadiusSecret_output); //Tr181	

/* wifi_setApSecurityRadiusServer() function */
/**
* @brief Set the IP Address and port number of the RADIUS server, which are used for WLAN security.
*
* RadiusServerIPAddr is only applicable when ModeEnabled is an Enterprise type
* (i.e. WPA-Enterprise, WPA2-Enterprise or WPA-WPA2-Enterprise). String is 64 bytes max.
* Device.WiFi.AccessPoint.{i}.Security.RadiusServerIPAddr
* Device.WiFi.AccessPoint.{i}.Security.RadiusServerPort
* Device.WiFi.AccessPoint.{i}.Security.RadiusSecret
*
* @param[in] apIndex       Access Point index
* @param[in] IPAddress     IP Address
* @param[in] port          Port 
* @param[in] RadiusSecret  Radius Secret
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setApSecurityRadiusServer(INT apIndex, CHAR *IPAddress, UINT port, CHAR *RadiusSecret); //Tr181	

/* wifi_getApSecuritySecondaryRadiusServer() function */
/**
* @brief Get secondary IP Address, port number and RADIUS server.
*
* Used for WLAN security.
* RadiusServerIPAddr is only applicable when ModeEnabled is an Enterprise type
* (i.e. WPA-Enterprise, WPA2-Enterprise or WPA-WPA2-Enterprise).
* String is 64 bytes max.
*
* @param[in]   apIndex               Access Point index
* @param[out]  IP_output             IP Address, to be returned
* @param[out]  Port_output           Port,to be returned
* @param[out]  RadiusSecret_output   Radius Secret, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getApSecuritySecondaryRadiusServer(INT apIndex, CHAR *IP_output, UINT *Port_output, CHAR *RadiusSecret_output); //Tr181	

/* wifi_setApSecuritySecondaryRadiusServer() function */
/**
* @brief Set secondary IP Address, port number and RADIUS server, which are used for WLAN security.
*
* RadiusServerIPAddr is only applicable when ModeEnabled is an Enterprise type
* (i.e. WPA-Enterprise, WPA2-Enterprise or WPA-WPA2-Enterprise).
* String is 64 bytes max.
*
* @param[in]  apIndex       Access Point index
* @param[in]  IPAddress     IP Address
* @param[in]  port          Port
* @param[in]  RadiusSecret  Radius Secret
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setApSecuritySecondaryRadiusServer(INT apIndex, CHAR *IPAddress, UINT port, CHAR *RadiusSecret); //Tr181	

/*wifi_getApDASRadiusServer function
* @brief Get the IP Address and port number of the RADIUS DAS server, which are used for WLAN security.
*
* Device.WiFi.AccessPoint.{i}.Security.RadiusDASIPAddr
* Device.WiFi.AccessPoint.{i}.Security.RadiusDASPort
* Device.WiFi.AccessPoint.{i}.Security.RadiusDASSecret

* @param[in] apIndex                 Access Point index
* @param[in] IP_output               IP Address
* @param[in] Port_output             Port
* @param[in] RadiusdasSecret_output  Radius DAS Secret
*
* Radius DAS Secret is not applicable for open ssid
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/


INT wifi_getApDASRadiusServer(INT apIndex, CHAR *IP_output, UINT *Port_output, CHAR *RadiusdasSecret_output);

/* wifi_setApDASRadiusServer() function */
/**
* @brief Set the IP Address and port number of the RADIUS DAS server, which are used for WLAN security.
*
* Device.WiFi.AccessPoint.{i}.Security.RadiusDASIPAddr
* Device.WiFi.AccessPoint.{i}.Security.RadiusDASPort
* Device.WiFi.AccessPoint.{i}.Security.RadiusDASSecret
*
* @param[in] IPAddress     IP Address
* @param[in] port          Port
* @param[in] RadiusdasSecret  Radius Secret
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setApDASRadiusServer(INT apIndex, CHAR *IPAddress, UINT port, CHAR *RadiusdasSecret); //Tr181

/* wifi_getApSecurityRadiusSettings() function */
/**
* @brief Get Access Point security radius settings.
*
* Device.WiFi.AccessPoint.{i}.Security.X_COMCAST-COM_RadiusSettings.
*
* @param[in]   apIndex  Access Point index
* @param[out]  output   wifi_radius_setting_t info (*output), to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.AccessPoint.{i}.Security.X_COMCAST-COM_RadiusSettings.		
//Device.WiFi.AccessPoint.{i}.Security.X_COMCAST-COM_RadiusSettings.RadiusServerRetries	int	W	
//Device.WiFi.AccessPoint.{i}.Security.X_COMCAST-COM_RadiusSettings.RadiusServerRequestTimeout	int	W	
//Device.WiFi.AccessPoint.{i}.Security.X_COMCAST-COM_RadiusSettings.PMKLifetime	int	W	
//Device.WiFi.AccessPoint.{i}.Security.X_COMCAST-COM_RadiusSettings.PMKCaching	boolean	W	
//Device.WiFi.AccessPoint.{i}.Security.X_COMCAST-COM_RadiusSettings.PMKCacheInterval	int	W	
//Device.WiFi.AccessPoint.{i}.Security.X_COMCAST-COM_RadiusSettings.MaxAuthenticationAttempts	int	W	
//Device.WiFi.AccessPoint.{i}.Security.X_COMCAST-COM_RadiusSettings.BlacklistTableTimeout	int	W	
//Device.WiFi.AccessPoint.{i}.Security.X_COMCAST-COM_RadiusSettings.IdentityRequestRetryInterval	int	W	
//Device.WiFi.AccessPoint.{i}.Security.X_COMCAST-COM_RadiusSettings.QuietPeriodAfterFailedAuthentication	int	W		
INT wifi_getApSecurityRadiusSettings(INT apIndex, wifi_radius_setting_t *output); //Tr181	

/* wifi_setApSecurityRadiusSettings() function */
/**
* @brief Set Access Point security radius settings.
*
* Device.WiFi.AccessPoint.{i}.Security.X_COMCAST-COM_RadiusSettings.
*
* @param[in] apIndex  Access Point index
* @param[in] input    wifi_radius_setting_t info
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setApSecurityRadiusSettings(INT apIndex, wifi_radius_setting_t *input); //Tr181	


//-----------------------------------------------------------------------------------------------

/* wifi_getApWpsEnable() function */
/**
* @brief Outputs the WPS enable state of this access point  in output_bool.
*
* Device.WiFi.AccessPoint.{i}.WPS.
* Device.WiFi.AccessPoint.{i}.WPS.Enable
*
* @param[in]  apIndex       Access Point index
* @param[out] output_bool   WPS enable state, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.AccessPoint.{i}.WPS.
//Device.WiFi.AccessPoint.{i}.WPS.Enable	
//Enables or disables WPS functionality for this access point.
INT wifi_getApWpsEnable(INT apIndex, BOOL *output_bool);   

/* wifi_setApWpsEnable() function */
/**
* @brief Enables or disables WPS functionality for this access point.
*
* Sets the WPS enable environment variable for this ap to the value of enableValue, 1==enabled, 0==disabled.
* Device.WiFi.AccessPoint.{i}.WPS.
* Device.WiFi.AccessPoint.{i}.WPS.Enable
*
* @param[in] apIndex      Access Point index
* @param[in] enableValue  WPS enable state
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setApWpsEnable(INT apIndex, BOOL enableValue);    

/* wifi_getApWpsConfigMethodsSupported() function */
/**
* @brief Indicates WPS configuration methods supported by the device.
*
* Device.WiFi.AccessPoint.{i}.WPS.ConfigMethodsSupported	
* Each list item is an enumeration of: USBFlashDrive,Ethernet,ExternalNFCToken,IntegratedNFCToken,NFCInterface,PushButton,PIN.
*
* @param[in]   apIndex  Access Point index
* @param[out]  output   WPS configuration methods supported (Comma-separated list of strings), to be returned.
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getApWpsConfigMethodsSupported(INT apIndex, CHAR *output); //Tr181				

/* wifi_getApWpsConfigMethodsEnabled() function */
/**
* @brief Indicates WPS configuration methods enabled on the device.
*
* Each list item MUST be a member of the list reported by the ConfigMethodsSupported parameter.
* Device.WiFi.AccessPoint.{i}.WPS.ConfigMethodsEnabled	string	W
*
* @param[in]  apIndex         Access Point index
* @param[out] output_string   WPS configuration methods enabled, to be returned.
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getApWpsConfigMethodsEnabled(INT apIndex, CHAR *output_string); // Outputs a common separated list of the enabled WPS config methods, 64 bytes max

/* wifi_setApWpsConfigMethodsEnabled() function */
/**
* @brief Enable WPS configuration methods on the device.
*
* Each list item MUST be a member of the list reported by the ConfigMethodsSupported parameter.
* Device.WiFi.AccessPoint.{i}.WPS.ConfigMethodsEnabled	string	W
* Sets an environment variable that specifies the WPS configuration method(s).
* methodString is a comma separated list of methods:
* - USBFlashDrive
* - Ethernet
* - ExternalNFCToken
* - IntegratedNFCToken
* - NFCInterface
* - PushButton
* - PIN
*
* @param[in]  apIndex        Access Point index
* @param[in]  methodString   WPS configuration methods enabled.
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setApWpsConfigMethodsEnabled(INT apIndex, CHAR *methodString);
/* wifi_getApWpsDevicePIN() function */
/**
* @brief Outputs the WPS device pin value, ulong_pin must be allocated by the caller.
*
* @param[in] apIndex       Access Point index
* @param[in] output_ulong  WPS Device PIN value, to be returned.
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getApWpsDevicePIN(INT apIndex, ULONG *output_ulong);

/* wifi_setApWpsDevicePIN() function */
/**
* @brief Set an environment variable for the WPS pin for the selected Access Point.
*
* @param[in] apIndex  Access Point index
* @param[in] pin      WPS Device PIN value
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setApWpsDevicePIN(INT apIndex, ULONG pin);

/* wifi_getApWpsConfigurationState() function */
/**
* @brief Get WPS configuration state.
*
* Output string is either Not configured or Configured, max 32 characters.
*
* @param[in] apIndex        Access Point index
* @param[in] output_string  WPS configuration state, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getApWpsConfigurationState(INT apIndex, CHAR *output_string); // Output string is either Not configured or Configured, max 32 characters

/* wifi_setApWpsEnrolleePin() function */
/**
* @brief Sets the WPS pin for this Access Point.
*
* @param[in] apIndex  Access Point index
* @param[in]  pin     WPS enroll Pin
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setApWpsEnrolleePin(INT apIndex, CHAR *pin);                 // sets the WPS pin for this AP

/* wifi_setApWpsButtonPush() function */
/**
* @brief This function is called when the WPS push button has been pressed for this AP.
*
* @param[in] apIndex  Access Point index
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setApWpsButtonPush(INT apIndex);                             // This function is called when the WPS push button has been pressed for this AP

/* wifi_cancelApWPS() function */
/**
* @brief Cancels WPS mode for this Access Point.
*
* @param[in] apIndex  Access Point index
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_cancelApWPS(INT apIndex);                                    // cancels WPS mode for this AP

/* wifi_getApManagementFramePowerControl() function */
/**
* @brief Get the ApManagementFramePowerControl
*
* @param[in]  apIndex      Access Point index
* @param[out] output_dBm   ManagementFramePowerControl, to be returned 
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getApManagementFramePowerControl(INT apIndex, INT *output_dBm);  // Get the RadioManagementFramePowerControl supported. 

/* wifi_setApManagementFramePowerControl() function */
/**
* @brief Sets the ApManagementFramePowerControl 
*
* @param[in] apIndex  Access Point index
* @param[in] dBm      RadioManagementFramePowerControl
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setApManagementFramePowerControl(INT apIndex, INT dBm); // sets the RadioManagementFramePowerControl in db
//-----------------------------------------------------------------------------------------------

/* wifi_getApAssociatedDeviceDiagnosticResult() function */
/**
* @brief The function  provides a list of the devices currently associated with the access point.
*
* HAL funciton should allocate an data structure array, and return to caller with "associated_dev_array".
* Device.WiFi.AccessPoint.{i}.AssociatedDevice.{i}.	
*
* @param[in] apIndex                Access Point index
* @param[in] associated_dev_array   Associated device array, to be returned
* @param[in] output_array_size      Array size, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
//Device.WiFi.AccessPoint.{i}.AssociatedDevice.{i}.	  
//Device.WiFi.AccessPoint.{i}.AssociatedDevice.{i}.X_COMCAST-COM_OperatingStandard	
//Device.WiFi.AccessPoint.{i}.AssociatedDevice.{i}.X_COMCAST-COM_OperatingChannelBandwidth	
//Device.WiFi.AccessPoint.{i}.AssociatedDevice.{i}.X_COMCAST-COM_SNR
//Device.WiFi.AccessPoint.{i}.AssociatedDevice.{i}.X_COMCAST-COM_InterferenceSources	//P3
//Device.WiFi.AccessPoint.{i}.AssociatedDevice.{i}.X_COMCAST-COM_DataFramesSentAck		//P3
//Device.WiFi.AccessPoint.{i}.AssociatedDevice.{i}.X_COMCAST-COM_DataFramesSentNoAck	//P3
//Device.WiFi.AccessPoint.{i}.AssociatedDevice.{i}.X_COMCAST-COM_BytesSent
//Device.WiFi.AccessPoint.{i}.AssociatedDevice.{i}.X_COMCAST-COM_BytesReceived
//Device.WiFi.AccessPoint.{i}.AssociatedDevice.{i}.X_COMCAST-COM_RSSI
//Device.WiFi.AccessPoint.{i}.AssociatedDevice.{i}.X_COMCAST-COM_MinRSSI				//P3
//Device.WiFi.AccessPoint.{i}.AssociatedDevice.{i}.X_COMCAST-COM_MaxRSSI				//P3
//Device.WiFi.AccessPoint.{i}.AssociatedDevice.{i}.X_COMCAST-COM_Disassociations		//P3
//Device.WiFi.AccessPoint.{i}.AssociatedDevice.{i}.X_COMCAST-COM_AuthenticationFailures	//P3
//HAL funciton should allocate an data structure array, and return to caller with "associated_dev_array"
INT wifi_getApAssociatedDeviceDiagnosticResult(INT apIndex, wifi_associated_dev_t **associated_dev_array, UINT *output_array_size); //Tr181

/**
* @brief The function  provides a list of the devices currently associated with the access point.
*
* HAL funciton should allocate an data structure array, and return to caller with "associated_dev_array".
* Device.WiFi.AccessPoint.{i}.AssociatedDevice.{i}.	
*
* @param[in] apIndex                Access Point index
* @param[in] associated_dev_array   Associated device array, to be returned
* @param[in] output_array_size      Array size, to be returned
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/

INT wifi_getApAssociatedDeviceDiagnosticResult2(INT apIndex, wifi_associated_dev2_t **associated_dev_array, UINT *output_array_size); //Tr181	
INT wifi_getApAssociatedDeviceDiagnosticResult3(INT apIndex, wifi_associated_dev3_t **associated_dev_array, UINT *output_array_size);

INT wifi_getApAssociatedClientDiagnosticResult(INT apIndex, char *mac_addr, wifi_associated_dev3_t *dev_conn);

/* wifi_setClientDetailedStatisticsEnable() function */
/**
* @brief This function enabled/disabled collection of detailed statistics of associated clients on Access Point. 
*
* @param[in] radioIndex          Radio Index
* @param[in] enable  			enable/disable 
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setClientDetailedStatisticsEnable(INT radioIndex, BOOL enable);



//------------------------------------------------------------------------------------------------------
////SSID stearing APIs using blacklisting
//INT wifi_setSsidSteeringPreferredList(INT radioIndex,INT apIndex, INT *preferredAPs[32]);  // prevent any client device from assocating with this ipIndex that has previously had a valid assocation on any of the listed "preferred" SSIDs unless SsidSteeringTimeout has expired for this device. The array lists all APs that are preferred over this AP.  Valid AP values are 1 to 32. Unused positions in this array must be set to 0. This setting becomes active when committed.  The wifi subsystem must default to no preferred SSID when initalized.  
////Using the concept of an "preferred list" provides a solution to most use cases that requrie SSID Steering.  To implement this approach, the AP places the STA into the Access Control DENY list for a given SSID only if the STA has previously associated to one of the SSIDs in the "preferred list" that for SSID.
//INT wifi_setSsidSteeringTimout(INT radioIndex,INT apIndex, ULONG SsidSteeringTimout);  // only prevent the client device from assocatign with this apIndex if the device has connected to a preferred SSID within this timeout period - in units of hours.  This setting becomes active when committed.  


/* wifi_newApAssociatedDevice_callback() function */
/**
* @brief This call back will be invoked when new wifi client come to associate to Access Point.	
*
* @param[in] apIndex          Access Point Index
* @param[in] associated_dev   Associated device info
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
typedef INT ( * wifi_newApAssociatedDevice_callback)(INT apIndex, wifi_associated_dev_t *associated_dev);

/* wifi_newApAssociatedDevice_callback_register() function */
/**
* @brief Callback registration function.	
*
* @param[in] callback_proc  wifi_newApAssociatedDevice_callback callback function
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
void wifi_newApAssociatedDevice_callback_register(wifi_newApAssociatedDevice_callback callback_proc);

/* wifi_apDisassociatedDevice_callback() function */
/**
* @brief This call back will be invoked when new wifi client disassociates from Access Point.	
*
* @param[in] apIndex          Access Point Index
* @param[in] MAC   			  MAC address of disassociated device
* @param[in] event_type   	  type of disassociation, explicit or due to client inactivity
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
typedef INT ( * wifi_apDisassociatedDevice_callback)(INT apIndex, char *MAC, INT event_type);

/* wifi_apDisassociatedDevice_callback_register() function */
/**
* @brief Callback registration function.	
*
* @param[in] callback_proc  wifi_apDisassociatedDevice_callback callback function
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
void wifi_apDisassociatedDevice_callback_register(wifi_apDisassociatedDevice_callback callback_proc);

/* wifi_apDeAuthEvent_callback() function */
/**
* @brief This call back will be invoked when DeAuth Event (reason 2 wrong password) comes from client.
*
* @param[in] apIndex          Access Point Index
* @param[in] MAC              MAC address of client device
* @param[in] reason           reason for Auth Event
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
typedef INT ( * wifi_apDeAuthEvent_callback)(int ap_index, char *mac, int reason);

/* wifi_apDeAuthEvent_callback_register() function */
/**
* @brief Callback registration function.
*
* @param[in] callback_proc  wifi_apDeAuthEvent_callback callback function
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*/
void wifi_apDeAuthEvent_callback_register(wifi_apDeAuthEvent_callback callback_proc);

/**
* @brief This API returns the radio enabled status.
*
* The radio status switch in driver include Transmission status, background channel scan, capacity status etc
* Radio Status should be disabled by default.
* If driver do not support those switch, or switch has enabled by default, please just fillup with stumb function.
*
* @param[in]  radioIndex      Index of the radio array.
* @param[out] output_enable   Radio enabled status.
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_getRadioStatsEnable(INT radioIndex, BOOL *output_enable);

/**
* @brief This API is to enable/disable radio status.
*
* @param[in]  radioIndex      Index of the radio array.
* @param[out] output_enable   Radio enabled status.
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*
*/
INT wifi_setRadioStatsEnable(INT radioIndex, BOOL enable);


/**
* @brief This API set the CSA (Channel Switch Announcement) deauthentication to all clients before moving to a new channel,
* in the event some client doesn't support/react to CSA.
*
* This makes that client scan and re-connect faster then if we were to do nothing.
* This is an "automatic" kick-mac type of functionality, that happens during the CSA process.
* What happens is that after all clients should have moved to the new channel, and just before the radio moves to the new channel,
* it will broadcast (or unicast all clients) a deauth packet.
* This helps clients who don't understand or ignore CSA to quickly realize the ap is gone/moved channels, and to scan and
* reconnect quickly.
*
* @param[in] apIndex  The index of access point array
* @param[in] mode     Enum value indicates none, unicast, broadcast
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*/
INT wifi_setApCsaDeauth(INT apIndex, INT mode);  //mode(enum): none, ucast, bcast

/**
* @brief Enable/Disable scan filter in the driver.
*
* When scanfilter is enabled in the driver, we configure two values: enable: yes/no, and essid : <string>.
* When essid is blank (apIndex==-1), the configured SSID on that interface is used.
* when it's not empty (apIndex==0 to 15), the filter will apply to whatever ssid is provided.
*
* @param[in] apIndex  The index of access point array
* @param[in] mode     Enum value indicates disabled/enabled
* @param[in] essid    Extended Service Set Identifier
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
* @note This function must not suspend and must not invoke any blocking system
* calls. It should probably just send a message to a driver event handler task.
*/
INT wifi_setApScanFilter(INT apIndex, INT mode, CHAR *essid); //mode(enum): disabled, enabled, first; essid could be empty to get all matching ESSID

/***************************************************************************************/
/** @} */  //END OF GROUP WIFI_HAL_APIS


/**
 * @addtogroup WIFI_HAL_TYPES
 * @{
 */

/**
 * @brief Configuration per apIndex
 *
 * This defines the configuration for each @b apIndex added to a steering
 * group
 *
 * Channel utilization is to be sampled every @b utilCheckIntervalSec seconds,
 * and after collecting $b utilAvgCount samples, the steering event
 * @b WIFI_STEERING_EVENT_CHAN_UTILIZATION should be sent with the averaged value.
 *
 * Client active/inactive checking is done every @b inactCheckIntervalSec seconds
 * and if a given client is idle/inactive for @b inactCheckThresholdSec seconds then
 * it should be marked as inactive.  Whenever a client changes states between active
 * and inactive, the steering event @b WIFI_STEERING_EVENT_CLIENT_ACTIVITY should be
 * sent.
 */
typedef struct {
    INT         apIndex;

    UINT        utilCheckIntervalSec;   /**< Channel utilization check interval        */
    UINT        utilAvgCount;           /**< Number of samples to average           */

    UINT        inactCheckIntervalSec;  /**< Client inactive check internval        */
    UINT        inactCheckThresholdSec; /**< Client inactive threshold              */
} wifi_steering_apConfig_t;

/**
 * @brief Configuration per Client
 *
 * This defines the per-client, per-apIndex configuration settings.  The
 * high water mark + low water mark pairs define RSSI ranges, in which
 * given packet types (probe or auth) are responded to as long as the RSSI
 * of the request packet is within the defined range.
 *
 * The RSSI crossings define thresholds which result in steering events
 * being generated when a connected clients RSSI crosses above or below
 * the given threshold.
 *
 * authRejectReason, when non-zero, results in auth requests being
 * rejected with the given reason code.  When set to zero, auth requests
 * that do not fall in the RSSI hwm+lwm range will be silently ignored.
 *
 * @see https://supportforums.cisco.com/document/141136/80211-association-status-80211-deauth-reason-codes
 */
typedef struct {
    UINT        rssiProbeHWM;           /**< Probe response RSSI high water mark    */
    UINT        rssiProbeLWM;           /**< Probe response RSSI low water mark     */
    UINT        rssiAuthHWM;            /**< Auth response RSSI high water mark     */
    UINT        rssiAuthLWM;            /**< Auth response RSSI low water mark      */
    UINT        rssiInactXing;          /**< Inactive RSSI crossing threshold       */
    UINT        rssiHighXing;           /**< High RSSI crossing threshold           */
    UINT        rssiLowXing;            /**< Low RSSI crossing threshold            */
    UINT        authRejectReason;       /**< Inactive RSSI crossing threshold       */
} wifi_steering_clientConfig_t;

/**
 * @brief Wifi Disconnect Sources
 *
 * These are the possible sources of a wifi disconnect.
 * If the disconnect was initiated by the client, then @b DISCONNECT_SOURCE_REMOTE
 * should be used.
 * If initiated by the local AP, then @b DISCONNECT_SOURCE_LOCAL should be used.
 * If this information is not available, then @b DISCONNECT_SOURCE_UNKNOWN should be used.
 */
typedef enum {
    DISCONNECT_SOURCE_UNKNOWN               = 0,    /**< Unknown source             */
    DISCONNECT_SOURCE_LOCAL,                        /**< Initiated locally          */
    DISCONNECT_SOURCE_REMOTE                        /**< Initiated remotely         */
} wifi_disconnectSource_t;

/**
 * @brief Wifi Disconnect Types
 * These are the types of wifi disconnects.
 */
typedef enum {
    DISCONNECT_TYPE_UNKNOWN                 = 0,    /**< Unknown type               */
    DISCONNECT_TYPE_DISASSOC,                       /**< Disassociation             */
    DISCONNECT_TYPE_DEAUTH                          /**< Deauthentication           */
} wifi_disconnectType_t;

/**
 * @brief Wifi Steering Event Types
 * These are the different steering event types that are sent by the wifi_hal
 * steering library.
 */
typedef enum {
    WIFI_STEERING_EVENT_PROBE_REQ           = 1,    /**< Probe Request Event                             */
    WIFI_STEERING_EVENT_CLIENT_CONNECT,             /**< Client association completed successfully Event */
    WIFI_STEERING_EVENT_CLIENT_DISCONNECT,          /**< Client Disconnect Event                         */
    WIFI_STEERING_EVENT_CLIENT_ACTIVITY,            /**< Client Active Change Event                      */
    WIFI_STEERING_EVENT_CHAN_UTILIZATION,           /**< Channel Utilization Event                       */
    WIFI_STEERING_EVENT_RSSI_XING,                  /**< Client RSSI Crossing Event                      */
    WIFI_STEERING_EVENT_RSSI,                       /**< Instant Measurement Event                       */
    WIFI_STEERING_EVENT_AUTH_FAIL                   /**< Client Auth Failure Event                       */
} wifi_steering_eventType_t;

/**
 * @brief RSSI Crossing Values
 * These are the RSSI crossing values provided in RSSI crossing events
 */
typedef enum {
    WIFI_STEERING_RSSI_UNCHANGED            = 0,    /**< RSSI hasn't crossed        */
    WIFI_STEERING_RSSI_HIGHER,                      /**< RSSI went higher           */
    WIFI_STEERING_RSSI_LOWER                        /**< RSSI went lower            */
} wifi_steering_rssiChange_t;

/**
 * @brief STA datarate information
 * These are STA capabilities values
 */
typedef struct {
    UINT                            maxChwidth;         /**< Max bandwidth supported                */
    UINT                            maxStreams;         /**< Max spatial streams supported          */
    UINT                            phyMode;            /**< PHY Mode supported                     */
    UINT                            maxMCS;             /**< Max MCS  supported                     */
    UINT                            maxTxpower;         /**< Max TX power supported                 */
    UINT                            isStaticSmps;       /**< Operating in Static SM Power Save Mode */
    UINT                            isMUMimoSupported;  /**< Supports MU-MIMO                       */
} wifi_steering_datarateInfo_t;

typedef struct {
    BOOL                            linkMeas;           /**< Supports link measurement      */
    BOOL                            neighRpt;           /**< Supports neighbor reports      */
    BOOL                            bcnRptPassive;      /**< Supports Passive 11k scans     */
    BOOL                            bcnRptActive;       /**< Supports Active 11k scans      */
    BOOL                            bcnRptTable;        /**< Supports beacon report table   */
    BOOL                            lciMeas;            /**< Supports LCI measurement       */
    BOOL                            ftmRangeRpt;        /**< Supports FTM Range report      */
} wifi_steering_rrmCaps_t;

/**
 * @brief Probe Request Event Data
 * This data is provided with @b WIFI_STEERING_EVENT_PROBE_REQ
 */
typedef struct {
    mac_address_t                   client_mac;     /**< Client MAC Address         */
    UINT                            rssi;           /**< RSSI of probe frame        */
    BOOL                            broadcast;      /**< True if broadcast probe    */
    BOOL                            blocked;        /**< True if response blocked   */
} wifi_steering_evProbeReq_t;

/**
 * @brief Client Connect Event Data
 * This data is provided with @b WIFI_STEERING_EVENT_CLIENT_CONNECT
 */
typedef struct {
    mac_address_t                   client_mac;     /**< Client MAC Address                     */
    UINT                            isBTMSupported; /**< Client supports BSS TM                 */
    UINT                            isRRMSupported; /**< Client supports RRM                    */
    BOOL                            bandCap2G;      /**< Client is 2.4GHz capable               */
    BOOL                            bandCap5G;      /**< Client is 5GHz capable                 */
    wifi_steering_datarateInfo_t    datarateInfo;   /**< Client supported datarate information  */
    wifi_steering_rrmCaps_t         rrmCaps;        /**< Client supported RRM capabilites       */
} wifi_steering_evConnect_t;

/**
 * @brief Client Disconnect Event Data
 * This data is provided with @b WIFI_STEERING_EVENT_CLIENT_DISCONNECT
 */
typedef struct {
    mac_address_t                   client_mac;     /**< Client MAC Address         */
    UINT                            reason;         /**< Reason code of disconnect  */
    wifi_disconnectSource_t         source;         /**< Source of disconnect       */
    wifi_disconnectType_t           type;           /**< Disconnect Type            */
} wifi_steering_evDisconnect_t;

/**
 * @brief Client Activity Change Event Data
 * This data is provided with @b WIFI_STEERING_EVENT_CLIENT_ACTIVITY
 */
typedef struct {
    mac_address_t                   client_mac;     /**< Client MAC Address         */
    BOOL                            active;         /**< True if client is active   */
} wifi_steering_evActivity_t;

/**
 * @brief Channel Utilization Event Data
 * This data is provided with @b WIFI_STEERING_EVENT_CHAN_UTILIZATION
 */
typedef struct {
    UINT                            utilization;    /**< Channel Utilization 0-100  */
} wifi_steering_evChanUtil_t;

/**
 * @brief Client RSSI Crossing Event Data
 * This data is provided with @b WIFI_STEERING_EVENT_RSSI_XING
 */
typedef struct {
    mac_address_t                   client_mac;     /**< Client MAC Address         */
    UINT                            rssi;           /**< Clients current RSSI       */
    wifi_steering_rssiChange_t      inactveXing;    /**< Inactive threshold Value   */
    wifi_steering_rssiChange_t      highXing;       /**< High threshold Value       */
    wifi_steering_rssiChange_t      lowXing;        /**< Low threshold value        */
} wifi_steering_evRssiXing_t;

/**
 * @brief Client RSSI Measurement Event Data
 * This data is provided with @b WIFI_STEERING_EVENT_RSSI, which is sent in
 * response to a requset for the client's current RSSI measurement
 */
typedef struct {
    mac_address_t                   client_mac;     /**< Client MAC Address         */
    UINT                            rssi;           /**< Clients current RSSI       */
} wifi_steering_evRssi_t;

/**
 * @brief Auth Failure Event Data
 * This data is provided with @b WIFI_STEERING_EVENT_AUTH_FAIL
 */
typedef struct {
    mac_address_t                   client_mac;     /**< Client MAC Address         */
    UINT                            rssi;           /**< RSSI of auth frame         */
    UINT                            reason;         /**< Reject Reason              */
    BOOL                            bsBlocked;      /**< True if purposely blocked  */
    BOOL                            bsRejected;     /**< True if rejection sent     */
} wifi_steering_evAuthFail_t;

/**
 * @brief Wifi Steering Event
 * This is the data containing a single steering event.
 */
typedef struct {
    wifi_steering_eventType_t       type;           /**< Event Type                 */
    INT                             apIndex;        /**< apIndex event is from      */
    ULLONG                          timestamp_ms;   /**< Optional: Event Timestamp  */
    union {
        wifi_steering_evProbeReq_t      probeReq;   /**< Probe Request Data         */
        wifi_steering_evConnect_t       connect;    /**< Client Connect Data        */
        wifi_steering_evDisconnect_t    disconnect; /**< Client Disconnect Data     */
        wifi_steering_evActivity_t      activity;   /**< Client Active Change Data  */
        wifi_steering_evChanUtil_t      chanUtil;   /**< Channel Utilization Data   */
        wifi_steering_evRssiXing_t      rssiXing;   /**< Client RSSI Crossing Data  */
        wifi_steering_evRssi_t          rssi;       /**< Client Measured RSSI Data  */
        wifi_steering_evAuthFail_t      authFail;   /**< Auth Failure Data          */
    } data;
} wifi_steering_event_t;

/** @} */  //END OF GROUP WIFI_HAL_TYPES 

/**
 * @addtogroup WIFI_HAL_APIS
 * @{
 */

/**
 * @brief Wifi Steering Event Callback Definition
 *
 * This is the definition of the event callback provided when upper layer
 * registers for steering events.
 *
 * @warning the @b event passed to the callback is not dynamically
 * allocated the call back function handler must allocate wifi_steering_event_t
 * and copy the "event" into that
 */
typedef void (*wifi_steering_eventCB_t)(UINT steeringgroupIndex, wifi_steering_event_t *event);


/**
 * @brief Steering API Supported.
 *
 * This tells the upper layer if the steering API is supported or not.
 *
 * @return @b TRUE on platforms that support steering, @b FALSE if not
 */
extern BOOL     wifi_steering_supported(void);

/**
 * @brief Add a Steering Group.
 *
 * A steering group defines a group of apIndex's which can have steering done
 * between them.
 *
 * @param[in] steeringgroupIndex  Wifi Steering Group index
 * @param[in] cfg_2               2.4G apConfig
 * @param[in] cfg_5               5G apConfig
 *
 * @return RETURN_OK on success, RETURN_ERR on failure
 *
 * @warning All apIndex's provided within a group must have the same SSID,
 * encryption, and passphrase configured for steering to function properly.
 *
 * @note The hal need to allocate (no matter static or dynamic) to store those two config
 * if cfg_2 and cfg_5 are NULL, this steering group will be removed
 */
INT wifi_steering_setGroup(UINT steeringgroupIndex, wifi_steering_apConfig_t *cfg_2, wifi_steering_apConfig_t *cfg_5);

/**
 * @brief Register for Steering Event Callbacks.
 *
 * This is called by the upper layer to register for steering event
 * callbacks.
 *
 * @param[in]  event_cb Event callback function pointer.
 *
 * @return RETURN_OK on success, RETURN_ERR on failure.
 *
 * @warning the @b event passed to the callback should be a dynamically
 * allocated event which upper layer will free by calling wifi_steering_eventFree()
 */
INT wifi_steering_eventRegister(wifi_steering_eventCB_t event_cb);

/**
 * @brief Unregister for Steering Event Callbacks.
 *
 * This is called by the upper layer to stop receiving event callbacks.
 *
 * @return RETURN_OK on success, RETURN_ERR on failure
 */
INT wifi_steering_eventUnregister(void);


/**
 * @brief Add Client Config to apIndex.
 *
 * The upper layer calls this funciton to @b add/modify per-client configuration @p config
 * of @p client_mac for @p apIndex
 *
 * @param[in] steeringgroupIndex   Wifi Steering Group index
 * @param[in] apIndex              Accesspoint index the client config should be added to
 * @param[in] client_mac           The Client's MAC address.
 *                                 If client_mac is not there, the hal need to add record,
 *                                 else, the hal need to update the config
 * @param[in] config               The client configuration
 *
 * @return RETURN_OK on success, RETURN_ERR on failure
 */
INT wifi_steering_clientSet(
                                UINT steeringgroupIndex,
                                INT apIndex,
                                mac_address_t client_mac,
                                wifi_steering_clientConfig_t *config);

/**
 * @brief Remove Client Config from apIndex
 *
 * The upper layer calls this function to @b remove per-client configuration
 * of @p client_mac from @p apIndex
 *
 * @param[in] steeringgroupIndex  Wifi Steering Group index
 * @param[in] apIndex             Access point index, the client config to be removed.
 * @param[in] client_mac          The Client's MAC address
 *
 * @return RETURN_OK on success, RETURN_ERR on failure
 */
INT wifi_steering_clientRemove(
                                UINT steeringgroupIndex,
                                INT apIndex,
                                mac_address_t client_mac);

/**
 * @brief Initiate Instant Client RSSI Measurement.
 *
 * This initiates an instant client RSSI measurement.  The recommended method of
 * performing this measurement is to send five NUL wifi frames to the client, and
 * average the RSSI of the ACK frames returned.  This averaged RSSI value should
 * be sent back using @b WIFI_STEERING_EVENT_RSSI steering event type.
 * Instant measurement improves user experience by not reacting to false-positive
 * RSSI crossings.
 * If for some reason instant measurement is not supported, the function should
 * return RETURN_ERR and set errno to @b ENOTSUP.
 *
 * @param[in]  steeringgroupIndex  Wifi Steering Group index
 * @param[in]  apIndex             Access point index, the client config should be added to
 * @param[in]  client_mac          The Client's MAC address
 *
 * @return RETURN_OK on success, RETURN_ERR on failure.  Set errno to ENOTSUP if
 * instant measurement is not supported
 */
INT wifi_steering_clientMeasure(
                                UINT steeringgroupIndex,
                                INT apIndex,
                                mac_address_t client_mac);

/**
 * @brief Initiate a Client Disconnect.
 *
 * This is used by the upper layer to kick off a client, for steering purposes.
 *
 * @param[in]  steeringgroupIndex  Wifi Steering Group index
 * @param[in]  apIndex             The access point index, the client config should be added to
 * @param[in]  client_mac          The Client's MAC address
 * @param[in]  type                Disconnect Type
 * @param[in]  reason              Reason code to provide in deauth/disassoc frame.
 *
 * @return RETURN_OK on success, RETURN_ERR on failure
 * @see https://supportforums.cisco.com/document/141136/80211-association-status-80211-deauth-reason-codes
 */
INT wifi_steering_clientDisconnect(
                                UINT steeringgroupIndex,
                                INT apIndex,
                                mac_address_t client_mac,
                                wifi_disconnectType_t type,
                                UINT reason);


// Radio Timeout for device disassociation.
// Device.WiFi.Radio.i.X_RDKCENTRAL-COM_connectionTimeOut. Integer r/w (default 180 sec, range: 15 to 1200)
//INT wifi_getRadioConnectionTimeOut(INT radioIndex, INT *output_timout_sec);
//INT wifi_setRadioConnectionTimeOut(INT radioIndex, INT timout_sec);

//This call back will be invoked when driver detect the client "authentication fail".
//event_type: 0=unknow reason; 1=wrong password; 2=timeout;
typedef INT ( * wifi_apAuthEvent_callback)(INT apIndex, char *MAC, INT event_type);
//Callback registration function.
void wifi_apAuthEvent_callback_register(wifi_apAuthEvent_callback callback_proc);
/** @} */  //END OF GROUP WIFI_HAL_APIS

/**
 * @addtogroup WIFI_HAL_TYPES
 * @{
 */

// The pulling function to retrieve the existing authenticated device Mac for specified VAP
typedef struct _mac_t {
	unsigned char byte[6];
} mac_t;
/** @} */  //END OF GROUP WIFI_HAL_TYPES

/**
 * @addtogroup WIFI_HAL_APIS
 * @{
 */

INT wifi_getApAuthenticatedDevices(INT apIndex, mac_t **mac_array, UINT *output_array_size);

//The call back function to send the notification for new authenticated devices.
typedef INT ( * wifi_apAuthenticatedEvent_callback)(INT apIndex, char *MAC);

//Callback registration function.
void wifi_apAuthenticatedEvent_callback_register(wifi_apAuthEvent_callback callback_proc);

/** @} */  //END OF GROUP WIFI_HAL_APIS

/**
 * @addtogroup WIFI_HAL_TYPES
 * @{
 */

//Mode 1: When a client connect or associate message is received by the AP, then the WiFi HAL lay must invoke wifi_apAssociatedDevice_callback with event_type: CONN_NEW.
#define CONN_NEW 1
//Mode 2: If the AP status for a client is connected or associated and the AP receives a client connect or associate message from this client, wifi_apAssociatedDevice_callback need to be invoked from hal layer to notify RDKB with event_type: CONN_RENEW
#define CONN_RENEW 2
//Mode 3: If the AP changes a client’s status to “disconnected” due to the AP’s client inactivity timeout (RDKB could read this timeout from wifi_getRadioClientInactivityTimout ) and then the client re-connects or associates back to same AP , then the WiFi HAL layer must invoke a wifi_apAssociatedDevice_callback with event_type: CONN_RECONN_AFTER_INACTIVITY
#define CONN_RECONN_AFTER_INACTIVITY 3

/** @} */  //END OF GROUP WIFI_HAL_TYPES

/**
 * @addtogroup WIFI_HAL_APIS
 * @{
 */
//Device.WiFi.Radio.i.X_RDKCENTRAL-COM_clientInactivityTimout. Integer ro
//This is used to read the ClientInactivityTimout from driver.
INT wifi_getRadioClientInactivityTimout(INT radioIndex, INT *output_timout_sec);

//This call back will be invoked when driver detect the client connection or association happen.
typedef INT ( * wifi_apAssociatedDevice_callback)(INT apIndex, char *MAC, INT event_type);

//Callback registration function.
void wifi_apAssociatedDevice_callback_register(wifi_apAssociatedDevice_callback callback_proc);


//-----------------------------------------------------------------------------------------------
//Device.WiFi.AccessPoint.{i}.X_RDKCENTRAL-COM_InterworkingServiceCapability
//Device.WiFi.AccessPoint.{i}.X_RDKCENTRAL-COM_InterworkingServiceEnable
//Device.WiFi.AccessPoint.{i}.X_RDKCENTRAL-COM_InterworkingElement.AccessNetworkType
//Access Network Type value to be included in the Interworking IE in the beaconds. (refer 8.4.2.94 of IEEE Std 802.11-2012). Possible values are: 0 - Private network;1 - Private network with guest access;2 - Chargeable public network;3 - Free public network;4 - Personal device network;5 - Emergency services only network;6-13 - Reserved;14 - Test or experimental;15 - Wildcard
INT wifi_setInterworkingAccessNetworkType(INT apIndex, INT accessNetworkType);   // P3
INT wifi_getInterworkingAccessNetworkType(INT apIndex, UINT *output_uint);   // P3
//Device.WiFi.AccessPoint.{i}.X_RDKCENTRAL-COM_InterworkingElement.Internet	
//Device.WiFi.AccessPoint.{i}.X_RDKCENTRAL-COM_InterworkingElement.ASRA
//Device.WiFi.AccessPoint.{i}.X_RDKCENTRAL-COM_InterworkingElement.ESR
//Device.WiFi.AccessPoint.{i}.X_RDKCENTRAL-COM_InterworkingElement.UESA
//Device.WiFi.AccessPoint.{i}.X_RDKCENTRAL-COM_InterworkingElement.VenueOptionPresent
//Device.WiFi.AccessPoint.{i}.X_RDKCENTRAL-COM_InterworkingElement.VenueGroup
//Device.WiFi.AccessPoint.{i}.X_RDKCENTRAL-COM_InterworkingElement.VenueType
//Device.WiFi.AccessPoint.{i}.X_RDKCENTRAL-COM_InterworkingElement.HESSOptionPresent
//Device.WiFi.AccessPoint.{i}.X_RDKCENTRAL-COM_InterworkingElement.HESSID	

/** @} */  //END OF GROUP WIFI_HAL_APIS

/**
 * @addtogroup WIFI_HAL_TYPES
 * @{
 */

// Interworking Element structure; see 802.11-2016 section 9.4.2.92 for field definition.
typedef struct {
	BOOL					interworkingEnabled;
	UINT 					accessNetworkType;
    BOOL 					internetAvailable;
    BOOL 					asra;
    BOOL 					esr;
    BOOL 					uesa;
    BOOL 					venueOptionPresent;    /**< True when venue information has not been provided, e.g. the hostspot is in a residence. */
	UCHAR					venueGroup;
    UCHAR 					venueType;
	BOOL					hessOptionPresent;
    mac_addr_str_t 			hessid;    /**< Optional; use empty string to indicate no value provided. */
} wifi_InterworkingElement_t;

/** @} */  //END OF GROUP WIFI_HAL_TYPES

/**
 * @addtogroup WIFI_HAL_APIS
 * @{
 */
/**
* @brief Get the Interworking Service Capability of the AP.
*
* @param[in] pIndex		Index of the Access Point.
* @param[in] output_bool	Indication as to whether the AP supports the Interworking Service.
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*/
INT wifi_getApInterworkingServiceCapability(INT apIndex, BOOL *output_bool);

/**
* @brief Get the Interworking Service enable/disable value for the AP.
*
* @param[in] apIndex		Index of the Access Point.
* @param[in] output_bool	Indication as to whether the AP Interworking Service is enabled (true) or disabled (false).
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*/
INT wifi_getApInterworkingServiceEnable(INT apIndex, BOOL *output_bool);

/**
 * @brief Set the Interworking Service enable/disable value for the AP.
 *
 * @param[in] apIndex		Index of the Access Point.
 * @param[in] input_bool		Value to set the Interworking Service enable to, true or false.
 *
 * @return The status of the operation
 * @retval RETURN_OK if successful
 * @retval RETURN_ERR if any error is detected
 */
INT wifi_setApInterworkingServiceEnable(INT apIndex, BOOL input_bool);

/**
 * @brief Get the Interworking Element that will be sent by the AP.
 *
 * @param[in] apIndex			Index of the Access Point.
 * @param[in] output_struct		Interworking Element.
 *
 * @return The status of the operation
 * @retval RETURN_OK if successful
 * @retval RETURN_ERR if any error is detected
 */
INT wifi_getApInterworkingElement(INT apIndex, wifi_InterworkingElement_t *output_struct);

INT	wifi_pushApInterworkingElement(INT apIndex, 
								wifi_InterworkingElement_t	*infoEelement);
/** @} */  //END OF GROUP WIFI_HAL_APIS

/**
 * @addtogroup WIFI_HAL_TYPES
 * @{
 */

// 802.11r Fast Trasition definitions.
typedef struct {
    mac_address_t   mac;
    nas_id_t        nasId;
    r0r1_key_t      key;
} wifi_r0KH_t;

typedef struct {
    mac_address_t   mac;
    mac_address_t   r1khId;
    r0r1_key_t      key;
} wifi_r1KH_t;

typedef enum {
	FT_SUPPORT_DISABLED,
	FT_SUPPORT_FULL,
	FT_SUPPORT_ADAPTIVE	
} wifi_fastTrasitionSupport_t;

#define MAX_KEY_HOLDERS		8
typedef struct {
	wifi_fastTrasitionSupport_t	support;
    USHORT                  mobilityDomain;
	BOOL					overDS;
    nas_id_t                r0KeyHolder;
    USHORT                  r0KeyLifeTime;
    mac_address_t           r1KeyHolder;
    USHORT                  reassocDeadLine;
    BOOL                    pmkR1Push;
    UCHAR                   numR0KHs;
    wifi_r0KH_t             r0KH[MAX_KEY_HOLDERS];
    UCHAR                   numR1KHs;
    wifi_r1KH_t             r1KH[MAX_KEY_HOLDERS];
} wifi_FastTransitionConfig_t;

/** @} */  //END OF GROUP WIFI_HAL_TYPES

/**
 * @addtogroup WIFI_HAL_APIS
 * @{
 */

/**
 * @brief Set the Fast Transition capability to disabled, full FT
**
 * @brief EAP/EAPOL Authenticator information.
 *
 * Structure which holds the the EAP/EAPOL Config param values.
 */
typedef struct _wifi_eap_config_t
{
    unsigned int    uiEAPOLKeyTimeout;
    unsigned int    uiEAPOLKeyRetries;
    unsigned int    uiEAPIdentityRequestTimeout;
    unsigned int    uiEAPIdentityRequestRetries;
    unsigned int    uiEAPRequestTimeout;
    unsigned int    uiEAPRequestRetries;
} wifi_eap_config_t;


/* @description Set the Fast Transition capability to disabled, full FT
 * support, or adaptive FT support.  Adaptive support is the same as full
 * support except the Mobility Domain Element is not sent in Beacon Frames.
 * 
 * Receipt of the TR-181 Object for writing
 * Device.WiFi.AccessPoint.{i}.X_RDKCENTRAL-COM_FTTransitionActivated via
 * TR-069 or WebPA causes this function to be called with the value of the
 * Object.
 * 
 * @param apIndex - AP Index the setting applies to.
 * @param activate - 0 = disabled, 1 = full FT support, 2 = adaptive support.
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */
INT wifi_setFastBSSTransitionActivated(INT apIndex, UCHAR activate);

/**
 * @brief Get the Fast Transition capability value.  
 * 
 * Receipt of the TR-181 Object for reading
 * Device.WiFi.AccessPoint.{i}.X_RDKCENTRAL-COM_BSSTransitionActivated via
 * TR-069 or WebPA causes this function to be called with the value of the
 * Object.
 * 
 * @param apIndex - AP Index the setting applies to.
 * @param activate - 0 = disabled, 1 = full FT support, 2 = adaptive support.
 *
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */
INT wifi_getBSSTransitionActivated(INT apIndex, BOOL *activate);


/**
 * @description Set the EAP authentication and EAPOL Handshake parameters.
 * EAPOL Key Timeout and max retries [M1 and M3]
 * EAP Identity Request and max retries
 * EAP Request Timeout and max retries
 * @param [in] apIndex - VAP number
 * @param [in] value - Either timeout or retry value
 * @param [in] param - Parameter string name to be configured as follows
 * eapolkey(timeout or retries),
 * eapidentityrequest(timeout or retries),
 * eaprequest(timeout or retries)
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */
INT wifi_setEAP_Param(UINT apIndex, UINT value, char  *param);

/* @description Get the EAP authentication and EAPOL Handshake parameters.
 * EAPOL Key Timeout and max retries [M1 and M3]
 * EAP Identity Request and max retries
 * EAP Request Timeout and max retries
 * @param [in] apIndex - VAP number
 * @param [in] output to be filled
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */
INT wifi_getEAP_Param(UINT apIndex, wifi_eap_config_t *output);

/* @description Get the Fast Transition over DS activated value.  
 * See 802.11-2016 section 13.3.
 * 
 * Receipt of the TR-181 Object for reading
 * Device.WiFi.AccessPoint.{i}.X_RDKCENTRAL-COM_FTOverDSActivated via
 * TR-069 or WebPA causes this function to be called with the value of the
 * Object.
 * 
 * @param apIndex - AP Index the setting applies to.
 * @param activate - True for activated (enabled), false for not activated
 * (disabled).
 *
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */
INT wifi_getFTOverDSActivated(INT apIndex, BOOL *activate);

/**
 * @brief Set the Fast Transition over DS activated value. 
 * See 802.11-2016 section 13.3. 
 * 
 * Receipt of the TR-181 Object for writing
 * Device.WiFi.AccessPoint.{i}.X_RDKCENTRAL-COM_FTOverDSActivated via
 * TR-069 or WebPA causes this function to be called with the value of the
 * Object.
 * 
 * @param apIndex - AP Index the setting applies to.
 * @param activate - True for activated (enabled), false for not activated
 * (disabled).
 *
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */
INT wifi_setFTOverDSActivated(INT apIndex, BOOL *activate);

/**
 * @brief Get the Fast Transition Mobility Domain value. 
 * See 802.11-2016 section 13.3. 
 * 
 * Receipt of the TR-181 Object for reading
 * Device.WiFi.AccessPoint.{i}.X_RDKCENTRAL-COM_FTMobilityDomain via
 * TR-069 or WebPA causes this function to be called with the value of the
 * Object.
 * 
 * @param apIndex - AP Index the setting applies to.
 * @param mobilityDomain - Value of the FT Mobility Domain for this AP.
 *
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */
INT wifi_getFTMobilityDomainID(INT apIndex, UCHAR mobilityDomain[2]);

/**
 * @brief Set the Fast Transition Mobility Domain value.  
 * See 802.11-2016 section 13.3.
 * 
 * Receipt of the TR-181 Object for writing
 * Device.WiFi.AccessPoint.{i}.X_RDKCENTRAL-COM_FTMobilityDomain via
 * TR-069 or WebPA causes this function to be called with the value of the
 * Object.
 * 
 * @param apIndex - AP Index the setting applies to.
 * @param mobilityDomain - Value of the FT Mobility Domain for this AP.
 *
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */
INT wifi_setFTMobilityDomainID(INT apIndex, UCHAR mobilityDomain[2]);

/**
 * @brief Get the Fast Transition Resource Request Support value. 
 * See 802.11-2016 section 13.3. 
 * 
 * Receipt of the TR-181 Object for reading
 * Device.WiFi.AccessPoint.{i}.X_RDKCENTRAL-COM_FTResourceRequestSupported via
 * TR-069 or WebPA causes this function to be called with the value of the
 * Object.
 * 
 * @param apIndex - AP Index the setting applies to.
 * @param supported - True is FT resource request supported, false is not
 * supported.
 *
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */
INT wifi_getFTResourceRequestSupported(INT apIndex, BOOL *supported);

/**
 * @brief Set the Fast Transition Resource Request Support value.  
 * See 802.11-2016 section 13.3.
 * 
 * Receipt of the TR-181 Object for writing
 * Device.WiFi.AccessPoint.{i}.X_RDKCENTRAL-COM_FTResourceRequestSupported via
 * TR-069 or WebPA causes this function to be called with the value of the
 * Object.
 * 
 * @param apIndex - AP Index the setting applies to.
 * @param suppored - True is FT resource request supported, false is not
 * supported.
 *
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */
INT wifi_setFTResourceRequestSupported(INT apIndex, BOOL *supported);

/**
 * @brief Get the Fast Transition R0 Key Lifetime value.  
 * See 802.11-2016 section 13.4.2.
 * 
 * Receipt of the TR-181 Object for reading
 * Device.WiFi.AccessPoint.{i}.X_RDKCENTRAL-COM_FTR0KeyLifetime via
 * TR-069 or WebPA causes this function to be called with the value of the
 * Object.
 * 
 * @param apIndex - AP Index the setting applies to.
 * @param lifetime - R0 Key Lifetime.
 *
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */
INT wifi_getFTR0KeyLifetime(INT apIndex, UINT *lifetime);

/**
 * @brief Set the Fast Transition R0 Key Lifetime value.  
 * See 802.11-2016 section 13.4.2
 * 
 * Receipt of the TR-181 Object for writing
 * Device.WiFi.AccessPoint.{i}.X_RDKCENTRAL-COM_FTR0KeyLifetime via
 * TR-069 or WebPA causes this function to be called with the value of the
 * Object.
 * 
 * @param apIndex - AP Index the setting applies to.
 * @param lifetime - R0 Key Lifetime.
 *
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */
INT wifi_setFTR0KeyLifetime(INT apIndex, UINT *lifetime);

/**
 * @brief Get the Fast Transition R0 Key Holder ID value.  
 * See 802.11-2016 section 13.3.
 * 
 * Receipt of the TR-181 Object for reading
 * Device.WiFi.AccessPoint.{i}.X_RDKCENTRAL-COM_FTR0KeyHolderID via
 * TR-069 or WebPA causes this function to be called with the value of the
 * Object.
 * 
 * @param apIndex - AP Index the setting applies to.
 * @param keyHolderID - R0 Key Holder ID string.
 *
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */
INT wifi_getFTR0KeyHolderID(INT apIndex, UCHAR *keyHolderID);

/**
 * @brief Set the Fast Transition R0 Key Holder ID value.  
 * See 802.11-2016 section 13.3.
 * 
 * Receipt of the TR-181 Object for writing
 * Device.WiFi.AccessPoint.{i}.X_RDKCENTRAL-COM_FTR0KeyHolderID via
 * TR-069 or WebPA causes this function to be called with the value of the
 * Object.
 * 
 * @param apIndex - AP Index the setting applies to.
 * @param keyHolderID - R0 Key Holder ID string.
 *
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */
INT wifi_setFTR0KeyHolderID(INT apIndex, UCHAR *keyHolderID);

/**
 * @brief Get the Fast Transition R1 Key Holder ID value.  
 * See 802.11-2016 section 13.3.
 * 
 * Receipt of the TR-181 Object for reading
 * Device.WiFi.AccessPoint.{i}.X_RDKCENTRAL-COM_FTR1KeyHolderID via
 * TR-069 or WebPA causes this function to be called with the value of the
 * Object.
 * 
 * @param apIndex - AP Index the setting applies to.
 * @param keyHolderID - R0 Key Holder ID string.
 *
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */
INT wifi_getFTR1KeyHolderID(INT apIndex, UCHAR *keyHolderID);

/**
 * @brief Set the Fast Transition R1 Key Holder ID value.  
 * See 802.11-2016 section 13.3.
 * 
 * Receipt of the TR-181 Object for writing
 * Device.WiFi.AccessPoint.{i}.X_RDKCENTRAL-COM_FTR1KeyHolderID via
 * TR-069 or WebPA causes this function to be called with the value of the
 * Object.
 * 
 * @param apIndex - AP Index the setting applies to.
 * @param keyHolderID - R0 Key Holder ID string.
 *
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */
INT wifi_setFTR1KeyHolderID(INT apIndex, UCHAR *keyHolderID);

INT wifi_pushApFastTransitionConfig(INT apIndex, wifi_FastTransitionConfig_t *ftData);

/** @} */  //END OF GROUP WIFI_HAL_APIS

/**
 * @addtogroup WIFI_HAL_TYPES
 * @{
 */

// 802.11v BSS Transition Management Definitions

#define MAX_BTM_DEVICES     64
#define MAX_URL_LEN         512
#define MAX_CANDIDATES      64
#define MAX_VENDOR_SPECIFIC 32

// BSS Termination Duration subelement, ID = 4, 802.11 section 9.4.2.2.
// This is a subelement because it is specific to Neighbor Report, and BTM
// Request Frame.
typedef struct {
    ULONG               tsf;    // 8 octet TSF timer value.
    USHORT              duration;
} wifi_BTMTerminationDuration_t;

typedef struct {
    CHAR                condensedStr[3];  // 2 char country code from do11CountryString.
} wifi_CondensedCountryString_t;

typedef struct {
    USHORT              offset;
    USHORT              interval;
} wifi_TSFInfo_t;

typedef struct {
    UCHAR               preference;
} wifi_BSSTransitionCandidatePreference_t;

typedef struct {
    USHORT              bearing;
    UINT                dist;
    USHORT              height;
} wifi_Bearing_t;

// Wide Bandwidth Channel Element, ID = 194.  802.11-2016 section 9.4.2.161.
typedef struct {
    UCHAR               bandwidth;
    UCHAR               centerSeg0;
    UCHAR               centerSeg1;
} wifi_WideBWChannel_t;

typedef struct {
    UCHAR                   token;
    UCHAR                   mode;
    UCHAR                   type;
    union {
        UCHAR               lci;
        UCHAR               lcr;
    } u;
} wifi_Measurement_t;

// HT Capabilities Element, ID = 45.  802.11-2016 section 9.4.2.56.
typedef struct {
    
    USHORT                  info;           // Bitfield where bit 0 is info[0] bit 0.
    UCHAR                   ampduParams;
    UCHAR                   mcs[16];        // Bitfield where bit 0 is mcs[0] bit 0.
    USHORT                  extended;       // Bitfield where bit 0 is ele_HTExtendedCapabilities[0] bit 0.
    UINT                    txBeamCaps;     // Bitfield where bit 0 is ele_TransmitBeamFormingCapabilities[0] bit 0.
    UCHAR                   aselCaps;
} wifi_HTCapabilities_t;

// VHT Capabilities Element, ID = 191.  802.11-2016 section 9.4.2.158.
typedef struct {
    UINT                    info;
    // The Supported VHT-MCS and NSS Set field is 64 bits long, but is broken
    // into 4 16 bit fields for convenience.
    USHORT                  mcs;
    USHORT                  rxHighestSupportedRate;
    USHORT                  txVHTmcs;
    USHORT                  txHighestSupportedRate;
} wifi_VHTCapabilities_t;

// HT OperationElement, ID = 61, 802.11-2016 section 9.4.2.57.
typedef struct {
    UCHAR                   primary;
    UCHAR                   opInfo[5];   // Bitfield where bit 0 is ele_HTOperationInfo[0] bit 0;
    UCHAR                   mcs[16];
} wifi_HTOperation_t;

// VHT Operation Element, ID = 192.  802.11-2016 section 9.4.2.159.
typedef struct {
    wifi_WideBWChannel_t        opInfo;         // channel width, center of seg0, center of seg1
    USHORT                      mcs_nss;        // Bit field.
} wifi_VHTOperation_t;

// Secondary Channel Offset Element, ID = 62, 802.11-2016 section
// 9.4.2.20.
typedef struct {
    UCHAR                       secondaryChOffset;
} wifi_SecondaryChannelOffset_t;

// RM Enabled Capabilities Element, ID = 70, 802.11-2016 section
// 9.4.2.45.
typedef struct {
    // This is a bit field defined by table 9-157.  Bit 0 for all of the
    // capabilities is ele_RMEnabledCapabilities[5] bit 0.
    UCHAR                       capabilities[5];
} wifi_RMEnabledCapabilities_t;

// Vendor Specific Element, ID = 221.  802.11-2016 section 9.4.2.26.
typedef struct {
    // 3 or 5 octet OUI depending on format; see 802.11-2016 section 9.4.1.32.
    UCHAR           oui[5];
    // Vendor specific content.
    UCHAR           buff[MAX_VENDOR_SPECIFIC];
} wifi_VendorSpecific_t;

// Measurement Pilot Transmission Element, ID = 66, 802.11-2016 section
// 9.4.2.42.
typedef struct {
    UCHAR                       pilot;
    // Series of (sub)elements.  Table 9-155 only lists vendor specific.
    wifi_VendorSpecific_t                       vendorSpecific;
} wifi_MeasurementPilotTransmission_t;


typedef struct {
    bssid_t             bssid;
    //  32 bit optional value, bit fileds are
    //  b0, b1 for reachability
    //  b2 security
    //  b3 key scope
    //  b4 to b9 capabilities
    //  b10 mobility domain
    //  b11 high troughput
    //  b12 very high throughput
    //  b13 ftm
    //  b14 to b31 reserved
    UINT                info;
    UCHAR               opClass;
    UCHAR               channel;
    UCHAR               phyTable;
    BOOL                tsfPresent;
    wifi_TSFInfo_t      tsfInfo;
    BOOL                condensedCountrySringPresent;
    wifi_CondensedCountryString_t   condensedCountryStr;
    BOOL                bssTransitionCandidatePreferencePresent;
    wifi_BSSTransitionCandidatePreference_t         bssTransitionCandidatePreference;
    BOOL                btmTerminationDurationPresent;
    wifi_BTMTerminationDuration_t   btmTerminationDuration;
    BOOL                bearingPresent;
    wifi_Bearing_t      bearing;
    BOOL                wideBandWidthChannelPresent;
    wifi_WideBWChannel_t    wideBandwidthChannel;
    BOOL                htCapsPresent;
    wifi_HTCapabilities_t   htCaps;
    BOOL                vhtCapsPresent;
    wifi_VHTCapabilities_t  vbhtCaps;
    BOOL                    htOpPresent;
    wifi_HTOperation_t      htOp;
    BOOL                    vhtOpPresent;
    wifi_VHTOperation_t     vhtOp;
    BOOL                    secondaryChannelOffsetPresent;
    wifi_SecondaryChannelOffset_t   secondaryChannelOffset;
    BOOL                    rmEnabledCapsPresent;
    wifi_RMEnabledCapabilities_t    rmEnabledCaps;
    BOOL                            msmtPilotTransmissionPresent;
    wifi_MeasurementPilotTransmission_t     msmtPilotTransmission;
    BOOL                    vendorSpecificPresent;
    wifi_VendorSpecific_t   vendorSpecific;
    ssid_t                  target_ssid;
} wifi_NeighborReport_t;

// BSS Transition Management Request Frame, 802.11-2016 section 9.6.14.9.
typedef struct {
    UCHAR               token;              /**< set by STA to relate reports */
    UCHAR               requestMode;        /**< Requested instructions for the STA. */
    USHORT              timer;
    UCHAR               validityInterval;
    // The optional fields may include:
    // 1. BSS Termination Duration Subelement, ID = 4. 802.11-2016 Figure 9-300.
    // 2. Session Information URL.
    // 3. BSS Transition Candidate List Entries
    wifi_BTMTerminationDuration_t    termDuration;
    UCHAR               disassociationImminent;
    USHORT              urlLen;
    CHAR                url[MAX_URL_LEN];
    UCHAR               numCandidates;
    wifi_NeighborReport_t    candidates[MAX_CANDIDATES];
} wifi_BTMRequest_t;

// BSS Transition Management Query Frame, 802.11-2016 section 9.6.14.8.
// Received from non-AP STA.
typedef struct {
    UCHAR                   token;          /**< set by STA to relate reports */
    UCHAR                   queryReason;
    UCHAR                   numCandidates;
    wifi_NeighborReport_t   candidates[MAX_CANDIDATES];
} wifi_BTMQuery_t;

// BSS Transition Management Response Frame, 802.11-2016 section 9.6.14.10.
// Received from non-AP STA.
typedef struct {
    UCHAR               token;          /**< set by STA to relate reports */
    UCHAR               status;
    UCHAR               terminationDelay;
    bssid_t             target;
    UCHAR                   numCandidates;
    wifi_NeighborReport_t    candidates[MAX_CANDIDATES];
} wifi_BTMResponse_t;

// Structure to return BTM extended capability from devices on the LAN.
// The peer and capability arrays are parallel
// and have the same number of entries.
typedef struct {
    UINT                entries;                        /**< Number of entries in each of the following arrays. */
    mac_address_t       peer[MAX_BTM_DEVICES];          /**< Array a peer device MAC addresses. */
    BOOL                capability[MAX_BTM_DEVICES];    /**< Array of bool indicating peer BSS transition capability. */
} wifi_BTMCapabilities_t;

/** @} */  //END OF GROUP WIFI_HAL_TYPES

/**
 * @addtogroup WIFI_HAL_APIS
 * @{
 */

/**
 * @brief This call back is invoked when a STA sends a BTM query
 * message to a vAP in the gateway.  The driver will use the frame returned
 * from this function to process the response to the query.
 * A BTM transaction is started by a STA sending a query or by the AP sending
 * an autonomous request.  This callback is used for the former.
 *
 * @param apIndex - Access Point Index.
 * @param peerMACAddress - MAC address of the peer STA the Query was received from.
 * @param inQueryFrame - Query frame received from a non-AP STA.
 * @param inMemSize - Size of the memory allocated by the callback.  The caller
 *      should set to max size for the request.  Otherwise the callback may
 *      drop elements or return an error.
 * @param inRequestFrame - Frame to use for the response.  The caller
 *      allocates the memory for the response.  The caller may free the memory
 *      when the callback returns and the response is sent to the STA.
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 *
 * @execution Synchronous
 * @sideeffect None
 *
 * @note This function must not suspend and must not invoke any blocking system
 * calls.
 */
typedef INT (* wifi_BTMQueryRequest_callback)(UINT apIndex,
                                                    CHAR *peerMac,
                                                    wifi_BTMQuery_t *query,
                                                    UINT inMemSize,
                                                    wifi_BTMRequest_t *request);

/**
 * @brief This call back is invoked when a STA responds to a BTM Request
 * from the gateway.
 *
 * @param apIndex - Access Point Index.
 * @param peerMACAddress - MAC address of the peer the response was received
 * from.
 * @param in_struct - Response frame received from a non-AP STA.
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 *
 * @execution Synchronous
 * @sideeffect None
 *
 * @note This function must not suspend and must not invoke any blocking system
 * calls.
 */
typedef INT (* wifi_BTMResponse_callback)(UINT apIndex,
                                            CHAR *peerMac,
                                            wifi_BTMResponse_t *response);
/**
 * @brief BTM Query callback registration function.
 *
 * @param callback_proc - wifi_newApAssociatedDevice_callback callback function
 *
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 *
 * @execution Synchronous
 * @sideeffect None
 *
 * @note This function must not suspend and must not invoke any blocking system
 * calls.
 */
INT wifi_BTMQueryRequest_callback_register(UINT apIndex,
                                            wifi_BTMQueryRequest_callback btmQueryCallback,
                                            wifi_BTMResponse_callback btmResponseCallback);

/**
 * @brief Set a BTM Request to a non-AP STA.  The callback register
 * function should be called first so that the response can be handled by the
 * application.
 *
 * @param apIndex; index of the vAP to send the request from.
 * @param peerMACAddress; MAC address of the peer device to send the request to.
 * @param in_struct; BTM Request Frame to send to the non-AP STA.
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 *
 * @execution Synchronous
 * @sideeffect None
 *
 * @note This function must not suspend and must not invoke any blocking system
 * calls.
 */
INT wifi_setBTMRequest(UINT apIndex,
                        CHAR       *peerMac,
                        wifi_BTMRequest_t *request);

/**
 * @brief Get the BTM implemented value.  When not implemented the
 * gateway ignores a BTM query request as defined in 802.11-2016 section
 * 11.11.10.3.
 *
 * @param apIndex - AP Index the setting applies to.
 * @param activate - True for implemented false for not implemented.
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */
INT wifi_getBSSTransitionImplemented(UINT apIndex, BOOL *activate);

/**
 * @brief Set the BTM capability to activated or deactivated,
 * same as enabled or disabled.  The word "activated" is used here because
 * that's what's used in the 802.11 specification.  When deactivate the
 * gateway ignores a BTM report request as defined in 802.11-2016 section
 * 11.11.10.3.  The AP (apIndex) BSS Transition bit in any Extended Capabilities
 * element sent out is set corresponding to the activate parameter.
 *
 * @param apIndex - AP Index the setting applies to.
 * @param activate - True for activate false for deactivate.
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */
INT wifi_setBSSTransitionActivation(UINT apIndex, BOOL activate);

/**
 * @description Get the BTM capability of activated or deactivated,
 * same as enabled or disabled.
 *
 * @param apIndex - AP Index the setting applies to.
 * @param activate - True for activate false for deactivate.
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */
INT wifi_getBSSTransitionActivation(UINT apIndex, BOOL *activate);

/**
 * @description Get the BTM capability of an external STA.  Reports the value
 * of the BSS Transition bit in the Extended Capabilities element, if detected,
 * from an external STA.  Reports the latest value detected in the element
 * received by any vAP in any frame type.
 *
 * @param apIndex - AP the Extended Capabilities elements were received on.
 * @param extBTMCapabilities - structure with parallel arrays of peer MAC
 * addresses and BTM capability indicators.
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */
INT wifi_getBTMClientCapabilityList(UINT apIndex,
                                     wifi_BTMCapabilities_t *extBTMCapabilities);

// 802.11k neighbor report definitions

/** @} */  //END OF GROUP WIFI_HAL_APIS

/**
 * @addtogroup WIFI_HAL_TYPES
 * @{
 */

// 802.11-2016 section 9.6.7.6
typedef struct {
    UCHAR                   token;       /**< set by STA to relate reports */
    UCHAR                   ssidLen;     /**< set length to 0 if ssid is not present, otherwise to length of ssid field */
    ssid_t                  ssid;
    UCHAR                   measCount;      /**< Request for LCI/LCR may come in any order */
    wifi_Measurement_t      measurements[2];
} wifi_NeighborRequestFrame_t;

/** @} */  //END OF GROUP WIFI_HAL_TYPES

/**
 * @addtogroup WIFI_HAL_APIS
 * @{
 */

/* @description Set the neighbor reports to be reported by the AP.  Calling
 * this function over-writes any previously set Neighbor BSSID set.  The set
 * is not persistent.
 *
 * @param apIndex - Index of the vAP to send the request from
 * @param in_NumberNeighborReports - Number of reports in the in_NeighborReports
 *      set.
 * @param in_NeighborReports - One or more neighbor reports to be reported by
 *      the AP.
 *
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */
INT wifi_setNeighborReports(UINT apIndex,
                             UINT numNeighborReports,
                             wifi_NeighborReport_t *neighborReports);

/* @description Set the neighbor report capability to activated or deactivated,
 * same as enabled or disabled.  The word "activated" is used here because
 * that's what's used in the 802.11 specification.  When deactivate the
 * gateway ignores a neighbor report request as defined in 802.11-2016 section
 * 11.11.10.3.
 *
 * Reciept of the TR-181 Object
 * Device.WiFi.AccessPoint.{i}.X_RDKCENTRAL-COM_NeighborReportActivated via
 * TR-069 or WebPA causes this function to be called with the value of the
 * Object.
 *
 * @param apIndex - AP Index the setting applies to.
 * @param activate - True for activate false for deactivate.
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */
INT wifi_setNeighborReportActivation(UINT apIndex, BOOL activate);

/* @description Get the neighbor report capability of activated or deactivated,
 * same as enabled or disabled.
 *
 * Reciept of the TR-181 Object
 * Device.WiFi.AccessPoint.{i}.X_RDKCENTRAL-COM_NeighborReportActivated via
 * TR-069 or WebPA causes this function to be called with the value of the
 * Object.
 *
 * @param apIndex - AP Index the setting applies to.
 * @param activate - True for activate false for deactivate.
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */
INT wifi_getNeighborReportActivation(UINT apIndex, BOOL *activate);

/** @} */  //END OF GROUP WIFI_HAL_APIS

/**
 * @addtogroup WIFI_HAL_TYPES
 * @{
 */

// 802.11k Beacon request & report structures and function prototypes
#define MAX_REQUESTED_ELEMS     8
#define MAX_CHANNELS            16

typedef struct {
    UCHAR               condition;
    UCHAR               threshold;
} wifi_BeaconReporting_t;

typedef struct {
    UCHAR               ids[MAX_REQUESTED_ELEMS];
} wifi_RequestedElementIDS_t;

typedef wifi_RequestedElementIDS_t  wifi_ExtdRequestedElementIDS_t;

// AP Channel Report Element, ID = 51, 802.11-2016 section 9.4.2.36.
typedef struct {
    UCHAR               opClass;
    UCHAR               channels[MAX_CHANNELS];
} wifi_ChannelReport_t;

// 802.11-2016 section 9.4.2.21.7
typedef struct {
    UCHAR               opClass;
    UCHAR               channel;
    USHORT              randomizationInterval;
    USHORT              duration;
    UCHAR               mode;
    bssid_t             bssid;
    BOOL                ssidPresent;
    ssid_t              ssid;
    BOOL                beaconReportingPresent;
    wifi_BeaconReporting_t  beaconReporting;
    BOOL                reportingRetailPresent;
    UCHAR               reportingDetail;
    BOOL                wideBandWidthChannelPresent;
    wifi_WideBWChannel_t    wideBandwidthChannel;
    BOOL                requestedElementIDSPresent;
    wifi_RequestedElementIDS_t      requestedElementIDS;
    BOOL                extdRequestedElementIDSPresent;
    wifi_ExtdRequestedElementIDS_t  extdRequestedElementIDS;
    BOOL                channelReportPresent;
    wifi_ChannelReport_t    channelReport;
    BOOL                vendorSpecificPresent;
    wifi_VendorSpecific_t   vendorSpecific;
    USHORT               numRepetitions;
} wifi_BeaconRequest_t;

// 802.11-2016 section 9.4.2.22.7
typedef struct {
    UCHAR               opClass;
    UCHAR               channel;
    ULLONG              startTime;
    USHORT              duration;
    UCHAR               frameInfo;
    UCHAR               rcpi;
    UCHAR               rsni;
    bssid_t             bssid;
    UCHAR               antenna;
    UINT                tsf;
    BOOL                wideBandWidthChannelPresent;
    wifi_WideBWChannel_t    wideBandwidthChannel;
    USHORT              numRepetitions;
} wifi_BeaconReport_t;

/** @} */  //END OF GROUP WIFI_HAL_TYPES

/**
 * @addtogroup WIFI_HAL_APIS
 * @{
 */

/* @description This call back is invoked when a STA responds to a Beacon
 * Request from the gateway, or as a triggered autonomous report.  Noting that
 * an autonomous report can be configured by a Beacon Request by setting the
 * enable, request, and report bits in the measurement request; 802.11-2016
 * Table 9-81 and section 11.11.8.  When a triggered autonomous report
 * causes the callback to be called the dialog token and measurement token are
 * both set to 0.
 *
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 *
 * @execution Synchronous
 * @sideeffect None
 *
 * @note This function must not suspend and must not invoke any blocking system
 * calls.
 */
typedef INT (*wifi_RMBeaconReport_callback)(UINT apIndex,
													wifi_BeaconReport_t *out_struct,
													UINT	*out_array_size,
                                                    UCHAR 	*out_DialogToken);

/* @description Register a callback for a Beacon Request.  Called when a
 * response to a Beacon Request is received, or a Beacon Report is received
 * from an autonomous trigger.
 *
 * @param apIndex; index of the vAP the Beacon Report was received on.
 * @param beaconReportCallback; the callback function being registered.
 *
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */
INT wifi_RMBeaconRequestCallbackRegister(UINT apIndex,
                                          wifi_RMBeaconReport_callback beaconReportCallback);

/* @description Unegister a callback for a Beacon Request.  Returns an error
 * if the callback hasn't been registered.
 *
 * @param apIndex; index of the vAP the Beacon Report was received on.
 * @param beaconReportCallback; the callback function being unregistered.
 *
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */
INT wifi_RMBeaconRequestCallbackUnregister(UINT apIndex,
                                            wifi_RMBeaconReport_callback beaconReportCallback);

/* @description Set a radio measurement (RM) beacon request.  Causes the
 * request to be sent based on the information in the request parameter.
 * Sent from the AP at apIndex.  Returns an error if a callback has not been
 * registered for the AP.
 *
 * @param apIndex; index of the vAP to send the request from.
 * @param peerMACAddress, MAC address of the peer device to send the request
 *      to.  Must be an external device MAC address.
 * @param in_request; pointer to a Beacon Report request structure.
 * @param out_DialogToken; the token chosen by the STA for the requested
 *      measurement(s);
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.  If the AP can determine that
 *      the target device does not support Radio Measurement, then an error
 *      is returned.
 */
INT wifi_setRMBeaconRequest(UINT apIndex,
                             CHAR *peer,
                             wifi_BeaconRequest_t *in_request,
                             UCHAR *out_DialogToken);

/* @description Cancel all of the currently cached beacon reports and ignore
 *      reports received that match the dialog tokan
 * // @param apIndex; index of the vAP the beacon request was sent from.
 * @param dialogToken; token the STA assigned to the beacon request.
 */
INT wifi_cancelRMBeaconRequest(UINT apIndex, UCHAR dialogToken);

/* @description Get the Radio Measurement Capabilities from another peer
 * device.
 *
 * @param peerMACAddress; MAC Address of the external peer device used to
 * determine if an Radio Measurement Capabiliites Element is available.
 * @param out_Capabilities; array formatted as defined in 802.11-2016
 * Table 9-157.  The Beacon Report Capability is indicated by bit 7.  The
 * Beacon Passive, Active, and Table Capabilities are indicated by bits
 * 4, 5, 6 respectively.
 *
 * @return The capabilities returned in a Radio Measurement Element if
 * received.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.  If the AP has not received
 * a Radio Measurement Element from the peer, then an error is returned.
 */
INT wifi_getRMCapabilities(CHAR *peer, UCHAR out_Capabilities[5]);

/** @} */  //END OF GROUP WIFI_HAL_APIS

/**
 * @addtogroup WIFI_HAL_TYPES
 * @{
 */

// VAP telemetry report structures and function prototypes
#define MAX_VAP 16
/**
 * @brief VAP Telemetry information.
 *
 * Structure which holds the VAP Telemetry information.
 */
typedef struct {
    UINT                txOverflow;            // WiFi TX overflow counter.
} wifi_VAPTelemetry_t;

/** @} */  //END OF GROUP WIFI_HAL_TYPES

/**
 * @addtogroup WIFI_HAL_APIS
 * @{
 */

/* @description Get the VAP Telemetry data.
 *
 * @param[in] apIndex - Access Point index.
 * @param[out] telemetry - structure for vap telemetry data.
 *
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */
INT wifi_getVAPTelemetry(UINT apIndex, wifi_VAPTelemetry_t *telemetry);

/* @description This call back is invoked when a STA responds to a DPP Authentication
 * Request from the gateway    with DPP Authentication Response
 *
 * @param apIndex; index of the vAP where the DPP Authentication Response frame is received
 * @param staMAC, MAC address of the peer device
 * @param status, one of wifi_dppAuthResponseStatus_t authentication response status
 *
 * @execution Synchronous
 * @sideeffect None
 *
 * @note This function must not suspend and must not invoke any blocking system
 * calls.
 */

/** @} */  //END OF GROUP WIFI_HAL_APIS

/**
 * @addtogroup WIFI_HAL_TYPES
 * @{
 */

/************* DPP *************************/
typedef enum {
    WIFI_DPP_TECH_INFRA
} wifi_dpp_technology_t;

typedef enum {
    WIFI_DPP_KEY_MGMT_PSK,
    WIFI_DPP_KEY_MGMT_DPP,
    WIFI_DPP_KEY_MGMT_SAE,
    WIFI_DPP_KEY_MGMT_PSKSAE,
    WIFI_DPP_KEY_MGMT_DPPPSKSAE
} wifi_dpp_key_management_t;

typedef struct {
      wifi_dpp_key_management_t keyManagement;
      union {
        unsigned char    preSharedKey[128];
        char    passPhrase[64];
      } creds;
} wifi_dpp_credential_object_t;

typedef ssid_t        wifi_dpp_discovery_object_t;

// DPP Configuration Object
typedef struct {
	wifi_dpp_technology_t             wifiTech;
    wifi_dpp_discovery_object_t         discovery;
    wifi_dpp_credential_object_t        credentials;
	void 	*reconfigCtx;
	void 	*cSignInstance;		
} wifi_dpp_configuration_object_t;

typedef enum {
    STATE_DPP_UNPROVISIONED,
    STATE_DPP_AUTH_RSP_PENDING,
    STATE_DPP_AUTH_FAILED,
    STATE_DPP_AUTHENTICATED,
    STATE_DPP_CFG_RSP_SENT,
    STATE_DPP_CFG_FAILED,
    STATE_DPP_CFG_ASSOC_IND_RECEIVED,
    STATE_DPP_PROVISIONED = STATE_DPP_CFG_ASSOC_IND_RECEIVED,
    STATE_DPP_RECFG_AUTH_RSP_PENDING,
    STATE_DPP_RECFG_AUTH_FAILED,
} wifi_dpp_state_t;

typedef enum {
    RESPONDER_STATUS_OK,
    RESPONDER_STATUS_NOT_COMPATIBLE,
    RESPONDER_STATUS_AUTH_FAILURE,
    RESPONDER_STATUS_BAD_CODE,
    RESPONDER_STATUS_BAD_GROUP,
    RESPONDER_STATUS_CONFIGURATION_FAILURE,
    RESPONDER_STATUS_RESPONSE_PENDING,
    RESPONDER_STATUS_INVALID_CONNECTOR,
    RESPONDER_STATUS_NO_MATCH,
    RESPONDER_STATUS_CONFIG_REJECTED,
    RESPONDER_STATUS_NOT_AVAILABLE,
} wifi_enrollee_responder_status_t;

typedef enum {
    ActStatus_Idle,
    ActStatus_Config_Error,
    ActStatus_In_Progress,
    ActStatus_No_Response,
    ActStatus_Failed,
    ActStatus_OK
} wifi_activation_status_t;

typedef struct {
    char            iPubKey[512];
    char            rPubKey[512];
} wifi_dpp_config_data_t;

typedef struct {
	unsigned char 	tran_id[120];
    char            iPubKey[512];
        unsigned char match_tran_id;
} wifi_dpp_reconfig_data_t;

typedef enum {
	wifi_dpp_session_type_config,
	wifi_dpp_session_type_reconfig,
} wifi_dpp_session_type_t;

typedef struct {
    mac_address_t  sta_mac;
	wifi_dpp_session_type_t	session;
    union {
        wifi_dpp_config_data_t  config_data;
        wifi_dpp_reconfig_data_t    reconfig_data;
    } u;
    unsigned int    channel; // current channel that DPP Authentication request will be sent on
    void            *instance;
    wifi_dpp_state_t    state;
} wifi_dpp_session_data_t;

typedef enum {
	dpp_context_type_session_data,
	dpp_context_type_received_frame_auth_rsp,
	dpp_context_type_received_frame_cfg_req,
	dpp_context_type_received_frame_cfg_result,
	dpp_context_type_received_frame_recfg_announce,
	dpp_context_type_received_frame_recfg_auth_rsp,
} wifi_device_dpp_context_type_t;

typedef enum {
    wifi_dpp_public_action_frame_type_auth_req,
    wifi_dpp_public_action_frame_type_auth_rsp,
    wifi_dpp_public_action_frame_type_auth_cnf,
    wifi_dpp_public_action_frame_type_reserved_1,
    wifi_dpp_public_action_frame_type_reserved_2,
    wifi_dpp_public_action_frame_type_peer_disc_req,
    wifi_dpp_public_action_frame_type_peer_disc_rsp,
    wifi_dpp_public_action_frame_type_pkex_req,
    wifi_dpp_public_action_frame_type_pkex_rsp,
    wifi_dpp_public_action_frame_type_pkex_rev_req,
    wifi_dpp_public_action_frame_type_pkex_rev_rsp,
    wifi_dpp_public_action_frame_type_cfg_result,
    wifi_dpp_public_action_frame_type_conn_status_result,
    wifi_dpp_public_action_frame_type_presence_announcement,
    wifi_dpp_public_action_frame_type_recfg_announcement,
    wifi_dpp_public_action_frame_type_recfg_auth_req,
    wifi_dpp_public_action_frame_type_recfg_auth_rsp,
    wifi_dpp_public_action_frame_type_recfg_auth_cnf,
} wifi_dpp_public_action_frame_type_t;

typedef struct {
	UCHAR *frame;
	UINT 	length;
	wifi_dpp_public_action_frame_type_t	frame_type;
} wifi_dpp_received_frame_t;

typedef struct {
    unsigned int ap_index;
	unsigned char configurator_version;
	unsigned char enrollee_version;
	wifi_device_dpp_context_type_t type;
	wifi_dpp_session_data_t		session_data;
	wifi_dpp_received_frame_t	received_frame;
    unsigned int    dpp_init_retries;
    unsigned int    max_retries;
    unsigned char   token;
    wifi_dpp_configuration_object_t config;
    wifi_enrollee_responder_status_t     enrollee_status;
    wifi_activation_status_t    activation_status;
    unsigned int    check_for_associated;
    unsigned int    check_for_config_requested;
	unsigned int	num_channels;	// number of channels that enrollee can listen on
	unsigned int	channels_list[32]; // list of channels that enrollee can listen on
	unsigned int	current_attempts; // number of failed attempts on N different channels off the list
} wifi_device_dpp_context_t;
/** @} */  //END OF GROUP WIFI_HAL_TYPES

/**
 * @addtogroup WIFI_HAL_APIS
 * @{
 */

/* @description Initiate device provisioning with unprovisioned DPP enabled client.
 * Causes AP to start transmitting DPP Authentication Request unicast message to client on current operating channel.
 * Sent from the AP at apIndex.  Returns an error if message transmission fails.
 *
 * @param apIndex; index of the vAP to send the request from.
 * @param staMAC, MAC address string of the peer device to send the request
 *      to.  Must be an external device MAC address.
 * @param iBootstrapSubjectPubKeyInfoB64; pointer to initiator's DER encoded ASN.1 base64 subject public key info (RFC 5280).
 * @param rBootstrapSubjectPubKeyInfoB64; pointer to responder's DER encoded ASN.1 base64 subject public key info (RFC 5280).
 * @param channel; frequency in Hz of channel on which initiator should send the DPP Authentication Request public action frame

 
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */

INT wifi_dppInitiate(wifi_device_dpp_context_t *ctx);

/* @description Cancel device provisioning if DPP Authentication Response is not receieved after number of sent requets
 * Causes AP to stop transmitting DPP Authentication Request unicast message to client on current operating channel.
 * If DPP Authentication Response has already been received from client, the provisioning sequence can not be cancelled.
 * Sent from the AP at apIndex.  Returns an error if message transmission fails.
 *
 * @param apIndex; index of the vAP to send the request from.
 * @param staMAC, MAC address string of the peer device to send the request
 *      to.  Must be an external device MAC address.
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */
INT wifi_dppCancel(wifi_device_dpp_context_t *ctx);

/* @description This call back is invoked when a STA responds to a DPP Authentication
 * Request from the gateway    with DPP Authentication Response
 *
 * @param apIndex; index of the vAP where the DPP Authentication Response frame is received
 * @param staMAC, MAC address of the peer device
 * @param status, one of wifi_dppAuthResponseStatus_t authentication response status
 *
 * @execution Synchronous
 * @sideeffect None
 *
 * @note This function must not suspend and must not invoke any blocking system
 * calls.
 */
typedef void (*wifi_dppAuthResponse_callback_t)(UINT apIndex, 
                                                mac_address_t sta,
                                                UCHAR *frame,
												UINT len);

/* @description This call back is invoked when a STA sends DPP Configuration
 * Request to the gateway
 *
 * @param apIndex; index of the vAP where the DPP Configuration Request frame is received
 * @param staMAC, MAC address string of the peer device
 * @param configAttributes, address of memory pointing to configuration attributes
 * @param length, length of memory in bytes
 *
 * @execution Synchronous
 * @sideeffect None
 *
 * @note This function must not suspend and must not invoke any blocking system
 * calls.
 */
typedef void (*wifi_dppConfigRequest_callback_t)(UINT apIndex,
                                                mac_address_t sta,
												UCHAR token,
                                                UCHAR *attribs,
                                                UINT length);

/* @description This call back is invoked when a STA sends DPP Configuration
 * Result to the gateway
 *
 * @param apIndex; index of the vAP where the DPP Configuration Request frame is received
 * @param staMAC, MAC address string of the peer device
 * @param attributes, address of memory pointing to attributes
 * @param length, length of memory in bytes
 *
 * @execution Synchronous
 * @sideeffect None
 *
 * @note This function must not suspend and must not invoke any blocking system
 * calls.
 */
typedef void (*wifi_dppConfigResult_callback_t)(UINT apIndex,
                                                mac_address_t sta,
                                                UCHAR *attribs,
                                                UINT length);

/* @description This call back is invoked when a STA sends DPP Reconfig
 * Announcement to the gateway
 *
 * @param apIndex; index of the vAP where the DPP Configuration Request frame is received
 * @param staMAC, MAC address string of the peer device
 * @param attributes, address of memory pointing to attributes
 * @param length, length of memory in bytes
 *
 * @execution Synchronous
 * @sideeffect None
 *
 * @note This function must not suspend and must not invoke any blocking system
 * calls.
 */
typedef void (*wifi_dppReconfigAnnounce_callback_t)(UINT apIndex,
                                                mac_address_t sta,
                                                UCHAR *attribs,
                                                UINT length);

/* @description This call back is invoked when a STA responds to a DPP Reconfig Authentication
 * Request from the gateway    with DPP Authentication Response
 *
 * @param apIndex; index of the vAP where the DPP Authentication Response frame is received
 * @param staMAC, MAC address of the peer device
 * @param status, one of wifi_dppAuthResponseStatus_t authentication response status
 *
 * @execution Synchronous
 * @sideeffect None
 *
 * @note This function must not suspend and must not invoke any blocking system
 * calls.
 */
typedef void (*wifi_dppReconfigAuthResponse_callback_t)(UINT apIndex, 
                                                mac_address_t sta,
                                                UCHAR *frame,
												UINT len);

typedef enum
{
    WIFI_MGMT_FRAME_TYPE_INVALID=-1,
    WIFI_MGMT_FRAME_TYPE_PROBE_REQ=0,
    WIFI_MGMT_FRAME_TYPE_PROBE_RSP=1,
    WIFI_MGMT_FRAME_TYPE_ASSOC_REQ=2,
    WIFI_MGMT_FRAME_TYPE_ASSOC_RSP=3,
    WIFI_MGMT_FRAME_TYPE_AUTH=4,
    WIFI_MGMT_FRAME_TYPE_DEAUTH=5,
    WIFI_MGMT_FRAME_TYPE_REASSOC_REQ=6,
    WIFI_MGMT_FRAME_TYPE_REASSOC_RSP=7,
    WIFI_MGMT_FRAME_TYPE_DISASSOC=8,
    WIFI_MGMT_FRAME_TYPE_ACTION=9,
} wifi_mgmtFrameType_t;

/** @} */  //END OF GROUP WIFI_HAL_TYPES

/**
 * @addtogroup WIFI_HAL_APIS
 * @{
 */

typedef enum
{
    WIFI_DATA_FRAME_TYPE_INVALID=-1,
    WIFI_DATA_FRAME_TYPE_8021x,
} wifi_dataFrameType_t;

typedef INT (* wifi_receivedDataFrame_callback)(INT apIndex, UCHAR *sta_mac, UCHAR *frame, UINT len, wifi_dataFrameType_t type);

typedef enum
{
    WIFI_FRAME_TYPE_INVALID=-1,
    WIFI_FRAME_TYPE_MGMT,
    WIFI_FRAME_TYPE_CTRL,
    WIFI_FRAME_TYPE_DATA,
} wifi_frameType_t;
  
 /*
 * @description DPP callbacks registration for AuthResponse & dppConfigRequest functions.
 *
 * @param wifi_dppAuthResponse_callback - DPP Authentication Response callback function
 * @param wifi_dppConfigRequest_callback - DPP Config Request callback function
 *
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 *
 * @execution Synchronous
 * @sideeffect None
 *
 * @note This function must not suspend and must not invoke any blocking system
 * calls.
 */
 
INT wifi_dpp_frame_received_callbacks_register(wifi_dppAuthResponse_callback_t dppAuthCallback,
                                    wifi_dppConfigRequest_callback_t dppCpnfigCallback,
									wifi_dppConfigResult_callback_t dppConfigResultCallback,
									wifi_dppReconfigAnnounce_callback_t dppReconfigAnnounceCallback,
									wifi_dppReconfigAuthResponse_callback_t dppReconfigAuthRspCallback);

/* @description send athentication confiration as per DPP specifications
 * Causes AP to transmit DPP Authentication Conf message frame to STA that will cause STA 
 * to request configuration process in case confirmation status is set to success
 * Sent from the AP at apIndex.  Returns an error if message transmission fails.
 *
 * @param apIndex; index of the vAP to send the request from.
 * @param staMAC, MAC address string of the peer device to send the request
 *      to.  Must be an external device MAC address.
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */

INT wifi_dppSendAuthCnf(wifi_device_dpp_context_t *ctx);

/* @description send device specific configuration as per DPP specifications
 * Causes AP to transmit DPP Configuration Response frame to STA with device specific 
 * configuration
 * Sent from the AP at apIndex.  Returns an error if message transmission fails.
 *
 * @param apIndex; index of the vAP to send the request from.
 * @param staMAC, MAC address string of the peer device to send the request
 *      to.  Must be an external device MAC address.
 * @param config, start address of memory pointing to configuration
 * @param length, length in bytes of memory pointing to configuration
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */

INT wifi_dppSendConfigResponse(wifi_device_dpp_context_t *ctx);

/* @description set STA specific password key
 * Configures STA specific password in AP
 *
 * @param apIndex; index of the vAP to send the request from.
 * @param staMAC, MAC address string of the peer device to send the request
 *      to.  Must be an external device MAC address.
 * @param key
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */

INT wifi_dppSetSTAPassphrase(UINT apIndex, 
                                CHAR *sta,
                                CHAR *key);

/* @description remove STA specific password key
 * Removes STA specific password in AP
 *
 * @param apIndex; index of the vAP to send the request from.
 * @param staMAC, MAC address string of the peer device to send the request
 *      to.  Must be an external device MAC address.
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */

INT wifi_dppRemoveSTAPassphrase(UINT apIndex,
                                CHAR *sta);

/* @description transmit an action frame 
 * Transmits an action frame to STA from a specific VAP
 *
 * @param apIndex; index of the vAP to send the frame from.
 * @param staMAC; MAC address string of the peer device to send the frame to
 * @param frequency; frequency of the channel on which this action frame should be sent on (for public action frames that 
 *      can be sent to a device on/off channel)
 * @param frame; pointer to the frame buffer
 * @param len; length of the buffer
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */

INT wifi_sendActionFrame(INT apIndex,
                                mac_address_t sta,
                                UINT frequency,
                                UCHAR *frame,
                                UINT len);

/* @description call back for a received management frame (probe requests and action frames) on a VAP
 * This callback is a manadatory static callback that MUST be implemented when management frames 
 * are received. If a hander for probe requests or some specific public action frames are registered, 
 * then the registered callbacks should be invoked
 * Callback for a received management frame on a VAP
 *
 * @param apIndex; index of the vAP
 * @param frame; pointer to the frame buffer, the buffer points to the data
 * @param len; length of the buffer
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */


/** @} */  //END OF GROUP WIFI_HAL_APIS

/**
 * @addtogroup WIFI_HAL_TYPES
 * @{
 */
typedef enum {
    pre_assoc_probe_block,
    pre_assoc_assoc_block,
    post_assoc_idle_80211v,
    post_assoc_idle_kick_mac,
    post_assoc_active_80211v,
    post_assoc_active_kickmac,
} wifi_steer_type_t;
typedef unsigned int     wifi_steer_matching_condition_t;

typedef struct {
    CHAR     *module;
    mac_address_t     sta_mac;
    mac_address_t     src_bss;
    mac_address_t     dst_bss;
    wifi_steer_type_t     type;
    wifi_steer_matching_condition_t     cond;
} wifi_steer_trigger_data_t;
/** @} */  //END OF GROUP WIFI_HAL_TYPES

typedef INT (* wifi_steerTriggered_callback)(INT apIndex, wifi_steer_trigger_data_t *data);
void wifi_steerTriggered_callback_register(wifi_steerTriggered_callback callback_proc, CHAR *module);

/* 802.11ax HAL API prototypes */

INT wifi_setDownlinkMuType  (INT    radio_index,
                                wifi_dl_mu_type_t   mu_type);

INT wifi_getDownlinkMuType  (INT    radio_index,
                                wifi_dl_mu_type_t   *mu_type);

INT wifi_setUplinkMuType    (INT    radio_index,
                                wifi_ul_mu_type_t   mu_type);

INT wifi_getUplinkMuType    (INT    radio_index,
                                wifi_ul_mu_type_t   *mu_type);

INT wifi_setGuardInterval   (INT radio_index,
                                wifi_guard_interval_t   guard_interval);

INT wifi_getGuardInterval   (INT radio_index,
                                wifi_guard_interval_t   *guard_interval);

INT wifi_setBSSColorEnabled (INT apIndex, BOOL enabled);

INT wifi_getBSSColorEnabled (INT apindex, BOOL *enabled);

INT wifi_getBSSColor    (INT apIndex, UCHAR *color);

INT wifi_setDownlinkDataAckType (INT radio_index,
                                wifi_dl_data_ack_type_t ack_type);

INT	wifi_getTWTParams	(CHAR *sta, wifi_twt_params_t *twt_params);

INT	wifi_get80211axDefaultParameters	(INT apIndex, wifi_80211ax_params_t	*params);

typedef enum {
    wifi_anqp_element_name_reserved_0,
    wifi_anqp_element_name_query_list = 256,
    wifi_anqp_element_name_capability_list,
    wifi_anqp_element_name_venue_name,
    wifi_anqp_element_name_emergency_call_number,
    wifi_anqp_element_name_network_auth_type,
    wifi_anqp_element_name_roaming_consortium,
    wifi_anqp_element_name_ip_address_availabality,
    wifi_anqp_element_name_nai_realm,
    wifi_anqp_element_name_3gpp_cellular_network,
    wifi_anqp_element_name_geo_location,
    wifi_anqp_element_name_civic_location,
    wifi_anqp_element_name_loc_public_id,
    wifi_anqp_element_name_domain_name,
    wifi_anqp_element_name_emergency_alert_id,
    wifi_anqp_element_name_tdls_capability,
    wifi_anqp_element_name_emergency_nai,
    wifi_anqp_element_name_neighbor_report,
    wifi_anqp_element_name_venue_url,
    wifi_anqp_element_name_advice_of_charge,
    wifi_anqp_element_name_local_content,
    wifi_anqp_element_name_network_auth_type_with_timestamp,
    wifi_anqp_element_name_reserved_1 = 273,
    wifi_anqp_element_name_vendor_specific = 56797,
    wifi_anqp_element_name_reserved_2
} wifi_anqp_element_name_t;

typedef enum {
    wifi_anqp_element_hs_subtype_reserved_0,
    wifi_anqp_element_hs_subtype_hs_query_list,
    wifi_anqp_element_hs_subtype_hs_capability_list,
    wifi_anqp_element_hs_subtype_operator_friendly_name,
    wifi_anqp_element_hs_subtype_wan_metrics,
    wifi_anqp_element_hs_subtype_conn_capability,
    wifi_anqp_element_hs_subtype_nai_home_realm_query,
    wifi_anqp_element_hs_subtype_op_class_ind,
    wifi_anqp_element_hs_subtype_osu_providers_list,
    wifi_anqp_element_hs_subtype_reserved_1,
    wifi_anqp_element_hs_subtype_icon_request,
    wifi_anqp_element_hs_subtype_icon_bin_file,
    wifi_anqp_element_hs_subtype_op_icon_metadata,
    wifi_anqp_element_hs_subtype_op_providers_nai_list,
    wifi_anqp_element_hs_subtype_reserved_2
} wifi_anqp_element_hs_subtype_t;

typedef enum {
    wifi_anqp_id_type_anqp,
    wifi_anqp_id_type_hs
} wifi_anqp_id_type_t;

typedef struct {
    wifi_anqp_id_type_t     type;
    union {
        wifi_anqp_element_name_t    anqp_elem_id;
        wifi_anqp_element_hs_subtype_t  anqp_hs_id;
    } u;
    UINT    len;
    UCHAR   *data;
} wifi_anqp_elem_t;

typedef struct wifi_anqp_node {
    struct wifi_anqp_node    *next;
    wifi_anqp_elem_t    *value;
} wifi_anqp_node_t;


typedef enum {
    wifi_ipv4_field_values_not_available,
    wifi_ipv4_field_values_available,
    wifi_ipv4_field_values_post_restricted,
    wifi_ipv4_field_values_single_nated_private,
    wifi_ipv4_field_values_double_nated_private,
    wifi_ipv4_field_values_port_restricted_single_nated,
    wifi_ipv4_field_values_port_restricted_double_nated,
    wifi_ipv4_field_values_not_known
} wifi_ipv4_field_values_t;

typedef enum {
    wifi_ipv6_field_values_not_available,
    wifi_ipv6_field_values_available,
    wifi_ipv6_field_values_not_known
} wifi_ipv6_field_values_t;

typedef struct {
    UCHAR   field_format;
}__attribute__((packed)) wifi_ipAddressAvailabality_t;

typedef enum {
    wifi_auth_id_reserved,
    wifi_auth_id_expanded_eap,
    wifi_auth_id_inner_auth_eap,
    wifi_auth_id_expanded_inner_auth_eap,
    wifi_auth_id_credential_type,
    wifi_auth_id_tunneled_eap,
}wifi_auth_id_t;

typedef struct {
    UCHAR  id;
    UCHAR   length;
    UCHAR   val[16];
}__attribute__((packed)) wifi_authMethod_t;

typedef struct {
    UCHAR   length;
    UCHAR   method;
    UCHAR   auth_param_count;
    wifi_authMethod_t   auth_method[16];
}__attribute__((packed)) wifi_eapMethod_t;

typedef struct {
    USHORT            data_field_length;
    UCHAR             encoding;
    UCHAR             realm_length;
    UCHAR             realm[256];
    UCHAR             eap_method_count;
    wifi_eapMethod_t  eap_method[16];
} __attribute__((packed))wifi_naiRealm_t;

typedef struct {
    USHORT nai_realm_count;
    wifi_naiRealm_t nai_realm_tuples[20];
}__attribute__((packed)) wifi_naiRealmElement_t;

typedef struct {
    UCHAR    length;
    UCHAR    language[3];
    UCHAR    name[256];
}__attribute__((packed)) wifi_venueName_t;

typedef struct {
    UCHAR            venueGroup;
    UCHAR            venueType;
    wifi_venueName_t venueNameTuples[16];
}__attribute__((packed)) wifi_venueNameElement_t;

typedef struct {
    UCHAR   PLMN[3];
}__attribute__((packed)) wifi_plmn_t;

typedef struct {
    UCHAR   iei;//copy zero for now.
    UCHAR   plmn_length;
    UCHAR number_of_plmns;
    wifi_plmn_t plmn[16];
}__attribute__((packed))wifi_3gpp_plmn_list_information_element_t;

typedef struct {
    UCHAR   gud;
    UCHAR   uhdLength;//Length of remaining fields
    wifi_3gpp_plmn_list_information_element_t plmn_information;
}__attribute__((packed)) wifi_3gppCellularNetwork_t;

typedef struct {
    UCHAR length;
    UCHAR domainName[255]; //max domain name allowed based on the spec.
}__attribute__((packed)) wifi_domainNameTuple_t;
  
typedef struct {
    wifi_domainNameTuple_t  domainNameTuple[4];
}__attribute__((packed)) wifi_domainName_t;

typedef struct {
    UCHAR length;
    UCHAR oui[15];
}__attribute__((packed)) wifi_ouiDuple_t;

typedef struct {
    wifi_ouiDuple_t ouiDuple[32];
}__attribute__((packed)) wifi_roamingConsortium_t;

typedef struct {
    USHORT capabilityList[64];
}__attribute__((packed)) wifi_capabilityListANQP_t;

typedef struct {
    UCHAR    wifiRoamingConsortiumCount;
    UCHAR    wifiRoamingConsortiumOui[3][15+1];//only 3 OIS is allowed in beacon and probe responses OIS length is variable between 3-15
    UCHAR    wifiRoamingConsortiumLen[3];
}__attribute__((packed)) wifi_roamingConsortiumElement_t;

INT wifi_pushApRoamingConsortiumElement(INT apIndex, wifi_roamingConsortiumElement_t *infoEelement);

//HS2 Related ANQP Elements start

//=========================================Start-HS-Operator Friendly Name=========================================================================
//HS2.0 Operator Name Duple #1     HS2.0 Operator Name Duple #2        ......    HS2.0 Operator Name Duple #n
//           variable                            variable                                      variable

//HS2.0 Operator name Duple
//Length                                  Language Code                                     Operator Name
//   1   (3+ operator name)                   3                                               variable

typedef struct _wifi_HS2_OperatorNameDuple_t // figure 9-595
{
    UCHAR length; //length is 3(language code)+number of octects in operator name field  eg. operatorName= aaaa length is 4+3 = 7
    UCHAR languageCode[3];
    UCHAR operatorName[252]; //based on spec the maximum length of operator name is 252
} __attribute__((packed)) wifi_HS2_OperatorNameDuple_t;

typedef struct
{
    wifi_HS2_OperatorNameDuple_t operatorNameDuple[16]; //putting 16 duples supported for now story RDKB-1317 does not tell how many duples we are supporting nor the spec (spec mentions n duples)
} __attribute__((packed)) wifi_HS2_OperatorFriendlyName_t;

//=========================================End-HS2-Operator Friendly Name=========================================================================

//=========================================Start-HS2-Wan Metrics Element=========================================================================
//WAN Info                 Downlink Speed           Uplink Speed     Downlink Load       Uplink Load        LMD
//   1                            4                       4                  1                1               2
typedef struct // figure 9-595
{
    UCHAR wanInfo;
    UINT downLinkSpeed;
    UINT upLinkSpeed;
    UCHAR downLinkLoad;
    UCHAR upLinkLoad;
    USHORT lmd;
} __attribute__((packed)) wifi_HS2_WANMetrics_t;

//WAN Info Bit fields
//        B0     B1                  B2                      B3                 B4    B7
//        Link Status              Symetric Link          At Capacity          Reserved
//Bits:        2                        1                    1                     4
typedef enum
{
    wifi_hs2_wan_info_reserved,
    wifi_hs2_wan_info_linkup,
    wifi_hs2_wan_info_linkdown,
    wifi_hs2_wan_info_link_in_test_state
} wifi_HS2_Wan_Info_Link_Status_t;

//=========================================End-HS2-Wan Metrics Element=========================================================================

//=========================================Start-HS2-Connection Capability Element=========================================================================

//Proto Port Tuple #1        Proto Port Tuple #2  ............. Proto Port Tuple #n
//         4                        4(optional)                   4(optional)

//Proto Port Tuple Format
//IP Protocol                             Port Number                  Status
//     1                                       2                         1

typedef struct // figure 9-595
{
    UCHAR ipProtocol;
    USHORT portNumber;
    UCHAR status;
} __attribute__((packed)) wifi_HS2_Proto_Port_Tuple_t;

typedef struct // figure 9-595
{
    wifi_HS2_Proto_Port_Tuple_t protoPortTuple[16];//putting 16 duples supported for now. story RDKB-1317 does not tell how many duples we are supporting nor the spec (spec mentions n duples)
} __attribute__((packed)) wifi_HS2_ConnectionCapability_t;

typedef enum
{
    wifi_hs2_connection_capability_closed,
    wifi_hs2_connection_capability_open,
    wifi_hs2_connection_capability_unknown,
    wifi_hs2_connection_capability_reserved
} wifi_HS2_ConnectionCapability_Status_t;

    //=========================================End-HS2-Connection Capability Element=========================================================================

    //=========================================Start-HS2-NAI Realm Query Element=========================================================================

    //NAI Realm Count                          NAI Home Realm                    NAI Home Realm   ....  NAI Home Realm
    //                                         Name Data #1                        Name Data #2            Name Data #n
    //      1                                   variable                         (variable optional)       (variable optional)

    //NAI Realm Encoding                                 NAI Home Realm Name Length              NAI Home Realm
    //         1                                                      1                               variable
typedef struct// figure 9-595
{
    UCHAR encoding;
    UCHAR length;
    UCHAR name[255];//per spec maximum length is 255
} __attribute__((packed)) wifi_HS2_NAI_Home_Realm_Data_t;

typedef struct// figure 9-595
{
    UCHAR realmCount;
    wifi_HS2_NAI_Home_Realm_Data_t homeRealmData[20];//as realm count is unsigned char we can put 255 realms here spec says n story does not define how many we support
} __attribute__((packed)) wifi_HS2_NAI_Home_Realm_Query_t;

//=========================================End-HS2-NAI Realm Query Element=========================================================================

//=========================================Start-HS-Capability List=========================================================================
//HS2.0 Capability #1     HS2.0 Capability #2        ......    HS2.0 Capability #n
//            1             0 or 1 (optional)                      0 or 1 (optional)
//=========================================End-HS-Capability List=========================================================================
typedef struct
{
    UCHAR capabilityList[64];
} __attribute__((packed)) wifi_HS2_CapabilityList_t;

typedef void (* wifi_anqp_request_callback_t)   (UINT apIndex, mac_address_t sta, unsigned char token,  wifi_anqp_node_t *list);
INT wifi_anqp_request_callback_register(wifi_anqp_request_callback_t callback);

INT wifi_anqpSendResponse(UINT apIndex, mac_address_t sta, unsigned char token, wifi_anqp_node_t *list);

//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_InterworkingService.DGAFEnable	
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_InterworkingService.ANQPDomainID
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_InterworkingService.VenueNamesNumberOfEntries	
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_InterworkingService.OperatorNamesNumberOfEntries	
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_InterworkingService.ConsortiumOIsNumberOfEntries	
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_InterworkingService.DomainNamesNumberOfEntries	
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_InterworkingService.3GPPNetworksNumberOfEntries	
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_InterworkingService.NAIRealmsNumberOfEntries	

//-----------------------------------------------------------------------------------------------
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.VenueNames.{i}.	
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.VenueNames.{i}.LanguageCode
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.VenueNames.{i}.VanueName

//-----------------------------------------------------------------------------------------------
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.OperatorNames.{i}.	
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.OperatorNames.{i}.LanguageCode
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.OperatorNames.{i}.OperatorName

//-----------------------------------------------------------------------------------------------
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.ConsortiumOIs.{i}.	
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.ConsortiumOIs.{i}.OI	

//-----------------------------------------------------------------------------------------------
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.DomainNames.{i}.	
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.DomainNames.{i}.DomainName	

//-----------------------------------------------------------------------------------------------
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.3GPPNetworks.{i}.	
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.3GPPNetworks.{i}.MCC	
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.3GPPNetworks.{i}.MNC	

//-----------------------------------------------------------------------------------------------
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.NAIRealms.{i}.	
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.NAIRealms.{i}.NAIRealmEncodingType	
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.NAIRealms.{i}.NAIRealm	
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.NAIRealms.{i}.EAPMethodsNumberOfEntries	

//-----------------------------------------------------------------------------------------------
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.NAIRealms.{i}.EAPMethods.{i}.	
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.NAIRealms.{i}.EAPMethods.{i}.EAPMethod	
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.NAIRealms.{i}.EAPMethods.{i}.AuthenticationParametersNumberOfEntries	

//-----------------------------------------------------------------------------------------------
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.NAIRealms.{i}.EAPMethods.{i}.AuthenticationParameters.{i}.	
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.NAIRealms.{i}.EAPMethods.{i}.AuthenticationParameters.{i}.ID	
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.NAIRealms.{i}.EAPMethods.{i}.AuthenticationParameters.{i}.Value	

//-----------------------------------------------------------------------------------------------
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.WANMetrics.	
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.WANMetrics.LinkStatus	
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.WANMetrics.AtCapacity	
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.WANMetrics.DownlinkSpeed	
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.WANMetrics.UplinkSpeed	
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.WANMetrics.DownlinkLoad	
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.WANMetrics.UplinkLoad	

//-----------------------------------------------------------------------------------------------
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.OSU.	
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.OSU.OSUProvidersNumberOfEntries	

//-----------------------------------------------------------------------------------------------
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.OSU.OSUProviders.{i}.	
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.OSU.OSUProviders.{i}.OSUServerURI	
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.OSU.OSUProviders.{i}.OSUMethodsList	
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.OSU.OSUProviders.{i}.OSUNAI	
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.OSU.OSUProviders.{i}.NamesNumberOfEntries	
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.OSU.OSUProviders.{i}.IconsNumberOfEntries	
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.OSU.OSUProviders.{i}ServiceDescriptionsNumberOfEntries	

//-----------------------------------------------------------------------------------------------
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.OSU.OSUProviders.{i}.Names.{i}.	
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.OSU.OSUProviders.{i}.Names.{i}.LanguageCode	
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.OSU.OSUProviders.{i}.Names.{i}.OSUProviderFriendlyName	

//-----------------------------------------------------------------------------------------------
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.OSU.OSUProviders.{i}.Icons.{i}.	
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.OSU.OSUProviders.{i}.Icons.{i}.IconWidth	
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.OSU.OSUProviders.{i}.Icons.{i}.IconHeight	
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.OSU.OSUProviders.{i}.Icons.{i}.LanguageCode	

//-----------------------------------------------------------------------------------------------
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.OSU.OSUProviders.{i}.ServiceDescriptions.{i}.	
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.OSU.OSUProviders.{i}.ServiceDescriptions.{i}.LanguageCode	
//Device.WiFi.AccessPoint.{i}.X_COMCAST-COM_Passpoint.OSU.OSUProviders.{i}.ServiceDescriptions.{i}.ServiceDescription	

//-----------------------------------------------------------------------------------------------
//Device.IP.Diagnostics.	
//Device.IP.Diagnostics.IPPing.	
//Device.IP.Diagnostics.IPPing.DiagnosticsState	
//Device.IP.Diagnostics.IPPing.Interface
//Device.IP.Diagnostics.IPPing.Host	
//Device.IP.Diagnostics.IPPing.NumberOfRepetitions		
//Device.IP.Diagnostics.IPPing.Timeout	
//Device.IP.Diagnostics.IPPing.DataBlockSize	
//Device.IP.Diagnostics.IPPing.DSCP			

//Device.IP.Diagnostics.IPPing.SuccessCount	
//Device.IP.Diagnostics.IPPing.FailureCount		
//Device.IP.Diagnostics.IPPing.AverageResponseTime		
//Device.IP.Diagnostics.IPPing.MinimumResponseTime		
//Device.IP.Diagnostics.IPPing.MaximumResponseTime			

//Start the ping test and get the result
//INT wifi_getIPDiagnosticsIPPingResult(wifi_diag_ipping_setting_t *input, wifi_diag_ipping_result_t *result); //Tr181		
//--------------------------------------------------------------------------------------------------
// Wifi Airtime Management and QOS APIs to control contention based access to airtime
//INT wifi_clearDownLinkQos(INT apIndex);                             // clears the QOS parameters to the WMM default values for the downlink direction (from the access point to the stations.  This set must take affect when the api wifi_applySSIDSettings() is called.
//INT wifi_setDownLinkQos(INT apIndex, wifi_qos_t qosStruct);        // sets the QOS variables used in the downlink direction (from the access point to the stations).  Values must be allowable values per IEEE 802.11-2012 section 8.4.2.31.  Note:  Some implementations may requrie that all downlink APs on the same radio are set to the same QOS values. Default values are per the WMM spec.  This set must take affect when the api wifi_applySSIDSettings() is called.
//INT wifi_clearUpLinkQos(INT apIndex);                               // clears the QOS parameters to the WMM default values for the uplink direction (from the Wifi stations to the ap.  This must take affect when the api wifi_applySSIDSettings() is called.
//INT wifi_setUpLinkQos (INT apIndex, wifi_qos_t qosStruct);         // sets the QOS variables used in the uplink direction (from the Wifi stations to the AP). Values must be allowable values per IEEE 802.11-2012 section 8.4.2.31. The default values must be per the WMM spec.  This set must take affect when the api wifi_applySSIDSettings() is called.

//--------------------------------------------------------------------------------------------------
// Wifi Airtime Management and QOS APIs to control downlink queue prioritization
//INT wifi_getDownLinkQueuePrioritySupport (INT apIndex, INT *supportedPriorityLevels);  //This api is used to get the the number of supported downlink queuing priority levels for each AP/SSID.  If priority queuing levels for AP/SSIDs are not supported, the output should be set to 1. A value of 1 indicates that only the same priority level is supported for all AP/SSIDs.
//INT wifi_setDownLinkQueuePriority(INT apIndex, INT priorityLevel); // this sets the queue priority level for each AP/SSID in the downlink direction.  It is used with the downlink QOS api to manage priority access to airtime in the downlink direction.  This set must take affect when the api wifi_applySSIDSettings() is called.

//<< ------------------------------ wifi_ap_hal -----------------------
#ifdef __cplusplus
}
#endif

typedef enum {
	WIFI_EVENT_CHANNELS_CHANGED,
	WIFI_EVENT_DFS_RADAR_DETECTED
} wifi_chan_eventType_t;

typedef void (*wifi_chan_eventCB_t)(UINT radioIndex, wifi_chan_eventType_t event, UCHAR channel);

typedef enum {
	CHAN_STATE_AVAILABLE = 1,
	CHAN_STATE_DFS_NOP_FINISHED,
	CHAN_STATE_DFS_NOP_START,
	CHAN_STATE_DFS_CAC_START,
	CHAN_STATE_DFS_CAC_COMPLETED
} wifi_channelState_t;

typedef struct _wifi_channelMap_t {
	INT ch_number;
	wifi_channelState_t ch_state;
} wifi_channelMap_t;

/* wifi_getRadioChannels() function */
/*
 * Description: This function returns a map of current states of each possible channel for given radio.
 * The implementation must fill the map for each channel reported by wifi_getRadioPossibleChannels().
 *
 * Parameters :
 * radioIndex - The index of the radio. First radio is index 0. 2nd radio is index 1 - type INT
 * output_map - a pointer to an array of wifi_channelMap_t structures, preallocated by the caller.
 *              This is where the output is written. If the item in the array is not used,
 *              the "ch_number" should be set to 0.
 * output_map_size - The size of the output_map array in array items.
 *
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected
 *
 * @execution Synchronous.
 * @sideeffect None.
 *
 * @note This function must not suspend and must not invoke any blocking system
 * calls. It should probably just send a message to a driver event handler task.
 */
INT wifi_getRadioChannels(INT radioIndex, wifi_channelMap_t *output_map, INT output_map_size);

INT wifi_chan_eventRegister(wifi_chan_eventCB_t event_cb);

/* GAS Configuration */ 
#define GAS_CFG_TYPE_SUPPORTED 1

typedef struct _wifi_GASConfiguration_t{   // Values correspond to the dot11GASAdvertisementEntry field definitions; see 802.11-2016 Annex C.3.
    UINT AdvertisementID;
    BOOL PauseForServerResponse;
    UINT ResponseTimeout;
    UINT ComeBackDelay;
    UINT ResponseBufferingTime;
    UINT QueryResponseLengthLimit;
}__attribute__((packed))wifi_GASConfiguration_t;

INT wifi_applyGASConfiguration(wifi_GASConfiguration_t *input_struct);

/* Passpoint R1 related APIs */

/***********************************************************************************
 * Description : Set Country code information element in Beacon and Probe Response
 * Parameters: AP Index, Enabled Status
 * Return: The status of the operation
 **********************************************************************************/
INT wifi_setCountryIe(INT apIndex, BOOL enabled);

/***********************************************************************************
 * Description : Get status of country code information element in Beacon 
                 and Probe Response
 * Parameters: AP Index, pointer to Enabled Status variable
 * Return: The status of the operation
 **********************************************************************************/
INT wifi_getCountryIe(INT apIndex, BOOL *enabled);

/***********************************************************************************
 * Description : Enable Traffic Inspection and Filtering 
 * Parameters: AP Index, Enabled Status
 * Return: The status of the operation
 **********************************************************************************/
INT wifi_setLayer2TrafficInspectionFiltering(INT apIndex, BOOL enabled);

/***********************************************************************************
 * Description : Get Traffic Inspection and Filtering status
 * Parameters: AP Index, pointer to Enabled Status variable
 * Return: The status of the operation
 **********************************************************************************/
INT wifi_getLayer2TrafficInspectionFiltering(INT apIndex, BOOL *enabled);

/***********************************************************************************
 * Description : Disable DGAF. When set to true, DGAF disabled bit should be set 
                 in HS2.0 Indication Information Element in Beacon and Probe
 * Parameters: AP Index, Disabled Status
 * Return: The status of the operation
 **********************************************************************************/
INT wifi_setDownStreamGroupAddress(INT apIndex, BOOL disabled);

/***********************************************************************************
 * Description : Status of Disable DGAF
 * Parameters: AP Index, pointer to Disabled Status variable
 * Return: The status of the operation
 **********************************************************************************/
INT wifi_getDownStreamGroupAddress(INT apIndex, BOOL *disabled);

/***********************************************************************************
 * Description : Enable BSS Load Information Element in Beacon/Probe Response
 * Parameters: AP Index, Enabled Status
 * Return: The status of the operation
 **********************************************************************************/
INT wifi_setBssLoad(INT apIndex, BOOL enabled);

/***********************************************************************************
 * Description : Get Status of BSS Load Information Element in Beacon/Probe Response
 * Parameters: AP Index, pointer to Enabled Status variable
 * Return: The status of the operation
 **********************************************************************************/
INT wifi_getBssLoad(INT apIndex, BOOL *enabled);

/***********************************************************************************
 * Description : Enable Proxy Arp function on device Driver
 * Parameters: AP Index, Enabled Status
 * Return: The status of the operation
 **********************************************************************************/
INT wifi_setProxyArp(INT apIndex, BOOL enabled);

/***********************************************************************************
 * Description : Get Status of Proxy Arp from Driver
 * Parameters: AP Index, pointer to Enabled Status variable 
 * Return: The status of the operation
 **********************************************************************************/
INT wifi_getProxyArp(INT apIndex, BOOL *enable);

/***********************************************************************************
 * Description : Set Hotspot 2.0 Status for the Access Point
 * Parameters: AP Index, HS20 Enabled status
 * Return: The status of the operation
 **********************************************************************************/
INT wifi_pushApHotspotElement(INT apIndex, BOOL enabled);

/***********************************************************************************
 * Description : Enable Traffic Inspection and Filtering 
 * Parameters: AP Index, pointer to Enabled Status variable
 * Return: The status of the operation
 **********************************************************************************/
INT wifi_getApHotspotElement(INT apIndex, BOOL *enabled);

/***********************************************************************************
 * Description : Push Roaming Consortium Information Element Contents to HAL 
 * Parameters: AP Index, pointer to wifi_roamingConsortiumElement_t
               the structure contains OI count, length of first 3 OIs,
               and first 3 OI as a hex string. When count > 0 and interworking is 
               enabled, Roaming Consortium Information Element should be present
               in Beacon and Probe Response with this information.
 * Return: The status of the operation
 **********************************************************************************/
INT wifi_pushApRoamingConsortiumElement(INT apIndex, wifi_roamingConsortiumElement_t *infoElement);

/***********************************************************************************
 * Description : Get Roaming Consortium Information Element Contents
 * Parameters: AP Index, pointer to wifi_roamingConsortiumElement_t
               If Roaming Consortium is not present, return count as 0,
               and length and OI fileds can be ignored
 * Return: The status of the operation
 **********************************************************************************/
INT wifi_getApRoamingConsortiumElement(INT apIndex, wifi_roamingConsortiumElement_t *infoElement);

/***********************************************************************************
 * Description : Disable P2P Cross Connect
                 When Set to True, Include P2P Information element in Beacon and Probe Response
                 Include P2P Manageability attribute with the Cross Connection Permitted field value 0
 * Parameters: AP Index, Disabled Status
 * Return: The status of the operation
 **********************************************************************************/
INT wifi_setP2PCrossConnect(INT apIndex, BOOL disabled);

/***********************************************************************************
 * Description : Get Disable P2P Cross Connect status
 * Parameters: AP Index, pointer to Disabled Status variable
 * Return: The status of the operation
 **********************************************************************************/
INT wifi_getP2PCrossConnect(INT apIndex, BOOL *disabled);

/* End Passpoint related APIs */

/**
* @brief Check if Zero DFS is supported
*
* Zero DFS (also known as Background CAC) allows Wi-Fi stack to continue
* operation on main channel and at the same time run background CAC.
*
* @param[in]  radioIndex Index of Wi-Fi radio
* @param[out] supported  True if ZeroDFS is supported, false otherwise.
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
*/
INT wifi_isZeroDFSSupported(UINT radioIndex, BOOL *supported);

/**
* @brief Set Zero DFS State
*
* The Zero DFS feature can be enabled or disabled. For EU countries
* the "Pre-CAC" can be also set. If the "Pre-CAC" is set, then
* after passing background CAC driver can start background CAC
* on the next channel.
*
* @param[in]  radioIndex Index of Wi-Fi radio
* @param[in]  enabled    True if ZeroDFS must be enabled, false otherwise.
* @param[in]  precac     Valid only for EU regulatory domain. If set,
*                        after passing requested backgronund CAN, driver
*                        can start background CAC on the next channel.
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
*/
INT wifi_setZeroDFSState(UINT radioIndex, BOOL enable, BOOL precac);

/**
* @brief Get Zero DFS State
*
* The Zero DFS feature can be enabled or disabled. For EU countries
* the "Pre-CAC" can be also set. If the "Pre-CAC" is set, then
* after passing background CAC driver can start background CAC
* on next channel.
*
* @param[in]   radioIndex Index of Wi-Fi radio
* @param[out]  enabled    True if ZeroDFS is enabled, false otherwise.
* @param[out]  precac     Valid only for EU regulatory domain. If true,
*                         then driver can start background CAC on
*                         the next channel.
*
* @return The status of the operation
* @retval RETURN_OK if successful
* @retval RETURN_ERR if any error is detected
*
* @execution Synchronous
* @sideeffect None
*
*/
INT wifi_getZeroDFSState(UINT radioIndex, BOOL *enable, BOOL *precac);

#define KI1_VER_MASK            0xf8

#define KI1_PW_KEY              0x08
#define KI1_INSTALL             0x40
#define KI1_ACK                 0x80

#define KI1_MSG1_BITS   (KI1_PW_KEY | KI1_ACK)
#define KI1_MSG3_BITS   (KI1_PW_KEY | KI1_INSTALL|KI1_ACK)

#define KI0_MIC                 0x01
#define KI0_SECURE              0x02
#define KI0_ENCR                0x10

#define KI0_MSG3_BITS   (KI0_MIC | KI0_SECURE | KI0_ENCR)
#define KI0_MSG4_BITS   (KI0_MIC | KI0_SECURE)

#define KEY_MSG_1_OF_4(msg)             \
        ((((msg)->key_info[1] & KI1_VER_MASK) == KI1_MSG1_BITS) && ((msg)->key_info[0] == 0))

#define KEY_MSG_2_OF_4(msg)             \
        ((((msg)->key_info[1] & KI1_VER_MASK) == KI1_PW_KEY) && ((msg)->key_info[0] == KI0_MIC))

#define KEY_MSG_3_OF_4(msg)             \
        ((((msg)->key_info[1] & KI1_VER_MASK) == KI1_MSG3_BITS) && ((msg)->key_info[0] == KI0_MSG3_BITS))

#define KEY_MSG_4_OF_4(msg)             \
        ((((msg)->key_info[1] & KI1_VER_MASK) == KI1_PW_KEY) && ((msg)->key_info[0] == KI0_MSG4_BITS))


typedef struct {
    unsigned char   descriptor;
    unsigned char   key_info[2];
    unsigned short  key_len;
    unsigned char   replay[8];
    unsigned char   nonce[32];
    unsigned char   init_vector[16];
    unsigned char   rsc[8];
    unsigned char   key_id[8];
    unsigned char   mic[16];
    unsigned short  len;
    unsigned char   data[0];
} wifi_eapol_key_frame_t;

typedef enum {
    wifi_eap_code_request = 1,
    wifi_eap_code_response,
    wifi_eap_code_success,
    wifi_eap_code_failure,
} wifi_eap_code_t;

typedef struct {
    unsigned char   code;
    unsigned char   id;
    unsigned short  len;
    unsigned char   data[0];
} __attribute__((__packed__)) wifi_eap_frame_t;

typedef enum {
    wifi_eapol_type_eap_packet,
    wifi_eapol_type_eapol_start,
    wifi_eapol_type_eapol_logoff,
    wifi_eapol_type_eapol_key,
} wifi_eapol_type_t;

typedef struct {
    unsigned char   version;
    unsigned char   type;
    unsigned short  len;
    unsigned char   data[0];
} __attribute__((__packed__)) wifi_8021x_frame_t;

typedef enum {
    wifi_direction_unknown,
    wifi_direction_uplink,
    wifi_direction_downlink
} wifi_direction_t;

typedef void (* wifi_received8021xFrame_callback)(unsigned int ap_index, mac_address_t sta, wifi_eapol_type_t type, void *data, unsigned int len);
typedef void (* wifi_sent8021xFrame_callback)(unsigned int ap_index, mac_address_t sta, wifi_eapol_type_t type, void *data, unsigned int len);

typedef void (* wifi_receivedAuthFrame_callback)(unsigned int ap_index, mac_address_t sta, void *data, unsigned int len);
typedef void (* wifi_sentAuthFrame_callback)(unsigned int ap_index, mac_address_t sta, void *data, unsigned int len);

typedef void (* wifi_receivedAssocReqFrame_callback)(unsigned int ap_index, mac_address_t sta, void *data, unsigned int len);
typedef void (* wifi_sentAssocRspFrame_callback)(unsigned int ap_index, mac_address_t sta, void *data, unsigned int len);

typedef INT (* wifi_receivedMgmtFrame_callback)(INT apIndex, UCHAR *sta_mac, UCHAR *frame, UINT len, wifi_mgmtFrameType_t type, wifi_direction_t dir);

int mgmt_frame_received_callback(INT ap_index, mac_address_t sta_mac, UCHAR *frame, UINT len, wifi_mgmtFrameType_t type, wifi_direction_t dir);
INT wifi_mgmt_frame_callbacks_register(wifi_receivedMgmtFrame_callback dppRecvRxCallback);

/* wifi_enableCSIEngine() function */
/*
 * Description: This function enables or disables CSI engine data for a specific STA on a VAP
 * If the MAC address is NULL mac address, enable argument MUST be false, otherwise function MUST return failure.
 * If the MAC address is NULL mac address, data engine for all STA(s) need to be disabled on this VAP
 *
 * Parameters :
 * apIndex - Index of VAP
 * sta - MAC address of the station associated in this VAP for which engine is being enabled/disabled
 * enable - Enable or diable
 *
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected
 *
 * @execution Synchronous.
 * @sideeffect None.
 *
 */

INT wifi_enableCSIEngine(INT apIndex,
                         mac_address_t sta,
                         BOOL enable);

/* wifi_sendDataFrame() function */
/*
 * Description: This function sends data frame to a client associated in the specified VAP index
 * If the MAC address is NULL mac address, the data should be broadcast on the VAP
 *
 * Parameters :
 * apIndex - Index of VAP
 * sta - MAC address of the station associated in this VAP
 * data - Pointer to the data buffer. The data does not have any layer 2 information but has starts with layer 3.
 * len - length of data
 * insert_llc - whether LLC header should be inserted. If set to TRUE, HAL implementation MUST insert the following bytes before type field. DSAP = 0xaa, SSAP = 0xaa, Control = 0x03, followed by 3 bytes each = 0x00
 * eth_proto - ethernet protocol, the definitions are listed below.a
 * prio - priority of the frame with which scheduler should transmit the frame, please see the enumeration below.
 *
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected
 *
 * @execution Synchronous.
 * @sideeffect None.
 *
 */

#define WIFI_ETH_TYPE_IP 0x0800
#define WIFI_ETH_TYPE_ARP 0x0806
#define WIFI_ETH_TYPE_REVARP 0x8035
#define WIFI_ETH_TYPE_VLAN 0x8100
#define WIFI_ETH_TYPE_LOOPBACK 0x9000
#define WIFI_ETH_TYPE_IP6 0x86DD
#define WIFI_ETH_TYPE_EAPOL 0x888e

typedef enum {
    wifi_data_priority_be,
    wifi_data_priority_bk,
    wifi_data_priority_ee,
    wifi_data_priority_ca,
    wifi_data_priority_vi,
    wifi_data_priority_vo,
    wifi_data_prioirty_ic,
    wifi_data_priority_nc
} wifi_data_priority_t;

INT wifi_sendDataFrame(INT apIndex,
                       mac_address_t sta,
                       UCHAR	*data,
                       UINT	len,
                       BOOL 	insert_llc,
                       UINT	eth_proto,
                       wifi_data_priority_t prio);

/**
 * _wifi_key_multi_psk structure is a container for multi psk keys.
 * It is based on hostapd wpa_psk_file implementation
 * https://w1.fi/cgit/hostap/tree/hostapd/hostapd.wpa_psk
 */
typedef struct _wifi_key_multi_psk {
	    CHAR wifi_keyId[64];   //<! Key identifier. for example "key-15" or "key_example"
	    CHAR wifi_psk[64];     //<! PSK is an ASCII passphrase of 8..63 characters
} wifi_key_multi_psk_t;

/* wifi_pushMultiPskKeys() function */
/**
  * @description Function sets the new set of multi psk keys.
  * Previously set keys are removed.
  * This API is for setting all keys except the primary key.
  * Primary key is set by wifi_setApSecurityKeyPassphrase.
  *
  * @param[in] apIndex - access point index
  * @param[in] keys - Array with all keys that client devices can associate with AP
  * @param[in] keysNumber - Number of elements in 'keys' array
  *
  * @return The status of the operation
  * @retval RETURN_OK if successful
  * @retval RETURN_ERR if any error is detected
  *
  * @sideeffect None
  */
INT wifi_pushMultiPskKeys(INT apIndex, wifi_key_multi_psk_t *keys, INT keysNumber);

/* wifi_getMultiPskKeys() function */
/**
  * @description Function gets all keys from multi psk config.
  * Implementation has to fill all active keys into wifi_key_multi_psk_t array.
  * The keys should be consecutively written starting at [0] element of the keys array.
  * This API is for getting all keys the except primary key.
  * Primary key can be get by wifi_getApSecurityKeyPassphrase.
  *
  * @param[in] apIndex - access point index
  * @param[out] keys - An array to which multi-psk keys active on the AP should be written to. If there are less present keys than 'keysNumber' the unused array elements should contain empty strings (""). Array is allocated by the caller.
  * @param[in] keysNumber - Number of elements in 'keys' array
  *
  * @return The status of the operation
  * @retval RETURN_OK if successful
  * @retval RETURN_ERR if any error is detected
  *
  * @sideeffect None
  */
INT wifi_getMultiPskKeys(INT apIndex, wifi_key_multi_psk_t *keys, INT keysNumber);

/* wifi_getMultiPskClientKey() function */
/**
  * @description Function gets multi psk key for the client mac address.
  * If wifi_key_multi_psk_t.wifi_keyID is null this means that
  * multi psk key was not used for authentication.
  *
  * @param[in] apIndex - access point index
  * @param[in] mac - client mac address
  * @param[out] key - multi psk key structure
  *
  * @return The status of the operation
  * @retval RETURN_OK if successful
  * @retval RETURN_ERR if any error is detected
  *
  * @sideeffect None
  */
INT wifi_getMultiPskClientKey(INT apIndex, mac_address_t mac, wifi_key_multi_psk_t *key);

typedef struct {
    wifi_GASConfiguration_t gas_config;
}__attribute__((packed))wifi_config_t;

typedef UINT wifi_vap_index_t;

typedef char    wifi_vap_name_t[64];

typedef enum {
    wifi_mac_filter_mode_black_list,
    wifi_mac_filter_mode_white_list,
} wifi_mac_filter_mode_t;

typedef struct {
    unsigned char	ip[45];
    unsigned short 	port;
    char	key[64];
    unsigned char	s_ip[45];
    unsigned short 	s_port;
    char	s_key[64];
} wifi_radius_settings_t;

typedef enum {
    wifi_security_key_type_psk,
    wifi_security_key_type_pass,
} wifi_security_key_type_t;

typedef struct {
    wifi_security_key_type_t type;
    char 	key[256];
} wifi_security_key_t;

typedef enum {
    wifi_encryption_tkip = 1,
    wifi_encryption_aes,
    wifi_encryption_aes_tkip,
} wifi_encryption_method_t;

typedef enum {
    wifi_security_mode_none = 0x00000001,
    wifi_security_mode_wep_64 = 0x00000002,
    wifi_security_mode_wep_128 = 0x00000004,
    wifi_security_mode_wpa_personal = 0x00000008,
    wifi_security_mode_wpa2_personal = 0x00000010,
    wifi_security_mode_wpa_wpa2_personal = 0x00000020,
    wifi_security_mode_wpa_enterprise = 0x00000040,
    wifi_security_mode_wpa2_enterprise = 0x00000080,
    wifi_security_mode_wpa_wpa2_enterprise = 0x00000100
} wifi_security_modes_t;

typedef struct {
    wifi_security_modes_t mode;
    wifi_encryption_method_t	encr;
    char	mfpConfig[32];
    union {
        wifi_radius_settings_t	radius;
        wifi_security_key_t	key;
    } u;
} __attribute__((packed)) wifi_vap_security_t;

typedef struct {
    int capabilityInfoLength;
    wifi_capabilityListANQP_t capabilityInfo;
    int venueInfoLength;
    wifi_venueNameElement_t venueInfo;
    int roamInfoLength;
    wifi_roamingConsortium_t roamInfo;
    wifi_ipAddressAvailabality_t ipAddressInfo;
    int realmInfoLength;
    wifi_naiRealmElement_t realmInfo;
    int gppInfoLength;
    wifi_3gppCellularNetwork_t gppInfo;
    int domainInfoLength;
    wifi_domainName_t domainNameInfo;
    UCHAR  passpointStats[1024];
    UINT   domainRespCount;
    UINT   realmRespCount;
    UINT   gppRespCount;
    UINT   domainFailedCount;
    UINT   realmFailedCount;
    UINT   gppFailedCount;
    UCHAR  anqpParameters[4096];
} __attribute__((packed)) wifi_anqp_settings_t;

typedef struct {
    BOOL        enable;
    BOOL        gafDisable;
    BOOL        p2pDisable;
    BOOL        l2tif;
    BOOL        bssLoad;
    BOOL        countryIE;
    BOOL        proxyArp;
    int capabilityInfoLength;
    wifi_HS2_CapabilityList_t capabilityInfo;
    int opFriendlyNameInfoLength;
    wifi_HS2_OperatorFriendlyName_t opFriendlyNameInfo;
    int connCapabilityLength;
    wifi_HS2_ConnectionCapability_t connCapabilityInfo;
    int realmInfoLength;
    wifi_HS2_NAI_Home_Realm_Query_t realmInfo;
    wifi_HS2_WANMetrics_t wanMetricsInfo;
    UCHAR hs2Parameters[4096];
} __attribute__((packed)) wifi_passpoint_settings_t;

typedef struct {
    wifi_InterworkingElement_t   interworking;
    wifi_roamingConsortiumElement_t roamingConsortium;
    wifi_anqp_settings_t        anqp;
    wifi_passpoint_settings_t	passpoint;
} __attribute__((packed)) wifi_interworking_t;

typedef struct {
    
} __attribute__((packed)) wifi_back_haul_sta_t;

typedef struct {
    char	ssid[32];
    BOOL	enabled;
    BOOL	showSsid;
    BOOL 	isolation;
    UINT	mgmtPowerControl;
    UINT	bssMaxSta;
    BOOL 	bssTransitionActivated;
    BOOL 	nbrReportActivated;
    BOOL 	rapidReconnectEnable;
    UINT	rapidReconnThreshold;
    BOOL 	vapStatsEnable;
    wifi_vap_security_t	security;
    wifi_interworking_t	interworking;
    BOOL	mac_filter_enable;
    wifi_mac_filter_mode_t mac_filter_mode;
    BOOL        sec_changed;
} wifi_front_haul_bss_t;

typedef struct {
    wifi_vap_index_t 	vap_index;
    wifi_vap_name_t     vap_name;
    union {
        wifi_front_haul_bss_t	bss_info;
        wifi_back_haul_sta_t	sta_info;
    } u;
} __attribute__((packed)) wifi_vap_info_t;

typedef struct {
    unsigned int        num_vaps;
    wifi_vap_info_t vap_array[8];
} __attribute__((packed)) wifi_vap_info_map_t;


/* DRAFT FOR CLIENT API */


/* APPROACH 1*/
typedef enum wifi_sta_network_flags {
	WIFI_STA_NET_F_OPEN,
	WIFI_STA_NET_F_PSK1,
	WIFI_STA_NET_F_PSK2,WIFI_STA_NET_F_PSK_FT,
	WIFI_STA_NET_F_AES,
	WIFI_STA_NET_F_TKIP,
	WIFI_STA_NET_F_4ADDR_MULTI_AP, /* future: easymesh's wds */
	WIFI_STA_NET_F_OWE, /* future: opportunistic wireless encryption */
	WIFI_STA_NET_F_SAE, /* future: wpa3 */
} wifi_sta_network_flags_t;
typedef enum wifi_sta_multi_ap_flag {
	WIFI_STA_MULTI_AP_NOT_SUPPORTED,
        WIFI_STA_MULTI_AP_NONE,
        WIFI_STA_MULTI_AP_BHAUL_STA,
} wifi_sta_multi_ap_flag_t;


typedef struct wifi_sta_network {
        int id;
	char bridge[16]; /* valid for 4addr_multi_ap */
	char ssid[32];
	char psk[128];
	char bssid[6]; /* 00:00:00:00:00:00 means any */
	size_t ssid_len;
	size_t psk_len;
        int multi_ap;
        char pairwise[64]; 
        char proto[64];
        char key_mgmt[64];
	unsigned long flags; /* enum wifi_sta_network_flags */
} wifi_sta_network_t;

typedef struct wifi_sta_network_state {
	struct wifi_sta_network desired_network;
	char current_bssid[6]; /* 00:00:00:00:00:00 means disconnected */
	int last_deauth_reason;
	int last_disassoc_reason;
} wifi_sta_network_state_t;

typedef void (*wifi_sta_network_cb)(INT radioIndex, void *ctx);
INT wifi_setStaNetwork(INT radioIndex, const wifi_sta_network_t *network);
INT wifi_setStaNetworkSetEventCallback(INT radioIndex, wifi_sta_network_cb *cb, void *ctx);
INT wifi_getStaNetworkState(INT radioIndex, wifi_sta_network_state_t *state);
INT wifi_getStaNetworkCapabilities(INT radioIndex, INT *flags);

/* APPROACH 2 */
typedef struct {
	CHAR ssid[33];
	CHAR bssid[17];
	CHAR passphrase[65];
} wifi_staNetwork_t;

INT wifi_getSTANetworks(INT apIndex, wifi_sta_network_t **out_staNetworks_array, INT out_array_size, BOOL *out_scan_cur_freq);

INT wifi_setSTANetworks(INT apIndex, wifi_sta_network_t **staNetworks_array, INT array_size, BOOL scan_cur_freq);

INT wifi_delSTANetworks(INT apIndex);


/* notdefined approach */
INT wifi_getSTANumberOfEntries(ULONG *output);
INT wifi_getSTARadioIndex(INT ssidIndex, INT *radioIndex);
INT wifi_getSTAName(INT apIndex, CHAR *output_string);
INT wifi_getSTABSSID(INT ssidIndex, CHAR *output_string);
INT wifi_getSTASSID(INT ssidIndex, CHAR *output_string);
INT wifi_getSTAMAC(INT ssidIndex, CHAR *output_string);
INT wifi_getSTAEnabled(INT ssidIndex, BOOL *enabled);
INT wifi_setSTAEnabled(INT ssidIndex, BOOL enabled);

typedef struct _wifi_client_associated_dev
{
        UCHAR MACAddress[6];                /**< The MAC address of an associated device. */
        INT NetworkID;			    /**< Network ID */
        INT connected;			    /**< If network is connected */
        INT reason;
        INT locally_generated;
} wifi_client_associated_dev_t;       

typedef INT ( * wifi_client_event_callback)(INT apIndex, wifi_client_associated_dev_t *state);

void wifi_client_event_callback_register(wifi_client_event_callback callback_proc);
INT wifi_getApChannel(INT radioIndex,ULONG *output_ulong);   //RDKB

INT wifi_setApChannel(INT radioIndex, ULONG channel);        //RDKB  //AP only


#endif
