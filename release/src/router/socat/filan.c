/* source: filan.c */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

/* the subroutine filan makes a "FILe descriptor ANalysis". It checks the
   type of file descriptor and tries to retrieve as much info about it as 
   possible without modifying its state.
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

static int filan_streams_analyze(int fd, FILE *outfile);

/* dirty workaround so we dont get an error on AIX when being linked with
   libwrap */
int allow_severity, deny_severity;

/* global variables for configuring filan */
bool filan_followsymlinks;
bool filan_rawoutput;


int sockoptan(int fd, const struct sockopt *optname, int socklay, FILE *outfile);
int tcpan(int fd, FILE *outfile);
const char *getfiletypestring(int st_mode);

static int printtime(FILE *outfile, time_t time);

static int headprinted;

/* analyse a file system entry, referred by file name */
int filan_file(const char *filename, FILE *outfile) {
   int fd = -1;
   int result;
#if HAVE_STAT64
   struct stat64 buf = {0};
#else
   struct stat buf = {0};
#endif /* !HAVE_STAT64 */

   if (filan_followsymlinks) {
#if HAVE_STAT64
      result = Stat64(filename, &buf);
#else
      result = Stat(filename, &buf);
#endif /* !HAVE_STAT64 */
      if (result < 0) {
	 Warn3("stat(\"%s\", %p): %s", filename, &buf, strerror(errno));
      }
   } else {
#if HAVE_STAT64
      result = Lstat64(filename, &buf);
#else
      result = Lstat(filename, &buf);
#endif /* !HAVE_STAT64 */
      if (result < 0) {
	 Warn3("lstat(\"%s\", %p): %s", filename, &buf, strerror(errno));
      }
   }
   switch (buf.st_mode&S_IFMT) {
#ifdef S_IFSOCK
   case S_IFSOCK: /* probably, it's useless to make a socket and describe it */
      break;
#endif /* S_IFSOCK */
   default:
      if ((fd =
	   Open(filename,  O_RDONLY|O_NOCTTY|O_NONBLOCK
#ifdef O_LARGEFILE
		|O_LARGEFILE
#endif
		, 0700))
	  < 0) {
	 Warn2("open(\"%s\", O_RDONLY|O_NOCTTY|O_NONBLOCK|O_LARGEFILE, 0700): %s",
	       filename, strerror(errno));
      }
   }
     
   result = filan_stat(&buf, fd, -1, outfile);
   fputc('\n', outfile);
   return result;
}

