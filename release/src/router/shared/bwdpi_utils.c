#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <ctype.h>
#include <bcmnvram.h>
#include <time.h>

#include "shutils.h"
#include "shared.h"

#if defined(RTCONFIG_BWDPI)
/*
	WRS (web reputation system) checking
	return:
	0 : no funtion enabled
	1 : as least one function enabled
*/
int check_wrs_switch()
{
	int FULL = nvram_get_int("wrs_protect_enable");
	int MALS = nvram_get_int("wrs_mals_enable");
	int VP = nvram_get_int("wrs_vp_enable");
	int CC = nvram_get_int("wrs_cc_enable");

	if ((FULL & MALS) == 0 && (FULL & VP) == 0 && (FULL & CC) == 0)
		return 0;
	else
		return 1;
}

/*
	usage in rc or bwdpi for checking service
	return:
	0 : dpi engine disabled
	1 : dpi engine enabled
*/
int check_bwdpi_nvram_setting()
{
	int enabled = 1;
	int debug = nvram_get_int("bwdpi_debug");

	// check no qos service
	if (check_wrs_switch() == 0 &&
		nvram_get_int("wrs_enable") == 0 &&
		nvram_get_int("wrs_app_enable") == 0 &&
		nvram_get_int("bwdpi_db_enable") == 0 &&
		nvram_get_int("apps_analysis") == 0 &&
		nvram_get_int("bwdpi_wh_enable") == 0 &&
		nvram_get_int("qos_enable") == 0)
		enabled = 0;

	// check qos service (not adaptive qos)
	if (check_wrs_switch() == 0 &&
		nvram_get_int("wrs_enable") == 0 &&
		nvram_get_int("wrs_app_enable") == 0 &&
		nvram_get_int("bwdpi_db_enable") == 0 &&
		nvram_get_int("apps_analysis") == 0 &&
		nvram_get_int("bwdpi_wh_enable") == 0 &&
		IS_NON_AQOS())
		enabled = 0;

	if (debug) dbg("[check_bwdpi_nvram_setting] enabled= %d\n", enabled);

	return enabled;
}

int check_wan_2P5G_10G_speed()
{
	int ret = 0;
	char wan_if[8] = {0};
	char wan_if_ppp[8] = {0};
	char wan_proto[8] = {0};
	char wan_speed[64] = {0};
	char buf[64] = {0};
	char tmp[64] = {0};
	char prefix[sizeof("wanX_XXXXXXX")];
	int speed = 0;
	int debug = nvram_get_int("bwdpi_debug");

	// ethX, pppX
	strlcpy(wan_if, get_wan_ifname(wan_primary_ifunit()), sizeof(wan_if));

	// ethX
	snprintf(prefix, sizeof(prefix), "wan%d_", wan_primary_ifunit());
	snprintf(wan_proto, sizeof(wan_proto), "%s", nvram_safe_get(strcat_r(prefix, "proto", tmp)));
	snprintf(wan_if_ppp, sizeof(wan_if_ppp), "%s", nvram_safe_get(strcat_r(prefix, "ifname", tmp)));

	// fix ppp case issue due to pppX can't get speed
	if (!strcmp(wan_proto, "pppoe") || !strcmp(wan_proto, "pptp") || !strcmp(wan_proto, "l2tp")) {
		snprintf(wan_speed, sizeof(wan_speed), "/sys/class/net/%s/speed", wan_if_ppp);
	}
	else {
		snprintf(wan_speed, sizeof(wan_speed), "/sys/class/net/%s/speed", wan_if);
	}

	if (f_read_string(wan_speed, buf, sizeof(buf)) > 0) {
		speed = safe_atoi(buf);
	}

	if (debug) dbg("[check_wan_2P5G_10G_speed] wan_speed=%s, speed=%d\n", wan_speed, speed);
	if (debug) logmessage("BWDPI", "[check_wan_2P5G_10G_speed] wan_speed=%s, speed=%d", wan_speed, speed);

	if (speed >= 2500) ret = 1;

	return ret;
}

int check_AQoS_only_enabled()
{
	int ret = 0;
	int debug = nvram_get_int("bwdpi_debug");

	// check qos service (adaptive qos only)
	if (check_wrs_switch() == 0 &&
		nvram_get_int("wrs_enable") == 0 &&
		nvram_get_int("wrs_app_enable") == 0 &&
		nvram_get_int("bwdpi_db_enable") == 0 &&
		nvram_get_int("apps_analysis") == 0 &&
		nvram_get_int("bwdpi_wh_enable") == 0 &&
		IS_AQOS()) {
		ret = 1;
	}

	if (debug) dbg("[check_AQoS_only_enabled] ret=%d\n", ret);
	if (debug) logmessage("BWDPI", "[check_AQoS_only_enabled] ret=%d", ret);

	return ret;
}

