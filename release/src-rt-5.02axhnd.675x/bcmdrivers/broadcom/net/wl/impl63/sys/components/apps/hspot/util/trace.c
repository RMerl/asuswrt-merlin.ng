/*
 * Tracing utility.
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
 * $Id:$
 */

#include <stdio.h>
#include <stdarg.h>
#if !defined(BCMDRIVER)
#include <time.h>
#include <sys/time.h>
#endif /* !BCMDRIVER */
#include <ctype.h>
#include <trace.h>

traceLevelE gTraceLevel = TRACE_NONE;

static char *getTimestamp(traceLevelE level)
{
	static char buffer[32] = {0};
#if !defined(BCMDRIVER)
	struct timeval tv;
	struct tm tm;
#endif /* !BCMDRIVER */

	if (level == TRACE_PRINTF) {
		/* no timestamp for printf */
		buffer[0] = 0;
	}
	else {
#if !defined(BCMDRIVER)
		gettimeofday(&tv, NULL);
		localtime_r(&tv.tv_sec, &tm);

		snprintf(buffer, sizeof(buffer), "%d:%02d:%02d.%03d - ",
			tm.tm_hour, tm.tm_min, tm.tm_sec, (int)tv.tv_usec / 1000);
#else
		uint32 uptime = OSL_SYSUPTIME();
		uint32 ms, sec, min, hour;

		ms = uptime % 1000;
		uptime /= 1000;
		sec = uptime % 60;
		uptime /= 60;
		min = uptime % 60;
		hour = uptime / 60;
		sprintf(buffer, "%d:%02d:%02d.%03d - ",
			hour, min, sec, ms);
#endif /* !BCMDRIVER */
	}

	return buffer;
}

void trace(const char *file, int line, const char *function,
	traceLevelE level, const char *format, ...)
{
	va_list argp;

	if (level & gTraceLevel || level == TRACE_PRINTF) {
		char *time = getTimestamp(level);

		if (level & TRACE_ERROR)
			printf("%sERROR!!! %s:%d %s()\n", time, file, line, function);

		printf("%s", time);
		va_start(argp, format);
#if !defined(BCMDRIVER)
		vprintf(format, argp);
#else
		{
			char buffer[128] = {0};
			vsprintf(buffer, format, argp);
			printf(buffer);
		}
#endif /* !BCMDRIVER */
		va_end(argp);
	}
}

void traceMacAddr(const char *file, int line, const char *function,
	traceLevelE level, const char *str, struct ether_addr *mac)
{
	if (level & gTraceLevel || level == TRACE_PRINTF) {
		char *time = getTimestamp(level);
		if (level & TRACE_ERROR)
			printf("%sERROR!!! %s:%d %s()\n", time, file, line, function);

		printf("%s%s[6] = %02X:%02X:%02X:%02X:%02X:%02X\n", time, str,
			mac->octet[0], mac->octet[1], mac->octet[2],
			mac->octet[3], mac->octet[4], mac->octet[5]);
	}
}

void traceHexDump(const char *file, int line, const char *function,
	traceLevelE level, const char *str, int len, uint8 *buf)
{
	if (level & gTraceLevel || level == TRACE_PRINTF) {
		char *time = getTimestamp(level);
		int i, j;
		int sol, eol;

		if (level & TRACE_ERROR)
			printf("%sERROR!!! %s:%d %s()\n", time, file, line, function);

		printf("%s%s[%d] = ", time, str, len);

		sol = eol = 0;
		for (i = 0; i < (int)len; i++) {
			if ((i % 16) == 0) {
				eol = i;

				/* print ascii */
				printf("   ");
				for (j = sol; j < eol; j++) {
					if (isprint(buf[j]))
						printf("%c", (char)buf[j]);
					else
						printf(".");
				}
				printf("\n   ");
				sol = eol;
			}

			printf("%02X ", buf[i]);
		}

		if (len > 0) {
			/* pad to EOL */
			for (j = 0; j < 16 - (i - sol); j++)
				printf("   ");

			/* print ascii */
			printf("   ");
			for (j = sol; j < i; j++) {
				if (isprint(buf[j]))
					printf("%c", (char)buf[j]);
				else
					printf(".");
			}
		}

		printf("\n");
	}
}
