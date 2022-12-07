#include <sys/types.h>
#include <sys/wait.h>


#include <rc.h>
#include <shared.h>
#include <amas-utils.h>
#include <libasuslog.h>

//#include <amas/amas.h>
enum {
	ROLE_NONE = 0,
	ROLE_LAN = 1,
};


#if defined(RTCONFIG_QCA_PLC2)
#include <plc_utils.h>
#endif

#if defined(RTCONFIG_QCA_PLC2)
#define PLC_FAILED_CNT 2
#define PLC_TIMEOUT_CNT 4


#define PLC_DBG(fmt, args...) \
	do { \
		_dprintf(fmt, ##args); \
		asusdebuglog(LOG_INFO, PLC_LOG_FILE, LOG_CUSTOM, LOG_SHOWTIME, 100 /*KB*/, fmt, ##args); \
	} while (0)

int compare_mac_skip3(const char *mac_1, const char *mac_2)
{
	unsigned long value_1, value_2;
	if(mac_1 == NULL || mac_2 == NULL || strncasecmp(mac_1, mac_2, 15) != 0)
		return 0;

	value_1 = strtoul(mac_1+15, NULL, 16);
	value_2 = strtoul(mac_2+15, NULL, 16);
	return (value_1 < 256 && value_2 < 256 && ((value_1 & ~0x7) == (value_2 & ~0x7)));
}

static void h_chld(int signo)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

static int wps_state = 0;
static unsigned int isolation_rst = 0;

static void h_plc_wps(int signo)
{
	if (nvram_match("wps_enable", "1"))
		wps_state = 1;
	else
		wps_state = 3;
}

void amas_set_bridge_GN(char *plc_ifname, int add_to_bridge)
{
	char wgn_ifnames[64];
	char br_lan_name[32];
	char br_lan_value[32];
	char br_name[16], *br_next = NULL;
	char if_name[16], *if_next = NULL;
	int len;
	char *action;

	if (plc_ifname == NULL || (len = strlen(plc_ifname)) <= 0)
		return;

	if (add_to_bridge)
		action = "addif";
	else
		action = "delif";

	snprintf(wgn_ifnames, sizeof(wgn_ifnames), "%s", nvram_safe_get("wgn_ifnames"));	// wgn_ifnames=br1 br2 

	if (wgn_ifnames[0] == '\0')
		return;
	/* chcek each wgn_ifnames */
	foreach (br_name, wgn_ifnames, br_next) {
		snprintf(br_lan_name, sizeof(br_lan_name), "wgn_%s_lan_ifnames", br_name);		// wgn_br1_lan_ifnames=eth2.501 eth3.501 eth1.501 eth4.501
		snprintf(br_lan_value, sizeof(br_lan_value), "%s", nvram_safe_get(br_lan_name));

		if (br_lan_value[0] == '\0')
			continue;
		/* check each lan if_name in bridge */
		foreach (if_name, br_lan_value, if_next) {
			if (strncmp(if_name, plc_ifname, len) == 0 && if_name[len] == '.') {
				eval("brctl", action, br_name, if_name);
			}
		}
	}
}

void amas_set_bridge(int is_cap_re, char *plc_ifname, int add_to_bridge, int *in_bridge)
{
	const char *action = NULL;
	if (is_cap_re > 0 && add_to_bridge != *in_bridge) {
		char amas_role[32];
		if (add_to_bridge) {
			snprintf(amas_role, sizeof(amas_role), "%s:%d", plc_ifname, ROLE_LAN);
			action = "addif";
		}
		else {
			snprintf(amas_role, sizeof(amas_role), "%s:%d", plc_ifname, ROLE_NONE);
			action = "delif";
		}
		amas_set_eth_role(amas_role);
		amas_set_bridge_GN(plc_ifname, add_to_bridge);	//also remove guest network from bridge
		*in_bridge = add_to_bridge;
	   if (isolation_rst < 1)
		syslog(LOG_NOTICE, "PLC: CAP %s %s\n", action, plc_ifname);
	}
	else if (is_cap_re == 0) { // RE
		if (add_to_bridge)
			action = "addif";
		else
			action = "delif";
	}
	else if (is_cap_re < 0 && add_to_bridge == 0 && (*in_bridge) == -1 
		&& (strcmp(get_2g_hwaddr(), "00:AA:BB:CC:DD:E0") == 0 && strcmp(get_5g_hwaddr(), "00:AA:BB:CC:DD:E4") == 0))
	{ // add plc to bridge in Factory Default for PLC test
		action = "addif";
	}
	if (action)
	{
		eval("brctl", action, nvram_safe_get("lan_ifname"), plc_ifname);
		PLC_DBG("#PLC# cap(%d) %s %s\n", is_cap_re, action, plc_ifname);
	}
}

void chk_plc_master(char *plc_ifname, int *in_bridge, int *add_try)
{
	char cfg_plc_master[18];
	char *plc_master;
	int add_to_bridge;
	int role;
	char mac[18];
	int res;

	*mac = '\0';
	strlcpy(cfg_plc_master, nvram_safe_get("cfg_plc_m_ex"), sizeof(cfg_plc_master));
	if (isValidMacAddress(cfg_plc_master) && strcasecmp(cfg_plc_master, get_lan_hwaddr()) != 0)
		plc_master = cfg_plc_master;
	else
		plc_master = NULL;

	add_to_bridge = 0;
	role = -1;
	if (plc_master == NULL) {
		add_to_bridge = 1;
	}
	else {
		amas_find_mac_role(plc_ifname, plc_master, &role);
		if (role == -1)	{ // plc_master not exist
				add_to_bridge = 1;
		}
	}
	if (add_to_bridge == *in_bridge)
		return;

	PLC_DBG("#PLC# add_to_bridge(%d) plc_master(%s) role(%d) *add_try(%d)\n", add_to_bridge, plc_master, role, *add_try);
	if (add_to_bridge == 1) {
		res = amas_find_role_lan(plc_ifname, mac);	//check ROLE_LAN PLC devices before add to bridge
		if (res == AMAS_RESULT_SUCCESS && *mac != '\0')
		{
			PLC_DBG("#PLC# find_role_lan() res(%d) mac(%s)\n", res, mac);
			return;
		}
		(*add_try)++;
		if (res != AMAS_RESULT_SUCCESS && *add_try < 5)
			return;
	}
	else
		*add_try = 0;
	amas_set_bridge(1, plc_ifname, add_to_bridge, in_bridge);
}

enum {
	PLC_LOG_INIT = 0,
	PLC_LOG_READY,
	PLC_LOG_NORMAL,
	PLC_LOG_RESET,
	PLC_LOG_MAX
};

int detect_plc_main(int argc, char *argv[])
{
	int num, last_num;
	int interval = 5;
	int i;
	struct remote_plc *rplc = NULL;
	int tx, rx;
	int tx_mimo, rx_mimo;
	int failed_cnt;
	int reset_cnt;
	int retry = 0;
	int isolation_cnt = 0;
	char plc_ifname[16];
	char br_ifname[16];
	int is_cap_re;
	int in_bridge = -1;
	int add_try = 0;
	int log_state = PLC_LOG_INIT;
	char peer_mac[18];

	signal(SIGCHLD, h_chld);
	signal(SIGUSR1, h_plc_wps);

	if (nvram_get_int("x_Setting"))
		is_cap_re = !nvram_get_int("re_mode");
	else
		is_cap_re = -1;

	PLC_DBG("#PLC# == %s == is_cap_re(%d)\n", __func__, is_cap_re);
	PLC_DBG("#PLC# cfg_plc_m_ex(%s) cfg_plc_maste(%s)\n", nvram_get("cfg_plc_m_ex"), nvram_get("cfg_plc_master"));
	nvram_set("cfg_plc_m_ex", nvram_get("cfg_plc_master"));		//save for apply
	get_plc_ifname(plc_ifname);
	strlcpy(br_ifname, nvram_safe_get("lan_ifname"), sizeof(br_ifname));
	amas_set_bridge(is_cap_re, plc_ifname, 0, &in_bridge);		//remove from LAN bridge (br0) first
	last_num = -1;
	failed_cnt = PLC_FAILED_CNT;
	reset_cnt = 0;
	nvram_unset("plc_pb_state");
	peer_mac[0] = '\0';
	while(1) {
		if (is_intf_up(plc_ifname) <= 0 || is_intf_up(br_ifname) <= 0
				 || nvram_get_int("plchost_active") <= 0)
		{ //interface inactive OR plchost is stopped
			sleep(2);
			continue;
		}
		if (pids("plchost") == 0) {
			PLC_DBG("#PLC# MISSING plchost !!\n");
			extern int start_plchost();
			start_plchost();
		}

		/* normal work */
		if (nvram_get_int("plc_ready"))
		{
			if (log_state != PLC_LOG_READY && log_state != PLC_LOG_NORMAL) {
			    if (isolation_rst <= 1)
				syslog(LOG_NOTICE, "PLC: ready");
				PLC_DBG("#PLC# ready !!\n");
				log_state = PLC_LOG_READY;
			}
			num = get_connected_plc(&rplc);
		}
		else
			num = 0;

	    if (num > 0 || failed_cnt++ >= PLC_FAILED_CNT) {
		tx = rx = 0;
		tx_mimo = rx_mimo = 0;
		if (num > 0) {
			int cnt = num;
#ifdef RTCONFIG_AMAS
			int found = 0;
			char cfg_plc_master[18];
			char amas_cap_addr[18];
			if(is_cap_re == 0) {
				snprintf(amas_cap_addr, sizeof(amas_cap_addr), "%s", nvram_safe_get("amas_cap_addr"));
				if (!isValidMacAddress(amas_cap_addr))
					*amas_cap_addr = '\0';
				snprintf(cfg_plc_master, sizeof(cfg_plc_master), "%s", nvram_safe_get("cfg_plc_m_ex"));
				if (!isValidMacAddress(cfg_plc_master))
					*cfg_plc_master = '\0';
			}
#endif	/* RTCONFIG_AMAS */

			for(i = 0; i < num; i++) {
				if ((num > last_num && strcmp(peer_mac, rplc[i].mac) != 0) || num < last_num)
					snprintf(peer_mac, sizeof(peer_mac), "%s", rplc[i].mac);
#ifdef RTCONFIG_AMAS
				// calculate to PLC master (or CAP)
				if (is_cap_re == 1)
					;		// cap 
				else if (*cfg_plc_master && compare_mac_skip3(cfg_plc_master, rplc[i].mac)) {
					found = 1;
				}
				else if (*amas_cap_addr && compare_mac_skip3(amas_cap_addr, rplc[i].mac)) {
					found = 1;
				}
				if (found) {
					tx = rplc[i].tx;
					rx = rplc[i].rx;
					tx_mimo = rplc[i].tx_mimo;
					rx_mimo = rplc[i].rx_mimo;
					cnt = 1;
					break;
				}
#endif	/* RTCONFIG_AMAS */
				tx += rplc[i].tx;
				rx += rplc[i].rx;
				tx_mimo += rplc[i].tx_mimo;
				rx_mimo += rplc[i].rx_mimo;
			}
			tx = tx/cnt;
			rx = rx/cnt;

			free(rplc);
			rplc = NULL;
			failed_cnt = 0;
			reset_cnt = 0;
			interval = 10;
		}
		else {
			peer_mac[0] = '\0';
		}

		if (num != last_num) {
			PLC_DBG("#PLC# num(%d) last_num(%d) tx(%d) rx(%d) peer_mac(%s)\n", num, last_num, tx, rx, peer_mac);
			nvram_set_int("autodet_plc_state" , num);
			last_num = num;
		}
		nvram_set_int("autodet_plc_tx", tx);
		nvram_set_int("autodet_plc_rx", rx);
		nvram_set_int("autodet_plc_tx_mimo", tx_mimo);
		nvram_set_int("autodet_plc_rx_mimo", rx_mimo);
		if (num > 0 && (tx < 10 || rx < 10)) {
			void run_plcrate(int duration);
			run_plcrate(1);
		}
	    }
		if (num <= 0) {
			if (failed_cnt > PLC_TIMEOUT_CNT) {
				if (!chk_plc_alive()) {
					PLC_DBG("#PLC# not alive !!\n");
					if (log_state != PLC_LOG_RESET) {
						syslog(LOG_NOTICE, "PLC: not alive !!");
						log_state = PLC_LOG_RESET;
					}
				    if (nvram_get_int("plc_head") <= 0)	//when plc_head==1 the state change would not be re-add into bridge 
					amas_set_bridge(is_cap_re, plc_ifname, 0, &in_bridge); //remove from LAN bridge (br0)
					add_try = 0;
					do_plc_reset(1);
				}
				else if (!nvram_get_int("plc_ready")) {
				    if (nvram_get_int("plc_head") <= 0)	//when plc_head==1 the state change would not be re-add into bridge 
					amas_set_bridge(is_cap_re, plc_ifname, 0, &in_bridge); //remove from LAN bridge (br0)
					add_try = 0;
					if (reset_cnt++ >= 3) {
						PLC_DBG("#PLC# force reset !!\n");
						if (log_state != PLC_LOG_RESET) {
							syslog(LOG_NOTICE, "PLC: force reset !!");
							log_state = PLC_LOG_RESET;
						}
						do_plc_reset(1);
						reset_cnt = 0;
					}
					else {
						PLC_DBG("#PLC# not reset normally !!\n");
						do_plc_reset(0);
					}
				}
				failed_cnt = 0;
			}
			interval = 5;
		}
		/* check PLC isolation state */
#define ISOLATION_TIME  	(300)	//in second
#define ISOLATION_TIME_1	(240)	//in second
		if (((tx < 10 && rx < 10) || num <= 0) && wps_state == 0 && nvram_match("x_Setting", "1")) {
			if ((isolation_cnt += ((num <= 0)?5:10)) > ISOLATION_TIME) {
				/* "plc_iso_rst": 0 disable */
				if (!nvram_match("plc_iso_rst", "0")) {
					PLC_DBG("#PLC# isolation reset: isolation_cnt(%d) tx(%d) rx(%d) num(%d)\n", isolation_cnt, tx, rx, num);
					if (log_state != PLC_LOG_RESET) {
					    if (++isolation_rst <= 1)
						syslog(LOG_NOTICE, "PLC: isolation reset isolation_cnt(%d) tx(%d) rx(%d) num(%d) !!", isolation_cnt, tx, rx, num);
						log_state = PLC_LOG_RESET;
					}
				    if (nvram_get_int("plc_head") <= 0)	//when plc_head==1 the state change would not be re-add into bridge 
					amas_set_bridge(is_cap_re, plc_ifname, 0, &in_bridge); //remove from LAN bridge (br0)
					add_try = 0;
					do_plc_reset(1);
					nvram_set_int("plc_iso_rst", nvram_get_int("plc_iso_rst")+1);
				}
				isolation_cnt = 0;
			}
		}
		else if (num > 0 && tx > 10 && rx > 10)
		{ //PLC normal
			isolation_cnt = ISOLATION_TIME_1;	//left 60 second for normal to abnormal
			log_state = PLC_LOG_NORMAL;
			isolation_rst = 0;
		}

		/* check WPS state */
		if (wps_state == 1) {
			if (is_wps_stopped() == 0) {
				PLC_DBG("#PLC# wifi wps running!\n");
				do_plc_pushbutton(6);	/* 1: PLC join procedure */
				nvram_set("plc_pb_state", "2");
					wps_state = 2;
			}
			else {
				interval = 1;
				if (retry++ > 10) {
					PLC_DBG("#PLC# wifi wps NOT run!\n");
					wps_state = 0;
					retry = 0;
				}
			}
		}
		else if (wps_state == 2) {
			int pb_state;
			int wps_stopped;

			pb_state = get_plc_pb_state();
			wps_stopped = is_wps_stopped();

			if (wps_stopped == 0 && (pb_state == 0 || pb_state == 4 || pb_state == 5 || pb_state == 6)) {
				PLC_DBG("#PLC# plc push button is stopped! pb_state(%d)\n", pb_state);
				nvram_set_int("plc_pb_state", pb_state);
				stop_wps_method();
			}
			else if (wps_stopped && (pb_state == 1 || pb_state == 2 || pb_state == 3)) {
				PLC_DBG("#PLC# wifi wps is stopped! wps_stopped(%d)\n", wps_stopped);
				nvram_set("plc_pb_state", "5");
				do_plc_pushbutton(5);	/* 5: stop PLC join procedure */
			}
			if (wps_stopped || (pb_state == 0 || pb_state == 4 || pb_state == 5 || pb_state == 6)) {
				PLC_DBG("#PLC# BOTH (wps/plc) END\n");
				nvram_set_int("plc_pb_state", pb_state);
				wps_state = 0;
				retry = 0;
			}
		}
		else if (wps_state == 3) {
			do_plc_pushbutton(6);	/* 1: PLC join procedure */
			nvram_set("plc_pb_state", "2");
			wps_state = 4;
		}
		else if (wps_state == 4) {
			int pb_state = get_plc_pb_state();
			if (IS_PLC_JOIN_STOPPED(pb_state)) {
				PLC_DBG("#PLC# plc pair END\n");
				nvram_set_int("plc_pb_state", pb_state);
				wps_state = 0;
				retry = 0;
			}
		}

		if (is_cap_re == 1 && nvram_get_int("plc_ready"))
			chk_plc_master(plc_ifname, &in_bridge, &add_try);

		sleep(interval);
	}
	return 0;
}
#endif	/* RTCONFIG_QCA_PLC_UTILS || RTCONFIG_QCA_PLC2 */