int check_WRS_only_enabled()
{
	/* QCA no need to check */
	int ret = 0;
#if !defined(RTCONFIG_QCA)
	int debug = nvram_get_int("bwdpi_debug");

	// check wrs service only - No QoS, other should be disabled
	if ((check_wrs_switch() == 1 || nvram_get_int("wrs_enable") == 1
		|| nvram_get_int("wrs_app_enable") == 1 || nvram_get_int("bwdpi_wh_enable") == 1)
		&& nvram_get_int("bwdpi_db_enable") == 0
		&& nvram_get_int("apps_analysis") == 0
		&& nvram_get_int("qos_enable") == 0) {
		ret = 1;
	}

#if 0
	// check wrs service only - Non-A.QoS, ignore this case due to they all need to disable fc / runner
	if ((check_wrs_switch() == 1 || nvram_get_int("wrs_enable") == 1
		|| nvram_get_int("wrs_app_enable") == 1 || nvram_get_int("bwdpi_wh_enable") == 1)
		&& nvram_get_int("bwdpi_db_enable") == 0
		&& nvram_get_int("apps_analysis") == 0
		&& IS_NON_AQOS()) {
		ret = 1;
	}
#endif

	if (debug) dbg("[check_WRS_only_enabled] ret=%d\n", ret);
	if (debug) logmessage("BWDPI", "[check_WRS_only_enabled] ret=%d", ret);
#endif

	return ret;
}

void disable_dpi_engine_setting(void)
{
	nvram_set_int("wrs_protect_enable", 0);
	nvram_set_int("wrs_enable", 0);
	nvram_set_int("wrs_app_enable", 0);
	nvram_set_int("bwdpi_db_enable", 0);
	nvram_set_int("apps_analysis", 0);
	nvram_set_int("bwdpi_wh_enable", 0);

	// only for adaptive qos
	if (IS_AQOS())
		nvram_set_int("qos_enable", 0);

	nvram_commit();
}
#endif

/*
	erase_symbol : erase some specific symbol from string
	ex. 
	MAC=00:11:22:33:44:55 
	After using erase_symbol(MAC, ":")
	MAC=001122334455

	old : mac format
	sym : symbol

*/
void erase_symbol(char *old, char *sym)
{
	char buf[20];
	int strLen;

	char *FindPos = strstr(old, sym);
	if ((!FindPos) || (!sym)) {
		return;
	}

	// add protection of mac size
	if (strlen(old) > (sizeof(buf) - 1)) {
		return;
	}

	while (FindPos != NULL) {
		//dbg("FindPos=%s, old=%s\n", FindPos, old);
		memset(buf, 0, sizeof(buf));
		strLen = FindPos - old;
		strncpy(buf, old, strLen);
		strlcat(buf, FindPos+1, sizeof(buf));
		strcpy(old, buf);
		FindPos = strstr(old, sym);
	}
	
	//dbg("macaddr=%s\n", old);
}


/*
	transfer timestamp into date
	ex. date = 2014-07-14 19:20:10
*/
void StampToDate(unsigned long timestamp, char *date)
{
	struct tm *local;
	time_t now;
	
	now = timestamp;
	local = localtime(&now);
	strftime(date, 30, "%Y-%m-%d %H:%M:%S", local);
}

/*
	check filesize is over or not
	if over size, return 1, else return 0
	size : KB
*/
int check_filesize_over(char *path, long int size)
{
	struct stat st;
	off_t cursize;

	stat(path, &st);
	cursize = st.st_size;

	size = size * 1024; // KB

	if(cursize > size)
		return 1;
	else
		return 0;
}

/*
	get last month's timestamp
	ex.
	now = 1445817600
	tm  = 2015/10/26 00:00:00
	t   = 2015/10/01 00:00:00
	t_t = 1443628800
*/
time_t get_last_month_timestamp()
{
	struct tm local, t;
	time_t now, t_t = 0;
			
	// get timestamp and tm
	time(&now);
	localtime_r(&now, &local);

	// copy t from local
	t.tm_year = local.tm_year;
	t.tm_mon = local.tm_mon;
	t.tm_mday = 1;
	t.tm_hour = 0;
	t.tm_min = 0;
	t.tm_sec = 0;

	// transfer tm to timestamp
	t_t = mktime(&t);

	return t_t;
}
