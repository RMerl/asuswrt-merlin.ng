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

/**
 * @defgroup windows windows
 * @{ @ingroup compat
 */

#ifndef WINDOWS_H_
#define WINDOWS_H_

#include <winsock2.h>
#include <ws2tcpip.h>
#include <direct.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/stat.h>

/* undef Windows variants evaluating values more than once */
#undef min
#undef max

/* interface is defined as an alias to "struct" in basetypes.h, but
 * we use it here and there as ordinary identifier. */
#undef interface

/* used by Windows API, but we have our own */
#undef CALLBACK

/* UID/GID types for capabilities, even if not supported */
typedef u_int uid_t;
typedef u_int gid_t;

/**
 * Initialize Windows libraries
 */
void windows_init();

/**
 * Deinitialize windows libraries
 */
void windows_deinit();

/**
 * Replacement for random(3)
 */
static inline long random(void)
{
	return rand();
}

/**
 * Replacement for srandom(3)
 */
static inline void srandom(unsigned int seed)
{
	srand(seed);
}

/**
 * Replacement of sched_yield(2) from <sched.h>
 */
static inline int sched_yield(void)
{
	Sleep(0);
	return 0;
}

/**
 * Replacement of sleep(3), cancellable by thread_cancel()
 */
#define sleep sleep_cancellable
static inline int sleep_cancellable(unsigned int seconds)
{
	SleepEx(seconds * 1000, TRUE);
	return 0;
}

/**
 * Replacement of usleep(3), cancellable, ms resolution only
 */
int usleep(useconds_t usec);

/**
 * strdup(3), the Windows variant can't free(strdup("")) and others
 */
#define strdup strdup_windows
static inline char* strdup_windows(const char *src)
{
	size_t len;
	char *dst;

	len = strlen(src) + 1;
	dst = malloc(len);
	memcpy(dst, src, len);
	return dst;
}

/**
 * strndup(3)
 */
char* strndup(const char *s, size_t n);

/**
 * From winsock2.h
 */
#ifndef IPPROTO_IPIP
#define IPPROTO_IPIP IPPROTO_IPV4
#endif

/**
 * Provided via ws2_32
 */
#ifndef InetNtop
const char WINAPI *inet_ntop(int af, const void *src, char *dst, socklen_t size);
#endif

/**
 * Provided via ws2_32
 */
#ifndef InetPton
int WINAPI inet_pton(int af, const char *src, void *dst);
#endif

/**
 * Provided by printf hook backend
 */
int asprintf(char **strp, const char *fmt, ...);

/**
 * Provided by printf hook backend
 */
int vasprintf(char **strp, const char *fmt, va_list ap);

/**
 * timeradd(3) from <sys/time.h>
 */
static inline void timeradd(struct timeval *a, struct timeval *b,
							struct timeval *res)
{
	res->tv_sec = a->tv_sec + b->tv_sec;
	res->tv_usec = a->tv_usec + b->tv_usec;
	if (res->tv_usec >= 1000000)
	{
		res->tv_usec -= 1000000;
		res->tv_sec++;
	}
}

/**
 * timersub(3) from <sys/time.h>
 */
static inline void timersub(struct timeval *a, struct timeval *b,
							struct timeval *res)
{
	res->tv_sec = a->tv_sec - b->tv_sec;
	res->tv_usec = a->tv_usec - b->tv_usec;
	if (res->tv_usec < 0)
	{
		res->tv_usec += 1000000;
		res->tv_sec--;
	}
}

/**
 * gmtime_r(3) from <time.h>
 */
static inline struct tm *gmtime_r(const time_t *timep, struct tm *result)
{
	struct tm *ret;

	/* gmtime_s() and friends seem not to be implemented/functioning.
	 * Relying on gmtime() on Windows works as well, as it uses thread
	 * specific buffers. */
	ret = gmtime(timep);
	if (ret)
	{
		memcpy(result, ret, sizeof(*result));
	}
	return ret;
}

