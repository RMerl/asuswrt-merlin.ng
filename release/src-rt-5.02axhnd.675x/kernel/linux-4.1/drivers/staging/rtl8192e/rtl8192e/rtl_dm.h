/******************************************************************************
 * Copyright(c) 2008 - 2010 Realtek Corporation. All rights reserved.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 * The full GNU General Public License is included in this distribution in the
 * file called LICENSE.
 *
 * Contact Information:
 * wlanfae <wlanfae@realtek.com>
******************************************************************************/
#ifndef	__R8192UDM_H__
#define __R8192UDM_H__


/*--------------------------Define Parameters-------------------------------*/
#define			OFDM_Table_Length	19
#define		CCK_Table_length	12

#define		DM_DIG_THRESH_HIGH					40
#define		DM_DIG_THRESH_LOW					35

#define		DM_FALSEALARM_THRESH_LOW	40
#define		DM_FALSEALARM_THRESH_HIGH	1000

#define		DM_DIG_HIGH_PWR_THRESH_HIGH		75
#define		DM_DIG_HIGH_PWR_THRESH_LOW		70

#define		BW_AUTO_SWITCH_HIGH_LOW			25
#define		BW_AUTO_SWITCH_LOW_HIGH			30

#define		DM_check_fsync_time_interval				500


#define		DM_DIG_BACKOFF				12
#define		DM_DIG_MAX					0x36
#define		DM_DIG_MIN					0x1c
#define		DM_DIG_MIN_Netcore			0x12

#define		DM_DIG_BACKOFF_MAX			12
#define		DM_DIG_BACKOFF_MIN			-4

#define		RxPathSelection_SS_TH_low		30
#define		RxPathSelection_diff_TH			18

#define		RateAdaptiveTH_High			50
#define		RateAdaptiveTH_Low_20M		30
#define		RateAdaptiveTH_Low_40M		10
#define		VeryLowRSSI					15

#define		CTSToSelfTHVal					35

#define		WAIotTHVal						25

#define		E_FOR_TX_POWER_TRACK	       300
#define		TX_POWER_NEAR_FIELD_THRESH_HIGH		68
#define		TX_POWER_NEAR_FIELD_THRESH_LOW		62
#define	 TX_POWER_ATHEROAP_THRESH_HIGH	   78
#define		TX_POWER_ATHEROAP_THRESH_LOW		72

#define			Current_Tx_Rate_Reg	 0x1e0
#define			Initial_Tx_Rate_Reg	 0x1e1
#define			Tx_Retry_Count_Reg	 0x1ac
#define		RegC38_TH				 20

#define		TX_POWER_NEAR_FIELD_THRESH_LVL2	74
#define		TX_POWER_NEAR_FIELD_THRESH_LVL1	67

#define		TxHighPwrLevel_Normal		0
#define		TxHighPwrLevel_Level1		1
#define		TxHighPwrLevel_Level2		2

#define		DM_Type_ByFW			0
#define		DM_Type_ByDriver		1

/*--------------------------Define Parameters-------------------------------*/


/*------------------------------Define structure----------------------------*/
struct dig_t {
	u8		dig_enable_flag;
	u8		dig_algorithm;
	u8		Dig_TwoPort_Algorithm;
	u8		Dig_Ext_Port_Stage;
	u8		dbg_mode;
	u8		dig_algorithm_switch;

	long		rssi_low_thresh;
	long		rssi_high_thresh;

	u32		FALowThresh;
	u32		FAHighThresh;

	long		rssi_high_power_lowthresh;
	long		rssi_high_power_highthresh;

	u8		dig_state;
	u8		dig_highpwr_state;
	u8		CurSTAConnectState;
	u8		PreSTAConnectState;
	u8		CurAPConnectState;
	u8		PreAPConnectState;

	u8		curpd_thstate;
	u8		prepd_thstate;
	u8		curcs_ratio_state;
	u8		precs_ratio_state;

