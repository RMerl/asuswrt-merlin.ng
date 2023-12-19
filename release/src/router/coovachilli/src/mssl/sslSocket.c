/*
 *	socketLayer.c
 *	Release $Name: MATRIXSSL_1_8_6_OPEN $
 *
 *	Sample SSL socket layer for MatrixSSL example exectuables
 */
/*
 *	Copyright (c) PeerSec Networks, 2002-2008. All Rights Reserved.
 *	The latest version of this code is available at http://www.matrixssl.org
 *
 *	This software is open source; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This General Public License does NOT permit incorporating this software 
 *	into proprietary programs.  If you are unable to comply with the GPL, a 
 *	commercial license for this software may be purchased from PeerSec Networks
 *	at http://www.peersec.com
 *	
 *	This program is distributed in WITHOUT ANY WARRANTY; without even the 
 *	implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
 *	See the GNU General Public License for more details.
 *	
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *	http://www.gnu.org/copyleft/gpl.html
 */
/******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include "sslSocket.h"

SOCKET socketListen(short port, int *err)
{
  struct sockaddr_in	addr;
  SOCKET				fd;
  int					rc;
  
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;
  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    fprintf(stderr, "Error creating listen socket\n");
    *err = getSocketError();
    return INVALID_SOCKET;
  }
  fcntl(fd, F_SETFD, FD_CLOEXEC);
  rc = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&rc, sizeof(rc));
  
  if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
    fprintf(stderr, 
	    "Can't bind socket. Port in use or insufficient privilege\n");
    *err = getSocketError();
    return INVALID_SOCKET;
  }
  if (listen(fd, SOMAXCONN) < 0) {
    fprintf(stderr, "Error listening on socket\n");
    *err = getSocketError();
    return INVALID_SOCKET;
  }
  return fd;
}

SOCKET socketAccept(SOCKET listenfd, int *err)
{
  struct sockaddr_in	addr;
  SOCKET	fd;
  int	len;

  len = sizeof(addr);
  if ((fd = accept(listenfd, (struct sockaddr *)&addr, &len)) 
      == INVALID_SOCKET) {
    *err = getSocketError();
    if (*err != WOULD_BLOCK) {
      fprintf(stderr, "Error %d accepting new socket\n", *err);
    }
    return INVALID_SOCKET;
  }
  setSocketNodelay(fd);
  setSocketBlock(fd);
  return fd;
}

SOCKET socketConnect(char *ip, short port, int *err)
{
  struct sockaddr_in addr;
  SOCKET fd;
  int	rc;
  
  struct hostent *hp;
  
  if(!(hp = gethostbyname(ip))) {
    fprintf(stderr, "Host not found %s\n",ip);
    return INVALID_SOCKET;
  }
  
  memset(&addr,0,sizeof(addr));
  addr.sin_addr=*(struct in_addr *)hp->h_addr_list[0];
  addr.sin_family=AF_INET;
  addr.sin_port=htons(port);
  
  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
    fprintf(stderr, "Error creating socket\n");
    *err = getSocketError();
    return INVALID_SOCKET;
  }

  fcntl(fd, F_SETFD, FD_CLOEXEC);
  rc = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&rc, sizeof(rc));
  setSocketNodelay(fd);
  setSocketBlock(fd);
  
  rc = connect(fd, (struct sockaddr *)&addr, sizeof(addr));
#if WIN
  if (rc != 0) 
#else
  if (rc < 0) 
#endif
  {
    *err = getSocketError();
    perror("connect");
    return INVALID_SOCKET;
  }

  return fd;
}

int sslAccept(sslConn_t **cpp, SOCKET fd, sslKeys_t *keys,
	      int (*certValidator)(sslCertInfo_t *t, void *arg), int flags)
{
  sslConn_t		*conn;
  unsigned char	buf[1024];
  int				status, rc;
  conn = calloc(sizeof(sslConn_t), 1);
  conn->fd = fd;
  if (matrixSslNewSession(&conn->ssl, keys, NULL,
			  SSL_FLAGS_SERVER | flags) < 0) {
    sslFreeConnection(&conn);
    return -1;
  }
  
#ifdef USE_CLIENT_AUTH
  matrixSslSetCertValidator(conn->ssl, certValidator, keys);
#endif /* USE_CLIENT_AUTH */
  memset(&conn->inbuf, 0x0, sizeof(sslBuf_t));
  conn->insock.size = 1024;
  conn->insock.start = conn->insock.end = conn->insock.buf = 
    (unsigned char *)malloc(conn->insock.size);
  conn->outsock.size = 1024;
  conn->outsock.start = conn->outsock.end = conn->outsock.buf = 
    (unsigned char *)malloc(conn->outsock.size);
  conn->inbuf.size = 0;
  conn->inbuf.start = conn->inbuf.end = conn->inbuf.buf = NULL;
  *cpp = conn;
  
 readMore:
  rc = sslRead(conn, buf, sizeof(buf), &status);
  if (rc == 0) {
    if (status == SSLSOCKET_EOF || status == SSLSOCKET_CLOSE_NOTIFY) {
      sslFreeConnection(&conn);
      return -1;
    }
    if (matrixSslHandshakeIsComplete(conn->ssl) == 0) {
      goto readMore;
    }
  } else if (rc > 0) {
    socketAssert(0);
    return -1;
  } else {
    fprintf(stderr, "sslRead error in sslAccept\n");
    sslFreeConnection(&conn);
    return -1;
  }
  *cpp = conn;
  
  return 0;
}

