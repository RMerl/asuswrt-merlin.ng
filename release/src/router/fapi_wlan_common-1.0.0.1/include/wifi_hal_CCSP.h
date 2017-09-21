/**********************************************************************
   Copyright [2014] [Cisco Systems, Inc.]

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

	   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
**********************************************************************/

/**********************************************************************

	module: wifi_hal.h

		For CCSP Component:  Wifi_Provisioning_and_management

	---------------------------------------------------------------

	copyright:

		Cisco Systems, Inc., 2014
		All Rights Reserved.

	---------------------------------------------------------------

	description:

		This header file gives the function call prototypes and
		structure definitions used for the RDK-Broadband
		Wifi hardware abstraction layer

		NOTE:
		THIS VERSION IS AN EARLY DRAFT INTENDED TO GET COMMENTS FROM COMCAST.
		TESTING HAS NOT YET BEEN COMPLETED.

	---------------------------------------------------------------

	environment:

		This HAL layer is intended to support Wifi drivers
		through an open API.  The current implementation created
		below this HAL supports Qualcomm / Atheros Wifi.
		Changes may be needed to support other Wifi enviornments.

	---------------------------------------------------------------

	author:

		Cisco

**********************************************************************/

#ifndef __WIFI_HAL_H__
#define __WIFI_HAL_H__


#ifndef ULONG
#define ULONG unsigned long
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


/**********************************************************************
				STRUCTURE DEFINITIONS
**********************************************************************/

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

typedef struct _wifi_ssidTrafficStats2
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
} wifi_ssidTrafficStats2_t;

typedef struct _wifi_metrics
{
	char wifi_StartTime[32];
	int wifi_NoiseFloorMedian;
	int wifi_NoiseFloorMin;
	int wifi_NoiseFloorMax;
	ULONG wifi_ChannelUtilization;
	ULONG wifi_TxCount;
	ULONG wifi_RxCount;
	ULONG wifi_RetryCount;
	char wifi_RssiHist[120];
} wifi_metrics_t;

typedef enum _WIFI_METRICS
{
	WIFI_METRICS_START_TIME = 1,
	WIFI_METRICS_NOISE_FLOOR_MEDIAN,
	WIFI_METRICS_NOISE_FLOOR_MIN,
	WIFI_METRICS_NOISE_FLOOR_MAX,
	WIFI_METRICS_CHANNEL_UTILIZATION,
	WIFI_METRICS_TX_COUNT,
	WIFI_METRICS_RX_COUNT,
	WIFI_METRICS_RETRY_COUNT,
	WIFI_METRICS_RSSI_HIST
} WIFI_METRICS;

typedef struct _wifi_device
{
	UCHAR wifi_devMacAddress[6];
	CHAR wifi_devIPAddress[64];
	BOOL wifi_devAssociatedDeviceAuthentiationState;
	INT  wifi_devSignalStrength;
	INT  wifi_devTxRate;
	INT  wifi_devRxRate;
	UCHAR wifi_devMode[3];
} wifi_device_t;

typedef struct _wifi_associated_dev
{
	UCHAR cli_MACAddress[6];		// The MAC address of an associated device.
	CHAR  cli_IPAddress[64];		// IP of the associated device
	BOOL  cli_AuthenticationState; // Whether an associated device has authenticated (true) or not (false).
	UINT  cli_LastDataDownlinkRate; //The data transmit rate in kbps that was most recently used for transmission from the access point to the associated device.
	UINT  cli_LastDataUplinkRate; 	// The data transmit rate in kbps that was most recently used for transmission from the associated device to the access point.
	INT   cli_SignalStrength; 		//An indicator of radio signal strength of the uplink from the associated device to the access point, measured in dBm, as an average of the last 100 packets received from the device.
	UINT  cli_Retransmissions; 	//The number of packets that had to be re-transmitted, from the last 100 packets sent to the associated device. Multiple re-transmissions of the same packet count as one.
	BOOL  cli_Active; 				//	boolean	-	Whether or not this node is currently present in the WiFi AccessPoint network.

	CHAR  cli_OperatingStandard[64];	//Radio standard the associated Wi-Fi client device is operating under. Enumeration of:
//	 CHAR  cli_OperatingChannelBandwidth[64];	//The operating channel bandwidth of the associated device. The channel bandwidth (applicable to 802.11n and 802.11ac specifications only). Enumeration of:
	INT   cli_SNR;		//Measured in decibels (dB).
	CHAR  cli_InterferenceSources[64]; //This parameter reports the probable interference sources that this Wi-Fi access point may be observing. The value of this parameter is a comma seperated list of the following possible sources: eg: MicrowaveOven,CordlessPhone,BluetoothDevices,FluorescentLights,ContinuousWaves,Others
	ULONG cli_DataFramesSentAck;	//The DataFramesSentAck parameter indicates the total number of MSDU frames marked as duplicates and non duplicates acknowledged. The value of this counter may be reset to zero when the CPE is rebooted. Refer section A.2.3.14 of CableLabs Wi-Fi MGMT Specification.
	ULONG cli_DataFramesSentNoAck;	//The DataFramesSentNoAck parameter indicates the total number of MSDU frames retransmitted out of the interface (i.e., marked as duplicate and non-duplicate) and not acknowledged, but does not exclude those defined in the DataFramesLost parameter. The value of this counter may be reset to zero when the CPE is rebooted. Refer section A.2.3.14 of CableLabs Wi-Fi MGMT Specification.
	ULONG cli_BytesSent;	//The total number of bytes transmitted to the client device, including framing characters.
	ULONG cli_BytesReceived;	//The total number of bytes received from the client device, including framing characters.
	INT   cli_RSSI;	//The Received Signal Strength Indicator, RSSI, parameter is the energy observed at the antenna receiver for transmissions from the device averaged over past 100 packets recevied from the device.
	INT   cli_MinRSSI;	//The Minimum Received Signal Strength Indicator, RSSI, parameter is the minimum energy observed at the antenna receiver for past transmissions (100 packets).
	INT   cli_MaxRSSI;	//The Maximum Received Signal Strength Indicator, RSSI, parameter is the energy observed at the antenna receiver for past transmissions (100 packets).
	UINT  cli_Disassociations;	//This parameter  represents the total number of client disassociations. Reset the parameter evey 24hrs or reboot
	UINT  cli_AuthenticationFailures;	//This parameter indicates the total number of authentication failures.  Reset the parameter evey 24hrs or reboot
} wifi_associated_dev_t;	//~COSA_DML_WIFI_AP_ASSOC_DEVICE

