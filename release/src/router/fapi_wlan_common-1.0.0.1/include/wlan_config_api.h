/*##################################################################################################
# "Copyright (c) 2013 Intel Corporation                                                            #
# DISTRIBUTABLE AS SAMPLE SOURCE SOFTWARE                                                          #
# This Distributable As Sample Source Software is subject to the terms and conditions              #
# of the Intel Software License Agreement for the Intel(R) Cable and GW Software Development Kit"  #
##################################################################################################*/

#ifndef __RPC_WLAN_CONFIG_API_H_
#define __RPC_WLAN_CONFIG_API_H_

#if defined WLAN_STANDALONE_TEST || !defined YOCTO
#define WLAN_STATUS_ERROR (-1)
#define WLAN_STATUS_OK 1
#else
#include "rpc_wlan_config.h"
#include "help_structs.h"
#endif

#ifndef u_int_32
#define u_int_32 unsigned int
#endif


#ifndef bool
#define bool int
#define true 1
#define false 0
#endif

/** \defgroup FAPI_WLAN_ABSTRACTION FAPI Abstraction layer
*   @ingroup FAPI_WLAN
\brief It provides a set of APIs to configure WLAN subsystem.
*/

/** \addtogroup FAPI_WLAN_ABSTRACTION */
/* @{ */

/*!
*     \brief Wi-Fi FAPI Device Structure
*     */
typedef struct _wifi_device_fapi
{
  unsigned char wifi_devMacAddress[6]; /*!<  Device MAC  */
  char wifi_devIPAddress[64]; /*!<  Device IP  */
  bool wifi_devAssociatedDeviceAuthentiationState;  /*!<  Device Associated Device Authentication State  */
  int  wifi_devSignalStrength;  /*!<  Device Signal Strength  */
  int  wifi_devTxRate;  /*!<  Device TX Rate  */
  int  wifi_devRxRate;  /*!<  Device RX Rate  */
  unsigned char wifi_devMode[3];  /*!<  Device Mode  */
} wifi_device_fapi_t;


#if defined YOCTO
/**************************************************************************/
/*! \fn int wlan_registerRpc(char *remoteHostIpAddr)
 **************************************************************************
 *  \brief registered with this LIB to connect to the rpc wlan config server.
 *  \param[in] char *remoteHostIpAddr - The server IPv4 addr
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_registerRpc(const char *remoteHostIpAddr);

/**************************************************************************/
/*! \fn int wlan_closeRpc(void)
 **************************************************************************
 *  \brief close the connection with the rpc wlan config server.
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_closeRpc(void);
#endif


/**************************************************************************/
/*! \fn int wlan_load(void)
 **************************************************************************
 *  \brief pre initialize the WLAN AP.
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_load(void);


/**************************************************************************/
/*! \fn int wlan_init(void)
 **************************************************************************
 *  \brief initialize the WLAN AP and start forwarding data.
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_init(void);

/**************************************************************************/
/*! \fn int wlan_uninit(void)
**************************************************************************
*  \brief uninitialize the WLAN AP and start forwarding data.
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_uninit(void);

/**************************************************************************/
/*! \fn int wlan_reset(void)
 **************************************************************************
 *  \brief  Put the AP in reset.
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_reset(void);

/**************************************************************************/
/*! \fn int wlan_factoryResetRadios(void)
**************************************************************************
*  \brief  Restart the radio section in nvram and reinit 
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_factoryResetRadios(void);

/**************************************************************************/
/*! \fn int wlan_disableRpcMode(void)
 **************************************************************************
 *  \brief  disableRpcMode and return to regular SDK (UGW)
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_disableRpcMode(void);

/**************************************************************************/
/*! \fn int wlan_createInitialConfigFiles(void)
**************************************************************************
*  \brief  converts xml database to FAPI format
*  \return 0 if success, negative if error
***************************************************************************/
int wlan_createInitialConfigFiles(char *xmlPath);


