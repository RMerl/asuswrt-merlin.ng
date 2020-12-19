#ifndef __WLSCAN_H_
#define __WLSCAN_H_

#define BIT(n) (1 << (n))
#define WPA_GET_LE16(a) ((unsigned int) (((a)[1] << 8) | (a)[0]))

#define WPA_CIPHER_NONE_ BIT(0)
#define WPA_CIPHER_WEP40_ BIT(1)
#define WPA_CIPHER_WEP104_ BIT(2)
#define WPA_CIPHER_TKIP_ BIT(3)
#define WPA_CIPHER_CCMP_ BIT(4)

#define WPA_KEY_MGMT_IEEE8021X_ BIT(0)
#define WPA_KEY_MGMT_IEEE8021X2_ BIT(6)
#define WPA_KEY_MGMT_PSK_ BIT(1)
#define WPA_KEY_MGMT_PSK2_ BIT(5)
#define WPA_KEY_MGMT_NONE_ BIT(2)
#define WPA_KEY_MGMT_IEEE8021X_NO_WPA_ BIT(3)
#define WPA_KEY_MGMT_WPA_NONE_ BIT(4)

#define WPA_PROTO_WPA_ BIT(0)
#define WPA_PROTO_RSN_ BIT(1)

#ifdef RTCONFIG_HND_ROUTER_AX
#define _WPA_CIPHER_NONE_ BIT(0)
#define _WPA_CIPHER_WEP_40_ BIT(1)
#define _WPA_CIPHER_WEP_104_ BIT(2)
#define _WPA_CIPHER_TKIP_ BIT(3)
#define _WPA_CIPHER_AES_CCM_ BIT(4)

#define _RSN_AKM_UNSPECIFIED_ BIT(0)
#define _RSN_AKM_PSK_ BIT(1)
#define _RSN_AKM_SHA256_1X_ BIT(2)
#define _RSN_AKM_SHA256_PSK_ BIT(3)
#define _RSN_AKM_SAE_PSK_ BIT(4)
#define _RSN_AKM_OWE_ BIT(5)
#endif

#define PMKID_LEN 16

#define NUMCHANS 64

#define WSC_ID_VERSION				0x104A
#define WSC_ID_VERSION_LEN			1
#define WSC_ID_VERSION_BEACON			0x00000001

#define WSC_ID_SC_STATE				0x1044
#define WSC_ID_SC_STATE_LEN			1
#define WSC_ID_SC_STATE_BEACON			0x00000002

#define WSC_ID_AP_SETUP_LOCKED			0x1057
#define WSC_ID_AP_SETUP_LOCKED_LEN		1
#define WSC_ID_AP_SETUP_LOCKED_BEACON		0x00000004

#define WSC_ID_SEL_REGISTRAR			0x1041
#define WSC_ID_SEL_REGISTRAR_LEN		1
#define WSC_ID_SEL_REGISTRAR_BEACON		0x00000008

#define WSC_ID_DEVICE_PWD_ID			0x1012
#define WSC_ID_DEVICE_PWD_ID_LEN		2
#define WSC_ID_DEVICE_PWD_ID_BEACON		0x00000010

#define WSC_ID_SEL_REG_CFG_METHODS		0x1053
#define WSC_ID_SEL_REG_CFG_METHODS_LEN		2
#define WSC_ID_SEL_REG_CFG_METHODS_BEACON	0x00000020

#define WSC_ID_UUID_E				0x1047
#define WSC_ID_UUID_E_LEN			16
#define WSC_ID_UUID_E_BEACON			0x00000040

#define WSC_ID_RF_BAND				0x103C
#define WSC_ID_RF_BAND_LEN			1
#define WSC_ID_RF_BAND_BEACON			0x00000080

#define WSC_ID_PRIMARY_DEVICE_TYPE		0x1054
#define WSC_ID_PRIMARY_DEVICE_TYPE_LE		8
#define WSC_ID_PRIMARY_DEVICE_TYPE_BEACON	0x00000100

static const int WPA_SELECTOR_LEN = 4;
static const unsigned char WPA_OUI_TYPE_ARR[] = { 0x00, 0x50, 0xf2, 1 };
static const unsigned int WPA_VERSION_ = 1;
static const unsigned char WPA_AUTH_KEY_MGMT_NONE[] = { 0x00, 0x50, 0xf2, 0 };
static const unsigned char WPA_AUTH_KEY_MGMT_UNSPEC_802_1X[] = { 0x00, 0x50, 0xf2, 1 };
static const unsigned char WPA_AUTH_KEY_MGMT_PSK_OVER_802_1X[] = { 0x00, 0x50, 0xf2, 2 };
static const unsigned char WPA_CIPHER_SUITE_NONE[] = { 0x00, 0x50, 0xf2, 0 };
static const unsigned char WPA_CIPHER_SUITE_WEP40[] = { 0x00, 0x50, 0xf2, 1 };
static const unsigned char WPA_CIPHER_SUITE_TKIP[] = { 0x00, 0x50, 0xf2, 2 };
static const unsigned char WPA_CIPHER_SUITE_WRAP[] = { 0x00, 0x50, 0xf2, 3 };
static const unsigned char WPA_CIPHER_SUITE_CCMP[] = { 0x00, 0x50, 0xf2, 4 };
static const unsigned char WPA_CIPHER_SUITE_WEP104[] = { 0x00, 0x50, 0xf2, 5 };