typedef struct _wifi_neighbor_ap2
{
	CHAR  ap_Radio[64];	//The value MUST be the path name of a row in Device.WiFi.Radio table. The Radio that detected the neighboring WiFi SSID.  
	CHAR  ap_SSID[64];	//The current service set identifier in use by the neighboring WiFi SSID. The value MAY be empty for hidden SSIDs.
	CHAR  ap_BSSID[64];	//[MACAddress] The BSSID used for the neighboring WiFi SSID.
	CHAR  ap_Mode[64];	//The mode the neighboring WiFi radio is operating in. Enumeration of: AdHoc, Infrastructure
	UINT  ap_Channel;	//The current radio channel used by the neighboring WiFi radio.
	INT   ap_SignalStrength;	//An indicator of radio signal strength (RSSI) of the neighboring WiFi radio measured in dBm, as an average of the last 100 packets received.
	CHAR  ap_SecurityModeEnabled[64];	//The type of encryption the neighboring WiFi SSID advertises. Enumeration of:None, WPA-WPA2 etc.
	CHAR  ap_EncryptionMode[64];	//Comma-separated list of strings. The type of encryption the neighboring WiFi SSID advertises. Each list item is an enumeration of: TKIP, AES
	CHAR  ap_OperatingFrequencyBand[16];	//Indicates the frequency band at which the radio this SSID instance is operating. Enumeration of:2.4GHz, 5GHz
	CHAR  ap_SupportedStandards[64];	//Comma-separated list of strings. List items indicate which IEEE 802.11 standards this Result instance can support simultaneously, in the frequency band specified by OperatingFrequencyBand. Each list item is an enumeration of:
	CHAR  ap_OperatingStandards[16];	//Comma-separated list of strings. Each list item MUST be a member of the list reported by theSupportedStandards parameter. List items indicate which IEEE 802.11 standard that is detected for thisResult.
	CHAR  ap_OperatingChannelBandwidth[16];	//Indicates the bandwidth at which the channel is operating. Enumeration of:
	UINT  ap_BeaconPeriod;	//Time interval (in ms) between transmitting beacons.
	INT   ap_Noise;	//Indicator of average noise strength (in dBm) received from the neighboring WiFi radio.
	CHAR  ap_BasicDataTransferRates[256];	//Comma-separated list (maximum list length 256) of strings. Basic data transmit rates (in Mbps) for the SSID. For example, if BasicDataTransferRates is "1,2", this indicates that the SSID is operating with basic rates of 1 Mbps and 2 Mbps.
	CHAR  ap_SupportedDataTransferRates[256];	//Comma-separated list (maximum list length 256) of strings. Data transmit rates (in Mbps) for unicast frames at which the SSID will permit a station to connect. For example, if SupportedDataTransferRates is "1,2,5.5", this indicates that the SSID will only permit connections at 1 Mbps, 2 Mbps and 5.5 Mbps.
	UINT  ap_DTIMPeriod;	//The number of beacon intervals that elapse between transmission of Beacon frames containing a TIM element whose DTIM count field is 0. This value is transmitted in the DTIM Period field of beacon frames. [802.11-2012]
	UINT  ap_ChannelUtilization;	//Indicates the fraction of the time AP senses that the channel is in use by the neighboring AP for transmissions.

} wifi_neighbor_ap2_t;	//COSA_DML_NEIGHTBOURING_WIFI_RESULT


/**********************************************************************************
 *
 *  Wifi Subsystem level function prototypes
 *
**********************************************************************************/

/* wifi_init() function */
/**
* Description: Initializes the Wifi subsystem.  This includes initilization of all
*  radios.  Implementation specifics may dictate what is actualy initializes since
*  different hardware implementations may have different requirements.
*  Parameters : None
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
INT wifi_init();                                                    // Initializes the wifi subsystem (all radios)

/* wifi_reset() function */
/**
* Description: Resets the Wifi subsystem.  This includes reset of all AP varibles.
*  Implementation specifics may dictate what is actualy reset since
*  different hardware implementations may have different requirements.
*  Parameters : None
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
INT wifi_reset();                                                   // resets the wifi subsystem, deletes all APs

/* wifi_down() function */
/**
* Description: 
*  Turns off transmit power to all radios.
*  Implementation specifics may dictate some functionality since
*  different hardware implementations may have different requirements.
*  Parameters : None
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
INT wifi_down();                                                    // turns off transmit power for the entire Wifi subsystem, for all radios

/* wifi_factoryReset() function */
/**
* Description: 
*  Resets Implementation specifics may dictate some functionality since
*  different hardware implementations may have different requirements.
*  Parameters : None
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
INT wifi_factoryReset();                                            // clears internal variables to implement a factory reset of the wifi subsystem

/* wifi_factoryResetRadios() function */
/**

* Description: 
*  Resets Implementation specifics may dictate some functionality since
*  different hardware implementations may have different requirements.
*  Parameters : None
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
INT wifi_factoryResetRadios();

/* wifi_createInitialConfigFiles() function */
/**
* Description: 
*  This function creates wifi configuration files.  The format
*  and content of these files are implementation dependent.  This function call is
*  used to trigger this task if necessary. Some implementations may not need this
*  function. If an implementation does not need to create config files the function call can
*  do nothing and return RETURN_OK.
*  Parameters : None
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
INT wifi_createInitialConfigFiles();                                // creates initial implementation dependent configuration files that are later used for variable storage.  Not all implementations may need this function.  If not needed for a particular implementation simply return no-error (0)


/**********************************************************************************
 *
 *  Wifi radio level function prototypes
 *
**********************************************************************************/

