#ifndef __REALTEK_H__
#define __REALTEK_H__

#ifndef RPAC92
#include "realtek_old.h"
#else
#include "rtl8198d.h"
#endif
#include <rtconfig.h>
/*
 * LED/Button GPIO# definitions
 */
#define GPIO_DIR_OUT		1 
#define GPIO_DIR_IN		0

#define REALTEK_DEBUG 1
#define DFS	1	/* for dfs */

#define MIB_BUFF_MAX_SIZE 1024
//#define RTK_DEBUG
#ifdef RTK_DEBUG
#define rtklog cprintf
#define rtk_printf cprintf
#else
#define rtklog(fmt,args...) do {} while(0)
#define rtk_printf(fmt,args...) do {} while(0)
#endif
#define rtkerr cprintf
#define rtkinfo cprintf

#ifdef RPAC92
typedef enum wlan_band{WLAN_5G=0,WLAN_5G_2=1,WLAN_2G=2}wlan_band;
#else
typedef enum wlan_band{WLAN_5G=0,WLAN_2G=1}wlan_band;
#endif

typedef enum { FCC=1, IC, ETSI, SPAIN, FRANCE, MKK } REG_DOMAIN_T;
enum { WSC_AUTH_OPEN=1, WSC_AUTH_WPAPSK=2, WSC_AUTH_SHARED=4, WSC_AUTH_WPA=8, WSC_AUTH_WPA2=0x10, WSC_AUTH_WPA2PSK=0x20, WSC_AUTH_WPA2PSKMIXED=0x22 };
enum { WSC_ENCRYPT_NONE=1, WSC_ENCRYPT_WEP=2, WSC_ENCRYPT_TKIP=4, WSC_ENCRYPT_AES=8, WSC_ENCRYPT_TKIPAES=12 };

enum { 
	CONFIG_METHOD_ETH=0x2, 
	CONFIG_METHOD_PIN=0x4, 
	CONFIG_METHOD_DISPLAY=0x8  ,		
	CONFIG_METHOD_PBC=0x80, 
	CONFIG_METHOD_KEYPAD=0x100,
	CONFIG_METHOD_VIRTUAL_PBC=0x280	,
	CONFIG_METHOD_PHYSICAL_PBC=0x480,
	CONFIG_METHOD_VIRTUAL_PIN=0x2008,
	CONFIG_METHOD_PHYSICAL_PIN=0x4008
};
enum { 
	MODE_AP_UNCONFIG=1, 			// AP unconfigured (enrollee)
	MODE_CLIENT_UNCONFIG=2, 		// client unconfigured (enrollee) 
	MODE_CLIENT_CONFIG=3,			// client configured (External registrar) 
	MODE_AP_PROXY=4, 				// AP configured (proxy)
	MODE_AP_PROXY_REGISTRAR=5,		// AP configured (proxy and registrar)
	MODE_CLIENT_UNCONFIG_REGISTRAR=6		// client unconfigured (registrar)
};

typedef struct {
	const char *name;
	unsigned char band_2G;
	unsigned char band_5G;
	unsigned short txpwr_lmt_index;
} reg_domain_t;

enum WIFI_REG_DOMAIN {
	DOMAIN_FCC		= 1,
	DOMAIN_IC		= 2,
	DOMAIN_ETSI		= 3,
	DOMAIN_SPAIN	= 4,
	DOMAIN_FRANCE	= 5,
	DOMAIN_MKK		= 6,
	DOMAIN_ISRAEL	= 7,
	DOMAIN_MKK1		= 8,
	DOMAIN_MKK2		= 9,
	DOMAIN_MKK3		= 10,
	DOMAIN_NCC		= 11,
	DOMAIN_RUSSIAN	= 12,
	DOMAIN_CN		= 13,
	DOMAIN_GLOBAL	= 14,
	DOMAIN_WORLD_WIDE = 15,
	DOMAIN_TEST		= 16,
	DOMAIN_5M10M	= 17,
	DOMAIN_SG		= 18,
	DOMAIN_KR		= 19,
	DOMAIN_AU		= 20,
	DOMAIN_MAX
};

/*
 *
 * static const COUNTRY_IE_ELEMENT countryIEArray[]
 * realtek/rtl819x/linux-2.6.30/drivers/net/wireless/rtl8192cd/8192cd_11h.c
 * src-rt-6.x/router/shared/sysdeps/realtek/realtek.h
 *
 * the country and reg mapping in countryIEArray is not up to date.
 *
 */