/**
 * localtime_r(3) from <time.h>
 */
static inline struct tm *localtime_r(const time_t *timep, struct tm *result)
{
	struct tm *ret;

	/* localtime_s() and friends seem not to be implemented/functioning.
	 * Relying on localtime() on Windows works as well, as it uses thread
	 * specific buffers. */
	ret = localtime(timep);
	if (ret)
	{
		memcpy(result, ret, sizeof(*result));
	}
	return ret;
}

/**
 * setenv(3) from <stdlib.h>, overwrite flag is ignored
 */
static inline int setenv(const char *name, const char *value, int overwrite)
{
	if (SetEnvironmentVariableA(name, value) == 0)
	{	/* failed */
		return -1;
	}
	return 0;
}

/**
 * Lazy binding, ignored on Windows
 */
#define RTLD_LAZY 1

/**
 * Immediate binding, ignored on Windows
 */
#define RTLD_NOW 2

/**
 * Default handle targeting .exe
 */
#define RTLD_DEFAULT (NULL)

/**
 * Find symbol in next library
 */
#define RTLD_NEXT ((void*)~(uintptr_t)0)

/**
 * dlopen(3) from <dlfcn.h>
 */
void* dlopen(const char *filename, int flag);

/**
 * dlsym() from <dlfcn.h>
 */
void* dlsym(void *handle, const char *symbol);

/**
 * dlerror(3) from <dlfcn.h>, currently not thread save
 */
char* dlerror(void);

/**
 * dlclose() from <dlfcn.h>
 */
int dlclose(void *handle);

/**
 * socketpair(2) for SOCK_STREAM, uses TCP on loopback
 */
int socketpair(int domain, int type, int protocol, int sv[2]);

/**
 * getpass(3) on Windows consoles
 */
char* getpass(const char *prompt);
#define HAVE_GETPASS

/**
 * Map MSG_DONTWAIT to the reserved, but deprecated MSG_INTERRUPT
 */
#define MSG_DONTWAIT MSG_INTERRUPT

/**
 * shutdown(2) "how"-aliases, to use Unix variant on Windows
 */
#define SHUT_RD SD_RECEIVE
#define SHUT_WR SD_SEND
#define SHUT_RDWR SD_BOTH

/**
 * shutdown(2) setting errno
 */
#define shutdown windows_shutdown
int windows_shutdown(int sockfd, int how);

/**
 * accept(2) setting errno
 */
#define accept windows_accept
int windows_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

/**
 * bind(2) setting errno
 */
#define bind windows_bind
int windows_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

/**
 * connect(2) setting errno
 */
#define connect windows_connect
int windows_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

/**
 * getsockname(2) setting errno
 */
#define getsockname windows_getsockname
int windows_getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

/**
 * getsockopt(2) setting errno
 */
#define getsockopt windows_getsockopt
int windows_getsockopt(int sockfd, int level, int optname,
					   void *optval, socklen_t *optlen);

/**
 * setsockopt(2) setting errno
 */
#define setsockopt windows_setsockopt
int windows_setsockopt(int sockfd, int level, int optname,
					   const void *optval, socklen_t optlen);

/**
 * socket(2) setting errno
 */
#define socket windows_socket
int windows_socket(int domain, int type, int protocol);

/**
 * select(2) setting errno
 */
#define select windows_select
int windows_select(int nfds, fd_set *readfds, fd_set *writefds,
				   fd_set *exceptfds, struct timeval *timeout);

/**
 * close(2) working for file handles and Winsock sockets
 */
#define close windows_close
int windows_close(int fd);

/**
 * recv(2) with support for MSG_DONTWAIT
 */
#define recv windows_recv
ssize_t windows_recv(int sockfd, void *buf, size_t len, int flags);

/**
 * recvfrom(2) with support for MSG_DONTWAIT
 */
#define recvfrom windows_recvfrom
ssize_t windows_recvfrom(int sockfd, void *buf, size_t len, int flags,
						 struct sockaddr *src_addr, socklen_t *addrlen);

