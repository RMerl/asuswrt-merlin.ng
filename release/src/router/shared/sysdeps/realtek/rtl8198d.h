#ifndef __RTL8198D_H__
#define __RTL8198D_H__

#define FLASH_DEVICE_NAME "/dev/mtdblock5"
#define HW_SETTING_OFFSET 0

#define MAX_2G_CHANNEL_NUM_MIB		14
#define MAX_5G_CHANNEL_NUM_MIB		196

#define MAX_CHAN_NUM					14
#define MAX_5G_CHANNEL_NUM			196
#define MAX_5G_DIFF_NUM					14
#define SIGNATURE_LEN			4

#define SPI_PARALLEL_NOR_FLASH_FACTORY_LENGTH	0x80000
#define OFFSET_MTD_FACTORY	0x0

typedef enum
{
	BYTE_T,
	WORD_T,
	DWORD_T,
	INTEGER_T,
	STRING_T,
	BYTE5_T,
	BYTE6_T,
	BYTE13_T,
	IA_T,
	IA6_T,
	BYTE_ARRAY_T,
	WORD_ARRAY_T,
	INT_ARRAY_T,
	BYTE64_T=64,
	OBJECT_T,
} TYPE_T;

typedef struct factory_set_hdr_s {
	uint32_t ih_magic;	/* Image Header Magic Number = 'F', 'T', 'R', 'Y' */
	uint32_t ih_hcrc;	/* Image Header CRC Checksum    */
	uint32_t ih_hdr_ver;	/* Image Header Version Number  */
	uint32_t ih_write_ver;	/* Number of writes             */
	uint32_t ih_dcrc;	/* Image Data CRC Checksum      */
} factory_set_hdr_t;

#define __PACK__				__attribute__ ((packed))