static const reg_domain_t reg_domain[] = {
	{ "US", DOMAIN_FCC,  DOMAIN_FCC,  0  },
#ifdef RPAC55
	{ "CA", DOMAIN_IC,   DOMAIN_IC,	  5  },
#else
	{ "CA", DOMAIN_FCC,  DOMAIN_FCC,  3  },
#endif
	{ "EU", DOMAIN_ETSI, DOMAIN_ETSI, 0  },
	{ "AA", DOMAIN_ETSI, DOMAIN_ETSI, 0  },
	{ "TW", DOMAIN_FCC,  DOMAIN_FCC,  0  },
	{ "SG", DOMAIN_SG,   DOMAIN_SG,   0  },
	{ "CN", DOMAIN_CN,   DOMAIN_CN,   0  },
	{ "KR", DOMAIN_KR,   DOMAIN_KR,   0  },
	{ "JP", DOMAIN_MKK,  DOMAIN_MKK,  3  },
	{ "AU", DOMAIN_AU,   DOMAIN_AU,   4  },
	{ "IL", DOMAIN_ISRAEL, DOMAIN_ISRAEL, 2},
};

typedef enum _Capability {
	cESS 		= 0x01,
	cIBSS		= 0x02,
	cPollable		= 0x04,
	cPollReq		= 0x01,
	cPrivacy		= 0x10,
	cShortPreamble	= 0x20,
} Capability;

typedef struct wlan_rate{
unsigned int id;
unsigned char rate[20];
}WLAN_RATE_T, *WLAN_RATE_Tp;
typedef enum {
	MCS0=0x80,
	MCS1=0x81,
	MCS2=0x82,
	MCS3=0x83,
	MCS4=0x84,
	MCS5=0x85,
	MCS6=0x86,
	MCS7=0x87,
	MCS8=0x88,
	MCS9=0x89,
	MCS10=0x8a,
	MCS11=0x8b,
	MCS12=0x8c,
	MCS13=0x8d,
	MCS14=0x8e,
	MCS15=0x8f,
	MCS16=0x90,
	MCS17=0x91,
	MCS18=0x92,
	MCS19=0x93,
	MCS20=0x94,
	MCS21=0x95,
	MCS22=0x96,
	MCS23=0x97
	} RATE_11N_T;


//changes in following table should be synced to VHT_MCS_DATA_RATE[] in 8812_vht_gen.c
// 				20/40/80,	ShortGI,	MCS Rate
static const unsigned short VHT_MCS_DATA_RATE[3][2][30] =
	{	{	{13, 26, 39, 52, 78, 104, 117, 130, 156, 156,
			 26, 52, 78, 104, 156, 208, 234, 260, 312, 312,
			 39, 78, 117, 156, 234, 312, 351, 390, 468, 520},					// Long GI, 20MHz

			{14, 29, 43, 58, 87, 116, 130, 144, 173, 173,
			 29, 58, 87, 116, 173, 231, 260, 289, 347, 347,
			 43, 86, 130, 173, 260, 347, 390, 433, 520, 578}			},		// Short GI, 20MHz

		{	{27, 54, 81, 108, 162, 216, 243, 270, 324, 360,
			 54, 108, 162, 216, 324, 432, 486, 540, 648, 720,
			 81, 162, 243, 342, 486, 648, 729, 810, 972, 1080}, 				// Long GI, 40MHz

			{30, 60, 90, 120, 180, 240, 270, 300,360, 400,
			 60, 120, 180, 240, 360, 480, 540, 600, 720, 800,
			 90, 180, 270, 360, 540, 720, 810, 900, 1080, 1200}			},		// Short GI, 40MHz

		{	{59, 117,  176, 234, 351, 468, 527, 585, 702, 780,
			 117, 234, 351, 468, 702, 936, 1053, 1170, 1404, 1560,
			 176, 351, 527, 702, 1053, 1408, 1408, 1745, 2106, 2340}, 			// Long GI, 80MHz

			{65, 130, 195, 260, 390, 520, 585, 650, 780, 867,
			 130, 260, 390, 520, 780, 1040, 1170, 1300, 1560, 1733,
			 195, 390, 585, 780, 1170, 1560, 1560, 1950, 2340, 2600}	}		// Short GI, 80MHz

	};
