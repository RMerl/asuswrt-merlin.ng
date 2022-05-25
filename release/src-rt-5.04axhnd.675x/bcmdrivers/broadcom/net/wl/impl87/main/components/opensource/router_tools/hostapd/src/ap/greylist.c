/*
 * hostapd / RADIUS Greylist Access Control
 *
 * Copyright (C) 2022, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#define __USE_XOPEN
#define _GNU_SOURCE
#include <time.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include "utils/includes.h"
#include "utils/common.h"
#include "utils/eloop.h"
#include "greylist.h"

/* Timeout value from the client association time for deleting the
 * mac entry from greylist_mac.txt, and deleting the mac from access
 * control list of all greylist enabled vaps. */
#define GREYLIST_TIMEOUT_IN_SECONDS (24 * 60 * 60)

#define GREYLIST_MAX_NUM_OF_RECORDS 128

struct greylist_data {
	struct hapd_interfaces *interfaces;
	char txtaddr[TXT_MAC_ADDR_LEN];
};

/* Global array that stores CM mac of the gateway. */
char cmmac[TXT_MAC_ADDR_LEN];

static const char *wifi_health_log = "/rdklogs/logs/wifihealth.txt";
static const char *greylist_file = "/nvram/greylist_mac.txt";

static void greylist_log_to_file(char *fmt, ...);
static int greylist_get_cmmac();
static size_t greylist_delete_line(char *buffer, size_t size, const char *txtaddr);
static void greylist_delete_from_file(struct hapd_interfaces *interfaces, const char *txtaddr);
static void greylist_timeout(void *eloop_ctx, void *timeout_ctx);
static void greylist_add_to_driver(struct hapd_interfaces *interfaces, const char *txtaddr);
static int greylist_add_to_other_hostapd(const char *txtaddr);

/**
 * greylist_get_vap_index - Get RDK specific vap index of the given interface
 */
static int greylist_get_vap_index(const char* ifname)
{
	char str[IFNAMSIZ + 1];
	char *p;
	int unit = -1, subunit = -1;
	size_t ifname_len, len;
	unsigned long val;

	if (!ifname || *ifname == '\0')
		return -1;

	ifname_len = strlen(ifname);
	if (ifname_len + 1 > sizeof(str))
		return -1;

	strcpy(str, ifname);
	p = str + ifname_len - 1;

	/* find the trailing digit chars */
	len = 0;
	while (p >= str && (*p >= '0' && *p <= '9')) {
		--p;
		++len;
	}

	/* fail if there are no trailing digits */
	if (len == 0)
		return -1;

	++p;
	val = strtoul(p, NULL, 10);

	/* if we are at the beginning of the string, or the previous
	 * character is not a '.', then we have the unit number and
	 * we are done parsing
	 */
	if (p == str || p[-1] != '.') {
		unit = val;

		return unit + 1;
	} else
		subunit = val;

	/* chop off the '.NNN' and get the unit number */
	p--;
	*p = '\0';
	p--;

	/* find the trailing digit chars */
	len = 0;
	while (p >= str && (*p >= '0' && *p <= '9')) {
		--p;
		++len;
	}

	/* fail if there were no trailing digits */
	if (len == 0)
		return -1;

	/* point to the beginning of the last integer and convert */
	++p;
	val = strtoul(p, NULL, 10);

	/* save the unit number */
	unit = val;

	return unit + subunit*2 + 1;
}

/**
 * greylist_log_to_file - Add the log into wifihealth.txt for telemetry usage
 */
static void greylist_log_to_file(char *fmt, ...)
{
    FILE *fp = NULL;
    va_list args;

    fp = fopen(wifi_health_log, "a+");
    if (fp == NULL) {
        return;
    }

    va_start(args, fmt);
    vfprintf(fp, fmt, args);
    va_end(args);

    fflush(fp);
    fclose(fp);
}

/**
 * greylist_get_cmmac - Call the script to get cm mac of the gateway
 */
