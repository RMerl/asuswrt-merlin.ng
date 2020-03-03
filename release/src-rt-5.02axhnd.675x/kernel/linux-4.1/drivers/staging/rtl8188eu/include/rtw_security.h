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
#ifndef __RTW_SECURITY_H_
#define __RTW_SECURITY_H_

#include <osdep_service.h>
#include <drv_types.h>

#define _NO_PRIVACY_			0x0
#define _WEP40_				0x1
#define _TKIP_				0x2
#define _TKIP_WTMIC_			0x3
#define _AES_				0x4
#define _WEP104_			0x5
#define _WEP_WPA_MIXED_			0x07  /*  WEP + WPA */
#define _SMS4_				0x06

#define is_wep_enc(alg) (((alg) == _WEP40_) || ((alg) == _WEP104_))

#define _WPA_IE_ID_	0xdd
#define _WPA2_IE_ID_	0x30

#define SHA256_MAC_LEN 32
#define AES_BLOCK_SIZE 16
#define AES_PRIV_SIZE (4 * 44)

enum {
	ENCRYP_PROTOCOL_OPENSYS,   /* open system */
	ENCRYP_PROTOCOL_WEP,       /* WEP */
	ENCRYP_PROTOCOL_WPA,       /* WPA */
	ENCRYP_PROTOCOL_WPA2,      /* WPA2 */
	ENCRYP_PROTOCOL_WAPI,      /* WAPI: Not support in this version */
	ENCRYP_PROTOCOL_MAX
};


#ifndef Ndis802_11AuthModeWPA2
#define Ndis802_11AuthModeWPA2 (Ndis802_11AuthModeWPANone + 1)
#endif

#ifndef Ndis802_11AuthModeWPA2PSK
#define Ndis802_11AuthModeWPA2PSK (Ndis802_11AuthModeWPANone + 2)
#endif

union pn48	{
	u64	val;

#ifdef __LITTLE_ENDIAN
	struct {
		u8 TSC0;
		u8 TSC1;
		u8 TSC2;
		u8 TSC3;
		u8 TSC4;
		u8 TSC5;
		u8 TSC6;
		u8 TSC7;
	} _byte_;

#elif defined(__BIG_ENDIAN)

	struct {
		u8 TSC7;
		u8 TSC6;
		u8 TSC5;
		u8 TSC4;
		u8 TSC3;
		u8 TSC2;
		u8 TSC1;
		u8 TSC0;
	} _byte_;
#endif
};

union Keytype {
	u8   skey[16];
	u32    lkey[4];
};

struct rt_pmkid_list {
	u8	bUsed;
	u8	Bssid[6];
	u8	PMKID[16];
	u8	SsidBuf[33];
	u8	*ssid_octet;
	u16	ssid_length;
};

struct security_priv {
	u32	  dot11AuthAlgrthm;	/*  802.11 auth, could be open,
					 * shared, 8021x and authswitch */
	u32	  dot11PrivacyAlgrthm;	/*  This specify the privacy for
					 * shared auth. algorithm. */
	/* WEP */
	u32	  dot11PrivacyKeyIndex;	/*  this is only valid for legendary
					 * wep, 0~3 for key id.(tx key index) */
	union Keytype dot11DefKey[4];	/*  this is only valid for def. key */
	u32	dot11DefKeylen[4];
	u32 dot118021XGrpPrivacy;	/*  This specify the privacy algthm.
					 * used for Grp key */
	u32	dot118021XGrpKeyid;	/*  key id used for Grp Key
					 * ( tx key index) */
	union Keytype	dot118021XGrpKey[4];	/*  802.1x Group Key,
						 * for inx0 and inx1 */
	union Keytype	dot118021XGrptxmickey[4];
	union Keytype	dot118021XGrprxmickey[4];
	union pn48	dot11Grptxpn;		/* PN48 used for Grp Key xmit.*/
	union pn48	dot11Grprxpn;		/* PN48 used for Grp Key recv.*/
#ifdef CONFIG_88EU_AP_MODE
	/* extend security capabilities for AP_MODE */
	unsigned int dot8021xalg;/* 0:disable, 1:psk, 2:802.1x */
	unsigned int wpa_psk;/* 0:disable, bit(0): WPA, bit(1):WPA2 */
	unsigned int wpa_group_cipher;
	unsigned int wpa2_group_cipher;
	unsigned int wpa_pairwise_cipher;
	unsigned int wpa2_pairwise_cipher;
#endif
	u8 wps_ie[MAX_WPS_IE_LEN];/* added in assoc req */
	int wps_ie_len;
	u8	binstallGrpkey;
	u8	busetkipkey;
	u8	bcheck_grpkey;
	u8	bgrpkey_handshake;
	s32	sw_encrypt;/* from registry_priv */
	s32	sw_decrypt;/* from registry_priv */
	s32	hw_decrypted;/* if the rx packets is hw_decrypted==false,i
			      * it means the hw has not been ready. */

