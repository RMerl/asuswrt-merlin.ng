/*
 * bsd deamon (Linux)
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
 * $Id: bsd_main.c $
 */
#include "bsd.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <common_utils.h>

/* policy extension flag usage */
typedef struct bsd_flag_description_ {
	uint32	flag;
	char	*descr;
} bsd_flag_description_t;

static bsd_flag_description_t bsd_streering_flag_descr[] = {
	{BSD_STEERING_POLICY_FLAG_RULE, "BSD_STEERING_POLICY_FLAG_RULE"},
	{BSD_STEERING_POLICY_FLAG_RSSI, "BSD_STEERING_POLICY_FLAG_RSSI"},
	{BSD_STEERING_POLICY_FLAG_VHT, "BSD_STEERING_POLICY_FLAG_VHT"},
	{BSD_STEERING_POLICY_FLAG_NON_VHT, "BSD_STEERING_POLICY_FLAG_NON_VHT"},
	{BSD_STEERING_POLICY_FLAG_NEXT_RF, "BSD_STEERING_POLICY_FLAG_NEXT_RF"},
	{BSD_STEERING_POLICY_FLAG_PHYRATE, "BSD_STEERING_POLICY_FLAG_PHYRATE"},
	{BSD_STEERING_POLICY_FLAG_LOAD_BAL, "BSD_STEERING_POLICY_FLAG_LOAD_BAL"},
	{BSD_STEERING_POLICY_FLAG_STA_NUM_BAL, "BSD_STEERING_POLICY_FLAG_STA_NUM_BAL"},
	{BSD_STEERING_POLICY_FLAG_CHAN_OVERSUB, "BSD_STEERING_POLICY_FLAG_CHAN_OVERSUB"},
	{0, ""},
};

static bsd_flag_description_t bsd_sta_select_flag_descr[] = {
	{BSD_STA_SELECT_POLICY_FLAG_RULE, "BSD_STA_SELECT_POLICY_FLAG_RULE"},
	{BSD_STA_SELECT_POLICY_FLAG_RSSI, "BSD_STA_SELECT_POLICY_FLAG_RSSI"},
	{BSD_STA_SELECT_POLICY_FLAG_VHT, "BSD_STA_SELECT_POLICY_FLAG_VHT"},
	{BSD_STA_SELECT_POLICY_FLAG_NON_VHT, "BSD_STA_SELECT_POLICY_FLAG_NON_VHT"},
	{BSD_STA_SELECT_POLICY_FLAG_NEXT_RF, "BSD_STA_SELECT_POLICY_FLAG_NEXT_RF"},
	{BSD_STA_SELECT_POLICY_FLAG_PHYRATE, "BSD_STA_SELECT_POLICY_FLAG_PHYRATE"},
	{BSD_STA_SELECT_POLICY_FLAG_LOAD_BAL, "BSD_STA_SELECT_POLICY_FLAG_LOAD_BAL"},
	{BSD_STA_SELECT_POLICY_FLAG_SINGLEBAND, "BSD_STA_SELECT_POLICY_FLAG_SINGLEBAND"},
	{BSD_STA_SELECT_POLICY_FLAG_DUALBAND, "BSD_STA_SELECT_POLICY_FLAG_DUALBAND"},
	{BSD_STA_SELECT_POLICY_FLAG_ACTIVE_STA, "BSD_STA_SELECT_POLICY_FLAG_ACTIVE_STA"},
	{0, ""},
};

static bsd_flag_description_t bsd_if_qualify_flag_descr[] = {
	{BSD_QUALIFY_POLICY_FLAG_RULE, "BSD_QUALIFY_POLICY_FLAG_RULE"},
	{BSD_QUALIFY_POLICY_FLAG_VHT, "BSD_QUALIFY_POLICY_FLAG_VHT"},
	{BSD_QUALIFY_POLICY_FLAG_NON_VHT, "BSD_QUALIFY_POLICY_FLAG_NON_VHT"},
	{BSD_QUALIFY_POLICY_FLAG_PHYRATE, "BSD_QUALIFY_POLICY_FLAG_PHYRATE"},
	{BSD_QUALIFY_POLICY_FLAG_LOAD_BAL, "BSD_QUALIFY_POLICY_FLAG_LOAD_BAL"},
	{BSD_QUALIFY_POLICY_FLAG_STA_BAL, "BSD_QUALIFY_POLICY_FLAG_STA_BAL"},
	{0, ""},
};

