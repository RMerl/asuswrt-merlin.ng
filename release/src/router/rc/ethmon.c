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

#define NORMAL_PERIOD           5               /* second */
#define REC_N			7
#ifdef EBG15
#define ETHMON_PORTS		4
#elif defined(EBG19P)
#define ETHMON_PORTS		8
#endif

#define PERIOD_1		30
//#define PERIOD_2		30

#define ETHMON_DEBUG_DBG	0x0001
#define ETHMON_DEBUG_SYSLOG	0x0002

#define BCAST_JAM_THRESHOLD	100
#define PHY_RESET_TIME		8

typedef struct ethmon_ports_info {
	char ifname[12];
	char label_name[12];
	int port_id;
	unsigned long ctrl_reg;
	int disrx;
	unsigned long long rx_bcast, pre_rx_bcast, rx_diff[REC_N];
} ETHMON_PORT_INFO_T;

typedef struct {
        int        	port_id;
        unsigned long	reg_addr;
} Portctrl_Reg;

Portctrl_Reg  preg[] = {
	{0, 0x80080000},
	{1, 0x80080004},
	{2, 0x80080008},
	{3, 0x8008000c},
	{4, 0x80080010},
	{5, 0x80080014},
	{7, 0x8008001c},
	{8, 0x80080020},
	{-1, 0}
};

#define ethmondbg(fmt, args...) do { FILE *fp = fopen("/dev/console", "w"); if (fp) { fprintf(fp, fmt, ## args); fclose(fp); } else fprintf(stderr, fmt, ## args); } while (0)

static int period1_event = 0;
static int period2_count = 0;
static sigset_t sigs_to_catch;
static unsigned int signals_noticed = 0;
static struct itimerval itv;
static unsigned long long diff;

ETHMON_PORT_INFO_T  ethmon_info[ETHMON_PORTS];
extern int ethmon_debug_level = 0;
phy_port_mapping port_mapping;
unsigned int bcast_jam_thresh = 0, poll_t = 0;
int phy_reset_time = 0;

