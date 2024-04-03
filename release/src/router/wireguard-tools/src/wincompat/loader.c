// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2015-2021 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 */

#include <windows.h>
#include <delayimp.h>

static FARPROC WINAPI delayed_load_library_hook(unsigned dliNotify, PDelayLoadInfo pdli)
{
	HMODULE library;
	if (dliNotify != dliNotePreLoadLibrary)
		return NULL;
	library = LoadLibraryExA(pdli->szDll, NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
	if (!library)
		abort();
	return (FARPROC)library;
}

PfnDliHook __pfnDliNotifyHook2 = delayed_load_library_hook;
