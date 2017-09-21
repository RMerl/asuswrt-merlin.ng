/*
** =============================================================================
**   FILE NAME        : fapi_wlan.h
**   PROJECT          : UGW WLAN FAPI
**   DATE             : 13-Dec-2014
**   AUTHOR           : WLAN Subsystem
**   DESCRIPTION      : This file contains the WLAN FAPI prototypes.
**   REFERENCES       :
**   COPYRIGHT        : Copyright (c) 2015
**                      Lantiq Beteiligungs-GmbH & Co. KG
**
**   Any use of this software is subject to the conclusion of a respective
**   License agreement. Without such a License agreement no rights to the
**   software are granted

**   HISTORY          :
**   $Date            $Author                         $Comment
**
** ============================================================================
*/
/*! \file	fapi_wlan.h
	\brief	This file provides prototype definitions for the WLAN FAPI.
	\todo	Need to add APIs for VAP add/delete
*/

#ifndef _FAPI_WLAN_H
#define _FAPI_WLAN_H



#include "help_structs.h"

/** \defgroup FAPI_WLAN_COMMON FAPI core
*   @ingroup FAPI_WLAN
	\brief It provides a set of API to perform common operations on WLAN configuration.
*/

/** \addtogroup FAPI_WLAN_COMMON */
/* @{ */

/* INIT */
/*! \brief  This function is used to initialize WLAN Radios
		\param[in, out] wlObj - in:  Object: Device.Wifi.Radio.Vendor - all parameters from this object
							   out: List of objects:
									1. Device.Wifi, Param: RadioNumberOfEntries - number of radio interfaces that were created
									2. Device.Wifi.Radio, Param: Name - Name of interface  (This object * number of interfaces created)
		\param[in] flags - Flags are placeholder for any future usage.
		\return FAPI_SUCCESS / FAPI_FAILURE

		Detects what vendors are supported by this SW distribution.
		Initializes the FAPI internal data needed for mapping the generic APIs to vendor specific handling.
		Probes the HW to detect what WLAN radios exist and does basic WLAN vendor initialization, e.g. insmods the appropriate drivers,
		creating the initial WLAN radio interfaces.
*/
int fapi_wlan_hw_init(ObjList *wlObj, unsigned int flags);

/*! \brief  This function is used to initialize WLAN Radios.
		\param[in, out] wlObj - should be NULL, unused object, placeholder for any future usage.
		\param[in] flags are placeholder for any future usage.
		\return FAPI_SUCCESS / FAPI_FAILURE

		Uninitializes the WLAN radios, removes the WLAN drivers, and cleans the internal SW structures for the WLAN vendor implementations
*/
int fapi_wlan_hw_uninit(ObjList *wlObj, unsigned int flags);

/* WLAN Capabilities Query */
/*! \brief  This function is used to query WLAN capability of underlying WLAN Radio.
		\param[in] ifname - WLAN radio interface name.
		\param[in, out] wlObj - Object: Device.Wifi.Radio - empty object, out: object will be filled with capabilities as parameters
		\param[in] flags - Flags are placeholder for any future usage.
		\return FAPI_SUCCESS / FAPI_FAILURE
*/
int fapi_wlan_capability_query(const char *ifname, ObjList *wlObj, unsigned int flags);

/* WLAN Radio Configuration */
/*! \brief  This function is used to set WLAN Radio Info.
		\param[in] ifname - WLAN radio interface name.
		\param[in] wlObj - List of 1 or 2 objects: Device.Wifi.Radio and/or Device.Wifi.Radio.Vendor
		\param[in] flags - Flags are placeholder for any future usage.
		\return FAPI_SUCCESS / FAPI_FAILURE
*/
int fapi_wlan_radio_set(const char *ifname, ObjList *wlObj, unsigned int flags);