int sslConnect(sslConn_t **cpp, SOCKET fd, sslKeys_t *keys, 
	       sslSessionId_t *id, short cipherSuite, 
	       int (*certValidator)(sslCertInfo_t *t, void *arg))
{
  sslConn_t *conn;

  conn = calloc(sizeof(sslConn_t), 1);
  conn->fd = fd;

  if (matrixSslNewSession(&conn->ssl, keys, id, 0) < 0) {
    fprintf(stderr, "error %s:%d\n",__FILE__,__LINE__);
    sslFreeConnection(&conn);
    return -1;
  }
  
  matrixSslSetCertValidator(conn->ssl, certValidator, keys);
  
  *cpp = sslDoHandshake(conn, cipherSuite);
  
  if (*cpp == NULL) {
    fprintf(stderr, "error %s:%d\n",__FILE__,__LINE__);
    return -1;
  }

  return 0;
}

sslConn_t *sslDoHandshake(sslConn_t *conn, short cipherSuite)
{
  char	buf[1024];
  int	bytes, status, rc;
  
  conn->insock.size = 1024;
  conn->insock.start = conn->insock.end = conn->insock.buf = 
    (unsigned char *)malloc(conn->insock.size);
  conn->outsock.size = 1024;
  conn->outsock.start = conn->outsock.end = conn->outsock.buf = 
    (unsigned char *)malloc(conn->outsock.size);
  conn->inbuf.size = 0;
  conn->inbuf.start = conn->inbuf.end = conn->inbuf.buf = NULL;
  
  bytes = matrixSslEncodeClientHello(conn->ssl, &conn->outsock, cipherSuite);
  if (bytes < 0) {
    fprintf(stderr, "error %s:%d\n",__FILE__,__LINE__);
    socketAssert(bytes < 0);
    goto error;
  }
  if (psSocketWrite(conn->fd, &conn->outsock) < 0) {
    fprintf(stdout, "Error in socketWrite\n");
    goto error;
  }
  conn->outsock.start = conn->outsock.end = conn->outsock.buf;
 readMore:
  rc = sslRead(conn, buf, sizeof(buf), &status);
  if (rc == 0) {
    if (status == SSLSOCKET_EOF || status == SSLSOCKET_CLOSE_NOTIFY) {
      fprintf(stderr, "error %s:%d\n",__FILE__,__LINE__);
      goto error;
    }
    if (matrixSslHandshakeIsComplete(conn->ssl) == 0) {
      goto readMore;
    }
  } else if (rc > 0) {
    fprintf(stderr, "sslRead got %d data in sslDoHandshake %s\n", rc, buf);
    goto readMore;
  } else {
    fprintf(stderr, "sslRead error in sslDoHandhake\n");
    goto error;
  }
  
  return conn;
  
 error:
  fprintf(stderr, "error %s:%d\n",__FILE__,__LINE__);
  sslFreeConnection(&conn);
  return NULL;
}

