/*
 * Copyright (C) 2013 Martin Willi
 * Copyright (C) 2013 revosec AG
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

/* WSAPoll() */
#define _WIN32_WINNT 0x0600

#include <utils/utils.h>

#include <errno.h>

/**
 * See header
 */
void windows_init()
{
	WSADATA wsad;

	/* initialize winsock2 */
	WSAStartup(MAKEWORD(2, 2), &wsad);
}

/**
 * See header
 */
void windows_deinit()
{
	WSACleanup();
}

/**
 * See header
 */
int usleep(useconds_t usec)
{
	if (usec > 0 && usec < 1000)
	{	/* do not Sleep(0) for small values */
		usec = 1000;
	}
	SleepEx(usec / 1000, TRUE);
	return 0;
}

/**
 * See header.
 */
char* strndup(const char *s, size_t n)
{
	char *dst;

	n = min(strnlen(s, n), n);
	dst = malloc(n + 1);
	memcpy(dst, s, n);
	dst[n] = '\0';

	return dst;
}

/*
 * See header.
 */
void *dlopen(const char *filename, int flag)
{
	return LoadLibrary(filename);
}

/**
 * Load a symbol from known default libs (monolithic build)
 */
static void* dlsym_default(const char *name)
{
	const char *dlls[] = {
		"libstrongswan-0.dll",
		"libcharon-0.dll",
		"libtnccs-0.dll",
		NULL /* .exe */
	};
	HANDLE handle;
	void *sym = NULL;
	int i;

	for (i = 0; i < countof(dlls); i++)
	{
		handle = GetModuleHandle(dlls[i]);
		if (handle)
		{
			sym = GetProcAddress(handle, name);
			if (sym)
			{
				break;
			}
		}
	}
	return sym;
}

/**
 * Emulate RTLD_NEXT for some known symbols
 */
static void* dlsym_next(const char *name)
{
	struct {
		const char *dll;
		const char *syms[4];
	} dlls[] = {
		/* for leak detective */
		{ "msvcrt",
			{ "malloc", "calloc", "realloc", "free" }
		},
	};
	HANDLE handle = NULL;
	int i, j;

	for (i = 0; i < countof(dlls); i++)
	{
		for (j = 0; j < countof(dlls[0].syms); j++)
		{
			if (dlls[i].syms[j] && streq(dlls[i].syms[j], name))
			{
				handle = GetModuleHandle(dlls[i].dll);
				break;
			}
		}
	}
	if (handle)
	{
		return GetProcAddress(handle, name);
	}
	return handle;
}

/**
 * See header.
 */
void* dlsym(void *handle, const char *symbol)
{
	if (handle == RTLD_DEFAULT)
	{
		return dlsym_default(symbol);
	}
	if (handle == RTLD_NEXT)
	{
		return dlsym_next(symbol);
	}
	return GetProcAddress((HMODULE)handle, symbol);
}

/**
 * See header.
 */
char* dlerror(void)
{
	static char buf[128];
	char *pos;
	DWORD err;

	err = GetLastError();
	if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
					  NULL, err, 0, buf, sizeof(buf), NULL) > 0)
	{
		pos = strchr(buf, '\n');
		if (pos)
		{
			*pos = '\0';
		}
	}
	else
	{
		snprintf(buf, sizeof(buf), "(%u)", err);
	}
	return buf;
}

/**
 * See header.
 */
int dlclose(void *handle)
{
	return FreeLibrary((HMODULE)handle);
}

/**
 * See header
 */
int socketpair(int domain, int type, int protocol, int sv[2])
{
	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_addr.s_addr = htonl(INADDR_LOOPBACK),
	};
	socklen_t len = sizeof(addr);
	int s, c, sc;
	BOOL on;

	/* We don't check domain for AF_INET, as we use it as replacement for
	 * AF_UNIX. */
	if (type != SOCK_STREAM)
	{
		errno = EINVAL;
		return -1;
	}
	if (protocol != 0 && protocol != IPPROTO_TCP)
	{
		errno = EINVAL;
		return -1;
	}
	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == -1)
	{
		return -1;
	}
	c = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (c == -1)
	{
		closesocket(s);
		return -1;
	}
	if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) == 0 &&
		getsockname(s,(struct sockaddr*)&addr, &len) == 0 &&
		listen(s, 0) == 0 &&
		connect(c, (struct sockaddr*)&addr, sizeof(addr)) == 0)
	{
		sc = accept(s, NULL, NULL);
		if (sc >= 0)
		{
			closesocket(s);
			s = sc;
			if (setsockopt(s, IPPROTO_TCP, TCP_NODELAY,
						   (void*)&on, sizeof(on)) == 0 &&
				setsockopt(c, IPPROTO_TCP, TCP_NODELAY,
						   (void*)&on, sizeof(on)) == 0)
			{
				sv[0] = s;
				sv[1] = c;
				return 0;
			}
		}
	}
	closesocket(s);
	closesocket(c);
	return -1;
}

