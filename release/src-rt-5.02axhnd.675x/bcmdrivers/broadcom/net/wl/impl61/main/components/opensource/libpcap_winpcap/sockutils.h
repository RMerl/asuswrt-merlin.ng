/*
 * Copyright (c) 2002 - 2003
 * NetGroup, Politecnico di Torino (Italy)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Politecnico di Torino nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef __SOCKUTILS_H__
#define __SOCKUTILS_H__

#if _MSC_VER > 1000
#pragma once
#endif // endif

#ifdef WIN32
/* Prevents a compiler warning in case this was already defined (to avoid that */
/* windows.h includes winsock.h */
#ifdef _WINSOCKAPI_
#undef _WINSOCKAPI_
#endif // endif
/* Need windef.h for defines used in winsock2.h under MingW32 */
#ifdef __MINGW32__
#include <windef.h>
#endif // endif
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <stdio.h>
#include <string.h>	/* for memset() */
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>	/* DNS lookup */
#include <unistd.h>	/* close() */
#include <errno.h>	/* errno() */
#include <netinet/in.h> /* for sockaddr_in, in BSD at least */
#include <arpa/inet.h>
#include <net/if.h>
#endif // endif

/* MingW headers include this definition, but only for Windows XP and above.
   MSDN states that this function is available for most versions on Windows.
*/
#if ((defined(__MINGW32__)) && (_WIN32_WINNT < 0x0501))
int WSAAPI getnameinfo(const struct sockaddr*,socklen_t,char*,DWORD,
		       char*,DWORD,int);
#endif // endif

/*!
	\defgroup SockUtils Cross-platform socket utilities (IPv4-IPv6)
*/

/*! \addtogroup SockUtils
	\{
*/

/*!
	\defgroup ExportedStruct Exported Structures and Definitions
*/

/*! \addtogroup ExportedStruct
	\{
*/

// Some minor differences between UNIX and Win32
#ifdef WIN32
/*!
	\brief In Win32, sockets use unsigned integers; in UNIX, they use signed integer.

	So, we define a generic SOCKET in order to be cross-platform compatible.
*/
	#define SOCKET unsigned int
#else
/*!
	\brief In Win32, sockets use unsigned integers; in UNIX, they use signed integer.

	So, we define a generic SOCKET in order to be cross-platform compatible.
*/

	#define SOCKET int

/*!
	\brief In Win32, the close() call cannot be used for socket.

	So, we define a generic closesocket() call in order to be cross-platform compatible.
*/
	#define closesocket(a) close(a)
#endif // endif

/*!
	\brief DEBUG facility: it prints an error message on the screen (stderr)

	This macro prints the error on the standard error stream (stderr);
	if we are working in debug mode (i.e. there is no NDEBUG defined) and we are in
	Microsoft Visual C++, the error message will appear on the MSVC console as well.

	When NDEBUG is defined, this macro is empty.

	\param msg: the message you want to print.

	\param expr: 'false' if you want to abort the program, 'true' it you want
	to print the message and continue.

	\return No return values.
*/
#ifdef NDEBUG
#define SOCK_ASSERT(msg, expr) ((void)0)
#else
	#include <assert.h>
	#if (defined(WIN32) && defined(_MSC_VER))
		#include <crtdbg.h>				// for _CrtDbgReport
		// Use MessageBox(NULL, msg, "warning", MB_OK)' instead of the other calls if you want to debug a Win32 service
		// Remember to activate the 'allow service to interact with desktop' flag of the service
		#define SOCK_ASSERT(msg, expr) { _CrtDbgReport(_CRT_WARN, NULL, 0, NULL, "%s\n", msg); fprintf(stderr, "%s\n", msg); assert(expr); }
	#else
		#define SOCK_ASSERT(msg, expr) { fprintf(stderr, "%s\n", msg); assert(expr); }
	#endif
#endif // endif

/****************************************************
 *                                                  *
 * Exported functions / definitions                 *
 *                                                  *
 ****************************************************/

//! 'checkonly' flag, into the rpsock_bufferize()
#define SOCKBUF_CHECKONLY 1
//! no 'checkonly' flag, into the rpsock_bufferize()
#define SOCKBUF_BUFFERIZE 0

//! no 'server' flag; it opens a client socket
#define SOCKOPEN_CLIENT 0
//! 'server' flag; it opens a server socket
#define SOCKOPEN_SERVER 1

//! Changes the behaviour of the sock_recv(); it does not wait to receive all data
#define SOCK_RECEIVEALL_NO 0
//! Changes the behaviour of the sock_recv(); it waits to receive all data
#define SOCK_RECEIVEALL_YES 1

/*!
	\}
*/

#ifdef __cplusplus
extern "C" {
#endif // endif

/*!
	\defgroup ExportedFunc Exported Functions
*/

/*! \addtogroup ExportedFunc
	\{
*/

int sock_init(char *errbuf, int errbuflen);
void sock_cleanup();
// It is 'public' because there are calls (like accept() ) which are not managed from inside the sockutils files
void sock_geterror(const char *caller, char *errbuf, int errbufsize);
int sock_initaddress(const char *address, const char *port,
						struct addrinfo *hints, struct addrinfo **addrinfo, char *errbuf, int errbuflen);
int sock_recv(SOCKET socket, char *buffer, int size, int receiveall, char *errbuf, int errbuflen);
SOCKET sock_open(struct addrinfo *addrinfo, int server, int nconn, char *errbuf, int errbuflen);
int sock_close(SOCKET sock, char *errbuf, int errbuflen);

int sock_send(SOCKET socket, const char *buffer, int size, char *errbuf, int errbuflen);
int sock_bufferize(const char *buffer, int size, char *tempbuf, int *offset, int totsize, int checkonly, char *errbuf, int errbuflen);
int sock_discard(SOCKET socket, int size, char *errbuf, int errbuflen);
int	sock_check_hostlist(char *hostlist, const char *sep, struct sockaddr_storage *from, char *errbuf, int errbuflen);
int sock_cmpaddr(struct sockaddr_storage *first, struct sockaddr_storage *second);

int sock_getmyinfo(SOCKET sock, char *address, int addrlen, char *port, int portlen, int flags, char *errbuf, int errbuflen);

int sock_getascii_addrport(const struct sockaddr_storage *sockaddr, char *address, int addrlen, char *port, int portlen, int flags, char *errbuf, int errbuflen);
int sock_present2network(const char *address, struct sockaddr_storage *sockaddr, int addr_family, char *errbuf, int errbuflen);

#ifdef __cplusplus
}
#endif // endif

/*!
	\}
*/

/*!
	\}
*/

#endif // endif
