#if defined(RTCONFIG_QCA)
#include <net/ethernet.h>
#endif
//amas_bhctrl connection status
#define ETH 		0x01  	//ethernet
#define WL_2G 		0x02  	//2G
#define WL_5G 		0x04	//5G
#define WL_5G_1 	0x08	//5G-1
#define ETH_2		0x10	//ethernet_2
#define ETH_3		0x20	//ethernet_3
#define ETH_4		0x40	//ethernet_4
#define WL_6G		0x80	//6G


#define CH_SYNC_INIT_STATE	0
#define CH_SYNC_CONNECTING	1
#define CH_SYNC_CONNECTED	2
#define CH_SYNC_NO_USE		-1
#define CH_SYNC_NO_COONECT	-2
#define CH_SYNC_ETH_BHL		-3
#define CH_SYNC_WIFI_BHL	-4

/*
Ethernet selection rule:
ETHERNET_PLUGIN: ethernet connected to P-AP and CAP.
ETHERNET_HOP : ethernet connected to P-AP and CAP, ethernet hop count smaller than wireless.
We add nvram [amas_ethernet] for config ethernet selection rule.
*/
enum {
		NONE=0,
		ETHERNET_NONE,
		ETHERNET_PLUGIN,
		ETHERNET_HOP
};

/**
 * @brief Connection priority
 *
 */
enum {
    /* AiMesh 1.0 */
    CONN_PRI_NONE = 0,
    CONN_PRI_WIFI_ONLY = 1,
    CONN_PRI_ETH = 2,
    CONN_PRI_AUTO = 3,
    /* Ethernet 4X, X: index, 4: Connection Priority */
    CONN_PRI_ETH1G = 4,
    CONN_PRI_ETH25G = 5,
    CONN_PRI_ETH5G = 6,
    CONN_PRI_ETH10G = 7,
    CONN_PRI_ETH10GPLUS = 8,	/* 10G SFP+ */
    /* Wireless 100X, X: index, 100: Connection Priority */
    CONN_PRI_WIFI_2G = 100,
    CONN_PRI_WIFI_5G = 200,
    CONN_PRI_WIFI_5G2 = 300,
    CONN_PRI_WIFI_6G = 400,
    /* PLC 10000X, X: index, 10000: Connection Priority */
    CONN_PRI_PLC = 10000,
    CONN_PRI_CUSTOM = 0xFFFFFFFF
};

/**
 * @brief Ethernet type
 *
 */
enum {
    ETH_TYPE_NONE    = 0,
    ETH_TYPE_10      = 1,
    ETH_TYPE_100     = 2,
    ETH_TYPE_1000    = 4,
    ETH_TYPE_25G     = 8,
    ETH_TYPE_5G      = 16,
    ETH_TYPE_10G     = 32,
    ETH_TYPE_10GPLUS = 64,	/* 10G SFP+ */
    ETH_TYPE_PLC     = 65536
};

/**
 * @brief bridge contorl acton value
 *
 */
enum { ARG_addbr = 0, ARG_delbr, ARG_addif, ARG_delif};

/**
 * @brief AiMesh backhaul select algorithm
 *
 */
enum { AIMESH_ALG_DEFAULT = 0, AIMESH_ALG_COST = 1, AIMESH_ALG_RSSISCORE = 2};

#define OB_OFF			1
#define OB_AVALIABLE	2
#define OB_REQ			3
#define OB_LOCKED		4
#define OB_SUCCESS		5

#define SS_OFF              0
#define SS_KEY				1
#define SS_KEYACK			2
#define SS_SECURITY			3
#define SS_SUCCESS			4
#define SS_FAIL         	5
#define SS_KEYFAIL			6
#define SS_SECURITYFAIL		7
#define SS_TIMEOUT          8
#define SS_KEY_FIN			9
#define	SS_KEYACK_FIN		10
#define SS_SECURITY_FIN		11
#define SS_SUCCESS_FIN		12
#define SS_OBD_FIN          13


#define HASH_LEN			32
#define GEN_KEY_LEN			32

#define MAX_ETH				15
#define MAX_WIFI			15

/**
 * @brief for amas_path_stat_v2
 *
 */
#define ETH1_U_V2				0x001000
#define ETH2_U_V2				0x002000
#define ETH3_U_V2				0x004000
#define ETH4_U_V2				0x008000
#define WL2G_U_V2				0x100000
#define WL5G1_U_V2				0x200000
#define WL5G2_U_V2				0x400000

/**
 * For defined ifnames index.
 *
 */