/* analyze a file descriptor */
int filan_fd(int fd, FILE *outfile) {
#if HAVE_STAT64
   struct stat64 buf = {0};
#else
   struct stat buf = {0};
#endif /* !HAVE_STAT64 */
   int result;

   Debug1("checking file descriptor %u", fd);
#if HAVE_STAT64
   result = Fstat64(fd, &buf);
#else
   result = Fstat(fd, &buf);
#endif /* !HAVE_STAT64 */
   if (result < 0) {
      if (errno == EBADF) {
         Debug2("fstat(%d): %s", fd, strerror(errno));
      } else {
         Warn2("fstat(%d): %s", fd, strerror(errno));
      }
      return -1;
   }
   Debug2("fd %d is a %s", fd, getfiletypestring(buf.st_mode));

   result = filan_stat(&buf, fd, fd, outfile);

   if (result >= 0) {
      /* even more dynamic info */
      { /* see if data is available */
	 struct pollfd ufds;
	 ufds.fd = fd;
	 ufds.events = POLLIN|POLLPRI|POLLOUT
#ifdef POLLRDNORM
	    |POLLRDNORM
#endif
#ifdef POLLRDBAND
	    |POLLRDBAND
#endif
	    |POLLWRNORM
#ifdef POLLWRBAND
	    |POLLWRBAND
#endif
#ifdef POLLMSG
	    |POLLMSG
#endif
	    ;
	 if (Poll(&ufds, 1, 0) < 0) {
	    Warn4("poll({%d, %hd, %hd}, 1, 0): %s",
		   ufds.fd, ufds.events, ufds.revents, strerror(errno));
	 } else {
	    fputs("poll: ", outfile);
	    if (ufds.revents & POLLIN)   fputs("IN,", outfile);
	    if (ufds.revents & POLLPRI)  fputs("PRI,", outfile);
	    if (ufds.revents & POLLOUT)  fputs("OUT,", outfile);
	    if (ufds.revents & POLLERR)  fputs("ERR,", outfile);
	    if (ufds.revents & POLLNVAL) fputs("NVAL,", outfile);
#ifdef FIONREAD
	    if (ufds.revents & POLLIN) {
	       size_t sizet;
	       if ((result = Ioctl(fd, FIONREAD, &sizet) >= 0)) {
		  fprintf (outfile, "; FIONREAD="F_Zu, sizet);
	       }
	    }
#endif /* defined(FIONREAD) */
#if _WITH_SOCKET && defined(MSG_DONTWAIT)
	    if ((ufds.revents & POLLIN) && isasocket(fd)) {
	       char _peername[SOCKADDR_MAX];
	       struct sockaddr *pa = (struct sockaddr *)_peername;
	       struct msghdr msgh = {0};
	       char peekbuff[1];	/* [0] fails with some compilers */
#if HAVE_STRUCT_IOVEC
	       struct iovec iovec;
#endif
	       char ctrlbuff[5120];
	       ssize_t bytes;

	       fputs("; ", outfile);
	       msgh.msg_name = pa;
	       msgh.msg_namelen = sizeof(*pa);
#if HAVE_STRUCT_IOVEC
	       iovec.iov_base = peekbuff;
	       iovec.iov_len  = sizeof(peekbuff);
	       msgh.msg_iov = &iovec;
	       msgh.msg_iovlen = 1;
#endif
#if HAVE_STRUCT_MSGHDR_MSGCONTROL
	       msgh.msg_control = ctrlbuff;
#endif
#if HAVE_STRUCT_MSGHDR_MSGCONTROLLEN
	       msgh.msg_controllen = sizeof(ctrlbuff);
#endif
#if HAVE_STRUCT_MSGHDR_MSGFLAGS
	       msgh.msg_flags = 0;
#endif
	       if ((bytes = Recvmsg(fd, &msgh, MSG_PEEK|MSG_DONTWAIT)) < 0) {
		  Warn1("recvmsg(): %s", strerror(errno));
	       } else {
		  fprintf(outfile, "recvmsg="F_Zd", ", bytes);
	       }
	    }
#endif /* _WITH_SOCKET && defined(MSG_DONTWAIT) */
	 }	 
      }
   }
   fputc('\n', outfile);
   return 0;
}


