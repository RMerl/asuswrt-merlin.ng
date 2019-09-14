 /*
 * Copyright 2019, ASUSTeK Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. ASUS
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

/*
	NVIDIA GeForceNow UPnP QoS related API
*/

#include <stdio.h>
#include <string.h>
#include <shared.h>
#include <bcmnvram.h>
#include <shutils.h>

#define UDP_RMEM_MIN    "/proc/sys/net/ipv4/udp_rmem_min"
#define UDP_WMEM_MIN    "/proc/sys/net/ipv4/udp_wmem_min"
#define UDP_MEM         "/proc/sys/net/ipv4/udp_mem"

void nvgfn_kernel_setting(int val)
{
	if (val) {
		f_write_string(UDP_RMEM_MIN, "16384", 0, 0);
		f_write_string(UDP_WMEM_MIN, "16384", 0, 0);
		f_write_string(UDP_MEM     , "65536 131072 262144", 0, 0);
	}
	else {
		f_write_string(UDP_RMEM_MIN, "4096", 0, 0);
		f_write_string(UDP_WMEM_MIN, "4096", 0, 0);
		f_write_string(UDP_MEM     , "10317 13758 20634", 0, 0);
	}
	NVGFN_DBG("modify kernel setting (%d)\n", val);
}

char *upnp_nvram_get(char *name)
{
	return nvram_safe_get(name);
}

int upnp_nvram_set(char *name, char *value)
{
	nvram_set(name, value);
	return 1;
}

static void nvgfn_run_GFNQoS()
{
	if (IS_GFN_QOS()) {
		notify_rc("restart_qos");
		notify_rc("restart_firewall");
	}
}

int nvgfn_GetQosState(int *value)
{
	if (IS_GFN_QOS()) {
		*value = 1;
	}
	else {
		*value = 0;
	}

	NVGFN_DBG("value = %d\n", *value);
	return 1;
}

int nvgfn_SetQosState(char *value)
{
	int is_changed = 0;

	if (strcmp(nvram_safe_get("qos_enable"), value))
		is_changed = 1;

	if (!strcmp(value, "1")) {
		nvram_set("qos_enable", "1");
		nvram_set("qos_type", "3");
	}
	else {
		nvram_set("qos_enable", "0");
		nvram_set("qos_type", nvram_default_get("qos_type"));
	}
	nvram_commit();

	if (is_changed) {
		notify_rc("restart_qos");
		notify_rc("restart_firewall");
	}

	NVGFN_DBG("qos_state = %s, is_changed = %d\n", value, is_changed);
	return 1;
}

static void nvgfn_GetQosParameters(char *str, NVGFN_QOS_RULE_T *input)
{
	char *buf = NULL, *g = NULL, *p = NULL;
	char *type = NULL, *proto = NULL, *port = NULL;

	g = buf = strdup(nvram_safe_get("nvgfn_ch_rulelist"));
	while (g) {
		if ((p = strsep(&g, "<")) == NULL) break;
		if ((vstrsep(p, ">", &type, &proto, &port)) != 3) continue;
		if (!strcmp(str, type)) {
			snprintf(input->protocol, sizeof(input->protocol), "%s", proto);
			input->port = atoi(port);
			break;
		}
	}
	if (buf) free(buf);
	snprintf(input->direction, sizeof(input->direction), "%s", "inout");
	NVGFN_DBG("input channel/proto/port/direction = %s/%s/%d/%s\n", input->channel, input->protocol, input->port, input->direction);
}

int nvgfn_GetQosRule(NVGFN_QOS_RULE_T *input)
{
	int ret = 1;

	if (!strcmp(input->channel, "audio")) {
		nvgfn_GetQosParameters("audio", input);
	}
	else if (!strcmp(input->channel, "mic")) {
		nvgfn_GetQosParameters("mic", input);
	}
	else if (!strcmp(input->channel, "video")) {
		nvgfn_GetQosParameters("video", input);
	}
	else if (!strcmp(input->channel, "control")) {
		nvgfn_GetQosParameters("control", input);
	}
	else {
		// return error code
		ret = 0;
	}

	return ret;
} 

static void nvgfn_SetQosParameters(char *str, NVGFN_QOS_RULE_T *input)
{
	char tmp[20] = {0};
	char new[80] = {0};
	char final[80] = {0};
	char *buf = NULL, *g = NULL, *p = NULL;
	char *type = NULL, *proto = NULL, *port = NULL;
	int is_first = 1;

	g = buf = strdup(nvram_safe_get("nvgfn_ch_rulelist"));
	while (g) {
		if ((p = strsep(&g, "<")) == NULL) break;
		if ((vstrsep(p, ">", &type, &proto, &port)) != 3) continue;
		if (!strcmp(str, type)) {
			snprintf(tmp, sizeof(tmp), "%s>%s>%d", type, input->protocol, input->port);
		}
		else
			snprintf(tmp, sizeof(tmp), "%s>%s>%s", type, proto, port);

		if (is_first) {
			snprintf(new, sizeof(new), "%s", tmp);
			is_first = 0;
		}
		else {
			snprintf(new, sizeof(new), "%s<%s", final, tmp);
		}
		snprintf(final, sizeof(final), "%s", new);
	}
	if (buf) free(buf);

	nvram_set("nvgfn_ch_rulelist", final);
	nvram_commit();

	NVGFN_DBG("input channel/proto/port/direction = %s/%s/%d/%s\n", input->channel, input->protocol, input->port, input->direction);
	NVGFN_DBG("input buf = %s\n", final);
}

