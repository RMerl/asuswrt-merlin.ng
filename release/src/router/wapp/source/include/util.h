/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * Copyright  (C) 2019-2020  MediaTek Inc. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
 
#ifndef __UTIL_H__
#define __UTIL_H__

#include "os.h"
#include "types.h"

#ifdef DPP_SUPPORT
#define OUI_WFA 0x506f9a
#define WLAN_EID_ADV_PROTO 108
#define WLAN_EID_VENDOR_SPECIFIC 221
#define WLAN_ACTION_PUBLIC 4

#define WLAN_PA_VENDOR_SPECIFIC 9
#define WLAN_PA_GAS_INITIAL_REQ 10
#define WLAN_PA_GAS_INITIAL_RESP 11
#define WLAN_PA_GAS_COMEBACK_REQ 12
#define WLAN_PA_GAS_COMEBACK_RESP 13


#define WLAN_ACTION_PROTECTED_DUAL 9
#define WLAN_STATUS_QUERY_RESP_OUTSTANDING 95

#define WLAN_STATUS_SUCCESS 0

/* Advertisement Protocol ID definitions (IEEE Std 802.11-2016, Table 9-215) */
enum adv_proto_id {
	ACCESS_NETWORK_QUERY_PROTOCOL = 0,
	MIH_INFO_SERVICE = 1,
	MIH_CMD_AND_EVENT_DISCOVERY = 2,
	EMERGENCY_ALERT_SYSTEM = 3,
	REGISTERED_LOCATION_QUERY_PROTO = 4,
	ADV_PROTO_VENDOR_SPECIFIC = 221
};

/* DPP Public Action frame identifiers - OUI_WFA */
#define DPP_OUI_TYPE 0x1A

#endif

u8 BtoH(char ch);
void AtoH(char *src, char *dest, int destlen);

size_t os_strlcpy(char *dest, const char *src, size_t siz);
void * os_memdup(const void *src, size_t len);

#define STRUCT_PACKED __attribute__ ((packed))

#ifndef __must_check
#if __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4)
#define __must_check __attribute__((__warn_unused_result__))
#else
#define __must_check
#endif /* __GNUC__ */
#endif /* __must_check */

#define PRINTF_FORMAT(a,b) __attribute__ ((format (printf, (a), (b))))

#define ETH_ALEN 6

/* Endian byte swapping codes */
#define SWAP16(x) \
    ((u16) (\
           (((u16) (x) & (u16) 0x00ffU) << 8) | \
           (((u16) (x) & (u16) 0xff00U) >> 8))) 
 
#define SWAP32(x) \
    ((u32) (\
           (((u32) (x) & (u32) 0x000000ffUL) << 24) | \
           (((u32) (x) & (u32) 0x0000ff00UL) << 8) | \
           (((u32) (x) & (u32) 0x00ff0000UL) >> 8) | \
           (((u32) (x) & (u32) 0xff000000UL) >> 24))) 

#define SWAP64(x) \
    ((u64)( \
    (u64)(((UINT64)(x) & (u64) 0x00000000000000ffULL) << 56) | \
    (u64)(((UINT64)(x) & (u64) 0x000000000000ff00ULL) << 40) | \
    (u64)(((UINT64)(x) & (u64) 0x0000000000ff0000ULL) << 24) | \
    (u64)(((UINT64)(x) & (u64) 0x00000000ff000000ULL) <<  8) | \
    (u64)(((UINT64)(x) & (u64) 0x000000ff00000000ULL) >>  8) | \
    (u64)(((UINT64)(x) & (u64) 0x0000ff0000000000ULL) >> 24) | \
    (u64)(((UINT64)(x) & (u64) 0x00ff000000000000ULL) >> 40) | \
    (u64)(((UINT64)(x) & (u64) 0xff00000000000000ULL) >> 56) ))
 