int filan_stat(
#if HAVE_STAT64
	       struct stat64 *buf
#else
	       struct stat *buf
#endif /* !HAVE_STAT64 */
	       , int statfd, int dynfd, FILE *outfile) {
   char stdevstr[8];

   /* print header */
   if (!headprinted) {
      if (filan_rawoutput) {
	 fputs("  FD  type\tdevice\tinode\tmode\tlinks\tuid\tgid"
#if HAVE_ST_RDEV
	       "\trdev"
#endif
	       "\tsize"
#if HAVE_ST_BLKSIZE
	       "\tblksize"
#endif
#if HAVE_ST_BLOCKS
	       "\tblocks"
#endif
	       "\tatime\t\tmtime\t\tctime\t\tcloexec\tflags"
#if defined(F_GETOWN)
	       "\tsigown"
#endif
	       , outfile);
      } else /* !rawoutput */ {
	 fputs("  FD  type\tdevice\tinode\tmode\tlinks\tuid\tgid"
#if HAVE_ST_RDEV
	       "\trdev"
#endif
	       "\tsize"
#if HAVE_ST_BLKSIZE
	       "\tblksize"
#endif
#if HAVE_ST_BLOCKS
	       "\tblocks"
#endif
	       "\tatime\t\t\t\tmtime\t\t\t\tctime\t\t\t\tcloexec\tflags"
#if defined(F_GETOWN)
	       "\tsigown"
#endif
	       , outfile);

      } /* endif !rawoutput */

#if defined(F_GETSIG)
      fputs("\tsigio", outfile);
#endif /* defined(F_GETSIG) */
      fputc('\n', outfile);
      headprinted = 1;
   }
   if (filan_rawoutput) {
      snprintf(stdevstr, 8, F_dev, buf->st_dev);
   } else {
      snprintf(stdevstr, 8, "%hu,%hu", (unsigned short)(buf->st_dev>>8), (unsigned short)(buf->st_dev&0xff));
   }
   fprintf(outfile, "%4d: %s\t%s\t"
#if HAVE_STAT64
	   F_st64_ino
#else
	   F_st_ino
#endif /* HAVE_STAT64 */
	   "\t"F_mode"\t"F_st_nlink"\t"F_uid"\t"F_gid
#if HAVE_ST_RDEV
	   "\t%hu,%hu"
#endif
	   "\t"
#if HAVE_STAT64
	   F_st64_size
#else
	   F_st_size
#endif /* HAVE_STAT64 */
#if HAVE_ST_BLKSIZE
	   "\t"F_st_blksize
#endif
#if HAVE_ST_BLOCKS
#if HAVE_STAT64
	   "\t"F_st64_blocks
#else
	   "\t"F_st_blocks
#endif /* HAVE_STAT64 */
#endif
	   ,
	   (dynfd>=0?dynfd:statfd), getfiletypestring(buf->st_mode),
	   stdevstr,
	   buf->st_ino,
	   buf->st_mode, buf->st_nlink, buf->st_uid,
	   buf->st_gid,
#if HAVE_ST_RDEV
	   (unsigned short)(buf->st_rdev>>8), (unsigned short)(buf->st_rdev&0xff),
#endif
	   buf->st_size
#if HAVE_ST_BLKSIZE
	   , buf->st_blksize
#endif
#if HAVE_ST_BLOCKS
	   , buf->st_blocks	/* on Linux, this applies to stat and stat64 */
#endif
	   );

      printtime(outfile, buf->st_atime);
      printtime(outfile, buf->st_mtime);
      printtime(outfile, buf->st_ctime);

#if 0
   {
      fputc('\t', outfile);
      time = asctime(localtime(&buf->st_mtime));
      if (strchr(time, '\n'))  *strchr(time, '\n') = '\0';
      fputs(time, outfile);

      fputc('\t', outfile);
      time = asctime(localtime(&buf->st_ctime));
      if (strchr(time, '\n'))  *strchr(time, '\n') = '\0';
      fputs(time, outfile);
   }
#endif

   /* here comes dynamic info - it is only meaningful with preexisting FDs */
  if (dynfd >= 0) {	/*!indent */
   int cloexec, flags;
#if defined(F_GETOWN)
   int sigown;
#endif
#if defined(F_GETSIG)
   int sigio;
#endif /* defined(F_GETSIG) */

   cloexec = Fcntl(dynfd, F_GETFD);
   flags   = Fcntl(dynfd, F_GETFL);
#if defined(F_GETOWN)
   sigown  = Fcntl(dynfd, F_GETOWN);
#endif
#if defined(F_GETSIG)
   sigio   = Fcntl(dynfd, F_GETSIG);
#endif /* defined(F_GETSIG) */
   fprintf(outfile, "\t%d\tx%06x", cloexec, flags);
#if defined(F_GETOWN)
   fprintf(outfile, "\t%d", sigown);
#endif
#if defined(F_GETSIG)
   fprintf(outfile, "\t%d", sigio);
#endif /* defined(F_GETSIG) */
  } else {
     fputs("\t\t"
#if defined(F_GETOWN)
	   "\t"
#endif
#if defined(F_GETSIG)
	   "\t"
#endif /* defined(F_GETSIG) */
	   , outfile);
  }

   /* ever heard of POSIX streams? here we handle these */
   filan_streams_analyze(statfd, outfile);

   /* now see for type specific infos */
  if (statfd >= 0) { /*!indent */
   switch (buf->st_mode&S_IFMT) {
   case (S_IFIFO):	/* 1, FIFO */
      break;
   case (S_IFCHR):	/* 2, character device */
      cdevan(statfd, outfile);
      break;
   case (S_IFDIR):	/* 4, directory */
      break;
   case (S_IFBLK):	/* 6, block device */
      break;
   case (S_IFREG):	/* 8, regular file */
      break;
   case (S_IFLNK):	/* 10, symbolic link */
      break;
#ifdef S_IFSOCK
   case (S_IFSOCK): /* 12, socket */
#if _WITH_SOCKET
      sockan(statfd, outfile);
#else
      Warn("SOCKET support not compiled in");
      return -1;
#endif /* !_WITH_SOCKET */
      break;
#endif /* S_IFSOCK */
   }
  }
   /* ioctl() */
   return 0;
}


#if LATER
int fdinfo(int fd) {
   int result;

   result = Fcntl(fd, F_GETFD);
   fcntl(fd, F_GETFL, );
   fcntl(fd, F_GETLK, );
#ifdef F_GETOWN
   fcntl(fd, F_GETOWN, );
#endif
#ifdef F_GETSIG
   fcntl(fd, F_GETSIG, );
#endif
}


