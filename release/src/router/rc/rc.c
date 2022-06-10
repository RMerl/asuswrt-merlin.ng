/*

	Tomato Firmware
	Copyright (C) 2006-2009 Jonathan Zarate

*/
#include "rc.h"
#include "interface.h"
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#ifdef HND_ROUTER
#include <time.h>
#include <pthread.h>
#endif
#ifdef RTCONFIG_RALINK
#include <ralink.h>
#endif

#ifdef RTCONFIG_QCA
#include <qca.h>
#include <sys/mman.h>
#endif

#if defined(RTCONFIG_LP5523)
#include <lp5523led.h>
#endif

#if defined(RTCONFIG_SOC_IPQ8074)
#include <sys/vfs.h>
#include <inttypes.h>
#include <sys/reboot.h>
#endif

#ifdef RTCONFIG_COMFW
#include <comfw.h>
#endif

#include <model.h>

#ifndef ARRAYSIZE
#define ARRAYSIZE(a) (sizeof(a) / sizeof(a[0]))
#endif /* ARRAYSIZE */

#if defined(RTCONFIG_HW_DOG)
int hwdog_main(int, char **);
#endif
#if defined(RTCONFIG_PTHSAFE_POPEN)
void PS_pod_main(void);
#endif

void exe_eu_wa_rr(void);
#ifdef RTCONFIG_BCMBSD_V2
extern void gen_bcmbsd_def_policy(int sel);
#endif

#ifdef HND_ROUTER
typedef enum cmds_e {
	REGACCESS,
	PMDIOACCESS,
} ecmd_t;

extern uint64_t hnd_ethswctl(ecmd_t act, unsigned int val, int len, int wr, unsigned long long regdata);
#endif

#if defined(CONFIG_BCMWL5)
double wl_get_txpwr_target_max(char *name);
double get_wifi_maxpower(int target_unit);
#endif

// led_str_ctrl
enum led_id get_led_id(const char *led_str)
{
	enum led_id led = LED_ID_MAX;

	if(!strcmp(led_str, "LED_POWER")){
		return LED_POWER;
	}
	else if(!strcmp(led_str, "LED_WPS")){
		return LED_WPS;
	}
	else if(!strcmp(led_str, "LED_WAN")){
		return LED_WAN;
	}
#ifdef HND_ROUTER
	else if(!strcmp(led_str, "LED_WAN_NORMAL")){
		return LED_WAN_NORMAL;
	}
#endif
#ifdef RTCONFIG_EXTPHY_BCM84880
	else if(!strcmp(led_str, "LED_EXTPHY")){
		return LED_EXTPHY;
	}
#endif
#ifdef RTCONFIG_LAN4WAN_LED
	else if(!strcmp(led_str, "LED_LAN1")){
		return LED_LAN1;
	}
	else if(!strcmp(led_str, "LED_LAN2")){
		return LED_LAN2;
	}
	else if(!strcmp(led_str, "LED_LAN3")){
		return LED_LAN3;
	}
	else if(!strcmp(led_str, "LED_LAN4")){
		return LED_LAN4;
	}
#endif
#if 0
	else if(!strcmp(led_str, "LED_LAN")){
		return LED_LAN;
	}
	else if(!strcmp(led_str, "LED_USB")){
		return LED_USB;
	}
	else if(!strcmp(led_str, "LED_USB3")){
		return LED_USB3;
	}
	else if(!strcmp(led_str, "LED_2G")){
		return LED_2G;
	}
	else if(!strcmp(led_str, "LED_5G")){
		return LED_5G;
	}
#endif
	else{
		dbg("%s: Unknown LED: %s!\n", __func__, led_str);
	}

	return led;
}


#ifdef  __CONFIG_WBD__
static void
wbd_restore_defaults(void)
{
	char tmp[100], prefix[] = "wlXXXXXXXXXXXXXXXXXXXXXXXXXXXX_mssid_";
	int i, j, iter_param;

	/* List of WBD NVRAMs */
	char* wbd_nvrams[] = {
		/* NVRAMs general to WBD application */
		"wbd_ifnames",
		"wbd_mode",
		"wbd_msglevel",
		"wbd_ignr_maclst",
		"wbd_fixed_ifnames",
		"wbd_no_dedicated_backhaul",
		/* NVRAMs for Target BSS Identification Configuration */
		"wbd_tbss_wght_idx",
		"wbd_tbss_wght",
		"wbd_tbss_thld_idx",
		"wbd_tbss_thld",
		"wbd_tbss_algo",
		"wbd_adv_thld",
		"wbd_tbss_stacnt_thld",
		/* NVRAMs for Weak Client Identification Configuration */
		"wbd_wc_algo",
		"wbd_wc_thld_idx",
		"wbd_wc_thld",
		"wbd_weak_sta_cfg",
		"wbd_weak_sta_policy",
		/* NVRAMs for Timeouts */
		"wbd_tm_join",
		"wbd_tm_keepalive",
		"wbd_tm_wd_probe",
		"wbd_tm_slv_inactv",
		"wbd_tm_subscribe",
		"wbd_tm_sta_reports",
		"wbd_tm_blk_sta",
		"wbd_tm_wd_weakclient",
		"wbd_tm_evt_grace",
		"wbd_tm_wd_tbss",
		"wbd_tm_config",
		"wbd_tm_actframe",
		"wbd_bounce_detect",
		"wbd_tm_dfs_list",
		"bcm_stamon_get_interval"
	};

	/* Traverse through all WBD NVRAMs */
	for (iter_param = 0; iter_param < ARRAYSIZE(wbd_nvrams); iter_param++) {

		/* Clear NVRAMs without Prefix */
		nvram_unset(wbd_nvrams[iter_param]);

		/* Traverse through all Primary Prefix */
		for (i = 0; i < MAX_NVPARSE; i++) {
			sprintf(prefix, "wl%d_", i);

			/* Clear NVRAMs with Primary Prefix */
			nvram_unset(strcat_r(prefix, wbd_nvrams[iter_param], tmp));

			/* Traverse through all Virtual Prefix */
			for (j = 0; j < MAX_NVPARSE; j++) {
				sprintf(prefix, "wl%d.%d_", i, j);

				/* Clear NVRAMs with Virtual Prefix */
				nvram_unset(strcat_r(prefix, wbd_nvrams[iter_param], tmp));
			}
		}
	}
}
#endif /* __CONFIG_WBD__ */

#ifdef WLHOSTFBT

/* Clear FBT_APs NVRAMS based on prefix */
void
fbt_aps_restore_defaults(char *prefix)
{
	char tmp_prefix[] = "wlXXXXXXXXXXXXXXXXXXXXXXXXXXXX_mssid_";
	char *fbt_aps, *next;
	char tmp[100], fbt_ap[100], tmp_fbt_ap[100];
	int iter_param, i, j;

	/* List of FBT_AP NVRAMs */
	char* fbt_ap_nvrams[] = {
		"addr",
		"r1kh_id",
		"r0kh_id",
		"r0kh_id_len",
		"br_addr",
		"r0kh_key",
		"r1kh_key",
	};

	/* Get fbt_all_gen_aps NVRAM */
	fbt_aps = nvram_safe_get(strcat_r(prefix, "fbt_all_gen_aps", tmp));
	/* If no values, no need to restore those */
	if (strlen(fbt_aps) <= 0) {
		goto fbt_all_aps;
	}
	/* For each fbt_all_gen_aps, clear the fbt_ap_nvrams */
	foreach(fbt_ap, fbt_aps, next) {
		/* Traverse through all FBT_AP NVRAMs */
		for (iter_param = 0; iter_param < ARRAYSIZE(fbt_ap_nvrams); iter_param++) {
			snprintf(tmp_fbt_ap, sizeof(tmp_fbt_ap), "%s_%s", fbt_ap,
				fbt_ap_nvrams[iter_param]);
			nvram_unset(tmp_fbt_ap);
		}
	}

fbt_all_aps:
	/* Get fbt_all_aps NVRAM to clear the fbt_bssid NVRAM*/
	fbt_aps = nvram_safe_get(strcat_r(prefix, "fbt_all_aps", tmp));
	/* If no values, no need to restore those */
	if (strlen(fbt_aps) <= 0) {
		return;
	}

	/* For each fbt_aps, clear the fbt_bssid nvrams */
	foreach(fbt_ap, fbt_aps, next) {
		/* Traverse through all Primary Prefix to restore fbt_aps NVRAMs */
		for (i = 0; i < MAX_NVPARSE; i++) {
			sprintf(tmp_prefix, "wl%d_", i);
			snprintf(tmp_fbt_ap, sizeof(tmp_fbt_ap), "%s%s_fbt_bssid",
				tmp_prefix, fbt_ap);
			nvram_unset(tmp_fbt_ap);

			/* Traverse through all Virtual Prefix */
			for (j = 0; j < MAX_NVPARSE; j++) {
				sprintf(tmp_prefix, "wl%d.%d_", i, j);
				snprintf(tmp_fbt_ap, sizeof(tmp_fbt_ap), "%s%s_fbt_bssid",
					tmp_prefix, fbt_ap);
				nvram_unset(tmp_fbt_ap);
			}
		}
	}
}

/* Clear all the FBT NVRAMs */
void
fbt_restore_defaults(void)
{
	char tmp[100], prefix[] = "wlXXXXXXXXXXXXXXXXXXXXXXXXXXXX_mssid_";
	int i, j, iter_param;

	/* List of FBT NVRAMs */
	char* fbt_nvrams[] = {
		"fbt",
		"fbt_mdid",
		"fbtoverds",
		"fbt_reassoc_time",
		"fbt_ap",
		"r0kh_id",
		"r1kh_id",
		"r0kh_key",
		"fbt_aps",
		"fbt_all_aps",
		"fbt_all_gen_aps"
	};

	/* Traverse through all Primary Prefix to restore fbt_aps NVRAMs */
	for (i = 0; i < MAX_NVPARSE; i++) {
		sprintf(prefix, "wl%d_", i);
		fbt_aps_restore_defaults(prefix);

		/* Traverse through all Virtual Prefix */
		for (j = 0; j < MAX_NVPARSE; j++) {
			sprintf(prefix, "wl%d.%d_", i, j);
			fbt_aps_restore_defaults(prefix);
		}
	}

	/* Traverse through all FBT NVRAMs */
	for (iter_param = 0; iter_param < ARRAYSIZE(fbt_nvrams); iter_param++) {
		/* Traverse through all Primary Prefix */
		for (i = 0; i < MAX_NVPARSE; i++) {
			sprintf(prefix, "wl%d_", i);

			/* Clear NVRAMs with Primary Prefix */
			nvram_unset(strcat_r(prefix, fbt_nvrams[iter_param], tmp));

			/* Traverse through all Virtual Prefix */
			for (j = 0; j < MAX_NVPARSE; j++) {
				sprintf(prefix, "wl%d.%d_", i, j);

				/* Clear NVRAMs with Virtual Prefix */
				nvram_unset(strcat_r(prefix, fbt_nvrams[iter_param], tmp));
			}
		}
	}
}
#endif /* WLHOSTFBT */

#ifdef LINUX_2_6_36
static int
coma_uevent(void)
{
	char *modalias = NULL;
	char lan_ifname[32], *lan_ifnames, *next;

	modalias = getenv("MODALIAS");
	if (!strcmp(modalias, "platform:coma_dev")) {

		/* down WiFi adapter */
		lan_ifnames = nvram_safe_get("lan_ifnames");
		foreach(lan_ifname, lan_ifnames, next) {
			if (!strncmp(lan_ifname, "eth", 3)) {
				eval("wl", "-i", lan_ifname, "down");
			}
		}

		system("echo \"2\" > /proc/bcm947xx/coma");
	}
	return 0;
}
#endif /* LINUX_2_6_36 */

#ifdef DEBUG_RCTEST

int pt_loops;
void *inc_x(void *x_void_ptr)
{
	printf("thread-%s:pid:%d, x increment start\n", __func__, getpid());

	int *x_ptr = (int *)x_void_ptr;
	while(++(*x_ptr) < pt_loops) {
		nvram_set("ppap_x", "1");
		if(!nvram_match("ppap_x", "1")) {
			printf("%s, pid=%d. nvram read err:%s\n", __func__, getpid(), nvram_safe_get("ppap_x"));
		}
	}

	printf("thread-%s:pid:%d, x increment finished\n", __func__, getpid());

	/* the function must return something - NULL will do */
	return NULL;
}

#ifdef HND_ROUTER
int pt_main(int loops)
{

	int x = 0, y = 0;

	printf("%s, pid:%d, x=%d, y=%d\n", __func__, getpid(), x, y);

	pt_loops = loops;

	pthread_t inc_x_thread;

	if(pthread_create(&inc_x_thread, NULL, inc_x, &x)) {
		fprintf(stderr, "Error creating thread\n");
		return 1;
	}

	/* increment y to 100 in the first thread */
	while(++y < pt_loops) {
		nvram_set("ppap_y", "2");
		if(!nvram_match("ppap_y", "2")) {
			printf("%s, pid=%d, nvram read err:%s\n", __func__, getpid(), nvram_safe_get("ppap_y"));
		}
	}

	printf("y increment finished\n");

	/* wait for the second thread to finish */
	if(pthread_join(inc_x_thread, NULL)) {
		fprintf(stderr, "Error joining thread\n");
	return 2;

	}

	/* show the results - x is now 100 thanks to the second thread */
	printf("x: %d, y: %d\n", x, y);

	return 0;
}
#endif	/* HND_ROUTER */

