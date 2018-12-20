#include <stdio.h>
#include <syslog.h>

#if defined(RTCONFIG_RALINK)
#include <bcmnvram.h>
#include <ralink.h>
#include <net/ethernet.h>
#include <netinet/ether.h>
#include  <float.h>
#ifdef RTCONFIG_WIRELESSREPEATER
#include <ap_priv.h>
#endif
#elif defined(RTCONFIG_QCA)
#include <bcmnvram.h>
#include <net/ethernet.h>
#include <netinet/ether.h>
#elif defined(RTCONFIG_ALPINE)
#include <bcmnvram.h>
#include <net/ethernet.h>
#include <netinet/ether.h>
#else
#include <wlioctl.h>
#include <bcmendian.h>
#endif
#include <wlutils.h>


#define NORMAL_PERIOD 600 /* second */

static int diag_dbg = 0;
static int diag_syslog = 0;

enum {
	STATE_PHY_DISCONN = 0,
	STATE_PHY_CONN,
	STATE_MODEM_SCAN,
	STATE_MODEM_RESET,
	STATE_MODEM_NOSIM,
	STATE_MODEM_LOCK_PIN,
	STATE_MODEM_LOCK_PUK,
	STATE_MAX
};

enum {
	DIAGMODE_NONE = 0,
	DIAGMODE_CHKSTA = 1,
	DIAGMODE_SYS_DETECT = 2,
	DIAGMODE_SYS_SETTING = 4,
	DIAGMODE_WIFI_DETECT = 8,
	DIAGMODE_WIFI_SETTING = 16,
	DIAGMODE_STAINFO = 32,
	DIAGMODE_NET_DETECT = 64,
	DIAGMODE_MAX
};

#define FIELD_SYS "SYS" // FIELD_SYS>node type>IP>MAC>FW version>t-code>AiProtection>USB mode>CTF or Runner/FC disable>got_previous_oops
#define FIELD_SYS2 "SYS2" // FIELD_SYS2>node type>IP>MAC>CPU freq>MemFree>CPU temperature>2G's temperature>5G's temperature
#define FIELD_WIFISYS "WIFISYS" // FIELD_WIFISYS>node type>IP>MAC>2G's ifname,2G's chip,2G's country/rev>5G's ifname,5G's chip,5G's country/rev
#define FIELD_WIFISYS2 "WIFISYS2" // FIELD_WIFISYS2>node type>IP>MAC>2G's ifname,2G's noise,2G's MCS>5G's ifname,5G's noise,5G's MCS
#define FIELD_STAINFO "STAINFO" // FIELD_STAINFO>node type>IP>MAC>Band>STA's MAC>active>STA's RSSI>STA's Tx>STA's Rx
#define FIELD_WLCE "WLCEVENT" // FIELD_WLCE>node type>IP>MAC>STA's MAC>last_act>count_auth>count_deauth>count_assoc>count_disassoc>count_reassoc
#define FIELD_TG_ROAMING "TG_ROAMING" // FIELD_TG_ROAMING>node type>IP>MAC>timestamp>user_low_rssi>rssi_cnt>idle_period>STA's MAC>STA's RSSI>idle_start
#define FIELD_ROAMING "ROAMING" // FIELD_ROAMING>node type>IP>MAC>timestamp>STA's MAC>STA's RSSI>candidate_rssi_criteria>candidate's MAC>candidate's RSSI
#define FIELD_NET "NET" // FIELD_NET>node type>IP>MAC>wan_unit>link_wan>runIP>runMASK>runGATE>got_Route>got Redirect rules>DNS>Ping
#ifdef RTCONFIG_BCMBSD
#define FIELD_TG_BSD "TG_BSD" // FIELD_TG_BSD>node type>IP>MAC>timestamp>STA's MAC>triggered timestamp>from_chanspec>to_chanspec>reason
#endif

#if 0
#define	LOG_EMERG	0	/* system is unusable */
#define	LOG_ALERT	1	/* action must be taken immediately */
#define	LOG_CRIT	2	/* critical conditions */
#define	LOG_ERR		3	/* error conditions */
#define	LOG_WARNING	4	/* warning conditions */
#define	LOG_NOTICE	5	/* normal but significant condition */
#define	LOG_INFO	6	/* informational */
#define	LOG_DEBUG	7	/* debug-level messages */
#endif

#define LOG_TITLE_CHK "CHKSTA"
#define LOG_TITLE_DIAG "CONNDIAG"

extern char GID[64];
extern char title_chksta[128];
extern char title_diag[128];

#define CHK_LOG(LV, fmt, arg...) \
	do { \
		if(diag_dbg >= LV) \
			_dprintf("%lu: "fmt"\n", uptime(), ##arg); \
		if(diag_syslog >= LV) \
			logmessage_lv(LV, title_chksta, fmt, ##arg); \
	} while (0)

#define DIAG_LOG(LV, fmt, arg...) \
	do { \
		if(diag_dbg >= LV) \
			_dprintf("%lu: "fmt"\n", uptime(), ##arg); \
		if(diag_syslog >= LV) \
			logmessage_lv(LV, title_diag, fmt, ##arg); \
	} while (0)

#ifdef RTCONFIG_LIBASUSLOG
#define DBG_CHK_DATA "chksta_data.log"

#define CHK_DATA(fmt, arg...) \
	do { \
		asusdebuglog(0, DBG_CHK_DATA, LOG_CUSTOM, LOG_SHOWTIME, 0, fmt, ##arg); \
	} while (0)
#else
#define CHK_DATA(fmt, arg...) \
	do { \
		logmessage("CHKSTA", fmt, ##arg); \
	} while (0)
#endif

extern void logmessage_lv(int lv, char *logheader, char *fmt, ...);

extern int get_hw_acceleration(char *output, int size);
extern int get_sys_clk(char *output, int size);
extern int get_sys_temp(unsigned int *temp);

extern int get_wifi_chip(char *ifname, char *output, int size);
extern int get_wifi_temp(char *ifname, unsigned int *temp);
extern int get_wifi_country(char *ifname, char *output, int len);
extern int get_wifi_noise(char *ifname, char *output, int len);
extern int get_wifi_mcs(char *ifname, char *output, int len);