static int greylist_get_cmmac()
{
	FILE *fp;
	const char *cmd = "/usr/sbin/deviceinfo.sh -cmac";

	fp = popen(cmd, "r");

	if (fp == NULL) {
		wpa_printf(MSG_ERROR, "GREYLIST: %s: popen failed\n",
				__func__);
		return -1;
	}

	fgets(cmmac, sizeof(cmmac), fp);
	pclose(fp);
	if (*cmmac == '\0') {
		wpa_printf(MSG_ERROR, "GREYLIST: %s: \'%s\' failed\n",
				__func__, cmd);
		return -1;
	}
	return 0;
}

/**
 * greylist_delete_line - Delete the mac entry in memory
 */
static size_t greylist_delete_line(char *buffer, size_t size, const char *txtaddr)
{
	char *p = buffer, *q, *end = buffer + size;
	char *mac_in_buf;

	while (p < end) {
		q = p;
		while (q < end && *q != ' ') ++q; //skip date
		++q;

		while (q < end && *q != ' ') ++q; //skip time
		if (++q >= end)
			break;

		mac_in_buf = q;
		while (q < end && *q != '\n') ++q;
		++q; //q now points to beginning of next line or end

		if (os_memcmp(mac_in_buf, txtaddr, TXT_MAC_ADDR_LEN - 1) == 0) { //found
			size_t line_size = q - p;
			size_t rest_size = buffer + size -  q;

			os_memmove(p, q, rest_size);

			return size - line_size;
		}

		p = q;
	}

	return size;
}

/*
 * greylist_delete_from_file - Delete the mac entry from /nvram/greylist_mac.txt
 */
static void greylist_delete_from_file(struct hapd_interfaces *interfaces, const char *txtaddr)
{
	struct stat st;
	int fd;
	char *buffer = NULL;
	struct hostapd_iface *interface;
	struct hostapd_bss_config *conf;

	wpa_printf(MSG_DEBUG, "GREYLIST: delete %s from %s\n",
			txtaddr, greylist_file);

	interface = interfaces->iface[0];
	conf = interface->bss[0]->conf;

	fd = open(greylist_file, O_RDWR);
	if (fd < 0) {
		wpa_printf(MSG_ERROR, "GREYLIST: %s: fail to open %s\n",
				__func__, greylist_file);
		return;
	}

	if (fstat(fd, &st) != 0) {
		wpa_printf(MSG_ERROR, "GREYLIST: %s: fail to get size of %s\n",
				__func__, greylist_file);
		close(fd);
		return;
	}

	buffer = os_malloc(st.st_size);
	if (!buffer) {
		wpa_printf(MSG_ERROR,
				"GREYLIST: %s: fail to allocate buffer\n", __func__);
		close(fd);
		return;
	}

	wpa_printf(MSG_DEBUG, "GREYLIST: %s: %s: trying to get file lock\n",
			__func__, conf->iface);
	flock(fd, LOCK_EX);
	wpa_printf(MSG_DEBUG, "GREYLIST: %s: %s: got file lock\n",
			__func__, conf->iface);
	if (read(fd, buffer, st.st_size) == st.st_size) {
		size_t new_size;

		new_size = greylist_delete_line(buffer, st.st_size, txtaddr);

		if (ftruncate(fd, 0) == 0) {
			if (write(fd, buffer, new_size) != new_size) {
				wpa_printf(MSG_ERROR,
						"GREYLIST: %s: fail to write to %s\n",
						__func__, greylist_file);
			}
		}
		else {
			wpa_printf(MSG_ERROR,
					"GREYLIST: %s: fail to truncate %s\n",
					__func__, greylist_file);
		}
	}
	else {
		wpa_printf(MSG_ERROR, "GREYLIST: %s: fail to read %s\n",
				__func__, greylist_file);
	}

	flock(fd, LOCK_UN);
	wpa_printf(MSG_DEBUG, "GREYLIST: %s: %s: released file lock\n",
			__func__, conf->iface);
	close(fd);
	os_free(buffer);
}

/**
 * greylist_timeout - Timeout handler to remove client's mac from greylist
 *
 * The function is used to delete the mac entry from greylist_mac.txt and
 * delete the mac from access control list of all greylist enabled vaps.
 */
