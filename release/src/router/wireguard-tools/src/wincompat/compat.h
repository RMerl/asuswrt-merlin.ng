// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2015-2020 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 */

#define __USE_MINGW_ANSI_STDIO 1
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include <winsock2.h>
#include <ws2ipdef.h>
#include <ws2tcpip.h>
#include <in6addr.h>
#include <windows.h>

#undef interface
#undef min
#undef max

#define IFNAMSIZ 64
#define EAI_SYSTEM -99

/* libc.c */
char *strsep(char **str, const char *sep);
ssize_t getdelim(char **buf, size_t *bufsiz, int delimiter, FILE *fp);
ssize_t getline(char **buf, size_t *bufsiz, FILE *fp);