static bsd_flag_description_t bsd_debug_flag_descr[] = {
	{BSD_DEBUG_ERROR, "BSD_DEBUG_ERROR"},
	{BSD_DEBUG_WARNING, "BSD_DEBUG_WARNING"},
	{BSD_DEBUG_INFO, "BSD_DEBUG_INFO"},
	{BSD_DEBUG_TO, "BSD_DEBUG_TO"},
	{BSD_DEBUG_STEER, "BSD_DEBUG_STEER"},
	{BSD_DEBUG_EVENT, "BSD_DEBUG_EVENT"},
	{BSD_DEBUG_HISTO, "BSD_DEBUG_HISTO"},
	{BSD_DEBUG_CCA, "BSD_DEBUG_CCA"},
	{BSD_DEBUG_AT, "BSD_DEBUG_AT"},
	{BSD_DEBUG_RPC, "BSD_DEBUG_RPC"},
	{BSD_DEBUG_RPCD, "BSD_DEBUG_RPCD"},
	{BSD_DEBUG_RPCEVT, "BSD_DEBUG_RPCEVT"},
	{BSD_DEBUG_MULTI_RF, "BSD_DEBUG_MULTI_RF"},
	{BSD_DEBUG_BOUNCE, "BSD_DEBUG_BOUNCE"},
	{BSD_DEBUG_DUMP, "BSD_DEBUG_DUMP"},
	{BSD_DEBUG_PROBE, "BSD_DEBUG_PROBE"},
	{BSD_DEBUG_ALL, "BSD_DEBUG_ALL"},
	{0, ""},
};

static void bsd_describe_flag(bsd_flag_description_t *descr)
{
	while (descr->flag != 0) {
		printf("%35s\t0x%08x\n", descr->descr, descr->flag);
		descr++;
	}
}

static void bsd_usage(void)
{
	printf("wlx[.y]_bsd_steering_policy=<bw util percentage> <sample period> "
		"<consecutive sample count> <rssi  threshold> "
		"<phy rate threshold> <extension flag>\n");

	bsd_describe_flag(bsd_streering_flag_descr);

	printf("\nwlx[.y]_bsd_sta_select_policy=<idle_rate> <rssi> <phy rate> "
		"<wprio> <wrssi> <wphy_rate> <wtx_failures> <wtx_rate> <wrx_rate> "
		"<extension_flag>\n");
	bsd_describe_flag(bsd_sta_select_flag_descr);

	printf("\nwlx[.y]_bsd_if_qualify_policy=<bw util percentage> "
		"<extension_flag>\n");
	bsd_describe_flag(bsd_if_qualify_flag_descr);

	printf("\nband steering debug flags\n");
	bsd_describe_flag(bsd_debug_flag_descr);

	printf("\n bsd command line options:\n");
	printf("-f\n");
	printf("-F keep bsd on the foreground\n");
	printf("-i show bsd config info\n");
	printf("-s show bsd sta info\n");
	printf("-l show bsd steer log\n");
	printf("-S [MAC] poll bsd sta stats\n");
	printf("-r poll bsd radio stats\n");
	printf("-t change tty console\n");
	printf("-h\n");
	printf("-H this help usage\n");
	printf("\n");

}

bsd_info_t *bsd_info;
bsd_intf_info_t *bsd_intf_info;
int bsd_msglevel = BSD_DEBUG_ERROR;

static bsd_info_t *bsd_info_alloc(void)
{
	bsd_info_t *info;

	BSD_ENTER();

	info = (bsd_info_t *)malloc(sizeof(bsd_info_t));
	if (info == NULL) {
		BSD_PRINT("malloc fails\n");
	}
	else {
		memset(info, 0, sizeof(bsd_info_t));
		BSD_INFO("info=%p\n", info);
	}

	BSD_EXIT();
	return info;
}

