/*
 *	httpClient.c
 *	Release $Name: MATRIXSSL_1_8_6_OPEN $
 *
 *	Simple example program for MatrixSSL
 *	Sends a HTTPS request and echos the response back to the sender.
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
#ifndef WINCE
#include <time.h>
#endif

#include "sslSocket.h"

#ifdef CAPC_SYSCONFDIR
#define SYSCONFDIR  CAPC_SYSCONFDIR
#else
#ifndef SYSCONFDIR
#define SYSCONFDIR "/etc/"
#endif
#endif

#define KEYFILE  SYSCONFDIR"/node_key.pem"
#define CERTFILE SYSCONFDIR"/node_cert.pem"
#define CA_LIST  SYSCONFDIR"/node_ca.pem"

#define PASSWORD NULL
#define HOST	 "dashboard"
#define RANDOM   "random.pem"
#define PORT	2444

static char *postFile=0;
static char *certFile=CERTFILE;
static char *keyFile=KEYFILE;
static char *keyPass=PASSWORD;
static char *caFile=CA_LIST;

static char *uri="/";
static char *host=HOST;
static int port=PORT;

static int require_server_auth=0;

static char *REQUEST_TEMPLATE=
   "GET %s HTTP/1.0\r\nUser-Agent:capc\r\nHost: %s:%d\r\n\r\n";

static char *POST_TEMPLATE=
   "POST %s HTTP/1.0\r\nContent-Length: %d\r\nUser-Agent:capc\r\nHost: %s:%d\r\n\r\n";

static int certChecker(sslCertInfo_t *cert, void *arg);

int err_exit(string)
     char *string;
{
  fprintf(stderr,"%s\n",string);
  exit(-1);
}

#if VXWORKS
int _httpsClient(char *arg1)
#elif WINCE
  int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
		      LPWSTR lpCmdLine, int nCmdShow)
#else
  int main(int argc, char **argv)
#endif
{
  struct stat s;
  
  sslConn_t *conn=0;
  sslKeys_t *keys=0;
  WSADATA wsaData;
  SOCKET fd=-1;
  short	cipherSuite=0;
  unsigned char	*ip, *c, *requestBuf=0;
  unsigned char	buf[1024];
  int status;
  int rc, bytes, i, j, err;
  time_t t0, t1;
#if VXWORKS
  int	argc;
  char	**argv;
  parseCmdLineArgs(arg1, &argc, &argv);
#endif /* VXWORKS */
  
#if WINCE
  int	argc;
  char	**argv;
  char	args[256];
  
  WideCharToMultiByte(CP_ACP, 0, lpCmdLine, -1, args, 256, NULL, NULL);
  
  parseCmdLineArgs(args, &argc, &argv);