/**
 * See header
 */
char* getpass(const char *prompt)
{
	static char buf[64] = "";
	char *pos;
	HANDLE in, out;
	DWORD mode, written = 0, total, done;

	out = GetStdHandle(STD_OUTPUT_HANDLE);
	in = GetStdHandle(STD_INPUT_HANDLE);

	if (out == INVALID_HANDLE_VALUE || in == INVALID_HANDLE_VALUE ||
		!GetConsoleMode(out, &mode) || !GetConsoleMode(in, &mode))
	{
		return NULL;
	}

	total = strlen(prompt);
	while (written < total)
	{
		if (!WriteConsole(out, prompt + written, total - written, &done, NULL))
		{
			return NULL;
		}
		written += done;
	}

	if (!SetConsoleMode(in, mode & ~ENABLE_ECHO_INPUT))
	{
		return NULL;
	}

	while (TRUE)
	{
		if (!ReadConsole(in, buf, sizeof(buf), &done, NULL))
		{
			SetConsoleMode(in, mode);
			return NULL;
		}
		buf[sizeof(buf)-1] = '\0';

		if (done)
		{
			pos = strchr(buf, '\r');
			if (pos)
			{
				*pos = '\0';
			}
			break;
		}
	}
	SetConsoleMode(in, mode);

	/* append a newline, as we have no echo during input */
	WriteConsole(out, "\r\n", 2, &done, NULL);

	return buf;
}

/**
 * See header.
 */
#undef strerror_s
int strerror_s_extended(char *buf, size_t buflen, int errnum)
{
	const char *errstr [] = {
		/* EADDRINUSE */		"Address in use",
		/* EADDRNOTAVAIL */		"Address not available",
		/* EAFNOSUPPORT */		"Address family not supported",
		/* EALREADY */			"Connection already in progress",
		/* EBADMSG */			"Bad message",
		/* ECANCELED */			"Operation canceled",
		/* ECONNABORTED */		"Connection aborted",
		/* ECONNREFUSED */		"Connection refused",
		/* ECONNRESET */		"Connection reset",
		/* EDESTADDRREQ */		"Destination address required",
		/* EHOSTUNREACH */		"Host is unreachable",
		/* EIDRM */				"Identifier removed",
		/* EINPROGRESS */		"Operation in progress",
		/* EISCONN */			"Socket is connected",
		/* ELOOP */				"Too many levels of symbolic links",
		/* EMSGSIZE */			"Message too large",
		/* ENETDOWN */			"Network is down",
		/* ENETRESET */			"Connection aborted by network",
		/* ENETUNREACH */		"Network unreachable",
		/* ENOBUFS */			"No buffer space available",
		/* ENODATA */			"No message is available",
		/* ENOLINK */			"No link",
		/* ENOMSG */			"No message of the desired type",
		/* ENOPROTOOPT */		"Protocol not available",
		/* ENOSR */				"No stream resources",
		/* ENOSTR */			"Not a stream",
		/* ENOTCONN */			"The socket is not connected",
		/* ENOTRECOVERABLE */	"State not recoverable",
		/* ENOTSOCK */			"Not a socket",
		/* ENOTSUP */			"Not supported",
		/* EOPNOTSUPP */		"Operation not supported on socket",
		/* EOTHER */			"Other error",
		/* EOVERFLOW */			"Value too large to be stored in data type",
		/* EOWNERDEAD */		"Previous owner died",
		/* EPROTO */			"Protocol error",
		/* EPROTONOSUPPORT */	"Protocol not supported",
		/* EPROTOTYPE */		"Protocol wrong type for socket",
		/* ETIME */				"Timeout",
		/* ETIMEDOUT */			"Connection timed out",
		/* ETXTBSY */			"Text file busy",
		/* EWOULDBLOCK */		"Operation would block",
	};
	int offset = EADDRINUSE;

	if (errnum < offset || errnum >= offset + countof(errstr))
	{
		return strerror_s(buf, buflen, errnum);
	}
	strncpy(buf, errstr[errnum - offset], buflen);
	buf[buflen - 1] = '\0';
	return 0;
}