static void greylist_timeout(void *eloop_ctx, void *timeout_ctx)
{
	struct greylist_data *data = eloop_ctx;
	struct hapd_interfaces *interfaces;
	struct hostapd_iface *interface;
	struct hostapd_bss_config *conf;
	size_t i, j;
	char cmd[128];

	if (!data) {
		wpa_printf(MSG_ERROR, "GREYLIST: %s: invalid data\n",
				__func__);
		return;
	}

	wpa_printf(MSG_DEBUG, "GREYLIST: Timeout expires for client :%s\n",
			data->txtaddr);

	/* Delete the entry from /nvram/greylist_mac.txt file */
	interfaces = data->interfaces;
	greylist_delete_from_file(interfaces, data->txtaddr);

	for (i = 0; i < interfaces->count; i++) {
		interface = interfaces->iface[i];
		for (j = 0; j < interface->num_bss; j++) {
			conf = interface->bss[j]->conf;
			if (conf->rdk_greylist) {
				wpa_printf(MSG_DEBUG,
						"GREYLIST: %s: remove %s on %s\n",
						__func__,
						data->txtaddr,
						conf->iface);
				snprintf(cmd, sizeof(cmd), "wl -i %s mac del %s",
						conf->iface, data->txtaddr);
				system(cmd);
			}
		}
	}

	os_free(data);
}

/**
 * greylist_add_to_driver - Add  mac to access control list of
 * all greylist enabled vaps.
 *
 * The parameter 'txtaddr' can be a list of mac strings speparated by space.
 */
static void greylist_add_to_driver(struct hapd_interfaces *interfaces, const char *txtaddr)
{
	struct hostapd_iface *interface;
	struct hostapd_bss_config *conf;
	size_t i, j;
	char cmd[128];

	wpa_printf(MSG_DEBUG, "GREYLIST: %s: %s\n",
			__func__, txtaddr);

	for (i = 0; i < interfaces->count; i++) {
		interface = interfaces->iface[i];
		for (j = 0; j < interface->num_bss; j++) {
			conf = interface->bss[j]->conf;
			if (conf->rdk_greylist) {
				wpa_printf(MSG_DEBUG,
						"GREYLIST: %s: add %s to %s\n",
						__func__,
						txtaddr,
						conf->iface);
				snprintf(cmd, sizeof(cmd), "wl -i %s mac %s",
						conf->iface, txtaddr);
				system(cmd);
			}
		}
	}
}

/**
 * greylist_load - Read /nvram/greylist_mac.txt and handle each mac entry
 *
 * This function is used to parse /nvram/greylist_mac.txt at init time,
 * add each mac to driver's acl, and register the timeout.
 *
 * The timeout handler removes the mac from driver's acl and greylist_mac.txt.
 * The timeout value is 24 hour from the client's association time.
 */