/**
 * recvfrom(2) with support for MSG_DONTWAIT
 */
#define send windows_send
ssize_t windows_send(int sockfd, const void *buf, size_t len, int flags);

/**
 * recvfrom(2) with support for MSG_DONTWAIT
 */
#define sendto windows_send
ssize_t windows_sendto(int sockfd, const void *buf, size_t len, int flags,
					   const struct sockaddr *dest_addr, socklen_t addrlen);

/**
 * read(2) working on files and sockets, cancellable on sockets only
 *
 * On Windows, there does not seem to be a way how a cancellable read can
 * be implemented on Low level I/O functions for files, _pipe()s or stdio.
 */
#define read windows_read
ssize_t windows_read(int fd, void *buf, size_t count);

/**
 * write(2) working on files and sockets
 */
#define write windows_write
ssize_t windows_write(int fd, void *buf, size_t count);

#if _WIN32_WINNT < 0x0600
/**
 * Define pollfd and flags on our own if not specified
 */
struct pollfd {
	SOCKET fd;
	short events;
	short revents;
};
enum {
	POLLERR =		0x0001,
	POLLHUP =		0x0002,
	POLLNVAL =		0x0004,
	POLLWRNORM =	0x0010,
	POLLWRBAND =	0x0020,
	POLLPRI =		0x0400,
	POLLRDNORM =	0x0100,
	POLLRDBAND =	0x0200,
	POLLIN =		POLLRDNORM | POLLRDBAND,
	POLLOUT =		POLLWRNORM,
};
#endif /* _WIN32_WINNT < 0x0600 */

/**
 * poll(2), implemented using Winsock2 WSAPoll()
 */
int poll(struct pollfd *fds, int nfds, int timeout);

/**
 * Declaration missing on older WinGW
 */
_CRTIMP errno_t strerror_s(char *buf, size_t size, int errnum);

/**
 * strerror_s, but supporting POSIX compatibility errno >= 100
 */
#define strerror_s strerror_s_extended
int strerror_s_extended(char *buf, size_t buflen, int errnum);

/**
 * strerror_r(2) replacement, XSI variant
 */
static inline int strerror_r(int errnum, char *buf, size_t buflen)
{
	return strerror_s(buf, buflen, errnum);
}
#define HAVE_STRERROR_R /* but not STRERROR_R_CHAR_P */

/**
 * MinGW does provide extended errno values. Windows itself knowns them
 * for POSIX compatibility; we define them as well.
 */