static int
bsd_init(bsd_info_t *info)
{
	int err = BSD_FAIL;
	char *str, *endptr = NULL;
	char tmp[16];

	BSD_ENTER();

	err = bsd_intf_info_init(info);
	if (err != BSD_OK) {
		return err;
	}

	info->version = BSD_VERSION;
	info->event_fd = BSD_DFLT_FD;
	info->event_fd2 = BSD_DFLT_FD;
	info->rpc_listenfd  = BSD_DFLT_FD;
	info->rpc_eventfd = BSD_DFLT_FD;
	info->rpc_ioctlfd = BSD_DFLT_FD;
	info->poll_interval = BSD_POLL_INTERVAL;
	info->mode = BSD_MODE_STEER;
	info->role = BSD_ROLE_STANDALONE;
	info->status_poll = BSD_STATUS_POLL_INTV;
	info->counter_poll = BSD_COUNTER_POLL_INTV;
	info->idle_rate = 10;

	if ((str = nvram_get("bsd_role"))) {
		info->role = (uint8)strtol(str, &endptr, 0);
		if (info->role >= BSD_ROLE_MAX) {
			BSD_ERROR("Err: bsd_role[%s] default to Standalone.\n", str);
			info->role = BSD_ROLE_STANDALONE;
			sprintf(tmp, "%d", info->role);
			nvram_set("bsd_role", tmp);
		}
#ifdef BCM_WBD
		/* If the BSD role is none and WBD is enabled make it as standalone */
		if ((info->role == BSD_ROLE_NONE) &&
			(info->enable_flag & BSD_FLAG_WBD_ENABLED)) {
			info->role = BSD_ROLE_STANDALONE;
			BSD_WBD("info->role[%d]\n", info->role);
		}
#endif /* BCM_WBD */
	}

	if ((str = nvram_get("bsd_helper"))) {
		BSDSTRNCPY(info->helper_addr, str, sizeof(info->helper_addr) - 1);
	}
	else {
		strcpy(info->helper_addr, BSD_DEFT_HELPER_ADDR);
		nvram_set("bsd_helper", BSD_DEFT_HELPER_ADDR);
	}

	info->hport = HELPER_PORT;
	if ((str = nvram_get("bsd_hport"))) {
		info->hport = (uint16)strtol(str, &endptr, 0);
	} else {
		sprintf(tmp, "%d", info->hport);
		nvram_set("bsd_hport", tmp);
	}

	if ((str = nvram_get("bsd_primary"))) {
		BSDSTRNCPY(info->primary_addr, str, sizeof(info->primary_addr) - 1);
	}
	else {
		strcpy(info->primary_addr, BSD_DEFT_PRIMARY_ADDR);
		nvram_set("bsd_primary", BSD_DEFT_PRIMARY_ADDR);
	}

	info->pport = PRIMARY_PORT;
	if ((str = nvram_get("bsd_pport"))) {
		info->pport = (uint16)strtol(str, &endptr, 0);
	}
	else {
		sprintf(tmp, "%d", info->pport);
		nvram_set("bsd_pport", tmp);
	}

	BSD_INFO("role:%d helper:%s[%d] primary:%s[%d]\n",
		info->role, info->helper_addr, info->hport,
		info->primary_addr, info->pport);

	info->scheme = BSD_SCHEME;
	if ((str = nvram_get("bsd_scheme"))) {
		info->scheme = (uint8)strtol(str, &endptr, 0);
		if (info->scheme >= bsd_get_max_scheme(info))
			info->scheme = BSD_SCHEME;
	}
	BSD_INFO("scheme:%d\n", info->scheme);

	err = bsd_info_init(info);
	if (err == BSD_OK) {
		bsd_retrieve_config(info);
		err = bsd_open_eventfd(info);
		if (err == BSD_OK)
			err = bsd_open_rpc_eventfd(info);
		if (err == BSD_OK)
			err = bsd_open_server_cli_fd(info);
	}

	BSD_EXIT();
	return err;
}

static void
bsd_cleanup(bsd_info_t*info)
{
	if (info) {
		bsd_close_eventfd(info);
		bsd_close_rpc_eventfd(info);
		bsd_close_socket(&(info->cli_listenfd));
		bsd_bssinfo_cleanup(info);
		bsd_cleanup_sta_bounce_table(info);

		if (info->intf_info != NULL) {
			free(info->intf_info);
		}

#ifdef BCM_WBD
		/* Cleanup WBD info */
		bsd_cleanup_wbd(info->wbd_info);
#endif /* BCM_WBD */

		free(info);
	}
}