int test_mknode(int id);
// used for various testing
static int rctest_main(int argc, char *argv[])
{
	int on;

	if (argc < 2) {
		_dprintf("test what?\n");
	}
	else if (strcmp(argv[1], "rc_service")==0) {
		notify_rc(argv[2]);
	}
#if defined(CONFIG_BCMWL5) && defined(HND_ROUTER)
	else if (strcmp(argv[1], "gpy211")==0) {
		GPY211_INIT_SPEED();
	}
	else if (strcmp(argv[1], "gpy211_wan")==0) {
		GPY211_WAN_SPEED();
	}
#endif
#if defined(RTCONFIG_FRS_FEEDBACK)
	else if (strcmp(argv[1], "sendfeedback")==0) {
		start_sendfeedback();
	}
#endif
#ifdef RTCONFIG_BCM_7114
	else if (strcmp(argv[1], "spect")==0) {
		start_dfs();
	}
#endif
#ifdef RTCONFIG_PERMISSION_MANAGEMENT
	else if (strcmp(argv[1], "permission") == 0) {
		setup_passwd();
	}
#endif
	else if (strcmp(argv[1], "GetPhyStatus")==0) {
		printf("Get Phy status:%d\n", GetPhyStatus(0, NULL));
	}
	else if (strcmp(argv[1], "GetExtPhyStatus")==0) {
		printf("Get Ext Phy status:%d\n", GetPhyStatus(atoi(argv[2]), NULL));
	}
#ifdef CONFIG_BCMWL5
	else if(strcmp(argv[1], "frdwa")==0){
		printf("frdwa\n");
		exe_eu_wa_rr();
	}
#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_BHCOST_OPT)
	else if (strcmp(argv[1], "linkrate")==0) {
		if(argv[2]) {
			int rate = 0;
			rate = get_uplinkports_linkrate(argv[2]);
			printf("portif[%s] link rate is %d\n", argv[2], rate);
		}
	}
#endif
#endif
#ifdef HND_ROTUER
	else if (strcmp(argv[1], "memdw")==0) {
		const char *dws[]={"dw", argv[2]};
		_memaccess(2, dws);
	}
#endif
	else if (strcmp(argv[1], "get_phy_status")==0) {
		int mask;
		mask = atoi(argv[2]);
		TRACE_PT("debug for phy_status %x\n", get_phy_status(mask));
	}
	else if (strcmp(argv[1], "get_phy_speed")==0) {
		int mask;
		mask = atoi(argv[2]);
		TRACE_PT("debug for phy_speed %x\n", get_phy_speed(mask));
	}
	else if (strcmp(argv[1], "dumpx")==0) {
		FILE *fp;
		int i, ch;
		int size = nvram_get_int("dump_size");
		//int start = nvram_get_int("dump_start");

		if(!argv[2])	
			return 0;

		_dprintf("dumpx [%s] -------->\n", argv[2]);
		fp = fopen(argv[2], "r");
		for(i=0; i<size; ++i) {
                        ch = fgetc(fp);
			if(ch == EOF) {
				_dprintf("\n---- EOF(%d) ---\n", i);
				break;
			}
                        _dprintf("[%2x] ", ch);
                        if(i%16 == 0)
                                _dprintf("\n");
		}
		_dprintf("\n<----------------\n");

		fclose(fp);
	}
#ifdef RTCONFIG_BCM_CLED
	else if (strcmp(argv[1], "cled")==0) {
		unsigned int led, mode;

		led = atoi(argv[2]);
		mode = atoi(argv[3]);
		_dprintf("Set Cled %d as %d\n", led, mode);
#if defined(RPAX56) || defined(RPAX58)
		rc_bcm_cled_ctrl(led, mode);
#else
		bcm_cled_ctrl(led, mode);
#endif
	}
#if defined(ET12) || defined(XT12)
	else if (strcmp(argv[1], "cled_white")==0) {
		unsigned int led, mode;

		led = atoi(argv[2]);
		mode = atoi(argv[3]);
		_dprintf("set Cled_WHITE %d as %d\n", led, mode);
		bcm_cled_ctrl_single_white(led, mode);
	}
#endif
#endif
#ifdef HND_ROUTER
	else if (strcmp(argv[1], "regr")==0) {
		unsigned int reg;
		sscanf(argv[2], "%x", &reg);
		_dprintf("regaccess rd 0x%x\n", reg);
		hnd_ethswctl(REGACCESS, reg, 2, 0, 0);
	}
	else if (strcmp(argv[1], "regw")==0) {
		unsigned int reg, data;
		sscanf(argv[2], "%x", &reg);
		sscanf(argv[3], "%x", &data);
		_dprintf("regaccess wr 0x%x, 0x%x\n", reg, data);
		hnd_ethswctl(REGACCESS, reg, 2, 1, data);
	}
	else if (strcmp(argv[1], "pregr")==0) {
		unsigned int reg;
		sscanf(argv[2], "%x", &reg);
		_dprintf("pregaccess rd 0x%x\n", reg);
		hnd_ethswctl(PMDIOACCESS, reg, 2, 0, 0);
	}
	else if (strcmp(argv[1], "pregw")==0) {
		unsigned int reg, data;
		sscanf(argv[2], "%x", &reg);
		sscanf(argv[3], "%x", &data);
		_dprintf("pregaccess wr 0x%x, 0x%x\n", reg, data);
		hnd_ethswctl(PMDIOACCESS, reg, 2, 1, data);
	}
	else if (strcmp(argv[1], "set_phy_ctrl")==0) {
		unsigned int mask, ctrl;
		sscanf(argv[2], "%x", &mask);
		sscanf(argv[3], "%x", &ctrl);
		_dprintf("phy_ctrl 0x%x/%x (%d)\n", mask, ctrl, set_phy_ctrl(mask, ctrl));
	}
#if defined(RTCONFIG_EXT_BCM53134)
	else if (strcmp(argv[1], "set_ex53134_ctrl")==0) {
		unsigned int mask, ctrl;
		sscanf(argv[2], "%x", &mask);
		sscanf(argv[3], "%x", &ctrl);
		_dprintf("ex53134 phy_ctrl 0x%x/%x (%d)\n", mask, ctrl, set_ex53134_ctrl(mask, ctrl));
	}
#endif
#endif
	else if (strcmp(argv[1], "lanports_ctrl")==0) {
		int val;
		val = atoi(argv[2]);
		_dprintf("lan ctrl %d\n", lanport_ctrl(val));
	}
	else if (strcmp(argv[1], "handle_notifications")==0) {
		handle_notifications();
	}
	else if (strcmp(argv[1], "check_action")==0) {
		_dprintf("check: %d\n", check_action());
	}
#ifdef RTCONFIG_AMAS_WGN
	else if (strcmp(argv[1], "wgn")==0) {
		_dprintf("check: %d\n", is_wgn_enabled());
	}
#endif
#ifdef RTCONFIG_BCMBSD_V2
	else if (strcmp(argv[1], "bsdsel")==0) {
		int selif_val = nvram_get_int("smart_connect_selif");

		gen_bcmbsd_def_policy(selif_val);
	}
#endif
#if defined(RPAX56) || defined(RPAX58)
	else if (strcmp(argv[1], "is_client") == 0) {
		printf("client_mode=%d\n", client_mode());
	}
	else if (strcmp(argv[1], "is_psr") == 0) {
		printf("is_psr(0:%d)(1:%d)\n", is_psr(0), is_psr(1));
	}
	else if (strcmp(argv[1], "is_dpsta") == 0) {
		printf("dpsta_mode=%d\n", dpsta_mode());
	}
	else if (strcmp(argv[1], "is_psta") == 0) {
		printf("is_psta=(0:%d, 1:%d)\n", is_psta(0), is_psta(1));
	}
	else if (strcmp(argv[1], "is_rp") == 0) {
		printf("rp_mode=%d(0:%d, 1:%d)\n", rp_mode(), is_rp_unit(0), is_rp_unit(1));
	}
#endif
        else if (strcmp(argv[1], "getbw") == 0) {
                int unit = atoi(argv[2]);
                int bw = wl_get_bw(unit);
                printf("get wl_bw of unit_%d=%d\n", unit, bw);
        }
#ifdef HND_ROUTER
	else if (strcmp(argv[1], "ptest")==0) {
		pt_main(atoi(argv[2]));
		return 0;
	}
	else if (strcmp(argv[1], "forkd")==0) {
		pid_t pid = 0;
		int i, err=0;
		char *nvp = NULL;
		clock_t begin, end;
		int loops = atoi(argv[2])?:2000;
		int utime = atoi(argv[3])?:0;
		int no_fork = atoi(argv[4]);

		if(no_fork) {
			begin = clock();
			for(i=0; i<loops; ++i) {
				nvram_set("ppap1", "1");
				nvp = nvram_safe_get("ppap1\n");
				if(nvram_get_int("ppap1")!=1) {
					err++;
					_dprintf("pid_%d get wrong nv_ppap1=%s(%d)\n", pid, nvp, err);
				}
				if(utime)
					usleep(utime);
			}
			end = clock();
			_dprintf("pid_%d, clock counts=%d\n", pid, (int)(end-begin));
			return 0;
		} 

		_dprintf("(NVP)fork nv test w/ loops(%d), usleep(%d)\n", loops, utime);

		pid = fork();

		if(pid == 0) {
			begin = clock();
			for(i=0; i<loops; ++i) {
				nvram_set("ppap1", "1");
				nvp = nvram_safe_get("ppap1\n");
				if(nvram_get_int("ppap1")!=1) {
					err++;
					_dprintf("pid_%d get wrong nv_ppap1=%s(%d)\n", pid, nvp, err);
				}
				if(utime)
					usleep(utime);
			}
			end = clock();
			_dprintf("pid_%d, clock counts=%d\n", pid, (int)(end-begin));
		} else if(pid > 0) {
			begin = clock();
			for(i=0; i<loops; ++i) {
				nvram_set("ppap2", "2");
				nvp = nvram_safe_get("ppap2\n");
				if(nvram_get_int("ppap2")!=2) {
					err++;
					_dprintf("pid_%d get wrong nv_ppap2=%s(%d)\n", pid, nvp, err);
				}
				if(utime)
					usleep(utime);
			}
			end = clock();
			_dprintf("pid_%d, clock counts=%d\n", pid, (int)(end-begin));
		} else {
			_dprintf("wrong fork.\n");
		}

		_dprintf("pid_%d exit\n", pid);
	}
#endif
	else if (strcmp(argv[1], "nvramhex")==0) {
		int i;
		char *nv;

		nv = nvram_safe_get(argv[2]);

		_dprintf("nvram %s(%d): ", nv, strlen(nv));
		for(i=0;i<strlen(nv);i++) {
			_dprintf(" %x", (unsigned char)*(nv+i));
		}
		_dprintf("\n");
	}
	else if (strcmp(argv[1], "phy_info")==0) {
		phy_info_list phy_list = {0};
		int /*ret,*/ i;

		//if ((ret = GetPhyStatus(1, &phy_list)) == 1) {
			GetPhyStatus(1, &phy_list);
			for(i=0;i<phy_list.count;i++) {
				fprintf(stderr, " phy_port_id=%d, label_name=%s, cap_name=%s, state=%s, link_rate=%d, duplex=%s, tx_packets=%u, rx_packets=%u, tx_bytes=%llu, rx_bytes=%llu, crc_errors=%u\n", 
					phy_list.phy_info[i].phy_port_id,
					phy_list.phy_info[i].label_name,
					phy_list.phy_info[i].cap_name,
					phy_list.phy_info[i].state,
					phy_list.phy_info[i].link_rate,
					phy_list.phy_info[i].duplex,
					phy_list.phy_info[i].tx_packets,
					phy_list.phy_info[i].rx_packets,
					phy_list.phy_info[i].tx_bytes,
					phy_list.phy_info[i].rx_bytes,
					phy_list.phy_info[i].crc_errors);
			}
		//} else
		//	_dprintf("GetPhyStatus failed (%d): ", ret);
		_dprintf("\n");
	}
	else {
		on = atoi(argv[2]);
		_dprintf("%s %d\n", argv[1], on);

		if (strcmp(argv[1], "vlan") == 0)
		{
			if (on) start_vlan();
			else stop_vlan();
		}
		else if (strcmp(argv[1], "lan") == 0) {
			if (on) start_lan();
			else stop_lan();
		}
		else if (strcmp(argv[1], "wl") == 0) {
			if (on)
			{
				start_wl();
				lanaccess_wl();
			}
		}
		else if (strcmp(argv[1], "wan") == 0) {
			if (on) start_wan();
			else stop_wan();
		}
		else if (strcmp(argv[1], "wan_port") == 0) {
			if (on) start_wan_port();
			else stop_wan_port();
		}
		else if (strcmp(argv[1], "firewall") == 0) {
			//if (on) start_firewall();
			//else stop_firewall();
		}
		else if (strcmp(argv[1], "watchdog") == 0) {
			if (on) start_watchdog();
			else stop_watchdog();
		}
		else if (strcmp(argv[1], "check_watchdog") == 0) {
			if (on) start_check_watchdog();
			else stop_check_watchdog();
		}
		else if (strcmp(argv[1], "fwupg_flashing") == 0) {
			printf("go rc applt: fwupg_flashing\n");
			start_fwupg_flashing();
		}
#ifdef RTAC87U
		else if (strcmp(argv[1], "watchdog02") == 0) {
			if (on) start_watchdog02();
			else stop_watchdog02();
		}
#endif
#ifdef SW_DEVLED
		else if (strcmp(argv[1], "sw_devled") == 0) {
			if (on) start_sw_devled();
			else stop_sw_devled();
		}
#endif
#if defined(RTAC1200G) || defined(RTAC1200GP)
		else if (strcmp(argv[1], "wdg_monitor") == 0) {
			if (on) start_wdg_monitor();
			else stop_wdg_monitor();
		}
#endif
#if defined(CONFIG_BCMWL5) && defined(RTCONFIG_FANCTRL)
		else if (strcmp(argv[1], "phy_tempsense") == 0) {
			if (on) start_phy_tempsense();
			else stop_phy_tempsense();
		}
#endif
#if defined(RTCONFIG_BCMWL6) && defined(RTCONFIG_PROXYSTA)
		else if (strcmp(argv[1], "psta_monitor") == 0) {
			if (on) start_psta_monitor();
			else stop_psta_monitor();
		}
#endif
#if defined(RTCONFIG_AMAS) && (defined(RTCONFIG_BCMWL6) || defined(RTCONFIG_LANTIQ) || defined(RTCONFIG_QCA) || defined(RTCONFIG_REALTEK) || defined(RTCONFIG_RALINK))
		else if (strcmp(argv[1], "obd") == 0) {
			if (on) start_obd();
			else stop_obd();
		}
#endif
#ifdef RTCONFIG_IPERF
		else if (strcmp(argv[1], "monitor") == 0) {
			if (on) start_monitor();
			else stop_monitor();
		}
#endif
		else if (strcmp(argv[1], "qos") == 0) {//qos test
			if (on) {
#ifdef RTCONFIG_RALINK
				if (module_loaded("hw_nat"))
				{
#if defined (RTCONFIG_WLMODULE_MT7615E_AP)
					doSystem("iwpriv %s set hw_nat_register=%d", get_wifname(0), 0);
#ifdef RTCONFIG_HAS_5G
					doSystem("iwpriv %s set hw_nat_register=%d", get_wifname(1), 0);
#endif
#endif
					modprobe_r("hw_nat");
					sleep(1);
#if 0
					f_write_string("/proc/sys/net/ipv4/conf/default/force_igmp_version", "0", 0, 0);
					f_write_string("/proc/sys/net/ipv4/conf/all/force_igmp_version", "0", 0, 0);
#endif
				}
#endif
			add_iQosRules(get_wan_ifname(wan_primary_ifunit()));
#ifdef RTCONFIG_BWDPI
				if (nvram_get_int("qos_type") == 1) {
					start_dpi_engine_service();
				}

				else
#endif
				start_iQos();
			}
			else
			{
#ifdef RTCONFIG_RALINK
				if (nvram_get_int("hwnat") &&
					/* TODO: consider RTCONFIG_DUALWAN case */
//					!nvram_match("wan0_proto", "l2tp") &&
//					!nvram_match("wan0_proto", "pptp") &&
//					!(nvram_get_int("fw_pt_l2tp") || nvram_get_int("fw_pt_ipsec") &&
//					(nvram_match("wl0_radio", "0") || nvram_get_int("wl0_mrate_x")) &&
//					(nvram_match("wl1_radio", "0") || nvram_get_int("wl1_mrate_x")) &&
					!module_loaded("hw_nat"))
				{
#if 0
					f_write_string("/proc/sys/net/ipv4/conf/default/force_igmp_version", "2", 0, 0);
					f_write_string("/proc/sys/net/ipv4/conf/all/force_igmp_version", "2", 0, 0);
#endif

#if defined(RTN14U) || defined(RTAC52U) || defined(RTAC51U) || defined(RTN11P) || defined(RTN300) || defined(RTN54U) || defined(RTAC1200HP) || defined(RTN56UB1) || defined(RTAC54U) || defined(RTN56UB2) || defined(RTAC1200GA1) || defined (RTAC1200GU) || defined(RTAC85U) || defined(RTAC85P) || defined(RTAC51UP) || defined(RTAC53) || defined(RTN800HP) || defined(RTACRH26)
					if (!(!nvram_match("switch_wantag", "none")&&!nvram_match("switch_wantag", "")))
#endif
					{
						modprobe("hw_nat");
#if defined (RTCONFIG_WLMODULE_MT7615E_AP)
						doSystem("iwpriv %s set hw_nat_register=%d", get_wifname(0), 1);
#ifdef RTCONFIG_HAS_5G
						doSystem("iwpriv %s set hw_nat_register=%d", get_wifname(1), 1);
#endif
#endif
#if defined (RTCONFIG_WLMODULE_MT7615E_AP)
						doSystem("iwpriv %s set LanNatSpeedUpEn=%d", get_wifname(0), 1);
#ifdef RTCONFIG_HAS_5G
						doSystem("iwpriv %s set LanNatSpeedUpEn=%d", get_wifname(1), 1);
#endif
#endif
						sleep(1);
					}
				}
#endif
#ifdef RTCONFIG_BWDPI
				if (nvram_get_int("qos_type") == 1) {
					stop_dpi_engine_service(1);
				}
				else
#endif
				stop_iQos();
				del_iQosRules();
			}
		}
#ifdef RTCONFIG_WEBDAV
		else if (strcmp(argv[1], "webdav") == 0) {
			if (on)
				start_webdav();
		}
#endif
#ifdef RTCONFIG_TUNNEL
		else if (strcmp(argv[1], "mastiff") == 0) {
			if (on)
				start_mastiff();
		}
#endif
		else if (strcmp(argv[1], "gpiow") == 0) {
			if (argc>=4) set_gpio(atoi(argv[2]), atoi(argv[3]));
		}
		else if (strcmp(argv[1], "gpior") == 0) {
			printf("%d\n", get_gpio(atoi(argv[2])));
		}
#if defined(RTCONFIG_HND_ROUTER_AX_6710) || defined(RTAX58U) || defined(TUFAX3000) || defined(TUFAX5400) || defined(RTAX82U) || defined(RTAX82_XD6) || defined(RTAX82_XD6S) || defined(GSAX3000) || defined(GSAX5400) || defined(BCM6756) || defined(GTAX6000) || defined(RTAX86U_PRO) || defined(BCM6855) || defined(RTAX82U_V2)
		else if (strcmp(argv[1], "gpio2r") == 0) {
			printf("%d\n", get_gpio2(atoi(argv[2])));
		}
#endif
#ifndef HND_ROUTER
		else if (strcmp(argv[1], "gpiod") == 0) {
			if (argc>=4) gpio_dir(atoi(argv[2]), atoi(argv[3]));
		}
#endif
#if defined(RTCONFIG_HND_ROUTER_AX_6756)
		else if (strcmp(argv[1], "bootstate") == 0) {
			chk_same_boot_policy();
		}
		else if (strcmp(argv[1], "syncboot") == 0) {
			sync_boot_state();
		}
#endif
		else if (strcmp(argv[1], "init_switch") == 0) {
			init_switch();
		}
		else if (strcmp(argv[1], "set_action") == 0) {
			set_action(on);
		}
		else if (strcmp(argv[1], "pwr_usb") == 0) {
			set_pwr_usb(atoi(argv[2]));
			_dprintf("done.\n");
		}
#ifdef RT4GAC68U
		else if (strcmp(argv[1], "pwr_modem") == 0) {
			set_pwr_modem(atoi(argv[2]));
			_dprintf("done.\n");
		}
#endif
		else if (strcmp(argv[1], "enc_chk") == 0) {
			unsigned char enc_buf[ENC_WORDS_LEN];
			unsigned char dec_buf[DATA_WORDS_LEN + 1];

			_dprintf("get enc str:[%s]\n", enc_str(argv[2], (char *) enc_buf));
			_dprintf("get dec str:[%s]\n", dec_str((char *) enc_buf, (char *) dec_buf));

			_dprintf("done(%d)\n", strcmp(argv[2], (const char *) dec_buf));
		}
#ifdef RTCONFIG_BCMFA
		else if (strcmp(argv[1], "fa_rev") == 0) {
			_dprintf("(%d) done.\n", get_fa_rev());
		}
		else if (strcmp(argv[1], "fa_dump") == 0) {
			_dprintf("(%d) done.\n", get_fa_dump());
		}
#endif
#ifdef RTCONFIG_ASUSCTRL
		else if (strcmp(argv[1], "asusctrl") == 0) {
			printf("ignore=%d, en=%d, flag=(0x%x)\n", asus_ctrl_ignore(), asus_ctrl_en(atoi(argv[2])), nvram_get_hex("asusctrl_flags"));
		}
#endif
#ifdef RTCONFIG_BROOP
		else if (strcmp(argv[1], "broop") == 0) {
			int i, dtime = atoi(argv[2])?:10;

			for(i=0; i<dtime; ++i) {
				sleep(1);
				_dprintf("detect broop: (%d) (max:%d)\n", i, ismax_broop());
			}
		}
#endif
#ifdef RTCONFIG_COMFW
		else if (strcmp(argv[1], "dump_models") == 0) {
			int i, cfd = 0;
			cfd = argv[2]?atoi(argv[2]):0;

			for(i=1; i<MODEL_MAX; ++i) {
				_dprintf("%s: %d : %d\n", asus_models_str[i], i, cfd ? get_cf_id(i, NULL) : 0);
			}		
		}
		else if (strcmp(argv[1], "dump_cfid_byname") == 0) {
			dump_cfid_from_modellist();
		}
		else if (strcmp(argv[1], "dump_cft") == 0) {
			int i;

			for(i=1; i<MAX_FTYPE; ++i) {
				_dprintf("%s: %d \n", comfw_modid_s[i], i);
			}
		}
		else if (strcmp(argv[1], "cfid") == 0) {
			_dprintf("modelid:%d, model's cfid=%d\n", get_model(), get_cf_id(get_model(), NULL));

			int target = argv[2]? atoi(argv[2]):0;

			if(target)
				_dprintf("chk target(%d)'s cfid=%d\n", target, get_cf_id(target, NULL));
		}
#endif
#if defined(CONFIG_BCMWL5)
		else if (strcmp(argv[1], "txpwr_max") == 0) {
			char *wlif = argv[2];
			double max_txpwr;

			max_txpwr = (double)wl_get_txpwr_target_max(wlif);

			_dprintf("Chk txpwr_target_max w/ %s is %f .\n", wlif, max_txpwr);
		}
		else if (strcmp(argv[1], "txpwr_max_band") == 0) {
			int unit = atoi(argv[2]);
			double max_txpwr;

			max_txpwr = (double)get_wifi_maxpower(unit);

			_dprintf("chk txpwr_target_max of unit-%d is %f ...\n", unit, max_txpwr);
		}
#endif
		else {
			printf("what?\n");
		}
	}
	return 0;
}
#endif

