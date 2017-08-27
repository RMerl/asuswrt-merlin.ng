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

#ifdef PROTOCOL_QUERY
//Service Port
#define HTTP_PORT		80
#define NBNS_PORT		137
#define NBSS_PORT		139
#define LPD_PORT		515
#define MDNS_PORT		5353
#define RAW_PORT		9100

//for itune
#define uint16 unsigned short
#define PROTOCOL_NAME "_daap._tcp.local"
#define SET_UINT16( num, buff) num = htons(*(uint16*)buff); buff += 2

//for UPNP
#define SERVICE_NUM		10
#define UPNP_BUFSIZE		1500
#define MIN_SEARCH_TIME		3
#define MAX_SEARCH_TIME		180
#define SSDP_IP			"239.255.255.250"
#define SSDP_PORT		1900
#define MATCH_PREFIX(a, b)	(strncmp((a),(b),sizeof(b)-1)==0)
#define IMATCH_PREFIX(a, b)	(strncasecmp((a),(b),sizeof(b)-1)==0)

//for smb
#define TIME_OUT_TIME		5
#define NBSS_REQ		1
#define NBSS_RSP		2
#define SMB_NEGOTIATE_REQ	3
#define SMB_NEGOTIATE_RSP	4
#define SMB_SESSON_ANDX_REQ	5
#define SMB_SESSON_ANDX_RSP	6
#endif //end of PROTOCOL_QUERY

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


#if (defined(RTCONFIG_JFFS2) || defined(RTCONFIG_JFFSV1) || defined(RTCONFIG_BRCM_NAND_JFFS2))
#define NMP_CLIENT_LIST_FILENAME	"/jffs/nmp_client_list"
#else
#define NMP_CLIENT_LIST_FILENAME	"/tmp/nmp_client_list"
#endif

#define NCL_LIMIT		14336   //nmp_client_list limit to 14KB to avoid UI glitch
#define SINGLE_CLIENT_SIZE	109

#define NMP_DEBUG_FILE			"/tmp/NMP_DEBUG"
#define NMP_DEBUG_MORE_FILE		"/tmp/NMP_DEBUG_MORE"
#define NMP_DEBUG_FUNCTION_FILE		"/tmp/NMP_DEBUG_FUNCTION"

#define NEWORKMAP_OUI_FILE		"/usr/networkmap/networkmap.oui.js"
#if (defined(RTCONFIG_JFFS2) || defined(RTCONFIG_JFFSV1) || defined(RTCONFIG_BRCM_NAND_JFFS2))
#define NMP_CL_JSON_FILE		"/jffs/nmp_cl_json.js"
#else
#define NMP_CL_JSON_FILE		"/tmp/nmp_cl_json.js"
#endif

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

#if defined(RTCONFIG_SOC_IPQ40XX) && defined(RTCONFIG_WIFI_QCA9994_QCA9994)
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

#ifdef PROTOCOL_QUERY
typedef struct Raw_socket {
	int raw_sockfd;
	int raw_buflen;
	char device;
	char pad[3];
	unsigned char *raw_buffer;
}r_socket;

typedef struct
{
	unsigned char dest_hwaddr[6];
	unsigned char source_hwaddr[6];
	unsigned short	frame_type;
	char		pad[2];
} ETH_HEADER;

/* NetBIOS Datagram header: 14 Bytes */
typedef struct
{
	unsigned char message_type;
	unsigned char frag_node;
	unsigned short datagram_id;
	unsigned char source_ipaddr[4];
	unsigned short src_port;
	unsigned short datagram_len;
	unsigned short packet_off;
	char		 pad[2];
} NETBIOS_D_HEADER;

/* NetBIOS Datagram data section */
typedef struct
{
	unsigned char src_name[34]; // 255 bytes maximum
	unsigned char dst_name[34]; // 255 bytes maximum
	unsigned char usr_data[512]; // maximum of 512 bytes
}NETBIOS_D_SECTION;

typedef struct
{
	unsigned short transaction_id;
	unsigned char flags[2];
	unsigned short questions;
	unsigned short answer_rrs;
	unsigned short authority_rrs;
	unsigned short additional_rrs;
	unsigned char name[34];
	unsigned short type;
	unsigned short name_class;
	unsigned char ttl[4];
	unsigned short data_len;
	unsigned char number_of_names;
	unsigned char device_name1[16];
	unsigned char name_flags1[2];
	unsigned char device_name2[16];
	unsigned char name_flags2[2];
	unsigned char device_name3[16];
	unsigned char name_flags3[2];
	unsigned char device_name4[16];
	unsigned char name_flags4[2];
	unsigned char device_name5[16];
	unsigned char name_flags5[2];
	unsigned char device_name6[16];
	unsigned char name_flags6[2];
	unsigned char mac_addr[6];
	unsigned char name_info[58];
	char		pad[3];
} NBNS_RESPONSE;