int nvgfn_SetQoSRule(NVGFN_QOS_RULE_T *input)
{
	NVGFN_DBG("qos rule channel/proto/port/direction = %s/%s/%d/%s\n", input->channel, input->protocol, input->port, input->direction);
	int ret = 1;

	if (!strcmp(input->channel, "audio")) {
		nvgfn_SetQosParameters("audio", input);
	}
	else if (!strcmp(input->channel, "mic")) {
		nvgfn_SetQosParameters("mic", input);
	}
	else if (!strcmp(input->channel, "video")) {
		nvgfn_SetQosParameters("video", input);
	}
	else if (!strcmp(input->channel, "control")) {
		nvgfn_SetQosParameters("control", input);
	}
	else {
		// return error code
		ret = 0;
	}

	/* run GFN QoS service */
	nvgfn_run_GFNQoS();

	return ret;
}

static void nvgfn_StopWifiScan(int value)
{
	char wl[16] = {0};
	char *next = NULL;
	int i = 0;

	foreach(wl, nvram_safe_get("wl_ifnames"), next)
	{
		wl_set_wifiscan(wl, value);
		i++;
		NVGFN_DBG("[%d] wifi scan (%s)\n", i, value ? "stop scan" : "scan");
	}

	nvram_set_int("nvgfn_scansuppress", value);
	nvram_commit();
}

int nvgfn_GetWifiScanState(int *value)
{
	*value = (nvram_get_int("nvgfn_scansuppress") == 1)? 0: 1;

	NVGFN_DBG("value = %d\n", *value);
	return 1;
}

int nvgfn_SetWifiScanState(char *value)
{
	int stopscan = 0;

	/* check whether need to stop scan */
	if (!strcmp(value, "0")) stopscan = 1;

	/* call system cmd */
	nvgfn_StopWifiScan(stopscan);

	return 1;
}

int nvgfn_GetWifiScanInterval(int *value)
{
	*value = nvram_get_int("nvgfn_scaninterval");

	NVGFN_DBG("value = %d\n", *value);
	return 1;
}

int nvgfn_SetWifiScanInterval(char *value)
{
	/* interaction between WifiScanInterval and WifiScanState */
	nvgfn_SetWifiScanState(value);

	nvram_set("nvgfn_scaninterval", value);
	nvram_commit();

	return 1;
}

int nvgfn_GetDownloadBandwidth(int *value)
{
	unsigned int bw = strtoul(nvram_safe_get("qos_ibw"), NULL, 10);
	*value = bw / 1024;

	NVGFN_DBG("ibw = %d\n", *value);
	return 1;
}

int nvgfn_SetDownloadBandwidth(char *value)
{
	int ibw = atoi(value) * 1024;
	char ibw_str[32] = {0};

	snprintf(ibw_str, sizeof(ibw_str), "%d", ibw);
	nvram_set("qos_ibw", ibw_str);
	nvram_commit();

	/* run GFN QoS service */
	nvgfn_run_GFNQoS();

	NVGFN_DBG("ibw = %s\n", ibw_str);
	return 1;
}

int nvgfn_GetDownloadBandwidthReservation(int *value)
{
	unsigned int bw = strtoul(nvram_safe_get("nvgfn_ibw_r"), NULL, 10);
	*value = bw / 1024;

	NVGFN_DBG("nvgfn_ibw_reservation = %s\n", *value);
	return 1;
}

int nvgfn_SetDownloadBandwidthReservation(char *value)
{
	int ret = 1;
	int ibwre = atoi(value) * 1024;
	int bound = strtoul(nvram_safe_get("qos_ibw"), NULL, 10) * 1024;
	char ibwre_str[32] = {0};

	// avoid the reservation bandwidth is over the setting
	if (ibwre > bound) {
		ibwre = bound;
		ret = 0;
	}

	snprintf(ibwre_str, sizeof(ibwre_str), "%d", ibwre);
	nvram_set("nvgfn_ibw_r", ibwre_str);
	nvram_commit();

	/* run GFN QoS service */
	nvgfn_run_GFNQoS();

	NVGFN_DBG("nvgfn_ibw_reservation = %s, ret = %d\n", ibwre_str, ret);
	return ret;
}

