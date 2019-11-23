/* source: procan-cdefs.c */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

/* a function that prints compile time parameters */
/* the set of parameters is only a small subset of the available defines and
   will be extended on demand */


#include "xiosysincludes.h"
#include "mytypes.h"
#include "compat.h"
#include "error.h"

#include "procan.h"

int procan_cdefs(FILE *outfile) {
   /* basic C/system constants */
#ifdef FD_SETSIZE
   fprintf(outfile, "#define FD_SETSIZE %u\n", FD_SETSIZE);
#endif
#ifdef NFDBITS
   fprintf(outfile, "#define NFDBITS %d\n", (int)NFDBITS);
#endif
#ifdef O_RDONLY
   fprintf(outfile, "#define O_RDONLY %u\n", O_RDONLY);
#endif
#ifdef O_WRONLY
   fprintf(outfile, "#define O_WRONLY %u\n", O_WRONLY);
#endif
#ifdef O_RDWR
   fprintf(outfile, "#define O_RDWR %u\n", O_RDWR);
#endif
#ifdef SHUT_RD
   fprintf(outfile, "#define SHUT_RD %u\n", SHUT_RD);
#endif
#ifdef SHUT_WR
   fprintf(outfile, "#define SHUT_WR %u\n", SHUT_WR);
#endif
#ifdef SHUT_RDWR
   fprintf(outfile, "#define SHUT_RDWR %u\n", SHUT_RDWR);
#endif

   /* termios constants */
#ifdef CRDLY
   fprintf(outfile, "#define CRDLY 0%011o\n", CRDLY);
#endif
#ifdef CR0
   fprintf(outfile, "#define CR0 0%011o\n", CR0);
#endif
#ifdef CR1
   fprintf(outfile, "#define CR1 0%011o\n", CR1);
#endif
#ifdef CR2
   fprintf(outfile, "#define CR2 0%011o\n", CR2);
#endif
#ifdef CR3
   fprintf(outfile, "#define CR3 0%011o\n", CR3);
#endif
#ifdef TABDLY
   fprintf(outfile, "#define TABDLY 0%011o\n", TABDLY);
#endif
#ifdef TAB0
   fprintf(outfile, "#define TAB0 0%011o\n", TAB0);
#endif
#ifdef TAB1
   fprintf(outfile, "#define TAB1 0%011o\n", TAB1);
#endif
#ifdef TAB2
   fprintf(outfile, "#define TAB2 0%011o\n", TAB2);
#endif
#ifdef TAB3
   fprintf(outfile, "#define TAB3 0%011o\n", TAB3);
#endif
#ifdef CSIZE
   fprintf(outfile, "#define CSIZE 0%011o\n", CSIZE);
#endif
#ifdef TIOCEXCL
   fprintf(outfile, "#define TIOCEXCL 0x%lx\n", (unsigned long)TIOCEXCL);
#endif

   /* stdio constants */
#ifdef FOPEN_MAX
   fprintf(outfile, "#define FOPEN_MAX %u\n", FOPEN_MAX);
#endif

   /* socket constants */
#ifdef PF_UNIX
   fprintf(outfile, "#define PF_UNIX %d\n", PF_UNIX);
#elif defined(PF_LOCAL)
   fprintf(outfile, "#define PF_LOCAL %d\n", PF_LOCAL);
#endif
#ifdef PF_INET
   fprintf(outfile, "#define PF_INET %d\n", PF_INET);
#endif
#ifdef PF_INET6
   fprintf(outfile, "#define PF_INET6 %d\n", PF_INET6);
#endif
#ifdef PF_APPLETALK
   fprintf(outfile, "#define PF_APPLETALK %d\n", PF_APPLETALK);
#endif
#ifdef PF_PACKET
   fprintf(outfile, "#define PF_PACKET %d\n", PF_PACKET);
#endif
#ifdef SOCK_STREAM
   fprintf(outfile, "#define SOCK_STREAM %d\n", SOCK_STREAM);
#endif
#ifdef SOCK_DGRAM
   fprintf(outfile, "#define SOCK_DGRAM %d\n", SOCK_DGRAM);
#endif
#ifdef SOCK_RAW
   fprintf(outfile, "#define SOCK_RAW %d\n", SOCK_RAW);
#endif
#ifdef SOCK_SEQPACKET
   fprintf(outfile, "#define SOCK_SEQPACKET %d\n", SOCK_SEQPACKET);
#endif
#ifdef SOCK_PACKET
   fprintf(outfile, "#define SOCK_PACKET %d\n", SOCK_PACKET);
#endif
#ifdef IPPROTO_IP
   fprintf(outfile, "#define IPPROTO_IP %d\n", IPPROTO_IP);
#endif
#ifdef IPPROTO_TCP
   fprintf(outfile, "#define IPPROTO_TCP %d\n", IPPROTO_TCP);
#endif
#ifdef IPPROTO_UDP
   fprintf(outfile, "#define IPPROTO_UDP %d\n", IPPROTO_UDP);
#endif
#ifdef IPPROTO_SCTP
   fprintf(outfile, "#define IPPROTO_SCTP %d\n", IPPROTO_SCTP);
#endif
#ifdef IPPROTO_DCCP
   fprintf(outfile, "#define IPPROTO_DCCP %d\n", IPPROTO_DCCP);
#endif
#ifdef SOL_SOCKET
   fprintf(outfile, "#define SOL_SOCKET 0x%x\n", SOL_SOCKET);
#endif
#ifdef SOL_PACKET
   fprintf(outfile, "#define SOL_PACKET 0x%x\n", SOL_PACKET);
#endif
#ifdef SOL_IP
   fprintf(outfile, "#define SOL_IP 0x%x\n", SOL_IP);
#endif
#ifdef SOL_IPV6
   fprintf(outfile, "#define SOL_IPV6 0x%x\n", SOL_IPV6);
#endif
#ifdef SOL_TCP
   fprintf(outfile, "#define SOL_TCP 0x%x\n", SOL_TCP);
#endif
#ifdef SOL_UDP
   fprintf(outfile, "#define SOL_UDP 0x%x\n", SOL_UDP);
#endif
#ifdef SOL_SCTP
   fprintf(outfile, "#define SOL_SCTP 0x%x\n", SOL_SCTP);
#endif
#ifdef SOL_DCCP
   fprintf(outfile, "#define SOL_DCCP 0x%x\n", SOL_DCCP);
#endif
#ifdef SO_REUSEADDR
   fprintf(outfile, "#define SO_REUSEADDR %d\n", SO_REUSEADDR);
#endif

   return 0;
}