#ifdef RT_BIG_ENDIAN
#define cpu2le64(x) SWAP64((x))
#define le2cpu64(x) SWAP64((x))
#define cpu2le32(x) SWAP32((x))
#define le2cpu32(x) SWAP32((x))
#define cpu2le16(x) SWAP16((x))
#define le2cpu16(x) SWAP16((x))
#define cpu2be64(x) ((u64)(x))
#define be2cpu64(x) ((u64)(x))
#define cpu2be32(x) ((u32)(x))
#define be2cpu32(x) ((u32)(x))
#define cpu2be16(x) ((u16)(x))
#define be2cpu16(x) ((u16)(x))
#else /* Little_Endian */
#define cpu2le64(x) ((u64)(x))
#define le2cpu64(x) ((u64)(x))
#define cpu2le32(x) ((u32)(x))
#define le2cpu32(x) ((u32)(x))
#define cpu2le16(x) ((u16)(x))
#define le2cpu16(x) ((u16)(x))
#define cpu2be64(x) SWAP64((x))
#define be2cpu64(x) SWAP64((x))
#define cpu2be32(x) SWAP32((x))
#define be2cpu32(x) SWAP32((x))
#define cpu2be16(x) SWAP16((x))
#define be2cpu16(x) SWAP16((x))
#endif /* RT_BIG_ENDIAN */

void * __hide_aliasing_typecast(void *foo);
#define aliasing_hide_typecast(a,t) (t *) __hide_aliasing_typecast((a))

#define SSID_MAX_LEN 32
/* Macros for handling unaligned memory accesses */

static inline u16 WPA_GET_LE16(const u8 *a)
{
	return (a[1] << 8) | a[0];
}

static inline void WPA_PUT_LE16(u8 *a, u16 val)
{
	a[1] = val >> 8;
	a[0] = val & 0xff;
}

static inline void WPA_PUT_BE24(u8 *a, u32 val)
{
	a[0] = (val >> 16) & 0xff;
	a[1] = (val >> 8) & 0xff;
	a[2] = val & 0xff;
}

static inline u32 WPA_GET_BE24(const u8 *a)
{
	return (a[0] << 16) | (a[1] << 8) | a[2];
}

static inline u32 WPA_GET_LE32(const u8 *a)
{
	return ((u32) a[3] << 24) | (a[2] << 16) | (a[1] << 8) | a[0];
}

static inline void WPA_PUT_LE32(u8 *a, u32 val)
{
	a[3] = (val >> 24) & 0xff;
	a[2] = (val >> 16) & 0xff;
	a[1] = (val >> 8) & 0xff;
	a[0] = val & 0xff;
}

static inline u64 WPA_GET_LE64(const u8 *a)
{
	return (((u64) a[7]) << 56) | (((u64) a[6]) << 48) |
		(((u64) a[5]) << 40) | (((u64) a[4]) << 32) |
		(((u64) a[3]) << 24) | (((u64) a[2]) << 16) |
		(((u64) a[1]) << 8) | ((u64) a[0]);
}

static inline void WPA_PUT_LE64(u8 *a, u64 val)
{
	a[7] = val >> 56;
	a[6] = val >> 48;
	a[5] = val >> 40;
	a[4] = val >> 32;
	a[3] = val >> 24;
	a[2] = val >> 16;
	a[1] = val >> 8;
	a[0] = val & 0xff;
}

#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"

int hwaddr_aton2(const char *txt, u8 *addr);
int hex2byte(const char *hex);
int hexstr2bin(const char *hex, u8 *buf, size_t len);
int ascii2hexstr(const char *input, u8 *output);
int os_snprintf_hex(char *buf, size_t buf_size, const u8 *data, size_t len);

void printf_encode(char *txt, size_t maxlen, const u8 *data, size_t len);
const char * os_ssid_txt(const u8 *ssid, size_t ssid_len);

int is_hex(const u8 *data, size_t len);

static inline int is_zero_ether_addr(const u8 *a)
{
	return !(a[0] | a[1] | a[2] | a[3] | a[4] | a[5]);
}

static inline int is_broadcast_ether_addr(const u8 *a)
{
	return (a[0] & a[1] & a[2] & a[3] & a[4] & a[5]) == 0xff;
}

static inline int is_multicast_ether_addr(const u8 *a)
{
	return a[0] & 0x01;
}
#ifdef DPP_SUPPORT
#include "wpa_debug.h"
#endif /*DPP_SUPPORT*/

#define broadcast_ether_addr (const u8 *) "\xff\xff\xff\xff\xff\xff"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

void str_clear_free(char *str);
void bin_clear_free(void *bin, size_t len);

int is_ctrl_char(char c);
char * get_param(const char *cmd, const char *param);

#endif /* __UTIL_H__ */