int nvgfn_GetUploadBandwidth(int *value)
{
	unsigned int bw = strtoul(nvram_safe_get("qos_obw"), NULL, 10);
	*value = bw / 1024;

	NVGFN_DBG("obw = %d\n", *value);
	return 1;
}

int nvgfn_SetUploadBandwidth(char *value)
{
	int obw = atoi(value) * 1024;
	char obw_str[32] = {0};

	snprintf(obw_str, sizeof(obw_str), "%d", obw);
	nvram_set("qos_obw", obw_str);
	nvram_commit();

	/* run GFN QoS service */
	nvgfn_run_GFNQoS();

	NVGFN_DBG("obw = %s\n", obw_str);
	return 1;
}

int nvgfn_GetUploadBandwidthReservation(int *value)
{
	unsigned int bw = strtoul(nvram_safe_get("nvgfn_obw_r"), NULL, 10);
	*value = bw / 1024;

	NVGFN_DBG("nvgfn_obw_reservation = %s\n", *value);
	return 1;
}

int nvgfn_SetUploadBandwidthReservation(char *value)
{
	int ret = 1;
	int obwre = atoi(value) * 1024;
	int bound = strtoul(nvram_safe_get("qos_obw"), NULL, 10) * 1024;
	char obwre_str[32] = {0};

	// avoid the reservation bandwidth is over the setting
	if (obwre > bound) {
		obwre = bound;
		ret = 0;
	}

	snprintf(obwre_str, sizeof(obwre_str), "%d", obwre);
	nvram_set("nvgfn_obw_r", obwre_str);
	nvram_commit();

	/* run GFN QoS service */
	nvgfn_run_GFNQoS();

	NVGFN_DBG("nvgfn_obw_reservation = %s, ret = %d\n", obwre_str, ret);
	return ret;
}

int nvgfn_GetMcsIndex(NVGFN_MCS_INDEX_T *input)
{
	char *buf = NULL, *g = NULL;
	char *idx_auto = NULL, *idx = NULL, *type = NULL, *stream = NULL;

	g = buf = strdup(nvram_safe_get("nvgfn_mcs"));
	if ((vstrsep(g, "<", &idx_auto, &idx, &type, &stream)) == 4) {
		if (!strcmp(idx_auto, "1")) {
			input->McsIndexAuto = atoi(idx_auto);
			input->McsIndex = 0;
			snprintf(input->McsIndexType, sizeof(input->McsIndexType), "%s", "");
			input->SpatialStreams = 0;
		}
		else {
			input->McsIndexAuto = atoi(idx_auto);
			input->McsIndex = atoi(idx);
			snprintf(input->McsIndexType, sizeof(input->McsIndexType), "%s", type);
			input->SpatialStreams = atoi(stream);
		}
		NVGFN_DBG("auto/idx/type/stream = %d/%d/%s/%d\n", input->McsIndexAuto, input->McsIndex, input->McsIndexType, input->SpatialStreams);
	}
	else {
		NVGFN_DBG("illegal format = %s\n", buf);
	}

	if(buf) free(buf);

	return 1;
}

int nvgfn_SetMcsIndex(NVGFN_MCS_INDEX_T *input)
{
	int is_auto = 1;
	int idx = 0;
	int stream = 0;
	char idx_type[8] = {0};
	char buf[128] = {0};
	char wl[16] = {0};
	char *next = NULL;

	is_auto = input->McsIndexAuto;
	idx = input->McsIndex;
	stream = input->SpatialStreams;
	snprintf(idx_type, sizeof(idx_type), "%s", input->McsIndexType);

	if (stream > 4) stream = 4;
	NVGFN_DBG("is_auto/idx/idx_type/stream = %d/%d/%s/%d\n", is_auto, idx, idx_type, stream);

	/* command line part */
	foreach(wl, nvram_safe_get("wl_ifnames"), next)
	{
		wl_set_mcsindex(wl, &is_auto, &idx, idx_type, &stream);
	}

	/* nvram part */
	if (is_auto) {
		idx = 0;
		stream = 0;
		snprintf(idx_type, sizeof(idx_type), "%s", "");
	}
	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf), "%d<%d<%s<%d", is_auto, idx, idx_type, stream);
	nvram_set("nvgfn_mcs", buf);
	nvram_commit();
	NVGFN_DBG("nvram = %s\n", buf);

	return 1;
}

void nvgfn_mcs_isauto(int val)
{
	if (val == 0) {
		NVGFN_MCS_INDEX_T input;
		input.McsIndexAuto = 1;
		input.McsIndex = 0;
		snprintf(input.McsIndexType, sizeof(input.McsIndexType), "%s", "");
		input.SpatialStreams = 0;
		nvgfn_SetMcsIndex(&input);
		NVGFN_DBG("recover mcs setting into auto (%d)\n", val);
	}
}