/**************************************************************************/
/*! \fn int wlan_getNumberOfEntries(int *numEntries)
 **************************************************************************
 *  \brief return the number of WLAN entries.
 *  \param[out] int *numEntries -  number of WLAN logical or physical networks
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getNumberOfEntries(int *numEntries);


/**************************************************************************/
/*! \fn int wlan_ifconfigUp(int vap)
**************************************************************************
*  \brief up the virtual access point
*  \param[in] int vap - virtual access point
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_ifconfigUp(int vap);


/**************************************************************************/
/*! \fn int wlan_ifConfigDown(int vap)
**************************************************************************
*  \brief Down the virtual access point
*  \param[in] int vap - virtual access point
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_ifconfigDown(int vap);


/**************************************************************************/
/*! \fn wlan_getRadioAutoChannelRefreshPeriodSupported(int index, bool *supported)
**************************************************************************
*  \brief Returns if Auto Channel refresh period supported
*  \param[in] int index - radio index
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_getRadioAutoChannelRefreshPeriodSupported(int index, bool *supported);


/**************************************************************************/
/*! \fn wlan_getRadioAutoChannelRefreshPeriod(int index, u_int_32 *refreshPeriod)
**************************************************************************
*  \brief Returns Auto Channel Refresh Period
*  \param[in] int index - radio index
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_getRadioAutoChannelRefreshPeriod(int index, u_int_32 *refreshPeriod);

/**************************************************************************/
/*! \fn wlan_setRadioAutoChannelRefreshPeriod(int index, u_int_32 refreshPeriod)
**************************************************************************
*  \brief Sets Auto Channel Refresh Period
*  \param[in] int index - radio index
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_setRadioAutoChannelRefreshPeriod(int index, u_int_32 refreshPeriod);

/**************************************************************************/
/*! \fn int wlan_getRadioActive(int index, bool *active)
**************************************************************************
*  \brief get Radio is active or not.
*  \param[in] int index - Radio Index
*  \param[out] bool *active - True if radio is enabled
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_getRadioActive(int index, bool *active);

/**************************************************************************/
/*! \fn int wlan_addEntry(int *entries)
 **************************************************************************
 *  \brief return the number of WLAN entries.
 *  \param[out] int *entries - new number of WLAN logical or physical networks.
 *  The newly added entry holds the last index and it is be default disabled
 *  and populated with defaults.
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_addEntry(int *entries);


/**************************************************************************/
/*! \fn int wlan_delEntry(int index, int *entries)
 **************************************************************************
 *  \brief Delete WLAN entry.
 *  \param[in] int index - The index of the entry to be deleted
 *  \param[out] int *entries - new number of WLAN logical or physical networks.
 *  Since the entries are held consecutively, deleting one index causes all
 *  following ones to collapse one index down.
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_delEntry(int index, int *entries);


/**************************************************************************/
/*! \fn int wlan_getBaseBSSID(int index, char *bssid)
**************************************************************************
*  \brief get the base (lowest) mac address associated with this radio
*  \param[in] int index - Radio Index Number
*  \param[out] char *bssid - mac address
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_getBaseBSSID(int index, char *bssid);


/**************************************************************************/
/*! \fn int wlan_getApSecurityModeEnabled(int index, char *modeEnabled)
**************************************************************************
*  \brief get security mode
*  \param[in] int index - Radio Index Number
*  \param[out] char *modeEnabled
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_getApSecurityModeEnabled(int index, char *modeEnabled);


/**************************************************************************/
/*! \fn int wlan_getBssIdPort(char *bssid, int *port, int *index)
 **************************************************************************
 *  \brief Get the port which is associated with a BSSID.
 *  \param[in] char *bssid - MAC address of the BSS interface
 *  \param[out] int *port - L2 switch port on which packets from the BSSID are sent:
 *  SW_PORT_APP, SW_PORT_WLAN1, SW_PORT_WLAN2
 *  \param[out] int *index - The WLAN index of this BSSID
 *  Notes: BSS to port mapping is maintained by the APP
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getBssIdPort(char *bssid, int *port, int *index);


/**************************************************************************/
/*! \fn int wlan_associateVlan(char *bssid, unsigned short vlan)
 **************************************************************************
 *  \brief Associate a VLAN ID for a given BSSID
 *  \param[in] char *bssid - MAC address of the BSS interface
 *  \param[in] unsigned short vlan - VLAN id for the BSS interface
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_associateVlan(char *bssid, unsigned short vlan);


/**************************************************************************/
/*! \fn int wlan_getEnable(int index, bool *enable)
 **************************************************************************
 *  \brief Enable/disable WLAN get.
 *  \param[in] int index - The index of the entry
 *  \param[out] bool *enable - the value of enable.
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getEnable(int index, bool *enable);

/**************************************************************************/
/*! \fn int wlan_getAutoChannelEnable(int index, bool *enable)
**************************************************************************
*  \brief Enable/disable AutoChannelEnable get.
*  \param[in] int index - The index of the entry
*  \param[out] bool *enable - the value of enable.
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_AutoChannelEnable(int index, bool *enable);


/**************************************************************************/
/*! \fn int wlan_setEnable(int index, bool enable)
 **************************************************************************
 *  \brief Enable/disable WLAN set.
 *  \param[in] int index - The index of the entry
 *  \param[in] bool enable - the value of enable.
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setEnable(int index, bool enable);


/**************************************************************************/
/*! \fn int wlan_getStatus(int vap, char *status)
 **************************************************************************
 *  \brief get WLAN status.
 *  \param[in] int index - The index of the entry
 *  \param[out] char *status - the value of the status, "Up", "Error" or "Disabled".
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getStatus(int vap, char *status);


/**************************************************************************/
/*! \fn int wlan_getWifiEnableStatus(int index, char *status)
**************************************************************************
*  \brief get WLAN Radio status.
*  \param[in] int index - The index of the entry
*  \param[out] char *status - the value of the status, "Up", "Error" or "Disabled".
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_getWifiEnableStatus(int index, char *status);


/**************************************************************************/
/*! \fn int wlan_getName(int index, char *name)
 **************************************************************************
 *  \brief get WLAN Network name.
 *  \param[in] int index - The index of the entry
 *  \param[out] char *name - a 16 byte or less name of the interface associated
 *  with the network (for instance wlan0).
 *  String buffer is pre-allocated by the caller.
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getName(int index, char *name);

/**************************************************************************/
/*! \fn int wlan_setApBridging(int index, bool enable)
**************************************************************************
*  \brief Enable /Disable AP bridging
*  \param[in] int index - AP Index
*  \param[in] int enable - true/false
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_setApBridging(int index, bool enable);

/**************************************************************************/
/*! \fn int wlan_getWifiEnableStatusBool(int index, char *status)
**************************************************************************
*  \brief get WLAN Radio status.
*  \param[in] int index - The index of the entry
*  \param[out] bool *status - true if Up, else false
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_getWifiEnableStatusBool(int index, bool *status);

/**************************************************************************/
/*! \fn int fapi_wlan_notify(char *ifDriverName, ObjList *wlObj, char *dbCliFileName)
**************************************************************************
*  \brief notifications, used for saving parameters to WLAN TR181 database from the driver
*  \param[in] char *name - interface name as presented by ifconfig
*  \param[in] ObjList *wlObj - object name
*  \param[in] char *dbCliFilename - path to input file
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_notify(char *ifDriverName, ObjList *wlObj, char *dbCliFileName);

/**************************************************************************/
/*! \fn int wlan_getBSSID(int index, char *bssid)
 **************************************************************************
 *  \brief get WLAN BSSID.
 *  \param[in] int index - The index of the entry
 *  \param[out] char *bssid - MAC address of the BSS interface as the string
 *  String buffer is pre-allocated by the caller.
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getBSSID(int index, char *bssid);


/**************************************************************************/
/*! \fn int wlan_getSSID(int index, char *ssid)
**************************************************************************
*  \brief get WLAN SSID.
*  \param[in] int index - The index of the entry
*  \param[out] char *ssid -holds the interface ssid
*  String buffer is pre-allocated by the caller.
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_getSSID(int index, char *ssid);


/**************************************************************************/
/*! \fn int wlan_getMaxBitRate(int index, char *maxBitRate)
 **************************************************************************
 *  \brief get maximum upstream and downstream bit rate.
 *  \param[in] int index - The index of the entry
 *  \param[out] char *maxBitRate - The maximum upstream and downstream bit rate available
 *  to this connection in Mbps. Either Auto, or the largest of
 *  the OperationalDataTransmitRates values.
 *  Buffer is pre-allocated by the caller and is four bytes long.
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getMaxBitRate(int index, char *maxBitRate);


/**************************************************************************/
/*! \fn int wlan_setMaxBitRate(int index, char * maxBitRate)
 **************************************************************************
 *  \brief set maximum upstream and downstream bit rate.
 *  \param[in] int index - The index of the entry
 *  \param[in] char * maxBitRate - The maximum upstream and downstream bit rate available
 *  to this connection in Mbps. Either Auto, or the largest of
 *  the OperationalDataTransmitRates values.
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setMaxBitRate(int index, char * maxBitRate);


/**************************************************************************/
/*! \fn int wlan_getChannel(int index, u_int_32 *channel)
 **************************************************************************
 *  \brief get radio channel.
 *  \param[in] int index - The index of the entry
 *  \param[out] u_int_32 *channel - radio channel (0-255); used either by setting
 *  explicitly or by applying auth auto channel selection.
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getChannel(int index, u_int_32 *channel);


/**************************************************************************/
/*! \fn int wlan_setChannel(int index, u_int_32 channel)
 **************************************************************************
 *  \brief set radio channel.
 *  \param[in] int index - The index of the entry
 *  \param[in] u_int_32 channel - radio channel (0-255)
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setChannel(int index, u_int_32 channel);


/**************************************************************************/
/*! \fn int wlan_getAutoChannelEnable(int index, bool *enable)
 **************************************************************************
 *  \brief set auto channel enabled/disabled flag.
 *  \param[in] int index - The index of the entry
 *  \param[in] bool enable - Auto channel enabled flag
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getAutoChannelEnable(int index, bool *enable);


/**************************************************************************/
/*! \fn int wlan_setAutoChannelEnable(int index, bool enable)
 **************************************************************************
 *  \brief set auto channel enabled/disabled flag.
 *  \param[in] int index - The index of the entry
 *  \param[in] bool enable - Auto channel enabled flag
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setAutoChannelEnable(int index, bool enable);


/**************************************************************************/
/*! \fn int wlan_getSSID(int index, char *ssid)
 **************************************************************************
 *  \brief get SSID.
 *  \param[in] int index - The index of the entry
 *  \param[out] char *ssid - SSID string (max 32 bytes)
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getSSID(int index, char *ssid);


/**************************************************************************/
/*! \fn int wlan_setSSID(int index, char *ssid)
 **************************************************************************
 *  \brief set SSID.
 *  \param[in] int index - The index of the entry
 *  \param[in] char *out_str - SSID string (max 32 bytes)
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setSSID(int index, char *ssid);

/**************************************************************************/
/*! \fn int wlan_getEnableOnline(int index, bool * enabled)
**************************************************************************
*  \brief get Enable online
*  \param[in] int index - AP Index
*  \param[in] bool *enabled - true/false
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_getEnableOnline(int index, bool * enabled);


/**************************************************************************/
/*! \fn int wlan_setEnableOnline(int index, bool enabled)
**************************************************************************
*  \brief set Enable Online
*  \param[in] int index - AP Index
*  \param[in] bool routerEnabled - true/false
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_setEnableOnline(int index, bool enabled);


/**************************************************************************/
/*! \fn int wlan_getBeaconType(int index, char *beaconType)
 **************************************************************************
 *  \brief get Beacon type.
 *  \param[in] int index - The index of the entry
 *  \param[out] char *beaconType - Beacon type string:
 *  "None", "Basic", "WPA", "11i" (OPTIONAL), "WPAand11i" (OPTIONAL)
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getBeaconType(int index, char *beaconType);


/**************************************************************************/
/*! \fn int wlan_setBeaconType(int index, char *beaconType)
 **************************************************************************
 *  \brief set Beacon type.
 *  \param[in] int index - The index of the entry
 *  \param[in] char *beaconType - Beacon type string:
 *  "None", "Basic", "WPA", "11i" (OPTIONAL), "WPAand11i" (OPTIONAL)
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setBeaconType(int index, char *beaconType);


/**************************************************************************/
/*! \fn int wlan_getMacAddressControlEnabled(int index, bool *enabled)
 **************************************************************************
 *  \brief get MAC Address Control Enable flag.
 *  \param[in] int index - The index of the entry
 *  \param[out] bool *enabled - enable flag. When enabled, a MAC address list
 *  of allowed stations is provided by proprietary API
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getMacAddressControlEnabled(int index, bool *enabled);


/**************************************************************************/
/*! \fn int wlan_setMacAddressControlEnabled(int index, bool enabled)
 **************************************************************************
 *  \brief set MAC Address Control Enable flag.
 *  \param[in] int index - The index of the entry
 *  \param[in] bool enabled - enable flag. When enabled, a MAC address list
 *  of allowed stations is provided by proprietary API
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setMacAddressControlEnabled(int index, bool enabled);


/**************************************************************************/
/*! \fn int wlan_getStandard(int index, char *standard)
 **************************************************************************
 *  \brief get Standard.
 *  \param[in] int index - The index of the entry
 *  \param[out] char *standard - Either one of the following standard represented as a string -
 *  "a","b","g" (b and g clients supported),"g-only" (only g clients supported),"n".
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getStandard(int index, char *standard);


/**************************************************************************/
/*! \fn int wlan_getChannelBandwidth(int radioIndex, char *channelBandwidth)
**************************************************************************
*  \brief returns Channel Bandwidth of the required radio (20Mhz, 40Mhz, 80Mhz, Auto)
*  \param[in] int index - AP Index
*  \param[in] char channelBandwidth - pointer to string array to hold the value
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_getChannelBandwidth(int radioIndex, char *channelBandwidth);


/**************************************************************************/
/*! \fn int wlan_getOperatingFrequencyBand(int radioIndex, char *operatingFrequencyBand)
**************************************************************************
*  \brief returns radio - 2.4GHz, 5GHz
*  \param[in] int index - AP Index
*  \param[in] char operatingFrequencyBand - pointer to string array to hold the value
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_getOperatingFrequencyBand(int radioIndex, char *operatingFrequencyBand);


/**************************************************************************/
/*! \fn int wlan_getExtensionChannel(int radioIndex, char *extensionChannel)
**************************************************************************
*  \brief returns Channel extensionChannel filed (upper, lower,auto)
*  \param[in] int index - AP Index
*  \param[in] char extensionChannel - pointer to string array to hold the value
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_getExtensionChannel(int radioIndex, char *extensionChannel);

int wlan_getApWMMCapability(int index, bool *output);
int wlan_getApUAPSDCapability(int index, bool *output);
int wlan_getApIsolationEnable(int index, bool *output);
int wlan_getApWmmUapsdEnable(int index, bool *output);
int wlan_setApWmmUapsdEnable(int index, bool enable);
int wlan_getApSecurityRadiusServerIPAddr(int index, char *output);
int wlan_setApSecurityRadiusServerIPAddr(int index, char *IPAddress);
int wlan_getApSecurityRadiusServerPort(int index, unsigned int *output);
int wlan_setApSecurityRadiusServerPort(int index, unsigned int port);
int wlan_getApSecurityRadiusServer(int index, char *IP_output, unsigned int *Port_output);
int wlan_setApSecurityRadiusServer(int index, char *IPAddress, unsigned int port);
int wlan_kickAclAssociatedDevices(int index, bool enable);
int wlan_getApAclDevices(int index, char  ***macArray, unsigned int *buf_size);
int wlan_getRadiusSecret(int index, char *radiusSecret);
int wlan_setRadiusSecret(int index, char *radiusSecret);
int wlan_getWpsStatus(int index, char *Status);

/**************************************************************************/
/*! \fn int wlan_cancelWps(int index)
**************************************************************************
*  \brief Cancel WPS
*  \param[in] int index - AP Index
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_cancelWps(int index);


/**************************************************************************/
/*! \fn int wlan_setMacAddressControlMode(int index, int mode)
**************************************************************************
*  \brief set Mac Address Control Mode
*  \param[in] int index - AP Index
*  \param[in] int mode - Mode
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_setMacAddressControlMode(int index, int mode);


/**************************************************************************/
/*! \fn int wlan_getWepKeyIndex(int index, u_int_32 *keyIndex)
 **************************************************************************
 *  \brief get WEP key index.
 *  \param[in] int index - The index of the entry
 *  \param[out] u_int_32 *keyIndex - WEP key index (value 1-4)
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getWepKeyIndex(int index, u_int_32 *keyIndex);


/**************************************************************************/
/*! \fn int wlan_setWepKeyIndex(int index, u_int_32 keyIndex)
 **************************************************************************
 *  \brief set WEP key index.
 *  \param[in] int index - The index of the entry
 *  \param[out] u_int_32 keyIndex - WEP key index (value 1-4)
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setWepKeyIndex(int index, u_int_32 keyIndex);


/**************************************************************************/
/*! \fn int wlan_getPassphrase(int index, char *methods)
**************************************************************************
*  \brief get key pass-phrase
*  \param[in] int index - The index of the entry
*  \param[out] *passphrase - The passphrase (max 63 characters)
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_getPassphrase(int index, char *methods);


/**************************************************************************/
/*! \fn int wlan_getWepKeyPassphrase(int index, char *passphrase)
 **************************************************************************
 *  \brief get WEP key passphrase.
 *  \param[in] int index - The index of the entry
 *  \param[out] char *passphrase - The passphrase (max 63 characters)
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getWepKeyPassphrase(int index, char *passphrase);


/**************************************************************************/
/*! \fn int wlan_setWepKeyPassphrase(int index, char *passphrase)
 **************************************************************************
 *  \brief set WEP key passphrase.
 *  \param[in] int index - The index of the entry
 *  \param[in] char *passphrase - The passphrase (max 63 characters)
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setWepKeyPassphrase(int index, char *passphrase);


/**************************************************************************/
/*! \fn int wlan_getWepEncryptionLevel(int index, char *encLevel)
 **************************************************************************
 *  \brief get WEP Encryption Level.
 *  \param[in] int index - The index of the entry
 *  \param[out] char *encLevel - A comma separated list of the supported levels:
 *  "Disabled", "40-bit", "104-bit"
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getWepEncryptionLevel(int index, char *encLevel);


/**************************************************************************/
/*! \fn int wlan_getBasicEncryptionModes(int index, char *encModes)
 **************************************************************************
 *  \brief get Basic Encryption Modes.
 *  \param[in] int index - The index of the entry
 *  \param[out] char *encModes - Either "None" or "WEPEncryption"
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getBasicEncryptionModes(int index, char *encModes);


/**************************************************************************/
/*! \fn int wlan_setBasicEncryptionModes(int index, char *encModes)
 **************************************************************************
 *  \brief set Basic Encryption Modes.
 *  \param[in] int index - The index of the entry
 *  \param[in] char *encModes - Either "None" or "WEPEncryption"
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setBasicEncryptionModes(int index, char *encModes);


/**************************************************************************/
/*! \fn int wlan_getBasicAuthenticationModes(int index, char *authMode)
 **************************************************************************
 *  \brief get Basic Authentication Modes.
 *  \param[in] int index - The index of the entry
 *  \param[out] char *authMode - Either one of "None" (Open authentication),
 *  "EAPAuthentication" (OPTIONAL), "SharedAuthentication" (OPTIONAL)
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getBasicAuthenticationModes(int index, char *authMode);


/**************************************************************************/
/*! \fn int wlan_setBasicAuthenticationModes(int index, char *authMode)
 **************************************************************************
 *  \brief set Basic Authentication Modes.
 *  \param[in] int index - The index of the entry
 *  \param[in] char *authMode - Either one of "None" (Open authentication),
 *  "EAPAuthentication" (OPTIONAL), "SharedAuthentication" (OPTIONAL)
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setBasicAuthenticationModes(int index, char *authMode);


/**************************************************************************/
/*! \fn int wlan_getWpaEncryptionModes(int index, char *encModes)
 **************************************************************************
 *  \brief get WPA Encryption Modes.
 *  \param[in] int index - The index of the entry
 *  \param[out] char *encModes - Either one of "TKIPEncryption",
 *  "AESEncryption" (OPTIONAL), "TKIPandAESEncryption" (OPTIONAL)
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getWpaEncryptionModes(int index, char *encModes);


/**************************************************************************/
/*! \fn int wlan_setWpaEncryptionModes(int index, char *encModes)
 **************************************************************************
 *  \brief set WPA Encryption Modes.
 *  \param[in] int index - The index of the entry
 *  \param[in] char *encModes - Either one of "TKIPEncryption",
 *  "AESEncryption" (OPTIONAL), "TKIPandAESEncryption" (OPTIONAL)
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setWpaEncryptionModes(int index, char *encModes);


/**************************************************************************/
/*! \fn int wlan_getWpaAuthenticationModes(int index, char *authMode)
 **************************************************************************
 *  \brief get WPA Authentication Modes.
 *  \param[in] int index - The index of the entry
 *  \param[out] char *authMode - Either one of "PSKAuthentication",
 *  "EAPAuthentication" (OPTIONAL)
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getWpaAuthenticationModes(int index, char *authMode);


/**************************************************************************/
/*! \fn int wlan_setWpaBasicAuthenticationModes(int index, char *authMode)
 **************************************************************************
 *  \brief set WPA Authentication Modes.
 *  \param[in] int index - The index of the entry
 *  \param[in] char *authMode - Either one of "PSKAuthentication",
 *  "EAPAuthentication" (OPTIONAL)
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setWpaBasicAuthenticationModes(int index, char *authMode);


/**************************************************************************/
/*! \fn int wlan_getPossibleChannels(int index, char *channels)
 **************************************************************************
 *  \brief get Possible Channels.
 *  \param[in] int index - The index of the entry
 *  \param[out] char *channels - A comma separated list of possible channel scopes like 1-11
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getPossibleChannels(int index, char *channels);


/**************************************************************************/
/*! \fn int wlan_getBasicDataTransmitRates(int index, char *txRates)
 **************************************************************************
 *  \brief get Basic Data Transmit Rates as comma-separated list of the maximum access point
 *  data transmit rates in Mbps for unicast, multicast and broadcast frames.
 *  For instance: 1,2 indicates that 1 and 2 MbPS rates are supported
 *  \param[in] int index - The index of the entry
 *  \param[out] char *txRates - the maximum access point data transmit rates in Mbps.
 *  For instance: 1,2 indicates that 1 and 2 MbPS rates are supported
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getBasicDataTransmitRates(int index, char *txRates);


/**************************************************************************/
/*! \fn int wlan_setBasicDataTransmitRates(int index, char *txRates)
 **************************************************************************
 *  \brief set Basic Data Transmit Rates as Comma-separated list of the maximum access point
 *  data transmit rates in Mbps for unicast, multicast and broadcast frames.
 *  For instance: 1,2 indicates that 1 and 2 MbPS rates are supported
 *  \param[in] int index - The index of the entry
 *  \param[in] char *txRates - the maximum access point data transmit rates in Mbps.
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setBasicDataTransmitRates(int index, char *txRates);


/**************************************************************************/
/*! \fn int wlan_getOperationalDataTransmitRates(int index, char *out_str)
 **************************************************************************
 *  \brief get Operational Data Transmit Rates as comma-separated list of the maximum access point
 *  data transmit rates in Mbps for unicast frames (a superset of BasicDataTransmitRates).
 *  \param[in] int index - The index of the entry
 *  \param[out] char *out_str - the maximum access point data transmit rates in Mbps.
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getOperationalDataTransmitRates(int index, char *txRates);


/**************************************************************************/
/*! \fn int wlan_setOperationalDataTransmitRates(int index, char *txRates)
 **************************************************************************
 *  \brief set Operational Data Transmit Rates as comma-separated list of the maximum access point
 *  data transmit rates in Mbps for unicast frames (a superset of BasicDataTransmitRates).
 *  \param[in] int index - The index of the entry
 *  \param[in] char *txRates - the maximum access point data transmit rates in Mbps.
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setOperationalDataTransmitRates(int index, char *txRates);


/**************************************************************************/
/*! \fn int wlan_getPossibleDataTransmitRates(int index, char *txRates)
 **************************************************************************
 *  \brief get Possible Data Transmit Rates.
 *  \param[in] int index - The index of the entry
 *  \param[out] char *txRates - Comma-separated list of the data transmit rates
 *  for unicast frames at which the access point will permit a station to connect
 *  (a superset of OperationalDataTransmitRates).
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getPossibleDataTransmitRates(int index, char *txRates);


/**************************************************************************/
/*! \fn int wlan_getSsidAdvertisementEnabled(int index, bool *enabled)
 **************************************************************************
 *  \brief get SSID advertisement enabled/disabled flag.
 *  \param[in] int index - The index of the entry
 *  \param[out] bool *enabled - True if SSID advertisement is enabled
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getSsidAdvertisementEnabled(int index, bool *enabled);


/**************************************************************************/
/*! \fn int wlan_setSsidAdvertisementEnabled(int index, bool enabled)
 **************************************************************************
 *  \brief set SSID advertisement enabled/disabled flag.
 *  \param[in] int index - The index of the entry
 *  \param[in] bool enabled - SSID advertisement enabled flag
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setSsidAdvertisementEnabled(int index, bool enabled);


/**************************************************************************/
/*! \fn int wlan_getRadioEnabled(int index, bool *enabled)
 **************************************************************************
 *  \brief get Radio enable flag.
 *  \param[in] int index - The index of the entry
 *  \param[out] bool *enabled - True if Radio is enabled
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getRadioEnabled(int index, bool *enabled);



/**************************************************************************/
/*! \fn int wlan_setRadioEnabled(int index, bool enabled)
 **************************************************************************
 *  \brief set Radio enable flag.
 *  \param[in] int index - The index of the entry
 *  \param[out] bool enabled - Radio enabled flag
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setRadioEnabled(int index, bool enabled);

/**************************************************************************/
/*! \fn int wlan_getRouterEnable(int index, bool * routerEnabled)
**************************************************************************
*  \brief get Router Enable
*  \param[in] int index - AP Index
*  \param[out] bool * routerEnabled - true/false
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_getRouterEnable(int index, bool * routerEnabled);


/**************************************************************************/
/*! \fn int wlan_setRouterEnable(int index, bool routerEnabled)
**************************************************************************
*  \brief set Router Enable
*  \param[in] int index - AP Index
*  \param[in] bool routerEnabled - true/false
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_setRouterEnable(int index, bool routerEnabled);
/**************************************************************************/
/*! \fn int wlan_getTransmitPowerSupported(int index, char *power)
 **************************************************************************
 *  \brief get supported transmit power levels as percentages of full power.
 *  Each value MUST be an integer in the range 0 to 100 inclusive. For example, 0,25,50,75,100.
 *  \param[in] int index - The index of the entry
 *  \param[out] char *power - Comma-separated list of the supported transmit power levels
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getTransmitPowerSupported(int index, char *power);


/**************************************************************************/
/*! \fn int wlan_getTransmitPower(int index, u_int_32 *power)
 **************************************************************************
 *  \brief get the current transmit power level. Indicates the current transmit power level
 *  as a percentage of full power.
 *  \param[in] int index - The index of the entry
 *  \param[out] u_int_32 *power - The current transmit power level
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getTransmitPower(int index, u_int_32 *power);


/**************************************************************************/
/*! \fn int wlan_setTransmitPower(int index, u_int_32 power)
 **************************************************************************
 *  \brief get the current transmit power level. Indicates the current transmit power level
 *  as a percentage of full power. The value MUST be one of the values reported
 *  by the TransmitPowerSupported parameter.
 *  \param[in] int index - The index of the entry
 *  \param[in] u_int_32 power - The current transmit power level
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setTransmitPower(int index, u_int_32 power);


/**************************************************************************/
/*! \fn int wlan_getAutoRateFallbackEnable(int index, bool *enable)
 **************************************************************************
 *  \brief get automatic rate fallback enable flag.
 *  \param[in] int index - The index of the entry
 *  \param[out] bool *enable - Automatic rate fallback enable flag.
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getAutoRateFallbackEnable(int index, bool *enable);


/**************************************************************************/
/*! \fn int wlan_setAutoRateFallbackEnable(int index, bool enable)
 **************************************************************************
 *  \brief set automatic rate fallback enable flag.
 *  \param[in] int index - The index of the entry
 *  \param[in] bool enable - Automatic rate fallback enable flag.
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setAutoRateFallbackEnable(int index, bool enable);

/* Output: a structure of WLAN logical network basic statistics. */
int
#if defined(WLAN_STANDALONE_TEST) || !defined YOCTO
wlan_getBasicStats(int index, void *stats);
#else
wlan_getBasicStats(int index, wlan_basicStats_t *stats);
#endif