static void
bsd_watchdog(bsd_info_t*info, uint ticks)
{

	BSD_ENTER();

	BSD_TO("\nticks[%d] [%lu]\n", ticks, (unsigned long)time(NULL));

	if ((info->enable_flag & BSD_FLAG_ENABLED) &&
		(info->role != BSD_ROLE_PRIMARY) &&
		(info->role != BSD_ROLE_STANDALONE)) {
		BSD_TO("no Watchdog operation fro helper...\n");
		BSD_EXIT();
		return;
	}

	if ((info->counter_poll != 0) && (ticks % info->counter_poll == 1)) {
		BSD_TO("bsd_update_counters [%d] ...\n", info->counter_poll);
		bsd_update_stb_info(info);
	}

	if ((info->status_poll != 0) && (ticks % info->status_poll == 1)) {
		BSD_TO("bsd_update_stainfo [%d] ...\n", info->status_poll);
		bsd_update_stainfo(info);
	}

	if ((info->status_poll != 0) && (ticks % info->status_poll == 0)) {
		bsd_update_sta_bounce_table(info);
	}

	bsd_update_cca_stats(info);

	/* Only if BSD is enabled */
	if (info->enable_flag & BSD_FLAG_ENABLED) {
		/* use same poll interval with stainfo */
		if ((info->status_poll != 0) && (ticks % info->status_poll == 1)) {
			BSD_TO("bsd_check_steer [%d] ...\n", info->status_poll);
			bsd_check_steer(info);
		}
	}
#ifdef BCM_WBD
	/* if WBD is enabled */
	if (info->enable_flag & BSD_FLAG_WBD_ENABLED) {
		/* use same poll interval with stainfo */
		if ((info->status_poll != 0) && (ticks % info->status_poll == 1)) {
			/* Update WBD related information */
			bsd_wbd_reinit(info);
			/* Check and Inform weak STAs to WBD */
			bsd_wbd_check_weak_sta(info);
		}
	}
#endif /* BCM_WBD */

	if ((info->probe_timeout != 0) && (ticks % info->probe_timeout == 0)) {
		BSD_TO("bsd_timeout_prbsta [%d] ...\n", info->probe_timeout);
		bsd_timeout_prbsta(info);
	}

	/* Only if BSD is enabled */
	if (info->enable_flag & BSD_FLAG_ENABLED) {
		if ((info->maclist_timeout != 0) && (ticks % info->maclist_timeout == 0)) {
			BSD_TO("bsd_timeout_maclist [%d] ...\n", info->maclist_timeout);
			bsd_timeout_maclist(info);
		}
	}

	if ((info->sta_timeout != 0) &&(ticks % info->sta_timeout == 0)) {
		BSD_TO("bsd_timeout_sta [%d] ...\n", info->sta_timeout);
		bsd_timeout_sta(info);
	}

	BSD_EXIT();
}

static void bsd_hdlr(int sig)
{
	bsd_info->mode = BSD_MODE_DISABLE;
	return;
}

void bsd_info_hdlr(void)
{
	bsd_dump_config_info(bsd_info);
	return;
}

void bsd_log_hdlr(void)
{
	bsd_steering_record_display();
	return;
}

void bsd_sta_hdlr(void)
{
	bsd_dump_sta_info(bsd_info);
	return;
}

void bsd_query_sta_stats_hdlr(void)
{
	bsd_query_sta_stats(bsd_info);
	return;
}

void bsd_query_radio_stats_hdlr(void)
{
	bsd_query_radio_stats(bsd_info);
	return;
}

static pid_t get_bsd_pid(void)
{
	pid_t pid;

	if ((pid = get_pid_by_name("/usr/sbin/bsd")) <= 0) {
		if ((pid = get_pid_by_name("/usr/bin/bsd")) <= 0) {
			if ((pid = get_pid_by_name("/bin/bsd")) <= 0) {
				if ((pid = get_pid_by_name("bsd")) <= 0) {
					printf("BSD is not running\n");
				}
			}
		}
	}
	return pid;
}

void bsd_tty_hdlr(void)
{
	char tty[32] = "";
	FILE* ff;
	int fd;

	if ((ff = fopen(BSD_OUTPUT_FILE_TTY, "r")) == NULL) {
		BSD_ERROR("Err: failed to open tty file!\n");
		return;
	}

	fgets(tty, 32, ff);
	fclose(ff);

	if (tty[0] == '\0') {
		printf("Err: tty file is empty!\n");
		return;
	}

	if ((fd = open(tty, O_RDWR, 0644)) == -1) {
		BSD_ERROR("Err: failed to open tty %s!\n", tty);
		return;
	}

	if (dup2(fd, STDOUT_FILENO) != -1)
		printf("Switched stdout to %s.\n", tty);
	else
		printf("Failed to switch stdout to %s!\n", tty);

	close(fd);

	return;
}

