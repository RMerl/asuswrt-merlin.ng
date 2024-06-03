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

    module: wifi_api.c

        For CCSP Component:  WiFi Hal test util

    ---------------------------------------------------------------

    copyright:

        Comcast, Corp., 2015
        All Rights Reserved.

    ---------------------------------------------------------------

    description:

        The wifi hal test api wifi_api.

    ---------------------------------------------------------------

    environment:

        This HAL layer is intended to test Wifi drivers
        through an open API.

    ---------------------------------------------------------------

    author:

**********************************************************************/
#if defined(_CBR_PRODUCT_REQ_) && !defined(_CBR2_PRODUCT_REQ_)
#define _CBR1_PRODUCT_REQ_
#endif /* _CBR_PRODUCT_REQ_ && !_CBR2_PRODUCT_REQ_ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <inttypes.h>
#include <ctype.h>
#include "typedefs.h"
#include "wifi_hal.h"
#include "wifi_hal_defs.h"
#include <wlcsm_defs.h>

#if WIFI_HAL_VERSION_GE_2_12
#pragma message "wifi_hal.h version is >= 2.12"
#else
#ifndef MAX_BTM_DEVICES
#pragma message "wifi_hal.h not define MAX_BTM_DEVICES"
#include "wifi_hal_btm.h"
#else
#pragma message "wifi_hal.h defined MAX_BTM_DEVICES"
#endif /* MAX_BTM_DEVICES */
#endif /* WIFI_HAL_VERSION_GE_2_12 */

#define MAX_OUTPUT_STRING_LEN_1024   1024
#define WIFI_API_EVENT_UDP_SPORT  55010
#define WIFI_API_EVENT_UDP_SIP  "127.0.0.1"
#ifndef MAXMACLIST
#define MAXMACLIST 64 /* Sync with wifi_hal.c */
#endif
#define MAC_STR_LEN 18
#define CHCOUNT2 11
#define CHCOUNT5 25
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

INT wifi_getRadioWifiTrafficStats(INT radioIndex, wifi_radioTrafficStats_t *output_struct);
extern INT wifi_apply(void);
extern INT wifi_getIndexFromName(CHAR *inputSsidString, INT *output_int);
extern INT wifi_initRadio(INT radioIndex);
extern int wifi_factoryReset_post(int index, int commit, int restart);
extern INT wifi_getApTxBeaconFrameCount(INT apIndex, UINT *count);
extern INT wifi_setRadioAutoChannelRefreshPeriod(INT radioIndex, ULONG seconds);
extern INT wifi_setWldmMsglevel(unsigned long msglevel);
extern INT wifi_allow2G80211ax(BOOL enable);
extern INT wifi_getAllow2G80211ax(BOOL *enable);

extern int get_hex_data(unsigned char *data_str, unsigned char *hex_data, int len);

#if defined(BUILD_RDKWIFI) && !defined(BCA_CPEROUTER_RDK)
extern pthread_t wifi_apiThread;
#endif /* defined(BUILD_RDKWIFI) && !defined(BCA_CPEROUTER_RDK) */

BOOL print_compact = FALSE;
#define PRINT_FMT(NAME, TAG, FMT, VALUE) \
	do {\
		if (print_compact) { \
			char tmp[256]; \
			snprintf(tmp, sizeof(tmp), FMT, VALUE); \
			printf("%-4s ", tmp); \
		} else { \
			printf("  %-30s %s = ", NAME, TAG); \
			printf(FMT, VALUE); \
			printf("\n"); \
		} \
	} while (0)

#define PRINT_STR(A, B) PRINT_FMT(#B, "(str)", "'%s'", A->B)

#define PRINT_INT2(N, V) \
	do { \
		if (sizeof(V) == sizeof(int64_t)) { \
			if ( (typeof(V))-1 < 1 ) { \
				PRINT_FMT(N, "(i64)", "%"PRId64, (int64_t)V); \
			} else { \
				PRINT_FMT(N, "(u64)", "%"PRIu64, (uint64_t)V); \
			} \
		} else { \
			if ( (typeof(V))-1 < 1 ) { \
				PRINT_FMT(N, "(i32)", "%"PRId32, (int32_t)V); \
			} else { \
				PRINT_FMT(N, "(u32)", "%"PRIu32, (uint32_t)V); \
			} \
		} \
		fflush(stdout); \
	} while (0)

#define PRINT_INT1(V) PRINT_INT2(#V, V)

#define PRINT_INT(A, B) PRINT_INT2(#B, A->B)

#define PRINT_HEX2(N, V) \
	do { \
		if (sizeof(V) == sizeof(int64_t)) { \
			PRINT_FMT(N, "(x64)", "0x%"PRIx64, (uint64_t)V); \
		} else { \
			PRINT_FMT(N, "(x32)", "0x%"PRIx32, (uint32_t)V); \
		} \
	} while (0)

#define PRINT_HEX1(V) PRINT_HEX2(#V, V)

#define PRINT_HEX(A, B) PRINT_HEX2(#B, A->B)

#define PRINT_DOUBLE(A, B) PRINT_FMT(#B, "(dbl)", "%.2f", A->B);
BOOL opt_compact = TRUE;

#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN 6
#endif

static void
print_neighbor_stats(wifi_neighbor_ap2_t *neighbor_ap_array, int array_size)
{
	int i;
	wifi_neighbor_ap2_t *pt = NULL;
	for (i = 0, pt = neighbor_ap_array; i < array_size; i++, pt++) {
		printf("\nNeighbor:%d\n", i + 1);
		printf("\tap_SSID\t\t\t\t:%s\n", pt->ap_SSID);
		printf("\tap_BSSID\t\t\t:%s\n", pt->ap_BSSID);
		printf("\tap_Mode\t\t\t\t:%s\n", pt->ap_Mode);
		printf("\tap_Channel\t\t\t:%d\n", pt->ap_Channel);
		printf("\tap_SignalStrength\t\t:%d\n",
			pt->ap_SignalStrength);
		printf("\tap_SecurityModeEnabled\t\t:%s\n",
			pt->ap_SecurityModeEnabled);
		printf("\tap_EncryptionMode\t\t:%s\n", pt->ap_EncryptionMode);
		printf("\tap_OperatingFrequencyBand\t:%s\n", pt->ap_OperatingFrequencyBand);
		printf("\tap_SupportedStandards\t\t:%s\n",
			pt->ap_SupportedStandards);
		printf("\tap_OperatingStandards\t\t:%s\n",
			pt->ap_OperatingStandards);
		printf("\tap_OperatingChannelBandwidth\t:%s\n",
			pt->ap_OperatingChannelBandwidth);
		printf("\tap_SecurityModeEnabled\t\t:%s\n",
			pt->ap_SecurityModeEnabled);
		printf("\tap_BeaconPeriod\t\t\t:%d\n",pt->ap_BeaconPeriod);
		printf("\tap_Noise\t\t\t:%d\n",
			pt->ap_Noise);
		printf("\tap_BasicDataTransferRates\t:%s\n",
			pt->ap_BasicDataTransferRates);
		printf("\tap_SupportedDataTransferRates\t:%s\n",
			pt->ap_SupportedDataTransferRates);
		printf("\tap_DTIMPeriod\t\t\t:%d\n", pt->ap_DTIMPeriod);
		printf("\tap_ChannelUtilization\t\t:%d\n",
			pt->ap_ChannelUtilization);
	}

}

#if defined(WIFI_HAL_VERSION_3)
extern wifi_enum_to_str_map_t wps_config_method_table[];
extern wifi_enum_to_str_map_t security_mode_table[];
extern wifi_enum_to_str_map_t encryption_table[];
extern wifi_enum_to_str_map_t cipher_cap_table[];
extern wifi_enum_to_str_map_t countrycode_table[];
extern INT wifi_getRadioFrequencyBand(wifi_radio_index_t radioIndex, wifi_freq_bands_t *band);

static void
print_bands(wifi_freq_bands_t bands)
{
	printf("bands: 0x%04x", bands);
	if (bands & WIFI_FREQUENCY_2_4_BAND) printf(" 2_4G");
	if (bands & WIFI_FREQUENCY_5_BAND) printf(" 5G");
	if (bands & WIFI_FREQUENCY_6_BAND) printf(" 6G");
	if (bands & WIFI_FREQUENCY_60_BAND) printf(" 60G");
	printf("\n");
}

static void
print_chBandwidths(wifi_channelBandwidth_t chBW)
{
	printf("channelWidth: 0x%04x", chBW);
	if (chBW & WIFI_CHANNELBANDWIDTH_20MHZ) printf(" 20");
	if (chBW & WIFI_CHANNELBANDWIDTH_40MHZ) printf(" 40");
	if (chBW & WIFI_CHANNELBANDWIDTH_80MHZ) printf(" 80");
	if (chBW & WIFI_CHANNELBANDWIDTH_160MHZ) printf(" 160");
	if (chBW & WIFI_CHANNELBANDWIDTH_80_80MHZ) printf(" 80+80");
#ifdef WIFI_HAL_VERSION_GE_3_0_3
	if (chBW & WIFI_CHANNELBANDWIDTH_320MHZ) printf(" 320");
#endif /* WIFI_HAL_VERSION_GE_3_0_3 */
	printf(" MHz\n");
}

static void
print_80211_variants(wifi_ieee80211Variant_t mode)
{
	printf("80211_variants: 0x%04x", mode);
	if (mode & WIFI_80211_VARIANT_A) printf(" a");
	if (mode & WIFI_80211_VARIANT_B) printf(" b");
	if (mode & WIFI_80211_VARIANT_G) printf(" g");
	if (mode & WIFI_80211_VARIANT_N) printf(" n");
	if (mode & WIFI_80211_VARIANT_H) printf(" h");
	if (mode & WIFI_80211_VARIANT_AC) printf(" ac");
	if (mode & WIFI_80211_VARIANT_AD) printf(" ad");
	if (mode & WIFI_80211_VARIANT_AX) printf(" ax");
#ifdef WIFI_HAL_VERSION_GE_3_0_3
	if (mode & WIFI_80211_VARIANT_BE) printf(" be");
#endif /* WIFI_HAL_VERSION_GE_3_0_3 */
	printf("\n");
}

static void
print_bitrates(wifi_bitrate_t bitrate)
{
	printf("BitRates: 0x%04x", bitrate);
	if (bitrate & WIFI_BITRATE_1MBPS) printf(" 1");
	if (bitrate & WIFI_BITRATE_2MBPS) printf(" 2");
	if (bitrate & WIFI_BITRATE_5_5MBPS) printf(" 5.5");
	if (bitrate & WIFI_BITRATE_6MBPS) printf(" 6");
	if (bitrate & WIFI_BITRATE_9MBPS) printf(" 9");
	if (bitrate & WIFI_BITRATE_11MBPS) printf(" 11");
	if (bitrate & WIFI_BITRATE_12MBPS) printf(" 12");
	if (bitrate & WIFI_BITRATE_18MBPS) printf(" 18");
	if (bitrate & WIFI_BITRATE_24MBPS) printf(" 24");
	if (bitrate & WIFI_BITRATE_36MBPS) printf(" 36");
	if (bitrate & WIFI_BITRATE_48MBPS) printf(" 48");
	if (bitrate & WIFI_BITRATE_54MBPS) printf(" 54");
	printf(" Mbps\n");
}

static void
print_chList(wifi_channels_list_t *chList)
{
	int i;
	printf("ChannelList num: %d list:", chList->num_channels);
	for (i = 0; i < chList->num_channels; i++) {
		printf(" %d", chList->channels_list[i]);
	}
	printf("\n");
}

static void
print_onBoardingMethods(wifi_onboarding_methods_t obm)
{
	int i;

	printf("OnBoardingMethods: 0x%04x", obm);
	for (i = 0; wps_config_method_table[i].str_val != NULL; ++i) {
		if (obm & wps_config_method_table[i].enum_val)
			printf(" %s", wps_config_method_table[i].str_val);
	}
}

static void
print_security_modes(wifi_security_modes_t secm)
{
	int i;
	printf("SecurityModes: 0x%04x", secm);
	for (i = 0; security_mode_table[i].str_val != NULL; ++i) {
		if (secm & security_mode_table[i].enum_val)
			printf(" %s", security_mode_table[i].str_val);
	}
}

static void
print_cipher_cap(unsigned int cipher)
{
	int i;
	printf("Cipher Capability: 0x%04x", cipher);
	for (i = 0; cipher_cap_table[i].str_val != NULL; ++i) {
		if (cipher & cipher_cap_table[i].enum_val)
			printf(" %s", cipher_cap_table[i].str_val);
	}
}

static void
print_TWTparams(wifi_twt_params_t *twtp, unsigned int numSess)
{
	unsigned int i;
	wifi_twt_params_t *tp = twtp;
	wifi_twt_individual_params_t *tpi;
	unsigned int wakeInt, wakeDur;

	printf("Num TWT Sessions = %d\n", numSess);
	if ((numSess == 0) || (twtp == NULL))
		return;
	printf("flowID Interval (usec)  Duration (usec)  Unannounced  Trigger  Implicit"
		"     Agreement\n");
	for (i = 0 ; i < numSess; i++, tp++) {
		wakeInt = 0;
		wakeDur = 0;
		if (tp->agreement == wifi_twt_agreement_type_individual) {
			tpi = &(tp->params.individual);
			wakeInt = tpi->wakeInterval_uSec;
			wakeDur = tpi->wakeTime_uSec;
		}
		printf("%2d)     %6d           %6d              %s       %s"
			"         %s          %s\n",
			/* IdTWTsession */ tp->operation.flowID, wakeInt, wakeDur,
			(tp->operation.announced) ? "NO" : "YES",
			(tp->operation.trigger_enabled) ? "YES" : "NO",
			(tp->operation.implicit) ? "YES" : "NO",
			(tp->agreement == wifi_twt_agreement_type_individual) ? "Individual" :
				"Broadcast");
	}
}

static void
print_TWTsessions(wifi_twt_sessions_t *ts, unsigned int numSess)
{
	unsigned int i, j;
	wifi_twt_sessions_t *tsp = ts;
	wifi_twt_params_t *tp;
	wifi_twt_individual_params_t *tpi;
	unsigned int wakeInt, wakeDur;
	printf("TWT numSessions = %d\n", numSess);
	for (j = 0; j < numSess; j++, tsp++) {
		/* current support is 1 indiv twt link per sta
		 * future may have multiple stas on a bcast twt schedule
		 */
		printf("Num twt Links = %d", tsp->numDevicesInSession);
		for (i = 0; i < tsp->numDevicesInSession; i++) {
			printf(" "MACF"", MAC_TO_MACF(tsp->macAddr[i]));
		}
		printf("\nID    Interval (usec)  Duration (usec)  Unannounced  Trigger  Implicit"
			"     Agreement\n");
		tp = (wifi_twt_params_t *)&(tsp->twtParameters);
		for (i = 0; i < tsp->numDevicesInSession; i++) {
			wakeInt = 0;
			wakeDur = 0;
			if (tp->agreement == wifi_twt_agreement_type_individual) {
				tpi = &(tp->params.individual);
				wakeInt = tpi->wakeInterval_uSec;
				wakeDur = tpi->wakeTime_uSec;
			}
			printf("%2d)     %6d           %6d              %s       %s"
				"         %s          %s\n",
				tsp->IdTWTsession, wakeInt, wakeDur,
				(tp->operation.announced) ? "NO" : "YES",
				(tp->operation.trigger_enabled) ? "YES" : "NO",
				(tp->operation.implicit) ? "YES" : "NO",
				(tp->agreement == wifi_twt_agreement_type_individual) ?
					"Individual" : "Broadcast");
			tp++ ;
		}
	}
}

static void
print_RadioCapabilities(wifi_radio_capabilities_t *rcap)
{
	unsigned int i, j;
	wifi_radio_trasmitPowerSupported_list_t *xpwrsup;

	printf("\n\tindex: %d", rcap->index);
	printf("\n\tifaceName: %s", rcap->ifaceName);
	printf("\n\tnumSupportedFreqBand: %d", rcap->numSupportedFreqBand);

	for (i = 0; i < rcap->numSupportedFreqBand; i++) {
		printf("\n\t(B%d) ", i);
		print_bands(rcap->band[i]);
		printf("\t(B%d) ", i);
		print_chList(&(rcap->channel_list[i]));
		printf("\n\t(B%d) ", i);
		print_chBandwidths(rcap->channelWidth[i]);
		printf("\n\t(B%d) ", i);
		print_80211_variants(rcap->mode[i]);
		printf("\n\t(B%d) maxBitRate: %d Mbps", i, rcap->maxBitRate[i]);
		print_bitrates(rcap->supportedBitRate[i]);

		xpwrsup = &(rcap->transmitPowerSupported_list[i]);
		printf("\n\t(B%d) transmitPowerSupported_list: num: %d list:",
			i, xpwrsup->numberOfElements);
		for (j = 0; j < xpwrsup->numberOfElements; j++) {
			printf(" %u", xpwrsup->transmitPowerSupported[j]);
		}
	}

	/* autoChannelSupported */
	printf("\n\tautoChannelSupported: %s",
		(rcap->autoChannelSupported) ? "TRUE" : "FALSE");

	/* DCSSupported */
	printf("\n\tDCSSupported: %s",
		(rcap->DCSSupported) ? "TRUE" : "FALSE");

	/* zeroDFSSupported */
	printf("\n\tzeroDFSSupported: %s",
		(rcap->zeroDFSSupported) ? "TRUE" : "FALSE");

	/* csi - TBD update */
	printf("\n\tCSI: maxDevices: %d soudingFrameSupported = %s", rcap->csi.maxDevices,
		(rcap->csi.soudingFrameSupported) ? "TRUE" : "FALSE");

	/* cipher */
	printf("\n\t");
	print_cipher_cap(rcap->cipherSupported);

	/* numcountrySupported */
	printf("\n\tnumcountrySupported: %d ", rcap->numcountrySupported);

	/* countrySupported */
	printf("countrySupported:");
	for (i = 0; i < rcap->numcountrySupported; i++) {
		j = rcap->countrySupported[i];
		printf(" %d(%s)", countrycode_table[j].enum_val, countrycode_table[j].str_val);
	}
	/* maxNumberVAPs */
	printf("\n\tmaxNumberVAPs: %d", rcap->maxNumberVAPs);
#if defined(WIFI_HAL_VERSION_GE_3_0_1)
	printf("\n\tmcast2ucastSupported: %s", rcap->mcast2ucastSupported ? "TRUE" : "FALSE");
#endif /* WIFI_HAL_VERSION_GE_3_0_1 */
}

static void
print_countryCode(wifi_countrycode_type_t countrycode)
{
	int i;
	printf("CountryCode: 0x%04x", countrycode);
	for (i = 0; countrycode_table[i].str_val != NULL; ++i) {
		if (countrycode == (wifi_countrycode_type_t)countrycode_table[i].enum_val)
			printf(" %s", countrycode_table[i].str_val);
	}
	printf("\n");
}
#endif /* WIFI_HAL_VERSION_3 */

/* Max STAs supported to read CSI data, defined by Comcast */
#define MAX_CSI_STA 5

typedef struct {
	char *api_name;
	int num_args;
	char *usage;
} usage_t;

/* Wifi HAL API's list */
/*Add at the end for any new API's in this table - api name, min number of arguments, argument list */
usage_t hal_cmd_table[] =
{
	{ "wifi_getBandSteeringEnable",			0, "" },
	{ "wifi_down",					0, "" },
	{ "wifi_getHalVersion",				0, "" },
	{ "wifi_setWldmMsglevel",			1,
		"<msglevel Debug(0x08)|Info(0x04)|Warning(0x02)|Error(0x01)>"},
	{ "wifi_getRadioNumberOfEntries",		0, "" },
	{ "wifi_getApAclDeviceNum",			1, "<AP Index>" },
	{ "wifi_getSSIDNumberOfEntries",		0, "" },
	{ "wifi_getBandSteeringApGroup",		0, "" },
	{ "wifi_factoryReset",				1, "<reboot 0/1>" },
	{ "wifi_factoryResetRadios",			2, "<commit 0/1> <restart 0/1>" },
	{ "wifi_factoryResetRadio",			3, "<RadioIndex> <commit 0/1> \
		<restart 0/1>" },
	{ "wifi_factoryResetAP",			3, "<AP Index> <commit 0/1> <restart 0/1>"
		},
	{ "wifi_reset",					0, "" },
	{ "wifi_getRadioEnable",			1, "<RadioIndex>" },
	{ "wifi_getRadioUpTime",			1, "<RadioIndex>" },
	{ "wifi_getRadioStatus",			1, "<RadioIndex>" },
	{ "wifi_getSSIDRadioIndex",			1, "<RadioIndex>" },
	{ "wifi_getRadioResetCount",			1, "<RadioIndex>" },
	{ "wifi_getApIsolationEnable",			1, "<RadioIndex>" },
	{ "wifi_getRadioIfName",			1, "<RadioIndex>" },
	{ "wifi_getRadioMaxBitRate",			1, "<RadioIndex>" },
	{ "wifi_getRadioSupportedFrequencyBands",	1, "<RadioIndex>" },
	{ "wifi_getRadioOperatingFrequencyBand",	1, "<RadioIndex>" },
	{ "wifi_getRadioSupportedStandards",		1, "<RadioIndex>" },
	{ "wifi_getRadioStandard",			1, "<RadioIndex>" },
	{ "wifi_getSSIDEnable",				1, "<AP Index>" },
	{ "wifi_getSSIDStatus",				1, "<AP Index>" },
	{ "wifi_getSSIDName",				1, "<AP Index>" },
	{ "wifi_getRadioChannelStats",			1, "<RadioIndex>" },
	{ "wifi_getApAssociatedDeviceRxStatsResult",	2, "<AP Index> <Client MAC>" },
	{ "wifi_getApAssociatedDeviceTxStatsResult",	2, "<AP Index> <Client MAC>" },
	{ "wifi_getApAssociatedDeviceStats",		2, "<AP Index> <Client MAC>" },
	{ "wifi_getBaseBSSID",				1, "<SSID>" },
	{ "wifi_getApEnable",				1, "<AP Index>" },
	{ "wifi_getApStatus",				1, "<AP Index>" },
	{ "wifi_getSSIDTrafficStats2",			1, "<SSID>" },
	{ "wifi_getApAssociatedDeviceDiagnosticResult",	1, "<AP Index>" },
	{ "wifi_getApAssociatedDeviceDiagnosticResult2",1, "<AP Index>" },
	{ "wifi_getApAssociatedDeviceDiagnosticResult3",1, "<AP Index>" },
	{ "wifi_getRadioBandUtilization",		1, "<RadioIndex>" },
	{ "wifi_getApAssociatedDevice",			1, "<AP Index>" },
#ifdef WIFI_HAL_VERSION_3
	{ "wifi_getApAuthenticatedDevices",		1, "<AP Index>" },
#endif /* WIFI_HAL_VERSION_3 */
	{ "wifi_getApBeaconRate",			1, "<AP Index>" },
	{ "wifi_getApTxBeaconFrameCount",		1, "<AP Index>" },
	{ "wifi_getRadioOperationalDataTransmitRates",	1, "<RadioIndex>" },
	{ "wifi_getATMCapable",				0, "" },
	{ "wifi_getRadioDcsDwelltime",			1, "<RadioIndex>" },
	{ "wifi_setRadioDcsDwelltime",			2, "<RadioIndex>" "<timeMillisec>" },
	{ "wifi_getRadioGuardInterval",			1, "<RadioIndex>" },
	{ "wifi_getApManagementFramePowerControl",	1, "<AP Index>" },
	{ "wifi_getRadioDcsScanning",			1, "<RadioIndex>" },
	{ "wifi_deleteAp",				1, "<AP Index>" },
	{ "wifi_getSSIDNameStatus",			1, "<SSID>" },
	{ "wifi_getApMacAddressControlMode",		1, "<AP Index>" },
	{ "wifi_getRadioCountryCode",			1, "<RadioIndex>" },
	{ "wifi_getApBasicAuthenticationMode",		1, "<AP Index>" },
	{ "wifi_getApWpaEncryptionMode",		1, "<AP Index>" },
	{ "wifi_getApWpsDevicePIN",			1, "<AP Index>" },
	{ "wifi_getBandSteeringCapability",		1, "<AP Index>" },
	{ "wifi_getRadio11nGreenfieldEnable",		1, "<AP Index>" },
	{ "wifi_getRadio11nGreenfieldSupported",	1, "<AP Index>" },
	{ "wifi_getApWpsEnable",			1, "<AP Index>" },
	{ "wifi_getApSecuritySecondaryRadiusServer",	1, "<AP Index>" },
	{ "wifi_getApSecurityRadiusServer",		1, "<AP Index>" },
	{ "wifi_getApSecurityRadiusSettings",		1, "<AP Index>" },
	{ "wifi_setApSecurityRadiusSettings",		10, "<AP Index> <RadiusServerRetries> \
		<RadiusServerRequestTimeout> <PMKCaching> <PMKCacheInterval> <PMKLifetime> \
		<MaxAuthenticationAttempts> <BlacklistTableTimeout> <IdentityRequestRetryInterval> \
		<QuietPeriodAfterFailedAuthentication>" },
	{ "wifi_getApBridgeInfo",			1, "<AP Index>" },
	{ "wifi_getRadioDCSChannelPool",		1, "<RadioIndex>" },
	{ "wifi_getRadioBasicDataTransmitRates",	1, "<RadioIndex>" },
	{ "wifi_getApSecurityModesSupported",		1, "<AP Index>" },
	{ "wifi_getApSecurityModeEnabled",		1, "<AP Index>" },
	{ "wifi_getApSecurityKeyPassphrase",		1, "<AP Index>" },
	{ "wifi_getRadioOperatingChannelBandwidth",	1, "<RadioIndex>" },
	{ "wifi_getApWpsConfigMethodsEnabled",		1, "<AP Index>" },
	{ "wifi_getRadioChannel",			1, "<RadioIndex>" },
	{ "wifi_getRadioAutoChannelEnable",		1, "<RadioIndex>" },
	{ "wifi_getRadioAutoChannelSupported",		1, "<RadioIndex>" },
	{ "wifi_getRadioPossibleChannels",		1, "<RadioIndex>" },
	{ "wifi_getRadioTransmitPower",			1, "<RadioIndex>" },
	{ "wifi_getRadioAMSDUEnable",			1, "<RadioIndex>" },
	{ "wifi_getRadioTxChainMask",			1, "<RadioIndex>" },
	{ "wifi_getRadioRxChainMask",			1, "<RadioIndex>" },
	{ "wifi_getApName",				1, "<AP Index>" },
	{ "wifi_getRadioChannelsInUse",			1, "<RadioIndex>" },
	{ "wifi_getApSsidAdvertisementEnable",		1, "<AP Index>" },
	{ "wifi_getApBeaconType",			1, "<AP Index>" },
	{ "wifi_getBasicTrafficStats",			1, "<AP Index>" },
	{ "wifi_getWifiTrafficStats",			1, "<AP Index>" },
	{ "wifi_getApNumDevicesAssociated",		1, "<AP Index>" },
	{ "wifi_getAllAssociatedDeviceDetail",		1, "<AP Index>" },
	{ "wifi_getApAclDevices",			1, "<AP Index>" },
	{ "wifi_getApSecurityPreSharedKey",		1, "<AP Index>" },
	{ "wifi_getApWpsConfigurationState",		1, "<AP Index>" },
	{ "wifi_getRadioSupportedDataTransmitRates",	1, "<RadioIndex>" },
	{ "wifi_getRadioExtChannel",			1, "<RadioIndex>" },
	{ "wifi_getRadioTransmitPowerSupported",	1, "<RadioIndex>" },
	{ "wifi_getApWpsConfigMethodsSupported",	1, "<AP Index>" },
	{ "wifi_getBandSteeringBandUtilizationThreshold",	1, "<RadioIndex>" },
	{ "wifi_getApRadioIndex",			1, "<AP Index>" },
	{ "wifi_getRadioDCSScanTime",			1, "<RadioIndex>" },
	{ "wifi_getBandSteeringRSSIThreshold",		1, "<RadioIndex>" },
	{ "wifi_getBandSteeringPhyRateThreshold",	1, "<RadioIndex>" },
	{ "wifi_getRadioChannelStats2",			1, "<RadioIndex>" },
	{ "wifi_getRadioTrafficStats2",			1, "<RadioIndex>" },
	{ "wifi_getApSecurityMFPConfig",		1, "<AP Index>" },
#ifdef MAX_KEY_HOLDERS
	{ "wifi_testApFBTFeature",			1, "<AP Index>" },
	{ "wifi_setFastBSSTransitionActivated",		2, "<AP Index> <activate>" },
	{ "wifi_getBSSTransitionActivated",		1, "<AP Index>" },
	{ "wifi_getFTOverDSActivated",			1, "<AP Index>" },
	{ "wifi_setFTOverDSActivated",			2, "<AP Index> <activate>" },
	{ "wifi_getFTMobilityDomainID",			1, "<AP Index>" },
	{ "wifi_setFTMobilityDomainID",			2, "<AP Index> <domain>" },
	{ "wifi_getFTResourceRequestSupported",		1, "<AP Index>" },
	{ "wifi_setFTResourceRequestSupported",		2, "<AP Index> <supported>" },
	{ "wifi_getFTR0KeyLifetime",			1, "<AP Index>" },
	{ "wifi_setFTR0KeyLifetime",			2, "<AP Index> <key_lifetime>" },
	{ "wifi_getFTR0KeyHolderID",			1, "<AP Index>" },
	{ "wifi_setFTR0KeyHolderID",			2, "<AP Index> <keyHolderID>" },
	{ "wifi_getFTR1KeyHolderID",			1, "<AP Index>" },
	{ "wifi_setFTR1KeyHolderID",			2, "<AP Index> <keyHolderID>" },
	{ "wifi_pushApFastTransitionConfig",		4, "<support> <mobilityDomain> <overDS> \
		<r0KeyLifeTime>" },
#endif /* MAX_KEY_HOLDERS */
	{ "wifi_setBandSteeringEnable",			1, "<Enable/Disable(0/1)>" },
	{ "wifi_removeApSecVaribles",			1, "<AP Index>" },
	{ "wifi_disableApEncryption",			1, "<AP Index>" },
	{ "wifi_setApWpsButtonPush",			1, "<AP Index>" },
	{ "wifi_cancelApWPS",				1, "<AP Index>" },
	{ "wifi_applyRadioSettings",			1, "<RadioIndex>" },
	{ "wifi_delApAclDevices",			1, "<AP Index>" },
	{ "wifi_setBandSteeringApGroup",		1, "<AP Indexes(E.g: 0,2,5)>" },
#ifdef WIFI_HAL_VERSION_3_PHASE2
	{ "wifi_getNeighboringWiFiDiagnosticResult2",	2, "<RadioIndex> <Scan 0/1>" },
#else
	{ "wifi_getNeighboringWiFiDiagnosticResult2",	1, "<RadioIndex>" },
#endif /* WIFI_HAL_VERSION_3_PHASE2 */
	{ "wifi_getNeighboringWiFiStatus",		1, "<AP Index>" },
	{ "wifi_setApSecurityReset",			3, "<AP Index> <commit 0/1> <restart 0/1>"
		},
	{ "wifi_applySSIDSettings",			1, "<SSID>" },
	{ "wifi_apAssociatedDevice_callback_register",	0, "" },
	{ "wifi_newApAssociatedDevice_callback_register",	0, "" },
	{ "wifi_apAuthEvent_callback_register",		0, "" },
	{ "wifi_apDisassociatedDevice_callback_register",	0, "" },
	{ "wifi_apDeAuthEvent_callback_register",	0, "" },
	{ "wifi_steering_event_callback_register",	0, "" },
	{ "wifi_steering_setGroup",			11, "<groupIndex> <AP Index> <utilCheckInteval> ... (total 11 arguments)" },
	{ "wifi_steering_clientSet",			11, "<groupIndex> <AP Index> <client mac> \
		<rssiProbeHWM etc. 8 parameters>" },
	{ "wifi_steering_clientRemove",			3, "<groupIndex> <AP Index> <client mac>" },
	{ "wifi_steering_clientMeasure",		3, "<groupIndex> <AP Index> <client mac>" },
	{ "wifi_steering_clientDisconnect",		5, "<groupIndex> <AP Index> <client mac> \
		<type> <reason>" },
	{ "wifi_getRadioClientInactivityTimeout",	1, "<RadioIndex>" },
#if WIFI_HAL_VERSION_GE_2_12 || defined(WIFI_HAL_VERSION_3)
	/* 802.11K api */
	{ "wifi_RMBeaconRequest_callback_register",	0, "" },
	{ "wifi_RMBeaconRequest_callback_unregister",	0, "" },
	{ "wifi_setRMBeaconRequest",			7, "<AP Index> <PeerMACAddress> <opClass>"
		" <Channel> <RandomizationInterval> <Duration> <req_MeasurementMode>"
		" [<bssid> <numRepetitions> <ssidPresent <ssid>>"
		"  <beaconReportingPresent <condition threshold>>"
		"  <reportingRetailPresent <reportingDetail=0,1,2>>"
		"  <wideBandWidthChannelPresent <bandwidth centerSeg0 centerSeg1>>]" },
	{ "wifi_cancelRMBeaconRequest",			2, "<AP Index> <Dialog Token>" },
	{ "wifi_getRMCapabilities",			1, "<Peer mac>" },
	{ "wifi_setNeighborReports",			2, "<AP Index> <cnt> [<bssid1> \
		<bssid1 info> <regulatory1> <channel1> <phytype1>] cnt times" },
	{ "wifi_setNeighborReportActivation",		2, "<AP Index> <Activate(0/1)>" },
	{ "wifi_getNeighborReportActivation",		1, "<AP Index>" },
#endif /* (WIFI_HAL_VERSION_GE_2_12) || defined(WIFI_HAL_VERSION_3) */
	{ "wifi_getRadioDCSSupported",			1, "<RadioIndex>" },
	{ "wifi_getRadioDCSEnable",			1, "<RadioIndex>" },
	{ "wifi_BTMQueryRequest_callback_register",	0, "" },
	{ "wifi_setBTMRequest",				4, "<AP Index> <PeerMACAddress> <Token> \
		mode [<url_len> <url>] <num-candidate> <len1> <candidate1 like \
		00904C1DA06100000000162400> [<len2> <candidate2>...]" },
	{ "wifi_getBSSTransitionImplemented",		1, "<RadioIndex>" },
	{ "wifi_setBSSTransitionActivation",		2, "<AP Index> <0|1>" },
	{ "wifi_getBSSTransitionActivation",		1, "<AP Index>" },
	{ "wifi_getBTMClientCapabilityList",		2, "<AP Index> <MAC 1> [<MAC 2> ...]" },
	{ "wifi_getApInterworkingElement",		1, "<AP Index>" },
	{ "wifi_getInterworkingAccessNetworkType",	1, "<AP Index>" },
	{ "wifi_getApInterworkingServiceCapability",	1, "<AP Index>" },
	{ "wifi_getApInterworkingServiceEnable",	1, "<AP Index>" },
#if WIFI_HAL_VERSION_GE_2_19 || defined(WIFI_HAL_VERSION_3)
	{ "wifi_pushApHotspotElement",			2, "<AP Index> <0|1>" },
	{ "wifi_getApHotspotElement",			1, "<AP Index>" },
	{ "wifi_pushApRoamingConsortiumElement",	2, "<AP Index> <ouilist>" },
	{ "wifi_getApRoamingConsortiumElement",		1, "<AP Index>" },
	{ "wifi_setCountryIe",				2, "<AP Index> <0|1>" },
	{ "wifi_getCountryIe",				1, "<AP Index>" },
	{ "wifi_setLayer2TrafficInspectionFiltering",	2, "<AP Index> <0|1>" },
	{ "wifi_getLayer2TrafficInspectionFiltering",	1, "<AP Index>" },
	{ "wifi_setDownStreamGroupAddress",		2, "<AP Index> <0|1>" },
	{ "wifi_getDownStreamGroupAddress",		1, "<AP Index>" },
	{ "wifi_setBssLoad",				2, "<AP Index> <0|1>" },
	{ "wifi_getBssLoad",				1, "<AP Index>" },
	{ "wifi_setProxyArp",				2, "<AP Index> <0|1>" },
	{ "wifi_getProxyArp",				1, "<AP Index>" },
	{ "wifi_setP2PCrossConnect",			2, "<AP Index> <0|1>" },
	{ "wifi_getP2PCrossConnect",			1, "<AP Index>" },
	{ "wifi_applyGASConfiguration",			1, "<GAS Configuration>" },
#endif /* WIFI_HAL_VERSION_GE_2_19 */
#if WIFI_HAL_VERSION_GE_2_16 || defined(WIFI_HAL_VERSION_3)
#if WIFI_HAL_VERSION_GE_2_19 || defined(WIFI_HAL_VERSION_3)
	{ "wifi_mgmt_frame_callbacks_register",		1, "<AP Index>" },
#else
	{ "wifi_dpp_frame_received_callbacks_register",	1, "<AP Index>" },
#endif /* WIFI_HAL_VERSION_GE_2_19 */
	{ "wifi_sendActionFrame",			4, "<AP Index> <PeerMACAddress> <Frequency> \
		<Action Frame Hex String>" },
#endif /* WIFI_HAL_VERSION_GE_2_16 */
#if WIFI_HAL_VERSION_GE_2_15 && (!defined(_CBR_PRODUCT_REQ_) && \
	!defined(_XF3_PRODUCT_REQ_))
	{ "wifi_getRadioMode",				1, "<RadioIndex>" },
	{ "wifi_setRadioMode",				3, "<RadioIndex> <channel mode> \
		<pure mode>" },
	{ "wifi_setDownlinkDataAckType",		2, "<RadioIndex> <ack_type>" },
	{ "wifi_setDownlinkMuType",			2, "<RadioIndex> <ack_type>" },
	{ "wifi_getDownlinkMuType",			1, "<RadioIndex>" },
	{ "wifi_setUplinkMuType",			2, "<RadioIndex> <ack_type>" },
	{ "wifi_getUplinkMuType",			1, "<RadioIndex>" },
	{ "wifi_setGuardInterval",			2, "<RadioIndex> <guard interval>" },
	{ "wifi_getGuardInterval",			1, "<RadioIndex>" },
	{ "wifi_setBSSColorEnabled",			2, "<AP Index> <Enable/Disable(0/1)>" },
	{ "wifi_getBSSColorEnabled",			1, "<AP Index>" },
	{ "wifi_getBSSColor",				1, "<Radio/AP Index>" },
	{ "wifi_getTWTParams",				1, "<AP Index>" },
	{ "wifi_get80211axDefaultParameters",		1, "<AP Index>" },