struct LPDProtocol
{
	unsigned char cmd_code;		/* command code */
	unsigned char options[32];  /* Queue name */
	unsigned char lf;
	char	  pad[2];
};

//for itune
typedef struct dns_header_s{
	uint16 id;
	uint16 flags;
	uint16 qdcount;
	uint16 ancount;
	uint16 nscount;
	uint16 arcount;
} dns_header_s;

//for UPNP
struct service
{
	char name[LINE_SIZE];
	char url[LINE_SIZE];
};

struct device_info
{
	// name:		<friendlyName>
	char friendlyname[LINE_SIZE];
	// Manufacturer:	<manufacturer>
	char manufacturer[LINE_SIZE];
	// Description:		<modelDescription>
	char description[LINE_SIZE];
	// Model Name:		<modelName>
	char modelname[LINE_SIZE];
	// Model Number:	<modelNumber>
	char modelnumber[LINE_SIZE];
	// Device Address:	<presentationURL>
	char presentation[LINE_SIZE];
	// the service information
	struct service service[SERVICE_NUM];
	int service_num;
};

//for smb
typedef struct {
	unsigned char msg_type;
	unsigned char flags;
	unsigned short length;
} NBSS_HEADER;

union {
	struct {
		UCHAR  ErrorClass;	  // Error class
		UCHAR  Reserved;	  // Reserved for future use
		USHORT Error;		  // Error code
	} DosError;
	ULONG Status;		      // 32-bit error code
} Status;

union {
	USHORT Pad[6];		      // Ensure section is 12 bytes long
	struct {
		USHORT PidHigh;		  // High part of PID
		ULONG  Unused;		  // Not used
		ULONG  Unused2;
	} Extra;
} Pad;

typedef struct
{
	UCHAR Protocol[4];		  // Contains 0xFF,'SMB'
	UCHAR Command;			  // Command code
	UCHAR Status[4];
	UCHAR  Flags;			  // Flags
	USHORT Flags2;			  // More flags
	USHORT Pad[6];
	USHORT Tid;			  // Tree identifier
	USHORT Pid;			  // Caller's process id
	USHORT Uid;			  // Unauthenticated user id
	USHORT Mid;			  // multiplex id
} SMB_HEADER;

typedef struct
{
	UCHAR  WordCount;		      // Count of parameter words = 13
	UCHAR  AndXCommand;		      // Secondary (X) command;  0xFF = none
	UCHAR  AndXReserved;		      // Reserved (must be 0)
	USHORT AndXOffset;		      // Offset to next command WordCount
	USHORT MaxBufferSize;		      // Client's maximum buffer size
	USHORT MaxMpxCount;		      // Actual maximum multiplexed pending requests
	USHORT VcNumber;		      // 0=first (only),nonzero=additional VC number
	ULONG  SessionKey;		      // Session key (valid iff VcNumber != 0)
	USHORT CaseInsensitivePasswordLength; // Account password size, ANSI
	USHORT CaseSensitivePasswordLength;   // Account password size, Unicode
	USHORT SecurityBlobLen;
	ULONG  Reserved;		      // must be 0
	ULONG  Capabilities;		      // Client capabilities
} SMB_SESSION_SETUPX_REQ;

typedef struct
{
	USHORT ByteCount;		      //Count of data bytes;	min = 0
	UCHAR  CaseInsensitivePassword[32];	//Account Password, ANSI
	UCHAR  CaseSensitivePassword[32];	//Account Password, Unicode
	UCHAR AccountName[32];		       //Account Name, Unicode
	UCHAR PrimaryDomain[32];	       //Client's primary domain, Unicode
	UCHAR NativeOS[128];			//Client's native operating system, Unicode
	UCHAR NativeLanMan[128];		//Client's native LAN Manager type, Unicode
} SMB_CLIENT_INFO;

typedef struct
{
	UCHAR *des_hostname;
	UCHAR *my_hostname;
	UCHAR *account;
	UCHAR *primarydomain;
	UCHAR *nativeOS;
	UCHAR *nativeLanMan;
	USHORT des_hostname_len;
	USHORT my_hostname_len;
	USHORT account_len;
	USHORT primarydomain_len;
	USHORT nativeOS_len;
	USHORT nativeLanMan_len;
} MY_DEVICE_INFO;

int FindAllApp( unsigned char *src_ip, P_CLIENT_DETAIL_INFO_TABLE p_client_detail_info_tab, int i);
#endif //end of PROTOCOL_QUERY
int FindHostname(P_CLIENT_DETAIL_INFO_TABLE p_client_detail_info_tab);
void find_wireless_device(P_CLIENT_DETAIL_INFO_TABLE p_client_detail_info_tab, int offline);
#endif
