/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 ******************************************************************************/


#ifndef	__HALDMOUTSRC_H__
#define __HALDMOUTSRC_H__

/*  */
/*  Definition */
/*  */
/*  */
/*  2011/09/22 MH Define all team supprt ability. */
/*  */

/*  */
/*  2011/09/22 MH Define for all teams. Please Define the constan in your precomp header. */
/*  */
/* define		DM_ODM_SUPPORT_AP			0 */
/* define		DM_ODM_SUPPORT_ADSL			0 */
/* define		DM_ODM_SUPPORT_CE			0 */
/* define		DM_ODM_SUPPORT_MP			1 */

#define	TP_MODE		0
#define	RSSI_MODE		1
#define	TRAFFIC_LOW	0
#define	TRAFFIC_HIGH	1


/*  */
/* 3 Tx Power Tracking */
/* 3============================================================ */
#define		DPK_DELTA_MAPPING_NUM	13
#define		index_mapping_HP_NUM	15


/*  */
/* 3 PSD Handler */
/* 3============================================================ */

#define	AFH_PSD		1	/* 0:normal PSD scan, 1: only do 20 pts PSD */
#define	MODE_40M		0	/* 0:20M, 1:40M */
#define	PSD_TH2		3
#define	PSD_CHMIN		20   /*  Minimum channel number for BT AFH */
#define	SIR_STEP_SIZE	3
#define   Smooth_Size_1		5
#define	Smooth_TH_1	3
#define   Smooth_Size_2		10
#define	Smooth_TH_2	4
#define   Smooth_Size_3		20
#define	Smooth_TH_3	4
#define   Smooth_Step_Size 5
#define	Adaptive_SIR	1
#define	PSD_RESCAN		4
#define	PSD_SCAN_INTERVAL	700 /* ms */

/* 8723A High Power IGI Setting */
#define DM_DIG_HIGH_PWR_IGI_LOWER_BOUND	0x22
#define DM_DIG_Gmode_HIGH_PWR_IGI_LOWER_BOUND 0x28
#define DM_DIG_HIGH_PWR_THRESHOLD	0x3a

/*  LPS define */
#define DM_DIG_FA_TH0_LPS				4 /*  4 in lps */
#define DM_DIG_FA_TH1_LPS				15 /*  15 lps */
#define DM_DIG_FA_TH2_LPS				30 /*  30 lps */
#define RSSI_OFFSET_DIG					0x05;

/* ANT Test */
#define			ANTTESTALL		0x00		/* Ant A or B will be Testing */
#define		ANTTESTA		0x01		/* Ant A will be Testing */
#define		ANTTESTB		0x02		/* Ant B will be testing */


/*  */
/*  structure and define */
/*  */

struct  dig_t {
	u8		Dig_Enable_Flag;
	u8		Dig_Ext_Port_Stage;

	int		RssiLowThresh;
	int		RssiHighThresh;

	u32		FALowThresh;
	u32		FAHighThresh;

	u8		CurSTAConnectState;
	u8		PreSTAConnectState;
	u8		CurMultiSTAConnectState;

	u8		PreIGValue;
	u8		CurIGValue;
	u8		BackupIGValue;

	s8		BackoffVal;
	s8		BackoffVal_range_max;
	s8		BackoffVal_range_min;
	u8		rx_gain_range_max;
	u8		rx_gain_range_min;
	u8		Rssi_val_min;

	u8		PreCCK_CCAThres;
	u8		CurCCK_CCAThres;
	u8		PreCCKPDState;
	u8		CurCCKPDState;

	u8		LargeFAHit;
	u8		ForbiddenIGI;
	u32		Recover_cnt;

	u8		DIG_Dynamic_MIN_0;
	u8		DIG_Dynamic_MIN_1;
	bool		bMediaConnect_0;
	bool		bMediaConnect_1;

	u32		RSSI_max;
};

struct dynamic_pwr_sav {
	u8		PreCCAState;
	u8		CurCCAState;

	u8		PreRFState;
	u8		CurRFState;

	int		    Rssi_val_min;

