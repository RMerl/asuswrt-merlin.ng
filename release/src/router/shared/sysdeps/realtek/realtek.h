#ifndef __REALTEK_H__
#define __REALTEK_H__
#include "../../rtconfig.h"
//#include "../../../rc/rc.h"

#include "rtl_flashmapping.h"

#define BLUETOOTH_HW_SETTING_SUPPORT
#ifdef RTCONFIG_RTK_NAND
#ifndef CONFIG_MTD_NAND 
#error "Check your kernel config, is CONFIG_MTD_NAND enabled?"
#endif
#define HW_SETTING_CHECKSUM
#define NAND_DUAL_SETTING
#endif /* RTCONFIG_RTK_NAND */

#define DYN_ROOTFS_OFFSET
#define ASUS_TRX_FORMAT
extern const char WIF_2G[];
extern const char WIF_5G[];
extern const char VXD_5G[];
extern const char VXD_2G[];

#ifdef ASUS_TRX_FORMAT
//#define IH_MAGIC	0x27051956	/* Image Magic Number */
#include "image.h"
#endif

/*
 * LED/Button GPIO# definitions
 */
#define GPIO_DIR_OUT		1 
#define GPIO_DIR_IN		0

#define REALTEK_DEBUG 1
#define DFS	1	/* for dfs */

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
/*wlan related*/

#ifndef _WPS_DEF_H_
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

enum { WSC_AUTH_OPEN=1, WSC_AUTH_WPAPSK=2, WSC_AUTH_SHARED=4, WSC_AUTH_WPA=8, WSC_AUTH_WPA2=0x10, WSC_AUTH_WPA2PSK=0x20, WSC_AUTH_WPA2PSKMIXED=0x22 };
enum { WSC_ENCRYPT_NONE=1, WSC_ENCRYPT_WEP=2, WSC_ENCRYPT_TKIP=4, WSC_ENCRYPT_AES=8, WSC_ENCRYPT_TKIPAES=12 };


#endif

typedef enum wlan_band{WLAN_5G=0,WLAN_2G=1}wlan_band;

typedef struct _COUNTRY_IE_ELEMENT_ {
    unsigned int		countryNumber;
    unsigned char 		countryA2[3];
	unsigned char		A_Band_Region;	//if support 5G A band? ;  0 == no support ; aBandRegion == real region domain
	unsigned char		G_Band_Region;	//if support 2.4G G band? ;  0 == no support ; bBandRegion == real region domain
} COUNTRY_IE_ELEMENT;


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
	/* ISRAEL */	{{3,4,5,6,7,8,9,10,11,12,13},11},
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

