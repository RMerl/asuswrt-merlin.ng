/*
 * Copyright 2023, ASUSTeK Inc.
 * All Rights Reserved.
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
#include <shared.h>
#include <syslog.h>
#include <bcmnvram.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/reboot.h>
#include <sys/sysinfo.h>

#include "PsElib.h"
#include "poe.h"

#define DEVICE0         0x74   

#define MAX_RETRY		      30
#define IP808_MV_VALID_UPPER		58000
#define IP808_MV_VALID_LOWER		47000
#define IP808_MV_VALID_DEFAULT		52000

#define NORMAL_PERIOD           6               /* second */

typedef unsigned char u8_t;
typedef unsigned long u32_t;

static sigset_t sigs_to_catch;
static unsigned int signals_noticed = 0;
static struct itimerval itv;
static unsigned long long diff;

static unsigned int poll_t = 0;

extern unsigned int poemon_debug_level;
static int sw_overload = 1;
static int icut_adj = ICUT_ADJ, total_adj = TOTAL_ADJ;

enum {
	CONF_PRIO,
	CONF_STATE,
	CONF_BUDGET,
	CONF_MAX,
};

typedef struct poe_port_info_t {
        unsigned char id;
	unsigned char conf_port_prio;
	unsigned char conf_poe_enable;	/* disable, enable */
	unsigned char conf_port_limit;
	unsigned char port_limit_update;
        unsigned char poe_enable;	/* disable, enable */
        unsigned char poe_link;		/* has poe client */
	unsigned int  port_mA;
	unsigned int  port_W;
} POE_PORT_INFO_T;

static POE_PORT_INFO_T  poe_port_info[POEMON_PORTS];
// normally Vmain is 53.8
static u32_t Vmain = 0, Max_Power_mA = 0, Alert_Power_mA_on = 0, Alert_Power_mA_off = 0; // Recover_Power_mA = 0, 
static u32_t total_W = 0, total_mA = 0;
static unsigned short Max_Total_limit = 0, Max_Port_Icut = 0;

char *poe_Conf[] =
{
        "priority",
        "enable",
        "limit",
        NULL
};

static void _note_sig(int signo) {
        signals_noticed |= 1<<signo;
}

static void *fn_acts[_NSIG];

static void chld_reap_local(int sig)
{
        chld_reap(sig);
}

static void
alarmtimer(unsigned long sec, unsigned long usec)
{
        itv.it_value.tv_sec = sec;
        itv.it_value.tv_usec = usec;
        itv.it_interval = itv.it_value;
        setitimer(ITIMER_REAL, &itv, NULL);
}

static void catch_sig(int sig)
{
        if (sig == SIGUSR1)
        {
                POEMON_DBG("[poemon] dump poe info\n");

                alarmtimer(poll_t, 0);
        }
}

typedef void (*mon_func)(int sig);
static mon_func _ep=NULL;

void _watch_notices()
{
        int sig;
        for (sig = 0; sig < (_NSIG - 1); sig++) {
                if(signals_noticed & 1<<sig  &&  (_ep=fn_acts[sig])) {
                        (*_ep)(sig);
                        signals_noticed &= ~(1<<sig);
                }
        }
}

u32_t Watt2mA(u32_t Watt)	// needs Vmain first
{
        u32_t Current_mA;

	if(!Vmain)  return 0;

        //Current_mA = (u32_t)Watt;
        Current_mA = 1000*Watt;
        Current_mA = (Current_mA / Vmain)+1;
        return (u32_t)(Current_mA & 0xFFFF);
}

unsigned short Watt2mA_reg54(unsigned short watt)
{
        unsigned short tmp;

        tmp = watt*1000 / Vmain;
	tmp += total_adj;
        tmp <<= 2;
        
        return tmp;
}

unsigned short Watt2mA_regIcut(unsigned short watt)
{
        unsigned short tmp;

	if(watt>=30 && watt!=100)
		return 0xB5;	// ic default 
	else if(watt == 100)
		watt = 30;	// test eval

        tmp = watt*1000 / Vmain;
        tmp = tmp / 3.5359;
	tmp += icut_adj;

        return tmp;
}