	u8		initialize;
	u32		Reg874, RegC70, Reg85C, RegA74;
};

struct false_alarm_stats {
	u32	Cnt_Parity_Fail;
	u32	Cnt_Rate_Illegal;
	u32	Cnt_Crc8_fail;
	u32	Cnt_Mcs_fail;
	u32	Cnt_Ofdm_fail;
	u32	Cnt_Cck_fail;
	u32	Cnt_all;
	u32	Cnt_Fast_Fsync;
	u32	Cnt_SB_Search_fail;
	u32	Cnt_OFDM_CCA;
	u32	Cnt_CCK_CCA;
	u32	Cnt_CCA_all;
	u32	Cnt_BW_USC;	/* Gary */
	u32	Cnt_BW_LSC;	/* Gary */
};

#define ASSOCIATE_ENTRY_NUM					32 /*  Max size of AsocEntry[]. */
#define	ODM_ASSOCIATE_ENTRY_NUM				ASSOCIATE_ENTRY_NUM

/*  This indicates two different the steps. */
/*  In SWAW_STEP_PEAK, driver needs to switch antenna and listen to the signal on the air. */
/*  In SWAW_STEP_DETERMINE, driver just compares the signal captured in SWAW_STEP_PEAK */
/*  with original RSSI to determine if it is necessary to switch antenna. */
#define SWAW_STEP_PEAK		0
#define SWAW_STEP_DETERMINE	1

#define	TP_MODE		0
#define	RSSI_MODE		1
#define	TRAFFIC_LOW	0
#define	TRAFFIC_HIGH	1

struct sw_ant_sw {
	u8		try_flag;
	s32		PreRSSI;
	u8		CurAntenna;
	u8		PreAntenna;
	u8		RSSI_Trying;
	u8		TestMode;
	u8		bTriggerAntennaSwitch;
	u8		SelectAntennaMap;
	u8		RSSI_target;

	/*  Before link Antenna Switch check */
	u8		SWAS_NoLink_State;
	u32		SWAS_NoLink_BK_Reg860;
	bool		ANTA_ON;	/* To indicate Ant A is or not */
	bool		ANTB_ON;	/* To indicate Ant B is on or not */

	s32		RSSI_sum_A;
	s32		RSSI_sum_B;
	s32		RSSI_cnt_A;
	s32		RSSI_cnt_B;

	u64		lastTxOkCnt;
	u64		lastRxOkCnt;
	u64		TXByteCnt_A;
	u64		TXByteCnt_B;
	u64		RXByteCnt_A;
	u64		RXByteCnt_B;
	u8		TrafficLoad;
};

struct edca_turbo {
	bool bCurrentTurboEDCA;
	u32	prv_traffic_idx; /*  edca turbo */
};

struct odm_rate_adapt {
	u8	Type;		/*  DM_Type_ByFW/DM_Type_ByDriver */
	u8	HighRSSIThresh;	/*  if RSSI > HighRSSIThresh	=> RATRState is DM_RATR_STA_HIGH */
	u8	LowRSSIThresh;	/*  if RSSI <= LowRSSIThresh	=> RATRState is DM_RATR_STA_LOW */
	u8	RATRState;	/*  Current RSSI level, DM_RATR_STA_HIGH/DM_RATR_STA_MIDDLE/DM_RATR_STA_LOW */
	u32	LastRATR;	/*  RATR Register Content */
};

#define IQK_MAC_REG_NUM		4
#define IQK_ADDA_REG_NUM		16
#define IQK_BB_REG_NUM_MAX	10
#define IQK_BB_REG_NUM		9
#define HP_THERMAL_NUM		8

#define AVG_THERMAL_NUM		8
#define IQK_Matrix_REG_NUM	8
#define IQK_Matrix_Settings_NUM	1+24+21

#define		DM_Type_ByFW			0
#define		DM_Type_ByDriver		1

/*  Declare for common info */

struct odm_phy_dbg_info {
	/* ODM Write,debug info */
	s8		RxSNRdB[RF_PATH_MAX];
	u64		NumQryPhyStatus;
	u64		NumQryPhyStatusCCK;
	u64		NumQryPhyStatusOFDM;
	/* Others */
	s32		RxEVM[RF_PATH_MAX];

};