/**************************************************************************/
/*! \fn int wlan_getStats(int index, wlan_stats_t *stats)
 **************************************************************************
 *  \brief get WLAN logical network basic statistics.
 *  \param[in] int index - The index of the entry
 *  \param[out] wlan_stats_t *stats -  a structure of WLAN logical network statistics.
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int
#if defined(WLAN_STANDALONE_TEST) || !defined YOCTO
wlan_getStats(int index, void *stats);
#else
wlan_getStats(int index, wlan_stats_t *stats);
#endif


/**************************************************************************/
/*! \fn int wlan_getAssocDevicesNum(int index, u_int_32 *devNum)
 **************************************************************************
 *  \brief get number of WLAN associated devices.
 *  \param[in] int index - The index of the entry
 *  \param[out] u_int_32 *devNum -  number of associated devices.
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getAssocDevicesNum(int index, u_int_32 *devNum);


/**************************************************************************/
/*! \fn int wlan_getAssocDevice(int index, int devIndex, wlan_device_t *dev)
 **************************************************************************
 *  \brief get WLAN associated device.
 *  \param[in] int index - The index of the entry
 *  \param[in] int devIndex - The index of an associated device
 *  \param[out] wlan_device_t *dev -  a structure parameters belonging to an associated device
 *  held at index "devIndex" in AP tables.
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int
#if defined(WLAN_STANDALONE_TEST) || !defined YOCTO
wlan_getAssocDevice(int index, int devIndex, void *dev);
#else
wlan_getAssocDevice(int index, int devIndex, wlan_device_t *dev);
#endif


/**************************************************************************/
/*! \fn int wlan_getWepKey(int index, int keyIndex, char *key)
 **************************************************************************
 *  \brief get WEP key
 *  \param[in] int index - The WLAN config index
 *  \param[in] int keyIndex - Key index (1:4)
 *  \param[out] char *key -  WEP key.
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getWepKey(int index, int keyIndex, char *key);


/**************************************************************************/
/*! \fn int wlan_setWepKey(int index, int keyIndex, char *key)
 **************************************************************************
 *  \brief set WEP key
 *  \param[in] int index - The WLAN config index
 *  \param[in] int keyIndex - Key index (1:4)
 *  \param[in] char *key -  WEP key, maximum 128 characters.
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setWepKey(int index, int keyIndex, char *key);


/**************************************************************************/
/*! \fn int wlan_getPreSharedKey(int index, int keyIndex, char *psk)
 **************************************************************************
 *  \brief get Pre-shared key
 *  \param[in] int index - The WLAN config index
 *  \param[in] int keyIndex - Key index (1:4)
 *  \param[out] char *psk -  Pre-shared key represented as a hexadecimal string.
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getPreSharedKey(int index, int keyIndex, char *psk);


/**************************************************************************/
/*! \fn int wlan_setPreSharedKey(int index, int keyIndex, char *psk)
 **************************************************************************
 *  \brief set Pre-shared key
 *  \param[in] int index - The WLAN config index
 *  \param[in] int keyIndex - Key index (1:4)
 *  \param[in] char *psk -  Pre-shared key represented as a hexadecimal string.
 *  Max 64 characters
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setPreSharedKey(int index, int keyIndex, char *psk);


/**************************************************************************/
/*! \fn int wlan_getKeyPassphrase(int index, int keyIndex, char *psk)
 **************************************************************************
 *  \brief get Key passphrase
 *  \param[in] int index - The WLAN config index
 *  \param[in] int keyIndex - Key index (1:4)
 *  \param[out] char *psk -  Key passphrase string.
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getKeyPassphrase(int index, int keyIndex, char *psk);


/**************************************************************************/
/*! \fn int wlan_setKeyPassphrase(int index, int keyIndex, char *psk)
 **************************************************************************
 *  \brief set Key passphrase. Notes that setting both the Key passphrase and the Pre-Shared Key
 *  directly is not allowed and the result is undefined. Either one of the two techniques must be used.
 *  \param[in] int index - The WLAN config index
 *  \param[in] int keyIndex - Key index (1:4)
 *  \param[in] char *psk -  Key passphrase string. Max 63 characters.
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setKeyPassphrase(int index, int keyIndex, char *psk);


/**************************************************************************/
/*! \fn int wlan_getWmmSupported(int index, bool *supported)
 **************************************************************************
 *  \brief get WMM supported flag.
 *  \param[in] int index - The index of the entry
 *  \param[out] bool *supported - True if WMM is supported
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getWmmSupported(int index, bool *supported);


/**************************************************************************/
/*! \fn int wlan_getWmmUaspdSupported(int index, bool *supported)
 **************************************************************************
 *  \brief get WMM Unscheduled Automatic Power Save Delivery supported flag.
 *  \param[in] int index - The index of the entry
 *  \param[out] bool *supported - True if WMM Unscheduled Automatic Power Save Delivery is supported
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getWmmUaspdSupported(int index, bool *supported);


/**************************************************************************/
/*! \fn int wlan_getWmmEnable(int index, bool *enable)
 **************************************************************************
 *  \brief get WMM enable flag.
 *  \param[in] int index - The index of the entry
 *  \param[out] bool *enable - True if WMM is enabled
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getWmmEnable(int index, bool *enable);


/**************************************************************************/
/*! \fn int wlan_setWmmEnable(int index, bool enable)
 **************************************************************************
 *  \brief set WMM enable flag.
 *  \param[in] int index - The index of the entry
 *  \param[in] bool enable - WMM enable flag. This is valid only if WMM is supported.
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setWmmEnable(int index, bool enable);


/**************************************************************************/
/*! \fn int wlan_getWmmUaspdEnable(int index, bool *enable)
 **************************************************************************
 *  \brief get WMM Unscheduled Automatic Power Save Delivery flag.
 *  \param[in] int index - The index of the entry
 *  \param[out] bool *enable - True if WMM Unscheduled Automatic Power Save Delivery flag enabled
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getWmmUaspdEnable(int index, bool *enable);


/**************************************************************************/
/*! \fn int wlan_setWmmUaspdEnable(int index, bool enable)
 **************************************************************************
 *  \brief set WMM Unscheduled Automatic Power Save Delivery flag.
 *  \param[in] int index - The index of the entry
 *  \param[in] bool enable - WMM Automatic Power Save Delivery flag. This is valid only
 *  if WMM and UAPSD are supported.
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setWmmUaspdEnable(int index, bool enable);


/**************************************************************************/
/*! \fn int wlan_getWmmOgAifsn(int index, int class, u_int_32 *aifsn)
 **************************************************************************
 *  \brief get Arbitration Inter Frame Spacing of Outgoing Traffic.
 *  \param[in] int index - The index of the entry
 *  \param[in] int class - Priority class level (1-Best Effort, 2-Background, 3-Video, 4-Voice)
 *  \param[out] u_int_32 *aifsn -  Arbitration Inter Frame Spacing (Number)
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getWmmOgAifsn(int index, int cl, u_int_32 *aifsn);


/**************************************************************************/
/*! \fn int wlan_setWmmOgAifsn(int index, int class, u_int_32 aifsn)
 **************************************************************************
 *  \brief set Arbitration Inter Frame Spacing of Outgoing Traffic.
 *  \param[in] int index - The index of the entry
 *  \param[in] int class - Priority class level (1-Best Effort, 2-Background, 3-Video, 4-Voice)
 *  \param[in] u_int_32 aifsn -  Arbitration Inter Frame Spacing (Number), (2-15)
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setWmmOgAifsn(int index, int cl, u_int_32 aifsn);


/**************************************************************************/
/*! \fn int wlan_getWmmOgEcwMin(int index, int class, u_int_32 *ecwMin)
 **************************************************************************
 *  \brief get Exponent of Contention Window (Minimum) of Outgoing Traffic.
 *  \param[in] int index - The index of the entry
 *  \param[in] int class - Priority class level (1-Best Effort, 2-Background, 3-Video, 4-Voice)
 *  \param[out] u_int_32 *ecwMin -  Exponent of Contention Window (Minimum)
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getWmmOgEcwMin(int index, int cl, u_int_32 *ecwMin);


/**************************************************************************/
/*! \fn int wlan_setWmmOgEcwMin(int index, int class, u_int_32 ecwMin)
 **************************************************************************
 *  \brief set Exponent of Contention Window (Minimum) of Outgoing Traffic.
 *  \param[in] int index - The index of the entry
 *  \param[in] int class - Priority class level (1-Best Effort, 2-Background, 3-Video, 4-Voice)
 *  \param[in] u_int_32 ecwMin -  Exponent of Contention Window (Minimum), (0-15)
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setWmmOgEcwMin(int index, int cl, u_int_32 ecwMin);


/**************************************************************************/
/*! \fn int wlan_getWmmOgEcwMax(int index, int class, u_int_32 *ecwMax)
 **************************************************************************
 *  \brief get Exponent of Contention Window (Maximum) of Outgoing Traffic.
 *  \param[in] int index - The index of the entry
 *  \param[in] int class - Priority class level (1-Best Effort, 2-Background, 3-Video, 4-Voice)
 *  \param[out] u_int_32 *ecwMax - Exponent of Contention Window (Maximum)
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getWmmOgEcwMax(int index, int cl, u_int_32 *ecwMax);


/**************************************************************************/
/*! \fn int wlan_setWmmOgEcwMax(int index, int class, u_int_32 ecwMax)
 **************************************************************************
 *  \brief set Exponent of Contention Window (Maximum) of Outgoing Traffic.
 *  \param[in] int index - The index of the entry
 *  \param[in] int class - Priority class level (1-Best Effort, 2-Background, 3-Video, 4-Voice)
 *  \param[in] u_int_32 ecwMax -  Exponent of Contention Window (Maximum), (0-15). Must be >= EcwMin.
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setWmmOgEcwMax(int index, int cl, u_int_32 ecwMax);


/**************************************************************************/
/*! \fn int wlan_getWmmOgTxOp(int index, int class, u_int_32 *txOp)
 **************************************************************************
 *  \brief get Transmit Opportunity of Outgoing Traffic.
 *  \param[in] int index - The index of the entry
 *  \param[in] int class - Priority class level (1-Best Effort, 2-Background, 3-Video, 4-Voice)
 *  \param[out] u_int_32 *txOp - Transmit Opportunity, in multiples of 32 microseconds
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getWmmOgTxOp(int index, int cl, u_int_32 *txOp);


/**************************************************************************/
/*! \fn int wlan_setWmmOgTxOp(int index, int class, u_int_32 txOp)
 **************************************************************************
 *  \brief set Transmit Opportunity of Outgoing Traffic.
 *  \param[in] int index - The index of the entry
 *  \param[in] int class - Priority class level (1-Best Effort, 2-Background, 3-Video, 4-Voice)
 *  \param[in] u_int_32 txOp -  Transmit Opportunity, in multiples of 32 microseconds
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setWmmOgTxOp(int index, int cl, u_int_32 txOp);


/**************************************************************************/
/*! \fn int wlan_getWmmOgAckPolicy(int index, int class, bool *policy)
 **************************************************************************
 *  \brief get Ack Policy of Outgoing Traffic.
 *  \param[in] int index - The index of the entry
 *  \param[in] int class - Priority class level (1-Best Effort, 2-Background, 3-Video, 4-Voice)
 *  \param[out] bool *policy -  Ack Policy, where False=Do Not Acknowledge and True=Acknowledge
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getWmmOgAckPolicy(int index, int cl, bool *policy);


/**************************************************************************/
/*! \fn int wlan_setWmmOgAckPolicy(int index, int class, bool policy)
 **************************************************************************
 *  \brief set Ack Policy of Outgoing Traffic.
 *  \param[in] int index - The index of the entry
 *  \param[in] int class - Priority class level (1-Best Effort, 2-Background, 3-Video, 4-Voice)
 *  \param[in] bool policy -  Ack Policy, where False=Do Not Acknowledge and True=Acknowledge
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setWmmOgAckPolicy(int index, int cl, bool policy);


/**************************************************************************/
/*! \fn int wlan_getWmmIcAifsn(int index, int class, u_int_32 *aifsn)
 **************************************************************************
 *  \brief get Arbitration Inter Frame Spacing of Incoming Traffic.
 *  \param[in] int index - The index of the entry
 *  \param[in] int class - Priority class level (1-Best Effort, 2-Background, 3-Video, 4-Voice)
 *  \param[out] u_int_32 *aifsn -  Arbitration Inter Frame Spacing (Number)
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getWmmIcAifsn(int index, int cl, u_int_32 *aifsn);


/**************************************************************************/
/*! \fn int wlan_setWmmIcAifsn(int index, int class, u_int_32 aifsn)
 **************************************************************************
 *  \brief set Arbitration Inter Frame Spacing of Incoming Traffic.
 *  \param[in] int index - The index of the entry
 *  \param[in] int class - Priority class level (1-Best Effort, 2-Background, 3-Video, 4-Voice)
 *  \param[in] u_int_32 aifsn -  Arbitration Inter Frame Spacing (Number), (2-15)
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setWmmIcAifsn(int index, int cl, u_int_32 aifsn);


/**************************************************************************/
/*! \fn int wlan_getWmmIcEcwMin(int index, int class, u_int_32 *ecwMin)
 **************************************************************************
 *  \brief get Exponent of Contention Window (Minimum) of Incoming Traffic.
 *  \param[in] int index - The index of the entry
 *  \param[in] int class - Priority class level (1-Best Effort, 2-Background, 3-Video, 4-Voice)
 *  \param[out] u_int_32 *ecwMin -  Exponent of Contention Window (Minimum)
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getWmmIcEcwMin(int index, int cl, u_int_32 *ecwMin);


/**************************************************************************/
/*! \fn int wlan_setWmmIcEcwMin(int index, int class, u_int_32 ecwMin)
 **************************************************************************
 *  \brief set Exponent of Contention Window (Minimum) of Incoming Traffic.
 *  \param[in] int index - The index of the entry
 *  \param[in] int class - Priority class level (1-Best Effort, 2-Background, 3-Video, 4-Voice)
 *  \param[in] u_int_32 ecwMin -  Exponent of Contention Window (Minimum), (0-15)
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setWmmIcEcwMin(int index, int cl, u_int_32 ecwMin);


/**************************************************************************/
/*! \fn int wlan_getWmmIcEcwMax(int index, int class, u_int_32 *ecwMax)
 **************************************************************************
 *  \brief get Exponent of Contention Window (Maximum) of Incoming Traffic.
 *  \param[in] int index - The index of the entry
 *  \param[in] int class - Priority class level (1-Best Effort, 2-Background, 3-Video, 4-Voice)
 *  \param[out] u_int_32 *ecwMax -  Exponent of Contention Window (Maximum)
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getWmmIcEcwMax(int index, int cl, u_int_32 *ecwMax);


/**************************************************************************/
/*! \fn int wlan_setWmmIcEcwMax(int index, int class, u_int_32 ecwMax)
 **************************************************************************
 *  \brief set Exponent of Contention Window (Maximum) of Incoming Traffic.
 *  \param[in] int index - The index of the entry
 *  \param[in] int class - Priority class level (1-Best Effort, 2-Background, 3-Video, 4-Voice)
 *  \param[in] u_int_32 ecwMax - Exponent of Contention Window (Maximum), (0-15). Must be >= EcwMin.
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setWmmIcEcwMax(int index, int cl, u_int_32 ecwMax);


/**************************************************************************/
/*! \fn int wlan_getWmmIcTxOp(int index, int class, u_int_32 *txOp)
 **************************************************************************
 *  \brief get Transmit Opportunity of Incoming Traffic.
 *  \param[in] int index - The index of the entry
 *  \param[in] int class - Priority class level (1-Best Effort, 2-Background, 3-Video, 4-Voice)
 *  \param[out] u_int_32 *txOp -  Transmit Opportunity, in multiples of 32 microseconds
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getWmmIcTxOp(int index, int cl, u_int_32 *txOp);


/**************************************************************************/
/*! \fn int wlan_setWmmIcTxOp(int index, int class, u_int_32 txOp)
 **************************************************************************
 *  \brief set Arbitration Inter Frame Spacing of Incoming Traffic.
 *  \param[in] int index - The index of the entry
 *  \param[in] int class - Priority class level (1-Best Effort, 2-Background, 3-Video, 4-Voice)
 *  \param[in] u_int_32 txOp - Transmit Opportunity, in multiples of 32 microseconds
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setWmmIcTxOp(int index, int cl, u_int_32 txOp);


/**************************************************************************/
/*! \fn int wlan_getWmmIcAckPolicy(int index, int class, bool *policy)
 **************************************************************************
 *  \brief get Ack Policy of Incoming Traffic.
 *  \param[in] int index - The index of the entry
 *  \param[in] int class - Priority class level (1-Best Effort, 2-Background, 3-Video, 4-Voice)
 *  \param[out] bool *policy - Ack Policy, where False=Do Not Acknowledge and True=Acknowledge
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getWmmIcAckPolicy(int index, int cl, bool *policy);


/**************************************************************************/
/*! \fn int wlan_setWmmIcAckPolicy(int index, int class, bool policy)
 **************************************************************************
 *  \brief set Ack Policy of Incoming Traffic.
 *  \param[in] int index - The index of the entry
 *  \param[in] int class - Priority class level (1-Best Effort, 2-Background, 3-Video, 4-Voice)
 *  \param[in] bool policy - Ack Policy, where False=Do Not Acknowledge and True=Acknowledge
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setWmmIcAckPolicy(int index, int cl, bool policy);


/**************************************************************************/
/*! \fn int wlan_getWpsEnable(int index, bool *enable)
 **************************************************************************
 *  \brief get WPS enable flag.
 *  \param[in] int index - The index of the entry
 *  \param[out] bool *enable - True if WPS enabled
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getWpsEnable(int index, bool *enable);


/**************************************************************************/
/*! \fn int wlan_setWpsEnable(int index, bool enable)
 **************************************************************************
 *  \brief set WPS enable flag.
 *  \param[in] int index - The index of the entry
 *  \param[in] bool enable - True if WPS enabled
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setWpsEnable(int index, bool enable);


/**************************************************************************/
/*! \fn int wlan_getWpsDeviceName(int radioIndex, char *devName)
 **************************************************************************
 *  \brief get WPS device name.
 *  \param[in] int radioIndex - The index of the entry
 *  \param[out] char *devName - User-friendly description of the device
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getWpsDeviceName(int radioIndex, char *devName);


/**************************************************************************/
/*! \fn int wlan_getWpsDevicePassword(int radioIndex, u_int_32 *password)
 **************************************************************************
 *  \brief get WPS Device Password (PIN).
 *  \param[in] int radioIndex - The index of the entry
 *  \param[out] u_int_32 *password - device password (PIN)
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getWpsDevicePassword(int radioIndex, u_int_32 *password);


/**************************************************************************/
/*! \fn int wlan_setWpsDevicePassword(int index, u_int_32 password)
 **************************************************************************
 *  \brief get WPS Device Password (PIN).
 *  \param[in] int index - The index of the entry
 *  \param[in] u_int_32 password - device password (PIN)
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setWpsDevicePassword(int index, u_int_32 password);


/**************************************************************************/
/*! \fn int wlan_getWpsUuid(int radioIndex, char *uuid)
 **************************************************************************
 *  \brief get Device UUID. UUID represented as specified in RFC 4122 [48],
 *  but omitting the leading urn:uuid:, e.g. f81d4fae-7dec-11d0-a765-00a0c91e6bf6.
 *  \param[in] int radioIndex - The index of the entry
 *  \param[out] char *uuid - Device UUID
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getWpsUuid(int radioIndex, char *uuid);


/**************************************************************************/
/*! \fn int wlan_getWpsVersion(int index, u_int_32 *ver)
 **************************************************************************
 *  \brief get the WiFi Protected Setup version supported by the device.
 *  \param[in] int index - The index of the entry
 *  \param[out] u_int_32 *ver - The WiFi Protected Setup version
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getWpsVersion(int index, u_int_32 *ver);


/**************************************************************************/
/*! \fn int wlan_getWpsConfigMethodsSupported(int index, char *methods)
 **************************************************************************
 *  \brief get WPS configuration methods supported by the device as comma-separated list
 *  of the WPS configuration methods supported by the device. Each entry in the list is an enumeration of:
 *  "USBFlashDrive", "Ethernet", "Label ", "Display", "ExternalNFCToken", "IntegratedNFCToken",
 *  "NFCInterface", "PushButton", "Keypad"
 *  \param[in] int index - The index of the entry
 *  \param[out] char *methods - The WPS configuration methods supported by the device.
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getWpsConfigMethodsSupported(int index, char *methods);


/**************************************************************************/
/*! \fn int wlan_getWpsConfigMethodsEnabled(int radioIndex, char *methods)
 **************************************************************************
 *  \brief get WPS enabled configuration methods as comma-separated list of
 *  the WPS configuration methods currently enabled by the device.
 *  \param[in] int radioIndex - The index of the entry
 *  \param[out] char *methods - The WPS enabled configuration methods.
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getWpsConfigMethodsEnabled(int radioIndex, char *methods);


/**************************************************************************/
/*! \fn int wlan_setWpsConfigMethodsEnabled(int radioIndex, char *methods)
 **************************************************************************
 *  \brief set WPS enabled configuration methods as comma-separated list of
 *  the WPS configuration methods currently enabled by the device.
 *  \param[in] int radioIndex - The index of the entry
 *  \param[out] char *methods - The WPS enabled configuration methods.
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setWpsConfigMethodsEnabled(int radioIndex, char *methods);


/**************************************************************************/
/*! \fn int wlan_getWpsSetupLockedState(int index, char *state)
 **************************************************************************
 *  \brief get WPS Setup Locked State as either one of "Unlocked", "LockedByLocalManagement",
 *  "LockedByRemoteManagement", "PINRetryLimitReached" values
 *  \param[in] int index - The index of the entry
 *  \param[out] char *state - The WPS Setup Locked State
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getWpsSetupLockedState(int index, char *state);


/**************************************************************************/
/*! \fn int wlan_getWpsSetupLock(int index, bool *lock)
 **************************************************************************
 *  \brief get WPS Setup Lock value. True - the Access Point will refuse
 *  to accept new external registrars. False - the Access Point is enabled
 *  for configuration through an external registrar.
 *  \param[in] int index - The index of the entry
 *  \param[out] bool *lock - The WPS Setup Lock value
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getWpsSetupLock(int index, bool *lock);


/**************************************************************************/
/*! \fn int wlan_setWpsSetupLock(int index, bool lock)
 **************************************************************************
 *  \brief set WPS Setup Lock value. When set to True, the Access Point will refuse
 *  to accept new external registrars; already established registrars will continue
 *  to be able to add new enrollees (the Setup Locked State becomes "LockedByRemoteManagement").
 *  When set to False, the Access Point is enabled for configuration through an external
 *  registrar (the Setup Locked State becomes "Unlocked").
 *  \param[in] int index - The index of the entry
 *  \param[in] bool lock - The WPS Setup Lock value
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setWpsSetupLock(int index, bool lock);


/**************************************************************************/
/*! \fn int wlan_getWpsConfigurationState(int radioIndex, char *state)
 **************************************************************************
 *  \brief get WPS Configuration State as either one of "Not configured"
 *  (WLAN interface is unconfigured: out-of-the box configuration) or
 *  "Configured" (WLAN interface is configured) values
 *  \param[in] int radioIndex - The index of the entry
 *  \param[out] char *state - The WPS Configuration State
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getWpsConfigurationState(int radioIndex, char *state);


/**************************************************************************/
/*! \fn int wlan_getWpsLastConfigurationError(int index, char *error)
 **************************************************************************
 *  \brief get WPS Last Configuration Error as either one of "NoError", "DecryptionCRCFailure",
 *  "SignalTooWeak", "CouldntConnectToRegistrar", "RogueActivitySuspected", "DeviceBusy",
 *  "SetupLocked", "MessageTimeout", "RegistrationSessionTimeout", "DevicePasswordAuthFailure" values
 *  \param[in] int index - The index of the entry
 *  \param[out] char *error - The WPS Last Configuration Error
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getWpsLastConfigurationError(int index, char *error);


/**************************************************************************/
/*! \fn int wlan_getWpsRegistrarNumEntries(int index, u_int_32 *numEntries)
 **************************************************************************
 *  \brief get Number of Registrars.
 *  \param[in] int index - The index of the entry
 *  \param[out] u_int_32 *numEntries - The number of Registrars.
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getWpsRegistrarNumEntries(int index, u_int_32 *numEntries);


int wlan_addWpsRegistrarEntry(int index, unsigned int *numEntries);


int wlan_delWpsRegistrarEntry(int index, int registrarIndex, int *numEntries);


/**************************************************************************/
/*! \fn int wlan_getWpsRegistrarEstablished(int index, bool *established)
 **************************************************************************
 *  \brief get Registrar established flag.
 *  \param[in] int index - The index of the entry
 *  \param[out] bool *established - Registrar established flag.
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getWpsRegistrarEstablished(int index, bool *established);


/**************************************************************************/
/*! \fn int wlan_getWpsRegistrarEnabled(int index, int registrarIndex, bool *enabled)
 **************************************************************************
 *  \brief get Registrar enabled flag.
 *  \param[in] int index - The index of the WLAN entry
 *  \param[in] int registrarIndex - The index of the Registrar entry
 *  \param[out] bool *enabled - Registrar enabled flag.
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getWpsRegistrarEnabled(int index, int registrarIndex, bool *enabled);


/**************************************************************************/
/*! \fn int wlan_setWpsRegistrarEnabled(int index, int registrarIndex, bool enabled)
 **************************************************************************
 *  \brief set Registrar enabled flag.
 *  \param[in] int index - The index of the WLAN entry
 *  \param[in] int registrarIndex - The index of the Registrar entry
 *  \param[in] bool enabled - Registrar enabled flag.
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setWpsRegistrarEnabled(int index, int registrarIndex, bool enabled);


/**************************************************************************/
/*! \fn int wlan_getWpsRegistrarUuid(int index, int registrarIndex, char *uuid)
 **************************************************************************
 *  \brief get UUID of the registrar. This is represented as specified in RFC 4122 [48]
 *  but omitting the leading urn:uuid:, e.g. f81d4fae-7dec-11d0-a765-00a0c91e6bf6.
 *  \param[in] int index - The index of the WLAN entry
 *  \param[in] int registrarIndex - The index of the Registrar entry
 *  \param[out] char *uuid - UUID of the registrar.
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getWpsRegistrarUuid(int index, int registrarIndex, char *uuid);


/**************************************************************************/
/*! \fn int wlan_getWpsRegistrarDeviceName(int index, int registrarIndex, char *name)
 **************************************************************************
 *  \brief get Device Name of the registrar.
 *  \param[in] int index - The index of the WLAN entry
 *  \param[in] int registrarIndex - The index of the Registrar entry
 *  \param[out] char *name - Device Name of the registrar.
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getWpsRegistrarDeviceName(int index, int registrarIndex, char *devName);


int wlan_setWpsPbcTrigger(int index);


/**************************************************************************/
/*! \fn int wlan_getGreenApPowerSave(int index, bool *enabled, u_int_32 *timeout)
 **************************************************************************
 *  \brief get Green AP Power Save mode.
 *  \param[in] int index - The index of the entry
 *  \param[out] bool *enabled - True if Green AP Power Save mode enabled
 *  \param[out] u_int_32 *timeout - Timeout value
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getGreenApPowerSave(int index, bool *enabled, u_int_32 *timeout);


/**************************************************************************/
/*! \fn int wlan_setGreenApPowerSave(int index, bool enable, u_int_32 timeout)
 **************************************************************************
 *  \brief set Green AP Power Save mode.
 *  \param[in] int index - The index of the entry
 *  \param[in] bool enable - True for Green AP Power Save mode enable
 *  \param[in] u_int_32 timeout - Timeout value
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setGreenApPowerSave(int index, bool enable, u_int_32 timeout);


int wlan_setWpsEnrolleePin(int radioIndex, char *pin);


int wlan_getSsidQosEnabled(int radio, bool *enable);


int wlan_setSsidQosEnabled(int radio, bool enabled);


int wlan_getSsidQosLevel(int index, char *lvl);


int wlan_setSsidQosLevel(int index, char *lvl);

/**************************************************************************/
/*! \fn int wlan_createVap(int vap,int radio, char *essid, bool hideSsid)
 **************************************************************************
 *  \brief create virtual access point
 *  \param[in] int vap - virtual access point
 *  \param[in] int radio - Radio Index
 *  \param[in] char *essid - SSID Name
 *  \param[in] bool hideSsid - true/false
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_createVap(int vap, int radio, char *essid, bool hideSsid);

/**************************************************************************/
/*! \fn int wlan_deleteVap(int vap)
 **************************************************************************
 *  \brief delete virtual access point
 *  \param[in] int vap - virtual access point
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_deleteVap(int vap);

/**************************************************************************/
/*! \fn int wlan_factoryReset(void)
 **************************************************************************
 *  \brief  Factory Reset Wifi.
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_factoryReset(void);

/* @} */