#if defined(RTCONFIG_SOC_IPQ8074)
/* Adjust @orig_fw_name if necessary.
 * "IPQ8074/xxx" should be translated as "IPQ8074A/xxx" on Hawkeye 2.0 board.
 * @orig_fw_name:
 * @return:		pointer to new filename, maybe equal to @orig_fw_name
 */
char *fix_fw_name(char *orig_fw_name)
{
	static char fw_name_buf[512];
	unsigned char v;
	char *p, *q;

	if (!orig_fw_name || *orig_fw_name == '\0')
		return orig_fw_name;

	v = get_soc_version_major();
	if (v != 2 || !(q = strstr(orig_fw_name, "IPQ8074/")))
		return orig_fw_name;

	if (sizeof(fw_name_buf) < (strlen(orig_fw_name) + 1 + 1)) {
		dbg("%s: fw_name_buf too small. orig_fw_name [%s]\n", __func__, orig_fw_name);
		return orig_fw_name;
	}


	/* replace "IPQ8074/" as "IPQ8074A/" */
	p = stpncpy(fw_name_buf, orig_fw_name, q - orig_fw_name);
	p = stpcpy(p, "IPQ8074A/");
	p = stpcpy(p, q + strlen("IPQ8074/"));

	return fw_name_buf;
}
#else
static inline char *fix_fw_name(char *orig_fw_name) { return orig_fw_name; }
#endif

#if defined(RTCONFIG_QCA) || defined(RTCONFIG_LANTIQ) || defined(RTAX95Q) || defined(XT8PRO) || defined(XT8_V2) || defined(RTAXE95Q) || defined(ET8PRO) || defined(RTAX56_XD4) || defined(XD4PRO) || defined(RTAX55) || defined(RTAX1800)
/* download firmware */
#ifndef FIRMWARE_DIR
#if defined(RTCONFIG_QCA) || defined(RTAX95Q) || defined(XT8PRO) || defined(XT8_V2) || defined(RTAXE95Q) || defined(ET8PRO) || defined(RTAX56_XD4) || defined(XD4PRO) || defined(RTAX55) || defined(RTAX1800)
#define FIRMWARE_DIR	"/lib/firmware"
#else
#define FIRMWARE_DIR	"/tmp"
#endif
#endif
#ifndef FW_BUF_SIZE
#define FW_BUF_SIZE	4096
#endif
static int hotplug_firmware(void)
{
	int ret = EINVAL;
	FILE *f_fw = NULL, *f_loading = NULL, *f_data = NULL;
	char sysfs_path[PATH_MAX], fw_path[PATH_MAX];
	unsigned char buf[FW_BUF_SIZE] __attribute__((aligned(4)));
	char *action, *devpath, *fw_name;
	const char *fw_root_tbl[] = {
		FIRMWARE_DIR,
#if defined(RTCONFIG_GLOBAL_INI)
		GLOBAL_INI_TOPDIR,
#endif
#if defined(RTCONFIG_QCA_WLAN_SCRIPTS)
		"/ini",
#endif
		NULL
	}, **fw_root;
	char *sysfs_root = "/sys";
	const void *hook_data;
	size_t hook_size = 0, len, tlen = 0;

	action = getenv("ACTION");
	devpath = getenv("DEVPATH");
	fw_name = getenv("FIRMWARE");
	if (!action || !devpath || !fw_name) {
		_dprintf("ACTION (%s), DEVPATH (%s), or FIRMWARE (%s) are NULL!\n",
			(!action)?"<NULL>":action, (!devpath)?"<NULL>":devpath, (!fw_name)?"<NULL>":fw_name);
		return EINVAL;
	}
	if (strcmp(action, "add"))	/* Only "add" action is required to support downloade firmware */
		return 0;

	fw_name = fix_fw_name(fw_name);

	// Generate filename that are required.
	sysfs_path[0] = fw_path[0] = '\0';
	sprintf(sysfs_path, "%s/%s/loading", sysfs_root, devpath);
	f_loading = fopen(sysfs_path, "w");
	if (!f_loading) {
		_dprintf("Open %s/%s/loading fail, errno %d (%s)\n",
			sysfs_root, devpath, errno, strerror(errno));
		goto err_exit1;
	}
	sprintf(sysfs_path, "%s/%s/data", sysfs_root, devpath);
	f_data = fopen(sysfs_path, "wb");

	hook_data = req_fw_hook(fw_name, &hook_size);
	if (!hook_data || !hook_size) {
		for (fw_root = &fw_root_tbl[0]; f_fw == NULL && *fw_root != NULL; ++fw_root) {
			sprintf(fw_path, "%s/%s", *fw_root, fw_name);
			f_fw = fopen(fw_path, "rb");
		}
	}
	// If open firmware successful, notify kernel we are going to download firmware.
	// If open firmware failure, notify kernel we cannot get firmware
	if ((hook_data && hook_size) || (f_fw && f_data)) {
		fputs ("1", f_loading);
	} else {
		fputs ("-1", f_loading);
		_dprintf("Open data (%s,%p) or firmware (%s,%p) failure\n", sysfs_path, f_data, fw_path, f_fw);
		goto err_exit2;
	}
	fflush (f_loading);

	/* Download firmware */
	if (hook_data && hook_size) {
		tlen = len = hook_size;
		while (tlen > 0) {
			len = fwrite(hook_data + (hook_size - tlen), 1, len, f_data);
			tlen -= len;
		}
	} else {
		while (!feof(f_fw)) {
			len = fread(buf, 1, FW_BUF_SIZE, f_fw);
			len = fwrite(buf, 1, len, f_data);
			tlen += len;
		}
	}
	fflush (f_data);

	/* Notify kernel the downloading process had finished */
	fputs ("0", f_loading);
	fflush (f_loading);

	ret = 0;

err_exit2:
	if (f_loading)
		fclose(f_loading);
	if (f_data)
		fclose(f_data);
	if (f_fw)
		fclose(f_fw);
err_exit1:
	return ret;
}
#endif

#if defined(RTCONFIG_SOC_IPQ8074)
static int count_q6mem_size(const char *basedir, const struct dirent *de, void *arg)
{
	uint64_t *space = arg;
	struct stat s = { 0 };
	char path[sizeof("/jffs/dmesg_YYYYMMDD_HHMMSS.txtXXX")];

	if (!basedir || !de || !arg)
		return -1;

	if (strncmp(de->d_name, "q6mem_", 6) && strncmp(de->d_name, "dmesg_", 6))
		return 0;

	snprintf(path, sizeof(path), "%s/%s", basedir, de->d_name);
	if (stat(path, &s) == -1 || S_ISDIR(s.st_mode))
		return 0;

	*space += s.st_size;
	return 0;
}

static int del_q6mem(const char *basedir, const struct dirent *de, void *arg)
{
	char path[sizeof("/jffs/dmesg_YYYYMMDD_HHMMSS.txtXXX")];

	if (!basedir || !de)
		return -1;

	if (strncmp(de->d_name, "q6mem_", 6) && strncmp(de->d_name, "dmesg_", 6))
		return 0;

	snprintf(path, sizeof(path), "%s/%s", basedir, de->d_name);

	unlink(path);
	return 0;
}

/* Splite specified /PATH/TO/q6mem_xxx.gz as 10MB files, store them in @outdir, and then remove original one.
 * @argv[1]:	path and filename to a Q6 crashdump, e.g., /tmp/q6mem_202008041020.gz
 * @argv[2]:	output directory. q6mem_202008041020.gz is splited as q6mem_202008041020.gz.1, q6mem_202008041020.gz.2, etc.
 * @return:
 *  0:		successful
 *  otherwise:	error
 */