void greylist_load(struct hapd_interfaces *interfaces)
{
	FILE *fp;
	int fd, size = 0, max_size = 0;
	char record_date[11] = {0}, record_time[9] = {0}, record_mac[TXT_MAC_ADDR_LEN] = {0};
	char *macstr_list;
	char time_buf[20] = {0};
	time_t now, t;
	struct tm time_info;
	struct greylist_data *data;
	unsigned int timeout;
	struct hostapd_iface *interface;
	struct hostapd_bss_config *conf;
	int vap_index = -1;

	wpa_printf(MSG_DEBUG, "GREYLIST: %s\n", __func__);

	if (greylist_get_cmmac() == 0) {
		wpa_printf(MSG_DEBUG, "GREYLIST: %s: cmmac=%s\n",
				__func__, cmmac);
	}

	interface = interfaces->iface[0];
	conf = interface->bss[0]->conf;

	if ((fp = fopen(greylist_file, "r")) == NULL) {
		wpa_printf(MSG_ERROR, "GREYLIST: %s: %s does not exist\n",
				__func__, greylist_file);
		return;
	}
	fd = fileno(fp);
	if (fd == -1) {
		wpa_printf(MSG_ERROR, "GREYLIST: %s: "
				"fail to get fd\n", __func__);
		fclose(fp);
		return;
	}

	/* each mac is speparated by space */
	max_size = GREYLIST_MAX_NUM_OF_RECORDS * TXT_MAC_ADDR_LEN;
	macstr_list = os_zalloc(max_size);
	if (macstr_list == NULL) {
		wpa_printf(MSG_ERROR, "GREYLIST: %s: allocate memory for macstrlist"
				"failed\n", __func__);
		fclose(fp);
		return;
	}

	wpa_printf(MSG_DEBUG,
			"GREYLIST: %s: %s: trying to get file lock\n",
			__func__, conf->iface);
	flock(fd, LOCK_SH);
	wpa_printf(MSG_DEBUG,
			"GREYLIST: %s: %s: got file lock\n",
			__func__, conf->iface);

	while ((fscanf(fp, "%s %s %s %d", record_date, record_time, record_mac, &vap_index)) == 4) {
		wpa_printf(MSG_DEBUG, "GREYLIST: %s: %s %s %s\n",
				__func__, record_date, record_time, record_mac);

		data = os_zalloc(sizeof(*data));
		if (data == NULL) {
			wpa_printf(MSG_ERROR, "GREYLIST: %s: %s: allocate memory"
				"for eloop data failed\n", __func__, record_mac);
			continue;
		}

		os_memset(&time_info, 0, sizeof(time_info));
		snprintf(time_buf, sizeof(time_buf), "%s %s", record_date, record_time);
		strptime(time_buf, "%Y-%m-%d %H:%M:%S", &time_info);
		t = mktime(&time_info);
		if (t == (time_t)-1) {
			wpa_printf(MSG_ERROR, "GREYLIST: %s: %s %s %s:"
					"time convert failed\n",
					__func__, record_date, record_time, record_mac);
			os_free(data);
			continue;
		}

		time(&now);
		if ((t > now)
			|| (now - t > GREYLIST_TIMEOUT_IN_SECONDS)) {
			timeout = 0;
		}
		else {
			timeout = GREYLIST_TIMEOUT_IN_SECONDS - (now - t);
		}

		os_memcpy(data->txtaddr, record_mac, sizeof(data->txtaddr));
		data->interfaces = interfaces;
		wpa_printf(MSG_DEBUG, "GREYLIST: %s: %s: timeout=%u\n",
				__func__, record_mac, timeout);
		eloop_register_timeout(timeout, 0, greylist_timeout, data, NULL);

		if (size == 0)
			size = snprintf(macstr_list, max_size, "%s", record_mac);
		else
			size += snprintf(macstr_list + size, max_size - size, " %s", record_mac);
	}
	flock(fd, LOCK_UN);
	wpa_printf(MSG_DEBUG,
			"GREYLIST: %s: %s: released file lock\n",
			__func__, conf->iface);

	if (size > 0)
		greylist_add_to_driver(interfaces, macstr_list);
	free(macstr_list);
	fclose(fp);
}

/**
 * greylist_add - Add client's mac to greylist
 *
 * This function is used to add a client's mac to /nvram/greylist_mac.txt
 * and acl of all greylist enabled vaps, and register a 24 hour timeout.
 * The timeout handler removes the mac from greylist.
 */

