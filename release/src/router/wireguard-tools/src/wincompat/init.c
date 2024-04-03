// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2015-2020 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 */

#include <winsock2.h>
#include <windows.h>

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x4
#endif

extern void NTAPI RtlGetNtVersionNumbers(DWORD *major, DWORD *minor, DWORD *build);
bool is_win7 = false;

__attribute__((constructor)) static void init(void)
{
	char *colormode;
	DWORD console_mode, major, minor;
	HANDLE stdout_handle;
	WSADATA wsaData;

	if (!SetDllDirectoryA("") || !SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_SYSTEM32))
		abort();

	RtlGetNtVersionNumbers(&major, &minor, NULL);
	is_win7 = (major == 6 && minor <= 1) || major < 6;

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE); // We don't close this.
	if (stdout_handle == INVALID_HANDLE_VALUE)
		goto no_color;
	if (!GetConsoleMode(stdout_handle, &console_mode))
		goto no_color;
	if (!SetConsoleMode(stdout_handle, ENABLE_VIRTUAL_TERMINAL_PROCESSING | console_mode))
		goto no_color;
	return;

no_color:
	colormode = getenv("WG_COLOR_MODE");
	if (!colormode)
		putenv("WG_COLOR_MODE=never");
}

__attribute__((destructor)) static void deinit(void)
{
	WSACleanup();
}