int devinfo(int fd) {
  ioctl();
}
#endif


/* returns 0 on success (not a stream descriptor, or no module)
   returns <0 on failure */
static int filan_streams_analyze(int fd, FILE *outfile) {
#ifdef I_LIST
#  define SL_NMODS 8	/* max number of module names we can store */
   struct str_list modnames;
   int i;

   if (!isastream(fd)) {
      fprintf(outfile, "\t(no STREAMS modules)");
      return 0;
   }
#if 0	/* uncomment for debugging */
   fprintf(outfile, "\tfind=%d", ioctl(fd, I_FIND, "ldterm"));
#endif
   modnames.sl_nmods = ioctl(fd, I_LIST, 0);
   if (modnames.sl_nmods < 0) {
      fprintf(stderr, "ioctl(%d, I_LIST, 0): %s\n", fd, strerror(errno));
      return -1;
   }
   modnames.sl_modlist = Malloc(modnames.sl_nmods*(sizeof(struct str_mlist)));
   if (modnames.sl_modlist == NULL) {
      fprintf(stderr, "out of memory\n");
      return -1;
   }
   if (ioctl(fd, I_LIST, &modnames) < 0) {
      fprintf(stderr, "ioctl(%d, I_LIST, %p): %s\n",
	      fd, &modnames, strerror(errno));
      free(modnames.sl_modlist);
      return -1;
   }
   fprintf(outfile, "\tSTREAMS: ");
   for (i = 0; i < modnames.sl_nmods; ++i) {
      fprintf(outfile, "\"%s\"", modnames.sl_modlist[i].l_name);
      if (i+1 < modnames.sl_nmods)  fputc(',', outfile);
   }
   free(modnames.sl_modlist);
#endif /* defined(I_LIST) */
   return 0;
}


/* character device analysis */
int cdevan(int fd, FILE *outfile) {
   int ret;

#if _WITH_TERMIOS
   if ((ret = Isatty(fd)) < 0) {
      Warn2("isatty(%d): %s", fd, strerror(errno));
      return -1;
   }
   if (ret > 0) {
      struct termios termarg;
      char *name;
      int i;

      if ((name = Ttyname(fd)) == NULL) {
	 /*Warn2("ttyname(%d): %s", fd, strerror(errno));*/
	 fputs("\tNULL", outfile);
      } else {
	 fprintf(outfile, "\t%s", name);
      }
      if (Tcgetattr(fd, &termarg) < 0) {
	 Warn3("tcgetattr(%d, %p): %s", fd, &termarg, strerror(errno));
	 return -1;
      }
      fprintf(outfile, " \tIFLAGS=%08x OFLAGS=%08x CFLAGS=%08x LFLAGS=%08x",
	      (unsigned int)termarg.c_iflag,
	      (unsigned int)termarg.c_oflag,
	      (unsigned int)termarg.c_cflag,
	      (unsigned int)termarg.c_lflag);

      /* and the control characters */
      if (filan_rawoutput) {
	 for (i=0; i<NCCS; ++i) {
	    fprintf(outfile, " cc[%d]=%d", i, termarg.c_cc[i]);
	 }
      } else {
	 for (i=0; i<NCCS; ++i) {
	    int ch;
	    unsigned char s[4];
	    ch = termarg.c_cc[i];
	    if (isprint(ch)) {
	       s[0] = ch; s[1]= '\0';
	    } else if (ch < ' ') {
	       s[0] = '^'; s[1] = ch+'@'; s[2] = '\0';
	    } else {
	       s[0] = 'x';
	       s[1] = (ch>>4)>=10?(ch>>4)-10+'A':(ch>>4)+'0';
	       s[2] = (ch&0x0f)>=10?(ch&0x0f)-10+'A':(ch&0x0f)+'0';
	       s[3] = '\0';
	    }
	    fprintf(outfile, " cc[%d]=%s", i, s);
	 }
      }
   }
#endif /* _WITH_TERMIOS */
   return 0;
}


