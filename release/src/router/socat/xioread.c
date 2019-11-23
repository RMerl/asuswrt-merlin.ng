/* source: xioread.c */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

/* this is the source of the extended read function */

#include "xiosysincludes.h"
#include "xioopen.h"

#include "xio-termios.h"
#include "xio-socket.h"
#include "xio-readline.h"
#include "xio-openssl.h"

 
/* xioread() performs read() or recvfrom()
   If result is < 0, errno is valid */
ssize_t xioread(xiofile_t *file, void *buff, size_t bufsiz) {
   ssize_t bytes;
#if WITH_IP6 && 0
   int nexthead;
#endif
   struct single *pipe;
   int _errno;

   if (file->tag == XIO_TAG_INVALID) {
      Error1("xioread(): invalid xiofile descriptor %p", file);
      errno = EINVAL;
      return -1;
   }

   if (file->tag == XIO_TAG_DUAL) {
      pipe = file->dual.stream[0];
      if (pipe->tag == XIO_TAG_INVALID) {
	 Error1("xioread(): invalid xiofile sub descriptor %p[0]", file);
	 errno = EINVAL;
	 return -1;
      }
   } else {
      pipe = &file->stream;
   }

   if (pipe->readbytes) {
      if (pipe->actbytes == 0) {
	 return 0;	/* EOF by count */
      }

      if (pipe->actbytes < bufsiz) {
	 bufsiz = pipe->actbytes;
      }
   }

   switch (pipe->dtype & XIODATA_READMASK) {
   case XIOREAD_STREAM:
      do {
	 bytes = Read(pipe->fd, buff, bufsiz);
      } while (bytes < 0 && errno == EINTR);
      if (bytes < 0) {
	 _errno = errno;
	 switch (_errno) {
#if 1
	 case EPIPE: case ECONNRESET:
	    Warn4("read(%d, %p, "F_Zu"): %s",
		  pipe->fd, buff, bufsiz, strerror(_errno));
	    break;
#endif
	 default:
	    Error4("read(%d, %p, "F_Zu"): %s",
		   pipe->fd, buff, bufsiz, strerror(_errno));
	 }
	 errno = _errno;
	 return -1;
      }
      break;

   case XIOREAD_PTY:
      do {
	 bytes = Read(pipe->fd, buff, bufsiz);
      } while (bytes < 0 && errno == EINTR);
      if (bytes < 0) {
	 _errno = errno;
	 if (_errno == EIO) {
	    Notice4("read(%d, %p, "F_Zu"): %s (probably PTY closed)",
		    pipe->fd, buff, bufsiz, strerror(_errno));
	    return 0;
	 } else {
	    Error4("read(%d, %p, "F_Zu"): %s",
		   pipe->fd, buff, bufsiz, strerror(_errno));
	 }
	 errno = _errno;
	 return -1;
      }
      break;

#if WITH_READLINE
   case XIOREAD_READLINE:
      if ((bytes = xioread_readline(pipe, buff, bufsiz)) < 0) {
	 return -1;
      }      
      break;
#endif /* WITH_READLINE */

#if WITH_OPENSSL
   case XIOREAD_OPENSSL:
      /* this function prints its error messages */
      if ((bytes = xioread_openssl(pipe, buff, bufsiz)) < 0) {
	 return -1;
      }
      break;
#endif /* WITH_OPENSSL */

#if _WITH_SOCKET
   case XIOREAD_RECV:
     if (pipe->dtype & XIOREAD_RECV_FROM) {
#if WITH_RAWIP || WITH_UDP || WITH_UNIX
      struct msghdr msgh = {0};
      union sockaddr_union from = {{0}};
      socklen_t fromlen = sizeof(from);
      char infobuff[256];
      char ctrlbuff[1024];	/* ancillary messages */

      msgh.msg_name = &from;
      msgh.msg_namelen = fromlen;
#if HAVE_STRUCT_MSGHDR_MSGCONTROL
      msgh.msg_control = ctrlbuff;
#endif
#if HAVE_STRUCT_MSGHDR_MSGCONTROLLEN
      msgh.msg_controllen = sizeof(ctrlbuff);
#endif
      if (xiogetpacketsrc(pipe->fd, &msgh) < 0) {
	 return -1;
      }
      do {
	 bytes =
	    Recvfrom(pipe->fd, buff, bufsiz, 0, &from.soa, &fromlen);
      } while (bytes < 0 && errno == EINTR);
      if (bytes < 0) {
	 char infobuff[256];
	 _errno = errno;
	 Error6("recvfrom(%d, %p, "F_Zu", 0, %s, {"F_socklen"}): %s",
		pipe->fd, buff, bufsiz,
		sockaddr_info(&from.soa, fromlen, infobuff, sizeof(infobuff)),
		fromlen, strerror(errno));
	 errno = _errno;
	 return -1;
      }
      /* on packet type we also receive outgoing packets, this is not desired
       */
#if defined(PF_PACKET) && defined(PACKET_OUTGOING)
      if (from.soa.sa_family == PF_PACKET) {
	 if ((from.ll.sll_pkttype & PACKET_OUTGOING)
	    == 0) {
	    errno = EAGAIN;  return -1;
	 }
      }
#endif /* defined(PF_PACKET) && defined(PACKET_OUTGOING) */
	    
      Notice2("received packet with "F_Zu" bytes from %s",
	      bytes,
	      sockaddr_info(&from.soa, fromlen, infobuff, sizeof(infobuff)));
      if (bytes == 0) {
	 if (!pipe->para.socket.null_eof) {
	    errno = EAGAIN; return -1;
	 }
	 return bytes;
      }

      if (pipe->peersa.soa.sa_family != PF_UNSPEC) {
	 /* a peer address is registered, so we need to check if it matches */
#if 0 /* with UNIX sockets we find inconsistent lengths */
	 if (fromlen != pipe->salen) {
	    Info("recvfrom(): wrong peer address length, ignoring packet");
	    errno = EAGAIN; return -1;
	 }
#endif
	 if (pipe->dtype & XIOREAD_RECV_SKIPIP) {
	    if (pipe->peersa.soa.sa_family != from.soa.sa_family) {
	       Info("recvfrom(): wrong peer protocol, ignoring packet");
	       errno = EAGAIN; return -1;
	    }
#if WITH_IP4
	    switch (pipe->peersa.soa.sa_family) {
	    case PF_INET:
	       if (pipe->peersa.ip4.sin_addr.s_addr !=
		   from.ip4.sin_addr.s_addr) {
		  Info("recvfrom(): wrong peer address, ignoring packet");
		  errno = EAGAIN; return -1;
	       }
	       break;
	    }
#endif /* WITH_IP4 */
	 } else {
	    switch (pipe->peersa.soa.sa_family) {
#if 0
	    case PF_UNIX:
	       if (strncmp(pipe->peersa.un.sun_path, from.un.sun_path,
			   sizeof(from.un.sun_path))) {
		  Info("recvfrom(): wrong peer address, ignoring packet");
		  errno = EAGAIN; return -1;
	       }
	       break;
#endif
#if WITH_IP6
	    case PF_INET6:
	       /* e.g. Solaris recvfrom sets a __sin6_src_id component */
	       if (memcmp(&from.ip6.sin6_addr, &pipe->peersa.ip6.sin6_addr,
			  sizeof(from.ip6.sin6_addr)) ||
		   from.ip6.sin6_port != pipe->peersa.ip6.sin6_port) {
		  Info("recvfrom(): wrong peer address, ignoring packet");
		  errno = EAGAIN; return -1;
	       }
	       break;
#endif /* WITH_IP6 */
	    default:
	       if (memcmp(&from, &pipe->peersa, fromlen)) {
		  Info("recvfrom(): wrong peer address, ignoring packet");
		  errno = EAGAIN; return -1;
	       }
	    }
	 }
      }

      switch(from.soa.sa_family) {
#if HAVE_STRUCT_IP
	 int headlen;
#endif /* HAVE_STRUCT_IP */
#if WITH_IP4
      case AF_INET:
#if HAVE_STRUCT_IP
	 if (pipe->dtype & XIOREAD_RECV_SKIPIP) {
	    /* IP4 raw sockets include the header when passing a packet to the
	       application - we don't need it here. */
#if HAVE_STRUCT_IP_IP_HL
	    headlen = 4*((struct ip *)buff)->ip_hl;
#else /* happened on Tru64 */
	    headlen = 4*((struct ip *)buff)->ip_vhl;
#endif
	    if (headlen > bytes) {
	       Warn1("xioread(%d, ...)/IP4: short packet", pipe->fd);
	       bytes = 0;
	    } else {
	       memmove(buff, ((char *)buff)+headlen, bytes-headlen);
	       bytes -= headlen;
	    }
	 }
#endif /* HAVE_STRUCT_IP */
	 break;
#endif
#if WITH_IP6
      case AF_INET6:
	 /* does not seem to include header on Linux */
	 /* but sometimes on AIX */
	 break;
#endif
      default:
	 /* do nothing, for now */
	 break;
      }
      if (pipe->dtype & XIOREAD_RECV_ONESHOT) {
#if 1
	 pipe->eof = 2;
#else
	 Shutdown(pipe->fd, SHUT_RD);
#endif
	 if (pipe->ppid > 0) {
	    Kill(pipe->ppid, SIGUSR1);
	 }
      }

#if 0
      if (fromlen != pipe->fd[0].salen) {
	 Debug("recvfrom(): wrong peer address length, ignoring packet");
	 continue;
      }
      if (memcmp(&from, &pipe->fd[0].peersa.sa, fromlen)) {
	 Debug("recvfrom(): other peer address, ignoring packet");
	 Debug16("peer: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
		 pipe->fd[0].peersa.space[0],
		 pipe->fd[0].peersa.space[1],
		 pipe->fd[0].peersa.space[2],
		 pipe->fd[0].peersa.space[3],
		 pipe->fd[0].peersa.space[4],
		 pipe->fd[0].peersa.space[5],
		 pipe->fd[0].peersa.space[6],
		 pipe->fd[0].peersa.space[7],
		 pipe->fd[0].peersa.space[8],
		 pipe->fd[0].peersa.space[9],
		 pipe->fd[0].peersa.space[10],
		 pipe->fd[0].peersa.space[11],
		 pipe->fd[0].peersa.space[12],
		 pipe->fd[0].peersa.space[13],
		 pipe->fd[0].peersa.space[14],
		 pipe->fd[0].peersa.space[15]);
	 Debug16("from: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
		 from.space[0], from.space[1],
		 from.space[2], from.space[3],
		 from.space[4], from.space[5],
		 from.space[6], from.space[7],
		 from.space[8], from.space[9],
		 from.space[10], from.space[11],
		 from.space[12], from.space[13],
		 from.space[14], from.space[15]);
	 continue;
      }
#endif
#else /* !WITH_RAWIP */
      Fatal("address requires raw sockets, but they are not compiled in");
      return -1;
#endif /* !WITH_RAWIP || WITH_UDP || WITH_UNIX */

     } else /* ~XIOREAD_RECV_FROM */ {
      union sockaddr_union from;  socklen_t fromlen = sizeof(from);
      char infobuff[256];
      struct msghdr msgh = {0};
      char ctrlbuff[1024];	/* ancillary messages */

      socket_init(pipe->para.socket.la.soa.sa_family, &from);
      /* get source address */
      msgh.msg_name = &from;
      msgh.msg_namelen = fromlen;
#if HAVE_STRUCT_MSGHDR_MSGCONTROL
      msgh.msg_control = ctrlbuff;
#endif
#if HAVE_STRUCT_MSGHDR_MSGCONTROLLEN
      msgh.msg_controllen = sizeof(ctrlbuff);
#endif
      if (xiogetpacketsrc(pipe->fd, &msgh) < 0) {
	 return -1;
      }
      xiodopacketinfo(&msgh, true, false);
      if (xiocheckpeer(pipe, &from, &pipe->para.socket.la) < 0) {
	 Recvfrom(pipe->fd, buff, bufsiz, 0, &from.soa, &fromlen);  /* drop */
	 errno = EAGAIN;  return -1;
      }
      Info1("permitting packet from %s",
	    sockaddr_info((struct sockaddr *)&from, fromlen,
			  infobuff, sizeof(infobuff)));

      do {
	 bytes =
	    Recvfrom(pipe->fd, buff, bufsiz, 0, &from.soa, &fromlen);
      } while (bytes < 0 && errno == EINTR);
      if (bytes < 0) {
	 char infobuff[256];
	 _errno = errno;
	 Error6("recvfrom(%d, %p, "F_Zu", 0, %s, "F_socklen"): %s",
		pipe->fd, buff, bufsiz,
		sockaddr_info(&from.soa, fromlen, infobuff, sizeof(infobuff)),
		fromlen, strerror(errno));
	 errno = _errno;
	 return -1;
      }
      Notice2("received packet with "F_Zu" bytes from %s",
	      bytes,
	      sockaddr_info(&from.soa, fromlen, infobuff, sizeof(infobuff)));
      if (bytes == 0) {
	 if (!pipe->para.socket.null_eof) {
	    errno = EAGAIN; return -1;
	 }
	 return bytes;
      }

      switch(from.soa.sa_family) {
#if HAVE_STRUCT_IP
	 int headlen;
#endif /* HAVE_STRUCT_IP */
#if WITH_IP4
      case AF_INET:
#if HAVE_STRUCT_IP
	 if (pipe->dtype & XIOREAD_RECV_SKIPIP) {
	    /* IP4 raw sockets include the header when passing a packet to the
	       application - we don't need it here. */
#if HAVE_STRUCT_IP_IP_HL
	    headlen = 4*((struct ip *)buff)->ip_hl;
#else /* happened on Tru64 */
	    headlen = 4*((struct ip *)buff)->ip_vhl;
#endif
	    if (headlen > bytes) {
	       Warn1("xioread(%d, ...)/IP4: short packet", pipe->fd);
	       bytes = 0;
	    } else {
	       memmove(buff, ((char *)buff)+headlen, bytes-headlen);
	       bytes -= headlen;
	    }
	 }
#endif /* HAVE_STRUCT_IP */
	 break;
#endif
#if WITH_IP6
      case AF_INET6: /* does not seem to include header... */
	 break;
#endif
      default:
	 /* do nothing, for now */
	 break;
      }

     }
     break;
#endif /* _WITH_SOCKET */

   default:
      Error("internal: undefined read operation");
      errno = EINVAL;  return -1;
   }
   pipe->actbytes -= bytes;
   return bytes;
}


/* this function is intended only for some special address types where the
   select()/poll() calls cannot strictly determine if (more) read data is
   available. currently this is for the OpenSSL based addresses.
*/
ssize_t xiopending(xiofile_t *file) {
   struct single *pipe;

   if (file->tag == XIO_TAG_INVALID) {
      Error1("xiopending(): invalid xiofile descriptor %p", file);
      errno = EINVAL;
      return -1;
   }

   if (file->tag == XIO_TAG_DUAL) {
      pipe = file->dual.stream[0];
      if (pipe->tag == XIO_TAG_INVALID) {
	 Error1("xiopending(): invalid xiofile sub descriptor %p[0]", file);
	 errno = EINVAL;
	 return -1;
      }
   } else {
      pipe = &file->stream;
   }

   switch (pipe->dtype & XIODATA_READMASK) {
#if WITH_OPENSSL
   case XIOREAD_OPENSSL:
      return xiopending_openssl(pipe);
#endif /* WITH_OPENSSL */
   default:
      return 0;
   }
}