//changes in following table should be synced to MCS_DATA_RATEStr[] in 8190n_proc.c
static const WLAN_RATE_T rate_11n_table_20M_LONG[]={
	{MCS0, 	"65"},
	{MCS1, 	"130"},
	{MCS2, 	"195"},
	{MCS3, 	"260"},
	{MCS4, 	"390"},
	{MCS5, 	"520"},
	{MCS6, 	"5850"},
	{MCS7, 	"650"},
	{MCS8, 	"130"},
	{MCS9, 	"260"},
	{MCS10, 	"390"},
	{MCS11, 	"520"},
	{MCS12, 	"780"},
	{MCS13, 	"1040"},
	{MCS14, 	"1170"},
	{MCS15, 	"1300"},
	{MCS16, 	"195"},
	{MCS17, 	"390"},
	{MCS18, 	"585"},
	{MCS19, 	"780"},
	{MCS20, 	"1170"},
	{MCS21, 	"1560"},
	{MCS22, 	"1755"},
	{MCS23, 	"1950"},
	{0}
};
static const WLAN_RATE_T rate_11n_table_20M_SHORT[]={
	{MCS0, 	"72"},
	{MCS1, 	"144"},
	{MCS2, 	"217"},
	{MCS3, 	"289"},
	{MCS4, 	"433"},
	{MCS5, 	"578"},
	{MCS6, 	"650"},
	{MCS7, 	"722"},
	{MCS8, 	"144"},
	{MCS9, 	"289"},
	{MCS10, 	"433"},
	{MCS11, 	"578"},
	{MCS12, 	"867"},
	{MCS13, 	"1156"},
	{MCS14, 	"1300"},
	{MCS15, 	"1445"},
	{MCS16, 	"217"},
	{MCS17, 	"433"},
	{MCS18, 	"650"},
	{MCS19, 	"867"},
	{MCS20, 	"1300"},
	{MCS21, 	"1733"},
	{MCS22, 	"1950"},
	{MCS23, 	"2167"},
	{0}
};
static const WLAN_RATE_T rate_11n_table_40M_LONG[]={
	{MCS0, 	"135"},
	{MCS1, 	"270"},
	{MCS2, 	"405"},
	{MCS3, 	"540"},
	{MCS4, 	"810"},
	{MCS5, 	"1080"},
	{MCS6, 	"1215"},
	{MCS7, 	"1350"},
	{MCS8, 	"270"},
	{MCS9, 	"540"},
	{MCS10, 	"810"},
	{MCS11, 	"1080"},
	{MCS12, 	"1620"},
	{MCS13, 	"2160"},
	{MCS14, 	"2430"},
	{MCS15, 	"2700"},
	{MCS16, 	"405"},
	{MCS17, 	"810"},
	{MCS18, 	"1215"},
	{MCS19, 	"1620"},
	{MCS20, 	"2430"},
	{MCS21, 	"3240"},
	{MCS22, 	"3645"},
	{MCS23, 	"4050"},
	{0}
};
static const WLAN_RATE_T rate_11n_table_40M_SHORT[]={
	{MCS0, 	"150"},
	{MCS1, 	"300"},
	{MCS2, 	"450"},
	{MCS3, 	"600"},
	{MCS4, 	"900"},
	{MCS5, 	"1200"},
	{MCS6, 	"1350"},
	{MCS7, 	"1500"},
	{MCS8, 	"300"},
	{MCS9, 	"600"},
	{MCS10, 	"900"},
	{MCS11, 	"1200"},
	{MCS12, 	"1800"},
	{MCS13, 	"2400"},
	{MCS14, 	"2700"},
	{MCS15, 	"3000"},
	{MCS16, 	"450"},
	{MCS17, 	"900"},
	{MCS18, 	"1350"},
	{MCS19, 	"1800"},
	{MCS20, 	"2700"},
	{MCS21, 	"3600"},
	{MCS22, 	"4050"},
	{MCS23, 	"4500"},
	{0}
};

typedef struct _COUNTRY_IE_ELEMENT_ {
	unsigned int		countryNumber;
	unsigned char 		countryA2[3];
	unsigned char		A_Band_Region;	//if support 5G A band? ;  0 == no support ; aBandRegion == real region domain
	unsigned char		G_Band_Region;	//if support 2.4G G band? ;  0 == no support ; bBandRegion == real region domain
} COUNTRY_IE_ELEMENT;