#if _WITH_SOCKET
int sockan(int fd, FILE *outfile) {
#define FILAN_OPTLEN 256
#define FILAN_NAMELEN 256
   socklen_t optlen;
   int result /*0, i*/;
   static const char *socktypes[] = {
      "undef", "STREAM", "DGRAM", "RAW", "RDM",
      "SEQPACKET", "undef", "undef", "undef", "undef", 
      "PACKET", "undef" } ;
   char nambuff[FILAN_NAMELEN];
   /* in Linux these optcodes are 'enum', but on AIX they are bits! */
   static const struct sockopt sockopts[] = {
      {SO_DEBUG, "DEBUG"},
      {SO_REUSEADDR, "REUSEADDR"},
      {SO_TYPE, "TYPE"},
      {SO_ERROR, "ERROR"},
#ifdef SO_PROTOTYPE
      {SO_PROTOTYPE, "PROTOTYPE"},
#endif
      {SO_DONTROUTE, "DONTROUTE"},
      {SO_BROADCAST, "BROADCAST"},
      {SO_SNDBUF, "SNDBUF"},
      {SO_RCVBUF, "RCVBUF"},
      {SO_KEEPALIVE, "KEEPALIVE"},
      {SO_OOBINLINE, "OOBINLINE"},
#ifdef SO_NO_CHECK
      {SO_NO_CHECK, "NO_CHECK"},
#endif
#ifdef SO_PRIORITY
      {SO_PRIORITY, "PRIORITY"},
#endif
      {SO_LINGER, "LINGER"},
#ifdef SO_BSDCOMPAT
      {SO_BSDCOMPAT, "BSDCOMPAT"},
#endif
#ifdef SO_REUSEPORT
      {SO_REUSEPORT, "REUSEPORT"},
#endif /* defined(SO_REUSEPORT) */
#ifdef SO_PASSCRED
      {SO_PASSCRED, "PASSCRED"},
#endif
#ifdef SO_PEERCRED
      {SO_PEERCRED, "PEERCRED"},
#endif
#ifdef SO_RCVLOWAT
      {SO_RCVLOWAT, "RCVLOWAT"},
#endif
#ifdef SO_SNDLOWAT
      {SO_SNDLOWAT, "SNDLOWAT"},
#endif
#ifdef SO_RCVTIMEO
      {SO_RCVTIMEO, "RCVTIMEO"},
#endif
#ifdef SO_SNDTIMEO
      {SO_SNDTIMEO, "SNDTIMEO"},
#endif
#ifdef SO_SECURITY_AUTHENTICATION
      {SO_SECURITY_AUTHENTICATION, "SECURITY_AUTHENTICATION"},
#endif
#ifdef SO_SECURITY_ENCRYPTION_TRANSPORT
      {SO_SECURITY_ENCRYPTION_TRANSPORT, "SECURITY_ENCRYPTION_TRANSPORT"},
#endif
#ifdef SO_SECURITY_ENCRYPTION_NETWORK
      {SO_SECURITY_ENCRYPTION_NETWORK, "SECURITY_ENCRYPTION_NETWORK"},
#endif
#ifdef SO_BINDTODEVICE
      {SO_BINDTODEVICE, "BINDTODEVICE"},
#endif
#ifdef SO_ATTACH_FILTER
      {SO_ATTACH_FILTER, "ATTACH_FILTER"},
#endif
#ifdef SO_DETACH_FILTER
      {SO_DETACH_FILTER, "DETACH_FILTER"},
#endif
      {0, NULL} } ;
   union {
      char c[FILAN_OPTLEN];
      int  i[FILAN_OPTLEN/sizeof(int)];
   } optval;
   const struct sockopt *optname;
   union sockaddr_union sockname, peername;	/* the longest I know of */
   socklen_t namelen;
#if 0 && defined(SIOCGIFNAME)
   /*Linux struct ifreq ifc = {{{ 0 }}};*/
   struct ifreq ifc = {{ 0 }};
#endif

   optlen = FILAN_OPTLEN;
   result = Getsockopt(fd, SOL_SOCKET, SO_TYPE, optval.c, &optlen);
   if (result < 0) {
      Debug4("getsockopt(%d, SOL_SOCKET, SO_TYPE, %p, {"F_socklen"}): %s",
	     fd, optval.c, optlen, strerror(errno));
   } else {
      Debug3("fd %d: socket of type %d (\"%s\")", fd, *optval.i,
	  socktypes[*optval.i]);
   }

   optname = sockopts; while (optname->so) {
      optlen = FILAN_OPTLEN;
      result =
	 Getsockopt(fd, SOL_SOCKET, optname->so, (void *)optval.c, &optlen);
      if (result < 0) {
	 Debug5("getsockopt(%d, SOL_SOCKET, %d, %p, {"F_socklen"}): %s",
		fd, optname->so, optval.c, optlen, strerror(errno));
	 fputc('\t', outfile);
      } else if (optlen == sizeof(int)) {
	 Debug2("getsockopt(,,, {%d}, %d)",
	     *optval.i, optlen);
	 /*Info2("%s: %d", optname->name, optval.i);*/
	 fprintf(outfile, "%s=%d\t", optname->name, *optval.i);
      } else {
	 Debug3("getsockopt(,,, {%d,%d}, %d)",
	     optval.i[0], optval.i[1], optlen);
	 fprintf(outfile, "%s={%d,%d}\t", optname->name,
		 optval.i[0], optval.i[1]);
      }
      ++optname;
   }

   namelen = sizeof(sockname);
   result = Getsockname(fd, (struct sockaddr *)&sockname, &namelen);
   if (result < 0) {
      putc('\n', outfile);
      Warn2("getsockname(%d): %s", fd, strerror(errno));
      return -1;
   }
   fputc('\t', outfile);
   fputs(sockaddr_info((struct sockaddr *)&sockname, namelen, nambuff, sizeof(nambuff)),
	 outfile);

   namelen = sizeof(peername);
   result = Getpeername(fd, (struct sockaddr *)&peername, &namelen);
   if (result < 0) {
      putc('\n', outfile);
      Warn2("getpeername(%d): %s", fd, strerror(errno));
   } else {
      /* only valid if getpeername() succeeded */
      fputs(" <-> ", outfile);
      fprintf(outfile, "%s\t",
	      sockaddr_info((struct sockaddr *)&peername, namelen,
			    nambuff, sizeof(nambuff)));
   }

#if 0 && defined(SIOCGIFNAME)
   if ((result = Ioctl(fd, SIOCGIFNAME, &ifc)) < 0) {
      Warn3("ioctl(%d, SIOCGIFNAME, %p): %s", fd, &ifc, strerror(errno));
   } else {
      fprintf(outfile, "IFNAME=\"%s\"\t", ifc.ifr_name);
   }
#endif /* SIOCGIFNAME */

   switch (((struct sockaddr *)&sockname)->sa_family) {
#if WITH_UNIX
   case AF_UNIX:
      /* no options for unix domain sockets known yet -> no unixan() */
      result = 0;
      break;
#endif
#if WITH_IP4
   case AF_INET:
      result = ipan(fd, outfile);
      break;
#endif
#if WITH_IP6
   case AF_INET6:
      result = ipan(fd, outfile);
      result |= ip6an(fd, outfile);
      break;
#endif
   default:
      fputs("**** NO FURTHER ANALYSIS FOR THIS SOCKET TYPE IMPLEMENTED", outfile);
      result = 0;
   }
   return result;
#undef FILAN_OPTLEN
#undef FILAN_NAMELEN
}
#endif /* _WITH_SOCKET */


