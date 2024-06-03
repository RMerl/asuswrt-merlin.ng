/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2015 RDK Management
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
   Copyright [2015] [Comcast, Corp.]

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

    module: wifi_hal.c

        For CCSP Component:  Wifi_Provisioning_and_management

    ---------------------------------------------------------------

    copyright:

        Comcast, Corp., 2015
        All Rights Reserved.

    ---------------------------------------------------------------

    description:

        This sample implementation file gives the function call prototypes and
        structure definitions used for the RDK-Broadband
        Wifi hardware abstraction layer

    ---------------------------------------------------------------

    environment:

        This HAL layer is intended to support Wifi drivers
        through an open API.

    ---------------------------------------------------------------

    author:

        zhicheng_qiu@cable.comcast.com

**********************************************************************/
#if defined(_CBR_PRODUCT_REQ_) && !defined(_CBR2_PRODUCT_REQ_)
#define _CBR1_PRODUCT_REQ_
#endif /* _CBR_PRODUCT_REQ_ && !_CBR2_PRODUCT_REQ_ */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <linux/un.h>
#include <linux/if.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>
#include "pthread.h"
#include <ctype.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "typedefs.h"
#include "bcmnvram.h"

#include "wlcsm_defs.h"
#include "wifi_hal.h"
#include "wifi_hal_defs.h"
#include "wifi_hal_cb.h"

#include "bcmwifi_channels.h"
#include "wlcsm_lib_wl.h"
#include "wlcsm_lib_api.h"

#if defined(WIFI_HAL_VERSION_3)
/* to avoid compile error */
#ifdef ENABLE
#undef ENABLE
#endif
#include "wlioctl.h"
#endif /* WIFI_HAL_VERSION_3 */
#include "wldm_lib.h"

#define FACTORY_NVRAM_DATA	"/tmp/factory_nvram.data"

#define MAX_ASSOC_DEVICES	75
#define MAC_STR_LEN		18
#define MAC_ADDR_LEN		6

#define HAL_GET_MAX_RADIOS	wldm_get_radios()
#define HAL_GET_MAX_APS		wldm_get_max_aps()

#if defined(WLDM_FOR_HUB4) || defined(_CBR1_PRODUCT_REQ_)
#define ACSD                    "acsd"
#define ACS_CLI                 "acs_cli"
#else
#define ACSD                    "acsd2"
#define ACS_CLI                 "acs_cli2"
#endif

void hal_log_file(const char *logfile, char *time_fmt, const char *fmt, ...)
{
	FILE *log_fd = NULL;
	time_t ltime;
	char timestr[64] = {0};
	struct tm time_info = {0};
	va_list args;

	log_fd = fopen(logfile, "a");
	if (log_fd) {
		ltime = time(NULL);
		localtime_r(&ltime, &time_info);
		strftime(timestr, sizeof(timestr), time_fmt, &time_info);
		fprintf(log_fd, "%s", timestr);

		va_start(args, fmt);
		vfprintf(log_fd, fmt, args);
		va_end(args);

		fflush(log_fd);
		fclose(log_fd);
	}
	return;
}

/* 802.11ax */
#if WIFI_HAL_VERSION_GE_2_15
#pragma message "wifi_hal.h version is >= 2.15"
#endif
/* 802.11v BTM */
#if WIFI_HAL_VERSION_GE_2_12
#pragma message "wifi_hal.h version is >= 2.12"
#else
#ifndef MAX_BTM_DEVICES
#pragma message "wifi_hal.h not define MAX_BTM_DEVICES"
#include "wifi_hal_btm.h"
#else
#pragma message "wifi_hal.h defined MAX_BTM_DEVICES"
#endif /* MAX_BTM_DEVICES */
#endif /* WIFI_HAL_VERSION_GE_2_12*/

extern int get_hex_data(unsigned char *data_str, unsigned char *hex_data, int len);

#if WIFI_HAL_VERSION_GE_2_15 && (!defined(_CBR_PRODUCT_REQ_) && \
	!defined(_XF3_PRODUCT_REQ_))
/* the comment in wifi_hal.h needs to be updated to match this pureMode definition */
#define PMODE_NONE	0x00
#define PMODE_A		0x01
#define PMODE_B		0x02
#define PMODE_G		0x04
#define PMODE_N		0x08
#define PMODE_AC	0x10
#define PMODE_AX	0x20
#ifdef WIFI_HAL_VERSION_GE_3_0_3
#define PMODE_BE	0x100
#endif /* WIFI_HAL_VERSION_GE_3_0_3 */

/* Indicates the std info in the channelMode String - 11N, 11AC etc */
#define CHMODE_NONE	PMODE_NONE
#define CHMODE_11A	PMODE_A
#define CHMODE_11B	PMODE_B
#define CHMODE_11G	PMODE_G
#define CHMODE_11N	PMODE_N
#define CHMODE_11AC	PMODE_AC
#define CHMODE_11AX	PMODE_AX
#ifdef WIFI_HAL_VERSION_GE_3_0_3
#define CHMODE_11BE	PMODE_BE
#endif /* WIFI_HAL_VERSION_GE_3_0_3 */
#endif /* (WIFI_HAL_VERSION_GE_2_15) && (!defined(_CBR_PRODUCT_REQ_) && !defined(_XF3_PRODUCT_REQ_)) */

#define RADIO_INDEX_ASSERT_RC(radioIndex, retcode) \
do { \
	int index = (int)radioIndex; \
	if ((index >= (HAL_GET_MAX_RADIOS)) || (index < 0)) { \
		HAL_WIFI_ERR(("%s: INCORRECT radioIndex = %d numRadios = %d\n", \
			__FUNCTION__, index, HAL_GET_MAX_RADIOS)); \
		return retcode; \
	} \
} while (0)

#define AP_INDEX_ASSERT_RC(apIndex, retcode) \
do { \
	int index = (int)apIndex; \
	if ((index >= (HAL_GET_MAX_APS)) || (index < 0)) { \
		HAL_WIFI_ERR(("%s, INCORRECT apIndex [%d] \n", __FUNCTION__, index)); \
		return retcode; \
	} \
} while (0)

#define NULL_PTR_ASSERT_RC(ptr, retcode) \
do { \
	if (NULL == ptr) { \
		HAL_WIFI_ERR(("%s:%d NULL pointer!\n", __FUNCTION__, __LINE__)); \
		return retcode; \
	} \
} while (0)

#define RADIO_NOT_UP_ASSERT_RC(radioIndex, retcode) \
do { \
	BOOL radioUp = FALSE; \
	int retStatus; \
	retStatus = wifi_getRadioStatus(radioIndex, &radioUp); \
	if ((retStatus) || (!radioUp)) { \
		HAL_WIFI_ERR(("%s: Error radioIndex [%d] %s Up retStatus=%d\n", \
			__FUNCTION__, radioIndex, radioUp ? "Is" : "Not", retStatus)); \
		return retcode; \
	} \
} while (0)

#if WIFI_HAL_VERSION_GE_2_15 && (!defined(_CBR_PRODUCT_REQ_) && \
	!defined(_XF3_PRODUCT_REQ_))
#define AX_CAPABLE_ASSERT_RC(radioIndex, retcode) \
do { \
	if (!wl_is80211axCapable(radioIndex)) { \
		HAL_WIFI_ERR(("%s: Error radioIndex=%d AX not supported\n", \
			__FUNCTION__, radioIndex)); \
		return retcode; \
	} \
} while (0)
#else
#define AX_CAPABLE_ASSERT_RC(radioIndex, retcode)
#endif /* (WIFI_HAL_VERSION_GE_2_15) && (!defined(_CBR_PRODUCT_REQ_) &&
	!defined(_XF3_PRODUCT_REQ_)) */

#ifdef WIFI_HAL_VERSION_3
static int wl_map_str2enum_fromTable(char *istr, unsigned int *ivar, char *delim,
	struct wifi_enum_to_str_map *aTable);
static int wl_str2uintArray(char *istr, char *delim, unsigned int *count,
	unsigned int *uarray, unsigned int maxCount);

#define RADIO_INDEX_ASSERT(radioIndex)	RADIO_INDEX_ASSERT_RC(radioIndex, WIFI_HAL_INVALID_ARGUMENTS)
#define AP_INDEX_ASSERT(apIndex)	AP_INDEX_ASSERT_RC(apIndex, WIFI_HAL_INVALID_ARGUMENTS)
#define NULL_PTR_ASSERT(ptr)		NULL_PTR_ASSERT_RC(ptr, WIFI_HAL_INVALID_ARGUMENTS)
#define AX_CAPABLE_ASSERT(radioIndex)	AX_CAPABLE_ASSERT_RC(radioIndex, WIFI_HAL_INVALID_ARGUMENTS)
#define RADIO_NOT_UP_ASSERT(radioIndex)	RADIO_NOT_UP_ASSERT_RC(radioIndex, WIFI_HAL_INTERNAL_ERROR)
#else
#define RADIO_INDEX_ASSERT(radioIndex)	RADIO_INDEX_ASSERT_RC(radioIndex, RETURN_ERR)
#define AP_INDEX_ASSERT(apIndex)	AP_INDEX_ASSERT_RC(apIndex, RETURN_ERR)
#define NULL_PTR_ASSERT(ptr)		NULL_PTR_ASSERT_RC(ptr, RETURN_ERR);
#define AX_CAPABLE_ASSERT(radioIndex)	AX_CAPABLE_ASSERT_RC(radioIndex, RETURN_ERR);
#define RADIO_NOT_UP_ASSERT(radioIndex)	RADIO_NOT_UP_ASSERT_RC(radioIndex, RETURN_ERR);
#endif /* WIFI_HAL_VERSION_3 */

/* Broadcom uses weak symbols, so you can override them by implementing your own functions.
   If you don't override, then the original Broadcom HAL functions (weak symbols) will be chosen.
*/

#pragma weak _syscmd
#pragma weak wifi_getHalVersion
#pragma weak wifi_factoryReset
#pragma weak wifi_factoryResetRadios
#pragma weak wifi_factoryResetRadio
#pragma weak wifi_initRadio
#pragma weak wifi_lanBridgeConfigNvram_priv
#pragma weak WiFiSystemRadioInit_priv
#pragma weak wifi_init
#pragma weak wifi_reset
#pragma weak wifi_down
#pragma weak wifi_createInitialConfigFiles
#pragma weak wifi_getATMCapable
#pragma weak wifi_setATMEnable
#pragma weak wifi_getATMEnable
#pragma weak wifi_setApATMAirTimePercent
#pragma weak wifi_getApATMAirTimePercent
#pragma weak wifi_getApATMSta
#if !defined(WIFI_HAL_VERSION_GE_3_0)
#pragma weak wifi_setApATMSta
#endif
#pragma weak wifi_getRadioNumberOfEntries
#pragma weak wifi_getSSIDNumberOfEntries
#pragma weak wifi_getRadioEnable
#pragma weak wifi_setRadioEnable
#pragma weak wifi_getRadioStatus
#pragma weak wifi_getRadioIfName
#pragma weak wifi_getRadioMaxBitRate
#pragma weak wifi_getRadioSupportedFrequencyBands
#pragma weak wifi_getRadioOperatingFrequencyBand
#pragma weak wifi_getRadioSupportedStandards
#pragma weak wifi_getRadioStandard
#pragma weak wifi_setRadioChannelMode
#pragma weak wifi_getRadioMode
#pragma weak wifi_setRadioMode
/* #pragma weak sort */
#pragma weak wifi_getRadioPossibleChannels
#pragma weak wifi_getRadioChannelsInUse
#pragma weak wifi_getRadioChannel
#pragma weak wifi_setRadioChannel
#pragma weak wifi_setRadioAutoChannelEnable
#pragma weak wifi_getRadioAutoChannelEnable
#pragma weak wifi_getRadioAutoChannelSupported
#pragma weak wifi_getRadioDCSSupported
#pragma weak wifi_getRadioDCSEnable
#pragma weak wifi_setRadioDCSEnable
#pragma weak wifi_getRadioDfsSupport
#pragma weak wifi_getRadioDfsEnable
#pragma weak wifi_setRadioDfsEnable
#pragma weak wifi_getRadioAutoChannelRefreshPeriodSupported
#pragma weak wifi_getRadioAutoChannelRefreshPeriod
#pragma weak wifi_setRadioAutoChannelRefreshPeriod
#pragma weak wifi_setRadioDfsRefreshPeriod
#pragma weak wifi_getRadioOperatingChannelBandwidth
#pragma weak wifi_setRadioOperatingChannelBandwidth
#pragma weak wifi_getRadioExtChannel
#pragma weak wifi_setRadioExtChannel
#pragma weak wifi_getRadioGuardInterval
#pragma weak wifi_setRadioGuardInterval
#pragma weak wifi_getRadioMCS
#pragma weak wifi_setRadioMCS
#pragma weak wifi_getRadioTransmitPowerSupported
#pragma weak wifi_getRadioTransmitPower
#pragma weak wifi_setRadioTransmitPower
#pragma weak wifi_getRadioIEEE80211hSupported
#pragma weak wifi_getRadioIEEE80211hEnabled
#pragma weak wifi_setRadioIEEE80211hEnabled
#pragma weak wifi_getRadioCarrierSenseThresholdRange
#pragma weak wifi_getRadioCarrierSenseThresholdInUse
#pragma weak wifi_setRadioCarrierSenseThresholdInUse
#pragma weak wifi_getRadioBeaconPeriod
#pragma weak wifi_setRadioBeaconPeriod
#pragma weak wifi_getRadioOperationalDataTransmitRates
#pragma weak wifi_setRadioOperationalDataTransmitRates
#pragma weak wifi_getRadioSupportedDataTransmitRates
#pragma weak wifi_getRadioBasicDataTransmitRates
#pragma weak wifi_setRadioBasicDataTransmitRates
#pragma weak wifi_getRadioTrafficStats2
#pragma weak wifi_getRadioStatsReceivedSignalLevel
#pragma weak wifi_applyRadioSettings
#pragma weak wifi_getSSIDRadioIndex
#pragma weak wifi_getSSIDEnable
#pragma weak wifi_setSSIDEnable
#pragma weak wifi_getSSIDStatus
#pragma weak wifi_getSSIDName
#pragma weak wifi_setSSIDName
#pragma weak wifi_getBaseBSSID
#pragma weak wifi_getSSIDMACAddress
#pragma weak wifi_getSSIDTrafficStats2
#pragma weak wifi_applySSIDSettings
#pragma weak wifi_getNeighboringWiFiDiagnosticResult2
#pragma weak wifi_getRadioWifiTrafficStats
#pragma weak wifi_getBasicTrafficStats
#pragma weak wifi_getWifiTrafficStats
#pragma weak wifi_getSSIDTrafficStats
#pragma weak wifi_getNeighboringWiFiDiagnosticResult
#pragma weak wifi_getAssociatedDeviceDetail
#pragma weak wifi_getAllAssociatedDeviceDetail
#pragma weak wifi_kickAssociatedDevice
#pragma weak wifi_factoryResetAP
#pragma weak wifi_setRadioCtsProtectionEnable
#pragma weak wifi_setRadioObssCoexistenceEnable
#pragma weak wifi_setRadioFragmentationThreshold
#pragma weak wifi_setRadioSTBCEnable
#pragma weak wifi_getRadioAMSDUEnable
#pragma weak wifi_getRadioTxChainMask
#pragma weak wifi_setRadioTxChainMask
#pragma weak wifi_getRadioRxChainMask
#pragma weak wifi_setRadioRxChainMask
#pragma weak wifi_getRadioReverseDirectionGrantSupported
#pragma weak wifi_getRadioReverseDirectionGrantEnable
#pragma weak wifi_setRadioReverseDirectionGrantEnable
#pragma weak wifi_getRadioDeclineBARequestEnable
#pragma weak wifi_setRadioDeclineBARequestEnable
#pragma weak wifi_getRadioAutoBlockAckEnable
#pragma weak wifi_setRadioAutoBlockAckEnable
#pragma weak wifi_getRadio11nGreenfieldSupported
#pragma weak wifi_getRadio11nGreenfieldEnable
#pragma weak wifi_setRadio11nGreenfieldEnable
#pragma weak wifi_getRadioIGMPSnoopingEnable
#pragma weak wifi_setRadioIGMPSnoopingEnable
#pragma weak wifi_createAp
#pragma weak wifi_deleteAp
#pragma weak wifi_getApName
#pragma weak wifi_getIndexFromName
#pragma weak wifi_getApBeaconType
#pragma weak wifi_setApBeaconType
#pragma weak wifi_setApBeaconInterval
#pragma weak wifi_setApDTIMInterval
#pragma weak wifi_setApRtsThresholdSupported
#pragma weak wifi_getApRtsThresholdSupported
#pragma weak wifi_setApRtsThreshold
#pragma weak wifi_getApWpaEncryptionMode
#pragma weak wifi_setApWpaEncryptionMode
#pragma weak wifi_removeApSecVaribles
#pragma weak wifi_disableApEncryption
#pragma weak wifi_setApAuthMode
#pragma weak wifi_getApBasicAuthenticationMode
#pragma weak wifi_setApBasicAuthenticationMode
#pragma weak wifi_getApNumDevicesAssociated
#pragma weak wifi_kickApAssociatedDevice
#pragma weak wifi_getApRadioIndex
#pragma weak wifi_setApRadioIndex
#pragma weak wifi_getApAclDevices
#pragma weak wifi_getApDevicesAssociated
#pragma weak wifi_addApAclDevice
#pragma weak wifi_delApAclDevice
#pragma weak wifi_getApAclDeviceNum
#pragma weak wifi_kickApAclAssociatedDevices
#pragma weak wifi_setApMacAddressControlMode
#pragma weak wifi_setApVlanEnable
#pragma weak wifi_getApVlanID
#pragma weak wifi_setApVlanID
#pragma weak wifi_getApBridgeInfo
#pragma weak wifi_setApBridgeInfo
#pragma weak wifi_resetApVlanCfg
#pragma weak wifi_createHostApdConfig
#pragma weak wifi_startHostApd
#pragma weak wifi_stopHostApd
#pragma weak wifi_setApEnable
#pragma weak wifi_getApEnable
#pragma weak wifi_getApStatus
#pragma weak wifi_getApSsidAdvertisementEnable
#pragma weak wifi_setApSsidAdvertisementEnable
#pragma weak wifi_getApRetryLimit
#pragma weak wifi_setApRetryLimit
#pragma weak wifi_getApWMMCapability
#pragma weak wifi_getApUAPSDCapability
#pragma weak wifi_getApWmmEnable
#pragma weak wifi_setApWmmEnable
#pragma weak wifi_getApWmmUapsdEnable
#pragma weak wifi_setApWmmUapsdEnable
#pragma weak wifi_setApWmmOgAckPolicy
#pragma weak wifi_getApIsolationEnable
#pragma weak wifi_setApIsolationEnable
#pragma weak wifi_getApMaxAssociatedDevices
#pragma weak wifi_setApMaxAssociatedDevices
#pragma weak wifi_getApAssociatedDevicesHighWatermarkThreshold
#pragma weak wifi_setApAssociatedDevicesHighWatermarkThreshold
#pragma weak wifi_getApAssociatedDevicesHighWatermarkThresholdReached
#pragma weak wifi_getApAssociatedDevicesHighWatermark
#pragma weak wifi_getApAssociatedDevicesHighWatermarkDate
#pragma weak wifi_getApSecurityModesSupported
#pragma weak wifi_getApSecurityModeEnabled
#pragma weak wifi_setApSecurityModeEnabled
#pragma weak wifi_getApSecurityPreSharedKey
#pragma weak wifi_setApSecurityPreSharedKey
#pragma weak wifi_getApSecurityKeyPassphrase
#pragma weak wifi_setApSecurityKeyPassphrase
#pragma weak wifi_setApSecurityReset
#pragma weak wifi_getApSecurityRadiusServer
#pragma weak wifi_setApSecurityRadiusServer
#pragma weak wifi_getApSecurityRadiusSettings
#pragma weak wifi_setApSecurityRadiusSettings
#pragma weak wifi_getApWpsEnable
#pragma weak wifi_setApWpsEnable
#pragma weak wifi_getApWpsConfigMethodsSupported
#pragma weak wifi_getApWpsConfigMethodsEnabled
#pragma weak wifi_setApWpsConfigMethodsEnabled
#pragma weak wifi_getApWpsDevicePIN
#pragma weak wifi_setApWpsDevicePIN
#pragma weak wifi_getApWpsConfigurationState
#pragma weak wifi_wpsPairingCompletionCheck_priv
#pragma weak wifi_startWpsPollingTask_priv
#pragma weak wifi_setApWpsEnrolleePin
#pragma weak wifi_setApWpsButtonPush
#pragma weak wifi_cancelApWPS
#pragma weak wifi_getApAssociatedDeviceDiagnosticResult
#pragma weak wifi_getApAssociatedDeviceDiagnosticResult2
#pragma weak wifi_getApAssociatedDeviceDiagnosticResult3
#pragma weak wifi_getApSecurityMFPConfig
#pragma weak wifi_setApSecurityMFPConfig
#pragma weak wifi_ifConfigUp
#pragma weak wifi_pushBridgeInfo
#pragma weak wifi_pushRadioChannel
#pragma weak wifi_pushRadioChannelMode
#pragma weak wifi_pushDefaultValues
#pragma weak wifi_pushRadioTxChainMask
#pragma weak wifi_pushRadioRxChainMask
#pragma weak wifi_pushSSID
#pragma weak wifi_pushSsidAdvertisementEnable
#pragma weak wifi_getRadioUpTime
#pragma weak wifi_setLED
#pragma weak wifi_setLED_priv
#pragma weak wifi_getRadioCountryCode
#pragma weak wifi_setRadioCountryCode
#pragma weak wifi_getRadioDCSChannelPool
#pragma weak wifi_setRadioDCSChannelPool
#pragma weak wifi_getRadioDCSScanTime
#pragma weak wifi_setRadioDCSScanTime
#pragma weak wifi_setRadioTrafficStatsMeasure
#pragma weak wifi_setRadioTrafficStatsRadioStatisticsEnable
#pragma weak wifi_getRadioResetCount
#pragma weak wifi_pushApSsidAdvertisementEnable
#pragma weak wifi_getApSecuritySecondaryRadiusServer
#pragma weak wifi_setApSecuritySecondaryRadiusServer
#pragma weak wifi_getBandSteeringCapability
#pragma weak wifi_getBandSteeringEnable
#pragma weak wifi_setBandSteeringEnable
#pragma weak wifi_getRadioBandUtilization
#pragma weak wifi_getApAssociatedDevice
#pragma weak wifi_getApDeviceRSSI
#pragma weak wifi_getApDeviceRxrate
#pragma weak wifi_getApDeviceTxrate
#pragma weak wifi_getApManagementFramePowerControl
#pragma weak wifi_setApManagementFramePowerControl
#pragma weak wifi_getApBeaconRate
#pragma weak wifi_setApBeaconRate
#pragma weak wifi_getApTxBeaconFrameCount
#pragma weak wifi_getBandSteeringBandUtilizationThreshold
#pragma weak wifi_setBandSteeringBandUtilizationThreshold
#pragma weak wifi_getBandSteeringRSSIThreshold
#pragma weak wifi_setBandSteeringRSSIThreshold
#pragma weak wifi_eventHandlerCallback
#pragma weak wifi_newApAssociatedDevice_callback_register
#pragma weak wifi_getBandSteeringPhyRateThreshold
#pragma weak wifi_setBandSteeringPhyRateThreshold
#pragma weak wifi_getBandSteeringLog
#pragma weak wifi_setRadioDcsDwelltime
#pragma weak wifi_getRadioDcsDwelltime
#pragma weak wifi_getRadioDcsChannelMetrics
#pragma weak wifi_apAssociatedDevice_callback_register
#pragma weak wifi_apAuthEvent_callback_register
#pragma weak wifi_apDeAuthEvent_callback_register
#pragma weak wifi_steering_setGroup
#pragma weak wifi_steering_clientSet
#pragma weak wifi_steering_clientRemove
#pragma weak wifi_steering_clientMeasure
#pragma weak wifi_steering_clientDisconnect
#pragma weak wifi_setFastBSSTransitionActivated
#pragma weak wifi_getBSSTransitionActivated
#pragma weak wifi_getFTOverDSActivated
#pragma weak wifi_setFTOverDSActivated
#pragma weak wifi_getFTMobilityDomainID
#pragma weak wifi_setFTMobilityDomainID
#pragma weak wifi_getFTResourceRequestSupported
#pragma weak wifi_setFTResourceRequestSupported
#pragma weak wifi_getFTR0KeyLifetime
#pragma weak wifi_setFTR0KeyLifetime
#pragma weak wifi_getFTR0KeyHolderID
#pragma weak wifi_setFTR0KeyHolderID
#pragma weak wifi_getFTR1KeyHolderID
#pragma weak wifi_setFTR1KeyHolderID
#pragma weak wifi_pushApFastTransitionConfig
#pragma weak wifi_pushApInterworkingElement
#pragma weak wifi_getApInterworkingElement
#pragma weak wifi_setInterworkingAccessNetworkType
#pragma weak wifi_getInterworkingAccessNetworkType
#pragma weak wifi_getApInterworkingServiceCapability
#pragma weak wifi_setApInterworkingServiceEnable
#pragma weak wifi_getApInterworkingServiceEnable
#pragma weak wifi_allow2G80211ax
#pragma weak wifi_getAllow2G80211ax
#if WIFI_HAL_VERSION_GE_2_19 || defined(WIFI_HAL_VERSION_3)
#pragma weak wifi_pushApHotspotElement
#pragma weak wifi_getApHotspotElement
#pragma weak wifi_pushApRoamingConsortiumElement
#pragma weak wifi_getApRoamingConsortiumElement
#pragma weak wifi_setCountryIe
#pragma weak wifi_getCountryIe
#pragma weak wifi_setP2PCrossConnect
#pragma weak wifi_getP2PCrossConnect
#pragma weak wifi_setDownStreamGroupAddress
#pragma weak wifi_getDownStreamGroupAddress
#pragma weak wifi_setBssLoad
#pragma weak wifi_getBssLoad
#pragma weak wifi_setProxyArp
#pragma weak wifi_getProxyArp
#pragma weak wifi_setLayer2TrafficInspectionFiltering
#pragma weak wifi_getLayer2TrafficInspectionFiltering
#pragma weak wifi_applyGASConfiguration
#endif /* (WIFI_HAL_VERSION_GE_2_19) */
#pragma weak wifi_getRadioDfsMoveBackEnable
#pragma weak wifi_setRadioDfsMoveBackEnable
#if WIFI_HAL_VERSION_GE_2_15 && (!defined(_CBR_PRODUCT_REQ_) && \
	!defined(_XF3_PRODUCT_REQ_))
#pragma weak wifi_setBSSColorEnabled
#endif
#if WIFI_HAL_VERSION_GE_2_16 || defined(WIFI_HAL_VERSION_3)
#pragma weak wifi_dpp_frame_received_callbacks_register
#pragma weak wifi_sendActionFrame
#endif
#if WIFI_HAL_VERSION_GE_2_18 || defined(WIFI_HAL_VERSION_3)
#pragma weak wifi_dppInitiate
#pragma weak wifi_dppCancel
#pragma weak wifi_dppSendAuthCnf
#pragma weak wifi_dppSendConfigResponse
#pragma weak wifi_dppSetSTAPassphrase
#pragma weak wifi_dppRemoveSTAPassphrase
#pragma weak wifi_dppProcessAuthResponse
#pragma weak wifi_dppProcessConfigRequest
#pragma weak wifi_api_is_device_associated
#pragma weak wifi_getRadioChannels
#pragma weak wifi_chan_eventRegister
#endif /* WIFI_HAL_VERSION_GE_2_18 */
#if WIFI_HAL_VERSION_GE_2_19
#pragma weak wifi_enableCSIEngine
#pragma weak wifi_sendDataFrame
#pragma weak wifi_csi_callback_register
#pragma weak wifi_enableGreylistAccessControl
#pragma weak wifi_getApDASRadiusServer
#pragma weak wifi_setApDASRadiusServer
#endif /* WIFI_HAL_VERSION_GE_2_19 */
#if defined(WIFI_HAL_VERSION_3)
#pragma weak wifi_getApWpsConfiguration
#pragma weak wifi_setApWpsConfiguration
#pragma weak wifi_getRadioFrequencyBand
#pragma weak wifi_getRadioOperatingParameters
#pragma weak wifi_setRadioOperatingParameters
#pragma weak wifi_getApSecurity
#pragma weak wifi_setApSecurity
#pragma weak wifi_getEAP_Param
#pragma weak wifi_setEAP_Param
#pragma weak wifi_getAvailableBSSColor
#pragma weak wifi_setBSSColor
#pragma weak wifi_getMuEdca
#pragma weak wifi_getRadioVapInfoMap
#pragma weak wifi_createVAP
#endif /* WIFI_HAL_VERSION_3 */
#ifdef RDKB_LGI
#pragma weak wifi_getRADIUSAcctEnable
#pragma weak wifi_setRADIUSAcctEnable
#pragma weak wifi_getApSecurityAcctServer
#pragma weak wifi_setApSecurityAcctServer
#pragma weak wifi_getApSecuritySecondaryAcctServer
#pragma weak wifi_setApSecuritySecondaryAcctServer
#pragma weak wifi_getApSecurityAcctInterimInterval
#pragma weak wifi_setApSecurityAcctInterimInterval
#pragma weak wifi_setApRadiusTransportInterface
#pragma weak wifi_getApRadiusReAuthInterval
#pragma weak wifi_setApRadiusReAuthInterval
#pragma weak wifi_getRadiusOperatorName
#pragma weak wifi_setRadiusOperatorName
#pragma weak wifi_getRadiusLocationData
#pragma weak wifi_setRadiusLocationData
#pragma weak wifi_getApPMKCacheInterval
#pragma weak wifi_setApPMKCacheInterval
#pragma weak wifi_getWpsStatus
#pragma weak wifi_getRadioConfiguredChannel
#pragma weak wifi_getRadioRunningChannel
#pragma weak wifi_getSoftBlockEnable
#pragma weak wifi_setSoftBlockEnable
#pragma weak wifi_clearSoftBlockBlacklist
#pragma weak wifi_getSoftBlockBlacklistEntries
#pragma weak wifi_getSupportRatesBitmapControlFeature
#pragma weak wifi_setSupportRatesBitmapControlFeature
#pragma weak wifi_getSupportRatesDisableBasicRates
#pragma weak wifi_setSupportRatesDisableBasicRates,
#pragma weak wifi_getSupportRatesDisableSupportedRates
#pragma weak wifi_setSupportRatesDisableSupportedRates
#pragma weak wifi_getApWmmOgAckPolicy
#pragma weak wifi_getRadioExcludeDfs
#pragma weak wifi_setRadioExcludeDfs
#pragma weak wifi_getRadioChannelWeights
#pragma weak wifi_setRadioChannelWeights
#pragma weak wifi_isZeroDFSSupported
#pragma weak wifi_setZeroDFSState
#pragma weak wifi_getZeroDFSState
#pragma weak wifi_getZeroWaitDFSChannelsStatus
#pragma weak wifi_getCurrentRadioOperatingChannelBandwidth
#endif /* RDKB_LGI */
/* #pragma weak int main */

static int wifi_hal_cb_assoc_dev_evt_handler(int sock_fd);
static wldm_callback_thread_t wifi_cb_info = {0, 0, (pthread_t) NULL};

static wifi_callback_fnc_t callback_fnc; /* remember cb function locally */
pthread_t cbThreadId; /* shared with wifi_api */

static pthread_t wpsThread = (pthread_t) NULL;
static int timeCount = 0;
static BOOL WiFi_Ready_after_hal_started = FALSE;

extern int wifi_apply(void);

BOOL wifi_lanLinkDone = FALSE;
static BOOL lanBridgeThread_lock_created = FALSE;
BOOL lanBridgeThread_exit = FALSE;
static int wifi_led_gpio = -1;

static int wifi_api_socket_init(void);
static pthread_t wifi_lanBridgeThread = (pthread_t)NULL;
#if defined(BUILD_RDKWIFI) && !defined(BCA_CPEROUTER_RDK)
pthread_t wifi_apiThread = (pthread_t)NULL;
#else
static pthread_t wifi_apiThread = (pthread_t)NULL;
#endif
static void wifi_api_thread_main_loop( void );
pthread_mutex_t lanBridgeThread_lock;

int WiFiSystemRadioInit_priv(UINT32 radioIndex);

/* Used by wifi_setRadioMode */
typedef struct chVar_info {
	char		channelBWStr[32];
	char		extensionChStr[32];
	int		VhtModeValue;
	unsigned int	HeFeatValue;
	opmode_cap_t	omcVal;
	char		operatingStandards[32];
	int		channelStd;
	unsigned int	curr_channel;
} chVar_info_t;

#if defined(WIFI_HAL_VERSION_3)
static INT wifi_updateApSecurity(INT apIndex, wifi_vap_security_t *security);
#endif

static INT wl_channelModeInfo_to_channelVars(int radioIndex, char *channelMode,
	unsigned int pureMode, chVar_info_t *chInfo);
static INT wl_setRadioMode(INT radioIndex, CHAR *channelMode, UINT pureMode);

static char wifi_led_blink_str[WIFI_LED_MAX_BLINK][20] ={"off", "on", "slow-blink", "medium-blink", "fast-blink", "off-fast", "off-slow"};
static wifi_led_blink_state_t wifi_prev_led_state[HAL_RADIO_NUM_RADIOS] = {WIFI_LED_MAX_BLINK};
static int hal_bss_status(INT ApIndex);

INT wifi_setLED_priv(INT radioIndex, wifi_led_blink_state_t led_state);

static int wifi_numRegisteredAssociatedCallbacks = 0;
static int wifi_numRegisteredDisassociatedCallbacks = 0;

static wifi_newApAssociatedDevice_callback wifi_app_associated_callbacks[MAX_REGISTERED_CB_NUM] = {NULL};
#if WIFI_HAL_VERSION_GE_2_12 || defined(WIFI_HAL_VERSION_3)
static wifi_apDisassociatedDevice_callback wifi_app_disassociated_callbacks[MAX_REGISTERED_CB_NUM] = {NULL};
#endif

const CHAR * beaconTypeStr[BEACON_TYPE_TOTAL] = { "None", "Basic", "WPA", "11i", "WPAand11i" };
const CHAR * basicAuthenticationModeStr[BASIC_AUTHENTICATION_MODE_TOTAL] = { "None", "EAPAuthentication", "SharedAuthentication", "PSKAuthentication" };
const CHAR * wpaEncryptionModeStr[WPA_ENCRYPTION_MODE_TOTAL] = { "TKIPEncryption", "AESEncryption", "TKIPandAESEncryption" };

static BEACON_TYPE beaconType[HAL_AP_NUM_APS_PER_RADIO*HAL_RADIO_NUM_RADIOS];
static BASIC_AUTHENTICATION_MODE basicAuthenticationMode[HAL_AP_NUM_APS_PER_RADIO*HAL_RADIO_NUM_RADIOS];

UINT32 radio_map[2] = {1,2};
UINT32 ap_map[16] = {1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15, 8, 16};

UCHAR GUARDINTERVAL[3][8] = {"800", "400", "auto"};
UCHAR OPERATINGBANDWIDTH[MAX_OPERATING_BANDWIDTH][5] = {"20", "40", "80", "160", "Auto"};

static scanfilter_t gscanfilter[HAL_WIFI_TOTAL_NO_OF_APS];

static char bridgeName[HAL_WIFI_TOTAL_NO_OF_APS][32 + 1] = {"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""};
static char bridgeIP[HAL_WIFI_TOTAL_NO_OF_APS][16] = {"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""};
static char bridgeSubnet[HAL_WIFI_TOTAL_NO_OF_APS][16] = {"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""};

/* ============== Plume things here ============= */
typedef struct wifi_net_dev_stats{
	char					interfacename[OUTPUT_STRING_LENGTH_128];
	long unsigned int		RxBytes;
	long unsigned int		RxPkts;
	long unsigned int		RxErrors;
	long unsigned int		RxDrops;
	long unsigned int		RxFifo;
	long unsigned int		RxFrame;
	long unsigned int		RxCompressed;
	long unsigned int		RxMulticast;
	long unsigned int		TxBytes;
	long unsigned int		TxPkts;
	long unsigned int		TxErrors;
	long unsigned int		TxDrops;
	long unsigned int		TxFifo;
	long unsigned int		TxFrame;
	long unsigned int		TxCompressed;
	long unsigned int		TxMulticast;
} wifi_net_dev_stats;

#define ACL_MAC_ARRAY_MAX	512
#define ASSOC_MAC_ARRAY_MAX	1024

extern INT wifi_setRadioDfsMoveBackEnable(INT radioIndex, BOOL enabled);

static char *
RESETCOUNT_FILENAME(int index)
{
	static char resetCountFile[OUTPUT_STRING_LENGTH_32];
	snprintf(resetCountFile, sizeof(resetCountFile), "/tmp/.resetcount_wl%d", index);
	return resetCountFile;
}

static unsigned int
getRadioUpdateCount(int radioIndex)
{
	FILE *fptr = NULL;
	unsigned int count = 0;

	fptr = fopen(RESETCOUNT_FILENAME(radioIndex), "r");
	if (fptr) {
		if (fscanf(fptr,"%d", &count) == 0) {
			count = 0;
		}
		fclose(fptr);
		fptr = NULL;
	}

	return count;
}

static void
incrementRadioUpdateCounter(int radioIndex)
{
	FILE *fptr = NULL;
	unsigned int count = 0;

	count = getRadioUpdateCount(radioIndex);
	fptr = fopen(RESETCOUNT_FILENAME(radioIndex), "w");
	if (fptr != NULL) {
		fprintf(fptr,"%d", ++count);
		fclose(fptr);
		fptr = NULL;
	}
}

static INT wifi_CheckAndConfigureLEDS(void);

static void restart_acsd(void)
{
	char cmdBuf[BUF_SIZE] = {0};
        HAL_WIFI_DBG(("%s, Restart acsd!!!\n", __FUNCTION__));

        snprintf(cmdBuf, sizeof(cmdBuf), "killall -q -9 %s 2>/dev/null",ACSD);
        if (system(cmdBuf) != 0) {
                HAL_WIFI_DBG(("%s: system kill acsd failed\n", __FUNCTION__));
        }
        snprintf(cmdBuf, sizeof(cmdBuf), "%s",ACSD);
        if (system(cmdBuf) != 0) {
                HAL_WIFI_DBG(("%s: system acsd failed\n", __FUNCTION__));
        }
#if defined(_CBR1_PRODUCT_REQ_)
        wlcsm_nvram_set("acsd_started", "1");
#else
        nvram_set("acsd2_started", "1");
#endif /* _CBR1_PRODUCT_REQ_ */
}

char *_strlwr(char *str)
{
	unsigned char *p = (unsigned char *)str;

	while (*p) {
		*p = tolower((unsigned char)*p);
		p++;
	}

	return str;
}

#if defined(RDKB_TIMER)
/*thread and variable to call  wldm_apply  at specific intervals*/
static pthread_t wifi_timerThread = (pthread_t)NULL;
static void wifi_timer_thread_main_loop( void );
static volatile int gApplySetCounter;
static volatile int Pending;
void wifi_set_timer_variable(void);
#define WIFI_APPLY_TIMER_SLEEP 50000
#define WIFI_APPLY_TIMER_COUNTER 500
#define WIFI_APPLY_TIMER_DECREMENT 50

/* wifi_set_timer_variable() function */
/**
* @brief function to set the timer for wifi apply settings
*
*
*/
void wifi_set_timer_variable(void)
{
	gApplySetCounter = WIFI_APPLY_TIMER_COUNTER;
	Pending = 1;
}

/* wifi_timer_thread_main_loop() thread */
/**
* @brief function to maintain a timer thread for applying wifi settings
*
*
*/
void wifi_timer_thread_main_loop( void )
{
	HAL_WIFI_DBG(("wifi_api_timer_main_loop is ready for new msg...\n"));

	while (1) {
		usleep(WIFI_APPLY_TIMER_SLEEP);
		if (gApplySetCounter == 0 ) {
			if(Pending == 1) {
				//call wifi_appy here
				Pending = 0;
				wifi_apply();
			}
		} else {
			gApplySetCounter =  gApplySetCounter - WIFI_APPLY_TIMER_DECREMENT ;
		}
	}

	pthread_exit(NULL);
}
#endif /* End of defined(RDKB_TIMER)*/

int _syscmd(char *cmd, char *retBuf, int retBufSize)
{
	FILE *f;
	char *ptr = retBuf;
	int bufSize = retBufSize, bufbytes = 0, readbytes = 0;

	if ((f = popen(cmd, "r")) == NULL) {
		printf("popen %s error\n", cmd);
		return -1;
	}

	while (!feof(f)) {
		*ptr = 0;
		if (bufSize >= 128) {
			bufbytes = 128;
		} else {
			bufbytes = bufSize - 1;
		}

		if (fgets(ptr, bufbytes, f) == NULL) {
			break;
		}
		readbytes = strlen(ptr);
		if (readbytes == 0) {
			break;
		}
		bufSize -= readbytes;
		ptr += readbytes;
	}
	pclose(f);
	retBuf[retBufSize-1] = 0;
	return 0;
}

/**********************************************************************************
 *
 *  Wifi Subsystem level function prototypes
 *
**********************************************************************************/

//Wifi system api
//Get the wifi hal version in string, eg "2.0.0".  WIFI_HAL_MAJOR_VERSION.WIFI_HAL_MINOR_VERSION.WIFI_HAL_MAINTENANCE_VERSION
INT wifi_getHalVersion(CHAR *output_string) //RDKB
{
	if (NULL == output_string) {
		HAL_WIFI_ERR(("%s, output_string parameter error!!!\n", __FUNCTION__));
		return RETURN_ERR;
	}

	snprintf(output_string, 64, "%d.%d.%d", WIFI_HAL_MAJOR_VERSION, WIFI_HAL_MINOR_VERSION, WIFI_HAL_MAINTENANCE_VERSION);

	HAL_WIFI_DBG(("%s, output_string = %s\n", __FUNCTION__, output_string));
#ifdef WIFI_HAL_VERSION_3
	HAL_WIFI_DBG(("WIFI_HAL_VERSION_3 is TRUE\n"));
#else
	HAL_WIFI_DBG(("WIFI_HAL_VERSION_3 is FALSE\n"));
#endif
	HAL_WIFI_DBG(("MAX_NUM_RADIOS=%d HAL_RADIO_NUM_RADIOS=%d MAX_NUM_VAP_PER_RADIO=%d"
		" HAL_WIFI_TOTAL_NO_OF_APS=%d\n",
		MAX_NUM_RADIOS, HAL_RADIO_NUM_RADIOS, MAX_NUM_VAP_PER_RADIO,
		HAL_WIFI_TOTAL_NO_OF_APS));
	HAL_WIFI_DBG(("HAL_GET_MAX_RADIOS=%d HAL_GET_MAX_APS=%d\n",
		HAL_GET_MAX_RADIOS, HAL_GET_MAX_APS));

	return RETURN_OK;
}

/* For wifi_api: the further action after factoryResetXXX
   because wifi_hal API such as wifi_factoryReset no option/argu for commit/restart
 */
int wifi_factoryReset_post(int ap_index, int commit, int restart)
{
	char cmd[64] = {0};

	if (ap_index == -1) {
		/* special case for all */
		HAL_WIFI_DBG(("%s, for all interfaces\n", __FUNCTION__));
	} else {
		AP_INDEX_ASSERT(ap_index);
	}
	if (commit) {
#ifndef BCA_CPEROUTER_RDK
		nvram_commit();
#endif /* BCA_CPEROUTER_RDK */
	}
	if (restart) {
#ifdef BCA_CPEROUTER_RDK
		if (ap_index == -1) {
			struct stat sb;
			int retries = 120;
			int interval = 1000; // interval between retries in ms
			while (retries > 0) {
				if (stat("/tmp/.brcm_wifi_ready", &sb) == 0) {
					if (S_ISREG(sb.st_mode)) {
						system("systemctl restart ccspwifiagent.service");
						break;
					}
				} else if (errno != ENOENT) {
					perror("open ready file error");
					break;
				}
				retries--;
				usleep(interval * 1000); // wait for interval ms
			}
		}
#else
		snprintf(cmd, sizeof(cmd), "wifi_setup.sh restart %s",
				(ap_index == -1) ? "" : wldm_get_nvifname(ap_index));
		HAL_WIFI_DBG(("%s, cmd=<%s>\n", __FUNCTION__, cmd));
		if (system(cmd)) {
			HAL_WIFI_ERR(("%s, cmd=<%s FAILED\n", __FUNCTION__, cmd));
			return -1;
		}
#endif /* BCA_CPEROUTER_RDK */
	}

	return 0;
}

#define NVRAM_DEFAULT_RAIDO_PATH	"/usr/local/etc/wlan/nvram_default_radio.txt"
static BOOL
use_nvram_factory_reset_template()
{
	char cmd[256] = {0};
	snprintf(cmd, sizeof(cmd), "grep -i template %s", NVRAM_DEFAULT_RAIDO_PATH);
	if (system(cmd))
		return FALSE;
	else
		return TRUE;
}
/* wifi_factoryReset() function */
/**
* Description:
*  Resets Implementation specifics may dictate some functionality since
*  different hardware implementations may have different requirements.
* Parameters : None
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
INT wifi_factoryReset()
{
	INT radioIdx;
	for (radioIdx = 0; radioIdx < HAL_GET_MAX_RADIOS; radioIdx ++) {
		incrementRadioUpdateCounter(radioIdx);
	}

	HAL_WIFI_LOG(("%s: Start\n", __FUNCTION__));
	/* not "commit", reboot will take effect (by wifi_setup.sh) */
	return wldm_xbrcm_factory_reset(WLDM_NVRAM_FACTORY_RESTORE, 0, 0, 0);
}

/* wifi_factoryResetRadios() function */
/**

* Description:
*  Resets Implementation specifics may dictate some functionality since
*  different hardware implementations may have different requirements.
*  Parameters : None
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
INT wifi_factoryResetRadios()
{
	HAL_WIFI_LOG(("%s: Start\n", __FUNCTION__));
	BOOL useTemplate = use_nvram_factory_reset_template();
	INT iRet = RETURN_OK, iIter = 0;
	/* remove /nvram/greylist_mac.txt */
	unlink("/nvram/greylist_mac.txt");

	if (useTemplate) {
		if (system("wifi_setup.sh factory_reset_all_radios")) {
			HAL_WIFI_ERR(("%s: failed\n", __FUNCTION__));
			return RETURN_ERR;
		}
		return RETURN_OK;
	}

	for (iIter = 0; iIter < HAL_GET_MAX_RADIOS; iIter++) {
		if ((iRet = wifi_factoryResetRadio(iIter)) != RETURN_OK) {
			/* resetting both Radios to Factory defaults */
			HAL_WIFI_ERR(("%s: failed wifi_factoryResetRadio for radio %d\n", __FUNCTION__, iIter));
			return iRet;
		}
	}

	return iRet;
}

/* wifi_factoryResetRadio() function */
/**

* Description:
*  Resets Implementation specifics may dictate some functionality since
*  different hardware implementations may have different requirements.
*  Parameters : None
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
INT wifi_factoryResetRadio(int radioIndex) //RDKB
{
	BOOL useTemplate = use_nvram_factory_reset_template();
	char cmd[BUF_SIZE];

	HAL_WIFI_LOG(("%s: Start radioIndex=%d\n", __FUNCTION__, radioIndex));
	RADIO_INDEX_ASSERT(radioIndex);

	if (useTemplate) {
		snprintf(cmd, sizeof(cmd), "wifi_setup.sh reset_radio_nvram %s",
				wldm_get_radio_osifname(radioIndex));
		if (system(cmd)) {
			HAL_WIFI_ERR(("%s: system command %s failed\n", __FUNCTION__, cmd));
			return RETURN_ERR;
		}
	} else {
		if (wldm_xbrcm_factory_reset(WLDM_NVRAM_FACTORY_RESET_RADIO, radioIndex, 0, 0) != 0 ) {
			HAL_WIFI_ERR(("%s: wldm_xbrcm_factory_reset radio %d failed\n",
				__FUNCTION__, radioIndex));
			return RETURN_ERR;
		}
	}

#if defined(_XB7_PRODUCT_REQ_) || defined(_XB8_PRODUCT_REQ_) || \
	defined(_CBR2_PRODUCT_REQ_) || defined(_CBR1_PRODUCT_REQ_)
	/* Remove nvram upgrade flag files. */
	if (system("rm -f /nvram/.bcmwifi*")) {
		HAL_WIFI_DBG(("rm -f /nvram/.bcmwifi* -- failed\n"));
	}
#endif /* _XB7_PRODUCT_REQ_ || _XB8_PRODUCT_REQ_ || _CBR2_PRODUCT_REQ_ || _CBR1_PRODUCT_REQ_ */

	/* wlconf wlX up to restore iovars */
	snprintf(cmd, sizeof(cmd), "wlconf %s up", wldm_get_radio_osifname(radioIndex));
	if (system(cmd)) {
		HAL_WIFI_ERR(("%s: system command %s failed\n", __FUNCTION__, cmd));
		return RETURN_ERR;
	}

	/* Run acsd2 acs_restart cmd, sets chanspec with default bw */
	if (wldm_xbrcm_acs(CMD_SET_IOCTL, radioIndex, NULL, NULL, "acs_restart", NULL) < 0) {
		HAL_WIFI_ERR(("%s: wldm_xbrcm_acs CMD_SET_IOCTL failed.\n", __FUNCTION__));
		return RETURN_ERR;
	}

	incrementRadioUpdateCounter(radioIndex);

	return RETURN_OK;
}

/* wifi_initRadio() function */
/**
* Description: This function call initializes the specified radio.
*  Implementation specifics may dictate the functionality since
*  different hardware implementations may have different initilization requirements.
* Parameters : radioIndex - The index of the radio. First radio is index 0. 2nd radio is index 1   - type INT
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
INT wifi_initRadio(INT radioIndex)
{
	UNUSED_PARAMETER(radioIndex);
	HAL_WIFI_DBG(("%s: Returning OK Stub Function\n", __FUNCTION__));
	return RETURN_OK;
}

void* wifi_lanBridgeConfigNvram_priv(void * data)
{
	INT lanTimeCount = 55 /*check every 2 seconds*/;
	FILE *file = NULL;
	char line_test[256] = {0};
	char *linePtr = line_test;
	size_t size;
	unsigned linkStatus = 0;
	int need_to_exit = FALSE;
	int ret;

	char tmp1[50] = {0}, tmp2[10] = {0},tmp3[10] = {0},tmp4[10] = {0},tmp5[10] = {0},line[200] = {0};
	int a = 0,b = 0,radioIndex = 0,max_usage = 0;
	char ch,cmd[100]={0};
	BOOL radio_enable;

	char filename[] = "/tmp/led_status-XXXXXX";
	int fd;
	mode_t old_umask;
	UNUSED_PARAMETER(data);

	old_umask = umask(S_IXUSR |
			S_IRGRP | S_IWGRP | S_IXGRP |
			S_IROTH | S_IWOTH | S_IXOTH);
	fd = mkstemp(filename);
	if (fd != -1) {
		umask(old_umask);
		close(fd);
	} else {
		HAL_WIFI_ERR(("%s: mkstemp failed, errno is %d, %s\n", __FUNCTION__, errno, strerror(errno)));
	}

	do {
		size = sizeof(line_test);
		file = fopen("/sys/class/net/brlan0/flags", "r");
		if (file != NULL) {
			if ((getline(&linePtr, &size, file) >= 0)) {
				sscanf(linePtr, "%x", &linkStatus);
			}
			ret = fclose(file);
			if (ret == 0) {
				file = NULL;
			} else {
				return NULL;
			}
		}
		/*	Add mask before comparing the value to cover the extra flag IFF_PROMISC
			used in OVS bridge */
#if defined(_CBR1_PRODUCT_REQ_)
		/* Check /tmp/.xfinity_hotspot_ifname_up instead
		 * since we need wait until all bridges are configured before starting wifi apps(eapd) in CBR1. */
		if (file = fopen("/tmp/.xfinity_hotspot_ifname_up", "r")) {
			linkStatus = 1;
			ret = fclose(file);
			if (ret == 0) {
				file = NULL;
			} else {
				return NULL;
			}
			HAL_WIFI_DBG(("%s, bridge has been setup, time =  %d, linkStatus = 0x%x\n", __FUNCTION__, lanTimeCount, linkStatus));
			break;
		}

#else
		if ((linkStatus & WIFI_LANBRIDGE_UP) == WIFI_LANBRIDGE_UP) {
			HAL_WIFI_DBG(("%s, bridge has been setup, time = %d, linkStatus = 0x%x\n",
				__FUNCTION__, lanTimeCount, linkStatus));
			break;
		}
#endif /* _CBR1_PRODUCT_REQ_ */
		HAL_WIFI_DBG(("%s, apply LAN bridge info. time = %d, linkStatus = 0x%x\n",
			__FUNCTION__, lanTimeCount, linkStatus));
		lanTimeCount--;

		pthread_mutex_lock(&lanBridgeThread_lock);

		if (lanBridgeThread_exit) {
			need_to_exit = TRUE;
		}

		pthread_mutex_unlock(&lanBridgeThread_lock);

		if (need_to_exit) {
			HAL_WIFI_DBG(("%s, Need to exit the thread 111\n", __FUNCTION__));
			pthread_exit(NULL);
			return NULL;
		}
		sleep(2);
	} while (lanTimeCount > 0);

	HAL_WIFI_ERR(("%s: lanTimeCount=%d\n", __FUNCTION__, lanTimeCount));

	if (system("wifi_setup.sh restart")) {
		HAL_WIFI_ERR(("%s, System command wifi_setup.sh restart failed\n", __FUNCTION__));
	}
	restart_acsd();

	lanTimeCount = 0;
	wifi_lanLinkDone = TRUE;

	wifi_setLED(0, TRUE);
	wifi_setLED(1, TRUE);

	while (1) {
		pthread_mutex_lock(&lanBridgeThread_lock);

		if (lanBridgeThread_exit) {
			need_to_exit = TRUE;
		}

		pthread_mutex_unlock(&lanBridgeThread_lock);

		if (need_to_exit) {
			HAL_WIFI_DBG(("%s, Need to exit the thread 222\n", __FUNCTION__));
			pthread_exit(NULL);
			return NULL;
		}

		for (radioIndex = 0 ; radioIndex < HAL_GET_MAX_RADIOS; radioIndex++) {
			wifi_getRadioEnable(radioIndex, &radio_enable);
			if (!radio_enable) {
				wifi_setLED(radioIndex, FALSE);
				continue;
			}

			snprintf(cmd, sizeof(cmd), "wl -i wl%d bs_data > %s", radioIndex, filename);
			if (system(cmd)) {
				HAL_WIFI_ERR(("%s, System command %s failed\n", __FUNCTION__, cmd));
			}
			file = fopen(filename, "r");
			if (file != NULL) {
				if (fgets(line, sizeof(line), file) == NULL) {
					HAL_WIFI_ERR(("%s, fgets is NULL \n", __FUNCTION__));
					line[0] = 0;
				}
				if (strstr(line,"Not up")) {
					wifi_setLED(radioIndex, FALSE);
				} else if (strstr(line,"No stations")) {
					wifi_setLED(radioIndex, TRUE);
				} else {
					max_usage = 0;
					while (fgets(line, sizeof(line)-1, file) != NULL) {
						sscanf(line,"%50s\t%10s\t%10s\t%10s\t%d.%d%c\t%10s",
							tmp1,tmp2,tmp3,tmp4,&a,&b,&ch,tmp5);
						if (a > max_usage)
							max_usage = a;
					}

					if (max_usage < 35) {
						wifi_setLED_priv(radioIndex,
							WIFI_LED_SLOW_CONTINUOUS_BLINK);
					} else if (max_usage < 75) {
						wifi_setLED_priv(radioIndex,
							WIFI_LED_MEDIUM_CONTINUOUS_BLINK);
					} else {
						wifi_setLED_priv(radioIndex,
							WIFI_LED_FAST_CONTINUOUS_BLINK);
					}
				}
				ret = fclose(file);
				if (ret == 0) {
					file = NULL;
				} else {
					return NULL;
				}
				ret = remove(filename);
				if (ret == 0) {
					file = NULL;
				} else {
					return NULL;
				}
			}
		}
		sleep(3);
	}

	WiFi_Ready_after_hal_started = TRUE;

	pthread_exit(NULL);
	return NULL;
}

int WiFiSystemRadioInit_priv(UINT32 radioIndex)
{
	UNUSED_PARAMETER(radioIndex);
	HAL_WIFI_DBG(("%s: Returning OK Stub function\n", __FUNCTION__));
	return RETURN_OK;
}

// Initializes the wifi context
INT wifi_context_init()
{
	HAL_WIFI_DBG(("%s: Enter\n", __FUNCTION__));

	if (wldm_init(HAL_RADIO_NUM_RADIOS) < 0) {
		HAL_WIFI_DBG(("%s: wldm_init(%d) failed!\n", __FUNCTION__, HAL_RADIO_NUM_RADIOS))
		return RETURN_ERR;
	}
	return RETURN_OK;
}

// Deletes the wifi context
INT wifi_context_delete()
{
	HAL_WIFI_DBG(("%s: Enter\n", __FUNCTION__));

	if (wldm_deinit() < 0) {
		HAL_WIFI_DBG(("%s: wldm_deinit() failed!\n", __FUNCTION__))
		return RETURN_ERR;
	}
	return RETURN_OK;
}

// Initializes the wifi subsystem (all radios)
INT wifi_init()		//RDKB
{
	/*TODO: Initializes the wifi subsystem*/
	INT apIndex = 0;
	INT status = 0;
	CHAR encMode[OUTPUT_STRING_LENGTH_32 + 1] = { 0 };

	static int CALLONCE = 1;

	if (0 == CALLONCE) {
		HAL_WIFI_LOG(("%s: Already intialised!!!\n", __FUNCTION__));
		return RETURN_OK;
	}

	CALLONCE=0;

	if ((status = wldm_init(-1)) < 0) {
		HAL_WIFI_ERR(("%s: wldm_init(-1) failed status = %d!\n",
			__FUNCTION__, status));
		goto ret_err;
	}

	wifi_setLED(0, FALSE);
	wifi_setLED(1, FALSE);
	HAL_WIFI_LOG(("%s: !!! \n", __FUNCTION__));

	for (apIndex = 0; apIndex < HAL_GET_MAX_APS; apIndex++) {
		beaconType[apIndex] = BEACON_TYPE_11I;
		basicAuthenticationMode[apIndex] = BASIC_AUTHENTICATION_MODE_NONE;
		wifi_setApScanFilter(apIndex, WIFI_SCANFILTER_MODE_DISABLED, NULL);
	}

	if (!lanBridgeThread_lock_created) {
		pthread_mutex_init(&lanBridgeThread_lock, NULL);
		lanBridgeThread_lock_created = TRUE;
	}

	if (wifi_lanBridgeThread) {
		HAL_WIFI_DBG(("%s: wait for the previous wifi_lanBridgeThread to exit \n", __FUNCTION__));
		pthread_mutex_lock(&lanBridgeThread_lock);
		lanBridgeThread_exit = TRUE;
		pthread_mutex_unlock(&lanBridgeThread_lock);
		HAL_WIFI_DBG(("%s: wait for the previous wifi_lanBridgeThread mutex unlock \n", __FUNCTION__));
		pthread_join(wifi_lanBridgeThread, NULL);
		HAL_WIFI_DBG(("%s: wifi_lanBridgeThread joined thread \n", __FUNCTION__));
		wifi_lanBridgeThread = (pthread_t)NULL;
	}

	for (apIndex = 0; apIndex < HAL_GET_MAX_APS; apIndex++) {
		memset(encMode, 0, sizeof(encMode));
		wifi_getApSecurityModeEnabled(apIndex, encMode);
		if ((strcmp("WPA-WPA2-Enterprise", encMode) == 0) ||
			(strcmp("WPA-WPA2-Personal", encMode) == 0)) {
			beaconType[apIndex] = BEACON_TYPE_WPA_11I;
		} else if ((strcmp("WPA2-Personal", encMode) == 0) ||
			(strcmp("WPA2-Enterprise", encMode) == 0)) {
			beaconType[apIndex] = BEACON_TYPE_11I;
		} else if ((strcmp("WPA-Personal", encMode) == 0) ||
			(strcmp("WPA-Enterprise", encMode) == 0)) {
			beaconType[apIndex] = BEACON_TYPE_WPA;
		} else {
			beaconType[apIndex] = BEACON_TYPE_NONE;
		}
	}

	/* Needs to enable/disble WiFi LED based on radio enable/disable case */
	wifi_CheckAndConfigureLEDS();

#ifndef WIFI_HAL_VERSION_3
	/* apply BridgeInfo once to change nvram setting for lan bridge MAC address */
	if (!wifi_lanBridgeThread) {
		pthread_mutex_lock(&lanBridgeThread_lock);
		lanBridgeThread_exit = FALSE;
		pthread_mutex_unlock(&lanBridgeThread_lock);

		if (pthread_create(&wifi_lanBridgeThread, NULL, wifi_lanBridgeConfigNvram_priv, NULL)) {
			HAL_WIFI_ERR(("%s, fail to create wifi_lanBridgeThread = 0x%x\n", __FUNCTION__, wifi_lanBridgeThread));
		} else {
			HAL_WIFI_DBG(("%s, create lanBridgeThread = 0x%x\n", __FUNCTION__, wifi_lanBridgeThread));;
		}
	}
#else
#if defined(_CBR2_PRODUCT_REQ_)
	if (v_secure_system("wifi_setup.sh restart &")) {
		HAL_WIFI_DBG(("%s: system wifi_setup.sh restart failed\n", __FUNCTION__));
	}
#elif !defined(_CBR1_PRODUCT_REQ_)
	if (system("wifi_setup.sh restart"))
		HAL_WIFI_DBG(("%s: system wifi_setup.sh restart failed\n", __FUNCTION__));
#endif /* _CBR2_PRODUCT_REQ_ !_CBR1_PRODUCT_REQ_ */
#endif
	if (!wifi_apiThread) {
		if (wifi_api_socket_init() >= 0 ) {
			if (pthread_create(&wifi_apiThread, NULL, (void *)&wifi_api_thread_main_loop, NULL)) {
				HAL_WIFI_ERR(("create wifi_apiThread fail. \n"));
				pthread_cancel(wifi_apiThread);
				return RETURN_ERR;
			} else {
				HAL_WIFI_LOG(("create wifi_apiThread success. \n"));
			}
		}
	}

#if defined(RDKB_TIMER)
	/*wldm apply variables*/
	gApplySetCounter = 0;
	Pending = 0;

	/*create a thread for wldm apply timer*/
	if (!wifi_timerThread) {
		if (pthread_create(&wifi_timerThread, NULL, (void *)&wifi_timer_thread_main_loop, NULL)) {
			HAL_WIFI_DBG(("create thread fail. \n"));
			fprintf(stderr,"%s:%d: wifi_timerThread thread create failure!\n",__FUNCTION__,__LINE__);
			pthread_cancel(wifi_timerThread);
			return RETURN_ERR;
		} else {
			HAL_WIFI_DBG(("create timer thread success. \n"));
			printf("%s:%d: wifi_timerThread created!\n",__FUNCTION__,__LINE__);
		}
	}
#endif /* End of defined(RDKB_TIMER)*/

	return RETURN_OK;

ret_err:

	return RETURN_ERR;
}

// Initializes the wifi subsystem (used for TDK)
INT wifi_init_hal()		//RDKB
{
	/* set flag to be ready */
	WiFi_Ready_after_hal_started = TRUE;

	HAL_WIFI_LOG(("%s: WiFi_Ready_after_hal_started set to TRUE!\n", __FUNCTION__));

	return RETURN_OK;
}

static void wifi_sort_scan_results(wifi_neighbor_ap2_t **neighbor, unsigned int idx, unsigned int *num, int num_ssid_match)
{
	unsigned int i;
	int j;
	wifi_neighbor_ap2_t *ptr = *neighbor;
	if (gscanfilter[idx].mode == WIFI_SCANFILTER_MODE_DISABLED) {
		/* No change needed if scan filter is disabled */
		return;
	} else if (gscanfilter[idx].mode == WIFI_SCANFILTER_MODE_ENABLED) {
		/* Return only the matching ssid's */
		wifi_neighbor_ap2_t *neighbor_match = NULL;
		neighbor_match = (wifi_neighbor_ap2_t*) malloc(num_ssid_match * sizeof(wifi_neighbor_ap2_t));
		if (neighbor_match == NULL) {
			HAL_WIFI_ERR(("%s: Can't aloocate space for neighbor_match, bailing\n", __FUNCTION__));
			return;
		}

		for (i = 0, j = 0; (i < *num) && (j < num_ssid_match); ++i) {
			if (!strncmp(ptr[i].ap_SSID, gscanfilter[idx].essid, sizeof(ptr[i].ap_SSID))) {
				memcpy(&neighbor_match[j], &ptr[i], sizeof(ptr[i]));
				++j;
			}
		}
		free(*neighbor);
		*neighbor = neighbor_match;
		*num = num_ssid_match;
	} else if (gscanfilter[idx].mode == WIFI_SCANFILTER_MODE_FIRST) {
		/* Return the matching SSID's first */
		wifi_neighbor_ap2_t temp;

		for (i = 0, j = 0; (i < *num) && (j < num_ssid_match); ++i) {
			if (!strncmp(ptr[i].ap_SSID, gscanfilter[idx].essid, sizeof(ptr[i].ap_SSID))) {
				if ((unsigned int)j != i) {
					/* Swap the entries */
					memcpy(&temp, &ptr[j], sizeof(wifi_neighbor_ap2_t));
					memcpy(&ptr[j], &ptr[i], sizeof(wifi_neighbor_ap2_t));
					memcpy(&ptr[i], &temp, sizeof(wifi_neighbor_ap2_t));
					++j;
				} else {
					/* No need to swap*/
					++i;
					j = i;
				}
			}
		}
	}
}

static INT get_neighboring_wifi_status(INT radioIndex, wifi_neighbor_ap2_t **neighbor_ap_array,
	UINT *output_array_size)
{
	unsigned int len, i, num, scanfilter_ssid_match = 0;
	wldm_neighbor_ap2_t *wldm_neighbor_ptr;
	wifi_neighbor_ap2_t *wifi_neighbor_ptr;

	HAL_WIFI_DBG(("%s: radioIndex %d\n", __FUNCTION__, radioIndex));

	RADIO_INDEX_ASSERT(radioIndex);
	RADIO_NOT_UP_ASSERT(radioIndex);
	NULL_PTR_ASSERT(output_array_size);

	*output_array_size = 0;
	*neighbor_ap_array = NULL;

	if (wldm_xbrcm_scan(CMD_GET, radioIndex, NULL, &num, "num_scan_results", NULL) < 0) {
		HAL_WIFI_ERR(("%s: radioIndex %d wldm_xbrcm_scan failed, errno [%d]\n",
			__FUNCTION__, radioIndex, errno));
		/*	some caller apps will check errno for error cases.
			ideally, when driver return EINVAL, wifi_hal change it to EAGAIN
			but errno may be changed by other system call in the middle
			To simplify, use EAGAIN for all error cases */
		errno = EAGAIN;
		return RETURN_ERR;
	}

	len = num * sizeof(wldm_neighbor_ap2_t);
	wldm_neighbor_ptr = (wldm_neighbor_ap2_t *)malloc(len);

	NULL_PTR_ASSERT(wldm_neighbor_ptr);
	memset(wldm_neighbor_ptr, 0, len);

	if (wldm_xbrcm_scan(CMD_GET, radioIndex, (void *)wldm_neighbor_ptr, &num,
		"scan_results", NULL) < 0) {
		HAL_WIFI_ERR(("%s:%d wldm_xbrcm_scan failed, errno [%d]\n",
			__FUNCTION__, __LINE__, errno));
		free(wldm_neighbor_ptr);
		errno = EAGAIN;
		return RETURN_ERR;
	}
	len = num * sizeof(wifi_neighbor_ap2_t);
	wifi_neighbor_ptr = (wifi_neighbor_ap2_t *)malloc(len);
	if (wifi_neighbor_ptr == NULL) {
		HAL_WIFI_ERR(("%s:%d Malloc failed\n", __FUNCTION__, __LINE__));
		free(wldm_neighbor_ptr);
		return RETURN_ERR;
	}
	for (i = 0; i < num; ++i) {
		snprintf(wifi_neighbor_ptr[i].ap_SSID, sizeof(wifi_neighbor_ptr[i].ap_SSID),
			"%s", wldm_neighbor_ptr[i].ap_SSID);
		snprintf(wifi_neighbor_ptr[i].ap_BSSID, sizeof(wifi_neighbor_ptr[i].ap_BSSID),
			"%s", wldm_neighbor_ptr[i].ap_BSSID);
		snprintf(wifi_neighbor_ptr[i].ap_Mode, sizeof(wifi_neighbor_ptr[i].ap_Mode),
			"%s", wldm_neighbor_ptr[i].ap_Mode);
		wifi_neighbor_ptr[i].ap_Channel = wldm_neighbor_ptr[i].ap_Channel;
		wifi_neighbor_ptr[i].ap_SignalStrength = wldm_neighbor_ptr[i].ap_SignalStrength;
		snprintf(wifi_neighbor_ptr[i].ap_SecurityModeEnabled,
			sizeof(wifi_neighbor_ptr[i].ap_SecurityModeEnabled), "%s",
			wldm_neighbor_ptr[i].ap_SecurityModeEnabled);
		snprintf(wifi_neighbor_ptr[i].ap_EncryptionMode,
			sizeof(wifi_neighbor_ptr[i].ap_EncryptionMode), "%s",
			wldm_neighbor_ptr[i].ap_EncryptionMode);
		snprintf(wifi_neighbor_ptr[i].ap_OperatingFrequencyBand,
			sizeof(wifi_neighbor_ptr[i].ap_OperatingFrequencyBand), "%s",
			wldm_neighbor_ptr[i].ap_OperatingFrequencyBand);
		snprintf(wifi_neighbor_ptr[i].ap_SupportedStandards,
			sizeof(wifi_neighbor_ptr[i].ap_SupportedStandards), "%s",
			wldm_neighbor_ptr[i].ap_SupportedStandards);
		snprintf(wifi_neighbor_ptr[i].ap_OperatingStandards,
			sizeof(wifi_neighbor_ptr[i].ap_OperatingStandards), "%s",
			wldm_neighbor_ptr[i].ap_OperatingStandards);
		snprintf(wifi_neighbor_ptr[i].ap_OperatingChannelBandwidth,
			sizeof(wifi_neighbor_ptr[i].ap_OperatingChannelBandwidth), "%s",
			wldm_neighbor_ptr[i].ap_OperatingChannelBandwidth);
		wifi_neighbor_ptr[i].ap_BeaconPeriod = wldm_neighbor_ptr[i].ap_BeaconPeriod;
		wifi_neighbor_ptr[i].ap_Noise = wldm_neighbor_ptr[i].ap_Noise;
		snprintf(wifi_neighbor_ptr[i].ap_BasicDataTransferRates,
			sizeof(wifi_neighbor_ptr[i].ap_BasicDataTransferRates), "%s",
			wldm_neighbor_ptr[i].ap_BasicDataTransferRates);
		snprintf(wifi_neighbor_ptr[i].ap_SupportedDataTransferRates,
			sizeof(wifi_neighbor_ptr[i].ap_SupportedDataTransferRates), "%s",
			wldm_neighbor_ptr[i].ap_SupportedDataTransferRates);
		wifi_neighbor_ptr[i].ap_DTIMPeriod = wldm_neighbor_ptr[i].ap_DTIMPeriod;
		wifi_neighbor_ptr[i].ap_ChannelUtilization =
			wldm_neighbor_ptr[i].ap_ChannelUtilization;
		if (gscanfilter[radioIndex].mode != WIFI_SCANFILTER_MODE_DISABLED) {
			if (!strncmp(wifi_neighbor_ptr[i].ap_SSID, gscanfilter[radioIndex].essid,
				sizeof(wifi_neighbor_ptr[i].ap_SSID))) {
				++scanfilter_ssid_match;
			}
		}
	}
	wifi_sort_scan_results(&wifi_neighbor_ptr, radioIndex, &num, scanfilter_ssid_match);
	*output_array_size = num;
	*neighbor_ap_array = wifi_neighbor_ptr;
	free(wldm_neighbor_ptr);
	errno = 0;
	return RETURN_OK;
}

#ifdef WIFI_HAL_VERSION_3_PHASE2
INT wifi_getNeighboringWiFiStatus(INT radioIndex, BOOL scan, wifi_neighbor_ap2_t **neighbor_ap_array,
	UINT *output_array_size)
#else /* WIFI_HAL_VERSION_3_PHASE2 */
INT wifi_getNeighboringWiFiStatus(INT radioIndex, wifi_neighbor_ap2_t **neighbor_ap_array,
	UINT *output_array_size)
#endif /* WIFI_HAL_VERSION_3_PHASE2 */
{
	HAL_WIFI_DBG(("%s: radioIndex %d\n", __FUNCTION__, radioIndex));

	return get_neighboring_wifi_status(radioIndex, neighbor_ap_array, output_array_size);
}

/* radioIndex = 2 for 6G */
#define DEFAULT_SCAN_DWELL_TIME(radioIndex) (radioIndex == 2) ? 110 : 20

/* convert valid wifi_neighborScanMode_t to corresponding wldm_neighborScanMode_t
 * when there is new enum added for wifi_neighborScanMode_t, need to change this funciton */
static wldm_neighborScanMode_t  map_wifi_neighborScanMode(wifi_neighborScanMode_t scan_mode) {
	switch (scan_mode) {
		case WIFI_RADIO_SCAN_MODE_NONE:
			return WLDM_RADIO_SCAN_MODE_NONE;
		case WIFI_RADIO_SCAN_MODE_FULL:
			return WLDM_RADIO_SCAN_MODE_FULL;
		case WIFI_RADIO_SCAN_MODE_ONCHAN:
			return WLDM_RADIO_SCAN_MODE_ONCHAN;
		case WIFI_RADIO_SCAN_MODE_OFFCHAN:
			return WLDM_RADIO_SCAN_MODE_OFFCHAN;
		case WIFI_RADIO_SCAN_MODE_SURVEY:
			return WLDM_RADIO_SCAN_MODE_SURVEY;
		default:
			return WLDM_RADIO_SCAN_MODE_INVALID;
	}
}

static INT start_neighbor_scan(INT radioIndex, wldm_neighborScanMode_t scan_mode, INT dwell_time,
	UINT chan_num, UINT *chan_list)
{
	int ret = RETURN_OK, errno_tmp = 0;
	unsigned int len;
	wldm_scan_params_t wldm_scan_params;

	HAL_WIFI_DBG(("%s: radioIndex %d scan_mode %d dwell_time %d num_channels %d\n", __FUNCTION__,
		radioIndex, scan_mode, dwell_time, chan_num));

	RADIO_INDEX_ASSERT(radioIndex);
	RADIO_NOT_UP_ASSERT(radioIndex);

	errno = 0;
	memset(&wldm_scan_params, 0, sizeof(wldm_scan_params));
	wldm_scan_params.scan_mode = scan_mode;
	wldm_scan_params.dwell_time = dwell_time;
	wldm_scan_params.num_channels = chan_num;

	if (chan_num > 0) {
		len = chan_num * sizeof(unsigned int);
		wldm_scan_params.chan_list = (unsigned int *)malloc(len);
		if (wldm_scan_params.chan_list == NULL) {
			HAL_WIFI_ERR(("%s:%d, Malloc returned error!\n", __FUNCTION__, __LINE__));
			return RETURN_ERR;
		}
		memcpy(wldm_scan_params.chan_list, chan_list, len);
	} else {
		wldm_scan_params.chan_list = NULL;
	}
	if (wldm_xbrcm_scan(CMD_SET_IOCTL, radioIndex, (void *)&wldm_scan_params, &len, "scan",
		NULL) != 0) {
		HAL_WIFI_ERR(("%s: Failed Scan radioIndex [%d] errno [%d]\n",
			__FUNCTION__, radioIndex, errno));
		errno_tmp = EAGAIN;
		ret = RETURN_ERR;
	}

	if (wldm_scan_params.chan_list) {
		free(wldm_scan_params.chan_list);
	}

	errno = errno_tmp;
	return ret;
}

INT wifi_startNeighborScan(INT apIndex, wifi_neighborScanMode_t scan_mode, INT dwell_time,
	UINT chan_num, UINT *chan_list)
{
	INT radioIndex;

	AP_INDEX_ASSERT(apIndex);
	wldm_neighborScanMode_t wldm_scan_mode = map_wifi_neighborScanMode(scan_mode);
	if (wldm_scan_mode == WLDM_RADIO_SCAN_MODE_INVALID) {
		HAL_WIFI_ERR(("%s: wifi_neighborScanMode_t %d is not handled by WLDM yet\n",
		 __FUNCTION__, scan_mode));
		return  RETURN_ERR;
	}
	radioIndex = HAL_AP_IDX_TO_HAL_RADIO(apIndex);

	HAL_WIFI_DBG(("%s: radioIndex%d scan_mode %d dwell_time %d numCh %d\n",
		 __FUNCTION__, radioIndex, scan_mode, dwell_time, chan_num));
	return start_neighbor_scan(radioIndex, wldm_scan_mode, dwell_time, chan_num, chan_list);
}

INT wifi_getApAssociatedDeviceTidStatsResult(
	INT radioIndex,
	mac_address_t *clientMacAddress,
	wifi_associated_dev_tid_stats_t *tid_stats,
	uint64_t *handle)
{
/* NOT IMPLEMENTED */
	(void)radioIndex;
	(void)clientMacAddress;
	memset(tid_stats, 0, sizeof(*tid_stats));
	*handle = 0;
	return RETURN_OK;
}

/* wifi_reset() function */
/**
* Description: Resets the Wifi subsystem. This includes reset of all AP varibles.
* Implementation specifics may dictate what is actualy reset since
* different hardware implementations may have different requirements.
* Parameters : None
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
INT wifi_reset()
{
	INT radioIdx;
	int ret = RETURN_OK;

	HAL_WIFI_DBG(("%s\n", __FUNCTION__));
	ret = system("wifi_setup.sh restart");
	if (ret != 0) {
		HAL_WIFI_ERR(("%s wifi_setup.sh restart failed\n", __FUNCTION__));
		ret = RETURN_ERR;
	}
	for (radioIdx = 0; radioIdx < HAL_GET_MAX_RADIOS; radioIdx++) {
		incrementRadioUpdateCounter(radioIdx);
	}
	return ret;
}

/* wifi_down() function */
/**
* Description:
* Turns off transmit power to all radios.
* Implementation specifics may dictate some functionality since
* different hardware implementations may have different requirements.
* Parameters : None
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
INT wifi_down()
{
	INT returnStatus = RETURN_OK;
	INT radioCount = 0;
	HAL_WIFI_DBG(("%s: Enter\n", __FUNCTION__));

	for (radioCount = 0; radioCount < HAL_GET_MAX_RADIOS; radioCount++) {
		if ((returnStatus = wifi_setRadioEnable(radioCount, 0)) != RETURN_OK) {
			HAL_WIFI_DBG(("%s: wifi_setRadioEnable Fail %d\n", __func__, radioCount));
			return returnStatus;
		}
	}
	HAL_WIFI_DBG(("%s: Completed\n", __FUNCTION__));
	return returnStatus;
}

/* wifi_createInitialConfigFiles() function */
/**
* Description:
* This function creates wifi configuration files. The format
* and content of these files are implementation dependent. This function call is
* used to trigger this task if necessary. Some implementations may not need this
* function. If an implementation does not need to create config files the function call can
* do nothing and return RETURN_OK.
* Parameters : None
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
INT wifi_createInitialConfigFiles()
{
	//TODO: creates initial implementation dependent configuration files that are later used for variable storage.  Not all implementations may need this function.  If not needed for a particular implementation simply return no-error (0)

	HAL_WIFI_DBG(("%s, not implemented!!!\n", __FUNCTION__));
	/* Not Applicable */
	return RETURN_OK;
}

/**********************************************************************************
 *
 * Wifi radio level function prototypes
 *
**********************************************************************************/

INT wifi_getATMCapable(BOOL *output_bool)
{
	if (output_bool == NULL) {
		HAL_WIFI_ERR(("%s, output_bool is NULL \n", __FUNCTION__));
		return RETURN_ERR;
	}
	*output_bool = TRUE;

	HAL_WIFI_DBG(("%s, output = %d\n", __FUNCTION__, *output_bool));

	return RETURN_OK;
}

INT wifi_setATMEnable(BOOL enable)
{
#if !defined(_CBR1_PRODUCT_REQ_)
	int value, radioIndex;
	unsigned int len = sizeof(value);

	HAL_WIFI_DBG(("%s: enable = %d\n", __FUNCTION__, enable));

	value = enable ? 3 : 0;
	/* API definition does not indicate radioIndex or apIndex, so set API enables or disables
	 * all WiFi interfaces
	 */
	for (radioIndex = 0; radioIndex < HAL_GET_MAX_RADIOS; radioIndex++) {
		if (wldm_xbrcm_atm(CMD_SET_NVRAM | CMD_SET_IOCTL,
			radioIndex, &value, &len, "enable", NULL) < 0) {
			HAL_WIFI_ERR(("%s: wldm_xbrcm_atm CMD_SET_NVRAM|IOCTL %d failed.\n",
				__FUNCTION__, value));
			return RETURN_ERR;
		}
	}

	return RETURN_OK;
#else
	return RETURN_ERR;
#endif /* !_CBR1_PRODUCT_REQ_ */
}

INT wifi_getATMEnable(BOOL *output_enable)
{
#if !defined(_CBR1_PRODUCT_REQ_)
	int value, radioIndex;
	unsigned int len = sizeof(value);

	/* API definition does not indicate radioIndex or apIndex, so get API returns status of
	 * last WiFi interface
	 */
	for (radioIndex = 0; radioIndex < HAL_GET_MAX_RADIOS; radioIndex++) {
		if (wldm_xbrcm_atm(CMD_GET, radioIndex, &value, &len, "enable", NULL) < 0) {
			HAL_WIFI_ERR(("%s: wldm_xbrcm_atm CMD_GET failed.\n", __FUNCTION__));
			return RETURN_ERR;
		}
		*output_enable = (value == 3) ? 1 : 0;

		HAL_WIFI_DBG(("%s: radioIndex = %d, enable = %d\n", __FUNCTION__, radioIndex,
			*output_enable));
	}
	return RETURN_OK;
#else
	return RETURN_ERR;
#endif /* !_CBR1_PRODUCT_REQ_ */
}

INT wifi_setApATMAirTimePercent(INT apIndex, UINT ap_AirTimePercent)
{
#if !defined(_CBR1_PRODUCT_REQ_)
	unsigned int len = sizeof(ap_AirTimePercent);

	HAL_WIFI_DBG(("%s: apIndex = %d, ap_AirTimePercent = %d\n", __FUNCTION__, apIndex,
		ap_AirTimePercent));
	AP_INDEX_ASSERT(apIndex);

	if (ap_AirTimePercent > 100) {
		HAL_WIFI_ERR(("%s: ap_AirTimePercent %d invalid.\n",
			__FUNCTION__, ap_AirTimePercent));
		return RETURN_ERR;
	}
	if (wldm_xbrcm_atm(CMD_SET_NVRAM | CMD_SET_IOCTL,
		apIndex, &ap_AirTimePercent, &len, "bssperc", NULL) < 0) {
		HAL_WIFI_ERR(("%s: wldm_xbrcm_atm bssperc CMD_SET_NVRAM|IOCTL %d failed.\n",
			__FUNCTION__, ap_AirTimePercent));
		return RETURN_ERR;
	}
	return RETURN_OK;
#else
	return RETURN_ERR;
#endif /* !_CBR1_PRODUCT_REQ_ */
}

INT wifi_getApATMAirTimePercent(INT apIndex, UINT *output_ap_AirTimePercent)
{
#if !defined(_CBR1_PRODUCT_REQ_)
	unsigned int len = sizeof(*output_ap_AirTimePercent);

	AP_INDEX_ASSERT(apIndex);
	if (wldm_xbrcm_atm(CMD_GET, apIndex, output_ap_AirTimePercent, &len, "bssperc", NULL) < 0) {
		HAL_WIFI_ERR(("%s: wldm_xbrcm_atm bssperc CMD_GET failed.\n", __FUNCTION__));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: apIndex = %d, output_ap_AirTimePercent = %d\n", __FUNCTION__, apIndex,
		*output_ap_AirTimePercent));
	return RETURN_OK;
#else
	return RETURN_ERR;
#endif /* !_CBR1_PRODUCT_REQ_ */
}

INT wifi_getApATMSta(INT apIndex, UCHAR *output_sta_MAC_ATM_array, UINT  buf_size)
{
#if !defined(_CBR1_PRODUCT_REQ_)
	char assoclist[OUTPUT_STRING_LENGTH_1024], *pmac, *psave = NULL;
	unsigned int assoclist_len = sizeof(assoclist);
	char stas[WL_MAX_ASSOC_STA][MAC_STR_LEN] = {{0}};
	int stapercs[WL_MAX_ASSOC_STA], num_stas = 0, i;
	wldm_atm_staperc_t wldm_staperc;
	unsigned int wldm_staperc_len = sizeof(wldm_staperc);
	int output_array_len;

	AP_INDEX_ASSERT(apIndex);

	/* Get assoclist */
	memset(assoclist, 0, sizeof(assoclist));
	if (wldm_xbrcm_ap(CMD_GET, apIndex, assoclist, &assoclist_len, "assoclist", NULL) < 0) {
		HAL_WIFI_ERR(("%s: wldm_xbrcm_ap assoclist CMD_GET failed.\n", __FUNCTION__));
		return RETURN_ERR;
	}
	HAL_WIFI_DBG(("%s, MAC of associated STAs = %s\n", __FUNCTION__, assoclist));

	/* Get number of STA's */
	pmac = strtok_r(assoclist, ",", &psave);
	while (pmac != NULL) {
		memcpy(stas[num_stas], pmac, sizeof(stas[num_stas]));
		num_stas++;
		pmac = strtok_r(NULL, ",", &psave);
	}
	memset(output_sta_MAC_ATM_array, 0, buf_size);
	if (!num_stas) {
		HAL_WIFI_ERR(("%s, No STA's associated apIndex [%d] \n", __FUNCTION__, apIndex));
		return RETURN_OK;
	}

	for (i = 0; i < num_stas; i++) {
		if (snprintf(wldm_staperc.macstr, sizeof(wldm_staperc.macstr), "%s", stas[i]) < 0) {
			HAL_WIFI_ERR(("%s snprintf failed for wldm_staperc\n", __FUNCTION__));
			return RETURN_ERR;
		}
		if (wldm_xbrcm_atm(CMD_GET, apIndex, (void *) &wldm_staperc, &wldm_staperc_len,
			"staperc", NULL) < 0) {
			HAL_WIFI_ERR(("%s: wldm_xbrcm_atm staperc CMD_GET failed.\n", __FUNCTION__));
			return RETURN_ERR;
		}
		stapercs[i] = (int) wldm_staperc.perc;
	}

	/* output_sta_MAC_ATM_array contains the atm array in format of
	 * "$MAC $ATM_percent|$MAC $ATM_percent|$MAC $ATM_percent"
	 */
	for (i = 0, output_array_len = 0; i < num_stas; i++) {
		output_array_len += snprintf((char *)&output_sta_MAC_ATM_array[output_array_len],
			sizeof(stas[i]) + 1 + 1 + 3, "%s %03d|", stas[i], stapercs[i]);
	}
	output_sta_MAC_ATM_array[--output_array_len] = '\0';

	HAL_WIFI_DBG(("%s: apIndex = %d, output_sta_MAC_ATM_array = %s output_array_len = %d\n",
		__FUNCTION__, apIndex, output_sta_MAC_ATM_array, output_array_len));
	return RETURN_OK;
#else
	return RETURN_ERR;
#endif /* !_CBR1_PRODUCT_REQ_ */
}

#if !defined(WIFI_HAL_VERSION_GE_3_0)
INT wifi_setApATMSta(INT apIndex, UCHAR *sta_MAC, UINT sta_AirTimePercent)
{
#if !defined(_CBR1_PRODUCT_REQ_)
	wldm_atm_staperc_t wldm_staperc;
	unsigned int wldm_staperc_len = sizeof(wldm_staperc);

	HAL_WIFI_DBG(("%s: apIndex = %d, sta_MAC = %s sta_AirTimePercent = %d\n", __FUNCTION__,
		apIndex, sta_MAC, sta_AirTimePercent));
	AP_INDEX_ASSERT(apIndex);

	snprintf(wldm_staperc.macstr, sizeof(wldm_staperc.macstr), "%s", sta_MAC);
	wldm_staperc.perc = sta_AirTimePercent;
	/* Note that this version supports CMD_SET_IOCTL only. CMD_SET_NVRAM is not supported
	 * for now and the support may require a daemon to apply changes thru IOCTL.
	 */
	if (wldm_xbrcm_atm(CMD_SET_IOCTL, apIndex, (void *) &wldm_staperc, &wldm_staperc_len,
		"staperc", NULL) < 0) {
		HAL_WIFI_ERR(("%s: wldm_xbrcm_atm staperc CMD_SET failed.\n", __FUNCTION__));
		return RETURN_ERR;
	}
	return RETURN_OK;
#else
	return RETURN_ERR;
#endif /* !_CBR1_PRODUCT_REQ_ */
}
#endif

INT wifi_setWldmMsglevel(unsigned long msglevel)
{
	return wldm_set_wldm_msglevel(msglevel);
}

//Get the total number of radios in this wifi subsystem
INT wifi_getRadioNumberOfEntries(ULONG *output) //Tr181
{
	int radios = 0;
	if (NULL == output) {
		HAL_WIFI_ERR(("%s, output parameter error!!!\n", __FUNCTION__));
		return RETURN_ERR;
	}

	radios = wldm_get_radios();
	if (radios < 0) {
		HAL_WIFI_ERR(("%s: wldm_get_radios return radios=%d\n", __FUNCTION__, radios));
		return RETURN_ERR;
	}
	HAL_WIFI_DBG(("%s: wldm_get_radios return radios=%d\n", __FUNCTION__, radios));

	*output = (ULONG) radios;
	return RETURN_OK;
}

//Get the total number of SSID entries in this wifi subsystem
INT wifi_getSSIDNumberOfEntries(ULONG *output) //Tr181
{
	UINT total;
	int len = sizeof(total);

	if (NULL == output) {
		HAL_WIFI_ERR(("%s: output parameter error!!!\n", __FUNCTION__));
		return RETURN_ERR;
	}

	if (wldm_SSIDNumberOfEntries(CMD_GET, -1, &total, &len, NULL, NULL) < 0) {
		HAL_WIFI_DBG(("%s: wldm_SSIDNumberOfEntries() CMD_GET failed!\n", __FUNCTION__));
		return RETURN_ERR;
	}
	*output = (ULONG) total;

	HAL_WIFI_DBG(("%s: output = %d\n", __FUNCTION__, *output));

	return RETURN_OK;
}

//Get the Radio enable config parameter
INT wifi_getRadioEnable(INT radioIndex, BOOL *output_bool)	// Tr181
{
	int len = sizeof(BOOL);
	int returnStatus = 0;

	NULL_PTR_ASSERT(output_bool);
	RADIO_INDEX_ASSERT(radioIndex);

	returnStatus = wldm_Radio_Enable(CMD_GET_NVRAM, radioIndex, output_bool, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_ERR(("%s: wldm_Radio_Enable CMD_GET_NVRAM failed, Status = %d\n",
				__FUNCTION__, returnStatus));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: radioIndex = %d, output_bool = %d\n", __FUNCTION__, radioIndex, *output_bool));
	return RETURN_OK;
}

//Set the Radio enable config parameter
INT wifi_setRadioEnable(INT radioIndex, BOOL enable)	//RDKB
{
	int returnStatus;
	int len = sizeof(enable);

	HAL_WIFI_LOG(("%s: Enter, radioIndex = %d, enable = %d\n", __FUNCTION__, radioIndex, enable));
	RADIO_INDEX_ASSERT(radioIndex);

	returnStatus = wldm_Radio_Enable(CMD_SET, radioIndex, &enable, &len, NULL, NULL);
	if (returnStatus != RETURN_OK)
	{
		HAL_WIFI_DBG(("%s: wldm_Radio_Enable CMD_GET Failed, Status = %d\n",
			__FUNCTION__, returnStatus));
		return RETURN_ERR;
	}

	HAL_WIFI_LOG(("%s: radioIndex = %d, enable = %d\n", __FUNCTION__, radioIndex, enable));

#ifdef RDKB_TIMER
	wifi_set_timer_variable();
#endif

	//Turn off the LED if Radio is off
	if( FALSE == enable )
	{
		wifi_setLED(radioIndex, FALSE);
		/* wifi_CheckAndDisableWiFiLED( radioIndex ); */
	}
	return RETURN_OK;
}

//Get the Radio enable status
INT wifi_getRadioStatus(INT radioIndex, BOOL *output_bool)	//RDKB
{
	int len = sizeof(BOOL);
	int returnStatus;

	HAL_WIFI_DBG(("%s: Enter, radioIndex = %d\n", __FUNCTION__, radioIndex));

	NULL_PTR_ASSERT(output_bool);
	RADIO_INDEX_ASSERT(radioIndex);

	*output_bool = FALSE;

	returnStatus = wldm_Radio_Status(CMD_GET, radioIndex, output_bool, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_Radio_Status CMD_GET Failed, Status = %d\n",
			__FUNCTION__, returnStatus));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: radioIndex = %d, output_bool = %d\n", __FUNCTION__, radioIndex, *output_bool));

	return RETURN_OK;
}

//Get the Radio Interface name from platform, eg "wifi0"
INT wifi_getRadioIfName(INT radioIndex, CHAR *output_string) //Tr181
{
	unsigned int len;

	HAL_WIFI_DBG(("%s: Enter, radioIndex = %d\n", __FUNCTION__, radioIndex));

	NULL_PTR_ASSERT(output_string);
	RADIO_INDEX_ASSERT(radioIndex);

	len = strlen(wldm_get_radio_osifname(radioIndex));

	*output_string = '\0';

	strncpy(output_string, wldm_get_radio_osifname(radioIndex), len + 1);

	HAL_WIFI_DBG(("%s: radioIndex = %d, output_string = %s\n", __FUNCTION__, radioIndex, output_string));

	return RETURN_OK;
}

//Get the maximum PHY bit rate supported by this interface. eg: "216.7 Mb/s", "1.3 Gb/s"
//The output_string is a max length 64 octet string that is allocated by the RDKB code. Implementations must ensure that strings are not longer than this.
INT wifi_getRadioMaxBitRate(INT radioIndex, CHAR *output_string)	//RDKB
{
	int returnStatus;
	int MaxBitRate = 0;
	int len = sizeof(MaxBitRate);

	HAL_WIFI_DBG(("%s: Enter, radioIndex = %d\n", __FUNCTION__, radioIndex));
	NULL_PTR_ASSERT(output_string);
	RADIO_INDEX_ASSERT(radioIndex);

	output_string[0] = '\0';

	returnStatus = wldm_Radio_MaxBitRate(CMD_GET, radioIndex, &MaxBitRate, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_Radio_MaxBitRate CND_GET Failed, Status = %d\n",
			__FUNCTION__, returnStatus));
		return RETURN_ERR;
	}

	snprintf(output_string, 64, "%d Mb/s", (MaxBitRate/2)); /* MaxBitRate in Kbps/2 = BR in Mbps */

	HAL_WIFI_DBG(("%s: radioIndex = %d, output_string = %s\n", __FUNCTION__, radioIndex, output_string));

	return RETURN_OK;
}

//Get Supported frequency bands at which the radio can operate. eg: "2.4GHz,5GHz"
//The output_string is a max length 64 octet string that is allocated by the RDKB code. Implementations must ensure that strings are not longer than this.
INT wifi_getRadioSupportedFrequencyBands(INT radioIndex, CHAR *output_string)	//RDKB
{
	CHAR bands_str[OUTPUT_STRING_LENGTH_64];
	INT len = OUTPUT_STRING_LENGTH_64;
	INT returnStatus;

	HAL_WIFI_DBG(("%s: Enter, radioIndex=[%d]\n", __FUNCTION__, radioIndex));
	NULL_PTR_ASSERT(output_string);
	RADIO_INDEX_ASSERT(radioIndex);

	output_string[0] = '\0';

	returnStatus = wldm_Radio_SupportedStandards(CMD_GET, radioIndex, bands_str, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_ERR(("%s: Fail, wldm_Radio_SupportedStandards CMD_GET fail, Status = %d\n",
			__FUNCTION__, returnStatus));
		return RETURN_ERR;
	}

	if (strchr(bands_str, 'b') && strchr(bands_str, 'a')) {
		snprintf(output_string, OUTPUT_STRING_LENGTH_64, "2.4GHz, 5GHz");
	} else if (!strncmp(bands_str, WIFI_SUPPORTEDSTANDARDS_NONAX_2G, sizeof(bands_str)) ||
		!strncmp(bands_str, WIFI_SUPPORTEDSTANDARDS_AX_2G, sizeof(bands_str))) {
		snprintf(output_string, OUTPUT_STRING_LENGTH_64, "2.4GHz");
	} else if (!strncmp(bands_str, WIFI_SUPPORTEDSTANDARDS_NONAX_5G, sizeof(bands_str)) ||
		!strncmp(bands_str, WIFI_SUPPORTEDSTANDARDS_AX_5G, sizeof(bands_str))) {
		snprintf(output_string, OUTPUT_STRING_LENGTH_64, "5GHz");
	} else if (!strncmp(bands_str, WIFI_SUPPORTEDSTANDARDS_NONAX_6G, sizeof(bands_str)) ||
		!strncmp(bands_str, WIFI_SUPPORTEDSTANDARDS_AX_6G, sizeof(bands_str))) {
		snprintf(output_string, OUTPUT_STRING_LENGTH_64, "6GHz");
	} else {
		HAL_WIFI_DBG(("%s: Fail, radioIndex = %d, INCORRECT operatingFrequencyBand = %s\n",
			__FUNCTION__, radioIndex, bands_str));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: radioIndex = %d, output_string = %s, bands_str = %s\n",
		__FUNCTION__, radioIndex, output_string, bands_str));

	return RETURN_OK;
}

//Get the frequency band at which the radio is operating, eg: "2.4GHz"
//The output_string is a max length 64 octet string that is allocated by the RDKB code. Implementations must ensure that strings are not longer than this.
INT wifi_getRadioOperatingFrequencyBand(INT radioIndex, CHAR *output_string) //Tr181
{
	CHAR operatingFrequencyBand[OUTPUT_STRING_LENGTH_64];
	int len = OUTPUT_STRING_LENGTH_64;
	int returnStatus;
	HAL_WIFI_DBG(("%s: Enter, radioIndex = %d\n", __FUNCTION__, radioIndex));

	NULL_PTR_ASSERT(output_string);
	RADIO_INDEX_ASSERT(radioIndex);

	output_string[0] = '\0';

	returnStatus = wldm_Radio_OperatingFrequencyBand(CMD_GET, radioIndex, operatingFrequencyBand, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_Radio_OperatingFrequencyBand CMD_GET Failed, Status = %d\n",
			__FUNCTION__, returnStatus));
		return RETURN_ERR;
	}

	if (!strncmp(operatingFrequencyBand, "a", sizeof(operatingFrequencyBand))) {
		snprintf(output_string, OUTPUT_STRING_LENGTH_64, "5GHz");
	} else if (!strncmp(operatingFrequencyBand, "b", sizeof(operatingFrequencyBand))) {
		snprintf(output_string, OUTPUT_STRING_LENGTH_64, "2.4GHz");
	} else if (!strncmp(operatingFrequencyBand, "6g", sizeof(operatingFrequencyBand))) {
		snprintf(output_string, OUTPUT_STRING_LENGTH_64, "6GHz");
	} else {
		HAL_WIFI_DBG(("%s: radioIndex = %d, INCORRECT operatingFrequencyBand = %s\n",
			__FUNCTION__, radioIndex, operatingFrequencyBand));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: radioIndex = %d, output_string = %s, operatingFrequencyBand = %s\n",
		__FUNCTION__, radioIndex, output_string, operatingFrequencyBand));

	return RETURN_OK;
}

/* Get the Supported Radio Mode. eg: "b,g,n"; "n,ac"
 * The output_string is a max length 64 octet string that is allocated by the RDKB code.
 * Implementations must ensure that strings are not longer than this. */

#if WIFI_HAL_VERSION_GE_2_15 || defined(WIFI_HAL_VERSION_3)
INT wifi_getRadioSupportedStandards(INT radioIndex, CHAR *output_string) //Tr181
{
	int len, ret;

	HAL_WIFI_DBG(("%s\n", __FUNCTION__));

	NULL_PTR_ASSERT(output_string);
	RADIO_INDEX_ASSERT(radioIndex);

	len = OUTPUT_STRING_LENGTH_64 * sizeof(char);
	ret = wldm_Radio_SupportedStandards(CMD_GET, radioIndex, output_string, &len, NULL, NULL);
	if (ret < 0) {
		HAL_WIFI_ERR(("%s: Failed to get SupportedStandards\n", __FUNCTION__));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: radioIndex=%d SupportedStandards=%s\n", __FUNCTION__, radioIndex, output_string));
	return RETURN_OK;
}
#endif /* (WIFI_HAL_VERSION_GE_2_15) */

struct operStdPMode_info {
	char		*operStd;
	char		*pmodeStr;
	unsigned int	pmodeVal;
};

/* operatingStandard to pureMode value for wifi_getRadioStandard */
static struct operStdPMode_info operStdPMode_infoTable[] =
{
	/* operStd	pmodeStr	pmodeVal	Old RadioStandard Reference */
	{"b,g,n",	"n",	PMODE_NONE},	/* gOnly = nOnly = acOnly = FALSE; */
#if !(defined(_XB7_PRODUCT_REQ_) || defined(_XB8_PRODUCT_REQ_) || \
	defined(_CBR2_PRODUCT_REQ_))
	{"g,n",		"n",	PMODE_G},	/* gOnly = TRUE; nOnly = acOnly = FALSE; */
#else
	{"g,n",		"n",	PMODE_NONE},	/* gOnly = nOnly = acOnly = FALSE; */
#endif
	{"g",		"g",	PMODE_G},	/* gOnly = TRUE; nOnly = acOnly = FALSE; */
	{"b,g",		"g",	PMODE_NONE},	/* gOnly = nOnly = acOnly = FALSE; */
	{"n",		"n",	PMODE_N},	/* nOnly = TRUE; gOnly = acOnly = FALSE; */
	{"a,n,ac",	"ac",	PMODE_NONE},	/* gOnly = nOnly = acOnly = FALSE; */
	{"n,ac",	"ac",	PMODE_N},	/* nOnly = TRUE; gOnly = acOnly = FALSE; */
	{"ac",		"ac",	PMODE_AC},	/* acOnly = TRUE; gOnly = nOnly = FALSE; */
	{"b,g,n,ax",	"ax",	PMODE_NONE},	/* gOnly = nOnly = acOnly = axOnly = FALSE; */
#if !(defined(_XB7_PRODUCT_REQ_) || defined(_XB8_PRODUCT_REQ_) || \
	defined(_CBR2_PRODUCT_REQ_))
	{"g,n,ax",	"ax",	(PMODE_AX | PMODE_G)},	/* 2G */
#else
	{"g,n,ax",	"ax",	PMODE_NONE},	/* gOnly = nOnly = acOnly = axOnly = FALSE; */
#endif
	{"n,ax",	"ax",	(PMODE_AX | PMODE_N)},	/* 2G */
	{"ac,ax",	"ax",	(PMODE_AX | PMODE_AC)}, /* 5G */
	{"n,ac,ax",	"ax",	(PMODE_AX | PMODE_N)},	/* 5G */
	{"a,n,ac,ax",	"ax",	PMODE_NONE},	/* gOnly = nOnly = acOnly = axOnly = FALSE; */
	{"ax",		"ax",	PMODE_AX},	/* axOnly = TRUE; gOnly = nOnly = acOnly = FALSE; */
#ifdef WIFI_HAL_VERSION_GE_3_0_3
	{"be",		"be",	PMODE_BE},	/* beOnly - TRUE; axOnly = gOnly = nOnly = acOnly = FALSE; */
#endif /* WIFI_HAL_VERSION_GE_3_0_3 */
	{NULL,		NULL,	PMODE_NONE}
};

static void
remove_all_chars(char* str, char c)
{
	char *pr = str, *pw = str;
	while (*pr) {
		*pw = *pr++;
		pw += (*pw != c);
	}
	*pw = '\0';
}

static int
wl_map_operatingStandard_to_pureMode(char *operStdInp, char *output_string, unsigned int *pureMode)
{
	int i;

	if (strstr(operStdInp, "\"")) {
		remove_all_chars(operStdInp, '"');
	}
	for (i = 0; operStdPMode_infoTable[i].operStd != NULL; i++) {
		if (!(strcmp(operStdPMode_infoTable[i].operStd, operStdInp))) {
			/* match */
			break;
		}
	}
	if (operStdPMode_infoTable[i].operStd != NULL) {
		strcpy(output_string, operStdPMode_infoTable[i].pmodeStr);
		*pureMode = operStdPMode_infoTable[i].pmodeVal;
	}
	else {
		HAL_WIFI_ERR(("%s: Invalid operatingStandard\n", __FUNCTION__));
		return -1;
	}
	return 0;
}

/* Deprecated from WIFI_HAL_MAJOR_VERSION >= 2 && WIFI_HAL_MINOR_VERSION >= 15 - use wifi_getRadioMode */
/* Get the radio operating mode, and pure mode flag. eg: "ac"
 * The output_string is a max length 64 octet string that is allocated by the RDKB code.
 * Implementations must ensure that strings are not longer than this.
 * Valid value: 2.4G	b,g,n
 *						g,n
 *						n
 *				5G		a,n,ac
 *						n,ac
 *						ac
 */
INT wifi_getRadioStandard(INT radioIndex, CHAR *output_string,
				BOOL *gOnly, BOOL *nOnly, BOOL *acOnly)
{
	CHAR operatingStandards[OUTPUT_STRING_LENGTH_64] = {0};
	unsigned int pureMode;
	int len;

	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(output_string);
	NULL_PTR_ASSERT(gOnly);
	NULL_PTR_ASSERT(nOnly);
	NULL_PTR_ASSERT(acOnly);

	len = OUTPUT_STRING_LENGTH_64 * sizeof(char);
	if (wldm_Radio_OperatingStandards(CMD_GET, radioIndex, operatingStandards,
						&len, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: Failed to get OperatingStandards\n", __FUNCTION__));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: radioIndex=%d OperatingStandards=%s\n", __FUNCTION__,
		radioIndex, operatingStandards));

	if (wl_map_operatingStandard_to_pureMode(operatingStandards, output_string, &pureMode) != 0) {
		HAL_WIFI_ERR(("%s: Failed to find pureMode\n", __FUNCTION__));
		return RETURN_ERR;
	}
	/* for PMODE_NONE */
	*gOnly = *nOnly = *acOnly = FALSE; /* *axOnly = FALSE - use getRadioMode if ax */
	if (pureMode == PMODE_G) {
		*gOnly = TRUE;
	}
	else if (pureMode == PMODE_N) {
		*nOnly = TRUE;
	}
	else if (pureMode == PMODE_AC) {
		*acOnly = TRUE;
	}
	HAL_WIFI_DBG(("%s, operatingStandard = %s, output_string = %s, gOnly = %d, nOnly = %d, acOnly = %d\n",
		__FUNCTION__,operatingStandards, output_string, *gOnly, *nOnly, *acOnly));

	return RETURN_OK;
}

static bool
wl_is80211axCapable(int radioIndex)
{
	bool axCap = FALSE;
	unsigned int len = sizeof(axCap);

	if (wldm_xbrcm_radio(CMD_GET, radioIndex, &axCap, &len, "axCapable", NULL) < 0) {
		return FALSE;
	}
	return axCap;
}

/* Set the radio operating mode, and pure mode flag.
 * Need wifi_applyRadioSettings after this for changes to be effective
 */

/* Derive parameters to set for wifi_setRadioMode */
static
INT wl_channelModeInfo_to_channelVars(int radioIndex, char *channelMode,
	unsigned int pureMode, chVar_info_t *chInfo)
{
	int len, heFeatVal;
	char *opStdp = NULL, *bwCapStr = NULL, *extChStr = NULL;
	bool acsEn;

	/* Derive chInfo->channelStd */
	memset(chInfo, 0, sizeof(chVar_info_t));

	chInfo->channelStd = (strstr(channelMode, "11AX")) ? CHMODE_11AX :
		(strstr(channelMode, "11AC")) ? CHMODE_11AC :
		(strstr(channelMode, "11N")) ? CHMODE_11N :
		(strstr(channelMode, "11G")) ? CHMODE_11G :
		(strstr(channelMode, "11B")) ? CHMODE_11B :
		(strstr(channelMode, "11A")) ? CHMODE_11A : CHMODE_NONE;

	extChStr = "Auto";
	if (strstr(channelMode, "20")) {
		bwCapStr = "20MHz";
	} else if (strstr(channelMode, "40")) {
		bwCapStr = "40MHz";
		extChStr = (strstr(channelMode, "MINUS")) ? "BelowControlChannel" :
				 ((strstr(channelMode, "PLUS")) ? "AboveControlChannel" : NULL);
	} else if (strstr(channelMode, "80")) {
		bwCapStr = "80MHz";
	} else if (strstr(channelMode, "160")) {
		bwCapStr = "160MHz";
	} else if (strstr(channelMode, "320")) {
		bwCapStr = "320MHz";
	}

	if (bwCapStr != NULL) {
		strncpy(chInfo->channelBWStr, bwCapStr, strlen(bwCapStr) + 1);
	}
	if (extChStr != NULL) {
		strncpy(chInfo->extensionChStr, extChStr, strlen(extChStr) + 1);
	}
	HAL_WIFI_DBG(("%s: channelMode=%s chStd=0x%x bwCap=%s extCh=%s\n", __FUNCTION__, channelMode,
		chInfo->channelStd, (bwCapStr != NULL) ? chInfo->channelBWStr : "NULL",
		(extChStr != NULL) ? chInfo->extensionChStr : "NULL"));

	/* Init HeFeatValue VhtModeValue omcVal */
	chInfo->omcVal = OMC_NONE;
	chInfo->HeFeatValue = 0;
	chInfo->VhtModeValue = 0;

	/* OperatingStandards omcVal HeFeatValue VhtModeValue */
	switch (chInfo->channelStd) {
		case CHMODE_11A:
			opStdp = "a";
			break;

		case CHMODE_11B:
			opStdp = "b";
			break;

		case CHMODE_11G:
			opStdp = (pureMode == PMODE_G) ? "g" : "b,g";
			chInfo->omcVal = (pureMode == PMODE_G) ? OMC_ERP : OMC_NONE;
			break;

		case CHMODE_11N:
			opStdp = (pureMode == PMODE_N) ? "n" :
#if !(defined(_XB7_PRODUCT_REQ_) || defined(_XB8_PRODUCT_REQ_) || \
	defined(_CBR2_PRODUCT_REQ_))
				 (pureMode == PMODE_G) ? "g,n" : "b,g,n";
#else
				 "g,n";
#endif
			chInfo->omcVal = (pureMode == PMODE_N) ? OMC_HT :
				(pureMode == PMODE_G) ? OMC_ERP : OMC_NONE;
			break;

		case CHMODE_11AC:
			opStdp = (pureMode == PMODE_AC) ? "ac" :
				 (pureMode == PMODE_N) ? "n,ac" : "a,n,ac";
			chInfo->omcVal = (pureMode == PMODE_AC) ? OMC_VHT : OMC_NONE;
			chInfo->VhtModeValue = (pureMode == PMODE_N) ? 0 : 1;
			break;

		case CHMODE_11AX:
			AX_CAPABLE_ASSERT(radioIndex);
			len = sizeof(heFeatVal);
			wldm_AXfeatures(CMD_GET_NVRAM, radioIndex, &heFeatVal, &len, NULL, NULL);
			chInfo->HeFeatValue = heFeatVal;

			/* vhtmode for 2G in AX */
			if (wl_is80211axCapable(radioIndex)) {
				if (radioIndex == BAND_2G_INDEX) {
					int len, allowAX = 0;
					len = sizeof(allowAX);
					if (wldm_xbrcm_radio(CMD_GET_NVRAM, radioIndex, (uint *)&allowAX,
						(uint *)&len, "allow80211ax", NULL) != 0) {
						HAL_WIFI_ERR(("%s: radio %d rfc_allow_11ax get error\n",
						__FUNCTION__, radioIndex));
					}
					if ((!allowAX) && (pureMode == PMODE_AX)) {
						HAL_WIFI_ERR(("%s: radio %d PMODE_AX allowAX=0 Not valid\n",
							__FUNCTION__, radioIndex));
						return -1;
					}
					chInfo->VhtModeValue = allowAX ? -1 : 0;
				}
				else {
					chInfo->VhtModeValue = 1;
				}
			}
			else {
				chInfo->VhtModeValue = (radioIndex == 0) ? -1 : 1;
			}

			/* determine omcVal */
			if (pureMode == PMODE_AX) {
				opStdp = "ax";
				chInfo->omcVal = OMC_HE;
			}
			else if (pureMode == (PMODE_AX | PMODE_AC)) {
				if (radioIndex == 1) {
					opStdp = "ac,ax";
					chInfo->omcVal = OMC_VHT;
				}
				else {
					/* not valid for 2G */
					return -1;
				}
			}
			else if (pureMode == (PMODE_AX | PMODE_N)) {
				opStdp = (radioIndex == 0) ? "n,ax" : "n,ac,ax";
				chInfo->omcVal = OMC_HT;
			}
#if !(defined(_XB7_PRODUCT_REQ_) || defined(_XB8_PRODUCT_REQ_) || \
	defined(_CBR2_PRODUCT_REQ_))
			else if (pureMode == (PMODE_AX | PMODE_G)) {
				if (radioIndex == 0) {
					opStdp = "g,n,ax";
					chInfo->omcVal = OMC_ERP;
				}
				else {
					/* not valid for 5G */
					return -1;
				}
			}
			else {
				opStdp = (radioIndex == 0) ? "b,g,n,ax" : "a,n,ac,ax";
				chInfo->omcVal = OMC_NONE;
			}
#else
			else {
				opStdp = (radioIndex == 0) ? "g,n,ax" : "a,n,ac,ax";
				chInfo->omcVal = OMC_NONE;
			}
#endif /*!(defined(_XB7_PRODUCT_REQ_) || defined(_XB8_PRODUCT_REQ_) || defined(_CBR2_PRODUCT_REQ_)) */
			break;

		default:
			HAL_WIFI_ERR(("%s: Error radioIndex=%d Unsupported channelMode %s\n",
				__FUNCTION__, radioIndex, channelMode));
			return -1;
			break;
	} /* switch */
	if (opStdp != NULL) {
		strncpy(chInfo->operatingStandards, opStdp, strlen(opStdp) + 1);
	} else {
		HAL_WIFI_ERR(("%s: Error radioIndex=%d channelMode %s operatingStandards not defined\n",
			__FUNCTION__, radioIndex, channelMode));
		return -1;
	}

	len = sizeof(acsEn);
	acsEn = FALSE;
	if (wldm_Radio_AutoChannelEnable(CMD_GET, radioIndex, &acsEn, &len, NULL, NULL) < 0) {
		HAL_WIFI_ERR(("%s: Error checking AutoChannelEnable\n", __FUNCTION__ ));
		return RETURN_ERR;
	}

	if (!acsEn) {
		/* Get curr_channel */
		len = sizeof(unsigned int);
		if (wldm_Radio_Channel(CMD_GET, radioIndex, (unsigned int *)(&chInfo->curr_channel),
			&len, 0, 0, NULL, NULL) != 0) {
			HAL_WIFI_ERR(("%s: Error getting curr_channel radioIndex=%d\n", __FUNCTION__, radioIndex));
			return RETURN_ERR;
		}
	}
	else {
		chInfo->curr_channel = 0;
	}
	HAL_WIFI_DBG(("%s: radioIndex=%d omcVal=0x%x HeFeatValue=0x%x VhtModeValue=%d operatingStd=%s curr_channel=%d\n",
		__FUNCTION__, radioIndex, chInfo->omcVal, chInfo->HeFeatValue, chInfo->VhtModeValue,
		chInfo->operatingStandards, chInfo->curr_channel));
	return 0;
}

static
INT wl_setRadioMode(INT radioIndex, CHAR *channelMode, UINT pureMode)
{
	chVar_info_t	chVars;
	int		ret, len, val;

	HAL_WIFI_DBG(("DBG-15 Enter %s radioIndex=%d channelMode=%s \n",
		__FUNCTION__, radioIndex, channelMode));

	if ((radioIndex == 0) && ((strstr(channelMode, "80")) || (strstr(channelMode, "160")) ||
			       (strstr(channelMode, "320")))) {
		HAL_WIFI_ERR(("%s: Error radioIndex=%d Unsupported channelBW in %s\n",
			__FUNCTION__, radioIndex, channelMode));
		return RETURN_ERR;
	}

	if ((radioIndex > 1) && (pureMode != PMODE_AX)) {
		HAL_WIFI_ERR(("%s: Error radioIndex=%d given pureMode=0x%x need PMODE_AX=0x%x\n",
			__FUNCTION__, radioIndex, pureMode, PMODE_AX));
	}

	ret =  wl_channelModeInfo_to_channelVars(radioIndex, channelMode, pureMode, &chVars);
	if (ret < 0) {
		HAL_WIFI_ERR(("%s: Error wldm_channelModeInfo_to_channelVars radioIndex=%d\n",
			__FUNCTION__, radioIndex));
	}

	HAL_WIFI_DBG(("%s: radioIndex=%d omcVal=0x%x HeFeatValue=0x%x VhtModeValue=%d operatingStd=%s\n",
		__FUNCTION__, radioIndex, chVars.omcVal, chVars.HeFeatValue, chVars.VhtModeValue,
		chVars.operatingStandards));
	HAL_WIFI_DBG(("%s: channelMode=%s channel=%d bwCapStr=%s extChStr=%s\n", __FUNCTION__,
		channelMode, chVars.curr_channel, chVars.channelBWStr, chVars.extensionChStr));

	len = sizeof(chVars.omcVal);
	ret = wldm_xbrcm_ap(CMD_SET_NVRAM, HAL_RADIO_IDX_TO_HAL_AP(radioIndex), &(chVars.omcVal), (unsigned int *)&len, "mode_reqd", NULL);
	if (ret != 0) {
		HAL_WIFI_ERR(("%s: Error mode_reqd for radioIndex=%d\n", __FUNCTION__, radioIndex));
	}

	len = strlen(chVars.operatingStandards);
	ret = wldm_Radio_OperatingStandards(CMD_SET, radioIndex, chVars.operatingStandards,
			&len, NULL, NULL);
	if (ret != 0) {
		HAL_WIFI_ERR(("%s: Unable to setoperatingStandards to %s\n",
			__FUNCTION__, chVars.operatingStandards));
		return RETURN_ERR;
	}

	len = sizeof(chVars.VhtModeValue);
	ret = wldm_xbrcm_11ac(CMD_SET_NVRAM, radioIndex, &(chVars.VhtModeValue),
		(unsigned int *)&len, "vhtmode", NULL);
	if (ret != 0) {
		HAL_WIFI_ERR(("%s: Error wldm_Vhtmode radioIndex=%d\n", __FUNCTION__, radioIndex));
		return RETURN_ERR;
	}
#if !defined(_CBR1_PRODUCT_REQ_)
	if (wl_is80211axCapable(radioIndex)) {
		val = (int)chVars.HeFeatValue;
		len = sizeof(val);
		ret = wldm_AXfeatures(CMD_SET, radioIndex, &val, &len, NULL, NULL);
		if (ret != RETURN_OK) {
			HAL_WIFI_DBG(("%s: radioIndex=%d Failed set HeFeatValue=0x%x, ret=%d\n",
				__FUNCTION__, radioIndex, chVars.HeFeatValue, ret));
			return RETURN_ERR;
		}
	}
#endif /* !_CBR1_PRODUCT_REQ_ */
	if (chVars.channelBWStr[0] != '\0') {
		len = sizeof(chVars.channelBWStr);
		ret = wldm_Radio_OperatingChannelBandwidth(CMD_SET, radioIndex, chVars.channelBWStr,
			&len, NULL, NULL);
		if (ret != 0) {
			HAL_WIFI_ERR(("%s: Unable to setChannelBandwidth to %s\n",
				__FUNCTION__, chVars.channelBWStr));
			return RETURN_ERR;
		}
	}

	/* Apply Radio Settings */
	/* wifi_applyRadioSettings(radioIndex); */
	return RETURN_OK;
}

/* Deprecated from WIFI_HAL_MAJOR_VERSION >= 2 && WIFI_HAL_MINOR_VERSION >= 15
 * - use wifi_setRadioMode
 */
/* Set the radio operating mode, and pure mode flag. */
INT wifi_setRadioChannelMode(INT radioIndex, CHAR *channelMode,
				BOOL gOnlyFlag, BOOL nOnlyFlag, BOOL acOnlyFlag)
{
#if WIFI_HAL_VERSION_GE_2_15 && (!defined(_CBR_PRODUCT_REQ_) && \
	!defined(_XF3_PRODUCT_REQ_))
	UINT pureMode = PMODE_NONE;

	NULL_PTR_ASSERT(channelMode);
	RADIO_INDEX_ASSERT(radioIndex);

	pureMode = (gOnlyFlag) ? PMODE_G :
		(nOnlyFlag) ? PMODE_N :
		(acOnlyFlag) ? PMODE_AC : PMODE_NONE;
	return (wl_setRadioMode(radioIndex, channelMode, pureMode));
#endif /* (WIFI_HAL_VERSION_GE_2_15) && (!defined(_CBR_PRODUCT_REQ_) && !defined(_XF3_PRODUCT_REQ_)) */
}

#if WIFI_HAL_VERSION_GE_2_15 && (!defined(_CBR_PRODUCT_REQ_) && \
	!defined(_XF3_PRODUCT_REQ_))

/* Settings follow wifi_getRadioStandard from before
 * Add ax and use bits for pureMode instead of xxOnly flags
 */
INT wifi_getRadioMode(INT radioIndex, CHAR *output_string, UINT *pureMode)
{
	CHAR operatingStandards[OUTPUT_STRING_LENGTH_64] = {0};
	int len, ret;

	HAL_WIFI_DBG(("%s\n", __FUNCTION__));

	NULL_PTR_ASSERT(output_string);
	RADIO_INDEX_ASSERT(radioIndex);

	len = OUTPUT_STRING_LENGTH_64 * sizeof(char);
	ret = wldm_Radio_OperatingStandards(CMD_GET, radioIndex, operatingStandards, &len, NULL, NULL);
	if (ret < 0) {
		HAL_WIFI_ERR(("%s: Failed to get OperatingStandards\n", __FUNCTION__));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: radioIndex=%d OperatingStandards=%s\n", __FUNCTION__, radioIndex, output_string));

	ret = wl_map_operatingStandard_to_pureMode(operatingStandards, output_string, pureMode);
	if (ret < 0) {
		HAL_WIFI_ERR(("%s: radioIndex=%d Failed to get pureMode Value\n", __FUNCTION__, radioIndex));
		return RETURN_ERR;
	}
	HAL_WIFI_DBG(("%s, | ax | ac | n | g | b | a | operatingStandard = %s ,output_string = %s pureMode = 0x%x\n",
		__FUNCTION__, operatingStandards, output_string, *pureMode));
	return RETURN_OK;
}

/* Set the radio operating mode, and pure mode flag.
 * Need wifi_applyRadioSettings after this for changes to be effective
 */
INT wifi_setRadioMode(INT radioIndex, CHAR *channelMode, UINT pureMode)	//RDKB
{
	HAL_WIFI_DBG(("Enter %s radioIndex=%d channelMode=%s \n", __FUNCTION__, radioIndex, channelMode));

	NULL_PTR_ASSERT(channelMode);
	RADIO_INDEX_ASSERT(radioIndex);

	return (wl_setRadioMode(radioIndex, channelMode, pureMode));
}
#endif /* (WIFI_HAL_VERSION_GE_2_15) &&
	(!defined(_CBR_PRODUCT_REQ_) && !defined(_XF3_PRODUCT_REQ_)) */

//Get the list of supported channel. eg: "1-11"
//The output_string is a max length 64 octet string that is allocated by the RDKB code.  Implementations must ensure that strings are not longer than this.
INT wifi_getRadioPossibleChannels(INT radioIndex, CHAR *output_string) //RDKB
{
	int len = OUTPUT_STRING_LENGTH_512; /* Instead of 64, use 512 per PossibleChannel in cosa_wifi_apis */
	int returnStatus;

	HAL_WIFI_DBG(("%s: Enter, radioIndex = %d\n", __FUNCTION__, radioIndex));

	NULL_PTR_ASSERT(output_string);
	RADIO_INDEX_ASSERT(radioIndex);

	output_string[0] = '\0';

	returnStatus = wldm_Radio_PossibleChannels(CMD_GET, radioIndex, output_string, &len, NULL, NULL);
	if (returnStatus < 0) {
		HAL_WIFI_DBG(("%s: wldm_Radio_PossibleChannels CMD_GET Failed, Status = %d\n",
			__FUNCTION__, returnStatus));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: radioIndex = %d, output_string = %s\n", __FUNCTION__, radioIndex, output_string));

	return RETURN_OK;
}

#ifdef WIFI_HAL_VERSION_3_PHASE2
INT wifi_getRadioChannelsInUse(wifi_radio_index_t radioIndex, wifi_channels_list_t *channel_list)
{
	char chList_str[OUTPUT_STRING_LENGTH_256];
	int ret, len = sizeof(chList_str);

	HAL_WIFI_DBG(("%s: Enter, radioIndex = %d\n", __FUNCTION__, radioIndex));

	NULL_PTR_ASSERT(channel_list);
	RADIO_INDEX_ASSERT(radioIndex);
	RADIO_NOT_UP_ASSERT(radioIndex);

	ret = wldm_Radio_ChannelsInUse(CMD_GET, radioIndex, (char *) chList_str,
		&len, NULL, NULL);
	if (ret != 0) {
		HAL_WIFI_ERR(("%s: wldm_Radio_ChannelsInUse CMD_GET Failed, Status = %d\n",
			__FUNCTION__, ret));
		return WIFI_HAL_ERROR;
	}

	/* Allow comma or space delimiters in string */
	ret = wl_str2uintArray(chList_str, ", ",
		(unsigned int *)(&channel_list->num_channels),
		(unsigned int *)(channel_list->channels_list), MAX_CHANNELS);
	if (ret != 0) {
		HAL_WIFI_ERR(("%s: wl_str2uintArray Failed\n", __FUNCTION__));
		return WIFI_HAL_ERROR;
	}
	return WIFI_HAL_SUCCESS;
}
#else /* WIFI_HAL_VERSION_3_PHASE2 */
//Get the list for used channel. eg: "1,6,9,11"
//The output_string is a max length 256 octet string that is allocated by the RDKB code.  Implementations must ensure that strings are not longer than this.
INT wifi_getRadioChannelsInUse(INT radioIndex, CHAR *output_string) //RDKB
{
	int len = OUTPUT_STRING_LENGTH_256;
	int returnStatus;

	HAL_WIFI_DBG(("%s: Enter, radioIndex = %d\n", __FUNCTION__, radioIndex));

	NULL_PTR_ASSERT(output_string);
	RADIO_INDEX_ASSERT(radioIndex);
	RADIO_NOT_UP_ASSERT(radioIndex);

	returnStatus = wldm_Radio_ChannelsInUse(CMD_GET, radioIndex, (CHAR *) output_string, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_ERR(("%s: wldm_Radio_ChannelsInUse CMD_GET Failed, Status = %d\n",
			__FUNCTION__, returnStatus));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: radioIndex = %d, output_string = %s\n", __FUNCTION__, radioIndex, output_string));

	return RETURN_OK;
}
#endif /* WIFI_HAL_VERSION_3_PHASE2 */

//Get the running channel number
INT wifi_getRadioChannel(INT radioIndex, ULONG *output_ulong) //RDKB
{
	/* WLDM METHOD OF IMPLEMENTATION */
	int len;
	unsignedInt channel;

	NULL_PTR_ASSERT(output_ulong);
	RADIO_INDEX_ASSERT(radioIndex);

	len = sizeof(channel);
	if (wldm_Radio_Channel(CMD_GET, radioIndex, &channel, &len, 0, 0, NULL, NULL) == 0) {
		*output_ulong = (ULONG) channel;
		return RETURN_OK;
	}
	HAL_WIFI_ERR(("%s: Failed to get target channel info\n", __FUNCTION__));
	return RETURN_ERR;
}

//Set the running channel number
INT wifi_setRadioChannel(INT radioIndex, ULONG channel) //RDKB  //AP only
{
	/* WLDM METHOD OF IMPLEMENTATION */
	int len;
	unsignedInt chan;

	RADIO_INDEX_ASSERT(radioIndex);

	len = sizeof(chan);
	chan = (unsigned int) channel;
	if (wldm_Radio_Channel(CMD_SET, radioIndex, &chan, &len, 0, 0, NULL, NULL) == 0) {
		return RETURN_OK;
	}
	HAL_WIFI_ERR(("%s: Failed to set channel\n", __FUNCTION__));
	return RETURN_ERR;
}

//Enables or disables a driver level variable to indicate if auto channel selection is enabled on this radio
//This "auto channel" means the auto channel selection when radio is up. (which is different from the dynamic channel/frequency selection (DFC/DCS))
INT wifi_setRadioAutoChannelEnable(INT radioIndex, BOOL enable) //RDKB
{
	HAL_WIFI_DBG(("%s, radioIndex %d \n", __FUNCTION__, radioIndex));

	RADIO_INDEX_ASSERT(radioIndex);

	int len = sizeof(enable);

	if (wldm_Radio_AutoChannelEnable(CMD_SET, radioIndex, &enable, &len, NULL, NULL) < 0) {
		HAL_WIFI_ERR(("%s: wldm_Radio_AutoChannelEnable set failed\n", __FUNCTION__ ));
		return RETURN_ERR;
	}

#ifdef RDKB_TIMER
	wifi_set_timer_variable();
#endif

	return RETURN_OK;
}

//Get the AutoChannel enable status
INT wifi_getRadioAutoChannelEnable(INT radioIndex, BOOL *output_bool) //Tr181
{
	HAL_WIFI_DBG(("%s, radioIndex %d \n", __FUNCTION__, radioIndex));

	NULL_PTR_ASSERT(output_bool);
	RADIO_INDEX_ASSERT(radioIndex);

	int len = sizeof(output_bool);
	BOOL enable;
	if (wldm_Radio_AutoChannelEnable(CMD_GET, radioIndex, &enable, &len, NULL, NULL) < 0) {
		HAL_WIFI_ERR(("%s: wldm_Radio_AutoChannelEnable\n", __FUNCTION__ ));
		return RETURN_ERR;
	}

	*output_bool = enable;
	return RETURN_OK;
}

//Check if the driver support the AutoChannel
INT wifi_getRadioAutoChannelSupported(INT radioIndex, BOOL *output_bool) //Tr181
{
	NULL_PTR_ASSERT(output_bool);
	RADIO_INDEX_ASSERT(radioIndex);

	int len = sizeof(output_bool);
	BOOL enable;

	if (wldm_Radio_AutoChannelSupported(CMD_GET, radioIndex, &enable, &len, NULL, NULL) < 0) {
		HAL_WIFI_ERR(("%s: wldm_Radio_AutoChannelSupported get failed\n", __FUNCTION__ ));
		return RETURN_ERR;
	}
	*output_bool = enable;
	return RETURN_OK;
}

INT wifi_getRadioDCSSupported(INT radioIndex, BOOL *output_bool) //RDKB
{
	HAL_WIFI_DBG(("%s: Enter, radioIndex = %d\n", __FUNCTION__, radioIndex));

	NULL_PTR_ASSERT(output_bool);
	RADIO_INDEX_ASSERT(radioIndex);

	int returnStatus;
	int len = sizeof(*output_bool);
	*output_bool = FALSE;

	returnStatus = wldm_Radio_AutoChannelSupported(CMD_GET, radioIndex, output_bool, &len, NULL, NULL);
	if (RETURN_OK != returnStatus) {
		HAL_WIFI_DBG(("%s: fail for radioIndex = %d\n", __FUNCTION__, radioIndex));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: radioIndex = %d, output_bool = %lu\n", __FUNCTION__, radioIndex, *output_bool));
	return RETURN_OK;
}

INT wifi_getRadioDCSEnable(INT radioIndex, BOOL *output_bool) //RDKB
{
	HAL_WIFI_DBG(("%s: Enter, radioIndex = %d\n", __FUNCTION__, radioIndex));
	NULL_PTR_ASSERT(output_bool);
	RADIO_INDEX_ASSERT(radioIndex);

	int len = sizeof(*output_bool);
	int returnStatus;
	*output_bool = FALSE;

	returnStatus = wldm_Radio_AutoChannelEnable(CMD_GET, radioIndex, output_bool, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_Radio_AutoChannelEnable Failed, Status = %d\n",
			__FUNCTION__, returnStatus));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: radioIndex = %d, output_bool = %d\n", __FUNCTION__, radioIndex, *output_bool));

	return RETURN_OK;
}

INT wifi_setRadioDCSEnable(INT radioIndex, BOOL enable)         //RDKB
{
	int len = sizeof(enable);
	int returnStatus;

	HAL_WIFI_DBG(("%s: Enter, radioIndex = %d\n", __FUNCTION__, radioIndex));
	RADIO_INDEX_ASSERT(radioIndex);

	returnStatus = wldm_Radio_AutoChannelEnable(CMD_SET, radioIndex, &enable, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_Radio_AutoChannelEnable Failed, Status = %d\n",
			__FUNCTION__, returnStatus));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: radioIndex = %d, enable = %d\n", __FUNCTION__, radioIndex, enable));

	return RETURN_OK;
}

//Check if the driver support the Dfs
INT wifi_getRadioDfsSupport(INT radioIndex, BOOL *output_bool) //Tr181
{
	HAL_WIFI_DBG(("%s: Enter, radioIndex = %d\n", __FUNCTION__, radioIndex));
	NULL_PTR_ASSERT(output_bool);
	RADIO_INDEX_ASSERT(radioIndex);

	int returnStatus;
	int len = sizeof(*output_bool);
	*output_bool = FALSE;

	returnStatus = wldm_Radio_DfsSupport(CMD_GET, radioIndex, output_bool, &len, NULL, NULL);
	if (RETURN_OK != returnStatus) {
		HAL_WIFI_DBG(("%s: wldm_Radio_DfsSupport CMD_GET failed for radioIndex = %d\n",
			__FUNCTION__, radioIndex));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: radioIndex = %d, output_bool = %d\n", __FUNCTION__, radioIndex, *output_bool));

	return RETURN_OK;
}

static INT _wifi_getRadioDfsEnable(INT radioIndex, int cmd, BOOL *output_bool)
{
	int returnStatus;
	int len = sizeof(*output_bool);
	*output_bool = FALSE;

	returnStatus = wldm_Radio_DfsEnable(cmd, radioIndex, output_bool, &len, NULL, NULL);
	if (RETURN_OK != returnStatus) {
		HAL_WIFI_DBG(("%s: wldm_Radio_DfsEnable %d failed for radioIndex=%d Status=%d\n",
			__FUNCTION__, radioIndex, cmd, returnStatus));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: radioIndex = %d, output_bool = %d\n", __FUNCTION__, radioIndex, *output_bool));

	return RETURN_OK;
}
//Get the Dfs enable status
INT wifi_getRadioDfsEnable(INT radioIndex, BOOL *output_bool)   //Tr181
{
	HAL_WIFI_DBG(("%s: Enter, radioIndex = %d\n", __FUNCTION__, radioIndex));
	NULL_PTR_ASSERT(output_bool);
	RADIO_INDEX_ASSERT(radioIndex);

	return _wifi_getRadioDfsEnable(radioIndex, CMD_GET_NVRAM, output_bool);
}

static INT _wifi_setRadioDfsEnable(INT radioIndex, int cmd, BOOL enable)
{
	int returnStatus;
	int len = sizeof(enable);
	returnStatus = wldm_Radio_DfsEnable(cmd, radioIndex, &enable, &len, NULL, NULL);
	if (RETURN_OK != returnStatus) {
		HAL_WIFI_DBG(("%s: wldm_Radio_DfsEnable CMD_SET failed for radioIndex=%d Status=%d\n",
			__FUNCTION__, radioIndex, returnStatus));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: radioIndex = %d, enable = %d\n", __FUNCTION__, radioIndex, enable));

	return RETURN_OK;
}

//Set the Dfs enable status
INT wifi_setRadioDfsEnable(INT radioIndex, BOOL enable) //Tr181
{
	HAL_WIFI_DBG(("%s: Enter, radioIndex = %d\n", __FUNCTION__, radioIndex));
	RADIO_INDEX_ASSERT(radioIndex);

	return _wifi_setRadioDfsEnable(radioIndex, CMD_SET, enable);
}

/* Set the DfsAtBootUp enable status */
INT wifi_setRadioDfsAtBootUpEnable(INT radioIndex, BOOL enable) //Tr181
{
	int nvramVal, len = sizeof(nvramVal);
	HAL_WIFI_DBG(("%s: radioIndex = %d enable = %d\n", __FUNCTION__, radioIndex, enable));
	RADIO_INDEX_ASSERT(radioIndex);
	nvramVal = enable ? 0 : 1;

	if (wldm_xbrcm_acs(CMD_SET_NVRAM, radioIndex,
			&nvramVal, &len, "acs_start_on_nondfs", NULL) < 0) {
		HAL_WIFI_ERR(("%s: wldm_xbrcm_acs CMD_SET failed.\n", __FUNCTION__));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: enable = %d, nvramVal = %d\n", __FUNCTION__, enable, nvramVal));
	return RETURN_OK;
}

/* Get the DfsAtBootUp enable status */
INT wifi_getRadioDfsAtBootUpEnable(INT radioIndex, BOOL *output_bool)   //Tr181
{
	int nvramVal, len = sizeof(nvramVal);
	HAL_WIFI_DBG(("%s: radioIndex = %d\n", __FUNCTION__, radioIndex));
	NULL_PTR_ASSERT(output_bool);
	RADIO_INDEX_ASSERT(radioIndex);

	if (wldm_xbrcm_acs(CMD_GET_NVRAM, radioIndex, &nvramVal, &len, "acs_start_on_nondfs", NULL) < 0) {
		HAL_WIFI_ERR(("%s: wldm_xbrcm_acs CMD_GET failed.\n", __FUNCTION__));
		return RETURN_ERR;
	}
	*output_bool = nvramVal ? FALSE : TRUE;

	HAL_WIFI_DBG(("%s: radioIndex = %d, output_bool = %d\n", __FUNCTION__, radioIndex, *output_bool));
	return RETURN_OK;
}

//Check if the driver support the AutoChannelRefreshPeriod
INT wifi_getRadioAutoChannelRefreshPeriodSupported(INT radioIndex, BOOL *output_bool) //Tr181
{
	INT iRet = RETURN_OK;
	NULL_PTR_ASSERT(output_bool);
	RADIO_INDEX_ASSERT(radioIndex);

	*output_bool = TRUE;
	HAL_WIFI_DBG(("%s: %s\n", __FUNCTION__, *output_bool ? "Enabled" : "Disabled"));

	return iRet;
}

//Get the ACS refresh period in seconds
INT wifi_getRadioAutoChannelRefreshPeriod(INT radioIndex, ULONG *output_ulong) //Tr181
{
	HAL_WIFI_DBG(("%s: Enter, radioIndex = %d\n", __FUNCTION__, radioIndex));

	NULL_PTR_ASSERT(output_ulong);
	RADIO_INDEX_ASSERT(radioIndex);

	unsigned int refreshPeriod;
	int returnStatus, len = sizeof(refreshPeriod);
	*output_ulong = 0;

	returnStatus = wldm_Radio_AutoChannelRefreshPeriod(CMD_GET, radioIndex,
				&refreshPeriod, &len, NULL, NULL);
	if (RETURN_OK != returnStatus) {
		HAL_WIFI_DBG(("%s: wldm_Radio_AutoChannelRefreshPeriod CMD_GET failed radioIndex = %d, Status=%d\n",
			__FUNCTION__, radioIndex, returnStatus));
		return RETURN_ERR;
	}
	*output_ulong = (ULONG) refreshPeriod;
	HAL_WIFI_DBG(("%s: radioIndex = %d, output_ulong = %lu\n", __FUNCTION__, radioIndex, *output_ulong));
	return RETURN_OK;
}

//Set the ACS refresh period in seconds
INT wifi_setRadioDfsRefreshPeriod(INT radioIndex, ULONG seconds)
{
	int len = sizeof(seconds);
	HAL_WIFI_DBG(("%s: radioIndex = %d, seconds = %d\n", __FUNCTION__, radioIndex, seconds));

	RADIO_INDEX_ASSERT(radioIndex);

	if (wldm_xbrcm_acs(CMD_SET_NVRAM | CMD_SET_IOCTL, radioIndex, (void *)&seconds, &len, "DfsRefreshPeriod", NULL) < 0) {
		HAL_WIFI_ERR(("%s: wldm_xbrcm_acs CMD_SET_IOCTL %d failed.\n", __FUNCTION__, seconds));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

//Get the Operating Channel Bandwidth. eg "20MHz", "40MHz", "80MHz", "80+80", "160", "320"
//The output_string is a max length 64 octet string that is allocated by the RDKB code.  Implementations must ensure that strings are not longer than this.
INT wifi_getRadioOperatingChannelBandwidth(INT radioIndex, CHAR *output_string) //Tr181
{
	int len, ret;

	NULL_PTR_ASSERT(output_string);
	RADIO_INDEX_ASSERT(radioIndex);

	len = OUTPUT_STRING_LENGTH_64 * sizeof(char);
	ret = wldm_Radio_OperatingChannelBandwidth(CMD_GET, radioIndex, output_string, &len, NULL, NULL);
	if (ret < 0) {
		HAL_WIFI_ERR(("%s: Failed to get OperatingChannelBandwidth\n", __FUNCTION__));
		return RETURN_ERR;
	}
	HAL_WIFI_DBG(("%s, radioIndex = %d, output_string = %s\n", __FUNCTION__, radioIndex, output_string));
	return RETURN_OK;
}

//Set the Operating Channel Bandwidth.
INT wifi_setRadioOperatingChannelBandwidth(INT radioIndex, CHAR *bandwidth) //Tr181 //AP only
{
	int len, ret;

	NULL_PTR_ASSERT(bandwidth);
	RADIO_INDEX_ASSERT(radioIndex);

	HAL_WIFI_DBG(("%s: Entry, radioIndex = %d, = %s\n", __FUNCTION__, radioIndex, bandwidth));
	len = strlen(bandwidth);
	ret = wldm_Radio_OperatingChannelBandwidth(CMD_SET, radioIndex, bandwidth, &len, NULL, NULL);
	if (ret < 0) {
		HAL_WIFI_ERR(("%s: Failed to set OperatingChannelBandwidth\n", __FUNCTION__));
		return RETURN_ERR;
	}
#ifdef RDKB_TIMER
	wifi_set_timer_variable();
#endif

	HAL_WIFI_DBG(("%s: done, radioIndex = %d, bandwidth = %s\n", __FUNCTION__, radioIndex, bandwidth));
	return RETURN_OK;
}

//Get the secondary extension channel position, "AboveControlChannel" or "BelowControlChannel". (this is for 40MHz and 80MHz bandwith only)
//The output_string is a max length 64 octet string that is allocated by the RDKB code.  Implementations must ensure that strings are not longer than this.
INT wifi_getRadioExtChannel(INT radioIndex, CHAR *output_string) //Tr181
{
	int len, ret;

	NULL_PTR_ASSERT(output_string);
	RADIO_INDEX_ASSERT(radioIndex);

	len = sizeof(char) * OUTPUT_STRING_LENGTH_64;
	ret = wldm_Radio_ExtensionChannel(CMD_GET, radioIndex, output_string, &len, NULL, NULL);
	if (ret < 0) {
		HAL_WIFI_ERR(("%s: Failed to get RadioExtensionChannel\n", __FUNCTION__));
		return RETURN_ERR;
	}
	return RETURN_OK;
}

//Set the extension channel.
INT wifi_setRadioExtChannel(INT radioIndex, CHAR *string) //Tr181 //AP only
{
	int len, ret;

	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(string);

	len = strlen(string);
	ret = wldm_Radio_ExtensionChannel(CMD_SET, radioIndex, string, &len, NULL, NULL);
	if (ret < 0) {
		HAL_WIFI_ERR(("%s: Failed to set RadioExtensionChannel\n", __FUNCTION__));
		return RETURN_ERR;
	}
	return RETURN_OK;
}

//Get the guard interval value. eg "400nsec" or "800nsec"
//The output_string is a max length 64 octet string that is allocated by the RDKB code.
//  Implementations must ensure that strings are not longer than this.
INT wifi_getRadioGuardInterval(INT radioIndex, CHAR *output_string) //Tr181
{
	CHAR guard_interval[OUTPUT_STRING_LENGTH_64]={'\0'};

	HAL_WIFI_DBG(("%s: radioIndex  %d\n", __FUNCTION__, radioIndex));
	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(output_string);

	int len = sizeof(guard_interval);

	if (wldm_Radio_GuardInterval(CMD_GET, radioIndex, guard_interval, &len, NULL, NULL) < 0) {
		HAL_WIFI_ERR(("%s: Failed to get RadioGarudInterval\n", __FUNCTION__));
		return RETURN_ERR;
	}

	snprintf(output_string, OUTPUT_STRING_LENGTH_64, "%s", guard_interval);
	return RETURN_OK;
}

//Set the guard interval value.
INT wifi_setRadioGuardInterval(INT radioIndex, CHAR *string) //Tr181
{
	HAL_WIFI_DBG(("%s: radioIndex = %d, string = %s\n", __FUNCTION__, radioIndex, string));

	NULL_PTR_ASSERT(string);
	RADIO_INDEX_ASSERT(radioIndex);

	int len = strlen(string);

	if (wldm_Radio_GuardInterval(CMD_SET, radioIndex, string, &len, NULL, NULL) < 0) {
		HAL_WIFI_ERR(("%s: Failed to set RadioGuardInterval\n", __FUNCTION__));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

//Get the Modulation Coding Scheme index, eg: "-1", "1", "15"
INT wifi_getRadioMCS(INT radioIndex, INT *MCS) //Tr181
{
	HAL_WIFI_DBG(("%s: Enter, radioIndex = %d\n", __FUNCTION__, radioIndex));
	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(MCS);

	int returnStatus;
	int len = sizeof(*MCS);
	*MCS = -1;
	returnStatus = wldm_Radio_MCS(CMD_GET, radioIndex, MCS, &len, NULL, NULL);
	if (RETURN_OK != returnStatus) {
		HAL_WIFI_ERR(("%s: wldm_Radio_MCS CMD_GET failed radioIndex = %d, Status=%d\n",
			__FUNCTION__, radioIndex, returnStatus));
		return RETURN_ERR;
	}
	HAL_WIFI_DBG(("%s: radioIndex = %d, MCS = %d\n", __FUNCTION__, radioIndex, *MCS));
	return RETURN_OK;
}

//Set the Modulation Coding Scheme index
INT wifi_setRadioMCS(INT radioIndex, INT MCS) //Tr181
{
	int returnStatus;
	int len = sizeof(MCS);

	HAL_WIFI_DBG(("%s: Enter, radioIndex = %d\n", __FUNCTION__, radioIndex));
	RADIO_INDEX_ASSERT(radioIndex);

	returnStatus = wldm_Radio_MCS(CMD_SET_IOCTL, radioIndex, &MCS, &len, NULL, NULL);
	if (RETURN_OK != returnStatus) {
		HAL_WIFI_ERR(("%s: wldm_Radio_MCS CMD_SET failed radioIndex=%d, Status=%d\n",
			__FUNCTION__, radioIndex, returnStatus));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: radioIndex = %d, MCS = %d\n", __FUNCTION__, radioIndex, MCS));

	return RETURN_OK;
}

//Device.WiFi.Radio.{i}.TransmitPowerSupported
//Get supported Transmit Power list, eg : "0,25,50,75,100"
//The output_list is a max length 64 octet string that is allocated by the RDKB code.  Implementations must ensure that strings are not longer than this.
INT wifi_getRadioTransmitPowerSupported(INT radioIndex, CHAR *output_list) //Tr181
{
	int len = OUTPUT_STRING_LENGTH_64;
	int returnStatus;

	HAL_WIFI_DBG(("%si:  Enter radioIndex %d\n", __FUNCTION__, radioIndex));
	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(output_list);

	output_list[0] = '\0';

	returnStatus = wldm_Radio_TransmitPowerSupported(CMD_GET, radioIndex, output_list, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_Radio_TransmitPowerSupported CMD_GET Failed, Status = %d\n",
			__FUNCTION__, returnStatus));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: radioIndex = %d, output_list = %s\n", __FUNCTION__, radioIndex, output_list));
	return RETURN_OK;
}

#if WIFI_HAL_VERSION_GE_2_18 || defined(WIFI_HAL_VERSION_3)
/* Device.WiFi.Radio.{i}.TransmitPower
* Get Transmit Power level as a units (percentage) of full power. eg : "0,25,50,75,100"
* The value MUST be one of the values reported by the {{param|TransmitPowerSupported}} parameter.
* A value of -1 indicates auto mode (automatic decision by CPE) */
INT wifi_getRadioPercentageTransmitPower(INT radioIndex, ULONG *output_ulong) //Tr181
{
	int transmitPower = 0;
	int returnStatus, len = sizeof(transmitPower);

	HAL_WIFI_DBG(("%s: Enter, radioIndex = %d\n", __FUNCTION__, radioIndex));
	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(output_ulong);

	returnStatus = wldm_Radio_TransmitPower(CMD_GET, radioIndex, &transmitPower, &len, NULL, NULL);
	if (RETURN_OK != returnStatus) {
		HAL_WIFI_ERR(("%s: wldm_Radio_TransmitPower CMD_GET failed for radioIndex = %d, Status=%d\n",
					__FUNCTION__, radioIndex, returnStatus));
		return RETURN_ERR;
	}

	*output_ulong = (ULONG)transmitPower;
	HAL_WIFI_DBG(("%s, radioIndex = %d, output_ulong = %lu\n", __FUNCTION__, radioIndex, *output_ulong));

	return RETURN_OK;
}
#endif /* #if (WIFI_HAL_VERSION_GE_2_18) */

INT wifi_getRadioTransmitPower(INT radioIndex, ULONG *output_ulong) //RDKB: absolute tx pwr in dBm
{
	HAL_WIFI_DBG(("%s: Enter, radioIndex = %d\n", __FUNCTION__, radioIndex));
	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(output_ulong);

	int		returnStatus;
	unsigned int	len = sizeof(*output_ulong);
	*output_ulong = 0;

	returnStatus = wldm_xbrcm_phy(CMD_GET, radioIndex, (int *)output_ulong, &len, "txpwr",
		NULL);
	if (RETURN_OK != returnStatus) {
		HAL_WIFI_ERR(("%s: wldm_xbrcm_phy CMD_GET failed for radioIndex = %d, Status=%d\n",
			__FUNCTION__, radioIndex, returnStatus));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s, radioIndex = %d, output_ulong = %lu\n", __FUNCTION__, radioIndex,
		*output_ulong));

	return RETURN_OK;
}

//Device.WiFi.Radio.{i}.TransmitPower
//Set Transmit Power level as a units (percentage) of full power. eg : "0,25,50,75,100"
//The value MUST be one of the values reported by the {{param|TransmitPowerSupported}} parameter.
//A value of -1 indicates auto mode (automatic decision by CPE)
INT wifi_setRadioTransmitPower(INT radioIndex, ULONG TransmitPower) //Tr181
{
	int len;

	RADIO_INDEX_ASSERT(radioIndex);

	len = sizeof(TransmitPower);
	if (wldm_Radio_TransmitPower(CMD_SET, radioIndex, (int *)&TransmitPower, &len, NULL, NULL) < 0) {
		HAL_WIFI_ERR(("%s: wldm_Radio_TranmitPower CMD_SET_IOCTL set %d failed.\n", __FUNCTION__, TransmitPower));
		return RETURN_ERR;
	}
#ifdef RDKB_TIMER
	wifi_set_timer_variable();
#endif
	HAL_WIFI_DBG(("%s: radioIndex = %d, set transmit power = %d\n", __FUNCTION__, radioIndex, TransmitPower));

	return RETURN_OK;
}

//get 80211h Supported.  80211h solves interference with satellites and radar using the same 5 GHz frequency band
INT wifi_getRadioIEEE80211hSupported(INT radioIndex, BOOL *Supported) //Tr181
{
	HAL_WIFI_DBG(("%s: Enter", __FUNCTION__));
	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(Supported);

	/* IEEE80211h is always supported */
	*Supported = TRUE;

	HAL_WIFI_DBG(("%s: DONE", __FUNCTION__));
	return RETURN_OK;
}

//Get 80211h feature enable
INT wifi_getRadioIEEE80211hEnabled(INT radioIndex, BOOL *enable) //Tr181
{
	HAL_WIFI_DBG(("%s: Enter, radioIndex=[%d]\n", __FUNCTION__, radioIndex));
	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(enable);

	*enable = FALSE;

	int returnStatus = RETURN_OK;
	int len = sizeof(*enable);
	returnStatus = wldm_Radio_IEEE80211hEnabled(CMD_GET, radioIndex, enable, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_Radio_IEEE80211hEnabled CMD_GET Failed, radioIndex=[%d], Status=[%d]\n",
			__FUNCTION__, radioIndex, returnStatus));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: radioIndex = %d, enable = %d\n", __FUNCTION__, radioIndex, *enable));
	return RETURN_OK;
}

//Set 80211h feature enable
INT wifi_setRadioIEEE80211hEnabled(INT radioIndex, BOOL enable)  //Tr181
{
	int returnStatus = RETURN_OK;
	int len = sizeof(enable);
	HAL_WIFI_DBG(("%s: Enter, radioIndex=[%d]\n", __FUNCTION__, radioIndex));
	RADIO_INDEX_ASSERT(radioIndex);

	returnStatus = wldm_Radio_IEEE80211hEnabled(CMD_SET, radioIndex, &enable, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_Radio_IEEE80211hEnabled CMD_SET Failed, radioIndex=[%d], Status=[%d]\n",
			__FUNCTION__, radioIndex, returnStatus));
		return RETURN_ERR;
	}
#ifdef RDKB_TIMER
	wifi_set_timer_variable();
#endif
	HAL_WIFI_DBG(("%s: radioIndex = %d, enable = %d\n", __FUNCTION__, radioIndex, enable));

	return RETURN_OK;
}

//Indicates the Carrier Sense ranges supported by the radio. It is measured in dBm. Refer section A.2.3.2 of CableLabs Wi-Fi MGMT Specification.
INT wifi_getRadioCarrierSenseThresholdRange(INT radioIndex, INT *output) //P3
{
	HAL_WIFI_DBG(("%s: radioIndex %d\n", __FUNCTION__, radioIndex));

	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(output);
	*output = 100;

	return RETURN_OK;
}

//The RSSI signal level at which CS/CCA detects a busy condition. This attribute enables APs to increase minimum sensitivity to avoid detecting busy condition from multiple/weak Wi-Fi sources in dense Wi-Fi environments. It is measured in dBm. Refer section A.2.3.2 of CableLabs Wi-Fi MGMT Specification.
INT wifi_getRadioCarrierSenseThresholdInUse(INT radioIndex, INT *output) //P3
{
	HAL_WIFI_DBG(("%s: radioIndex %d\n", __FUNCTION__, radioIndex));

	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(output);
	*output = -99;

	return RETURN_OK;
}

INT wifi_setRadioCarrierSenseThresholdInUse(INT radioIndex, INT threshold) //P3
{
	HAL_WIFI_DBG(("%s, radioIndex = %d, threshold = %d, not implemented!!!\n", __FUNCTION__, radioIndex, threshold));

	return RETURN_OK;
}

//Time interval between transmitting beacons (expressed in milliseconds). This parameter is based ondot11BeaconPeriod from [802.11-2012].
INT wifi_getRadioBeaconPeriod(INT radioIndex, UINT *output)
{
	HAL_WIFI_DBG(("wifi_getRadioBeaconPeriod radioIndex %d\n", radioIndex));
	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(output);

	int returnStatus, len = sizeof(*output);
	returnStatus = wldm_Radio_BeaconPeriod(CMD_GET, radioIndex, (int *) output, &len, NULL, NULL);
	if (returnStatus != 0) {
		HAL_WIFI_DBG(("%s: wldm_Radio_BeaconPeriod CMD_GET Failed, Status=%d\n",
				__FUNCTION__, returnStatus));
		return RETURN_ERR;
	}
	return RETURN_OK;
}

INT wifi_setRadioBeaconPeriod(INT radioIndex, UINT BeaconPeriod)
{
	int returnStatus;
	int len = sizeof(BeaconPeriod);

	HAL_WIFI_DBG(("wifi_setRadioBeaconPeriod radioIndex %d BeaconPeriod %d\n", radioIndex, BeaconPeriod));
	RADIO_INDEX_ASSERT(radioIndex);

	returnStatus = wldm_Radio_BeaconPeriod(CMD_SET, radioIndex, (int *) &BeaconPeriod, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_Radio_BeaconPeriod CMD_SET Fail, Status = %d\n",
			__FUNCTION__, returnStatus));
		return RETURN_ERR;
	}
#ifdef RDKB_TIMER
	wifi_set_timer_variable();
#endif
	HAL_WIFI_DBG(("%s: radioIndex = %d, BeaconPeriod = %d\n", __FUNCTION__, radioIndex, BeaconPeriod));
	return RETURN_OK;
}

INT wifi_setRadioOperationalDataTransmitRates(INT radioIndex, CHAR *rates)
{
	UNUSED_PARAMETER(radioIndex);
	UNUSED_PARAMETER(rates);
	// stub function
	return RETURN_ERR;
}

INT wifi_getRadioSupportedDataTransmitRates(INT radioIndex, CHAR *output)
{
	// Supported and operational Data Transmit Rates are same for BRCM.
	CHAR supportedDataTransmitRates[OUTPUT_STRING_LENGTH_256];
	int len;

	HAL_WIFI_DBG(("%s: radioIndex %d\n", __FUNCTION__, radioIndex));

	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(output);

	len = sizeof(supportedDataTransmitRates);
	if (wldm_Radio_SupportedDataTransmitRates(CMD_GET, radioIndex, supportedDataTransmitRates,
		&len, NULL, NULL) < 0) {
		HAL_WIFI_LOG(("%s: wldm_Radio_SupportedDataTransmitRates get failed\n", __FUNCTION__));
		return RETURN_ERR;
	}
	if (snprintf(output, OUTPUT_STRING_LENGTH_64, "%s", supportedDataTransmitRates) < 0) {
		HAL_WIFI_ERR(("%s: snprintf supportedDataTransmitRates failed\n", __FUNCTION__));
		return RETURN_ERR;
	}
	HAL_WIFI_DBG(("%s, supported rates: %s \n", __FUNCTION__, supportedDataTransmitRates));
	return RETURN_OK;
}

INT wifi_getRadioOperationalDataTransmitRates(INT radioIndex, CHAR *output)
{
	CHAR operationalDataTransmitRates[OUTPUT_STRING_LENGTH_256];
	int len;

	HAL_WIFI_DBG(("%s: radioIndex %d\n", __FUNCTION__, radioIndex));

	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(output);

	len = sizeof(operationalDataTransmitRates);
	if (wldm_Radio_OperationalDataTransmitRates(CMD_GET, radioIndex, operationalDataTransmitRates,
		&len, NULL, NULL) < 0) {
		HAL_WIFI_LOG(("%s: wldm_Radio_OperationalDataTransmitRate get failed\n", __FUNCTION__));
		return RETURN_ERR;
	}
	if (snprintf(output, OUTPUT_STRING_LENGTH_64, "%s", operationalDataTransmitRates) < 0) {
		HAL_WIFI_ERR(("%s: snprintf operationalDataTransmitRates failed\n", __FUNCTION__));
		return RETURN_ERR;
	}
	return RETURN_OK;
}

/*
  Return: Comma-separated rate list.
	The set of data rates, in Mbps, that have to be supported by all stations that desire to join this BSS.
	The stations have to be able to receive and transmit at each of the data rates listed inBasicDataTransmitRates.
	For example, a value of "1,2", indicates that stations support 1 Mbps and 2 Mbps.
	Most control packets use a data rate in BasicDataTransmitRates.
*/
INT wifi_getRadioBasicDataTransmitRates(INT radioIndex, CHAR *output)
{
	CHAR basicDataTransmitRates[OUTPUT_STRING_LENGTH_64];
	int len;

	HAL_WIFI_DBG(("%s: radioIndex %d\n", __FUNCTION__, radioIndex));

	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(output);

	len = sizeof(basicDataTransmitRates);
	if (wldm_Radio_BasicDataTransmitRates(CMD_GET, radioIndex, basicDataTransmitRates,
		&len, NULL, NULL) < 0) {
		HAL_WIFI_LOG(("%s: wldm_Radio_BasicDataTransmitRates CMD_GET failed\n", __FUNCTION__));
		return RETURN_ERR;
	}
	if (snprintf(output, OUTPUT_STRING_LENGTH_32, "%s", basicDataTransmitRates) < 0) {
		HAL_WIFI_ERR(("%s: snprintf basicDataTransmitRates failed\n", __FUNCTION__));
		return RETURN_ERR;
	}
	HAL_WIFI_DBG(("%s: basic rates: %s \n", __FUNCTION__, basicDataTransmitRates));

	return RETURN_OK;
}

/*
* wifi_setRadioBasicDataTransmitRates() function
*
* @brief Set the data rates, in Mbps.
*
* This have to be supported by all stations that desire to join this BSS.
* The stations have to be able to receive and transmit at each of the data rates listed inBasicDataTransmitRates.
* For example, a value of "1,2", indicates that stations support 1 Mbps and 2 Mbps.
* Most control packets use a data rate in BasicDataTransmitRates.
* Device.WiFi.Radio.{i}.BasicDataTransmitRates
*
* @param[in] radioIndex		Index of Wi-Fi radio channel
* @param[in] TransmitRates	Comma-separated list of strings
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
INT wifi_setRadioBasicDataTransmitRates(INT radioIndex, CHAR *BasicRates)
{
	HAL_WIFI_LOG(("%s: radioIndex %d BasicRates %s\n", __FUNCTION__, radioIndex, BasicRates));

	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(BasicRates);
	int slen = strlen(BasicRates) + 1;

	if (wldm_Radio_BasicDataTransmitRates(CMD_SET, radioIndex, BasicRates,
		&slen, NULL, NULL) < 0) {
		HAL_WIFI_LOG(("%s: wldm_Radio_BasicRates CMD SET failed\n", __FUNCTION__));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

//Get detail radio traffic static info
INT wifi_getRadioTrafficStats2(INT radioIndex, wifi_radioTrafficStats2_t *output_struct) //Tr181
{
	int returnStatus;
	Device_WiFi_Radio_TrafficStats2 traffic_stats;
	int len = sizeof(traffic_stats);

	HAL_WIFI_DBG(("%s: Enter, radioIndex = %d\n", __FUNCTION__, radioIndex));
	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(output_struct);
	RADIO_NOT_UP_ASSERT(radioIndex);

	memset(output_struct, 0, sizeof(wifi_radioTrafficStats2_t));
	memset(&traffic_stats, 0, sizeof(traffic_stats));

	returnStatus = wldm_Radio_TrafficStats2(CMD_GET, radioIndex, &traffic_stats, &len, NULL,
		NULL);
	if (RETURN_OK != returnStatus) {
		HAL_WIFI_DBG(("%s: wldm_Radio_TrafficStats2 CMD GET failed radioIndex = %d, Status=%d\n",
			__FUNCTION__, radioIndex, returnStatus));
		output_struct = NULL;
		return RETURN_ERR;
	}
	output_struct->radio_BytesSent = traffic_stats.radio_BytesSent;
	output_struct->radio_BytesReceived = traffic_stats.radio_BytesReceived;
	output_struct->radio_PacketsSent = traffic_stats.radio_PacketsSent;
	output_struct->radio_PacketsReceived = traffic_stats.radio_PacketsReceived;
	output_struct->radio_ErrorsSent = traffic_stats.radio_ErrorsSent;
	output_struct->radio_ErrorsReceived = traffic_stats.radio_ErrorsReceived;
	output_struct->radio_DiscardPacketsSent = traffic_stats.radio_DiscardPacketsSent;
	output_struct->radio_DiscardPacketsReceived = traffic_stats.radio_DiscardPacketsReceived;
	output_struct->radio_PLCPErrorCount = traffic_stats.radio_PLCPErrorCount;
	output_struct->radio_FCSErrorCount = traffic_stats.radio_FCSErrorCount;
	output_struct->radio_InvalidMACCount = traffic_stats.radio_InvalidMACCount;
	output_struct->radio_PacketsOtherReceived = traffic_stats.radio_PacketsOtherReceived;
	output_struct->radio_NoiseFloor = traffic_stats.radio_NoiseFloor;
	output_struct->radio_ChannelUtilization = traffic_stats.radio_ChannelUtilization;
	output_struct->radio_ActivityFactor = traffic_stats.radio_ActivityFactor;
	output_struct->radio_CarrierSenseThreshold_Exceeded =
		traffic_stats.radio_CarrierSenseThreshold_Exceeded;
	output_struct->radio_RetransmissionMetirc = traffic_stats.radio_RetransmissionMetirc;
	output_struct->radio_MaximumNoiseFloorOnChannel =
		traffic_stats.radio_MaximumNoiseFloorOnChannel;
	output_struct->radio_MinimumNoiseFloorOnChannel =
		traffic_stats.radio_MinimumNoiseFloorOnChannel;
	output_struct->radio_MedianNoiseFloorOnChannel =
		traffic_stats.radio_MedianNoiseFloorOnChannel;
	output_struct->radio_StatisticsStartTime = traffic_stats.radio_StatisticsStartTime;

	HAL_WIFI_DBG(("%s: Completed, radioIndex = %d\n", __FUNCTION__, radioIndex));

	return RETURN_OK;
}

//Clients associated with the AP over a specific interval.  The histogram MUST have a range from -110to 0 dBm and MUST be divided in bins of 3 dBM, with bins aligning on the -110 dBm end of the range.  Received signal levels equal to or greater than the smaller boundary of a bin and less than the larger boundary are included in the respective bin.  The bin associated with the client?s current received signal level MUST be incremented when a client associates with the AP. Additionally, the respective bins associated with each connected client?s current received signal level MUST be incremented at the interval defined by "Radio Statistics Measuring Rate".  The histogram?s bins MUST NOT be incremented at any other time.  The histogram data collected during the interval MUST be published to the parameter only at the end of the interval defined by "Radio Statistics Measuring Interval".  The underlying histogram data MUST be cleared at the start of each interval defined by "Radio Statistics Measuring Interval?. If any of the parameter's representing this histogram is queried before the histogram has been updated with an initial set of data, it MUST return -1. Units dBm
INT wifi_getRadioStatsReceivedSignalLevel(INT radioIndex, INT signalIndex, INT *SignalLevel) //Tr181
{
	UNUSED_PARAMETER(radioIndex);
	UNUSED_PARAMETER(signalIndex);
	//zqiu: Please ignore signalIndex.
	HAL_WIFI_DBG(("wifi_getRadioStatsReceivedSignalLevel radioIndex %d\n", radioIndex));
	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(SignalLevel);

	*SignalLevel = -19;
	return RETURN_OK;
}

//Not all implementations may need this function.  If not needed for a particular implementation simply return no-error (0)
INT wifi_applyRadioSettings(INT radioIndex)
{
	RADIO_INDEX_ASSERT(radioIndex);

	HAL_WIFI_DBG(("%s, radioIndex = %d\n", __FUNCTION__, radioIndex));
	if (wldm_apply_RadioObject(radioIndex) == 0) {
		return RETURN_OK;
	}
	else {
		return RETURN_ERR;
	}
}

//Get the radio index assocated with this SSID entry
INT wifi_getSSIDRadioIndex(INT ssidIndex, INT *radioIndex)
{
	NULL_PTR_ASSERT(radioIndex);
	AP_INDEX_ASSERT(ssidIndex);

	*radioIndex = wldm_get_radioIndex(ssidIndex);
	HAL_WIFI_DBG(("%s, ssidIndex = %d, radioIndex = %d\n", __FUNCTION__, ssidIndex, *radioIndex));

	return RETURN_OK;
}

//Get SSID enable configuration parameters (not the SSID enable status)
INT wifi_getSSIDEnable(INT ssidIndex, BOOL *output_bool) //Tr181
{
	HAL_WIFI_DBG(("%s, ssidIndex = %d, call wifi_getApEnable\n", __FUNCTION__, ssidIndex));
	return wifi_getApEnable(ssidIndex, output_bool);
}

//Set SSID enable configuration parameters
INT wifi_setSSIDEnable(INT ssidIndex, BOOL enable) //Tr181
{
	HAL_WIFI_DBG(("%s, ssidIndex = %d, enable = %d, call wifi_setApEnable\n", __FUNCTION__, ssidIndex, enable));
	return wifi_setApEnable(ssidIndex, enable);

}

/* Get the SSID enable status - return Enabled or Disable to match cosa */
INT wifi_getSSIDStatus(INT ssidIndex, CHAR *output_string) //Tr181
{
	BOOLEAN status = FALSE;
	int len = sizeof(status), iRet = RETURN_OK;

	HAL_WIFI_DBG(("%s: ssidIndex = %d\n", __FUNCTION__, ssidIndex));
	NULL_PTR_ASSERT(output_string);
	AP_INDEX_ASSERT(ssidIndex);

	iRet = wldm_SSID_Enable(CMD_GET, ssidIndex, &status, &len, NULL, NULL);
	if (RETURN_OK == iRet) {
		snprintf(output_string, OUTPUT_STRING_LENGTH_64, "%s",
			status ? "Enabled" : "Disable");
		HAL_WIFI_DBG(("%s: ssidIndex %d output_string=%s\n",
				__FUNCTION__, ssidIndex, output_string));
	}
	else {
		//HAL_WIFI_ERR(("%s: %d Error!!!\n", __FUNCTION__, __LINE__));
		iRet = RETURN_ERR;
	}

	return iRet;
}

// Outputs a 32 byte or less string indicating the SSID name.  Sring buffer must be preallocated by the caller.
INT wifi_getSSIDName(INT apIndex, CHAR *output)
{
	int len = OUTPUT_STRING_LENGTH_32  + 1;

	HAL_WIFI_DBG(("%s, apIndex [%d]\n", __FUNCTION__, apIndex));
	NULL_PTR_ASSERT(output);
	AP_INDEX_ASSERT(apIndex);
#ifdef RDKB_ONE_WIFI
	if (wldm_SSID_SSID(CMD_GET, apIndex, output, &len, NULL, NULL) == 0) {
		HAL_WIFI_DBG(("%s, CMD_GET apIndex [%d] ssid=%s\n", __FUNCTION__, apIndex, output));
		return RETURN_OK;
	}
#else
	if (wldm_SSID_SSID(CMD_GET_NVRAM, apIndex, output, &len, NULL, NULL) == 0) {
		HAL_WIFI_DBG(("%s, CMD_GET_NVRAM apIndex [%d] ssid=%s\n", __FUNCTION__, apIndex, output));
		return RETURN_OK;
	}
#endif /* RDKB_ONE_WIFI */
	HAL_WIFI_ERR(("%s, CMD_GET_NVRAM failed, ERROR!!!\n", __FUNCTION__));
	return RETURN_ERR;
}

// To read the run time ssid name
INT wifi_getSSIDNameStatus(INT apIndex, CHAR *output_string)
{
	int len = OUTPUT_STRING_LENGTH_32  + 1;

	HAL_WIFI_DBG(("%s, apIndex [%d]\n", __FUNCTION__, apIndex));
	NULL_PTR_ASSERT(output_string);
	AP_INDEX_ASSERT(apIndex);

	if (wldm_SSID_SSID(CMD_GET, apIndex, output_string, &len, NULL, NULL) == 0) {
		HAL_WIFI_DBG(("%s, CMD_GET apIndex [%d] ssid=%s\n", __FUNCTION__, apIndex, output_string));
		return RETURN_OK;
	}
	HAL_WIFI_ERR(("%s, CMD_GET apIndex [%d] Error!!!\n", __FUNCTION__, apIndex));
	return RETURN_ERR;
}

// Set a max 32 byte string and sets an internal variable to the SSID name
INT wifi_setSSIDName(INT apIndex, CHAR *ssid_string)
{
	NULL_PTR_ASSERT(ssid_string);
	AP_INDEX_ASSERT(apIndex);

	HAL_WIFI_LOG(("%s,  apIndex = %d, ssid = %s\n", __FUNCTION__, apIndex, ssid_string));
	int len;

	len = strlen(ssid_string);
	if (wldm_SSID_SSID(CMD_SET, apIndex, ssid_string, &len, NULL, NULL) < 0) {
		HAL_WIFI_ERR(("%s, CMD_SET apIndex = %d, ssid = %s Failed!!\n",
			__FUNCTION__, apIndex, ssid_string));
		return RETURN_ERR;
	}

#ifdef RDKB_TIMER
	wifi_set_timer_variable();
#endif
	return RETURN_OK;
}

//Get the BSSID
INT wifi_getBaseBSSID(INT ssidIndex, CHAR *output_string) //RDKB
{
	int returnStatus;
	int len = OUTPUT_STRING_LENGTH_18 + 1;
	HAL_WIFI_DBG(("wifi_getBaseBSSID ssidIndex %d\n", ssidIndex));

	NULL_PTR_ASSERT(output_string);
	AP_INDEX_ASSERT(ssidIndex);

	output_string[0] = '\0';
	returnStatus = wldm_SSID_MACAddress(CMD_GET, ssidIndex, output_string, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_SSID_MACAddress CMD_GET Failed, ssidIndex=[%d], Status=[%d]\n",
			__FUNCTION__, ssidIndex, returnStatus));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: ssidIndex=[%d] output_string=[%s]\n", __FUNCTION__, ssidIndex, output_string));

	return RETURN_OK;
}

//Get the MAC address associated with this Wifi SSID
INT wifi_getSSIDMACAddress(INT ssidIndex, CHAR *output_string) //Tr181
{
	int returnStatus;
	int len = OUTPUT_STRING_LENGTH_18 + 1;
	HAL_WIFI_DBG(("wifi_getSSIDMACAddress ssidIndex %d\n", ssidIndex));

	NULL_PTR_ASSERT(output_string);
	AP_INDEX_ASSERT(ssidIndex);

	output_string[0] = '\0';

	if (!WiFi_Ready_after_hal_started) {
		returnStatus = wldm_SSID_MACAddress(CMD_GET_NVRAM, ssidIndex, output_string, &len, NULL, NULL);
	} else {
		returnStatus = wldm_SSID_MACAddress(CMD_GET, ssidIndex, output_string, &len, NULL, NULL);
	}

	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_SSID_MACAddress CMD_GET Failed, ssidIndex=[%d], Status=[%d]\n",
			__FUNCTION__, ssidIndex, returnStatus));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: ssidIndex=[%d] output_string=[%s]\n", __FUNCTION__, ssidIndex, output_string));

	return RETURN_OK;
}

INT WiFi_SSIDStatsNetDevStatsGetNext(FILE *fs, wifi_net_dev_stats *net_dev_ctx)
{
	char line[OUTPUT_STRING_LENGTH_512] = {'\0'};
	int length, cnt=0;
	/* check if reach the end of file */
	if (fgets(line, sizeof(line), fs) == NULL) {
		HAL_WIFI_DBG(("%s: Fail, Failed reading at line %s \n", __FUNCTION__, line));
		return RETURN_ERR;
	}

	cnt = sscanf(line," %127s  %lu  %lu  %lu  %lu  %lu  %lu  %lu  %lu  %lu  %lu  %lu  %lu  %lu  %lu  %lu  %lu",
		net_dev_ctx->interfacename,&(net_dev_ctx->RxBytes),&(net_dev_ctx->RxPkts),&(net_dev_ctx->RxErrors),
		&(net_dev_ctx->RxDrops),&(net_dev_ctx->RxFifo),&(net_dev_ctx->RxFrame),&(net_dev_ctx->RxCompressed),
		&(net_dev_ctx->RxMulticast), &(net_dev_ctx->TxBytes),&(net_dev_ctx->TxPkts),&(net_dev_ctx->TxErrors),
		&(net_dev_ctx->TxDrops),&(net_dev_ctx->TxFifo),&(net_dev_ctx->TxFrame),&(net_dev_ctx->TxCompressed),
		&(net_dev_ctx->TxMulticast));

	/* if we didn't read in 17 parameters, return failure */
	if(17 > cnt)
	{
		HAL_WIFI_DBG(("%s: Fail, Failed reading at line %s \n", __FUNCTION__, line));
		return RETURN_ERR;
	}
	/* scanf always read in interface name with the ':' character, as 'eth0:', 'cm0:',...
	remove the last ':' character, so interface name become 'eth0', 'cm0',... */
	length = strlen(net_dev_ctx->interfacename);
	if(length > 0) {
		if(net_dev_ctx->interfacename[length - 1] == ':')
			net_dev_ctx->interfacename[length - 1] = '\0';
	}
	return RETURN_OK;
}

static INT  WiFi_SSIDStatsGet(UINT32 sSIDInstance, wifi_net_dev_stats *net_stats)
{
	int rc = RETURN_OK;
	CHAR line[OUTPUT_STRING_LENGTH_512] = {'\0'}, *devName, *result;

	AP_INDEX_ASSERT(sSIDInstance);
	NULL_PTR_ASSERT(net_stats);
	devName = wldm_get_nvifname(sSIDInstance);

	HAL_WIFI_DBG(("Looking for stats for interface[%s]", devName));

	FILE* fs = fopen("/proc/net/dev", "r");

	if ( fs == NULL ) {
		HAL_WIFI_DBG(("%s: fopen() fail !!", __FUNCTION__));
		return RETURN_ERR;
	}

	/* first two lines are column description, ignore them */
	result = fgets(line, sizeof(line), fs);
	if (result == NULL) {
		result = fgets(line, sizeof(line), fs);
		if (result == NULL) {
			HAL_WIFI_ERR(("%s: fgets for first two lines failed!!\n", __FUNCTION__));
			fclose(fs);
			return RETURN_ERR;
		}
	}

	/* read in line until the file end */
	while (WiFi_SSIDStatsNetDevStatsGetNext(fs, net_stats) == RETURN_OK) {
		/* check if is is the interface we're looking for, if yes return with the data */
		if(!strcmp(devName, net_stats->interfacename)) {
			rc = RETURN_OK;
			goto exit;
		}
	}

	if (fs) {
		fclose(fs);
		fs = NULL;
	}
	HAL_WIFI_DBG(("Didn't find interface[%s] in net_dev_stats\n", devName));
	rc = RETURN_ERR;

exit:
	if (fs) {
		fclose(fs);
		fs = NULL;
	}
	return rc;
}

//Get the basic SSID traffic static info
INT wifi_getSSIDTrafficStats2(INT ssidIndex, wifi_ssidTrafficStats2_t *output_struct) //Tr181
{
	int returnStatus, len;
	Device_WiFi_SSID_Stats ssid_stats;
	boolean enable;

	HAL_WIFI_DBG(("%s: ssidIndex %d\n", __FUNCTION__, ssidIndex));
	NULL_PTR_ASSERT(output_struct);
	AP_INDEX_ASSERT(ssidIndex);

	HAL_WIFI_DBG(("%s: ssidIndex %d", __FUNCTION__, ssidIndex));

	memset(output_struct, 0, sizeof(*output_struct));

	len = sizeof(enable);
	if (wldm_SSID_Enable(CMD_GET, ssidIndex, &enable, &len,  NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: wldm_SSID_Enable CMD_GET Failed, ssidIndex=[%d]\n",
			__FUNCTION__, ssidIndex));
		return RETURN_ERR;
	}

	if (!enable) {
		HAL_WIFI_DBG(("%s: ssidIndex=[%d] is disabled.\n",
			__FUNCTION__, ssidIndex));
		return RETURN_OK;
	}

	len = sizeof(ssid_stats);
	memset(&ssid_stats, 0, sizeof(ssid_stats));

	returnStatus =  wldm_SSID_TrafficStats(CMD_GET, ssidIndex, &ssid_stats, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_SSID_TrafficStats Failed, ssidIndex=[%d]  returnStatus=[%d]\n",
			__FUNCTION__, ssidIndex, returnStatus));
		return RETURN_ERR;
	}
	output_struct->ssid_BytesSent = ssid_stats.BytesSent;
	output_struct->ssid_BytesReceived = ssid_stats.BytesReceived;
	output_struct->ssid_PacketsSent = ssid_stats.PacketsSent;
	output_struct->ssid_PacketsReceived = ssid_stats.PacketsReceived;
	output_struct->ssid_ErrorsSent = ssid_stats.ErrorsSent;
	output_struct->ssid_ErrorsReceived = ssid_stats.ErrorsReceived;
	output_struct->ssid_UnicastPacketsSent = ssid_stats.UnicastPacketsSent;
	output_struct->ssid_UnicastPacketsReceived = ssid_stats.UnicastPacketsReceived;
	output_struct->ssid_DiscardedPacketsSent = ssid_stats.DiscardPacketsSent;
	output_struct->ssid_DiscardedPacketsReceived = ssid_stats.DiscardPacketsReceived;
	output_struct->ssid_MulticastPacketsSent = ssid_stats.MulticastPacketsSent;
	output_struct->ssid_MulticastPacketsReceived = ssid_stats.MulticastPacketsReceived;

	HAL_WIFI_DBG(("ssid_BytesSent               %lu\n", output_struct->ssid_BytesSent));
	HAL_WIFI_DBG(("ssid_BytesReceived           %lu\n", output_struct->ssid_BytesReceived));
	HAL_WIFI_DBG(("ssid_PacketsSent             %lu\n", output_struct->ssid_PacketsSent));
	HAL_WIFI_DBG(("ssid_PacketsReceived         %lu\n", output_struct->ssid_PacketsReceived));
	HAL_WIFI_DBG(("ssid_ErrorsSent              %lu\n", output_struct->ssid_ErrorsSent));
	HAL_WIFI_DBG(("ssid_ErrorsReceived          %lu\n", output_struct->ssid_ErrorsReceived));
	HAL_WIFI_DBG(("ssid_UnicastPacketsSent      %lu\n", output_struct->ssid_UnicastPacketsSent));
	HAL_WIFI_DBG(("ssid_UnicastPacketsReceived  %lu\n", output_struct->ssid_UnicastPacketsReceived));
	HAL_WIFI_DBG(("ssid_DiscardedPacketsSent    %lu\n", output_struct->ssid_DiscardedPacketsSent));
	HAL_WIFI_DBG(("ssid_DiscardedPacketsReceived%lu\n", output_struct->ssid_DiscardedPacketsReceived));
	HAL_WIFI_DBG(("ssid_MulticastPacketsSent    %lu\n", output_struct->ssid_MulticastPacketsSent));
	HAL_WIFI_DBG(("ssid_MulticastPacketsReceived%lu\n", output_struct->ssid_MulticastPacketsReceived));

	HAL_WIFI_DBG(("%s: Done for ssidIndex %d\n", __FUNCTION__, ssidIndex));
	return RETURN_OK;
}

//Apply SSID and AP (in the case of Acess Point devices) to the hardware
//Not all implementations may need this function.  If not needed for a particular implementation simply return no-error (0)
INT wifi_applySSIDSettings(INT apIndex)
{
	AP_INDEX_ASSERT(apIndex);

	if (wldm_apply_AccessPointObject(apIndex) == 0) {
		HAL_WIFI_DBG(("%s, Done apply_AccessPointObject apIndex [%d]\n", __FUNCTION__, apIndex));
		return RETURN_OK;
	}
	else {
		HAL_WIFI_ERR(("%s, apply_AccessPointObject Failed apIndex [%d]\n", __FUNCTION__, apIndex));
		return RETURN_ERR;
	}
	/* wifi_apply */
}

/*	HAL function should allocate an data structure array, and return to
	caller with "neighbor_ap_array"
	scan argument controls whether or not a scan needs to be done
	If scan is set to FALSE, this API needs to be preceded with wifi_startNeighborScan */
#ifdef WIFI_HAL_VERSION_3_PHASE2
INT wifi_getNeighboringWiFiDiagnosticResult2(INT radioIndex, BOOL scan,
	wifi_neighbor_ap2_t **neighbor_ap_array, UINT *output_array_size)
{
	RADIO_INDEX_ASSERT(radioIndex);
	HAL_WIFI_DBG(("%s: radioIndex %d scan %d DEFAULT_SCAN_DWELL_TIME %d\n", __FUNCTION__, radioIndex,
		scan, DEFAULT_SCAN_DWELL_TIME(radioIndex)));

	if (scan) {
		if (start_neighbor_scan(radioIndex, WLDM_RADIO_SCAN_MODE_FULL,
			DEFAULT_SCAN_DWELL_TIME(radioIndex), 0, NULL) != RETURN_OK) {
			HAL_WIFI_ERR(("%s: start_neighbor_scan failed radioIndex [%d] errno [%d]\n",
				__FUNCTION__, radioIndex, errno));
			errno = EAGAIN;
			return WIFI_HAL_ERROR;
		}
	}
	if (get_neighboring_wifi_status(radioIndex, neighbor_ap_array, output_array_size) != RETURN_OK) {
		HAL_WIFI_ERR(("%s: get_neighboring_wifi_status failed radioIndex [%d] errno [%d]\n",
			__FUNCTION__, radioIndex, errno));
		errno = EAGAIN;
		return WIFI_HAL_ERROR;
	}
	errno = 0;
	return WIFI_HAL_SUCCESS;
}
#else /* WIFI_HAL_VERSION_3_PHASE2 */
INT wifi_getNeighboringWiFiDiagnosticResult2(INT radioIndex, wifi_neighbor_ap2_t **neighbor_ap_array,
	UINT *output_array_size) //Tr181
{
	RADIO_INDEX_ASSERT(radioIndex);
	HAL_WIFI_DBG(("%s: radioIndex %d DEFAULT_SCAN_DWELL_TIME %d\n", __FUNCTION__, radioIndex,
		DEFAULT_SCAN_DWELL_TIME(radioIndex)));

	if (start_neighbor_scan(radioIndex, WLDM_RADIO_SCAN_MODE_FULL,
		DEFAULT_SCAN_DWELL_TIME(radioIndex), 0, NULL) != RETURN_OK) {
		HAL_WIFI_ERR(("%s: start_neighbor_scan failed radioIndex [%d] errno [%d] \n",
			__FUNCTION__, radioIndex, errno));
		errno = EAGAIN;
		return RETURN_ERR;
	}
	return get_neighboring_wifi_status(radioIndex, neighbor_ap_array, output_array_size);
}
#endif /* WIFI_HAL_VERSION_3_PHASE2 */

//>> Deprecated: used for old RDKB code.
INT wifi_getRadioWifiTrafficStats(INT radioIndex, wifi_radioTrafficStats_t *output_struct)
{
	wifi_radioTrafficStats2_t traffic_stats;
	int ret;

	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(output_struct);
	RADIO_NOT_UP_ASSERT(radioIndex);

	ret = wifi_getRadioTrafficStats2(radioIndex, &traffic_stats);
	if (ret != RETURN_OK)
		return ret;

	output_struct->wifi_ErrorsSent = traffic_stats.radio_ErrorsSent;
	output_struct->wifi_ErrorsReceived = traffic_stats.radio_ErrorsReceived;
	output_struct->wifi_DiscardPacketsSent = traffic_stats.radio_DiscardPacketsSent;
	output_struct->wifi_DiscardPacketsReceived = traffic_stats.radio_DiscardPacketsReceived;
	output_struct->wifi_PLCPErrorCount = traffic_stats.radio_PLCPErrorCount;
	output_struct->wifi_FCSErrorCount = traffic_stats.radio_FCSErrorCount;
	output_struct->wifi_InvalidMACCount = traffic_stats.radio_InvalidMACCount;
	output_struct->wifi_PacketsOtherReceived = traffic_stats.radio_PacketsOtherReceived;
	output_struct->wifi_Noise = traffic_stats.radio_NoiseFloor;

	return RETURN_OK;
}

INT wifi_getBasicTrafficStats(INT apIndex, wifi_basicTrafficStats_t *output_struct)
{
	HAL_WIFI_DBG(("%s: Enter, ssidIndex %d\n", __FUNCTION__, apIndex));
	int returnStatus;
	Device_WiFi_SSID_Stats ssid_stats;
	int len = sizeof(ssid_stats);

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(output_struct);

	HAL_WIFI_DBG(("%s: apIndex %d", __FUNCTION__, apIndex));

	memset(output_struct, 0, sizeof(wifi_basicTrafficStats_t));

	returnStatus = wifi_getApNumDevicesAssociated(apIndex, &output_struct->wifi_Associations);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: Failed to get wifi associations\n", __FUNCTION__));
		return RETURN_ERR;
	}

	if (output_struct->wifi_Associations == 0) {
		HAL_WIFI_DBG(("%s: No device connected on apIndex %d.\n", __FUNCTION__, apIndex));
		return RETURN_OK;
	}

	memset(&ssid_stats, 0, sizeof(ssid_stats));

	returnStatus = wldm_SSID_TrafficStats(CMD_GET, apIndex, &ssid_stats, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_SSID_TrafficStats Failed, apIndex=[%d]  returnStatus=[%d]\n",
			__FUNCTION__, apIndex, returnStatus));
		return RETURN_ERR;
	}

	output_struct->wifi_BytesSent = ssid_stats.BytesSent;
	output_struct->wifi_BytesReceived = ssid_stats.BytesReceived;
	output_struct->wifi_PacketsSent = ssid_stats.PacketsSent;
	output_struct->wifi_PacketsReceived = ssid_stats.PacketsReceived;

	HAL_WIFI_DBG(("wifi_BytesSent       %lu\n", output_struct->wifi_BytesSent));
	HAL_WIFI_DBG(("wifi_BytesReceived   %lu\n", output_struct->wifi_BytesReceived));
	HAL_WIFI_DBG(("wifi_PacketsSent     %lu\n", output_struct->wifi_PacketsSent));
	HAL_WIFI_DBG(("wifi_PacketsReceived %lu\n", output_struct->wifi_PacketsReceived));
	HAL_WIFI_DBG(("wifi_Associations    %lu\n", output_struct->wifi_Associations));

	HAL_WIFI_DBG(("%s: Done for ssidIndex %d \n", __FUNCTION__, apIndex));

	return RETURN_OK;
}

INT wifi_getWifiTrafficStats(INT apIndex, wifi_trafficStats_t *output_struct)
{
	int returnStatus;
	wifi_net_dev_stats net_stats;

	HAL_WIFI_DBG(("%s: Enter, ssidIndex %d\n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(output_struct);

	memset(output_struct, 0, sizeof(wifi_trafficStats_t));
	returnStatus = WiFi_SSIDStatsGet(apIndex, &net_stats);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: Failed to get Staticstics info\n", __FUNCTION__));
		return RETURN_ERR;
	}

	output_struct->wifi_ErrorsSent = net_stats.TxErrors;
	output_struct->wifi_ErrorsReceived =  net_stats.RxErrors;
	output_struct->wifi_UnicastPacketsSent = net_stats.TxPkts;
	output_struct->wifi_UnicastPacketsReceived = net_stats.RxPkts;
	output_struct->wifi_DiscardedPacketsSent = net_stats.TxDrops;
	output_struct->wifi_DiscardedPacketsReceived = net_stats.RxDrops;
	output_struct->wifi_MulticastPacketsSent = net_stats.TxMulticast;
	output_struct->wifi_MulticastPacketsReceived = net_stats.RxMulticast;

	HAL_WIFI_DBG((" wifi_ErrorsSent                 %lu\n", output_struct->wifi_ErrorsSent));
	HAL_WIFI_DBG((" wifi_ErrorsReceived             %lu\n", output_struct->wifi_ErrorsReceived));
	HAL_WIFI_DBG((" wifi_UnicastPacketsSent         %lu\n", output_struct->wifi_UnicastPacketsSent));
	HAL_WIFI_DBG((" wifi_UnicastPacketsReceived     %lu\n", output_struct->wifi_UnicastPacketsReceived));
	HAL_WIFI_DBG((" wifi_DiscardedPacketsSent       %lu\n", output_struct->wifi_DiscardedPacketsSent));
	HAL_WIFI_DBG((" wifi_DiscardedPacketsReceived   %lu\n", output_struct->wifi_DiscardedPacketsReceived));
	HAL_WIFI_DBG((" wifi_MulticastPacketsSent       %lu\n", output_struct->wifi_MulticastPacketsSent));
	HAL_WIFI_DBG((" wifi_MulticastPacketsReceived   %lu\n", output_struct->wifi_MulticastPacketsReceived));

	/*
	* Missing values in /proc/net/dev file
	* No input for wifi_BroadcastPacketsSent;
	* wifi_BroadcastPacketsRecevied;
	* wifi_UnknownPacketsReceived;
	*/

	HAL_WIFI_DBG(("%s: Done , ssidIndex %d\n", __FUNCTION__, apIndex));

	return RETURN_OK;
}

INT wifi_getSSIDTrafficStats(INT apIndex, wifi_ssidTrafficStats_t *output_struct)
{
	UNUSED_PARAMETER(apIndex);
	UNUSED_PARAMETER(output_struct);
	HAL_WIFI_DBG(("%s: There is version 2 of this API so please use that\n", __FUNCTION__));
	return RETURN_OK;
}

INT wifi_getNeighboringWiFiDiagnosticResult(wifi_neighbor_ap_t **neighbor_ap_array, UINT *output_array_size)
{
	UNUSED_PARAMETER(neighbor_ap_array);
	UNUSED_PARAMETER(output_array_size);
	HAL_WIFI_DBG(("%s: There is version 2 of this API so please use that\n", __FUNCTION__));
	return RETURN_OK;
}

//----------------- AP HAL -------------------------------

//>> Deprecated: used for old RDKB code.
INT wifi_getAssociatedDeviceDetail(INT apIndex, INT devIndex, wifi_device_t *output_struct)
{
	UNUSED_PARAMETER(apIndex);
	UNUSED_PARAMETER(devIndex);
	UNUSED_PARAMETER(output_struct);
	HAL_WIFI_DBG(("%s: Deprecated, Won't be implemented\n", __FUNCTION__));
	return RETURN_ERR;
}

INT wifi_getAllAssociatedDeviceDetail(INT apIndex, ULONG *output_ulong, wifi_device_t **output_struct)
{
	INT ret;
	wifi_associated_dev2_t *associated_dev_array = NULL, *pt = NULL;
	wifi_device_t *pwifi_dev = NULL;
	UINT i = 0;
	UINT array_size = 0;

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(output_ulong);
	NULL_PTR_ASSERT(output_struct);

	ret = wifi_getApAssociatedDeviceDiagnosticResult2(apIndex, &associated_dev_array,
		&array_size);
	if (ret != RETURN_OK)
		return ret;

	*output_ulong = array_size;
	if (*output_ulong == 0) {
		free(associated_dev_array);
		return ret;
	}

	pwifi_dev = (wifi_device_t*) malloc(array_size * sizeof(wifi_device_t));
	if (pwifi_dev == NULL) {
		HAL_WIFI_ERR(("%s:%d Malloc failed\n", __FUNCTION__, __LINE__));
		free(associated_dev_array);
		return RETURN_ERR;
	}
	memset(pwifi_dev, 0, array_size * sizeof(wifi_device_t));

	for (i = 0, pt = associated_dev_array; i < array_size; i++, pt++) {
		memcpy(pwifi_dev[i].wifi_devMacAddress, pt->cli_MACAddress, 6);
		pwifi_dev[i].wifi_devAssociatedDeviceAuthentiationState = pt->cli_AuthenticationState;
		pwifi_dev[i].wifi_devSignalStrength = pt->cli_SignalStrength;
		pwifi_dev[i].wifi_devTxRate = pt->cli_LastDataDownlinkRate;
		pwifi_dev[i].wifi_devRxRate = pt->cli_LastDataUplinkRate;
	}
	/* the caller is responsible to free the memory */
	*output_struct = pwifi_dev;

	free(associated_dev_array);
	return RETURN_OK;
}

//old one
INT wifi_kickAssociatedDevice(INT apIndex, wifi_device_t *device)
{
	HAL_WIFI_DBG((" %s apIndex %d\n", __FUNCTION__, apIndex));
	char DeviceMacAddress[18] = {'\0'};
	INT iResult, len;

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(device);

	snprintf(DeviceMacAddress, sizeof(DeviceMacAddress), "%02x:%02x:%02x:%02x:%02x:%02x",
                device->wifi_devMacAddress[0], device->wifi_devMacAddress[1],
                device->wifi_devMacAddress[2], device->wifi_devMacAddress[3],
                device->wifi_devMacAddress[4], device->wifi_devMacAddress[5]);
	HAL_WIFI_DBG(("%s: apIndex = %d, DeviceMacAddress = %s\n",
		__FUNCTION__, apIndex, DeviceMacAddress));
	len = strlen(DeviceMacAddress);
	iResult =  wldm_AccessPoint_kickAssociatedDevice(CMD_SET_IOCTL, apIndex,
			DeviceMacAddress, &len, NULL, NULL);
	if (iResult != RETURN_OK) {
		HAL_WIFI_ERR(("%s: wldm_AccessPoint_kickAssociatedDevice CMD_SET_IOCTL Failed, apIndex=[%d]  iResult=[%d]\n",
			__FUNCTION__, apIndex, iResult));
		return RETURN_ERR;
	}
	HAL_WIFI_DBG(("%s: Kick MAC [%s] from [%d] \n", __FUNCTION__, DeviceMacAddress, apIndex));
	return RETURN_OK;
}
//<<

//--------------wifi_ap_hal-----------------------------

//Restore AP paramters to default without change other AP nor Radio parameters (No need to reboot wifi)
INT wifi_factoryResetAP(int apIndex)
{
	BOOL useTemplate = use_nvram_factory_reset_template();
	char cmd[BUF_SIZE];

	AP_INDEX_ASSERT(apIndex);
	HAL_WIFI_LOG(("%s: Start apIndex = %d\n", __FUNCTION__, apIndex));

	/* Supported with wifi 21.2 or later */
	if (useTemplate) {
		snprintf(cmd, sizeof(cmd), "wifi_setup.sh reset_ap_nvram %s", wldm_get_osifname(apIndex));
		if (system(cmd)) {
			HAL_WIFI_ERR(("%s: failed\n", __FUNCTION__));
			return RETURN_ERR;
		}
		return RETURN_OK;
	}
#if defined(_XB7_PRODUCT_REQ_) || defined(_CBR2_PRODUCT_REQ_) || \
	defined(_XB8_PRODUCT_REQ_)
	snprintf(cmd, sizeof(cmd),
		"echo wl%d_ssid=$(awk -F \": \" '/%s:/ {print $2}' %s) > %s",
		apIndex, (apIndex == 0) ? "Default 2.4 GHz SSID" : "Default 5.0 GHz SSID",
		FACTORY_NVRAM_DATA, NVRAM_FACTORY_DEFAULT_TMP);

	if (system(cmd)) {
		HAL_WIFI_ERR(("%s: Default SSID not found\n", __FUNCTION__));
	}
	snprintf(cmd, sizeof(cmd), "echo wl%d_wpa_psk=$(awk -F \": \" '/%s:/ {print $2}' %s) >> %s",
		apIndex, "Default WIFI Password",
		FACTORY_NVRAM_DATA, NVRAM_FACTORY_DEFAULT_TMP);
	if (system(cmd)) {
		HAL_WIFI_ERR(("%s: Default password not found\n", __FUNCTION__));
	}

	snprintf(cmd, sizeof(cmd),
		"echo wl%d_wps_device_pin=$(awk -F \": \" '/%s:/ {print $2}' %s) >> %s",
		apIndex, "Default WPS Pin", FACTORY_NVRAM_DATA, NVRAM_FACTORY_DEFAULT_TMP);
	if (system(cmd)) {
		HAL_WIFI_ERR(("%s: Default WPS Pin not found\n", __FUNCTION__));
	}

	snprintf(cmd, sizeof(cmd), "nvram load -t %s", NVRAM_FACTORY_DEFAULT_TMP);
	if (system(cmd)) {
		HAL_WIFI_ERR(("%s: nvram load failed\n", __FUNCTION__));
	}
	return wldm_xbrcm_factory_reset(WLDM_NVRAM_FACTORY_RESET_AP, apIndex, 1, 1);
#else
	return wldm_xbrcm_factory_reset(WLDM_NVRAM_FACTORY_RESET_AP, apIndex, 0, 0);
#endif /* _XB7_PRODUCT_REQ_ || _CBR2_PRODUCT_REQ_ || _XB8_PRODUCT_REQ_ */
}

//enables CTS protection for the radio used by this AP
INT wifi_setRadioCtsProtectionEnable(INT apIndex, BOOL enable)
{
	INT radioIndex;

	AP_INDEX_ASSERT(apIndex);
	radioIndex = HAL_AP_IDX_TO_HAL_RADIO(apIndex);

	HAL_WIFI_DBG(("%s: radioIndex %d, enable %d\n", __FUNCTION__, radioIndex, enable));

	int len = sizeof(enable);

	if (wldm_xbrcm_Radio_CtsProtectionEnable(CMD_SET, radioIndex, &enable,
		&len, NULL, NULL) < 0) {
		HAL_WIFI_LOG(("%s: wldm_xbrcm_Radio_CtsProtectionEnable CMD SET failed\n", __FUNCTION__));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

// enables OBSS Coexistence - fall back to 20MHz if necessary for the radio used by this ap
INT wifi_setRadioObssCoexistenceEnable(INT apIndex, BOOL enable)
{
	int returnStatus;
	int len = sizeof(enable);

	HAL_WIFI_DBG(("%s: Enter, apIndex %d enable %d\n", __FUNCTION__, apIndex, enable));
	AP_INDEX_ASSERT(apIndex);

	returnStatus = wldm_Radio_ObssCoexistenceEnable(CMD_SET_IOCTL | CMD_SET_NVRAM, apIndex, &enable, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_Radio_ObssCoexistenceEnable Failed, Status = %d\n",
			__FUNCTION__, returnStatus));
		return RETURN_ERR;
	}
	HAL_WIFI_DBG(("%s: apIndex = %d, enable = %d\n", __FUNCTION__, apIndex, enable));
#ifdef RDKB_TIMER
	wifi_set_timer_variable();
#endif
	return RETURN_OK;
}

//P3 // sets the fragmentation threshold in bytes for the radio used by this ap
INT wifi_setRadioFragmentationThreshold(INT apIndex, UINT threshold)
{
	INT radioIndex, returnStatus;
	INT len = sizeof(threshold);

	AP_INDEX_ASSERT(apIndex);
	radioIndex = HAL_AP_IDX_TO_HAL_RADIO(apIndex);

	returnStatus = wldm_Radio_FragmentationThreshold(CMD_SET, radioIndex, &threshold, &len, NULL, NULL);
	if (returnStatus != 0) {
		HAL_WIFI_DBG(("%s: wldm_Radio_FragmentationThreshold CMD_SET failed, Status=%d\n",
			__FUNCTION__, returnStatus));
		return RETURN_ERR;
	}
#ifdef RDKB_TIMER
	wifi_set_timer_variable();
#endif
	return RETURN_OK;
}

/* enable STBC mode in the hardwarwe, 0 == not enabled, 1 == enabled
*  STBC_TX and STBC_RX are not applicable to 6G band
*/
INT wifi_setRadioSTBCEnable(INT radioIndex, BOOL STBC_Enable)
{
	HAL_WIFI_DBG(("%s: radioIndex = %d STBC_Enable = %d\n", __FUNCTION__, radioIndex, STBC_Enable));

	RADIO_INDEX_ASSERT(radioIndex);

	int STBC_int = STBC_Enable ? -1 : 0;
	int len = sizeof(STBC_int);

	if (wldm_xbrcm_Radio_STBCEnable(CMD_SET, radioIndex, &STBC_int, &len, NULL, NULL) < 0) {
		HAL_WIFI_ERR(("%s: wldm_xbrcm_Radio_STBCEnable set %d failed", __FUNCTION__, STBC_Enable));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

// outputs A-MSDU enable status, 0 == not enabled, 1 == enabled
INT wifi_getRadioAMSDUEnable(INT radioIndex, BOOL *output_bool)
{
	int returnStatus;
	int len = sizeof(*output_bool);

	HAL_WIFI_DBG(("%s: Enter, radioIndex = %d\n", __FUNCTION__, radioIndex));
	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(output_bool);

	*output_bool = FALSE;

	returnStatus = wldm_Radio_AMSDUEnable(CMD_GET, radioIndex, output_bool, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_Radio_AMSDUEnable CMD_GET Failed, Status = %d\n",
			__FUNCTION__, returnStatus));
		*output_bool = FALSE;
		return RETURN_ERR;
	}
	HAL_WIFI_DBG(("%s: radioIndex = %d, output_bool = %d\n", __FUNCTION__, radioIndex, *output_bool));
	return RETURN_OK;
}

// enables A-MSDU in the hardware, 0 == not enabled, 1 == enabled
INT wifi_setRadioAMSDUEnable(INT radioIndex, BOOL amsduEnable)
{
	int returnStatus;
	int len = sizeof(amsduEnable);

	HAL_WIFI_DBG(("%s: Enter, radioIndex = %d, amsduEnable = %d\n", __FUNCTION__, radioIndex, amsduEnable));
	RADIO_INDEX_ASSERT(radioIndex);

	returnStatus = wldm_Radio_AMSDUEnable(CMD_SET, radioIndex, &amsduEnable, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_Radio_AMSDUEnable CMD_SET Failed, Status = %d\n",
			__FUNCTION__, returnStatus));
		return RETURN_ERR;
	}
#ifdef RDKB_TIMER
	wifi_set_timer_variable();
#endif
	HAL_WIFI_DBG(("%s: radioIndex = %d, amsduEnable = %d\n", __FUNCTION__, radioIndex, amsduEnable));

	return RETURN_OK;
}

//P2  // outputs the number of Tx streams
INT wifi_getRadioTxChainMask(INT radioIndex, INT *output_int)
{
	HAL_WIFI_DBG(("%s: radioIndex %d\n", __FUNCTION__, radioIndex));

	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(output_int);

	int len = sizeof(int);
	int txchain_num;

	if (wldm_xbrcm_Radio_TxChainMask(CMD_GET, radioIndex, &txchain_num, &len, NULL, NULL) < 0) {
		HAL_WIFI_LOG(("%s: wldm_xbrcm_Radio_TxChainMask get failed\n", __FUNCTION__));
		return RETURN_ERR;
	}

	*output_int = txchain_num;
	return RETURN_OK;
}

//P2  // sets the number of Tx streams to an enviornment variable
INT wifi_setRadioTxChainMask(INT radioIndex, INT numStreams)
{
	//save to wifi config, wait for wifi reset or wifi_pushTxChainMask to apply
	HAL_WIFI_DBG(("%s: radioIndex %d numStreams %d\n", __FUNCTION__, radioIndex, numStreams));

	RADIO_INDEX_ASSERT(radioIndex);

	int len = sizeof(numStreams);

	if (wldm_xbrcm_Radio_TxChainMask(CMD_SET, radioIndex, &numStreams, &len, NULL, NULL) < 0) {
		HAL_WIFI_LOG(("%s: wldm_xbrcm_Radio_TxChainMask set failed\n", __FUNCTION__));
		return RETURN_ERR;
	}
	return RETURN_OK;
}

//P2  // outputs the number of Rx streams
INT wifi_getRadioRxChainMask(INT radioIndex, INT *output_int)
{
	HAL_WIFI_DBG(("%s: radioIndex %d\n", __FUNCTION__, radioIndex));

	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(output_int);

	int len = sizeof(int);
	int num;

	if (wldm_xbrcm_Radio_RxChainMask(CMD_GET, radioIndex, &num, &len, NULL, NULL) < 0) {
		HAL_WIFI_LOG(("%s: wldm_xbrcm_Radio_RxChainMask get failed\n", __FUNCTION__));
		return RETURN_ERR;
	}

	*output_int = num;
	return RETURN_OK;
}

//P2  // sets the number of Rx streams to an enviornment variable
INT wifi_setRadioRxChainMask(INT radioIndex, INT numStreams)
{
	HAL_WIFI_DBG(("%s: radioIndex %d, numStreams %d\n", __FUNCTION__, radioIndex, numStreams));

	RADIO_INDEX_ASSERT(radioIndex);

	int len = sizeof(numStreams);

	if (wldm_xbrcm_Radio_RxChainMask(CMD_SET, radioIndex, &numStreams, &len, NULL, NULL) < 0) {
		HAL_WIFI_LOG(("%s: wldm_xbrcm_Radio_RxChainMask set failed\n", __FUNCTION__));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

//Get radio RDG enable setting
INT wifi_getRadioReverseDirectionGrantSupported(INT radioIndex, BOOL *output_bool)
{
	HAL_WIFI_DBG(("%s: radioIndex %d\n", __FUNCTION__, radioIndex));

	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(output_bool);
	*output_bool = TRUE;

	return RETURN_OK;
}

//Get radio RDG enable setting
INT wifi_getRadioReverseDirectionGrantEnable(INT radioIndex, BOOL *output_bool)
{
	HAL_WIFI_DBG(("%s: radioIndex %d\n", __FUNCTION__, radioIndex));

	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(output_bool);
	*output_bool = TRUE;

	return RETURN_OK;
}

//Set radio RDG enable setting
INT wifi_setRadioReverseDirectionGrantEnable(INT radioIndex, BOOL enable)
{
	HAL_WIFI_DBG(("%s: radioIndex:%d enable:%d\n", __FUNCTION__, radioIndex, enable));

	RADIO_INDEX_ASSERT(radioIndex);

	return RETURN_OK;
}

//Get radio ADDBA enable setting
INT wifi_getRadioDeclineBARequestEnable(INT radioIndex, BOOL *output_bool)
{
	HAL_WIFI_DBG(("%s: radioIndex %d\n", __FUNCTION__, radioIndex));

	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(output_bool);
	*output_bool = TRUE;

	return RETURN_OK;
}

//Set radio ADDBA enable setting
INT wifi_setRadioDeclineBARequestEnable(INT radioIndex, BOOL enable)
{
	HAL_WIFI_DBG(("%s: radioIndex:%d enable:%d\n", __FUNCTION__, radioIndex, enable));

	RADIO_INDEX_ASSERT(radioIndex);

	return RETURN_OK;
}

//Get radio auto block ack enable setting
INT wifi_getRadioAutoBlockAckEnable(INT radioIndex, BOOL *output_bool)
{
	HAL_WIFI_DBG(("%s: radioIndex %d\n", __FUNCTION__, radioIndex));

	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(output_bool);
	*output_bool = TRUE;

	return RETURN_OK;

}

//Set radio auto block ack enable setting
INT wifi_setRadioAutoBlockAckEnable(INT radioIndex, BOOL enable)
{
	HAL_WIFI_DBG(("%s: radioIndex:%d enable:%d\n", __FUNCTION__, radioIndex, enable));

	RADIO_INDEX_ASSERT(radioIndex);

	return RETURN_OK;
}

//Get radio 11n pure mode enable support
INT wifi_getRadio11nGreenfieldSupported(INT radioIndex, BOOL *output_bool)
{
	HAL_WIFI_DBG(("%s: radioIndex %d\n", __FUNCTION__, radioIndex));

	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(output_bool);

	int len;
	BOOL enable = 0;

	if (wldm_xbrcm_Radio_Greenfield11nSupported(CMD_GET, radioIndex, &enable,
		&len, NULL, NULL ) < 0) {
		HAL_WIFI_ERR(("%s: wldm_xbrcm_Radio_11nGreenfieldSupported failed\n", __FUNCTION__));
		return RETURN_ERR;
	}

	*output_bool = enable ? TRUE: FALSE;
	return RETURN_OK;
}

//Get radio 11n pure mode enable setting
INT wifi_getRadio11nGreenfieldEnable(INT radioIndex, BOOL *output_bool)
{
	HAL_WIFI_DBG(("%s: radioIndex %d\n", __FUNCTION__, radioIndex));

	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(output_bool);

	int len;
	BOOL enable;

	if (wldm_xbrcm_Radio_Greenfield11nEnable(CMD_GET_NVRAM, radioIndex, &enable,
		&len, NULL, NULL) < 0) {
		HAL_WIFI_ERR(("%s: wldm_xbrcm_Radio_Greenfield11nEnable", __FUNCTION__));
		return RETURN_ERR;
	}

	*output_bool = enable;
	return RETURN_OK;
}

//Set radio 11n pure mode enable setting
INT wifi_setRadio11nGreenfieldEnable(INT radioIndex, BOOL enable)
{
	HAL_WIFI_DBG(("%s: radioIndex %d enable %d\n", __FUNCTION__, radioIndex, enable));

	RADIO_INDEX_ASSERT(radioIndex);

	int len = sizeof (enable);

	if (wldm_xbrcm_Radio_Greenfield11nEnable(CMD_SET, radioIndex, &enable, &len, NULL, NULL) < 0) {
		HAL_WIFI_ERR(("%s: wldm_Radio_Greenfield11nEnable", __FUNCTION__));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

//Get radio IGMP snooping enable setting
INT wifi_getRadioIGMPSnoopingEnable(INT radioIndex, BOOL *output_bool)
{
	HAL_WIFI_DBG(("%s: radioIndex %d\n", __FUNCTION__, radioIndex));

	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(output_bool);
	*output_bool = TRUE;

	return RETURN_OK;
}

//Set radio IGMP snooping enable setting
INT wifi_setRadioIGMPSnoopingEnable(INT radioIndex, BOOL enable)
{
	/* Not Applicable */
	HAL_WIFI_DBG(("%s: radioIndex:%d enable:%d\n", __FUNCTION__, radioIndex, enable));

	RADIO_INDEX_ASSERT(radioIndex);

	return RETURN_OK;
}

//---------------------------------------------------------------------------------------------------
//
// Additional Wifi AP level APIs used for Access Point devices
//
//---------------------------------------------------------------------------------------------------

// creates a new ap and pushes these parameters to the hardware
INT wifi_createAp(INT apIndex, INT radioIndex, CHAR *essid, BOOL hideSsid)
{
	INT returnStatus, len;
	BOOL enableSsid;
	HAL_WIFI_DBG(("%s, ENTER apIndex %d radioIndex %d essid %s hideSsid %d\n",
		__FUNCTION__, apIndex, radioIndex, essid, hideSsid));

	RADIO_INDEX_ASSERT(radioIndex);
	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(essid);

	if (HAL_AP_IDX_TO_HAL_RADIO(apIndex) != radioIndex) {
		HAL_WIFI_DBG(("Given apIndex %d not found in the radio %d ! \n", apIndex, radioIndex));
		return RETURN_ERR;
	}

	len = strlen(essid);
	returnStatus = wldm_AccessPoint(CMD_ADD | CMD_SET_NVRAM, apIndex, essid, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: Fail, returnStatus = %d\n", __FUNCTION__, returnStatus));
		return RETURN_ERR;
	}

	enableSsid = !hideSsid;
	len = sizeof(enableSsid);
	returnStatus = wldm_AccessPoint_SSIDAdvertisementEnabled(CMD_SET_NVRAM | CMD_SET_IOCTL,
				apIndex, &enableSsid, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: Fail, returnStatus = %d\n", __FUNCTION__, returnStatus));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s, DONE apIndex %d radioIndex %d essid %s hideSsid %d\n",
		__FUNCTION__, apIndex, radioIndex, essid, hideSsid));

#ifdef RDKB_TIMER
	wifi_set_timer_variable();
#endif
	return RETURN_OK;
}

/* Deletes this AP entry on the hardware, clears all internal variables associated with this AP */
INT wifi_deleteAp(INT apIndex)
{
	HAL_WIFI_DBG(("%s: ApIndex = %d\n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);

	if (wldm_AccessPoint(CMD_DEL, apIndex, NULL, NULL, NULL, NULL) != 0) {
		HAL_WIFI_DBG(("%s:%d wldm_AccessPoint Delete Failed\n", __FUNCTION__, __LINE__));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

// Outputs a 16 byte or less name assocated with the AP.  String buffer must be pre-allocated by the caller
INT wifi_getApName(INT apIndex, CHAR *output_string)
{
	NULL_PTR_ASSERT(output_string);
	AP_INDEX_ASSERT(apIndex);

	snprintf(output_string, OUTPUT_STRING_LENGTH_16, "%s", wldm_get_nvifname(apIndex));
	HAL_WIFI_DBG(("%s: apIndex = %d, output_string = %s\n",
		__FUNCTION__, apIndex, output_string));
	return RETURN_OK;
}

static const char *ignore_ifname_list[] = {"ath"};

static BOOL isValidSsidName(char *ifName)
{
	int i, len = sizeof(ignore_ifname_list) / sizeof(ignore_ifname_list[0]);

	for (i = 0; i < len; i++) {
		if (strncmp(ifName, ignore_ifname_list[i], strlen(ignore_ifname_list[i])) == 0) {
			return FALSE;
		}
	}
	return TRUE;
}

// Outputs the index number in that corresponds to the SSID string
INT wifi_getIndexFromName(CHAR *inputSsidString, INT *output_int)
{
	INT apIndex;

	NULL_PTR_ASSERT(output_int);
	NULL_PTR_ASSERT(inputSsidString);

	/* "SSID string" is not SSID, it is the wlan interface name actually */
	char *inputName = inputSsidString;

	if (!isValidSsidName(inputName)) {
		HAL_WIFI_ERR(("%s, Skip invalid interface %s\n", __FUNCTION__, inputName));
		return RETURN_ERR;
	}

	if ((apIndex = wldm_get_apindex(inputName)) < 0) {
		HAL_WIFI_ERR(("%s, failed as ifname %s\n", __FUNCTION__, inputSsidString));
		return RETURN_ERR;
	}

	*output_int = apIndex;
	HAL_WIFI_DBG(("%s: name=%s, index=%d\n", __FUNCTION__, inputSsidString, apIndex));
	return RETURN_OK;
}

// Outputs a 32 byte or less string indicating the beacon type as "None", "Basic", "WPA", "11i", "WPAand11i"
INT wifi_getApBeaconType(INT apIndex, CHAR *output_string)
{
	CHAR auth_mode[OUTPUT_STRING_LENGTH_32] = {0};
	int len = sizeof(auth_mode);

	HAL_WIFI_LOG(("%s: apIndex = %d\n", __FUNCTION__, apIndex));

	NULL_PTR_ASSERT(output_string);
	AP_INDEX_ASSERT(apIndex);

	/* TODO: Just return beaconType[apIndex] directly */
	if (wldm_AccessPoint_Security_ModeEnabled(CMD_GET_NVRAM, apIndex, auth_mode, &len, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: Unable to get the security mode\n", __FUNCTION__));
		return RETURN_ERR;
	}

	if ((strcmp("WPA-WPA2-Enterprise", auth_mode) == 0) ||
		(strcmp("WPA-WPA2-Personal", auth_mode) == 0)) {
		beaconType[apIndex] = BEACON_TYPE_WPA_11I;
	}
	else if ((strcmp("WPA2-Personal", auth_mode) == 0) ||
		(strcmp("WPA2-Enterprise", auth_mode) == 0)) {
		beaconType[apIndex] = BEACON_TYPE_11I;
	}
	else if ((strcmp("WPA-Personal", auth_mode) == 0) ||
		(strcmp("WPA-Enterprise", auth_mode) == 0)) {
		beaconType[apIndex] = BEACON_TYPE_WPA;
	}
	else {
		beaconType[apIndex] = BEACON_TYPE_NONE;
	}

	snprintf(output_string, OUTPUT_STRING_LENGTH_32, "%s", beaconTypeStr[beaconType[apIndex]]);
	HAL_WIFI_DBG(("%s: output_string = %s \n", __FUNCTION__, beaconTypeStr[beaconType[apIndex]]));
	return RETURN_OK;
}

// Sets the beacon type enviornment variable. Allowed input strings are "None", "Basic", "WPA, "11i", "WPAand11i"
INT wifi_setApBeaconType(INT apIndex, CHAR *beaconTypeString)
{
	BEACON_TYPE type;

	NULL_PTR_ASSERT(beaconTypeString);
	AP_INDEX_ASSERT(apIndex);

	HAL_WIFI_DBG(("%s: apIndex = %d beaconTypeString = %s\n", __FUNCTION__, apIndex, beaconTypeString));

	//save the beaconTypeString to wifi config and hostapd config file. Wait for wifi reset or hostapd restart to apply
	for (type = BEACON_TYPE_NONE; type < BEACON_TYPE_TOTAL; type++) {
		if (strcmp(beaconTypeString, beaconTypeStr[type]) == 0) {
			beaconType[apIndex] = type;
			break;
		}
	}
	return RETURN_OK;
}

// sets the beacon interval on the hardware for this AP
INT wifi_setApBeaconInterval(INT apIndex, INT beaconInterval)
{
	/* save config and apply instantly */
	INT radioIndex = 0;
	int returnStatus;
	int len = sizeof(beaconInterval);

	AP_INDEX_ASSERT(apIndex);
	radioIndex = HAL_AP_IDX_TO_HAL_RADIO(apIndex);

	HAL_WIFI_DBG(("%s: Enter, radioIndex = %d, beaconInterval = %d\n",
		__FUNCTION__, radioIndex, beaconInterval));
	returnStatus = wldm_Radio_BeaconPeriod(CMD_SET, radioIndex, &beaconInterval, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_Radio_BeaconPeriod CMD_SET Fail, Status = %d\n",
			__FUNCTION__, returnStatus));
		return RETURN_ERR;
	}
#ifdef RDKB_TIMER
	wifi_set_timer_variable();
#endif
	HAL_WIFI_DBG(("%s: Done , radioIndex = %d, beaconInterval = %d\n",
		__FUNCTION__, radioIndex, beaconInterval));

	return RETURN_OK;
}

INT wifi_setApDTIMInterval(INT apIndex, INT dtimInterval)
{
	int radioIndex,	returnStatus;
	int len = sizeof(dtimInterval);

	AP_INDEX_ASSERT(apIndex);
	radioIndex = HAL_AP_IDX_TO_HAL_RADIO(apIndex);

	HAL_WIFI_DBG(("%s: Enter, apIndex %d\n", __FUNCTION__, apIndex));

	if ((0 >= dtimInterval) || (255 < dtimInterval)) {
		HAL_WIFI_DBG(("%s: Fail, DTIM interval [%d] is out of range\n",
			__FUNCTION__, dtimInterval));
		return RETURN_ERR;
	}

	returnStatus = wldm_Radio_DTIMPeriod(CMD_SET, radioIndex, &dtimInterval, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_Radio_DTIMPeriod CMD_SET Failed, Status %d\n",
			__FUNCTION__, returnStatus));
		return RETURN_ERR;
	}
#ifdef RDKB_TIMER
	wifi_set_timer_variable();
#endif
	HAL_WIFI_DBG(("%s: Completed for  apIndex %d\n", __FUNCTION__, apIndex));

	return RETURN_OK;
}

// Get the packet size threshold supported.
INT wifi_getApRtsThresholdSupported(INT apIndex, BOOL *output_bool) // Get the packet size threshold supported.
{
	int returnStatus = RETURN_ERR;
	UINT32 threshold;
	INT radioIndex;
	int len = sizeof(threshold);

	HAL_WIFI_DBG(("%s: Enter, ap_index = %d\n", __FUNCTION__, apIndex));

	NULL_PTR_ASSERT(output_bool);
	AP_INDEX_ASSERT(apIndex);

	radioIndex = HAL_AP_IDX_TO_HAL_RADIO(apIndex);

	returnStatus = wldm_Radio_RTSThreshold(CMD_GET, radioIndex, &threshold, &len, NULL, NULL);
	if (returnStatus != 0) {
		*output_bool = FALSE;
		HAL_WIFI_DBG(("%s: wldm_Radio_RTSThreshold CMD_GET failed, Status = %d\n",
			__FUNCTION__, returnStatus));
		return RETURN_ERR;
	}
	*output_bool = TRUE;
	return RETURN_OK;
}

// sets the packet size threshold in bytes to apply RTS/CTS backoff rules.
INT wifi_setApRtsThreshold(INT apIndex, UINT threshold)
{
	/* Q: Is this for RADIO or AccessPoint? TR181 has this as a radio property Device.WiFi.Radio.{i}.RTSThreshold */
	INT radioIndex, returnStatus;
	int len = sizeof(threshold);

	AP_INDEX_ASSERT(apIndex);
	radioIndex = HAL_AP_IDX_TO_HAL_RADIO(apIndex);

	returnStatus = wldm_Radio_RTSThreshold(CMD_SET, radioIndex, &threshold, &len, NULL, NULL);
	if (returnStatus != 0) {
		HAL_WIFI_DBG(("%s: wldm_Radio_RTSThreshold CMD_SET Failed, Status=%d\n",
			__FUNCTION__, returnStatus));
		return RETURN_ERR;
	}
	return RETURN_OK;
}

// ouputs up to a 32 byte string as either "TKIPEncryption", "AESEncryption", or "TKIPandAESEncryption"
INT wifi_getApWpaEncryptionMode(INT apIndex, CHAR *output_string)
{
	int returnStatus;
	int len = OUTPUT_STRING_LENGTH_32+1;

	HAL_WIFI_DBG(("%s: Enter, apIndex %d\n", __FUNCTION__, apIndex));

	NULL_PTR_ASSERT(output_string);
	AP_INDEX_ASSERT(apIndex);

	output_string[0] = '\0';
	returnStatus = wldm_AccessPoint_Wpa_Encryptionmode(CMD_GET, apIndex, output_string, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_AccessPoint_Wpa_Encryptionmode CMD_GET Failed, Status = %d\n",
			__FUNCTION__, returnStatus));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: apIndex = %d, encryption = %s\n", __FUNCTION__, apIndex, output_string));
	return RETURN_OK;
}

// sets the encyption mode enviornment variable.  Valid string format is "TKIPEncryption", "AESEncryption", or "TKIPandAESEncryption"
INT wifi_setApWpaEncryptionMode(INT apIndex, CHAR *encMode)
{
	int returnStatus;
	int len = OUTPUT_STRING_LENGTH_32+1;

	NULL_PTR_ASSERT(encMode);
	AP_INDEX_ASSERT(apIndex);

	HAL_WIFI_LOG(("%s: apIndex = %d, encMode = %s\n", __FUNCTION__, apIndex, encMode));

	returnStatus = wldm_AccessPoint_Wpa_Encryptionmode(CMD_SET, apIndex, encMode, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_AccessPoint_Wpa_Encryptionmode CMD_SET Failed, Status = %d\n",
			__FUNCTION__, returnStatus));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: END, apIndex %d\n", __FUNCTION__, apIndex));
#ifdef RDKB_TIMER
	wifi_set_timer_variable();
#endif
	return RETURN_OK;
}

// deletes internal security varable settings for this ap
INT wifi_removeApSecVaribles(INT apIndex)
{
	UNUSED_PARAMETER(apIndex);
	/* CBR2-661 - This API should reset any internal encryption keys */
	HAL_WIFI_DBG(("%s: return as no internal encryption keys\n", __FUNCTION__));
	return RETURN_OK;
}

// changes the hardware settings to disable encryption on this ap
INT wifi_disableApEncryption(INT apIndex)
{
	HAL_WIFI_LOG(("%s: %d: ####dummy####\n", __FUNCTION__, apIndex));

	return RETURN_OK;
}

// set the authorization mode on this ap
// mode mapping as: 1: open, 2: shared, 4:auto
INT wifi_setApAuthMode(INT apIndex, INT mode)
{
	int returnStatus;
	int len = OUTPUT_STRING_LENGTH_32+1;

	HAL_WIFI_DBG(("%s: Enter, apIndex %d\n", __FUNCTION__, apIndex));
	AP_INDEX_ASSERT(apIndex);

	returnStatus = wldm_AccessPoint_Security_AuthMode(CMD_SET, apIndex, &mode, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_AccessPoint_Security_AuthMode CMD_SET Failed, Status = %d\n",
			__FUNCTION__, returnStatus));
		return RETURN_ERR;
	}
	HAL_WIFI_DBG(("%s: END, apIndex %d, mode %d\n", __FUNCTION__, apIndex, mode));
#ifdef RDKB_TIMER
	wifi_set_timer_variable();
#endif

	return RETURN_OK;
}

// get the authentication mode on this ap
INT wifi_getApBasicAuthenticationMode(INT apIndex, CHAR *authMode)
{
	int returnStatus, len = OUTPUT_STRING_LENGTH_32+1;

	NULL_PTR_ASSERT(authMode);
	AP_INDEX_ASSERT(apIndex);

	HAL_WIFI_DBG(("%s: Enter, apIndex %d\n", __FUNCTION__, apIndex));
	*authMode = '\0';
	returnStatus = wldm_AccessPoint_Basic_Authenticationmode(CMD_GET, apIndex, authMode, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_AccessPoint_Basic_Authenticationmode CMD_GET Failed, Status = %d\n",
			__FUNCTION__, returnStatus));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: apIndex = %d, authMode = %s\n", __FUNCTION__, apIndex, authMode));
	return RETURN_OK;
}
// sets an enviornment variable for the authMode. Valid strings are "None", "EAPAuthentication" or "SharedAuthentication"
// Assume upper layer call this API together with wifi_setApBeaconType. We need these two API mapping to WiFiAccessPointSecurityModeEnabledSet

// #define WIFIACCESSPOINTSECURITY_MODEENABLED_OPEN "None"
// wifi_setApBeaconType "None"		wifi_setApBasicAuthenticationMode("None")
// #define WIFIACCESSPOINTSECURITY_MODEENABLED_WEP_64 "WEP-64"
// wifi_setApBeaconType RDKB do not support WEP
// #define WIFIACCESSPOINTSECURITY_MODEENABLED_WEP_128 "WEP-128"
// wifi_setApBeaconType RDKB do not support WEP
// #define WIFIACCESSPOINTSECURITY_MODEENABLED_WPA_Personal "WPA-Personal"
// wifi_setApBeaconType "WPA"		wifi_setApBasicAuthenticationMode("PSKAuthentication")
// #define WIFIACCESSPOINTSECURITY_MODEENABLED_WPA2_Personal "WPA2-Personal"
// wifi_setApBeaconType "11i"		wifi_setApBasicAuthenticationMode("PSKAuthentication ")
// #define WIFIACCESSPOINTSECURITY_MODEENABLED_WPA_WPA2_Personal "WPA-WPA2-Personal"
// wifi_setApBeaconType "WPAand11i"	wifi_setApBasicAuthenticationMode("PSKAuthentication ")
// #define WIFIACCESSPOINTSECURITY_MODEENABLED_WPA_Enterprise "WPA-Enterprise"
// wifi_setApBeaconType "WPA"		wifi_setApBasicAuthenticationMode("EAPAuthentication")
// #define WIFIACCESSPOINTSECURITY_MODEENABLED_WPA2_Enterprise "WPA2-Enterprise"
// wifi_setApBeaconType "11i"		wifi_setApBasicAuthenticationMode("EAPAuthentication")
// #define WIFIACCESSPOINTSECURITY_MODEENABLED_WPA_WPA2_Enterprise "WPA-WPA2-Enterprise"
// wifi_setApBeaconType "WPAand11i"	wifi_setApBasicAuthenticationMode("EAPAuthentication")
INT wifi_setApBasicAuthenticationMode(INT apIndex, CHAR *authMode)
{
	int returnStatus;
	BASIC_AUTHENTICATION_MODE mode;
	CHAR encMode[WIFIACCESSPOINTSECURITY_MODEENABLED_MAX];
	int len = OUTPUT_STRING_LENGTH_32+1;

	HAL_WIFI_DBG(("%s: Enter, apIndex %d\n", __FUNCTION__, apIndex));

	NULL_PTR_ASSERT(authMode);
	AP_INDEX_ASSERT(apIndex);

	for (mode = BASIC_AUTHENTICATION_MODE_NONE; mode < BASIC_AUTHENTICATION_MODE_TOTAL; mode++) {
		if (strcmp(authMode, basicAuthenticationModeStr[mode]) == 0) {
			basicAuthenticationMode[apIndex] = mode;
			break;
		}
	}
	if (mode >= BASIC_AUTHENTICATION_MODE_TOTAL) {
		HAL_WIFI_DBG(("%s: Fail, authMode parameter error \n", __FUNCTION__));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: apIndex = %d, authMode = %s, beaconType = %d\n", __FUNCTION__, apIndex, authMode, beaconType[apIndex]));

	switch(mode) {
		case BASIC_AUTHENTICATION_MODE_NONE:
		{
			snprintf(encMode, sizeof(encMode), "%s", "None");
			break;
		}
		case BASIC_AUTHENTICATION_MODE_EAP:
		{
			if((beaconType[apIndex]) == BEACON_TYPE_WPA) {
				snprintf(encMode, sizeof(encMode), "%s", "WPA-Enterprise");
			}
			else if((beaconType[apIndex]) == BEACON_TYPE_11I) {
				snprintf(encMode, sizeof(encMode), "%s", "WPA2-Enterprise");
			}  else {
				snprintf(encMode, sizeof(encMode), "%s", "WPA-WPA2-Enterprise");
			}
			break;
		}
		case BASIC_AUTHENTICATION_MODE_SHARED:
		{
			HAL_WIFI_ERR(("%s: doesn't support this mode: %d!!! treat as MODE_EAP\n", __FUNCTION__, mode));
			return RETURN_ERR;
		}
		case BASIC_AUTHENTICATION_MODE_PSK:
		{
			if((beaconType[apIndex]) == BEACON_TYPE_WPA) {
				snprintf(encMode, sizeof(encMode), "%s", "WPA-Personal");
			} else if((beaconType[apIndex]) == BEACON_TYPE_11I) {
				snprintf(encMode, sizeof(encMode), "%s", "WPA2-Personal");
			} else {
				snprintf(encMode, sizeof(encMode), "%s", "WPA-WPA2-Personal");
			}
			break;
		}
		default :
		{
			HAL_WIFI_ERR(("%s: Fail, mode parameter error = %d\n", __FUNCTION__, mode));
			return RETURN_ERR;
		}
	}

	HAL_WIFI_DBG(("%s: encMode = %s\n", __FUNCTION__, encMode));
	returnStatus = wldm_AccessPoint_Basic_Authenticationmode(CMD_SET, apIndex, encMode, &len, NULL, NULL);

	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: Fail, returnStatus = %d\n", __FUNCTION__, returnStatus));
		return RETURN_ERR;
	}
	HAL_WIFI_DBG(("%s: apIndex = %d, set  = %s\n", __FUNCTION__, apIndex, authMode));
#ifdef RDKB_TIMER
	wifi_set_timer_variable();
#endif
	return RETURN_OK;
}

// Outputs the number of stations associated per AP
INT wifi_getApNumDevicesAssociated(INT apIndex, ULONG *output_ulong)
{
	int returnStatus, numDevices = 0;
	int len = sizeof(numDevices);

	HAL_WIFI_DBG(("%s: Enter, apIndex=[%d]\n", __FUNCTION__, apIndex));
	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(output_ulong);

	*output_ulong = 0;

	returnStatus = wldm_AccessPoint_AssociatedDeviceNumber(CMD_GET, apIndex, &numDevices, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_AccessPoint_AssociatedDeviceNumber CMD_GET Failed, apIndex=[%d], Status=[%d]\n",
		__FUNCTION__, apIndex, returnStatus));
		return RETURN_ERR;
	}
	*output_ulong = (ULONG) numDevices;
	HAL_WIFI_DBG(("%s: apIndex=[%d] output_ulong=[%d]\n", __FUNCTION__, apIndex, *output_ulong));

	return RETURN_OK;
}

/* Manually removes any active wi-fi association with the device specified on this access point */
#ifdef WIFI_HAL_VERSION_3_PHASE2
INT wifi_kickApAssociatedDevice(INT apIndex, mac_address_t client_mac)
{
	char mac_str[MAC_STR_LEN];
	INT iResult, len;

	AP_INDEX_ASSERT(apIndex);

	snprintf(mac_str, sizeof(mac_str), MACF, MAC_TO_MACF(client_mac));

	len = strlen(mac_str);
	HAL_WIFI_DBG(("%s: apIndex = %d, client_mac = %s\n", __FUNCTION__, apIndex, mac_str));

	iResult =  wldm_AccessPoint_kickAssociatedDevice(CMD_SET_IOCTL, apIndex, mac_str, &len, NULL, NULL);
	if (iResult != RETURN_OK) {
		HAL_WIFI_ERR(("%s: wldm_AccessPoint_kickAssociatedDevice CMD_SET_IOCTL Fail, apIndex=[%d], iResult=[%d]\n",
			__FUNCTION__, apIndex, iResult));
		return WIFI_HAL_ERROR;
	}
	HAL_WIFI_DBG(("%s: Kick MAC [%s] from [%d] \n", __FUNCTION__, mac_str, apIndex));

	return WIFI_HAL_SUCCESS;
}
#else /* WIFI_HAL_VERSION_3_PHASE2 */
INT wifi_kickApAssociatedDevice(INT apIndex, CHAR *client_mac)
{
	INT iResult, len;

	NULL_PTR_ASSERT(client_mac);
	AP_INDEX_ASSERT(apIndex);

	len = strlen(client_mac);
	HAL_WIFI_DBG(("%s: apIndex = %d, client_mac = %s\n", __FUNCTION__, apIndex, client_mac));

	iResult =  wldm_AccessPoint_kickAssociatedDevice(CMD_SET_IOCTL, apIndex, client_mac, &len, NULL, NULL);
	if (iResult != RETURN_OK) {
		HAL_WIFI_ERR(("%s: wldm_AccessPoint_kickAssociatedDevice CMD_SET_IOCTL Fail, apIndex=[%d], iResult=[%d]\n",
			__FUNCTION__, apIndex, iResult));
		return RETURN_ERR;
	}
	HAL_WIFI_DBG(("%s: Kick MAC [%s] from [%d] \n", __FUNCTION__, client_mac, apIndex));

	return RETURN_OK;
}
#endif /* WIFI_HAL_VERSION_3_PHASE2 */

// outputs the radio index for the specified ap. similar as wifi_getSsidRadioIndex
INT wifi_getApRadioIndex(INT apIndex, INT *output_int)
{
	NULL_PTR_ASSERT(output_int);

	*output_int = HAL_AP_IDX_TO_HAL_RADIO(apIndex);

	HAL_WIFI_DBG(("%s, apIndex = %d, output_int = %d\n", __FUNCTION__, apIndex, *output_int));

	return RETURN_OK;
}

// sets the radio index for the specific ap
INT wifi_setApRadioIndex(INT apIndex, INT radioIndex)
{
	UNUSED_PARAMETER(apIndex);
	UNUSED_PARAMETER(radioIndex);
	HAL_WIFI_DBG(("%s: Use wifi_createAp(), radioIndex and apIndex are fixed\n", __FUNCTION__));
	return RETURN_OK;
}

/* Get the ACL MAC list per AP */
#ifdef WIFI_HAL_VERSION_3_PHASE2
INT wifi_getApAclDevices(INT apIndex, mac_address_t *macArray, UINT maxArraySize, UINT * output_numEntries)
{
	char *mac_str_array = NULL, *mac_str = NULL, *rest = NULL;
	int returnStatus, len, i;

	HAL_WIFI_DBG(("%s: Enter, apIndex=[%d]\n", __FUNCTION__, apIndex));

	NULL_PTR_ASSERT(macArray);
	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(output_numEntries);

	len = maxArraySize * MAC_STR_LEN;
	mac_str_array = (char *)malloc(len);
	NULL_PTR_ASSERT(mac_str_array);
	memset(mac_str_array, 0, len);

	returnStatus = wldm_AccessPoint_AclDevices(CMD_GET, apIndex, mac_str_array, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_AccessPoint_AclDevices CMD_GET Failed, apIndex=[%d], Status=[%d]\n",
			__FUNCTION__, apIndex, returnStatus));
		free(mac_str_array);
		return WIFI_HAL_ERROR;
	}
	mac_str = strtok_r(mac_str_array, "\n", &rest);
	for (i = 0; (i < maxArraySize) && (mac_str != NULL); ++i) {
		sscanf(mac_str, MACF, MAC_TO_MACF(&macArray[i]));
		HAL_WIFI_DBG((MACF"\n", MAC_TO_MACF(macArray[i])));
		mac_str = strtok_r(NULL, "\n", &rest);
	}
	*output_numEntries = i;

	free(mac_str_array);
	return WIFI_HAL_SUCCESS;
}
#else /* WIFI_HAL_VERSION_3_PHASE2 */
INT wifi_getApAclDevices(INT apIndex, CHAR *macArray, UINT buf_size)
{
	int returnStatus;
	int len = buf_size;
	HAL_WIFI_DBG(("%s: Enter, apIndex=[%d]\n", __FUNCTION__, apIndex));

	NULL_PTR_ASSERT(macArray);
	AP_INDEX_ASSERT(apIndex);

	memset(macArray, 0, buf_size);
	returnStatus = wldm_AccessPoint_AclDevices(CMD_GET, apIndex, macArray, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_AccessPoint_AclDevices CMD_GET Failed, apIndex=[%d], Status=[%d]\n",
			__FUNCTION__, apIndex, returnStatus));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: apIndex=[%d] macArray=[%s]\n", __FUNCTION__, apIndex, macArray));
	return RETURN_OK;
}
#endif /* WIFI_HAL_VERSION_3_PHASE2 */

// Get the list of stations associated per AP
INT wifi_getApDevicesAssociated(INT apIndex, CHAR *macArray, UINT buf_size)
{
	UNUSED_PARAMETER(apIndex);
	UNUSED_PARAMETER(macArray);
	UNUSED_PARAMETER(buf_size);
	return RETURN_OK;
}

/* adds the mac address to the filter list */
#ifdef WIFI_HAL_VERSION_3_PHASE2
INT wifi_addApAclDevice(INT apIndex, mac_address_t DeviceMacAddress)
{
	char mac_str[MAC_STR_LEN];
	int len = sizeof(mac_str), returnStatus;
	HAL_WIFI_DBG(("%s: apIndex = %d\n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);

	snprintf(mac_str, len, MACF, MAC_TO_MACF(DeviceMacAddress));
	returnStatus = wldm_AccessPoint_AclDevice(CMD_ADD, apIndex, mac_str, &len, NULL, NULL);
	if (returnStatus != RETURN_OK){
		HAL_WIFI_DBG(("%s: wldm_AccessPoint_AclDevice CMD_ADD Failed, apIndex=[%d], Status=[%d]\n",
			__FUNCTION__, apIndex, returnStatus));
		return WIFI_HAL_ERROR;
	}
	HAL_WIFI_DBG(("%s: AP Index [%d]\n", __FUNCTION__, apIndex));
#ifdef RDKB_TIMER
	wifi_set_timer_variable();
#endif
	return WIFI_HAL_SUCCESS;
}
#else /* WIFI_HAL_VERSION_3_PHASE2 */
/* DeviceMacAddress is in XX:XX:XX:XX:XX:XX format */
INT wifi_addApAclDevice(INT apIndex, CHAR *DeviceMacAddress)
{
	int returnStatus;
	HAL_WIFI_DBG(("%s: apIndex = %d\n", __FUNCTION__, apIndex));

	NULL_PTR_ASSERT(DeviceMacAddress);
	AP_INDEX_ASSERT(apIndex);

	int len = strlen(DeviceMacAddress);

	returnStatus =  wldm_AccessPoint_AclDevice(CMD_ADD, apIndex, DeviceMacAddress, &len, NULL, NULL);
	if (returnStatus != RETURN_OK){
		HAL_WIFI_DBG(("%s: wldm_AccessPoint_AclDevice CMD_ADD Failed, apIndex=[%d], Status=[%d]\n",
			__FUNCTION__, apIndex, returnStatus));
		return RETURN_ERR;
	}
	HAL_WIFI_DBG(("%s: AP Index [%d]\n", __FUNCTION__, apIndex));
#ifdef RDKB_TIMER
	wifi_set_timer_variable();
#endif /* RDKB_TIMER */
	return RETURN_OK;
}
#endif /* WIFI_HAL_VERSION_3_PHASE2 */

// deletes the mac address from the filter list
//DeviceMacAddress is in XX:XX:XX:XX:XX:XX format
INT wifi_delApAclDevice(INT apIndex, CHAR *DeviceMacAddress)
{
	int len, returnStatus;

	HAL_WIFI_DBG(("%s: apIndex = %d\n", __FUNCTION__, apIndex));
	NULL_PTR_ASSERT(DeviceMacAddress);
	AP_INDEX_ASSERT(apIndex);

	len = strlen(DeviceMacAddress);
	returnStatus =  wldm_AccessPoint_AclDevice(CMD_DEL, apIndex, DeviceMacAddress, &len, NULL, NULL);
	if (returnStatus != RETURN_OK){
		HAL_WIFI_DBG(("%s: wldm_AccessPoint_AclDevice CMD_DEL Failed, apIndex=[%d], Status=[%d]\n",
			__FUNCTION__, apIndex, returnStatus));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: AP Index [%d]\n", __FUNCTION__, apIndex));
#ifdef RDKB_TIMER
	wifi_set_timer_variable();
#endif
	return RETURN_OK;
}

// Deletes all Device MAC address from the Access control filter list.
INT wifi_delApAclDevices(INT apIndex)
{
	int returnStatus;

	HAL_WIFI_DBG(("%s: apIndex = %d\n", __FUNCTION__, apIndex));
	AP_INDEX_ASSERT(apIndex);

	/* remove /nvram/greylist_mac.txt */
	unlink("/nvram/greylist_mac.txt");

	returnStatus =  wldm_AccessPoint_DelAclDevices(CMD_DEL, apIndex, NULL, NULL, NULL, NULL);
	if (returnStatus != RETURN_OK){
		HAL_WIFI_DBG(("%s: wldm_AccessPoint_DelAclDevices CMD_DEL Failed, apIndex=[%d], Status=[%d]\n",
			__FUNCTION__, apIndex, returnStatus));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: AP Index [%d]\n", __FUNCTION__, apIndex));
#ifdef RDKB_TIMER
	wifi_set_timer_variable();
#endif
	return RETURN_OK;
}

// outputs the number of devices in the filter list
INT wifi_getApAclDeviceNum(INT apIndex, UINT *output_uint)
{
	int returnStatus;
	int len = sizeof(UINT);

	HAL_WIFI_DBG(("%s: Enter, apIndex=[%d]\n", __FUNCTION__, apIndex));
	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(output_uint);

	*output_uint = 0;

	returnStatus = wldm_AccessPoint_AclDeviceNumber(CMD_GET, apIndex, (INT *)output_uint, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_AccessPoint_AclDeviceNumber CMD_GET Failed, apIndex=[%d] Status=[%d]\n",
			__FUNCTION__, apIndex, returnStatus));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: apIndex=[%d] output_uint=[%d]\n", __FUNCTION__, apIndex, *output_uint));

	return RETURN_OK;
}

// Kick associated devices on acl black list
INT wifi_kickApAclAssociatedDevices(INT apIndex, BOOL enable)
{
	INT iResult = RETURN_OK;
	CHAR assoc_macs[ASSOC_MAC_ARRAY_MAX] = {'\0'};
	CHAR acl_macs[ACL_MAC_ARRAY_MAX] = {'\0'};
	CHAR* devMac = NULL, *rest = NULL;
	BOOL bMacInACL = FALSE;
	ULONG assocDeviceNum = 0;
	int len = 0;

	HAL_WIFI_DBG(("%s: Enter \n", __FUNCTION__));
	AP_INDEX_ASSERT(apIndex);

	/* If there is no devices connected, kick is not needed */
	wifi_getApNumDevicesAssociated(apIndex, &assocDeviceNum);
	if (assocDeviceNum == 0) {
		HAL_WIFI_DBG(("%s: No devices connected on apIndex %d. Kick not required.\n", __FUNCTION__, apIndex));
		return RETURN_OK;
	}

	len = sizeof(assoc_macs);
	iResult = wldm_xbrcm_ap(CMD_GET, apIndex, assoc_macs, (unsigned int *)&len, "assoclist", NULL);
	if (iResult != RETURN_OK) {
		HAL_WIFI_ERR(("%s: wldm_xbrcm_ap - assoclist ERROR\n", __FUNCTION__));
		return RETURN_ERR;
	}
	_strlwr(assoc_macs);

	len = sizeof(acl_macs);
	iResult = wldm_AccessPoint_AclDevices(CMD_GET, apIndex, acl_macs, &len, NULL, NULL);
	if (iResult != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_AccessPoint_AclDevices CMD_GET Failed, apIndex=[%d], Status=[%d]\n",
			__FUNCTION__, apIndex, iResult));
		return RETURN_ERR;
	}
	_strlwr(acl_macs);

	devMac = strtok_r(assoc_macs, ",", &rest);
	while (devMac != NULL) {
		if (strstr(acl_macs, devMac))
			bMacInACL = TRUE;
		else
			bMacInACL = FALSE;

		/* If enable is TRUE, macmode is deny and devices in the list should be kicked.
		*  If enable is FALSE kick devices that are not in ACL list
		*/
		if (bMacInACL == enable) {
			len = strlen(devMac);
			iResult =  wldm_AccessPoint_kickAssociatedDevice(CMD_SET_IOCTL, apIndex, devMac, &len, NULL, NULL);
			if (iResult != RETURN_OK) {
				HAL_WIFI_DBG(("%s: Fail, apIndex=[%d]  iResult=[%d]\n",
					__FUNCTION__, apIndex, iResult));
				return RETURN_ERR;
			}
			/* wifi_apply(); Why do we need wifi_apply() here? */
			HAL_WIFI_DBG(("%s: Kick MAC [%s] from [%d] \n", __FUNCTION__, devMac, apIndex));
		}
		devMac = strtok_r(NULL, ",", &rest);
	}

	HAL_WIFI_DBG(("%s: DONE \n", __FUNCTION__));
	return RETURN_OK;
}

// sets the mac address filter control mode.  0 == filter disabled, 1 == filter as whitelist, 2 == filter as blacklist
INT wifi_setApMacAddressControlMode(INT apIndex, INT filterMode)
{
	int returnStatus;
	int ifiltermode;
	int len = sizeof(ifiltermode);

	HAL_WIFI_DBG(("%s: Enter, apIndex=[%d]\n", __FUNCTION__, apIndex));
	AP_INDEX_ASSERT(apIndex);

	if ((filterMode > 2) || (filterMode < 0)) {
		HAL_WIFI_DBG(("%s: INCORRECT filterMode [%d] \n", __FUNCTION__, filterMode));
		return RETURN_ERR;
	}
	if (filterMode == 1) {
		ifiltermode = 2; /*wldm input 2 for allow/whitelist*/
	} else if (filterMode == 2) {
		ifiltermode = 1; /*wldm input 1 for deny/blacklist*/
	} else {
		ifiltermode = 0;
	}

	returnStatus = wldm_AccessPoint_MACAddressControMode(CMD_SET, apIndex, &ifiltermode, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_AccessPoint_MACAddressControMode CMD_SET Failed, apIndex=[%d], Status=[%d]\n",
			__FUNCTION__, apIndex, returnStatus));
		return RETURN_ERR;
	}
	if (wldm_apply_AccessPointObject(apIndex) != 0) {
		HAL_WIFI_ERR(("%s: apply_AccessPointObject Failed apIndex [%d]\n", __FUNCTION__, apIndex));
		return RETURN_ERR;
	}
#ifdef RDKB_TIMER
	wifi_set_timer_variable();
#endif
	HAL_WIFI_DBG(("%s: apIndex=[%d] filterMode=[%d]\n", __FUNCTION__, apIndex, filterMode));

	return RETURN_OK;
}

// To read the ACL mode
INT wifi_getApMacAddressControlMode(INT apIndex, INT *output_filterMode)
{
	int len, returnStatus, ifiltermode;

	HAL_WIFI_DBG(("%s: Enter, apIndex=[%d]\n", __FUNCTION__, apIndex));
	NULL_PTR_ASSERT(output_filterMode);
	AP_INDEX_ASSERT(apIndex);

	len = sizeof(*output_filterMode);
	returnStatus = wldm_AccessPoint_MACAddressControMode(CMD_GET, apIndex, &ifiltermode, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_AccessPoint_MACAddressControMode CMD_GET Failed, apIndex=[%d], Status=[%d]\n",
			__FUNCTION__, apIndex, returnStatus));
		return RETURN_ERR;
	}
	if (ifiltermode == 1) { /* wldm returns 1 for deny/blacklist */
		*output_filterMode = 2;
	} else if (ifiltermode == 2) { /* wldm returns 2 for allow/whitelist */
		*output_filterMode = 1;
	} else {
		*output_filterMode = 0;
	}

	HAL_WIFI_DBG(("%s: apIndex=[%d] output_filterMode=[%d]\n", __FUNCTION__, apIndex, *output_filterMode));

	return RETURN_OK;
}

// enables internal gateway VLAN mode.  In this mode a Vlan tag is added to upstream (received) data packets before exiting the Wifi driver.  VLAN tags in downstream data are stripped from data packets before transmission.  Default is FALSE.
INT wifi_setApVlanEnable(INT apIndex, BOOL VlanEnabled)
{
	/* Not Applicable */
	HAL_WIFI_DBG(("%s: apIndex%d VlanEnabled:%d\n", __FUNCTION__, apIndex, VlanEnabled));

	AP_INDEX_ASSERT(apIndex);

	return RETURN_OK;
}

// gets the vlan ID for this ap from an internal enviornment variable
INT wifi_getApVlanID(INT apIndex, INT *output_int)
{
	/* Not Applicable */
	HAL_WIFI_DBG(("%s: apIndex: %d\n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(output_int);

	return RETURN_OK;

}

// sets the vlan ID for this ap to an internal enviornment variable
INT wifi_setApVlanID(INT apIndex, INT vlanId)
{
	/* Not Applicable */
	HAL_WIFI_DBG(("%s: apIndex%d vlanId:%d\n", __FUNCTION__, apIndex, vlanId));

	AP_INDEX_ASSERT(apIndex);

	return RETURN_OK;
}

// gets bridgeName, IP address and Subnet. bridgeName is a maximum of 32 characters,
INT wifi_getApBridgeInfo(INT ap_index, CHAR *name, CHAR *IP, CHAR *subnet)
{
	HAL_WIFI_DBG(("%s: index:%d\n", __FUNCTION__, ap_index));

	AP_INDEX_ASSERT(ap_index);
	NULL_PTR_ASSERT(name);
	NULL_PTR_ASSERT(IP);
	NULL_PTR_ASSERT(subnet);

	strncpy(name, bridgeName[ap_index], strlen(bridgeName[ap_index])+1);
	strncpy(IP, bridgeIP[ap_index], strlen(bridgeIP[ap_index])+1);
	strncpy(subnet, bridgeSubnet[ap_index], strlen(bridgeSubnet[ap_index])+1);

	return RETURN_OK;
}

//sets bridgeName, IP address and Subnet to internal enviornment variables. bridgeName is a maximum of 32 characters,
INT wifi_setApBridgeInfo(INT apIndex, CHAR *name, CHAR *IP, CHAR *subnet)
{
	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(name);
	NULL_PTR_ASSERT(IP);
	NULL_PTR_ASSERT(subnet);

	//save settings, wait for wifi reset or wifi_pushBridgeInfo to apply.
	//todo, for every AP
	HAL_WIFI_DBG(("%s: apIndex %d, bridgeName %s, IP %s, subnet %s\n", __FUNCTION__, apIndex, name, IP, subnet));
	strncpy(bridgeName[apIndex], name, strlen(name) + 1);
	strncpy(bridgeIP[apIndex], IP, strlen(IP) + 1);
	strncpy(bridgeSubnet[apIndex], subnet, strlen(subnet) + 1);
	return RETURN_OK;
}

// reset the vlan configuration for this ap
INT wifi_resetApVlanCfg(INT apIndex)
{
	//TODO: remove existing vlan for this ap
	/* Not Applicable */
	HAL_WIFI_DBG(("%s, apIndex = %d, not implemented!!!\n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);

	return RETURN_OK;
}

// creates configuration variables needed for WPA/WPS.  These variables are implementation dependent and in some implementations these variables are used by hostapd when it is started.  Specific variables that are needed are dependent on the hostapd implementation. These variables are set by WPA/WPS security functions in this wifi HAL.  If not needed for a particular implementation this function may simply return no error.
INT wifi_createHostApdConfig(INT apIndex, BOOL createWpsCfg)
{
	HAL_WIFI_LOG(("%s, apIndex = %d, createWpsCfg = %d!!!\n", __FUNCTION__, apIndex, createWpsCfg));

	/*
	 * Don't need to implement it.
	 * hostapd config file will be created in wifi_startHostApd, according to configurations
	 * set through other wifi HAL APIs.
	 */
	return RETURN_OK;
}

// starts the security related deamons
INT wifi_startHostApd()
{
	HAL_WIFI_LOG(("%s\n", __FUNCTION__));
	/*
	 * We have checked WiFi agent code with Comcast,
	 * wifi_stopHostApd() and wifi_startHostApd() are called in pair in one funciton,
	 * so we are good to just call wldm_apply when they are called
	 * hence make wifi_stopHostApd as dummy, and call wldm_apply in wifi_startHostApd
	 */
	wldm_apply_all();

#if (WLDM_AUTO_APPLY_TIME_MS == 0)
	HAL_WIFI_LOG(("%s: done wldm_apply_all, skip sleep\n", __FUNCTION__));
#else
	/*
	 * TODO: If we want to wait, the better solution is wait the apply thread to be finished,
	 * e.g., by condition variable.
	 */
	sleep(8);
#endif
	return RETURN_OK;
}

// stops the security related deamons
INT wifi_stopHostApd()
{
	HAL_WIFI_LOG(("%s: dummy\n", __FUNCTION__));

	/*
	 * We have checked WiFi agent code with Comcast,
	 * wifi_stopHostApd() and wifi_startHostApd() are called in pair in one funciton,
	 * so we are good to just call wldm_apply when they are called
	 * hence make wifi_stopHostApd as dummy, and call wldm_apply in wifi_startHostApd
	 */
	return RETURN_OK;
}

// sets the AP enable status variable for the specified ap.
INT wifi_setApEnable(INT apIndex, BOOL enable)
{
	int len = sizeof(enable);

	HAL_WIFI_DBG(("%s, apIndex=%d enable=%d\n", __FUNCTION__, apIndex, enable));
	if ((enable != TRUE) && (enable != FALSE)) {
		HAL_WIFI_ERR(("%s, enable parameter error!!!, apIndex = %d\n",
			__FUNCTION__, apIndex));
		return RETURN_ERR;
	}
	AP_INDEX_ASSERT(apIndex);

	if (wldm_AccessPoint_Enable(CMD_SET, apIndex, &enable, &len, NULL, NULL) < 0) {
		HAL_WIFI_ERR(("%s, wldm_AccessPoint_Enable CMD_SET Failed, apIndex = %d\n",
			__FUNCTION__, apIndex));
		return RETURN_ERR;
	}

#ifndef BCA_CPEROUTER_RDK
	{
		int radioIndex = HAL_AP_IDX_TO_HAL_RADIO(apIndex);
		if (wldm_apply(radioIndex, 0) < 0) {
			HAL_WIFI_ERR(("%s: wldm_apply() failed, radioIndex=[%d]\n",
				__FUNCTION__, radioIndex));
			return RETURN_ERR;
		}
	}
#endif /* BCA_CPEROUTER_RDK */

	HAL_WIFI_DBG(("%s, wldm_AccessPoint_Enable CMD_SET Done apIndex %d\n",
		__FUNCTION__, apIndex));
#ifdef RDKB_TIMER
	wifi_set_timer_variable();
#endif /* RDKB_TIMER */
	return RETURN_OK;
}

// Outputs the setting of the internal variable that is set by wifi_setEnable().
INT wifi_getApEnable(INT apIndex, BOOL *output_bool)
{
	int returnStatus, len = sizeof(*output_bool);

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(output_bool);

	/* Get configuraion variable */
	returnStatus = wldm_AccessPoint_Enable(CMD_GET_NVRAM, apIndex, output_bool, &len, NULL, NULL);
	if (returnStatus != 0) {
		HAL_WIFI_DBG(("%s: wldm_AccessPoint_Enable CMD_GET_NVRAM Failed, apIndex = %d, Status=%d\n",
			__FUNCTION__, apIndex, returnStatus));
		*output_bool = FALSE;
		return RETURN_OK;
	}

	return RETURN_OK;
}

/* Outputs the AP "Up" "Disable" status from driver - to match cosa_wifi_apis.
 * Comment in wifi_hal.h will need to be changed from "Enabled" "Disabled" to match cosa_wifi_apis
 */
INT wifi_getApStatus(INT apIndex, CHAR *output_string)
{
	int ret = RETURN_OK, bss_status = 0;

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(output_string);

	/* Check bss running status */
	bss_status = hal_bss_status(apIndex);
	if (bss_status < 0) {
		HAL_WIFI_ERR(("%s: %d Error!!!\n", __FUNCTION__, __LINE__));
		bss_status = 0;
		ret = RETURN_ERR;
	}
	snprintf(output_string, OUTPUT_STRING_LENGTH_64, "%s",
		bss_status ? "Up" : "Disable");
	HAL_WIFI_DBG(("%s: apIndex %d output_string=%s\n",
		__FUNCTION__, apIndex, output_string));
	return ret;
}

//Indicates whether or not beacons include the SSID name.
// outputs a 1 if SSID on the AP is enabled, else outputs 0
INT wifi_getApSsidAdvertisementEnable(INT apIndex, BOOL *output_bool)
{
	NULL_PTR_ASSERT(output_bool);
	AP_INDEX_ASSERT(apIndex);

	int returnStatus, len = sizeof(*output_bool);
	*output_bool = FALSE;

	if (!WiFi_Ready_after_hal_started) {
		returnStatus = wldm_AccessPoint_SSIDAdvertisementEnabled(CMD_GET_NVRAM, apIndex, output_bool, &len, NULL, NULL);
	} else {
		returnStatus = wldm_AccessPoint_SSIDAdvertisementEnabled(CMD_GET, apIndex, output_bool, &len, NULL, NULL);
	}

	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_AccessPoint_SSIDAdvertisementEnabled CMD_GET Fail, Status = %d\n",
			__FUNCTION__, returnStatus));
		*output_bool = FALSE;
		return RETURN_ERR;
	}

	return RETURN_OK;
}

// sets an internal variable for ssid advertisement.  Set to 1 to enable, set to 0 to disable
INT wifi_setApSsidAdvertisementEnable(INT apIndex, BOOL enable)
{
	int returnStatus, len = sizeof(enable);

	HAL_WIFI_DBG(("%s: Enter, apIndex = %d, enable = %u\n", __FUNCTION__, apIndex, (unsigned int)enable));
	AP_INDEX_ASSERT(apIndex);

	returnStatus = wldm_AccessPoint_SSIDAdvertisementEnabled(CMD_SET, apIndex, &enable, &len, NULL, NULL);
	if (returnStatus != 0) {
		HAL_WIFI_ERR(("%s: wldm_AccessPoint_SSIDAdvertisementEnabled CMD_SET Failed, Status = %d\n",
			__FUNCTION__, returnStatus));
		return RETURN_ERR;
	}
	returnStatus = wldm_AccessPoint_WPS_Enable(CMD_SET, apIndex, &enable, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_AccessPoint_WPS_Enable CMD_SET Failed, Status = %d\n",
			__FUNCTION__, returnStatus));
		return RETURN_ERR;
	}
#ifdef RDKB_TIMER
	wifi_set_timer_variable();
#endif
	HAL_WIFI_DBG(("%s: Done, apIndex = %d, enable = %u\n", __FUNCTION__, apIndex, (unsigned int)enable));
	return RETURN_OK;
}

//The maximum number of retransmission for a packet. This corresponds to IEEE 802.11 parameter dot11ShortRetryLimit.
INT wifi_getApRetryLimit(INT apIndex, UINT *output_uint)
{
	HAL_WIFI_DBG(("%s: Enter, apindex %d\n", __FUNCTION__, apIndex));
	NULL_PTR_ASSERT(output_uint);
	AP_INDEX_ASSERT(apIndex);

	int returnStatus, len = sizeof(*output_uint);
	*output_uint = 0;

	returnStatus = wldm_AccessPoint_RetryLimit(CMD_GET, apIndex, output_uint, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_AccessPoint_RetryLimit CMD_GET Failed, Status = %d\n",
			__FUNCTION__, returnStatus));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: apindex = %d, output = %d\n", __FUNCTION__, apIndex, *output_uint));
	return RETURN_OK;
}

INT wifi_setApRetryLimit(INT apIndex, UINT number)
{
	INT len = sizeof(number);
	int returnStatus;

	HAL_WIFI_DBG(("%s: Enter, apindex %d\n", __FUNCTION__, apIndex));
	AP_INDEX_ASSERT(apIndex);

	if (number > 255) {
		HAL_WIFI_DBG(("%s: Invalid packet transmissiion retry value %d \n",
			__FUNCTION__, number));
		return RETURN_ERR;
	}

	returnStatus = wldm_AccessPoint_RetryLimit(CMD_SET, apIndex, &number, &len, NULL, NULL);
	if ( returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_AccessPoint_RetryLimit CMD_SET failed, apIndex = %d, Status = %d\n",
			__FUNCTION__ , apIndex, returnStatus));
		return RETURN_ERR;
	}
	HAL_WIFI_DBG(("%s: apindex = %d, number = %d\n", __FUNCTION__, apIndex, number));
#ifdef RDKB_TIMER
	wifi_set_timer_variable();
#endif
	return RETURN_OK;
}

//Indicates whether this access point supports WiFi Multimedia (WMM) Access Categories (AC).
INT wifi_getApWMMCapability(INT apIndex, BOOL *output)
{
	int returnStatus = RETURN_OK;
	int len = sizeof(BOOL);

	HAL_WIFI_DBG(("%s: Enter, apIndex=[%d]\n", __FUNCTION__, apIndex));
	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(output);

	*output = FALSE;
	returnStatus = wldm_AccessPoint_WMMCapability(CMD_GET, apIndex, output, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_AccessPoint_WMMCapability CMD_GET Failed, apIndex=[%d], Status=[%d]\n",
			__FUNCTION__, apIndex, returnStatus));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: apIndex = %d, output = %d\n", __FUNCTION__, apIndex, *output));
	return RETURN_OK;
}

//Indicates whether this access point supports WMM Unscheduled Automatic Power Save Delivery (U-APSD). Note: U-APSD support implies WMM support.
INT wifi_getApUAPSDCapability(INT apIndex, BOOL *output)
{
	int returnStatus = RETURN_OK;
	int len = sizeof(BOOL);

	HAL_WIFI_DBG(("%s: Enter, apIndex=[%d]\n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(output);

	*output = FALSE;

	returnStatus = wldm_AccessPoint_UAPSDCapability(CMD_GET, apIndex, output, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_AccessPoint_UAPSDCapability CMD_GET Failed, apIndex=[%d], Status=[%d]\n",
			__FUNCTION__, apIndex, returnStatus));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: apIndex = %d, output = %d\n", __FUNCTION__, apIndex, *output));
	return RETURN_OK;
}

//Whether WMM support is currently enabled. When enabled, this is indicated in beacon frames.
INT wifi_getApWmmEnable(INT apIndex, BOOL *output)
{
	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(output);

	int returnStatus = RETURN_OK;
	int len = sizeof(*output);

	HAL_WIFI_DBG(("%s: Enter, apIndex=[%d]\n", __FUNCTION__, apIndex));
	*output = FALSE;
	returnStatus = wldm_AccessPoint_WMMEnable(CMD_GET, apIndex, output, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_AccessPoint_WMMEnable Failed, apIndex=[%d], Status=[%d]\n",
			__FUNCTION__, apIndex, returnStatus));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: apIndex = %d, output_bool = %d\n", __FUNCTION__, apIndex, *output));
	return RETURN_OK;
}

// enables/disables WMM on the hardwawre for this AP.  enable==1, disable == 0
INT wifi_setApWmmEnable(INT apIndex, BOOL enable)
{
	int returnStatus = RETURN_OK;
	int len = sizeof(enable);

	HAL_WIFI_DBG(("%s: Enter, apIndex=[%d]\n", __FUNCTION__, apIndex));
	AP_INDEX_ASSERT(apIndex);

	returnStatus = wldm_AccessPoint_WMMEnable(CMD_SET, apIndex, &enable, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_AccessPoint_WMMEnable CMD_SET Failed, apIndex=[%d], Status=[%d]\n",
			__FUNCTION__, apIndex, returnStatus));
		return RETURN_ERR;
	}
#ifdef RDKB_TIMER
	wifi_set_timer_variable();
#endif
	HAL_WIFI_DBG(("%s: Done, apIndex = %d, enable = %d\n", __FUNCTION__, apIndex, enable));

	return RETURN_OK;
}

//Whether U-APSD support is currently enabled. When enabled, this is indicated in beacon frames. Note: U-APSD can only be enabled if WMM is also enabled.
INT wifi_getApWmmUapsdEnable(INT apIndex, BOOL *output)
{
	HAL_WIFI_DBG(("%s: Enter, apIndex=[%d]\n", __FUNCTION__, apIndex));
	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(output);

	int returnStatus = RETURN_OK;
	int len = sizeof(*output);
	*output = FALSE;

	returnStatus = wldm_AccessPoint_UAPSDEnable(CMD_GET, apIndex, output, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_AccessPoint_UAPSDEnable CMD_GET Failed, apIndex=[%d], Status=[%d]\n",
			__FUNCTION__, apIndex, returnStatus));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: apIndex = %d, output = %d\n", __FUNCTION__, apIndex, *output));
	return RETURN_OK;
}

// enables/disables Automatic Power Save Delivery on the hardwarwe for this AP
INT wifi_setApWmmUapsdEnable(INT apIndex, BOOL enable)
{
	int returnStatus = RETURN_OK;
	int len = sizeof(BOOL);

	HAL_WIFI_DBG(("%s: Enter, apIndex=[%d]\n", __FUNCTION__, apIndex));
	AP_INDEX_ASSERT(apIndex);

	returnStatus = wldm_AccessPoint_UAPSDEnable(CMD_SET, apIndex, &enable, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_AccessPoint_UAPSDEnable CMD_SET Failed, apIndex=[%d], Status=[%d]\n",
			__FUNCTION__, apIndex, returnStatus));
		return RETURN_ERR;
	}
#ifdef RDKB_TIMER
	wifi_set_timer_variable();
#endif
	HAL_WIFI_DBG(("%s: apIndex = %d, enable = %u\n", __FUNCTION__, apIndex, (unsigned int)enable));

	return RETURN_OK;
}

// Sets the WMM ACK polity on the hardware. AckPolicy false means do not acknowledge, true means acknowledge
INT wifi_setApWmmOgAckPolicy(INT apIndex, INT class, BOOL ackPolicy) //RDKB
{
	UNUSED_PARAMETER(apIndex);
	UNUSED_PARAMETER(class);
	UNUSED_PARAMETER(ackPolicy);
	HAL_WIFI_DBG(("%s: TBD\n", __FUNCTION__));
	return RETURN_OK;
}

//Enables or disables device isolation. A value of true means that the devices connected to the Access Point are isolated from all other devices within the home network (as is typically the case for a Wireless Hotspot).
INT wifi_getApIsolationEnable(INT apIndex, BOOL *output)
{
	HAL_WIFI_DBG(("%s: apIndex %d\n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(output);

	int slen = sizeof(*output);
	bool enable;

	if (wldm_AccessPoint_IsolationEnable(CMD_GET, apIndex, &enable, &slen, NULL, NULL) < 0) {
		HAL_WIFI_ERR(("%s: wldm_AccessPoint_IsolationEnable CMD GET failed\n", __FUNCTION__));
		return RETURN_ERR;
	}

	*output = enable;
	return RETURN_OK;
}

INT wifi_setApIsolationEnable(INT apIndex, BOOL enable)
{

	HAL_WIFI_LOG(("%s: apIndex = %d, enable = %d\n", __FUNCTION__,
		apIndex, enable));

	AP_INDEX_ASSERT(apIndex);

	int length = sizeof(enable);

	if (wldm_AccessPoint_IsolationEnable(CMD_SET_NVRAM, apIndex, &enable, &length, NULL, NULL) < 0) {
		HAL_WIFI_ERR(("%s: wldm_AccessPoint_IsolationEnable CMD_SET_NVRAM failed\n", __FUNCTION__));
		return RETURN_ERR;
	}

	if (wldm_AccessPoint_IsolationEnable(CMD_SET_IOCTL, apIndex, &enable, &length, NULL, NULL) < 0) {
		HAL_WIFI_ERR(("%s: wldm_AccessPoint_IsolationEnable CMD_SET_IOCTL failed\n", __FUNCTION__));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

//The maximum number of devices that can simultaneously be connected to the access point. A value of 0 means that there is no specific limit.
INT wifi_getApMaxAssociatedDevices(INT apIndex, UINT *output_uint)
{
	HAL_WIFI_DBG(("%s: Enter, apIndex=[%d]\n", __FUNCTION__, apIndex));
	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(output_uint);

	int returnStatus;
	int len = sizeof(*output_uint);
	*output_uint = 0;
	returnStatus = wldm_AccessPoint_MaxAssociatedDevices(CMD_GET, apIndex, (int *) output_uint, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_AccessPoint_MaxAssociatedDevices CMD_GET Failed, apIndex=[%d], Status=[%d]\n",
			__FUNCTION__, apIndex, returnStatus));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: apIndex=[%d] output_uint=[%d]\n", __FUNCTION__, apIndex, *output_uint));
	return RETURN_OK;
}

INT wifi_setApMaxAssociatedDevices(INT apIndex, UINT number)
{
	int returnStatus;
	int len = sizeof(number);

	HAL_WIFI_DBG(("%s: Enter, apIndex=[%d]\n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);

	returnStatus = wldm_AccessPoint_MaxAssociatedDevices(CMD_SET, apIndex, (int *) &number, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_AccessPoint_MaxAssociatedDevices CMD_SET Failed, apIndex=[%d], Status=[%d]\n",
			__FUNCTION__, apIndex, returnStatus));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: apIndex=[%d] number=[%d]\n", __FUNCTION__, apIndex, number));
	return RETURN_OK;
}

/* For HighWatermarkThreshold */
#define WIFI_GET_ASSOC_DEV_HWM_VALUE(keyword) \
do { \
	unsigned len = sizeof(hwm_value); \
	ret = wldm_xbrcm_assoc_dev_hwm(CMD_GET_NVRAM, apIndex, &hwm_value, \
		&len, keyword, NULL); \
	if (ret != 0) { \
		HAL_WIFI_ERR(("%s: Error\n", __FUNCTION__)); \
		return RETURN_ERR; \
	} \
	HAL_WIFI_DBG(("%s: Done %s=%d\n", __FUNCTION__, keyword, hwm_value)); \
} while (0)

/* The HighWatermarkThreshold value that is lesser than or equal to MaxAssociatedDevices.
   Setting this parameter does not actually limit the number of clients that can associate
   with this access point as that is controlled by MaxAssociatedDevices.  MaxAssociatedDevices
   or 50. The default value of this parameter should be equal to MaxAssociatedDevices. In case
   MaxAssociatedDevices is 0 (zero), the default value of this parameter should be 50.
   A value of 0 means that there is no specific limit and Watermark calculation algorithm
   should be turned off.
*/
INT wifi_getApAssociatedDevicesHighWatermarkThreshold(INT apIndex, UINT *output_uint)
{
	int hwm_value, ret;

	HAL_WIFI_DBG(("%s: apIndex: %d\n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(output_uint);

	WIFI_GET_ASSOC_DEV_HWM_VALUE("assoc_dev_hwm_th");
	*output_uint = (UINT)hwm_value;

	return RETURN_OK;
}

INT wifi_setApAssociatedDevicesHighWatermarkThreshold(INT apIndex, UINT Threshold)
{
	unsigned int len;
	int hwm_value, ret;
	char *keyword = "assoc_dev_hwm_th";

	HAL_WIFI_DBG(("%s: apIndex:%d Threshold:%d\n", __FUNCTION__, apIndex, Threshold));

	AP_INDEX_ASSERT(apIndex);

	hwm_value = Threshold;
	len = sizeof(hwm_value);
	ret = wldm_xbrcm_assoc_dev_hwm(CMD_SET_NVRAM, apIndex, &hwm_value,
		&len, keyword, NULL);
	if (ret != 0) {
		HAL_WIFI_ERR(("%s: Error\n", __FUNCTION__));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: Done %s=%d\n", __FUNCTION__, keyword, hwm_value));
	return RETURN_OK;
}

/* Number of times the current total number of associated device has reached the
   HighWatermarkThreshold value. This calculation can be based on the parameter
   AssociatedDeviceNumberOfEntries as well. Implementation specifics about this
   parameter are left to the product group and the device vendors. It can be updated
   whenever there is a new client association request to the access point.
*/
INT wifi_getApAssociatedDevicesHighWatermarkThresholdReached(INT apIndex, UINT *output_uint)
{
	int hwm_value, ret;

	HAL_WIFI_DBG(("%s: apIndex: %d\n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(output_uint);

	WIFI_GET_ASSOC_DEV_HWM_VALUE("assoc_dev_hwm_th_reached");
	*output_uint = (UINT)hwm_value;

	return RETURN_OK;
}

/* Maximum number of associated devices that have ever associated with the access point
   concurrently since the last reset of the device or WiFi module.
*/
INT wifi_getApAssociatedDevicesHighWatermark(INT apIndex, UINT *output_uint)
{
	int hwm_value, ret;

	HAL_WIFI_DBG(("%s: apIndex: %d\n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(output_uint);

	WIFI_GET_ASSOC_DEV_HWM_VALUE("assoc_dev_hwm_max");
	*output_uint = (UINT)hwm_value;

	return RETURN_OK;
}

/* Date and Time at which the maximum number of associated devices ever associated with the
   access point concurrenlty since the last reset of the device or WiFi module (or in short
   when was X_COMCAST-COM_AssociatedDevicesHighWatermark updated). This dateTime value is in UTC.
*/
INT wifi_getApAssociatedDevicesHighWatermarkDate(INT apIndex, ULONG *output_in_seconds)
{
	int hwm_value, ret;

	HAL_WIFI_DBG(("%s: apIndex: %d\n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(output_in_seconds);

	WIFI_GET_ASSOC_DEV_HWM_VALUE("assoc_dev_hwm_max_date");
	*output_in_seconds = (ULONG)hwm_value;

	return RETURN_OK;
}

#if defined(WIFI_HAL_VERSION_3)
/* refer to wifi_ap_security_mode_t for the enum definitions */
wifi_enum_to_str_map_t security_mode_table[] = {
	{wifi_security_mode_none,				"None"},
	{wifi_security_mode_wep_64,				"WEP-64"},
	{wifi_security_mode_wep_128,				"WEP-128"},
	{wifi_security_mode_wpa_personal,			"WPA-Personal"},
	{wifi_security_mode_wpa2_personal,			"WPA2-Personal"},
	{wifi_security_mode_wpa_wpa2_personal,			"WPA-WPA2-Personal"},
	{wifi_security_mode_wpa_enterprise,			"WPA-Enterprise"},
	{wifi_security_mode_wpa2_enterprise,			"WPA2-Enterprise"},
	{wifi_security_mode_wpa_wpa2_enterprise,		"WPA-WPA2-Enterprise"},
	{wifi_security_mode_wpa3_personal,			"WPA3-Personal"},
	{wifi_security_mode_wpa3_transition,		"WPA3-Personal-Transition"},
	{wifi_security_mode_wpa3_enterprise,			"WPA3-Enterprise"},
	{0xff,							NULL}
};

wifi_enum_to_str_map_t encryption_table[] = {
	{wifi_encryption_tkip,		"TKIPEncryption"},
	{wifi_encryption_aes,		"AESEncryption"},
	{wifi_encryption_aes_tkip,	"TKIPandAESEncryption"},
	{0xff,				NULL}
};

wifi_enum_to_str_map_t mfp_table[] = {
	{wifi_mfp_cfg_disabled,		"Disabled"},
	{wifi_mfp_cfg_optional,		"Optional"},
	{wifi_mfp_cfg_required,		"Required"},
	{0xff,				NULL}
};

/* Note: Current values from the cosa layer are for TKIP and AES, with strings as below
 * TBD: for 6710 and later (corerev 130 and above), GCMP_256 when tested and working,
 *	update this table and related use
 */
wifi_enum_to_str_map_t cipher_cap_table[] = {
	{WIFI_CIPHER_CAPA_ENC_WEP40,		"WEP-40"},
	{WIFI_CIPHER_CAPA_ENC_WEP104,		"WEP-104"},
	{WIFI_CIPHER_CAPA_ENC_TKIP,		"TKIPEncryption"},
	{WIFI_CIPHER_CAPA_ENC_CCMP,		"AESEncryption"},
	{WIFI_CIPHER_CAPA_ENC_WEP128,		"WEP128"},
	{WIFI_CIPHER_CAPA_ENC_GCMP,		"GCMP"},
	{WIFI_CIPHER_CAPA_ENC_GCMP_256,		"GCMP-256"},
	{WIFI_CIPHER_CAPA_ENC_CCMP_256,		"CCMP-256"},
	{WIFI_CIPHER_CAPA_ENC_BIP,		"BIP"},
	{WIFI_CIPHER_CAPA_ENC_BIP_GMAC_128,	"BIP_GMAC_128"},
	{WIFI_CIPHER_CAPA_ENC_BIP_GMAC_256,	"BIP_GMAC_256"},
	{WIFI_CIPHER_CAPA_ENC_BIP_CMAC_256,	"BIP_CMAC_256"},
	{WIFI_CIPHER_CAPA_ENC_GTK_NOT_USED,	"GTK_NOT_USED"},
	{0x00,					NULL}
};

/* country code enums from wifi_hal.h; strings returned by wldm
 * Note: skips internal broadcom ref. board specific ccodes, typically used for testing
 * i.e, does not include "#a", "E0", "Q1", "Q2"
 */
wifi_enum_to_str_map_t countrycode_table[] = {
	{wifi_countrycode_AC, "AC"}, /**< ASCENSION ISLAND */
	{wifi_countrycode_AD, "AD"}, /**< ANDORRA */
	{wifi_countrycode_AE, "AE"}, /**< UNITED ARAB EMIRATES */
	{wifi_countrycode_AF, "AF"}, /**< AFGHANISTAN */
	{wifi_countrycode_AG, "AG"}, /**< ANTIGUA AND BARBUDA */
	{wifi_countrycode_AI, "AI"}, /**< ANGUILLA */
	{wifi_countrycode_AL, "AL"}, /**< ALBANIA */
	{wifi_countrycode_AM, "AM"}, /**< ARMENIA */
	{wifi_countrycode_AN, "AN"}, /**< NETHERLANDS ANTILLES */
	{wifi_countrycode_AO, "AO"}, /**< ANGOLA */
	{wifi_countrycode_AQ, "AQ"}, /**< ANTARCTICA */
	{wifi_countrycode_AR, "AR"}, /**< ARGENTINA */
	{wifi_countrycode_AS, "AS"}, /**< AMERICAN SAMOA */
	{wifi_countrycode_AT, "AT"}, /**< AUSTRIA */
	{wifi_countrycode_AU, "AU"}, /**< AUSTRALIA */
	{wifi_countrycode_AW, "AW"}, /**< ARUBA */
	{wifi_countrycode_AZ, "AZ"}, /**< AZERBAIJAN */
	{wifi_countrycode_BA, "BA"}, /**< BOSNIA AND HERZEGOVINA */
	{wifi_countrycode_BB, "BB"}, /**< BARBADOS */
	{wifi_countrycode_BD, "BD"}, /**< BANGLADESH */
	{wifi_countrycode_BE, "BE"}, /**< BELGIUM */
	{wifi_countrycode_BF, "BF"}, /**< BURKINA FASO */
	{wifi_countrycode_BG, "BG"}, /**< BULGARIA */
	{wifi_countrycode_BH, "BH"}, /**< BAHRAIN */
	{wifi_countrycode_BI, "BI"}, /**< BURUNDI */
	{wifi_countrycode_BJ, "BJ"}, /**< BENIN */
	{wifi_countrycode_BM, "BM"}, /**< BERMUDA */
	{wifi_countrycode_BN, "BN"}, /**< BRUNEI DARUSSALAM */
	{wifi_countrycode_BO, "BO"}, /**< BOLIVIA */
	{wifi_countrycode_BR, "BR"}, /**< BRAZIL */
	{wifi_countrycode_BS, "BS"}, /**< BAHAMAS */
	{wifi_countrycode_BT, "BT"}, /**< BHUTAN */
	{wifi_countrycode_BV, "BV"}, /**< BOUVET ISLAND */
	{wifi_countrycode_BW, "BW"}, /**< BOTSWANA */
	{wifi_countrycode_BY, "BY"}, /**< BELARUS */
	{wifi_countrycode_BZ, "BZ"}, /**< BELIZE */
	{wifi_countrycode_CA, "CA"}, /**< CANADA */
	{wifi_countrycode_CC, "CC"}, /**< COCOS (KEELING) ISLANDS */
	{wifi_countrycode_CD, "CD"}, /**< CONGO, THE DEMOCRATIC REPUBLIC OF THE */
	{wifi_countrycode_CF, "CF"}, /**< CENTRAL AFRICAN REPUBLIC */
	{wifi_countrycode_CG, "CG"}, /**< CONGO */
	{wifi_countrycode_CH, "CH"}, /**< SWITZERLAND */
	{wifi_countrycode_CI, "CI"}, /**< COTE D'IVOIRE */
	{wifi_countrycode_CK, "CK"}, /**< COOK ISLANDS */
	{wifi_countrycode_CL, "CL"}, /**< CHILE */
	{wifi_countrycode_CM, "CM"}, /**< CAMEROON */
	{wifi_countrycode_CN, "CN"}, /**< CHINA */
	{wifi_countrycode_CO, "CO"}, /**< COLOMBIA */
	{wifi_countrycode_CP, "CP"}, /**< CLIPPERTON ISLAND */
	{wifi_countrycode_CR, "CR"}, /**< COSTA RICA */
	{wifi_countrycode_CU, "CU"}, /**< CUBA */
	{wifi_countrycode_CV, "CV"}, /**< CAPE VERDE */
	{wifi_countrycode_CY, "CY"}, /**< CYPRUS */
	{wifi_countrycode_CX, "CX"}, /**< CHRISTMAS ISLAND */
	{wifi_countrycode_CZ, "CZ"}, /**< CZECH REPUBLIC */
	{wifi_countrycode_DE, "DE"}, /**< GERMANY */
	{wifi_countrycode_DJ, "DJ"}, /**< DJIBOUTI */
	{wifi_countrycode_DK, "DK"}, /**< DENMARK */
	{wifi_countrycode_DM, "DM"}, /**< DOMINICA */
	{wifi_countrycode_DO, "DO"}, /**< DOMINICAN REPUBLIC */
	{wifi_countrycode_DZ, "DZ"}, /**< ALGERIA */
	{wifi_countrycode_EC, "EC"}, /**< ECUADOR */
	{wifi_countrycode_EE, "EE"}, /**< ESTONIA */
	{wifi_countrycode_EG, "EG"}, /**< EGYPT */
	{wifi_countrycode_EH, "EH"}, /**< WESTERN SAHARA */
	{wifi_countrycode_ER, "ER"}, /**< ERITREA */
	{wifi_countrycode_ES, "ES"}, /**< SPAIN */
	{wifi_countrycode_ET, "ET"}, /**< ETHIOPIA */
	{wifi_countrycode_FI, "FI"}, /**< FINLAND */
	{wifi_countrycode_FJ, "FJ"}, /**< FIJI */
	{wifi_countrycode_FK, "FK"}, /**< FALKLAND ISLANDS (MALVINAS) */
	{wifi_countrycode_FM, "FM"}, /**< MICRONESIA, FEDERATED STATES OF */
	{wifi_countrycode_FO, "FO"}, /**< FAROE ISLANDS */
	{wifi_countrycode_FR, "FR"}, /**< FRANCE */
	{wifi_countrycode_GA, "GA"}, /**< GABON */
	{wifi_countrycode_GB, "GB"}, /**< UNITED KINGDOM */
	{wifi_countrycode_GD, "GD"}, /**< GRENADA */
	{wifi_countrycode_GE, "GE"}, /**< GEORGIA */
	{wifi_countrycode_GF, "GF"}, /**< FRENCH GUIANA */
	{wifi_countrycode_GG, "GG"}, /**< GUERNSEY */
	{wifi_countrycode_GH, "GH"}, /**< GHANA */
	{wifi_countrycode_GI, "GI"}, /**< GIBRALTAR */
	{wifi_countrycode_GL, "GL"}, /**< GREENLAND */
	{wifi_countrycode_GM, "GM"}, /**< GAMBIA */
	{wifi_countrycode_GN, "GN"}, /**< GUINEA */
	{wifi_countrycode_GP, "GP"}, /**< GUADELOUPE */
	{wifi_countrycode_GQ, "GQ"}, /**< EQUATORIAL GUINEA */
	{wifi_countrycode_GR, "GR"}, /**< GREECE */
	{wifi_countrycode_GS, "GS"}, /**< SOUTH GEORGIA AND THE SOUTH SANDWICH ISLANDS */
	{wifi_countrycode_GT, "GT"}, /**< GUATEMALA */
	{wifi_countrycode_GU, "GU"}, /**< GUAM */
	{wifi_countrycode_GW, "GW"}, /**< GUINEA-BISSAU */
	{wifi_countrycode_GY, "GY"}, /**< GUYANA */
	{wifi_countrycode_HR, "HR"}, /**< CROATIA */
	{wifi_countrycode_HT, "HT"}, /**< HAITI */
	{wifi_countrycode_HM, "HM"}, /**< HEARD ISLAND AND MCDONALD ISLANDS */
	{wifi_countrycode_HN, "HN"}, /**< HONDURAS */
	{wifi_countrycode_HK, "HK"}, /**< HONG KONG */
	{wifi_countrycode_HU, "HU"}, /**< HUNGARY */
	{wifi_countrycode_IS, "IS"}, /**< ICELAND */
	{wifi_countrycode_IN, "IN"}, /**< INDIA */
	{wifi_countrycode_ID, "ID"}, /**< INDONESIA */
	{wifi_countrycode_IR, "IR"}, /**< IRAN, ISLAMIC REPUBLIC OF */
	{wifi_countrycode_IQ, "IQ"}, /**< IRAQ */
	{wifi_countrycode_IE, "IE"}, /**< IRELAND */
	{wifi_countrycode_IL, "IL"}, /**< ISRAEL */
	{wifi_countrycode_IM, "IM"}, /**< MAN, ISLE OF */
	{wifi_countrycode_IT, "IT"}, /**< ITALY */
	{wifi_countrycode_IO, "IO"}, /**< BRITISH INDIAN OCEAN TERRITORY */
	{wifi_countrycode_JM, "JM"}, /**< JAMAICA */
	{wifi_countrycode_JP, "JP"}, /**< JAPAN */
	{wifi_countrycode_JE, "JE"}, /**< JERSEY */
	{wifi_countrycode_JO, "JO"}, /**< JORDAN */
	{wifi_countrycode_KE, "KE"}, /**< KENYA */
	{wifi_countrycode_KG, "KG"}, /**< KYRGYZSTAN */
	{wifi_countrycode_KH, "KH"}, /**< CAMBODIA */
	{wifi_countrycode_KI, "KI"}, /**< KIRIBATI */
	{wifi_countrycode_KM, "KM"}, /**< COMOROS */
	{wifi_countrycode_KN, "KN"}, /**< SAINT KITTS AND NEVIS */
	{wifi_countrycode_KP, "KP"}, /**< KOREA, DEMOCRATIC PEOPLE'S REPUBLIC OF */
	{wifi_countrycode_KR, "KR"}, /**< KOREA, REPUBLIC OF */
	{wifi_countrycode_KW, "KW"}, /**< KUWAIT */
	{wifi_countrycode_KY, "KY"}, /**< CAYMAN ISLANDS */
	{wifi_countrycode_KZ, "KZ"}, /**< KAZAKHSTAN */
	{wifi_countrycode_LA, "LA"}, /**< LAO PEOPLE'S DEMOCRATIC REPUBLIC */
	{wifi_countrycode_LB, "LB"}, /**< LEBANON */
	{wifi_countrycode_LC, "LC"}, /**< SAINT LUCIA */
	{wifi_countrycode_LI, "LI"}, /**< LIECHTENSTEIN */
	{wifi_countrycode_LK, "LK"}, /**< SRI LANKA */
	{wifi_countrycode_LR, "LR"}, /**< LIBERIA */
	{wifi_countrycode_LS, "LS"}, /**< LESOTHO */
	{wifi_countrycode_LT, "LT"}, /**< LITHUANIA */
	{wifi_countrycode_LU, "LU"}, /**< LUXEMBOURG */
	{wifi_countrycode_LV, "LV"}, /**< LATVIA */
	{wifi_countrycode_LY, "LY"}, /**< LIBYAN ARAB JAMAHIRIYA */
	{wifi_countrycode_MA, "MA"}, /**< MOROCCO */
	{wifi_countrycode_MC, "MC"}, /**< MONACO */
	{wifi_countrycode_MD, "MD"}, /**< MOLDOVA, REPUBLIC OF */
	{wifi_countrycode_ME, "ME"}, /**< MONTENEGRO */
	{wifi_countrycode_MG, "MG"}, /**< MADAGASCAR */
	{wifi_countrycode_MH, "MH"}, /**< MARSHALL ISLANDS */
	{wifi_countrycode_MK, "MK"}, /**< MACEDONIA, THE FORMER YUGOSLAV REPUBLIC OF */
	{wifi_countrycode_ML, "ML"}, /**< MALI */
	{wifi_countrycode_MM, "MM"}, /**< MYANMAR */
	{wifi_countrycode_MN, "MN"}, /**< MONGOLIA */
	{wifi_countrycode_MO, "MO"}, /**< MACAO */
	{wifi_countrycode_MQ, "MQ"}, /**< MARTINIQUE */
	{wifi_countrycode_MR, "MR"}, /**< MAURITANIA */
	{wifi_countrycode_MS, "MS"}, /**< MONTSERRAT */
	{wifi_countrycode_MT, "MT"}, /**< MALTA */
	{wifi_countrycode_MU, "MU"}, /**< MAURITIUS */
	{wifi_countrycode_MV, "MV"}, /**< MALDIVES */
	{wifi_countrycode_MW, "MW"}, /**< MALAWI */
	{wifi_countrycode_MX, "MX"}, /**< MEXICO */
	{wifi_countrycode_MY, "MY"}, /**< MALAYSIA */
	{wifi_countrycode_MZ, "MZ"}, /**< MOZAMBIQUE */
	{wifi_countrycode_NA, "NA"}, /**< NAMIBIA */
	{wifi_countrycode_NC, "NC"}, /**< NEW CALEDONIA */
	{wifi_countrycode_NE, "NE"}, /**< NIGER */
	{wifi_countrycode_NF, "NF"}, /**< NORFOLK ISLAND */
	{wifi_countrycode_NG, "NG"}, /**< NIGERIA */
	{wifi_countrycode_NI, "NI"}, /**< NICARAGUA */
	{wifi_countrycode_NL, "NL"}, /**< NETHERLANDS */
	{wifi_countrycode_NO, "NO"}, /**< NORWAY */
	{wifi_countrycode_NP, "NP"}, /**< NEPAL */
	{wifi_countrycode_NR, "NR"}, /**< NAURU */
	{wifi_countrycode_NU, "NU"}, /**< NIUE */
	{wifi_countrycode_NZ, "NZ"}, /**< NEW ZEALAND */
	{wifi_countrycode_MP, "MP"}, /**< NORTHERN MARIANA ISLANDS */
	{wifi_countrycode_OM, "OM"}, /**< OMAN */
	{wifi_countrycode_PA, "PA"}, /**< PANAMA */
	{wifi_countrycode_PE, "PE"}, /**< PERU */
	{wifi_countrycode_PF, "PF"}, /**< FRENCH POLYNESIA */
	{wifi_countrycode_PG, "PG"}, /**< PAPUA NEW GUINEA */
	{wifi_countrycode_PH, "PH"}, /**< PHILIPPINES */
	{wifi_countrycode_PK, "PK"}, /**< PAKISTAN */
	{wifi_countrycode_PL, "PL"}, /**< POLAND */
	{wifi_countrycode_PM, "PM"}, /**< SAINT PIERRE AND MIQUELON */
	{wifi_countrycode_PN, "PN"}, /**< PITCAIRN */
	{wifi_countrycode_PR, "PR"}, /**< PUERTO RICO */
	{wifi_countrycode_PS, "PS"}, /**< PALESTINIAN TERRITORY, OCCUPIED */
	{wifi_countrycode_PT, "PT"}, /**< PORTUGAL */
	{wifi_countrycode_PW, "PW"}, /**< PALAU */
	{wifi_countrycode_PY, "PY"}, /**< PARAGUAY */
	{wifi_countrycode_QA, "QA"}, /**< QATAR */
	{wifi_countrycode_RE, "RE"}, /**< REUNION */
	{wifi_countrycode_RO, "RO"}, /**< ROMANIA */
	{wifi_countrycode_RS, "RS"}, /**< SERBIA */
	{wifi_countrycode_RU, "RU"}, /**< RUSSIAN FEDERATION */
	{wifi_countrycode_RW, "RW"}, /**< RWANDA */
	{wifi_countrycode_SA, "SA"}, /**< SAUDI ARABIA */
	{wifi_countrycode_SB, "SB"}, /**< SOLOMON ISLANDS */
	{wifi_countrycode_SD, "SD"}, /**< SUDAN */
	{wifi_countrycode_SE, "SE"}, /**< SWEDEN */
	{wifi_countrycode_SC, "SC"}, /**< SEYCHELLES */
	{wifi_countrycode_SG, "SG"}, /**< SINGAPORE */
	{wifi_countrycode_SH, "SH"}, /**< SAINT HELENA */
	{wifi_countrycode_SI, "SI"}, /**< SLOVENIA */
	{wifi_countrycode_SJ, "SJ"}, /**< SVALBARD AND JAN MAYEN */
	{wifi_countrycode_SK, "SK"}, /**< SLOVAKIA */
	{wifi_countrycode_SL, "SL"}, /**< SIERRA LEONE */
	{wifi_countrycode_SM, "SM"}, /**< SAN MARINO */
	{wifi_countrycode_SN, "SN"}, /**< SENEGAL */
	{wifi_countrycode_SO, "SO"}, /**< SOMALIA */
	{wifi_countrycode_SR, "SR"}, /**< SURINAME */
	{wifi_countrycode_ST, "ST"}, /**< SAO TOME AND PRINCIPE */
	{wifi_countrycode_SV, "SV"}, /**< EL SALVADOR */
	{wifi_countrycode_SY, "SY"}, /**< SYRIAN ARAB REPUBLIC */
	{wifi_countrycode_SZ, "SZ"}, /**< SWAZILAND */
	{wifi_countrycode_TA, "TA"}, /**< TRISTAN DA CUNHA */
	{wifi_countrycode_TC, "TC"}, /**< TURKS AND CAICOS ISLANDS */
	{wifi_countrycode_TD, "TD"}, /**< CHAD */
	{wifi_countrycode_TF, "TF"}, /**< FRENCH SOUTHERN TERRITORIES */
	{wifi_countrycode_TG, "TG"}, /**< TOGO */
	{wifi_countrycode_TH, "TH"}, /**< THAILAND */
	{wifi_countrycode_TJ, "TJ"}, /**< TAJIKISTAN */
	{wifi_countrycode_TK, "TK"}, /**< TOKELAU */
	{wifi_countrycode_TL, "TL"}, /**< TIMOR-LESTE (EAST TIMOR) */
	{wifi_countrycode_TM, "TM"}, /**< TURKMENISTAN */
	{wifi_countrycode_TN, "TN"}, /**< TUNISIA */
	{wifi_countrycode_TO, "TO"}, /**< TONGA */
	{wifi_countrycode_TR, "TR"}, /**< TURKEY */
	{wifi_countrycode_TT, "TT"}, /**< TRINIDAD AND TOBAGO */
	{wifi_countrycode_TV, "TV"}, /**< TUVALU */
	{wifi_countrycode_TW, "TW"}, /**< TAIWAN, PROVINCE OF CHINA */
	{wifi_countrycode_TZ, "TZ"}, /**< TANZANIA, UNITED REPUBLIC OF */
	{wifi_countrycode_UA, "UA"}, /**< UKRAINE */
	{wifi_countrycode_UG, "UG"}, /**< UGANDA */
	{wifi_countrycode_UM, "UM"}, /**< UNITED STATES MINOR OUTLYING ISLANDS */
	{wifi_countrycode_US, "US"}, /**< UNITED STATES */
	{wifi_countrycode_UY, "UY"}, /**< URUGUAY */
	{wifi_countrycode_UZ, "UZ"}, /**< UZBEKISTAN */
	{wifi_countrycode_VA, "VA"}, /**< HOLY SEE (VATICAN CITY STATE) */
	{wifi_countrycode_VC, "VC"}, /**< SAINT VINCENT AND THE GRENADINES */
	{wifi_countrycode_VE, "VE"}, /**< VENEZUELA */
	{wifi_countrycode_VG, "VG"}, /**< VIRGIN ISLANDS, BRITISH */
	{wifi_countrycode_VI, "VI"}, /**< VIRGIN ISLANDS, U.S. */
	{wifi_countrycode_VN, "VN"}, /**< VIET NAM */
	{wifi_countrycode_VU, "VU"}, /**< VANUATU */
	{wifi_countrycode_WF, "WF"}, /**< WALLIS AND FUTUNA */
	{wifi_countrycode_WS, "WS"}, /**< SAMOA */
	{wifi_countrycode_YE, "YE"}, /**< YEMEN */
	{wifi_countrycode_YT, "YT"}, /**< MAYOTTE */
	{wifi_countrycode_YU, "YU"}, /**< YUGOSLAVIA */
	{wifi_countrycode_ZA, "ZA"}, /**< SOUTH AFRICA */
	{wifi_countrycode_ZM, "ZM"}, /**< ZAMBIA */
	{wifi_countrycode_ZW, "ZW"}, /**< ZIMBABWE */
	{wifi_countrycode_max, NULL}, /**< Max number of country code */
};

#endif /* WIFI_HAL_VERSION_3 */

//Comma-separated list of strings. Indicates which security modes this AccessPoint instance is capable of supporting. Each list item is an enumeration of: None, WEP-64, WEP-128, WPA-Personal, WPA2-Personal, WPA3-Personal, WPA-WPA2-Personal, WPA3-Personal-Transition, WPA-Enterprise, WPA2-Enterprise, WPA3-Enterprise, WPA-WPA2-Enterprise
INT wifi_getApSecurityModesSupported(INT apIndex, CHAR *output)
{
	INT len = OUTPUT_STRING_LENGTH_256;
	INT returnStatus ;

	HAL_WIFI_DBG(("%s: Enter, apIndex %d\n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(output);

	output[0] = '\0';

	returnStatus = wldm_AccessPoint_Security_Modessupported(CMD_GET, apIndex, output, &len, NULL, NULL);

	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_AccessPoint_Security_Modessupported CMD_GET Failed,Status = %d\n",
			__FUNCTION__, returnStatus));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s, apIndex = %d, output = %s\n", __FUNCTION__, apIndex, output));
	return RETURN_OK;
}

//The value MUST be a member of the list reported by the ModesSupported parameter. Indicates which security mode is enabled.
INT wifi_getApSecurityModeEnabled(INT apIndex, CHAR *output)
{
	char auth_mode[OUTPUT_STRING_LENGTH_32] = {0};
	int len = sizeof(auth_mode);

	HAL_WIFI_DBG(("%s: Enter, apIndex %d\n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(output);

	if (wldm_AccessPoint_Security_ModeEnabled(CMD_GET_NVRAM, apIndex, auth_mode, &len, NULL, NULL) != 0) {
		HAL_WIFI_DBG(("%s: wldm_AccessPoint_Security_ModeEnabled Failed\n",  __FUNCTION__));
		return RETURN_ERR;
	}

	snprintf(output, OUTPUT_STRING_LENGTH_32, "%s", auth_mode);
	HAL_WIFI_DBG(("%s: apIndex = %d, output = %s\n", __FUNCTION__, apIndex, output));
	return RETURN_OK;
}

INT wifi_setApSecurityModeEnabled(INT apIndex, CHAR *encMode)
{
	int len;

	HAL_WIFI_LOG(("%s: Enter, apIndex %d\n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(encMode);

	len = strlen(encMode);
	if (wldm_AccessPoint_Security_ModeEnabled(CMD_SET, apIndex, encMode, &len, NULL, NULL) != 0) {
		HAL_WIFI_DBG(("%s: wldm_AccessPoint_Security_ModeEnabled CMD_SET Failed\n", __FUNCTION__));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: apIndex = %d, encMode = %s\n", __FUNCTION__, apIndex, encMode));
#ifdef RDKB_TIMER
	wifi_set_timer_variable();
#endif
	return RETURN_OK;
}

//A literal PreSharedKey (PSK) expressed as a hexadecimal string.
// output_string must be pre-allocated as 64 character string by caller
// PSK Key of 8 to 63 characters is considered an ASCII string, and 64 characters are considered as HEX value
INT wifi_getApSecurityPreSharedKey(INT apIndex, CHAR *output_string)
{
	CHAR preSharedKey[OUTPUT_STRING_LENGTH_64 + 1] = {'\0'};
	int returnStatus, len = sizeof(preSharedKey);
	memset(preSharedKey, 0, OUTPUT_STRING_LENGTH_64 + 1);

	HAL_WIFI_DBG(("%s: apIndex %d\n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(output_string);

	output_string[0] = '\0';

	returnStatus = wldm_AccessPoint_Security_PreSharedKey(CMD_GET, apIndex,
				preSharedKey, &len, NULL, NULL);
	if (returnStatus < 0) {
		HAL_WIFI_DBG(("%s: wldm_AccessPoint_Security_PreSharedKey CMD_GET Failed Status=%d\n",
				__FUNCTION__, returnStatus));
		return RETURN_ERR;
	}

	snprintf(output_string, OUTPUT_STRING_LENGTH_64 + 1, "%s", preSharedKey);
	return RETURN_OK;
}

// sets an enviornment variable for the psk. Input string preSharedKey must be a maximum of 64 characters
// PSK Key of 8 to 63 characters is considered an ASCII string, and 64 characters are considered as HEX value
INT wifi_setApSecurityPreSharedKey(INT apIndex, CHAR *preSharedKey)
{
	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(preSharedKey);

	int len = strlen(preSharedKey);
	int returnStatus;

	/*Sensitive Information Like KeyPassphrase should not print in log*/
	HAL_WIFI_DBG(("%s: apIndex = %d\n", __FUNCTION__, apIndex));
	returnStatus =  wldm_AccessPoint_Security_PreSharedKey(CMD_SET, apIndex, preSharedKey, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_AccessPoint_Security_PreSharedKey CMD_GET Failed, apIndex=[%d], Status=[%d]\n",
			__FUNCTION__, apIndex, returnStatus));
		return RETURN_ERR;
	}
#ifdef RDKB_TIMER
	wifi_set_timer_variable();
#endif
	return RETURN_OK;
}

//A passphrase from which the PreSharedKey is to be generated, for WPA-Personal or WPA2-Personal or WPA-WPA2-Personal security modes.
// outputs the passphrase, maximum 63 characters
INT wifi_getApSecurityKeyPassphrase(INT apIndex, CHAR *output_string)
{
	int status, len = OUTPUT_STRING_LENGTH_64;
	char *pTemp = NULL;

	NULL_PTR_ASSERT(output_string);
	AP_INDEX_ASSERT(apIndex);

	/*Sensitive Information Like KeyPassphrase should not print in log*/
	HAL_WIFI_DBG(("%s, apIndex = %d\n", __FUNCTION__, apIndex));

	status = wldm_AccessPoint_Security_KeyPassphrase(CMD_GET, apIndex, output_string, &len, NULL, NULL);
	if (status == RETURN_OK) {
		//Remove new line character at the end
		if((pTemp = strchr(output_string, '\n')) != NULL)
			*pTemp = '\0';

		/*Sensitive Information Like KeyPassphrase should not print in log*/
		HAL_WIFI_DBG(("%s, apIndex = %d, output_string = xxxx\n", __FUNCTION__, apIndex));
		return RETURN_OK;
	}
	else {
		HAL_WIFI_ERR(("%s, wldm_AccessPoint_Security_KeyPassphrase fail, status = %d\n", __FUNCTION__, status));
		return RETURN_ERR;
	}
}

// sets the passphrase enviornment variable, max 63 characters
INT wifi_setApSecurityKeyPassphrase(INT apIndex, CHAR *passPhrase)
{
	int len = strlen(passPhrase);

	//save to wifi config and hotapd config. wait for wifi reset or hostapd restet to apply
	NULL_PTR_ASSERT(passPhrase);
	AP_INDEX_ASSERT(apIndex);

	/*Sensitive Information Like KeyPassphrase should not print in log*/
	HAL_WIFI_DBG(("%s,  apIndex = %d\n", __FUNCTION__, apIndex));

	if (wldm_AccessPoint_Security_KeyPassphrase(CMD_SET, apIndex, passPhrase, &len, NULL, NULL) < 0)
		return RETURN_ERR;

	if (wldm_apply_AccessPointSecurityObject(apIndex) != 0) {
		HAL_WIFI_ERR(("%s: apply_AccessPointObject Failed apIndex [%d]\n", __FUNCTION__, apIndex));
		return RETURN_ERR;
	}
#ifdef RDKB_TIMER
	wifi_set_timer_variable();
#endif
	return RETURN_OK;
}

// STUB function sets the internal variable for the rekey interval
INT wifi_setApSecurityWpaRekeyInterval(INT apIndex, INT rekeyInterval)
{
	UNUSED_PARAMETER(apIndex);
	UNUSED_PARAMETER(rekeyInterval);
	return RETURN_ERR;
}

//When set to true, this AccessPoint instance's WiFi security settings are reset to their factory default values. The affected settings include ModeEnabled, WEPKey, PreSharedKey and KeyPassphrase.
INT wifi_setApSecurityReset(INT apIndex)
{
	AP_INDEX_ASSERT(apIndex);
	return wldm_xbrcm_factory_reset(WLDM_NVRAM_FACTORY_RESET_APSEC, apIndex, 0, 0);
}

//The IP Address and port number of the RADIUS server used for WLAN security. RadiusServerIPAddr is only applicable when ModeEnabled is an Enterprise type (i.e. WPA-Enterprise, WPA2-Enterprise or WPA-WPA2-Enterprise).
/*NEW todo Secret*/
INT wifi_getApSecurityRadiusServer(INT apIndex, CHAR *IP_output, UINT *Port_output, CHAR *RadiusSecret_output) //Tr181
{
	HAL_WIFI_DBG(("%s, apIndex = %d\n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(IP_output);
	NULL_PTR_ASSERT(Port_output);
	NULL_PTR_ASSERT(RadiusSecret_output);

	INT len = OUTPUT_STRING_LENGTH_64;

	if (wldm_AccessPoint_Security_RadiusServerIPAddr(CMD_GET, apIndex, IP_output, &len, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: Fail to get IP\n", __FUNCTION__));
		return RETURN_ERR;
	}

	len = sizeof(*Port_output);
	if (wldm_AccessPoint_Security_RadiusServerPort(CMD_GET, apIndex, Port_output, &len, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: Fail to get port\n", __FUNCTION__));
		return RETURN_ERR;
	}

	len = OUTPUT_STRING_LENGTH_64;
	if (wldm_AccessPoint_Security_RadiusSecret(CMD_GET, apIndex, RadiusSecret_output, &len, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: Fail to get secret\n", __FUNCTION__));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

/*NEW todo Secret*/
INT wifi_setApSecurityRadiusServer(INT apIndex, CHAR *IPAddress, UINT port, CHAR *RadiusSecret) //Tr181
{
	//store the paramters, and apply instantly

	NULL_PTR_ASSERT(IPAddress);
	NULL_PTR_ASSERT(RadiusSecret);
	AP_INDEX_ASSERT(apIndex);

	HAL_WIFI_DBG(("%s, apIndex = %d, IPAddress=%s, port = %d, RadiusSecret = %s\n",
			 __FUNCTION__, apIndex, IPAddress, port, RadiusSecret));
	INT len = 0;

	len = strlen(IPAddress);
	if (wldm_AccessPoint_Security_RadiusServerIPAddr(CMD_SET, apIndex, IPAddress, &len, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: Fail to set IP\n", __FUNCTION__));
		return RETURN_ERR;
	}

	len = sizeof(port);
	if (wldm_AccessPoint_Security_RadiusServerPort(CMD_SET, apIndex, &port, &len, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: Fail to set port\n", __FUNCTION__));
		return RETURN_ERR;
	}

	len = strlen(RadiusSecret);
	if (wldm_AccessPoint_Security_RadiusSecret(CMD_SET, apIndex, RadiusSecret, &len, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: Fail to set secret\n", __FUNCTION__));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

//RadiusSettings
INT wifi_getApSecurityRadiusSettings(INT apIndex, wifi_radius_setting_t *output)
{
	HAL_WIFI_DBG(("wifi_getApSecurityRadiusSettings apIndex %d\n", apIndex));

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(output);

	int len;
	Device_WiFi_AccessPoint_Security_X_COMCAST_COM_RadiusSettings RadiusSettings;

	len = sizeof(RadiusSettings);
	if (wldm_AccessPoint_Security_X_COMCAST_COM_RadiusSettings(CMD_GET, apIndex, &RadiusSettings, &len, NULL, NULL)) {
		HAL_WIFI_ERR(("%s: Fail to get RadiusSettings\n", __FUNCTION__));
		return RETURN_ERR;
	}
	output->RadiusServerRetries = RadiusSettings.RadiusServerRetries;
	output->RadiusServerRequestTimeout = RadiusSettings.RadiusServerRequestTimeout;
	output->PMKLifetime = RadiusSettings.PMKLifetime;
	output->PMKCaching = RadiusSettings.PMKCaching;
	output->PMKCacheInterval = RadiusSettings.PMKCacheInterval;
	output->MaxAuthenticationAttempts = RadiusSettings.MaxAuthenticationAttempts;
	output->BlacklistTableTimeout = RadiusSettings.BlacklistTableTimeout;
	output->IdentityRequestRetryInterval = RadiusSettings.IdentityRequestRetryInterval;
	output->QuietPeriodAfterFailedAuthentication = RadiusSettings.QuietPeriodAfterFailedAuthentication;

	return RETURN_OK;
}

INT wifi_setApSecurityRadiusSettings(INT apIndex, wifi_radius_setting_t *input)
{
	HAL_WIFI_DBG(("wifi_setApSecurityRadiusSettings apIndex %d\n", apIndex));

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(input);

	int len;
	Device_WiFi_AccessPoint_Security_X_COMCAST_COM_RadiusSettings RadiusSettings;

	len = sizeof(RadiusSettings);
	RadiusSettings.RadiusServerRetries = input->RadiusServerRetries;
	RadiusSettings.RadiusServerRequestTimeout = input->RadiusServerRequestTimeout;
	RadiusSettings.PMKLifetime = input->PMKLifetime;
	RadiusSettings.PMKCaching = input->PMKCaching;
	RadiusSettings.PMKCacheInterval = input->PMKCacheInterval;
	RadiusSettings.MaxAuthenticationAttempts = input->MaxAuthenticationAttempts;
	RadiusSettings.BlacklistTableTimeout = input->BlacklistTableTimeout;
	RadiusSettings.IdentityRequestRetryInterval = input->IdentityRequestRetryInterval;
	RadiusSettings.QuietPeriodAfterFailedAuthentication = input->QuietPeriodAfterFailedAuthentication;
	if (wldm_AccessPoint_Security_X_COMCAST_COM_RadiusSettings(CMD_SET, apIndex, &RadiusSettings, &len, NULL, NULL)) {
		HAL_WIFI_ERR(("%s: Fail to set RadiusSettings\n", __FUNCTION__));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

//Enables or disables WPS functionality for this access point.
// outputs the WPS enable state of this ap in output_bool
INT wifi_getApWpsEnable(INT apIndex, BOOL *output_bool)
{
	HAL_WIFI_DBG(("%s: Enter, apIndex %d\n", __FUNCTION__, apIndex));

	NULL_PTR_ASSERT(output_bool);
	AP_INDEX_ASSERT(apIndex);

	INT returnStatus, len = sizeof(*output_bool);
	*output_bool = FALSE;

	returnStatus = wldm_AccessPoint_WPS_Enable(CMD_GET, apIndex, output_bool, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_AccessPoint_WPS_Enable CMD_GET Failed, Status = %d\n",
			__FUNCTION__, returnStatus));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: apIndex = %d, output_bool = %d\n", __FUNCTION__, apIndex, *output_bool));
	return RETURN_OK;
}

// sets the WPS enable enviornment variable for this ap to the value of enableValue, 1==enabled, 0==disabled
INT wifi_setApWpsEnable(INT apIndex, BOOL enableValue)
{
	int  returnStatus ;
	int len = sizeof(enableValue);

	HAL_WIFI_DBG(("%s: Enter, apIndex %d\n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);

	returnStatus = wldm_AccessPoint_WPS_Enable(CMD_SET, apIndex, &enableValue, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_AccessPoint_WPS_Enable CMD_SET Failed, Status = %d\n",
			__FUNCTION__, returnStatus));
		return RETURN_ERR;
	}
#ifdef RDKB_TIMER
	wifi_set_timer_variable();
#endif
	HAL_WIFI_DBG(("%s: apIndex = %d, enableValue = %d\n", __FUNCTION__, apIndex, enableValue));

	WPS_LOG("RDKB_WPS_ENABLED_%d %s\n", apIndex + 1,
		enableValue ? "TRUE" : "FALSE");

	return RETURN_OK;
}

//Comma-separated list of strings. Indicates WPS configuration methods supported by the device. Each list item is an enumeration of: USBFlashDrive,Ethernet,ExternalNFCToken,IntegratedNFCToken,NFCInterface,PushButton,PIN
INT wifi_getApWpsConfigMethodsSupported(INT apIndex, CHAR *methodString)
{
	INT len = OUTPUT_STRING_LENGTH_128+1 ;
	INT returnStatus ;

	NULL_PTR_ASSERT(methodString);
	AP_INDEX_ASSERT(apIndex);

	HAL_WIFI_DBG(("%s: Enter, apIndex %d\n", __FUNCTION__, apIndex));
	methodString[0] = '\0';
	returnStatus = wldm_AccessPoint_WPS_ConfigMethodsSupported(CMD_GET, apIndex,
		methodString, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_AccessPoint_WPS_ConfigMethodsSupported CMD_GET Failed, Status = %d\n",
			__FUNCTION__, returnStatus));
		return RETURN_ERR;
	}
	HAL_WIFI_DBG(("%s: apIndex = %d, methodString = %s\n", __FUNCTION__, apIndex, methodString));
	return RETURN_OK;
}

//Comma-separated list of strings. Each list item MUST be a member of the list reported by the ConfigMethodsSupported parameter. Indicates WPS configuration methods enabled on the device.
// Outputs a common separated list of the enabled WPS config methods, 64 bytes max
INT wifi_getApWpsConfigMethodsEnabled(INT apIndex, CHAR *output)
{
	INT len = OUTPUT_STRING_LENGTH_128+1 ;
	INT returnStatus ;

	HAL_WIFI_DBG(("%s: Enter, apIndex %d\n", __FUNCTION__, apIndex));
	NULL_PTR_ASSERT(output);
	AP_INDEX_ASSERT(apIndex);

	output[0] = '\0';
	returnStatus = wldm_AccessPoint_WPS_ConfigMethodsEnabled(CMD_GET, apIndex, output, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_AccessPoint_WPS_ConfigMethodsEnabled CMD_GET Failed, Status = %d\n",
			__FUNCTION__, returnStatus));
		return RETURN_ERR;
	}
	HAL_WIFI_DBG(("%s: apIndex = %d, output = %s\n", __FUNCTION__, apIndex, output));
	return RETURN_OK;
}

// sets an enviornment variable that specifies the WPS configuration method(s).  methodString is a comma separated list of methods USBFlashDrive,Ethernet,ExternalNFCToken,IntegratedNFCToken,NFCInterface,PushButton,PIN
INT wifi_setApWpsConfigMethodsEnabled(INT apIndex, CHAR *methodString)
{
	NULL_PTR_ASSERT(methodString);
	AP_INDEX_ASSERT(apIndex);

	HAL_WIFI_DBG(("%s, apIndex = %d, methodString = %s\n", __FUNCTION__, apIndex, methodString));

	int len = strlen(methodString) + 1;
	int ret;

	ret = wldm_AccessPoint_WPS_ConfigMethodsEnabled(CMD_SET, apIndex,
		methodString, &len, NULL, NULL);
	if (ret < 0) {
		HAL_WIFI_ERR(("%s: failed, ret=%d", __FUNCTION__, ret));
		return RETURN_ERR;
	}

	WPS_LOG("RDKB_WPS_PBC_CONFIGURED_%d %s\n", apIndex + 1,
		strstr(methodString, "PushButton") ? "TRUE" : "FALSE");
	WPS_LOG("RDKB_WPS_PIN_CONFIGURED_%d %s\n", apIndex + 1,
		(strstr(methodString, "PIN") || strstr(methodString, "Keypad"))
		? "TRUE" : "FALSE");
	return RETURN_OK;
}

// outputs the pin value, ulong_pin must be allocated by the caller
INT wifi_getApWpsDevicePIN(INT apIndex, ULONG *output_ulong)
{
	CHAR Wpspin[OUTPUT_STRING_LENGTH_32+1];
	INT len = OUTPUT_STRING_LENGTH_32+1 ;
	INT returnStatus ;

	HAL_WIFI_DBG(("%s: Enter, apIndex %d\n", __FUNCTION__, apIndex));
	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(output_ulong);

	*output_ulong = 0;
	returnStatus = wldm_AccessPoint_WPS_PIN(CMD_GET, apIndex, Wpspin, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_AccessPoint_WPS_PIN CMD_GET Failed, Status = %d\n",
			__FUNCTION__, returnStatus));
		return RETURN_ERR;
	}
	*output_ulong = strtoul(Wpspin, NULL, 10);

	/*sensitive information should not print in log*/
	HAL_WIFI_DBG(("%s: apIndex = %d, output_ulong = xxxx, pin = xxxx\n", __FUNCTION__, apIndex));
	return RETURN_OK;
}

// set an enviornment variable for the WPS pin for the selected AP. Normally, Device PIN should not be changed.
INT wifi_setApWpsDevicePIN(INT apIndex, ULONG pin)
{
	CHAR Wpspin[OUTPUT_STRING_LENGTH_32+1];
	INT len ;
	INT returnStatus ;

	HAL_WIFI_DBG(("%s: Enter, apIndex %d\n", __FUNCTION__, apIndex));
	AP_INDEX_ASSERT(apIndex);

	memset(Wpspin, 0, sizeof(Wpspin));
	snprintf(Wpspin, OUTPUT_STRING_LENGTH_32,"%lu", pin);
	len = strlen(Wpspin);
	returnStatus = wldm_AccessPoint_WPS_PIN(CMD_SET, apIndex, Wpspin, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_AccessPoint_WPS_PIN CMD_SET Failed, Status = %d\n",
			__FUNCTION__, returnStatus));
		return RETURN_ERR;
	}
#ifdef RDKB_TIMER
	wifi_set_timer_variable();
#endif
	/*sensitive information should not print in log*/
	HAL_WIFI_DBG(("%s: apIndex = %d, devicePIN = xxxx\n", __FUNCTION__, apIndex));
	return RETURN_OK;
}

// Output string is either Not configured or Configured, max 32 characters
INT wifi_getApWpsConfigurationState(INT apIndex, CHAR *output_string)
{
	INT len = OUTPUT_STRING_LENGTH_32 + 1;
	INT returnStatus ;

	HAL_WIFI_DBG(("%s: Enter, apIndex %d\n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(output_string);

	output_string[0] = '\0';
	returnStatus = wldm_AccessPoint_WPS_ConfigurationState(CMD_GET, apIndex, output_string, &len, NULL, NULL);

	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_AccessPoint_WPS_ConfigurationState CMD_GET Failed, Status = %d\n",
			__FUNCTION__, returnStatus));
		return RETURN_ERR;
	}
	HAL_WIFI_DBG(("%s: apIndex = %d, output_string = %s\n", __FUNCTION__, apIndex, output_string));
	return RETURN_OK;
}

static void *wifi_wpsPairingCompletionCheck_priv(void *arg)
{
	INT apIndex = 1;
	INT input_arg = *(int *)arg;
	INT32 status = 0;
	timeCount = WPS_PAIRING_TIMEOUT;
	PCHAR pValue = NULL;

	do
	{
		pValue = nvram_get("wps_proc_status");
		if (!pValue) {
			HAL_WIFI_ERR(("%s, fail to get WPS status\n", __FUNCTION__));
#if defined(_XB7_PRODUCT_REQ_) || defined(_XB8_PRODUCT_REQ_)
			HAL_WIFI_DBG(("%s: Stop WPS Pairing, RDKB_WPS_PIN Failed\n", __FUNCTION__));
			system("led_wps_active 0");
#endif
			WPS_LOG("RDKB_WPS_%d FAILED\n", (input_arg + 1));
			goto exit;
		}

		status = atoi(pValue);
		HAL_WIFI_DBG(("%s, status = %d, timeCount = %d\n", __FUNCTION__, status, timeCount));

		if (status == HAL_WPS_OK) {
			/* disable LED*/
#ifdef WPS_LED_DRIVER_SUPPORT
			system("echo stop_pairing > /proc/driver/wps/cmd");
#endif /* WPS_LED_DRIVER_SUPPORT */
			pValue = nvram_get("wps_ifname");
			if (!pValue || (RETURN_ERR == wifi_getIndexFromName(pValue, &apIndex))) {
				HAL_WIFI_ERR(("%s, fail to get WPS AP index\n", __FUNCTION__));
				goto exit;
			}
			WPS_LOG("RDKB_WPS_%d SUCCESS\n", apIndex + 1);

#if defined(_XB7_PRODUCT_REQ_) || defined(_XB8_PRODUCT_REQ_)
			HAL_WIFI_DBG(("%s: Stop WPS Pairing, status = %d\n", __FUNCTION__, status));
			system("led_wps_active 0");
#endif
			goto exit;
		}

		timeCount--;
		sleep(2);
	} while (timeCount > 0);
	if (timeCount == 0) {
		WPS_LOG("RDKB_WPS_%d TIMEOUT\n", (input_arg + 1));
	}
exit:
#if defined(_XB7_PRODUCT_REQ_) || defined(_XB8_PRODUCT_REQ_)
	HAL_WIFI_DBG(("%s: Stop WPS Pairing\n", __FUNCTION__));
	system("led_wps_active 0");
#endif
	timeCount = 0;
	pthread_exit(NULL);
	return NULL;
}

static void wifi_startWpsPollingTask_priv(INT apIndex)
{
#ifdef WPS_LED_DRIVER_SUPPORT
	system("echo start_pairing > /proc/driver/wps/cmd");
#endif
	if ((pthread_t) NULL != wpsThread) {
		if (timeCount > 2) {
			HAL_WIFI_DBG(("%s, privious thread is still alive, wpsThread = 0x%x, timeCount = %d\n", __FUNCTION__, wpsThread, timeCount));
			timeCount = WPS_PAIRING_TIMEOUT;
			return;
		} else {
			pthread_join(wpsThread, NULL);
			wpsThread = (pthread_t)NULL;
		}
	}

	if (pthread_create(&wpsThread, NULL, wifi_wpsPairingCompletionCheck_priv, (void *)&apIndex)) {
		HAL_WIFI_DBG(("%s, fail to create wps polling task, wpsThread = 0x%x, timeCount = %d\n", __FUNCTION__, wpsThread, timeCount));
	} else {
		HAL_WIFI_DBG(("%s, create wps polling task, wpsThread = 0x%x, timeCount = %d\n", __FUNCTION__, wpsThread, timeCount));;
	}

	return;
}

/* condition for WPS:
	both bss_enabled == 1 and  wps_mode == enabled
	return: RETURN_OK to proceed, RETURN_ERR to skip
*/
static INT wifi_checkApWpsEnable(INT apIndex)
{
	BOOLEAN enable;
	INT32 status = 0;

	/* condition-1: AP is enabled? */
	if (((status = wifi_getApEnable(apIndex, &enable)) == RETURN_ERR) || (enable != TRUE)) {
		HAL_WIFI_ERR(("%s, AP (%d) not enabled (status=%d, enable=%d)\n", __FUNCTION__, apIndex, status, enable));
		return RETURN_ERR;
	}

	/* condition-2: wps is enabled? */
	if (((status = wifi_getApWpsEnable(apIndex, &enable)) == RETURN_ERR) || (enable != TRUE)) {
		HAL_WIFI_ERR(("%s, AP (%d) WPS not enabled (status=%d, enable=%d)\n", __FUNCTION__, apIndex, status, enable));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

// sets the WPS pin for this AP
INT wifi_setApWpsEnrolleePin(INT apIndex, CHAR *pin)
{
	NULL_PTR_ASSERT(pin);
	AP_INDEX_ASSERT(apIndex);

	if (wifi_checkApWpsEnable(apIndex) != RETURN_OK) {
		HAL_WIFI_DBG(("%s, Skip WPS-PIN for apIndex [%d], not enabled\n", __FUNCTION__, apIndex));
		goto print_err;
	}

	wldm_wfa_wps_param_t param;

	param.cmd = WFA_WPS_SET_CLIENT_PIN;
	param.apIndex = apIndex;
	param.param.pin = pin;
	if (wldm_wfa_wps(&param) != 0)
		goto print_err;
	WPS_LOG("RDKB_WPS_PIN_%d START\n", apIndex + 1);

	/*CCSP will start WPS for both AP 0 and AP 1*/
	// if(apIndex == 1)
	{
#if defined(_XB7_PRODUCT_REQ_) || defined(_XB8_PRODUCT_REQ_)
		HAL_WIFI_DBG(("%s: Start WPS Pairing\n", __FUNCTION__));
		system("led_wps_active 1");
#endif
		wifi_startWpsPollingTask_priv(apIndex);
	}

	return RETURN_OK;
print_err:
	WPS_LOG("RDKB_WPS_PIN_%d FAILED\n", apIndex + 1);

	return RETURN_ERR;
}

// This function is called when the WPS push button has been pressed for this AP
// Need to apply instantly
INT wifi_setApWpsButtonPush(INT apIndex)
{
	AP_INDEX_ASSERT(apIndex);
	if (wifi_checkApWpsEnable(apIndex) != RETURN_OK) {
		HAL_WIFI_DBG(("%s, Skip WPS-PBC for apIndex [%d], not enabled\n", __FUNCTION__, apIndex));
		goto print_err;
	}

	wldm_wfa_wps_param_t param;

	param.cmd = WFA_WPS_ACTIVATE_PUSH_BUTTON;
	param.apIndex = apIndex;
	if (wldm_wfa_wps(&param) != 0)
		goto print_err;

	WPS_LOG("RDKB_WPS_CLI PBC_%d START\n", apIndex + 1);

	/*CCSP will start WPS for both AP 0 and AP 1*/
	// if(apIndex == 1)
	{
		wifi_startWpsPollingTask_priv(apIndex);
	}

	return RETURN_OK;
print_err:
	WPS_LOG("RDKB_WPS_PBC_%d FAILED\n", apIndex + 1);
	return RETURN_ERR;

}

// cancels WPS mode for this AP
INT wifi_cancelApWPS(INT apIndex)
{
	AP_INDEX_ASSERT(apIndex);

	if (wifi_checkApWpsEnable(apIndex) != RETURN_OK) {
		HAL_WIFI_ERR(("%s, Skip WPS-CANCEL for apIndex [%d], not enabled\n", __FUNCTION__, apIndex));
		goto print_err;
	}

#if defined(_XB7_PRODUCT_REQ_) || defined(_XB8_PRODUCT_REQ_)
	HAL_WIFI_DBG(("%s: Cancel WPS Pairing\n", __FUNCTION__));
	system("led_wps_active 0");
#endif

	if ((pthread_t)NULL != wpsThread) {
		if (timeCount > 0) {
			HAL_WIFI_DBG(("%s, privious thread is still alive, wpsThread = 0x%x, timeCount = %d\n", __FUNCTION__, wpsThread, timeCount));
			timeCount = 0;
		}
	}

	wldm_wfa_wps_param_t param;

	param.cmd = WFA_WPS_CANCEL;
	param.apIndex = apIndex;
	if (wldm_wfa_wps(&param) != 0) {
		goto print_err;
	}

	WPS_LOG("RDKB_WPS_CANCEL_%d START\n", apIndex + 1);
	return RETURN_OK;

print_err:
	WPS_LOG("RDKB_WPS_CANCEL_%d FAILED\n", apIndex + 1);
	return RETURN_ERR;
}

//Device.WiFi.AccessPoint.{i}.AssociatedDevice.*
//HAL funciton should allocate an data structure array, and return to caller with "associated_dev_array"
INT wifi_getApAssociatedDeviceDiagnosticResult(INT apIndex, wifi_associated_dev_t **associated_dev_array, UINT *output_array_size)
{
	unsigned long assoc_num;
	unsigned int max_assoc_devices, len;
	int ret;
	unsigned int i;
	wifi_associated_dev_t *parray;
	wldm_wifi_associated_dev1_t *p_wldm_array;

	NULL_PTR_ASSERT(output_array_size);
	AP_INDEX_ASSERT(apIndex);

	*output_array_size = 0;
	*associated_dev_array = NULL;

	wifi_getApNumDevicesAssociated(apIndex, &assoc_num);
	if (assoc_num == 0) {
		HAL_WIFI_DBG(("%s: apIndex=%d, numberOfAssociatedDevices=0\n", __FUNCTION__,
			apIndex));
		return RETURN_OK;
	}

	ret = wifi_getApMaxAssociatedDevices(apIndex, &max_assoc_devices);

	if ((ret != RETURN_OK) || (!max_assoc_devices)) {
		max_assoc_devices = MAX_ASSOC_DEVICES;
	}

	len = (max_assoc_devices) * sizeof(wldm_wifi_associated_dev1_t);
	p_wldm_array = (wldm_wifi_associated_dev1_t *)malloc(len);
	if (!p_wldm_array) {
		HAL_WIFI_ERR(("%s:%d Malloc failed\n", __FUNCTION__, __LINE__));
		return RETURN_ERR;
	}

	memset(p_wldm_array, 0, len);
	len = max_assoc_devices; /* Pass the # of allocated as a input */
	ret = wldm_AccessPoint_AssociatedDevice(CMD_GET, apIndex, DIAG_RESULT_1,
		(void *)p_wldm_array, &len, NULL, NULL);

	if (ret < 0) {
		free(p_wldm_array);
		return RETURN_ERR;
	}

	parray = (wifi_associated_dev_t *)malloc(len * sizeof(wifi_associated_dev_t));
	if (!parray) {
		HAL_WIFI_ERR(("%s:%d Malloc failed\n", __FUNCTION__, __LINE__));
		free(p_wldm_array);
		return RETURN_ERR;
	}

	memset(parray, 0, len * sizeof(wifi_associated_dev_t));
	*associated_dev_array = parray;
	for (i = 0; i < len; ++i) {
		memcpy(parray[i].cli_MACAddress, p_wldm_array[i].cli_MACAddress, ETHER_ADDR_LEN);
		parray[i].cli_AuthenticationState = p_wldm_array[i].cli_AuthenticationState;
		parray[i].cli_LastDataDownlinkRate = p_wldm_array[i].cli_LastDataDownlinkRate;
		parray[i].cli_LastDataUplinkRate = p_wldm_array[i].cli_LastDataUplinkRate;
		parray[i].cli_SignalStrength = p_wldm_array[i].cli_SignalStrength;
		parray[i].cli_Retransmissions = p_wldm_array[i].cli_Retransmissions;
		parray[i].cli_Active = p_wldm_array[i].cli_Active;
		snprintf(parray[i].cli_OperatingStandard, sizeof(parray[i].cli_OperatingStandard),
			"%s", p_wldm_array[i].cli_OperatingStandard);
		snprintf(parray[i].cli_OperatingChannelBandwidth,
			sizeof(parray[i].cli_OperatingChannelBandwidth), "%s",
			p_wldm_array[i].cli_OperatingChannelBandwidth);
		parray[i].cli_SNR = p_wldm_array[i].cli_SNR;
		parray[i].cli_BytesSent = p_wldm_array[i].cli_BytesSent;
		parray[i].cli_BytesReceived = p_wldm_array[i].cli_BytesReceived;
		parray[i].cli_RSSI = p_wldm_array[i].cli_RSSI;
		parray[i].cli_DataFramesSentAck = p_wldm_array[i].cli_DataFramesSentAck;
		parray[i].cli_DataFramesSentNoAck = p_wldm_array[i].cli_DataFramesSentNoAck;
	}

	*output_array_size = len;
	free(p_wldm_array);
	return RETURN_OK;
}

INT wifi_getApAssociatedDeviceDiagnosticResult2(INT apIndex, wifi_associated_dev2_t **associated_dev_array, UINT *output_array_size)
{
	unsigned long assoc_num;
	unsigned int max_assoc_devices, len;
	int ret;
	unsigned int i;
	wifi_associated_dev2_t *parray;
	wldm_wifi_associated_dev2_t *p_wldm_array;

	NULL_PTR_ASSERT(output_array_size);
	AP_INDEX_ASSERT(apIndex);

	*output_array_size = 0;
	*associated_dev_array = NULL;

	wifi_getApNumDevicesAssociated(apIndex, &assoc_num);
	if (assoc_num == 0) {
		HAL_WIFI_DBG(("%s: apIndex=%d, numberOfAssociatedDevices=0\n", __FUNCTION__,
			apIndex));
		return RETURN_OK;
	}

	ret = wifi_getApMaxAssociatedDevices(apIndex, &max_assoc_devices);
	if ((ret != RETURN_OK) || (!max_assoc_devices)) {
		max_assoc_devices = MAX_ASSOC_DEVICES;
	}

	len = (max_assoc_devices) * sizeof(wldm_wifi_associated_dev2_t);
	p_wldm_array = (wldm_wifi_associated_dev2_t *)malloc(len);
	if (!p_wldm_array) {
		HAL_WIFI_ERR(("%s:%d Malloc failed\n", __FUNCTION__, __LINE__));
		return RETURN_ERR;
	}

	memset(p_wldm_array, 0, len);
	len = max_assoc_devices; /* Pass the # of allocated as a input */
	ret = wldm_AccessPoint_AssociatedDevice(CMD_GET, apIndex, DIAG_RESULT_2,
		(void *)p_wldm_array, &len, NULL, NULL);

	if (ret < 0) {
		free(p_wldm_array);
		return RETURN_ERR;
	}
	parray = (wifi_associated_dev2_t *)malloc(len * sizeof(wifi_associated_dev2_t));
	if (!parray) {
		HAL_WIFI_ERR(("%s:%d Malloc failed\n", __FUNCTION__, __LINE__));
		free(p_wldm_array);
		return RETURN_ERR;
	}

	memset(parray, 0, len * sizeof(wifi_associated_dev2_t));
	*associated_dev_array = parray;
	for (i = 0; i < len; ++i) {
		memcpy(parray[i].cli_MACAddress, p_wldm_array[i].cli_MACAddress, ETHER_ADDR_LEN);
		parray[i].cli_AuthenticationState = p_wldm_array[i].cli_AuthenticationState;
		parray[i].cli_LastDataDownlinkRate = p_wldm_array[i].cli_LastDataDownlinkRate;
		parray[i].cli_LastDataUplinkRate = p_wldm_array[i].cli_LastDataUplinkRate;
		parray[i].cli_SignalStrength = p_wldm_array[i].cli_SignalStrength;
		parray[i].cli_Retransmissions = p_wldm_array[i].cli_Retransmissions;
		parray[i].cli_Active = p_wldm_array[i].cli_Active;
		snprintf(parray[i].cli_OperatingStandard, sizeof(parray[i].cli_OperatingStandard),
			"%s", p_wldm_array[i].cli_OperatingStandard);
		snprintf(parray[i].cli_OperatingChannelBandwidth,
			sizeof(parray[i].cli_OperatingChannelBandwidth), "%s",
			p_wldm_array[i].cli_OperatingChannelBandwidth);
		parray[i].cli_SNR = p_wldm_array[i].cli_SNR;
		parray[i].cli_BytesSent = p_wldm_array[i].cli_BytesSent;
		parray[i].cli_BytesReceived = p_wldm_array[i].cli_BytesReceived;
		parray[i].cli_RSSI = p_wldm_array[i].cli_RSSI;
		parray[i].cli_DataFramesSentAck = p_wldm_array[i].cli_DataFramesSentAck;
		parray[i].cli_DataFramesSentNoAck = p_wldm_array[i].cli_DataFramesSentNoAck;
		parray[i].cli_Associations = p_wldm_array[i].cli_Associations;
	}
	*output_array_size = len;
	free(p_wldm_array);
	return RETURN_OK;
}

typedef struct wifi_params_mcs_rate {
	char		*oper_standard;	/* Operating standard - n/ac/ax */
	char		*bw;		/* bandwidth 20/40/80/160/320 */
	int		gi;		/* guard interval 400nsec/Auto = 1 800nsec = -1*/
	unsigned int	nss;		/* NSS */
	unsigned int	rate;		/* Max uplink/downlink rate */
} wifi_params_mcs_rate_t;

/* Values in the table below are from
* N: 802.11 spec Section 19.5 Parameters for HT MCSs Pages 2427 - 2431
* AC: 802.11 spec Section 21.5 Parameters for VHT-MCSs Pages: 2608 - 2624
* AC max rates accounting for mcs 11 based on
* nss:4 - 20Mhz LGI - 433, 20Mhz SGI - 481, 40Mhz LGI - 900, 40Mhz SGI - 1000,
* 80Mhz LGI - 1950, 80Mhz SGI - 21667. Calculate nssx_rate = nss4_rate*x/4
* AX: Draft IEEE P802.11ax/D6.0 Section 27.5 Parameters for HE-MCSs Pages: 702 - 721
* BE: Draft IEEE p802.11be D4.1 */
wifi_params_mcs_rate_t mcs_rate_tbl[] = {
	/* Standard,	BW,	GI,	NSS,	Rate */
	{"n",		"20",	1,	1,	72},
	{"n",		"20",	1,	2,	144},
	{"n",		"20",	1,	3,	217},
	{"n",		"20",	1,	4,	289},

	{"n",		"20",	-1,	1,	65},
	{"n",		"20",	-1,	2,	130},
	{"n",		"20",	-1,	3,	195},
	{"n",		"20",	-1,	4,	260},

	{"n",		"40",	1,	1,	150},
	{"n",		"40",	1,	2,	300},
	{"n",		"40",	1,	3,	450},
	{"n",		"40",	1,	4,	600},

	{"n",		"40",	-1,	1,	135},
	{"n",		"40",	-1,	2,	270},
	{"n",		"40",	-1,	3,	405},
	{"n",		"40",	-1,	4,	540},

	{"ac",		"20",	1,	1,	120},
	{"ac",		"20",	1,	2,	241},
	{"ac",		"20",	1,	3,	361},
	{"ac",		"20",	1,	4,	481},
	{"ac",		"20",	1,	5,	601},
	{"ac",		"20",	1,	6,	722},
	{"ac",		"20",	1,	7,	842},
	{"ac",		"20",	1,	8,	962},

	{"ac",		"20",	-1,	1,	108},
	{"ac",		"20",	-1,	2,	217},
	{"ac",		"20",	-1,	3,	325},
	{"ac",		"20",	-1,	4,	433},
	{"ac",		"20",	-1,	5,	541},
	{"ac",		"20",	-1,	6,	650},
	{"ac",		"20",	-1,	7,	758},
	{"ac",		"20",	-1,	8,	866},

	{"ac",		"40",	1,	1,	250},
	{"ac",		"40",	1,	2,	500},
	{"ac",		"40",	1,	3,	750},
	{"ac",		"40",	1,	4,	1000},
	{"ac",		"40",	1,	5,	1250},
	{"ac",		"40",	1,	6,	1500},
	{"ac",		"40",	1,	7,	1750},
	{"ac",		"40",	1,	8,	2000},

	{"ac",		"40",	-1,	1,	225},
	{"ac",		"40",	-1,	2,	450},
	{"ac",		"40",	-1,	3,	675},
	{"ac",		"40",	-1,	4,	900},
	{"ac",		"40",	-1,	5,	1125},
	{"ac",		"40",	-1,	6,	1350},
	{"ac",		"40",	-1,	7,	1575},
	{"ac",		"40",	-1,	8,	1800},

	{"ac",		"80",	1,	1,	542},
	{"ac",		"80",	1,	2,	1084},
	{"ac",		"80",	1,	3,	1625},
	{"ac",		"80",	1,	4,	2167},
	{"ac",		"80",	1,	5,	2709},
	{"ac",		"80",	1,	6,	3251},
	{"ac",		"80",	1,	7,	3792},
	{"ac",		"80",	1,	8,	4334},

	{"ac",		"80",	-1,	1,	488},
	{"ac",		"80",	-1,	2,	975},
	{"ac",		"80",	-1,	3,	1463},
	{"ac",		"80",	-1,	4,	1950},
	{"ac",		"80",	-1,	5,	2438},
	{"ac",		"80",	-1,	6,	2925},
	{"ac",		"80",	-1,	7,	3413},
	{"ac",		"80",	-1,	8,	3900},

	{"ac",		"160",	1,	1,	867},
	{"ac",		"160",	1,	2,	1733},
	{"ac",		"160",	1,	3,	2340},
	{"ac",		"160",	1,	4,	3467},
	{"ac",		"160",	1,	5,	4333},
	{"ac",		"160",	1,	6,	5200},
	{"ac",		"160",	1,	7,	6067},
	{"ac",		"160",	1,	8,	6933},

	{"ac",		"160",	-1,	1,	780},
	{"ac",		"160",	-1,	2,	1560},
	{"ac",		"160",	-1,	3,	2106},
	{"ac",		"160",	-1,	4,	3120},
	{"ac",		"160",	-1,	5,	3900},
	{"ac",		"160",	-1,	6,	4680},
	{"ac",		"160",	-1,	7,	5460},
	{"ac",		"160",	-1,	8,	6240},

	{"ax",		"20",	1,	1,	143},
	{"ax",		"20",	1,	2,	287},
	{"ax",		"20",	1,	3,	430},
	{"ax",		"20",	1,	4,	574},
	{"ax",		"20",	1,	5,	717},
	{"ax",		"20",	1,	6,	860},
	{"ax",		"20",	1,	7,	1004},
	{"ax",		"20",	1,	8,	1147},

	{"ax",		"40",	1,	1,	287},
	{"ax",		"40",	1,	2,	574},
	{"ax",		"40",	1,	3,	860},
	{"ax",		"40",	1,	4,	1147},
	{"ax",		"40",	1,	5,	1434},
	{"ax",		"40",	1,	6,	1721},
	{"ax",		"40",	1,	7,	2007},
	{"ax",		"40",	1,	8,	2294},

	{"ax",		"80",	1,	1,	600},
	{"ax",		"80",	1,	2,	1201},
	{"ax",		"80",	1,	3,	1802},
	{"ax",		"80",	1,	4,	2402},
	{"ax",		"80",	1,	5,	3002},
	{"ax",		"80",	1,	6,	3603},
	{"ax",		"80",	1,	7,	4203},
	{"ax",		"80",	1,	8,	4804},

	{"ax",		"160",	1,	1,	1201},
	{"ax",		"160",	1,	2,	2402},
	{"ax",		"160",	1,	3,	3603},
	{"ax",		"160",	1,	4,	4804},
	{"ax",		"160",	1,	5,	6005},
	{"ax",		"160",	1,	6,	7206},
	{"ax",		"160",	1,	7,	8407},
	{"ax",		"160",	1,	8,	9608},

};

static INT get_assoc_dev_max_tx_rx_rate(INT apIndex, unsigned int nss, wifi_associated_dev3_t *dev)
{
	char guard[OUTPUT_STRING_LENGTH_64], mac_str[MAC_STR_LEN];
	int gi = 1, i, tbl_size;

	dev->cli_MaxDownlinkRate = 0;
	dev->cli_MaxUplinkRate = 0;

	snprintf(mac_str, sizeof(mac_str), MACF, MAC_TO_MACF(dev->cli_MACAddress));

	if (strcmp(dev->cli_OperatingStandard, "a") == 0) {
		dev->cli_MaxDownlinkRate = 54;
		dev->cli_MaxUplinkRate = 54;
	} else if (strcmp(dev->cli_OperatingStandard, "b") == 0) {
		dev->cli_MaxDownlinkRate = 11;
		dev->cli_MaxUplinkRate = 11;
	} else if (strcmp(dev->cli_OperatingStandard, "g") == 0) {
		dev->cli_MaxDownlinkRate = 54;
		dev->cli_MaxUplinkRate = 54;
	} else {
		memset(guard, 0, sizeof(guard));

		if (wifi_getRadioGuardInterval(HAL_AP_IDX_TO_HAL_RADIO(apIndex), guard) != RETURN_OK) {
			HAL_WIFI_DBG(("%s:%d Error getting guard interval\n", __func__, __LINE__));
		}
		if ((strcmp(guard, "800nsec") == 0)) {
			gi = -1;
		}
		HAL_WIFI_DBG(("%s: Operating Standard: %s Operating bandwidth: %s guard: %s = %d NSS:%d MAC %s\n",
			__FUNCTION__, dev->cli_OperatingStandard,
			dev->cli_OperatingChannelBandwidth, guard, gi, nss, mac_str));

		tbl_size = sizeof(mcs_rate_tbl) / sizeof(mcs_rate_tbl[0]);
#ifdef WIFI_HAL_VERSION_GE_3_0_1
		dev->cli_activeNumSpatialStreams = nss;
#endif /* WIFI_HAL_VERSION_GE_3_0_1 */
		for (i = 0; i < tbl_size; i++) {
			if (!strcmp(mcs_rate_tbl[i].oper_standard, dev->cli_OperatingStandard) &&
				!strcmp(mcs_rate_tbl[i].bw, dev->cli_OperatingChannelBandwidth) &&
				mcs_rate_tbl[i].gi == gi && mcs_rate_tbl[i].nss == nss) {
				dev->cli_MaxUplinkRate = mcs_rate_tbl[i].rate;
				dev->cli_MaxDownlinkRate = mcs_rate_tbl[i].rate;
				return RETURN_OK;
			}
		}
		HAL_WIFI_ERR(("%s: No match found Operating Standard:%s BW:%s GI:%d NSS:%d\n",
			__FUNCTION__, dev->cli_OperatingStandard,
			dev->cli_OperatingChannelBandwidth, gi, nss));
		return RETURN_ERR;
	}
	return RETURN_OK;
}

#define COMCAST_BLASTER_DEFAULT_PKTSIZE	1470

INT wifi_getApAssociatedDeviceDiagnosticResult3(INT apIndex,
	wifi_associated_dev3_t **associated_dev_array, UINT *output_array_size)
{
	unsigned long assoc_num;
	unsigned int max_assoc_devices, len;
	int ret;
	unsigned int i;
	wifi_associated_dev3_t *parray;
	wldm_wifi_associated_dev3_t *p_wldm_array;

	NULL_PTR_ASSERT(output_array_size);
	AP_INDEX_ASSERT(apIndex);

	*output_array_size = 0;
	*associated_dev_array = NULL;

	wifi_getApNumDevicesAssociated(apIndex, &assoc_num);
	if (assoc_num == 0) {
		HAL_WIFI_DBG(("%s apIndex=%d, numberOfAssociatedDevices=0\n", __FUNCTION__,
			apIndex));
		return RETURN_OK;
	}

	ret = wifi_getApMaxAssociatedDevices(apIndex, &max_assoc_devices);
	if ((ret != RETURN_OK) || (!max_assoc_devices)) {
		max_assoc_devices = MAX_ASSOC_DEVICES;
	}

	len = (max_assoc_devices) * sizeof(wldm_wifi_associated_dev3_t);
	p_wldm_array = (wldm_wifi_associated_dev3_t *)malloc(len);
	if (!p_wldm_array) {
		HAL_WIFI_ERR(("%s:%d Malloc failed\n", __FUNCTION__, __LINE__));
		return RETURN_ERR;
	}

	memset(p_wldm_array, 0, len);
	len = max_assoc_devices; /* Pass the # of allocated as a input */
	ret = wldm_AccessPoint_AssociatedDevice(CMD_GET, apIndex, DIAG_RESULT_3,
		(void *)p_wldm_array, &len, NULL, NULL);

	if (ret < 0) {
		free(p_wldm_array);
		return RETURN_ERR;
	}

	parray = (wifi_associated_dev3_t *)malloc(len * sizeof(wifi_associated_dev3_t));
	if (!parray) {
		HAL_WIFI_ERR(("%s:%d Malloc failed\n", __FUNCTION__, __LINE__));
		free(p_wldm_array);
		return RETURN_ERR;
	}

	memset(parray, 0, len * sizeof(wifi_associated_dev3_t));
	*associated_dev_array = parray;
	*output_array_size = len;

	for (i = 0; i < len; ++i) {
		memcpy(parray[i].cli_MACAddress, p_wldm_array[i].cli_MACAddress, ETHER_ADDR_LEN);
		parray[i].cli_AuthenticationState = p_wldm_array[i].cli_AuthenticationState;
		parray[i].cli_LastDataDownlinkRate = p_wldm_array[i].cli_LastDataDownlinkRate;
		parray[i].cli_LastDataUplinkRate = p_wldm_array[i].cli_LastDataUplinkRate;
		parray[i].cli_SignalStrength = p_wldm_array[i].cli_SignalStrength;
		parray[i].cli_Retransmissions = p_wldm_array[i].cli_Retransmissions;
		parray[i].cli_Active = p_wldm_array[i].cli_Active;
		snprintf(parray[i].cli_OperatingStandard, sizeof(parray[i].cli_OperatingStandard),
			"%s", p_wldm_array[i].cli_OperatingStandard);
		snprintf(parray[i].cli_OperatingChannelBandwidth,
			sizeof(parray[i].cli_OperatingChannelBandwidth), "%s",
			p_wldm_array[i].cli_OperatingChannelBandwidth);
		parray[i].cli_SNR = p_wldm_array[i].cli_SNR;
		parray[i].cli_BytesSent = p_wldm_array[i].cli_BytesSent;
		parray[i].cli_BytesReceived = p_wldm_array[i].cli_BytesReceived;
		parray[i].cli_RSSI = p_wldm_array[i].cli_RSSI;
                parray[i].cli_DataFramesSentNoAck = p_wldm_array[i].cli_DataFramesSentNoAck;
#ifdef CMWIFI_RDKB_COMCAST
		/*
		 * Assume the default packet size for wifi blaster is 1470
		 * Sometimes when the AP is just up, the p_wldm_array[i].cli_BytesSent
		 * is very low as just a couple of frames have been sent and not real data.
		 * In this case p_wldm_array[i].cli_BytesSent / COMCAST_BLASTER_DEFAULT_PKTSIZE
		 * will be 1 or 2 or another low value which is in fact lower than
		 * p_wldm_array[i].cli_PacketsSent.
		 * So check for this first
		 * TODO: A check whether the procfs can be used will be done later as currently
		 * it doesn't read wlx.y, it only parses wlx instead.
		 */
		if ((p_wldm_array[i].cli_BytesSent / COMCAST_BLASTER_DEFAULT_PKTSIZE) <
				p_wldm_array[i].cli_PacketsSent) {
			parray[i].cli_DataFramesSentAck = p_wldm_array[i].cli_PacketsSent -
				parray[i].cli_DataFramesSentNoAck;
		}
		else {
			parray[i].cli_DataFramesSentAck =
				(p_wldm_array[i].cli_BytesSent / COMCAST_BLASTER_DEFAULT_PKTSIZE) -
					parray[i].cli_DataFramesSentNoAck;
		}
		parray[i].cli_PacketsSent = parray[i].cli_DataFramesSentAck +
			parray[i].cli_DataFramesSentNoAck;
#else
		parray[i].cli_DataFramesSentAck = p_wldm_array[i].cli_DataFramesSentAck;
		parray[i].cli_PacketsSent = p_wldm_array[i].cli_PacketsSent;
#endif /* CMWIFI_RDKB_COMCAST */
		parray[i].cli_Associations = p_wldm_array[i].cli_Associations;
		parray[i].cli_PacketsReceived = p_wldm_array[i].cli_PacketsReceived;
		parray[i].cli_ErrorsSent = p_wldm_array[i].cli_ErrorsSent;
		parray[i].cli_RetransCount = p_wldm_array[i].cli_RetransCount;
		parray[i].cli_FailedRetransCount = p_wldm_array[i].cli_FailedRetransCount;
		parray[i].cli_RetryCount = p_wldm_array[i].cli_RetryCount;
#ifdef WIFI_HAL_VERSION_GE_3_0_2
		parray[i].cli_TxFrames = p_wldm_array[i].cli_TxFrames;
		parray[i].cli_RxRetries = p_wldm_array[i].cli_RxRetries;
		parray[i].cli_RxErrors = p_wldm_array[i].cli_RxErrors;
#endif /* WIFI_HAL_VERSION_GE_3_0_2 */

		get_assoc_dev_max_tx_rx_rate(apIndex, p_wldm_array[i].cli_Nss, &parray[i]);
#ifdef WIFI_HAL_VERSION_3
		/* TBD cli_DownlinkMuStats cli_UplinkMuStats */
		/* cli_TwtParams */
		parray[i].cli_TwtParams.numTwtSession = p_wldm_array[i].cli_TwtParams.numTwtSession;
		memcpy((char *)(parray[i].cli_TwtParams.twtParams),
			(char *)(p_wldm_array[i].cli_TwtParams.twtParams),
			(parray[i].cli_TwtParams.numTwtSession) * sizeof(wifi_twt_params_t));
#endif /* WIFI_HAL_VERSION_3 */
	}

	free(p_wldm_array);
	return RETURN_OK;
}

INT wifi_getApSecurityMFPConfig(INT apIndex, CHAR *MfpConfig)
{
	CHAR mfpConf[OUTPUT_STRING_LENGTH_64];
	int returnStatus, len = sizeof(mfpConf);
	memset(mfpConf, 0, OUTPUT_STRING_LENGTH_64);
	HAL_WIFI_DBG(("%s: apIndex = %d\n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(MfpConfig);

	MfpConfig[0] = '\0';

	returnStatus = wldm_AccessPoint_Security_MFPConfig(CMD_GET, apIndex, mfpConf, &len, NULL, NULL);
	if (returnStatus != 0) {
		HAL_WIFI_DBG(("%s: wldm_AccessPoint_Security_MFPConfig CMD_GET Failed, Status=%d\n",
			__FUNCTION__, returnStatus));
		return RETURN_ERR;
	}

	snprintf(MfpConfig, OUTPUT_STRING_LENGTH_64, "%s", mfpConf);
	HAL_WIFI_DBG(("%s: Done,  apIndex = %d, MfpConfig=%s\n", __FUNCTION__, apIndex, MfpConfig));

	return RETURN_OK;
}

INT wifi_setApSecurityMFPConfig(INT apIndex, CHAR *MfpConfig)
{
	HAL_WIFI_DBG(("%s: apIndex = %d & MfpConfig=%s \n", __FUNCTION__, apIndex, MfpConfig));

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(MfpConfig);

	int len = strlen(MfpConfig);
	int returnStatus;

	returnStatus =  wldm_AccessPoint_Security_MFPConfig(CMD_SET, apIndex, MfpConfig, &len, NULL, NULL);
	if (returnStatus != RETURN_OK){
		HAL_WIFI_DBG(("%s: wldm_AccessPoint_Security_MFPConfig CMD_SET Failed, apIndex=[%d], Status=[%d]\n",
			__FUNCTION__, apIndex, returnStatus));
		return RETURN_ERR;
	}
	HAL_WIFI_DBG(("%s: AP Index [%d]\n", __FUNCTION__, apIndex));
#ifdef RDKB_TIMER
	wifi_set_timer_variable();
#endif
	return RETURN_OK;
}

INT wifi_ifConfigUp(INT apIndex)
{
	char cmd[128];
	char buf[1024];

	HAL_WIFI_LOG(("%s: apIndex: %d\n", __FUNCTION__, apIndex));
	AP_INDEX_ASSERT(apIndex);

	snprintf(cmd, sizeof(cmd), "ifconfig %s%d up 2>/dev/null", AP_PREFIX, apIndex);
	_syscmd(cmd, buf, sizeof(buf));
	return 0;
}

INT wifi_ifConfigDown(INT apIndex)
{
	char cmd[128], buf[1024];

	HAL_WIFI_LOG(("%s: apIndex: %d\n", __FUNCTION__, apIndex));
	AP_INDEX_ASSERT(apIndex);

	snprintf(cmd, sizeof(cmd), "ifconfig %s%d down 2>/dev/null", AP_PREFIX, apIndex);
	_syscmd(cmd, buf, sizeof(buf));
	return 0;
}

//>> Deprecated. Replace with wifi_applyRadioSettings
INT wifi_pushBridgeInfo(INT apIndex)
{
	HAL_WIFI_DBG(("%s: apIndex: %d\n", __FUNCTION__, apIndex));

	/* Not Applicable */
	return RETURN_OK;
}

// Applying changes with wifi_applyRadioSettings(), this will also apply all other cached setting together with channel mode into driver.
// It may be different with other chp vendor.
INT wifi_pushRadioChannelMode(INT radioIndex)
{
	UNUSED_PARAMETER(radioIndex);
	HAL_WIFI_DBG(("%s: Stub function\n", __FUNCTION__));

	return RETURN_OK;
}

INT wifi_pushDefaultValues(INT radioIndex)
{
	//Apply Comcast specified default radio settings instantly
	//AMPDU = 1
	//AMPDUFrames = 32
	//AMPDULim = 50000
	//txqueuelen = 1000
	UNUSED_PARAMETER(radioIndex);
	return RETURN_ERR;
}

INT wifi_pushRadioTxChainMask(INT radioIndex)
{
	//Apply default TxChainMask instantly
	UNUSED_PARAMETER(radioIndex);
	return RETURN_ERR;
}

INT wifi_pushRadioRxChainMask(INT radioIndex)
{
	//Apply default RxChainMask instantly
	UNUSED_PARAMETER(radioIndex);
	return RETURN_ERR;
}
//<<

// Applying changes with wifi_applyRadioSettings(), this will apply all other cached setting together with SSID into driver.
// It may be different with other chp vendor.
INT wifi_pushSSID(INT apIndex, CHAR *ssid)
{
	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(ssid);

	if (wifi_setSSIDName(apIndex, ssid) != RETURN_OK) {
		HAL_WIFI_ERR(("%s, apIndex = %d, failed!!\n", __FUNCTION__, apIndex));
	}
	HAL_WIFI_DBG(("%s: apIndex = %d wldm_apply_SSIDObject newSSID=%s\n",
		__FUNCTION__, apIndex, ssid));
	if (wldm_apply_SSIDObject(apIndex) != 0) {
		HAL_WIFI_ERR(("%s, apply Failed apIndex [%d]\n", __FUNCTION__, apIndex));
		return RETURN_ERR;
	}
	return RETURN_OK;
}

//Apply default Ssid Advertisement instantly
INT wifi_pushSsidAdvertisementEnable(INT apIndex, BOOL enable)
{
	int returnStatus, len = sizeof(enable);

	AP_INDEX_ASSERT(apIndex);

	returnStatus = wldm_AccessPoint_SSIDAdvertisementEnabled(CMD_SET_IOCTL,
			apIndex, &enable, &len, NULL, NULL);
	if (returnStatus != 0) {
		HAL_WIFI_ERR(("%s: wldm_AccessPoint_SSIDAdvertisementEnabled failed, Status = %d\n",
			__FUNCTION__, returnStatus));
		return RETURN_ERR;
	}
	return RETURN_OK;
}

INT wifi_getRadioUpTime(INT radioIndex, ULONG *output)
{
	int returnStatus = RETURN_ERR;
	int len;
	unsigned int upSec;

	NULL_PTR_ASSERT(output);
	RADIO_INDEX_ASSERT(radioIndex);

	len = sizeof(upSec);
	returnStatus = wldm_Radio_LastChange(CMD_GET, radioIndex, &upSec, &len, NULL, NULL);
	*output = upSec;
	if (returnStatus < 0) {
		HAL_WIFI_ERR(("%s, Err ret=%d from wldm_Radio_LastChange radioIndex=%d\n",
			__FUNCTION__, returnStatus, radioIndex));
		return RETURN_ERR;
	} else {
		HAL_WIFI_DBG(("%s, radioIndex=%d  output is %d secs\n",
			__FUNCTION__, radioIndex, *output));
		return RETURN_OK;
	}
}

INT wifi_setLED(INT radioIndex, BOOL enable)
{
	int ret;

	RADIO_INDEX_ASSERT(radioIndex);
	ret = wifi_setLED_priv(radioIndex,
		            enable ? WIFI_LED_ON_CONTINUOUS : WIFI_LED_OFF_CONTINUOUS);

	if (ret < 0) {
		HAL_WIFI_ERR(("%s: Failed to set LED\n", __FUNCTION__));
		return RETURN_ERR;
	}

	return ret;
}

//Set the system LED status
INT wifi_setLED_priv(INT radioIndex, wifi_led_blink_state_t led_state) //RDKB
{
	char cmd[1024];
	FILE *fp = NULL;
	char file_name[1024];
	char line[1024];
	int wifi_driver_led_state = 0;
	char tmpStr[10] = "";

	if (wifi_led_gpio == -1) {
		snprintf(file_name, sizeof(file_name), "/nvram/provisioned_wifi_wl%d_vars.txt", radioIndex);
		wifi_led_gpio = -2;

		if (!(fp = fopen(file_name, "r"))) {
			snprintf(file_name, sizeof(file_name),
				"/data/provisioned_wifi_wl%d_vars.txt", radioIndex);
			fp = fopen(file_name, "r");
		}
		if (fp) {
			while (fgets(line, sizeof(line), fp)!=NULL) {
				if (sscanf(line,"ledbh%d%10s",&wifi_led_gpio,tmpStr)) {
					if (strstr(tmpStr,"=0x")) {
						break;
					} else {
						wifi_led_gpio = -2;
					}
				}
			}

			fclose(fp);
			fp = NULL;
		} else {
			HAL_WIFI_DBG(("wifi_setLED_priv failed. no file %s to find out GPIO\n",file_name));
		}
		/* temp WAR if not ledbh defined in provision file */
		if (wifi_led_gpio == -2) {
			wifi_led_gpio = 10; /* use default gpio 10 */
		}
	}

	if (wifi_led_gpio < 0) {
		HAL_WIFI_DBG(("wifi_setLED_priv: failed. no entry with ledbh to findout GPIO number \n"));
		return RETURN_ERR;
	}

	if (led_state == wifi_prev_led_state[radioIndex]) {
		return RETURN_OK;
	}

	wifi_prev_led_state[radioIndex] = led_state;
	switch(led_state) {
		case WIFI_LED_OFF_CONTINUOUS:
			wifi_driver_led_state = 0;
			break;

		case WIFI_LED_ON_CONTINUOUS:
			wifi_driver_led_state = 1;
			break;

		case WIFI_LED_SLOW_CONTINUOUS_BLINK:
			wifi_driver_led_state = 15;
			break;

		case WIFI_LED_MEDIUM_CONTINUOUS_BLINK:
			wifi_driver_led_state = 18;
			break;

		case WIFI_LED_FAST_CONTINUOUS_BLINK:
			wifi_driver_led_state = 16;
			break;

		case WIFI_LED_OFF_AND_FAST_BLINK:
			wifi_driver_led_state = 2;
			break;

		case WIFI_LED_OFF_AND_SLOW_BLINK:
			wifi_driver_led_state = 19;
			break;

		default:
			HAL_WIFI_ERR(("wifi_setLED_priv : Wrong led state=%d\n",led_state));
			return RETURN_ERR;
	}

	/* DON'T set LEDBH to 0 to turn off LED; Set LEDBH to 21 instead */
	/* LEDBH  1: ON	*/
	/* LEDBH 21: OFF */
	/* LEDBH  0: Special NULL function used to disable GPIO at the time of wlc_led_init */
	snprintf(cmd, sizeof(cmd), "wl -i wl%d ledbh %d %d", ((radioIndex == 0) ? 0 : 1), wifi_led_gpio, ((wifi_driver_led_state == 0) ? 21 : 1));
	HAL_WIFI_DBG(("wifi_setLED_priv: radio=%d, state=%s, cmd = %s\n",radioIndex,wifi_led_blink_str[led_state],cmd));
	if (system(cmd)) {
		HAL_WIFI_ERR(("wifi_setLED_priv system cmd %s failed\n", cmd));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

static INT wifi_CheckAndConfigureLEDS(void)
{
	INT ret24Radio = RETURN_ERR, ret5Radio = RETURN_ERR;
	BOOL Is24GHzradioEnabled = FALSE, Is5GHzradioEnabled = FALSE;

	ret24Radio = wifi_getRadioEnable(0, &Is24GHzradioEnabled);
	ret5Radio = wifi_getRadioEnable(1, &Is5GHzradioEnabled);

	HAL_WIFI_DBG(("%s: Radio 2.4GHz ret:%d Status:%d\n",__FUNCTION__, ret24Radio, Is24GHzradioEnabled));
	HAL_WIFI_DBG(("%s: Radio 5GHz ret:%d Status:%d\n",__FUNCTION__, ret5Radio,Is5GHzradioEnabled));

	if (((RETURN_OK == ret24Radio) && (TRUE == Is24GHzradioEnabled)) || \
		((RETURN_OK == ret5Radio) && (TRUE == Is5GHzradioEnabled))) {
		wifi_setLED(0, TRUE);
		wifi_setLED(1, TRUE);
	}

	if ((RETURN_OK == ret24Radio) && (RETURN_OK == ret5Radio)) {
		if ((FALSE == Is24GHzradioEnabled) && (FALSE == Is5GHzradioEnabled)) {
			wifi_setLED(0, FALSE);
			wifi_setLED(1, FALSE);
		}
	}

	return RETURN_OK;
}

// outputs the country code to a max 64 character string
INT wifi_getRadioCountryCode(INT radioIndex, CHAR *output_string)
{
	int len = OUTPUT_STRING_LENGTH_64;
	int returnStatus;

	HAL_WIFI_DBG(("%s: Enter, radioIndex = %d\n", __FUNCTION__, radioIndex));
	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(output_string);

	output_string[0] = '\0';
	returnStatus = wldm_Radio_RegulatoryDomain(CMD_GET, radioIndex, output_string, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_Radio_RegulatoryDomain CMD_GET Failed, Status = %d\n",
			__FUNCTION__, returnStatus));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: radioIndex = %d, output_string = %s\n", __FUNCTION__, radioIndex, output_string));
	return RETURN_OK;
}

INT wifi_setRadioCountryCode(INT radioIndex, CHAR *CountryCode)
{
	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(CountryCode);

	int len = strlen(CountryCode);
	int returnStatus;

	returnStatus = wldm_Radio_RegulatoryDomain(CMD_SET, radioIndex, (CHAR *)CountryCode, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_Radio_RegulatoryDomain CMD_SET Failed, returnStatus = %d\n",
			__FUNCTION__, returnStatus));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s, radioIndex = %d, CountryCode = %s\n", __FUNCTION__, radioIndex, CountryCode));

	return RETURN_OK;
}

//The output_string is a max length 256 octet string that is allocated by the RDKB code.  Implementations must ensure that strings are not longer than this.
//The value of this parameter is a comma seperated list of channel number
INT wifi_getRadioDCSChannelPool(INT radioIndex, CHAR *output_pool) //RDKB
{
	unsigned int len;
	int ret;
	HAL_WIFI_DBG(("%s: Enter", __FUNCTION__));

	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(output_pool);

	HAL_WIFI_DBG(("%s: radioIndex %d", __FUNCTION__, radioIndex));

	len = OUTPUT_STRING_LENGTH_256;
	memset(output_pool, 0, len);
	ret = wldm_xbrcm_scan(CMD_GET_NVRAM, radioIndex, output_pool, &len, "acs_pref_chans", NULL);
	if (ret != 0) {
		HAL_WIFI_ERR(("%s: Error radioIndex=%d\n", __FUNCTION__, radioIndex));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: Done acs_pref_chans radioIndex=%d output_pool=%s\n",
		__FUNCTION__, radioIndex, output_pool ? output_pool : "NULL"));

	return RETURN_OK;
}

INT wifi_setRadioDCSChannelPool(INT radioIndex, CHAR *pool) //RDKB
{
	unsigned int len;
	int ret;
	HAL_WIFI_DBG(("%s: Enter", __FUNCTION__));

	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(pool);

	HAL_WIFI_DBG(("%s: radioIndex %d pool %s", __FUNCTION__, radioIndex, pool));

	len = strlen(pool);
	ret = wldm_xbrcm_scan(CMD_SET_IOCTL | CMD_SET_NVRAM, radioIndex, pool, &len, "acs_pref_chans", NULL);
	if (ret != 0) {
		HAL_WIFI_ERR(("%s: Error radioIndex=%d\n", __FUNCTION__, radioIndex));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: Done radioIndex=%d pool=%s\n", __FUNCTION__, radioIndex, pool));

	return RETURN_OK;
}

INT wifi_getRadioDCSScanTime(INT radioIndex, INT *output_interval_seconds, INT *output_dwell_milliseconds)
{
	int returnStatus;
	int len = sizeof(unsigned int);

	HAL_WIFI_DBG(("%s: Enter, radioIndex = %d\n", __FUNCTION__, radioIndex));

	NULL_PTR_ASSERT(output_interval_seconds);
	NULL_PTR_ASSERT(output_dwell_milliseconds);
	RADIO_INDEX_ASSERT(radioIndex);

	*output_interval_seconds  = 0;
	*output_dwell_milliseconds = 0;
	len = sizeof(*output_interval_seconds);
	returnStatus = wldm_Radio_AutoChannelRefreshPeriod(CMD_GET, radioIndex,
			(unsigned int *) output_interval_seconds, &len, NULL, NULL);
	if (RETURN_OK != returnStatus) {
		HAL_WIFI_DBG(("%s: fail for radioIndex = %d for output_interval_seconds\n",
			__FUNCTION__, radioIndex));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: radioIndex = %d, output_interval_seconds = %lu\n",
		__FUNCTION__, radioIndex, *output_interval_seconds));
	len = sizeof(*output_dwell_milliseconds);
	returnStatus = wldm_Radio_AutoChannelDwellTime(CMD_GET, radioIndex,
				output_dwell_milliseconds, &len, NULL, NULL);
	if (RETURN_OK != returnStatus)
	{
		HAL_WIFI_DBG(("%s: fail for radioIndex = %d for output_dwell_milliseconds\n",
			__FUNCTION__, radioIndex));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: radioIndex = %d, output_dwell_milliseconds = %lu\n",
		__FUNCTION__, radioIndex, *output_dwell_milliseconds));

	return RETURN_OK;
}

INT wifi_setRadioDCSScanTime(INT radioIndex, INT interval_seconds, INT dwell_milliseconds)
{
	int returnStatus;
	int len = sizeof(unsigned int);

	HAL_WIFI_DBG(("%s: Enter, radioIndex = %d\n", __FUNCTION__, radioIndex));

	RADIO_INDEX_ASSERT(radioIndex);

	len = sizeof(interval_seconds);
	returnStatus = wldm_Radio_AutoChannelRefreshPeriod(CMD_SET, radioIndex,
				(unsigned int *) &interval_seconds, &len, NULL, NULL);
	if (RETURN_OK != returnStatus) {
		HAL_WIFI_DBG(("%s: wldm_Radio_AutoChannelRefreshPeriod CMD_SET failed for radioIndex = %d for interval_seconds\n",
			__FUNCTION__, radioIndex));
		return RETURN_ERR;
	}
	HAL_WIFI_DBG(("%s: radioIndex = %d, interval_seconds = %d\n",
		__FUNCTION__, radioIndex, interval_seconds));
	len = sizeof(dwell_milliseconds);
	returnStatus = wldm_Radio_AutoChannelDwellTime(CMD_SET, radioIndex,
			&dwell_milliseconds, &len, NULL, NULL);
	if (RETURN_OK != returnStatus) {
		HAL_WIFI_DBG(("%s: wldm_Radio_AutoChannelDwellTime CMD_SET failed for radioIndex = %d for dwell_milliseconds\n",
			__FUNCTION__, radioIndex));
		return RETURN_ERR;
	}
#ifdef RDKB_TIMER
	wifi_set_timer_variable();
#endif
	HAL_WIFI_DBG(("%s: radioIndex = %d, dwell_milliseconds = %d\n",
		__FUNCTION__, radioIndex, dwell_milliseconds));
	return RETURN_OK;
}

//Set radio traffic static Measureing rules
INT wifi_setRadioTrafficStatsMeasure(INT radioIndex, wifi_radioTrafficStatsMeasure_t *input_struct) //Tr181
{
#ifdef RDKB_RADIO_STATS_MEASURE /* changed to use real-time value by wifi_hal.h */
	char str[8], nv_name[64];
#endif /* RDKB_RADIO_STATS_MEASURE */

	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(input_struct);

#ifdef RDKB_RADIO_STATS_MEASURE /* changed to use real-time value by wifi_hal.h */
	snprintf(str, sizeof(str), "%d", input_struct->radio_RadioStatisticsMeasuringRate);
	snprintf(nv_name, sizeof(nv_name), "wl%d_%s", radioIndex, NVRAM_RADIO_STATS_MEAS_RATE);
	nvram_set(nv_name, str);

	snprintf(str, sizeof(str), "%d", input_struct->radio_RadioStatisticsMeasuringInterval);
	snprintf(nv_name, sizeof(nv_name), "wl%d_%s", radioIndex, NVRAM_RADIO_STATS_MEAS_INTEVAL);
	nvram_set(nv_name, str);
#endif /* RDKB_RADIO_STATS_MEASURE */

	return RETURN_OK;
}

//Device.WiFi.Radio.{i}.Stats.X_COMCAST-COM_RadioStatisticsEnable bool writable
INT wifi_setRadioTrafficStatsRadioStatisticsEnable(INT radioIndex, BOOL enable)
{
	UNUSED_PARAMETER(radioIndex);
	UNUSED_PARAMETER(enable);
	return RETURN_OK;
}

//Radio reset count
INT wifi_getRadioResetCount(INT radioIndex, ULONG *output_int)
{
	NULL_PTR_ASSERT(output_int);
	RADIO_INDEX_ASSERT(radioIndex);

	*output_int = (ULONG) getRadioUpdateCount(radioIndex);

	return RETURN_OK;
}

//Apply default Ssid Advertisement instantly
INT wifi_pushApSsidAdvertisementEnable(INT apIndex, BOOL enable) // push the ssid advertisement enable variable to the hardware //Applying changs with wifi_applyRadioSettings()
{
	HAL_WIFI_DBG(("%s: apIndex = %d, enable = %d\n", __FUNCTION__, apIndex, enable));
	AP_INDEX_ASSERT(apIndex);

	return wifi_pushSsidAdvertisementEnable(apIndex, enable);
}

INT wifi_getApSecuritySecondaryRadiusServer(INT apIndex, CHAR *IP_output, UINT *Port_output, CHAR *RadiusSecret_output) //Tr181
{
	HAL_WIFI_DBG(("%s, apIndex = %d\n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(IP_output);
	NULL_PTR_ASSERT(Port_output);
	NULL_PTR_ASSERT(RadiusSecret_output);

	INT len = OUTPUT_STRING_LENGTH_64;

	if (wldm_AccessPoint_Security_SecondaryRadiusServerIPAddr(CMD_GET, apIndex, IP_output, &len, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: Fail to get IP\n", __FUNCTION__));
		return RETURN_ERR;
	}

	len = sizeof(*Port_output);
	if (wldm_AccessPoint_Security_SecondaryRadiusServerPort(CMD_GET, apIndex, Port_output, &len, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: Fail to get port\n", __FUNCTION__));
		return RETURN_ERR;
	}

	len = OUTPUT_STRING_LENGTH_64;
	if (wldm_AccessPoint_Security_SecondaryRadiusSecret(CMD_GET, apIndex, RadiusSecret_output, &len, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: Fail to get secret\n", __FUNCTION__));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

INT wifi_setApSecuritySecondaryRadiusServer(INT apIndex, CHAR *IPAddress, UINT port, CHAR *RadiusSecret) //Tr181
{
	NULL_PTR_ASSERT(IPAddress);
	NULL_PTR_ASSERT(RadiusSecret);
	AP_INDEX_ASSERT(apIndex);

	INT len = 0;

	len = strlen(IPAddress);
	if (wldm_AccessPoint_Security_SecondaryRadiusServerIPAddr(CMD_SET, apIndex, IPAddress, &len, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: Fail to set IP\n", __FUNCTION__));
		return RETURN_ERR;
	}

	len = sizeof(port);
	if (wldm_AccessPoint_Security_SecondaryRadiusServerPort(CMD_SET, apIndex, &port, &len, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: Fail to set port\n", __FUNCTION__));
		return RETURN_ERR;
	}

	len = strlen(RadiusSecret);
	if (wldm_AccessPoint_Security_SecondaryRadiusSecret(CMD_SET, apIndex, RadiusSecret, &len, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: Fail to set secret\n", __FUNCTION__));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

//Device.WiFi.X_RDKCENTRAL-COM_BandSteering object
//Device.WiFi.X_RDKCENTRAL-COM_BandSteering.Capability bool r/o
//To get Band Steering Capability
INT wifi_getBandSteeringCapability(BOOL *support)
{
	return wldm_xbrcm_bsd(CMD_GET, 0, WLDM_BSD_STEER_CAP, (void *)support, NULL);
}

//Device.WiFi.X_RDKCENTRAL-COM_BandSteering.Enable bool r/w
//To get Band Steering enable status
INT wifi_getBandSteeringEnable(BOOL *enable)
{
	return wldm_xbrcm_bsd(CMD_GET, 0, WLDM_BSD_STEER_ENABLE, (void *)enable, NULL);
}

//To turn on/off Band steering
INT wifi_setBandSteeringEnable(BOOL enable)
{
	return wldm_xbrcm_bsd(CMD_SET, 0, WLDM_BSD_STEER_ENABLE, (void *)&enable, NULL);
}

INT wifi_getRadioBandUtilization (INT radioIndex, INT *output_percentage)
{
	unsigned int len = sizeof(*output_percentage);
	HAL_WIFI_LOG(("wifi_getRadioBandUtilization radioIndex %d\n", radioIndex));

	NULL_PTR_ASSERT(output_percentage);
	RADIO_INDEX_ASSERT(radioIndex);
	RADIO_NOT_UP_ASSERT(radioIndex);

	if (wldm_xbrcm_lq(CMD_GET, radioIndex, (void *)output_percentage, &len, "bw_util", NULL) == 0) {
		return RETURN_OK;
	}

	HAL_WIFI_ERR(("%s: Failed to get band utilization\n", __FUNCTION__));
	return RETURN_ERR;
}

/* Gets the ApAssociatedDevice list for client MAC addresses */
#ifdef WIFI_HAL_VERSION_3_PHASE2
INT wifi_getApAssociatedDevice(INT ap_index, mac_address_t *output_clientMacAddressArray,
	UINT maxNumDevices, UINT *output_numEntries)
{
	char *mac_str_array = NULL, *mac_str, *rest = NULL;
	int len, i;

	NULL_PTR_ASSERT(output_clientMacAddressArray);
	NULL_PTR_ASSERT(output_numEntries);
	AP_INDEX_ASSERT(ap_index);

	HAL_WIFI_LOG(("%s: apIndex = %d max num = %d\n", __FUNCTION__, ap_index, maxNumDevices));
	if (maxNumDevices == 0) {
		HAL_WIFI_ERR(("%s: Incorrect max num:%d\n", __FUNCTION__, maxNumDevices));
		return WIFI_HAL_ERROR;
	}
	len = maxNumDevices * MAC_STR_LEN;
	mac_str_array = (char *)malloc(len);
	NULL_PTR_ASSERT(mac_str_array);
	memset(mac_str_array, 0, len);

	if (wldm_xbrcm_ap(CMD_GET, ap_index, mac_str_array, &len, "assoclist", NULL)
		!= 0) {
		HAL_WIFI_ERR(("%s: wldm_xbrcm_ap - assoclist ERROR\n", __FUNCTION__));
		free(mac_str_array);
		return WIFI_HAL_ERROR;
	}
	mac_str = strtok_r(mac_str_array, ",", &rest);
	for (i = 0; (i < maxNumDevices) && (mac_str != NULL); ++i) {
		sscanf(mac_str, MACF, MAC_TO_MACF(&output_clientMacAddressArray[i]));
		mac_str = strtok_r(NULL, ",", &rest);
	}
	*output_numEntries = i;
	free(mac_str_array);
	return WIFI_HAL_SUCCESS;

}
#else /* WIFI_HAL_VERSION_3_PHASE2 */
// Deprecated
INT wifi_getApAssociatedDevice(INT ap_index, CHAR *output_buf, INT output_buf_size)
{
	NULL_PTR_ASSERT(output_buf);
	AP_INDEX_ASSERT(ap_index);

	HAL_WIFI_LOG(("%s, apIndex = %d\n", __FUNCTION__, ap_index));

	if (wldm_xbrcm_ap(CMD_GET, ap_index, output_buf, (unsigned int *)&output_buf_size, "assoclist", NULL)
		== 0) {
		HAL_WIFI_DBG(("%s: MAC of associated STAs = %s\n", __FUNCTION__, output_buf));
		return RETURN_OK;
	}
	HAL_WIFI_ERR(("%s: wldm_xbrcm_ap - assoclist ERROR\n", __FUNCTION__));
	return RETURN_ERR;
}
#endif /* WIFI_HAL_VERSION_3_PHASE2 */

#ifdef WIFI_HAL_VERSION_3
//INT wifi_getApAuthenticatedDevices(INT ap_index, mac_address_t *mac_array, UINT maxNumDevices, UINT *output_numEntries)
INT wifi_getApAuthenticatedDevices(INT apIndex, mac_t **mac_array, UINT *output_array_size)
{
        UNUSED_PARAMETER(apIndex);
        UNUSED_PARAMETER(mac_array);
        UNUSED_PARAMETER(output_array_size);

	return WIFI_HAL_SUCCESS;

}
#endif /* WIFI_HAL_VERSION_3 */

/* to get the RSSI of an associated STA */
INT wifi_getApDeviceRSSI(INT ap_index, CHAR *MAC, INT *output_RSSI)
{
	int returnStatus, len;

	HAL_WIFI_DBG(("%s: Enter, ap_index = %d, MAC = %s\n", __FUNCTION__, ap_index, MAC));

	NULL_PTR_ASSERT(output_RSSI);
	NULL_PTR_ASSERT(MAC);
	AP_INDEX_ASSERT(ap_index);

	len = sizeof(*output_RSSI);
	*output_RSSI = -125;

	returnStatus = wldm_AccessPoint_Device_SignalStrength(CMD_GET, ap_index, output_RSSI, &len,
		MAC, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_AccessPoint_Device_SignalStrength CMD_GET Failed, Status = %d\n",
			__FUNCTION__, returnStatus));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: ap_index = %d, MAC = %s output_RSSI: %d\n", __FUNCTION__, ap_index, MAC, *output_RSSI));
	return RETURN_OK;
}

/* to get the Rx Rate of an associated STA */
INT wifi_getApDeviceRxrate (INT ap_index, CHAR *MAC, INT *output_RxMb)
{
	unsigned int len;
	void *p = MAC;

	NULL_PTR_ASSERT(output_RxMb);
	NULL_PTR_ASSERT(MAC);
	AP_INDEX_ASSERT(ap_index);

	HAL_WIFI_LOG(("%s: apIndex = %d, DeviceMacAddress = %s\n", __FUNCTION__, ap_index, MAC));

	len = sizeof(*output_RxMb);
	*output_RxMb = 0;

	if (wldm_xbrcm_sta(CMD_GET, ap_index, p, &len, "rx_rate", NULL) == 0) {
		*output_RxMb = *(int *)p;
		HAL_WIFI_DBG(("%s: *output_RxMb:%d\n", __FUNCTION__, *output_RxMb));
		return RETURN_OK;
	}
	return RETURN_ERR;
}

/* To get the Tx Rate of an associated STA */
INT wifi_getApDeviceTxrate (INT ap_index, CHAR *MAC, INT *output_TxMb)
{
	unsigned int len = sizeof(*output_TxMb);
	void *p = MAC;

	NULL_PTR_ASSERT(output_TxMb);
	NULL_PTR_ASSERT(MAC);
	AP_INDEX_ASSERT(ap_index);

	HAL_WIFI_LOG(("%s: apIndex = %d, DeviceMacAddress = %s\n", __FUNCTION__, ap_index, MAC));

	*output_TxMb = 0;

	if (wldm_xbrcm_sta(CMD_GET, ap_index, (void *)p, &len, "tx_rate", NULL) == 0) {
		*output_TxMb = *(int *)p;
		HAL_WIFI_DBG(("%s: *output_TxMb:%d\n", __FUNCTION__, *output_TxMb));
		return RETURN_OK;
	}
	return RETURN_ERR;
}

INT wifi_getApAssociatedDeviceStats(INT apIndex, mac_address_t *clientMacAddress, wifi_associated_dev_stats_t *associated_dev_stats, ULLONG *handle)
{
	unsigned int len = sizeof(wldm_wifi_associated_dev_stats_t) + sizeof(ULLONG), i;
	char *p = NULL, mac_address[MAC_STR_LEN], *ptr;
	unsigned int macInt[ETHER_ADDR_LEN] = {0}, rssi_size = 0;
	wldm_wifi_associated_dev_stats_t *wldm_wifi_associated_dev_stats;

	NULL_PTR_ASSERT(clientMacAddress);
	NULL_PTR_ASSERT(associated_dev_stats);
	NULL_PTR_ASSERT(handle);

	p = (char *) malloc(len);

	if (p == NULL) {
		HAL_WIFI_ERR(("%s:%d Malloc failed\n", __FUNCTION__, __LINE__));
		return RETURN_ERR;
	}
	memset(p, 0, len);
	for (i = 0; i < ETHER_ADDR_LEN; i++) {
		macInt[i] = (unsigned int)(*clientMacAddress)[i];
	}
	snprintf(mac_address, sizeof(mac_address), "%02x:%02x:%02x:%02x:%02x:%02x", macInt[0],
		macInt[1], macInt[2], macInt[3], macInt[4], macInt[5]);

	HAL_WIFI_DBG(("%s, apIndex = %d, DeviceMacAddress = %s\n", __FUNCTION__, apIndex, mac_address));

	memcpy(p, mac_address, sizeof(mac_address));

	if (wldm_xbrcm_sta(CMD_GET, apIndex, p, &len, "assoc_dev_stats", NULL) == 0) {
		/* Both associated_dev_stats and handle is returned in the output */
		ptr = p;
		wldm_wifi_associated_dev_stats = (wldm_wifi_associated_dev_stats_t *)ptr;
		associated_dev_stats->cli_rx_bytes = wldm_wifi_associated_dev_stats->cli_rx_bytes;
		associated_dev_stats->cli_tx_bytes = wldm_wifi_associated_dev_stats->cli_tx_bytes;
		associated_dev_stats->cli_rx_frames = wldm_wifi_associated_dev_stats->cli_rx_frames;
		associated_dev_stats->cli_tx_frames = wldm_wifi_associated_dev_stats->cli_tx_frames;
		associated_dev_stats->cli_rx_bytes = wldm_wifi_associated_dev_stats->cli_rx_bytes;
		associated_dev_stats->cli_rx_retries = wldm_wifi_associated_dev_stats->cli_rx_retries;
		associated_dev_stats->cli_tx_retries = wldm_wifi_associated_dev_stats->cli_tx_retries;
		associated_dev_stats->cli_rx_errors = wldm_wifi_associated_dev_stats->cli_rx_errors;
		associated_dev_stats->cli_tx_errors = wldm_wifi_associated_dev_stats->cli_tx_errors;
		associated_dev_stats->cli_rx_rate = wldm_wifi_associated_dev_stats->cli_rx_rate;
		associated_dev_stats->cli_tx_rate = wldm_wifi_associated_dev_stats->cli_tx_rate;
		rssi_size = (4 > WL_STA_ANT_MAX) ? WL_STA_ANT_MAX : 4;
		for (i = 0; i < rssi_size; ++i) {
			associated_dev_stats->cli_rssi_bcn.rssi[i] =
				wldm_wifi_associated_dev_stats->cli_rssi_bcn.rssi[i];
			associated_dev_stats->cli_rssi_ack.rssi[i] =
				wldm_wifi_associated_dev_stats->cli_rssi_bcn.rssi[i];
		}

		ptr += sizeof(wldm_wifi_associated_dev_stats_t);
		memcpy(handle, ptr, sizeof(unsigned long long));
		free(p);
		HAL_WIFI_DBG(("%s, apIndex = %d, handle = %llu\n", __FUNCTION__, apIndex, *handle));
		return RETURN_OK;
	}

	free(p);
	return RETURN_ERR;
}

static int hal_bss_status(INT apIndex)
{
	BOOLEAN status = FALSE;
	int len = sizeof(status), iRet = RETURN_OK;

	AP_INDEX_ASSERT(apIndex);

	iRet = wldm_AccessPoint_Enable(CMD_GET, apIndex, &status, &len, NULL, NULL);

	if (iRet < 0) {
		HAL_WIFI_ERR(("%s: apIndex %d Error!!!!\n", __FUNCTION__, apIndex));
		return -1;
	}
	HAL_WIFI_DBG(("%s: apIndex %d is %s\n", __FUNCTION__, apIndex, status ? "Up" : "Down"));
	return status ? 1 : 0;
}

INT wifi_getApManagementFramePowerControl(INT apIndex, INT *output_dBm)
{
	HAL_WIFI_LOG(("%s: apIndex = %d", __FUNCTION__,  apIndex));
	NULL_PTR_ASSERT(output_dBm);
	AP_INDEX_ASSERT(apIndex);

	*output_dBm = 0;

	if (hal_bss_status(apIndex) != 1) {
		HAL_WIFI_ERR(("%s: failed. apIndex %d is not up\n", __FUNCTION__, apIndex));
		return RETURN_ERR;
	}

	unsigned int len = sizeof(*output_dBm);
	if (wldm_xbrcm_ap(CMD_GET, apIndex, output_dBm, &len, "bcnprs_txpwr_offset", NULL) < 0) {
		HAL_WIFI_ERR(("%s: wldm_xbrcm_ap CMD_GET bcnprs_txpwr_offset for apIndex = %d failed\n",
			__FUNCTION__, apIndex));
		return RETURN_ERR;
	}
	*output_dBm = 0 - *output_dBm;

	HAL_WIFI_DBG(("%s: apIndex %d, adjust %d dBm\n", __FUNCTION__, apIndex, *output_dBm));
	return RETURN_OK;
}

INT wifi_setApManagementFramePowerControl(INT apIndex, INT dBm) /* Applying changes with wifi_applyRadioSettings() */
{
	HAL_WIFI_LOG(("%s: Index %d, adjust %d dBm\n", __FUNCTION__, apIndex, dBm));

	AP_INDEX_ASSERT(apIndex);

	if (hal_bss_status(apIndex) != 1) {
		HAL_WIFI_ERR(("%s: failed. apIndex %d is not up\n", __FUNCTION__, apIndex));
		return RETURN_ERR;
	}
	if (dBm < 0)
		dBm = 0 - dBm;

	unsigned int len = sizeof(dBm);
	if (wldm_xbrcm_ap(CMD_SET_IOCTL | CMD_SET_NVRAM, apIndex, &dBm, &len, "bcnprs_txpwr_offset", NULL) < 0) {
		HAL_WIFI_ERR(("%s: wldm_xbrcm_ap CMD_SET_IOCTL/NVRAM for bcnprs_txpwr_offset for apIndex = %d failed\n",
			__FUNCTION__, apIndex));
		return RETURN_ERR;
	}
	else
		HAL_WIFI_DBG(("%s: apIndex %d, adjust %d dBm\n", __FUNCTION__, apIndex, dBm));
	return RETURN_OK;
}

/**
* @brief Set Access Point Beacon TX rate.
*
* Device.WiFi.AccessPoint.{i}.X_RDKCENTRAL-COM_BeaconRate
*
* @param[in] apIndex Access point index will be 0, 2, 4, 6, 8, 10,
*	12, 14(for 2.4G) only;
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
INT wifi_setApBeaconRate(INT apIndex, CHAR *input_string)
{
	HAL_WIFI_LOG(("%s: AP Index %d, beaconRate %s\n", __FUNCTION__, apIndex, input_string ));

	AP_INDEX_ASSERT(apIndex);

	if (!input_string) {
		HAL_WIFI_ERR(("%s: Missing input argument\n", __FUNCTION__));
		return RETURN_ERR;
	}

	unsigned int len = strlen(input_string);
	char *pvar = "beacon_rate";

	if (wldm_xbrcm_ap(CMD_SET_NVRAM | CMD_SET_IOCTL, apIndex, input_string,
		&len, pvar, NULL) < 0) {
		HAL_WIFI_ERR(("%s: wldm_xbrcm_ap CMD_SET_IOCTL or CMD_SET_NVRAM %s failed\n",
			__FUNCTION__, pvar));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

INT wifi_getApBeaconRate(INT apIndex, CHAR *output_string)
{
	CHAR beaconrate[OUTPUT_STRING_LENGTH_32] = {0};

	HAL_WIFI_LOG(("%s: AP Index %d\n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(output_string);

	unsigned int len = sizeof(beaconrate);
	char *pvar = "beacon_rate";

	if (wldm_xbrcm_ap(CMD_GET, apIndex, beaconrate, &len, pvar, NULL) < 0) {
		HAL_WIFI_ERR(("%s: wldm_xbrcm_api CMD GET %s failed\n", __FUNCTION__, pvar));
		return RETURN_ERR;
	}

	snprintf(output_string, sizeof(beaconrate), "%s", beaconrate);
	return RETURN_OK;
}

INT wifi_getApTxBeaconFrameCount(INT apIndex, UINT32 *count)
{
	HAL_WIFI_LOG(("%s: AP Index %d\n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);

	unsigned int len = sizeof(UINT32);
	uint txbcncnt = 0;
	if (wldm_xbrcm_counter(CMD_GET, apIndex, &txbcncnt, &len, "txbcnfrm", NULL) < 0) {
		HAL_WIFI_ERR(("%s: wldm_xbrcm_ap failed get txbcnfrm count\n", __FUNCTION__));
		return RETURN_ERR;
	}
	*count = txbcncnt;
	HAL_WIFI_DBG(("%s, txbcmfram count %d\n", __FUNCTION__, *count));

	return RETURN_OK;
}

#define WLDM_HISTO_MAX_ARR_LEN 204 /* LEGACY + (NUM_VHT_RATES * NUM_BW) = 12 + 48 * 4 */

INT wifi_getApAssociatedDeviceRxStatsResult(INT apIndex, mac_address_t *clientMacAddress, wifi_associated_dev_rate_info_rx_stats_t **stats_array, UINT *output_array_size, ULLONG *handle)
{
	unsigned int i;
	char *p = NULL, mac_address[MAC_STR_LEN], *ptr;
	unsigned int len, size;
	wldm_wifi_associatedDevRateInfoStats_t *wldm_rx_stats;
	wifi_associated_dev_rate_info_rx_stats_t *rx_stats = NULL;

	NULL_PTR_ASSERT(clientMacAddress);
	NULL_PTR_ASSERT(output_array_size);
	NULL_PTR_ASSERT(handle);
	AP_INDEX_ASSERT(apIndex);

	*output_array_size = 0;
	*handle = 0;
	*stats_array = NULL;
	len = WLDM_HISTO_MAX_ARR_LEN;

	size = len * sizeof(wldm_wifi_associatedDevRateInfoStats_t);

	p = (char *)malloc(size);

	if (p == NULL) {
		HAL_WIFI_ERR(("%s:%d Malloc failed\n", __FUNCTION__, __LINE__));
		return RETURN_ERR;
	}
	memset(p, 0, size);

	snprintf(mac_address, sizeof(mac_address), "%02x:%02x:%02x:%02x:%02x:%02x",
		(*clientMacAddress)[0], (*clientMacAddress)[1], (*clientMacAddress)[2],
		(*clientMacAddress)[3], (*clientMacAddress)[4], (*clientMacAddress)[5]);

	memcpy(p, mac_address, sizeof(mac_address));

	if (wldm_xbrcm_sta(CMD_GET, apIndex, p, &len, "rx_stats", NULL) < 0) {
		free(p);
		return RETURN_ERR;
	}
	rx_stats = (wifi_associated_dev_rate_info_rx_stats_t *)malloc(len *
		sizeof(wifi_associated_dev_rate_info_rx_stats_t));
	if (rx_stats == NULL) {
		HAL_WIFI_ERR(("%s:%d Malloc failed\n", __FUNCTION__, __LINE__));
		free(p);
		return RETURN_ERR;
	}
	memset(rx_stats, 0, len * sizeof(wifi_associated_dev_rate_info_rx_stats_t));
	*stats_array = rx_stats;
	ptr = p;
	wldm_rx_stats = (wldm_wifi_associatedDevRateInfoStats_t *)p;
	for (i = 0; i < len; ++i) {
		rx_stats[i].nss = wldm_rx_stats[i].nss;
		rx_stats[i].mcs = wldm_rx_stats[i].mcs;
		rx_stats[i].bw = wldm_rx_stats[i].bw;
		rx_stats[i].flags = wldm_rx_stats[i].flags;
		rx_stats[i].msdus = wldm_rx_stats[i].msdus;
		rx_stats[i].mpdus = wldm_rx_stats[i].mpdus;
		rx_stats[i].rssi_combined = wldm_rx_stats[i].rx_rssi_combined;
		ptr += sizeof(wldm_wifi_associatedDevRateInfoStats_t);
	}
	memcpy(handle, ptr, sizeof(unsigned long long));
	*output_array_size = len;
	free(p);
	return RETURN_OK;
}

INT wifi_getApAssociatedDeviceTxStatsResult(INT apIndex, mac_address_t *clientMacAddress, wifi_associated_dev_rate_info_tx_stats_t **stats_array, UINT *output_array_size, ULLONG *handle)
{
	unsigned int i;
	char *p = NULL, mac_address[MAC_STR_LEN], *ptr;
	unsigned int len, size;
	wldm_wifi_associatedDevRateInfoStats_t *wldm_tx_stats;
	wifi_associated_dev_rate_info_tx_stats_t *tx_stats = NULL;

	NULL_PTR_ASSERT(clientMacAddress);
	NULL_PTR_ASSERT(output_array_size);
	NULL_PTR_ASSERT(handle);
	AP_INDEX_ASSERT(apIndex);

	*output_array_size = 0;
	*handle = 0;
	*stats_array = NULL;
	len = WLDM_HISTO_MAX_ARR_LEN;

	size = len * sizeof(wldm_wifi_associatedDevRateInfoStats_t);

	p = (char *)malloc(size);

	if (p == NULL) {
		HAL_WIFI_ERR(("%s:%d Malloc failed\n", __FUNCTION__, __LINE__));
		return RETURN_ERR;
	}
	memset(p, 0, size);

	snprintf(mac_address, sizeof(mac_address), "%02x:%02x:%02x:%02x:%02x:%02x",
		(*clientMacAddress)[0], (*clientMacAddress)[1], (*clientMacAddress)[2],
		(*clientMacAddress)[3], (*clientMacAddress)[4], (*clientMacAddress)[5]);

	memcpy(p, mac_address, sizeof(mac_address));

	if (wldm_xbrcm_sta(CMD_GET, apIndex, p, &len, "tx_stats", NULL) < 0) {
		free(p);
		return RETURN_ERR;
	}
	tx_stats = (wifi_associated_dev_rate_info_tx_stats_t *)malloc(len *
			sizeof(wifi_associated_dev_rate_info_tx_stats_t));
	if (tx_stats == NULL) {
		HAL_WIFI_ERR(("%s:%d Malloc failed\n", __FUNCTION__, __LINE__));
		free(p);
		return RETURN_ERR;
	}
	memset(tx_stats, 0, len * sizeof(wifi_associated_dev_rate_info_tx_stats_t));
	*stats_array = tx_stats;
	ptr = p;
	wldm_tx_stats = (wldm_wifi_associatedDevRateInfoStats_t *)p;
	for (i = 0; i < len; ++i) {
		tx_stats[i].nss = wldm_tx_stats[i].nss;
		tx_stats[i].mcs = wldm_tx_stats[i].mcs;
		tx_stats[i].bw = wldm_tx_stats[i].bw;
		tx_stats[i].flags = wldm_tx_stats[i].flags;
		tx_stats[i].msdus = wldm_tx_stats[i].msdus;
		tx_stats[i].mpdus = wldm_tx_stats[i].mpdus;
		ptr += sizeof(wldm_wifi_associatedDevRateInfoStats_t);
	}
	memcpy(handle, ptr, sizeof(unsigned long long));
	*output_array_size = len;
	free(p);
	return RETURN_OK;
}

//Device.WiFi.X_RDKCENTRAL-COM_BandSteering.BandSetting.{i}.UtilizationThreshold int r/w
//to set and read the band steering BandUtilizationThreshold parameters
INT wifi_getBandSteeringBandUtilizationThreshold(INT radioIndex, INT *pBuThreshold)
{
	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(pBuThreshold);

	return wldm_xbrcm_bsd(CMD_GET_NVRAM, radioIndex, WLDM_BSD_STEER_BANDUTIL,
		(void *)pBuThreshold, NULL);
}

INT wifi_setBandSteeringBandUtilizationThreshold(INT radioIndex, INT buThreshold)
{
	RADIO_INDEX_ASSERT(radioIndex);
	return wldm_xbrcm_bsd(CMD_SET_NVRAM, radioIndex, WLDM_BSD_STEER_BANDUTIL,
		(void *)&buThreshold, NULL);
}

//Device.WiFi.X_RDKCENTRAL-COM_BandSteering.BandSetting.{i}.RSSIThreshold int r/w
//to set and read the band steering RSSIThreshold parameters
INT wifi_getBandSteeringRSSIThreshold(INT radioIndex, INT *pRssiThreshold)
{
	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(pRssiThreshold);

	return wldm_xbrcm_bsd(CMD_GET_NVRAM, radioIndex, WLDM_BSD_STEER_RSSI,
		(void *)pRssiThreshold, NULL);
}

INT wifi_setBandSteeringRSSIThreshold(INT radioIndex, INT rssiThreshold)
{
	RADIO_INDEX_ASSERT(radioIndex);

	return wldm_xbrcm_bsd(CMD_SET_NVRAM, radioIndex, WLDM_BSD_STEER_RSSI,
		(void *)&rssiThreshold, NULL);
}

void wifi_newApAssociatedDevice_callback_register(wifi_newApAssociatedDevice_callback callback_proc)
{
	HAL_WIFI_DBG(("%s\n", __FUNCTION__));

	if (wifi_numRegisteredAssociatedCallbacks == 0) {
		if (wldm_callback(WLDM_CB_REGISTER, FD_ASSOC_DEV, wifi_hal_cb_assoc_dev_evt_handler, &wifi_cb_info) < 0) {
			HAL_WIFI_ERR(("Failed to register the %s callback\n", __FUNCTION__));
			return;
		}
		cbThreadId = wifi_cb_info.cbThreadId;
	}
	if (wifi_numRegisteredAssociatedCallbacks < MAX_REGISTERED_CB_NUM) {
		wifi_app_associated_callbacks[wifi_numRegisteredAssociatedCallbacks] = callback_proc;
		wifi_numRegisteredAssociatedCallbacks++;
	} else {
		HAL_WIFI_DBG(("%s: associated_callback max number is reached\n",__FUNCTION__));
	}
}

#if WIFI_HAL_VERSION_GE_2_12 || defined(WIFI_HAL_VERSION_3)
void wifi_apDisassociatedDevice_callback_register(wifi_apDisassociatedDevice_callback callback_proc)
{
	if (wifi_numRegisteredDisassociatedCallbacks < MAX_REGISTERED_CB_NUM) {
		wifi_app_disassociated_callbacks[wifi_numRegisteredDisassociatedCallbacks] = callback_proc;
		wifi_numRegisteredDisassociatedCallbacks++;
	} else {
		HAL_WIFI_DBG(("%s: dissociated_callback max number is reached\n",__FUNCTION__));
	}
	return;
}
#endif

//Device.WiFi.X_RDKCENTRAL-COM_BandSteering.BandSetting.{i}.PhyRateThreshold int r/w
//to set and read the band steering physical modulation rate threshold parameters
INT wifi_getBandSteeringPhyRateThreshold(INT radioIndex, INT *pPrThreshold) //If chip is not support, return -1
{
	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(pPrThreshold);

	return wldm_xbrcm_bsd(CMD_GET_NVRAM, radioIndex, WLDM_BSD_STEER_PHYRATE,
		(void *)pPrThreshold, NULL);
}

INT wifi_setBandSteeringPhyRateThreshold(INT radioIndex, INT prThreshold) //If chip is not support, return -1
{
	RADIO_INDEX_ASSERT(radioIndex);
	return wldm_xbrcm_bsd(CMD_SET_NVRAM, radioIndex, WLDM_BSD_STEER_PHYRATE,
		(void *)&prThreshold, NULL);
}

#define WLAN_CM_RDK_BSD_LOG_FILE_NVRAM "bsd_rdk_log_file"
#define WLAN_CM_RDK_BSD_LOG_FILE "/rdklogs/logs/bandsteering_log.txt"

//Device.WiFi.X_RDKCENTRAL-COM_BandSteering.History string r/o
INT wifi_getBandSteeringLog(INT record_index, ULONG *pSteeringTime, CHAR *pClientMAC, INT *pSourceSSIDIndex, INT *pDestSSIDIndex, INT *pSteeringReason) //if no steering or redord_index is out of boundary, return -1. pSteeringTime returns the UTC time in seconds. pClientMAC is pre allocated as 64bytes. pSteeringReason returns the predefined steering trigger reason
{
	char tmp[OUTPUT_STRING_LENGTH_256], steer_reason[OUTPUT_STRING_LENGTH_64];
	char time_str[OUTPUT_STRING_LENGTH_64], date_str[OUTPUT_STRING_LENGTH_64] = {0};
	char filename[] = "/tmp/steering_log.txt", *str = NULL;
	FILE *fp;

	HAL_WIFI_DBG(("wifi_getBandSteeringLog record_index %d\n", record_index));

	if ((record_index < 0) || (pSteeringTime == NULL) || (pClientMAC == NULL) ||
		(pSourceSSIDIndex == NULL) || (pDestSSIDIndex == NULL) ||
		(pSteeringReason == NULL)) {
		HAL_WIFI_ERR(("%s, Input parameter error!!! %d %p %p %p %p %p\n", __FUNCTION__,
			record_index, pSteeringTime, pClientMAC, pSourceSSIDIndex, pDestSSIDIndex,
			pSteeringReason));
		return RETURN_ERR;
	}

	memset(tmp, 0, sizeof(tmp));
	/* Steering logs are in the following format */
	/*
	1586386566 2020-4-8 22:56:6 steerexecImplCmnStartSteer RDKB_LOG_STEERED MAC=88:b4:a6:12:51:ff FROM_BAND=0 TO_BAND=1 REASON=1 steering_in_progress
	1586386566 2020-4-8 22:56:6 steerexecImplCmnHandleAssocUpdateBTM Steering MAC=88:b4:a6:12:51:ff FROM_BAND=0 TO_BAND=1 REASON=1 is_complete
	*/
	str = nvram_get(WLAN_CM_RDK_BSD_LOG_FILE_NVRAM);

	snprintf(tmp, sizeof(tmp), "awk '/RDKB_LOG_STEERED/{i++}i==%d{print; exit}' %s > %s",
		record_index + 1, str ? str : WLAN_CM_RDK_BSD_LOG_FILE, filename);

	if (system(tmp)) {
		HAL_WIFI_DBG(("%s: %s failed to run\n", __FUNCTION__, tmp));
	}

	fp = fopen(filename, "r");
	if (fp) {
		if (9 != fscanf(fp, "%lu %11s %8s %40s RDKB_LOG_STEERED MAC=%17s FROM_BAND=%d TO_BAND=%d REASON=%d %25s\n",
			pSteeringTime, date_str, time_str, tmp, pClientMAC, pSourceSSIDIndex,
			pDestSSIDIndex, pSteeringReason, steer_reason)) {
			HAL_WIFI_ERR(("%s: Bandsteering log does not have info\n", __FUNCTION__));
			fclose(fp);
			return RETURN_ERR;
		}
		fclose(fp);
	}
	if (remove(filename)) {
		HAL_WIFI_ERR(("%s: Unable to remove %s\n", __FUNCTION__, filename));
	}
	return RETURN_OK;
}

//Device.WiFi.X_RDKCENTRAL-COM_BandSteering.APGroup string r/w
//To get Band Steering AP group
INT wifi_getBandSteeringApGroup(char *output_ApGroup)
{
	int len = STRING_LENGTH_64;

	NULL_PTR_ASSERT(output_ApGroup);

	return wldm_xbrcm_bsd(CMD_GET, 0, WLDM_BSD_STEER_APGROUP, (void *)output_ApGroup, &len);
}

/*
  The definiton from wifi_hal.h
  To set Band Steering AP group
  ApGroup contains AP index(start from 1) pair array, in following format "$index_2.4G,$index_5G;$index_2.4G,$index_5G"
  Example "1,2;3,4;7,8" for Private, XH, LnF pairs.
  ApGroup have to contain at least one AP pair, such as "1,2"

  Note: Curerntly Broadcom bsd only support one group in one instance (daemon)
 */
INT wifi_setBandSteeringApGroup(char *ApGroup)
{
	return wldm_xbrcm_bsd(CMD_SET, 0, WLDM_BSD_STEER_APGROUP, (void *)ApGroup, NULL);
}

/* Device.WiFi.Radio.i.X_RDKCENTRAL-COM_clientInactivityTimout. Integer ro
 * This is used to read the ClientInactivityTimout from driver.
 */
INT wifi_getRadioClientInactivityTimout(INT radioIndex, INT *output_timout_sec)
{
	unsigned int len = sizeof(*output_timout_sec);

	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(output_timout_sec);

	if (wldm_xbrcm_ap(CMD_GET, radioIndex, output_timout_sec, &len, "sta_inactivity_timeout",
		NULL) != 0) {
		HAL_WIFI_ERR(("%s, radioIndex[%d]\n", __FUNCTION__, radioIndex));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

//Device.WiFi.X_RDKCENTRAL-COM_BandSteering.BandSetting.{i}.OverloadInactiveTime int r/w
//to set and read the inactivity time (in seconds) for steering under overload condition
INT wifi_getBandSteeringOverloadInactiveTime (INT radioIndex, INT *overloadInactiveTime)
{
	UNUSED_PARAMETER(radioIndex);
	UNUSED_PARAMETER(overloadInactiveTime);
	// stub function
	return RETURN_ERR;
}

INT wifi_setBandSteeringOverloadInactiveTime (INT radioIndex, INT overloadInactiveTime)
{
	// stub function
	UNUSED_PARAMETER(radioIndex);
        UNUSED_PARAMETER(overloadInactiveTime);
	return RETURN_ERR;
}

//Device.WiFi.X_RDKCENTRAL-COM_BandSteering.BandSetting.{i}.IdleInactiveTime int r/w
//to set and read the inactivity time (in seconds) for steering under Idle condition
INT wifi_getBandSteeringIdleInactiveTime (INT radioIndex, INT *idleInactiveTime)
{
	// stub function
	UNUSED_PARAMETER(radioIndex);
	UNUSED_PARAMETER(idleInactiveTime);
	return RETURN_ERR;
}

INT wifi_setBandSteeringIdleInactiveTime (INT radioIndex, INT idleInactiveTime)
{
	// stub function
	UNUSED_PARAMETER(radioIndex);
	UNUSED_PARAMETER(idleInactiveTime);
	return RETURN_ERR;
}

/* for DCS */
INT wifi_setRadioDcsDwelltime(INT radioIndex, INT millisecond)
{
	unsigned int len;
	int ret;
	HAL_WIFI_DBG(("%s: Enter", __FUNCTION__));

	RADIO_INDEX_ASSERT(radioIndex);

	HAL_WIFI_DBG(("%s: radioIndex %d %dmSec\n", __FUNCTION__, radioIndex, millisecond));

	len = sizeof(millisecond);
	ret = wldm_xbrcm_scan(CMD_SET_NVRAM, radioIndex, &millisecond, &len, "dcs_dwell_time", NULL);
	if (ret != 0) {
		HAL_WIFI_ERR(("%s: Error radioIndex=%d\n", __FUNCTION__, radioIndex));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: Done radioIndex=%d millisecond=%d\n", __FUNCTION__, radioIndex, millisecond));

	return RETURN_OK;
}

INT wifi_getRadioDcsDwelltime(INT radioIndex, INT *output_millisecond)
{
	unsigned int len;
	int ret;
	HAL_WIFI_DBG(("%s: Enter", __FUNCTION__));

	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(output_millisecond);

	len = sizeof(*output_millisecond);
	ret = wldm_xbrcm_scan(CMD_GET_NVRAM, radioIndex, output_millisecond, &len, "dcs_dwell_time", NULL);
	if (ret != 0) {
		HAL_WIFI_ERR(("%s: Error radioIndex=%d\n", __FUNCTION__, radioIndex));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: Done dcs_dwell_time radioIndex=%d output_millisecond=%d\n",
		__FUNCTION__, radioIndex, *output_millisecond));

	return RETURN_OK;
}

static BOOL dcsscanning[2] = {FALSE, FALSE};
INT wifi_setRadioDcsScanning(INT radioIndex, BOOL enable_background_scanning)
{
	dcsscanning[(radioIndex == 0) ? 0 : 1] = enable_background_scanning;
	return RETURN_OK;
}

INT wifi_getRadioDcsScanning(INT radioIndex, BOOL *output_enable_background_scanning)
{
	if (output_enable_background_scanning) {
		*output_enable_background_scanning = dcsscanning[(radioIndex == 0) ? 0 : 1];
		return RETURN_OK;
	} else
		return RETURN_ERR;
}

#define MAX_NUMCHANNELS		64

#define AP_CH_WIDTH_20		0x01
#define AP_CH_WIDTH_40		0x02
#define AP_CH_WIDTH_80		0x04
#define AP_CH_WIDTH_160		0x08
#define AP_CH_WIDTH_80_80	0x10

#define OPER_CH_BWSTR_20	"20"
#define OPER_CH_BWSTR_40	"40"
#define OPER_CH_BWSTR_80	"80"
#define OPER_CH_BWSTR_160	"160"
#define OPER_CH_BWSTR_80_80	"80+80"

/* Convert channel width to unsigned int bitmask */
#define CH_BWSTR_TO_BITMASK(bwStr) \
	(!strcmp(bwStr, OPER_CH_BWSTR_20)) ? AP_CH_WIDTH_20 : \
	(!strcmp(bwStr, OPER_CH_BWSTR_40)) ? AP_CH_WIDTH_40 : \
	(!strcmp(bwStr, OPER_CH_BWSTR_80)) ? AP_CH_WIDTH_80 : \
	(!strcmp(bwStr, OPER_CH_BWSTR_160)) ? AP_CH_WIDTH_160 : \
	(!strcmp(bwStr, OPER_CH_BWSTR_80_80)) ? AP_CH_WIDTH_80_80 : 0

/* Bubble Sort by ascending channel_number
 * len = array_size
 */
static void
sortInpChannelMetrics_arrayByChNum(int len, wifi_channelMetrics_t *x)
{
	int i, j, swapped;
	wifi_channelMetrics_t tmp;

	for (i = 0; i < (len - 1); i++) {
		swapped = 0;
		for (j = 0; j < (len - i - 1); j++) {
			if (x[j].channel_number > x[j + 1].channel_number) {
				/* swap j and j + 1 */
				tmp = x[j];
				x[j] = x[j + 1];
				x[j + 1] = tmp;
				swapped = 1;
			}
		} /* for j */
		if (swapped == 0)
			break;
	} /* for i */
}

/* Bubble Sort by descending ap_rssi
 * len = array_size
 */
static void
sortChannelRssiList(int len, wifi_apRssi_t *x)
{
	int i, j, swapped;
	wifi_apRssi_t tmp;

	for (i = 0; i < (len - 1); i++) {
		swapped = 0;
		for (j = 0; j < (len - i - 1); j++) {
			if (x[j].ap_rssi < x[j + 1].ap_rssi) {
				/* swap j and j + 1 */
				tmp = x[j];
				x[j] = x[j + 1];
				x[j + 1] = tmp;
				swapped = 1;
			}
		} /* for j */
		if (swapped == 0)
			break;
	} /* for i */
}

INT wifi_getRadioDcsChannelMetrics(INT radioIndex,
	wifi_channelMetrics_t *input_output_channelMetrics_array,
	INT array_size)
{
	int num_channels, acnt, ch_rssiCnt, k;
	wifi_channelMetrics_t *wifi_cmp, *aPtr;
	wifi_apRssi_t *wifi_cm_apRssip;
	unsigned int num_res, i, len, macInt[6], prnCh, chList[MAX_NUMCHANNELS] = {0};
	wldm_neighbor_ap2_t *wldm_neighbor_ptr, *nbp;
	wifi_channelStats_t *csp, ch_stats[MAX_NUMCHANNELS] = {0};

	HAL_WIFI_DBG(("%s: Enter\n", __FUNCTION__));
	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(input_output_channelMetrics_array);

	sortInpChannelMetrics_arrayByChNum(array_size, input_output_channelMetrics_array);

	/* Get num_channels and channel list to get metrics for */
	num_channels = 0;
	acnt = 0;
	memset(ch_stats, 0, sizeof(ch_stats));
	while (acnt < array_size) {
		aPtr = &(input_output_channelMetrics_array[acnt]);
		if (aPtr->channel_in_pool) {
			chList[num_channels] = (unsigned int)(aPtr->channel_number);
			ch_stats[num_channels].ch_number = (unsigned int)(aPtr->channel_number);
			ch_stats[num_channels].ch_in_pool = TRUE;
			++num_channels;
		}
		acnt++;
	}

	/* Start Scan */
	HAL_WIFI_DBG(("%s: Start Scan num_channels = %d radioIndex [%d]\n",
		__FUNCTION__, num_channels, radioIndex));
	if (start_neighbor_scan(radioIndex, WLDM_RADIO_SCAN_MODE_DCS, 0, num_channels, chList)
			!= RETURN_OK) {
		HAL_WIFI_ERR(("%s: Error start_neighbor_scan radioIndex [%d]\n",
			__FUNCTION__, radioIndex));
		return RETURN_ERR;
	}

	/* Get Scan Results */
	if (wldm_xbrcm_scan(CMD_GET, radioIndex, NULL, &num_res, "num_scan_results", NULL) < 0) {
		HAL_WIFI_ERR(("%s:%d wldm_xbrcm_scan failed\n", __FUNCTION__, __LINE__));
		return RETURN_ERR;
	}
	if (array_size < (int)num_res) {
		HAL_WIFI_ERR(("%s: Truncate to array_size %d smaller than num_res %d radioIndex [%d]\n",
			 __FUNCTION__, array_size, num_res, radioIndex));
		num_res = (unsigned int)array_size;
		/* return RETURN_ERR; */
	}

	HAL_WIFI_DBG(("%s: Get Scan results num_res = %d radioIndex [%d]\n",
		__FUNCTION__, num_res, radioIndex));
	len = num_res * sizeof(wldm_neighbor_ap2_t);
	wldm_neighbor_ptr = (wldm_neighbor_ap2_t *)malloc(len);
	if (wldm_neighbor_ptr == NULL) {
		HAL_WIFI_ERR(("%s:%d Malloc failed\n", __FUNCTION__, __LINE__));
		return RETURN_ERR;
	}
	memset(wldm_neighbor_ptr, 0, len);
	if (wldm_xbrcm_scan(CMD_GET, radioIndex, (void *)wldm_neighbor_ptr, &num_res,
		"scan_results", NULL) < 0) {
		HAL_WIFI_ERR(("%s:%d wldm_xbrcm_scan failed\n", __FUNCTION__, __LINE__));
		free(wldm_neighbor_ptr);
		return RETURN_ERR;
	}

	/* Get chanim_stats */
	len = num_channels;
	if (wldm_xbrcm_lq(CMD_GET, radioIndex, (void *)&ch_stats, &len, "channel_stats", NULL) != 0) {
		HAL_WIFI_ERR(("%s:%d wldm_xbrcm_lq failed\n", __FUNCTION__, __LINE__));
		free(wldm_neighbor_ptr);
		return RETURN_ERR;
	}

	/* Fill in output array */
	for (k = 0; k < num_channels; ++k) {
		csp = &(ch_stats[k]);
		wifi_cmp = &(input_output_channelMetrics_array[k]);
		wifi_cmp->channel_number = csp->ch_number;
		wifi_cmp->channel_in_pool = TRUE;
		wifi_cmp->channel_noise = csp->ch_noise;
		wifi_cmp->channel_radar_noise = csp->ch_radar_noise;
		wifi_cmp->channel_non_80211_noise = csp->ch_non_80211_noise;
		wifi_cmp->channel_utilization = csp->ch_utilization;
		wifi_cmp->channel_txpower = 0; // TBD
		wifi_cmp->channel_rssi_count = 0;
	}

	/* Convert to DcsChannelMetrics */
	prnCh = 0;
	ch_rssiCnt = 0;
	wifi_cmp = &(input_output_channelMetrics_array[0]);
	for (i = 0; i < num_res; ++i) {
		nbp = &(wldm_neighbor_ptr[i]);
		if (prnCh != nbp->ap_Channel) {
			if (ch_rssiCnt != 0) {
				wifi_cmp->channel_rssi_count = ch_rssiCnt;
				sortChannelRssiList(ch_rssiCnt, wifi_cmp->channel_rssi_list);
				ch_rssiCnt = 0;
			}
			for (k = 0; k < num_channels; k++) {
				csp = &(ch_stats[k]);
				if ((unsigned int)csp->ch_number == nbp->ap_Channel) {
					prnCh = csp->ch_number;
					wifi_cmp = &(input_output_channelMetrics_array[k]);
					break;
				}
			}
		}
		wifi_cm_apRssip = &(wifi_cmp->channel_rssi_list[ch_rssiCnt]);
		sscanf(nbp->ap_BSSID, "%02x:%02x:%02x:%02x:%02x:%02x", &macInt[0], &macInt[1],
			&macInt[2], &macInt[3], &macInt[4], &macInt[5]);
		for (k = 0; k < 6; k++) {
			wifi_cm_apRssip->ap_BSSID[k] = (unsigned char)macInt[k];
		}
		wifi_cm_apRssip->ap_channelWidth = CH_BWSTR_TO_BITMASK(nbp->ap_OperatingChannelBandwidth);
		wifi_cm_apRssip->ap_rssi = nbp->ap_SignalStrength;
		ch_rssiCnt++;
	} /* for */
	if (ch_rssiCnt != 0) {
		wifi_cmp->channel_rssi_count = ch_rssiCnt;
		sortChannelRssiList(ch_rssiCnt, wifi_cmp->channel_rssi_list);
	}

	free(wldm_neighbor_ptr);
	return RETURN_OK;
}

INT wifi_pushRadioChannel(INT radioIndex, UINT channel)
{
	RADIO_INDEX_ASSERT(radioIndex);

	HAL_WIFI_DBG(("%s, radioIndex = %d, channel = %d\n", __FUNCTION__, radioIndex, channel));
	if (wifi_setRadioChannel(radioIndex, channel) != 0) {
		HAL_WIFI_ERR(("%s: wifi_setRadioChannel Failed\n", __FUNCTION__));
		return RETURN_ERR;
	}

	if (wifi_applyRadioSettings(radioIndex) != 0) {
		HAL_WIFI_ERR(("%s: wifi_applyRadioSettings Failed\n", __FUNCTION__));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

/* This HAL is used to change the channel to destination channel, with destination bandwidth,
 * csa_beacon_count is used to specify how long CSA need to be announced.
 */
INT wifi_pushRadioChannel2(INT radioIndex, UINT channel, UINT channel_width_MHz, UINT csa_beacon_count)
{
	bool acsEn;
	int len, ret;
	csa_t csaInfo;
	char bwStr[32] = {0};
	unsigned int bw;
	wldm_xbrcm_radio_param_t param;

	HAL_WIFI_DBG(("%s: Enter radioIndex = %d, channel = %d channel_width_MHz=%d\n",
		__FUNCTION__, radioIndex, channel, channel_width_MHz));

	RADIO_INDEX_ASSERT(radioIndex);

	/* Get current channel and bandwidth */
	len = sizeof(param);
	ret = wldm_xbrcm_radio(CMD_GET, radioIndex, &param, (unsigned int *)&len, "chanspec", NULL);
	if (ret < 0) {
		HAL_WIFI_ERR(("%s: wldm_xbrcm_Radio_Chanspec CMD_GET radioIndex=%d failed\n",
				__FUNCTION__, radioIndex));
		return RETURN_ERR;
	}
	HAL_WIFI_DBG(("%s: radioIndex=%d wldm_xbrcm_Radio_Chanspec channel=%d bandwidth=%s\n",
			__FUNCTION__, radioIndex, param.channel, param.bandwidth));

	/* no action if csa channel and banwidth same as current channel */
	if (param.channel == channel) {
		sscanf(param.bandwidth, "%d%32s", &bw, bwStr);
		if (bw == channel_width_MHz) {
			HAL_WIFI_ERR(("%s: wl%d No csa for current channel %d/%d\n",
				__FUNCTION__, radioIndex, channel, channel_width_MHz));
			return RETURN_OK;
		}
	}

	len = sizeof(acsEn);
	acsEn = FALSE;
	if (wldm_Radio_AutoChannelEnable(CMD_GET, radioIndex, &acsEn, &len, NULL, NULL) < 0) {
		HAL_WIFI_ERR(("%s: Error checking AutoChannelEnable\n", __FUNCTION__ ));
		return RETURN_ERR;
	}
	if (acsEn) {
		HAL_WIFI_LOG(("%s: Disabling AutoChannel mode!\n", __FUNCTION__));
		acsEn = FALSE;
		if (wldm_Radio_AutoChannelEnable(CMD_SET, radioIndex, &acsEn, &len, NULL, NULL) < 0) {
			HAL_WIFI_ERR(("%s: wldm_Radio_AutoChannelEnable set failed\n", __FUNCTION__ ));
			return RETURN_ERR;
		}
	}

	/* Send CSA */
	csaInfo.channel = channel;
	csaInfo.channel_width = channel_width_MHz;
	csaInfo.csa_beacon_count = csa_beacon_count;

	HAL_WIFI_DBG(("%s: radioIndex = %d, send csa csa_beacon_count=%d\n",
		__FUNCTION__, radioIndex, csa_beacon_count));

	len = sizeof(csaInfo);
	ret = wldm_11h_dfs(CMD_SET_IOCTL, radioIndex, &csaInfo, (unsigned int *)&len, "csa_bcast", NULL);
	if (ret != 0) {
		HAL_WIFI_ERR(("%s: Error csa radioIndex=%d\n", __FUNCTION__, radioIndex));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

INT wifi_getRadioChannelStats(INT radioIndex, wifi_channelStats_t *input_output_channelStats_array, INT array_size)
{
	RADIO_INDEX_ASSERT(radioIndex);
	HAL_WIFI_DBG(("%s, radioIndex = %d\n", __FUNCTION__, radioIndex));
	NULL_PTR_ASSERT(input_output_channelStats_array);
	RADIO_NOT_UP_ASSERT(radioIndex);

	if (wldm_xbrcm_lq(CMD_GET, radioIndex, (void *)input_output_channelStats_array, (unsigned int *)&array_size, "channel_stats", NULL) != 0) {
		HAL_WIFI_ERR(("%s: wldm_xbrcm_lq returned error\n", __FUNCTION__));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

INT wifi_getRadioChannelStats2(INT radioIndex, wifi_channelStats2_t *outputChannelStats2)
{
	unsigned int len = sizeof(wifi_channelStats2_t);
	wldm_channel_stats2_t channel_stats2;

	HAL_WIFI_DBG(("%s, radioIndex = %d\n", __FUNCTION__, radioIndex));

	NULL_PTR_ASSERT(outputChannelStats2);
	RADIO_INDEX_ASSERT(radioIndex);
	RADIO_NOT_UP_ASSERT(radioIndex);

	memset(&channel_stats2, 0, sizeof(channel_stats2));
	if (wldm_xbrcm_lq(CMD_GET, radioIndex, (void *)&channel_stats2, &len, "channel_stats2", NULL) != 0) {
		HAL_WIFI_ERR(("%s: channel_stats2 CMD_GET returned error\n", __FUNCTION__));
		return RETURN_ERR;
	}
	memset(outputChannelStats2, 0, sizeof(*outputChannelStats2));
	outputChannelStats2->ch_Frequency = channel_stats2.ch_Frequency;
	outputChannelStats2->ch_NoiseFloor = channel_stats2.ch_NoiseFloor;
	outputChannelStats2->ch_Non80211Noise = channel_stats2.ch_Non80211Noise;
	outputChannelStats2->ch_Max80211Rssi = channel_stats2.ch_Max80211Rssi;
	outputChannelStats2->ch_ObssUtil = channel_stats2.ch_ObssUtil;
	outputChannelStats2->ch_SelfBssUtil = channel_stats2.ch_SelfBssUtil;
	return RETURN_OK;
}

static BOOL radio_stats[HAL_RADIO_NUM_RADIOS];

INT wifi_getRadioStatsEnable(INT radioIndex, BOOL *output_enable)
{
	RADIO_INDEX_ASSERT(radioIndex);

	*output_enable = radio_stats[radioIndex];
	return RETURN_OK;
}

INT wifi_setRadioStatsEnable(INT radioIndex, BOOL enable)
{
	RADIO_INDEX_ASSERT(radioIndex);

	radio_stats[radioIndex] = enable;
	return RETURN_OK;
}

/* Save csa_deauth_mode in nvram
 * Used to decide whether to deauth associated clients before sending csa
 */
INT wifi_setApCsaDeauth(INT apIndex, INT mode)
{
	CHAR nvName[OUTPUT_STRING_LENGTH_64] = {0};
	CHAR deauth_modeStr[OUTPUT_STRING_LENGTH_64];

	HAL_WIFI_DBG(("%s, apIndex [%d] mode[%d] \n", __FUNCTION__, apIndex, mode));

	AP_INDEX_ASSERT(apIndex);

	if ((mode < WIFI_CSA_DEAUTH_MODE_NONE) || (mode > WIFI_CSA_DEAUTH_MODE_BCAST)) {
		HAL_WIFI_ERR(("%s: apIndex [%d] INCORRECT mode [%d]\n", __FUNCTION__, apIndex, mode));
		return RETURN_ERR;
	}

	snprintf(deauth_modeStr, sizeof(deauth_modeStr), "%d", mode);
	snprintf(nvName, sizeof(nvName), "wl%d_%s", HAL_AP_IDX_TO_HAL_RADIO(apIndex), "csa_deauth_mode");

	/* save config in nvram */
	if (nvram_set(nvName, deauth_modeStr)) {
		HAL_WIFI_ERR(("%s: %s=%s set err\n", __FUNCTION__, nvName, deauth_modeStr));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

INT wifi_setApScanFilter(INT apIndex, INT mode, CHAR *essid)
{
	HAL_WIFI_DBG(("%s: apIndex = %d mode:%d essid:%s\n", __FUNCTION__, apIndex, mode, essid));

	AP_INDEX_ASSERT(apIndex);

	if ((mode > WIFI_SCANFILTER_MODE_FIRST || mode < 0)) {
		HAL_WIFI_ERR(("%s, INCORRECT mode [%d] \n", __FUNCTION__, mode));
		return RETURN_ERR;
	}

	gscanfilter[apIndex].mode = mode;
	memset(gscanfilter[apIndex].essid, '\0', OUTPUT_STRING_LENGTH_32);
	if (essid == NULL) {
		wifi_getSSIDName(apIndex, gscanfilter[apIndex].essid);
	} else {
		strncpy(gscanfilter[apIndex].essid, essid, OUTPUT_STRING_LENGTH_32);
	}
	return RETURN_OK;
}

BOOL wifi_steering_supported(void)
{
	return RETURN_OK;
}

#ifdef WIFI_HAL_VERSION_3_PHASE2
INT wifi_steering_setGroup(UINT steeringgroupIndex, UINT numElements, wifi_steering_apConfig_t *cfgArray)
{
	int i;
	UCHAR buf[MAX_EVENT_BUFFER_LEN];
	wifi_hal_message_t *ptr;
	wifi_steering_setGroup_t *setgroup;
	wifi_steering_apConfig_t *apcfg_ptr;
	UINT32 tlen;

	if ((numElements == 0) || (numElements > HAL_RADIO_NUM_RADIOS)) {
		HAL_WIFI_ERR(("%s: numElements=%d not correct\n",
			__FUNCTION__, numElements));
		return WIFI_HAL_ERROR;
	}

	ptr = (wifi_hal_message_t *)buf;
	ptr->hal_msg_type = WIFI_HAL_MSG_AP_CONFIG;
	ptr->hal_msg_len = sizeof(steeringgroupIndex) +
		numElements * sizeof(wifi_steering_apConfig_t);

	tlen = sizeof(wifi_hal_message_t) + ptr->hal_msg_len;
	if (tlen > sizeof(buf)) {
		HAL_WIFI_ERR(("%s: total len (%d) too long\n", __FUNCTION__, tlen));
		return WIFI_HAL_ERROR;
	}

	setgroup = (wifi_steering_setGroup_t *)ptr->data;
	setgroup->steeringgroupIndex = steeringgroupIndex;

	for (i = 0; i < numElements; i++) {
		apcfg_ptr = &(setgroup->cfg[i]);
		memcpy(apcfg_ptr, &cfgArray[i], sizeof(wifi_steering_apConfig_t));

		HAL_WIFI_LOG(("%s: index=%d apcfg_ptr=%p apidx=%d intv=%d cnt=%d inact_sec=%d inact_th=%d\n",
			__FUNCTION__, i, apcfg_ptr, apcfg_ptr->apIndex,
			apcfg_ptr->utilCheckIntervalSec, apcfg_ptr->utilAvgCount,
			apcfg_ptr->inactCheckIntervalSec, apcfg_ptr->inactCheckThresholdSec));
	}

	HAL_WIFI_LOG(("%s: hal_msg_len=%d tlen=%d buf=%p buf_ptr=%p\n",
		__FUNCTION__, ptr->hal_msg_len, tlen, buf, apcfg_ptr));

	wifi_hal_notify_ecbd((void *)buf, tlen, WIFI_HAL_TO_ECBD_MSG_UDP_PORT);

	return WIFI_HAL_SUCCESS;
}
#else /* WIFI_HAL_VERSION_3_PHASE2 */
INT wifi_steering_setGroup(UINT steeringgroupIndex, wifi_steering_apConfig_t *cfg_2, wifi_steering_apConfig_t *cfg_5)
{
	UCHAR buf[MAX_EVENT_BUFFER_LEN];
	wifi_hal_message_t *ptr;
	wifi_steering_setGroup_t *setgroup;
	UINT32 tlen;
	wifi_steering_apConfig_t *apcfg_ptr;

	ptr = (wifi_hal_message_t *)buf;
	ptr->hal_msg_type = WIFI_HAL_MSG_AP_CONFIG;

	if (cfg_2 == NULL && cfg_5 == NULL)
		ptr->hal_msg_len = sizeof(steeringgroupIndex); /* as a flag to remove */
	else {
		ptr->hal_msg_len = sizeof(steeringgroupIndex) +
			2 * sizeof(wifi_steering_apConfig_t);
	}

	tlen = sizeof(wifi_hal_message_t) + ptr->hal_msg_len;
	if (tlen > sizeof(buf)) {
		HAL_WIFI_ERR(("%s: total len (%d) too long\n", __FUNCTION__, tlen));
		return RETURN_ERR;
	}

	setgroup = (wifi_steering_setGroup_t *)ptr->data;
	setgroup->steeringgroupIndex = steeringgroupIndex;

	apcfg_ptr = &(setgroup->cfg[0]);
	if (cfg_2 != NULL) {
		memcpy(apcfg_ptr, cfg_2, sizeof(wifi_steering_apConfig_t));
	}

	apcfg_ptr = &(setgroup->cfg[1]);
	if (cfg_5 != NULL) {
		memcpy(apcfg_ptr, cfg_5, sizeof(wifi_steering_apConfig_t));
	}

	HAL_WIFI_LOG(("%s:%d: hal_msg_len=%d, tlen=%d, buf=%p, buf_ptr=%p\n",
		__FUNCTION__, __LINE__, ptr->hal_msg_len, tlen, buf, apcfg_ptr));

	wifi_hal_notify_ecbd((void *)buf, tlen, WIFI_HAL_TO_ECBD_MSG_UDP_PORT);

	return RETURN_OK;
}
#endif /* WIFI_HAL_VERSION_3_PHASE2 */

INT wifi_steering_clientSet(UINT steeringgroupIndex,INT apIndex,mac_address_t client_mac,wifi_steering_clientConfig_t *config)
{
	CHAR buf[MAX_EVENT_BUFFER_LEN], *buf_ptr;
	wifi_hal_message_t *ptr;
	wifi_steering_client_t *cli_ptr;
	UINT32 tlen;

	NULL_PTR_ASSERT(config);

	ptr = (wifi_hal_message_t *)buf;
	ptr->hal_msg_type = WIFI_HAL_MSG_CLIENT_SET;

	ptr->hal_msg_len = sizeof(wifi_steering_client_t);
	cli_ptr = (wifi_steering_client_t *)ptr->data;
	cli_ptr->groupIndex = steeringgroupIndex;
	cli_ptr->apIndex = apIndex;
	memcpy(cli_ptr->cli_mac, client_mac, sizeof(mac_address_t));

	buf_ptr = cli_ptr->data;
	memcpy(buf_ptr, config, sizeof(wifi_steering_clientConfig_t));
	ptr->hal_msg_len += sizeof(wifi_steering_clientConfig_t);

	tlen = sizeof(wifi_hal_message_t) + ptr->hal_msg_len;

	HAL_WIFI_LOG(("%s:%d: hal_msg_len=%d, tlen=%d, buf=%p, buf_ptr=%p, "MACF"\n",
		__FUNCTION__, __LINE__, ptr->hal_msg_len, tlen, buf, buf_ptr, MAC_TO_MACF(cli_ptr->cli_mac)));

	wifi_hal_notify_ecbd((void *)buf, tlen, WIFI_HAL_TO_ECBD_MSG_UDP_PORT);

	return RETURN_OK;
}

/* for Remove and Measure */
static void wifi_steering_client_common(UINT steeringgroupIndex,INT apIndex,mac_address_t client_mac, UINT32 msg_type)
{
	UCHAR buf[MAX_EVENT_BUFFER_LEN];
	wifi_hal_message_t *ptr;
	wifi_steering_client_t *cli_ptr;
	UINT32 tlen;

	ptr = (wifi_hal_message_t *)buf;
	ptr->hal_msg_type = msg_type;

	ptr->hal_msg_len = sizeof(wifi_steering_client_t);
	cli_ptr = (wifi_steering_client_t *)ptr->data;
	cli_ptr->groupIndex = steeringgroupIndex;
	cli_ptr->apIndex = apIndex;
	memcpy(cli_ptr->cli_mac, client_mac, sizeof(mac_address_t));

	tlen = sizeof(wifi_hal_message_t) + ptr->hal_msg_len;

	HAL_WIFI_LOG(("%s:%d: hal_msg_len=%d, tlen=%d, buf=%p, "MACF"\n",
		__FUNCTION__, __LINE__, ptr->hal_msg_len, tlen, buf, MAC_TO_MACF(cli_ptr->cli_mac)));

	wifi_hal_notify_ecbd((void *)buf, tlen, WIFI_HAL_TO_ECBD_MSG_UDP_PORT);
}

INT wifi_steering_clientRemove(UINT steeringgroupIndex,INT apIndex,mac_address_t client_mac)
{
	wifi_steering_client_common(steeringgroupIndex, apIndex, client_mac, WIFI_HAL_MSG_CLIENT_RM);
	return RETURN_OK;
}

INT wifi_steering_clientMeasure(UINT steeringgroupIndex,INT apIndex,mac_address_t client_mac)
{
	wifi_steering_client_common(steeringgroupIndex, apIndex, client_mac, WIFI_HAL_MSG_CLIENT_MEAS);
	return RETURN_OK;
}

INT wifi_steering_clientDisconnect(UINT steeringgroupIndex,INT apIndex,mac_address_t client_mac,wifi_disconnectType_t type,UINT reason)
{
	CHAR buf[MAX_EVENT_BUFFER_LEN], *buf_ptr;
	wifi_hal_message_t *ptr;
	wifi_steering_client_t *cli_ptr;
	UINT32 tlen;

	ptr = (wifi_hal_message_t *)buf;
	ptr->hal_msg_type = WIFI_HAL_MSG_CLIENT_DISCONN;

	ptr->hal_msg_len = sizeof(wifi_steering_client_t);
	cli_ptr = (wifi_steering_client_t *)ptr->data;
	cli_ptr->groupIndex = steeringgroupIndex;
	cli_ptr->apIndex = apIndex;
	memcpy(cli_ptr->cli_mac, client_mac, sizeof(mac_address_t));

	/* type and reason */
	buf_ptr = cli_ptr->data;
	memcpy(buf_ptr, &type, sizeof(wifi_disconnectType_t));
	ptr->hal_msg_len += sizeof(wifi_disconnectType_t);

	buf_ptr += sizeof(wifi_disconnectType_t);
	memcpy(buf_ptr, &reason, sizeof(UINT));
	ptr->hal_msg_len += sizeof(UINT);

	tlen = sizeof(wifi_hal_message_t) + ptr->hal_msg_len;

	HAL_WIFI_LOG(("%s:%d: hal_msg_len=%d, tlen=%d, buf=%p, buf_ptr=%p, "MACF"\n",
		__FUNCTION__, __LINE__, ptr->hal_msg_len, tlen, buf, buf_ptr, MAC_TO_MACF(cli_ptr->cli_mac)));

	wifi_hal_notify_ecbd((void *)buf, tlen, WIFI_HAL_TO_ECBD_MSG_UDP_PORT);

	return RETURN_OK;
}

/* send message to ecbd */
static int
wifi_hal_notify_ecbd(void *hal_msg, uint len, unsigned short port)
{
	struct sockaddr_in sockaddr;
	unsigned int sentBytes = 0;
	int ecbd_sock = -1;

	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	sockaddr.sin_port = htons(port);

	if ((ecbd_sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		HAL_WIFI_ERR(("%s:%d: Unable to create socket\n", __FUNCTION__, __LINE__));
		return RETURN_ERR;
	}

	if (ecbd_sock >= 0) {
		sentBytes = sendto(ecbd_sock, hal_msg, len, 0,
				(struct sockaddr *)&sockaddr, sizeof(struct sockaddr_in));

		if (sentBytes != len) {
			HAL_WIFI_DBG(("%s:%d: UDP send failed; sendingBytes[%d], sentBytes[%d]\n",
				__FUNCTION__, __LINE__, len, sentBytes));
		} else {
			HAL_WIFI_DBG(("%s:%d: send wifi_hal message to ecbd, len=[%d]\n",
				__FUNCTION__, __LINE__, len));
		}
	}

	if (ecbd_sock >= 0) {
		close(ecbd_sock);
	}

	return RETURN_OK;
}

/* for WM wifi_newApAssociatedDevice_callback */
#define MAX_WM_EVENT_BUFFER_LEN 4096

/* replace wifi_eventHandlerCallback */
static int
wifi_hal_cb_assoc_dev_evt_handler(int sock_fd)
{
	int ret = 0, bytes = 0;
	char *buf_ptr = NULL, *ptr;
	WL_STATION_LIST_ENTRY *wlStaList = NULL;
	int idx, sub_idx, event_type, apIndex;
	char OperBandwidth[64];
	char macStr[MAC_STR_LEN];
	WL_STA_EVENT_DETAIL *p_sta_detail;
#ifdef WIFI_HAL_VERSION_3_PHASE2
	wifi_associated_dev3_t dev;
#else /* WIFI_HAL_VERSION_3_PHASE2 */
	wifi_associated_dev_t dev;
#endif /* WIFI_HAL_VERSION_3_PHASE2 */
	int k, len_assoc, len_disassoc;
	char encMode[64];
	memset(encMode, 0, sizeof(encMode));

	buf_ptr = (void *)calloc(MAX_WM_EVENT_BUFFER_LEN, 1);
	if (!buf_ptr) {
		HAL_WIFI_ERR(("%s:%d: malloc fails!\n", __FUNCTION__, __LINE__));
		return RETURN_ERR;
	}

	len_disassoc = sizeof(WL_STA_EVENT_DETAIL);
	len_assoc = sizeof(WL_STA_EVENT_DETAIL) + sizeof(WL_STATION_LIST_ENTRY);

	if ((bytes = recv(sock_fd, buf_ptr, MAX_WM_EVENT_BUFFER_LEN, MSG_NOSIGNAL)) > 0) {
		buf_ptr[MAX_WM_EVENT_BUFFER_LEN-1] = '\0';

		HAL_WIFI_DBG(("%s:%d: receive total_len=%d\n", __FUNCTION__, __LINE__, bytes));
		ptr = buf_ptr;

		/* one buffer may contain several events */
		while (bytes >= len_disassoc) {
			p_sta_detail = (WL_STA_EVENT_DETAIL *)(ptr);
			event_type = p_sta_detail->event_type;
			idx = p_sta_detail->radio_idx;
			sub_idx = p_sta_detail->sub_idx;

			apIndex = WL_DRIVER_TO_AP_IDX(idx, sub_idx) - 1; /* hal apIndex starting from 0 */
			/*
			 * Do not use the AP_INDEX_ASSERT(apIndex) macro here
			 * for we may leak in case of a failure
			 * so implement the macro here so we don't leak upon error.
			 */
			if ((apIndex >= (HAL_GET_MAX_APS)) || (apIndex < 0)) {
				HAL_WIFI_ERR(("%s, INCORRECT apIndex [%d] \n", __FUNCTION__, apIndex));
				free(buf_ptr);
				return RETURN_ERR;
			}

			snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X", p_sta_detail->mac[0], p_sta_detail->mac[1], p_sta_detail->mac[2], p_sta_detail->mac[3], p_sta_detail->mac[4], p_sta_detail->mac[5]);

			HAL_WIFI_DBG(("%s:%d: recev event on wl%d.%d ApIndex=%d Event=%d STA MAC=[%s] bytes=%d\n",
				__FUNCTION__, __LINE__, idx, sub_idx, apIndex, event_type, macStr, bytes));

			if ((event_type == WLC_E_ASSOC_IND) || (event_type == WLC_E_REASSOC_IND)) {
				if (bytes >= len_assoc) {
					/* use the info passed from ecbd */
					wlStaList = (WL_STATION_LIST_ENTRY *)(ptr + sizeof(WL_STA_EVENT_DETAIL));

					/* data to dev */
					memset(&dev, 0, sizeof(dev));
					for (k = 0; k < WIFI_MAC_ADDRESS_LENGTH; k++) {
						dev.cli_MACAddress[k] = (UCHAR)p_sta_detail->mac[k];
					}

					memset(dev.cli_IPAddress, 0, 64);
					dev.cli_AuthenticationState = wlStaList->authenticationState;
					dev.cli_Active = wlStaList->active;
					dev.cli_SignalStrength = wlStaList->signalStrength;
					dev.cli_LastDataDownlinkRate = (wlStaList->lastDataDownlinkRate)/1000;
					dev.cli_LastDataUplinkRate = (wlStaList->lastDataUplinkRate)/1000;
					dev.cli_Retransmissions = wlStaList->retransmissions;
					dev.cli_RSSI= wlStaList->signalStrength;

					snprintf(dev.cli_OperatingStandard, sizeof(dev.cli_OperatingStandard),
						"%s", wlStaList->operStandard);

					snprintf(OperBandwidth, sizeof(OperBandwidth), "%d",
						wlStaList->operBandwidth);

					snprintf(dev.cli_OperatingChannelBandwidth, sizeof(dev.cli_OperatingChannelBandwidth),
						"%s", OperBandwidth);

					dev.cli_SNR= wlStaList->snr;
					snprintf(dev.cli_InterferenceSources, sizeof(dev.cli_InterferenceSources), "%s", "");
					dev.cli_DataFramesSentAck=0;
					dev.cli_DataFramesSentNoAck=0;
					dev.cli_BytesSent=0;
					dev.cli_BytesReceived=0;
#ifdef WIFI_HAL_VERSION_GE_3_0_1
/* Temporary fix until HUB4 updates driver to build from source */
#ifndef _SR300_PRODUCT_REQ_
					dev.cli_CapableNumSpatialStreams = wlStaList->nss;
#endif /* _SR300_PRODUCT_REQ_ */
#endif /* WIFI_HAL_VERSION_GE_3_0_1 */

					if ((wifi_numRegisteredAssociatedCallbacks != 0) &&
						(wifi_getApSecurityModeEnabled(apIndex, encMode) == RETURN_OK)) {
						if ((wlStaList->authenticationState == TRUE) ||
							((wlStaList->authenticationState == FALSE) &&
							(strncmp(encMode, "None", sizeof(encMode)) == 0))) {
							for (k = 0; k < wifi_numRegisteredAssociatedCallbacks; k++) {
								if (wifi_app_associated_callbacks[k] == NULL) {
									HAL_WIFI_DBG(("%s: wifi_associated_callback is not registered \n", __FUNCTION__));
									continue;
								}
								HAL_WIFI_LOG(("%s:%d assoc event for %s invoke callback (k=%d)\n",
									__FUNCTION__, __LINE__, macStr, k));
								wifi_app_associated_callbacks[k](apIndex, &dev);
							}
						}
					}
					bytes -= len_assoc;
					ptr += len_assoc;
					continue;
				}
				else {
					HAL_WIFI_DBG(("message length too short[%d]", bytes));
					break;
				}
			} else if (event_type == WLC_E_DISASSOC_IND) {
					if (wifi_numRegisteredDisassociatedCallbacks != 0 ) {
						HAL_WIFI_LOG(("%s: disconnected event %d for %s (total cb %d)\n",
							__FUNCTION__, event_type, macStr, wifi_numRegisteredDisassociatedCallbacks));
#if WIFI_HAL_VERSION_GE_2_12 || defined(WIFI_HAL_VERSION_3)
						for (k = 0; k < wifi_numRegisteredDisassociatedCallbacks; k++) {
							if (wifi_app_disassociated_callbacks[k] == NULL) {
								HAL_WIFI_DBG(("%s: wifi_associated_callback (k=%d) is not registered \n", __FUNCTION__, k));
								continue;
							}

							HAL_WIFI_LOG(("%s: invoke disassoc callback for link k=%d\n", __FUNCTION__, k));
							wifi_app_disassociated_callbacks[k](apIndex, macStr, 0);
						}
#endif
					}
					bytes -= len_disassoc;
					ptr += len_disassoc;
					continue;
			} else {
				HAL_WIFI_DBG(("%s:%d: Invalid STA event[%d]!\n", __FUNCTION__, __LINE__, event_type));
				break;
			}
		} /* while */
	}
	else if (bytes < 0) {
		perror("recv failed ");
		ret = -errno;
	}
	else if (bytes == 0) {
		char buf[32] = {0};
		bytes = snprintf(buf, 32, "RU There");
		ret = send(sock_fd, buf, bytes, MSG_NOSIGNAL);
		if ((ret < 0) && (errno == EPIPE))
			ret = -EPIPE;
	}

	if (buf_ptr)
		free(buf_ptr);

	return ret;
}

static int
wifi_hal_cb_sta_conn_evt_handler (int sock_fd) {

	INT ret = 0;
	INT bytes = 0;
	UCHAR buf_ptr[MAX_EVENT_BUFFER_LEN], *ptr = buf_ptr;
	wifi_hal_cb_evt_t *dpkt;
	UINT32 event_id;

	if ((bytes = recv(sock_fd, ptr, MAX_EVENT_BUFFER_LEN, MSG_NOSIGNAL)) > 0) {

		dpkt = (wifi_hal_cb_evt_t *)ptr;
		if (dpkt->version == WIFI_HAL_EVT_VERSION) {
			event_id = dpkt->type;

			switch (event_id) {
				case WIFI_HAL_CB_STA_CONN:
					if ((dpkt->reason >= CONN_NEW) && (dpkt->reason <= CONN_RECONN_AFTER_INACTIVITY)) {
						HAL_WIFI_LOG(("%s: %d: apIndex:%d mac:%s reason:%d\n", __FUNCTION__, __LINE__, dpkt->apIndex, dpkt->mac, dpkt->reason));
						callback_fnc.associate_cb(dpkt->apIndex, dpkt->mac, dpkt->reason);
					} else
						HAL_WIFI_ERR(("%s: %d: Invalid Reason: %d\n", __FUNCTION__, __LINE__, dpkt->reason));
					break;

				default:
					HAL_WIFI_DBG(("%s:%d: Unsupported Event: %d \n", __FUNCTION__, __LINE__, event_id));
					break;
			}
		}
	}
	else if (bytes < 0) {
		perror("recv failed ");
		ret = -errno;
	}
	else if (bytes == 0) {
		char buf[32] = {0};
		bytes = snprintf(buf, 32, "RU There");
		ret = send(sock_fd, buf, bytes, MSG_NOSIGNAL);
		if ((ret < 0) && (errno == EPIPE))
			ret = -EPIPE;
	}
	return ret;
}

static int
wifi_hal_cb_auth_evt_handler(int sock_fd)
{
	int ret = 0, bytes = 0;
	UCHAR buf_ptr[MAX_EVENT_BUFFER_LEN], *ptr = buf_ptr;
	wifi_hal_cb_evt_t *dpkt;
	UINT32 event_id;

	if ((bytes = recv(sock_fd, ptr, MAX_EVENT_BUFFER_LEN, 0)) > 0) {

		dpkt = (wifi_hal_cb_evt_t *)ptr;
		if (dpkt->version == WIFI_HAL_EVT_VERSION) {
			event_id = dpkt->type;

			switch (event_id) {
			case WIFI_HAL_CB_AUTH_FAIL:
				HAL_WIFI_DBG(("%s: %d: apIndex:%d mac:%s reason:%d\n",
					__FUNCTION__, __LINE__, dpkt->apIndex, dpkt->mac, dpkt->reason));
				AP_INDEX_ASSERT(dpkt->apIndex);

				if (callback_fnc.auth_cb != NULL)
					callback_fnc.auth_cb(dpkt->apIndex, dpkt->mac, dpkt->reason);
				break;
			default:
				HAL_WIFI_DBG(("%s:%d: Unsupported Event: %d \n", __FUNCTION__, __LINE__, event_id));
				break;
			}
		}
	} else if (bytes < 0) {
		perror("recv failed ");
		ret = -errno;
	} else if (bytes == 0) {
		/* return -EPIPE if socket is broken, so that caller can close the socket */
		char buf[32] = {0};
		bytes = snprintf(buf, 32, "RU There");
		ret = send(sock_fd, buf, bytes, MSG_NOSIGNAL);
		if ((ret < 0) && (errno == EPIPE)) {
			ret = -EPIPE;
			HAL_WIFI_LOG(("%s: socket=%d broken errno=%d\n",
				__FUNCTION__, sock_fd, errno));
		}
	}

	return ret;
}

static int
wifi_hal_cb_mesh_evt_handler(int sock_fd)
{
	int ret = 0, bytes = 0;
	UCHAR buf_ptr[MAX_EVENT_BUFFER_LEN], *ptr = buf_ptr;
	wifi_steering_event_t *dpkt;
	UINT32 event_id, apIndex, group_idx;
	ULLONG stamp;

	if ((bytes = recv(sock_fd, ptr, MAX_EVENT_BUFFER_LEN, 0)) > 0) {

		group_idx = *(UINT32 *)ptr;
		dpkt = (wifi_steering_event_t *)(ptr + sizeof(UINT32));
		event_id = dpkt->type;
		apIndex = dpkt->apIndex;
		stamp = dpkt->timestamp_ms;
		HAL_WIFI_DBG(("%s: %d: group_idx=%d, event_id=%d, apIndex=%d, stamp=%llu\n",
			__FUNCTION__, __LINE__, group_idx, event_id, apIndex, stamp));

		switch (event_id) {
			case WIFI_STEERING_EVENT_PROBE_REQ:
				HAL_WIFI_DBG(("%s: %d: event_id=%d, MAC="MACF", rssi=%d\n", __FUNCTION__, __LINE__, event_id,
					MAC_TO_MACF(dpkt->data.probeReq.client_mac), dpkt->data.probeReq.rssi));
				break;
			case WIFI_STEERING_EVENT_CLIENT_CONNECT:
				HAL_WIFI_DBG(("%s: %d: event_id=%d, MAC="MACF"\n", __FUNCTION__, __LINE__, event_id,
					MAC_TO_MACF(dpkt->data.connect.client_mac)));
				break;
			case WIFI_STEERING_EVENT_CLIENT_DISCONNECT:
				HAL_WIFI_DBG(("%s: %d: event_id=%d, MAC="MACF", reason=%d\n", __FUNCTION__, __LINE__, event_id,
					MAC_TO_MACF(dpkt->data.disconnect.client_mac), dpkt->data.disconnect.reason));
				break;
			case WIFI_STEERING_EVENT_CLIENT_ACTIVITY:
				HAL_WIFI_DBG(("%s: %d: event_id=%d, MAC="MACF", active=%s\n", __FUNCTION__, __LINE__, event_id,
					MAC_TO_MACF(dpkt->data.activity.client_mac), (dpkt->data.activity.active == TRUE)?"Yes":"No"));
				break;
			case WIFI_STEERING_EVENT_CHAN_UTILIZATION:
				HAL_WIFI_DBG(("%s: %d: CHAN_UTILIZATION: %d\n", __FUNCTION__, __LINE__, dpkt->data.chanUtil.utilization));
				break;
			case WIFI_STEERING_EVENT_RSSI_XING:
				HAL_WIFI_DBG(("%s: %d: event_id=%d, MAC="MACF", rssi=%d (in=%d hi=%d lo=%d)\n", __FUNCTION__, __LINE__, event_id,
					MAC_TO_MACF(dpkt->data.rssiXing.client_mac), dpkt->data.rssiXing.rssi,
					dpkt->data.rssiXing.inactveXing, dpkt->data.rssiXing.highXing, dpkt->data.rssiXing.lowXing));
				break;
			case WIFI_STEERING_EVENT_RSSI:
				HAL_WIFI_DBG(("%s: %d: event_id=%d, MAC="MACF", rssi=%d\n", __FUNCTION__, __LINE__, event_id,
					MAC_TO_MACF(dpkt->data.rssi.client_mac), dpkt->data.rssi.rssi));
				break;
			case WIFI_STEERING_EVENT_AUTH_FAIL:
				HAL_WIFI_DBG(("%s: %d: event_id=%d, MAC="MACF"\n", __FUNCTION__, __LINE__, event_id,
					MAC_TO_MACF(dpkt->data.authFail.client_mac)));
				break;
			default:
				HAL_WIFI_DBG(("%s:%d: Unsupported Event: %d \n", __FUNCTION__, __LINE__, event_id));
				break;
		}

		if (callback_fnc.mesh_cb != NULL)
			callback_fnc.mesh_cb(group_idx, dpkt);
	} else if (bytes < 0) {
		perror("recv failed ");
		ret = -errno;
	} else if (bytes == 0) {
		/* return -EPIPE if socket is broken, so that caller can close the socket */
		char buf[32] = {0};
		bytes = snprintf(buf, 32, "RU There");
		ret = send(sock_fd, buf, bytes, MSG_NOSIGNAL);
		if ((ret < 0) && (errno == EPIPE)) {
			ret = -EPIPE;
			HAL_WIFI_LOG(("%s: socket=%d broken errno=%d\n",
				__FUNCTION__, sock_fd, errno));
		}
	}

	return ret;
}

static int
wifi_log_rrm_event(char *logStr) {
	char *str = NULL;
	FILE *rrm_log_fd = NULL;
	char date_str[RDKB_RRM_STRING_LENGTH] = {0};
	time_t t = time(NULL);
	struct tm time_info = {0};
	localtime_r(&t, &time_info);

	if (rrm_log_fd == NULL) {
		if ((str = nvram_get(WLAN_CM_RDK_RRM_LOG_FILE_NVRAM))) {
			rrm_log_fd = fopen(str, "a");
		}
		else {
			rrm_log_fd = fopen(WLAN_CM_RDK_RRM_LOG_FILE, "a");
		}
	}

	if (rrm_log_fd != NULL) {
		snprintf(date_str, sizeof(date_str), "%d-%d-%d %d:%d:%d\n",
				 time_info.tm_year + 1900, time_info.tm_mon + 1, time_info.tm_mday,
				 time_info.tm_hour, time_info.tm_min, time_info.tm_sec);
		fprintf(rrm_log_fd, "%9lu %s %s \n", (unsigned long)(time(NULL)), date_str, logStr);
		fclose(rrm_log_fd);
	}
	return 0;
}

#if WIFI_HAL_VERSION_GE_2_12 || defined(WIFI_HAL_VERSION_3)
/* 802.11K api */

#define UNPACK_BEACON_REPORT(bcnRep_packed, bcnRep) \
	bcnRep->opClass = bcnRep_packed->opClass; \
	bcnRep->channel = bcnRep_packed->channel; \
	bcnRep->startTime = bcnRep_packed->startTime; \
	bcnRep->duration = bcnRep_packed->duration; \
	bcnRep->frameInfo = bcnRep_packed->frameInfo; \
	bcnRep->rcpi = bcnRep_packed->rcpi; \
	bcnRep->rsni = bcnRep_packed->rsni; \
	memcpy(bcnRep->bssid, bcnRep_packed->bssid, sizeof(bcnRep->bssid)); \
	bcnRep->antenna = bcnRep_packed->antenna; \
	bcnRep->tsf = bcnRep_packed->tsf; \
	/* TODO parse the following two fields \
	bcnRep->wideBandWidthChannelPresent = FALSE; \
	bcnRep->numRepetitions = 0 */

/* support multiple beacon reports in one event payload (RM frame) */
#define MAX_BEACON_REPORT_CNT 16
static int
wifi_hal_parse_rm_beacon_report(char *pkt, int pkt_len, wifi_BeaconReport_t **bcn_rep, UINT *cnt, UCHAR *token)
{
	INT evt_len, rpt_len, len = 0, buflen;
	UINT est_cnt = 0, idx = 0;
	UCHAR first_token = 0;
	char *ie_ptr;
	wifi_BeaconReport_t *bcn_rep_array = NULL, *bcnRep;
	wifi_MeasurementReportElement_t *measRep;
	wifi_BeaconMeasurement_packed_t *bcnRep_packed;

	/* the pkt may contains several cb events */
	rpt_len = sizeof(wifi_MeasurementReportElement_t) + DOT11_RMREP_BCN_LEN; /* 5 + 26 */
	evt_len = sizeof(wifi_hal_cb_evt_t) + rpt_len;  /* 36+, it's single event length */
	est_cnt = pkt_len / evt_len;
	est_cnt = (est_cnt > MAX_BEACON_REPORT_CNT) ? MAX_BEACON_REPORT_CNT : est_cnt;

	/* allocate buffer */
	buflen = est_cnt * sizeof(wifi_BeaconReport_t);
	bcn_rep_array = (wifi_BeaconReport_t *)malloc(buflen);
	if (bcn_rep_array == NULL) {
		HAL_WIFI_ERR(("%s: fail to alloc buffer len=%d est_cnt=%d buflen=%d\n",
			__FUNCTION__, len, est_cnt, buflen));
		return RETURN_ERR;
	}
	memset(bcn_rep_array, 0, buflen);

	HAL_WIFI_DBG(("%s: pkt_len=%d est_cnt=%d buflen=%d bcn_rep_array=%p\n",
		__FUNCTION__, pkt_len, est_cnt, buflen, bcn_rep_array));

	/* point to first report */
	ie_ptr = pkt + sizeof(wifi_hal_cb_evt_t);
	len = pkt_len - sizeof(wifi_hal_cb_evt_t);

	HAL_WIFI_DBG(("%s: pkt=%p ie_ptr=%p len=%d ie_ptr (0-3): 0x%x 0x%x 0x%x 0x%x\n",
		__FUNCTION__, pkt, ie_ptr, len, *ie_ptr, *(ie_ptr+1), *(ie_ptr+2), *(ie_ptr+3)));

	while (len >= rpt_len) {
		measRep = (wifi_MeasurementReportElement_t *)(ie_ptr);

		HAL_WIFI_DBG(("%s: idx=%d Report rep_Id=0x%x rep_Type=0x%x len=%d\n",
			__FUNCTION__, idx, measRep->rep_Id, measRep->rep_Type, len));

		HAL_WIFI_DBG(("%s: ie_ptr=%p ie_ptr (0-3): 0x%x 0x%x 0x%x 0x%x\n",
			__FUNCTION__, ie_ptr, *ie_ptr, *(ie_ptr+1), *(ie_ptr+2), *(ie_ptr+3)));

		if ((measRep->rep_Id == DOT11_MNG_MEASURE_REPORT_ID) && /* 0x27 (39) */
			(measRep->rep_Type == DOT11_MEASURE_TYPE_BEACON)) { /* 5 */

			if (measRep->rep_Len <
				sizeof(wifi_BeaconMeasurement_packed_t)) {
					HAL_WIFI_ERR(("%s: Received Beacon"
					" Report is too small %u bytes\n",
					__FUNCTION__, measRep->rep_Len));
				break;
			}
			else {
				bcnRep = &bcn_rep_array[idx];
				bcnRep_packed = (wifi_BeaconMeasurement_packed_t *)
					measRep->rep_Report;
				UNPACK_BEACON_REPORT(bcnRep_packed, bcnRep);

				/* print bssid to verify */
				HAL_WIFI_DBG(("%s: Beacon Report: idx=%d bcnRep=%p channel=%d "
					"BSSID="MACF" sizeof(wifi_BeaconReport_t)=%d token=%d\n",
					__FUNCTION__, idx, (char*)bcnRep, bcnRep->channel,
					MAC_TO_MACF(bcnRep->bssid), sizeof(wifi_BeaconReport_t),
					measRep->rep_Token));

				/* use token in the first report */
				if (idx == 0)
					first_token = measRep->rep_Token;

				idx ++;
				if (idx >= est_cnt)
					break;
			}
		}
		else {
			HAL_WIFI_DBG(("%s: not beacon report len=%d est_cnt=%d\n",
				__FUNCTION__, len, est_cnt));
			break;
		}
		len -= (measRep->rep_Len + 2 + sizeof(wifi_hal_cb_evt_t));
		ie_ptr += (measRep->rep_Len + 2 + sizeof(wifi_hal_cb_evt_t));
	}

	if (idx) {
		*bcn_rep = bcn_rep_array;
		*cnt = idx;
		*token = first_token;
		HAL_WIFI_DBG(("%s: %d beacon report found, array=%p token=0x%x\n",
			__FUNCTION__, idx, bcn_rep_array, first_token));
		return RETURN_OK;
	}
	else {
		HAL_WIFI_DBG(("%s: no beacon report found\n", __FUNCTION__));
		if (bcn_rep_array)
			free(bcn_rep_array);
		return RETURN_ERR;
	}
}

static int
wifi_hal_cb_rrm_evt_handler(int sock_fd)
{
	INT ret = 0;
	INT bytes = 0;
	UCHAR buf_ptr[MAX_EVENT_BUFFER_LEN], *ptr = buf_ptr;
	wifi_hal_cb_evt_t *dpkt;
	UINT32 event_id;
	wifi_BeaconReport_t *bcnRepArray = NULL;
	UCHAR out_DialogToken = 0x11;
	UINT out_array_size = 0;

	if ((bytes = recv(sock_fd, ptr, MAX_EVENT_BUFFER_LEN, MSG_NOSIGNAL)) > 0) {
		dpkt = (wifi_hal_cb_evt_t *)ptr;

		if (dpkt->version == WIFI_HAL_EVT_VERSION) {
			event_id = dpkt->type;
			switch (event_id) {
				case WIFI_HAL_CB_RRM_BCNREP:
					if (wifi_hal_parse_rm_beacon_report((char *)dpkt, bytes,
						&bcnRepArray, &out_array_size, &out_DialogToken)
						== RETURN_OK) {
						if (callback_fnc.beaconReport_cb != NULL) {
							/* note: caller should free the buffer bcnRepArray */
							HAL_WIFI_DBG(("%s: SUCC parse beacon report: array=%p cnt=%d token=0x%x\n",
								__FUNCTION__, bcnRepArray, out_array_size, out_DialogToken));
							callback_fnc.beaconReport_cb(dpkt->apIndex,
								bcnRepArray, &out_array_size, &out_DialogToken);
						}
					}
					else {
						if (bcnRepArray)
							free(bcnRepArray);
						printf("%s:%d: Unsupported Category for Event: %d\n",
							__FUNCTION__, __LINE__, event_id);
					}
					break;

				default:
					HAL_WIFI_DBG(("%s:%d: Unsupported Event: %d \n",
						__FUNCTION__, __LINE__, event_id));
					break;
			}
		}
	}
	else if (bytes < 0) {
		perror("recv failed ");
		ret = -errno;
	}
	else if (bytes == 0) {
		char buf[32] = {0};
		bytes = snprintf(buf, 32, "RU There");
		ret = send(sock_fd, buf, bytes, MSG_NOSIGNAL);
		if ((ret < 0) && (errno == EPIPE))
			ret = -EPIPE;
	}
	return ret;
}
/* 802.11K api */
#endif /* (WIFI_HAL_VERSION_GE_2_12) */

/* 802.11v support */
/* parse data in event to nbr array in btm response
   sample content pointed by data:
   after skip 2 bytes (0a 08): data will start from token,status,delay (e.g 10 06 00)
   then some neighbor report tag such as:
   34 0d a2 9a 7d a3 d4 fa 00 00 00 00 51 01 00
   34 0d 1c 9e cc 21 76 30 00 00 00 10 51 0b 00
   34 0d a2 9a 7d a3 d4 fb 00 00 00 10 80 2a 00
 */

static int
wifi_hal_cb_data_to_btm_resp(UCHAR *data, wifi_BTMResponse_t *btm_resp, int len)
{
	int cnt = 0, min, max;
	uint tag_len = 0;

	NULL_PTR_ASSERT(btm_resp);

	memset(btm_resp, 0, sizeof(wifi_BTMResponse_t));

	HAL_WIFI_DBG(("%s: Enter len=%d data=%p\n", __FUNCTION__, len, data));

	/* data starting from token, status ... */
	btm_resp->token = *data++;
	btm_resp->status = *data++;
	btm_resp->terminationDelay = *data++;
	len -= 3;

	HAL_WIFI_DBG(("%s: token=%d status=%d len=%d data=%p\n",
		__FUNCTION__, btm_resp->token, btm_resp->status, len, data));

	min = DOT11_NEIGHBOR_REP_IE_FIXED_LEN + 2; /* 13 + 2 */
	max = min * MAX_CANDIDATES;
	if (len < min || len > max) {
		HAL_WIFI_ERR(("%s: data_len=%d out of range (%d - %d)\n",
			__FUNCTION__, len, min, max));
		return RETURN_ERR;
	}

	while (len >= min)
	{
		if (*data == DOT11_MNG_NEIGHBOR_REP_ID) { /* 52 (0x34) */
			tag_len = *++data;
			data ++;
			if (cnt < MAX_CANDIDATES) {
				if (tag_len <= sizeof(wifi_NeighborReport_t)) {
					memcpy((UCHAR *)&(btm_resp->candidates[cnt]), data, tag_len);
				} else {
					HAL_WIFI_ERR(("%s: tag_len (%d) too long.\n",
						__FUNCTION__, tag_len));
					break;
				}

				/* consider the first nbr as the target */
				if (cnt == 0)
					memcpy(btm_resp->target, data, sizeof(bssid_t));

				HAL_WIFI_DBG(("%s: MAC="MACF", cnt=%d data=%p\n",
					__FUNCTION__, MAC_TO_MACF(btm_resp->candidates[cnt].bssid), cnt, data));

				cnt ++;
			} else {
				break;
			}
			data += tag_len;
			len -= (tag_len + 2);
			HAL_WIFI_DBG(("%s: len=%d cnt=%d data=%p\n",
				__FUNCTION__, len, cnt, data));
		}
		else {
			HAL_WIFI_DBG(("%s: tag_id %d is not nbr report\n",
				__FUNCTION__, *data));
			break;
		}
	}

	btm_resp->numCandidates = cnt;
	HAL_WIFI_DBG(("%s: target MAC="MACF", cnt=%d len=%d data=%p\n",
		__FUNCTION__, MAC_TO_MACF(btm_resp->target), cnt, len, data));
	return RETURN_OK;
}

static int
wifi_hal_cb_bsstrans_evt_handler (int sock_fd)
{
	INT ret = 0;
	INT bytes = 0;
	UCHAR buf_ptr[MAX_EVENT_BUFFER_LEN], *ptr = buf_ptr;
	wifi_hal_cb_evt_t *dpkt;
	UINT32 event_id;
	INT subtype;
	wifi_BTMQuery_t *btm_query;
	wifi_BTMRequest_t *btm_req=NULL;
#ifdef WIFI_HAL_VERSION_3
	mac_address_t mac_address;
#endif

	if ((bytes = recv(sock_fd, ptr, MAX_EVENT_BUFFER_LEN, MSG_NOSIGNAL)) > 0) {

		dpkt = (wifi_hal_cb_evt_t *)ptr;
		if (dpkt->version == WIFI_HAL_EVT_VERSION) {
			event_id = dpkt->type;
			subtype = dpkt->reason;

			ptr += sizeof(wifi_hal_cb_evt_t); /* point to cb event->data */
			HAL_WIFI_DBG(("%s: %d: MAC=%s, bytes=%d (%d) event_id=%d subtype=%d ap_idx=%d p0=0x%x p1=0x%x\n",
				__FUNCTION__, __LINE__, dpkt->mac, bytes, sizeof(wifi_hal_cb_evt_t),
				event_id, subtype, dpkt->apIndex, ptr[0], ptr[1]));

			/* ptr: category action token ... */
			ptr += 2; /* skip cat/action for new struct wifi_BTMResponse_t */
			bytes -= (sizeof(wifi_hal_cb_evt_t) + 2);

#ifdef WIFI_HAL_VERSION_3
			/* convert mac string to 6-byte hex */
			MACF_TO_MAC(dpkt->mac, mac_address);
#endif

			/* DUMP */
			switch (subtype) {
				case WIFI_HAL_CB_BSSTRANS_RESP:
					/* invoke callback function here */
					if (callback_fnc.btm_resp_cb) {
						wifi_BTMResponse_t *btm_resp;
						/* `ptr[1]` corresponds to `((wifi_BTMResponse_t *)ptr)->status` */
						if (ptr[1] == 6) {
							btm_resp = (wifi_BTMResponse_t *)malloc(sizeof(wifi_BTMResponse_t));
							if (btm_resp == NULL) {
								HAL_WIFI_ERR(("%s:%d: malloc(wifi_BTMResponse_t) failed %d",
									__FUNCTION__, __LINE__, errno));
								ret = -errno;
								break;
							}
							if (wifi_hal_cb_data_to_btm_resp(ptr, btm_resp, bytes) != RETURN_OK) {
								HAL_WIFI_ERR(("%s:%d: Fail to parse BSSTRANS Response\n",
									__FUNCTION__, __LINE__));
								free(btm_resp);
								ret = -EINVAL;
								break;
							}
						}
						else {
							btm_resp = (wifi_BTMResponse_t *)ptr;
							HAL_WIFI_DBG(("%s:%d: BSSTRANS Response Event from "MACF" status=%d\n",
								__FUNCTION__, __LINE__, MAC_TO_MACF(((char *)&(btm_resp->target))),
								(*(char *)&(btm_resp->status))));
						}
#ifdef WIFI_HAL_VERSION_3
						callback_fnc.btm_resp_cb(dpkt->apIndex, (char *)mac_address, btm_resp);
#else
						callback_fnc.btm_resp_cb(dpkt->apIndex, dpkt->mac, btm_resp);
#endif
						if ((void *)btm_resp != (void *)ptr) {
							/* it's a pointer to a true wifi_BTMResponse_t structure
							   that was malloc'ed in the special case above for `ptr[1] == 6`,
							   and it goes out of scope here - need to free it */
							free(btm_resp);
						}
					}
					break;
				case WIFI_HAL_CB_BSSTRANS_REQ:
					HAL_WIFI_DBG(("%s:%d: BSSTRANS Request Event: %d \n", __FUNCTION__, __LINE__, subtype));
					break;

				case WIFI_HAL_CB_BSSTRANS_QUERY:

					/* invoke callback function here */
					if (callback_fnc.btm_query_req_cb) {
						btm_query = (wifi_BTMQuery_t *)ptr; /* tmp, todo */
						// btm_req = (wifi_BTMRequest_t *)ptr;
#ifdef WIFI_HAL_VERSION_3
						callback_fnc.btm_query_req_cb(dpkt->apIndex, (char *)mac_address, btm_query, 0, btm_req); /* tmp, todo */
#else
						callback_fnc.btm_query_req_cb(dpkt->apIndex, dpkt->mac, btm_query, 0, btm_req); /* tmp, todo */
#endif
					}

					break;
				default:
					HAL_WIFI_DBG(("%s:%d: Unsupported BSSTRANS Event: %d (subtype=%d)\n",
						__FUNCTION__, __LINE__, event_id, subtype));
					break;
			}
		}
	}
	else if (bytes < 0) {
		perror("recv failed ");
		ret = -errno;
	}
	else if (bytes == 0) {
		char buf[32] = {0};
		bytes = snprintf(buf, 32, "RU There");
		ret = send(sock_fd, buf, bytes, MSG_NOSIGNAL);
		if ((ret < 0) && (errno == EPIPE))
			ret = -EPIPE;
	}
	return ret;
}

#if WIFI_HAL_VERSION_GE_2_16 || defined(WIFI_HAL_VERSION_3)
/* DPP event: used for wifi_mgmt_frame_callbacks actually */
static int
wifi_hal_cb_dpp_evt_handler(int sock_fd)
{
	INT ret = 0;
	INT bytes = 0;
	UCHAR buf_ptr[MAX_EVENT_BUFFER_LEN], *ptr = buf_ptr;
	wifi_hal_cb_evt_t *dpkt;
	UINT32 event_id;
	INT subtype, len;
	mac_address_t peer_mac;

	if ((bytes = recv(sock_fd, ptr, MAX_EVENT_BUFFER_LEN, MSG_NOSIGNAL)) > 0) {

		dpkt = (wifi_hal_cb_evt_t *)ptr;
		if (dpkt->version == WIFI_HAL_EVT_VERSION) {
			event_id = dpkt->type;
			subtype = dpkt->reason;

			ptr += sizeof(wifi_hal_cb_evt_t);
			HAL_WIFI_DBG(("%s: %d: MAC=%s, bytes=%d (%d) event_id=%d subtype=%d ap_idx=%d p0=0x%x p1=0x%x\n",
				__FUNCTION__, __LINE__, dpkt->mac, bytes, sizeof(wifi_hal_cb_evt_t),
				event_id, subtype, dpkt->apIndex, ptr[0], ptr[1]));

			/* convert to MAC 6 bytes hex */
			MACF_TO_MAC(dpkt->mac, peer_mac);

			len = bytes - sizeof(wifi_hal_cb_evt_t);
			HAL_WIFI_DBG(("%s: %d: MAC="MACF", bytes=%d len=%d\n",
				__FUNCTION__, __LINE__, MAC_TO_MACF(peer_mac), bytes, len));

			/* ptr points to 04 (cat) 09 (action) ...*/
			/* DUMP */
			switch (subtype) {
#if WIFI_HAL_VERSION_GE_2_19 || defined(WIFI_HAL_VERSION_3)
				case WIFI_HAL_CB_DPP_AUTH_RESP:
				case WIFI_HAL_CB_DPP_CONFIG_REQ:
				case WIFI_HAL_CB_ANQP_QAS_REQ:
					/* format
					int mgmt_frame_received_callback(INT ap_index, mac_address_t sta_mac,
						UCHAR *frame, UINT len, wifi_mgmtFrameType_t type, wifi_direction_t dir);
					*/
					if (callback_fnc.mgmt_frame_cb) {
						callback_fnc.mgmt_frame_cb(dpkt->apIndex, peer_mac,
							dpkt->data, len, dpkt->type, wifi_direction_uplink);
					}
					break;
#else
				case WIFI_HAL_CB_DPP_AUTH_RESP:
					/* invoke callback function here */
					if (callback_fnc.dpp_authresp_cb) {
						HAL_WIFI_DBG(("%s:%d: DPP Auth response: apindex=%d len=%d\n",
							__FUNCTION__, __LINE__, dpkt->apIndex, len));
						callback_fnc.dpp_authresp_cb(dpkt->apIndex, peer_mac, dpkt->data, len);
					}
					break;

				case WIFI_HAL_CB_DPP_CONFIG_REQ:
					/* invoke callback function here */
					if (callback_fnc.dpp_configreq_cb) {
						wifi_dpp_gas_act_frame_t *dpp_cfg;
						dpp_cfg = (wifi_dpp_gas_act_frame_t *)ptr;

						HAL_WIFI_DBG(("%s:%d: DPP Config request: action=%d token=%d req_len=%d\n",
							__FUNCTION__, __LINE__, dpp_cfg->action, dpp_cfg->token, dpp_cfg->req_len));

						callback_fnc.dpp_configreq_cb(dpkt->apIndex, peer_mac, dpp_cfg->token, dpp_cfg->gas_request, dpp_cfg->req_len);
					}

					break;
#endif /* WIFI_HAL_VERSION_GE_2_19 */
				default:
					HAL_WIFI_ERR(("%s:%d: Unsupported DPP Event: %d (subtype=%d)\n",
						__FUNCTION__, __LINE__, event_id, subtype));
					break;
			}
		}
	}
	else if (bytes < 0) {
		perror("recv failed ");
		ret = -errno;
	}
	else if (bytes == 0) {
		char buf[32] = {0};
		bytes = snprintf(buf, 32, "RU There");
		ret = send(sock_fd, buf, bytes, MSG_NOSIGNAL);
		if ((ret < 0) && (errno == EPIPE))
			ret = -EPIPE;
	}
	return ret;
}
/* DPP */
#endif /* (WIFI_HAL_VERSION_GE_2_16) */

#if WIFI_HAL_VERSION_GE_2_18 || defined(WIFI_HAL_VERSION_3)
/* Channel events: channel change, radar detection */
static int
wifi_hal_cb_channel_evt_handler(int sock_fd)
{
	int ret = 0, bytes = 0;
	unsigned char buf_ptr[MAX_EVENT_BUFFER_LEN], *ptr = buf_ptr;
	wifi_hal_cb_evt_t *dpkt;
	unsigned int event_id;
	wl_event_change_chan_t *ch_chg_evtp;
	wl_event_radar_detect_t *radar_evtp;
	static bool radar_detected = FALSE;
	static uint16_t exit_dfs_chanspec = 0;

	bytes = recv(sock_fd, ptr, MAX_EVENT_BUFFER_LEN, MSG_NOSIGNAL);
	if (bytes < 0) {
		perror("recv failed ");
		return -errno;
	} else if (bytes == 0) {
		char buf[32] = {0};
		bytes = snprintf(buf, 32, "RU There");
		ret = send(sock_fd, buf, bytes, MSG_NOSIGNAL);
		if ((ret < 0) && (errno == EPIPE))
			ret = -EPIPE;
		return ret;
	}

	dpkt = (wifi_hal_cb_evt_t *)ptr;

	if (dpkt->version == WIFI_HAL_EVT_VERSION) {
		event_id = dpkt->type;
		if (event_id == WIFI_HAL_CB_CH_CHG) {
			ch_chg_evtp = (wl_event_change_chan_t *) (dpkt->data);
			HAL_WIFI_DBG(("%s: Channel Change apIndex=%d reason=%d "
				"target_chanspec=0x%x \n", __FUNCTION__, dpkt->apIndex,
				ch_chg_evtp->reason, ch_chg_evtp->target_chanspec));
			if (radar_detected &&
				exit_dfs_chanspec == ch_chg_evtp->target_chanspec) {
				HAL_WIFI_DBG(("%s: Ignore exit DFS chanspec:0x%x \n",
					__FUNCTION__, exit_dfs_chanspec));
				radar_detected = FALSE;
				exit_dfs_chanspec = 0;
			} else if (callback_fnc.chan_change_cb != NULL) {
				UINT radioIndex = HAL_AP_IDX_TO_HAL_RADIO(dpkt->apIndex);
				UCHAR channel;

				channel = ((UCHAR)((ch_chg_evtp->target_chanspec) & WL_CHANSPEC_CHAN_MASK));
				RADIO_INDEX_ASSERT(radioIndex);
				callback_fnc.chan_change_cb(radioIndex,
					WIFI_EVENT_CHANNELS_CHANGED, channel);
			}
		} else if (event_id == WIFI_HAL_CB_RADAR) {
			radar_evtp = (wl_event_radar_detect_t *) (dpkt->data);
			HAL_WIFI_DBG(("%s: Radar Detect apIndex=%d curr_chspec=0x%x "
				"target_chspec=0x%x \n",__FUNCTION__, dpkt->apIndex,
				radar_evtp->current_chanspec,
				radar_evtp->target_chanspec));
			radar_detected = TRUE;
			exit_dfs_chanspec = radar_evtp->target_chanspec;
			if (callback_fnc.chan_change_cb != NULL) {
				UINT radioIndex = HAL_AP_IDX_TO_HAL_RADIO(dpkt->apIndex);
				UCHAR channel;

				channel = ((UCHAR)((radar_evtp->target_chanspec) & WL_CHANSPEC_CHAN_MASK));
				RADIO_INDEX_ASSERT(radioIndex);
				callback_fnc.chan_change_cb(radioIndex,
					WIFI_EVENT_DFS_RADAR_DETECTED, channel);
			}
		} else {
			HAL_WIFI_DBG(("%s:%d: Unsupported Event: %d \n", __FUNCTION__, __LINE__, event_id));
		}
	}
	return RETURN_OK;
}

#endif /* (WIFI_HAL_VERSION_GE_2_18) */

void wifi_apAssociatedDevice_callback_unregister(void)
{
	wldm_callback(WLDM_CB_UNREGISTER, FD_STA_CONN, NULL, &wifi_cb_info);
	callback_fnc.associate_cb = NULL;
}

//INT wifi_apAssociatedDevice_callback_register(INT radioIndex)
void wifi_apAssociatedDevice_callback_register(wifi_apAssociatedDevice_callback callback_proc)
{
	if (wldm_callback(WLDM_CB_REGISTER, FD_STA_CONN, wifi_hal_cb_sta_conn_evt_handler, &wifi_cb_info) < 0) {
		HAL_WIFI_ERR(("Failed to register the %s callback\n", __FUNCTION__));
		return;
	}
	callback_fnc.associate_cb = callback_proc;
	cbThreadId = wifi_cb_info.cbThreadId;
	return;
}

void wifi_apAuthEvent_callback_unregister()
{
	wldm_callback(WLDM_CB_UNREGISTER, FD_AUTH_FAIL, NULL, &wifi_cb_info);
	callback_fnc.auth_cb = NULL;
}

//INT wifi_apDeAuthEvent_callback_register(INT radioIndex)
void wifi_apDeAuthEvent_callback_register(wifi_apDeAuthEvent_callback callback_proc)
{
	if (wldm_callback(WLDM_CB_REGISTER, FD_AUTH_FAIL, wifi_hal_cb_auth_evt_handler, &wifi_cb_info) < 0) {
		HAL_WIFI_ERR(("Failed to register the %s callback\n", __FUNCTION__));
		return;
	}
	callback_fnc.auth_cb = callback_proc;
	cbThreadId = wifi_cb_info.cbThreadId;
	return;
}

//Dummy function to support 19.2ER2 RDKM
void wifi_apAuthEvent_callback_register(wifi_apAuthEvent_callback callback_proc)
{
	UNUSED_PARAMETER(callback_proc);
	HAL_WIFI_DBG(("Enter %s\n", __FUNCTION__));
	return;
}

INT wifi_steering_eventUnregister(void)
{
	wldm_callback(WLDM_CB_UNREGISTER, FD_MESH, NULL, &wifi_cb_info);
	callback_fnc.mesh_cb = NULL;
	return RETURN_OK;
}

pthread_t monitor_thread;

void *monitor_thread_main(void *arg)
{
	UNUSED_PARAMETER(arg);
	while (1) {
		sleep(1);
	}
	pthread_exit(0);
}

INT wifi_steering_eventRegister(wifi_steering_eventCB_t event_cb)
{
	if (wldm_callback(WLDM_CB_REGISTER, FD_MESH, wifi_hal_cb_mesh_evt_handler, &wifi_cb_info) < 0) {
		HAL_WIFI_ERR(("Failed to register the %s callback\n", __FUNCTION__));
		return RETURN_ERR;
	}
	callback_fnc.mesh_cb = event_cb;
	cbThreadId = wifi_cb_info.cbThreadId;

	return RETURN_OK;
}

#if WIFI_HAL_VERSION_GE_2_12 || defined(WIFI_HAL_VERSION_3)
/* 802.11K api */

INT wifi_RMBeaconRequestCallbackUnregister(UINT apIndex,
	wifi_RMBeaconReport_callback beaconReportCallback)
{
	UNUSED_PARAMETER(beaconReportCallback);
	// Note: apIndex is not used
	if ((apIndex >= HAL_WIFI_TOTAL_NO_OF_APS))
	{
		HAL_WIFI_DBG(("%s, INCORRECT apIndex [%d] \n", __FUNCTION__, apIndex));
		// return RETURN_ERR;
	}

	wldm_callback(WLDM_CB_UNREGISTER, FD_RRM_BCNREP, NULL, &wifi_cb_info);
	callback_fnc.beaconReport_cb = NULL;
	return RETURN_OK;
}

INT wifi_RMBeaconRequestCallbackRegister(UINT apIndex, wifi_RMBeaconReport_callback beaconReportCallback)
{
	// Note: apIndex is not used
	if ((apIndex >= HAL_WIFI_TOTAL_NO_OF_APS)) {
		HAL_WIFI_DBG(("%s, INCORRECT apIndex [%d] \n", __FUNCTION__, apIndex));
		// return RETURN_ERR;
	}

	if (wldm_callback(WLDM_CB_REGISTER, FD_RRM_BCNREP, wifi_hal_cb_rrm_evt_handler, &wifi_cb_info) < 0) {
		HAL_WIFI_ERR(("Failed to register the %s callback\n", __FUNCTION__));
		return RETURN_ERR;
	}
	callback_fnc.beaconReport_cb = beaconReportCallback;
	cbThreadId = wifi_cb_info.cbThreadId;

	HAL_WIFI_DBG(("Done %s beaconReportCallback=%p \n", __FUNCTION__, beaconReportCallback));

	return RETURN_OK;
}

static int
parse_macstr(char *macstr, unsigned char *macp)
{
	unsigned char *mac = macp;
	return (sscanf(macstr, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx",
		&mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]));
}

#define WL_BEACON_REQUEST_SUBELEM_SSID			0
#define WL_BEACON_REQUEST_SUBELEM_INFO			1	/* Beacon Reporting */
#define WL_BEACON_REQUEST_SUBELEM_DETAIL		2	/* Reporting Detail */
#define WL_BEACON_REQUEST_SUBELEM_REQUEST		10
#define WL_BEACON_REQUEST_SUBELEM_EXT_REQUEST		11
#define WL_BEACON_REQUEST_SUBELEM_AP_CHANNEL		51	/* AP Channel Report */
#define WL_BEACON_REQUEST_SUBELEM_WIDEBANDWIDTHCHANNEL	163
#define WL_BEACON_REQUEST_SUBELEM_LAST_INDICATION	164
#define WL_BEACON_REQUEST_SUBELEM_VENDOR		221

#define ADD_SUBELEMENT_TO_BCN_REQ(subElemId, subElemLen, inPtr) \
	request_serialized[request_len++] = subElemId; \
	request_serialized[request_len++] = subElemLen; \
	memcpy(&(request_serialized[request_len]), inPtr, subElemLen); \
	request_len += subElemLen;

INT wifi_setRMBeaconRequest(UINT apIndex,
#ifdef WIFI_HAL_VERSION_3_PHASE2
	mac_address_t peer,
#else
	CHAR *peer_mstr, /* XX:XX:XX:XX:XX:XX */
#endif /* WIFI_HAL_VERSION_3_PHASE2 */
	wifi_BeaconRequest_t *in_request, UCHAR *out_DialogToken)
{
	int reqlen, outlen;
	wl_af_rrm_req_info_t wl_af_rrm_req;
	wl_af_rrm_req_info_t *reqp;
	char logStr[128];
	int ret, request_len = 0;
	unsigned char request_serialized[128];

#ifdef WIFI_HAL_VERSION_3_PHASE2
	NULL_PTR_ASSERT(peer);
#else
	unsigned char peer[6];
	NULL_PTR_ASSERT(peer_mstr);
	parse_macstr(peer_mstr, &(peer[0]));
#endif /* WIFI_HAL_VERSION_3_PHASE2 */

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(in_request);
	NULL_PTR_ASSERT(out_DialogToken);

	// HAL_DBG_PRINT_BUF(__FUNCTION__, (char *)(in_request), sizeof(wifi_BeaconRequest_t));
	HAL_WIFI_DBG(("%s: apIndex = %d MAC = "MACF" numRep = %d\n",
		__FUNCTION__, apIndex, MAC_TO_MACF(peer), in_request->numRepetitions));

	/* Beacon Request:
	 * Oper class | Channel | Rand Interval | Meas Dur | Meas Mode |  BSSID  | Subelements
	 *    1 byte  |  1 byte |   2 bytes     | 2 bytes  |  1 byte   | 6 bytes |  variable
	 */
	request_serialized[request_len++] = in_request->opClass;
	request_serialized[request_len++] = in_request->channel;
	request_serialized[request_len++] = (in_request->randomizationInterval >> 0) & 0xff;
	request_serialized[request_len++] = (in_request->randomizationInterval >> 8) & 0xff;
	request_serialized[request_len++] = (in_request->duration >> 0) & 0xff;
	request_serialized[request_len++] = (in_request->duration >> 8) & 0xff;
	request_serialized[request_len++] = in_request->mode;
	memcpy(&request_serialized[request_len], in_request->bssid, sizeof(bssid_t));
	request_len += sizeof(bssid_t);

	/* Optional subelements - keep in order of subelement ID; Ref: Table 9-88 802.11-2016 */
	/* SubElemID   Name	              Extensible
	 *   0         SSID                         No
	 *   1         beaconReporting              Yes
	 *   2         reportingDetail              Yes
	 *  10         requestedElementIDS          No
	 *  11         extdRequestedElementIDS      No
	 *  51         channelReport                No
	 * 163         wideBandwidthChannel         Yes
	 * 221         vendorSpecific               No
	 */

	/* Optional subelement SSID */
	if (in_request->ssidPresent) {
		size_t ssid_len = strnlen(in_request->ssid, sizeof(in_request->ssid));
		ADD_SUBELEMENT_TO_BCN_REQ(WL_BEACON_REQUEST_SUBELEM_SSID, ssid_len,
			in_request->ssid);
	}

	/* Optional subelement beaconReporting - TBD */
	if (in_request->beaconReportingPresent) {
		if (in_request->beaconReporting.condition > 10) {
			HAL_WIFI_ERR(("%s: Error beaconReporting.condition = %d\n",
				__FUNCTION__, in_request->beaconReporting.condition));
			return RETURN_ERR;
		}
		ADD_SUBELEMENT_TO_BCN_REQ(WL_BEACON_REQUEST_SUBELEM_INFO,
			sizeof(in_request->beaconReporting), &(in_request->beaconReporting));
	}
	/* Optional subelement reportingDetail */
	if (in_request->reportingRetailPresent) {
		if (in_request->reportingDetail > 2) {
			HAL_WIFI_ERR(("%s: Error reportingDetail = %d\n", __FUNCTION__,
				in_request->reportingDetail));
			return RETURN_ERR;
		}
		ADD_SUBELEMENT_TO_BCN_REQ(WL_BEACON_REQUEST_SUBELEM_DETAIL,
			sizeof(in_request->reportingDetail), &(in_request->reportingDetail));
	}
	/* Optional subelement requestedElementIDS - TBD */
	if (in_request->requestedElementIDSPresent) {
		ADD_SUBELEMENT_TO_BCN_REQ(WL_BEACON_REQUEST_SUBELEM_REQUEST,
			sizeof(in_request->requestedElementIDS),
			&(in_request->requestedElementIDS));
	}
	/* Optional subelement extdRequestedElementIDS - TBD */
	if (in_request->extdRequestedElementIDSPresent) {
		ADD_SUBELEMENT_TO_BCN_REQ(WL_BEACON_REQUEST_SUBELEM_EXT_REQUEST,
			sizeof(in_request->extdRequestedElementIDS),
			&(in_request->extdRequestedElementIDS));
	}
	/* Optional subelement channelReport - TBD */
	if (in_request->channelReportPresent) {
		ADD_SUBELEMENT_TO_BCN_REQ(WL_BEACON_REQUEST_SUBELEM_AP_CHANNEL,
			sizeof(in_request->channelReport), &(in_request->channelReport));
	}
	/* Optional subelement wideBandwidthChannel - TBD */
	if (in_request->wideBandWidthChannelPresent) {
		ADD_SUBELEMENT_TO_BCN_REQ(WL_BEACON_REQUEST_SUBELEM_WIDEBANDWIDTHCHANNEL,
			sizeof(in_request->wideBandwidthChannel),
			&(in_request->wideBandwidthChannel));
	}
	/* Optional subelement vendorSpecific - TBD */

	reqp = &wl_af_rrm_req;
	reqlen = (sizeof(wl_af_rrm_req_info_t) + MAC_STR_LEN);
	reqp->rm_actionId = DOT11_RM_ACTION_RM_REQ;
	reqp->rm_ieType = DOT11_MEASURE_TYPE_BEACON;
	reqp->in_request = request_serialized;
	reqp->in_reqLen = request_len;
	reqp->numRepetitions = in_request->numRepetitions;
	outlen = sizeof(*out_DialogToken);
	ret = wldm_11k_rrm_cmd(WLDM_RRM_SEND_REQUEST, apIndex, peer, (unsigned char *)reqp,
		&reqlen, out_DialogToken, &outlen);

	if (!ret) {
		snprintf(logStr, sizeof(logStr),
			"<802.11K Tx> Ap:%d Beacon Request of length %d to "MACF" Token=0x%x \n",
			apIndex,  request_len, MAC_TO_MACF(peer), *out_DialogToken);
		wifi_log_rrm_event(logStr);
	}
	return ret;
}

INT wifi_cancelRMBeaconRequest(UINT apIndex, UCHAR dialogToken)
{
	printf("-HAL In %s apIndex=%d dialogToken=0x%x \n", __FUNCTION__, apIndex, dialogToken);
	/* No action */
	return RETURN_OK;
}

/* If numNeighborReports = 0 delete all in current nbr_list
 * else add to current list
 */
INT wifi_setNeighborReports(UINT apIndex, UINT numNeighborReports,
	wifi_NeighborReport_t *neighborReports)
{
	unsigned char *buf = NULL, rep_len;
	int buflen, numRep;
	unsigned int i;
	wifi_NeighborReport_t *in_rep;
	dot11_neighbor_rep_ie_t *nbr_rep;
	BOOL nr_active = FALSE;

	HAL_WIFI_DBG(("%s: apIndex=%d numNeighborReports=%d\n", __FUNCTION__, apIndex, numNeighborReports));

	AP_INDEX_ASSERT(apIndex);

	if (wifi_getNeighborReportActivation(apIndex, &nr_active) < 0) {
		HAL_WIFI_ERR(("%s: Error getNeighborReportActivation Index=[%d]\n",
			__FUNCTION__, apIndex));
		return RETURN_ERR;
	}
	if (nr_active == FALSE) {
		HAL_WIFI_ERR(("%s: NeighborReportActivation is FALSE Index=[%d]\n",
			__FUNCTION__, apIndex));
		return RETURN_ERR;
	}
	if (numNeighborReports > 0) {
		/* TBD - optional element support - will need to update buflen with
		 * TLV_HDR_LEN + sizeof(optElement) for each optElement
		 */
		buflen = numNeighborReports * (TLV_HDR_LEN + sizeof(dot11_neighbor_rep_ie_t));
		buf = malloc(buflen);
		if (buf == NULL) {
			HAL_WIFI_ERR(("%s: Error Allocatiing memory\n", __FUNCTION__));
			return RETURN_ERR;
		}
		/* Translate to dot11_neighbor_rep_ie_t */
		nbr_rep = (dot11_neighbor_rep_ie_t *) buf;
		memset((unsigned char *)nbr_rep, 0, buflen);
		for (i = 0; i < numNeighborReports; i++) {
			in_rep = (wifi_NeighborReport_t *) ((UCHAR *)neighborReports +
				(i * sizeof(wifi_NeighborReport_t)));
			rep_len = DOT11_NEIGHBOR_REP_IE_FIXED_LEN;
			/* can be different when optional elements come in */

			nbr_rep->id = DOT11_MNG_NEIGHBOR_REP_ID;
			nbr_rep->len = rep_len;
			memcpy((unsigned char *)(&(nbr_rep->bssid)),
				(unsigned char *)(&(in_rep->bssid)), ETHER_ADDR_LEN);
			nbr_rep->bssid_info = in_rep->info;
			nbr_rep->reg = in_rep->opClass;
			nbr_rep->channel = in_rep->channel;
			nbr_rep->phytype = in_rep->phyTable;
			/* no optional elements */
			/* to next report */
			nbr_rep = (dot11_neighbor_rep_ie_t *)((UCHAR *)(nbr_rep) +
				(rep_len + TLV_HDR_LEN));
		}
	}
	numRep = numNeighborReports;
	HAL_WIFI_DBG(("%s: Do setNeighborReports Index=[%d] numRep=%d\n",
		__FUNCTION__, apIndex, numRep));
	if (wldm_11k_rrm_cmd(WLDM_RRM_SET_NEIGHBOR_REPORTS, apIndex, NULL, buf, &numRep,
		NULL, NULL) < 0) {
		HAL_WIFI_ERR(("%s: Error setting NeighborReportsradio Index=[%d]\n",
			__FUNCTION__, apIndex));
		if (buf != NULL) {
			free(buf);
		}
		return RETURN_ERR;
	}
	if (buf != NULL) {
		free(buf);
	}
	return RETURN_OK;
}

INT wifi_setNeighborReportActivation(UINT apIndex, BOOL activate)
{
	HAL_WIFI_DBG(("%s\n", __FUNCTION__));
	AP_INDEX_ASSERT(apIndex);

	HAL_WIFI_DBG(("%s: apIndex=%d\n", __FUNCTION__, apIndex));

	int len;
	len = DOT11_RRM_CAP_LEN;
	unsigned char rrm_val[DOT11_RRM_CAP_LEN] = {0};

	if (wldm_xbrcm_AccessPoint_RMCapabilities(CMD_GET, apIndex, rrm_val,
		&len, NULL, NULL) < 0) {
		HAL_WIFI_ERR(("%s: Error GET rmcap apIndex[%d]\n", __FUNCTION__, apIndex));
		return -1;
	}

	RRM_CAP_NEIGHBOR_REPORT_SET(activate, rrm_val[0]);
	HAL_WIFI_DBG(("%s: activate=%d rrm_val "RMCAPF" apIndex [%d]\n",  __FUNCTION__,
		activate, RMCAP_TO_RMCAPF(rrm_val), apIndex));

	len = DOT11_RRM_CAP_LEN;
	if (wldm_xbrcm_AccessPoint_RMCapabilities(CMD_SET, apIndex, rrm_val, &len,
		NULL, NULL) < 0) {
		HAL_WIFI_ERR(("%s: Error setting NeighborReportActivation apIndex=[%d]\n",
			__FUNCTION__, apIndex));
		return RETURN_ERR;
	}

	if (wldm_apply_AccessPointObject(apIndex) != 0) {
		HAL_WIFI_ERR(("%s: wldm_apply_AccessPointObject Failed for apIndex[%d]\n",
			__FUNCTION__, apIndex));
		return RETURN_ERR;
	}
	return RETURN_OK;
}

INT wifi_getNeighborReportActivation(UINT apIndex, BOOL *activatep)
{
	HAL_WIFI_DBG(("%s\n", __FUNCTION__));
	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(activatep);

	HAL_WIFI_DBG(("%s: apIndex=%d\n", __FUNCTION__, apIndex));

	int len;
	len = DOT11_RRM_CAP_LEN;
	unsigned char rrm_val[DOT11_RRM_CAP_LEN] = {0};

	if (wldm_xbrcm_AccessPoint_RMCapabilities(CMD_GET, apIndex, rrm_val, &len,
		NULL, NULL) < 0) {
		HAL_WIFI_ERR(("%s: Error getting rmcap apIndex[%d]\n",
			__FUNCTION__, apIndex));
		return RETURN_ERR;
	}
	HAL_WIFI_DBG(("%s: Got rrm_val "RMCAPF" apIndex [%d]\n",  __FUNCTION__,
		RMCAP_TO_RMCAPF(rrm_val), apIndex));

	*activatep = RRM_CAP_NEIGHBOR_REPORT_GET(rrm_val[0]);

	return RETURN_OK;
}

#ifdef WIFI_HAL_VERSION_3_PHASE2
INT wifi_getRMCapabilities(mac_address_t peer, UCHAR out_Capabilities[5])
{
	int ret;
	unsigned int len = DOT11_RRM_CAP_LEN;

	ret = wldm_11k_rrm_cmd(WLDM_RRM_GET_CLIENT_RRM_CAP, 0, /* dummy index */
		 (unsigned char *)peer, NULL, NULL, &(out_Capabilities[0]), &len);
	if (ret < 0) {
		HAL_WIFI_ERR(("%s: Error getting RM Capabilities for "MACF"\n",
			__FUNCTION__, MAC_TO_MACF(peer)));
		return WIFI_HAL_ERROR;
	}
	return WIFI_HAL_SUCCESS;
}
#else /* WIFI_HAL_VERSION_3_PHASE2 */
/* peer contains a 6 byte mac address of associated client */
INT wifi_getRMCapabilities(CHAR *peer, UCHAR out_Capabilities[DOT11_RRM_CAP_LEN])
{
	int ret, len = DOT11_RRM_CAP_LEN;

	NULL_PTR_ASSERT(peer);
	ret = wldm_11k_rrm_cmd(WLDM_RRM_GET_CLIENT_RRM_CAP, 0, /* dummy index */
		 (unsigned char *)peer, NULL, NULL, &(out_Capabilities[0]), &len);
	if (ret != RETURN_OK) {
		HAL_WIFI_ERR(("%s: Error getting RM Capabilities for "MACF"\n",
			__FUNCTION__, MAC_TO_MACF(peer)));
		return RETURN_ERR;
	}
	return RETURN_OK;
}
#endif /* WIFI_HAL_VERSION_3_PHASE2 */

/* 802.11K api */
#endif /* WIFI_HAL_VERSION_GE_2_12 */
/* 802.11v support */
INT wifi_BTMQueryRequest_callback_register(
	UINT apIndex,
	wifi_BTMQueryRequest_callback btmRequestCallback,
	wifi_BTMResponse_callback btmResponseCallback)
{
	UNUSED_PARAMETER(apIndex);
	if (wldm_callback(WLDM_CB_REGISTER, FD_BSSTRANS, wifi_hal_cb_bsstrans_evt_handler, &wifi_cb_info) < 0) {
		HAL_WIFI_ERR(("Failed to register the %s callback\n", __FUNCTION__));
		return RETURN_ERR;
	}

	callback_fnc.btm_query_req_cb = btmRequestCallback;
	callback_fnc.btm_resp_cb = btmResponseCallback;
	cbThreadId = wifi_cb_info.cbThreadId;
	HAL_WIFI_DBG(("Done %s btmRequestCallback=%p btmResponseCallback=%p\n",
		__FUNCTION__, btmRequestCallback, btmResponseCallback));

	return RETURN_OK;
}

/* convert wifi_NeighborReport_t to btm request option frame
 * return: length of frame
 */
int wifi_build_btmreq_option(char *buffer, wifi_NeighborReport_t *p_report)
{
	int len = 0, elen;
	char *ptr;

	if ((ptr = buffer) == NULL) {
		printf("%s: buffer is nULL\n", __FUNCTION__);
		return len;
	}

	/* mandatory (first 13 bytes) */
	/* copy/assign every component one by one to avoid "holes" because
	   wifi_NeighborReport_t is not a packed data struct.
	   for reference, dot11_neighbor_rep_ie_t is a packed struct.
	*/
	elen = 6; /* hardcode for bssid */
	memcpy(ptr, p_report->bssid, elen);
	len += elen;
	ptr += elen;
	elen = 4; /* hardcode for info */
	memcpy(ptr, &p_report->info, elen);
	len += elen;
	ptr += elen;
	*ptr = p_report->opClass;
	len++;
	ptr++;
	*ptr = p_report->channel;
	len++;
	ptr++;
	*ptr = p_report->phyTable;
	len++;
	ptr++;

	/* option */
	if (p_report->tsfPresent) {
		/* total tlv 12 bytes */
		elen = sizeof(wifi_TSFInfo_t); /* 10 bytes */
		*ptr++ = 1; /* tsf subelement ID */
		*ptr++ = elen; /* element len */
		memcpy(ptr, (char*)&p_report->tsfInfo, elen);
		len += (2 + elen);
		ptr += elen;
	}

	if (p_report->bssTransitionCandidatePreferencePresent)
	{
		*ptr++ = 3; /* BSS Transition Candidate Preference subelement ID */
		*ptr++ = 1; /* element len */
		*ptr++ = p_report->bssTransitionCandidatePreference.preference;
		len += 3;
	}

	/* to do for all other options */

	return len;
}

#ifdef WIFI_HAL_VERSION_3_PHASE2
INT wifi_setBTMRequest(UINT apIndex,
	mac_address_t peerMac,
	wifi_BTMRequest_t *in_struct)
{
	int ret, i, opt_total_len = 0, opt_sub_len;
	unsigned char macAddress[6];
	int radioIndex = HAL_AP_IDX_TO_HAL_RADIO(apIndex);
	UCHAR buf[MAX_EVENT_BUFFER_LEN], *buf_ptr = buf;
	BOOL activate;

	AP_INDEX_ASSERT((int)apIndex);
	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(in_struct);

	memcpy(macAddress, peerMac, sizeof(mac_address_t));
	HAL_WIFI_DBG(("%s: MAC="MACF", apIndex=%d\n",
		__FUNCTION__, MAC_TO_MACF(macAddress), apIndex));

	/* don't send BTM request if BTM is not activated on the radio */
	if (wldm_11v_btm(WLDM_BTM_GET_ACTIVATION, apIndex, NULL,
		&activate, NULL, 0, NULL) == 0) {
		if (activate == 0) {
			HAL_WIFI_ERR(("%s: BTM is not activated for apIndex=%d\n", __FUNCTION__, apIndex));
			return WIFI_HAL_ERROR;
		}
	}
	else {
		HAL_WIFI_ERR(("%s: fail to get BTM activation for apIndex=%d \n", __FUNCTION__, apIndex));
		return WIFI_HAL_ERROR;
	}

	HAL_WIFI_DBG(("%s: apIndex=%d radioIndex=%d num_candidate=%d\n",
		 __FUNCTION__, apIndex, radioIndex, in_struct->numCandidates));

	/* convert multiple candidates to BTM request action frame */
	for (i = 0; i < in_struct->numCandidates; i++) {
		HAL_WIFI_DBG(("%s: (start) i=%d buf_ptr=%p opt_total_len=%d\n",
			 __FUNCTION__, i, buf_ptr, opt_total_len));
		*(buf_ptr++) = 0x34; /* ID for candidate list */
		opt_sub_len = wifi_build_btmreq_option(buf_ptr + 1, &in_struct->candidates[i]);
		*(buf_ptr++) = opt_sub_len;
		buf_ptr += opt_sub_len;
		opt_total_len += (2 + opt_sub_len);
		HAL_WIFI_DBG(("%s: (end) buf_ptr=%p opt_total_len=%d\n",
			 __FUNCTION__, buf_ptr, opt_total_len));
	}

	ret = wldm_11v_btm(WLDM_BTM_SEND_REQUEST, apIndex, macAddress, NULL, (char *)in_struct, opt_total_len, buf);
	HAL_WIFI_DBG(("%s: completed ret=%d\n", __FUNCTION__, ret));

	return ret;
}
#else /* WIFI_HAL_VERSION_3_PHASE2 */
INT wifi_setBTMRequest(UINT apIndex,
	CHAR *peerMACAddress,
	wifi_BTMRequest_t *in_struct)
{
	int ret, i, opt_total_len = 0, opt_sub_len;
	unsigned char macAddress[6];
	int radioIndex = HAL_AP_IDX_TO_HAL_RADIO(apIndex);
	CHAR buf[MAX_EVENT_BUFFER_LEN], *buf_ptr = buf;
	BOOL activate;

	HAL_WIFI_DBG(("%s: apindex=%d MAC=%s\n", __FUNCTION__, apIndex, peerMACAddress));

	AP_INDEX_ASSERT((int)apIndex);
	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(in_struct);

	MACF_TO_MAC(peerMACAddress, macAddress);

	/* don't send BTM request if BTM is not activated on the radio */
	if (wldm_11v_btm(WLDM_BTM_GET_ACTIVATION, apIndex, NULL,
		&activate, NULL, 0, NULL) == 0) {
		if (activate == 0) {
			HAL_WIFI_ERR(("%s: BTM is not activated for apIndex=%d\n", __FUNCTION__, apIndex));
			return RETURN_ERR;
		}
	}
	else {
		HAL_WIFI_ERR(("%s: fail to get BTM activation for apIndex=%d \n", __FUNCTION__, apIndex));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: apIndex=%d radioIndex=%d num_candidate=%d\n",
		 __FUNCTION__, apIndex, radioIndex, in_struct->numCandidates));

	/* convert multiple candidates to BTM request action frame */
	for (i = 0; i < in_struct->numCandidates; i++) {
		HAL_WIFI_DBG(("%s: (start) i=%d buf_ptr=%p opt_total_len=%d\n",
			 __FUNCTION__, i, buf_ptr, opt_total_len));
		*(buf_ptr++) = 0x34; /* ID for candidate list */
		opt_sub_len = wifi_build_btmreq_option(buf_ptr + 1, &in_struct->candidates[i]);
		*(buf_ptr++) = opt_sub_len;
		buf_ptr += opt_sub_len;
		opt_total_len += (2 + opt_sub_len);
		HAL_WIFI_DBG(("%s: (end) buf_ptr=%p opt_total_len=%d\n",
			 __FUNCTION__, buf_ptr, opt_total_len));
	}

	ret = wldm_11v_btm(WLDM_BTM_SEND_REQUEST, apIndex, macAddress, NULL, (char *)in_struct, opt_total_len, buf);

	HAL_WIFI_DBG(("%s: completed ret=%d\n", __FUNCTION__, ret));

	return ret;
}
#endif /* WIFI_HAL_VERSION_3_PHASE2 */

INT wifi_getBSSTransitionImplemented(UINT apIndex, BOOL *activate)
{
	AP_INDEX_ASSERT((int)apIndex);

	return wldm_11v_btm(WLDM_BTM_GET_IMPLEMENTED, apIndex, NULL,
		activate, NULL, 0, NULL);
}

INT wifi_setBSSTransitionActivation(UINT apIndex, BOOL activate)
{
	int ret;
	unsigned int radioIndex;

	HAL_WIFI_DBG(("%s\n", __FUNCTION__));
	AP_INDEX_ASSERT((int)apIndex);
	radioIndex = HAL_AP_IDX_TO_HAL_RADIO(apIndex);

	HAL_WIFI_DBG(("HAL %s apIndex=%d radioIndex=%d \n",
		__FUNCTION__, apIndex, radioIndex));

	ret = wldm_11v_btm(WLDM_BTM_SET_ACTIVATION, apIndex, NULL,
		&activate, NULL, 0, NULL);
	HAL_WIFI_DBG(("%s: completed ret=%d\n", __FUNCTION__, ret));

	return ret;
}

INT wifi_getBSSTransitionActivation(UINT apIndex, BOOL *activate)
{
	int ret;
	unsigned int radioIndex;

	HAL_WIFI_DBG(("%s\n", __FUNCTION__));
	AP_INDEX_ASSERT((int)apIndex);
	radioIndex = HAL_AP_IDX_TO_HAL_RADIO(apIndex);

	HAL_WIFI_DBG(("HAL %s apIndex=%d radioIndex=%d \n",
		__FUNCTION__, apIndex, radioIndex));

	ret = wldm_11v_btm(WLDM_BTM_GET_ACTIVATION, apIndex, NULL,
		activate, NULL, 0, NULL);
	HAL_WIFI_DBG(("%s: completed ret=%d\n", __FUNCTION__, ret));
	return(ret);
}

/*
typedef struct _wifi_BTMCapabilities_t
{
	// Number of entries in each of the following arrays.
	UINT cap_NumberOfEntries;
	// Array a peer device MAC addresses.
	UCHAR *cap_PeerMACAddress[6];
	// Array of bool indicating peer BSS transition capability.
	BOOL *cap_BSSTransitionCapability;
} wifi_BTMCapabilities_t;

change to
#define MAX_BTM_DEVICES		64
typedef struct {
	UINT			entries;						// Number of entries in each of the following arrays.
	mac_address_t	peer[MAX_BTM_DEVICES];			// Array a peer device MAC addresses.
	BOOL			capability[MAX_BTM_DEVICES];	// Array of bool indicating peer BSS transition capability.
} wifi_BTMCapabilities_t;
*/

INT wifi_getBTMClientCapabilityList(UINT apIndex,
	wifi_BTMCapabilities_t *extBTMCapabilities)
{
	int ret, i, count, radio_index, subindex;
	mac_address_t *mac;
	BOOL *bsst_cap;
	UCHAR *peer_mac;

	AP_INDEX_ASSERT((int)apIndex);
	NULL_PTR_ASSERT(extBTMCapabilities);

	radio_index = HAL_AP_IDX_TO_HAL_RADIO(apIndex);
	subindex = HAL_AP_IDX_TO_SSID_IDX(apIndex);
	count = extBTMCapabilities->entries;

	HAL_WIFI_DBG(("HAL %s apIndex=%d radio_index=%d subindex=%d count=%d\n",
		 __FUNCTION__, apIndex, radio_index, subindex, count));

	mac = extBTMCapabilities->peer;
	bsst_cap = extBTMCapabilities->capability;

	for (i = 0; i < count && i < MAX_BTM_DEVICES; i++) {
		ret = wldm_11v_btm(WLDM_BTM_GET_CLIENT_CAP, apIndex, (unsigned char *)mac,
			bsst_cap, NULL, 0, NULL);

		peer_mac = (UCHAR *)mac;

		if (ret != 0) {
			HAL_WIFI_ERR(("%s: fail for MAC="MACF" on apIndex=%d\n",
				__FUNCTION__, MAC_TO_MACF(peer_mac), apIndex));
			return RETURN_ERR;
		}

		HAL_WIFI_DBG(("HAL %s i=%d mac=%p MAC="MACF" bsst_cap=%p value=%d ret=%d\n",
			__FUNCTION__, i, mac, MAC_TO_MACF(peer_mac), bsst_cap, *bsst_cap, ret));

		mac++;
		bsst_cap++;
	}

	return RETURN_OK;
}

/* end of 802.11v API */

#if WIFI_HAL_VERSION_GE_2_16 || defined(WIFI_HAL_VERSION_3)
/* API for DPP */
#if WIFI_HAL_VERSION_GE_2_19 || defined(WIFI_HAL_VERSION_3)
/* wifi_hal.h function rename RDKB-31351 */
INT wifi_dpp_frame_received_callbacks_register(wifi_dppAuthResponse_callback_t dppAuthCallback,
		wifi_dppConfigRequest_callback_t dppConfigCallback,
		wifi_dppConfigResult_callback_t dppConfigResultCallback,
		wifi_dppReconfigAnnounce_callback_t dppReconfigAnnounceCallback,
		wifi_dppReconfigAuthResponse_callback_t dppReconfigAuthRspCallback)
{
	UNUSED_PARAMETER(dppAuthCallback);
	UNUSED_PARAMETER(dppConfigCallback);
	UNUSED_PARAMETER(dppConfigResultCallback);
	UNUSED_PARAMETER(dppReconfigAnnounceCallback);
	UNUSED_PARAMETER(dppReconfigAuthRspCallback);
	/* obsolete, use dummy code to avoid error in case some app still call it */
	return RETURN_OK;
}

INT wifi_mgmt_frame_callbacks_register(wifi_receivedMgmtFrame_callback dppRecvRxCallback)
{
	if (wldm_callback(WLDM_CB_REGISTER, FD_DPP, wifi_hal_cb_dpp_evt_handler, &wifi_cb_info) < 0) {
		HAL_WIFI_ERR(("Failed to register the %s callback\n", __FUNCTION__));
		return RETURN_ERR;
	}

	callback_fnc.mgmt_frame_cb = dppRecvRxCallback;

	cbThreadId = wifi_cb_info.cbThreadId;

	/* inform hspotap not handle anqp request */
	nvram_set(NVNM_HS2_ANQP_HAL, "1");

	HAL_WIFI_DBG(("Done %s dppRecvRxCallback=%p\n",
		__FUNCTION__, dppRecvRxCallback));
	return RETURN_OK;
}
#else /* old solution */
INT wifi_dpp_frame_received_callbacks_register(wifi_dppAuthResponse_callback_t dppAuthCallback,
	wifi_dppConfigRequest_callback_t dppConfigCallback)
{
	if (wldm_callback(WLDM_CB_REGISTER, FD_DPP, wifi_hal_cb_dpp_evt_handler, &wifi_cb_info) < 0) {
		HAL_WIFI_ERR(("Failed to register the %s callback\n", __FUNCTION__));
		return RETURN_ERR;
	}

	callback_fnc.dpp_authresp_cb = dppAuthCallback;
	callback_fnc.dpp_configreq_cb = dppConfigCallback;
	cbThreadId = wifi_cb_info.cbThreadId;

	HAL_WIFI_DBG(("Done %s dppAuthCallback=%p dppConfigCallback=%p\n",
		__FUNCTION__, dppAuthCallback, dppConfigCallback));

	return RETURN_OK;
}
#endif /* WIFI_HAL_MINOR_VERSION >= 19 */

/* freq in MHz */
static int freq2channel(UINT freq)
{
	int ch = -1;

	if (freq <= 2484) {
		/* 2.4G */
		ch = (freq == 2484) ? 14 : (freq - 2407) / 5;
	} else if ((freq >= 5160) && (freq <= 5825)) {
		/* 5G */
		ch = (freq - 5000) / 5;
	}

	return ch;
}

INT wifi_sendActionFrame(INT apIndex,
	mac_address_t sta,
	UINT frequency,
	UCHAR *frame,
	UINT len)
{
	int channel, ret;
	unsigned int radioIndex;

	HAL_WIFI_DBG(("%s\n", __FUNCTION__));
	AP_INDEX_ASSERT(apIndex);
	radioIndex = HAL_AP_IDX_TO_HAL_RADIO(apIndex);
	NULL_PTR_ASSERT(frame);

	if ((channel = freq2channel(frequency)) < 0) {
		HAL_WIFI_ERR(("%s, Bad freq %d, use current ch\n", __FUNCTION__, frequency));
		channel = 0;
	}

	ret = wl_sendActionFrame(apIndex, sta, NULL, channel, (char *)frame, len);

	HAL_WIFI_DBG(("HAL %s apIndex=%d radioIndex=%d frequency=%d ch=%d len=%d ret=%d\n",
		 __FUNCTION__, apIndex, radioIndex, frequency, channel, len, ret));

	return ret;
}

/* New requirment: the function will be called from ecbd.c directly when receiving
typedef enum
{
	WIFI_FRAME_TYPE_PROBE_REQ,
	WIFI_FRAME_TYPE_ACTION,
} wifi_mgmtFrameType_t;

This is sample code for prototype:
typedef INT (* wifi_receivedMgmtFrame_callback)(INT apIndex, UCHAR *sta_mac, UCHAR *frame, UINT len, wifi_mgmtFrameType_t type);
*/
INT wifi_receivedMgmtFrame(INT apIndex, UCHAR *sta_mac, UCHAR *frame, UINT len, wifi_mgmtFrameType_t type)
{
	HAL_WIFI_DBG(("HAL %s apIndex=%d len=%d type=%d from MAC="MACF"\n",
		__FUNCTION__, apIndex, len, type, MAC_TO_MACF(sta_mac)));

	UNUSED_PARAMETER(frame);
	return RETURN_OK;
}

/* end of DPP API */
#endif /* (WIFI_HAL_VERSION_GE_2_16) */

#if WIFI_HAL_VERSION_GE_2_18 || defined(WIFI_HAL_VERSION_3)
INT wifi_chan_eventUnRegister(void)
{
	wldm_callback(WLDM_CB_UNREGISTER, FD_CH_CHG, NULL, &wifi_cb_info);

	callback_fnc.chan_change_cb = NULL;
	return RETURN_OK;
}

INT wifi_chan_eventRegister(wifi_chan_eventCB_t event_cb)
{
	if (event_cb == NULL) {
		wifi_chan_eventUnRegister();
		return RETURN_OK;
	}
	if (wldm_callback(WLDM_CB_REGISTER, FD_CH_CHG, wifi_hal_cb_channel_evt_handler, &wifi_cb_info) < 0) {
		HAL_WIFI_ERR(("Failed to register the %s callback\n", __FUNCTION__));
		return RETURN_ERR;
	}
	callback_fnc.chan_change_cb = event_cb;
	cbThreadId = wifi_cb_info.cbThreadId;

	HAL_WIFI_DBG(("Done %s channnel_change_cb=%p \n", __FUNCTION__, event_cb));

	return RETURN_OK;
}

/* Dummy function for some other DPP API */
INT wifi_dppInitiate(wifi_device_dpp_context_t *ctx)
{
	UNUSED_PARAMETER(ctx);
	HAL_WIFI_DBG(("HAL %s\n", __FUNCTION__));
	return RETURN_OK;
}

INT wifi_dppCancel(wifi_device_dpp_context_t *ctx)
{
	UNUSED_PARAMETER(ctx);
	HAL_WIFI_DBG(("HAL %s\n", __FUNCTION__));
	return RETURN_OK;
}

INT wifi_dppSendAuthCnf(wifi_device_dpp_context_t *ctx)
{
	UNUSED_PARAMETER(ctx);
	HAL_WIFI_DBG(("HAL %s\n", __FUNCTION__));
	return RETURN_OK;
}

INT wifi_dppSendConfigResponse(wifi_device_dpp_context_t *ctx)
{
	UNUSED_PARAMETER(ctx);
	HAL_WIFI_DBG(("HAL %s\n", __FUNCTION__));
	return RETURN_OK;
}

#ifdef WIFI_HAL_VERSION_3_PHASE2
INT wifi_dppSetSTAPassphrase(UINT apIndex, mac_address_t sta, CHAR *key)
{
	HAL_WIFI_DBG(("HAL %s\n", __FUNCTION__));
	return RETURN_OK;
}

INT wifi_dppRemoveSTAPassphrase(UINT apIndex, mac_address_t sta)
{
	HAL_WIFI_DBG(("HAL %s\n", __FUNCTION__));
	return RETURN_OK;
}
#else /* WIFI_HAL_VERSION_3_PHASE2 */
INT wifi_dppSetSTAPassphrase(UINT apIndex, CHAR *sta, CHAR *key)
{
	UNUSED_PARAMETER(apIndex);
	UNUSED_PARAMETER(sta);
	UNUSED_PARAMETER(key);

	HAL_WIFI_DBG(("HAL %s\n", __FUNCTION__));
	return RETURN_OK;
}

INT wifi_dppRemoveSTAPassphrase(UINT apIndex, CHAR *sta)
{
	UNUSED_PARAMETER(apIndex);
	UNUSED_PARAMETER(sta);

	HAL_WIFI_DBG(("HAL %s\n", __FUNCTION__));
	return RETURN_OK;
}
#endif /* WIFI_HAL_VERSION_3_PHASE2 */

INT wifi_dppProcessAuthResponse(wifi_device_dpp_context_t *ctx)
{
	UNUSED_PARAMETER(ctx);

	HAL_WIFI_DBG(("HAL %s\n", __FUNCTION__));
	return RETURN_OK;
}

INT wifi_dppProcessConfigRequest(wifi_device_dpp_context_t *ctx)
{
	UNUSED_PARAMETER(ctx);

	HAL_WIFI_DBG(("HAL %s\n", __FUNCTION__));
	return RETURN_OK;
}

#endif /* (WIFI_HAL_VERSION_GE_2_18) */

static int wifi_setApInterworkingElement(INT apIndex, wifi_InterworkingElement_t *infoElement)
{
	int intValue;

	HAL_WIFI_DBG(("%s: apIndex = %d\n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(infoElement);

	if (wldm_11u_iw(CMD_SET_NVRAM, apIndex,
		(void *)&(infoElement->interworkingEnabled), NULL,
		"iw_enable", NULL)) {
		HAL_WIFI_ERR(("%s: interworkingEnabled \n", __FUNCTION__));
		return RETURN_ERR;
	}

	if (wldm_11u_iw(CMD_SET_NVRAM, apIndex,
		(void *)&(infoElement->internetAvailable), NULL,
		"iw_internet_av", NULL)) {
		HAL_WIFI_ERR(("%s: internetAvailable \n", __FUNCTION__));
		return RETURN_ERR;
	}

	if (wldm_11u_iw(CMD_SET_NVRAM, apIndex,
		(void *)&(infoElement->accessNetworkType), NULL,
		"iw_net_type", NULL)) {
		HAL_WIFI_ERR(("%s: accessNetworkType \n", __FUNCTION__));
		return RETURN_ERR;
	}

	if (wldm_11u_iw(CMD_SET_NVRAM, apIndex,
		(void *)&(infoElement->asra), NULL,
		"iw_asra", NULL)) {
		HAL_WIFI_ERR(("%s: asra \n", __FUNCTION__));
		return RETURN_ERR;
	}

	if (wldm_11u_iw(CMD_SET_NVRAM, apIndex,
		(void *)&(infoElement->esr), NULL,
		"iw_esr", NULL)) {
		HAL_WIFI_ERR(("%s: esr \n", __FUNCTION__));
		return RETURN_ERR;
	}

	if (wldm_11u_iw(CMD_SET_NVRAM, apIndex,
		(void *)&(infoElement->uesa), NULL,
		"iw_uesa", NULL)) {
		HAL_WIFI_ERR(("%s: uesa \n", __FUNCTION__));
		return RETURN_ERR;
	}

	/* venueOptionPresent will be set when venuelist is set in nvram */

	/* venueGroup type is uchar, wldm_11u_iw expects int */
	intValue = infoElement->venueGroup;
	if (wldm_11u_iw(CMD_SET_NVRAM, apIndex,
		(void *)&intValue, NULL,
		"iw_venue_grp", NULL)) {
		HAL_WIFI_ERR(("%s: venueGroup \n", __FUNCTION__));
		return RETURN_ERR;
	}

	/* venueType type is uchar, wldm_11u_iw expects int */
	intValue = infoElement->venueType;
	if (wldm_11u_iw(CMD_SET_NVRAM, apIndex,
		(void *)&intValue, NULL,
		"iw_venue_type", NULL)) {
		HAL_WIFI_ERR(("%s: venueType \n", __FUNCTION__));
		return RETURN_ERR;
	}

	/*  hessOptionPresent will be set when hessid is set in nvram */

	if (wldm_11u_iw(CMD_SET_NVRAM, apIndex,
		(void *)(infoElement->hessid), NULL,
		"iw_hessid", NULL)) {
		HAL_WIFI_ERR(("%s: hessid \n", __FUNCTION__));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

INT wifi_pushApInterworkingElement(INT apIndex, wifi_InterworkingElement_t *infoElement)
{
	HAL_WIFI_DBG(("%s: apIndex = %d\n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(infoElement);

	if (wifi_setApInterworkingElement(apIndex, infoElement)) {
		HAL_WIFI_ERR(("%s: failed \n", __FUNCTION__));
		return RETURN_ERR;
	}

	/* restart hspotap right away in push cmd */
	wldm_hspot_restart_if_needed();

	return RETURN_OK;
}

#if WIFI_HAL_VERSION_GE_2_12 || defined(WIFI_HAL_VERSION_3)
// @description Get the Interworking Element that will be sent by the AP.
//
// @param apIndex - Index of the Access Point.
// @param output_struct - Interworking Element.
// @return The status of the operation.
// @retval RETURN_OK if successful.
// @retval RETURN_ERR if any error is detected.
INT wifi_getApInterworkingElement(INT apIndex, wifi_InterworkingElement_t *output_struct)
{
	int intValue;
	unsigned int len;

	HAL_WIFI_DBG(("%s: apIndex = %d\n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(output_struct);

	HAL_WIFI_DBG(("%s: apIndex %d", __FUNCTION__, apIndex));

	if (wldm_11u_iw(CMD_GET_NVRAM, apIndex,
		(void *)&(output_struct->interworkingEnabled), NULL,
		"iw_enable", NULL)) {
		HAL_WIFI_ERR(("%s: interworkingEnabled \n", __FUNCTION__));
		return RETURN_ERR;
	}

	if (wldm_11u_iw(CMD_GET_NVRAM, apIndex,
		(void *)&(output_struct->internetAvailable), NULL,
		"iw_internet_av", NULL)) {
		HAL_WIFI_ERR(("%s: internetAvailable \n", __FUNCTION__));
		return RETURN_ERR;
	}

	if (wldm_11u_iw(CMD_GET_NVRAM, apIndex,
		(void *)&(output_struct->accessNetworkType), NULL,
		"iw_net_type", NULL)) {
		HAL_WIFI_ERR(("%s: accessNetworkType \n", __FUNCTION__));
		return RETURN_ERR;
	}

	if (wldm_11u_iw(CMD_GET_NVRAM, apIndex,
		(void *)&(output_struct->asra), NULL,
		"iw_asra", NULL)) {
		HAL_WIFI_ERR(("%s: asra \n", __FUNCTION__));
		return RETURN_ERR;
	}

	if (wldm_11u_iw(CMD_GET_NVRAM, apIndex,
		(void *)&(output_struct->esr), NULL,
		"iw_esr", NULL)) {
		HAL_WIFI_ERR(("%s: esr \n", __FUNCTION__));
		return RETURN_ERR;
	}

	if (wldm_11u_iw(CMD_GET_NVRAM, apIndex,
		(void *)&(output_struct->uesa), NULL,
		"iw_uesa", NULL)) {
		HAL_WIFI_ERR(("%s: uesa \n", __FUNCTION__));
		return RETURN_ERR;
	}
	/* venueOptionPresent will be set when venuelist is set in nvram */

	/* venueGroup type is uchar, wldm_11u_iw expects int */
	if (wldm_11u_iw(CMD_GET_NVRAM, apIndex,
		(void *)&intValue, NULL,
		"iw_venue_grp", NULL)) {
		HAL_WIFI_ERR(("%s: venueGroup \n", __FUNCTION__));
		return RETURN_ERR;
	}
	output_struct->venueGroup = intValue;

	/* venueType type is uchar, wldm_11u_iw expects int */
	if (wldm_11u_iw(CMD_GET_NVRAM, apIndex,
		(void *)&intValue, NULL,
		"iw_venue_type", NULL)) {
		HAL_WIFI_ERR(("%s: venueType \n", __FUNCTION__));
		return RETURN_ERR;
	}
	output_struct->venueType = intValue;

	len = sizeof(output_struct->hessid);
	if (wldm_11u_iw(CMD_GET_NVRAM, apIndex,
		(void *)(output_struct->hessid), &len,
		"iw_hessid", NULL)) {
		HAL_WIFI_ERR(("%s: hessid \n", __FUNCTION__));
		return RETURN_ERR;
	}

	/*  hessOptionPresent will be set when hessid is set in nvram */
	output_struct->hessOptionPresent = strlen(output_struct->hessid) ? TRUE : FALSE;

	return RETURN_OK;
}

/**
 * Access Network Type value to be included in the Interworking IE
 * (refer 8.4.2.94 of IEEE Std 802.11-2012). Possible values are:
 * 0 - Private network;
 * 1 - Private network with guest access;
 * 2 - Chargeable public network;
 * 3 - Free public network;
 * 4 - Personal device network;
 * 5 - Emergency services only network;
 * 6-13 - Reserved;
 * 14 - Test or experimental;
 * 15 - Wildcard
 */
INT wifi_setInterworkingAccessNetworkType(INT apIndex, INT accessNetworkType)
{
	HAL_WIFI_DBG(("%s: apIndex = %d accessNetworkType = %d\n",
				__FUNCTION__, apIndex, accessNetworkType));

	AP_INDEX_ASSERT(apIndex);

	if (wldm_11u_iw(CMD_SET_NVRAM, apIndex, (void *)&accessNetworkType, NULL,
		"iw_net_type", NULL)) {
		HAL_WIFI_ERR(("%s: Failed to set info\n", __FUNCTION__));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

INT wifi_getInterworkingAccessNetworkType(INT apIndex, UINT *output_uint)
{
	HAL_WIFI_DBG(("%s: apIndex = %d \n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(output_uint);

	if (wldm_11u_iw(CMD_GET_NVRAM, apIndex, output_uint, NULL,
		"iw_net_type", NULL)) {
		HAL_WIFI_ERR(("%s: Failed to get info\n", __FUNCTION__));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

// @description Get the Interworking Service Capability of the AP
// @param apIndex - Index of the Access Point.
// @param output_bool - Indication as to whether the AP supports the Interworking Service.
// @return The status of the operation.
// @retval RETURN_OK if successful.
// @retval RETURN_ERR if any error is detected.
INT wifi_getApInterworkingServiceCapability(INT apIndex, BOOL *output_bool)
{
	HAL_WIFI_DBG(("%s: apIndex = %d \n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(output_bool);

	if (wldm_11u_iw(CMD_GET_NVRAM, apIndex, output_bool, NULL,
		"iw_capability", NULL)) {
		HAL_WIFI_ERR(("%s: Failed to get info\n", __FUNCTION__));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

// @description Get the Interworking Service enable/disable value for the AP.
// @param apIndex - Index of the Access Point.
// @param output_bool - Indication as to whether the AP Interworking Service is enabled (true) or disabled (false).
// @return The status of the operation.
// @retval RETURN_OK if successful.
// @retval RETURN_ERR if any error is detected.
INT wifi_getApInterworkingServiceEnable(INT apIndex, BOOL *output_bool)
{
	HAL_WIFI_DBG(("%s: apIndex = %d \n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(output_bool);

	if (wldm_11u_iw(CMD_GET_NVRAM, apIndex, output_bool, NULL,
		"iw_enable", NULL)) {
		HAL_WIFI_ERR(("%s: Failed to get info\n", __FUNCTION__));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

// @description Set the Interworking Service enable/disable value for the AP.
// @param apIndex - Index of the Access Point.
// @param input_bool - Value to set the Interworking Service enable to, true or false.
// @return The status of the operation.
// @retval RETURN_OK if successful.
// @retval RETURN_ERR if any error is detected.
INT wifi_setApInterworkingServiceEnable(INT apIndex, BOOL input_bool)
{
	HAL_WIFI_DBG(("%s: apIndex = %d input_bool = %d\n", __FUNCTION__,
				apIndex, input_bool ? 1 : 0));

	AP_INDEX_ASSERT(apIndex);

	if (wldm_11u_iw(CMD_SET_NVRAM, apIndex, (void *)&input_bool, NULL,
		"iw_enable", NULL)) {
		HAL_WIFI_ERR(("%s: Failed to set info\n", __FUNCTION__));
		return RETURN_ERR;
	}

	return RETURN_OK;
}
#endif /* #if (WIFI_HAL_VERSION_GE_2_12) */

#if WIFI_HAL_VERSION_GE_2_19 || defined(WIFI_HAL_VERSION_3)
/* add passpoint R1 APIs */
static int wifi_setApHotspotElement(INT apIndex, BOOL enabled)
{
	HAL_WIFI_DBG(("%s: apIndex = %d enabled = %d\n", __FUNCTION__,
				apIndex, enabled ? 1 : 0));

	AP_INDEX_ASSERT(apIndex);

	if (wldm_hspot(CMD_SET_NVRAM, apIndex, (void *)&enabled, NULL,
		"hs_hspot_ie", NULL)) {
		HAL_WIFI_ERR(("%s: Failed\n", __FUNCTION__));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

INT wifi_pushApHotspotElement(INT apIndex, BOOL enabled)
{
	HAL_WIFI_DBG(("%s: apIndex = %d enabled = %d\n", __FUNCTION__,
				apIndex, enabled ? 1 : 0));

	AP_INDEX_ASSERT(apIndex);

	if (wifi_setApHotspotElement(apIndex, enabled)) {
		HAL_WIFI_ERR(("%s: Failed\n", __FUNCTION__));
		return RETURN_ERR;
	}

	/* restart hspotap right away in push cmd */
	wldm_hspot_restart_if_needed();

	return RETURN_OK;
}

/***********************************************************************************
 * Description : Get Hotspot 2.0 Status for the Access Point
 * Parameters: AP Index, pointer to Enabled Status variable
 * Return: The status of the operation
 **********************************************************************************/
INT wifi_getApHotspotElement(INT apIndex, BOOL *enabled)
{
	HAL_WIFI_DBG(("%s: apIndex = %d \n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(enabled);

	if (wldm_hspot(CMD_GET_NVRAM, apIndex, (void *)enabled, NULL,
		"hs_hspot_ie", NULL)) {
		HAL_WIFI_ERR(("%s: Failed\n", __FUNCTION__));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

/***********************************************************************************
 * Description : Push Roaming Consortium Information Element Contents to HAL
 * Parameters: AP Index, pointer to wifi_roamingConsortiumElement_t
	the structure contains OI count, length of first 3 OIs,
	and first 3 OI as a hex string. When count > 0 and interworking is
	enabled, Roaming Consortium Information Element should be present
	in Beacon and Probe Response with this information.
 * Return: The status of the operation
 **********************************************************************************/
static int wifi_hs_oui2str(wifi_roamingConsortiumElement_t *infoElement, char *oui_str, int array_size)
{
	char byte_str[3];
	int i, j, len;
	UCHAR count, *phex;

	NULL_PTR_ASSERT(infoElement);
	NULL_PTR_ASSERT(oui_str);

	count = infoElement->wifiRoamingConsortiumCount;
	count = (count > 3) ? 3 : count;
	for (i = 0; i < count; i++) {
		if (i)
			strncat(oui_str, ";", (array_size - strlen(oui_str) - 1));

		phex = infoElement->wifiRoamingConsortiumOui[i];
		len = infoElement->wifiRoamingConsortiumLen[i];
		for (j = 0; j < len; j++) {
			snprintf(byte_str, sizeof(byte_str), "%02x", phex[j]);
			snprintf(oui_str, array_size, "%s", byte_str);
		}
		strncat(oui_str, ":1", (array_size - strlen(oui_str) - 1));
	}
	return RETURN_OK;
}

static int wifi_hs_str2oui(wifi_roamingConsortiumElement_t *infoElement, char *oui_str)
{
	char ouilist[128], *ptr, *s, *rest = NULL;
	int total = 0, len;

	NULL_PTR_ASSERT(infoElement);
	NULL_PTR_ASSERT(oui_str);

	strncpy(ouilist, oui_str, sizeof(ouilist) - 1);
	ptr = strtok_r(ouilist, ";", &rest);
	while (ptr) {
		if ((s = strchr(ptr, ':')) != NULL)
			*s = '\0';
		len = strlen(ptr) / 2;
		HAL_WIFI_DBG(("%s: index=%d string=%s len=%d\n",
			__FUNCTION__, total, ptr, len));
		if ((len < 15) && (total < 3)) {
			get_hex_data((unsigned char *)ptr, infoElement->wifiRoamingConsortiumOui[total], len);
			infoElement->wifiRoamingConsortiumLen[total] = len;
		} else {
			HAL_WIFI_ERR(("%s: Skip index=%d string=%s len=%d\n",
				__FUNCTION__, total, ptr, len));
		}

		/*	only allow first 3 OUIs go to the data struct,
			additional as "Number of ANQP OIs:"
		*/
		total++;
		ptr = strtok_r(NULL, ";", &rest);
	}
	infoElement->wifiRoamingConsortiumCount = total;

	return RETURN_OK;
}

static int wifi_setApRoamingConsortiumElement(INT apIndex, wifi_roamingConsortiumElement_t *infoElement)
{
	char varvalue[128] = {0}, *ptr = varvalue;
	int num_oui;

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(infoElement);

	if (wifi_hs_oui2str(infoElement, ptr, sizeof(varvalue)) != RETURN_OK) {
		HAL_WIFI_ERR(("%s: failed to convert oui to string!!!\n", __FUNCTION__));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: str=%s\n", __FUNCTION__, ptr));
	/* save ouilist */
	if (wldm_hspot(CMD_SET_NVRAM, apIndex, (void *)ptr, NULL,
		"hs_oui_list", NULL)) {
		HAL_WIFI_ERR(("%s: Failed\n", __FUNCTION__));
		return RETURN_ERR;
	}

	num_oui = (infoElement->wifiRoamingConsortiumCount > 3) ?
			(infoElement->wifiRoamingConsortiumCount - 3) : 0;

	HAL_WIFI_DBG(("%s: num_oui=%d\n", __FUNCTION__, num_oui));
	/* save oui_extra */
	if (wldm_hspot(CMD_SET_NVRAM, apIndex, (void *)&num_oui, NULL,
		"hs_oui_extra", NULL)) {
		HAL_WIFI_ERR(("%s: Failed\n", __FUNCTION__));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

INT wifi_pushApRoamingConsortiumElement(INT apIndex, wifi_roamingConsortiumElement_t *infoElement)
{
	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(infoElement);

	if (wifi_setApRoamingConsortiumElement(apIndex, infoElement) != RETURN_OK) {
		HAL_WIFI_ERR(("%s: failed\n", __FUNCTION__));
		return RETURN_ERR;
	}

	/* restart hspotap right away in push cmd */
	wldm_hspot_restart_if_needed();

	return RETURN_OK;
}

/***********************************************************************************
 * Description : Get Roaming Consortium Information Element Contents
 * Parameters: AP Index, pointer to wifi_roamingConsortiumElement_t
			If Roaming Consortium is not present, return count as 0,
			and length and OI fileds can be ignored
 * Return: The status of the operation
 **********************************************************************************/
INT wifi_getApRoamingConsortiumElement(INT apIndex, wifi_roamingConsortiumElement_t *infoElement)
{
	char varvalue[128] = {0};
	int num_oui = 0;
	unsigned int len;

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(infoElement);

	len = sizeof(varvalue);
	if (wldm_hspot(CMD_GET_NVRAM, apIndex, (void *)varvalue, &len,
		"hs_oui_list", NULL)) {
		HAL_WIFI_ERR(("%s: Failed\n", __FUNCTION__));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: oui_str=%s \n", __FUNCTION__, varvalue));

	if (wifi_hs_str2oui(infoElement, varvalue) != RETURN_OK) {
		HAL_WIFI_ERR(("%s: failed to convert string to oui!!!\n", __FUNCTION__));
		return RETURN_ERR;
	}

	len = sizeof(num_oui);
	if (wldm_hspot(CMD_GET_NVRAM, apIndex, (void *)&num_oui, &len,
		"hs_oui_extra", NULL)) {
		HAL_WIFI_DBG(("%s: Failed to read oui_extra\n", __FUNCTION__));
	}
	infoElement->wifiRoamingConsortiumCount += num_oui;

	HAL_WIFI_DBG(("%s: wifiRoamingConsortiumCount=%d \n",
		__FUNCTION__, infoElement->wifiRoamingConsortiumCount));

	return RETURN_OK;
}

/***********************************************************************************
 * Description : Set Country code information element in Beacon and Probe Response
 * Parameters: AP Index, Enabled Status
 * Return: The status of the operation
 **********************************************************************************/
INT wifi_setCountryIe(INT apIndex, BOOL enabled)
{
	HAL_WIFI_DBG(("%s: apIndex = %d enabled = %d\n", __FUNCTION__,
				apIndex, enabled ? 1 : 0));

	AP_INDEX_ASSERT(apIndex);

	if (wldm_hspot(CMD_SET_NVRAM, apIndex, enabled ? "h" : "off", NULL,
		"hs_cntry_ie", NULL)) {
		HAL_WIFI_ERR(("%s: Failed\n", __FUNCTION__));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

/***********************************************************************************
 * Description : Get status of country code information element in Beacon
				and Probe Response
 * Parameters: AP Index, pointer to Enabled Status variable
 * Return: The status of the operation
 **********************************************************************************/
INT wifi_getCountryIe(INT apIndex, BOOL *enabled)
{
	char varvalue[32];
	unsigned int len = sizeof(varvalue);

	HAL_WIFI_DBG(("%s: apIndex = %d \n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(enabled);

	if (wldm_hspot(CMD_GET_NVRAM, apIndex, (void *)varvalue, &len,
		"hs_cntry_ie", NULL)) {
		HAL_WIFI_ERR(("%s: Failed\n", __FUNCTION__));
		return RETURN_ERR;
	}
	*enabled = (strcmp("off", varvalue) != 0) ? TRUE : FALSE;

	return RETURN_OK;
}

/***********************************************************************************
 * Description : Disable P2P Cross Connect
				When Set to True, Include P2P Information element in Beacon and Probe Response
				Include P2P Manageability attribute with the Cross Connection Permitted field value 0
 * Parameters: AP Index, Disabled Status
 * Return: The status of the operation
 **********************************************************************************/
INT wifi_setP2PCrossConnect(INT apIndex, BOOL disabled)
{
	HAL_WIFI_DBG(("%s: apIndex = %d disabled = %d\n", __FUNCTION__,
				apIndex, disabled ? 1 : 0));

	AP_INDEX_ASSERT(apIndex);

	if (wldm_hspot(CMD_SET_NVRAM, apIndex, (void *)&disabled, NULL,
		"hs_p2p_cross", NULL)) {
		HAL_WIFI_ERR(("%s: Failed\n", __FUNCTION__));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

/***********************************************************************************
 * Description : Get Disable P2P Cross Connect status
 * Parameters: AP Index, pointer to Disabled Status variable
 * Return: The status of the operation
 **********************************************************************************/
INT wifi_getP2PCrossConnect(INT apIndex, BOOL *disabled)
{
	HAL_WIFI_DBG(("%s: apIndex = %d \n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(disabled);

	if (wldm_hspot(CMD_GET_NVRAM, apIndex, (void *)disabled, NULL,
		"hs_p2p_cross", NULL)) {
		HAL_WIFI_ERR(("%s: Failed\n", __FUNCTION__));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

/***********************************************************************************
 * Description : Disable DGAF. When set to true, DGAF disabled bit should be set
				in HS2.0 Indication Information Element in Beacon and Probe
 * Parameters: AP Index, Disabled Status
 * Return: The status of the operation
 **********************************************************************************/
INT wifi_setDownStreamGroupAddress(INT apIndex, BOOL disabled)
{
	HAL_WIFI_DBG(("%s: apIndex = %d disabled = %d\n", __FUNCTION__,
				apIndex, disabled ? 1 : 0));

	AP_INDEX_ASSERT(apIndex);

	if (wldm_hspot(CMD_SET_NVRAM, apIndex, (void *)&disabled, NULL,
		"hs_dgaf_ds", NULL)) {
		HAL_WIFI_ERR(("%s: Failed\n", __FUNCTION__));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

/***********************************************************************************
 * Description : Status of Disable DGAF
 * Parameters: AP Index, pointer to Disabled Status variable
 * Return: The status of the operation
 **********************************************************************************/
INT wifi_getDownStreamGroupAddress(INT apIndex, BOOL *disabled)
{
	HAL_WIFI_DBG(("%s: apIndex = %d \n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(disabled);

	if (wldm_hspot(CMD_GET_NVRAM, apIndex, (void *)disabled, NULL,
		"hs_dgaf_ds", NULL)) {
		HAL_WIFI_ERR(("%s: Failed\n", __FUNCTION__));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

/***********************************************************************************
 * Description : Enable BSS Load Information Element in Beacon/Probe Response
 * Parameters: AP Index, Enabled Status
 * Return: The status of the operation
 **********************************************************************************/
INT wifi_setBssLoad(INT apIndex, BOOL enabled)
{
	HAL_WIFI_DBG(("%s: apIndex = %d enabled = %d\n", __FUNCTION__,
				apIndex, enabled ? 1 : 0));

	AP_INDEX_ASSERT(apIndex);

	if (wldm_hspot(CMD_SET, apIndex, (void *)&enabled, NULL,
		"hs_bss_load", NULL)) {
		/*  when wlx.y interface does not exist, it could fail, but hspot anyway will
		 *  enable it by calling update_bssload_ie(hspotap, FALSE, TRUE) in addIes_hspot
		 *  from hspotap.c.
		 */
		HAL_WIFI_DBG(("%s: Ignore Failure for ap:%d\n",  __FUNCTION__, apIndex));
	}

	return RETURN_OK;
}

/***********************************************************************************
 * Description : Get Status of BSS Load Information Element in Beacon/Probe Response
 * Parameters: AP Index, pointer to Enabled Status variable
 * Return: The status of the operation
 **********************************************************************************/
INT wifi_getBssLoad(INT apIndex, BOOL *enabled)
{
	HAL_WIFI_DBG(("%s: apIndex = %d \n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(enabled);

	if (wldm_hspot(CMD_GET, apIndex, (void *)enabled, NULL,
		"hs_bss_load", NULL)) {
		/* bssload iovar read could fail if the ap is not enabled */
		HAL_WIFI_DBG(("%s: Failed\n", __FUNCTION__));
		*enabled = FALSE;
	}

	return RETURN_OK;
}

/***********************************************************************************
 * Description : Enable Proxy Arp function on device Driver
 * Parameters: AP Index, Enabled Status
 * Return: The status of the operation
 **********************************************************************************/
INT wifi_setProxyArp(INT apIndex, BOOL enabled)
{
	HAL_WIFI_DBG(("%s: apIndex = %d enabled = %d\n", __FUNCTION__,
				apIndex, enabled ? 1 : 0));

	AP_INDEX_ASSERT(apIndex);

	if (wldm_hspot(CMD_SET_NVRAM, apIndex, (void *)&enabled, NULL,
		"hs_proxy_arp", NULL)) {
		HAL_WIFI_ERR(("%s: Failed\n", __FUNCTION__));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

/***********************************************************************************
 * Description : Get Status of Proxy Arp from Driver
 * Parameters: AP Index, pointer to Enabled Status variable
 * Return: The status of the operation
 **********************************************************************************/
INT wifi_getProxyArp(INT apIndex, BOOL *enable)
{
	HAL_WIFI_DBG(("%s: apIndex = %d \n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(enable);

	if (wldm_hspot(CMD_GET_NVRAM, apIndex, (void *)enable, NULL,
		"hs_proxy_arp", NULL)) {
		HAL_WIFI_ERR(("%s: Failed\n", __FUNCTION__));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

/***********************************************************************************
 * Description : Enable Traffic Inspection and Filtering
 * Parameters: AP Index, Enabled Status
 * Return: The status of the operation
 **********************************************************************************/
INT wifi_setLayer2TrafficInspectionFiltering(INT apIndex, BOOL enabled)
{
	HAL_WIFI_DBG(("%s: apIndex = %d enabled = %d\n", __FUNCTION__,
				apIndex, enabled ? 1 : 0));

	AP_INDEX_ASSERT(apIndex);

	if (wldm_hspot(CMD_SET_NVRAM, apIndex, (void *)&enabled, NULL,
		"hs_l2_trf", NULL)) {
		HAL_WIFI_ERR(("%s: Failed\n", __FUNCTION__));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

/***********************************************************************************
 * Description : Get Traffic Inspection and Filtering status
 * Parameters: AP Index, pointer to Enabled Status variable
 * Return: The status of the operation
 **********************************************************************************/
INT wifi_getLayer2TrafficInspectionFiltering(INT apIndex, BOOL *enabled)
{
	HAL_WIFI_DBG(("%s: apIndex = %d \n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(enabled);

	if (wldm_hspot(CMD_GET_NVRAM, apIndex, (void *)enabled, NULL,
		"hs_l2_trf", NULL)) {
		HAL_WIFI_ERR(("%s: Failed\n", __FUNCTION__));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

INT wifi_applyGASConfiguration(wifi_GASConfiguration_t *input_struct)
{
	HAL_WIFI_DBG(("%s: \n", __FUNCTION__));

	NULL_PTR_ASSERT(input_struct);

	/* Currently only support QueryResponseLengthLimit */
	/* HSFLG_ANQP is 1 by default */
	if (wldm_hspot(CMD_SET_NVRAM, 0, (void *)&(input_struct->QueryResponseLengthLimit),
		NULL, "hs_gas_qrlimit", NULL)) {
		HAL_WIFI_ERR(("%s: Failed\n", __FUNCTION__));
		return RETURN_ERR;
	}

	/* restart hspotap right away in this apply API */
	wldm_hspot_restart_if_needed();

	return RETURN_OK;
}
#endif /* #if (WIFI_HAL_VERSION_GE_2_19) */

#if defined(WIFI_HAL_VERSION_3)
/* 802.11 standard to wifi_ieee80211Variant_t */
static struct wifi_enum_to_str_map std2ieee80211Variant_infoTable[] =
{
	/* ivar	operStd */
#ifdef WIFI_HAL_VERSION_GE_3_0_3
	{WIFI_80211_VARIANT_BE,	"be"},
#endif /* WIFI_HAL_VERSION_GE_3_0_3 */
	{WIFI_80211_VARIANT_AX,	"ax"},
	{WIFI_80211_VARIANT_AD,	"ad"},
	{WIFI_80211_VARIANT_AC,	"ac"},
	{WIFI_80211_VARIANT_H,	"h"},
	{WIFI_80211_VARIANT_N,	"n"},
	{WIFI_80211_VARIANT_G,	"g"},
	{WIFI_80211_VARIANT_B,	"b"},
	{WIFI_80211_VARIANT_A,	"a"},
	{0,  NULL}
};

#ifdef WIFI_HAL_VERSION_GE_3_0_3
#define OPER_STANDS_MASK	(WIFI_80211_VARIANT_BE | \
				WIFI_80211_VARIANT_AX |  \
				WIFI_80211_VARIANT_AC | WIFI_80211_VARIANT_N |  \
				WIFI_80211_VARIANT_G | WIFI_80211_VARIANT_B | \
				WIFI_80211_VARIANT_A)
#else
#define OPER_STANDS_MASK	(WIFI_80211_VARIANT_AX |  \
				WIFI_80211_VARIANT_AC | WIFI_80211_VARIANT_N |  \
				WIFI_80211_VARIANT_G | WIFI_80211_VARIANT_B | \
				WIFI_80211_VARIANT_A)
#endif /* WIFI_HAL_VERSION_GE_3_0_3 */
/* bandwidth string to wifi_channelBandwidth_t */
static struct wifi_enum_to_str_map wifi_bandwidth_infoTable[] =
{
	/* bwShift	bwStr */
	{WIFI_CHANNELBANDWIDTH_20MHZ,		"20"},
	{WIFI_CHANNELBANDWIDTH_40MHZ,		"40"},
	{WIFI_CHANNELBANDWIDTH_80MHZ,		"80"},
	{WIFI_CHANNELBANDWIDTH_160MHZ,		"160"},
	{WIFI_CHANNELBANDWIDTH_80_80MHZ,	"8080"},
#ifdef WIFI_HAL_VERSION_GE_3_0_3
	{WIFI_CHANNELBANDWIDTH_320MHZ,		"320"},
#endif /* WIFI_HAL_VERSION_GE_3_0_3 */
	{0,  NULL}
};

/* refer to wifi_ap_OnBoardingMethods_t for the enum definitions */
wifi_enum_to_str_map_t wps_config_method_table[] = {
	{WIFI_ONBOARDINGMETHODS_USBFLASHDRIVE,		"USBFlashDrive"},
	{WIFI_ONBOARDINGMETHODS_ETHERNET,			"Ethernet"},
	{WIFI_ONBOARDINGMETHODS_LABEL,				"Label"},
	{WIFI_ONBOARDINGMETHODS_DISPLAY,			"Display"},
	{WIFI_ONBOARDINGMETHODS_EXTERNALNFCTOKEN,	"ExternalNFCToken"},
	{WIFI_ONBOARDINGMETHODS_INTEGRATEDNFCTOKEN,	"IntegratedNFCToken"},
	{WIFI_ONBOARDINGMETHODS_NFCINTERFACE,		"NFCInterface"},
	{WIFI_ONBOARDINGMETHODS_PUSHBUTTON,			"PushButton"},
	{WIFI_ONBOARDINGMETHODS_PIN,				"Keypad"},
	{WIFI_ONBOARDINGMETHODS_PHYSICALPUSHBUTTON,	"PhysicalPushButton"},
	{WIFI_ONBOARDINGMETHODS_PHYSICALDISPLAY,	"PhysicalDisplay"},
	{WIFI_ONBOARDINGMETHODS_VIRTUALPUSHBUTTON,	"VirtualPushButton"},
	{WIFI_ONBOARDINGMETHODS_VIRTUALDISPLAY,		"VirtualDisplay"},
	{WIFI_ONBOARDINGMETHODS_EASYCONNECT,		"EASYCONNECT"}, // not expected in WPS APIs
	{0xff,						NULL}
};

/* frequency band string to wifi_freq_bands_t */
static struct wifi_enum_to_str_map wifi_freq_band_infoTable[] = {
	/* bandShift	bandStr */
	{WIFI_FREQUENCY_2_4_BAND,	"2g"},
	{WIFI_FREQUENCY_5_BAND,		"5g"},
	{WIFI_FREQUENCY_5L_BAND,	"5gl"},
	{WIFI_FREQUENCY_5H_BAND,	"5gh"},
	{WIFI_FREQUENCY_6_BAND,		"6g"},
	{WIFI_FREQUENCY_60_BAND,	"60g"},
	{0,  NULL}
};
#endif /* End of WIFI_HAL_VERSION_3 */

#ifdef WIFI_HAL_VERSION_3_PHASE2
INT wifi_getApAssociatedClientDiagnosticResult(INT apIndex, mac_address_t mac_addr, wifi_associated_dev3_t *dev_conn)
{
	wifi_associated_dev3_t *associated_dev_array = NULL,*pt = NULL;
	UINT array_size = 0, i = 0, found_match = 0;
	int ret = RETURN_ERR;

	HAL_WIFI_DBG(("%s\n", __FUNCTION__));

	NULL_PTR_ASSERT(dev_conn);
	AP_INDEX_ASSERT(apIndex);

	ret = wifi_getApAssociatedDeviceDiagnosticResult3(apIndex, &associated_dev_array, &array_size);
	if (ret != RETURN_OK) {
		HAL_WIFI_ERR(("%s: wifi_getApAssociatedDeviceDiagnosticResult3 returning error %d\n",
			__FUNCTION__, ret));
		return ret;
	}

	if (associated_dev_array == NULL) {
		HAL_WIFI_ERR(("%s: No associated clients\n", __FUNCTION__));
		return WIFI_HAL_ERROR;
	}

	for (i = 0, pt = associated_dev_array; i < array_size; i++, pt++) {
		if (!memcmp(mac_addr, pt->cli_MACAddress, sizeof(mac_address_t))) {
			found_match = 1;
			memcpy(dev_conn, pt, sizeof(wifi_associated_dev3_t));
			break;
		}
	}

	free(associated_dev_array);

	if (!found_match) {
		HAL_WIFI_ERR(("%s: Associated device "MACF" not found\n",
			__FUNCTION__, MAC_TO_MACF(mac_addr)));
		return WIFI_HAL_ERROR;
	}

	return WIFI_HAL_SUCCESS;
}
#else /* WIFI_HAL_VERSION_3_PHASE2 */
#if WIFI_HAL_VERSION_GE_2_17
INT wifi_getApAssociatedClientDiagnosticResult(INT apIndex, char *mac_addr, wifi_associated_dev3_t *dev_conn)
{
	wifi_associated_dev3_t *associated_dev_array = NULL,*pt = NULL;
	UINT array_size = 0;
	UINT i = 0, found_match = 0;
	mac_address_t mac_address;
	int ret = RETURN_ERR;

	HAL_WIFI_DBG(("%s\n", __FUNCTION__));

	NULL_PTR_ASSERT(dev_conn);
	NULL_PTR_ASSERT(mac_addr);
	AP_INDEX_ASSERT(apIndex);

	if (sscanf(mac_addr, "%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx", MAC_TO_MACF(&mac_address)) != ETHER_ADDR_LEN) {
		return RETURN_ERR;
	}

	ret = wifi_getApAssociatedDeviceDiagnosticResult3(apIndex, &associated_dev_array, &array_size);
	if (ret != RETURN_OK) {
		HAL_WIFI_ERR(("%s, wifi_getApAssociatedDeviceDiagnosticResult3 returning error %d\n",
			 __FUNCTION__, ret));
		return ret;
	}

	if (associated_dev_array == NULL) {
		HAL_WIFI_ERR(("%s, No associated clients\n", __FUNCTION__));
		return RETURN_ERR;
	}

	for (i = 0, pt = associated_dev_array; i < array_size; i++, pt++) {
		if (!memcmp(mac_address, pt->cli_MACAddress, sizeof(mac_address_t))) {
			found_match = 1;
			memcpy(dev_conn, pt, sizeof(wifi_associated_dev3_t));
			break;
		}
	}

	free(associated_dev_array);

	if (!found_match) {
		HAL_WIFI_ERR(("%s, Associated device "MACF" not found\n",
			__FUNCTION__, MAC_TO_MACF(mac_address)));
		return RETURN_ERR;
	}

	return RETURN_OK;
}
#endif /* #if (WIFI_HAL_VERSION_GE_2_17) */
#endif /* WIFI_HAL_VERSION_3_PHASE2 */

INT wifi_setClientDetailedStatisticsEnable(INT radioIndex, BOOL enable)
{
	UNUSED_PARAMETER(radioIndex);
	UNUSED_PARAMETER(enable);

	HAL_WIFI_DBG(("%s: Returning OK Stub function\n", __FUNCTION__));
	return RETURN_OK;
}

static int
wl_allow80211ax(int radioIndex, bool enable)
{
	int len;

	len = sizeof(enable);
	if (wldm_xbrcm_radio(CMD_SET_NVRAM | CMD_SET_IOCTL,
			radioIndex, &enable, (uint *)&len, "allow80211ax", NULL) < 0) {
		HAL_WIFI_ERR(("%s: radioIndex %d rfc_allow_11ax nvram set error \n",
				__FUNCTION__, radioIndex));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

INT wifi_allow2G80211ax(BOOL enable)
{
	int radioIndex = BAND_2G_INDEX;
	HAL_WIFI_DBG(("%s: Enter, radioIndex = %d enable = %d\n", __FUNCTION__, radioIndex, enable));

	return wl_allow80211ax(radioIndex, enable);
}

static int
wl_getAllow80211ax(int radioIndex, bool *enable)
{
	int len;

	len = sizeof(*enable);
	if (wldm_xbrcm_radio(CMD_GET_NVRAM,
			radioIndex, enable, (uint *)&len, "allow80211ax", NULL) < 0) {
		HAL_WIFI_ERR(("%s: radioIndex %d rfc_allow_11ax nvram get error\n",
				__FUNCTION__, radioIndex));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

INT wifi_getAllow2G80211ax(BOOL *enable)
{
	int radioIndex = BAND_2G_INDEX;
	HAL_WIFI_DBG(("%s: Enter, radioIndex %d\n", __FUNCTION__, radioIndex));

	return wl_getAllow80211ax(radioIndex, enable);
}

#define WIFI_API_EVENT_UDP_SPORT  55010
static int	wifi_api_socket = -1;

/**
 * @brief wifi api set event parameters
 */
typedef struct {
	char	api_name[1024];
	int		radioIndex;
	char	api_data[1024];
} wifi_api_info_t;

static int wifi_api_socket_init(void)
{
	int err = 0;
	struct sockaddr_in sockaddr;

	if (wifi_api_socket < 0) {
		memset(&sockaddr, 0, sizeof(sockaddr));
		sockaddr.sin_family = AF_INET;
		sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
		sockaddr.sin_port = htons(WIFI_API_EVENT_UDP_SPORT);

		if (( wifi_api_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
			HAL_WIFI_DBG(("%s@%d Unable to create socket\n", __FUNCTION__, __LINE__ ));
			err = -1;
			goto done;
		}
		if ((err = bind(wifi_api_socket, (struct sockaddr *)&sockaddr, sizeof(sockaddr))) < 0) {
			HAL_WIFI_DBG(("%s@%d Unable to bind to loopback socket %d, closing it\n", __FUNCTION__, __LINE__, wifi_api_socket));
			err = -1;
			close(wifi_api_socket);
			goto done;
		}

		HAL_WIFI_DBG(("%s:%d initialized \n",__FUNCTION__,__LINE__));
	}

done:
	return err;
}

static int wifi_api_socket_deinit(void)
{
	if (wifi_api_socket >= 0) {
		HAL_WIFI_DBG(("%s:%d deinitialized \n",__FUNCTION__,__LINE__));
		close(wifi_api_socket);
		wifi_api_socket = -1;
	}

	return 0;
}

void wifi_api_thread_main_loop( void )
{
	char *pkt = NULL, ifname[32] = {0};
	int fdmax = -1, bytes, len = 4096, ret = RETURN_ERR;
	struct sockaddr_in from;
	socklen_t sock_len = sizeof(from);
	fd_set fdset;
	wifi_api_info_t *pApiInfo;

	HAL_WIFI_DBG(("wifi_api_thread_main_loop is ready for new msg...\n"));

	pkt = (void *)calloc(4096, 1);
	if (!pkt) {
		HAL_WIFI_DBG(("malloc fails!\n"));
		return;
	}

	WiFi_Ready_after_hal_started = TRUE;

	FD_ZERO(&fdset);
	FD_SET(wifi_api_socket, &fdset);
	fdmax = wifi_api_socket;

	while (1) {
		select(fdmax+1, &fdset, NULL, NULL, NULL);
		if (FD_ISSET(wifi_api_socket, &fdset)) {
			bytes = recvfrom(wifi_api_socket, pkt, len, 0, (struct sockaddr *)&from, &sock_len);

			pApiInfo = (wifi_api_info_t *)pkt;
			HAL_WIFI_DBG(("%s:%d pkt %s \n",__FUNCTION__,__LINE__,pApiInfo->api_name));

			if (!strcmp(pApiInfo->api_name, "wifi_setApEnable")) {
				BOOL enable = atoi(pApiInfo->api_data);
				ret = wifi_setApEnable(pApiInfo->radioIndex, enable);
			}  else if (!strcmp(pApiInfo->api_name, "wifi_apply")) {
				ret = wifi_apply();
			} else if (!strcmp(pApiInfo->api_name, "wifi_setWldmMsglevel")) {
				unsigned long msglevel;
				msglevel = strtoul(pApiInfo->api_data, NULL, 0);
				ret = wifi_setWldmMsglevel(msglevel);
			}  else if(!strcmp(pApiInfo->api_name, "wifi_setApRtsThreshold")) {
				UINT threshold = atoi(pApiInfo->api_data);
				ret = wifi_setApRtsThreshold(pApiInfo->radioIndex, threshold);
			}  else if (!strcmp(pApiInfo->api_name, "wifi_pushRadioChannel")) {
				UINT channel = atoi(pApiInfo->api_data);
				ret = wifi_pushRadioChannel(pApiInfo->radioIndex, channel);
			}  else if (!strcmp(pApiInfo->api_name, "wifi_pushRadioChannel2")) {
				UINT channel, channel_width, csa_beacon_count;
				sscanf(pApiInfo->api_data, "%d %d %d\n", &channel, &channel_width, &csa_beacon_count);
				ret = wifi_pushRadioChannel2(pApiInfo->radioIndex, channel, channel_width, csa_beacon_count);
			}  else if (!strcmp(pApiInfo->api_name, "wifi_setRadioChannelMode")) {
				CHAR channelMode[32];
				BOOL gOnlyFlag, nOnlyFlag, acOnlyFlag; /* BOOL is unsigned char */
				sscanf(pApiInfo->api_data, "%32s %hhu %hhu %hhu\n", channelMode, &gOnlyFlag, &nOnlyFlag, &acOnlyFlag);
				ret = wifi_setRadioChannelMode(pApiInfo->radioIndex, channelMode, gOnlyFlag, nOnlyFlag, acOnlyFlag);
#if WIFI_HAL_VERSION_GE_2_15 && (!defined(_CBR_PRODUCT_REQ_) && \
	!defined(_XF3_PRODUCT_REQ_))
			} else if (!strcmp(pApiInfo->api_name, "wifi_setRadioMode")) {
				CHAR channelMode[32];
				UINT pureMode;
				sscanf(pApiInfo->api_data, "%32s %d\n", channelMode, &pureMode);
				ret = wifi_setRadioMode(pApiInfo->radioIndex, channelMode, pureMode);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setDownlinkMuType")) {
				wifi_dl_mu_type_t mutype = atoi(pApiInfo->api_data);
				ret = wifi_setDownlinkMuType(pApiInfo->radioIndex, mutype);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setUplinkMuType")) {
				wifi_ul_mu_type_t mutype = atoi(pApiInfo->api_data);
				ret = wifi_setUplinkMuType(pApiInfo->radioIndex, mutype);
#endif /* (WIFI_HAL_VERSION_GE_2_15) && (!defined(_CBR_PRODUCT_REQ_) && !defined(_XF3_PRODUCT_REQ_)) */
			} else if (!strcmp(pApiInfo->api_name, "wifi_setRadioDcsScanning")) {
				UINT dcs = atoi(pApiInfo->api_data);
				ret = wifi_setRadioDcsScanning(pApiInfo->radioIndex, dcs);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setApBasicAuthenticationMode")) {
				wifi_setApBasicAuthenticationMode(pApiInfo->radioIndex, pApiInfo->api_data);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setApWpaEncryptionMode")) {
				ret = wifi_setApWpaEncryptionMode(pApiInfo->radioIndex, pApiInfo->api_data);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setApAuthMode")) {
				INT mode = atoi(pApiInfo->api_data);
				ret = wifi_setApAuthMode(pApiInfo->radioIndex, mode);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setApWpsDevicePIN")) {
				UINT pin = atoi(pApiInfo->api_data);
				ret = wifi_setApWpsDevicePIN(pApiInfo->radioIndex, pin);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setRadio11nGreenfieldEnable")) {
				BOOL enable = atoi(pApiInfo->api_data);
				ret = wifi_setRadio11nGreenfieldEnable(pApiInfo->radioIndex, enable);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setBandSteeringEnable")) {
				BOOL enable = atoi(pApiInfo->api_data);
				ret = wifi_setBandSteeringEnable(enable);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setApWpsEnable")) {
				BOOL enable = atoi(pApiInfo->api_data);
				ret = wifi_setApWpsEnable(pApiInfo->radioIndex, enable);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setRadioDCSChannelPool")) {
				ret = wifi_setRadioDCSChannelPool(pApiInfo->radioIndex, pApiInfo->api_data);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setSSIDEnable")) {
				ret = wifi_setSSIDEnable(pApiInfo->radioIndex, atoi(pApiInfo->api_data));
			} else if (!strcmp(pApiInfo->api_name, "wifi_setApSecurityModeEnabled")) {
				ret = wifi_setApSecurityModeEnabled(pApiInfo->radioIndex, pApiInfo->api_data);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setApSecurityKeyPassphrase")) {
				ret = wifi_setApSecurityKeyPassphrase(pApiInfo->radioIndex, pApiInfo->api_data);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setApIsolationEnable")) {
				BOOL enable = atoi(pApiInfo->api_data);
				ret = wifi_setApIsolationEnable(pApiInfo->radioIndex, enable);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setApMaxAssociatedDevices")) {
				unsigned int number = atoi(pApiInfo->api_data);
				ret = wifi_setApMaxAssociatedDevices(pApiInfo->radioIndex, number);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setApMacAddressControlMode")) {
				INT filterMode = atoi(pApiInfo->api_data);
				ret = wifi_setApMacAddressControlMode(pApiInfo->radioIndex, filterMode);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setRadioBasicDataTransmitRates")) {
				ret = wifi_setRadioBasicDataTransmitRates(pApiInfo->radioIndex, pApiInfo->api_data);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setRadioOperatingChannelBandwidth")) {
				ret = wifi_setRadioOperatingChannelBandwidth(pApiInfo->radioIndex, pApiInfo->api_data);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setApSecuritySecondaryRadiusServer")) {
				char ipAddress[45], secret[64];
				int port;
				sscanf(pApiInfo->api_data, "%45s %d %64s\n", ipAddress, &port, secret);
				ret = wifi_setApSecuritySecondaryRadiusServer(pApiInfo->radioIndex, ipAddress, port, secret);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setApSecurityRadiusServer")) {
				char ipAddress[45], secret[64];
				int port;
				sscanf(pApiInfo->api_data, "%45s %d %64s\n", ipAddress, &port, secret);
				ret = wifi_setApSecurityRadiusServer(pApiInfo->radioIndex, ipAddress, port, secret);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setApSecurityRadiusSettings")) {
				wifi_radius_setting_t settings;
				int PMKCaching;
				sscanf(pApiInfo->api_data, "%d %d %d %d %d %d %d %d %d",
					&settings.RadiusServerRetries,
					&settings.RadiusServerRequestTimeout,
					&settings.PMKLifetime,
					&PMKCaching,
					&settings.PMKCacheInterval,
					&settings.MaxAuthenticationAttempts,
					&settings.BlacklistTableTimeout,
					&settings.IdentityRequestRetryInterval,
					&settings.QuietPeriodAfterFailedAuthentication);
					settings.PMKCaching = PMKCaching?TRUE:FALSE;
				HAL_WIFI_DBG(("%s: wifi_setApSecurityRadiusSettings %d %d %d %d %d %d %d %d %d\n",
					__FUNCTION__,
					settings.RadiusServerRetries,
					settings.RadiusServerRequestTimeout,
					settings.PMKLifetime,
					settings.PMKCaching,
					settings.PMKCacheInterval,
					settings.MaxAuthenticationAttempts,
					settings.BlacklistTableTimeout,
					settings.IdentityRequestRetryInterval,
					settings.QuietPeriodAfterFailedAuthentication));
				ret = wifi_setApSecurityRadiusSettings(pApiInfo->radioIndex, &settings);
			} else if (!strcmp(pApiInfo->api_name, "wifi_applyRadioSettings")) {
				ret = wifi_applyRadioSettings(pApiInfo->radioIndex);
			} else if (!strcmp(pApiInfo->api_name, "wifi_applySSIDSettings")) {
				ret = wifi_applySSIDSettings(pApiInfo->radioIndex);
			} else if (!strcmp(pApiInfo->api_name, "wifi_removeApSecVaribles")) {
				ret = wifi_removeApSecVaribles(pApiInfo->radioIndex);
			} else if (!strcmp(pApiInfo->api_name, "wifi_disableApEncryption")) {
				ret = wifi_disableApEncryption(pApiInfo->radioIndex);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setApWpsConfigMethodsEnabled")) {
				ret = wifi_setApWpsConfigMethodsEnabled(pApiInfo->radioIndex, pApiInfo->api_data);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setRadioChannel")) {
				ret = wifi_setRadioChannel(pApiInfo->radioIndex, atoi(pApiInfo->api_data));
#if WIFI_HAL_VERSION_GE_2_15 && (!defined(_CBR_PRODUCT_REQ_) && \
	!defined(_XF3_PRODUCT_REQ_))
			} else if (!strcmp(pApiInfo->api_name, "wifi_setBSSColorEnabled")) {
				ret = wifi_setBSSColorEnabled(pApiInfo->radioIndex, atoi(pApiInfo->api_data));
#endif /* (WIFI_HAL_VERSION_GE_2_15) &&
	(!defined(_CBR_PRODUCT_REQ_) && !defined(_XF3_PRODUCT_REQ_)) */
			} else if (!strcmp(pApiInfo->api_name, "wifi_setApRetryLimit")) {
				ret = wifi_setApRetryLimit(pApiInfo->radioIndex, atoi(pApiInfo->api_data));
			} else if (!strcmp(pApiInfo->api_name, "wifi_setRadioDCSEnable")) {
				ret = wifi_setRadioDCSEnable(pApiInfo->radioIndex, atoi(pApiInfo->api_data));
			} else if (!strcmp(pApiInfo->api_name, "wifi_setRadioBeaconPeriod")) {
				ret = wifi_setRadioBeaconPeriod(pApiInfo->radioIndex, atoi(pApiInfo->api_data));
			} else if (!strcmp(pApiInfo->api_name, "wifi_setRadioAutoChannelEnable")) {
				BOOL enable = atoi(pApiInfo->api_data);
				ret = wifi_setRadioAutoChannelEnable(pApiInfo->radioIndex, enable);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setRadioAutoChannelRefreshPeriod")) {
				ULONG period = (ULONG)atoi(pApiInfo->api_data);
				ret = wifi_setRadioAutoChannelRefreshPeriod(pApiInfo->radioIndex, period);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setRadioDfsRefreshPeriod")) {
				ULONG period = (ULONG)atoi(pApiInfo->api_data);
				ret = wifi_setRadioDfsRefreshPeriod(pApiInfo->radioIndex, period);
			} else if ((!strcmp(pApiInfo->api_name, "wifi_setRadioDfsEnable")) ||
				   (!strcmp(pApiInfo->api_name, "wifi_setRadioDFSEnable"))) {
				BOOL enable = atoi(pApiInfo->api_data);
				ret = wifi_setRadioDfsEnable(pApiInfo->radioIndex, enable);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setRadioEnable")) {
				BOOL enable = atoi(pApiInfo->api_data);
				ret = wifi_setRadioEnable(pApiInfo->radioIndex, enable);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setApDTIMInterval")) {
				int dtim = atoi(pApiInfo->api_data);
				ret = wifi_setApDTIMInterval(pApiInfo->radioIndex, dtim);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setRadioTransmitPower")) {
				int power = atoi(pApiInfo->api_data);
				ret = wifi_setRadioTransmitPower(pApiInfo->radioIndex, power);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setRadioCountryCode")) {
				ret = wifi_setRadioCountryCode(pApiInfo->radioIndex, pApiInfo->api_data);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setRadioIEEE80211hEnabled")) {
				BOOL enable = atoi(pApiInfo->api_data) ? TRUE : FALSE;
				ret = wifi_setRadioIEEE80211hEnabled(pApiInfo->radioIndex, enable);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setRadioCtsProtectionEnable")) {
				int enable = atoi(pApiInfo->api_data);
				ret = wifi_setRadioCtsProtectionEnable(pApiInfo->radioIndex, enable);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setRadioObssCoexistenceEnable")) {
				int enable = atoi(pApiInfo->api_data);
				ret = wifi_setRadioObssCoexistenceEnable(pApiInfo->radioIndex, enable);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setRadioFragmentationThreshold")) {
				int thresh = atoi(pApiInfo->api_data);
				ret = wifi_setRadioFragmentationThreshold(pApiInfo->radioIndex, thresh);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setRadioSTBCEnable")) {
				int enable = atoi(pApiInfo->api_data);
				ret = wifi_setRadioSTBCEnable(pApiInfo->radioIndex, enable);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setRadioAMSDUEnable")) {
				int enable = atoi(pApiInfo->api_data);
				ret = wifi_setRadioAMSDUEnable(pApiInfo->radioIndex, enable);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setRadioGuardInterval")) {
				ret = wifi_setRadioGuardInterval(pApiInfo->radioIndex, pApiInfo->api_data);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setRadioAMSDUEnable")) {
				int enable = atoi(pApiInfo->api_data);
				ret = wifi_setRadioAMSDUEnable(pApiInfo->radioIndex, enable);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setRadioTxChainMask")) {
				int txchain_mask = atoi(pApiInfo->api_data);
				ret = wifi_setRadioTxChainMask(pApiInfo->radioIndex, txchain_mask);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setRadioRxChainMask")) {
				int rxchain_mask = atoi(pApiInfo->api_data);
				ret = wifi_setRadioRxChainMask(pApiInfo->radioIndex, rxchain_mask);
			} else if (!strcmp(pApiInfo->api_name, "wifi_createAp")) {
				char essid[64];
				int idx, hideSsid;
				sscanf(pApiInfo->api_data, "%d %64s %d\n", &idx, essid, &hideSsid);
				ret = wifi_createAp(pApiInfo->radioIndex, idx, essid, hideSsid);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setSSIDName")) {
				ret = wifi_setSSIDName(pApiInfo->radioIndex, pApiInfo->api_data);
			} else if (!strcmp(pApiInfo->api_name, "wifi_pushSSID")) {
				ret = wifi_pushSSID(pApiInfo->radioIndex, pApiInfo->api_data);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setApSsidAdvertisementEnable")) {
				int enable = atoi(pApiInfo->api_data);
				ret = wifi_setApSsidAdvertisementEnable(pApiInfo->radioIndex, enable);
			} else if (!strcmp(pApiInfo->api_name, "wifi_pushSsidAdvertisementEnable")) {
				int enable = atoi(pApiInfo->api_data);
				ret = wifi_pushSsidAdvertisementEnable(pApiInfo->radioIndex, enable);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setApSecurityPreSharedKey")) {
				ret = wifi_setApSecurityPreSharedKey(pApiInfo->radioIndex, pApiInfo->api_data);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setApWpsEnrolleePin")) {
				ret = wifi_setApWpsEnrolleePin(pApiInfo->radioIndex, pApiInfo->api_data);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setApWpsButtonPush")) {
				ret = wifi_setApWpsButtonPush(pApiInfo->radioIndex);
			} else if (!strcmp(pApiInfo->api_name, "wifi_cancelApWPS")) {
				ret = wifi_cancelApWPS(pApiInfo->radioIndex);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setApWmmEnable")) {
				int enable = atoi(pApiInfo->api_data);
				ret = wifi_setApWmmEnable(pApiInfo->radioIndex, enable);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setApWmmUapsdEnable")) {
				int enable = atoi(pApiInfo->api_data);
				ret = wifi_setApWmmUapsdEnable(pApiInfo->radioIndex, enable);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setApBeaconInterval")) {
				int interval = atoi(pApiInfo->api_data);
				ret = wifi_setApBeaconInterval(pApiInfo->radioIndex, interval);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setApWmmOgAckPolicy")) {
				int class;
				BOOL policy; /* BOOL is unsigned char */
				sscanf(pApiInfo->api_data, "%d %hhu\n", &class, &policy);
				ret = wifi_setApWmmOgAckPolicy(pApiInfo->radioIndex, class, policy);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setRadioExtChannel")) {
				ret = wifi_setRadioExtChannel(pApiInfo->radioIndex, pApiInfo->api_data);
			} else if (!strcmp(pApiInfo->api_name, "wifi_addApAclDevice")) {
#ifdef WIFI_HAL_VERSION_3_PHASE2
				mac_address_t mac_address;
				unsigned int macInt[6];
				int k;
				sscanf(pApiInfo->api_data, "%02x:%02x:%02x:%02x:%02x:%02x", &macInt[0], &macInt[1],
					&macInt[2], &macInt[3], &macInt[4], &macInt[5]);
				for (k = 0; k < 6; k++) {
					mac_address[k] = (unsigned char)macInt[k];
				}
				ret = wifi_addApAclDevice(pApiInfo->radioIndex, mac_address);
#else
				ret = wifi_addApAclDevice(pApiInfo->radioIndex, pApiInfo->api_data);
#endif /* WIFI_HAL_VERSION_3_PHASE2 */
			} else if (!strcmp(pApiInfo->api_name, "wifi_delApAclDevices")) {
				ret = wifi_delApAclDevices(pApiInfo->radioIndex);
			} else if (!strcmp(pApiInfo->api_name, "wifi_delApAclDevice")) {
				ret = wifi_delApAclDevice(pApiInfo->radioIndex, pApiInfo->api_data);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setBandSteeringBandUtilizationThreshold")) {
				int threshold = atoi(pApiInfo->api_data);
				ret = wifi_setBandSteeringBandUtilizationThreshold(pApiInfo->radioIndex, threshold);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setApRadioIndex")) {
				int index = atoi(pApiInfo->api_data);
				ret = wifi_setApRadioIndex(pApiInfo->radioIndex, index); // Ap Index is in the radioIndex param
			} else if (!strcmp(pApiInfo->api_name, "wifi_setRadioDCSScanTime")) {
				int sec, msec;
				sscanf(pApiInfo->api_data, "%d %d\n", &sec, &msec);
				ret = wifi_setRadioDCSScanTime(pApiInfo->radioIndex, sec, msec);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setBandSteeringRSSIThreshold")) {
				int rssi = atoi(pApiInfo->api_data);
				ret = wifi_setBandSteeringRSSIThreshold(pApiInfo->radioIndex, rssi);
			} else if (!strcmp(pApiInfo->api_name, "wifi_down")) {
				ret = wifi_down();
			} else if (!strcmp(pApiInfo->api_name, "wifi_setBandSteeringApGroup")) {
				ret = wifi_setBandSteeringApGroup(pApiInfo->api_data);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setBandSteeringPhyRateThreshold")) {
				int prThreshold = atoi(pApiInfo->api_data);
				ret = wifi_setBandSteeringPhyRateThreshold(pApiInfo->radioIndex, prThreshold);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setApSecurityMFPConfig")) {
				ret = wifi_setApSecurityMFPConfig(pApiInfo->radioIndex, pApiInfo->api_data);
#if WIFI_HAL_VERSION_GE_2_12 || defined(WIFI_HAL_VERSION_3)
			} else if (!strcmp(pApiInfo->api_name, "wifi_setInterworkingAccessNetworkType")) {
				uint networkType = atoi(pApiInfo->api_data);
				ret = wifi_setInterworkingAccessNetworkType(pApiInfo->radioIndex, networkType);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setApInterworkingServiceEnable")) {
				BOOL enable = atoi(pApiInfo->api_data);
				ret = wifi_setApInterworkingServiceEnable(pApiInfo->radioIndex, enable);
#endif
			} else if (!strcmp(pApiInfo->api_name, "wifi_setApScanFilter")) {
				INT mode = 0;
				CHAR essid[OUTPUT_STRING_LENGTH_32 + 1] = {0};
				sscanf(pApiInfo->api_data, "%d %s\n", &mode, essid);
				if (!strcmp(essid, "")) {
					ret = wifi_setApScanFilter(pApiInfo->radioIndex, mode, NULL);
				} else {
					ret = wifi_setApScanFilter(pApiInfo->radioIndex, mode, essid);
				}
			} else if (!strcmp(pApiInfo->api_name, "wifi_pushApInterworkingElement")) {
				wifi_InterworkingElement_t infoElement;
				memcpy(&infoElement, pApiInfo->api_data, sizeof(wifi_InterworkingElement_t));
				ret = wifi_pushApInterworkingElement(pApiInfo->radioIndex, &infoElement);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setRMBeaconRequest")) {
				printf("In %s wifi_setRMBeaconRequest \n", __FUNCTION__);
			} else if (!strcmp(pApiInfo->api_name, "wifi_cancelRMBeaconRequest")) {
				printf("In %s wifi_cancelRMBeaconRequest\n", __FUNCTION__);
			} else if (!strcmp(pApiInfo->api_name, "wifi_getRMCapabilities")) {
				printf("TBD In %s wifi_getRMCapabilities \n", __FUNCTION__);
				snprintf(ifname, sizeof(ifname) - 1, "wl%d", pApiInfo->radioIndex);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setNeighborReports")) {
				printf("TBD In %s wifi_setNeighborReports\n", __FUNCTION__);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setNeighborReportActivation")) {
				int activate = atoi(pApiInfo->api_data);
				printf("In %s wifi_setNeighborReportActivation\n", __FUNCTION__);
				ret = wifi_setNeighborReportActivation(pApiInfo->radioIndex, activate);
			} else if (!strcmp(pApiInfo->api_name, "wifi_getNeighborReportActivation")) {
				printf("TBD In %s wifi_getNeighborReportActivation\n", __FUNCTION__);
			} else if (!strcmp(pApiInfo->api_name, "wifi_deleteAp")) {
				ret = wifi_deleteAp(pApiInfo->radioIndex);
#if WIFI_HAL_VERSION_GE_2_15 && (!defined(_CBR_PRODUCT_REQ_) && \
	!defined(_XF3_PRODUCT_REQ_))
			} else if (!strcmp(pApiInfo->api_name, "wifi_setRadioDfsMoveBackEnable")) {
				BOOL enabled = atoi(pApiInfo->api_data);
				ret = wifi_setRadioDfsMoveBackEnable(pApiInfo->radioIndex, enabled);
#endif /* (WIFI_HAL_VERSION_GE_2_15) &&
	(!defined(_CBR_PRODUCT_REQ_) && !defined(_XF3_PRODUCT_REQ_)) */
#if WIFI_HAL_VERSION_GE_2_19
			} else if (!strcmp(pApiInfo->api_name, "wifi_enableGreylistAccessControl")) {
				int enable = atoi(pApiInfo->api_data);

				ret = wifi_enableGreylistAccessControl(enable ? TRUE : FALSE);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setApDASRadiusServer")) {
				char ipAddress[64], secret[64] = {'\0'};
				int port;

				sscanf(pApiInfo->api_data, "%65s %d %64s\n", ipAddress, &port, secret);
				ret = wifi_setApDASRadiusServer(pApiInfo->radioIndex,
						ipAddress, port, secret[0] ? secret : NULL);
#endif /* WIFI_HAL_VERSION_GE_2_19 */
			} else if (!strcmp(pApiInfo->api_name, "wifi_allow2G80211ax")) {
				BOOL enable = atoi(pApiInfo->api_data);
				ret = wifi_allow2G80211ax(enable);
#if defined(WIFI_HAL_VERSION_3)
			} else if (!strcmp(pApiInfo->api_name, "wifi_setApWpsConfiguration")) {
				wifi_wps_t wpsConfig;

				memcpy(&wpsConfig, pApiInfo->api_data, sizeof(wpsConfig));
				ret = wifi_setApWpsConfiguration(pApiInfo->radioIndex, &wpsConfig);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setRadioOperatingParameters")) {
				wifi_radio_operationParam_t operationParam;
				memcpy(&operationParam, pApiInfo->api_data, sizeof(wifi_radio_operationParam_t));
				ret = wifi_setRadioOperatingParameters(pApiInfo->radioIndex, &operationParam);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setApSecurity")) {
				wifi_vap_security_t security;

				memcpy(&security, pApiInfo->api_data, sizeof(security));
				ret = wifi_setApSecurity(pApiInfo->radioIndex, &security);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setBSSColor")) {
				wifi_setBSSColor(pApiInfo->radioIndex, atoi(pApiInfo->api_data));
			} else if (!strcmp(pApiInfo->api_name, "wifi_createVAP")) {
				wifi_vap_info_map_t *map;
				int i, apIndex, configType = atoi(pApiInfo->api_data);
				int radioIndex = pApiInfo->radioIndex;
				BOOL enabled = 1, showSsid = 1, isolation = 0;

				map = (wifi_vap_info_map_t *) malloc(sizeof(wifi_vap_info_map_t));
				if (map == NULL) {
					printf("Could not allocate memory for map for API \
						wifi_getRadioVapInfoMap");
					return;
				}
				map->num_vaps = MAX_NUM_VAP_PER_RADIO;

				for (i = 0; i < MAX_NUM_VAP_PER_RADIO; i++) {

					apIndex = WL_DRIVER_TO_AP_IDX(radioIndex, i) - 1;
					map->vap_array[i].vap_index = apIndex;
					snprintf(map->vap_array[i].vap_name,
						sizeof(map->vap_array[i].vap_name),
						"%s", wldm_get_nvifname(apIndex));
					if (configType == 1) {
						snprintf(map->vap_array[i].u.bss_info.ssid,
							sizeof(map->vap_array[i].u.bss_info.ssid),
							"vap-noSec-%d", apIndex);
					} else {
						snprintf(map->vap_array[i].u.bss_info.ssid,
							sizeof(map->vap_array[i].u.bss_info.ssid),
							"vap-psk2-%d", apIndex);
					}
					map->vap_array[i].u.bss_info.enabled = enabled;
					map->vap_array[i].u.bss_info.showSsid = showSsid;
					map->vap_array[i].u.bss_info.isolation =	isolation;

					if (configType == 1) {
						enabled ^= 1;
						showSsid ^= 1;
						isolation ^= 1;
						map->vap_array[i].u.bss_info.mgmtPowerControl = -5;
						map->vap_array[i].u.bss_info.bssMaxSta = 10;
						map->vap_array[i].u.bss_info.bssTransitionActivated = FALSE;
						map->vap_array[i].u.bss_info.nbrReportActivated = FALSE;
						map->vap_array[i].u.bss_info.mac_filter_enable = FALSE;
						map->vap_array[i].u.bss_info.mac_filter_mode =
							wifi_mac_filter_mode_black_list;

						map->vap_array[i].u.bss_info.security.mode =
							wifi_security_mode_none;
						map->vap_array[i].u.bss_info.security.mfp =
							wifi_mfp_cfg_optional;
						map->vap_array[i].u.bss_info.wps.enable = FALSE;
						map->vap_array[i].u.bss_info.wmm_enabled = FALSE;
					} else {
						map->vap_array[i].u.bss_info.mgmtPowerControl = -10;
						map->vap_array[i].u.bss_info.bssMaxSta = 0;
						map->vap_array[i].u.bss_info.bssTransitionActivated = TRUE;
						map->vap_array[i].u.bss_info.nbrReportActivated = TRUE;
						map->vap_array[i].u.bss_info.mac_filter_enable = TRUE;
						map->vap_array[i].u.bss_info.mac_filter_mode =
							wifi_mac_filter_mode_white_list;

						map->vap_array[i].u.bss_info.security.mode =
							wifi_security_mode_wpa2_personal;

						map->vap_array[i].u.bss_info.security.encr =
							wifi_encryption_aes;
						map->vap_array[i].u.bss_info.security.mfp =
							wifi_mfp_cfg_optional;
						map->vap_array[i].u.bss_info.security.u.key.type =
							wifi_security_key_type_pass;
						snprintf(map->vap_array[i].u.bss_info.security.u.key.key,
							sizeof(map->vap_array[i].u.bss_info.security.u.key.key),
							"test1234");

						map->vap_array[i].u.bss_info.wps.enable = TRUE;
						snprintf(map->vap_array[i].u.bss_info.wps.pin,
							sizeof(map->vap_array[i].u.bss_info.wps.pin),
							"87878787");
						/* onBoardingMethods = Label | Display | PushButton | Keypad */
						map->vap_array[i].u.bss_info.wps.methods =
							wps_config_method_table[2].enum_val |
							wps_config_method_table[3].enum_val |
							wps_config_method_table[7].enum_val |
							wps_config_method_table[8].enum_val;
						HAL_WIFI_DBG(("%s: wps.methods=0x%lx\n", __FUNCTION__,
							map->vap_array[i].u.bss_info.wps.methods));

						map->vap_array[i].u.bss_info.wmm_enabled = TRUE;
					}

					/*
					typedef struct {
						wifi_InterworkingElement_t   interworking;
						wifi_roamingConsortiumElement_t roamingConsortium;
						wifi_anqp_settings_t			anqp;	//should not be implemented in the hal
						wifi_passpoint_settings_t   passpoint;
					} __attribute__((packed)) wifi_interworking_t;
					*/
					/* set interworking params */
					map->vap_array[i].u.bss_info.interworking.interworking.interworkingEnabled = 1;
					map->vap_array[i].u.bss_info.interworking.interworking.accessNetworkType = 2;
					map->vap_array[i].u.bss_info.interworking.interworking.internetAvailable = 3;
					map->vap_array[i].u.bss_info.interworking.interworking.asra = 1;
					map->vap_array[i].u.bss_info.interworking.interworking.esr = 1;
					map->vap_array[i].u.bss_info.interworking.interworking.uesa = 1;
					map->vap_array[i].u.bss_info.interworking.interworking.venueGroup = 2;
					map->vap_array[i].u.bss_info.interworking.interworking.venueType = 5;
					if (configType == 1) {
						map->vap_array[i].u.bss_info.interworking.interworking.hessOptionPresent = 0;
						map->vap_array[i].u.bss_info.interworking.interworking.hessid[0] = 0;
					} else {
						map->vap_array[i].u.bss_info.interworking.interworking.hessOptionPresent = 1;
						snprintf(map->vap_array[i].u.bss_info.interworking.interworking.hessid,
								sizeof(map->vap_array[i].u.bss_info.interworking.interworking.hessid),
								"00:4c:90:22:22:22");
					}

					/* set ApRoamingConsortiumElement */
					/* set ApPasspoint */
					if (configType == 1) {
						map->vap_array[i].u.bss_info.interworking.passpoint.enable = 1;
						map->vap_array[i].u.bss_info.interworking.passpoint.gafDisable = 1;
						map->vap_array[i].u.bss_info.interworking.passpoint.p2pDisable = 1;
						map->vap_array[i].u.bss_info.interworking.passpoint.l2tif = 1;
						map->vap_array[i].u.bss_info.interworking.passpoint.bssLoad = 1;
						map->vap_array[i].u.bss_info.interworking.passpoint.countryIE = 1;
						map->vap_array[i].u.bss_info.interworking.passpoint.proxyArp = 1;
					} else {
						map->vap_array[i].u.bss_info.interworking.passpoint.enable = 0;
						map->vap_array[i].u.bss_info.interworking.passpoint.gafDisable = 0;
						map->vap_array[i].u.bss_info.interworking.passpoint.p2pDisable = 0;
						map->vap_array[i].u.bss_info.interworking.passpoint.l2tif = 0;
						map->vap_array[i].u.bss_info.interworking.passpoint.bssLoad = 0;
						map->vap_array[i].u.bss_info.interworking.passpoint.countryIE = 0;
						map->vap_array[i].u.bss_info.interworking.passpoint.proxyArp = 0;
					}
				}

				ret = wifi_createVAP(radioIndex, map);
				free(map);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setEAP_Param")) {
				char param[128];
				UINT value;

				sscanf(pApiInfo->api_data, "%u %128s", &value, param);
				ret = wifi_setEAP_Param(pApiInfo->radioIndex, value, param);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setBroadcastTWTSchedule")) {
				/* TBD */
			} else if (!strcmp(pApiInfo->api_name, "wifi_setTeardownTWTSession")) {
				int sessionID;
				sscanf(pApiInfo->api_data, "%d", &sessionID);
				ret = wifi_setTeardownTWTSession(pApiInfo->radioIndex, sessionID);
#endif /* WIFI_HAL_VERSION_3 */
#ifdef RDKB_LGI
			} else if (!strcmp(pApiInfo->api_name, "wifi_setRADIUSAcctEnable")) {
				BOOL enable = atoi(pApiInfo->api_data);
				ret = wifi_setRADIUSAcctEnable(pApiInfo->radioIndex, enable);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setApSecurityAcctServer")) {
				char ipAddress[45], secret[64];
				int port;
				sscanf(pApiInfo->api_data, "%45s %d %64s\n", ipAddress, &port, secret);
				ret = wifi_setApSecurityAcctServer(pApiInfo->radioIndex, ipAddress, port, secret);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setApSecuritySecondaryAcctServer")) {
				char ipAddress[45], secret[64];
				int port;
				sscanf(pApiInfo->api_data, "%45s %d %64s\n", ipAddress, &port, secret);
				ret = wifi_setApSecuritySecondaryAcctServer(pApiInfo->radioIndex, ipAddress, port, secret);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setApSecurityAcctInterimInterval")) {
				UINT interval = atoi(pApiInfo->api_data);
				ret = wifi_setApSecurityAcctInterimInterval(pApiInfo->radioIndex, interval);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setApRadiusTransportInterface")) {
				int RadiusInterface = atoi(pApiInfo->api_data);
				ret = wifi_setApRadiusTransportInterface(RadiusInterface);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setApRadiusReAuthInterval")) {
				UINT interval = strtoul(pApiInfo->api_data, NULL, 10);
				ret = wifi_setApRadiusReAuthInterval(pApiInfo->radioIndex, interval);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setRadiusOperatorName")) {
				ret = wifi_setRadiusOperatorName(pApiInfo->radioIndex, pApiInfo->api_data);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setRadiusLocationData")) {
				ret = wifi_setRadiusLocationData(pApiInfo->radioIndex, pApiInfo->api_data);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setApPMKCacheInterval")) {
				ret = wifi_setApPMKCacheInterval(pApiInfo->radioIndex, atoi(pApiInfo->api_data));
			} else if (!strcmp(pApiInfo->api_name, "wifi_setSupportRatesBitmapControlFeature")) {
				BOOL enable = atoi(pApiInfo->api_data);
				ret = wifi_setSupportRatesBitmapControlFeature(enable);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setSupportRatesDisableBasicRates")) {
				ret = wifi_setSupportRatesDisableBasicRates(pApiInfo->radioIndex, pApiInfo->api_data);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setSupportRatesDisableSupportedRates")) {
				ret = wifi_setSupportRatesDisableSupportedRates(pApiInfo->radioIndex, pApiInfo->api_data);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setRadioExcludeDfs")) {
				BOOL enable = atoi(pApiInfo->api_data);
				ret = wifi_setRadioExcludeDfs(pApiInfo->radioIndex, enable);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setRadioChannelWeights")) {
				BOOL enable = atoi(pApiInfo->api_data);
				ret = wifi_setRadioChannelWeights(pApiInfo->radioIndex, enable);
#endif /* RDKB_LGI */
			} else if (!strcmp(pApiInfo->api_name, "wifi_setApBeaconRate")) {
				ret = wifi_setApBeaconRate(pApiInfo->radioIndex, pApiInfo->api_data);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setRadioAutoBlockAckEnable")) {
				BOOL enable = atoi(pApiInfo->api_data);
				ret = wifi_setRadioAutoBlockAckEnable(pApiInfo->radioIndex, enable);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setRadioMCS")) {
				int RadioMCS = atoi(pApiInfo->api_data);
				ret = wifi_setRadioMCS(pApiInfo->radioIndex, RadioMCS);
			} else if (!strcmp(pApiInfo->api_name, "wifi_setApSecurityReset")) {
				ret = wifi_setApSecurityReset(pApiInfo->radioIndex);
			} else if (!strcmp(pApiInfo->api_name, "wifi_factoryResetRadio")) {
				ret = wifi_factoryResetRadio(pApiInfo->radioIndex);
			} else if (!strcmp(pApiInfo->api_name, "wifi_pushApSsidAdvertisementEnable")) {
				BOOL enable = atoi(pApiInfo->api_data);
				ret = wifi_pushApSsidAdvertisementEnable(pApiInfo->radioIndex, enable);
			} else {
				HAL_WIFI_DBG(("%s:%d wifi api set %s unsupported \n",__FUNCTION__,__LINE__,pApiInfo->api_name));
			}

			/* Send the execution result back to wifi_api */
			if (sendto(wifi_api_socket, &ret, sizeof(ret), 0, (struct sockaddr *)&from, sock_len) < 0) {
				HAL_WIFI_ERR(("### %s: %s send ret %d failed, errno %d ###\n",
					__FUNCTION__, pApiInfo->api_name, ret, errno));
			} else {
				HAL_WIFI_DBG(("### %s: %s send ret %d done ###\n",
					__FUNCTION__, pApiInfo->api_name, ret));
			}

			if (bytes <= 0) {
				HAL_WIFI_DBG(("Recv bytes Failure...\n"));
				continue;
			}
		}
	}
	if (pkt) {
		free((void *)pkt);
	}
	wifi_api_socket_deinit();
	pthread_exit(NULL);
}

/* 802.11r FBT HAL Imp. Starts */
#ifdef MAX_KEY_HOLDERS
/* wifi_hal.h's 802.11r Fast Trasition definitions. macro MAX_KEY_HOLDERS as a condition to avoid RDKM compiling errors */

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
#if WIFI_HAL_VERSION_GE_2_12 || defined(WIFI_HAL_VERSION_3)
INT wifi_setFastBSSTransitionActivated(INT apIndex, UCHAR activate)
{
	INT ret = 0;
#else
UINT wifi_setFastBSSTransitionActivated(INT apIndex, UCHAR activate)
{
	UINT ret = 0;
#endif
	int len = sizeof(activate);
	ret = wldm_11r_ft(WLDM_FT_SET_ACTIVATED_NVRAM, apIndex, &activate, &len, NULL);
	if (ret < 0) {
		HAL_WIFI_ERR(("%s: apIndex=[%d], wldm_11r_ft WLDM_FT_SET_ACTIVATED_NVRAM failed\n",
			__FUNCTION__, apIndex));
	} else {
		len = sizeof(activate);
		ret = wldm_11r_ft(WLDM_FT_SET_ACTIVATED, apIndex, &activate, &len, NULL);
		if (ret < 0) {
			HAL_WIFI_ERR(("%s: apIndex=[%d], wldm_11r_ft WLDM_FT_SET_ACTIVATED failed\n",
				__FUNCTION__, apIndex));
		}
	}
	return (ret);
}

/* @description Get the Fast Transition capability value.
 *
 * Receipt of the TR-181 Object for reading
 * Device.WiFi.AccessPoint.{i}.X_RDKCENTRAL-COM_BSSTransitionActivated via
 * TR-069 or WebPA causes this function to be called with the value of the
 * Object.
 *
 * @param apIndex - AP Index the setting applies to.
 * @param activate - 0 = disabled, 1 = full FT support, 2 = adaptive support.
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */
#if WIFI_HAL_VERSION_GE_2_12 || defined(WIFI_HAL_VERSION_3)
INT wifi_getBSSTransitionActivated(INT apIndex, BOOL *activate)
{
	INT ret;
#else
UINT wifi_getBSSTransitionActivated(INT apIndex, BOOL *activate)
{
	UINT ret;
#endif
	int len = sizeof(*activate);
	ret = wldm_11r_ft(WLDM_FT_GET_ACTIVATED, apIndex, activate, &len, NULL);
	return ret;
}

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
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */
#if WIFI_HAL_VERSION_GE_2_12 || defined(WIFI_HAL_VERSION_3)
INT wifi_getFTOverDSActivated(INT apIndex, BOOL *activate)
{
	INT ret;
#else
UINT wifi_getFTOverDSActivated(INT apIndex, BOOL *activate)
{
	UINT ret;
#endif
	int len = sizeof(*activate);
	ret = wldm_11r_ft(WLDM_FT_GET_OverDSACTIVATED, apIndex, activate, &len, NULL);
	return ret;
}

/* @description Set the Fast Transition over DS activated value.
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
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */
/* UINT wifi_setFTOverDSActivated(UINT apIndex, BOOL activate) BCM suggested API */
#if WIFI_HAL_VERSION_GE_2_12 || defined(WIFI_HAL_VERSION_3)
INT wifi_setFTOverDSActivated(INT apIndex, BOOL *activate)
{
	INT ret;
#else
UINT wifi_setFTOverDSActivated(INT apIndex, BOOL *activate)
{
	UINT ret;
#endif
	int len = sizeof(*activate);
	ret = wldm_11r_ft(WLDM_FT_SET_OverDSACTIVATED, apIndex, activate, &len, NULL);
	return ret;
}

/* @description Get the Fast Transition Mobility Domain value.
 * See 802.11-2016 section 13.3.
 *
 * Receipt of the TR-181 Object for reading
 * Device.WiFi.AccessPoint.{i}.X_RDKCENTRAL-COM_FTMobilityDomain via
 * TR-069 or WebPA causes this function to be called with the value of the
 * Object.
 *
 * @param apIndex - AP Index the setting applies to.
 * @param mobilityDomain - Value of the FT Mobility Domain for this AP.
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */
#if WIFI_HAL_VERSION_GE_2_12 || defined(WIFI_HAL_VERSION_3)
INT wifi_getFTMobilityDomainID(INT apIndex, UCHAR mobilityDomain[2])
{
	INT ret;
#else
UINT wifi_getFTMobilityDomainID(INT apIndex, UCHAR mobilityDomain[2])
{
	UINT ret;
#endif
	HAL_WIFI_DBG(("%s,%d apIndex %d mobilityDomain 0x%x 0x%x\n", __FUNCTION__, __LINE__, apIndex, mobilityDomain[0], mobilityDomain[1]));

	int len = 2;
	ret = wldm_11r_ft(WLDM_FT_GET_MobilityDomainID, apIndex, 0, &len, (char *)mobilityDomain);
	return ret;
}

/* @description Set the Fast Transition Mobility Domain value.
 * See 802.11-2016 section 13.3.
 *
 * Receipt of the TR-181 Object for writing
 * Device.WiFi.AccessPoint.{i}.X_RDKCENTRAL-COM_FTMobilityDomain via
 * TR-069 or WebPA causes this function to be called with the value of the
 * Object.
 *
 * @param apIndex - AP Index the setting applies to.
 * @param mobilityDomain - Value of the FT Mobility Domain for this AP.
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */
#if WIFI_HAL_VERSION_GE_2_12 || defined(WIFI_HAL_VERSION_3)
INT wifi_setFTMobilityDomainID(INT apIndex, UCHAR mobilityDomain[2])
{
	INT ret;
#else
UINT wifi_setFTMobilityDomainID(INT apIndex, UCHAR mobilityDomain[2])
{
	UINT ret;
#endif
	int len = 2;
	ret = wldm_11r_ft(WLDM_FT_SET_MobilityDomainID, apIndex, 0, &len, (char *)mobilityDomain);
	return ret;
}

/* @description Get the Fast Transition Resource Request Support value.
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
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */
#if WIFI_HAL_VERSION_GE_2_12 || defined(WIFI_HAL_VERSION_3)
INT wifi_getFTResourceRequestSupported(INT apIndex, BOOL *supported)
#else
UINT wifi_getFTResourceRequestSupported(INT apIndex, BOOL *supported)
#endif
{
	HAL_WIFI_DBG(("%s: %d apIndex %d supported 0x%p\n", __FUNCTION__, __LINE__, apIndex, supported));

	AP_INDEX_ASSERT(apIndex);

	HAL_WIFI_DBG(("%s: %d not supported yet\n", __FUNCTION__, __LINE__));

	*supported = 0;

	return RETURN_ERR;
}

/* @description Set the Fast Transition Resource Request Support value.
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
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */
#if WIFI_HAL_VERSION_GE_2_12 || defined(WIFI_HAL_VERSION_3)
INT wifi_setFTResourceRequestSupported(INT apIndex, BOOL *supported)
#else
UINT wifi_setFTResourceRequestSupported(INT apIndex, BOOL *supported)
/* UINT wifi_setFTResourceRequestSupported(UINT apIndex, BOOL supported) BCM suggested API */
#endif
{
	NULL_PTR_ASSERT(supported);
	AP_INDEX_ASSERT(apIndex);

	HAL_WIFI_DBG(("%s: %d not supported yet\n", __FUNCTION__, __LINE__));

	return RETURN_ERR;
}

/* @description Get the Fast Transition R0 Key Lifetime value.
 * See 802.11-2016 section 13.4.2.
 *
 * Receipt of the TR-181 Object for reading
 * Device.WiFi.AccessPoint.{i}.X_RDKCENTRAL-COM_FTR0KeyLifetime via
 * TR-069 or WebPA causes this function to be called with the value of the
 * Object.
 *
 * @param apIndex - AP Index the setting applies to.
 * @param lifetime - R0 Key Lifetime.
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */
#if WIFI_HAL_VERSION_GE_2_12 || defined(WIFI_HAL_VERSION_3)
INT wifi_getFTR0KeyLifetime(INT apIndex, UINT *lifetime)
#else
UINT wifi_getFTR0KeyLifetime(INT apIndex, UINT *lifetime)
#endif
{
	HAL_WIFI_DBG(("%s: %d apIndex %d lifetime 0x%p\n", __FUNCTION__, __LINE__, apIndex, lifetime));

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(lifetime);

	HAL_WIFI_DBG(("%s: %d not supported yet\n", __FUNCTION__, __LINE__));

	*lifetime = -1;

	return RETURN_ERR;
}

/* @description Set the Fast Transition R0 Key Lifetime value.
 * See 802.11-2016 section 13.4.2
 *
 * Receipt of the TR-181 Object for writing
 * Device.WiFi.AccessPoint.{i}.X_RDKCENTRAL-COM_FTR0KeyLifetime via
 * TR-069 or WebPA causes this function to be called with the value of the
 * Object.
 *
 * @param apIndex - AP Index the setting applies to.
 * @param lifetime - R0 Key Lifetime.
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */
#if WIFI_HAL_VERSION_GE_2_12 || defined(WIFI_HAL_VERSION_3)
INT wifi_setFTR0KeyLifetime(INT apIndex, UINT *lifetime)
#else
UINT wifi_setFTR0KeyLifetime(INT apIndex, UINT *lifetime)
#endif
{
	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(lifetime);

	HAL_WIFI_DBG(("%s: %d not supported yet\n", __FUNCTION__, __LINE__));

	return RETURN_ERR;
}

/* @description Get the Fast Transition R0 Key Holder ID value.
 * See 802.11-2016 section 13.3.
 *
 * Receipt of the TR-181 Object for reading
 * Device.WiFi.AccessPoint.{i}.X_RDKCENTRAL-COM_FTR0KeyHolderID via
 * TR-069 or WebPA causes this function to be called with the value of the
 * Object.
 *
 * @param apIndex - AP Index the setting applies to.
 * @param keyHolderID - R0 Key Holder ID string.
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */
#if WIFI_HAL_VERSION_GE_2_12 || defined(WIFI_HAL_VERSION_3)
INT wifi_getFTR0KeyHolderID(INT apIndex, UCHAR *keyHolderID)
{
	INT ret;
#else
UINT wifi_getFTR0KeyHolderID(INT apIndex, UCHAR *keyHolderID)
{
	UINT ret;
#endif
	ret = wldm_11r_ft(WLDM_FT_GET_R0KeyHolderID, apIndex, 0, NULL, (char *)keyHolderID);
	return ret;
}

/* @description Set the Fast Transition R0 Key Holder ID value.
 * See 802.11-2016 section 13.3.
 *
 * Receipt of the TR-181 Object for writing
 * Device.WiFi.AccessPoint.{i}.X_RDKCENTRAL-COM_FTR0KeyHolderID via
 * TR-069 or WebPA causes this function to be called with the value of the
 * Object.
 *
 * @param apIndex - AP Index the setting applies to.
 * @param keyHolderID - R0 Key Holder ID string.
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */
#if WIFI_HAL_VERSION_GE_2_12 || defined(WIFI_HAL_VERSION_3)
INT wifi_setFTR0KeyHolderID(INT apIndex, UCHAR *keyHolderID)
{
	INT ret;
#else
UINT wifi_setFTR0KeyHolderID(INT apIndex, UCHAR *keyHolderID)
{
	UINT ret;
#endif
	ret = wldm_11r_ft(WLDM_FT_SET_R0KeyHolderID, apIndex, 0, NULL, (char *)keyHolderID);
	return ret;
}

/* @description Get the Fast Transition R1 Key Holder ID value.
 * See 802.11-2016 section 13.3.
 *
 * Receipt of the TR-181 Object for reading
 * Device.WiFi.AccessPoint.{i}.X_RDKCENTRAL-COM_FTR1KeyHolderID via
 * TR-069 or WebPA causes this function to be called with the value of the
 * Object.
 *
 * @param apIndex - AP Index the setting applies to.
 * @param keyHolderID - R0 Key Holder ID string.
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */
#if WIFI_HAL_VERSION_GE_2_12 || defined(WIFI_HAL_VERSION_3)
INT wifi_getFTR1KeyHolderID(INT apIndex, UCHAR *keyHolderID)
{
	INT ret;
#else
UINT wifi_getFTR1KeyHolderID(INT apIndex, UCHAR *keyHolderID)
{
	UINT ret;
#endif
	ret = wldm_11r_ft(WLDM_FT_GET_R1KeyHolderID, apIndex, 0, NULL, (char *)keyHolderID);
	return ret;
}

/* @description Set the Fast Transition R1 Key Holder ID value.
 * See 802.11-2016 section 13.3.
 *
 * Receipt of the TR-181 Object for writing
 * Device.WiFi.AccessPoint.{i}.X_RDKCENTRAL-COM_FTR1KeyHolderID via
 * TR-069 or WebPA causes this function to be called with the value of the
 * Object.
 *
 * @param apIndex - AP Index the setting applies to.
 * @param keyHolderID - R0 Key Holder ID string.
 * @return The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected.
 */
#if WIFI_HAL_VERSION_GE_2_12 || defined(WIFI_HAL_VERSION_3)
INT wifi_setFTR1KeyHolderID(INT apIndex, UCHAR *keyHolderID)
{
	INT ret;
#else
UINT wifi_setFTR1KeyHolderID(INT apIndex, UCHAR *keyHolderID)
{
	UINT ret;
#endif
	ret = wldm_11r_ft(WLDM_FT_SET_R1KeyHolderID, apIndex, 0, NULL, (char *)keyHolderID);
	return ret;
}

INT wifi_pushApFastTransitionConfig(INT apIndex, wifi_FastTransitionConfig_t *ftData)
{
	INT ret;
	BOOL activate = 0;
	CHAR mdid[FT_LEN_MDID - 1];
	unsigned short *ptr;
	int len;

	HAL_WIFI_DBG(("%s,%d apIndex %d ftData 0x%p\n",
		__FUNCTION__, __LINE__, apIndex, ftData));

	AP_INDEX_ASSERT(apIndex);

	switch (ftData->support) {
		case FT_SUPPORT_DISABLED:
			activate = 0;
			break;
		case FT_SUPPORT_FULL:
		case FT_SUPPORT_ADAPTIVE:
			activate = 1;
			break;
		default:
			HAL_WIFI_ERR(("%s,%d apIndex %d ftData->support %d invalid\n",
				__FUNCTION__, __LINE__, apIndex, ftData->support));
			return RETURN_ERR;
	}

	len = sizeof(activate);
	ret = wldm_11r_ft(WLDM_FT_SET_ACTIVATED, apIndex, &activate, &len, NULL);
	if (ret < 0) {
		HAL_WIFI_ERR(("%s, Fail to set fbt_activate to %d for apIndex=%d\n",
			__FUNCTION__, activate, apIndex));
		return ret;
	}
	len = sizeof(activate);
	ret = wldm_11r_ft(WLDM_FT_SET_ACTIVATED_NVRAM, apIndex, &activate, &len, NULL);
	if (ret < 0) {
		HAL_WIFI_ERR(("%s, Fail to set fbt_activate nvram  to %d for apIndex=%d\n",
			__FUNCTION__, activate, apIndex));
		return ret;
	}

	len = FT_LEN_MDID - 1;
	ptr = (unsigned short *)mdid;
	*ptr = (unsigned short)ftData->mobilityDomain;
	ret = wldm_11r_ft(WLDM_FT_SET_MobilityDomainID, apIndex, 0, &len, mdid);
	if (ret	< 0) {
		HAL_WIFI_ERR(("%s, Fail to set fbt_mdid to %s for apIndex=%d\n",
			__FUNCTION__, mdid, apIndex));
		return ret;
	}

	activate = ftData->overDS;
	len = sizeof(activate);
	ret = wldm_11r_ft(WLDM_FT_SET_OverDSACTIVATED, apIndex, &activate, &len, NULL);
	if (ret < 0) {
		HAL_WIFI_ERR(("%s, Fail to set fbt_overds to %d for apIndex=%d\n",
			__FUNCTION__, activate, apIndex));
		return ret;
	}
	return RETURN_OK;
}

#endif	/* MAX_KEY_HOLDERS */
/* 802.11r FBT HAL Imp. Ends */

#if WIFI_HAL_VERSION_GE_2_15 && (!defined(_CBR_PRODUCT_REQ_) && \
	!defined(_XF3_PRODUCT_REQ_)) || defined(WIFI_HAL_VERSION_3)
/* 802.11ax HAL Imp. Starts */
static char * he_mu_type_str[HE_MU_UL_OFDMA + 1] = {"HE_MU_DL_NONE", "HE_MU_DL_OFDMA", "HE_MU_DL_HEMUMIMO",
	"HE_MU_DL_OFDMA_HEMUMIMO", "HE_MU_UL_NONE", "HE_MU_UL_OFDMA" };

INT wifi_setDownlinkMuType(INT radioIndex, wifi_dl_mu_type_t dl_mu_type)
{
	he_mu_type_t mutype = HE_MU_DL_NONE;
	uint len;

	RADIO_INDEX_ASSERT(radioIndex);
	AX_CAPABLE_ASSERT(radioIndex);
	HAL_WIFI_DBG(("%s: radioIndex=%d dl_mu_type=%d\n", __FUNCTION__, radioIndex, dl_mu_type));

	switch (dl_mu_type) {
		case WIFI_DL_MU_TYPE_NONE:
			mutype = HE_MU_DL_NONE;
			break;
#if WIFI_HAL_VERSION_GE_3_0
		case WIFI_DL_MU_TYPE_OFDMA:
#else
		case WIFI_DL_MU_TYPE_HE:
#endif
			mutype = HE_MU_DL_OFDMA;
			break;
		case WIFI_DL_MU_TYPE_MIMO:
			mutype = HE_MU_DL_HEMUMIMO;
			break;
#if WIFI_HAL_VERSION_GE_3_0
		case WIFI_DL_MU_TYPE_OFDMA_MIMO:
#else
		case WIFI_DL_MU_TYPE_HE_MIMO:
#endif
			mutype = HE_MU_DL_OFDMA_HEMUMIMO;
			break;
		default:
			HAL_WIFI_ERR(("%s: %d is invalide dl_mu_type dl_mu_type\n", __FUNCTION__, dl_mu_type));
			return RETURN_ERR;
	}

	len = sizeof(mutype);

	if (wldm_xbrcm_Radio_AXmuType(CMD_SET, radioIndex, &mutype, &len, NULL, NULL) < 0) {
		HAL_WIFI_ERR(("%s: wldm_11ac_mutype CMD_IOCTL_Set radioIndex=%d"
				"wifi_dl_mu_type_t to %d failed\n", __FUNCTION__, radioIndex, dl_mu_type));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: radioIndex=%d Set dl_mu_type[%d]= %s\n", __FUNCTION__, radioIndex,
				(int)(dl_mu_type), he_mu_type_str[(int)(dl_mu_type)]));

	return RETURN_OK;
}

INT wifi_getDownlinkMuType(INT radioIndex, wifi_dl_mu_type_t *dl_mu_type)
{
	he_mu_type_t mutype = HE_MU_DL_NONE;
	uint len;

	RADIO_INDEX_ASSERT(radioIndex);
	AX_CAPABLE_ASSERT(radioIndex);
	HAL_WIFI_DBG(("%s: radioIndex=%d\n", __FUNCTION__, radioIndex));

	len = sizeof(mutype);
	if (wldm_xbrcm_Radio_AXmuType(CMD_GET, radioIndex, &mutype, &len, NULL, NULL) < 0) {
		HAL_WIFI_ERR(("%s: wldm_11ac_mutype CMD_GET radioIndex=%d downlink failed\n", __FUNCTION__, radioIndex));
		return RETURN_ERR;
	}

	switch (mutype) {
		case HE_MU_DL_NONE:
			*dl_mu_type = WIFI_DL_MU_TYPE_NONE;
			break;
		case HE_MU_DL_OFDMA:
#if WIFI_HAL_VERSION_GE_3_0
			*dl_mu_type = WIFI_DL_MU_TYPE_OFDMA;
#else
			*dl_mu_type = WIFI_DL_MU_TYPE_HE;
#endif
			break;
		case HE_MU_DL_HEMUMIMO:
			*dl_mu_type = WIFI_DL_MU_TYPE_MIMO;
			break;
		case HE_MU_DL_OFDMA_HEMUMIMO:
#if WIFI_HAL_VERSION_GE_3_0
			*dl_mu_type = WIFI_DL_MU_TYPE_OFDMA_MIMO;
#else
			*dl_mu_type = WIFI_DL_MU_TYPE_HE_MIMO;
#endif
			break;
		default:
			HAL_WIFI_ERR(("%s: %d is invalid wifi downlink mutype\n", __FUNCTION__, mutype));
			return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: radioIndex=%d Get dl_mu_type[%d]= %s\n", __FUNCTION__, radioIndex,
			(int)(*dl_mu_type), he_mu_type_str[(int)(*dl_mu_type)]));
	return RETURN_OK;
}

INT wifi_setUplinkMuType(INT radioIndex, wifi_ul_mu_type_t ul_mu_type)
{
	he_mu_type_t mutype = HE_MU_UL_NONE;
	uint len;

	RADIO_INDEX_ASSERT(radioIndex);
	AX_CAPABLE_ASSERT(radioIndex);
	HAL_WIFI_DBG(("%s: radioIndex=%d ul_mu_type=%d\n", __FUNCTION__, radioIndex, ul_mu_type));

	switch (ul_mu_type) {
		case WIFI_UL_MU_TYPE_NONE:
			mutype = HE_MU_UL_NONE;
			break;
#if WIFI_HAL_VERSION_GE_3_0
		case WIFI_UL_MU_TYPE_OFDMA:
#else
		case WIFI_UL_MU_TYPE_HE:
#endif
			mutype = HE_MU_UL_OFDMA;
			break;
		default:
			HAL_WIFI_ERR(("%s: %d is invalide wifi_ul_mu_type_t\n", __FUNCTION__, ul_mu_type));
			return RETURN_ERR;
	}

	len = sizeof(mutype);

	if (wldm_xbrcm_Radio_AXmuType(CMD_SET, radioIndex, &mutype, &len, NULL, NULL) < 0) {
		HAL_WIFI_ERR(("%s: wldm_11ac_mutype CMD_IOCTL_Set radioIndex=%d"
			"uplink wifi_ul_mu_type_t to %d failed\n", __FUNCTION__, radioIndex, ul_mu_type));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: radioIndex=%d Set ul_mu_type[%d] = %s\n", __FUNCTION__, radioIndex,
				(int)(ul_mu_type), he_mu_type_str[(int)(ul_mu_type)]));
	return RETURN_OK;
}

INT wifi_getUplinkMuType(INT radioIndex, wifi_ul_mu_type_t *ul_mu_type)
{
	he_mu_type_t mutype = HE_MU_UL_NONE;
	uint len;

	RADIO_INDEX_ASSERT(radioIndex);
	AX_CAPABLE_ASSERT(radioIndex);
	HAL_WIFI_DBG(("%s: radioIndex=%d\n", __FUNCTION__, radioIndex));

	len = sizeof(mutype);
	if (wldm_xbrcm_Radio_AXmuType(CMD_GET, radioIndex, &mutype, &len, NULL, NULL) < 0) {
		HAL_WIFI_ERR(("%s: wldm_11ac_mutype CMD_GET radioIndex=%d"
			"uplink wifi_ul_mu_type_t failed\n", __FUNCTION__, radioIndex));
		return RETURN_ERR;
	}

	switch (mutype) {
		case HE_MU_UL_NONE:
			*ul_mu_type = WIFI_UL_MU_TYPE_NONE;
			break;
		case HE_MU_UL_OFDMA:
#if WIFI_HAL_VERSION_GE_3_0
			*ul_mu_type = WIFI_UL_MU_TYPE_OFDMA;
#else
			*ul_mu_type = WIFI_UL_MU_TYPE_HE;
#endif
			break;
		default:
			HAL_WIFI_ERR(("%s: %d is invalid wifi uplink mutype\n", __FUNCTION__, mutype));
			return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: radioIndex=%d Get ul_mu_type[%d] = %s\n", __FUNCTION__, radioIndex,
		(int)(*ul_mu_type), he_mu_type_str[(int)(*ul_mu_type)]));

	return RETURN_OK;
}

INT wifi_setGuardInterval(INT radioIndex, wifi_guard_interval_t guard_interval)
{
	int ret;
	unsigned int len;

	HAL_WIFI_DBG(("%s: Enter\n", __FUNCTION__));

	RADIO_INDEX_ASSERT(radioIndex);
	HAL_WIFI_LOG(("%s: radioIndex=%d\n", __FUNCTION__, radioIndex));

	len = sizeof(guard_interval);
	ret = wldm_xbrcm_radio(CMD_SET_IOCTL, radioIndex, &guard_interval, &len, "guardInterval", NULL);
	if (ret != 0) {
		HAL_WIFI_ERR(("%s: radioIndex=%d wldm_xbrcm_radio CMD_SET_IOCTL set %d failed\n",
					__FUNCTION__, radioIndex, (unsigned int)(guard_interval)));
#ifdef WIFI_HAL_VERSION_3
		return WIFI_HAL_ERROR;
#else
		return RETURN_ERR;
#endif
	}
	HAL_WIFI_DBG(("HAL %s radioIndex=%d Set guard_interval=%d\n",
				__FUNCTION__, radioIndex, (unsigned int)(guard_interval)));
#ifdef WIFI_HAL_VERSION_3
	return WIFI_HAL_SUCCESS;
#else
	return RETURN_OK;
#endif
}

INT wifi_getGuardInterval(INT radioIndex, wifi_guard_interval_t *guard_intervalp)
{
	int ret;
	unsigned int len;
	HAL_WIFI_DBG(("%s: Enter\n", __FUNCTION__));

	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(guard_intervalp);

	HAL_WIFI_DBG(("%s: radioIndex=%d\n", __FUNCTION__, radioIndex));

	len = sizeof(wifi_guard_interval_t);
	ret = wldm_xbrcm_radio(CMD_GET, radioIndex, guard_intervalp, &len, "guardInterval", NULL);
	if (ret != 0) {
		HAL_WIFI_ERR(("%s: radioIndex=%d wldm_xbrcm_radio CMD_GET failed\n", __FUNCTION__, radioIndex));
#ifdef WIFI_HAL_VERSION_3
		return WIFI_HAL_ERROR;
#else
		return RETURN_ERR;
#endif
	}
	HAL_WIFI_DBG(("HAL %s radioIndex=%d Got guard_interval=%d\n",
				__FUNCTION__, radioIndex, (unsigned int)(*guard_intervalp)));

#ifdef WIFI_HAL_VERSION_3
	return WIFI_HAL_SUCCESS;
#else
	return RETURN_OK;
#endif
}

INT wifi_setDownlinkDataAckType(INT radio_index, wifi_dl_data_ack_type_t ack_type)
{
	int radioIndex = radio_index;

	UNUSED_PARAMETER(ack_type);
	HAL_WIFI_DBG(("%s\n", __FUNCTION__));
	RADIO_INDEX_ASSERT(radioIndex);
	HAL_WIFI_DBG(("HAL %s radioIndex=%d\n", __FUNCTION__, radioIndex));

#ifdef WIFI_HAL_VERSION_3
	return WIFI_HAL_SUCCESS;
#else
	return RETURN_OK;
#endif
}

INT wifi_getDownlinkDataAckType(INT radio_index, wifi_dl_data_ack_type_t *ack_type)
{
	int radioIndex = radio_index;

	HAL_WIFI_DBG(("%s\n", __FUNCTION__));
	RADIO_INDEX_ASSERT(radioIndex);
	HAL_WIFI_DBG(("HAL %s radioIndex=%d\n", __FUNCTION__, radioIndex));

	*ack_type = 0;

	HAL_WIFI_DBG(("HAL %s radioIndex=%d Get ack_type=%d \n", __FUNCTION__, radioIndex, (unsigned int)(*ack_type)));

#ifdef WIFI_HAL_VERSION_3
	return WIFI_HAL_SUCCESS;
#else
	return RETURN_OK;
#endif
}

INT wifi_setBSSColorEnabled (INT apIndex, BOOL enabled)
{
	int ret;
	unsigned int radioIndex;
	unsigned int colorEn = enabled ? 1 : 0;

	HAL_WIFI_DBG(("%s\n", __FUNCTION__));
	AP_INDEX_ASSERT(apIndex);
	radioIndex = HAL_AP_IDX_TO_HAL_RADIO(apIndex);
	AX_CAPABLE_ASSERT(radioIndex);

	HAL_WIFI_DBG(("HAL %s apIndex=%d radioIndex=%d\n", __FUNCTION__, apIndex, radioIndex));

	int len = sizeof(colorEn);
	ret = wldm_AXbssColor(CMD_SET, radioIndex, &colorEn, &len, NULL, NULL);
	HAL_WIFI_DBG(("HAL %s radioIndex=%d Set colorEn=%d ret=%d\n",
		__FUNCTION__, radioIndex, (unsigned int)(colorEn), ret));

	return ret;
}

#if defined(WIFI_HAL_VERSION_3)
INT wifi_setBSSColor(INT radio_index, UCHAR color)
{
	unsigned int xcolor = color;
	int ret, len = sizeof(xcolor);

	RADIO_INDEX_ASSERT(radio_index);
	AX_CAPABLE_ASSERT(radio_index);
	HAL_WIFI_DBG(("%s: radio_index=%d color=%d\n", __FUNCTION__, radio_index, xcolor));

	ret = wldm_AXbssColor(CMD_SET, radio_index, &xcolor, &len, NULL, NULL);
	if (ret < 0) {
		HAL_WIFI_DBG(("%s: wldm_AXbssColor failed, radio_index=%d Set color=%d ret=%d\n",
			__FUNCTION__, radio_index, xcolor, ret));
		return WIFI_HAL_ERROR;
	}

	return WIFI_HAL_SUCCESS;
}

INT wifi_getAvailableBSSColor(INT radio_index, INT maxNumberColors, UCHAR* colorList, INT *numColorReturned)
{
	int ret = 0;
	unsigned int len = maxNumberColors;

	RADIO_INDEX_ASSERT(radio_index);
	AX_CAPABLE_ASSERT(radio_index);
	NULL_PTR_ASSERT(colorList);
	NULL_PTR_ASSERT(numColorReturned);

	ret = wldm_AXavailableBssColors(CMD_GET, radio_index, colorList, (int *)&len, NULL, NULL);

	if (ret < 0) {
		HAL_WIFI_ERR(("%s: wldm_AXavailableBssColors failed\n", __FUNCTION__));
		return WIFI_HAL_ERROR;
	} else {
		*numColorReturned = ret;
	}

	return WIFI_HAL_SUCCESS;
}

INT wifi_getMuEdca(INT radio_index, wifi_access_category_t ac, wifi_edca_t *edca)
{
	int ret;
	wldm_wifi_edca_t wldm_edca;
	unsigned int len = sizeof(wldm_edca);

	NULL_PTR_ASSERT(edca);
	RADIO_INDEX_ASSERT(radio_index);
	AX_CAPABLE_ASSERT(radio_index);

	if (ac == wifi_access_category_background) {
		wldm_edca.aci = AC_BK;
	} else if (ac == wifi_access_category_best_effort) {
		wldm_edca.aci = AC_BE;
	} else if (ac == wifi_access_category_video) {
		wldm_edca.aci = AC_VI;
	} else if (ac == wifi_access_category_voice) {
		wldm_edca.aci = AC_VO;
	} else {
		HAL_WIFI_ERR(("%s: Wrong ac parameter %d\n", __FUNCTION__, ac));
		return WIFI_HAL_INVALID_ARGUMENTS;
	}

	ret = wldm_xbrcm_Radio_AXmuEdca(CMD_GET, radio_index, &wldm_edca, &len, NULL, NULL);
	if (ret < 0) {
		HAL_WIFI_ERR(("%s: Error wldm_xbrxm_Radio_AXmuEdca\n", __FUNCTION__));
		return WIFI_HAL_ERROR;
	}
	edca->aifsn = wldm_edca.aifsn;
	edca->cw_min = wldm_edca.ecw_min;
	edca->cw_max = wldm_edca.ecw_max;
	edca->timer = wldm_edca.timer;

	return WIFI_HAL_SUCCESS;
}

static int wifi_setApPasspoint(int apIndex, wifi_passpoint_settings_t *passpoint)
{
	if ((wifi_setApHotspotElement(apIndex, passpoint->enable)
		!= WIFI_HAL_SUCCESS) ||
		(wifi_setDownStreamGroupAddress(apIndex, passpoint->gafDisable)
		!= WIFI_HAL_SUCCESS) ||
		(wifi_setP2PCrossConnect(apIndex, passpoint->p2pDisable)
		!= WIFI_HAL_SUCCESS) ||
		(wifi_setLayer2TrafficInspectionFiltering(apIndex, passpoint->l2tif)
		!= WIFI_HAL_SUCCESS) ||
		(wifi_setBssLoad(apIndex, passpoint->bssLoad)
		!= WIFI_HAL_SUCCESS) ||
		(wifi_setCountryIe(apIndex, passpoint->countryIE)
		!= WIFI_HAL_SUCCESS) ||
		(wifi_setProxyArp(apIndex, passpoint->proxyArp)
		!= WIFI_HAL_SUCCESS)) {
		HAL_WIFI_ERR(("%s: failed\n", __FUNCTION__));
		return WIFI_HAL_ERROR;
	}

	return WIFI_HAL_SUCCESS;
}

static int wifi_setApInterworking(int apIndex, wifi_interworking_t *interworking)
{
	int ret, result = WIFI_HAL_SUCCESS;
	wifi_InterworkingElement_t infoElement;

	HAL_WIFI_DBG(("%s: apIndex = %d\n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(interworking);

	infoElement = interworking->interworking;
	ret = wifi_setApInterworkingElement(apIndex, &infoElement);
	if (ret != WIFI_HAL_SUCCESS) {
		HAL_WIFI_ERR(("%s: wifi_setApInterworkingElement failed\n", __FUNCTION__));
		result = WIFI_HAL_ERROR;
	}

	ret = wifi_setApRoamingConsortiumElement(apIndex, &(interworking->roamingConsortium));
	if (ret != WIFI_HAL_SUCCESS) {
		HAL_WIFI_ERR(("%s: wifi_setApRoamingConsortiumElement failed\n", __FUNCTION__));
		result = WIFI_HAL_ERROR;
	}

	ret = wifi_setApPasspoint(apIndex, &(interworking->passpoint));
	if (ret != WIFI_HAL_SUCCESS) {
		HAL_WIFI_ERR(("%s: wifi_setApPasspoint failed\n", __FUNCTION__));
		result = WIFI_HAL_ERROR;
	}

	return result;
}

static int wifi_getApPasspoint(int apIndex, wifi_passpoint_settings_t *passpoint)
{
	if ((wifi_getApHotspotElement(apIndex, &(passpoint->enable))
		!= WIFI_HAL_SUCCESS) ||
		(wifi_getDownStreamGroupAddress(apIndex, &(passpoint->gafDisable))
		!= WIFI_HAL_SUCCESS) ||
		(wifi_getP2PCrossConnect(apIndex, &(passpoint->p2pDisable))
		!= WIFI_HAL_SUCCESS) ||
		(wifi_getLayer2TrafficInspectionFiltering(apIndex, &(passpoint->l2tif))
		!= WIFI_HAL_SUCCESS) ||
		(wifi_getBssLoad(apIndex, &(passpoint->bssLoad))
		!= WIFI_HAL_SUCCESS) ||
		(wifi_getCountryIe(apIndex, &(passpoint->countryIE))
		!= WIFI_HAL_SUCCESS) ||
		(wifi_getProxyArp(apIndex, &(passpoint->proxyArp))
		!= WIFI_HAL_SUCCESS)) {
		HAL_WIFI_ERR(("%s: failed\n", __FUNCTION__));
		return WIFI_HAL_ERROR;
	}

	return WIFI_HAL_SUCCESS;
}

static int wifi_getApInterworking(int apIndex, wifi_interworking_t *interworking)
{
	int ret, result = WIFI_HAL_SUCCESS;
	wifi_InterworkingElement_t infoElement;

	HAL_WIFI_DBG(("%s: apIndex = %d\n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(interworking);

	infoElement = interworking->interworking;
	ret = wifi_getApInterworkingElement(apIndex, &infoElement);
	if (ret != WIFI_HAL_SUCCESS) {
		HAL_WIFI_ERR(("%s: wifi_getApInterworkingElement failed\n", __FUNCTION__));
		result = WIFI_HAL_ERROR;
	}

	ret = wifi_getApRoamingConsortiumElement(apIndex, &(interworking->roamingConsortium));
	if (ret != WIFI_HAL_SUCCESS) {
		HAL_WIFI_ERR(("%s: wifi_getApRoamingConsortiumElement failed\n", __FUNCTION__));
		result = WIFI_HAL_ERROR;
	}

	ret = wifi_getApPasspoint(apIndex, &(interworking->passpoint));
	if (ret != WIFI_HAL_SUCCESS) {
		HAL_WIFI_ERR(("%s: wifi_getApPasspoint failed\n", __FUNCTION__));
		result = WIFI_HAL_ERROR;
	}

	return result;
}

INT wifi_getRadioVapInfoMap(wifi_radio_index_t index, wifi_vap_info_map_t *map)
{
	int i, apIndex, ret, len, enable, result = WIFI_HAL_SUCCESS;
	unsigned char rrm_val[DOT11_RRM_CAP_LEN] = {0};
	char bssid[18];
	int bssMaxSta;
	unsigned int radioIndex;

	radioIndex = index;
	RADIO_INDEX_ASSERT(radioIndex);

	NULL_PTR_ASSERT(map);

	map->num_vaps = MAX_NUM_VAP_PER_RADIO;
	for (i = 0; i < MAX_NUM_VAP_PER_RADIO; i++) {
		apIndex = WL_DRIVER_TO_AP_IDX(index, i) - 1;
		map->vap_array[i].vap_index = apIndex;

		snprintf(map->vap_array[i].vap_name, sizeof(map->vap_array[i].vap_name),
			"%s", wldm_get_osifname(apIndex));

		map->vap_array[i].radio_index = index;

		len = sizeof(map->vap_array[i].u.bss_info.ssid);
		ret = wldm_SSID_SSID(CMD_GET_NVRAM, apIndex, map->vap_array[i].u.bss_info.ssid,
			&len, NULL, NULL);
		if (ret < 0) {
			HAL_WIFI_ERR(("%s: Error wldm_SSID_SSID reading SSID, apIndex=[%d]\n",
				__FUNCTION__, apIndex));
			map->vap_array[i].u.bss_info.ssid[0] = '\0';
			result = WIFI_HAL_ERROR;
		}

		len = sizeof(map->vap_array[i].u.bss_info.enabled);
		ret = wldm_AccessPoint_Enable(CMD_GET_NVRAM, apIndex,
			&(map->vap_array[i].u.bss_info.enabled), &len, NULL, NULL);
		if (ret < 0) {
			HAL_WIFI_ERR(("%s: wldm_AccessPoint_Enable Error reading status, apIndex=[%d]\n",
				__FUNCTION__, apIndex));
			map->vap_array[i].u.bss_info.enabled = FALSE;
			result = WIFI_HAL_ERROR;
		}

		len = sizeof(map->vap_array[i].u.bss_info.showSsid);
		ret = wldm_AccessPoint_SSIDAdvertisementEnabled(CMD_GET_NVRAM, apIndex,
			&(map->vap_array[i].u.bss_info.showSsid), &len, NULL, NULL);
		if (ret < 0) {
			HAL_WIFI_ERR(("%s: wldm_AccessPoint_SSIDAdvertisementEnabled Error"
				" reading Status, apIndex=[%d]\n", __FUNCTION__, apIndex));
			map->vap_array[i].u.bss_info.showSsid = FALSE;
			result = WIFI_HAL_ERROR;
		}

		len = sizeof(map->vap_array[i].u.bss_info.isolation);
		ret = wldm_AccessPoint_IsolationEnable(CMD_GET_NVRAM, apIndex,
			&(map->vap_array[i].u.bss_info.isolation), &len, NULL, NULL);
		if (ret < 0) {
			HAL_WIFI_ERR(("%s: wldm_AccessPoint_IsolationEnable Error reading"
				" status, apIndex=[%d]\n", __FUNCTION__, apIndex));
			map->vap_array[i].u.bss_info.isolation = FALSE;
			result = WIFI_HAL_ERROR;
		}

		len = sizeof(map->vap_array[i].u.bss_info.mgmtPowerControl);
		ret = wldm_xbrcm_ap(CMD_GET_NVRAM, apIndex,
			(int *) &(map->vap_array[i].u.bss_info.mgmtPowerControl), (uint *)&len,
			"bcnprs_txpwr_offset", NULL);
		if (ret < 0) {
			HAL_WIFI_ERR(("%s: wldm_xbrcm_ap CMD_GET bcnprs_txpwr_offset for apIndex=[%d]"
				" failed\n", __FUNCTION__, apIndex));
			map->vap_array[i].u.bss_info.mgmtPowerControl = 0;
			result = WIFI_HAL_ERROR;
		}

		if (map->vap_array[i].u.bss_info.mgmtPowerControl > 0) {
			map->vap_array[i].u.bss_info.mgmtPowerControl =
				-map->vap_array[i].u.bss_info.mgmtPowerControl;
		}

		len = sizeof(map->vap_array[i].u.bss_info.bssMaxSta);
		bssMaxSta = map->vap_array[i].u.bss_info.bssMaxSta;
		ret = wldm_AccessPoint_MaxAssociatedDevices(CMD_GET_NVRAM, apIndex,
			&bssMaxSta, &len, NULL, NULL);
		if (ret < 0) {
			HAL_WIFI_ERR(("%s: wldm_AccessPoint_MaxAssociatedDevices failed, apIndex=[%d]\n",
				__FUNCTION__, apIndex));
			map->vap_array[i].u.bss_info.bssMaxSta = 0;
			result = WIFI_HAL_ERROR;
		}

		ret = wldm_11v_btm(WLDM_BTM_GET_ACTIVATION, apIndex, NULL,
			&(map->vap_array[i].u.bss_info.bssTransitionActivated), NULL, 0, NULL);
		if (ret < 0) {
			HAL_WIFI_ERR(("%s: wldm_11v_btm WLDM_BTM_GET_ACTIVATION Failed, apIndex=[%d]\n",
				__FUNCTION__, apIndex));
			map->vap_array[i].u.bss_info.bssTransitionActivated = FALSE;
			result = WIFI_HAL_ERROR;
		}

		len = DOT11_RRM_CAP_LEN;
		ret = wldm_xbrcm_AccessPoint_RMCapabilities(CMD_GET_NVRAM, apIndex, rrm_val, &len, NULL, NULL);
		if (ret < 0) {
			HAL_WIFI_ERR(("%s: Error getting rmcap apIndex[%d]\n",
				__FUNCTION__, apIndex));
			result = WIFI_HAL_ERROR;
		}
		map->vap_array[i].u.bss_info.nbrReportActivated = RRM_CAP_NEIGHBOR_REPORT_GET(rrm_val[0]);

		ret = wifi_getApSecurity(apIndex, &(map->vap_array[i].u.bss_info.security));
		if (ret != WIFI_HAL_SUCCESS) {
			HAL_WIFI_ERR(("%s: wifi_getApSecurity failed, apIndex=[%d]\n",
				__FUNCTION__, apIndex));
			result = WIFI_HAL_ERROR;
		}

		ret = wifi_getApInterworking(apIndex, &(map->vap_array[i].u.bss_info.interworking));
		if (ret != WIFI_HAL_SUCCESS) {
			HAL_WIFI_ERR(("%s: wifi_getApInterworking failed, apIndex=[%d]\n",
				__FUNCTION__, apIndex));
			result = WIFI_HAL_ERROR;
		}

		len = sizeof(enable);
		ret = wldm_AccessPoint_MACAddressControMode(CMD_GET_NVRAM, apIndex,
			&enable, &len, NULL, NULL);
		if (ret < 0) {
			HAL_WIFI_ERR(("%s: wldm_AccessPoint_MACAddressControMode failed, apIndex=[%d]\n",
				__FUNCTION__, apIndex));
			map->vap_array[i].u.bss_info.mac_filter_enable = FALSE;
			result = WIFI_HAL_ERROR;
		} else {
			if (enable == 0) {
				map->vap_array[i].u.bss_info.mac_filter_enable = FALSE;
			} else if (enable == 1) {
				map->vap_array[i].u.bss_info.mac_filter_enable = TRUE;
				map->vap_array[i].u.bss_info.mac_filter_mode =
					wifi_mac_filter_mode_black_list;
			} else {
				map->vap_array[i].u.bss_info.mac_filter_enable = TRUE;
				map->vap_array[i].u.bss_info.mac_filter_mode =
					wifi_mac_filter_mode_white_list;
			}
		}

		ret = wifi_getApWpsConfiguration(apIndex, &(map->vap_array[i].u.bss_info.wps));
		if (ret < WIFI_HAL_SUCCESS) {
			HAL_WIFI_ERR(("%s: wifi_getApWpsConfiguration failed for apIndex=[%d]\n",
				__FUNCTION__, apIndex));
			result = WIFI_HAL_ERROR;
		}

		len = sizeof(map->vap_array[i].u.bss_info.wmm_enabled);
		ret = wldm_AccessPoint_WMMEnable(CMD_GET_NVRAM, apIndex,
			&(map->vap_array[i].u.bss_info.wmm_enabled), &len, NULL, NULL);
		if (ret < 0) {
			HAL_WIFI_ERR(("%s: wldm_AccessPoint_WMMEnable CMD_GET_NVRAM failed for apIndex=[%d]\n",
				__FUNCTION__, apIndex));
			result = WIFI_HAL_ERROR;
		}

		len = sizeof(bssid);
		ret = wldm_SSID_MACAddress(CMD_GET_NVRAM, apIndex, bssid, &len, NULL, NULL);
		if (ret < 0) {
			HAL_WIFI_ERR(("%s: wldm_SSID_MACAddress Error reading from nvram\n", __FUNCTION__));
			snprintf(bssid, sizeof(bssid), "00:00:00:00:00:00");
			result = WIFI_HAL_ERROR;
		}
		MACF_TO_MAC(bssid, map->vap_array[i].u.bss_info.bssid);

		len = sizeof(map->vap_array[i].u.bss_info.UAPSDEnabled);
		ret = wldm_AccessPoint_UAPSDEnable(CMD_GET_NVRAM, apIndex,
			&(map->vap_array[i].u.bss_info.UAPSDEnabled), &len, NULL, NULL);
		if (ret < 0) {
			HAL_WIFI_ERR(("%s wldm_AccessPoint_UAPSDEnable failed for apIndex=[%d]\n",
					__FUNCTION__, apIndex));
			result = WIFI_HAL_ERROR;
		}

	}

	HAL_WIFI_DBG(("%s: radioIndex=[%d], result = %d\n", __FUNCTION__, index, result));
	return result;
}

INT wifi_createVAP(wifi_radio_index_t index, wifi_vap_info_map_t *map)
{
	int ap_index, ret, len, enable, result = WIFI_HAL_SUCCESS;
	unsigned char rrm_val[DOT11_RRM_CAP_LEN] = { 0 };
	unsigned int i;
	int mgmtPowerControl;
	wifi_wps_t wpsCfg;
	int wpsSet = 1, priApIndex;
	bool prePrimaryEnabled = TRUE, pstPrimaryEnabled = FALSE, validPrimaryStatus = FALSE;
	int bssMaxSta;
	unsigned int radioIndex;

	radioIndex = index;
	RADIO_INDEX_ASSERT(radioIndex);
	HAL_WIFI_LOG(("%s radioIndex = %d\n", __FUNCTION__, index));

	priApIndex = HAL_RADIO_IDX_TO_HAL_AP(index);
	len = sizeof(prePrimaryEnabled);
	if (wldm_AccessPoint_Enable(CMD_GET, priApIndex, &prePrimaryEnabled, &len, NULL, NULL) < 0) {
		HAL_WIFI_ERR(("%s: pre createVAP, get AP Enable Failed, priApIndex = %d\n",
			__FUNCTION__, priApIndex));
		result = WIFI_HAL_ERROR;
	} else {
		HAL_WIFI_DBG(("%s: pre createVAP priApIndex %d is %s\n", __FUNCTION__, priApIndex,
			prePrimaryEnabled ?  "Up" : "Down"));
		validPrimaryStatus = TRUE;
	}
	for (i = 0; i < map->num_vaps; i++) {
		ap_index = map->vap_array[i].vap_index;
		AP_INDEX_ASSERT(ap_index);

		HAL_WIFI_DBG(("%s: ssid=%s, enabled=%d, showSsid=%d, isolation=%d, security=%d\n"
			"\tmgmtPowerControl=%d, bssMaxSta=%d, bssTransitionActivated=%d, nbrReportActivated=%d\n"
			"\tmac_filter_enable=%d, mac_filter_mode=%d\n",
			__FUNCTION__, map->vap_array[i].u.bss_info.ssid,
			map->vap_array[i].u.bss_info.enabled, map->vap_array[i].u.bss_info.showSsid,
			map->vap_array[i].u.bss_info.isolation,
			map->vap_array[i].u.bss_info.security.mode,
			map->vap_array[i].u.bss_info.mgmtPowerControl,
			map->vap_array[i].u.bss_info.bssMaxSta,
			map->vap_array[i].u.bss_info.bssTransitionActivated,
			map->vap_array[i].u.bss_info.nbrReportActivated,
			map->vap_array[i].u.bss_info.mac_filter_enable,
			map->vap_array[i].u.bss_info.mac_filter_mode));

		ret = wldm_AccessPoint_Enable(CMD_SET, ap_index,
			&(map->vap_array[i].u.bss_info.enabled), NULL, NULL, NULL);
		if (ret < 0) {
			HAL_WIFI_ERR(("%s: wldm_AccessPoint_Enable CMD_SET failed, ap_index=[%d]\n",
				__FUNCTION__, ap_index));
			result = WIFI_HAL_ERROR;
		}

		if (!map->vap_array[i].u.bss_info.enabled) {
			HAL_WIFI_DBG(("%s: ap_index=[%d], map->vap_array[%d].u.bss_info.enabled=%d\n",
					__FUNCTION__, ap_index, i, map->vap_array[i].u.bss_info.enabled));
			continue;
		}

		len = sizeof(map->vap_array[i].u.bss_info.ssid);
		ret = wldm_SSID_SSID(CMD_SET, ap_index,
				map->vap_array[i].u.bss_info.ssid, &len, NULL, NULL);
		if (ret < 0) {
			HAL_WIFI_ERR(("%s: wldm_SSID_SSID CMM_SET Failed,"
				" ap_index=%d, ret = %d\n", __FUNCTION__, ap_index, ret));
			result = WIFI_HAL_ERROR;
		}

		ret = wldm_AccessPoint_SSIDAdvertisementEnabled(CMD_SET, ap_index,
			&(map->vap_array[i].u.bss_info.showSsid), NULL, NULL, NULL);
		if (ret < 0) {
			HAL_WIFI_ERR(("%s: wldm_AccessPoint_SSIDAdvertisementEnabled CMD_SET"
				" Failed, ap_index=%d, ret = %d\n", __FUNCTION__, ap_index, ret));
			result = WIFI_HAL_ERROR;
		}

		ret = wldm_AccessPoint_IsolationEnable(CMD_SET, ap_index,
			&(map->vap_array[i].u.bss_info.isolation), NULL, NULL, NULL);
		if (ret < 0) {
			HAL_WIFI_ERR(("%s: wldm_AccessPoint_IsolationEnable CMD_SET Failed"
				" ap_index=[%d]\n", __FUNCTION__, ap_index));
			result = WIFI_HAL_ERROR;
		}

		mgmtPowerControl = map->vap_array[i].u.bss_info.mgmtPowerControl;
		if (mgmtPowerControl < 0) mgmtPowerControl = -mgmtPowerControl;
		len = sizeof(mgmtPowerControl);
		ret = wldm_xbrcm_ap(CMD_SET_NVRAM, ap_index,
			&mgmtPowerControl, (uint *)&len,
			"bcnprs_txpwr_offset", NULL);
		if (ret < 0) {
			HAL_WIFI_ERR(("%s: wldm_xbrcm_ap CMD_SET_NVRAM for bcnprs_txpwr_offset"
				" for ap_index = %d failed\n", __FUNCTION__, ap_index));
			result = WIFI_HAL_ERROR;
		}

		if (map->vap_array[i].u.bss_info.bssMaxSta > 0) {
			bssMaxSta = map->vap_array[i].u.bss_info.bssMaxSta;
			ret = wldm_AccessPoint_MaxAssociatedDevices(CMD_SET, ap_index,
				&bssMaxSta, &len, NULL, NULL);
		}
		if (ret < 0) {
			HAL_WIFI_ERR(("%s: wldm_AccessPoint_MaxAssociatedDevices CMD_SET failed"
				"ap_index=[%d]\n", __FUNCTION__, ap_index));
			result = WIFI_HAL_ERROR;
		}

		ret = wldm_11v_btm(WLDM_BTM_SET_ACTIVATION, ap_index, NULL,
			&(map->vap_array[i].u.bss_info.bssTransitionActivated), NULL, 0, NULL);
		if (ret < 0) {
			HAL_WIFI_ERR(("%s: wldm_11v_btm WLDM_BTM_SET_ACTIVATION Failed,"
				" ap_index=[%d]\n", __FUNCTION__, ap_index));
			result = WIFI_HAL_ERROR;
		}
		/* Configure FT Security mode, TBD */

		len = DOT11_RRM_CAP_LEN;
		if (wldm_xbrcm_AccessPoint_RMCapabilities(CMD_GET_NVRAM, ap_index, rrm_val,
			&len, NULL, NULL) < 0) {
			HAL_WIFI_ERR(("%s: Error GET rmcap ap_index[%d]\n",
				__FUNCTION__, ap_index));
			result = WIFI_HAL_ERROR;
		}
		RRM_CAP_NEIGHBOR_REPORT_SET(map->vap_array[i].u.bss_info.nbrReportActivated, rrm_val[0]);
		len = DOT11_RRM_CAP_LEN;
		ret = wldm_xbrcm_AccessPoint_RMCapabilities(CMD_SET, ap_index, rrm_val, &len, NULL, NULL);
		if (ret < 0) {
			HAL_WIFI_ERR(("%s: Error setting NeighborReportActivation ap_index=[%d]\n",
				__FUNCTION__, ap_index));
			result = WIFI_HAL_ERROR;
		}

		ret = wifi_updateApSecurity(ap_index, &(map->vap_array[i].u.bss_info.security));
		if (ret != WIFI_HAL_SUCCESS) {
			HAL_WIFI_ERR(("%s: wifi_updateApSecurity failed, ap_index=[%d]\n",
				__FUNCTION__, ap_index));
			result = WIFI_HAL_ERROR;
		}

		ret = wifi_setApInterworking(ap_index, &(map->vap_array[i].u.bss_info.interworking));
		if (ret != WIFI_HAL_SUCCESS) {
			HAL_WIFI_ERR(("%s: wifi_setApInterworking failed, ap_index=[%d]\n",
				__FUNCTION__, ap_index));
			result = WIFI_HAL_ERROR;
		}

		if (map->vap_array[i].u.bss_info.mac_filter_enable) {
			if (map->vap_array[i].u.bss_info.mac_filter_mode ==
				wifi_mac_filter_mode_black_list) {
				enable = 1;
			} else {
				enable = 2;
			}
		} else {
			enable = 0;
		}
		ret = wldm_AccessPoint_MACAddressControMode(CMD_SET, ap_index,
			&enable, NULL, NULL, NULL);
		if (ret < 0) {
			HAL_WIFI_ERR(("%s: wldm_AccessPoint_MACAddressControMode failed"
				"ap_index=[%d]\n", __FUNCTION__, ap_index));
			result = WIFI_HAL_ERROR;
		}
		wpsSet = 1;
		ret = wifi_getApWpsConfiguration(ap_index, &wpsCfg);
		if ((ret == WIFI_HAL_SUCCESS) &&
			(wpsCfg.enable == map->vap_array[i].u.bss_info.wps.enable) &&
			!strncmp(map->vap_array[i].u.bss_info.wps.pin, wpsCfg.pin, sizeof(map->vap_array[i].u.bss_info.wps.pin)) &&
			(wpsCfg.methods == map->vap_array[i].u.bss_info.wps.methods)) {
			wpsSet = 0;
		}

		if (wpsSet) {
			ret = wifi_setApWpsConfiguration(ap_index, &(map->vap_array[i].u.bss_info.wps));
			if (ret != WIFI_HAL_SUCCESS) {
				HAL_WIFI_ERR(("%s: wifi_setApWpsConfiguration failed for ap_index=[%d]\n",
					__FUNCTION__, ap_index));
				result = WIFI_HAL_ERROR;
			}
		}

		ret = wldm_AccessPoint_WMMEnable(CMD_SET, ap_index,
			&(map->vap_array[i].u.bss_info.wmm_enabled), NULL, NULL, NULL);
		if (ret < 0) {
			HAL_WIFI_ERR(("%s: wldm_AccessPoint_WMMEnable CMD_SET failed for ap_index=[%d]\n",
				__FUNCTION__, ap_index));
			result = WIFI_HAL_ERROR;
		}

		len = sizeof(map->vap_array[i].u.bss_info.UAPSDEnabled);
                ret = wldm_AccessPoint_UAPSDEnable(CMD_SET, ap_index,
			&(map->vap_array[i].u.bss_info.UAPSDEnabled), &len, NULL, NULL);
		if (ret < 0) {
			HAL_WIFI_ERR(("%s wldm_AccessPoint_UAPSDEnable failed for ap_index=[%d]\n",
				__FUNCTION__, ap_index));
			result = WIFI_HAL_ERROR;
		}
	}
	if (result == WIFI_HAL_SUCCESS) {
		ret = wldm_apply(index, 0);
		if (ret < 0) {
			HAL_WIFI_ERR(("%s: wldm_apply() failed, radioIndex=[%d], ret=%d\n",
				__FUNCTION__, index, ret));
			result = WIFI_HAL_ERROR;
		}
	}

	/* update valid vap interface list */
	if (index == (unsigned int)(HAL_GET_MAX_RADIOS - 1)) {
		wldm_xbrcm_validate_bss_ifs();
	}

	if (validPrimaryStatus) {
		len = sizeof(pstPrimaryEnabled);
		if (wldm_AccessPoint_Enable(CMD_GET, priApIndex, &pstPrimaryEnabled, &len, NULL,
			NULL) < 0) {
			HAL_WIFI_ERR(("%s: post createVAP, get AP Enable Failed, priApIndex = %d\n",
				__FUNCTION__, priApIndex));
			result = WIFI_HAL_ERROR;
			validPrimaryStatus = FALSE;
		} else {
			HAL_WIFI_DBG(("%s: post createVAP priApIndex %d is %s\n", __FUNCTION__,
				priApIndex, pstPrimaryEnabled ? "Up" : "Down"));
		}
		/* If primary BSS is disabled and needs to be enabled, then acs_reinit radio */
		if (validPrimaryStatus && !prePrimaryEnabled && pstPrimaryEnabled) {
			/* Relaunch acsd2 to init specific disabled radio params in acsd2 */
			if (wldm_xbrcm_acs(CMD_SET_IOCTL, index, NULL, NULL, "acs_reinit", NULL)
				< 0) {
				HAL_WIFI_ERR(("%s: wldm_xbrcm_acs acs_reeinit CMD_SET_IOCTL "
					"Failed.\n", __FUNCTION__));
				result = WIFI_HAL_ERROR;
			}
		}
	}
	HAL_WIFI_DBG(("%s: radioIndex=[%d], result = %d\n", __FUNCTION__, index, result));
	return result;
}
#endif /* End of WIFI_HAL_VERSION_3 */

//Get the DfsMoveBackEnable status
INT wifi_getRadioDfsMoveBackEnable(INT radioIndex, BOOL *output_bool)
{
	int value, len = sizeof(value);

	RADIO_INDEX_ASSERT(radioIndex);

	if (wldm_xbrcm_acs(CMD_GET, radioIndex, &value, &len, "AcsDfsMoveBack", NULL) < 0) {
		HAL_WIFI_ERR(("%s: wldm_xbrcm_acs CMD_GET failed.\n", __FUNCTION__));
		return RETURN_ERR;
	}
	*output_bool = value ? 1 : 0;

	HAL_WIFI_DBG(("%s: radioIndex = %d, enable = %d\n", __FUNCTION__, radioIndex, *output_bool));

	return RETURN_OK;
}

//Set the DfsMoveBackEnable mode
INT wifi_setRadioDfsMoveBackEnable(INT radioIndex, BOOL enabled)
{
	int value, len = sizeof(value);

	HAL_WIFI_DBG(("%s: radioIndex = %d, enable = %d\n", __FUNCTION__, radioIndex, enabled));
	RADIO_INDEX_ASSERT(radioIndex);

	value = enabled ? 1 : 0;

	if (wldm_xbrcm_acs(CMD_SET_NVRAM | CMD_SET_IOCTL, radioIndex, &value, &len, "AcsDfsMoveBack", NULL) < 0) {
		HAL_WIFI_ERR(("%s: wldm_xbrcm_acs CMD_SET_IOCTL %d failed.\n", __FUNCTION__, enabled));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

#if defined(WIFI_HAL_VERSION_3)

void wifi_nvramCommit()
{
	if (system("nvram commit")) {
		HAL_WIFI_ERR(("%s: failed\n", __FUNCTION__));
	}
}

INT wifi_getBSSColor(INT radio_index, UCHAR *colorp)
{
	unsigned int color = 0;
	int ret, len = sizeof(color);

	HAL_WIFI_DBG(("%s: radio_index=%d\n", __FUNCTION__, radio_index));
	RADIO_INDEX_ASSERT(radio_index);
	NULL_PTR_ASSERT(colorp);

	*colorp = 0;
	ret = wldm_AXbssColor(CMD_GET, radio_index, &color, &len, NULL, NULL);
	if (ret == 0) {
		*colorp = color & 0xff;
	} else {
		HAL_WIFI_ERR(("%s: wldm_AXbssColor() failed\n", __FUNCTION__));
		return WIFI_HAL_ERROR;
	}
	HAL_WIFI_DBG(("%s: radio_index=%d Got color=%d ret=%d\n",
		__FUNCTION__, radio_index, *colorp, ret));

	return(ret);
}
#else
INT wifi_getBSSColor(INT apIndex, UCHAR *colorp)
{
	int ret;
	unsigned int radioIndex = HAL_AP_IDX_TO_HAL_RADIO(apIndex);

	HAL_WIFI_DBG(("%s\n", __FUNCTION__));

	AP_INDEX_ASSERT(apIndex);
	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(colorp);
	HAL_WIFI_DBG(("%s: apIndex=%d radioIndex=%d\n",
		__FUNCTION__, apIndex, radioIndex));

	*colorp = 0;
	unsigned int color = 0;
	int len = sizeof(color);
	ret = wldm_AXbssColor(CMD_GET, radioIndex, &color, &len, NULL, NULL);
	if (ret == 0) {
		*colorp = color & 0xff;
	} else {
		HAL_WIFI_ERR(("%s: wldm_AXbssColor() failed\n", __FUNCTION__));
		return RETURN_ERR;
	}
	HAL_WIFI_DBG(("%s: apIndex=%d radioIndex=%d Got color=%d ret=%d \n",
		__FUNCTION__, apIndex, radioIndex, *colorp, ret));

	return(ret);
}
#endif /* End of WIFI_HAL_VERSION_3 */

INT wifi_getBSSColorEnabled(INT apIndex, BOOL *enabled)
{
	int ret;
	unsigned int color = 0, radioIndex = HAL_AP_IDX_TO_HAL_RADIO(apIndex);

	HAL_WIFI_DBG(("%s: radioIndex=%d\n", __FUNCTION__, radioIndex));
	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(enabled);
	AX_CAPABLE_ASSERT(radioIndex);

	*enabled = FALSE;
	int len = sizeof(color);
	ret = wldm_AXbssColor(CMD_GET, radioIndex, &color, &len, NULL, NULL);
	if (ret == 0) {
		*enabled = (color >> 8) & 0x01 ? TRUE : FALSE;
	} else {
		HAL_WIFI_ERR(("%s: wldm_AXbssColor() failed\n", __FUNCTION__));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: apIndex=%d Got enabled=%s ret=%d \n",
		__FUNCTION__, apIndex, ((*enabled) ? "TRUE" : "FALSE" ), ret));

	return(ret);
}

INT wifi_getTWTParams(CHAR *sta, wifi_twt_params_t *twt_params)
{
	int ret = 0;

	UNUSED_PARAMETER(sta);
	UNUSED_PARAMETER(twt_params);

	HAL_WIFI_DBG(("HAL %s TBD\n",
		__FUNCTION__));
	return(ret);
}

INT wifi_get80211axDefaultParameters(INT apIndex, wifi_80211ax_params_t *params)
{
	int ret = 0;

	UNUSED_PARAMETER(apIndex);
	UNUSED_PARAMETER(params);

	HAL_WIFI_DBG(("HAL %s TBD\n",
		__FUNCTION__));
	return(ret);
}
/* 802.11ax HAL Imp. Ends */
#endif /* (WIFI_HAL_VERSION_GE_2_15) && (!defined(_CBR_PRODUCT_REQ_) && !defined(_XF3_PRODUCT_REQ_)) */

/* wifi_setRadioAutoChannelRefreshPeriod() function */
/**
* @brief Set the DCS refresh period in seconds.
*
* @param[in] radioIndex	Index of Wi-Fi radio channel
* @param[in] seconds	Set auto channel refresh period in seconds support for the selected radio channel.
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
INT wifi_setRadioAutoChannelRefreshPeriod(INT radioIndex, ULONG seconds)//Tr181
{
	HAL_WIFI_DBG(("%s: radioIndex %d, seconds %d\n", __FUNCTION__, radioIndex, seconds));
	RADIO_INDEX_ASSERT(radioIndex);

	int len = sizeof(seconds);
	unsigned int sec = (unsigned int) seconds;
	if (wldm_Radio_AutoChannelRefreshPeriod(CMD_SET, radioIndex, &sec, &len, NULL, NULL) < 0) {
		HAL_WIFI_ERR(("%s, wldm_Radio_AutoChannelRefreshPeriod CMD_SET failed\n", __FUNCTION__));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

#if WIFI_HAL_VERSION_GE_2_18 || defined(WIFI_HAL_VERSION_3)
INT wifi_getRadioChannels(INT radioIndex, wifi_channelMap_t *output_map, INT output_map_size)
{
	int ret, i;
	char channels[OUTPUT_STRING_LENGTH_512];
	int numChannels = 0, len = OUTPUT_STRING_LENGTH_512, channelMapSize;
	wldm_channelMap_t *wldm_channelMap_ptr;

	HAL_WIFI_DBG(("%s: Enter, radioIndex = %d\n", __FUNCTION__, radioIndex));

	NULL_PTR_ASSERT(output_map);
	RADIO_INDEX_ASSERT(radioIndex);

	ret = wldm_Radio_PossibleChannels(CMD_GET, radioIndex, channels, &len, NULL, NULL);
	if (ret < 0) {
		HAL_WIFI_ERR(("%s: wldm_Radio_PossibleChannels CMD_GET Failed, Status = %d\n",
			__FUNCTION__, ret));
		return RETURN_ERR;
	}
	numChannels = ret;
	if (numChannels > output_map_size) {
		HAL_WIFI_ERR(("%s: queried numChannels(%d) > output_map_size(%d)\n",
			__FUNCTION__, numChannels, output_map_size));
		return RETURN_ERR;
	}
	channelMapSize = numChannels * sizeof(wldm_channelMap_t);
	wldm_channelMap_ptr = (wldm_channelMap_t *)malloc(channelMapSize);
	if (wldm_channelMap_ptr == NULL) {
		HAL_WIFI_ERR(("%s: wldm_channel_map malloc failed\n", __FUNCTION__));
		return RETURN_ERR;
	}

	ret = wldm_xplume_opensync(CMD_GET, radioIndex, (void *)wldm_channelMap_ptr, (uint *)&numChannels,
		"channels_states", NULL);
	if (ret < 0) {
		HAL_WIFI_ERR(("%s: Error channels_states radioIndex=%d\n", __FUNCTION__, radioIndex));
		free(wldm_channelMap_ptr);
		return RETURN_ERR;
	}

	for (i = 0; i < numChannels; i++) {
		output_map[i].ch_number = (INT) wldm_channelMap_ptr[i].ch_number;
		output_map[i].ch_state = (wifi_channelState_t) wldm_channelMap_ptr[i].ch_state;
	}

	free(wldm_channelMap_ptr);
	return RETURN_OK;
}

#endif /* (WIFI_HAL_VERSION_GE_2_18) */
/*
*  Apply all new values to the nvram and drivers previously set by wifi_set_XXX APIs.
*  All set APIs shall verify and record the values, but new values will not take effect until wifi_apply is called.
*  The wifi_apply must be called once after all the wifi_set_XXX APIs are calls.
*/
INT wifi_apply(void)
{
	int ret;

	HAL_WIFI_LOG(("%s: calling wldm_apply_all()...\n", __FUNCTION__));
	ret = wldm_apply_all();
	HAL_WIFI_LOG(("%s: wldm_apply_all() done!\n", __FUNCTION__));
	return (ret < 0) ? RETURN_ERR : RETURN_OK;
}

#if !defined(_CBR1_PRODUCT_REQ_) && !defined(_SR300_PRODUCT_REQ_) && \
	!defined(_SR203_PRODUCT_REQ_)
#if WIFI_HAL_VERSION_GE_2_19
/* CSI functions */
#ifdef CMWIFI_RDKB_COMCAST
static wifi_csi_callback wifi_csi_cb = NULL;

/* get 32bit epoch time */
ULLONG get_epoch_ms()
{
	struct timespec ts_real;
	ULLONG time_epoch_ms;
	ULLONG ms;
	clock_gettime(CLOCK_REALTIME,  &ts_real);

	time_epoch_ms = (ULLONG)ts_real.tv_sec*1000;
	/* convert ns into ms, avoid divide operation
	* Note1: This will have a worse-case error of 0.076ms
	* [ (4295)/2^32 - 0.000001 ] * 999,999,999 = 0.0076ms
	* Note2: This calculation will take 29.9 + 12.1 = 42.0 bits.
	* Therefore, a 64-bit unsigned int is sufficent storage.
	*/
	ms = (ULLONG)( (ULLONG)ts_real.tv_nsec * (ULLONG)(4295));
	ms = ms >> 32;
	time_epoch_ms += ms;
	return time_epoch_ms;
}

/* convert CSI header format and call callback to forward csi data to wifiAgent */
static int hal_csi_rec_cb(unsigned int RadioIndex, char *raw_buf)
{
	wifi_csi_data_t *hal_csi_data;
	wifi_frame_info_t *hal_frame_info;
	bcm_csimon_hdr_t *hdr;
	bcm_csi_rpt_config_t *cfg;
	unsigned int *ptr = NULL;
	int n, nrow, tone_idx, nr_idx;
	mac_address_t mac;
	static unsigned int cnt = 0;

	NULL_PTR_ASSERT(raw_buf);
	cnt++;

	WL_CSI_INFO("Enter:%s cnt:%d\n", __FUNCTION__, cnt);

	hal_csi_data = (wifi_csi_data_t *)malloc(sizeof(wifi_csi_data_t));
	if (hal_csi_data == NULL) {
		WL_CSI_ERR("%s, malloc cli_CsiData fails\n", __FUNCTION__);
		return RETURN_ERR;
	}
	memset((char *)hal_csi_data, 0, sizeof(wifi_csi_data_t));

	hal_frame_info = &(hal_csi_data->frame_info);

	/* convert Brcm raw data to hal csi format */
	hdr = (bcm_csimon_hdr_t *)raw_buf;
	cfg = (bcm_csi_rpt_config_t *)(hdr + 1);

	memcpy(mac, hdr->data.client_ea, ETHER_ADDR_LEN);
	if (WL_CSI_VERBOSE_ENABLED) {
		WL_CSI_PRT("%s Header:\n", __FUNCTION__);
		WL_CSI_PRT("frameid:%x\n", hdr->data.format_id);
		WL_CSI_PRT("Client-ea:"MACF"\n", MAC_TO_MACF(hdr->data.client_ea));
		WL_CSI_PRT("bss-ea:"MACF"\n", MAC_TO_MACF(hdr->data.bss_ea));
		WL_CSI_PRT("chaspec:%04X chan:%d\n", hdr->data.chanspec,
			wf_chspec_ctlchan(hdr->data.chanspec));
		WL_CSI_PRT("tx:%X\n", hdr->data.txstreams);
		WL_CSI_PRT("rx:%X\n", hdr->data.rxstreams);
		WL_CSI_PRT("report_ts:%x\n", hdr->data.report_ts);
		WL_CSI_PRT("assoc_ts:%x\n", hdr->data.assoc_ts);
		WL_CSI_PRT("rssi:%02x:%02x:%02x:%02x\n", hdr->data.rssi[0], hdr->data.rssi[1],
			hdr->data.rssi[2], hdr->data.rssi[3]);
#if defined(BCM_CSIMON_VER) && (BCM_CSIMON_VER > 0)
		WL_CSI_PRT("cfo:%x\n", hdr->data.cfo);
#endif /* defined(BCM_CSIMON_VER) && (BCM_CSIMON_VER > 0) */
		WL_CSI_PRT("time_stamp:0x%llx\n", (ULLONG)hdr->data.report_ts);
	}

	/* fillout HAL csi header */
	hal_frame_info->phy_bw = 20 * (1 << CSI_EXTRACT_CONFIG_WORD(cfg->config, PHYBW));
	hal_frame_info->cap_bw = 20 * (1 << CSI_EXTRACT_CONFIG_WORD(cfg->config, PKTBW));
	hal_frame_info->bw_mode = 1 << (CSI_EXTRACT_CONFIG_WORD(cfg->config, PKTBW));
	hal_frame_info->Nr = CSI_EXTRACT_CONFIG_WORD(cfg->config, NTX) + 1;
	/* Brcm only support 1 stream */
	hal_frame_info->Nc = 1;
	hal_frame_info->num_sc = CSI_EXTRACT_CONFIG_WORD_SPILL(cfg->config, NUM_TONES);
	/* Not applied to Brcm CSI format */
	hal_frame_info->valid_mask = 0xffff;
	/* Brcm only support decimal 0 */
	hal_frame_info->decimation = 0;
	hal_frame_info->channel = wf_chspec_ctlchan(hdr->data.chanspec);

#if defined(BCM_CSIMON_VER) && (BCM_CSIMON_VER > 0)
	hal_frame_info->cfo = (INT)(hdr->data.cfo);
#endif /* defined(BCM_CSIMON_VER) && (BCM_CSIMON_VER > 0) */

	hal_frame_info->time_stamp = get_epoch_ms();
	for (n = 0; n < 4; n++) {
		hal_frame_info->nr_rssi[n] = hdr->data.rssi[n];
	}

	if (hal_frame_info->num_sc > MAX_SUB_CARRIERS) {
		WL_CSI_ERR("%s: num_sc[%d] too big \n", __FUNCTION__, hal_frame_info->num_sc);

		/* free csi buffer */
		free(hal_csi_data);
		return RETURN_ERR;
	}

	WL_CSI_VERBOSE("frame_info: phy_bw:%x cap_bw:%x Nr:%x Nc:%x num_sc:%x cfo:%x\n",
		hal_frame_info->phy_bw, hal_frame_info->cap_bw, hal_frame_info->Nr,
		hal_frame_info->Nc, hal_frame_info->num_sc, hal_frame_info->cfo);

	/* Brcm csi data format (32bits):
	 * Nr:1:  V0[0] V1[0] ... Vn[0]
	 * Nr:2:  V0[0] V0[1] V1[0] v1[1] ...Vn[0]Vn[1]
	 * Nr:3:  V0[0] V0[1] V0[2] [zero] V1[0] v1[1] V1[2] [zero] ...Vn[0]Vn[1]vn[2] [zero]
	 * Nr:4:  V0[0] V0[1] V0[2] V0[3] V1[0] v1[1] V1[2] V1[3] ...Vn[0]Vn[1]vn[2]Vn[3]
	*/
	nrow = hal_frame_info->Nr;
	if (nrow == 3)
		nrow = 4;

	ptr = (unsigned int *)(cfg + 1);
	for (tone_idx = 0; tone_idx < hal_frame_info->num_sc; tone_idx++) {
		for (nr_idx = 0; nr_idx < nrow; nr_idx++) {
			/* brcm csi only support 1 stream, the 3rd index (Nc) is always 0 */
			hal_csi_data->csi_matrix[tone_idx][nr_idx][0] =
				ptr[nr_idx + nrow * tone_idx];
		}
	}

	/* callback to forward csi data to wifiAgent */
	if (wifi_csi_cb) {
		wifi_csi_cb(mac, hal_csi_data);
	}
	else {
		WL_CSI_ERR("%s: wifi csi call back is NULL\n", __FUNCTION__);
	}

	/* free csi buffer */
	free(hal_csi_data);

	return RETURN_OK;
}

void wifi_csi_callback_register(wifi_csi_callback callback_proc)
{
	WL_CSI_INFO("%s: %p\n", __FUNCTION__, callback_proc);
	if (callback_proc) {
		wifi_csi_cb = callback_proc;
		wl_csi_register(hal_csi_rec_cb);
		WL_CSI_DBG("%s: callback_proc registered\n", __FUNCTION__);
		return;
	}
	WL_CSI_ERR("%s: callback_proc is NULL\n", __FUNCTION__);
	return;
}
#endif /* CMWIFI_RDKB_COMCAST */

#ifndef _SR300_PRODUCT_REQ_
/* Enable/Disable CSI Engine function */
INT wifi_enableCSIEngine(INT apIndex, mac_address_t sta, BOOL enable)
{
#ifdef CMWIFI_RDKB_COMCAST
	int ret = 0;
	char stop_csithread = 0;
	char BCAST_MAC[ETHER_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	unsigned int radioIndex;

	NULL_PTR_ASSERT(sta);
	AP_INDEX_ASSERT(apIndex);
	radioIndex = HAL_AP_IDX_TO_HAL_RADIO(apIndex);

	WL_CSI_INFO("Start: %s wl:%d Mac:"MACF " enab:%d\n", __FUNCTION__,
		radioIndex, MAC_TO_MACF(sta), enable);

	if (ETHER_ISNULLADDR(sta)) {
		if (enable) {
			WL_CSI_ERR("%s: failure\n", __FUNCTION__);
			return RETURN_ERR;
		}
		memcpy((char *)sta, BCAST_MAC, ETHER_ADDR_LEN);
		stop_csithread = 1;
	}

	/* Start csi collection */
	if (enable) {
		ret = wl_csi_start(radioIndex);
		WL_CSI_DBG("start csi thread. ret:%d\n", ret);
	}

	ret = wl_set_csimon(radioIndex, (unsigned char *)sta, enable, CSI_DELAY_PERIOD);
	WL_CSI_DBG("%s wl:%d ret:%d\n", __FUNCTION__, radioIndex, ret);

	if (ret) {
		WL_CSI_ERR("%s: fails:%d\n", __FUNCTION__, ret);
		return RETURN_ERR;
	}

	if (!enable) {
		if (stop_csithread) {
			ret = wl_csi_stop(radioIndex);
			WL_CSI_DBG("stop csi thread. ret:%d\n", ret);
		}
	}
#else
	UNUSED_PARAMETER(apIndex);
	UNUSED_PARAMETER(sta);
	UNUSED_PARAMETER(enable);
#endif /* CMWIFI_RDKB_COMCAST */

	return RETURN_OK;
}
#endif /* _SR300_PRODUCT_REQ_ */
INT wifi_sendDataFrame(INT apIndex, mac_address_t sta, UCHAR *data, UINT len,
	BOOL insert_llc, UINT eth_proto, wifi_data_priority_t prio)
{
	UNUSED_PARAMETER(apIndex);
	UNUSED_PARAMETER(sta);
	UNUSED_PARAMETER(data);
	UNUSED_PARAMETER(len);
	UNUSED_PARAMETER(insert_llc);
	UNUSED_PARAMETER(eth_proto);
	UNUSED_PARAMETER(prio);

	/* Stub function. Brcm current implementation doesnot send data packet to measure CSI. */
	return RETURN_OK;
}
#else
INT wifi_enableCSIEngine(INT apIndex, mac_address_t sta, BOOL enable)
{
	UNUSED_PARAMETER(apIndex);
	UNUSED_PARAMETER(sta);
	UNUSED_PARAMETER(enable);

	HAL_WIFI_ERR(("Not Supported\n"));
	return RETURN_ERR;
}

INT wifi_sendDataFrame(INT apIndex, mac_address_t sta, UCHAR *data, UINT len,
	BOOL insert_llc, UINT eth_proto, wifi_data_priority_t prio)
{
	UNUSED_PARAMETER(apIndex);
	UNUSED_PARAMETER(len);
	UNUSED_PARAMETER(insert_llc);
	UNUSED_PARAMETER(eth_proto);
	UNUSED_PARAMETER(prio);

	HAL_WIFI_ERR(("Not Supported\n"));
	return RETURN_ERR;
}
#endif /* WIFI_HAL_VERSION_GE_2_19 */
#endif/* !defined(_CBR1_PRODUCT_REQ_)  && !defined(_SR300_PRODUCT_REQ_) && !defined(_SR203_PRODUCT_REQ_)*/
#if WIFI_HAL_VERSION_GE_2_19
/**
 * Until apIndex is provided as a parameter, hardcode to set to all hotspot
 * vaps, i.e., vap 4, 5, 8 and 9.
 */
INT wifi_enableGreylistAccessControl(BOOL enable)
{
	INT hotspot_index[] = {4, 5, 8, 9};
	INT apIndex;
	UINT i;
	BOOL cur_enable, changed = FALSE;
	HAL_WIFI_LOG(("%s: enable=%u\n",  __FUNCTION__, enable ? 1 : 0));

	for (i = 0; i < ARRAY_SIZE(hotspot_index); i++) {
		apIndex = hotspot_index[i];
		if (wldm_AccessPoint_Security_RadiusGreylist(CMD_GET, apIndex,
			&cur_enable, NULL, NULL, NULL) != 0 ) {
			HAL_WIFI_ERR(("%s: %d: "
					"wldm_AccessPoint_Security_RadiusGreylist "
					"CMD_GET failed\n",
					__FUNCTION__, apIndex));
			return RETURN_ERR;
		}

		if (cur_enable != enable) {
			if (wldm_AccessPoint_Security_RadiusGreylist(CMD_SET,
				apIndex, &enable, NULL, NULL, NULL) != 0) {
				HAL_WIFI_ERR(("%s: %d: "
						"wldm_AccessPoint_Security_RadiusGreylist "
						"CMD_SET failed\n",
						__FUNCTION__, apIndex));
				return RETURN_ERR;
			}
			changed = TRUE;
		}
	}

	if (changed) {
		/* apply immediately */
		if (wifi_apply()) {
			HAL_WIFI_LOG(("%s: Fail to apply\n",  __FUNCTION__));
			return RETURN_ERR;
		}
	}

	return RETURN_OK;
}

INT wifi_getApDASRadiusServer(INT apIndex, CHAR *IP_output, UINT *Port_output, CHAR *RadiusdasSecret_output)
{
	int len;

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(IP_output);
	NULL_PTR_ASSERT(Port_output);
	NULL_PTR_ASSERT(RadiusdasSecret_output);

	if (wldm_AccessPoint_Security_RadiusDASPort(CMD_GET, apIndex, Port_output, NULL, NULL, NULL)) {
		HAL_WIFI_ERR(("%s: %d: wldm_AccessPoint_Security_RadiusDASPort CMD_GET failed\n",
			__FUNCTION__, apIndex));
		return RETURN_ERR;
	}

	len = OUTPUT_STRING_LENGTH_64;
	if (wldm_AccessPoint_Security_RadiusDASClientIPAddr(CMD_GET, apIndex, IP_output, &len, NULL, NULL)) {
		HAL_WIFI_ERR(("%s: %d: wldm_AccessPoint_Security_RadiusDASClientIPAddr CMD_SET failed\n",
			__FUNCTION__, apIndex));
		return RETURN_ERR;
	}

	len = OUTPUT_STRING_LENGTH_64;
	/* When DAS secret is null, get fails, it is not an error. */
	if (wldm_AccessPoint_Security_RadiusDASSecret(CMD_GET, apIndex, RadiusdasSecret_output, &len, NULL, NULL))
		RadiusdasSecret_output[0] = '\0';

	return RETURN_OK;
}

/**
 * Radius DAS Secret is not applicable for open ssid,
 * @param[in] RadiusdasSecret could be NULL.
 *
 */
INT wifi_setApDASRadiusServer(INT apIndex, CHAR *IPAddress, UINT port, CHAR *RadiusdasSecret)
{
	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(IPAddress);

	HAL_WIFI_LOG(("%s: %d: IPAddress=%s, port=%u, RadiusSecret=%s\n",
			__FUNCTION__, apIndex, IPAddress, port,
			RadiusdasSecret ? RadiusdasSecret : "null"));

	if (wldm_AccessPoint_Security_RadiusDASPort(CMD_SET, apIndex, &port, NULL, NULL, NULL)) {
		HAL_WIFI_ERR(("%s: %d: wldm_AccessPoint_Security_RadiusDASPort CMD_SET failed\n",
			__FUNCTION__, apIndex));
		return RETURN_ERR;
	}

	if (wldm_AccessPoint_Security_RadiusDASClientIPAddr(CMD_SET, apIndex, IPAddress, NULL, NULL, NULL)) {
		HAL_WIFI_ERR(("%s: %d: wldm_AccessPoint_Security_RadiusDASClientIPAddr CMD_SET failed\n",
			__FUNCTION__, apIndex));
		return RETURN_ERR;
	}

	if (RadiusdasSecret &&
		wldm_AccessPoint_Security_RadiusDASSecret(CMD_SET, apIndex, RadiusdasSecret, NULL, NULL, NULL)) {
		HAL_WIFI_ERR(("%s: %d: wldm_AccessPoint_Security_RadiusDASPort CMD_SET failed\n",
			__FUNCTION__, apIndex));
		return RETURN_ERR;
	}

	return RETURN_OK;
}
#endif /* WIFI_HAL_VERSION_GE_2_19 */

/*-------------------------*/
#if defined(WIFI_HAL_VERSION_3)

static int
wl_map_str2enum_fromTable(char *istr, unsigned int *ivar, char *delim,
	struct wifi_enum_to_str_map *aTable)
{
	char *tmp_str, *tok, *rest = NULL;
	int i;

	if ((istr == NULL) || (ivar == NULL) || (delim == NULL) || (aTable == NULL)) {
		HAL_WIFI_ERR(("%s, Error Input param\n", __FUNCTION__));
		return RETURN_ERR;
	}

	tmp_str = strdup(istr);
	if (tmp_str == NULL) {
		HAL_WIFI_ERR(("%s, Error strdup\n", __FUNCTION__));
		return RETURN_ERR;
	}
	*ivar = 0;
	tok = strtok_r(tmp_str, delim, &rest);
	while (tok) {
		for (i = 0; aTable[i].str_val != NULL; i++) {
			if (!(strcmp(aTable[i].str_val, tok))) {
				/* match */
				*ivar |= aTable[i].enum_val;
				break;
			}
		}
		tok = strtok_r(NULL, delim, &rest);
	}
	free(tmp_str);

	return RETURN_OK;
}

static int
wl_str2uintArray(char *istr, char *delim, unsigned int *count,
	unsigned int *uarray, unsigned int maxCount)
{
	char *tmp_str, *tok, *rest = NULL;
	unsigned int i = 0;

	if ((istr == NULL) || (delim == NULL) || (count == NULL) || (uarray == NULL)) {
		HAL_WIFI_ERR(("%s, Error Input param\n", __FUNCTION__));
		return RETURN_ERR;
	}
	tmp_str = strdup(istr);
	if (tmp_str == NULL) {
		HAL_WIFI_ERR(("%s, Error strdup\n", __FUNCTION__));
		return RETURN_ERR;
	}
	tok = strtok_r(tmp_str, delim, &rest);
	while (tok && (i < maxCount)) {
		uarray[i] = atoi(tok);
		i++;
		tok = strtok_r(NULL, delim, &rest);
	}
	free(tmp_str);
	*count = i;
	return RETURN_OK;
}

#define CSI_MAX_DEVICES_BRCM 10
static int
getNumSetBits(int inVal)
{
	int cnt = 0, num = inVal;

	while (num > 0) {
		num &= (num -1);
		cnt ++;
	}
	return cnt;
}

#if defined(WIFI_HAL_VERSION_GE_3_0_1)

/* phy_index is the attribute for Wiphy index with 802.11 netlink cmds, NL80211_ATTR_WIPHY
 * It is the phy# in
 *   iw phy |egrep 'Band|Wiphy' or iw dev wl0 info |egrep 'Interface|wiphy'
 * phy_index matches the pcie port# for wl0, wl1 and wl2
 * Default is <phy0, pcie0, wl0, 2G> <phy1, pcie1, wl1, 5G> <phy2, pcie2, wl2, 6G>
 * If not using the default, we use
 *   wl_radio_to_ifid_map in nvram to map the pcie port#
 *   /data/board_hw_nvram file with the device ids as wlX_deviceid=<device_id>
 */
static unsigned int
wl_getPhyIndex(int radioIndex)
{
	char *nvStr = nvram_get("wl_radio_to_ifid_map");
	unsigned int phy_index, val;
	if (nvStr) {
		/* example wl_radio_to_ifid_map = 0x201 - maps to pcie port#
		 * radio0 phy_index is 1; radio1 phy_index is 0
		 * radio2 phy_index is 2 */
		val = (unsigned int)strtoul(nvStr, NULL, 0);
		phy_index = (val >> (radioIndex * 4)) & (0x0F);
	}
	else {
		phy_index = radioIndex;
	}
	return phy_index;
}

static int
wl_getWifiInterfaceMap(wifi_interface_name_idex_map_t *imap, int radios_cnt)
{
	wifi_interface_name_idex_map_t *iptr = imap;
	int i, blen, ilen, apIndex;
	UNUSED_PARAMETER(radios_cnt);

	/* Max number of entries is MAX_NUM_RADIOS * MAX_NUM_VAP_PER_RADIO */
	for (i = 0; i < (MAX_NUM_VAP_PER_RADIO + 1); i++) {
		char lname[32], lnames[32], *iname, *inext, *s, *brlanStr, *ifStr, *tmpStr;
		if (i > 0) {
			snprintf(lname, sizeof(lname), "lan%d_ifname", i);
			snprintf(lnames, sizeof(lnames), "lan%d_ifnames", i);
		}
		else {
			snprintf(lname, sizeof(lname), "lan_ifname");
			snprintf(lnames, sizeof(lnames), "lan_ifnames");
		}
		/* examples
		 * lan_ifname=brlan0   lan_ifnames=wl0 wl1 wl2
		 * lan1_ifname=brlan1  lan1_ifnames=wl0.1 wl1.1 wl2.1
		 * lan4_ifname=br1an4   lan4_ifnames=wl0.3 wl0.5
		 */
		brlanStr = nvram_get(lname);
		if (brlanStr == NULL) continue;
		blen = strlen(brlanStr);
		ifStr = nvram_get(lnames);
		if (ifStr == NULL) continue;

		ilen = strlen(ifStr)+1;
		tmpStr = malloc(ilen);  //(8 * MAX_NUM_VAP_PER_RADIO * MAX_NUM_RADIOS);
		if (tmpStr == NULL) {
			HAL_WIFI_ERR(("%s, malloc tmpStr len %d failed\n", __FUNCTION__, ilen));
			return WIFI_HAL_ERROR;
		}
		memcpy(tmpStr, ifStr, ilen);
		iname = strtok_r(tmpStr, " ", &inext);
		while (iname) {
			if ((s = strchr(iname, ' ')) != NULL) {
				*s = '\0';
			}
			/* get interface map info for each wlx.y in use */
			if (strstr(iname, "wl")) {
				ilen = strlen(iname);
				apIndex = wldm_get_apindex(iname);
				if (apIndex >=  0) {
					iptr->rdk_radio_index = wldm_get_radioIndex(apIndex);
					iptr->phy_index = wl_getPhyIndex(iptr->rdk_radio_index);
					memcpy(iptr->interface_name, iname, (ilen + 1));
					memcpy(iptr->bridge_name, brlanStr, (blen+1));
					memcpy(iptr->vap_name, iname, (ilen + 1)); /* same as interface_name now */
					iptr->index = apIndex;
					iptr->vlan_id = 0; /* TBD vlan_util or brctl cmds don't seem to work */
					iptr++;
				}
			}
			iname = strtok_r(NULL, " ", &inext);
		} /* while */
		free(tmpStr);
	}
	return WIFI_HAL_SUCCESS;
}

#if defined(RDKB_LGI)
static int
wl_getRadioInterfaceMap(int radioIndex, radio_interface_mapping_t *rmap)
{
	rmap->phy_index = wl_getPhyIndex(radioIndex);
	rmap->radio_index = radioIndex;
	snprintf(rmap->radio_name, sizeof(rmap->radio_name),
		wldm_get_radio_nvifname(radioIndex));
	snprintf(rmap->interface_name, sizeof(rmap->interface_name),
		wldm_get_radio_osifname(radioIndex));
	return WIFI_HAL_SUCCESS;
}
#endif /* RDKB_LGI */
#endif /* WIFI_HAL_VERSION_GE_3_0_1 */

static int
wl_getRadioCapabilities(int radioIndex, wifi_radio_capabilities_t *rcap)
{
	wldm_freq_bands_t bands = {0};
	char xpwrSupStr[OUTPUT_STRING_LENGTH_256], encModeStr[OUTPUT_STRING_LENGTH_256],
		*abbrev, cbuf[OUTPUT_STRING_LENGTH_1024];
	int ccnt, len, ret, bandis2g, bcnt, acnt;
	uint32 possibleChannels[MAX_CHANNELS + 1] = {0};
	wldm_uint32_list_t *list;
	wifi_channels_list_t *chlistp, *bchlistp, *achlistp;
	wl_country_list_t *cl = (wl_country_list_t *)cbuf;
	wifi_enum_to_str_map_t *cctentry;
	unsigned int cipherSupported, numberOfElements = 0;
	unsigned int transmitPowerSupported[MAXNUMBEROFTRANSMIPOWERSUPPORTED];
	unsigned int j, i, numbands;

	NULL_PTR_ASSERT(rcap);
	RADIO_INDEX_ASSERT(radioIndex);

	rcap->index = radioIndex;
	snprintf(rcap->ifaceName, sizeof(rcap->ifaceName), "%s", wldm_get_radio_nvifname(radioIndex));

	len = sizeof(bands);
	ret = wldm_xbrcm_radio(CMD_GET, radioIndex, (void *)(&(bands)), (uint *)&len, "bands", NULL);
	if (ret < 0) {
		HAL_WIFI_ERR(("%s: Error wldm_xbrcm_radio bands idx=%d\n",
		__FUNCTION__, radioIndex));
		return WIFI_HAL_ERROR;
	}

	/* numSupportedFreqBand - if dual band is 2G and 5G capable */
	rcap->numSupportedFreqBand = getNumSetBits((int)bands);
	numbands = rcap->numSupportedFreqBand;

	/* band - if dual band, get current band into band[0] */
	len = sizeof(rcap->band[0]);
	ret = wldm_xbrcm_radio(CMD_GET, radioIndex, (void *)(&(rcap->band[0])), (uint *)&len, "band", NULL);
	if (ret < 0) {
		HAL_WIFI_ERR(("%s: Error wldm_xbrcm_radio band idx=%d\n",
			__FUNCTION__, radioIndex));
		return WIFI_HAL_ERROR;
	}

	/* channel_list - gets all for dualband as well */
	list = (wldm_uint32_list_t *)possibleChannels;
	len = sizeof(possibleChannels);
	ret = wldm_xbrcm_radio(CMD_GET, radioIndex, (void *)list, (uint *)&len, "possibleChannels", NULL);
	if (ret < 0) {
		HAL_WIFI_ERR(("%s: Error wldm_xbrcm_radio possibleChannels idx=%d\n",
			__FUNCTION__, radioIndex));
		return WIFI_HAL_ERROR;
	}

	if (numbands == 1) {
		chlistp = &(rcap->channel_list[0]);
		chlistp->num_channels = (int)(list->count);
		for (j = 0; j < list->count; j++) {
			chlistp->channels_list[j] = (int)(list->element[j]);
		}
	}
	else { /* dual band */
		/* currently only do dualband as 2G and 5G */
		bandis2g = (rcap->band[0] == WIFI_FREQUENCY_2_4_BAND) ? TRUE : FALSE;
		if (bandis2g) {
			rcap->band[1] = WIFI_FREQUENCY_5_BAND;
			bchlistp = &(rcap->channel_list[0]); /* 2G - b band */
			achlistp = &(rcap->channel_list[1]); /* 5G - a band */
		}
		else {
			rcap->band[1] = WIFI_FREQUENCY_2_4_BAND;
			achlistp = &(rcap->channel_list[0]);
			bchlistp = &(rcap->channel_list[1]);
		}
		for (j = 0, acnt = 0, bcnt = 0; j < list->count; j++) {
			/* separate channels by band */
			if (bandis2g && (CHANNEL_IS_2G(list->element[j]))) {
				bchlistp->channels_list[bcnt] = (int)(list->element[j]);
				bcnt++;
			}
			else {
				achlistp->channels_list[acnt] = (int)(list->element[j]);
				acnt++;
			}
		}
		bchlistp->num_channels = (int)(bcnt);
		achlistp->num_channels = (int)(acnt);
	}

	HAL_WIFI_DBG(("%s bands=0x%x rcap->band[0]=0x%x rcap->band[1]=0x%x numbands=%d\n",
		__FUNCTION__, bands, rcap->band[0], rcap->band[1], numbands));
	for (i = 0; i < numbands; i++) {
		/* channelWidth - all supported bandwidths */
		rcap->channelWidth[i] = 0;
		if (rcap->band[i] & WIFI_FREQUENCY_2_4_BAND) {
			rcap->channelWidth[i] |= (WIFI_CHANNELBANDWIDTH_20MHZ |
				WIFI_CHANNELBANDWIDTH_40MHZ);
		}
		else if (rcap->band[i] & (WIFI_FREQUENCY_5_BAND | WIFI_FREQUENCY_6_BAND)) {
			rcap->channelWidth[i] |= (WIFI_CHANNELBANDWIDTH_20MHZ |
				WIFI_CHANNELBANDWIDTH_40MHZ |
				WIFI_CHANNELBANDWIDTH_80MHZ | WIFI_CHANNELBANDWIDTH_160MHZ);
#ifdef WIFI_HAL_VERSION_GE_3_0_3
			rcap->channelWidth[i] |= WIFI_CHANNELBANDWIDTH_320MHZ;
#endif /* WIFI_HAL_VERSION_GE_3_0_3 */
		}

		/* mode - all supported variants */
		rcap->mode[i] = WIFI_80211_VARIANT_H;
		if (rcap->band[i] & WIFI_FREQUENCY_2_4_BAND) {
			rcap->mode[i] |= (WIFI_80211_VARIANT_G | WIFI_80211_VARIANT_N |
				WIFI_80211_VARIANT_AX);
#ifdef WIFI_HAL_VERSION_GE_3_0_3
			rcap->mode[i] |= WIFI_80211_VARIANT_BE;
#endif /* WIFI_HAL_VERSION_GE_3_0_3 */
		}
		else if (rcap->band[i] & WIFI_FREQUENCY_5_BAND) {
			rcap->mode[i] |= (WIFI_80211_VARIANT_A | WIFI_80211_VARIANT_N |
				WIFI_80211_VARIANT_AC | WIFI_80211_VARIANT_AX);
#ifdef WIFI_HAL_VERSION_GE_3_0_3
			rcap->mode[i] |= WIFI_80211_VARIANT_BE;
#endif /* WIFI_HAL_VERSION_GE_3_0_3 */
		}
		else if (rcap->band[i] & (WIFI_FREQUENCY_6_BAND)) {
			rcap->mode[i] |= WIFI_80211_VARIANT_AX;
#ifdef WIFI_HAL_VERSION_GE_3_0_3
			rcap->mode[i] |= WIFI_80211_VARIANT_BE;
#endif /* WIFI_HAL_VERSION_GE_3_0_3 */
		}

		/* maxBitRate - from mcs_rate_tbl, pick max values for each band
		 *		Standard,	BW,	GI,	NSS,	Rate
		 * 2.4G:	{"ax",		"40",	1,	4,	1147},
		 * 5G:		{"ax",		"160",	1,	4,	4804},
		 * 6G:		{"ax",		"160",	1,	8,	9608},
		 */
		rcap->maxBitRate[i] = (rcap->band[i] & WIFI_FREQUENCY_2_4_BAND) ? 1147 :
			((rcap->band[i] & WIFI_FREQUENCY_5_BAND) ? 4804 :
			((rcap->band[i] & (WIFI_FREQUENCY_6_BAND) ? 9608 : 0)));

		/* supportedBitRate - all supported bitrates */
		rcap->supportedBitRate[i] = 0;
		if (rcap->band[i] & WIFI_FREQUENCY_2_4_BAND) {
			rcap->supportedBitRate[i] |= (WIFI_BITRATE_1MBPS | WIFI_BITRATE_2MBPS |
				WIFI_BITRATE_5_5MBPS | WIFI_BITRATE_6MBPS | WIFI_BITRATE_9MBPS |
				WIFI_BITRATE_11MBPS | WIFI_BITRATE_12MBPS);
		}
		else if (rcap->band[i] & (WIFI_FREQUENCY_5_BAND | WIFI_FREQUENCY_6_BAND)) {
			rcap->supportedBitRate[i] |= (WIFI_BITRATE_6MBPS | WIFI_BITRATE_9MBPS |
				WIFI_BITRATE_12MBPS | WIFI_BITRATE_18MBPS | WIFI_BITRATE_24MBPS |
				WIFI_BITRATE_36MBPS | WIFI_BITRATE_48MBPS | WIFI_BITRATE_54MBPS);
		}

		/* transmitPowerSupported_list - refer wifi_getRadioTransmitPowerSupported */
		len = sizeof(xpwrSupStr);
		ret = wldm_Radio_TransmitPowerSupported(CMD_GET, radioIndex,
			xpwrSupStr, &len, NULL, NULL);
		if (ret < 0) {
			HAL_WIFI_ERR(("%s: Error wldm_Radio_TransmitPowerSupported GET idx=%d\n",
				__FUNCTION__, radioIndex));
			return WIFI_HAL_ERROR;
		}
		/* Allow comma or space delimiters in string */
		wl_str2uintArray(xpwrSupStr, ", ", &numberOfElements, transmitPowerSupported,
			MAXNUMBEROFTRANSMIPOWERSUPPORTED);
		rcap->transmitPowerSupported_list[i].numberOfElements = numberOfElements;
		for (j = 0; j < numberOfElements; j++)
			rcap->transmitPowerSupported_list[i].transmitPowerSupported[j] = transmitPowerSupported[j];
	} /* for numbands */

	/* autoChannelSupported */
	/* always ON with wifi_getRadioAutoChannelSupported */
	rcap->autoChannelSupported = TRUE;

	/* DCSSupported */
	/* always ON with wifi_getRadioDCSSupported */
	rcap->DCSSupported = TRUE;

	/* zeroDFSSupported - TBD */
	rcap->zeroDFSSupported = FALSE;

	/* csi */
	rcap->csi.maxDevices = CSI_MAX_DEVICES_BRCM;
	rcap->csi.soudingFrameSupported = TRUE;

	snprintf(rcap->ifaceName, MAXIFACENAMESIZE, "%s", wldm_get_radio_osifname(radioIndex));

	/* cipher */
	len = sizeof(encModeStr);
	ret = wldm_AccessPoint_Security_EncryptionModesSupported(CMD_GET, radioIndex, encModeStr,
		&len, NULL, NULL);
	if (ret < 0) {
		HAL_WIFI_ERR(("%s: Error wldm_AccessPoint_Security_EncryptionModesSupported %d\n",
			__FUNCTION__, radioIndex));
		return WIFI_HAL_ERROR;
	}
	cipherSupported = rcap->cipherSupported;
	wl_map_str2enum_fromTable(encModeStr, &cipherSupported,
		", ", cipher_cap_table);

	len = sizeof(cbuf);
	ret = wldm_xbrcm_radio(CMD_GET, radioIndex, (void *)(&(cbuf)), (uint *)&len, "countryList", NULL);
	if (ret < 0) {
		HAL_WIFI_ERR(("%s: Error wldm_xbrcm_radio countryList idx=%d\n",
		__FUNCTION__, radioIndex));
		return WIFI_HAL_ERROR;
	}
	cl = (wl_country_list_t *)cbuf;

	ccnt = 0;
	for (i = 0; i < dtoh32(cl->count); i++) {
		abbrev = &cl->country_abbrev[i*WLC_CNTRY_BUF_SZ];
		for (j = 0; j < wifi_countrycode_max; j++) {
			cctentry = &(countrycode_table[j]);
			if ((!strncmp(cctentry->str_val, abbrev, strlen(abbrev))) &&
				(strlen(abbrev) == strlen(cctentry->str_val))) {
				/* match */
				rcap->countrySupported[ccnt] = cctentry->enum_val;
				ccnt++;
				break;
			}
		} /* for j */
	}
	rcap->numcountrySupported = ccnt;

	rcap->maxNumberVAPs = MAX_NUM_VAP_PER_RADIO;

#if defined(WIFI_HAL_VERSION_GE_3_0_1)
	rcap->mcast2ucastSupported = TRUE;
#endif /* WIFI_HAL_VERSION_GE_3_0_1 */

	return WIFI_HAL_SUCCESS;
}

#ifdef WIFI_HAL_VERSION_GE_3_0_3
static int
wl_getMLOCapabilities(wifi_multi_link_bands_t *mu_bands) {
	int ret, len, mlo_wl0, mlo_wl1, mlo_wl2;
	char mlo_cap[OUTPUT_STRING_LENGTH_32];

	len = sizeof(mlo_cap);
	ret = wldm_xbrcm_11be(CMD_GET_NVRAM, -1, mlo_cap, &len, "wl_mlo_config", NULL);
	if (ret < 0) {
		HAL_WIFI_ERR(("%s: Error wldm_xbrcm_11be wl_mlo_config\n",
		__FUNCTION__));
		return WIFI_HAL_ERROR;
	}
	sscanf(mlo_cap, "%d %d %d", &mlo_wl0, &mlo_wl1, &mlo_wl2);
	if (mlo_wl0 >= 0) {
		*mu_bands = ((mlo_wl1 >= 0) && (mlo_wl2 >= 0)) ? WIFI_BAND_2_5_6 : \
			((mlo_wl1 >= 0) ? WIFI_BAND_2_5 : WIFI_BAND_2_6);
	} else if (mlo_wl1 >= 0 && mlo_wl2 >= 0) {
		*mu_bands = WIFI_BAND_5_6;
	} else {
		HAL_WIFI_DBG(("%s: MLO not enabled mlo_cap:%s\n", __FUNCTION__, mlo_cap));
	}
	return WIFI_HAL_SUCCESS;
}
#endif /* WIFI_HAL_VERSION_GE_3_0_3 */

INT wifi_getHalCapability(wifi_hal_capability_t *cap)
{
	unsigned int i = 0, radios_cnt = wldm_get_radios();
	int ret = 0;
#ifdef WIFI_HAL_VERSION_GE_3_0_3
	wifi_multi_link_bands_t mu_bands;
#endif /* WIFI_HAL_VERSION_GE_3_0_3 */
	HAL_WIFI_DBG(("%s: Enter\n", __FUNCTION__));

	NULL_PTR_ASSERT(cap);

	memset(cap, 0, sizeof(wifi_hal_capability_t));

	/* version */
	cap->version.major = WIFI_HAL_MAJOR_VERSION;
	cap->version.minor = WIFI_HAL_MINOR_VERSION;

        if (radios_cnt > MAX_NUM_RADIOS) {
                HAL_WIFI_ERR(("\n Error numRadios=%d MAX_NUM_RADIOS=%d\n",
                        radios_cnt, MAX_NUM_RADIOS));
		radios_cnt = MAX_NUM_RADIOS; /* set to max allowed */
        }

	/* platform_property */
	cap->wifi_prop.numRadios = radios_cnt;

	/* get RadioCapabilities */
	for (i = 0; i < radios_cnt; i++) {
		ret = wl_getRadioCapabilities(i, &(cap->wifi_prop.radiocap[i]));
		if (ret < 0) {
			HAL_WIFI_ERR(("%s: Error wl_getRadioCapabilities idx=%d ret=%d\n",
				__FUNCTION__, i, ret));
			return WIFI_HAL_ERROR;
		}
#if defined(WIFI_HAL_VERSION_GE_3_0_1) && defined(RDKB_LGI)
		/* init to invalid radio index - just to identify where the table stops
		 * a valid radio index is < radios_cnt */
		memset((char *)(&(cap->wifi_prop.radio_interface_map[i])), radios_cnt,
			sizeof(cap->wifi_prop.radio_interface_map[i]));
		ret = wl_getRadioInterfaceMap(i, &(cap->wifi_prop.radio_interface_map[i]));
		if (ret < 0) {
			HAL_WIFI_ERR(("%s: Error wl_getRadioInterfaceMap idx=%d ret=%d\n",
				__FUNCTION__, i, ret));
			return WIFI_HAL_ERROR;
		}
#endif /* WIFI_HAL_VERSION_GE_3_0_1 && RDKB_LGI */
	}

#if defined(WIFI_HAL_VERSION_GE_3_0_1)
	/* init to invalid radio index - just to identify where the table stops
	 * a valid radio index is < radios_cnt */
	memset((char *)(cap->wifi_prop.interface_map), radios_cnt,
		sizeof(cap->wifi_prop.interface_map));
	ret = wl_getWifiInterfaceMap(cap->wifi_prop.interface_map, radios_cnt);
	if (ret < 0) {
		HAL_WIFI_ERR(("%s: Error wl_getWifiInterfaceMap ret=%d\n",
			__FUNCTION__, ret));
		return WIFI_HAL_ERROR;
	}
#endif /* WIFI_HAL_VERSION_GE_3_0_1 */
#ifdef WIFI_HAL_VERSION_GE_3_0_3
	ret = wl_getMLOCapabilities(&mu_bands);
	if (ret < 0) {
		HAL_WIFI_DBG(("%s: wl_getMLOCapabilities\n", __FUNCTION__));
		cap->wifi_prop.mu_bands = WIFI_BAND_NONE;
	}
	else {
		cap->wifi_prop.mu_bands = mu_bands;
	}
#endif /* WIFI_HAL_VERSION_GE_3_0_3 */
	/* BandSteeringSupported */
	ret = wldm_xbrcm_bsd(CMD_GET, 0, WLDM_BSD_STEER_CAP,
		(void *)(&(cap->BandSteeringSupported)), NULL);
	if (ret < 0) {
		HAL_WIFI_ERR(("%s: wldm_xbrcm_bsd GET WLDM_BSD_STEER_CAP error ret=%d\n",
			__FUNCTION__, ret));
		return WIFI_HAL_ERROR;
	}

	return WIFI_HAL_SUCCESS;
}

INT wifi_getAPCapabilities(INT ap_index, wifi_ap_capabilities_t *apCapabilities)
{
	BOOL bval = FALSE;
	int ret, len;
	unsigned int securityModesSupported, methodsSupported;
	char secmStr[OUTPUT_STRING_LENGTH_256], obmStr[OUTPUT_STRING_LENGTH_256];

	NULL_PTR_ASSERT(apCapabilities);
	AP_INDEX_ASSERT(ap_index);

	/* packet size threshold to apply RTS/CTS backoff rules is supported */
	apCapabilities->rtsThresholdSupported = TRUE;

	/* SecurityModesSupported */
	len = sizeof(secmStr);
	ret = wldm_AccessPoint_Security_Modessupported(CMD_GET, ap_index, secmStr,
		&len, NULL, NULL);
	if (ret < 0) {
		HAL_WIFI_ERR(("%s: Error wldm_AccessPoint_Security_Modessupported GET idx=%d\n",
			__FUNCTION__, ap_index));
		return WIFI_HAL_ERROR;
	}
	/* Allow comma or space delimiters in string */
	securityModesSupported = apCapabilities->securityModesSupported;
	wl_map_str2enum_fromTable(secmStr, &securityModesSupported,
		", ", security_mode_table);

	/* refer wifi_getApWpsConfigMethodsSupported */
	len = sizeof(obmStr);
	ret = wldm_AccessPoint_WPS_ConfigMethodsSupported(CMD_GET, ap_index, obmStr, &len, NULL, NULL);
	if (ret < 0) {
		HAL_WIFI_ERR(("%s: Error wldm_AccessPoint_WPS_ConfigMethodsSupported GET idx=%d\n",
			__FUNCTION__, ap_index));
		return WIFI_HAL_ERROR;
	}
	/* Allow comma or space delimiters in string */
	methodsSupported = apCapabilities->methodsSupported;
	wl_map_str2enum_fromTable(obmStr, &methodsSupported,
		", ", wps_config_method_table);

	/* WiFi Multimedia (WMM) Access Categories (AC) is supported */
	apCapabilities->WMMSupported = TRUE;

	/* WMM Unscheduled Automatic Power Save Delivery (U-APSD) is supported */
	apCapabilities->UAPSDSupported = TRUE;

	/* access point supports interworking with external networks */
	apCapabilities->interworkingServiceSupported = TRUE;

	if (!(wldm_11v_btm(WLDM_BTM_GET_IMPLEMENTED, ap_index, NULL, &bval, NULL, 0, NULL))) {
		apCapabilities->BSSTransitionImplemented = bval;
	}
	else {
		return WIFI_HAL_ERROR;
	}
	return WIFI_HAL_SUCCESS;
}

INT wifi_getApWpsConfiguration(INT apIndex, wifi_wps_t* wpsConfig)
{
	int len;
	char enabled_methods[512], *method, *rest = NULL;
	int i;

	HAL_WIFI_DBG(("%s: apIndex [%d]\n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(wpsConfig);

	memset(wpsConfig, 0, sizeof(*wpsConfig));

	/* Get WPS enable */
	if (wldm_AccessPoint_WPS_Enable(CMD_GET, apIndex,
		&wpsConfig->enable, NULL, NULL, NULL)) {
		HAL_WIFI_ERR(("%s: %d: wldm_AccessPoint_WPS_Enable failed\n",
			__FUNCTION__, apIndex));
		return WIFI_HAL_ERROR;
	}

	if (!wpsConfig->enable) {
		return WIFI_HAL_SUCCESS;
	}

	/* Get WPS device PIN */
	len = sizeof(wpsConfig->pin);
	if (wldm_AccessPoint_WPS_PIN(CMD_GET, apIndex,
		wpsConfig->pin, &len, NULL, NULL)) {
		HAL_WIFI_ERR(("%s: %d: wldm_AccessPoint_WPS_PIN failed\n",
			__FUNCTION__, apIndex));
		return WIFI_HAL_ERROR;
	}

	/* Get enabled WPS config methods */
	len = sizeof(enabled_methods);
	if (wldm_AccessPoint_WPS_ConfigMethodsEnabled(CMD_GET, apIndex,
		enabled_methods, &len, NULL, NULL)) {
		HAL_WIFI_ERR(("%s: %d: wldm_AccessPoint_WPS_ConfigMethodsEnabled failed\n",
			__FUNCTION__, apIndex));
		return WIFI_HAL_ERROR;
	}
	method = strtok_r(enabled_methods, ",", &rest);
	while (method != NULL) {
		i = 0;
		while (TRUE) {
			if (wps_config_method_table[i].str_val == NULL) {
				HAL_WIFI_ERR(("%s: %d: unsupported method [%s]\n",
					__FUNCTION__, apIndex, method));
				break;
			}

			if (!strcmp(method, wps_config_method_table[i].str_val)) {
				wpsConfig->methods |= wps_config_method_table[i].enum_val;
				break;
			}
			++i;
		}
		method = strtok_r(NULL, ",", &rest);
	}

	return WIFI_HAL_SUCCESS;
}

INT wifi_setApWpsConfiguration(INT apIndex, wifi_wps_t* wpsConfig)
{
	char enabled_methods[512] = {'\0'}; //buffer is big enough to hold all methods

	AP_INDEX_ASSERT_RC(apIndex, WIFI_HAL_INVALID_ARGUMENTS);
	NULL_PTR_ASSERT_RC(wpsConfig, WIFI_HAL_INVALID_ARGUMENTS);

	HAL_WIFI_LOG(("%s: apIndex [%d]\n", __FUNCTION__, apIndex));

	/* Set WPS enable */
	if (wldm_AccessPoint_WPS_Enable(CMD_SET, apIndex,
		&wpsConfig->enable, NULL, NULL, NULL)) {
		HAL_WIFI_ERR(("%s: %d: wldm_AccessPoint_WPS_Enable failed\n",
			__FUNCTION__, apIndex));
		return WIFI_HAL_ERROR;
	}
	WPS_LOG("RDKB_WPS_ENABLED_%d %s\n", apIndex + 1,
		(wpsConfig->enable) ? "TRUE" : "FALSE");

	if (!wpsConfig->enable)
		return WIFI_HAL_SUCCESS;

	/* Set WPS device PIN */
	if (wpsConfig->pin[0] != '\0') {
		if (wldm_AccessPoint_WPS_PIN(CMD_SET, apIndex,
			wpsConfig->pin, NULL, NULL, NULL)) {
			HAL_WIFI_ERR(("%s: %d: wldm_AccessPoint_WPS_PIN failed\n",
				__FUNCTION__, apIndex));
			return WIFI_HAL_ERROR;
		}
	}

	/* Set enabled WPS config methods */
	if (wpsConfig->methods) {
		int i, size = 0, len;

		len = sizeof(enabled_methods);
		for (i = 0;  wps_config_method_table[i].str_val != NULL; ++i) {
			if (wpsConfig->methods & wps_config_method_table[i].enum_val) {
				if (enabled_methods[0] == '\0')
					size = snprintf(enabled_methods, len, "%s",
						wps_config_method_table[i].str_val);
				else
					size += snprintf(enabled_methods + size, len - size,
						",%s", wps_config_method_table[i].str_val);
			}
		}
		HAL_WIFI_DBG(("%s: %d: enabled_methods=%s\n", __FUNCTION__, apIndex, enabled_methods));
		if (wldm_AccessPoint_WPS_ConfigMethodsEnabled(CMD_SET, apIndex,
			enabled_methods, NULL, NULL, NULL)) {
			HAL_WIFI_ERR(("%s: %d: wldm_AccessPoint_WPS_ConfigMethodsEnabled failed\n",
				__FUNCTION__, apIndex));
			return WIFI_HAL_ERROR;
		}
	}
	WPS_LOG("RDKB_WPS_PBC_CONFIGURED_%d %s\n", apIndex + 1,
		strstr(enabled_methods, "PushButton") ? "TRUE" : "FALSE");

	WPS_LOG("RDKB_WPS_PIN_CONFIGURED_%d %s\n", apIndex + 1,
		(strstr(enabled_methods, "PIN") ||
		strstr(enabled_methods, "Keypad")) ? "TRUE" : "FALSE");
	return WIFI_HAL_SUCCESS;
}

static void
str_to_upper(char *str)
{
	char *p = str;

	while (*p) {
		*p = toupper(*p);
		p++;
	}
}

INT wifi_setRadioOperatingParameters(wifi_radio_index_t index, wifi_radio_operationParam_t *operationParam)
{
	int len, tblSize, i, ret = 0;
	char channelMode[OUTPUT_STRING_LENGTH_32] = "11", variant[OUTPUT_STRING_LENGTH_32];
	char operStd[OUTPUT_STRING_LENGTH_32] = {0};
	unsigned int pureMode = PMODE_NONE, channelWidth, tempVariant = 0;
	bool enable;
	csa_t csaInfo;
	unsigned int radioIndex;

	radioIndex = index;
	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(operationParam);

	len = sizeof(enable);
	enable = operationParam->enable;
	if (wldm_Radio_Enable(CMD_SET, index, &enable, &len, NULL, NULL) < 0) {
		HAL_WIFI_ERR(("%s Fail, wldm_Radio_Enable set failed for index : %d\n", __FUNCTION__, index));
		return WIFI_HAL_ERROR;
	}
	HAL_WIFI_LOG(("%s: radioIndex = %d, Enable=%d\n",
				  __FUNCTION__, index, enable));

	/* enable/disable auto channel mode */
	enable = operationParam->autoChannelEnabled;
	len = sizeof(enable);
	if (wldm_Radio_AutoChannelEnable(CMD_SET, index, &enable, &len, NULL, NULL) < 0) {
		HAL_WIFI_ERR(("%s: wldm_Radio_AutoChannelEnable set failed\n", __FUNCTION__ ));
		return WIFI_HAL_ERROR;
	}
	HAL_WIFI_DBG(("%s: radioIndex = %d, AutoChannelEnabled=%d\n",
				  __FUNCTION__, index, enable));

	/* Translate Variant to oper std mode and append to channel mode */
	tempVariant = operationParam->variant & OPER_STANDS_MASK;
	tblSize = ARRAY_SIZE(std2ieee80211Variant_infoTable);
	for (i = tblSize - 1; i >= 0; i--) {
		if (tempVariant & std2ieee80211Variant_infoTable[i].enum_val) {
			strncat(operStd, std2ieee80211Variant_infoTable[i].str_val,
				sizeof(operStd) - strlen(operStd));
			strncat(operStd, ",",
				 sizeof(operStd) - strlen(operStd));
		}
	}

	/* operStd to channelMode such as 11AX80 and pureMode value as in PMODE_xxx */
	len = strlen(operStd);
	operStd[len - 1] = '\0';
	for (i = 0; operStdPMode_infoTable[i].operStd != NULL; i++) {
		if (!(strcmp(operStdPMode_infoTable[i].operStd, operStd))) {
			/* match */
			snprintf(variant, sizeof(variant), operStdPMode_infoTable[i].pmodeStr);
			str_to_upper(variant);
			strncat(channelMode, variant, sizeof(channelMode) - strlen(channelMode));
			pureMode = operStdPMode_infoTable[i].pmodeVal;
			break;
		}
	}

	/* Translate bandwidth and append to channel mode */
	tblSize = ARRAY_SIZE(wifi_bandwidth_infoTable);
	for (i = 0; i < tblSize; i++) {
		if (operationParam->channelWidth == wifi_bandwidth_infoTable[i].enum_val) {
			strncat(channelMode, wifi_bandwidth_infoTable[i].str_val,
				sizeof(channelMode) - strlen(channelMode));
			channelWidth = atoi(wifi_bandwidth_infoTable[i].str_val);
			break;
		}
	}
	if (i == tblSize) {
		HAL_WIFI_ERR(("%s: invalid bandwidth %d\n", __FUNCTION__,
			operationParam->channelWidth));
		return WIFI_HAL_INVALID_ARGUMENTS;
	}
	HAL_WIFI_DBG(("%s: radioIndex = %d, operationParam channelWidth=%d variant=%d\n",
		__FUNCTION__, index, operationParam->channelWidth, operationParam->variant));

	HAL_WIFI_DBG(("%s: radioIndex = %d, channelMode = %s pureMode=%d operStd %s\n",
		__FUNCTION__, index, channelMode, pureMode, operStd));

	/* Set oper std mode and bandwidth */
	if (wl_setRadioMode(index, channelMode, pureMode) < 0) {
		HAL_WIFI_ERR(("%s: wl_setRadioMode failed\n", __FUNCTION__ ));
		return WIFI_HAL_ERROR;
	}

	if (!operationParam->autoChannelEnabled) {
		unsignedInt channel;
		len = sizeof(operationParam->channel);
		channel = operationParam->channel;
		if (wldm_Radio_Channel(CMD_SET, index, &channel, &len, 0, 0, NULL, NULL) < 0) {
			HAL_WIFI_ERR(("%s: wldm_Radio_Channel set failed\n", __FUNCTION__ ));
			return WIFI_HAL_ERROR;
		}
		if (operationParam->csa_beacon_count) {
			/* Send CSA */
			csaInfo.channel = operationParam->channel;
			csaInfo.channel_width = channelWidth;
			csaInfo.csa_beacon_count = operationParam->csa_beacon_count;

			len = sizeof(csaInfo);
			if (wldm_11h_dfs(CMD_SET_IOCTL, index, &csaInfo, (uint *)&len, "csa_bcast", NULL) < 0) {
				HAL_WIFI_ERR(("%s: Error csa radioIndex=%d\n", __FUNCTION__, index));
				return WIFI_HAL_ERROR;
			}
		}
		HAL_WIFI_DBG(("%s: radioIndex = %d, channel=%d, channelWidth=%d, "
			"csa_beacon_count=%d\n", __FUNCTION__, index, operationParam->channel,
			channelWidth, operationParam->csa_beacon_count));
	}

	ret = wldm_apply(index, 0);
	if (ret < 0) {
		HAL_WIFI_ERR(("%s: wldm_apply() failed, radioIndex=[%d], ret=%d\n",
			__FUNCTION__, index, ret));
		return WIFI_HAL_ERROR;
	}

	return WIFI_HAL_SUCCESS;
}

/* rate string to wifi_bitrate_t */
static struct wifi_enum_to_str_map wifi_bitrate_infoTable[] =
{
        /* brate        rateStr */
        {WIFI_BITRATE_1MBPS,    "1"},
        {WIFI_BITRATE_2MBPS,    "2"},
        {WIFI_BITRATE_5_5MBPS,  "5.5"},
        {WIFI_BITRATE_6MBPS,    "6"},
        {WIFI_BITRATE_9MBPS,    "9"},
        {WIFI_BITRATE_11MBPS,   "11"},
        {WIFI_BITRATE_12MBPS,   "12"},
        {WIFI_BITRATE_18MBPS,   "18"},
        {WIFI_BITRATE_24MBPS,   "24"},
        {WIFI_BITRATE_36MBPS,   "36"},
        {WIFI_BITRATE_48MBPS,   "48"},
        {WIFI_BITRATE_54MBPS,   "54"},
        {0,  NULL}
};

INT wifi_getRadioOperatingParameters(wifi_radio_index_t radioIndex,
		wifi_radio_operationParam_t *operationParam)
{
	BOOL autoChannelEnabled, enable, ieee80211hEanble = FALSE;
	wifi_ieee80211Variant_t variant;	/* The radio operating mode */
	UINT dtimPeriod, beaconInterval;
	UINT basicDataTransmitRates;
	UINT operationalDataTransmitRates, fragmentationThreshold;
	UINT transmitPower, rtsThreshold;
	INT len, ret = WIFI_HAL_SUCCESS;
	CHAR operatingStandards[OUTPUT_STRING_LENGTH_64] = {0};
	CHAR output_string[OUTPUT_STRING_LENGTH_64] = {0};
	wldm_xbrcm_radio_param_t param;
	wifi_channelBandwidth_t bw;
	wifi_freq_bands_t band;
	wifi_countrycode_type_t countryCode;
	wifi_guard_interval_t guardInterval;
	unsigned int i;

	HAL_WIFI_DBG(("%s: Enter, index = %d\n", __FUNCTION__, radioIndex));

	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(operationParam);

	memset(operationParam, 0, sizeof(wifi_radio_operationParam_t));

	len = sizeof(enable);
	if (wldm_Radio_Enable(CMD_GET, radioIndex, &enable, &len, NULL, NULL) != 0) {
	       HAL_WIFI_ERR(("%s: Error checking Radio Enable\n", __FUNCTION__));
	       ret = WIFI_HAL_INTERNAL_ERROR;
	}
	HAL_WIFI_LOG(("%s:  radioIndex=%d radioEnabled=%d\n", __FUNCTION__, radioIndex, enable));

	len = sizeof(autoChannelEnabled);
	if (wldm_Radio_AutoChannelEnable(CMD_GET, radioIndex,
			&autoChannelEnabled, &len, NULL, NULL) < 0) {
		HAL_WIFI_ERR(("%s: Error checking AutoChannelEnable\n", __FUNCTION__));
		ret = WIFI_HAL_INTERNAL_ERROR;
	}
	HAL_WIFI_DBG(("%s:  radioIndex=%d autoChannelEnabled=%d\n",
			__FUNCTION__, radioIndex, autoChannelEnabled));

	len = sizeof(operatingStandards);
	ret = wldm_Radio_OperatingStandards(CMD_GET, radioIndex,
			operatingStandards, &len, NULL, NULL);
	if (ret < 0) {
		HAL_WIFI_ERR(("%s: Failed to get OperatingStandards %d\n", __FUNCTION__, ret));
		ret = WIFI_HAL_INTERNAL_ERROR;
	}
	HAL_WIFI_DBG(("%s: radioIndex=%d OperatingStandards=%s\n",
			__FUNCTION__, radioIndex, operatingStandards));
	wl_map_str2enum_fromTable(operatingStandards,
			&variant, ",", std2ieee80211Variant_infoTable);

	operationParam->variant = variant;
	len = sizeof(ieee80211hEanble);
	ret = wldm_Radio_IEEE80211hEnabled(CMD_GET, radioIndex,
			&ieee80211hEanble, &len, NULL, NULL);
	if (ret < 0) {
		HAL_WIFI_ERR(("%s: wldm_Radio_IEEE80211hEnabled CMD_GET failed inde %d %d\n",
				__FUNCTION__, radioIndex, ret));
		ret = WIFI_HAL_INTERNAL_ERROR;
	}
	HAL_WIFI_DBG(("%s: radioIndex=%d wldm_Radio_IEEE80211hEnabled=%d\n", __FUNCTION__,
			radioIndex, ieee80211hEanble));
	if (ieee80211hEanble)
		operationParam->variant |=  WIFI_80211_VARIANT_H;

	HAL_WIFI_DBG(("%s: radioIndex=%d wl_map_str2enum_fromTable variant=%d\n",
			__FUNCTION__, radioIndex, variant));

	len = sizeof(param);
	ret = wldm_xbrcm_radio(CMD_GET, radioIndex, &param, (uint *)&len, "chanspec", NULL);
	if (ret < 0) {
		HAL_WIFI_ERR(("%s: wldm_xbrcm_Radio_Chanspec CMD_GET radioIndex=%d failed %d\n",
				__FUNCTION__, radioIndex, ret));
		ret = WIFI_HAL_INTERNAL_ERROR;
	}
	HAL_WIFI_DBG(("%s: radioIndex=%d wldm_xbrcm_Radio_Chanspec channel=%d bandwidth=%s\n",
			__FUNCTION__, radioIndex, param.channel, param.bandwidth));

	wl_map_str2enum_fromTable(param.bandwidth, &bw, "MHz", wifi_bandwidth_infoTable);
	HAL_WIFI_DBG(("%s: radioIndex=%d wl_map_str2enum_fromTable bw=%d\n",
			__FUNCTION__, radioIndex, bw));

	wl_map_str2enum_fromTable(param.band, &band, "", wifi_freq_band_infoTable);
	HAL_WIFI_DBG(("%s: radioIndex=%d wl_map_str2enum_fromTable band=0x%x\n",
			__FUNCTION__, radioIndex, band));

	operationParam->enable = enable;
	operationParam->autoChannelEnabled = autoChannelEnabled;
	operationParam->channel = param.channel;
	operationParam->channelWidth = bw;
	operationParam->band = band;

	/* csa_beacon_count is set only. Leave value to 0 till get confirmation from Comcast */
	operationParam->csa_beacon_count = 0;
	HAL_WIFI_DBG(("%s: radioIndex=%d csa_beacon_count is set only %d\n",
			__FUNCTION__, radioIndex, operationParam->csa_beacon_count));

	/* Number of SecondaryChannels */
	operationParam->numSecondaryChannels = param.numSecondaryChannels;
	HAL_WIFI_DBG(("%s: radioIndex=%d  operationParam->numSecondaryChannels = %u\n",
			__FUNCTION__, radioIndex, operationParam->numSecondaryChannels));

	/* Secondary channel list */
	for (i = 0; i < operationParam->numSecondaryChannels; i++) {
		operationParam->channelSecondary[i] = param.channelSecondary[i];
		HAL_WIFI_DBG(("%s: radioIndex=%d  operationParam.channelSecondary[%d] = %u\n",
				__FUNCTION__, radioIndex, i, operationParam->channelSecondary[i]));
	}

	/* countryCode */
	len = sizeof(output_string);
	memset(output_string, 0, sizeof(output_string));
	if (wldm_Radio_RegulatoryDomain(CMD_GET, radioIndex,
			output_string, &len, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: wldm_Radio_RegulatoryDomain CMD_GET failed\n", __FUNCTION__));
		ret =  WIFI_HAL_INTERNAL_ERROR;
	}
	output_string[strlen(output_string) - 1] = '\0';	/* remove traiing "I/O" */
	wl_map_str2enum_fromTable(output_string, &countryCode, "", countrycode_table);
	operationParam->countryCode = countryCode;
	HAL_WIFI_DBG(("%s: radioIndex=%d countrycode_table countryCode=%d\n",
			__FUNCTION__, radioIndex, countryCode));

	/* BOOL DCSEnabled */
	operationParam->DCSEnabled = FALSE;   /* Does not support */
	HAL_WIFI_DBG(("%s: radioIndex=%d operationParam->DCSEnabled = %d\n",
			__FUNCTION__, radioIndex, operationParam->DCSEnabled));

	/* UINT dtimPeriod */
	len = sizeof(dtimPeriod);
	if (wldm_Radio_DTIMPeriod(CMD_GET, radioIndex, (int *)&dtimPeriod, &len, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: wldm_Radio_DTIMPeriod CMD_GET failed\n", __FUNCTION__));
		ret = WIFI_HAL_INTERNAL_ERROR;
	}
	operationParam->dtimPeriod = dtimPeriod;
	HAL_WIFI_DBG(("%s: radioIndex=%d dtimPeriod=0x%d\n", __FUNCTION__, radioIndex, dtimPeriod));

	/* UINT beaconInterval */
	len = sizeof(beaconInterval);
	if (wldm_Radio_BeaconPeriod(CMD_GET, radioIndex, (int *)&beaconInterval, &len, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: wldm_Radio_BeaconPeriod CMD_GET Failed\n", __FUNCTION__));
		ret = WIFI_HAL_INTERNAL_ERROR;
	}
	operationParam->beaconInterval = beaconInterval;
	HAL_WIFI_DBG(("%s: radioIndex=%d beaconInterval=0x%u\n",
			__FUNCTION__, radioIndex, beaconInterval));

	/* UINT operatingClass */
	operationParam->operatingClass = 0; /* new parameter, not implemented yet */
	HAL_WIFI_DBG(("%s: radioIndex=%d  operationParam->operatingClass not implemented\n",
			__FUNCTION__, radioIndex));

	/* UINT basicDataTransmitRates */
	memset(output_string, 0, sizeof(output_string));
	len = sizeof(output_string);
	if (wldm_Radio_BasicDataTransmitRates(CMD_GET, radioIndex,
			output_string, &len, NULL, NULL) != 0) {
		HAL_WIFI_LOG(("%s: wldm_Radio_BasicDataTransmitRates CMD_GET failed\n",
				__FUNCTION__));
		ret = WIFI_HAL_INTERNAL_ERROR;
	}
	wl_map_str2enum_fromTable(output_string, &basicDataTransmitRates,
			",", wifi_bitrate_infoTable);
	operationParam->basicDataTransmitRates = basicDataTransmitRates;
	HAL_WIFI_DBG(("%s: radioIndex=%d basicDataTransmitRates=%u\n",
			__FUNCTION__, radioIndex, basicDataTransmitRates));

	/* UINT operationalDataTransmitRates */
	memset(output_string, 0, sizeof(output_string));
	len = sizeof(output_string);
	if (wldm_Radio_OperationalDataTransmitRates(CMD_GET, radioIndex,
		output_string, &len, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: wldm_Radio_BasicDataTransmitRates CMD_GET failed\n",
				__FUNCTION__));
		ret = WIFI_HAL_INTERNAL_ERROR;
	}
	wl_map_str2enum_fromTable(output_string, &operationalDataTransmitRates, ",",
		wifi_bitrate_infoTable);
	operationParam->operationalDataTransmitRates = operationalDataTransmitRates;
	HAL_WIFI_DBG(("%s: radioIndex=%d operationalDataTransmitRates=%u\n", __FUNCTION__,
		radioIndex, operationalDataTransmitRates));

	/* UINT fragmentationThreshold */
	len = sizeof(fragmentationThreshold);
	if (wldm_Radio_FragmentationThreshold(CMD_GET, radioIndex,
		&fragmentationThreshold, &len, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: wldm_Radio_FragmentationThreshold CMD_GET failed\n",
				__FUNCTION__));
		ret = WIFI_HAL_INTERNAL_ERROR;
	}
	operationParam->fragmentationThreshold = fragmentationThreshold;
	HAL_WIFI_DBG(("%s: radioIndex=%d fragmentationThreshold=%u\n",
		__FUNCTION__, radioIndex, fragmentationThreshold));

	/* wifi_guard_interval_t guardInterval */
	len = sizeof(guardInterval);
	if (wldm_xbrcm_radio(CMD_GET, radioIndex, (void *)&guardInterval,
		(uint *)&len, "guardInterval", NULL) != 0) {
		HAL_WIFI_ERR(("%s: wldm_xbrcm_radio CMD_GET guardInterval failed\n",
				__FUNCTION__));
		ret = WIFI_HAL_INTERNAL_ERROR;
	}
	operationParam->guardInterval = guardInterval;
	HAL_WIFI_DBG(("%s: radioIndex=%d guardInterval=%u\n",
		__FUNCTION__, radioIndex, guardInterval));

	/* UINT transmitPower */
	len = sizeof(transmitPower);
	if (wldm_Radio_TransmitPower(CMD_GET, radioIndex,
			(int *)&transmitPower, &len, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: wldm_Radio_TransmitPower CMD_GET failed\n", __FUNCTION__));
		ret = WIFI_HAL_INTERNAL_ERROR;
	}
	operationParam->transmitPower = transmitPower;
	HAL_WIFI_DBG(("%s: radioIndex=%d transmitPower=%d\n",
			__FUNCTION__, radioIndex, transmitPower));

	/* UINT rtsThreshold */
	len = sizeof(rtsThreshold);
	if (wldm_Radio_RTSThreshold(CMD_GET, radioIndex,
			&rtsThreshold, &len, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: wldm_Radio_RTSThreshold CMD_GET failed\n", __FUNCTION__));
		ret = WIFI_HAL_INTERNAL_ERROR;
	}
	operationParam->rtsThreshold = rtsThreshold;
	HAL_WIFI_DBG(("%s: radioIndex=%d rtsThreshold=%u\n",
			__FUNCTION__, radioIndex, rtsThreshold));

	return ret;
}

INT wifi_getRadioFrequencyBand(wifi_radio_index_t radioIndex, wifi_freq_bands_t *band)
{
	CHAR operatingFrequencyBand[OUTPUT_STRING_LENGTH_64];
	int len = OUTPUT_STRING_LENGTH_64, returnStatus;

	HAL_WIFI_DBG(("%s: Enter, index = %d\n", __FUNCTION__, radioIndex));

	RADIO_INDEX_ASSERT(radioIndex);
	NULL_PTR_ASSERT(band);

	returnStatus = wldm_Radio_OperatingFrequencyBand(CMD_GET, (int)radioIndex, operatingFrequencyBand, &len, NULL, NULL);
	if (returnStatus != RETURN_OK) {
		HAL_WIFI_DBG(("%s: wldm_Radio_OperatingFrequencyBand CMD_GET Failed, Status = %d\n",
			__FUNCTION__, returnStatus));
		return WIFI_HAL_INTERNAL_ERROR;
	}

	if (!strncmp(operatingFrequencyBand, "auto", sizeof(operatingFrequencyBand))) {
		*band = WIFI_FREQUENCY_5_BAND | WIFI_FREQUENCY_2_4_BAND;
	} else if (!strncmp(operatingFrequencyBand, "a", sizeof(operatingFrequencyBand))) {
		*band = WIFI_FREQUENCY_5_BAND;
	} else if (!strncmp(operatingFrequencyBand, "b", sizeof(operatingFrequencyBand))) {
		*band = WIFI_FREQUENCY_2_4_BAND;
	} else if (!strncmp(operatingFrequencyBand, "6g", sizeof(operatingFrequencyBand))) {
		*band = WIFI_FREQUENCY_6_BAND;
	} else {
		HAL_WIFI_DBG(("%s: radioIndex = %d, INCORRECT operatingFrequencyBand = %s\n",
			__FUNCTION__, radioIndex, operatingFrequencyBand));
		return WIFI_HAL_INVALID_VALUE;
	}

	return WIFI_HAL_SUCCESS;
}

INT wifi_getApSecurity(INT apIndex, wifi_vap_security_t *security)
{
	int len, i;
	char security_mode[128] = {'\0'}, encryption[128] = {'\0'}, mfpStr[32] = {'\0'};

	AP_INDEX_ASSERT_RC(apIndex, WIFI_HAL_INVALID_ARGUMENTS);

	HAL_WIFI_DBG(("%s: apIndex [%d]\n", __FUNCTION__, apIndex));

	NULL_PTR_ASSERT_RC(security, WIFI_HAL_INVALID_ARGUMENTS);
	memset(security, 0, sizeof(*security));

	/* get security mode */
	len = sizeof(security_mode);
	if (wldm_AccessPoint_Security_ModeEnabled(CMD_GET, apIndex, security_mode, &len, NULL, NULL)) {
		HAL_WIFI_ERR(("%s: %d: wldm_AccessPoint_Security_ModeEnabled failed\n",
			__FUNCTION__, apIndex));
		return WIFI_HAL_ERROR;
	}
	HAL_WIFI_DBG(("%s: %d: security mode [%s]\n", __FUNCTION__,
		apIndex, security_mode));

	for (i = 0; security_mode_table[i].str_val != NULL; ++i) {
		if (!strcmp(security_mode_table[i].str_val, security_mode)) {
			security->mode = security_mode_table[i].enum_val;
			break;
		}
	}
	if (security_mode_table[i].str_val == NULL) {
		HAL_WIFI_ERR(("%s: %d: unsupported security mode [%s]\n", __FUNCTION__,
			apIndex, security_mode));
		return WIFI_HAL_INVALID_ARGUMENTS;
	}

	/* get mfp */
	len = sizeof(mfpStr);
	if (wldm_AccessPoint_Security_MFPConfig(CMD_GET_NVRAM, apIndex, mfpStr, &len, NULL, NULL)) {
		HAL_WIFI_ERR(("%s: %d: wldm_AccessPoint_Security_MFPConfig failed\n",
			__FUNCTION__, apIndex));
		return WIFI_HAL_ERROR;
	}
	HAL_WIFI_DBG(("%s: %d: mfp [%s]\n", __FUNCTION__,
		apIndex, mfpStr));
	for (i = 0; mfp_table[i].str_val != NULL; ++i) {
		if (!strcmp(mfp_table[i].str_val, mfpStr)) {
			security->mfp = mfp_table[i].enum_val;
			break;
		}
	}
	if (mfp_table[i].str_val == NULL) {
		HAL_WIFI_ERR(("%s: %d: unsupported mfp mode [%s]\n", __FUNCTION__,
			apIndex, mfpStr));
		return WIFI_HAL_INVALID_ARGUMENTS;
	}

	if (security->mode == wifi_security_mode_none)
		return WIFI_HAL_SUCCESS;

	/* get wep key */
	if (security->mode == wifi_security_mode_wep_64
		|| security->mode == wifi_security_mode_wep_128) {
		return WIFI_HAL_UNSUPPORTED; /* wep not supported */
	}

	/* get wpa encryption */
	len = sizeof(encryption);
	if (wldm_AccessPoint_Wpa_Encryptionmode(CMD_GET, apIndex, encryption, &len, NULL, NULL)) {
		HAL_WIFI_ERR(("%s: %d: wldm_AccessPoint_Wpa_Encryptionmode failed\n",
			__FUNCTION__, apIndex));
		return WIFI_HAL_ERROR;
	}
	HAL_WIFI_DBG(("%s: %d: encryption [%s]\n", __FUNCTION__,
		apIndex, encryption));

	for (i = 0; encryption_table[i].str_val != NULL; ++i) {
		if (!strcmp(encryption_table[i].str_val, encryption)) {
			security->encr = encryption_table[i].enum_val;
			break;
		}
	}

	if (encryption_table[i].str_val == NULL) {
		HAL_WIFI_ERR(("%s: %d: unsupported encryption [%s]\n", __FUNCTION__,
			apIndex, encryption));
		return WIFI_HAL_INVALID_ARGUMENTS;
	}

	/* get wpa psk or passphrase */
	if (security->mode == wifi_security_mode_wpa_personal
		|| security->mode == wifi_security_mode_wpa2_personal
		|| security->mode == wifi_security_mode_wpa3_personal
		|| security->mode == wifi_security_mode_wpa_wpa2_personal
		|| security->mode == wifi_security_mode_wpa3_transition) {
		len = sizeof(security->u.key.key);
		if (wldm_AccessPoint_Security_KeyPassphrase(CMD_GET, apIndex,
			security->u.key.key, &len, NULL, NULL) == 0) {
			if (security->mode == wifi_security_mode_wpa3_personal)
				security->u.key.type = wifi_security_key_type_sae;
			else if (security->mode == wifi_security_mode_wpa3_transition)
				security->u.key.type = wifi_security_key_type_psk_sae;
			else
				security->u.key.type = wifi_security_key_type_pass;
		} else { /* not passphrase, try to get psk */
			len = sizeof(security->u.key.key);
			if (wldm_AccessPoint_Security_PreSharedKey(CMD_GET,
				apIndex, security->u.key.key, &len, NULL, NULL) == 0) {
				security->u.key.type = wifi_security_key_type_psk;
			} else {
				HAL_WIFI_ERR(("%s: %d: fail to get psk or passphrase\n",
					__FUNCTION__, apIndex));
				return WIFI_HAL_ERROR;
			}
		}

		/* get wpa3_transition_disable */
		if (security->mode == wifi_security_mode_wpa3_transition) {
			if (wldm_AccessPoint_Security_WPA3TransitionDisable(CMD_GET, apIndex,
				&security->wpa3_transition_disable, NULL, NULL, NULL)) {
				HAL_WIFI_ERR(("%s: %d: fail to get wpa3_transition_disable\n",
					__FUNCTION__, apIndex));
				return WIFI_HAL_ERROR;
			}
		}
	}

	/* get RADIUS auth server params */
	if (security->mode == wifi_security_mode_wpa_enterprise
		|| security->mode == wifi_security_mode_wpa2_enterprise
		|| security->mode == wifi_security_mode_wpa3_enterprise
		|| security->mode == wifi_security_mode_wpa_wpa2_enterprise) {
		char ip_txt[INET6_ADDRSTRLEN] = {'\0'};
		unsigned int port;

		/* get primary radius auth server ip addr, port, secret */
		len = sizeof(ip_txt);
		if (wldm_AccessPoint_Security_RadiusServerIPAddr(CMD_GET, apIndex,
			ip_txt, &len, NULL, NULL)) {
			HAL_WIFI_ERR(("%s: %d: wldm_AccessPoint_Security_RadiusServerIPAddr failed\n",
				__FUNCTION__, apIndex));
			return WIFI_HAL_ERROR;
		}
#if defined(WIFI_HAL_VERSION_3_PHASE2)
		if (inet_pton(AF_INET, ip_txt, &security->u.radius.ip.u.IPv4addr) > 0) {
			security->u.radius.ip.family = wifi_ip_family_ipv4;
		} else if (inet_pton(AF_INET6, ip_txt, security->u.radius.ip.u.IPv6addr) > 0) {
			security->u.radius.ip.family = wifi_ip_family_ipv6;
		} else {
			 HAL_WIFI_ERR(("%s: %d: fail to convert primary RADIUS server ip addr "
				"from text to binary form [%s]\n",
				__FUNCTION__, apIndex, ip_txt));
			return WIFI_HAL_INVALID_VALUE;
		}
#else /* WIFI_HAL_VERSION_3_PHASE2 */
		snprintf((char *)security->u.radius.ip, sizeof(security->u.radius.ip), ip_txt);
#endif /* WIFI_HAL_VERSION_3_PHASE2 */
		HAL_WIFI_DBG(("%s: %d: primary RADIUS server ip addr [%s]\n", __FUNCTION__,
			apIndex, ip_txt));

		len = sizeof(port);
		if (wldm_AccessPoint_Security_RadiusServerPort(CMD_GET, apIndex,
			&port, &len, NULL, NULL)) {
			HAL_WIFI_ERR(("%s: %d: wldm_AccessPoint_Security_RadiusServerPort failed\n",
				__FUNCTION__, apIndex));
			return WIFI_HAL_ERROR;
		}
		security->u.radius.port = (unsigned short)port;
		HAL_WIFI_DBG(("%s: %d: primary RADIUS server port [%u]\n", __FUNCTION__,
			apIndex, port));

		len = sizeof(security->u.radius.key);
		if (wldm_AccessPoint_Security_RadiusSecret(CMD_GET, apIndex,
			security->u.radius.key, &len, NULL, NULL)) {
			HAL_WIFI_ERR(("%s: %d: wldm_AccessPoint_Security_RadiusSecret failed\n",
				__FUNCTION__, apIndex));
			return WIFI_HAL_ERROR;
		}
		HAL_WIFI_DBG(("%s: %d: primary RADIUS server secret [%s]\n", __FUNCTION__,
			apIndex, security->u.radius.key));

		/* get secondary radius auth server ip addr, port, secret,
		 * it is not an error if secondary radius server is not set */
		len = sizeof(ip_txt);
		if (wldm_AccessPoint_Security_SecondaryRadiusServerIPAddr(CMD_GET, apIndex,
			ip_txt, &len, NULL, NULL) == 0) {
#if defined(WIFI_HAL_VERSION_3_PHASE2)
			if (inet_pton(AF_INET, ip_txt, &security->u.radius.s_ip.u.IPv4addr) > 0) {
				security->u.radius.s_ip.family = wifi_ip_family_ipv4;
			} else if (inet_pton(AF_INET6, ip_txt, security->u.radius.s_ip.u.IPv6addr) > 0) {
				security->u.radius.s_ip.family = wifi_ip_family_ipv6;
			} else {
				HAL_WIFI_ERR(("%s: %d: fail to convert secondary RADIUS server ip addr "
					"from text to binary form [%s]\n",
					__FUNCTION__, apIndex, ip_txt));
				return WIFI_HAL_INVALID_VALUE;
			}
#else /* WIFI_HAL_VERSION_3_PHASE2 */
			snprintf((char *)security->u.radius.s_ip, sizeof(security->u.radius.s_ip), ip_txt);
#endif /* WIFI_HAL_VERSION_3_PHASE2 */
			HAL_WIFI_DBG(("%s: %d: secondary RADIUS server ip addr [%s]\n", __FUNCTION__,
				apIndex, ip_txt));

			len = sizeof(port);
			if (wldm_AccessPoint_Security_SecondaryRadiusServerPort(CMD_GET, apIndex,
				&port, &len, NULL, NULL)) {
				HAL_WIFI_ERR(("%s: %d: wldm_AccessPoint_Security_SecondaryRadiusServerPort failed\n",
					__FUNCTION__, apIndex));
				return WIFI_HAL_ERROR;
			}
			security->u.radius.s_port = (unsigned short)port;
			HAL_WIFI_DBG(("%s: %d: secondary RADIUS server port [%d]\n", __FUNCTION__,
				apIndex, port));

			len = sizeof(security->u.radius.s_key);
			if (wldm_AccessPoint_Security_SecondaryRadiusSecret(CMD_GET, apIndex,
				security->u.radius.s_key, &len, NULL, NULL)) {
				HAL_WIFI_ERR(("%s: %d: wldm_AccessPoint_Security_SecondaryRadiusSecret failed\n",
					__FUNCTION__, apIndex));
				return WIFI_HAL_ERROR;
			}
			HAL_WIFI_DBG(("%s: %d: secondary RADIUS server secret [%s]\n", __FUNCTION__,
				apIndex, security->u.radius.s_key));
		} else {
			HAL_WIFI_DBG(("%s: %d: secondary RADIUS server not set yet\n",
				__FUNCTION__, apIndex));
		}
	}

	return WIFI_HAL_SUCCESS;
}

static INT
wifi_updateApSecurity(INT apIndex, wifi_vap_security_t *security)
{
	int i, len;
	char *mfpStr;

	AP_INDEX_ASSERT_RC(apIndex, WIFI_HAL_INVALID_ARGUMENTS);

	HAL_WIFI_LOG(("%s: apIndex [%d]\n", __FUNCTION__, apIndex));

	NULL_PTR_ASSERT_RC(security, WIFI_HAL_INVALID_ARGUMENTS);

	if (security->mode == wifi_security_mode_wep_64
		|| security->mode == wifi_security_mode_wep_128) {
		HAL_WIFI_ERR(("%s: %d: WEP not supported\n", __FUNCTION__, apIndex));
		return WIFI_HAL_UNSUPPORTED;
	}

	/* set mfp */
	/* WIFI_HAL_VERSION_3_PHASE2 for RDKM */
	for (i = 0; mfp_table[i].str_val != NULL; ++i) {
		if (mfp_table[i].enum_val == security->mfp)
			break;
	}
	if (mfp_table[i].str_val == NULL) {
		HAL_WIFI_ERR(("%s: %d: wrong mfp [%d]\n", __FUNCTION__,
			apIndex, security->mfp));
		return WIFI_HAL_INVALID_ARGUMENTS;
	}

	mfpStr = (char *)mfp_table[i].str_val;
	HAL_WIFI_LOG(("%s: %d: mfp [%s]\n", __FUNCTION__,
		apIndex, mfpStr));
	if (wldm_AccessPoint_Security_MFPConfig(CMD_SET, apIndex, mfpStr, NULL, NULL, NULL)) {
		HAL_WIFI_ERR(("%s: %d: wldm_AccessPoint_Security_MFPConfig failed\n",
			__FUNCTION__, apIndex));
		return WIFI_HAL_ERROR;
	}

	/* set security mode */
	for (i = 0; security_mode_table[i].str_val != NULL; ++i) {
		if (security_mode_table[i].enum_val == security->mode)
			break;
	}
	if (security_mode_table[i].str_val == NULL) {
		HAL_WIFI_ERR(("%s: %d: wrong security mode [%d]\n", __FUNCTION__,
			apIndex, security->mode));
		return WIFI_HAL_INVALID_ARGUMENTS;
	}
	HAL_WIFI_LOG(("%s: %d: security mode [%s]\n", __FUNCTION__,
		apIndex, security_mode_table[i].str_val));
	if (wldm_AccessPoint_Security_ModeEnabled(CMD_SET, apIndex,
		(char *)security_mode_table[i].str_val, NULL, NULL, NULL)) {
		HAL_WIFI_ERR(("%s: %d: wldm_AccessPoint_Security_ModeEnabled failed\n",
			__FUNCTION__, apIndex));
		return WIFI_HAL_ERROR;
	}
	if (security->mode == wifi_security_mode_none)
		return WIFI_HAL_SUCCESS;

	/* set wpa encryption */
	for (i = 0; encryption_table[i].str_val != NULL; ++i) {
		if (encryption_table[i].enum_val == security->encr)
			break;
	}

	if (encryption_table[i].str_val == NULL) {
		HAL_WIFI_ERR(("%s: %d: wrong encryption [%d]\n", __FUNCTION__,
			apIndex, security->encr));
		return WIFI_HAL_INVALID_ARGUMENTS;
	}

	HAL_WIFI_LOG(("%s: %d: encryption [%s]\n", __FUNCTION__, apIndex, encryption_table[i].str_val));
	if (wldm_AccessPoint_Wpa_Encryptionmode(CMD_SET, apIndex,
		(char *)encryption_table[i].str_val, NULL, NULL, NULL)) {
		HAL_WIFI_ERR(("%s: %d: wldm_AccessPoint_Wpa_Encryptionmode failed\n",
			__FUNCTION__, apIndex));
		return WIFI_HAL_ERROR;
	}

	/* set wpa psk or passphrase */
	if (security->mode == wifi_security_mode_wpa_personal
		|| security->mode == wifi_security_mode_wpa2_personal
		|| security->mode == wifi_security_mode_wpa3_personal
		|| security->mode == wifi_security_mode_wpa_wpa2_personal
		|| security->mode == wifi_security_mode_wpa3_transition) {
		len = strlen(security->u.key.key) + 1;
		if (security->u.key.type == wifi_security_key_type_psk) {
			HAL_WIFI_LOG(("%s: %d: wpa psk [%s]\n", __FUNCTION__,
				apIndex, security->u.key.key));
			if (wldm_AccessPoint_Security_PreSharedKey(CMD_SET,
				apIndex, security->u.key.key, &len, NULL, NULL)) {
				HAL_WIFI_ERR(("%s: %d: wldm_AccessPoint_Security_PreSharedKey failed\n",
					__FUNCTION__, apIndex));
				return WIFI_HAL_ERROR;
			}

		} else {
			HAL_WIFI_LOG(("%s: %d: wpa passphrase [%s]\n", __FUNCTION__,
				apIndex, security->u.key.key));
			if (wldm_AccessPoint_Security_KeyPassphrase(CMD_SET, apIndex,
				security->u.key.key, &len, NULL, NULL)) {
				HAL_WIFI_ERR(("%s: %d: wldm_AccessPoint_Security_KeyPassphrase failed\n",
					__FUNCTION__, apIndex));
				return WIFI_HAL_ERROR;
			}

		}

		/* set wpa3_transition_disable */
		if (security->mode == wifi_security_mode_wpa3_transition) {
			if (wldm_AccessPoint_Security_WPA3TransitionDisable(CMD_SET, apIndex,
				&security->wpa3_transition_disable, NULL, NULL, NULL)) {
				HAL_WIFI_ERR(("%s: %d: fail to set wpa3_transition_disable\n",
					__FUNCTION__, apIndex));
				return WIFI_HAL_ERROR;
			}
		}
	}

	/* set RADIUS auth server params
	 * greylist requires RADIUS params for open mode,
	 * but greylist is not configured through this API, and greylist
	 * may be enabled after RADIUS params are set, we have to set RADIUS
	 * params for open mode even greylist may not be enabled.
	 */
	if (security->mode == wifi_security_mode_wpa_enterprise
		|| security->mode == wifi_security_mode_wpa2_enterprise
		|| security->mode == wifi_security_mode_wpa3_enterprise
		|| security->mode == wifi_security_mode_wpa_wpa2_enterprise
		|| security->mode == wifi_security_mode_none) {
		char ip_txt[INET6_ADDRSTRLEN] = {'\0'};
                unsigned int port;
#if defined(WIFI_HAL_VERSION_3_PHASE2)
                int domain;
#endif

#if defined(WIFI_HAL_VERSION_3_PHASE2)
		/* set primary radius auth server ip addr, port, secret */
		if (security->u.radius.ip.family == wifi_ip_family_ipv4) {
			domain = AF_INET;
		} else if (security->u.radius.ip.family == wifi_ip_family_ipv6) {
			domain = AF_INET6;
		} else {
			HAL_WIFI_ERR(("%s: %d: unknown IP addr family\n", __FUNCTION__, apIndex));
			return WIFI_HAL_INVALID_ARGUMENTS;
		}
		/* addr of IPv4adddr and IPv6addr is the same as they are in the same union */
		if (inet_ntop(domain, (void *)security->u.radius.ip.u.IPv6addr,
			ip_txt, sizeof(ip_txt)) == NULL) {
			HAL_WIFI_ERR(("%s: %d: fail to convert primary RADIUS server ip addr "
				"from binary form to text\n", __FUNCTION__, apIndex));
			return WIFI_HAL_INVALID_ARGUMENTS;
		}
#else /* WIFI_HAL_VERSION_3_PHASE2 */
		snprintf(ip_txt, sizeof(ip_txt), (char *)security->u.radius.ip);
#endif /* WIFI_HAL_VERSION_3_PHASE2 */

		HAL_WIFI_LOG(("%s: %d: primary RADIUS server ip addr [%s]\n", __FUNCTION__,
			apIndex, ip_txt));
		if (wldm_AccessPoint_Security_RadiusServerIPAddr(CMD_SET, apIndex,
			ip_txt, NULL, NULL, NULL)) {
			HAL_WIFI_ERR(("%s: %d: wldm_AccessPoint_Security_RadiusServerIPAddr failed\n",
				__FUNCTION__, apIndex));
			return WIFI_HAL_ERROR;
		}

		port = security->u.radius.port;
		HAL_WIFI_LOG(("%s: %d: primary RADIUS server port [%u]\n", __FUNCTION__, apIndex, port));
		if (wldm_AccessPoint_Security_RadiusServerPort(CMD_SET, apIndex,
			&port, NULL, NULL, NULL)) {
			HAL_WIFI_ERR(("%s: %d: wldm_AccessPoint_Security_RadiusServerPort failed\n",
				__FUNCTION__, apIndex));
			return WIFI_HAL_ERROR;
		}

		HAL_WIFI_LOG(("%s: %d: primary RADIUS server secret [%s]\n", __FUNCTION__,
			apIndex, security->u.radius.key));
		if (wldm_AccessPoint_Security_RadiusSecret(CMD_SET, apIndex,
			security->u.radius.key, NULL, NULL, NULL)) {
			HAL_WIFI_ERR(("%s: %d: wldm_AccessPoint_Security_RadiusSecret failed\n",
				__FUNCTION__, apIndex));
			return WIFI_HAL_ERROR;
		}

#if defined(WIFI_HAL_VERSION_3_PHASE2)
		/* set secondary radius auth server ip addr, port, secret */
		if (security->u.radius.s_ip.family == wifi_ip_family_ipv4) {
			if (security->u.radius.s_ip.u.IPv4addr == 0) {
				/* So far, we cannot know explicitly how secondary RADIUS auth server
				 * is not set , but mostly likely, memory for secondary RADIUS auth
				 * server is set to all '\0'
				 *
				 * TODO: To discuss how to decide secondary RADIUS auth server
				 * is not set in a better way */
				HAL_WIFI_LOG(("%s: %d: secondary RADIUS server not set\n",
					__FUNCTION__, apIndex));
				return WIFI_HAL_SUCCESS;
			}
			domain = AF_INET;
		} else if (security->u.radius.s_ip.family == wifi_ip_family_ipv6) {
			domain = AF_INET6;
		} else {
			/* consider this as secondary RADIUS auth is not configured,
			 * until we have a better way to decide secondary RADIUS is not set */
			HAL_WIFI_DBG(("%s: %d: secondary RADIUS auth server is not configured\n",
				__FUNCTION__, apIndex));
			return WIFI_HAL_SUCCESS;
		}
		/* addr of IPv4adddr and IPv6addr is the same as they are in the same union */
		if (inet_ntop(domain, (void *)security->u.radius.s_ip.u.IPv6addr,
			ip_txt, sizeof(ip_txt)) == NULL) {
			HAL_WIFI_ERR(("%s: %d: fail to convert secondary RADIUS server ip addr "
				"from binary form to text\n", __FUNCTION__, apIndex));
			/* Don't consider this as error */
			return WIFI_HAL_SUCCESS;
		}
#else /* WIFI_HAL_VERSION_3_PHASE2 */
		snprintf(ip_txt, sizeof(ip_txt), (char *)security->u.radius.s_ip);
#endif /* WIFI_HAL_VERSION_3_PHASE2 */
		HAL_WIFI_LOG(("%s: %d: secondary RADIUS server ip addr [%s]\n", __FUNCTION__,
			apIndex, ip_txt));
		if (wldm_AccessPoint_Security_SecondaryRadiusServerIPAddr(CMD_SET, apIndex,
			ip_txt, NULL, NULL, NULL)) {
			HAL_WIFI_ERR(("%s: %d: wldm_AccessPoint_Security_SecondaryRadiusServerIPAddr failed\n",
				__FUNCTION__, apIndex));
			return WIFI_HAL_ERROR;
		}

		port = security->u.radius.s_port;
		HAL_WIFI_LOG(("%s: %d: secondary RADIUS server port [%u]\n", __FUNCTION__,
			apIndex, port));
		if (wldm_AccessPoint_Security_SecondaryRadiusServerPort(CMD_SET, apIndex,
			&port, NULL, NULL, NULL)) {
			HAL_WIFI_ERR(("%s: %d: wldm_AccessPoint_Security_SecondaryRadiusServerPort failed\n",
				__FUNCTION__, apIndex));
			return WIFI_HAL_ERROR;
		}

		HAL_WIFI_LOG(("%s: %d: secondary RADIUS server secret [%s]\n", __FUNCTION__,
			apIndex, security->u.radius.s_key));
		if (wldm_AccessPoint_Security_SecondaryRadiusSecret(CMD_SET, apIndex,
			security->u.radius.s_key, NULL, NULL, NULL)) {
			HAL_WIFI_ERR(("%s: %d: wldm_AccessPoint_Security_SecondaryRadiusSecret failed\n",
				__FUNCTION__, apIndex));
			return WIFI_HAL_ERROR;
		}
	}

	return WIFI_HAL_SUCCESS;
}

INT wifi_setApSecurity(INT apIndex, wifi_vap_security_t *security)
{
	if (wifi_updateApSecurity(apIndex, security) != WIFI_HAL_SUCCESS) {
		HAL_WIFI_ERR(("%s: %d: wifi_updateApSecurity failed\n",
					  __FUNCTION__, apIndex));
		return WIFI_HAL_ERROR;
	}

	if (wldm_apply_AccessPointSecurityObject(apIndex) != 0) {
		HAL_WIFI_ERR(("%s: apply_AccessPointObject Failed apIndex [%d]\n", __FUNCTION__, apIndex));
		return RETURN_ERR;
	}

	return WIFI_HAL_SUCCESS;
}

INT wifi_getEAP_Param(UINT apIndex, wifi_eap_config_t *output)
{
	AP_INDEX_ASSERT_RC(apIndex, WIFI_HAL_INVALID_ARGUMENTS);

	NULL_PTR_ASSERT_RC(output, WIFI_HAL_INVALID_ARGUMENTS);

	if (wldm_AccessPoint_Security_WPAPairwiseRetries(CMD_GET, apIndex,
		&output->uiEAPOLKeyRetries, NULL, NULL, NULL)) {
		HAL_WIFI_ERR(("%s: %d: wldm_AccessPoint_Security_WPAPairwiseRetries CMD_GET failed\n",
			__FUNCTION__, apIndex));
	}

	/* hostapd sets different eapol key timeouts in different conditions */
	output->uiEAPOLKeyTimeout = 1;

	/* hostapd sets initial retransmission to 3 seconds,
	 * then double the retransmission timeout to provide back off per RFC 3748 5.5.
	 * The maximum retransmission timeout is limited to 20 seocnds. */
	output->uiEAPIdentityRequestTimeout = output->uiEAPRequestTimeout = 3;

	/* hostapd hardcodes max retries of EAP Request packets to 5 */
	output->uiEAPIdentityRequestRetries = output->uiEAPRequestRetries = 5;

	return WIFI_HAL_SUCCESS;
}

INT wifi_setEAP_Param(UINT apIndex, UINT value, char *param)
{
	AP_INDEX_ASSERT_RC(apIndex, WIFI_HAL_INVALID_ARGUMENTS);

	NULL_PTR_ASSERT_RC(param, WIFI_HAL_INVALID_ARGUMENTS);

	HAL_WIFI_LOG(("%s: %d: %s=%d\n", __FUNCTION__, apIndex, param, value));

	if (!strcmp(param, "eapolkeyretries")) {
		if (value < 1) {
			HAL_WIFI_ERR(("%s: %d: invalid value [%u], allowed range 1..4294967295\n",
				__FUNCTION__, apIndex, value));
			return WIFI_HAL_INVALID_ARGUMENTS;
		}

		if (wldm_AccessPoint_Security_WPAPairwiseRetries(CMD_SET, apIndex,
			&value, NULL, NULL, NULL)) {
			HAL_WIFI_ERR(("%s: %d: wldm_AccessPoint_Security_WPAPairwiseRetries CMD_SET failed\n",
				__FUNCTION__, apIndex));
			return WIFI_HAL_ERROR;
		}
	} else {
		HAL_WIFI_ERR(("%s: %d: %s is unspupported\n", __FUNCTION__, apIndex, param));
		return WIFI_HAL_UNSUPPORTED;
	}

	return WIFI_HAL_SUCCESS;
}

/* TWT hal api */
/* wl twt list gives bcast sechedules and indiv twt links
 * 0..WL_TWT_ID_BCAST_MAX is for broadcast twt schedules
 * In twt_sessions table, use these sessIDs for indiv twt links per radio
 * WLDM_INDIV_IDX_ST..MAX_NUM_TWT_SESSION-1
 */
#define WLDM_INDIV_IDX_ST			(WL_TWT_ID_BCAST_MAX + 1)
#define WLDM_INDIV_IDX_END			(WLDM_INDIV_IDX_ST + MAX_NUM_TWT_SESSION)
#define TWT_SESS_INFO_PTR(apIndex, sessInd)	&(twt_sessions[apIndex][sessInd])

/* save bcast scheduleIDs in use */
static int twt_bcast_sched_idx[HAL_WIFI_TOTAL_NO_OF_APS][WL_TWT_ID_BCAST_MAX + 1] = {0};

/* For individual twt links from associated STA, generate a unique sessionID and save it */
static wldm_twt_sess_info_t twt_sessions[HAL_WIFI_TOTAL_NO_OF_APS][MAX_NUM_TWT_SESSION] = {0};

static int
wl_save_twt_broadcast_ids(int apIndex)
{
	int i;
	char buf[64];
	FILE *fp_twt;

	AP_INDEX_ASSERT(apIndex);
	snprintf(buf, sizeof(buf), "/tmp/ap%d_twtbcast", apIndex);
	if (!(fp_twt = fopen(buf, "w"))) {
		HAL_WIFI_ERR(("Err: %s:%d fopen %s failed\n", __FUNCTION__, __LINE__, buf));
		return WIFI_HAL_ERROR;
	}
	for (i = 0; i <= WL_TWT_ID_BCAST_MAX; i++) {
		fprintf(fp_twt, "%d ", twt_bcast_sched_idx[apIndex][i]);
	}
	fflush(fp_twt);
	fclose(fp_twt);
	return WIFI_HAL_SUCCESS;
}

static int
wl_read_twt_broadcast_ids(int apIndex)
{
	int i;
	char buf[64];
	FILE *fp_twt;

	snprintf(buf, sizeof(buf), "/tmp/ap%d_twtbcast", apIndex);
	if (!(fp_twt = fopen(buf, "r"))) {
		HAL_WIFI_ERR(("Err: %s:%d fopen %s failed\n", __FUNCTION__, __LINE__, buf));
		return WIFI_HAL_ERROR;
	}

	for (i = 0; i <= WL_TWT_ID_BCAST_MAX; i++) {
                if (fscanf(fp_twt, "%d ", &(twt_bcast_sched_idx[apIndex][i])) == 0) {
			HAL_WIFI_ERR(("%s fscanf failed\n", __FUNCTION__));
                        fflush(fp_twt);
                        fclose(fp_twt);
                        return WIFI_HAL_ERROR;
                }
	}
	fflush(fp_twt);
	fclose(fp_twt);
	return WIFI_HAL_SUCCESS;
}

static int
wl_get_new_bcastID(int apIndex)
{
	int i, bcastId = -1;

	/* bcastId 0 is not used for now */
	for (i = 1; i <= WL_TWT_ID_BCAST_MAX; i++) {
		if (twt_bcast_sched_idx[apIndex][i] == 0) {
			twt_bcast_sched_idx[apIndex][i] = i;
			bcastId = i;
			break;
		}
	}
	if (wl_save_twt_broadcast_ids(apIndex) != 0) {
		return WIFI_HAL_ERROR;
	}
	return bcastId;
}

static int
wl_clear_bcastID(int apIndex, int bcastId) {
	if ((bcastId > 0) && (bcastId <= WL_TWT_ID_BCAST_MAX)) {
		twt_bcast_sched_idx[apIndex][bcastId] = 0;
	}
	if (wl_save_twt_broadcast_ids(apIndex) != 0) {
		return WIFI_HAL_ERROR;
	}
	return WIFI_HAL_SUCCESS;
}

static int
wl_save_twt_sessions_table(int apIndex)
{
	int i;
	char buf[64];
	FILE *fp_twt;
	wldm_twt_sess_info_t *sessp;

	AP_INDEX_ASSERT(apIndex);
	snprintf(buf, sizeof(buf), "/tmp/ap%d_twtsess", apIndex);
	if (!(fp_twt = fopen(buf, "w"))) {
		HAL_WIFI_ERR(("Err: %s:%d fopen %s failed\n", __FUNCTION__, __LINE__, buf));
		return WIFI_HAL_ERROR;
	}

	for (i = 0; i < MAX_NUM_TWT_SESSION; i++) {
		/* i peer twtId agtype sessId */
		sessp = TWT_SESS_INFO_PTR(apIndex, i);
		fprintf(fp_twt, "%d %02x %02x %02x %02x %02x %02x %d %d %d\n", i,
			sessp->peer.octet[0], sessp->peer.octet[1], sessp->peer.octet[2],
			sessp->peer.octet[3], sessp->peer.octet[4], sessp->peer.octet[5],
			sessp->twtId, sessp->agtype, sessp->sessId);
	}
	fflush(fp_twt);
	fclose(fp_twt);
	return WIFI_HAL_SUCCESS;
}

static int
wl_read_twt_sessions_table(int apIndex)
{
	int i, k;
	char buf[64];
	FILE *fp_twt;
	wldm_twt_sess_info_t *sessp;

	snprintf(buf, sizeof(buf), "/tmp/ap%d_twtsess", apIndex);
	if (!(fp_twt = fopen(buf, "r"))) {
		// HAL_WIFI_ERR(("Err: %s:%d fopen %s failed\n", __FUNCTION__, __LINE__, buf));
		return WIFI_HAL_ERROR;
	}

	for (i = 0; i < MAX_NUM_TWT_SESSION; i++) {
		// i peer twtId agtype sessId
		sessp = TWT_SESS_INFO_PTR(apIndex, i);
                if (fscanf(fp_twt, "%d %02x %02x %02x %02x %02x %02x %d %d %d\n",
                        &k, (unsigned int *)&sessp->peer.octet[0],
				(unsigned int *)&sessp->peer.octet[1],
				(unsigned int *)&sessp->peer.octet[2],
				(unsigned int *)&sessp->peer.octet[3],
				(unsigned int *)&sessp->peer.octet[4],
				(unsigned int *)&sessp->peer.octet[5],
				&sessp->twtId, &sessp->agtype, &sessp->sessId) == 0) {
			HAL_WIFI_ERR(("%s fscanf failed\n", __FUNCTION__));
			fflush(fp_twt);
			fclose(fp_twt);
			return WIFI_HAL_ERROR;
		}
	}
	fflush(fp_twt);
	fclose(fp_twt);
	return WIFI_HAL_SUCCESS;
}

/* get indx into twt_sessions table for given sessId; sessId = 0 gives index to a free entry */
static int
wl_get_twt_sess_indx_indiv(int apIndex, int sessId, int *sessInd)
{
	int i;
	wldm_twt_sess_info_t *sessp;

	AP_INDEX_ASSERT(apIndex);
	for (i = 0; i < MAX_NUM_TWT_SESSION; i++) {
		sessp = TWT_SESS_INFO_PTR(apIndex, i);
		if (sessp->sessId == sessId) {
			/* found */
			*sessInd = i;
			return 0;
		}
	} /* for */
	*sessInd = -1;
	return -1;
}

/* get sessId in entry matching given mac, flowId and agtype from twt_sessions table */
static int
wl_get_twt_sessId(int apIndex, struct ether_addr *peer, unsigned int twtId, unsigned int agtype,
	int *sessId)
{
	int i;
	wldm_twt_sess_info_t *sessp;

	AP_INDEX_ASSERT(apIndex);
	for (i = 0; i < MAX_NUM_TWT_SESSION; i++) {
		sessp = TWT_SESS_INFO_PTR(apIndex, i);
		if ((!memcmp((void *)(&sessp->peer), (void *)(peer), WIFI_MAC_ADDRESS_LENGTH)) &&
			(sessp->twtId == twtId) &&
			(sessp->agtype == agtype)) {
			/* found */
			*sessId = sessp->sessId;
			return 0;
		}
	} /* for */
	return -1;
}

static void
wl_twt_sdesc_to_wifi_twt_params(wldm_twt_sdesc_t *dp, wifi_twt_params_t *tparamp)
{
	wifi_twt_individual_params_t *tpi;
	wifi_twt_broadcast_params_t *tpb;
	unsigned int wake_dur_unit = 256;

#if (WL_TWT_LIST_VER >= 2)
	wake_dur_unit = dp->wake_duration_unit ?  1024 : 256;
#endif /* WL_TWT_LIST_VER */

	tparamp->agreement = (dp->flow_flags & WL_TWT_FLOW_FLAG_BROADCAST) ?
		wifi_twt_agreement_type_broadcast : wifi_twt_agreement_type_individual;
	tparamp->operation.implicit = (dp->flow_flags & WL_TWT_FLOW_FLAG_IMPLICIT) ? 1 : 0;
	tparamp->operation.announced = (dp->flow_flags & WL_TWT_FLOW_FLAG_UNANNOUNCED) ? 0 : 1;
	tparamp->operation.trigger_enabled = (dp->flow_flags & WL_TWT_FLOW_FLAG_TRIGGER) ? 1 : 0;
	tparamp->operation.flowID = dp->id;
	if (tparamp->agreement == wifi_twt_agreement_type_individual) {
		tpi = &(tparamp->params.individual);
		/* In TWT element from Draft P802.11ax_D7.0 wake_duration_unit is
		 * 0 for 256usec and 1 for 1024 (TU)
		 */
		tpi->wakeTime_uSec = wake_dur_unit * dp->wake_duration;
		tpi->wakeInterval_uSec = dp->wake_interval_mantissa * (1 << dp->wake_interval_exponent);
		tpi->minWakeDuration_uSec = 0;
		tpi->channel = dp->channel;
	}
	else if (tparamp->agreement == wifi_twt_agreement_type_broadcast) {
		/* TBD */
		tpb = &(tparamp->params.broadcast);
		tpb->wakeDuration_uSec= 0;
		tpb->wakeInterval_uSec= 0;
	}
}

/* add 4 for the version and length fields not included in length here */
#define TO_NEXT_TWT_LIST(tlist) tlist = (wldm_twt_list_t *)((char *)tlist + tlist->length + 4)
INT wifi_getTWTsessions(INT ap_index, UINT maxNumberSessions, wifi_twt_sessions_t *twtSessions,
	UINT *numSessionReturned)
{
	int ret, i, num, cnt, sessId = 0, tall_len, dcnt;
	wldm_twt_list_all_t *tallp;
	wldm_twt_list_t *tlist, *totlp;
	wldm_twt_sdesc_t *dp;
	wifi_twt_sessions_t *tsessp = twtSessions;
	wifi_twt_params_t *tparamp;
	wldm_twt_sess_info_t sessInfo, *tblsessp;

	AP_INDEX_ASSERT(ap_index);
	NULL_PTR_ASSERT(twtSessions);
	NULL_PTR_ASSERT(numSessionReturned);
	AX_CAPABLE_ASSERT(HAL_AP_IDX_TO_HAL_RADIO(ap_index));

	/* read in table for indiv twt info */
	ret = wl_read_twt_sessions_table(ap_index);
	if (ret < 0) {
		/* first time */
		HAL_WIFI_DBG(("%s: ap_index=%d: wl_read_twt_sessions_table failed\n",
			__FUNCTION__, ap_index));
	}

	tall_len = sizeof(wldm_twt_list_all_t) + sizeof(wldm_twt_list_t) + MAX_NUM_TWT_SESSION * (sizeof(wldm_twt_sdesc_t));
	tallp = (wldm_twt_list_all_t *)(malloc(tall_len));
	if (tallp == NULL) {
		HAL_WIFI_ERR(("%s: ap_index=%d: alloc error\n", __FUNCTION__, ap_index));
		return WIFI_HAL_ERROR;
	}
	memset((char *)tallp, 0, tall_len);

	ret = wldm_11ax_twt(CMD_GET, ap_index, (void *)(tallp), &tall_len, NULL, NULL);
	if (ret < 0) {
		HAL_WIFI_ERR(("%s: ap_index=%d: wl_getHE_TWTSessions falied\n",  __FUNCTION__,
			ap_index));
		free(tallp);
		return WIFI_HAL_ERROR;
	}

	*numSessionReturned = 0;
	totlp = tallp->tlist;
	tlist = totlp;
	/* for each associated mac list twt sessions */
	for (dcnt = 0; dcnt < tallp->devcnt; dcnt++) {
		/* skip bcast schedule; only list active individual TWT schedule
		 * returned data has configured bcast schedules followed by active schedules
		 */
		if (tlist->indv_count == 0) {
			TO_NEXT_TWT_LIST(tlist);
			continue;
		}
		num = tlist->bcast_count + tlist->indv_count;
		dp = &tlist->desc[tlist->bcast_count];
		for (cnt = 0, i = tlist->bcast_count; i < num; i++, dp++, cnt++) {
			/* copy to wifi_twt_sessions_t */
			memset((char *)(tsessp), 0, sizeof(*tsessp));
			sessInfo.peer = tlist->peer;
			sessInfo.twtId = dp->id;
			sessInfo.agtype = (dp->flow_flags & WL_TWT_FLOW_FLAG_BROADCAST) ?
				wifi_twt_agreement_type_broadcast : wifi_twt_agreement_type_individual;
			/* update IdTWTsession */
			if (wl_get_twt_sessId(ap_index, &(sessInfo.peer), sessInfo.twtId,
				sessInfo.agtype, &sessId) != 0) {
				/* not already there; find an available entry */
				if (wl_get_twt_sess_indx_indiv(ap_index, 0, &sessId) == 0) {
					tblsessp = TWT_SESS_INFO_PTR(ap_index, sessId);
					memcpy((void *)(tblsessp), (void *)(&sessInfo), sizeof(*tblsessp));
					sessId += WLDM_INDIV_IDX_ST; /* to make a unique IdTWT */
					tblsessp->sessId = sessId;
				}
				else {
					HAL_WIFI_ERR(("%s: ap_index=%d: no more free twt table entry\n",
						__FUNCTION__, ap_index));
					free(tallp);
					return WIFI_HAL_ERROR;
				}
			}
			tsessp->IdTWTsession = sessId;
			tsessp->numDevicesInSession = 1; /* we only support one per sta */
			memcpy((void *)(&(tsessp->macAddr[0])), (void *)(&(tlist->peer)), sizeof(tsessp->macAddr));
			tparamp = &(tsessp->twtParameters);
			wl_twt_sdesc_to_wifi_twt_params(dp, tparamp);
			/* TBD - sessionPaused - always 0 now */

		} /* for */
		TO_NEXT_TWT_LIST(tlist);
		*numSessionReturned += 1;
		tsessp++;
	}

	HAL_WIFI_DBG(("%s: ap_index=%d maxNumberSessions=%d num=%d\n", __FUNCTION__, ap_index,
		maxNumberSessions, *numSessionReturned));

	free(tallp);

	wl_save_twt_sessions_table(ap_index);

	return WIFI_HAL_SUCCESS;
}

INT wifi_setBroadcastTWTSchedule(INT ap_index , wifi_twt_params_t twtParams, BOOL create, INT *sessionID)
{
	int ret, bcastId;
	wldm_twt_setup_info_t tsi;
	wldm_twt_cmd_info_t tc = {0};

	AP_INDEX_ASSERT(ap_index);
	NULL_PTR_ASSERT(sessionID);
	AX_CAPABLE_ASSERT(HAL_AP_IDX_TO_HAL_RADIO(ap_index));

	if (!create) {
		/* update existing broadcast schedule - not supported yet */
		bcastId = *sessionID;
	}
	else {
		ret = wl_read_twt_broadcast_ids(ap_index);
		if (ret < 0) {
			/* first time */
			HAL_WIFI_DBG(("%s: ap_index=%d: wl_read_twt_broadcast_ids failed\n",
				__FUNCTION__, ap_index));
		}
		bcastId = wl_get_new_bcastID(ap_index);
		*sessionID = bcastId;
		HAL_WIFI_DBG(("%s: ap_index=%d: wl_get_new_bcastID %d\n", __FUNCTION__, ap_index, *sessionID));
	}
	if ((bcastId <= 0) || (bcastId > WL_TWT_ID_BCAST_MAX)) {
		HAL_WIFI_ERR(("%s: ap_index=%d: invalid bcastId %d\n",
			__FUNCTION__, ap_index, bcastId));
		return WIFI_HAL_ERROR;
	}
	tc.twtCmd = WLDM_TWT_CMD_SETUP;
	memcpy((void *)(&tsi.tparams), (void *)(&twtParams), sizeof(tsi.tparams));
	tsi.create = create;
	tsi.sessId = bcastId;

	memcpy((void *)(tc.twtCmdInfo), (void *)(&tsi), sizeof(tsi));
	ret = wldm_11ax_twt(CMD_SET_IOCTL, ap_index, (void *)(&tc), NULL, NULL, NULL);
	if (ret < 0) {
		HAL_WIFI_ERR(("%s: ap_index=%d: wldm_11ax_twt WLDM_TWT_CMD_SETUP %d falied\n",
			__FUNCTION__, ap_index, *sessionID));
		return WIFI_HAL_ERROR;
	}
	return WIFI_HAL_SUCCESS;
}

INT wifi_setTeardownTWTSession(INT ap_index,  INT sessionID)
{
	wldm_twt_cmd_info_t tc;
	wldm_twt_sess_info_t *tsess, tdSess = {0};
	int ret;

	AP_INDEX_ASSERT(ap_index);
	AX_CAPABLE_ASSERT(HAL_AP_IDX_TO_HAL_RADIO(ap_index));

	if ((sessionID <= 0) || (sessionID >= WLDM_INDIV_IDX_END)) {
		HAL_WIFI_ERR(("%s: ap_index=%d: Invalid sessID %d\n",
			 __FUNCTION__, ap_index, sessionID));
		return WIFI_HAL_ERROR;
	}

	if (sessionID < WLDM_INDIV_IDX_ST) {
		/* read in table for bcast ids */
		ret = wl_read_twt_broadcast_ids(ap_index);
		if (ret < 0) {
			HAL_WIFI_ERR(("%s: ap_index=%d: wl_read_twt_broadcast_ids %d failed\n",
				__FUNCTION__, ap_index, sessionID));
			return WIFI_HAL_ERROR;
		}
	}
	else {
		/* read in table for indiv twt info */
		ret = wl_read_twt_sessions_table(ap_index);
		if (ret < 0) {
			HAL_WIFI_ERR(("%s: ap_index=%d: wl_read_twt_sessions_table %d failed\n",
				__FUNCTION__, ap_index, sessionID));
			return WIFI_HAL_ERROR;
		}
	}

	if (sessionID < WLDM_INDIV_IDX_ST) {
		/* bcast schedule teardown */
		tdSess.agtype = wifi_twt_agreement_type_broadcast;
		tdSess.twtId = sessionID;
		memcpy((void *)(tc.twtCmdInfo), (void *)(&tdSess), sizeof(tdSess));
		wl_clear_bcastID(ap_index, sessionID);
	}
	else {
		int sidx = sessionID;
		sidx -= WLDM_INDIV_IDX_ST;
		tsess = TWT_SESS_INFO_PTR(ap_index, sidx);
		if (tsess->sessId != sessionID) {
			HAL_WIFI_ERR(("%s: ap_index=%d: wl_get_twt_sess_indx_indiv %d failed\n",
				__FUNCTION__, ap_index, sessionID));
			return WIFI_HAL_ERROR;
		}
		memcpy((void *)(tc.twtCmdInfo), (void *)(tsess), sizeof(*tsess));
		/* clear entry in table */
		memset((void *)(tsess), 0, sizeof(*tsess));
		wl_save_twt_sessions_table(ap_index);

	}

	tc.twtCmd = WLDM_TWT_CMD_TEARDOWN;
	ret = wldm_11ax_twt(CMD_SET_IOCTL, ap_index, (void *)(&tc), NULL, NULL, NULL);
	if (ret < 0) {
		HAL_WIFI_ERR(("%s: ap_index=%d: wldm_11ax_twt WLDM_TWT_CMD_TEARDOWN %d falied\n",
			__FUNCTION__, ap_index, sessionID));
		return WIFI_HAL_ERROR;
	}
	return WIFI_HAL_SUCCESS;
}

#endif /* WIFI_HAL_VERSION_3 */

#ifdef RDKB_LGI
INT wifi_getRADIUSAcctEnable(INT apIndex, BOOL *enable)
{
	int ret, len;

	HAL_WIFI_DBG(("%s: Enter, apIndex %d\n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(enable);

	len = sizeof(*enable);
	ret = wldm_AccessPoint_Accounting_Enable(CMD_GET, apIndex,
							enable, &len, NULL, NULL);
	if (ret != 0) {
		HAL_WIFI_ERR(("%s: wldm_AccessPoint_Accounting_Enable CMD_GET Failed,"
				"Status = %d\n",  __FUNCTION__, ret));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: apIndex = %d, enable = %d\n", __FUNCTION__,
			apIndex, *enable));

	return RETURN_OK;
}

INT wifi_setRADIUSAcctEnable(INT apIndex, BOOL enable)
{
	int ret, len = sizeof(enable);

	HAL_WIFI_LOG(("%s: Enter, apIndex %d\n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);

	ret = wldm_AccessPoint_Accounting_Enable(CMD_SET, apIndex,
							&enable, &len, NULL, NULL);
	if (ret != 0) {
		HAL_WIFI_ERR(("%s: wldm_AccessPoint_Accounting_Enable CMD_SET Failed,"
				"Status = %d\n",  __FUNCTION__, ret));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

INT wifi_getApSecurityAcctServer(INT apIndex, CHAR *IP_output, UINT *Port_output, CHAR *AcctSecret_output)
{
	int len;

	HAL_WIFI_DBG(("%s: apIndex = %d\n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);

	if (NULL == IP_output || NULL == Port_output || NULL == AcctSecret_output) {
		HAL_WIFI_ERR(("%s: Parameter Error!!!\n", __FUNCTION__));
		return RETURN_ERR;
	}

	len = OUTPUT_STRING_LENGTH_64;
	if (wldm_AccessPoint_Accounting_ServerIPAddr(CMD_GET, apIndex,
							IP_output, &len, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: Fail to get IP\n", __FUNCTION__));
		return RETURN_ERR;
	}

	len = sizeof(*Port_output);
	if (wldm_AccessPoint_Accounting_ServerPort(CMD_GET, apIndex,
							Port_output, &len, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: Fail to get port\n", __FUNCTION__));
		return RETURN_ERR;
	}

	len = OUTPUT_STRING_LENGTH_64;
	if (wldm_AccessPoint_Accounting_Secret(CMD_GET, apIndex,
							AcctSecret_output, &len, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: Fail to get secret\n", __FUNCTION__));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

INT wifi_setApSecurityAcctServer(INT apIndex, CHAR *IPAddress, UINT port, CHAR *AcctSecret)
{
	int len;

	HAL_WIFI_LOG(("%s: apIndex = %d\n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);

	if (NULL == IPAddress || NULL == AcctSecret) {
		HAL_WIFI_ERR(("%s: Parameter Error!!!\n", __FUNCTION__));
		return RETURN_ERR;
	}

	len = strlen(IPAddress) + 1;
	if (wldm_AccessPoint_Accounting_ServerIPAddr(CMD_SET, apIndex,
							IPAddress, &len, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: Fail to get IP\n", __FUNCTION__));
		return RETURN_ERR;
	}

	len = sizeof(port);
	if (wldm_AccessPoint_Accounting_ServerPort(CMD_SET, apIndex,
							&port, &len, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: Fail to get port\n", __FUNCTION__));
		return RETURN_ERR;
	}

	len = strlen(AcctSecret) + 1;
	if (wldm_AccessPoint_Accounting_Secret(CMD_SET, apIndex,
							AcctSecret, &len, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: Fail to get secret\n", __FUNCTION__));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

INT wifi_getApSecuritySecondaryAcctServer(INT apIndex, CHAR *IP_output, UINT *Port_output, CHAR *AcctSecret_output)
{
	int len;

	HAL_WIFI_DBG(("%s: apIndex = %d\n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);

	if (NULL == IP_output || NULL == Port_output || NULL == AcctSecret_output) {
		HAL_WIFI_ERR(("%s: Parameter Error!!!\n", __FUNCTION__));
		return RETURN_ERR;
	}

	len = OUTPUT_STRING_LENGTH_64;
	if (wldm_AccessPoint_Accounting_SecondaryServerIPAddr(CMD_GET, apIndex,
							IP_output, &len, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: Fail to get IP\n", __FUNCTION__));
		return RETURN_ERR;
	}

	len = sizeof(*Port_output);
	if (wldm_AccessPoint_Accounting_SecondaryServerPort(CMD_GET, apIndex,
							Port_output, &len, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: Fail to get port\n", __FUNCTION__));
		return RETURN_ERR;
	}

	len = OUTPUT_STRING_LENGTH_64;
	if (wldm_AccessPoint_Accounting_SecondarySecret(CMD_GET, apIndex,
							AcctSecret_output, &len, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: Fail to get secret\n", __FUNCTION__));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

INT wifi_setApSecuritySecondaryAcctServer(INT apIndex, CHAR *IPAddress, UINT port, CHAR *AcctSecret)
{
	int len;

	HAL_WIFI_LOG(("%s: apIndex = %d\n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);

	if (NULL == IPAddress || NULL == AcctSecret) {
		HAL_WIFI_ERR(("%s: %d Error!!!\n", __FUNCTION__, __LINE__));
		return RETURN_ERR;
	}

	len = strlen(IPAddress) + 1;
	if (wldm_AccessPoint_Accounting_SecondaryServerIPAddr(CMD_SET, apIndex,
							IPAddress, &len, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: Fail to get IP\n", __FUNCTION__));
		return RETURN_ERR;
	}

	len = sizeof(port);
	if (wldm_AccessPoint_Accounting_SecondaryServerPort(CMD_SET, apIndex,
							&port, &len, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: Fail to get port\n", __FUNCTION__));
		return RETURN_ERR;
	}

	len = strlen(AcctSecret) + 1;
	if (wldm_AccessPoint_Accounting_SecondarySecret(CMD_SET, apIndex,
							AcctSecret, &len, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: Fail to get secret\n", __FUNCTION__));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

INT wifi_getApSecurityAcctInterimInterval(INT apIndex, UINT *Interval_output)
{
	int ret, len;

	HAL_WIFI_DBG(("%s: Enter, apIndex %d\n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);

	if (NULL == Interval_output) {
		HAL_WIFI_ERR(("%s: Interval_ouput parameter error!!!\n", __FUNCTION__));
		return RETURN_ERR;
	}

	len = sizeof(*Interval_output);
	ret = wldm_AccessPoint_Accounting_InterimInterval(CMD_GET, apIndex,
							Interval_output, &len, NULL, NULL);
	if (ret != 0) {
		HAL_WIFI_ERR(("%s: wldm_AccessPoint_Accounting_InterimInterval CMD_GET Failed,"
				"Status = %d\n",  __FUNCTION__, ret));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: apIndex = %d, Interval_output = %d\n", __FUNCTION__,
			apIndex, *Interval_output));

	return RETURN_OK;
}

INT wifi_setApSecurityAcctInterimInterval(INT apIndex, UINT NewInterval)
{
	int ret, len;

	HAL_WIFI_LOG(("%s: Enter, apIndex %d\n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);

	len = sizeof(NewInterval);
	ret = wldm_AccessPoint_Accounting_InterimInterval(CMD_SET, apIndex,
							&NewInterval, &len, NULL, NULL);
	if (ret != 0) {
		HAL_WIFI_ERR(("%s: wldm_AccessPoint_Accounting_InterimInterval CMD_SET Failed,"
				"Status = %d\n",  __FUNCTION__, ret));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

/**
 * @parameter RadiusInterface: 0/1 (wan0/eRouter0)
 */
INT wifi_setApRadiusTransportInterface(UINT RadiusInterface)
{
	INT i;

	HAL_WIFI_LOG(("%s: Enter, RadiusInterface %d\n", __FUNCTION__, RadiusInterface));

	switch (RadiusInterface) {
		case 0:
		/*
		 * Bind RADIUS packets to local IP address of privbr:cmnat,
		 * NAT rule is added in kernel to forward RADIUS packets from
		 * RG to CM, so RADIUS packets will be sent through CM WAN port.
		 */
		nvram_set("radius_client_ifname", "privbr:cmnat");
		break;
		case 1:
		/*
		 * By default, kernel selects eRouter0 to send RADIUS packets.
		 * We don't need to configure radius_client_ifname.
		 */
		nvram_unset("radius_client_ifname");
		break;
		default:
		HAL_WIFI_ERR(("%s: wrong RadiusInterface %d\n", __FUNCTION__, RadiusInterface));
		return RETURN_ERR;
	}
	nvram_commit();

	/* Restart hostapds to make the change take effect. */
	for (i = 0; i < HAL_GET_MAX_RADIOS; ++i) {
		wldm_stop_wsec_daemons(i);
		wldm_start_wsec_daemons(i)
	}
	return RETURN_OK;
}

INT wifi_getApRadiusReAuthInterval(INT apIndex, UINT *interval)
{
	int ret, len;

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(interval);

	HAL_WIFI_DBG(("%s: apIndex %d\n", __FUNCTION__, apIndex));

	len = sizeof(*interval);
	ret = wldm_AccessPoint_Security_RadiusReAuthInterval(CMD_GET, apIndex,
							interval, &len, NULL, NULL);
	if (ret != 0) {
		HAL_WIFI_ERR(("%s: wldm_AccessPoint_Security_RadiusReAuthInterval CMD_GET Failed,"
				"Status=%d\n",  __FUNCTION__, ret));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: apIndex=%d, interval=%d\n", __FUNCTION__,
			apIndex, *interval));

	return RETURN_OK;
}

INT wifi_setApRadiusReAuthInterval(INT apIndex, UINT interval)
{
	int ret, len;

	AP_INDEX_ASSERT(apIndex);

	HAL_WIFI_LOG(("%s: apIndex=%d, interval=%d\n", __FUNCTION__, apIndex, interval));

	len = sizeof(interval);
	ret = wldm_AccessPoint_Security_RadiusReAuthInterval(CMD_SET, apIndex,
							&interval, &len, NULL, NULL);
	if (ret != 0) {
		HAL_WIFI_ERR(("%s: wldm_AccessPoint_Security_RadiusReAuthInterval CMD_SET Failed,"
				"Status=%d\n",  __FUNCTION__, ret));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

INT wifi_getRadiusOperatorName(INT index, CHAR *op_name)
{
	INT radioIndex, len;

	AP_INDEX_ASSERT(index);
	NULL_PTR_ASSERT(op_name);
	radioIndex = HAL_AP_IDX_TO_HAL_RADIO(index);

	/* max len of operator name is 32,
	 * argument op_name should be at least 33 bytes in size,
	 * to include the NUL byte at the end */
	len = 33;
	if (wldm_AccessPoint_Security_RadiusOperatorName(CMD_GET, radioIndex,
		op_name, &len, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: wldm_AccessPoint_Security_RadiusOperatorName failed\n", __FUNCTION__));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: index=%d, op_name=%s\n", __FUNCTION__, index, op_name));
	return RETURN_OK;
}

INT wifi_setRadiusOperatorName(INT band, CHAR *op_name)
{
	INT maxLen = OUTPUT_STRING_LENGTH_32 + 1, len;

	RADIO_INDEX_ASSERT(band);
	NULL_PTR_ASSERT(op_name);

	HAL_WIFI_LOG(("%s: radioIndex=%d, op_name=%s\n", __FUNCTION__, band, op_name));

	len = strlen(op_name) + 1;
	if (len > maxLen) {
		HAL_WIFI_ERR(("%s: op_name len [%d] exceeds max [%d]\n", __FUNCTION__, len, maxLen));
		return RETURN_ERR;
	}

	/* len is actually not used in wldm,
	 * op_name is NUL ended string, its length is derived from strlen */
	if (wldm_AccessPoint_Security_RadiusOperatorName(CMD_SET, band,
		op_name, &len, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: wldm_AccessPoint_Security_RadiusOperatorName failed\n", __FUNCTION__));
		return RETURN_ERR;
	} else
		return RETURN_OK;
}

INT wifi_getRadiusLocationData(INT index, CHAR *loc_data)
{
	INT radioIndex, len;

	AP_INDEX_ASSERT(index);
	NULL_PTR_ASSERT(loc_data);
	radioIndex = HAL_AP_IDX_TO_HAL_RADIO(index);

	/* max len of location data is 253,
	 * argument loc_data should be at least 254 bytes in size,
	 * to include the NUL byte at the end */
	len = 254;
	if (wldm_AccessPoint_Security_RadiusLocationData(CMD_GET, radioIndex,
		loc_data, &len, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: wldm_AccessPoint_Security_RadiusLocationData failed\n", __FUNCTION__));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: index=%d, loc_data=%d\n", __FUNCTION__, index, loc_data));
	return RETURN_OK;
}

INT wifi_setRadiusLocationData(INT band, CHAR *loc_data)
{
	INT maxLen = 254, len;

	RADIO_INDEX_ASSERT(band);
	NULL_PTR_ASSERT(loc_data);

	HAL_WIFI_LOG(("%s: radioIndex=%d, loc_data=%s\n", __FUNCTION__, band, loc_data));

	len = strlen(loc_data) + 1;
	if (len > maxLen) {
		HAL_WIFI_ERR(("%s: loc_data len [%d] exceeds max [%d]\n", __FUNCTION__, len, maxLen));
		return RETURN_ERR;
	}

	/* len is actually not used in wldm,
	 * loc_data is NUL ended string, its length is derived from strlen */
	if (wldm_AccessPoint_Security_RadiusLocationData(CMD_SET, band,
		loc_data, &len, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: wldm_AccessPoint_Security_RadiusLocationData failed\n", __FUNCTION__));
		return RETURN_ERR;
	} else
		return RETURN_OK;
}

INT wifi_getApPMKCacheInterval(INT apIndex, UINT *output_uint)
{
	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(output_uint);

	if (wldm_AccessPoint_Security_WPAPMKLifetime(CMD_GET, apIndex, output_uint, NULL, NULL, NULL)) {
		HAL_WIFI_ERR(("%s: wldm_AccessPoint_Security_WPAPMKLifetime CMD_GET failed\n", __FUNCTION__));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

INT wifi_setApPMKCacheInterval(INT apIndex, UINT number)
{
	AP_INDEX_ASSERT(apIndex);

	HAL_WIFI_LOG(("%s: %d, interval=%d\n", __FUNCTION__, apIndex, number));

	if (wldm_AccessPoint_Security_WPAPMKLifetime(CMD_SET, apIndex, &number, NULL, NULL, NULL)) {
		HAL_WIFI_ERR(("%s: wldm_AccessPoint_Security_WPAPMKLifetime CMD_SET failed\n", __FUNCTION__));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

INT wifi_getWpsStatus(INT apIndex, CHAR *output_string)
{
	wldm_wfa_wps_param_t param;

	UNUSED_PARAMETER(apIndex);
	NULL_PTR_ASSERT(output_string);

	param.cmd = WFA_WPS_GET_STATUS;
	param.apIndex = apIndex;
	if (wldm_wfa_wps(&param)) {
		HAL_WIFI_ERR(("%s: wldm_wfa_wps failed\n", __FUNCTION__));
		return RETURN_ERR;
	}

	/* cosa guaranteens output_string is big enough to hold all valid WPS status strings */
	strcpy(output_string, param.param.status);

	return RETURN_OK;
}

INT wifi_getRadioConfiguredChannel(INT radioIndex, ULONG *cfgdChannel)
{
	unsignedInt channel;
	int len;

	HAL_WIFI_DBG(("%s: Enter, radioIndex = %d\n", __FUNCTION__, radioIndex));
	RADIO_INDEX_ASSERT(radioIndex);

	if (wldm_Radio_Channel(CMD_GET_NVRAM, radioIndex, &channel, &len, 0, 0, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: wldm_Radio_Channel CMD_GET_NVRAM failed!\n", __FUNCTION__));
		return RETURN_ERR;
	}

	*cfgdChannel = (ULONG) channel;
	return RETURN_OK;
}

INT wifi_getRadioRunningChannel(INT radioIndex, ULONG *cfgdChannel)
{
	unsignedInt channel;
	int len;

	HAL_WIFI_DBG(("%s: Enter, radioIndex = %d\n", __FUNCTION__, radioIndex));
	RADIO_INDEX_ASSERT(radioIndex);

	if (wldm_Radio_Channel(CMD_GET, radioIndex, &channel, &len, 0, 0, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: wldm_Radio_Channel CMD_GET failed!\n", __FUNCTION__));
		return RETURN_ERR;
	}

	*cfgdChannel = (ULONG) channel;
	return RETURN_OK;
}

/* softblock (ssd) */
INT wifi_getSoftBlockEnable(BOOL *enable)
{
	int len, ret;

	len = sizeof(*enable);
	ret = wldm_xbrcm_ssd(CMD_GET_NVRAM, 0, enable, &len, "softblock_enable", NULL);
	if (ret != 0) {
		HAL_WIFI_ERR(("%s: Error\n", __FUNCTION__));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: Done enable=%d\n", __FUNCTION__, *enable));

	return RETURN_OK;
}

INT wifi_setSoftBlockEnable(BOOL enable)
{
	int len, ret;

	len = sizeof(enable);
	ret = wldm_xbrcm_ssd(CMD_SET_NVRAM, 0, &enable, &len, "softblock_enable", NULL);
	if (ret != 0) {
		HAL_WIFI_ERR(("%s: Error\n", __FUNCTION__));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: Done enable=%d\n", __FUNCTION__, enable));

	return RETURN_OK;
}

INT wifi_clearSoftBlockBlacklist()
{
	int ret, len;

	len = strlen("clear") + 1;
	ret = wldm_xbrcm_ssd(CMD_SET, 0, "clear", &len, "softblock_list", NULL);
	if (ret != 0) {
		HAL_WIFI_ERR(("%s: Error\n", __FUNCTION__));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: Done\n", __FUNCTION__));

	return RETURN_OK;
}

#ifndef MAXMACLIST
#define MAXMACLIST 64
#endif
/* Consider the first argument is apIndex (instead of "band" in wifi_hal.h) */
INT wifi_getSoftBlockBlacklistEntries(INT apIndex, ULONG *num, wifi_softblock_mac_table_t **table)
{
	char time_str[32], mac_str[32], line[128] = {0}, *line_ptr = line, cmd[128], fname[128];
	FILE *fp = NULL;
	size_t size;
	wifi_softblock_mac_table_t *buf_ptr, *tb_ptr;
	int len, count = 0, max_num = MAXMACLIST;
	int radio_index, sub_index, ifidx, bssidx, ignore_index;

	ignore_index = 1; /* per customer request currently */

	if (ignore_index)
		(void)apIndex;
	else
		AP_INDEX_ASSERT(apIndex);

	NULL_PTR_ASSERT(num);
	NULL_PTR_ASSERT(table);

	*num = 0;
	*table = NULL;

	/* create log file SSD_SOFTBLOCK_LIST_FILE */
	len = strlen("list") + 1;
	if (wldm_xbrcm_ssd(CMD_GET, 0, "list", &len, "softblock_list", NULL) != 0) {
		HAL_WIFI_ERR(("%s: Fail to create SoftBlock log file\n",  __FUNCTION__));
		return RETURN_ERR;
	}

	if (ignore_index) {
		snprintf(fname, sizeof(fname), "%s.tmp2", SSD_SOFTBLOCK_LIST_FILE);

		/* convert "ssd_cli -l" output format */
		snprintf(cmd, sizeof(cmd), "cat %s | cut -f1,4 -d\" \" | sort | uniq > %s",
			SSD_SOFTBLOCK_LIST_FILE, fname);

		if (system(cmd) != 0) {
			HAL_WIFI_ERR(("%s: Fail to convert SoftBlock log format <%s>\n",
				__FUNCTION__, cmd));
			return RETURN_ERR;
		}
	} else {
		snprintf(fname, sizeof(fname), "%s", SSD_SOFTBLOCK_LIST_FILE);
		radio_index = HAL_AP_IDX_TO_HAL_RADIO(apIndex);
		sub_index = HAL_AP_IDX_TO_SSID_IDX(apIndex);

		HAL_WIFI_DBG(("%s: apIndex=%d radio_index=%d sub_index=%d max_num=%d\n",
			__FUNCTION__, apIndex, radio_index, sub_index, max_num));
	}

	if ((fp = fopen(fname, "r")) == NULL) {
		HAL_WIFI_ERR(("%s: Fail to open softblock log file <%s>\n", __FUNCTION__, fname));
		return RETURN_ERR;
	}

	/* alloc memory inside API, will be freed by caller */
	buf_ptr = (wifi_softblock_mac_table_t *)malloc(max_num * sizeof(wifi_softblock_mac_table_t));
	if (buf_ptr == NULL) {
		HAL_WIFI_ERR(("%s: Fail to alloc memory for SoftBlock\n",  __FUNCTION__));
		fclose(fp);
		fp = NULL;
		return RETURN_ERR;
	}

	memset(buf_ptr, 0, max_num * sizeof(wifi_softblock_mac_table_t));
	*table = buf_ptr;

	/* parse file SSD_SOFTBLOCK_LIST_FILE */
	size = sizeof(line);
	while (fgets(line, size, fp) != NULL) {
		if (count >= max_num) {
			HAL_WIFI_DBG(("%s: count=%d exceeds max_num=%d\n",
				 __FUNCTION__, count, max_num));
			break;
		}

		if ((sscanf(line_ptr, "%s %d %d %s", time_str, &ifidx, &bssidx, mac_str) != 4) &&
			(sscanf(line_ptr, "%s %s", time_str, mac_str) != 2)) {
			HAL_WIFI_ERR(("%s: fail to scan line_ptr=%s\n",
				__FUNCTION__, line_ptr));
			continue;
		}

		if (!ignore_index) {
			HAL_WIFI_DBG(("%s: time=%s ifidx=%d bssidx=%d mac=%s count=%d\n",
				__FUNCTION__, time_str, ifidx, bssidx, mac_str, count));

			if ((ifidx != radio_index) || (bssidx != sub_index)) {
				continue;
			}
		}

		tb_ptr = &buf_ptr[count];
		HAL_WIFI_DBG(("%s: count=%d tb_ptr=%p\n", __FUNCTION__, count, tb_ptr));
		strncpy(tb_ptr->time, time_str, sizeof(tb_ptr->time));
		strncpy(tb_ptr->mac, mac_str, sizeof(tb_ptr->mac));
		count++;
	} /* while */

	fclose(fp);
	fp = NULL;

	unlink(SSD_SOFTBLOCK_LIST_FILE);
	if (ignore_index) {
		unlink(fname);
	}

	*num = (ULONG)count;
	if (!count) {
		if (buf_ptr)
			free(buf_ptr);
		*table = NULL;
	}

	HAL_WIFI_DBG(("%s: done count=%d\n", __FUNCTION__, count));
	return RETURN_OK;
}

INT wifi_getSupportRatesBitmapControlFeature(BOOL *enable)
{
	int len = sizeof(*enable);

	HAL_WIFI_DBG(("%s: Enter\n", __FUNCTION__));

	if (wldm_RatesBitmapControl_Enable(CMD_GET, enable, &len, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: wldm_RatesBitmapControl_Enable CMD_GET failed\n", __FUNCTION__));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: RateContMapControlFeature is %s\n", __FUNCTION__, (*enable) ? "enabled" : " diabled"));

	return RETURN_OK;
}

INT wifi_setSupportRatesBitmapControlFeature(BOOL enable)
{
	int index, len;
	char rate[OUTPUT_STRING_LENGTH_64] = {0};

	HAL_WIFI_DBG(("%s: Enter\n", __FUNCTION__));

	if (enable) {
		/* Update initial bitmap in nvram based on current rateset per radio */
		for (index = 0; index < HAL_GET_MAX_RADIOS; index++) {
			wifi_getSupportRatesDisableBasicRates(index, rate);
			len = strlen(rate);
			wldm_RatesBitmapControl_BasicRate(CMD_SET_NVRAM, index, rate, &len, NULL, NULL);
			wifi_getSupportRatesDisableSupportedRates(index, rate);
			len = strlen(rate);
			wldm_RatesBitmapControl_SupportedRate(CMD_SET_NVRAM, index, rate, &len, NULL, NULL);
		}
	}

	len = sizeof(enable);
	if (wldm_RatesBitmapControl_Enable(CMD_SET, &enable, &len, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: wldm_RatesBitmapControl_Enable CMD_SET failed\n", __FUNCTION__));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

INT wifi_getSupportRatesDisableBasicRates(INT apIndex, CHAR *rate)
{
	char output[OUTPUT_STRING_LENGTH_32] = {0};
	int len = sizeof(output);

	HAL_WIFI_DBG(("%s: Enter, apIndex %d\n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(rate);

	if (wldm_RatesBitmapControl_BasicRate(CMD_GET, apIndex,
				output, &len, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: wldm_RatesBitmapControl_BasicRate CMD_GET failed\n", __FUNCTION__));
		return RETURN_ERR;
	}

	snprintf(rate, OUTPUT_STRING_LENGTH_32, "%s", output);
	return RETURN_OK;
}

INT wifi_setSupportRatesDisableBasicRates(INT apIndex, CHAR *rate)
{
	int len;
	HAL_WIFI_LOG(("%s: Enter, ssidIndex %d\n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(rate);

	len = strlen(rate);
	if (wldm_RatesBitmapControl_BasicRate(CMD_SET, apIndex,
				rate, &len, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: wldm_RatesBitmapControl_BasicRate CMD_SET failed\n", __FUNCTION__));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

INT wifi_getSupportRatesDisableSupportedRates(INT apIndex, CHAR *rate)
{
	char output[OUTPUT_STRING_LENGTH_64] = {0};
	int len = sizeof(output);

	HAL_WIFI_DBG(("%s: Enter, apIndex %d\n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(rate);

	if (wldm_RatesBitmapControl_SupportedRate(CMD_GET, apIndex,
				output, &len, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: wldm_RatesBitmapControl_SupportedRate CMD_GET failed\n", __FUNCTION__));
		return RETURN_ERR;
	}

	snprintf(rate, OUTPUT_STRING_LENGTH_64, "%s", output);
	return RETURN_OK;
}

INT wifi_setSupportRatesDisableSupportedRates(INT apIndex, CHAR *rate)
{
	int len;
	HAL_WIFI_DBG(("%s: Enter, apIndex %d\n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(rate);

	len = strlen(rate);
	if (wldm_RatesBitmapControl_SupportedRate(CMD_SET, apIndex,
				rate, &len, NULL, NULL) != 0) {
		HAL_WIFI_ERR(("%s: wldm_RatesBitmapControl_SupportedRate CMD_SET_NVRAM_ failed\n", __FUNCTION__));
		return RETURN_ERR;
	}

	return RETURN_OK;
}

INT wifi_getApWmmOgAckPolicy(INT apIndex, BOOL *output)
{
	HAL_WIFI_DBG(("%s: Enter, apIndex %d\n", __FUNCTION__, apIndex));

	AP_INDEX_ASSERT(apIndex);
	NULL_PTR_ASSERT(output);

	*output = TRUE; /* dummy function, we support it now */
	return RETURN_OK;
}

/* Consider the paramter mac is with type mac_addr_str_t
 * typedef char	mac_addr_str_t[18];
 * thus always with format xx:xx:xx:xx:xx:xx
 */
BOOL wifi_api_is_device_associated(int ap_index, char *mac)
{
	char *p = NULL;
	int len = OUTPUT_STRING_LENGTH_2048;
	unsigned char mactmp[6];
	BOOL ret = FALSE;

	AP_INDEX_ASSERT_RC(ap_index, FALSE);

	if (mac == NULL || 6 != parse_macstr(mac, &(mactmp[0]))) {
		HAL_WIFI_ERR(("%s, invalid mac address !!\n", __FUNCTION__));
		return ret;
	}

	p = (char *) malloc(len);
	NULL_PTR_ASSERT_RC(p, FALSE);

	if ((wldm_xbrcm_ap(CMD_GET, ap_index, p, &len, "assoclist", NULL) == 0) &&
		strcasestr(p, mac)) {
		ret = TRUE;
	}

	free(p);
	return ret;
}

// Kick all associated devices
INT wifi_kickAllAssociatedDevice(INT apIndex)
{
	char *p = NULL;
	int ret, assocNum = 0;
	int len = OUTPUT_STRING_LENGTH_2048;
	char *macStr, *sp;

	AP_INDEX_ASSERT(apIndex);

	ret = wifi_getApNumDevicesAssociated(apIndex, &assocNum);
	if (ret != RETURN_OK) {
		HAL_WIFI_DBG(("%s: Failed to get wifi associations\n", __FUNCTION__));
		return RETURN_ERR;
	} else if (assocNum == 0) {
		HAL_WIFI_DBG(("%s: No devices connected on apIndex %d. Kick not required.\n",
			__FUNCTION__, apIndex));
		return RETURN_OK;
	}

	p = (char *) malloc(len);
	NULL_PTR_ASSERT(p);

	if (wldm_xbrcm_ap(CMD_GET, apIndex, p, &len, "assoclist", NULL) != 0) {
		HAL_WIFI_ERR(("%s: wldm_xbrcm_ap - assoclist ERROR\n", __FUNCTION__));
		free(p);
		return RETURN_ERR;
	}

	macStr = strtok_r(p, ",", &sp);
	while (macStr) {
		len = strlen(macStr);
		ret = wldm_AccessPoint_kickAssociatedDevice(CMD_SET_IOCTL, apIndex, macStr, &len,
			NULL, NULL);
		if (ret != RETURN_OK) {
			HAL_WIFI_DBG(("%s: wldm_AccessPoint_kickAssociatedDevice Fail, "
				"apIndex=[%d], mac=MACF", __FUNCTION__, apIndex, macStr));
		}
		macStr = strtok_r(NULL, ",", &sp);
	}

	free(p);
	return RETURN_OK;
}

/* Get DFS channel exclusion in automatic channel selection */
INT wifi_getRadioExcludeDfs(INT radioIndex, BOOL *output_bool)
{
	int returnStatus, len;

	HAL_WIFI_DBG(("%s: Enter, radioIndex = %d\n", __FUNCTION__, radioIndex));

	NULL_PTR_ASSERT(output_bool);
	len = sizeof(*output_bool);
	RADIO_INDEX_ASSERT(radioIndex);
	*output_bool = FALSE;

	if (wldm_xbrcm_acs(CMD_GET_NVRAM, radioIndex, output_bool, &len, "exclude_dfs", NULL) < 0) {
		HAL_WIFI_ERR(("%s: wldm_xbrcm_acs CMD_GET failed.\n", __FUNCTION__));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: radioIndex = %d, output_bool = %d\n", __FUNCTION__, radioIndex, *output_bool));

	return RETURN_OK;
}

/* Set DFS channel exclusion in automatic channel selection */
INT wifi_setRadioExcludeDfs(INT radioIndex, BOOL enable)
{
	int returnStatus, len = sizeof(enable);

	HAL_WIFI_DBG(("%s: Enter, radioIndex = %d\n", __FUNCTION__, radioIndex));

	RADIO_INDEX_ASSERT(radioIndex);
	if (wldm_xbrcm_acs(CMD_SET_IOCTL | CMD_SET_NVRAM, radioIndex, &enable, &len, "exclude_dfs", NULL) < 0) {
		HAL_WIFI_ERR(("%s: wldm_xbrcm_acs CMD_SET failed.\n", __FUNCTION__));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: radioIndex = %d, enable = %d\n", __FUNCTION__, radioIndex, enable));

	return RETURN_OK;
}

#define MAX_NUM_CHAN_WTS_0	3
#define MAX_NUM_CHAN_WTS_1	28
static int ch_list_0[WLDM_MAX_CH_LIST_LEN] = {1, 6, 11, 0};
static int ch_list_1[WLDM_MAX_CH_LIST_LEN] = {36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112,
	116, 120, 124, 128, 132, 136, 140, 144, 149, 153, 157, 161, 165, 169, 173, 177, 0};

/*  Get channel weights to configure channel preference in acsd2 autochannel mode.
 *  For ACSChannelsNumberOfEntries,
 *	Radio 0, ChannelWeights are for channels 1,6,11
 *	Radio 1, ChannelWeights are for channels 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112,
 *	116, 120, 124, 128, 132, 136, 140, 144,149, 153, 157, 161, 165, 169, 173, 177
 *  A channel with a higher weight has more chance of getting selected.
 *  If the weight is set to 0 then the channel is not selected.
 */
INT wifi_getRadioChannelWeights(INT radioIndex, ULONG *output_weights)
{
	int *ch_list;
	int values[WLDM_MAX_CH_LIST_LEN * 2] = {0};
	int i = 0, j, len = sizeof(values);

	HAL_WIFI_DBG(("%s: Enter, radioIndex = %d\n", __FUNCTION__, radioIndex));

	NULL_PTR_ASSERT(output_weights);
	len = sizeof(values);
	RADIO_INDEX_ASSERT(radioIndex);
	/* Assume max 2 radios for now */
	ch_list = (radioIndex == 0) ? ch_list_0 : (radioIndex == 1) ? ch_list_1 : NULL;
	NULL_PTR_ASSERT(ch_list);

	if (wldm_xbrcm_acs(CMD_GET, radioIndex, &values, &len, "channel_weights", NULL) < 0) {
		HAL_WIFI_ERR(("%s: wldm_xbrcm_acs CMD_GET failed.\n", __FUNCTION__));
		return RETURN_ERR;
	}

	if (!len) {
		HAL_WIFI_DBG(("%s: radioIndex = %d, NO output_weights\n", __FUNCTION__,
			radioIndex));
		/* If not output_weights, default values per channel is 100, 1x */
		while (ch_list[i]) {
			output_weights[i++] = 100;
		}
	} else {
		while (ch_list[i]) {
			/* len is number of items in list */
			for (j = 0; j < len ; j++) {
				if (values[j * 2] == ch_list[i]) {
					output_weights[i] = values[j * 2 + 1];
					break;
				}
			}
			i++;
		}
	}

	HAL_WIFI_DBG(("%s: radioIndex = %d, output_weights = %d,%d,%d\n", __FUNCTION__,
		radioIndex, output_weights[0], output_weights[1], output_weights[2]));
	return RETURN_OK;
}

/*  Set channel weights to configure channel preference in acsd2 autochannel mode.
 *  Assumptions for ACSChannelsNumberOfEntries, number of entries in output_weights:
 *	Radio 0, ChannelWeights for channels 1,6,11
 *	ACSChannelsNumberOfEntries = 3
 *	Radio 1, ChannelWeights for channels 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112,
 *	116, 120, 124, 128, 132, 136, 140, 144,149, 153, 157, 161, 165, 169, 173, 177
 *	ACSChannelsNumberOfEntries = 28
 *  A channel with a higher weight has more chance of getting selected.
 *  If channel weight is set to 0 then the channel is not selected.
 *  If channel weight is not specified, default channel weight is 100 or 1x.
 */
INT wifi_setRadioChannelWeights(INT radioIndex, const ULONG *output_weights)
{
	int *ch_list;
	int values[WLDM_MAX_CH_LIST_LEN * 2] = {0};
	int i, num_output_weights, num_chan_weights;

	HAL_WIFI_DBG(("%s: Enter, radioIndex = %d\n", __FUNCTION__, radioIndex));

	NULL_PTR_ASSERT(output_weights);
	RADIO_INDEX_ASSERT(radioIndex);
	/* Assume max 2 radios for now */
	ch_list = (radioIndex == 0) ? ch_list_0 : (radioIndex == 1) ? ch_list_1 : NULL;
	NULL_PTR_ASSERT(ch_list);

	if (radioIndex != 0 && radioIndex != 1) {
		HAL_WIFI_ERR(("%s: radioIndex:%d not supported\n", __FUNCTION__,
			radioIndex));
	} else if ((radioIndex == 0 && num_output_weights > MAX_NUM_CHAN_WTS_0) ||
		(radioIndex == 1 && num_output_weights > MAX_NUM_CHAN_WTS_1)){
		HAL_WIFI_ERR(("%s: invalid number of channel weights:%d radioIndex:%d\n",
			__FUNCTION__, num_output_weights, radioIndex));
	}
	num_output_weights = (radioIndex == 0) ? MAX_NUM_CHAN_WTS_0 : MAX_NUM_CHAN_WTS_1;

	for (i = 0; i < num_output_weights; i++) {
		if (((signed long) output_weights[i] < 0) ||
				((signed long) output_weights[i] > 100)) {
			HAL_WIFI_ERR(("output_weights[%d]:%d out of range\n", __FUNCTION__,
				i, (signed long) output_weights[i]));
			break;
		}
		values[i * 2] = ch_list[i];
		values[i * 2 + 1] = output_weights[i];
	}

	num_chan_weights = num_output_weights * 2;

	if (wldm_xbrcm_acs(CMD_SET_IOCTL | CMD_SET_NVRAM, radioIndex, &values, &num_chan_weights, "channel_weights", NULL) < 0) {
		HAL_WIFI_ERR(("%s: wldm_xbrcm_acs CMD_GET failed.\n", __FUNCTION__));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: radioIndex = %d, output_weights = %d,%d,%d\n", __FUNCTION__,
		radioIndex, output_weights[0], output_weights[1], output_weights[2]));
	return RETURN_OK;
}

/* Check if Zero DFS is supported */
INT wifi_isZeroDFSSupported(UINT radioIndex, BOOL *supported)
{
	int len;

	HAL_WIFI_DBG(("%s: Enter, radioIndex = %d\n", __FUNCTION__, radioIndex));

	NULL_PTR_ASSERT(supported);
	len = sizeof(*supported);
	RADIO_INDEX_ASSERT(radioIndex);
	*supported = FALSE;

	if (wldm_xbrcm_acs(CMD_GET, radioIndex, supported, &len, "zdfs_supp", NULL) < 0) {
		HAL_WIFI_ERR(("%s: wldm_xbrcm_acs CMD_GET failed.\n", __FUNCTION__));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: radioIndex = %d, *supported = %d\n", __FUNCTION__, radioIndex,
		*supported));
	return RETURN_OK;
}

/* Set Zero DFS State */
INT wifi_setZeroDFSState(UINT radioIndex, BOOL enable, BOOL precac)
{
	int value, len = sizeof(value);

	HAL_WIFI_DBG(("%s: Enter, radioIndex = %d\n", __FUNCTION__, radioIndex));

	RADIO_INDEX_ASSERT(radioIndex);

	value = enable ? 1 : 0;
	if (wldm_xbrcm_acs(CMD_SET_NVRAM | CMD_SET_IOCTL, radioIndex, &value, &len, "zdfs_state",
		NULL) < 0) {
		HAL_WIFI_ERR(("%s: wldm_xbrcm_acs CMD_SET_NVRAM, CMD_SET_IOCTL %d failed.\n",
			__FUNCTION__, enable));
		return RETURN_ERR;
	}

	len = sizeof(value);
	value = precac ? 1 : 0;
	if (wldm_xbrcm_acs(CMD_SET_NVRAM | CMD_SET_IOCTL, radioIndex, &value, &len, "zdfs_preclr",
		NULL) < 0) {
		HAL_WIFI_ERR(("%s: wldm_xbrcm_acs CMD_SET_NVRAM, CMD_SET_IOCTL %d failed.\n",
			__FUNCTION__, enable));
		return RETURN_ERR;
	}

	HAL_WIFI_DBG(("%s: radioIndex = %d, enable = %d, precac = %d\n", __FUNCTION__, radioIndex,
		enable, precac));
	return RETURN_OK;
}

/* Get Zero DFS State */
INT wifi_getZeroDFSState(UINT radioIndex, BOOL *enable, BOOL *precac)
{
	int value, len;

	HAL_WIFI_DBG(("%s: Enter, radioIndex = %d\n", __FUNCTION__, radioIndex));

	NULL_PTR_ASSERT(enable);
	NULL_PTR_ASSERT(precac);
	RADIO_INDEX_ASSERT(radioIndex);

	len = sizeof(value);
	*enable = FALSE;
	if (wldm_xbrcm_acs(CMD_GET, radioIndex, &value, &len, "zdfs_state", NULL) < 0) {
		HAL_WIFI_ERR(("%s: wldm_xbrcm_acs CMD_GET failed.\n", __FUNCTION__));
		return RETURN_ERR;
	}
	*enable = value ? TRUE : FALSE;

	len = sizeof(value);
	*precac = FALSE;
	if (wldm_xbrcm_acs(CMD_GET, radioIndex, &value, &len, "zdfs_preclr", NULL) < 0) {
		HAL_WIFI_ERR(("%s: wldm_xbrcm_acs CMD_GET failed.\n", __FUNCTION__));
		return RETURN_ERR;
	}
	*precac = value ? TRUE : FALSE;

	HAL_WIFI_DBG(("%s: radioIndex = %d, *enable = %d *precac = %d\n", __FUNCTION__, radioIndex,
		*enable, *precac));
	return RETURN_OK;
}

INT wifi_getZeroWaitDFSChannelsStatus(UINT radioIndex, wifi_zwdf_list_t **ListOfDFSChannelStatus)
{
	uint num_dfs_channels = 0, len = sizeof(num_dfs_channels), listlen = 0;
	wifi_zwdf_list_t *wifi_dfs_chs_status_list = NULL;

	HAL_WIFI_DBG(("%s: Enter, radioIndex = %d\n", __FUNCTION__, radioIndex));
	RADIO_INDEX_ASSERT(radioIndex);

	if (wldm_11h_dfs(CMD_GET, radioIndex, &num_dfs_channels, &len, "dfs_num_chs", NULL) < 0) {
		HAL_WIFI_ERR(("%s: wldm_11h_dfs dfs_num_chs CMD_GET failed.\n", __FUNCTION__));
		return RETURN_ERR;
	}
	if (!num_dfs_channels) {
		HAL_WIFI_DBG(("%s: no valid DFS channels\n", __FUNCTION__));
		return RETURN_OK;
	}

	/* Allocate 1 extra in list to indicate end of list with parameter channel = 0 */
	listlen = (num_dfs_channels + 1) * sizeof(wifi_zwdf_list_t);
	wifi_dfs_chs_status_list = (wifi_zwdf_list_t *) malloc(listlen);
	if (!wifi_dfs_chs_status_list) {
		HAL_WIFI_ERR(("%s: malloc failed\n", __FUNCTION__));
		return RETURN_ERR;
	}
	memset(wifi_dfs_chs_status_list, 0, listlen);

	if (wldm_11h_dfs(CMD_GET, radioIndex, (void *)wifi_dfs_chs_status_list, &num_dfs_channels,
		"dfs_chs_status", NULL) < 0) {
		HAL_WIFI_ERR(("%s: wldm_11h_dfs dfs_chs_status CMD_GET failed.\n", __FUNCTION__));
		return RETURN_ERR;
	}

	/* Set end of list with channel = 0 */
	wifi_dfs_chs_status_list[num_dfs_channels].Channel = 0;
	wifi_dfs_chs_status_list[num_dfs_channels].Status = 0;
	*ListOfDFSChannelStatus = wifi_dfs_chs_status_list;

	HAL_WIFI_DBG(("%s: radioIndex = %d len = %d\n", __FUNCTION__, radioIndex, len));
	return RETURN_OK;
}

//Get the Current Operating Channel Bandwidth. eg "20MHz", "40MHz", "80MHz", "80+80", "160"
//The output_string is a max length 64 octet string that is allocated by the RDKB code.
//Implementations must ensure that strings are not longer than this.
INT  wifi_getCurrentRadioOperatingChannelBandwidth(int radioIndex, char *output_string,
	unsigned int len)
{
	int ret;

	NULL_PTR_ASSERT(output_string);
	RADIO_INDEX_ASSERT(radioIndex);

	if (len > OUTPUT_STRING_LENGTH_64 * sizeof(char)) {
		HAL_WIFI_ERR(("%s: radioIndex = %d len = %d > max length = %d\n", __FUNCTION__,
			radioIndex, OUTPUT_STRING_LENGTH_64 * sizeof(char)));
		return RETURN_ERR;
	}
	ret = wldm_Radio_OperatingChannelBandwidth(CMD_GET, radioIndex, output_string, (int *)&len,
		NULL, NULL);
	if (ret < 0) {
		HAL_WIFI_ERR(("%s: radioIndex = %d Failed to get CurrentOperatingChannelBandwidth\n",
			__FUNCTION__, radioIndex));
		return RETURN_ERR;
	}
	HAL_WIFI_DBG(("%s, radioIndex = %d output_string = %s len = %d\n", __FUNCTION__,
		radioIndex, output_string, len));
	return RETURN_OK;
}
#endif /* RDKB_LGI */
