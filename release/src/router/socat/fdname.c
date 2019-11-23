/* source: fdname.c */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

/* the subroutine sockname prints the basic info about the address of a socket
   NOTE: it works on UNIX (kernel) file descriptors, not on libc files! */

#include "config.h"
#include "xioconfig.h"	/* what features are enabled */

#include "sysincludes.h"

#include "mytypes.h"
#include "compat.h"
#include "error.h"
#include "sycls.h"
#include "sysutils.h"

#include "filan.h"


struct sockopt {
   int so;
   char *name;
};


int statname(const char *file, int fd, int filetype, FILE *outfile);
int cdevname(int fd, FILE *outfile);
int sockname(int fd, FILE *outfile);
int unixame(int fd, FILE *outfile);
int tcpname(int fd, FILE *outfile);


int fdname(const char *file, int fd, FILE *outfile, const char *numform) {
   struct stat buf = {0};
   int filetype;
   Debug1("checking file descriptor %u", fd);
   if (fd >= 0) {
      if (Fstat(fd, &buf) < 0) {
	 if (errno == EBADF) {
	    Debug2("fstat(%d): %s", fd, strerror(errno));
	    return -1;
	 } else {
	    Error2("fstat(%d): %s", fd, strerror(errno));
	 }
      }
      filetype = (buf.st_mode&S_IFMT)>>12;
      if (numform != NULL) {
	 fprintf(outfile, numform, fd);
      }
      return statname(file, fd, filetype, outfile);
   } else {
      if (Stat(file, &buf) < 0) {
	 Error2("stat(\"%s\"): %s", file, strerror(errno));
      }
      filetype = (buf.st_mode&S_IFMT)>>12;
      return statname(file, -1, filetype, outfile);
   }
}

#if HAVE_PROC_DIR_FD
static int procgetfdname(int fd, char *filepath, size_t pathsize) {
   static pid_t pid = -1;
   char procpath[PATH_MAX];
   int len;

   /* even if configure has shown that we have /proc, we must check if it
      exists at runtime, because we might be in a chroot environment */
#if HAVE_STAT64
   {
      struct stat64 buf;
      if (Stat64("/proc", &buf) < 0) {
	 return -1;
      }
      if (!S_ISDIR(buf.st_mode)) {
	 return -1;
      }
   }
#else /* !HAVE_STAT64 */
   {
      struct stat buf;
      if (Stat("/proc", &buf) < 0) {
	 return -1;
      }
      if (!S_ISDIR(buf.st_mode)) {
	 return -1;
      }
   }
#endif /* !HAVE_STAT64 */
       
   if (pid < 0)  pid = Getpid();
   snprintf(procpath, sizeof(procpath), "/proc/"F_pid"/fd/%d", pid, fd);
   if ((len = Readlink(procpath, filepath, pathsize-1)) < 0) {
      Error4("readlink(\"%s\", %p, "F_Zu"): %s",
	     procpath, filepath, pathsize, strerror(errno));
      return -1;
   }
   filepath[len] = '\0';
   return 0;
}
#endif /* HAVE_PROC_DIR_FD */
   
int statname(const char *file, int fd, int filetype, FILE *outfile) {
   char filepath[PATH_MAX];

   filepath[0] = '\0';
#if HAVE_PROC_DIR_FD
   if (fd >= 0) {
      procgetfdname(fd, filepath, sizeof(filepath));
      if (filepath[0] == '/') {
	 file = filepath;
      }
   }
#endif /*  HAVE_PROC_DIR_FD */
   /* now see for type specific infos */
   switch (filetype) {
   case (S_IFIFO>>12):	/* 1, FIFO */
      fputs("pipe", outfile);
      if (file) fprintf(outfile, " %s", file);
      break;
   case (S_IFCHR>>12):	/* 2, character device */
      if (cdevname(fd, outfile) == 0) {
	 if (file) fprintf(outfile, " %s", file);
      }
      break;
   case (S_IFDIR>>12):	/* 4, directory */
      fputs("dir", outfile);
      if (file) fprintf(outfile, " %s", file);
      break;
   case (S_IFBLK>>12):	/* 6, block device */
      fputs("blkdev", outfile);
      if (file) fprintf(outfile, " %s", file);
      break;
   case (S_IFREG>>12):	/* 8, regular file */
      fputs("file", outfile);
      if (file) fprintf(outfile, " %s", file);
      break;
   case (S_IFLNK>>12):	/* 10, symbolic link */
      fputs("link", outfile);
      if (file) fprintf(outfile, " %s", file);
      break;
   case (S_IFSOCK>>12): /* 12, socket */
#if _WITH_SOCKET
      if (fd >= 0) {
	 sockname(fd, outfile);
      } else if (file) {
	 fprintf(outfile, "socket %s", file);
      } else {
	 fputs("socket", outfile);
      }
#else
      Error("SOCKET support not compiled in");
      return -1;
#endif /* !_WITH_SOCKET */
      break;
   }
   /* ioctl() */
   fputc('\n', outfile);

   return 0;
}


