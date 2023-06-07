#include <stdio.h>
#include <syslog.h>
#include <time.h>
#include <linux/limits.h>

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

#include <shared.h>

#define MAX_DATA 8192
#define JSON_DATA_BUF 512
#define MAX_DATA_RAW MAX_DATA - JSON_DATA_BUF

#define MAX_RE 2
#define MACF_UP	"%02X:%02X:%02X:%02X:%02X:%02X"
#define ACS_CHANIM_BUF_LEN (2*1024)

#define LOG_DIR "/tmp/asusdebuglog"
#define DIAG_SITE_SURVEY_CD 60
#define AMAS_SSD_CD_WAIT_TIME 10
#define NORMAL_PERIOD 600 /* second */
#define DIAG_FILE_LOCK		"conn_diag"
#define CMD_MAC "CMD_MAC"
#define CABLEDIAG_TIME_BUF 15

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

#define STATUS_NOT_CHANGE 0
#define STATUS_CHANGE 1

/* these enums will transmit between nodes, so do NOT delete/modity existed enums */
enum {
	//legacy,dont change
	DIAGMODE_NONE = 0, 				//legacy,dont change
	DIAGMODE_CHKSTA = 0x1,			//legacy,dont change
	DIAGMODE_SYS_DETECT = 0x2,		//legacy,dont change
	DIAGMODE_SYS_SETTING = 0x4,		//legacy,dont change
	DIAGMODE_WIFI_DETECT = 0x8,		//legacy,dont change
	DIAGMODE_WIFI_SETTING = 0x10,	//legacy,dont change
	DIAGMODE_STAINFO = 0x20,		//legacy,dont change
	DIAGMODE_NET_DETECT = 0x40,		//legacy,dont change
	DIAGMODE_ETH_DETECT = 0x80,		//legacy,dont change
	DIAGMODE_PORTINFO = 0x100,		//legacy,dont change
	DIAGMODE_WIFI_DFS = 0x200,		//legacy,dont change
	
	//legacy,dont change

	DIAGMODE_LEGACY_MAX = 0x0FFF,




	//DIAGMODE_SITE_SURVEY = 0x400,
	DIAGMODE_MODE = 0x1000,
	DIAGMODE_STAINFO_STABLE = DIAGMODE_MODE+1,
	//DIAGMODE_MODEXXX = DIAGMODE_MODE + x-;
	DIAGMODE_ACTION = 0x2000,
	DIAGMODE_ACTION_SITE_SURVEY 	= DIAGMODE_ACTION+1,
	DIAGMODE_ACTION_SITE_SURVEY_2G 	= DIAGMODE_ACTION+2,
	DIAGMODE_ACTION_SITE_SURVEY_5G1 = DIAGMODE_ACTION+3,
	DIAGMODE_ACTION_SITE_SURVEY_5G2 = DIAGMODE_ACTION+4,
	DIAGMODE_ACTION_CABLE_DIAG 		= DIAGMODE_ACTION+5,
	DIAGMODE_ACTION_IPERF_SERVER	= DIAGMODE_ACTION+6,
	DIAGMODE_ACTION_IPERF_CLIENT	= DIAGMODE_ACTION+7,
	//DIAGMODE_ACTION
	//DIAGMODE_ACTION
	//DIAGMODE_ACTION
	DIAGMODE_EVENT = 0x4000,
	DIAGMODE_EVENT_CHANNEL_CHANGE 			= DIAGMODE_EVENT+1,
	DIAGMODE_EVENT_PORT_STATUS_CHANGE 		= DIAGMODE_EVENT+2,
	DIAGMODE_EVENT_ALL_CHAN_RADAR 			= DIAGMODE_EVENT+3,
	DIAGMODE_EVENT_CHLIST_CHANGE 			= DIAGMODE_EVENT+4,
	DIAGMODE_EVENT_PORT_STATUS_USB_CHANGE 	= DIAGMODE_EVENT+5,
	DIAGMODE_EVENT_WLC 	                    = DIAGMODE_EVENT+6,
	DIAGMODE_WIFI_CBP 	                    = DIAGMODE_EVENT+7,
	DIAGMODE_EVENT_PORT_STATUS_MOCA_CHANGE  = DIAGMODE_EVENT+8,
	//DIAGMODE_EVENT
	//DIAGMODE_SITE_SURVEY_2G = 0x800, //need modify
	//DIAGMODE_SITE_SURVEY_5G1 = 0x1000, //need modify
	//DIAGMODE_SITE_SURVEY_5G2 = 0x2000, //need modify
	//DIAGMODE_CHANNEL_CHANGE = 0x4000,
	//DIAGMODE_PORT_STATUS_CHANGE = 0x8000,
	//DIAGMODE_ALL_CHAN_RADAR = 0x10000,
	//DIAGMODE_CHLIST_CHANGE = 0x20000,
	DIAGMODE_MIX = CFG_CONNDIAG_MIX_MODE, // 0xF000, for indicating received pkt are mixed(multi-mode)
	DIAGMODE_MAX
};