struct odm_packet_info {
	u8		Rate;
	u8		StationID;
	bool		bPacketMatchBSSID;
	bool		bPacketToSelf;
	bool		bPacketBeacon;
};


enum {
	/*  BB Team */
	ODM_DIG			= 0x00000001,
	ODM_HIGH_POWER		= 0x00000002,
	ODM_CCK_CCA_TH		= 0x00000004,
	ODM_FA_STATISTICS	= 0x00000008,
	ODM_RAMASK		= 0x00000010,
	ODM_RSSI_MONITOR	= 0x00000020,
	ODM_SW_ANTDIV		= 0x00000040,
	ODM_HW_ANTDIV		= 0x00000080,
	ODM_BB_PWRSV		= 0x00000100,
	ODM_2TPATHDIV		= 0x00000200,
	ODM_1TPATHDIV		= 0x00000400,
	ODM_PSD2AFH		= 0x00000800
};

/*  */
/*  2011/10/20 MH Define Common info enum for all team. */
/*  */

enum odm_cmninfo {
	/*  Fixed value: */
	/*  */

	ODM_CMNINFO_MP_TEST_CHIP = 2,
	ODM_CMNINFO_IC_TYPE,			/*  enum odm_ic_type_def */
	ODM_CMNINFO_CUT_VER,			/*  enum odm_cut_version */
	ODM_CMNINFO_FAB_VER,			/*  enum odm_fab_version */
	ODM_CMNINFO_BOARD_TYPE,			/*  enum odm_board_type */
	ODM_CMNINFO_EXT_LNA,			/*  true */
	ODM_CMNINFO_EXT_PA,
	ODM_CMNINFO_EXT_TRSW,
	ODM_CMNINFO_BINHCT_TEST,
	ODM_CMNINFO_BWIFI_TEST,
	ODM_CMNINFO_SMART_CONCURRENT,


	/*  */
	/*  Dynamic value: */
	/*  */
	ODM_CMNINFO_MP_MODE,

	ODM_CMNINFO_WIFI_DIRECT,
	ODM_CMNINFO_WIFI_DISPLAY,
	ODM_CMNINFO_LINK,
	ODM_CMNINFO_RSSI_MIN,
	ODM_CMNINFO_DBG_COMP,				/*  u64 */
	ODM_CMNINFO_DBG_LEVEL,				/*  u32 */
	ODM_CMNINFO_RA_THRESHOLD_HIGH,		/*  u8 */
	ODM_CMNINFO_RA_THRESHOLD_LOW,		/*  u8 */
	ODM_CMNINFO_RF_ANTENNA_TYPE,		/*  u8 */
	ODM_CMNINFO_BT_DISABLED,
	ODM_CMNINFO_BT_OPERATION,
	ODM_CMNINFO_BT_DIG,
	ODM_CMNINFO_BT_BUSY,					/* Check Bt is using or not */
	ODM_CMNINFO_BT_DISABLE_EDCA,

	/*  */
	/*  Dynamic ptr array hook itms. */
	/*  */
	ODM_CMNINFO_STA_STATUS,
	ODM_CMNINFO_PHY_STATUS,
	ODM_CMNINFO_MAC_STATUS,

	ODM_CMNINFO_MAX,
};

/*  Define ODM support ability.  ODM_CMNINFO_ABILITY */
enum {
	/*  BB ODM section BIT 0-15 */
	ODM_BB_ANT_DIV				= BIT(6),
};

/*	ODM_CMNINFO_INTERFACE */
enum odm_interface_def {
	ODM_ITRF_PCIE	=	0x1,
	ODM_ITRF_USB	=	0x2,
	ODM_ITRF_SDIO	=	0x4,
	ODM_ITRF_ALL	=	0x7,
};