int sslRead(sslConn_t *cp, char *buf, int len, int *status)
{
  int				bytes, rc, remaining;
  unsigned char	error, alertLevel, alertDescription, performRead;
  
  *status = 0;
  
  if (cp->ssl == NULL || len <= 0) {
    return -1;
  }
  if (cp->inbuf.buf) {
    if (cp->inbuf.start < cp->inbuf.end) {
      remaining = (int)(cp->inbuf.end - cp->inbuf.start);
      bytes = (int)min(len, remaining);
      memcpy(buf, cp->inbuf.start, bytes);
      cp->inbuf.start += bytes;
      return bytes;
    }
    free(cp->inbuf.buf);
    cp->inbuf.buf = NULL;
  }
  if (cp->insock.buf < cp->insock.start) {
    if (cp->insock.start == cp->insock.end) {
      cp->insock.start = cp->insock.end = cp->insock.buf;
    } else {
      memmove(cp->insock.buf, cp->insock.start, cp->insock.end - cp->insock.start);
      cp->insock.end -= (cp->insock.start - cp->insock.buf);
      cp->insock.start = cp->insock.buf;
    }
  }
  performRead = 0;
 readMore:
  if (cp->insock.end == cp->insock.start || performRead) {
    performRead = 1;
    bytes = recv(cp->fd, (char *)cp->insock.end, 
		 (int)((cp->insock.buf + cp->insock.size) - cp->insock.end), MSG_NOSIGNAL);
    if (bytes == SOCKET_ERROR) {
      *status = getSocketError();
      return -1;
    }
    if (bytes == 0) {
      *status = SSLSOCKET_EOF;
      return 0;
    }
    cp->insock.end += bytes;
  }
  cp->inbuf.start = cp->inbuf.end = cp->inbuf.buf = malloc(len);
  cp->inbuf.size = len;
 decodeMore:
  error = 0;
  alertLevel = 0;
  alertDescription = 0;
  
  rc = matrixSslDecode(cp->ssl, &cp->insock, &cp->inbuf, &error, &alertLevel, 
		       &alertDescription);
  switch (rc) {
  case SSL_SUCCESS:
    return 0;
  case SSL_PROCESS_DATA:
    rc = (int)(cp->inbuf.end - cp->inbuf.start);
    rc = min(rc, len);
    memcpy(buf, cp->inbuf.start, rc);
    cp->inbuf.start += rc;
    return rc;
  case SSL_SEND_RESPONSE:
    bytes = send(cp->fd, (char *)cp->inbuf.start, 
		 (int)(cp->inbuf.end - cp->inbuf.start), MSG_NOSIGNAL);
    if (bytes == SOCKET_ERROR) {
      *status = getSocketError();
      if (*status != WOULD_BLOCK) {
	fprintf(stdout, "Socket send error:  %d\n", *status);
	goto readError;
      }
      *status = 0;
    }
    cp->inbuf.start += bytes;
    if (cp->inbuf.start < cp->inbuf.end) {
      setSocketBlock(cp->fd);
      bytes = send(cp->fd, (char *)cp->inbuf.start, 
		   (int)(cp->inbuf.end - cp->inbuf.start), MSG_NOSIGNAL);
      if (bytes == SOCKET_ERROR) {
	*status = getSocketError();
	goto readError;
      }
      cp->inbuf.start += bytes;
      socketAssert(cp->inbuf.start == cp->inbuf.end);
      setSocketNonblock(cp->fd);
    }
    cp->inbuf.start = cp->inbuf.end = cp->inbuf.buf;
    return 0;
  case SSL_ERROR:
    fprintf(stderr, "SSL: Closing on protocol error %d\n", error);
    if (cp->inbuf.start < cp->inbuf.end) {
      setSocketNonblock(cp->fd);
      bytes = send(cp->fd, (char *)cp->inbuf.start, 
		   (int)(cp->inbuf.end - cp->inbuf.start), MSG_NOSIGNAL);
    }
    goto readError;
  case SSL_ALERT:
    if (alertDescription == SSL_ALERT_CLOSE_NOTIFY) {
      *status = SSLSOCKET_CLOSE_NOTIFY;
      goto readZero;
    }
    fprintf(stderr, "SSL: Closing on client alert %d: %d\n",
	    alertLevel, alertDescription);
    goto readError;
  case SSL_PARTIAL:
    if (cp->insock.start == cp->insock.buf && cp->insock.end == 
	(cp->insock.buf + cp->insock.size)) {
      if (cp->insock.size > SSL_MAX_BUF_SIZE) {
	goto readError;
      }
      cp->insock.size *= 2;
      cp->insock.start = cp->insock.buf = 
	(unsigned char *)realloc(cp->insock.buf, cp->insock.size);
      cp->insock.end = cp->insock.buf + (cp->insock.size / 2);
    }
    if (!performRead) {
      performRead = 1;
      free(cp->inbuf.buf);
      cp->inbuf.buf = NULL;
      goto readMore;
    } else {
      goto readZero;
    }
  case SSL_FULL:
    cp->inbuf.size *= 2;
    if (cp->inbuf.buf != (unsigned char*)buf) {
      free(cp->inbuf.buf);
      cp->inbuf.buf = NULL;
    }
    cp->inbuf.start = cp->inbuf.end = cp->inbuf.buf = 
      (unsigned char *)malloc(cp->inbuf.size);
    goto decodeMore;
  }
 readZero:
  if (cp->inbuf.buf == (unsigned char*)buf) {
    cp->inbuf.buf = NULL;
  }
  return 0;
 readError:
  if (cp->inbuf.buf == (unsigned char*)buf) {
    cp->inbuf.buf = NULL;
  }
  return -1;
}