	/* keeps the auth_type & enc_status from upper layer
	 * ioctl(wpa_supplicant or wzc) */
	u32 ndisauthtype;	/*  NDIS_802_11_AUTHENTICATION_MODE */
	u32 ndisencryptstatus;	/*  NDIS_802_11_ENCRYPTION_STATUS */
	struct wlan_bssid_ex sec_bss;  /* for joinbss (h2c buffer) usage */
	struct ndis_802_11_wep ndiswep;
	u8 assoc_info[600];
	u8 szofcapability[256]; /* for wpa2 usage */
	u8 oidassociation[512]; /* for wpa/wpa2 usage */
	u8 authenticator_ie[256];  /* store ap security information element */
	u8 supplicant_ie[256];  /* store sta security information element */

	/* for tkip countermeasure */
	u32 last_mic_err_time;
	u8	btkip_countermeasure;
	u8	btkip_wait_report;
	u32 btkip_countermeasure_time;

	/*  */
	/*  For WPA2 Pre-Authentication. */
	/*  */
	struct rt_pmkid_list PMKIDList[NUM_PMKID_CACHE];
	u8	PMKIDIndex;
	u8 bWepDefaultKeyIdxSet;
};

struct sha256_state {
	u64 length;
	u32 state[8], curlen;
	u8 buf[64];
};

#define GET_ENCRY_ALGO(psecuritypriv, psta, encry_algo, bmcst)		\
do {									\
	switch (psecuritypriv->dot11AuthAlgrthm) {			\
	case dot11AuthAlgrthm_Open:					\
	case dot11AuthAlgrthm_Shared:					\
	case dot11AuthAlgrthm_Auto:					\
		encry_algo = (u8)psecuritypriv->dot11PrivacyAlgrthm;	\
		break;							\
	case dot11AuthAlgrthm_8021X:					\
		if (bmcst)						\
			encry_algo = (u8)psecuritypriv->dot118021XGrpPrivacy;\
		else							\
			encry_algo = (u8)psta->dot118021XPrivacy;	\
		break;							\
	case dot11AuthAlgrthm_WAPI:					\
		encry_algo = (u8)psecuritypriv->dot11PrivacyAlgrthm;	\
		break;							\
	}								\
} while (0)

#define SET_ICE_IV_LEN(iv_len, icv_len, encrypt)			\
do {									\
	switch (encrypt) {						\
	case _WEP40_:							\
	case _WEP104_:							\
		iv_len = 4;						\
		icv_len = 4;						\
		break;							\
	case _TKIP_:							\
		iv_len = 8;						\
		icv_len = 4;						\
		break;							\
	case _AES_:							\
		iv_len = 8;						\
		icv_len = 8;						\
		break;							\
	case _SMS4_:							\
		iv_len = 18;						\
		icv_len = 16;						\
		break;							\
	default:							\
		iv_len = 0;						\
		icv_len = 0;						\
		break;							\
	}								\
} while (0)


