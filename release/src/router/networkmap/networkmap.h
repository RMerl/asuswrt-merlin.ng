#ifndef __NETWORKMAP_H__
#define __NETWORKMAP_H__
/*
#include <sys/socket.h>
#include <stdio.h>
#include <linux/in.h>
#include <linux/if_ether.h>
#include <net/if.h>
 */
#include <syslog.h>
#include <version.h>
#include <shared.h>
#include <sm.h>
#include <json.h>

#ifndef FALSE
#define FALSE	0
#endif
#ifndef TRUE
#define TRUE	1
#endif
#define INTERFACE	"br0"
#define MODEL_NAME	RT_BUILD_NAME
#define ARP_BUFFER_SIZE	512

// Hardware type field in ARP message
#define DIX_ETHERNET		1
// Type number field in Ethernet frame
#define IP_PACKET		0x0800
#define ARP_PACKET		0x0806
#define RARP_PACKET		0x8035
// Message type field in ARP messages
#define ARP_REQUEST		1
#define ARP_RESPONSE		2
#define RARP_REQUEST		3
#define RARP_RESPONSE		4
#define RCV_TIMEOUT		2 //sec
#define MAXDATASIZE		512
#define LPR			0x02
#define LPR_RESPONSE		0x00

#define LINE_SIZE		200

//for subnet scan(vlan & free-wifi & captive portal)
#define SHMKEY_LAN	1001
#ifdef RTCONFIG_BONJOUR
#define SHMKEY_BONJOUR	1002
#endif
#if defined(RTCONFIG_TAGGED_BASED_VLAN) && !defined(FLAG_FUNCTION_TEMPORARILY_CANCELED)
#define SHMKEY_VLAN1	1003
#define SHMKEY_VLAN2	1004
#define SHMKEY_VLAN3	1005
#define SHMKEY_VLAN4	1006
#define SHMKEY_VLAN5	1007
#define SHMKEY_VLAN6	1008
#define SHMKEY_VLAN7	1009
#define SHMKEY_VLAN8	1010
#endif
#if defined(RTCONFIG_CAPTIVE_PORTAL) && !defined(FLAG_FUNCTION_TEMPORARILY_CANCELED)
#define SHMKEY_FREEWIFI	1011
#define SHMKEY_CP	1012
#endif
#ifdef RTCONFIG_AMAS_WGN
#define AMAS_WGN_BR_1	"br2"
#define AMAS_WGN_BR_2	"br3"
#define AMAS_WGN_BR_3	"br4"
#define AMAS_WGN_BR_4	"br5"
#endif

//for Notification Center trigger flag
#ifdef RTCONFIG_NOTIFICATION_CENTER
enum
{
	FLAG_SAMBA_INLAN = 0,
	FLAG_OSX_INLAN,
	FLAG_XBOX_PS,
	FLAG_UPNP_RENDERER
};
#endif

#define USERAGENT           "Asuswrt/networkmap"
#define NMPDB_FILE_LOCK     "nmpdb"
#define USERAGENT           "Asuswrt/networkmap"
#define NMP_VC_FILE_LOCK    "nmpvc"

#define CFG_FILE_LOCK                "cfg_mnt"
#define ALLWEVENT_FILE_LOCK          "allwevent"
#define ALLWCLIENT_LIST_JSON_PATH    "/tmp/allwclientlist.json"
#define CLIENTLIST_FILE_LOCK         "clientlist"
#define CLIENT_LIST_JSON_PATH        "/tmp/clientlist.json"
#define WIREDCLIENTLIST_FILE_LOCK    "wiredclientlist"
#define WIRED_CLIENT_LIST_JSON_PATH  "/tmp/wiredclientlist.json"
#define BRCTL_TABLE_PATH             "/tmp/nmp_brctl_table"
#define ASUS_DEVICE_JSON_FILE        "/tmp/asus_device.json"

#ifdef RTCONFIG_IPV6
#define IP6_TABLE_PATH               "/tmp/nmp_ip6_table"
#endif

#define NMP_CLIENT_LIST             "/tmp/nmp_client_list"

#define NCL_LIMIT		262144   //database limit to 256KB to avoid UI glitch

#define IP_TABLE_PATH               "/tmp/nmp_ip_table"
#define NMP_DEBUG_FILE				"/tmp/NMP_DEBUG"
#define NMP_DEBUG_MORE_FILE			"/tmp/NMP_DEBUG_MORE"
#define NMP_DEBUG_FUNCTION_FILE		"/tmp/NMP_DEBUG_FUNCTION"
#define NMP_DEBUG_VC_FILE			"/tmp/NMP_DEBUG_VC"

