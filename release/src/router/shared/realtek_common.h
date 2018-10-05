#ifndef __REALTEK_COMMON_H__
#define __REALTEK_COMMON_H__



#define MAC_ADDR_LEN    6
#define MAX_STA_NUM			64	// max support sta number

/* wlan driver ioctl id */
#define SIOCGIWRTLSTAINFO   		0x8B30	// get station table information
#define SIOCGIWRTLSTANUM		0x8B31	// get the number of stations in table
#define SIOCGIWRTLSCANREQ		0x8B33	// scan request
#define SIOCGIWRTLGETBSSDB		0x8B34	// get bss data base
#define SIOCGIWRTLJOINREQ		0x8B35	// join request
#define SIOCGIWRTLJOINREQSTATUS		0x8B36	// get status of join request
#define SIOCGIWRTLGETBSSINFO		0x8B37	// get currnet bss info
#define SIOCGIWRTLGETWDSINFO		0x8B38
#define SIOCGMISCDATA	0x8B48	// get misc data
#define SIOCGIWRTLACLINFO   0x8BF7
#define SIOCGIWRTLACLCLIENTLIST   0x8BF8
#define SIOCGIWRTLMONITORSTARSSI 0x8BFA

/* flag of sta info */
#define STA_INFO_FLAG_AUTH_OPEN     	0x01
#define STA_INFO_FLAG_AUTH_WEP      	0x02
#define STA_INFO_FLAG_ASOC          	0x04
#define STA_INFO_FLAG_ASLEEP        	0x08

#define	MAX_BSS_DESC	64
#define SSID_LEN	32
#define MESHID_LEN 32
#define DEF_COUNTRY_CODE "US"

#ifndef uint32_t
typedef unsigned int uint32_t;
#endif
typedef enum { BAND_11B=1, BAND_11G=2, BAND_11BG=3, BAND_11A=4, BAND_11N=8, BAND_5G_11AN=12, 
	BAND_5G_11AC=64,BAND_5G_11AAC=68,BAND_5G_11NAC=72,BAND_5G_11ANAC=76} BAND_TYPE_T;

typedef enum _wlan_mac_state {
    STATE_DISABLED=0, STATE_IDLE, STATE_SCANNING, STATE_STARTED, STATE_CONNECTED, STATE_WAITFORKEY
} wlan_mac_state;

typedef struct _bss_info {
    unsigned char state;
    unsigned char channel;
    unsigned char txRate;
    unsigned char bssid[6];
    unsigned char rssi, sq;	// RSSI  and signal strength
    unsigned char ssid[SSID_LEN+1];
} bss_info;

typedef struct _OCTET_STRING {
    unsigned char *Octet;
    unsigned short Length;
} OCTET_STRING;
typedef enum _BssType {
    infrastructure = 1,
    independent = 2,
} BssType;
typedef	struct _IbssParms {
    unsigned short	atimWin;
} IbssParms;

typedef struct _BssDscr {
    unsigned char bdBssId[6];
    unsigned char bdSsIdBuf[SSID_LEN];
    OCTET_STRING  bdSsId;

 
	//by GANTOE for site survey 2008/12/26
	unsigned char bdMeshIdBuf[MESHID_LEN]; 
	OCTET_STRING bdMeshId; 

    BssType bdType;
    unsigned short bdBcnPer;			// beacon period in Time Units
    unsigned char bdDtimPer;			// DTIM period in beacon periods
    unsigned long bdTstamp[2];			// 8 Octets from ProbeRsp/Beacon
    IbssParms bdIbssParms;			// empty if infrastructure BSS
    unsigned short bdCap;				// capability information
    unsigned char ChannelNumber;			// channel number
    unsigned long bdBrates;
    unsigned long bdSupportRates;		
    unsigned char bdsa[6];			// SA address
    unsigned char rssi, sq;			// RSSI and signal strength
    unsigned char network;			// 1: 11B, 2: 11G, 4:11G
	// P2P_SUPPORT
	unsigned char	p2pdevname[33];		
	unsigned char	p2prole;	
	unsigned short	p2pwscconfig;		
	unsigned char	p2paddress[6];	
   unsigned char        stage;
}BssDscr, *pBssDscr;


typedef struct _sitesurvey_status {
    unsigned char number;
    unsigned char pad[3];
    BssDscr bssdb[MAX_BSS_DESC];
} SS_STATUS_T, *SS_STATUS_Tp;


/* WLAN sta info structure */
typedef struct wlan_sta_info {
	unsigned short	aid;
	unsigned char	addr[6];
	unsigned long	tx_packets;
	unsigned long	rx_packets;
	unsigned long	expired_time;	// 10 msec unit
	unsigned short	flag;
	unsigned char	txOperaRates;
	unsigned char	rssi;
	unsigned long	link_time;		// 1 sec unit
	unsigned long	tx_fail;
	unsigned long tx_bytes;
	unsigned long rx_bytes;
	unsigned char network;
	unsigned char ht_info;	// bit0: 0=20M mode, 1=40M mode; bit1: 0=longGI, 1=shortGI
	unsigned char	RxOperaRate;
	unsigned char 	resv[5];
} WLAN_STA_INFO_T, *WLAN_STA_INFO_Tp;

/* WLAN acl info structure */
typedef struct wlan_acl_info
{
    int mode;
} WLAN_ACL_INFO_T, *WLAN_ACL_INFO_Tp;

/* WLAN acl client list structure */
typedef struct wlan_acl_client_list
{
    int count;
    unsigned char macAddr[128][MAC_ADDR_LEN];
} WLAN_ACL_CLIENT_LIST_T, *WLAN_ACL_CLIENT_LIST_Tp;

#endif