	u32		pre_ig_value;
	u32		cur_ig_value;

	u8		Backoff_Enable_Flag;
	u8		backoff_val;
	char		BackoffVal_range_max;
	char		BackoffVal_range_min;
	u8		rx_gain_range_max;
	u8		rx_gain_range_min;
	bool		initialgain_lowerbound_state;

	long		rssi_val;
};

enum dm_dig_sta {
	DM_STA_DIG_OFF = 0,
	DM_STA_DIG_ON,
	DM_STA_DIG_MAX
};


enum dm_ratr_sta {
	DM_RATR_STA_HIGH = 0,
	DM_RATR_STA_MIDDLE = 1,
	DM_RATR_STA_LOW = 2,
	DM_RATR_STA_MAX
};

enum dm_dig_op_sta {
	DIG_TYPE_THRESH_HIGH	= 0,
	DIG_TYPE_THRESH_LOW	= 1,
	DIG_TYPE_THRESH_HIGHPWR_HIGH	= 2,
	DIG_TYPE_THRESH_HIGHPWR_LOW	= 3,
	DIG_TYPE_DBG_MODE				= 4,
	DIG_TYPE_RSSI						= 5,
	DIG_TYPE_ALGORITHM				= 6,
	DIG_TYPE_BACKOFF					= 7,
	DIG_TYPE_PWDB_FACTOR			= 8,
	DIG_TYPE_RX_GAIN_MIN				= 9,
	DIG_TYPE_RX_GAIN_MAX				= 10,
	DIG_TYPE_ENABLE			= 20,
	DIG_TYPE_DISABLE		= 30,
	DIG_OP_TYPE_MAX
};

enum dm_dig_alg {
	DIG_ALGO_BY_FALSE_ALARM = 0,
	DIG_ALGO_BY_RSSI	= 1,
	DIG_ALGO_BEFORE_CONNECT_BY_RSSI_AND_ALARM = 2,
	DIG_ALGO_BY_TOW_PORT = 3,
	DIG_ALGO_MAX
};

enum dm_dig_two_port_alg {
	DIG_TWO_PORT_ALGO_RSSI = 0,
	DIG_TWO_PORT_ALGO_FALSE_ALARM = 1,
};


enum dm_dig_ext_port_alg {
	DIG_EXT_PORT_STAGE_0 = 0,
	DIG_EXT_PORT_STAGE_1 = 1,
	DIG_EXT_PORT_STAGE_2 = 2,
	DIG_EXT_PORT_STAGE_3 = 3,
	DIG_EXT_PORT_STAGE_MAX = 4,
};

enum dm_dig_dbg {
	DIG_DBG_OFF = 0,
	DIG_DBG_ON = 1,
	DIG_DBG_MAX
};

enum dm_dig_connect {
	DIG_STA_DISCONNECT = 0,
	DIG_STA_CONNECT = 1,
	DIG_STA_BEFORE_CONNECT = 2,
	DIG_AP_DISCONNECT = 3,
	DIG_AP_CONNECT = 4,
	DIG_AP_ADD_STATION = 5,
	DIG_CONNECT_MAX
};

enum dm_dig_pd_th {
	DIG_PD_AT_LOW_POWER = 0,
	DIG_PD_AT_NORMAL_POWER = 1,
	DIG_PD_AT_HIGH_POWER = 2,
	DIG_PD_MAX
};

enum dm_dig_cs_ratio {
	DIG_CS_RATIO_LOWER = 0,
	DIG_CS_RATIO_HIGHER = 1,
	DIG_CS_MAX
};

struct drx_path_sel {
	u8		Enable;
	u8		DbgMode;
	u8		cck_method;
	u8		cck_Rx_path;

	u8		SS_TH_low;
	u8		diff_TH;
	u8		disabledRF;
	u8		reserved;

	u8		rf_rssi[4];
	u8		rf_enable_rssi_th[4];
	long		cck_pwdb_sta[4];
};