int split_q6mem_main(int argc, char *argv[])
{
	int r = 0, c, ifd, ofd;
	off_t tlen;
	size_t map_size;
	ssize_t len, l;
	char *p, *q6mem_fn, *outdir;
	char fn[sizeof("q6mem_YYYYMMDD_HHMMSS.gzXXX")];
	char out_path[128];
	unsigned char *ptr = NULL;

	if (argc < 3)
		return -1;

	q6mem_fn = argv[1];
	outdir = argv[2];
	if (!q6mem_fn || *q6mem_fn == '\0' || !outdir || *outdir == '\0')
		return -1;

	if (!(p = strstr(q6mem_fn, "q6mem_")))
		return - 2;

	strlcpy(fn, p, sizeof(fn));
	if (!(ifd = open(q6mem_fn, O_RDONLY)))
		return -3;

	if ((tlen = lseek(ifd, 0, SEEK_END)) < 0) {
		r = -4;
		goto exit_split_q6mem_main_1;
	}

	map_size = tlen;
	ptr = (unsigned char *)mmap(0, map_size, PROT_READ, MAP_SHARED, ifd, 0);
	if (ptr == (unsigned char *)MAP_FAILED) {
		dbg("%s: Can't map %s. (%s)\n", __func__, q6mem_fn, strerror(errno));
		r = -5;
		goto exit_split_q6mem_main_1;
	}

	for (c = 1, len = 0; !r && tlen > 0; ++c) {
		len = (tlen > (10L * 1048576))? 10L * 1048576 : tlen;
		snprintf(out_path, sizeof(out_path), "%s/%s.%d", outdir, fn, c);
		if ((ofd = creat(out_path, 0777)) == -1) {
			dbg("%s: Can't create %s. (%s)\n", __func__, out_path, strerror(errno));
			r = -6;
			break;
		}

		while (len > 0) {
			l = write(ofd, ptr, len);
			if (l <= 0) {
				dbg("%s: Can't write to %s, %ld remains. (%s)\n", __func__, out_path, len, strerror(errno));
				r = -7;
				break;
			}

			tlen -= l;
			len -= l;
			ptr += l;
		}
		close(ofd);
		sync();
	}

	if (ptr != NULL)
		munmap(ptr, map_size);

 exit_split_q6mem_main_1:
	close(ifd);

	if (!r) {
		unlink(q6mem_fn);
	}

	return r;
}