// DIAGMODE_ACTION
// DIAGMODE_EVENT

typedef struct _if_stats {
	char ifname[8];
	unsigned long long tx_byte;
	unsigned long long tx_diff;
	unsigned long long rx_byte;
	unsigned long long rx_diff;
	time_t ts;
} _if_stats;

struct chanel_info {
	int control_chan;
	int center_chan;
	int bw;
};

//for ui actions to conn diag SQL 
enum {
	UI_ACTION_NONE=0,
	UI_ACTION_GET_PORTSTATUS,
	UI_ACTION_RUN_CABLEDIAG,
	UI_ACTION_RUN_IPERF,
	UI_ACTION_GET_STAINFO,  // for networkmap use
	UI_ACTION_MAX
};

//for cable diag actions
enum {
	CABLEDIAG_ACTION_NONE=0,
	CABLEDIAG_ACTION_RUN_ONE,
	CABLEDIAG_ACTION_RUN_ALL,
	CABLEDIAG_ACTION_MAX
};

//for iperf actions
enum {
	IPERF_ACTION_NONE=0,
	IPERF_ACTION_SERVER,
	IPERF_ACTION_CLIENT,
	IPERF_ACTION_MAX
};

enum {
	IPERF_TEST_STATUS_READY=0,
	IPERF_TEST_STATUS_RUNNING,
	IPERF_TEST_STATUS_SUCCESS,
	IPERF_TEST_STATUS_FAIL
};

enum {
	IPERF_TEST_ALLSTATUS_STOP=0,
	IPERF_TEST_ALLSTATUS_RUNNING
};

