/*
 * ESCAND deamon command module (Linux)
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
 * $Id: escand_cmd.c 761206 2018-05-07 05:50:21Z $
 */

#include "escand_svr.h"

extern void escand_cleanup_scan_entry(escand_chaninfo_t *c_info);

static int
escand_extract_token_val(char* data, const char *token, char *output, int len)
{
	char *p, *c;
	char copydata[512];
	char *val;

	if (data == NULL)
		goto err;

	strncpy(copydata, data, sizeof(copydata));
	copydata[sizeof(copydata) - 1] = '\0';

	ESCAND_DEBUG("copydata: %s\n", copydata);

	p = strstr(copydata, token);
	if (!p)
		goto err;

	if ((c = strchr(p, '&')))
		*c++ = '\0';

	val = strchr(p, '=');
	if (!val)
		goto err;

	val += 1;
	ESCAND_DEBUG("token_val: %s\n", val);

	strncpy(output, val, len);
	output[len - 1] = '\0';

	return strlen(output);

err:
	return -1;
}

/* buf should be null terminated. rcount doesn;t include the terminuating null */
int
escand_proc_cmd(escand_wksp_t* d_info, char* buf, uint rcount, uint* r_size)
{
	char *c, *data;
	int err = 0, ret;
	char ifname[IFNAMSIZ];

	/* Check if we have command and data in the expected order */
	if (!(c = strchr(buf, '&'))) {
		ESCAND_ERROR("Missing Command\n");
		err = -1;
		goto done;
	}
	*c++ = '\0';
	data = c;

	if (!strcmp(buf, "info")) {
		time_t ltime;
		int i;
		const char *mode_str[] = {"disabled", "monitor"};
		d_info->stats.valid_cmds++;

		time(&ltime);
		*r_size = sprintf(buf, "time: %s \n", ctime(&ltime));
		*r_size += sprintf(buf+ *r_size, "escand version: %d\n", d_info->version);
		*r_size += sprintf(buf+ *r_size, "escand ticks: %d\n", d_info->ticks);
		*r_size += sprintf(buf+ *r_size, "escand poll_interval: %d seconds\n",
			d_info->poll_interval);

		for (i = 0; i < ESCAND_MAX_IF_NUM; i++) {
			escand_chaninfo_t *c_info = NULL;

			c_info = d_info->escand_info->chan_info[i];

			if (!c_info)
				continue;

			*r_size += sprintf(buf+ *r_size, "ifname: %s, mode: %s\n",
				c_info->name, mode_str[c_info->mode]);
		}

		goto done;
	}

	if (!strcmp(buf, "chanlist")) {
		escand_chaninfo_t *c_info = NULL;
		int i, index;
		escand_scan_chspec_t* chspec_q;
		scan_chspec_elemt_t *ch_list;

		d_info->stats.valid_cmds++;

		if ((ret = escand_extract_token_val(data, "ifname", ifname, sizeof(ifname))) < 0) {
			*r_size = sprintf(buf, "Request failed: missing ifname");
			goto done;
		}

		ESCAND_DEBUG("cmd: %s, data: %s, ifname: %s\n",
			buf, data, ifname);

		index = escand_idx_from_map(ifname);
		if (index != -1) {
			c_info = d_info->escand_info->chan_info[index];
		}

		if (!c_info) {
			*r_size = sprintf(buf, "Request not permitted: "
				"Interface was not intialized properly");
			goto done;
		}

		chspec_q = &c_info->scan_chspec_list;
		ch_list = c_info->scan_chspec_list.chspec_list;

		if (chspec_q->count == 0) {
			ESCAND_PRINT("scan chanspec queue count %u.\n", chspec_q->count);
			goto done;
		}

		ESCAND_PRINT("List of channels for intf: %s:\n", ifname);
		for (i = 0; i < chspec_q->count; i++) {
			ESCAND_PRINT("chanspec: (0x%04x), chspec_info: 0x%08x  flags: 0x%08x\n",
				ch_list[i].chspec, ch_list[i].chspec_info, ch_list[i].flags);
		}
		ESCAND_PRINT("Number of channels %d, preferred %d, excluded %d\n", chspec_q->count,
				c_info->pref_chans.count,
				c_info->excl_chans.count);

		goto done;
	}

	if (!strcmp(buf, "csscan")) {
		escand_chaninfo_t *c_info = NULL;
		int index;

		d_info->stats.valid_cmds++;

                ESCAND_PRINT("For future use\n");
                goto done;

		if ((ret = escand_extract_token_val(data, "ifname", ifname, sizeof(ifname))) < 0) {
			*r_size = sprintf(buf, "Request failed: missing ifname");
			goto done;
		}

		ESCAND_DEBUG("cmd: %s, data: %s, ifname: %s\n",
			buf, data, ifname);

		index = escand_idx_from_map(ifname);
		if (index != -1) {
			c_info = d_info->escand_info->chan_info[index];
		}

		if (!c_info) {
			*r_size = sprintf(buf, "Request not permitted: "
				"Interface was not intialized properly");
			goto done;
		}

		err = escand_run_cs_scan(c_info);

		escand_cleanup_scan_entry(c_info);
		err = escand_request_data(c_info);

		*r_size = sprintf(buf, "Request finished");
		goto done;
	}

	if (!strcmp(buf, "dump")) {
		char param[128];
		int index;
		escand_chaninfo_t *c_info = NULL;

		if ((ret = escand_extract_token_val(data, "param", param, sizeof(param))) < 0) {
			*r_size = sprintf(buf, "Request failed: missing param");
			goto done;
		}

		if ((ret = escand_extract_token_val(data, "ifname", ifname, sizeof(ifname))) < 0) {
			*r_size = sprintf(buf, "Request failed: missing ifname");
			goto done;
		}

		ESCAND_DEBUG("cmd: %s, data: %s, param: %s, ifname: %s\n",
			buf, data, param, ifname);

		index = escand_idx_from_map(ifname);

		ESCAND_DEBUG("index is : %d\n", index);
		if (index != -1)
			c_info = d_info->escand_info->chan_info[index];

		ESCAND_DEBUG("c_info: %p\n", c_info);

		if (!c_info) {
			*r_size = sprintf(buf, "ERR: Requested info not available");
			goto done;
		}

		d_info->stats.valid_cmds++;
		if (!strcmp(param, "help")) {
			*r_size = sprintf(buf,
				"dump: stats bss scanresults");
		} else if (!strcmp(param, "bss")) {
			escand_chan_bssinfo_t *bssinfo = c_info->ch_bssinfo;

			if (!bssinfo) {
				*r_size = sprintf(buf, "ERR: Requested info not available");
				goto done;
			}

			memcpy((void*)buf, (void*)bssinfo, sizeof(escand_chan_bssinfo_t) *
				c_info->scan_chspec_list.count);
			*r_size = sizeof(escand_chan_bssinfo_t) * c_info->scan_chspec_list.count;

		} else if (!strcmp(param, "stats")) {
			escand_stats_t * d_stats = &d_info->stats;

			if (!d_stats) {
				*r_size = sprintf(buf, "ERR: Requested info not available");
				goto done;
			}

			*r_size = sprintf(buf, "ESCAND stats:\n");
			*r_size += sprintf(buf + *r_size, "Total cmd: %d, Valid cmd: %d\n",
				d_stats->num_cmds, d_stats->valid_cmds);
			*r_size += sprintf(buf + *r_size, "Total events: %d, Valid events: %d\n",
				d_stats->num_events, d_stats->valid_events);

			goto done;

		} else if (!strcmp(param, "scanresults")) {

			escand_dump_scan_entry(c_info);
		} else {
			*r_size = sprintf(buf, "Unsupported dump command (try \"dump help\")");
		}
		goto done;
	}

	if (!strcmp(buf, "get")) {
		char param[128];
		int index;
		escand_chaninfo_t *c_info = NULL;

		if ((ret = escand_extract_token_val(data, "param", param, sizeof(param))) < 0) {
			*r_size = sprintf(buf, "Request failed: missing param");
			goto done;
		}

		if (!strcmp(param, "msglevel")) {
			d_info->stats.valid_cmds++;
			*r_size = sprintf(buf, "%d", escand_debug_level);
			ESCAND_DEBUG("cmd: %s, data: %s, param: %s\n",
				buf, data, param);
			goto done;
		}

		if ((ret = escand_extract_token_val(data, "ifname", ifname, sizeof(ifname))) < 0) {
			*r_size = sprintf(buf, "Request failed: missing ifname");
			goto done;
		}

		ESCAND_DEBUG("cmd: %s, data: %s, param: %s, ifname: %s\n",
			buf, data, param, ifname);

		index = escand_idx_from_map(ifname);

		ESCAND_DEBUG("index is : %d\n", index);
		if (index != -1)
			c_info = d_info->escand_info->chan_info[index];

		ESCAND_DEBUG("c_info: %p\n", c_info);

		if (!c_info) {
			*r_size = sprintf(buf, "ERR: Requested info not available");
			goto done;
		}

		d_info->stats.valid_cmds++;
		if (!strcmp(param, "msglevel")) {
			*r_size = sprintf(buf, "%d", escand_debug_level);
			goto done;
		}

		if (!strcmp(param, "mode")) {
			const char *mode_str[] = {"disabled", "monitor"};
			int escand_mode = c_info->mode;
			*r_size = sprintf(buf, "%d: %s", escand_mode, mode_str[escand_mode]);
			goto done;
		}

		if (!strcmp(param, "escand_cs_scan_timer")) {
			if (c_info->escand_cs_scan_timer)
				*r_size = sprintf(buf, "%d sec", c_info->escand_cs_scan_timer);
			else
				*r_size = sprintf(buf, "OFF");
			goto done;
		}

		if (!strcmp(param, "escand_flags")) {
			*r_size = sprintf(buf, "0x%x", c_info->flags);
			goto done;
		}

		if (!strcmp(param, "escand_ci_scan_timeout")) {
			*r_size = sprintf(buf, "%d sec", c_info->escand_ci_scan_timeout);
			goto done;
		}

		if (!strcmp(param, "escand_far_sta_rssi")) {
			*r_size = sprintf(buf, "%d", c_info->escand_far_sta_rssi);
			goto done;
		}
		if (!strcmp(param, "escand_scan_entry_expire")) {
			*r_size = sprintf(buf, "%d sec", c_info->escand_scan_entry_expire);
			goto done;
		}
		if (!strcmp(param, "escand_ci_scan_timer")) {
			*r_size = sprintf(buf, "%d sec", c_info->escand_ci_scan_timer);
			goto done;
		}

		*r_size = sprintf(buf, "GET: Unknown variable \"%s\".", param);
		err = -1;
		goto done;
	}

	if (!strcmp(buf, "readchans")) {
		escand_chaninfo_t *c_info = NULL;
		int i, index;
		char *str;
		char prefix[32];
		char tmp[200];
		int unit;
		uint8 chan_count;

		d_info->stats.valid_cmds++;

		if ((ret = escand_extract_token_val(data, "ifname", ifname, sizeof(ifname))) < 0) {
			*r_size = sprintf(buf, "Request failed: missing ifname");
			goto done;
		}

		ESCAND_DEBUG("cmd: %s, data: %s, ifname: %s\n",
			buf, data, ifname);

		index = escand_idx_from_map(ifname);
		if (index != -1) {
			c_info = d_info->escand_info->chan_info[index];
		}

		if (!c_info) {
			*r_size = sprintf(buf, "Request not permitted: "
				"Interface was not intialized properly");
			goto done;
		}

		ret = wl_ioctl(ifname, WLC_GET_INSTANCE, &unit, sizeof(unit));
		escand_snprintf(prefix, sizeof(prefix), "wl%d_", unit);

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

		ESCAND_PRINT("List of preferred channels for intf: %s, count = %d:\n", ifname,
			c_info->pref_chans.count);
		for (i = 0; i < c_info->pref_chans.count; i++) {
			ESCAND_PRINT("channel: (0x%04x)\n", c_info->pref_chans.clist[i]);
		}

		ESCAND_PRINT("List of excluded channels for intf: %s, count = %d:\n", ifname,
			c_info->excl_chans.count);
		for (i = 0; i < c_info->excl_chans.count; i++) {
			ESCAND_PRINT("channel: (0x%04x)\n", c_info->excl_chans.clist[i]);
		}

		ret = escand_build_scanlist(c_info);
		goto done;
	}

	if (!strcmp(buf, "set")) {
		char param[128];
		char val[16];
		int setval = 0;
		int index;
		escand_chaninfo_t *c_info = NULL;

		if ((ret = escand_extract_token_val(data, "param", param, sizeof(param))) < 0) {
			*r_size = sprintf(buf, "Request failed: missing param");
			goto done;
		}

		if ((ret = escand_extract_token_val(data, "val", val, sizeof(val))) < 0) {
			*r_size = sprintf(buf, "Request failed: missing val");
			goto done;
		}

		setval = atoi(val);

		if (!strcmp(param, "msglevel")) {
			d_info->stats.valid_cmds++;
			escand_debug_level = setval;
			*r_size = sprintf(buf, "%d", escand_debug_level);
			ESCAND_DEBUG("cmd: %s, data: %s, param: %s val: %d\n",
				buf, data, param, setval);
			goto done;
		}

		if ((ret = escand_extract_token_val(data, "ifname", ifname, sizeof(ifname))) < 0) {
			*r_size = sprintf(buf, "Request failed: missing ifname");
			goto done;
		}

		index = escand_idx_from_map(ifname);

		ESCAND_DEBUG("index is : %d\n", index);
		if (index != -1)
			c_info = d_info->escand_info->chan_info[index];

		ESCAND_DEBUG("c_info: %p\n", c_info);

		if (!c_info) {
			*r_size = sprintf(buf, "ERR: Requested ifname not available");
			goto done;
		}

		ESCAND_DEBUG("cmd: %s, data: %s, param: %s val: %d, ifname: %s\n",
			buf, data, param, setval, ifname);

		d_info->stats.valid_cmds++;

		if (!strcmp(param, "msglevel")) {
			escand_debug_level = setval;
			*r_size = sprintf(buf, "%d", escand_debug_level);
			goto done;
		}

		if (!strcmp(param, "mode")) {
			const char *mode_str[] = {"disabled", "monitor"};

			if (setval < ESCAND_MODE_DISABLE || setval > ESCAND_MODE_LAST) {
				*r_size = sprintf(buf, "Out of range");
				goto done;
			}

			c_info->mode = setval;
			*r_size = sprintf(buf, "%d: %s", setval, mode_str[setval]);
			ESCAND_DEBUG("Setting ESCAND mode = %d: %s\n", setval, mode_str[setval]);
			goto done;
		}

		if (!strcmp(param, "escand_cs_scan_timer")) {
			if (setval != 0 && setval < ESCAND_CS_SCAN_TIMER_MIN) {
				*r_size = sprintf(buf, "Out of range");
				goto done;
			}

			c_info->escand_cs_scan_timer = setval;

			if (setval)
				*r_size = sprintf(buf, "%d sec", c_info->escand_cs_scan_timer);
			else
				*r_size = sprintf(buf, "OFF");
			goto done;
		}

		if (!strcmp(param, "escand_flags")) {

			c_info->flags = setval;

			*r_size = sprintf(buf, "flags: 0x%x", c_info->flags);
			goto done;
		}

		if (!strcmp(param, "escand_ci_scan_timeout")) {
			c_info->escand_ci_scan_timeout = setval;
			*r_size = sprintf(buf, "%d sec", c_info->escand_ci_scan_timeout);
			goto done;
		}

		if (!strcmp(param, "escand_far_sta_rssi")) {
			c_info->escand_far_sta_rssi = setval;
			*r_size = sprintf(buf, "%d", c_info->escand_far_sta_rssi);
			goto done;
		}
		if (!strcmp(param, "escand_scan_entry_expire")) {
			c_info->escand_scan_entry_expire = setval;
			*r_size = sprintf(buf, "%d sec", c_info->escand_scan_entry_expire);
			goto done;
		}
		if (!strcmp(param, "escand_ci_scan_timer")) {
			c_info->escand_ci_scan_timer = setval;
			*r_size = sprintf(buf, "%d sec", c_info->escand_ci_scan_timer);
			goto done;
		}

		*r_size = sprintf(buf, "SET: Unknown variable \"%s\".", param);
		err = -1;
		goto done;
	}
done:
	return err;
}