/* wifi_initRadio() function */
/**
* Description: This function call initializes the specified radio.
*  Implementation specifics may dictate the functionality since
*  different hardware implementations may have different initilization requirements.
* Parameters : radioIndex - The index of the radio. First radio is index 0. 2nd radio is index 1   - type INT
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
INT wifi_initRadio(INT radioIndex);                                 // Initializes the specified radio, 0 is the first radio index, 1 is the 2nd radio index


/* wifi_getBaseBSSID() function */
/**
* Description: This function call outputs a 32 byte or less string with the base mac address of this radio.
*  Each radio may support mulitiple BSSIDs with each BSSID having a mac address.  This funtion call
*  outputs the base (lowest) mac address assocated with this radio.
* Parameters : 
*  radioIndex - The index of the radio. First radio is index 0. 2nd radio is index 1   - type INT
*  *output_string - a pointer to a 32 buffer that was preallocated by the caller.  This is where the output is written
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
INT wifi_getBaseBSSID(INT radioIndex, CHAR *output_string);         // Outputs a 32 byte or less status string with the base mac address of the radio.  The string buffer must be pre-allocated by caller.


/* wifi_getMaxBitRate() function */
/**
* Description: 
*  This fuction outputs a 32 byte or less string indicating the max bit rate of this radio in bps.
*  Parameters :
*  radioIndex - The index of the radio. First radio is index 0. 2nd radio is index 1   - type INT
*  *output_string - a pointer to a 32 buffer that was preallocated by the caller.  This is where the output is written
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
INT wifi_getMaxBitRate(INT radioIndex, CHAR *output_string);        // Outputs a 32 byte or less string indicating the max bit rate.  String buffer must be preallocated.

/* wifi_getChannel() function */
/**
* Description: 
*  This fuction outputs a value indicating the channel number currently in use on this radio
*  Parameters :
*  radioIndex - The index of the radio. First radio is index 0. 2nd radio is index 1   - type INT
*  *output_ulong - a pointer to a unsigned long value that was preallocated by the caller.  This is where the output is written
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
INT wifi_getChannel(INT radioIndex, ULONG *output_ulong);            // outputs the channel currently in use on this radio 

/* wifi_setChannel() function */
/**
* Description: 
*  This fuction sets a driver internal paramater correspdonding to the channel number to use on this radio
*  Parameters :
*  radioIndex - The index of the radio. First radio is index 0. 2nd radio is index 1   - type INT
*  channel - a unsigned long value that is used to pass the channel number
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
INT wifi_setChannel(INT radioIndex, ULONG channel);                 // sets an environment variable for this radio to a specfic channel


/* wifi_getAutoChannelEnable() function */
/**
* Description: 
*  This function outputs a bool value indicating if auto channel selection is currently in use on this radio
*  Parameters :
*  radioIndex - The index of the radio. First radio is index 0. 2nd radio is index 1   - type INT
*  *output_bool - a pointer to a bool value that was preallocated by the caller.  This is where the output is written. 1 == enabled, 0 == disabled
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
INT wifi_getAutoChannelEnable(INT radioIndex, BOOL *output_bool);   // gets the current status of the auto channel enable flag 1 == enabled, 0 == disabled

/* wifi_setAutoChannelEnable() function */
/**
* Description: 
*  This function enables or disables a driver level variable to indicate if auto channel selection is enabled on this radio
*  Parameters :
*  radioIndex - The index of the radio. First radio is index 0. 2nd radio is index 1  - type INT
*  enable - a BOOL value that enables auto channel selection on this radio.  1 == enabled, 0 == disabled
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
INT wifi_setAutoChannelEnable(INT radioIndex, BOOL enable);         // sets the auto channel enable enviornemnt flag cooresponding to this radio, 1 == enabled, 0 == disabled

/* wifi_getStandard() function */
/**
* Description: 
*  This function outputs the PHY level standards supported by this radio.
*  Parameters :
*  radioIndex - The index of the radio. First radio is index 0. 2nd radio is index 1   - type INT
*  *output_string - a pointer to a string that was preallocated by the caller.  This is where the output is written.
*     valid strings are formated as "a", "b", "g"(b&g clients),"g-"(only g),"n","ac",
*  *gOnly - a pointer to a BOOL value.  1 indicates that the radio is in g-only mode, 0 indicates not in g-only mode
*  *nOnly - a pointer to a BOOL value.  1 indicates that the radio is in n-only mode, 0 indicates not in n-only mode
*  *acOnly - a pointer to a BOOL value.  1 indicates that the radio is in ac-only mode, 0 indicates not in ac-only mode
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
INT wifi_getStandard(INT radioIndex, CHAR *output_string, BOOL *gOnly, BOOL *nOnly, BOOL *acOnly); // Outputs a string formated as "a", "b", "g"(b&g clients),"g-"(only g),"n","ac", outputs BOOL flags for specific modes

/* wifi_getPossibleChannels() function */
/**
* Description: This function call outputs a 128 byte or less string with the supported radio channels on this radio.
*  Format is comma separated list of possible channels. String must be preallocaated by caller.
* Parameters : 
*  radioIndex - The index of the radio. First radio is index 0. 2nd radio is index 1   - type INT
*  *output_string - a pointer to a 128 buffer that was preallocated by the caller.  This is where the output is written
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
INT wifi_getPossibleChannels(INT radioIndex, CHAR *output_string);  // outputs a 128 byte or less string.  Format is comma separated list of possible channels. String must be preallocaated by caller.

/*******
 * NOTE: FILE FORMATING AND CREATION OF COMMENTS IS A WORK IN PROGRESS BELOW THIS POINT
**********/

