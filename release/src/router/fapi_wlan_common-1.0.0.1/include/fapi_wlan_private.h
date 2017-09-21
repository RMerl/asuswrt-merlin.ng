/******************************************************************************

						 Copyright (c) 2015
						Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/*! \file fapi_wlan_private.h
	\brief fapi_wlan_private.h defines the FAPI WLAN internals, specifically, the WLAN Vendor handling

	fapi_wlan_private.h defines the interfaces used for mapping the FAPI generic interface to vendor-specific implementations.
	It also includes private APIs, i.e. shared functionality among WLAN vendor implementations, but not exposed to FAPI users.

	\todo 	return dll_uninit when issues of calling in loop will be solved
*/

#ifndef _FAPI_WLAN_PRIVATE_H
#define _FAPI_WLAN_PRIVATE_H 

#include "help_structs.h"

#define logger(...) printf(__VA_ARGS__)

#define FAPI_WLAN_ASSERT(X) {if (!(X)) {logger("FATAL ERROR in FAPI WLAN (%s, %d)\n",__FILE__, __LINE__); return -1;} }

#define FLOG_HELP_FUNC		0x01
#define FLOG_SYSTEM_REPLACE 0x02
#define FLOG_PRIORITY 		(FLOG_HELP_FUNC | FLOG_SYSTEM_REPLACE)
#define FLOG(priority,format,args...)							\
                  if (priority & FLOG_PRIORITY) {				\
                      printf("%s:L%d >",__FUNCTION__,__LINE__); \
					  printf(format, ## args);					\
                  }

#define DB_IFNAME "ifname"			/*!< \Internal DB parameter */
#define DB_RADIO "radio"			/*!< \Internal DB parameter */
#define DB_STATUS "status"			/*!< \Internal DB parameter */
#define DB_ENABLED "true"			/*!< \Internal DB parameter */
#define DB_VENDOR "vendor"			/*!< \Internal DB parameter */
#define DB_RPC_INDEX "rpc_index"	/*!< \Internal DB parameter */

#define DEVICE_WIFI_NAME					"Device.WiFi"											/*!< \brief TR181 Object */
#define DEVICE_ACCESS_POINT_SECURITY		"Device.WiFi.AccessPoint.Security"						/*!< \brief TR181 Object */
#define DEVICE_ACCESS_POINT_SECURITY_VENDOR	"Device.WiFi.AccessPoint.X_LANTIQ_COM_Vendor.Security" /*!< \brief TR181 Object */
#define DEVICE_ACCESS_POINT_WPS				"Device.WiFi.AccessPoint.WPS"							/*!< \brief TR181 Object */
//#define DEVICE_ACCESS_POINT_WPS_VENDOR		"Device.WiFi.AccessPoint.X_LANTIQ_COM_Vendor.WPS"		/*!< \brief TR181 Object */
#define DEVICE_RADIO_WPS_VENDOR				"Device.WiFi.Radio.X_LANTIQ_COM_Vendor.WPS"				/*!< \brief TR181 Object */
#define DEVICE_ACCESS_POINT_AC				"Device.WiFi.AccessPoint.AC"							/*!< \brief TR181 Object */
#define DEVICE_ACCESS_POINT					"Device.WiFi.AccessPoint"								/*!< \brief TR181 Object */
#define DEVICE_RADIO						"Device.WiFi.Radio"										/*!< \brief TR181 Object */
#define DEVICE_RADIO_VENDOR					"Device.WiFi.Radio.X_LANTIQ_COM_Vendor"					/*!< \brief TR181 Object */
#define DEVICE_SSID							"Device.WiFi.SSID"										/*!< \brief TR181 Object */
#define DEVICE_RADIO_STATS					"Device.WiFi.Radio.Stats"								/*!< \brief TR181 Object */
#define DEVICE_SSID_STATS					"Device.WiFi.SSID.Stats"								/*!< \brief TR181 Object */
#define DEVICE_ACCESS_POINT_AD				"Device.WiFi.AccessPoint.AssociatedDevice"				/*!< \brief TR181 Object */
#define DEVICE_ACCESS_POINT_AC_STATS		"Device.WiFi.AccessPoint.AC.Stats"						/*!< \brief TR181 Object */
#define DEVICE_ACCESS_POINT_VENDOR			"Device.WiFi.AccessPoint.X_LANTIQ_COM_Vendor"			/*!< \brief TR181 Object */
#define DEVICE_SECURITY_STATE				"Device.WiFi.Security_State"							/*!< \brief Security internal-db Object */
#define DEVICE_WIFI_NEIGHBOR_DIAG			"Device.WiFi.NeighboringWiFiDiagnostic"
#define DEVICE_INFO							"Device.DeviceInfo"

#define INTERFACE_NAME_FIELD				"Name"                                             /*!< \brief TR69 Parameter VAP\RADIO name */
#define RADIO_NUM_OF_ENTRIES				"RadioNumberOfEntries"                             /*!< \brief TR69 Parameter VAP\RADIO name, number of radios created by each vendor */

#if defined YOCTO
#define DB_PATH "/nvram/etc/wave/db/instance/"
#define DB_PATH_DEFAULT "/etc/wave/db/default/"
#define SCRIPTS_PATH "/etc/wave/scripts/"
#define FAPI_RPC_MODE "/nvram/etc/fapi_rpc_mode"
#else
#define DB_PATH "/opt/lantiq/wave/db/instance/"
#define DB_PATH_DEFAULT "/opt/lantiq/wave/db/default/"
#define SCRIPTS_PATH "/opt/lantiq/wave/scripts/"
#define FAPI_RPC_MODE "/opt/lantiq/wave/fapi_rpc_mode"
#endif

#define DB_TEMP_PATH "/tmp/"                                                               /*!< \brief path to store t DB */
#define WLAN_OBJECT_MAPPER "wlan_fapi_mapping"                                             /*!< \brief name of each object in the list of Internal DB */
#define LAST_SAVED_VAPS_DB "wlan_fapi_vaps"                                                /*!< \brief path to store internal DB */
#define NEW_VAP "new_vap"                                                                  /*!< \brief path to store internal DB */

/*!
	\brief A define for number of possible VAPS on a certain device
*/
#define FAPI_WLAN_MAX_VAPS       100
#define NUM_OF_ACCESS_CATEGORIES   4
#define WPA_CTRL_DISCONNECT        1
#define WLAN_MAX_NUM_VAP 64

#define WPS_ENABLE_BANDS_NONE 0x00

#define WPS_ENABLE_BANDS_WLAN0 0x01

#define WPS_ENABLE_BANDS_WLAN1 0x02

#define WPS_ENABLE_BANDS_WLAN0_WLAN1 0x03

#define WPS_ENABLE_BANDS_WLAN2 0x04

#define WPS_ENABLE_BANDS_WLAN0_WLAN2 0x05

#define WPS_ENABLE_BANDS_WLAN1_WLAN2 0x06

#define WPS_ENABLE_BANDS_WLAN0_WLAN1_WLAN2 0x07


typedef int(*FapiWlanCallBack)(char *opCode, const char *ifname, ObjList *wlObj, unsigned int flags, void *context);


/*! \brief structure of vendor-specific function pointers.

	There must be one vendor function pointer for each generic API exposed in FAPI.
	Each vendor must provide a complete structure for its implementation.
	Function pointers can be NULL for unsupported APIs.

	TBD: These are a lot of functions to implement!
	Maybe we want to unify some APIs and add another parameter for the function to perform?
	That way, instead of having NULL function pointers, we will have a default empty handler in the one function pointer for unsupported features.
*/
typedef struct {
	char vendor_name[MAX_LEN_PARAM_NAME];					/*!< Printable vendor name */
	char vendor_filename[MAX_LEN_PARAM_NAME];				/*!< name of vendor dynamic library file implementing this functionality */
	void *vendor_handle;									/*!< Handle to the vendor dynamic library. This will be used on uninit to free the handle */
	int (*fapi_wlan_hw_init)								(ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_hw_uninit)								(ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_capability_query)						(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_radio_set)								(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_radio_stats_query)						(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_neighboring_diagnostic_query)			(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_ssid_add)								(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_ssid_delete)							(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_ssid_set)								(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_ssid_get)								(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_ssid_stats_query)						(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_ap_set)									(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_ap_get)									(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_security_set)							(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_security_get)							(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_wps_set)								(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_wps_get)								(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_wms_set)								(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_assoc_devices_query)					(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_assoc_devices_stats_query)				(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_wmm_sta_set)							(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_wmm_sta_get)							(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_wmm_sta_stats_query)					(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_wmm_ap_set)								(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_wmm_ap_get)								(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_wmm_ap_stats_query)						(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_wds_set)								(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_wds_get)								(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_hotspot_set)							(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_hotspot_get)							(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_endpoint_set)							(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_endpoint_wps_set)						(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_endpoint_stats_query)					(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_profile_query)							(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_wmm_endpoint_stats_query)				(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_add_to_bridge)							(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_up)										(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_down)									(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_hostapd_stop)							(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_beerock_init)							(ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_hostapd_callback_register)				(const char *ifname, FapiWlanCallBack fapiWlanCallBackFunc, void *context);
	int (*fapi_wlan_start_monitoring)						(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_stop_monitoring)						(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_interface_info_get)						(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_hostapd_socket_get)						(const char *ifname, void **ctrlInterface);
	int (*fapi_wlan_hostapd_report_process)					(const char *ifname, ObjList *wlObj, char *opCode);
	int (*fapi_wlan_is_master_state_get)					(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_radio_state_get)						(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_mac_address_get)						(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_radio_ctrl_a_filter_bank_set)			(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_radio_ctrl_a_filter_bank_get)			(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_general_command_set)					(const char *ifname, char *command, char *report);
	int (*fapi_wlan_iface_enable)							(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_iface_disable)							(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_sta_steer)								(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_bss_transition_management_req)			(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_sta_allow)								(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_sta_deny)								(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_sta_disassociate)						(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_sta_deauthenticate)						(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_channel_switch)							(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_unassociated_devices_info_req)			(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_restricted_channels_set)				(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_failsafe_channel_set)					(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_turn_radio_on)							(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_mbo_sta_allow)							(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_mbo_sta_deny)							(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_neighbor_set)							(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_neighbor_remove)						(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_cellular_pref_set)						(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_cellular_aware_set)						(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_set_command_send)						(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_channel_load_request)					(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_channel_load_response)					(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_beacon_request)							(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_beacon_report_response)					(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_self_beacon_request)					(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_sta_statistics_request)					(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_sta_statistics_response)				(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_sta_link_measurements_request)			(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_sta_link_measurements_response)			(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_noise_histigram_request)				(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_noise_histogram_response)				(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_oce_wan_metrics_set)					(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_oce_assoc_reject_set)					(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_antennas_set)							(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_sta_measurement_get)					(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_vap_measurement_get)					(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_radio_info_get)							(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_acs_report_get)							(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_failsafe_channel_get)					(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_restricted_channels_get)				(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_beacon_report_get)						(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_client_mode_network_add)				(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_client_mode_network_set)				(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_client_mode_network_enable)				(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_client_mode_network_select)				(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_client_mode_network_disconnect)			(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_client_mode_network_remove)				(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_client_mode_scan_start)					(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_client_mode_scan_results_get)			(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_client_mode_connection_status_get)		(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_client_mode_data_routing_mode_set)		(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_client_mode_data_routing_mode_get)		(const char *ifname, ObjList *wlObj, unsigned int flags);
	int (*fapi_wlan_client_mode_radio_info_get)				(const char *ifname, ObjList *wlObj, unsigned int flags);
} fapi_wlan_vendor_ops_t;



/*!
	\brief An enum for referencing the WLAN vendor index in the array of WLAN vendors
*/
typedef enum { ILLEGAL_VENDOR = 0, FAPI_WLAN_LTQ_WAVE, FAPI_WLAN_QCA, FAPI_WLAN_QUANTENNA, FAPI_WLAN_MEDIATEK, FAPI_WLAN_REALTEK, FAPI_WLAN_MAX_VENDORS } fapi_wlan_vendors_t;

/*! \brief An array of ptrs to the vendor-specific implementation structure

	The array is NULL-initialized.
	On wlan init, the HW is probed, the WLAN vendors are identified,
	and the relevant entries in the array are set to point to the fapi_wlan_vendor_ops_t structure

	\todo TBD: To prevent implementing vendor detection in the generic layer, it might be better to initialize this with non-NULL structure.
	For every supported vendor the probe function will be implemented, all other functions NULL, and on successful probe they will be added.
*/
extern fapi_wlan_vendor_ops_t* fapi_wlan_vendor_ops[FAPI_WLAN_MAX_VENDORS];


////// FAPI WLAN internal APIs  //////


/*! \brief Get the vendor ops struct from a vendor implementation library
		\param[in] vendor_handle Handle returned by a dlopen operation.
		\return pointer to the vendor ops structure of this specific vendor
*/
fapi_wlan_vendor_ops_t* get_vendor_ops(void* vendor_handle);



/*! \brief Get the vendor index of the specified interface name
		\param[in] ifname WLAN interface name.
		\return vendor index
*/
fapi_wlan_vendors_t  fapi_wlan_get_vendor(const char *ifname);



/*! \brief helper function to add object to internal DB
		\param[in, out] wlObj - in:  Object
		\param[in] ObjectName - used to identify the original object, there is different handling for radio /ssid objects
		\param[in] i - Vendor enum to be updated in the object
		\param[in] ifname - interface name to be updated in the object
		\return FAPI_SUCCESS / FAPI_FAILURE
*/
int addMapObj(ObjList *wlObj, const char* ObjectName, fapi_wlan_vendors_t i, const char *ifname);



/*! \brief Helper function to update interface status(disabled\enabled)
		\param[in, out] wlObj - in:  Object, fields parsed in the object and written into internal DB
		\param[in] ifname - interface name, used to find the relevant object
		\return FAPI_SUCCESS / FAPI_FAILURE
*/
int updateMapObj(ObjList *wlObj, const char *ifname);



/*! \brief Helper function to delete object from internal DB
		\param[in] ifname - interface name, used to find the relevant object
		\return FAPI_SUCCESS / FAPI_FAILURE
*/
int deleteMapObj(const char *ifname);



/*! \brief dll init function
		\return FAPI_SUCCESS / FAPI_FAILURE
*/
void __attribute__((constructor)) dll_init(void);



/*! \brief dll uninit function
		\return FAPI_SUCCESS / FAPI_FAILURE
void __attribute__ ((destructor)) dll_uninit(void);
*/


/*! \brief file_exists
		\param[in] fname - file name to check
        \return FAPI_SUCCESS / FAPI_FAILURE
*/
int file_exists(const char *fname);

/*! \brief debug_obj helper function for debug use
		\param[in] debug_obj- object to debug
		\return FAPI_SUCCESS / FAPI_FAILURE
*/
int debug_obj(ObjList *debug_obj);

#endif //fapi_wlan_private