/*  ODM_CMNINFO_IC_TYPE */
enum odm_ic_type_def {
	ODM_RTL8192S	=	BIT(0),
	ODM_RTL8192C	=	BIT(1),
	ODM_RTL8192D	=	BIT(2),
	ODM_RTL8723A	=	BIT(3),
	ODM_RTL8188E	=	BIT(4),
	ODM_RTL8812	=	BIT(5),
	ODM_RTL8821	=	BIT(6),
};

/* ODM_CMNINFO_CUT_VER */
enum odm_cut_version {
	ODM_CUT_A		=	1,
	ODM_CUT_B		=	2,
	ODM_CUT_C		=	3,
	ODM_CUT_D		=	4,
	ODM_CUT_E		=	5,
	ODM_CUT_F		=	6,
	ODM_CUT_TEST		=	7,
};

/*  ODM_CMNINFO_FAB_VER */
enum odm_fab_version {
	ODM_TSMC	=	0,
	ODM_UMC		=	1,
};

/*  For example 1T2R (A+AB = BIT0|BIT4|BIT5) */
enum rf_path_def {
	ODM_RF_TX_A	=	BIT(0),
	ODM_RF_TX_B	=	BIT(1),
	ODM_RF_TX_C	=	BIT(2),
	ODM_RF_TX_D	=	BIT(3),
	ODM_RF_RX_A	=	BIT(4),
	ODM_RF_RX_B	=	BIT(5),
	ODM_RF_RX_C	=	BIT(6),
	ODM_RF_RX_D	=	BIT(7),
};

/*  ODM Dynamic common info value definition */

enum odm_mac_phy_mode {
	ODM_SMSP	= 0,
	ODM_DMSP	= 1,
	ODM_DMDP	= 2,
};


enum odm_bt_coexist {
	ODM_BT_BUSY		= 1,
	ODM_BT_ON		= 2,
	ODM_BT_OFF		= 3,
	ODM_BT_NONE		= 4,
};

/*  ODM_CMNINFO_OP_MODE */
enum odm_operation_mode {
	ODM_NO_LINK		= BIT(0),
	ODM_LINK		= BIT(1),
	ODM_SCAN		= BIT(2),
	ODM_POWERSAVE		= BIT(3),
	ODM_AP_MODE		= BIT(4),
	ODM_CLIENT_MODE		= BIT(5),
	ODM_AD_HOC		= BIT(6),
	ODM_WIFI_DIRECT		= BIT(7),
	ODM_WIFI_DISPLAY	= BIT(8),
};

/*  ODM_CMNINFO_WM_MODE */
enum odm_wireless_mode {
	ODM_WM_UNKNOW		= 0x0,
	ODM_WM_B		= BIT(0),
	ODM_WM_G		= BIT(1),
	ODM_WM_A		= BIT(2),
	ODM_WM_N24G		= BIT(3),
	ODM_WM_N5G		= BIT(4),
	ODM_WM_AUTO		= BIT(5),
	ODM_WM_AC		= BIT(6),
};

/*  ODM_CMNINFO_BAND */
enum odm_band_type {
	ODM_BAND_2_4G		= BIT(0),
	ODM_BAND_5G		= BIT(1),

};

/*  ODM_CMNINFO_SEC_CHNL_OFFSET */
enum odm_sec_chnl_offset {
	ODM_DONT_CARE		= 0,
	ODM_BELOW		= 1,
	ODM_ABOVE		= 2
};

/*  ODM_CMNINFO_CHNL */

/*  ODM_CMNINFO_BOARD_TYPE */
enum odm_board_type {
	ODM_BOARD_NORMAL	= 0,
	ODM_BOARD_HIGHPWR	= 1,
	ODM_BOARD_MINICARD	= 2,
	ODM_BOARD_SLIM		= 3,
	ODM_BOARD_COMBO		= 4,

};

/*  ODM_CMNINFO_ONE_PATH_CCA */
enum odm_cca_path {
	ODM_CCA_2R			= 0,
	ODM_CCA_1R_A			= 1,
	ODM_CCA_1R_B			= 2,
};

struct iqk_matrix_regs_set {
	bool	bIQKDone;
	s32	Value[1][IQK_Matrix_REG_NUM];
};

