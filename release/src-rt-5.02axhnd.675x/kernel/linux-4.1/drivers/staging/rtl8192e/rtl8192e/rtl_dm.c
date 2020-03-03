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
#include "rtl_core.h"
#include "rtl_dm.h"
#include "r8192E_hw.h"
#include "r8192E_phy.h"
#include "r8192E_phyreg.h"
#include "r8190P_rtl8256.h"
#include "r8192E_cmdpkt.h"

/*---------------------------Define Local Constant---------------------------*/
static u32 edca_setting_DL[HT_IOT_PEER_MAX] = {
	0x5e4322,
	0x5e4322,
	0x5ea44f,
	0x5e4322,
	0x604322,
	0xa44f,
	0x5e4322,
	0x5e4332
};

static u32 edca_setting_DL_GMode[HT_IOT_PEER_MAX] = {
	0x5e4322,
	0x5e4322,
	0x5e4322,
	0x5e4322,
	0x604322,
	0xa44f,
	0x5e4322,
	0x5e4322
};

static u32 edca_setting_UL[HT_IOT_PEER_MAX] = {
	0x5e4322,
	0xa44f,
	0x5ea44f,
	0x5e4322,
	0x604322,
	0x5e4322,
	0x5e4322,
	0x5e4332
};

#define RTK_UL_EDCA 0xa44f
#define RTK_DL_EDCA 0x5e4322
/*---------------------------Define Local Constant---------------------------*/


/*------------------------Define global variable-----------------------------*/
struct dig_t dm_digtable;
u8 dm_shadow[16][256] = {
	{0}
};

struct drx_path_sel DM_RxPathSelTable;
/*------------------------Define global variable-----------------------------*/


/*------------------------Define local variable------------------------------*/
/*------------------------Define local variable------------------------------*/



/*---------------------Define local function prototype-----------------------*/
static	void	dm_check_rate_adaptive(struct net_device *dev);

static	void	dm_init_bandwidth_autoswitch(struct net_device *dev);
static	void	dm_bandwidth_autoswitch(struct net_device *dev);


static	void	dm_check_txpower_tracking(struct net_device *dev);





static	void	dm_bb_initialgain_restore(struct net_device *dev);


static	void	dm_bb_initialgain_backup(struct net_device *dev);

static	void dm_dig_init(struct net_device *dev);
static	void dm_ctrl_initgain_byrssi(struct net_device *dev);
static	void dm_ctrl_initgain_byrssi_highpwr(struct net_device *dev);
static	void dm_ctrl_initgain_byrssi_by_driverrssi(struct net_device *dev);
static	void dm_ctrl_initgain_byrssi_by_fwfalse_alarm(struct net_device *dev);
static	void dm_initial_gain(struct net_device *dev);
static	void dm_pd_th(struct net_device *dev);
static	void dm_cs_ratio(struct net_device *dev);

static	void dm_init_ctstoself(struct net_device *dev);
static	void dm_Init_WA_Broadcom_IOT(struct net_device *dev);

static	void	dm_check_edca_turbo(struct net_device *dev);

static	void dm_check_pbc_gpio(struct net_device *dev);


static	void dm_check_rx_path_selection(struct net_device *dev);
static	void dm_init_rxpath_selection(struct net_device *dev);
static	void dm_rxpath_sel_byrssi(struct net_device *dev);


static void dm_init_fsync(struct net_device *dev);
static void dm_deInit_fsync(struct net_device *dev);

static	void dm_check_txrateandretrycount(struct net_device *dev);
static  void dm_check_ac_dc_power(struct net_device *dev);

/*---------------------Define local function prototype-----------------------*/

static	void	dm_init_dynamic_txpower(struct net_device *dev);
static	void	dm_dynamic_txpower(struct net_device *dev);


static	void dm_send_rssi_tofw(struct net_device *dev);
static	void	dm_ctstoself(struct net_device *dev);
/*---------------------------Define function prototype------------------------*/

void init_hal_dm(struct net_device *dev)
{
	struct r8192_priv *priv = rtllib_priv(dev);

	priv->DM_Type = DM_Type_ByDriver;

	priv->undecorated_smoothed_pwdb = -1;

	dm_init_dynamic_txpower(dev);

	init_rate_adaptive(dev);

	dm_dig_init(dev);
	dm_init_edca_turbo(dev);
	dm_init_bandwidth_autoswitch(dev);
	dm_init_fsync(dev);
	dm_init_rxpath_selection(dev);
	dm_init_ctstoself(dev);
	if (IS_HARDWARE_TYPE_8192SE(dev))
		dm_Init_WA_Broadcom_IOT(dev);

	INIT_DELAYED_WORK_RSL(&priv->gpio_change_rf_wq, (void *)dm_CheckRfCtrlGPIO, dev);
}

void deinit_hal_dm(struct net_device *dev)
{

	dm_deInit_fsync(dev);

}

void hal_dm_watchdog(struct net_device *dev)
{
	struct r8192_priv *priv = rtllib_priv(dev);

	if (priv->being_init_adapter)
		return;

	dm_check_ac_dc_power(dev);

	dm_check_pbc_gpio(dev);
	dm_check_txrateandretrycount(dev);
	dm_check_edca_turbo(dev);

	dm_check_rate_adaptive(dev);
	dm_dynamic_txpower(dev);
	dm_check_txpower_tracking(dev);

	dm_ctrl_initgain_byrssi(dev);
	dm_bandwidth_autoswitch(dev);

	dm_check_rx_path_selection(dev);
	dm_check_fsync(dev);

	dm_send_rssi_tofw(dev);
	dm_ctstoself(dev);
}

static void dm_check_ac_dc_power(struct net_device *dev)
{
	struct r8192_priv *priv = rtllib_priv(dev);
	static char *ac_dc_check_script_path = "/etc/acpi/wireless-rtl-ac-dc-power.sh";
	char *argv[] = {ac_dc_check_script_path, DRV_NAME, NULL};
	static char *envp[] = {"HOME=/",
			"TERM=linux",
			"PATH=/usr/bin:/bin",
			 NULL};

	if (priv->ResetProgress == RESET_TYPE_SILENT) {
		RT_TRACE((COMP_INIT | COMP_POWER | COMP_RF),
			 "GPIOChangeRFWorkItemCallBack(): Silent Reset!!!!!!!\n");
		return;
	}

	if (priv->rtllib->state != RTLLIB_LINKED)
		return;
	call_usermodehelper(ac_dc_check_script_path, argv, envp, UMH_WAIT_PROC);

	return;
};


void init_rate_adaptive(struct net_device *dev)
{

	struct r8192_priv *priv = rtllib_priv(dev);
	struct rate_adaptive *pra = (struct rate_adaptive *)&priv->rate_adaptive;

	pra->ratr_state = DM_RATR_STA_MAX;
	pra->high2low_rssi_thresh_for_ra = RateAdaptiveTH_High;
	pra->low2high_rssi_thresh_for_ra20M = RateAdaptiveTH_Low_20M+5;
	pra->low2high_rssi_thresh_for_ra40M = RateAdaptiveTH_Low_40M+5;

	pra->high_rssi_thresh_for_ra = RateAdaptiveTH_High+5;
	pra->low_rssi_thresh_for_ra20M = RateAdaptiveTH_Low_20M;
	pra->low_rssi_thresh_for_ra40M = RateAdaptiveTH_Low_40M;

	if (priv->CustomerID == RT_CID_819x_Netcore)
		pra->ping_rssi_enable = 1;
	else
		pra->ping_rssi_enable = 0;
	pra->ping_rssi_thresh_for_ra = 15;


	if (priv->rf_type == RF_2T4R) {
		pra->upper_rssi_threshold_ratr		=	0x8f0f0000;
		pra->middle_rssi_threshold_ratr		=	0x8f0ff000;
		pra->low_rssi_threshold_ratr		=	0x8f0ff001;
		pra->low_rssi_threshold_ratr_40M	=	0x8f0ff005;
		pra->low_rssi_threshold_ratr_20M	=	0x8f0ff001;
		pra->ping_rssi_ratr	=	0x0000000d;
	} else if (priv->rf_type == RF_1T2R) {
		pra->upper_rssi_threshold_ratr		=	0x000fc000;
		pra->middle_rssi_threshold_ratr		=	0x000ff000;
		pra->low_rssi_threshold_ratr		=	0x000ff001;
		pra->low_rssi_threshold_ratr_40M	=	0x000ff005;
		pra->low_rssi_threshold_ratr_20M	=	0x000ff001;
		pra->ping_rssi_ratr	=	0x0000000d;
	}

}


static void dm_check_rate_adaptive(struct net_device *dev)
{
	struct r8192_priv *priv = rtllib_priv(dev);
	struct rt_hi_throughput *pHTInfo = priv->rtllib->pHTInfo;
	struct rate_adaptive *pra = (struct rate_adaptive *)&priv->rate_adaptive;
	u32 currentRATR, targetRATR = 0;
	u32 LowRSSIThreshForRA = 0, HighRSSIThreshForRA = 0;
	bool bshort_gi_enabled = false;
	static u8 ping_rssi_state;

	if (!priv->up) {
		RT_TRACE(COMP_RATE, "<---- dm_check_rate_adaptive(): driver is going to unload\n");
		return;
	}

	if (pra->rate_adaptive_disabled)
		return;

	if (!(priv->rtllib->mode == WIRELESS_MODE_N_24G ||
	    priv->rtllib->mode == WIRELESS_MODE_N_5G))
		return;

	if (priv->rtllib->state == RTLLIB_LINKED) {

		bshort_gi_enabled = (pHTInfo->bCurTxBW40MHz && pHTInfo->bCurShortGI40MHz) ||
			(!pHTInfo->bCurTxBW40MHz && pHTInfo->bCurShortGI20MHz);


		pra->upper_rssi_threshold_ratr =
				(pra->upper_rssi_threshold_ratr & (~BIT31)) | ((bshort_gi_enabled) ? BIT31 : 0);

		pra->middle_rssi_threshold_ratr =
				(pra->middle_rssi_threshold_ratr & (~BIT31)) | ((bshort_gi_enabled) ? BIT31 : 0);

		if (priv->CurrentChannelBW != HT_CHANNEL_WIDTH_20) {
			pra->low_rssi_threshold_ratr =
				(pra->low_rssi_threshold_ratr_40M & (~BIT31)) | ((bshort_gi_enabled) ? BIT31 : 0);
		} else {
			pra->low_rssi_threshold_ratr =
			(pra->low_rssi_threshold_ratr_20M & (~BIT31)) | ((bshort_gi_enabled) ? BIT31 : 0);
		}
		pra->ping_rssi_ratr =
				(pra->ping_rssi_ratr & (~BIT31)) | ((bshort_gi_enabled) ? BIT31 : 0);

		if (pra->ratr_state == DM_RATR_STA_HIGH) {
			HighRSSIThreshForRA	= pra->high2low_rssi_thresh_for_ra;
			LowRSSIThreshForRA	= (priv->CurrentChannelBW != HT_CHANNEL_WIDTH_20) ?
					(pra->low_rssi_thresh_for_ra40M) : (pra->low_rssi_thresh_for_ra20M);
		} else if (pra->ratr_state == DM_RATR_STA_LOW) {
			HighRSSIThreshForRA	= pra->high_rssi_thresh_for_ra;
			LowRSSIThreshForRA	= (priv->CurrentChannelBW != HT_CHANNEL_WIDTH_20) ?
					(pra->low2high_rssi_thresh_for_ra40M) : (pra->low2high_rssi_thresh_for_ra20M);
		} else {
			HighRSSIThreshForRA	= pra->high_rssi_thresh_for_ra;
			LowRSSIThreshForRA	= (priv->CurrentChannelBW != HT_CHANNEL_WIDTH_20) ?
					(pra->low_rssi_thresh_for_ra40M) : (pra->low_rssi_thresh_for_ra20M);
		}

		if (priv->undecorated_smoothed_pwdb >= (long)HighRSSIThreshForRA) {
			pra->ratr_state = DM_RATR_STA_HIGH;
			targetRATR = pra->upper_rssi_threshold_ratr;
		} else if (priv->undecorated_smoothed_pwdb >= (long)LowRSSIThreshForRA) {
			pra->ratr_state = DM_RATR_STA_MIDDLE;
			targetRATR = pra->middle_rssi_threshold_ratr;
		} else {
			pra->ratr_state = DM_RATR_STA_LOW;
			targetRATR = pra->low_rssi_threshold_ratr;
		}

		if (pra->ping_rssi_enable) {
			if (priv->undecorated_smoothed_pwdb < (long)(pra->ping_rssi_thresh_for_ra+5)) {
				if ((priv->undecorated_smoothed_pwdb < (long)pra->ping_rssi_thresh_for_ra) ||
				    ping_rssi_state) {
					pra->ratr_state = DM_RATR_STA_LOW;
					targetRATR = pra->ping_rssi_ratr;
					ping_rssi_state = 1;
				}
			} else {
				ping_rssi_state = 0;
			}
		}

		if (priv->rtllib->GetHalfNmodeSupportByAPsHandler(dev))
			targetRATR &=  0xf00fffff;

		currentRATR = read_nic_dword(dev, RATR0);
		if (targetRATR !=  currentRATR) {
			u32 ratr_value;

			ratr_value = targetRATR;
			RT_TRACE(COMP_RATE,
				 "currentRATR = %x, targetRATR = %x\n",
				 currentRATR, targetRATR);
			if (priv->rf_type == RF_1T2R)
				ratr_value &= ~(RATE_ALL_OFDM_2SS);
			write_nic_dword(dev, RATR0, ratr_value);
			write_nic_byte(dev, UFWP, 1);

			pra->last_ratr = targetRATR;
		}

	} else {
		pra->ratr_state = DM_RATR_STA_MAX;
	}
}

static void dm_init_bandwidth_autoswitch(struct net_device *dev)
{
	struct r8192_priv *priv = rtllib_priv(dev);

	priv->rtllib->bandwidth_auto_switch.threshold_20Mhzto40Mhz = BW_AUTO_SWITCH_LOW_HIGH;
	priv->rtllib->bandwidth_auto_switch.threshold_40Mhzto20Mhz = BW_AUTO_SWITCH_HIGH_LOW;
	priv->rtllib->bandwidth_auto_switch.bforced_tx20Mhz = false;
	priv->rtllib->bandwidth_auto_switch.bautoswitch_enable = false;
}

static void dm_bandwidth_autoswitch(struct net_device *dev)
{
	struct r8192_priv *priv = rtllib_priv(dev);

	if (priv->CurrentChannelBW == HT_CHANNEL_WIDTH_20 ||
	   !priv->rtllib->bandwidth_auto_switch.bautoswitch_enable)
		return;
	if (priv->rtllib->bandwidth_auto_switch.bforced_tx20Mhz == false) {
		if (priv->undecorated_smoothed_pwdb <=
		    priv->rtllib->bandwidth_auto_switch.threshold_40Mhzto20Mhz)
			priv->rtllib->bandwidth_auto_switch.bforced_tx20Mhz = true;
	} else {
		if (priv->undecorated_smoothed_pwdb >=
		    priv->rtllib->bandwidth_auto_switch.threshold_20Mhzto40Mhz)
			priv->rtllib->bandwidth_auto_switch.bforced_tx20Mhz = false;
	}
}

static u32 OFDMSwingTable[OFDM_Table_Length] = {
	0x7f8001fe,
	0x71c001c7,
	0x65400195,
	0x5a400169,
	0x50800142,
	0x47c0011f,
	0x40000100,
	0x390000e4,
	0x32c000cb,
	0x2d4000b5,
	0x288000a2,
	0x24000090,
	0x20000080,
	0x1c800072,
	0x19800066,
	0x26c0005b,
	0x24400051,
	0x12000048,
	0x10000040
};

static u8	CCKSwingTable_Ch1_Ch13[CCK_Table_length][8] = {
	{0x36, 0x35, 0x2e, 0x25, 0x1c, 0x12, 0x09, 0x04},
	{0x30, 0x2f, 0x29, 0x21, 0x19, 0x10, 0x08, 0x03},
	{0x2b, 0x2a, 0x25, 0x1e, 0x16, 0x0e, 0x07, 0x03},
	{0x26, 0x25, 0x21, 0x1b, 0x14, 0x0d, 0x06, 0x03},
	{0x22, 0x21, 0x1d, 0x18, 0x11, 0x0b, 0x06, 0x02},
	{0x1f, 0x1e, 0x1a, 0x15, 0x10, 0x0a, 0x05, 0x02},
	{0x1b, 0x1a, 0x17, 0x13, 0x0e, 0x09, 0x04, 0x02},
	{0x18, 0x17, 0x15, 0x11, 0x0c, 0x08, 0x04, 0x02},
	{0x16, 0x15, 0x12, 0x0f, 0x0b, 0x07, 0x04, 0x01},
	{0x13, 0x13, 0x10, 0x0d, 0x0a, 0x06, 0x03, 0x01},
	{0x11, 0x11, 0x0f, 0x0c, 0x09, 0x06, 0x03, 0x01},
	{0x0f, 0x0f, 0x0d, 0x0b, 0x08, 0x05, 0x03, 0x01}
};

static u8	CCKSwingTable_Ch14[CCK_Table_length][8] = {
	{0x36, 0x35, 0x2e, 0x1b, 0x00, 0x00, 0x00, 0x00},
	{0x30, 0x2f, 0x29, 0x18, 0x00, 0x00, 0x00, 0x00},
	{0x2b, 0x2a, 0x25, 0x15, 0x00, 0x00, 0x00, 0x00},
	{0x26, 0x25, 0x21, 0x13, 0x00, 0x00, 0x00, 0x00},
	{0x22, 0x21, 0x1d, 0x11, 0x00, 0x00, 0x00, 0x00},
	{0x1f, 0x1e, 0x1a, 0x0f, 0x00, 0x00, 0x00, 0x00},
	{0x1b, 0x1a, 0x17, 0x0e, 0x00, 0x00, 0x00, 0x00},
	{0x18, 0x17, 0x15, 0x0c, 0x00, 0x00, 0x00, 0x00},
	{0x16, 0x15, 0x12, 0x0b, 0x00, 0x00, 0x00, 0x00},
	{0x13, 0x13, 0x10, 0x0a, 0x00, 0x00, 0x00, 0x00},
	{0x11, 0x11, 0x0f, 0x09, 0x00, 0x00, 0x00, 0x00},
	{0x0f, 0x0f, 0x0d, 0x08, 0x00, 0x00, 0x00, 0x00}
};

#define		Pw_Track_Flag				0x11d
#define		Tssi_Mea_Value				0x13c
#define		Tssi_Report_Value1			0x134
#define		Tssi_Report_Value2			0x13e
#define		FW_Busy_Flag				0x13f