#if WITH_IP4 || WITH_IP6
/* prints the option values for the IP protocol and the IP based protocols */
/* no distinction between IP4 and IP6 yet */
int ipan(int fd, FILE *outfile) {
   /* in Linux these optcodes are 'enum', but on AIX they are bits! */
   static const struct sockopt ipopts[] = {
      {IP_TOS,          "IP_TOS"},
      {IP_TTL,          "IP_TTL"},
#ifdef IP_HDRINCL
      {IP_HDRINCL,      "IP_HDRINCL"},
#endif
#ifdef IP_OPTIONS
      {IP_OPTIONS,      "IP_OPTIONS"},
#endif
#ifdef IP_ROUTER_ALERT
      {IP_ROUTER_ALERT, "IP_ROUTER_ALERT"},
#endif
#ifdef IP_RECVOPTS
      {IP_RECVOPTS,     "IP_RECVOPTS"},
#endif
#ifdef IP_RETOPTS
      {IP_RETOPTS,      "IP_RETOPTS"},
#endif
#ifdef IP_PKTINFO
      {IP_PKTINFO,      "IP_PKTINFO"},
#endif
#ifdef IP_PKTOPTIONS
      {IP_PKTOPTIONS,   "IP_PKTOPTIONS"},
#endif
#ifdef IP_MTU_DISCOVER
      {IP_MTU_DISCOVER, "IP_MTU_DISCOVER"},
#endif
#ifdef IP_RECVERR
      {IP_RECVERR,      "IP_RECVERR"},
#endif
#ifdef IP_RECVTTL
      {IP_RECVTTL,      "IP_RECVTTL"},
#endif
#ifdef IP_RECVTOS
      {IP_RECVTOS,      "IP_RECVTOS"},
#endif
#ifdef IP_MTU
      {IP_MTU,          "IP_MTU"},
#endif
#ifdef IP_FREEBIND
      {IP_FREEBIND,       "IP_FREEBIND"},
#endif
#ifdef IP_MULTICAST_TTL
      {IP_MULTICAST_TTL,  "IP_MULTICAST_TTL"},
#endif
#ifdef IP_MULTICAST_LOOP
      {IP_MULTICAST_LOOP, "IP_MULTICAST_LOOP"},
#endif
      {0, NULL} } ;
   const struct sockopt *optname;
   int opttype;
   socklen_t optlen = sizeof(opttype);
   
   optname = ipopts; while (optname->so) {
      sockoptan(fd, optname, SOL_IP, outfile);
      ++optname;
   }
   /* want to pass the fd to the next layer protocol. dont know how to get the
      protocol number from the fd? use TYPE to identify TCP. */
   if (Getsockopt(fd, SOL_SOCKET, SO_TYPE, &opttype, &optlen) >= 0) {
      switch (opttype) {
#if WITH_TCP
      case SOCK_STREAM: tcpan(fd, outfile); break;
#endif
      }
   }
   return 0;
}
#endif /* WITH_IP */


