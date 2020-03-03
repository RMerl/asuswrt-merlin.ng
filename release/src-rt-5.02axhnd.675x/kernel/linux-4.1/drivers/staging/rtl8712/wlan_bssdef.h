/******************************************************************************
 *
 * Copyright(c) 2007 - 2010 Realtek Corporation. All rights reserved.
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
 * Modifications for inclusion into the Linux staging tree are
 * Copyright(c) 2010 Larry Finger. All rights reserved.
 *
 * Contact information:
 * WLAN FAE <wlanfae@realtek.com>
 * Larry Finger <Larry.Finger@lwfinger.net>
 *
 ******************************************************************************/
#ifndef __WLAN_BSSDEF_H__
#define __WLAN_BSSDEF_H__

#define MAX_IE_SZ	768

#define NDIS_802_11_LENGTH_SSID         32
#define NDIS_802_11_LENGTH_RATES        8
#define NDIS_802_11_LENGTH_RATES_EX     16

/* Set of 8 data rates*/
typedef unsigned char   NDIS_802_11_RATES[NDIS_802_11_LENGTH_RATES];
/* Set of 16 data rates */
typedef unsigned char   NDIS_802_11_RATES_EX[NDIS_802_11_LENGTH_RATES_EX];

struct ndis_802_11_ssid {
	u32 SsidLength;
	u8  Ssid[32];
};

enum NDIS_802_11_NETWORK_TYPE {
	Ndis802_11FH,
	Ndis802_11DS,
	Ndis802_11OFDM5,
	Ndis802_11OFDM24,
	Ndis802_11NetworkTypeMax /* not a real type, defined as an upper bound*/
};

struct NDIS_802_11_CONFIGURATION_FH {
	u32 Length;             /* Length of structure */
	u32 HopPattern;         /* As defined by 802.11, MSB set */
	u32 HopSet;             /* to one if non-802.11 */
	u32 DwellTime;          /* units are Kusec */
};

/*
	FW will only save the channel number in DSConfig.
	ODI Handler will convert the channel number to freq. number.
*/
struct NDIS_802_11_CONFIGURATION {
	u32 Length;             /* Length of structure */
	u32 BeaconPeriod;       /* units are Kusec */
	u32 ATIMWindow;         /* units are Kusec */
	u32 DSConfig;           /* Frequency, units are kHz */
	struct NDIS_802_11_CONFIGURATION_FH FHConfig;
};

enum NDIS_802_11_NETWORK_INFRASTRUCTURE {
	Ndis802_11IBSS,
	Ndis802_11Infrastructure,
	Ndis802_11AutoUnknown,
	Ndis802_11InfrastructureMax, /*Not a real value,defined as upper bound*/
	Ndis802_11APMode
};

struct NDIS_802_11_FIXED_IEs {
	u8  Timestamp[8];
	u16 BeaconInterval;
	u16 Capabilities;
};

/*
 * Length is the 4 bytes multiples of the sume of
 * 6 * sizeof (unsigned char) + 2 + sizeof (ndis_802_11_ssid) + sizeof (u32)
 * + sizeof (s32) + sizeof (NDIS_802_11_NETWORK_TYPE)
 * + sizeof (struct NDIS_802_11_CONFIGURATION)
 * + sizeof (NDIS_802_11_RATES_EX) + IELength

 * Except the IELength, all other fields are fixed length. Therefore, we can
 * define a macro to present the partial sum.
 */

struct ndis_wlan_bssid_ex {
	u32 Length;
	unsigned char  MacAddress[6];
	u8  Reserved[2];
	struct ndis_802_11_ssid  Ssid;
	u32 Privacy;
	s32 Rssi;
	enum NDIS_802_11_NETWORK_TYPE  NetworkTypeInUse;
	struct NDIS_802_11_CONFIGURATION  Configuration;
	enum NDIS_802_11_NETWORK_INFRASTRUCTURE  InfrastructureMode;
	NDIS_802_11_RATES_EX  SupportedRates;
	u32 IELength;
	/*(timestamp, beacon interval, and capability information) */
	u8 IEs[MAX_IE_SZ];
};

enum NDIS_802_11_AUTHENTICATION_MODE {
	Ndis802_11AuthModeOpen,
	Ndis802_11AuthModeShared,
	Ndis802_11AuthModeAutoSwitch,
	Ndis802_11AuthModeWPA,
	Ndis802_11AuthModeWPAPSK,
	Ndis802_11AuthModeWPANone,
	Ndis802_11AuthModeMax      /* Not a real mode, defined as upper bound */
};

enum {
	Ndis802_11WEPEnabled,
	Ndis802_11Encryption1Enabled = Ndis802_11WEPEnabled,
	Ndis802_11WEPDisabled,
	Ndis802_11EncryptionDisabled = Ndis802_11WEPDisabled,
	Ndis802_11WEPKeyAbsent,
	Ndis802_11Encryption1KeyAbsent = Ndis802_11WEPKeyAbsent,
	Ndis802_11WEPNotSupported,
	Ndis802_11EncryptionNotSupported = Ndis802_11WEPNotSupported,
	Ndis802_11Encryption2Enabled,
	Ndis802_11Encryption2KeyAbsent,
	Ndis802_11Encryption3Enabled,
	Ndis802_11Encryption3KeyAbsent
};