static void dm_TXPowerTrackingCallback_TSSI(struct net_device *dev)
{
	struct r8192_priv *priv = rtllib_priv(dev);
	bool	bHighpowerstate, viviflag = false;
	struct dcmd_txcmd tx_cmd;
	u8	powerlevelOFDM24G;
	int	i = 0, j = 0, k = 0;
	u8	RF_Type, tmp_report[5] = {0, 0, 0, 0, 0};
	u32	Value;
	u8	Pwr_Flag;
	u16	Avg_TSSI_Meas, TSSI_13dBm, Avg_TSSI_Meas_from_driver = 0;
	u32	delta = 0;

	RT_TRACE(COMP_POWER_TRACKING, "%s()\n", __func__);
	write_nic_byte(dev, Pw_Track_Flag, 0);
	write_nic_byte(dev, FW_Busy_Flag, 0);
	priv->rtllib->bdynamic_txpower_enable = false;
	bHighpowerstate = priv->bDynamicTxHighPower;

	powerlevelOFDM24G = (u8)(priv->Pwr_Track>>24);
	RF_Type = priv->rf_type;
	Value = (RF_Type<<8) | powerlevelOFDM24G;

	RT_TRACE(COMP_POWER_TRACKING, "powerlevelOFDM24G = %x\n",
		 powerlevelOFDM24G);


	for (j = 0; j <= 30; j++) {

		tx_cmd.Op		= TXCMD_SET_TX_PWR_TRACKING;
		tx_cmd.Length	= 4;
		tx_cmd.Value		= Value;
		cmpk_message_handle_tx(dev, (u8 *)&tx_cmd,
				       DESC_PACKET_TYPE_INIT,
				       sizeof(struct dcmd_txcmd));
		mdelay(1);
		for (i = 0; i <= 30; i++) {
			Pwr_Flag = read_nic_byte(dev, Pw_Track_Flag);

			if (Pwr_Flag == 0) {
				mdelay(1);

				if (priv->bResetInProgress) {
					RT_TRACE(COMP_POWER_TRACKING,
						 "we are in silent reset progress, so return\n");
					write_nic_byte(dev, Pw_Track_Flag, 0);
					write_nic_byte(dev, FW_Busy_Flag, 0);
					return;
				}
				if (priv->rtllib->eRFPowerState != eRfOn) {
					RT_TRACE(COMP_POWER_TRACKING,
						 "we are in power save, so return\n");
					write_nic_byte(dev, Pw_Track_Flag, 0);
					write_nic_byte(dev, FW_Busy_Flag, 0);
					return;
				}

				continue;
			}

			Avg_TSSI_Meas = read_nic_word(dev, Tssi_Mea_Value);

			if (Avg_TSSI_Meas == 0) {
				write_nic_byte(dev, Pw_Track_Flag, 0);
				write_nic_byte(dev, FW_Busy_Flag, 0);
				return;
			}

			for (k = 0; k < 5; k++) {
				if (k != 4)
					tmp_report[k] = read_nic_byte(dev,
							 Tssi_Report_Value1+k);
				else
					tmp_report[k] = read_nic_byte(dev,
							 Tssi_Report_Value2);

				RT_TRACE(COMP_POWER_TRACKING,
					 "TSSI_report_value = %d\n",
					 tmp_report[k]);

			       if (tmp_report[k] <= 20) {
					viviflag = true;
					break;
				}
			}

			if (viviflag) {
				write_nic_byte(dev, Pw_Track_Flag, 0);
				viviflag = false;
				RT_TRACE(COMP_POWER_TRACKING, "we filted this data\n");
				for (k = 0; k < 5; k++)
					tmp_report[k] = 0;
				break;
			}

			for (k = 0; k < 5; k++)
				Avg_TSSI_Meas_from_driver += tmp_report[k];

			Avg_TSSI_Meas_from_driver = Avg_TSSI_Meas_from_driver*100/5;
			RT_TRACE(COMP_POWER_TRACKING,
				 "Avg_TSSI_Meas_from_driver = %d\n",
				 Avg_TSSI_Meas_from_driver);
			TSSI_13dBm = priv->TSSI_13dBm;
			RT_TRACE(COMP_POWER_TRACKING, "TSSI_13dBm = %d\n", TSSI_13dBm);

			if (Avg_TSSI_Meas_from_driver > TSSI_13dBm)
				delta = Avg_TSSI_Meas_from_driver - TSSI_13dBm;
			else
				delta = TSSI_13dBm - Avg_TSSI_Meas_from_driver;

			if (delta <= E_FOR_TX_POWER_TRACK) {
				priv->rtllib->bdynamic_txpower_enable = true;
				write_nic_byte(dev, Pw_Track_Flag, 0);
				write_nic_byte(dev, FW_Busy_Flag, 0);
				RT_TRACE(COMP_POWER_TRACKING,
					 "tx power track is done\n");
				RT_TRACE(COMP_POWER_TRACKING,
					 "priv->rfa_txpowertrackingindex = %d\n",
					 priv->rfa_txpowertrackingindex);
				RT_TRACE(COMP_POWER_TRACKING,
					 "priv->rfa_txpowertrackingindex_real = %d\n",
					 priv->rfa_txpowertrackingindex_real);
				RT_TRACE(COMP_POWER_TRACKING,
					 "priv->CCKPresentAttentuation_difference = %d\n",
					 priv->CCKPresentAttentuation_difference);
				RT_TRACE(COMP_POWER_TRACKING,
					 "priv->CCKPresentAttentuation = %d\n",
					 priv->CCKPresentAttentuation);
				return;
			}
			if (Avg_TSSI_Meas_from_driver < TSSI_13dBm - E_FOR_TX_POWER_TRACK) {
				if (RF_Type == RF_2T4R) {

					if ((priv->rfa_txpowertrackingindex > 0) &&
					    (priv->rfc_txpowertrackingindex > 0)) {
						priv->rfa_txpowertrackingindex--;
						if (priv->rfa_txpowertrackingindex_real > 4) {
							priv->rfa_txpowertrackingindex_real--;
							rtl8192_setBBreg(dev,
								 rOFDM0_XATxIQImbalance,
								 bMaskDWord,
								 priv->txbbgain_table[priv->rfa_txpowertrackingindex_real].txbbgain_value);
						}

						priv->rfc_txpowertrackingindex--;
						if (priv->rfc_txpowertrackingindex_real > 4) {
							priv->rfc_txpowertrackingindex_real--;
							rtl8192_setBBreg(dev,
								 rOFDM0_XCTxIQImbalance,
								 bMaskDWord,
								 priv->txbbgain_table[priv->rfc_txpowertrackingindex_real].txbbgain_value);
						}
					} else {
						rtl8192_setBBreg(dev, rOFDM0_XATxIQImbalance,
								 bMaskDWord,
								 priv->txbbgain_table[4].txbbgain_value);
						rtl8192_setBBreg(dev,
								 rOFDM0_XCTxIQImbalance,
								 bMaskDWord, priv->txbbgain_table[4].txbbgain_value);
					}
				} else {
					if (priv->rfa_txpowertrackingindex > 0) {
						priv->rfa_txpowertrackingindex--;
						if (priv->rfa_txpowertrackingindex_real > 4) {
							priv->rfa_txpowertrackingindex_real--;
							rtl8192_setBBreg(dev,
									 rOFDM0_XATxIQImbalance,
									 bMaskDWord,
									 priv->txbbgain_table[priv->rfa_txpowertrackingindex_real].txbbgain_value);
						}
					} else
						rtl8192_setBBreg(dev, rOFDM0_XATxIQImbalance,
								 bMaskDWord, priv->txbbgain_table[4].txbbgain_value);

				}
			} else {
				if (RF_Type == RF_2T4R) {
					if ((priv->rfa_txpowertrackingindex <
					    TxBBGainTableLength - 1) &&
					    (priv->rfc_txpowertrackingindex <
					    TxBBGainTableLength - 1)) {
						priv->rfa_txpowertrackingindex++;
						priv->rfa_txpowertrackingindex_real++;
						rtl8192_setBBreg(dev,
							 rOFDM0_XATxIQImbalance,
							 bMaskDWord,
							 priv->txbbgain_table
							 [priv->rfa_txpowertrackingindex_real].txbbgain_value);
						priv->rfc_txpowertrackingindex++;
						priv->rfc_txpowertrackingindex_real++;
						rtl8192_setBBreg(dev,
							 rOFDM0_XCTxIQImbalance,
							 bMaskDWord,
							 priv->txbbgain_table[priv->rfc_txpowertrackingindex_real].txbbgain_value);
					} else {
						rtl8192_setBBreg(dev,
							 rOFDM0_XATxIQImbalance,
							 bMaskDWord,
							 priv->txbbgain_table[TxBBGainTableLength - 1].txbbgain_value);
						rtl8192_setBBreg(dev,
							 rOFDM0_XCTxIQImbalance,
							 bMaskDWord, priv->txbbgain_table[TxBBGainTableLength - 1].txbbgain_value);
					}
				} else {
					if (priv->rfa_txpowertrackingindex < (TxBBGainTableLength - 1)) {
						priv->rfa_txpowertrackingindex++;
						priv->rfa_txpowertrackingindex_real++;
						rtl8192_setBBreg(dev, rOFDM0_XATxIQImbalance,
								 bMaskDWord,
								 priv->txbbgain_table[priv->rfa_txpowertrackingindex_real].txbbgain_value);
					} else
						rtl8192_setBBreg(dev, rOFDM0_XATxIQImbalance,
								 bMaskDWord,
								 priv->txbbgain_table[TxBBGainTableLength - 1].txbbgain_value);
				}
			}
			if (RF_Type == RF_2T4R) {
				priv->CCKPresentAttentuation_difference
					= priv->rfa_txpowertrackingindex - priv->rfa_txpowertracking_default;
			} else {
				priv->CCKPresentAttentuation_difference
					= priv->rfa_txpowertrackingindex_real - priv->rfa_txpowertracking_default;
			}

			if (priv->CurrentChannelBW == HT_CHANNEL_WIDTH_20)
				priv->CCKPresentAttentuation =
					 priv->CCKPresentAttentuation_20Mdefault +
					 priv->CCKPresentAttentuation_difference;
			else
				priv->CCKPresentAttentuation =
					 priv->CCKPresentAttentuation_40Mdefault +
					 priv->CCKPresentAttentuation_difference;

			if (priv->CCKPresentAttentuation > (CCKTxBBGainTableLength-1))
				priv->CCKPresentAttentuation = CCKTxBBGainTableLength-1;
			if (priv->CCKPresentAttentuation < 0)
				priv->CCKPresentAttentuation = 0;

			if (priv->CCKPresentAttentuation > -1 &&
			    priv->CCKPresentAttentuation < CCKTxBBGainTableLength) {
				if (priv->rtllib->current_network.channel == 14 &&
				    !priv->bcck_in_ch14) {
					priv->bcck_in_ch14 = true;
					dm_cck_txpower_adjust(dev, priv->bcck_in_ch14);
				} else if (priv->rtllib->current_network.channel != 14 && priv->bcck_in_ch14) {
					priv->bcck_in_ch14 = false;
					dm_cck_txpower_adjust(dev, priv->bcck_in_ch14);
				} else
					dm_cck_txpower_adjust(dev, priv->bcck_in_ch14);
			}
			RT_TRACE(COMP_POWER_TRACKING,
				 "priv->rfa_txpowertrackingindex = %d\n",
				 priv->rfa_txpowertrackingindex);
			RT_TRACE(COMP_POWER_TRACKING,
				 "priv->rfa_txpowertrackingindex_real = %d\n",
				 priv->rfa_txpowertrackingindex_real);
			RT_TRACE(COMP_POWER_TRACKING,
				 "priv->CCKPresentAttentuation_difference = %d\n",
				 priv->CCKPresentAttentuation_difference);
			RT_TRACE(COMP_POWER_TRACKING,
				 "priv->CCKPresentAttentuation = %d\n",
				 priv->CCKPresentAttentuation);

			if (priv->CCKPresentAttentuation_difference <= -12 || priv->CCKPresentAttentuation_difference >= 24) {
				priv->rtllib->bdynamic_txpower_enable = true;
				write_nic_byte(dev, Pw_Track_Flag, 0);
				write_nic_byte(dev, FW_Busy_Flag, 0);
				RT_TRACE(COMP_POWER_TRACKING, "tx power track--->limited\n");
				return;
			}

			write_nic_byte(dev, Pw_Track_Flag, 0);
			Avg_TSSI_Meas_from_driver = 0;
			for (k = 0; k < 5; k++)
				tmp_report[k] = 0;
			break;
		}
		write_nic_byte(dev, FW_Busy_Flag, 0);
	}
	priv->rtllib->bdynamic_txpower_enable = true;
	write_nic_byte(dev, Pw_Track_Flag, 0);
}

static void dm_TXPowerTrackingCallback_ThermalMeter(struct net_device *dev)
{
#define ThermalMeterVal	9
	struct r8192_priv *priv = rtllib_priv(dev);
	u32 tmpRegA, TempCCk;
	u8 tmpOFDMindex, tmpCCKindex, tmpCCK20Mindex, tmpCCK40Mindex, tmpval;
	int i = 0, CCKSwingNeedUpdate = 0;

	if (!priv->btxpower_trackingInit) {
		tmpRegA = rtl8192_QueryBBReg(dev, rOFDM0_XATxIQImbalance, bMaskDWord);
		for (i = 0; i < OFDM_Table_Length; i++) {
			if (tmpRegA == OFDMSwingTable[i]) {
				priv->OFDM_index[0] = (u8)i;
				RT_TRACE(COMP_POWER_TRACKING, "Initial reg0x%x = 0x%x, OFDM_index = 0x%x\n",
					rOFDM0_XATxIQImbalance, tmpRegA, priv->OFDM_index[0]);
			}
		}

		TempCCk = rtl8192_QueryBBReg(dev, rCCK0_TxFilter1, bMaskByte2);
		for (i = 0; i < CCK_Table_length; i++) {
			if (TempCCk == (u32)CCKSwingTable_Ch1_Ch13[i][0]) {
				priv->CCK_index = (u8) i;
				RT_TRACE(COMP_POWER_TRACKING,
					 "Initial reg0x%x = 0x%x, CCK_index = 0x%x\n",
					 rCCK0_TxFilter1, TempCCk,
					 priv->CCK_index);
				break;
			}
		}
		priv->btxpower_trackingInit = true;
		return;
	}

	tmpRegA = rtl8192_phy_QueryRFReg(dev, RF90_PATH_A, 0x12, 0x078);
	RT_TRACE(COMP_POWER_TRACKING, "Readback ThermalMeterA = %d\n", tmpRegA);
	if (tmpRegA < 3 || tmpRegA > 13)
		return;
	if (tmpRegA >= 12)
		tmpRegA = 12;
	RT_TRACE(COMP_POWER_TRACKING, "Valid ThermalMeterA = %d\n", tmpRegA);
	priv->ThermalMeter[0] = ThermalMeterVal;
	priv->ThermalMeter[1] = ThermalMeterVal;

	if (priv->ThermalMeter[0] >= (u8)tmpRegA) {
		tmpOFDMindex = tmpCCK20Mindex = 6+(priv->ThermalMeter[0] -
			      (u8)tmpRegA);
		tmpCCK40Mindex = tmpCCK20Mindex - 6;
		if (tmpOFDMindex >= OFDM_Table_Length)
			tmpOFDMindex = OFDM_Table_Length-1;
		if (tmpCCK20Mindex >= CCK_Table_length)
			tmpCCK20Mindex = CCK_Table_length-1;
		if (tmpCCK40Mindex >= CCK_Table_length)
			tmpCCK40Mindex = CCK_Table_length-1;
	} else {
		tmpval = ((u8)tmpRegA - priv->ThermalMeter[0]);
		if (tmpval >= 6)
			tmpOFDMindex = tmpCCK20Mindex = 0;
		else
			tmpOFDMindex = tmpCCK20Mindex = 6 - tmpval;
		tmpCCK40Mindex = 0;
	}
	if (priv->CurrentChannelBW != HT_CHANNEL_WIDTH_20)
		tmpCCKindex = tmpCCK40Mindex;
	else
		tmpCCKindex = tmpCCK20Mindex;

	priv->Record_CCK_20Mindex = tmpCCK20Mindex;
	priv->Record_CCK_40Mindex = tmpCCK40Mindex;
	RT_TRACE(COMP_POWER_TRACKING,
		 "Record_CCK_20Mindex / Record_CCK_40Mindex = %d / %d.\n",
		 priv->Record_CCK_20Mindex, priv->Record_CCK_40Mindex);

	if (priv->rtllib->current_network.channel == 14 &&
	    !priv->bcck_in_ch14) {
		priv->bcck_in_ch14 = true;
		CCKSwingNeedUpdate = 1;
	} else if (priv->rtllib->current_network.channel != 14 &&
		   priv->bcck_in_ch14) {
		priv->bcck_in_ch14 = false;
		CCKSwingNeedUpdate = 1;
	}

	if (priv->CCK_index != tmpCCKindex) {
		priv->CCK_index = tmpCCKindex;
		CCKSwingNeedUpdate = 1;
	}

	if (CCKSwingNeedUpdate)
		dm_cck_txpower_adjust(dev, priv->bcck_in_ch14);
	if (priv->OFDM_index[0] != tmpOFDMindex) {
		priv->OFDM_index[0] = tmpOFDMindex;
		rtl8192_setBBreg(dev, rOFDM0_XATxIQImbalance, bMaskDWord,
				 OFDMSwingTable[priv->OFDM_index[0]]);
		RT_TRACE(COMP_POWER_TRACKING, "Update OFDMSwing[%d] = 0x%x\n",
			 priv->OFDM_index[0],
			 OFDMSwingTable[priv->OFDM_index[0]]);
	}
	priv->txpower_count = 0;
}

void	dm_txpower_trackingcallback(void *data)
{
	struct r8192_priv *priv = container_of_dwork_rsl(data,
				  struct r8192_priv, txpower_tracking_wq);
	struct net_device *dev = priv->rtllib->dev;

	if (priv->IC_Cut >= IC_VersionCut_D)
		dm_TXPowerTrackingCallback_TSSI(dev);
	else
		dm_TXPowerTrackingCallback_ThermalMeter(dev);
}