INT wifi_setRadioEnable(INT radioIndex, BOOL enable);               // sets an environment variable that indicates radio enable status.  1 enables, 0 disables radio
INT wifi_getRadioEnable(INT radioIndex, BOOL *output_bool);         // outputs 1 if radio is enabled, outputs 0 if radio is disabled
INT wifi_getRadioActive(INT radioIndex, BOOL *output_bool);         // outputs 1 if radio is active, outputs 0 if radio is not active
INT wifi_getTransmitPower(INT radioIndex, ULONG *output_ulong);     // outputs the transmit power in dBm
INT wifi_setTransmitPower(INT radioIndex, ULONG TransmitPower);     // sets the transmit power in dBm in the hardware
INT wifi_setDTIMInterval(INT apIndex, INT dtimInterval);            // sets the Delivery Traffic Indication Message Interval for the radio used by this ap, pushes this setting to the hardware
INT wifi_setCtsProtectionEnable(INT apIndex, BOOL enable);          // enables CTS protection for the radio used by this AP, pushes this setting to the hardware
INT wifi_setObssCoexistenceEnable(INT apIndex, BOOL enable);        // enables OBSS Coexistence - fall back to 20MHz if necessary for the radio used by this ap, pushes this setting to the hardware
INT wifi_setFragmentationThreshold(INT apIndex, UINT threshold);    // sets the fragmentation threshold in bytes for the radio used by this ap, pushes this setting to the hardware
INT wifi_getChannelMode(INT radioIndex, CHAR *output_string);	  // outputs to a 32 byte string, must be pre-allocataed by caller.  String will contain either "20","40" or "80".  If 40 then extension channel frequency will be indicated by either "PLUS" or "MINUS"
INT wifi_setChannelMode(INT radioIndex, CHAR *channelMode, BOOL gOnlyFlag, BOOL nOnlyFlag, BOOL acOnlyFlag);  // sets the channel mode environment variable.  32 charater max string may contain "11A",11B","11G","11NGHT20","11NAHT20","11NGHT40PLUS","11NGHT40MINUS","11NAHT40PLUS","11NAHT40MINUS","11ACVHT20","11ACVHT40PLUS","11ACVHT40MINUS","11ACVHT80"  
INT wifi_getCountryCode(INT radioIndex, CHAR *output_string);       // outputs the country code to a max 64 character string
INT wifi_setSTBCEnable(INT radioIndex, BOOL STBC_Enable);           // enable STBC mode in the hardware, 0 == not enabled, 1 == enabled 
INT wifi_getAMSDUEnable(INT radioIndex, BOOL *output_bool);         // outputs A-MSDU enable status, 0 == not enabled, 1 == enabled 
INT wifi_setAMSDUEnable(INT radioIndex, BOOL amsduEnable);          // enables A-MSDU in the hardware, 0 == not enabled, 1 == enabled  
INT wifi_setShortGuardInterval(INT radioIndex, BOOL enable);        // enables short guard interval mode on the specified radio, 0 == not enabled, 1 == enabled 
INT wifi_getTxChainMask(INT radioIndex, INT *output_int);            // outputs the number of Tx streams
INT wifi_setTxChainMask(INT radioIndex, INT numStreams);            // sets the number of Tx streams to an environment variable  
INT wifi_getRxChainMask(INT radioIndex, INT *output_int);            // outputs the number of Rx streams
INT wifi_setRxChainMask(INT radioIndex, INT numStreams);            // sets the number of Rx streams to an environment variable
INT wifi_getChannelsInUse(INT radioIndex, CHAR *output_string);     // outputs the channels in use
INT wifi_scanApChannels(INT radioIndex, CHAR *scanData);            // outputs the scan data
INT wifi_getWifiEnableStatus(INT radioIndex, BOOL *output_bool);    // outputs the current state of the internal environment variable that indicates enable/disable status. enabled==1 or disabled==0
INT wifi_setWifiEnableStatus(INT radioIndex, BOOL enabled);         // sets the internal environment variable for wifi enable/disable
INT wifi_getSupportedFrequencyBands(INT radioIndex, CHAR *output_string);  // Outputs supported frequency bands to a max 10 character string
INT wifi_pushDefaultValues(INT radioIndex);                         // pushes the default values for this radio to the hardware. 
INT wifi_pushChannel(INT radioIndex, UINT channel);                 // push the channel number setting to the hardware
INT wifi_pushChannelMode(INT radioIndex);                           // push the channel mode environment variable that is set by "wifi_setChannelMode()" to the hardware
INT wifi_pushTxChainMask(INT radioIndex);                           // push the environment variable that is set by "wifi_setTxChainMask()" to the hardware
INT wifi_pushRxChainMask(INT radioIndex);                           // push the environment variable that is set by "wifi_setRxChainMask()" to the hardware
INT wifi_getRadioIEEE80211hEnabled(INT radioIndex, BOOL *enable);    // outputs the IEEE80211hEnabled enabled status, 0 == not enabled, 1 == enabled
INT wifi_getRadioIEEE80211hSupported(INT radioIndex, BOOL *Supported);// outputs the IEEE80211hSupported status, 0 == not supported, 1 == supported  
INT wifi_getRadioAutoChannelRefreshPeriodSupported(INT radioIndex, BOOL *output_bool);// outputs the AutoChannelRefreshPeriodSupported  status, 0 == not supported, 1 == supported
INT wifi_getRadioAutoChannelRefreshPeriod(INT radioIndex, ULONG *output_ulong);//outputs the AutoChannelRefreshPeriodSupported  enabled status, 0 == not enabled, 1 == enabled
INT wifi_setRadioAutoChannelRefreshPeriod(INT radioIndex, ULONG seconds);// set the AutoChannelRefreshPeriod    
INT wifi_getRadioDCSSupported(INT radioIndex, BOOL *output_bool);   // outputs the DCS support status, 0 == not supported, 1 == supported
INT wifi_getRadioDCSEnable(INT radioIndex, BOOL *output_bool);      // outputs the DCS enable status, 0 == not enabled, 1 == enabled
INT wifi_setRadioDCSEnable(INT radioIndex, BOOL enable);            // enables DCS, 0 == not enabled, 1 == enabled