#endif /* WIFI_HAL_MINOR_VERSION >= 15 */
#if WIFI_HAL_VERSION_GE_2_18 && !defined(_CBR1_PRODUCT_REQ_) && \
	!defined(_XF3_PRODUCT_REQ_)
	{ "wifi_chan_eventRegister",			0, "" },
	{ "wifi_getRadioChannels",			1, "<RadioIndex>" },
#endif /* WIFI_HAL_VERSION_GE_2_18 && !_CBR1_PRODUCT_REQ_ && !_XF3_PRODUCT_REQ_ */
	{ "wifi_setApEnable",				2, "<AP Index> <Enable/Disable(0/1)>" },
	{ "wifi_pushRadioChannel",			2, "<RadioIndex> <Channel>" },
	{ "wifi_setRadioDcsScanning",			2, "<RadioIndex> <Enable/Disable(0/1)>" },
	{ "wifi_setApBasicAuthenticationMode",		2, "<AP Index> <Authentication Mode>" },
	{ "wifi_setApWpaEncryptionMode",		2, "<AP Index> <Encryption Mode>" },
	{ "wifi_setApWpsDevicePIN",			2, "<AP Index> <PIN>" },
	{ "wifi_setRadio11nGreenfieldEnable",		2, "<RadioIndex> <Enable/Disable(0/1)>" },
	{ "wifi_setApWpsEnable",			2, "<AP Index> <Enable/Disable(0/1)>" },
	{ "wifi_setRadioBasicDataTransmitRates",	2, "<RadioIndex> \
		<TransmitRates(E.g: 6,54)>" },
	{ "wifi_setRadioOperatingChannelBandwidth",	2, "<RadioIndex> <Bandwidth(E.g: 20MHz)>" },
	{ "wifi_setApWpsConfigMethodsEnabled",		2, "<AP Index> <ConfigMethodsEnabled \
		(E.g: PushButton/PIN)>" },
	{ "wifi_setRadioChannel",			2, "<RadioIndex> <Channel>" },
	{ "wifi_setRadioAutoChannelEnable",		2, "<RadioIndex> <Enable/Disable(0/1)>" },
	{ "wifi_setRadioEnable",			2, "<RadioIndex> <Enable/Disable(0/1)>" },
	{ "wifi_setRadioTransmitPower",			2, "<RadioIndex> <TransmitPower>" },
	{ "wifi_setApDTIMInterval",			2, "<AP Index> <DTIM-Interval>" },
	{ "wifi_setRadioCtsProtectionEnable",		2, "<AP Index> <Enable/Disable(0/1)>" },
	{ "wifi_setRadioObssCoexistenceEnable",		2, "<AP Index> <Enable/Disable(0/1)>" },
	{ "wifi_setRadioFragmentationThreshold",	2, "<AP Index> <Threshold>" },
	{ "wifi_setRadioSTBCEnable",			2, "<RadioIndex> <Enable/Disable(0/1)>" },
	{ "wifi_setRadioAMSDUEnable",			2, "<RadioIndex> <Enable/Disable(0/1)>" },
	{ "wifi_setRadioGuardInterval",			2, "<RadioIndex> <Guard-Interval \
		(E.g:auto/400/800)>" },
	{ "wifi_setRadioTxChainMask",			2, "<RadioIndex> <NumberStreams>" },
	{ "wifi_setRadioRxChainMask",			2, "<RadioIndex> <NumberStreams>" },
	{ "wifi_setSSIDName",				2, "<AP Index> <SSID String>" },
	{ "wifi_pushSSID",				2, "<AP Index> <SSID String>" },
	{ "wifi_setApSsidAdvertisementEnable",		2, "<AP Index> <Enable/Disable(1/0)>" },
	{ "wifi_pushSsidAdvertisementEnable",		2, "<AP Index> <Enable/Disable(1/0)>" },
	{ "wifi_setApSecurityPreSharedKey",		2, "<AP Index> <PreSharedKey>" },
	{ "wifi_setApWpsEnrolleePin",			2, "<AP Index> <WPS PIN>" },
	{ "wifi_setApWmmEnable",			2, "<AP Index> <Enable/Disable(1/0)>" },
	{ "wifi_setApWmmUapsdEnable",			2, "<AP Index> <Enable/Disable(1/0)>" },
	{ "wifi_setApBeaconInterval",			2, "<AP Index> <Beacon Interval>" },
	{ "wifi_setRadioExtChannel",			2, "<RadioIndex> \
		<Extension Channel String>" },
	{ "wifi_addApAclDevice",			2, "<AP Index> <MAC Address>" },
	{ "wifi_setBandSteeringBandUtilizationThreshold",	2, "<RadioIndex> <BUThreshold>" },
	{ "wifi_setRadioDCSChannelPool",		2, "<RadioIndex> <Channel-Pool>" },
	{ "wifi_setSSIDEnable",				2, "<SSID> <Enable/Disable(1/0)>" },
	{ "wifi_setApSecurityModeEnabled",		2, "<AP Index> <SecurityMode>" },
	{ "wifi_setApSecurityKeyPassphrase",		2, "<AP Index> <PassPhrase>" },
	{ "wifi_setApIsolationEnable",			2, "<AP Index> <Enable/Disable(1/0)>" },
	{ "wifi_setApRadioIndex",			2, "<AP Index> <RadioIndex>" },
	{ "wifi_setBandSteeringRSSIThreshold",		2, "<RadioIndex> \
		<BandSteeringRSSIThreshold>" },
	{ "wifi_setBandSteeringPhyRateThreshold",	2, "<RadioIndex> <PhyRateThreshold>" },
	{ "wifi_setApSecurityMFPConfig",		2, "<AP Index> <MFP config(E.g: Disabled/\
		Optional/Required)>" },
	{ "wifi_delApAclDevice",			2, "<AP Index> <MAC Address>" },
	{ "wifi_setInterworkingAccessNetworkType",	2, "<AP Index> <Network Type>" },
	{ "wifi_setApInterworkingServiceEnable",	2, "<AP Index> <Enable/Disable(0/1)>" },
	{ "wifi_setApMacAddressControlMode",		2, "<AP Index> <FilterMode>" },
	{ "wifi_getApDeviceRSSI",			2, "<AP Index> <MAC Address of STA>" },
	{ "wifi_getApDeviceRxrate",			2, "<AP Index> <MAC Address of STA>" },
	{ "wifi_getApDeviceTxrate",			2, "<AP Index> <MAC Address of STA>" },
	{ "wifi_getRadioDcsChannelMetrics",		1, "<RadioIndex>" },
	{ "wifi_setApManagementFramePowerControl",	2, "<AP Index> <Power in dBM>" },
	{ "wifi_setApBeaconRate",			2, "<AP Index> <Beacon Rate>" },
	{ "wifi_setApBeaconType",			2, "<AP Index> <Beacon String (None/Basic/\
		WPA/11i/WPAand11i)>" },
	{ "wifi_getApAssociatedDeviceRxTxStatsResult",	2, "<AP Index> <Client MAC>" },
	{ "wifi_setApScanFilter",			3, "<AP Index> <Mode> <ESS-ID>" },
	{ "wifi_setApCsaDeauth",			2, "<AP Index> <Mode>" },
	{ "wifi_kickApAssociatedDevice",		2, "<AP Index> <Client MAC>" },
	{ "wifi_setRadioDCSEnable",			2, "<RadioIndex> <Enable/Disable(0/1)>" },
	{ "wifi_setApWmmOgAckPolicy",			3, "<AP Index> <Class> <AckPolicy>" },
	{ "wifi_setRadioDCSScanTime",			3, "<RadioIndex> <interval in secs> <dwell\
		in ms>" },
	{ "wifi_setApSecuritySecondaryRadiusServer",	4, "<AP Index> <IPAddress> <Port> <Radius\
		SecretKey>" },
	{ "wifi_setApSecurityRadiusServer",		4, "<AP Index> <IPAddress> <Port> <Radius\
		SecretKey>" },
	{ "wifi_createAp",				4, "<AP Index> <radioIndex> <E-SSID> <Hide\
		-SSID> " },
	{ "wifi_pushRadioChannel2",			4, "<RadioIndex> <Channel> <Channel-Width>\
		<CSA Beacon Count>" },
	{ "wifi_setRadioChannelMode",			5, "<RadioIndex> <Channel Mode> <802.11g>\
		<802.11n> <802.11a,c>(E.g: wifi_setRadioChannelMode 0 11A 1 1 0)" },
	{ "wifi_startNeighborScan",			5, "<AP Index> <Scan Mode> <Dwell Time>\
		<Channel Number> <list of channels>" },
	{ "wifi_pushApInterworkingElement",		11, "<AP Index> <InterworkingEnabled>\
		<Access N/W Type> <Internet Available> <asra> <esr> <uesa> <venueOptionPresent>\
		<venueGroup> <venueType> <HESSID OptionPresent> <HESSID>" },
	{ "wifi_setRadioTrafficStatsMeasure",		3, "<RadioIndex> <Radio Statistics\
		Measuring Rate> <Radio Statistics Measuring Interval>" },
	{ "wifi_setRadioTrafficStatsRadioStatisticsEnable",	2, "<RadioIndex> <Enable/Disable\
(0/1)>" },
	{ "wifi_getRadioStatsReceivedSignalLevel",	2, "<RadioIndex> <Signal Index>" },
	{ "wifi_getRadioWifiTrafficStats",		1, "<RadioIndex>" },
	{ "wifi_getSSIDTrafficStats",			1, "<AP Index>" },
	{ "wifi_setRadioStatsEnable",			2, "<RadioIndex> <Enable/Disable(0/1)>" },
	{ "wifi_getRadioStatsEnable",			1, "<RadioIndex>" },
	{ "wifi_stopHostApd",				0, "" },
	{ "wifi_startHostApd",				0, "" },
	{ "wifi_getApWmmEnable",			1, "<AP Index>" },
	{ "wifi_setLED",				2, "<RadioIndex> <Enable/Disable(0/1)>" },
	{ "wifi_setApAuthMode",				2, "<RadioIndex> <Mode>" },
	{ "wifi_setRadioCountryCode",			2, "<RadioIndex> <coutry code>" },
	{ "wifi_getRadioAutoChannelRefreshPeriodSupported",	1, "<RadioIndex>" },
	{ "wifi_getRadioAutoChannelRefreshPeriod",	1, "<RadioIndex>" },
	{ "wifi_setRadioAutoChannelRefreshPeriod",	2, "<RadioIndex> <RefreshPeriod" },
	{ "wifi_setRadioMCS",				2, "<RadioIndex> <MCS index>" },
	{ "wifi_getRadioMCS",				1, "<RadioIndex>" },
	{ "wifi_getRadioIEEE80211hSupported",		1, "<RadioIndex>" },
	{ "wifi_getRadioIEEE80211hEnabled",		1, "<RadioIndex" },
	{ "wifi_setRadioIEEE80211hEnabled",		2, "<RadioIndex> <Enable/Disable(0/1)" },
	{ "wifi_getRadioBeaconPeriod",			1, "<RadioIndex>" },
	{ "wifi_setRadioBeaconPeriod",			2, "<RadioIndex> <BeaconPerios in ms>" },
	{ "wifi_getSSIDMACAddress",			1, "<RadioIndex>" },
	{ "wifi_getApRtsThresholdSupported",		1, "<AP Index>" },
	{ "wifi_kickApAclAssociatedDevices",		2, "<AP Index> <Enable/Disable(1/0)>" },
	{ "wifi_getApRetryLimit",			1, "<AP Index" },
	{ "wifi_setApRetryLimit",			2, "<AP Index> <Retry number>" },
	{ "wifi_getApWMMCapability",			1, "<AP Index>" },
	{ "wifi_getApUAPSDCapability",			1, "<AP Index>" },
	{ "wifi_getApWmmUapsdEnable",			1, "<AP Index>" },
	{ "wifi_getApMaxAssociatedDevices",		1, "<AP Index>" },
	{ "wifi_setApMaxAssociatedDevices",		2, "<AP Index> <maxDevices>" },
	{ "wifi_setApBridgeInfo",			4, "<AP Index> <BridgeName> <IP> <Subnet>" },
	{ "wifi_getIndexFromName",			1, "<SSID>" },
	{ "wifi_initRadio",				1, "<RadioIndex>" },
	{ "wifi_setApRtsThreshold",			2, "<RadioIndex> <Threshold>" },
	{ "wifi_getRadioDfsSupport",			1, "<RadioIndex>" },
	{ "wifi_getRadioDfsEnable",			1, "<RadioIndex>" },
	{ "wifi_setRadioDfsEnable",			2, "<RadioIndex> <Enable/Disable(0/1)" },
	{ "wifi_getRadioDfsAtBootUpEnable",		1, "<RadioIndex>" },
	{ "wifi_setRadioDfsAtBootUpEnable",		2, "<RadioIndex> <Enable/Disable(1/0)" },
	{ "wifi_setRadioDfsRefreshPeriod",		2, "<RadioIndex> <RefreshPeriod(seconds)" },
	{ "wifi_setApAssociatedDevicesHighWatermarkThreshold",		2, "<AP Index> <Threshold>" },
	{ "wifi_getApAssociatedDevicesHighWatermarkThreshold",		1, "<AP Index>" },
	{ "wifi_getApAssociatedDevicesHighWatermarkThresholdReached",	1, "<AP Index>" },
	{ "wifi_getApAssociatedDevicesHighWatermark",			1, "<AP Index>" },
	{ "wifi_getApAssociatedDevicesHighWatermarkDate",		1, "<AP Index>" },
#if WIFI_HAL_VERSION_GE_2_15 && (!defined(_CBR_PRODUCT_REQ_) && \
	!defined(_XF3_PRODUCT_REQ_))
	{ "wifi_getRadioDfsMoveBackEnable",		1, "<RadioIndex>" },
	{ "wifi_setRadioDfsMoveBackEnable",		2, "<RadioIndex> <Enable/Disable(0/1)" },
#endif /* WIFI_HAL_VERSION_GE_2_15 && (!defined(_CBR_PRODUCT_REQ_) && !defined(_XF3_PRODUCT_REQ_)) */
#if WIFI_HAL_VERSION_GE_2_17 || defined(WIFI_HAL_VERSION_3)
	{ "wifi_getApAssociatedClientDiagnosticResult",	2, "<AP Index> <Client MAC without colon Ex:0011223344>" },
#endif /* WIFI_HAL_VERSION_GE_2_17 */
	{ "wifi_setClientDetailedStatisticsEnable",	2, "<RadioIndex> <Enable/Disable(0/1)" },
#if WIFI_HAL_VERSION_GE_2_16
#ifdef _XF3_PRODUCT_REQ_
	{ "wifi_getVAPTelemetry",			1, "<AP Index>" },
#endif /* _XF3_PRODUCT_REQ_ */
#endif /* WIFI_HAL_VERSION_GE_2_16 */
#if WIFI_HAL_VERSION_GE_2_18 || defined(WIFI_HAL_VERSION_3)
	{ "wifi_getRadioPercentageTransmitPower",	1, "<RadioIndex>" },
#endif /* WIFI_HAL_VERSION_GE_2_18 */
	{ "wifi_getBandSteeringLog",			1, "<Record Index>" },
	{ "wifi_apply",					0, "" },
	{ "wifi_init",					0, "" },
#if !defined(_CBR1_PRODUCT_REQ_) && !defined(_XF3_PRODUCT_REQ_)
#if WIFI_HAL_VERSION_GE_2_19
	{ "wifi_enableCSIEngine",			3, "<AP Index> <enable|disable> <Client MAC>" },
#ifdef CMWIFI_RDKB_COMCAST
	{ "wifi_csitest",				3, "<AP Index> <Period> <Client MACs>" },
#endif /* CMWIFI_RDKB_COMCAST */
	{ "wifi_enableGreylistAccessControl",		1, "<Enable/Disable(1/0)>" },
	{ "wifi_getApDASRadiusServer",			1, "<AP Index>" },
	{ "wifi_setApDASRadiusServer",			4,
		"<AP Index> <IPAddress> <Port> <DAS secret>" },
#endif /* WIFI_HAL_VERSION_GE_2_19 */
#endif /* !defined(_CBR1_PRODUCT_REQ_) && !defined(_XF3_PRODUCT_REQ_) */
#if !defined(_CBR1_PRODUCT_REQ_) && !defined(_XF3_PRODUCT_REQ_)
	{ "wifi_allow2G80211ax",			1, "<Enable/Disable(1/0)>" },
	{ "wifi_getAllow2G80211ax",			0, ""},
#endif /* !_CBR1_PRODUCT_REQ_ && !_XF3_PRODUCT_REQ_ */
#if defined(WIFI_HAL_VERSION_3)
	{ "wifi_getHalCapability",			0, "" },
	{ "wifi_getAPCapabilities",			1, "<AP Index>" },
	{ "wifi_getApWpsConfiguration",			1, "<AP Index>" },
	{ "wifi_setApWpsConfiguration",			2, "<AP Index> <Enable/Disable(1/0)> <...>"},
	{ "wifi_getRadioFrequencyBand",			1, "<RadioIndex>" },
	{ "wifi_getRadioOperatingParameters",		1, "<RadioIndex>" },
	{ "wifi_setRadioOperatingParameters",		6, "<RadioIndex> <AutoChannelEnabled(0/1)>\
<Channel> <Bandwidth(1/2/4/8/16)> <OperatingMode> <CsaBeaconCount>" },
	{ "wifi_getApSecurity",				1, "<AP Index>" },
	{ "wifi_setApSecurity",				2, "<AP Index> <SecurityMode> <...>"},
	{ "wifi_setEAP_Param",				3, "<AP Index> <value> <param>" },
	{ "wifi_getEAP_Param",				1, "<AP Index>" },
	{ "wifi_setBSSColor",				2, "<RadioIndex> <color value>" },
	{ "wifi_getAvailableBSSColor",			1, "<RadioIndex>" },
	{ "wifi_getMuEdca",				2, "<RadioIndex> <aci>" },
	{ "wifi_setMuEdca",                             6, "<RadioIndex> <aci> <aifsn> <ecw_min> \
								<ecw_max> <timer>" },
	{ "wifi_getRadioVapInfoMap",			1, "<RadioIndex>" },
	{ "wifi_getTWTsessions",			2, "<AP Index> <maxNumberSessions>" },
	{ "wifi_setBroadcastTWTSchedule",		7, "<AP Index> <implicit(0/1)> "
							"<announced(0/1)> <trigger(0/1)> "
							"<wakeDuration_uSec> <wakeInterval_uSec> "
							"<create> <sessId>" },
	{ "wifi_setTeardownTWTSession",			2, "<AP Index> <sessionID>" },
	{ "wifi_createVAP",				2, "<RadioIndex> <configType>"},
/* configType = 1 => enable, hideSsid & isolation toggle between 1 & 0 and Security=NONE for all apIndices
 * configType = 2 => enable=1, hideSsid=1, isloation=0 and security=WPA2_PSK AES for all apIndices */
#endif /* WIFI_HAL_VERSION_3 */
#ifdef RDKB_LGI
	{ "wifi_getRADIUSAcctEnable",			1, "<AP Index>" },
	{ "wifi_setRADIUSAcctEnable",			2, "<AP Index> <Enable/Disable(0/1)>" },
	{ "wifi_getApSecurityAcctServer",		1, "<AP Index>" },
	{ "wifi_setApSecurityAcctServer",		4,
		"<AP Index> <IPAddress> <Port> <Radius SecretKey>" },
	{ "wifi_getApSecuritySecondaryAcctServer",	1, "<AP Index>" },
	{ "wifi_setApSecuritySecondaryAcctServer",	4,
		"<AP Index> <IPAddress> <Port> <Radius SecretKey>" },
	{ "wifi_getApSecurityAcctInterimInterval",	1, "<AP Index>" },
	{ "wifi_setApSecurityAcctInterimInterval",	2, "<AP Index> <NewInterval>" },
	{ "wifi_setApRadiusTransportInterface",		1, "<RadiusInterface>" },
	{ "wifi_getApRadiusReAuthInterval",		1, "<AP Index>" },
	{ "wifi_setApRadiusReAuthInterval",		2, "<AP Index> <Interval>" },
	{ "wifi_getRadiusOperatorName",			1, "<AP Index>" },
	{ "wifi_setRadiusOperatorName",			2, "<RadioIndex> <op_name>" },
	{ "wifi_getRadiusLocationData",			1, "<AP Index>" },
	{ "wifi_setRadiusLocationData",			2, "<RadiusIndex> <loc_data>" },
	{ "wifi_getApPMKCacheInterval",			1, "<AP Index>" },
	{ "wifi_setApPMKCacheInterval",			2, "<AP Index> <interval>" },
	{ "wifi_getWpsStatus",				1, "<AP Index>" },
	{ "wifi_getRadioConfiguredChannel",		1, "<AP Index>" },
	{ "wifi_getRadioRunningChannel",		1, "<AP Index>" },
	{ "wifi_getSoftBlockEnable",			0, "" },
	{ "wifi_setSoftBlockEnable",			1, "<Disable/Enable(0/1)>" },
	{ "wifi_clearSoftBlockBlacklist",		0, "" },
	{ "wifi_getSoftBlockBlacklistEntries",		1, "<AP Index>" },
	{ "wifi_getSupportRatesBitmapControlFeature",	0, "" },
	{ "wifi_setSupportRatesBitmapControlFeature",	1, "<Enable/Disable(1/0)>" },
	{ "wifi_getSupportRatesDisableBasicRates",	1, "<AP Index>" },
	{ "wifi_setSupportRatesDisableBasicRates",	2, "<AP Index> <rate>" },
	{ "wifi_getSupportRatesDisableSupportedRates",	1, "<AP Index>" },
	{ "wifi_setSupportRatesDisableSupportedRates",	2, "<AP Index> <rate>" },
	{ "wifi_getApWmmOgAckPolicy",			1, "<AP Index>" },
	{ "wifi_api_is_device_associated",		2, "<AP Index> <MAC Address of STA>" },
	{ "wifi_kickAllAssociatedDevice",		1, "<AP Index>" },
	{ "wifi_getRadioExcludeDfs",			1, "<RadioIndex>" },
	{ "wifi_setRadioExcludeDfs",			2, "<RadioIndex> <Enable/Disable(1/0)" },
	{ "wifi_getRadioChannelWeights",		1, "<RadioIndex>" },
	{ "wifi_setRadioChannelWeights",		2, "<RadioIndex> <ch0 wt%> ch1 wt%> <...>" },
	{ "wifi_isZeroDFSSupported",			1, "<RadioIndex>" },
	{ "wifi_setZeroDFSState",			3, "<RadioIndex> <ZeroDFSState Enable/"
							"Disable(1/0)> <Preclear Enable/Disable(1/0)>" },
	{ "wifi_getZeroDFSState",			1, "<RadioIndex>" },
	{ "wifi_getZeroWaitDFSChannelsStatus",		1, "<RadioIndex>" },
	{ "wifi_getCurrentRadioOperatingChannelBandwidth",	1, "<RadioIndex>" },
#endif /* RDKB_LGI */
	{ "wifi_setATMEnable",				1, "Enable/Disable(0/1)" },
	{ "wifi_getATMEnable",				0, "" },
	{ "wifi_setApATMAirTimePercent",		2, "<AP Index> <AP Air Time Percent>" },
	{ "wifi_getApATMAirTimePercent",		1, "<AP Index>" },
	{ "wifi_getApATMSta",				1, "<AP Index>" },
#if !defined(WIFI_HAL_VERSION_GE_3_0)
	{ "wifi_setApATMSta",				3, "<AP Index> <STA MAC Address> <STA Air Time Percent>" },
#endif
	{ NULL,						0, NULL },
};

#if WIFI_HAL_VERSION_GE_2_12 || defined(WIFI_HAL_VERSION_3)
/* 802.11K api */

#define WLC_RRM_MAX_MEAS_DUR 0x0400 /* MAx meas-dur if channel switch -- about 1sec */
#define NUM_INPARAM_PER_NBR 5
#define MAX_NBR_CNT  32

INT wifi_RMBeaconReport_callback_test_func(UINT apIndex, wifi_BeaconReport_t *out_struct,
	UINT *out_array_size, UCHAR *out_DialogToken);

/* 802.11K api */
#endif /* WIFI_HAL_VERSION_GE_2_12 || defined(WIFI_HAL_VERSION_3) */

INT wifi_getApBasicAuthenticationMode(INT apIndex, CHAR *authMode);
INT wifi_apAssociatedDevice_callback_test_func(INT apIndex, char *MAC, INT event_type);

/**
 * @brief wifi api set event parameters
 */
typedef struct {
	char		api_name[1024];
	int		radioIndex;
	char		api_data[1024];
} wifi_api_info_t;

static int  wifi_api_socket = -1;
extern pthread_t cbThreadId;

/* for Comcast/Plume WM */
#ifdef WIFI_HAL_VERSION_3_PHASE2
INT wifi_newApAssociatedDevice_callback_test_func(INT apIndex, wifi_associated_dev3_t *pdev)
#else /* WIFI_HAL_VERSION_3_PHASE2 */
INT wifi_newApAssociatedDevice_callback_test_func(INT apIndex, wifi_associated_dev_t *pdev)
#endif /* WIFI_HAL_VERSION_3_PHASE2 */
{
	printf("Called %s successfully \n", __FUNCTION__);
	printf("%s: apIndex - %d pdev - %p\n", __FUNCTION__, apIndex, pdev);
	if (pdev) {
		printf("cli_MACAddress = %02x:%02x:%02x:%02x:%02x:%02x\n",
			pdev->cli_MACAddress[0], pdev->cli_MACAddress[1], pdev->cli_MACAddress[2],
			pdev->cli_MACAddress[3], pdev->cli_MACAddress[4], pdev->cli_MACAddress[5]);
	}
	return 0;
}

INT wifi_apAssociatedDevice_callback_test_func(INT apIndex, char *MAC, INT event_type) {

	printf("Called %s successfully \n", __FUNCTION__);
	printf("%s: apIndex - %d MAC %s event_type - %d\n", __FUNCTION__, apIndex, MAC, event_type);

	return 0;
}

INT wifi_apDisassociatedDevice_callback_test_func(INT apIndex, char *MAC, INT event_type) {

	printf("Called %s successfully \n", __FUNCTION__);
	printf("%s: apIndex - %d MAC %s event_type - %d\n", __FUNCTION__, apIndex, MAC, event_type);

	return 0;
}

INT wifi_apDeAuthEvent_callback_test_func(INT apIndex, char *MAC, INT event_type) {

	printf("Called %s successfully \n", __FUNCTION__);
	printf("%s: apIndex - %d MAC %s event_type - %d\n", __FUNCTION__, apIndex, MAC, event_type);

	return 0;
}

void wifi_steering_event_callback_test_func(UINT steeringgroupIndex, wifi_steering_event_t *event) {
	printf("Called %s successfully \n", __FUNCTION__);
	printf("%s: groupIndex - %d event_type - %d\n", __FUNCTION__, steeringgroupIndex, event->type);
	return;
}

#if WIFI_HAL_VERSION_GE_2_12 || defined(WIFI_HAL_VERSION_3)
/* 802.11K api */

INT wifi_RMBeaconReport_callback_test_func(UINT apIndex, wifi_BeaconReport_t *rm_bcnRep_in,
	UINT *out_array_size, UCHAR *out_DialogToken)
{
	UCHAR *ea;
	wifi_BeaconReport_t *rm_bcnRep = rm_bcnRep_in;
	int i, cnt = *out_array_size;
	// API_DBG_PRINT_BUF(__FUNCTION__, (unsigned char *)out_struct, 64);
	if ((rm_bcnRep == NULL) || (cnt == 0)) {
		printf("%s: ap %d Null Beacon Report \n", __FUNCTION__, apIndex);
	}
	printf("%s ap %d cnt=%d size=%d *out_DialogToken=%d\n", __FUNCTION__, apIndex,
		cnt, sizeof(wifi_BeaconReport_t), *out_DialogToken);
	for (i = 0; i < cnt; i++) {
		ea = rm_bcnRep->bssid;
 		printf("<802.11K Rx BcnRep> %s: ap %d opClass: %d channel: %d, dur: %d, frame info: %d,"
			"rcpi: %d, rsni: %d, bssid: %02x:%02x:%02x:%02x:%02x:%02x, antenna id: %d,"
			" parent tsf: %u out_DialogToken=%d i=%d\n",
			__FUNCTION__, apIndex, rm_bcnRep->opClass, rm_bcnRep->channel,
			rm_bcnRep->duration, rm_bcnRep->frameInfo, rm_bcnRep->rcpi, rm_bcnRep->rsni,
			ea[0], ea[1], ea[2], ea[3], ea[4], ea[5],
			rm_bcnRep->antenna, rm_bcnRep->tsf, *out_DialogToken, i);
		rm_bcnRep++;
	}
	printf("\n---------------\n");
	free(rm_bcnRep_in);
	return 0;
}
/* 802.11K api */
#endif /* WIFI_HAL_VERSION_GE_2_12 || defined(WIFI_HAL_VERSION_3) */

/* 802.11V support */
#ifndef DOT11_BSSTRANS_REQMODE_PREF_LIST_INCL
/* BSS Mgmt Transition Request Mode Field - 802.11v */
#define DOT11_BSSTRANS_REQMODE_PREF_LIST_INCL		0x01
#define DOT11_BSSTRANS_REQMODE_ABRIDGED			0x02
#define DOT11_BSSTRANS_REQMODE_DISASSOC_IMMINENT	0x04
#define DOT11_BSSTRANS_REQMODE_BSS_TERM_INCL		0x08
#define DOT11_BSSTRANS_REQMODE_ESS_DISASSOC_IMNT	0x10
#endif /* DOT11_BSSTRANS_REQMODE_PREF_LIST_INCL */

#define BTM_MAX_CANDIDATES 4

#ifdef WIFI_HAL_VERSION_3_PHASE2
INT wifi_BTMQueryRequest_callback_test_func(UINT apIndex, mac_address_t peerMac,
	wifi_BTMQuery_t *inQueryFrame, UINT inMemSize, wifi_BTMRequest_t *inRequestFrame)
{
	UNUSED_PARAMETER(inMemSize);
	UNUSED_PARAMETER(inRequestFrame);
	printf("Called %s successfully inQueryFrame=%p\n", __FUNCTION__, inQueryFrame);

	if (inQueryFrame) {
		printf("%s: apIndex=%d MAC="MACF" token=0x%x\n",
			__FUNCTION__, apIndex, MAC_TO_MACF(peerMac), inQueryFrame->token);
	}
	/* todo */
	return 0;
}

INT wifi_BTMResponse_callback_test_func(UINT apIndex, mac_address_t peerMac,
	wifi_BTMResponse_t *in_struct)
{
	printf("Called %s successfully wifi_BTMResponse_t=%p\n", __FUNCTION__, in_struct);

	if (in_struct) {
		printf("%s: apIndex=%d MAC="MACF" token=0x%x\n",
			__FUNCTION__, apIndex, MAC_TO_MACF(peerMac), in_struct->token);
	}

	/* todo */
	return 0;
}
#else
INT wifi_BTMQueryRequest_callback_test_func(UINT apIndex, CHAR peerMACAddress[6],
	wifi_BTMQuery_t *inQueryFrame, UINT inMemSize, wifi_BTMRequest_t *inRequestFrame)
{
	UCHAR *ptr = (UCHAR *)peerMACAddress;

	UNUSED_PARAMETER(inMemSize);
	UNUSED_PARAMETER(inRequestFrame);
	printf("Called %s successfully inQueryFrame=%p\n", __FUNCTION__, inQueryFrame);

	if (inQueryFrame) {
		printf("%s: apIndex=%d MAC="MACF" token=0x%x\n",
			__FUNCTION__, apIndex, MAC_TO_MACF(ptr), inQueryFrame->token);
	}
	/* todo */
	return 0;
}

INT wifi_BTMResponse_callback_test_func(UINT apIndex, CHAR peerMACAddress[6],
	wifi_BTMResponse_t *in_struct)
{
	UCHAR *ptr = (UCHAR *)peerMACAddress;
	printf("Called %s successfully wifi_BTMResponse_t=%p\n", __FUNCTION__, in_struct);

	if (in_struct) {
		printf("%s: apIndex=%d MAC="MACF" token=0x%x\n",
			__FUNCTION__, apIndex, MAC_TO_MACF(ptr), in_struct->token);
	}

	/* todo */
	return 0;
}
#endif /* WIFI_HAL_VERSION_3 */

#if WIFI_HAL_VERSION_GE_2_16 || defined(WIFI_HAL_VERSION_3)
#if WIFI_HAL_VERSION_GE_2_19 || defined(WIFI_HAL_VERSION_3)
/* prototype defined in wifi_hal.h
#ifdef WIFI_HAL_VERSION_3_PHASE2
typedef INT (* wifi_receivedMgmtFrame_callback)(INT apIndex, mac_address_t sta_mac, UCHAR *frame, UINT len, wifi_mgmtFrameType_t type, wifi_direction_t dir);
#else
typedef INT (* wifi_receivedMgmtFrame_callback)(INT apIndex, UCHAR *sta_mac, UCHAR *frame, UINT len, wifi_mgmtFrameType_t type, wifi_direction_t dir);
#endif
*/
#ifdef WIFI_HAL_VERSION_3_PHASE2
INT wifi_receivedMgmtFrame_callback_test_func(INT apIndex, mac_address_t sta_mac, UCHAR *frame,
	UINT len, wifi_mgmtFrameType_t type, wifi_direction_t dir)
#else
INT wifi_receivedMgmtFrame_callback_test_func(INT apIndex, UCHAR *sta_mac, UCHAR *frame,
	UINT len, wifi_mgmtFrameType_t type, wifi_direction_t dir)
#endif /* WIFI_HAL_VERSION_3_PHASE2 */
{
	unsigned int i;

	UNUSED_PARAMETER(dir);
	printf("%s: successfully invoked apIndex=%d MAC="MACF" len=%d type=%d\n",
		__FUNCTION__, apIndex, MAC_TO_MACF(sta_mac), len, type);

	/* DEBUG */
	printf("==== MgmtFrame ===\n");
	for(i = 0; i < len; i++) {
		printf("%02x", (unsigned char)frame[i]);
		if (!((i+1)%2))
			printf(" ");
		if (!((i+1)%16))
			printf("\n");
	}
	printf("\n");

	return 0;
}
#else
/* DPP support, need some definition in new wifi_hal.h  */
/* Format changed since 2.15
typedef void (*wifi_dppAuthResponse_callback_t)(UINT apIndex,
						mac_address_t sta,
						UCHAR *frame,
						UINT len);

typedef void (*wifi_dppConfigRequest_callback_t)(UINT apIndex,
						mac_address_t sta,
						UCHAR token,
						UCHAR *attribs,
						UINT length);
*/

void wifi_dppAuthResponse_callback_test_func(UINT apIndex, mac_address_t sta, UCHAR *frame,
	UINT len)
{
	UCHAR *ptr = (UCHAR *)sta;

	printf("%s: successfully invoked apIndex=%d MAC="MACF" len=%d\n",
			__FUNCTION__, apIndex, MAC_TO_MACF(ptr), len);
}

void wifi_dppConfigRequest_callback_test_func(UINT apIndex, mac_address_t sta, UCHAR token,
	UCHAR *attribs, UINT length)
{
	UCHAR *ptr = (UCHAR *)sta;

	printf("%s: successfully invoked apIndex=%d MAC="MACF" token=%x length=%d\n",
			__FUNCTION__, apIndex, MAC_TO_MACF(ptr), token, length);

	if (attribs) {
		int i;
		printf("==== dppConfigRequest attributes ===\n");
		for(i = 0; i < length; i++) {
			printf("%02x", (unsigned char)attribs[i]);
			if (!((i+1)%2))
				printf(" ");
			if (!((i+1)%16))
				printf("\n");
		}
		printf("\n");
	}
}
#endif /* WIFI_HAL_VERSION_GE_2_19 */
#endif /* WIFI_HAL_VERSION_GE_2_16 */

#if WIFI_HAL_VERSION_GE_2_18 || defined(WIFI_HAL_VERSION_3)
static int test_wifi_getRadioChannels(int index)
{
	int ret = 0;
	wifi_channelMap_t *output_map = NULL;
	INT output_map_size = 0, num_channels = 0, i;
	char channels[512], *pch, *saveptr;

	ret = wifi_getRadioPossibleChannels(index, channels);
	if (ret == RETURN_ERR) {
		printf("%s wifi_getRadioPossibleChannels returned ERROR\n", __FUNCTION__);
		return ret;
	}
	/* Get num possible channels */
	pch = strtok_r(channels, ",", &saveptr);
	while (pch != NULL) {
		num_channels++;
		pch = strtok_r(NULL, ",", &saveptr);
	}
	if (!num_channels) {
		printf("%s wifi_getRadioPossibleChannels returned zero channels\n",
			__FUNCTION__);
		return RETURN_ERR;
	}

	output_map_size = num_channels * sizeof(wifi_channelMap_t);
	output_map = malloc (output_map_size);
	if (output_map == NULL) {
		printf("%s output_map malloc failed\n", __FUNCTION__);
		return RETURN_ERR;
	}
	ret = wifi_getRadioChannels(index, output_map, num_channels);
	if (ret == RETURN_ERR) {
		printf("%s wifi_getRadioChannels returned ERROR\n",
		__FUNCTION__);
		free(output_map);
		return ret;
	}

	for (i = 0; i < num_channels; i++) {
		wifi_channelState_t ch_state = output_map[i].ch_state;
		printf("ch_number:%d ch_state:%s\n",
			output_map[i].ch_number,
			(ch_state == CHAN_STATE_DFS_CAC_COMPLETED) ?
				"CHAN_STATE_DFS_CAC_COMPLETED" :
			(ch_state == CHAN_STATE_DFS_CAC_START) ?
				"CHAN_STATE_DFS_CAC_START" :
			(ch_state == CHAN_STATE_DFS_NOP_START) ?
				"CHAN_STATE_DFS_NOP_START" :
			(ch_state == CHAN_STATE_DFS_NOP_FINISHED) ?
				"CHAN_STATE_DFS_NOP_FINISHED" :
			"CHAN_STATE_AVAILABLE");
	}

	free(output_map);
	return ret;
}