#if WITH_IP6
/* prints the option values for the IPv6 protocol */
int ip6an(int fd, FILE *outfile) {
   static const struct sockopt ip6opts[] = {
#ifdef IPV6_V6ONLY
      {IPV6_V6ONLY,         "IPV6_V6ONLY"},
#endif
      {0, NULL} } ;
   const struct sockopt *optname;
   
   optname = ip6opts; while (optname->so) {
      sockoptan(fd, optname, SOL_IPV6, outfile);
      ++optname;
   }
   return 0;
}
#endif /* WITH_IP6 */


#if WITH_TCP
int tcpan(int fd, FILE *outfile) {
   static const struct sockopt tcpopts[] = {
#ifdef TCP_NODELAY
      { TCP_NODELAY, "TCP_NODELAY" },
#endif
#ifdef TCP_MAXSEG
      { TCP_MAXSEG,  "TCP_MAXSEG" },
#endif
#ifdef TCP_STDURG
      { TCP_STDURG,  "TCP_STDURG" },
#endif
#ifdef TCP_RFC1323
      { TCP_RFC1323, "TCP_RFC1323" },
#endif
#ifdef TCP_CORK
      { TCP_CORK,    "TCP_CORK" },
#endif
#ifdef TCP_KEEPIDLE
      { TCP_KEEPIDLE, "TCP_KEEPIDLE" },
#endif
#ifdef TCP_KEEPINTVL
      { TCP_KEEPINTVL, "TCP_KEEPINTVL" },
#endif
#ifdef TCP_KEEPCNT
      { TCP_KEEPCNT, "TCP_KEEPCNT" },
#endif
#ifdef TCP_SYNCNT
      { TCP_SYNCNT, "TCP_SYNCNT" },
#endif
#ifdef TCP_LINGER2
      { TCP_LINGER2, "TCP_LINGER2" },
#endif
#ifdef TCP_DEFER_ACCEPT
      { TCP_DEFER_ACCEPT, "TCP_ACCEPT" },
#endif
#ifdef TCP_WINDOW_CLAMP
      { TCP_WINDOW_CLAMP, "TCP_WINDOW_CLAMP" },
#endif
#ifdef TCP_INFO
      { TCP_INFO, "TCP_INFO" },
#endif
#ifdef TCP_QUICKACK
      { TCP_QUICKACK, "TCP_QUICKACK" },
#endif
#ifdef TCP_MD5SIG
      { TCP_MD5SIG, "TCP_MD5SIG" },
#endif
#ifdef TCP_NOOPT
      { TCP_NOOPT, "TCP_NOOPT" },
#endif
#ifdef TCP_NOPUSH
      { TCP_NOPUSH, "TCP_NOPUSH" },
#endif
#ifdef TCP_SACK_DISABLE
      { TCP_SACK_DISABLE, "TCP_SACK_DISABLE" },
#endif
#ifdef TCP_SIGNATURE_ENABLE
      { TCP_SIGNATURE_ENABLE, "TCP_SIGNATURE_ENABLE" },
#endif
#ifdef TCP_ABORT_THRESHOLD
      { TCP_ABORT_THRESHOLD, "TCP_ABORT_THRESHOLD" },
#endif
#ifdef TCP_CONN_ABORT_THRESHOLD
      { TCP_CONN_ABORT_THRESHOLD, "TCP_CONN_ABORT_THRESHOLD" },
#endif
#ifdef TCP_KEEPINIT
      { TCP_KEEPINIT, "TCP_KEEPINIT" },
#endif
#ifdef TCP_PAWS
      { TCP_PAWS, "TCP_PAWS" },
#endif
#ifdef TCP_SACKENA
      { TCP_SACKENA, "TCP_SACKENA" },
#endif
#ifdef TCP_TSOPTENA
      { TCP_TSOPTENA, "TCP_TSOPTENA" },
#endif
      {0, NULL}
   } ;
   const struct sockopt *optname;

   optname = tcpopts; while (optname->so) {
      sockoptan(fd, optname, SOL_TCP, outfile);
      ++optname;
   }
   return 0;
}
#endif /* WITH_TCP */