void get_Vmain()
{
	u8_t j;
	u32_t tempV = 0;

	for(j=0 ; j < MAX_RETRY ; j++) {
		get_Vmain_mV(DEVICE0, &tempV);
		POEMON_DBG3("%s: get v:%d (retry:%d)\n", __func__, tempV, j);

		if (tempV>IP808_MV_VALID_LOWER && tempV<IP808_MV_VALID_UPPER)
                        break;

                usleep(10000);
        }
	if (j==MAX_RETRY) {
		POEMON_DBG("getVmain exceed retry, set as default\n");
		tempV = IP808_MV_VALID_DEFAULT;
	}

	tempV = tempV/1000;

        if ((tempV) != Vmain)
        {
                Vmain = tempV;

                Max_Power_mA = Watt2mA(MAX_TOTAL_POWER);
		Alert_Power_mA_on = Watt2mA(ALERT_LED_ON);
		Alert_Power_mA_off = Watt2mA(ALERT_LED_OFF);
		//Recover_Power_mA = Watt2mA(POWER_RECOVER);

                POEMON_DBG("\n%s: Vmain=%d, max_mA=%d, alert_on=%d mA, alert_off=%d mA\n", __func__, Vmain, Max_Power_mA, Alert_Power_mA_on, Alert_Power_mA_off);
	}
}

int choose_prio_port(int enable)	// choose highest_prio port when enable port; lowest_prio port when disable port
{
	int i, tp = -1;

	for(i=0; i<POEMON_PORTS; ++i) {
		if(
		  (enable==0 && (poe_port_info[i].poe_enable == STATE_ENABLE)  &&  poe_port_info[i].poe_link) ||
		  (enable==1 && (poe_port_info[i].poe_enable == STATE_DISABLE) && !poe_port_info[i].poe_link)
		) {
			if((tp == -1) || 
			  (enable==0 && (poe_port_info[tp].conf_port_prio < poe_port_info[i].conf_port_prio)) ||
			  (enable==1 && (poe_port_info[tp].conf_port_prio > poe_port_info[i].conf_port_prio))
			)
				tp = i;
		}
	}

	if(tp >= 0)
		POEMON_DBG("%s pwr: choosed %s_prio port=%d\n", enable?"resume":"disable", enable?"highest":"lowest", tp);

	return tp;
}

int pwr_budget_ctrl(int enable)
{
	int i, tp = -1, ret = 0;

	tp = choose_prio_port(enable);

	if(tp >= 0) {
		ret = set_port_state(DEVICE0, i, enable);
		POEMON_DBG("%s: %s port%d\n", __func__, enable?"enable":"disable", tp);
		if(ret < 0)
			POEMON_DBG("%s, port%d reset_state ret=%d.\n", __func__, i, ret);
	} //else 
	//	POEMON_DBG("%s: idle.\n", __func__);

	return ret;
}

void get_poe_ports_info()
{
	int i, ret;
	unsigned char ps;

	get_Vmain();
	get_power_status(DEVICE0, &ps);

	total_W = 0;
	total_mA = 0;
	POEMON_DBG("%s: poe_info.:\n-----------------\n", __func__);
	for(i=0; i<POEMON_PORTS; ++i) {
		get_port_state(DEVICE0, i, &poe_port_info[i].poe_enable);
		poe_port_info[i].poe_link = ps & (1<<i);
		ret = get_port_current_mA(DEVICE0, i, &poe_port_info[i].port_mA);
		poe_port_info[i].port_W = (poe_port_info[i].port_mA * Vmain) / 1000;
		total_mA += poe_port_info[i].port_mA;
		total_W += poe_port_info[i].port_W;

		POEMON_DBG("port-[%d]: <%s> <%s>, mA=%d(err:%d), W=%d\n", i, poe_port_info[i].poe_enable?"EN":"DIS", poe_port_info[i].poe_link?"Y":"N", poe_port_info[i].port_mA, ret, poe_port_info[i].port_W);
	}
	POEMON_DBG("now total_mA=%d, total_W=%d\n", total_mA, total_W);
}

void poemon(int sig)
{
	static int led_on=0, led_pre=0;

	get_poe_ports_info();

	if(nvram_match("AllLED", "0")) {
		led_control(LED_POE_ALERT, LED_OFF);
		led_on = 0;
	} else if(sw_overload == 1) {
		if((total_mA >= Alert_Power_mA_on) || nvram_match("force_poeAlert", "1")) {
			led_on = 1;
			if(led_on != led_pre) {
				POEMON_DBG("poe consumption over 80%!(%d)\n", total_mA);
				syslog(LOG_NOTICE, "POE overload!\n");

				led_control(LED_POE_ALERT, LED_ON);
			}

			//pwr_budget_ctrl(PoE_DISABLE);

		} else if((total_mA <= Alert_Power_mA_off) || nvram_match("force_poeAlert", "0")){
			led_on = 0;
			if(led_on != led_pre) {
				POEMON_DBG("poe consumption alleviated.(%d)\n", total_mA);
				led_control(LED_POE_ALERT, LED_OFF);
			}
			//if(total_mA <= Recover_Power_mA)
			//	pwr_budget_ctrl(PoE_ENABLE);
		}
	}
	led_pre = led_on;
}