static void chan_event_cb(UINT radioIndex,
	wifi_chan_eventType_t event, UCHAR channel)
{
	printf("Called %s successfully \n", __FUNCTION__);
	printf("%s: radioIndex - %d event - %d channel - %d\n",
		__FUNCTION__, radioIndex, event, channel);
	/* Wait a bit for channels states to be updated */
	sleep(1);
	test_wifi_getRadioChannels(radioIndex);
}

#endif /* WIFI_HAL_VERSION_GE_2_18 */
static int wifi_api_send_msg( wifi_api_info_t *p_apiInfo )
{
	int err = -1;
	struct sockaddr_in sockaddr;
	int sentBytes = 0;

        memset(&sockaddr, 0, sizeof(sockaddr));
        sockaddr.sin_family = AF_INET;
        sockaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        sockaddr.sin_port = htons(WIFI_API_EVENT_UDP_SPORT);

	if (wifi_api_socket < 0) {
		if (inet_aton(WIFI_API_EVENT_UDP_SIP, &sockaddr.sin_addr) == 0) {
			printf("inet_aton() failed\n");
			return err;
		}

		if (( wifi_api_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
			printf("%s@%d Unable to create socket\n", __FUNCTION__, __LINE__ );
			return err;
		}
	}

	sentBytes = sendto(wifi_api_socket, p_apiInfo, sizeof(wifi_api_info_t), 0,
		(struct sockaddr *)&sockaddr, sizeof(struct sockaddr_in));

	//printf("UDP pkt sent; sendingBytes[%d], sentBytes[%d]\n", sizeof(wifi_api_info_t), sentBytes);
	if (sentBytes != sizeof(wifi_api_info_t)) {
		printf("UDP send failed; sendingBytes[%d], sentBytes[%d]\n",
			sizeof(wifi_api_info_t), sentBytes);
	}
	else {
		int ret = 0;
		struct sockaddr_in from;
		socklen_t sock_len = sizeof(from);

		/* Get the execution result from socket */
		if (recvfrom(wifi_api_socket, &ret, sizeof(ret), 0,
			(struct sockaddr *)&from, &sock_len) < 0) {
			printf("%s: %s recvfrom error %d!\n", __FUNCTION__,
				p_apiInfo->api_name, errno);
		}
		else {
			err = ret;
		}
	}

	//printf("%s@%d: Close socket\n", __FUNCTION__, __LINE__ );
	close(wifi_api_socket);
	wifi_api_socket = -1;

	return err;
}

void parse_mac(char *macstr, mac_address_t *macp) {
	unsigned char *mac = &(*macp)[0];
	int ret = sscanf(macstr, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx",
		&mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
	if (ret != 6) {
		printf("sscanf()=%d of mac address failed", ret);
		exit(1);
	}
}

static BOOL isNumeric(const char *input) {
	while (*input) {
		if (isdigit(*input)) {
			input++;
		}
		else return FALSE;
	}
	return TRUE;
}

BOOL printUsage(char *apiName, int numArgs, BOOL isCmd) {
	int i = 0;
	int validCmd = 0;
	if (apiName) {
		while (hal_cmd_table[i].api_name) {
			if (!strcmp(hal_cmd_table[i].api_name, apiName)) {
				validCmd = 1;

				if ((numArgs < hal_cmd_table[i].num_args) || !isCmd) {
					printf("Usage : wifi_api %s %s\n\n", apiName,
						hal_cmd_table[i].usage);
					return FALSE;
				} else {
					return TRUE;
				}
			}
			i++;
		}
		if (!validCmd) {
			printf("%s : Not supported\n\n", apiName);
			return FALSE;
		}
	} else {
		printf("\n****************HAL API's****************\n");
		printf("*****************************************\n\n");
		//Print all api's
		while (hal_cmd_table[i].api_name) {
			printf("%-50s	%s\n", hal_cmd_table[i].api_name, hal_cmd_table[i].usage);
			i += 1;
		}
		return FALSE;
	}
	return FALSE;
}

extern int wldm_init(int radios);
extern INT wifi_getRadioDfsMoveBackEnable(INT radioIndex, BOOL *output_bool);
#ifndef MAX_CH_LIST_LEN
#define MAX_CH_LIST_LEN 128
#endif /* MAX_CH_LIST_LEN */

#if WIFI_HAL_VERSION_GE_2_19 && !defined(_CBR1_PRODUCT_REQ_) && \
	!defined(_XF3_PRODUCT_REQ_) && !defined(_SKY_HUB_COMMON_PRODUCT_REQ_) && \
	defined(CMWIFI_RDKB_COMCAST)
static unsigned int csi_recv = 0;
/* Sample csi callback function */
static void wifi_csi_proc(mac_address_t mac_addr, wifi_csi_data_t *csi_data)
{
	wifi_frame_info_t *frame_info;
	wifi_csi_matrix_t *csi_matrix;
	unsigned int *raw_data;
	int i, j;

	if (!csi_data) {
		WL_CSI_ERR("%s: Error... csi_data is NULL\n", __FUNCTION__);
		return;
	}

	csi_recv++;
	frame_info = &(csi_data->frame_info);
	csi_matrix = &(csi_data->csi_matrix);
	raw_data = (unsigned int *)csi_matrix;
	if (WL_CSI_VERBOSE_ENABLED) {
		WL_CSI_PRT("%s csi_recv:[%d]\n", __FUNCTION__, csi_recv);
		WL_CSI_PRT("csi: "MACF"\n", MAC_TO_MACF(mac_addr));

		WL_CSI_PRT("csi_data: %p  frame_info: %p\n", (void *)csi_data, (void *)frame_info);
		WL_CSI_PRT("bw_mode:%d\n", frame_info->bw_mode);
		WL_CSI_PRT("mcs:%d\n", frame_info->mcs);
		WL_CSI_PRT("Nr:%d\n", frame_info->Nr);
		WL_CSI_PRT("Nc:%d\n", frame_info->Nc);
		WL_CSI_PRT("nr_rssi:%02x:%02x:%02x:%02x\n", frame_info->nr_rssi[0],
			frame_info->nr_rssi[1], frame_info->nr_rssi[2], frame_info->nr_rssi[3]);
		WL_CSI_PRT("valid_mask:0x%x\n", frame_info->valid_mask);
		WL_CSI_PRT("phy_bw:%d\n", frame_info->phy_bw);
		WL_CSI_PRT("cap_bw:%d\n", frame_info->cap_bw);
		WL_CSI_PRT("num_sc:%d\n", frame_info->num_sc);
		WL_CSI_PRT("decimation:%d\n", frame_info->decimation);
		WL_CSI_PRT("channel:%d\n", frame_info->channel);
		WL_CSI_PRT("cfo:0x%x\n", frame_info->cfo);
		WL_CSI_PRT("time_stamp:0x%llx [%llu]\n", frame_info->time_stamp, frame_info->time_stamp);

		WL_CSI_PRT("CSI matrix:%p\n", raw_data);
		for (i = 0; i < 4; i++) {
			for (j = 0; j < 8; j++) {
				WL_CSI_PRT("0x%08x\t", raw_data[i * 8 + j]);
			}
			WL_CSI_PRT("\n");
		}
	}
	return;
}
#endif /* WIFI_HAL_VERSION_GE_2_19 && !_CBR1_PRODUCT_REQ_ && */
	/* !_XF3_PRODUCT_REQ_ && !_SKY_HUB_COMMON_PRODUCT_REQ_ && CMWIFI_RDKB_COMCAST */

int main(int argc, char **argv) {
	INT ret = 0;
	INT index = 0;

	FILE *fptr = fopen(WLAN_APPS_LOG_FILE, "a+");
	if (fptr == NULL) {
		printf("Could not open file");
		return 0;
	}
	if (argc > 1) {
		int i = 0;
		time_t mytime;
		char ctime_buf[32] = {0};
		mytime = time(NULL);
		ctime_r(&mytime, ctime_buf);
		fprintf(fptr, "%s ==> %s called", ctime_buf, argv[1]);
		if (argc > 2) {
			fprintf(fptr, " with parameters");
			for (i = 2; i < argc; i++) {
				fprintf(fptr, " %s", argv[i]);
			}
		}
		fprintf(fptr, "\n");
	}
	fclose(fptr);

	if (argc == 1) {
		printf("\nwifi_api : missing arguments\n");
		printf("\nTry wifi_api --help or wifi_api --help <API-NAME> for more options\n\n");
		return 0;
	}
	if (((argc >= 2) && !strncmp(argv[1], "--help", 6)) || ((argc > 2) &&
		!strncmp(argv[2], "--help", 6))) {
		if (argc > 2) {
			/* wifi_api --help <API-NAME> or wifi_api <API-NAME> --help
			prints Usage of the given API. */
			if (strncmp(argv[2], "--help", 6)) {
				printUsage(argv[2], 0, FALSE);
			} else {
				printUsage(argv[1], 0, FALSE);
			}
		} else {
			// wifi_api --help prints all API List.
			printUsage(NULL, 0, FALSE);
		}
		return 0;
	}
	if (!printUsage(argv[1], argc - 2, TRUE)) return 0;

	if (argc > 2 && argv[2] != NULL && strcmp(argv[1], "wifi_getIndexFromName") &&
		strcmp(argv[1], "wifi_getRMCapabilities") &&
		strcmp(argv[1], "wifi_setBandSteeringApGroup") &&
		strcmp(argv[1], "wifi_setWldmMsglevel")) {
		if (!isNumeric(argv[2])) {
			printf("Incorrect input\n");
			printUsage(argv[1], 0, FALSE);
			return -1;
		}
		index = atoi(argv[2]);
	}

	if (strcmp(argv[1], "wifi_init")) {
		wldm_init(-1);
	}
	if (!strcmp(argv[1], "wifi_getBandSteeringEnable")) {
		BOOL enable;
		ret = wifi_getBandSteeringEnable(&enable);
		if (ret == RETURN_OK) {
			printf("%d\n", enable);
		}
	} else if (!strcmp(argv[1], "wifi_down")) {
		wifi_api_info_t apiInfo;
		strncpy(apiInfo.api_name, argv[1], 1024);
		ret = wifi_api_send_msg(&apiInfo);
	} else if (!strcmp(argv[1], "wifi_init")) {
#if defined(BUILD_RDKWIFI) && !defined(BCA_CPEROUTER_RDK)
		if (daemon(1, 1) != -1) {
			ret = wifi_init();
			if (ret == RETURN_OK && wifi_apiThread) {
				ret = pthread_join(wifi_apiThread, NULL);
			}
		}
#else
		ret = wifi_init();
#endif /* defined(BUILD_RDKWIFI) && !defined(BCA_CPEROUTER_RDK) */
	} else if (!strcmp(argv[1], "wifi_apply")) {
		wifi_api_info_t apiInfo;
		strncpy(apiInfo.api_name, argv[1], 1024);
		ret = wifi_api_send_msg(&apiInfo);
		if (ret == RETURN_OK) {
			printf("%s complete.\n", argv[1]);
		}
	 } else if (!strcmp(argv[1], "wifi_getHalVersion")) {
		char version[64];
		ret = wifi_getHalVersion(version);
		if (ret == RETURN_OK) {
			printf("%s\n", version);
		}
	} else if (!strcmp(argv[1], "wifi_setWldmMsglevel")) {
		wifi_api_info_t apiInfo;
		strncpy(apiInfo.api_name, argv[1], 1024);
		strncpy(apiInfo.api_data, argv[2], 1024);
		ret = wifi_api_send_msg(&apiInfo);
	} else if (!strcmp(argv[1], "wifi_getRadioNumberOfEntries")) {
		unsigned long numRadio;
		ret = wifi_getRadioNumberOfEntries(&numRadio);
		if (ret == RETURN_OK) {
			printf("%ld\n", numRadio);
		}
	} else if (!strcmp(argv[1], "wifi_getSSIDNumberOfEntries")) {
		unsigned long numSsid;
		ret = wifi_getSSIDNumberOfEntries(&numSsid);
		if (ret == RETURN_OK) {
			printf("%ld\n", numSsid);
		}
	} else if (!strcmp(argv[1], "wifi_getBandSteeringApGroup")) {
		char apgroup[64];
		ret = wifi_getBandSteeringApGroup(apgroup);
		if (ret == RETURN_OK) {
			printf("%s\n", apgroup);
		}
	} else if (!strcmp(argv[1], "wifi_factoryReset")) {
		ret = wifi_factoryReset();
		if (ret == RETURN_OK) {
			if (atoi(argv[2])) {
				ret = system("reboot");
			}
#if defined(BCA_CPEROUTER_RDK)
			else  {
				ret = wifi_factoryReset_post(-1, 1, 1);
			}
#endif /* BCA_CPEROUTER_RDK */
		}
	} else if (!strcmp(argv[1], "wifi_factoryResetRadios")) {
		ret = wifi_factoryResetRadios();
		if (ret == RETURN_OK) {
			ret = wifi_factoryReset_post(-1, atoi(argv[2]), atoi(argv[3]));
		}
	} else if (!strcmp(argv[1], "wifi_factoryResetRadio")) {
		ret = wifi_factoryResetRadio(index);
		if (ret == RETURN_OK) {
			ret = wifi_factoryReset_post(index, atoi(argv[3]), atoi(argv[4]));
		}
	} else if (!strcmp(argv[1], "wifi_factoryResetAP")) {
		ret = wifi_factoryResetAP(index);
		if (ret == RETURN_OK) {
			ret = wifi_factoryReset_post(index, atoi(argv[3]), atoi(argv[4]));
		}
	} else if (!strcmp(argv[1], "wifi_setApSecurityReset")) {
		ret = wifi_setApSecurityReset(index);
		if (ret == RETURN_OK) {
			ret = wifi_factoryReset_post(index, atoi(argv[3]), atoi(argv[4]));
		}
	} else if (!strcmp(argv[1], "wifi_reset")) {
		ret = wifi_reset();
	} else if (!strcmp(argv[1], "wifi_getRadioEnable")) {
		BOOL enable;
		ret = wifi_getRadioEnable(index, &enable);
		if (ret == RETURN_OK) {
			printf("%s\n", enable ? "TRUE" : "FALSE");
		}
	} else if (!strcmp(argv[1], "wifi_getRadioUpTime")) {
		ULONG uptime;
		ret = wifi_getRadioUpTime(index, &uptime);
		if (ret == RETURN_OK) {
			printf("%lu\n", uptime);
		}
	} else if (!strcmp(argv[1], "wifi_getRadioStatus")) {
		BOOL enable;
		ret = wifi_getRadioStatus(index, &enable);
		if (ret == RETURN_OK) {
			printf("%s\n", enable ? "TRUE" : "FALSE");
		}
	} else if (!strcmp(argv[1], "wifi_getSSIDRadioIndex")) {
		int radioIndexOut;
		ret = wifi_getSSIDRadioIndex(index, &radioIndexOut);
		if (ret == RETURN_OK) {
			printf("%d\n", radioIndexOut);
		}
	} else if (!strcmp(argv[1], "wifi_getRadioResetCount")) {
		ULONG resetCount;
		ret = wifi_getRadioResetCount(index, &resetCount);
		if (ret == RETURN_OK) {
			printf("%lu\n", resetCount);
		}
	} else if (!strcmp(argv[1], "wifi_getApIsolationEnable")) {
		BOOL apIsolationEnable;
		ret = wifi_getApIsolationEnable(index, &apIsolationEnable);
		if (ret == RETURN_OK) {
			printf("%s\n", apIsolationEnable ? "TRUE" : "FALSE");
		}
	} else if (!strcmp(argv[1], "wifi_getRadioIfName")) {
		char name[64];
		ret = wifi_getRadioIfName(index, name);
		if (ret == RETURN_OK) {
			printf("%s\n", name);
		}
	} else if (!strcmp(argv[1], "wifi_getRadioMaxBitRate")) {
		char bitrate[64];
		ret = wifi_getRadioMaxBitRate(index, bitrate);
		if (ret == RETURN_OK) {
			printf("%s\n", bitrate);
		}
	} else if (!strcmp(argv[1], "wifi_getRadioSupportedFrequencyBands")) {
		char freqbands[64];
		ret = wifi_getRadioSupportedFrequencyBands(index, freqbands);
		if (ret == RETURN_OK) {
			printf("%s\n", freqbands);
		}
	} else if (!strcmp(argv[1], "wifi_getRadioOperatingFrequencyBand")) {
		char freqbands[64];
		ret = wifi_getRadioOperatingFrequencyBand(index, freqbands);
		if (ret == RETURN_OK) {
			printf("%s\n", freqbands);
		}
	} else if (!strcmp(argv[1], "wifi_getRadioSupportedStandards")) {
		char standards[64];
		ret = wifi_getRadioSupportedStandards(index, standards);
		if (ret == RETURN_OK) {
			printf("%s\n", standards);
		}
	/* Deprecated from WIFI_HAL_MAJOR_VERSION >= 2  && WIFI_HAL_MINOR_VERSION >= 15 */
	} else if (!strcmp(argv[1], "wifi_getRadioStandard")) {
		char standard[64];
		BOOL gOnly, nOnly, acOnly;
		ret = wifi_getRadioStandard(index, standard, &gOnly, &nOnly, &acOnly);
		if (ret == RETURN_OK) {
			printf("index=%d standard=%s gOnly=%d nOnly=%d acOnly=%d\n",
				index, standard, gOnly, nOnly, acOnly);
		}
	} else if (!strcmp(argv[1], "wifi_getSSIDEnable")) {
		BOOL enable;
		ret = wifi_getSSIDEnable(index, &enable);
		if (ret == RETURN_OK) {
			printf("%s\n", enable ? "TRUE" : "FALSE");
		}
	} else if (!strcmp(argv[1], "wifi_getSSIDStatus")) {
		char status[64];
		ret = wifi_getSSIDStatus(index, status);
		if (ret == RETURN_OK) {
			printf("%s\n", status);
		}
	} else if (!strcmp(argv[1], "wifi_getSSIDName")) {
		char name[64];
		ret = wifi_getSSIDName(index, name);
		if (ret == RETURN_OK) {
			printf("%s\n", name);
		}
	} else if (!strcmp(argv[1], "wifi_getRadioChannelStats")) {
		wifi_channelStats_t ch_stats[OUTPUT_STRING_LENGTH_128], *s;
		int i = 0, n = 0, chan_num;
		char channels[OUTPUT_STRING_LENGTH_128] = {0}, *tok, *rest = NULL;
		memset(&ch_stats, 0, sizeof(ch_stats));
		ret = wifi_getRadioPossibleChannels(index, channels);
		if (ret == RETURN_OK) {
			tok = strtok_r(channels, ",", &rest);
			while (tok) {
				sscanf(tok, "%d", &chan_num);
				ch_stats[i].ch_number = chan_num;
				ch_stats[i].ch_in_pool = TRUE;
				i++;
				tok = strtok_r(NULL, ",", &rest);
			}
			printf("wifi_getRadioChannelStats(index[%d]) num of chan=%d\n", index, i);
			ret = wifi_getRadioChannelStats(index, ch_stats, i);
			printf("return: %d\n", ret);
			if (ret == RETURN_OK) {
				for (i = 0; i < (int)ARRAY_SIZE(ch_stats); i++) {
					s = &ch_stats[i];
					if (!s->ch_in_pool) continue;
					if (!s->ch_utilization_busy_tx
						&& !s->ch_utilization_busy_rx
						&& !s->ch_utilization_busy
						&& !s->ch_utilization_total
						&& !s->ch_noise) continue;
					printf("  ch %3u tx %10"PRIu64" rx %10"PRIu64" busy %10"
						PRIu64" total %10"PRIu64" noise %d\n",
						s->ch_number,
						s->ch_utilization_busy_tx,
						s->ch_utilization_busy_rx,
						s->ch_utilization_busy,
						s->ch_utilization_total,
						s->ch_noise);
					n++;
				}
				printf("channels with stats: %d\n", n);
                        }
		}
	} else if (!strcmp(argv[1], "wifi_getApAssociatedDeviceRxStatsResult")) {
		mac_address_t mac;
		wifi_associated_dev_rate_info_rx_stats_t * stats_array,*s;
		UINT output_array_size;
		ULLONG handle;
		unsigned int i, j;
		int k;
		INT ret = 0;

		parse_mac(argv[3], &mac);
		ret = wifi_getApAssociatedDeviceRxStatsResult(index, &mac, &stats_array,
			&output_array_size, &handle);
		if (ret == RETURN_ERR) {
			printf("%s returned error\n", argv[1]);
			printUsage(argv[1], 0, FALSE);
			return ret;
		}
		printf("wifi_getApAssociatedDeviceRxStatsResult(%d %s)\n", index, argv[3]);
		printf("output_array_size: %d\n", output_array_size);

		PRINT_HEX1(handle);

		if (opt_compact) {
			print_compact = TRUE;
			printf("  [i  bw   nss  mcs] flag byte msdu mpdu ppdu retr rssi\n");
		}
		for (i = 0; i < output_array_size; i++) {
			if (opt_compact) printf("   %-2d ", i);
			else printf("  [%d]\n", i);
			s = &stats_array[i];
			PRINT_INT(s, bw);
			PRINT_INT(s, nss);
			PRINT_INT(s, mcs);
			PRINT_HEX(s, flags);
			PRINT_INT(s, bytes);
			PRINT_INT(s, msdus);
			PRINT_INT(s, mpdus);
			PRINT_INT(s, ppdus);
			PRINT_INT(s, retries);
			PRINT_INT(s, rssi_combined);
			if (opt_compact) {
				printf("[");
				for (j = 0; j < 8; j++) {
					if (j) printf(",");
					for (k = 0; k < 4; k++) {
						printf(" %2d", s->rssi_array[j][k]);
					}
				}
				printf(" ]\n");
			} else {
				printf("  rssi_array:\n");
				for (j = 0; j < 8; j++) {
					printf("	 ");
					for (k = 0; k < 4; k++) {
						printf("%-2d ", s->rssi_array[j][k]);
					}
					printf("\n");
				}
			}
		}
		print_compact = FALSE;
		free(stats_array);
	} else if (!strcmp(argv[1], "wifi_getApAssociatedDeviceTxStatsResult")) {
		mac_address_t mac;
		wifi_associated_dev_rate_info_tx_stats_t * stats_array,*s;
		UINT output_array_size;
		ULLONG handle;
		int i;
		INT ret = 0;

		parse_mac(argv[3], &mac);

		ret = wifi_getApAssociatedDeviceTxStatsResult(index, &mac, &stats_array,
			&output_array_size, &handle);
		if (ret == RETURN_ERR) {
			printf("%s returned error\n", argv[1]);
			printUsage(argv[1], 0, FALSE);
			return ret;
		}
		printf("wifi_getApAssociatedDeviceTxStatsResult(%d %s)\n", index, argv[3]);
		printf("output_array_size: %d\n", output_array_size);
		PRINT_HEX1(handle);

		if (opt_compact) {
			print_compact = TRUE;
			printf("  [i  bw   nss  mcs] flag byte msdu mpdu ppdu retr attempts\n");
		}
		for (i = 0; i < (int)output_array_size; i++) {
			if (opt_compact) printf("   %-2d ", i);
			else printf("  [%d]\n", i);
			s = &stats_array[i];
			PRINT_INT(s, bw);
			PRINT_INT(s, nss);
			PRINT_INT(s, mcs);
			PRINT_HEX(s, flags);
			PRINT_INT(s, bytes);
			PRINT_INT(s, msdus);
			PRINT_INT(s, mpdus);
			PRINT_INT(s, ppdus);
			PRINT_INT(s, retries);
			PRINT_INT(s, attempts);
			if (opt_compact) printf("\n");
		}
		print_compact = FALSE;
		free(stats_array);
	} else if (!strcmp(argv[1], "wifi_getApAssociatedDeviceStats")) {
		mac_address_t mac;
		wifi_associated_dev_stats_t dev_stats, *s = &dev_stats;
		ULLONG handle;
		INT ret = 0;

		parse_mac(argv[3], &mac);

		ret = wifi_getApAssociatedDeviceStats(index, &mac, &dev_stats, &handle);
		if (ret == RETURN_OK) {
			printf("wifi_getApAssociatedDeviceStats(%d %s)\n", index, argv[3]);
			PRINT_HEX1(handle);
			PRINT_INT(s, cli_rx_bytes);
			PRINT_INT(s, cli_tx_bytes);
			PRINT_INT(s, cli_rx_frames);
			PRINT_INT(s, cli_tx_frames);
			PRINT_INT(s, cli_rx_retries);
			PRINT_INT(s, cli_tx_retries);
			PRINT_INT(s, cli_rx_errors);
			PRINT_INT(s, cli_tx_errors);
			PRINT_DOUBLE(s, cli_rx_rate);
			PRINT_DOUBLE(s, cli_tx_rate);
		}
	} else if (!strcmp(argv[1], "wifi_getBaseBSSID")) {
		char bssid[18];
		ret = wifi_getBaseBSSID(index, bssid);
		if (ret == RETURN_OK) {
			printf("%s\n", bssid);
		}
	} else if (!strcmp(argv[1], "wifi_getApEnable")) {
		BOOL enable;
		ret = wifi_getApEnable(index, &enable);
		if (ret == RETURN_OK) {
			printf("%s\n", enable ? "TRUE" : "FALSE");
		}
	} else if (!strcmp(argv[1], "wifi_getApStatus")) {
		char status[64];
		ret = wifi_getApStatus(index, status);
		if (ret == RETURN_OK) {
			printf("%s\n", status);
		}
	} else if (!strcmp(argv[1], "wifi_getSSIDTrafficStats2")) {
		wifi_ssidTrafficStats2_t stats = { 0 };
		ret = wifi_getSSIDTrafficStats2(index, &stats); //Tr181
		if (ret == RETURN_OK) {
			printf("%s %d\n", argv[1], index);
			printf("ssid_BytesSent			= %lu\n", stats.ssid_BytesSent);
			printf("ssid_BytesReceived		= %lu\n",
				stats.ssid_BytesReceived);
			printf("ssid_PacketsSent		= %lu\n", stats.ssid_PacketsSent);
			printf("ssid_PacketsReceived		= %lu\n",
				stats.ssid_PacketsReceived);
			printf("ssid_RetransCount		= %lu\n", stats.ssid_RetransCount);
			printf("ssid_FailedRetransCount		= %lu\n",
				stats.ssid_FailedRetransCount);
			printf("ssid_RetryCount			= %lu\n", stats.ssid_RetryCount);
			printf("ssid_MultipleRetryCount		= %lu\n",
				stats.ssid_MultipleRetryCount);
			printf("ssid_ACKFailureCount		= %lu\n",
				stats.ssid_ACKFailureCount);
			printf("ssid_AggregatedPacketCount	= %lu\n",
				stats.ssid_AggregatedPacketCount);
			printf("ssid_ErrorsSent			= %lu\n", stats.ssid_ErrorsSent);
			printf("ssid_ErrorsReceived		= %lu\n",
				stats.ssid_ErrorsReceived);
			printf("ssid_UnicastPacketsSent		= %lu\n",
				stats.ssid_UnicastPacketsSent);
			printf("ssid_UnicastPacketsReceived	= %lu\n",
				stats.ssid_UnicastPacketsReceived);
			printf("ssid_DiscardedPacketsSent	= %lu\n",
				stats.ssid_DiscardedPacketsSent);
			printf("ssid_DiscardedPacketsReceived	= %lu\n",
				stats.ssid_DiscardedPacketsReceived);
			printf("ssid_MulticastPacketsSent	= %lu\n",
				stats.ssid_MulticastPacketsSent);
			printf("ssid_MulticastPacketsReceived	= %lu\n",
				stats.ssid_MulticastPacketsReceived);
			printf("ssid_BroadcastPacketsSent	= %lu\n",
				stats.ssid_BroadcastPacketsSent);
			printf("ssid_BroadcastPacketsRecevied	= %lu\n",
				stats.ssid_BroadcastPacketsRecevied);
			printf("ssid_UnknownPacketsReceived	= %lu\n",
				stats.ssid_UnknownPacketsReceived);
		}
	} else if (!strcmp(argv[1], "wifi_getApAssociatedDeviceDiagnosticResult")) {
		wifi_associated_dev_t *associated_dev_array = NULL,*pt = NULL;
		UINT array_size = 0;
		UINT i = 0;
		ret = wifi_getApAssociatedDeviceDiagnosticResult(index, &associated_dev_array,
			&array_size);
		if (ret == RETURN_OK) {
			printf("Total_STA:%d\n", array_size);
			for (i = 0, pt = associated_dev_array; i < array_size; i++, pt++) {
				printf("\nsta_%d: cli_MACAddress\t\t\t:%02X:%02X:%02X:%02X:%02X:%02X\n",
					i + 1, pt->cli_MACAddress[0], pt->cli_MACAddress[1],
					pt->cli_MACAddress[2], pt->cli_MACAddress[3],
					pt->cli_MACAddress[4], pt->cli_MACAddress[5]);
				printf("sta_%d: cli_AuthenticationState\t\t:%d\n", i + 1,
					pt->cli_AuthenticationState);
				printf("sta_%d: cli_LastDataDownlinkRate\t\t:%d\n", i + 1,
					pt->cli_LastDataDownlinkRate);
				printf("sta_%d: cli_LastDataUplinkRate\t\t:%d\n", i + 1,
					pt->cli_LastDataUplinkRate);
				printf("sta_%d: cli_SignalStrength\t\t:%d\n", i + 1,
					pt->cli_SignalStrength);
				printf("sta_%d: cli_Retransmissions\t\t:%d\n", i + 1,
					pt->cli_Retransmissions);
				printf("sta_%d: cli_OperatingStandard\t\t:%s\n", i + 1,
					pt->cli_OperatingStandard);
				printf("sta_%d: cli_OperatingChannelBandwidth\t:%s\n", i + 1,
					pt->cli_OperatingChannelBandwidth);
				printf("sta_%d: cli_SNR\t\t\t\t:%d\n", i + 1,
					pt->cli_SNR);
				printf("sta_%d: cli_DataFramesSentAck\t\t:%lu\n", i + 1,
					pt->cli_DataFramesSentAck);
				printf("sta_%d: cli_DataFramesSentNoAck\t\t:%lu\n", i + 1,
					pt->cli_DataFramesSentNoAck);
				printf("sta_%d: cli_RSSI\t\t\t\t:%d\n", i + 1,
					pt->cli_RSSI);
				printf("sta_%d: cli_Disassociations\t\t:%d\n", i + 1,
					pt->cli_Disassociations);
				printf("sta_%d: cli_AuthenticationFailures\t:%d\n", i + 1,
					pt->cli_AuthenticationFailures);
			}
		}
		if (associated_dev_array) {
			free(associated_dev_array);
		}
	} else if (!strcmp(argv[1], "wifi_getApAssociatedDeviceDiagnosticResult2")) {
		wifi_associated_dev2_t *associated_dev_array = NULL,*pt = NULL;
		UINT array_size = 0;
		UINT i = 0;
		ret = wifi_getApAssociatedDeviceDiagnosticResult2(index, &associated_dev_array, &array_size);
		if (ret == RETURN_OK) {
			printf("Total_STA:%d\n", array_size);
			for (i = 0, pt = associated_dev_array; i < array_size; i++, pt++) {
				printf("\nsta_%d: cli_MACAddress\t\t\t:%02X:%02X:%02X:%02X:%02X:%02X\n",
					i + 1, pt->cli_MACAddress[0], pt->cli_MACAddress[1],
					pt->cli_MACAddress[2], pt->cli_MACAddress[3],
					pt->cli_MACAddress[4], pt->cli_MACAddress[5]);
				printf("sta_%d: cli_AuthenticationState\t\t:%d\n", i + 1,
					pt->cli_AuthenticationState);
				printf("sta_%d: cli_LastDataDownlinkRate\t\t:%d\n", i + 1,
					pt->cli_LastDataDownlinkRate);
				printf("sta_%d: cli_LastDataUplinkRate\t\t:%d\n", i + 1,
					pt->cli_LastDataUplinkRate);
				printf("sta_%d: cli_SignalStrength\t\t:%d\n", i + 1,
					pt->cli_SignalStrength);
				printf("sta_%d: cli_Retransmissions\t\t:%d\n", i + 1,
					pt->cli_Retransmissions);
				printf("sta_%d: cli_OperatingStandard\t\t:%s\n", i + 1,
					pt->cli_OperatingStandard);
				printf("sta_%d: cli_OperatingChannelBandwidth\t:%s\n", i + 1,
					pt->cli_OperatingChannelBandwidth);
				printf("sta_%d: cli_SNR\t\t\t\t:%d\n", i + 1,
					pt->cli_SNR);
				printf("sta_%d: cli_DataFramesSentAck\t\t:%lu\n", i + 1,
					pt->cli_DataFramesSentAck);
				printf("sta_%d: cli_DataFramesSentNoAck\t\t:%lu\n", i + 1,
					pt->cli_DataFramesSentNoAck);
				printf("sta_%d: cli_RSSI\t\t\t\t:%d\n", i + 1,
					pt->cli_RSSI);
				printf("sta_%d: cli_Disassociations\t\t:%d\n", i + 1,
					pt->cli_Disassociations);
				printf("sta_%d: cli_AuthenticationFailures\t:%d\n", i + 1,
					pt->cli_AuthenticationFailures);
				printf("sta_%d: cli_Associations\t\t\t:%llu\n", i + 1,
					pt->cli_Associations);
			}
		}
		if (associated_dev_array) {
			free(associated_dev_array);
		}
	} else if (!strcmp(argv[1], "wifi_getApAssociatedDeviceDiagnosticResult3")) {
		wifi_associated_dev3_t *associated_dev_array = NULL,*pt = NULL;
		UINT array_size = 0;
		UINT i = 0;
		ret = wifi_getApAssociatedDeviceDiagnosticResult3(index, &associated_dev_array,
			&array_size);
		if (ret == RETURN_OK) {
			printf("Total_STA:%d\n", array_size);
			for (i = 0, pt = associated_dev_array; i < array_size; i++, pt++) {
				printf("\nsta_%d: cli_MACAddress\t\t\t:%02X:%02X:%02X:%02X:%02X:%02X\n",
					i + 1, pt->cli_MACAddress[0], pt->cli_MACAddress[1],
					pt->cli_MACAddress[2], pt->cli_MACAddress[3],
					pt->cli_MACAddress[4], pt->cli_MACAddress[5]);
				printf("sta_%d: cli_AuthenticationState\t\t:%d\n", i + 1,
					pt->cli_AuthenticationState);
				printf("sta_%d: cli_LastDataDownlinkRate\t\t:%d\n", i + 1,
					pt->cli_LastDataDownlinkRate);
				printf("sta_%d: cli_LastDataUplinkRate\t\t:%d\n", i + 1,
					pt->cli_LastDataUplinkRate);
				printf("sta_%d: cli_SignalStrength\t\t:%d\n", i + 1,
					pt->cli_SignalStrength);
				printf("sta_%d: cli_Retransmissions\t\t:%d\n", i + 1,
					pt->cli_Retransmissions);
				printf("sta_%d: cli_OperatingStandard\t\t:%s\n", i + 1,
					pt->cli_OperatingStandard);
				printf("sta_%d: cli_OperatingChannelBandwidth\t:%s\n", i + 1,
					pt->cli_OperatingChannelBandwidth);
				printf("sta_%d: cli_SNR\t\t\t\t:%d\n", i + 1,
					pt->cli_SNR);
				printf("sta_%d: cli_DataFramesSentAck\t\t:%lu\n", i + 1,
					pt->cli_DataFramesSentAck);
				printf("sta_%d: cli_DataFramesSentNoAck\t\t:%lu\n", i + 1,
					pt->cli_DataFramesSentNoAck);
				printf("sta_%d: cli_BytesSent\t\t\t:%lu\n", i + 1,
					pt->cli_BytesSent);
				printf("sta_%d: cli_BytesReceived\t\t:%lu\n", i + 1,
					pt->cli_BytesReceived);
				printf("sta_%d: cli_RSSI\t\t\t\t:%d\n", i + 1,
					pt->cli_RSSI);
				printf("sta_%d: cli_Disassociations\t\t:%d\n", i + 1,
					pt->cli_Disassociations);
				printf("sta_%d: cli_AuthenticationFailures\t:%d\n", i + 1,
					pt->cli_AuthenticationFailures);
				printf("sta_%d: cli_Associations\t\t\t:%llu\n", i + 1,
					pt->cli_Associations);
				printf("sta_%d: cli_PacketsSent\t\t\t:%lu\n", i + 1,
					pt->cli_PacketsSent);
				printf("sta_%d: cli_PacketsReceived\t\t:%lu\n", i + 1,
					pt->cli_PacketsReceived);
				printf("sta_%d: cli_ErrorsSent\t\t\t:%lu\n", i + 1,
					pt->cli_ErrorsSent);
				printf("sta_%d: cli_RetransCount\t\t\t:%lu\n", i + 1,
					pt->cli_RetransCount);
				printf("sta_%d: cli_FailedRetransCount\t\t:%lu\n", i + 1,
					pt->cli_FailedRetransCount);
				printf("sta_%d: cli_RetryCount\t\t\t:%lu\n", i + 1,
					pt->cli_RetryCount);
				printf("sta_%d: cli_MultipleRetryCount\t\t:%lu\n", i + 1,
					pt->cli_MultipleRetryCount);
				printf("sta_%d: cli_MaxDownlinkRate\t\t:%d\n", i + 1,
					pt->cli_MaxDownlinkRate);
				printf("sta_%d: cli_MaxUplinkRate\t\t:%d\n", i + 1,
					pt->cli_MaxUplinkRate);
#ifdef WIFI_HAL_VERSION_GE_3_0_2
				printf("sta_%d: cli_TxFrames\t\t\t:%llu\n", i + 1,
					pt->cli_TxFrames);
				printf("sta_%d: cli_RxRetries\t\t\t:%llu\n", i + 1,
					pt->cli_RxRetries);
				printf("sta_%d: cli_RxErrors\t\t\t:%llu\n", i + 1,
					pt->cli_RxErrors);
#endif /* WIFI_HAL_VERSION_GE_3_0_2 */
#ifdef WIFI_HAL_VERSION_GE_3_0_1
				printf("sta_%d: cli_activeNumSpatialStreams\t:%d\n", i + 1,
					pt->cli_activeNumSpatialStreams);
#endif /* WIFI_HAL_VERSION_GE_3_0_1 */
#ifdef WIFI_HAL_VERSION_3
				/* TBD cli_DownlinkMuStats cli_UplinkMuStats */
				printf("sta_%d: TWT Parameters: ", i + 1);
				print_TWTparams(pt->cli_TwtParams.twtParams,
					pt->cli_TwtParams.numTwtSession);
#endif /* WIFI_HAL_VERSION_3 */
			}
			if (associated_dev_array) {
				free(associated_dev_array);
			}
		}
	} else if (!strcmp(argv[1], "wifi_getRadioBandUtilization")) {
		INT output_percentage;
		ret = wifi_getRadioBandUtilization(index, &output_percentage);
		if (ret == RETURN_OK) {
			printf("%d\n", output_percentage);
		}
	} else if (!strcmp(argv[1], "wifi_getApAclDeviceNum")) {
		UINT output_devicenum;
		ret = wifi_getApAclDeviceNum(index, &output_devicenum);
		if (ret == RETURN_OK) {
			printf("%u\n", output_devicenum);
		}
#ifdef WIFI_HAL_VERSION_3_PHASE2
	} else if (!strcmp(argv[1], "wifi_getApAssociatedDevice")) {
		UINT num, i;
		mac_address_t mac_array[256];

		memset(mac_array, 0, sizeof(mac_array));
		ret = wifi_getApAssociatedDevice(index, mac_array,
			sizeof(mac_array)/sizeof(mac_array[0]), &num);
		if (ret == RETURN_OK) {
			for (i = 0; i < num; ++i) {
				printf(MACF, MAC_TO_MACF(mac_array[i]));
				if (i < num - 1) {
					printf(",");
				}
			}
			printf("\n");
		}
#else /* WIFI_HAL_VERSION_3_PHASE2 */
	} else if (!strcmp(argv[1], "wifi_getApAssociatedDevice")) {
		CHAR output_buf[MAX_OUTPUT_STRING_LEN_1024];
		INT output_buf_size = MAX_OUTPUT_STRING_LEN_1024;
		memset(output_buf, 0, MAX_OUTPUT_STRING_LEN_1024);
		ret = wifi_getApAssociatedDevice(index, output_buf, output_buf_size);
		if (ret == RETURN_OK) {
			printf("%s\n", output_buf);
		}
#endif /* WIFI_HAL_VERSION_3_PHASE2 */
#ifdef WIFI_HAL_VERSION_3_PHASE2
	} else if (!strcmp(argv[1], "wifi_getApAuthenticatedDevices")) {
		UINT num, i;
		mac_address_t mac_array[256];

		memset(mac_array, 0, sizeof(mac_array));
		ret = wifi_getApAuthenticatedDevices(index, mac_array,
			sizeof(mac_array)/sizeof(mac_array[0]), &num);
		if (ret == RETURN_OK) {
			for (i = 0; i < num; ++i) {
				printf(MACF, MAC_TO_MACF(mac_array[i]));
				if (i < num - 1) {
					printf(",");
				}
			}
			printf("\n");
		}
#endif /* WIFI_HAL_VERSION_3_PHASE2 */
	} else if (!strcmp(argv[1], "wifi_getApBeaconRate")) {
		CHAR beaconrate[32] = {0};
		ret = wifi_getApBeaconRate(index, beaconrate);
		if (ret == RETURN_OK) {
			printf("%s\n", beaconrate);
		}
	} else if (!(strcmp(argv[1], "wifi_getApTxBeaconFrameCount"))) {
		UINT count;
		ret = wifi_getApTxBeaconFrameCount(index, &count);
		if (ret == RETURN_OK) {
			printf("txbcnfrm count %d\n", count);
		}
	} else if (!strcmp(argv[1], "wifi_getATMCapable")) {
		BOOL capable;
		ret = wifi_getATMCapable(&capable);
		if (ret == RETURN_OK) {
			printf("%s\n", capable ? "TRUE" : "FALSE");
		}
	} else if (!strcmp(argv[1], "wifi_getRadioDcsDwelltime")) {
		INT output_millsecond;
		ret = wifi_getRadioDcsDwelltime(index, &output_millsecond);
		if (ret == RETURN_OK) {
			printf("%d\n", output_millsecond);
		}
	} else if (!strcmp(argv[1], "wifi_setRadioDcsDwelltime")) {
		ret = wifi_setRadioDcsDwelltime(index, atoi(argv[3]));
	} else if (!strcmp(argv[1], "wifi_getRadioGuardInterval")) {
		char guardInterval[64];
		ret = wifi_getRadioGuardInterval(index, guardInterval);
		if (ret == RETURN_OK) {
			printf("%s\n", guardInterval);
		}
	} else if (!strcmp(argv[1], "wifi_getApManagementFramePowerControl")) {
		INT output_power;
		ret = wifi_getApManagementFramePowerControl(index, &output_power);
		if (ret == RETURN_OK) {
			printf("%d\n", output_power);
		}
	} else if (!strcmp(argv[1], "wifi_getRadioDcsScanning")) {
		BOOL dcs;
		ret = wifi_getRadioDcsScanning(index, &dcs);
		if (ret == RETURN_OK) {
			printf("%d\n", dcs);
		}
	} else if (!strcmp(argv[1], "wifi_deleteAp")) {
		wifi_api_info_t apiInfo;
		strncpy(apiInfo.api_name, argv[1], 1024);
		apiInfo.radioIndex = index;
		ret = wifi_api_send_msg(&apiInfo);
	} else if (!strcmp(argv[1], "wifi_getSSIDNameStatus")) {
		char name[33];
		ret = wifi_getSSIDNameStatus(index, name);
		if (ret == RETURN_OK) {
			printf("%s\n", name);
		}
	} else if (!strcmp(argv[1], "wifi_getApMacAddressControlMode")) {
		int mac_mode;
		ret = wifi_getApMacAddressControlMode(index, &mac_mode);
		if (ret == RETURN_OK) {
			printf("%d\n", mac_mode);
		}
	} else if (!strcmp(argv[1], "wifi_getRadioCountryCode")) {
		char countryStr[64];
		ret = wifi_getRadioCountryCode(index, countryStr);
		if (ret == RETURN_OK) {
			printf("%s\n", countryStr);
		}
	} else if (!strcmp(argv[1], "wifi_getApBasicAuthenticationMode")) {
		char authMode[32];
		ret = wifi_getApBasicAuthenticationMode(index, authMode);
		if (ret == RETURN_OK) {
			printf("%s\n", authMode);
		}
	} else if (!strcmp(argv[1], "wifi_getApWpaEncryptionMode")) {
		char mode[32];
		ret = wifi_getApWpaEncryptionMode(index, mode);
		if (ret == RETURN_OK) {
			printf("%s\n", mode);
		}
	} else if (!strcmp(argv[1], "wifi_getApWpsDevicePIN")) {
		long unsigned int pin;
		ret = wifi_getApWpsDevicePIN(index, &pin);
		if (ret == RETURN_OK) {
			printf("%ld\n", pin);
		}
	} else if (!strcmp(argv[1], "wifi_getBandSteeringCapability")) {
		BOOL support;
		ret = wifi_getBandSteeringCapability(&support);
		if (ret == RETURN_OK) {
			printf("%d\n", support);
		}
	} else if (!strcmp(argv[1], "wifi_getRadio11nGreenfieldEnable")) {
		BOOL enable;
		ret = wifi_getRadio11nGreenfieldEnable(index, &enable);
		if (ret == RETURN_OK) {
			printf("%d\n", enable);
		}
	} else if (!strcmp(argv[1], "wifi_getRadio11nGreenfieldSupported")) {
		BOOL enable;
		ret = wifi_getRadio11nGreenfieldSupported(index, &enable);
		if (ret == RETURN_OK) {
			printf("%d\n", enable);
		}
	} else if (!strcmp(argv[1], "wifi_getApWpsEnable")) {
		BOOL enable;
		ret = wifi_getApWpsEnable(index, &enable);
		if (ret == RETURN_OK) {
			printf("%d\n", enable);
		}
	} else if (!strcmp(argv[1], "wifi_getApSecuritySecondaryRadiusServer")) {
		char secondaryRadiusServerIPAddr[45] = { 0 };
		unsigned int secondaryRadiusServerPort = 0;
		char secondaryRadiusSecret[64] = { 0 };

		ret = wifi_getApSecuritySecondaryRadiusServer(index, secondaryRadiusServerIPAddr,
			&secondaryRadiusServerPort, secondaryRadiusSecret);
		if (ret == RETURN_OK) {
			printf("Secondary Radius server IP Address:%s\n", secondaryRadiusServerIPAddr);
			printf("Secondary Radius server Port:%d\n", secondaryRadiusServerPort);
			printf("Secondary Radius server Secret Key:%s\n", secondaryRadiusSecret);
		}
	} else if (!strcmp(argv[1], "wifi_getApSecurityRadiusServer")) {
		char radiusServerIPAddr[45];
		unsigned int radiusServerPort;
		char radiusSecret[64];

		ret = wifi_getApSecurityRadiusServer(index, radiusServerIPAddr, &radiusServerPort,
			radiusSecret);
		if (ret == RETURN_OK) {
			printf("Radius server IP Address:%s\n", radiusServerIPAddr);
			printf("Radius server Port:%d\n", radiusServerPort);
			printf("Radius server Secret Key:%s\n", radiusSecret);
		}
	} else if (!strcmp(argv[1], "wifi_getApSecurityRadiusSettings")) {
		wifi_radius_setting_t radius = { 0 };
		ret = wifi_getApSecurityRadiusSettings(index, &radius);
		if (ret == RETURN_OK) {
			printf(" RadiusServerRetries			:%d\n",
				radius.RadiusServerRetries);
			printf(" RadiusServerRequestTimeout		:%d\n",
				radius.RadiusServerRequestTimeout);
			printf(" PMKLifetime				:%d\n",
				radius.PMKLifetime);
			printf(" PMKCaching				:%s\n",
				(radius.PMKCaching == TRUE) ? "Enable" : "Disable");
			printf(" PMKCacheInterva			:%d\n",
				radius.PMKCacheInterval);
			printf(" MaxAuthenticationAttempts		:%d\n",
				radius.MaxAuthenticationAttempts);
			printf(" BlacklistTableTimeout			:%d\n",
				radius.BlacklistTableTimeout);
			printf(" IdentityRequestRetryInterva		:%d\n",
				radius.IdentityRequestRetryInterval);
			printf(" QuietPeriodAfterFailedAuthentication	:%d\n",
				radius.QuietPeriodAfterFailedAuthentication);
		}
	} else if (!strcmp(argv[1], "wifi_setApSecurityRadiusSettings")) {
		wifi_api_info_t apiInfo;

		strncpy(apiInfo.api_name, argv[1], 1024);
		snprintf(apiInfo.api_data, 1024, "%s %s %s %s %s %s %s %s %s",
			argv[3], argv[4], argv[5], argv[6], argv[7], argv[8], argv[9], argv[10],
			argv[11]);
		apiInfo.radioIndex = index;
		ret = wifi_api_send_msg(&apiInfo);
	} else if (!strcmp(argv[1], "wifi_getApBridgeInfo")) {
		char apBridgeName[32 + 1];
		char apBridgeIP[16];
		char apBridgeSubnet[16];

		ret = wifi_getApBridgeInfo(index, apBridgeName, apBridgeIP, apBridgeSubnet);
		printf("AP bridge name:%s\n", apBridgeName);
		printf("AP bridge IP:%s\n", apBridgeIP);
		printf("AP bridge subnet:%s\n", apBridgeSubnet);
	} else if (!strcmp(argv[1], "wifi_getRadioDCSChannelPool")) {
		char pool[256];
		ret = wifi_getRadioDCSChannelPool(index, pool);
		if (ret == RETURN_OK) {
			printf("%s\n", pool);
		}
	} else if (!strcmp(argv[1], "wifi_setRadioDCSChannelPool")) {
		ret = wifi_setRadioDCSChannelPool(index, argv[3]);
	} else if (!strcmp(argv[1], "wifi_getRadioBasicDataTransmitRates")) {
		char transmitRates[64];
		ret = wifi_getRadioBasicDataTransmitRates(index, transmitRates);
		if (ret == RETURN_OK) {
			printf("%s\n", transmitRates);
		}
	} else if (!strcmp(argv[1], "wifi_getApSecurityModesSupported")) {
		char securityModes[256];
		ret = wifi_getApSecurityModesSupported(index, securityModes);
		if (ret == RETURN_OK) {
			printf("%s\n", securityModes);
		}
	} else if (!strcmp(argv[1], "wifi_getApSecurityModeEnabled")) {
		char securityModeEnabled[64];
		ret = wifi_getApSecurityModeEnabled(index, securityModeEnabled);
		if (ret == RETURN_OK) {
			printf("%s\n", securityModeEnabled);
		}
	} else if (!strcmp(argv[1], "wifi_getApSecurityKeyPassphrase")) {
		char securityKeyPassphrase[64];
		ret = wifi_getApSecurityKeyPassphrase(index, securityKeyPassphrase);
		if (ret == RETURN_OK) {
			printf("%s\n", securityKeyPassphrase);
		}
	} else if (!strcmp(argv[1], "wifi_getRadioOperatingChannelBandwidth")) {
		char bandwidth[64];
		ret = wifi_getRadioOperatingChannelBandwidth(index, bandwidth);
		if (ret == RETURN_OK) {
			printf("%s\n", bandwidth);
		}
	} else if (!strcmp(argv[1], "wifi_getApWpsConfigMethodsEnabled")) {
		char configMethods[256];
		ret = wifi_getApWpsConfigMethodsEnabled(index, configMethods);
		if (ret == RETURN_OK) {
			printf("%s\n", configMethods);
		}
	} else if (!strcmp(argv[1], "wifi_getRadioChannel")) {
		unsigned long channel;
		ret = wifi_getRadioChannel(index, &channel);
		if (ret == RETURN_OK) {
			printf("%ld\n", channel);
		}
	} else if (!strcmp(argv[1], "wifi_getRadioAutoChannelEnable")) {
		BOOL enable;
		ret = wifi_getRadioAutoChannelEnable(index, &enable);
		if (ret == RETURN_OK) {
			printf("%d\n", enable);
		}
	} else if (!strcmp(argv[1], "wifi_getRadioAutoChannelSupported")) {
		BOOL enable;
		ret = wifi_getRadioAutoChannelSupported(index, &enable);
		if (ret == RETURN_OK) {
			printf("%d\n", enable);
		}
	} else if (!strcmp(argv[1], "wifi_getRadioPossibleChannels")) {
		char channels[512];
		ret = wifi_getRadioPossibleChannels(index, channels);
		if (ret == RETURN_OK) {
			printf("%s\n", channels);
		}
	} else if (!strcmp(argv[1], "wifi_getRadioTransmitPower")) {
		unsigned long power;
		ret = wifi_getRadioTransmitPower(index, &power);
		if (ret == RETURN_OK) {
			printf("%ld\n", power);
		}
	} else if (!strcmp(argv[1], "wifi_getRadioAMSDUEnable")) {
		BOOL enable;
		ret = wifi_getRadioAMSDUEnable(index, &enable);
		if (ret == RETURN_OK) {
			printf("%d\n", enable);
		}
	} else if (!strcmp(argv[1], "wifi_getRadioTxChainMask")) {
		int txchain_mask;
		ret = wifi_getRadioTxChainMask(index, &txchain_mask);
		if (ret == RETURN_OK) {
			printf("%d\n", txchain_mask);
		}
	} else if (!strcmp(argv[1], "wifi_getRadioRxChainMask")) {
		int rxchain_mask;
		ret = wifi_getRadioRxChainMask(index, &rxchain_mask);
		if (ret == RETURN_OK) {
			printf("%d\n", rxchain_mask);
		}
	} else if (!strcmp(argv[1], "wifi_getApName")) {
		char name[64];
		ret = wifi_getApName(index, name);
		if (ret == RETURN_OK) {
			printf("%s\n", name);
		}
	} else if (!strcmp(argv[1], "wifi_getRadioChannelsInUse")) {
#ifdef WIFI_HAL_VERSION_3_PHASE2
		wifi_channels_list_t chList;
		ret = wifi_getRadioChannelsInUse(index, &chList);
		if (ret == RETURN_OK) {
			print_chList(&chList);
			printf("\n");
                }
#else
		char channels_in_use[256];
		ret = wifi_getRadioChannelsInUse(index, channels_in_use);
		if (ret == RETURN_OK) {
			printf("%s\n", channels_in_use);
		}
#endif /* WIFI_HAL_VERSION_3 */
	} else if (!strcmp(argv[1], "wifi_getApSsidAdvertisementEnable")) {
		BOOL enable;
		ret = wifi_getApSsidAdvertisementEnable(index, &enable);
		if (ret == RETURN_OK) {
			printf("%d\n", enable);
		}
	} else if (!strcmp(argv[1], "wifi_getApBeaconType")) {
		char securityType[32];
		ret = wifi_getApBeaconType(index, securityType);
		if (ret == RETURN_OK) {
			printf("%s\n", securityType);
		}
	} else if (!strcmp(argv[1], "wifi_getBasicTrafficStats")) {
		wifi_basicTrafficStats_t basicStats;
		ret = wifi_getBasicTrafficStats(index, &basicStats);
		if (ret == RETURN_OK) {
			printf("Bytes Sent:		%ld\n", basicStats.wifi_BytesSent);
			printf("Bytes Received:		%ld\n", basicStats.wifi_BytesReceived);
			printf("Packets Sent:		%ld\n", basicStats.wifi_PacketsSent);
			printf("Packets Received:	%ld\n", basicStats.wifi_PacketsReceived);
			printf("Associations:		%ld\n", basicStats.wifi_Associations);
		}
	} else if (!strcmp(argv[1], "wifi_getWifiTrafficStats")) {
		wifi_trafficStats_t wifiStats;
		ret = wifi_getWifiTrafficStats(index, &wifiStats);
		if (ret == RETURN_OK) {
			printf("Errors Sent			:%ld\n",
				wifiStats.wifi_ErrorsSent);
			printf("Errors Received			:%ld\n",
				wifiStats.wifi_ErrorsReceived);
			printf("Unicast Packets Sent		:%ld\n",
				wifiStats.wifi_UnicastPacketsSent);
			printf("Unicast Packets Received	:%ld\n",
				wifiStats.wifi_UnicastPacketsReceived);
			printf("Discarded Packets Sent		:%ld\n",
				wifiStats.wifi_DiscardedPacketsSent);
			printf("Discarded Packets Received	:%ld\n",
				wifiStats.wifi_DiscardedPacketsReceived);
			printf("Multicast Packets Sent		:%ld\n",
				wifiStats.wifi_MulticastPacketsSent);
			printf("Multicast Packets Received	:%ld\n",
				wifiStats.wifi_MulticastPacketsReceived);
			printf("Broadcast Packets Sent		:%ld\n",
				wifiStats.wifi_BroadcastPacketsSent);
			printf("Broadcast Packets Recevied	:%ld\n",
				wifiStats.wifi_BroadcastPacketsRecevied);
			printf("Unknown Packets Received	:%ld\n",
				wifiStats.wifi_UnknownPacketsReceived);
		}
	} else if (!strcmp(argv[1], "wifi_getApNumDevicesAssociated")) {
		unsigned long num;
		ret = wifi_getApNumDevicesAssociated(index, &num);
		if (ret == RETURN_OK) {
			printf("%ld\n", num);
		}
	} else if (!strcmp(argv[1], "wifi_getAllAssociatedDeviceDetail")) {
		unsigned long array_size;
		wifi_device_t *associated_dev_array = NULL,*pt = NULL;
		unsigned long i;
		ret = wifi_getAllAssociatedDeviceDetail(index, &array_size, &associated_dev_array);
		if (ret == RETURN_OK) {
			printf("Total_STA:%ld\n", array_size);
			for (i = 0, pt = associated_dev_array; i < array_size; i++, pt++) {
				printf("sta_%ld: wifi_devMacAddress			:"MACF"\n",
					i + 1, MAC_TO_MACF(pt->wifi_devMacAddress));
				printf("sta_%ld: wifi_devIPAddress			:%s\n",
					i + 1, pt->wifi_devIPAddress);
				printf("sta_%ld: wifi_devAssociatedDeviceAuthentiationState:%d\n",
					i + 1, pt->wifi_devAssociatedDeviceAuthentiationState);
				printf("sta_%ld: wifi_devSignalStrength			:%d\n",
					i + 1, pt->wifi_devSignalStrength);
				printf("sta_%ld: wifi_devTxRate				:%d\n",
					i + 1, pt->wifi_devTxRate);
				printf("sta_%ld: wifi_devRxRate				:%d\n",
					i + 1, pt->wifi_devRxRate);
			}
		}
		if (associated_dev_array) {
			free(associated_dev_array); //make sure to free the list
		}
	} else if (!strcmp(argv[1], "wifi_getApAclDevices")) {
#ifdef WIFI_HAL_VERSION_3_PHASE2
		mac_address_t acl_list[100];
		unsigned int acl_num = 0, i = 0;
		memset(acl_list, '\0', sizeof(acl_list));
		ret = wifi_getApAclDevices(index, acl_list, sizeof(acl_list) / sizeof(acl_list[0]),
			&acl_num);
		if (ret == RETURN_OK) {
			for (i = 0; i < acl_num; ++i) {
				printf(MACF"\n", MAC_TO_MACF(acl_list[i]));
			}
		}
#else /* WIFI_HAL_VERSION_3_PHASE2 */
		char apAclDevList[MAC_STR_LEN * MAXMACLIST];
		char *acl = NULL, *rest;
		unsigned int apAclDeviceNum = 0, i = 0;
		ret = wifi_getApAclDeviceNum(index, &apAclDeviceNum);
		if (ret == RETURN_OK) {
			if (apAclDeviceNum > 0) {
				memset(apAclDevList, '\0', sizeof(apAclDevList));
				wifi_getApAclDevices(index, apAclDevList, sizeof(apAclDevList));

				acl = strtok_r(apAclDevList, " ", &rest);
				while ((acl != NULL) && (i < apAclDeviceNum)) {
					if (strlen(acl) >= 17) {
						printf("%s\n", acl);
					}
					acl = strtok_r(NULL, " ", &rest);
					++i;
				}
			}
		}
#endif /* WIFI_HAL_VERSION_3_PHASE2 */
	} else if (!strcmp(argv[1], "wifi_getApSecurityPreSharedKey")) {
		char presharedkey[32];
		ret = wifi_getApSecurityPreSharedKey(index, presharedkey);
		if (ret == RETURN_OK) {
			printf("%s\n", presharedkey);
		}
	} else if (!strcmp(argv[1], "wifi_getApWpsConfigurationState")) {
		char configstate[32];
		ret = wifi_getApWpsConfigurationState(index, configstate);
		if (ret == RETURN_OK) {
			printf("%s\n", configstate);
		}
	} else if (!strcmp(argv[1], "wifi_getRadioSupportedDataTransmitRates")) {
		char transmit_rates[256];
		ret = wifi_getRadioSupportedDataTransmitRates(index, transmit_rates);
		if (ret == RETURN_OK) {
			printf("%s\n", transmit_rates);
		}
	} else if (!strcmp(argv[1], "wifi_getRadioOperationalDataTransmitRates")) {
		char transmit_rates[256];
		ret = wifi_getRadioOperationalDataTransmitRates(index, transmit_rates);
		if (ret == RETURN_OK) {
			printf("%s\n", transmit_rates);
		}
	} else if (!strcmp(argv[1], "wifi_getRadioExtChannel")) {
		char ext[64];
		ret = wifi_getRadioExtChannel(index, ext);
		if (ret == RETURN_OK) {
			printf("%s\n", ext);
		}
	} else if (!strcmp(argv[1], "wifi_getRadioTransmitPowerSupported")) {
		char transmitPowerSupported[64];
		ret = wifi_getRadioTransmitPowerSupported(index, transmitPowerSupported);
		if (ret == RETURN_OK) {
			printf("%s\n", transmitPowerSupported);
		}
	}
	else if (!strcmp(argv[1], "wifi_getApWpsConfigMethodsSupported")) {
		char methods[256];
		ret = wifi_getApWpsConfigMethodsSupported(index, methods);
		if (ret == RETURN_OK) {
			printf("%s\n", methods);
		}
	} else if (!strcmp(argv[1], "wifi_getBandSteeringBandUtilizationThreshold")) {
		int threshold;
		ret = wifi_getBandSteeringBandUtilizationThreshold(index, &threshold);
		if (ret == RETURN_OK) {
			printf("%d\n", threshold);
		}
	} else if (!strcmp(argv[1], "wifi_getApRadioIndex")) {
		int radio_index;
		ret = wifi_getApRadioIndex(index, &radio_index);
		if (ret == RETURN_OK) {
			printf("%d\n", radio_index);
		}
	} else if (!strcmp(argv[1], "wifi_getRadioDCSScanTime")) {
		int sec, msec;
		ret = wifi_getRadioDCSScanTime(index, &sec, &msec);
		if (ret == RETURN_OK) {
			printf("%d %d\n", sec, msec);
		}
	} else if (!strcmp(argv[1], "wifi_getBandSteeringRSSIThreshold")) {
		int rssi;
		ret = wifi_getBandSteeringRSSIThreshold(index, &rssi);
		if (ret == RETURN_OK) {
			printf("%d\n", rssi);
		}
	} else if (!strcmp(argv[1], "wifi_getBandSteeringPhyRateThreshold")) {
		int prThreshold;
		ret = wifi_getBandSteeringPhyRateThreshold(index, &prThreshold);
		if (ret == RETURN_OK) {
			printf("%d\n", prThreshold);
		}
	} else if (!strcmp(argv[1], "wifi_getRadioChannelStats2")) {
		wifi_channelStats2_t outputChannelStats2;
		ret = wifi_getRadioChannelStats2(index, &outputChannelStats2);
		if (ret == RETURN_OK) {
			printf("Center Frequency	:%d\n", outputChannelStats2.ch_Frequency);
			printf("Non 802.11 Noise	:%d\n",
				outputChannelStats2.ch_Non80211Noise);
			printf("Max 802.11 Rssi		:%d\n",
				outputChannelStats2.ch_Max80211Rssi);
			printf("Other bss utilization	:%d\n", outputChannelStats2.ch_ObssUtil);
			printf("Self bss utilization	:%d\n",
				outputChannelStats2.ch_SelfBssUtil);
		}
	} else if (!strcmp(argv[1], "wifi_getRadioTrafficStats2")) {
		wifi_radioTrafficStats2_t traffic_stats;
		ret = wifi_getRadioTrafficStats2(index, &traffic_stats);
		if (ret == RETURN_OK) {
			printf("Bytes Sent			:%lu\n",
				traffic_stats.radio_BytesSent);
			printf("Bytes Recieved			:%lu\n",
				traffic_stats.radio_BytesReceived);
			printf("Packets Sent			:%lu\n",
				traffic_stats.radio_PacketsSent);
			printf("Packets Received		:%lu\n",
				traffic_stats.radio_PacketsReceived);
			printf("Errors Sent			:%lu\n",
				traffic_stats.radio_ErrorsSent);
			printf("Errors Received			:%lu\n",
				traffic_stats.radio_ErrorsReceived);
			printf("Discard Packets Sent		:%lu\n",
				traffic_stats.radio_DiscardPacketsSent);
			printf("Discard Packets Received	:%lu\n",
				traffic_stats.radio_DiscardPacketsReceived);
			printf("PLCP Error Count		:%lu\n",
				traffic_stats.radio_PLCPErrorCount);
			printf("FCS Error Count			:%lu\n",
				traffic_stats.radio_FCSErrorCount);
			printf("Invalid MAC Count		:%lu\n",
				traffic_stats.radio_InvalidMACCount);
			printf("Packets Other Received		:%lu\n",
				traffic_stats.radio_PacketsOtherReceived);
			printf("Noise Floor			:%d\n",
				traffic_stats.radio_NoiseFloor);
			printf("Channel Utilization		:%lu\n",
				traffic_stats.radio_ChannelUtilization);
			printf("Activity Factor			:%d\n",
				traffic_stats.radio_ActivityFactor);
			printf("CarrierSenseThreshold Exceeded	:%d\n",
				traffic_stats.radio_CarrierSenseThreshold_Exceeded);
			printf("Retransmission Metric		:%d\n",
				traffic_stats.radio_RetransmissionMetirc);
			printf("Maximum Noise Floor On Channel	:%d\n",
				traffic_stats.radio_MaximumNoiseFloorOnChannel);
			printf("Minimum Noise Floor On Channel	:%d\n",
				traffic_stats.radio_MinimumNoiseFloorOnChannel);
			printf("Median Noise Floor On Channel	:%d\n",
				traffic_stats.radio_MedianNoiseFloorOnChannel);
			printf("Statistics Start Time		:%lu\n",
				traffic_stats.radio_StatisticsStartTime);
		}
	} else if (!strcmp(argv[1], "wifi_getApSecurityMFPConfig")) {
		char mfp_config[32];
		ret = wifi_getApSecurityMFPConfig(index, mfp_config);
		if (ret == RETURN_OK) {
			printf("%s\n", mfp_config);
		}
    /* 802.11r FBT HAL test starts */
#ifdef MAX_KEY_HOLDERS
	/* wifi_hal.h's 802.11r Fast Trasition definitions. macro MAX_KEY_HOLDERS as a condition
	to avoid RDKM compiling errors */
	} else if (!strcmp(argv[1], "wifi_testApFBTFeature")) {
		BOOL activate, support;
		UINT key_lifetime;
		// mac_address_t key_id;
		UCHAR key_id[64];
		UCHAR fbt_cfg[64];
		wifi_FastTransitionConfig_t ftCfg;

		/* test FBT get/set configurations */

		/* wifi_[get|set]BSSTransitionActivated() */
		ret = wifi_getBSSTransitionActivated(index, &activate);
		if (ret == RETURN_ERR) {
			printf("wifi_getBSSTransitionActivated returned error\n");
			return ret;
		}

		printf("wifi_testApFBTConfig tests: wifi_getBSSTransitionActivated(apIndex %d, active %d)\n",
			index, activate);

		/* change FBT config */
		if (activate) activate = 0;
		else activate = 1;

		ret = wifi_setFastBSSTransitionActivated(index, activate);
		if (ret == RETURN_ERR) {
			printf("wifi_setFastBSSTransitionActivated returned error\n");
			return ret;
		}

		printf("wifi_testApFBTConfig tests:  wifi_setBSSTransitionActivated(apIndex %d, activate %d)\n",
			index, activate);

		/* wifi_[get|set]FTOverDSActivated() */
		ret = wifi_getFTOverDSActivated(index, &activate);
		if (ret == RETURN_ERR) {
			printf("wifi_getFTOverDSActivated returned error\n");
			return ret;
		}

		printf("wifi_testApFBTConfig tests: wifi_getFTOverDSActivated(apIndex %d, activate %d)\n",
			index, activate);

		/* wifi_[get|set]FTMobilityDomainID() */
		ret = wifi_getFTMobilityDomainID(index, fbt_cfg);
		if (ret == RETURN_ERR) {
			printf("wifi_getFTMobilityDomainID returned error\n");
			return ret;
		}

		printf("wifi_testApFBTConfig tests:  wifi_getFTMobilityDomainID(apIndex %d, 0x%x 0x%x)\n",
			index, fbt_cfg[0], fbt_cfg[1]);

		ret = wifi_setFTMobilityDomainID(index, fbt_cfg);
		if (ret == RETURN_ERR) {
			printf("wifi_setFTMobilityDomainID returned error\n");
			return ret;
		}

		printf("wifi_testApFBTConfig tests: wifi_setFTMobilityDomainID(apIndex %d, 0x%x 0x%x)\n",
			index, fbt_cfg[0], fbt_cfg[1]);

		/* wifi_[get|set]FTResourceRequestSupported() */
		ret = wifi_getFTResourceRequestSupported(index, &support);
		if (ret == RETURN_ERR) {
			printf("wifi_getFTResourceRequestSupported returned error\n");
			return ret;
		}

		printf("wifi_testApFBTConfig tests:  wifi_getFTResourceRequestSupported (apIndex %d, supported %d)\n",
			index, support);

		support = TRUE;
		ret = wifi_setFTResourceRequestSupported(index, &support);
		if (ret == RETURN_ERR) {
			printf("wifi_setFTResourceRequestSupported returned error\n");
			return ret;
		}

		printf("wifi_testApFBTConfig tests:  wifi_setFTResourceRequestSupported (apIndex %d, supported %d)\n",
			index, support);

		/* wifi_[get|set]FTR0KeyLifetime() */
		ret = wifi_getFTR0KeyLifetime(index, &key_lifetime);
		if (ret == RETURN_ERR) {
			printf("wifi_getFTR0KeyLifetime returned error\n");
			return ret;
		}

		printf("wifi_testApFBTConfig tests:  wifi_getFTR0KeyLifetime (apIndex %d, lifetime %d)\n",
			index, key_lifetime);

		key_lifetime = 9999999;
		ret = wifi_setFTR0KeyLifetime(index, &key_lifetime);
		if (ret == RETURN_ERR) {
			printf("wifi_setFTR0KeyLifetime returned error\n");
			return ret;
		}

		printf("wifi_testApFBTConfig tests:  wifi_getFTR0KeyLifetime (apIndex %d, lifetime %d)\n",
			index, key_lifetime);

		/* wifi_[get|set]FTR0KeyHolderID() */
		ret = wifi_getFTR0KeyHolderID(index, &key_id[0]);
		if (ret == RETURN_ERR) {
			printf("wifi_getFTR0KeyHolderID returned error\n");
			return ret;
		}

		printf("wifi_testApFBTConfig tests:  wifi_getFTR0KeyHolderID (apIndex %d, key_id[0] 0x%x)\n",
			index, key_id[0]);

		key_id[0] = 0x88;
		ret = wifi_setFTR0KeyHolderID(index, &key_id[0]);
		if (ret == RETURN_ERR) {
			printf("wifi_setFTR0KeyHolderID returned error\n");
			return ret;
		}

		printf("wifi_testApFBTConfig tests:  wifi_setFTR0KeyHolderId (apIndex %d, key_id[0] 0x%x)\n",
			index, key_id[0]);

		/* wifi_[get|set]FTR1KeyHolderID() */
		ret = wifi_getFTR1KeyHolderID(index, &key_id[0]);
		if (ret == RETURN_ERR) {
			printf("wifi_getFTR1KeyHolderID returned error\n");
			return ret;
		}

		printf("wifi_testApFBTConfig tests:  wifi_getFTR1KeyHolderID (apIndex %d, key_id[0] 0x%x)\n",
			index, key_id[0]);

		key_id[0] = 0x99;
		ret = wifi_setFTR1KeyHolderID(index, &key_id[0]);
		if (ret == RETURN_ERR) {
			printf("wifi_setFTR1KeyHolderID returned error\n");
			return ret;
		}

		printf("wifi_testApFBTConfig tests:  wifi_setFTR1KeyHolderId (apIndex %d, key_id[0] 0x%x)\n",
			index, key_id[0]);

		/* wifi_pushApFastTransitionConfig() */
		ftCfg.support = FT_SUPPORT_FULL;
		ftCfg.mobilityDomain = 0x0;
		ftCfg.overDS = TRUE;
		ftCfg.r0KeyLifeTime = 0xFF;

		/* params .. */

		ret = wifi_pushApFastTransitionConfig(index, &ftCfg);
		if (ret == RETURN_ERR) {
			printf("wifi_pushApFastTransitionConfig returned error\n");
			return ret;
		}

		printf("wifi_testApFBTConfig tests:  wifi_pushApFastTransitionConfig(apIndex %d, 0x%p)\n",
			index, &ftCfg);
	} else if (!strcmp(argv[1], "wifi_setFastBSSTransitionActivated")) {
		BOOL activate = atoi(argv[3]);
		ret = wifi_setFastBSSTransitionActivated(index, activate);
		if (ret == RETURN_OK) {
			printf("wifi_setBSSTransitionActivated(apIndex %d, ..)\n", index);
			printf("wifi_setFastBSSTransitionActivated(apIndex %d, activate %d)\n",
				index, activate);
		}
	} else if (!strcmp(argv[1], "wifi_getBSSTransitionActivated")) {
		BOOL activate;
		ret = wifi_getBSSTransitionActivated(index, &activate);
		if (ret == RETURN_OK) {
			printf("wifi_getBSSTransitionActivated(apIndex %d, ..)\n", index);
			printf("wifi_getBSSTransitionActivated(apIndex %d, activate %d)\n", index,
			activate);
		}
	} else if (!strcmp(argv[1], "wifi_getFTOverDSActivated")) {
		BOOL activate;
		ret = wifi_getFTOverDSActivated(index, &activate);
		if (ret == RETURN_OK) {
			printf("wifi_getFTOverDSActivated(apIndex %d, ..)\n", index);
			printf("wifi_getFTOverDSActivated(apIndex %d, activate %d)\n", index,
				activate);
		}
	} else if (!strcmp(argv[1], "wifi_setFTOverDSActivated")) {
		BOOL activate = atoi(argv[3]);
		ret = wifi_setFTOverDSActivated(index, &activate);
		if (ret == RETURN_OK) {
			printf("wifi_setFTOverDSActivated(apIndex %d, ..)\n", index);
			printf("wifi_setFTOverDSActivated(apIndex %d, &activate %d)\n", index,
				activate);
		}
	} else if (!strcmp(argv[1], "wifi_getFTMobilityDomainID")) {
		UCHAR fbt_cfg[64];
		ret = wifi_getFTMobilityDomainID(index, fbt_cfg);
		if (ret == RETURN_OK) {
			printf("wifi_getFTMobilityDomainID(apIndex %d, ..)\n", index);
			printf("wifi_getFTMobilityDomainID(apIndex %d, 0x%x 0x%x 0x%x ,,)\n",
				index, fbt_cfg[0], fbt_cfg[1], fbt_cfg[2]);
		}
	} else if (!strcmp(argv[1], "wifi_setFTMobilityDomainID")) {
		UCHAR fbt_cfg[2];
		unsigned short *ptr = (unsigned short *)fbt_cfg;
		*ptr = (unsigned short)strtoul(argv[3], NULL, 0);
		ret = wifi_setFTMobilityDomainID(index, fbt_cfg);
		if (ret == RETURN_OK) {
			printf("wifi_setFTMobilityDomainID: apIndex %d ID %s (0x%x 0x%x 0x%x)\n",
				index, argv[3], fbt_cfg[0], fbt_cfg[1], *ptr);
		}
	} else if (!strcmp(argv[1], "wifi_getFTResourceRequestSupported")) {
		BOOL support;
		ret = wifi_getFTResourceRequestSupported(index, &support);
		if (ret == RETURN_OK) {
			printf("wifi_getFTResourceRequestSupported(apIndex %d, ..)\n", index);
			printf("wifi_getFTResourceRequestSupported (apIndex %d, supported %d)\n",
				index, support);
		}
	} else if (!strcmp(argv[1], "wifi_setFTResourceRequestSupported")) {
		BOOL support = atoi(argv[3]);
		ret = wifi_setFTResourceRequestSupported(index, &support);
		if (ret == RETURN_OK) {
			printf("wifi_setFTResourceRequestSupported(apIndex %d, ..)\n", index);
			printf("wifi_setFTResourceRequestSupported (apIndex %d, supported %d)\n",
				index, support);
		}
	} else if (!strcmp(argv[1], "wifi_getFTR0KeyLifetime")) {
		UINT key_lifetime;
		ret = wifi_getFTR0KeyLifetime(index, &key_lifetime);
		if (ret == RETURN_OK) {
			printf("wifi_getFTR0KeyLifetime(apIndex %d, ..)\n", index);
			printf("wifi_getFTR0KeyLifetime (apIndex %d, lifetime %d)\n", index,
				key_lifetime);
		}
	} else if (!strcmp(argv[1], "wifi_setFTR0KeyLifetime")) {
		UINT key_lifetime = atoi(argv[3]);
		ret = wifi_setFTR0KeyLifetime(index, &key_lifetime);
		if (ret == RETURN_OK) {
			printf("wifi_setFTR0KeyLifetime(apIndex %d, ..)\n", index);
			printf("wifi_getFTR0KeyLifetime (apIndex %d, lifetime %d)\n", index,
				key_lifetime);
		}
	} else if (!strcmp(argv[1], "wifi_getFTR0KeyHolderID")) {
		UCHAR key_id[64];
		ret = wifi_getFTR1KeyHolderID(index, &key_id[0]);
		if (ret == RETURN_OK) {
			printf("wifi_getFTR0KeyHolderID(apIndex %d, ..)\n", index);
			printf("wifi_getFTR0KeyHolderID (apIndex %d, key_id[0] 0x%x)\n", index,
				key_id[0]);
		}
	} else if (!strcmp(argv[1], "wifi_setFTR0KeyHolderID")) {
		UCHAR key_id[64] = {0};

		if ( strlen(argv[3]) >= 64 ) {
			printf("Usage: wifi_setFTR0KeyHolderID keyID string too long \n");
			return RETURN_ERR;
		}

		strncpy((char *)&key_id[0], argv[3], strlen(argv[3]));
		key_id[strlen(argv[3])] = '\0'; /* need to include the null terminator */

		ret = wifi_setFTR0KeyHolderID(index, &key_id[0]);
		if (ret == RETURN_OK) {
			printf("wifi_setFTR0KeyHolderID(apIndex %d, ..)\n", index);
			printf("wifi_setFTR0KeyHolderID (apIndex %d, key_id[0] 0x%x)\n", index,
				key_id[0]);
		}
	} else if (!strcmp(argv[1], "wifi_getFTR1KeyHolderID")) {
		UCHAR key_id[64];
		ret = wifi_getFTR1KeyHolderID(index, &key_id[0]);
		if (ret == RETURN_OK) {
			printf("wifi_getFTR1KeyHolderID(apIndex %d, ..)\n", index);
			printf("wifi_getFTR0KeyHolderID (apIndex %d, key_id[0] 0x%x)\n", index,
				key_id[0]);
		}
	} else if (!strcmp(argv[1], "wifi_setFTR1KeyHolderID")) {
		UCHAR key_id[64] = {0};

		if ( strlen(argv[3]) >= 64 ) {
			printf("Usage: wifi_setFTR0KeyHolderID keyID string too long \n");
			return RETURN_ERR;
		}

		strncpy((char *)&key_id[0], argv[3], strlen(argv[3]));
		key_id[strlen(argv[3])] = '\0'; /* need to include the null terminator */

		ret = wifi_setFTR1KeyHolderID(index, &key_id[0]);
		if (ret == RETURN_OK) {
			printf("wifi_setFTR1KeyHolderID(apIndex %d, ..)\n", index);
			printf("wifi_setFTR0KeyHolderID (apIndex %d, key_id[0] 0x%x)\n", index,
				key_id[0]);
		}
	} else if (!strcmp(argv[1], "wifi_pushApFastTransitionConfig")) {
		wifi_FastTransitionConfig_t ftCfg;

		ftCfg.support = atoi(argv[3]);
		ftCfg.mobilityDomain = atoi(argv[4]);
		ftCfg.overDS = atoi(argv[5]);
		ftCfg.r0KeyLifeTime = atoi(argv[6]);

		/* wifi_pushApFastTransitionConfig() */
		ret = wifi_pushApFastTransitionConfig(index, &ftCfg);
		if (ret == RETURN_OK) {
			printf("wifi_pushApFastTransitionConfig(apIndex %d, ..)\n", index);
			printf("wifi_pushApFastTransitionConfig(apIndex %d, 0x%p)\n", index,
				&ftCfg);
		}
#endif /* MAX_KEY_HOLDERS */
	/* 802.11r FBT HAL test ends */
	} else if ((!strcmp(argv[1], "wifi_setBandSteeringEnable")) ||
		(!strcmp(argv[1], "wifi_removeApSecVaribles")) ||
		(!strcmp(argv[1], "wifi_disableApEncryption")) ||
		(!strcmp(argv[1], "wifi_setApWpsButtonPush")) ||
		(!strcmp(argv[1], "wifi_cancelApWPS")) ||
		(!strcmp(argv[1], "wifi_applyRadioSettings")) ||
		(!strcmp(argv[1], "wifi_delApAclDevices")) ||
		(!strcmp(argv[1], "wifi_setBandSteeringApGroup")) ||
		(!strcmp(argv[1], "wifi_applySSIDSettings"))) {
		wifi_api_info_t apiInfo;
		strncpy(apiInfo.api_name, argv[1], 1024);
		strncpy(apiInfo.api_data, argv[2], 1024);
		apiInfo.radioIndex = index;
		ret = wifi_api_send_msg(&apiInfo);
	} else if (!strcmp(argv[1], "wifi_apAssociatedDevice_callback_register")) {
		/* Needs to be revisited:
		* The below is just a dummy function wrapper to call
		* wifi_apAssociatedDevice_callback_register */
		wifi_apAssociatedDevice_callback callback =
			wifi_apAssociatedDevice_callback_test_func;
		wifi_apAssociatedDevice_callback_register(callback);
		printf("wifi_apAssociatedDevice_callback_register cbThreadId=%lu\n", cbThreadId);
		pthread_join(cbThreadId, NULL);
	} else if (!strcmp(argv[1], "wifi_newApAssociatedDevice_callback_register")) {
		/* Needs to be revisited:
		* The below is just a dummy function wrapper to call
		* wifi_newApAssociatedDevice_callback_register */
		wifi_newApAssociatedDevice_callback callback =
			wifi_newApAssociatedDevice_callback_test_func;
		wifi_newApAssociatedDevice_callback_register(callback);
		printf("wifi_newApAssociatedDevice_callback_register cbThreadId=%lu\n", cbThreadId);
		pthread_join(cbThreadId, NULL);
	} else if (!strcmp(argv[1], "wifi_apAuthEvent_callback_register")) {
		/* The below is just a dummy function wrapper to call
		* wifi_apDeAuthEvent_callback_register */
		wifi_apAuthEvent_callback callback = NULL;
		/* obsolete, this is dummy call */
		wifi_apAuthEvent_callback_register(callback);
	} else if (!strcmp(argv[1], "wifi_apDisassociatedDevice_callback_register")) {
		/* The below is just a dummy function wrapper to call
		* wifi_apDeAuthEvent_callback_register */
		wifi_apDisassociatedDevice_callback callback =
			wifi_apDisassociatedDevice_callback_test_func;
		wifi_apDisassociatedDevice_callback_register(callback);
		/* no thread for this register */
	} else if (!strcmp(argv[1], "wifi_apDeAuthEvent_callback_register")) {
		/* The below is just a dummy function wrapper to call
		* wifi_apDeAuthEvent_callback_register */
		wifi_apDeAuthEvent_callback callback = wifi_apDeAuthEvent_callback_test_func;
		wifi_apDeAuthEvent_callback_register(callback);
		pthread_join(cbThreadId, NULL);
	} else if (!strcmp(argv[1], "wifi_steering_event_callback_register")) {
		/* The below is just a dummy function wrapper to call
		* wifi_steering_eventRegister */
		wifi_steering_eventCB_t callback = wifi_steering_event_callback_test_func;
		ret = wifi_steering_eventRegister(callback);
		if (ret == RETURN_OK) {
			printf("wifi_steering_event_callback_register cbThreadId=%lu\n",
			cbThreadId);
			pthread_join(cbThreadId, NULL);
		}
#ifdef WIFI_HAL_VERSION_3_PHASE2
	} else if (!strcmp(argv[1], "wifi_steering_setGroup")) {
		UINT g_idx, i, my_argc;
		wifi_steering_apConfig_t cfg_array[HAL_RADIO_NUM_RADIOS];

		g_idx = atoi(argv[2]);
		my_argc = 3;

		for (i = 0; i < HAL_RADIO_NUM_RADIOS; i++) {
			if (((argc - my_argc) < 5) || argv[my_argc] == NULL)
				break;
			cfg_array[i].apIndex = atoi(argv[my_argc++]);
			cfg_array[i].utilCheckIntervalSec = atoi(argv[my_argc++]);
			cfg_array[i].utilAvgCount = atoi(argv[my_argc++]);
			cfg_array[i].inactCheckIntervalSec = atoi(argv[my_argc++]);
			cfg_array[i].inactCheckThresholdSec = atoi(argv[my_argc++]);
		}

		ret = wifi_steering_setGroup(g_idx, i, cfg_array);
#else /* WIFI_HAL_VERSION_3_PHASE2 */
	} else if (!strcmp(argv[1], "wifi_steering_setGroup")) {
		UINT g_idx;
		wifi_steering_apConfig_t cfg2g, cfg5g;

		g_idx = atoi(argv[2]);

		cfg2g.apIndex = atoi(argv[3]);
		cfg2g.utilCheckIntervalSec = atoi(argv[4]);
		cfg2g.utilAvgCount = atoi(argv[5]);
		cfg2g.inactCheckIntervalSec = atoi(argv[6]);
		cfg2g.inactCheckThresholdSec = atoi(argv[7]);

		cfg5g.apIndex = atoi(argv[8]);
		cfg5g.utilCheckIntervalSec = atoi(argv[9]);
		cfg5g.utilAvgCount = atoi(argv[10]);
		cfg5g.inactCheckIntervalSec = atoi(argv[11]);
		cfg5g.inactCheckThresholdSec = atoi(argv[12]);

		ret = wifi_steering_setGroup(g_idx, &cfg2g, &cfg5g);
#endif /* WIFI_HAL_VERSION_3_PHASE2 */
	} else if (!strcmp(argv[1], "wifi_steering_clientSet")) {
		UINT g_idx;
		INT ap_idx;
		mac_address_t mac_address;
		wifi_steering_clientConfig_t cli_cfg; /* 8 uint */

		g_idx = atoi(argv[2]);
		ap_idx = atoi(argv[3]);

		MACF_TO_MAC(argv[4], mac_address);
		cli_cfg.rssiProbeHWM = atoi(argv[5]);
		cli_cfg.rssiProbeLWM = atoi(argv[6]);
		cli_cfg.rssiAuthHWM = atoi(argv[7]);
		cli_cfg.rssiAuthLWM = atoi(argv[8]);
		cli_cfg.rssiInactXing = atoi(argv[9]);
		cli_cfg.rssiHighXing = atoi(argv[10]);
		cli_cfg.rssiLowXing = atoi(argv[11]);
		cli_cfg.authRejectReason = atoi(argv[12]);
		ret = wifi_steering_clientSet(g_idx, ap_idx, mac_address, &cli_cfg);
	} else if (!strcmp(argv[1], "wifi_steering_clientRemove")) {
		UINT g_idx;
		INT ap_idx;
		mac_address_t mac_address;

		g_idx = atoi(argv[2]);
		ap_idx = atoi(argv[3]);
		MACF_TO_MAC(argv[4], mac_address);
		ret = wifi_steering_clientRemove(g_idx, ap_idx, mac_address);
	} else if (!strcmp(argv[1], "wifi_steering_clientMeasure")) {
		UINT g_idx;
		INT ap_idx;
		mac_address_t mac_address;

		g_idx = atoi(argv[2]);
		ap_idx = atoi(argv[3]);
		MACF_TO_MAC(argv[4], mac_address);
		ret = wifi_steering_clientMeasure(g_idx, ap_idx, mac_address);
	} else if (!strcmp(argv[1], "wifi_steering_clientDisconnect")) {
		UINT g_idx;
		INT ap_idx;
		mac_address_t mac_address;
		wifi_disconnectType_t type;
		UINT reason;

		g_idx = atoi(argv[2]);
		ap_idx = atoi(argv[3]);
		MACF_TO_MAC(argv[4], mac_address);
		type = atoi(argv[5]);
		reason = atoi(argv[6]);
		ret = wifi_steering_clientDisconnect(g_idx, ap_idx, mac_address, type, reason);
	} else if (!strcmp(argv[1], "wifi_getRadioClientInactivityTimeout")) {
		int timeout = 0;

		ret = wifi_getRadioClientInactivityTimout(index, &timeout);
		if (ret == RETURN_OK) {
			printf("Client Inactivity Timeout: wl%d - %d\n", index, timeout);
		}
#if WIFI_HAL_VERSION_GE_2_18 || defined(WIFI_HAL_VERSION_3)
	} else if (!strcmp(argv[1], "wifi_chan_eventRegister")) {
		/* The below is just a dummy function wrapper to call wifi_channelChange_callback_register */
		wifi_chan_eventCB_t callback = chan_event_cb;
		wifi_chan_eventRegister(callback);
		pthread_join(cbThreadId, NULL);
#endif /* WIFI_HAL_VERSION_GE_2_18 */
#if WIFI_HAL_VERSION_GE_2_12 || defined(WIFI_HAL_VERSION_3)
	/* 802.11K api */
	} else if (!strcmp(argv[1], "wifi_RMBeaconRequest_callback_register")) {
		/* The below is just a dummy function wrapper to call
		* wifi_RMBeaconRequestCallbackRegister */
		wifi_RMBeaconReport_callback callback = wifi_RMBeaconReport_callback_test_func;
		ret = wifi_RMBeaconRequestCallbackRegister(index, callback);
		if (ret == RETURN_OK) {
			printf("%s complete. callback=%p\n", argv[1], callback);
			pthread_join(cbThreadId, NULL);
		}
	} else if (!strcmp(argv[1], "wifi_RMBeaconRequest_callback_unregister")) {
		wifi_RMBeaconReport_callback callback = wifi_RMBeaconReport_callback_test_func;
		ret = wifi_RMBeaconRequestCallbackRegister(index, callback);
		if (ret == RETURN_ERR) {
			printf("wifi_RMBeaconRequestCallbackRegister returned error\n");
			return ret;
		}
		printf("%s Done Register. 6min before Unregister callback=%p cbThreadId=%p\n",
			argv[1], callback, (void *)cbThreadId);
		sleep(360);
		/*  Do the setRMBeaconRequest test in these 360 sec - and look for callback
		*  - and see no callbacks if beyond the sleep time; then ecbd closes pipe
		*/
		ret = wifi_RMBeaconRequestCallbackUnregister(index, callback);
		if (ret == RETURN_OK) {
			printf("%s Unregister. callback=%p cbThreadId=%p\n", argv[1], callback,
				(void *)cbThreadId);
		}
	} else if (!strcmp(argv[1], "wifi_setRMBeaconRequest")) {
		wifi_BeaconRequest_t *bcnReqInfo;
		UCHAR *reqbuf, *bssidp, out_DialogToken = 0;
		int buflen, avInd;

		buflen = sizeof(wifi_BeaconRequest_t);
		reqbuf = malloc(buflen);
		if (reqbuf == NULL) {
			printf("%s Error Allocating reqbuf\n", argv[1]);
			return (-1);
		}

		memset(reqbuf, 0, buflen);
		bcnReqInfo = (wifi_BeaconRequest_t *)reqbuf;

		printf("%s argcnt=%d \n", argv[1], argc);
		/* Get wifi_BeaconRequest_t parameters */
		bcnReqInfo->opClass = atoi(argv[4]);
		bcnReqInfo->channel = atoi(argv[5]);
		bcnReqInfo->randomizationInterval = atoi(argv[6]);
		bcnReqInfo->duration = atoi(argv[7]);
		if (bcnReqInfo->duration > WLC_RRM_MAX_MEAS_DUR)
			bcnReqInfo->duration = WLC_RRM_MAX_MEAS_DUR; // about 1 sec
		if (bcnReqInfo->duration < 1) bcnReqInfo->duration = 1; // atleast 1 TU
		bcnReqInfo->mode = atoi(argv[8]);
		bssidp = (unsigned char *)(bcnReqInfo->bssid);
		if (argv[9] != NULL) {
			MACF_TO_MAC(argv[9], bssidp);
		} else {
			memset((bssidp), 0xff, 6);
		}
		if (argv[10] != NULL) {
			bcnReqInfo->numRepetitions = atoi(argv[10]);
		} else {
			bcnReqInfo->numRepetitions = 0;
		}
		/* Optional subelements - keep in order of subelement ID
		 * Ref: Table 9-88 802.11-2016
		 * SubElemID   Name		      Extensible
		 *   0         SSID                         No
		 *   1         beaconReporting              Yes
		 *   2         reportingDetail              Yes
		 *  10         requestedElementIDS          No
		 *  11         extdRequestedElementIDS      No
		 *  51         channelReport                No
		 * 163         wideBandwidthChannel         Yes
		 * 221         vendorSpecific               No
		 */
		avInd = 11;
		if ((argc >= avInd) && (argv[avInd] != NULL)) {
			/* ssidPresent ssid */
			bcnReqInfo->ssidPresent = (atoi(argv[avInd++]) == 0) ? FALSE : TRUE;
			if (bcnReqInfo->ssidPresent) {
				if (argc < avInd) {
					printf(" ssid Invalid param cnt %d\n", argc);
					return (-1);
				}
				if (argv[avInd] != NULL)
					strncpy((char *)(bcnReqInfo->ssid), argv[avInd],
						sizeof(ssid_t));
				avInd++;
				printf(" avInd=%d bcnReqInfo->ssidPresent=%d bcnReqInfo->ssid=%s\n",
					avInd, bcnReqInfo->ssidPresent, bcnReqInfo->ssid);
			}
		}

		if ((argc >= avInd) && (argv[avInd] != NULL)) {
			/* beaconReportingPresent condition threshold */
			bcnReqInfo->beaconReportingPresent = (atoi(argv[avInd++]) == 0) ?
				FALSE : TRUE;
			if (bcnReqInfo->beaconReportingPresent) {
				if (argc < (avInd+1)) {
					printf(" beaconReporting Invalid param cnt %d\n", argc);
					return (-1);
				}
				if (argv[avInd] != NULL)
					bcnReqInfo->beaconReporting.condition =
						(UCHAR) (atoi(argv[avInd++]));
				if (argv[avInd] != NULL)
					bcnReqInfo->beaconReporting.threshold =
						(UCHAR) (atoi(argv[avInd++]));
				printf(" avInd=%d bcnReqInfo->beaconReportingPresent=%d"
					" condition=%d threshold=%d\n",
					avInd, bcnReqInfo->beaconReportingPresent,
					bcnReqInfo->beaconReporting.condition,
					bcnReqInfo->beaconReporting.threshold);
			}
		}

		if ((argc >= avInd) && (argv[avInd] != NULL)) {
			/* reportingRetailPresent reportingDetail */
			bcnReqInfo->reportingRetailPresent = (atoi(argv[avInd++]) == 0) ?
				FALSE : TRUE;
			if (bcnReqInfo->reportingRetailPresent) {
				if (argc < avInd) {
					printf(" reportingDetail Invalid param cnt %d\n", argc);
					return (-1);
				}
				if (argv[avInd] != NULL)
					bcnReqInfo->reportingDetail =
						(UCHAR) (atoi(argv[avInd++]));
				/* reportingDetail valid values are 0,1,2 */
				printf(" avInd=%d bcnReqInfo->reportingRetailPresent=%d"
					"reportingDetail=%d\n",
					avInd, bcnReqInfo->reportingRetailPresent,
					bcnReqInfo->reportingDetail);
			}
		}

		/* TBD - requestedElementIDS extdRequestedElementIDS channelReport
		 * wideBandwidthChannel
		 */
		if ((argc >= avInd) && (argv[avInd] != NULL)) {
			/* wideBandWidthChannelPresent wideBandwidthChannel */
			bcnReqInfo->wideBandWidthChannelPresent = (atoi(argv[avInd++]) == 0) ?
				FALSE : TRUE;
			if (bcnReqInfo->wideBandWidthChannelPresent) {
				if (argc < (avInd + 2)) {
					printf(" wideBandwidthCh Invalid param cnt %d\n", argc);
					return (-1);
				}
				wifi_WideBWChannel_t *wbcp = &(bcnReqInfo->wideBandwidthChannel);
				if (argv[avInd] != NULL)
					wbcp->bandwidth = (UCHAR) (atoi(argv[avInd++]));
				if (argv[avInd] != NULL)
					wbcp->centerSeg0 = (UCHAR) (atoi(argv[avInd++]));
				if (argv[avInd] != NULL)
					wbcp->centerSeg1 = (UCHAR) (atoi(argv[avInd++]));
				printf(" avInd=%d bcnReqInfo->wideBandWidthChannelPresent=%d bw=%d"
					" centerSeg0=%d centerSeg1=%d\n", avInd,
					bcnReqInfo->wideBandWidthChannelPresent, wbcp->bandwidth,
					wbcp->centerSeg0, wbcp->centerSeg1);
			}
		}
		printf(" call wifi_setRMBeaconRequest argc=%d\n", argc);

		/* API_DBG_PRINT_BUF(__FUNCTION__, (unsigned char *)bcnReqInfo, buflen); */
#ifdef WIFI_HAL_VERSION_3_PHASE2
		mac_address_t mac_address;
		MACF_TO_MAC(argv[3], mac_address);
		ret = wifi_setRMBeaconRequest(index, mac_address, bcnReqInfo, &out_DialogToken);
#else /* WIFI_HAL_VERSION_3_PHASE2 */
		ret = wifi_setRMBeaconRequest(index, argv[3], bcnReqInfo, &out_DialogToken);
#endif /* WIFI_HAL_VERSION_3_PHASE2 */
		free(reqbuf);
		if (ret == RETURN_OK) {
			printf("%s ap %d complete out_DialogToken %d\n", argv[1], index,
				out_DialogToken);
		}
	} else if (!strcmp(argv[1], "wifi_cancelRMBeaconRequest")) {
		unsigned char dtoken =  0;

		dtoken = atoi(argv[2]);
		ret = wifi_cancelRMBeaconRequest(index, dtoken);
		if (ret == RETURN_OK) {
			printf("%s complete\n", argv[1]);
		}
	} else if (!strcmp(argv[1], "wifi_getRMCapabilities")) {
		// Input:  UCHAR peerMACAddress[6]
		// Output: UCHAR out_Capabilities[5]
		UCHAR out_Capabilities[5];
		mac_address_t mac;
		printf("Enter %s wifi_getRMCapabilities \n", __FUNCTION__);
		MACF_TO_MAC(argv[2], mac);
#ifdef WIFI_HAL_VERSION_3_PHASE2
		ret = wifi_getRMCapabilities(mac, out_Capabilities);
#else /* WIFI_HAL_VERSION_3_PHASE2 */
		ret = wifi_getRMCapabilities((CHAR *)(&mac[0]), out_Capabilities);
#endif /* WIFI_HAL_VERSION_3_PHASE2 */
		if (ret == RETURN_OK) {
			printf("In %s out_Capabilities = %02X %02X %02X %02X %02X  \n",
				__FUNCTION__,  out_Capabilities[0], out_Capabilities[1],
				out_Capabilities[2], out_Capabilities[3], out_Capabilities[4]);
		}
	} else if (!strcmp(argv[1], "wifi_setNeighborReports")) {
		/*	Example: wifi_api wifi_setNeighborReports [apIndex] [cnt]
		*	[bssid1] [bssid info1] [regulatory1] [channel1] [phytype1]
		*	[bssid2] [bssid info2] [regulatory2] [channel2] [phytype2]
		*/
		UCHAR *reqbuf = NULL;
		int buflen, i, k, aind;
		int ret = 0;
		int ncnt = 0;
		unsigned int macInt[6];
		wifi_NeighborReport_t *nrp;
		int nrsize;
		UCHAR *bssidp;

		printf("%s argcnt=%d \n", argv[1], argc);

		ncnt = atoi(argv[3]);
		if ((ncnt < 0) ||
			(ncnt > MAX_NBR_CNT) ||
			(argc < (4 + (ncnt * NUM_INPARAM_PER_NBR)))) {
			printf("%s Error in input\n", argv[1]);
			return (-1);
		} else {
			if (ncnt > 0) {
				nrsize = sizeof(wifi_NeighborReport_t);
				/* Get request buffer */
				buflen = ncnt * nrsize;
				reqbuf = malloc(buflen);
				if (reqbuf == NULL) {
					printf("%s Error Allocating reqbuf\n", argv[1]);
					return (-1);
				}
				memset((void *)(reqbuf), 0, buflen);

				/* parse input neighbor info and fill reqbuf */
				for (i = 0; i < ncnt; i++) {
					// [bssid] [bssid info] [regulatory] [channel] [phytype]
					aind = 4 + (i * NUM_INPARAM_PER_NBR);
					if (argv[aind] == NULL) {
						free(reqbuf);
						printf("%s Error NULL bssid \n", argv[1]);
						return (-1);
					}
					sscanf(argv[aind], "%02x:%02x:%02x:%02x:%02x:%02x",
						&macInt[0], &macInt[1], &macInt[2], &macInt[3],
						&macInt[4], &macInt[5]);
					nrp = (wifi_NeighborReport_t *)
						((UCHAR *)reqbuf + (i * nrsize));
					bssidp = (unsigned char *)(nrp->bssid);
					for (k = 0; k < 6; k++) {
						*bssidp = (unsigned char)(macInt[k]);
						bssidp++;
					}
					++aind;
					nrp->info = atoi(argv[aind++]);
					nrp->opClass = (UCHAR)(atoi(argv[aind++]));
					nrp->channel = (UCHAR)(atoi(argv[aind++]));
					nrp->phyTable = (UCHAR)(atoi(argv[aind++]));
				}
			} /* ncnt > 0 */
			ret = wifi_setNeighborReports(index, ncnt,
				(wifi_NeighborReport_t *)(reqbuf));

			if (reqbuf != NULL) {
				free(reqbuf);
			}
			if (ret == RETURN_OK) {
				printf("%s complete\n", argv[1]);
			}
		}
	} else if (!strcmp(argv[1], "wifi_setNeighborReportActivation")) {
		wifi_api_info_t apiInfo;
		strncpy(apiInfo.api_name, argv[1], 1024);
		snprintf(apiInfo.api_data, 1024, "%s", argv[3]);
		apiInfo.radioIndex = index;
		ret = wifi_api_send_msg(&apiInfo);
	} else if (!strcmp(argv[1], "wifi_getNeighborReportActivation")) {
		BOOL activate;
		printf("%s argcnt=%d \n", argv[1], argc);
		ret = wifi_getNeighborReportActivation(index, &activate);
		if (ret == RETURN_OK) {
			printf("%s complete. returned %d index=%d activate=%d \n",
				argv[1], ret, index, activate);
		}
	}
	/* 802.11K api */
	else if (!strcmp(argv[1], "wifi_getRadioDCSSupported")) {
		BOOL dcs;
		ret = wifi_getRadioDCSSupported(index, &dcs);
		if (ret == RETURN_OK) {
			printf("%s complete. index=%d dcs=%d\n", argv[1], index, dcs);
		}
	} else if (!strcmp(argv[1], "wifi_getRadioDCSEnable")) {
		BOOL dcs;
		ret = wifi_getRadioDCSEnable(index, &dcs);
		if (ret == RETURN_OK) {
			printf("%s complete. index=%d dcs=%d\n", argv[1], index, dcs);
		}
	/* 802.11V support */
	} else if (!strcmp(argv[1], "wifi_BTMQueryRequest_callback_register")) {
		/* The below is just a dummy function wrapper to call
		* wifi_RMBeaconRequestCallbackRegister */
		wifi_BTMQueryRequest_callback callback1 = wifi_BTMQueryRequest_callback_test_func;
		wifi_BTMResponse_callback callback2 = wifi_BTMResponse_callback_test_func;

		ret = wifi_BTMQueryRequest_callback_register(index, callback1, callback2);
		if (ret == RETURN_OK) {
			printf("%s complete. callback1=%p callback1=%p\n", argv[1], callback1, callback2);
			pthread_join(cbThreadId, NULL);
		}
	} else if (!strcmp(argv[1], "wifi_setBTMRequest")) {
		/* The API format:
		apidx peer_mac token mode [<url_len> <url>] <num-candidate> <len1>
		<00904C1DA06100000000162400> <len2> ...
		*/
		wifi_BTMRequest_t *btmReqInfo;
		int buflen, k;
		UCHAR *reqbuf = NULL;
		UCHAR mode;
		UCHAR *dest;
		char *src;
		char hexstr[3];
		unsigned int idx, url_len;
		int argc_idx;
#ifdef WIFI_HAL_VERSION_3_PHASE2
		mac_address_t mac_address;
#endif /* WIFI_HAL_VERSION_3_PHASE2 */

		buflen = sizeof(wifi_BTMRequest_t);
		printf("%s: buflen=%d argc=%d\n", argv[1], buflen, argc);

		reqbuf = malloc(buflen);
		if (reqbuf == NULL) {
			printf("%s Error Allocating reqbuf\n", argv[1]);
			goto btm_reg_error;
		}

		memset(reqbuf, 0, buflen);
		btmReqInfo = (wifi_BTMRequest_t *)reqbuf;

		if (argv[3] == NULL) {
			printf("%s Error: peer MAC can't be NULL\n", argv[1]);
			goto btm_reg_error;
		}

		/* read req mode */
		mode = 5; /* default mode */
		if (argv[5] != NULL && argc >= 6) {
			mode = (UCHAR)atoi(argv[5]);
		}

		/* Get wifi_BTMRequest_t parameters */

		btmReqInfo->token = strtoul(argv[4], NULL, 0);
		btmReqInfo->requestMode = mode; /* DOT11_BSSTRANS_REQMODE_PREF_LIST_INCL|DOT11_BSSTRANS_REQMODE_ABRIDGED */
		btmReqInfo->timer = 0x0000;
		btmReqInfo->validityInterval = 0xFF;

		/* handle option field "termDuration" */
		if (mode & DOT11_BSSTRANS_REQMODE_BSS_TERM_INCL) {
			/* bit-3 termDuration present */
			/* TODO: the lower function wlcsm_mngr_wifi_setBTMRequest will read the real tsf */
			btmReqInfo->termDuration.duration = 2; /* test only */
		}

		argc_idx = 6; /* argument after "mode" */
		/* handle option field "url" */
		url_len = 0;
		if (mode & DOT11_BSSTRANS_REQMODE_ESS_DISASSOC_IMNT) {
			/* bit-4 url present */
			if (argc < (argc_idx + 2)) {
				printf("%s Error: need url_len and url string \n", argv[1]);
				goto btm_reg_error;
			}

			url_len = atoi(argv[argc_idx++]);
			if (url_len >= MAX_URL_LEN) {
				printf("%s url_len %d is too long\n", argv[1], url_len);
				goto btm_reg_error;
			}

			/* str argv[argc_idx++] to hex */
			dest = (UCHAR *)btmReqInfo->url;
			btmReqInfo->urlLen = url_len;
			if ((src = argv[argc_idx++]) == NULL) {
				printf("%s url content not exist\n", argv[1]);
				goto btm_reg_error;
			}

			/* string to hex */
			for (idx = 0; idx < url_len; idx++) {
				hexstr[0] = src[0];
				hexstr[1] = src[1];
				hexstr[2] = '\0';

				*dest = (UCHAR)strtoul(hexstr, NULL, 16);

				printf("%s (url): idx=%d dest=%p value=0x%x\n", argv[1],
					idx, dest, *dest);
				dest++;
				src += 2;
			}
		}

		printf("%s: argc_idx=%d url_len=%d mode=0x%x\n", argv[1], argc_idx, url_len, mode);

		/* handle candidate list */
		if (argv[argc_idx] != NULL && argc >= (argc_idx + 1)) {
			UCHAR count;
			unsigned int candidate_len;

			count = atoi(argv[argc_idx++]);
			if ((count <= 0) || (count > BTM_MAX_CANDIDATES)) {
				printf("%s Error: num of candidates %d is out of range (1-%d)\n",
					   argv[1], count, BTM_MAX_CANDIDATES);
				goto btm_reg_error;
			}
			btmReqInfo->numCandidates = count;

			for (k = 0; k < count; k++) {
				if (argv[argc_idx] == NULL || argv[argc_idx + 1] == NULL) {
					printf("Error: missing <len> <hex-string> pair for candidate %d\n",
						k + 1);
					goto btm_reg_error;
				}

				candidate_len = atoi(argv[argc_idx++]);

				if (candidate_len != strlen(argv[argc_idx])/2) {
					printf("Error: len %d not match hex-string len %d (divided by 2)\n",
						candidate_len, strlen(argv[argc_idx]));
					goto btm_reg_error;
				}

				dest = (UCHAR *)&(btmReqInfo->candidates[k]);
				src = argv[argc_idx++];

				printf("%s: btmReqInfo=%p dest=%p argc_idx=%d\n", argv[1],
					btmReqInfo, dest, argc_idx);
				/* string to hex */
				for (idx = 0; idx < candidate_len; idx++) {
					hexstr[0] = src[0];
					hexstr[1] = src[1];
					hexstr[2] = '\0';

					*dest = (UCHAR)strtoul(hexstr, NULL, 16);

					printf("%s: idx=%d dest=%p value=0x%x\n", argv[1], idx,
						dest, *dest);
					dest++;
					src += 2;
				}
			}
		}
		else {
			printf("Error: missing candidates\n");
			goto btm_reg_error;
		}

#ifdef WIFI_HAL_VERSION_3_PHASE2
		MACF_TO_MAC(argv[3], mac_address);
		ret = wifi_setBTMRequest(index, mac_address, btmReqInfo);
#else /* WIFI_HAL_VERSION_3_PHASE2 */
		ret = wifi_setBTMRequest(index, argv[3], btmReqInfo);
#endif /* WIFI_HAL_VERSION_3_PHASE2 */

		if (reqbuf) {
			free(reqbuf);
			reqbuf = NULL;
		}
		printf("%s complete. returned %d\n", argv[1], ret);
		return (ret);

	btm_reg_error:
		if (reqbuf) {
			free(reqbuf);
			reqbuf = NULL;
		}
		printf("Sample: wifi_api wifi_setBTMRequest 1 F0:99:BF:76:FF:54 15 5 1 13 44AAF59B178F000000000C0B00\n");
	} else if (!strcmp(argv[1], "wifi_getBSSTransitionImplemented")) {
		BOOL bsst_impl = 1;
		ret = wifi_getBSSTransitionImplemented(index, &bsst_impl);
		if (ret == RETURN_OK) {
			printf("%s complete. BSSTransitionImplemented=%d\n", argv[1], bsst_impl);
		}
	} else if (!strcmp(argv[1], "wifi_setBSSTransitionActivation")) {
		BOOL enable;
		if (atoi(argv[3]) == 0) enable = 0;
		else enable = 1;

		ret = wifi_setBSSTransitionActivation(index, enable);
		if (ret == RETURN_OK) {
			printf("%s complete=\n", argv[1]);
		}
	} else if (!strcmp(argv[1], "wifi_getBSSTransitionActivation")) {
		BOOL enable;
		ret = wifi_getBSSTransitionActivation(index, &enable);
		if (ret == RETURN_OK) {
			printf("%s complete. BSSTransitionActivation=%d\n", argv[1], enable);
		}
	} else if (!strcmp(argv[1], "wifi_getBTMClientCapabilityList")) {
		wifi_BTMCapabilities_t btm_caps, *btm_ptr = &btm_caps;
		int count = 0, i, k;

		mac_address_t *mac_ptr = NULL;
		unsigned int macInt[6] = { 0 };
		unsigned char *ptr;

		count = argc - 3;
		if (count > MAX_BTM_DEVICES) {
			printf("Warning:  STA counts %d exceeds the limitation %d\n",
				count, MAX_BTM_DEVICES);
			return (ret);
		}

		memset(btm_ptr, 0, sizeof(wifi_BTMCapabilities_t));

		btm_ptr->entries = count;
		mac_ptr = btm_ptr->peer;

		for (i = 0; i < count; i++) {
			/* MAC string to hex */
			if (argv[3 + i] != NULL) {
			sscanf(argv[3 + i], "%02x:%02x:%02x:%02x:%02x:%02x", &macInt[0], &macInt[1],
				   &macInt[2], &macInt[3], &macInt[4], &macInt[5]);

			ptr = (unsigned char *)(mac_ptr++);

			for (k = 0; k < 6; k++) {
				ptr[k] = (unsigned char)macInt[k];
			}

			printf("%s ptr=%p mac_ptr=%p MAC="MACF"\n", argv[1], ptr,
				mac_ptr, MAC_TO_MACF(ptr));
			}
		}

		ret = wifi_getBTMClientCapabilityList(index, btm_ptr);
		if (ret == RETURN_OK) {
			printf("%s complete. count=%d\n", argv[1], count);
		}
	/* end of 802.11V support */

	/* HS2.0 api */
	} else if (!strcmp(argv[1], "wifi_getApInterworkingElement")) {
		wifi_InterworkingElement_t output_struct;
		ret = wifi_getApInterworkingElement(index, &output_struct);
		if (ret == RETURN_OK) {
			printf("interworkingEnabled:	%d\n", output_struct.interworkingEnabled);
			printf("accessNetworkType:	%d\n", output_struct.accessNetworkType);
			printf("internetAvailable:	%d\n", output_struct.internetAvailable);
			printf("asra:			%d\n", output_struct.asra);
			printf("esr:			%d\n", output_struct.esr);
			printf("uesa:			%d\n", output_struct.uesa);
			printf("venueOptionPresent:	%d\n", output_struct.venueOptionPresent);
			printf("venueGroup:		%d\n", output_struct.venueGroup);
			printf("venueType:		%d\n", output_struct.venueType);
			printf("hessOptionPresent:	%d\n", output_struct.hessOptionPresent);
			printf("hessid:			%s\n", output_struct.hessid);
		}
	} else if (!strcmp(argv[1], "wifi_getInterworkingAccessNetworkType")) {
		UINT networkType;
		ret = wifi_getInterworkingAccessNetworkType(index, &networkType);
		if (ret == RETURN_OK) {
			printf("%u\n", networkType);
		}
	} else if (!strcmp(argv[1], "wifi_getApInterworkingServiceCapability")) {
		BOOL interworking;
		ret = wifi_getApInterworkingServiceCapability(index, &interworking);
		if (ret == RETURN_OK) {
			printf("%s\n", interworking ? "TRUE" : "FALSE");
		}
	} else if (!strcmp(argv[1], "wifi_getApInterworkingServiceEnable")) {
		BOOL enable;
		ret = wifi_getApInterworkingServiceEnable(index, &enable);
		if (ret == RETURN_OK) {
			printf("%s\n", enable ? "TRUE" : "FALSE");
		}
#endif /* WIFI_HAL_VERSION_GE_2_12 */
#if WIFI_HAL_VERSION_GE_2_19 || defined(WIFI_HAL_VERSION_3)
	} else if (!strcmp(argv[1], "wifi_pushApHotspotElement")) {
		BOOL enable = atoi(argv[3]) ? TRUE : FALSE;
		ret = wifi_pushApHotspotElement(index, enable);
		if (ret == RETURN_OK) {
			printf("%s\n", enable ? "TRUE" : "FALSE");
		}
	} else if (!strcmp(argv[1], "wifi_getApHotspotElement")) {
		BOOL enable;
		ret = wifi_getApHotspotElement(index, &enable);
		if (ret == RETURN_OK) {
			printf("%s\n", enable ? "TRUE" : "FALSE");
		}
	} else if (!strcmp(argv[1], "wifi_pushApRoamingConsortiumElement")) {
		wifi_roamingConsortiumElement_t cons_ie, *infoElement = &cons_ie;
		char ouilist[64], *ptr, *s, *rest;
		int total = 0, len;

		/* string (nvram) to hex */
		strncpy(ouilist, argv[3], sizeof(ouilist) - 1);
		ptr = strtok_r(ouilist, ";", &rest);
		while (ptr) {
			if ((s = strchr(ptr, ':')) != NULL)
				*s = '\0';
			len = strlen(ptr) / 2;
			printf("%s: index=%d string=%s len=%d\n",
				__FUNCTION__, total, ptr, len);
			if ((len < 15) && (total < 3)) {
				get_hex_data((unsigned char *)ptr,
					infoElement->wifiRoamingConsortiumOui[total], len);
				infoElement->wifiRoamingConsortiumLen[total] = len;
			} else {
				printf("%s: Skip index=%d string=%s len=%d\n",
					__FUNCTION__, total, ptr, len);
			}
			/* only allow first 3 OUIs go to the data struct,
			   additional as "Number of ANQP OIs:"
			*/
			total++;
			ptr = strtok_r(NULL, ";", &rest);
		}
		infoElement->wifiRoamingConsortiumCount = total;

		ret = wifi_pushApRoamingConsortiumElement(index, infoElement);
		if (ret == RETURN_OK) {
			printf("wifi_pushApRoamingConsortiumElement %s\n", argv[3]);
		}
	} else if (!strcmp(argv[1], "wifi_getApRoamingConsortiumElement")) {
		wifi_roamingConsortiumElement_t infoElement;
		ret = wifi_getApRoamingConsortiumElement(index, &infoElement);
		if (ret == RETURN_OK) {
			printf("wifi_getApRoamingConsortiumElement count=%d\n",
				infoElement.wifiRoamingConsortiumCount);
		}
	} else if (!strcmp(argv[1], "wifi_setCountryIe")) {
		BOOL enable = atoi(argv[3]) ? TRUE : FALSE;
		ret = wifi_setCountryIe(index, enable);
		if (ret == RETURN_OK) {
			printf("%s complete. ret=%d\n", argv[1], ret);
		}
	} else if (!strcmp(argv[1], "wifi_getCountryIe")) {
		BOOL enable;
		ret = wifi_getCountryIe(index, &enable);
		if (ret == RETURN_OK) {
			printf("%s\n", enable ? "TRUE" : "FALSE");
		}
	} else if (!strcmp(argv[1], "wifi_setLayer2TrafficInspectionFiltering")) {
		BOOL enable = atoi(argv[3]) ? TRUE : FALSE;
		ret = wifi_setLayer2TrafficInspectionFiltering(index, enable);
		if (ret == RETURN_OK) {
			printf("%s complete. ret=%d\n", argv[1], ret);
		}
	} else if (!strcmp(argv[1], "wifi_getLayer2TrafficInspectionFiltering")) {
		BOOL enable;
		ret = wifi_getLayer2TrafficInspectionFiltering(index, &enable);
		if (ret == RETURN_OK) {
			printf("%s\n", enable ? "TRUE" : "FALSE");
		}
	} else if (!strcmp(argv[1], "wifi_setDownStreamGroupAddress")) {
		BOOL enable = atoi(argv[3]) ? TRUE : FALSE;
		ret = wifi_setDownStreamGroupAddress(index, enable);
		if (ret == RETURN_OK) {
			printf("%s complete. ret=%d\n", argv[1], ret);
		}
	} else if (!strcmp(argv[1], "wifi_getDownStreamGroupAddress")) {
		BOOL enable;
	        ret = wifi_getDownStreamGroupAddress(index, &enable);
		if (ret == RETURN_OK) {
		        printf("%s\n", enable ? "TRUE" : "FALSE");
		}
	} else if (!strcmp(argv[1], "wifi_setBssLoad")) {
		BOOL enable = atoi(argv[3]) ? TRUE : FALSE;
		ret = wifi_setBssLoad(index, enable);
		if (ret == RETURN_OK) {
			printf("%s complete. ret=%d\n", argv[1], ret);
		}
	} else if (!strcmp(argv[1], "wifi_getBssLoad")) {
		BOOL enable;
		ret = wifi_getBssLoad(index, &enable);
		if (ret == RETURN_OK) {
			printf("%s\n", enable ? "TRUE" : "FALSE");
		}
	} else if (!strcmp(argv[1], "wifi_setProxyArp")) {
		BOOL enable = atoi(argv[3]) ? TRUE : FALSE;
		ret = wifi_setProxyArp(index, enable);
		if (ret == RETURN_OK) {
			printf("%s complete. ret=%d\n", argv[1], ret);
		}
	} else if (!strcmp(argv[1], "wifi_getProxyArp")) {
		BOOL enable;
		ret = wifi_getProxyArp(index, &enable);
		if (ret == RETURN_OK) {
			printf("%s\n", enable ? "TRUE" : "FALSE");
		}
	} else if (!strcmp(argv[1], "wifi_setP2PCrossConnect")) {
		BOOL enable = atoi(argv[3]) ? TRUE : FALSE;
		ret = wifi_setP2PCrossConnect(index, enable);
		if (ret == RETURN_OK) {
			printf("%s complete. ret=%d\n", argv[1], ret);
		}
	} else if (!strcmp(argv[1], "wifi_getP2PCrossConnect")) {
		BOOL enable;
		ret = wifi_getP2PCrossConnect(index, &enable);
		if (ret == RETURN_OK) {
			printf("%s\n", enable ? "TRUE" : "FALSE");
		}
	} else if (!strcmp(argv[1], "wifi_applyGASConfiguration")) {
		wifi_GASConfiguration_t input_struct;
		input_struct.QueryResponseLengthLimit = atoi(argv[2]);
		ret = wifi_applyGASConfiguration(&input_struct);
		if (ret == RETURN_OK) {
			printf("%s complete. ret=%d\n", argv[1], ret);
		}
#endif /* WIFI_HAL_VERSION_GE_2_19 */
/* End of HS2.0 api */
#if WIFI_HAL_VERSION_GE_2_16 || defined(WIFI_HAL_VERSION_3)
/* DPP support */
#if WIFI_HAL_VERSION_GE_2_19 || defined(WIFI_HAL_VERSION_3)
	} else if (!strcmp(argv[1], "wifi_mgmt_frame_callbacks_register")) {
		/* The below is just a dummy function wrapper to call
		* wifi_dpp_frame_received_callbacks_register */
		wifi_receivedMgmtFrame_callback callback1 =
			wifi_receivedMgmtFrame_callback_test_func;

		ret = wifi_mgmt_frame_callbacks_register(callback1);
		if (ret == RETURN_OK) {
			printf("%s complete. callback1=%p\n", argv[1], callback1);
			pthread_join(cbThreadId, NULL);
		}
#else
	} else if (!strcmp(argv[1], "wifi_dpp_frame_received_callbacks_register")) {
		/* The below is just a dummy function wrapper to call
		* wifi_dpp_frame_received_callbacks_register */
		wifi_dppAuthResponse_callback_t callback1 =
			wifi_dppAuthResponse_callback_test_func;
		wifi_dppConfigRequest_callback_t callback2 =
			wifi_dppConfigRequest_callback_test_func;

		ret = wifi_dpp_frame_received_callbacks_register(callback1, callback2);
		if (ret == RETURN_OK) {
			printf("%s complete. callback1=%p callback2=%p\n", argv[1], callback1,
				callback2);
			pthread_join(cbThreadId, NULL);
		}
#endif /* WIFI_HAL_VERSION_GE_2_19 */
	} else if(!strcmp(argv[1], "wifi_sendActionFrame")) {
		/* The API format from argv[2]:
		* apidx peer_mac frequency <frame in hex>
		*/
		UCHAR af_buf[1400], peerMACAddress[6],  *dest;
		int buflen, k, af_len = 0;
		unsigned int macInt[6];
		UINT frequency = 0;
		char *src, hexstr[3];

		buflen = sizeof(af_buf);
		printf("%s: buflen=%d argc=%d\n", argv[1], buflen, argc);

		memset(af_buf, 0, buflen);

		if (sscanf(argv[3], "%02x:%02x:%02x:%02x:%02x:%02x", &macInt[0], &macInt[1],
			&macInt[2], &macInt[3], &macInt[4], &macInt[5]) != 6) {
			printf("%s Error: peer MAC format %s\n", argv[1], argv[3]);
			goto action_frame_error;
		}

		for (k = 0; k < 6; k++) {
			peerMACAddress[k] = (unsigned char)macInt[k];
		}

		/* read channel/frequency */
		frequency = (UINT)atoi(argv[4]);

		/* handle action frame string */
		if (argv[5] != NULL) {
			dest = af_buf;
			src = argv[5];

			/* string to hex */
			while (*src != 0) {
				hexstr[0] = src[0];
				hexstr[1] = src[1];
				if (src[1] == 0) {
					printf("%s Error: action frame hex format incorrect %s\n",
						argv[1], argv[5]);
					goto action_frame_error;
				}
				hexstr[2] = '\0';

				*dest = (UCHAR) strtoul(hexstr, NULL, 16);

				af_len++;
				printf("%s: idx=%d dest=%p value=0x%x\n", argv[1], af_len,
					dest, *dest);
				dest++;
				src += 2;
			}
		}

		printf("%s: channel=%d af_len=%d\n", argv[1], frequency, af_len);

		ret = wifi_sendActionFrame(index, (UCHAR *)(&(peerMACAddress[0])), frequency,
			af_buf, af_len);
		if (ret == RETURN_OK) {
			printf("%s complete.\n", argv[1]);
			return (ret);
		}
action_frame_error:
		printf("Usage: wifi_sendActionFrame <apIndex> <peerMACAddress>"
			   "channel <action frame like 0409506F9A1A0100>\n");
		return RETURN_ERR;
/* end of DPP */
#endif /* WIFI_HAL_VERSION_GE_2_16 */
#if WIFI_HAL_VERSION_GE_2_15 && (!defined(_CBR_PRODUCT_REQ_) && \
	!defined(_XF3_PRODUCT_REQ_))
	} else if (!strcmp(argv[1], "wifi_getRadioMode")) {
		UINT pureMode;  /* Bits:  | ax | ac | n | a | g | b | */
		char standards[64];
		ret = wifi_getRadioMode(index, standards, &pureMode);
		if (ret == RETURN_OK) {
			printf("standards=%s pureMode=0x%x \n",  standards, pureMode);
		}
	} else if (!strcmp(argv[1], "wifi_setRadioMode")) {
		if ((argv[3] != NULL) && (argv[4] != NULL)) {
			wifi_api_info_t apiInfo;
			strncpy(apiInfo.api_name, argv[1], 1024);
			/* pass channelMode, pureMode */
			snprintf(apiInfo.api_data, 1024, "%s %d", argv[3], atoi(argv[4]));
			apiInfo.radioIndex = index;
			ret = wifi_api_send_msg(&apiInfo);
		}
	} else if (!strcmp(argv[1], "wifi_setDownlinkDataAckType")) {
		wifi_dl_data_ack_type_t ack_type;
		ack_type = (wifi_dl_data_ack_type_t)(atoi(argv[3]));
		ret = wifi_setDownlinkDataAckType(index, ack_type);
		if (ret == RETURN_OK) {
			printf("%s-TBD complete\n", argv[1]);
		}
	} else if (!strcmp(argv[1], "wifi_getDownlinkMuType")) {
		wifi_dl_mu_type_t ack_type;
		ret = wifi_getDownlinkMuType(index, &ack_type);
		if (ret == RETURN_OK) {
			printf("TBD-DownlinkMuType=%d\n", (unsigned int)ack_type);
		}
	} else if (!strcmp(argv[1], "wifi_getUplinkMuType")) {
		wifi_ul_mu_type_t ack_type;
		ret = wifi_getUplinkMuType(index, &ack_type);
		if (ret == RETURN_OK) {
			printf("TBD-UplinkMuType=%d\n", (unsigned int)ack_type);
		}
	} else if (!strcmp(argv[1], "wifi_setGuardInterval")) {
		wifi_guard_interval_t guard_interval;
		guard_interval = (wifi_guard_interval_t)(atoi(argv[3]));
		ret = wifi_setGuardInterval(index, guard_interval);
		if (ret == RETURN_OK) {
			printf("%s complete index=%d argc=%d\n", argv[1], index, argc);
		}
	} else if (!strcmp(argv[1], "wifi_getGuardInterval")) {
		wifi_guard_interval_t guard_interval;
		ret = wifi_getGuardInterval(index, &guard_interval);
		if (ret == RETURN_OK) {
			printf("guard_interval=0x%x\n", (unsigned int)guard_interval);
		}
	} else if (!strcmp(argv[1], "wifi_getBSSColorEnabled")) {
		BOOL enabled;
		ret = wifi_getBSSColorEnabled(index, &enabled);
		if (ret == RETURN_OK) {
			printf("BSSColorEnabled=%s\n", (enabled) ? "TRUE" : "FALSE");
		}
	} else if (!strcmp(argv[1], "wifi_getBSSColor")) {
		UCHAR bssColor;
		ret = wifi_getBSSColor(index, &bssColor);
		if (ret == RETURN_OK) {
			printf("BSSColor=%d\n", (int)bssColor);
		}
	} else if (!strcmp(argv[1], "wifi_getTWTParams")) {
		wifi_twt_params_t twt_params;
		CHAR sta[64] = { 0 };
		ret = wifi_getTWTParams(sta, &twt_params);
		if (ret == RETURN_OK) {
			/* TBD - print twt param for given sta */
			printf("%s-TBD index=%d argc=%d\n", argv[1], index, argc);
		}
	} else if (!strcmp(argv[1], "wifi_get80211axDefaultParameters")) {
		wifi_80211ax_params_t params;
		ret = wifi_get80211axDefaultParameters(index, &params);
		if (ret == RETURN_OK) {
			/* TBD - print axDefaultParameters */
			printf("%s-TBD index=%d argc=%d\n", argv[1], index, argc);
		}
	/* 11ax api done */
#endif /* (WIFI_HAL_VERSION_GE_2_15) &&
	(!defined(_CBR_PRODUCT_REQ_) && !defined(_XF3_PRODUCT_REQ_)) */
	} else if ((!strcmp(argv[1], "wifi_setApEnable")) ||
		(!strcmp(argv[1], "wifi_pushRadioChannel")) ||
		(!strcmp(argv[1], "wifi_setRadioDcsScanning")) ||
		(!strcmp(argv[1], "wifi_setApBasicAuthenticationMode")) ||
		(!strcmp(argv[1], "wifi_setApWpaEncryptionMode")) ||
		(!strcmp(argv[1], "wifi_setApAuthMode")) ||
		(!strcmp(argv[1], "wifi_setApWpsDevicePIN")) ||
		(!strcmp(argv[1], "wifi_setRadio11nGreenfieldEnable")) ||
		(!strcmp(argv[1], "wifi_setApWpsEnable")) ||
		(!strcmp(argv[1], "wifi_setRadioBasicDataTransmitRates")) ||
		(!strcmp(argv[1], "wifi_setRadioOperatingChannelBandwidth")) ||
		(!strcmp(argv[1], "wifi_setApWpsConfigMethodsEnabled")) ||
		(!strcmp(argv[1], "wifi_setRadioChannel")) ||
		(!strcmp(argv[1], "wifi_setRadioAutoChannelEnable")) ||
		(!strcmp(argv[1], "wifi_setRadioAutoChannelRefreshPeriod")) ||
		(!strcmp(argv[1], "wifi_setRadioEnable")) ||
		(!strcmp(argv[1], "wifi_setRadioTransmitPower")) ||
		(!strcmp(argv[1], "wifi_setRadioCountryCode")) ||
		(!strcmp(argv[1], "wifi_setApDTIMInterval")) ||
		(!strcmp(argv[1], "wifi_setRadioCtsProtectionEnable")) ||
		(!strcmp(argv[1], "wifi_setRadioObssCoexistenceEnable")) ||
		(!strcmp(argv[1], "wifi_setRadioFragmentationThreshold")) ||
		(!strcmp(argv[1], "wifi_setRadioSTBCEnable")) ||
		(!strcmp(argv[1], "wifi_setRadioAMSDUEnable")) ||
		(!strcmp(argv[1], "wifi_setRadioGuardInterval")) ||
		(!strcmp(argv[1], "wifi_setRadioTxChainMask")) ||
		(!strcmp(argv[1], "wifi_setRadioRxChainMask")) ||
		(!strcmp(argv[1], "wifi_setSSIDName")) ||
		(!strcmp(argv[1], "wifi_pushSSID")) ||
		(!strcmp(argv[1], "wifi_setApSsidAdvertisementEnable")) ||
		(!strcmp(argv[1], "wifi_pushSsidAdvertisementEnable")) ||
		(!strcmp(argv[1], "wifi_setApSecurityPreSharedKey")) ||
		(!strcmp(argv[1], "wifi_setApWpsEnrolleePin")) ||
		(!strcmp(argv[1], "wifi_setApWmmEnable")) ||
		(!strcmp(argv[1], "wifi_setApWmmUapsdEnable")) ||
		(!strcmp(argv[1], "wifi_setApBeaconInterval")) ||
		(!strcmp(argv[1], "wifi_setRadioExtChannel")) ||
		(!strcmp(argv[1], "wifi_addApAclDevice")) ||
		(!strcmp(argv[1], "wifi_setBandSteeringBandUtilizationThreshold")) ||
		(!strcmp(argv[1], "wifi_setSSIDEnable")) ||
		(!strcmp(argv[1], "wifi_setApSecurityModeEnabled")) ||
		(!strcmp(argv[1], "wifi_setApSecurityKeyPassphrase")) ||
		(!strcmp(argv[1], "wifi_setApIsolationEnable")) ||
		(!strcmp(argv[1], "wifi_setApRadioIndex")) ||
		(!strcmp(argv[1], "wifi_setBandSteeringRSSIThreshold")) ||
		(!strcmp(argv[1], "wifi_setBandSteeringPhyRateThreshold")) ||
		(!strcmp(argv[1], "wifi_setApSecurityMFPConfig")) ||
		(!strcmp(argv[1], "wifi_delApAclDevice")) ||
		(!strcmp(argv[1], "wifi_setRadioDfsRefreshPeriod")) ||
		(!strcmp(argv[1], "wifi_setRadioDfsEnable")) ||
#if WIFI_HAL_VERSION_GE_2_15 && (!defined(_CBR_PRODUCT_REQ_) && \
	!defined(_XF3_PRODUCT_REQ_))
		(!strcmp(argv[1], "wifi_setRadioDfsMoveBackEnable")) ||
		(!strcmp(argv[1], "wifi_setBSSColorEnabled")) ||
#endif /* (WIFI_HAL_VERSION_GE_2_15) && \
	(!defined(_CBR_PRODUCT_REQ_) && !defined(_XF3_PRODUCT_REQ_)) */
#if defined(WIFI_HAL_VERSION_3)
		(!strcmp(argv[1], "wifi_setBSSColor")) ||
#endif /* End of WIFI_HAL_VERSION_3 */
#if WIFI_HAL_VERSION_GE_2_12 || defined(WIFI_HAL_VERSION_3)
		(!strcmp(argv[1], "wifi_setInterworkingAccessNetworkType")) ||
		(!strcmp(argv[1], "wifi_setApInterworkingServiceEnable")) ||
#endif /* WIFI_HAL_MINOR_VERSION  >= 12 || defined(WIFI_HAL_VERSION_3) */
#if WIFI_HAL_VERSION_GE_2_15 || defined(WIFI_HAL_VERSION_3)
		(!strcmp(argv[1], "wifi_setDownlinkMuType")) ||
		(!strcmp(argv[1], "wifi_setUplinkMuType")) ||
#endif /* WIFI_HAL_VERSION_GE_2_15 */
		(!strcmp(argv[1], "wifi_setRadioDCSEnable")) ||
		(!strcmp(argv[1], "wifi_setRadioBeaconPeriod")) ||
		(!strcmp(argv[1], "wifi_setApRetryLimit")) ||
		(!strcmp(argv[1], "wifi_setApMacAddressControlMode"))) {
		wifi_api_info_t apiInfo;
		strncpy(apiInfo.api_name, argv[1], 1024);
		strncpy(apiInfo.api_data, argv[3], 1024);
		apiInfo.radioIndex = index;
		ret = wifi_api_send_msg(&apiInfo);

	} else if (!strcmp(argv[1], "wifi_getApDeviceRSSI")) {
		INT output_RSSI;
		ret = wifi_getApDeviceRSSI(index, argv[3], &output_RSSI);
		if (ret == RETURN_OK) {
			printf("%d\n", output_RSSI);
		}
	} else if (!strcmp(argv[1], "wifi_getApDeviceRxrate")) {
		INT output_RxMb;
		ret = wifi_getApDeviceRxrate(index, argv[3], &output_RxMb);
		if (ret == RETURN_OK) {
			printf("%d\n", output_RxMb);
		}
	} else if (!strcmp(argv[1], "wifi_getApDeviceTxrate")) {
		INT output_TxMb;

		ret = wifi_getApDeviceTxrate(index, argv[3], &output_TxMb);
		if (ret == RETURN_OK) {
			printf("%d\n", output_TxMb);
		}
	} else if (!strcmp(argv[1], "wifi_getRadioDcsChannelMetrics")) {
		wifi_channelMetrics_t * ptr, *channelMetrics_array_0;
		wifi_channelMetrics_t *channelMetrics_array_1;
		unsigned int array_size, i, j, num_channels = 0;
		char *ptr_bssid;

		channelMetrics_array_0 = (wifi_channelMetrics_t *)
						malloc(sizeof(wifi_channelMetrics_t) * CHCOUNT2);
		if (channelMetrics_array_0 == NULL) {
			printf("memory alloc falied for channelMetrics_array_0\n");
			return -1;
		}
		channelMetrics_array_1 = (wifi_channelMetrics_t *)
						malloc(sizeof(wifi_channelMetrics_t) * CHCOUNT5);
		if (channelMetrics_array_1 == NULL) {
			printf("memory alloc falied for channelMetrics_array_1\n");
			free(channelMetrics_array_0);
			return -1;
		}

		if (index == 0) {
			ptr = channelMetrics_array_0;
			memset(channelMetrics_array_0, 0, sizeof(wifi_channelMetrics_t) * CHCOUNT2);
			array_size = CHCOUNT2;
		} else {
			ptr = channelMetrics_array_1;
			memset(channelMetrics_array_1, 0, sizeof(wifi_channelMetrics_t) * CHCOUNT5);
			array_size = CHCOUNT5;
		}
		for (i = 0, j = 3; (i < array_size) && argv[j]; i++, j++) {
			ptr[i].channel_in_pool = TRUE;
			ptr[i].channel_number = atoi(argv[j]);
			++num_channels;
		}
		ret = wifi_getRadioDcsChannelMetrics(index, ptr, array_size);
		if (ret == RETURN_ERR) {
			printf("%s returned error\n", argv[1]);
			printUsage(argv[1], 0, FALSE);
			free(channelMetrics_array_0);
			free(channelMetrics_array_1);
			return ret;
		}
		for (i = 0; i < num_channels; i++, ptr++) {
			printf("\n*** DcsChannelMetrics result for channel %d ***\n",
				ptr->channel_number);
			printf("Channel in pool		:%s\n",
				((ptr->channel_in_pool) == 0) ? "FALSE" : "TRUE");
			printf("Channel Noise		:%d\n", ptr->channel_noise);
			printf("Channel Radar Noise	:%d\n", ptr->channel_radar_noise);
			printf("Average non 802.11 Noise:%d\n", ptr->channel_non_80211_noise);
			printf("Channel Utilization	:%d\n", ptr->channel_utilization);
			printf("Channel Tx Power	:%d\n", ptr->channel_txpower);
			printf("Rssi list of the Neighbouring AP on this channel\n");
			for (j = 0; j < ptr->channel_rssi_count; j++) {
				ptr_bssid = ptr->channel_rssi_list[j].ap_BSSID;
				printf("\tIndex:%d BSSID:%02x:%02x:%02x:%02x:%02x:%02x Channel width:%d RSSI:%d\n",
					j, (unsigned char)ptr_bssid[0], (unsigned char)ptr_bssid[1],
					(unsigned char)ptr_bssid[2], (unsigned char)ptr_bssid[3],
					(unsigned char)ptr_bssid[4], (unsigned char)ptr_bssid[5],
					ptr->channel_rssi_list[j].ap_channelWidth,
					ptr->channel_rssi_list[j].ap_rssi);
			}
			printf("Channel RSSI Count	:%d\n", ptr->channel_rssi_count);
		}
		free(channelMetrics_array_0);
		free(channelMetrics_array_1);
	}  else if (!strcmp(argv[1], "wifi_setApManagementFramePowerControl")) {
		int power;
		power = atoi(argv[3]);
		ret = wifi_setApManagementFramePowerControl(index, power);
	} else if (!strcmp(argv[1], "wifi_setApBeaconRate")) {
		ret = wifi_setApBeaconRate(index, argv[3]);
		if (ret == RETURN_OK) {
			printf("%s complete\n", argv[1]);
		}
	} else if (!strcmp(argv[1], "wifi_setApBeaconType")) {
		ret = wifi_setApBeaconType(index, argv[3]);
	} else if (!strcmp(argv[1], "wifi_getApAssociatedDeviceRxTxStatsResult")) {
		wifi_associated_dev_rate_info_rx_stats_t *rx_stats_array;
		wifi_associated_dev_rate_info_tx_stats_t *tx_stats_array;
		ULLONG handle;
		UINT output_array_size;
		unsigned int i;
		mac_address_t mac_address;

		MACF_TO_MAC(argv[3], mac_address);
		ret = wifi_getApAssociatedDeviceRxStatsResult(index, &mac_address, &rx_stats_array,
			&output_array_size, &handle);
		if (ret == RETURN_ERR) {
			printf("wifi_getApAssociatedDeviceRxStatsResult returned error\n");
			if (rx_stats_array) {
				free(rx_stats_array);
			}
			return ret;
		}

		printf("handle			:%llu \n", handle);
		for (i = 0; i < output_array_size; ++i) {
			printf("\n***Rx Stats %d***\n", i);
			printf("NSS		:%d \n", rx_stats_array[i].nss);
			printf("MCS		:%d \n", rx_stats_array[i].mcs);
			printf("BW		:%d \n", rx_stats_array[i].bw);
			printf("Flags		:%llu \n", rx_stats_array[i].flags);
			printf("Bytes		:%llu \n", rx_stats_array[i].bytes);
			printf("MSDUs		:%llu \n", rx_stats_array[i].msdus);
			printf("MPDUs		:%llu \n", rx_stats_array[i].mpdus);
			printf("PPDUs		:%llu \n", rx_stats_array[i].ppdus);
			printf("Retries		:%llu \n", rx_stats_array[i].retries);
			printf("RSSI Combined	:%d \n", rx_stats_array[i].rssi_combined);
		}
		ret = wifi_getApAssociatedDeviceTxStatsResult(index, &mac_address, &tx_stats_array,
			&output_array_size, &handle);
		if (ret == RETURN_ERR) {
			printf("wifi_getApAssociatedDeviceTxStatsResult returned error");
			if (rx_stats_array) {
				free(rx_stats_array);
			}
			if (tx_stats_array) {
				free(tx_stats_array);
			}
			return ret;
		}

		printf("handle			:%llu \n", handle);
		for (i = 0; i < output_array_size; ++i) {
			printf("\n***Tx Stats %d***\n", i);
			printf("NSS		:%d \n", tx_stats_array[i].nss);
			printf("MCS		:%d \n", tx_stats_array[i].mcs);
			printf("BW		:%d \n", tx_stats_array[i].bw);
			printf("Flags		:%llu \n", tx_stats_array[i].flags);
			printf("Bytes		:%llu \n", tx_stats_array[i].bytes);
			printf("MSDUs		:%llu \n", tx_stats_array[i].msdus);
			printf("MPDUs		:%llu \n", tx_stats_array[i].mpdus);
			printf("PPDUs		:%llu \n", tx_stats_array[i].ppdus);
			printf("Retries		:%llu \n", tx_stats_array[i].retries);
			printf("Attempts	:%llu \n", tx_stats_array[i].attempts);
		}
		if (rx_stats_array) {
			free(rx_stats_array);
		}
		if (tx_stats_array) {
			free(tx_stats_array);
		}
	} else if (!strcmp(argv[1], "wifi_setApScanFilter")) {
		wifi_api_info_t apiInfo;
		strncpy(apiInfo.api_name, argv[1], 1024);
		if (argv[4] != NULL) snprintf(apiInfo.api_data, 1024, "%d %s", atoi(argv[3]),
			argv[4]);
		else snprintf(apiInfo.api_data, 1024, "%d", atoi(argv[3]));
		apiInfo.radioIndex = index;
		ret = wifi_api_send_msg(&apiInfo);
	} else if (!strcmp(argv[1], "wifi_setApCsaDeauth")) {
		ret = wifi_setApCsaDeauth(index, atoi(argv[3]));
	} else if (!strcmp(argv[1], "wifi_kickApAssociatedDevice")) {
#ifdef WIFI_HAL_VERSION_3_PHASE2
		mac_address_t mac_address;
		MACF_TO_MAC(argv[3], mac_address);
		ret = wifi_kickApAssociatedDevice(index, mac_address);
#else
		ret = wifi_kickApAssociatedDevice(index, argv[3]);
#endif /* WIFI_HAL_VERSION_3 */
	} else if (!strcmp(argv[1], "wifi_setApWmmOgAckPolicy")) {
		wifi_api_info_t apiInfo;
		strncpy(apiInfo.api_name, argv[1], 1024);
		snprintf(apiInfo.api_data, 1024, "%d %d", atoi(argv[3]), atoi(argv[4]));
		apiInfo.radioIndex = index;
		ret = wifi_api_send_msg(&apiInfo);
	} else if (!strcmp(argv[1], "wifi_setRadioDCSScanTime")) {
		wifi_api_info_t apiInfo;
		strncpy(apiInfo.api_name, argv[1], 1024);
		snprintf(apiInfo.api_data, 1024, "%d %d", atoi(argv[3]), atoi(argv[4]));
		apiInfo.radioIndex = index;
		ret = wifi_api_send_msg(&apiInfo);
	} else if (!strcmp(argv[1], "wifi_setApSecuritySecondaryRadiusServer")) {
		wifi_api_info_t apiInfo;
		strncpy(apiInfo.api_name, argv[1], 1024);
		snprintf(apiInfo.api_data, 1024, "%s %d %s", argv[3], atoi(argv[4]), argv[5]);
		apiInfo.radioIndex = index;
		ret = wifi_api_send_msg(&apiInfo);
	} else if (!strcmp(argv[1], "wifi_setApSecurityRadiusServer")) {
		wifi_api_info_t apiInfo;
		strncpy(apiInfo.api_name, argv[1], 1024);
		snprintf(apiInfo.api_data, 1024, "%s %d %s", argv[3], atoi(argv[4]), argv[5]);
		apiInfo.radioIndex = index;
		ret = wifi_api_send_msg(&apiInfo);
	} else if (!strcmp(argv[1], "wifi_createAp")) {
		wifi_api_info_t apiInfo;
		strncpy(apiInfo.api_name, argv[1], 1024);
		snprintf(apiInfo.api_data, 1024, "%d %s %d", atoi(argv[3]), argv[4],
			atoi(argv[5]));
		apiInfo.radioIndex = index;
		ret = wifi_api_send_msg(&apiInfo);
	} else if (!strcmp(argv[1], "wifi_pushRadioChannel2")) {
		wifi_api_info_t apiInfo;
		strncpy(apiInfo.api_name, argv[1], 1024);
		snprintf(apiInfo.api_data, 1024, "%d %d %d", atoi(argv[3]), atoi(argv[4]),
			atoi(argv[5]));
		apiInfo.radioIndex = index;
		ret = wifi_api_send_msg(&apiInfo);
	} else if (!strcmp(argv[1], "wifi_setRadioChannelMode")) {
		/* Deprecated from WIFI_HAL_MAJOR_VERSION >= 2  && WIFI_HAL_MINOR_VERSION  < 15 */
		wifi_api_info_t apiInfo;
		strncpy(apiInfo.api_name, argv[1], 1024);
		snprintf(apiInfo.api_data, 1024, "%s %d %d %d", argv[3], atoi(argv[4]),
			atoi(argv[5]), atoi(argv[6]));
		apiInfo.radioIndex = index;
		ret = wifi_api_send_msg(&apiInfo);
	} else if (!strcmp(argv[1], "wifi_startNeighborScan")) {
		unsigned int *list = NULL, num = 0, i = 0;
		num = atoi(argv[5]);
		list = (unsigned int *)malloc(num * sizeof(int));
		for (i = 0; i < num; ++i) {
			if (!argv[6 + i]) {
				printf("Channel list arg[%d] is null\n", 6 + i);
				return RETURN_ERR;
			}
			list[i] = atoi(argv[6 + i]);
		}
		ret = wifi_startNeighborScan(index, atoi(argv[3]), atoi(argv[4]), atoi(argv[5]),
			list);
	} else if (!strcmp(argv[1], "wifi_pushApInterworkingElement")) {
		wifi_api_info_t apiInfo;
		wifi_InterworkingElement_t infoElement;
		strncpy(apiInfo.api_name, argv[1], 1024);
		apiInfo.radioIndex = index;
		infoElement.interworkingEnabled = atoi(argv[3]);
		infoElement.accessNetworkType = atoi(argv[4]);
		infoElement.internetAvailable = atoi(argv[5]);
		infoElement.asra = atoi(argv[6]);
		infoElement.esr = atoi(argv[7]);
		infoElement.uesa = atoi(argv[8]);
		infoElement.venueOptionPresent = atoi(argv[9]);
		infoElement.venueGroup = atoi(argv[10]);
		infoElement.venueType = atoi(argv[11]);
		infoElement.hessOptionPresent = atoi(argv[12]);
		if (argv[13] != NULL) {
			strncpy(infoElement.hessid, argv[13], 18);
		} else {
			strncpy(infoElement.hessid, "", 18);
		}
		memcpy(apiInfo.api_data, &infoElement, sizeof(wifi_InterworkingElement_t));
		ret = wifi_api_send_msg(&apiInfo);
	} else if (!strcmp(argv[1], "wifi_setRadioTrafficStatsMeasure")) {
		wifi_radioTrafficStatsMeasure_t stats;
		stats.radio_RadioStatisticsMeasuringRate = atoi(argv[3]);
		stats.radio_RadioStatisticsMeasuringInterval = atoi(argv[4]);
		ret = wifi_setRadioTrafficStatsMeasure(index, &stats);
	} else if (!strcmp(argv[1], "wifi_setRadioTrafficStatsRadioStatisticsEnable")) {
		ret = wifi_setRadioTrafficStatsRadioStatisticsEnable(index, atoi(argv[3]));
	} else if (!strcmp(argv[1], "wifi_getRadioStatsReceivedSignalLevel")) {
		INT level = 0;
		ret = wifi_getRadioStatsReceivedSignalLevel(index, atoi(argv[3]), &level);
		if (ret == RETURN_OK) {
			printf("Signal level:%d\n", level);
		}
	} else if (!strcmp(argv[1], "wifi_getRadioWifiTrafficStats")) {
		wifi_radioTrafficStats_t stats;
		memset(&stats, 0, sizeof(stats));
		ret = wifi_getRadioWifiTrafficStats(index, &stats);
		if (ret == RETURN_OK) {
			printf("Errors Sent:			%lu\n", stats.wifi_ErrorsSent);
			printf("Errors Received:		%lu\n", stats.wifi_ErrorsReceived);
			printf("Discard Packets Sent:		%lu\n",
				stats.wifi_DiscardPacketsSent);
			printf("Discard Packets Received:	%lu\n",
				stats.wifi_DiscardPacketsReceived);
			printf("PCP Error Count:		%lu\n", stats.wifi_PLCPErrorCount);
			printf("FCS Error Count:		%lu\n", stats.wifi_FCSErrorCount);
			printf("Invalid MAC Count:		%lu\n", stats.wifi_InvalidMACCount);
			printf("Other Packets Received:		%lu\n", stats.wifi_PacketsOtherReceived);
			printf("Noise:				%d\n", stats.wifi_Noise);
		}
	} else if (!strcmp(argv[1], "wifi_getSSIDTrafficStats")) {
		wifi_ssidTrafficStats_t stats;
		memset(&stats, 0, sizeof(stats));
		ret = wifi_getSSIDTrafficStats(index, &stats);
		if (ret == RETURN_OK) {
			printf("Retrans count:%lu\n", stats.wifi_RetransCount);
			printf("Failed Retrans count:%lu\n", stats.wifi_FailedRetransCount);
			printf("Retry count:%lu\n", stats.wifi_RetryCount);
			printf("Multiple Retry count:%lu\n", stats.wifi_MultipleRetryCount);
			printf("ACK Failure Count:%lu\n", stats.wifi_ACKFailureCount);
			printf("Aggregated Packet Count:%lu\n", stats.wifi_AggregatedPacketCount);
		}
	} else if (!strcmp(argv[1], "wifi_setRadioStatsEnable")) {
		ret = wifi_setRadioStatsEnable(index, atoi(argv[3]));
	} else if (!strcmp(argv[1], "wifi_getRadioStatsEnable")) {
		BOOL enable = FALSE;
		ret = wifi_getRadioStatsEnable(index, &enable);
		if (ret == RETURN_OK) {
			printf("%s\n", enable ? "TRUE" : "FALSE");
		}
	} else if (!strcmp(argv[1], "wifi_startHostApd")) {
		ret = wifi_startHostApd();
	} else if (!strcmp(argv[1], "wifi_stopHostApd")) {
		ret = wifi_stopHostApd();
	} else if (!strcmp(argv[1], "wifi_getApWmmEnable")) {
		BOOL enable;
		ret = wifi_getApWmmEnable(index, &enable);
		if (ret == RETURN_OK) {
			printf("%s\n", enable? "TRUE" : "FALSE");
		}
	} else if (!strcmp(argv[1], "wifi_setLED")) {
		ret = wifi_setLED(index, atoi(argv[3]));
	} else if (!strcmp(argv[1], "wifi_getRadioAutoChannelRefreshPeriodSupported")) {
		BOOL enable;
		ret = wifi_getRadioAutoChannelRefreshPeriodSupported(index, &enable);
		if (ret == RETURN_OK) {
			printf("%s\n", enable? "TRUE" : "FALSE");
		}
	} else if (!strcmp(argv[1], "wifi_getRadioAutoChannelRefreshPeriod")) {
		ULONG period;
		ret = wifi_getRadioAutoChannelRefreshPeriod(index, &period);
		if (ret == RETURN_OK) {
			printf("AutoChannelRefreshPeriod %lu\n", period);
		}
	} else if (!strcmp(argv[1], "wifi_setRadioMCS")) {
		ret = wifi_setRadioMCS(index, atoi(argv[3]));
	} else if (!strcmp(argv[1], "wifi_getRadioMCS")) {
		int mcs;
		ret = wifi_getRadioMCS(index, &mcs);
		if (ret == RETURN_OK) {
			printf("Mcs index is %d\n", mcs);
		}
	} else if (!strcmp(argv[1], "wifi_getRadioIEEE80211hSupported")) {
		BOOL enable;
		ret = wifi_getRadioIEEE80211hSupported(index, &enable);
		if (ret == RETURN_OK) {
			printf("%s\n", enable? "TRUE" : "FALSE");
		}
	} else if (!strcmp(argv[1], "wifi_getRadioIEEE80211hEnabled")) {
		BOOL enable;
		ret = wifi_getRadioIEEE80211hEnabled(index, &enable);
		if (ret == RETURN_OK) {
			printf("%s\n", enable? "TRUE" : "FALSE");
		}
	} else if (!strcmp(argv[1], "wifi_setRadioIEEE80211hEnabled")) {
		wifi_api_info_t apiInfo;
		strncpy(apiInfo.api_name, argv[1], 1024);
		snprintf(apiInfo.api_data, 1024, "%d", atoi(argv[3]));
		apiInfo.radioIndex = index;
		ret = wifi_api_send_msg(&apiInfo);
	} else if (!strcmp(argv[1], "wifi_getRadioBeaconPeriod")) {
		uint period;
		ret = wifi_getRadioBeaconPeriod(index, &period);
		if (ret == RETURN_OK) {
			printf("Beacon Period: %d\n", period);
		}
	} else if (!strcmp(argv[1], "wifi_getSSIDMACAddress")) {
		char mac[18];
		ret = wifi_getSSIDMACAddress(index, mac);
		if (ret == RETURN_OK) {
			printf("SSIDMacAddress: %s\n", mac);
		}
	} else if (!strcmp(argv[1], "wifi_getApRtsThresholdSupported")) {
		BOOL supported;
		ret = wifi_getApRtsThresholdSupported(index, &supported);
		if (ret == RETURN_OK) {
			printf("%s\n", supported ? "TRUE" : "FALSE");
		}
	} else if (!strcmp(argv[1], "wifi_kickApAclAssociatedDevices")) {
		ret = wifi_kickApAclAssociatedDevices(index, atoi(argv[3]));
	} else if (!strcmp(argv[1], "wifi_getApRetryLimit")) {
		uint limit;
		ret = wifi_getApRetryLimit(index, &limit);
		if (ret == RETURN_OK) {
			printf("AP retry limit: %d\n", limit);
		}
	} else if (!strcmp(argv[1], "wifi_getApWMMCapability")) {
		BOOL enable;
		ret = wifi_getApWMMCapability(index, &enable);
		if (ret == RETURN_OK) {
			printf("%s\n", enable ? "Enabled" : "Disabled");
		}
	} else if (!strcmp(argv[1], "wifi_getApUAPSDCapability")) {
		BOOL enable;
		ret = wifi_getApUAPSDCapability(index, &enable);
		if (ret == RETURN_OK) {
			printf("%s\n", enable ? "Enabled" : "Disabled");
		}
	} else if (!strcmp(argv[1], "wifi_getApWmmUapsdEnable")) {
		BOOL enable;
		ret = wifi_getApWmmUapsdEnable(index, &enable);
		if (ret == RETURN_OK) {
			printf("%s\n", enable ? "Enabled" : "Disabled");
		}
	} else if (!strcmp(argv[1], "wifi_getApMaxAssociatedDevices")) {
		uint num;
		ret = wifi_getApMaxAssociatedDevices(index, &num);
		if (ret == RETURN_OK) {
			printf("Max Associated Devices: %d\n", num);
		}
	} else if (!strcmp(argv[1], "wifi_setApMaxAssociatedDevices")) {
		wifi_api_info_t apiInfo;
		strncpy(apiInfo.api_name, argv[1], 1024);
		strncpy(apiInfo.api_data, argv[3], 1024);
		apiInfo.radioIndex = index;
		ret = wifi_api_send_msg(&apiInfo);
	} else if (!strcmp(argv[1], "wifi_setApBridgeInfo")) {
		ret = wifi_setApBridgeInfo(index, argv[3], argv[4], argv[5]);
	} else if (!strcmp(argv[1], "wifi_getIndexFromName")) {
		int bss_idx;
		ret = wifi_getIndexFromName(argv[2], &bss_idx);
		if (ret == RETURN_OK) {
			printf("bssid index: %d\n", bss_idx);
		}
	} else if (!strcmp(argv[1], "wifi_initRadio")) {
		ret = wifi_initRadio(index);
	} else if (!strcmp(argv[1], "wifi_setApRtsThreshold")) {
		wifi_api_info_t apiInfo;
		strncpy(apiInfo.api_name, argv[1], 1024);
		snprintf(apiInfo.api_data, 1024, "%d", atoi(argv[3]));
		apiInfo.radioIndex = index;
		ret = wifi_api_send_msg(&apiInfo);
	} else if (!strcmp(argv[1], "wifi_getRadioDfsSupport")) {
		BOOL enable;
		ret = wifi_getRadioDfsSupport(index, &enable);
		if (ret == RETURN_OK) {
			printf("%s\n", enable ? "Supported" : "Not supported");
		}
	} else if (!strcmp(argv[1], "wifi_getRadioDfsEnable")) {
		BOOL enable;
		ret = wifi_getRadioDfsEnable(index, &enable);
		if (ret == RETURN_OK) {
			printf("%s\n", enable ? "Enabled" : "Disabled");
		}
	}
#ifdef WIFI_HAL_VERSION_GE_3_0_1
	else if (!strcmp(argv[1], "wifi_getRadioDfsAtBootUpEnable")) {
		BOOL enable;
		ret = wifi_getRadioDfsAtBootUpEnable(index, &enable);
		if (ret == RETURN_OK) {
			printf("%s\n", enable ? "Enabled" : "Disabled");
		}
	} else if (!strcmp(argv[1], "wifi_setRadioDfsAtBootUpEnable")) {
		ret = wifi_setRadioDfsAtBootUpEnable(index, atoi(argv[3]));
	}
#endif /* WIFI_HAL_VERSION_GE_3_0_1 */
	else if (!strcmp(argv[1], "wifi_setApAssociatedDevicesHighWatermarkThreshold")) {
		ret = wifi_setApAssociatedDevicesHighWatermarkThreshold(index, atoi(argv[3]));
	} else if (!strcmp(argv[1], "wifi_getApAssociatedDevicesHighWatermarkThreshold")) {
		UINT hwm_value;
		ret = wifi_getApAssociatedDevicesHighWatermarkThreshold(index, &hwm_value);
		if (ret == RETURN_OK) {
			printf("ApAssociatedDevicesHighWatermarkThreshold %d for apIndex=%d\n", hwm_value, index);
		}
	} else if (!strcmp(argv[1], "wifi_getApAssociatedDevicesHighWatermarkThresholdReached")) {
		UINT hwm_value;
		ret = wifi_getApAssociatedDevicesHighWatermarkThresholdReached(index, &hwm_value);
		if (ret == RETURN_OK) {
			printf("ApAssociatedDevicesHighWatermarkThresholdReached %d for apIndex=%d\n", hwm_value, index);
		}
	} else if (!strcmp(argv[1], "wifi_getApAssociatedDevicesHighWatermark")) {
		UINT hwm_value;
		ret = wifi_getApAssociatedDevicesHighWatermark(index, &hwm_value);
		if (ret == RETURN_OK) {
			printf("ApAssociatedDevicesHighWatermark %d for apIndex=%d\n", hwm_value, index);
		}
	} else if (!strcmp(argv[1], "wifi_getApAssociatedDevicesHighWatermarkDate")) {
		ULONG hwm_value;
		ret = wifi_getApAssociatedDevicesHighWatermarkDate(index, &hwm_value);
		if (ret == RETURN_OK) {
			printf("ApAssociatedDevicesHighWatermarkThresholdReachedDate %lu for apIndex=%d\n", hwm_value, index);
		}
#if WIFI_HAL_VERSION_GE_2_15 && (!defined(_CBR_PRODUCT_REQ_) && \
	!defined(_XF3_PRODUCT_REQ_))
	} else if (!strcmp(argv[1], "wifi_getRadioDfsMoveBackEnable")) {
		BOOL enable;
		ret = wifi_getRadioDfsMoveBackEnable(index, &enable);
		if (ret == RETURN_OK) {
			printf("%s\n", enable ? "Enabled" : "Disabled");
		}
#endif /* (WIFI_HAL_VERSION_GE_2_15) && \
	(!defined(_CBR_PRODUCT_REQ_) && !defined(_XF3_PRODUCT_REQ_)) */
#if WIFI_HAL_VERSION_GE_2_17 || defined(WIFI_HAL_VERSION_3)
	} else if (!strcmp(argv[1], "wifi_getApAssociatedClientDiagnosticResult")) {
		wifi_associated_dev3_t assocInfo, *pt = &assocInfo;
#ifdef WIFI_HAL_VERSION_3_PHASE2
		mac_address_t mac_address;
		if (sscanf(argv[3], "%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx", MAC_TO_MACF(&mac_address)) == ETHER_ADDR_LEN) {
			ret = wifi_getApAssociatedClientDiagnosticResult(index, mac_address, pt);
		} else {
			ret = RETURN_ERR;
		}
#else /* WIFI_HAL_VERSION_3_PHASE2 */
		ret = wifi_getApAssociatedClientDiagnosticResult(index, argv[3], pt);
#endif /* WIFI_HAL_VERSION_3_PHASE2 */
		if (ret == RETURN_OK) {
			printf("cli_MACAddress			:%02x:%02x:%02x:%02x:%02x:%02x\n",
				pt->cli_MACAddress[0], pt->cli_MACAddress[1],
				pt->cli_MACAddress[2], pt->cli_MACAddress[3],
				pt->cli_MACAddress[4], pt->cli_MACAddress[5]);
			printf("cli_LastDataDownlinkRate	:%d\n",
				pt->cli_LastDataDownlinkRate);
			printf("cli_LastDataUplinkRate		:%d\n",
				pt->cli_LastDataUplinkRate);
			printf("cli_SignalStrength		:%d\n", pt->cli_SignalStrength);
			printf("cli_OperatingStandard		:%s\n", pt->cli_OperatingStandard);
			printf("cli_OperatingChannelBandwidth	:%s\n",
				pt->cli_OperatingChannelBandwidth);
			printf("cli_SNR				:%d\n", pt->cli_SNR);
			printf("cli_RSSI			:%d\n", pt->cli_RSSI);
			printf("cli_AuthenticationState		:%d\n",
				pt->cli_AuthenticationState);
			printf("cli_Retransmissions		:%d\n", pt->cli_Retransmissions);
			printf("cli_Disassociations		:%d\n", pt->cli_Disassociations);
			printf("cli_AuthenticationFailures	:%d\n",
				pt->cli_AuthenticationFailures);
			printf("cli_BytesSent			:%lu\n", pt->cli_BytesSent);
			printf("cli_BytesReceived		:%lu\n", pt->cli_BytesReceived);
			printf("cli_PacketsSent			:%lu\n", pt->cli_PacketsSent);
			printf("cli_PacketsReceived		:%lu\n", pt->cli_PacketsReceived);
			printf("cli_ErrorsSent			:%lu\n", pt->cli_ErrorsSent);
			printf("cli_RetransCount		:%lu\n", pt->cli_RetransCount);
			printf("cli_FailedRetransCount		:%lu\n",
				pt->cli_FailedRetransCount);
			printf("cli_RetryCount			:%lu\n", pt->cli_RetryCount);
			printf("cli_MultipleRetryCount		:%lu\n",
				pt->cli_MultipleRetryCount);
			printf("cli_MaxDownlinkRate		:%d\n", pt->cli_MaxDownlinkRate);
			printf("cli_MaxUplinkRate		:%d\n", pt->cli_MaxUplinkRate);
		}
#endif /* #if (WIFI_HAL_VERSION_GE_2_17 */
	} else if (!strcmp(argv[1], "wifi_setClientDetailedStatisticsEnable")) {
		ret = wifi_setClientDetailedStatisticsEnable(index, atoi(argv[3]));
#if WIFI_HAL_VERSION_GE_2_16
#ifdef _XF3_PRODUCT_REQ_
	} else if (!strcmp(argv[1], "wifi_getVAPTelemetry")) {
		wifi_VAPTelemetry_t telemetry;
		ret = wifi_getVAPTelemetry(index, &telemetry);
		if (ret == RETURN_OK) {
			printf("%d\n", *(int *)&telemetry);
		}
#endif /* _XF3_PRODUCT_REQ_ */
#endif /* WIFI_HAL_VERSION_GE_2_16 */
#if WIFI_HAL_VERSION_GE_2_18 || defined(WIFI_HAL_VERSION_3)
	} else if (!strcmp(argv[1], "wifi_getRadioPercentageTransmitPower")) {
		unsigned long power;
		ret = wifi_getRadioPercentageTransmitPower(index, &power);
		if (ret == RETURN_OK) {
			printf("%ld\n", power);
		}
#endif /* WIFI_HAL_VERSION_GE_2_18 */
	} else if (!strcmp(argv[1], "wifi_getBandSteeringLog")) {
		unsigned long steering_time;
		char mac[MAC_STR_LEN];
		int source, dest, reason;
		ret = wifi_getBandSteeringLog(index, &steering_time, mac, &source, &dest, &reason);
		if (ret == RETURN_OK) {
			printf("Steering Time:%lu, MAC:%s Source SSID:%d Destination SSID:%d Reason:%d\n",
				steering_time, mac, source, dest, reason);
		}
	} else if (!strcmp(argv[1], "wifi_getNeighboringWiFiDiagnosticResult2")) {
		wifi_neighbor_ap2_t *neighbor_ap_array = NULL;
		UINT array_size = 0;
#ifdef WIFI_HAL_VERSION_3_PHASE2
		BOOL scan = atoi(argv[3]);
		ret = wifi_getNeighboringWiFiDiagnosticResult2(index, scan, &neighbor_ap_array,
			&array_size);
#else /* WIFI_HAL_VERSION_3_PHASE2 */
		ret = wifi_getNeighboringWiFiDiagnosticResult2(index, &neighbor_ap_array,
			&array_size);
#endif /* WIFI_HAL_VERSION_3_PHASE2 */
		if (ret == RETURN_OK) {
			printf("Total Scan Results:%d\n", array_size);
			print_neighbor_stats(neighbor_ap_array, array_size);
		}
		if (neighbor_ap_array) {
			free(neighbor_ap_array);
			neighbor_ap_array = NULL;
		}
	} else if (!strcmp(argv[1], "wifi_getNeighboringWiFiStatus")) {
		wifi_neighbor_ap2_t *neighbor_ap_array = NULL;
		UINT array_size = 0;
		ret = wifi_getNeighboringWiFiStatus(index, &neighbor_ap_array, &array_size);
		if (ret == RETURN_OK) {
			printf("Total Scan Results:%d\n", array_size);
			print_neighbor_stats(neighbor_ap_array, array_size);
		}
		if (neighbor_ap_array) {
			free(neighbor_ap_array);
			neighbor_ap_array = NULL;
		}
#if WIFI_HAL_VERSION_GE_2_18
	} else if (!strcmp(argv[1], "wifi_getRadioChannels")) {
		test_wifi_getRadioChannels(index);
#endif /* WIFI_HAL_VERSION_GE_2_18 */
#if WIFI_HAL_VERSION_GE_2_19 && (!defined(_CBR1_PRODUCT_REQ_) && \
	!defined(_XF3_PRODUCT_REQ_))