#define GET_TKIP_PN(iv, dot11txpn)					\
do {									\
	dot11txpn._byte_.TSC0 = iv[2];					\
	dot11txpn._byte_.TSC1 = iv[0];					\
	dot11txpn._byte_.TSC2 = iv[4];					\
	dot11txpn._byte_.TSC3 = iv[5];					\
	dot11txpn._byte_.TSC4 = iv[6];					\
	dot11txpn._byte_.TSC5 = iv[7];					\
} while (0)


#define ROL32(A, n)	(((A) << (n)) | (((A)>>(32-(n)))  & ((1UL << (n)) - 1)))
#define ROR32(A, n)	ROL32((A), 32-(n))

struct mic_data {
	u32  K0, K1;         /*  Key */
	u32  L, R;           /*  Current state */
	u32  M;              /*  Message accumulator (single word) */
	u32  nBytesInM;      /*  # bytes in M */
};

extern const u32 Te0[256];
extern const u32 Td0[256];
extern const u32 Td1[256];
extern const u32 Td2[256];
extern const u32 Td3[256];
extern const u32 Td4[256];
extern const u32 rcon[10];
extern const u8 Td4s[256];
extern const u8 rcons[10];

#define RCON(i) (rcons[(i)] << 24)

static inline u32 rotr(u32 val, int bits)
{
	return (val >> bits) | (val << (32 - bits));
}

#define TE0(i) Te0[((i) >> 24) & 0xff]
#define TE1(i) rotr(Te0[((i) >> 16) & 0xff], 8)
#define TE2(i) rotr(Te0[((i) >> 8) & 0xff], 16)
#define TE3(i) rotr(Te0[(i) & 0xff], 24)

#define GETU32(pt) (((u32)(pt)[0] << 24) ^ ((u32)(pt)[1] << 16) ^ \
			((u32)(pt)[2] <<  8) ^ ((u32)(pt)[3]))

#define PUTU32(ct, st) { \
(ct)[0] = (u8)((st) >> 24); (ct)[1] = (u8)((st) >> 16); \
(ct)[2] = (u8)((st) >>  8); (ct)[3] = (u8)(st); }

#define WPA_GET_BE32(a) ((((u32)(a)[0]) << 24) | (((u32)(a)[1]) << 16) | \
			 (((u32)(a)[2]) << 8) | ((u32)(a)[3]))

#define WPA_PUT_LE16(a, val)			\
	do {					\
		(a)[1] = ((u16)(val)) >> 8;	\
		(a)[0] = ((u16)(val)) & 0xff;	\
	} while (0)

#define WPA_PUT_BE32(a, val)					\
	do {							\
		(a)[0] = (u8)((((u32)(val)) >> 24) & 0xff);	\
		(a)[1] = (u8)((((u32)(val)) >> 16) & 0xff);	\
		(a)[2] = (u8)((((u32)(val)) >> 8) & 0xff);	\
		(a)[3] = (u8)(((u32)(val)) & 0xff);		\
	} while (0)

#define WPA_PUT_BE64(a, val)				\
	do {						\
		(a)[0] = (u8)(((u64)(val)) >> 56);	\
		(a)[1] = (u8)(((u64)(val)) >> 48);	\
		(a)[2] = (u8)(((u64)(val)) >> 40);	\
		(a)[3] = (u8)(((u64)(val)) >> 32);	\
		(a)[4] = (u8)(((u64)(val)) >> 24);	\
		(a)[5] = (u8)(((u64)(val)) >> 16);	\
		(a)[6] = (u8)(((u64)(val)) >> 8);	\
		(a)[7] = (u8)(((u64)(val)) & 0xff);	\
	} while (0)

/* ===== start - public domain SHA256 implementation ===== */

/* This is based on SHA256 implementation in LibTomCrypt that was released into
 * public domain by Tom St Denis. */