void poemon_init_sig()
{
        int sig;
        for (sig = 0; sig < (_NSIG - 1); sig++) {
                if(sig == SIGCHLD
                || sig == SIGUSR1
                || sig == SIGALRM
                )
                signal(sig, _note_sig);
                fn_acts[sig] = NULL;
        }

        fn_acts[SIGCHLD] = chld_reap_local;
        fn_acts[SIGUSR1] = catch_sig;
        fn_acts[SIGALRM] = poemon;

        sigemptyset(&sigs_to_catch);
        sigaddset(&sigs_to_catch, SIGCHLD);
        sigaddset(&sigs_to_catch, SIGUSR1);
        sigaddset(&sigs_to_catch, SIGALRM);
}

int get_conf(int port, int conf)
{
	char tmp[64], prefix[] = "poeXXXXXXXXXX_";
	unsigned int ret = 0;

	if(conf >= CONF_MAX)
		return -1;
#ifdef EBG19P
        snprintf(prefix, sizeof(prefix), "poe%d_", port+1);
#else
        snprintf(prefix, sizeof(prefix), "poe%d_", port+1);
#endif

	switch(conf) {
		case CONF_STATE:
			if(!nvram_get(strcat_r(prefix, poe_Conf[conf], tmp)))
				return 1;
		default:
	        	ret = atoi(nvram_safe_get(strcat_r(prefix, poe_Conf[conf], tmp)));
			break;
	}

	//_dprintf("%s, nv:%s, ret=%d\n", __func__, strcat_r(prefix, poe_Conf[conf], tmp), ret);
	return ret;
}

int diff_pwrConf(int port)
{
	char tmp[64], prefix[] = "pre_poeXXXXXXXXXX_";
	int ret = 0;

#ifdef EBG19P
        snprintf(prefix, sizeof(prefix), "pre_poe%d_", port+1);
#else
        snprintf(prefix, sizeof(prefix), "pre_poe%d_", port+1);
#endif
	ret = (atoi(nvram_safe_get(strcat_r(prefix, poe_Conf[CONF_BUDGET], tmp))) != get_conf(port, CONF_BUDGET));

	return ret;
}

void record_pwrConf(int port, int value)
{
	char tmp[64], prefix[] = "pre_poeXXXXXXXXXX_";

#ifdef EBG19P
        snprintf(prefix, sizeof(prefix), "pre_poe%d_", port+1);
#else
        snprintf(prefix, sizeof(prefix), "pre_poe%d_", port+1);
#endif
	nvram_set_int(strcat_r(prefix, poe_Conf[CONF_BUDGET], tmp), value);
}