int sslWrite(sslConn_t *cp, char *buf, int len, int *status)
{
  int		rc;
  
  *status = 0;
  if (cp->outsock.buf < cp->outsock.start) {
    if (cp->outsock.start == cp->outsock.end) {
      cp->outsock.start = cp->outsock.end = cp->outsock.buf;
    } else {
      memmove(cp->outsock.buf, cp->outsock.start, cp->outsock.end - cp->outsock.start);
      cp->outsock.end -= (cp->outsock.start - cp->outsock.buf);
      cp->outsock.start = cp->outsock.buf;
    }
  }
  if (cp->outBufferCount > 0 && len != cp->outBufferCount) {
    socketAssert(len != cp->outBufferCount);
    return -1;
  }
  if (cp->outBufferCount == 0) {
  retryEncode:
    rc = matrixSslEncode(cp->ssl, (unsigned char *)buf, len, &cp->outsock);
    switch (rc) {
    case SSL_ERROR:
      return -1;
    case SSL_FULL:
      if (cp->outsock.size > SSL_MAX_BUF_SIZE) {
	return -1;
      }
      cp->outsock.size *= 2;
      cp->outsock.buf = 
	(unsigned char *)realloc(cp->outsock.buf, cp->outsock.size);
      cp->outsock.end = cp->outsock.buf + (cp->outsock.end - cp->outsock.start);
      cp->outsock.start = cp->outsock.buf;
      goto retryEncode;
    }
  }
  rc = send(cp->fd, (char *)cp->outsock.start, 
	    (int)(cp->outsock.end - cp->outsock.start), MSG_NOSIGNAL);
  if (rc == SOCKET_ERROR) {
    *status = getSocketError();
    return -1;
	}
  cp->outsock.start += rc;
  if (cp->outsock.start == cp->outsock.end) {
    cp->outBufferCount = 0;
    return len;
  }
  cp->outBufferCount = len;
  return 0;
}

void sslWriteClosureAlert(sslConn_t *cp)
{
  if (cp != NULL) {
    cp->outsock.start = cp->outsock.end = cp->outsock.buf;
    matrixSslEncodeClosureAlert(cp->ssl, &cp->outsock);
    setSocketNonblock(cp->fd);
    send(cp->fd, cp->outsock.start,
	 (int)(cp->outsock.end - cp->outsock.start), MSG_NOSIGNAL);
  }
}

void sslRehandshake(sslConn_t *cp)
{
  matrixSslEncodeHelloRequest(cp->ssl, &cp->outsock);
  psSocketWrite(cp->fd, &cp->outsock);
  cp->outsock.start = cp->outsock.end = cp->outsock.buf;
}

void sslFreeConnection(sslConn_t **cpp)
{
  sslConn_t	*conn;
  
  conn = *cpp;
  matrixSslDeleteSession(conn->ssl);
  conn->ssl = NULL;
  if (conn->insock.buf) {
    free(conn->insock.buf);
    conn->insock.buf = NULL;
  }
  if (conn->outsock.buf) {
    free(conn->outsock.buf);
    conn->outsock.buf = NULL;
  }
  if (conn->inbuf.buf) {
    free(conn->inbuf.buf);
    conn->inbuf.buf = NULL;
  }
  free(conn);
  *cpp = NULL;
}

void sslFreeConnectionBuffers(sslConn_t **cpp)
{
  sslConn_t	*conn;
  
  conn = *cpp;
  if (conn->insock.buf) {
    free(conn->insock.buf);
    conn->insock.buf = NULL;
  }
  if (conn->outsock.buf) {
    free(conn->outsock.buf);
    conn->outsock.buf = NULL;
  }
  if (conn->inbuf.buf) {
    free(conn->inbuf.buf);
    conn->inbuf.buf = NULL;
  }
}

void socketShutdown(SOCKET sock)
{
  char	buf[32];
  
  if (sock != INVALID_SOCKET) {
    setSocketNonblock(sock);
    if (shutdown(sock, 1) >= 0) {
      while (recv(sock, buf, sizeof(buf), 0) > 0);
    }
    closesocket(sock);
  }
}