void bsd_sta_config_hdlr(void)
{
	bsd_staprio_config_t *staprio, *next_staprio;

	/* cleanup staprio list */
	BSD_INFO("Clean STA Config \n");
	staprio = bsd_info->staprio;
	while (staprio) {
		BSD_INFO("staprio: "MACF"\n", ETHER_TO_MACF(staprio->addr));
		next_staprio = staprio->next;
		free(staprio);
		staprio = next_staprio;
	}
	bsd_info->staprio = NULL;

	/* retrieve staprio */
	BSD_INFO("Update STA Config \n");
	bsd_retrieve_staprio_config(bsd_info);
	bsd_set_staprio(bsd_info);

	return;
}

/* mac string format "%02x:%02x:%02x:%02x:%02x:%02x" */
static bool is_mac_str(char *ptr)
{
	int i, len;

	if ((len = strlen(ptr)) != 17)
		return FALSE;

	for (i = 0; i < len; i++) {
		if ((i % 3 != 2) && !isxdigit(ptr[i]))
			return FALSE;
		if ((i % 3 == 2) && ptr[i] != ':')
			return FALSE;
	}
	if (ptr[len] != '\0')
		return FALSE;

	return TRUE;
}

/* service main entry */
int main(int argc, char *argv[])
{
	int err = BSD_OK;
	struct timeval tv;
	char *val;
	int role, flag = 0;
	int c;
	bool foreground = FALSE;
	pid_t pid;
	char filename[128];
	char cmd[128];
	uint8 cmd_id;
	struct stat buffer;
	int wait_time = 0;

	if (argc > 1) {
		while ((c = getopt(argc, argv, "chHfFilsSrt")) != -1) {
			switch (c) {
				case 'f':
				case 'F':
					foreground = TRUE;
					break;
				case 'c':
				case 'i':
				case 'l':
				case 's':
				case 't':
					/* for both bsd -i/-l/-s (info/log/sta) */
					if ((pid = get_bsd_pid()) <= 0) {
						return BSD_FAIL;
					}

					if (c == 'c') {
						cmd_id = BSD_CMD_STA_CONFIG_HDLR;
						bsd_open_cli_fd(cmd_id);
						return BSD_OK;
					}
					else if (c == 'i') {
						snprintf(filename, sizeof(filename), "%s",
							BSD_OUTPUT_FILE_INFO);
						cmd_id = BSD_CMD_CONFIG_INFO;
					}
					else if (c == 's') {
						snprintf(filename, sizeof(filename), "%s",
							BSD_OUTPUT_FILE_STA);
						cmd_id = BSD_CMD_STA_INFO;

					}
					else if (c == 't') {
						cmd_id = BSD_CMD_CHANGE_TTY_CONSOLE;
						FILE *ff = fopen(BSD_OUTPUT_FILE_TTY, "w+");
						if (!ff) {
							BSD_ERROR("Failed to open tty file w+!\n");
							return BSD_FAIL;
						}
						fprintf(ff, "%s", ttyname(0));
						fclose(ff);
						bsd_open_cli_fd(cmd_id);
						return BSD_OK;

					}
					else {
						snprintf(filename, sizeof(filename), "%s",
							BSD_OUTPUT_FILE_LOG);
						cmd_id = BSD_CMD_STEER_LOG;
					}

					unlink(filename);
					bsd_open_cli_fd(cmd_id);

					while (1) {
						usleep(BSD_OUTPUT_FILE_INTERVAL);
						if (stat(filename, &buffer) == 0) {
							snprintf(cmd, sizeof(cmd), "cat %s",
								filename);
							system(cmd);
							return BSD_OK;
						}
						wait_time += BSD_OUTPUT_FILE_INTERVAL;
						if (wait_time >= BSD_OUTPUT_FILE_TIMEOUT)
							break;
					}

					printf("BSD: info not ready\n");
					return BSD_FAIL;
				case 'S':
				case 'r':
					/* for both bsd -S/-r (STA/Radio stats to appeventd) */
					if ((pid = get_bsd_pid()) <= 0) {
						return BSD_FAIL;
					}

					if (c == 'S') {
						/* format bsd -S [MAC], w/o [MAC] means all */
						FILE *fp;

						fp = fopen(BSD_APPEVENT_STA_MAC, "w+");
						if (!fp) {
							BSD_ERROR("Failed to open sta mac file\n");
							return BSD_FAIL;
						}

						cmd_id = BSD_CMD_STA_STATS;
						if (argc >= 3) {
							if (is_mac_str(argv[2]) == TRUE) {
								/* only allow MAC addr */
								fprintf(fp, "%s", argv[2]);
							}
							else {
								BSD_ERROR("Err: wrong MAC %s\n",
									argv[2]);
								fclose(fp);
								return BSD_FAIL;
							}
						}
						else {
							fprintf(fp, "%s", "");
						}
						fclose(fp);
					}
					else {
						cmd_id = BSD_CMD_RADIO_STATS;
					}

					bsd_open_cli_fd(cmd_id);
					return BSD_OK;

				case 'h':
				case 'H':
					bsd_usage();
					break;
				default:
					printf("%s invalid option\n", argv[0]);
			}
		}

		if (foreground == FALSE) {
			exit(0);
		}
	}

#ifdef BCM_WBD
	/* Get WBD mode */
	char *wbd_val = nvram_safe_get(BSD_WBD_NVRAM_MAP_MODE);
	int map_mode = strtoul(wbd_val, NULL, 0);
	if (!BSD_WBD_DISABLED(map_mode)) {
		flag |= BSD_FLAG_WBD_ENABLED;
	} else {
		printf("WBD(%s) is not enabled: %s=%d\n", BSD_WBD_NVRAM_MAP_MODE,
			wbd_val, map_mode);
	}
#endif /* BCM_WBD */

	val = nvram_safe_get("bsd_role");
	role = strtoul(val, NULL, 0);
	if ((role != BSD_ROLE_PRIMARY) &&
		(role != BSD_ROLE_HELPER) &&
		(role != BSD_ROLE_STANDALONE)) {
		printf("BSD is not enabled: %s=%d\n", val, role);
	} else {
		flag |= BSD_FLAG_ENABLED;
	}

	if (((flag & BSD_FLAG_ENABLED) == 0) &&
		((flag & BSD_FLAG_WBD_ENABLED) == 0)) {
		printf("Both BSD and WBD is Disabled. flag=%d\n", flag);
		goto done;
	}

	val = nvram_safe_get("bsd_msglevel");
	if (strcmp(val, ""))
		bsd_msglevel = strtoul(val, NULL, 0);

	BSD_INFO("bsd start...\n");

#if !defined(DEBUG)
	if (foreground == FALSE) {
		if (daemon(1, 1) == -1) {
			BSD_ERROR("err from daemonize.\n");
			goto done;
		}
	}
#endif // endif

	if ((bsd_info = bsd_info_alloc()) == NULL) {
		printf("BSD alloc fails. Aborting...\n");
		goto done;
	}
	/* Initialize the flag */
	bsd_info->enable_flag = flag;

	if (bsd_init(bsd_info) != BSD_OK) {
		printf("BSD Aborting...\n");
		goto done;
	}

#ifdef BCM_WBD
	if (bsd_wbd_init(bsd_info) != BSD_OK) {
		printf("WBD init failed BSD Aborting...\n");
		goto done;
	}
#endif /* BCM_WBD */
	/* Provide necessary info to debug_monitor for service restart */
	dm_register_app_restart_info(getpid(), argc, argv, NULL);
	tv.tv_sec = bsd_info->poll_interval;
	tv.tv_usec = 0;

	/* BSD is initialized properly. Now enable signal handlers */
	signal(SIGTERM, bsd_hdlr);

	while (bsd_info->mode != BSD_MODE_DISABLE) {

		if (tv.tv_sec == 0 && tv.tv_usec == 0) {
			bsd_info->ticks ++;
			tv.tv_sec = bsd_info->poll_interval;
			tv.tv_usec = 0;
			BSD_INFO("ticks: %d\n", bsd_info->ticks);

			bsd_watchdog(bsd_info, bsd_info->ticks);

			val = nvram_safe_get("bsd_msglevel");
			if (strcmp(val, ""))
				bsd_msglevel = strtoul(val, NULL, 0);
		}
		bsd_proc_socket(bsd_info, &tv);
	}

done:
	bsd_cleanup(bsd_info);
	return err;
}
