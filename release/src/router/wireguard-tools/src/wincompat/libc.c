// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2015-2020 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

char *strsep(char **str, const char *sep)
{
	char *s = *str, *end;
	if (!s)
		return NULL;
	end = s + strcspn(s, sep);
	if (*end)
		*end++ = 0;
	else
		end = 0;
	*str = end;
	return s;
}

ssize_t getdelim(char **buf, size_t *bufsiz, int delimiter, FILE *fp)
{
	char *ptr, *eptr;

	if (!*buf || !*bufsiz) {
		*bufsiz = BUFSIZ;
		if (!(*buf = malloc(*bufsiz)))
			return -1;
	}

	for (ptr = *buf, eptr = *buf + *bufsiz;;) {
		int c = fgetc(fp);
		if (c == -1) {
			if (feof(fp)) {
				ssize_t diff = (ssize_t)(ptr - *buf);
				if (diff != 0) {
					*ptr = '\0';
					return diff;
				}
			}
			return -1;
		}
		*ptr++ = c;
		if (c == delimiter) {
			*ptr = '\0';
			return ptr - *buf;
		}
		if (ptr + 2 >= eptr) {
			char *nbuf;
			size_t nbufsiz = *bufsiz * 2;
			ssize_t d = ptr - *buf;
			if ((nbuf = realloc(*buf, nbufsiz)) == NULL)
				return -1;
			*buf = nbuf;
			*bufsiz = nbufsiz;
			eptr = nbuf + nbufsiz;
			ptr = nbuf + d;
		}
	}
}

ssize_t getline(char **buf, size_t *bufsiz, FILE *fp)
{
	return getdelim(buf, bufsiz, '\n', fp);
}

int inet_pton(int af, const char *src, void *dst)
{
	struct sockaddr_storage ss = { 0 };
	int size = sizeof(ss);
	char s[INET6_ADDRSTRLEN + 1];

	strncpy(s, src, INET6_ADDRSTRLEN + 1);
	s[INET6_ADDRSTRLEN] = '\0';

	if (WSAStringToAddress(s, af, NULL, (struct sockaddr *)&ss, &size))
		return 0;
	if (af == AF_INET)
		*(struct in_addr *)dst = ((struct sockaddr_in *)&ss)->sin_addr;
	else if (af == AF_INET6)
		*(struct in6_addr *)dst = ((struct sockaddr_in6 *)&ss)->sin6_addr;
	else
		return 0;
	return 1;
}

const char *inet_ntop(int af, const void *src, char *dst, socklen_t size)
{
	struct sockaddr_storage ss = { .ss_family = af };
	unsigned long s = size;

	if (af == AF_INET)
		((struct sockaddr_in *)&ss)->sin_addr = *(struct in_addr *)src;
	else if (af == AF_INET6)
		((struct sockaddr_in6 *)&ss)->sin6_addr = *(struct in6_addr *)src;
	else
		return NULL;
	return WSAAddressToString((struct sockaddr *)&ss, sizeof(ss), NULL, dst, &s) ? NULL : dst;
}