static void dm_InitializeTXPowerTracking_TSSI(struct net_device *dev)
{

	struct r8192_priv *priv = rtllib_priv(dev);

	priv->txbbgain_table[0].txbb_iq_amplifygain = 12;
	priv->txbbgain_table[0].txbbgain_value = 0x7f8001fe;
	priv->txbbgain_table[1].txbb_iq_amplifygain = 11;
	priv->txbbgain_table[1].txbbgain_value = 0x788001e2;
	priv->txbbgain_table[2].txbb_iq_amplifygain = 10;
	priv->txbbgain_table[2].txbbgain_value = 0x71c001c7;
	priv->txbbgain_table[3].txbb_iq_amplifygain = 9;
	priv->txbbgain_table[3].txbbgain_value = 0x6b8001ae;
	priv->txbbgain_table[4].txbb_iq_amplifygain = 8;
	priv->txbbgain_table[4].txbbgain_value = 0x65400195;
	priv->txbbgain_table[5].txbb_iq_amplifygain = 7;
	priv->txbbgain_table[5].txbbgain_value = 0x5fc0017f;
	priv->txbbgain_table[6].txbb_iq_amplifygain = 6;
	priv->txbbgain_table[6].txbbgain_value = 0x5a400169;
	priv->txbbgain_table[7].txbb_iq_amplifygain = 5;
	priv->txbbgain_table[7].txbbgain_value = 0x55400155;
	priv->txbbgain_table[8].txbb_iq_amplifygain = 4;
	priv->txbbgain_table[8].txbbgain_value = 0x50800142;
	priv->txbbgain_table[9].txbb_iq_amplifygain = 3;
	priv->txbbgain_table[9].txbbgain_value = 0x4c000130;
	priv->txbbgain_table[10].txbb_iq_amplifygain = 2;
	priv->txbbgain_table[10].txbbgain_value = 0x47c0011f;
	priv->txbbgain_table[11].txbb_iq_amplifygain = 1;
	priv->txbbgain_table[11].txbbgain_value = 0x43c0010f;
	priv->txbbgain_table[12].txbb_iq_amplifygain = 0;
	priv->txbbgain_table[12].txbbgain_value = 0x40000100;
	priv->txbbgain_table[13].txbb_iq_amplifygain = -1;
	priv->txbbgain_table[13].txbbgain_value = 0x3c8000f2;
	priv->txbbgain_table[14].txbb_iq_amplifygain = -2;
	priv->txbbgain_table[14].txbbgain_value = 0x390000e4;
	priv->txbbgain_table[15].txbb_iq_amplifygain = -3;
	priv->txbbgain_table[15].txbbgain_value = 0x35c000d7;
	priv->txbbgain_table[16].txbb_iq_amplifygain = -4;
	priv->txbbgain_table[16].txbbgain_value = 0x32c000cb;
	priv->txbbgain_table[17].txbb_iq_amplifygain = -5;
	priv->txbbgain_table[17].txbbgain_value = 0x300000c0;
	priv->txbbgain_table[18].txbb_iq_amplifygain = -6;
	priv->txbbgain_table[18].txbbgain_value = 0x2d4000b5;
	priv->txbbgain_table[19].txbb_iq_amplifygain = -7;
	priv->txbbgain_table[19].txbbgain_value = 0x2ac000ab;
	priv->txbbgain_table[20].txbb_iq_amplifygain = -8;
	priv->txbbgain_table[20].txbbgain_value = 0x288000a2;
	priv->txbbgain_table[21].txbb_iq_amplifygain = -9;
	priv->txbbgain_table[21].txbbgain_value = 0x26000098;
	priv->txbbgain_table[22].txbb_iq_amplifygain = -10;
	priv->txbbgain_table[22].txbbgain_value = 0x24000090;
	priv->txbbgain_table[23].txbb_iq_amplifygain = -11;
	priv->txbbgain_table[23].txbbgain_value = 0x22000088;
	priv->txbbgain_table[24].txbb_iq_amplifygain = -12;
	priv->txbbgain_table[24].txbbgain_value = 0x20000080;
	priv->txbbgain_table[25].txbb_iq_amplifygain = -13;
	priv->txbbgain_table[25].txbbgain_value = 0x1a00006c;
	priv->txbbgain_table[26].txbb_iq_amplifygain = -14;
	priv->txbbgain_table[26].txbbgain_value = 0x1c800072;
	priv->txbbgain_table[27].txbb_iq_amplifygain = -15;
	priv->txbbgain_table[27].txbbgain_value = 0x18000060;
	priv->txbbgain_table[28].txbb_iq_amplifygain = -16;
	priv->txbbgain_table[28].txbbgain_value = 0x19800066;
	priv->txbbgain_table[29].txbb_iq_amplifygain = -17;
	priv->txbbgain_table[29].txbbgain_value = 0x15800056;
	priv->txbbgain_table[30].txbb_iq_amplifygain = -18;
	priv->txbbgain_table[30].txbbgain_value = 0x26c0005b;
	priv->txbbgain_table[31].txbb_iq_amplifygain = -19;
	priv->txbbgain_table[31].txbbgain_value = 0x14400051;
	priv->txbbgain_table[32].txbb_iq_amplifygain = -20;
	priv->txbbgain_table[32].txbbgain_value = 0x24400051;
	priv->txbbgain_table[33].txbb_iq_amplifygain = -21;
	priv->txbbgain_table[33].txbbgain_value = 0x1300004c;
	priv->txbbgain_table[34].txbb_iq_amplifygain = -22;
	priv->txbbgain_table[34].txbbgain_value = 0x12000048;
	priv->txbbgain_table[35].txbb_iq_amplifygain = -23;
	priv->txbbgain_table[35].txbbgain_value = 0x11000044;
	priv->txbbgain_table[36].txbb_iq_amplifygain = -24;
	priv->txbbgain_table[36].txbbgain_value = 0x10000040;

	priv->cck_txbbgain_table[0].ccktxbb_valuearray[0] = 0x36;
	priv->cck_txbbgain_table[0].ccktxbb_valuearray[1] = 0x35;
	priv->cck_txbbgain_table[0].ccktxbb_valuearray[2] = 0x2e;
	priv->cck_txbbgain_table[0].ccktxbb_valuearray[3] = 0x25;
	priv->cck_txbbgain_table[0].ccktxbb_valuearray[4] = 0x1c;
	priv->cck_txbbgain_table[0].ccktxbb_valuearray[5] = 0x12;
	priv->cck_txbbgain_table[0].ccktxbb_valuearray[6] = 0x09;
	priv->cck_txbbgain_table[0].ccktxbb_valuearray[7] = 0x04;

	priv->cck_txbbgain_table[1].ccktxbb_valuearray[0] = 0x33;
	priv->cck_txbbgain_table[1].ccktxbb_valuearray[1] = 0x32;
	priv->cck_txbbgain_table[1].ccktxbb_valuearray[2] = 0x2b;
	priv->cck_txbbgain_table[1].ccktxbb_valuearray[3] = 0x23;
	priv->cck_txbbgain_table[1].ccktxbb_valuearray[4] = 0x1a;
	priv->cck_txbbgain_table[1].ccktxbb_valuearray[5] = 0x11;
	priv->cck_txbbgain_table[1].ccktxbb_valuearray[6] = 0x08;
	priv->cck_txbbgain_table[1].ccktxbb_valuearray[7] = 0x04;

	priv->cck_txbbgain_table[2].ccktxbb_valuearray[0] = 0x30;
	priv->cck_txbbgain_table[2].ccktxbb_valuearray[1] = 0x2f;
	priv->cck_txbbgain_table[2].ccktxbb_valuearray[2] = 0x29;
	priv->cck_txbbgain_table[2].ccktxbb_valuearray[3] = 0x21;
	priv->cck_txbbgain_table[2].ccktxbb_valuearray[4] = 0x19;
	priv->cck_txbbgain_table[2].ccktxbb_valuearray[5] = 0x10;
	priv->cck_txbbgain_table[2].ccktxbb_valuearray[6] = 0x08;
	priv->cck_txbbgain_table[2].ccktxbb_valuearray[7] = 0x03;

	priv->cck_txbbgain_table[3].ccktxbb_valuearray[0] = 0x2d;
	priv->cck_txbbgain_table[3].ccktxbb_valuearray[1] = 0x2d;
	priv->cck_txbbgain_table[3].ccktxbb_valuearray[2] = 0x27;
	priv->cck_txbbgain_table[3].ccktxbb_valuearray[3] = 0x1f;
	priv->cck_txbbgain_table[3].ccktxbb_valuearray[4] = 0x18;
	priv->cck_txbbgain_table[3].ccktxbb_valuearray[5] = 0x0f;
	priv->cck_txbbgain_table[3].ccktxbb_valuearray[6] = 0x08;
	priv->cck_txbbgain_table[3].ccktxbb_valuearray[7] = 0x03;

	priv->cck_txbbgain_table[4].ccktxbb_valuearray[0] = 0x2b;
	priv->cck_txbbgain_table[4].ccktxbb_valuearray[1] = 0x2a;
	priv->cck_txbbgain_table[4].ccktxbb_valuearray[2] = 0x25;
	priv->cck_txbbgain_table[4].ccktxbb_valuearray[3] = 0x1e;
	priv->cck_txbbgain_table[4].ccktxbb_valuearray[4] = 0x16;
	priv->cck_txbbgain_table[4].ccktxbb_valuearray[5] = 0x0e;
	priv->cck_txbbgain_table[4].ccktxbb_valuearray[6] = 0x07;
	priv->cck_txbbgain_table[4].ccktxbb_valuearray[7] = 0x03;

	priv->cck_txbbgain_table[5].ccktxbb_valuearray[0] = 0x28;
	priv->cck_txbbgain_table[5].ccktxbb_valuearray[1] = 0x28;
	priv->cck_txbbgain_table[5].ccktxbb_valuearray[2] = 0x22;
	priv->cck_txbbgain_table[5].ccktxbb_valuearray[3] = 0x1c;
	priv->cck_txbbgain_table[5].ccktxbb_valuearray[4] = 0x15;
	priv->cck_txbbgain_table[5].ccktxbb_valuearray[5] = 0x0d;
	priv->cck_txbbgain_table[5].ccktxbb_valuearray[6] = 0x07;
	priv->cck_txbbgain_table[5].ccktxbb_valuearray[7] = 0x03;

	priv->cck_txbbgain_table[6].ccktxbb_valuearray[0] = 0x26;
	priv->cck_txbbgain_table[6].ccktxbb_valuearray[1] = 0x25;
	priv->cck_txbbgain_table[6].ccktxbb_valuearray[2] = 0x21;
	priv->cck_txbbgain_table[6].ccktxbb_valuearray[3] = 0x1b;
	priv->cck_txbbgain_table[6].ccktxbb_valuearray[4] = 0x14;
	priv->cck_txbbgain_table[6].ccktxbb_valuearray[5] = 0x0d;
	priv->cck_txbbgain_table[6].ccktxbb_valuearray[6] = 0x06;
	priv->cck_txbbgain_table[6].ccktxbb_valuearray[7] = 0x03;

	priv->cck_txbbgain_table[7].ccktxbb_valuearray[0] = 0x24;
	priv->cck_txbbgain_table[7].ccktxbb_valuearray[1] = 0x23;
	priv->cck_txbbgain_table[7].ccktxbb_valuearray[2] = 0x1f;
	priv->cck_txbbgain_table[7].ccktxbb_valuearray[3] = 0x19;
	priv->cck_txbbgain_table[7].ccktxbb_valuearray[4] = 0x13;
	priv->cck_txbbgain_table[7].ccktxbb_valuearray[5] = 0x0c;
	priv->cck_txbbgain_table[7].ccktxbb_valuearray[6] = 0x06;
	priv->cck_txbbgain_table[7].ccktxbb_valuearray[7] = 0x03;

	priv->cck_txbbgain_table[8].ccktxbb_valuearray[0] = 0x22;
	priv->cck_txbbgain_table[8].ccktxbb_valuearray[1] = 0x21;
	priv->cck_txbbgain_table[8].ccktxbb_valuearray[2] = 0x1d;
	priv->cck_txbbgain_table[8].ccktxbb_valuearray[3] = 0x18;
	priv->cck_txbbgain_table[8].ccktxbb_valuearray[4] = 0x11;
	priv->cck_txbbgain_table[8].ccktxbb_valuearray[5] = 0x0b;
	priv->cck_txbbgain_table[8].ccktxbb_valuearray[6] = 0x06;
	priv->cck_txbbgain_table[8].ccktxbb_valuearray[7] = 0x02;

	priv->cck_txbbgain_table[9].ccktxbb_valuearray[0] = 0x20;
	priv->cck_txbbgain_table[9].ccktxbb_valuearray[1] = 0x20;
	priv->cck_txbbgain_table[9].ccktxbb_valuearray[2] = 0x1b;
	priv->cck_txbbgain_table[9].ccktxbb_valuearray[3] = 0x16;
	priv->cck_txbbgain_table[9].ccktxbb_valuearray[4] = 0x11;
	priv->cck_txbbgain_table[9].ccktxbb_valuearray[5] = 0x08;
	priv->cck_txbbgain_table[9].ccktxbb_valuearray[6] = 0x05;
	priv->cck_txbbgain_table[9].ccktxbb_valuearray[7] = 0x02;

	priv->cck_txbbgain_table[10].ccktxbb_valuearray[0] = 0x1f;
	priv->cck_txbbgain_table[10].ccktxbb_valuearray[1] = 0x1e;
	priv->cck_txbbgain_table[10].ccktxbb_valuearray[2] = 0x1a;
	priv->cck_txbbgain_table[10].ccktxbb_valuearray[3] = 0x15;
	priv->cck_txbbgain_table[10].ccktxbb_valuearray[4] = 0x10;
	priv->cck_txbbgain_table[10].ccktxbb_valuearray[5] = 0x0a;
	priv->cck_txbbgain_table[10].ccktxbb_valuearray[6] = 0x05;
	priv->cck_txbbgain_table[10].ccktxbb_valuearray[7] = 0x02;

	priv->cck_txbbgain_table[11].ccktxbb_valuearray[0] = 0x1d;
	priv->cck_txbbgain_table[11].ccktxbb_valuearray[1] = 0x1c;
	priv->cck_txbbgain_table[11].ccktxbb_valuearray[2] = 0x18;
	priv->cck_txbbgain_table[11].ccktxbb_valuearray[3] = 0x14;
	priv->cck_txbbgain_table[11].ccktxbb_valuearray[4] = 0x0f;
	priv->cck_txbbgain_table[11].ccktxbb_valuearray[5] = 0x0a;
	priv->cck_txbbgain_table[11].ccktxbb_valuearray[6] = 0x05;
	priv->cck_txbbgain_table[11].ccktxbb_valuearray[7] = 0x02;

	priv->cck_txbbgain_table[12].ccktxbb_valuearray[0] = 0x1b;
	priv->cck_txbbgain_table[12].ccktxbb_valuearray[1] = 0x1a;
	priv->cck_txbbgain_table[12].ccktxbb_valuearray[2] = 0x17;
	priv->cck_txbbgain_table[12].ccktxbb_valuearray[3] = 0x13;
	priv->cck_txbbgain_table[12].ccktxbb_valuearray[4] = 0x0e;
	priv->cck_txbbgain_table[12].ccktxbb_valuearray[5] = 0x09;
	priv->cck_txbbgain_table[12].ccktxbb_valuearray[6] = 0x04;
	priv->cck_txbbgain_table[12].ccktxbb_valuearray[7] = 0x02;

	priv->cck_txbbgain_table[13].ccktxbb_valuearray[0] = 0x1a;
	priv->cck_txbbgain_table[13].ccktxbb_valuearray[1] = 0x19;
	priv->cck_txbbgain_table[13].ccktxbb_valuearray[2] = 0x16;
	priv->cck_txbbgain_table[13].ccktxbb_valuearray[3] = 0x12;
	priv->cck_txbbgain_table[13].ccktxbb_valuearray[4] = 0x0d;
	priv->cck_txbbgain_table[13].ccktxbb_valuearray[5] = 0x09;
	priv->cck_txbbgain_table[13].ccktxbb_valuearray[6] = 0x04;
	priv->cck_txbbgain_table[13].ccktxbb_valuearray[7] = 0x02;

	priv->cck_txbbgain_table[14].ccktxbb_valuearray[0] = 0x18;
	priv->cck_txbbgain_table[14].ccktxbb_valuearray[1] = 0x17;
	priv->cck_txbbgain_table[14].ccktxbb_valuearray[2] = 0x15;
	priv->cck_txbbgain_table[14].ccktxbb_valuearray[3] = 0x11;
	priv->cck_txbbgain_table[14].ccktxbb_valuearray[4] = 0x0c;
	priv->cck_txbbgain_table[14].ccktxbb_valuearray[5] = 0x08;
	priv->cck_txbbgain_table[14].ccktxbb_valuearray[6] = 0x04;
	priv->cck_txbbgain_table[14].ccktxbb_valuearray[7] = 0x02;

	priv->cck_txbbgain_table[15].ccktxbb_valuearray[0] = 0x17;
	priv->cck_txbbgain_table[15].ccktxbb_valuearray[1] = 0x16;
	priv->cck_txbbgain_table[15].ccktxbb_valuearray[2] = 0x13;
	priv->cck_txbbgain_table[15].ccktxbb_valuearray[3] = 0x10;
	priv->cck_txbbgain_table[15].ccktxbb_valuearray[4] = 0x0c;
	priv->cck_txbbgain_table[15].ccktxbb_valuearray[5] = 0x08;
	priv->cck_txbbgain_table[15].ccktxbb_valuearray[6] = 0x04;
	priv->cck_txbbgain_table[15].ccktxbb_valuearray[7] = 0x02;

	priv->cck_txbbgain_table[16].ccktxbb_valuearray[0] = 0x16;
	priv->cck_txbbgain_table[16].ccktxbb_valuearray[1] = 0x15;
	priv->cck_txbbgain_table[16].ccktxbb_valuearray[2] = 0x12;
	priv->cck_txbbgain_table[16].ccktxbb_valuearray[3] = 0x0f;
	priv->cck_txbbgain_table[16].ccktxbb_valuearray[4] = 0x0b;
	priv->cck_txbbgain_table[16].ccktxbb_valuearray[5] = 0x07;
	priv->cck_txbbgain_table[16].ccktxbb_valuearray[6] = 0x04;
	priv->cck_txbbgain_table[16].ccktxbb_valuearray[7] = 0x01;

	priv->cck_txbbgain_table[17].ccktxbb_valuearray[0] = 0x14;
	priv->cck_txbbgain_table[17].ccktxbb_valuearray[1] = 0x14;
	priv->cck_txbbgain_table[17].ccktxbb_valuearray[2] = 0x11;
	priv->cck_txbbgain_table[17].ccktxbb_valuearray[3] = 0x0e;
	priv->cck_txbbgain_table[17].ccktxbb_valuearray[4] = 0x0b;
	priv->cck_txbbgain_table[17].ccktxbb_valuearray[5] = 0x07;
	priv->cck_txbbgain_table[17].ccktxbb_valuearray[6] = 0x03;
	priv->cck_txbbgain_table[17].ccktxbb_valuearray[7] = 0x02;

	priv->cck_txbbgain_table[18].ccktxbb_valuearray[0] = 0x13;
	priv->cck_txbbgain_table[18].ccktxbb_valuearray[1] = 0x13;
	priv->cck_txbbgain_table[18].ccktxbb_valuearray[2] = 0x10;
	priv->cck_txbbgain_table[18].ccktxbb_valuearray[3] = 0x0d;
	priv->cck_txbbgain_table[18].ccktxbb_valuearray[4] = 0x0a;
	priv->cck_txbbgain_table[18].ccktxbb_valuearray[5] = 0x06;
	priv->cck_txbbgain_table[18].ccktxbb_valuearray[6] = 0x03;
	priv->cck_txbbgain_table[18].ccktxbb_valuearray[7] = 0x01;

	priv->cck_txbbgain_table[19].ccktxbb_valuearray[0] = 0x12;
	priv->cck_txbbgain_table[19].ccktxbb_valuearray[1] = 0x12;
	priv->cck_txbbgain_table[19].ccktxbb_valuearray[2] = 0x0f;
	priv->cck_txbbgain_table[19].ccktxbb_valuearray[3] = 0x0c;
	priv->cck_txbbgain_table[19].ccktxbb_valuearray[4] = 0x09;
	priv->cck_txbbgain_table[19].ccktxbb_valuearray[5] = 0x06;
	priv->cck_txbbgain_table[19].ccktxbb_valuearray[6] = 0x03;
	priv->cck_txbbgain_table[19].ccktxbb_valuearray[7] = 0x01;

	priv->cck_txbbgain_table[20].ccktxbb_valuearray[0] = 0x11;
	priv->cck_txbbgain_table[20].ccktxbb_valuearray[1] = 0x11;
	priv->cck_txbbgain_table[20].ccktxbb_valuearray[2] = 0x0f;
	priv->cck_txbbgain_table[20].ccktxbb_valuearray[3] = 0x0c;
	priv->cck_txbbgain_table[20].ccktxbb_valuearray[4] = 0x09;
	priv->cck_txbbgain_table[20].ccktxbb_valuearray[5] = 0x06;
	priv->cck_txbbgain_table[20].ccktxbb_valuearray[6] = 0x03;
	priv->cck_txbbgain_table[20].ccktxbb_valuearray[7] = 0x01;

	priv->cck_txbbgain_table[21].ccktxbb_valuearray[0] = 0x10;
	priv->cck_txbbgain_table[21].ccktxbb_valuearray[1] = 0x10;
	priv->cck_txbbgain_table[21].ccktxbb_valuearray[2] = 0x0e;
	priv->cck_txbbgain_table[21].ccktxbb_valuearray[3] = 0x0b;
	priv->cck_txbbgain_table[21].ccktxbb_valuearray[4] = 0x08;
	priv->cck_txbbgain_table[21].ccktxbb_valuearray[5] = 0x05;
	priv->cck_txbbgain_table[21].ccktxbb_valuearray[6] = 0x03;
	priv->cck_txbbgain_table[21].ccktxbb_valuearray[7] = 0x01;

	priv->cck_txbbgain_table[22].ccktxbb_valuearray[0] = 0x0f;
	priv->cck_txbbgain_table[22].ccktxbb_valuearray[1] = 0x0f;
	priv->cck_txbbgain_table[22].ccktxbb_valuearray[2] = 0x0d;
	priv->cck_txbbgain_table[22].ccktxbb_valuearray[3] = 0x0b;
	priv->cck_txbbgain_table[22].ccktxbb_valuearray[4] = 0x08;
	priv->cck_txbbgain_table[22].ccktxbb_valuearray[5] = 0x05;
	priv->cck_txbbgain_table[22].ccktxbb_valuearray[6] = 0x03;
	priv->cck_txbbgain_table[22].ccktxbb_valuearray[7] = 0x01;

	priv->cck_txbbgain_ch14_table[0].ccktxbb_valuearray[0] = 0x36;
	priv->cck_txbbgain_ch14_table[0].ccktxbb_valuearray[1] = 0x35;
	priv->cck_txbbgain_ch14_table[0].ccktxbb_valuearray[2] = 0x2e;
	priv->cck_txbbgain_ch14_table[0].ccktxbb_valuearray[3] = 0x1b;
	priv->cck_txbbgain_ch14_table[0].ccktxbb_valuearray[4] = 0x00;
	priv->cck_txbbgain_ch14_table[0].ccktxbb_valuearray[5] = 0x00;
	priv->cck_txbbgain_ch14_table[0].ccktxbb_valuearray[6] = 0x00;
	priv->cck_txbbgain_ch14_table[0].ccktxbb_valuearray[7] = 0x00;

	priv->cck_txbbgain_ch14_table[1].ccktxbb_valuearray[0] = 0x33;
	priv->cck_txbbgain_ch14_table[1].ccktxbb_valuearray[1] = 0x32;
	priv->cck_txbbgain_ch14_table[1].ccktxbb_valuearray[2] = 0x2b;
	priv->cck_txbbgain_ch14_table[1].ccktxbb_valuearray[3] = 0x19;
	priv->cck_txbbgain_ch14_table[1].ccktxbb_valuearray[4] = 0x00;
	priv->cck_txbbgain_ch14_table[1].ccktxbb_valuearray[5] = 0x00;
	priv->cck_txbbgain_ch14_table[1].ccktxbb_valuearray[6] = 0x00;
	priv->cck_txbbgain_ch14_table[1].ccktxbb_valuearray[7] = 0x00;

	priv->cck_txbbgain_ch14_table[2].ccktxbb_valuearray[0] = 0x30;
	priv->cck_txbbgain_ch14_table[2].ccktxbb_valuearray[1] = 0x2f;
	priv->cck_txbbgain_ch14_table[2].ccktxbb_valuearray[2] = 0x29;
	priv->cck_txbbgain_ch14_table[2].ccktxbb_valuearray[3] = 0x18;
	priv->cck_txbbgain_ch14_table[2].ccktxbb_valuearray[4] = 0x00;
	priv->cck_txbbgain_ch14_table[2].ccktxbb_valuearray[5] = 0x00;
	priv->cck_txbbgain_ch14_table[2].ccktxbb_valuearray[6] = 0x00;
	priv->cck_txbbgain_ch14_table[2].ccktxbb_valuearray[7] = 0x00;

	priv->cck_txbbgain_ch14_table[3].ccktxbb_valuearray[0] = 0x2d;
	priv->cck_txbbgain_ch14_table[3].ccktxbb_valuearray[1] = 0x2d;
	priv->cck_txbbgain_ch14_table[3].ccktxbb_valuearray[2] = 0x27;
	priv->cck_txbbgain_ch14_table[3].ccktxbb_valuearray[3] = 0x17;
	priv->cck_txbbgain_ch14_table[3].ccktxbb_valuearray[4] = 0x00;
	priv->cck_txbbgain_ch14_table[3].ccktxbb_valuearray[5] = 0x00;
	priv->cck_txbbgain_ch14_table[3].ccktxbb_valuearray[6] = 0x00;
	priv->cck_txbbgain_ch14_table[3].ccktxbb_valuearray[7] = 0x00;

	priv->cck_txbbgain_ch14_table[4].ccktxbb_valuearray[0] = 0x2b;
	priv->cck_txbbgain_ch14_table[4].ccktxbb_valuearray[1] = 0x2a;
	priv->cck_txbbgain_ch14_table[4].ccktxbb_valuearray[2] = 0x25;
	priv->cck_txbbgain_ch14_table[4].ccktxbb_valuearray[3] = 0x15;
	priv->cck_txbbgain_ch14_table[4].ccktxbb_valuearray[4] = 0x00;
	priv->cck_txbbgain_ch14_table[4].ccktxbb_valuearray[5] = 0x00;
	priv->cck_txbbgain_ch14_table[4].ccktxbb_valuearray[6] = 0x00;
	priv->cck_txbbgain_ch14_table[4].ccktxbb_valuearray[7] = 0x00;

	priv->cck_txbbgain_ch14_table[5].ccktxbb_valuearray[0] = 0x28;
	priv->cck_txbbgain_ch14_table[5].ccktxbb_valuearray[1] = 0x28;
	priv->cck_txbbgain_ch14_table[5].ccktxbb_valuearray[2] = 0x22;
	priv->cck_txbbgain_ch14_table[5].ccktxbb_valuearray[3] = 0x14;
	priv->cck_txbbgain_ch14_table[5].ccktxbb_valuearray[4] = 0x00;
	priv->cck_txbbgain_ch14_table[5].ccktxbb_valuearray[5] = 0x00;
	priv->cck_txbbgain_ch14_table[5].ccktxbb_valuearray[6] = 0x00;
	priv->cck_txbbgain_ch14_table[5].ccktxbb_valuearray[7] = 0x00;

	priv->cck_txbbgain_ch14_table[6].ccktxbb_valuearray[0] = 0x26;
	priv->cck_txbbgain_ch14_table[6].ccktxbb_valuearray[1] = 0x25;
	priv->cck_txbbgain_ch14_table[6].ccktxbb_valuearray[2] = 0x21;
	priv->cck_txbbgain_ch14_table[6].ccktxbb_valuearray[3] = 0x13;
	priv->cck_txbbgain_ch14_table[6].ccktxbb_valuearray[4] = 0x00;
	priv->cck_txbbgain_ch14_table[6].ccktxbb_valuearray[5] = 0x00;
	priv->cck_txbbgain_ch14_table[6].ccktxbb_valuearray[6] = 0x00;
	priv->cck_txbbgain_ch14_table[6].ccktxbb_valuearray[7] = 0x00;

	priv->cck_txbbgain_ch14_table[7].ccktxbb_valuearray[0] = 0x24;
	priv->cck_txbbgain_ch14_table[7].ccktxbb_valuearray[1] = 0x23;
	priv->cck_txbbgain_ch14_table[7].ccktxbb_valuearray[2] = 0x1f;
	priv->cck_txbbgain_ch14_table[7].ccktxbb_valuearray[3] = 0x12;
	priv->cck_txbbgain_ch14_table[7].ccktxbb_valuearray[4] = 0x00;
	priv->cck_txbbgain_ch14_table[7].ccktxbb_valuearray[5] = 0x00;
	priv->cck_txbbgain_ch14_table[7].ccktxbb_valuearray[6] = 0x00;
	priv->cck_txbbgain_ch14_table[7].ccktxbb_valuearray[7] = 0x00;

	priv->cck_txbbgain_ch14_table[8].ccktxbb_valuearray[0] = 0x22;
	priv->cck_txbbgain_ch14_table[8].ccktxbb_valuearray[1] = 0x21;
	priv->cck_txbbgain_ch14_table[8].ccktxbb_valuearray[2] = 0x1d;
	priv->cck_txbbgain_ch14_table[8].ccktxbb_valuearray[3] = 0x11;
	priv->cck_txbbgain_ch14_table[8].ccktxbb_valuearray[4] = 0x00;
	priv->cck_txbbgain_ch14_table[8].ccktxbb_valuearray[5] = 0x00;
	priv->cck_txbbgain_ch14_table[8].ccktxbb_valuearray[6] = 0x00;
	priv->cck_txbbgain_ch14_table[8].ccktxbb_valuearray[7] = 0x00;

	priv->cck_txbbgain_ch14_table[9].ccktxbb_valuearray[0] = 0x20;
	priv->cck_txbbgain_ch14_table[9].ccktxbb_valuearray[1] = 0x20;
	priv->cck_txbbgain_ch14_table[9].ccktxbb_valuearray[2] = 0x1b;
	priv->cck_txbbgain_ch14_table[9].ccktxbb_valuearray[3] = 0x10;
	priv->cck_txbbgain_ch14_table[9].ccktxbb_valuearray[4] = 0x00;
	priv->cck_txbbgain_ch14_table[9].ccktxbb_valuearray[5] = 0x00;
	priv->cck_txbbgain_ch14_table[9].ccktxbb_valuearray[6] = 0x00;
	priv->cck_txbbgain_ch14_table[9].ccktxbb_valuearray[7] = 0x00;

	priv->cck_txbbgain_ch14_table[10].ccktxbb_valuearray[0] = 0x1f;
	priv->cck_txbbgain_ch14_table[10].ccktxbb_valuearray[1] = 0x1e;
	priv->cck_txbbgain_ch14_table[10].ccktxbb_valuearray[2] = 0x1a;
	priv->cck_txbbgain_ch14_table[10].ccktxbb_valuearray[3] = 0x0f;
	priv->cck_txbbgain_ch14_table[10].ccktxbb_valuearray[4] = 0x00;
	priv->cck_txbbgain_ch14_table[10].ccktxbb_valuearray[5] = 0x00;
	priv->cck_txbbgain_ch14_table[10].ccktxbb_valuearray[6] = 0x00;
	priv->cck_txbbgain_ch14_table[10].ccktxbb_valuearray[7] = 0x00;

	priv->cck_txbbgain_ch14_table[11].ccktxbb_valuearray[0] = 0x1d;
	priv->cck_txbbgain_ch14_table[11].ccktxbb_valuearray[1] = 0x1c;
	priv->cck_txbbgain_ch14_table[11].ccktxbb_valuearray[2] = 0x18;
	priv->cck_txbbgain_ch14_table[11].ccktxbb_valuearray[3] = 0x0e;
	priv->cck_txbbgain_ch14_table[11].ccktxbb_valuearray[4] = 0x00;
	priv->cck_txbbgain_ch14_table[11].ccktxbb_valuearray[5] = 0x00;
	priv->cck_txbbgain_ch14_table[11].ccktxbb_valuearray[6] = 0x00;
	priv->cck_txbbgain_ch14_table[11].ccktxbb_valuearray[7] = 0x00;

	priv->cck_txbbgain_ch14_table[12].ccktxbb_valuearray[0] = 0x1b;
	priv->cck_txbbgain_ch14_table[12].ccktxbb_valuearray[1] = 0x1a;
	priv->cck_txbbgain_ch14_table[12].ccktxbb_valuearray[2] = 0x17;
	priv->cck_txbbgain_ch14_table[12].ccktxbb_valuearray[3] = 0x0e;
	priv->cck_txbbgain_ch14_table[12].ccktxbb_valuearray[4] = 0x00;
	priv->cck_txbbgain_ch14_table[12].ccktxbb_valuearray[5] = 0x00;
	priv->cck_txbbgain_ch14_table[12].ccktxbb_valuearray[6] = 0x00;
	priv->cck_txbbgain_ch14_table[12].ccktxbb_valuearray[7] = 0x00;

	priv->cck_txbbgain_ch14_table[13].ccktxbb_valuearray[0] = 0x1a;
	priv->cck_txbbgain_ch14_table[13].ccktxbb_valuearray[1] = 0x19;
	priv->cck_txbbgain_ch14_table[13].ccktxbb_valuearray[2] = 0x16;
	priv->cck_txbbgain_ch14_table[13].ccktxbb_valuearray[3] = 0x0d;
	priv->cck_txbbgain_ch14_table[13].ccktxbb_valuearray[4] = 0x00;
	priv->cck_txbbgain_ch14_table[13].ccktxbb_valuearray[5] = 0x00;
	priv->cck_txbbgain_ch14_table[13].ccktxbb_valuearray[6] = 0x00;
	priv->cck_txbbgain_ch14_table[13].ccktxbb_valuearray[7] = 0x00;

	priv->cck_txbbgain_ch14_table[14].ccktxbb_valuearray[0] = 0x18;
	priv->cck_txbbgain_ch14_table[14].ccktxbb_valuearray[1] = 0x17;
	priv->cck_txbbgain_ch14_table[14].ccktxbb_valuearray[2] = 0x15;
	priv->cck_txbbgain_ch14_table[14].ccktxbb_valuearray[3] = 0x0c;
	priv->cck_txbbgain_ch14_table[14].ccktxbb_valuearray[4] = 0x00;
	priv->cck_txbbgain_ch14_table[14].ccktxbb_valuearray[5] = 0x00;
	priv->cck_txbbgain_ch14_table[14].ccktxbb_valuearray[6] = 0x00;
	priv->cck_txbbgain_ch14_table[14].ccktxbb_valuearray[7] = 0x00;

	priv->cck_txbbgain_ch14_table[15].ccktxbb_valuearray[0] = 0x17;
	priv->cck_txbbgain_ch14_table[15].ccktxbb_valuearray[1] = 0x16;
	priv->cck_txbbgain_ch14_table[15].ccktxbb_valuearray[2] = 0x13;
	priv->cck_txbbgain_ch14_table[15].ccktxbb_valuearray[3] = 0x0b;
	priv->cck_txbbgain_ch14_table[15].ccktxbb_valuearray[4] = 0x00;
	priv->cck_txbbgain_ch14_table[15].ccktxbb_valuearray[5] = 0x00;
	priv->cck_txbbgain_ch14_table[15].ccktxbb_valuearray[6] = 0x00;
	priv->cck_txbbgain_ch14_table[15].ccktxbb_valuearray[7] = 0x00;

	priv->cck_txbbgain_ch14_table[16].ccktxbb_valuearray[0] = 0x16;
	priv->cck_txbbgain_ch14_table[16].ccktxbb_valuearray[1] = 0x15;
	priv->cck_txbbgain_ch14_table[16].ccktxbb_valuearray[2] = 0x12;
	priv->cck_txbbgain_ch14_table[16].ccktxbb_valuearray[3] = 0x0b;
	priv->cck_txbbgain_ch14_table[16].ccktxbb_valuearray[4] = 0x00;
	priv->cck_txbbgain_ch14_table[16].ccktxbb_valuearray[5] = 0x00;
	priv->cck_txbbgain_ch14_table[16].ccktxbb_valuearray[6] = 0x00;
	priv->cck_txbbgain_ch14_table[16].ccktxbb_valuearray[7] = 0x00;

	priv->cck_txbbgain_ch14_table[17].ccktxbb_valuearray[0] = 0x14;
	priv->cck_txbbgain_ch14_table[17].ccktxbb_valuearray[1] = 0x14;
	priv->cck_txbbgain_ch14_table[17].ccktxbb_valuearray[2] = 0x11;
	priv->cck_txbbgain_ch14_table[17].ccktxbb_valuearray[3] = 0x0a;
	priv->cck_txbbgain_ch14_table[17].ccktxbb_valuearray[4] = 0x00;
	priv->cck_txbbgain_ch14_table[17].ccktxbb_valuearray[5] = 0x00;
	priv->cck_txbbgain_ch14_table[17].ccktxbb_valuearray[6] = 0x00;
	priv->cck_txbbgain_ch14_table[17].ccktxbb_valuearray[7] = 0x00;

	priv->cck_txbbgain_ch14_table[18].ccktxbb_valuearray[0] = 0x13;
	priv->cck_txbbgain_ch14_table[18].ccktxbb_valuearray[1] = 0x13;
	priv->cck_txbbgain_ch14_table[18].ccktxbb_valuearray[2] = 0x10;
	priv->cck_txbbgain_ch14_table[18].ccktxbb_valuearray[3] = 0x0a;
	priv->cck_txbbgain_ch14_table[18].ccktxbb_valuearray[4] = 0x00;
	priv->cck_txbbgain_ch14_table[18].ccktxbb_valuearray[5] = 0x00;
	priv->cck_txbbgain_ch14_table[18].ccktxbb_valuearray[6] = 0x00;
	priv->cck_txbbgain_ch14_table[18].ccktxbb_valuearray[7] = 0x00;

	priv->cck_txbbgain_ch14_table[19].ccktxbb_valuearray[0] = 0x12;
	priv->cck_txbbgain_ch14_table[19].ccktxbb_valuearray[1] = 0x12;
	priv->cck_txbbgain_ch14_table[19].ccktxbb_valuearray[2] = 0x0f;
	priv->cck_txbbgain_ch14_table[19].ccktxbb_valuearray[3] = 0x09;
	priv->cck_txbbgain_ch14_table[19].ccktxbb_valuearray[4] = 0x00;
	priv->cck_txbbgain_ch14_table[19].ccktxbb_valuearray[5] = 0x00;
	priv->cck_txbbgain_ch14_table[19].ccktxbb_valuearray[6] = 0x00;
	priv->cck_txbbgain_ch14_table[19].ccktxbb_valuearray[7] = 0x00;

	priv->cck_txbbgain_ch14_table[20].ccktxbb_valuearray[0] = 0x11;
	priv->cck_txbbgain_ch14_table[20].ccktxbb_valuearray[1] = 0x11;
	priv->cck_txbbgain_ch14_table[20].ccktxbb_valuearray[2] = 0x0f;
	priv->cck_txbbgain_ch14_table[20].ccktxbb_valuearray[3] = 0x09;
	priv->cck_txbbgain_ch14_table[20].ccktxbb_valuearray[4] = 0x00;
	priv->cck_txbbgain_ch14_table[20].ccktxbb_valuearray[5] = 0x00;
	priv->cck_txbbgain_ch14_table[20].ccktxbb_valuearray[6] = 0x00;
	priv->cck_txbbgain_ch14_table[20].ccktxbb_valuearray[7] = 0x00;

	priv->cck_txbbgain_ch14_table[21].ccktxbb_valuearray[0] = 0x10;
	priv->cck_txbbgain_ch14_table[21].ccktxbb_valuearray[1] = 0x10;
	priv->cck_txbbgain_ch14_table[21].ccktxbb_valuearray[2] = 0x0e;
	priv->cck_txbbgain_ch14_table[21].ccktxbb_valuearray[3] = 0x08;
	priv->cck_txbbgain_ch14_table[21].ccktxbb_valuearray[4] = 0x00;
	priv->cck_txbbgain_ch14_table[21].ccktxbb_valuearray[5] = 0x00;
	priv->cck_txbbgain_ch14_table[21].ccktxbb_valuearray[6] = 0x00;
	priv->cck_txbbgain_ch14_table[21].ccktxbb_valuearray[7] = 0x00;

	priv->cck_txbbgain_ch14_table[22].ccktxbb_valuearray[0] = 0x0f;
	priv->cck_txbbgain_ch14_table[22].ccktxbb_valuearray[1] = 0x0f;
	priv->cck_txbbgain_ch14_table[22].ccktxbb_valuearray[2] = 0x0d;
	priv->cck_txbbgain_ch14_table[22].ccktxbb_valuearray[3] = 0x08;
	priv->cck_txbbgain_ch14_table[22].ccktxbb_valuearray[4] = 0x00;
	priv->cck_txbbgain_ch14_table[22].ccktxbb_valuearray[5] = 0x00;
	priv->cck_txbbgain_ch14_table[22].ccktxbb_valuearray[6] = 0x00;
	priv->cck_txbbgain_ch14_table[22].ccktxbb_valuearray[7] = 0x00;

	priv->btxpower_tracking = true;
	priv->txpower_count       = 0;
	priv->btxpower_trackingInit = false;

}