#define NEWORKMAP_OUI_FILE		"/usr/networkmap/networkmap.oui.js"

#if (defined(RTCONFIG_JFFS2) || defined(RTCONFIG_JFFSV1) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS) || defined(RTCONFIG_JFFS_PARTITION))
#define NMP_CL_JSON_FILE		"/jffs/nmp_cl_json.js"
#define NMP_VC_JSON_FILE		"/jffs/nmp_vc_json.js"
#else
#define NMP_CL_JSON_FILE		"/tmp/nmp_cl_json.js"
#define NMP_VC_JSON_FILE		"/tmp/nmp_vc_json.js"
#endif

#define DEV_TYPE_PATH		"/jffs/dev_type_query.json"
#define ARP_PATH			"/proc/net/arp"

#ifdef RTCONFIG_MULTILAN_CFG
#define APG_IFNAMES_USED_FILE			"/jffs/.sys/cfg_mnt/apg_ifnames_used.json"
#endif

#define NMP_CONSOLE_DEBUG(fmt, args...) do{ \
	if(nvram_match("nmp_debug", "1")) { \
		cprintf(fmt, ## args); \
	} \
}while(0)

#if !defined(RTCONFIG_RALINK) && !defined(HND_ROUTER)
#define NMP_DEBUG(fmt, args...) \
	if(f_exists(NMP_DEBUG_FILE)) { \
		_dprintf("%s(%d) > "fmt, __func__, __LINE__, ## args); \
	}
#else
#define NMP_DEBUG(fmt, args...) \
	if(f_exists(NMP_DEBUG_FILE)) { \
		printf("%s(%d) > "fmt, __func__, __LINE__, ## args); \
	}
#endif

#if !defined(RTCONFIG_RALINK) && !defined(HND_ROUTER)
#define NMP_DEBUG_M(fmt, args...) \
	if(f_exists(NMP_DEBUG_MORE_FILE)) { \
		_dprintf(fmt, ## args); \
	}
#else
#define NMP_DEBUG_M(fmt, args...) \
	if(f_exists(NMP_DEBUG_MORE_FILE)) { \
		printf(fmt, ## args); \
	}
#endif

#if !defined(RTCONFIG_RALINK) && !defined(HND_ROUTER)
#define NMP_DEBUG_F(fmt, args...) \
	if(f_exists(NMP_DEBUG_FUNCTION_FILE)) { \
		_dprintf(fmt, ## args); \
	}
#else
#define NMP_DEBUG_F(fmt, args...) \
	if(f_exists(NMP_DEBUG_FUNCTION_FILE)) { \
		printf(fmt, ## args); \
	}
#endif

#if !defined(RTCONFIG_RALINK) && !defined(HND_ROUTER)
#define NMP_DEBUG_VC(fmt, args...) \
	if(f_exists(NMP_DEBUG_VC_FILE)) { \
		_dprintf(fmt, ## args); \
	}
#else
#define NMP_DEBUG_VC(fmt, args...) \
	if(f_exists(NMP_DEBUG_VC_FILE)) { \
		printf(fmt, ## args); \
	}
#endif

typedef unsigned char UCHAR;
typedef unsigned short USHORT;
typedef unsigned long ULONG;

#if defined(RTCONFIG_SOC_IPQ8064) || defined(RTCONFIG_SOC_IPQ8074)
#define MAX_NR_CLIENT_LIST	1024	/* occupies 436252 bytes. */
#else
#if defined(RTCONFIG_BUSINESS)
#define MAX_NR_CLIENT_LIST	1021
#else
#define MAX_NR_CLIENT_LIST	255	/* occupies 108656 bytes. */
#endif
#endif

enum
{
	FLAG_HTTP = 0,
	FLAG_PRINTER,
	FLAG_ITUNE,
	FLAG_EXIST,
	FLAG_VENDOR,
	FLAG_ASUS
};

#define TYPE_LINUX_DEVICE	22
#define TYPE_WINDOWS		30
#define TYPE_ANDROID		31

#define SUCCESS		0

enum
{
	BASE_TYPE_DEFAULT = 0,
	BASE_TYPE_ANDROID,
	BASE_TYPE_WINDOW,
	BASE_TYPE_LINUX,
	BASE_TYPE_ASUS,
	BASE_TYPE_APPLE
};

#ifdef RTCONFIG_MULTILAN_CFG
#define IS_STAINFO          1
#define IS_ALLWCLIENTLIST   0
#endif

//Device service info data structure
typedef struct {
	unsigned char	ip_addr[MAX_NR_CLIENT_LIST][4];
	unsigned char	mac_addr[MAX_NR_CLIENT_LIST][6];
	unsigned char	user_define[MAX_NR_CLIENT_LIST][16];
	unsigned char	vendor_name[MAX_NR_CLIENT_LIST][128];
	unsigned char	device_name[MAX_NR_CLIENT_LIST][32];
	unsigned char	apple_model[MAX_NR_CLIENT_LIST][16];
	unsigned char	device_type[MAX_NR_CLIENT_LIST][32];
	unsigned char	vendorClass[MAX_NR_CLIENT_LIST][32];
	unsigned char	os_type[MAX_NR_CLIENT_LIST];
#ifdef RTCONFIG_IPV6
	char			ip6_addr[MAX_NR_CLIENT_LIST][40];
	char			ip6_prefix[MAX_NR_CLIENT_LIST][50];
#endif
#ifdef RTCONFIG_MULTILAN_CFG
	unsigned char	sdn_idx[MAX_NR_CLIENT_LIST];
	unsigned char	sdn_type[MAX_NR_CLIENT_LIST][32];
	unsigned char	vlan_id[MAX_NR_CLIENT_LIST];
#endif
	unsigned char	online[MAX_NR_CLIENT_LIST];
	unsigned char	type[MAX_NR_CLIENT_LIST];
	unsigned char	ipMethod[MAX_NR_CLIENT_LIST][7];
	unsigned char	opMode[MAX_NR_CLIENT_LIST];
	unsigned char	dhcp_flag[MAX_NR_CLIENT_LIST];
/* define bitmap flag:	0.http
			1.printer
			2.itune
			3.exist
			4.vendor
			5.asus
*/			
	unsigned char	device_flag[MAX_NR_CLIENT_LIST];
/* wireless: 0:wired 1:2.4G 2:5G 3:5G-2
*/
	unsigned char	wireless[MAX_NR_CLIENT_LIST];
#ifdef RTCONFIG_MLO
	unsigned char	mlo[MAX_NR_CLIENT_LIST];
	char			mlo_2G_mac[MAX_NR_CLIENT_LIST][18];
	char			mlo_5G_mac[MAX_NR_CLIENT_LIST][18];
	char			mlo_5G1_mac[MAX_NR_CLIENT_LIST][18];
	char			mlo_6G_mac[MAX_NR_CLIENT_LIST][18];
	char			mlo_6G1_mac[MAX_NR_CLIENT_LIST][18];
#endif
	unsigned char	is_wireless[MAX_NR_CLIENT_LIST];
	int        		conn_ts[MAX_NR_CLIENT_LIST];		// connect  timestamp
	int        		offline_time[MAX_NR_CLIENT_LIST];
/* wireless log information
*/
#ifdef RTCONFIG_LANTIQ
	time_t		tstamp[MAX_NR_CLIENT_LIST];
#endif
	char		pap_mac[MAX_NR_CLIENT_LIST][18];
	char		guest_network[MAX_NR_CLIENT_LIST][4];
	char		ssid[MAX_NR_CLIENT_LIST][32];
	char 		txrate[MAX_NR_CLIENT_LIST][7];
	char 		rxrate[MAX_NR_CLIENT_LIST][10];
	char 		mac_src[MAX_NR_CLIENT_LIST][30];
	char 		name_src[MAX_NR_CLIENT_LIST][30];
	char 		vendor_src[MAX_NR_CLIENT_LIST][30];
	char 		type_src[MAX_NR_CLIENT_LIST][30];
	char 		online_src[MAX_NR_CLIENT_LIST][30];
	char 		wireless_src[MAX_NR_CLIENT_LIST][30];
	unsigned int 	rssi[MAX_NR_CLIENT_LIST];
	char 		conn_time[MAX_NR_CLIENT_LIST][12];
	char 		wireless_auth[MAX_NR_CLIENT_LIST][32];
#if defined(RTCONFIG_FBWIFI) || defined(RTCONFIG_CAPTIVE_PORTAL)
	char		subunit[MAX_NR_CLIENT_LIST];
#endif
#if (defined(RTCONFIG_BWDPI) || defined(RTCONFIG_BWDPI_DEP))
	char		bwdpi_host[MAX_NR_CLIENT_LIST][32];
	char		bwdpi_vendor[MAX_NR_CLIENT_LIST][100];
	char		bwdpi_type[MAX_NR_CLIENT_LIST][100];
	char		bwdpi_device[MAX_NR_CLIENT_LIST][100];
#endif
	int		ip_mac_num;
	int		detail_info_num;
//for sorting asus device
	int		asus_device_num;
	int		commit_no;
	char		delete_mac[13];

} CLIENT_DETAIL_INFO_TABLE, *P_CLIENT_DETAIL_INFO_TABLE;

// walf test
typedef struct
{
	unsigned short	hardware_type; 
	unsigned short	protocol_type;		 
	unsigned char	hwaddr_len;
	unsigned char	ipaddr_len;		
	unsigned short	message_type;
	unsigned char	source_hwaddr[6];		     
	unsigned char	source_ipaddr[4];
	unsigned char	dest_hwaddr[6];	 
	unsigned char	dest_ipaddr[4];
} ARP_HEADER;

int FindHostname(P_CLIENT_DETAIL_INFO_TABLE p_client_detail_info_tab, int i);
int FindDevice(unsigned char *pIP, unsigned char *pMac, int replaceMac);
void find_wireless_device(P_CLIENT_DETAIL_INFO_TABLE p_client_detail_info_tab, int offline);
void rc_diag_stainfo(P_CLIENT_DETAIL_INFO_TABLE p_client_detail_info_tab, int i, char *mlo_mac);

void type_filter(P_CLIENT_DETAIL_INFO_TABLE p_client_detail_info_tab, int x, unsigned char type, unsigned char base, int isDev, const char *type_src);
int isBaseType(int type);

int QueryConvTypes(P_CLIENT_DETAIL_INFO_TABLE p_client_detail_info_tab, int i);

#ifdef RTCONFIG_DNSQUERY_INTERCEPT
void QueryDevType(P_CLIENT_DETAIL_INFO_TABLE p_client_detail_info_tab, int i);
#endif

#ifdef RTCONFIG_MULTILAN_CFG
void check_manual_dhcp(P_CLIENT_DETAIL_INFO_TABLE p_client_detail_info_tab, int i, const int subnet_idx);
void get_subnet_ifname(const int subnet_idx, char * subnet_ifname, int ifname_len);
void get_ip_from_arp_table(P_CLIENT_DETAIL_INFO_TABLE p_client_detail_info_tab, const int i, const char *subnet);
int get_sdn_type(const int sdn_idx, char *sdn_type, int sdn_type_len, unsigned char *vlan_id, int *apg_idx);
int get_vlan_id(const int vlan_idx);
int get_sdn_idx_form_apg(const char *papMac, const char *ifname, const int ifname_type);
#endif

int get_brctl_macs(char * mac);

int check_allwclientlist_json(const char *client_mac, const int opMode);

int check_wrieless_info(P_CLIENT_DETAIL_INFO_TABLE p_client_detail_info_tab, const int i, const int is_file, struct json_object *clients);

void regularly_check_devices(P_CLIENT_DETAIL_INFO_TABLE p_client_detail_info_tab);

void check_clientlist_offline(CLIENT_DETAIL_INFO_TABLE *p_client_detail_info_tab);
#ifdef RTCONFIG_MLO
int check_mlo_info(CLIENT_DETAIL_INFO_TABLE *p_client_detail_info_tab, const int i, const int wireless_type, char *guest_network, char *client_mac, char *papMac, struct json_object *macObj, char *mlo_mac, int *is_mlo);
#endif
int check_wire_info(P_CLIENT_DETAIL_INFO_TABLE p_client_detail_info_tab, const int i);

int check_wireless_clientlist(CLIENT_DETAIL_INFO_TABLE *p_client_detail_info_tab);

void check_clients_from_ip_cmd(CLIENT_DETAIL_INFO_TABLE *p_client_detail_info_tab);

void check_dhcp_ip_online(CLIENT_DETAIL_INFO_TABLE *p_client_detail_info_tab, const char *mac, const char *ip_addr);

int get_client_list();

void check_brctl_mac_online(CLIENT_DETAIL_INFO_TABLE *p_client_detail_info_tab);

int check_asus_device(CLIENT_DETAIL_INFO_TABLE *p_client_detail_info_tab, const int i);

void network_ip_scan();

int json_checker(const char *json_str);

int check_arp_table(const char *ipaddr);

int check_brctl_macs(CLIENT_DETAIL_INFO_TABLE *p_client_detail_info_tab);

#ifdef RTCONFIG_IPV6
void check_ip6_addr(CLIENT_DETAIL_INFO_TABLE *p_client_detail_info_tab);

int check_ip6_mac(CLIENT_DETAIL_INFO_TABLE *p_client_detail_info_tab, const char *mac, const char *ip6_addr);

int check_clientlist_json(char *mac);
#endif

int get_clientlist_rssi(const char *mac);

void add_client_info(CLIENT_DETAIL_INFO_TABLE *p_client_detail_info_tab, const char *mac, const char *ip_addr, const int ipv6);

void arp_compare_clientlist(CLIENT_DETAIL_INFO_TABLE *p_client_detail_info_tab);

#endif  /*__NETWORKMAP_H__*/
