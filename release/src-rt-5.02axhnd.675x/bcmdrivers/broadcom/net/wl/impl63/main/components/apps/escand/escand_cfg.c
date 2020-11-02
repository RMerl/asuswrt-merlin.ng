/*
 *      escand_cfg.c
 *
 *      This module will retrieve escand configurations from nvram, if params are
 *      not set it will retrieve default values.
 *
 *      Copyright 2020 Broadcom
 *
 *      This program is the proprietary software of Broadcom and/or
 *      its licensors, and may only be used, duplicated, modified or distributed
 *      pursuant to the terms and conditions of a separate, written license
 *      agreement executed between you and Broadcom (an "Authorized License").
 *      Except as set forth in an Authorized License, Broadcom grants no license
 *      (express or implied), right to use, or waiver of any kind with respect to
 *      the Software, and Broadcom expressly reserves all rights in and to the
 *      Software and all intellectual property rights therein.  IF YOU HAVE NO
 *      AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 *      WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 *      THE SOFTWARE.
 *
 *      Except as expressly set forth in the Authorized License,
 *
 *      1. This program, including its structure, sequence and organization,
 *      constitutes the valuable trade secrets of Broadcom, and you shall use
 *      all reasonable efforts to protect the confidentiality thereof, and to
 *      use this information only in connection with your use of Broadcom
 *      integrated circuit products.
 *
 *      2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *      "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 *      REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 *      OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 *      DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 *      NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 *      ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 *      CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 *      OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *      3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 *      BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 *      SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 *      IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 *      IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 *      ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 *      OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 *      NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *	$Id: escand_cfg.c 768862 2018-10-30 06:15:40Z $
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <assert.h>
#include <typedefs.h>

#include "escand_svr.h"

#define MAX_KEY_LEN 16				/* the expanded key format string must fit within */

#define ESCAND_DFLT_FLAGS ESCAND_FLAGS_LASTUSED_CHK

extern int bcm_ether_atoe(const char *p, struct ether_addr *ea);

/*
 * Function to set a channel table by parsing a list consisting
 * of a comma-separated channel numbers.
 */
int
escand_set_chan_table(char *channel_list, chanspec_t *chspec_list,
                      unsigned int vector_size)
{
	int chan_index;
	int channel;
	int chan_count = 0;
	char *chan_str;
	char *delim = ",";
	char chan_buf[ESCAND_MAX_VECTOR_LEN + 2];
	int list_length;

	/*
	* NULL list means no channels are set. Return without
	* modifying the vector.
	*/
	if (channel_list == NULL)
		return 0;

	/*
	* A non-null list means that we must set the vector.
	*  Clear it first.
	* Then parse a list of <chan>,<chan>,...<chan>
	*/
	memset(chan_buf, 0, sizeof(chan_buf));
	list_length = strlen(channel_list);
	list_length = MIN(list_length, ESCAND_MAX_VECTOR_LEN);
	strncpy(chan_buf, channel_list, list_length);
	strncat(chan_buf, ",", list_length);

	chan_str = strtok(chan_buf, delim);

	for (chan_index = 0; chan_index < vector_size; chan_index++)
	{
		if (chan_str == NULL)
			break;
		channel = strtoul(chan_str, NULL, 16);
		if (channel == 0)
			break;
		chspec_list[chan_count++] = channel;
		chan_str = strtok(NULL, delim);
	}
	return chan_count;
}