#if _WITH_SOCKET
int sockoptan(int fd, const struct sockopt *optname, int socklay, FILE *outfile) {
#define FILAN_OPTLEN 256
   union {
      char c[FILAN_OPTLEN];
      int  i[FILAN_OPTLEN/sizeof(int)];
   } optval;
   socklen_t optlen;
   int result;

   optlen = FILAN_OPTLEN;
   result =
      Getsockopt(fd, socklay, optname->so, (void *)optval.c, &optlen);
   if (result < 0) {
      Debug6("getsockopt(%d, %d, %d, %p, {"F_socklen"}): %s",
	     fd, socklay, optname->so, optval.c, optlen, strerror(errno));
      fputc('\t', outfile);
      return -1;
   } else if (optlen == 0) {
      Debug1("getsockopt(,,, {}, %d)", optlen);
      fprintf(outfile, "%s=\"\"\t", optname->name);
   } else if (optlen == sizeof(int)) {
      Debug2("getsockopt(,,, {%d}, %d)",
	     *optval.i, optlen);
      fprintf(outfile, "%s=%d\t", optname->name, *optval.i);
   } else {
      char outbuf[FILAN_OPTLEN*9+128], *cp = outbuf;
      int i;
      for (i = 0; i < optlen/sizeof(unsigned int); ++i) {
	 cp += sprintf(cp, "%08x ", (unsigned int)optval.i[i]);
      }
      *--cp = '\0';	/* delete trailing space */
      Debug2("getsockopt(,,, {%s}, %d)", outbuf, optlen);
      fflush(outfile);
      fprintf(outfile, "%s={%s}\t", optname->name, outbuf);
   }
   return 0;
#undef FILAN_OPTLEN
}
#endif /* _WITH_SOCKET */


#if _WITH_SOCKET
int isasocket(int fd) {
   int retval;
#if HAVE_STAT64
   struct stat64 props;
#else
   struct stat props;
#endif /* HAVE_STAT64 */
   retval =
#if HAVE_STAT64
      Fstat64(fd, &props);
#else
      Fstat(fd, &props);
#endif
   if (retval < 0) {
      Info3("fstat(%d, %p): %s", fd, &props, strerror(errno));
      return 0;
   }
   /* note: when S_ISSOCK was undefined, it always gives 0 */
   return S_ISSOCK(props.st_mode);
}
#endif /* _WITH_SOCKET */


const char *getfiletypestring(int st_mode) {
   const char *s;

   switch (st_mode&S_IFMT) {
   case S_IFIFO:  s = "pipe";    break;
   case S_IFCHR:  s = "chrdev";  break;
   case S_IFDIR:  s = "dir";     break;
   case S_IFBLK:  s = "blkdev";  break;
   case S_IFREG:  s = "file";    break;
   case S_IFLNK:  s = "symlink"; break;
   case S_IFSOCK: s = "socket";  break;
      /*! AIX: MT? */
   default:       s = "undef";   break;
   }
   return s;
}

static int printtime(FILE *outfile, time_t time) {
   const char *s;

   if (filan_rawoutput) {
      fprintf(outfile, "\t"F_time, time);
   } else {
      fputc('\t', outfile);
      s = asctime(localtime(&time));
      if (strchr(s, '\n'))  *strchr(s, '\n') = '\0';
      fputs(s, outfile);
   }
   return 0;
}   