struct odm_rf_cal_t {
	/* for tx power tracking */

	u32	RegA24; /*  for TempCCK */
	s32	RegE94;
	s32	RegE9C;
	s32	RegEB4;
	s32	RegEBC;

	/* u8 bTXPowerTracking; */
	u8		TXPowercount;
	bool bTXPowerTrackingInit;
	bool bTXPowerTracking;
	u8		TxPowerTrackControl; /* for mp mode, turn off txpwrtracking as default */
	u8		TM_Trigger;
	u8		InternalPA5G[2];	/* pathA / pathB */

	u8		ThermalMeter[2];    /*  ThermalMeter, index 0 for RFIC0, and 1 for RFIC1 */
	u8		ThermalValue;
	u8		ThermalValue_LCK;
	u8		ThermalValue_IQK;
	u8	ThermalValue_DPK;
	u8	ThermalValue_AVG[AVG_THERMAL_NUM];
	u8	ThermalValue_AVG_index;
	u8	ThermalValue_RxGain;
	u8	ThermalValue_Crystal;
	u8	ThermalValue_DPKstore;
	u8	ThermalValue_DPKtrack;
	bool	TxPowerTrackingInProgress;
	bool	bDPKenable;

	bool	bReloadtxpowerindex;
	u8	bRfPiEnable;
	u32	TXPowerTrackingCallbackCnt; /* cosa add for debug */

	u8	bCCKinCH14;
	u8	CCK_index;
	u8	OFDM_index[2];
	bool bDoneTxpower;

	u8	ThermalValue_HP[HP_THERMAL_NUM];
	u8	ThermalValue_HP_index;
	struct iqk_matrix_regs_set IQKMatrixRegSetting[IQK_Matrix_Settings_NUM];

	u8	Delta_IQK;
	u8	Delta_LCK;

	/* for IQK */
	u32	RegC04;
	u32	Reg874;
	u32	RegC08;
	u32	RegB68;
	u32	RegB6C;
	u32	Reg870;
	u32	Reg860;
	u32	Reg864;

	bool	bIQKInitialized;
	bool bLCKInProgress;
	bool	bAntennaDetected;
	u32	ADDA_backup[IQK_ADDA_REG_NUM];
	u32	IQK_MAC_backup[IQK_MAC_REG_NUM];
	u32	IQK_BB_backup_recover[9];
	u32	IQK_BB_backup[IQK_BB_REG_NUM];

	/* for APK */
	u32	APKoutput[2][2]; /* path A/B; output1_1a/output1_2a */
	u8	bAPKdone;
	u8	bAPKThermalMeterIgnore;
	u8	bDPdone;
	u8	bDPPathAOK;
	u8	bDPPathBOK;
};

enum ant_dif_type {
	NO_ANTDIV			= 0xFF,
	CG_TRX_HW_ANTDIV		= 0x01,
	CGCS_RX_HW_ANTDIV		= 0x02,
	FIXED_HW_ANTDIV			= 0x03,
	CG_TRX_SMART_ANTDIV		= 0x04,
	CGCS_RX_SW_ANTDIV		= 0x05,
};

/*  2011/09/22 MH Copy from SD4 defined structure. We use to support PHY DM integration. */
struct dm_odm_t {
	/*  */
	/*	Add for different team use temporarily */
	/*  */
	struct rtw_adapter	*Adapter;		/*  For CE/NIC team */

	u64			DebugComponents;
	u32			DebugLevel;

/*  ODM HANDLE, DRIVER NEEDS NOT TO HOOK------ */
	bool			bCckHighPower;
	u8			RFPathRxEnable;		/*  ODM_CMNINFO_RFPATH_ENABLE */
/*  ODM HANDLE, DRIVER NEEDS NOT TO HOOK------ */

/* 1  COMMON INFORMATION */