static int __hotplug_dump_q6mem(void)
{
	int ret = EINVAL;
	time_t now;
	uint64_t space = 25;	/* one copy of compressed q6mem is around 25MB. */
	struct tm *tm = NULL;
	struct statfs s = { 0 };
	char *action, *devicename, *major, *minor;
	char path[128], ts[sizeof("YYYYMMDD_HHMMSSXXX")];
	char gz_cmd[sizeof("gzip -c /dev/q6mem > /jffs/q6mem_XXXXXX.gz") + sizeof(ts)];
	char dmesg_cmd[sizeof("dmesg > /jffs/dmesg_XXXXXX.txt") + sizeof(ts)];
	char cp_cmd[sizeof("sleep 30 ; cp -f /tmp/dmesg_XXX.txt /jffs ; split_q6mem /tmp/q6mem_XXX.gz /jffs &") + 2 * sizeof(ts)];

	action = getenv("ACTION");
	devicename = getenv("DEVICENAME");
	if (!action || strcmp(action, "add") || !devicename) {
		dbg("ACTION (%s), DEVICENAME (%s) \n", action? : "<NULL>", devicename? : "<NULL>");
		return ret;
	}

	snprintf(path, sizeof(path), "/dev/%s", devicename);
	if (!f_exists(path)) {
		major = getenv("MAJOR");
		minor = getenv("MINOR");
		if (!major || !minor)
			return ret;

		if (mknod(path, S_IFCHR | 0644, makedev(safe_atoi(major), safe_atoi(minor))) && errno != EEXIST) {
			dbg("%s: mknod failed, errno %d (%s)\n", __func__, errno, strerror(errno));
			return ret;
		}
	}

	time(&now);
	if (!(tm = localtime(&now)))
		return ret;

	snprintf(ts, sizeof(ts), "%04d%02d%02d_%02d%02d%02d", tm->tm_year + 1900,
		tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

	/* Delete old Q6 crashdump and dmesgs */
	system("rm -f /tmp/dmesg_*.txt /tmp/q6mem_*.gz");
	readdir_wrapper("/jffs", NULL, count_q6mem_size, &space);
	if ((space >> 20) > 55 || (!statfs("/jffs", &s) && (s.f_bfree * s.f_bsize >> 20) < (27 + 5))) {
		dbg("%s: /jffs/{q6mem,dmesg}_* occupies %"PRIu64" MB\n", __func__, space >> 20);
		readdir_wrapper("/jffs", NULL, del_q6mem, NULL);
	}

	snprintf(dmesg_cmd, sizeof(dmesg_cmd), "dmesg > /tmp/dmesg_%s.txt", ts);
	system(dmesg_cmd);
	snprintf(path, sizeof(path), "/tmp/%s_%s.gz", devicename, ts);
	snprintf(gz_cmd, sizeof(gz_cmd), "gzip -c /dev/%s > %s", devicename, path);
	dbg("%s: gzip /dev/%s to %s ...\n", __func__, devicename, path);
	system(gz_cmd);

	if (nvram_match("Ate_power_on_off_enable", "2")) {
		/* move dmesg* to /jffs, splite /tmp/q6mem_*.gz as several 10MB files to /jffs then remove original one,
		 * then, reboot DUT.
		 */
		snprintf(cp_cmd, sizeof(cp_cmd), "cp -f /tmp/dmesg_%s.txt /jffs ; split_q6mem /tmp/q6mem_%s.gz /jffs", ts, ts);
		system(cp_cmd);
		sync();
		dbg("%s: Got Q6 crashdump in run-in mode, reboot instead!\n", __func__);
		reboot(RB_AUTOBOOT);
	} else {
		system("restart_wireless &");
		dbg("%s: gzip /dev/%s to %s done, copy it to /jffs and restart_wireless\n", __func__, devicename, path);

		/* sleep 30 seconds, move dmesg* to /jffs, splite /tmp/q6mem_*.gz as several 10MB files to /jffs and then remove original one. */
		snprintf(cp_cmd, sizeof(cp_cmd), "sleep 30 ; cp -f /tmp/dmesg_%s.txt /jffs ; split_q6mem /tmp/q6mem_%s.gz /jffs &", ts, ts);
		system(cp_cmd);
	}

	return 0;
}

static int hotplug_dump(void)
{
	int ret = EINVAL;
	char *devicename = getenv("DEVICENAME");
	struct dump_fn_s {
		char *devicename;
		int (*func)(void);
	} dump_fn_tbl[] = {
		{ "q6mem", __hotplug_dump_q6mem },

		{ NULL, NULL }
	}, *p;

	if (!devicename)
		return ret;

	for (p = &dump_fn_tbl[0]; p->devicename != NULL; ++p) {
		if (strcmp(devicename, p->devicename))
			continue;

		if (p->func)
			ret = p->func();
		else
			ret = 0;
		break;
	}

	return ret;
}
#elif defined(RTCONFIG_SOC_IPQ60XX)
static int hotplug_dump(void)
{
	int ret = EINVAL;
	char *devicename = getenv("DEVICENAME");
	char *action = getenv("ACTION");
	char *major = getenv("MAJOR");
	char *minor = getenv("MINOR");
	char tftp_server[16];

	if (!devicename || !action || !major || !minor)
		return ret;

	if (!nvram_get_int("q6_quick_recover"))
		return ret;

	strncpy(tftp_server, nvram_safe_get("q6_tftp_server"), sizeof(tftp_server));
	if (!illegal_ipv4_address(tftp_server)) { /* collect q6 iram & dmesg */
		char dev_path[256], ts[sizeof("YYYYMMDD_HHMMSSXXX")];
		char cmd_buf[256];
		time_t now;
		struct tm *tm = NULL;

		snprintf(dev_path, sizeof(dev_path), "/dev/%s", devicename);
		if (!f_exists(dev_path)) {
			if (mknod(dev_path, S_IFCHR | 0644, makedev(safe_atoi(major), safe_atoi(minor))) && errno != EEXIST) {
				_dprintf("%s: mknod failed, errno %d (%s)\n", __func__, errno, strerror(errno));
				return ret;
			}
		}

		time(&now);
		if (!(tm = localtime(&now)))
			return ret;

		snprintf(ts, sizeof(ts), "%04d%02d%02d_%02d%02d%02d", tm->tm_year + 1900,
			tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

		snprintf(cmd_buf, sizeof(cmd_buf), "dmesg > /tmp/dmesg_%s.txt", ts);
		system(cmd_buf);

		snprintf(cmd_buf, sizeof(cmd_buf), "tftp -p -l /tmp/dmesg_%s.txt -r dmesg_%s.txt %s", ts, ts, tftp_server);
		_dprintf("%s: do [%s]\n", __func__, cmd_buf);
		system(cmd_buf);
		sync();

		snprintf(cmd_buf, sizeof(cmd_buf), "tftp -p -l %s -r q6_%s.bin %s", dev_path, ts, tftp_server);
		_dprintf("%s: do [%s]\n", __func__, cmd_buf);
		system(cmd_buf);
		sync();

		/* send a dummy file to prevent tftp end incompletely */
		system("echo dummy > /tmp/dummy.txt");
		snprintf(cmd_buf, sizeof(cmd_buf), "tftp -p -l /tmp/dummy.txt -r dummy.txt %s", tftp_server);
		system(cmd_buf);
		sync();
	}
	system("restart_wireless &");
	return 0;
}
#endif

static int hotplug_main(int argc, char *argv[])
{
	if (argc >= 2) {
		if (strcmp(argv[1], "net") == 0) {
			hotplug_net();
		}
#ifdef RTCONFIG_USB
		// !!TB - USB Support
		else if (strcmp(argv[1], "usb") == 0) {
			hotplug_usb();
		}
#ifdef LINUX26
		else if (strcmp(argv[1], "block") == 0) {
			hotplug_usb();
		}
#endif
#if defined(LINUX_2_6_36)
		else if (!strcmp(argv[1], "platform"))
			return coma_uevent();
#endif /* LINUX_2_6_36 */
#endif
#if defined(RTCONFIG_QCA) || defined(RTCONFIG_LANTIQ) || defined(RTAX95Q) || defined(XT8PRO) || defined(XT8_V2) || defined(RTAXE95Q) || defined(ET8PRO) || defined(RTAX56_XD4) || defined(XD4PRO) || defined(RTAX55) || defined(RTAX1800)
		else if(!strcmp(argv[1], "firmware")) {
			hotplug_firmware();
		}
#endif
#if defined(RTCONFIG_SOC_IPQ8074) || defined(RTCONFIG_SOC_IPQ60XX)
		else if(!strcmp(argv[1], "dump")) {
			hotplug_dump();
		}
#endif
	}
	return 0;
}

#if defined(RTCONFIG_QCA) && defined(RTCONFIG_TURBO_BTN) && defined(RTCONFIG_BLINK_LED)
/**
 * Blink WiFi LED in router/access-point mode if bit-rate is zero and waiting for it becomes non-zero value in background.
 * If WiFi is disabled, wlX_radio = 0, @onoff must be zero.
 * @led_id:
 * @onoff:
 * @return:
 */
static int do_led_ctrl(enum led_id led_id, int onoff)
{
	enum wl_band_id band = WL_NR_BANDS;
	char *prefix = NULL, *led_gpio = NULL, vap[IFNAMSIZ];
	int t1, t2, fd, ret = 0;
	pid_t pid;
	const int sw_mode = sw_mode();

	if (led_id < 0 || led_id >= LED_ID_MAX)
		return -1;

	/* Don't change BLED setting of WiFi LED, just turn on/off WiFi LED.
	 * Because we use BLED to blink WiFi LED of upper link to reflect status of it too.
	 */
	if (__repeater_mode(sw_mode) || __mediabridge_mode(sw_mode)) {
		led_control(led_id, (!inhibit_led_on() && onoff)? LED_ON : LED_OFF);
		return 0;
	}

	switch (led_id) {
	case LED_5G:
		band = WL_5G_BAND;
		prefix = "wl1_";
		led_gpio = "led_5g_gpio";
		break;
	case LED_5G2:
		band = WL_5G_2_BAND;
		prefix = "wl2_";
		led_gpio = "led_5g2_gpio";
		break;
	default:
		led_control(led_id, (!inhibit_led_on() && onoff)? LED_ON : LED_OFF);
		return 0;
	}

	if (band >= WL_NR_BANDS || !prefix || !led_gpio || absent_band(band))
		return -1;

	/* Switch to daemon mode and wait VAP up for 1 minute. */
	pid = fork();
	if (pid == -1) {
		ret = -1;
		goto exit_do_led_ctrl;
	} else if (pid != 0) {
		exit(EXIT_SUCCESS);
	}

	if (setsid() == -1 || chdir("/") == -1) {
		ret = -1;
		goto exit_do_led_ctrl;
	}

	/* redirect fd's 0,1,2 to /dev/null */
	fd = open("/dev/null", O_RDWR);
	dup2(fd, 0);
	dup2(fd, 1);
	dup2(fd, 2);

	/* Always blink LED due to we need to wait qca-wifi to settle mode, 8 seconds,
	 * even LED is inhibited. (AllLED = 0).
	 */
	led_control(led_id, LED_ON);
	set_bled_udef_pattern(led_gpio, 60, "0 1 0 1 0 1 0 1 0 1 0 1 0 0 0 0 1 1 1 1");
	set_bled_udef_pattern_mode(led_gpio);

	strlcpy(vap, get_wififname(band), sizeof(vap));
	t1 = uptime();
	while ((uptime() - t1) < 10 && !get_radio_status(vap)) {
		sleep(1);
	}

	/* If bit-rate is non-zero value turn on/off WiFi LED directly. */
	if (get_bitrate(vap) > 0) {
		goto exit_do_led_ctrl;
		return 0;
	}

	/* DFS channel is used, blink WiFi LED until bitrate becomes non-zero value or 80 seconds elapsed,
	 * even LED is inhibited. (AllLED = 0).
	 */
	t1 = uptime();
	while ((uptime() - t1) < (10 * 60 + 5) && get_bitrate(vap) <= 0) {
		sleep(1);
	}

	if (get_bitrate(vap) <= 0) {
		t2 = uptime();
		dbg("Bit-rate of %s is 0 for %d seconds.\n", vap, t2 - t1);
		logmessage("Bit-rate of %s is 0 for %d seconds.\n", vap, t2 - t1);
	}

 exit_do_led_ctrl:
	set_bled_normal_mode(led_gpio);
	led_control(led_id, (!inhibit_led_on() && onoff)? LED_ON : LED_OFF);

	return ret;
}
#else
static int do_led_ctrl(enum led_id led_id, int onoff)
{
	if (led_id < 0 || led_id >= LED_ID_MAX)
		return -1;
	return led_control(led_id, onoff);
}
#endif

typedef struct {
	const char *name;
	int (*main)(int argc, char *argv[]);
} applets_t;

static const applets_t applets[] = {
	{ "preinit",			init_main				},
	{ "init",			init_main				},
	{ "console",			console_main				},
#ifdef DEBUG_RCTEST
	{ "rc",				rctest_main				},
#endif
	{ "ip-up",			ipup_main				},
	{ "ip-down",			ipdown_main				},
	{ "ip-pre-up",			ippreup_main				},
#ifdef RTCONFIG_IPV6
	{ "ipv6-up",			ip6up_main				},
	{ "ipv6-down",			ip6down_main				},
#endif
	{ "auth-fail",			authfail_main				},
#ifdef RTCONFIG_VPNC
	{ "vpnc-ip-up",			vpnc_ipup_main				},
	{ "vpnc-ip-down",		vpnc_ipdown_main			},
	{ "vpnc-ip-pre-up",		vpnc_ippreup_main			},
	{ "vpnc-auth-fail",		vpnc_authfail_main			},
#ifdef RTCONFIG_VPN_FUSION
	{ "ovpnc-up",			vpnc_ovpn_up_main				},
	{ "ovpnc-down",		vpnc_ovpn_down_main			},
	{ "ovpnc-route-up",	vpnc_ovpn_route_up_main			},
	{ "ovpnc-route-pre-down",vpnc_ovpn_route_pre_down_main	},
#endif
#endif
#ifdef RTCONFIG_OPENVPN
	{ "ovpn-up",			ovpn_up_main				},
	{ "ovpn-down",			ovpn_down_main			},
	{ "ovpn-route-up",		ovpn_route_up_main				},
	{ "ovpn-route-pre-down",	ovpn_route_pre_down_main	},
	{ "ovpn-route-pre-down",ovpn_route_pre_down_main	},
#endif
#ifdef RTCONFIG_TPVPN
#ifdef RTCONFIG_OPENVPN
	{ "hmavpn",				hmavpn_main					},
#endif
#ifdef RTCONFIG_WIREGUARD
	{ "nordvpn",			nordvpn_main				},
#endif
#endif
#ifdef RTCONFIG_EAPOL
	{ "wpa_cli",			wpacli_main			},
#endif
	{ "hotplug",			hotplug_main			},
#ifndef HND_ROUTER
#ifdef RTCONFIG_BCMARM
	{ "mtd-write",			mtd_write_main_old		},
	{ "mtd-erase",			mtd_unlock_erase_main_old	},
	{ "mtd-unlock",			mtd_unlock_erase_main_old	},
#else
	{ "mtd-write",			mtd_write_main			},
	{ "mtd-erase",			mtd_unlock_erase_main		},
	{ "mtd-unlock",			mtd_unlock_erase_main		},
#endif
#endif
#if defined(RTCONFIG_DUAL_TRX2)
	{ "fixdmgfw",			fixdmgfw_main			},
#endif
	{ "watchdog",			watchdog_main			},
	{ "check_watchdog",		check_watchdog_main		},
	{ "fwupg_flashing",		fwupg_flashing_main		},
#ifdef RTCONFIG_CONNTRACK
	{ "pctime",			pctime_main			},
#endif
#if ! (defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK))
	{ "watchdog02",			watchdog02_main			},
#endif  /* ! (RTCONFIG_QCA || RTCONFIG_RALINK) */
#ifdef SW_DEVLED
	{ "sw_devled",			sw_devled_main			},
#endif
#if defined(RTAC1200G) || defined(RTAC1200GP)
	{ "wdg_monitor",		wdg_monitor_main		},
#endif
#if defined(CONFIG_BCMWL5) && defined(RTCONFIG_FANCTRL)
	{ "phy_tempsense",		phy_tempsense_main		},
#endif
#if defined(RTCONFIG_BCMWL6) && defined(RTCONFIG_PROXYSTA)
	{ "psta_monitor",		psta_monitor_main		},
#endif
#if defined(RTCONFIG_NBR_RPT)
	{ "nbr_monitor",			nbr_monitor_main			},
#endif
#if defined(RTCONFIG_AMAS) && (defined(RTCONFIG_BCMWL6) || defined(RTCONFIG_LANTIQ) || defined(RTCONFIG_QCA) || defined(RTCONFIG_REALTEK) || defined(RTCONFIG_RALINK))
	{ "obd",			obd_main			},
#endif
#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_ETHOBD)
	{ "obd_eth",		obdeth_main				},
	{ "obd_monitor",	obd_monitor_main			},
#endif
#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_CFGSYNC)
#if defined(RTCONFIG_HND_ROUTER_AX)
	{ "rmd",			rmd_main			},
#endif
#endif
#ifdef RTCONFIG_IPERF
	{ "monitor",			monitor_main			},
#endif
#ifdef RTCONFIG_QTN
	{ "qtn_monitor",		qtn_monitor_main		},
#endif
#ifdef RTCONFIG_LANTIQ
	{ "wave_monitor",		wave_monitor_main		},
#endif
#if defined(RTCONFIG_USB) && !defined(RTCONFIG_NO_USBPORT)
	{ "usbled",			usbled_main			},
#endif
	{ "ddns_updated", 		ddns_updated_main		},
	{ "ddns_custom_updated",	ddns_custom_updated_main	},
	{ "radio",			radio_main			},
	{ "udhcpc_wan",			udhcpc_wan			},
	{ "udhcpc_lan",			udhcpc_lan			},
	{ "zcip",			zcip_wan			},
#ifdef RTCONFIG_IPV6
	{ "dhcp6c",			dhcp6c_wan			},
#endif
#if defined(RTCONFIG_CONCURRENTREPEATER) || defined(RTCONFIG_AMAS)
#if defined(RTCONFIG_RALINK)
	{ "re_wpsc",			re_wpsc_main			},
#endif
#endif
#if defined(RTCONFIG_CONCURRENTREPEATER)
#if !defined(RPAC68U)
	{ "led_monitor",		led_monitor_main		},
#endif
#if defined(RTCONFIG_RALINK)
	{ "air_monitor",		air_monitor_main		},
#endif
#endif
#ifdef RTCONFIG_WPS
	{ "wpsaide",			wpsaide_main			},
#ifdef RTCONFIG_QCA
	{ "get_wps_er",			get_wps_er_main			},
#if defined(RTCONFIG_CFG80211)
	{ "vap_evhandler",		vap_evhandler_main		},
#endif
#endif
#endif
	{ "halt",			reboothalt_main			},
	{ "reboot",			reboothalt_main			},
#ifndef RTCONFIG_NTPD
	{ "ntp", 			ntp_main			},
#else
	{ "ntpd_synced",		ntpd_synced_main		},
#endif
#ifdef RTCONFIG_NETOOL
	{ "netool", 			netool_main			},
#endif
#ifdef RTCONFIG_SOFTWIRE46
	{ "s46map_rptd", 		s46map_rptd_main		},
#endif
#if defined(RTCONFIG_RALINK) || defined(RTCONFIG_EXT_RTL8365MB) || defined(RTCONFIG_EXT_RTL8370MB) || defined(RTAX55) || defined(RTAX1800) || defined(RTAX58U_V2) || defined(RTAX3000N)
	{ "rtkswitch",			config_rtkswitch		},
#if defined(RTAC53) || defined(RTAC51UP)
	{ "mtkswitch",			config_mtkswitch		},
#endif
#elif defined(RTCONFIG_QCA)
	{ "rtkswitch",			config_rtkswitch		},
#endif
	{ "delay_exec",			delay_main			},

	{ "wanduck",			wanduck_main			},
#if defined(RTCONFIG_CONNDIAG) && defined(RTCONFIG_ADV_RAST)
	{ "conn_diag",			conn_diag_main			},
	{ "diag_data",			diag_data_main			},
#endif
#if defined(CONFIG_BCMWL5) && !defined(HND_ROUTER) && defined(RTCONFIG_DUALWAN)
	{ "dualwan",			dualwan_control			},
#endif
	{ "tcpcheck",			tcpcheck_main			},
	{ "autodet", 			autodet_main			},
#ifdef RTCONFIG_DETWAN
	{ "detwan", 			detwan_main			},
#endif
#ifdef RTCONFIG_QCA_PLC_UTILS
	{ "autodet_plc", 		autodet_plc_main		},
#endif
#ifdef RTCONFIG_CIFS
	{ "mount-cifs",			mount_cifs_main			},
#endif
#ifdef RTCONFIG_USB
	{ "ejusb",			ejusb_main			},
#ifdef RTCONFIG_DISK_MONITOR
	{ "disk_monitor",		diskmon_main			},
#endif
	{ "disk_remove",		diskremove_main			},
#endif
	{ "firmware_check",		firmware_check_main		},
#if defined(RTCONFIG_FRS_LIVE_UPDATE)
	{ "firmware_check_update",	firmware_check_update_main	},
#endif
#ifdef RTAC68U
	{ "firmware_enc_crc",		firmware_enc_crc_main		},
	{ "fw_check",			fw_check_main			},
#endif
#if defined(RTAX82U) || defined(GSAX3000) || defined(GSAX5400) || defined(TUFAX5400) || defined(GTAX11000_PRO) || defined(GTAXE16000) || defined(GTAX6000) || defined(GT10) || defined(RTAX82U_V2)
	{ "ledg",			ledg_main			},
	{ "ledbtn",			ledbtn_main			},
#endif
#if defined(DSL_AX82U)
	{ "ledg",			ledg_main			},
#endif
#if defined(GTAX6000)
	{ "antled",			antled_main			},
#endif
#ifdef BUILD_READMEM
	{ "readmem",			readmem_main			},
#endif
#if defined(RTCONFIG_WIFI_QCN5024_QCN5054) || defined(RTCONFIG_QCA_AXCHIP)
	{ "stress_pktgen",		stress_pktgen_main		},
#endif
#if defined(RTCONFIG_SOC_IPQ8074)
	{ "test_blu",			test_bl_updater_main		},
	{ "split_q6mem",		split_q6mem_main		},
#endif
#if defined(RTCONFIG_SOC_IPQ8074)
	{ "test_switch_log",		test_switch_log_main		},
	{ "test_wifi_stats_log",	test_wifi_stats_log_main	},
#endif
#ifdef RTCONFIG_HTTPS
	{ "rsasign_check",		rsasign_check_main		},
	{ "rsarootca_check",		rsarootca_check_main		},
#endif
	{ "service",			service_main			},
#ifdef RTCONFIG_SPEEDTEST
	{ "speedtest",			speedtest_main			},
#endif
#if defined(RTCONFIG_BWDPI)
	{ "bwdpi",			bwdpi_main			},
	{ "bwdpi_check",		bwdpi_check_main		},
	{ "bwdpi_wred_alive",		bwdpi_wred_alive_main		},
	{ "bwdpi_db_10",		bwdpi_db_10_main		},
	{ "rsasign_sig_check",		rsasign_sig_check_main		},
	{ "hour_monitor",		hour_monitor_main		},
#endif
#ifdef RTCONFIG_AMAS
	{ "amas_lib",		        amas_lib_main			},
#ifdef RTCONFIG_BHCOST_OPT    
	{ "amas_ipc",			amas_ipc_main			},
#endif    
#endif
#ifdef RTCONFIG_USB_MODEM
#ifdef RTCONFIG_INTERNAL_GOBI
	{ "lteled",			lteled_main			},
#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS)
	{ "modem_data",			modem_data_main			},
#endif
#endif
#endif
	{ "dhcpc_lease",		dnsmasq_script_main		},
#ifdef RTCONFIG_NEW_USER_LOW_RSSI
	{ "roamast",			roam_assistant_main		},
#endif
#ifdef RTCONFIG_DHCP_OVERRIDE
	{ "detectWAN_arp",		detectWAN_arp_main		},
#endif
#if defined(RTCONFIG_KEY_GUARD)
	{ "keyguard",			keyguard_main			},
#endif
#ifdef RTCONFIG_LETSENCRYPT
	{ "le_acme",				le_acme_main			},
#endif
#if !(defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK) || defined(RTCONFIG_REALTEK)) \
 ||  (defined(RTCONFIG_SOC_IPQ8074))
	{ "erp_monitor",		erp_monitor_main		},
#endif
#ifdef RTCONFIG_WIFI_SON
#if defined(MAPAC2200)
	{ "dpdt_ant",			dpdt_ant_main		},
#endif
#endif	
#if defined(MAPAC1300) || defined(VZWAC1300) || defined(SHAC1300)
	{ "thermal_txpwr",		thermal_txpwr_main		},
#endif
#ifdef RTCONFIG_ADTBW
	{ "adtbw",			adtbw_main		},
#endif
#ifdef RTCONFIG_AMAS_ADTBW
	{ "amas_adtbw",			amas_adtbw_main              },
#endif
#ifdef RTCONFIG_DSL_HOST
	{ "dsld",				dsld_main				},
#endif	
#if defined(RTCONFIG_QCA_PLC_UTILS) || defined(RTCONFIG_QCA_PLC2)
	{ "restart_plc",		restart_plc_main				},
	{ "detect_plc",			detect_plc_main					},
#endif
#ifdef RTCONFIG_ISP_CUSTOMIZE_TOOL
	{ "tci",			tci_main		},
#endif
#ifdef RTCONFIG_ASUSDDNS_ACCOUNT_BASE
	{ "update_asus_ddns_token",		update_asus_ddns_token_main			},
#endif
	{NULL, NULL}
};

extern void gen_spcmd(char *);
int main(int argc, char **argv)
{
	char *base;
	int f;

	/*
		Make sure std* are valid since several functions attempt to close these
		handles. If nvram_*() runs first, nvram=0, nvram gets closed. - zzz
	*/

	if ((f = open("/dev/null", O_RDWR)) < 0) {
	}
	else if (f < 3) {
		dup(f);
		dup(f);
	}
	else {
		close(f);
	}

	base = strrchr(argv[0], '/');
	base = base ? base + 1 : argv[0];

#if 0
	if (strcmp(base, "rc") == 0) {
		if (argc < 2) return 1;
		if (strcmp(argv[1], "start") == 0) return kill(1, SIGUSR2);
		if (strcmp(argv[1], "stop") == 0) return kill(1, SIGINT);
		if (strcmp(argv[1], "restart") == 0) return kill(1, SIGHUP);
		++argv;
		--argc;
		base = argv[0];
	}
#endif

#if !defined(CONFIG_BCMWL5)
    if (getpid() != 1)
    {
#endif

#if defined(DEBUG_NOISY)
	if (nvram_match("debug_logrc", "1")) {
		int i;

		cprintf("[rc %d] ", getpid());
		for (i = 0; i < argc; ++i) {
			cprintf("%s ", argv[i]);
		}
		cprintf("\n");

	}
#endif

#if defined(DEBUG_NOISY)
	if (nvram_match("debug_ovrc", "1")) {
		char tmp[256];
		char *a[32];

		realpath(argv[0], tmp);
		if ((strncmp(tmp, "/tmp/", 5) != 0) && (argc < 32)) {
			snprintf(tmp, sizeof(tmp),"%s%s", "/tmp/", base);
			if (f_exists(tmp)) {
				cprintf("[rc] override: %s\n", tmp);
				memcpy(a, argv, argc * sizeof(a[0]));
				a[argc] = 0;
				a[0] = tmp;
				execvp(tmp, a);
				exit(0);
			}
		}
	}
#endif
#if !defined(CONFIG_BCMWL5)
    }
#endif
	const applets_t *a;
	for (a = applets; a->name; ++a) {
		if (strcmp(base, a->name) == 0) {
			openlog(base, LOG_PID, LOG_USER);
			return a->main(argc, argv);
		}
	}

#if defined(RTCONFIG_HW_DOG)
	if(!strcmp(base, "hwdog")) {
		return hwdog_main(argc, argv);
	}
#endif

#if defined(RTCONFIG_PTHSAFE_POPEN)
	if(!strcmp(base, "PS_pod")){
		PS_pod_main();
		return 0;
	}
#endif

#ifdef RTCONFIG_WIFI_SON
        if(!strcmp(base, "hive_cap")){
                if(nvram_get_int("sw_mode")==SW_MODE_ROUTER || nvram_match("cfg_master", "1")) {
                        printf("start central ap...\n");
                        if (argv[1] && (!strcmp(argv[1], "start")))
                                start_cap(0);
                        else if (argv[1] && (!strcmp(argv[1], "config_change")))
                                start_cap(1);
                        else if (argv[1] && (!strcmp(argv[1], "restart")))
                                start_cap(2);
#if defined(RTCONFIG_HIDDEN_BACKHAUL)
                        else if (argv[1] && (!strcmp(argv[1], "renew_bh")))
                                renew_bh();
#endif
                        else
                                printf("error command.\n");
                }
     		else
                        printf("Error. CAP should run in router mode.\n");
                return 0;
        }

        if(!strcmp(base, "hive_re")){
                if (nvram_get_int("sw_mode")==SW_MODE_AP && !nvram_match("cfg_master", "1")) {
                        printf("start range extender ...\n");
                        if (argv[1] && (!strcmp(argv[1], "start")))
                                start_re(0);
                        else if (argv[1] && (!strcmp(argv[1], "reset")))
                                start_re(1);
                        else if (argv[1] && (!strcmp(argv[1], "restart")))
                                start_re(2);
                        else if (argv[1] && (!strcmp(argv[1], "waitimeout")))
                                start_re(3);
#if defined(RTCONFIG_HIDDEN_BACKHAUL)
                        else if (argv[1] && (!strcmp(argv[1], "renew_bh")))
                                renew_bh();
#endif
                        else
                                printf("error command.\n");
                }
                else
        		printf("Error. RE should run in AP mode.\n");
                return 0;
       }
        if(!strcmp(base, "hive_cmd")){
                if(nvram_get_int("sw_mode")==SW_MODE_ROUTER || nvram_match("cfg_master", "1")) {
                        printf("start central ap...\n");
			if(argv[1] && strlen(argv[1])) {
				if (!strcmp(argv[1], "reboot"))
					gen_spcmd("xx");
                                start_cmd(argv[1]);
			}
                        else
                                printf("error command.\n");
                }
     		else
                        printf("Error. CAP should run in router mode.\n");
                return 0;
        }
	if(!strcmp(base, "hive_wsplcd")){
                start_wsplcd();
                return 0;
       }
	if(!strcmp(base, "hive_hyd")){
                start_hyd();
                return 0;
       }
#ifdef RTCONFIG_ETHBACKHAUL
        if(!strcmp(base, "hive_eth")){
                if (nvram_get_int("sw_mode")==SW_MODE_AP && !nvram_match("cfg_master", "1")) {
			nvram_set("eth_detect_proc","1");
                        if (argv[1] && (!strcmp(argv[1], "1")))
                                start_eth(1);
                        else if (argv[1] && (!strcmp(argv[1], "0")))
			{
				_dprintf("run when upstream plugout ...RE\n");
                                start_eth(0);
			}
                        else if (argv[1] && (!strcmp(argv[1], "2")))
			{
				_dprintf("run when downstream plugout ...RE\n");
                                start_eth(2);
			}
                        else if (argv[1] && (!strcmp(argv[1], "3")))
                                start_eth(3);

			nvram_set("eth_detect_proc","0");
			nvram_set_int("prelink_pap_status", -1); // trigger LED to change
                }
		else if (nvram_get_int("sw_mode")==SW_MODE_ROUTER || nvram_match("cfg_master", "1")) {
                        if (argv[1] && (!strcmp(argv[1], "1")))
			{
				start_eth(1); /* move to repacd */
			}
                        else if (argv[1] && (!strcmp(argv[1], "0")))
			{
				nvram_set("eth_detect_proc","1");
				_dprintf("run when lan plugout...CAP\n");
				start_eth(0); /* move to repacd */
				nvram_set("eth_detect_proc","0");
			}
			nvram_set_int("prelink_pap_status", -1); // trigger LED to change
		}
                else
        		printf("Error command.\n");
                return 0;
       }

       if(!strcmp(base, "eth_bh_mon")){
		start_eth_bh_mon();
                return 0;
       }

       if(!strcmp(base, "ethbh_peer_detect")){
		if(argc >= 4)
			ethbh_peer_detect(argv[1], argv[2], argv[3]);
                return 0;
       }
#endif
       if(!strcmp(base, "wifimon_check")){
		int default_sec=0;
                if (nvram_get_int("sw_mode")==SW_MODE_AP && !nvram_match("cfg_master", "1")) {
			if(argv[1] && strlen(argv[1]))
			{
				if (safe_atoi(argv[1])>0 && safe_atoi(argv[1])<240)
				{
					start_wifimon_check(safe_atoi(argv[1]));
					return 0;
				}
			}
			start_wifimon_check(default_sec);
		}
		else
			printf("Error. wifimon_check should run in RE.\n");
		return 0;
	}
#endif /* RTCONFIG_WIFI_SON */
#if defined(RTCONFIG_LP5523)
/*
 * Manual setting LP5523 leds
 * usage: lp55xx_set_led [behavior_mode] [Blue_vol] [Green_vol] [Red_vol]
 *
 * Behavior:	300~xxx(xxx please refer to lp55xx_leds_mode in shared/lp5523led.h)
 * B_vol:	0~255
 * G_vol:	0~255
 * R_vol:	0~255
 * Brightness:	0~100
 *
 * */
	if (!strcmp(base, "lp55xx_set_led")) {
		if (argv[1] && argv[2] && argv[3] && argv[4] && argv[5])
		{
			char tmp[32];
			int ptb_mode=0;
			int i=0;
			char *p = tmp;

			memset(tmp, '\0', 32);
			while (argv[i])
			{
				if (i>5) {
					printf("Error. Enter too many parameters.\n");
					return 0;
				}

				switch (i) {
				case 1:
					if (safe_atoi(argv[i])>LP55XX_END_BLINK || safe_atoi(argv[i])<LP55XX_END_COLOR) {
						printf("Error. Set LED behavior failed. [%d - %d]\n", LP55XX_END_COLOR, LP55XX_END_BLINK);
						return 0;
					}

					ptb_mode = safe_atoi(argv[i]);
					break;
				case 2:
				case 3:
				case 4:
					if (safe_atoi(argv[i])>255 || safe_atoi(argv[i])<0) {
						printf("Error. Set LED(%d) power failed. [%d - %d]\n", i-1, 0, 255);
						return 0;
					}

					if (i==2)
						p += sprintf(p, "0x%s%x", safe_atoi(argv[i])<16?"0":"", safe_atoi(argv[i]));
					else
						p += sprintf(p, "_0x%s%x", safe_atoi(argv[i])<16?"0":"", safe_atoi(argv[i]));

					break;
				case 5:
					if (safe_atoi(argv[i])>100 || safe_atoi(argv[i])<0) {
						printf("Error. Set Brightness(%d) failed. [%d - %d]\n", i-1, 0, 100);
						return 0;
					}
					nvram_set_int("lp55xx_lp5523_user_brightness", safe_atoi(argv[i]));

					break;
				}

				i++;
			}

			nvram_set("lp55xx_lp5523_manual", tmp);
			lp55xx_leds_proc(LP55XX_MANUAL_COL, ptb_mode);
		}
		else
			printf("Error. Enter parameter failed.\n\n"
				"Usage: lp55xx_set_led [behavior_mode] [Blue_vol] [Green_vol] [Red_vol] [Brightness] \n\n"
				"Behavior: 300~xxx(xxx please refer to lp55xx_leds_mode in shared/lp5523led.h)\n"
				"XXX_vol:  0~255\n"
				"Brightness: 1~100\n");

		return 0;
	}
/*
 * Manual setting LP5523 leds
 * usage: lp55xx_user_set [enable] [color] [behavior] [brightness] [start] [end]
 *
 * enable:	0:OFF, 1:ON
 * color:	100~xxx(xxx please refer to lp55xx_leds_mode in shared/lp5523led.h)
 * behavior:	300~xxx(xxx please refer to lp55xx_leds_mode in shared/lp5523led.h)
 * brightness:	10~100(%)
 * start:	0~24(schedule)/ 99(user defined led)
 * end:		0~24(schedule)/ 99(user defined led)
 *
 * */
	if (!strcmp(base, "lp55xx_user_set")) {
		if (argv[1] && argv[2] && argv[3] && argv[4] && argv[5] && argv[6])
		{
			int i=0, prestate=0;
			int ptc_mode=0, ptb_mode=0, brightness=0;
			int enable=0, sch_start=0, sch_end=0;

			while (argv[i])
			{
				if (i>7)
				{
					printf("Error. Enter too many parameters.\n");
					return 0;
				}

				switch (i) {
				case 1:
					enable = safe_atoi(argv[i]);

					if (enable>2 || enable<0)
					{
						printf("Error. Set LED enable failed. [%d - %d]\n", 0 , 1);
						return 0;
					}
					break;
				case 2:
					ptc_mode = safe_atoi(argv[i]);

					if (ptc_mode>LP55XX_END_COLOR || ptc_mode<LP55XX_ALL_BREATH_LEDS)
					{
						printf("Error. Set LED color failed. [%d - %d]\n", LP55XX_ALL_BREATH_LEDS, LP55XX_END_COLOR);
						return 0;
					}
					break;
				case 3:
					ptb_mode = safe_atoi(argv[i]);

					if (ptb_mode>LP55XX_END_BLINK || ptb_mode<LP55XX_ACT_NONE)
					{
						printf("Error. Set LED behavior failed. [%d - %d]\n", LP55XX_ACT_NONE, LP55XX_END_BLINK);
						return 0;
					}
					break;
				case 4:
					brightness = safe_atoi(argv[i]);

					if (brightness>100 || brightness<10)
					{
						printf("Error. Set LED brightness failed. [%d - %d]\n", 10 , 100);
						return 0;
					}
					break;
				case 5:
					sch_start = safe_atoi(argv[i]);
				case 6:
					sch_end = safe_atoi(argv[i]);

					if ( sch_end>24 || sch_end<0)
					{
						if (sch_end!=99)
						{
							printf("Error. Set LED %s time failed. [%d - %d] or %d to set user defininaton\n", i==5?"start":"end", 0, 24, 99);
							return 0;
						}
					}
					break;
				}

				i++;
			}

			if (sch_start==99 || sch_end==99)
			{
				if (enable)
				{
					nvram_set_int("lp55xx_lp5523_user_col", ptc_mode);
					nvram_set_int("lp55xx_lp5523_user_beh", ptb_mode);
					nvram_set_int("lp55xx_lp5523_user_brightness", brightness);
				}

				prestate = 1;
				nvram_set_int("lp55xx_lp5523_user_enable", enable);
			}
			else
			{
				if (enable)
				{
					nvram_set_int("lp55xx_lp5523_sch_col", ptc_mode);
					nvram_set_int("lp55xx_lp5523_sch_beh", ptb_mode);
					nvram_set_int("lp55xx_lp5523_sch_brightness", brightness);
					lp55xx_leds_sch(sch_start, sch_end);
				}
				else if (nvram_get_int("lp55xx_lp5523_sch_enable"))
					prestate = 1;

				nvram_set_int("lp55xx_lp5523_sch_enable", enable);

				if (enable==2)
					lp55xx_leds_proc(LP55XX_ALL_LEDS_OFF, LP55XX_SCH_ENABLE);
			}

			if (prestate)
				lp55xx_leds_proc(LP55XX_ALL_LEDS_OFF, LP55XX_PREVIOUS_STATE);


			nvram_commit();
		}
		else
			printf("Error. Enter parameter failed.\n\n"
				"Usage: lp55xx_user_set [enable] [color_mode] [behavior_mode] [brightness] [start time] [end time]\n\n"
				"enable:  	0:OFF, 1:ON\n"
				"color:  	100~xxx(xxx please refer to lp55xx_leds_mode in shared/lp5523led.h)\n"
				"behavior:  	300~xxx(xxx please refer to lp55xx_leds_mode in shared/lp5523led.h)\n"
				"brightness:  	10~100\n"
				"start:  	0~24(schedule)/ 99(user defined led)\n"
				"end:  		0~24(schedule)/ 99(user defined led)\n");

		return 0;
	}
#endif
#if defined(RTCONFIG_QCA)
/*
 * QCA create_tmp_sta wrapper
 * usage: start_tmpsta band_no sta_name ssid_prefix(or "NULL")
 * */
	if (!strcmp(base, "tmpsta")) {
		if (argv[1] && argv[2] && argv[3]) {
			unsigned char band;
			char *prefix;
			if (argv[1][0]=='\0' || argv[2][0]=='\0' || argv[3][0]=='\0')
				return 0;
			band = (unsigned char)atoi(argv[1]);
			if (band >= MAX_NR_WL_IF)
				return 0;
			if (!strncmp(argv[3], "NULL", 4))
				prefix = NULL;
			else
				prefix = argv[3];
			_dprintf("tmpsta wrap:[%d]/[%s]/[%s]\n", band, argv[2], prefix);
			create_tmp_sta(band, argv[2], prefix);
		}
		return 0;
	}
#endif

	if (!strcmp(base, "restart_wireless")) {
		printf("restart wireless...\n");
		restart_wireless();
		return 0;
	}
#if defined(RTCONFIG_BCMWL6) && defined(RTCONFIG_PROXYSTA)
	else if (!strcmp(base, "sendarp")) {
		send_arpreq();
		return 0;
	}
#endif
#ifdef RTCONFIG_BCM_7114
	else if (!strcmp(base, "stop_wl")) {
		stop_wl_bcm();
		return 0;
	}
#endif
#ifdef RTCONFIG_USB
#ifdef RTCONFIG_ERPTEST
	else if (!strcmp(base, "restart_usb")) {
		int f_stop = 0;
		if (argc == 2)
			f_stop = atoi(argv[1]);
		printf("%s usb...\n", f_stop ? "stop" : "restart");
		restart_usb(f_stop);
		return 0;
	}
#endif
	else if (!strcmp(base, "get_apps_name")) {
		if (argc != 2) {
			printf("Usage: get_apps_name [File name]\n");
			return 0;
		}

		return get_apps_name(argv[1]);
	}
#ifdef BCM_MMC
	else if (!strcmp(base, "asus_mmc")) {
		if (argc != 3) {
			printf("Usage: asus_mmc [device_name] [action]\n");
			return 0;
		}

		return asus_mmc(argv[1], argv[2]);
	}
#endif
	else if (!strcmp(base, "asus_sd")) {
		if (argc != 3) {
			printf("Usage: asus_sd [device_name] [action]\n");
			return 0;
		}

		return asus_sd(argv[1], argv[2]);
	}
	else if (!strcmp(base, "asus_lp")) {
		if (argc != 3) {
			printf("Usage: asus_lp [device_name] [action]\n");
			return 0;
		}

		return asus_lp(argv[1], argv[2]);
	}
	else if (!strcmp(base, "asus_sg")) {
		if (argc != 3) {
			printf("Usage: asus_sg [device_name] [action]\n");
			return 0;
		}

		return asus_sg(argv[1], argv[2]);
	}
	else if (!strcmp(base, "asus_sr")) {
		if (argc != 3) {
			printf("Usage: asus_sr [device_name] [action]\n");
			return 0;
		}

		return asus_sr(argv[1], argv[2]);
	}
	else if (!strcmp(base, "asus_tty")) {
		if (argc != 3) {
			printf("Usage: asus_tty [device_name] [action]\n");
			return 0;
		}

		return asus_tty(argv[1], argv[2]);
	}
	else if (!strcmp(base, "asus_usbbcm")) {
		if (argc != 3) {
			printf("Usage: asus_usbbcm [device_name] [action]\n");
			return 0;
		}

		return asus_usbbcm(argv[1], argv[2]);
	}
	else if (!strcmp(base, "asus_usb_interface")) {
		if (argc != 3) {
			printf("Usage: asus_usb_interface [device_name] [action]\n");
			return 0;
		}

		return asus_usb_interface(argv[1], argv[2]);
	}
	else if (!strcmp(base, "get_usb_node_by_string")) {
		char usb_node[16];

		if(argc != 2)
			return 0;

		if(!get_usb_node_by_string(argv[1], usb_node, sizeof(usb_node)))
			return -1;

		printf("%s", usb_node);

		return 0;
	}
	else if (!strcmp(base, "unset_usb_nvram")) {
		if(argc > 2)
			return 0;

		if(argc == 2)
			unset_usb_nvram(argv[1]);
		else
			unset_usb_nvram(NULL);

		return 0;
	}
	else if (!strcmp(base, "detect_usb_devices")) {
		return detect_usb_devices();
	}
	else if (!strcmp(base, "usb_notify")) {
#if defined(RTCONFIG_APP_PREINSTALLED) || defined(RTCONFIG_APP_NETINSTALLED)
		usb_notify();
#endif

		return 0;
	}
#if defined(RTCONFIG_APP_PREINSTALLED) || defined(RTCONFIG_APP_NETINSTALLED)
	else if (!strcmp(base, "run_app_script")) {
		if (argc != 3) {
			printf("Usage: run_app_script [Package name | allpkg] [APP action]\n");
			return 0;
		}

		if (!strcmp(argv[1], "allpkg"))
			return run_app_script(NULL, argv[2]);
		else
			return run_app_script(argv[1], argv[2]);
	}
	else if (!strcmp(base, "chk_app_state")) {
#define PID_FILE "/var/run/chk_app_state.pid"
		FILE *fp;
		char chk_value[4];

		if (f_read_string(PID_FILE, chk_value, 4) > 0
				&& atoi(chk_value) != getpid()) {
			_dprintf("Already running!\n");
			return 0;
		}

		if ((fp = fopen(PID_FILE, "w")) == NULL) {
			_dprintf("Can't open the pid file!\n");
			return 0;
		}

		fprintf(fp, "%d", getpid());
		fclose(fp);

		strlcpy(chk_value, nvram_safe_get("apps_state_switch"), sizeof(chk_value));
		if (strcmp(chk_value, "")) {
			if (atoi(chk_value) != APPS_SWITCH_FINISHED && !pids("app_switch.sh")) {
				_dprintf("Don't have the switch script.\n");
				nvram_set("apps_state_switch", "");
			}

			unlink(PID_FILE);
			return 0;
		}

		strlcpy(chk_value, nvram_safe_get("apps_state_install"), sizeof(chk_value));
		if (strcmp(chk_value, "")) {
			if (atoi(chk_value) != APPS_INSTALL_FINISHED && !pids("app_install.sh")) {
				_dprintf("Don't have the install script.\n");
				nvram_set("apps_state_install", "");
			}

			unlink(PID_FILE);
			return 0;
		}

		strlcpy(chk_value, nvram_safe_get("apps_state_upgrade"), sizeof(chk_value));
		if (strcmp(chk_value, "")) {
			if (atoi(chk_value) != APPS_UPGRADE_FINISHED && !pids("app_upgrade.sh")) {
				_dprintf("Don't have the upgrade script.\n");
				nvram_set("apps_state_upgrade", "");
			}

			unlink(PID_FILE);
			return 0;
		}

		strlcpy(chk_value, nvram_safe_get("apps_state_enable"), sizeof(chk_value));
		if (strcmp(chk_value, "")) {
			if (atoi(chk_value) != APPS_ENABLE_FINISHED && !pids("app_set_enabled.sh")) {
				_dprintf("Don't have the enable script.\n");
				nvram_set("apps_state_enable", "");
			}

			unlink(PID_FILE);
			return 0;
		}

		unlink(PID_FILE);
		return 0;
	}
#endif
#endif
#ifdef RTCONFIG_LANTIQ
	else if(!strcmp(base, "restart_bluetoothd")) {
		system("killall bluetoothd");
		system("hciconfig hci0 down");
		system("hciconfig hci0 reset");
		system("hciconfig hci0 up");
		system("hciconfig hci0 leadv 0");
		system("bluetoothd -n &");
		return 0;
	}
	else if(!strcmp(base, "set_usb3_to_usb2")) {
		set_usb3_to_usb2();
		puts("1");
		return 0;
	}
	else if(!strcmp(base, "set_usb2_to_usb3")) {
		set_usb2_to_usb3();
		puts("1");
		return 0;
	}
	else if (!strcmp(base, "start_repeater")) {
		start_repeater();
		return 0;
	}
#endif
	else if (!strcmp(base, "ATE")) {
		int ret;
		if ( argc == 2 || argc == 3 || argc == 4) {
			ret = asus_ate_command(argv[1], argv[2], argv[3]);
		}
		else {
			ret = -1;
			printf("ATE_ERROR\n");
		}
		return ret;
	}
#if defined(RTCONFIG_DSL)
	else if (!strcmp(base, "asustest")) {
		if ( argc == 2 || argc == 3) {
			asustest_command(argv[1], argv[2]);
		}
		else
			printf("asustest:parameter error\n");
		return 0;
	}
#endif

#if defined(RTCONFIG_RALINK) || defined(RTCONFIG_QCA) || defined(RTCONFIG_ALPINE)
	else if (!strcmp(base, "FWRITE")) {
		if (argc == 3)
			return FWRITE(argv[1], argv[2]);
		else
		return 0;
	}
	else if (!strcmp(base, "FREAD")) {
		if (argc == 3)
		{
			unsigned int addr;
			int len;
			addr = strtoul(argv[1], NULL, 16);
			if (argv[2][0] == '0' && argv[2][1] == 'x')
				len  = (int) strtoul(argv[2], NULL, 16);
			else
				len  = (int) strtoul(argv[2], NULL, 10);

			if (len > 0)
				return FREAD(addr, len);
		}
		printf("ATE_ERROR\n");
		return 0;
	}
#endif	/* RTCONFIG_RALINK || RTCONFIG_QCA */
#if defined(RTCONFIG_RALINK)
#if defined(RTCONFIG_HAS_5G)
	else if (!strcmp(base, "asuscfe_5g")) {
		if (argc == 2)
			return asuscfe(argv[1], WIF_5G);
		else
			return EINVAL;
	}
#endif	/* RTCONFIG_HAS_5G */
	else if (!strcmp(base, "asuscfe_2g")) {
		if (argc == 2)
			return asuscfe(argv[1], WIF_2G);
		else
			return EINVAL;
	}
	else if (!strcmp(base, "stainfo_2g")) {
		return stainfo(0);
	}
#if defined(RTCONFIG_HAS_5G)
	else if (!strcmp(base, "stainfo_5g")) {
		return stainfo(1);
	}
#endif	/* RTCONFIG_HAS_5G */
#ifdef RTCONFIG_DSL
	else if (!strcmp(base, "gen_ralink_config")) {
		if (argc != 3) {
			printf("Usage: gen_ralink_config [band] [is_iNIC]\n");
			return 0;
		}
		return gen_ralink_config(atoi(argv[1]), atoi(argv[2]));
	}
#endif
#endif

#if defined(CONFIG_BCMWL5) || defined(RTCONFIG_RALINK) || defined(RTCONFIG_QCA) || defined(RTCONFIG_REALTEK)
	else if (!strcmp(base, "set_factory_mode")) {
		set_factory_mode();
		return 0;
	}
#endif
#if defined(RTCONFIG_OPENPLUS_TFAT) || defined(RTCONFIG_OPENPLUSPARAGON_NTFS) || defined(RTCONFIG_OPENPLUSTUXERA_NTFS) || defined(RTCONFIG_OPENPLUSPARAGON_HFS) || defined(RTCONFIG_OPENPLUSTUXERA_HFS)
	else if(!strcmp(base, "set_fs_coexist")){
		set_fs_coexist();
		puts("1");
		return 0;
	}
#endif
	else if (!strcmp(base, "run_telnetd")) {
		_start_telnetd(1);
		return 0;
	}
#ifdef RTCONFIG_SSH
	else if (!strcmp(base, "run_sshd")) {
		start_sshd();
		return 0;
	}
#endif
#ifdef RTCONFIG_WTFAST
	else if (!strcmp(base, "run_wtfast")) {
		start_wtfast();
		return 0;
	}
#endif
#if defined(RTCONFIG_PPTPD) || defined(RTCONFIG_ACCEL_PPTPD)
	else if (!strcmp(base, "run_pptpd")) {
		start_pptpd();
		return 0;
	}
#endif
#ifdef RTCONFIG_PARENTALCTRL
	else if (!strcmp(base, "pc")) {
		pc_main(argc, argv);
		return 0;
	}
	else if (!strcmp(base, "pc_block")) {
		pc_block_main(argc, argv);
		return 0;
	}
	else if (!strcmp(base, "pc_tmp")) {
		pc_tmp_main(argc, argv);
		return 0;
	}
#endif
#ifdef RTCONFIG_INTERNETCTRL
	else if (!strcmp(base, "ic")) {
		ic_main(argc, argv);
		return 0;
	}
#endif
#if defined(CONFIG_BCMWL5) \
		|| (defined(RTCONFIG_RALINK) && defined(RTCONFIG_WIRELESSREPEATER)) \
		|| defined(RTCONFIG_QCA) || defined(RTCONFIG_REALTEK) \
		|| defined(RTCONFIG_QSR10G) || defined(RTCONFIG_LANTIQ)
	else if (!strcmp(base, "wlcscan")) {
		return wlcscan_main();
	}
#if defined(RTCONFIG_WLCSCAN_RSSI)
	else if (!strcmp(base, "wlcscan_ssid_rssi")) {
		return wlcscan_ssid_rssi(atoi(argv[1]), argv[2]);
	}
#endif
#ifdef RTCONFIG_QTN
	else if (!strcmp(base, "start_psta_qtn")) {
		return start_psta_qtn();
	}
	else if (!strcmp(base, "start_ap_qtn")) {
		return start_ap_qtn();
	}
	else if (!strcmp(base, "start_channel_scan_qtn")) {
		return start_channel_scan_qtn(1);
	}
	else if (!strcmp(base, "start_qtn_stateless")) {
		return gen_stateless_conf();
	}
	else if (!strcmp(base, "restart_qtn")) {
		return reset_qtn(0);
	}
#endif
#endif
#if defined(CONFIG_BCMWL5) && !defined(HND_ROUTER) && defined(RTCONFIG_DUALWAN)
	else if (!strcmp(base, "dualwan")) {
		dualwan_control(argc, argv);
	}
#endif
#ifdef RTCONFIG_WIRELESSREPEATER
	else if (!strcmp(base, "wlcconnect")) {
		return wlcconnect_main();
	}
#endif
#ifdef RTCONFIG_AMAS
	else if (!strcmp(base, "amas_wlcconnect")) {
		return amas_wlcconnect_main();
	}
	else if (!strcmp(base, "amas_bhctrl")) {
		return amas_bhctrl_main();
	}
	else if (!strcmp(base, "amas_lanctrl")) {
		return amas_lanctrl_main();
	}
#ifdef RTCONFIG_BHCOST_OPT
	else if (!strcmp(base, "amas_status")) {
		return amas_status_main();
	}
	else if (!strcmp(base, "amas_ssd")) {
		return amas_ssd_main();
	}
	else if (!strcmp(base, "amas_misc")) {
		return amas_misc_main();
	}
#endif
#endif
#ifdef CONFIG_BCMWL5
	else if (!strcmp(base, "setup_dnsmq")) {
		if (argc != 2)
			return 0;

		return setup_dnsmq(atoi(argv[1]));
	}
#endif
	else if (!strcmp(base, "add_multi_routes")) {
		if(argc == 2)
			return add_multi_routes(atoi(argv[1]), -1);
		else
			return add_multi_routes(0, -1);
	}
	else if (!strcmp(base, "led_ctrl")) {
		if (argc != 3)
			return 0;

		return do_led_ctrl(atoi(argv[1]), atoi(argv[2]));
	}
	else if (!strcmp(base, "led_str_ctrl")) {
		if(argc != 3){
			if(argc == 2)
				printf("Usage: led_str_ctrl %s [1/0]\n", argv[1]);
			else
				printf("Usage: led_str_ctrl LED_STRING [1/0]\n");
			return 0;
		}

		int led_id = get_led_id(argv[1]);
		int onoff = atoi(argv[2]);

		printf("Switch %s(%d) %s...\n", argv[1], led_id, (onoff)?"on":"off");
		return do_led_ctrl(led_id, atoi(argv[2]));
	}
#ifdef HND_ROUTER
	else if (!strcmp(base, "hnd-erase")) {
		if (argv[1] && (!strcmp(argv[1], "nvram"))) {
			nvram_set("restore_defaults", "1");
			nvram_commit();
			nvram_set(ASUS_STOP_COMMIT, "1");
			return hnd_nvram_erase();
		}
	}
	else if (!strcmp(base, "hnd-write")) {
                int ret = -1;

                nvram_set_int("hndwr", -100);
                if (argc >= 2) {
                        ret = bca_sys_upgrade(argv[1]);
                } else {
                        _dprintf("%s@%d *** Error argc=%d\n", __FUNCTION__, __LINE__, argc);
                        ret = EINVAL;
                }

                nvram_set_int("hndwr", ret);
                return ret;
	}
#ifndef RTAC68U_V4
	else if (!strcmp(base, "mtd_erase_image_update")) {
		return mtd_erase_image_update();
	}
#endif
	else if (!strcmp(base, "mtd_erase_misc2")) {
		return mtd_erase_misc2();
	}
#else
	else if (!strcmp(base, "nvram_erase")) {
		nvram_set(ASUS_STOP_COMMIT, "1");
		erase_nvram();
		return 0;
	}
#ifdef RTCONFIG_BCMARM
#if defined(RTAC1200G) || defined(RTAC1200GP)
	/* mtd-erase2 [device] */
	else if (!strcmp(base, "mtd-erase2")) {
		if (argv[1] && ((!strcmp(argv[1], "boot")) ||
				(!strcmp(argv[1], "linux")) ||
				(!strcmp(argv[1], "rootfs")) ||
				(!strcmp(argv[1], "nvram")))) {
			return mtd_erase(argv[1]);
		} else {
			fprintf(stderr, "usage: mtd-erase2 [device]\n");
			return EINVAL;
		}
	}
	/* mtd-write2 [path] [device] */
	else if (!strcmp(base, "mtd-write2")) {
		if (argc >= 3)
			return mtd_write(argv[1], argv[2]);
		else {
			fprintf(stderr, "usage: mtd-write2 [path] [device]\n");
			return EINVAL;
		}
	}
#else
	/* mtd-erase2 [device] */
	else if (!strcmp(base, "mtd-erase2")) {
		if (argv[1] && ((!strcmp(argv[1], "boot")) ||
				(!strcmp(argv[1], "linux")) ||
				(!strcmp(argv[1], "linux2")) ||
				(!strcmp(argv[1], "rootfs")) ||
				(!strcmp(argv[1], "rootfs2")) ||
				(!strcmp(argv[1], "brcmnand")) ||
				(!strcmp(argv[1], "nvram")))) {
			return mtd_erase(argv[1]);
		} else {
			fprintf(stderr, "usage: mtd-erase2 [device]\n");
			return EINVAL;
		}
	}
	/* mtd-write2 [path] [device] */
	else if (!strcmp(base, "mtd-write2")) {
		if (argc >= 3)
			return mtd_write(argv[1], argv[2]);
		else {
			fprintf(stderr, "usage: mtd-write2 [path] [device]\n");
			return EINVAL;
		}
	}
#endif
#endif
#endif
	else if (!strcmp(base, "test_endian")) {
		int num = 0x04030201;
		char c = *(char *)(&num);

		if (c == 0x04 || c == 0x01) {
			if (c == 0x04)
				printf("Big.\n");
			else
				printf("Little.\n");
		}
		else
			printf("test error!\n");

		return 0;
	}
	else if (!strcmp(base, "free_caches")) {
		int c;
		unsigned int test_num;
		char *set_value = NULL;
		int clean_time = 1;
		int threshold = 0;

		if (argc) {
			while((c = getopt(argc, argv, "c:w:t:")) != -1) {
				switch(c) {
					case 'c': // set the clean-cache mode: 0~3.
						test_num = strtol(optarg, NULL, 10);
			if (test_num == LONG_MIN || test_num == LONG_MAX) {
				_dprintf("ERROR: unknown value %s...\n", optarg);
							return 0;
						}

						if (test_num < 0 || test_num > 3) {
							_dprintf("ERROR: the value %s was over the range...\n", optarg);
							return 0;
						}

						set_value = optarg;

						break;
					case 'w': // set the waited time for cleaning.
						test_num = strtol(optarg, NULL, 10);
			if (test_num < 0 || test_num == LONG_MIN || test_num == LONG_MAX) {
				_dprintf("ERROR: unknown value %s...\n", optarg);
							return 0;
						}

						clean_time = test_num;

						break;
					case 't': // set the waited time for cleaning.
						test_num = strtol(optarg, NULL, 10);
			if (test_num < 0 || test_num == LONG_MIN || test_num == LONG_MAX) {
				_dprintf("ERROR: unknown value %s...\n", optarg);
							return 0;
						}

						threshold = test_num;

						break;
					default:
						fprintf(stderr, "Usage: free_caches [ -c clean_mode ] [ -w clean_time ] [ -t threshold ]\n");
						break;
				}
			}
		}

		if (!set_value)
			set_value = FREE_MEM_PAGE;

		free_caches(set_value, clean_time, threshold);

		return 0;
	}
#ifdef RTCONFIG_USB_MODEM
	else if (!strcmp(base, "write_3g_ppp_conf")) {
		int modem_unit;

		if (argc != 1 && argc != 2) {
			printf("Usage: %s <modem_unit>\n", argv[0]);
			return 0;
		}

		if (argc == 2)
			modem_unit = atoi(argv[1]);
		else
			modem_unit = 0;

		return write_3g_ppp_conf(modem_unit);
	}
#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS)
	else if (!strcmp(base, "lplus")) {
		if (argc != 3) {
			printf("Usage: %s <integer1> <integer2>.\n", argv[0]);
			return 0;
		}

		unsigned long long int1 = strtoull(argv[1], NULL, 10);
		unsigned long long int2 = strtoull(argv[2], NULL, 10);
		unsigned long long total = int1+int2;

		printf("%llu", total);

		return 0;
	}
	else if (!strcmp(base, "lminus")) {
		if (argc != 3) {
			printf("Usage: %s <integer1> <integer2>.\n", argv[0]);
			return 0;
		}

		unsigned long long int1 = strtoull(argv[1], NULL, 10);
		unsigned long long int2 = strtoull(argv[2], NULL, 10);
		unsigned long long total = int1-int2;

		printf("%llu", total);

		return 0;
	}
#endif
#endif
#ifdef RTCONFIG_TOR
	else if (!strcmp(base, "start_tor")) {
		start_Tor_proxy();
		return 0;
	}
#endif
#ifdef RTCONFIG_NEW_USER_LOW_RSSI
	else if (!strcmp(base, "start_roamast")) {
		start_roamast();
		return 0;
	}
#endif
#if defined(RTCONFIG_KEY_GUARD)
	else if (!strcmp(base, "start_keyguard")) {
		start_keyguard();
		return 0;
	}
#endif
#if defined(RTCONFIG_BT_CONN)
	else if (!strcmp(base, "start_bluetooth_service")) {
		start_bluetooth_service();
		return 0;
	}
#if defined(RTCONFIG_BT_CONN_UART)
	else if (!strcmp(base, "execute_bt_bscp")) {
		execute_bt_bscp();
		return 0;
	}
#endif
#endif	/* RTCONFIG_BT_CONN */
#if defined(RTCONFIG_RALINK) || defined(RTCONFIG_QCA)
	else if (!strcmp(base, "dump_powertable")) {
		if (!IS_ATE_FACTORY_MODE())
			return 0;
		dump_powertable();
		return 0;
	}
#endif
#if defined(RTCONFIG_RALINK)
	else if (!strcmp(base, "dump_txbftable")) {
		if (!IS_ATE_FACTORY_MODE())
			return 0;
		dump_txbftable();
		return 0;
	}
#endif
#ifdef RTCONFIG_EXTPHY_BCM84880
	else if (!strcmp(base, "extphy_bit_op")) {
		unsigned int reg, val, start_bit, end_bit, wait_ms;
		int wr, ret;

		if(argc < 2 || (strcmp(argv[1], "rd") && strcmp(argv[1], "wr")))
			return 0;

		if(argc < 3)
			return 0;

		reg = strtol(argv[2], 0, 16);

		if(!strcmp(argv[1], "wr")){
			wr = 1;
			start_bit = wait_ms = 0;
			end_bit = 15;

			if(argc >= 4 && argc <= 7){
				if(argc >= 4)
					val = strtol(argv[3], 0, 16);
				if(argc >= 5)
					start_bit = strtol(argv[4], 0, 10);
				if(argc >= 6)
					end_bit = strtol(argv[5], 0, 10);
				if(argc == 7)
					wait_ms = strtol(argv[6], 0, 10);
			}
			else
				return 0;
		}
		else{ // !strcmp(argv[1], "rd")
			wr = 0;
			val = start_bit = wait_ms = 0;
			end_bit = 15;

			if(argc >= 3 && argc <= 5){
				if(argc >= 4)
					start_bit = strtol(argv[3], 0, 10);
				if(argc == 5)
					end_bit = strtol(argv[4], 0, 10);
			}
			else
				return 0;
		}

		ret = extphy_bit_op(reg, val, wr, start_bit, end_bit, wait_ms);

#if defined(RTAX86U)
		if(!strcmp(get_productid(), "RT-AX86S")) ;
		else if(nvram_get_int("ext_phy_model") == EXT_PHY_GPY211)
			_dprintf("addr=0x%02x, reg=0x%06x, val=0x%04x\n", EXTPHY_GPY_ADDR, reg, ret);
		else if(nvram_get_int("ext_phy_model") == EXT_PHY_RTL8226)
			_dprintf("addr=0x%02x, reg=0x%06x, val=0x%04x\n", EXTPHY_RTL_ADDR, reg, ret);
		else
#endif
#if defined(GTAX11000) || defined(ET12) || defined(XT12)
		if(nvram_get_int("ext_phy_model") == EXT_PHY_GPY211)
			_dprintf("addr=0x%02x, reg=0x%06x, val=0x%04x\n", EXTPHY_GPY_ADDR, reg, ret);
		else
#endif
			_dprintf("addr=0x%02x, reg=0x%06x, val=0x%04x\n", EXTPHY_ADDR, reg, ret);

		return 0;
	}
	else if (!strcmp(base, "get_ext_phy_id")) {
		get_ext_phy_id();
		return 0;
	}
#endif
#if RTCONFIG_SOFTWIRE46
	else if (!strcmp(base, "mapcalc")) {
		char peerbuf[INET6_ADDRSTRLEN];
		char addr6buf[INET6_ADDRSTRLEN];
		char addr4buf[INET_ADDRSTRLEN + sizeof("/32")];
		char rules[4096], *fmrs;
		int k, offset, psidlen, psid, start, end;
		int draft = argv[1] && strcmp(argv[1], "draft") == 0;
		int wan_proto = -1;

		if (!strcmp(argv[2], "map-e"))
			wan_proto = WAN_MAPE;
		else if (!strcmp(argv[2], "lw4o6"))
			wan_proto = WAN_LW4O6;
		else
			wan_proto = WAN_V6PLUS;

		while (fgets(rules, sizeof(rules), stdin) != NULL) {
			if (s46_mapcalc(wan_proto, rules, peerbuf, sizeof(peerbuf), addr6buf, sizeof(addr6buf),
					addr4buf, sizeof(addr4buf), &offset, &psidlen, &psid, &fmrs, draft) <= 0) {
				peerbuf[0] = addr6buf[0] = addr4buf[0] = '\0';
				offset = 0, psidlen = 0, psid = 0;
				fmrs = NULL;
			}
			printf("peer: %s\n", peerbuf);
			printf("addr6: %s\n", addr6buf);
			printf("addr4: %s\n", addr4buf);
			printf("offset: %d\n", offset);
			printf("psidlen: %d\n", psidlen);
			printf("psid: 0x%x\n", psid);
			printf("fmrs: %s\n", fmrs ? : "");
			free(fmrs);
			printf("ports:");
			start = end = 0;
			for (k = offset ? 1 : 0; k < (1 << offset); k++) {
				start = (k << (16 - offset)) | (psid << (16 - offset - psidlen));
				end = start + (1 << (16 - offset - psidlen)) - 1;
				if (start == 0)
					start = 1;
				if (start <= end)
					printf(" %d-%d", start, end);
			}
			printf("\n");
		}
		return 0;
	}
#endif
#if defined(RTCONFIG_HND_ROUTER_AX)
	else if (!strcmp(base, "set_cable_media")) {
		if(argc != 3)
			return 0;

		set_cable_media(argv[1], argv[2]);
		return 0;
	}
#endif
	else if (!strcmp(base, "collect_debuglog")) {
		if (argv[1]) {
			if (!strcmp(argv[1], "klog"))
				collect_debuglog(DEBUGLOG_KLOG);
			else if (!strcmp(argv[1], "coredump"))
				collect_debuglog(DEBUGLOG_CORE);
			else if (!strcmp(argv[1], "syslog"))
				collect_debuglog(DEBUGLOG_SLOG);
			else if (!strcmp(argv[1], "all"))
				collect_debuglog(DEBUGLOG_KLOG|DEBUGLOG_CORE|DEBUGLOG_SLOG);
			else {
				printf("Invalid argument\n");
			}
		}
		return 0;
	}
	printf("Unknown applet: %s\n", base);
	return 0;
}

#if defined(CONFIG_BCMWL5)
void exe_eu_wa_rr(void){
	char cmd_cc_tmp[64];
	char cmd_cc_ori[64];
	char cmd_up[64];
	char cmd2[64];
	char cmd3[64];
	char cmd_down[64];
	char ccode[64];

	char prefix[16];
	char ifname[16];
	char *next;
	char ifnames_tmp[128];
	int if_num=0;
	char tmp[32],prefix_country_cdoe[16];
	char tmp_country_cdoe[16];
	char nv_nband[64];
	int nband;

#ifdef NO_NBAND
	return ;
#endif

	//find dfs ifname
	strncpy(ifnames_tmp,nvram_safe_get("wl_ifnames"),sizeof(ifnames_tmp));

	foreach(ifname, ifnames_tmp, next){
		if_num++;
	}

	snprintf(nv_nband, sizeof(nv_nband), "wl%d_nband", if_num-1);
	nband = nvram_get_int(nv_nband);

	if( nband < 4 && nband > 0) { //1 is for 5G, 2 is for 2G, 4 is for 6G
		if(nvram_get_int("re_mode")) snprintf(prefix, sizeof(prefix), "wl%d.1_", if_num-1);
		else snprintf(prefix, sizeof(prefix), "wl%d_", if_num-1);

		snprintf(prefix_country_cdoe, sizeof(prefix_country_cdoe), "wl%d_", if_num-1);

	} else if(nband == 4) {//6g(nband = 4) do not have DFS
		if(nvram_get_int("re_mode")) snprintf(prefix, sizeof(prefix), "wl%d.1_", if_num-2);
		else snprintf(prefix, sizeof(prefix), "wl%d_", if_num-2);

		snprintf(prefix_country_cdoe, sizeof(prefix_country_cdoe), "wl%d_", if_num-2);
	} else {
		_dprintf("unknow nband number\n");
		return ;
	}

	strlcpy(ifname, nvram_safe_get(strcat_r(prefix, "ifname", tmp)), sizeof(ifname));

	snprintf(ccode,sizeof(ccode),"%scountry_code",prefix_country_cdoe);
	/* get tmp conutry code */
	if(get_wifi_country_code_tmp(ccode,tmp_country_cdoe,sizeof(tmp_country_cdoe)) !=0)
	{
		_dprintf("get country list failed\n");
		return;
	}


	snprintf(cmd_down,sizeof(cmd_up),"wl -i %s down",ifname);
	snprintf(cmd2,sizeof(cmd2),"wl -i %s spect 0",ifname);
	snprintf(cmd3,sizeof(cmd3),"wl -i %s radar 0",ifname);
	snprintf(cmd_up,sizeof(cmd_down),"wl -i %s up",ifname);
	snprintf(cmd_cc_tmp,sizeof(cmd_cc_tmp),"wl -i %s country %s",ifname,tmp_country_cdoe);
	snprintf(cmd_cc_ori,sizeof(cmd_cc_ori),"wl -i %s country %s",ifname,nvram_safe_get(ccode));

	system(cmd_down);
	//_dprintf("%s\n",cmd_down);
	sleep(1);
	system(cmd_cc_tmp);
	//_dprintf("%s\n",cmd_cc_tmp);
	sleep(1);
	system(cmd_up);
	//_dprintf("%s\n",cmd_up);
	sleep(1);

	//_dprintf("%s\n",cmd_down);
	system(cmd_down);
	sleep(1);
	//_dprintf("%s\n",cmd2);
	system(cmd2);
	sleep(1);
	//_dprintf("%s\n",cmd3);
	system(cmd3);
	sleep(1);
	//_dprintf("%s\n",cmd_cc_ori);
	system(cmd_cc_ori);
	sleep(1);	
	//_dprintf("%s\n",cmd_up);
	system(cmd_up);
	sleep(1);
	//_dprintf("%s\n",);
	notify_rc("restart_acsd");

}
#endif