INT wifi_getNeighboringWiFiDiagnosticResult2(INT radioIndex, wifi_neighbor_ap2_t ***neighbor_ap_array, UINT *output_array_size); //Tr181

/**********************************************************************************
 *
 *  Access Point level function prototypes (aka Virtual Access Point)
 *
**********************************************************************************/

INT wifi_createAp(INT apIndex, INT radioIndex, CHAR *essid, BOOL hideSsid);  // creates a new ap and pushes these parameters to the hardware
INT wifi_deleteAp(INT apIndex);                                     // deletes this ap entry on the hardware, clears all internal variables associaated with this ap
INT wifi_setEnable(INT apIndex, BOOL enable);                       // sets the AP enable status variable for the specified ap.
INT wifi_getEnable(INT apIndex, BOOL *output_bool);                 // Outputs the state of the internal variable that is set by wifi_setEnable().  
INT wifi_getApEnable(INT apIndex, BOOL *enableFlag);                // Outputs the actual status of the specified ap. 0 == not enabled, 1 == enabled 
INT wifi_getEnableOnLine(INT apIndex, BOOL *output_bool);           // Outputs a bool that indicates on the state of the Enable OnLine variable for this ap.
INT wifi_setEnableOnLine(INT apIndex, BOOL enabled);                // sets the on line enable variable for this ap.  If set the ap will not be enabled until all required network connetions are ready.
INT wifi_ifConfigUp(INT apIndex);                                   // sets internal variables to indicate that the ap is enabled, specific internal variabls are implementation dependendent,
INT wifi_ifConfigDown(INT apIndex);									// 
INT wifi_getStatus(INT apIndex, CHAR *output_string);               // Outputs a 32 byte or less AP status string.  The string buffer must be pre-allocated by caller.
INT wifi_getName(INT apIndex, CHAR *output_string);                 // Outputs a 16 byte or less name associated with the AP.  String buffer must be pre-allocated by the caller

INT wifi_getSSID(INT apIndex, CHAR *output_string);                 // Outputs a 32 byte or less string indicating the SSID name.  Sring buffer must be preallocated by the caller.
INT wifi_setSSID(INT apIndex, CHAR *ssid_string);                   // accepts a max 32 byte string and sets an internal variable to the SSID name 
INT wifi_pushSSID(INT apIndex, CHAR *ssid);                         // push the ssid name to the hardware

INT wifi_getSsidAdvertisementEnable(INT apIndex, BOOL *output_bool);// outputs a 1 if SSID on the AP is enabled, else ouputs 0
INT wifi_setSsidAdvertisementEnable(INT apIndex, BOOL enable);      // sets an internal variable for ssid advertisement.  Set to 1 to enable, set to 0 to disable
INT wifi_pushSsidAdvertisementEnable(INT apIndex, BOOL enable);     // push the ssid advertisement enable variable to the hardware

INT wifi_getBeaconType(INT apIndex, CHAR *output_string);           // Outputs a 32 byte or less string indicating the beacon type as "None", "Basic", "WPA, "11i", "WPAand11i"
INT wifi_setBeaconType(INT apIndex, CHAR *beaconTypeString);        // Sets the beacon type environment variable. Allowed input strings are "None", "Basic", "WPA, "11i", "WPAand11i"

INT wifi_setBasicEncryptionMode(INT apIndex, CHAR *encMode);        // sets an environment variable for the basic encryption mode.  Valid encMode strings are "None" or "WEPEncryption"
INT wifi_setBasicAuthenticationMode(INT apIndex, CHAR *authMode);   // sets an environment variable for the authMode. Valid strings are "None", "EAPAuthentication" or "SharedAuthentication"
INT wifi_getWpaEncryptoinMode(INT apIndex, CHAR *output_string);    // outputs up to a 32 byte string as either "TKIPEncryption", "AESEncryption", or "TKIPandAESEncryption"
INT wifi_setWpaEncryptionMode(INT apIndex, CHAR *encMode);          // sets the encryption mode environment variable.  Valid string format is "TKIPEncryption", "AESEncryption", or "TKIPandAESEncryption"
INT wifi_getWpaRekeyInterval(INT apIndex, INT *output_int);         // outputs the rekey interval
INT wifi_setWpaRekeyInterval(INT apIndex, INT rekeyInterval);       // sets the internal variable for the rekey interval

INT wifi_getWepKey(INT apIndex, INT keyIndex, CHAR *output_string); // output_string must be pre-allocated as 128 character string by caller
INT wifi_setWepKey(INT apIndex, INT keyIndex, CHAR *WepKey);        // sets an environment variable for the WepKey.  WepKey string must be a maximum of 128 characters
INT wifi_pushWepKey(INT apIndex, INT keyIndex);                     // push the Wep key to the hardware
INT wifi_getApWepKeyIndex(INT apIndex, ULONG *output_ulong);         // Outputs the WEP key index (1-4) for this AP
INT wifi_setApWepKeyIndex(INT apIndex, ULONG output_ulong);          // Sets an environment variable for the WEP key index (1-4) for this AP
INT wifi_pushWepKeyIndex(INT apIndex, UINT keyIndex);               // push the wep key index to the hardware