static const COUNTRY_IE_ELEMENT countryIEArray[] =
{
	/*
	 format: countryNumber | CountryCode(A2)
	*/
	{8,"AL",   3, 3},   /*ALBANIA*/
	{12,"DZ",  3, 3},  /*ALGERIA*/
	{32,"AR",  3, 3},  /*ARGENTINA*/
	{51,"AM",  3, 3},  /*ARMENIA*/
	{36,"AU",  20, 20},  /*AUSTRALIA*/
	{40,"AT",  3, 3},  /*AUSTRIA*/
	{31,"AZ",  3, 3},  /*AZERBAIJAN*/
	{48,"BH",  3, 3},  /*BAHRAIN*/
	{112,"BY",  3, 3},  /*BELARUS*/
	{56,"BE",  3, 3},  /*BELGIUM*/
	{84,"BZ",  3, 3},  /*BELIZE*/
	{68,"BO",  3, 3},  /*BOLIVIA*/
	{76,"BR",  3, 3},  /*BRAZIL*/
	{96,"BN",  3, 3},  /*BRUNEI*/
	{100,"BG", 3, 3}, /*BULGARIA*/
	{124,"CA", 1, 1}, /*CANADA*/
	{152,"CL", 3, 3}, /*CHILE*/
	{156,"CN",13,13}, /*CHINA*/
	{170,"CO", 1, 1}, /*COLOMBIA*/
	{188,"CR", 3, 3}, /*COSTA RICA*/
	{191,"HR", 3, 3}, /*CROATIA*/
	{196,"CY", 3, 3}, /*CYPRUS*/
	{203,"CZ", 3, 3}, /*CZECH REPUBLIC*/
	{208,"DK", 3, 3}, /*DENMARK*/
	{214,"DO", 1, 1}, /*DOMINICAN REPUBLIC*/
	{218,"EC", 3, 3}, /*ECUADOR*/
	{818,"EG", 3, 3}, /*EGYPT*/
	{221,"EU", 3, 3}, /*EU*/
	{142,"AA", 3, 3}, /*AA*/
	{222,"SV", 3, 3}, /*EL SALVADOR*/
	{233,"EE", 3, 3}, /*ESTONIA*/
	{246,"FI", 3, 3}, /*FINLAND*/
	{250,"FR", 3, 3}, /*FRANCE*/
	{268,"GE", 3, 3}, /*GEORGIA*/
	{276,"DE", 3, 3}, /*GERMANY*/
	{300,"GR", 3, 3}, /*GREECE*/
	{320,"GT", 1, 1}, /*GUATEMALA*/
	{340,"HN", 3, 3}, /*HONDURAS*/
	{344,"HK", 3, 3}, /*HONG KONG*/
	{348,"HU", 3, 3}, /*HUNGARY*/
	{352,"IS", 3, 3}, /*ICELAND*/
	{356,"IN", 3, 3}, /*INDIA*/
	{360,"ID", 3, 3}, /*INDONESIA*/
	{364,"IR", 3, 3}, /*IRAN*/
	{372,"IE", 3, 3}, /*IRELAND*/
	{376,"IL", 7, 7}, /*ISRAEL*/
	{380,"IT", 3, 3}, /*ITALY*/
	{392,"JP", 6, 6}, /*JAPAN*/
	{400,"JO", 3, 3}, /*JORDAN*/
	{398,"KZ", 3, 3}, /*KAZAKHSTAN*/
	{410,"KR",19,19}, /*NORTH KOREA*/
	{408,"KP", 3, 3}, /*KOREA REPUBLIC*/
	{414,"KW", 3, 3}, /*KUWAIT*/
	{428,"LV", 3, 3}, /*LATVIA*/
	{422,"LB", 3, 3}, /*LEBANON*/
	{438,"LI", 3, 3}, /*LIECHTENSTEIN*/
	{440,"LT", 3, 3}, /*LITHUANIA*/
	{442,"LU", 3, 3}, /*LUXEMBOURG*/
	{446,"MO", 3, 3}, /*CHINA MACAU*/
	{807,"MK", 3, 3}, /*MACEDONIA*/
	{458,"MY", 3, 3}, /*MALAYSIA*/
	{484,"MX", 1, 1}, /*MEXICO*/
	{492,"MC", 3, 3}, /*MONACO*/
	{504,"MA", 3, 3}, /*MOROCCO*/
	{528,"NL", 3, 3}, /*NETHERLANDS*/
	{554,"NZ", 3, 3}, /*NEW ZEALAND*/
	{578,"NO", 3, 3}, /*NORWAY*/
	{512,"OM", 3, 3}, /*OMAN*/
	{586,"PK", 3, 3}, /*PAKISTAN*/
	{591,"PA", 1, 1}, /*PANAMA*/
	{604,"PE", 3, 3}, /*PERU*/
	{608,"PH", 3, 3}, /*PHILIPPINES*/
	{616,"PL", 3, 3}, /*POLAND*/
	{620,"PT", 3, 3}, /*PORTUGAL*/
	{630,"PR", 1, 1}, /*PUERTO RICO*/
	{634,"QA", 3, 3}, /*QATAR*/
	{642,"RA", 3, 3}, /*ROMANIA*/
	{643,"RU",12,12}, /*RUSSIAN*/
	{682,"SA", 3, 3}, /*SAUDI ARABIA*/
	{702,"SG",18,18}, /*SINGAPORE*/
	{703,"SK", 3, 3}, /*SLOVAKIA*/
	{705,"SI", 3, 3}, /*SLOVENIA*/
	{710,"ZA", 3, 3}, /*SOUTH AFRICA*/
	{724,"ES", 3, 3}, /*SPAIN*/
	{752,"SE", 3, 3}, /*SWEDEN*/
	{756,"CH", 3, 3}, /*SWITZERLAND*/
	{760,"SY", 3, 3}, /*SYRIAN ARAB REPUBLIC*/
	//{158,"TW", 11, 11}, /*TAIWAN*/
	{158,"TW", 1, 1}, /*TAIWAN*/
	{764,"TH", 3, 3}, /*THAILAND*/
	{780,"TT", 3, 3}, /*TRINIDAD AND TOBAGO*/
	{788,"TN", 3, 3}, /*TUNISIA*/
	{792,"TR", 3, 3}, /*TURKEY*/
	{804,"UA", 3, 3}, /*UKRAINE*/
	{784,"AE", 3, 3}, /*UNITED ARAB EMIRATES*/
	{826,"GB", 3, 3}, /*UNITED KINGDOM*/
	{840,"US", 1, 1}, /*UNITED STATES*/
	{858,"UY", 3, 3}, /*URUGUAY*/
	{860,"UZ", 1, 1}, /*UZBEKISTAN*/
	{862,"VE", 3, 3}, /*VENEZUELA*/
	{704,"VN", 3, 3}, /*VIET NAM*/
	{887,"YE", 3, 3}, /*YEMEN*/
	{716,"ZW", 3, 3}, /*ZIMBABWE*/
};