#ifndef EADDRINUSE
#define EADDRINUSE			100
#endif
#ifndef EADDRNOTAVAIL
#define EADDRNOTAVAIL		101
#endif
#ifndef EAFNOSUPPORT
#define EAFNOSUPPORT		102
#endif
#ifndef EALREADY
#define EALREADY			103
#endif
#ifndef EBADMSG
#define EBADMSG				104
#endif
#ifndef ECANCELED
#define ECANCELED			105
#endif
#ifndef ECONNABORTED
#define ECONNABORTED		106
#endif
#ifndef ECONNREFUSED
#define ECONNREFUSED		107
#endif
#ifndef ECONNRESET
#define ECONNRESET			108
#endif
#ifndef EDESTADDRREQ
#define EDESTADDRREQ		109
#endif
#ifndef EHOSTUNREACH
#define EHOSTUNREACH		110
#endif
#ifndef EIDRM
#define EIDRM				111
#endif
#ifndef EINPROGRESS
#define EINPROGRESS			112
#endif
#ifndef EISCONN
#define EISCONN				113
#endif
#ifndef ELOOP
#define ELOOP				114
#endif
#ifndef EMSGSIZE
#define EMSGSIZE			115
#endif
#ifndef ENETDOWN
#define ENETDOWN			116
#endif
#ifndef ENETRESET
#define ENETRESET			117
#endif
#ifndef ENETUNREACH
#define ENETUNREACH			118
#endif
#ifndef ENOBUFS
#define ENOBUFS				119
#endif
#ifndef ENODATA
#define ENODATA				120
#endif
#ifndef ENOLINK
#define ENOLINK				121
#endif
#ifndef ENOMSG
#define ENOMSG				122
#endif
#ifndef ENOPROTOOPT
#define ENOPROTOOPT			123
#endif
#ifndef ENOSR
#define ENOSR				124
#endif
#ifndef ENOSTR
#define ENOSTR				125
#endif
#ifndef ENOTCONN
#define ENOTCONN			126
#endif
#ifndef ENOTRECOVERABLE
#define ENOTRECOVERABLE		127
#endif
#ifndef ENOTSOCK
#define ENOTSOCK			128
#endif
#ifndef ENOTSUP
#define ENOTSUP				129
#endif
#ifndef EOPNOTSUPP
#define EOPNOTSUPP			130
#endif
#ifndef EOTHER
#define EOTHER				131
#endif
#ifndef EOVERFLOW
#define EOVERFLOW			132
#endif
#ifndef EOWNERDEAD
#define EOWNERDEAD			133
#endif
#ifndef EPROTO
#define EPROTO				134
#endif
#ifndef EPROTONOSUPPORT
#define EPROTONOSUPPORT		135
#endif
#ifndef EPROTOTYPE
#define EPROTOTYPE			136
#endif
#ifndef ETIME
#define ETIME				137
#endif
#ifndef ETIMEDOUT
#define ETIMEDOUT			138
#endif
#ifndef ETXTBSY
#define ETXTBSY				139
#endif
#ifndef EWOULDBLOCK
#define EWOULDBLOCK			140
#endif


/* Windows does not support "ll" format printf length modifiers. Mingw
 * therefore maps these to the Windows specific I64 length modifier. That
 * won't work for us, as we use our own printf backend on Windows, which works
 * just fine with "ll". */
#undef PRId64
#define PRId64 "lld"
#undef PRId64
#define PRId64 "lld"
#undef PRIdLEAST64
#define PRIdLEAST64 "lld"
#undef PRIdFAST64
#define PRIdFAST64 "lld"
#undef PRIdMAX
#define PRIdMAX "lld"
#undef PRIi64
#define PRIi64 "lli"
#undef PRIiLEAST64
#define PRIiLEAST64 "lli"
#undef PRIiFAST64
#define PRIiFAST64 "lli"
#undef PRIiMAX
#define PRIiMAX "lli"
#undef PRIo64
#define PRIo64 "llo"
#undef PRIoLEAST64
#define PRIoLEAST64 "llo"
#undef PRIoFAST64
#define PRIoFAST64 "llo"
#undef PRIoMAX
#define PRIoMAX "llo"
#undef PRIu64
#define PRIu64 "llu"
#undef PRIuLEAST64
#define PRIuLEAST64 "llu"
#undef PRIuFAST64
#define PRIuFAST64 "llu"
#undef PRIuMAX
#define PRIuMAX "llu"
#undef PRIx64
#define PRIx64 "llx"
#undef PRIxLEAST64
#define PRIxLEAST64 "llx"
#undef PRIxFAST64
#define PRIxFAST64 "llx"
#undef PRIxMAX
#define PRIxMAX "llx"
#undef PRIX64
#define PRIX64 "llX"
#undef PRIXLEAST64
#define PRIXLEAST64 "llX"
#undef PRIXFAST64
#define PRIXFAST64 "llX"
#undef PRIXMAX
#define PRIXMAX "llX"

#ifdef _WIN64
# undef PRIdPTR
# define PRIdPTR "lld"
# undef PRIiPTR
# define PRIiPTR "lli"
# undef PRIoPTR
# define PRIoPTR "llo"
# undef PRIuPTR
# define PRIuPTR "llu"
# undef PRIxPTR
# define PRIxPTR "llx"
# undef PRIXPTR
# define PRIXPTR "llX"
#endif /* _WIN64 */

#endif /** WINDOWS_H_ @}*/