int psSocketWrite(SOCKET sock, sslBuf_t *out)
{
  unsigned char	*s;
  int				bytes;
  
  s = out->start;
  while (out->start < out->end) {
    bytes = send(sock, out->start, (int)(out->end - out->start), MSG_NOSIGNAL);
    if (bytes == SOCKET_ERROR) {
      return -1;
    }
    out->start += bytes;
  }
  return (int)(out->start - s);
}

int psSocketRead(SOCKET sock, sslBuf_t **out, int *status)
{
  sslBuf_t	*local;
  char		*c;
  int			bytes;
  
  local = *out;
  c = local->start;
  
  bytes = recv(sock, c, (int)((local->buf + local->size) - local->end), MSG_NOSIGNAL);
  if (bytes == SOCKET_ERROR) {
    *status = getSocketError();
    return -1;
  }
  if (bytes == 0) {
    *status = SSLSOCKET_EOF;
    return 0;
  }
  local->end += bytes;
  return bytes;
}

void setSocketBlock(SOCKET sock)
{
#if _WIN32
  int		block = 0;
  ioctlsocket(sock, FIONBIO, &block);
#elif LINUX
  fcntl(sock, F_SETFL, fcntl(sock, F_GETFL) & ~O_NONBLOCK);
  fcntl(sock, F_SETFD, FD_CLOEXEC);
#endif
}

void setSocketNonblock(SOCKET sock)
{
#if _WIN32
  int		block = 1;
  ioctlsocket(sock, FIONBIO, &block);
#elif LINUX
  fcntl(sock, F_SETFL, fcntl(sock, F_GETFL) | O_NONBLOCK);
#endif
}

void setSocketNodelay(SOCKET sock)
{
#if _WIN32
  BOOL	tmp = TRUE;
#else
  int		tmp = 1;
#endif /* WIN32 */
  setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *)&tmp, sizeof(tmp));
}

void breakpoint()
{
  static int preventInline = 0;
#if _WIN32
  DebugBreak();
#elif LINUX
  abort();
#endif
}



#if WINCE || VXWORKS

void parseCmdLineArgs(char *args, int *pargc, char ***pargv)
{
  char			**argv;
  char			*ptr;
  int				size, i;
  
  for (size = 3, ptr = args; ptr && *ptr != '\0'; ptr++) {
    if (isspace(*ptr)) {
      size++;
      while (isspace(*ptr)) {
	ptr++;
      }
      if (*ptr == '\0') {
	break;
      }
    }
  }
  argv = (char**) malloc(size * sizeof(char*));
  *pargv = argv;
  
  for (i = 1, ptr = args; ptr && *ptr != '\0'; i++) {
    while (isspace(*ptr)) {
      ptr++;
    }
    if (*ptr == '\0')  {
      break;
    }
    if (*ptr == '"') {
      ptr++;
      argv[i] = ptr;
      while ((*ptr != '\0') && (*ptr != '"')) {
	ptr++;
      }
    } else {
      argv[i] = ptr;
      while (*ptr != '\0' && !isspace(*ptr)) {
	ptr++;
      }
    }
    if (*ptr != '\0') {
      *ptr = '\0';
      ptr++;
    }
  }
  argv[i] = NULL;
  *pargc = i ;
  
  argv[0] = "PeerSec";
  for (ptr = argv[0]; *ptr; ptr++) {
    if (*ptr == '\\') {
      *ptr = '/';
    }
  }
}
#endif /* WINCE || VXWORKS */

#ifdef WINCE

static FILETIME YearToFileTime(WORD wYear)
{	
  SYSTEMTIME sbase;
  FILETIME fbase;
  
  sbase.wYear         = wYear;
  sbase.wMonth        = 1;
  sbase.wDayOfWeek    = 1; //assumed
  sbase.wDay          = 1;
  sbase.wHour         = 0;
  sbase.wMinute       = 0;
  sbase.wSecond       = 0;
  sbase.wMilliseconds = 0;
  
  SystemTimeToFileTime( &sbase, &fbase );
  
  return fbase;
}

time_t time() {
  
  __int64 time1, time2, iTimeDiff;
  FILETIME fileTime1, fileTime2;
  SYSTEMTIME  sysTime;
  
  fileTime1 = YearToFileTime(1970);
  
  GetSystemTime(&sysTime);
  SystemTimeToFileTime(&sysTime, &fileTime2);
  
  
  time1 = fileTime1.dwHighDateTime;
  time1 <<= 32;				
  time1 |= fileTime1.dwLowDateTime;

  time2 = fileTime2.dwHighDateTime;
  time2 <<= 32;				
  time2 |= fileTime2.dwLowDateTime;
  
  iTimeDiff = (time2 - time1) / 10000000;
  return (int)iTimeDiff;
}
#endif /* WINCE */



