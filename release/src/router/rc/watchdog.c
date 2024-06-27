/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * Copyright 2004, ASUSTeK Inc.
 * All Rights Reserved.
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */
#include <rc.h>

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <shutils.h>
#include <stdarg.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <limits.h>
#ifdef RTCONFIG_RALINK
#include <ralink.h>
#endif
#ifdef RTCONFIG_QCA
#include <qca.h>
#if defined(RTCONFIG_WIFI_SON) && defined(RTCONFIG_AMAS)
#include <sys/mount.h>
#endif
#endif
#ifdef RTCONFIG_REALTEK
#include "realtek.h"
#endif
#include <shared.h>

#include <syslog.h>
#include <bcmnvram.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <math.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/reboot.h>
#include <sys/sysinfo.h>
#ifdef RTCONFIG_USER_LOW_RSSI
#if defined(RTCONFIG_RALINK)
#include <typedefs.h>
#else
#include <wlioctl.h>
#include <wlutils.h>
#endif
#endif
#ifdef RTAC88U
#include <rtk_switch.h>
#endif

#if defined(RTCONFIG_NOTIFICATION_CENTER)
#include <libnt.h>
#endif
#if defined(RTCONFIG_LP5523)
#include <lp5523led.h>
#endif

#ifdef RTCONFIG_BSC_SR
#include <bcmparams.h>
#include <wlioctl.h>
#endif

#ifdef RTCONFIG_CFGSYNC
#include <cfg_event.h>
#endif
#ifdef RTCONFIG_AMAS
#include <amas_ob.h>
#ifdef RTCONFIG_LIBASUSLOG
#include <libasuslog.h>
#endif
#endif

#if defined(RTCONFIG_RGBLED)
#include <aura_rgb.h>
#ifdef GTAC2900
#define AURA_LED_RST	"255,255,255,1,0,0"	// White
#define AURA_LED_WPS	"255,255,255,1,0,0"	// White
#define AURA_LED_BTN	"0,0,255,7,0,0"		// Blue Comet
#define AURA_LED_OFF	"0,0,0,1,0,0"		// Off
#endif
#endif

#ifdef RTCONFIG_WL_SCHED_V2
#include <sched_v2.h>
#endif
#include <json.h>

#if defined(RTCONFIG_QCA_PLC_UTILS) || defined(RTCONFIG_QCA_PLC2)
#include <plc_utils.h>
#endif
#ifdef RTCONFIG_BCMWL6
extern int restore_defaults_g;
#endif

#define BCM47XX_SOFTWARE_RESET	0x40		/* GPIO 6 */
#define RESET_WAIT		2		/* seconds */
#define RESET_WAIT_COUNT	RESET_WAIT * 10 /* 10 times a second */

#define TEST_PERIOD		100		/* second */
#define NORMAL_PERIOD		1		/* second */
#define URGENT_PERIOD		100 * 1000	/* microsecond */
#define RUSHURGENT_PERIOD	50 * 1000	/* microsecond */
#define DAY_PERIOD		2 * 60 * 24	/* 1 day (in 30 sec periods) */

#define WPS_TIMEOUT_COUNT	121 * 20
#ifdef RTCONFIG_WPS_LED
#define WPS_SUCCESS_COUNT	3
#endif
#define WPS_WAIT		1		/* seconds */
#define WPS_WAIT_COUNT		WPS_WAIT * 20	/* 20 times a second */
#ifdef BLUECAVE
static int bc_wps_led = 0;
#endif

#ifdef RTCONFIG_AMAS
#define AMESH_TIMEOUT_COUNT	30 * 20		/* 30 secnods */

struct time_mapping_s time_mapping;
#endif

#ifdef RTCONFIG_WPS_RST_BTN
#define WPS_RST_DO_WPS_COUNT		( 1*10)	/* 1 seconds */
#define WPS_RST_DO_RESTORE_COUNT	(10*10)	/* 10 seconds */
#undef RESET_WAIT_COUNT
#define RESET_WAIT_COUNT		WPS_RST_DO_RESTORE_COUNT
#endif	/* RTCONFIG_WPS_RST_BTN */

#ifdef RTCONFIG_WPS_ALLLED_BTN
#define WPS_LED_WAIT_COUNT		15 //use URGENT_PERIOD, press 1.5 ~ 2.5 secs to turn LED on/off
#endif

#if defined(RTCONFIG_EJUSB_BTN)
#define EJUSB_WAIT_COUNT	2		/* 2 seconds */
#endif

//#if defined(RTCONFIG_JFFS2LOG) && defined(RTCONFIG_JFFS2)
#if defined(RTCONFIG_JFFS2LOG) && (defined(RTCONFIG_JFFS2)||defined(RTCONFIG_BRCM_NAND_JFFS2))
#define LOG_COMMIT_PERIOD	2		/* 2 x 30 seconds */
static int log_commit_count = 0;
#endif
#if defined(RTCONFIG_USB_MODEM)
#define LOG_MODEM_PERIOD	20		/* 10 minutes */
static int log_modem_count = 0;
#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS)
#define MODEM_FLOW_PERIOD	1
static int modem_flow_count = 0;
static int modem_data_save = 0;
#endif
#endif
#if 0	//defined(RTCONFIG_TOR) && (defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2))
#define TOR_CHECK_PERIOD	10		/* 10 x 30 seconds */
unsigned int tor_check_count = 0;
#endif

#ifdef RTCONFIG_REALTEK
#define REINIT_WEB_FILE		("/tmp/reinit_web")
#define PARAM_TEMP_FILE		("/tmp/flash_param")
#define PARAM_TEMP_FILE2	("/tmp/flash_param2")
#endif

static sigset_t sigs_to_catch;

static struct itimerval itv;
/* to check watchdog alive */
#if ! (defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK))
static struct itimerval itv02;
#endif
static int watchdog_period = 0;
#ifdef WATCHDOG_PERIOD2
static int watchdog_period2 = 0;
#endif
#ifdef RTAX55
static int drop_caches_period = 0;
static int drop_caches = 0;
#endif
static int top_period = 0;
static int top = 0;
#ifdef RTCONFIG_BCMARM
static int chkusb3_period = 0;
static int u3_chk_life = 6;
#endif
#if !defined(RTCONFIG_BCM_MFG) && (defined(RTAX82U) || defined(GSAX3000) || defined(GSAX5400) || defined(TUFAX5400) || defined(GTAX11000_PRO) || defined(GTAXE16000) || defined(GTBE98) || defined(GTBE98_PRO) || defined(GTAX6000) || defined(GT10) || defined(RTAX82U_V2) || defined(TUFAX5400_V2) || defined(DSL_AX82U))
static int ledg_count = 0;
#endif
static int wanduck_count = 0;
static int btn_pressed = 0;
static int btn_count = 0;
#ifdef BTN_SETUP
static int btn_pressed_setup = 0;
static int btn_count_setup = 0;
static int wsc_timeout = 0;
static int btn_count_setup_second = 0;
static int btn_pressed_toggle_radio = 0;
#endif
static long ddns_update_timer = 0;

#if defined(RTCONFIG_WIRELESS_SWITCH) && defined(RTCONFIG_DSL)
// for WLAN sw init, only for slide switch
static int wlan_sw_init = 0;
#elif defined(RTCONFIG_WIRELESS_SWITCH) && defined(RTCONFIG_QCA)
static int wifi_sw_old = -1;
#endif
#if defined(RTCONFIG_TURBO_BTN)
static int g_boost_status[BOOST_MODE_MAX] = { 0 };
#endif
#if (defined(RTCONFIG_LED_BTN) || (!defined(RTCONFIG_WIFI_TOG_BTN) && !defined(RTCONFIG_QCA) && !defined(RTCONFIG_WPS_ALLLED_BTN))) && !defined(RTAX82U) && !defined(DSL_AX82U) && !defined(GSAX3000) && !defined(GSAX5400) && !defined(TUFAX5400) && !defined(GTAX6000) && !defined(GT10) && !defined(RTAX82U_V2) && !defined(TUFAX5400_V2) && !defined(RTAX88U_PRO)
#if defined(RTCONFIG_QCA)
static int LED_status_old = 0;
static int LED_status = 0;
#else
static int LED_status_old = -1;
static int LED_status = -1;
#endif
static int LED_status_changed = 0;
static int LED_status_first = 1;
static int LED_status_on = -1;
#ifdef RTAC87U
static int LED_switch_count = 0;
static int BTN_pressed_count = 0;
#endif
#endif

#if defined(RTCONFIG_WPS_ALLLED_BTN)
static int LED_status_old = -1;
static int LED_status = -1;
static int LED_status_changed = 0;
static int LED_status_on = -1;
static int BTN_pressed_count = 0;
#endif

#ifdef RTCONFIG_BCMWL6
static int wlonunit = -1;
#endif

extern int g_wsc_configured;

static unsigned int sigbones = 0;

#ifdef RTCONFIG_ISP_CUSTOMIZE_TOOL
static int ICT_LED_status = -1;
#endif

#if defined(RTAX58U_V2) || defined(BR63)
static int check_x_Setting = 0;
#endif

void watch_sig(int signo) {
	sigbones |= 1<<signo;
}

static void *fn_acts[_NSIG];

#define REGULAR_DDNS_CHECK	10 //10x30 sec
static int ddns_check_count = 0;
static int freeze_duck_count = 0;
static int ddns_recover_min = 0;

#ifndef RTCONFIG_AVOID_TZ_ENV
static char time_zone_t[32]={0};
#endif

static const struct mfg_btn_s {
	enum btn_id id;
	char *name;
	char *nv;
	int (*pre_req)(void);	/* Model-speceific prerequisite */
} mfg_btn_table[] = {
#ifndef RTCONFIG_N56U_SR2
	{ BTN_RESET,	"RESET", 	"btn_rst", NULL },
#endif
	{ BTN_WPS,	"WPS",		"btn_ez", NULL },
#if defined(RTCONFIG_WIFI_TOG_BTN)
	{ BTN_WIFI_TOG,	"WIFI_TOG",	"btn_wifi_toggle", NULL },
#endif
#ifdef RTCONFIG_WIRELESS_SWITCH
	{ BTN_WIFI_SW,	"WIFI_SW",	"btn_wifi_sw", NULL },
#endif
#if defined(RTCONFIG_EJUSB_BTN)
	{ BTN_EJUSB1,	"EJECT USB1",	"btn_ejusb_btn1", NULL },
	{ BTN_EJUSB2,	"EJECT USB2",	"btn_ejusb_btn2", NULL },
#endif
#ifndef RTAC68U
#if defined(RTCONFIG_LED_BTN)
	{ BTN_LED,	"LED ON/OFF", 	"btn_led", NULL },
#endif
#endif
#if defined(RTCONFIG_TURBO_BTN)
	{ BTN_TURBO,	"TURBO", 	"btn_turbo", NULL },
#endif

	{ BTN_ID_MAX,	NULL,		NULL, NULL },
};

#if defined(RTCONFIG_QCA)
static time_t g_t1;
#endif

/* ErP Test */
#ifdef RTCONFIG_ERP_TEST
#define MODE_NORMAL 0
#define MODE_PWRSAVE 1
#define NO_ASSOC_CHECK 6 // 6x30 sec
static int pwrsave_status = MODE_NORMAL;
static int no_assoc_check = 0;
#endif

void led_table_ctrl(int on_off);

#if defined(RTAC1200G) || defined(RTAC1200GP)
#define WDG_MONITOR_PERIOD 60 /* second */
static int wdg_timer_alive = 1;
#endif

#ifdef HND_ROUTER
const char *dw_aggled[]={"dw", "0x800c00b8", NULL};
const char *sw_aggled_alloff[]={"sw", "0x800c00b8", "0x40000", NULL};
const char *sw_aggled_allon[]={"sw", "0x800c00b8", "0x4008f", NULL};
const char *sw_aggled_extoff[]={"sw", "0x800c00b8", "0x4000f", NULL};
#ifdef RTCONFIG_BONDING
static int bs=-1, bs_pre=-1;
extern char *bs_desc[];
#endif
#endif

/* DEBUG DEFINE */
#define SCHED_DEBUG	"/tmp/SCHED_DEBUG"
#define WL_SCHED_DBG(fmt,args...) \
	if(nvram_get_int("sched_dbg") || f_exists(SCHED_DEBUG) > 0) { \
	printf("[SCHED][%s:(%d)]"fmt, __FUNCTION__, __LINE__, ##args); \
	}

void watchdog(int sig);
#if defined(RTCONFIG_RGBLED) && defined(GTAC2900)
void aura_led_control(char *rgb);
#endif

#if defined(RTCONFIG_NOTIFICATION_CENTER)
void RC_SEND_NT_EVENT(int NT_EVENT_FLAG, char *sub_event);
#endif

int
elm_of_strr(const char *strr[])
{
	int num = 0;
	while (strr[num++]);
	return num-1;
}

void
sys_exit()
{
	printf("[watchdog] sys_exit");
	set_action(ACT_REBOOT);
	kill(1, SIGTERM);
}

static void
alarmtimer(unsigned long sec, unsigned long usec)
{
	itv.it_value.tv_sec = sec;
	itv.it_value.tv_usec = usec;
	itv.it_interval = itv.it_value;
	setitimer(ITIMER_REAL, &itv, NULL);
}

/* to check watchdog alive */
#if ! (defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK))
static void
alarmtimer02(unsigned long sec, unsigned long usec)
{
	itv02.it_value.tv_sec = sec;
	itv02.it_value.tv_usec = usec;
	itv02.it_interval = itv02.it_value;
	setitimer(ITIMER_REAL, &itv02, NULL);
}
#endif

extern int no_need_to_start_wps();
extern void uptime_wait(int);

#ifdef RTCONFIG_ISP_CUSTOMIZE_TOOL
void ict_led_control()
{
	if (nvram_get("ict_status")) {
		if (nvram_match("x_Setting", "1")) {
			nvram_unset("ict_status");
			nvram_commit();
			setAllLedOn();
			return;
		}

		if (nvram_get_int("ict_status"))
			alarmtimer(0, URGENT_PERIOD);
		else
			alarmtimer(NORMAL_PERIOD, 0);

		if (ICT_LED_status != 1) {
#if defined(RTCONFIG_BCM_CLED) && defined(RTCONFIG_SINGLE_LED)
			bcm_cled_ctrl(BCM_CLED_BLUE, BCM_CLED_STEADY_NOBLINK);
#else
			setAllLedOn();
#endif
			ICT_LED_status = 1;
		}
		else {
#if defined(RTCONFIG_BCM_CLED) && defined(RTCONFIG_SINGLE_LED)
			bcm_cled_ctrl(BCM_CLED_OFF, BCM_CLED_STEADY_NOBLINK);
#else
			setAllLedOff();
#endif
			ICT_LED_status = 0;
		}
	}
}
#endif

void led_control_normal(void)
{
#ifdef BLUECAVE
	if(nvram_match("bc_ledbh", "wps_done"))
		kill_pidfile_s("/var/run/sw_devled.pid", SIGUSR1);
	return;
#endif

#if defined(RTCONFIG_ALPINE) || defined(RTCONFIG_LANTIQ) || defined(RPAC92)
	return;
#endif
#ifdef RTAC87U
	LED_switch_count = nvram_get_int("LED_switch_count");
#endif

#if defined(RTCONFIG_LED_BTN) || defined(RTCONFIG_WPS_ALLLED_BTN) || defined(RTCONFIG_TURBO_BTN) || (!defined(RTCONFIG_WIFI_TOG_BTN) && !defined(RTCONFIG_QCA))
	if (inhibit_led_on()) return;
#endif

#ifdef RTCONFIG_WPS_LED
	int v = LED_OFF;
	// the behavior in normal when wps led != power led
	// wps led = off, power led = on

	if (nvram_match("wps_success", "1"))
		v = LED_ON;
	__wps_led_control(v);
#endif

#if !defined(RTCONFIG_CONCURRENTREPEATER)
	led_control(LED_POWER, LED_ON);
#endif
#if defined(RTCONFIG_LOGO_LED) && !defined(GTAX11000) && !defined(GTAXE11000) && !defined(GTAX11000_PRO) && !defined(GTAXE16000)
	led_control(LED_LOGO, LED_ON);
#endif
#if defined(RTN11P) || defined(RTN300) || defined(RTN11P_B1)
	led_control(LED_WPS, LED_ON);	//wps led is also 2g led. and NO power led.
#else
	if (nvram_get_int("led_pwr_gpio") != nvram_get_int("led_wps_gpio"))
		led_control(LED_WPS, LED_OFF);
#endif

#if defined(RTCONFIG_RGBLED) && defined(GTAC2900)
	if(nvram_match("dfs_aura_nt_ctrl", "1")){
		nvram_set("pause_aura_rgb_nt", "0");
		nvram_unset("dfs_aura_nt_ctrl");
		return;
	}
	start_aurargb();
#endif
}

#ifndef HND_ROUTER
void erase_nvram(void)
{
	switch (get_model()) {
		case MODEL_BLUECAVE:
		case MODEL_GTAC9600:
			eval("cp", "-f", "/sbin/nvram.txt", "/tmp/nvram.txt");
			eval("cp", "-f", "/sbin/nvram.txt", "/jffs/nvram.txt");
		default:
#ifdef RTCONFIG_BCMARM
			eval("mtd-erase2", "nvram");
#else
			eval("mtd-erase","-d","nvram");
#endif
	}
}
#endif

int init_toggle(void)
{
	switch (get_model()) {
#ifdef RTCONFIG_WIFI_TOG_BTN
		case MODEL_RTAC56S:
		case MODEL_RTAC56U:
		case MODEL_RTAC3200:
		case MODEL_RTAC68U:
		case MODEL_DSLAC68U:
		case MODEL_RTAC87U:
		case MODEL_RTAC5300:
		case MODEL_GTAC5300:
		case MODEL_RTAC88U:
		case MODEL_RTAC86U:
		case MODEL_RTAC3100:
		case MODEL_RTAX88U:
		case MODEL_GTAX11000:
		case MODEL_RTAX92U:
		case MODEL_RTAX95Q:
		case MODEL_XT8PRO:
		case MODEL_BM68:
		case MODEL_XT8_V2:
		case MODEL_RTAXE95Q:
		case MODEL_ET8PRO:
		case MODEL_ET8_V2:
		case MODEL_RTAX56_XD4:
		case MODEL_XD4PRO:
		case MODEL_CTAX56_XD4:
		case MODEL_RTAX58U:
		case MODEL_RTAX82U_V2:
		case MODEL_TUFAX5400_V2:
		case MODEL_RTAX5400:
		case MODEL_RTAX82_XD6S:
		case MODEL_XD6_V2:
		case MODEL_RTAX58U_V2:
		case MODEL_GT10:
		case MODEL_RTAX9000:
		case MODEL_RTAXE7800:
		case MODEL_TUFAX3000_V2:
		case MODEL_RTAX56U:
		case MODEL_RTAX86U:
		case MODEL_GTAXE11000:
			nvram_set("btn_ez_radiotoggle", "1");
			return BTN_WIFI_TOG;
#endif
		default:
			return BTN_WPS;
	}
}

#ifdef RTCONFIG_REALTEK
/* [MUST]: Need to re-coding this part */
typedef struct rtk_wps_result{
	int band;
	char ssid[256];
	char wsc_ssid[256];
	int auth_type;
	int encrypt;
	int wsc_auth;
	int wpa_auth;
	char wpa_psk[256];
	int psk_format;
	char wsc_psk[256];
	int wpa_suite;
	int wpa2_suite;
	int wep;
	int wsc_enc;
}rtk_wps_result;
//#define SET_WPS_RESULT_TO_ROOT
#ifdef RTCONFIG_CONCURRENTREPEATER
#define RTK_SCAN_LIST_2G "/tmp/rtk_ss_list_2g"
#define RTK_SCAN_LIST_5G "/tmp/rtk_ss_list_5g"
#define MAX_WAIT_COUNT ((15*1000*1000)/(RUSHURGENT_PERIOD))

static int wait_5g_time = 0;
static int wait_2g_time = 0;
static int wps_2g_done = 0;
static int wps_5g_done = 0;
static rtk_wps_result wps_res_2g,wps_res_5g;

struct save_fuple {
	int length;
	char *cmppart;
	char *setpart1;
	char *setpart2;
};
static int rtk_comparetmp( char *arraylist[], int sizelist, char ssidptr1[], char *ssidptr2) {
	int sizetmp = 0;
	char ssidcat[128];

	strcpy ( ssidcat, ssidptr1 );
	strcat ( ssidcat, ssidptr2 );
	while( sizetmp < sizelist) {
		if( !strcmp( arraylist[sizetmp], ssidcat ) ) {
			strcat ( ssidptr1, ssidptr2 );
			return 1;
		}
		sizetmp ++;
	}
	return 0;
}
static char *rtk_readfile(char *fname,int *fsize)
{
 FILE *fp;
 unsigned long size,lsize;
 char *pt;
 int len;
 char buf[100];

 size=0;
 pt=NULL;
 fp=fopen(fname,"r");
 if (!fp) return NULL;
 while (1)
  {
   len=fread(buf,1,100,fp);
   if (len==-1)
    goto sysfail;
   lsize=size;
   size+=len;
   pt=(char *)realloc(pt,size+1);
   if (len==0)
    {
     pt[size]='\0';
     break;
    }
   if (!pt)
    goto sysfail;
   memcpy(pt+lsize,buf,len);
  }
 fclose(fp);
 pt[size]='\0';
 *fsize=size;
 return pt;

sysfail:
 fclose(fp);
 if (pt)
  free(pt);
 return NULL;
}

static int rtk_auto_detect_ssid(int band_chk,char* ssid_buf, char* result)
{
	char file_name[128], substrl[128], strNULL[]="";
	char *getptr1, *getptr2, *substrr, *gettmp[128];
	int fsize, idlength=0, cmpresult=0;
	struct stat status;
	struct save_fuple *bandlist;
	struct save_fuple getSsidRule0[] = {
		{ 5	, "-2.4G"  , "-5G"	, ""  },
		{ 5	, "_2.4G"  , "_5G"	, ""  },
		{ 5	, ".2.4G"  , ".5G"	, ""  },
		{ 5	, " 2.4G"  , " 5G"	, ""  },
		{ 5	, "-2.4g"  , "-5g"	, ""  },
		{ 5	, "_2.4g"  , "_5g"	, ""  },
		{ 5	, ".2.4g"  , ".5g"      , ""  },
		{ 5	, " 2.4g"  , " 5g"      , ""  },
		{ 4     , "2.4G"   , "5G"       , ""  },
		{ 4	, "2.4g"   , "5g"  	, ""  },
		{ 4	, "-2.4"   , "-5"       , ""  },
		{ 4	, "_2.4"   , "_5"       , ""  },
		{ 4	, ".2.4"   , ".5"       , ""  },
		{ 4	, " 2.4"   , " 5"       , ""  },
		{ 3	, "2.4"    , "5"	, ""  },
		{ 3	, "-2G"    , "-5G"      , ""  },
		{ 3	, "_2G"    , "_5G"      , ""  },
		{ 3	, ".2G"    , ".5G"      , ""  },
		{ 3	, " 2G"    , " 5G"      , ""  },
		{ 3     , "-2g"    , "-5g"      , ""  },
		{ 0     , "_2g"    , "_5g"      , ""  },
		{ 3     , ".2g"    , ".5g"      , ""  },
		{ 3     , " 2g"    , " 5g"      , ""  },
		{ 2	, "2G"     , "5G"       , ""  },
		{ 2     , "2g"     , "5g"       , ""  },
		{ 2	, "-2"     , "-5"       , ""  },
		{ 2	, "_2"     , "_5"       , ""  },
		{ 2	, ".2"     , ".5"       , ""  },
		{ 2	, " 2"     , " 5"       , ""  },
		{ 1	, "2"      , "5"	, ""  },
		{ 0	, ""	   , "-5G"      , ""  },
		{ 0	, ""	   , "_5G"      , ""  },
		{ 0	, ""	   , ".5G"      , ""  },
		{ 0	, ""	   , " 5G"      , ""  },
		{ 0	, ""	   , "-5g"      , ""  },
		{ 0	, ""	   , "_5g"      , ""  },
		{ 0	, ""	   , ".5g"      , ""  },
		{ 0	, ""	   , " 5g"      , ""  },
		{ 0	, ""	   , "5G"       , ""  },
		{ 0	, ""	   , "5g"       , ""  },
		{ 0	, ""	   , "-5"       , ""  },
		{ 0	, ""	   , "_5"       , ""  },
		{ 0	, ""	   , ".5"       , ""  },
		{ 0	, ""	   , " 5"       , ""  },
		{ 0	, ""	   , "5"	, ""  },
		{ 0	, ""	   , ""		, ""  },
		{ 99	, ""	   , ""		, ""  }
	};
	struct save_fuple getSsidRule1[] = {
		{ 3	, "-5G"	, "-2G"	, "-2.4G"  },
		{ 3	, "_5G"	, "_2G"	, "_2.4G"  },
		{ 3	, ".5G"	, ".2G"	, ".2.4G"  },
		{ 3	, " 5G"	, " 2G"	, " 2.4G"  },
		{ 3	, "-5g"	, "-2g"	, "-2.4g"  },
		{ 3	, "_5g"	, "_2g"	, "_2.4g"  },
		{ 3	, ".5g"	, ".2g"	, ".2.4g"  },
		{ 3	, " 5g"	, " 2g"	, " 2.4g"  },
		{ 2	, "5G"	, "2G"	, "2.4G"   },
		{ 2	, "5g"	, "2g" 	, "2.4g"   },
		{ 2	, "-5"	, "-2"	, "-2.4"   },
		{ 2	, "_5"	, "_2"	, "_2.4"   },
		{ 2	, ".5"	, ".2"	, ".2.4"   },
		{ 2	, " 5"	, " 2"	, " 2.4"   },
		{ 1	, "5"	, "2" 	, "2.4"    },
		{ 0	, ""	, "-2G"	, "-2.4G"  },
		{ 0     , ""    , "_2G" , "_2.4G"  },
		{ 0     , ""    , ".2G" , ".2.4G"  },
		{ 0     , ""    , " 2G" , " 2.4G"  },
		{ 0     , ""    , "-2g" , "-2.4g"  },
		{ 0     , ""    , "_2g" , "_2.4g"  },
		{ 0     , ""    , ".2g" , ".2.4g"  },
		{ 0     , ""    , " 2g" , " 2.4g"  },
		{ 0     , ""    , "2G" 	, "2.4G"   },
		{ 0     , ""    , "2g"  , "2.4g"   },
		{ 0     , ""    , "-2"  , "-2.4"   },
		{ 0     , ""    , "_2"  , "_2.4"   },
		{ 0     , ""    , ".2"  , ".2.4"   },
		{ 0     , ""    , " 2"  , " 2.4"   },
		{ 0     , ""    , "2"  	, "2.4"    },
		{ 0     , ""    , ""  	, ""       },
		{ 99    , ""    , ""    , ""       }
	};

	//Get another band's SSID list
	if(band_chk == 0)
	{
		sprintf(file_name,RTK_SCAN_LIST_2G);
		while(!stat("/var/pbc_state1_cancel", &status) != 0); // file exists, wps not stopped
	}
	else if(band_chk == 1)
	{
		sprintf(file_name,RTK_SCAN_LIST_5G);
		while(!stat("/var/pbc_state2_cancel", &status) != 0); // file exists, wps not stopped
	}
	if( access( file_name, F_OK ) != -1 ) {
		getptr1 = rtk_readfile(file_name, &fsize);
		getptr2 = strstr(getptr1, "\n");
		while( getptr2 != NULL ) {
			*getptr2 = '\0';
			gettmp[idlength] = getptr1;
			getptr1 = getptr2 + 1;
			getptr2 = strstr(getptr1, "\n");
			idlength ++;
		}
	}

	if ( band_chk )
		bandlist = getSsidRule0;
	else
		bandlist = getSsidRule1;

	//compare the SSID with SCAN LIST
	while( bandlist->length != 99 ) {
		if ( strlen(ssid_buf) > bandlist->length )
			substrr = ssid_buf + strlen(ssid_buf) - bandlist->length;
		else {
			bandlist++;
			continue;
		}

		if( bandlist->length == 0 ) {
			strcpy( substrl, ssid_buf );
			if ( rtk_comparetmp( gettmp, idlength, substrl, bandlist->setpart1 ) ) {
				cmpresult=1;
				break ;
			}
			if ( strcmp(bandlist->setpart2, "")) {
				if ( rtk_comparetmp( gettmp, idlength, substrl, bandlist->setpart2 ) ) {
					cmpresult=1;
					break;
				}
			}
		}
		else if( !strcmp(substrr, bandlist->cmppart) ){
			strncpy( substrl, ssid_buf, strlen(ssid_buf)-bandlist->length );
			if( rtk_comparetmp( gettmp, idlength, substrl, bandlist->setpart1 ) ) {
				cmpresult=1;
				break;
			}
			if ( strcmp(bandlist->setpart2, "")) {
				if ( rtk_comparetmp( gettmp, idlength, substrl, bandlist->setpart2 ) ) {
					cmpresult=1;
					break;
				}
			}
			if ( rtk_comparetmp( gettmp, idlength, substrl, strNULL ) ) {
				cmpresult = 1;
				break;
			}
		}
		bandlist++;
	}

	if( cmpresult == 1 ) {
		TRACE_PT("=== Find the SSID : [ %s ]\n", substrl);
		strcpy(result,substrl);
	}
	else
		TRACE_PT("=== Can't find the SSID : [  ]\n");

	return cmpresult;
}
#endif

static char *rtk_get_token(char *data, char *token)
{
	char *ptr=data;
	int len=0, idx=0;

	while (*ptr && *ptr != '\n' ) {
		if (*ptr == '=') {
			if (len <= 1)
				return NULL;
			memcpy(token, data, len);

			/* delete ending space */
			for (idx=len-1; idx>=0; idx--) {
				if (token[idx] != ' ')
					break;
			}
			token[idx+1] = '\0';

			return ptr+1;
		}
		len++;
		ptr++;
	}
	return NULL;
}
static int rtk_get_value(char *data, char *value)
{
	char *ptr=data;
	int len=0, idx, i;

	while (*ptr && *ptr != '\n' && *ptr != '\r') {
		len++;
		ptr++;
	}

	/* delete leading space */
	idx = 0;
	while (len-idx > 0) {
		if (data[idx] != ' ')
			break;
		idx++;
	}
	len -= idx;

	/* delete bracing '"' */
	if (data[idx] == '"') {
		for (i=idx+len-1; i>idx; i--) {
			if (data[i] == '"') {
				idx++;
				len = i - idx;
			}
			break;
		}
	}

	if (len > 0) {
		memcpy(value, &data[idx], len);
		value[len] = '\0';
	}
	return len;
}

int rtk_get_wps_result(char* result_file,rtk_wps_result* res)
{
	FILE* fp;
	char buffer[256] = {'\0'};
	char value[100],token[40];
	char* ptr;
	if(result_file == NULL || res == NULL)
	{
		return -1;
	}
	fp = fopen(result_file,"r");
	if(fp != NULL)
	{
		while(fgets(buffer,sizeof(buffer),fp))
		{
			ptr = rtk_get_token(buffer, token);
			if (ptr == NULL)
				continue;
			if (rtk_get_value(ptr, value)==0)
				continue;
			if(strncmp(token,"INTERFACE",strlen("INTERFACE")) == 0)
			{
#ifdef RPAC92
				if (!strcmp(value, "wlan2") || !strcmp(value, "wlan2-vxd"))
#else
				if (!strcmp(value, "wl0") || !strcmp(value, "wl0-vxd"))
#endif
					res->band = 0;
				else
					res->band = 1;

				TRACE_PT("band=%d\n",res->band);
				continue;
			}
			if(strncmp(token,"SSID",strlen("SSID")) == 0)
			{
				strcpy(res->ssid,value);
				TRACE_PT("SSID=%s\n",res->ssid);
				continue;
			}
			if(strncmp(token,"WSC_SSID",strlen("WSC_SSID")) == 0)
			{
				strcpy(res->wsc_ssid,value);
				TRACE_PT("WSC_SSID=%s\n",res->wsc_ssid);
				continue;
			}
			if(strncmp(token,"AUTH_TYPE",strlen("AUTH_TYPE")) == 0)
			{
				res->auth_type = atoi(value);
				TRACE_PT("auth_type:%d\n",res->auth_type);
				continue;
			}
			if(strncmp(token,"ENCRYPT",strlen("ENCRYPT")) == 0)
			{
				res->encrypt = atoi(value);
				TRACE_PT("ENCRYPT:%d\n",res->encrypt);
				continue;
			}
			if(strncmp(token,"WSC_AUTH",strlen("WSC_AUTH")) == 0)
			{
				res->wsc_auth = atoi(value);
				TRACE_PT("WSC_AUTH:%d\n",res->wsc_auth);
				continue;
			}
			if(strncmp(token,"WPA_AUTH",strlen("WPA_AUTH")) == 0)
			{
				res->wpa_auth = atoi(value);
				TRACE_PT("WPA_AUTH:%d\n",res->wpa_auth);
				continue;
			}
			if(strncmp(token,"WPA_PSK",strlen("WPA_PSK")) == 0)
			{
				strcpy(res->wpa_psk,value);
				TRACE_PT("WPA_PSK:%s\n",res->wpa_psk);
				continue;
			}
			if(strncmp(token,"PSK_FORMAT",strlen("PSK_FORMAT")) == 0)
			{
				res->psk_format = atoi(value);
				TRACE_PT("PSK_FORMAT:%d\n",res->psk_format);
				continue;
			}
			if(strncmp(token,"WSC_PSK",strlen("WSC_PSK")) == 0)
			{
				strcpy(res->wsc_psk,value);
				TRACE_PT("WSC_PSK:%s\n",res->wsc_psk);
				continue;
			}
			if(strncmp(token,"WPA_CIPHER_SUITE",strlen("WPA_CIPHER_SUITE")) == 0)
			{
				res->wpa_suite = atoi(value);
				TRACE_PT("WPA_CIPHER_SUITE:%d\n",res->wpa_suite);
				continue;
			}
			if(strncmp(token,"WPA2_CIPHER_SUITE",strlen("WPA2_CIPHER_SUITE")) == 0)
			{
				res->wpa2_suite = atoi(value);
				TRACE_PT("WPA2_CIPHER_SUITE:%d\n",res->wpa2_suite);
				continue;
			}
			if(strncmp(token,"WEP",strlen("WEP")) == 0)
			{
				res->wep = atoi(value);
				TRACE_PT("wep:%d\n",res->wep);
				continue;
			}
			if(strncmp(token,"WSC_ENC",strlen("WSC_ENC")) == 0)
			{
				res->wsc_enc = atoi(value);
				TRACE_PT("WSC_ENC:%d\n",res->wsc_enc);
				continue;
			}
		}
		fclose(fp);
	}
	return 0;
}

/* mode=0: Both get WiFi profile. mode=1: Only one band get WiFi profile. */
int set_wps_result_to_ap_nvram(int mode, int src_unit, int dst_unit)
{
	char prefix[]="wlXXXXXX_";
	char prefix_1[]="wlXXXXX.1";
	char wlc_prefix[]="wlcXXXXX_";
	char tmp[32] = {0}, buf[128] = {0};
	int ncount;
#ifdef RTCONFIG_CONCURRENTREPEATER
	snprintf(prefix, sizeof(prefix), "wl%d_", dst_unit);
	snprintf(prefix_1, sizeof(prefix_1), "wl%d.1_", dst_unit);
	snprintf(wlc_prefix, sizeof(wlc_prefix), "wlc%d_", src_unit);
#else
	strcpy(prefix, "wl_");
	strcpy(wlc_prefix, "wlc_");
#endif

	/* Set SSID */
	strcpy(buf, nvram_safe_get(strcat_r(wlc_prefix, "ssid", tmp)));
	ncount = sizeof(buf) - strlen(buf) - 1;
	if (src_unit == 0 && dst_unit == 0) // 2.4G to 2.4G
		strncat(buf, "_RPT", ncount);
	else if (src_unit == 0 && dst_unit == 1) // 2.4G to 5G
		strncat(buf, "_RPT5G", ncount);
	else if (src_unit == 1 && dst_unit == 0) // 5G to 2.4G
		strncat(buf, "_RPT", ncount);
	else if (src_unit == 1 && dst_unit == 1 && mode == 0) // 5G to 5G
		strncat(buf, "_RPT", ncount);
	else if (src_unit == 1 && dst_unit == 1 && mode != 0) // 5G to 5G
		strncat(buf, "_RPT5G", ncount);

	nvram_set(strcat_r(prefix, "ssid", tmp), buf);
#ifdef RTCONFIG_CONCURRENTREPEATER
	nvram_set(strcat_r(prefix_1, "ssid", tmp), buf);
#endif

	/* Set WPS PSK */
	strcpy(buf, nvram_safe_get(strcat_r(wlc_prefix, "wpa_psk", tmp)));
	nvram_set(strcat_r(prefix, "wpa_psk", tmp), buf);
#ifdef RTCONFIG_CONCURRENTREPEATER
	nvram_set(strcat_r(prefix_1, "wpa_psk", tmp), buf);
#endif

	/* Set auth node*/
	strcpy(buf, nvram_safe_get(strcat_r(wlc_prefix, "auth_mode", tmp)));
	nvram_set(strcat_r(prefix, "auth_mode_x", tmp), buf);
#ifdef RTCONFIG_CONCURRENTREPEATER
	nvram_set(strcat_r(prefix_1, "auth_mode_x", tmp), buf);
#endif

	/* Set crypto */
	strcpy(buf, nvram_safe_get(strcat_r(wlc_prefix, "crypto", tmp)));
	nvram_set(strcat_r(prefix, "crypto", tmp), buf);
#ifdef RTCONFIG_CONCURRENTREPEATER
	nvram_set(strcat_r(prefix_1, "crypto", tmp), buf);
#endif

	return 0;
}

int rtk_set_wps_result_to_nvram(rtk_wps_result res)
{
	int sw_mode = -1;
	int wps_band = -1;
	char temp_str[64] = {'\0'};
	char auth_mode_str[32] = {'\0'};
	char crypto_str[32] = {'\0'};

	char wlc_auth_mode[64];
	char wlc_crypto[64];
	sw_mode = sw_mode();
	if(sw_mode == SW_MODE_REPEATER || (sw_mode == SW_MODE_AP && nvram_get_int("wlc_psta") == 1)) /* repeater mode & media bridge */
	{
		if(nvram_match("x_Setting","0")
#ifdef RTCONFIG_AMAS
			&& !nvram_get_int("wps_amas_enrollee")
#endif
		)
		{
			nvram_set("x_Setting","1");
		}
#ifdef RTCONFIG_CONCURRENTREPEATER
		wps_band = res.band;

		sprintf(wlc_auth_mode,"wlc%d_auth_mode",wps_band);
		sprintf(wlc_crypto,"wlc%d_crypto",wps_band);

		sprintf(temp_str,"wlc%d_ssid",wps_band);
		nvram_set(temp_str,res.wsc_ssid);

		sprintf(temp_str,"wlc%d_wpa_psk",wps_band);
		nvram_set(temp_str,res.wpa_psk);
#else
		strcpy(wlc_auth_mode,"wlc_auth_mode");
		strcpy(wlc_crypto,"wlc_crypto");

		nvram_set("wlc_ssid",res.wsc_ssid);
		nvram_set("wlc_wpa_psk",res.wpa_psk);
#endif
#ifdef SET_WPS_RESULT_TO_ROOT
		sprintf(temp_str,"wl%d_wpa_psk",wps_band);
		nvram_set(temp_str,res.wsc_psk);
		sprintf(temp_str,"wl%d.1_wpa_psk",wps_band);
		nvram_set(temp_str,res.wsc_psk);
		sprintf(temp_str,"%d",res.wep);
		nvram_set("wlc_wep",temp_str);
#endif
#ifdef SET_WPS_RESULT_TO_ROOT
		sprintf(auth_mode_str,"wl%d_auth_mode_x",wps_band);
		sprintf(crypto_str,"wl%d_crypto",wps_band);
#endif
		switch(res.wsc_auth)/*set authentication nvram*/
		{
			case 1:/*open*/
			{
//				nvram_set("wsc_auth","open");
//				nvram_set("wsc_crypto","none");
				nvram_set(wlc_auth_mode,"open");
				nvram_set(wlc_crypto,"none");
#ifdef SET_WPS_RESULT_TO_ROOT
				nvram_set(auth_mode_str,"open");
				nvram_set(crypto_str,"none");
#endif
				break;
			}
			case 2:
			{
				if(res.wsc_enc == 8)/*wpa:aes*/
				{
					nvram_set(wlc_auth_mode,"pskpsk2");
					nvram_set(wlc_crypto,"aes");
#ifdef SET_WPS_RESULT_TO_ROOT
					nvram_set(auth_mode_str,"pskpsk2");
					nvram_set(crypto_str,"aes");
#endif
				}
				break;
			}
			case 32:
			{
				if(res.wsc_enc == 8)/*wpa2:aes*/
				{
					nvram_set(wlc_auth_mode,"psk2");
					nvram_set(wlc_crypto,"aes");
#ifdef SET_WPS_RESULT_TO_ROOT
					nvram_set(auth_mode_str,"psk2");
					nvram_set(crypto_str,"aes");
#endif
				}
				break;
			}
			case 34:
			{
				if(res.wsc_enc == 12)/*wpa:tkip+aes*/
				{
					nvram_set(wlc_auth_mode,"pskpsk2");
					nvram_set(wlc_crypto,"tkip+aes");
#ifdef SET_WPS_RESULT_TO_ROOT
					nvram_set(auth_mode_str,"pskpsk2");
					nvram_set(crypto_str,"tkip+aes");
#endif
				}
				break;
			}
			default:
				break;
		}

	}
	else/*not repeater mode*/
	{
		wps_band = res.band;
		sprintf(auth_mode_str,"wl%d_auth_mode_x",wps_band);
		sprintf(crypto_str,"wl%d_crypto",wps_band);

		sprintf(temp_str,"wl%d_ssid",wps_band);
		nvram_set(temp_str,res.ssid);
		sprintf(temp_str,"wl%d_wpa_psk",wps_band);
		nvram_set(temp_str,res.wpa_psk);
		switch(res.wsc_auth)/*set authentication nvram*/
		{
			case 1:/*open*/
			{
				nvram_set(auth_mode_str,"open");
				nvram_set(crypto_str,"none");
				break;
			}
			case 2:
			{
				if(res.wsc_enc == 8)/*wpa:aes*/
				{
					nvram_set(auth_mode_str,"pskpsk2");
					nvram_set(crypto_str,"aes");
				}
				break;
			}
			case 32:
			{
				if(res.wsc_enc == 8)/*wpa2:aes*/
				{
					nvram_set(auth_mode_str,"psk2");
					nvram_set(crypto_str,"aes");
				}
				break;
			}
			case 34:
			{
				if(res.wsc_enc == 12)/*wpa:tkip+aes*/
				{
					nvram_set(auth_mode_str,"pskpsk2");
					nvram_set(crypto_str,"tkip+aes");
				}
				break;
			}
			default:
				break;
		}
	}
	nvram_commit();
	return 0;
}

int rtk_amas_wps2nvram(rtk_wps_result res)
{
	char prefix[]="wlxxxxxx";
	char pfcred[] = "wlc_";
	char pfcred0[] = "wlc0_";
	char pfcred1[] = "wlc1_";
	char tmp[128];
	char *auth_mode, *crypto;

	snprintf(prefix, sizeof(prefix), "wl%d_", res.band);
	/* SSID */
	nvram_set(strcat_r(prefix, "ssid", tmp), res.wsc_ssid);
	nvram_set(strcat_r(pfcred, "ssid", tmp), res.wsc_ssid);
	nvram_set(strcat_r(pfcred0, "ssid", tmp), res.wsc_ssid);
	nvram_set(strcat_r(pfcred1, "ssid", tmp), res.wsc_ssid);
	/* pwd */
	nvram_set(strcat_r(prefix, "wpa_psk", tmp), res.wpa_psk);
	nvram_set(strcat_r(pfcred, "wpa_psk", tmp), res.wpa_psk);
	nvram_set(strcat_r(pfcred0, "wpa_psk", tmp), res.wpa_psk);
	nvram_set(strcat_r(pfcred1, "wpa_psk", tmp), res.wpa_psk);

	switch(res.wsc_auth)/*set authentication nvram*/
	{
		case 1:/*open*/
			auth_mode = "open";
			crypto = "none";
			break;
		case 2:
			if(res.wsc_enc == 8) {
				auth_mode = "pskpsk2";
				crypto = "aes";
			}
			break;
		case 32:
			if(res.wsc_enc == 8) {
				auth_mode = "psk2";
				crypto ="aes";
			}
			break;
		case 34:
			if(res.wsc_enc == 12) {
				auth_mode = "pskpsk2";
				crypto = "tkip+aes";
			}
			break;
		default:
			break;
	}
	nvram_set(strcat_r(prefix, "auth_mode_x", tmp), auth_mode);
	nvram_set(strcat_r(pfcred, "auth_mode", tmp), auth_mode);
	nvram_set(strcat_r(pfcred0, "auth_mode", tmp), auth_mode);
	nvram_set(strcat_r(pfcred1, "auth_mode", tmp), auth_mode);

	nvram_set(strcat_r(prefix, "crypto", tmp), crypto);
	nvram_set(strcat_r(pfcred, "crypto", tmp), crypto);
	nvram_set(strcat_r(pfcred0, "crypto", tmp), crypto);
	nvram_set(strcat_r(pfcred1, "crypto", tmp), crypto);

	nvram_commit();
	return 0;
}

#define WSCD_CONFIG_STATUS "/tmp/wscd_status"

enum {
	NOT_USED = -1,
	PROTOCOL_START = 0,
	PROTOCOL_SUCCESS = 3
};

static void wps_processing_check(void)
{
	FILE *fp = NULL;
	int status = -1;
	static int skip_flag = 0;

	fp = fopen(WSCD_CONFIG_STATUS, "r");
	if (fp != NULL) {

		fscanf(fp, "%d", &status);
		if (status != NOT_USED || status != PROTOCOL_START || status != PROTOCOL_SUCCESS) {

			if (!skip_flag) {
#ifdef RPAC68U
				set_led(LED_BLINK_QUICK, LED_BLINK_QUICK);
#else
				nvram_set_int("led_status", LED_WPS_PROCESSING);
#endif
				skip_flag = 1;
			}
		}
		else {
			skip_flag = 0;
		}
		fclose(fp);
	}
}



static int wps_process_finish()
{
	FILE *fp = NULL;
	int status = -1;

	fp = fopen(WSCD_CONFIG_STATUS, "r");
	if (fp != NULL) {

		fscanf(fp, "%d", &status);
		fclose(fp);
		if (status == PROTOCOL_SUCCESS)
			return 1;
	}
	return 0;
}

void rtk_wl_led(void)
{
	int sw_mode = sw_mode();
	if (sw_mode == SW_MODE_AP && nvram_get_int("wlc_psta") != 1) // AP mode
		return;
	if (!is_wps_stopped())
		return;
	if (btn_pressed == 2) // Ready process button event.
		return;
	if (nvram_get_int("restore_defaults") == 1)
		return;
	if (nvram_invmatch("Ate_power_on_off_enable", "0"))	/* avoid run in test to let all led off */
		return;

	static int p_wlc0_state = -99;
	static int p_wlc1_state = -99;

	int wlc0_led = 0, wlc1_led = 0;
	int wlc0_state = 0, wlc1_state = 0;
	int wlc_express = nvram_get_int("wlc_express");

	wlc0_state = nvram_get_int("wlc0_state");
	if (wlc0_state == WLC_STATE_CONNECTED) {
		if (sw_mode == SW_MODE_REPEATER && wlc_express == 0)
			wlc0_led = LED_SHOW_SIG_STR;
		else
			wlc0_led = LED_SHOW_SIG_STR2;
	}
	else
		wlc0_led = LED_OFF_ALL;

	wlc1_state = nvram_get_int("wlc1_state");
	if (wlc1_state == WLC_STATE_CONNECTED) {
		if (sw_mode == SW_MODE_REPEATER && wlc_express == 0)
			wlc1_led = LED_SHOW_SIG_STR;
		else
			wlc1_led = LED_SHOW_SIG_STR2;
	}
	else
		wlc1_led = LED_OFF_ALL;

	if (p_wlc0_state != wlc0_state || p_wlc1_state != wlc1_state) {
		set_led(wlc0_led, wlc1_led);
		p_wlc0_state = wlc0_state;
		p_wlc1_state = wlc1_state;
	}
	else if (wlc0_led == LED_SHOW_SIG_STR || wlc1_led == LED_SHOW_SIG_STR ||
		 wlc0_led == LED_SHOW_SIG_STR2 || wlc1_led == LED_SHOW_SIG_STR2)
		set_led(wlc0_led, wlc1_led);
}

void rtk_wps_state_check(void)
{
	struct stat status;
	int sw_mode = 0;
	int need_restart = 0;
	rtk_wps_result wps_result;
	int detect_result = 0;
#ifndef RTCONFIG_CONCURRENTREPEATER
	int wlc_band = -1;
	char res_file[64] = {0};
#endif
	sw_mode = sw_mode();
#ifdef RTCONFIG_CONCURRENTREPEATER
	short int wlc_express = nvram_get_int("wlc_express");
#endif
	if(stat(REINIT_WEB_FILE, &status) != 0) // file does not exist
	{
        if(access_point_mode()
#ifdef RTCONFIG_AMAS
            || (sw_mode == SW_MODE_AP && nvram_match("re_mode", "1"))
#endif
        ) { // AP mode.
			if (nvram_get_int("w_Setting") == 1) { // CONFIGURED
				if (wps_process_finish()) { // Special case!! Only for wscd process.
#ifdef RTCONFIG_CFGSYNC
					send_event_to_cfgmnt(EID_RC_WPS_STOP);
#endif
					stop_wps_method();
					return;
				}
			}
		}
		return;
	}
#ifdef RTCONFIG_AMAS
	if(nvram_get_int("wps_amas_enrollee") == 1) {
		if(stat(PARAM_TEMP_FILE,&status) == 0)/*get wl0-vxd result*/
		{
			if(rtk_get_wps_result(PARAM_TEMP_FILE,&wps_res_2g) == 0)
			{
				rtk_amas_wps2nvram(wps_res_2g);
				nvram_set_int("obd_Setting", 1);
				nvram_set_int("wps_e_success", 1);
				unlink(PARAM_TEMP_FILE);
				stop_wps_method();
			}
		}
	} else
#endif
	if(sw_mode == SW_MODE_REPEATER || (sw_mode == SW_MODE_AP && nvram_get_int("wlc_psta") == 1))/*repeater mode*/ /* media bridge */
	{
		wps_processing_check();
#ifdef RTCONFIG_CONCURRENTREPEATER
		if(stat(PARAM_TEMP_FILE,&status) == 0)/*get wl0-vxd result*/
		{
			if(rtk_get_wps_result(PARAM_TEMP_FILE,&wps_res_2g) == 0)
			{
				rtk_set_wps_result_to_nvram(wps_res_2g);
				unlink(PARAM_TEMP_FILE);
				wps_2g_done = 1;
			}
		}
		if(stat(PARAM_TEMP_FILE2,&status) == 0)/*get wl1-vxd result*/
		{
			if(rtk_get_wps_result(PARAM_TEMP_FILE2,&wps_res_5g) == 0)
			{
				rtk_set_wps_result_to_nvram(wps_res_5g);
				unlink(PARAM_TEMP_FILE2);
				wps_5g_done = 1;
			}
		}
		if(wps_2g_done)
		{
			if (wlc_express == 0 || (sw_mode == SW_MODE_AP && nvram_get_int("wlc_psta") == 1)) {
				if(wps_5g_done || wait_5g_time >= MAX_WAIT_COUNT)
				{
					if(!wps_5g_done)
					{
						TRACE_PT("5g wps not done, guess the ssid...\n");
						memcpy(&wps_res_5g,&wps_res_2g,sizeof(rtk_wps_result));
						wps_res_5g.band = 1;
						stop_wps_method();
						if(rtk_auto_detect_ssid(1,wps_res_2g.wsc_ssid,wps_res_5g.wsc_ssid))
						{
							rtk_set_wps_result_to_nvram(wps_res_5g);
							detect_result = 1;
						}
					}

					if (wps_5g_done) { // Both
						set_wps_result_to_ap_nvram(0, 1, 1);
						set_wps_result_to_ap_nvram(0, 0, 0);
					}
					else if (!wps_5g_done && detect_result) { // Both
						set_wps_result_to_ap_nvram(0, 1, 1);
						set_wps_result_to_ap_nvram(0, 0, 0);
					}
					else { // Only 2.4G
						set_wps_result_to_ap_nvram(1, 0, 1);
						set_wps_result_to_ap_nvram(1, 0, 0);
					}

					unlink(REINIT_WEB_FILE);
					TRACE_PT("2.4g done. WPS need to reinit wireless.\n");
					notify_rc("restart_wireless");
#ifdef RPAC68U
					set_led(LED_BLINK_NORNAL, LED_BLINK_NORNAL);
#else
					nvram_set_int("led_status", LED_WPS_RESTART_WL);
#endif
				}
				else
				{
					wait_5g_time ++;
				}
			}
			else {
				set_wps_result_to_ap_nvram(1, 0, 1);
				unlink(REINIT_WEB_FILE);
				TRACE_PT("2.4g done. WPS need to reinit wireless.\n");
				notify_rc("restart_wireless");
#ifdef RPAC68U
				set_led(LED_BLINK_NORNAL, LED_BLINK_NORNAL);
#else
				nvram_set_int("led_status", LED_WPS_RESTART_WL);
#endif
			}
		}
		if(wps_5g_done)
		{
			if (wlc_express == 0 || (sw_mode == SW_MODE_AP && nvram_get_int("wlc_psta") == 1)) {
				if(wps_2g_done || wait_2g_time >= MAX_WAIT_COUNT)
				{
					if(!wps_2g_done)
					{
						TRACE_PT("2.4g wps not done, guess the ssid...\n");
						memcpy(&wps_res_2g,&wps_res_5g,sizeof(rtk_wps_result));
						wps_res_2g.band = 0;
						stop_wps_method();
						if(rtk_auto_detect_ssid(0,wps_res_5g.wsc_ssid,wps_res_2g.wsc_ssid))
						{
							rtk_set_wps_result_to_nvram(wps_res_2g);
							detect_result = 1;
						}
					}

					if (wps_2g_done) { // Both
						set_wps_result_to_ap_nvram(0, 1, 1);
						set_wps_result_to_ap_nvram(0, 0, 0);
					}
					else if (!wps_2g_done && detect_result) { // Both
						set_wps_result_to_ap_nvram(0, 1, 1);
						set_wps_result_to_ap_nvram(0, 0, 0);
					}
					else { // Only 5G
						set_wps_result_to_ap_nvram(1, 1, 1);
						set_wps_result_to_ap_nvram(1, 1, 0);
					}

					unlink(REINIT_WEB_FILE);
					TRACE_PT("5g done. WPS need to reinit wireless.\n");
					notify_rc("restart_wireless");
#ifdef RPAC68U
					set_led(LED_BLINK_NORNAL, LED_BLINK_NORNAL);
#else
					nvram_set_int("led_status", LED_WPS_RESTART_WL);
#endif
				}
				else
				{
					wait_2g_time ++;
				}
			}
			else {
				set_wps_result_to_ap_nvram(1, 1, 0);
				unlink(REINIT_WEB_FILE);
				TRACE_PT("5g done. WPS need to reinit wireless.\n");
				notify_rc("restart_wireless");
#ifdef RPAC68U
				set_led(LED_BLINK_NORNAL, LED_BLINK_NORNAL);
#else
				nvram_set_int("led_status", LED_WPS_RESTART_WL);
#endif
			}
		}
#else
		wlc_band = nvram_get_int("wlc_band");
		if(wlc_band == 0)
		{
			strcpy(res_file,PARAM_TEMP_FILE);
		}
		else if(wlc_band == 1)
		{
			strcpy(res_file,PARAM_TEMP_FILE2);
		}
		if(rtk_get_wps_result(res_file,&wps_result) == 0)
		{
			rtk_set_wps_result_to_nvram(wps_result);
			unlink(res_file);
			unlink(REINIT_WEB_FILE);
			TRACE_PT("Done. WPS need to reinit wireless.\n");
			notify_rc("restart_wireless");
#ifdef RPAC68U
			set_led(LED_BLINK_NORNAL, LED_BLINK_NORNAL);
#else
			nvram_set_int("led_status", LED_WPS_RESTART_WL);
#endif
		}
#endif
	}
	else /*AP mode*/
	{
		if(stat(PARAM_TEMP_FILE,&status) == 0)/*get wps result*/
		{
			if(rtk_get_wps_result(PARAM_TEMP_FILE,&wps_result) == 0)
			{
				rtk_set_wps_result_to_nvram(wps_result);
				if(nvram_get_int("w_Setting") == 0)
				{
					need_restart = 1;
					nvram_set("w_Setting","1");
					nvram_commit();
					TRACE_PT("WPS need to reinit wireless.\n");
					stop_wps_method();
					notify_rc("restart_wireless");
				}
			}
			unlink(REINIT_WEB_FILE);
			unlink(PARAM_TEMP_FILE);
		}
	}
}
#else /* #endif RTCONFIG_REALTEK */

#ifdef RTCONFIG_CONCURRENTREPEATER

#define MAX_WAIT_COUNT ((15*1000*1000)/(RUSHURGENT_PERIOD))

static int wait_5g_time = 0;
static int wait_2g_time = 0;
static int wps_5g_done = 0;
static int wps_2g_done = 0;

static int qca_comparetmp( char *arraylist[], int sizelist, char ssidptr1[], char *ssidptr2) {
	int sizetmp = 0;
	char ssidcat[128];

	strcpy ( ssidcat, ssidptr1 );
	strcat ( ssidcat, ssidptr2 );
	while( sizetmp < sizelist) {
		if( !strcmp( arraylist[sizetmp], ssidcat ) ) {
			strcat ( ssidptr1, ssidptr2 );
			return 1;
		}
		sizetmp ++;
	}
	return 0;
}

/* mode=0: Both get WiFi profile. mode=1: Only one band get WiFi profile. */
int set_wps_result_to_ap_nvram(int mode, int src_unit, int dst_unit)
{
	char prefix[]="wlXXXXXX_";
	char prefix_1[]="wlXXXXX.1";
	char wlc_prefix[]="wlcXXXXX_";
	char tmp[32] = {0}, buf[128] = {0};
	int ncount;
#ifdef RTCONFIG_CONCURRENTREPEATER
	snprintf(prefix, sizeof(prefix), "wl%d_", dst_unit);
	snprintf(prefix_1, sizeof(prefix_1), "wl%d.1_", dst_unit);
	snprintf(wlc_prefix, sizeof(wlc_prefix), "wlc%d_", src_unit);
#else
	strcpy(prefix, "wl_");
	strcpy(wlc_prefix, "wlc_");
#endif

	/* Set SSID */
	strcpy(buf, nvram_safe_get(strcat_r(wlc_prefix, "ssid", tmp)));
	ncount = sizeof(buf) - strlen(buf) - 1;
	if (src_unit == 0 && dst_unit == 0) // 2.4G to 2.4G
		strncat(buf, "_RPT", ncount);
	else if (src_unit == 0 && dst_unit == 1) // 2.4G to 5G
		strncat(buf, "_RPT5G", ncount);
	else if (src_unit == 1 && dst_unit == 0) // 5G to 2.4G
		strncat(buf, "_RPT", ncount);
	else if (src_unit == 1 && dst_unit == 1 && mode == 0) // 5G to 5G
		strncat(buf, "_RPT", ncount);
	else if (src_unit == 1 && dst_unit == 1 && mode != 0) // 5G to 5G
		strncat(buf, "_RPT5G", ncount);

	nvram_set(strcat_r(prefix, "ssid", tmp), buf);
#ifdef RTCONFIG_CONCURRENTREPEATER
	nvram_set(strcat_r(prefix_1, "ssid", tmp), buf);
#endif

	/* Set WPS PSK */
	strcpy(buf, nvram_safe_get(strcat_r(wlc_prefix, "wpa_psk", tmp)));
	nvram_set(strcat_r(prefix, "wpa_psk", tmp), buf);
#ifdef RTCONFIG_CONCURRENTREPEATER
	nvram_set(strcat_r(prefix_1, "wpa_psk", tmp), buf);
#endif

	/* Set auth node*/
	strcpy(buf, nvram_safe_get(strcat_r(wlc_prefix, "auth_mode", tmp)));
	nvram_set(strcat_r(prefix, "auth_mode_x", tmp), buf);
#ifdef RTCONFIG_CONCURRENTREPEATER
	nvram_set(strcat_r(prefix_1, "auth_mode_x", tmp), buf);
#endif

	/* Set crypto */
	strcpy(buf, nvram_safe_get(strcat_r(wlc_prefix, "crypto", tmp)));
	nvram_set(strcat_r(prefix, "crypto", tmp), buf);
#ifdef RTCONFIG_CONCURRENTREPEATER
	nvram_set(strcat_r(prefix_1, "crypto", tmp), buf);
#endif

	return 0;
}

int qca_set_wps_result(int config_index, int band)
{
	FILE *fp = NULL;
	FILE *pfp = NULL;
	char cmd[64] = {0}, tmp_str[64] = {0}, tmp[32] = {0}, result_file[64] = {0};
	char *buf = NULL;
	char *sta = get_staifname(config_index);
	int id = 0;

	struct wlc_setting {

		char ssid[32];
		char key_mgmt[16];
		char auth_alg[16];
		char proto[16];
		char pairwise[16];
		char group[16];
		char psk[32];
		char wep_tx_keyidx[4];
		char wep_key[32];

	} *wlc_s;

	wlc_s = malloc(sizeof(struct wlc_setting));
	memset(wlc_s, 0, sizeof(wlc_s));

	buf = malloc(sizeof(char) * 128);
	memset(buf, 0, 128);

	/* Get current station ID */
	sprintf(cmd, "wpa_cli -p /var/run/wpa_supplicant-%s -i %s list_networks", sta, sta);
	pfp = popen(cmd, "r");

	if (pfp != NULL) {
		while(fgets(buf, 128, pfp) != NULL) {
			if (strstr(buf, "CURRENT")) {
				sscanf(buf, "%d", &id);
			}
		}
	}
	pclose(pfp);

	sleep(3); // Wait update config file.
	sprintf(result_file, "/etc/Wireless/conf/wpa_supplicant-%s.conf", sta);
	fp = fopen(result_file, "r");

	if (fp != NULL) {
		int network_index = 0;
		int found = 0;
		while (fgets(buf, 128, fp) != NULL) {

			if (strstr(buf, "network={")) {
				if (network_index == id) {
					found = 1;
				}
				else {
					network_index++;
					continue;
				}
			}

			if (found && !strstr(buf, "}")) {

				if (strstr(buf, "ssid")) {
					sscanf(buf, "%*[^\"]\"%[^\"]", tmp);
					strcpy(wlc_s->ssid, tmp);
				}
				else if (strstr(buf, "key_mgmt")) {
					sscanf(buf, "%*[^=]=%s", tmp);
					strcpy(wlc_s->key_mgmt, tmp);
				}
				else if (strstr(buf, "auth_alg")) {
					sscanf(buf, "%*[^=]=%s", tmp);
					strcpy(wlc_s->auth_alg, tmp);
				}
				else if (strstr(buf, "proto")) {
					sscanf(buf, "%*[^=]=%s", tmp);
					strcpy(wlc_s->proto, tmp);
				}
				else if (strstr(buf, "pairwise")) {
					sscanf(buf, "%*[^=]=%s", tmp);
					strcpy(wlc_s->pairwise, tmp);
				}
				else if (strstr(buf, "group")) {
					sscanf(buf, "%*[^=]=%s", tmp);
					strcpy(wlc_s->group, tmp);
				}
				else if (strstr(buf, "psk")) {
					sscanf(buf, "%*[^\"]\"%[^\"]", tmp);
					strcpy(wlc_s->psk, tmp);
				}
				else if (strstr(buf, "wep_tx_keyidx")) {
					sscanf(buf, "%*[^=]=%s", tmp);
					strcpy(wlc_s->wep_tx_keyidx, tmp);
				}
				else if (strstr(buf, "wep_key")) {
					sscanf(buf, "%*[^\"]\"%[^\"]", tmp);
					strcpy(wlc_s->wep_key, tmp);
				}
			}
			else if (found && strstr(buf, "}"))
				break;
		}
	}

	fclose(fp);
	free(buf);

	/* Set to nvram */
	/* ssid */
	if (strlen(wlc_s->ssid)) {
		if (config_index == band) {
			sprintf(tmp_str, "wlc%d_ssid", band);
			nvram_set(tmp_str, wlc_s->ssid);
		}
		else // Copy mode. So skip process SSID.
			;
	}
	else {
		free(wlc_s);
		return -1;
	}

	/* auth_mode */
	/* open, shared, psk, psk2, */
	int wep_flag = 0;
	if (strlen(wlc_s->key_mgmt)) {
		if (!strcmp(wlc_s->key_mgmt, "NONE")) {

			if (!strcmp(wlc_s->auth_alg, "OPEN")) {
				wep_flag = 1;
				sprintf(tmp_str, "wlc%d_auth_mode", band);
				nvram_set(tmp_str, "open");
				sprintf(tmp_str, "wlc%d_wep", band);
				nvram_set(tmp_str, "1");
			}
			else if (!strcmp(wlc_s->auth_alg, "SHARED")) {
				wep_flag = 1;
				sprintf(tmp_str, "wlc%d_auth_mode", band);
				nvram_set(tmp_str, "shared");
			}
			else {
				sprintf(tmp_str, "wlc%d_auth_mode", band);
				nvram_set(tmp_str, "open");
				sprintf(tmp_str, "wlc%d_wep", band);
				nvram_set(tmp_str, "0");
			}
		}
		else if (!strcmp(wlc_s->key_mgmt, "WPA-PSK")) {
			if (!strcmp(wlc_s->proto, "WPA")) {
				sprintf(tmp_str, "wlc%d_auth_mode", band);
				nvram_set(tmp_str, "psk");
			}
			else if (!strcmp(wlc_s->proto, "RSN")) {
				sprintf(tmp_str, "wlc%d_auth_mode", band);
				nvram_set(tmp_str, "psk2");
			}

			if (!strcmp(wlc_s->pairwise, "TKIP")) {
				sprintf(tmp_str, "wlc%d_crypto", band);
				nvram_set(tmp_str, "tkip");
			}
			else if (!strcmp(wlc_s->pairwise, "CCMP TKIP") || !strcmp(wlc_s->pairwise, "CCMP")) {
				sprintf(tmp_str, "wlc%d_crypto", band);
				nvram_set(tmp_str, "aes");
			}

			sprintf(tmp_str, "wlc%d_wpa_psk", band);
			nvram_set(tmp_str, wlc_s->psk);
		}
		else
		{
			sprintf(tmp_str, "wlc%d_auth_mode", band);
			nvram_set(tmp_str, "open");
		}
	}
	else {
		sprintf(tmp_str, "wlc%d_auth_mode", band);
		nvram_set(tmp_str, "open");
	}

	if (wep_flag) {

		sprintf(tmp_str, "wlc%d_wep_key", band);
		nvram_set(tmp_str, wlc_s->wep_key);

		sprintf(tmp_str, "wlc_key");
		sprintf(tmp, "%d", atoi(wlc_s->wep_tx_keyidx) + 1);
		nvram_set(tmp_str, tmp);
	}

	/* Finish WPS while dut is default, change the x_Setting as 1 */
	if(nvram_match("x_Setting", "0")) {
		nvram_set("x_Setting", "1");
		nvram_commit();
	}

	free(wlc_s);
	return 0;
}

static char *qca_readfile(FILE *fp, int *fsize)
{
	int size = 0;
	char *pt = NULL;

	if (fp == NULL)
		return NULL;

	char buf[128] = {0}, str[128] = {0};

	while (fgets(buf, sizeof(buf), fp) != NULL) {
		if (strstr(buf, "Failed"))
			return NULL;

		memset(str, 0, sizeof(str));
		sscanf(buf, "%*s%*s%*s%*s%s", str);

		str[strlen(str)] = '\n';

		size += strlen(str);

		pt=(char *)realloc(pt,size + 1);

		memcpy(pt+(size - strlen(str)), str, strlen(str));
	}

	if (pt != NULL)
		pt[size] = '\0';

	*fsize = size;

	return pt;
}

struct save_fuple {
	int length;
	char *cmppart;
	char *setpart1;
	char *setpart2;
};

static int qca_auto_detect_ssid(int band_chk,char* ssid_buf, char* result)
{
	char substrl[128], strNULL[]="";
	char *getptr1, *getptr2, *substrr, *gettmp[128];
	int fsize = 0, idlength=0, cmpresult=0;

	struct save_fuple *bandlist;
	struct save_fuple getSsidRule0[] = {
		{ 5	, "-2.4G"  , "-5G"	, ""  },
		{ 5	, "_2.4G"  , "_5G"	, ""  },
		{ 5	, ".2.4G"  , ".5G"	, ""  },
		{ 5	, " 2.4G"  , " 5G"	, ""  },
		{ 5	, "-2.4g"  , "-5g"	, ""  },
		{ 5	, "_2.4g"  , "_5g"	, ""  },
		{ 5	, ".2.4g"  , ".5g"      , ""  },
		{ 5	, " 2.4g"  , " 5g"      , ""  },
		{ 4     , "2.4G"   , "5G"       , ""  },
		{ 4	, "2.4g"   , "5g"  	, ""  },
		{ 4	, "-2.4"   , "-5"       , ""  },
		{ 4	, "_2.4"   , "_5"       , ""  },
		{ 4	, ".2.4"   , ".5"       , ""  },
		{ 4	, " 2.4"   , " 5"       , ""  },
		{ 3	, "2.4"    , "5"	, ""  },
		{ 3	, "-2G"    , "-5G"      , ""  },
		{ 3	, "_2G"    , "_5G"      , ""  },
		{ 3	, ".2G"    , ".5G"      , ""  },
		{ 3	, " 2G"    , " 5G"      , ""  },
		{ 3     , "-2g"    , "-5g"      , ""  },
		{ 0     , "_2g"    , "_5g"      , ""  },
		{ 3     , ".2g"    , ".5g"      , ""  },
		{ 3     , " 2g"    , " 5g"      , ""  },
		{ 2	, "2G"     , "5G"       , ""  },
		{ 2     , "2g"     , "5g"       , ""  },
		{ 2	, "-2"     , "-5"       , ""  },
		{ 2	, "_2"     , "_5"       , ""  },
		{ 2	, ".2"     , ".5"       , ""  },
		{ 2	, " 2"     , " 5"       , ""  },
		{ 1	, "2"      , "5"	, ""  },
		{ 0	, ""	   , "-5G"      , ""  },
		{ 0	, ""	   , "_5G"      , ""  },
		{ 0	, ""	   , ".5G"      , ""  },
		{ 0	, ""	   , " 5G"      , ""  },
		{ 0	, ""	   , "-5g"      , ""  },
		{ 0	, ""	   , "_5g"      , ""  },
		{ 0	, ""	   , ".5g"      , ""  },
		{ 0	, ""	   , " 5g"      , ""  },
		{ 0	, ""	   , "5G"       , ""  },
		{ 0	, ""	   , "5g"       , ""  },
		{ 0	, ""	   , "-5"       , ""  },
		{ 0	, ""	   , "_5"       , ""  },
		{ 0	, ""	   , ".5"       , ""  },
		{ 0	, ""	   , " 5"       , ""  },
		{ 0	, ""	   , "5"	, ""  },
		{ 0	, ""	   , ""		, ""  },
		{ 99	, ""	   , ""		, ""  }
	};
	struct save_fuple getSsidRule1[] = {
		{ 3	, "-5G"	, "-2G"	, "-2.4G"  },
		{ 3	, "_5G"	, "_2G"	, "_2.4G"  },
		{ 3	, ".5G"	, ".2G"	, ".2.4G"  },
		{ 3	, " 5G"	, " 2G"	, " 2.4G"  },
		{ 3	, "-5g"	, "-2g"	, "-2.4g"  },
		{ 3	, "_5g"	, "_2g"	, "_2.4g"  },
		{ 3	, ".5g"	, ".2g"	, ".2.4g"  },
		{ 3	, " 5g"	, " 2g"	, " 2.4g"  },
		{ 2	, "5G"	, "2G"	, "2.4G"   },
		{ 2	, "5g"	, "2g" 	, "2.4g"   },
		{ 2	, "-5"	, "-2"	, "-2.4"   },
		{ 2	, "_5"	, "_2"	, "_2.4"   },
		{ 2	, ".5"	, ".2"	, ".2.4"   },
		{ 2	, " 5"	, " 2"	, " 2.4"   },
		{ 1	, "5"	, "2" 	, "2.4"    },
		{ 0	, ""	, "-2G"	, "-2.4G"  },
		{ 0     , ""    , "_2G" , "_2.4G"  },
		{ 0     , ""    , ".2G" , ".2.4G"  },
		{ 0     , ""    , " 2G" , " 2.4G"  },
		{ 0     , ""    , "-2g" , "-2.4g"  },
		{ 0     , ""    , "_2g" , "_2.4g"  },
		{ 0     , ""    , ".2g" , ".2.4g"  },
		{ 0     , ""    , " 2g" , " 2.4g"  },
		{ 0     , ""    , "2G" 	, "2.4G"   },
		{ 0     , ""    , "2g"  , "2.4g"   },
		{ 0     , ""    , "-2"  , "-2.4"   },
		{ 0     , ""    , "_2"  , "_2.4"   },
		{ 0     , ""    , ".2"  , ".2.4"   },
		{ 0     , ""    , " 2"  , " 2.4"   },
		{ 0     , ""    , "2"  	, "2.4"    },
		{ 0     , ""    , ""  	, ""       },
		{ 99    , ""    , ""    , ""       }
	};

	//Get another band's SSID list
	FILE *pfp = NULL;
	if(band_chk == 0)
	{
		eval("wpa_cli", "-p", "/var/run/wpa_supplicant-sta0", "scan");
		sleep(3);
		pfp = popen("wpa_cli -p /var/run/wpa_supplicant-sta0 -i sta0 scan_results", "r");
	}
	else if(band_chk == 1)
	{
		eval("wpa_cli", "-p", "/var/run/wpa_supplicant-sta1", "scan");
		sleep(3);
		pfp = popen("wpa_cli -p /var/run/wpa_supplicant-sta1 -i sta1 scan_results", "r");
	}

	if (pfp != NULL) {
		getptr1 = qca_readfile(pfp, &fsize);
		pclose(pfp);

		if (getptr1 == NULL)
			return 0;

		getptr2 = strstr(getptr1, "\n");

		while (getptr2 != NULL) {
			*getptr2 = '\0';
			gettmp[idlength] = getptr1;
			getptr1 = getptr2 + 1;
			getptr2 = strstr(getptr1, "\n");
			idlength ++;
		}
	}
	else
		return 0;

	if ( band_chk )
		bandlist = getSsidRule0;
	else
		bandlist = getSsidRule1;

	//compare the SSID with SCAN LIST
	while( bandlist->length != 99 ) {
		if ( strlen(ssid_buf) > bandlist->length )
			substrr = ssid_buf + strlen(ssid_buf) - bandlist->length;
		else {
			bandlist++;
			continue;
		}

		if( bandlist->length == 0 ) {
			strcpy( substrl, ssid_buf );
			if ( qca_comparetmp( gettmp, idlength, substrl, bandlist->setpart1 ) ) {
				cmpresult=1;
				break ;
			}
			if ( *bandlist->setpart2 != '\0' ) {
				if ( qca_comparetmp( gettmp, idlength, substrl, bandlist->setpart2 ) ) {
					cmpresult=1;
					break;
				}
			}
		}
		else if( !strcmp(substrr, bandlist->cmppart) ){
			strncpy( substrl, ssid_buf, strlen(ssid_buf)-bandlist->length );
			substrl[strlen(ssid_buf)-bandlist->length] = '\0';
			if( qca_comparetmp( gettmp, idlength, substrl, bandlist->setpart1 ) ) {
				cmpresult=1;
				break;
			}
			if ( *bandlist->setpart2 != '\0' ) {
				if ( qca_comparetmp( gettmp, idlength, substrl, bandlist->setpart2 ) ) {
					cmpresult=1;
					break;
				}
			}
			if ( qca_comparetmp( gettmp, idlength, substrl, strNULL ) ) {
				cmpresult = 1;
				break;
			}
		}
		bandlist++;
	}

	if( cmpresult == 1 ) {
		TRACE_PT("=== Find the SSID : [ %s ]\n", substrl);
		strcpy(result,substrl);
	}
	else
		TRACE_PT("=== Can't find the SSID : [  ]\n");

	return cmpresult;
}

void qca_wps_state_check(void)
{

	if (nvram_get_int("wps_enable") != 1)
		return;

	if (nvram_get_int("wps_cli_state") != 1)
		return;

	if (sw_mode() != SW_MODE_REPEATER)
		return;

	/* Get 2.4G/5G WPS Status */
	char cmd[64] = {0}, wps_2g_ssid[32] = {0}, wps_5g_ssid[32] = {0};
	char *buf = malloc(sizeof(char) * 64);
	char *wpa_state = NULL, *wps_ssid = NULL;
	FILE *fp1 = NULL, *fp2 = NULL;
	int detect_result = 0;

	/* 0: Repeater. 1: Express way 2.4G 2: Express way 5G */
	int wlc_express = nvram_get_int("wlc_express");
	if (wlc_express < 0 || wlc_express > 2)
		wlc_express = 0;

	memset(buf, 0, sizeof(buf));

	if (wlc_express == 0 || wlc_express == 1) {
		sprintf(cmd, "wpa_cli -p /var/run/wpa_supplicant-sta0 -i sta0 status");
		fp1 = popen(cmd, "r");
	}
	if (wlc_express == 0 || wlc_express == 2) {
		sprintf(cmd, "wpa_cli -p /var/run/wpa_supplicant-sta1 -i sta1 status");
		fp2 = popen(cmd, "r");
	}
	if (fp1 != NULL) {
		while (fgets(buf, 64, fp1) != NULL) {
			if (strstr(buf, "ssid") && !strstr(buf, "bssid")) {
				wps_ssid = buf + 5;
				sprintf(wps_2g_ssid, "%s", wps_ssid);
				wps_2g_ssid[strlen(wps_2g_ssid)-1] = '\0';
			}

			if (strstr(buf, "wpa_state")) {
				wpa_state = buf + 10;

				if (strstr(wpa_state, "AUTHENTICATING")||
				     strstr(wpa_state, "ASSOCIATING") ||
				     strstr(wpa_state, "ASSOCIATED") ||
				     strstr(wpa_state, "ASSOCIATED") ||
				     strstr(wpa_state, "4WAY_HANDSHAKE") ||
				     strstr(wpa_state, "GROUP_HANDSHAKE"))
					nvram_set_int("led_status", LED_WPS_PROCESSING);

				if (strstr(wpa_state, "COMPLETED") && nvram_get_int("led_status") == LED_WPS_PROCESSING) {
					if (!wps_2g_done)
						qca_set_wps_result(0, 0);
					wps_2g_done = 1;
				}
			}
		}
	}

	if (fp2 != NULL) {
		while (fgets(buf, 64, fp2) != NULL) {

			if (strstr(buf, "ssid") && !strstr(buf, "bssid")) {
				wps_ssid = buf + 5;
				sprintf(wps_5g_ssid, "%s", wps_ssid);
				wps_5g_ssid[strlen(wps_5g_ssid)-1] = '\0';
			}

			if (strstr(buf, "wpa_state")) {
				wpa_state = buf + 10;

				if (strstr(wpa_state, "AUTHENTICATING")||
				     strstr(wpa_state, "ASSOCIATING") ||
				     strstr(wpa_state, "ASSOCIATED") ||
				     strstr(wpa_state, "ASSOCIATED") ||
				     strstr(wpa_state, "4WAY_HANDSHAKE") ||
				     strstr(wpa_state, "GROUP_HANDSHAKE"))
					nvram_set_int("led_status", LED_WPS_PROCESSING);

				if (strstr(wpa_state, "COMPLETED") && nvram_get_int("led_status") == LED_WPS_PROCESSING) {
					if (!wps_5g_done)
						qca_set_wps_result(1, 1);
					wps_5g_done = 1;
				}
			}
		}
	}

	if (wps_2g_done) {
		if (wlc_express == 0) {
			if(wps_5g_done || wait_5g_time >= MAX_WAIT_COUNT) {

				if (!wps_5g_done) {
					stop_wps_method();

					if (qca_auto_detect_ssid(1, wps_2g_ssid, wps_5g_ssid))
					{
						qca_set_wps_result(0, 1);
						nvram_set("wlc1_ssid", wps_5g_ssid);
						detect_result = 1;
					}
				}

				if (wps_5g_done) { // Both
					set_wps_result_to_ap_nvram(0, 1, 1);
					set_wps_result_to_ap_nvram(0, 0, 0);
				}
				else if (!wps_5g_done && detect_result) { // Both
					set_wps_result_to_ap_nvram(0, 1, 1);
					set_wps_result_to_ap_nvram(0, 0, 0);
				}
				else { // Only 2.4G
					set_wps_result_to_ap_nvram(1, 0, 1);
					set_wps_result_to_ap_nvram(1, 0, 0);
				}

				nvram_set("wps_cli_state", "2");
				wps_2g_done = 0;
				wps_5g_done = 0;
				wait_2g_time = 0;
				wait_5g_time = 0;
				wait_5g_time = 0;
				nvram_commit();
				notify_rc("restart_wireless");
				nvram_set_int("led_status", LED_WPS_RESTART_WL);
			}
			else
				 wait_5g_time++;
		}
		else {
			stop_wps_method();
			set_wps_result_to_ap_nvram(1, 0, 1);
			nvram_set("wps_cli_state", "2");
			wps_2g_done = 0;
			wps_5g_done = 0;
			wait_2g_time = 0;
			wait_5g_time = 0;
			wait_5g_time = 0;
			nvram_commit();
			notify_rc("restart_wireless");
			nvram_set_int("led_status", LED_WPS_RESTART_WL);
		}
	}

	if (wps_5g_done) {
		if (wlc_express == 0) {
			if(wps_2g_done || wait_2g_time >= MAX_WAIT_COUNT) {
				if (!wps_2g_done) {
					stop_wps_method();

					if (qca_auto_detect_ssid(0, wps_5g_ssid, wps_2g_ssid))
					{
						qca_set_wps_result(1, 0);
						nvram_set("wlc0_ssid", wps_2g_ssid);
						detect_result = 1;
					}
				}

				if (wps_2g_done) { // Both
					set_wps_result_to_ap_nvram(0, 1, 1);
					set_wps_result_to_ap_nvram(0, 0, 0);
				}
				else if (!wps_2g_done && detect_result) { // Both
					set_wps_result_to_ap_nvram(0, 1, 1);
					set_wps_result_to_ap_nvram(0, 0, 0);
				}
				else { // Only 5G
					set_wps_result_to_ap_nvram(1, 1, 1);
					set_wps_result_to_ap_nvram(1, 1, 0);
				}

				nvram_set("wps_cli_state", "2");
				wps_2g_done = 0;
				wps_5g_done = 0;
				wait_2g_time = 0;
				wait_5g_time = 0;
				nvram_commit();
				notify_rc("restart_wireless");
				nvram_set_int("led_status", LED_WPS_RESTART_WL);
			}
			else
				 wait_2g_time++;
		}
		else {
			stop_wps_method();
			set_wps_result_to_ap_nvram(1, 1, 0);
			nvram_set("wps_cli_state", "2");
			wps_2g_done = 0;
			wps_5g_done = 0;
			wait_2g_time = 0;
			wait_5g_time = 0;
			wait_5g_time = 0;
			nvram_commit();
			notify_rc("restart_wireless");
			nvram_set_int("led_status", LED_WPS_RESTART_WL);
		}
	}

	pclose(fp1);
	pclose(fp2);
	free(buf);

	return;
}
#endif /* RTCONFIG_CONCURRENTREPEATER */
#endif /* RTCONFIG_REALTEK */

#ifdef RTCONFIG_BSC_SR
int
wl_sr_config(char *ifname)
{
	FILE *fp;
	int sz = 0;
        char cmd[32], *ptr = NULL, *val = NULL;
	char buf[16] = {0};
	int bsc_dbg = nvram_get_int("bsc_dbg");

        snprintf(cmd, sizeof(cmd), "wl -i %s sr_config options", ifname);
	//_dprintf("%s, ifname is %s\n", __func__, ifname);

        fp = popen(cmd, "r");
        if (fp == NULL) {
                dprintf("Err: shared %s failed to open cmd = [%s] \n", __func__, cmd);
                return -1;
        }

 	sz = sizeof(buf) - 1;
	if (!fgets(buf, sz, fp)) {
		if(bsc_dbg) _dprintf("set sr_config option as 1\n");
		eval("wl", "-i", ifname, "sr_config", "options", "1");
	} else {
		buf[strcspn(buf, "\r\n")] = 0;
		if(*buf && bsc_dbg)
			_dprintf("sr_config:[%s]\n", buf);
	} 

        pclose(fp);

        return 1;
}

#define BSC_INTERVAL 	8

void bsc_sr_check(void)
{
        char lan_ifname[16], *lan_ifnames, *ifname, *p;
        int unit, subunit;
        char tmp[100], tmp2[100], prefix[] = "wlXXXXXXXXXXXXXX";
	static int chki = 0;
	int bsc_interval = nvram_get_int("bsc_interval")?:BSC_INTERVAL;
	int bsc_dbg = nvram_get_int("bsc_dbg");

	if(chki++ == bsc_interval) {
		chki = 0;	
	} else
		return 0;

        if(bsc_dbg) _dprintf("%s.....(c:%d)\n", __func__, client_mode());
        snprintf(lan_ifname, sizeof(lan_ifname), "%s", nvram_safe_get("lan_ifname"));
        if ((lan_ifnames = strdup(nvram_safe_get("lan_ifnames"))) != NULL) {
                p = lan_ifnames;
                while ((ifname = strsep(&p, " ")) != NULL) {
                        while (*ifname == ' ') ++ifname;
                        if (*ifname == 0) break;

                        unit = -1; subunit = -1;

                        // ignore disabled wl vifs
                        if (strncmp(ifname, "wl", 2) == 0) {
                                if (get_ifname_unit(ifname, &unit, &subunit) < 0) {
                                        continue;
				}
                        }
                        // get the instance number of the wl i/f
                        else if (wl_ioctl(ifname, WLC_GET_INSTANCE, &unit, sizeof(unit))) {
                                continue;
			}

			if(nvram_match("disable_sr", "1")) {
        			if(bsc_dbg) _dprintf("%s disable reset sr conifg\n", __func__);	
			} else
                        	wl_sr_config(ifname);

			if(nvram_match("disable_bsc", "1")) {
        			if(bsc_dbg) _dprintf("%s disable sync bsscolor\n", __func__);	
			} else
				record_bsscolor(ifname, unit, subunit, client_mode());
                }
        }
}
#endif

void service_check(void)
{
	static int boot_ready = 0;

	if (boot_ready > 6) {
#if defined(RTCONFIG_BCM_7114) || (defined(HND_ROUTER) && !defined(RTCONFIG_HND_ROUTER_AX))
	    dummy_alert_led_wifi();
#endif
		return;
	}

	if (!nvram_match("success_start_service", "1"))
		return;
#if defined(RTCONFIG_BCM_7114) || (defined(HND_ROUTER) && !defined(RTCONFIG_HND_ROUTER_AX))
	int idx;
	if (dummy_alert_led_pwr()) {
	    if(boot_ready) {
		for(idx=0; idx < 6; idx++) {
		    usleep(100*1000);
		    led_control(LED_POWER, idx%2);
		}
	    }
	    boot_ready++;
	}
	else
#endif
		led_control(LED_POWER, ++boot_ready%2);
}

#ifdef RTAC88U
int rtl_period = 10, sltime = 3, rtl_fail_max = 1;
static int rtl_count = 0, rtl_fail = 0;

void rtkl_check()
{
	if(rtl_count++ >= rtl_period) {
		rtl_count = 0;

		if((rtkswitch_ioctl(GET_AWARE, 0, 0)) < 0 )
			rtl_fail++;

		if(nvram_match("rtl_dbg", "1")) {
			_dprintf("NOW rtl_fail=%d\n", rtl_fail);
		}

		if(rtl_fail >= rtl_fail_max) {
			logmessage("rtl_fail", "\nrtkswitch fail access, restart.\n");
			_dprintf("rtl_fail:%d, hw reset it\n", rtl_fail);

			set_gpio(10, 0);
			sleep(1);
			set_gpio(10, 1);

			sleep(sltime);
			rtkswitch_ioctl(INIT_SWITCH, 0, 0);
			rtkswitch_ioctl(INIT_SWITCH_UP, 0, 0);
			rtkswitch_ioctl(SET_EXT_TXDELAY, 1, 0);
			rtkswitch_ioctl(SET_EXT_RXDELAY, 4, 0);
		
			rtl_fail = 0;
		}
	}
}
#endif

/* @return:
 * 	0:	not in MFG mode.
 *  otherwise:	in MFG mode.
 */
static int handle_btn_in_mfg(void)
{
	char msg[64];
	const struct mfg_btn_s *p;

	if (!nvram_match("asus_mfg", "1"))
		return 0;

	// TRACE_PT("asus mfg btn check!!!\n");
	for (p = &mfg_btn_table[0]; p->id < BTN_ID_MAX; ++p) {
		if (p->pre_req != NULL && !p->pre_req())
			continue;
		if (button_pressed(p->id)) {
			if (p->name) {
				snprintf(msg, sizeof(msg), "button %s pressed\n", p->name);
				TRACE_PT("%s", msg);
			}

			nvram_set(p->nv, "1");
		} else {
			/* TODO: handle button release. */
		}
	}

#ifdef RTCONFIG_WIRELESS_SWITCH
	if (!button_pressed(BTN_WIFI_SW)) {
		nvram_set("btn_wifi_sw", "0");
	}
#endif

#if defined(RTAC68U) && defined(RTCONFIG_LED_BTN)
	if (is_ac66u_v2_series())
		;
	else if (((!nvram_match("cpurev", "c0") || nvram_get_int("PA") == 5023 || (nvram_match("cpurev", "c0") && !nvram_get_int("PA"))) && button_pressed(BTN_LED)) ||
		   (nvram_match("cpurev", "c0") && nvram_get_int("PA") != 5023 && nvram_get_int("PA") != 0 && !button_pressed(BTN_LED)))
	{
		TRACE_PT("button LED pressed\n");
		nvram_set("btn_led", "1");
	}
	else if (!nvram_match("cpurev", "c0") || nvram_get_int("PA") == 5023 || (nvram_match("cpurev", "c0") && !nvram_get_int("PA")))
	{
		TRACE_PT("button LED released\n");
		nvram_set("btn_led", "0");
	}
#endif

#ifdef RTCONFIG_SWMODE_SWITCH
#if defined(PLAC66U)
	if (button_pressed(BTN_SWMODE_SW_ROUTER) != nvram_get_int("swmode_switch"))
	{
		TRACE_PT("Switch changeover\n");
		nvram_set("switch_mode", "1");
	}
#endif
#endif

	return 1;
}

#if defined(RTCONFIG_EJUSB_BTN) && defined(RTCONFIG_BLINK_LED)
struct ejusb_btn_s {
	enum btn_id btn;
	char *name;
	int state;
	float t1;
	unsigned int port_mask;
	unsigned int nr_pids;
	pid_t pids[8];
	char *led[2];
};


/**
 * Set USB LED blink pattern and enable it for eject USB button.
 * @p:
 * @mode:
 * 	2:	user defined pattern mode,
 * 		OFF 0.15s, ON 0.15s, OFF 0.05s, ON 0.05s, OFF 0.05s, ON 0.05s, OFF 0.05s, ON 0.05s
 * 	1:	user defined pattern mode,
 * 		OFF 0.7s, ON 0.7s
 *  otherwise:	normal mode
 */
static inline void set_usbled_for_ejusb_btn(struct ejusb_btn_s *p, int mode)
{
	int i;

	if (!p)
		return;

	switch (mode) {
	case 2:
		/* user defined pattern mode */
		for (i = 0 ; i < ARRAY_SIZE(p->led); ++i) {
			if (!p->led[i])
				break;

			set_bled_udef_pattern(p->led[i], 50, "0 0 0 1 1 1 0 1 0 1 0 1");
			set_bled_udef_pattern_mode(p->led[i]);
		}
		break;
	case 1:
		/* user defined pattern mode */
		for (i = 0 ; i < ARRAY_SIZE(p->led); ++i) {
			if (!p->led[i])
				break;

			set_bled_udef_pattern(p->led[i], 700, "0 1");
			set_bled_udef_pattern_mode(p->led[i]);
		}
		break;
	default:
		/* normal mode */
		for (i = 0 ; i < ARRAY_SIZE(p->led); ++i) {
			if (!p->led[i])
				break;

			set_bled_normal_mode(p->led[i]);
		}
	}
}


/**
 * Handle eject USB button.
 */
static void handle_eject_usb_button(void)
{
	int i, v, port, m, model;
	unsigned int c;
	char *gpio, nv[32], port_str[5] = "-1";
	char *ejusb_argv[] = { "ejusb", port_str, "1", "-u", "1", NULL };
	struct ejusb_btn_s *p;
	static unsigned int first = 1, nr_ejusb_btn = 0;
	static struct ejusb_btn_s ejusb_btn[] = {
		{
			.btn = BTN_EJUSB1,
			.name = "Eject USB button1",
			.state = 0,
			.port_mask = 0,
			.led = { NULL, NULL },
		},
		{
			.btn = BTN_EJUSB2,
			.name = "Eject USB button2",
			.state = 0,
			.port_mask = 0,
			.led = { NULL, NULL },
		}
	};

	/* If RESET button is triggered, don't handle eject USB button. */
	if (btn_pressed)
		return;

	if (first) {
		first = 0;
		p = &ejusb_btn[0];

		for (i = 0; i < ARRAY_SIZE(ejusb_btn); ++i) {
			sprintf(nv, "btn_ejusb%d_gpio", i + 1);
			if (!(gpio = nvram_get(nv)))
				continue;
			if (((v = atoi(gpio)) & GPIO_PIN_MASK) == GPIO_PIN_MASK)
				continue;

			nr_ejusb_btn++;
			p[i].port_mask = (v & GPIO_EJUSB_MASK) >> GPIO_EJUSB_SHIFT;
		}

		/* If DUT has two USB LED, associate 2-nd USB LED with eject USB button
		 * based on number of eject USB button.
		 */
		model = get_model();
		switch (nr_ejusb_btn) {
		case 2:
			if (have_usb3_led(model)) {
				p[0].led[0] = "led_usb_gpio";
				p[1].led[0] = "led_usb3_gpio";
			} else {
				p[0].led[0] = "led_usb_gpio";
				p[1].led[0] = "led_usb_gpio";
			}
			break;
		case 1:
			if (have_usb3_led(model)) {
				p[0].led[0] = "led_usb_gpio";
				p[0].led[1] = "led_usb3_gpio";
			} else {
				p[0].led[0] = "led_usb_gpio";
				p[0].led[1] = NULL;
			}
			break;
		default:
			nr_ejusb_btn = 0;
		}
	}
	if (!nr_ejusb_btn)
		return;

	for (i = 0, p = &ejusb_btn[0]; i < ARRAY_SIZE(ejusb_btn); ++i, ++p) {
		if (button_pressed(p->btn)) {
			TRACE_PT("%s pressed\n", p->name);
			/* button pressed */
			switch (p->state) {
			case 0:
				p->state = 1;
				p->t1 = uptime2();
				break;
			case 1:
				if ((int)(uptime2() - p->t1) >= EJUSB_WAIT_COUNT) {
					_dprintf("You can release %s now!\n", p->name);
					set_usbled_for_ejusb_btn(p, 1);
					p->state = 2;
				}
				break;
			case 2:
				/* nothing to do */
				break;
			case 3:
				/* nothing to do */
				break;
			default:
				_dprintf("%s: %s pressed, unknown state %d\n", __func__, p->state);
				p->state = 0;
				set_usbled_for_ejusb_btn(p, 0);
			}
		} else {
			/* button released */
			if (p->state)
				TRACE_PT("%s released\n", p->name);

			switch (p->state) {
			case 0:
				/* nothing to do */
				break;
			case 1:
				p->state = 0;
				break;
			case 2:
				/* Issue ejusb command. */
				p->nr_pids = 0;
				memset(&p->pids[0], 0, sizeof(p->pids));
				set_usbled_for_ejusb_btn(p, 2);
				if (p->port_mask) {
					for (m = p->port_mask, port = 1; m > 0; m >>= 1, port++) {
						if (!(m & 1))
							continue;
						sprintf(port_str, "%d", port);
						_eval(ejusb_argv, NULL, 0, &p->pids[p->nr_pids++]);
					}
				} else {
					strcpy(port_str, "-1");
					_eval(ejusb_argv, NULL, 0, &p->pids[p->nr_pids++]);
				}
				p->state = 3;
				break;
			case 3:
				for (i = 0, c = 0; p->nr_pids > 0 && i < p->nr_pids; ++i) {
					if (!p->pids[i])
						continue;
					if (!process_exists(p->pids[i]))
						p->pids[i] = 0;
					else
						c++;
				}
				if (!c) {
					p->state = 0;
					set_usbled_for_ejusb_btn(p, 0);
				}
				break;
			default:
				_dprintf("%s: %s released, unknown state %d\n", __func__, p->state);
				p->state = 0;
				set_usbled_for_ejusb_btn(p, 0);
			}
		}
	}
}
#else	/* !(RTCONFIG_EJUSB_BTN && RTCONFIG_BLINK_LED) */
static inline void handle_eject_usb_button(void) { }
#endif	/* RTCONFIG_EJUSB_BTN && RTCONFIG_BLINK_LED */

#if defined(RTCONFIG_TURBO_BTN) && defined(RTCONFIG_RGBLED)
static inline void toggle_aura_rgb_mode(int led_onoff)
{
	RGB_LED_STATUS_T rgb_cfg = { 0 };
	int status = -1;

	if (led_onoff)
		status = switch_rgb_mode("aurargb_val", &rgb_cfg, led_onoff);
	else
		status = 0;
	if (status == 0) {
		aura_rgb_led(ROUTER_AURA_SET, &rgb_cfg, 0, 0);
	}

	if (led_onoff == 1 && status == 0) {
#ifdef RTCONFIG_LOGO_LED
		led_control(LED_LOGO, LED_ON);
#endif
	}
	else if (led_onoff == 0 && status == 0) {
#ifdef RTCONFIG_LOGO_LED
		led_control(LED_LOGO, LED_OFF);
#endif
	}
}
#else
static inline void toggle_aura_rgb_mode(int led_onoff) { }
#endif

#if ((defined(RTCONFIG_LED_BTN) || defined(RTCONFIG_TURBO_BTN)) && defined(RTCONFIG_QCA))
static inline void __handle_led_onoff_button(int led_onoff)
{
#if defined(GTAXY16000) || defined(RTAX89U)
	int aqr_addr = aqr_phy_addr();
#endif
#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_SW_CTRL_ALLLED)
	pid_t pid;
	char *restart_cfgsync[] = { "service", "restart_cfgsync", NULL };
#endif
	int unit, onoff;
	int wl_led_id[WL_NR_BANDS] = { LED_2G, LED_5G, LED_5G2, LED_60G };
	char prefix[sizeof("wlXXX_")];

	nvram_set_int("AllLED", !!led_onoff);
#if defined(RTCONFIG_RGBLED)
	nvram_set_int("aurargb_enable", !!led_onoff);
	nvram_commit();	
	start_aurargb();
#endif
	if (led_onoff) {
		led_control(LED_POWER, LED_ON);

		kill_pidfile_s("/var/run/wanduck.pid", SIGUSR2);

		/* LAN LED */
#if defined(RTCONFIG_LAN4WAN_LED)
		led_control(LED_LAN1, LED_ON);
		led_control(LED_LAN2, LED_ON);
		led_control(LED_LAN3, LED_ON);
		led_control(LED_LAN4, LED_ON);
#else
		led_control(LED_LAN, LED_ON);
#endif

		/* WL LEDs */
		for (unit = 0; unit < WL_NR_BANDS; ++unit) {
			SKIP_ABSENT_BAND(unit);

			snprintf(prefix, sizeof(prefix), "wl%d_", unit);
			onoff = nvram_pf_match(prefix, "radio", "1")? LED_ON : LED_OFF;
			led_control(wl_led_id[unit], onoff);
		}

		logo_led_control(LED_ON);
		all_led_control(LED_ON);

		/* USB LED */
		kill_pidfile_s("/var/run/usbled.pid", SIGTSTP); // inform usbled to reset status
		/* check LED_WAN status */
		kill_pidfile_s("/var/run/wanduck.pid", SIGUSR2);

#if defined(GTAXY16000) || defined(RTAX89U)
		if (aqr_addr >= 0) {
			/* AQR107 LED0/2: GREEN, LED1: ORANGE */
			write_phy_reg(aqr_addr, 0x401EC430, 0xC0EF);
			write_phy_reg(aqr_addr, 0x401EC431, 0xC060);
			write_phy_reg(aqr_addr, 0x401EC432, 0x0080);
		}
#endif
	}
	else
		setAllLedOff();

#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_SW_CTRL_ALLLED)
	nvram_set_int("led_val", !!led_onoff);
	if (aimesh_re_node()) {
		/* To load latest led_val and update to CAP's UI, restart cfg_client. */
		_eval(restart_cfgsync, NULL, 0, &pid);
	}
#endif
}
#elif ((defined(RTCONFIG_LED_BTN) || defined(RTCONFIG_TURBO_BTN)) && (defined(RTCONFIG_HND_ROUTER) || defined(RTCONFIG_HND_ROUTER_AX)))
static inline void __handle_led_onoff_button(int led_onoff)
{
	nvram_set_int("AllLED", !!led_onoff);
#if defined(RTCONFIG_RGBLED)
	nvram_set_int("aurargb_enable", !!led_onoff);
	nvram_commit();
	start_aurargb();
#endif
	if (led_onoff) {
		led_control(LED_POWER, LED_ON);
		kill_pidfile_s("/var/run/wanduck.pid", SIGUSR2);
#if defined(TUFAX3000_V2) || defined(RTAXE7800) || defined(EBG19_504)
		bcm53134_led_control(2);
#endif
#if defined(HND_ROUTER)
#if defined(TUFAX3000_V2) || defined(RTAXE7800) || defined(TUFAX5400_V2) || defined(RTAX5400) || defined(RTAX88U_PRO)
#if defined(TUFAX3000_V2) || defined(RTAXE7800)
		if (!nvram_get_int("wans_extwan"))
#endif
		lan_phy_led_pinmux(0);
#endif
		setLANLedOn();
#endif
#if defined(GTAXE11000)
		eval("wl", "-i", "eth6", "ledbh", "9", "7");
		eval("wl", "-i", "eth7", "ledbh", "9", "7");
		eval("wl", "-i", "eth8", "ledbh", "9", "7");
#endif
#if defined(RTAX88U) || defined(GTAX11000)
		eval("wl", "-i", "eth6", "ledbh", "15", "7");
		eval("wl", "-i", "eth7", "ledbh", "15", "7");
#if defined(GTAX11000)
		eval("wl", "-i", "eth8", "ledbh", "15", "7");
#endif
#endif
#if defined(GTAX6000)
		eval("wl", "-i", "eth6", "ledbh", "13", "7");
		eval("wl", "-i", "eth7", "ledbh", "13", "7");
#endif
#if defined(GTAX11000_PRO)
		eval("wl", "-i", "eth6", "ledbh", "13", "7");
		eval("wl", "-i", "eth7", "ledbh", "13", "7");
		eval("wl", "-i", "eth8", "ledbh", "13", "7");
#endif
#if defined(GTAXE16000)
		eval("wl", "-i", "eth7", "ledbh", "13", "7");
		eval("wl", "-i", "eth8", "ledbh", "13", "7");
		eval("wl", "-i", "eth9", "ledbh", "13", "7");
		eval("wl", "-i", "eth10", "ledbh", "13", "7");
#endif
#if defined(GTAX11000) || defined(GTAXE11000)
#ifdef RTCONFIG_EXTPHY_BCM84880
		eval("ethctl", "phy", "ext", EXTPHY_ADDR_STR, "0x7fff0", "0x1");
		eval("ethctl", "phy", "ext", EXTPHY_ADDR_STR, "0x1a832", "0x6");
#endif
#endif
#ifdef GTAX6000
		if (nvram_get_int("ext_phy_model") == EXT_PHY_GPY211)
			eval("ethctl", "phy", "ext", EXTPHY_GPY_ADDR_STR, "0x1e0001", "0x3f0");
		else
		{
			eval("ethctl", "phy", "ext", EXTPHY_ADDR_STR, "0x1a832", "0x6");	// default. CTL LED3 MASK LOW
			eval("ethctl", "phy", "ext", EXTPHY_ADDR_STR, "0x1a835", "0x40");	// default. CTL LED4 MASK LOW
		}
#endif
#if defined(GTAX11000_PRO)
		// TBD, 2.5G LED
		notify_rc_and_wait_2min("restart_ledg");
#endif
#if defined(GTAXE16000)
		// TBD, 10G LED
		notify_rc_and_wait_2min("restart_ledg");
#endif
#ifdef RTCONFIG_LOGO_LED
		led_control(LED_LOGO, LED_ON);
#endif
		kill_pidfile_s("/var/run/usbled.pid", SIGTSTP); // inform usbled to reset status
	}
	else
		setAllLedOff();
}
#else
static inline void __handle_led_onoff_button(int led_onoff) { }
#endif

#if defined(RTCONFIG_TURBO_BTN) && defined(RTCONFIG_BWDPI)
static inline void toggle_qos_gameboost_mode(int led_onoff)
{
#if defined(RTCONFIG_NOTIFICATION_CENTER)
	int notify_nt = 0;
#endif
	if (led_onoff) {
#if defined(RTCONFIG_RGBLED) && defined(GTAC2900)
		send_aura_event("BOOST_GAME_BOOST_SW");
#elif defined(RTCONFIG_LOGO_LED)
		led_control(LED_LOGO, LED_ON);
#endif
#if defined(RTCONFIG_NOTIFICATION_CENTER)
		if(!nvram_match("qos_enable", "1") || !nvram_match("qos_type", "1") || !nvram_match("bwdpi_app_rulelist", "9,20<8<4<0,5,6,15,17<13,24<1,3,14<7,10,11,21,23<<game"))
			notify_nt = 1;
#endif
		nvram_set_int("qos_enable", 1);
		nvram_set_int("qos_type", 1);
		nvram_set("bwdpi_app_rulelist", "9,20<8<4<0,5,6,15,17<13,24<1,3,14<7,10,11,21,23<<game");
		start_dpi_engine_service();
	}
	else {
#ifdef RTCONFIG_LOGO_LED
		led_control(LED_LOGO, LED_OFF);
#endif
#if defined(RTCONFIG_NOTIFICATION_CENTER)
		if(!nvram_match("qos_enable", "0"))
			notify_nt = 1;
#endif
		nvram_set_int("qos_enable", 0);
		stop_dpi_engine_service(0);
	}
#if defined(RTCONFIG_NOTIFICATION_CENTER)
	if(notify_nt)
		RC_SEND_NT_EVENT(GENERAL_QOS_UPDATE, NULL);
#endif
	start_firewall(wan_primary_ifunit(), 0);
}
#else
static inline void toggle_qos_gameboost_mode(int led_onoff) { }
#endif	/*  RTCONFIG_TURBO_BTN && RTCONFIG_BWDPI */

#if defined(RTCONFIG_RGBLED) && defined(GTAC2900)
static inline void toggle_aura_shuffle()
{
	if (inhibit_led_on() || !nvram_get_int("aurargb_enable"))
		return;
#ifdef RTCONFIG_AMAS
	if (nvram_get_int("re_mode") == 1)
		return;
#endif
	int i=0, status = -1, len=0;
	int shuffle_array[11] = {0,1,2,5,8,3,12,4,13,6,11};
	char nv[30];
	RGB_LED_STATUS_T rgb_cfg = { 0 };

	len = sizeof(shuffle_array)/sizeof(int);
	status = nv_to_rgb("aurargb_val", &rgb_cfg);

	if(status != -1){
		for(i=0;i<len;i++){
			if(rgb_cfg.mode == shuffle_array[i]){
				rgb_cfg.mode = shuffle_array[(i+1)%11];
				break;
			}
		}
		snprintf(nv, sizeof(nv), "%d,%d,%d,%d,%d,%d", rgb_cfg.red, rgb_cfg.green, rgb_cfg.blue, rgb_cfg.mode, rgb_cfg.speed, rgb_cfg.direction);
		nvram_set("aurargb_val", nv);
		nvram_commit();
		aura_rgb_led(ROUTER_AURA_SET, &rgb_cfg, 0, 0);
		dbg("toggle_aura_shuffle: %s\n", nv);
	}
	return;
}
#endif

#if defined(RTCONFIG_LED_BTN) && !defined(RTCONFIG_TURBO_BTN) && defined(RTCONFIG_QCA)
static inline void handle_led_onoff_button(void)
{
	/* If RESET button is triggered, don't handle LED ON/OFF button. */
	if (btn_pressed)
		return;

	LED_status_old = LED_status;
	LED_status = button_pressed(BTN_LED);

	if (!LED_status &&
	    (LED_status != LED_status_old))
	{
		LED_status_changed = 1;
		if (LED_status_first)
		{
			LED_status_first = 0;
			LED_status_on = !nvram_get_int("AllLED");
		}
		else
			LED_status_on = 1 - LED_status_on;
	}
	else
		LED_status_changed = 0;

	if (!LED_status_changed)
		return;

	TRACE_PT("LED ON/OFF button status changed\n");
	__handle_led_onoff_button(LED_status_on);
}
#else
static inline void handle_led_onoff_button(void) { }
#endif	/* RTCONFIG_LED_BTN && RTCONFIG_QCA */

#ifdef RTCONFIG_GEFORCENOW
static inline void toggle_geforcenow_mode(int led_onoff)
{
#if defined(RTCONFIG_NOTIFICATION_CENTER)
	int notify_nt = 0;
#endif
	if (led_onoff) {
#if defined(RTCONFIG_RGBLED) && defined(GTAC2900)
		send_aura_event("BOOST_GEFORCENOW");
#endif
#if defined(RTCONFIG_NOTIFICATION_CENTER)
		if(!nvram_match("qos_enable", "1") || !nvram_match("qos_type", "3"))
			notify_nt = 1;
#endif
		nvram_set_int("nvgfn_enable", 1);
		nvram_set_int("qos_enable", 1);
		nvram_set_int("qos_type", 3);
	}
	else {
#if defined(RTCONFIG_NOTIFICATION_CENTER)
		if(!nvram_match("qos_enable", "0"))
			notify_nt = 1;
#endif
		nvram_set_int("nvgfn_enable", 0);
		nvram_set_int("qos_enable", 0);
	}
#if defined(RTCONFIG_NOTIFICATION_CENTER)
	if(notify_nt)
		RC_SEND_NT_EVENT(GENERAL_QOS_UPDATE, NULL);
#endif

	nvram_commit();
	notify_rc("restart_qos");
	notify_rc("restart_firewall");
}
#endif

#if defined(RTCONFIG_TURBO_BTN)
static inline void handle_turbo_button(void)
{
	static int turbo_status_old = 1, boost_mode_old = -1;
	static int turbo_status = 0;
	static int turbo_status_on = 0;
	int *bstatus, acs_dfs, onoff, boost_mode, sw_mode __attribute__((unused)) = sw_mode();
	const char *boost_led = "led_turbo_gpio";

	boost_mode = nvram_get_int("turbo_mode");
	if (!strlen(nvram_safe_get("turbo_mode")) || boost_mode < 0 || boost_mode >= BOOST_MODE_MAX)
		boost_mode = BOOST_LED_SW;

#ifdef RTCONFIG_LED_BTN
	if (boost_mode == BOOST_LED_SW && nvram_get_int("btn_led_gpio") != 255 && nvram_get_int("btn_led_gpio") != 0)
		boost_mode++;	/* BOOST_ACS_DFS_SW */
#endif
	if (boost_mode == BOOST_ACS_DFS_SW
		&& (!is_routing_enabled() && !__access_point_mode(sw_mode))
#if defined(RTCONFIG_BCMWL6) && defined(RTCONFIG_PROXYSTA)
		&& !psr_exist_except(0)
#endif
	)
		boost_mode++;	/* BOOST_AURA_RGB_SW */
#ifdef GTAC2900
	if (boost_mode == BOOST_AURA_RGB_SW)
		boost_mode++;	/* BOOST_GAME_BOOST_SW */
#endif

	if (nvram_get_int("turbo_mode") != boost_mode)
		nvram_set_int("turbo_mode", boost_mode);

	bstatus = &g_boost_status[boost_mode];

	/* If boost mode changed, update boost LED based on old status of selected mode. */
	if (boost_mode != boost_mode_old) {
		boost_mode_old = boost_mode;
		boost_led_control((!inhibit_led_on() && *bstatus)? LED_ON : LED_OFF);
	}

#if defined(RTCONFIG_RGBLED)
	/* If Aura RGB is selected and aurargb_enable is changed by UI, update related variables. */
	if ((boost_mode == BOOST_AURA_RGB_SW || boost_mode == BOOST_AURA_SHUFFLE_SW) &&
		*bstatus != nvram_get_int("aurargb_enable")) {
		*bstatus = !!nvram_get_int("aurargb_enable");
		boost_led_control((!inhibit_led_on() && *bstatus)? LED_ON : LED_OFF);
	}
#endif

	/* If DFS in ACS is selected, don't process it if WiFi not ready or not in router/access-point mode. */
	if (boost_mode == BOOST_ACS_DFS_SW) {
		if (!nvram_get_int(WLREADY))
			return;
		else if (!is_routing_enabled() && !__access_point_mode(sw_mode))
			return;

		/* If DFS in ACS changed by another code, e.g. UI, copy latest setting here. */
		acs_dfs = !!nvram_get_int("acs_dfs");
		if (*bstatus != acs_dfs) {
			*bstatus = acs_dfs;
			boost_led_control((!inhibit_led_on() && *bstatus)? LED_ON : LED_OFF);
		}
	}

	if (boost_mode == BOOST_GAME_BOOST_SW && !is_routing_enabled())
		return;

#ifdef RTCONFIG_GEFORCENOW
	if (boost_mode == BOOST_GEFORCENOW && !is_routing_enabled())
		return;
#endif

	/* If RESET button is triggered, don't handle turbo button. */
	if (btn_pressed)
		return;

	turbo_status_old = turbo_status;
	turbo_status = button_pressed(BTN_TURBO);
#if defined(RTCONFIG_BLINK_LED)
	if ((turbo_status && turbo_status != turbo_status_old) &&
	    (nvram_get_int(boost_led) & GPIO_BLINK_LED)) {
		boost_led_control(LED_ON);
		set_bled_udef_pattern(boost_led, 150, "0 1");
		set_bled_udef_pattern_mode(boost_led);
	}
#endif
	if (turbo_status || turbo_status == turbo_status_old)
		return;

	TRACE_PT("Turbo ON/OFF button status changed\n");
	turbo_status_on = !!(turbo_status_on ^ 1);
	*bstatus = !!(*bstatus ^ 1);
	onoff = (!inhibit_led_on() && *bstatus)? LED_ON : LED_OFF;

	switch(boost_mode) {
#if defined(RTCONFIG_RGBLED)
	case BOOST_AURA_RGB_SW:
		nvram_set_int("aurargb_enable", *bstatus);
		nvram_commit();
		toggle_aura_rgb_mode(*bstatus);
		break;
#elif defined(RTCONFIG_BCM_CLED)
	case BOOST_AURA_RGB_SW:
		nvram_set_int("ledg_led_enable", *bstatus);
		if (nvram_get_int("ledg_scheme") != LEDG_SCHEME_OFF)
		{
			nvram_set_int("ledg_scheme_old", nvram_get_int("ledg_scheme"));
			nvram_set_int("ledg_scheme", LEDG_SCHEME_OFF);
		}
		else
			nvram_set_int("ledg_scheme", nvram_get_int("ledg_scheme_old"));
		nvram_commit();
		kill_pidfile_s("/var/run/ledg.pid", SIGTSTP);
		break;
#endif
	case BOOST_GAME_BOOST_SW:
		toggle_qos_gameboost_mode(*bstatus);
		break;
	case BOOST_ACS_DFS_SW:
		toggle_dfs_in_acs(*bstatus);
		break;
	case BOOST_LED_SW:
#ifdef RTCONFIG_LED_BTN
		if (nvram_get_int("btn_led_gpio") == 255 || nvram_get_int("btn_led_gpio") == 0)
#endif
			__handle_led_onoff_button(*bstatus);
		break;
#if defined(RTCONFIG_RGBLED) && defined(GTAC2900)
	case BOOST_AURA_SHUFFLE_SW:
		toggle_aura_shuffle();
		break;
#endif
#ifdef RTCONFIG_GEFORCENOW
	case BOOST_GEFORCENOW:
		toggle_geforcenow_mode(*bstatus);
		break;
#endif
	}
	set_bled_normal_mode(boost_led);
	boost_led_control(onoff);
}
#else
static inline void handle_turbo_button(void) { }
#endif	/* RTCONFIG_TURBO_BTN */

#if defined(RTCONFIG_RGBLED) && defined(GTAC2900)
void aura_led_control(char *rgb)
{
	RGB_LED_STATUS_T rgb_cfg = { 0 };

	nvram_set("pause_aura_rgb_nt", "1");
	__nv_to_rgb(rgb, &rgb_cfg);
	aura_rgb_led(ROUTER_AURA_SET, &rgb_cfg, 0, 0);
}
#endif

#ifdef DSL_AX82U
void ledg_scheme_switch(void)
{
	int ledg_scheme = nvram_get_int("ledg_scheme");

	if ((ledg_scheme >= LEDG_SCHEME_BLINKING) || (ledg_scheme == LEDG_SCHEME_OFF)) {
		if (ledg_scheme == LEDG_SCHEME_OFF)
			LED_status = 1;
		ledg_scheme = LEDG_SCHEME_WATER_FLOW;
	} else {
		ledg_scheme = (ledg_scheme + 1) % (LEDG_SCHEME_MAX - 2);
		if (ledg_scheme == LEDG_SCHEME_OFF)
			ledg_scheme = LEDG_SCHEME_GRADIENT;
	}

	nvram_set_int("ledg_scheme", ledg_scheme);
	nvram_commit();
	nvram_set_int("ledg_frombtn", 1);

	dbg("switch effect to %d\n", ledg_scheme);
	kill_pidfile_s("/var/run/ledg.pid", SIGTSTP);
}
#endif

void btn_check(void)
{
#ifdef RTCONFIG_WIFI_SON
	pid_t pid;
	char *argv[]={"/sbin/delay_exec","4","rc rc_service restart_allnet",NULL};
#endif

	eval("touch", "/tmp/watchdog_heartbeat");

	if (handle_btn_in_mfg())
		return;

#ifdef BTN_SETUP
	if (btn_pressed_setup == BTNSETUP_NONE)
	{
#endif
#if !(definedRTCONFIG_N56U_SR2)
		if (button_pressed(BTN_RESET))
#else
		if (0)
#endif
		{
			TRACE_PT("button RESET pressed\n");

	/*--------------- Add BTN_RST MFG test ------------------------*/
#ifndef RTCONFIG_WPS_RST_BTN
#if defined(RTCONFIG_DSL) || defined(EBG15) || defined(EBG19) /* Paul add 2013/4/2 */
			if ((btn_count % 2) == 0)
				led_control(LED_POWER, LED_ON);
			else
				led_control(LED_POWER, LED_OFF);
#endif
#endif	/* ! RTCONFIG_WPS_RST_BTN */
			if (!btn_pressed)
			{
				btn_pressed = 1;
				btn_count = 0;
				alarmtimer(0, URGENT_PERIOD);
#if defined(RTCONFIG_BCM_CLED) && defined(RTCONFIG_SINGLE_LED)
				nvram_unset("bcm_cled_in_reset");
#endif
			}
			else
			{	/* Whenever it is pushed steady */
				if (++btn_count > RESET_WAIT_COUNT)
				{
					dbg("You can release RESET button now!\n");
#if defined(RTCONFIG_BCM_CLED) && defined(RTCONFIG_SINGLE_LED)
			nvram_set("bcm_cled_in_reset", "1");
			if ((btn_count % 2) == 0)
				bcm_cled_ctrl(BCM_CLED_YELLOW, BCM_CLED_STEADY_NOBLINK);
			else
				bcm_cled_ctrl(BCM_CLED_OFF, BCM_CLED_STEADY_NOBLINK);
#endif
#ifdef BLUECAVE
					if(btn_pressed == 1) {
						nvram_set("bc_ledbh", "reset");
						kill_pidfile_s("/var/run/sw_devled.pid", SIGUSR1);
					}
					run_app_script(NULL, "stop");
					system("ejusb -1");
#endif
#if (defined(PLN12) || defined(PLAC56))
					if (btn_pressed == 1)
						set_wifiled(5);
#elif defined(RTCONFIG_FIXED_BRIGHTNESS_RGBLED)
					if (btn_pressed == 1)
						set_rgbled(RGBLED_PRESS_RSTBTN);
#elif defined(RTCONFIG_REALTEK)
					if (btn_pressed == 1)
						set_led(LED_BLINK_SLOW, LED_BLINK_SLOW);
#endif
					btn_pressed = 2;
				}
				if (btn_pressed == 2)
				{
#ifdef RTCONFIG_DSL_REMOTE
					led_control(LED_POWER, LED_OFF);
					alarmtimer(0, 0);
					nvram_set("restore_defaults", "1");
					if (notify_rc_after_wait("resetdefault")) {
						/* Send resetdefault rc_service failed. */
						alarmtimer(NORMAL_PERIOD, 0);
					}
#elif defined(RTCONFIG_LP5523)
					if (btn_count == RESET_WAIT_COUNT+1)
						lp55xx_leds_proc(LP55XX_ORANGE_LEDS, LP55XX_RESET_SUCCESS);
#elif defined(RTCONFIG_FIXED_BRIGHTNESS_RGBLED)
#else
				/* 0123456789 */
				/* 0011100111 */
					if ((btn_count % 10) < 2 || ((btn_count % 10) > 4 && (btn_count % 10) < 7))
#if defined(RTN11P) || defined(RTN300)
					{
						led_control(LED_LAN, LED_OFF);
						led_control(LED_WAN, LED_OFF);
						led_control(LED_WPS, LED_OFF);
					}
					else
					{
						led_control(LED_LAN, LED_ON);
						led_control(LED_WAN, LED_ON);
						led_control(LED_WPS, LED_ON);
					}
#else	/* ! (RTN11P || RTN300) */
					{
						led_control(LED_POWER, LED_OFF);
#if defined(RTCONFIG_RGBLED) && defined(GTAC2900)
						aura_led_control(AURA_LED_OFF);
#endif
					}
					else
					{
						led_control(LED_POWER, LED_ON);
#if defined(RTCONFIG_RGBLED) && defined(GTAC2900)
						aura_led_control(AURA_LED_RST);
#endif
					}
#endif	/* ! (RTN11P || RTN300) */
#endif
				}
			}
		}
#if defined(RTCONFIG_WIRELESS_SWITCH) && defined(RTCONFIG_DSL)
		else if (button_pressed(BTN_WIFI_SW))
		{
		//TRACE_PT("button BTN_WIFI_SW pressed\n");
			if (wlan_sw_init == 0)
			{
				wlan_sw_init = 1;
/*
				eval("iwpriv", "ra0", "set", "RadioOn=1");
				eval("iwpriv", "rai0", "set", "RadioOn=1");
				TRACE_PT("Radio On\n");
				nvram_set("wl0_radio", "1");
				nvram_set("wl1_radio", "1");
				nvram_commit();
*/
			}
			else
			{
				// if wlan switch on , btn reset routine goes here
				if (btn_pressed == 2)
				{
					// IT MUST BE SAME AS BELOW CODE
					led_control(LED_POWER, LED_OFF);
					alarmtimer(0, 0);
					nvram_set("restore_defaults", "1");
					if (notify_rc_after_wait("resetdefault")) {
						/* Send resetdefault rc_service failed. */
						alarmtimer(NORMAL_PERIOD, 0);
					}
				}

				if (nvram_match("wl0_HW_switch", "0") || nvram_match("wl1_HW_switch", "0")) {
					//Ever apply the Wireless-Professional Web GU.
					//Not affect the status of WiFi interface, so do nothing
				}
				else{	//trun on WiFi by HW slash, make sure both WiFi interface enable.
					if (nvram_match("wl0_radio", "0") || nvram_match("wl1_radio", "0")) {
						eval("iwpriv", "ra0", "set", "RadioOn=1");
						eval("iwpriv", "rai0", "set", "RadioOn=1");
						TRACE_PT("Radio On\n");
						nvram_set("wl0_radio", "1");
						nvram_set("wl1_radio", "1");

						nvram_set("wl0_HW_switch", "0");
						nvram_set("wl1_HW_switch", "0");
						nvram_commit();
					}
				}
			}
		}
#endif	/* RTCONFIG_WIRELESS_SWITCH && RTCONFIG_DSL */
		else
		{
#ifdef RTCONFIG_WPS_RST_BTN
			if (btn_pressed == 0)
				;
			else if (btn_count < WPS_RST_DO_RESTORE_COUNT)
			{
				if (btn_count < WPS_RST_DO_RESTORE_COUNT && btn_count >= WPS_RST_DO_WPS_COUNT && nvram_match("btn_ez_radiotoggle", "1"))
				{
					radio_switch(0);
				}
#ifdef RTCONFIG_WPS_ALLLED_BTN
				else if (btn_count < WPS_RST_DO_RESTORE_COUNT && btn_count >= WPS_RST_DO_WPS_COUNT && nvram_match("btn_ez_mode", "1"))
				{
					LED_status_on = nvram_get_int("AllLED");


					if (LED_status_on)
						nvram_set_int("AllLED", 0);
					else
						nvram_set_int("AllLED", 1);
					LED_status_on = !LED_status_on;

					if (LED_status_on) {
						TRACE_PT("LED turn to normal\n");
						led_control(LED_POWER, LED_ON);
#if defined(RTN11P_B1)
						system("reg s 0xB0000000; reg w 0x64 0x30015014");
						system("reg s 0xB0000600; reg w 0x04 0x1C20; reg w 0x24 0x69CB");
						led_control(LED_WAN, LED_ON);
						led_control(LED_LAN, LED_ON);
						led_control(LED_2G, LED_ON);
#endif
					}
					else {
						TRACE_PT("LED turn off\n");
						setAllLedOff();
					}

					/* check LED_WAN status */
					kill_pidfile_s("/var/run/wanduck.pid", SIGUSR2);
				}
#endif
				if (btn_count < WPS_RST_DO_RESTORE_COUNT && btn_count >= WPS_RST_DO_WPS_COUNT
				   && !no_need_to_start_wps()
				   && !wps_band_radio_off(get_radio_band(0))
				   && !wps_band_ssid_broadcast_off(get_radio_band(0))
				   && nvram_match("btn_ez_radiotoggle", "0")
#ifdef RTCONFIG_WPS_ALLLED_BTN
				   && nvram_match("btn_ez_mode", "0")
#endif
				   && !nvram_match("wps_ign_btn", "1")
				)
				{
					btn_pressed_setup = BTNSETUP_DETECT;
					btn_count_setup = WPS_WAIT_COUNT;	//to trigger WPS
					alarmtimer(0, RUSHURGENT_PERIOD);
				}
				else
				{
					btn_pressed_setup = BTNSETUP_NONE;
					btn_count_setup = 0;
					alarmtimer(NORMAL_PERIOD, 0);
				}

#ifdef RTCONFIG_WPS_ALLLED_BTN
				if (!(btn_count < WPS_RST_DO_RESTORE_COUNT && btn_count >= WPS_RST_DO_WPS_COUNT && nvram_match("btn_ez_mode", "1")))
#endif
				led_control(LED_POWER, LED_ON);

				btn_count = 0;
				btn_pressed = 0;
			}
			else if (btn_count >= WPS_RST_DO_RESTORE_COUNT)	// to do restore
#else	/* ! RTCONFIG_WPS_RST_BTN */
			if (btn_pressed == 1)
			{
				btn_count = 0;
				btn_pressed = 0;
#if defined(RTCONFIG_LP5523)
				lp55xx_leds_proc(LP55XX_ALL_LEDS_OFF, LP55XX_PREVIOUS_STATE);
#elif defined(RTCONFIG_FIXED_BRIGHTNESS_RGBLED)
#else
				led_control(LED_POWER, LED_ON);
#endif
				alarmtimer(NORMAL_PERIOD, 0);
			}
			else if (btn_pressed == 2)
#endif	/* ! RTCONFIG_WPS_RST_BTN */
			{
#if defined(RTN11P_B1)
				stop_wanduck();
				//Set WLED_N(GPIO44) to GPIO Mode, and turn on.
				system("reg s 0xB0000000; reg w 0x64 0x30015015");
				system("reg s 0xB0000600; reg w 0x04 0x1C20; reg w 0x24 0x69CB");
				led_control(LED_POWER, LED_OFF);
				led_control(LED_WAN, LED_OFF);
				led_control(LED_LAN, LED_OFF);
				led_control(LED_2G, LED_OFF);
				sleep(1);
				led_control(LED_WAN, LED_ON);
				led_control(LED_LAN, LED_ON);
				led_control(LED_2G, LED_ON);
				sleep(1);
				led_control(LED_WAN, LED_OFF);
				led_control(LED_LAN, LED_OFF);
				led_control(LED_2G, LED_OFF);
				sleep(1);
				led_control(LED_POWER, LED_ON);
				sleep(1);
#elif defined(RTCONFIG_LP5523)
				lp55xx_leds_proc(LP55XX_ORANGE_LEDS, LP55XX_ACT_BREATH_DOWN_00);
#elif defined(PLN12) || defined(PLAC56)
				set_wifiled(2);
#elif defined(RTCONFIG_FIXED_BRIGHTNESS_RGBLED)
				set_rgbled(RGBLED_RST_EVENT);
#else
				led_control(LED_POWER, LED_OFF);
#if defined(RTCONFIG_RGBLED) && defined(GTAC2900)
				aura_led_control(AURA_LED_OFF);
#endif
#endif
				alarmtimer(0, 0);
				nvram_set("restore_defaults", "1");
				if(notify_rc_after_wait("resetdefault_erase"))
				{
					/* Send resetdefault rc_service failed. */
					alarmtimer(NORMAL_PERIOD, 0);
				}
			}
#if defined(RTCONFIG_WIRELESS_SWITCH) && defined(RTCONFIG_DSL)
			else
			{
				// no button is pressed or released
				if (wlan_sw_init == 0)
				{
					wlan_sw_init = 1;
					eval("iwpriv", "ra0", "set", "RadioOn=0");
					eval("iwpriv", "rai0", "set", "RadioOn=0");
					TRACE_PT("Radio Off\n");
					nvram_set("wl0_radio", "0");
					nvram_set("wl1_radio", "0");

					nvram_set("wl0_HW_switch", "1");
					nvram_set("wl1_HW_switch", "1");

					nvram_commit();
				}
				else
				{
					if (nvram_match("wl0_radio", "1") || nvram_match("wl1_radio", "1")) {
						eval("iwpriv", "ra0", "set", "RadioOn=0");
						eval("iwpriv", "rai0", "set", "RadioOn=0");
						TRACE_PT("Radio Off\n");
						nvram_set("wl0_radio", "0");
						nvram_set("wl1_radio", "0");

						nvram_set("wl0_timesched", "0");
						nvram_set("wl1_timesched", "0");
					}

					//indicate use switch HW slash manually.
					if (nvram_match("wl0_HW_switch", "0") || nvram_match("wl1_HW_switch", "0")) {
						nvram_set("wl0_HW_switch", "1");
						nvram_set("wl1_HW_switch", "1");
					}
				}
			}
#endif	/* RTCONFIG_WIRELESS_SWITCH && RTCONFIG_DSL */
		}

#ifdef BTN_SETUP
	}

	if (btn_pressed != 0) return;

#if defined(RTCONFIG_EJUSB_BTN) && defined(RTCONFIG_BLINK_LED)
	handle_eject_usb_button();
#endif

#if defined(CONFIG_BCMWL5) || defined(RTCONFIG_QCA)
	// wait until wl is ready
	if (!nvram_get_int("wlready")) return;
#endif

#if defined(RTCONFIG_WIRELESS_SWITCH) && defined(RTCONFIG_QCA)
	if (wifi_sw_old != button_pressed(BTN_WIFI_SW))
	{
		wifi_sw_old = button_pressed(BTN_WIFI_SW);
		if (wifi_sw_old != 0 && (!nvram_match("wl0_HW_switch", "0") || !nvram_match("wl1_HW_switch", "0")))
		{
			TRACE_PT("button BTN_WIFI_SW pressed: ON\n");
			nvram_set("wl0_radio", "1");
			nvram_set("wl1_radio", "1");
			nvram_set("wl0_HW_switch", "0");	// 0 to be ON
			nvram_set("wl1_HW_switch", "0");	// 0 to be ON
			nvram_commit();
			eval("radio", "on");			// ON
		}
		else if (wifi_sw_old == 0 && (!nvram_match("wl0_HW_switch", "1") || !nvram_match("wl1_HW_switch", "1")))
		{
			TRACE_PT("button BTN_WIFI_SW released: OFF\n");
			eval("radio", "off");			// OFF
			nvram_set("wl0_radio", "0");
			nvram_set("wl1_radio", "0");
			nvram_set("wl0_HW_switch", "1");	// 1 to be OFF
			nvram_set("wl1_HW_switch", "1");	// 1 to be OFF
			nvram_commit();
		}
	}
#endif	/* RTCONFIG_WIRELESS_SWITCH && RTCONFIG_QCA */


#ifndef RTCONFIG_WPS_RST_BTN
	// independent wifi-toggle btn or Added WPS button radio toggle option
#if defined(RTCONFIG_WIFI_TOG_BTN)
	if (button_pressed(BTN_WIFI_TOG))
#else
	if (nvram_match("btn_ez_radiotoggle", "1") &&
	    (is_router_mode() || access_point_mode()) && // client mode dost not support HW radio
	    button_pressed(BTN_WPS))
#endif
	{
		TRACE_PT("button WIFI_TOG pressed\n");
		if (btn_pressed_toggle_radio == 0) {
#if defined(RTCONFIG_RGBLED) && defined(GTAC2900)
			aura_led_control(AURA_LED_BTN);
			usleep(1800*1000);
			start_aurargb();
#endif
			radio_switch(0);
			btn_pressed_toggle_radio = 1;
			return;
		}
	}
	else{
		btn_pressed_toggle_radio = 0;
	}

#if defined(RTCONFIG_WPS_ALLLED_BTN)
	//use wps button to control all led on/off
	if (nvram_match("btn_ez_mode", "1"))
	{
		LED_status_old = LED_status;
		LED_status = button_pressed(BTN_WPS);
#if defined(DSL_AX82U)
		if (LED_status_old  > 0 && !LED_status && !LED_status_changed) //press, release, not on/off
		{
			//_dprintf("\n=====\nLED_status_old: %d, LED_status: %d, LED_status_changed: %d\n=====\n", LED_status_old, LED_status, LED_status_changed);
			ledg_scheme_switch();
		}
#endif
		if (LED_status) {
			TRACE_PT("button LED pressed\n");
			++BTN_pressed_count;
			alarmtimer(0, URGENT_PERIOD);
		}
		else{
			BTN_pressed_count = 0;
			LED_status_changed = 0;
			alarmtimer(NORMAL_PERIOD, 0);
		}

		if (BTN_pressed_count > WPS_LED_WAIT_COUNT && LED_status_changed == 0) {
			LED_status_changed = 1;
			LED_status_on = nvram_get_int("AllLED");
			LED_status_on = !LED_status_on;
			if (LED_status_on) {
				TRACE_PT("LED turn to normal\n");
				nvram_set_int("AllLED", 1);
				setAllLedNormal();
			}
			else {
				TRACE_PT("LED turn off\n");
				nvram_set_int("AllLED", 0);
				setAllLedOff();
			}

#ifdef RTCONFIG_SW_CTRL_ALLLED
			if (LED_status_on)
				nvram_set_int("led_val", nvram_get_int("AllLED_brightness"));
			else
				nvram_set_int("led_val", nvram_get_int("AllLED"));
			nvram_commit();

			char msg[256], config[128];

			if (nvram_get_int("re_mode") == 1) {
				snprintf(config, sizeof(config), "{\"led_val\":\"%s\"}", nvram_safe_get("led_val"));
				snprintf(msg, sizeof(msg), RC_CONFIG_CHANGED_MSG, EID_RC_CONFIG_CHANGED, config);
				(void)send_cfgmnt_event(msg);
			}
#endif

			return;
		}
	}
#endif
#endif	/* RTCONFIG_WPS_RST_BTN */

	handle_turbo_button();
	handle_led_onoff_button();

#if (((defined(RTCONFIG_LED_BTN) || !defined(RTCONFIG_WIFI_TOG_BTN)) && !defined(RTCONFIG_QCA)) && !defined(RTAX82U) && !defined(DSL_AX82U) && !defined(GSAX3000) && !defined(GSAX5400) && !defined(TUFAX5400)) && !defined(GTAX6000) && !defined(GT10) && !defined(RTAX82U_V2) && !defined(TUFAX5400_V2)
	LED_status_old = LED_status;
#if !defined(RTCONFIG_LED_BTN) && !defined(RTCONFIG_WIFI_TOG_BTN)
	LED_status = nvram_match("btn_ez_radiotoggle", "0") && nvram_match("btn_ez_mode", "1") &&
		     button_pressed(BTN_WPS);
#else
	LED_status = button_pressed(BTN_LED);
#endif

#if (defined(RTAC68U) || defined(RTAC3200) || defined(RTCONFIG_BCM_7114) || defined(HND_ROUTER)) && !defined(RTCONFIG_WPS_ALLLED_BTN)
#if defined(RTAC68U)
	if (is_ac66u_v2_series())
		;
	else if (nvram_match("cpurev", "c0") && nvram_get_int("PA") != 5023 && nvram_get_int("PA") != 0) {
		if (!nvram_get_int("AllLED") && LED_status_first)
		{
			LED_status_first = 0;
			LED_status_changed = 1;
			LED_status_on = 0;
		}
		else if (!LED_status &&
		    (LED_status != LED_status_old))
		{
			LED_status_changed = 1;
			if (LED_status_first)
			{
				LED_status_first = 0;
				LED_status_on = 0;
			}
			else
				LED_status_on = 1 - LED_status_on;
		}
		else
			LED_status_changed = 0;
	} else {
		LED_status_changed = 0;
		if (LED_status != LED_status_old)
		{
			if (LED_status_first)
			{
				LED_status_first = 0;
				if (nvram_get_int("AllLED"))
					LED_status_on = LED_status;
				else
					LED_status_on = 1 - LED_status;

				if (!nvram_get_int("AllLED"))
					LED_status = 1 - LED_status;
			}
			else
				LED_status_changed = 1;
		}
	}
#elif defined(RTAC3200) || defined(RTCONFIG_BCM_7114) || defined(HND_ROUTER)
	if (!nvram_get_int("AllLED") && LED_status_first)
	{
		LED_status_first = 0;
		LED_status_changed = 1;
		LED_status_on = 0;
	}
	else if (LED_status &&
	    (LED_status != LED_status_old))
	{
		LED_status_changed = 1;
		if (LED_status_first)
		{
			LED_status_first = 0;
			LED_status_on = 0;
		}
		else
			LED_status_on = 1 - LED_status_on;
	}
	else
		LED_status_changed = 0;
#endif

	if (LED_status_changed)
	{
		TRACE_PT("button BTN_LED pressed\n");
#if defined(RTAC68U)
		if (((!nvram_match("cpurev", "c0") || nvram_get_int("PA") == 5023 || (nvram_match("cpurev", "c0") && !nvram_get_int("PA"))) && LED_status == LED_status_on) ||
		      (nvram_match("cpurev", "c0") && nvram_get_int("PA") != 5023 && nvram_get_int("PA") != 0 && LED_status_on))
			nvram_set_int("AllLED", 1);
		else
			nvram_set_int("AllLED", 0);
		nvram_commit();
#elif defined(RTAC3200) || defined(RTCONFIG_BCM_7114) || defined(HND_ROUTER)
		if (LED_status && (LED_status != LED_status_old)) {
			if (LED_status_on)
				nvram_set_int("AllLED", 1);
			else
				nvram_set_int("AllLED", 0);
			nvram_commit();
		}
#endif
#if defined(RTCONFIG_RGBLED)
		start_aurargb();
#endif
#if defined(RTAC68U)
		if (((!nvram_match("cpurev", "c0") || nvram_get_int("PA") == 5023 || (nvram_match("cpurev", "c0") && !nvram_get_int("PA"))) && LED_status == LED_status_on) ||
		      (nvram_match("cpurev", "c0") && nvram_get_int("PA") != 5023 && nvram_get_int("PA") != 0 && LED_status_on))
#elif defined(RTAC3200) || defined(RTCONFIG_BCM_7114) || defined(HND_ROUTER)
		if (LED_status_on)
#endif
		{
#if defined(RTAX86U_PRO)
			setAllLedNormal();
#else
			led_control(LED_POWER, LED_ON);

#if defined(RTAC3200) || defined(RTCONFIG_BCM_7114) || defined(HND_ROUTER)
#ifdef HND_ROUTER
#ifndef GTAC2900
#if defined(RTAX58U_V2) || defined(GTAX6000) || defined(RTAX3000N) || defined(BR63) || defined(RTAX82U_V2) || defined(TUFAX5400_V2) || defined(RTAX5400) || defined(RTAX88U_PRO)
			wan_phy_led_pinmux(0);
#else
			led_control(LED_WAN_NORMAL, LED_ON);
#endif
#endif
#else
			kill_pidfile_s("/var/run/wanduck.pid", SIGUSR2);
#endif
#if defined(RTAX58U_V2) || defined(RTAX3000N) || defined(BR63)
			system("rtkswitch 43");
#endif
#if defined(HND_ROUTER)
#if defined(TUFAX3000_V2) || defined(RTAXE7800) || defined(EBG19_504)
			bcm53134_led_control(2);
#endif
#if defined(TUFAX3000_V2) || defined(RTAXE7800) || defined(TUFAX5400_V2) || defined(RTAX5400) || defined(RTAX88U_PRO)
#if defined(TUFAX3000_V2) || defined(RTAXE7800)
			if (!nvram_get_int("wans_extwan"))
#endif
			lan_phy_led_pinmux(0);
#endif
#if !defined(RTCONFIG_FAKE_ETLAN_LED) || defined(RTAX86U) || defined(RTAX88U) || defined(GTAC5300)
			setLANLedOn();
#endif
#ifdef RTCONFIG_FAKE_ETLAN_LED
			nvram_set_int("etlan_led_reset", 1);
#ifdef RTAX9000
			nvram_set_int("etwan_led_reset", 1);
#endif
#endif
#endif
#else
#ifdef RTAC68U
			if (is_ac66u_v2_series() || is_ac68u_v3_series())
				kill_pidfile_s("/var/run/wanduck.pid", SIGUSR2);
			else
#endif
			eval("et", "robowr", "0", "0x18", "0x01ff");
			eval("et", "robowr", "0", "0x1a", "0x01ff");
#endif

			if (wlonunit == -1 || wlonunit == 0) {
#ifdef RTAC68U
				eval("wl", "ledbh", "10", "7");
#elif defined(RTAC3200)
				eval("wl", "-i", "eth2", "ledbh", "10", "7");
#elif defined(GTAC5300) || defined(GTAXE11000)
				eval("wl", "-i", "eth6", "ledbh", "9", "7");
#elif defined(RTAX88U) || defined(GTAX11000)
				eval("wl", "-i", "eth6", "ledbh", "15", "7");
#elif defined(GTAX6000)
				eval("wl", "-i", "eth6", "ledbh", "13", "7");
#elif defined(GTAX11000_PRO)
				eval("wl", "-i", "eth6", "ledbh", "13", "7");
#elif defined(GTAXE16000)
				eval("wl", "-i", "eth7", "ledbh", "13", "7");
#elif defined(RTAX92U)
				eval("wl", "-i", "eth5", "ledbh", "10", "7");
#elif defined(RTAX95Q) || defined(XT8PRO) || defined(BM68) || defined(XT8_V2) || defined(RTAXE95Q) || defined(ET8PRO) || defined(ET8_V2)
				eval("wl", "-i", "eth4", "ledbh", "10", "7");
#elif defined(RTAX56_XD4) || defined(XD4PRO) || defined(CTAX56_XD4)
				eval("wl", "-i", "wl0", "ledbh", "10", "7");
#elif defined(RTAX55) || defined(RTAX1800) || defined(RTAX58U_V2) || defined(RTAX3000N) || defined(BR63)
				eval("wl", "-i", "eth2", "ledbh", "0", "25");
#elif defined(TUFAX3000_V2) || defined(RTAXE7800)
				eval("wl", "-i", "eth5", "ledbh", "0", "25");
#elif defined(RTAX9000)
				eval("wl", "-i", "eth8", "ledbh", "0", "25");
#elif defined(RTAX82_XD6S)
				eval("wl", "-i", "eth2", "ledbh", "0", "25");
#elif defined(BCM6750)
#if defined(RTAX82U) && !defined(RTCONFIG_BCM_MFG)
				if (!nvram_get_int("LED_order"))
					eval("wl", "-i", "eth5", "ledbh", "0", "1");
				else
#endif
				eval("wl", "-i", "eth5", "ledbh", "0", "25");
#elif defined(RTAX86U)
				if(!strcmp(get_productid(), "RT-AX86S"))
					eval("wl", "-i", "eth5", "ledbh", "7", "7");
				else
					eval("wl", "-i", "eth6", "ledbh", "7", "7");
#elif defined(RTAX68U)
				eval("wl", "-i", "eth5", "ledbh", "7", "7");
#elif defined(RTAC68U_V4)
				eval("wl", "-i", "eth5", "ledbh", "10", "7");
#elif defined(RTAX56U)
				eval("wl", "-i", "eth5", "ledbh", "0", "25");
//#elif defined(RPAX56) 
//				eval("wl", "-i", "eth1", "ledbh", "0", "25");
#elif defined(RTCONFIG_BCM_7114) || defined(RTAC86U)
				eval("wl", "ledbh", "9", "7");
#elif defined(GTAC2900)
				eval("wl", "ledbh", "9", "1");
#endif
			}
			if (wlonunit == -1 || wlonunit == 1) {
#ifdef RTAC68U
				eval("wl", "-i", "eth2", "ledbh", "10", "7");
#elif defined(RTAC3200)
				eval("wl", "ledbh", "10", "7");
#elif defined(GTAC5300) || defined(GTAXE11000)
				eval("wl", "-i", "eth7", "ledbh", "9", "7");
#elif defined(RTAX88U) || defined(GTAX11000)
				eval("wl", "-i", "eth7", "ledbh", "15", "7");
#elif defined(RTAX92U)
				eval("wl", "-i", "eth6", "ledbh", "10", "7");
#elif defined(RTAX95Q) || defined(XT8PRO) || defined(BM68) || defined(XT8_V2) || defined(RTAXE95Q) || defined(ET8PRO) || defined(ET8_V2)
				eval("wl", "-i", "eth5", "ledbh", "10", "7");
#elif defined(RTAX56_XD4) || defined(XD4PRO) || defined(CTAX56_XD4)
				eval("wl", "-i", "wl1", "ledbh", "10", "7");
#elif defined(RTAX55) || defined(RTAX1800) || defined(RTAX58U_V2) || defined(RTAX3000N) || defined(BR63)
				eval("wl", "-i", "eth3", "ledbh", "0", "25");
#elif defined(TUFAX3000_V2)
				eval("wl", "-i", "eth6", "ledbh", "0", "25");
#elif defined(RTAXE7800)
				eval("wl", "-i", "eth7", "ledbh", "15", "7");
#elif defined(RTAX9000)
				eval("wl", "-i", "eth6", "ledbh", "13", "7");
#elif defined(GTAX6000)
				eval("wl", "-i", "eth7", "ledbh", "13", "7");
#elif defined(GTAX11000_PRO)
				eval("wl", "-i", "eth7", "ledbh", "13", "7");
#elif defined(GTAXE16000)
				eval("wl", "-i", "eth8", "ledbh", "13", "7");
#elif defined(RTAX82_XD6S)
				eval("wl", "-i", "eth3", "ledbh", "15", "7");
#elif defined(BCM6750)
#if defined(RTAX82U) && !defined(RTCONFIG_BCM_MFG)
				if (!nvram_get_int("LED_order")) {
					led_control(LED_5G, LED_ON);
					kill_pidfile_s("/var/run/ledbtn.pid", SIGUSR1);
				} else
#endif
				eval("wl", "-i", "eth6", "ledbh", "15", "7");
#elif defined(RTAX86U)
				if(!strcmp(get_productid(), "RT-AX86S"))
					eval("wl", "-i", "eth6", "ledbh", "15", "7");
				else
					eval("wl", "-i", "eth7", "ledbh", "15", "7");
#elif defined(RTAX68U)
				eval("wl", "-i", "eth6", "ledbh", "7", "7");
#elif defined(RTAC68U_V4)
				eval("wl", "-i", "eth6", "ledbh", "10", "7");
#elif defined(RTAX56U)
				eval("wl", "-i", "eth6", "ledbh", "0", "25");
//#elif defined(RPAX56)
//				eval("wl", "-i", "eth2", "ledbh", "0", "25");
#elif defined(RTAC86U)
				eval("wl", "-i", "eth6", "ledbh", "9", "7");
#elif defined(GTAC2900)
				eval("wl", "-i", "eth6", "ledbh", "9", "1");
#elif defined(RTCONFIG_BCM_7114)
				eval("wl", "-i", "eth2", "ledbh", "9", "7");
#endif
			}
#ifdef RTCONFIG_HAS_5G_2
			if (wlonunit == -1 || wlonunit == 2) {
#if defined(RTAC3200)
				eval("wl", "-i", "eth3", "ledbh", "10", "7");
#elif defined(GTAC5300) || defined(GTAXE11000)
				eval("wl", "-i", "eth8", "ledbh", "9", "7");
#elif defined(GTAX11000)
				eval("wl", "-i", "eth8", "ledbh", "15", "7");
#elif defined(RTAX92U)
				eval("wl", "-i", "eth7", "ledbh", "15", "7");
#elif defined(RTAX95Q) || defined(XT8PRO) || defined(BM68) || defined(XT8_V2) || defined(RTAXE95Q) || defined(ET8PRO) || defined(ET8_V2)
				eval("wl", "-i", "eth6", "ledbh", "15", "7");
#elif defined(RTAC5300)
				eval("wl", "-i", "eth3", "ledbh", "9", "7");
#elif defined(GTAX11000_PRO)
				eval("wl", "-i", "eth8", "ledbh", "13", "7");
#elif defined(GTAXE16000)
				eval("wl", "-i", "eth9", "ledbh", "13", "7");
#elif defined(RTAXE7800)
				eval("wl", "-i", "eth6", "ledbh", "0", "25");
#elif defined(RTAX9000)
				eval("wl", "-i", "eth7", "ledbh", "13", "7");
#endif
			}
#endif
#ifdef RTCONFIG_EXTPHY_BCM84880
#if defined(RTAX86U) || defined(GTAX11000) || defined(GTAX6000) || defined(TUFAX3000_V2) || defined(RTAXE7800) || defined(RTAX9000)
			int ext_phy_model = nvram_get_int("ext_phy_model");

			if(!strcmp(get_productid(), "RT-AX86S")) ;
			else if(ext_phy_model == EXT_PHY_GPY211)
				eval("ethctl", "phy", "ext", EXTPHY_GPY_ADDR_STR, "0x1e0001", "0x3f0");
			else if(ext_phy_model == EXT_PHY_RTL8226)
				eval("ethctl", "phy", "ext", EXTPHY_RTL_ADDR_STR, "0x1fd032", "0x0027");	// RTL LCR2 LED Control Reg
			else if(ext_phy_model == EXT_PHY_BCM54991){
				eval("ethctl", "phy", "ext", EXTPHY_ADDR_STR, "0x1a832", "0x6");	// default. CTL LED3 MASK LOW
				eval("ethctl", "phy", "ext", EXTPHY_ADDR_STR, "0x1a835", "0x40");	// default. CTL LED4 MASK LOW
			}
#endif
#endif
#ifdef RTCONFIG_LOGO_LED
			led_control(LED_LOGO, LED_ON);
#endif
			kill_pidfile_s("/var/run/usbled.pid", SIGTSTP); // inform usbled to reset status
#if defined(RTAX82U) || defined(GSAX3000) || defined(GSAX5400) || defined(TUFAX5400) || defined(GTAX11000_PRO) || defined(GTAXE16000) || defined(GTAX6000) || defined(GT10) || defined(RTAX82U_V2) || defined(TUFAX5400_V2)
			kill_pidfile_s("/var/run/ledg.pid", SIGTSTP);
#endif
#ifdef GTAX6000
			AntennaGroupReset(LED_ON);
			setAntennaGroupOn();
			eval("sw", "0xff803140", "0x0000e002", "0x00c34a24", "0x00000a24", "0x00800080");
			eval("sw", "0xff80301c", "0x00040000");
			eval("sw", "0xff803150", "0x0000e002", "0x00c34a24", "0x00000a24", "0x00800080");
			eval("sw", "0xff80301c", "0x00080000");
			eval("sw", "0xff803160", "0x0000e002", "0x00c34a24", "0x00000a24", "0x00800080");
			eval("sw", "0xff80301c", "0x00100000");
			eval("sw", "0xff803170", "0x0000e002", "0x00c34a24", "0x00000a24", "0x00800080");
			eval("sw", "0xff80301c", "0x00200000");
#endif
#if defined(XT12) || defined(ET12)
			bcm_cled_ctrl_single_white(BCM_CLED_WHITE, BCM_CLED_STEADY_NOBLINK);
#endif
#endif // RTAX86U_PRO
		}
		else
			setAllLedOff();
	}
#elif defined(RTAC87U)
	if (LED_status)
		++BTN_pressed_count;
	else{
		BTN_pressed_count = 0;
		LED_status_changed = 0;
	}

	if (BTN_pressed_count >= LED_switch_count && LED_status_changed == 0) {
		LED_status_changed = 1;
		LED_status_on = nvram_get_int("AllLED");

		if (LED_status_on)
			nvram_set_int("AllLED", 0);
		else
			nvram_set_int("AllLED", 1);
		LED_status_on = !LED_status_on;

		if (LED_status_on) {
			led_control(LED_POWER, LED_ON);
			eval("et", "robowr", "0", "0x18", "0x01ff");
			eval("et", "robowr", "0", "0x1a", "0x01ff");
			qcsapi_wifi_run_script("router_command.sh", "lan4_led_ctrl on");

			if (nvram_match("wl0_radio", "1"))
				eval("wl", "ledbh", "10", "7");
			if (nvram_match("wl1_radio", "1")) {
				qcsapi_wifi_run_script("router_command.sh", "wifi_led_on");
				qcsapi_led_set(1, 1);
			}

#ifdef RTCONFIG_EXT_LED_WPS
			led_control(LED_WPS, LED_OFF);
#else
			led_control(LED_WPS, LED_ON);
#endif

			kill_pidfile_s("/var/run/usbled.pid", SIGTSTP); // inform usbled to reset status
		}
		else
			setAllLedOff();

		/* check LED_WAN status */
		kill_pidfile_s("/var/run/wanduck.pid", SIGUSR2);
	}
#endif
#endif	/* RTCONFIG_LED_BTN */

#if defined(RTCONFIG_BCMWL6) && defined(RTCONFIG_PROXYSTA)
	if ((psta_exist() || psr_exist())
		&& !dpsr_mode()
#ifdef RTCONFIG_DPSTA
		&& !dpsta_mode()
#endif
		&& !rp_mode()
	)
		return;
#endif

	if (btn_pressed_setup < BTNSETUP_START)
	{
#ifdef RTCONFIG_WPS_RST_BTN
		if (btn_pressed_setup == BTNSETUP_DETECT)
#else
		if (button_pressed(BTN_WPS) &&
		    !no_need_to_start_wps() &&
		    !wps_band_radio_off(get_radio_band(0)) &&
		    !wps_band_ssid_broadcast_off(get_radio_band(0)) &&
#ifndef RTCONFIG_WIFI_TOG_BTN
		    nvram_match("btn_ez_radiotoggle", "0") &&
#ifndef RTCONFIG_QCA
		    nvram_match("btn_ez_mode", "0") &&
#endif
#endif
#ifdef RTCONFIG_WPS_ALLLED_BTN
		    nvram_match("btn_ez_mode", "0") &&
#endif
		    !nvram_match("wps_ign_btn", "1"))
#endif	/* ! RTCONFIG_WPS_RST_BTN */
		{
			int wps_enable = nvram_match("wps_enable", "1");
#if defined(RTCONFIG_QCA_PLC2)
			nvram_unset("plc_pb_state");
			if (1)
#else
			if (wps_enable)
#endif
			{
				TRACE_PT("button WPS pressed\n");
#ifdef BLUECAVE
				bc_wps_led = 0;
#endif
				if (btn_pressed_setup == BTNSETUP_NONE)
				{
					btn_pressed_setup = BTNSETUP_DETECT;
					btn_count_setup = 0;
					alarmtimer(0, RUSHURGENT_PERIOD);
				}
				else
				{	/* Whenever it is pushed steady */
					if (++btn_count_setup > WPS_WAIT_COUNT)
					{
						btn_pressed_setup = BTNSETUP_START;
						btn_count_setup = 0;
						btn_count_setup_second = 0;
					    if (wps_enable) {
						nvram_set("wps_ign_btn", "1");
#if defined(RPAX56) || defined(RPAX58)
						nvram_set("btn_wps", "1");
#endif
#ifdef RTCONFIG_WIFI_CLONE
#if defined(RTCONFIG_WIFI_SON) && defined(RTCONFIG_AMAS)
						if (nvram_match("x_Setting", "0"))
							nvram_set("wifison_ready", "1");
						/* WiFi-SON */
						if (nvram_match("wifison_ready", "1")) {
							/* RE or default */
							if ((sw_mode() == SW_MODE_AP && !nvram_match("cfg_master", "1")) ||
							    nvram_match("x_Setting", "0")) {
								doSystem("killall wifimon_check");
								kill_wifi_wpa_supplicant(1);

								if(nvram_get_int("dfs_check_period"))
									doSystem("iwpriv wifi1 staDFSEn 1");

								doSystem("ifconfig %s up",get_staifname(1));

								nvram_set("wps_enrollee", "1");
								nvram_set("wps_e_success", "0");
							}
							/* CAP */
							else
								nvram_set("wps_enrollee", "0");
						}
						/* AiMesh */
						else
							nvram_set("wps_enrollee", "0");
#elif defined(RTCONFIG_AMAS)
//TBD.
#else /* WiFi clone for PLC */
						if (sw_mode() == SW_MODE_ROUTER || sw_mode() == SW_MODE_AP) {
							nvram_set("wps_enrollee", "1");
							nvram_set("wps_e_success", "0");
						}
#endif
#endif // RTCONFIG_WIFI_CLONE
					    } // wps_enable

#if defined(RTCONFIG_LP5523)
						lp55xx_leds_proc(LP55XX_WPS_SYNC_LEDS, LP55XX_WPS_PARAM_SYNC);
#elif (defined(PLN12) || defined(PLAC56))
						set_wifiled(3);
#elif defined(RTCONFIG_FIXED_BRIGHTNESS_RGBLED)
						set_rgbled(RGBLED_WPS_EVENT);
#endif

#if 0
						start_wps_pbc(0);	// always 2.4G
#else
#ifdef RTCONFIG_REALTEK
						if (sw_mode() == SW_MODE_AP && nvram_get_int("wlc_psta") == 0) { // AP mode
							if (nvram_get_int("wps_band_x") != 0) { // always 2.4G
								nvram_set_int("wps_restart", 1);
								stop_wps();
								nvram_set_int("wps_band_x", 0);
								start_wps();
							}
						}
#endif
						kill_pidfile_s("/var/run/wpsaide.pid", SIGTSTP);
#endif
						wsc_timeout = WPS_TIMEOUT_COUNT;
#if defined(RTCONFIG_CONCURRENTREPEATER) && defined(RTCONFIG_MTK_REP)
						if (sw_mode() == SW_MODE_AP)
							nvram_set_int("led_status", LED_AP_WPS_START);
#endif
					}
				}
			} else {
				TRACE_PT("button WPS pressed, skip\n");
			}
		}
		else if (btn_pressed_setup == BTNSETUP_DETECT)
		{
			btn_pressed_setup = BTNSETUP_NONE;
#ifndef RTCONFIG_WPS_LED
#ifdef BLUECAVE
#else
			wps_led_control(LED_ON);
#endif
#endif
			btn_count_setup = 0;
			alarmtimer(NORMAL_PERIOD, 0);
		}
#ifdef RTCONFIG_WPS_LED
		else
		{
			if (nvram_match("wps_success", "1") && ++btn_count_setup_second > WPS_SUCCESS_COUNT) {
				btn_count_setup_second = 0;
				nvram_set("wps_success", "0");
				__wps_led_control(LED_OFF);
			}
		}
#endif
	}
	else
	{
		int wps_enable = nvram_match("wps_enable", "1");
		if (!nvram_match("wps_ign_btn", "1")) {
#ifndef RTCONFIG_WPS_RST_BTN
			if (wps_enable && button_pressed(BTN_WPS) &&
			    !no_need_to_start_wps() &&
			    !wps_band_radio_off(get_radio_band(0)) &&
			    !wps_band_ssid_broadcast_off(get_radio_band(0))
#ifndef RTCONFIG_WIFI_TOG_BTN
			    && nvram_match("btn_ez_radiotoggle", "0")
#ifndef RTCONFIG_QCA
			    && nvram_match("btn_ez_mode", "0")
#endif
#endif
			) {
				/* Whenever it is pushed steady, again... */
				if (++btn_count_setup_second > WPS_WAIT_COUNT)
				{
#ifdef RTCONFIG_HND_ROUTER_AX
					if (!nvram_get_int("w_Setting") && !is_wps_stopped()) {
						dbg("skip WPS PBC re-triggering\n");
						return;
					}
#endif
					btn_pressed_setup = BTNSETUP_START;
					btn_count_setup_second = 0;
					nvram_set("wps_ign_btn", "1");
#if 0
					start_wps_pbc(0);	// always 2.4G
#else
#ifdef RTCONFIG_REALTEK
					if (sw_mode() == SW_MODE_AP && nvram_get_int("wlc_psta") == 0) { // AP mode
						if (nvram_get_int("wps_band_x") != 0) { // always 2.4G
							nvram_set_int("wps_restart", 1);
							stop_wps();
							nvram_set_int("wps_band_x", 0);
							start_wps();
						}
					}
#endif
					kill_pidfile_s("/var/run/wpsaide.pid", SIGTSTP);
#endif
					wsc_timeout = WPS_TIMEOUT_COUNT;
				}
			}
#endif	/* ! RTCONFIG_WPS_RST_BTN */

#if defined(RTCONFIG_QCA_PLC2)
			char *plc_pb_state = nvram_get("plc_pb_state");
			int pb_state;
			if (plc_pb_state != NULL)
				pb_state = atoi(plc_pb_state);
			else
				pb_state = -1;

			if (((wps_enable || pb_state == -2 /*OB*/) && is_wps_stopped()) || --wsc_timeout == 0 || IS_PLC_JOIN_STOPPED(pb_state))
#else
			if (is_wps_stopped() || --wsc_timeout == 0)
#endif
			{
				wsc_timeout = 0;

				btn_pressed_setup = BTNSETUP_NONE;
				btn_count_setup = 0;
				btn_count_setup_second = 0;
#ifdef BLUECAVE
				nvram_set("bc_ledbh", "wps_done");
#endif

#if defined(RTCONFIG_LP5523)
				lp55xx_leds_proc(LP55XX_ALL_LEDS_OFF, LP55XX_PREVIOUS_STATE);
#elif defined(RTCONFIG_FIXED_BRIGHTNESS_RGBLED)
				if (nvram_match("x_Setting", "0") && wsc_timeout == 0)
					set_rgbled(RGBLED_DEFAULT_STANDBY);
				else
					nvram_set("prelink_pap_status", "-1");
#else
				led_control_normal();
#endif // RTCONFIG_LP5523

				alarmtimer(NORMAL_PERIOD, 0);
#if defined(RTCONFIG_BCM_CLED) && defined(RTCONFIG_SINGLE_LED)
				bcm_cled_ctrl(BCM_CLED_WHITE, BCM_CLED_STEADY_NOBLINK);
				nvram_unset("bcm_cled_in_wps");
#endif
#if defined(RTCONFIG_CONCURRENTREPEATER)
				nvram_set_int("led_status", LED_WPS_FAIL);
#endif

#if (defined(RTCONFIG_QCA) && defined(RTCONFIG_WIFI_CLONE)) || \
    defined(RTCONFIG_WPSMULTIBAND)
#ifndef RTCONFIG_RALINK       
				stop_wps_method();
#endif				
#endif

#if defined(RTCONFIG_QCA_PLC2)
				if (!wps_enable)
					nvram_set("prelink_pap_status", "-1");	//PLC finish
			}
			if ((wps_enable || pb_state == -2 /*OB*/) && wsc_timeout == 0) //keep doing when stop with wps_enable
			{
#endif
#ifdef RTCONFIG_WIFI_CLONE
				if (nvram_match("wps_e_success", "1")) {
#if (defined(PLN12) || defined(PLAC56))
					set_wifiled(2);
#elif defined(RTCONFIG_WIFI_SON)
					if(nvram_match("wifison_ready", "1"))
					{
						nvram_unset("cfg_group"); //get it from CAP
                              			if (sw_mode() == SW_MODE_ROUTER) //default
                                        	{
							_dprintf("=> switch router to RE mode.\n");
							nvram_set("lan_proto", "dhcp");
							nvram_set("lan_dnsenable_x", "1");
							nvram_set("w_Setting", "1");
							nvram_set("x_Setting", "1");
							nvram_set_int("sw_mode", SW_MODE_AP);
							nvram_set("qis_Setting", "1");
							nvram_unset("cfg_master");
							nvram_commit();
							_eval(argv, NULL, 0, &pid);
							if (pids("bluetoothd"))
								notify_rc_and_wait("stop_bluetooth_service");

							uptime_wait(22); //RE estimate time
						}
						else if (sw_mode() == SW_MODE_AP)
						{
							_dprintf("=> run RE process\n");
							nvram_set("qis_Setting", "1");
							nvram_commit();
							start_re(0);
						}
						else
							notify_rc("restart_wireless");
					}
					else
#endif
					notify_rc("restart_wireless");
				}
				else
				{
#if (defined(PLN12) || defined(PLAC56))
					set_wifiled(1);
#elif defined(RTCONFIG_WIFI_SON)
					if(((sw_mode() == SW_MODE_AP) && nvram_get_int("x_Setting")) && nvram_match("wifison_ready", "1")) //Range extender
					{
						pid_t pid;
						_dprintf("=>wifimon is back\n");
						char *wifimon[]={"wifimon_check",NULL};
						if (!pids(wifimon[0]))
						_eval(wifimon, NULL, 0, &pid);
					}
#if defined(RTCONFIG_AMAS)
					if (!nvram_match("x_Setting", "1")) {
						char *unset_wifison_argv[] = { "delay_exec", "10", "nvram", "set", "wifison_ready=0"};
						pid_t pid;
						_eval(unset_wifison_argv, NULL, 0, &pid);	/* unset when failed and have to delay */
					}
#endif	/* RTCONFIG_AMAS */
					_dprintf("[%s] wps_e_success value = %s\n", __FUNCTION__, nvram_safe_get("wps_e_success"));
#endif
				}
#endif // RTCONFIG_WIFI_CLONE

#if defined(RTCONFIG_WIFI_SON)
				if (sw_mode() != SW_MODE_REPEATER && nvram_match("wifison_ready", "1")) {
#ifdef RTCONFIG_WPS_ENROLLEE
					if (nvram_match("wps_enrollee", "0"))  //CAP
#endif
					uptime_wait(30); //CAP estimate time
#if defined(RTCONFIG_LP5523)
					lp55xx_leds_proc(LP55XX_ALL_LEDS_OFF, LP55XX_PREVIOUS_STATE);
#elif defined(RTCONFIG_FIXED_BRIGHTNESS_RGBLED)
					if (nvram_match("x_Setting", "0") && wsc_timeout == 0)
						set_rgbled(RGBLED_DEFAULT_STANDBY);
					else
						nvram_set("prelink_pap_status", "-1");
#endif
				}
#if defined(RTCONFIG_AMAS) && (defined(RTCONFIG_LP5523) || defined(RTCONFIG_FIXED_BRIGHTNESS_RGBLED))
				else {
					nvram_set("prelink_pap_status", "-1");
				}
#endif // AMAS
#elif defined(RTCONFIG_AMAS) && (defined(RTCONFIG_LP5523) || defined(RTCONFIG_FIXED_BRIGHTNESS_RGBLED))
				nvram_set("prelink_pap_status", "-1");
#endif
#ifdef RTCONFIG_CFGSYNC
				send_event_to_cfgmnt(EID_RC_WPS_STOP);
#endif
				return;
			}
		}

		++btn_count_setup;
		btn_count_setup = (btn_count_setup % 20);

		/* 0123456789 */
		/* 1010101010 */
#if defined(RTCONFIG_LP5523)
#elif defined(RTCONFIG_FIXED_BRIGHTNESS_RGBLED)
#elif defined(BLUECAVE)
		if (!bc_wps_led) {
			bc_wps_led = 1;
			nvram_set("bc_ledbh", "wps");
			kill_pidfile_s("/var/run/sw_devled.pid", SIGUSR1);
		}
#else
		if ((btn_count_setup % 2) == 0 && (btn_count_setup > 10))
		{
			wps_led_control(LED_ON);
#if defined(RTCONFIG_RGBLED) && defined(GTAC2900)
			aura_led_control(AURA_LED_WPS);
#endif
#if defined(RTCONFIG_BCM_CLED) && defined(RTCONFIG_SINGLE_LED)
			nvram_set("bcm_cled_in_wps", "1");
			bcm_cled_ctrl(BCM_CLED_BLUE, BCM_CLED_STEADY_BLINK);
#endif
		}
		else
		{
			wps_led_control(LED_OFF);
#if defined(RTCONFIG_RGBLED) && defined(GTAC2900)
			aura_led_control(AURA_LED_OFF);
#endif
		}
#endif
#if defined(RTCONFIG_CONCURRENTREPEATER) && defined(RTCONFIG_MTK_REP)
		if (sw_mode() == SW_MODE_AP)
			nvram_set_int("led_status", LED_AP_WPS_START);
#endif
	}
#endif	/* BTN_SETUP */
#if defined(RTCONFIG_BCM_CLED) && defined(RTCONFIG_SINGLE_LED)
	if(nvram_get_int("bcm_cled_in_reset") == 1){
		nvram_unset("bcm_cled_in_reset");
	}
#endif
}

#if defined(RTCONFIG_AMAS_WGN) && defined(RTCONFIG_QCA)
int clear_wgn_wloff_vifs(char *ifname)
{
  	char *nv = NULL, *nvp = NULL, *b = NULL;
	char new_nv[256],gn[10];

	if(nvram_match("re_mode", "1"))
		return -1;
	
        nv = nvp = strdup(nvram_safe_get("wgn_wloff_vifs"));
	memset(new_nv,0,sizeof(new_nv));
	if(nv && strstr(nv,ifname)!=NULL)
	{	
                while ((b = strsep(&nvp, " ")) != NULL)
                {
			if(b && strcmp(b,ifname)!=0)
			{
				snprintf(gn,sizeof(gn),"%s ",b);
                                strcat(new_nv,gn);
			}	
                }
                free(nv);

		nvram_set("wgn_wloff_vifs",new_nv);
	}
	return 0;
}	
#endif				

#define DAYSTART (0)
#define DAYEND (60*60*23 + 60*59 + 60) // 86400
static int in_sched(int now_mins, int now_dow, int (*enableTime)[24])
{
	int currentTime;
	int x=0,y=0;
	int tableTimeStart, tableTimeEnd;
	// 8*60*60 = AM0.00~AM8.00
	currentTime = (now_mins*60) + now_dow*DAYEND;

	for(;x<7;x++)
	{
		y=0;
		for(;y<24;y++)
		{
			if (enableTime[x][y] == 1)
			{
				tableTimeStart = x*DAYEND + y*60*60;
				tableTimeEnd = x*DAYEND + (y+1)*60*60;

				if (y == 23)
				tableTimeEnd = tableTimeEnd - 1; //sunday = 0 ~ 86399
				WL_SCHED_DBG("tableTimeStart=%d, tableTimeEnd=%d, currentTime=%d\n",tableTimeStart,tableTimeEnd,currentTime);

				if (tableTimeStart<=currentTime && currentTime < tableTimeEnd)
				{
					return 1;
				}
			}
		}
	}
	return 0;
}

int timecheck_item(char *activeTime)
{
	int current, active;
	int now_dow;
	time_t now;
	struct tm *tm;
	char Date[] = "XX";
	char startTime[] = "XX";
	char endTime[] = "XX";
	int tableAllOn = 0;	// 0: wifi time all off 1: wifi time all on 2: check&calculate wifi open slot
	int schedTable[7][24];
	int x=0, y=0, z=0;	//for loop usage

	/* current router time */
#ifndef RTCONFIG_AVOID_TZ_ENV
	setenv("TZ", nvram_safe_get("time_zone_x"), 1);
#endif
	time(&now);
	tm = localtime(&now);
	now_dow = tm->tm_wday;
	current = tm->tm_hour * 60 + tm->tm_min; //minutes

	/* initial time table */
	x=0;
	for(;x<7;x++)
	{
		y=0;
		for(;y<24;y++)
		{
			schedTable[x][y]=0;
		}
	 }

	active = 0;
	if (activeTime[0] == '0' && activeTime[1] == '0' && activeTime[2] == '0' && activeTime[3] == '0'
		&& activeTime[4] == '0' && activeTime[5] == '0')
	{
		tableAllOn = 1; // all open
	}
	else if (!strcmp(activeTime, ""))
	{
		tableAllOn = 0; // all close
	}
	else
	{
		tableAllOn = 2;
		/* Counting variables quantity*/
		x=0;
		int schedCount = 1; // how many variables in activeTime	111014<222024<331214 count will be 3
		for(;x<strlen(activeTime);x++)
		{
			if (activeTime[x] == '<')
			schedCount++;
		}

		/* analyze for sched Date, startTime, endTime*/
		x=0;
		int loopCount=0;
		for(;x<schedCount;x++) // start for loop (schedCount)
		{
			do
			{
				if (loopCount < (2 + (7*x)))
				{
					Date[loopCount-7*x] = activeTime[loopCount];
				}
				else if (loopCount < (4 + (7*x)))
				{
					startTime[loopCount - (2+7*x)] = activeTime[loopCount];
				}
				else
				{
					endTime[loopCount - (4+7*x)] = activeTime[loopCount];
					if ((loopCount - (4+7*x)) == 1&& atoi(endTime) == 0)
					{
						endTime[0] = '2';
						endTime[1] = '4';

						if (Date[1] == '0')
							Date[1] = '6';
						else
							Date[1]=Date[1]-1;
					}
				}
				loopCount++;
			} while(activeTime[loopCount] != '<' && loopCount < strlen(activeTime));

			loopCount++;

			/*Check which time will enable or disable wifi radio*/
			int offSet=0;
			if (Date[0] == Date[1])
			{
				z=0;
				for(;z<(atoi(endTime) - atoi(startTime));z++)
					schedTable[Date[0]-'0'][atoi(startTime)+z] = 1;
			}
			else
			{
				z=0;
				for(;z<((atoi(endTime) - atoi(startTime))+24*(Date[1] - Date[0]));z++)
				{
					schedTable[Date[0]-'0'+offSet][(atoi(startTime)+z)%24] = 1;
					if ((atoi(startTime)+z)%24 == 23)
					{
						offSet++;
					}
				}
			}
		}//end for loop (schedCount)
	}

	if (tableAllOn == 0)
		active = 0;
	else if (tableAllOn == 1)
		active = 1;
	else
		active = in_sched(current, now_dow, schedTable);

	WL_SCHED_DBG("[wifi-scheduler] active=%d\n", active);
	return active;
}


int timecheck_reboot(char *activeSchedule)
{
	int active, current_time, current_date, Time2Active, Date2Active;
	time_t now;
	struct tm *tm;
	int i;

#ifndef RTCONFIG_AVOID_TZ_ENV
	setenv("TZ", nvram_safe_get("time_zone_x"), 1);
#endif

	time(&now);
	tm = localtime(&now);
	current_time = tm->tm_hour * 60 + tm->tm_min;
	current_date = 1 << (6-tm->tm_wday);
	active = 0;
	Time2Active = 0;
	Date2Active = 0;

	Time2Active = ((activeSchedule[7]-'0')*10 + (activeSchedule[8]-'0'))*60 + ((activeSchedule[9]-'0')*10 + (activeSchedule[10]-'0'));

	for(i=0;i<=6;i++) {
		Date2Active += (activeSchedule[i]-'0') << (6-i);
	}

	if ((current_time == Time2Active) && (Date2Active & current_date))	active = 1;

	//dbG("[watchdog] current_time=%d, ActiveTime=%d, current_date=%d, ActiveDate=%d, active=%d\n",
	//	current_time, Time2Active, current_date, Date2Active, active);

	return active;
}

int svcStatus[12] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

/* Check for time-reated service 	*/
/* 1. Wireless Radio			*/
/* 2. Guess SSID expire time 		*/
/* 3. Reboot Schedule			*/
//int timecheck(int argc, char *argv[])
void timecheck(void)
{
#if !defined(RTCONFIG_WL_SCHED_V2) || defined(RTCONFIG_LP5523)
	int activeNow;
	char schedTime[2048];
	char prefix[]="wlXXXXXX_";
	char tmp2[100];
	int unit, item;
#endif
	char word[256], *next, tmp[100];
	char lan_ifname[16];
	char wl_vifs[256], nv[40];
	int expire, need_commit = 0;
#if !defined(RTCONFIG_WL_SCHED_V2) && defined(RTCONFIG_AMAS) && defined(RTCONFIG_QCA)
       char sctmp[20];
#endif 

#ifdef RTCONFIG_AMAS_WGN
	char wloff_vifs[256], *p_vifs = NULL, cfgVer[9] = {0};
#endif	

#ifndef CONFIG_BCMWL5
#if defined(RTCONFIG_PROXYSTA)
	if (mediabridge_mode())
		return;
#endif
#endif

#ifndef RTCONFIG_WL_SCHED_V2
	WL_SCHED_DBG("[wifi-scheduler] start to timecheck()...\n");

	item = 0;
	unit = 0;

	if (nvram_match("svc_ready", "0") || nvram_match("wlready", "0"))
		goto end_of_wl_sched;

	if (nvram_match("reload_svc_radio", "1"))
	{
		nvram_set("reload_svc_radio", "0");

		foreach (word, nvram_safe_get("wl_ifnames"), next) {
			SKIP_ABSENT_BAND_AND_INC_UNIT(unit);
			svcStatus[item] = -1;
			item++;
			unit++;
		}

		item = 0;
		unit = 0;
	}

	if (nvram_get_int("ntp_ready") != 1)
		goto end_of_wl_sched;

	// radio on/off
	foreach (word, nvram_safe_get("wl_ifnames"), next) {
		SKIP_ABSENT_BAND_AND_INC_UNIT(unit);
		snprintf(prefix, sizeof(prefix), "wl%d_", unit);
#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_QCA)
		snprintf(sctmp, sizeof(sctmp), "wl%d_qca_sched", unit);
#endif
		//dbG("[watchdog] timecheck unit=%s radio=%s, timesched=%s\n", prefix, nvram_safe_get(strcat_r(prefix, "radio", tmp)), nvram_safe_get(strcat_r(prefix, "timesched", tmp2))); // radio toggle test
		if (nvram_match(strcat_r(prefix, "radio", tmp), "0") ||
			nvram_match(strcat_r(prefix, "timesched", tmp2), "0")) {
			item++;
			unit++;
			continue;
		}

		/*transfer wl_sched NULL value to 000000 value, because
		of old version firmware with wrong default value*/
		if (!strcmp(nvram_safe_get(strcat_r(prefix, "sched", tmp)), ""))
		{
			nvram_set(strcat_r(prefix, "sched", tmp),"000000");
			//nvram_set("wl_sched", "000000");
		}

		snprintf(schedTime, sizeof(schedTime), "%s", nvram_safe_get(strcat_r(prefix, "sched", tmp)));

		activeNow = timecheck_item(schedTime);

#if defined(RTCONFIG_LYRA_5G_SWAP)
		snprintf(tmp, sizeof(tmp), "%d", swap_5g_band(unit));
#else
		snprintf(tmp, sizeof(tmp), "%d", unit);
#endif
		WL_SCHED_DBG("[wifi-scheduler] unit=%d, activeNow=%d\n", unit, activeNow);
		
		if (svcStatus[item] != activeNow) {
#ifdef RTCONFIG_QCA
#if defined(RTCONFIG_AMAS)
		  	nvram_set_int(sctmp,activeNow);
#endif

#if defined(RTCONFIG_LYRA_5G_SWAP)
			if (match_radio_status(swap_5g_band(unit), activeNow)) {
#else
			if (match_radio_status(unit, activeNow)) {
#endif
				svcStatus[item] = activeNow;
				item++;
				unit++;
				continue;
			}
#else
			svcStatus[item] = activeNow;
#endif

			if (activeNow == 0) {
				eval("radio", "off", tmp);
				WL_SCHED_DBG("[wifi-scheduler] Turn radio [band_index=%s] off\n", tmp);
				logmessage("wifi scheduler", "Turn radio [band_index=%s] off.", tmp);
			} else {
				eval("radio", "on", tmp);
				WL_SCHED_DBG("[wifi-scheduler] Turn radio [band_index=%s] on\n", tmp);
				logmessage("wifi scheduler", "Turn radio [band_index=%s] on.", tmp);
			}
		}
		item++;
		unit++;

	}
end_of_wl_sched:
#endif // ifndef RTCONFIG_WL_SCHED_V2

	// guest ssid expire check
	if ((is_router_mode() || access_point_mode()) &&
		(strlen(nvram_safe_get("wl0_vifs")) || strlen(nvram_safe_get("wl1_vifs")) ||
		 strlen(nvram_safe_get("wl2_vifs")) || strlen(nvram_safe_get("wl3_vifs"))))
	{
		snprintf(lan_ifname, sizeof(lan_ifname), "%s", nvram_safe_get("lan_ifname"));
		sprintf(wl_vifs, "%s %s %s %s", nvram_safe_get("wl0_vifs"), nvram_safe_get("wl1_vifs"), nvram_safe_get("wl2_vifs"), nvram_safe_get("wl3_vifs"));

#ifdef RTCONFIG_AMAS_WGN
		memset(wloff_vifs, 0, sizeof(wloff_vifs));
		p_vifs = &wloff_vifs[0];
#endif		

		foreach (word, wl_vifs, next) {
			snprintf(nv, sizeof(nv) - 1, "%s_expire_tmp", wif_to_vif (word));
			expire = nvram_get_int(nv);

			if (expire)
			{
#if defined(RTCONFIG_AMAS_WGN) && defined(RTCONFIG_QCA)
				clear_wgn_wloff_vifs(wif_to_vif(word));
#endif				
				if (expire <= 30)
				{
					nvram_set(nv, "0");
					snprintf(nv, sizeof(nv) - 1, "%s_bss_enabled", wif_to_vif (word));
#if defined(RTCONFIG_NOTIFICATION_CENTER)
					char nt_gn_prefix[36] = {0};
					snprintf(nt_gn_prefix, sizeof(nt_gn_prefix), ".%d_bss_enabled", num_of_mssid_support(0));
					if(strstr(nv, nt_gn_prefix))
						RC_SEND_NT_EVENT(GENERAL_TOGGLE_STATES_UPDATE, "guestnetwork");
#endif
					nvram_set(nv, "0");
					if (!need_commit) need_commit = 1;
#ifdef CONFIG_BCMWL5
					eval("wl", "-i", word, "closed", "1");
					eval("wl", "-i", word, "bss_maxassoc", "1");
					eval("wl", "-i", word, "bss", "down");
#endif
					ifconfig(word, 0, NULL, NULL);
					eval("brctl", "delif", lan_ifname, word);
#ifdef RTCONFIG_WIFI_SON
					if (sw_mode() != SW_MODE_REPEATER && nvram_match("wifison_ready", "1")) {
						eval("brctl", "delif", BR_GUEST, word);
						char *tmp_str = strdup(nvram_safe_get("lan_ifnames"));
						if (tmp_str) {
							if (remove_word(tmp_str, word)) {
								trim_space(tmp_str);
								nvram_set("lan_ifnames", tmp_str);
							}
							free(tmp_str);
						}
						doSystem("wlanconfig %s destroy", word);
						/* restart hyd */
						eval("hive_hyd");
					}
#endif
#ifdef RTCONFIG_AMAS_WGN
					p_vifs += snprintf(p_vifs, sizeof(wloff_vifs) - (p_vifs - wloff_vifs), "%s ", wif_to_vif(word));
#endif						
				}
				else
				{
					expire -= 30;
					sprintf(tmp, "%d", expire);
					nvram_set(nv, tmp);
				}
			}
		}

		if (need_commit)
		{
			need_commit = 0;
			nvram_commit();
		}

#ifdef RTCONFIG_AMAS_WGN
		if (strlen(wloff_vifs) > 0)
		{
			if (wloff_vifs[strlen(wloff_vifs)-1] == ' ')
				wloff_vifs[strlen(wloff_vifs)-1] = '\0';
			nvram_set("wgn_wloff_vifs", wloff_vifs);
	        /* update cfg_ver info */
	        srand(time(NULL));
	        snprintf(cfgVer, sizeof(cfgVer), "%d%d", rand(), rand());
	        nvram_set("cfg_ver", cfgVer);
	        nvram_commit();
			kill_pidfile_s("/var/run/cfg_server.pid", SIGUSR2);			
		}		
#endif
	}

#ifdef RTCONFIG_REBOOT_SCHEDULE
	/* Reboot Schedule */
	char reboot_schedule[PATH_MAX];
	if (nvram_match("reboot_schedule_enable", "1"))
	{
		if (nvram_match("ntp_ready", "1"))
		{
			//SMTWTFSHHMM
			//XXXXXXXXXXX
			snprintf(reboot_schedule, sizeof(reboot_schedule), "%s", nvram_safe_get("reboot_schedule"));
			if (strlen(reboot_schedule) == 11)
			{
				if (timecheck_reboot(reboot_schedule))
				{
					char reboot[sizeof("255")];
					char upgrade[sizeof("255")];

					memset(reboot, 0, sizeof("255"));
					memset(upgrade, 0, sizeof("255"));
					f_read_string("/tmp/reboot", reboot, sizeof(reboot));
					f_read_string("/tmp/upgrade", upgrade, sizeof(upgrade));

					if (atoi(reboot) || atoi(upgrade))
						return;

					nvram_set("sys_reboot_reason", "rbt_scheduler");
					logmessage("reboot scheduler", "[%s] The system is going down for reboot\n", __FUNCTION__);
					kill(1, SIGTERM);
				}
			}
		}
		else
			logmessage("reboot scheduler", "[%s] NTP sync error\n", __FUNCTION__);
	}
#endif

#if defined(RTCONFIG_LP5523)
	// lp55xx led schedule
	int lp55xx_sch_enable = nvram_get_int("lp55xx_lp5523_sch_enable");
	if (lp55xx_sch_enable > 0 && nvram_match("x_Setting", "1")) {
		snprintf(schedTime, sizeof(schedTime), "%s", nvram_safe_get("lp55xx_lp5523_sch"));
		activeNow = timecheck_item(schedTime);

		if (activeNow == 1) {
			if (lp55xx_sch_enable==1) {
				nvram_set_int("lp55xx_lp5523_sch_enable", 2);
				lp55xx_leds_proc(LP55XX_ALL_LEDS_OFF, LP55XX_SCH_ENABLE);
			}
		}
		else {
			if (lp55xx_sch_enable!=1) {
				nvram_set_int("lp55xx_lp5523_sch_enable", 1);
				lp55xx_leds_proc(LP55XX_ALL_LEDS_OFF, LP55XX_PREVIOUS_STATE);
			}
		}
	}
#endif

	return;
}

#ifdef RTCONFIG_WL_SCHED_V2
void timecheck_v2(void)
{
	int activeNow;
	char schedTime[2048];
	char prefix[]="wlXXXXXX_", tmp[100], tmp2[100];
	char word[256], *next;
	int unit = 0, item = 0;
#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_QCA)
       char sctmp[20];
#endif 
	// Check whether conversion needed.
	convert_wl_sched_v1_to_sched_v2();

	WL_SCHED_DBG("[wifi-scheduler] start to timecheck()...\n");
	item = 0;
	unit = 0;

	if (nvram_match("reload_svc_radio", "1"))
	{
		nvram_set("reload_svc_radio", "0");

		foreach (word, nvram_safe_get("wl_ifnames"), next) {
			SKIP_ABSENT_BAND_AND_INC_UNIT(unit);
			svcStatus[item] = -1;
			item++;
			unit++;
		}

		item = 0;
		unit = 0;
	}

	if (nvram_get_int("ntp_ready") != 1)
		return;

	// radio on/off
	if (nvram_match("svc_ready", "1") && nvram_match("wlready", "1"))
	foreach (word, nvram_safe_get("wl_ifnames"), next) {
		SKIP_ABSENT_BAND_AND_INC_UNIT(unit);
		snprintf(prefix, sizeof(prefix), "wl%d_", unit);
#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_QCA)
		snprintf(sctmp, sizeof(sctmp), "wl%d_qca_sched", unit);
#endif
		//dbG("[watchdog] timecheck unit=%s radio=%s, timesched=%s\n", prefix, nvram_safe_get(strcat_r(prefix, "radio", tmp)), nvram_safe_get(strcat_r(prefix, "timesched", tmp2))); // radio toggle test
		if (nvram_match(strcat_r(prefix, "radio", tmp), "0") ||
			nvram_match(strcat_r(prefix, "timesched", tmp2), "0")) {
			item++;
			unit++;
			continue;
		}

		/*transfer wl_sched NULL value to "" value, because
		of old version firmware with wrong default value*/
		if (!nvram_get(strcat_r(prefix, "sched_v2", tmp)))
		{
			nvram_set(strcat_r(prefix, "sched_v2", tmp), "");
		}

		snprintf(schedTime, sizeof(schedTime), "%s", nvram_safe_get(strcat_r(prefix, "sched_v2", tmp)));

		activeNow = check_sched_v2_on_off(schedTime);

#if defined(RTCONFIG_LYRA_5G_SWAP)
		snprintf(tmp, sizeof(tmp), "%d", swap_5g_band(unit));
#else
		snprintf(tmp, sizeof(tmp), "%d", unit);
#endif
		WL_SCHED_DBG("[wifi-scheduler] 3 uschedTime=%s, unit=%d, activeNow=%d\n", schedTime, unit, activeNow);
		if (svcStatus[item] != activeNow) {
#ifdef RTCONFIG_QCA
#if defined(RTCONFIG_AMAS)
		  	nvram_set_int(sctmp,activeNow);
#endif
#if defined(RTCONFIG_LYRA_5G_SWAP)
			if (match_radio_status(swap_5g_band(unit), activeNow)) {
#else
			if (match_radio_status(unit, activeNow)) {
#endif
				svcStatus[item] = activeNow;
				item++;
				unit++;
				continue;
			}
#else
			svcStatus[item] = activeNow;
#endif

			if (activeNow == 0) {
				eval("radio", "off", tmp);
				WL_SCHED_DBG("[wifi-scheduler] Turn radio [band_index=%s] off\n", tmp);
				logmessage("wifi scheduler", "Turn radio [band_index=%s] off.", tmp);
			} else {
				eval("radio", "on", tmp);
				WL_SCHED_DBG("[wifi-scheduler] Turn radio [band_index=%s] on\n", tmp);
				logmessage("wifi scheduler", "Turn radio [band_index=%s] on.", tmp);
			}
		}
		item++;
		unit++;

	}
}
#endif

static void chld_reap_local(int sig)
{
	chld_reap(sig);
}

#ifdef RTCONFIG_RALINK
int need_restart_wsc = 0;
#endif

static void catch_sig(int sig)
{
#if !defined(RTCONFIG_AMAS) && (defined(RTCONFIG_ALPINE) || defined(RTCONFIG_LANTIQ))
	dbG("watchdog: skip catch_sig(), sig=[%d]\n", sig);
	return;
#endif

	if (sig == SIGUSR1)
	{
		dbG("[watchdog] Handle WPS LED for WPS Start\n");

#if defined(RTCONFIG_QCA_PLC2)
		nvram_set("plc_pb_state", "-2");	//set plc_pb_state=-2 as OnBoarding for wps check
#endif

#if defined(RTCONFIG_LP5523)
		lp55xx_leds_proc(LP55XX_WPS_SYNC_LEDS, LP55XX_WPS_PARAM_SYNC);
#elif defined(RTCONFIG_FIXED_BRIGHTNESS_RGBLED)
		set_rgbled(RGBLED_WPS_EVENT);
#endif
		alarmtimer(NORMAL_PERIOD, 0);

		btn_pressed_setup = BTNSETUP_START;
		btn_count_setup = 0;
		btn_count_setup_second = 0;
#ifdef RTCONFIG_AMAS
#if defined(RTCONFIG_WIFI_SON) && defined(RTCONFIG_AMAS)
		if(nvram_match("wifison_ready", "1"))
			wsc_timeout = WPS_TIMEOUT_COUNT;
		else
#endif

		if (nvram_get_int("amesh_led"))
		wsc_timeout = AMESH_TIMEOUT_COUNT;
		else
#endif
		wsc_timeout = WPS_TIMEOUT_COUNT;
		alarmtimer(0, RUSHURGENT_PERIOD);

#if (defined(PLN12) || defined(PLAC56))
		set_wifiled(3);
#endif
	}
	else if (sig == SIGUSR2)
	{
		dbG("[watchdog] Handle WPS LED for WPS Stop\n");

		if (nvram_match("wps_ign_btn", "1")) {
			dbG("[watchdog] ignore SIGUSR2 for wps_ign_btn is set\n");
			return;
		}

#if defined(RTCONFIG_QCA_PLC2)
		if (btn_pressed_setup == BTNSETUP_START)
			nvram_set("prelink_pap_status", "-1");	//WPS or PLC finish to update LED
#endif	/* RTCONFIG_QCA_PLC2 */
		btn_pressed_setup = BTNSETUP_NONE;
		btn_count_setup = 0;
		btn_count_setup_second = 0;
		wsc_timeout = 1;
		alarmtimer(NORMAL_PERIOD, 0);
#if (defined(PLN12) || defined(PLAC56))
		set_wifiled(1);
#else
		led_control_normal();
#endif
	}
	else if (sig == SIGTSTP)
	{
		dbG("[watchdog] Reset WPS timeout count\n");

		wsc_timeout = WPS_TIMEOUT_COUNT;
	}
#if defined(RTAC1200G) || defined(RTAC1200GP)
	else if (sig == SIGHUP)
	{
		dbG("[watchdog] Reset alarm timer...\n");
		alarmtimer(NORMAL_PERIOD, 0);
	}
#endif
#ifdef RTCONFIG_RALINK
	else if (sig == SIGTTIN)
	{		
		wsc_user_commit();
		need_restart_wsc = 1;	
#ifdef RTCONFIG_WPSMULTIBAND
		stop_wps_method();
#endif	
	}
#endif
}


/* simple traffic-led hints */
unsigned long get_devirq_count(char *irqs)
{
	FILE *f;
	char buf[256];
	char *irqname, *p;
	unsigned long counter1, counter2;

	if ((f = fopen("/proc/interrupts", "r")) == NULL) return -1;

	fgets(buf, sizeof(buf), f);

	counter1=counter2=0;

	while (fgets(buf, sizeof(buf), f)) {
		if ((p=strchr(buf, ':')) == NULL) continue;
		*p = 0;
		if ((irqname = strrchr(buf, ' ')) == NULL) irqname = buf;
		else ++irqname;

		if (strcmp(irqname, irqs)) continue;

#ifdef RTCONFIG_BCMSMP
		if (sscanf(p+1, "%lu%lu", &counter1, &counter2) != 2) continue;
#else
		if (sscanf(p+1, "%lu", &counter1) != 1) continue;
#endif

	}
	fclose(f);

	return counter1+counter2 > 2 ? counter1+counter2 : 0;
}

#if !defined(RTCONFIG_BLINK_LED)
void fake_dev_led(char *irqs, unsigned int LED_WHICH)
{
	static unsigned int blink_dev_check = 0;
	static unsigned int blink_dev = 0;
	static unsigned int data_dev = 0;
	unsigned long count_dev;
	int i;
	static int j;
	static int status = -1;
	static int status_old;

	if (!*irqs || !LED_WHICH)
		return;

	// check data per 10 count
	if ((blink_dev_check%10) == 0) {
		count_dev = get_devirq_count(irqs);
		if (count_dev && data_dev != count_dev) {
			blink_dev = 1;
			data_dev = count_dev;
		}
		else blink_dev = 0;
		led_control(LED_WHICH, LED_ON);
	}

	if (blink_dev) {
		j = rand_seed_by_time() % 3;
		for(i=0;i<10;i++) {
			usleep(33*1000);

			status_old = status;
			if (((i%2) == 0) && (i > (3 + 2*j)))
				status = 0;
			else
				status = 1;

			if (status != status_old)
			{
				if (status)
					led_control(LED_WHICH, LED_ON);
				else
					led_control(LED_WHICH, LED_OFF);
			}
		}
		led_control(LED_WHICH, LED_ON);
	}

	blink_dev_check++;
}
#endif

#ifdef RTCONFIG_FAKE_ETLAN_LED
unsigned long get_etlan_count()
{
	FILE *f;
	char buf[256];
	char *ifname, *p;
	unsigned long counter=0;
#if defined(GTAC5300) || defined(RTAX88U) || defined(GTAX11000) || defined(RTAX92U) || defined(RTAX95Q) || defined(XT8PRO) || defined(BM68) || defined(XT8_V2) || defined(RTAXE95Q) || defined(ET8PRO) || defined(ET8_V2) || defined(RTAX56U) || defined(RTAX56_XD4) || defined(XD4PRO) || defined(CTAX56_XD4) || defined(RTAX86U) || defined(RTAX68U) || defined(RTAX55) || defined(RTAX1800) || defined(GTAXE11000) || defined(GTAX11000_PRO) || defined(GTAXE16000) || defined(GTAX6000) || defined(RTAX3000N) || defined(RTAX88U_PRO) || defined(XC5) || defined(RTAX9000)
	unsigned long tmpcnt=0;
#endif

#if defined(RTAX95Q) || defined(XT8PRO) || defined(BM68) || defined(XT8_V2) || defined(RTAXE95Q) || defined(ET8PRO) || defined(ET8_V2) || defined(RTAX56_XD4) || defined(XD4PRO) || defined(CTAX56_XD4)
	return -1;
#endif
#if defined(RTAX86U)
	if(!strcmp(get_productid(), "RT-AX86S"))
		return -1;
#endif

	if ((f = fopen("/proc/net/dev", "r")) == NULL) return -1;

	fgets(buf, sizeof(buf), f);
	fgets(buf, sizeof(buf), f);

	while (fgets(buf, sizeof(buf), f)) {
		if ((p=strchr(buf, ':')) == NULL) continue;
		*p = 0;
		if ((ifname = strrchr(buf, ' ')) == NULL) ifname = buf;
		else ++ifname;

#if defined(GTAC5300) || defined(RTAX88U) || defined(GTAX11000) || defined(RTAX92U) || defined(RTAX95Q) || defined(XT8PRO) || defined(BM68) || defined(XT8_V2) || defined(RTAXE95Q) || defined(ET8PRO) || defined(ET8_V2) || defined(RTAX56U) || defined(RTAX68U) || defined(RTAX55) || defined(RTAX1800) || defined(GTAXE11000) || defined(GTAX11000_PRO) || defined(GTAXE16000) || defined(GTAX6000) || defined(RTAX3000N) || defined(RTAX88U_PRO) || defined(RTAX9000)
		if (strcmp(ifname, "eth1")
#if defined(GTAC5300) || defined(RTAX88U) || defined(GTAX11000) || defined(RTAX92U) || defined(RTAX95Q) || defined(XT8PRO) || defined(BM68) || defined(XT8_V2) || defined(RTAXE95Q) || defined(ET8PRO) || defined(ET8_V2) || defined(RTAX56U) || defined(RTAX68U) || defined(GTAXE11000) || defined(GTAX11000_PRO) || defined(GTAXE16000) || defined(GTAX6000) || defined(RTAX88U_PRO) || defined(RTAX9000)
			&& strcmp(ifname, "eth2") && strcmp(ifname, "eth3") && strcmp(ifname, "eth4")
#endif
#if defined(GTAXE16000) || defined(GTAX11000_PRO)
			&& strcmp(ifname, "eth0") && !nvram_match("wan_ifname", ifname)
#endif
#ifdef RTCONFIG_EXT_BCM53134
 			&& strcmp(ifname, "eth5")
#elif defined(RTCONFIG_EXTPHY_BCM84880) && !defined(BCM4912)
			&& (strcmp(ifname, "eth5") && !nvram_get_int("wans_extwan"))
#endif
		) continue;


		if (sscanf(p+1, "%lu", &tmpcnt) != 1) continue;
		counter += tmpcnt;
#elif defined(RTAX86U)
		if (strcmp(ifname, "eth5"))
			continue;

		if (sscanf(p+1, "%lu", &tmpcnt) != 1) continue;
		counter += tmpcnt;
#else
		if (strcmp(ifname, "vlan1")) continue;
		if (sscanf(p+1, "%lu", &counter) != 1) continue;
#endif

	}
	fclose(f);

	return counter;
}

unsigned long get_etlan_count_specific_if(char* target_if)
{
	FILE *f;
	char buf[256];
	char *ifname, *p;
	unsigned long counter=0;
	unsigned long tmpcnt=0;

	if ((f = fopen("/proc/net/dev", "r")) == NULL) return -1;

	fgets(buf, sizeof(buf), f);
	fgets(buf, sizeof(buf), f);
	while (fgets(buf, sizeof(buf), f)) {
		if ((p=strchr(buf, ':')) == NULL) continue;
		*p = 0;
		if ((ifname = strrchr(buf, ' ')) == NULL) ifname = buf;
		else ++ifname;

		if (strcmp(ifname, target_if)) continue;
		if (sscanf(p+1, "%lu", &tmpcnt) != 1) continue;
		counter += tmpcnt;
		break;
	}
	fclose(f);

	return counter;
}

#if defined(GTAC5300) || defined(RTAX88U)
enum {
	AGGLED_ACT_ALLOFF,
	AGGLED_ACT_ALLON,
	AGGLED_ACT_EXTOFF
};

extern unsigned long _memaccess(int argc, char *argv[]);

void
aggled_control(int mode)
{
	//printf("aggled ctrl: change to mode %d\n", mode);
	switch(mode) {
	case AGGLED_ACT_ALLOFF:
		_memaccess(elm_of_strr(sw_aggled_alloff), (char **) sw_aggled_alloff);
		break;
	case AGGLED_ACT_ALLON:
		_memaccess(elm_of_strr(sw_aggled_allon), (char **) sw_aggled_allon);
		break;
	case AGGLED_ACT_EXTOFF:
		_memaccess(elm_of_strr(sw_aggled_extoff), (char **) sw_aggled_extoff);
		break;
	}
}
#endif

#if !defined(RTAX55) && !defined(RTAX1800) && !defined(RTAX3000N) && !defined(BR63)
static int lstatus = -1;
#endif
#ifndef RTCONFIG_LAN4WAN_LED
#if defined(RTCONFIG_LED_BTN) || defined(RTCONFIG_WPS_ALLLED_BTN) || (!defined(RTCONFIG_WIFI_TOG_BTN) && !defined(RTCONFIG_QCA))
static int allstatus = 0;
#endif
#endif

void fake_etlan_led(void)
{
#if !defined(GTAC5300) && !defined(RTAX88U)
	static unsigned int blink_etlan_check = 0;
	static unsigned int blink_etlan = 0;
	static unsigned int data_etlan = 0;
	unsigned long count_etlan;
	int i;
	static int j;
#endif
	static int status = -1;
	static int status_old;
	int phystatus = 0;

#if defined(RTAX86U)
	if(!strcmp(get_productid(), "RT-AX86S") || !nvram_get_int("wans_extwan"))
		return;
#endif

#ifndef RTCONFIG_LAN4WAN_LED
#if defined(RTCONFIG_LED_BTN) || defined(RTCONFIG_WPS_ALLLED_BTN) || (!defined(RTCONFIG_WIFI_TOG_BTN) && !defined(RTCONFIG_QCA))
	if (nvram_match("AllLED", "0")) {
		if (allstatus)
			led_control(LED_LAN, LED_OFF);
		allstatus = 0;
		return;
	}
	allstatus = 1;
#endif
#endif

	if (nvram_get_int("etlan_led_reset")) {
		nvram_set_int("etlan_led_reset", 0);
#if !defined(RTAX55) && !defined(RTAX1800) && !defined(RTAX3000N) && !defined(BR63)
		lstatus = 1;
#endif
		status = -1;
	}

#if defined(DSL_AX82U)
	if (lanport_status())
		led_control(LED_LAN, LED_ON);
	else
		led_control(LED_LAN, LED_OFF);
	return;
#endif

#if defined(RTAX55) || defined(RTAX1800) || defined(RTAX3000N) || defined(BR63)
	phystatus = rtkswitch_lanPorts_phyStatus();
	if (!phystatus) {
		led_control(LED_LAN, LED_OFF);
		status = -1;
		return;
	}
#else
#if defined(RTAX86U)
	phystatus = hnd_get_phy_status("eth5");
	if (!phystatus)
#else
	phystatus = GetPhyStatus(0, NULL);

#if defined(RTCONFIG_EXTPHY_BCM84880) && !defined(BCM4912) && !defined(BCM6855)
	if ((nvram_get_int("wans_extwan") && !(phystatus & 0x3e)) || // configure 2.5G port as WAN, need to consider 1G WAN connectivity
			(!nvram_get_int("wans_extwan") && !(phystatus & 0x1e)))  // configure 2.5G port as LAN, ignore 2.5G port
#elif defined(GTAX6000) || defined(RTAX88U_PRO)
	if (!((phystatus & 0x2) || (phystatus & 0x4) || (phystatus & 0x8) || (phystatus & 0x10)))
#elif defined(RTAX9000)
	if (!((phystatus & 0x2) || (phystatus & 0x4) || (phystatus & 0x8) || (phystatus & 0x10) || (phystatus & 0x20)))
#else
	if (!phystatus
#if defined(RTAX92U) || defined(RTAX95Q) || defined(XT8PRO) || defined(BM68) || defined(XT8_V2) || defined(RTAXE95Q) || defined(ET8PRO) || defined(ET8_V2) || defined(RTAX68U)
			|| phystatus == 1 // ignore WAN
#endif
#if defined(GTAXE16000) || defined(GTAX11000_PRO)
			|| (nvram_match("wan_ifname", "eth0") && phystatus == 0x1)
			|| (nvram_match("wan_ifname", "eth5") && phystatus == 0x20 && !hnd_get_phy_status("eth0"))
			|| (nvram_match("wan_ifname", "eth6") && phystatus == 0x40 && !hnd_get_phy_status("eth0"))
#endif
	)
#endif // RTCONFIG_EXTPHY_BCM84880
#endif // RTAX86U
	{
		if (lstatus)
#if defined(GTAC5300) || defined(RTAX88U)
			aggled_control(AGGLED_ACT_ALLOFF);
#else
			led_control(LED_LAN, LED_OFF);
#endif
		lstatus = 0;
		status = -1;
		return;
	}
	lstatus = 1;
#endif

#if defined(GTAC5300) || defined(RTAX88U)
	status_old = status;
	status = GetPhyStatus(53134, NULL);
	if (status != status_old || status_old == -1) {
		if (status)
			aggled_control(AGGLED_ACT_ALLON);
		else
			aggled_control(AGGLED_ACT_EXTOFF);
	}
#else
	// check data per 10 count
	if ((blink_etlan_check%10) == 0) {
		count_etlan = get_etlan_count();
		if (count_etlan && data_etlan != count_etlan) {
			blink_etlan = 1;
			data_etlan = count_etlan;
		}
		else blink_etlan = 0;
		led_control(LED_LAN, LED_ON);
	}

	if (blink_etlan) {
		j = rand_seed_by_time() % 3;
		for(i=0;i<10;i++) {
			usleep(33*1000);

			status_old = status;
			if (((i%2) == 0) && (i > (3 + 2*j)))
				status = 0;
			else
				status = 1;

			if (status != status_old)
			{
				if (status)
					led_control(LED_LAN, LED_ON);
				else
					led_control(LED_LAN, LED_OFF);
			}
		}
		led_control(LED_LAN, LED_ON);
	}

	blink_etlan_check++;
#endif
}

#if defined(GTAXE16000) || defined(GTAX11000_PRO)
static int lstatus_extra = -1;
static int allstatus_extra = 0;

void fake_etlan_led_extra(void)
{
	static unsigned int blink_check = 0;
	static unsigned int blink = 0;
	static unsigned int prev_count = 0;
	unsigned long curr_count = 0;
	int i;
	static int j;
	static int status = -1;
	static int status_old;
	int phystatus = 0;
	int lan_num = 0;
	static int led;

	if (nvram_match("AllLED", "0")) {
		if (allstatus_extra) {
			led_control(LED_10G_WHITE, LED_OFF);
			led_control(LED_10G_RGB_BLUE, LED_OFF);
		}
		allstatus_extra = 0;
		return;
	}
	allstatus_extra = 1;

	phystatus = GetPhyStatus(0, NULL);
	if (!(phystatus & 0x60)) // both 10GE ports are not connected
	{
		if (lstatus_extra) {
			led_control(LED_10G_WHITE, LED_OFF);
			led_control(LED_10G_RGB_BLUE, LED_OFF);
			lstatus_extra = 0;
		}
		lstatus_extra = 0;
		return;
	}

	// check data per 10 count
	if ((blink_check%10) == 0) {
		if (phystatus & 0x20) { curr_count += get_etlan_count_specific_if("eth5"); lan_num++;}
		if (phystatus & 0x40) { curr_count += get_etlan_count_specific_if("eth6"); lan_num++;}
		if (curr_count && prev_count != curr_count) {
			blink = 1;
			prev_count = curr_count;
		}
		else
		    blink = 0;

		if(lan_num > 1) {
			led = LED_10G_RGB_BLUE;
			led_control(LED_10G_WHITE, LED_OFF);
			led_control(LED_10G_RGB_BLUE, LED_ON);
		}
		else {
			led = LED_10G_WHITE;
			led_control(LED_10G_WHITE, LED_ON);
			led_control(LED_10G_RGB_BLUE, LED_OFF);
		}
		lstatus_extra = 1;
	}

	if (blink) {
		j = rand_seed_by_time() % 3;
		for(i=0;i<10;i++) {
			usleep(33*1000);

			status_old = status;
			if (((i%2) == 0) && (i > (3 + 2*j)))
				status = 0;
			else
				status = 1;

			if (status != status_old)
			{
				if (status)
					led_control(led, LED_ON);
				else
					led_control(led, LED_OFF);
			}
		}
		led_control(led, LED_ON);
		lstatus_extra = 1;
	}

	blink_check++;
}
#endif

#ifdef RTAX9000
unsigned long get_etwan_count()
{
	FILE *f;
	char buf[256];
	char *ifname, *p;
	unsigned long tmpcnt=0;
	unsigned long counter=0;

	if ((f = fopen("/proc/net/dev", "r")) == NULL) return -1;

	fgets(buf, sizeof(buf), f);
	fgets(buf, sizeof(buf), f);

	while (fgets(buf, sizeof(buf), f)) {
		if ((p=strchr(buf, ':')) == NULL) continue;
		*p = 0;
		if ((ifname = strrchr(buf, ' ')) == NULL) ifname = buf;
		else ++ifname;

		if (strcmp(ifname, "eth0")) continue;

		if (sscanf(p+1, "%lu", &tmpcnt) != 1) continue;
		counter += tmpcnt;

	}
	fclose(f);

	return counter;
}

void fake_etwan_led(void)
{
	static unsigned int blink_etwan_check = 0;
	static unsigned int blink_etwan = 0;
	static unsigned int data_etwan = 0;
	unsigned long count_etwan;
	int i;
	static int j;

	static int status = -1;
	static int status_old;
	int phystatus = 0;

	phystatus = hnd_get_phy_status(0);
	if (!phystatus ) {
		led_control(LED_WAN_NORMAL, LED_OFF);
		status = -1;
		return;
	}

	if (nvram_match("AllLED", "0") || nvram_match("x_Setting", "0") || !nvram_match("link_internet", "2")) {
		led_control(LED_WAN_NORMAL, LED_OFF);
		return;
	}

	if (nvram_get_int("etwan_led_reset")) {
		nvram_set_int("etwan_led_reset", 0);
		status = -1;
	}

	// check data per 10 count
	if ((blink_etwan_check%10) == 0) {
		count_etwan = get_etwan_count();
		if (count_etwan && data_etwan != count_etwan) {
			blink_etwan = 1;
			data_etwan = count_etwan;
		}
		else blink_etwan = 0;
		led_control(LED_WAN_NORMAL, LED_ON);
	}

	if (blink_etwan) {
		j = rand_seed_by_time() % 3;
		for(i=0;i<10;i++) {
			usleep(33*1000);

			status_old = status;
			if (((i%2) == 0) && (i > (3 + 2*j)))
				status = 0;
			else
				status = 1;

			if (status != status_old)
			{
				if (status)
					led_control(LED_WAN_NORMAL, LED_ON);
				else
					led_control(LED_WAN_NORMAL, LED_OFF);
			}
		}
		led_control(LED_WAN_NORMAL, LED_ON);
	}

	blink_etwan_check++;
}
#endif
#endif	// RTCONFIG_FAKE_ETLAN_LED

#if defined(RTCONFIG_WLAN_LED) || defined(RTN18U)
unsigned long get_2g_count()
{
	FILE *f;
	char buf[256];
	char *ifname, *p;
	unsigned long counter1, counter2;

	if ((f = fopen("/proc/net/dev", "r")) == NULL) return -1;

	fgets(buf, sizeof(buf), f);
	fgets(buf, sizeof(buf), f);

	counter1=counter2=0;

	while (fgets(buf, sizeof(buf), f)) {
		if ((p=strchr(buf, ':')) == NULL) continue;
		*p = 0;
		if ((ifname = strrchr(buf, ' ')) == NULL) ifname = buf;
		else ++ifname;

		if (strcmp(ifname, "eth1")) continue;

		if (sscanf(p+1, "%lu%*u%*u%*u%*u%*u%*u%*u%*u%lu", &counter1, &counter2) != 2) continue;

	}
	fclose(f);

	return counter1;
}

void fake_wl_led_2g(void)
{
	static unsigned int blink_2g_check = 0;
	static unsigned int blink_2g = 0;
	static unsigned int data_2g = 0;
	unsigned long count_2g;
	int i;
	static int j;
	static int status = -1;
	static int status_old;

	// check data per 10 count
	if ((blink_2g_check%10) == 0) {
		count_2g = get_2g_count();
		if (count_2g && data_2g != count_2g) {
			blink_2g = 1;
			data_2g = count_2g;
		}
		else blink_2g = 0;
		led_control(LED_2G, LED_ON);
	}

	if (blink_2g) {
		j = rand_seed_by_time() % 3;
		for(i=0;i<10;i++) {
			usleep(33*1000);

			status_old = status;
			if (((i%2) == 0) && (i > (3 + 2*j)))
				status = 0;
			else
				status = 1;

			if (status != status_old)
			{
				if (status)
					led_control(LED_2G, LED_ON);
				else
					led_control(LED_2G, LED_OFF);
			}
		}
		led_control(LED_2G, LED_ON);
	}

	blink_2g_check++;
}
#endif	/* RTCONFIG_WLAN_LED */

#if defined(RTCONFIG_BRCM_USBAP) || defined(RTAC66U) || defined(BCM4352)
unsigned long get_5g_count()
{
	FILE *f;
	char buf[256];
	char *ifname, *p;
	unsigned long counter1, counter2;

	if ((f = fopen("/proc/net/dev", "r")) == NULL) return -1;

	fgets(buf, sizeof(buf), f);
	fgets(buf, sizeof(buf), f);

	counter1=counter2=0;

	while (fgets(buf, sizeof(buf), f)) {
		if ((p=strchr(buf, ':')) == NULL) continue;
		*p = 0;
		if ((ifname = strrchr(buf, ' ')) == NULL) ifname = buf;
		else ++ifname;

		if (strcmp(ifname, "eth2")) continue;

		if (sscanf(p+1, "%lu%*u%*u%*u%*u%*u%*u%*u%*u%lu", &counter1, &counter2) != 2) continue;

	}
	fclose(f);

	return counter1;
}

void fake_wl_led_5g(void)
{
	static unsigned int blink_5g_check = 0;
	static unsigned int blink_5g = 0;
	static unsigned int data_5g = 0;
	unsigned long count_5g;
	int i;
	static int j;
	static int status = -1;
	static int status_old;
	int debug = nvram_get_int("debug_fake_wl_led_5g");

	// check data per 10 count
	if ((blink_5g_check%10) == 0) {
		count_5g = get_5g_count();
		if (count_5g && data_5g != count_5g) {
			blink_5g = 1;
			data_5g = count_5g;
		}
		else blink_5g = 0;
		led_control(LED_5G, LED_ON);
	}

	if (blink_5g) {
#if defined(RTAC66U) || defined(BCM4352)
		j = rand_seed_by_time() % 3;
#endif
		for(i=0;i<10;i++) {
#if defined(RTAC66U) || defined(BCM4352)
			usleep(33*1000);

			status_old = status;
			if (((i%2) == 0) && (i > (3 + 2*j)))
				status = 0;
			else
				status = 1;

			if (status != status_old)
			{
				if (status)
					led_control(LED_5G, LED_ON);
				else
					led_control(LED_5G, LED_OFF);
			}
#else
			usleep(50*1000);
			if (i%2 == 0) {
				led_control(LED_5G, LED_OFF);
			}
			else {
				led_control(LED_5G, LED_ON);
			}
			if (debug)
			_dprintf("****** %s:%d ******\n", __func__, __LINE__);
#endif
		}
		led_control(LED_5G, LED_ON);
		if (debug)
		_dprintf("****** %s:%d ******\n", __func__, __LINE__);
	}

	blink_5g_check++;
}
#endif

static int led_confirmed = 0;
extern int led_gpio_table[LED_ID_MAX];

int confirm_led()
{
	if (
		1
#if defined(RTN53) || defined(RTN18U)
		&& led_gpio_table[LED_2G] != 0xff
		&& led_gpio_table[LED_2G] != -1
#endif
#ifndef RTCONFIG_LAN4WAN_LED
#ifdef RTCONFIG_FAKE_ETLAN_LED
		&& led_gpio_table[LED_LAN] != 0xff
		&& led_gpio_table[LED_LAN] != -1
#endif
#endif
#if defined(RTCONFIG_USB) && !defined(RTCONFIG_BLINK_LED)
#ifdef RTCONFIG_USB_XHCI
		&& led_gpio_table[LED_USB3] != 0xff
		&& led_gpio_table[LED_USB3] != -1
#endif
		&& led_gpio_table[LED_USB] != 0xff
		&& led_gpio_table[LED_USB] != -1
#endif
#ifdef RTCONFIG_MMC_LED
		&& led_gpio_table[LED_MMC] != 0xff
		&& led_gpio_table[LED_MMC] != -1
#endif
#if defined(RTCONFIG_BRCM_USBAP) || defined(RTAC66U) || defined(BCM4352)
		&& led_gpio_table[LED_5G] != 0xff
		&& led_gpio_table[LED_5G] != -1
#endif
#ifdef RTCONFIG_DSL
#ifndef RTCONFIG_DUALWAN
		&& led_gpio_table[LED_WAN] != 0xff
		&& led_gpio_table[LED_WAN] != -1
#endif
#endif
	)
		led_confirmed = 1;
	else
		led_confirmed = 0;

	return led_confirmed;
}

#ifdef SW_DEVLED
#if defined(RTCONFIG_LED_BTN) || defined(RTCONFIG_WPS_ALLLED_BTN) || defined(RTCONFIG_TURBO_BTN) || (!defined(RTCONFIG_WIFI_TOG_BTN) && !defined(RTCONFIG_QCA))
static int swled_alloff_counts = 0;
static int swled_alloff_x = 0;
#endif
#ifdef BLUECAVE
enum {
	CASE_NONE = 0,
	CASE_INDICATOR_INIT,
	CASE_INDICATOR_WPS,
	CASE_INDICATOR_WPS_DONE,
	CASE_INDICATOR_RESET,
};

static int bh_case = CASE_NONE;
static int indicator_rush_counts = -1;
static int central_rush_counts = 0;
static int led_alarm_rush = 0;
static int central_cycle = 0;
static int brightness_level = 0;
static int indicator_no_internet_red = -1;
static int indicator_no_internet_red_old = -1;
static int indicator_no_internet_cnt = 0;
static int central_bh[10] = {0,1,2,3,3,3,3,3,2,1};

void led_rush(int sig)
{
	if(nvram_match("bc_ledbh", "reset"))
		bh_case = CASE_INDICATOR_RESET;
	else if(nvram_match("bc_ledbh", "wps"))
		bh_case = CASE_INDICATOR_WPS;
	else if(nvram_match("bc_ledbh", "wps_done"))
		bh_case = CASE_INDICATOR_WPS_DONE;
	else if(!nvram_match("x_Setting", "1"))
		bh_case = CASE_INDICATOR_INIT;
	else
		bh_case = CASE_NONE;

	central_cycle = nvram_match("x_Setting", "0") ? 1 : 0;

	nvram_set("bc_ledbh", "");
	led_alarm_rush = 1;
	indicator_rush_counts = -1;
	alarmtimer(0, RUSHURGENT_PERIOD);
}

void led_stop(int sig)
{
	bh_case = CASE_NONE;
	indicator_rush_counts = -1;

	led_control(LED_INDICATOR_SIG1, LED_OFF); //turn off RED led
	if (!nvram_get_int("wave_ready"))
		led_control(LED_INDICATOR_SIG2, LED_ON); //turn on BLUE led
	else
		led_control(LED_INDICATOR_SIG2, LED_OFF); //turn off BLUE led


	if(!central_cycle) {
		led_alarm_rush = 0;
		alarmtimer(NORMAL_PERIOD, 0);
	}
}

void bluecave_ledbh_central()
{

	if(!nvram_get_int("wave_ready") || (!central_cycle && central_rush_counts == -1))
		return;

	/* cycle central led util QIS is completed */
	if(central_cycle) {
		central_rush_counts = (central_rush_counts + 1) % 6;
		if(central_rush_counts == 0)	//300 ms
			setCentralLedLv(central_bh[++brightness_level % 10]);
	}
	else {
		notify_rc("reset_led");
		central_rush_counts = -1;
	}
}

void bluecave_ledbh_indicator()
{
	int WpsLedBlink, WpsLedOff;

	if (!nvram_get_int("wave_ready") && bh_case != CASE_INDICATOR_RESET)
		return;


	/* indicator led control */
	switch(bh_case) {
		/* 0123456789 */
		/* 0011100111 */
		/* blink BLUE led util QIS is completed */
		case CASE_INDICATOR_INIT:

			if(indicator_rush_counts == -1) {
				led_control(LED_INDICATOR_SIG1, LED_OFF); //turn off RED led
				led_control(LED_INDICATOR_SIG2, LED_OFF); //turn off BLUE led
				indicator_rush_counts++;
			}

			if(nvram_match("x_Setting", "1"))
				kill_pidfile_s("/var/run/sw_devled.pid", SIGUSR1);
			break;

		case CASE_INDICATOR_WPS:
			led_control(LED_INDICATOR_SIG1, LED_OFF);
			indicator_rush_counts = (indicator_rush_counts + 1) % 20;
			if ((indicator_rush_counts % 2) == 0 && (indicator_rush_counts > 10))
				led_control(LED_INDICATOR_SIG2, LED_ON);
			else
				led_control(LED_INDICATOR_SIG2, LED_OFF);
			break;

		/* blink led for WPS complete/terminate */
		case CASE_INDICATOR_WPS_DONE:

			indicator_rush_counts++;
			if(nvram_get_int("wps_success")) {	// blink BLUE LED
				WpsLedBlink = LED_INDICATOR_SIG2;
				WpsLedOff = LED_INDICATOR_SIG1;
			}
			else {					// blink RED LED
				WpsLedBlink = LED_INDICATOR_SIG1;
				WpsLedOff = LED_INDICATOR_SIG2;
			}

			led_control(WpsLedOff, LED_OFF);
			if ((indicator_rush_counts % 10) < 2 || ((indicator_rush_counts % 10) > 4 && (indicator_rush_counts % 10) < 7))
				led_control(WpsLedBlink, LED_OFF);
			else
				led_control(WpsLedBlink, LED_ON);

			if(indicator_rush_counts >= 20 * 6) {	// blink 6s then turn off
				led_control(WpsLedBlink, LED_OFF);
				nvram_set("wps_success", "0");
				kill_pidfile_s("/var/run/sw_devled.pid", SIGUSR1);
			}
			break;

		case CASE_INDICATOR_RESET:
			indicator_rush_counts = (indicator_rush_counts + 1) % 10;
			if ((indicator_rush_counts % 10) < 2 || ((indicator_rush_counts % 10) > 4 && (indicator_rush_counts % 10) < 7)) {
					led_control(LED_INDICATOR_SIG1, LED_OFF);
					led_control(LED_INDICATOR_SIG2, LED_OFF);
			}
			else {
				led_control(LED_INDICATOR_SIG1, LED_ON);
				led_control(LED_INDICATOR_SIG2, LED_ON);
			}
			if (!button_pressed(BTN_RESET))
				kill_pidfile_s("/var/run/sw_devled.pid", SIGUSR1);
			break;

		case CASE_NONE:
		default:

			// regular checking WAN status
			if(!nvram_match("link_internet", "2")) {
				if(++indicator_no_internet_cnt >= 3)
					indicator_no_internet_red = 1;
			}
			else {
				indicator_no_internet_red = 0;
				indicator_no_internet_cnt = 0;
			}

			// Solid RED led if no internet ability
			if((indicator_no_internet_red != indicator_no_internet_red_old) ||
			   (indicator_no_internet_red && !get_gpio(nvram_get_int("led_idr_sig1_gpio")) ||
			   (!indicator_no_internet_red) && get_gpio(nvram_get_int("led_idr_sig2_gpio")))) // WAR for gpio was reset by mem xxx
			{
				indicator_no_internet_red == 1 ?
					led_control(LED_INDICATOR_SIG1, LED_ON) :
					led_control(LED_INDICATOR_SIG1, LED_OFF);
				led_control(LED_INDICATOR_SIG2, LED_OFF);
				indicator_no_internet_red_old = indicator_no_internet_red;
			}

			break;
	}
}
#endif

#ifdef DSL_AX82U
void led_DSLWAN(int force_update)
{
	static unsigned count = 0;
	static int status_old = -1;
	static int link_internet_old = -1;
	int link_internet = nvram_get_int("link_internet");
	int status = 0;
	int unit = WAN_UNIT_FIRST;

	// DSL or ETH up
	if (nvram_match("dsltmp_adslsyncsts", "init"))
		status = 1;
	else if (nvram_match("dsltmp_adslsyncsts", "up") || hnd_get_phy_status(4))
		status = 2;
	else
		status = 0;

	// DSL initializing, Flash RED LED only.
	if (status == 1)
	{
#if defined(RTCONFIG_WANRED_LED)
		led_control(LED_WAN, LED_OFF);
		if (++count%2)
			led_control(LED_WAN_RED, LED_ON);
		else
			led_control(LED_WAN_RED, LED_OFF);
#else
		led_control(LED_WAN_NORMAL, LED_OFF);
		if (++count%2)
			led_control(LED_WAN, LED_ON);
		else
			led_control(LED_WAN, LED_OFF);
#endif
		if (status != status_old)
			status_old = status;
	}
	else
	{
		if (link_internet != link_internet_old || status != status_old || force_update)
		{
			//_dprintf("\n=====\nstatus: %d, status_old: %d, link_internet: %d, link_internet_old: %d\n=====\n", status, status_old, link_internet, link_internet_old);
			if (link_internet == 2)
			{
#if defined(RTCONFIG_WANRED_LED)
				led_control(LED_WAN_RED, LED_OFF);
				led_control(LED_WAN, LED_ON);
#else
				led_control(LED_WAN, LED_OFF);
				led_control(LED_WAN_NORMAL, LED_ON);
#endif
			}
			else
			{
				if (status == 2)
				{
#if defined(RTCONFIG_WANRED_LED)
					led_control(LED_WAN_RED, LED_ON);
					led_control(LED_WAN, LED_OFF);
#else
					led_control(LED_WAN, LED_ON);
					led_control(LED_WAN_NORMAL, LED_OFF);
#endif
				}
				else
				{
#if defined(RTCONFIG_WANRED_LED)
					led_control(LED_WAN_RED, LED_OFF);
					led_control(LED_WAN, LED_OFF);
#else
					led_control(LED_WAN, LED_OFF);
					led_control(LED_WAN_NORMAL, LED_OFF);
#endif
				}
			}
			link_internet_old = link_internet;
			status_old = status;
		}
	}
}
#endif

void led_check(int sig)
{
#ifdef BLUECAVE
	bluecave_ledbh_central();
	bluecave_ledbh_indicator();
	if(led_alarm_rush && !central_cycle && bh_case == CASE_NONE)
		kill_pidfile_s("/var/run/sw_devled.pid", SIGUSR2);
#endif

#if (defined(RTCONFIG_LED_BTN) || defined(RTCONFIG_WPS_ALLLED_BTN) || defined(RTCONFIG_TURBO_BTN) || (!defined(RTCONFIG_WIFI_TOG_BTN) && !defined(RTCONFIG_QCA))) && !defined(GTAX6000) && !defined(TUFAX3000_V2) && !defined(RTAXE7800) && !defined(GT10) && !defined(RTAX9000)
	int all_led;
	int turnoff_counts = swled_alloff_counts?:3;

	if ((all_led=nvram_get_int("AllLED")) == 0 && swled_alloff_x < turnoff_counts) {
		/* turn off again x times in case timing issues */
		setAllLedOff();
		led_table_ctrl(LED_OFF);
		swled_alloff_x++;
		_dprintf("force turnoff led table again!\n");

#ifdef RTCONFIG_EXTPHY_BCM84880
#if defined(RTAX86U) || defined(GTAX11000)
		int ext_phy_model = nvram_get_int("ext_phy_model");

		if(!strcmp(get_productid(), "RT-AX86S")) ;
		else if(nvram_get_int("wans_extwan")){
			if(ext_phy_model == EXT_PHY_GPY211)
				eval("ethctl", "phy", "ext", EXTPHY_GPY_ADDR_STR, "0x1e0001", "0x0");
			else if(ext_phy_model == EXT_PHY_RTL8226)
				eval("ethctl", "phy", "ext", EXTPHY_RTL_ADDR_STR, "0x1fd032", "0x0000");	// RTL LCR2 LED Control Reg
			else if(ext_phy_model == EXT_PHY_BCM54991)
				eval("ethctl", "phy", "ext", EXTPHY_ADDR_STR, "0x1a835", "0x0");	// CTL LED4 MASK LOW
		}
#endif
#endif

		return;
	}

	if (all_led)
		swled_alloff_x = 0;
	else
		return;
#endif

	if (!confirm_led()) {
		get_gpio_values_once(1);
	}

#ifdef RTCONFIG_WLAN_LED
	if (nvram_contains_word("rc_support", "led_2g"))
	{
#if defined(RTN53)
		if (nvram_get_int("wl0_radio") == 0)
			led_control(LED_2G, LED_OFF);
		else
#endif
		fake_wl_led_2g();
	}
#endif

#ifdef RTN18U
	if (nvram_match("bl_version", "1.0.0.0"))
		fake_wl_led_2g();
#endif

#ifdef RTCONFIG_FAKE_ETLAN_LED
	fake_etlan_led();
#if defined(GTAXE16000) || defined(GTAX11000_PRO)
	fake_etlan_led_extra();
#endif
#ifdef RTAX9000
	fake_etwan_led();
#endif
#endif

#if defined(RTCONFIG_USB) && (defined(RTCONFIG_BCM_7114) || defined(HND_ROUTER))
	char p1_node[32], p2_node[32], ehci_ports[32], xhci_ports[32];

	/* led indicates which usb port */
	switch (get_model()) {
		case MODEL_RTAC5300:
		case MODEL_GTAC5300:
		case MODEL_RTAC88U:
		case MODEL_RTAC86U:
		case MODEL_RTAC3100:
		case MODEL_RTAX88U:
		case MODEL_BC109:
		case MODEL_EBG19:
		case MODEL_EBG15:
		case MODEL_EBP15:
		case MODEL_GTAX11000:
		case MODEL_RTAX92U:
		case MODEL_RTAX95Q:
		case MODEL_XT8PRO:
		case MODEL_BM68:
		case MODEL_XT8_V2:
		case MODEL_RTAXE95Q:
		case MODEL_ET8PRO:
		case MODEL_ET8_V2:
		case MODEL_RTAX56_XD4:
		case MODEL_XD4PRO:
		case MODEL_CTAX56_XD4:
		case MODEL_RTAX58U:
		case MODEL_RTAX82U_V2:
		case MODEL_TUFAX5400_V2:
		case MODEL_RTAX58U_V2:
		case MODEL_GT10:
		case MODEL_RTAX9000:
		case MODEL_RTAXE7800:
		case MODEL_TUFAX3000_V2:
		case MODEL_RTAX56U:
		case MODEL_GTAXE11000:
		case MODEL_GTAX6000:
		case MODEL_GTAX11000_PRO:
		case MODEL_GTAXE16000:
		case MODEL_RTAX88U_PRO:
		default:
			snprintf(p1_node, sizeof(p1_node), "%s", nvram_safe_get("usb_path1_node"));
			snprintf(p2_node, sizeof(p2_node), "%s", nvram_safe_get("usb_path2_node"));
			break;
	}

	snprintf(ehci_ports, sizeof(ehci_ports), "%s", nvram_safe_get("ehci_ports"));
	snprintf(xhci_ports, sizeof(xhci_ports), "%s", nvram_safe_get("xhci_ports"));

#ifdef RTCONFIG_USB_XHCI
	if(strlen(p1_node) > 0){
		if (strstr(xhci_ports, p1_node))
			fake_dev_led(nvram_safe_get("xhci_irq"), LED_USB3);
		else if (strstr(ehci_ports, p1_node))
			fake_dev_led(nvram_safe_get("ehci_irq"), LED_USB3);
	}
#endif
	if(strlen(p2_node) > 0)
		fake_dev_led(nvram_safe_get("ehci_irq"), LED_USB);
#endif

#ifdef RTCONFIG_MMC_LED
	if (*nvram_safe_get("usb_path3"))
		fake_dev_led(nvram_safe_get("mmc_irq"), LED_MMC);
#endif

#if defined(RTCONFIG_BRCM_USBAP) || defined(RTAC66U) || defined(BCM4352)
#if defined(RTAC66U) || defined(BCM4352)
	if (nvram_match("led_5g", "1") &&
	   (wlonunit == -1 || wlonunit == 1))
#endif
	{
#if defined(RTN53)
		if (nvram_get_int("wl1_radio") == 0)
			led_control(LED_5G, LED_OFF);
		else
#endif
		fake_wl_led_5g();
	}
#endif

// it is not really necessary, but if required, add internet led check here
// using wan_primary_ifunit() to get current working wan unit wan0 or wan1
// using wan0_state_t or wan1_state_t to get status of working wan,
//	WAN_STATE_CONNECTED means internet connected
//	else means internet disconnted

/* Paul add 2012/10/25 */
#ifdef RTCONFIG_DSL
#ifndef RTCONFIG_DUALWAN
	if (nvram_match("dsltmp_adslsyncsts","up") && is_wan_connect(0))
		led_DSLWAN();
#endif
#ifdef DSL_AX82U
	led_DSLWAN(0);
#endif
#endif
}
#endif

#if defined (RTCONFIG_LED_BTN) || defined (RTCONFIG_WPS_ALLLED_BTN) || defined(RTCONFIG_TURBO_BTN) || (!defined(RTCONFIG_WIFI_TOG_BTN) && !defined(RTCONFIG_QCA))
void led_table_ctrl(int on_off)
{
	int i;

	for(i=0; i < LED_ID_MAX; ++i) {
		if (led_gpio_table[i] != 0xff && led_gpio_table[i] != -1 && i != PWR_USB) {
			led_control(i, on_off);
		}
	}
}
#endif

/* Paul add 2012/10/25 */
#ifdef RTCONFIG_DSL
#ifndef RTCONFIG_DUALWAN
unsigned long get_dslwan_count()
{
	FILE *f;
	char buf[256];
	char *ifname, *p;
	unsigned long counter1, counter2;

	if ((f = fopen("/proc/net/dev", "r")) == NULL) return -1;

	fgets(buf, sizeof(buf), f);
	fgets(buf, sizeof(buf), f);

	counter1=counter2=0;

	while (fgets(buf, sizeof(buf), f)) {
		if ((p=strchr(buf, ':')) == NULL) continue;
		*p = 0;
		if ((ifname = strrchr(buf, ' ')) == NULL) ifname = buf;
		else ++ifname;

		if (nvram_match("dsl0_proto","pppoe") || nvram_match("dsl0_proto","pppoa"))
		{
			if (strcmp(ifname, "ppp0")) continue;
		}
		else //Mer, IPoA
		{
			if (strcmp(ifname, "eth2.1.1")) continue;
		}

		if (sscanf(p+1, "%lu%*u%*u%*u%*u%*u%*u%*u%*u%lu", &counter1, &counter2) != 2) continue;
	}
	fclose(f);

	return counter1;
}

void led_DSLWAN(void)
{
	static unsigned int blink_dslwan_check = 0;
	static unsigned int blink_dslwan = 0;
	static unsigned int data_dslwan = 0;
	unsigned long count_dslwan;
	int i;
	static int j;
	static int status = -1;
	static int status_old;

	// check data per 10 count
	if ((blink_dslwan_check%10) == 0) {
		count_dslwan = get_dslwan_count();
		if (count_dslwan && data_dslwan != count_dslwan) {
			blink_dslwan = 1;
			data_dslwan = count_dslwan;
}
		else blink_dslwan = 0;
		led_control(LED_WAN, LED_ON);
	}

	if (blink_dslwan) {
		j = rand_seed_by_time() % 3;
		for(i=0;i<10;i++) {
			usleep(33*1000);

			status_old = status;
			if (((i%2) == 0) && (i > (3 + 2*j)))
				status = 0;
			else
				status = 1;

			if (status != status_old)
			{
				if (status)
					led_control(LED_WAN, LED_ON);
				else
					led_control(LED_WAN, LED_OFF);
			}
		}
		led_control(LED_WAN, LED_ON);
	}

	blink_dslwan_check++;
}
#endif
#endif

/* signal handlers stuff */
void unblock_sigs()
{
	sigprocmask(SIG_UNBLOCK, &sigs_to_catch, NULL);
}

typedef void (*dog_func)(int sig);
dog_func dp=NULL;

void put_all_dogs()
{
	int sig;
        for (sig = 0; sig < (_NSIG - 1); sig++) {
		if(sigbones & 1<<sig  &&  (dp=fn_acts[sig])) {
			(*dp)(sig);
			sigbones &= ~(1<<sig);
		}
	}
}

void init_sig() 
{
	int sig;
        for (sig = 0; sig < (_NSIG - 1); sig++) {
		if(sig == SIGCHLD 
		|| sig == SIGUSR1
		|| sig == SIGUSR2
		|| sig == SIGTSTP
		|| sig == SIGALRM
#if defined(RTAC1200G) || defined(RTAC1200GP)
		|| sig == SIGHUP
#endif
#ifdef RTCONFIG_RALINK
		|| sig == SIGTTIN
#endif
		)
                signal(sig, watch_sig);
		fn_acts[sig] = NULL;
	}

	fn_acts[SIGCHLD] = chld_reap_local;
	fn_acts[SIGUSR1] = catch_sig;
	fn_acts[SIGUSR2] = catch_sig;
	fn_acts[SIGTSTP] = catch_sig;
	fn_acts[SIGALRM] = watchdog;
#if defined(RTCONFIG_QCA)
	g_t1 = uptime();
#endif
#if defined(RTAC1200G) || defined(RTAC1200GP)
	fn_acts[SIGHUP]  = catch_sig;
#endif
#ifdef RTCONFIG_RALINK
	fn_acts[SIGTTIN] = catch_sig;
#endif

	sigemptyset(&sigs_to_catch);
	sigaddset(&sigs_to_catch, SIGCHLD);
	sigaddset(&sigs_to_catch, SIGUSR1);
	sigaddset(&sigs_to_catch, SIGUSR2);
	sigaddset(&sigs_to_catch, SIGTSTP);
	sigaddset(&sigs_to_catch, SIGALRM);
#if defined(RTAC1200G) || defined(RTAC1200GP)
	sigaddset(&sigs_to_catch, SIGHUP);
#endif
#ifdef RTCONFIG_RALINK
	sigaddset(&sigs_to_catch, SIGTTIN);
#endif
}

#ifdef SW_DEVLED
void init_sig_swled() 
{
	int sig;
        for (sig = 0; sig < (_NSIG - 1); sig++) {
		if(sig == SIGALRM
#ifdef BLUECAVE
		|| sig == SIGUSR1
		|| sig == SIGUSR2
#endif
		)
                signal(sig, watch_sig);
		fn_acts[sig] = NULL;
	}

	fn_acts[SIGALRM] = led_check;
#ifdef BLUECAVE
	fn_acts[SIGUSR1] = led_rush;
	fn_acts[SIGUSR2] = led_stop;
#endif
}
#endif

#if defined(RTAC1200G) || defined(RTAC1200GP)
void wdg_heartbeat(int sig)
{
	if(factory_debug())
		return;

	if(sig == SIGUSR1) {
		wdg_timer_alive++;
	}
	else if(sig == SIGALRM) {
		if(wdg_timer_alive) {
			wdg_timer_alive = 0;
		}
		else {
			_dprintf("[%s] Watchdog's heartbeat is stop! Recover...\n", __FUNCTION__);
			kill_pidfile_s("/var/run/watchdog.pid", SIGHUP);
		}
	}
}

void init_sig_wmon() 
{
	int sig;
        for (sig = 0; sig < (_NSIG - 1); sig++) {
		if(sig == SIGALRM
		|| sig == SIGUSR1
		)
                signal(sig, watch_sig);
		fn_acts[sig] = NULL;
	}

	fn_acts[SIGALRM] = wdg_heartbeat;
	fn_acts[SIGUSR1] = wdg_heartbeat;
}
#endif

/* sw_mode */
#ifdef RTCONFIG_SWMODE_SWITCH
// copied from 2.x code
int pre_sw_mode=0, sw_mode=0;
int flag_sw_mode=0;
int tmp_sw_mode=0;
int count_stable=0;

void swmode_check()
{
#if defined(PLAC66U)
	int pre_sw_mode=sw_mode();

	if (!button_pressed(BTN_SWMODE_SW_ROUTER))
		sw_mode = SW_MODE_ROUTER;
	else    sw_mode = SW_MODE_AP;

	if ((sw_mode != pre_sw_mode) && !flag_sw_mode) {
		if( sw_mode == SW_MODE_ROUTER )
			_dprintf("[%s], switch to ROUTER Mode!\n", __FUNCTION__);
		else
			_dprintf("[%s], switch to AP Mode!\n", __FUNCTION__);

		flag_sw_mode=1;
		count_stable=0;
	}
	else if (flag_sw_mode == 1 && nvram_invmatch("asus_mfg", "1")) {
		if (sw_mode != pre_sw_mode) {
			if (++count_stable>4) { // stable for more than 5 second
				dbg("Reboot to switch sw mode ..\n");
				flag_sw_mode=0;
				/* sw mode changed: restore defaults */
				led_control(LED_POWER, LED_OFF);
				alarmtimer(0, 0);
				nvram_set("restore_defaults", "1");
				if (notify_rc_after_wait("resetdefault")) {	/* Send resetdefault rc_service failed. */
					alarmtimer(NORMAL_PERIOD, 0);
				}
			}
		}
		else flag_sw_mode = 0;
	}
#else
	if (!nvram_get_int("swmode_switch")) return;
	pre_sw_mode = sw_mode;

	if (button_pressed(BTN_SWMODE_SW_REPEATER))
		sw_mode=SW_MODE_REPEATER;
	else if (button_pressed(BTN_SWMODE_SW_AP))
		sw_mode=SW_MODE_AP;
	else sw_mode=SW_MODE_ROUTER;

	if (sw_mode != pre_sw_mode) {
		if (sw_mode() != sw_mode) {
			flag_sw_mode=1;
			count_stable=0;
			tmp_sw_mode=sw_mode;
		}
		else flag_sw_mode=0;
	}
	else if (flag_sw_mode == 1 && nvram_invmatch("asus_mfg", "1")) {
		if (tmp_sw_mode == sw_mode) {
			if (++count_stable>4) // stable for more than 5 second
			{
				dbg("Reboot to switch sw mode ..\n");
				flag_sw_mode=0;
				sync();
				/* sw mode changed: restore defaults */
				nvram_set("nvramver", "0");
				nvram_commit();
				reboot(RB_AUTOBOOT);
			}
		}
		else flag_sw_mode = 0;
	}
#endif	/* Model */
}
#endif	/* RTCONFIG_SWMODE_SWITCH */
#ifdef WEB_REDIRECT
void wanduck_check(void)
{
	if ((freeze_duck_count = nvram_get_int("freeze_duck")) > 0)
		nvram_set_int("freeze_duck", --freeze_duck_count);
}
#endif

#if (defined(PLN12) || defined(PLAC56) || defined(PLAC66U))
static int client_check_cnt = 0;
static int no_client_cnt = 0;
static int plc_wake = 1;

static void client_check(void)
{
	#define WIFI_STA_LIST_PATH "/tmp/wifi_sta_list"
	FILE *fp;
	char word[256], *next, ifnames[128];
	char buf[512];
	int i, n = 0;

	if (!nvram_match("plc_ready", "1"))
		return;

	/* every 2 seconds check */
	if (client_check_cnt++ > 0) {
		client_check_cnt = 0;
		return;
	}
	//dbg("%s:\n", __func__);

	/* check wireless client */
	i = 0;
	strcpy(ifnames, nvram_safe_get("wl_ifnames"));
	foreach(word, ifnames, next) {
		SKIP_ABSENT_BAND_AND_INC_UNIT(i);
		doSystem("wlanconfig %s list > %s", get_wifname(i), WIFI_STA_LIST_PATH);

		fp = fopen(WIFI_STA_LIST_PATH, "r");
		if (!fp) {
			dbg("%s: fopen fail\n", __func__);
			return;
		}
		while (fgets(buf, 512, fp) != NULL)
			n++;
		fclose(fp);
		unlink(WIFI_STA_LIST_PATH);

		++i;
	}

	/* check wire port */
	if (get_lanports_status())
		n++;

	if (n == 0)
		no_client_cnt++;
	else
		no_client_cnt = 0;

	if (plc_wake == 1 && no_client_cnt >= 5) {
		//dbg("%s: trigger Powerline to sleep...\n", __func__);
#if defined(PLN12)
		doSystem("swconfig dev %s port 1 set power 0", MII_IFNAME);
#elif defined(PLAC56)
		set_gpio((nvram_get_int("plc_wake_gpio") & 0xff), 1);
#else
#warning TBD: PL-AC66U...
#endif
		plc_wake = 0;
		nvram_set("plc_wake", "0");
	}
	else if (plc_wake == 0 && no_client_cnt == 0) {
		//dbg("%s: trigger Powerline to wake...\n", __func__);
#if defined(PLN12)
		doSystem("swconfig dev %s port 1 set power 1", MII_IFNAME);
#elif defined(PLAC56)
		set_gpio((nvram_get_int("plc_wake_gpio") & 0xff), 0);
#else
#warning TBD: PL-AC66U...
#endif
		plc_wake = 1;
		nvram_set("plc_wake", "1");
	}

	if (no_client_cnt >= 5)
		no_client_cnt = 0;
}
#endif

void regular_ddns_check(void)
{
#ifdef RPAC68U
/* The workaround solution avoiding watchdog segfault on RP-AC68U. */
	int wan_unit = rtk_wan_primary_ifunit(), last_unit = nvram_get_int("ddns_last_wan_unit");
#else
	int wan_unit = wan_primary_ifunit(), last_unit = nvram_get_int("ddns_last_wan_unit");
#endif
	char prefix[sizeof("wanXXXXXXXXXX_")], *host;
	struct in_addr ip_addr;
	struct hostent *hostinfo;

	//_dprintf("regular_ddns_check...\n");

	host = get_ddns_hostname();
	if (*host == '\0')
		return;

	hostinfo = gethostbyname(host);
	ddns_check_count = 0;
	if (!hostinfo)
		return;

#if defined(RTCONFIG_DUALWAN)
	if (nvram_match("wans_mode", "lb")) {
		int ddns_wan_unit = nvram_get_int("ddns_wan_unit");

		if (ddns_wan_unit >= WAN_UNIT_FIRST && ddns_wan_unit < WAN_UNIT_MAX) {
			wan_unit = ddns_wan_unit;
		} else {
			int u = get_first_connected_public_wan_unit();
			if (u < WAN_UNIT_FIRST || u >= WAN_UNIT_MAX)
			{
				u = get_first_connected_dual_wan_unit(); /* can use private WAN IP */
				if (u < WAN_UNIT_FIRST || u >= WAN_UNIT_MAX)
				{
					logmessage("DDNS", "[%s] None of wan can connect to internet in load balance Dual WAN.", __FUNCTION__);
					return;
				}
			}
			wan_unit = u;
		}
	}
#endif

	if (!nvram_match("wans_mode", "lb") && !is_wan_connect(wan_unit))
		return;

	// Only check nvram IP for internal IP check mode
	if (nvram_get_int("ddns_realip_x") == 0) {
		snprintf(prefix, sizeof(prefix), "wan%d_", wan_unit);
		ip_addr.s_addr = *(unsigned long *)hostinfo -> h_addr_list[0];
		//_dprintf("%s ?= %s\n", nvram_pf_get(prefix, "ipaddr"), inet_ntoa(ip_addr));
		if (nvram_pf_match(prefix, "ipaddr", inet_ntoa(ip_addr)))
			return;
	}
	
	//_dprintf("WAN IP change!\n");
	if (wan_unit != last_unit) {
#ifndef RTCONFIG_INADYN
		unlink("/tmp/ddns.cache");
#else
		system("rm -f /var/cache/inadyn/*.cache");
#endif
	}
	logmessage("watchdog", "Hostname/IP mapping error! Restart ddns.");
	stop_ddns();
	start_ddns(NULL);

	return;
}

void ddns_check(void)
{
#ifdef RPAC68U
/* The workaround solution avoiding watchdog segfault on RP-AC68U. */
	int wan_unit = rtk_wan_primary_ifunit(), last_unit = nvram_get_int("ddns_last_wan_unit");
#else
	int wan_unit = wan_primary_ifunit(), last_unit = nvram_get_int("ddns_last_wan_unit");
#endif

	//_dprintf("ddns_check... %d\n", ddns_check_count);
	if (!nvram_get_int("ddns_enable_x"))
		return;

#if defined(RTCONFIG_DUALWAN)
	if (nvram_match("wans_mode", "lb")) {
		int ddns_wan_unit = nvram_get_int("ddns_wan_unit");

		if (ddns_wan_unit >= WAN_UNIT_FIRST && ddns_wan_unit < WAN_UNIT_MAX) {
			wan_unit = ddns_wan_unit;
		} else {
			int u = get_first_connected_public_wan_unit();
			if (u < WAN_UNIT_FIRST || u >= WAN_UNIT_MAX)
			{
				u = get_first_connected_dual_wan_unit(); /* can use private WAN IP */
				if (u < WAN_UNIT_FIRST || u >= WAN_UNIT_MAX)
				{
					logmessage("DDNS", "[%s] None of wan can connect to internet in load balance Dual WAN.", __FUNCTION__);
					return;
				}
			}
			wan_unit = u;
		}
	}
#endif

	if (!nvram_match("wans_mode", "lb") && !is_wan_connect(wan_unit))
		return;

	/* Check existence of ddns daemon
	 * if and only if last WAN unit is equal to new WAN unit.
	 */
	if (last_unit == wan_unit) {
#ifndef RTCONFIG_INADYN
		if (pids("ez-ipupdate"))	//ez-ipupdate is running!
			return;
#else
		if (pids("inadyn"))		//inadyn is running!
			return;
#endif
		if (pids("phddns"))		//phddns is running!
			return;
	}

	if (nvram_match("ddns_regular_check", "1")&& !nvram_match("ddns_server_x", "WWW.ASUS.COM") && !nvram_match("ddns_server_x", "WWW.ASUS.COM.CN")) {
		int period = nvram_get_int("ddns_regular_period"); //minute(s)
		if (period < 30) period = 60;
		if (ddns_check_count >= (period*2)) {
			regular_ddns_check();
			ddns_check_count = 0;
			return;
		}
		ddns_check_count++;
	}

	if (wan_unit == last_unit && nvram_match("ddns_updated", "1")) { //already updated success
#if defined(RTCONFIG_IPV6) && defined(RTCONFIG_INADYN)
		/* not enable IPv6 or IPv6 already updated success */
		if (!ipv6_enabled() || nvram_match("ddns_ipv6_update", "0") || (ipv6_enabled() && nvram_match("ddns_ipv6_update", "1") && nvram_match("ddns_ipv6_updated", "1"))) {
			//logmessage("watchdog", "IPv4/IPv6 already updated success, exit DDNS Retry.\n");
			return;
		}
#else
		return;
#endif
	}

	if (wan_unit == last_unit) { /* DDNS has already been run, ddns_last_wan_unit has been set. */
		if ( nvram_match("ddns_server_x", "WWW.ASUS.COM") || nvram_match("ddns_server_x", "WWW.ASUS.COM.CN")) {
			/* Only Time-out, connect_fail, -1 (in asusddns) or not auth_fail (not in asusddns) */
			if ( !( !strcmp(nvram_safe_get("ddns_return_code_chk"),"Time-out") ||
				!strcmp(nvram_safe_get("ddns_return_code_chk"),"connect_fail") ||
				strstr(nvram_safe_get("ddns_return_code_chk"), "-1") ) )
				return;
		}
		else{ //non asusddns service
			if ( !strcmp(nvram_safe_get("ddns_return_code_chk"),"auth_fail") )
				return;
		}
	}

	if (nvram_get_int("ntp_ready") != 1) {
		logmessage("watchdog", "NTP time not sync, exit DDNS Retry.\n");
		return;
	}

	/* MAX Retry Count mechanism */
	int ddns_check_retry = nvram_get_int("ddns_check_retry");
	//logmessage("watchdog", "ddns_check_retry=%d, ddns_recover_min=%d\n", ddns_check_retry, (ddns_recover_min/2));
	ddns_check_retry--;
	/* Stop retry and show the return code to UI */
	if (ddns_check_retry <= 0) {
		/* If in "ddns_query" then continue to next round, otherwise start ddns. */
		if (nvram_match("ddns_return_code", "ddns_query")) {
			nvram_set("ddns_return_code", nvram_safe_get("ddns_return_code_chk"));
			ddns_recover_min = ((30 + rand_seed_by_time() % 30)*2);
			logmessage("watchdog", "DDNS Retry reach MAX.(%d), DDNS Recover Time set %d\n", ddns_check_retry, (ddns_recover_min/2));
		}
		if (ddns_recover_min > 0) {
			ddns_recover_min--;
			return;
		}
		else {
			logmessage("watchdog", "DDNS Recover Time reached, recover DDNS Retry.\n");
			ddns_check_retry = 10;
		}
	}
	nvram_set_int("ddns_check_retry", ddns_check_retry);

	if (wan_unit != last_unit) {
#ifndef RTCONFIG_INADYN
		unlink("/tmp/ddns.cache");
#else
		system("rm -f /var/cache/inadyn/*.cache");
#endif
	}
	logmessage("watchdog", "start ddns.");
	stop_ddns();
	start_ddns("watchdog");

	return;
}

void networkmap_check()
{
#ifdef RTCONFIG_WIFI_SON
	if ((sw_mode() == SW_MODE_AP && !nvram_match("cfg_master", "1")) && nvram_match("wifison_ready", "1"))
		return;
#endif
	if (!pids("networkmap"))
		start_networkmap(0);
}

void httpd_check()
{
	if (nvram_get_int("wait_httpd")) {
		nvram_set_int("wait_httpd", 0);
		return;
	}

#ifdef RTCONFIG_HTTPS
	int enable = nvram_get_int("http_enable");
	if ((enable != 1 && !pids("httpd")) ||
	    (enable != 0 && !pids("httpds")))
#else
	if (!pids("httpd"))
#endif
	{
#if defined(RTL_WTDOG)
		stop_rtl_watchdog();
#endif
		logmessage("watchdog", "start httpd");
		//stop_httpd();
		nvram_set("last_httpd_handle_request", nvram_safe_get("httpd_handle_request"));
		nvram_set("last_httpd_handle_request_fromapp", nvram_safe_get("httpd_handle_request_fromapp"));
		nvram_commit();
		start_httpd();
#if defined(RTL_WTDOG)
		start_rtl_watchdog();
#endif
	}
}

void awsiot_check()
{
#ifdef RTCONFIG_AWSIOT
    if (!pids("awsiot"))
        start_awsiot();
#endif
}

#ifdef RTCONFIG_UPNPC_NEW
void renew_ipsec_upnpc_duration(int type)
{
	static int run = 0;
	char cmd[256] = {0};
	int r = 0;
	char protocol[4] = "UDP";
	char duration[16] = "86400";
	char rest_duration[16] = {0};
	char prefix[16] = {0};
	char wan_ifname[16] = {0};
	char wan_ipaddr[32] = {0};
	upnpc_list_t target;

	if((type != UPNPC_TYPE_UDP) && (type != UPNPC_TYPE_TCP))
	{
		_dprintf("[%s]Wrong type:[%d]\n", __FUNCTION__, type);
		return;
	}

	memset(&target, 0, sizeof(target));
	if(type == UPNPC_TYPE_TCP)
	{
		snprintf(protocol, sizeof(protocol), "%s", "TCP");
	}

	if(run == 1)
	{
		snprintf(cmd, sizeof(cmd), "miniupnpc-new -m eth0 -i -f %s", UPNPC_WDG_OUTPUT_FILE);
		system(cmd);
		snprintf(prefix, sizeof(prefix), "wan%d_", get_active_wan_unit());
		snprintf(wan_ifname, sizeof(wan_ifname), "%s", nvram_pf_safe_get(prefix, "ifname"));
		snprintf(wan_ipaddr, sizeof(wan_ipaddr), "%s", nvram_pf_safe_get(prefix, "ipaddr"));

		if((strcmp(nvram_safe_get("ipsec_upnpc_duration"), "") != 0) && (nvram_get_int("ipsec_upnpc_duration") != 0))
		{
			snprintf(duration, sizeof(duration), "%s", nvram_safe_get("ipsec_upnpc_duration"));
		}
		snprintf(target.protocol, sizeof(target.protocol), "%s", protocol);
		snprintf(target.intClient, sizeof(target.intClient), "%s", wan_ipaddr);
		snprintf(target.extPort, sizeof(target.extPort), "%d", nvram_get_int("ipsec_isakmp_ext_port"));
		snprintf(target.intPort, sizeof(target.intPort), "%d", nvram_get_int("ipsec_isakmp_port"));

		r = scan_upnpclist(UPNPC_WDG_OUTPUT_FILE, target, rest_duration, sizeof(rest_duration));
		if(r == 1)
		{
			if(nvram_get_int("ipsec_upnpc_debug") == 1)
			{
				_dprintf("duration_isakmp=[%s]\n", rest_duration);
			}
			if(atoll(rest_duration) < 3600)
			{
				snprintf(cmd, sizeof(cmd), "miniupnpc-new -m %s -i -a %s %d %d %s %s", wan_ifname, wan_ipaddr, nvram_get_int("ipsec_isakmp_port"), nvram_get_int("ipsec_isakmp_ext_port"), protocol, duration);
				system(cmd);
			}
		}

		r = 0;
		snprintf(target.extPort, sizeof(target.extPort), "%d", nvram_get_int("ipsec_nat_t_ext_port"));
		snprintf(target.intPort, sizeof(target.intPort), "%d", nvram_get_int("ipsec_nat_t_port"));
		r = scan_upnpclist(UPNPC_WDG_OUTPUT_FILE, target, rest_duration, sizeof(rest_duration));
		if(r == 1)
		{
			if(nvram_get_int("ipsec_upnpc_debug") == 1)
			{
				_dprintf("duration_esp=[%s]\n", rest_duration);
			}
			if(atoll(rest_duration) < 3600)
			{
				snprintf(cmd, sizeof(cmd), "miniupnpc-new -m %s -i -a %s %d %d %s %s", wan_ifname, wan_ipaddr, nvram_get_int("ipsec_nat_t_port"), nvram_get_int("ipsec_nat_t_ext_port"), protocol, duration);
				system(cmd);
			}
		}
		run = 0;
	}
	else
	{
		run = 1;
	}
}
#endif /* RTCONFIG_UPNPC_NEW */

#ifdef RTCONFIG_LANTIQ
int need_to_restart_wifi_bak(void)
{
	static int not_ready_count = 0;

	if (nvram_get_int("wave_ready") == 1){
		if(!pids("wave_monitor")){
			return 1;
		}

		if( (client_mode() || aimesh_re_mode()) ){
		}else{
			if(nvram_get_int("wl0_radio") == 1 &&
					is_if_up("wlan0") != 1){
				return 1;
			}
			if(nvram_get_int("wl1_radio") == 1 &&
				is_if_up("wlan2") != 1){
				return 1;
			}
		}
	}

	if(nvram_get_int("check_wave_ready") == -1){
		nvram_unset("check_wave_ready");
		return 1;
	}

	if(nvram_get_int("wave_ready") == 0){
		if(!pids("wave_monitor")){
			/* wave_ready = 0 and cannot trigger restart_wireless case */
			if(not_ready_count > 10){
				_dprintf("[%s][%d] count down to reload_mtlk:[%d]\n",
					__func__, __LINE__, not_ready_count);
				not_ready_count = 0;
				return 1;
			}else{
				not_ready_count++;
			}
		}
		if(nvram_get_int("wave_action_cur") == 0){
			return 1;
		}
	}

	not_ready_count = 0;
	return 0;
}

int need_to_restart_wifi(void)
{
	if(nvram_get_int("unload_mtlk") == 1){
		nvram_unset("unload_mtlk");
		return 1;
	}
}

void wave_monitor_check()
{
	static int drop_caches_check = 0;

	if (need_to_restart_wifi()){
		while(pidof("wave_monitor") > 0){
			system("kill -9 `pidof wave_monitor`");
		}
		unload_mtlk();
		logmessage("watchdog", "restart wave_monitor");
		nvram_set("wave_unload_mtlk", "1");
		nvram_unset("wave_CFG");
		_dprintf("[%s][%d] start to unload_mtlk\n", __func__, __LINE__);
		sleep(1);
		start_wave_monitor();
		sleep(5);
		nvram_unset("wave_unload_mtlk");
		_dprintf("[%s][%d] unload_mtlk finished\n", __func__, __LINE__);
	}
	if (!pids("wave_monitor")){
		nvram_set("wave_action", "3");
		nvram_set("wave_CFG", "1");
		logmessage("watchdog", "restart wave_monitor");
		killall_tk("wave_monitor");
		start_wave_monitor();
	}
	if(drop_caches_check < 4){
		f_write_string("/proc/sys/vm/drop_caches", "1", 0, 0);
		drop_caches_check++;
	}else{
		drop_caches_check = 0;
	}
}
#endif

#if defined(RTCONFIG_BCM_7114)
void check_dnsmasq_logs(char *log_path)
{
	if(log_path && check_filesize_over(log_path, 20*1024)) {
		syslog(LOG_NOTICE, "remove dnsmasq logs due too big, restart dnsmasq for new logs\n");
		eval("rm", "-f", log_path);
		stop_dnsmasq();
		sleep(1);
		start_dnsmasq();
	}
}
#endif

void dnsmasq_check()
{
	if (nvram_get_int("asus_mfg") == 1)
		return;

	if (!is_routing_enabled()
#ifdef RTCONFIG_WIRELESSREPEATER
		&& sw_mode() != SW_MODE_REPEATER
#endif
	)
		return;

#if defined(RTCONFIG_BCM_7114)
	if(nvram_match("dhcp_logs", "1")) 
		check_dnsmasq_logs("/tmp/dhcp.logs");
#endif

	if (!pids("dnsmasq")) {
#if defined(RTL_WTDOG)
		stop_rtl_watchdog();
#endif
		start_dnsmasq();
		TRACE_PT("watchdog: dnsmasq died. start dnsmasq...\n");

#if defined(RTL_WTDOG)
		start_rtl_watchdog();
#endif
	}
#ifdef RTCONFIG_DNSPRIVACY
	else if (nvram_get_int("dnspriv_enable") && !pids("stubby")) {
#if defined(RTL_WTDOG)
		stop_rtl_watchdog();
#endif
		start_stubby();
		TRACE_PT("watchdog: stubby died. start stubby...\n");

#if defined(RTL_WTDOG)
		start_rtl_watchdog();
#endif
	}
#endif
}

#if defined(RPAX56)
void amas_linkctrl()
{
        char xif[256]={0}, *next = NULL;
        int wan_state;
	int bktime = nvram_get_int("bktime")?:5;

	if(dpsta_mode() && !nvram_match("x_Setting", "0")) {
        	if( nvram_get_int("cfg_alive")==1 && *nvram_safe_get("amas_ifname") &&  strstr(nvram_safe_get("sta_phy_ifnames"), nvram_safe_get("amas_ifname")) && !nvram_match("aet_ctrl", "1") ) {
			_dprintf("%s phypower off:\n", __func__);
                        foreach(xif, nvram_safe_get("lan_ifnames"), next) {
				if(!strstr(nvram_safe_get("wl_ifnames"), xif))
					eth_phypower(xif, 0);
			}
			_dprintf("%s phypower on after %d secs:\n", __func__, bktime);
			sleep(bktime);
                        foreach(xif, nvram_safe_get("lan_ifnames"), next) {
				if(!strstr(nvram_safe_get("wl_ifnames"), xif))
					eth_phypower(xif, 1);
			}

                        nvram_set("aet_ctrl", "1");
                }
        }
}
#endif

#ifdef RTCONFIG_NEW_USER_LOW_RSSI
void roamast_check()
{
	char prefix[]="wlXXXXXX_", tmp[64];
	char word[256], *next;
	int unit = 0, rast = 0;
#if defined(RTCONFIG_STA_AP_BAND_BIND) || defined(RTCONFIG_FORCE_ROAMING)
	/* if sta_binding or force roaming is enabled, roamast always runs */
	rast = 1;
	if (rast && !pids("roamast"))
		start_roamast();
	return;
#endif
	foreach (word, nvram_safe_get("wl_ifnames"), next) {
		snprintf(prefix, sizeof(prefix), "wl%d_", unit);

		if (nvram_get_int(strcat_r(prefix, "user_rssi", tmp)) != 0) {
			rast = 1;
			break;
		}
		else {
			unit++;
			continue;
		}
	}

	if (rast && !pids("roamast"))
		start_roamast();
}
#endif

#if ! (defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK))
void watchdog_check()
{
#if defined(RTCONFIG_RALINK) || defined(RTCONFIG_QCA) || defined(RTCONFIG_BCM_7114)
	/* Do nothing. */
#else
	if (!pids("watchdog")) {
		if (nvram_get_int("upgrade_fw_status") == FW_INIT) {
			nvram_set("sys_reboot_reason", "wdg_gone");
			logmessage("watchdog02", "no wathdog, restarting");
			kill(1, SIGTERM);
		}
	}
	return;
#endif
}
#endif /* ! (RTCONFIG_QCA || RTCONFIG_RALINK) */

#if defined(RTCONFIG_QCA) && defined(RTCONFIG_WIGIG)
#define WIGIG_TEMP_PRINT_INTERVAL	(30)		/* seconds */
#define WIGIG_TEMP_CHECK_INTERVAL	(3 * 60)	/* seconds */
void wigig_temperatore_check(void)
{
	static double t_mac_avg = 0, t_radio_avg = 0;
	static long chk_t1 = 0, prn_t1 = 0;
	double t_mac = 0, t_radio = 0;
	char buf[48], *p;
	long t2;
	int r;

	t2 = uptime();
	if (chk_t1 != 0 && (t2 - chk_t1) < WIGIG_TEMP_CHECK_INTERVAL)
		return;

	/* Example: cat /sys/kernel/debug/ieee80211/phy0/wil6210/temp
	 * T_mac   = 73.089
	 * T_radio = 91.875
	 */
	chk_t1 = t2;
	r = f_read_string("/sys/kernel/debug/ieee80211/phy2/wil6210/temp", buf, sizeof(buf));
	if (r < 32)
		return;
	if (!(p = strstr(buf, "T_mac")) || (r = sscanf(p, "T_mac = %lf", &t_mac)) != 1)
		return;
	if (!(p = strstr(buf, "T_radio")) || (r = sscanf(p, "T_radio = %lf", &t_radio)) != 1)
		return;

	if (t_mac_avg != 0)
		t_mac_avg = (t_mac_avg + t_mac) / 2;
	else
		t_mac_avg = t_mac;

	if (t_radio_avg != 0)
		t_radio_avg = (t_radio_avg + t_radio) / 2;
	else
		t_radio_avg = t_radio;

	if (t_mac < 75 && t_mac_avg < 75 && t_radio < 75 && t_radio_avg < 75)
		return;
	if (prn_t1 != 0 && (t2 - prn_t1) < WIGIG_TEMP_PRINT_INTERVAL)
		return;
	dbg("WiGig temperature: MAC %.2lf (avg %.2lf), Radio %.2lf (avg %.2lf)\n",
		t_mac, t_mac_avg, t_radio, t_radio_avg);
	logmessage("WiGig temperature", "MAC %.2lf (avg %.2lf), Radio %.2lf (avg %.2lf)\n",
		t_mac, t_mac_avg, t_radio, t_radio_avg);
	prn_t1 = t2;
}
#endif	/* RTCONFIG_QCA && RTCONFIG_WIGIG */

#ifdef RTAC87U
void qtn_module_check(void)
{
	int ret;
	uint32_t channel;
	int channel_nvram;
	static int waiting = 0;
	static int failed = 0;
	char src_ip[16];
	char dst_ip[16];

	snprintf(src_ip, sizeof(src_ip), "%s", nvram_safe_get("QTN_RPC_CLIENT"));
	snprintf(dst_ip, sizeof(dst_ip), "%s", nvram_safe_get("QTN_RPC_SERVER"));

//	if (ATE_BRCM_FACTORY_MODE())
//		return;

	if (!nvram_get_int("qtn_ready"))
		return;

	if(nvram_get_int("wl1_chanspec") != 0){
		ret = rpc_qcsapi_get_channel(&channel);
		if ( ret == 0 ){
			channel_nvram =
				wf_chspec_ctlchan(wf_chspec_aton(nvram_safe_get("wl1_chanspec")));
			if(channel != channel_nvram){
				rpc_qcsapi_set_channel(nvram_safe_get("wl1_chanspec"));
				fprintf(stderr, "[qtn] current_channel=%d, next_channelActiveTime=%d\n", channel, channel_nvram);
			}
		}else{
				fprintf(stderr, "rpc_qcsapi_get_channel error:[%d][%d]\n", ret, channel);
		}
	}

	if(waiting < 2){
		waiting++;
		return;
	}

	if(icmp_check(src_ip, dst_ip) == 0 ){
		failed++;
		if(failed > 2){
			logmessage("QTN", "QTN connection lost[%s][%s]", src_ip, dst_ip);
			system("reboot &");
		}
	} else {
		failed = 0;
	}
	waiting = 0;

	return;
}

#endif

//#if defined(RTCONFIG_JFFS2LOG) && defined(RTCONFIG_JFFS2)
#if defined(RTCONFIG_JFFS2LOG) && (defined(RTCONFIG_JFFS2)||defined(RTCONFIG_BRCM_NAND_JFFS2))
void syslog_commit_check(void)
{
	struct stat tmp_log_stat, jffs_log_stat;
	int tmp_stat, jffs_stat;
	char prefix[PATH_MAX], path1[PATH_MAX];

	snprintf(prefix, sizeof(prefix), "%s", nvram_safe_get("log_path"));
	snprintf(path1, sizeof(path1), "%s/syslog.log", prefix);

	tmp_stat = stat("/tmp/syslog.log", &tmp_log_stat);
	if (tmp_stat == -1)
		return;

	if (++log_commit_count >= LOG_COMMIT_PERIOD) {
		jffs_stat = stat(path1, &jffs_log_stat);
		if ( jffs_stat == -1) {
			eval("cp", "/tmp/syslog.log", "/tmp/syslog.log-1", prefix);
			return;
		}

		if ( tmp_log_stat.st_size > jffs_log_stat.st_size ||
		     difftime(tmp_log_stat.st_mtime, jffs_log_stat.st_mtime) > 0)
			eval("cp", "/tmp/syslog.log", "/tmp/syslog.log-1", prefix);

		log_commit_count = 0;
	}
	return;
}
#endif

#if defined(RTCONFIG_USB_MODEM)
void modem_log_check(void) {
#define MODEMLOG_FILE "/tmp/usb.log"
#define MAX_MODEMLOG_LINE 3
	FILE *fp;
	char cmd[64], var[16];
	int line = 0;

	if (++log_modem_count >= LOG_MODEM_PERIOD) {
		snprintf(cmd, 64, "cat %s |wc -l", MODEMLOG_FILE);
		if ((fp = popen("cat /tmp/usb.log |wc -l", "r")) != NULL) {
			var[0] = '\0';
			while(fgets(var, 16, fp)) {
				line = safe_atoi(var);
			}
			pclose(fp);

			if (line > MAX_MODEMLOG_LINE) {
				snprintf(cmd, 64, "cat %s |tail -n %d > %s-1", MODEMLOG_FILE, MAX_MODEMLOG_LINE, MODEMLOG_FILE);
				system(cmd);

				snprintf(cmd, 64, "mv %s-1 %s", MODEMLOG_FILE, MODEMLOG_FILE);
				system(cmd);

				snprintf(cmd, 64, "rm -f %s-1", MODEMLOG_FILE);
				system(cmd);
			}
		}

		log_modem_count = 0;
	}
	return;
}

#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS)
void modem_flow_check(int modem_unit) {
	time_t now, start, diff_mon;
	struct tm tm_now, tm_start;
	char timebuf[32];
	int day_cycle;
	int reset;
	int unit;
	int data_save_sec = nvram_get_int("modem_bytes_data_save"), count;
	int debug = nvram_get_int("modem_bytes_data_cycle_debug");
	char tmp2[100], prefix2[32];
	char env_unit[32];

	usb_modem_prefix(modem_unit, prefix2, sizeof(prefix2));

	unit = get_wan_unit(nvram_safe_get(strcat_r(prefix2, "act_dev", tmp2)));
	if (unit == -1)
		return;

	if (!is_wan_connect(unit))
		return;

	if(!nvram_get_int("ntp_ready"))
		return;

	snprintf(env_unit, 32, "unit=%d", modem_unit);
	putenv(env_unit);

	if (data_save_sec == 0)
		data_save_sec = atoi(nvram_default_get("modem_bytes_data_save"));
	count = data_save_sec/30;
	modem_data_save = (modem_data_save+1)%count;
	if (!modem_data_save) {
		if (debug == 1)
			_dprintf("modem_flow_check: save the data usage.\n");
		eval("/usr/sbin/modem_status.sh", "bytes+");
	}

	if (++modem_flow_count >= MODEM_FLOW_PERIOD) {
		time(&now);
		memcpy(&tm_now, localtime(&now), sizeof(struct tm));
		if (debug == 1)
			_dprintf("modem_flow_check: now. year %4d, month %2d, day %2d, hour, %2d, minute %2d.\n", tm_now.tm_year+1900, tm_now.tm_mon+1, tm_now.tm_mday, tm_now.tm_hour, tm_now.tm_min);

		snprintf(timebuf, 32, "%s", nvram_safe_get("modem_bytes_data_start"));
		if (strlen(timebuf) <= 0 || !strcmp(timebuf, "0")) {
			snprintf(timebuf, 32, "%d", (int)now);
			nvram_set("modem_bytes_data_start", timebuf);
			eval("/usr/sbin/modem_status.sh", "bytes-");
		}
		start = strtol(nvram_safe_get("modem_bytes_data_start"), NULL, 10);
		memcpy(&tm_start, localtime(&start), sizeof(struct tm));
		if (debug == 1)
			_dprintf("modem_flow_check: start. year %4d, month %2d, day %2d, hour, %2d, minute %2d.\n", tm_start.tm_year+1900, tm_start.tm_mon+1, tm_start.tm_mday, tm_start.tm_hour, tm_start.tm_min);

		day_cycle = strtol(nvram_safe_get("modem_bytes_data_cycle"), NULL, 10);
		if (debug == 1)
			_dprintf("modem_flow_check: cycle=%d.\n", day_cycle);

		reset = 0;
		if (day_cycle >= 1 && day_cycle <= 31 && tm_now.tm_year+1900 >= 2014) {
			if (!start || tm_start.tm_year+1900 < 2014) {
				_dprintf("Start the cycle of the data count!\n");
				reset = 1;
			}
			else{
				if (tm_now.tm_mday == day_cycle) {
					// ex: day_cycle=20, start=2015/2/15 or 2014/4/30, now=2015/3/20. it should be reset by 2015/3/20, but in fact didn't yet.
					if (tm_now.tm_mday != tm_start.tm_mday || tm_now.tm_mon != tm_start.tm_mon || tm_now.tm_year != tm_start.tm_year)
						reset = 1;
				}
				else if (tm_now.tm_year != tm_start.tm_year) {
					reset = 1;
				}
				else if (tm_now.tm_mon < tm_start.tm_mon) {
					diff_mon = tm_now.tm_mon+12-tm_start.tm_mon;
					// ex: over 2 months. it should be reset, but in fact didn't yet.
					if (diff_mon > 1)
						reset = 1;
					else if (diff_mon == 1) {
						// ex: day_cycle=20, start=12/15, now=1/2. it should be reset by 12/20, but in fact didn't yet.
						// tm_start.tm_mday >= day_cycle: it had been reset.
						if (tm_start.tm_mday < day_cycle)
							reset = 1;
						// ex: day_cycle=20, start=12/25, now=1/21. it should be reset by 1/20, but in fact didn't yet.
						// tm_now.tm_mday < day_cycle: it didn't need to be reset.
						// tm_now.tm_mday == day_cycle: it had been reset by the above codes.
						else if (tm_now.tm_mday > day_cycle)
							reset = 1;
					}
				}
				else if (tm_now.tm_mon > tm_start.tm_mon) {
					diff_mon = tm_now.tm_mon-tm_start.tm_mon;
					// ex: over 2 months. it should be reset, but in fact didn't yet.
					if (diff_mon > 1)
						reset = 1;
					else if (diff_mon == 1) {
						// ex: day_cycle=31, start=2/28, now=3/1. it should be reset by 2/28(31?), but in fact didn't yet.
						// tm_start.tm_mday >= day_cycle: it had been reset.
						if (tm_start.tm_mday < day_cycle)
							reset = 1;
						// ex: day_cycle=20, start=2/25, now=3/21. it should be reset by 3/20, but in fact didn't yet.
						// tm_now.tm_mday < day_cycle: it didn't need to be reset.
						// tm_now.tm_mday == day_cycle: it had been reset by the above codes.
						else if (tm_now.tm_mday > day_cycle)
							reset = 1;
					}
				}
				else{ // tm_now.tm_mon == tm_start.tm_mon
					// ex: day_cycle=20, start=2/15, now=2/28. it should be reset by 2/20, but in fact didn't yet.
					// tm_now.tm_mday < day_cycle: it didn't need to be reset.
					// tm_now.tm_mday == day_cycle: it had been reset by the above codes.
					// tm_start.tm_mday >= day_cycle: it had been reset.
					if (tm_now.tm_mday > day_cycle && tm_start.tm_mday < day_cycle)
						reset = 1;
				}
			}
		}

		if (reset) {
			snprintf(timebuf, 32, "%d", (int)now);
			nvram_set("modem_bytes_data_start", timebuf);
			eval("/usr/sbin/modem_status.sh", "bytes-");
		}

		modem_flow_count = 0;
	}

	unsetenv("unit");

	return;
}
#endif
#endif

#ifdef RTCONFIG_NOTIFICATION_CENTER
void ntevent_intranet_usage_insight()
{
	time_t now;
	struct tm *tm;

	if (!nvram_get_int("ntp_ready")) return;

	char str[32];
	time(&now);
	tm = localtime(&now);

	/* send event at 9:00 each Monday */
	if (tm->tm_wday == 1 && tm->tm_hour == 9 && tm->tm_min == 0) {
		snprintf(str, 32, "0x%x", HINT_INTERNET_USAGE_INSIGHT_EVENT);
		eval("Notify_Event2NC", str, "");
	}
}
#endif

#if defined(RTCONFIG_USB) && defined(RTCONFIG_NOTIFICATION_CENTER) && defined(RTCONFIG_CLOUDSYNC)
static void ntevent_disk_usage_check(){
	char str[32];
	int ntevent_firstdisk = nvram_get_int("ntevent_firstdisk");
	int ntevent_nodiskuse_sec = nvram_get_int("ntevent_nodiskuse_sec");
	int ntevent_nodiskuse = nvram_get_int("ntevent_nodiskuse");
	int link_internet = nvram_get_int("link_internet");
	time_t now;
	unsigned long timestamp;

	if(ntevent_nodiskuse_sec <= 0){
		ntevent_nodiskuse_sec = 86400*3;
		nvram_set("ntevent_nodiskuse_sec", "259200");
		nvram_commit();
	}

	if(link_internet == 2 && !ntevent_nodiskuse && !ntevent_firstdisk){
		time(&now);
		snprintf(str, 32, "%s", nvram_safe_get("ntevent_nodiskuse_start"));
		if(strlen(str) > 0){
			timestamp = strtoll(str, (char **) NULL, 10);
			if(now-timestamp > ntevent_nodiskuse_sec){
				snprintf(str, 32, "0x%x", HINT_USB_CHECK_EVENT);
				eval("Notify_Event2NC", str, "");
				nvram_set("ntevent_nodiskuse", "1");
				nvram_commit();
			}
		}
		else{
			snprintf(str, 32, "%lu", now);
			nvram_set("ntevent_nodiskuse_start", str);
			nvram_commit();
		}
	}
}
#endif

#ifdef RTCONFIG_CFGSYNC
#define CFG_SERVER_PID         "/var/run/cfg_server.pid"
int is_cfg_server_ready()
{
	if (nvram_match("x_Setting", "1") && (nvram_get_int("cfg_recount") > 0) 
		&& pids("cfg_server") && check_if_file_exist(CFG_SERVER_PID)
	)
		return 1;

	return 0;
}
#endif

/* DEBUG DEFINE */
#define FAUPGRADE_DEBUG             "/tmp/FAUPGRADE_DEBUG"

/* DEBUG FUNCTION */

#define FAUPGRADE_DBG(fmt,args...) \
    { \
        char msg[1024]; \
        snprintf(msg, sizeof(msg), "[FAUPGRADE][%s:(%d)]"fmt"", __FUNCTION__, __LINE__, ##args); \
        logmessage("WATCHDOG", "%s",msg); \
        dbg("%s\n",msg); \
        if(f_exists(FAUPGRADE_DEBUG) > 0) { \
                char info[1024]; \
                snprintf(info, sizeof(info), "echo \"%s\" >> /tmp/FAUPGRADE_DEBUG.log", msg); \
                system(info); \
        } \
    }

#ifdef RTCONFIG_FORCE_AUTO_UPGRADE
static void auto_firmware_check()
{
	int periodic_check = 0;
	static time_t periodic_check_timestamp = 0;
	static int period_retry = 0;
	static int bootup_check_period = 3;	//wait 3 times(90s) to check
	static int bootup_check = 1;
#ifndef RTCONFIG_FW_JUMP
	char *datestr[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
	time_t now;
	struct tm local;
	static int rand_hr, rand_min;

	int update_time_hr, update_time_min;	//auto upgrade trigger time from user defined
	int update_enable = 0;
	char update_time_tmp[6];
#endif

#if defined(RTAX58U) || defined(RTAX56U)
	if (!strncmp(nvram_safe_get("territory_code"), "CX", 2))
		return;
#endif
	if (!nvram_get_int("ntp_ready")){
		//FAUPGRADE_DBG("ntp_ready false");
		return;
	}

	if(bootup_check_period > 0){	//bootup wait 90s to check
		bootup_check_period--;
		return;
	}

	time(&now);
	localtime_r(&now, &local);

#ifdef RTCONFIG_AUTO_FW_UPGRADE
	if(get_ASUS_privacy_policy()){
		update_enable = nvram_get_int("webs_update_enable");
	}
	strlcpy(update_time_tmp, nvram_safe_get("webs_update_time"), sizeof(update_time_tmp));
	if(sscanf(update_time_tmp, "%d:%d", &update_time_hr, &update_time_min) != 2){
		update_time_hr = (2 + rand_hr);
		update_time_min = rand_min;
	}
	if( update_enable == 1 && local.tm_hour == update_time_hr && local.tm_min == update_time_min){ //at user defined time to check
		periodic_check = 1;
		period_retry = 0;
		FAUPGRADE_DBG("update_enable : %d , update_time : %d:%d", update_enable, update_time_hr, update_time_min);
	}
	else if( update_enable == 0 && local.tm_hour == (2 + rand_hr) && local.tm_min == rand_min ){ //at 2 am + random offset to check
		periodic_check = 1;
		period_retry = 0;
	}
#else
	if(local.tm_hour == (2 + rand_hr) && local.tm_min == rand_min) //at 2 am + random offset to check
		periodic_check = 1;
		period_retry = 0;
#endif
	//FAUPGRADE_DBG("periodic_check = %d, period_retry = %d, bootup_check = %d", periodic_check, period_retry, bootup_check);
#ifndef RTCONFIG_FW_JUMP
	if (bootup_check || periodic_check || period_retry!=0)
#endif
	{
#if defined(RTCONFIG_ASUSCTRL)
		if (periodic_check)
			asus_ctrl_sku_update();
#endif
#if defined(RTCONFIG_ASD) && !defined(RTCONFIG_ASD_2_1)
		//notify asd to download version file
		if (pids("asd") && (bootup_check || (periodic_check && period_retry == 0)))
		{
			killall("asd", SIGUSR1);
		}
#endif
#ifdef RTCONFIG_TPVPN
		if (bootup_check || (periodic_check && period_retry == 0))
			system("hmavpn update &");
#endif
#ifndef RTCONFIG_FW_JUMP
		if(nvram_get_int("webs_state_dl_error")){
			if(!strncmp(datestr[local.tm_wday], nvram_safe_get("webs_state_dl_error_day"), 3))
				return;
			else
				nvram_set("webs_state_dl_error", "0");
		}

		if (bootup_check)
		{
			bootup_check = 0;
			rand_hr = rand_seed_by_time() % 4;
			rand_min = rand_seed_by_time() % 60;
			FAUPGRADE_DBG("periodic_check AM %d:%d", 2 + rand_hr, rand_min);
#ifdef RTCONFIG_AMAS
			if(nvram_match("re_mode", "1"))
			{
				FAUPGRADE_DBG("do RE webs_update at bootup");
				nvram_set("webs_update_trigger", "WDG_RE_BT");
				eval("/usr/sbin/webs_update.sh");
				return;
			}
#endif
		}

		period_retry = (period_retry+1) % 3;
#endif

		if(!nvram_contains_word("rc_support", "noupdate")){
#if defined(RTL_WTDOG)
			stop_rtl_watchdog();
#endif

#if defined(RTCONFIG_AUTO_FW_UPGRADE) && defined(RTCONFIG_CFGSYNC)
		if(update_enable == 1){
			if(nvram_get_int("re_mode") == 1){
				FAUPGRADE_DBG("DUT in re_mode");
				return;
			}

			if(is_cfg_server_ready()){
				if(uptime() - periodic_check_timestamp > 60){
					FAUPGRADE_DBG("fimrware update check first time");
					periodic_check_timestamp = uptime();
					period_retry = 0;
				}
				else{
					FAUPGRADE_DBG("cfg fimrware update check once");
					return;
				}

				FAUPGRADE_DBG("notify_rc firmware_webs_update");
				notify_rc("firmware_webs_update");
				return;
			}
		}
#endif
		FAUPGRADE_DBG("do webs_update");
		nvram_set("webs_update_trigger", "WDG");
		eval("/usr/sbin/webs_update.sh");

#if defined(RTL_WTDOG)
			start_rtl_watchdog();
#endif
		}
#ifdef RTCONFIG_DSL
		eval("/usr/sbin/notif_update.sh");
#endif

		if (nvram_get_int("webs_state_update")
				&& !nvram_get_int("webs_state_error")
				&& !nvram_get_int("webs_state_dl_error")
				&& strlen(nvram_safe_get("webs_state_info"))
				)
		{
			FAUPGRADE_DBG("retrieve firmware information");

			if (!get_chance_to_control()){
				FAUPGRADE_DBG("user in use");
				return;
			}

#ifndef RTCONFIG_FW_JUMP
			if (nvram_match("x_Setting", "0")){
				FAUPGRADE_DBG("default status");
				return;
			}
#endif

			if(uptime() - periodic_check_timestamp > 60){
				FAUPGRADE_DBG("fimrware update check first time");
				periodic_check_timestamp = uptime();
				period_retry = 0; //stop retry
			}
			else{
				FAUPGRADE_DBG("fimrware update check once");
				return;
			}

#ifdef RTCONFIG_FORCE_AUTO_UPGRADE
			if(
			nvram_get_int("webs_state_flag") == 2
#ifdef RTCONFIG_AUTO_FW_UPGRADE
			|| (update_enable == 1 && nvram_get_int("webs_state_flag") == 1)
#endif
			)
			{
				nvram_set("webs_state_dl", "1");

				notify_rc_and_wait("stop_upgrade;start_webs_upgrade");

				nvram_set("webs_state_dl", "0");

				if (nvram_get_int("webs_state_dl_error"))
				{
					FAUPGRADE_DBG("error execute upgrade script");
					reboot(RB_AUTOBOOT);
					return;
				}
			}
#endif
			FAUPGRADE_DBG("no need to upgrade firmware");
			return;
		}
		else{
			FAUPGRADE_DBG("could not retrieve firmware information: webs_state_update = %d, webs_state_error = %d, webs_state_dl_error = %d, webs_state_info.len = %d", nvram_get_int("webs_state_update"), nvram_get_int("webs_state_error"), nvram_get_int("webs_state_dl_error"), (unsigned int)strlen(nvram_safe_get("webs_state_info")));
		}		

		return;
	}

}
#endif

#if defined(RTCONFIG_LP5523) || defined(RTCONFIG_FIXED_BRIGHTNESS_RGBLED)
#define FILE_LP5523 "/tmp/lp5523_log"
static void link_pap_status()
{
	FILE *fp;
	int count_point = 100;
	int link_pap_status = 0;
#if defined(RTCONFIG_WIFI_SON)
	int wifison_ready = nvram_get_int("wifison_ready");
#else
	int wifison_ready = 0;
#endif
	int sw_mode = nvram_get_int("sw_mode");
	int cfg_master = nvram_get_int("cfg_master");
	int prelink_pap_status = nvram_get_int("prelink_pap_status");
	int alive = nvram_get_int("cfg_alive");
#if defined(RTCONFIG_AMAS)
	int link_quality = 0;
	int wlc_band = 0;
#if defined(RTCONFIG_BHCOST_OPT)
	char amas_wlc_state[] = "amas_wlcXXX_state";
#endif
#endif
#ifdef RTCONFIG_ETHBACKHAUL
	static int is_ethbh = 0;
#endif
#if defined(RTCONFIG_LP5523)
	int brightness_flag = 1000;
#endif

	if (nvram_get("prelink_pap_status")==NULL || !nvram_match("x_Setting", "1")
#if defined(RTCONFIG_BT_CONN)
			|| pids("bluetoothd")
#endif
		)
		return;

	if (prelink_pap_status == -1) {
		if (!f_exists(FILE_LP5523)) {
			fp = fopen(FILE_LP5523, "w+");
			fclose(fp);

			if ( (wifison_ready && sw_mode==SW_MODE_AP) || (!wifison_ready && nvram_match("re_mode", "1")) // WifiSon/AiMesh Re
				|| sw_mode==SW_MODE_REPEATER || mediabridge_mode()
			) {
				prelink_pap_status = count_point + 180;
			}
			else {
				prelink_pap_status = count_point + 20;
			}
		}
	}

#if defined(RTCONFIG_AMAS)
#if defined(RTCONFIG_LP5523)
	if (prelink_pap_status > brightness_flag)
		prelink_pap_status = prelink_pap_status % brightness_flag;
#endif

	if ( sw_mode != SW_MODE_ROUTER) {
#if defined(RTCONFIG_BHCOST_OPT)
		wlc_band = get_sta_ifname_unit(nvram_safe_get("amas_ifname"));
		snprintf(amas_wlc_state, sizeof(amas_wlc_state),"amas_wlc%d_state", wlc_band);
#else
		wlc_band = nvram_get_int("wlc_band");
#endif
	}
#endif

	if ((wifison_ready && (sw_mode==SW_MODE_ROUTER || (sw_mode==SW_MODE_AP && cfg_master==1))) //WifiSon Cap
		|| (!wifison_ready && cfg_master==1) // AiMesh Cap
	) {
		if (nvram_match("link_internet", "2")) {
			link_pap_status = 1;
		}

#if defined(RTCONFIG_AMAS)
		if(alive || prelink_pap_status>count_point) {
			if (alive) {
				nvram_set_int("cfg_alive", 0);
				prelink_pap_status = count_point + 60;
#if defined(RTCONFIG_LP5523)
				lp55xx_leds_proc(LP55XX_AMAS_REJOIN_LDES, LP55XX_ACT_NONE);
#elif defined(RTCONFIG_FIXED_BRIGHTNESS_RGBLED)
				set_rgbled(RGBLED_RE_JOIN);
#endif
			}
			link_pap_status = -1;
		}
#endif
	}
	else if (wifison_ready && sw_mode==SW_MODE_AP && cfg_master!=1) { //WifiSon Re
#ifdef RTCONFIG_ETHBACKHAUL
		is_ethbh = nvram_get_int("eth_backl");
		if (is_ethbh) {
			link_pap_status = 1;
		}
		else
#endif
		{
			if (nvram_get_int("re_syncing")) //do not check wifi-status when CAP sync with RE
				return;

			if (!alive && (prelink_pap_status==-1 || prelink_pap_status>count_point))
				link_pap_status = 0;
			else
				link_pap_status = (int)getPapState(1)==2?1:((int)getPapState(0)==2?2:0);
		}

	}
#if defined(RTCONFIG_AMAS)
	else if (!wifison_ready && nvram_match("re_mode", "1")) { //Aimesh Re
#if defined(RTCONFIG_QCA953X) || \
    defined(RTCONFIG_QCA956X) || \
    defined(RTCONFIG_QCN550X)
		if (strncmp(nvram_safe_get("amas_ifname"),"vlan",4)==0)
#else
		if (strncmp(nvram_safe_get("amas_ifname"),"eth",3)==0)
#endif
		{
#if defined(RTCONFIG_QCA_PLC_UTILS) || defined(RTCONFIG_QCA_PLC2)
			if (strncmp(nvram_safe_get("amas_ifname"),"eth1",4)==0 && nvram_get_int("autodet_plc_state")!=0 && nvram_get_int("autodet_plc_tx")<100 ) {
				link_pap_status = 2;
			}
			else
#endif
			{
				link_pap_status = 1;
			}
		}
		else {
#if defined(RTCONFIG_BHCOST_OPT)
			if (wlc_band>-1 && nvram_get_int(amas_wlc_state)==WLC_STATE_CONNECTED)
#else
			if (nvram_get_int("wlc_state")==WLC_STATE_CONNECTED) 
#endif
			{
				link_pap_status = 1;
				link_quality = getStaXRssi(wlc_band);

				if (link_quality < -80 || wlc_band==0)
					link_pap_status = 2; // Ok or Weak or 2.4G
			}
		}
	}
#endif // RTCONFIG_AMAS
	else if(sw_mode==SW_MODE_REPEATER || mediabridge_mode()) {
		if (prelink_pap_status==-1)
			nvram_set_int("wlc_state", WLC_STATE_STOPPED);
		else if (nvram_get_int("wlc_state")==WLC_STATE_CONNECTED && nvram_get_int("link_ap"))
			link_pap_status = 1;
	}
	else if(sw_mode==SW_MODE_AP) { //Only AP Mode
		link_pap_status = 1;
	}

	if (link_pap_status != prelink_pap_status)
	{
		if (wifison_ready && sw_mode==SW_MODE_AP && cfg_master!=1)
			nvram_unset("lyra_re_dist");

		if (link_pap_status > 0) {	//Connect to PAP or Internet
#if defined(RTCONFIG_ETHBACKHAUL)
			if (is_ethbh) {
#if defined(RTCONFIG_LP5523)
				lp55xx_leds_proc(LP55XX_AMAS_ETH_LINK_LEDS, LP55XX_ACT_NONE);
#elif defined(RTCONFIG_FIXED_BRIGHTNESS_RGBLED)
				set_rgbled(RGBLED_ETH_BACKHAUL);
#endif
			}
			else
#endif
			{
				if (link_pap_status == 1) {
#if defined(RTCONFIG_LP5523)
					lp55xx_leds_proc(LP55XX_LINKCOR_LEDS, LP55XX_ACT_NONE);
#elif defined(RTCONFIG_FIXED_BRIGHTNESS_RGBLED)
					set_rgbled(RGBLED_CONNECTED);
#endif
				}
				else if (link_pap_status == 2) {
#if defined(RTCONFIG_LP5523)
					lp55xx_leds_proc(LP55XX_ORANGE_LEDS, LP55XX_ACT_NONE);
#elif defined(RTCONFIG_FIXED_BRIGHTNESS_RGBLED)
					set_rgbled(RGBLED_WEAK_BACKHAUL);
#endif
				}
			}
		}
		else {		//Disconnect
			if (prelink_pap_status > count_point) {
				link_pap_status = prelink_pap_status-1;

				if (link_pap_status == count_point) {
					link_pap_status = -1;
				}
				else {
					nvram_set_int("prelink_pap_status", link_pap_status);
					return;
				}
			}
			else {
				if ((wifison_ready && (sw_mode==SW_MODE_ROUTER || (sw_mode==SW_MODE_AP && cfg_master==1))) //WifiSon Cap AP mode
					|| (!wifison_ready && sw_mode==SW_MODE_AP && cfg_master==1) // AiMesh Cap AP mode
				) {
#if defined(RTCONFIG_LP5523)
					lp55xx_leds_proc(LP55XX_AMAS_CAPAP_LEDS, LP55XX_ACT_NONE);
#elif defined(RTCONFIG_FIXED_BRIGHTNESS_RGBLED)
					set_rgbled(RGBLED_AP_MODE_CONNECTED);
#endif
				}
				else if ((wifison_ready && sw_mode==SW_MODE_AP) || (!wifison_ready && nvram_match("re_mode", "1"))) { //AiMesh Re
					if (nvram_get_int("amas_eap_bhmode") > 0) { //Re Eth Only
#if defined(RTCONFIG_LP5523)
						lp55xx_leds_proc(LP55XX_DISCONNCOR_LDES, LP55XX_ACT_NONE);
#elif defined(RTCONFIG_FIXED_BRIGHTNESS_RGBLED)
						set_rgbled(RGBLED_DISCONNECTED);
#endif
					}
#if defined(MAPAC2200) || defined(MAPAC1300)
					else if (nvram_match("productid", "MAP-AC2200") || nvram_match("productid", "MAP-AC1300")) { //Lyra only
						if (prelink_pap_status == 1) {
							link_pap_status = count_point + 20;
							lp55xx_leds_proc(LP55XX_AMAS_RE_SYNC_LEDS, LP55XX_ACT_3ON1OFF);
						}
						else {
							lp55xx_leds_proc(LP55XX_DISCONNCOR_LDES, LP55XX_ACT_NONE);
						}
					}
#endif
					else {
#if defined(RTCONFIG_LP5523)
						lp55xx_leds_proc(LP55XX_AMAS_RE_SYNC_LEDS, LP55XX_ACT_3ON1OFF);
#elif defined(RTCONFIG_FIXED_BRIGHTNESS_RGBLED)
						set_rgbled(RGBLED_SYNC_EVENT);
#endif
					}
				}
				else {
#if defined(RTCONFIG_LP5523)
					lp55xx_leds_proc(LP55XX_DISCONNCOR_LDES, LP55XX_ACT_NONE);
#elif defined(RTCONFIG_FIXED_BRIGHTNESS_RGBLED)
					set_rgbled(RGBLED_DISCONNECTED);
#endif
				}
			}
		}
		logmessage("LP55XX", "State info: cur:%5d, pre:%5d", link_pap_status, prelink_pap_status);
		nvram_set_int("prelink_pap_status", link_pap_status);
	}
#if defined(RTCONFIG_AMAS) && defined(RTAC95U)
	else {
		if (link_pap_status) {
			prelink_pap_status = nvram_get_int("prelink_pap_status");
			prelink_pap_status = prelink_pap_status / brightness_flag;
			if (prelink_pap_status < 6) {
				if (prelink_pap_status == 5) {
					lp55xx_leds_proc(LP55XX_ALL_LEDS_OFF, LP55XX_PREVIOUS_STATE);
					logmessage("LP55XX", "Brightness is reduced");
				}

				prelink_pap_status = ((prelink_pap_status+1)*brightness_flag) + link_pap_status;
				nvram_set_int("prelink_pap_status", prelink_pap_status);
			}
		}
	}
#endif
}
#endif

#if defined(RTCONFIG_WIFI_SON)
extern char *get_tds(char *slevel);
extern void gen_spcmd(char *);
static void wifison_check(void)
{
	if (nvram_match("x_Setting", "1")) {
		if (f_exists("/tmp/hyd.conf"))
		{
			static int invalid_state=0;
			pid_t *pidList;
			pid_t *pl;
			int count;
			static int hyd_wake_cnt=0, hyd_reset_cnt=0;
			static long hyd_last_wake_time=0;
			static long log_time=0, five_minute_time=0;
			long uptime_now;
#ifdef RTCONFIG_ETHBACKHAUL
			static int chaos_eth_state=0;
#endif

			uptime_now = uptime();
			if(uptime_now- atol(nvram_get("hyd_cfg_time"))>40)
			{
				pidList = find_pid_by_name("hyd");
				count=0;
				for (pl = pidList; *pl; pl++)
					count++;
				if (count!=1)
					invalid_state++;
				else
					invalid_state=0;
				if (invalid_state >= 2) {
					if (count==0) {
						if ((uptime_now-hyd_last_wake_time)>10) { /* reset */
							hyd_wake_cnt=0;
							hyd_reset_cnt=0;
						}
						hyd_last_wake_time = uptime_now;
						hyd_wake_cnt++;
						if (hyd_wake_cnt > 5) { /* something wrong, regen hyd config */
							hyd_wake_cnt=0;
							hyd_reset_cnt++;
							if (hyd_reset_cnt > 2) { /* serious situation... */
								logmessage("HYD", "hyd cannot startup, reboot!");
								eval("reboot");
							} else {
								char lbuf[40];
								_dprintf("[[[WATCHDOG]]] : reset hyd process!\n");
								/* log some information */
								logmessage("HYD", "==================================");
								system("ifconfig -a | logger -s");
								logmessage("HYD", "==================================");
								system("iwconfig | logger -s");
								logmessage("HYD", "==================================");
								system("cat /tmp/hyd.conf | logger -s");
								logmessage("HYD", "==================================");
								eval("hive_hyd");
								sprintf(lbuf, "%lu", uptime_now-40);
								nvram_set("hyd_cfg_time", lbuf);
							}
						} else {
							_dprintf("[[[WATCHDOG]]] : wakup hyd\n");
							eval("hyd","-C","/tmp/hyd.conf");
						}
					} else {
						int tmp_count;
						for (tmp_count=0; tmp_count<=(count-1); tmp_count++) {
							if (pidList[tmp_count]>1) {
								_dprintf("[[[WATCHDOG]]] : kill duplicated hyd[%d]\n", pidList[tmp_count]);
								kill(pidList[tmp_count], SIGKILL);
							}
						}
					}
				}
				free(pidList);
			}
			if (nvram_get_int("lyra_dbg") && (uptime_now - log_time) > 180) {
				char *tds;
				logmessage("DDDD", "==================================");
				logmessage("meminfo", "==");
				system("cat /proc/meminfo | logger -s");
				logmessage("lsmod", "==");
				system("lsmod | logger -s");
				logmessage("top", "==");
				system("top -b -n 1 -m | logger -s");
				tds = get_tds("s1");
				if (tds) {
					logmessage("hyd", "==");
					f_write_string("/tmp/hyd_tds", tds, 0, 0);
					system("cat /tmp/hyd_tds | logger -s");
					unlink("/tmp/hyd_tds");
					free(tds);
				}
				log_time = uptime_now;
			}
#ifdef RTCONFIG_ETHBACKHAUL
			if (nvram_get_int("chaos_eth_daemon")) {
				if (chaos_eth_state++ > 10) {
					pidList = find_pid_by_name("eth_bh_mon");
					count=0;
					for (pl = pidList; *pl; pl++)
						count++;
					free(pidList);
					if (count==0) {
						pid_t eth_pid;
						char *ethmon[]={"eth_bh_mon", NULL};
						_dprintf("[[[WATCHDOG]]] : wakup eth_bh_mon\n");
						_eval(ethmon, NULL, 0, &eth_pid);
					} else
						nvram_unset("chaos_eth_daemon");
				}
			} else
				chaos_eth_state=0;
#endif
			if ((uptime_now - five_minute_time) >= 300) {
				five_minute_time = uptime_now;
				if(sw_mode() == SW_MODE_ROUTER || nvram_match("cfg_master", "1")) /* CAP */{
					char cmdbuf[16]; /* SCH2G_5G1_5G2 */
					int ch0,ch1,ch2;
					ch0 = get_channel(get_wififname(0));
					ch1 = get_channel(get_wififname(1));
#if defined(RTCONFIG_HAS_5G_2)
					ch2 = get_channel(get_wififname(2));
#else
					ch2 = 0;
#endif
					sprintf(cmdbuf, "SCH%02d_%03d_%03d", ch0, ch1, ch2);
					gen_spcmd(cmdbuf);
				}
			}
		}
	}
}
#endif

#ifdef RTCONFIG_BT_CONN
static void bt_turn_off_service()
{
	char buf[256];
	char *delim=";";
	char *tmp;

#if defined(RTAX56_XD4) || defined(PLAX56_XP4)
	if(nvram_match("HwId", "B") || nvram_match("HwId", "D")){
		/* Slave, no bluetooth */
		return;
	}
#endif
#if defined(RTAX56_XD4)
	if((nvram_match("HwId", "A") && nvram_get_int("BLE_BT") == 99) ||
		(nvram_match("HwId", "C") && nvram_get_int("BLE_BT") == 99)){
		/* Master without BT */
		return;
	}
#endif
	if (!nvram_match("x_Setting", "0"))
		return;

	memset(buf, '\0', sizeof(buf));
	strncpy(buf, nvram_safe_get("bt_turn_off_service"), sizeof(buf));

	if (strlen(buf)) {
		if (strstr(buf, "ble_qis_done")) {
			if (nvram_get_int("bt_turn_off") != 2)
				return;
			nvram_set("x_Setting", "1");
			nvram_set("qis_Setting", "1");
			nvram_commit();
			stop_bluetooth_service();
		}
		nvram_set("w_Setting", "1");

		tmp = strtok(buf, delim);
		while (tmp!=NULL) {
			int retry = 20;

			while(retry-- > 0 && strlen(nvram_safe_get("rc_service"))) {
				_dprintf("\nwaiting for previous service");
				sleep(1);
			}

			if (strncmp(tmp, "ble_qis_done", strlen(tmp))) {
				if (!strncmp(tmp, "restart_wireless", strlen(tmp)) && nvram_invmatch("ipv6_service", "disabled")) {
					notify_rc_and_wait("restart_allnet");
				}
				else
					notify_rc_and_wait(tmp);
			}

			tmp = strtok(NULL, delim);
		}
	}
}
#endif /* RTCONFIG_BT_CONN */

#ifdef RTCONFIG_AMAS
void amas_ctl_check()
{

	if (!nvram_match("start_service_ready", "1"))
		return;

	if (nvram_get_int("re_mode") == 1) {
#if defined(RTCONFIG_QCA)
		if (nvram_get_int("obd_prelinking") == 1)
			return;
#endif
		if (nvram_match("amas_bhctrl_service_ready", "1") && !pids("amas_bhctrl"))
			notify_rc("start_amas_bhctrl");
		if (nvram_match("amas_wlcconnect_service_ready", "1") && !pids("amas_wlcconnect"))
			notify_rc("start_amas_wlcconnect");
		if (nvram_match("amas_lanctrl_service_ready", "1") && !pids("amas_lanctrl"))
			notify_rc("start_amas_lanctrl");
#ifdef RTCONFIG_BHCOST_OPT
		if (nvram_match("amas_status_service_ready", "1") && !pids("amas_status"))
			notify_rc("start_amas_status");
		if (nvram_match("amas_misc_service_ready", "1") && !pids("amas_misc"))
			notify_rc("start_amas_misc");
		if (nvram_match("amas_ssd_service_ready", "1") && !pids("amas_ssd"))
			notify_rc("start_amas_ssd");
#endif
	}
	else
	{
#if defined(RTCONFIG_FRONTHAUL_DWB) || defined(RTCONFIG_VIF_ONBOARDING)
		if (nvram_match("amas_lanctrl_service_ready", "1") && !pids("amas_lanctrl")) {
			if (is_router_mode() || access_point_mode())
				start_amas_lanctrl();
		}
#endif
	}
}

void onboarding_check()
{
	static int bh_selected = 0;
	static int onboarding_count = 0;
	int lock = 0;

	if (!nvram_match("start_service_ready", "1"))
		return;

	if (nvram_get_int("re_mode") == 0)
		return;

	if (strlen(nvram_safe_get("cfg_group")))
		return;

	if (!check_if_dir_exist(CFG_MNT_FOLDER))
		mkdir(CFG_MNT_FOLDER, 0755);

	// If amas backhual selected, reset the onboarding_count for timeout counting of data sync phase.
	if (!bh_selected && nvram_get_int("amas_path_stat") > 0) {
		bh_selected = 1;
		onboarding_count = 0; // reset onboarding_count for obd_data_sync_timeout.
		_dprintf("### onboarding connected(%d), reset onboarding_count ###\n", nvram_get_int("amas_path_stat"));
#ifdef RTCONFIG_LIBASUSLOG
		asusdebuglog(LOG_INFO, CFG_MNT_FOLDER"cfg_dbg.log", LOG_CUSTOM, LOG_SHOWTIME, 0,
			"onboarding connected(%d), reset onboarding_count\n", nvram_get_int("amas_path_stat"));
#endif
	} else {
		_dprintf("### onboarding connected(%d), onboarding_count=[%d] ###\n", nvram_get_int("amas_path_stat"), onboarding_count);
#ifdef RTCONFIG_LIBASUSLOG
		asusdebuglog(LOG_INFO, CFG_MNT_FOLDER"cfg_dbg.log", LOG_CUSTOM, LOG_SHOWTIME, 0,
			"onboarding connected(%d), onboarding_count=[%d]\n", nvram_get_int("amas_path_stat"), onboarding_count);
#endif
	}

	onboarding_count++;

	if ((!bh_selected && onboarding_count > time_mapping.connection_timeout) || 
		(bh_selected && onboarding_count > time_mapping.traffic_timeout)) {

		if (!bh_selected) {
			_dprintf("### onboarding connection timeout, restore to default ###\n");
#ifdef RTCONFIG_LIBASUSLOG
			asusdebuglog(LOG_INFO, CFG_MNT_FOLDER"cfg_dbg.log", LOG_CUSTOM, LOG_SHOWTIME, 0,
				"onboarding connection timeout, restore to default\n");
#endif
		}
		else
		{
			_dprintf("### onboarding traffic timeout, restore to default ###\n");
#ifdef RTCONFIG_LIBASUSLOG
			asusdebuglog(LOG_INFO, CFG_MNT_FOLDER"cfg_dbg.log", LOG_CUSTOM, LOG_SHOWTIME, 0,
				"onboarding traffic timeout, restore to default\n");
#endif
		}

		lock = file_lock("onboarding");
		if (strlen(nvram_safe_get("cfg_group")) == 0) {
			stop_cfgsync();
			notify_rc("resetdefault");
		}
		file_unlock(lock);
	}
}
#endif


#ifdef RTCONFIG_CFGSYNC
void cfgsync_check()
{
	char reboot[sizeof("255")];
	char upgrade[sizeof("255")];

	memset(reboot, 0, sizeof("255"));
	memset(upgrade, 0, sizeof("255"));
	f_read_string("/tmp/reboot", reboot, sizeof(reboot));
	f_read_string("/tmp/upgrade", upgrade, sizeof(upgrade));
	
	if (atoi(reboot) || atoi(upgrade))
		return;

	if (repeater_mode() || mediabridge_mode() || psr_mode())
		return;

#ifdef RTCONFIG_SW_HW_AUTH
#if defined(RTCONFIG_WIFI_SON) 
	if (nvram_match("wifison_ready", "1"))
	{
  		if (nvram_match("x_Setting", "1") && !pids("cfg_client") && !pids("cfg_server"))
		{
			_dprintf("start cfgsync\n");
			notify_rc_and_wait_2min("start_cfgsync");
		}
		return;
	}
#endif

	if (nvram_match("x_Setting", "1") && nvram_match("w_Setting","1") &&
		(
		(!pids("cfg_client") &&
#if defined(RTCONFIG_BCMWL6) && defined(RTCONFIG_PROXYSTA)
				(dpsr_mode()
#ifdef RTCONFIG_DPSTA
				|| dpsta_mode()
#endif
				) &&
#endif
			nvram_get_int("re_mode") == 1
			
#ifdef RTCONFIG_AMAS
			&& ((getAmasSupportMode() & AMAS_RE)
			&& (nvram_get_int("lan_state_t") == LAN_STATE_CONNECTED)
			)
#endif
		) ||
		(!pids("cfg_server") && (is_router_mode() || access_point_mode())
#ifdef RTCONFIG_AMAS
			&& (getAmasSupportMode() & AMAS_CAP)
			
#endif
	)))
	{
		_dprintf("start cfgsync\n");
		notify_rc_and_wait_2min("start_cfgsync");
	}
#endif	/* RTCONFIG_SW_HW_AUTH */
}
#endif /* RTCONFIG_CFGSYNC */

#ifdef RTCONFIG_TUNNEL
void mastiff_check()
{
	if (!pids("mastiff"))
		start_mastiff();
}
#endif /* RTCONFIG_TUNNEL */

#ifdef RTCONFIG_USER_LOW_RSSI
#define ETHER_ADDR_STR_LEN	18

typedef struct wl_low_rssi_count{
	char	wlif[32];
	int	lowc;
}wl_lowr_count_t;

#define		WLLC_SIZE	2
static wl_lowr_count_t wllc[WLLC_SIZE];

void init_wllc(void)
{
	char wlif[128], *next;
	int idx=0;

	memset(wllc, 0, sizeof(wllc));

	foreach (wlif, nvram_safe_get("wl_ifnames"), next)
	{
		SKIP_ABSENT_BAND_AND_INC_UNIT(idx);
		strncpy(wllc[idx].wlif, wlif, 32);
		wllc[idx].lowc = 0;

		idx++;
	}
}

#if defined(RTCONFIG_RALINK)
/* Defined in router/rc/sysdeps/ralink/ralink.c */
#elif defined(RTCONFIG_QCA)
/* Defined in router/rc/sysdeps/qca/qca.c */
#elif defined(RTCONFIG_ALPINE)
/* Defined in router/rc/sysdeps/alpine/alpine.c */
#elif defined(RTCONFIG_LANTIQ)
/* Defined in router/rc/sysdeps/lantiq/lantiq.c */
#else
#define	MAX_STA_COUNT	128
void rssi_check_unit(int unit)
{
	int lrsi = 0, lrc = 0;

	scb_val_t scb_val;
	char ea[ETHER_ADDR_STR_LEN];
	int i, ii;
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	char name[16];
	int val = 0;
	char name_vif[] = "wlX.Y_XXXXXXXXXX";
	struct maclist *mac_list;
	int mac_list_size;

	snprintf(prefix, sizeof(prefix), "wl%d_", unit);

	lrc = nvram_get_int(strcat_r(prefix, "lrc", tmp));
	if (!lrc) lrc = 2;
	if (!(lrsi = nvram_get_int(strcat_r(prefix, "user_rssi", tmp))))
		return;

#if defined(RTCONFIG_BCMWL6) && defined(RTCONFIG_PROXYSTA)
	if (psta_exist_except(unit) || psr_exist_except(unit))
	{
		dbg("%s radio is disabled\n",
			nvram_match(strcat_r(prefix, "nband", tmp), "1") ? "5 GHz" : "2.4 GHz");
		return;
	}
	else if (is_psta(unit) || is_psr(unit))
	{
		dbg("skip interface %s under psta or psr mode\n", nvram_safe_get(strcat_r(prefix, "ifname", tmp)));
		return;
	}
#endif
	snprintf(name, sizeof(name), "%s", nvram_safe_get(strcat_r(prefix, "ifname", tmp)));
	wl_ioctl(name, WLC_GET_RADIO, &val, sizeof(val));
	val &= WL_RADIO_SW_DISABLE | WL_RADIO_HW_DISABLE;

	if (val)
	{
		dbg("%s radio is disabled\n",
			wl_nband_name(nvram_pf_get(prefix, "nband")));
		return;
	}

	if ((repeater_mode() || psr_mode())
		&& (nvram_get_int("wlc_band") == unit))
	{
		sprintf(name_vif, "wl%d.%d", unit, 1);
		strlcpy(name, name_vif, sizeof(name));
	}

	/* buffers and length */
	mac_list_size = sizeof(mac_list->count) + MAX_STA_COUNT * sizeof(struct ether_addr);
	mac_list = malloc(mac_list_size);

	if (!mac_list)
		goto exit;

	memset(mac_list, 0, mac_list_size);

	/* query wl for authenticated sta list */
	strcpy((char*) mac_list, "authe_sta_list");
	if (wl_ioctl(name, WLC_GET_VAR, mac_list, mac_list_size))
		goto exit;

	for (i = 0; i < mac_list->count; i ++) {
		memcpy(&scb_val.ea, &mac_list->ea[i], ETHER_ADDR_LEN);

		if (wl_ioctl(name, WLC_GET_RSSI, &scb_val, sizeof(scb_val_t)))
			continue;

		ether_etoa((void *)&mac_list->ea[i], ea);

		_dprintf("rssi chk.1. wlif (%s), chk ea=%s, rssi=%d(%d), lowr_cnt=%d, lrc=%d\n", name, ea, scb_val.val, lrsi, wllc[unit].lowc, lrc);

		if (scb_val.val < lrsi) {
			_dprintf("rssi chk.2. low rssi: ea=%s, lowc=%d(%d)\n", ea, wllc[unit].lowc, lrc);
			if (++wllc[unit].lowc > lrc) {
				_dprintf("rssi chk.3. deauth ea=%s\n", ea);

				scb_val.val = 8;	// reason code: Disassociated because sending STA is leaving BSS
				wllc[unit].lowc = 0;

				if (wl_ioctl(name, WLC_SCB_DEAUTHENTICATE_FOR_REASON, &scb_val, sizeof(scb_val_t)))
					continue;
			}
		} else
			wllc[unit].lowc = 0;
	}

	for (i = 1; i < MAX_NO_MSSID; i++) {
		if ((repeater_mode() || psr_mode())
			&& (unit == nvram_get_int("wlc_band")) && (i == 1))
			break;

		sprintf(prefix, "wl%d.%d_", unit, i);
		if (nvram_match(strcat_r(prefix, "bss_enabled", tmp), "1"))
		{
			sprintf(name_vif, "wl%d.%d", unit, i);

			memset(mac_list, 0, mac_list_size);

			/* query wl for authenticated sta list */
			strcpy((char*) mac_list, "authe_sta_list");
			if (wl_ioctl(name_vif, WLC_GET_VAR, mac_list, mac_list_size))
				goto exit;

			for (ii = 0; ii < mac_list->count; ii ++) {
				memcpy(&scb_val.ea, &mac_list->ea[ii], ETHER_ADDR_LEN);

				if (wl_ioctl(name_vif, WLC_GET_RSSI, &scb_val, sizeof(scb_val_t)))
					continue;

				ether_etoa((void *)&mac_list->ea[ii], ea);

				_dprintf("rssi chk.1. wlif (%s), chk ea=%s, rssi=%d(%d), lowr_cnt=%d, lrc=%d\n", name_vif, ea, scb_val.val, lrsi, wllc[unit].lowc, lrc);

				if (scb_val.val < lrsi) {
					_dprintf("rssi chk.2. low rssi: ea=%s, lowc=%d(%d)\n", ea, wllc[unit].lowc, lrc);
					if (++wllc[unit].lowc > lrc) {
						_dprintf("rssi chk.3. deauth ea=%s\n", ea);

						scb_val.val = 8;	// reason code: Disassociated because sending STA is leaving BSS
						wllc[unit].lowc = 0;

						if (wl_ioctl(name_vif, WLC_SCB_DEAUTHENTICATE_FOR_REASON, &scb_val, sizeof(scb_val_t)))
							continue;
					}
				} else
					wllc[unit].lowc = 0;
			}
		}
	}

	/* error/exit */
exit:
	if (mac_list) free(mac_list);

	return;
}
#endif
void rssi_check()
{
	int ii = 0;
	char nv_param[NVRAM_MAX_PARAM_LEN];
	char temp[16];
	int wlif_num = num_of_wl_if()?:DEV_NUMIFS;

	if (!nvram_get_int("wlready"))
		return;

	for (ii = 0; ii < wlif_num; ii++) {
		sprintf(nv_param, "wl%d_unit", ii);
		snprintf(temp, sizeof(temp), "%s", nvram_safe_get(nv_param));

		if(strlen(temp) > 0)
			rssi_check_unit(ii);
	}
}
#endif

#ifdef RTCONFIG_WATCH_WLREINIT
void wlcnt_chk()
{
	char cmdbuf[64], buf[16], rbuf[128];
        int i, unit = 0;
        char nv_param[NVRAM_MAX_PARAM_LEN];
        char temp[16], wlif[16];
        char tmp[64], prefix[] = "wlXXXXXXXXXX_";
        static unsigned int pre_val = 0, pre_all[WL_NR_BANDS], watch_prd = 1;
	unsigned int val = 0, tmp_val = 0, val_all[WL_NR_BANDS];
	int fd;
	int wlshoot = nvram_get_int("reinits")?:9;
	int wlshoot_period = nvram_get_int("ws_prd")?:200;

	if(nvram_match("nocnt", "1") || !nvram_get_int("wlready"))
		return;

	memset(cmdbuf, 0, sizeof(cmdbuf));
	memset(&val_all[0], 0, sizeof(val_all));

        for (unit = 0; unit < WL_NR_BANDS; unit++) {
                snprintf(nv_param, sizeof(nv_param), "wl%d_unit", unit);
                snprintf(temp, sizeof(temp), "%s", nvram_safe_get(nv_param));

                if(strlen(temp) > 0){
                        snprintf(prefix, sizeof(prefix), "wl%d_", unit);
                        snprintf(wlif, sizeof(wlif), "%s", nvram_safe_get(strcat_r(prefix, "ifname", tmp)));
			snprintf(cmdbuf, sizeof(cmdbuf), "wl -i %s counters | grep \"reinit \" > /tmp/.wlcnts", wlif);
			system(cmdbuf);

			for(i=0; i<3; ++i) {
				if((fd = open("/tmp/.wlcnts", O_RDONLY)) < 0) {
					usleep(30*1000);
					continue;
				}
				break;
			}
			if(i == 3 && fd < 0) {
				printf("failed to open %s wlreinit file.\n", wlif);
				continue;
			}

			read(fd, rbuf, sizeof(rbuf));
			sscanf(rbuf, "%s %d", buf, &tmp_val);

			if(tmp_val > 0) {
				val += tmp_val;
				val_all[unit] += tmp_val;
			}
			close(fd);
		}
	}
	if(watch_prd++ % wlshoot_period == 0) {
		if(val - pre_val > wlshoot) {
			printf("\nWL go insanity! calm down it\n");
#ifndef RTCONFIG_AHS
			logmessage("watchdog", "detect wl reinit count %d", val - pre_val);
			for(unit = 0; unit < WL_NR_BANDS; ++unit) {
				if(val_all[unit] - pre_all[unit] > 0)
					logmessage("watchdog", "reinit of unit%d:%d", unit, val_all[unit] - pre_all[unit]);
			}
#else
			/* export specific string to syslog for ahsd recover action*/
			logmessage("watchdog", "wl reinit count %d", val - pre_val);
			for(unit = 0; unit < WL_NR_BANDS; ++unit) {
				if(val_all[unit] - pre_all[unit] > 0)
					logmessage("watchdog", "reinit of unit%d:%d", unit, val_all[unit] - pre_all[unit]);
			}

#endif
		}

		pre_val = val;
		for(unit = 0; unit < WL_NR_BANDS; ++unit) {
			pre_all[unit] = val_all[unit];
		}
	}
}
#endif


#if 0 //#ifdef RTCONFIG_TOR
#if (defined(RTCONFIG_JFFS2)||defined(RTCONFIG_BRCM_NAND_JFFS2))
static void Tor_microdes_check() {

	FILE *f;
	char buf[256];
	char *p;
	struct stat tmp_db_stat, jffs_db_stat;
	int tmp_stat, jffs_stat;

	if (++tor_check_count >= TOR_CHECK_PERIOD) {
		tor_check_count = 0;

		jffs_stat = stat("/jffs/.tordb/cached-microdesc-consensus", &jffs_db_stat);
		if (jffs_stat != -1) {
			return;
		}

		tmp_stat = stat("/tmp/.tordb/cached-microdesc-consensus", &tmp_db_stat);
		if (tmp_stat == -1) {
			return;
		}

		if ((f = fopen("/tmp/torlog", "r")) == NULL) return;

		while (fgets(buf, sizeof(buf), f)) {
			if ((p=strstr(buf, "now have enough directory")) == NULL) continue;
			*p = 0;
			eval("cp", "-rfa", "/tmp/.tordb", "/jffs/.tordb");
			break;
		}
		fclose(f);
	}

	return;
}
#endif
#endif

#ifdef RTCONFIG_ERP_TEST
static void ErP_Test() {
	int i;
	int unit = 0;
	char nv_param[NVRAM_MAX_PARAM_LEN];
	char temp[16], name[16];
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	int val = 0;
	struct maclist *mac_list;
	int mac_list_size;
	char name_vif[] = "wlX.Y_XXXXXXXXXX";
	char ifname[32];
	char *next;
	int assoc_count = 0;
	int wlif_num = num_of_wl_if()?:DEV_NUMIFS;

	if (!nvram_get_int("wlready"))
		return;

_dprintf("### ErP Check... ###\n");
	for (unit = 0; unit < wlif_num; unit++) {
		snprintf(nv_param, sizeof(nv_param), "wl%d_unit", unit);
		snprintf(temp, sizeof(temp), "%s", nvram_safe_get(nv_param));

		if(strlen(temp) > 0){
			snprintf(prefix, sizeof(prefix), "wl%d_", unit);
			snprintf(name, sizeof(name), "%s", nvram_safe_get(strcat_r(prefix, "ifname", tmp)));
			wl_ioctl(name, WLC_GET_RADIO, &val, sizeof(val));
			val &= WL_RADIO_SW_DISABLE | WL_RADIO_HW_DISABLE;

			if (val)
			{
				dbg("%s radio is disabled\n",
					wl_nband_name(nvram_pf_get(prefix, "nband")));
				continue;
			}

			/* buffers and length */
			mac_list_size = sizeof(mac_list->count) + 128 * sizeof(struct ether_addr);
			mac_list = malloc(mac_list_size);

			if (!mac_list)
				goto exit;

			memset(mac_list, 0, mac_list_size);

			/* query wl for authenticated sta list */
			strcpy((char*) mac_list, "authe_sta_list");
			if (wl_ioctl(name, WLC_GET_VAR, mac_list, mac_list_size))
				goto exit;

			if (mac_list->count) {
				assoc_count += mac_list->count;
				break;
			}

			for (i = 1; i < MAX_NO_MSSID; i++) {
				sprintf(prefix, "wl%d.%d_", unit, i);
				if (nvram_match(strcat_r(prefix, "bss_enabled", tmp), "1")) {
					sprintf(name_vif, "wl%d.%d", unit, i);

					memset(mac_list, 0, mac_list_size);

					/* query wl for authenticated sta list */
					strcpy((char*) mac_list, "authe_sta_list");
					if (wl_ioctl(name_vif, WLC_GET_VAR, mac_list, mac_list_size))
						goto exit;

					if (mac_list->count) {
						assoc_count += mac_list->count;
						break;
					}
				}
			}
		}
	}//end unit for loop

	if (assoc_count) {
		//Back to Normal
		if (pwrsave_status == MODE_PWRSAVE) {
			_dprintf("ErP: Back to normal mode\n");
			no_assoc_check = 0;

			eval("wl", "-i", "eth2", "radio", "on"); // Turn on 5G-1 radio

			foreach(ifname, nvram_safe_get("wl_ifnames"), next) {
				eval("wl", "-i", ifname, "txchain", "0xf");
				eval("wl", "-i", ifname, "rxchain", "0xf");
				eval("wl", "-i", ifname, "down");
				eval("wl", "-i", ifname, "up");
			}

			pwrsave_status = MODE_NORMAL;
		}
	}
	else {
		//No sta assoc. Enter PWESAVE Mode
		if (pwrsave_status == MODE_NORMAL) {
			no_assoc_check++;
			_dprintf("ErP: no_assoc_check = %d\n", no_assoc_check);

			if (no_assoc_check >= NO_ASSOC_CHECK) {
				_dprintf("ErP: Enter Power Save Mode\n");
				foreach(ifname, nvram_safe_get("wl_ifnames"), next) {
					eval("wl", "-i", ifname, "txchain", "0x1");
					eval("wl", "-i", ifname, "rxchain", "0x1");
					eval("wl", "-i", ifname, "down");
					eval("wl", "-i", ifname, "up");
				}

				eval("wl", "-i", "eth2", "radio", "off"); // Turn off 5G-1 radio

				no_assoc_check = 0;
				pwrsave_status = MODE_PWRSAVE;
			}
		}
	}

	/* error/exit */
exit:
	if (mac_list) free(mac_list);

	return;
}
#endif

#ifdef RTCONFIG_CONCURRENTREPEATER
#define POWER_ON_TIME			40
#define CHECK_ASSOC_STATUS_TIME 60
#define OFFLINE_MAX_TIME		15
#ifdef RPAC92
#define WLC_NUM					3
#else
#define WLC_NUM 				2
#endif
typedef struct _wl_br_status{
	char wlcif[32];
	int rssi;
	int offline;
	int in_br;
}wl_br_status,*pwl_br_status;
wl_br_status wlbrs_list[WLC_NUM];

void dump_br_status(){
	pwl_br_status pstatus;
	int i;
	if(wlbrs_list[0].wlcif[0] == 0)
		return ;
	for(i = 0;i < WLC_NUM;i++){
		pstatus = &wlbrs_list[0]+ i*sizeof(wl_br_status);
		_dprintf("ifname=%s,in_br=%d,rssi=%d,offline = %d\n",pstatus->wlcif
			,pstatus->in_br,pstatus->rssi,pstatus->offline);
	}
}

void init_wl_br_status()
{
	pwl_br_status pstatus;
	int i = 0;
	for (i = 0; i < WLC_NUM; i++) {
		pstatus = &wlbrs_list[0]+ i*sizeof(wl_br_status);
		memset(pstatus->wlcif, 0, sizeof(pstatus->wlcif));
		pstatus->in_br = 0;
		pstatus->rssi = 0;
		pstatus->offline = 0;
	}
}

void update_wl_br_status(pwl_br_status list)
{
	int i;
	char buf[32];
	pwl_br_status pstatus;
	if(!list)
		return;
	if(list->wlcif[0]==0){
		if ((sw_mode() == SW_MODE_REPEATER && nvram_get_int("wlc_express") ==0) ||
			mediabridge_mode()){
			pstatus = list;
#if defined(RTCONFIG_QCA)
			strncpy(pstatus->wlcif, STA_2G,32);
#elif defined(RTCONFIG_RALINK)
			strncpy(pstatus->wlcif, APCLI_2G,32);
#elif defined(RTCONFIG_REALTEK)
			if (mediabridge_mode())
				strncpy(pstatus->wlcif, WIF_2G,32);
			else
				strncpy(pstatus->wlcif, VXD_2G,32);
#endif
			pstatus->offline = -1;
			if(WLC_NUM > 1){
				pstatus = list + sizeof(wl_br_status);
#if defined(RTCONFIG_QCA)
				strncpy(pstatus->wlcif, STA_5G,32);
#elif defined(RTCONFIG_RALINK)
				strncpy(pstatus->wlcif, APCLI_5G,32);
#elif defined(RTCONFIG_REALTEK)
			if (mediabridge_mode())
				strncpy(pstatus->wlcif, WIF_5G,32);
			else
				strncpy(pstatus->wlcif, VXD_5G,32);
#endif
				pstatus->offline = -1;
			}
#ifdef RPAC92
			if(WLC_NUM == 3) {
				pstatus = list + 2*sizeof(wl_br_status);
				if (mediabridge_mode())
					strncpy(pstatus->wlcif, WIF_5G2, 32);
				else
					strncpy(pstatus->wlcif, VXD_5G2, 32);

				pstatus->offline = -1;
			}
#endif
		}
		else{
			return;
		}
	}

	for(i = 0;i < WLC_NUM;i++){
		pstatus = list + i*sizeof(wl_br_status);
		if(pstatus ->wlcif[0]){

			sprintf(buf, "wlc%d_state", i);
			if(nvram_get_int(buf) != WLC_STATE_CONNECTED){
				if(pstatus->offline >= 0)
					pstatus->offline++;
				continue;
			}

			pstatus->offline = 0;

			if (nvram_get_int(buf) == WLC_STATE_CONNECTED) {
				int rssi = get_conn_link_quality(i);
				if (rssi)
					pstatus->rssi = rssi;
				else
					pstatus->rssi = -1;
			}
			else
				pstatus->rssi = -1;
		}
	}
}
int is_wlc_br_online(pwl_br_status list){
	pwl_br_status pstatus;
	int i;
	if(!list || list->wlcif[0] == 0)
		return 0;
	for(i = 0;i < WLC_NUM;i++){
		pstatus = list + i*sizeof(wl_br_status);
		if(pstatus->in_br && pstatus->wlcif[0])
			return 1;
	}
	return 0;
}
int select_one_online(pwl_br_status list)
{
	pwl_br_status pstatus;
	int i;
	int max = -1,j = -1;
	char *lan_ifname;
	char ssidbuf[32];
	int connected = 0;
	static int skip_wait_flag = 0;
	static int wait_time = -99;

	if(!list || list->wlcif[0] == 0) {
		nvram_set_int("wlc_band", -1);
		nvram_set("wlc_ssid", "");
		return 0;
	}

	for(i = 0;i < WLC_NUM;i++){
		pstatus = list + i*sizeof(wl_br_status);
		if(pstatus->offline == 0 && pstatus->wlcif[0]) {
			if(max <= pstatus ->rssi){
				max = pstatus ->rssi;
				j = i;
			}
			if (pstatus->rssi >= 0)
				connected++;
		}
	}

	/* When DUT booted, wait for bands connected to select a the better quility band. */
	if (skip_wait_flag == 0 && (access_point_mode() || (repeater_mode() && nvram_get_int("wlc_express") != 0))) { // AP mode and express way mode
		skip_wait_flag = 1;
	}
	if (!skip_wait_flag) {
		if (((repeater_mode() && nvram_get_int("wlc_express") == 0) || mediabridge_mode()) && j >= 0) {
			if (WLC_NUM > 1) {
				if (j == 0 && wait_time == -99) { // 2G connected, wait 5g

		#if defined(RTCONFIG_WLMODULE_MT7615E_AP)
					wait_time = nvram_get_int("wl_time");
		#else
					wait_time = 10;
		#endif
				}
				else if (wait_time == -99){// 5G connected, wait 2g

		#if defined(RTCONFIG_WLMODULE_MT7615E_AP)
					wait_time = nvram_get_int("wl_time");
		#else
					wait_time = 10;
		#endif

				}

				if (wait_time > 0) {
					if (connected == WLC_NUM) { // all connected
						wait_time = 0;
					}
					else {
						wait_time--;
						return 0;
					}
				}
				else if (wait_time != -99)
					skip_wait_flag = 1;
			}
		}
	}

	if( j >=0 ){
		pstatus = list + j*sizeof(wl_br_status);
		pstatus ->in_br = 1;
		lan_ifname = strdup(nvram_safe_get("lan_ifname"));
		eval("brctl", "addif", lan_ifname, pstatus->wlcif);
		nvram_set_int("wlc_band", j);

		sprintf(ssidbuf, "wlc%d_ssid", j);
		nvram_set("wlc_ssid", nvram_safe_get(ssidbuf));

		free(lan_ifname);
		return 1;
	}

	nvram_set_int("wlc_band", -1);
	nvram_set("wlc_ssid", "");
	return 0;
}
void check_wlc_br_online(pwl_br_status list)
{
	pwl_br_status pstatus;
	int i;
	int j = -1;
	char *lan_ifname;
	if(!list || list->wlcif[0] == 0)
		return ;
	for(i = 0;i < WLC_NUM;i++){
		pstatus = list + i*sizeof(wl_br_status);
		if(pstatus->offline > OFFLINE_MAX_TIME && pstatus->wlcif[0] && pstatus->in_br){
			j = i;
			break;
		}
	}
	if( j >=0 ){
		pstatus = list + j*sizeof(wl_br_status);
		pstatus ->in_br = 0;
		lan_ifname = strdup(nvram_safe_get("lan_ifname"));
		eval("brctl", "delif", lan_ifname, pstatus->wlcif);
#if defined(RTCONFIG_QCA)
		/* special case for restart 2G/5G sta */
		ifconfig(pstatus->wlcif, 0, NULL, NULL);
		ifconfig(pstatus->wlcif, IFUP, NULL, NULL);
		if (!strcmp(pstatus->wlcif, STA_2G))
			eval("wpa_cli", "-p", "/var/run/wpa_supplicant-sta0", "-i", STA_2G, "reconnect");
		else if (!strcmp(pstatus->wlcif, STA_5G))
			eval("wpa_cli", "-p", "/var/run/wpa_supplicant-sta1", "-i", STA_5G, "reconnect");
#elif defined(RTCONFIG_RALINK)
		//TODO
#endif
		select_one_online(list);
		if (!nvram_match("wlc_band", "-1")) {
			/* renew IP */
			eval("killall", "-SIGUSR1", "udhcpc");
		}
		free(lan_ifname);
	}
}
void bridge_check()
{
	unsigned long uptime;
	struct sysinfo info;
	pwl_br_status plist = &wlbrs_list[0];
	sysinfo(&info);
	uptime = (unsigned long) info.uptime ;

	if (nvram_get_int("wps_cli_state") == 1 && sw_mode() == SW_MODE_REPEATER)
		return;
	if (uptime > POWER_ON_TIME) {
		update_wl_br_status(plist);
		if (!plist->wlcif[0])
			return;
		if (!is_wlc_br_online(plist)) {
			select_one_online(plist);
		}
		else {
			if (uptime > CHECK_ASSOC_STATUS_TIME) {
				check_wlc_br_online(plist);
			}
		}
	}
}
#endif /* RTCONFIG_CONCURRENTREPEATER */

#ifdef RTL_WTDOG
void watchdog_func()
{
	FILE *file;
	file = fopen("/proc/watchdog_kick","w+");
	if(file)
	{
		fputs("111", file);
		fclose(file);
	}
}
#endif

#ifdef RTCONFIG_ALPINE
#define RF_EXTERNAL_TEMPE	"temperature_rfic_external"
#define BB_INTERNAL_TEMPE	"temperature_bbic_internal"
static int preTempe = 0;
enum {
	//ascend degree
	ASCEND_DEGREE_1 = 30,
	ASCEND_DEGREE_2 = 45,
	ASCEND_DEGREE_3 = 50,
	ASCEND_DEGREE_4 = 55,
	ASCEND_DEGREE_5 = 65,
	ASCEND_DEGREE_6 = 80,
	ASCEND_DEGREE_7 = 85,
	ASCEND_DEGREE_8 = 90,
	//descend degree
	DESCEND_DEGREE_1 = 20,
	DESCEND_DEGREE_2 = 40,
	DESCEND_DEGREE_3 = 45,
	DESCEND_DEGREE_4 = 50,
	DESCEND_DEGREE_5 = 53,
	DESCEND_DEGREE_6 = 75,
	DESCEND_DEGREE_7 = 80,
	DESCEND_DEGREE_8 = 85,
};

void set_fan(int curTempe)
{
	//ascend path
	if(preTempe < curTempe)
	{
		if(curTempe >= ASCEND_DEGREE_8)
			system("i2cset -y 0 0x2e 0x22 0xff i");	//100% duty
		else if(curTempe >= ASCEND_DEGREE_7)
			system("i2cset -y 0 0x2e 0x22 0xe4 i");	//88% duty
		else if(curTempe >= ASCEND_DEGREE_6)
			system("i2cset -y 0 0x2e 0x22 0xd5 i");	//83% duty
		else if(curTempe >= ASCEND_DEGREE_5)
			system("i2cset -y 0 0x2e 0x22 0xc7 i");	//77% duty
		else if(curTempe >= ASCEND_DEGREE_4)
			system("i2cset -y 0 0x2e 0x22 0xab i");	//66% duty
		else if(curTempe >= ASCEND_DEGREE_3)
			system("i2cset -y 0 0x2e 0x22 0x9c i");	//61% duty
		else if(curTempe >= ASCEND_DEGREE_2)
			system("i2cset -y 0 0x2e 0x22 0x8e i");	//55% duty
		else if(curTempe >= ASCEND_DEGREE_1)
			system("i2cset -y 0 0x2e 0x22 0x72 i");	//44% duty
	}
	//descend path
	if(preTempe > curTempe)
	{
		if(curTempe <= DESCEND_DEGREE_2)
			system("i2cset -y 0 0x2e 0x22 0x72 i");	//44% duty
		else if(curTempe <= DESCEND_DEGREE_3)
			system("i2cset -y 0 0x2e 0x22 0x8e i");	//55% duty
		else if(curTempe <= DESCEND_DEGREE_4)
			system("i2cset -y 0 0x2e 0x22 0x9c i");	//61% duty
		else if(curTempe <= DESCEND_DEGREE_5)
			system("i2cset -y 0 0x2e 0x22 0xab i");	//66% duty
		else if(curTempe <= DESCEND_DEGREE_6)
			system("i2cset -y 0 0x2e 0x22 0xc7 i");	//77% duty
		else if(curTempe <= DESCEND_DEGREE_7)
			system("i2cset -y 0 0x2e 0x22 0xd5 i");	//83% duty
		else if(curTempe <= DESCEND_DEGREE_8)
			system("i2cset -y 0 0x2e 0x22 0xe4 i");	//88% duty
	}
}
void fan_check()
{	char *buffer;
	char *tmp, *p;
	int tTempe;
	system("qcsapi_pcie get_temperature > /tmp/qtn_tmpereture.txt");

	buffer = read_whole_file("/tmp/qtn_tmpereture.txt");
	if(buffer)
	{
		tmp = p = buffer;
		if((tmp = strstr(tmp, RF_EXTERNAL_TEMPE)))
		{
			if((tmp = strchr(tmp, '=')))
			{
				tmp++;
				p = tmp;
				tTempe = atoi(p);
				if(preTempe != tTempe)
					set_fan(tTempe);
			}
		}

		if((tmp = strstr(tmp, BB_INTERNAL_TEMPE)))
		{
			if((tmp = strchr(tmp, '=')))
			{
				tmp++;
				p = tmp;
				tTempe = atoi(p);
				if(preTempe != tTempe)
					set_fan(tTempe);
			}
		}
		free(buffer);
	}
	unlink("/tmp/qtn_tmpereture.txt");
}
#endif

int pre_dst = -1, is_dst = -1;
/* from non-dst time to dst time or inverse */
int dst_critical()
{
	FILE *pfp;
	char buf[32];

        pfp = popen("date", "r");

	is_dst = 0;
        if (pfp != NULL) {
                if(fgets(buf, 32, pfp) != NULL)
			is_dst = strstr(buf, " DST")?1:0;
        	pclose(pfp);
        }

	if(nvram_match("dbg", "1"))
		_dprintf("DST Time chk(%d, %d)\n", pre_dst, is_dst);

	if(pre_dst != -1 && pre_dst != is_dst) {
		_dprintf("DST time changed(%d->%d)\n", pre_dst, is_dst);
		logmessage("watchdog", "restart_firewall due DST time changed(%d->%d)", pre_dst, is_dst);
		pre_dst = is_dst;
		return 1;
	}

	pre_dst = is_dst;
	return 0;
}

#ifdef RTCONFIG_FPROBE
void fprobe_check()
{
#define FP_PID "/var/run/fprobe.pid"
	if(!access(FP_PID, F_OK)) //pid file is existed
		return;

	//if(!pids("fprobe"))
	start_fprobe();
}
#endif

#if defined(RTCONFIG_BT_CONN)
#if (defined(RTAX82_XD6) || defined(RTAX82_XD6S) || defined(XD6_V2)) && !defined(RTCONFIG_BCM_MFG)
static int bt_down_count = 0;
static int bt_reset_once = 0;
#endif
static void bluetooth_check()
{
#if defined(RTAX56_XD4) || defined(PLAX56_XP4)
	if(nvram_match("HwId", "B") || nvram_match("HwId", "D")){
		/* Slave, no bluetooth */
		return;
	}
#endif
#if defined(RTAX56_XD4)
	if((nvram_match("HwId", "A") && nvram_get_int("BLE_BT") == 99) ||
		(nvram_match("HwId", "C") && nvram_get_int("BLE_BT") == 99)){
		/* Master without BT */
		return;
	}
#endif
	if (nvram_match("x_Setting", "1"))
	{
		if (nvram_get_int("ble_dut_con"))
			nvram_unset("ble_dut_con");
		return;
	}
	if (!nvram_match("success_start_service", "1"))
		return;
	if (check_bluetooth_device("hci0"))
	{
#if (defined(RTAX82_XD6) || defined(RTAX82_XD6S) || defined(XD6_V2)) && !defined(RTCONFIG_BCM_MFG)
		if (!check_bluetooth_device_up("hci0") && (uptime() > 24) && !nvram_get_int("bt_turn_off"))
		{
			if (bt_down_count++ < 10)
				return;

			if (!bt_reset_once)
				bt_reset_once = 1;
			else
				return;

			logmessage("watchdog", "reset BT hardware");
			set_gpio(29, 0);
			set_gpio(29, 1);
		}
#endif
		return;
	}
	if (nvram_get_int("ble_init"))
		return;
	if (!pids("bluetoothd")) {
#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_PRELINK)
		nvram_unset("ble_dut_con");
		nvram_unset("ble_rename_ssid");
#endif
		notify_rc("start_bluetooth_service");
	}
}
#endif

#ifdef RTCONFIG_STRONGSWAN
void rotate_strongswan_logfile(void)
{
	struct stat st;
	char cmd[256] = {0};

	if(stat("/var/log/"SS_CHARON_LOG, &st) == 0)
	{
		// if size > 1.5 MB
		if(st.st_size > 1536*1024)
		{
			//copy original log file to "/var/log/"SS_CHARON_1_LOG
			snprintf(cmd, sizeof(cmd), "cp /var/log/%s /var/log/%s", SS_CHARON_LOG, SS_CHARON_1_LOG);
			system(cmd);

			if(check_if_file_exist(SS_CHARON_PIDFILE))
			{
				snprintf(cmd, sizeof(cmd), "kill -SIGHUP `cat %s`", SS_CHARON_PIDFILE);
				system(cmd);
			}
			else
			{
				logmessage("watchdog", "[%s] Cannot find [%s]\n", __FUNCTION__, SS_CHARON_PIDFILE);
				_dprintf("[%s] Cannot find [%s]\n", __FUNCTION__, SS_CHARON_PIDFILE);
			}
		}
	}
	else
	{
		if (errno != ENOENT)
			_dprintf("stat [%s] errno:[%d]\n", "/var/log/"SS_CHARON_LOG, errno);
	}
}
#endif /* RTCONFIG_STRONGSWAN */

#if defined(RTCONFIG_QCA_PLC2)
void check_process_plc(void)
{
	if (!pids("detect_plc")) {
		notify_rc("start_detect_plc");
	}
}
#endif

#ifdef HND_ROUTER
void udhcpc_check(void)
{
	FILE *fp;
	char *pidfile = "/var/run/tmp_udhcpc_wan.pid";
	char buf[8] = {0};
	pid_t pid = 0;
	int p = pidof("udhcpc_wan");
	static int count = 0;

	// _dprintf("%s\n", __FUNCTION__);
	if (f_exists(pidfile)) {
		f_read_string(pidfile, buf, sizeof(buf));
		pid = strtoul(buf, NULL, 0);
	}
	if (pid && p == (int)pid) {
		if (count) {
			_dprintf("%s: kill %u\n", __FUNCTION__, pid);
			kill_pid_tk(pid);
			unlink(pidfile);
			count = 0;
		}
		else {
			_dprintf("%s: count %d\n", __FUNCTION__, count);
			count++;
		}
	}
	else if (p > 1) {
		fp = fopen(pidfile, "w");
		if (fp) {
			_dprintf("%s: record %d\n", __FUNCTION__, p);
			fprintf(fp, "%d", p);
			fclose(fp);
			count = 0;
		}
	}
}
#endif

/*******************************************************************
* NAME: record_current_sys_uptime
* AUTHOR: Renjie Lee
* CREATE DATE: 2022/08/15
* DESCRIPTION: Record system uptime in nvram for every 60 seconds
* INPUT:  None
* OUTPUT: None
* RETURN: None
* NOTE:
*
*******************************************************************/
void record_current_sys_uptime(void)
{
	char buf[64] = {0};
	int record_period = 60;
	static int cnt = 0;

	if(cnt == 0)
	{
		snprintf(buf, sizeof(buf), "%ld", get_sys_uptime());
		nvram_set("sys_uptime_now", buf);
	}
	cnt++;
	cnt %= record_period;
}

/*******************************************************************
* NAME: feedback_check
* AUTHOR: Renjie Lee
* CREATE DATE: 2023/12/27
* DESCRIPTION: Check 'fb_state', if it is '0', it means that the feedback was failed for some reasons at last time.
*     We need to call 'sendfeedback' to resend the feedback.
* INPUT:  None
* OUTPUT: None
* RETURN: None
* NOTE: Watchdog calls this function for every 30 seconds.
*     We skip the first 3 calls and execute the 4th call. So the function will work for every 120 seconds.
*
*******************************************************************/
#ifdef RTCONFIG_FRS_FEEDBACK
void feedback_check(void)
{
	static int check = -1;
	int skip_times = 4;

	check++;
	check %= skip_times;
	if(check != skip_times - 1)
	{
		return;
	}

	if(!nvram_get_int("ntp_ready"))
	{
		return;
	}

	if(nvram_match("fb_state", "0"))
	{
		char *cmd[] = {"sendfeedback", NULL};
		int pid;

		if(!pids("sendfeedback"))
		{
			logmessage("watchdog", "[%s] Resend the last feedback..\n", __FUNCTION__);
			_dprintf("[%s] Resend the last feedback..\n", __FUNCTION__);
			nvram_set("fb_resend", "1");
			_eval(cmd, NULL, 0, &pid);
		}
	}
}
#endif /* RTCONFIG_FRS_FEEDBACK */

/* wathchdog is runned in NORMAL_PERIOD, 1 seconds
 * check in each NORMAL_PERIOD
 *	1. button
 *
 * check in each NORAML_PERIOD*10
 *
 *	1. time-dependent service
 */

void watchdog(int sig)
{
	int period;

#ifdef RTL_WTDOG
	watchdog_func();
#endif
	/* handle button */
	btn_check();

#ifdef RTCONFIG_BCM_7114
	if (ATE_BRCM_FACTORY_MODE() || nvram_match("mfgfw", "1"))
		httpd_check();
#endif

#ifdef RTCONFIG_ISP_CUSTOMIZE_TOOL
	ict_led_control();
#endif

	if (nvram_match("asus_mfg", "0") && !inhibit_led_on())
	{
#if defined(RTCONFIG_CONCURRENTREPEATER)
#if defined(RTCONFIG_QCA)
	qca_wps_state_check();
#endif
#ifndef RPAC92
#ifdef RTCONFIG_AMAS 
    if(!nvram_match("re_mode","1"))
#endif
	bridge_check();
#endif
#else
	service_check();
#endif
	}

#ifdef RTCONFIG_BSC_SR
	bsc_sr_check();
#endif
#ifdef RTAC88U
	rtkl_check();
#endif

#if defined(RTCONFIG_QCA) && defined(RTCONFIG_WIGIG)
	wigig_temperatore_check();
#endif

	/* some io func will delay whole process in urgent mode, move this to sw_devled process */
	//led_check();

#ifdef RTCONFIG_CONCURRENTREPEATER
#ifdef RTCONFIG_QCA
	qca_wps_state_check();
#endif
#ifdef RTCONFIG_REALTEK
	rtk_wps_state_check();
	rtk_wl_led();
#endif
#endif

#ifdef RTCONFIG_RALINK
	if (need_restart_wsc) {
		char word[256], *next, ifnames[128];

		strcpy(ifnames, nvram_safe_get("wl_ifnames"));
		foreach (word, ifnames, next) {
			SKIP_ABSENT_FAKE_IFACE(word);
			eval("iwpriv", word, "set", "WscConfMode=0");
		}

		need_restart_wsc = 0;
		start_wsc_pin_enrollee();
	}
#endif
#ifdef RTCONFIG_SWMODE_SWITCH
	swmode_check();
#endif
#ifdef WEB_REDIRECT
	wanduck_check();
#endif

	/* if timer is set to less than 1 sec, then bypass the following */
	if (itv.it_value.tv_sec == 0)
		return;


	record_current_sys_uptime();

#ifdef RTCONFIG_WIFI_SON
	if(nvram_match("wifison_ready", "1")) {
		wifison_check();
#if defined(RTCONFIG_HIDDEN_BACKHAUL)
#if defined(MAPAC2200)
		if (get_radio_status("ath1")) 
			doSystem("ifconfig ath1 down");
#endif
#endif
	}
#endif // WIFI_SON

#if defined(RTCONFIG_LP5523) || defined(RTCONFIG_FIXED_BRIGHTNESS_RGBLED)
	link_pap_status();
#endif 
#if defined(RTCONFIG_BCM_CLED) && defined(RTCONFIG_SINGLE_LED)
	single_led_status();
#endif
#if defined(RTCONFIG_BT_CONN)
#if defined(RTAX95Q) || defined(XT8PRO) || defined(BM68) || defined(XT8_V2) || defined(RTAXE95Q) || defined(ET8PRO) || defined(ET8_V2) || defined(RTAX56_XD4) || defined(XD4PRO) || defined(RTAX82_XD6) || defined(RTAX82_XD6S) || defined(XD6_V2)
	bluetooth_check();
#endif
	bt_turn_off_service();
#endif
#if (defined(PLN12) || defined(PLAC56) || defined(PLAC66U))
	if (nvram_match("plc_sleep_enabled", "1"))
		client_check();
#endif

#ifdef RTCONFIG_AMAS
#if defined(RTCONFIG_WIFI_SON)
	if(!nvram_match("wifison_ready", "1"))
#endif
	onboarding_check();
#endif

#ifdef RTCONFIG_FPROBE
	//fprobe_check();
#endif

#if defined(RTCONFIG_QCA)
	if (nvram_match("Ate_power_on_off_enable", "2") && ((uptime() - g_t1) > 30)) {
		ate_temperature_record();
		g_t1 = uptime();
	}
#endif

	if (!nvram_match("asus_mfg", "0")) 
		return;

	watchdog_period = (watchdog_period + 1) % 30;
#ifdef WATCHDOG_PERIOD2
	watchdog_period2 = (watchdog_period2 + 1) % 10;
#endif

#ifdef RTCONFIG_BCMARM
	if ((u3_chk_life < 20)
#ifdef RTAC68U
		&& hw_usb_cap()
#endif
	) {
		chkusb3_period = (chkusb3_period + 1) % u3_chk_life;
		if (!chkusb3_period && nvram_get_int("usb_usb3") &&
		    ((nvram_match("usb_path1_speed", "12") &&
		      !nvram_match("usb_path1", "printer") && !nvram_match("usb_path1", "modem")) ||
		     (nvram_match("usb_path2_speed", "12") &&
		      !nvram_match("usb_path2", "printer") && !nvram_match("usb_path2", "modem")))) {
			_dprintf("force reset usb pwr\n");
#ifdef RTCONFIG_USB
			stop_usb_program(1);
#endif
			sleep(1);
			set_pwr_usb(0);
			sleep(3);
			set_pwr_usb(1);
			u3_chk_life *= 2;
		}
	}
#endif

#ifdef BTN_SETUP
	if (btn_pressed_setup >= BTNSETUP_START)
		return;
#endif

#ifdef WATCHDOG_PERIOD2
	if (watchdog_period2) {
		if (!watchdog_period)
			goto wdp;
		return;
	}
#ifdef RTCONFIG_BONDING
	nvram_set_int("bondst", (bs = get_bonding_status()));

	if(bs!=bs_pre)
		logmessage("BONDING", bs_desc[bs]);
	bs_pre = bs;
#endif
#endif

#ifdef RTCONFIG_HND_ROUTER_AX
	if (nvram_get_int("dfs_cac_check"))
		dfs_cac_check();
#endif

	if (watchdog_period)
		return;

#if defined(RTAX58U_V2) || defined(BR63)
	if (check_x_Setting && nvram_match("x_Setting", "1")) {
		check_x_Setting = 0;
		notify_rc("restart_firewall");
	}
#endif
#ifdef RTAX55
	drop_caches_period = nvram_get_int("drop_caches_period");
	if  (drop_caches_period) {
		drop_caches = (drop_caches + 1) % drop_caches_period ;
		if (!drop_caches)
			f_write_string("/proc/sys/vm/drop_caches", "1", 0, 0);
	}
#endif
	top_period = nvram_get_int("top_period");
	if (top_period) {
		top = (top + 1) % top_period ;
		if (!top)
			system("top -b -n 1 | head | logger -t top");
	}
#if !defined(RTCONFIG_BCM_MFG) && (defined(RTAX82U) || defined(GSAX3000) || defined(GSAX5400) || defined(TUFAX5400) || defined(GTAX11000_PRO) || defined(GTAXE16000) || defined(GTBE98) || defined(GTBE98_PRO) || defined(GTAX6000) || defined(GT10) || defined(RTAX82U_V2) || defined(TUFAX5400_V2) || defined(DSL_AX82U))
	if (nvram_get_int("x_Setting") && !pids("ledg")) {
		ledg_count = (ledg_count + 1) % 2 ;
		if (!ledg_count) {
			notify_rc_and_wait_2min("restart_ledg");
		}
	}
#endif
#ifdef RTCONFIG_BCMWL6
	if (!restore_defaults_g && strlen(nvram_safe_get("acs_ifnames")) && nvram_get_int("wlready") &&
		!mediabridge_mode() &&
#ifdef RTCONFIG_HND_ROUTER_AX
		!pids("acsd2")
#else
		!pids("acsd")
#endif
	)
		notify_rc_and_wait_1min("restart_acsd");
#endif
#if defined(RTAX3000N) || defined(RPAX58)
	parse_ptf();
#endif

	if (!no_need_to_start_wanduck() && !pids("wanduck")) {
		wanduck_count = (wanduck_count + 1) % 4 ;
		if (!wanduck_count) {
			stop_wanduck();
			start_wanduck();
		}
	}

	if(nvram_match("ntp_ready", "1")) {
#ifndef RTCONFIG_AVOID_TZ_ENV
		if(!nvram_match("time_zone_x", time_zone_t)){
			strlcpy(time_zone_t, nvram_safe_get("time_zone_x"), sizeof(time_zone_t));
			setenv("TZ", nvram_safe_get("time_zone_x"), 1);
			tzset();
		}
#endif	/* RTCONFIG_AVOID_TZ_ENV */
		if(strstr(nvram_safe_get("time_zone_x"), "DST") && dst_critical()) {
			_dprintf("dst critical.\n");
			notify_rc("restart_firewall");
			notify_rc("restart_wan");
			eval("pc", "flush");
		}
	}

#if defined(RTCONFIG_HND_ROUTER_AX) && defined(RTCONFIG_BCM_MFG)
	ate_temperature_record();
#endif

#if defined(HND_ROUTER) || defined(RTCONFIG_BCM_7114) || defined(RTCONFIG_BCM4708)
	dump_WlGetDriverStats(0, 1);
#endif

#ifdef WATCHDOG_PERIOD2
wdp:
#endif
#ifdef CONFIG_BCMWL5
#ifndef RTCONFIG_BCM_MFG
	if (factory_debug())
#endif
#else
	if (IS_ATE_FACTORY_MODE())
#endif
	{
		return;
	}
#ifdef RTCONFIG_MFGFW
	if(nvram_match("mfgfw", "1"))
		return;
#endif

#ifdef RTCONFIG_USER_LOW_RSSI
	rssi_check();
#endif
#ifdef RTCONFIG_WATCH_WLREINIT
	wlcnt_chk();
#endif
	/* check for time-related services */
	timecheck();

#ifdef RTCONFIG_WL_SCHED_V2
	timecheck_v2();
#endif
#ifdef RTCONFIG_BACKUP_LOG
#ifdef RTCONFIG_USB
	bk_center_main();
#endif
#endif

	if (nvram_match("ddns_enable_x", "1")) {
		/* Force a DDNS update every "x" days - default is 21 days */
		period = nvram_get_int("ddns_refresh_x");
		if ((period) && (++ddns_update_timer >= (DAY_PERIOD * period))) {
			ddns_update_timer = 0;
			logmessage("watchdog", "Forced DDNS update (after %d days)", period);
			notify_rc("restart_ddns");
		} else {
			ddns_check();
		}
	}

	networkmap_check();
	httpd_check();
	dnsmasq_check();
#ifdef RTCONFIG_NEW_USER_LOW_RSSI
	roamast_check();
#endif
#if defined(RPAX56)
	amas_linkctrl();
#endif
#ifdef RTAC87U
	qtn_module_check();
#endif
#ifdef RTCONFIG_ALPINE
	fan_check();
#endif
#ifdef RTCONFIG_LANTIQ
	wave_monitor_check();
#endif
//#if defined(RTCONFIG_JFFS2LOG) && defined(RTCONFIG_JFFS2)
#if defined(RTCONFIG_JFFS2LOG) && (defined(RTCONFIG_JFFS2)||defined(RTCONFIG_BRCM_NAND_JFFS2))
	syslog_commit_check();
#endif
#if defined(RTCONFIG_USB_MODEM)
	modem_log_check();
#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS)
	int modem_unit;

	for(modem_unit = MODEM_UNIT_FIRST; modem_unit < MODEM_UNIT_MAX; ++modem_unit)
		modem_flow_check(modem_unit);
#endif
#endif
#if 0
	auto_firmware_check();
#elif RTCONFIG_MERLINUPDATE
	auto_firmware_check_merlin();
#endif
#if defined(RTCONFIG_LANTIQ) && defined(RTCONFIG_GN_WBL)
	GN_WBL_restart();
#endif

#ifdef RTCONFIG_NOTIFICATION_CENTER
	alert_mail_service();
	ntevent_intranet_usage_insight();
#endif

#ifdef RTCONFIG_BWDPI
	check_hour_monitor_service();
#endif

#if 0 //#if defined(RTCONFIG_TOR) && (defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2))
	if (nvram_get_int("Tor_enable"))
		Tor_microdes_check();
#endif

#ifdef RTCONFIG_ERP_TEST
	ErP_Test();
#endif

#if defined(RTCONFIG_USB) && defined(RTCONFIG_NOTIFICATION_CENTER) && defined(RTCONFIG_CLOUDSYNC)
	ntevent_disk_usage_check();
#endif

#if defined(RTAC1200G) || defined(RTAC1200GP)
	if (pids("wdg_monitor"))
		kill_pidfile_s("/var/run/wdg_monitor.pid", SIGUSR1);
#endif

#ifdef RTCONFIG_AMAS
#if defined(RTCONFIG_WIFI_SON) && defined(RTCONFIG_AMAS)
	if(!nvram_match("wifison_ready", "1"))
#endif
	amas_ctl_check();
#endif
#ifdef RTCONFIG_CFGSYNC
	cfgsync_check();
#endif
#ifdef RTCONFIG_TUNNEL
	if(!nvram_get_int("aae_disable_force"))
		mastiff_check();
#endif
#if defined(RTCONFIG_AMAS)
	amaslib_check();
#if defined(RTCONFIG_QCA_LBD)
	if (nvram_match("smart_connect_x", "1") && !pids("lbd") && !mediabridge_mode() && f_exists(LBD_PATH))
		start_qca_lbd();
#endif
#endif

#if defined(RTCONFIG_SOC_IPQ8074)
	beacon_counter_monitor();
	thermal_monitor();
#endif

#ifdef RTCONFIG_AWSIOT
	awsiot_check();
#endif

#ifdef RTCONFIG_UPNPC_NEW
	if(nvram_get_int("ipsec_ig_enable") == 1)
	{
		renew_ipsec_upnpc_duration(UPNPC_TYPE_UDP);
	}
#endif /* RTCONFIG_UPNPC_NEW */

#ifdef RTCONFIG_STRONGSWAN
	rotate_strongswan_logfile();
#endif /* RTCONFIG_STRONGSWAN */
#if defined(RTCONFIG_QCA_PLC2)
	check_process_plc();
#endif
#ifdef HND_ROUTER
	udhcpc_check();
#endif
#if 1   /* Do Function Test Script */
		if(nvram_match("ci", "1")){
			system("sh /tmp/mnt/sda1/auto_test.sh");
		}
#endif
#ifdef RTCONFIG_FRS_FEEDBACK
	feedback_check();
#endif /* RTCONFIG_FRS_FEEDBACK */
}

#if ! (defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK))
void watchdog02(int sig)
{
	watchdog_check();
	return;
}
#endif /* ! (RTCONFIG_QCA || RTCONFIG_RALINK) */

int
watchdog_main(int argc, char *argv[])
{
	FILE *fp;
	const struct mfg_btn_s *p;

	/* write pid */
	if ((fp = fopen("/var/run/watchdog.pid", "w")) != NULL)
	{
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}

#if defined(RTAX58U_V2) || defined(BR63)
	if (is_routing_enabled() && nvram_match("x_Setting", "0"))
		check_x_Setting = 1;
#endif
#ifdef RTCONFIG_AMAS
	/* Prepare timeout value */
	time_mapping_get(get_productid(), &time_mapping);
	_dprintf("### onboarding model=%s, reboot_time=%d, connection_timeout=%d, traffic_timeout=%d\n", 
		get_productid(), time_mapping.reboot_time, time_mapping.connection_timeout, time_mapping.traffic_timeout);
#endif

#if defined(RTCONFIG_TURBO_BTN)
#if defined(RTCONFIG_RGBLED)
	g_boost_status[BOOST_AURA_RGB_SW] = !!nvram_get_int("aurargb_enable");
#elif defined(RTCONFIG_BCM_CLED)
	g_boost_status[BOOST_AURA_RGB_SW] = !!nvram_get_int("ledg_led_enable");
#endif
	g_boost_status[BOOST_ACS_DFS_SW] = !!nvram_get_int("acs_dfs");
	g_boost_status[BOOST_LED_SW] = !!nvram_get_int("AllLED");
#endif
#if defined(RPAX56) || defined(RPAX58)
	if(ATE_BRCM_FACTORY_MODE()) {
		_dprintf("watchdog turn on factory mode led\n");
#ifdef RPAX58
                eval("sw", "0xff803014", "0xd8a0");
#else
                eval("sw", "0xff803014", "0xffffa75f");
#endif
                eval("sw", "0xff803018", "0x00001000");
		_bcm_cled_ctrl(BCM_CLED_BLUE, BCM_CLED_STEADY_NOBLINK);
#ifdef RPAX58
                eval("sw", "0xFF80301c", "0xd8a0");
#else
                eval("sw", "0xFF80301c", "0x58a0");
#endif

	}
#endif

#ifdef RTCONFIG_CONCURRENTREPEATER
	if (sw_mode() == SW_MODE_REPEATER) // Repeater mode
		nvram_set("wps_cli_state", "0");//init

	init_wl_br_status();
#endif

#ifdef RTCONFIG_SWMODE_SWITCH
	pre_sw_mode=sw_mode();
#endif

#ifdef RTCONFIG_BCMWL6
	if (mediabridge_mode())
		wlonunit = nvram_get_int("wlc_band");
#endif

#ifdef RTAC88U
	rtl_period = nvram_get_int("rtl_period")?:10;
	sltime = nvram_get_int("sleep")?:3;
	rtl_fail_max = nvram_get_int("rtl_fail_max")?:1;
#endif

#if defined(RTCONFIG_BCM_CLED) && defined(RTCONFIG_SINGLE_LED)
	nvram_unset("bcm_cled_in_wps");
	nvram_unset("bcm_cled_in_reset");
#endif

#ifdef RTCONFIG_RALINK
	doSystem("iwpriv %s set WatchdogPid=%d", WIF_2G, getpid());
#if defined(RTCONFIG_HAS_5G)
	doSystem("iwpriv %s set WatchdogPid=%d", WIF_5G, getpid());
#endif	/* RTCONFIG_HAS_5G */
#endif	/* RTCONFIG_RALINK */

	init_sig();

#ifdef RTCONFIG_DSL //Paul add 2012/6/27
	nvram_set("dsltmp_syncloss", "0");
	nvram_set("dsltmp_syncloss_apply", "0");
#endif
	nvram_unset("wps_ign_btn");

	/* Set nvram variables which are used to record button state in mfg mode to "0".
	 * Because original code in ate.c rely on such behavior.
	 * If those variables are unset here, related ATE command print empty string
	 * instead of "0".
	 */
	for (p = &mfg_btn_table[0]; p->id < BTN_ID_MAX; ++p) {
		nvram_set(p->nv, "0");
	}

#ifndef RTCONFIG_AVOID_TZ_ENV
	setenv("TZ", nvram_safe_get("time_zone_x"), 1);

	_dprintf("TZ watchdog\n");
#endif	/* RTCONFIG_AVOID_TZ_ENV */
	/* set timer */
	alarmtimer(NORMAL_PERIOD, 0);

	led_control_normal();
#ifdef RTL_WTDOG
	start_rtl_watchdog();
#endif

	/* Most of time it goes to sleep */
	int watchsig_dbg = nvram_get_int("watchsig");
	while (1)
	{
		while(sigbones)
			put_all_dogs();
		
		pause();

		if(watchsig_dbg) {
			sigset_t waitset;
			sigpending(&waitset);
			_dprintf("(now blocked sig:%s%s%s%s%s%s%s)\n", 
							sigismember(&waitset, SIGALRM)?"SIGALRM ":"",
							sigismember(&waitset, SIGCHLD)?"SIGCHLD ":"",
							sigismember(&waitset, SIGUSR1)?"SIGUSR1 ":"",
							sigismember(&waitset, SIGUSR2)?"SIGUSR2 ":"",
							sigismember(&waitset, SIGTSTP)?"SIGTSTP ":"",
							sigismember(&waitset, SIGHUP) ?"SIGHUP ":"",
							sigismember(&waitset, SIGTTIN)?"SIGTTIN ":""
			);
			if(nvram_match("reset_sig", "1"))
				unblock_sigs();
		}
	}

	return 0;
}

#if ! (defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK))
/* to check watchdog alive */
int watchdog02_main(int argc, char *argv[])
{
	FILE *fp;
	/* write pid */
	if ((fp = fopen("/var/run/watchdog02.pid", "w")) != NULL) {
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}
	/* set the signal handler */
	signal(SIGALRM, watchdog02);

	/* set timer */
	alarmtimer02(10, 0);
	/* Most of time it goes to sleep */
	while(1) {
		pause();
	}
	return 0;
}
#endif	/* ! (RTCONFIG_QCA || RTCONFIG_RALINK) */

#ifdef SW_DEVLED
/* to control misc dev led */
int sw_devled_main(int argc, char *argv[])
{
	FILE *fp;
	/* write pid */
	if ((fp = fopen("/var/run/sw_devled.pid", "w")) != NULL) {
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}

#if defined (RTCONFIG_LED_BTN) || defined (RTCONFIG_WPS_ALLLED_BTN) || (!defined(RTCONFIG_WIFI_TOG_BTN) && !defined(RTCONFIG_QCA))
	swled_alloff_counts = nvram_get_int("offc");
#endif

	init_sig_swled();
	/* set timer */
	alarmtimer(NORMAL_PERIOD, 0);

#ifdef BLUECAVE
	kill_pidfile_s("/var/run/sw_devled.pid", SIGUSR1);
#endif

	/* Most of time it goes to sleep */
	while(1) {
		while(sigbones)
			put_all_dogs();
		pause();
	}
	return 0;
}
#endif

#if defined(RTAC1200G) || defined(RTAC1200GP)
/* workaroud of watchdog timer shutdown */
int wdg_monitor_main(int argc, char *argv[])
{
	FILE *fp;
	if ((fp = fopen("/var/run/wdg_monitor.pid", "w")) != NULL) {
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}

	init_sig_wmon();

	alarmtimer(WDG_MONITOR_PERIOD, 0);

	while(1) {
		while(sigbones)
			put_all_dogs();
		pause();
	}
	return 0;
}
#endif


#ifdef RTCONFIG_MERLINUPDATE
// Asuswrt-Merlin's code, without the auto-upgrade and debug logging
void auto_firmware_check_merlin()
{
	int periodic_check = 0;
	static int period_retry = 0;
	static int bootup_check_period = 3;	//wait 3 times(90s) to check
	static int bootup_check = 1;
	int initial_state;
	time_t now;
	struct tm local;
	static int rand_hr, rand_min;

#if 0		//defined(RTAX58U) || defined(RTAX56U)
	if (!strncmp(nvram_safe_get("territory_code"), "CX", 2))
		return;
#endif
	if (!nvram_get_int("ntp_ready")){
		return;
	}

	if(bootup_check_period > 0){	//bootup wait 90s to check
		bootup_check_period--;
		return;
	}

	time(&now);
	localtime_r(&now, &local);

	if(local.tm_hour == (2 + rand_hr) && local.tm_min == rand_min) //at 2 am + random offset to check
		periodic_check = 1;

	if (bootup_check || periodic_check || period_retry!=0)
	{
#if defined(RTCONFIG_ASUSCTRL) && defined(GTAC5300)
		if (periodic_check)
			asus_ctrl_sku_update();
#endif
#ifdef RTCONFIG_ASD
		//notify asd to download version file
		if (pids("asd"))
		{
			killall("asd", SIGUSR1);
		}
#endif

		if (bootup_check)
		{
			bootup_check = 0;
			rand_hr = rand_seed_by_time() % 4;
			rand_min = rand_seed_by_time() % 60;
#ifdef RTCONFIG_AMAS
			if(nvram_match("re_mode", "1"))
				return;
#endif
		}

		if (!nvram_get_int("firmware_check_enable") ||
		    nvram_contains_word("rc_support", "noupdate"))
			return;

		period_retry = (period_retry+1) % 3;
		initial_state = nvram_get_int("webs_state_flag");

#if defined(RTL_WTDOG)
		stop_rtl_watchdog();
#endif
		eval("/usr/sbin/webs_update.sh");
#if defined(RTL_WTDOG)
		start_rtl_watchdog();
#endif

		if (nvram_get_int("webs_state_update") &&
		    !nvram_get_int("webs_state_error") &&
		    strlen(nvram_safe_get("webs_state_info_am")))
		{
			period_retry = 0;	// We got a response from server, no need to retry
			if ((initial_state == 0) && (nvram_get_int("webs_state_flag") == 1))		// New update
			{
				char version[4], revision[3], build[16];

				memset(version, 0, sizeof(version));
				memset(revision, 0, sizeof(revision));
				memset(build, 0, sizeof(build));

				sscanf(nvram_safe_get("webs_state_info_am"), "%3[^_]_%2[^_]_%15s", version, revision, build);
				logmessage("watchdog", "New firmware version %s.%s_%s is available.", version, revision, build);
				run_custom_script("update-notification", 0, NULL, NULL);
			}

		}
	}
}
#endif

#if defined(RTCONFIG_NOTIFICATION_CENTER)
void RC_SEND_NT_EVENT(int NT_EVENT_FLAG, char *sub_event)
{
	char str[32] = {0}, json_str[2048] = {0};
	struct json_object *nt_root = json_object_new_object();
	struct json_object *payload = json_object_new_object();
	json_object_object_add(nt_root, "from", json_object_new_string("HTTPD"));

	switch(NT_EVENT_FLAG)
	{
		case GENERAL_DEV_UPDATE:
			break;
		case GENERAL_QOS_UPDATE:
			if(nvram_get_int("qos_enable") == 1 && nvram_get_int("qos_type") == 1){
				char bwdpi_app_rulelist[128] = {0};
				strlcpy(bwdpi_app_rulelist, nvram_safe_get("bwdpi_app_rulelist"), sizeof(bwdpi_app_rulelist));
				if(strstr(bwdpi_app_rulelist, "game"))
					json_object_object_add(payload, "mode", json_object_new_string("game"));
				else if(strstr(bwdpi_app_rulelist, "media"))
					json_object_object_add(payload, "mode", json_object_new_string("media"));
				else
					json_object_object_add(payload, "mode", json_object_new_string("normal"));
			}else
				json_object_object_add(payload, "mode", json_object_new_string("normal"));
			json_object_object_add(nt_root, "payload", payload);
			break;
		case GENERAL_TOGGLE_STATES_UPDATE:
			if(sub_event != NULL){
				if(!strcmp(sub_event, "wps"))
					json_object_object_add(payload, "wps", json_object_new_string((nvram_get_int("wps_enable") == 1)?"ture":"false"));
				else if(!strcmp(sub_event, "guestnetwork")){
					int wl_unit = 0, max_sub_unit = 0;
					char wl_ifnames[32] = {0}, word[256]={0}, wl_bss_enabled[32] = {0};
					char *next=NULL;
					strlcpy(wl_ifnames, nvram_safe_get("wl_ifnames"), sizeof(wl_ifnames));
					foreach(word, wl_ifnames, next) {
						max_sub_unit = num_of_mssid_support(wl_unit);
						snprintf(wl_bss_enabled, sizeof(wl_bss_enabled), "wl%d.%d_bss_enabled", wl_unit++, max_sub_unit);
						if(nvram_get_int(wl_bss_enabled) == 1){
							json_object_object_add(payload, "guestnetwork", json_object_new_string("ture"));
							break;
						}
						json_object_object_add(payload, "guestnetwork", json_object_new_string("false"));
					}
				}
				json_object_object_add(nt_root, "payload", payload);
			}
			break;
	}

	snprintf(str, sizeof(str), "0x%x", NT_EVENT_FLAG);
	snprintf(json_str, sizeof(json_str), "%s", json_object_to_json_string(nt_root));
	eval("Notify_Event2NC", str, json_str);
	//SEND_NT_EVENT(NT_EVENT_FLAG, json_object_to_json_string(nt_root));

	if(payload) json_object_put(payload);
	if(nt_root) json_object_put(nt_root);
}
#endif