#ifdef DFS
static struct channel_list reg_channel_5g_full_band[] = {
	/* FCC */		{{36,40,44,48,149,153,157,161,165},9},
	/* IC */		{{36,40,44,48,52,56,60,64,149,153,157,161},12},
	/* ETSI */		{{36,40,44,48,52,56,60,64,100,104,108,112,116,132,136,140},16},
	/* SPAIN */		{{36,40,44,48,52,56,60,64,100,104,108,112,116,120,124,128,132,136,140},19},
	/* FRANCE */	{{36,40,44,48,52,56,60,64,100,104,108,112,116,120,124,128,132,136,140},19},
	/* MKK */		{{36,40,44,48,52,56,60,64,100,104,108,112,116,120,124,128,132,136,140},19},
	/* ISRAEL */	{{36,40,44,48,52,56,60,64,100,104,108,112,116,120,124,128,132,136,140},19},
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



/*HW Setting */
#define CMD_SET_ETHERNET			0x01
#define CMD_SET_WIFI				0x02

#define MAX_2G_CHANNEL_NUM_MIB		14
#define MAX_5G_CHANNEL_NUM_MIB		196


#define MAX_5G_DIFF_NUM		14

#define PIN_LEN					8
#ifdef RTCONFIG_REALTEK
#define NUM_WLAN_INTERFACE 2
#else
#define NUM_WLAN_INTERFACE 1
#endif
#define HW_SETTING_HEADER_TAG		((char *)"H6")
#define HW_WLAN_SETTING_OFFSET	13

#define HW_SETTING_HEADER_OFFSET 	6
#define HW_SETTING_ETHMAC_OFFSET 	1
#define ETH_ALEN					6

#define __PACK__			__attribute__ ((packed))


typedef struct hw_wlan_setting {
	unsigned char macAddr[6] __PACK__;
	unsigned char macAddr1[6] __PACK__;
	unsigned char macAddr2[6] __PACK__;
	unsigned char macAddr3[6] __PACK__;
	unsigned char macAddr4[6] __PACK__;
	unsigned char macAddr5[6] __PACK__; 
	unsigned char macAddr6[6] __PACK__; 
	unsigned char macAddr7[6] __PACK__; 
	unsigned char pwrlevelCCK_A[MAX_2G_CHANNEL_NUM_MIB] __PACK__; 	
	unsigned char pwrlevelCCK_B[MAX_2G_CHANNEL_NUM_MIB] __PACK__; 
	unsigned char pwrlevelHT40_1S_A[MAX_2G_CHANNEL_NUM_MIB] __PACK__;	
	unsigned char pwrlevelHT40_1S_B[MAX_2G_CHANNEL_NUM_MIB] __PACK__;	
	unsigned char pwrdiffHT40_2S[MAX_2G_CHANNEL_NUM_MIB] __PACK__;	
	unsigned char pwrdiffHT20[MAX_2G_CHANNEL_NUM_MIB] __PACK__; 
	unsigned char pwrdiffOFDM[MAX_2G_CHANNEL_NUM_MIB] __PACK__; 
	unsigned char regDomain __PACK__; 	
	unsigned char rfType __PACK__; 
	unsigned char ledType __PACK__; // LED type, see LED_TYPE_T for definition	
	unsigned char xCap __PACK__;	
	unsigned char TSSI1 __PACK__;	
	unsigned char TSSI2 __PACK__;	
	unsigned char Ther __PACK__;	
	unsigned char trswitch __PACK__;
	unsigned char trswpape_C9 __PACK__;
	unsigned char trswpape_CC __PACK__;
	unsigned char target_pwr __PACK__;
	unsigned char pa_type __PACK__;	
	unsigned char Ther2 __PACK__;
	unsigned char xCap2 __PACK__;	
	unsigned char Reserved8 __PACK__;
	unsigned char Reserved9 __PACK__;
	unsigned char Reserved10 __PACK__;
	unsigned char pwrlevel5GHT40_1S_A[MAX_5G_CHANNEL_NUM_MIB] __PACK__;	
	unsigned char pwrlevel5GHT40_1S_B[MAX_5G_CHANNEL_NUM_MIB] __PACK__;	
	unsigned char pwrdiff5GHT40_2S[MAX_5G_CHANNEL_NUM_MIB] __PACK__; 
	unsigned char pwrdiff5GHT20[MAX_5G_CHANNEL_NUM_MIB] __PACK__;	
	unsigned char pwrdiff5GOFDM[MAX_5G_CHANNEL_NUM_MIB] __PACK__;

	
	unsigned char wscPin[PIN_LEN+1] __PACK__;	

#if 1
	unsigned char pwrdiff_20BW1S_OFDM1T_A[MAX_2G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char pwrdiff_40BW2S_20BW2S_A[MAX_2G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char pwrdiff_OFDM2T_CCK2T_A[MAX_2G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char pwrdiff_40BW3S_20BW3S_A[MAX_2G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char pwrdiff_4OFDM3T_CCK3T_A[MAX_2G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char pwrdiff_40BW4S_20BW4S_A[MAX_2G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char pwrdiff_OFDM4T_CCK4T_A[MAX_2G_CHANNEL_NUM_MIB] __PACK__;

	unsigned char pwrdiff_5G_20BW1S_OFDM1T_A[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_40BW2S_20BW2S_A[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_40BW3S_20BW3S_A[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_40BW4S_20BW4S_A[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_RSVD_OFDM4T_A[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_80BW1S_160BW1S_A[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_80BW2S_160BW2S_A[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_80BW3S_160BW3S_A[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_80BW4S_160BW4S_A[MAX_5G_DIFF_NUM] __PACK__;


	unsigned char pwrdiff_20BW1S_OFDM1T_B[MAX_2G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char pwrdiff_40BW2S_20BW2S_B[MAX_2G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char pwrdiff_OFDM2T_CCK2T_B[MAX_2G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char pwrdiff_40BW3S_20BW3S_B[MAX_2G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char pwrdiff_OFDM3T_CCK3T_B[MAX_2G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char pwrdiff_40BW4S_20BW4S_B[MAX_2G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char pwrdiff_OFDM4T_CCK4T_B[MAX_2G_CHANNEL_NUM_MIB] __PACK__;

	unsigned char pwrdiff_5G_20BW1S_OFDM1T_B[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_40BW2S_20BW2S_B[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_40BW3S_20BW3S_B[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_40BW4S_20BW4S_B[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_RSVD_OFDM4T_B[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_80BW1S_160BW1S_B[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_80BW2S_160BW2S_B[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_80BW3S_160BW3S_B[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_80BW4S_160BW4S_B[MAX_5G_DIFF_NUM] __PACK__;
#if defined(RPAC68U)
	unsigned char pwrdiff_20BW1S_OFDM1T_C[MAX_2G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char pwrdiff_40BW2S_20BW2S_C[MAX_2G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char pwrdiff_OFDM2T_CCK2T_C[MAX_2G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char pwrdiff_40BW3S_20BW3S_C[MAX_2G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char pwrdiff_4OFDM3T_CCK3T_C[MAX_2G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char pwrdiff_40BW4S_20BW4S_C[MAX_2G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char pwrdiff_OFDM4T_CCK4T_C[MAX_2G_CHANNEL_NUM_MIB] __PACK__;

	unsigned char pwrdiff_5G_20BW1S_OFDM1T_C[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_40BW2S_20BW2S_C[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_40BW3S_20BW3S_C[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_40BW4S_20BW4S_C[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_RSVD_OFDM4T_C[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_80BW1S_160BW1S_C[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_80BW2S_160BW2S_C[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_80BW3S_160BW3S_C[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_80BW4S_160BW4S_C[MAX_5G_DIFF_NUM] __PACK__;

	unsigned char pwrdiff_20BW1S_OFDM1T_D[MAX_2G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char pwrdiff_40BW2S_20BW2S_D[MAX_2G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char pwrdiff_OFDM2T_CCK2T_D[MAX_2G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char pwrdiff_40BW3S_20BW3S_D[MAX_2G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char pwrdiff_OFDM3T_CCK3T_D[MAX_2G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char pwrdiff_40BW4S_20BW4S_D[MAX_2G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char pwrdiff_OFDM4T_CCK4T_D[MAX_2G_CHANNEL_NUM_MIB] __PACK__;

	unsigned char pwrdiff_5G_20BW1S_OFDM1T_D[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_40BW2S_20BW2S_D[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_40BW3S_20BW3S_D[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_40BW4S_20BW4S_D[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_RSVD_OFDM4T_D[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_80BW1S_160BW1S_D[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_80BW2S_160BW2S_D[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_80BW3S_160BW3S_D[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_80BW4S_160BW4S_D[MAX_5G_DIFF_NUM] __PACK__;

	unsigned char pwrlevelCCK_C[MAX_2G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char pwrlevelCCK_D[MAX_2G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char pwrlevelHT40_1S_C[MAX_2G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char pwrlevelHT40_1S_D[MAX_2G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char pwrlevel5GHT40_1S_C[MAX_5G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char pwrlevel5GHT40_1S_D[MAX_5G_CHANNEL_NUM_MIB] __PACK__;
#endif	
#endif

} HW_WLAN_SETTING_T, *HW_WLAN_SETTING_Tp;
typedef struct hw_wlan_ac_setting{
	unsigned char pwrdiff_20BW1S_OFDM1T_A[MAX_2G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char pwrdiff_40BW2S_20BW2S_A[MAX_2G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char pwrdiff_OFDM2T_CCK2T_A[MAX_2G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char pwrdiff_40BW3S_20BW3S_A[MAX_2G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char pwrdiff_4OFDM3T_CCK3T_A[MAX_2G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char pwrdiff_40BW4S_20BW4S_A[MAX_2G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char pwrdiff_OFDM4T_CCK4T_A[MAX_2G_CHANNEL_NUM_MIB] __PACK__;

	unsigned char pwrdiff_5G_20BW1S_OFDM1T_A[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_40BW2S_20BW2S_A[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_40BW3S_20BW3S_A[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_40BW4S_20BW4S_A[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_RSVD_OFDM4T_A[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_80BW1S_160BW1S_A[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_80BW2S_160BW2S_A[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_80BW3S_160BW3S_A[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_80BW4S_160BW4S_A[MAX_5G_DIFF_NUM] __PACK__;


	unsigned char pwrdiff_20BW1S_OFDM1T_B[MAX_2G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char pwrdiff_40BW3S_20BW3S_B[MAX_2G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char pwrdiff_OFDM3T_CCK3T_B[MAX_2G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char pwrdiff_40BW4S_20BW4S_B[MAX_2G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char pwrdiff_OFDM4T_CCK4T_B[MAX_2G_CHANNEL_NUM_MIB] __PACK__;

	unsigned char pwrdiff_5G_20BW1S_OFDM1T_B[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_40BW2S_20BW2S_B[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_40BW3S_20BW3S_B[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_40BW4S_20BW4S_B[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_RSVD_OFDM4T_B[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_80BW1S_160BW1S_B[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_80BW2S_160BW2S_B[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_80BW3S_160BW3S_B[MAX_5G_DIFF_NUM] __PACK__;
	unsigned char pwrdiff_5G_80BW4S_160BW4S_B[MAX_5G_DIFF_NUM] __PACK__;
}HW_WLAN_AC_SETTING_T, *HW_WLAN_AC_SETTING_Tp;
typedef struct hw_setting {
	unsigned char boardVer __PACK__;	// h/w board version
	unsigned char nic0Addr[6] __PACK__;
	unsigned char nic1Addr[6] __PACK__;
	HW_WLAN_SETTING_T wlan[NUM_WLAN_INTERFACE];
	unsigned char countryCode[6] __PACK__;
	unsigned char territoryCode[8] __PACK__;
	unsigned char modelName[16] __PACK__;
} HW_SETTING_T, *HW_SETTING_Tp;
#ifdef BLUETOOTH_HW_SETTING_SUPPORT
typedef struct bluetooth_hw_setting {
	unsigned char btAddr[6] __PACK__;
	unsigned char txPowerIdx[6] __PACK__;
	unsigned char thermalVal __PACK__;
	unsigned char antennaS0 __PACK__;
	unsigned char antennaS1 __PACK__;
	unsigned char xtalCapValue __PACK__;
} BLUETOOTH_HW_SETTING_T, *BLUETOOTH_HW_SETTING_Tp;
#endif
#define TAG_LEN					2
#define SIGNATURE_LEN			4
#define HW_SETTING_VER			3	// hw setting version

#define FW_HEADER_WITH_ROOT	((char *)"cr6c")
#define ROOT_HEADER			((char *)"r6cr")


#define DWORD_SWAP(v) ( (((v&0xff)<<24)&0xff000000) | ((((v>>8)&0xff)<<16)&0xff0000) | \
				((((v>>16)&0xff)<<8)&0xff00) | (((v>>24)&0xff)&0xff) )
#define WORD_SWAP(v) ((unsigned short)(((v>>8)&0xff) | ((v<<8)&0xff00)))

/* Firmware image file header */
typedef struct img_header {
	unsigned char signature[SIGNATURE_LEN];
	unsigned int startAddr;
	unsigned int burnAddr;
	unsigned int len;
}__PACK__ IMG_HEADER_T, *IMG_HEADER_Tp;


/* Config file header */
typedef struct param_header {
	unsigned char signature[SIGNATURE_LEN] __PACK__;  // Tag + version
	unsigned short len __PACK__;
} PARAM_HEADER_T, *PARAM_HEADER_Tp;

#ifdef CONFIG_MTD_NAND
#define FLASH_DEVICE_NAME		("/hw_setting/hw.bin")
#define FLASH_DEVICE_NAME1		("/hw_setting/hw1.bin")
#else
#define FLASH_DEVICE_NAME		("/dev/mtdblock0")
#define FLASH_DEVICE_NAME1		("/dev/mtdblock1")
#endif

#if 1 /* Don't use hardcode offset, refer rtl_flashmapping.h */
#define HW_SETTING_OFFSET  CONFIG_RTL_HW_SETTING_OFFSET
#ifdef BLUETOOTH_HW_SETTING_SUPPORT
#define BLUETOOTH_HW_SETTING_OFFSET  HW_SETTING_OFFSET+0x1000
#endif
#define CODE_IMAGE_OFFSET  CONFIG_RTL_LINUX_IMAGE_OFFSET
#else
#define HW_SETTING_OFFSET  0x20000
#define CODE_IMAGE_OFFSET	0x30000
#endif
#define RTK_HW_MIB_ITEM(name)  #name,((unsigned long)(long *)&(((HW_SETTING_T *)0)->name)),sizeof(((HW_SETTING_T *)0)->name)
#ifdef BLUETOOTH_HW_SETTING_SUPPORT
#define RTK_BLUETOOTH_HW_MIB_ITEM(name)        #name,((unsigned long)(long *)&(((BLUETOOTH_HW_SETTING_T *)0)->name)),sizeof(((BLUETOOTH_HW_SETTING_T *)0)->name)
#endif
#define RTK_HW_WLAN_MIB_ITEM(name)     #name,((unsigned long)(long *)&(((HW_WLAN_SETTING_T *)0)->name)),sizeof(((HW_WLAN_SETTING_T *)0)->name)

#define MIB_BUFF_MAX_SIZE 1024

typedef enum {
	BYTE_T,
	STRING_T,
	BYTE_ARRAY_T,
	WLAN_T
} MIB_TYPE_T;

typedef struct _hw_mib_info{
	char name[64];
	unsigned int offset;
	unsigned int size;
	MIB_TYPE_T type;
} HW_MIB_INFO_T, *HW_MIB_INFO_Tp;
static const HW_MIB_INFO_T hw_wlan_mib[]={
	//offset from HW_WLAN_SETTING_T begin
	//              name,   offset  size            type
	{RTK_HW_WLAN_MIB_ITEM(macAddr),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(macAddr1),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(macAddr2),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(macAddr3),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(macAddr4),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(macAddr5),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(macAddr6),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(macAddr7),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrlevelCCK_A),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrlevelCCK_B),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrlevelHT40_1S_A),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrlevelHT40_1S_B),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiffHT40_2S),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiffHT20),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiffOFDM),BYTE_ARRAY_T},
		
	{RTK_HW_WLAN_MIB_ITEM(regDomain),BYTE_T},
	{RTK_HW_WLAN_MIB_ITEM(rfType),BYTE_T},
	{RTK_HW_WLAN_MIB_ITEM(ledType),BYTE_T},
	{RTK_HW_WLAN_MIB_ITEM(xCap),BYTE_T},
	{RTK_HW_WLAN_MIB_ITEM(TSSI1),BYTE_T},
	{RTK_HW_WLAN_MIB_ITEM(TSSI2),BYTE_T},
	{RTK_HW_WLAN_MIB_ITEM(Ther),BYTE_T},
	{RTK_HW_WLAN_MIB_ITEM(trswitch),BYTE_T},
	{RTK_HW_WLAN_MIB_ITEM(trswpape_C9),BYTE_T},
	{RTK_HW_WLAN_MIB_ITEM(trswpape_CC),BYTE_T},
	{RTK_HW_WLAN_MIB_ITEM(target_pwr),BYTE_T},
	{RTK_HW_WLAN_MIB_ITEM(pa_type),BYTE_T},
	{RTK_HW_WLAN_MIB_ITEM(Ther2),BYTE_T},
	{RTK_HW_WLAN_MIB_ITEM(xCap2),BYTE_T},
	{RTK_HW_WLAN_MIB_ITEM(Reserved8),BYTE_T},
	{RTK_HW_WLAN_MIB_ITEM(Reserved9),BYTE_T},
	{RTK_HW_WLAN_MIB_ITEM(Reserved10),BYTE_T},
               
	{RTK_HW_WLAN_MIB_ITEM(pwrlevel5GHT40_1S_A),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrlevel5GHT40_1S_B),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff5GHT40_2S),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff5GHT20),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff5GOFDM),BYTE_ARRAY_T},
               
	{RTK_HW_WLAN_MIB_ITEM(wscPin),BYTE_ARRAY_T},
               
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_20BW1S_OFDM1T_A),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_40BW2S_20BW2S_A),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_OFDM2T_CCK2T_A),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_40BW3S_20BW3S_A),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_4OFDM3T_CCK3T_A),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_40BW4S_20BW4S_A),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_OFDM4T_CCK4T_A),BYTE_ARRAY_T},
               
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_5G_20BW1S_OFDM1T_A),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_5G_40BW2S_20BW2S_A),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_5G_40BW3S_20BW3S_A),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_5G_40BW4S_20BW4S_A),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_5G_RSVD_OFDM4T_A),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_5G_80BW1S_160BW1S_A),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_5G_80BW2S_160BW2S_A),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_5G_80BW3S_160BW3S_A),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_5G_80BW4S_160BW4S_A),BYTE_ARRAY_T},
               
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_20BW1S_OFDM1T_B),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_40BW2S_20BW2S_B),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_OFDM2T_CCK2T_B),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_40BW3S_20BW3S_B),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_OFDM3T_CCK3T_B),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_40BW4S_20BW4S_B),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_OFDM4T_CCK4T_B),BYTE_ARRAY_T},
               
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_5G_20BW1S_OFDM1T_B),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_5G_40BW2S_20BW2S_B),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_5G_40BW3S_20BW3S_B),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_5G_40BW4S_20BW4S_B),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_5G_RSVD_OFDM4T_B),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_5G_80BW1S_160BW1S_B),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_5G_80BW2S_160BW2S_B),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_5G_80BW3S_160BW3S_B),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_5G_80BW4S_160BW4S_B),BYTE_ARRAY_T},
#if defined(RPAC68U)
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_20BW1S_OFDM1T_C),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_40BW2S_20BW2S_C),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_OFDM2T_CCK2T_C),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_40BW3S_20BW3S_C),BYTE_ARRAY_T},
		{RTK_HW_WLAN_MIB_ITEM(pwrdiff_4OFDM3T_CCK3T_C),BYTE_ARRAY_T},
		{RTK_HW_WLAN_MIB_ITEM(pwrdiff_40BW4S_20BW4S_C),BYTE_ARRAY_T},
		{RTK_HW_WLAN_MIB_ITEM(pwrdiff_OFDM4T_CCK4T_C),BYTE_ARRAY_T},
               
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_5G_20BW1S_OFDM1T_C),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_5G_40BW2S_20BW2S_C),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_5G_40BW3S_20BW3S_C),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_5G_40BW4S_20BW4S_C),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_5G_RSVD_OFDM4T_C),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_5G_80BW1S_160BW1S_C),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_5G_80BW2S_160BW2S_C),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_5G_80BW3S_160BW3S_C),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_5G_80BW4S_160BW4S_C),BYTE_ARRAY_T},
               
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_20BW1S_OFDM1T_D),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_40BW2S_20BW2S_D),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_OFDM2T_CCK2T_D),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_40BW3S_20BW3S_D),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_OFDM3T_CCK3T_D),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_40BW4S_20BW4S_D),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_OFDM4T_CCK4T_D),BYTE_ARRAY_T},
               
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_5G_20BW1S_OFDM1T_D),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_5G_40BW2S_20BW2S_D),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_5G_40BW3S_20BW3S_D),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_5G_40BW4S_20BW4S_D),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_5G_RSVD_OFDM4T_D),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_5G_80BW1S_160BW1S_D),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_5G_80BW2S_160BW2S_D),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_5G_80BW3S_160BW3S_D),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrdiff_5G_80BW4S_160BW4S_D),BYTE_ARRAY_T},
               
	{RTK_HW_WLAN_MIB_ITEM(pwrlevelCCK_C),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrlevelCCK_D),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrlevelHT40_1S_C),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrlevelHT40_1S_D),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrlevel5GHT40_1S_C),BYTE_ARRAY_T},
	{RTK_HW_WLAN_MIB_ITEM(pwrlevel5GHT40_1S_D),BYTE_ARRAY_T},
#endif
	{0},
};

static const HW_MIB_INFO_T hw_mib[]={
	{RTK_HW_MIB_ITEM(boardVer),BYTE_T},
	{RTK_HW_MIB_ITEM(nic0Addr),BYTE_ARRAY_T},
	{RTK_HW_MIB_ITEM(nic1Addr),BYTE_ARRAY_T},
	{RTK_HW_MIB_ITEM(wlan),WLAN_T},
	{RTK_HW_MIB_ITEM(countryCode),BYTE_ARRAY_T},
	{RTK_HW_MIB_ITEM(territoryCode),BYTE_ARRAY_T},
	{RTK_HW_MIB_ITEM(modelName),STRING_T},
	{0},
};

#ifdef BLUETOOTH_HW_SETTING_SUPPORT
static const HW_MIB_INFO_T bluetooth_hw_mib[]={
	{RTK_BLUETOOTH_HW_MIB_ITEM(btAddr),BYTE_ARRAY_T},
	{RTK_BLUETOOTH_HW_MIB_ITEM(txPowerIdx),BYTE_ARRAY_T},
	{RTK_BLUETOOTH_HW_MIB_ITEM(thermalVal),BYTE_T},
	{RTK_BLUETOOTH_HW_MIB_ITEM(antennaS0),BYTE_T},
	{RTK_BLUETOOTH_HW_MIB_ITEM(antennaS1),BYTE_T},
	{RTK_BLUETOOTH_HW_MIB_ITEM(xtalCapValue),BYTE_T},               
	{0},
};
#endif
static int flash_get_mib_info(
		char* name,//input
		unsigned int *offset,unsigned int *size,MIB_TYPE_T *type//output
	)
{
	int i=0;
	int wlan_offset=0;
	unsigned int hw_offset=HW_SETTING_OFFSET+sizeof(PARAM_HEADER_T);
#ifdef BLUETOOTH_HW_SETTING_SUPPORT
	unsigned int bluetooth_offset=BLUETOOTH_HW_SETTING_OFFSET+sizeof(PARAM_HEADER_T);
#endif
	if(!name||!offset||!size||!type) {
		fprintf(stderr,"invlid null input!%s\n",__FUNCTION__);
		return -1;
	}
	for(i=0;hw_mib[i].name[0];i++){
		if(strcmp(name,hw_mib[i].name)==0){
			*offset=hw_mib[i].offset+hw_offset;
			*size=hw_mib[i].size;
			*type=hw_mib[i].type;
			return 0;
		}
	}

	if(strncmp(name,"wlan",4)==0 && name[4]){
		int wlan_idx=name[4]-'0';
		if(wlan_idx>=NUM_WLAN_INTERFACE){
			fprintf(stderr,"invalid wlan idx! max %d\n",NUM_WLAN_INTERFACE-1);
			return -1;
		}
		wlan_offset=((unsigned long)(long *)&(((HW_SETTING_T *)0)->wlan));              
		wlan_offset+=wlan_idx*sizeof(HW_WLAN_SETTING_T);
		name+=6;//wlan0_[realName]

		for(i=0;hw_wlan_mib[i].name[0];i++){
			if(strcmp(name,hw_wlan_mib[i].name)==0){
				*offset=hw_wlan_mib[i].offset+hw_offset+wlan_offset;
				*size=hw_wlan_mib[i].size;
				*type=hw_wlan_mib[i].type;
				return 0;
			}
		}
	}
#ifdef BLUETOOTH_HW_SETTING_SUPPORT
	if(strncmp(name,"bluetooth_",10)==0){   
		name+=10;//bluetooth_[realName]
		for(i=0;bluetooth_hw_mib[i].name[0];i++){
			if(strcmp(name,bluetooth_hw_mib[i].name)==0){
				*offset=bluetooth_hw_mib[i].offset+bluetooth_offset;
				*size=bluetooth_hw_mib[i].size;
				*type=bluetooth_hw_mib[i].type;
				return 0;
			}
		}
	}
#endif
	fprintf(stderr,"can't find the mib %s!\n",name);
	return -1;
}

/* Do checksum and verification for configuration data */
#ifndef WIN32
static inline unsigned char CHECKSUM(unsigned char *data, int len)
#else
__inline unsigned char CHECKSUM(unsigned char *data, int len)
#endif
{
	int i;
	unsigned char sum=0;

	for (i=0; i<len; i++)
		sum += data[i];

	sum = ~sum + 1;
	return sum;
}
#define CHECKSUM_LEN_MAX 0x800000
#ifndef WIN32
static inline int CHECKSUM_OK(unsigned char *data, int len)
#else
__inline int CHECKSUM_OK(unsigned char *data, int len)
#endif
{
	int i;
	unsigned char sum=0;

	if(len<0||len>CHECKSUM_LEN_MAX)
		return 0;
	for (i=0; i<len; i++)
		sum += data[i];

	if (sum == 0)
		return 1;
	else
		return 0;
}

typedef enum { FCC=1, IC, ETSI, SPAIN, FRANCE, MKK } REG_DOMAIN_T;

void rtl_configRps(void);

#if 1 //copy from drivers/net/wireless/rtl8192cd/wifi.h
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
#endif

int checkheaderend;

typedef struct {
	const char *name;
	unsigned char band_2G;
	unsigned char band_5G;
	unsigned short txpwr_lmt_index;
} reg_domain_t;

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
	{ "CA", DOMAIN_FCC,  DOMAIN_FCC,  3  },
	{ "EU", DOMAIN_ETSI, DOMAIN_ETSI, 0  },
	{ "AA", DOMAIN_ETSI, DOMAIN_ETSI, 0  },
	{ "TW", DOMAIN_FCC,  DOMAIN_FCC,  0  },
	{ "SG", DOMAIN_SG,   DOMAIN_SG,   0  },
	{ "CN", DOMAIN_CN,   DOMAIN_CN,   0  },
	{ "KR", DOMAIN_KR,   DOMAIN_KR,   0  },
	{ "JP", DOMAIN_MKK,  DOMAIN_MKK,  3  },
	{ "AU", DOMAIN_AU,   DOMAIN_AU,   4  },
};

#endif/*__REALTEK_H__*/
