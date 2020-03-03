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
 ******************************************************************************/
#ifndef _RTW_SRESET_C_
#define _RTW_SRESET_C_

#include <osdep_service.h>
#include <drv_types.h>

struct sreset_priv {
	struct mutex	silentreset_mutex;
	u8	silent_reset_inprogress;
	unsigned long last_tx_time;
	unsigned long last_tx_complete_time;
};

#include <rtl8723a_hal.h>

void rtw_sreset_init(struct rtw_adapter *padapter);
void rtw_sreset_reset_value(struct rtw_adapter *padapter);
bool rtw_sreset_inprogress(struct rtw_adapter *padapter);
void sreset_set_trigger_point(struct rtw_adapter *padapter, s32 tgp);
void rtw_sreset_reset(struct rtw_adapter *active_adapter);

#endif