#if 0
#define ETHMON_DBG(fmt, arg...) \
		do { \
			if (ethmon_debug_level & ETHMON_DEBUG_DBG) \
				ethmondbg("ethmon: "fmt, ##arg); \
			if (ethmon_debug_level & ETHMON_DEBUG_SYSLOG) \
				syslog(LOG_NOTICE, fmt, ##arg); \
		} while (0)
#endif

#define ETHMON_DBG(fmt, arg...) \
		do { \
			if (ethmon_debug_level & ETHMON_DEBUG_DBG) \
				ethmondbg("ethmon: "fmt, ##arg); \
		} while (0)

void note_sig(int signo) {
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
                ETHMON_DBG("[ethmon] dump eth ports info\n");

                alarmtimer(poll_t, 0);
        }
}

typedef void (*dog_func)(int sig);
dog_func ep=NULL;

void watch_notices()
{
        int sig;
        for (sig = 0; sig < (_NSIG - 1); sig++) {
                if(signals_noticed & 1<<sig  &&  (ep=fn_acts[sig])) {
                        (*ep)(sig);
                        signals_noticed &= ~(1<<sig);
                }
        }
}

int 
bcast_jam(unsigned long long *rx_diff)
{
	int i, counts = 0;
	
	for(i=0; i<REC_N; ++i) {
		if(rx_diff[i] >= bcast_jam_thresh)
			counts++;
	}

	if(counts >= (REC_N-1))
		return 1;

	return 0;
}

void
add_rxdiff(int k, unsigned long long data)
{
	int i;

	for(i=REC_N-1; i>0; --i) {
		ethmon_info[k].rx_diff[i] = ethmon_info[k].rx_diff[i-1];
	}
	ethmon_info[k].rx_diff[0] = data;
}

int get_ethmon_info(char *ifname)
{
	int i;

	for(i=0; i<ETHMON_PORTS; ++i) {
		if(strncmp(ethmon_info[i].ifname, ifname, strlen(ifname)) == 0)
			return i;
	}
	return -1;
}

int get_ethmon_idx(int id)
{
	int i;

	for(i=0; i<ETHMON_PORTS; ++i) {
		if(ethmon_info[i].ctrl_reg==0 && ethmon_info[i].port_id == id)
			return i;
	}
	return -1;
}

unsigned long get_preg(int pd)
{
	int i;
	unsigned long rdata = 0;

	for(i=0; preg[i].port_id!=-1; ++i) {
		if(pd == preg[i].port_id && preg[i].reg_addr!=-1) {
			rdata = preg[i].reg_addr;
			break;
		}
	}

	return rdata;
}

void dis_rx(int rec_k, char *ifname, unsigned long ctrl_reg)
{
        FILE *fp = NULL;
        char cmd[64], buf[32], buf2[32], *cp;
	unsigned long rdata = 0;
	int i;
	
	snprintf(cmd, sizeof(cmd), "dw 0x%lx", ctrl_reg);
	//ETHMON_DBG("%s, cmd: %s\n", __func__, cmd);

	fp = popen(cmd, "r");
	if(fp) {
		while (fgets(buf, sizeof(buf), fp) != NULL) {
			//ETHMON_DBG("%s: read buf is %s\n", __func__, buf);
			if((cp = strstr(buf, ":")) != NULL) {
				//ETHMON_DBG("%s: cp is %s\n", __func__, cp);
				rdata = strtoul(cp+1, NULL, 16);
				//ETHMON_DBG("%s: rdata is %lx\n", __func__, rdata);
				break;
			}
		}
		//ETHMON_DBG("%s: get rdata=0x%lx\n", ifname, rdata);
		rdata |= 0x1;
		ETHMON_DBG("%s: reset rdata=0x%lx\n", ifname, rdata);
		pclose(fp);
	}

	if(rdata) {
		eval("ethctl", ifname, "phy-reset");
		memset(ethmon_info[rec_k].rx_diff, 0, sizeof(ethmon_info[rec_k].rx_diff));
		ETHMON_DBG("%s: ethctl %s phy-reset\n", __func__, ifname);

		snprintf(buf,  sizeof(buf),  "0x%lx", ctrl_reg);
		snprintf(buf2, sizeof(buf2), "0x%lx", rdata);

		for(i=0; i<phy_reset_time; ++i){
			ETHMON_DBG("wait reset:%d/%d\n", i+1, phy_reset_time);
			sleep(1);
		}
		eval("sw", buf, buf2);
		ETHMON_DBG("%s: sw %s %s\n", __func__, buf, buf2);
	}
}

void ethmon(int sig)
{
        FILE *fp = NULL;
        char cmd[64], buf[256], *cp;
	int i = 0, k = 0, linked = 0, ret;
	unsigned long long data = 0;
	char bcast_str[32];

        period1_event = (period1_event + 1) % PERIOD_1;

	for (i=0; i<(port_mapping.count); i++) {
                if ((port_mapping.port[i].cap & PHY_PORT_CAP_LAN) == 0)
                        continue;
	
                if (port_mapping.port[i].ext_port_id == -1) {
			ret = hnd_get_phy_status(port_mapping.port[i].ifname);
			k = get_ethmon_info(port_mapping.port[i].ifname);
			sprintf(bcast_str, "rx_bcast_packets");
		} else {
#if defined(EBG19P)
			ret = rtk_get_phy_status(port_mapping.port[i].ext_port_id);
#endif
			k = get_ethmon_idx(port_mapping.port[i].ext_port_id);
			sprintf(bcast_str, "etherStatsBcastPkts");
		}

		if(ret > 0) {
                	if (port_mapping.port[i].ext_port_id == -1) {
				snprintf(cmd, sizeof(cmd), "ethctl %s stats", port_mapping.port[i].ifname);
			} else {
				snprintf(cmd, sizeof(cmd), "rtkswitch 1 %d", port_mapping.port[i].ext_port_id);
			}

			//ETHMON_DBG("[%s/%s] chk idx:%d, cmd:%s\n", ethmon_info[k].ifname, ethmon_info[k].label_name, k, cmd);

			fp = popen(cmd, "r");
			if(fp) {
				while (fgets(buf, sizeof(buf), fp) != NULL) {
					if((cp = strstr(buf, bcast_str)) != NULL)
					{
						for(;(*cp != ':') && (*cp != 0); cp++);
						data = strtoull(cp+1, NULL, 10);
						//ETHMON_DBG("[%s/%s] data is %llu\n", ethmon_info[k].ifname, ethmon_info[k].label_name, data);

						if(ethmon_info[k].rx_bcast)
							ethmon_info[k].pre_rx_bcast = ethmon_info[k].rx_bcast;
						ethmon_info[k].rx_bcast = data;
						if(ethmon_info[k].rx_bcast && ethmon_info[k].pre_rx_bcast) {
							diff = ethmon_info[k].rx_bcast - ethmon_info[k].pre_rx_bcast;
							//ETHMON_DBG("[%s/%s] diff is %llu\n", ethmon_info[k].ifname, ethmon_info[k].label_name,  diff);

							add_rxdiff(k, diff);
						}
						break;
					}
				}
				pclose(fp);
			}
			ETHMON_DBG("[%s] +bcast: %lld/ %lld/ %lld/ %lld/ %lld/ %lld/ %lld\n", ethmon_info[k].ifname, ethmon_info[k].rx_diff[0], ethmon_info[k].rx_diff[1], ethmon_info[k].rx_diff[2], ethmon_info[k].rx_diff[3], ethmon_info[k].rx_diff[4], ethmon_info[k].rx_diff[5], ethmon_info[k].rx_diff[6]);

			if(bcast_jam(&ethmon_info[k].rx_diff) || nvram_match("bcast_test", "1")) {
				ETHMON_DBG("[%s/%s] bcast flooding! %s\n", ethmon_info[k].ifname, ethmon_info[k].label_name, (port_mapping.port[i].ext_port_id == -1)?"dis_rx.":"");
				syslog(LOG_NOTICE, "[%s/%s] bcast flooding! %s\n", ethmon_info[k].ifname, ethmon_info[k].label_name, (port_mapping.port[i].ext_port_id == -1)?"dis_rx.":"");
				syslog(LOG_NOTICE, "[%s/%s] +bcast: %lld/ %lld/ %lld/ %lld/ %lld/ %lld/ %lld\n", ethmon_info[k].ifname, ethmon_info[k].label_name, ethmon_info[k].rx_diff[0], ethmon_info[k].rx_diff[1], ethmon_info[k].rx_diff[2], ethmon_info[k].rx_diff[3], ethmon_info[k].rx_diff[4], ethmon_info[k].rx_diff[5], ethmon_info[k].rx_diff[6]);

                		if (port_mapping.port[i].ext_port_id == -1) 
					dis_rx(k, ethmon_info[k].ifname, ethmon_info[k].ctrl_reg);
			}
		}
	}

        if (period1_event)
                return;


//emp2:	/* period2 tasks */

}

void ethmon_init_sig()
{
        int sig;
        for (sig = 0; sig < (_NSIG - 1); sig++) {
                if(sig == SIGCHLD
                || sig == SIGUSR1
                || sig == SIGALRM
                )
                signal(sig, note_sig);
                fn_acts[sig] = NULL;
        }

        fn_acts[SIGCHLD] = chld_reap_local;
        fn_acts[SIGUSR1] = catch_sig;
        fn_acts[SIGALRM] = ethmon;

        sigemptyset(&sigs_to_catch);
        sigaddset(&sigs_to_catch, SIGCHLD);
        sigaddset(&sigs_to_catch, SIGUSR1);
        sigaddset(&sigs_to_catch, SIGALRM);
}

void init_conf()
{
	int i, k=0;

	memset(ethmon_info, 0, sizeof(ethmon_info));
	ethmon_debug_level = nvram_get_int("etm_debug_level");
	bcast_jam_thresh = nvram_get_int("etm_bcast_thresh")?:BCAST_JAM_THRESHOLD;
	poll_t = nvram_get_int("etm_poll_t")?:NORMAL_PERIOD;
	phy_reset_time = nvram_get_int("etm_phy_reset_time")?:PHY_RESET_TIME;

	ETHMON_DBG("ethmon init, debug:%d, bcast_jam_thresh:%u, poll_time=%d, phy_reset_time=%d\n", ethmon_debug_level, bcast_jam_thresh, poll_t, phy_reset_time);

        get_phy_port_mapping(&port_mapping);
	for (i=0; i<(port_mapping.count); i++) {
                if ((port_mapping.port[i].cap & PHY_PORT_CAP_LAN) == 0)
                        continue;
                //if (port_mapping.port[i].ext_port_id != -1)
                //        continue;
		if(k >= ETHMON_PORTS)
			break;

		ETHMON_DBG("chk portmapping[%d]:[%s/%s], id[%d]\n", i, port_mapping.port[i].ifname, port_mapping.port[i].label_name, port_mapping.port[i].phy_port_id);

		strlcpy(ethmon_info[k].ifname, port_mapping.port[i].ifname, sizeof(ethmon_info[k].ifname));
		strlcpy(ethmon_info[k].label_name, port_mapping.port[i].label_name, sizeof(ethmon_info[k].label_name));
		ethmon_info[k].port_id = port_mapping.port[i].phy_port_id;
                if (port_mapping.port[i].ext_port_id == -1)
			ethmon_info[k].ctrl_reg = get_preg(ethmon_info[k].port_id);
		else
			ethmon_info[k].ctrl_reg = 0;
		ethmon_info[k].disrx = 0;
		memset(ethmon_info[k].rx_diff, 0, sizeof(ethmon_info[k].rx_diff));

		ETHMON_DBG("init [%s/%s], port_id=%d, ctrl_reg=%lx\n", ethmon_info[k].ifname, ethmon_info[k].label_name, ethmon_info[k].port_id, ethmon_info[k].ctrl_reg);
		k++;
	}

	ETHMON_DBG("\ndump ethmon info:\n");
	for(i=0; i<ETHMON_PORTS; ++i) {
		ETHMON_DBG("[%s/%s], port_id=%d, ctrl_reg=%lx\n", ethmon_info[i].ifname, ethmon_info[i].label_name, ethmon_info[i].port_id, ethmon_info[i].ctrl_reg);
	}
}

int
ethmon_main(int argc, char *argv[])
{
        FILE *fp;
        const struct mfg_btn_s *p;

	if(nvram_match("etm_disable", "1"))
		return 0;

        /* write pid */
        if ((fp = fopen("/var/run/ethmon.pid", "w")) != NULL)
        {
                fprintf(fp, "%d", getpid());
                fclose(fp);
        }

	ethmon_init_sig();
	init_conf();

        alarmtimer(poll_t, 0);

        /* Most of time it goes to sleep */
        while (1)
        {
                while(signals_noticed)
                        watch_notices();

                pause();
        }

	return 0;
}
