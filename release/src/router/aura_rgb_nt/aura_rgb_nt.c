 /*
 * Copyright 2019, ASUSTeK Inc.
 * All Rights Reserved.
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */
/* header */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

#include <aura_rgb.h>
#include <shared.h>
#include <shutils.h>
#include <aura_rgb_nt.h>

#define AURA_LED_DEFAULT     "255,0,0,1,0,0" //Default
#define ALARM_PERIOD_T 6

struct aura_event {
	char *event_name;
	char *rgb_val;
	int period;
	int btn_event;
};

static struct aura_event aura_events[] = {
	/*{"event name", "aurargb_val", "period", btn_event}*/
	{ "BOOST_GAME_BOOST_SW", "255,0,0,2,-2,0", 6, 1},
	{ "BOOST_ACS_DFS_SW", "255,255,0,7,0,0", 6, 1},
	{ "BOOST_GEFORCENOW", "0,255,0,7,0,0", 6, 1},
	{ "AttackBlocked", "255,0,0,7,0,0", 6, 0},
	{ "LoginFail", "255,0,0,7,0,0", 6, 0},
	{ NULL, NULL, 0, 0}
};

static int AttackBlocked_count = 0;
static int AttackBlocked_period = 0;
static int traffic_event_isfirst = 1;
static int bwdpi_event_isfirst = 1;
static time_t traffic_timestamp = 0;
static unsigned long long traffic_last_rx = 0;
static int btn_aura_event_trigger = 0;
static int boot_check_rgb = 0;

void aura_rgb_control(char *rgb)
{
	RGB_LED_STATUS_T rgb_cfg = { 0 };

	__nv_to_rgb(rgb, &rgb_cfg);
	aura_rgb_led(ROUTER_AURA_SET, &rgb_cfg, 0, 0);
}

void restore_aura_rgb(void)
{
	AURA_NT_DBG("restore_aura_rgb\n");
	RGB_LED_STATUS_T rgb_cfg = { 0 };

	if(inhibit_led_on() || !nvram_get_int("aurargb_enable"))
	{
		memset(&rgb_cfg, 0x00, sizeof(rgb_cfg));
		aura_rgb_led(ROUTER_AURA_SET, &rgb_cfg, 0, 0);
	}
	else if(nv_to_rgb("aurargb_val", &rgb_cfg) == 0)
		aura_rgb_led(ROUTER_AURA_SET, &rgb_cfg, 0, 0);
	else
		aura_rgb_control(AURA_LED_DEFAULT);
}

int get_aurargb_mode(void)
{
	RGB_LED_STATUS_T rgb_cfg = { 0 };
	if(nv_to_rgb("aurargb_val", &rgb_cfg) == 0)
		return rgb_cfg.mode;
	else
		return -1;
}

int update_rgb_led(char *aura_event)
{
	struct aura_event *aura_event_t;

	if(aura_event == NULL) return 0;

	if(nvram_get_int("pause_aura_rgb_nt") == 1){
		AURA_NT_DBG("pause_aura_rgb_nt\n");
		nvram_set("pause_aura_rgb_sw", "1");
		return 0;
	}

	AURA_NT_DBG("update_rgb_led: aura_event = %s\n", aura_event);
	for (aura_event_t = aura_events; aura_event_t->event_name; aura_event_t++) {
		if(!strcmp(aura_event, aura_event_t->event_name)){

			//pass sw_event when mode != 0
			if(get_aurargb_mode() != 0)
			{
				if(aura_event_t->btn_event == 0){
					AURA_NT_DBG("aura_event_t->btn_event = %d\n", aura_event_t->btn_event);
					return 0;
				}else
				btn_aura_event_trigger = 1;
			}
			 //stop aura_rgb_sw_control
			 nvram_set("pause_aura_rgb_sw", "1");
			aura_rgb_control(aura_event_t->rgb_val);

			return 1;
		}
	}
	return 0;
}

