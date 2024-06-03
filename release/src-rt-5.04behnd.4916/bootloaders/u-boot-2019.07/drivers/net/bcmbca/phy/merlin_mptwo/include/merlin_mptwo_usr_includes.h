// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    
*/

/** @file merlin_mptwo_usr_includes.h
 * Header file which includes all required std libraries and macros
 */

/* The user is expected to replace the macro definitions with their required implementation */

#ifndef MERLIN_MPTWO_API_USR_INCLUDES_H
#define MERLIN_MPTWO_API_USR_INCLUDES_H

/* Standard libraries that can be replaced by your custom libraries */
#ifdef _MSC_VER
/* Enclose all standard headers in a pragma to remove warings for MS compiler */
#pragma warning( push, 0 )
#endif
#ifndef EXCLUDE_STD_HEADERS
#include <stdint.h>
#include <string.h>
#endif

#ifdef _MSC_VER
#pragma warning( pop )
#endif

/* Redefine macros according your compiler requirements */
#define USR_PRINTF(paren_arg_list)    ((void)usr_logger_write paren_arg_list)
#define USR_MEMSET(mem, val, num)     memset(mem, val, num)
#define USR_STRLEN(string)            strlen(string)
#define USR_STRNCAT(str1, str2)        strcat(str1, str2)
#define USR_STRCPY(str1, str2)        strcpy(str1, str2)
#define USR_STRCMP(str1, str2)        strcmp(str1, str2)
#define USR_STRNCMP(str1, str2, num)  strncmp(str1, str2, num)

#ifdef SERDES_API_FLOATING_POINT
#define USR_DOUBLE                    double
#else
#define USR_DOUBLE       int
#define double       undefined
#define float        undefined
#endif

/* Implementation specific macros below */
#define usr_logger_write(...) logger_write(-1,__VA_ARGS__)

#endif