/*! \brief  This function is used to get WLAN Radio Statistics.
		\param[in] ifname - WLAN radio interface name.
		\param[in, out] wlObj - Object: Device.Wifi.Radio.Stats.
		\param[in] flags - Flags are placeholder for any future usage.
		\return FAPI_SUCCESS / FAPI_FAILURE

		Pass an object containing object name with no parameters, to get all statistics in the object.
		Pass an object with name and parameters that should be read if you want a subset of statistics.
*/
int fapi_wlan_radio_stats_query(const char *ifname, ObjList *wlObj, unsigned int flags);

/* SSID Configuration */
/*! \brief  This function is used to add a WLAN SSID interface, i.e. to create a VAP.
		\param[in] ifname - WLAN *RADIO* interface name, parent of VAP
		\param[in, out] wlObj - in (optional): Device.Wifi.SSID Object containing the VAP interface name and its MAC address, out: the created VAP interface name and its MAC address
		\param[in] flags - Flags are placeholder for any future usage.
		\return FAPI_SUCCESS / FAPI_FAILURE

		Create a virtual access point (AKA VAP or MBSSID), transmitting on the passed radio interface.
		For some vendors the radio interface will be the name of the main AP, for others there is a unique name for radio configuration.
 */
int fapi_wlan_ssid_add(const char *ifname, ObjList *wlObj, unsigned int flags);

/*! \brief  This function is used to remove a WLAN SSID interface, i.e. to delete a VAP.
		\param[in] ifname - WLAN *VAP* interface name
		\param[in, out] wlObj - should be NULL, unused object, placeholder for any future usage.
		\param[in] flags - Flags are placeholder for any future usage.
		\return FAPI_SUCCESS / FAPI_FAILURE
 */
int fapi_wlan_ssid_delete(const char *ifname, ObjList *wlObj, unsigned int flags);

/*! \brief  This function is used to set SSID in WLAN SSID Object.
		\param[in] ifname - WLAN AP/VAP interface name
		\param[in,out] wlObj - Device.Wifi.SSID Object Config for Setting.
		\param[in] flags - Flags are placeholder for any future usage.
		\return FAPI_SUCCESS / FAPI_FAILURE
*/
int fapi_wlan_ssid_set(const char *ifname, ObjList *wlObj, unsigned int flags);

/*! \brief  This function is used to get WLAN SSID Info, e.g. current configuration.
		\param[in] ifname - WLAN AP/VAP interface name
		\param[in,out] wlObj - WLAN Device.Wifi.SSID Object Info. The Object name and (optionally) parameter names to read are passed as input, values are returned as output.
		\param[in] flags - Flags are placeholder for any future usage.
		\return FAPI_SUCCESS / FAPI_FAILURE
*/
int fapi_wlan_ssid_get(const char *ifname, ObjList *wlObj, unsigned int flags);

/*! \brief  This function is used to get WLAN SSID Statistics.
		\param[in] ifname - WLAN AP/VAP interface name
		\param[in,out] wlObj - Device.Wifi.SSID.Stats Object. The Object name is passed as input.
		\param[in] flags - Flags are placeholder for any future usage.
		\return FAPI_SUCCESS / FAPI_FAILURE
*/
int fapi_wlan_ssid_stats_query(const char *ifname, ObjList *wlObj, unsigned int flags);

/*! \brief  This function is used to set WLAN AccessPoint Object.
		\param[in] ifname - WLAN AP/VAP interface name
		\param[in,out] wlObj - List of 1 or 2 objects: Device.Wifi.AccessPoint and/or Device.Wifi.AccessPoint.Vendor
		\param[in] flags - Flags are placeholder for any future usage.
		\return FAPI_SUCCESS / FAPI_FAILURE
*/
int fapi_wlan_ap_set(const char *ifname, ObjList *wlObj, unsigned int flags);

/*! \brief  This function is used to get WLAN AccessPoint Info, e.g. current configuration.
		\param[in] ifname - WLAN AP/VAP interface name
		\param[in,out] wlObj - Device.Wifi.AccessPoint Object Info. The Object name and parameter names to read are passed as input, values are returned as output.
		\param[in] flags - Flags are placeholder for any future usage.
		\return FAPI_SUCCESS / FAPI_FAILURE
*/
int fapi_wlan_ap_get(const char *ifname, ObjList *wlObj, unsigned int flags);