static int TrafficMeter_check(void)
{
	AURA_NT_DBG("TrafficMeter_check\n");
	FILE * fp;
	char buf[256], traffic_meter_buf[128];
	time_t now;
	unsigned long long rx = 0, tx = 0, curr_rx = 0, curr_tx = 0;
	unsigned long long rx2 = 0, tx2 = 0;
	unsigned long long traffic_meter_rx = 0;
	ino_t inode;
	struct ifino_s *ifino;
	static struct ifname_ino_tbl ifstat_tbl = { 0 };
	char *p;
	char *ifname;
	char ifname_desc[12], ifname_desc2[12];
	int ret = 0, sum, traffic_meter=0;
#if defined(RTCONFIG_QTN) || defined(RTCONFIG_QSR10G)
	qcsapi_unsigned_int l_counter_value;
#endif
	char buf_lan_ifname[2 * IFNAMSIZ], buf_lan_ifnames[20 * IFNAMSIZ];
	char *nv_lan_ifname = NULL;
	char *nv_lan_ifnames = NULL;
	struct desc_sum_s {
		const int exact;
		const char *desc;
		int valid;
		unsigned long long all_rx, all_tx;
	} desc_sum_tbl[] = {
		{ 1, "WIRED", 0, 0, 0 },
		{ 0, "WIRELESS0", 0, 0, 0 },
		{ 0, "WIRELESS1", 0, 0, 0 },
		{ 0, "WIRELESS2", 0, 0, 0 },

		{ 0, NULL, 0, 0, 0 },
	}, *ds;

	if (strlen(nvram_safe_get("lan_ifname")) >= sizeof(buf_lan_ifname))
		nv_lan_ifname = strdup(nvram_safe_get("lan_ifname"));
	if (!nv_lan_ifname) {
		strlcpy(buf_lan_ifname, nvram_safe_get("lan_ifname"), sizeof(buf_lan_ifname));
		nv_lan_ifname = buf_lan_ifname;
	}

	if (strlen(nvram_safe_get("lan_ifnames")) >= sizeof(buf_lan_ifnames))
		nv_lan_ifnames = strdup(nvram_safe_get("lan_ifnames"));
	if (!nv_lan_ifnames) {
		strlcpy(buf_lan_ifnames, nvram_safe_get("lan_ifnames"), sizeof(buf_lan_ifnames));
		nv_lan_ifnames = buf_lan_ifnames;
	}

	fp = fopen("/proc/net/dev", "r");
	if (fp) {

		fgets(buf, sizeof(buf), fp);
		fgets(buf, sizeof(buf), fp);

		while (fgets(buf, sizeof(buf), fp)) {
			if ((p = strchr(buf, ':')) == NULL) continue;
			*p = 0;
			if ((ifname = strrchr(buf, ' ')) == NULL) ifname = buf;
				else	++ifname;
			if (sscanf(p + 1, "%llu%*u%*u%*u%*u%*u%*u%*u%llu", &rx, &tx) != 2) continue;
#ifdef RTCONFIG_BCM5301X_TRAFFIC_MONITOR
			/* WAN1, WAN2, LAN */
			if(strncmp(ifname, "vlan", 4)==0){
				traffic_wanlan(ifname, (uint32_t *) &rx, (uint32_t *) &tx);
#if defined(RTCONFIG_QTN) || defined(RTCONFIG_QSR10G)
				if (nvram_contains_word("lan_ifnames", ifname)){
					if (rpc_qtn_ready()) {
						qcsapi_interface_get_counter("eth1_1", qcsapi_total_bytes_received, &l_counter_value);
						rx += l_counter_value;
						qcsapi_interface_get_counter("eth1_1", qcsapi_total_bytes_sent,	&l_counter_value);
						tx += l_counter_value;
					}
				}
#endif
			}
			if(nvram_match("wans_dualwan", "wan none")){
				if(strcmp(ifname, "eth0")==0){
					traffic_wanlan(WAN0DEV, (uint32_t *) &rx, (uint32_t *) &tx);
				}
			}
#endif	/* RTCONFIG_BCM5301X_TRAFFIC_MONITOR */
			if (!netdev_calc(ifname, ifname_desc, &rx, &tx, ifname_desc2, &rx2, &tx2, nv_lan_ifname, nv_lan_ifnames))
				continue;

			/* If inode of a interface changed, it means the interface was closed and reopened.
			 * In this case, we should calculate difference of old TX/RX bytes and new TX/RX
			 * bytes and shift from new TX/RX bytes to old TX/RX bytes.
			 */
			inode = get_iface_inode(ifname);
			curr_rx = rx;
			curr_tx = tx;
			if ((ifino = ifname_ino_ptr(&ifstat_tbl, ifname)) != NULL) {
				if (ifino->inode && ifino->inode != inode) {
					ifino->inode = inode;
					ifino->shift_rx = curr_rx - ifino->last_rx + ifino->shift_rx;
					ifino->shift_tx = curr_tx - ifino->last_tx + ifino->shift_tx;
				}
			} else {
				if ((ifstat_tbl.nr_items + 1) <= ARRAY_SIZE(ifstat_tbl.items)) {
					ifino = &ifstat_tbl.items[ifstat_tbl.nr_items];
					strlcpy(ifino->ifname, ifname, sizeof(ifino->ifname));
					ifino->inode = inode;
					ifino->last_rx = curr_rx;
					ifino->last_tx = curr_tx;
					ifino->shift_rx = ifino->shift_tx = 0;
					ifstat_tbl.nr_items++;
				}
			}

			if (ifino != NULL) {
				rx = curr_rx - ifino->shift_rx;
				tx = curr_tx - ifino->shift_tx;
				ifino->last_rx = curr_rx;
				ifino->last_tx = curr_tx;
			}

loopagain:
			/* If more than one interface are classified as same ifname_desc,
			 * sum TX/RX bytes and report it later.
			 */
			for (sum = 0, ds = &desc_sum_tbl[0]; !sum && ds->desc != NULL; ++ds) {
				if (!(ds->exact && !strcmp(ds->desc, ifname_desc)) &&
				    !(!ds->exact && !strncmp(ds->desc, ifname_desc, strlen(ds->desc))))
					continue;

				ds->all_rx += rx;
				ds->all_tx += tx;
				ds->valid = 1;
				sum = 1;
			}

			if (!sum){
				if(!strcmp(ifname_desc,"INTERNET")){
					now = uptime();
					traffic_meter_rx = rx - traffic_last_rx;

					if(traffic_event_isfirst){
						traffic_meter = 0;
						traffic_event_isfirst = 0;
					}
					else
						traffic_meter = (traffic_meter_rx/(1024))/(now - traffic_timestamp);

					AURA_NT_DBG("traffic_meter = %d\n", traffic_meter);	//KB
					traffic_last_rx = rx;
					traffic_timestamp = now;

					snprintf(traffic_meter_buf, sizeof(traffic_meter_buf), "%d", traffic_meter);
					nvram_set("aura_traffic_meter", traffic_meter_buf);

					nvram_set("pause_aura_rgb_sw", "0");
					ret = 1;
				}
			}

			if(strlen(ifname_desc2)) {
				strcpy(ifname_desc, ifname_desc2);
				rx = rx2;
				tx = tx2;
				strcpy(ifname_desc2, "");
				goto loopagain;
			}
		}

	if(fp) fclose(fp);
	}

	if (nv_lan_ifname != buf_lan_ifname)
		free(nv_lan_ifname);
	if (nv_lan_ifnames != buf_lan_ifnames)
		free(nv_lan_ifnames);
	return ret;
}
static int bwdpi_monitor_stat_check(void)
{
	AURA_NT_DBG("bwdpi_monitor_stat_check\n");
	FILE *fp;
	int mal, vp, cc;
	int AttackBlocked_count_tmp = 0;
	char *saveptr, *val;
	char buf[128];
	fp = popen("AiProtectionMonitor -c", "r");
	if (fp) {
		fgets(buf, sizeof (buf),fp);
		pclose(fp);
		if (!(val = strtok_r(buf, ":", &saveptr)))
			return -1;
		if (!(val = strtok_r(NULL, ",", &saveptr)))
			return -1;
		mal = safe_atoi(val);
		if (!(val = strtok_r(NULL, ",", &saveptr)))
			return -1;
		vp = safe_atoi(val);
		if (!(val = strtok_r(NULL, ",", &saveptr)))
			return -1;
		cc = safe_atoi(val);
		AURA_NT_DBG("mal = %d, vp = %d, cc = %d\n", mal, vp, cc);
		AttackBlocked_count_tmp = mal+vp+cc;
	}

	if(bwdpi_event_isfirst){
		AttackBlocked_count = AttackBlocked_count_tmp;
		bwdpi_event_isfirst = 0;
	}

	AURA_NT_DBG("AttackBlocked_count_tmp = %d, AttackBlocked_count = %d\n", AttackBlocked_count_tmp, AttackBlocked_count);
	if(AttackBlocked_count_tmp > AttackBlocked_count){
		AttackBlocked_count = AttackBlocked_count_tmp;
		update_rgb_led("AttackBlocked");
		return 1;
	}else
		return 0;
}