typedef struct hw_config_setting {

	unsigned char txPowerCCK[MAX_CHAN_NUM]; // CCK Tx power for each channel
	unsigned char pwrlevelCCK_A[MAX_CHAN_NUM]; //tx power ofdm 1s path A + B
	unsigned char pwrlevelCCK_B[MAX_CHAN_NUM]; //tx power ofdm 2s path A + B
	unsigned char pwrlevelHT40_1S_A[MAX_CHAN_NUM]; //difference between OFDM and HT40-1S path A
	unsigned char pwrlevelHT40_1S_B[MAX_CHAN_NUM]; //difference between OFDM and HT40-1S path B
	unsigned char pwrdiffHT40_2S[MAX_CHAN_NUM];
	unsigned char pwrdiffHT20[MAX_CHAN_NUM];
	unsigned char pwrdiffOFDM[MAX_CHAN_NUM];
	unsigned char TSSI_enable;
	unsigned char TSSI1;
	unsigned char TSSI2;
	unsigned char ther;
	unsigned char ther2;
	unsigned char ther3;
	unsigned char ther4;
	unsigned char PA_type;
	unsigned char regDomain; // regulation domain
	unsigned char trswpape_c9; // TRSWPAPE C9
	unsigned char trswpape_cc;
	unsigned char trswitch;
	unsigned char kfree_enable;
	unsigned char kfree_enable2;
// this variable for wlan1 8812AR
	unsigned char pwrlevelCCK_A_w1[MAX_CHAN_NUM]; //tx power ofdm 1s path A + B
	unsigned char pwrlevelCCK_B_w1[MAX_CHAN_NUM]; //tx power ofdm 2s path A + B
	unsigned char pwrlevelHT40_1S_A_w1[MAX_CHAN_NUM]; //difference between OFDM and HT40-1S path A
	unsigned char pwrlevelHT40_1S_B_w1[MAX_CHAN_NUM]; //difference between OFDM and HT40-1S path B
	unsigned char pwrdiffHT40_2S_w1[MAX_CHAN_NUM];
	unsigned char pwrdiffHT20_w1[MAX_CHAN_NUM];
	unsigned char pwrdiffOFDM_w1[MAX_CHAN_NUM];
	unsigned char TSSI_enable_w1;
	unsigned char TSSI1_w1;
	unsigned char TSSI2_w1;
	unsigned char ther_w1;
	unsigned char ther2_w1;
	unsigned char ther3_w1;
	unsigned char ther4_w1;
	unsigned char PA_type_w1;
	unsigned char regDomain_w1; // regulation domain
	unsigned char trswpape_c9_w1; // TRSWPAPE C9
	unsigned char trswpape_cc_w1;
	unsigned char trswitch_w1;

	unsigned char pwrlevelCCK_A_w2[MAX_CHAN_NUM]; //tx power ofdm 1s path A + B
	unsigned char pwrlevelCCK_B_w2[MAX_CHAN_NUM]; //tx power ofdm 2s path A + B
	unsigned char pwrlevelHT40_1S_A_w2[MAX_CHAN_NUM]; //difference between OFDM and HT40-1S path A
	unsigned char pwrlevelHT40_1S_B_w2[MAX_CHAN_NUM]; //difference between OFDM and HT40-1S path B
	unsigned char pwrdiffHT40_2S_w2[MAX_CHAN_NUM];
	unsigned char pwrdiffHT20_w2[MAX_CHAN_NUM];
	unsigned char pwrdiffOFDM_w2[MAX_CHAN_NUM];
	unsigned char TSSI_enable_w2;
	unsigned char TSSI1_w2;
	unsigned char TSSI2_w2;
	unsigned char ther_w2;
	unsigned char ther2_w2;
	unsigned char ther3_w2;
	unsigned char ther4_w2;
	unsigned char PA_type_w2;
	unsigned char regDomain_w2; // regulation domain
	unsigned char trswpape_c9_w2; // TRSWPAPE C9
	unsigned char trswpape_cc_w2;
	unsigned char trswitch_w2;

	unsigned char rfType; // RF module type
	unsigned char antDiversity; // rx antenna diversity on/off
	unsigned char txAnt; // select tx antenna, 0 - A, 1 - B
	unsigned char csThreshold;
	unsigned char ccaMode;	// 0, 1, 2
	unsigned char phyType; // for Philip RF module only (0 - analog, 1 - digital)
	unsigned char ledType; // LED type, see LED_TYPE_T for definition
	unsigned char pwrlevel5GHT40_1S_A[MAX_5G_CHANNEL_NUM];
	unsigned char pwrlevel5GHT40_1S_B[MAX_5G_CHANNEL_NUM];
	unsigned char pwrlevel_TSSI5GHT40_1S_A[MAX_5G_CHANNEL_NUM];
	unsigned char pwrlevel_TSSI5GHT40_1S_B[MAX_5G_CHANNEL_NUM];
	unsigned char pwrlevel_TSSI5GHT40_1S_C[MAX_5G_CHANNEL_NUM];
	unsigned char pwrlevel_TSSI5GHT40_1S_D[MAX_5G_CHANNEL_NUM];
	unsigned char pwrlevel_TSSIHT40_1S_A_w1[MAX_CHAN_NUM];
	unsigned char pwrlevel_TSSIHT40_1S_B_w1[MAX_CHAN_NUM];
	unsigned char pwrlevel_TSSIHT40_1S_C_w1[MAX_CHAN_NUM];
	unsigned char pwrlevel_TSSIHT40_1S_D_w1[MAX_CHAN_NUM];
	unsigned char pwrlevel5GHT40_1S_A_w1[MAX_5G_CHANNEL_NUM];
	unsigned char pwrlevel5GHT40_1S_B_w1[MAX_5G_CHANNEL_NUM];
	unsigned char pwrlevel_TSSI5GHT40_1S_A_w1[MAX_5G_CHANNEL_NUM];
	unsigned char pwrlevel_TSSI5GHT40_1S_B_w1[MAX_5G_CHANNEL_NUM];

	unsigned char pwrlevel5GHT40_1S_A_w2[MAX_5G_CHANNEL_NUM];
	unsigned char pwrlevel5GHT40_1S_B_w2[MAX_5G_CHANNEL_NUM];
	unsigned char pwrlevel_TSSI5GHT40_1S_A_w2[MAX_5G_CHANNEL_NUM];
	unsigned char pwrlevel_TSSI5GHT40_1S_B_w2[MAX_5G_CHANNEL_NUM];

	unsigned char pwrlevel_TSSI5GHT40_1S_C_w1[MAX_5G_CHANNEL_NUM];
	unsigned char pwrlevel_TSSI5GHT40_1S_D_w1[MAX_5G_CHANNEL_NUM];

	unsigned char pwrdiff_20BW1S_OFDM1T_A[MAX_CHAN_NUM];
	unsigned char pwrdiff_40BW2S_20BW2S_A[MAX_CHAN_NUM];
	unsigned char pwrdiff_OFDM2T_CCK2T_A[MAX_CHAN_NUM];
	unsigned char pwrdiff_40BW3S_20BW3S_A[MAX_CHAN_NUM];
	unsigned char pwrdiff_4OFDM3T_CCK3T_A[MAX_CHAN_NUM];
	unsigned char pwrdiff_40BW4S_20BW4S_A[MAX_CHAN_NUM];
	unsigned char pwrdiff_OFDM4T_CCK4T_A[MAX_CHAN_NUM];

	unsigned char pwrdiff_5G_20BW1S_OFDM1T_A[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_40BW2S_20BW2S_A[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_40BW3S_20BW3S_A[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_40BW4S_20BW4S_A[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_RSVD_OFDM4T_A[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW1S_160BW1S_A[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW2S_160BW2S_A[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW3S_160BW3S_A[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW4S_160BW4S_A[MAX_5G_DIFF_NUM];

	unsigned char pwrdiff_20BW1S_OFDM1T_B[MAX_CHAN_NUM];
	unsigned char pwrdiff_40BW2S_20BW2S_B[MAX_CHAN_NUM];
	unsigned char pwrdiff_OFDM2T_CCK2T_B[MAX_CHAN_NUM];
	unsigned char pwrdiff_40BW3S_20BW3S_B[MAX_CHAN_NUM];
	unsigned char pwrdiff_OFDM3T_CCK3T_B[MAX_CHAN_NUM];
	unsigned char pwrdiff_40BW4S_20BW4S_B[MAX_CHAN_NUM];
	unsigned char pwrdiff_OFDM4T_CCK4T_B[MAX_CHAN_NUM];

	unsigned char pwrdiff_5G_20BW1S_OFDM1T_B[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_40BW2S_20BW2S_B[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_40BW3S_20BW3S_B[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_40BW4S_20BW4S_B[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_RSVD_OFDM4T_B[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW1S_160BW1S_B[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW2S_160BW2S_B[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW3S_160BW3S_B[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW4S_160BW4S_B[MAX_5G_DIFF_NUM];

	unsigned char rf_xCap;
	unsigned char rf_xCap2;
	unsigned char rf_xCap_w1;
	unsigned char rf_xCap2_w1;
	unsigned char rf_xCap_w2;
	unsigned char rf_xCap2_w2;

	unsigned char target_pwr;
	unsigned char target_pwr_w1;
	unsigned char target_pwr_w2;

	unsigned char pwrlevelCCK_C[MAX_CHAN_NUM]; //tx power ofdm 1s path C + D
	unsigned char pwrlevelCCK_D[MAX_CHAN_NUM]; //tx power ofdm 2s path C + D
	unsigned char pwrlevelHT40_1S_C[MAX_CHAN_NUM]; //difference between OFDM and HT40-1S path C
	unsigned char pwrlevelHT40_1S_D[MAX_CHAN_NUM]; //difference between OFDM and HT40-1S path D
	unsigned char pwrlevel5GHT40_1S_C[MAX_5G_CHANNEL_NUM];
	unsigned char pwrlevel5GHT40_1S_D[MAX_5G_CHANNEL_NUM];
	unsigned char pwrlevelCCK_C_w1[MAX_CHAN_NUM]; //tx power ofdm 1s path C + D
	unsigned char pwrlevelCCK_D_w1[MAX_CHAN_NUM]; //tx power ofdm 2s path C + D
	unsigned char pwrlevelHT40_1S_C_w1[MAX_CHAN_NUM]; //difference between OFDM and HT40-1S path C
	unsigned char pwrlevelHT40_1S_D_w1[MAX_CHAN_NUM]; //difference between OFDM and HT40-1S path D
	unsigned char pwrlevel5GHT40_1S_C_w1[MAX_5G_CHANNEL_NUM];
	unsigned char pwrlevel5GHT40_1S_D_w1[MAX_5G_CHANNEL_NUM];

	unsigned char pwrlevelCCK_C_w2[MAX_CHAN_NUM]; //tx power ofdm 1s path C + D
	unsigned char pwrlevelCCK_D_w2[MAX_CHAN_NUM]; //tx power ofdm 2s path C + D
	unsigned char pwrlevelHT40_1S_C_w2[MAX_CHAN_NUM]; //difference between OFDM and HT40-1S path C
	unsigned char pwrlevelHT40_1S_D_w2[MAX_CHAN_NUM]; //difference between OFDM and HT40-1S path D
	unsigned char pwrlevel5GHT40_1S_C_w2[MAX_5G_CHANNEL_NUM];
	unsigned char pwrlevel5GHT40_1S_D_w2[MAX_5G_CHANNEL_NUM];

//#if defined(CONFIG_WLAN_HAL_8814AE) && (defined(WLAN0_5G_SUPPORT) || defined(WLAN1_5G_SUPPORT))
	unsigned char pwrdiff_20BW1S_OFDM1T_C[MAX_CHAN_NUM];
	unsigned char pwrdiff_40BW2S_20BW2S_C[MAX_CHAN_NUM];
	unsigned char pwrdiff_OFDM2T_CCK2T_C[MAX_CHAN_NUM];
	unsigned char pwrdiff_40BW3S_20BW3S_C[MAX_CHAN_NUM];
	unsigned char pwrdiff_4OFDM3T_CCK3T_C[MAX_CHAN_NUM];
	unsigned char pwrdiff_40BW4S_20BW4S_C[MAX_CHAN_NUM];
	unsigned char pwrdiff_OFDM4T_CCK4T_C[MAX_CHAN_NUM];

	unsigned char pwrdiff_5G_20BW1S_OFDM1T_C[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_40BW2S_20BW2S_C[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_40BW3S_20BW3S_C[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_40BW4S_20BW4S_C[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_RSVD_OFDM4T_C[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW1S_160BW1S_C[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW2S_160BW2S_C[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW3S_160BW3S_C[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW4S_160BW4S_C[MAX_5G_DIFF_NUM];

	unsigned char pwrdiff_20BW1S_OFDM1T_D[MAX_CHAN_NUM];
	unsigned char pwrdiff_40BW2S_20BW2S_D[MAX_CHAN_NUM];
	unsigned char pwrdiff_OFDM2T_CCK2T_D[MAX_CHAN_NUM];
	unsigned char pwrdiff_40BW3S_20BW3S_D[MAX_CHAN_NUM];
	unsigned char pwrdiff_OFDM3T_CCK3T_D[MAX_CHAN_NUM];
	unsigned char pwrdiff_40BW4S_20BW4S_D[MAX_CHAN_NUM];
	unsigned char pwrdiff_OFDM4T_CCK4T_D[MAX_CHAN_NUM];

	unsigned char pwrdiff_5G_20BW1S_OFDM1T_D[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_40BW2S_20BW2S_D[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_40BW3S_20BW3S_D[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_40BW4S_20BW4S_D[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_RSVD_OFDM4T_D[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW1S_160BW1S_D[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW2S_160BW2S_D[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW3S_160BW3S_D[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW4S_160BW4S_D[MAX_5G_DIFF_NUM];

	unsigned char pwrdiff_20BW1S_OFDM1T_A_w1[MAX_CHAN_NUM];
	unsigned char pwrdiff_40BW2S_20BW2S_A_w1[MAX_CHAN_NUM];
	unsigned char pwrdiff_OFDM2T_CCK2T_A_w1[MAX_CHAN_NUM];
	unsigned char pwrdiff_40BW3S_20BW3S_A_w1[MAX_CHAN_NUM];
	unsigned char pwrdiff_4OFDM3T_CCK3T_A_w1[MAX_CHAN_NUM];
	unsigned char pwrdiff_40BW4S_20BW4S_A_w1[MAX_CHAN_NUM];
	unsigned char pwrdiff_OFDM4T_CCK4T_A_w1[MAX_CHAN_NUM];

	unsigned char pwrdiff_5G_20BW1S_OFDM1T_A_w1[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_40BW2S_20BW2S_A_w1[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_40BW3S_20BW3S_A_w1[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_40BW4S_20BW4S_A_w1[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_RSVD_OFDM4T_A_w1[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW1S_160BW1S_A_w1[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW2S_160BW2S_A_w1[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW3S_160BW3S_A_w1[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW4S_160BW4S_A_w1[MAX_5G_DIFF_NUM];

	unsigned char pwrdiff_20BW1S_OFDM1T_B_w1[MAX_CHAN_NUM];
	unsigned char pwrdiff_40BW2S_20BW2S_B_w1[MAX_CHAN_NUM];
	unsigned char pwrdiff_OFDM2T_CCK2T_B_w1[MAX_CHAN_NUM];
	unsigned char pwrdiff_40BW3S_20BW3S_B_w1[MAX_CHAN_NUM];
	unsigned char pwrdiff_OFDM3T_CCK3T_B_w1[MAX_CHAN_NUM];
	unsigned char pwrdiff_40BW4S_20BW4S_B_w1[MAX_CHAN_NUM];
	unsigned char pwrdiff_OFDM4T_CCK4T_B_w1[MAX_CHAN_NUM];

	unsigned char pwrdiff_5G_20BW1S_OFDM1T_B_w1[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_40BW2S_20BW2S_B_w1[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_40BW3S_20BW3S_B_w1[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_40BW4S_20BW4S_B_w1[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_RSVD_OFDM4T_B_w1[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW1S_160BW1S_B_w1[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW2S_160BW2S_B_w1[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW3S_160BW3S_B_w1[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW4S_160BW4S_B_w1[MAX_5G_DIFF_NUM];

	unsigned char pwrdiff_20BW1S_OFDM1T_C_w1[MAX_CHAN_NUM];
	unsigned char pwrdiff_40BW2S_20BW2S_C_w1[MAX_CHAN_NUM];
	unsigned char pwrdiff_OFDM2T_CCK2T_C_w1[MAX_CHAN_NUM];
	unsigned char pwrdiff_40BW3S_20BW3S_C_w1[MAX_CHAN_NUM];
	unsigned char pwrdiff_4OFDM3T_CCK3T_C_w1[MAX_CHAN_NUM];
	unsigned char pwrdiff_40BW4S_20BW4S_C_w1[MAX_CHAN_NUM];
	unsigned char pwrdiff_OFDM4T_CCK4T_C_w1[MAX_CHAN_NUM];

	unsigned char pwrdiff_5G_20BW1S_OFDM1T_C_w1[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_40BW2S_20BW2S_C_w1[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_40BW3S_20BW3S_C_w1[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_40BW4S_20BW4S_C_w1[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_RSVD_OFDM4T_C_w1[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW1S_160BW1S_C_w1[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW2S_160BW2S_C_w1[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW3S_160BW3S_C_w1[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW4S_160BW4S_C_w1[MAX_5G_DIFF_NUM];

	unsigned char pwrdiff_20BW1S_OFDM1T_D_w1[MAX_CHAN_NUM];
	unsigned char pwrdiff_40BW2S_20BW2S_D_w1[MAX_CHAN_NUM];
	unsigned char pwrdiff_OFDM2T_CCK2T_D_w1[MAX_CHAN_NUM];
	unsigned char pwrdiff_40BW3S_20BW3S_D_w1[MAX_CHAN_NUM];
	unsigned char pwrdiff_OFDM3T_CCK3T_D_w1[MAX_CHAN_NUM];
	unsigned char pwrdiff_40BW4S_20BW4S_D_w1[MAX_CHAN_NUM];
	unsigned char pwrdiff_OFDM4T_CCK4T_D_w1[MAX_CHAN_NUM];

	unsigned char pwrdiff_5G_20BW1S_OFDM1T_D_w1[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_40BW2S_20BW2S_D_w1[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_40BW3S_20BW3S_D_w1[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_40BW4S_20BW4S_D_w1[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_RSVD_OFDM4T_D_w1[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW1S_160BW1S_D_w1[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW2S_160BW2S_D_w1[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW3S_160BW3S_D_w1[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW4S_160BW4S_D_w1[MAX_5G_DIFF_NUM];

	unsigned char pwrdiff_20BW1S_OFDM1T_A_w2[MAX_CHAN_NUM];
	unsigned char pwrdiff_40BW2S_20BW2S_A_w2[MAX_CHAN_NUM];
	unsigned char pwrdiff_OFDM2T_CCK2T_A_w2[MAX_CHAN_NUM];
	unsigned char pwrdiff_40BW3S_20BW3S_A_w2[MAX_CHAN_NUM];
	unsigned char pwrdiff_4OFDM3T_CCK3T_A_w2[MAX_CHAN_NUM];
	unsigned char pwrdiff_40BW4S_20BW4S_A_w2[MAX_CHAN_NUM];
	unsigned char pwrdiff_OFDM4T_CCK4T_A_w2[MAX_CHAN_NUM];
	unsigned char pwrdiff_5G_20BW1S_OFDM1T_A_w2[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_40BW2S_20BW2S_A_w2[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_40BW3S_20BW3S_A_w2[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_40BW4S_20BW4S_A_w2[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_RSVD_OFDM4T_A_w2[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW1S_160BW1S_A_w2[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW2S_160BW2S_A_w2[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW3S_160BW3S_A_w2[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW4S_160BW4S_A_w2[MAX_5G_DIFF_NUM];

	unsigned char pwrdiff_20BW1S_OFDM1T_B_w2[MAX_CHAN_NUM];
	unsigned char pwrdiff_40BW2S_20BW2S_B_w2[MAX_CHAN_NUM];
	unsigned char pwrdiff_OFDM2T_CCK2T_B_w2[MAX_CHAN_NUM];
	unsigned char pwrdiff_40BW3S_20BW3S_B_w2[MAX_CHAN_NUM];
	unsigned char pwrdiff_OFDM3T_CCK3T_B_w2[MAX_CHAN_NUM];
	unsigned char pwrdiff_40BW4S_20BW4S_B_w2[MAX_CHAN_NUM];
	unsigned char pwrdiff_OFDM4T_CCK4T_B_w2[MAX_CHAN_NUM];

	unsigned char pwrdiff_5G_20BW1S_OFDM1T_B_w2[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_40BW2S_20BW2S_B_w2[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_40BW3S_20BW3S_B_w2[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_40BW4S_20BW4S_B_w2[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_RSVD_OFDM4T_B_w2[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW1S_160BW1S_B_w2[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW2S_160BW2S_B_w2[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW3S_160BW3S_B_w2[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW4S_160BW4S_B_w2[MAX_5G_DIFF_NUM];

	unsigned char pwrdiff_20BW1S_OFDM1T_C_w2[MAX_CHAN_NUM];
	unsigned char pwrdiff_40BW2S_20BW2S_C_w2[MAX_CHAN_NUM];
	unsigned char pwrdiff_OFDM2T_CCK2T_C_w2[MAX_CHAN_NUM];
	unsigned char pwrdiff_40BW3S_20BW3S_C_w2[MAX_CHAN_NUM];
	unsigned char pwrdiff_4OFDM3T_CCK3T_C_w2[MAX_CHAN_NUM];
	unsigned char pwrdiff_40BW4S_20BW4S_C_w2[MAX_CHAN_NUM];
	unsigned char pwrdiff_OFDM4T_CCK4T_C_w2[MAX_CHAN_NUM];

	unsigned char pwrdiff_5G_20BW1S_OFDM1T_C_w2[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_40BW2S_20BW2S_C_w2[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_40BW3S_20BW3S_C_w2[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_40BW4S_20BW4S_C_w2[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_RSVD_OFDM4T_C_w2[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW1S_160BW1S_C_w2[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW2S_160BW2S_C_w2[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW3S_160BW3S_C_w2[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW4S_160BW4S_C_w2[MAX_5G_DIFF_NUM];

	unsigned char pwrdiff_20BW1S_OFDM1T_D_w2[MAX_CHAN_NUM];
	unsigned char pwrdiff_40BW2S_20BW2S_D_w2[MAX_CHAN_NUM];
	unsigned char pwrdiff_OFDM2T_CCK2T_D_w2[MAX_CHAN_NUM];
	unsigned char pwrdiff_40BW3S_20BW3S_D_w2[MAX_CHAN_NUM];
	unsigned char pwrdiff_OFDM3T_CCK3T_D_w2[MAX_CHAN_NUM];
	unsigned char pwrdiff_40BW4S_20BW4S_D_w2[MAX_CHAN_NUM];
	unsigned char pwrdiff_OFDM4T_CCK4T_D_w2[MAX_CHAN_NUM];

	unsigned char pwrdiff_5G_20BW1S_OFDM1T_D_w2[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_40BW2S_20BW2S_D_w2[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_40BW3S_20BW3S_D_w2[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_40BW4S_20BW4S_D_w2[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_RSVD_OFDM4T_D_w2[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW1S_160BW1S_D_w2[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW2S_160BW2S_D_w2[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW3S_160BW3S_D_w2[MAX_5G_DIFF_NUM];
	unsigned char pwrdiff_5G_80BW4S_160BW4S_D_w2[MAX_5G_DIFF_NUM];

	unsigned char  bDPPathAOK;
	unsigned char bDPPathBOK;
	unsigned char pwsf_2g_a[3];
	unsigned char pwsf_2g_b[3];
	unsigned int lut_2g_even_a0[64];
	unsigned int lut_2g_even_a1[64];
	unsigned int lut_2g_even_a2[64];
	unsigned int lut_2g_odd_a0[64];
	unsigned int lut_2g_odd_a1[64];
	unsigned int lut_2g_odd_a2[64];
	unsigned int lut_2g_even_b0[64];
	unsigned int lut_2g_even_b1[64];
	unsigned int lut_2g_even_b2[64];
	unsigned int lut_2g_odd_b0[64];
	unsigned int lut_2g_odd_b1[64];
	unsigned int lut_2g_odd_b2[64];

	unsigned char bDPPathCOK;
	unsigned char bDPPathDOK;
	unsigned char pwsf_2g_c[3];
	unsigned char pwsf_2g_d[3];
	unsigned int lut_2g_even_c0[64];
	unsigned int lut_2g_even_c1[64];
	unsigned int lut_2g_even_c2[64];
	unsigned int lut_2g_odd_c0[64];
	unsigned int lut_2g_odd_c1[64];
	unsigned int lut_2g_odd_c2[64];
	unsigned int lut_2g_even_d0[64];
	unsigned int lut_2g_even_d1[64];
	unsigned int lut_2g_even_d2[64];
	unsigned int lut_2g_odd_d0[64];
	unsigned int lut_2g_odd_d1[64];
	unsigned int lut_2g_odd_d2[64];

	unsigned char vendor_spec_info[4];
} __PACK__ HW_WLAN_SETTING_T, *HW_WLAN_SETTING_Tp;

typedef struct hw_setting {
	factory_set_hdr_t checksum;
	unsigned char bootVer[5];
	unsigned char hwid[4];
	unsigned char hwver[8];
	unsigned char hwbow[32];
	unsigned char datecode[8];
	HW_WLAN_SETTING_T wlan;
	unsigned char nic0Addr[6];
	unsigned char nic1Addr[6];
	unsigned char nic2Addr[6];
	unsigned char countryCode[6];
	unsigned char territoryCode[8];
	unsigned char modelName[16];
	unsigned char pinCode[10];
	unsigned char amas_bdl;
	unsigned char psk[15];
} __PACK__ HW_SETTING_T, *HW_SETTING_Tp;

/* Config file header */
typedef struct param_header {
	unsigned char signature[SIGNATURE_LEN] __PACK__;  // Tag + version
	unsigned short len __PACK__;
} PARAM_HEADER_T, *PARAM_HEADER_Tp;

extern int setup8812Wlan(int band);
extern int setup8197Wlan();
extern int setupWlanDPK_2G();

extern const char WIF_2G[];
extern const char WIF_5G[];
extern const char WIF_5G2[];
extern const char VXD_5G2[];
extern const char VXD_5G[];
extern const char VXD_2G[];

#define FIELD_OFFSET(type, field)	((unsigned long)(long *)&(((type *)0)->field))
#define _OFFSET_HW(field)			(OFFSET_MTD_FACTORY + (int)FIELD_OFFSET(HW_SETTING_T,field))
#define _SIZE_HW_ASUS(field)		sizeof(((HW_SETTING_T *)0)->field)
#define _OFFSET_HW_WLAN(field)		(_OFFSET_HW(wlan) + (int)FIELD_OFFSET(HW_WLAN_SETTING_T,field))
#define _SIZE_HW_WLAN(field)		sizeof(((HW_WLAN_SETTING_T *)0)->field)
//#define _OFFSET_HW_ASUS(field)		((int)FIELD_OFFSET(HW_SETTING_T,field))
//#define _SIZE_HW_ASUS(field)		sizeof(((HW_SETTING_T *)0)->field)

#define OFFSET_MAC_ADDR_2G			_OFFSET_HW(nic0Addr)
#define OFFSET_MAC_ADDR_5G			_OFFSET_HW(nic1Addr)
#define OFFSET_MAC_ADDR_5G_2		_OFFSET_HW(nic2Addr)
#define OFFSET_COUNTRY_CODE			_OFFSET_HW(countryCode)
#define OFFSET_TERRITORY_CODE		_OFFSET_HW(territoryCode)
#define OFFSET_ODMPID				_OFFSET_HW(modelName)
#define OFFSET_PIN_CODE				_OFFSET_HW(pinCode)
#define OFFSET_BOOT_VER				_OFFSET_HW(bootVer)
#define OFFSET_HWID				_OFFSET_HW(hwid)
#define OFFSET_HW_VERSION			_OFFSET_HW(hwver)
#define OFFSET_HW_BOM				_OFFSET_HW(hwbow)
#define OFFSET_HW_DATE_CODE			_OFFSET_HW(datecode)
#define OFFSET_PSK			_OFFSET_HW(psk)

#define rtklog(fmt,args...) do {} while(0)
#define rtk_printf(fmt,args...) do {} while(0)
#define rtkerr printf
#define rtkinfo printf

// MIB value, id mapping table
typedef struct _mib_table_entry {
	char name[48];
	TYPE_T type;
	int offset;
	int size;
	//const char *defaultValue;
} mib_table_entry_T;

extern mib_table_entry_T mib_table[];
#define MIB_TBL_SIZE sizeof(mib_table)/sizeof(mib_table_entry_T)
#define MIB_TBL_ENTRY(NAME, TYPE, OFFSET, SIZE) {NAME, TYPE, OFFSET, SIZE}
extern int flash_get_mib_info(char* name, int *offset, int *size, TYPE_T *type);
#endif