static void dm_InitializeTXPowerTracking_ThermalMeter(struct net_device *dev)
{
	struct r8192_priv *priv = rtllib_priv(dev);


	if (priv->rtllib->FwRWRF)
		priv->btxpower_tracking = true;
	else
		priv->btxpower_tracking = false;
	priv->txpower_count       = 0;
	priv->btxpower_trackingInit = false;
	RT_TRACE(COMP_POWER_TRACKING, "pMgntInfo->bTXPowerTracking = %d\n",
		 priv->btxpower_tracking);
}

void dm_initialize_txpower_tracking(struct net_device *dev)
{
	struct r8192_priv *priv = rtllib_priv(dev);

	if (priv->IC_Cut >= IC_VersionCut_D)
		dm_InitializeTXPowerTracking_TSSI(dev);
	else
		dm_InitializeTXPowerTracking_ThermalMeter(dev);
}

static void dm_CheckTXPowerTracking_TSSI(struct net_device *dev)
{
	struct r8192_priv *priv = rtllib_priv(dev);
	static u32 tx_power_track_counter;

	RT_TRACE(COMP_POWER_TRACKING, "%s()\n", __func__);
	if (read_nic_byte(dev, 0x11e) == 1)
		return;
	if (!priv->btxpower_tracking)
		return;
	tx_power_track_counter++;


	 if (tx_power_track_counter >= 180) {
		queue_delayed_work_rsl(priv->priv_wq, &priv->txpower_tracking_wq, 0);
		tx_power_track_counter = 0;
	}

}
static void dm_CheckTXPowerTracking_ThermalMeter(struct net_device *dev)
{
	struct r8192_priv *priv = rtllib_priv(dev);
	static u8	TM_Trigger;
	u8		TxPowerCheckCnt = 0;

	if (IS_HARDWARE_TYPE_8192SE(dev))
		TxPowerCheckCnt = 5;
	else
		TxPowerCheckCnt = 2;
	if (!priv->btxpower_tracking)
		return;

	if (priv->txpower_count  <= TxPowerCheckCnt) {
		priv->txpower_count++;
		return;
	}

	if (!TM_Trigger) {
		{
		rtl8192_phy_SetRFReg(dev, RF90_PATH_A, 0x02, bMask12Bits, 0x4d);
		rtl8192_phy_SetRFReg(dev, RF90_PATH_A, 0x02, bMask12Bits, 0x4f);
		rtl8192_phy_SetRFReg(dev, RF90_PATH_A, 0x02, bMask12Bits, 0x4d);
		rtl8192_phy_SetRFReg(dev, RF90_PATH_A, 0x02, bMask12Bits, 0x4f);
		}
		TM_Trigger = 1;
		return;
	}
	netdev_info(dev, "===============>Schedule TxPowerTrackingWorkItem\n");
	queue_delayed_work_rsl(priv->priv_wq, &priv->txpower_tracking_wq, 0);
	TM_Trigger = 0;

}