INT wifi_removeSecVaribles(INT apIndex);                            // deletes internal security varable settings for this ap
INT wifi_disableEncryption(INT apIndex);                            // changes the hardware settings to disable encryption on this ap
INT wifi_setAuthMode(INT apIndex, INT mode);                        // set the authorization mode on this ap

INT wifi_getBasicTrafficStats(INT apIndex, wifi_basicTrafficStats_t *output_struct);  // outputs basic traffic stats per AP
INT wifi_getSSIDTrafficStats2(INT apIndex, wifi_ssidTrafficStats2_t *output_struct);  // outputs basic traffic stats per AP
INT wifi_getWifiTrafficStats(INT apIndex, wifi_trafficStats_t *output_struct); // outputs more detailed traffic stats per AP
INT wifi_getNumDevicesAssociated(INT apIndex, ULONG *output_ulong); // outputs the number of stations assocated per AP
INT wifi_getAllAssociatedDeviceDetail(INT apIndex, ULONG *output_ulong, wifi_device_t ***output_struct); // Outputs the number of assocated devcies and the device details
INT wifi_getAssociatedDeviceDetail(INT apIndex, INT devIndex, wifi_device_t *output_struct); // Outputs device details of a single device with index devIndex 

INT wifi_kickAssociatedDevice(INT apIndex, wifi_device_t *device);  // manually removes any active wi-fi association with the device specified on this ap


INT wifi_getPreSharedKey(INT apIndex, CHAR *output_string);         // output_string must be pre-allocated as 64 character string by caller
INT wifi_setPreSharedKey(INT apIndex, CHAR *preSharedKey);          // sets an environment variable for the psk. Input string preSharedKey must be a maximum of 64 characters
INT wifi_getKeyPassphrase(INT apIndex, CHAR *output_string);        // outputs the passphrase, maximum 63 characters
INT wifi_setKeyPassphrase(INT apIndex, CHAR *passPhrase);           // sets the passphrase environment variable, max 63 characters
INT wifi_getWpsEnable(INT apIndex, BOOL *output_bool);              // outputs the WPS enable state of this ap in output_bool 
INT wifi_setWpsEnable(INT apIndex, BOOL enableValue);               // sets the WPS enable environment variable for this ap to the value of enableValue, 1==enabled, 0==disabled
INT wifi_getWpsDevicePIN(INT apIndex, ULONG *output_ulong);         // outputs the pin value, ulong_pin must be allocated by the caller
INT wifi_setWpsDevicePIN(INT apIndex, ULONG pin);                   // set an environment variable for the WPS pin for the selected AP
INT wifi_getWpsConfigMethodsEnabled(INT apIndex, CHAR *output_string); // Outputs a common separated list of the enabled WPS config methods, 64 bytes max
INT wifi_setWpsConfigMethodsEnabled(INT apIndex, CHAR *methodString); // sets an environment variable that specifies the WPS configuration method(s).  methodString is a comma separated list of methods "USBFlashDrive","Ethernet","Label","Display","ExternalNFCToken","NFCInterface","PushButton","Keypad"
INT wifi_getWpsConfigurationState(INT apIndex, CHAR *output_string); // Output string is either Not configured or Configured, max 32 characters
INT wifi_setWpsEnrolleePin(INT apIndex, CHAR *pin);                 // sets the WPS pin for this AP
INT wifi_setWpsButtonPush(INT apIndex);                             // This function is called when the WPS push button has been pressed for this AP
INT wifi_cancelWPS(INT apIndex);                                    // cancels WPS mode for this AP

INT wifi_setWmmEnable(INT apIndex, BOOL enable);                    // enables/disables WMM on the hardwawre for this AP.  enable==1, disable == 0
INT wifi_setWmmUapsdEnable(INT apIndex, BOOL enable);               // enables/disables Automatic Power Save Delivery on the hardware for this AP

INT wifi_setWmmOgAifsn(INT apIndex, INT accessCat, UINT aifsn);  // Set AP/Outgoing WMM AC parameter aifsn
INT wifi_getWmmOgAifsn(INT apIndex, INT accessCat, UINT *aifsn); // Get AP/Outgoing WMM AC parameter aifsn
INT wifi_setWmmOgEcwMin(INT apIndex, INT accessCat, UINT ecwMin); // Set AP/Outgoing WMM AC parameter ECWMin
INT wifi_getWmmOgEcwMin(INT apIndex, INT accessCat, UINT *ecwMin); // Get AP/Outgoing WMM AC parameter ECWMin
INT wifi_setWmmOgEcwMax(INT apIndex, INT accessCat, UINT ecwMax); // Set AP/Outgoing WMM AC parameter ECWMax
INT wifi_getWmmOgEcwMax(INT apIndex, INT accessCat, UINT *ecwMax); // Get AP/Outgoing WMM AC parameter ECWMax
INT wifi_setWmmOgTxOp(INT apIndex, INT accessCat, UINT txOp); // Set AP/Outgoing WMM AC parameter Tx Opportunity
INT wifi_getWmmOgTxOp(INT apIndex, INT accessCat, UINT *txOp); // Get AP/Outgoing WMM AC parameter Tx Opportunity
INT wifi_setWmmOgAckPolicy(INT apIndex, INT accessCat, UINT ackPolicy); // Set AP/Outgoing WMM ACK policy (false = NO acknowledge, true = acknowledge)
INT wifi_getWmmOgAckPolicy(INT apIndex, INT accessCat, UINT *ackPolicy); // Get AP/Outgoing WMM ACK policy

