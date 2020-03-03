/***********************************************************************
 *
 *  Copyright (c) 2013  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2013:proprietary:standard
 *
 *  This program is the proprietary software of Broadcom and/or its
 *  licensors, and may only be used, duplicated, modified or distributed pursuant
 *  to the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied), right
 *  to use, or waiver of any kind with respect to the Software, and Broadcom
 *  expressly reserves all rights in and to the Software and all intellectual
 *  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 *  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 *  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1. This program, including its structure, sequence and organization,
 *     constitutes the valuable trade secrets of Broadcom, and you shall use
 *     all reasonable efforts to protect the confidentiality thereof, and to
 *     use this information only in connection with your use of Broadcom
 *     integrated circuit products.
 *
 *  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
 *     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
 *     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
 *     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
 *     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
 *     PERFORMANCE OF THE SOFTWARE.
 *
 *  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
 *     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
 *     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 *     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
 *     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
 *     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
 *     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
 *     LIMITED REMEDY.
 * :>
 *
 * $Change: 111969 $
 ***********************************************************************/

#ifndef __IEEE1905_UTILS_H__
#define __IEEE1905_UTILS_H__

#include "ieee1905_datamodel_priv.h"

#define I5_MSEC_USEC(x) ((x) * 1000)
#define I5_SEC_MSEC(x) ((x) * 1000)

#define I5_WARN()   \
    do { \
       printf("[%s.%d]: I5 WARN\n", __func__, __LINE__); \
    } while(0)

#define I5_WARN_ON(x)  \
        do { \
           if( __builtin_expect(!!(x), 1==0) ) { \
               printf("[%s.%d]: I5 WARNING: " #x "\n", __func__, __LINE__); \
           } \
        } while(0)

#define I5_WARN_ON_ONCE(x)  \
        do { \
           static int _doOnce = 1; \
           if( __builtin_expect(!!(x), 1==0) && _doOnce ) { \
               printf("[%s.%d]: I5 WARNING: " #x "\n", __func__, __LINE__); \
               _doOnce = 0; \
           } \
        } while(0)

#define I5STRNCPY(dst, src, len)	 \
	do { \
		strncpy((dst), (src), (len) -1); \
		(dst)[(len) - 1] = '\0'; \
	} while (0)

static unsigned char const i5UtilsInterfaceNamesForMediaTypes [][16] __attribute__((unused)) =
{"Eth", "Eth",
 "Wi-Fi 11b", "Wi-Fi 11g", "Wi-Fi 11a", "Wi-Fi 11n24", "Wi-Fi 11n5", "Wi-Fi 11ac", "Wi-Fi 11ad", "Wi-Fi 11af",
 "PLC", "PLC",
 "MoCA",
 "Unknown"};

static unsigned char const* i5UtilsGetNameForMediaType(unsigned short mediaType) __attribute__((unused));
static unsigned char const* i5UtilsGetNameForMediaType(unsigned short mediaType)
{
   switch (mediaType) {
      case I5_MEDIA_TYPE_FAST_ETH:
         return i5UtilsInterfaceNamesForMediaTypes[0];
      case I5_MEDIA_TYPE_GIGA_ETH:
         return i5UtilsInterfaceNamesForMediaTypes[1];
      case I5_MEDIA_TYPE_WIFI_B:
         return i5UtilsInterfaceNamesForMediaTypes[2];
      case I5_MEDIA_TYPE_WIFI_G:
         return i5UtilsInterfaceNamesForMediaTypes[3];
      case I5_MEDIA_TYPE_WIFI_A:
         return i5UtilsInterfaceNamesForMediaTypes[4];
      case I5_MEDIA_TYPE_WIFI_N24:
         return i5UtilsInterfaceNamesForMediaTypes[5];
      case I5_MEDIA_TYPE_WIFI_N5:
         return i5UtilsInterfaceNamesForMediaTypes[6];
      case I5_MEDIA_TYPE_WIFI_AC:
         return i5UtilsInterfaceNamesForMediaTypes[7];
      case I5_MEDIA_TYPE_WIFI_AD:
         return i5UtilsInterfaceNamesForMediaTypes[8];
      case I5_MEDIA_TYPE_WIFI_AF:
         return i5UtilsInterfaceNamesForMediaTypes[9];
      case I5_MEDIA_TYPE_1901_WAVELET:
         return i5UtilsInterfaceNamesForMediaTypes[10];
      case I5_MEDIA_TYPE_1901_FFT:
         return i5UtilsInterfaceNamesForMediaTypes[11];
      case I5_MEDIA_TYPE_MOCA_V11:
         return i5UtilsInterfaceNamesForMediaTypes[12];
      default:
         return i5UtilsInterfaceNamesForMediaTypes[13];
   }
}

static int i5strnlen(const char *str, int max) {
  // note: strnlen is not supported on all compilers
  const char * end = memchr(str, '\0', max+1);
  if (end && *end == '\0') {
       return end-str;
  }
  return -1;
}

#define I5_MAC_STR_BUF_LEN      18
#define I5_MAC_FMT    "%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX"
#define I5_MAC_DELIM_FMT    "%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX"
#define I5_MAC_PRM(x) ((unsigned char *)(x))[0], ((unsigned char *)(x))[1], ((unsigned char *)(x))[2], ((unsigned char *)(x))[3], ((unsigned char *)(x))[4], ((unsigned char *)(x))[5]
#define I5_MAC_SCANF  "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx"
#define I5_MAC_SCANF1 "%2hhx%2hhx:%2hhx%2hhx:%2hhx%2hhx"
#define I5_MAC_SCANF2 "%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx"
#define I5_MAC_SCANF_PRM(x) &((x)[0]), &((x)[1]), &((x)[2]), &((x)[3]), &((x)[4]), &((x)[5])

static inline char * i5MacAddr2String(const unsigned char *macAddr, char *string) {
    if (macAddr && string) {
        snprintf(string, I5_MAC_STR_BUF_LEN, I5_MAC_DELIM_FMT, I5_MAC_PRM(macAddr));
        return string;
    }
    return NULL;
}

static inline unsigned char * i5String2MacAddr(const char *string, unsigned char *macAddr) {
    int rt;
    if (macAddr && string) {
        rt = sscanf(string, "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx", I5_MAC_SCANF_PRM(macAddr));
        if (rt != 6 && i5strnlen(string,14)==14)
            rt = sscanf(string, "%2hhx%2hhx:%2hhx%2hhx:%2hhx%2hhx", I5_MAC_SCANF_PRM(macAddr));
        if (rt != 6 && i5strnlen(string,12)==12)
            rt = sscanf(string, "%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx", I5_MAC_SCANF_PRM(macAddr));
        if (rt == 6)
            return macAddr;
    }
    return NULL;
}

static inline long getDeltaTimeInMs( struct timespec *t1, struct timespec *t2)
{
    long secs;
    long nsecs;

    if (t1->tv_nsec <= t2->tv_nsec) {
        secs = t2->tv_sec-t1->tv_sec;
        nsecs = t2->tv_nsec-t1->tv_nsec;
    }
    else {
        secs = t2->tv_sec - t1->tv_sec+1;
        nsecs = t2->tv_nsec+1000000000 - t1->tv_nsec;
    }

    return secs*1000 + nsecs/1000000;
}

#endif // endif