void
escand_retrieve_config(escand_chaninfo_t *c_info, char *prefix)
{
	/* retrieve policy related configuration from nvram */
	char conf_word[128], tmp[200];
	char *str;
	uint32 flags;
	uint8 chan_count;

	/* the current layout of config */
	ESCAND_INFO("retrieve config from nvram ...\n");

	escand_safe_get_conf(conf_word, sizeof(conf_word),
		strcat_r(prefix, "escand_scan_entry_expire", tmp));

	if (!strcmp(conf_word, "")) {
		ESCAND_INFO("No escand_scan_entry_expire set. Retrieve default.\n");
		c_info->escand_scan_entry_expire = ESCAND_CI_SCAN_EXPIRE;
	}
	else {
		char *endptr = NULL;
		c_info->escand_scan_entry_expire = strtoul(conf_word, &endptr, 0);
		ESCAND_DEBUG("escand_scan_entry_expire: 0x%x\n", c_info->escand_scan_entry_expire);
	}

	escand_safe_get_conf(conf_word, sizeof(conf_word),
		strcat_r(prefix, "escand_boot_only", tmp));

	if (!strcmp(conf_word, "")) {
		ESCAND_INFO("No escand_boot_only is set. Retrieve default. \n");
		c_info->escand_boot_only = ESCAND_BOOT_ONLY_DEFAULT;
	} else {
		char *endptr = NULL;
		c_info->escand_boot_only = strtoul(conf_word, &endptr, 0);
		ESCAND_DEBUG("escand_boot_only: 0x%x\n", c_info->escand_boot_only);
	}

	escand_safe_get_conf(conf_word, sizeof(conf_word),
		strcat_r(prefix, "escand_flags", tmp));

	if (!strcmp(conf_word, "")) {
		ESCAND_INFO("No escand flag set. Retrieve default.\n");
		flags = ESCAND_DFLT_FLAGS;
	}
	else {
		char *endptr = NULL;
		flags = strtoul(conf_word, &endptr, 0);
		ESCAND_DEBUG("escand flags: 0x%x\n", flags);
	}

	if ((str = nvram_get(strcat_r(prefix, "escand_far_sta_rssi", tmp))) == NULL) {
		c_info->escand_far_sta_rssi = ESCAND_FAR_STA_RSSI;
#ifdef MULTIAP
		 /* far sta rssi's default value should be same as in wbd_weak_sta_cfg */
		if (((str = nvram_get(strcat_r(prefix, "map", tmp))) != NULL) && atoi(str)) {
			if ((str = nvram_get(strcat_r(prefix, "wbd_weak_sta_cfg", tmp))) != NULL) {
				int wbd_rssi = 0;
				sscanf(str, "%d %d", &wbd_rssi, &wbd_rssi);
				c_info->escand_far_sta_rssi = wbd_rssi;
			}
		}
#endif /* MULTIAP */
	} else {
		c_info->escand_far_sta_rssi = atoi(str);
	}

	memset(&c_info->pref_chans, 0, sizeof(escand_conf_chspec_t));
	if ((str = nvram_get(strcat_r(prefix, "escand_pref_chans", tmp))) == NULL)	{
		c_info->pref_chans.count = 0;
	} else {
		chan_count = escand_set_chan_table(str, c_info->pref_chans.clist, ESCAND_MAX_LIST_LEN);
		c_info->pref_chans.count = chan_count;
	}
	memset(&c_info->excl_chans, 0, sizeof(escand_conf_chspec_t));
	if ((str = nvram_get(strcat_r(prefix, "escand_excl_chans", tmp))) == NULL)	{
		c_info->excl_chans.count = 0;
	} else {
		chan_count = escand_set_chan_table(str, c_info->excl_chans.clist, ESCAND_MAX_LIST_LEN);
		c_info->excl_chans.count = chan_count;
	}

	if ((str = nvram_get(strcat_r(prefix, "escand_ci_scan_timeout", tmp))) == NULL)
		c_info->escand_ci_scan_timeout = ESCAND_CI_SCAN_TIMEOUT;
	else
		c_info->escand_ci_scan_timeout = atoi(str);

	if ((str = nvram_get(strcat_r(prefix, "escand_cs_scan_timer", tmp))) == NULL)
		c_info->escand_cs_scan_timer = ESCAND_DFLT_CS_SCAN_TIMER;
	else
		c_info->escand_cs_scan_timer = atoi(str);

	if ((str = nvram_get(strcat_r(prefix, "escand_ci_scan_timer", tmp))) == NULL)
		c_info->escand_ci_scan_timer = ESCAND_DFLT_CI_SCAN_TIMER;
	else
		c_info->escand_ci_scan_timer = atoi(str);

	if ((str = nvram_get(strcat_r(prefix, "escand_scan_promisc", tmp))) == NULL)
		c_info->escand_scan_promisc = 0;
	else
		c_info->escand_scan_promisc = atoi(str);

	if (nvram_match(strcat_r(prefix, "dcs_csa_unicast", tmp), "1"))
		c_info->escand_dcs_csa = CSA_UNICAST_ACTION_FRAME;
	else
		c_info->escand_dcs_csa = CSA_BROADCAST_ACTION_FRAME;

	/* Customer Knob #2
	 * Preference for channel power
	 */
	escand_safe_get_conf(conf_word, sizeof(conf_word),
			strcat_r(prefix, "escand_cs_high_pwr_pref", tmp));

	if (!strcmp(conf_word, "")) {
		ESCAND_INFO("No escand_cs_high_pwr_pref set. Disabled by default.\n");
		c_info->escand_cs_high_pwr_pref = 0;
	}
	else {
		char *endptr = NULL;
		c_info->escand_cs_high_pwr_pref = strtoul(conf_word, &endptr, 0);
		ESCAND_DEBUG("escand_cs_high_pwr_pref: 0x%x\n", c_info->escand_cs_high_pwr_pref);
	}

	/* allocate core data structure for escan */
	c_info->escand_escan =
		(escand_escaninfo_t *)escand_malloc(sizeof(*(c_info->escand_escan)));

	escand_safe_get_conf(conf_word, sizeof(conf_word),
			strcat_r(prefix, "escand_use_escan", tmp));

	if (!strcmp(conf_word, "")) {
		ESCAND_INFO("No escan config set. use defaults\n");
		c_info->escand_escan->escand_use_escan = ESCAND_ESCAN_DEFAULT;
	}
	else {
		char *endptr = NULL;
		c_info->escand_escan->escand_use_escan = strtoul(conf_word, &endptr, 0);
		ESCAND_DEBUG("escand escan enable: %d\n", c_info->escand_escan->escand_use_escan);
	}

	c_info->flags = flags;
#ifdef DEBUG
	escand_dump_policy(a_pol);
#endif // endif
}