struct channel_list{
	unsigned char	channel[31];
	unsigned char	len;
};
static struct channel_list reg_channel_2_4g[] = {
	/* FCC */		{{1,2,3,4,5,6,7,8,9,10,11},11},
	/* IC */		{{1,2,3,4,5,6,7,8,9,10,11},11},
	/* ETSI */		{{1,2,3,4,5,6,7,8,9,10,11,12,13},13},
	/* SPAIN */		{{1,2,3,4,5,6,7,8,9,10,11,12,13},13},
	/* FRANCE */	{{10,11,12,13},4},
	/* MKK */		{{1,2,3,4,5,6,7,8,9,10,11,12,13,13},13},
	/* ISRAEL */	{{1,2,3,4,5,6,7,8,9,10,11,12,13},13},
	/* MKK1 */		{{1,2,3,4,5,6,7,8,9,10,11,12,13,14},14},
	/* MKK2 */		{{1,2,3,4,5,6,7,8,9,10,11,12,13,14},14},
	/* MKK3 */		{{1,2,3,4,5,6,7,8,9,10,11,12,13,14},14},
	/* NCC (Taiwan) */	{{1,2,3,4,5,6,7,8,9,10,11},11},
	/* RUSSIAN */	{{1,2,3,4,5,6,7,8,9,10,11,12,13},13},
	/* CN */		{{1,2,3,4,5,6,7,8,9,10,11,12,13},13},
	/* Global */		{{1,2,3,4,5,6,7,8,9,10,11,12,13,14},14},
	/* World_wide */	{{1,2,3,4,5,6,7,8,9,10,11,12,13},13},
	/* Test */		{{1,2,3,4,5,6,7,8,9,10,11,12,13,14},14},
	/* 5M10M */		{{},0},
	/* SG */		{{1,2,3,4,5,6,7,8,9,10,11},11},
	/* KR */		{{1,2,3,4,5,6,7,8,9,10,11,12,13},13},
	/* AU (Austraria) */	{{1,2,3,4,5,6,7,8,9,10,11,12,13},13},
};