	/*  Init Value */
/* HOOK BEFORE REG INIT----------- */
	/*  ODM Support Ability DIG/RATR/TX_PWR_TRACK/ �K�K = 1/2/3/�K */
	u32			SupportAbility;
	/*  ODM composite or independent. Bit oriented/ 92C+92D+ .... or any other type = 1/2/3/... */
	u32			SupportICType;
	/*  Cut Version TestChip/A-cut/B-cut... = 0/1/2/3/... */
	u8			CutVersion;
	/*  Fab Version TSMC/UMC = 0/1 */
	u8			FabVersion;
	/*  Board Type Normal/HighPower/MiniCard/SLIM/Combo/... = 0/1/2/3/4/... */
	u8			BoardType;
	/*  with external LNA  NO/Yes = 0/1 */
	u8			ExtLNA;
	/*  with external PA  NO/Yes = 0/1 */
	u8			ExtPA;
	/*  with external TRSW  NO/Yes = 0/1 */
	u8			ExtTRSW;
	bool			bInHctTest;
	bool			bWIFITest;

	bool			bDualMacSmartConcurrent;
	u32			BK_SupportAbility;
/* HOOK BEFORE REG INIT----------- */

	/*  */
	/*  Dynamic Value */
	/*  */
/*  POINTER REFERENCE----------- */

	u8			u8_temp;
	bool			bool_temp;
	struct rtw_adapter	*PADAPTER_temp;

/*  POINTER REFERENCE----------- */
	/*  */
/* CALL BY VALUE------------- */
	bool			bWIFI_Direct;
	bool			bWIFI_Display;
	bool			bLinked;
	u8			RSSI_Min;
	u8			InterfaceIndex; /*  Add for 92D  dual MAC: 0--Mac0 1--Mac1 */
	bool		bIsMPChip;
	bool			bOneEntryOnly;
	/*  Common info for BTDM */
	bool			bBtDisabled;			/*  BT is disabled */
	bool			bBtHsOperation;		/*  BT HS mode is under progress */
	u8			btHsDigVal;			/*  use BT rssi to decide the DIG value */
	bool			bBtDisableEdcaTurbo;	/*  Under some condition, don't enable the EDCA Turbo */
	bool			bBtBusy;			/*  BT is busy. */
/* CALL BY VALUE------------- */

	/* 2 Define STA info. */
	/*  _ODM_STA_INFO */
	/*  2012/01/12 MH For MP, we need to reduce one array pointer for default port.?? */
	struct sta_info *		pODM_StaInfo[ODM_ASSOCIATE_ENTRY_NUM];

	/*  Latest packet phy info (ODM write) */
	struct odm_phy_dbg_info	 PhyDbgInfo;
	/* PHY_INFO_88E		PhyInfo; */

	/*  Latest packet phy info (ODM write) */
	/* MAC_INFO_88E		MacInfo; */

	/*  Different Team independt structure?? */

	/*  */
	/* TX_RTP_CMN		TX_retrpo; */
	/* TX_RTP_88E		TX_retrpo; */
	/* TX_RTP_8195		TX_retrpo; */

	/*  */
	/* ODM Structure */
	/*  */
	struct dig_t	DM_DigTable;
	struct dynamic_pwr_sav		DM_PSTable;
	struct false_alarm_stats	FalseAlmCnt;
	struct false_alarm_stats	FlaseAlmCntBuddyAdapter;
	struct sw_ant_sw		DM_SWAT_Table;

	struct edca_turbo		DM_EDCA_Table;
	u32		WMMEDCA_BE;
	/*  Copy from SD4 structure */
	/*  */
	/*  ================================================== */
	/*  */

	/* PSD */
	u8			RSSI_BT;		/* come from BT */
	struct odm_rate_adapt	RateAdaptive;


	struct odm_rf_cal_t	RFCalibrateInfo;
};	/*  DM_Dynamic_Mechanism_Structure */

enum odm_rf_content {
	odm_radioa_txt = 0x1000,
	odm_radiob_txt = 0x1001,
	odm_radioc_txt = 0x1002,
	odm_radiod_txt = 0x1003
};

/*  Status code */
enum rt_status {
	RT_STATUS_SUCCESS,
	RT_STATUS_FAILURE,
	RT_STATUS_PENDING,
	RT_STATUS_RESOURCE,
	RT_STATUS_INVALID_CONTEXT,
	RT_STATUS_INVALID_PARAMETER,
	RT_STATUS_NOT_SUPPORT,
	RT_STATUS_OS_API_FAILED,
};

