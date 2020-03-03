/******************************************************************************
 *
 * Copyright(c) 2007 - 2012 Realtek Corporation. All rights reserved.
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
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/

#define _HAL_INTF_C_
#include <osdep_service.h>
#include <drv_types.h>
#include <hal_intf.h>
#include <usb_hal.h>

void rtw_hal_chip_configure(struct adapter *adapt)
{
	if (adapt->HalFunc.intf_chip_configure)
		adapt->HalFunc.intf_chip_configure(adapt);
}

void rtw_hal_read_chip_info(struct adapter *adapt)
{
	if (adapt->HalFunc.read_adapter_info)
		adapt->HalFunc.read_adapter_info(adapt);
}

void rtw_hal_read_chip_version(struct adapter *adapt)
{
	if (adapt->HalFunc.read_chip_version)
		adapt->HalFunc.read_chip_version(adapt);
}

void rtw_hal_def_value_init(struct adapter *adapt)
{
	if (adapt->HalFunc.init_default_value)
		adapt->HalFunc.init_default_value(adapt);
}

void rtw_hal_free_data(struct adapter *adapt)
{
	if (adapt->HalFunc.free_hal_data)
		adapt->HalFunc.free_hal_data(adapt);
}

void rtw_hal_dm_init(struct adapter *adapt)
{
	if (adapt->HalFunc.dm_init)
		adapt->HalFunc.dm_init(adapt);
}

void rtw_hal_sw_led_init(struct adapter *adapt)
{
	if (adapt->HalFunc.InitSwLeds)
		adapt->HalFunc.InitSwLeds(adapt);
}

void rtw_hal_sw_led_deinit(struct adapter *adapt)
{
	if (adapt->HalFunc.DeInitSwLeds)
		adapt->HalFunc.DeInitSwLeds(adapt);
}

u32 rtw_hal_power_on(struct adapter *adapt)
{
	if (adapt->HalFunc.hal_power_on)
		return adapt->HalFunc.hal_power_on(adapt);
	return _FAIL;
}

uint	 rtw_hal_init(struct adapter *adapt)
{
	uint	status = _SUCCESS;

	adapt->hw_init_completed = false;

	status = adapt->HalFunc.hal_init(adapt);

	if (status == _SUCCESS) {
		adapt->hw_init_completed = true;

		if (adapt->registrypriv.notch_filter == 1)
			rtw_hal_notch_filter(adapt, 1);

		rtw_hal_reset_security_engine(adapt);
	} else {
		adapt->hw_init_completed = false;
		DBG_88E("rtw_hal_init: hal__init fail\n");
	}

	RT_TRACE(_module_hal_init_c_, _drv_err_,
		 ("-rtl871x_hal_init:status=0x%x\n", status));

	return status;
}

uint rtw_hal_deinit(struct adapter *adapt)
{
	uint	status = _SUCCESS;

	status = adapt->HalFunc.hal_deinit(adapt);

	if (status == _SUCCESS)
		adapt->hw_init_completed = false;
	else
		DBG_88E("\n rtw_hal_deinit: hal_init fail\n");

	return status;
}

void rtw_hal_set_hwreg(struct adapter *adapt, u8 variable, u8 *val)
{
	if (adapt->HalFunc.SetHwRegHandler)
		adapt->HalFunc.SetHwRegHandler(adapt, variable, val);
}

void rtw_hal_get_hwreg(struct adapter *adapt, u8 variable, u8 *val)
{
	if (adapt->HalFunc.GetHwRegHandler)
		adapt->HalFunc.GetHwRegHandler(adapt, variable, val);
}

u8 rtw_hal_set_def_var(struct adapter *adapt, enum hal_def_variable var,
		      void *val)
{
	if (adapt->HalFunc.SetHalDefVarHandler)
		return adapt->HalFunc.SetHalDefVarHandler(adapt, var, val);
	return _FAIL;
}

u8 rtw_hal_get_def_var(struct adapter *adapt,
		       enum hal_def_variable var, void *val)
{
	if (adapt->HalFunc.GetHalDefVarHandler)
		return adapt->HalFunc.GetHalDefVarHandler(adapt, var, val);
	return _FAIL;
}

void rtw_hal_set_odm_var(struct adapter *adapt,
			 enum hal_odm_variable var, void *val1,
			 bool set)
{
	if (adapt->HalFunc.SetHalODMVarHandler)
		adapt->HalFunc.SetHalODMVarHandler(adapt, var,
						      val1, set);
}

void rtw_hal_enable_interrupt(struct adapter *adapt)
{
	if (adapt->HalFunc.enable_interrupt)
		adapt->HalFunc.enable_interrupt(adapt);
	else
		DBG_88E("%s: HalFunc.enable_interrupt is NULL!\n", __func__);
}

void rtw_hal_disable_interrupt(struct adapter *adapt)
{
	if (adapt->HalFunc.disable_interrupt)
		adapt->HalFunc.disable_interrupt(adapt);
	else
		DBG_88E("%s: HalFunc.disable_interrupt is NULL!\n", __func__);
}

u32 rtw_hal_inirp_init(struct adapter *adapt)
{
	u32 rst = _FAIL;

	if (adapt->HalFunc.inirp_init)
		rst = adapt->HalFunc.inirp_init(adapt);
	else
		DBG_88E(" %s HalFunc.inirp_init is NULL!!!\n", __func__);
	return rst;
}

u32 rtw_hal_inirp_deinit(struct adapter *adapt)
{
	if (adapt->HalFunc.inirp_deinit)
		return adapt->HalFunc.inirp_deinit(adapt);

	return _FAIL;
}

s32 rtw_hal_xmit(struct adapter *adapt, struct xmit_frame *pxmitframe)
{
	if (adapt->HalFunc.hal_xmit)
		return adapt->HalFunc.hal_xmit(adapt, pxmitframe);

	return false;
}

s32 rtw_hal_mgnt_xmit(struct adapter *adapt, struct xmit_frame *pmgntframe)
{
	s32 ret = _FAIL;
	if (adapt->HalFunc.mgnt_xmit)
		ret = adapt->HalFunc.mgnt_xmit(adapt, pmgntframe);
	return ret;
}

s32 rtw_hal_init_xmit_priv(struct adapter *adapt)
{
	if (adapt->HalFunc.init_xmit_priv != NULL)
		return adapt->HalFunc.init_xmit_priv(adapt);
	return _FAIL;
}

s32 rtw_hal_init_recv_priv(struct adapter *adapt)
{
	if (adapt->HalFunc.init_recv_priv)
		return adapt->HalFunc.init_recv_priv(adapt);

	return _FAIL;
}

void rtw_hal_free_recv_priv(struct adapter *adapt)
{
	if (adapt->HalFunc.free_recv_priv)
		adapt->HalFunc.free_recv_priv(adapt);
}

void rtw_hal_update_ra_mask(struct adapter *adapt, u32 mac_id, u8 rssi_level)
{
	struct mlme_priv *pmlmepriv = &(adapt->mlmepriv);

	if (check_fwstate(pmlmepriv, WIFI_AP_STATE) == true) {
#ifdef CONFIG_88EU_AP_MODE
		struct sta_info *psta = NULL;
		struct sta_priv *pstapriv = &adapt->stapriv;
		if ((mac_id-1) > 0)
			psta = pstapriv->sta_aid[(mac_id-1) - 1];
		if (psta)
			add_RATid(adapt, psta, 0);/* todo: based on rssi_level*/
