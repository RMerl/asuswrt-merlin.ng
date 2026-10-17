/*
   Copyright (c) 2024 Broadcom Corporation
   All Rights Reserved

   <:label-BRCM:2024:DUAL/GPL:standard

   Unless you and Broadcom execute a separate written software license
   agreement governing use of this software, this software is licensed
   to you under the terms of the GNU General Public License version 2
   (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
   with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

   Not withstanding the above, under no circumstances may you combine
   this software in any way with any other Broadcom software provided
   under a license other than the GPL, without Broadcom's express prior
   written consent.

   :>
*/


/**************************************************************************************
 *  File Name     :  merlin8_phy1_usr_includes.h                                        *
 *  Created On    :  05/07/2014                                                       *
 *  Created By    :  Kiran Divakar                                                    *
 *  Description   :  Header file which includes all required std libraries and macros *
 *  Revision      :   *
 *                                                                                    *
 **************************************************************************************
 **************************************************************************************/

#ifndef MERLIN8_PHY1_API_USR_INCLUDES_H
#define MERLIN8_PHY1_API_USR_INCLUDES_H

/* Standard libraries that can be replaced by your custom libraries */
#ifdef _MSC_VER
/* Enclose all standard headers in a pragma to remove warings for MS compiler */
#pragma warning( push, 0 )
#endif
#ifndef EXCLUDE_STD_HEADERS
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#endif

#include "merlin8_phy1_ipconfig.h"

/*! @file
 *  @brief Header file which includes all required std libraries and macros
 *
 * The user is expected to replace the macro definitions with their required implementation
 */

/*! @addtogroup APITag
 * @{
 */

/*! @defgroup SerdesAPIUsrIncludesTag User Provided Macros
 * Contains macros with default definitions which can be redefined
 * by users if desired.
 */

/*! @addtogroup SerdesAPIUsrIncludesTag
 * @{
 */

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#if defined _MSC_VER
#define API_FUNCTION_NAME __FUNCTION__
#else
#define API_FUNCTION_NAME __func__
#endif
/* Redefine macros according your compiler requirements */
#ifndef SMALL_FOOTPRINT
# ifdef SDK_LIBRARY
#  define USR_PRINTF(paren_arg_list)    ((void)printf paren_arg_list)
# else
#  define USR_PRINTF(paren_arg_list)    ((void)usr_logger_write paren_arg_list)
# endif
/*! @def USR_VPRINTF
 * Is likely not being used, and may be removed in the future.
 * usr_logger_verbose_write() appears broken (no 'sa__' arg)
 */
#define USR_VPRINTF(paren_arg_list)   ((void)usr_logger_verbose_write paren_arg_list)
#else
#define USR_PRINTF(paren_arg_list)
#endif

#define USR_MEMSET(mem, val, num)     memset(mem, val, num)
#define USR_STRLEN(string)            strlen(string)
#define USR_STRNCAT(str1, str2, num)  strncat(str1, str2, num)
#define USR_STRCPY(str1, str2)        strcpy(str1, str2)
#define USR_STRCMP(str1, str2)        strcmp(str1, str2)
#define USR_STRNCMP(str1, str2, num)  strncmp(str1, str2, num)
#define USR_SNPRINTF                  snprintf
#if defined(__KERNEL__)
#define USR_CALLOC(nmemb, size)       kmalloc(nmemb*size, GFP_KERNEL)
#define USR_FREE(ptr)                 kfree(ptr)
#else
#define USR_CALLOC(nmemb, size)       calloc(nmemb, size)
#define USR_FREE(ptr)                 free(ptr)
#endif
#define USR_UINTPTR                   uintptr_t


#ifdef SERDES_API_FLOATING_POINT
#define USR_DOUBLE                    double
#else
#define USR_DOUBLE       int
#define double       undefined
#define float        undefined
#endif

/* Mutex Definitions */
#ifndef NO_VARIADIC_MACROS
#define USR_CREATE_LOCK
#define USR_ACQUIRE_LOCK
#define USR_RELEASE_LOCK
#define USR_DESTROY_LOCK
#endif

/* Implementation specific macros below */
#ifndef NO_VARIADIC_MACROS
/*! @def usr_logger_write
 * is an API user provided Dependency function. In practice it's return value is
 * never checked. To do so, the following define can be used. If using one of the API error checking
 * macros (like EFUN()) which calls the API error processing functons, the
 * ERR_CODE_SRDS_REG_ACCESS_FAIL error code should be returned to avoid error processing recursion.
 * The (void) cast serves to quiet compiler warnings if so configured, otherwise it can be removed.
 * usr_logger_write(...)  ((void)(logger_write(0, -1, __VA_ARGS__) ? ERR_CODE_SRDS_REG_ACCESS_FAIL : ERR_CODE_NONE))
 * ********************************************************************************************
 */
#define usr_logger_write(...) logger_write(sa__, -1,__VA_ARGS__)

/*! @def usr_logger_verbose_write
 * appears broken (no 'sa__' arg), is likely not being used, and may be removed.
 */
#define usr_logger_verbose_write(...) logger_write(1,__VA_ARGS__)
#endif
#define USR_DELAY_MS(stuff) merlin8_phy1_acc_delay_ms(sa__,stuff)
#define USR_DELAY_US(stuff) merlin8_phy1_acc_delay_us(sa__,stuff)
#define USR_DELAY_NS(stuff) merlin8_phy1_acc_delay_ns(sa__,stuff)
/*! @} SerdesAPIUsrIncludesTag */
/*! @} APITag */
#endif /* !SERDES_API_USR_INCLUDES_H */