int greylist_add(struct hostapd_data *hapd, const char *txtaddr, bool fromRadiusServer)
{
	FILE *fp;
	int fd, num_of_records = 0;
	char record_date[11] = {0}, record_time[9] = {0}, record_mac[TXT_MAC_ADDR_LEN] = {0};
	char time_str[20] = {0};
	struct greylist_data *data;
	time_t now;
	struct tm *time_info;
	u32 timeout = GREYLIST_TIMEOUT_IN_SECONDS;
	struct hostapd_bss_config *conf;
	struct hapd_interfaces *interfaces;
	int vap_index = -1;

	wpa_printf(MSG_DEBUG, "GREYLIST: %s: %s\n", __func__, txtaddr);

	conf = hapd->conf;
	interfaces = hapd->iface->interfaces;

	if ((fp = fopen(greylist_file, "a+")) == NULL) {
		wpa_printf(MSG_DEBUG, "GREYLIST: %s: "
				"fail to open %s\n", __func__, greylist_file);
		return -1;
	}
	fd = fileno(fp);
	if (fd == -1) {
		wpa_printf(MSG_DEBUG, "GREYLIST: %s: "
				"fail to get fd\n", __func__);
		return -1;
	}

	/* When the client is rejected by the other hostapd instance
	 * running on the other radio, this hostapd instance will
	 * receive a message to add the client to greylist sent from
	 * the other hostapd instance.
	 *
	 * Also, in any unexpected case if we receive multiple reject messages
	 * from the server for the same client, no need to
	 * add the mac again to the file */
	wpa_printf(MSG_DEBUG, "GREYLIST: %s: %s: trying to get file lock\n",
			__func__, conf->iface);
	flock(fd, LOCK_SH);
	wpa_printf(MSG_DEBUG, "GREYLIST: %s: %s: got file lock\n",
			__func__, conf->iface);
	while ((fscanf(fp, "%s %s %s %d", record_date, record_time, record_mac, &vap_index) == 4)) {
		num_of_records++;
		if (strcmp(record_mac, txtaddr) == 0) {
			wpa_printf(MSG_ERROR, "GREYLIST: %s: "
					"%s already exists in file\n",
					__func__, txtaddr);
			fclose(fp);
			wpa_printf(MSG_DEBUG,
					"GREYLIST: %s: %s: released file lock\n",
					__func__, conf->iface);
			flock(fd, LOCK_UN);
			goto ADD_TO_DRIVER;
		}
	}
	flock(fd, LOCK_UN);
	wpa_printf(MSG_DEBUG,
			"GREYLIST: %s: %s: released file lock\n",
			__func__, conf->iface);

	if (num_of_records >= GREYLIST_MAX_NUM_OF_RECORDS) {
		wpa_printf(MSG_ERROR, "GREYLIST: %s: greylist is full\n", __func__);
		fclose(fp);
		return -1;
	}

	/* Get vap index */
	vap_index = greylist_get_vap_index(conf->iface);

	/* Get the current time and add the client mac and current time to the file */
	time(&now);
	time_info = localtime(&now);
	strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", time_info);
	wpa_printf(MSG_DEBUG, "GREYLIST: %s: %s: trying to get file lock\n",
			__func__, conf->iface);
	flock(fd, LOCK_EX);
	wpa_printf(MSG_DEBUG, "GREYLIST: %s: %s: got file lock\n",
			__func__, conf->iface);
	fseek(fp, 0, SEEK_END); //In case file position is updated by other processes
	fprintf(fp, "%s %s %d\n", time_str, txtaddr, vap_index);
	flock(fd, LOCK_UN);
	wpa_printf(MSG_DEBUG,
			"GREYLIST: %s: %s: released file lock\n",
			__func__, conf->iface);
	fclose(fp);

ADD_TO_DRIVER:
	/* TODO: check if already added to driver ? */
	greylist_add_to_driver(interfaces, txtaddr);

	data = os_zalloc(sizeof(*data));
	if (data == NULL) {
		wpa_printf(MSG_ERROR, "GREYLIST: %s: allocate memory failed\n",
				__func__);
		fclose(fp);
		return -1;
	}
	os_memcpy(data->txtaddr, txtaddr, sizeof(data->txtaddr));
	data->interfaces = interfaces;

	wpa_printf(MSG_DEBUG, "GREYLIST: %s: register %u seconds timeout for %s\n",
			__func__, timeout, txtaddr);
	eloop_register_timeout(timeout, 0,
				greylist_timeout, data, NULL);

	greylist_log_to_file("%s Client added to grey list from RADIUS: %s\n",
				time_str, txtaddr);

	/* Add to other hostapd instance running on the other radio */
	if (fromRadiusServer)
		greylist_add_to_other_hostapd(txtaddr);

	return 0;
}

/**
 * greylist_add_to_other_hostapd - Add the client to other hostapd's greylist
 *
 * This function is used to add the client mac to greylist of other hostapd
 * instance(s) running for other radio(s), so other hostapd instance(s) will
 * add the client's mac to all greylist enabled vaps controlled by it,
 * and create a 24 hour timeout to delete the mac entry from driver
 */
