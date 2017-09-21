/*##################################################################################################
# "Copyright (c) 2013 Intel Corporation                                                            #
# DISTRIBUTABLE AS SAMPLE SOURCE SOFTWARE                                                          #
# This Distributable As Sample Source Software is subject to the terms and conditions              #
# of the Intel Software License Agreement for the Intel(R) Cable and GW Software Development Kit"  #
##################################################################################################*/

#ifndef __FAPI_WLAN_BEEROCK_AML_H_
#define __FAPI_WLAN_BEEROCK_AML_H_

#include <stdio.h>
#include <stdlib.h>

#include "help_proto.h"
#include "help_defs.h"

#include "help_objlist.h"
#include "fapi_wlan.h"
#include "fapi_wlan_private.h"

/** \defgroup FAPI_WLAN_COMMON_BEEROCK_AML FAPI Beerock interfaces
*   @ingroup FAPI_WLAN
\brief It provides the set of API to support the Intel Beerock implementation (AP manager).
*/

/** \addtogroup FAPI_WLAN_COMMON_BEEROCK_AML */
/* @{ */

// AP Manager Lite
int ap_manager_lite_cellular_config_set(const char *ifname, char *MBOCellAware, char *CellPref);
int ap_manager_lite_bss_neighbor_set(const char *ifname, char *nr_BSSID, char *opClass, char *channelNumber, char *priority, char *oce, char *tbtt_offset);
int ap_manager_lite_btm_params_set(const char *ifname,
								   char *bssTermination,
								   char *disassocImminent,  /* Optional Param - can be 'NULL' */
                                   char *btmReqTermBit,     /* Optional Param - can be 'NULL' */
								   char *disassocTimer,     /* Optional Param - can be 'NULL' */
								   char *reassocDelay,      /* Optional Param - can be 'NULL' */
								   char *bssTermTSF);       /* Optional Param - can be 'NULL' */
int ap_manager_lite_association_disallow(const char *ifname, char *reassocCode);  /* BSSID will be get internally */
int ap_manager_lite_disassociate_set(const char *ifname, char *MACAddress);
int ap_manager_lite_btm_req(const char *ifname,
							char *MACAddress,
							char *dialog_token,
							char *pref,
							char *reason,
							char *disassocTimer,  /* Optional Param - can be 'NULL' */
							char *abridged,       /* Optional Param - can be 'NULL' */
							char *valid_int);     /* Optional Param - can be 'NULL' */
int ap_manager_lite_beacon_req_get(const char *ifname,
                                   char *MACAddress,
								   char *numOfRepetitions,
								   char *durationMandatory,
								   char *opClass,
								   char *Channel,
								   char *randInt,
								   char *duration,
								   char *mode,
								   char *bssid,
								   char *ssid,          /* Optional Param - can be 'NULL' */
								   char *repDetail,     /* Optional Param - can be 'NULL' */
								   char *repCond,       /* Optional Param - can be 'NULL' */
								   char *apChReport,    /* Optional Param - can be 'NULL' */
								   char *reqElements);  /* Optional Param - can be 'NULL' */
void beerock_aml_cli(char *ifname, char **argv, int argc);
int ap_manager_lite_band_steering_init(void);
int fapiWlanRefCallBackFunc(const char *ifname, char *buf);
int fapiWlanCallBackBandSteeringFunc(char *opCode, const char *ifname, ObjList *wlObj, unsigned int flags, void *context);
int ap_manager_lite_band_steering_perform(int signalStrengthThreshold_2_4, int signalStrengthThreshold_5, int intervalInSeconds, int toleranceInSeconds, int numOfTicksAllowedForSteering);
int fapiWlanCallBackApManagerLiteFunc(char *opCode, const char *ifname, ObjList *wlObj, unsigned int flags, void *context);

#endif  //__FAPI_WLAN_BEEROCK_AML_H_