int poe_init_conf(int update)
{
	int i, ret;
	unsigned char data_byte;
	unsigned short data_short;
	unsigned int max_port_cur;

	icut_adj = nvram_get_int("icut_adj")?:ICUT_ADJ;
	total_adj = nvram_get_int("total_adj")?:TOTAL_ADJ;

	poll_t = nvram_get_int("pm_poll_t")?:NORMAL_PERIOD;
	if(!poemon_debug_level)
		poemon_debug_level = nvram_get_hex("poemon_debug_level");

	memset(poe_port_info, 0, sizeof(poe_port_info));
	for(i=0; i<POEMON_PORTS; ++i) {
		poe_port_info[i].id = i;
		poe_port_info[i].conf_port_prio = get_conf(i, CONF_PRIO);
		poe_port_info[i].conf_poe_enable = get_conf(i, CONF_STATE);
		if(update)
			poe_port_info[i].port_limit_update = diff_pwrConf(i);
		else
			poe_port_info[i].port_limit_update = 0;
		poe_port_info[i].conf_port_limit = get_conf(i, CONF_BUDGET);
		record_pwrConf(i, poe_port_info[i].conf_port_limit);

		ret = set_port_state(DEVICE0, i, poe_port_info[i].conf_poe_enable);
		if(ret < 0)
			POEMON_DBG("%s, port%d conf_state=%d.\n", __func__, i, ret);
	}

	ret = set_ivt_auto_poll(DEVICE0, 1);
	
	POEMON_DBG("%s, enable ivt_auto_poll:%d. v:0x%x, s:%d, icut_adj:%d, total_adj=%d\n", __func__, ret, poemon_debug_level, sw_overload, icut_adj, total_adj);
	for(i=0; i<POEMON_PORTS; ++i) {
		POEMON_DBG("%s, port[%d]-state=%d, -prio=%d, -limit=%d, -update=%d\n", __func__, i, poe_port_info[i].conf_poe_enable, poe_port_info[i].conf_port_prio, poe_port_info[i].conf_port_limit, poe_port_info[i].port_limit_update);
	}

	/* Enable Total Power Limit */
	ret = set_TotalCurrentLimit_Enable(DEVICE0, ENABLE);
	POEMON_DBG("set_TotalCurrentLimit_Enable:%d\n", ret);

	/* set Port Priority */
	for(i=0; i<POEMON_PORTS; ++i) {
		ret = set_port_priority(DEVICE0, i, poe_port_info[i].conf_port_prio);
		POEMON_DBG("set_port_priority:0x%x, ret=%d\n", poe_port_info[i].conf_port_prio, ret);
		ret = get_port_priority(DEVICE0, i, &data_byte);
		POEMON_DBG("get_port_priority:0x%x, ret=%d\n", data_byte, ret);
	}

	get_Vmain();
	/* set Max Total limit */
	Max_Total_limit = Watt2mA_reg54(MAX_TOTAL_POWER); 
	ret = set_Available_current(DEVICE0, Max_Total_limit);
	POEMON_DBG("set_Available_current(0x%x)(%dmA), ret=%d\n", Max_Total_limit, Max_Total_limit/4, ret);
	ret = get_Available_current(DEVICE0, &data_short);
	POEMON_DBG("get_Available_current:0x%x , ret=%d\n", data_short, ret);

#if 0	// ip808 needs extra serial ic to output led, so not use it
	/* set Overload-led threshold */
	ret = set_Overloadled_threshold(DEVICE0, OVERLOAD_MA);
	POEMON_DBG("set_Overloadled_threshold, ret=%d\n", ret);
	ret = get_Overloadled_threshold(DEVICE0, &data_short);
	POEMON_DBG("get_Overloadled_threshold:0x%x, ret=%d\n", data_short, ret);

	/* enable Overload-led */
	ret = set_Overloadled_enable(DEVICE0, ENABLE);
	POEMON_DBG("set_Overloadled_enable, ret=%d\n", ret);
	ret = get_Overloadled_enable(DEVICE0, &data_byte);
	POEMON_DBG("get_Overloadled_enable=%d, ret=%d\n", data_byte, ret);
#endif

	/* set power mode as HDPL */
	ret = set_power_mode(DEVICE0, 0);

	/* set Max per-Port Cur*/
	for(i=0; i<POEMON_PORTS; ++i) {
		Max_Port_Icut = Watt2mA_regIcut(poe_port_info[i].conf_port_limit); 
		ret = set_def_port_Icut(DEVICE0, i, Max_Port_Icut);
		max_port_cur = Max_Port_Icut*3.5359;
		POEMON_DBG("set_def_port_Icut(0x%x)(%dmA), ret=%d\n", Max_Port_Icut, max_port_cur, ret);
		ret = get_def_port_Icut(DEVICE0, i, &data_byte);
		POEMON_DBG("get_def_port_Icut:0x%x , ret=%d\n", data_byte, ret);
	}

	for(i=0; i<POEMON_PORTS; ++i) {
		if(poe_port_info[i].port_limit_update) {
			POEMON_DBG("re-power poe-port_%d due limit-updated\n", i);
			set_port_state(DEVICE0, i, OFF);
			usleep(400*1000);
			set_port_state(DEVICE0, i, ON);
		}
	}

	//get_poe_ports_info();
	
	return 0;
}

int
poemon_main(int argc, char *argv[])
{
        FILE *fp;
        const struct mfg_btn_s *p;
	int ret, c, daemon = 1;

	POEMON_DBG("poemon start...\n");

	poemon_debug_level = 0;
        while ((c = getopt(argc, argv, "v")) != -1) {
                switch (c) {
                //case 'd':
		//	daemon = 1;
                //        break;
                case 'v':
			poemon_debug_level = POEMON_DEBUG_DBG | POEMON_DEBUG_DBG2;
                        break;
                //case 's':
		//	sw_overload = 1;
                //        break;
                default:
                        fprintf(stderr, "ERROR: unknown option %c\n", c);
                        break;
                }
        }

	PsE_init_lib(DEVICE0);
	set_led_order(DEVICE0, 0);
	poe_init_conf(0);

	if(!daemon)
		return 0;

        /* write pid */
        if ((fp = fopen("/var/run/poemon.pid", "w")) != NULL)
        {
                fprintf(fp, "%d", getpid());
                fclose(fp);
        }

	poemon_init_sig();
        alarmtimer(poll_t, 0);

        /* Most of time it goes to sleep */
        while (1)
        {
                while(signals_noticed)
                        _watch_notices();

                pause();
        }

	return 0;
}


int
poeInit_main(int argc, char *argv[])
{
	poemon_debug_level = 0;

	nvram_set_int("poe_init", getpid());
	return poe_init_conf(1);
}

