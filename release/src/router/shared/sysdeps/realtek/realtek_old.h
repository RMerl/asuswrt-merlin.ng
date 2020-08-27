#ifndef __REALTEK_OLD_H__
#define __REALTEK_OLD_H__
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
//#ifdef RTK_DEBUG
//#define rtklog cprintf
//#define rtk_printf cprintf
//#else
//#define rtklog(fmt,args...) do {} while(0)
//#define rtk_printf(fmt,args...) do {} while(0)
//#endif
//#define rtkerr cprintf
//#define rtkinfo cprintf
/*wlan related*/


//typedef enum wlan_band{WLAN_5G=0,WLAN_2G=1}wlan_band;

typedef enum {
	BYTE_T,
	STRING_T,
	BYTE_ARRAY_T,
	WLAN_T
} MIB_TYPE_T;

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
    unsigned char amas_bdl __PACK__;
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



void rtl_configRps(void);


int checkheaderend;


#ifdef RTCONFIG_AMAS
void wait_connection_finished(char *ifname);
#endif
#endif/*__REALTEK_H__*/