/* Security Configuration */

/*! \brief  This function is used to set WLAN Security Object (Device.Wifi.AccessPoint.Security).
		\param[in] ifname - WLAN AP/VAP interface name
		\param[in,out] wlObj - Device.Wifi.AccessPoint.Security Object Config for Setting.
		\param[in] flags - Flags are placeholder for any future usage.
		\return FAPI_SUCCESS / FAPI_FAILURE
*/
int fapi_wlan_security_set(const char  *ifname, ObjList *wlObj, unsigned int flags);

/*! \brief  This function is used to get WLAN Security Info.
		\param[in] ifname - WLAN AP/VAP interface name
		\param[in,out] wlObj - Device.Wifi.AccessPoint.Security Object Info. The Object name and parameter names to read are passed as input, values are returned as output.
		\param[in] flags - Flags are placeholder for any future usage.
		\return FAPI_SUCCESS / FAPI_FAILURE
*/
int fapi_wlan_security_get(const char *ifname, ObjList *wlObj, unsigned int flags);


/* WPS Configuration */

/*! \brief  This function is used to set WLAN WPS Object (Device.Wifi.AccessPoint.WPS).
		\param[in] ifname - WLAN AP/VAP interface name
		\param[in,out] wlObj - Device.Wifi.AccessPoint.WPS Object Config for Setting.
		\param[in] flags - Flags are placeholder for any future usage.
		\return FAPI_SUCCESS / FAPI_FAILURE
		\todo Accept a list to include the WPS vendor parameters (which object?)
*/
int fapi_wlan_wps_set(const char *ifname, ObjList *wlObj, unsigned int flags);

/*! \brief  This function is used to get WLAN WPS Info.
		\param[in] ifname - WLAN AP/VAP interface name
		\param[in,out] wlObj - Device.Wifi.AccessPoint.WPS Object Info. The Object name and parameter names to read are passed as input, values are returned as output.
		\param[in] flags - Flags are placeholder for any future usage.
		\return FAPI_SUCCESS / FAPI_FAILURE
*/
int fapi_wlan_wps_get(const char *ifname, ObjList *wlObj, unsigned int flags);


/*! \brief  This function is used to set WLAN WMS hostapd.
		\param[in] ifname - WLAN AP/VAP interface name
		\param[in,out] wlObj - Device.Wifi.AccessPoint.HS20 Object Config for Setting.
		\param[in] flags - Flags are placeholder for any future usage.
		\return FAPI_SUCCESS / FAPI_FAILURE
		\todo Accept a list to include the WMS vendor parameters (which object?)
*/
int fapi_wlan_wms_set(const char *ifname, ObjList *wlObj, unsigned int flags);

/* WLAN Device Association */

/*! \brief  This function is used to get all associated stations information.
		\param[in] ifname - WLAN AP/VAP Interface name.
		\param[in,out] wlObj - Device.Wifi.AccessPoint.AssociatedDevice object. Returns list of associated devices and their statistics.
		\param[in] flags - Flags for any future use.
		\return FAPI_SUCCESS / FAPI_FAILURE
		\todo Do we want to separate to API for getting a list of Associated devices and a different API for statistics?

		Returns list of objects, for each STA return a pair of objects:
		Device.WiFi.AccessPoint.AssociatedDevice and Device.WiFi.AccessPoint.AssociatedDevice.Stats
*/
int fapi_wlan_assoc_devices_query(const char *ifname, ObjList *wlObj, unsigned int flags);



/*! \brief  This function is used to get statistic for associated station.
		\param[in] ifname - WLAN AP/VAP Interface name.
		\param[in,out] wlObj - in: Device.Wifi.AccessPoint.AssociatedDevice with STA's MAC address out: device.wifi.accesspoint.associateddevice.stats object. Returns statistics of associated device.
		\param[in] flags - Flags for any future use.
		\return FAPI_SUCCESS / FAPI_FAILURE
		\todo This doesn't follow the common API of fapi_wlan.
		Do we need this? If so, clean up the API - get MAC in object, and pass interface name
*/
int fapi_wlan_assoc_devices_stats_query(const char *ifname, ObjList *wlObj, unsigned int flags);