static void dm_check_txpower_tracking(struct net_device *dev)
{
	struct r8192_priv *priv = rtllib_priv(dev);

	if (priv->IC_Cut >= IC_VersionCut_D)
		dm_CheckTXPowerTracking_TSSI(dev);
	else
		dm_CheckTXPowerTracking_ThermalMeter(dev);
}

static void dm_CCKTxPowerAdjust_TSSI(struct net_device *dev, bool  bInCH14)
{
	u32 TempVal;
	struct r8192_priv *priv = rtllib_priv(dev);

	TempVal = 0;
	if (!bInCH14) {
		TempVal = (u32)(priv->cck_txbbgain_table[(u8)(priv->CCKPresentAttentuation)].ccktxbb_valuearray[0] +
			  (priv->cck_txbbgain_table[(u8)(priv->CCKPresentAttentuation)].ccktxbb_valuearray[1]<<8));

		rtl8192_setBBreg(dev, rCCK0_TxFilter1, bMaskHWord, TempVal);
		TempVal = (u32)(priv->cck_txbbgain_table[(u8)(priv->CCKPresentAttentuation)].ccktxbb_valuearray[2] +
			  (priv->cck_txbbgain_table[(u8)(priv->CCKPresentAttentuation)].ccktxbb_valuearray[3]<<8) +
			  (priv->cck_txbbgain_table[(u8)(priv->CCKPresentAttentuation)].ccktxbb_valuearray[4]<<16)+
			  (priv->cck_txbbgain_table[(u8)(priv->CCKPresentAttentuation)].ccktxbb_valuearray[5]<<24));
		rtl8192_setBBreg(dev, rCCK0_TxFilter2, bMaskDWord, TempVal);
		TempVal = (u32)(priv->cck_txbbgain_table[(u8)(priv->CCKPresentAttentuation)].ccktxbb_valuearray[6] +
			  (priv->cck_txbbgain_table[(u8)(priv->CCKPresentAttentuation)].ccktxbb_valuearray[7]<<8));

		rtl8192_setBBreg(dev, rCCK0_DebugPort, bMaskLWord, TempVal);
	} else {
		TempVal = (u32)(priv->cck_txbbgain_ch14_table[(u8)(priv->CCKPresentAttentuation)].ccktxbb_valuearray[0] +
			  (priv->cck_txbbgain_ch14_table[(u8)(priv->CCKPresentAttentuation)].ccktxbb_valuearray[1]<<8));

		rtl8192_setBBreg(dev, rCCK0_TxFilter1, bMaskHWord, TempVal);
		TempVal = (u32)(priv->cck_txbbgain_ch14_table[(u8)(priv->CCKPresentAttentuation)].ccktxbb_valuearray[2] +
			  (priv->cck_txbbgain_ch14_table[(u8)(priv->CCKPresentAttentuation)].ccktxbb_valuearray[3]<<8) +
			  (priv->cck_txbbgain_ch14_table[(u8)(priv->CCKPresentAttentuation)].ccktxbb_valuearray[4]<<16)+
			  (priv->cck_txbbgain_ch14_table[(u8)(priv->CCKPresentAttentuation)].ccktxbb_valuearray[5]<<24));
		rtl8192_setBBreg(dev, rCCK0_TxFilter2, bMaskDWord, TempVal);
		TempVal = (u32)(priv->cck_txbbgain_ch14_table[(u8)(priv->CCKPresentAttentuation)].ccktxbb_valuearray[6] +
			  (priv->cck_txbbgain_ch14_table[(u8)(priv->CCKPresentAttentuation)].ccktxbb_valuearray[7]<<8));

		rtl8192_setBBreg(dev, rCCK0_DebugPort, bMaskLWord, TempVal);
	}


}

static void dm_CCKTxPowerAdjust_ThermalMeter(struct net_device *dev,	bool  bInCH14)
{
	u32 TempVal;
	struct r8192_priv *priv = rtllib_priv(dev);

	TempVal = 0;
	if (!bInCH14) {
		TempVal =	CCKSwingTable_Ch1_Ch13[priv->CCK_index][0] +
					(CCKSwingTable_Ch1_Ch13[priv->CCK_index][1]<<8);
		rtl8192_setBBreg(dev, rCCK0_TxFilter1, bMaskHWord, TempVal);
		RT_TRACE(COMP_POWER_TRACKING, "CCK not chnl 14, reg 0x%x = 0x%x\n",
			rCCK0_TxFilter1, TempVal);
		TempVal =	CCKSwingTable_Ch1_Ch13[priv->CCK_index][2] +
					(CCKSwingTable_Ch1_Ch13[priv->CCK_index][3]<<8) +
					(CCKSwingTable_Ch1_Ch13[priv->CCK_index][4]<<16)+
					(CCKSwingTable_Ch1_Ch13[priv->CCK_index][5]<<24);
		rtl8192_setBBreg(dev, rCCK0_TxFilter2, bMaskDWord, TempVal);
		RT_TRACE(COMP_POWER_TRACKING, "CCK not chnl 14, reg 0x%x = 0x%x\n",
			rCCK0_TxFilter2, TempVal);
		TempVal =	CCKSwingTable_Ch1_Ch13[priv->CCK_index][6] +
					(CCKSwingTable_Ch1_Ch13[priv->CCK_index][7]<<8);

		rtl8192_setBBreg(dev, rCCK0_DebugPort, bMaskLWord, TempVal);
		RT_TRACE(COMP_POWER_TRACKING, "CCK not chnl 14, reg 0x%x = 0x%x\n",
			rCCK0_DebugPort, TempVal);
	} else {
		TempVal =	CCKSwingTable_Ch14[priv->CCK_index][0] +
					(CCKSwingTable_Ch14[priv->CCK_index][1]<<8);

		rtl8192_setBBreg(dev, rCCK0_TxFilter1, bMaskHWord, TempVal);
		RT_TRACE(COMP_POWER_TRACKING, "CCK chnl 14, reg 0x%x = 0x%x\n",
			rCCK0_TxFilter1, TempVal);
		TempVal =	CCKSwingTable_Ch14[priv->CCK_index][2] +
					(CCKSwingTable_Ch14[priv->CCK_index][3]<<8) +
					(CCKSwingTable_Ch14[priv->CCK_index][4]<<16)+
					(CCKSwingTable_Ch14[priv->CCK_index][5]<<24);
		rtl8192_setBBreg(dev, rCCK0_TxFilter2, bMaskDWord, TempVal);
		RT_TRACE(COMP_POWER_TRACKING, "CCK chnl 14, reg 0x%x = 0x%x\n",
			rCCK0_TxFilter2, TempVal);
		TempVal =	CCKSwingTable_Ch14[priv->CCK_index][6] +
					(CCKSwingTable_Ch14[priv->CCK_index][7]<<8);

		rtl8192_setBBreg(dev, rCCK0_DebugPort, bMaskLWord, TempVal);
		RT_TRACE(COMP_POWER_TRACKING, "CCK chnl 14, reg 0x%x = 0x%x\n",
			rCCK0_DebugPort, TempVal);
	}
	}

void dm_cck_txpower_adjust(struct net_device *dev, bool  binch14)
{
	struct r8192_priv *priv = rtllib_priv(dev);

	if (priv->IC_Cut >= IC_VersionCut_D)
		dm_CCKTxPowerAdjust_TSSI(dev, binch14);
	else
		dm_CCKTxPowerAdjust_ThermalMeter(dev, binch14);
}

static void dm_txpower_reset_recovery(struct net_device *dev)
{
	struct r8192_priv *priv = rtllib_priv(dev);

	RT_TRACE(COMP_POWER_TRACKING, "Start Reset Recovery ==>\n");
	rtl8192_setBBreg(dev, rOFDM0_XATxIQImbalance, bMaskDWord,
			 priv->txbbgain_table[priv->rfa_txpowertrackingindex].txbbgain_value);
	RT_TRACE(COMP_POWER_TRACKING, "Reset Recovery: Fill in 0xc80 is %08x\n",
		 priv->txbbgain_table[priv->rfa_txpowertrackingindex].txbbgain_value);
	RT_TRACE(COMP_POWER_TRACKING, "Reset Recovery: Fill in RFA_txPowerTrackingIndex is %x\n",
		 priv->rfa_txpowertrackingindex);
	RT_TRACE(COMP_POWER_TRACKING, "Reset Recovery : RF A I/Q Amplify Gain is %ld\n",
		 priv->txbbgain_table[priv->rfa_txpowertrackingindex].txbb_iq_amplifygain);
	RT_TRACE(COMP_POWER_TRACKING, "Reset Recovery: CCK Attenuation is %d dB\n",
		 priv->CCKPresentAttentuation);
	dm_cck_txpower_adjust(dev, priv->bcck_in_ch14);

	rtl8192_setBBreg(dev, rOFDM0_XCTxIQImbalance, bMaskDWord,
			 priv->txbbgain_table[priv->rfc_txpowertrackingindex].txbbgain_value);
	RT_TRACE(COMP_POWER_TRACKING, "Reset Recovery: Fill in 0xc90 is %08x\n",
		 priv->txbbgain_table[priv->rfc_txpowertrackingindex].txbbgain_value);
	RT_TRACE(COMP_POWER_TRACKING, "Reset Recovery: Fill in RFC_txPowerTrackingIndex is %x\n",
		 priv->rfc_txpowertrackingindex);
	RT_TRACE(COMP_POWER_TRACKING, "Reset Recovery : RF C I/Q Amplify Gain is %ld\n",
		 priv->txbbgain_table[priv->rfc_txpowertrackingindex].txbb_iq_amplifygain);

}

void dm_restore_dynamic_mechanism_state(struct net_device *dev)
{
	struct r8192_priv *priv = rtllib_priv(dev);
	u32	reg_ratr = priv->rate_adaptive.last_ratr;
	u32 ratr_value;

	if (!priv->up) {
		RT_TRACE(COMP_RATE, "<---- dm_restore_dynamic_mechanism_state(): driver is going to unload\n");
		return;
	}

	if (priv->rate_adaptive.rate_adaptive_disabled)
		return;
	if (!(priv->rtllib->mode == WIRELESS_MODE_N_24G ||
	      priv->rtllib->mode == WIRELESS_MODE_N_5G))
		return;
	ratr_value = reg_ratr;
	if (priv->rf_type == RF_1T2R)
		ratr_value &= ~(RATE_ALL_OFDM_2SS);
	write_nic_dword(dev, RATR0, ratr_value);
	write_nic_byte(dev, UFWP, 1);
	if (priv->btxpower_trackingInit && priv->btxpower_tracking)
		dm_txpower_reset_recovery(dev);

	dm_bb_initialgain_restore(dev);

}

static void dm_bb_initialgain_restore(struct net_device *dev)
{
	struct r8192_priv *priv = rtllib_priv(dev);
	u32 bit_mask = 0x7f;

	if (dm_digtable.dig_algorithm == DIG_ALGO_BY_RSSI)
		return;

	rtl8192_setBBreg(dev, UFWP, bMaskByte1, 0x8);
	rtl8192_setBBreg(dev, rOFDM0_XAAGCCore1, bit_mask, (u32)priv->initgain_backup.xaagccore1);
	rtl8192_setBBreg(dev, rOFDM0_XBAGCCore1, bit_mask, (u32)priv->initgain_backup.xbagccore1);
	rtl8192_setBBreg(dev, rOFDM0_XCAGCCore1, bit_mask, (u32)priv->initgain_backup.xcagccore1);
	rtl8192_setBBreg(dev, rOFDM0_XDAGCCore1, bit_mask, (u32)priv->initgain_backup.xdagccore1);
	bit_mask  = bMaskByte2;
	rtl8192_setBBreg(dev, rCCK0_CCA, bit_mask, (u32)priv->initgain_backup.cca);

	RT_TRACE(COMP_DIG, "dm_BBInitialGainRestore 0xc50 is %x\n", priv->initgain_backup.xaagccore1);
	RT_TRACE(COMP_DIG, "dm_BBInitialGainRestore 0xc58 is %x\n", priv->initgain_backup.xbagccore1);
	RT_TRACE(COMP_DIG, "dm_BBInitialGainRestore 0xc60 is %x\n", priv->initgain_backup.xcagccore1);
	RT_TRACE(COMP_DIG, "dm_BBInitialGainRestore 0xc68 is %x\n", priv->initgain_backup.xdagccore1);
	RT_TRACE(COMP_DIG, "dm_BBInitialGainRestore 0xa0a is %x\n", priv->initgain_backup.cca);
	rtl8192_setBBreg(dev, UFWP, bMaskByte1, 0x1);

}


void dm_backup_dynamic_mechanism_state(struct net_device *dev)
{
	struct r8192_priv *priv = rtllib_priv(dev);

	priv->bswitch_fsync  = false;
	priv->bfsync_processing = false;
	dm_bb_initialgain_backup(dev);

}


static void dm_bb_initialgain_backup(struct net_device *dev)
{
	struct r8192_priv *priv = rtllib_priv(dev);
	u32 bit_mask = bMaskByte0;

	if (dm_digtable.dig_algorithm == DIG_ALGO_BY_RSSI)
		return;

	rtl8192_setBBreg(dev, UFWP, bMaskByte1, 0x8);
	priv->initgain_backup.xaagccore1 = (u8)rtl8192_QueryBBReg(dev, rOFDM0_XAAGCCore1, bit_mask);
	priv->initgain_backup.xbagccore1 = (u8)rtl8192_QueryBBReg(dev, rOFDM0_XBAGCCore1, bit_mask);
	priv->initgain_backup.xcagccore1 = (u8)rtl8192_QueryBBReg(dev, rOFDM0_XCAGCCore1, bit_mask);
	priv->initgain_backup.xdagccore1 = (u8)rtl8192_QueryBBReg(dev, rOFDM0_XDAGCCore1, bit_mask);
	bit_mask  = bMaskByte2;
	priv->initgain_backup.cca = (u8)rtl8192_QueryBBReg(dev, rCCK0_CCA, bit_mask);

	RT_TRACE(COMP_DIG, "BBInitialGainBackup 0xc50 is %x\n", priv->initgain_backup.xaagccore1);
	RT_TRACE(COMP_DIG, "BBInitialGainBackup 0xc58 is %x\n", priv->initgain_backup.xbagccore1);
	RT_TRACE(COMP_DIG, "BBInitialGainBackup 0xc60 is %x\n", priv->initgain_backup.xcagccore1);
	RT_TRACE(COMP_DIG, "BBInitialGainBackup 0xc68 is %x\n", priv->initgain_backup.xdagccore1);
	RT_TRACE(COMP_DIG, "BBInitialGainBackup 0xa0a is %x\n", priv->initgain_backup.cca);

}

