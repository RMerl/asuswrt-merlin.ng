/*##################################################################################################
# "Copyright (c) 2013 Intel Corporation                                                            #
# DISTRIBUTABLE AS SAMPLE SOURCE SOFTWARE                                                          #
# This Distributable As Sample Source Software is subject to the terms and conditions              #
# of the Intel Software License Agreement for the Intel(R) Cable and GW Software Development Kit"  #
##################################################################################################*/

#ifndef __FAPI_WLAN_BEEROCK_H_
#define __FAPI_WLAN_BEEROCK_H_

#include <stdio.h>
#include <stdlib.h>

#include "help_proto.h"
#include "help_defs.h"

#include "help_objlist.h"
#include "fapi_wlan.h"
#include "fapi_wlan_private.h"

/** \defgroup FAPI_WLAN_COMMON_BEEROCK FAPI Beerock interfaces
*   @ingroup FAPI_WLAN
\brief It provides the set of API to support the Intel Beerock implementation (AP manager).
*/

/** \addtogroup FAPI_WLAN_COMMON_BEEROCK */
/* @{ */


/*************************************************/
/* Global Functions for general use - prototypes */
/*************************************************/
char *paramValueFromObjGet(ObjList *objPtr, char *objName, char *paramName);
char *paramValueFromSubObjGet(ObjList *subObjDB, char *paramName);
int bssidGet(const char *ifname, char *BSSID);
ObjList *subObjForTwoSameFieldsGet(ObjList *wlObjDB, char *BSSID, char *nr_BSSID);
int channelNumberGet(const char *ifname, char *ChannelNumber);