INT wifi_setWmmIcAifsn(INT apIndex, INT accessCat, UINT aifsn);  // Set Sta/Incoming WMM AC parameter aifsn
INT wifi_getWmmIcAifsn(INT apIndex, INT accessCat, UINT *aifsn); // Get Sta/Incoming WMM AC parameter aifsn
INT wifi_setWmmIcEcwMin(INT apIndex, INT accessCat, UINT ecwMin); // Set Sta/Incoming WMM AC parameter ECWMin
INT wifi_getWmmIcEcwMin(INT apIndex, INT accessCat, UINT *ecwMin); // Get Sta/Incoming WMM AC parameter ECWMin
INT wifi_setWmmIcEcwMax(INT apIndex, INT accessCat, UINT ecwMax); // Set Sta/Incoming WMM AC parameter ECWMax
INT wifi_getWmmIcEcwMax(INT apIndex, INT accessCat, UINT *ecwMax); // Get Sta/Incoming WMM AC parameter ECWMax
INT wifi_setWmmIcTxOp(INT apIndex, INT accessCat, UINT txOp); // Set Sta/Incoming WMM AC parameter Tx Opportunity
INT wifi_getWmmIcTxOp(INT apIndex, INT accessCat, UINT *txOp); // Get Sta/Incoming WMM AC parameter Tx Opportunity
INT wifi_setWmmIcAckPolicy(INT apIndex, INT accessCat, UINT ackPolicy); // Set the Sta/Incoming WMM ACK policy (false = NO acknowledge, true = acknowledge)
INT wifi_getWmmIcAckPolicy(INT apIndex, INT accessCat, UINT *ackPolicy); // Get Sta/Incoming WMM ACK policy

INT wifi_getIndexFromName(CHAR *inputSsidString, INT *output_int);   // outputs the index number in that corresponds to the ssid string

INT wifi_setBeaconInterval(INT apIndex, INT beaconInterval);        // sets the beacon interval on the hardware for this AP
INT wifi_setRtsThreshold(INT apIndex, UINT threshold);              // sets the packet size threshold in bytes to apply RTS/CTS backoff rules, pushes this setting to the hardware

INT wifi_setMaxStations(INT apIndex, INT maxStations);              // sets the hardware for the maximum number of stations that can connect to each AP
INT wifi_getRadioIndex(INT apIndex, INT *output_int);                // outputs the radio index for the specified ap
INT wifi_setRadioIndex(INT apIndex, INT radioIndex);                // sets the radio index for the specific ap

INT wifi_addAclDevice(INT apIndex, CHAR *DeviceMacAddress);         // adds the mac address to the filter list
INT wifi_delAclDevice(INT apIndex, CHAR *DeviceMacAddress);         // deletes the mac address from the filter list
INT wifi_getAclDeviceNum(INT apIndex, UINT *output_uint);           // outputs the number of devices in the filter list
INT wifi_kickAclAssociatedDevices(INT apIndex, BOOL enable);         // enable kick for devices on acl black list
INT wifi_setMacAddressControlMode(INT apIndex, INT filterMode);     // sets the mac address filter control mode.  0 == filter disabled, 1 == filter as whitelist, 2 == filter as blacklist

INT wifi_getVlanID(INT apIndex, INT *vlanId);                        // gets the vlan ID for this ap to an internal environment variable
INT wifi_setVlanID(INT apIndex, INT vlanId);                        // sets the vlan ID for this ap to an internal environment variable
INT wifi_getBridgeInfo(INT index, CHAR *bridgeName, CHAR *IP, CHAR *subnet);	// gets bridgeName, IP address and Subnet.
INT wifi_setBridgeInfo(INT apIndex, CHAR *bridgeName, CHAR *IP, CHAR *subnet);   //sets bridgeName, IP address and Subnet to internal environment variables. bridgeName is a maximum of 32 characters, 
INT wifi_pushBridgeInfo(INT apIndex);                               // push the BridgeInfo environment variables to the hardware
INT wifi_resetVlanCfg(INT apIndex);                                 // reset the vlan configuration for this ap
INT wifi_setApBridging(INT apIndex, BOOL bridgeEnable);             // set the environment variables to control briding.  If isolation is requried then disable bridging.  
INT wifi_getRouterEnable(INT apIndex, BOOL *output_bool);           // Outputs a bool that indicates if router is enabled for this ap
INT wifi_setRouterEnable(INT apIndex, BOOL routerEnabled);          // sets the routerEnabled variable for this ap

INT wifi_setApMaxAssociatedDevices(INT apIndex, UINT maxStations); // sets the hardware for the maximum number of stations that can connect to each AP
INT wifi_getApMaxAssociatedDevices(INT apIndex, UINT *output);     // gets the maximum number of stations that can connect to each AP that was set

/**********************************************************************************
 *
 *  Hostapd control function calls used for WPA/WPS security
 *
**********************************************************************************/
INT wifi_createHostApdConfig(INT apIndex, BOOL createWpsCfg);       // creates configuration variables needed for WPA/WPS.  These variables are implementation dependent and in some implementations these variables are used by hostapd when it is started.  Specific variables that are needed are dependent on the hostapd implementation. These variables are set by WPA/WPS security functions in this wifi HAL.  If not needed for a particular implementation this function may simply return no error.
INT wifi_startHostApd();                                            // starts hostapd, uses the variables in the hostapd config with format compatible with the specific hostapd implementation
INT wifi_stopHostApd();                                             // stops hostapd

INT wifi_getRadioNumberOfEntries(ULONG *output);
INT wifi_getRadioIfName(INT radioIndex, CHAR *output_string);
INT wifi_getRadioSupportedStandards(INT radioIndex, CHAR *output_string);
INT wifi_getRadioAutoChannelSupported(INT radioIndex, BOOL *output_bool);
INT wifi_getRadioTransmitPowerSupported(INT radioIndex, CHAR *output_list);

INT wifi_getSSIDNumberOfEntries(ULONG *output);
INT wifi_getSSIDRadioIndex(INT ssidIndex, INT *radioIndex);
INT wifi_getSSIDEnable(INT ssidIndex, BOOL *output_bool);
INT wifi_setSSIDEnable(INT ssidIndex, BOOL enable);
INT wifi_getSSIDStatus(INT ssidIndex, CHAR *output_string);
INT wifi_getSSIDName(INT apIndex, CHAR *output_string);
INT wifi_setSSIDName(INT apIndex, CHAR *ssid_string);