#ifdef RPAC92
#ifdef DFS
static struct channel_list reg_channel_5g_full_band[] = {
	/* FCC */		{{36,40,44,48},4},
	/* IC */		{{36,40,44,48,52,56,60,64},8},
	/* ETSI */		{{36,40,44,48,52,56,60,64},8},
	/* SPAIN */		{{36,40,44,48,52,56,60,64},8},
	/* FRANCE */	{{36,40,44,48,52,56,60,64},8},
	/* MKK */		{{36,40,44,48,52,56,60,64},8},
	/* ISRAEL */	{{36,40,44,48,52,56,60,64},8},
	/* MKK1 */		{{34,38,42,46},4},
	/* MKK2 */		{{36,40,44,48},4},
	/* MKK3 */		{{36,40,44,48},4},
	/* NCC (Taiwan) */	{{56,60,64},3},
	/* RUSSIAN */	{{36,40,44,48,52,56,60,64},8},
	/* CN */		{{36,40,44,48,52,56,60,64},8},
	/* Global */		{{36,40,44,48,52,56,60,64},8},
	/* World_wide */	{{36,40,44,48,52,56,60,64},8},
	/* Test */		{{36,40,44,48,52,56,60,64}, 8},
	/* 5M10M */		{{146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170}, 25},
	/* SG */		{{36,40,44,48},4},
	/* KR */		{{36,40,44,48,52,56,60,64},8},
	/* AU (Austraria) */	{{36,40,44,48,52,56,60,64}, 8},
};
static struct channel_list reg_channel_5g_full_band_2[] = {
	/* FCC */		{{149,153,157,161,165},5},
	/* IC */		{{149,153,157,161},4},
	/* ETSI */		{{100,104,108,112,116,132,136,140},8},
	/* SPAIN */		{{100,104,108,112,116,120,124,128,132,136,140},11},
	/* FRANCE */	{{100,104,108,112,116,120,124,128,132,136,140},11},
	/* MKK */		{{100,104,108,112,116,120,124,128,132,136,140},11},
	/* ISRAEL */	{{100,104,108,112,116,120,124,128,132,136,140},11},
	/* MKK1 */		{{},0},
	/* MKK2 */		{{},0},
	/* MKK3 */		{{},0},
	/* NCC (Taiwan) */	{{100,104,108,112,116,136,140,149,153,157,161,165},12},
	/* RUSSIAN */	{{132,136,140,149,153,157,161,165},8},
	/* CN */		{{149,153,157,161,165},5},
	/* Global */		{{100,104,108,112,116,136,140,149,153,157,161,165},12},
	/* World_wide */	{{100,104,108,112,116,136,140,149,153,157,161,165},12},
	/* Test */		{{100,104,108,112, 116,120,124,128, 132,136,140,144, 149,153,157,161, 165,169,173,177}, 20},
	/* 5M10M */		{{146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170}, 25},
	/* SG */		{{149,153,157,161,165},5},
	/* KR */		{{100,104,108,112,116,136,140,149,153,157,161,165},12},
	/* AU (Austraria) */	{{100,104,108,112,116,132,136,140,149,153,157,161,165}, 13},
};

static struct channel_list reg_channel_5g_not_dfs_band[] = {
	/* FCC */		{{36,40,44,48},4},
	/* IC */		{{36,40,44,48},4},
	/* ETSI */		{{36,40,44,48},4},
	/* SPAIN */		{{36,40,44,48},4},
	/* FRANCE */	{{36,40,44,48},4},
	/* MKK */		{{36,40,44,48},4},
	/* ISRAEL */	{{36,40,44,48},4},
	/* MKK1 */		{{34,38,42,46},4},
	/* MKK2 */		{{36,40,44,48},4},
	/* MKK3 */		{{36,40,44,48},4},
	/* NCC (Taiwan) */	{{56,60,64},3},
	/* RUSSIAN */	{{36,40,44,48},4},
	/* CN */		{{36,40,44,48},4},
	/* Global */		{{36,40,44,48},4},
	/* World_wide */	{{36,40,44,48},4},
	/* Test */		{{36,40,44,48,}, 4},
	/* 5M10M */		{{146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170}, 25},
	/* SG */		{{36,40,44,48},4},
	/* KR */		{{36,40,44,48},4},
	/* AU (Austraria) */	{{36,40,44,48}, 4},
};