int greylist_add_to_other_hostapd(const char *txtaddr)
{
	FILE *fp;
	char cmd[256], pids_str[256];
	char *token, *config_file_fullname = NULL, *config_filename = NULL;
	char *pstart, *pend, *pch;
	pid_t pid, current_pid;

	current_pid = getpid();

	wpa_printf(MSG_DEBUG, "GREYLIST: %s: current pid=%d\n",
			__func__, current_pid);

	/* Get pid of all hostapd instances */
	snprintf(cmd, sizeof(cmd), "pidof hostapd");
	wpa_printf(MSG_DEBUG, "GREYLIST: %s: %s\n",
			__func__, cmd);
	fp = popen(cmd, "r");
	if (fp == NULL) {
		wpa_printf(MSG_ERROR, "GREYLIST: %s: pidof failed\n",
				__func__);
		return -1;
	}
	fgets(pids_str, sizeof(pids_str), fp);
	pclose(fp);

	token = strtok(pids_str, " \n");
	while (token != NULL) {
		pid = atoi(token);
		if (pid != current_pid) {
			wpa_printf(MSG_DEBUG, "GREYLIST: %s: pid=%s\n",
					__func__, token);

			/* get primary interface name from config filename,
			 * config file fullname is like /tmp/wl0_hapd.conf,
			 * wl0 is the primary interface name */
			snprintf(cmd, sizeof(cmd), "/proc/%s/cmdline", token);
			fp = fopen(cmd, "r");
			if (fp == NULL) {
				wpa_printf(MSG_ERROR, "GREYLIST: %s: %s failed\n",
						__func__, cmd);
				return -1;
			}
			if (fgets(cmd, sizeof(cmd), fp)) {
				pstart = cmd;
				pend = cmd + sizeof(cmd);
				while (pstart < pend) {
					if (strstr(pstart, ".conf")) {
						config_file_fullname = pstart;
						break;
					}
					else
						pstart += strlen(pstart) + 1;
				}
			}
			fclose(fp);

			if (!config_file_fullname) {
				wpa_printf(MSG_ERROR, "GREYLIST: %s: config file not specified: %s\n",
						__func__);
				return -1;
			}

			wpa_printf(MSG_DEBUG, "GREYLIST: %s: config_file=%s\n",
					__func__, config_file_fullname);
			config_filename = strrchr(config_file_fullname, '/');
			if (!config_filename)
				config_filename = config_file_fullname;
			else
				++config_filename; //skip '/'
			pch = strchr(config_filename, '_');
			if (!pch) {
				wpa_printf(MSG_ERROR, "GREYLIST: %s: unexpected config file: %s\n",
						__func__, config_file_fullname);
				return -1;
			}
			*pch = '\0'; //now config_filename only has ifname

			snprintf(cmd, sizeof(cmd),
				"hostapd_cli -i %s ADD_TO_GREYLIST %s",
				config_filename, txtaddr);
			wpa_printf(MSG_DEBUG, "GREYLIST: %s: %s\n",
					__func__, cmd);
			system(cmd);
		}
		token = strtok(NULL, " \n");
	}

	return 0;
}

/**
 * greylist_get_client_snr - Get SNR of a client
 */
u8 greylist_get_client_snr(struct hostapd_data *hapd, const char *txtaddr)
{
	FILE *fp;
	char cmd[128], buf[16];
	int rssi = 0, nf;
	u8 snr = 0;

	/* Get rssi with command 'wl rssi' */
	snprintf(cmd, sizeof(cmd), "wl -i %s rssi %s", hapd->conf->iface, txtaddr);
	wpa_printf(MSG_DEBUG, "GREYLIST: %s: %s\n",
			__func__, cmd);
	fp = popen(cmd, "r");

	if (fp == NULL) {
		wpa_printf(MSG_ERROR, "GREYLIST: %s: get rssi failed\n",
				__func__);
		return 0;
	}

	fgets(buf, sizeof(buf), fp);
	pclose(fp);
	rssi = atoi(buf);

	/* Get noise floor with command 'wl chanim_stats' */
	snprintf(cmd, sizeof(cmd),
			"wl -i %s chanim_stats | awk \'NR == 3 {print $13}\'",
			hapd->conf->iface);
	wpa_printf(MSG_DEBUG, "GREYLIST: %s: %s\n",
			__func__, cmd);
	fp = popen(cmd, "r");

	if (fp == NULL) {
		wpa_printf(MSG_ERROR, "GREYLIST: %s: get noise floor failed\n",
				__func__);
		return 0;
	}

	fgets(buf, sizeof(buf), fp);
	pclose(fp);
	nf = atoi(buf);
	snr = (u8)(rssi - nf);
	wpa_printf(MSG_DEBUG, "GREYLIST: %s: txtaddr=%s rssi=%d nf=%d snr=%u\n",
			__func__, txtaddr, rssi, nf, snr);
	return snr;
}