/* the K array */
static const unsigned long K[64] = {
	0x428a2f98UL, 0x71374491UL, 0xb5c0fbcfUL, 0xe9b5dba5UL, 0x3956c25bUL,
	0x59f111f1UL, 0x923f82a4UL, 0xab1c5ed5UL, 0xd807aa98UL, 0x12835b01UL,
	0x243185beUL, 0x550c7dc3UL, 0x72be5d74UL, 0x80deb1feUL, 0x9bdc06a7UL,
	0xc19bf174UL, 0xe49b69c1UL, 0xefbe4786UL, 0x0fc19dc6UL, 0x240ca1ccUL,
	0x2de92c6fUL, 0x4a7484aaUL, 0x5cb0a9dcUL, 0x76f988daUL, 0x983e5152UL,
	0xa831c66dUL, 0xb00327c8UL, 0xbf597fc7UL, 0xc6e00bf3UL, 0xd5a79147UL,
	0x06ca6351UL, 0x14292967UL, 0x27b70a85UL, 0x2e1b2138UL, 0x4d2c6dfcUL,
	0x53380d13UL, 0x650a7354UL, 0x766a0abbUL, 0x81c2c92eUL, 0x92722c85UL,
	0xa2bfe8a1UL, 0xa81a664bUL, 0xc24b8b70UL, 0xc76c51a3UL, 0xd192e819UL,
	0xd6990624UL, 0xf40e3585UL, 0x106aa070UL, 0x19a4c116UL, 0x1e376c08UL,
	0x2748774cUL, 0x34b0bcb5UL, 0x391c0cb3UL, 0x4ed8aa4aUL, 0x5b9cca4fUL,
	0x682e6ff3UL, 0x748f82eeUL, 0x78a5636fUL, 0x84c87814UL, 0x8cc70208UL,
	0x90befffaUL, 0xa4506cebUL, 0xbef9a3f7UL, 0xc67178f2UL
};

/* Various logical functions */
#define RORc(x, y) \
	(((((unsigned long)(x) & 0xFFFFFFFFUL) >> (unsigned long)((y)&31)) | \
	 ((unsigned long)(x) << (unsigned long)(32-((y)&31)))) & 0xFFFFFFFFUL)
#define Ch(x, y , z)       (z ^ (x & (y ^ z)))
#define Maj(x, y, z)      (((x | y) & z) | (x & y))
#define S(x, n)         RORc((x), (n))
#define R(x, n)         (((x)&0xFFFFFFFFUL)>>(n))
#define Sigma0(x)       (S(x, 2) ^ S(x, 13) ^ S(x, 22))
#define Sigma1(x)       (S(x, 6) ^ S(x, 11) ^ S(x, 25))
#define Gamma0(x)       (S(x, 7) ^ S(x, 18) ^ R(x, 3))
#define Gamma1(x)       (S(x, 17) ^ S(x, 19) ^ R(x, 10))
#ifndef MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif

void rtw_secmicsetkey(struct mic_data *pmicdata, u8 *key);
void rtw_secmicappendbyte(struct mic_data *pmicdata, u8 b);
void rtw_secmicappend(struct mic_data *pmicdata, u8 *src, u32 nBytes);
void rtw_secgetmic(struct mic_data *pmicdata, u8 *dst);
void rtw_seccalctkipmic(u8 *key, u8 *header, u8 *data, u32 data_len,
			u8 *Miccode, u8   priority);
u32 rtw_aes_encrypt(struct adapter *padapter, u8 *pxmitframe);
u32 rtw_tkip_encrypt(struct adapter *padapter, u8 *pxmitframe);
void rtw_wep_encrypt(struct adapter *padapter, u8  *pxmitframe);
u32 rtw_aes_decrypt(struct adapter *padapter, u8  *precvframe);
u32 rtw_tkip_decrypt(struct adapter *padapter, u8  *precvframe);
void rtw_wep_decrypt(struct adapter *padapter, u8  *precvframe);

#endif	/* __RTL871X_SECURITY_H_ */
