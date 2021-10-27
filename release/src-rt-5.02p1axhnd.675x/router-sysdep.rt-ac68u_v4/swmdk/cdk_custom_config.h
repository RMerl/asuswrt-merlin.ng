/*
* <:copyright-BRCM:2012:proprietary:standard
* 
*    Copyright (c) 2012 Broadcom 
*    All Rights Reserved
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
 
*/
#ifndef __CDK_CUSTOM_CONFIG_H__
#define __CDK_CUSTOM_CONFIG_H__

/*******************************************************************************
 *
 * CDK_CUSTOM_CONFIG File for the Linux User mode Example
 *
 ******************************************************************************/

#define CDK_CONFIG_MAX_UNITS                    2

/*
 * Our configuration of the software will use the system built-in data types
 */
#include <inttypes.h>
#include <stdlib.h>

/*
 * Instruct the CDK to inherit these types directly, rather than defining them. 
 */
#define CDK_CONFIG_DEFINE_SIZE_T                0
#define CDK_CONFIG_DEFINE_UINT8_T               0
#define CDK_CONFIG_DEFINE_UINT16_T              0
#define CDK_CONFIG_DEFINE_UINT32_T              0
#define CDK_CONFIG_DEFINE_PRIu32                0
#define CDK_CONFIG_DEFINE_PRIx32                0


/*
 * Instruct the CDK to use the system-provided LIBC
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#define CDK_ABORT                               abort
#define CDK_PRINTF                              cdk_printf
#define CDK_VPRINTF                             vprintf
#define CDK_SPRINTF                             sprintf
#define CDK_VSPRINTF                            vsprintf
#define CDK_VSNPRINTF                           vsnprintf
#define CDK_ATOI                                atoi
#define CDK_STRNCHR                             strnchr
#define CDK_STRCPY                              strcpy
#define CDK_STRNCPY                             strncpy
#define CDK_STRLEN                              strlen
#define CDK_STRCHR                              strchr
#define CDK_STRRCHR                             strrchr
#define CDK_STRCMP                              strcmp
#define CDK_MEMCMP                              memcmp
#define CDK_MEMSET                              memset
#define CDK_MEMCPY                              memcpy
#define CDK_STRUPR                              strupr
#define CDK_TOUPPER                             toupper
#define CDK_STRCAT                              strcat

//#define CDK_CONFIG_INCLUDE_DEBUG                0
#define CDK_CONFIG_INCLUDE_CHIP_SYMBOLS         0 
#define CDK_CONFIG_INCLUDE_FIELD_INFO           0 
#define CDK_CONFIG_INCLUDE_FIELD_NAMES          0 
#define CDK_CONFIG_SHELL_INCLUDE_DEFAULT        0
#define CDK_CONFIG_SHELL_INCLUDE_DEBUG          1

#endif /* __CDK_CUSTOM_CONFIG_H__ */
