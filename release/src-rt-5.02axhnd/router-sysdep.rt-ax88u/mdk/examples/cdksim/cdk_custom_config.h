/*
 * $Id: cdk_custom_config.h,v 1.5 Broadcom SDK $
 * $Copyright: Copyright 2013 Broadcom Corporation.
 * This program is the proprietary software of Broadcom Corporation
 * and/or its licensors, and may only be used, duplicated, modified
 * or distributed pursuant to the terms and conditions of a separate,
 * written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized
 * License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software
 * and all intellectual property rights therein.  IF YOU HAVE
 * NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE
 * IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
 * ALL USE OF THE SOFTWARE.  
 *  
 * Except as expressly set forth in the Authorized License,
 *  
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of
 * Broadcom integrated circuit products.
 *  
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS
 * PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY,
 * OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 * 
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL,
 * INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER
 * ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY
 * TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF
 * THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR USD 1.00,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING
 * ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.$1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING
 * ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.$
 *
 */

#ifndef __CDK_CUSTOM_CONFIG_H__
#define __CDK_CUSTOM_CONFIG_H__

/*******************************************************************************
 *
 * Basic types
 *
 * Use size_t and unsigned types provided by system to avoid
 * potential conflicts in type declarations.
 *
 ******************************************************************************/

#include <inttypes.h>

#define CDK_CONFIG_DEFINE_SIZE_T                0
#define CDK_CONFIG_DEFINE_UINT8_T               0
#define CDK_CONFIG_DEFINE_UINT16_T              0
#define CDK_CONFIG_DEFINE_UINT32_T              0
#define CDK_CONFIG_DEFINE_PRIu32                0
#define CDK_CONFIG_DEFINE_PRIx32                0


/*******************************************************************************
 *
 * C library
 *
 * The CDK will use its own C library functions by default. If the
 * system provides a C library, the CDK should be configured to use
 * the system library functions in order to reduce the size of the
 * application.
 *
 ******************************************************************************/

#ifdef USE_SYSTEM_LIBC

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#define CDK_ABORT                               abort
#define CDK_PRINTF                              printf
#define CDK_VPRINTF                             vprintf
#define CDK_SPRINTF                             sprintf
#define CDK_VSPRINTF                            vsprintf
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

#endif /* USE_SYSTEM_LIBC */

/* We increase the default size to include all installed devices */
#define CDK_CONFIG_MAX_UNITS 128

/*******************************************************************************
 *
 * Minimal CDK
 *
 * The CDK is by default fairly small in size, however it is possible
 * to reduce the size by excluding various features.
 *
 ******************************************************************************/

#ifdef MINIMAL_BUILD

/* Exclude all chips by default */
#define CDK_CONFIG_INCLUDE_CHIP_DEFAULT         0

/* Include support for one chip only */
#define CDK_CONFIG_INCLUDE_BCM56504             1

#undef  CDK_CONFIG_MAX_UNITS
#define CDK_CONFIG_MAX_UNITS                    1

/* Exclude all symbols */
#define CDK_CONFIG_INCLUDE_CHIP_SYMBOLS         0
#define CDK_CONFIG_INCLUDE_FIELD_INFO           0

/* Exclude all shell commands by default */
#define CDK_CONFIG_SHELL_INCLUDE_DEFAULT        0

/* Include only GETI and SETI */
#define CDK_CONFIG_SHELL_INCLUDE_GETI           1
#define CDK_CONFIG_SHELL_INCLUDE_SETI           1

/* Exclude help text fo shell commands */
#define CDK_CONFIG_SHELL_INCLUDE_HELP           0

/* Exclude debug messages */
#define CDK_CONFIG_INCLUDE_DEBUG                0

#endif /* CDK_MINIMAL_BUILD */

#endif /* __CDK_CUSTOM_CONFIG_H__ */