/**
 * Set errno for a function setting WSA error on failure
 */
static int wserr(int retval)
{
	if (retval < 0)
	{
		static const struct {
			DWORD wsa;
			int err;
		} map[] = {
			{ WSANOTINITIALISED,			EBADF						},
			{ WSAENETDOWN,					ENETDOWN					},
			{ WSAENETRESET,					ENETRESET					},
			{ WSAECONNABORTED,				ECONNABORTED				},
			{ WSAESHUTDOWN,					ECONNABORTED				},
			{ WSAEACCES,					EACCES						},
			{ WSAEINTR,						EINTR						},
			{ WSAEINPROGRESS,				EINPROGRESS					},
			{ WSAEFAULT,					EFAULT						},
			{ WSAENOBUFS,					ENOBUFS						},
			{ WSAENOTSOCK,					ENOTSOCK					},
			{ WSAEOPNOTSUPP,				EOPNOTSUPP					},
			{ WSAEWOULDBLOCK,				EWOULDBLOCK					},
			{ WSAEMSGSIZE,					EMSGSIZE					},
			{ WSAEINVAL,					EINVAL						},
			{ WSAENOTCONN,					ENOTCONN					},
			{ WSAEHOSTUNREACH,				EHOSTUNREACH				},
			{ WSAENETUNREACH,				ENETUNREACH					},
			{ WSAECONNABORTED,				ECONNABORTED				},
			{ WSAECONNRESET,				ECONNRESET					},
			{ WSAETIMEDOUT,					ETIMEDOUT					},
			{ WSAEMFILE,					EMFILE						},
			{ WSAEALREADY,					EALREADY					},
			{ WSAEDESTADDRREQ,				EDESTADDRREQ				},
			{ WSAEISCONN,					EISCONN						},
			{ WSAEOPNOTSUPP,				EOPNOTSUPP					},
			{ WSAEPROTOTYPE,				EPROTOTYPE					},
			{ WSAENOPROTOOPT,				ENOPROTOOPT					},
			{ WSAEPROTONOSUPPORT,			EPROTONOSUPPORT				},
			{ WSAEPFNOSUPPORT,				EPROTONOSUPPORT				},
			{ WSAEAFNOSUPPORT,				EAFNOSUPPORT				},
			{ WSAEADDRNOTAVAIL,				EADDRNOTAVAIL				},
			{ WSAEADDRINUSE,				EADDRINUSE					},
			{ WSAETIMEDOUT,					ETIMEDOUT					},
			{ WSAECONNREFUSED,				ECONNREFUSED				},
			{ WSAELOOP,						ELOOP						},
			{ WSAENAMETOOLONG,				ENAMETOOLONG				},
			{ WSAENOTEMPTY,					ENOTEMPTY					},
			{ WSAEPROTOTYPE,				EPROTOTYPE					},
			{ WSAVERNOTSUPPORTED,			ENOTSUP						},
		};
		DWORD wsa, i;

		wsa = WSAGetLastError();
		for (i = 0; i < countof(map); i++)
		{
			if (map[i].wsa == wsa)
			{
				errno = map[i].err;
				return retval;
			}
		}
		errno = ENOENT;
		return retval;
	}
	errno = 0;
	return retval;
}

/**
 * Check and clear the dontwait flag
 */
static bool check_dontwait(int *flags)
{
	if (*flags & MSG_DONTWAIT)
	{
		*flags &= ~MSG_DONTWAIT;
		return TRUE;
	}
	return FALSE;
}

/**
 * See header
 */
#undef shutdown
int windows_shutdown(int sockfd, int how)
{
	return wserr(shutdown(sockfd, how));
}

/**
 * See header
 */
#undef accept
int windows_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
	return wserr(accept(sockfd, addr, addrlen));
}

/**
 * See header
 */
#undef bind
int windows_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	return wserr(bind(sockfd, addr, addrlen));
}

/**
 * See header
 */
#undef connect
int windows_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	return wserr(connect(sockfd, addr, addrlen));
}

/**
 * See header
 */
#undef getsockname
int windows_getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
	return wserr(getsockname(sockfd, addr, addrlen));
}

/**
 * See header
 */
