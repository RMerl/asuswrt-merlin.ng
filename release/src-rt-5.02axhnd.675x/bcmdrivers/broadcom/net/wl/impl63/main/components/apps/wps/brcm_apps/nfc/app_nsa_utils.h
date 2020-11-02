/*
 * NSA generic application utility functions
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: $
 */
#ifndef APP_NSA_UTILS_H
#define APP_NSA_UTILS_H

/* self sufficiency */
/* for printf */
#include <stdio.h>
/* for BD_ADDR and DEV_CLASS */
#include "bt_types.h"
/* for scru_dump_hex */
#include "bsa_trace.h"

/* Macro to retrieve the number of elements in a statically allocated array */
#define APP_NUM_ELEMENTS(__a) ((int)(sizeof(__a)/sizeof(__a[0])))

/* Macro to print an error message */
#define APP_ERROR0(format)                                                      \
do {                                                                            \
    app_print_error("%s: " format "\n", __func__);                                \
} while (0)

#define APP_ERROR1(format, ...)                                                 \
do {                                                                            \
    app_print_error("%s: " format "\n", __func__, __VA_ARGS__);                   \
} while (0)

#ifdef APP_TRACE_NODEBUG

#define APP_DEBUG0(format) do {} while (0)
#define APP_DEBUG1(format, ...) do {} while (0)
#define APP_DUMP(prefix, pointer, length) do {} while (0)

#else /* APP_TRACE_NODEBUG */

/* Macro to print a debug message */
#define APP_DEBUG0(format)                                                      \
do {                                                                            \
    app_print_debug("%s: " format "\n", __func__);                              \
} while (0)

#define APP_DEBUG1(format, ...)                                                 \
do {                                                                            \
    app_print_debug("%s: " format "\n", __func__, __VA_ARGS__);                 \
} while (0)

#define APP_DUMP(prefix, pointer, length)                                         \
do                                                                              \
{                                                                               \
    scru_dump_hex(pointer, prefix, length, TRACE_LAYER_NONE, TRACE_TYPE_DEBUG); \
} while (0)

#endif /* !APP_TRACE_NODEBUG */

/* Macro to print an information message */
#define APP_INFO0(format)                                                       \
do {                                                                            \
    app_print_info(format "\n");                                                \
} while (0)

#define APP_INFO1(format, ...)                                                  \
do {                                                                            \
    app_print_info(format "\n", __VA_ARGS__);                                   \
} while (0)

/* This function is used to get readable string from Class of device */
char *app_get_cod_string(const DEV_CLASS class_of_device);

/*
 * Wait for a choice from user
 * Parameters: The string to print before waiting for input
 * Returns: The number typed by the user, or -1 if the value type was not parsable
 *
 */
int app_get_choice(const char *querystring);

/*
 * Ask the user to enter a string value
 * Parameters: querystring: to print before waiting for input
 *                  str: the char buffer to fill with user input
 *                  len: the length of the char buffer
 * Returns: The length of the string entered not including last NULL char
 *             negative value in case of error
 */
int app_get_string(const char *querystring, char *str, int len);

/*
 * This function is used to print an application information message
 * Parameters: format: Format string
 *                  optional parameters
 * Returns: void
 */
void app_print_info(char *format, ...);

/*
 * This function is used to print an application debug message
 * Parameters: format: Format string
 *                  optional parameters
 * Returns: Svoid
 */
void app_print_debug(char *format, ...);

/*
 * This function is used to print an application error message
 * Parameters: format: Format string
 *                  optional parameters
 * Returns: void
 */
void app_print_error(char *format, ...);

/*
 * Retrieve the size of a file identified by descriptor
 * Parameters: fd: File descriptor
 * Returns: File size if successful or negative error number
 */
int app_file_size(int fd);

#endif /* APP_NSA_UTILS_H */
