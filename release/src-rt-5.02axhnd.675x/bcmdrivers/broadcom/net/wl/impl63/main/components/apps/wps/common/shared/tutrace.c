/*
 * Debug messages
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
 * $Id: tutrace.c 470764 2014-04-16 08:40:23Z $
 */

#if defined(WIN32) || defined(WINCE)
#include <windows.h>
#endif // endif
#if !defined(WIN32)
#include <stdarg.h>
#endif // endif
#include <stdio.h>

#if defined(linux)
#include <sys/time.h>
#include <time.h>
#endif // endif

#include <ctype.h>
#include "tutrace.h"

static WPS_TRACEMSG_OUTPUT_FN traceMsgOutputFn = NULL;

#ifdef _TUDEBUGTRACE
static unsigned int wps_msglevel = TUTRACELEVEL;
#else
static unsigned int wps_msglevel = 0;
#endif // endif

void
wps_set_traceMsg_output_fn(WPS_TRACEMSG_OUTPUT_FN fn)
{
	traceMsgOutputFn = fn;
}

unsigned int
wps_tutrace_get_msglevel()
{
	return wps_msglevel;
}

void
wps_tutrace_set_msglevel(unsigned int level)
{
	wps_msglevel = level;
}

#ifdef _TUDEBUGTRACE
void
print_traceMsg(int level, const char *lpszFile,
                   int nLine, char *lpszFormat, ...)
{
	char szTraceMsg[2000];
	int cbMsg = 0;
	va_list lpArgv;

#ifdef _WIN32_WCE
	TCHAR szMsgW[2000];
#endif // endif
	char *TraceMsgPtr = szTraceMsg;

	if (!(TUTRACELEVEL & level)) {
		return;
	}

#ifdef __linux__
	if (wps_msglevel & TUTIME) {
		char time_stamp[80];
		struct timeval tv;
		struct timespec ts;
		struct tm lt;

		gettimeofday(&tv, NULL);
		ts.tv_sec = tv.tv_sec;
		ts.tv_nsec = tv.tv_usec * 1000;
		localtime_r(&ts.tv_sec, &lt);

		snprintf((char *)time_stamp, sizeof(time_stamp), "%02dh:%02dm:%02ds:%03ldms",
			lt.tm_hour, lt.tm_min, lt.tm_sec, (ts.tv_nsec/1000000));
		cbMsg = sprintf(TraceMsgPtr, "[@%s]: ", time_stamp);
		TraceMsgPtr += cbMsg;
	}
#endif /* __linux__ */

	/* Format trace msg prefix */
	if (traceMsgOutputFn != NULL)
		cbMsg = sprintf(TraceMsgPtr, "WPS: %s(%d):", lpszFile, nLine);
	else
		cbMsg = sprintf(TraceMsgPtr, "%s(%d):", lpszFile, nLine);
	TraceMsgPtr += cbMsg;

	/* Append trace msg to prefix. */
	va_start(lpArgv, lpszFormat);
	cbMsg = vsprintf(TraceMsgPtr, lpszFormat, lpArgv);
	va_end(lpArgv);
	TraceMsgPtr += cbMsg;

	if (traceMsgOutputFn != NULL) {
		traceMsgOutputFn(((TUTRACELEVEL & TUERR) != 0), szTraceMsg);
	} else {
#ifndef _WIN32_WCE
	#ifdef WIN32
		OutputDebugString(szTraceMsg);
	#else /* Linux */
		if (level & wps_msglevel)
			fprintf(stdout, "%s", szTraceMsg);
	#endif /* WIN32 */
#else /* _WIN32_WCE */
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szTraceMsg, -1,
			szMsgW, strlen(szTraceMsg)+1);
		RETAILMSG(1, (szMsgW));
#endif /* _WIN32_WCE */
	}
}

/* Just print the message, no msglevel check */
static void
WPS_Print(char *lpszFormat, ...)
{
	char szTraceMsg[2000];
	int cbMsg = 0;
	va_list lpArgv;

#ifdef _WIN32_WCE
	TCHAR szMsgW[2000];
#endif // endif
	char *TraceMsgPtr = szTraceMsg;

	/* Append trace msg to prefix. */
	va_start(lpArgv, lpszFormat);
	cbMsg = vsprintf(TraceMsgPtr, lpszFormat, lpArgv);
	va_end(lpArgv);
	TraceMsgPtr += cbMsg;

	if (traceMsgOutputFn != NULL) {
		traceMsgOutputFn(((TUTRACELEVEL & TUERR) != 0), szTraceMsg);
	} else {
#ifndef _WIN32_WCE
	#ifdef WIN32
		OutputDebugString(szTraceMsg);
	#else /* Linux */
		fprintf(stdout, "%s", szTraceMsg);
	#endif /* WIN32 */
#else /* _WIN32_WCE */
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szTraceMsg, -1,
			szMsgW, strlen(szTraceMsg)+1);
		RETAILMSG(1, (szMsgW));
#endif /* _WIN32_WCE */
	}
}
#endif /* _TUDEBUGTRACE */

int
WPS_HexDumpAscii(unsigned int level, char *title, unsigned char *buf, unsigned int len)
{
#ifdef _TUDEBUGTRACE
	int i, llen;
	const unsigned char *pos = buf;
	const int line_len = 16;

	if ((wps_msglevel & level) == 0)
		return -1;

	WPS_Print("WPS: %s : hexdump_ascii(len=%lu):\n", title, (unsigned long) len);
	while (len) {
		llen = len > line_len ? line_len : len;
		WPS_Print("    ");
		for (i = 0; i < llen; i++)
			WPS_Print(" %02x", pos[i]);
		for (i = llen; i < line_len; i++)
			WPS_Print("   ");
		WPS_Print("   ");
		for (i = 0; i < llen; i++) {
			if (isprint(pos[i]))
				WPS_Print("%c", pos[i]);
			else
				WPS_Print("*");
		}
		for (i = llen; i < line_len; i++)
			WPS_Print(" ");
		WPS_Print("\n");
		pos += llen;
		len -= llen;
	}
#endif /* _TUDEBUGTRACE */

	return 0;
}