static struct channel_list reg_channel_5g_not_dfs_band_2[] = {
	/* FCC */		{{149,153,157,161,165},5},
	/* IC */		{{149,153,157,161},5},
	/* ETSI */		{{100,104,108,112,116,132,136,140},8},
	/* SPAIN */		{{100,104,108,112,116,132,136,140},8},
	/* FRANCE */	{{100,104,108,112,116,132,136,140},8},
	/* MKK */		{{100,104,108,112,116,132,136,140},8},
	/* ISRAEL */	{{100,104,108,112,116,132,136,140},8},
	/* MKK1 */		{{},0},
	/* MKK2 */		{{},0},
	/* MKK3 */		{{},0},
	/* NCC (Taiwan) */	{{149,153,157,161,165},5},
	/* RUSSIAN */	{{149,153,157,161,165},5},
	/* CN */		{{149,153,157,161,165},5},
	/* Global */		{{149,153,157,161,165},5},
	/* World_wide */	{{149,153,157,161,165},5},
	/* Test */		{{149,153,157,161, 165,169,173,177}, 8},
	/* 5M10M */		{{146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170}, 25},
	/* SG */		{{149,153,157,161,165},5},
	/* KR */		{{149,153,157,161,165},5},
	/* AU (Austraria) */	{{149,153,157,161,165}, 5},
};
#else

// Exclude DFS channels
static struct channel_list reg_channel_5g_full_band[] = {
	/* FCC */		{{36,40,44,48,149,153,157,161,165},9},
	/* IC */		{{36,40,44,48,149,153,157,161},8},
	/* ETSI */		{{36,40,44,48},4},
	/* SPAIN */		{{36,40,44,48},4},
	/* FRANCE */	{{36,40,44,48},4},
	/* MKK */		{{36,40,44,48},4},
	/* ISRAEL */	{{36,40,44,48},4},
	/* MKK1 */		{{34,38,42,46},4},
	/* MKK2 */		{{36,40,44,48},4},
	/* MKK3 */		{{36,40,44,48},4},
	/* NCC (Taiwan) */	{{56,60,64,149,153,157,161,165},8},
	/* RUSSIAN */	{{36,40,44,48,149,153,157,161,165},9},
	/* CN */		{{36,40,44,48,149,153,157,161,165},9},
	/* Global */		{{36,40,44,48,149,153,157,161,165},9},
	/* World_wide */	{{36,40,44,48,149,153,157,161,165},9},
	/* Test */		{{36,40,44,48, 52,56,60,64, 100,104,108,112, 116,120,124,128, 132,136,140,144, 149,153,157,161, 165,169,173,177}, 28},
	/* 5M10M */		{{146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170}, 25},
	/* SG */		{{36,40,44,48,149,153,157,161,165},9},
	/* KR */		{{36,40,44,48,149,153,157,161,165},9},
	/* AU (Austraria) */	{{36,40,44,48,149,153,157,161,165}, 9},
};
#endif
#else
#ifdef DFS
static struct channel_list reg_channel_5g_full_band[] = {
	/* FCC */		{{36,40,44,48,149,153,157,161,165},9},
	/* IC */		{{36,40,44,48,149,153,157,161,165},9},
	/* ETSI */		{{36,40,44,48,52,56,60,64,100,104,108,112,116,132,136,140},16},
	/* SPAIN */		{{36,40,44,48,52,56,60,64,100,104,108,112,116,120,124,128,132,136,140},19},
	/* FRANCE */	{{36,40,44,48,52,56,60,64,100,104,108,112,116,120,124,128,132,136,140},19},
	/* MKK */		{{36,40,44,48,52,56,60,64,100,104,108,112,116,120,124,128,132,136,140},19},
	/* ISRAEL */	{{36,40,44,48,52,56,60,64},8},
	/* MKK1 */		{{34,38,42,46},4},
	/* MKK2 */		{{36,40,44,48},4},
	/* MKK3 */		{{36,40,44,48,52,56,60,64},8},
	/* NCC (Taiwan) */	{{56,60,64,100,104,108,112,116,136,140,149,153,157,161,165},15},
	/* RUSSIAN */	{{36,40,44,48,52,56,60,64,132,136,140,149,153,157,161,165},16},
	/* CN */		{{36,40,44,48,52,56,60,64,149,153,157,161,165},13},
	/* Global */		{{36,40,44,48,52,56,60,64,100,104,108,112,116,136,140,149,153,157,161,165},20},
	/* World_wide */	{{36,40,44,48,52,56,60,64,100,104,108,112,116,136,140,149,153,157,161,165},20},
	/* Test */		{{36,40,44,48, 52,56,60,64, 100,104,108,112, 116,120,124,128, 132,136,140,144, 149,153,157,161, 165,169,173,177}, 28},
	/* 5M10M */		{{146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170}, 25},
	/* SG */		{{36,40,44,48,149,153,157,161,165},9},
	/* KR */		{{36,40,44,48,52,56,60,64,100,104,108,112,116,136,140,149,153,157,161,165},20},
	/* AU (Austraria) */	{{36,40,44,48,52,56,60,64,100,104,108,112,116,132,136,140,149,153,157,161,165}, 21},
};