/* character device analysis */
/* return -1 on error, 0 if no name was found, or 1 if it printed ttyname */
int cdevname(int fd, FILE *outfile) {
   int ret;

   if ((ret = Isatty(fd)) < 0) {
      Error2("isatty(%d): %s", fd, strerror(errno));
      return -1;
   }
   if (ret > 0) {
      char *name;

      fputs("tty", outfile);
      if ((name = Ttyname(fd)) != NULL) {
	 fputc(' ', outfile);
	 fputs(name, outfile);
	 return 1;
      }
   } else {
      fputs("chrdev", outfile);
   }
   return 0;
}


#if _WITH_SOCKET
int sockname(int fd, FILE *outfile) {
#define FDNAME_OPTLEN 256
#define FDNAME_NAMELEN 256
   socklen_t optlen;
   int opttype;
#ifdef SO_ACCEPTCONN
   int optacceptconn;
#endif
   int result /*0, i*/;
   char namebuff[FDNAME_NAMELEN];
   char peerbuff[FDNAME_NAMELEN];
   /* in Linux these optcodes are 'enum', but on AIX they are bits! */
   union sockaddr_union sockname, peername;	/* the longest I know of */
   socklen_t namelen;
#if 0 && defined(SIOCGIFNAME)
   /*Linux struct ifreq ifc = {{{ 0 }}};*/
   struct ifreq ifc = {{ 0 }};
#endif

   optlen = FDNAME_OPTLEN;

   Getsockopt(fd, SOL_SOCKET, SO_TYPE,       &opttype,       &optlen);
#ifdef SO_ACCEPTCONN
   Getsockopt(fd, SOL_SOCKET, SO_ACCEPTCONN, &optacceptconn, &optlen);
#endif

   namelen = sizeof(sockname);
   result = Getsockname(fd, &sockname.soa, &namelen);
   if (result < 0) {
      Error2("getsockname(%d): %s", fd, strerror(errno));
      return -1;
   }

   namelen = sizeof(peername);
   result = Getpeername(fd, (struct sockaddr *)&peername, &namelen);
   if (result < 0) {
      Error2("getpeername(%d): %s", fd, strerror(errno));
   }

   switch (sockname.soa.sa_family) {
#if WITH_UNIX
   case AF_UNIX:
      fprintf(outfile, "unix%s%s %s",
	      opttype==SOCK_DGRAM?"datagram":"",
#ifdef SO_ACCEPTCONN
	      optacceptconn?"(listening)":
#endif
	      "",
	      sockaddr_unix_info(&sockname.un, namelen,
				 namebuff, sizeof(namebuff)));
      break;
#endif
#if WITH_IP4
   case AF_INET:
      switch (opttype) {
#if WITH_TCP
      case SOCK_STREAM:
	 fprintf(outfile, "tcp%s %s %s",
#ifdef SO_ACCEPTCONN
		 optacceptconn?"(listening)":
#endif
		 "",
		 sockaddr_inet4_info(&sockname.ip4,
				     namebuff, sizeof(namebuff)),
		 sockaddr_inet4_info(&peername.ip4,
				     peerbuff, sizeof(peerbuff)));
	 break;
#endif
#if WITH_UDP
      case SOCK_DGRAM:
	 fprintf(outfile, "udp%s %s %s",
#ifdef SO_ACCEPTCONN
		 optacceptconn?"(listening)":
#endif
		 "",
		 sockaddr_inet4_info(&sockname.ip4,
				     namebuff, sizeof(namebuff)),
		 sockaddr_inet4_info(&peername.ip4,
				     peerbuff, sizeof(peerbuff)));
	 break;
#endif
      default:
	 fprintf(outfile, "ip %s",
		 sockaddr_inet4_info(&sockname.ip4,
				     namebuff, sizeof(namebuff)));
	 break;
      }
      break;
#endif /* WITH_IP4 */

#if WITH_IP6
   case AF_INET6:
      switch (opttype) {
#if WITH_TCP
      case SOCK_STREAM:
	 fprintf(outfile, "tcp6%s %s %s",
#ifdef SO_ACCEPTCONN
		 optacceptconn?"(listening)":
#endif
		 "",
		 sockaddr_inet6_info(&sockname.ip6,
				     namebuff, sizeof(namebuff)),
		 sockaddr_inet6_info(&peername.ip6,
				     peerbuff, sizeof(peerbuff)));
	 break;
#endif
#if WITH_UDP
      case SOCK_DGRAM:
	 fprintf(outfile, "udp6%s %s %s",
#ifdef SO_ACCEPTCONN
		 optacceptconn?"(listening)":
#endif
		 "",
		 sockaddr_inet6_info(&sockname.ip6,
				     namebuff, sizeof(namebuff)),
		 sockaddr_inet6_info(&peername.ip6,
				     peerbuff, sizeof(peerbuff)));
	 break;
#endif
      default:
	 fprintf(outfile, "ip6 %s",
		 sockaddr_inet6_info(&sockname.ip6,
				     namebuff, sizeof(namebuff)));
	 break;
      }
#endif /* WITH_IP6 */
   default:
      fputs("socket", outfile);
   }

   return result;
#undef FDNAME_OPTLEN
#undef FDNAME_NAMELEN
}
#endif /* _WITH_SOCKET */