#define NDIS_802_11_AI_REQFI_CAPABILITIES      1
#define NDIS_802_11_AI_REQFI_LISTENINTERVAL    2
#define NDIS_802_11_AI_REQFI_CURRENTAPADDRESS  4

#define NDIS_802_11_AI_RESFI_CAPABILITIES      1
#define NDIS_802_11_AI_RESFI_STATUSCODE        2
#define NDIS_802_11_AI_RESFI_ASSOCIATIONID     4

struct NDIS_802_11_AI_REQFI {
	u16 Capabilities;
	u16 ListenInterval;
	unsigned char CurrentAPAddress[6];
};

struct NDIS_802_11_AI_RESFI {
	u16 Capabilities;
	u16 StatusCode;
	u16 AssociationId;
};

struct NDIS_802_11_ASSOCIATION_INFORMATION {
	u32 Length;
	u16 AvailableRequestFixedIEs;
	struct NDIS_802_11_AI_REQFI RequestFixedIEs;
	u32 RequestIELength;
	u32 OffsetRequestIEs;
	u16 AvailableResponseFixedIEs;
	struct NDIS_802_11_AI_RESFI ResponseFixedIEs;
	u32 ResponseIELength;
	u32 OffsetResponseIEs;
};

/* Key mapping keys require a BSSID*/
struct NDIS_802_11_KEY {
	u32 Length;			/* Length of this structure */
	u32 KeyIndex;
	u32 KeyLength;			/* length of key in bytes */
	unsigned char BSSID[6];
	unsigned long long KeyRSC;
	u8  KeyMaterial[32];		/* variable length */
};

struct NDIS_802_11_REMOVE_KEY {
	u32 Length;			/* Length of this structure */
	u32 KeyIndex;
	unsigned char BSSID[6];
};

struct NDIS_802_11_WEP {
	u32 Length;		  /* Length of this structure */
	u32 KeyIndex;		  /* 0 is the per-client key,
				   * 1-N are the global keys */
	u32 KeyLength;		  /* length of key in bytes */
	u8  KeyMaterial[16];      /* variable length depending on above field */
};

/* mask for authentication/integrity fields */
#define NDIS_802_11_AUTH_REQUEST_AUTH_FIELDS        0x0f
#define NDIS_802_11_AUTH_REQUEST_REAUTH			0x01
#define NDIS_802_11_AUTH_REQUEST_KEYUPDATE		0x02
#define NDIS_802_11_AUTH_REQUEST_PAIRWISE_ERROR		0x06
#define NDIS_802_11_AUTH_REQUEST_GROUP_ERROR		0x0E

/* MIC check time, 60 seconds. */
#define MIC_CHECK_TIME	60000000

#ifndef Ndis802_11APMode
#define Ndis802_11APMode (Ndis802_11InfrastructureMax+1)
#endif

struct	wlan_network {
	struct list_head list;
	int	network_type;	/*refer to ieee80211.h for WIRELESS_11A/B/G */
	int	fixed;		/* set to fixed when not to be removed asi
				 * site-surveying */
	unsigned int	last_scanned; /*timestamp for the network */
	int	aid;		/*will only be valid when a BSS is joined. */
	int	join_res;
	struct ndis_wlan_bssid_ex network; /*must be the last item */
};

enum VRTL_CARRIER_SENSE {
	DISABLE_VCS,
	ENABLE_VCS,
	AUTO_VCS
};

enum VCS_TYPE {
	NONE_VCS,
	RTS_CTS,
	CTS_TO_SELF
};

#define PWR_CAM 0
#define PWR_MINPS 1
#define PWR_MAXPS 2
#define PWR_UAPSD 3
#define PWR_VOIP 4

enum UAPSD_MAX_SP {
	NO_LIMIT,
	TWO_MSDU,
	FOUR_MSDU,
	SIX_MSDU
};

#define NUM_PRE_AUTH_KEY 16
#define NUM_PMKID_CACHE NUM_PRE_AUTH_KEY

/*
 *	WPA2
 */
struct wlan_bssid_ex {
	u32 Length;
	unsigned char  MacAddress[6];
	u8  Reserved[2];
	struct ndis_802_11_ssid  Ssid;
	u32 Privacy;
	s32 Rssi;
	enum NDIS_802_11_NETWORK_TYPE  NetworkTypeInUse;
	struct NDIS_802_11_CONFIGURATION  Configuration;
	enum NDIS_802_11_NETWORK_INFRASTRUCTURE  InfrastructureMode;
	NDIS_802_11_RATES_EX  SupportedRates;
	u32 IELength;
	u8  IEs[MAX_IE_SZ];	/* (timestamp, beacon interval, and capability
				 * information) */
};

#endif /* #ifndef WLAN_BSSDEF_H_ */

