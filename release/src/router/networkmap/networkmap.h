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

#define FALSE	0
#define TRUE	1
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
#ifdef RTCONFIG_TAGGED_BASED_VLAN
#define SHMKEY_VLAN1	1003
#define SHMKEY_VLAN2	1004
#define SHMKEY_VLAN3	1005
#define SHMKEY_VLAN4	1006
#define SHMKEY_VLAN5	1007
#define SHMKEY_VLAN6	1008
#define SHMKEY_VLAN7	1009
#define SHMKEY_VLAN8	1010
#endif
#ifdef RTCONFIG_CAPTIVE_PORTAL
#define SHMKEY_FREEWIFI	1011
#define SHMKEY_CP	1012
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


#define USERAGENT "Asuswrt/networkmap"


#define NCL_LIMIT		14336   //database limit to 14KB to avoid UI glitch

#define NMP_DEBUG_FILE			"/tmp/NMP_DEBUG"
#define NMP_DEBUG_MORE_FILE		"/tmp/NMP_DEBUG_MORE"
#define NMP_DEBUG_FUNCTION_FILE		"/tmp/NMP_DEBUG_FUNCTION"

#define NEWORKMAP_OUI_FILE		"/usr/networkmap/networkmap.oui.js"
#if (defined(RTCONFIG_JFFS2) || defined(RTCONFIG_JFFSV1) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS))
#define NMP_CL_JSON_FILE		"/jffs/nmp_cl_json.js"
#else
#define NMP_CL_JSON_FILE		"/tmp/nmp_cl_json.js"
#endif
#define ARP_PATH			"/proc/net/arp"

#define NMP_CONSOLE_DEBUG(fmt, args...) do{ \
	if(nvram_match("nmp_debug", "1")) { \
		cprintf(fmt, ## args); \
	} \
}while(0)

#if !defined(RTCONFIG_RALINK) && !defined(HND_ROUTER)
#define NMP_DEBUG(fmt, args...) \
	if(f_exists(NMP_DEBUG_FILE)) { \
		_dprintf(fmt, ## args); \
	}
#else
#define NMP_DEBUG(fmt, args...) \
	if(f_exists(NMP_DEBUG_FILE)) { \
		printf(fmt, ## args); \
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

typedef unsigned char UCHAR;
typedef unsigned short USHORT;
typedef unsigned long ULONG;

#if defined(RTCONFIG_SOC_IPQ8064) && defined(RTCONFIG_WIFI_QCA9994_QCA9994)
#define MAX_NR_CLIENT_LIST	1024	/* occupies 436252 bytes. */
#else
#define MAX_NR_CLIENT_LIST	255	/* occupies 108656 bytes. */
#endif

enum
{
	FLAG_HTTP = 0,
	FLAG_PRINTER,
	FLAG_ITUNE,
	FLAG_EXIST,
	FLAG_VENDOR
};

//Device service info data structure
typedef struct {
	unsigned char	ip_addr[MAX_NR_CLIENT_LIST][4];
	unsigned char	mac_addr[MAX_NR_CLIENT_LIST][6];
	unsigned char	user_define[MAX_NR_CLIENT_LIST][16];
	unsigned char	vendor_name[MAX_NR_CLIENT_LIST][32];
	unsigned char	device_name[MAX_NR_CLIENT_LIST][32];
	unsigned char	apple_model[MAX_NR_CLIENT_LIST][16];
	unsigned char	type[MAX_NR_CLIENT_LIST];
	unsigned char	ipMethod[MAX_NR_CLIENT_LIST][7];
	unsigned char	opMode[MAX_NR_CLIENT_LIST];
/* define bitmap flag:	0.http
			1.printer
			2.itune
			3.exist
			4.vendor
*/			
	unsigned char	device_flag[MAX_NR_CLIENT_LIST];
/* wireless: 0:wired 1:2.4G 2:5G 3:5G-2
*/
	unsigned char	wireless[MAX_NR_CLIENT_LIST];
/* wireless log information
*/
#ifdef RTCONFIG_LANTIQ
	time_t tstamp[MAX_NR_CLIENT_LIST];
#endif
	char		ssid[MAX_NR_CLIENT_LIST][32];
	char 		txrate[MAX_NR_CLIENT_LIST][7];
	char 		rxrate[MAX_NR_CLIENT_LIST][10];
	unsigned int 	rssi[MAX_NR_CLIENT_LIST];
	char 		conn_time[MAX_NR_CLIENT_LIST][12];
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
	unsigned char hwaddr_len;
	unsigned char ipaddr_len;		
	unsigned short	message_type;
	unsigned char source_hwaddr[6];		     
	unsigned char source_ipaddr[4];
	unsigned char dest_hwaddr[6];	 
	unsigned char dest_ipaddr[4];
} ARP_HEADER;

int FindHostname(P_CLIENT_DETAIL_INFO_TABLE p_client_detail_info_tab);
int FindDeviceMac(unsigned char *pIP, unsigned char *pMac);
void find_wireless_device(P_CLIENT_DETAIL_INFO_TABLE p_client_detail_info_tab, int offline);
#endif