/* WMM Configure */

/*! \brief  This function is used to set inbound WMM Settings.
		\param[in] ifname - WLAN AP/VAP interface name
		\param[in,out] wlObj - List of 1-4 objects of type Device.Wifi.AccessPoint.AC for WMM Inbound Config.
		\param[in] flags - Flags for any future use.
		\return FAPI_SUCCESS / FAPI_FAILURE
		\todo tbd Do we need APIs for STA WMM objects? if so, what object to pass?
*/
int fapi_wlan_wmm_sta_set(const char *ifname, ObjList *wlObj, unsigned int flags);

/*! \brief  This function is used to retrieve inbound WMM Settings.
		\param[in] ifname WLAN AP/VAP interface name
		\param[in,out] wlObj Device.Wifi.AccessPoint.AC for WLAN WMM Inbound Config. The Object name and parameter names to read are passed as input, values are returned as output.
		\param[in] flags for any future use.
		\return FAPI_SUCCESS / FAPI_FAILURE
		\todo tbd can this fapi accept a list of four objects for BE, BK, VI, VO?
*/
int fapi_wlan_wmm_sta_get(const char *ifname, ObjList *wlObj, unsigned int flags);

/*! \brief  This function is used to query inbound WMM statistics.
		\param[in] ifname WLAN AP/VAP interface name
		\param[in,out] wlObj Device.Wifi.AccessPoint.AC.Stats for WLAN  WMM Inbound Stats. The Object name and parameter names to read are passed as input, values are returned as output.
		\param[in] flags for any future use.
		\return FAPI_SUCCESS / FAPI_FAILURE
		\todo can this fapi accept a list of four objects for BE, BK, VI, VO?
*/
int fapi_wlan_wmm_sta_stats_query(const char *ifname, ObjList *wlObj, unsigned int flags);

/*! \brief  This function is used to set outbound WMM Settings.
		\param[in] ifname - WLAN AP/VAP interface name
		\param[in,out] wlObj - List of 1-4 objects (for BE, BK, VI, VO) of type Device.Wifi.AccessPoint.AC for WMM Outbound Config.
		\param[in] flags - Flags for any future use.
		\return FAPI_SUCCESS / FAPI_FAILURE
*/
int fapi_wlan_wmm_ap_set(const char *ifname, ObjList *wlObj, unsigned int flags);

/*! \brief  This function is used to retrieve outbound WMM Settings.
		\param[in] ifname WLAN AP/VAP interface name
		\param[in,out] wlObj - List of 1-4 objects (for BE, BK, VI, VO) of type Device.Wifi.AccessPoint.AC for WMM Outbound Config. Object name and access category must be filled in each object
		\param[in] flags - Flags for any future use.
		\return FAPI_SUCCESS / FAPI_FAILURE
*/
int fapi_wlan_wmm_ap_get(const char *ifname, ObjList *wlObj, unsigned int flags);

/*! \brief  This function is used to query outbound WMM statistics.
		\param[in] ifname - WLAN AP/VAP interface name
		\param[in,out] wlObj - List of 1-4 objects (for BE, BK, VI, VO) of type Device.Wifi.AccessPoint.AC.Stats for WMM Outbound Stats. The Object name, access category and (optional:) statistics names to read are passed as input, values are returned as output.
		\param[in] flags - Flags for any future use.
		\return FAPI_SUCCESS / FAPI_FAILURE
*/
int fapi_wlan_wmm_ap_stats_query(const char *ifname, ObjList *wlObj, unsigned int flags);


/* WDS Configuration */

/*! \brief  This function is used to set WLAN WDS Object (Device.Wifi.AccessPoint.X_LANTIQ_COM_WDS).
		\param[in] ifname - WLAN AP/VAP interface name
		\param[in,out] wlObj - Device.Wifi.AccessPoint.X_LANTIQ_COM_WDS Object Config for Setting.
		\param[in] flags - Flags are placeholder for any future usage.
		\return FAPI_SUCCESS / FAPI_FAILURE
		\todo 1. What is the object to pass
		\todo 2. WDS will be implemented at a future time based on one of several possible specs. This API can be skipped for now.
*/
int fapi_wlan_wds_set(const char *ifname, ObjList *wlObj, unsigned int flags);

