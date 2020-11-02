/*
 * NSA generic application utils functions
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>

#include "nsa_api.h"
#include "app_nsa_utils.h"

/*
 * Definitions
 */

/* Terminal Attribute definitions */
#define RESET       0
#define BRIGHT      1
#define DIM         2
#define UNDERLINE   3
#define BLINK       4
#define REVERSE     7
#define HIDDEN      8

/* Terminal Color definitions */
#define BLACK       0
#define RED         1
#define GREEN       2
#define YELLOW      3
#define BLUE        4
#define MAGENTA     5
#define CYAN        6
#define WHITE       7

/* Length of the string containing a TimeStamp */
#define APP_TIMESTAMP_LEN 80

/*
 * Local functions
 */
#ifdef APP_TRACE_COLOR
static void app_set_trace_color(int attr, int fg, int bg);
#endif // endif
#ifdef APP_TRACE_TIMESTAMP
static char *app_get_time_stamp(char *p_buffer, int buffer_size);
#endif // endif

#if (NSA != TRUE)
/*
 * This function is used to get readable string from Class of device
 * Parameters      class_of_device: The Class of device to decode
 *
 * Returns         Pointer on string containing device type
 *
 */
char *app_get_cod_string(const DEV_CLASS class_of_device)
{
	UINT8 major;
	char *cod_string = NULL;

	/* Extract Major Device Class value */
	BTM_COD_MAJOR_CLASS(major, class_of_device);

	switch (major) {
	case BTM_COD_MAJOR_MISCELLANEOUS:
		cod_string = "Misc device";
		break;
	case BTM_COD_MAJOR_COMPUTER:
		cod_string = "Computer";
		break;
	case BTM_COD_MAJOR_PHONE:
		cod_string = "Phone";
		break;
	case BTM_COD_MAJOR_LAN_ACCESS_PT:
		cod_string = "Access Point";
		break;
	case BTM_COD_MAJOR_AUDIO:
		cod_string = "Audio/Video";
		break;
	case BTM_COD_MAJOR_PERIPHERAL:
		cod_string = "Peripheral";
		break;
	case BTM_COD_MAJOR_IMAGING:
		cod_string = "Imaging";
		break;
	case BTM_COD_MAJOR_WEARABLE:
		cod_string = "Wearable";
		break;
	case BTM_COD_MAJOR_TOY:
		cod_string = "Toy";
		break;
	case BTM_COD_MAJOR_HEALTH:
		cod_string = "Health";
		break;
	default:
		cod_string = "Unknown device type";
		break;
	}

	return cod_string;
}
#endif /* NSA != TRUE */

/*
 * Wait for a choice from user
 * Parameters: The string to print before waiting for input
 * Returns: The number typed by the user, or -1 if the value type was
 *              not parsable
 */
int app_get_choice(const char *querystring)
{
	int neg, value, c, base;

	base = 10;
	neg = 1;
	printf("%s => ", querystring);
	value = 0;
	do {
		c = getchar();
		if ((c >= '0') && (c <= '9'))
			value = (value * base) + (c - '0');
		else if ((c >= 'a') && (c <= 'f'))
			value = (value * base) + (c - 'a' + 10);
		else if ((c >= 'A') && (c <= 'F'))
			value = (value * base) + (c - 'A' + 10);
		else if (c == '-')
			neg *= -1;
		else if (c == 'x')
			base = 16;
	} while ((c != EOF) && (c != '\n'));

	return value * neg;
}

/*
 * Ask the user to enter a string value
 * Parameters: querystring: to print before waiting for input
 *                  str: the char buffer to fill with user input
 *                  len: the length of the char buffer
 * Returns: The length of the string entered not including last NULL char
 *             negative value in case of error
 */
int app_get_string(const char *querystring, char *str, int len)
{
	int c, index;

	if (querystring)
		printf("%s => ", querystring);

	index = 0;
	do {
		c = getchar();
		if (c == EOF)
			return -1;
		if ((c != '\n') && (index < (len - 1))) {
			str[index] = (char)c;
			index++;
		}
	} while (c != '\n');

	str[index] = '\0';
	return index;
}

#ifdef APP_TRACE_TIMESTAMP
/*
 * This function is used to get a timestamp
 * Parameters: p_buffer: buffer to write the timestamp
 *                  buffer_size: buffer size
 * Returns: pointer on p_buffer
 */