static struct channel_list reg_channel_5g_not_dfs_band[] = {
	/* FCC */		{{36,40,44,48,149,153,157,161,165},9},
	/* IC */		{{36,40,44,48,149,153,157,161,165},9},
	/* ETSI */		{{36,40,44,48},4},
	/* SPAIN */		{{36,40,44,48},4},
	/* FRANCE */	{{36,40,44,48},4},
	/* MKK */		{{36,40,44,48},4},
	/* ISRAEL */	{{36,40,44,48},4},
	/* MKK1 */		{{34,38,42,46},4},
	/* MKK2 */		{{36,40,44,48},4},
	/* MKK3 */		{{36,40,44,48},4},
	/* NCC (Taiwan) */	{{56,60,64,149,153,157,161,165},8},
	/* RUSSIAN */	{{36,40,44,48,149,153,157,161,165},9},
	/* CN */		{{36,40,44,48,149,153,157,161,165},9},
	/* Global */		{{36,40,44,48,149,153,157,161,165},9},
	/* World_wide */	{{36,40,44,48,149,153,157,161,165},9},
	/* Test */		{{36,40,44,48, 149,153,157,161, 165,169,173,177}, 12},
	/* 5M10M */		{{146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170}, 25},
	/* SG */		{{36,40,44,48,149,153,157,161,165},9},
	/* KR */		{{36,40,44,48,149,153,157,161,165},9},
	/* AU (Austraria) */	{{36,40,44,48,149,153,157,161,165}, 9},
};
#else

// Exclude DFS channels
static struct channel_list reg_channel_5g_full_band[] = {
	/* FCC */		{{36,40,44,48,149,153,157,161,165},9},
	/* IC */		{{36,40,44,48,149,153,157,161,165},9},
	/* ETSI */		{{36,40,44,48},4},
	/* SPAIN */		{{36,40,44,48},4},
	/* FRANCE */	{{36,40,44,48},4},
	/* MKK */		{{36,40,44,48},4},
	/* ISRAEL */	{{36,40,44,48},4},
	/* MKK1 */		{{34,38,42,46},4},
	/* MKK2 */		{{36,40,44,48},4},
	/* MKK3 */		{{36,40,44,48},4},
	/* NCC (Taiwan) */	{{56,60,64,149,153,157,161,165},8},
	/* RUSSIAN */	{{36,40,44,48,149,153,157,161,165},9},
	/* CN */		{{36,40,44,48,149,153,157,161,165},9},
	/* Global */		{{36,40,44,48,149,153,157,161,165},9},
	/* World_wide */	{{36,40,44,48,149,153,157,161,165},9},
	/* Test */		{{36,40,44,48, 52,56,60,64, 100,104,108,112, 116,120,124,128, 132,136,140,144, 149,153,157,161, 165,169,173,177}, 28},
	/* 5M10M */		{{146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170}, 25},
	/* SG */		{{36,40,44,48,149,153,157,161,165},9},
	/* KR */		{{36,40,44,48,149,153,157,161,165},9},
	/* AU (Austraria) */	{{36,40,44,48,149,153,157,161,165}, 9},
};
#endif
#endif

extern const char WIF_2G[];
extern const char WIF_5G[];
extern const char VXD_5G[];
extern const char VXD_2G[];

extern int get_mac_2g(unsigned char dst[]);
extern int get_mac_5g(unsigned char dst[]);
extern int get_mac_5g_2(unsigned char dst[]);
extern int get_regdomain_from_countrycode(char* country_code, int unit);
extern int hex_to_string(unsigned char *hex,char *str,int len);
extern void assign_diff_AC(unsigned char* pMib, unsigned char* pVal);
#ifdef RTCONFIG_RTL8198D
extern void assign_diff_AC_hex_to_string(unsigned char* pmib,char* str);
#else
extern void assign_diff_AC_hex_to_string(unsigned char* pmib,char* str,int len);
#endif
void start_wlc_connect(int band);
void stop_wlc_connect(int band);
int get_wlan_status(int band);
void set_wlan_status(int band, int enable);
#endif