/*! \brief  This function is used to get WLAN WDS Info.
		\param[in] ifname WLAN AP/VAP interface name
		\param[in,out] wlObj Device.Wifi.AccessPoint.X_LANTIQ_COM_WDS Object Info. The Object name and parameter names to read are passed as input, values are returned as output.
		\param[in] flags are placeholder for any future usage.
		\return FAPI_SUCCESS / FAPI_FAILURE
*/
int fapi_wlan_wds_get(const char *ifname, ObjList *wlObj, unsigned int flags);


/* Hotspot Configuration */

/*! \brief  This function is used to set WLAN Hotspot Object (Device.Wifi.AccessPoint.X_LANTIQ_COM_Hotspot).
		\param[in] ifname WLAN AP/VAP interface name
		\param[in,out] wlObj Device.Wifi.AccessPoint.X_LANTIQ_COM_Hotspot Object Config for Setting.
		\param[in] flags are placeholder for any future usage.
		\return FAPI_SUCCESS / FAPI_FAILURE
		\todo define objects for HS20. Pass a list of objects for HS20 config.
*/
int fapi_wlan_hotspot_set(const char *ifname, ObjList *wlObj, unsigned int flags);

/*! \brief  This function is used to get WLAN WDS Info.
		\param[in] ifname WLAN AP/VAP interface name
		\param[in,out] wlObj Device.Wifi.AccessPoint.X_LANTIQ_COM_Hotspot Object Info. The Object name and parameter names to read are passed as input, values are returned as output.
		\param[in] flags are placeholder for any future usage.
		\return FAPI_SUCCESS / FAPI_FAILURE
*/
int fapi_wlan_hotspot_get(const char *ifname, ObjList *wlObj, unsigned int flags);


/* Endpoint Configuration */

/*! \brief  This function is used to set WLAN EndPoint (STA) Objects (Device.Wifi.Endpoint).
		\param[in] ifname WLAN EndPoint interface name
		\param[in,out] wlObj Device.Wifi.EndPoint Object Config for Setting.
		\param[in] flags are placeholder for any future usage.
		\return FAPI_SUCCESS / FAPI_FAILURE
*/
int fapi_wlan_endpoint_set(const char *ifname, ObjList *wlObj, unsigned int flags);

/*! \brief  This function is used to set WLAN EndPoint (STA) WPS action.
		\param[in] ifname WLAN EndPoint interface name
		\param[in,out] wlObj Device.Wifi.EndPoint Object Config for Setting.
		\param[in] flags are placeholder for any future usage.
		\return FAPI_SUCCESS / FAPI_FAILURE
*/
int fapi_wlan_endpoint_wps_set(const char *ifname, ObjList *wlObj, unsigned int flags);

/*! \brief  This function is used to get WLAN EndPoint Statistics.
		\param[in] ifname - WLAN VAP (EndPoint) interface name
		\param[in,out] wlObj - Device.Wifi.EndPoint.Stats Object. The Object name is passed as input.
		\param[in] flags - Flags are placeholder for any future usage.
		\return FAPI_SUCCESS / FAPI_FAILURE
*/
int fapi_wlan_endpoint_stats_query(const char *ifname, ObjList *wlObj, unsigned int flags);

/*! \brief  This function is used to get endpoint scan results information.
		\param[in] ifname - WLAN VAP (EndPoint) Interface name.
		\param[in,out] wlObj - Device.Wifi.EndPoint.Profile object. Returns list of APs found in scan and their information.
		\param[in] flags - Flags for any future use.
		\return FAPI_SUCCESS / FAPI_FAILURE

		Returns list of objects, for each AP found return a pair of objects:
		Device.WiFi.EndPoint.Profile and Device.WiFi.EndPoint.Profile.Security
*/
int fapi_wlan_profile_query(const char *ifname, ObjList *wlObj, unsigned int flags);