#endif
	} else {
		if (adapt->HalFunc.UpdateRAMaskHandler)
			adapt->HalFunc.UpdateRAMaskHandler(adapt, mac_id,
							      rssi_level);
	}
}

void rtw_hal_add_ra_tid(struct adapter *adapt, u32 bitmap, u8 arg,
			u8 rssi_level)
{
	if (adapt->HalFunc.Add_RateATid)
		adapt->HalFunc.Add_RateATid(adapt, bitmap, arg,
					       rssi_level);
}

u32 rtw_hal_read_rfreg(struct adapter *adapt, enum rf_radio_path rfpath,
		       u32 regaddr, u32 bitmask)
{
	u32 data = 0;

	if (adapt->HalFunc.read_rfreg)
		data = adapt->HalFunc.read_rfreg(adapt, rfpath, regaddr,
						    bitmask);
	return data;
}

void rtw_hal_write_rfreg(struct adapter *adapt, enum rf_radio_path rfpath,
			 u32 regaddr, u32 bitmask, u32 data)
{
	if (adapt->HalFunc.write_rfreg)
		adapt->HalFunc.write_rfreg(adapt, rfpath, regaddr,
					      bitmask, data);
}

void rtw_hal_set_bwmode(struct adapter *adapt,
			enum ht_channel_width bandwidth, u8 offset)
{
	if (adapt->HalFunc.set_bwmode_handler)
		adapt->HalFunc.set_bwmode_handler(adapt, bandwidth,
						     offset);
}

void rtw_hal_set_chan(struct adapter *adapt, u8 channel)
{
	if (adapt->HalFunc.set_channel_handler)
		adapt->HalFunc.set_channel_handler(adapt, channel);
}

void rtw_hal_dm_watchdog(struct adapter *adapt)
{
	if (adapt->HalFunc.hal_dm_watchdog)
		adapt->HalFunc.hal_dm_watchdog(adapt);
}

void rtw_hal_bcn_related_reg_setting(struct adapter *adapt)
{
	if (adapt->HalFunc.SetBeaconRelatedRegistersHandler)
		adapt->HalFunc.SetBeaconRelatedRegistersHandler(adapt);
}

u8 rtw_hal_antdiv_before_linked(struct adapter *adapt)
{
	if (adapt->HalFunc.AntDivBeforeLinkHandler)
		return adapt->HalFunc.AntDivBeforeLinkHandler(adapt);
	return false;
}

void rtw_hal_antdiv_rssi_compared(struct adapter *adapt,
				  struct wlan_bssid_ex *dst,
				  struct wlan_bssid_ex *src)
{
	if (adapt->HalFunc.AntDivCompareHandler)
		adapt->HalFunc.AntDivCompareHandler(adapt, dst, src);
}

void rtw_hal_sreset_init(struct adapter *adapt)
{
	if (adapt->HalFunc.sreset_init_value)
		adapt->HalFunc.sreset_init_value(adapt);
}

void rtw_hal_notch_filter(struct adapter *adapter, bool enable)
{
	if (adapter->HalFunc.hal_notch_filter)
		adapter->HalFunc.hal_notch_filter(adapter, enable);
}

void rtw_hal_reset_security_engine(struct adapter *adapter)
{
	if (adapter->HalFunc.hal_reset_security_engine)
		adapter->HalFunc.hal_reset_security_engine(adapter);
}