#define ETH_U_BASE			0x1
#define ETH1_U				ETH_U_BASE
#define ETH2_U				(ETH_U_BASE << 1)
#define ETH3_U				(ETH_U_BASE << 2)
#define ETH4_U				(ETH_U_BASE << 3)
#define ETH5_U				(ETH_U_BASE << 4)
#define ETH6_U				(ETH_U_BASE << 5)
#define ETH7_U				(ETH_U_BASE << 6)
#define ETH8_U				(ETH_U_BASE << 7)
#define ETH9_U				(ETH_U_BASE << 8)
#define ETH10_U				(ETH_U_BASE << 9)
#define ETH11_U				(ETH_U_BASE << 10)
#define ETH12_U				(ETH_U_BASE << 11)
#define ETH13_U				(ETH_U_BASE << 12)
#define ETH14_U				(ETH_U_BASE << 13)
#define ETH15_U				(ETH_U_BASE << 14)
#define ETH16_U				(ETH_U_BASE << 15)
#define ETH_MAX_BASE		ETH16_U
#define WL_U_BASE			0x10000
#define WL2G_U				WL_U_BASE
#define WL5G1_U				(WL_U_BASE << 1)
#define WL5G2_U				(WL_U_BASE << 2)
#define WL6G_U				(WL_U_BASE << 3)
#define WL_MAX_BASE			(WL_U_BASE << 14) // Reserved 32th bits for singned value.

/*COST Verify
DONT_COST (don't check cost)		0	0	1	0x01
ETH_COST  (cost for eth only)		0	1	0	0x02
WIFI_COST (cost for wifi only)		0	1	1	0x03
ALL_COST  (cost for wifi & eth)		1	0	0	0x04
AUTO_COST							1	0	1	0x05

AUTO_COST:
	If a part of the interface gets the cost, choose the interface that has the cost.
	If it fail to get the cost for all interface, we will not check the cost for any path.
	If it can get the cost for all interfaces, we will check the cost for all path.
*/
#define DONT_COST				0x01
#define ETH_COST				0x02
#define WIFI_COST				0x03
#define ALL_COST				0x04
#define AUTO_COST				0x05


/*RSSI SCORE Verify
DONT_RSSISCORE (don't check rssi_score)		1	0	0	0x01
AUTO_RSSISCORE								0	1	0	0x02

AUTO_RSSISCORE:
	If a part of the interface gets the cost, we will not check the cost for any path.
	If it fail to get the cost for all interface, we will not check the cost for any path.
	If it can get the cost for all interfaces, we will check the cost for all path.

	Wi-Fi backhaul's choice based on priority.
	Wi-Fi backhaul and Ethernet backhaul selection based on RSSIscore or cost.
*/

#define DONT_RSSISCORE			0x01
#define ETH_RSSISCORE			0x02
#define WIFI_RSSISCORE			0x03
#define ALL_RSSISCORE			0x04
#define AUTO_RSSISCORE			0x05

/*amas_bhctrl sent command for and amas_wlcconnect by IPC*/
#define ACTION_START					"START_CONNECTING"
#define ACTION_RESTART					"RESTART_CONNECTING"
#define ACTION_STOP						"MAINTAIN_STATUS_QUO"
#define ACTION_START_OPTIMIZATION		"START_SELF_OPTIMIZATION"
#define ACTION_STOP_OPTIMIZATION		"STOP_SELF_OPTIMIZATION"
#define ACTION_DISCONNECT				"DISCONNECT_BAND"
#define ACTION_START_BY_DRIVER			"START_CONNECTING_BY_DRIVER"
#define ACTION_START_FOLLOW_CONNECTION	"START_FOLLOW_CONNECTION"
#ifdef RTCONFIG_AMAS_CENTRAL_OPTMZ
#define ACTION_START_OPTIMIZATION_SITE_SURVEY	"START_OPTIMIZATION_SITE_SURVEY"
#define ACTION_START_OPTIMIZATION_CONNECT		"START_OPTIMIZATION_CONNECT"
#endif


#define START_CONNECTING				0x01
#define RESTART_CONNECTING				0x02
#define MAINTAIN_STATUS_QUO				0x03
#define START_SELF_OPTIMIZATION			0x04
#define STOP_SELF_OPTIMIZATION			0x05
#define DISCONNECT_BAND 				0x06
#define START_CONNECTING_BY_DRIVER		0x07
#ifdef RTCONFIG_AMAS_CENTRAL_OPTMZ
#define START_OPTIMIZATION_SITE_SURVEY	0x08
#define START_OPTIMIZATION_CONNECT		0x09
#endif

/*amas_wlc_action_state for amas_wlcconnect*/
#define IDLE						0x00
#define BUSY						0x01
#define FIN							0x02

/*Who triggered the behavior of self-optimize*/
#define OPTMZ_FROM_RE				0x01   	// trigger by amas_bhctrl
#define OPTMZ_FROM_CAP				0x02	// trigger by cfg_mnt


#define SEND_ACTION_RESET			0x00
#define SEND_ACTION_SUCCESS			0x01
#define SEND_ACTION_FAIL			0x02

/* Uplink Port capbility */

typedef struct uplinkport_capval_s {
    char name[32];
    int val;
} uplinkport_capval_s;

/* Uplink Port capbility End */

#ifdef RTCONFIG_AMAS_CENTRAL_OPTMZ
enum optFollow {
	OPT_FOLLOW_NONE = 0,
	OPT_FOLLOW_OLD,
	OPT_FOLLOW_NEW,
	OPT_FOLLOW_MAX
};
#endif