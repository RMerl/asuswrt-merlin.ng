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
#ifndef __RTL8188E_DM_H__
#define __RTL8188E_DM_H__
enum{
	UP_LINK,
	DOWN_LINK,
};
/*  duplicate code,will move to ODM ######### */
#define IQK_MAC_REG_NUM		4
#define IQK_ADDA_REG_NUM		16
#define IQK_BB_REG_NUM			9
#define HP_THERMAL_NUM		8
/*  duplicate code,will move to ODM ######### */
struct	dm_priv {
	u8	DM_Type;
	u8	DMFlag;
	u8	InitDMFlag;
	u32	InitODMFlag;

	/*  Upper and Lower Signal threshold for Rate Adaptive*/
	int	UndecoratedSmoothedPWDB;
	int	UndecoratedSmoothedCCK;
	int	EntryMinUndecoratedSmoothedPWDB;
	int	EntryMaxUndecoratedSmoothedPWDB;
	int	MinUndecoratedPWDBForDM;
	int	LastMinUndecoratedPWDBForDM;

	/* for High Power */
	u8 bDynamicTxPowerEnable;
	u8 LastDTPLvl;
	u8 DynamicTxHighPowerLvl;/* Tx Power Control for Near/Far Range */
	u8	PowerIndex_backup[6];
};

void rtl8188e_init_dm_priv(struct adapter *adapt);
void rtl8188e_InitHalDm(struct adapter *adapt);
void rtl8188e_HalDmWatchDog(struct adapter *adapt);

void AntDivCompare8188E(struct adapter *adapt, struct wlan_bssid_ex *dst,
			struct wlan_bssid_ex *src);
u8 AntDivBeforeLink8188E(struct adapter *adapt);

#endif
