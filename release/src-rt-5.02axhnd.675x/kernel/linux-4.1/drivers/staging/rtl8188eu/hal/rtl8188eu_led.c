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
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/

#include <osdep_service.h>
#include <drv_types.h>
#include <rtl8188e_hal.h>
#include <rtl8188e_led.h>
#include <usb_ops_linux.h>

/*  LED object. */

/*  LED_819xUsb routines. */
/*	Description: */
/*		Turn on LED according to LedPin specified. */
void SwLedOn(struct adapter *padapter, struct LED_871x *pLed)
{
	u8	LedCfg;

	if (padapter->bSurpriseRemoved || padapter->bDriverStopped)
		return;
	LedCfg = usb_read8(padapter, REG_LEDCFG2);
	usb_write8(padapter, REG_LEDCFG2, (LedCfg&0xf0)|BIT5|BIT6); /*  SW control led0 on. */
	pLed->bLedOn = true;
}

/*	Description: */
/*		Turn off LED according to LedPin specified. */
void SwLedOff(struct adapter *padapter, struct LED_871x *pLed)
{
	u8	LedCfg;
	struct hal_data_8188e	*pHalData = GET_HAL_DATA(padapter);

	if (padapter->bSurpriseRemoved || padapter->bDriverStopped)
		goto exit;

	LedCfg = usb_read8(padapter, REG_LEDCFG2);/* 0x4E */

	if (pHalData->bLedOpenDrain) {
			/*  Open-drain arrangement for controlling the LED) */
		LedCfg &= 0x90; /*  Set to software control. */
		usb_write8(padapter, REG_LEDCFG2, (LedCfg|BIT3));
		LedCfg = usb_read8(padapter, REG_MAC_PINMUX_CFG);
		LedCfg &= 0xFE;
		usb_write8(padapter, REG_MAC_PINMUX_CFG, LedCfg);
	} else {
		usb_write8(padapter, REG_LEDCFG2, (LedCfg|BIT3|BIT5|BIT6));
	}
exit:
	pLed->bLedOn = false;
}

/*  Interface to manipulate LED objects. */
/*  Default LED behavior. */

/*	Description: */
/*		Initialize all LED_871x objects. */
void rtl8188eu_InitSwLeds(struct adapter *padapter)
{
	struct led_priv *pledpriv = &(padapter->ledpriv);
	struct hal_data_8188e   *haldata = GET_HAL_DATA(padapter);

	pledpriv->bRegUseLed = true;
	pledpriv->LedControlHandler = LedControl8188eu;
	haldata->bLedOpenDrain = true;

	InitLed871x(padapter, &(pledpriv->SwLed0));
}

/*	Description: */
/*		DeInitialize all LED_819xUsb objects. */
void rtl8188eu_DeInitSwLeds(struct adapter *padapter)
{
	struct led_priv	*ledpriv = &(padapter->ledpriv);

	DeInitLed871x(&(ledpriv->SwLed0));
}