// Init (ifname stands for radio name)
/**************************************************************************/
/*! \fn fapi_wlan_hostapd_callback_register(const char *ifname, FapiWlanCallBack fapiWlanCallBack, void *context)
**************************************************************************
*  \brief Provides the AP Manager the ability to register to all reports FAPI gets from the hostapd
*  \param[in] ifname - radio interface name
*  \param[in] fapiWlanCallBack - function pointer to callback function to be registered
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_hostapd_callback_register(const char *ifname, FapiWlanCallBack fapiWlanCallBack, void *context);

/**************************************************************************/
/*! \fn fapi_wlan_start_monitoring(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Provides the AP Manager the ability to start the two way connection between FAPI and hostapd
*  \param[in] ifname - radio interface name
*  \param[in] wlObj - NULL
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_start_monitoring(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_stop_monitoring(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Provides the AP Manager the ability to stop the two way connection between FAPI and hostapd
*  \param[in] ifname - radio interface name
*  \param[in] wlObj - NULL
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_stop_monitoring(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_interface_info_get(const char *ifname, ObjList *wlObj, unsigned int flags);
**************************************************************************
*  \param[in] ifname - radio interface name
*  \param[in] wlObj
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_interface_info_get(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_hostapd_socket_get(const char *ifname, void **ctrlInterface)
**************************************************************************
*  \brief Provides the AP Manager the ability to get each hostapd socket
*  \param[in] ifname - radio interface name
*  \param[out] ctrlInterface - the ctrl i/f socket
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_hostapd_socket_get(const char *ifname, void **ctrlInterface);

/**************************************************************************/
/*! \fn fapi_wlan_hostapd_report_process(const char *ifname, ObjList *wlObj, char *opCode)
**************************************************************************
*  \brief Provides the AP Manager the ability to procees hostapd report
*  \param[in] ifname - radio interface name
*  \param[in] wlObj
*  \param[out] opCode
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_hostapd_report_process(const char *ifname, ObjList *wlObj, char *opCode);

/**************************************************************************/
/*! \fn fapi_wlan_set_radio_enable(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Provides the AP Manager the ability to turn radio on during init
*  \param[in] ifname - radio interface name
*  \param[in] wlObj - NULL
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_set_radio_enable(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_is_master_state_get(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief function to get the AP/IRE state
*  \param[in] ifname - radio interface name
*  \param[out] wlObj - the radio AP/IRE state
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_is_master_state_get(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_radio_state_get(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief function to get radio enable/disable state
*  \param[in] ifname - radio interface name
*  \param[out] wlObj - the radio state
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_radio_state_get(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_mac_address_get(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief function to get the MAC address
*  \param[in] ifname - radio interface name
*  \param[out] wlObj - the radio state
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_mac_address_get(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_radio_ctrl_a_filter_bank_set(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief function to set the a filter bank
*  \param[in] ifname - radio interface name
*  \param[out] wlObj - the filter state
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_radio_ctrl_a_filter_bank_set(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_radio_ctrl_a_filter_bank_get(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief function to get the a filter bank
*  \param[in] ifname - radio interface name
*  \param[out] wlObj - the filter state
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_radio_ctrl_a_filter_bank_get(const char *ifname, ObjList *wlObj, unsigned int flags);


// Controls (ifname stands for VAP name)
/**************************************************************************/
/*! \fn fapi_wlan_general_command_set(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Provides the AP Manager with the ability to send hostapd command
*  \param[in] ifname - radio interface name
*  \param[in] command - the command to send to the hostapd
*  \param[out] report - the report coming back from the hostapd
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_general_command_set(const char *ifname, char *command, char *report);

/**************************************************************************/
/*! \fn fapi_wlan_iface_enable(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Provides the AP Manager with the ability to Enable after hostapd reset and FAPI-hostapd connection reset. This is in order to disconnect all station and avoid a race condition in which a station was connected before AP Manager is up and running again
*  \param[in] ifname - radio interface name
*  \param[in] wlObj - NULL
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_iface_enable(const char *ifname, ObjList *wlObj, unsigned int flags);   // ifname stands for radio name

/**************************************************************************/
/*! \fn fapi_wlan_iface_disable(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Provides the AP Manager with the ability to Disable after hostapd reset and FAPI-hostapd connection reset. This is in order to disconnect all station and avoid a race condition in which a station was connected before AP Manager is up and running again
*  \param[in] ifname - radio interface name
*  \param[in] wlObj - NULL
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_iface_disable(const char *ifname, ObjList *wlObj, unsigned int flags);  // ifname stands for radio name

/**************************************************************************/
/*! \fn fapi_wlan_sta_steer(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief API for move a station from one band to the next, or from one AP to the next. This command must be sent by the BeeRock server to all APs managed by BeeRock
*  \param[in] ifname - radio interface name
*  \param[in] wlObj - Object list with two objects. Device.WiFi.AccessPoint.AssociatedDevice with parameter MACAddres of station to steer. Device.WiFi.SSID with parameter BSSID the station must steer to.
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_sta_steer(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_bss_transition_management_req(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief API for issuing Transition Management request.
*  \param[in] ifname - radio interface name
*  \param[in] wlObj - Object list with one object. Device.WiFi.SSID.X_LANTIQ_COM_Vendor
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_bss_transition_management_req(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_sta_allow(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Sends hostapd ctrl msg to allow a STA to connect again.
*  \param[in] ifname - radio interface name
*  \param[in] wlObj - Object list with one object. Device.WiFi.AccessPoint.X_LANTIQ_COM_Vendor with parameter MACAddressControlList
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_sta_allow(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_sta_deny(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Sends hostapd ctrl msg to deny a STA from connecting.
*  \param[in] ifname - radio interface name
*  \param[in] wlObj - Object list with one object. Device.WiFi.AccessPoint.X_LANTIQ_COM_Vendor with parameter MACAddressControlList
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_sta_deny(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_sta_disassociate(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Sends hostapd ctrl msg to disassociate a STA.
*  \param[in] ifname - radio interface name
*  \param[in] wlObj - Object list with one object. Device.WiFi.AccessPoint.X_LANTIQ_COM_Vendor with parameter MACAddress
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_sta_disassociate(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_sta_deauthenticate(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Sends hostapd ctrl msg to deauthenticate a STA.
*  \param[in] ifname - radio interface name
*  \param[in] wlObj - Object list with one object. Device.WiFi.AccessPoint.X_LANTIQ_COM_Vendor with parameter MACAddress
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_sta_deauthenticate(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_channel_switch(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Sends hostapd ctrl msg to change channel
*  \param[in] ifname - interface name
*  \param[in] wlObj - Object list with two objects Device.WiFi.Radio with parameters ExtensionChannel, OperatingChannelBandwidth, SupportedStandards. Device.WiFi.Radio.X_LANTIQ_COM_Vendor with two parameters CenterFrequency, WaveVhtCenterFrequency
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_channel_switch(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_unassociated_devices_info_req(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Sends hostapd ctrl msg to get the device info
*  \param[in] ifname - radio interface name
*  \param[in] wlObj - Object list with three objects Device.WiFi.AccessPoint.AssociatedDevice with parameter MACAddress. Device.WiFi.Radio with parameters Channel, OperatingChannelBandwidth. Device.WiFi.Radio.X_LANTIQ_COM_Vendor with parameter WaveVhtCenterFrequency
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_unassociated_devices_info_req(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_restricted_channels_set(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Sends hostapd ctrl msg to restricted channels set
*  \param[in] ifname - radio interface name
*  \param[in] wlObj - Object list with one objectDevice.WiFi.Radio.X_LANTIQ_COM_Vendor with list of channels not to use.
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_restricted_channels_set(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_failsafe_channel_set(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Sends hostapd ctrl msg to set the failsafe channel
*  \param[in] ifname - radio interface name
*  \param[in] wlObj - Object list with one objectDevice.WiFi.Radio.X_LANTIQ_COM_Vendor with parameters Channel, OperatingChannelBandwidth, ExtensionChannel
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_failsafe_channel_set(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_turn_radio_on(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Provides the AP Manager the ability to turn 0n/off the radio
*  \param[in] ifname - radio interface name
*  \param[in] wlObj - Object list with one Device.WiFi.Radio with parameter Enable (true / false)
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_turn_radio_on(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_mbo_sta_allow(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Provides the AP Manager the ability to allow MBO station
*  \param[in] ifname - radio interface name
*  \param[in] wlObj - Object list with two objects: Device.WiFi.SSID with parameter BSSID. Device.WiFi.AccessPoint.X_LANTIQ_COM_Vendor with parameter MboBssAssociationDisallow
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_mbo_sta_allow(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_mbo_sta_deny(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Provides the AP Manager the ability to deny MBO station
*  \param[in] ifname - radio interface name
*  \param[in] wlObj - Object list with two objects: Device.WiFi.SSID with parameter BSSID. Device.WiFi.AccessPoint.X_LANTIQ_COM_Vendor with parameter MboBssAssociationDisallow
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_mbo_sta_deny(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_neighbor_set(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Provides the AP Manager the ability to add a neighbor report
*  \param[in] ifname - radio interface name
*  \param[in] wlObj - Object list with two objects: Device.WiFi.SSID with parameters BSSID, SSID. Device.WiFi.SSID.X_LANTIQ_COM_Vendor with parameter NeighborReport
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_neighbor_set(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_neighbor_remove(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Provides the AP Manager the ability to add a neighbor report
*  \param[in] ifname - radio interface name
*  \param[in] wlObj - Object list with one object: Device.WiFi.SSID with parameters BSSID, SSID
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_neighbor_remove(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_cellular_pref_set(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Provides the AP Manager the ability to set the cellular preferences
*  \param[in] ifname - radio interface name
*  \param[in] wlObj - Object list with one object: Device.WiFi.Radio.X_LANTIQ_COM_Vendor with parameter MBOCellAware
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_cellular_pref_set(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_cellular_aware_set(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Provides the AP Manager the ability to configure the cellular awareness
*  \param[in] ifname - radio interface name
*  \param[in] wlObj - Object list with one object: Device.WiFi.Radio.X_LANTIQ_COM_Vendor with parameter MBOCellAware
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_cellular_aware_set(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_set_command_send(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Provides the AP Manager the ability to sent the SET command to the hostapd
*  \param[in] ifname - radio interface name
*  \param[in] wlObj - Object list with one object: Device.WiFi.Radio.X_LANTIQ_COM_Vendor with parameters paramName, paramValue.
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_set_command_send(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_channel_load_request(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Provides the AP Manager the ability to request the channel load from a station
*  \param[in] ifname - radio interface name
*  \param[in] wlObj - Object list with two objects: Device.WiFi.Radio with parameters Channel. Device.WiFi.Radio.X_LANTIQ_COM_Vendor with parameters dst, num_of_repetitions, measurement_req_mode, op_class, Channel, rand_int, duration, ch_load_rep, wide_band_ch_switch
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_channel_load_request(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_channel_load_response(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Provides the AP Manager the ability to get the channel load response
*  \param[in] ifname - radio interface name
*  \param[in] wlObj - Object list with two objects: Device.WiFi.Radio with parameters Channel. Device.WiFi.Radio.X_LANTIQ_COM_Vendor with parameters iface, dst, dialog_token, measurement_rep_mode, op_class, Channel, start_time, duration, channel_load, wide_band_ch_switch
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_channel_load_response(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_beacon_request(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Provides the AP Manager the ability to request the 11k beacon
*  \param[in] ifname - radio interface name
*  \param[in] wlObj - Object list with two objects: Device.WiFi.Radio with parameters Channel. Device.WiFi.Radio.X_LANTIQ_COM_Vendor with parameters iface, dst, num_of_repetitions, measurement_req_mode, op_class, Channel, rand_int, duration, mode, bssid, ssid, rep_detail, rep_cond, ap_ch_report, req_elements, wide_band_ch_switch
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_beacon_request(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_beacon_report_response(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Provides the AP Manager the ability to get 11k beacon response
*  \param[in] ifname - radio interface name
*  \param[in] wlObj - Object list with two objects: Device.WiFi.Radio with parameters Channel. Device.WiFi.Radio.X_LANTIQ_COM_Vendor with parameters dst, dialog_token, measurement_rep_mode, op_class, Channel, start_time, duration, frame_info, rcpi, rsni, bssid, ant_id, tsf, wide_band_ch_switch
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_beacon_report_response(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_self_beacon_request(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Provides the AP Manager the ability to request self beacon
*  \param[in] ifname - radio interface name
*  \param[in] wlObj - Object list with one object: Device.WiFi.Radio.X_LANTIQ_COM_Vendor with parameters rand_int, duration, mode
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_self_beacon_request(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_sta_statistics_request(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Provides the AP Manager the ability to request the sta statistics
*  \param[in] ifname - radio interface name
*  \param[in] wlObj - Object list with one object: Device.WiFi.Radio.X_LANTIQ_COM_Vendor with parameters dst, num_of_repetitions, measurement_req_mode, peer_mac_addr, rand_int, duration, group_identity, trig_rep_sta_counters, trig_rep_qos_sta_counters, trig_rep_rsna_counters
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_sta_statistics_request(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_sta_statistics_response(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Provides the AP Manager the ability to get statistics response
*  \param[in] ifname - radio interface name
*  \param[in] wlObj - Object list with one object: Device.WiFi.Radio.X_LANTIQ_COM_Vendor with parameters dst, dialog_token, measurement_rep_mode, duration, group_identity, statistics_group_data, rep_reason
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_sta_statistics_response(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_sta_link_measurements_request(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Provides the AP Manager the ability to request the sta link measurements
*  \param[in] ifname - radio interface name
*  \param[in] wlObj - Object list with one object: Device.WiFi.Radio.X_LANTIQ_COM_Vendor with parameter dst
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_sta_link_measurements_request(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_sta_link_measurements_response(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Provides the AP Manager the ability to get link measurements response
*  \param[in] ifname - radio interface name
*  \param[in] wlObj - Object list with one object: Device.WiFi.Radio.X_LANTIQ_COM_Vendor with parameters dst, dialog_token, rx_ant_id, tx_ant_id, rcpi, rsni, tpc_report, dmg_link_margin, dmg_link_adapt_ack
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_sta_link_measurements_response(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_noise_histigram_request(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Provides the AP Manager the ability to request the noise histogram
*  \param[in] ifname - radio interface name
*  \param[in] wlObj - Object list with one object: Device.WiFi.Radio.X_LANTIQ_COM_Vendor with parameters dst, num_of_repetitions, measurement_req_mode, op_class, channelrand_int, duration, noise_histogram_rep, wide_band_ch_switch
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_noise_histigram_request(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_noise_histogram_response(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Provides the AP Manager the ability to get noise histogram response
*  \param[in] ifname - radio interface name
*  \param[in] wlObj - Object list with one object: Device.WiFi.Radio.X_LANTIQ_COM_Vendor with parameters dst, dialog_token, measurement_token, measurement_rep_mode, op_class, channel, start_time, duration, ant_id, anpi, ipi
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_noise_histogram_response(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_oce_wan_metrics_set(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Set OCE WAN matrics.
*  \param[in] ifname - interface name
*  \param[in] wlObj - Object list with one object: Device.WiFi.Radio.X_LANTIQ_COM_Vendor with parameters uplink_capacity, downlink_capacity
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_oce_wan_metrics_set(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_oce_assoc_reject_set(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Set OCE WAN matrics.
*  \param[in] ifname - interface name
*  \param[in] wlObj - Object list with one object: Device.WiFi.Radio.X_LANTIQ_COM_Vendor with parameters enableDisable, min_rssi_threshold, retry_delay
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_oce_assoc_reject_set(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_antennas_set(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Set the active antennas.
*  \param[in] ifname - interface name
*  \param[in] wlObj - Object list with one object: Device.WiFi.Radio.X_LANTIQ_COM_Vendor with parameter antenna_mask
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_antennas_set(const char *ifname, ObjList *wlObj, unsigned int flags);


// Measurements (ifname stands for VAP name)
/**************************************************************************/
/*! \fn fapi_wlan_sta_measurement_get(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Get available STA statistics
*  \param[in] ifname - interface name
*  \param[in,out] wlObj - Object list in with one object Device.WiFi.AccessPoint.AssociatedDevice with parameter MACAddress. Object list out with two objects Device.WiFi.AccessPoint.AssociatedDevice with parameters MACAddress, AuthenticationState, LastDataDownlinkRate, LastDataUplinkRate, SignalStrength, OperatingStandard and Device.WiFi.AccessPoint.AssociatedDevice.Stats with paramters BytesSent, BytesReceived, PacketsSent, PacketsReceived, RetransCount
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_sta_measurement_get(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_vap_measurement_get(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Get VAP measurement results.
*  \param[in] ifname - interface name
*  \param[in,out] wlObj - Object list in empty object list (not NULL). Out object list Device.WiFi.SSID.Stats with parameters BytesSent, BytesReceived, PacketsSent, PacketsReceived, ErrorsSent, RetransCount, FailedRetransCount, RetryCount, MultipleRetryCount, ACKFailureCount, AggregatedPacketCount, ErrorsReceived, UnicastPacketsSent, UnicastPacketsReceived, DiscardPacketsReceived, MulticastPacketsSent, MulticastPacketsReceived, BroadcastPacketsSent, BroadcastPacketsReceived, UnknownProtoPacketsReceived
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_vap_measurement_get(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_radio_info_get(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Get Radio info.
*  \param[in] ifname - interface name
*  \param[in,out] wlObj - Object list in empty object list (not NULL). Out object list Device.WiFi.Radio.Stats with parameters BytesSent, BytesReceived, PacketsSent, PacketsReceived, ErrorsSent, ErrorsReceived, DiscardPacketsReceived, Noise
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_radio_info_get(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_acs_report_get(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Get latest ACS report. ACS procedure runs in background according to configurable timer.
*  \param[in] ifname - interface name
*  \param[in,out] wlObj - Object list in empty object list (not NULL). Out object list Device.WiFi.Radio.X_LANTIQ_COM_Vendor with ACS report as in hostapd.conf number of channels per AP, RSSI, BSS load, etc.
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_acs_report_get(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_failsafe_channel_get(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Get latest failsafe report.
*  \param[in] ifname - interface name
*  \param[in,out] wlObj - Object list in empty object list (not NULL). Out object list Device.WiFi.Radio.X_LANTIQ_COM_Vendor with failsafe report: center_freq1, center_freq2, bandwidth
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_failsafe_channel_get(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_restricted_channels_get(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Get latest restricted channels report.
*  \param[in] ifname - interface name
*  \param[in,out] wlObj - Object list in empty object list (not NULL). Out object list Device.WiFi.Radio.X_LANTIQ_COM_Vendor with the restricted channels: restricted_channels
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_restricted_channels_get(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_beacon_report_get(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Get beacon report.
*  \param[in] ifname - interface name
*  \param[in,out] wlObj - Object list in empty object list (not NULL). Out object list with two objects. Device.WiFi.Radio with the channel. Device.WiFi.Radio.X_LANTIQ_COM_Vendor with parameters: op_class, start_time, duration, frame_info, rcpi, rsni, bssid, antenna_id, parent_tsf
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_beacon_report_get(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_client_mode_network_add(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Client Mode - add network.
*  \param[in] ifname - interface name
*  \param[in,out] wlObj - NULL
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_client_mode_network_add(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_client_mode_network_set(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Client Mode - set network.
*  \param[in] ifname - interface name
*  \param[in,out] wlObj - NULL
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_client_mode_network_set(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_client_mode_network_enable(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Client Mode - enable network.
*  \param[in] ifname - interface name
*  \param[in,out] wlObj - NULL
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_client_mode_network_enable(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_client_mode_network_select(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Client Mode - select network.
*  \param[in] ifname - interface name
*  \param[in,out] wlObj - NULL
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_client_mode_network_select(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_client_mode_network_disconnect(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Client Mode - disconnect network.
*  \param[in] ifname - interface name
*  \param[in,out] wlObj - NULL
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_client_mode_network_disconnect(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_client_mode_network_remove(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Client Mode - remove network.
*  \param[in] ifname - interface name
*  \param[in,out] wlObj - NULL
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_client_mode_network_remove(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_client_mode_scan_start(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Client Mode - get the scan results.
*  \param[in] ifname - interface name
*  \param[in,out] wlObj
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_client_mode_scan_start(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_client_mode_scan_results_get(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Client Mode - get the scan results.
*  \param[in] ifname - interface name
*  \param[in,out] wlObj
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_client_mode_scan_results_get(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_client_mode_connection_status_get(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Client Mode - get the scan results.
*  \param[in] ifname - interface name
*  \param[in,out] wlObj
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_client_mode_connection_status_get(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_client_mode_data_routing_mode_set(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Client Mode - set the routing mode.
*  \param[in] ifname - interface name
*  \param[in,out] wlObj
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_client_mode_data_routing_mode_set(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_client_mode_data_routing_mode_get(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Client Mode - set the routing mode.
*  \param[in] ifname - interface name
*  \param[in,out] wlObj
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_client_mode_data_routing_mode_get(const char *ifname, ObjList *wlObj, unsigned int flags);

/**************************************************************************/
/*! \fn fapi_wlan_client_mode_radio_info_get(const char *ifname, ObjList *wlObj, unsigned int flags)
**************************************************************************
*  \brief Client Mode - get the radio info.
*  \param[in] ifname - interface name
*  \param[in,out] wlObj
*  \param[in] flags - reserved
*  \return 0 if success, negative if error / timeout
***************************************************************************/
int fapi_wlan_client_mode_radio_info_get(const char *ifname, ObjList *wlObj, unsigned int flags);

/* @} */




#endif  //__FAPI_WLAN_BEEROCK_H_