void dm_change_dynamic_initgain_thresh(struct net_device *dev,
				       u32 dm_type, u32 dm_value)
{
	if (dm_type == DIG_TYPE_THRESH_HIGH) {
		dm_digtable.rssi_high_thresh = dm_value;
	} else if (dm_type == DIG_TYPE_THRESH_LOW) {
		dm_digtable.rssi_low_thresh = dm_value;
	} else if (dm_type == DIG_TYPE_THRESH_HIGHPWR_HIGH) {
		dm_digtable.rssi_high_power_highthresh = dm_value;
	} else if (dm_type == DIG_TYPE_THRESH_HIGHPWR_LOW) {
		dm_digtable.rssi_high_power_lowthresh = dm_value;
	} else if (dm_type == DIG_TYPE_ENABLE) {
		dm_digtable.dig_state		= DM_STA_DIG_MAX;
		dm_digtable.dig_enable_flag	= true;
	} else if (dm_type == DIG_TYPE_DISABLE) {
		dm_digtable.dig_state		= DM_STA_DIG_MAX;
		dm_digtable.dig_enable_flag	= false;
	} else if (dm_type == DIG_TYPE_DBG_MODE) {
		if (dm_value >= DM_DBG_MAX)
			dm_value = DM_DBG_OFF;
		dm_digtable.dbg_mode		= (u8)dm_value;
	} else if (dm_type == DIG_TYPE_RSSI) {
		if (dm_value > 100)
			dm_value = 30;
		dm_digtable.rssi_val			= (long)dm_value;
	} else if (dm_type == DIG_TYPE_ALGORITHM) {
		if (dm_value >= DIG_ALGO_MAX)
			dm_value = DIG_ALGO_BY_FALSE_ALARM;
		if (dm_digtable.dig_algorithm != (u8)dm_value)
			dm_digtable.dig_algorithm_switch = 1;
		dm_digtable.dig_algorithm	= (u8)dm_value;
	} else if (dm_type == DIG_TYPE_BACKOFF) {
		if (dm_value > 30)
			dm_value = 30;
		dm_digtable.backoff_val		= (u8)dm_value;
	} else if (dm_type == DIG_TYPE_RX_GAIN_MIN) {
		if (dm_value == 0)
			dm_value = 0x1;
		dm_digtable.rx_gain_range_min = (u8)dm_value;
	} else if (dm_type == DIG_TYPE_RX_GAIN_MAX) {
		if (dm_value > 0x50)
			dm_value = 0x50;
		dm_digtable.rx_gain_range_max = (u8)dm_value;
	}
}

static void dm_dig_init(struct net_device *dev)
{
	struct r8192_priv *priv = rtllib_priv(dev);

	dm_digtable.dig_enable_flag	= true;
	dm_digtable.Backoff_Enable_Flag = true;

	dm_digtable.dig_algorithm = DIG_ALGO_BY_RSSI;

	dm_digtable.Dig_TwoPort_Algorithm = DIG_TWO_PORT_ALGO_RSSI;
	dm_digtable.Dig_Ext_Port_Stage = DIG_EXT_PORT_STAGE_MAX;
	dm_digtable.dbg_mode = DM_DBG_OFF;
	dm_digtable.dig_algorithm_switch = 0;

	dm_digtable.dig_state		= DM_STA_DIG_MAX;
	dm_digtable.dig_highpwr_state	= DM_STA_DIG_MAX;
	dm_digtable.CurSTAConnectState = dm_digtable.PreSTAConnectState = DIG_STA_DISCONNECT;
	dm_digtable.CurAPConnectState = dm_digtable.PreAPConnectState = DIG_AP_DISCONNECT;
	dm_digtable.initialgain_lowerbound_state = false;

	dm_digtable.rssi_low_thresh	= DM_DIG_THRESH_LOW;
	dm_digtable.rssi_high_thresh	= DM_DIG_THRESH_HIGH;

	dm_digtable.FALowThresh	= DM_FALSEALARM_THRESH_LOW;
	dm_digtable.FAHighThresh	= DM_FALSEALARM_THRESH_HIGH;

	dm_digtable.rssi_high_power_lowthresh = DM_DIG_HIGH_PWR_THRESH_LOW;
	dm_digtable.rssi_high_power_highthresh = DM_DIG_HIGH_PWR_THRESH_HIGH;

	dm_digtable.rssi_val = 50;
	dm_digtable.backoff_val = DM_DIG_BACKOFF;
	dm_digtable.rx_gain_range_max = DM_DIG_MAX;
	if (priv->CustomerID == RT_CID_819x_Netcore)
		dm_digtable.rx_gain_range_min = DM_DIG_MIN_Netcore;
	else
		dm_digtable.rx_gain_range_min = DM_DIG_MIN;

	dm_digtable.BackoffVal_range_max = DM_DIG_BACKOFF_MAX;
	dm_digtable.BackoffVal_range_min = DM_DIG_BACKOFF_MIN;
}

static void dm_ctrl_initgain_byrssi(struct net_device *dev)
{

	if (dm_digtable.dig_enable_flag == false)
		return;

	if (dm_digtable.dig_algorithm == DIG_ALGO_BY_FALSE_ALARM)
		dm_ctrl_initgain_byrssi_by_fwfalse_alarm(dev);
	else if (dm_digtable.dig_algorithm == DIG_ALGO_BY_RSSI)
		dm_ctrl_initgain_byrssi_by_driverrssi(dev);
	else
		return;
}

/*-----------------------------------------------------------------------------
 * Function:	dm_CtrlInitGainBeforeConnectByRssiAndFalseAlarm()
 *
 * Overview:	Driver monitor RSSI and False Alarm to change initial gain.
			Only change initial gain during link in progress.
 *
 * Input:		IN	PADAPTER	pAdapter
 *
 * Output:		NONE
 *
 * Return:		NONE
 *
 * Revised History:
 *	When		Who		Remark
 *	03/04/2009	hpfan	Create Version 0.
 *
 *---------------------------------------------------------------------------*/

static void dm_ctrl_initgain_byrssi_by_driverrssi(
	struct net_device *dev)
{
	struct r8192_priv *priv = rtllib_priv(dev);
	u8 i;
	static u8	fw_dig;

	if (dm_digtable.dig_enable_flag == false)
		return;

	if (dm_digtable.dig_algorithm_switch)
		fw_dig = 0;
	if (fw_dig <= 3) {
		for (i = 0; i < 3; i++)
			rtl8192_setBBreg(dev, UFWP, bMaskByte1, 0x8);
		fw_dig++;
		dm_digtable.dig_state = DM_STA_DIG_OFF;
	}

	if (priv->rtllib->state == RTLLIB_LINKED)
		dm_digtable.CurSTAConnectState = DIG_STA_CONNECT;
	else
		dm_digtable.CurSTAConnectState = DIG_STA_DISCONNECT;


	if (dm_digtable.dbg_mode == DM_DBG_OFF)
		dm_digtable.rssi_val = priv->undecorated_smoothed_pwdb;
	dm_initial_gain(dev);
	dm_pd_th(dev);
	dm_cs_ratio(dev);
	if (dm_digtable.dig_algorithm_switch)
		dm_digtable.dig_algorithm_switch = 0;
	dm_digtable.PreSTAConnectState = dm_digtable.CurSTAConnectState;

}

static void dm_ctrl_initgain_byrssi_by_fwfalse_alarm(
	struct net_device *dev)
{
	struct r8192_priv *priv = rtllib_priv(dev);
	static u32 reset_cnt;
	u8 i;

	if (dm_digtable.dig_enable_flag == false)
		return;

	if (dm_digtable.dig_algorithm_switch) {
		dm_digtable.dig_state = DM_STA_DIG_MAX;
		for (i = 0; i < 3; i++)
			rtl8192_setBBreg(dev, UFWP, bMaskByte1, 0x1);
		dm_digtable.dig_algorithm_switch = 0;
	}

	if (priv->rtllib->state != RTLLIB_LINKED)
		return;

	if ((priv->undecorated_smoothed_pwdb > dm_digtable.rssi_low_thresh) &&
		(priv->undecorated_smoothed_pwdb < dm_digtable.rssi_high_thresh))
		return;
	if (priv->undecorated_smoothed_pwdb <= dm_digtable.rssi_low_thresh) {
		if (dm_digtable.dig_state == DM_STA_DIG_OFF &&
			(priv->reset_count == reset_cnt))
			return;
		reset_cnt = priv->reset_count;

		dm_digtable.dig_highpwr_state = DM_STA_DIG_MAX;
		dm_digtable.dig_state = DM_STA_DIG_OFF;

		rtl8192_setBBreg(dev, UFWP, bMaskByte1, 0x8);

		write_nic_byte(dev, rOFDM0_XAAGCCore1, 0x17);
		write_nic_byte(dev, rOFDM0_XBAGCCore1, 0x17);
		write_nic_byte(dev, rOFDM0_XCAGCCore1, 0x17);
		write_nic_byte(dev, rOFDM0_XDAGCCore1, 0x17);

		if (priv->CurrentChannelBW != HT_CHANNEL_WIDTH_20)
			write_nic_byte(dev, (rOFDM0_XATxAFE+3), 0x00);
		else
			write_nic_byte(dev, rOFDM0_RxDetector1, 0x42);

		write_nic_byte(dev, 0xa0a, 0x08);

		return;
	}

	if (priv->undecorated_smoothed_pwdb >= dm_digtable.rssi_high_thresh) {
		u8 reset_flag = 0;

		if (dm_digtable.dig_state == DM_STA_DIG_ON &&
		    (priv->reset_count == reset_cnt)) {
			dm_ctrl_initgain_byrssi_highpwr(dev);
			return;
		}
		if (priv->reset_count != reset_cnt)
			reset_flag = 1;

		reset_cnt = priv->reset_count;

		dm_digtable.dig_state = DM_STA_DIG_ON;

		if (reset_flag == 1) {
			write_nic_byte(dev, rOFDM0_XAAGCCore1, 0x2c);
			write_nic_byte(dev, rOFDM0_XBAGCCore1, 0x2c);
			write_nic_byte(dev, rOFDM0_XCAGCCore1, 0x2c);
			write_nic_byte(dev, rOFDM0_XDAGCCore1, 0x2c);
		} else {
			write_nic_byte(dev, rOFDM0_XAAGCCore1, 0x20);
			write_nic_byte(dev, rOFDM0_XBAGCCore1, 0x20);
			write_nic_byte(dev, rOFDM0_XCAGCCore1, 0x20);
			write_nic_byte(dev, rOFDM0_XDAGCCore1, 0x20);
		}

		if (priv->CurrentChannelBW != HT_CHANNEL_WIDTH_20)
			write_nic_byte(dev, (rOFDM0_XATxAFE+3), 0x20);
		else
			write_nic_byte(dev, rOFDM0_RxDetector1, 0x44);

		write_nic_byte(dev, 0xa0a, 0xcd);

		rtl8192_setBBreg(dev, UFWP, bMaskByte1, 0x1);
	}
	dm_ctrl_initgain_byrssi_highpwr(dev);
}


static void dm_ctrl_initgain_byrssi_highpwr(struct net_device *dev)
{
	struct r8192_priv *priv = rtllib_priv(dev);
	static u32 reset_cnt_highpwr;

	if ((priv->undecorated_smoothed_pwdb > dm_digtable.rssi_high_power_lowthresh) &&
		(priv->undecorated_smoothed_pwdb < dm_digtable.rssi_high_power_highthresh))
		return;

	if (priv->undecorated_smoothed_pwdb >= dm_digtable.rssi_high_power_highthresh) {
		if (dm_digtable.dig_highpwr_state == DM_STA_DIG_ON &&
			(priv->reset_count == reset_cnt_highpwr))
			return;
		dm_digtable.dig_highpwr_state = DM_STA_DIG_ON;

		if (priv->CurrentChannelBW != HT_CHANNEL_WIDTH_20)
				write_nic_byte(dev, (rOFDM0_XATxAFE+3), 0x10);
		else
			write_nic_byte(dev, rOFDM0_RxDetector1, 0x43);
	} else {
		if (dm_digtable.dig_highpwr_state == DM_STA_DIG_OFF &&
			(priv->reset_count == reset_cnt_highpwr))
			return;
		dm_digtable.dig_highpwr_state = DM_STA_DIG_OFF;

		if (priv->undecorated_smoothed_pwdb < dm_digtable.rssi_high_power_lowthresh &&
			 priv->undecorated_smoothed_pwdb >= dm_digtable.rssi_high_thresh) {
			if (priv->CurrentChannelBW != HT_CHANNEL_WIDTH_20)
				write_nic_byte(dev, (rOFDM0_XATxAFE+3), 0x20);
			else
				write_nic_byte(dev, rOFDM0_RxDetector1, 0x44);
		}
	}
	reset_cnt_highpwr = priv->reset_count;
}

static void dm_initial_gain(struct net_device *dev)
{
	struct r8192_priv *priv = rtllib_priv(dev);
	u8 initial_gain = 0;
	static u8 initialized, force_write;
	static u32 reset_cnt;

	if (dm_digtable.dig_algorithm_switch) {
		initialized = 0;
		reset_cnt = 0;
	}

	if (rtllib_act_scanning(priv->rtllib, true) == true) {
		force_write = 1;
		return;
	}

	if (dm_digtable.PreSTAConnectState == dm_digtable.CurSTAConnectState) {
		if (dm_digtable.CurSTAConnectState == DIG_STA_CONNECT) {
			if ((dm_digtable.rssi_val+10-dm_digtable.backoff_val) > dm_digtable.rx_gain_range_max)
				dm_digtable.cur_ig_value = dm_digtable.rx_gain_range_max;
			else if ((dm_digtable.rssi_val+10-dm_digtable.backoff_val) < dm_digtable.rx_gain_range_min)
				dm_digtable.cur_ig_value = dm_digtable.rx_gain_range_min;
			else
				dm_digtable.cur_ig_value = dm_digtable.rssi_val+10-dm_digtable.backoff_val;
		} else {
			if (dm_digtable.cur_ig_value == 0)
				dm_digtable.cur_ig_value = priv->DefaultInitialGain[0];
			else
				dm_digtable.cur_ig_value = dm_digtable.pre_ig_value;
		}
	} else {
		dm_digtable.cur_ig_value = priv->DefaultInitialGain[0];
		dm_digtable.pre_ig_value = 0;
	}

	if (priv->reset_count != reset_cnt) {
		force_write = 1;
		reset_cnt = priv->reset_count;
	}

	if (dm_digtable.pre_ig_value != read_nic_byte(dev, rOFDM0_XAAGCCore1))
		force_write = 1;

	if ((dm_digtable.pre_ig_value != dm_digtable.cur_ig_value)
	    || !initialized || force_write) {
		initial_gain = (u8)dm_digtable.cur_ig_value;
		write_nic_byte(dev, rOFDM0_XAAGCCore1, initial_gain);
		write_nic_byte(dev, rOFDM0_XBAGCCore1, initial_gain);
		write_nic_byte(dev, rOFDM0_XCAGCCore1, initial_gain);
		write_nic_byte(dev, rOFDM0_XDAGCCore1, initial_gain);
		dm_digtable.pre_ig_value = dm_digtable.cur_ig_value;
		initialized = 1;
		force_write = 0;
	}
}

static void dm_pd_th(struct net_device *dev)
{
	struct r8192_priv *priv = rtllib_priv(dev);
	static u8 initialized, force_write;
	static u32 reset_cnt;

	if (dm_digtable.dig_algorithm_switch) {
		initialized = 0;
		reset_cnt = 0;
	}

	if (dm_digtable.PreSTAConnectState == dm_digtable.CurSTAConnectState) {
		if (dm_digtable.CurSTAConnectState == DIG_STA_CONNECT) {
			if (dm_digtable.rssi_val >= dm_digtable.rssi_high_power_highthresh)
				dm_digtable.curpd_thstate = DIG_PD_AT_HIGH_POWER;
			else if (dm_digtable.rssi_val <= dm_digtable.rssi_low_thresh)
				dm_digtable.curpd_thstate = DIG_PD_AT_LOW_POWER;
			else if ((dm_digtable.rssi_val >= dm_digtable.rssi_high_thresh) &&
					(dm_digtable.rssi_val < dm_digtable.rssi_high_power_lowthresh))
				dm_digtable.curpd_thstate = DIG_PD_AT_NORMAL_POWER;
			else
				dm_digtable.curpd_thstate = dm_digtable.prepd_thstate;
		} else {
			dm_digtable.curpd_thstate = DIG_PD_AT_LOW_POWER;
		}
	} else {
		dm_digtable.curpd_thstate = DIG_PD_AT_LOW_POWER;
	}

	if (priv->reset_count != reset_cnt) {
		force_write = 1;
		reset_cnt = priv->reset_count;
	}

	if ((dm_digtable.prepd_thstate != dm_digtable.curpd_thstate) ||
	    (initialized <= 3) || force_write) {
		if (dm_digtable.curpd_thstate == DIG_PD_AT_LOW_POWER) {
			if (priv->CurrentChannelBW != HT_CHANNEL_WIDTH_20)
				write_nic_byte(dev, (rOFDM0_XATxAFE+3), 0x00);
			else
				write_nic_byte(dev, rOFDM0_RxDetector1, 0x42);
		} else if (dm_digtable.curpd_thstate == DIG_PD_AT_NORMAL_POWER) {
			if (priv->CurrentChannelBW != HT_CHANNEL_WIDTH_20)
				write_nic_byte(dev, (rOFDM0_XATxAFE+3), 0x20);
			else
				write_nic_byte(dev, rOFDM0_RxDetector1, 0x44);
		} else if (dm_digtable.curpd_thstate == DIG_PD_AT_HIGH_POWER) {
			if (priv->CurrentChannelBW != HT_CHANNEL_WIDTH_20)
				write_nic_byte(dev, (rOFDM0_XATxAFE+3), 0x10);
			else
				write_nic_byte(dev, rOFDM0_RxDetector1, 0x43);
		}
		dm_digtable.prepd_thstate = dm_digtable.curpd_thstate;
		if (initialized <= 3)
			initialized++;
		force_write = 0;
	}
}