static char *app_get_time_stamp(char *p_buffer, int buffer_size)
{
	char time_string[80];

	/* use GKI_get_time_stamp to have the same clock than the BSA client traces */
	GKI_get_time_stamp((INT8 *)time_string);

	snprintf(p_buffer, buffer_size, "%s", time_string);

	return p_buffer;
}
#endif /* APP_TRACE_TIMESTAMP */

/*
 * This function is used to print an application information message
 * Parameters: format: Format string
 *                  optional parameters
 * Returns: void
 */
void app_print_info(char *format, ...)
{
	va_list ap;
#ifdef APP_TRACE_TIMESTAMP
	char time_stamp[APP_TIMESTAMP_LEN];
#endif // endif

#ifdef APP_TRACE_COLOR
	app_set_trace_color(RESET, BLACK, WHITE);
#endif // endif

#ifdef APP_TRACE_TIMESTAMP
	app_get_time_stamp(time_stamp, sizeof(time_stamp));
	printf("INFO@%s: ", time_stamp);
#endif // endif

	va_start(ap, format);
	vprintf(format, ap);
	va_end(ap);

#ifdef APP_TRACE_COLOR
	app_set_trace_color(RESET, BLACK, WHITE);
#endif // endif
}

/*
 * This function is used to print an application debug message
 * Parameters: format: Format string
 *                  optional parameters
 * Returns: void
 */
void app_print_debug(char *format, ...)
{
	va_list ap;
#ifdef APP_TRACE_TIMESTAMP
	char time_stamp[APP_TIMESTAMP_LEN];
#endif // endif

#ifdef APP_TRACE_COLOR
	app_set_trace_color(RESET, GREEN, WHITE);
#endif // endif

#ifdef APP_TRACE_TIMESTAMP
	app_get_time_stamp(time_stamp, sizeof(time_stamp));
	printf("DEBUG@%s: ", time_stamp);
#else
	printf("DEBUG: ");
#endif // endif

	va_start(ap, format);
	vprintf(format, ap);
	va_end(ap);

#ifdef APP_TRACE_COLOR
	app_set_trace_color(RESET, BLACK, WHITE);
#endif // endif
}

/*
 * This function is used to print an application error message
 * Parameters: format: Format string
 *                  optional parameters
 * Returns: void
 */
void app_print_error(char *format, ...)
{
	va_list ap;
#ifdef APP_TRACE_TIMESTAMP
	char time_stamp[APP_TIMESTAMP_LEN];
#endif // endif

#ifdef APP_TRACE_COLOR
	app_set_trace_color(RESET, RED, WHITE);
#endif // endif

#ifdef APP_TRACE_TIMESTAMP
	app_get_time_stamp(time_stamp, sizeof(time_stamp));
	printf("ERROR@%s: ", time_stamp);
#else
	printf("ERROR: ");
#endif // endif

	va_start(ap, format);
	vprintf(format, ap);
	va_end(ap);

#ifdef APP_TRACE_COLOR
	app_set_trace_color(RESET, BLACK, WHITE);
#endif // endif
}

#ifdef APP_TRACE_COLOR
/*
 * This function changes the text color
 * Parameters: attribute: Text attribute (reset/Blink/etc)
 *                  foreground: foreground color
 *                  background: background color
 * Returns: void
 */
static void app_set_trace_color(int attribute, int foreground, int background)
{
	char command[13];

	/* Command is the control command to the terminal */
	snprintf(command, sizeof(command), "%c[%d;%d;%dm", 0x1B, attribute,
		foreground + 30, background + 40);
	printf("%s", command);
}
#endif /* APP_TRACE_COLOR */

/*
 * Retrieve the size of a file identified by descriptor
 * Parameters: fd: File descriptor
 * Returns: File size if successful or negative error number
 */
int app_file_size(int fd)
{
	struct stat file_stat;
	int rc = -1;

	rc = fstat(fd, &file_stat);

	if (rc >= 0) {
		/* Retrieve the size of the file */
		rc = file_stat.st_size;
	}
	else {
		APP_ERROR1("could not fstat(fd=%d)", fd);
	}

	return rc;
}