INT wifi_getApWMMCapability(INT apIndex, BOOL *output);
INT wifi_getApUAPSDCapability(INT apIndex, BOOL *output);
INT wifi_getApIsolationEnable(INT apIndex, BOOL *output);
INT wifi_setApIsolationEnable(INT apIndex, BOOL enable);
INT wifi_getApSecurityModesSupported(INT apIndex, CHAR *output);
INT wifi_pushApSsidAdvertisementEnable(INT apIndex, BOOL enable);


// Hal V1
INT wifi_getApSecurityRadiusServerIPAddr(INT apIndex, CHAR *output);
INT wifi_setApSecurityRadiusServerIPAddr(INT apIndex, CHAR *IPAddress);
INT wifi_getApSecurityRadiusServerPort(INT apIndex, UINT *output);
INT wifi_setApSecurityRadiusServerPort(INT apIndex, UINT port);
// Hal V2
INT wifi_getApSecurityRadiusServer(INT apIndex, CHAR *IP_output, UINT *Port_output);
INT wifi_setApSecurityRadiusServer(INT apIndex, CHAR *IPAddress, UINT port);

INT wifi_getAccessNetworkType(INT apIndex, ULONG *output);
INT wifi_setAccessNetworkType(INT apIndex, ULONG ant);

INT wifi_getApAssociatedDeviceDiagnosticResult(INT apIndex, wifi_associated_dev_t  ***associated_dev_array, UINT *output_array_size);

INT wifi_getRadioBasicDataTransmitRates(INT radioIndex, CHAR *output);
INT wifi_setRadioBasicDataTransmitRates(INT radioIndex, CHAR *transmitRates);
INT wifi_getRadioOperatingFrequencyBand(INT radioIndex, CHAR *output_string);
INT wifi_getRadioOperatingChannelBandwidth(INT radioIndex, CHAR *output_string);
INT wifi_setRadioOperatingChannelBandwidth(INT radioIndex, CHAR *bandwidth);
INT wifi_getRadioExtChannel(INT radioIndex, CHAR *output_string);
INT wifi_setRadioExtChannel(INT radioIndex, CHAR *extChannel);
INT wifi_getRadioMCS(INT radioIndex, INT *output);
INT wifi_setRadioMCS(INT radioIndex, INT mcs);

INT wifi_getRadioBeaconPeriod(INT radioIndex, UINT *output);
INT wifi_setRadioBeaconPeriod(INT radioIndex, UINT beaconPeriod);
INT wifi_getRadioGuardInterval(INT radioIndex, CHAR *output_string);
INT wifi_setRadioGuardInterval(INT radioIndex, CHAR *string);

INT wifi_getRadioIGMPSnoopingEnable(INT radioIndex, BOOL *output);
INT wifi_setRadioIGMPSnoopingEnable(INT radioIndex, BOOL enable);

INT wifi_getRadioDeclineBARequestEnable(INT radioIndex, BOOL *output);
INT wifi_setRadioDeclineBARequestEnable(INT radioIndex, BOOL enable);
INT wifi_getRadioAutoBlockAckEnable(INT radioIndex, BOOL *output);
INT wifi_setRadioAutoBlockAckEnable(INT radioIndex, BOOL output);

INT wifi_getRadioUptime(INT radioIndex, ULONG *upTime);

INT wifi_getApWmmUapsdEnable(INT apIndex, BOOL * output);
INT wifi_setApWmmUapsdEnable(INT apIndex, BOOL enable);

INT wifi_getApAclDevices(INT apIndex, CHAR ***macArray, UINT *buf_size);

INT wifi_setApVlanEnable(INT apIndex, BOOL vlanEnable);

INT wifi_getApRetryLimit(INT apIndex, UINT *output);
INT wifi_setApRetryLimit(INT apIndex, UINT retryLimit);

INT wifi_getApSecurityModeEnabled(INT apIndex, CHAR *output);
INT wifi_setApSecurityModeEnabled(INT apIndex, CHAR *encMode);

/* WiFi Metrics for Comcast - Begin */
INT wifi_getMetricsEnable(BOOL *enable);
INT wifi_setMetricsEnable(BOOL enable);
INT wifi_getMetricsMeasuringInterval(UINT *interval);
INT wifi_setMetricsMeasuringInterval(UINT interval);
INT wifi_getMetricsMeasuringRate(UINT *rate);
INT wifi_setMetricsMeasuringRate(UINT interval);
INT wifi_getMetricsData(INT radioIndex, wifi_metrics_t *metrics, WIFI_METRICS param);
INT wifi_getMetricsStartTime(INT radioIndex, CHAR *start);
INT wifi_getMetricsNoiseFloorMedian(INT radioIndex, INT *median);
INT wifi_getMetricsNoiseFloorMin(INT radioIndex, INT *min);
INT wifi_getMetricsNoiseFloorMax(INT radioIndex, INT *max);
INT wifi_getMetricsRssiHist(INT radioIndex, CHAR *hist);
INT wifi_getMetricsChannelUtilization(INT radioIndex, ULONG *chutil);
INT wifi_getMetricsTxCount(INT radioIndex, ULONG *txcnt);
INT wifi_getMetricsRxCount(INT radioIndex, ULONG *rxcnt);
INT wifi_getMetricsRetryCount(INT radioIndex, ULONG *retry);
INT wifi_getMetricsCstExceeded(INT radioIndex, ULONG *percent);
INT wifi_getMetricsActivityFactor(INT radioIndex, ULONG *activity);
/* WiFi Metrics for Comcast - End */

#else
#error "! __WIFI_HAL_H__"
#endif