static void event_check(int sig)
{
	int ret=0;

	if(get_aurargb_mode() != 0 || nvram_get_int("pause_aura_rgb_nt") == 1){
		AURA_NT_DBG("btn_aura_event_trigger = %d\n", btn_aura_event_trigger);
		if(btn_aura_event_trigger){
			restore_aura_rgb();
			btn_aura_event_trigger = 0;
		}
		if(boot_check_rgb < 3){
			if(check_aura_rgb_reg() == -1){
				restore_aura_rgb();
			}
			boot_check_rgb++;
		}
		AURA_NT_DBG("event_check: pass event check when aurargb mode != 0\n");
		alarm(ALARM_PERIOD_T);
		return;
	}

	if(nvram_match("wrs_protect_enable","1")){
		AttackBlocked_period += ALARM_PERIOD_T;
		if(AttackBlocked_period >= 30){
			AttackBlocked_period = AttackBlocked_period%30;
			ret += bwdpi_monitor_stat_check();
		}
	}

	if(ret == 0)
		ret +=TrafficMeter_check();

	if(ret == 0)
		restore_aura_rgb();

}

static void catch_sig(int sig)
{
	char aura_event[64];

	AURA_NT_DBG("aura_rgb_nt: sig = %d\n", sig);
	if(sig == SIGALRM)
	{
		if (!nvram_get_int("aurargb_enable"))
			goto end;

		event_check(sig);
	}
	else if(sig == SIGUSR1)
	{
		if (!nvram_get_int("aurargb_enable"))
			goto end;

		strlcpy(aura_event, nvram_safe_get("aura_event"), sizeof(aura_event));
		nvram_set("aura_event", "");
		update_rgb_led(aura_event);
	}
	else if (sig == SIGTERM)
	{
		remove("/var/run/aura_rgb_nt.pid");
		exit(0);
	}

end:
	alarm(ALARM_PERIOD_T);
}

int main(void)
{
	FILE *fp;
	sigset_t sigs_to_catch;

	/* write pid */
	if ((fp = fopen("/var/run/aura_rgb_nt.pid", "w")) != NULL)
	{
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}

	/* set the signal handler */
	sigemptyset(&sigs_to_catch);
	sigaddset(&sigs_to_catch, SIGTERM);
	sigaddset(&sigs_to_catch, SIGALRM);
	sigprocmask(SIG_UNBLOCK, &sigs_to_catch, NULL);

	signal(SIGTERM, catch_sig);
	signal(SIGALRM, catch_sig);
	signal(SIGUSR1, catch_sig);

	alarm(ALARM_PERIOD_T);

	while(1)
	{
		pause();
	}

	return 0;
}