#ifndef _SKY_HUB_COMMON_PRODUCT_REQ_
	} else if (!strcmp(argv[1], "wifi_enableCSIEngine")) {
		mac_address_t mac = {0};
		BOOL enable = TRUE;

		parse_mac(argv[4], &mac);
		enable = !strncmp(argv[3], "enable", 6);

		ret = wifi_enableCSIEngine(index, mac, enable);

		printf("wifi_enableCSIEngine: %d\n", enable);
#ifdef CMWIFI_RDKB_COMCAST
	} else if (!strcmp(argv[1], "wifi_csitest")) {
		int n, cnt, loop, i, j, period = atoi(argv[3]);
		mac_address_t mac[MAX_CSI_STA], null_mac = {0x00};
		wifi_associated_dev3_t *assoc_buf, *ptr;
		unsigned int array_size, *raw_data;
		wifi_csi_data_t *csi_data;
		wifi_frame_info_t *frame_info;
		wifi_csi_matrix_t *csi_matrix;

		/* minumum 30sec for test run */
		if (period <= 30)
			period = 30;

		printf("total run:%d sec (at least 30sec)\n", period);
		/* register callback */
		wifi_csi_callback_register(wifi_csi_proc);

		/* Enable CSI */
		printf("Enable CSI\n");
		for (n = 0; n < MAX_CSI_STA && (n + 4) < argc; n++) {
			parse_mac(argv[n + 4], &mac[n]);

			printf("mac:"MACF"\n", MAC_TO_MACF(mac[n]));
			ret = wifi_enableCSIEngine(index, mac[n], 1);
			if (ret == RETURN_ERR) {
				printf("%s: csi enable failure\n", __FUNCTION__);
				return -1;
			}
		}

		/* wait (period) sec, to allow csi data collection by callback wifi_csi_proc*/
		sleep(period);
		printf("csi recv cnt:%u\n", csi_recv);

		/* Disable CSI and exit */
		ret = wifi_enableCSIEngine(index, null_mac, 0);
		printf("disable CSI and Exit");
#endif /* CMWIFI_RDKB_COMCAST */
#endif /* _SKY_HUB_COMMON_PRODUCT_REQ_ */
	} else if (!strcmp(argv[1], "wifi_enableGreylistAccessControl")) {
		wifi_api_info_t apiInfo;

		strncpy(apiInfo.api_name, argv[1], sizeof(apiInfo.api_name));
		strncpy(apiInfo.api_data, argv[2], sizeof(apiInfo.api_data));
		wifi_api_send_msg(&apiInfo);
	} else if (!strcmp(argv[1], "wifi_getApDASRadiusServer")) {
		char IPAddr[64], secret[64];
		unsigned int port;

		ret = wifi_getApDASRadiusServer(index, IPAddr, &port, secret);
		if (ret == RETURN_OK) {
			printf("DAS client IP Address:%s\n", IPAddr);
			printf("DAS Port:%d\n", port);
			printf("DAS Secret:%s\n", secret);
		}
	} else if (!strcmp(argv[1], "wifi_setApDASRadiusServer")) {
		wifi_api_info_t apiInfo;

		apiInfo.radioIndex = index;
		strncpy(apiInfo.api_name, argv[1], sizeof(apiInfo.api_name));
		snprintf(apiInfo.api_data, sizeof(apiInfo.api_data), "%s %s %s",
				argv[3], argv[4], argv[5]);
		ret = wifi_api_send_msg(&apiInfo);
#endif /* WIFI_HAL_VERSION_GE_2_19 && !_CBR1_PRODUCT_REQ_ && !_XF3_PRODUCT_REQ_ */
#if !defined(_CBR1_PRODUCT_REQ_) && !defined(_XF3_PRODUCT_REQ_)
	} else if (!strcmp(argv[1], "wifi_allow2G80211ax")) {
		wifi_api_info_t apiInfo;
		strncpy(apiInfo.api_name, argv[1], sizeof(apiInfo.api_name));
		snprintf(apiInfo.api_data, sizeof(apiInfo.api_data), "%s", argv[2]);
		ret = wifi_api_send_msg(&apiInfo);
	} else if (!strcmp(argv[1], "wifi_getAllow2G80211ax")) {
		bool enable;
		ret = wifi_getAllow2G80211ax(&enable);
		if (ret != RETURN_OK) {
			printf("%s returned error %d\n", argv[1], ret);
			printUsage(argv[1], 0, FALSE);
		}
		printf("%d\n", enable);
#endif /* !_CBR1_PRODUCT_REQ_ && !_XF3_PRODUCT_REQ_ */
#if defined(WIFI_HAL_VERSION_3)
	} else if (!strcmp(argv[1], "wifi_getHalCapability")) {
		wifi_hal_capability_t *cap;
		unsigned int i, radios_cnt;

		cap = (wifi_hal_capability_t *) malloc(sizeof(wifi_hal_capability_t));

		if (cap == NULL) {
			printf("malloc failed for wifi_hal_capability_t \n");
			return -1;
		}
		memset(cap, 0, sizeof(wifi_hal_capability_t));
		ret = wifi_getHalCapability(cap);
		if (ret != RETURN_OK) {
			printf("%s returned error %d\n", argv[1], ret);
			printUsage(argv[1], 0, FALSE);
		}
		printf("\nHal Capability:");
		printf("\n   version major: %d minor: %d",
			cap->version.major, cap->version.minor);
		printf("\n   BandSteeringSupported: %s",
			cap->BandSteeringSupported ? "TRUE" : "FALSE");
		radios_cnt = cap->wifi_prop.numRadios;
		printf("\n   numRadios: %d", radios_cnt);
		if (radios_cnt > MAX_NUM_RADIOS) {
			printf("\n Error numRadios=%d MAX_NUM_RADIOS=%d\n",
				radios_cnt, MAX_NUM_RADIOS);
		}
		for (i = 0; i < radios_cnt; i++) {
			printf("\n   Radio %d Capabilities:", i);
			print_RadioCapabilities(&(cap->wifi_prop.radiocap[i]));
		}
#if defined(WIFI_HAL_VERSION_GE_3_0_1)
		wifi_interface_name_idex_map_t *imap;
		printf("\n   Wifi Interface Map:");
		printf("\n   phy_index rdk_radio_index interface_name bridge_name");
		printf(" vlan_id index vap_name");
		for (i = 0; i < (radios_cnt * MAX_NUM_VAP_PER_RADIO); i++) {
			imap = &(cap->wifi_prop.interface_map[i]);
			if (imap->rdk_radio_index < radios_cnt) {
				printf("\n%7d \t%d %16s %16s %8d %8d \t%s",
					imap->phy_index, imap->rdk_radio_index,
					imap->interface_name, imap->bridge_name,
					imap->vlan_id, imap->index, imap->vap_name);
			}
		}
#if defined(RDKB_LGI)
		radio_interface_mapping_t *rmap;
		printf("\n   Radio Interface Map:");
		printf("\n   phy_index  radio_index  radio_name  interface_name");
		for (i = 0; i < radios_cnt; i++) {
			rmap = &(cap->wifi_prop.radio_interface_map[i]);
			printf("\n%7d \t%d %16s %12s", rmap->phy_index,
				rmap->radio_index, rmap->radio_name, rmap->interface_name);
		} /* for */
#endif /* RDKB_LGI */
		printf("\n..........");
#endif /* WIFI_HAL_VERSION_GE_3_0_1 */
#ifdef WIFI_HAL_VERSION_GE_3_0_3
		printf("\n   mu_bands: 0x%x", cap->wifi_prop.mu_bands);
#endif /* WIFI_HAL_VERSION_GE_3_0_3 */
		printf("\n");
		free(cap);
	} else if (!strcmp(argv[1], "wifi_getAPCapabilities")) {
		wifi_ap_capabilities_t *cap;

		cap = (wifi_ap_capabilities_t*) malloc(sizeof(wifi_ap_capabilities_t));

		if (cap == NULL) {
			printf("malloc failed for wifi_hal_capability_t \n");
			return -1;
		}
                memset(cap, 0, sizeof(wifi_ap_capabilities_t));
                ret = wifi_getAPCapabilities(index, cap);
                if (ret == RETURN_OK) {
                        printf("\nAP %d Capabilities:", index);
			printf("\n\trtsThresholdSupported: %s",
				(cap->rtsThresholdSupported) ? "TRUE" : "FALSE");
			printf("\n\t");
			print_security_modes(cap->securityModesSupported);
			printf("\n\t");
			print_onBoardingMethods(cap->methodsSupported);
			printf("\n\tWMMSupported: %s", (cap->WMMSupported ? "TRUE" : "FALSE"));
			printf("\n\tUAPSDSupported: %s", (cap->UAPSDSupported ? "TRUE" : "FALSE"));
			printf("\n\tinterworkingServiceSupported: %s",
				(cap->interworkingServiceSupported ? "TRUE" : "FALSE"));
			printf("\n\tBSSTransitionImplemented: %s",
				(cap->BSSTransitionImplemented ? "TRUE" : "FALSE"));
			printf("\n");
		}
		free(cap);
	} else if (!strcmp(argv[1], "wifi_getApWpsConfiguration")) {
		wifi_wps_t wpsConfig;

		ret = wifi_getApWpsConfiguration(index, &wpsConfig);
		if (ret == WIFI_HAL_SUCCESS) {
			int i;

			printf("WPS mode: %s\n", (wpsConfig.enable ? "Enabled" : "Disabled"));

			if (!wpsConfig.enable)
				return 0;

			printf("WPS device PIN: %s\n", wpsConfig.pin);
			printf("WPS enabled configuration methods:\n");
			for (i = 0; wps_config_method_table[i].str_val != NULL; ++i) {
				if (wpsConfig.methods & wps_config_method_table[i].enum_val)
					printf("\t%s\n", wps_config_method_table[i].str_val);

			}
		}
	} else if (!strcmp(argv[1], "wifi_setApWpsConfiguration")) {
		wifi_wps_t wpsConfig;
		wifi_api_info_t apiInfo;
		unsigned int i, j;

		wpsConfig.enable = atoi(argv[3]);
		if (wpsConfig.enable) {
			if (argc < 6)
				goto print_setApWpsConfiguration_usage;

			snprintf(wpsConfig.pin, sizeof(wpsConfig.pin), argv[4]);
			wpsConfig.methods = 0;
			for (i = 5; i < (unsigned int)argc; ++i) {
				j = 0;
				while (TRUE) {
					if (wps_config_method_table[j].str_val == NULL) {
						printf("unsupported method: %s\n", argv[i]);
						goto print_setApWpsConfiguration_usage;
					}

					if (!strcmp(argv[i], wps_config_method_table[j].str_val)) {
						wpsConfig.methods |= wps_config_method_table[j].enum_val;
						break;
					}

					++j;
				}
			}
		} else {
			if (argc != 4)
				goto print_setApWpsConfiguration_usage;
		}
		strncpy(apiInfo.api_name, argv[1], sizeof(apiInfo.api_name));
		memcpy(apiInfo.api_data, &wpsConfig, sizeof(wpsConfig)); //sizeof(wpsConfig) < sizeof(apiInfo.api_data)
		apiInfo.radioIndex = index;
		wifi_api_send_msg(&apiInfo);
		return 0;

print_setApWpsConfiguration_usage:
		printf("Usage: %s %s apIndex enable [wps_device_pin wps_enabled_config_methods]\n\n"
			"wps_enabled_config_methods are combination of below methods:\n"
			"\tLabel Display PushButton Keypad PhysicalPushButton "
			"PhysicalDisplay VirtualPushButton VirtualDisplay\n"
			"Example:\n"
			"\t%s %s apIndex 0\n"
			"\t%s %s apIndex 1 37471675 Label PushButton\n",
			argv[0], argv[1], argv[0], argv[1], argv[0], argv[1]);
		return -1;
	} else if (!strcmp(argv[1], "wifi_getRadioFrequencyBand")) {
		wifi_freq_bands_t band;
		ret = wifi_getRadioFrequencyBand(index, &band);
		if (ret == RETURN_OK) {
			print_bands(band);
		}
	} else if (!strcmp(argv[1], "wifi_getRadioOperatingParameters")) {
		wifi_radio_operationParam_t operationParam;
		ret = wifi_getRadioOperatingParameters(index, &operationParam);
		if (ret == RETURN_OK) {
			printf("radio Enable: %d\n", operationParam.enable);
			print_bands(operationParam.band);
			printf("autoChannelEnabled: %d\n", operationParam.autoChannelEnabled);
			printf("channel: %d\n", operationParam.channel);
			printf("numSecondaryChannels: %d\n", operationParam.numSecondaryChannels);
			printf("channelSecondary:\n");
			for (unsigned int i = 0; i < operationParam.numSecondaryChannels; i++) {
				printf("channelSecondary[%d]=%d ", i, operationParam.channelSecondary[i]);
			}
			printf("\n");
			print_chBandwidths(operationParam.channelWidth);
			print_80211_variants(operationParam.variant);
			printf("csa_beacon_count: %d\n", operationParam.csa_beacon_count);
			print_countryCode(operationParam.countryCode);
			printf("DCSEnabled: %d\n", operationParam.DCSEnabled);
			printf("dtimPeriod: %d\n", operationParam.dtimPeriod);
			printf("beaconInterval: %d\n", operationParam.beaconInterval);
			printf("operatingClass: %d\n", operationParam.operatingClass);
			print_bitrates(operationParam.basicDataTransmitRates);
			print_bitrates(operationParam.operationalDataTransmitRates);
			printf("fragmentationThreshold: %d\n", operationParam.fragmentationThreshold);
			printf("guardInterval: %d\n", operationParam.guardInterval);
			printf("transmitPower: %d\n", operationParam.transmitPower);
			printf("rtsThreshold: %d\n", operationParam.rtsThreshold);
		}
	} else if (!strcmp(argv[1], "wifi_setRadioOperatingParameters")) {
		wifi_api_info_t apiInfo;
		wifi_radio_operationParam_t operationParam;
		strncpy(apiInfo.api_name, argv[1], 1024);
		apiInfo.radioIndex = index;
		operationParam.autoChannelEnabled = atoi(argv[3]);
		operationParam.channel = atoi(argv[4]);
		operationParam.channelWidth = atoi(argv[5]);
		operationParam.variant = atoi(argv[6]);
		operationParam.csa_beacon_count = atoi(argv[7]);
		memcpy(apiInfo.api_data, &operationParam, sizeof(wifi_radio_operationParam_t));
		ret = wifi_api_send_msg(&apiInfo);
	} else if (!strcmp(argv[1], "wifi_getApSecurity")) {
		wifi_vap_security_t security;
#if defined(WIFI_HAL_VERSION_3_PHASE2)
		char ip_txt[INET6_ADDRSTRLEN] = {'\0'};
#endif /* WIFI_HAL_VERSION_3_PHASE2 */
		unsigned int i;

		ret = wifi_getApSecurity(index, &security);
		if (ret == WIFI_HAL_SUCCESS) {
			for (i = 0; security_mode_table[i].str_val != NULL; ++i) {
				if (security_mode_table[i].enum_val == security.mode)
					break;
			}
			/* If ret is SUCCESS, security mode must be valid */
			printf("security mode: %s\n", security_mode_table[i].str_val);
			printf("mfp: %d\n", security.mfp);

			if (security.mode == wifi_security_mode_none)
				return 0;

			for (i = 0; encryption_table[i].str_val != NULL; ++i) {
				if (encryption_table[i].enum_val == security.encr)
					break;
			}
			/* If ret is SUCCESS, encryption must be valid */
			printf("encryption: %s\n", encryption_table[i].str_val);

			switch (security.mode) {
				case wifi_security_mode_wep_64:
				case wifi_security_mode_wep_128: //not supported
					break;

				case wifi_security_mode_wpa_personal:
				case wifi_security_mode_wpa2_personal:
				case wifi_security_mode_wpa3_personal:
				case wifi_security_mode_wpa_wpa2_personal:
				case wifi_security_mode_wpa3_transition:
					if (security.u.key.type == wifi_security_key_type_psk) {
						printf("wpa psk: 0x%s\n", security.u.key.key);
					} else {
						printf("wpa passphrase: %s\n", security.u.key.key);
					}

					if (security.mode == wifi_security_mode_wpa3_transition) {
						printf("wpa3 transition: %s\n",
							security.wpa3_transition_disable ? "Disabled" : "Enabled");
					}
					break;

				case wifi_security_mode_wpa_enterprise:
				case wifi_security_mode_wpa2_enterprise:
				case wifi_security_mode_wpa3_enterprise:
				case wifi_security_mode_wpa_wpa2_enterprise:
#if defined(WIFI_HAL_VERSION_3_PHASE2)
					if (security.u.radius.ip.family == wifi_ip_family_ipv4) {
						inet_ntop(AF_INET, &security.u.radius.ip.u.IPv4addr,
							ip_txt, sizeof(ip_txt));
					} else if (security.u.radius.ip.family == wifi_ip_family_ipv6) {
						inet_ntop(AF_INET6, security.u.radius.ip.u.IPv6addr,
							ip_txt, sizeof(ip_txt));
					} else {
						printf("unrecognized addr family\n");
						return -1;
					}
					printf("primary RADIUS auth server addr: %s\n",
						ip_txt);
#else /* WIFI_HAL_VERSION_3_PHASE2 */
					printf("primary RADIUS auth server addr: %s\n",
						security.u.radius.ip);
#endif /* WIFI_HAL_VERSION_3_PHASE2 */
					printf("primary RADIUS auth server port: %u\n",
						security.u.radius.port);
					printf("primary RADIUS auth server secret: %s\n",
						security.u.radius.key);

#if defined(WIFI_HAL_VERSION_3_PHASE2)
					if (security.u.radius.s_ip.family == wifi_ip_family_ipv4) {
						inet_ntop(AF_INET, &security.u.radius.s_ip.u.IPv4addr,
							ip_txt, sizeof(ip_txt));
					} else if (security.u.radius.s_ip.family == wifi_ip_family_ipv6) {
						inet_ntop(AF_INET6, security.u.radius.s_ip.u.IPv6addr,
							ip_txt, sizeof(ip_txt));
					} else {
						printf("unrecognized addr family\n");
						return -1;
					}
					printf("secondary RADIUS auth server ip addr: %s\n",
						ip_txt);
#else /* WIFI_HAL_VERSION_3_PHASE2 */
					printf("secondary RADIUS auth server ip addr: %s\n",
						security.u.radius.s_ip);
#endif /* WIFI_HAL_VERSION_3_PHASE2 */
					printf("secondary RADIUS auth server port: %u\n",
						security.u.radius.s_port);
					printf("secondary RADIUS auth server secret: %s\n",
						security.u.radius.s_key);
					break;

				default:
					break;
			}
		}
	} else if (!strcmp(argv[1], "wifi_setApSecurity")) {
		wifi_vap_security_t security = {0};
		wifi_api_info_t apiInfo;
		int i;

		for (i = 0; security_mode_table[i].str_val != NULL; ++i) {
			if (!strcmp(security_mode_table[i].str_val, argv[3])) {
				security.mode = security_mode_table[i].enum_val;
				break;
			}
		}

		if (security_mode_table[i].str_val == NULL) {
			printf("%s %s: wrong security mode [%s]\n", argv[0], argv[1], argv[3]);
			goto print_setApSecurity_usage;
		}

		switch (security.mode) {
			case wifi_security_mode_none:
				security.mfp = atoi(argv[4]);
				if (argc != 5) {
					printf("%s %s: wrong argument number\n", argv[0], argv[1]);
					goto print_setApSecurity_usage;
				}
				break;
			case wifi_security_mode_wep_64:
			case wifi_security_mode_wep_128: //not supported
				break;
			case wifi_security_mode_wpa_personal:
			case wifi_security_mode_wpa2_personal:
			case wifi_security_mode_wpa3_personal:
			case wifi_security_mode_wpa_wpa2_personal:
			case wifi_security_mode_wpa3_transition:
				if (!((security.mode == wifi_security_mode_wpa3_transition && argc == 9)
					|| (security.mode != wifi_security_mode_wpa3_transition && argc == 8))) {
					printf("%s %s: wrong argument number\n", argv[0], argv[1]);
					goto print_setApSecurity_usage;
				}
				security.mfp = atoi(argv[4]);
				if (!strcmp(argv[5], "tkip")) {
					security.encr = wifi_encryption_tkip;
				} else if (!strcmp(argv[5], "aes")) {
					security.encr = wifi_encryption_aes;
				} else if (!strcmp(argv[5], "tkip+aes")) {
					security.encr = wifi_encryption_aes_tkip;
				} else {
					printf("%s %s: wrong encrytion [%s]\n", argv[0], argv[1], argv[5]);
					goto print_setApSecurity_usage;
				}

				if (!strcmp(argv[6], "psk")) {
					security.u.key.type = wifi_security_key_type_psk;
				} else if (!strcmp(argv[6], "passphrase")) {
					security.u.key.type = wifi_security_key_type_pass;
				} else {
					printf("%s %s: wrong key type [%s]\n", argv[0], argv[1], argv[6]);
					goto print_setApSecurity_usage;
				}
				snprintf(security.u.key.key, sizeof(security.u.key.key), argv[7]);
				if (security.mode == wifi_security_mode_wpa3_transition) {
					security.wpa3_transition_disable = atoi(argv[8]);
				}
				break;
			case wifi_security_mode_wpa_enterprise:
			case wifi_security_mode_wpa2_enterprise:
			case wifi_security_mode_wpa3_enterprise:
			case wifi_security_mode_wpa_wpa2_enterprise:
				if (argc != 9 && argc != 12) {
					printf("%s %s: wrong argument number [%d]\n", argv[0], argv[1], argc);
					goto print_setApSecurity_usage;
				}

				/* mfp */
				security.mfp = atoi(argv[4]);
				/* crypto */
				if (!strcmp(argv[5], "tkip")) {
					security.encr = wifi_encryption_tkip;
				} else if (!strcmp(argv[5], "aes")) {
					security.encr = wifi_encryption_aes;
				} else if (!strcmp(argv[5], "tkip+aes")) {
					security.encr = wifi_encryption_aes_tkip;
				} else {
					printf("%s %s: wrong primary encrytion [%s]\n", argv[0], argv[1], argv[5]);
					goto print_setApSecurity_usage;
				}

#if defined(WIFI_HAL_VERSION_3_PHASE2)
				/* primary radius auth server */
				if (inet_pton(AF_INET, argv[6], &security.u.radius.ip.u.IPv4addr) > 0) {
					security.u.radius.ip.family = wifi_ip_family_ipv4;
				} else if (inet_pton(AF_INET6, argv[6], security.u.radius.ip.u.IPv6addr) > 0) {
					security.u.radius.ip.family = wifi_ip_family_ipv6;
				} else {
					printf("%s %s: wrong ip [%s]\n", argv[0], argv[1], argv[6]);
					goto print_setApSecurity_usage;
				}
#else /* WIFI_HAL_VERSION_3_PHASE2 */
				snprintf((char*)security.u.radius.ip, sizeof(security.u.radius.ip), argv[6]);
#endif /* WIFI_HAL_VERSION_3_PHASE2 */

				security.u.radius.port = atoi(argv[7]);
				snprintf(security.u.radius.key, sizeof(security.u.radius.key), argv[8]);

				/* seceondary radius auth server */
				if (argc > 9) {
#if defined(WIFI_HAL_VERSION_3_PHASE2)
					if (inet_pton(AF_INET, argv[9], &security.u.radius.s_ip.u.IPv4addr) > 0) {
						security.u.radius.s_ip.family = wifi_ip_family_ipv4;
					} else if (inet_pton(AF_INET6, argv[9], security.u.radius.s_ip.u.IPv6addr) > 0) {
						security.u.radius.s_ip.family = wifi_ip_family_ipv6;
					} else {
						printf("%s %s: wrong ip [%s]\n", argv[0], argv[1], argv[9]);
						goto print_setApSecurity_usage;
					}
#else /* WIFI_HAL_VERSION_3_PHASE2 */
				snprintf((char *)security.u.radius.s_ip, sizeof(security.u.radius.s_ip), argv[9]);
#endif /* WIFI_HAL_VERSION_3_PHASE2 */

					security.u.radius.s_port = atoi(argv[10]);
					snprintf(security.u.radius.s_key, sizeof(security.u.radius.s_key), argv[11]);
				}
				break;
			default: //no way get here
				printf("%s %s: opps, no way get here\n", argv[0], argv[1]);
				goto print_setApSecurity_usage;
		}

		strncpy(apiInfo.api_name, argv[1], sizeof(apiInfo.api_name));
		memcpy(apiInfo.api_data, &security, sizeof(security)); //sizeof(security) < sizeof(apiInfo.api_data)
		apiInfo.radioIndex = index;
		wifi_api_send_msg(&apiInfo);
		return 0;

print_setApSecurity_usage:
		printf("Usage: %s %s apIndex security-mode mfp-value<0..2> [other-security-mode-dependent-parameters]\n\n"
			"security-mode:\n"
			"\tNone |\n"
			"\tWPA-Personal |\n"
			"\tWPA2-Personal |\n"
			"\tWPA3-Personal |\n"
			"\tWPA-WPA2-Personal |\n"
			"\tWPA3-Personal-Transition |\n"
			"\tWPA-Enterprise |\n"
			"\tWPA2-Enterprise |\n"
			"\tWPA3-Enterprise |\n"
			"\tWPA-WPA2-Enterprise\n\n"
			"other-security-mode-dependent-parameters:\n"
			"\tWPA|WPA2|WPA3 PERSONAL:\n"
			"\t\tencryption key-type key\n"
			"\tWPA|WPA2|WPA3 ENTERPRISE:\n"
			"\t\tencryption primary-radius-auth-server-params<ip, port, key> [secondary-radius-auth-server-params<ip, port, key>]\n\n"
			"encryption: tkip | aes | tkip+aes\n"
			"key-type: psk | passphrase\n", argv[0], argv[1]);
		return 0;
	} else if (!strcmp(argv[1], "wifi_getAvailableBSSColor")) {
		UCHAR colorList[63]; /* WL_COLOR_MAX_VALUE=63 from wlioctl.h */
		int numColorReturned = 0, i;
		ret = wifi_getAvailableBSSColor(index, sizeof(colorList), colorList, &numColorReturned);
		if (ret == WIFI_HAL_SUCCESS) {
			if (numColorReturned > 0) {
				printf("Available BSSColor List =\n");
				for (i = 0; i < numColorReturned; i++) {
					printf("%d ", colorList[i]);
				}
				printf("\n");
			} else {
				printf("Available BSSColor List is Empty\n");
			}
		}
	} else if (!strcmp(argv[1], "wifi_getMuEdca")) {
		wifi_edca_t edca;
		wifi_access_category_t ac = atoi(argv[3]);
		ret = wifi_getMuEdca(index, ac, &edca);
		if (ret == WIFI_HAL_SUCCESS) {
			printf("MuEdca aci=%d, aifsn=%d, cw_min=%d, cw_max=%d, timer=%d\n",
				ac, edca.aifsn, edca.cw_min, edca.cw_max, edca.timer);
		}
	} else if (!strcmp(argv[1], "wifi_setMuEdca")) {
		if (argv[7] != NULL) {
			wifi_api_info_t apiInfo;
			strncpy(apiInfo.api_name, argv[1], sizeof(apiInfo.api_name));
			/* pass channelMode, pureMode */
			snprintf(apiInfo.api_data, sizeof(apiInfo.api_data), "%d %d %d %d %d",
				atoi(argv[3]), atoi(argv[4]), atoi(argv[5]), atoi(argv[6]),
				atoi(argv[7]));
			apiInfo.radioIndex = index;
			ret = wifi_api_send_msg(&apiInfo);
		}
	} else if (!strcmp(argv[1], "wifi_getRadioVapInfoMap")) {
		wifi_vap_info_map_t  *map;
		wifi_vap_security_t *security;
		wifi_interworking_t *interworking;
#if defined(WIFI_HAL_VERSION_3_PHASE2)
		char ip_txt[INET6_ADDRSTRLEN] = {'\0'};
#endif /* WIFI_HAL_VERSION_3_PHASE2 */
		unsigned int i;
		int k, ret;

		map = (wifi_vap_info_map_t *) malloc(sizeof(wifi_vap_info_map_t));
		if (map == NULL) {
			printf("Could not allocate memory for map for API wifi_getRadioVapInfoMap");
			return -1;
		}

		memset(map, 0, sizeof(wifi_vap_info_map_t));
		ret = wifi_getRadioVapInfoMap(index, map);
		if (ret == WIFI_HAL_SUCCESS) {
			for (i = 0; i < map->num_vaps; i++) {
				security = &(map->vap_array[i].u.bss_info.security);
				interworking = &(map->vap_array[i].u.bss_info.interworking);

				printf("ap_index=%d, ap_name=%s, ssid=%s, "MACF"\n",
					map->vap_array[i].vap_index,
					map->vap_array[i].vap_name,
					map->vap_array[i].u.bss_info.ssid,
					MAC_TO_MACF(map->vap_array[i].u.bss_info.bssid));

				printf("\tenabled=%d, showSsid=%d, isolation=%d\n"
					"\tmgmtPowerControl=%d, bssMaxSta=%d, bssTransitionActivated=%d\n"
					"\tnbrReportActivated=%d, mac_filter_enable=%d\n"
					"\tmac_filter_mode=%d, wmm_enabled=%d\n",
					map->vap_array[i].u.bss_info.enabled,
					map->vap_array[i].u.bss_info.showSsid,
					map->vap_array[i].u.bss_info.isolation,
					map->vap_array[i].u.bss_info.mgmtPowerControl,
					map->vap_array[i].u.bss_info.bssMaxSta,
					map->vap_array[i].u.bss_info.bssTransitionActivated,
					map->vap_array[i].u.bss_info.nbrReportActivated,
					map->vap_array[i].u.bss_info.mac_filter_enable,
					map->vap_array[i].u.bss_info.mac_filter_mode,
					map->vap_array[i].u.bss_info.wmm_enabled);

				/* print security params */
				for (k = 0; security_mode_table[k].str_val != NULL; ++k) {
					if (security_mode_table[k].enum_val == security->mode)
						break;
				}
				/* If ret is SUCCESS, security mode must be valid */
				printf("\tsecurity mode: %s\n", security_mode_table[k].str_val);
				printf("mfp: %d\n", security->mfp);
				if (security->mode != wifi_security_mode_none) {
					for (k = 0; encryption_table[k].str_val != NULL; ++k) {
						if (encryption_table[k].enum_val == security->encr)
							break;
					}
					printf("\tencryption: %s\n", encryption_table[k].str_val);

					switch (security->mode) {
						case wifi_security_mode_wep_64:
						case wifi_security_mode_wep_128: //not supported
							break;

						case wifi_security_mode_wpa_personal:
						case wifi_security_mode_wpa2_personal:
						case wifi_security_mode_wpa3_personal:
						case wifi_security_mode_wpa_wpa2_personal:
						case wifi_security_mode_wpa3_transition:
							if (security->u.key.type ==
								wifi_security_key_type_psk) {
								printf("\twpa psk: 0x%s\n",
									security->u.key.key);
							} else {
								printf("\twpa passphrase: %s\n",
									security->u.key.key);
							}
							break;

						case wifi_security_mode_wpa_enterprise:
						case wifi_security_mode_wpa2_enterprise:
						case wifi_security_mode_wpa3_enterprise:
						case wifi_security_mode_wpa_wpa2_enterprise:
#if defined(WIFI_HAL_VERSION_3_PHASE2)
							if (security->u.radius.ip.family ==
								wifi_ip_family_ipv4) {
								inet_ntop(AF_INET,
									&(security->u.radius.ip.u.IPv4addr),
									ip_txt, sizeof(ip_txt));
							} else if (security->u.radius.ip.family ==
								wifi_ip_family_ipv6) {
								inet_ntop(AF_INET6,
									security->u.radius.ip.u.IPv6addr,
									ip_txt, sizeof(ip_txt));
							} else {
								printf("\tunrecognized addr family\n");
							}
							printf("\tprimary RADIUS auth server addr: %s\n",
								ip_txt);
#else /* WIFI_HAL_VERSION_3_PHASE2 */
							printf("\tprimary RADIUS auth server addr: %s\n",
								security->u.radius.ip);
#endif /* WIFI_HAL_VERSION_3_PHASE2 */
							printf("\tprimary RADIUS auth server port: %u\n",
								security->u.radius.port);
							printf("\tprimary RADIUS auth server secret: %s\n",
								security->u.radius.key);

#if defined(WIFI_HAL_VERSION_3_PHASE2)
							if (security->u.radius.s_ip.family ==
								wifi_ip_family_ipv4) {
								inet_ntop(AF_INET,
									&(security->u.radius.s_ip.u.IPv4addr),
									ip_txt, sizeof(ip_txt));
							} else if (security->u.radius.s_ip.family ==
								wifi_ip_family_ipv6) {
								inet_ntop(AF_INET6,
									security->u.radius.s_ip.u.IPv6addr,
									ip_txt, sizeof(ip_txt));
							} else {
								printf("\tunrecognized addr family\n");
							}
							printf("\tsecondary RADIUS auth server ip addr: %s\n",
								ip_txt);
#else /* WIFI_HAL_VERSION_3_PHASE2 */
							printf("\tsecondary RADIUS auth server ip addr: %s\n",
								security->u.radius.s_ip);
#endif /* WIFI_HAL_VERSION_3_PHASE2 */
							printf("\tsecondary RADIUS auth server port: %u\n",
								security->u.radius.s_port);
							printf("\tsecondary RADIUS auth server secret: %s\n",
								security->u.radius.s_key);
							break;

						default:
							break;
					}
				}

				/* Interworking Element */
				wifi_InterworkingElement_t output_struct;
				memcpy(&output_struct, &(interworking->interworking), sizeof(wifi_InterworkingElement_t));

				printf("\tInterworkingElement: \n");
				printf("\tinterworkingEnabled:	%d\n", output_struct.interworkingEnabled);
				printf("\taccessNetworkType:	%d\n", output_struct.accessNetworkType);
				printf("\tinternetAvailable:	%d\n", output_struct.internetAvailable);
				printf("\tasra:			%d\n", output_struct.asra);
				printf("\tesr:			%d\n", output_struct.esr);
				printf("\tuesa:			%d\n", output_struct.uesa);
				printf("\tvenueOptionPresent:	%d\n", output_struct.venueOptionPresent);
				printf("\tvenueGroup:		%d\n", output_struct.venueGroup);
				printf("\tvenueType:		%d\n", output_struct.venueType);
				printf("\thessOptionPresent:	%d\n", output_struct.hessOptionPresent);
				printf("\thessid:			%s\n", output_struct.hessid);

				/* RoamingConsortium */
				wifi_roamingConsortiumElement_t *roamingConsortium;
				roamingConsortium = &(interworking->roamingConsortium);

				printf("\tRoamingConsortium: \n");
				printf("\troamingConsortium count=%d\n", roamingConsortium->wifiRoamingConsortiumCount);

				/* passpoint */
				wifi_passpoint_settings_t *passpoint;
				passpoint = &(interworking->passpoint);

				printf("\tpasspoint: \n");
				printf("\tenable:			%d\n", passpoint->enable);
				printf("\tgafDisable:		%d\n", passpoint->gafDisable);
				printf("\tp2pDisable:		%d\n", passpoint->p2pDisable);
				printf("\tl2tif:			%d\n", passpoint->l2tif);
				printf("\tbssLoad:		%d\n", passpoint->bssLoad);
				printf("\tcountryIE:		%d\n", passpoint->countryIE);
				printf("\tproxyArp:		%d\n", passpoint->proxyArp);

				/* WPS */
				printf("\tWPS mode: %s\n",
					(map->vap_array[i].u.bss_info.wps.enable ? "Enabled" : "Disabled"));

				if (map->vap_array[i].u.bss_info.wps.enable) {
					printf("\tWPS device PIN: %s\n",
						map->vap_array[i].u.bss_info.wps.pin);
					printf("\tWPS enabled configuration methods:\n");
					for (k = 0; wps_config_method_table[k].str_val != NULL; ++k) {
						if (map->vap_array[i].u.bss_info.wps.methods &
							wps_config_method_table[k].enum_val) {
							printf("\t\t%s\n", wps_config_method_table[k].str_val);
						}
					}
				}

				/* UAPSD */
				printf("\tWMM UAPSD: \n");
				printf("\tUAPSD enabled=%s\n",
					map->vap_array[i].u.bss_info.UAPSDEnabled ? "Enabled" : "Disabled");
			}
		}
		free(map);
	} else if (!strcmp(argv[1], "wifi_createVAP")) {
		wifi_api_info_t apiInfo;

		if ((atoi(argv[3]) != 1) && (atoi(argv[3]) != 2)) {
			printf("Usage %s %s <radioIndex> <1/2>\n", argv[0], argv[1]);
			return 0;
		}
		strncpy(apiInfo.api_name, argv[1], sizeof(apiInfo.api_name));
		snprintf(apiInfo.api_data, 1024, "%s", argv[3]);
		apiInfo.radioIndex = index;
		wifi_api_send_msg(&apiInfo);
		return 0;
	} else if (!strcmp(argv[1], "wifi_getEAP_Param")) {
		wifi_eap_config_t eap_config;
		memset(&eap_config, 0, sizeof(eap_config));
		ret = wifi_getEAP_Param(index, &eap_config);

		if (ret == WIFI_HAL_SUCCESS) {
			printf("EAPOL Key Timeout: %u\n", eap_config.uiEAPOLKeyTimeout);
			printf("EAPOL Key Retries: %u\n", eap_config.uiEAPOLKeyRetries);
			printf("EAP Identity Request Timeout: %u\n", eap_config.uiEAPIdentityRequestTimeout);
			printf("EAP Identity Request Retries: %u\n", eap_config.uiEAPIdentityRequestRetries);
			printf("EAP Request Timeout: %u\n", eap_config.uiEAPRequestTimeout);
			printf("EAP Request Retries: %u\n", eap_config.uiEAPRequestRetries);
		} else {
			printf("Unable to get the info\n");
		}
	} else if (!strcmp(argv[1], "wifi_setEAP_Param")) {
		wifi_api_info_t apiInfo;

		strncpy(apiInfo.api_name, argv[1], 1024);
		snprintf(apiInfo.api_data, 1024, "%s %s", argv[3], argv[4]);
		apiInfo.radioIndex = index;
		ret = wifi_api_send_msg(&apiInfo);
	} else if (!strcmp(argv[1], "wifi_getTWTsessions")) {
		wifi_twt_sessions_t *twtSess;
		unsigned int maxSess, numSess = 0;
		maxSess = atoi(argv[3]);
		twtSess = (wifi_twt_sessions_t *) malloc(maxSess * sizeof(wifi_twt_sessions_t));
		if (twtSess != NULL) {
			ret = wifi_getTWTsessions(index, maxSess, twtSess, &numSess);
			if (ret == RETURN_OK) {
				print_TWTsessions(twtSess, numSess);
			}
			free(twtSess);
		}
		else {
			printf("Error Null twtSess\n");
		}
	} else if (!strcmp(argv[1], "wifi_setBroadcastTWTSchedule")) {
		wifi_twt_params_t tparams = {0};
		int sessionID = 0;
		BOOL create;

		tparams.agreement = wifi_twt_agreement_type_broadcast;
		tparams.operation.implicit = atoi(argv[3]);
		tparams.operation.announced = atoi(argv[4]);
		tparams.operation.trigger_enabled = atoi(argv[5]);
		tparams.params.broadcast.wakeDuration_uSec= atoi(argv[6]);
		tparams.params.broadcast.wakeInterval_uSec= atoi(argv[7]);
		create = atoi(argv[8]);
		if (argv[9])
			sessionID = atoi(argv[9]);
		ret = wifi_setBroadcastTWTSchedule(index, tparams, create, &sessionID);
		if (!ret) {
			printf("sessionID = %d\n", sessionID);
		}
		else {
			printf("Error wifi_setBroadcastTWTSchedule\n");
		}
	} else if (!strcmp(argv[1], "wifi_setTeardownTWTSession")) {
		int sessionID = atoi(argv[3]);
		ret = wifi_setTeardownTWTSession(index, sessionID);
#endif /* WIFI_HAL_VERSION_3 */
#ifdef RDKB_LGI
	} else if (!strcmp(argv[1], "wifi_getRADIUSAcctEnable")) {
		BOOL enable;
		ret = wifi_getRADIUSAcctEnable(index, &enable);
		if (ret == RETURN_OK) {
			printf("RADIUS accounting %s\n", enable ? "enabled" : "disabled");
		}
	} else if (!strcmp(argv[1], "wifi_setRADIUSAcctEnable")) {
		wifi_api_info_t apiInfo;
		strncpy(apiInfo.api_name, argv[1], 1024);
		snprintf(apiInfo.api_data, 1024, "%s", argv[3]);
		apiInfo.radioIndex = index;
		ret = wifi_api_send_msg(&apiInfo);
	} else if (!strcmp(argv[1], "wifi_getApSecurityAcctServer")) {
		CHAR IP[64], secret[64];
		UINT port;
		ret = wifi_getApSecurityAcctServer(index, IP, &port, secret);
		if (ret == RETURN_OK) {
			printf("Accounting Server IP: %s\n", IP);
			printf("Accounting Server Port: %d\n", port);
			printf("Accounting Server Secret: %s\n", secret);
		}
	} else if (!strcmp(argv[1], "wifi_setApSecurityAcctServer")) {
		wifi_api_info_t apiInfo;
		strncpy(apiInfo.api_name, argv[1], 1024);
		snprintf(apiInfo.api_data, 1024, "%s %s %s", argv[3], argv[4], argv[5]);
		apiInfo.radioIndex = index;
		ret = wifi_api_send_msg(&apiInfo);
	} else if (!strcmp(argv[1], "wifi_getApSecuritySecondaryAcctServer")) {
		CHAR IP[64], secret[64];
		UINT port;
		ret = wifi_getApSecuritySecondaryAcctServer(index, IP, &port, secret);
		if (ret == RETURN_OK) {
			printf("Secondary Accounting Server IP: %s\n", IP);
			printf("Secondary Accounting Server Port: %d\n", port);
			printf("Seoncdary Accounting Server Secret: %s\n", secret);
		}
	} else if (!strcmp(argv[1], "wifi_setApSecuritySecondaryAcctServer")) {
		wifi_api_info_t apiInfo;
		strncpy(apiInfo.api_name, argv[1], 1024);
		snprintf(apiInfo.api_data, 1024, "%s %s %s", argv[3], argv[4], argv[5]);
		apiInfo.radioIndex = index;
		ret = wifi_api_send_msg(&apiInfo);
	} else if (!strcmp(argv[1], "wifi_getApSecurityAcctInterimInterval")) {
		UINT interval;
		ret = wifi_getApSecurityAcctInterimInterval(index, &interval);
		if (ret == RETURN_OK) {
			printf("RADIUS accounting interim interval: %d\n", interval);
		}
	} else if (!strcmp(argv[1], "wifi_setApSecurityAcctInterimInterval")) {
		wifi_api_info_t apiInfo;
		strncpy(apiInfo.api_name, argv[1], 1024);
		snprintf(apiInfo.api_data, 1024, "%s", argv[3]);
		apiInfo.radioIndex = index;
		ret = wifi_api_send_msg(&apiInfo);
	} else if (!strcmp(argv[1], "wifi_setApRadiusTransportInterface")) {
		wifi_api_info_t apiInfo;

		strncpy(apiInfo.api_name, argv[1], sizeof(apiInfo.api_name));
		strncpy(apiInfo.api_data, argv[2], sizeof(apiInfo.api_data));
		ret = wifi_api_send_msg(&apiInfo);
	} else if (!strcmp(argv[1], "wifi_getApRadiusReAuthInterval")) {
		UINT interval;

		ret = wifi_getApRadiusReAuthInterval(index, &interval);
		if (ret == RETURN_OK) {
			printf("RADIUS reauth interval: %u\n", interval);
		}
	} else if (!strcmp(argv[1], "wifi_setApRadiusReAuthInterval")) {
		wifi_api_info_t apiInfo;

		strncpy(apiInfo.api_name, argv[1], sizeof(apiInfo.api_name));
		strncpy(apiInfo.api_data, argv[3], sizeof(apiInfo.api_data));
		apiInfo.radioIndex = index;
		ret = wifi_api_send_msg(&apiInfo);
	} else if (!strcmp(argv[1], "wifi_getRadiusOperatorName")) {
		CHAR op_name[33]; /* max len is 32, plus one for the NUL at the end */

		ret = wifi_getRadiusOperatorName(index, op_name);
		if (ret == RETURN_OK) {
			printf("RADIUS operator name: %s\n", op_name);
		}
	} else if (!strcmp(argv[1], "wifi_setRadiusOperatorName")) {
		wifi_api_info_t apiInfo;

		strncpy(apiInfo.api_name, argv[1], sizeof(apiInfo.api_name));
		strncpy(apiInfo.api_data, argv[3], sizeof(apiInfo.api_data));
		apiInfo.radioIndex = index;
		ret = wifi_api_send_msg(&apiInfo);
	} else if (!strcmp(argv[1], "wifi_getRadiusLocationData")) {
		CHAR loc_data[254]; /* max len is 253, plus one for the NUL at the end */

		ret = wifi_getRadiusLocationData(index, loc_data);
		if (ret == RETURN_OK) {
			printf("RADIUS location data: %s\n", loc_data);
		}
	} else if (!strcmp(argv[1], "wifi_setRadiusLocationData")) {
		wifi_api_info_t apiInfo;

		strncpy(apiInfo.api_name, argv[1], sizeof(apiInfo.api_name));
		strncpy(apiInfo.api_data, argv[3], sizeof(apiInfo.api_data));
		apiInfo.radioIndex = index;
		ret = wifi_api_send_msg(&apiInfo);
	} else if (!strcmp(argv[1], "wifi_getApPMKCacheInterval")) {
		UINT interval;

		ret = wifi_getApPMKCacheInterval(index, &interval);
		if (ret == RETURN_OK) {
			printf("PMK Cache Interval: %d\n", interval);
		}
	} else if (!strcmp(argv[1], "wifi_setApPMKCacheInterval")) {
		wifi_api_info_t apiInfo;

		strncpy(apiInfo.api_name, argv[1], sizeof(apiInfo.api_name));
		strncpy(apiInfo.api_data, argv[3], sizeof(apiInfo.api_data));
		apiInfo.radioIndex = index;
		ret = wifi_api_send_msg(&apiInfo);
	} else if (!strcmp(argv[1], "wifi_getWpsStatus")) {
		char wpsStatus[64];

		ret = wifi_getWpsStatus(index, wpsStatus);
		if (ret == RETURN_OK) {
			printf("WPS status: %s\n", wpsStatus);
		}
	} else if (!strcmp(argv[1], "wifi_getRadioConfiguredChannel")) {
		ULONG channel;
		ret = wifi_getRadioConfiguredChannel(index, &channel);
		if (ret == RETURN_OK) {
			printf("wifi_getRadioConfiguredChannel: %lu\n", channel);
		}
	} else if (!strcmp(argv[1], "wifi_getRadioRunningChannel")) {
		ULONG channel;
		ret = wifi_getRadioRunningChannel(index, &channel);
		if (ret == RETURN_OK) {
			printf("wifi_getRadioRunningChannel: %lu\n", channel);
		}
	} else if (!strcmp(argv[1], "wifi_getSoftBlockEnable")) {
		BOOL enable;
		ret = wifi_getSoftBlockEnable(&enable);
		if (ret == RETURN_OK) {
			printf("SoftBlock %s\n", enable ? "enabled" : "disabled");
		}
	} else if (!strcmp(argv[1], "wifi_setSoftBlockEnable")) {
		BOOL enable = atoi(argv[2]) ? TRUE : FALSE;
		ret = wifi_setSoftBlockEnable(enable);
	} else if (!strcmp(argv[1], "wifi_clearSoftBlockBlacklist")) {
		ret = wifi_clearSoftBlockBlacklist();
	} else if (!strcmp(argv[1], "wifi_getSoftBlockBlacklistEntries")) {
		UINT num, i;
		wifi_softblock_mac_table_t *softblock_table = NULL, *tbl_ptr;
		ret = wifi_getSoftBlockBlacklistEntries(index, (ULONG *)&num, &softblock_table);
		if (ret == RETURN_OK) {
			printf("SoftBlock number=%d\n", num);
			if (softblock_table) {
				for (i = 0; i < num; i++) {
					tbl_ptr = &softblock_table[i];
					printf("i=%d time=%s mac=%s\n", i, tbl_ptr->time, tbl_ptr->mac);
				}
				/* the buffer is allocated in API, must be freed by caller */
				free(softblock_table);
			}
		}
	} else if (!strcmp(argv[1], "wifi_getSupportRatesBitmapControlFeature")) {
		BOOL enable;
		ret = wifi_getSupportRatesBitmapControlFeature(&enable);
		if (ret == RETURN_OK) {
			printf("wifi_getSupportRatesBitmapControlFeature %s\n", enable ? "enabled" : "disabled");
		}
	} else if (!strcmp(argv[1], "wifi_setSupportRatesBitmapControlFeature")) {
		wifi_api_info_t apiInfo;
		strncpy(apiInfo.api_name, argv[1], sizeof(apiInfo.api_name));
		snprintf(apiInfo.api_data, sizeof(apiInfo.api_data), "%s", argv[2]);
		ret = wifi_api_send_msg(&apiInfo);
	} else if (!strcmp(argv[1], "wifi_getSupportRatesDisableBasicRates")) {
		char basicratemap[33];
		ret = wifi_getSupportRatesDisableBasicRates(index, basicratemap);
		if (ret == RETURN_OK) {
			printf("wifi_getSupportRatesDisableBasicRates: %s\n", basicratemap);
		}
	} else if (!strcmp(argv[1], "wifi_setSupportRatesDisableBasicRates")) {
		wifi_api_info_t apiInfo;
		strncpy(apiInfo.api_name, argv[1], sizeof(apiInfo.api_name));
		snprintf(apiInfo.api_data, sizeof(apiInfo.api_data), "%s", argv[3]);
		apiInfo.radioIndex = index;
		ret = wifi_api_send_msg(&apiInfo);
	} else if (!strcmp(argv[1], "wifi_getSupportRatesDisableSupportedRates")) {
		char supportedratemap[33];
		ret = wifi_getSupportRatesDisableSupportedRates(index, supportedratemap);
		if (ret == RETURN_OK) {
			printf("wifi_getSupportRatesDisableSupportedRates: %s\n", supportedratemap);
		}
	} else if (!strcmp(argv[1], "wifi_setSupportRatesDisableSupportedRates")) {
		wifi_api_info_t apiInfo;
		strncpy(apiInfo.api_name, argv[1], sizeof(apiInfo.api_name));
		snprintf(apiInfo.api_data, sizeof(apiInfo.api_data), "%s", argv[3]);
		apiInfo.radioIndex = index;
		ret = wifi_api_send_msg(&apiInfo);
	} else if (!strcmp(argv[1], "wifi_getApWmmOgAckPolicy")) {
		BOOL enable;
		ret = wifi_getApWmmOgAckPolicy(index, &enable);
		if (ret == RETURN_OK) {
			printf("wifi_getApWmmOgAckPolicy %s\n", enable ? "enabled" : "disabled");
		}
	} else if (!strcmp(argv[1], "wifi_api_is_device_associated")) {
		BOOL assoc;
		assoc = wifi_api_is_device_associated(index, argv[3]);
		printf("wifi_api_is_device_associated: %s\n", assoc ? "Associated" : "Not associated");
	} else if (!strcmp(argv[1], "wifi_kickAllAssociatedDevice")) {
		ret = wifi_kickAllAssociatedDevice(index);
	} else if (!strcmp(argv[1], "wifi_getRadioExcludeDfs")) {
		BOOL enable;
		ret = wifi_getRadioExcludeDfs(index, &enable);
		if (ret == RETURN_OK) {
			printf("%s\n", enable ? "Enabled" : "Disabled");
		}
	} else if (!strcmp(argv[1], "wifi_setRadioExcludeDfs")) {
		wifi_api_info_t apiInfo;
		strncpy(apiInfo.api_name, argv[1], 1024);
		snprintf(apiInfo.api_data, 1024, "%s", argv[3]);
		apiInfo.radioIndex = index;
		ret = wifi_api_send_msg(&apiInfo);
	} else if (!strcmp(argv[1], "wifi_getRadioChannelWeights")) {
		/* Per LGI, assume
		 *	radio 0 channels:
		 *		1, 6, 11
		 *	radio 1 channels:
		 *		36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112,
		 *		116, 120, 124, 128, 132, 136, 140, 144, 149, 153, 157, 161,
		 *		165, 169, 173, 177
		 */
		ULONG output_weights[MAX_CH_LIST_LEN] = {0};
		int i;
		ret = wifi_getRadioChannelWeights(index, output_weights);
		if (ret == RETURN_OK) {
			for (i = 0; i < MAX_CH_LIST_LEN; i++) {
				if (index == 0 && i == 3) {
					break;
				} else if (index == 1 && i == 28) {
					break;
				} else {
					printf("%d ", output_weights[i]);
				}
			}
			printf("\n");
		} else {
			printf("No output_weights\n");
		}
	} else if (!strcmp(argv[1], "wifi_setRadioChannelWeights")) {
		/* Per LGI, assume
		 *	radio 0 channels:
		 *		1, 6, 11
		 *	radio 1 channels:
		 *		36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112,
		 *		116, 120, 124, 128, 132, 136, 140, 144, 149, 153, 157, 161,
		 *		165, 169, 173, 177
		 */
		ULONG output_weights_0[3] = {100,100,100};
		ULONG output_weights_1[28] = {100,100,100,100,100,100,100,100,100,100,
						 100,100,100,100,100,100,100,100,100,100,
						 100,100,100,100,100,100,100,100};
		ULONG *output_weights = (index == 0) ? output_weights_0 :
					(index == 1) ? output_weights_1 : NULL;
		int i;

		if (index != 0 && index != 1) {
			printf("invalid radioIndex:%d\n", index);
		} else if ((index == 0 && argc > 6) ||
			(index == 1 && argc > 31)) {
			printf("invalid number of channel weights:%d radioIndex:%d\n", argc - 3, index);
		} else {
			/* List of channel weights start at argv[3], replace default values above */
			for (i = 0; i < (argc - 3); i++) {
				output_weights[i] = atoi(argv[3 + i]);
			}
			ret = wifi_setRadioChannelWeights(index, output_weights);
		}
	} else if (!strcmp(argv[1], "wifi_isZeroDFSSupported")) {
		BOOL supported;
		ret = wifi_isZeroDFSSupported(index, &supported);
		if (ret == RETURN_OK) {
			printf("%s\n", supported ? "TRUE" : "FALSE");
		}
	} else if (!strcmp(argv[1], "wifi_setZeroDFSState")) {
		BOOL enable = (atoi(argv[3])) ? TRUE : FALSE;
		BOOL precac = (atoi(argv[4])) ? TRUE : FALSE;
		ret = wifi_setZeroDFSState(index, enable, precac);
	} else if (!strcmp(argv[1], "wifi_getZeroDFSState")) {
		BOOL enable = FALSE, precac = FALSE;
		ret = wifi_getZeroDFSState(index, &enable, &precac);
		if (ret == RETURN_OK) {
			printf("enable:%s precac:%s\n", enable ? "1" : "0",
				precac ? "1" : "0");
		}
	} else if (!strcmp(argv[1], "wifi_getZeroWaitDFSChannelsStatus")) {
		wifi_zwdf_list_t *ListOfDFSChannelStatus = NULL;
		ret = wifi_getZeroWaitDFSChannelsStatus(index , &ListOfDFSChannelStatus);
		if (ret == RETURN_OK && ListOfDFSChannelStatus != NULL) {
			int i = 0;
			WIFI_ZWDFS_CHAN_STATUS_ENUM status;
			while (ListOfDFSChannelStatus[i].Channel != 0) {
				status = ListOfDFSChannelStatus[i].Status;
				printf("index=%d \tChannel=%lu \tStatus=%s\n", i,
					ListOfDFSChannelStatus[i].Channel,
					(status == AVAILABLE ? "AVAILABLE" :
						status == CAC_ONGOING ? "CAC_ONGOING" :
						status == NON_OCCUPANCY_PERIOD ? "NON_OCCUPANCY_PERIOD" :
						status == NOT_CLEARED ? "NOT_CLEARED" :
						"INVALID_STATUS"));
				i++;
			}
			printf("Number of DFS channels = %d\n", i);
			free(ListOfDFSChannelStatus);
			ListOfDFSChannelStatus = NULL;
		} else {
			printf("No valid DFS channels\n");
		}
	} else if (!strcmp(argv[1], "wifi_getCurrentRadioOperatingChannelBandwidth")) {
		char bandwidth[64];
		ret = wifi_getCurrentRadioOperatingChannelBandwidth(index, bandwidth,
			sizeof(bandwidth));
		if (ret == RETURN_OK) {
			printf("%s\n", bandwidth);
		}
#endif /* RDKB_LGI */
	} else if (!strcmp(argv[1], "wifi_setATMEnable")) {
		/* index = atoi(argv[2], which is disable/enable argument */
		ret = wifi_setATMEnable(index);
	} else if (!strcmp(argv[1], "wifi_getATMEnable")) {
		BOOL enable;
		ret = wifi_getATMEnable(&enable);
		if (ret == RETURN_OK) {
			printf("%s\n", enable ? "Enabled" : "Disabled");
		}
	} else if (!strcmp(argv[1], "wifi_setApATMAirTimePercent")) {
		ret = wifi_setApATMAirTimePercent(index, atoi(argv[3]));
	} else if (!strcmp(argv[1], "wifi_getApATMAirTimePercent")) {
		unsigned int apATMAirTimePercent;
		ret = wifi_getApATMAirTimePercent(index, &apATMAirTimePercent);
		if (ret == RETURN_OK) {
			printf("%d\n", apATMAirTimePercent);
		}
	} else if (!strcmp(argv[1], "wifi_getApATMSta")) {
		UCHAR output_sta_MAC_ATM_array[MAX_OUTPUT_STRING_LEN_1024];
		UINT buf_size = MAX_OUTPUT_STRING_LEN_1024;
		memset(output_sta_MAC_ATM_array, 0, MAX_OUTPUT_STRING_LEN_1024);
		ret = wifi_getApATMSta(index, output_sta_MAC_ATM_array, buf_size);
		if (ret == RETURN_OK) {
			printf("%s\n", output_sta_MAC_ATM_array);
		}
#if !defined(WIFI_HAL_VERSION_GE_3_0)
	} else if (!strcmp(argv[1], "wifi_setApATMSta")) {
		ret = wifi_setApATMSta(index, (UCHAR *) argv[3], atoi(argv[4]));
#endif
	}

	if (ret == RETURN_ERR) {
		printf("%s returned error\n", argv[1]);
		printUsage(argv[1], 0, FALSE);
	}
	return ret;
}