/*! \brief  This function is used to query outbound WMM statistics for an EndPoint.
		\param[in] ifname - WLAN VAP (EndPoint) interface name
		\param[in,out] wlObj - List of 1-4 objects (for BE, BK, VI, VO) of type Device.Wifi.EndPoint.AC.Stats for WMM EndPoint Stats. The Object name, access category and (optional:) statistics names to read are passed as input, values are returned as output.
		\param[in] flags - Flags for any future use.
		\return FAPI_SUCCESS / FAPI_FAILURE
*/
int fapi_wlan_wmm_endpoint_stats_query(const char *ifname, ObjList *wlObj, unsigned int flags);

/*! \brief  This function is used to add an interface to its bridge.
		\param[in] ifname - WLAN VAP interface name
		\param[in,out] wlObj - List of 1-4 objects (for BE, BK, VI, VO) of type Device.Wifi.EndPoint.AC.Stats for WMM EndPoint Stats. The Object name, access category and (optional:) statistics names to read are passed as input, values are returned as output.
		\param[in] flags - Flags for any future use.
		\return FAPI_SUCCESS / FAPI_FAILURE
*/
int fapi_wlan_add_to_bridge(const char *ifname, ObjList *wlObj, unsigned int flags);


/* MISCELLANEOUS */

/*! \brief  This function is used to bring up the WLAN interfaces after applying config commands.
		\param[in] ifname - WLAN AP/VAP interface name
		\param[in] flags - Flags are placeholder for any future usage.
		\return FAPI_SUCCESS / FAPI_FAILURE

	All interfaces that are configured as enabled (on this radio) are brought up.
	(All up is relevant for hostapd-based WLAN drivers, that bring down all interfaces on configuration).
*/
int fapi_wlan_up(const char *ifname, ObjList *wlObj, unsigned int flags);

/*! \brief  This function is used to down the WLAN interfaces.
	\param[in] ifname - WLAN AP/VAP interface name
	\param[in] flags - Flags are placeholder for any future usage.
	\return FAPI_SUCCESS / FAPI_FAILURE
*/
int fapi_wlan_down(const char *ifname, ObjList *wlObj, unsigned int flags);

/*! \brief  This function is used to stop hostapd.
	\param[in] ifname - WLAN *VAP* interface name
	\param[in, out] wlObj - should be NULL, unused object, placeholder for any future usage.
	\param[in] flags - Flags are placeholder for any future usage.
	\return FAPI_SUCCESS / FAPI_FAILURE
*/
int fapi_wlan_hostapd_stop(const char *ifname, ObjList *wlObj, unsigned int flags);

/*! \brief  This function is used to init the beerock process.
		\param[in,out] wlObj
		\param[in] flags - Flags are placeholder for any future usage.
		\return FAPI_SUCCESS / FAPI_FAILURE
*/
int fapi_wlan_beerock_init(ObjList *wlObj, unsigned int flags);

/*! \brief Function that used to load WLAN FAPI internal DB.
		\return FAPI_SUCCESS / FAPI_FAILURE

	FAPI WLAN has internal crash protection mechanism that supports rollback for last good configuration
*/
int fapiWlanFailSafeLoad(void);



/*! \brief Function that used to store WLAN FAPI internal DB.
		\return FAPI_SUCCESS / FAPI_FAILURE

	FAPI WLAN has internal crash protection mechanism that supports rollback for last good configuration
*/
int fapiWlanFailSafeStore(void);

/*! \brief Function that used to return FPAI interface name.
		\param[in] rpc_index - integer index
		\param[in] interfaceName - pointer to interface name
		\return FAPI_SUCCESS / FAPI_FAILURE
*/
int getInterfaceName(int rpc_index, char *interfaceName);

void setLog(const char* ifname, ObjList *wlObj, int logIndex);
/* @} */

/* WDS, WPS & HS-2.0 to be added (TBD). */

#endif //_FAPI_WLAN_H