enum dm_cck_rx_path_method {
	CCK_Rx_Version_1 = 0,
	CCK_Rx_Version_2 = 1,
	CCK_Rx_Version_MAX
};


enum dm_dbg {
	DM_DBG_OFF = 0,
	DM_DBG_ON = 1,
	DM_DBG_MAX
};

struct dcmd_txcmd {
	u32	Op;
	u32	Length;
	u32	Value;
};
/*------------------------------Define structure----------------------------*/


/*------------------------Export global variable----------------------------*/
extern	struct dig_t dm_digtable;
extern	u8		dm_shadow[16][256];
extern struct drx_path_sel DM_RxPathSelTable;

extern	u8			test_flag;
/*------------------------Export global variable----------------------------*/


/*--------------------------Exported Function prototype---------------------*/
/*--------------------------Exported Function prototype---------------------*/
extern  void    init_hal_dm(struct net_device *dev);
extern  void deinit_hal_dm(struct net_device *dev);

extern void hal_dm_watchdog(struct net_device *dev);


extern  void    init_rate_adaptive(struct net_device *dev);
extern  void    dm_txpower_trackingcallback(void *data);

extern  void dm_cck_txpower_adjust(struct net_device *dev, bool binch14);

extern  void    dm_restore_dynamic_mechanism_state(struct net_device *dev);
extern  void    dm_backup_dynamic_mechanism_state(struct net_device *dev);
extern  void    dm_change_dynamic_initgain_thresh(struct net_device *dev,
					u32	     dm_type,
					u32	     dm_value);
extern  void    DM_ChangeFsyncSetting(struct net_device *dev,
					s32	     DM_Type,
					s32	     DM_Value);
extern  void dm_force_tx_fw_info(struct net_device *dev,
					u32	     force_type,
					u32	     force_value);
extern  void    dm_init_edca_turbo(struct net_device *dev);
extern  void    dm_rf_operation_test_callback(unsigned long data);
extern  void    dm_rf_pathcheck_workitemcallback(void *data);
extern  void dm_fsync_timer_callback(unsigned long data);
extern  void dm_check_fsync(struct net_device *dev);
extern  void    dm_shadow_init(struct net_device *dev);
extern  void dm_initialize_txpower_tracking(struct net_device *dev);
extern  void    dm_CheckRfCtrlGPIO(void *data);
extern void dm_InitRateAdaptiveMask(struct net_device *dev);
extern	void	init_hal_dm(struct net_device *dev);
extern	void deinit_hal_dm(struct net_device *dev);
extern void hal_dm_watchdog(struct net_device *dev);
extern	void	init_rate_adaptive(struct net_device *dev);
extern	void	dm_txpower_trackingcallback(void *data);
extern	void	dm_restore_dynamic_mechanism_state(struct net_device *dev);
extern	void	dm_backup_dynamic_mechanism_state(struct net_device *dev);
extern	void	dm_change_dynamic_initgain_thresh(struct net_device *dev,
					u32	dm_type,
					u32	dm_value);
extern	void	DM_ChangeFsyncSetting(struct net_device *dev,
					s32		DM_Type,
					s32		DM_Value);
extern	void dm_force_tx_fw_info(struct net_device *dev,
					u32		force_type,
					u32		force_value);
extern	void	dm_init_edca_turbo(struct net_device *dev);
extern	void	dm_rf_operation_test_callback(unsigned long data);
extern	void	dm_rf_pathcheck_workitemcallback(void *data);
extern	void dm_fsync_timer_callback(unsigned long data);
extern	void dm_check_fsync(struct net_device *dev);
extern	void	dm_shadow_init(struct net_device *dev);
extern	void dm_initialize_txpower_tracking(struct net_device *dev);
extern  void    dm_CheckRfCtrlGPIO(void *data);

#endif	/*__R8192UDM_H__ */
