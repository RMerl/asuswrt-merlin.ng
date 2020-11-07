/*
 * WPS app utilies
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: $
 */

#include <stdio.h>
#include <string.h>
#include <wpstypes.h>
#include <tutrace.h>
#include <wps_wps.h>
#include <wps_apputils.h>
#include <wlif_utils.h>

/*
 * PR65690. Let customer have a chance to modify credential
 */
int (*wpsapp_update_custom_cred_callback)(char *, char *, char *,
	char *, int, bool, uint8);

int
wpsapp_utils_update_custom_cred(char *ssid, char *key, char *akm, char *crypto, int oob_addenr,
	bool b_wps_Version2)
{
	/*
	 * WSC 1.0 Old design : Because WSC 1.0 did not support Mix mode, in default
	 * we pick WPA2-PSK/AES up in Mix mode.  If NVRAM "wps_mixedmode" is "1"
	 * than change to pick WPA-PSK/TKIP up.
	 */

	/*
	 * WSC 2.0 support Mix mode and says "WPA-PSK and TKIP only allowed in
	 * Mix mode".  So in default we use Mix mode and if  NVRMA "wps_mixedmode"
	 * is "1" than change to pick WPA2-PSK/AES up.
	 */
	int mix_mode = 2;

	if (!strcmp(wps_safe_get_conf("wps_mixedmode"), "1"))
		mix_mode = 1;

	if (!strcmp(akm, "WPA-PSK WPA2-PSK")) {
		if (b_wps_Version2) {
				strcpy(akm, "WPA2-PSK");
		} else {
			if (mix_mode == 1)
				strcpy(akm, "WPA-PSK");
			else
				strcpy(akm, "WPA2-PSK");
		}
		TUTRACE((TUTRACE_INFO, "Update customized Key Mode : %s, ", akm));
	}

	if (!strcmp(crypto, "AES+TKIP")) {
		if (b_wps_Version2) {
				strcpy(crypto, "AES");
		} else {
			if (mix_mode == 1)
				strcpy(crypto, "TKIP");
			else
				strcpy(crypto, "AES");
		}
		TUTRACE((TUTRACE_INFO, "Update customized encrypt mode = %s\n", crypto));
	}

	if (oob_addenr) {
		char *p;

		/* get randomssid */
		if ((p = wps_get_conf("wps_randomssid"))) {
			strncpy(ssid, p, MAX_SSID_LEN);
			TUTRACE((TUTRACE_INFO, "Update customized SSID : %s\n", ssid));
		}

		/* get randomkey */
		if ((p = wps_get_conf("wps_randomkey")) && (strlen (p) >= 8)) {
			strncpy(key, p, SIZE_64_BYTES);
			TUTRACE((TUTRACE_INFO, "Update customized Key : %s\n", key));
		}

		/* Modify the crypto, if the station is legacy */
	}

	return 0;
}