#define MAX_CHIN_CNT 4
#define CONNDIAG_SQL_IPC_SOCKET_PATH	"/var/run/conndiag_sql_ipc_socket"
#define CONNDIAG_PS_IPC_SOCKET_PATH	"/var/run/conndiag_portstatus_ipc_socket"
#define AWSIOT_IPC_SOCKET_PATH  "/var/run/awsiot_ipc_socket"
#define DIAG_EVENT_SYS "SYS" // DIAG_EVENT_SYS>node type>IP>MAC>FW version>t-code>AiProtection>USB mode>CTF or Runner/FC disable
#define DIAG_EVENT_SYS2 "SYS2" // DIAG_EVENT_SYS2>node type>IP>MAC>CPU freq>MemFree>CPU temperature>2G's temperature>5G's temperature
#define DIAG_EVENT_WIFISYS "WIFISYS" // DIAG_EVENT_WIFISYS>node type>IP>MAC>2G's ifname,2G's chip,2G's country/rev>5G's ifname,5G's chip,5G's country/rev
#define DIAG_EVENT_WIFIDFS "WIFIDFS"
#define DIAG_EVENT_CBP "WIFICBP"
#define DIAG_EVENT_SITESURVEY "SITESURVEY"
#define DIAG_CHANNEL_CHANGE "CHANNELCHANGE"
#define DIAG_PS_CHANGE "PORTSTATUSCHANGE"
#define DIAG_CABLE_DIAG "CABLEDIAG"
#define DIAG_PS_USB_CHANGE "PORTSTATUSUSBCHANGE"
#define DIAG_PS_MOCA_CHANGE "PORTSTATUSMOCACHANGE"
#define DIAG_EVENT_IPERF_SERVER "IPERFSERVER"
#define DIAG_EVENT_IPERF_CLIENT "IPERFCLIENT"
#ifdef RTCONFIG_BCMARM
#define DIAG_EVENT_WIFISYS2 "WIFISYS2" // DIAG_EVENT_WIFISYS2>node type>IP>MAC>band>band's ifname>band's MAC>band's noise>band's MCS>band's capability>band's subif_count>base64encode(band's subif_ssid)>band's chanim>band's tx rate>band's rx rate>band's tx byte>band's rx byte
#else
#define DIAG_EVENT_WIFISYS2 "WIFISYS2" // DIAG_EVENT_WIFISYS2>node type>IP>MAC>band>band's ifname>band's MAC>band's noise>band's MCS>band's capability>band's subif_count>base64encode(band's subif_ssid)>band's tx rate>band's rx rate>band's tx byte>band's rx byte
#endif
#define DIAG_EVENT_STAINFO "STAINFO" // DIAG_EVENT_STAINFO>node type>IP>MAC>STA's MAC>STA's Band>STA's RSSI>active>STA's Tx>STA's Rx>STA's Tx byte>STA's Rx byte
#define DIAG_EVENT_STAINFO_STABLE "STAINFOSTABLE"
#define DIAG_EVENT_WLCE "WLCEVENT" // DIAG_EVENT_WLCE>node type>IP>MAC>STA's MAC>STA's Band>last_act>count_auth>count_deauth>count_assoc>count_disassoc>count_reassoc
#define DIAG_EVENT_TG_ROAMING "TG_ROAMING" // DIAG_EVENT_TG_ROAMING>node type>IP>MAC>STA's MAC>STA's Band>STA's RSSI>timestamp>user_low_rssi>rssi_cnt>idle_period>idle_start
#define DIAG_EVENT_ROAMING "ROAMING" // DIAG_EVENT_ROAMING>node type>IP>MAC>timestamp>STA's MAC>STA's RSSI>candidate_rssi_criteria>candidate's MAC>candidate's RSSI
#define DIAG_EVENT_NET "NET" // DIAG_EVENT_NET>node type>IP>MAC>wan_unit>link_wan>runIP>runMASK>runGATE>got_Route>got Redirect rules>DNS>Ping
#ifdef RTCONFIG_BCMBSD
#define DIAG_EVENT_TG_BSD "TG_BSD" // DIAG_EVENT_TG_BSD>node type>IP>MAC>STA's MAC>timestamp>from_chanspec>to_chanspec>reason
#endif
#define DIAG_EVENT_ETHINFO "ETHINFO" // DIAG_EVENT_ETHONFO>node type>type(BH or FH)>infame>IP>MAC>tx rate>rx rate>tx byte>rx byte
#define DIAG_EVENT_PORTINFO "PORTINFO"// DIAG_EVENT_PORTINFO>node type>IP>MAC>>label name>lan or wan>up or down>link rate>duplex>tx packets>rx packets>tx bytes>rx bytes>crc errors

extern int get_hw_acceleration(char *output, int size);
extern int get_sys_clk(char *output, int size);
extern int get_sys_temp(unsigned int *temp);

extern int get_wifi_chip(char *ifname, char *output, int size);
extern int get_wifi_temp(char *ifname, unsigned int *temp);
extern int get_wifi_country(char *ifname, char *output, int len);
extern int get_wifi_noise(char *ifname, char *output, int len);
extern int get_wifi_mcs(char *ifname, char *output, int len);
#ifdef RTCONFIG_BCMARM
extern int get_bss_info(char *ifname, int *capability); // TODO: non-Brcm needs to do.
extern int get_subif_count(char *target, int *count); // TODO: non-Brcm needs to do.
extern int get_subif_ssid(char *target, char *output, int outputlen); // TODO: non-Brcm needs to do.
extern int get_wifi_txop(char *ifname,int *txop, int outputlen);
extern int get_wifi_glitch(char *ifname,int *glitch, int outputlen);
extern int get_wifi_chanim(char *ifname, char *output, int outputlen);
extern int get_wifi_counters_info(char *ifname, char *info_name, int *value);
#endif
extern int get_wifi_dfs_status(char *output, int len, char *node, char *lan_ipaddr, char *lan_hwaddr);