/* include "odm_function.h" */

/* 3=========================================================== */
/* 3 DIG */
/* 3=========================================================== */

enum dm_dig_op {
	DIG_TYPE_THRESH_HIGH	= 0,
	DIG_TYPE_THRESH_LOW	= 1,
	DIG_TYPE_BACKOFF		= 2,
	DIG_TYPE_RX_GAIN_MIN	= 3,
	DIG_TYPE_RX_GAIN_MAX	= 4,
	DIG_TYPE_ENABLE			= 5,
	DIG_TYPE_DISABLE		= 6,
	DIG_OP_TYPE_MAX
};

#define		DM_DIG_THRESH_HIGH			40
#define		DM_DIG_THRESH_LOW			35

#define		DM_SCAN_RSSI_TH				0x14 /* scan return issue for LC */


#define		DM_FALSEALARM_THRESH_LOW	400
#define		DM_FALSEALARM_THRESH_HIGH	1000

#define		DM_DIG_MAX_NIC				0x4e
#define		DM_DIG_MIN_NIC				0x1e

#define		DM_DIG_MAX_AP				0x32
#define		DM_DIG_MIN_AP				0x20

#define		DM_DIG_MAX_NIC_HP			0x46
#define		DM_DIG_MIN_NIC_HP			0x2e

#define		DM_DIG_MAX_AP_HP				0x42
#define		DM_DIG_MIN_AP_HP				0x30

/* vivi 92c&92d has different definition, 20110504 */
/* this is for 92c */
#define		DM_DIG_FA_TH0				0x200
#define		DM_DIG_FA_TH1				0x300
#define		DM_DIG_FA_TH2				0x400
/* this is for 92d */
#define		DM_DIG_FA_TH0_92D			0x100
#define		DM_DIG_FA_TH1_92D			0x400
#define		DM_DIG_FA_TH2_92D			0x600

#define		DM_DIG_BACKOFF_MAX			12
#define		DM_DIG_BACKOFF_MIN			-4
#define		DM_DIG_BACKOFF_DEFAULT		10

/* 3=========================================================== */
/* 3 AGC RX High Power Mode */
/* 3=========================================================== */
#define          LNA_Low_Gain_1                      0x64
#define          LNA_Low_Gain_2                      0x5A
#define          LNA_Low_Gain_3                      0x58

#define          FA_RXHP_TH1                           5000
#define          FA_RXHP_TH2                           1500
#define          FA_RXHP_TH3                             800
#define          FA_RXHP_TH4                             600
#define          FA_RXHP_TH5                             500

/* 3=========================================================== */
/* 3 EDCA */
/* 3=========================================================== */

/* 3=========================================================== */
/* 3 Dynamic Tx Power */
/* 3=========================================================== */
/* Dynamic Tx Power Control Threshold */
#define		TX_POWER_NEAR_FIELD_THRESH_LVL2	74
#define		TX_POWER_NEAR_FIELD_THRESH_LVL1	67
#define		TX_POWER_NEAR_FIELD_THRESH_AP		0x3F

#define		TxHighPwrLevel_Normal		0
#define		TxHighPwrLevel_Level1		1
#define		TxHighPwrLevel_Level2		2
#define		TxHighPwrLevel_BT1			3
#define		TxHighPwrLevel_BT2			4
#define		TxHighPwrLevel_15			5
#define		TxHighPwrLevel_35			6
#define		TxHighPwrLevel_50			7
#define		TxHighPwrLevel_70			8
#define		TxHighPwrLevel_100			9

/* 3=========================================================== */
/* 3 Rate Adaptive */
/* 3=========================================================== */
#define		DM_RATR_STA_INIT			0
#define		DM_RATR_STA_HIGH			1
#define			DM_RATR_STA_MIDDLE		2
#define			DM_RATR_STA_LOW			3

/* 3=========================================================== */
/* 3 BB Power Save */
/* 3=========================================================== */