#endif /* WINCE */

  conn = NULL;

  {
    extern char *optarg;
    int c;
    
    while((c=getopt(argc,argv,"h:p:u:F:C:K:P:A:s"))!=-1){
      switch(c){
      case 'u':
	if(!(uri=strdup(optarg)))
	  err_exit("Out of memory");
	break;
      case 'h':
	if(!(host=strdup(optarg)))
	  err_exit("Out of memory");
	break;
      case 'F':
	if(!(postFile=strdup(optarg)))
	  err_exit("Out of memory");
	break;
      case 'C':
	if(!(certFile=strdup(optarg)))
	  err_exit("Out of memory");
	break;
      case 'K':
	if(!(keyFile=strdup(optarg)))
	  err_exit("Out of memory");
	break;
      case 'P':
	if(!(keyPass=strdup(optarg)))
	  err_exit("Out of memory");
	break;
      case 'A':
	if(!(caFile=strdup(optarg)))
	  err_exit("Out of memory");
	break;
      case 'p':
	if(!(port=atoi(optarg)))
	  err_exit("Bogus port specified");
	break;
      case 's':
	require_server_auth=1;
	break;
      }
    }
  }

  WSAStartup(MAKEWORD(1,1), &wsaData);

  if (matrixSslOpen() < 0) {
    fprintf(stderr, "matrixSslOpen failed, exiting...");
  }

  if (matrixSslReadKeys(&keys, certFile, keyFile, keyPass, caFile) < 0) {
    fprintf(stderr,"Could not load keys %s %s\n",certFile,keyFile);
    goto promptAndExit;
  }

  t0 = time(0);

  if ((fd = socketConnect(host, port, &err)) == INVALID_SOCKET) {
    fprintf(stdout, "Error connecting to server %s:%d\n", host, port);
    matrixSslFreeKeys(keys);
    goto promptAndExit;
  }

  if (sslConnect(&conn, fd, keys, NULL, cipherSuite, certChecker) < 0) {
    socketShutdown(fd);
    fprintf(stderr, "Error connecting to %s:%d\n", host, port);
    goto promptAndExit;
  }
  
  if (conn == NULL) {
    goto promptAndExit;
  }

  if (postFile) {

    if (stat(postFile, &s)) {
      postFile = 0;
    }

    bytes = strlen(POST_TEMPLATE)+
      strlen(uri) + strlen(host) + 16;

    if (!(requestBuf=(char *)malloc(bytes)))
      err_exit("Couldn't allocate request");

    snprintf(requestBuf,bytes,POST_TEMPLATE,
	     uri,postFile ? s.st_size : 0,host,port);

  } else {

    bytes=strlen(REQUEST_TEMPLATE)+
      strlen(uri)+strlen(host)+6;

    if (!(requestBuf=(char *)malloc(bytes)))
      err_exit("Couldn't allocate request");

    snprintf(requestBuf,bytes,REQUEST_TEMPLATE,
	     uri,host,port);
  }

  bytes = strlen(requestBuf);

 writeMore:
  rc = sslWrite(conn, requestBuf, bytes, &status);
  if (rc < 0) {
    fprintf(stdout, "Internal sslWrite error\n");
    socketShutdown(conn->fd);
    sslFreeConnection(&conn);
  } else if (rc == 0) {
    goto writeMore;
  }

  if (postFile) {
    int rleft = s.st_size;
    int fd = open(postFile, O_RDONLY);

    if (fd < 0) 
      err_exit("Could not read post file");

    while (rleft > 0) {
      int wleft = read(fd, buf, sizeof(buf));
      int off = 0;
      if (wleft <= 0) break;
      rleft -= wleft;
      while (wleft > 0) {
	int r=sslWrite(conn,buf+off,wleft,&status);
	if (r < 0 || status == SSLSOCKET_EOF ||
	    status == SSLSOCKET_CLOSE_NOTIFY) {
	  socketShutdown(conn->fd);
	  sslFreeConnection(&conn);
	  goto promptAndExit;
	}
	wleft -= r;
	off += r;
      }
    }
  }

  c = buf;
 readMore:
  if ((rc = sslRead(conn, c, sizeof(buf) - (int)(c - buf), &status)) > 0) {
    c += rc;
    if (c - buf < 4 || memcmp(c - 4, "\r\n\r\n", 4) != 0) {
      goto readMore;
    }
  } else {
    if (rc < 0) {
      fprintf(stdout, "sslRead error.  dropping connection.\n");
    }
    if (rc < 0 || status == SSLSOCKET_EOF ||
	status == SSLSOCKET_CLOSE_NOTIFY) {
      socketShutdown(conn->fd);
      sslFreeConnection(&conn);
    } else {
      goto readMore;
    }
  }

  while ((rc = sslRead(conn, buf, sizeof(buf), &status)) > 0) {
    write(1, buf, rc);
  }

  sslWriteClosureAlert(conn);

  socketShutdown(conn->fd);
  sslFreeConnection(&conn);

  t1 = time(0);
  if (conn && conn->ssl) {
    socketShutdown(conn->fd);
    sslFreeConnection(&conn);
  }

  matrixSslFreeKeys(keys);
  matrixSslClose();
  WSACleanup();

 promptAndExit:
  if (requestBuf)
    free(requestBuf);
  
#if WINCE || VXWORKS
  if (argv) {
    free((void*) argv);
  }
#endif /* WINCE */
  return 0;
}

static int certChecker(sslCertInfo_t *cert, void *arg)
{
  sslCertInfo_t	*next;
  sslKeys_t		*keys;
  next = cert;
  keys = arg;
  while (next->next != NULL) {
    next = next->next;
  }
  if (require_server_auth)
    return next->verified;
  if (next->verified != 1) {
    return SSL_ALLOW_ANON_CONNECTION;
  }
  return next->verified;
}		