extern char *diag_get_wifi_fh_ifnames(int wifi_unit, char *buffer, size_t buffer_size);
extern char *diag_get_eth_bh_ifnames(char *buffer, size_t buffer_size);
extern char *diag_get_eth_fh_ifnames(char *buffer, size_t buffer_size);
extern int diag_is_wlif(char *ifname);
extern int diag_is_uplink_ethif(char *ifname);
extern int get_plc_phy_rate(unsigned long *tx_rate, unsigned long *rx_rate);

// non-sysdep
extern char* diag_get_wl_ifname(int unit, int subunit, char *buffer, size_t buffer_size);
extern int diag_get_sub_if_bss_enabled(int unit, int subunit);
extern int diag_get_sub_if_closed(int unit, int subunit);

extern int special_alphasort(const void *d1, const void *d2);
extern void create_cd_sql_thread(void);
extern int mode_str_to_int(char *mode);
extern int mix_data_handler(char *raw);

#ifdef RTCONFIG_CD_IPERF
extern int diag_iperf_test_detect();
extern int run_iperf(char* caller, char *server_mac, char *client_mac);
extern int update_iperf_test_status(char *server_mac, char *client_mac, int update_status);
extern int update_iperf_test_allstatus(int update_allstatus);
extern int check_if_iperf_server_ok();
extern int iperf3_parse_client(int *bandwidth_list,int *retry_list);
extern void iperf_client(void *in);
extern void iperf_server(void);
extern void create_iperf_client(char* in);
extern void create_iperf_server(void);
extern void proc_iperf(char* caller, int is_server,char *smac,char *cmac, char *target_ip,int port);
extern int send_iperf_client_cmd_to_re(char *smac,char *cmac,char *serverip,int port);
extern int check_iperf_server_resp(char *in);
#endif

#define DIAG_DEBUG "/tmp/DIAG_DEBUG"

#define LOG_TITLE_DIAG "CONNDIAG"

#define DIAG_INFO(fmt, arg...) \
	do { \
		_dprintf("DIAG %lu: "fmt, uptime(), ##arg); \
		if(diag_syslog || f_exists(DIAG_DEBUG)) \
			logmessage(LOG_TITLE_DIAG, fmt, ##arg); \
	} while (0)
#define DIAG_DBG(fmt, arg...) \
	do { \
		if(diag_dbg || f_exists(DIAG_DEBUG)) \
			_dprintf("DIAG %lu: "fmt, uptime(), ##arg); \
		if(diag_syslog || f_exists(DIAG_DEBUG)) \
			logmessage(LOG_TITLE_DIAG, fmt, ##arg); \
	} while (0)
#define DIAG_SYSLOG(fmt, arg...) \
	do { \
		_dprintf("DIAG %lu: "fmt"\n", uptime(), ##arg); \
		logmessage(LOG_TITLE_DIAG, fmt, ##arg); \
	} while (0)

#define IPERF_PORT_BASE 30000
#define IPERF_PORT_RANGE 1000
#define IPERF_TEST_10_SEC 10
#define IPERF_DEFAULT_WAITING_TIME IPERF_TEST_10_SEC+10
#define IPERF_CLIENT_RET_PATH "/tmp/iperf_tmp_1"
#define IPERF_SERVER_RET_PATH "/tmp/iperf_tmp_2"
#define IPERF_CMD_PATH "/tmp/iperf_sh"
#define CD_SQNUM_BASE 0
#define CD_SQNUM_RANGE 9999
#define CD_SQNUM_MIN CD_SQNUM_BASE+1
#define CD_SQNUM_MAX CD_SQNUM_BASE+CD_SQNUM_RANGE

#define CD_CBP_EVENT_PATH "/jffs/cbp_event_file"
#define CD_CBP_EVENT_LOCK	"cd_cbp_event_lock"