enum dm_1r_cca {
	CCA_1R =0,
	CCA_2R = 1,
	CCA_MAX = 2,
};

enum dm_rf_def {
	RF_Save =0,
	RF_Normal = 1,
	RF_MAX = 2,
};

/* 3=========================================================== */
/* 3 Antenna Diversity */
/* 3=========================================================== */
enum dm_swas {
	Antenna_A = 1,
	Antenna_B = 2,
	Antenna_MAX = 3,
};

/*  Maximal number of antenna detection mechanism needs to perform, added by Roger, 2011.12.28. */
#define	MAX_ANTENNA_DETECTION_CNT	10

/*  */
/*  Extern Global Variables. */
/*  */
#define	OFDM_TABLE_SIZE_92C	37
#define	OFDM_TABLE_SIZE_92D	43
#define	CCK_TABLE_SIZE		33

extern	u32 OFDMSwingTable23A[OFDM_TABLE_SIZE_92D];
extern	u8 CCKSwingTable_Ch1_Ch1323A[CCK_TABLE_SIZE][8];
extern	u8 CCKSwingTable_Ch1423A [CCK_TABLE_SIZE][8];



/*  20100514 Joseph: Add definition for antenna switching test after link. */
/*  This indicates two different the steps. */
/*  In SWAW_STEP_PEAK, driver needs to switch antenna and listen to the signal on the air. */
/*  In SWAW_STEP_DETERMINE, driver just compares the signal captured in SWAW_STEP_PEAK */
/*  with original RSSI to determine if it is necessary to switch antenna. */
#define SWAW_STEP_PEAK		0
#define SWAW_STEP_DETERMINE	1

struct hal_data_8723a;

void ODM_Write_DIG23a(struct dm_odm_t *pDM_Odm,	u8	CurrentIGI);
void ODM_Write_CCK_CCA_Thres23a(struct dm_odm_t *pDM_Odm, u8	CurCCK_CCAThres);

void ODM_SetAntenna(struct dm_odm_t *pDM_Odm, u8 Antenna);


#define dm_RF_Saving	ODM_RF_Saving23a
void ODM_RF_Saving23a(struct dm_odm_t *pDM_Odm, u8 bForceInNormal);

#define dm_CheckTXPowerTracking		ODM_TXPowerTrackingCheck23a
void ODM_TXPowerTrackingCheck23a(struct dm_odm_t *pDM_Odm);

bool ODM_RAStateCheck23a(struct dm_odm_t *pDM_Odm, s32 RSSI, bool bForceUpdate,
		      u8 *pRATRState);


u32 ConvertTo_dB23a(u32 Value);

u32 GetPSDData(struct dm_odm_t *pDM_Odm, unsigned int point, u8 initial_gain_psd);

void odm_DIG23abyRSSI_LPS(struct dm_odm_t *pDM_Odm);

u32 ODM_Get_Rate_Bitmap23a(struct hal_data_8723a *pHalData, u32 macid, u32 ra_mask, u8 rssi_level);


void ODM23a_DMInit(struct dm_odm_t *pDM_Odm);

void ODM_DMWatchdog23a(struct rtw_adapter *adapter);

void ODM_CmnInfoInit23a(struct dm_odm_t *pDM_Odm, enum odm_cmninfo	CmnInfo, u32 Value);

void ODM_CmnInfoPtrArrayHook23a(struct dm_odm_t *pDM_Odm, enum odm_cmninfo	CmnInfo, u16 Index, void *pValue);

void ODM_CmnInfoUpdate23a(struct dm_odm_t *pDM_Odm, u32 CmnInfo, u64 Value);

void ODM_ResetIQKResult(struct dm_odm_t *pDM_Odm);

void ODM_AntselStatistics_88C(struct dm_odm_t *pDM_Odm, u8 MacId, u32 PWDBAll, bool isCCKrate);

void ODM_SingleDualAntennaDefaultSetting(struct dm_odm_t *pDM_Odm);

bool ODM_SingleDualAntennaDetection(struct dm_odm_t *pDM_Odm, u8 mode);

#endif