static	void dm_cs_ratio(struct net_device *dev)
{
	struct r8192_priv *priv = rtllib_priv(dev);
	static u8 initialized, force_write;
	static u32 reset_cnt;

	if (dm_digtable.dig_algorithm_switch) {
		initialized = 0;
		reset_cnt = 0;
	}

	if (dm_digtable.PreSTAConnectState == dm_digtable.CurSTAConnectState) {
		if (dm_digtable.CurSTAConnectState == DIG_STA_CONNECT) {
			if (dm_digtable.rssi_val <= dm_digtable.rssi_low_thresh)
				dm_digtable.curcs_ratio_state = DIG_CS_RATIO_LOWER;
			else if (dm_digtable.rssi_val >= dm_digtable.rssi_high_thresh)
				dm_digtable.curcs_ratio_state = DIG_CS_RATIO_HIGHER;
			else
				dm_digtable.curcs_ratio_state = dm_digtable.precs_ratio_state;
		} else {
			dm_digtable.curcs_ratio_state = DIG_CS_RATIO_LOWER;
		}
	} else {
		dm_digtable.curcs_ratio_state = DIG_CS_RATIO_LOWER;
	}

	if (priv->reset_count != reset_cnt) {
		force_write = 1;
		reset_cnt = priv->reset_count;
	}


	if ((dm_digtable.precs_ratio_state != dm_digtable.curcs_ratio_state) ||
	    !initialized || force_write) {
		if (dm_digtable.curcs_ratio_state == DIG_CS_RATIO_LOWER)
			write_nic_byte(dev, 0xa0a, 0x08);
		else if (dm_digtable.curcs_ratio_state == DIG_CS_RATIO_HIGHER)
			write_nic_byte(dev, 0xa0a, 0xcd);
		dm_digtable.precs_ratio_state = dm_digtable.curcs_ratio_state;
		initialized = 1;
		force_write = 0;
	}
}

void dm_init_edca_turbo(struct net_device *dev)
{
	struct r8192_priv *priv = rtllib_priv(dev);

	priv->bcurrent_turbo_EDCA = false;
	priv->rtllib->bis_any_nonbepkts = false;
	priv->bis_cur_rdlstate = false;
}

static void dm_check_edca_turbo(struct net_device *dev)
{
	struct r8192_priv *priv = rtllib_priv(dev);
	struct rt_hi_throughput *pHTInfo = priv->rtllib->pHTInfo;

	static unsigned long lastTxOkCnt;
	static unsigned long lastRxOkCnt;
	unsigned long curTxOkCnt = 0;
	unsigned long curRxOkCnt = 0;

	if (priv->rtllib->iw_mode == IW_MODE_ADHOC)
		goto dm_CheckEdcaTurbo_EXIT;
	if (priv->rtllib->state != RTLLIB_LINKED)
		goto dm_CheckEdcaTurbo_EXIT;
	if (priv->rtllib->pHTInfo->IOTAction & HT_IOT_ACT_DISABLE_EDCA_TURBO)
		goto dm_CheckEdcaTurbo_EXIT;

	{
		u8 *peername[11] = {
			"unknown", "realtek_90", "realtek_92se", "broadcom",
			"ralink", "atheros", "cisco", "marvell", "92u_softap",
			"self_softap"
		};
		static int wb_tmp;

		if (wb_tmp == 0) {
			netdev_info(dev,
				    "%s():iot peer is %s, bssid: %pM\n",
				    __func__, peername[pHTInfo->IOTPeer],
				    priv->rtllib->current_network.bssid);
			wb_tmp = 1;
		}
	}
	if (!priv->rtllib->bis_any_nonbepkts) {
		curTxOkCnt = priv->stats.txbytesunicast - lastTxOkCnt;
		curRxOkCnt = priv->stats.rxbytesunicast - lastRxOkCnt;
		if (pHTInfo->IOTAction & HT_IOT_ACT_EDCA_BIAS_ON_RX) {
			if (curTxOkCnt > 4*curRxOkCnt) {
				if (priv->bis_cur_rdlstate ||
				    !priv->bcurrent_turbo_EDCA) {
					write_nic_dword(dev, EDCAPARA_BE,
						 edca_setting_UL[pHTInfo->IOTPeer]);
					priv->bis_cur_rdlstate = false;
				}
			} else {
				if (!priv->bis_cur_rdlstate ||
				    !priv->bcurrent_turbo_EDCA) {
					if (priv->rtllib->mode == WIRELESS_MODE_G)
						write_nic_dword(dev, EDCAPARA_BE,
							 edca_setting_DL_GMode[pHTInfo->IOTPeer]);
					else
						write_nic_dword(dev, EDCAPARA_BE,
							 edca_setting_DL[pHTInfo->IOTPeer]);
					priv->bis_cur_rdlstate = true;
				}
			}
			priv->bcurrent_turbo_EDCA = true;
		} else {
			if (curRxOkCnt > 4*curTxOkCnt) {
				if (!priv->bis_cur_rdlstate || !priv->bcurrent_turbo_EDCA) {
					if (priv->rtllib->mode == WIRELESS_MODE_G)
						write_nic_dword(dev, EDCAPARA_BE,
							 edca_setting_DL_GMode[pHTInfo->IOTPeer]);
					else
						write_nic_dword(dev, EDCAPARA_BE,
							 edca_setting_DL[pHTInfo->IOTPeer]);
					priv->bis_cur_rdlstate = true;
				}
			} else {
				if (priv->bis_cur_rdlstate ||
				    !priv->bcurrent_turbo_EDCA) {
					write_nic_dword(dev, EDCAPARA_BE,
							edca_setting_UL[pHTInfo->IOTPeer]);
					priv->bis_cur_rdlstate = false;
				}

			}

			priv->bcurrent_turbo_EDCA = true;
		}
	} else {
		 if (priv->bcurrent_turbo_EDCA) {
			u8 tmp = AC0_BE;

			priv->rtllib->SetHwRegHandler(dev, HW_VAR_AC_PARAM, (u8 *)(&tmp));
			priv->bcurrent_turbo_EDCA = false;
		}
	}


dm_CheckEdcaTurbo_EXIT:
	priv->rtllib->bis_any_nonbepkts = false;
	lastTxOkCnt = priv->stats.txbytesunicast;
	lastRxOkCnt = priv->stats.rxbytesunicast;
}

static void dm_init_ctstoself(struct net_device *dev)
{
	struct r8192_priv *priv = rtllib_priv((struct net_device *)dev);

	priv->rtllib->bCTSToSelfEnable = true;
	priv->rtllib->CTSToSelfTH = CTSToSelfTHVal;
}

static void dm_ctstoself(struct net_device *dev)
{
	struct r8192_priv *priv = rtllib_priv((struct net_device *)dev);
	struct rt_hi_throughput *pHTInfo = priv->rtllib->pHTInfo;
	static unsigned long lastTxOkCnt;
	static unsigned long lastRxOkCnt;
	unsigned long curTxOkCnt = 0;
	unsigned long curRxOkCnt = 0;

	if (priv->rtllib->bCTSToSelfEnable != true) {
		pHTInfo->IOTAction &= ~HT_IOT_ACT_FORCED_CTS2SELF;
		return;
	}
	if (pHTInfo->IOTPeer == HT_IOT_PEER_BROADCOM) {
		curTxOkCnt = priv->stats.txbytesunicast - lastTxOkCnt;
		curRxOkCnt = priv->stats.rxbytesunicast - lastRxOkCnt;
		if (curRxOkCnt > 4*curTxOkCnt)
			pHTInfo->IOTAction &= ~HT_IOT_ACT_FORCED_CTS2SELF;
		else
			pHTInfo->IOTAction |= HT_IOT_ACT_FORCED_CTS2SELF;

		lastTxOkCnt = priv->stats.txbytesunicast;
		lastRxOkCnt = priv->stats.rxbytesunicast;
	}
}


static	void dm_Init_WA_Broadcom_IOT(struct net_device *dev)
{
	struct r8192_priv *priv = rtllib_priv((struct net_device *)dev);
	struct rt_hi_throughput *pHTInfo = priv->rtllib->pHTInfo;

	pHTInfo->bWAIotBroadcom = false;
	pHTInfo->WAIotTH = WAIotTHVal;
}

static	void	dm_check_pbc_gpio(struct net_device *dev)
{
}

void dm_CheckRfCtrlGPIO(void *data)
{
	struct r8192_priv *priv = container_of_dwork_rsl(data,
				  struct r8192_priv, gpio_change_rf_wq);
	struct net_device *dev = priv->rtllib->dev;
	u8 tmp1byte;
	enum rt_rf_power_state eRfPowerStateToSet;
	bool bActuallySet = false;
	char *argv[3];
	static char *RadioPowerPath = "/etc/acpi/events/RadioPower.sh";
	static char *envp[] = {"HOME=/", "TERM=linux", "PATH=/usr/bin:/bin", NULL};

	bActuallySet = false;

	if ((priv->up_first_time == 1) || (priv->being_init_adapter))
		return;

	if (priv->bfirst_after_down) {
		priv->bfirst_after_down = true;
		return;
	}

	tmp1byte = read_nic_byte(dev, GPI);

	eRfPowerStateToSet = (tmp1byte&BIT1) ?  eRfOn : eRfOff;

	if (priv->bHwRadioOff && (eRfPowerStateToSet == eRfOn)) {
		RT_TRACE(COMP_RF, "gpiochangeRF  - HW Radio ON\n");
		netdev_info(dev, "gpiochangeRF  - HW Radio ON\n");
		priv->bHwRadioOff = false;
		bActuallySet = true;
	} else if (!priv->bHwRadioOff && (eRfPowerStateToSet == eRfOff)) {
		RT_TRACE(COMP_RF, "gpiochangeRF  - HW Radio OFF\n");
		netdev_info(dev, "gpiochangeRF  - HW Radio OFF\n");
		priv->bHwRadioOff = true;
		bActuallySet = true;
	}

	if (bActuallySet) {
		mdelay(1000);
		priv->bHwRfOffAction = 1;
		MgntActSet_RF_State(dev, eRfPowerStateToSet, RF_CHANGE_BY_HW, true);
		if (priv->bHwRadioOff)
			argv[1] = "RFOFF";
		else
			argv[1] = "RFON";

		argv[0] = RadioPowerPath;
		argv[2] = NULL;
		call_usermodehelper(RadioPowerPath, argv, envp, UMH_WAIT_PROC);
	}
}

void	dm_rf_pathcheck_workitemcallback(void *data)
{
	struct r8192_priv *priv = container_of_dwork_rsl(data,
				  struct r8192_priv,
				  rfpath_check_wq);
	struct net_device *dev = priv->rtllib->dev;
	u8 rfpath = 0, i;

	rfpath = read_nic_byte(dev, 0xc04);

	for (i = 0; i < RF90_PATH_MAX; i++) {
		if (rfpath & (0x01<<i))
			priv->brfpath_rxenable[i] = true;
		else
			priv->brfpath_rxenable[i] = false;
	}
	if (!DM_RxPathSelTable.Enable)
		return;

	dm_rxpath_sel_byrssi(dev);
}

static void dm_init_rxpath_selection(struct net_device *dev)
{
	u8 i;
	struct r8192_priv *priv = rtllib_priv(dev);

	DM_RxPathSelTable.Enable = 1;
	DM_RxPathSelTable.SS_TH_low = RxPathSelection_SS_TH_low;
	DM_RxPathSelTable.diff_TH = RxPathSelection_diff_TH;
	if (priv->CustomerID == RT_CID_819x_Netcore)
		DM_RxPathSelTable.cck_method = CCK_Rx_Version_2;
	else
		DM_RxPathSelTable.cck_method = CCK_Rx_Version_1;
	DM_RxPathSelTable.DbgMode = DM_DBG_OFF;
	DM_RxPathSelTable.disabledRF = 0;
	for (i = 0; i < 4; i++) {
		DM_RxPathSelTable.rf_rssi[i] = 50;
		DM_RxPathSelTable.cck_pwdb_sta[i] = -64;
		DM_RxPathSelTable.rf_enable_rssi_th[i] = 100;
	}
}

#define PWDB_IN_RANGE	((cur_cck_pwdb < tmp_cck_max_pwdb) &&	\
			(cur_cck_pwdb > tmp_cck_sec_pwdb))

static void dm_rxpath_sel_byrssi(struct net_device *dev)
{
	struct r8192_priv *priv = rtllib_priv(dev);
	u8 i, max_rssi_index = 0, min_rssi_index = 0;
	u8 sec_rssi_index = 0, rf_num = 0;
	u8 tmp_max_rssi = 0, tmp_min_rssi = 0, tmp_sec_rssi = 0;
	u8 cck_default_Rx = 0x2;
	u8 cck_optional_Rx = 0x3;
	long tmp_cck_max_pwdb = 0, tmp_cck_min_pwdb = 0, tmp_cck_sec_pwdb = 0;
	u8 cck_rx_ver2_max_index = 0, cck_rx_ver2_min_index = 0;
	u8 cck_rx_ver2_sec_index = 0;
	u8 cur_rf_rssi;
	long cur_cck_pwdb;
	static u8 disabled_rf_cnt, cck_Rx_Path_initialized;
	u8 update_cck_rx_path;

	if (priv->rf_type != RF_2T4R)
		return;

	if (!cck_Rx_Path_initialized) {
		DM_RxPathSelTable.cck_Rx_path = (read_nic_byte(dev, 0xa07)&0xf);
		cck_Rx_Path_initialized = 1;
	}

	DM_RxPathSelTable.disabledRF = 0xf;
	DM_RxPathSelTable.disabledRF &= ~(read_nic_byte(dev, 0xc04));

	if (priv->rtllib->mode == WIRELESS_MODE_B)
		DM_RxPathSelTable.cck_method = CCK_Rx_Version_2;

	for (i = 0; i < RF90_PATH_MAX; i++) {
		if (!DM_RxPathSelTable.DbgMode)
			DM_RxPathSelTable.rf_rssi[i] = priv->stats.rx_rssi_percentage[i];

		if (priv->brfpath_rxenable[i]) {
			rf_num++;
			cur_rf_rssi = DM_RxPathSelTable.rf_rssi[i];

			if (rf_num == 1) {
				max_rssi_index = min_rssi_index = sec_rssi_index = i;
				tmp_max_rssi = tmp_min_rssi = tmp_sec_rssi = cur_rf_rssi;
			} else if (rf_num == 2) {
				if (cur_rf_rssi >= tmp_max_rssi) {
					tmp_max_rssi = cur_rf_rssi;
					max_rssi_index = i;
				} else {
					tmp_sec_rssi = tmp_min_rssi = cur_rf_rssi;
					sec_rssi_index = min_rssi_index = i;
				}
			} else {
				if (cur_rf_rssi > tmp_max_rssi) {
					tmp_sec_rssi = tmp_max_rssi;
					sec_rssi_index = max_rssi_index;
					tmp_max_rssi = cur_rf_rssi;
					max_rssi_index = i;
				} else if (cur_rf_rssi == tmp_max_rssi) {
					tmp_sec_rssi = cur_rf_rssi;
					sec_rssi_index = i;
				} else if ((cur_rf_rssi < tmp_max_rssi) &&
					   (cur_rf_rssi > tmp_sec_rssi)) {
					tmp_sec_rssi = cur_rf_rssi;
					sec_rssi_index = i;
				} else if (cur_rf_rssi == tmp_sec_rssi) {
					if (tmp_sec_rssi == tmp_min_rssi) {
						tmp_sec_rssi = cur_rf_rssi;
						sec_rssi_index = i;
					}
				} else if ((cur_rf_rssi < tmp_sec_rssi) &&
					   (cur_rf_rssi > tmp_min_rssi)) {
					;
				} else if (cur_rf_rssi == tmp_min_rssi) {
					if (tmp_sec_rssi == tmp_min_rssi) {
						tmp_min_rssi = cur_rf_rssi;
						min_rssi_index = i;
					}
				} else if (cur_rf_rssi < tmp_min_rssi) {
					tmp_min_rssi = cur_rf_rssi;
					min_rssi_index = i;
				}
			}
		}
	}

	rf_num = 0;
	if (DM_RxPathSelTable.cck_method == CCK_Rx_Version_2) {
		for (i = 0; i < RF90_PATH_MAX; i++) {
			if (priv->brfpath_rxenable[i]) {
				rf_num++;
				cur_cck_pwdb =
					 DM_RxPathSelTable.cck_pwdb_sta[i];

				if (rf_num == 1) {
					cck_rx_ver2_max_index = i;
					cck_rx_ver2_min_index = i;
					cck_rx_ver2_sec_index = i;
					tmp_cck_max_pwdb = cur_cck_pwdb;
					tmp_cck_min_pwdb = cur_cck_pwdb;
					tmp_cck_sec_pwdb = cur_cck_pwdb;
				} else if (rf_num == 2) {
					if (cur_cck_pwdb >= tmp_cck_max_pwdb) {
						tmp_cck_max_pwdb = cur_cck_pwdb;
						cck_rx_ver2_max_index = i;
					} else {
						tmp_cck_sec_pwdb = cur_cck_pwdb;
						tmp_cck_min_pwdb = cur_cck_pwdb;
						cck_rx_ver2_sec_index = i;
						cck_rx_ver2_min_index = i;
					}
				} else {
					if (cur_cck_pwdb > tmp_cck_max_pwdb) {
						tmp_cck_sec_pwdb =
							 tmp_cck_max_pwdb;
						cck_rx_ver2_sec_index =
							 cck_rx_ver2_max_index;
						tmp_cck_max_pwdb = cur_cck_pwdb;
						cck_rx_ver2_max_index = i;
					} else if (cur_cck_pwdb ==
						   tmp_cck_max_pwdb) {
						tmp_cck_sec_pwdb = cur_cck_pwdb;
						cck_rx_ver2_sec_index = i;
					} else if (PWDB_IN_RANGE) {
						tmp_cck_sec_pwdb = cur_cck_pwdb;
						cck_rx_ver2_sec_index = i;
					} else if (cur_cck_pwdb ==
						   tmp_cck_sec_pwdb) {
						if (tmp_cck_sec_pwdb ==
						    tmp_cck_min_pwdb) {
							tmp_cck_sec_pwdb =
								 cur_cck_pwdb;
							cck_rx_ver2_sec_index =
								 i;
						}
					} else if ((cur_cck_pwdb < tmp_cck_sec_pwdb) &&
						   (cur_cck_pwdb > tmp_cck_min_pwdb)) {
						;
					} else if (cur_cck_pwdb == tmp_cck_min_pwdb) {
						if (tmp_cck_sec_pwdb == tmp_cck_min_pwdb) {
							tmp_cck_min_pwdb = cur_cck_pwdb;
							cck_rx_ver2_min_index = i;
						}
					} else if (cur_cck_pwdb < tmp_cck_min_pwdb) {
						tmp_cck_min_pwdb = cur_cck_pwdb;
						cck_rx_ver2_min_index = i;
					}
				}

			}
		}
	}

	update_cck_rx_path = 0;
	if (DM_RxPathSelTable.cck_method == CCK_Rx_Version_2) {
		cck_default_Rx = cck_rx_ver2_max_index;
		cck_optional_Rx = cck_rx_ver2_sec_index;
		if (tmp_cck_max_pwdb != -64)
			update_cck_rx_path = 1;
	}

	if (tmp_min_rssi < DM_RxPathSelTable.SS_TH_low && disabled_rf_cnt < 2) {
		if ((tmp_max_rssi - tmp_min_rssi) >=
		     DM_RxPathSelTable.diff_TH) {
			DM_RxPathSelTable.rf_enable_rssi_th[min_rssi_index] =
				 tmp_max_rssi+5;
			rtl8192_setBBreg(dev, rOFDM0_TRxPathEnable,
				 0x1<<min_rssi_index, 0x0);
			rtl8192_setBBreg(dev, rOFDM1_TRxPathEnable,
				 0x1<<min_rssi_index, 0x0);
			disabled_rf_cnt++;
		}
		if (DM_RxPathSelTable.cck_method == CCK_Rx_Version_1) {
			cck_default_Rx = max_rssi_index;
			cck_optional_Rx = sec_rssi_index;
			if (tmp_max_rssi)
				update_cck_rx_path = 1;
		}
	}

	if (update_cck_rx_path) {
		DM_RxPathSelTable.cck_Rx_path = (cck_default_Rx<<2) |
						(cck_optional_Rx);
		rtl8192_setBBreg(dev, rCCK0_AFESetting, 0x0f000000,
				 DM_RxPathSelTable.cck_Rx_path);
	}

	if (DM_RxPathSelTable.disabledRF) {
		for (i = 0; i < 4; i++) {
			if ((DM_RxPathSelTable.disabledRF>>i) & 0x1) {
				if (tmp_max_rssi >=
				    DM_RxPathSelTable.rf_enable_rssi_th[i]) {
					rtl8192_setBBreg(dev,
						 rOFDM0_TRxPathEnable, 0x1 << i,
						 0x1);
					rtl8192_setBBreg(dev,
						 rOFDM1_TRxPathEnable,
						 0x1 << i, 0x1);
					DM_RxPathSelTable.rf_enable_rssi_th[i]
						 = 100;
					disabled_rf_cnt--;
				}
			}
		}
	}
}