#undef getsockopt
int windows_getsockopt(int sockfd, int level, int optname,
					   void *optval, socklen_t *optlen)
{
	return wserr(getsockopt(sockfd, level, optname, optval, optlen));
}

/**
 * See header
 */
#undef setsockopt
int windows_setsockopt(int sockfd, int level, int optname,
					   const void *optval, socklen_t optlen)
{
	return wserr(setsockopt(sockfd, level, optname, optval, optlen));
}

/**
 * See header
 */
#undef socket
int windows_socket(int domain, int type, int protocol)
{
	return wserr(socket(domain, type, protocol));
}

/**
 * See header
 */
#undef select
int windows_select(int nfds, fd_set *readfds, fd_set *writefds,
				   fd_set *exceptfds, struct timeval *timeout)
{
	return wserr(select(nfds, readfds, writefds, exceptfds, timeout));
}

/**
 * See header
 */
#undef close
int windows_close(int fd)
{
	int ret;

	ret = close(fd);
	if (ret == -1 && errno == EBADF)
	{	/* Winsock socket? */
		ret = wserr(closesocket(fd));
	}
	return ret;
}

/**
 * See header
 */
#undef recv
ssize_t windows_recv(int sockfd, void *buf, size_t len, int flags)
{
	u_long on = 1, off = 0;
	ssize_t outlen = -1;

	if (!check_dontwait(&flags))
	{
		return wserr(recv(sockfd, buf, len, flags));
	}
	if (wserr(ioctlsocket(sockfd, FIONBIO, &on) == 0))
	{
		outlen = wserr(recv(sockfd, buf, len, flags));
		ioctlsocket(sockfd, FIONBIO, &off);
	}
	return outlen;
}

/**
 * See header
 */
#undef recvfrom
ssize_t windows_recvfrom(int sockfd, void *buf, size_t len, int flags,
						 struct sockaddr *src_addr, socklen_t *addrlen)
{
	u_long on = 1, off = 0;
	ssize_t outlen = -1;

	if (!check_dontwait(&flags))
	{
		return wserr(recvfrom(sockfd, buf, len, flags, src_addr, addrlen));
	}
	if (wserr(ioctlsocket(sockfd, FIONBIO, &on)) == 0)
	{
		outlen = wserr(recvfrom(sockfd, buf, len, flags, src_addr, addrlen));
		ioctlsocket(sockfd, FIONBIO, &off);
	}
	return outlen;
}

/**
 * See header
 */
#undef send
ssize_t windows_send(int sockfd, const void *buf, size_t len, int flags)
{
	u_long on = 1, off = 0;
	ssize_t outlen = -1;

	if (!check_dontwait(&flags))
	{
		return wserr(send(sockfd, buf, len, flags));
	}
	if (wserr(ioctlsocket(sockfd, FIONBIO, &on)) == 0)
	{
		outlen = wserr(send(sockfd, buf, len, flags));
		ioctlsocket(sockfd, FIONBIO, &off);
	}
	return outlen;
}

/**
 * See header
 */
#undef sendto
ssize_t windows_sendto(int sockfd, const void *buf, size_t len, int flags,
					   const struct sockaddr *dest_addr, socklen_t addrlen)
{
	u_long on = 1, off = 0;
	ssize_t outlen = -1;

	if (!check_dontwait(&flags))
	{
		return wserr(sendto(sockfd, buf, len, flags, dest_addr, addrlen));
	}
	if (wserr(ioctlsocket(sockfd, FIONBIO, &on)) == 0)
	{
		outlen = wserr(sendto(sockfd, buf, len, flags, dest_addr, addrlen));
		ioctlsocket(sockfd, FIONBIO, &off);
	}
	return outlen;
}

/**
 * See header
 */
#undef read
ssize_t windows_read(int fd, void *buf, size_t count)
{
	ssize_t ret;

	ret = wserr(recv(fd, buf, count, 0));
	if (ret == -1 && errno == ENOTSOCK)
	{
		ret = read(fd, buf, count);
	}
	return ret;
}

/**
 * See header
 */
#undef write
ssize_t windows_write(int fd, void *buf, size_t count)
{
	ssize_t ret;

	ret = wserr(send(fd, buf, count, 0));
	if (ret == -1 && errno == ENOTSOCK)
	{
		ret = write(fd, buf, count);
	}
	return ret;
}

/**
 * See header
 */
int poll(struct pollfd *fds, int nfds, int timeout)
{
	return wserr(WSAPoll(fds, nfds, timeout));
}