/** \defgroup FAPI_WLAN_ABSTRACTION_RDKB FAPI Abstraction layer for RDKB
*   @ingroup FAPI_WLAN
\brief It provides a set of APIs to perform common operations on WLAN configuration supporting RDKB.
*/

/** \addtogroup FAPI_WLAN_ABSTRACTION_RDKB */
/* @{ */

/**************************************************************************/
/*!
**************************************************************************
*  \brief  Wi-Fi Device Structure
***************************************************************************/
typedef struct _wifi_device
{
	unsigned char wifi_devMacAddress[6]; /*!<  Device MAC  */
	char wifi_devIPAddress[64]; /*!<  Device IP  */
	bool wifi_devAssociatedDeviceAuthentiationState;  /*!<  Device Associated Device Authentication State  */
	int wifi_devSignalStrength;  /*!<  Device Signal Strength  */
	int wifi_devTxRate;  /*!<  Device TX Rate  */
	int wifi_devRxRate;  /*!<  Device RX Rate  */
} wifi_device_t;

/**************************************************************************/
/*! \fn int wlan_down(void)
 **************************************************************************
 *  \brief  Put the AP in down.
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_down(void);


/**************************************************************************/
/*! \fn int wlan_createCfgFile(void)
 **************************************************************************
 *  \brief  creates wifi configuration files.
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_createCfgFile(void);


/**************************************************************************/
/*! \fn int wlan_initRadio(int index)
 **************************************************************************
 *  \brief initializes the specified radio.
 *  \param[in] int index - The index of the entry
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_initRadio(int index);


/**************************************************************************/
/*! \fn int wlan_getActiveAthIndex(int radioIndex, int *activeIndex)
 **************************************************************************
 *  \brief Find the first ath Index that is up on the given radio.
 *  \param[in] int radioIndex - The index of the entry
 *  \param[out] bool *activeIndex - True if radio is enabled
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getActiveAthIndex(int radioIndex, int *activeIndex);



/**************************************************************************/
/*! \fn int wlan_setDTIMInterval(int index, int dtimPeriod)
 **************************************************************************
 *  \brief set DTIM Interval
 *  \param[in] int index - AP index
 *  \param[in] int dtimPeriod - value of dtim interval
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setDTIMInterval(int index, int dtimPeriod);


/**************************************************************************/
/*! \fn int wlan_setCtsProtectionEnable(int index, bool enabled)
 **************************************************************************
 *  \brief set CTS Protection
 *  \param[in] int index - AP index
 *  \param[in] int enable - true to enable false to disable
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setCtsProtectionEnable(int index, bool enabled);


/**************************************************************************/
/*! \fn int wlan_getChannelMode(int index, char *channelMode)
 **************************************************************************
 *  \brief get Channel Mode
 *  \param[in] int index - AP index
 *  \param[out] char *channelMode - channel Mode
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getChannelMode(int index, char *channelMode);


/**************************************************************************/
/*! \fn int wlan_getAllAssociatedDeviceDetail(int apIndex, unsigned long *output_ulong, wifi_device_fapi_t **output_struct);
**************************************************************************
*  \brief get Channel Mode
*  \param[in] int index - AP index
*  \param[out] unsigned long *output_ulong - number of fields
*	\param[out] wifi_device_t **output_struct - struct to hold stations info
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_getAllAssociatedDeviceDetail(int apIndex, unsigned long *output_ulong, wifi_device_fapi_t ***output_struct);


/**************************************************************************/
/*! \fn int wlan_getAssociatedDeviceDetail(int apIndex, int devIndex, wifi_device_fapi_t *output_struct);
**************************************************************************
*  \brief get Channel Mode
*  \param[in] int index - AP index
*  \param[out] int devIndex - device index
*	\param[out] wifi_device_fapi_t *output_struct - struct to hold stations info
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_getAssociatedDeviceDetail(int apIndex, int devIndex, wifi_device_fapi_t *output_struct);

/**************************************************************************/
/*! \fn int wlan_setChannelMode(int index, char *channelMode,bool gOnlyFlag,
 * 				bool nOnlyFlag,bool acOnlyFlag)
 **************************************************************************
 *  \brief get Channel Mode
 *  \param[in] int index - AP index
 *  \param[in] char *channelMode - channel Mode
 *  \param[in] bool gOnlyFlag
 *  \param[in] bool nOnlyFlag
 *  \param[in] bool acOnlyFlag
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setChannelMode(int index, char *channelMode,
	bool gOnlyFlag, bool nOnlyFlag, bool acOnlyFlag);


/**************************************************************************/
/*! \fn int wlan_getCountryCode(int index, char *code)
 **************************************************************************
 *  \brief get CountryCode
 *  \param[in] int index - Radio index
 *  \param[out] char *code - country code
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getCountryCode(int index, char *code);

/**************************************************************************/
/*! \fn int wlan_setCountryCode(int index, char *code)
 **************************************************************************
 *  \brief get CountryCode
 *  \param[in] int index - Radio index
 *  \param[out] char *code - country code
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setCountryCode(int index, char *code);

/**************************************************************************/
/*! \fn int wlan_setSTBCEnable(int index, bool enable)
 **************************************************************************
 *  \brief set STBC
 *  \param[in] int index - AP index
 *  \param[in] bool enable - true to enable false to disable
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setSTBCEnable(int index, bool enable);


/**************************************************************************/
/*! \fn int wlan_getAMSDUEnable(int index, bool * enable)
 **************************************************************************
 *  \brief get AMSDUEnable
 *  \param[in] int index - AP index
 *  \param[out] bool *enable - true if enable false if disable
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getAMSDUEnable(int index, bool * enable);


/**************************************************************************/
/*! \fn int wlan_setSTBCEnable(int index, bool enable)
 **************************************************************************
 *  \brief set AMSDUEnable
 *  \param[in] int index - AP index
 *  \param[in] bool enable - true to enable false to disable
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setAMSDUEnable(int index, bool enable);


/**************************************************************************/
/*! \fn int wlan_getTxChainMask(int index, int *numStreams)
 **************************************************************************
 *  \brief get Tx chain mask
 *  \param[in] int index - AP index
 *  \param[out] int *numStreams
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getTxChainMask(int index, int *numStreams);


/**************************************************************************/
/*! \fn int wlan_setTxChainMask(int index, int numStreams)
 **************************************************************************
 *  \brief set Tx chain mask to hardware
 *  \param[in] int index - AP index
 *  \param[in] int numStreams
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setTxChainMask(int index, int numStreams);


/**************************************************************************/
/*! \fn int wlan_pushTxChainMask(int index, int numStreams)
 **************************************************************************
 *  \brief push Tx chain mask on hardware
 *  \param[in] int index - AP index
 *  \param[in] int numStreams
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_pushTxChainMask(int index, int numStreams);


/**************************************************************************/
/*! \fn int wlan_getRxChainMask(int index, int *numStreams)
 **************************************************************************
 *  \brief get Tx chain mask
 *  \param[in] int index - AP index
 *  \param[out] int *numStreams
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getRxChainMask(int index, int *numStreams);


/**************************************************************************/
/*! \fn int wlan_setRxChainMask(int index, int numStreams)
 **************************************************************************
 *  \brief set Rx chain mask to hardware
 *  \param[in] int index - AP index
 *  \param[in] int numStreams
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setRxChainMask(int index, int numStreams);


/**************************************************************************/
/*! \fn int wlan_pushRxChainMask(int index, int numStreams)
 **************************************************************************
 *  \brief push Rx chain mask on hardware
 *  \param[in] int index - AP index
 *  \param[in] int numStreams
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_pushRxChainMask(int index, int numStreams);


/**************************************************************************/
/*! \fn int wlan_getChannelsInUse(int radioIndex, char *channelsInUse)
 **************************************************************************
 *  \brief get Channel Is in use
 *  \param[in] int RadioIndex - Radio index
 *  \param[out] char *channelsInUse
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getChannelsInUse(int radioIndex, char *channelsInUse);


/**************************************************************************/
/*! \fn int wlan_getIndexFromName(char *inputAthName, int *sSIDIndex)
 **************************************************************************
 *  \brief get Index Number from Ath Name
 *  \param[in] char *inputAthName - Ath Interface Name
 *  \param[out] int *sSIDIndex - Index Number of Ath Interface
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getIndexFromName(char *inputAthName, int *sSIDIndex);


/**************************************************************************/
/*! \fn int wlan_setBeaconInterval(int index, int bintval)
 **************************************************************************
 *  \brief Set Beacon Interval
 *  \param[in] int index - Index Number
 *  \param[in] int bintval - Beacon Interval
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setBeaconInterval(int index, int bintval);

/**************************************************************************/
/*! \fn int wlan_setRadioBeaconPeriod(int index, int bintval)
**************************************************************************
*  \brief Set radio Beacon Interval
*  \param[in] int index - Index Number
*  \param[in] int bintval - Beacon Interval
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_setRadioBeaconPeriod(int index, int bintval);

/**************************************************************************/
/*! \fn int wlan_getRadioBeaconPeriod(int index, int *bintval)
**************************************************************************
*  \brief Get radio Beacon Interval
*  \param[in] int index - Index Number
*  \param[in] int bintval - Beacon Interval
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_getRadioBeaconPeriod(int index, int *bintval);

/**************************************************************************/
/*! \fn int wlan_setMaxStations(int index, int maxStations)
 **************************************************************************
 *  \brief Set Max supported station for Radio Index
 *  \param[in] int index - Index Number
 *  \param[in] unsigned int maxStations - Max Number of stations supported
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setMaxStations(int index, unsigned int maxStations);


/**************************************************************************/
/*! \fn int wlan_getMaxStations(int index, int maxStations)
**************************************************************************
*  \brief Set Max supported station for Radio Index
*  \param[in] int index - Index Number
*  \param[in] unsigned int maxStations - Max Number of stations supported
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_getMaxStations(int index, unsigned int *maxStations);


/**************************************************************************/
/*! \fn int wlan_setWpsDevicePIN(int radioIndex, unsigned long pin);
**************************************************************************
*  \brief Get WPS device PIN
*  \param[in] int radioIndex - Index Number
*  \param[in] unsigned int pin - pin number
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_setWpsDevicePIN(int radioIndex, unsigned long pin);


/**************************************************************************/
/*! \fn int wlan_getWpsDevicePIN(int radioIndex, unsigned long *output_ulong);
**************************************************************************
*  \brief Set WPS device PIN
*  \param[in] int radioIndex - Index Number
*  \param[in] unsigned int *output_ulong - pin number holder
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_getWpsDevicePIN(int apIndex, unsigned long *output_ulong);


/**************************************************************************/
/*! \fn int wlan_scanApChannels(int radioIndex, char *scanData)
 **************************************************************************
 *  \brief scans the Access Point channels
 *  \param[in] int radioIndex - Radio Index Number
 *  \param[out] char *scanData  - scanned data
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_scanApChannels(int radioIndex, char *scanData);


/**************************************************************************/
/*! \fn int wlan_getSupportedFrequencyBands(int radioIndex,
 *                                                    char *frequencyBands)
 **************************************************************************
 *  \brief get the supported frequency band
 *  \param[in] int radioIndex - Radio Index Number
 *  \param[out] char *frequencyBands - frequency Bands
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getSupportedFrequencyBands(int radioIndex, char *frequencyBands);


/**************************************************************************/
/*! \fn int wlan_pushDefaultValues(int index)
 **************************************************************************
 *  \brief push Default values to the hardware
 *  \param[in] int index - Radio Index Number
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_pushDefaultValues(int index);


/**************************************************************************/
/*! \fn int wlan_pushChannel(int athIndex, unsigned int channel)
 **************************************************************************
 *  \brief push channel No.
 *  \param[in] int index - Radio Index Number
 *  \param[in] unsigned int channel - channel No
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_pushChannel(int athIndex, unsigned int channel);


/**************************************************************************/
/*! \fn int wlan_pushSSID(int index, char *ssid)
 **************************************************************************
 *  \brief push SSID to hardware
 *  \param[in] int index - AP Index
 *  \param[in] char *ssid - SSID
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_pushSSID(int index, char *ssid);


/**************************************************************************/
/*! \fn int wlan_pushChannelMode(int athIndex)
 **************************************************************************
 *  \brief push channel mode to hardware
 *  \param[in] int index - AP Index
 *  \param[in] char *ssid - SSID
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_pushChannelMode(int athIndex);


/**************************************************************************/
/*! \fn int wlan_setObssCoexistenceEnable(int index, bool enabled)
 **************************************************************************
 *  \brief Enable / Disable Obss Coexistence
 *  \param[in] int index - AP Index
 *  \param[in] bool enable
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setObssCoexistenceEnable(int index, bool enabled);


/**************************************************************************/
/*! \fn int wlan_setRTSThresh(int index, unsigned int threshold)
 **************************************************************************
 *  \brief set RTS Threshold
 *  \param[in] int index - AP Index
 *  \param[in] unsigned int threshold - value of RTS threshold to be set
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setRTSThresh(int index, unsigned int threshold);





/**************************************************************************/
/*! \fn int wlan_addAclDevice(int index, char *dev)
 **************************************************************************
 *  \brief add device to list of acl devices
 *  \param[in] int index - AP Index
 *  \param[in] char *dev - Dev Mac address
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_addAclDevice(int index, char *dev);


/**************************************************************************/
/*! \fn int wlan_delAclDevice(int index, char *dev)
 **************************************************************************
 *  \brief delete device from list of acl devices
 *  \param[in] int index - AP Index
 *  \param[in] char *dev - Dev Mac address
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_delAclDevice(int index, char *dev);


/**************************************************************************/
/*! \fn int wlan_setGuardInterval(int index, char* guardInterval)
 **************************************************************************
 *  \brief set Guard Interval
 *  \param[in] int index - AP Index
 *  \param[in] char* guardInterval - guard Interval
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setGuardInterval(int index, char* guardInterval);

/**************************************************************************/
/*! \fn int wlan_setShortGuardInterval(int index, bool enable)
**************************************************************************
*  \brief set Guard Interval
*  \param[in] int index - AP Index
*  \param[in] bool enable - true/false
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_setShortGuardInterval(int index, bool enable);



/**************************************************************************/
/*! \fn int wlan_getRadioIEEE80211hEnabled(int radio, bool *enable)
**************************************************************************
*  \brief get RadioIEEE80211hEnabled field
*  \param[in] int index - AP Index
*  \param[in] bool enable - true/false
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_getRadioIEEE80211hEnabled(int radio, bool *enable);

/**************************************************************************/
/*! \fn int wlan_getRadioIEEE80211hSupported(int radio, bool *enable)
**************************************************************************
*  \brief get RadioIEEE80211hSupported field
*  \param[in] int index - AP Index
*  \param[in] bool enable - true/false
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_getRadioIEEE80211hSupported(int radio, bool *enable);

/**************************************************************************/
/*! \fn int wlan_getGuardInterval(int index, char* guardInterval)
**************************************************************************
*  \brief get Guard Interval
*  \param[in] int index - AP Index
*  \param[in] char* guardInterval - guard Interval
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_getGuardInterval(int index, char* guardInterval);

/**************************************************************************/
/*! \fn int wlan_setBridgeName(int index, char *bridge)
 **************************************************************************
 *  \brief get Bridge Information
 *  \param[in] int index - AP Index
 *  \param[out] char * bridge - bridge Name
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setBridgeName(int index, char *bridge);

/**************************************************************************/
/*! \fn int wlan_getBridgeName(int index, char *bridge)
 **************************************************************************
 *  \brief get Bridge Information
 *  \param[in] int index - AP Index
 *  \param[out] char * bridge - bridge Name
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getBridgeName(int index, char *bridge);

/**************************************************************************/
/*! \fn int wlan_getVlanId(int index, int *vlanId)
 **************************************************************************
 *  \brief get vlanID associated with ap index
 *  \param[in] int index - AP Index
 *  \param[out] int *vlanId - vlan ID
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
	int wlan_getVlanId(int index, int *vlanId);


/**************************************************************************/
/*! \fn int wlan_getBridgeInfo(int index, char *bridge, char *ip, char *subnet)
 **************************************************************************
 *  \brief get Bridge Information
 *  \param[in] int index - AP Index
 *  \param[out] char * bridge - bridge Name
 *  \param[out] char * ip - IP address
 *  \param[out] char * subnet - subnet mask
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getBridgeInfo(int index, char *bridge, char *ip, char *subnet);


/**************************************************************************/
/*! \fn int wlan_pushBridgeInfo(int index)
 **************************************************************************
 *  \brief push Bridge Info to hardware
 *  \param[in] int index - AP Index
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_pushBridgeInfo(int index);


/**************************************************************************/
/*! \fn int wlan_setBridgeInfo(int index,char *bridge,char *ip,char *subnet)
 **************************************************************************
 *  \brief set Bridge Information
 *  \param[in] int index - AP Index
 *  \param[in] char * bridge - bridge Name
 *  \param[in] char * ip - IP address
 *  \param[in] char * subnet - subnet mask
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setBridgeInfo(int index, char *bridge, char *ip, char *subnet);


/**************************************************************************/
/*! \fn int wlan_resetVlanCfg(int index)
 **************************************************************************
 *  \brief reset Vlan Configuration
 *  \param[in] int index - AP Index
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_resetVlanCfg(int index);



/**************************************************************************/
/*! \fn int wlan_getWirelessOnOffButton(int index, bool * enable)
 **************************************************************************
 *  \brief get status of wirelessOnOffButton
 *  \param[in] int index - AP Index
 *  \param[out] bool *enable - true/false
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getWirelessOnOffButton(int index, bool * enable);


/**************************************************************************/
/*! \fn int wlan_setWirelessOnOffButton(int index, bool enable)
 **************************************************************************
 *  \brief set status of wirelessOnOffButton
 *  \param[in] int index - AP Index
 *  \param[in] bool enable - true/false
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setWirelessOnOffButton(int index, bool enable);


/**************************************************************************/
/*! \fn int wlan_getRadioIndex(int index, int *radio_idx)
 **************************************************************************
 *  \brief get Radio Index
 *  \param[in] int index - AP Index
 *  \param[in] int *radio_idx - the index of the radio it belongs
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getRadioIndex(int index, int *radio_idx);


/**************************************************************************/
/*! \fn int wlan_setRadioIndex(int index, int radio_idx)
 **************************************************************************
 *  \brief set Radio Index
 *  \param[in] int index - AP Index
 *  \param[in] int radio_idx - radio index
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setRadioIndex(int index, int radio_idx);


/**************************************************************************/
/*! \fn int wlan_removeSecFromConfFile(int vap)
 **************************************************************************
 *  \brief remove security from config file
 *  \param[in] int vap - virtual access point
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_removeSecFromConfFile(int vap);


/**************************************************************************/
/*! \fn int wlan_createSecurityFile(int vap, bool createWpsCfg)
 **************************************************************************
 *  \brief create security file
 *  \param[in] int vap- virtual access point
 *  \param[in] bool createWpsCfg - true/false
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_createSecurityFile(int vap, bool createWpsCfg);


/**************************************************************************/
/*! \fn int wlan_setEncryptionOff(int vap)
 **************************************************************************
 *  \brief set Encryption off
 *  \param[in] int vap- virtual access point
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setEncryptionOff(int vap);


/**************************************************************************/
/*! \fn int wlan_setAuthMode(int vap, int mode)
 **************************************************************************
 *  \brief set authentication mode of virtual access point
 *  \param[in] int vap - virtual access point
 *  \param[in] int mode - authentication Mode
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setAuthMode(int vap, int mode);


/**************************************************************************/
/*! \fn int wlan_startHostapd()
 **************************************************************************
 *  \brief start hostapd
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_startHostapd(void);


/**************************************************************************/
/*! \fn int wlan_stopHostapd()
 **************************************************************************
 *  \brief stop hostapd
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_stopHostapd(void);


/**************************************************************************/
/*! \fn int wlan_getVapEnable(int index, bool * enable)
 **************************************************************************
 *  \brief get virtual access point is enabled/ or not
 *  \param[in] int index - AP Index
 *  \param[out] bool * enable - true/false
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getVapEnable(int index, bool * enable);


/**************************************************************************/
/*! \fn int wlan_pushWepKeyIndex(int index, unsigned int keyIndex)
 **************************************************************************
 *  \brief push wep key index to hardware
 *  \param[in] int index - AP Index
 *  \param[in] unsigned int keyIndex - key index
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_pushWepKeyIndex(int index, unsigned int keyIndex);


/**************************************************************************/
/*! \fn int wlan_getWpaRekeyInterval(int vap, int *rekeyInterval)
 **************************************************************************
 *  \brief get wpa rekey interval
 *  \param[in] int vap - virtual access point
 *  \param[out] int *rekeyInterval - value of rekey interval
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getWpaRekeyInterval(int vap, int *rekeyInterval);


/**************************************************************************/
/*! \fn int wlan_setWpaRekeyInterval(int vap, int rekeyInterval)
 **************************************************************************
 *  \brief set wpa rekey interval
 *  \param[in] int vap - virtual access point
 *  \param[in] int rekeyInterval - value of rekey interval
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setWpaRekeyInterval(int vap, int rekeyInterval);


/**************************************************************************/
/*! \fn int wlan_pushSsidAdvertisementEnabled(int index, bool enabled)
 **************************************************************************
 *  \brief Hide/ Show SSID
 *  \param[in] int index - AP Index
 *  \param[in] bool enabled - true/false
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_pushSsidAdvertisementEnabled(int index, bool enabled);


/**************************************************************************/
/*! \fn int wlan_kickAssocDevice(int index, wifi_device_t * dev)
 **************************************************************************
 *  \brief kick off wifi device from associated device list
 *  \param[in] int index - AP Index
 *  \param[in] wifi_device_t * dev - wifi device
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_kickAssocDevice(int index, wifi_device_t * dev);


/**************************************************************************/
/*! \fn int wlan_getAclDeviceNum(int index, unsigned int *devNum)
 **************************************************************************
 *  \brief set Router Enable
 *  \param[in] int index - AP Index
 *  \param[out] unsigned int *devNum - Number of devices
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getAclDeviceNum(int index, unsigned int *devNum);


/**************************************************************************/
/*! \fn int wlan_getAclDevice(int index, unsigned int devIndex,char *dev)
 **************************************************************************
 *  \brief get ACL devices
 *  \param[in] int index - AP Index
 *  \param[in] unsigned int devIndex - device index
 *  \param[out] char * dev - device mac address
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_getAclDevice(int index, unsigned int devIndex, char *dev);


/**************************************************************************/
/*! \fn int wlan_kickAclAssocDevices(int index, bool denyList)
 **************************************************************************
 *  \brief kick off the devices from acl list
 *  \param[in] int index - AP Index
 *  \param[in] bool denyList - true/false
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_kickAclAssocDevices(int index, bool denyList);


/**************************************************************************/
/*! \fn int wlan_pushWepKey(int index, int key)
 **************************************************************************
 *  \brief push wep key to hardware
 *  \param[in] int index - AP Index
 *  \param[in] int key - wep key
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_pushWepKey(int index, int key);

/**************************************************************************/
/*! \fn int wlan_setAtfEnable(int index, char* atfEnable)
**************************************************************************
*  \brief set atf enable
*  \param[in] int index - AP Index
*  \param[in] char* atfEnable - atf enable
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_setAtfEnable(int index, bool enable);

/**************************************************************************/
/*! \fn int wlan_getAtfEnable(int index, char* atfEnable)
**************************************************************************
*  \brief get atf enable
*  \param[in] int index - AP Index
*  \param[in] char* atfEnable - atf enable
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_getAtfEnable(int index, bool *enable);

/**************************************************************************/
/*! \fn int wlan_setAtfDistributionType(int index, char* atfDistributionType)
**************************************************************************
*  \brief set Distribution Type
*  \param[in] int index - AP Index
*  \param[in] char* atfDistributionType - Static,Dynmic,Disable
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_setAtfDistributionType(int index, char *atfDistributionType);

/**************************************************************************/
/*! \fn int wlan_getAtfDistributionType(int index, char* atfDistributionType)
**************************************************************************
*  \brief get Distribution Type
*  \param[in] int index - AP Index
*  \param[in] char* atfDistributionType - Static,Dynmic,Disable
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_getAtfDistributionType(int index, char *atfDistributionType);

/**************************************************************************/
/*! \fn int wlan_setAtfAlgoType(int index, char* atfAlgoType)
**************************************************************************
*  \brief set Distribution Type
*  \param[in] int index - AP Index
*  \param[in] char* atfAlgoType - Weighted,Global
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_setAtfAlgoType(int index, char *atfAlgoType);

/**************************************************************************/
/*! \fn int wlan_getAtfAlgoType(int index, char* atfAlgoType)
**************************************************************************
*  \brief get Distribution Type
*  \param[in] int index - AP Index
*  \param[in] char* atfAlgoType - Weighted,Global
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_getAtfAlgoType(int index, char *atfAlgoType);

/**************************************************************************/
/*! \fn int wlan_setAtfInterval(int index, int* atfInterval)
**************************************************************************
*  \brief set Distribution Type
*  \param[in] int index - AP Index
*  \param[in] int* atfInterval - in msec
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_setAtfInterval(int index, int *atfInterval);

/**************************************************************************/
/*! \fn int wlan_getAtfInterval(int index, int* atfInterval)
**************************************************************************
*  \brief get Distribution Type
*  \param[in] int index - AP Index
*  \param[in] int* atfInterval - in msec
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_getAtfInterval(int index, int *atfInterval);

/**************************************************************************/
/*! \fn int wlan_setAtfFreeTime(int index, int* atfFreeTime)
**************************************************************************
*  \brief set Distribution Type
*  \param[in] int index - AP Index
*  \param[in] int* atfFreeTime - in msec
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_setAtfFreeTime(int index, int *atfFreeTime);

/**************************************************************************/
/*! \fn int wlan_getAtfFreeTime(int index, int* atfFreeTime)
**************************************************************************
*  \brief get Distribution Type
*  \param[in] int index - AP Index
*  \param[in] int* atfFreeTime - in msec
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_getAtfFreeTime(int index, int *atfFreeTime);

/**************************************************************************/
/*! \fn int wlan_setAtfVapEnabled(int index, char* atfVapEnabled)
**************************************************************************
*  \brief set VAP enable
*  \param[in] int index - AP Index
*  \param[in] char* atfVapEnabled - enable or disable
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_setAtfVapEnabled(int index, bool enable);

/**************************************************************************/
/*! \fn int wlan_getAtfVapEnabled(int index, char* atfVapEnabled)
**************************************************************************
*  \brief get VAP enable
*  \param[in] int index - AP Index
*  \param[in] char* atfVapEnabled -  enable or disable
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_getAtfVapEnabled(int index, bool *enable);

/**************************************************************************/
/*! \fn int wlan_setAtfStationEnabled(int index, char* atfStationEnabled)
**************************************************************************
*  \brief set station enable
*  \param[in] int index - AP Index
*  \param[in] char* atfStationEnabled -  enable or disable
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_setAtfStationEnabled(int index, bool enable);

/**************************************************************************/
/*! \fn int wlan_getAtfStationEnabled(int index, char* atfStationEnabled)
**************************************************************************
*  \brief get station enable
*  \param[in] int index - AP Index
*  \param[in] char* atfStationEnabled - station enable
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_getAtfStationEnabled(int index, bool *enable);

/**************************************************************************/
/*! \fn int wlan_setAtfRadioStationsAndWeights(int index, char* atfRadioStationsAndWeights)
**************************************************************************
*  \brief set Radio Stations And Weights
*  \param[in] int index - Radio Index
*  \param[in] char* atfRadioStationsAndWeights - MAC and Weight per STA
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_setAtfRadioStationsAndWeights(int index, char *atfRadioStationsAndWeights);

/**************************************************************************/
/*! \fn int wlan_getAtfRadioStationsAndWeights(int index, char* atfRadioStationsAndWeights)
**************************************************************************
*  \brief get Radio Stations And Weights
*  \param[in] int index - Radio Index
*  \param[in] char* atfRadioStationsAndWeights - MAC and Weight per STA
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_getAtfRadioStationsAndWeights(int index, char *atfRadioStationsAndWeights);

/**************************************************************************/
/*! \fn int wlan_setAtfVapWeight(int index, int* atfVapWeight)
**************************************************************************
*  \brief set main Vap Weight
*  \param[in] int index - Radio Index
*  \param[in] int* atfVapWeight - 1-100
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_setAtfVapWeight(int index, int *atfVapWeight);

/**************************************************************************/
/*! \fn int wlan_getAtfVapWeight(int index, int* atfVapWeight)
**************************************************************************
*  \brief get main Vap Weight
*  \param[in] int index - Radio Index
*  \param[in] int* atfVapWeight - 1-100
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_getAtfVapWeight(int index, int *atfVapWeight);

/**************************************************************************/
/*! \fn int wlan_setAtfVapStationsAndWeights(int index, char* atfVapStationsAndWeights)
**************************************************************************
*  \brief set VAP Stations And Weights
*  \param[in] int index - AP Index
*  \param[in] char* atfVapStationsAndWeights - MAC and Weight per STA per VAP
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_setAtfVapStationsAndWeights(int index, char *atfVapStationsAndWeights);

/**************************************************************************/
/*! \fn int wlan_getAtfVapStationsAndWeights(int index, char* atfVapStationsAndWeights)
**************************************************************************
*  \brief get VAP Stations And Weights
*  \param[in] int index - AP Index
*  \param[in] char* atfVapStationsAndWeights - MAC and Weight per STA per VAP
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_getAtfVapStationsAndWeights(int index, char *atfVapStationsAndWeights);

/* Native FAPI - START */
/**************************************************************************/
/*! \fn int fapi_wlan_radio_set_native(int index, ObjList *wlObj, unsigned int flags);
**************************************************************************
 *! \brief This function is used to set Device.WiFi.Radio configuration.
\param[in] index - radio index
\param[in] wlObj - in:  list of objects Device.WiFi.Radio and/or Device.WiFi.Radio.X_LANTIQ_COM_Vendor
\param[in] flags - Flags are placeholder for any future usage.
\return FAPI_SUCCESS / FAPI_FAILURE
*/
int fapi_wlan_radio_set_native(int index, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn int fapi_wlan_ssid_set_native(int index, ObjList *wlObj, unsigned int flags);
**************************************************************************
 *! \brief This function is used to set Device.WiFi.SSID configuration.
\param[in] index - vap index
\param[in] wlObj - in:  object Device.WiFi.SSID
\param[in] flags - Flags are placeholder for any future usage.
\return FAPI_SUCCESS / FAPI_FAILURE
*/
int fapi_wlan_ssid_set_native(int index, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn int fapi_wlan_ap_set_native(int index, ObjList *wlObj, unsigned int flags);
**************************************************************************
 *! \brief This function is used to set Device.WiFi.AccessPoint configuration.
\param[in] index - vap index
\param[in] wlObj - in:  list of objects Device.WiFi.AccessPoint and/or Device.WiFi.AccessPoint.X_LANTIQ_COM_Vendor
\param[in] flags - Flags are placeholder for any future usage.
\return FAPI_SUCCESS / FAPI_FAILURE
*/
int fapi_wlan_ap_set_native(int index, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn int fapi_wlan_security_set_native(int index, ObjList *wlObj, unsigned int flags);
**************************************************************************
 *! \brief This function is used to set Device.WiFi.AccessPoint.Security configuration.
\param[in] index - vap index
\param[in] wlObj - in:  list of objects Device.WiFi.AccessPoint.Security and/or Device.WiFi.AccessPoint.X_LANTIQ_COM_Vendor.Security
\param[in] flags - Flags are placeholder for any future usage.
\return FAPI_SUCCESS / FAPI_FAILURE
*/
int fapi_wlan_security_set_native(int index, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn int fapi_wlan_wps_set_native(int index, ObjList *wlObj, unsigned int flags);
**************************************************************************
 *! \brief This function is used to set Device.WiFi.Radio.WPS configuration.
\param[in] index - vap index
\param[in] wlObj - in:  list of objects Device.WiFi.AccessPoint.WPS and/or Device.WiFi.Radio.X_LANTIQ_COM_Vendor.WPS
\param[in] flags - Flags are placeholder for any future usage.
\return FAPI_SUCCESS / FAPI_FAILURE
*/
int fapi_wlan_wps_set_native(int index, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn int fapi_wlan_wmm_sta_set_native(int index, ObjList *wlObj, unsigned int flags);
**************************************************************************
 *! \brief This function is used to set Device.WiFi.AccessPoint.AC configuration.
\param[in] index - vap index
\param[in] wlObj - in:  list of objects Device.WiFi.AccessPoint.AC and/or Device.WiFi.AccessPoint.X_LANTIQ_COM_Vendor.AC
\param[in] flags - Flags are placeholder for any future usage.
\return FAPI_SUCCESS / FAPI_FAILURE
*/
int fapi_wlan_wmm_sta_set_native(int index, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn int fapi_wlan_wmm_ap_set_native(int index, ObjList *wlObj, unsigned int flags);
**************************************************************************
 *! \brief This function is used to set Device.WiFi.AccessPoint.AC configuration.
\param[in] index - vap index
\param[in] wlObj - in:  list of objects Device.WiFi.AccessPoint.AC and/or Device.WiFi.AccessPoint.X_LANTIQ_COM_Vendor.AC
\param[in] flags - Flags are placeholder for any future usage.
\return FAPI_SUCCESS / FAPI_FAILURE
*/
int fapi_wlan_wmm_ap_set_native(int index, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn int fapi_wlan_wds_set_native(int index, ObjList *wlObj, unsigned int flags);
**************************************************************************
 *! \brief This function is used to set Device.WiFi.AccessPoint.X_LANTIQ_COM_Vendor.WDS configuration.
\param[in] index - vap index
\param[in] wlObj - in:  list of objects Device.WiFi.AccessPoint.X_LANTIQ_COM_Vendor.WDS
\param[in] flags - Flags are placeholder for any future usage.
\return FAPI_SUCCESS / FAPI_FAILURE
*/
int fapi_wlan_wds_set_native(int index, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn int fapi_wlan_hotspot_set_native(int index, ObjList *wlObj, unsigned int flags);
**************************************************************************
 *! \brief This function is used to set Device.WiFi.AccessPoint.X_LANTIQ_COM_Vendor.HS20 configuration.
\param[in] index - vap index
\param[in] wlObj - in:  list of objects Device.WiFi.AccessPoint.X_LANTIQ_COM_Vendor.HS20
\param[in] flags - Flags are placeholder for any future usage.
\return FAPI_SUCCESS / FAPI_FAILURE
*/
int fapi_wlan_hotspot_set_native(int index, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn int fapi_wlan_endpoint_set_native(int index, ObjList *wlObj, unsigned int flags);
**************************************************************************
/*! \brief This function is used to set Device.WiFi.EndPoint configuration.
\param[in] index - vap index
\param[in] wlObj - in:  list of objects Device.WiFi.EndPoint
\param[in] flags - Flags are placeholder for any future usage.
\return FAPI_SUCCESS / FAPI_FAILURE
*/
//int fapi_wlan_endpoint_set_native(int index, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn int wlan_getWdsEnabled(int index, bool * enable)
**************************************************************************
*  \brief get WdsEnabled enable disable the WDS
*  \param[in] int index - AP index
*  \param[out]  bool *enable true or false
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_getWdsEnabled(int index, bool * enable);

/**************************************************************************/
/*! \fn int wlan_setWdsEnabled(int index, bool enable)
 **************************************************************************
 *  \brief set WdsEnabled enable disable the WDS
 *  \param[in] int index - AP index
 *  \param[in] bool enable true or false
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setWdsEnabled(int index, bool enable);


/**************************************************************************/
/*! \fn int wlan_getWdsSecurityMode(int index, char* strValue)
**************************************************************************
*  \brief get WdsSecurityMode WDS security mode 
*  \param[in] int index - AP index
*  \param[out] int *strValue
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_getWdsSecurityMode(int index, char* strValue);

/**************************************************************************/
/*! \fn int wlan_setWdsSecurityMode(int index, char* strValue)
 **************************************************************************
 *  \brief set WdsSecurityMode WDS security mode 
 *  \param[in] int index - AP index
 *  \param[in] char* strValue
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setWdsSecurityMode(int index, char* strValue);


/**************************************************************************/
/*! \fn int wlan_getWdsKey(int index, char* strValue)
**************************************************************************
*  \brief get WdsKey WDS key
*  \param[in] int index - AP index
*  \param[out] int *strValue
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_getWdsKey(int index, char* strValue);

/**************************************************************************/
/*! \fn int wlan_setWdsKey(int index, char* strValue)
 **************************************************************************
 *  \brief set WdsKey WDS key
 *  \param[in] int index - AP index
 *  \param[in] char* strValue
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setWdsKey(int index, char* strValue);


/**************************************************************************/
/*! \fn int wlan_getWdsPeers(int index, char* strValue)
**************************************************************************
*  \brief get WdsPeers WDS peers
*  \param[in] int index - AP index
*  \param[out] int *strValue
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int wlan_getWdsPeers(int index, char* strValue);

/**************************************************************************/
/*! \fn int wlan_setWdsPeers(int index, char* strValue)
 **************************************************************************
 *  \brief set WdsPeers WDS peers
 *  \param[in] int index - AP index
 *  \param[in] char* strValue
 *  \return 0 if success, negative if error / timeout
 ***************************************************************************/
int wlan_setWdsPeers(int index, char* strValue);

/* End of Native FAPI - END */
/* @} */

/** \addtogroup FAPI_WLAN_COMMON_BEEROCK */
/* @{ */
/* BeeRock APIs */
int fapi_wlan_start_monitoring_native(int index, ObjList *wlObj, unsigned int flags);

int fapi_wlan_sta_steer_native(int index, ObjList *wlObj, unsigned int flags);
int wlan_staSteer(int index, char *MACAddress, char *BSSID);

int fapi_wlan_sta_allow_native(int index, ObjList *wlObj, unsigned int flags);
int wlan_staAllow(int index, char *MACAddressControlList);

int fapi_wlan_channel_switch_native(int index, ObjList *wlObj, unsigned int flags);
int wlan_channelSwitch(int index, char *centerFrequency, char *waveVhtCenterFrequency, char *operatingChannelBandwidth);

int fapi_wlan_unassociated_devices_info_req_native(int index, ObjList *wlObj, unsigned int flags);
int wlan_unassociatedDevicesInfoReq(int index, char *MACAddress, char *operatingChannelBandwidth, char *centerFrequency, char *waveVhtCenterFrequency);

int fapi_wlan_restricted_channels_set_native(int index, ObjList *wlObj, unsigned int flags);
int wlan_restrictedChannelsSet(int index, char *channelList);

int fapi_wlan_failsafe_channel_set_native(int index, ObjList *wlObj, unsigned int flags);
int wlan_failsafeChannelSet(int index, char *channel, char *centerFrequency, char *operatingChannelBandwidth);

int fapi_wlan_sta_measurement_get_native(int index, ObjList *wlObj, unsigned int flags);

int fapi_wlan_vap_measurement_get_native(int index, ObjList *wlObj, unsigned int flags);

int fapi_wlan_radio_info_get_native(int index, ObjList *wlObj, unsigned int flags);

int fapi_wlan_acs_report_get_native(int index, ObjList *wlObj, unsigned int flags);

int fapi_wlan_failsafe_channel_get_native(int index, ObjList *wlObj, unsigned int flags);

int fapi_wlan_restricted_channels_get_native(int index, ObjList *wlObj, unsigned int flags);

/*End of BeeRock APIs */
/* @} */


// ### HELPER FUNCTIONS ###
int wlanLoadFromDB(char *pcName, char *ifName, ObjList *ObjPtr);
int securityStateClear(char *ifName);
// ### HELPER FUNCTIONS - End ###


#endif //__RPC_WLAN_CONFIG_API_H_