static	void	dm_check_rx_path_selection(struct net_device *dev)
{
	struct r8192_priv *priv = rtllib_priv(dev);

	queue_delayed_work_rsl(priv->priv_wq, &priv->rfpath_check_wq, 0);
}


static void dm_init_fsync(struct net_device *dev)
{
	struct r8192_priv *priv = rtllib_priv(dev);

	priv->rtllib->fsync_time_interval = 500;
	priv->rtllib->fsync_rate_bitmap = 0x0f000800;
	priv->rtllib->fsync_rssi_threshold = 30;
	priv->rtllib->bfsync_enable = false;
	priv->rtllib->fsync_multiple_timeinterval = 3;
	priv->rtllib->fsync_firstdiff_ratethreshold = 100;
	priv->rtllib->fsync_seconddiff_ratethreshold = 200;
	priv->rtllib->fsync_state = Default_Fsync;
	priv->framesyncMonitor = 1;

	init_timer(&priv->fsync_timer);
	setup_timer(&priv->fsync_timer, dm_fsync_timer_callback,
		   (unsigned long) dev);
}


static void dm_deInit_fsync(struct net_device *dev)
{
	struct r8192_priv *priv = rtllib_priv(dev);

	del_timer_sync(&priv->fsync_timer);
}

void dm_fsync_timer_callback(unsigned long data)
{
	struct net_device *dev = (struct net_device *)data;
	struct r8192_priv *priv = rtllib_priv((struct net_device *)data);
	u32 rate_index, rate_count = 0, rate_count_diff = 0;
	bool		bSwitchFromCountDiff = false;
	bool		bDoubleTimeInterval = false;

	if (priv->rtllib->state == RTLLIB_LINKED &&
	    priv->rtllib->bfsync_enable &&
	    (priv->rtllib->pHTInfo->IOTAction & HT_IOT_ACT_CDD_FSYNC)) {
		u32 rate_bitmap;

		for (rate_index = 0; rate_index <= 27; rate_index++) {
			rate_bitmap  = 1 << rate_index;
			if (priv->rtllib->fsync_rate_bitmap &  rate_bitmap)
				rate_count +=
				   priv->stats.received_rate_histogram[1]
				   [rate_index];
		}

		if (rate_count < priv->rate_record)
			rate_count_diff = 0xffffffff - rate_count +
					  priv->rate_record;
		else
			rate_count_diff = rate_count - priv->rate_record;
		if (rate_count_diff < priv->rateCountDiffRecord) {

			u32 DiffNum = priv->rateCountDiffRecord -
				      rate_count_diff;
			if (DiffNum >=
			    priv->rtllib->fsync_seconddiff_ratethreshold)
				priv->ContinueDiffCount++;
			else
				priv->ContinueDiffCount = 0;

			if (priv->ContinueDiffCount >= 2) {
				bSwitchFromCountDiff = true;
				priv->ContinueDiffCount = 0;
			}
		} else {
			priv->ContinueDiffCount = 0;
		}

		if (rate_count_diff <=
		    priv->rtllib->fsync_firstdiff_ratethreshold) {
			bSwitchFromCountDiff = true;
			priv->ContinueDiffCount = 0;
		}
		priv->rate_record = rate_count;
		priv->rateCountDiffRecord = rate_count_diff;
		RT_TRACE(COMP_HALDM,
			 "rateRecord %d rateCount %d, rateCountdiff %d bSwitchFsync %d\n",
			 priv->rate_record, rate_count, rate_count_diff,
			 priv->bswitch_fsync);
		if (priv->undecorated_smoothed_pwdb >
		    priv->rtllib->fsync_rssi_threshold &&
		    bSwitchFromCountDiff) {
			bDoubleTimeInterval = true;
			priv->bswitch_fsync = !priv->bswitch_fsync;
			if (priv->bswitch_fsync) {
				write_nic_byte(dev, 0xC36, 0x1c);
				write_nic_byte(dev, 0xC3e, 0x90);
			} else {
				write_nic_byte(dev, 0xC36, 0x5c);
				write_nic_byte(dev, 0xC3e, 0x96);
			}
		} else if (priv->undecorated_smoothed_pwdb <=
			   priv->rtllib->fsync_rssi_threshold) {
			if (priv->bswitch_fsync) {
				priv->bswitch_fsync  = false;
				write_nic_byte(dev, 0xC36, 0x5c);
				write_nic_byte(dev, 0xC3e, 0x96);
			}
		}
		if (bDoubleTimeInterval) {
			if (timer_pending(&priv->fsync_timer))
				del_timer_sync(&priv->fsync_timer);
			priv->fsync_timer.expires = jiffies +
				 msecs_to_jiffies(priv->rtllib->fsync_time_interval *
				 priv->rtllib->fsync_multiple_timeinterval);
			add_timer(&priv->fsync_timer);
		} else {
			if (timer_pending(&priv->fsync_timer))
				del_timer_sync(&priv->fsync_timer);
			priv->fsync_timer.expires = jiffies +
				 msecs_to_jiffies(priv->rtllib->fsync_time_interval);
			add_timer(&priv->fsync_timer);
		}
	} else {
		if (priv->bswitch_fsync) {
			priv->bswitch_fsync  = false;
			write_nic_byte(dev, 0xC36, 0x5c);
			write_nic_byte(dev, 0xC3e, 0x96);
		}
		priv->ContinueDiffCount = 0;
		write_nic_dword(dev, rOFDM0_RxDetector2, 0x465c52cd);
	}
	RT_TRACE(COMP_HALDM, "ContinueDiffCount %d\n", priv->ContinueDiffCount);
	RT_TRACE(COMP_HALDM,
		 "rateRecord %d rateCount %d, rateCountdiff %d bSwitchFsync %d\n",
		 priv->rate_record, rate_count, rate_count_diff,
		 priv->bswitch_fsync);
}

static void dm_StartHWFsync(struct net_device *dev)
{
	u8 rf_timing = 0x77;
	struct r8192_priv *priv = rtllib_priv(dev);

	RT_TRACE(COMP_HALDM, "%s\n", __func__);
	write_nic_dword(dev, rOFDM0_RxDetector2, 0x465c12cf);
	priv->rtllib->SetHwRegHandler(dev, HW_VAR_RF_TIMING,
				      (u8 *)(&rf_timing));
	write_nic_byte(dev, 0xc3b, 0x41);
}

static void dm_EndHWFsync(struct net_device *dev)
{
	u8 rf_timing = 0xaa;
	struct r8192_priv *priv = rtllib_priv(dev);

	RT_TRACE(COMP_HALDM, "%s\n", __func__);
	write_nic_dword(dev, rOFDM0_RxDetector2, 0x465c52cd);
	priv->rtllib->SetHwRegHandler(dev, HW_VAR_RF_TIMING, (u8 *)
				     (&rf_timing));
	write_nic_byte(dev, 0xc3b, 0x49);
}

static void dm_EndSWFsync(struct net_device *dev)
{
	struct r8192_priv *priv = rtllib_priv(dev);

	RT_TRACE(COMP_HALDM, "%s\n", __func__);
	del_timer_sync(&(priv->fsync_timer));

	if (priv->bswitch_fsync) {
		priv->bswitch_fsync  = false;

		write_nic_byte(dev, 0xC36, 0x5c);

		write_nic_byte(dev, 0xC3e, 0x96);
	}

	priv->ContinueDiffCount = 0;
	write_nic_dword(dev, rOFDM0_RxDetector2, 0x465c52cd);
}

static void dm_StartSWFsync(struct net_device *dev)
{
	struct r8192_priv *priv = rtllib_priv(dev);
	u32			rateIndex;
	u32			rateBitmap;

	RT_TRACE(COMP_HALDM, "%s\n", __func__);
	priv->rate_record = 0;
	priv->ContinueDiffCount = 0;
	priv->rateCountDiffRecord = 0;
	priv->bswitch_fsync  = false;

	if (priv->rtllib->mode == WIRELESS_MODE_N_24G) {
		priv->rtllib->fsync_firstdiff_ratethreshold = 600;
		priv->rtllib->fsync_seconddiff_ratethreshold = 0xffff;
	} else {
		priv->rtllib->fsync_firstdiff_ratethreshold = 200;
		priv->rtllib->fsync_seconddiff_ratethreshold = 200;
	}
	for (rateIndex = 0; rateIndex <= 27; rateIndex++) {
		rateBitmap  = 1 << rateIndex;
		if (priv->rtllib->fsync_rate_bitmap & rateBitmap)
			priv->rate_record +=
				 priv->stats.received_rate_histogram[1]
				[rateIndex];
	}
	if (timer_pending(&priv->fsync_timer))
		del_timer_sync(&priv->fsync_timer);
	priv->fsync_timer.expires = jiffies +
				    msecs_to_jiffies(priv->rtllib->fsync_time_interval);
	add_timer(&priv->fsync_timer);

	write_nic_dword(dev, rOFDM0_RxDetector2, 0x465c12cd);

}

void dm_check_fsync(struct net_device *dev)
{
#define	RegC38_Default			0
#define	RegC38_NonFsync_Other_AP	1
#define	RegC38_Fsync_AP_BCM		2
	struct r8192_priv *priv = rtllib_priv(dev);
	static u8 reg_c38_State = RegC38_Default;
	static u32 reset_cnt;

	RT_TRACE(COMP_HALDM,
		 "RSSI %d TimeInterval %d MultipleTimeInterval %d\n",
		 priv->rtllib->fsync_rssi_threshold,
		 priv->rtllib->fsync_time_interval,
		 priv->rtllib->fsync_multiple_timeinterval);
	RT_TRACE(COMP_HALDM,
		 "RateBitmap 0x%x FirstDiffRateThreshold %d SecondDiffRateThreshold %d\n",
		 priv->rtllib->fsync_rate_bitmap,
		 priv->rtllib->fsync_firstdiff_ratethreshold,
		 priv->rtllib->fsync_seconddiff_ratethreshold);

	if (priv->rtllib->state == RTLLIB_LINKED &&
	    priv->rtllib->pHTInfo->IOTPeer == HT_IOT_PEER_BROADCOM) {
		if (priv->rtllib->bfsync_enable == 0) {
			switch (priv->rtllib->fsync_state) {
			case Default_Fsync:
				dm_StartHWFsync(dev);
				priv->rtllib->fsync_state = HW_Fsync;
				break;
			case SW_Fsync:
				dm_EndSWFsync(dev);
				dm_StartHWFsync(dev);
				priv->rtllib->fsync_state = HW_Fsync;
				break;
			case HW_Fsync:
			default:
				break;
			}
		} else {
			switch (priv->rtllib->fsync_state) {
			case Default_Fsync:
				dm_StartSWFsync(dev);
				priv->rtllib->fsync_state = SW_Fsync;
				break;
			case HW_Fsync:
				dm_EndHWFsync(dev);
				dm_StartSWFsync(dev);
				priv->rtllib->fsync_state = SW_Fsync;
				break;
			case SW_Fsync:
			default:
				break;

			}
		}
		if (priv->framesyncMonitor) {
			if (reg_c38_State != RegC38_Fsync_AP_BCM) {
				write_nic_byte(dev, rOFDM0_RxDetector3, 0x95);

				reg_c38_State = RegC38_Fsync_AP_BCM;
			}
		}
	} else {
		switch (priv->rtllib->fsync_state) {
		case HW_Fsync:
			dm_EndHWFsync(dev);
			priv->rtllib->fsync_state = Default_Fsync;
			break;
		case SW_Fsync:
			dm_EndSWFsync(dev);
			priv->rtllib->fsync_state = Default_Fsync;
			break;
		case Default_Fsync:
		default:
			break;
		}

		if (priv->framesyncMonitor) {
			if (priv->rtllib->state == RTLLIB_LINKED) {
				if (priv->undecorated_smoothed_pwdb <=
				    RegC38_TH) {
					if (reg_c38_State !=
					    RegC38_NonFsync_Other_AP) {
							write_nic_byte(dev,
							    rOFDM0_RxDetector3,
							    0x90);

						reg_c38_State =
						     RegC38_NonFsync_Other_AP;
					}
				} else if (priv->undecorated_smoothed_pwdb >=
					   (RegC38_TH+5)) {
					if (reg_c38_State) {
						write_nic_byte(dev,
							rOFDM0_RxDetector3,
							priv->framesync);
						reg_c38_State = RegC38_Default;
					}
				}
			} else {
				if (reg_c38_State) {
					write_nic_byte(dev, rOFDM0_RxDetector3,
						       priv->framesync);
					reg_c38_State = RegC38_Default;
				}
			}
		}
	}
	if (priv->framesyncMonitor) {
		if (priv->reset_count != reset_cnt) {
			write_nic_byte(dev, rOFDM0_RxDetector3,
				       priv->framesync);
			reg_c38_State = RegC38_Default;
			reset_cnt = priv->reset_count;
		}
	} else {
		if (reg_c38_State) {
			write_nic_byte(dev, rOFDM0_RxDetector3,
				       priv->framesync);
			reg_c38_State = RegC38_Default;
		}
	}
}

void dm_shadow_init(struct net_device *dev)
{
	u8	page;
	u16	offset;

	for (page = 0; page < 5; page++)
		for (offset = 0; offset < 256; offset++)
			dm_shadow[page][offset] = read_nic_byte(dev,
						  offset+page * 256);

	for (page = 8; page < 11; page++)
		for (offset = 0; offset < 256; offset++)
			dm_shadow[page][offset] = read_nic_byte(dev,
						  offset+page * 256);

	for (page = 12; page < 15; page++)
		for (offset = 0; offset < 256; offset++)
			dm_shadow[page][offset] = read_nic_byte(dev,
						  offset+page*256);

}

/*---------------------------Define function prototype------------------------*/
static void dm_init_dynamic_txpower(struct net_device *dev)
{
	struct r8192_priv *priv = rtllib_priv(dev);

	priv->rtllib->bdynamic_txpower_enable = true;
	priv->bLastDTPFlag_High = false;
	priv->bLastDTPFlag_Low = false;
	priv->bDynamicTxHighPower = false;
	priv->bDynamicTxLowPower = false;
}

static void dm_dynamic_txpower(struct net_device *dev)
{
	struct r8192_priv *priv = rtllib_priv(dev);
	unsigned int txhipower_threshhold = 0;
	unsigned int txlowpower_threshold = 0;

	if (priv->rtllib->bdynamic_txpower_enable != true) {
		priv->bDynamicTxHighPower = false;
		priv->bDynamicTxLowPower = false;
		return;
	}
	if ((priv->rtllib->pHTInfo->IOTPeer == HT_IOT_PEER_ATHEROS) &&
	    (priv->rtllib->mode == IEEE_G)) {
		txhipower_threshhold = TX_POWER_ATHEROAP_THRESH_HIGH;
		txlowpower_threshold = TX_POWER_ATHEROAP_THRESH_LOW;
	} else {
		txhipower_threshhold = TX_POWER_NEAR_FIELD_THRESH_HIGH;
		txlowpower_threshold = TX_POWER_NEAR_FIELD_THRESH_LOW;
	}

	RT_TRACE(COMP_TXAGC, "priv->undecorated_smoothed_pwdb = %ld\n",
		 priv->undecorated_smoothed_pwdb);

	if (priv->rtllib->state == RTLLIB_LINKED) {
		if (priv->undecorated_smoothed_pwdb >= txhipower_threshhold) {
			priv->bDynamicTxHighPower = true;
			priv->bDynamicTxLowPower = false;
		} else {
			if (priv->undecorated_smoothed_pwdb <
			    txlowpower_threshold && priv->bDynamicTxHighPower)
				priv->bDynamicTxHighPower = false;
			if (priv->undecorated_smoothed_pwdb < 35)
				priv->bDynamicTxLowPower = true;
			else if (priv->undecorated_smoothed_pwdb >= 40)
				priv->bDynamicTxLowPower = false;
		}
	} else {
		priv->bDynamicTxHighPower = false;
		priv->bDynamicTxLowPower = false;
	}

	if ((priv->bDynamicTxHighPower != priv->bLastDTPFlag_High) ||
	    (priv->bDynamicTxLowPower != priv->bLastDTPFlag_Low)) {
		RT_TRACE(COMP_TXAGC, "SetTxPowerLevel8190()  channel = %d\n",
			 priv->rtllib->current_network.channel);

		rtl8192_phy_setTxPower(dev,
				 priv->rtllib->current_network.channel);
	}
	priv->bLastDTPFlag_High = priv->bDynamicTxHighPower;
	priv->bLastDTPFlag_Low = priv->bDynamicTxLowPower;

}

static void dm_check_txrateandretrycount(struct net_device *dev)
{
	struct r8192_priv *priv = rtllib_priv(dev);
	struct rtllib_device *ieee = priv->rtllib;

	ieee->softmac_stats.CurrentShowTxate = read_nic_byte(dev,
						 Current_Tx_Rate_Reg);

	ieee->softmac_stats.last_packet_rate = read_nic_byte(dev,
						 Initial_Tx_Rate_Reg);

	ieee->softmac_stats.txretrycount = read_nic_dword(dev,
						 Tx_Retry_Count_Reg);
}

static void dm_send_rssi_tofw(struct net_device *dev)
{
	struct r8192_priv *priv = rtllib_priv(dev);

	write_nic_byte(dev, DRIVER_RSSI, (u8)priv->undecorated_smoothed_pwdb);
}