static const int RSN_SELECTOR_LEN = 4;
static const unsigned int RSN_VERSION_ = 1;
static const unsigned char RSN_AUTH_KEY_MGMT_UNSPEC_802_1X[] = { 0x00, 0x0f, 0xac, 1 };
static const unsigned char RSN_AUTH_KEY_MGMT_PSK_OVER_802_1X[] = { 0x00, 0x0f, 0xac, 2 };
static const unsigned char RSN_CIPHER_SUITE_NONE[] = { 0x00, 0x0f, 0xac, 0 };
static const unsigned char RSN_CIPHER_SUITE_WEP40[] = { 0x00, 0x0f, 0xac, 1 };
static const unsigned char RSN_CIPHER_SUITE_TKIP[] = { 0x00, 0x0f, 0xac, 2 };
static const unsigned char RSN_CIPHER_SUITE_WRAP[] = { 0x00, 0x0f, 0xac, 3 };
static const unsigned char RSN_CIPHER_SUITE_CCMP[] = { 0x00, 0x0f, 0xac, 4 };
static const unsigned char RSN_CIPHER_SUITE_WEP104[] = { 0x00, 0x0f, 0xac, 5 };

struct wpa_ie_data {
	int proto;
	int pairwise_cipher;
	int group_cipher;
	int key_mgmt;
	int capabilities;
	int num_pmkid;
	const unsigned char *pmkid;
};

struct wpa_ie_hdr {
	unsigned char elem_id;
	unsigned char len;
	unsigned char oui[3];
	unsigned char oui_type;
	unsigned char version[2];
} __attribute__ ((packed));

struct rsn_ie_hdr {
	unsigned char elem_id; /* WLAN_EID_RSN */
	unsigned char len;
	unsigned char version[2];
} __attribute__ ((packed));

// Added new types for OFDM 5G and 2.4G
typedef enum _NDIS_802_11_NETWORK_TYPE
{
	Ndis802_11FH,
	Ndis802_11DS,
	Ndis802_11OFDM5,
	Ndis802_11OFDM24,
	Ndis802_11Automode,
	Ndis802_11OFDM5_N,
	Ndis802_11OFDM24_N,
	Ndis802_11OFDM5_VHT,
	Ndis802_11OFDMA5_HE,
	Ndis802_11OFDMA24_HE,
	Ndis802_11NetworkTypeMax    // not a real type, defined as an upper bound
} NDIS_802_11_NETWORK_TYPE;

struct bss_ie_hdr {
	unsigned char elem_id;
	unsigned char len;
	unsigned char oui[3];
};
extern struct bss_ie_hdr bss_ie;

#define MAX_NUMBER_OF_APINFO	128

#ifdef CONFIG_BCMWL5
/* 802.11i/WPA RSN IE parsing utilities */
typedef struct {
	uint16 version;
	wpa_suite_mcast_t *mcast;
	wpa_suite_ucast_t *ucast;
	wpa_suite_auth_key_mgmt_t *akm;
	uint8 *capabilities;
} rsn_parse_info_t;
#endif

struct apinfo
{
	char BSSID[18];
	char SSID[33];
	int RSSI_Quality;
	unsigned char channel;
	unsigned char ctl_ch;
	unsigned int capability;
	int wep;
	int wpa;
	struct wpa_ie_data wid;
#ifdef RTCONFIG_HND_ROUTER_AX
	rsn_parse_info_t rsn_info;
#endif
	int status;
	int NetworkType;
#ifdef RTCONFIG_AMAS
	int amas;
#endif
};
extern struct apinfo apinfos[MAX_NUMBER_OF_APINFO];

#ifdef RTCONFIG_REALTEK
/* [MUST] : must to modify */
#define APINFO_MAX 64
typedef struct apinfo apinf_t;
//int apinfo_count=0;
#endif
#define WIF "eth1"
#define WLC_SCAN_RESULT_BUF_LEN	64 * 1024
extern char buf[WLC_IOCTL_MAXLEN];

#endif

#if defined(RTCONFIG_AMAS) || defined(RTCONFIG_CFGSYNC)
struct tlvbase {
	uchar type;
	uchar len;
	uchar data[1];
};
#endif
