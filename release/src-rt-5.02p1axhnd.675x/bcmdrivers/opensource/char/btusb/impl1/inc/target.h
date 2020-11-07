/*
 * <:copyright-BRCM:2015:GPL/GPL:standard
 * 
 *    Copyright (c) 2015 Broadcom 
 *    All Rights Reserved
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 * :>
 */
 
#ifndef _TARGET_H
#define _TARGET_H

#include <linux/usb.h>

#define BT_API 
#define RFC_API 
#define L2C_API 

#define INT   int
#define UINT  unsigned int
#define DWORD unsigned long

#ifndef BOOL
#define BOOL int
#endif

#ifndef USHORT
#define USHORT unsigned short
#endif

#ifndef ULONG
#define ULONG unsigned long
#endif

#ifndef VOID
#define VOID void
#endif

#define _vsnprintf vsnprintf

#define __cdecl

#define GKI_USE_DYNAMIC_BUFFERS     TRUE           /* TRUE if using dynamic buffers */

#define GKI_NUM_FIXED_BUF_POOLS     1
#define GKI_DEF_BUFPOOL_PERM_MASK   0xfffc          /* Pool ID 0 is public */

/* Define the total number of buffer pools used, fixed and dynamic (Maximum is 16) */
#define GKI_NUM_TOTAL_BUF_POOLS 1

#define GKI_BUF0_SIZE           1740
#ifdef BTUSB_LITE
#define GKI_BUF0_MAX            10
#else
#define GKI_BUF0_MAX            5
#endif

#define GKI_POOL_ID_0           0

#define HCI_SCO_POOL_ID         GKI_POOL_ID_0   // all SCO data to/from the device

/* Set this flag to non zero if you want to do buffer corruption checks.
** If set, GKI will check buffer tail corruption every time it processes
** a buffer. This is very useful for debug, and is minimal overhead in
** a running system.
*/
#define GKI_ENABLE_BUF_CORRUPTION_CHECK 1

#define L2CAP_MTU_SIZE                  1691
//#define L2CAP_MTU_SIZE            200

/* Maximum size in bytes of the codec capabilities information element. */
#ifndef AVDT_CODEC_SIZE
#define AVDT_CODEC_SIZE                 10
#endif

/* Number of streams for dual stack */
#ifndef BTM_SYNC_INFO_NUM_STR
#define BTM_SYNC_INFO_NUM_STR           2
#endif

/* Number of streams for dual stack in BT Controller (simulation) */
#ifndef BTM_SYNC_INFO_NUM_STR_BTC
#define BTM_SYNC_INFO_NUM_STR_BTC       2
#endif

#ifndef BTA_AV_NUM_STRS
#define BTA_AV_NUM_STRS                 2
#endif

#define BTU_STACK_LITE_ENABLED TRUE
#define BTU_MULTI_AV_INCLUDED TRUE

/* Number of simultaneous links to different peer devices. */
#ifndef AVDT_NUM_LINKS
#define AVDT_NUM_LINKS              2
#endif

/* Number of simultaneous stream endpoints. */
#ifndef AVDT_NUM_SEPS
#define AVDT_NUM_SEPS               2
#endif

#endif 
