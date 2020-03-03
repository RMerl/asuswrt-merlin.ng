/*
 *  pcap-linux.c: Packet capture interface to the Linux kernel
 *
 *  Copyright (c) 2000 Torsten Landschoff <torsten@debian.org>
 *  		       Sebastian Krahmer  <krahmer@cs.uni-potsdam.de>
 *
 *  License: BSD
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *  3. The names of the authors may not be used to endorse or promote
 *     products derived from this software without specific prior
 *     written permission.
 *
 *  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 *  IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *  Modifications:     Added PACKET_MMAP support
 *                     Paolo Abeni <paolo.abeni@email.it>
 *
 *                     based on previous works of:
 *                     Simon Patarin <patarin@cs.unibo.it>
 *                     Phil Wood <cpw@lanl.gov>
 */

#ifndef lint
static const char rcsid[] _U_ =
    "@(#) $Header: /tcpdump/master/libpcap/pcap-linux.c,v 1.129.2.29 2008-10-28 00:50:39 guy Exp $ (LBL)";
#endif // endif

/*
 * Known problems with 2.0[.x] kernels:
 *
 *   - The loopback device gives every packet twice; on 2.2[.x] kernels,
 *     if we use PF_PACKET, we can filter out the transmitted version
 *     of the packet by using data in the "sockaddr_ll" returned by
 *     "recvfrom()", but, on 2.0[.x] kernels, we have to use
 *     PF_INET/SOCK_PACKET, which means "recvfrom()" supplies a
 *     "sockaddr_pkt" which doesn't give us enough information to let
 *     us do that.
 *
 *   - We have to set the interface's IFF_PROMISC flag ourselves, if
 *     we're to run in promiscuous mode, which means we have to turn
 *     it off ourselves when we're done; the kernel doesn't keep track
 *     of how many sockets are listening promiscuously, which means
 *     it won't get turned off automatically when no sockets are
 *     listening promiscuously.  We catch "pcap_close()" and, for
 *     interfaces we put into promiscuous mode, take them out of
 *     promiscuous mode - which isn't necessarily the right thing to
 *     do, if another socket also requested promiscuous mode between
 *     the time when we opened the socket and the time when we close
 *     the socket.
 *
 *   - MSG_TRUNC isn't supported, so you can't specify that "recvfrom()"
 *     return the amount of data that you could have read, rather than
 *     the amount that was returned, so we can't just allocate a buffer
 *     whose size is the snapshot length and pass the snapshot length
 *     as the byte count, and also pass MSG_TRUNC, so that the return
 *     value tells us how long the packet was on the wire.
 *
 *     This means that, if we want to get the actual size of the packet,
 *     so we can return it in the "len" field of the packet header,
 *     we have to read the entire packet, not just the part that fits
 *     within the snapshot length, and thus waste CPU time copying data
 *     from the kernel that our caller won't see.
 *
 *     We have to get the actual size, and supply it in "len", because
 *     otherwise, the IP dissector in tcpdump, for example, will complain
 *     about "truncated-ip", as the packet will appear to have been
 *     shorter, on the wire, than the IP header said it should have been.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // endif

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/utsname.h>
#include <sys/mman.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/if_ether.h>
#include <net/if_arp.h>
#include <poll.h>

/*
 * Got Wireless Extensions?
 */
#ifdef HAVE_LINUX_WIRELESS_H
#include <linux/wireless.h>
#endif // endif

#include "pcap-int.h"
#include "pcap/sll.h"
#include "pcap/vlan.h"

#ifdef HAVE_DAG_API
#include "pcap-dag.h"
#endif /* HAVE_DAG_API */

#ifdef HAVE_SEPTEL_API
#include "pcap-septel.h"
#endif /* HAVE_SEPTEL_API */

#ifdef PCAP_SUPPORT_USB
#include "pcap-usb-linux.h"
#endif // endif

#ifdef PCAP_SUPPORT_BT
#include "pcap-bt-linux.h"
#endif // endif

#ifdef HAVE_REMOTE
#include <pcap-remote.h>
#endif // endif

/*
 * If PF_PACKET is defined, we can use {SOCK_RAW,SOCK_DGRAM}/PF_PACKET
 * sockets rather than SOCK_PACKET sockets.
 *
 * To use them, we include <linux/if_packet.h> rather than
 * <netpacket/packet.h>; we do so because
 *
 *	some Linux distributions (e.g., Slackware 4.0) have 2.2 or
 *	later kernels and libc5, and don't provide a <netpacket/packet.h>
 *	file;
 *
 *	not all versions of glibc2 have a <netpacket/packet.h> file
 *	that defines stuff needed for some of the 2.4-or-later-kernel
 *	features, so if the system has a 2.4 or later kernel, we
 *	still can't use those features.
 *
 * We're already including a number of other <linux/XXX.h> headers, and
 * this code is Linux-specific (no other OS has PF_PACKET sockets as
 * a raw packet capture mechanism), so it's not as if you gain any
 * useful portability by using <netpacket/packet.h>
 *
 * XXX - should we just include <linux/if_packet.h> even if PF_PACKET
 * isn't defined?  It only defines one data structure in 2.0.x, so
 * it shouldn't cause any problems.
 */
#ifdef PF_PACKET
# include <linux/if_packet.h>

 /*
  * On at least some Linux distributions (for example, Red Hat 5.2),
  * there's no <netpacket/packet.h> file, but PF_PACKET is defined if
  * you include <sys/socket.h>, but <linux/if_packet.h> doesn't define
  * any of the PF_PACKET stuff such as "struct sockaddr_ll" or any of
  * the PACKET_xxx stuff.
  *
  * So we check whether PACKET_HOST is defined, and assume that we have
  * PF_PACKET sockets only if it is defined.
  */
# ifdef PACKET_HOST
#  define HAVE_PF_PACKET_SOCKETS
#  ifdef PACKET_AUXDATA
#   define HAVE_PACKET_AUXDATA
#  endif /* PACKET_AUXDATA */
# endif /* PACKET_HOST */

 /* check for memory mapped access avaibility. We assume every needed
  * struct is defined if the macro TPACKET_HDRLEN is defined, because it
  * uses many ring related structs and macros */
# ifdef TPACKET_HDRLEN
#  define HAVE_PACKET_RING
#  ifdef TPACKET2_HDRLEN
#   define HAVE_TPACKET2
#  else
#   define TPACKET_V1	0
#  endif /* TPACKET2_HDRLEN */
# endif /* TPACKET_HDRLEN */
#endif /* PF_PACKET */

#ifdef SO_ATTACH_FILTER
#include <linux/types.h>
#include <linux/filter.h>
#endif // endif

#ifndef HAVE_SOCKLEN_T
typedef int		socklen_t;
#endif // endif

#ifndef MSG_TRUNC
/*
 * This is being compiled on a system that lacks MSG_TRUNC; define it
 * with the value it has in the 2.2 and later kernels, so that, on
 * those kernels, when we pass it in the flags argument to "recvfrom()"
 * we're passing the right value and thus get the MSG_TRUNC behavior
 * we want.  (We don't get that behavior on 2.0[.x] kernels, because
 * they didn't support MSG_TRUNC.)
 */
#define MSG_TRUNC	0x20
#endif // endif

#ifndef SOL_PACKET
/*
 * This is being compiled on a system that lacks SOL_PACKET; define it
 * with the value it has in the 2.2 and later kernels, so that we can
 * set promiscuous mode in the good modern way rather than the old
 * 2.0-kernel crappy way.
 */
#define SOL_PACKET	263
#endif // endif

#define MAX_LINKHEADER_SIZE	256

/*
 * When capturing on all interfaces we use this as the buffer size.
 * Should be bigger then all MTUs that occur in real life.
 * 64kB should be enough for now.
 */
#define BIGGER_THAN_ALL_MTUS	(64*1024)

/*
 * Prototypes for internal functions and methods.
 */
static void map_arphrd_to_dlt(pcap_t *, int, int);
#ifdef HAVE_PF_PACKET_SOCKETS
static short int map_packet_type_to_sll_type(short int);
#endif // endif
static int pcap_activate_linux(pcap_t *);
static int activate_old(pcap_t *);
static int activate_new(pcap_t *);
static int activate_mmap(pcap_t *);
static int pcap_can_set_rfmon_linux(pcap_t *);
static int pcap_read_linux(pcap_t *, int, pcap_handler, u_char *);
static int pcap_read_packet(pcap_t *, pcap_handler, u_char *);
static int pcap_inject_linux(pcap_t *, const void *, size_t);
static int pcap_stats_linux(pcap_t *, struct pcap_stat *);
static int pcap_setfilter_linux(pcap_t *, struct bpf_program *);
static int pcap_setdirection_linux(pcap_t *, pcap_direction_t);
static void pcap_cleanup_linux(pcap_t *);

union thdr {
	struct tpacket_hdr	*h1;
	struct tpacket2_hdr	*h2;
	void			*raw;
};

#ifdef HAVE_PACKET_RING
#define RING_GET_FRAME(h) (((union thdr **)h->buffer)[h->offset])

static void destroy_ring(pcap_t *handle);
static int create_ring(pcap_t *handle);
static int prepare_tpacket_socket(pcap_t *handle);
static void pcap_cleanup_linux_mmap(pcap_t *);
static int pcap_read_linux_mmap(pcap_t *, int, pcap_handler , u_char *);
static int pcap_setfilter_linux_mmap(pcap_t *, struct bpf_program *);
static int pcap_setnonblock_mmap(pcap_t *p, int nonblock, char *errbuf);
static int pcap_getnonblock_mmap(pcap_t *p, char *errbuf);
#endif // endif

/*
 * Wrap some ioctl calls
 */
#ifdef HAVE_PF_PACKET_SOCKETS
static int	iface_get_id(int fd, const char *device, char *ebuf);
#endif // endif
static int	iface_get_mtu(int fd, const char *device, char *ebuf);
static int 	iface_get_arptype(int fd, const char *device, char *ebuf);
#ifdef HAVE_PF_PACKET_SOCKETS
static int 	iface_bind(int fd, int ifindex, char *ebuf);
static int	has_wext(int sock_fd, const char *device, char *ebuf);
static int	enter_rfmon_mode_wext(pcap_t *handle, int sock_fd,
    const char *device);
#endif // endif
static int 	iface_bind_old(int fd, const char *device, char *ebuf);

#ifdef SO_ATTACH_FILTER
static int	fix_program(pcap_t *handle, struct sock_fprog *fcode);
static int	fix_offset(struct bpf_insn *p);
static int	set_kernel_filter(pcap_t *handle, struct sock_fprog *fcode);
static int	reset_kernel_filter(pcap_t *handle);

static struct sock_filter	total_insn
	= BPF_STMT(BPF_RET | BPF_K, 0);
static struct sock_fprog	total_fcode
	= { 1, &total_insn };
#endif // endif

pcap_t *
pcap_create(const char *device, char *ebuf)
{
	pcap_t *handle;
	char *device_no_prefix = (char*)device;

#ifdef HAVE_REMOTE
	/*
		Retrofit; we have to make older applications compatible with the remote capture
		So, we're calling the pcap_open_remote() from here, that is a very dirty thing.
		Obviously, we cannot exploit all the new features; for instance, we cannot
		send authentication, we cannot use a UDP data connection, and so on.
	*/

	char host[PCAP_BUF_SIZE + 1];
	char port[PCAP_BUF_SIZE + 1];
	char name[PCAP_BUF_SIZE + 1];
	int srctype;

	if (pcap_parsesrcstr(device, &srctype, host, port, name, ebuf) )
		return NULL;

	if (srctype == PCAP_SRC_IFREMOTE)
	{
		handle = pcap_create_common(device, ebuf);
		if (handle == NULL)
			return NULL;

		handle->activate_op = pcap_activate_linux;
		handle->can_set_rfmon_op = NULL;
		return handle;
	}

	if (srctype == PCAP_SRC_IFLOCAL)
	{
		/*
		 * If it starts with rpcap://, cut down the string
		 */
		if (strncmp(device, PCAP_SRC_IF_STRING, strlen(PCAP_SRC_IF_STRING)) == 0)
		{
			device_no_prefix = (char*)device + strlen(PCAP_SRC_IF_STRING);
		}
	}
#endif		/* HAVE_REMOTE */

#ifdef HAVE_DAG_API
	if (strstr(device_no_prefix, "dag")) {
		return dag_create(device_no_prefix, ebuf);
	}
#endif /* HAVE_DAG_API */

#ifdef HAVE_SEPTEL_API
	if (strstr(device_no_prefix, "septel")) {
		return septel_create(device_no_prefix, ebuf);
	}
#endif /* HAVE_SEPTEL_API */

#ifdef PCAP_SUPPORT_BT
	if (strstr(device_no_prefix, "bluetooth")) {
		return bt_create(device_no_prefix, ebuf);
	}
#endif // endif

#ifdef PCAP_SUPPORT_USB
	if (strstr(device_no_prefix, "usb")) {
		return usb_create(device_no_prefix, ebuf);
	}
#endif // endif

	handle = pcap_create_common(device_no_prefix, ebuf);
	if (handle == NULL)
		return NULL;

#ifdef HAVE_TC_API
	if (IsTcDevice(handle) == TRUE)
	{
		handle->activate_op = TcActivate;
		handle->can_set_rfmon_op = NULL;
	}
#endif // endif
	else
	{
	handle->activate_op = pcap_activate_linux;
	handle->can_set_rfmon_op = pcap_can_set_rfmon_linux;
	}

	return handle;
}

static int
pcap_can_set_rfmon_linux(pcap_t *p)
{
#ifdef IW_MODE_MONITOR
	int sock_fd;	struct iwreq ireq;
#endif // endif

	if (p->opt.source == NULL) {
		/*
		 * This is equivalent to the "any" device, and we don't
		 * support monitor mode on it.
		 */
		return 0;
	}

#ifdef IW_MODE_MONITOR
	/*
	 * Bleah.  There doesn't appear to be an ioctl to use to ask
	 * whether a device supports monitor mode; we'll just do
	 * SIOCGIWMODE and, if it succeeds, assume the device supports
	 * monitor mode.
	 *
	 * Open a socket on which to attempt to get the mode.
	 * (We assume that if we have Wireless Extensions support
	 * we also have PF_PACKET support.)
	 */
	sock_fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (sock_fd == -1) {
		(void)snprintf(p->errbuf, PCAP_ERRBUF_SIZE,
		    "socket: %s", pcap_strerror(errno));
		return PCAP_ERROR;
	}

	/*
	 * Attempt to get the current mode.
	 */
	strncpy(ireq.ifr_ifrn.ifrn_name, p->opt.source,
	    sizeof ireq.ifr_ifrn.ifrn_name);
	ireq.ifr_ifrn.ifrn_name[sizeof ireq.ifr_ifrn.ifrn_name - 1] = 0;
	if (ioctl(sock_fd, SIOCGIWMODE, &ireq) != -1) {
		/*
		 * Well, we got the mode; assume we can set it.
		 */
		close(sock_fd);
		return 1;
	}
	if (errno == ENODEV) {
		/* The device doesn't even exist. */
		close(sock_fd);
		return PCAP_ERROR_NO_SUCH_DEVICE;
	}
	close(sock_fd);
#endif // endif
	return 0;
}

/*
 * With older kernels promiscuous mode is kind of interesting because we
 * have to reset the interface before exiting. The problem can't really
 * be solved without some daemon taking care of managing usage counts.
 * If we put the interface into promiscuous mode, we set a flag indicating
 * that we must take it out of that mode when the interface is closed,
 * and, when closing the interface, if that flag is set we take it out
 * of promiscuous mode.
 *
 * Even with newer kernels, we have the same issue with rfmon mode.
 */

static void	pcap_cleanup_linux( pcap_t *handle )
{
	struct ifreq	ifr;
#ifdef IW_MODE_MONITOR
	struct iwreq ireq;
#endif // endif

	if (handle->md.must_clear != 0) {
		/*
		 * There's something we have to do when closing this
		 * pcap_t.
		 */
		if (handle->md.must_clear & MUST_CLEAR_PROMISC) {
			/*
			 * We put the interface into promiscuous mode;
			 * take it out of promiscuous mode.
			 *
			 * XXX - if somebody else wants it in promiscuous
			 * mode, this code cannot know that, so it'll take
			 * it out of promiscuous mode.  That's not fixable
			 * in 2.0[.x] kernels.
			 */
			memset(&ifr, 0, sizeof(ifr));
			strncpy(ifr.ifr_name, handle->md.device,
			    sizeof(ifr.ifr_name));
			if (ioctl(handle->fd, SIOCGIFFLAGS, &ifr) == -1) {
				fprintf(stderr,
				    "Can't restore interface flags (SIOCGIFFLAGS failed: %s).\n"
				    "Please adjust manually.\n"
				    "Hint: This can't happen with Linux >= 2.2.0.\n",
				    strerror(errno));
			} else {
				if (ifr.ifr_flags & IFF_PROMISC) {
					/*
					 * Promiscuous mode is currently on;
					 * turn it off.
					 */
					ifr.ifr_flags &= ~IFF_PROMISC;
					if (ioctl(handle->fd, SIOCSIFFLAGS,
					    &ifr) == -1) {
						fprintf(stderr,
						    "Can't restore interface flags (SIOCSIFFLAGS failed: %s).\n"
						    "Please adjust manually.\n"
						    "Hint: This can't happen with Linux >= 2.2.0.\n",
						    strerror(errno));
					}
				}
			}
		}

#ifdef IW_MODE_MONITOR
		if (handle->md.must_clear & MUST_CLEAR_RFMON) {
			/*
			 * We put the interface into rfmon mode;
			 * take it out of rfmon mode.
			 *
			 * XXX - if somebody else wants it in rfmon
			 * mode, this code cannot know that, so it'll take
			 * it out of rfmon mode.
			 */
			strncpy(ireq.ifr_ifrn.ifrn_name, handle->md.device,
			    sizeof ireq.ifr_ifrn.ifrn_name);
			ireq.ifr_ifrn.ifrn_name[sizeof ireq.ifr_ifrn.ifrn_name - 1]
			    = 0;
			ireq.u.mode = handle->md.oldmode;
			if (ioctl(handle->fd, SIOCSIWMODE, &ireq) == -1) {
				/*
				 * Scientist, you've failed.
				 */
				fprintf(stderr,
				    "Can't restore interface wireless mode (SIOCSIWMODE failed: %s).\n"
				    "Please adjust manually.\n",
				    strerror(errno));
			}
		}
#endif // endif

		/*
		 * Take this pcap out of the list of pcaps for which we
		 * have to take the interface out of some mode.
		 */
		pcap_remove_from_pcaps_to_close(handle);
	}

	if (handle->md.device != NULL) {
		free(handle->md.device);
		handle->md.device = NULL;
	}
	pcap_cleanup_live_common(handle);
}

/*
 *  Get a handle for a live capture from the given device. You can
 *  pass NULL as device to get all packages (without link level
 *  information of course). If you pass 1 as promisc the interface
 *  will be set to promiscous mode (XXX: I think this usage should
 *  be deprecated and functions be added to select that later allow
 *  modification of that values -- Torsten).
 */
static int
pcap_activate_linux(pcap_t *handle)
{
	const char	*device;
	int		status = 0;
	int		activate_ok = 0;

#ifdef HAVE_REMOTE
	char host[PCAP_BUF_SIZE + 1];
	char port[PCAP_BUF_SIZE + 1];
	char name[PCAP_BUF_SIZE + 1];
	int srctype;
	int opensource_remote_result;

	/*
		Retrofit; we have to make older applications compatible with the remote capture
		So, we're calling the pcap_open_remote() from here, that is a very dirty thing.
		Obviously, we cannot exploit all the new features; for instance, we cannot
		send authentication, we cannot use a UDP data connection, and so on.
	*/
	if (pcap_parsesrcstr(handle->opt.source, &srctype, host, port, name, handle->errbuf) )
		return PCAP_ERROR;

	if (srctype == PCAP_SRC_IFREMOTE)
	{
		opensource_remote_result = pcap_opensource_remote(handle, NULL);

		if (opensource_remote_result != 0)
			return opensource_remote_result;

		handle->rmt_flags= (handle->opt.promisc) ? PCAP_OPENFLAG_PROMISCUOUS : 0;

		return 0;
	}

	if (srctype == PCAP_SRC_IFLOCAL)
	{
		/*
		 * If it starts with rpcap://, cut down the string
		 */
		if (strncmp(handle->opt.source, PCAP_SRC_IF_STRING, strlen(PCAP_SRC_IF_STRING)) == 0)
		{
			size_t len = strlen(handle->opt.source) - strlen(PCAP_SRC_IF_STRING) + 1;
			char *new_string;
			/*
			 allocate a new string and free the old one
			 */
			if (len > 0)
			{
				new_string = (char*)malloc(len);
				if (new_string != NULL)
				{
					char *tmp;
					strcpy(new_string, handle->opt.source + strlen(PCAP_SRC_IF_STRING));
					tmp = handle->opt.source;
					handle->opt.source = new_string;
					free(tmp);
				}
			}
		}
	}

#endif	/* HAVE_REMOTE */

	device = handle->opt.source;

	handle->inject_op = pcap_inject_linux;
	handle->setfilter_op = pcap_setfilter_linux;
	handle->setdirection_op = pcap_setdirection_linux;
	handle->set_datalink_op = NULL;	/* can't change data link type */
	handle->getnonblock_op = pcap_getnonblock_fd;
	handle->setnonblock_op = pcap_setnonblock_fd;
	handle->cleanup_op = pcap_cleanup_linux;
	handle->read_op = pcap_read_linux;
	handle->stats_op = pcap_stats_linux;

	/*
	 * NULL and "any" are special devices which give us the hint to
	 * monitor all devices.
	 */
	if (!device || strcmp(device, "any") == 0) {
		device			= NULL;
		handle->md.device	= strdup("any");
		if (handle->opt.promisc) {
			handle->opt.promisc = 0;
			/* Just a warning. */
			snprintf(handle->errbuf, PCAP_ERRBUF_SIZE,
			    "Promiscuous mode not supported on the \"any\" device");
			status = PCAP_WARNING_PROMISC_NOTSUP;
		}

	} else
		handle->md.device	= strdup(device);

	if (handle->md.device == NULL) {
		snprintf(handle->errbuf, PCAP_ERRBUF_SIZE, "strdup: %s",
			 pcap_strerror(errno) );
		return PCAP_ERROR;
	}

	/*
	 * Current Linux kernels use the protocol family PF_PACKET to
	 * allow direct access to all packets on the network while
	 * older kernels had a special socket type SOCK_PACKET to
	 * implement this feature.
	 * While this old implementation is kind of obsolete we need
	 * to be compatible with older kernels for a while so we are
	 * trying both methods with the newer method preferred.
	 */

	if ((status = activate_new(handle)) == 1) {
		activate_ok = 1;
		/*
		 * Try to use memory-mapped access.
		 */
		if (activate_mmap(handle) == 1)
			return 0;	/* we succeeded; nothing more to do */
	}
	else if (status == 0) {
		/* Non-fatal error; try old way */
		if ((status = activate_old(handle)) == 1)
			activate_ok = 1;
	}
	if (!activate_ok) {
		/*
		 * Both methods to open the packet socket failed. Tidy
		 * up and report our failure (ebuf is expected to be
		 * set by the functions above).
		 */
		goto fail;
	}

	if (handle->opt.buffer_size != 0) {
		/*
		 * Set the socket buffer size to the specified value.
		 */
		if (setsockopt(handle->fd, SOL_SOCKET, SO_RCVBUF,
		    &handle->opt.buffer_size,
		    sizeof(handle->opt.buffer_size)) == -1) {
			snprintf(handle->errbuf, PCAP_ERRBUF_SIZE,
				 "SO_RCVBUF: %s", pcap_strerror(errno));
			status = PCAP_ERROR;
			goto fail;
		}
	}

	/* Allocate the buffer */

	handle->buffer	 = malloc(handle->bufsize + handle->offset);
	if (!handle->buffer) {
		snprintf(handle->errbuf, PCAP_ERRBUF_SIZE,
			 "malloc: %s", pcap_strerror(errno));
		status = PCAP_ERROR;
		goto fail;
	}

	/*
	 * "handle->fd" is a socket, so "select()" and "poll()"
	 * should work on it.
	 */
	handle->selectable_fd = handle->fd;

	return status;

fail:
	pcap_cleanup_linux(handle);
	return status;
}

/*
 *  Read at most max_packets from the capture stream and call the callback
 *  for each of them. Returns the number of packets handled or -1 if an
 *  error occured.
 */
static int
pcap_read_linux(pcap_t *handle, int max_packets, pcap_handler callback, u_char *user)
{
	/*
	 * Currently, on Linux only one packet is delivered per read,
	 * so we don't loop.
	 */
	return pcap_read_packet(handle, callback, user);
}

/*
 *  Read a packet from the socket calling the handler provided by
 *  the user. Returns the number of packets received or -1 if an
 *  error occured.
 */
static int
pcap_read_packet(pcap_t *handle, pcap_handler callback, u_char *userdata)
{
	u_char			*bp;
	int			offset;
#ifdef HAVE_PF_PACKET_SOCKETS
	struct sockaddr_ll	from;
	struct sll_header	*hdrp;
#else
	struct sockaddr		from;
#endif // endif
#if defined(HAVE_PACKET_AUXDATA) && defined(HAVE_LINUX_TPACKET_AUXDATA_TP_VLAN_TCI)
	struct iovec		iov;
	struct msghdr		msg;
	struct cmsghdr		*cmsg;
	union {
		struct cmsghdr	cmsg;
		char		buf[CMSG_SPACE(sizeof(struct tpacket_auxdata))];
	} cmsg_buf;
#else /* defined(HAVE_PACKET_AUXDATA) && defined(HAVE_LINUX_TPACKET_AUXDATA_TP_VLAN_TCI) */
	socklen_t		fromlen;
#endif /* defined(HAVE_PACKET_AUXDATA) && defined(HAVE_LINUX_TPACKET_AUXDATA_TP_VLAN_TCI) */
	int			packet_len, caplen;
	struct pcap_pkthdr	pcap_header;

#ifdef HAVE_PF_PACKET_SOCKETS
	/*
	 * If this is a cooked device, leave extra room for a
	 * fake packet header.
	 */
	if (handle->md.cooked)
		offset = SLL_HDR_LEN;
	else
		offset = 0;
#else
	/*
	 * This system doesn't have PF_PACKET sockets, so it doesn't
	 * support cooked devices.
	 */
	offset = 0;
#endif // endif

	/*
	 * Receive a single packet from the kernel.
	 * We ignore EINTR, as that might just be due to a signal
	 * being delivered - if the signal should interrupt the
	 * loop, the signal handler should call pcap_breakloop()
	 * to set handle->break_loop (we ignore it on other
	 * platforms as well).
	 * We also ignore ENETDOWN, so that we can continue to
	 * capture traffic if the interface goes down and comes
	 * back up again; comments in the kernel indicate that
	 * we'll just block waiting for packets if we try to
	 * receive from a socket that delivered ENETDOWN, and,
	 * if we're using a memory-mapped buffer, we won't even
	 * get notified of "network down" events.
	 */
	bp = handle->buffer + handle->offset;

#if defined(HAVE_PACKET_AUXDATA) && defined(HAVE_LINUX_TPACKET_AUXDATA_TP_VLAN_TCI)
	msg.msg_name		= &from;
	msg.msg_namelen		= sizeof(from);
	msg.msg_iov		= &iov;
	msg.msg_iovlen		= 1;
	msg.msg_control		= &cmsg_buf;
	msg.msg_controllen	= sizeof(cmsg_buf);
	msg.msg_flags		= 0;

	iov.iov_len		= handle->bufsize - offset;
	iov.iov_base		= bp + offset;
#endif /* defined(HAVE_PACKET_AUXDATA) && defined(HAVE_LINUX_TPACKET_AUXDATA_TP_VLAN_TCI) */

	do {
		/*
		 * Has "pcap_breakloop()" been called?
		 */
		if (handle->break_loop) {
			/*
			 * Yes - clear the flag that indicates that it
			 * has, and return -2 as an indication that we
			 * were told to break out of the loop.
			 */
			handle->break_loop = 0;
			return -2;
		}

#if defined(HAVE_PACKET_AUXDATA) && defined(HAVE_LINUX_TPACKET_AUXDATA_TP_VLAN_TCI)
		packet_len = recvmsg(handle->fd, &msg, MSG_TRUNC);
#else /* defined(HAVE_PACKET_AUXDATA) && defined(HAVE_LINUX_TPACKET_AUXDATA_TP_VLAN_TCI) */
		fromlen = sizeof(from);
		packet_len = recvfrom(
			handle->fd, bp + offset,
			handle->bufsize - offset, MSG_TRUNC,
			(struct sockaddr *) &from, &fromlen);
#endif /* defined(HAVE_PACKET_AUXDATA) && defined(HAVE_LINUX_TPACKET_AUXDATA_TP_VLAN_TCI) */
	} while (packet_len == -1 && (errno == EINTR || errno == ENETDOWN));

	/* Check if an error occured */

	if (packet_len == -1) {
		if (errno == EAGAIN)
			return 0;	/* no packet there */
		else {
			snprintf(handle->errbuf, PCAP_ERRBUF_SIZE,
				 "recvfrom: %s", pcap_strerror(errno));
			return -1;
		}
	}

#ifdef HAVE_PF_PACKET_SOCKETS
	if (!handle->md.sock_packet) {
		/*
		 * Unfortunately, there is a window between socket() and
		 * bind() where the kernel may queue packets from any
		 * interface.  If we're bound to a particular interface,
		 * discard packets not from that interface.
		 *
		 * (If socket filters are supported, we could do the
		 * same thing we do when changing the filter; however,
		 * that won't handle packet sockets without socket
		 * filter support, and it's a bit more complicated.
		 * It would save some instructions per packet, however.)
		 */
		if (handle->md.ifindex != -1 &&
		    from.sll_ifindex != handle->md.ifindex)
			return 0;

		/*
		 * Do checks based on packet direction.
		 * We can only do this if we're using PF_PACKET; the
		 * address returned for SOCK_PACKET is a "sockaddr_pkt"
		 * which lacks the relevant packet type information.
		 */
		if (from.sll_pkttype == PACKET_OUTGOING) {
			/*
			 * Outgoing packet.
			 * If this is from the loopback device, reject it;
			 * we'll see the packet as an incoming packet as well,
			 * and we don't want to see it twice.
			 */
			if (from.sll_ifindex == handle->md.lo_ifindex)
				return 0;

			/*
			 * If the user only wants incoming packets, reject it.
			 */
			if (handle->direction == PCAP_D_IN)
				return 0;
		} else {
			/*
			 * Incoming packet.
			 * If the user only wants outgoing packets, reject it.
			 */
			if (handle->direction == PCAP_D_OUT)
				return 0;
		}
	}
#endif // endif

#ifdef HAVE_PF_PACKET_SOCKETS
	/*
	 * If this is a cooked device, fill in the fake packet header.
	 */
	if (handle->md.cooked) {
		/*
		 * Add the length of the fake header to the length
		 * of packet data we read.
		 */
		packet_len += SLL_HDR_LEN;

		hdrp = (struct sll_header *)bp;
		hdrp->sll_pkttype = map_packet_type_to_sll_type(from.sll_pkttype);
		hdrp->sll_hatype = htons(from.sll_hatype);
		hdrp->sll_halen = htons(from.sll_halen);
		memcpy(hdrp->sll_addr, from.sll_addr,
		    (from.sll_halen > SLL_ADDRLEN) ?
		      SLL_ADDRLEN :
		      from.sll_halen);
		hdrp->sll_protocol = from.sll_protocol;
	}

#if defined(HAVE_PACKET_AUXDATA) && defined(HAVE_LINUX_TPACKET_AUXDATA_TP_VLAN_TCI)
	for (cmsg = CMSG_FIRSTHDR(&msg); cmsg; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
		struct tpacket_auxdata *aux;
		unsigned int len;
		struct vlan_tag *tag;

		if (cmsg->cmsg_len < CMSG_LEN(sizeof(struct tpacket_auxdata)) ||
		    cmsg->cmsg_level != SOL_PACKET ||
		    cmsg->cmsg_type != PACKET_AUXDATA)
			continue;

		aux = (struct tpacket_auxdata *)CMSG_DATA(cmsg);
		if (aux->tp_vlan_tci == 0)
			continue;

		len = packet_len > iov.iov_len ? iov.iov_len : packet_len;
		if (len < 2 * ETH_ALEN)
			break;

		bp -= VLAN_TAG_LEN;
		memmove(bp, bp + VLAN_TAG_LEN, 2 * ETH_ALEN);

		tag = (struct vlan_tag *)(bp + 2 * ETH_ALEN);
		tag->vlan_tpid = htons(ETH_P_8021Q);
		tag->vlan_tci = htons(aux->tp_vlan_tci);

		packet_len += VLAN_TAG_LEN;
	}
#endif /* defined(HAVE_PACKET_AUXDATA) && defined(HAVE_LINUX_TPACKET_AUXDATA_TP_VLAN_TCI) */
#endif /* HAVE_PF_PACKET_SOCKETS */

	/*
	 * XXX: According to the kernel source we should get the real
	 * packet len if calling recvfrom with MSG_TRUNC set. It does
	 * not seem to work here :(, but it is supported by this code
	 * anyway.
	 * To be honest the code RELIES on that feature so this is really
	 * broken with 2.2.x kernels.
	 * I spend a day to figure out what's going on and I found out
	 * that the following is happening:
	 *
	 * The packet comes from a random interface and the packet_rcv
	 * hook is called with a clone of the packet. That code inserts
	 * the packet into the receive queue of the packet socket.
	 * If a filter is attached to that socket that filter is run
	 * first - and there lies the problem. The default filter always
	 * cuts the packet at the snaplen:
	 *
	 * # tcpdump -d
	 * (000) ret      #68
	 *
	 * So the packet filter cuts down the packet. The recvfrom call
	 * says "hey, it's only 68 bytes, it fits into the buffer" with
	 * the result that we don't get the real packet length. This
	 * is valid at least until kernel 2.2.17pre6.
	 *
	 * We currently handle this by making a copy of the filter
	 * program, fixing all "ret" instructions with non-zero
	 * operands to have an operand of 65535 so that the filter
	 * doesn't truncate the packet, and supplying that modified
	 * filter to the kernel.
	 */

	caplen = packet_len;
	if (caplen > handle->snapshot)
		caplen = handle->snapshot;

	/* Run the packet filter if not using kernel filter */
	if (!handle->md.use_bpf && handle->fcode.bf_insns) {
		if (bpf_filter(handle->fcode.bf_insns, bp,
		                packet_len, caplen) == 0)
		{
			/* rejected by filter */
			return 0;
		}
	}

	/* Fill in our own header data */

	if (ioctl(handle->fd, SIOCGSTAMP, &pcap_header.ts) == -1) {
		snprintf(handle->errbuf, PCAP_ERRBUF_SIZE,
			 "SIOCGSTAMP: %s", pcap_strerror(errno));
		return -1;
	}
	pcap_header.caplen	= caplen;
	pcap_header.len		= packet_len;

	/*
	 * Count the packet.
	 *
	 * Arguably, we should count them before we check the filter,
	 * as on many other platforms "ps_recv" counts packets
	 * handed to the filter rather than packets that passed
	 * the filter, but if filtering is done in the kernel, we
	 * can't get a count of packets that passed the filter,
	 * and that would mean the meaning of "ps_recv" wouldn't
	 * be the same on all Linux systems.
	 *
	 * XXX - it's not the same on all systems in any case;
	 * ideally, we should have a "get the statistics" call
	 * that supplies more counts and indicates which of them
	 * it supplies, so that we supply a count of packets
	 * handed to the filter only on platforms where that
	 * information is available.
	 *
	 * We count them here even if we can get the packet count
	 * from the kernel, as we can only determine at run time
	 * whether we'll be able to get it from the kernel (if
	 * HAVE_TPACKET_STATS isn't defined, we can't get it from
	 * the kernel, but if it is defined, the library might
	 * have been built with a 2.4 or later kernel, but we
	 * might be running on a 2.2[.x] kernel without Alexey
	 * Kuznetzov's turbopacket patches, and thus the kernel
	 * might not be able to supply those statistics).  We
	 * could, I guess, try, when opening the socket, to get
	 * the statistics, and if we can not increment the count
	 * here, but it's not clear that always incrementing
	 * the count is more expensive than always testing a flag
	 * in memory.
	 *
	 * We keep the count in "md.packets_read", and use that for
	 * "ps_recv" if we can't get the statistics from the kernel.
	 * We do that because, if we *can* get the statistics from
	 * the kernel, we use "md.stat.ps_recv" and "md.stat.ps_drop"
	 * as running counts, as reading the statistics from the
	 * kernel resets the kernel statistics, and if we directly
	 * increment "md.stat.ps_recv" here, that means it will
	 * count packets *twice* on systems where we can get kernel
	 * statistics - once here, and once in pcap_stats_linux().
	 */
	handle->md.packets_read++;

	/* Call the user supplied callback function */
	callback(userdata, &pcap_header, bp);

	return 1;
}

static int
pcap_inject_linux(pcap_t *handle, const void *buf, size_t size)
{
	int ret;

#ifdef HAVE_PF_PACKET_SOCKETS
	if (!handle->md.sock_packet) {
		/* PF_PACKET socket */
		if (handle->md.ifindex == -1) {
			/*
			 * We don't support sending on the "any" device.
			 */
			strlcpy(handle->errbuf,
			    "Sending packets isn't supported on the \"any\" device",
			    PCAP_ERRBUF_SIZE);
			return (-1);
		}

		if (handle->md.cooked) {
			/*
			 * We don't support sending on the "any" device.
			 *
			 * XXX - how do you send on a bound cooked-mode
			 * socket?
			 * Is a "sendto()" required there?
			 */
			strlcpy(handle->errbuf,
			    "Sending packets isn't supported in cooked mode",
			    PCAP_ERRBUF_SIZE);
			return (-1);
		}
	}
#endif // endif

	ret = send(handle->fd, buf, size, 0);
	if (ret == -1) {
		snprintf(handle->errbuf, PCAP_ERRBUF_SIZE, "send: %s",
		    pcap_strerror(errno));
		return (-1);
	}
	return (ret);
}

/*
 *  Get the statistics for the given packet capture handle.
 *  Reports the number of dropped packets iff the kernel supports
 *  the PACKET_STATISTICS "getsockopt()" argument (2.4 and later
 *  kernels, and 2.2[.x] kernels with Alexey Kuznetzov's turbopacket
 *  patches); otherwise, that information isn't available, and we lie
 *  and report 0 as the count of dropped packets.
 */
static int
pcap_stats_linux(pcap_t *handle, struct pcap_stat *stats)
{
#ifdef HAVE_TPACKET_STATS
	struct tpacket_stats kstats;
	socklen_t len = sizeof (struct tpacket_stats);
#endif // endif

#ifdef HAVE_TPACKET_STATS
	/*
	 * Try to get the packet counts from the kernel.
	 */
	if (getsockopt(handle->fd, SOL_PACKET, PACKET_STATISTICS,
			&kstats, &len) > -1) {
		/*
		 * On systems where the PACKET_STATISTICS "getsockopt()"
		 * argument is supported on PF_PACKET sockets:
		 *
		 *	"ps_recv" counts only packets that *passed* the
		 *	filter, not packets that didn't pass the filter.
		 *	This includes packets later dropped because we
		 *	ran out of buffer space.
		 *
		 *	"ps_drop" counts packets dropped because we ran
		 *	out of buffer space.  It doesn't count packets
		 *	dropped by the interface driver.  It counts only
		 *	packets that passed the filter.
		 *
		 *	Both statistics include packets not yet read from
		 *	the kernel by libpcap, and thus not yet seen by
		 *	the application.
		 *
		 * In "linux/net/packet/af_packet.c", at least in the
		 * 2.4.9 kernel, "tp_packets" is incremented for every
		 * packet that passes the packet filter *and* is
		 * successfully queued on the socket; "tp_drops" is
		 * incremented for every packet dropped because there's
		 * not enough free space in the socket buffer.
		 *
		 * When the statistics are returned for a PACKET_STATISTICS
		 * "getsockopt()" call, "tp_drops" is added to "tp_packets",
		 * so that "tp_packets" counts all packets handed to
		 * the PF_PACKET socket, including packets dropped because
		 * there wasn't room on the socket buffer - but not
		 * including packets that didn't pass the filter.
		 *
		 * In the BSD BPF, the count of received packets is
		 * incremented for every packet handed to BPF, regardless
		 * of whether it passed the filter.
		 *
		 * We can't make "pcap_stats()" work the same on both
		 * platforms, but the best approximation is to return
		 * "tp_packets" as the count of packets and "tp_drops"
		 * as the count of drops.
		 *
		 * Keep a running total because each call to
		 *    getsockopt(handle->fd, SOL_PACKET, PACKET_STATISTICS, ....
		 * resets the counters to zero.
		 */
		handle->md.stat.ps_recv += kstats.tp_packets;
		handle->md.stat.ps_drop += kstats.tp_drops;
		*stats = handle->md.stat;
		return 0;
	}
	else
	{
		/*
		 * If the error was EOPNOTSUPP, fall through, so that
		 * if you build the library on a system with
		 * "struct tpacket_stats" and run it on a system
		 * that doesn't, it works as it does if the library
		 * is built on a system without "struct tpacket_stats".
		 */
		if (errno != EOPNOTSUPP) {
			snprintf(handle->errbuf, PCAP_ERRBUF_SIZE,
			    "pcap_stats: %s", pcap_strerror(errno));
			return -1;
		}
	}
#endif // endif
	/*
	 * On systems where the PACKET_STATISTICS "getsockopt()" argument
	 * is not supported on PF_PACKET sockets:
	 *
	 *	"ps_recv" counts only packets that *passed* the filter,
	 *	not packets that didn't pass the filter.  It does not
	 *	count packets dropped because we ran out of buffer
	 *	space.
	 *
	 *	"ps_drop" is not supported.
	 *
	 *	"ps_recv" doesn't include packets not yet read from
	 *	the kernel by libpcap.
	 *
	 * We maintain the count of packets processed by libpcap in
	 * "md.packets_read", for reasons described in the comment
	 * at the end of pcap_read_packet().  We have no idea how many
	 * packets were dropped.
	 */
	stats->ps_recv = handle->md.packets_read;
	stats->ps_drop = 0;
	return 0;
}

/*
 * Description string for the "any" device.
 */
static const char any_descr[] = "Pseudo-device that captures on all interfaces";

int
pcap_platform_finddevs(pcap_if_t **alldevsp, char *errbuf)
{
	if (pcap_add_if(alldevsp, "any", 0, any_descr, errbuf) < 0)
		return (-1);

#ifdef HAVE_DAG_API
	if (dag_platform_finddevs(alldevsp, errbuf) < 0)
		return (-1);
#endif /* HAVE_DAG_API */

#ifdef HAVE_SEPTEL_API
	if (septel_platform_finddevs(alldevsp, errbuf) < 0)
		return (-1);
#endif /* HAVE_SEPTEL_API */

#ifdef PCAP_SUPPORT_BT
	if (bt_platform_finddevs(alldevsp, errbuf) < 0)
		return (-1);
#endif // endif

#ifdef PCAP_SUPPORT_USB
	if (usb_platform_finddevs(alldevsp, errbuf) < 0)
		return (-1);
#endif // endif

#ifdef HAVE_TC_API
	if (TcFindAllDevs(alldevsp, errbuf) < 0)
		return (-1);
#endif // endif

	return (0);
}

/*
 *  Attach the given BPF code to the packet capture device.
 */
static int
pcap_setfilter_linux(pcap_t *handle, struct bpf_program *filter)
{
#ifdef SO_ATTACH_FILTER
	struct sock_fprog	fcode;
	int			can_filter_in_kernel;
	int			err = 0;
#endif // endif

	if (!handle)
		return -1;
	if (!filter) {
	        strncpy(handle->errbuf, "setfilter: No filter specified",
			PCAP_ERRBUF_SIZE);
		return -1;
	}

	/* Make our private copy of the filter */

	if (install_bpf_program(handle, filter) < 0)
		/* install_bpf_program() filled in errbuf */
		return -1;

	/*
	 * Run user level packet filter by default. Will be overriden if
	 * installing a kernel filter succeeds.
	 */
	handle->md.use_bpf = 0;

	/* Install kernel level filter if possible */

#ifdef SO_ATTACH_FILTER
#ifdef USHRT_MAX
	if (handle->fcode.bf_len > USHRT_MAX) {
		/*
		 * fcode.len is an unsigned short for current kernel.
		 * I have yet to see BPF-Code with that much
		 * instructions but still it is possible. So for the
		 * sake of correctness I added this check.
		 */
		fprintf(stderr, "Warning: Filter too complex for kernel\n");
		fcode.len = 0;
		fcode.filter = NULL;
		can_filter_in_kernel = 0;
	} else
#endif /* USHRT_MAX */
	{
		/*
		 * Oh joy, the Linux kernel uses struct sock_fprog instead
		 * of struct bpf_program and of course the length field is
		 * of different size. Pointed out by Sebastian
		 *
		 * Oh, and we also need to fix it up so that all "ret"
		 * instructions with non-zero operands have 65535 as the
		 * operand, and so that, if we're in cooked mode, all
		 * memory-reference instructions use special magic offsets
		 * in references to the link-layer header and assume that
		 * the link-layer payload begins at 0; "fix_program()"
		 * will do that.
		 */
		switch (fix_program(handle, &fcode)) {

		case -1:
		default:
			/*
			 * Fatal error; just quit.
			 * (The "default" case shouldn't happen; we
			 * return -1 for that reason.)
			 */
			return -1;

		case 0:
			/*
			 * The program performed checks that we can't make
			 * work in the kernel.
			 */
			can_filter_in_kernel = 0;
			break;

		case 1:
			/*
			 * We have a filter that'll work in the kernel.
			 */
			can_filter_in_kernel = 1;
			break;
		}
	}

	if (can_filter_in_kernel) {
		if ((err = set_kernel_filter(handle, &fcode)) == 0)
		{
			/* Installation succeded - using kernel filter. */
			handle->md.use_bpf = 1;
		}
		else if (err == -1)	/* Non-fatal error */
		{
			/*
			 * Print a warning if we weren't able to install
			 * the filter for a reason other than "this kernel
			 * isn't configured to support socket filters.
			 */
			if (errno != ENOPROTOOPT && errno != EOPNOTSUPP) {
				fprintf(stderr,
				    "Warning: Kernel filter failed: %s\n",
					pcap_strerror(errno));
			}
		}
	}

	/*
	 * If we're not using the kernel filter, get rid of any kernel
	 * filter that might've been there before, e.g. because the
	 * previous filter could work in the kernel, or because some other
	 * code attached a filter to the socket by some means other than
	 * calling "pcap_setfilter()".  Otherwise, the kernel filter may
	 * filter out packets that would pass the new userland filter.
	 */
	if (!handle->md.use_bpf)
		reset_kernel_filter(handle);

	/*
	 * Free up the copy of the filter that was made by "fix_program()".
	 */
	if (fcode.filter != NULL)
		free(fcode.filter);

	if (err == -2)
		/* Fatal error */
		return -1;
#endif /* SO_ATTACH_FILTER */

	return 0;
}

/*
 * Set direction flag: Which packets do we accept on a forwarding
 * single device? IN, OUT or both?
 */
static int
pcap_setdirection_linux(pcap_t *handle, pcap_direction_t d)
{
#ifdef HAVE_PF_PACKET_SOCKETS
	if (!handle->md.sock_packet) {
		handle->direction = d;
		return 0;
	}
#endif // endif
	/*
	 * We're not using PF_PACKET sockets, so we can't determine
	 * the direction of the packet.
	 */
	snprintf(handle->errbuf, PCAP_ERRBUF_SIZE,
	    "Setting direction is not supported on SOCK_PACKET sockets");
	return -1;
}

#ifdef HAVE_PF_PACKET_SOCKETS
/*
 * Map the PACKET_ value to a LINUX_SLL_ value; we
 * want the same numerical value to be used in
 * the link-layer header even if the numerical values
 * for the PACKET_ #defines change, so that programs
 * that look at the packet type field will always be
 * able to handle DLT_LINUX_SLL captures.
 */
static short int
map_packet_type_to_sll_type(short int sll_pkttype)
{
	switch (sll_pkttype) {

	case PACKET_HOST:
		return htons(LINUX_SLL_HOST);

	case PACKET_BROADCAST:
		return htons(LINUX_SLL_BROADCAST);

	case PACKET_MULTICAST:
		return  htons(LINUX_SLL_MULTICAST);

	case PACKET_OTHERHOST:
		return htons(LINUX_SLL_OTHERHOST);

	case PACKET_OUTGOING:
		return htons(LINUX_SLL_OUTGOING);

	default:
		return -1;
	}
}
#endif // endif

/*
 *  Linux uses the ARP hardware type to identify the type of an
 *  interface. pcap uses the DLT_xxx constants for this. This
 *  function takes a pointer to a "pcap_t", and an ARPHRD_xxx
 *  constant, as arguments, and sets "handle->linktype" to the
 *  appropriate DLT_XXX constant and sets "handle->offset" to
 *  the appropriate value (to make "handle->offset" plus link-layer
 *  header length be a multiple of 4, so that the link-layer payload
 *  will be aligned on a 4-byte boundary when capturing packets).
 *  (If the offset isn't set here, it'll be 0; add code as appropriate
 *  for cases where it shouldn't be 0.)
 *
 *  If "cooked_ok" is non-zero, we can use DLT_LINUX_SLL and capture
 *  in cooked mode; otherwise, we can't use cooked mode, so we have
 *  to pick some type that works in raw mode, or fail.
 *
 *  Sets the link type to -1 if unable to map the type.
 */
static void map_arphrd_to_dlt(pcap_t *handle, int arptype, int cooked_ok)
{
	switch (arptype) {

	case ARPHRD_ETHER:
		/*
		 * This is (presumably) a real Ethernet capture; give it a
		 * link-layer-type list with DLT_EN10MB and DLT_DOCSIS, so
		 * that an application can let you choose it, in case you're
		 * capturing DOCSIS traffic that a Cisco Cable Modem
		 * Termination System is putting out onto an Ethernet (it
		 * doesn't put an Ethernet header onto the wire, it puts raw
		 * DOCSIS frames out on the wire inside the low-level
		 * Ethernet framing).
		 *
		 * XXX - are there any sorts of "fake Ethernet" that have
		 * ARPHRD_ETHER but that *shouldn't offer DLT_DOCSIS as
		 * a Cisco CMTS won't put traffic onto it or get traffic
		 * bridged onto it?  ISDN is handled in "activate_new()",
		 * as we fall back on cooked mode there; are there any
		 * others?
		 */
		handle->dlt_list = (u_int *) malloc(sizeof(u_int) * 2);
		/*
		 * If that fails, just leave the list empty.
		 */
		if (handle->dlt_list != NULL) {
			handle->dlt_list[0] = DLT_EN10MB;
			handle->dlt_list[1] = DLT_DOCSIS;
			handle->dlt_count = 2;
		}
		/* FALLTHROUGH */

	case ARPHRD_METRICOM:
	case ARPHRD_LOOPBACK:
		handle->linktype = DLT_EN10MB;
		handle->offset = 2;
		break;

	case ARPHRD_EETHER:
		handle->linktype = DLT_EN3MB;
		break;

	case ARPHRD_AX25:
		handle->linktype = DLT_AX25_KISS;
		break;

	case ARPHRD_PRONET:
		handle->linktype = DLT_PRONET;
		break;

	case ARPHRD_CHAOS:
		handle->linktype = DLT_CHAOS;
		break;

#ifndef ARPHRD_IEEE802_TR
#define ARPHRD_IEEE802_TR 800	/* From Linux 2.4 */
#endif // endif
	case ARPHRD_IEEE802_TR:
	case ARPHRD_IEEE802:
		handle->linktype = DLT_IEEE802;
		handle->offset = 2;
		break;

	case ARPHRD_ARCNET:
		handle->linktype = DLT_ARCNET_LINUX;
		break;

#ifndef ARPHRD_FDDI	/* From Linux 2.2.13 */
#define ARPHRD_FDDI	774
#endif // endif
	case ARPHRD_FDDI:
		handle->linktype = DLT_FDDI;
		handle->offset = 3;
		break;

#ifndef ARPHRD_ATM
#define ARPHRD_ATM 19
#endif // endif
	case ARPHRD_ATM:
		/*
		 * The Classical IP implementation in ATM for Linux
		 * supports both what RFC 1483 calls "LLC Encapsulation",
		 * in which each packet has an LLC header, possibly
		 * with a SNAP header as well, prepended to it, and
		 * what RFC 1483 calls "VC Based Multiplexing", in which
		 * different virtual circuits carry different network
		 * layer protocols, and no header is prepended to packets.
		 *
		 * They both have an ARPHRD_ type of ARPHRD_ATM, so
		 * you can't use the ARPHRD_ type to find out whether
		 * captured packets will have an LLC header, and,
		 * while there's a socket ioctl to *set* the encapsulation
		 * type, there's no ioctl to *get* the encapsulation type.
		 *
		 * This means that
		 *
		 *	programs that dissect Linux Classical IP frames
		 *	would have to check for an LLC header and,
		 *	depending on whether they see one or not, dissect
		 *	the frame as LLC-encapsulated or as raw IP (I
		 *	don't know whether there's any traffic other than
		 *	IP that would show up on the socket, or whether
		 *	there's any support for IPv6 in the Linux
		 *	Classical IP code);
		 *
		 *	filter expressions would have to compile into
		 *	code that checks for an LLC header and does
		 *	the right thing.
		 *
		 * Both of those are a nuisance - and, at least on systems
		 * that support PF_PACKET sockets, we don't have to put
		 * up with those nuisances; instead, we can just capture
		 * in cooked mode.  That's what we'll do, if we can.
		 * Otherwise, we'll just fail.
		 */
		if (cooked_ok)
			handle->linktype = DLT_LINUX_SLL;
		else
			handle->linktype = -1;
		break;

#ifndef ARPHRD_IEEE80211  /* From Linux 2.4.6 */
#define ARPHRD_IEEE80211 801
#endif // endif
	case ARPHRD_IEEE80211:
		handle->linktype = DLT_IEEE802_11;
		break;

#ifndef ARPHRD_IEEE80211_PRISM  /* From Linux 2.4.18 */
#define ARPHRD_IEEE80211_PRISM 802
#endif // endif
	case ARPHRD_IEEE80211_PRISM:
		handle->linktype = DLT_PRISM_HEADER;
		break;

#ifndef ARPHRD_IEEE80211_RADIOTAP /* new */
#define ARPHRD_IEEE80211_RADIOTAP 803
#endif // endif
	case ARPHRD_IEEE80211_RADIOTAP:
		handle->linktype = DLT_IEEE802_11_RADIO;
		break;

	case ARPHRD_PPP:
		/*
		 * Some PPP code in the kernel supplies no link-layer
		 * header whatsoever to PF_PACKET sockets; other PPP
		 * code supplies PPP link-layer headers ("syncppp.c");
		 * some PPP code might supply random link-layer
		 * headers (PPP over ISDN - there's code in Ethereal,
		 * for example, to cope with PPP-over-ISDN captures
		 * with which the Ethereal developers have had to cope,
		 * heuristically trying to determine which of the
		 * oddball link-layer headers particular packets have).
		 *
		 * As such, we just punt, and run all PPP interfaces
		 * in cooked mode, if we can; otherwise, we just treat
		 * it as DLT_RAW, for now - if somebody needs to capture,
		 * on a 2.0[.x] kernel, on PPP devices that supply a
		 * link-layer header, they'll have to add code here to
		 * map to the appropriate DLT_ type (possibly adding a
		 * new DLT_ type, if necessary).
		 */
		if (cooked_ok)
			handle->linktype = DLT_LINUX_SLL;
		else {
			/*
			 * XXX - handle ISDN types here?  We can't fall
			 * back on cooked sockets, so we'd have to
			 * figure out from the device name what type of
			 * link-layer encapsulation it's using, and map
			 * that to an appropriate DLT_ value, meaning
			 * we'd map "isdnN" devices to DLT_RAW (they
			 * supply raw IP packets with no link-layer
			 * header) and "isdY" devices to a new DLT_I4L_IP
			 * type that has only an Ethernet packet type as
			 * a link-layer header.
			 *
			 * But sometimes we seem to get random crap
			 * in the link-layer header when capturing on
			 * ISDN devices....
			 */
			handle->linktype = DLT_RAW;
		}
		break;

#ifndef ARPHRD_CISCO
#define ARPHRD_CISCO 513 /* previously ARPHRD_HDLC */
#endif // endif
	case ARPHRD_CISCO:
		handle->linktype = DLT_C_HDLC;
		break;

	/* Not sure if this is correct for all tunnels, but it
	 * works for CIPE */
	case ARPHRD_TUNNEL:
#ifndef ARPHRD_SIT
#define ARPHRD_SIT 776	/* From Linux 2.2.13 */
#endif // endif
	case ARPHRD_SIT:
	case ARPHRD_CSLIP:
	case ARPHRD_SLIP6:
	case ARPHRD_CSLIP6:
	case ARPHRD_ADAPT:
	case ARPHRD_SLIP:
#ifndef ARPHRD_RAWHDLC
#define ARPHRD_RAWHDLC 518
#endif // endif
	case ARPHRD_RAWHDLC:
#ifndef ARPHRD_DLCI
#define ARPHRD_DLCI 15
#endif // endif
	case ARPHRD_DLCI:
		/*
		 * XXX - should some of those be mapped to DLT_LINUX_SLL
		 * instead?  Should we just map all of them to DLT_LINUX_SLL?
		 */
		handle->linktype = DLT_RAW;
		break;

#ifndef ARPHRD_FRAD
#define ARPHRD_FRAD 770
#endif // endif
	case ARPHRD_FRAD:
		handle->linktype = DLT_FRELAY;
		break;

	case ARPHRD_LOCALTLK:
		handle->linktype = DLT_LTALK;
		break;

#ifndef ARPHRD_FCPP
#define ARPHRD_FCPP	784
#endif // endif
	case ARPHRD_FCPP:
#ifndef ARPHRD_FCAL
#define ARPHRD_FCAL	785
#endif // endif
	case ARPHRD_FCAL:
#ifndef ARPHRD_FCPL
#define ARPHRD_FCPL	786
#endif // endif
	case ARPHRD_FCPL:
#ifndef ARPHRD_FCFABRIC
#define ARPHRD_FCFABRIC	787
#endif // endif
	case ARPHRD_FCFABRIC:
		/*
		 * We assume that those all mean RFC 2625 IP-over-
		 * Fibre Channel, with the RFC 2625 header at
		 * the beginning of the packet.
		 */
		handle->linktype = DLT_IP_OVER_FC;
		break;

#ifndef ARPHRD_IRDA
#define ARPHRD_IRDA	783
#endif // endif
	case ARPHRD_IRDA:
		/* Don't expect IP packet out of this interfaces... */
		handle->linktype = DLT_LINUX_IRDA;
		/* We need to save packet direction for IrDA decoding,
		 * so let's use "Linux-cooked" mode. Jean II */
		//handle->md.cooked = 1;
		break;

	/* ARPHRD_LAPD is unofficial and randomly allocated, if reallocation
	 * is needed, please report it to <daniele@orlandi.com> */
#ifndef ARPHRD_LAPD
#define ARPHRD_LAPD	8445
#endif // endif
	case ARPHRD_LAPD:
		/* Don't expect IP packet out of this interfaces... */
		handle->linktype = DLT_LINUX_LAPD;
		break;

#ifndef ARPHRD_NONE
#define ARPHRD_NONE	0xFFFE
#endif // endif
	case ARPHRD_NONE:
		/*
		 * No link-layer header; packets are just IP
		 * packets, so use DLT_RAW.
		 */
		handle->linktype = DLT_RAW;
		break;

	default:
		handle->linktype = -1;
		break;
	}
}

/* ===== Functions to interface to the newer kernels ================== */

/*
 * Try to open a packet socket using the new kernel PF_PACKET interface.
 * Returns 1 on success, 0 on an error that means the new interface isn't
 * present (so the old SOCK_PACKET interface should be tried), and a
 * PCAP_ERROR_ value on an error that means that the old mechanism won't
 * work either (so it shouldn't be tried).
 */
static int
activate_new(pcap_t *handle)
{
#ifdef HAVE_PF_PACKET_SOCKETS
	int			sock_fd = -1, arptype, val;
	int			err = 0;
	struct packet_mreq	mr;
	const char* device = handle->opt.source;

	/*
	 * Open a socket with protocol family packet. If a device is
	 * given we try to open it in raw mode otherwise we use
	 * the cooked interface.
	 */
	sock_fd = device ?
		socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))
	      : socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_ALL));

	if (sock_fd == -1) {
		snprintf(handle->errbuf, PCAP_ERRBUF_SIZE, "socket: %s",
			 pcap_strerror(errno) );
		return 0;	/* try old mechanism */
	}

	/* It seems the kernel supports the new interface. */
	handle->md.sock_packet = 0;

	/*
	 * Get the interface index of the loopback device.
	 * If the attempt fails, don't fail, just set the
	 * "md.lo_ifindex" to -1.
	 *
	 * XXX - can there be more than one device that loops
	 * packets back, i.e. devices other than "lo"?  If so,
	 * we'd need to find them all, and have an array of
	 * indices for them, and check all of them in
	 * "pcap_read_packet()".
	 */
	handle->md.lo_ifindex = iface_get_id(sock_fd, "lo", handle->errbuf);

	/*
	 * Default value for offset to align link-layer payload
	 * on a 4-byte boundary.
	 */
	handle->offset	 = 0;

	/*
	 * What kind of frames do we have to deal with? Fall back
	 * to cooked mode if we have an unknown interface type
	 * or a type we know doesn't work well in raw mode.
	 */
	if (device) {
		/* Assume for now we don't need cooked mode. */
		handle->md.cooked = 0;

		if (handle->opt.rfmon) {
			/*
			 * We were asked to turn on monitor mode.
			 * Do so before we get the link-layer type,
			 * because entering monitor mode could change
			 * the link-layer type.
			 */
			err = enter_rfmon_mode_wext(handle, sock_fd, device);
			if (err < 0) {
				/* Hard failure */
				close(sock_fd);
				return err;
			}
			if (err == 0) {
				/*
				 * Nothing worked for turning monitor mode
				 * on.
				 */
				close(sock_fd);
				return PCAP_ERROR_RFMON_NOTSUP;
			}
		}
		arptype	= iface_get_arptype(sock_fd, device, handle->errbuf);
		if (arptype < 0) {
			close(sock_fd);
			return arptype;
		}
		map_arphrd_to_dlt(handle, arptype, 1);
		if (handle->linktype == -1 ||
		    handle->linktype == DLT_LINUX_SLL ||
		    handle->linktype == DLT_LINUX_IRDA ||
		    handle->linktype == DLT_LINUX_LAPD ||
		    (handle->linktype == DLT_EN10MB &&
		     (strncmp("isdn", device, 4) == 0 ||
		      strncmp("isdY", device, 4) == 0))) {
			/*
			 * Unknown interface type (-1), or a
			 * device we explicitly chose to run
			 * in cooked mode (e.g., PPP devices),
			 * or an ISDN device (whose link-layer
			 * type we can only determine by using
			 * APIs that may be different on different
			 * kernels) - reopen in cooked mode.
			 */
			if (close(sock_fd) == -1) {
				snprintf(handle->errbuf, PCAP_ERRBUF_SIZE,
					 "close: %s", pcap_strerror(errno));
				return PCAP_ERROR;
			}
			sock_fd = socket(PF_PACKET, SOCK_DGRAM,
			    htons(ETH_P_ALL));
			if (sock_fd == -1) {
				snprintf(handle->errbuf, PCAP_ERRBUF_SIZE,
				    "socket: %s", pcap_strerror(errno));
				return PCAP_ERROR;
			}
			handle->md.cooked = 1;

			/*
			 * Get rid of any link-layer type list
			 * we allocated - this only supports cooked
			 * capture.
			 */
			if (handle->dlt_list != NULL) {
				free(handle->dlt_list);
				handle->dlt_list = NULL;
				handle->dlt_count = 0;
			}

			if (handle->linktype == -1) {
				/*
				 * Warn that we're falling back on
				 * cooked mode; we may want to
				 * update "map_arphrd_to_dlt()"
				 * to handle the new type.
				 */
				snprintf(handle->errbuf, PCAP_ERRBUF_SIZE,
					"arptype %d not "
					"supported by libpcap - "
					"falling back to cooked "
					"socket",
					arptype);
			}

			/*
			 * IrDA capture is not a real "cooked" capture,
			 * it's IrLAP frames, not IP packets.  The
			 * same applies to LAPD capture.
			 */
			if (handle->linktype != DLT_LINUX_IRDA &&
			    handle->linktype != DLT_LINUX_LAPD)
				handle->linktype = DLT_LINUX_SLL;
		}

		handle->md.ifindex = iface_get_id(sock_fd, device,
		    handle->errbuf);
		if (handle->md.ifindex == -1) {
			close(sock_fd);
			return PCAP_ERROR;
		}

		if ((err = iface_bind(sock_fd, handle->md.ifindex,
		    handle->errbuf)) != 1) {
		    	close(sock_fd);
			if (err < 0)
				return err;
			else
				return 0;	/* try old mechanism */
		}
	} else {
		/*
		 * This is cooked mode.
		 */
		handle->md.cooked = 1;
		handle->linktype = DLT_LINUX_SLL;

		/*
		 * We're not bound to a device.
		 * XXX - true?  Or true only if we're using
		 * the "any" device?
		 * For now, we're using this as an indication
		 * that we can't transmit; stop doing that only
		 * if we figure out how to transmit in cooked
		 * mode.
		 */
		handle->md.ifindex = -1;
	}

	/*
	 * Select promiscuous mode on if "promisc" is set.
	 *
	 * Do not turn allmulti mode on if we don't select
	 * promiscuous mode - on some devices (e.g., Orinoco
	 * wireless interfaces), allmulti mode isn't supported
	 * and the driver implements it by turning promiscuous
	 * mode on, and that screws up the operation of the
	 * card as a normal networking interface, and on no
	 * other platform I know of does starting a non-
	 * promiscuous capture affect which multicast packets
	 * are received by the interface.
	 */

	/*
	 * Hmm, how can we set promiscuous mode on all interfaces?
	 * I am not sure if that is possible at all.
	 */

	if (device && handle->opt.promisc) {
		memset(&mr, 0, sizeof(mr));
		mr.mr_ifindex = handle->md.ifindex;
		mr.mr_type    = PACKET_MR_PROMISC;
		if (setsockopt(sock_fd, SOL_PACKET, PACKET_ADD_MEMBERSHIP,
		    &mr, sizeof(mr)) == -1) {
			snprintf(handle->errbuf, PCAP_ERRBUF_SIZE,
				"setsockopt: %s", pcap_strerror(errno));
			close(sock_fd);
			return PCAP_ERROR;
		}
	}

	/* Enable auxillary data if supported and reserve room for
	 * reconstructing VLAN headers. */
#ifdef HAVE_PACKET_AUXDATA
	val = 1;
	if (setsockopt(sock_fd, SOL_PACKET, PACKET_AUXDATA, &val,
		       sizeof(val)) == -1 && errno != ENOPROTOOPT) {
		snprintf(handle->errbuf, PCAP_ERRBUF_SIZE,
			 "setsockopt: %s", pcap_strerror(errno));
		close(sock_fd);
		return PCAP_ERROR;
	}
	handle->offset += VLAN_TAG_LEN;
#endif /* HAVE_PACKET_AUXDATA */

	/*
	 * This is a 2.2[.x] or later kernel (we know that
	 * because we're not using a SOCK_PACKET socket -
	 * PF_PACKET is supported only in 2.2 and later
	 * kernels).
	 *
	 * We can safely pass "recvfrom()" a byte count
	 * based on the snapshot length.
	 *
	 * If we're in cooked mode, make the snapshot length
	 * large enough to hold a "cooked mode" header plus
	 * 1 byte of packet data (so we don't pass a byte
	 * count of 0 to "recvfrom()").
	 */
	if (handle->md.cooked) {
		if (handle->snapshot < SLL_HDR_LEN + 1)
			handle->snapshot = SLL_HDR_LEN + 1;
	}
	handle->bufsize = handle->snapshot;

	/* Save the socket FD in the pcap structure */
	handle->fd = sock_fd;

	return 1;
#else
	strncpy(ebuf,
		"New packet capturing interface not supported by build "
		"environment", PCAP_ERRBUF_SIZE);
	return 0;
#endif // endif
}

static int
activate_mmap(pcap_t *handle)
{
#ifdef HAVE_PACKET_RING
	int ret;

	if (handle->opt.buffer_size == 0) {
		/* by default request 2M for the ring buffer */
		handle->opt.buffer_size = 2*1024*1024;
	}
	ret = prepare_tpacket_socket(handle);
	if (ret == 0)
		return ret;
	ret = create_ring(handle);
	if (ret == 0)
		return ret;

	/* override some defaults and inherit the other fields from
	 * activate_new
	 * handle->offset is used to get the current position into the rx ring
	 * handle->cc is used to store the ring size */
	handle->read_op = pcap_read_linux_mmap;
	handle->cleanup_op = pcap_cleanup_linux_mmap;
	handle->setfilter_op = pcap_setfilter_linux_mmap;
	handle->setnonblock_op = pcap_setnonblock_mmap;
	handle->getnonblock_op = pcap_getnonblock_mmap;
	handle->selectable_fd = handle->fd;
	return 1;
#else /* HAVE_PACKET_RING */
	return 0;
#endif /* HAVE_PACKET_RING */
}

#ifdef HAVE_PACKET_RING
static int
prepare_tpacket_socket(pcap_t *handle)
{
#ifdef HAVE_TPACKET2
	socklen_t len;
	int val;
#endif // endif

	handle->md.tp_version = TPACKET_V1;
	handle->md.tp_hdrlen = sizeof(struct tpacket_hdr);

#ifdef HAVE_TPACKET2
	/* Probe whether kernel supports TPACKET_V2 */
	val = TPACKET_V2;
	len = sizeof(val);
	if (getsockopt(handle->fd, SOL_PACKET, PACKET_HDRLEN, &val, &len) < 0) {
		if (errno == ENOPROTOOPT)
			return 1;
		snprintf(handle->errbuf, PCAP_ERRBUF_SIZE,
			 "can't get TPACKET_V2 header len on socket %d: %d-%s",
			 handle->fd, errno, pcap_strerror(errno));
		return 0;
	}
	handle->md.tp_hdrlen = val;

	val = TPACKET_V2;
	if (setsockopt(handle->fd, SOL_PACKET, PACKET_VERSION, &val,
		       sizeof(val)) < 0) {
		snprintf(handle->errbuf, PCAP_ERRBUF_SIZE,
			 "can't activate TPACKET_V2 on socket %d: %d-%s",
			 handle->fd, errno, pcap_strerror(errno));
		return 0;
	}
	handle->md.tp_version = TPACKET_V2;

	/* Reserve space for VLAN tag reconstruction */
	val = VLAN_TAG_LEN;
	if (setsockopt(handle->fd, SOL_PACKET, PACKET_RESERVE, &val,
		       sizeof(val)) < 0) {
		snprintf(handle->errbuf, PCAP_ERRBUF_SIZE,
			 "can't set up reserve on socket %d: %d-%s",
			 handle->fd, errno, pcap_strerror(errno));
		return 0;
	}

#endif /* HAVE_TPACKET2 */
	return 1;
}

static void
compute_ring_block(int frame_size, unsigned *block_size, unsigned *frames_per_block)
{
	/* compute the minumum block size that will handle this frame.
	 * The block has to be page size aligned.
	 * The max block size allowed by the kernel is arch-dependent and
	 * it's not explicitly checked here. */
	*block_size = getpagesize();
	while (*block_size < frame_size)
		*block_size <<= 1;

	*frames_per_block = *block_size/frame_size;
}

static int
create_ring(pcap_t *handle)
{
	unsigned i, j, ringsize, frames_per_block;
	struct tpacket_req req;

	/* Note that with large snapshot (say 64K) only a few frames
	 * will be available in the ring even with pretty large ring size
	 * (and a lot of memory will be unused).
	 * The snap len should be carefully chosen to achive best
	 * performance */
	req.tp_frame_size = TPACKET_ALIGN(handle->snapshot +
					  TPACKET_ALIGN(handle->md.tp_hdrlen) +
					  sizeof(struct sockaddr_ll));
	req.tp_frame_nr = handle->opt.buffer_size/req.tp_frame_size;
	compute_ring_block(req.tp_frame_size, &req.tp_block_size, &frames_per_block);
	req.tp_block_nr = req.tp_frame_nr / frames_per_block;

	/* req.tp_frame_nr is requested to match frames_per_block*req.tp_block_nr */
	req.tp_frame_nr = req.tp_block_nr * frames_per_block;

	/* ask the kernel to create the ring */
retry:
	if (setsockopt(handle->fd, SOL_PACKET, PACKET_RX_RING,
					(void *) &req, sizeof(req))) {
		/* try to reduce requested ring size to prevent memory failure */
		if ((errno == ENOMEM) && (req.tp_block_nr > 1)) {
			req.tp_frame_nr >>= 1;
			req.tp_block_nr = req.tp_frame_nr/frames_per_block;
			goto retry;
		}
		snprintf(handle->errbuf, PCAP_ERRBUF_SIZE, "can't create rx ring on "
				"packet socket %d: %d-%s", handle->fd, errno,
				pcap_strerror(errno));
		return 0;
	}

	/* memory map the rx ring */
	ringsize = req.tp_block_nr * req.tp_block_size;
	handle->bp = mmap(0, ringsize, PROT_READ| PROT_WRITE, MAP_SHARED,
					handle->fd, 0);
	if (handle->bp == MAP_FAILED) {
		snprintf(handle->errbuf, PCAP_ERRBUF_SIZE, "can't mmap rx ring: %d-%s",
			errno, pcap_strerror(errno));

		/* clear the allocated ring on error*/
		destroy_ring(handle);
		return 0;
	}

	/* allocate a ring for each frame header pointer*/
	handle->cc = req.tp_frame_nr;
	handle->buffer = malloc(handle->cc * sizeof(union thdr *));
	if (!handle->buffer) {
		destroy_ring(handle);
		return 0;
	}

	/* fill the header ring with proper frame ptr*/
	handle->offset = 0;
	for (i=0; i<req.tp_block_nr; ++i) {
		void *base = &handle->bp[i*req.tp_block_size];
		for (j=0; j<frames_per_block; ++j, ++handle->offset) {
			RING_GET_FRAME(handle) = base;
			base += req.tp_frame_size;
		}
	}

	handle->bufsize = req.tp_frame_size;
	handle->offset = 0;
	return 1;
}

/* free all ring related resources*/
static void
destroy_ring(pcap_t *handle)
{
	/* tell the kernel to destroy the ring*/
	struct tpacket_req req;
	memset(&req, 0, sizeof(req));
	setsockopt(handle->fd, SOL_PACKET, PACKET_RX_RING,
				(void *) &req, sizeof(req));

	/* if ring is mapped, unmap it*/
	if (handle->bp) {
		/* need to re-compute the ring size */
		unsigned frames_per_block, block_size;
		compute_ring_block(handle->bufsize, &block_size, &frames_per_block);

		/* do not perform sanity check here: we can't recover any error */
		munmap(handle->bp, block_size * handle->cc / frames_per_block);
		handle->bp = 0;
	}
}

static void
pcap_cleanup_linux_mmap( pcap_t *handle )
{
	destroy_ring(handle);
	pcap_cleanup_linux(handle);
}

static int
pcap_getnonblock_mmap(pcap_t *p, char *errbuf)
{
	/* use negative value of timeout to indicate non blocking ops */
	return (p->md.timeout<0);
}

static int
pcap_setnonblock_mmap(pcap_t *p, int nonblock, char *errbuf)
{
	/* map each value to the corresponding 2's complement, to
	 * preserve the timeout value provided with pcap_set_timeout */
	if (nonblock) {
		if (p->md.timeout > 0)
			p->md.timeout = p->md.timeout*-1 - 1;
	} else
		if (p->md.timeout < 0)
			p->md.timeout = (p->md.timeout+1)*-1;
	return 0;
}

static inline union thdr *
pcap_get_ring_frame(pcap_t *handle, int status)
{
	union thdr h;

	h.raw = RING_GET_FRAME(handle);
	switch (handle->md.tp_version) {
	case TPACKET_V1:
		if (status != (h.h1->tp_status ? TP_STATUS_USER :
						TP_STATUS_KERNEL))
			return NULL;
		break;
#ifdef HAVE_TPACKET2
	case TPACKET_V2:
		if (status != (h.h2->tp_status ? TP_STATUS_USER :
						TP_STATUS_KERNEL))
			return NULL;
		break;
#endif // endif
	}
	return h.raw;
}

static int
pcap_read_linux_mmap(pcap_t *handle, int max_packets, pcap_handler callback,
		u_char *user)
{
	int pkts = 0;

	/* wait for frames availability.*/
	if ((handle->md.timeout >= 0) &&
	    !pcap_get_ring_frame(handle, TP_STATUS_USER)) {
		struct pollfd pollinfo;
		int ret;

		pollinfo.fd = handle->fd;
		pollinfo.events = POLLIN;

		do {
			/* poll() requires a negative timeout to wait forever */
			ret = poll(&pollinfo, 1, (handle->md.timeout > 0)?
			 			handle->md.timeout: -1);
			if ((ret < 0) && (errno != EINTR)) {
				snprintf(handle->errbuf, PCAP_ERRBUF_SIZE,
					 "can't poll on packet socket fd %d: %d-%s",
					handle->fd, errno, pcap_strerror(errno));
				return -1;
			}
			/* check for break loop condition on interrupted syscall*/
			if (handle->break_loop) {
				handle->break_loop = 0;
				return -2;
			}
		} while (ret < 0);
	}

	/* non-positive values of max_packets are used to require all
	 * packets currently available in the ring */
	while ((pkts < max_packets) || (max_packets <= 0)) {
		int run_bpf;
		struct sockaddr_ll *sll;
		struct pcap_pkthdr pcaphdr;
		unsigned char *bp;
		union thdr h;
		unsigned int tp_len;
		unsigned int tp_mac;
		unsigned int tp_snaplen;
		unsigned int tp_sec;
		unsigned int tp_usec;

		h.raw = pcap_get_ring_frame(handle, TP_STATUS_USER);
		if (!h.raw)
			break;

		switch (handle->md.tp_version) {
		case TPACKET_V1:
			tp_len	   = h.h1->tp_len;
			tp_mac	   = h.h1->tp_mac;
			tp_snaplen = h.h1->tp_snaplen;
			tp_sec	   = h.h1->tp_sec;
			tp_usec	   = h.h1->tp_usec;
			break;
#ifdef HAVE_TPACKET2
		case TPACKET_V2:
			tp_len	   = h.h2->tp_len;
			tp_mac	   = h.h2->tp_mac;
			tp_snaplen = h.h2->tp_snaplen;
			tp_sec	   = h.h2->tp_sec;
			tp_usec	   = h.h2->tp_nsec / 1000;
			break;
#endif // endif
		default:
			snprintf(handle->errbuf, PCAP_ERRBUF_SIZE,
				"unsupported tpacket version %d",
				handle->md.tp_version);
			return -1;
		}
		/* perform sanity check on internal offset. */
		if (tp_mac + tp_snaplen > handle->bufsize) {
			snprintf(handle->errbuf, PCAP_ERRBUF_SIZE,
				"corrupted frame on kernel ring mac "
				"offset %d + caplen %d > frame len %d",
				tp_mac, tp_snaplen, handle->bufsize);
			return -1;
		}

		/* run filter on received packet
		 * If the kernel filtering is enabled we need to run the
		 * filter until all the frames present into the ring
		 * at filter creation time are processed.
		 * In such case md.use_bpf is used as a counter for the
		 * packet we need to filter.
		 * Note: alternatively it could be possible to stop applying
		 * the filter when the ring became empty, but it can possibly
		 * happen a lot later... */
		bp = (unsigned char*)h.raw + tp_mac;
		run_bpf = (!handle->md.use_bpf) ||
			((handle->md.use_bpf>1) && handle->md.use_bpf--);
		if (run_bpf && handle->fcode.bf_insns &&
				(bpf_filter(handle->fcode.bf_insns, bp,
					tp_len, tp_snaplen) == 0))
			goto skip;

		/* check direction and interface index */
		sll = (void *)h.raw + TPACKET_ALIGN(handle->md.tp_hdrlen);
		if ((sll->sll_ifindex == handle->md.lo_ifindex) &&
					(sll->sll_pkttype == PACKET_OUTGOING))
			goto skip;

		/* get required packet info from ring header */
		pcaphdr.ts.tv_sec = tp_sec;
		pcaphdr.ts.tv_usec = tp_usec;
		pcaphdr.caplen = tp_snaplen;
		pcaphdr.len = tp_len;

		/* if required build in place the sll header*/
		if (handle->md.cooked) {
			struct sll_header *hdrp;

			/*
			 * The kernel should have left us with enough
			 * space for an sll header; back up the packet
			 * data pointer into that space, as that'll be
			 * the beginning of the packet we pass to the
			 * callback.
			 */
			bp -= SLL_HDR_LEN;

			/*
			 * Let's make sure that's past the end of
			 * the tpacket header, i.e. >=
			 * ((u_char *)thdr + TPACKET_HDRLEN), so we
			 * don't step on the header when we construct
			 * the sll header.
			 */
			if (bp < (u_char *)h.raw +
					   TPACKET_ALIGN(handle->md.tp_hdrlen) +
					   sizeof(struct sockaddr_ll)) {
				snprintf(handle->errbuf, PCAP_ERRBUF_SIZE,
					"cooked-mode frame doesn't have room for sll header");
				return -1;
			}

			/*
			 * OK, that worked; construct the sll header.
			 */
			hdrp = (struct sll_header *)bp;
			hdrp->sll_pkttype = map_packet_type_to_sll_type(
							sll->sll_pkttype);
			hdrp->sll_hatype = htons(sll->sll_hatype);
			hdrp->sll_halen = htons(sll->sll_halen);
			memcpy(hdrp->sll_addr, sll->sll_addr, SLL_ADDRLEN);
			hdrp->sll_protocol = sll->sll_protocol;

			/* update packet len */
			pcaphdr.caplen += SLL_HDR_LEN;
			pcaphdr.len += SLL_HDR_LEN;
		}

#ifdef HAVE_TPACKET2
		if (handle->md.tp_version == TPACKET_V2 && h.h2->tp_vlan_tci &&
		    tp_snaplen >= 2 * ETH_ALEN) {
			struct vlan_tag *tag;

			bp -= VLAN_TAG_LEN;
			memmove(bp, bp + VLAN_TAG_LEN, 2 * ETH_ALEN);

			tag = (struct vlan_tag *)(bp + 2 * ETH_ALEN);
			tag->vlan_tpid = htons(ETH_P_8021Q);
			tag->vlan_tci = htons(h.h2->tp_vlan_tci);

			pcaphdr.caplen += VLAN_TAG_LEN;
			pcaphdr.len += VLAN_TAG_LEN;
		}
#endif // endif

		/* pass the packet to the user */
		pkts++;
		callback(user, &pcaphdr, bp);
		handle->md.packets_read++;

skip:
		/* next packet */
		switch (handle->md.tp_version) {
		case TPACKET_V1:
			h.h1->tp_status = TP_STATUS_KERNEL;
			break;
#ifdef HAVE_TPACKET2
		case TPACKET_V2:
			h.h2->tp_status = TP_STATUS_KERNEL;
			break;
#endif // endif
		}
		if (++handle->offset >= handle->cc)
			handle->offset = 0;

		/* check for break loop condition*/
		if (handle->break_loop) {
			handle->break_loop = 0;
			return -2;
		}
	}
	return pkts;
}

static int
pcap_setfilter_linux_mmap(pcap_t *handle, struct bpf_program *filter)
{
	int n, offset;
	int ret = pcap_setfilter_linux(handle, filter);
	if (ret < 0)
		return ret;

	/* if the kernel filter is enabled, we need to apply the filter on
	 * all packets present into the ring. Get an upper bound of their number
	 */
	if (!handle->md.use_bpf)
		return ret;

	/* walk the ring backward and count the free slot */
	offset = handle->offset;
	if (--handle->offset < 0)
		handle->offset = handle->cc - 1;
	for (n=0; n < handle->cc; ++n) {
		if (--handle->offset < 0)
			handle->offset = handle->cc - 1;
		if (!pcap_get_ring_frame(handle, TP_STATUS_KERNEL))
			break;
	}

	/* be careful to not change current ring position */
	handle->offset = offset;

	/* store the number of packets currently present in the ring */
	handle->md.use_bpf = 1 + (handle->cc - n);
	return ret;
}

#endif /* HAVE_PACKET_RING */

#ifdef HAVE_PF_PACKET_SOCKETS
/*
 *  Return the index of the given device name. Fill ebuf and return
 *  -1 on failure.
 */
static int
iface_get_id(int fd, const char *device, char *ebuf)
{
	struct ifreq	ifr;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, device, sizeof(ifr.ifr_name));

	if (ioctl(fd, SIOCGIFINDEX, &ifr) == -1) {
		snprintf(ebuf, PCAP_ERRBUF_SIZE,
			 "SIOCGIFINDEX: %s", pcap_strerror(errno));
		return -1;
	}

	return ifr.ifr_ifindex;
}

/*
 *  Bind the socket associated with FD to the given device.
 *  Return 1 on success, 0 if we should try a SOCK_PACKET socket,
 *  or a PCAP_ERROR_ value on a hard error.
 */
static int
iface_bind(int fd, int ifindex, char *ebuf)
{
	struct sockaddr_ll	sll;
	int			err;
	socklen_t		errlen = sizeof(err);

	memset(&sll, 0, sizeof(sll));
	sll.sll_family		= AF_PACKET;
	sll.sll_ifindex		= ifindex;
	sll.sll_protocol	= htons(ETH_P_ALL);

	if (bind(fd, (struct sockaddr *) &sll, sizeof(sll)) == -1) {
		if (errno == ENETDOWN) {
			/*
			 * Return a "network down" indication, so that
			 * the application can report that rather than
			 * saying we had a mysterious failure and
			 * suggest that they report a problem to the
			 * libpcap developers.
			 */
			return PCAP_ERROR_IFACE_NOT_UP;
		} else {
			snprintf(ebuf, PCAP_ERRBUF_SIZE,
				 "bind: %s", pcap_strerror(errno));
			return PCAP_ERROR;
		}
	}

	/* Any pending errors, e.g., network is down? */

	if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &errlen) == -1) {
		snprintf(ebuf, PCAP_ERRBUF_SIZE,
			"getsockopt: %s", pcap_strerror(errno));
		return 0;
	}

	if (err == ENETDOWN) {
		/*
		 * Return a "network down" indication, so that
		 * the application can report that rather than
		 * saying we had a mysterious failure and
		 * suggest that they report a problem to the
		 * libpcap developers.
		 */
		return PCAP_ERROR_IFACE_NOT_UP;
	} else if (err > 0) {
		snprintf(ebuf, PCAP_ERRBUF_SIZE,
			"bind: %s", pcap_strerror(err));
		return 0;
	}

	return 1;
}

/*
 * Check whether the device supports the Wireless Extensions.
 * Returns 1 if it does, 0 if it doesn't, PCAP_ERROR_NO_SUCH_DEVICE
 * if the device doesn't even exist.
 */
static int
has_wext(int sock_fd, const char *device, char *ebuf)
{
#ifdef IW_MODE_MONITOR
	struct iwreq ireq;

	strncpy(ireq.ifr_ifrn.ifrn_name, device,
	    sizeof ireq.ifr_ifrn.ifrn_name);
	ireq.ifr_ifrn.ifrn_name[sizeof ireq.ifr_ifrn.ifrn_name - 1] = 0;
	if (ioctl(sock_fd, SIOCGIWNAME, &ireq) >= 0)
		return 1;	/* yes */
	snprintf(ebuf, PCAP_ERRBUF_SIZE,
	    "%s: SIOCGIWPRIV: %s", device, pcap_strerror(errno));
	if (errno == ENODEV)
		return PCAP_ERROR_NO_SUCH_DEVICE;
#endif // endif
	return 0;
}

/*
 * Per me si va ne la citta dolente,
 * Per me si va ne l'etterno dolore,
 *	...
 * Lasciate ogne speranza, voi ch'intrate.
 *
 * XXX - airmon-ng does special stuff with the Orinoco driver and the
 * wlan-ng driver.
 */
typedef enum {
	MONITOR_WEXT,
	MONITOR_HOSTAP,
	MONITOR_PRISM,
	MONITOR_PRISM54,
	MONITOR_ACX100,
	MONITOR_RT2500,
	MONITOR_RT2570,
	MONITOR_RT73,
	MONITOR_RTL8XXX
} monitor_type;

/*
 * Use the Wireless Extensions, if we have them, to try to turn monitor mode
 * on if it's not already on.
 *
 * Returns 1 on success, 0 if we don't support the Wireless Extensions
 * on this device, or a PCAP_ERROR_ value if we do support them but
 * we weren't able to turn monitor mode on.
 */
static int
enter_rfmon_mode_wext(pcap_t *handle, int sock_fd, const char *device)
{
#ifdef IW_MODE_MONITOR
	/*
	 * XXX - at least some adapters require non-Wireless Extensions
	 * mechanisms to turn monitor mode on.
	 *
	 * Atheros cards might require that a separate "monitor virtual access
	 * point" be created, with later versions of the madwifi driver.
	 * airmon-ng does "wlanconfig ath create wlandev {if} wlanmode
	 * monitor -bssid", which apparently spits out a line "athN"
	 * where "athN" is the monitor mode device.  To leave monitor
	 * mode, it destroys the monitor mode device.
	 *
	 * Some Intel Centrino adapters might require private ioctls to get
	 * radio headers; the ipw2200 and ipw3945 drivers allow you to
	 * configure a separate "rtapN" interface to capture in monitor
	 * mode without preventing the adapter from operating normally.
	 * (airmon-ng doesn't appear to use that, though.)
	 *
	 * It would be Truly Wonderful if mac80211 and nl80211 cleaned this
	 * up, and if all drivers were converted to mac80211 drivers.
	 *
	 * If interface {if} is a mac80211 driver, the file
	 * /sys/class/net/{if}/phy80211 is a symlink to
	 * /sys/class/ieee80211/{phydev}, for some {phydev}.
	 *
	 * On Fedora 9, with a 2.6.26.3-29 kernel, my Zydas stick, at
	 * least, has a "wmaster0" device and a "wlan0" device; the
	 * latter is the one with the IP address.  Both show up in
	 * "tcpdump -D" output.  Capturing on the wmaster0 device
	 * captures with 802.11 headers.
	 *
	 * airmon-ng searches through /sys/class/net for devices named
	 * monN, starting with mon0; as soon as one *doesn't* exist,
	 * it chooses that as the monitor device name.  If the "iw"
	 * command exists, it does "iw dev {if} interface add {monif}
	 * type monitor", where {monif} is the monitor device.  It
	 * then (sigh) sleeps .1 second, and then configures the
	 * device up.  Otherwise, if /sys/class/ieee80211/{phydev}/add_iface
	 * is a file, it writes {mondev}, without a newline, to that file,
	 * and again (sigh) sleeps .1 second, and then iwconfig's that
	 * device into monitor mode and configures it up.  Otherwise,
	 * you can't do monitor mode.
	 *
	 * All these devices are "glued" together by having the
	 * /sys/class/net/{device}/phy80211 links pointing to the same
	 * place, so, given a wmaster, wlan, or mon device, you can
	 * find the other devices by looking for devices with
	 * the same phy80211 link.
	 *
	 * To turn monitor mode off, delete the monitor interface,
	 * either with "iw dev {monif} interface del" or by sending
	 * {monif}, with no NL, down /sys/class/ieee80211/{phydev}/remove_iface
	 *
	 * Note: if you try to create a monitor device named "monN", and
	 * there's already a "monN" device, it fails, as least with
	 * the netlink interface (which is what iw uses), with a return
	 * value of -ENFILE.  (Return values are negative errnos.)  We
	 * could probably use that to find an unused device.
	 */
	int err;
	struct iwreq ireq;
	struct iw_priv_args *priv;
	monitor_type montype;
	int i;
	__u32 cmd;
	int args[2];
	int channel;

	/*
	 * Does this device *support* the Wireless Extensions?
	 */
	err = has_wext(sock_fd, device, handle->errbuf);
	if (err <= 0)
		return err;	/* either it doesn't or the device doesn't even exist */
	/*
	 * Try to get all the Wireless Extensions private ioctls
	 * supported by this device.
	 *
	 * First, get the size of the buffer we need, by supplying no
	 * buffer and a length of 0.  If the device supports private
	 * ioctls, it should return E2BIG, with ireq.u.data.length set
	 * to the length we need.  If it doesn't support them, it should
	 * return EOPNOTSUPP.
	 */
	memset(&ireq, 0, sizeof ireq);
	strncpy(ireq.ifr_ifrn.ifrn_name, device,
	    sizeof ireq.ifr_ifrn.ifrn_name);
	ireq.ifr_ifrn.ifrn_name[sizeof ireq.ifr_ifrn.ifrn_name - 1] = 0;
	ireq.u.data.pointer = args;
	ireq.u.data.length = 0;
	ireq.u.data.flags = 0;
	if (ioctl(sock_fd, SIOCGIWPRIV, &ireq) != -1) {
		snprintf(handle->errbuf, PCAP_ERRBUF_SIZE,
		    "%s: SIOCGIWPRIV with a zero-length buffer didn't fail!",
		    device);
		return PCAP_ERROR;
	}
	if (errno == EOPNOTSUPP) {
		/*
		 * No private ioctls, so we assume that there's only one
		 * DLT_ for monitor mode.
		 */
		return 0;
	}
	if (errno != E2BIG) {
		/*
		 * Failed.
		 */
		snprintf(handle->errbuf, PCAP_ERRBUF_SIZE,
		    "%s: SIOCGIWPRIV: %s", device, pcap_strerror(errno));
		return PCAP_ERROR;
	}
	priv = malloc(ireq.u.data.length * sizeof (struct iw_priv_args));
	if (priv == NULL) {
		snprintf(handle->errbuf, PCAP_ERRBUF_SIZE,
			 "malloc: %s", pcap_strerror(errno));
		return PCAP_ERROR;
	}
	ireq.u.data.pointer = priv;
	if (ioctl(sock_fd, SIOCGIWPRIV, &ireq) == -1) {
		snprintf(handle->errbuf, PCAP_ERRBUF_SIZE,
		    "%s: SIOCGIWPRIV: %s", device, pcap_strerror(errno));
		free(priv);
		return PCAP_ERROR;
	}

	/*
	 * Look for private ioctls to turn monitor mode on or, if
	 * monitor mode is on, to set the header type.
	 */
	montype = MONITOR_WEXT;
	cmd = 0;
	for (i = 0; i < ireq.u.data.length; i++) {
		if (strcmp(priv[i].name, "monitor_type") == 0) {
			/*
			 * Hostap driver, use this one.
			 * Set monitor mode first.
			 * You can set it to 0 to get DLT_IEEE80211,
			 * 1 to get DLT_PRISM, or 2 to get
			 * DLT_IEEE80211_RADIO_AVS.
			 */
			if ((priv[i].set_args & IW_PRIV_TYPE_MASK) != IW_PRIV_TYPE_INT)
				break;
			if (!(priv[i].set_args & IW_PRIV_SIZE_FIXED))
				break;
			if ((priv[i].set_args & IW_PRIV_SIZE_MASK) != 1)
				break;
			montype = MONITOR_HOSTAP;
			cmd = priv[i].cmd;
			break;
		}
		if (strcmp(priv[i].name, "set_prismhdr") == 0) {
			/*
			 * Prism54 driver, use this one.
			 * Set monitor mode first.
			 * You can set it to 2 to get DLT_IEEE80211
			 * or 3 or get DLT_PRISM.
			 */
			if ((priv[i].set_args & IW_PRIV_TYPE_MASK) != IW_PRIV_TYPE_INT)
				break;
			if (!(priv[i].set_args & IW_PRIV_SIZE_FIXED))
				break;
			if ((priv[i].set_args & IW_PRIV_SIZE_MASK) != 1)
				break;
			montype = MONITOR_PRISM54;
			cmd = priv[i].cmd;
			break;
		}
		if (strcmp(priv[i].name, "forceprismheader") == 0) {
			/*
			 * RT2570 driver, use this one.
			 * Do this after turning monitor mode on.
			 * You can set it to 1 to get DLT_PRISM or 2
			 * to get DLT_IEEE80211.
			 */
			if ((priv[i].set_args & IW_PRIV_TYPE_MASK) != IW_PRIV_TYPE_INT)
				break;
			if (!(priv[i].set_args & IW_PRIV_SIZE_FIXED))
				break;
			if ((priv[i].set_args & IW_PRIV_SIZE_MASK) != 1)
				break;
			montype = MONITOR_RT2570;
			cmd = priv[i].cmd;
			break;
		}
		if (strcmp(priv[i].name, "forceprism") == 0) {
			/*
			 * RT73 driver, use this one.
			 * Do this after turning monitor mode on.
			 * Its argument is a *string*; you can
			 * set it to "1" to get DLT_PRISM or "2"
			 * to get DLT_IEEE80211.
			 */
			if ((priv[i].set_args & IW_PRIV_TYPE_MASK) != IW_PRIV_TYPE_CHAR)
				break;
			if (priv[i].set_args & IW_PRIV_SIZE_FIXED)
				break;
			montype = MONITOR_RT73;
			cmd = priv[i].cmd;
			break;
		}
		if (strcmp(priv[i].name, "prismhdr") == 0) {
			/*
			 * One of the RTL8xxx drivers, use this one.
			 * It can only be done after monitor mode
			 * has been turned on.  You can set it to 1
			 * to get DLT_PRISM or 0 to get DLT_IEEE80211.
			 */
			if ((priv[i].set_args & IW_PRIV_TYPE_MASK) != IW_PRIV_TYPE_INT)
				break;
			if (!(priv[i].set_args & IW_PRIV_SIZE_FIXED))
				break;
			if ((priv[i].set_args & IW_PRIV_SIZE_MASK) != 1)
				break;
			montype = MONITOR_RTL8XXX;
			cmd = priv[i].cmd;
			break;
		}
		if (strcmp(priv[i].name, "rfmontx") == 0) {
			/*
			 * RT2500 or RT61 driver, use this one.
			 * It has one one-byte parameter; set
			 * u.data.length to 1 and u.data.pointer to
			 * point to the parameter.
			 * It doesn't itself turn monitor mode on.
			 * You can set it to 1 to allow transmitting
			 * in monitor mode(?) and get DLT_IEEE80211,
			 * or set it to 0 to disallow transmitting in
			 * monitor mode(?) and get DLT_PRISM.
			 */
			if ((priv[i].set_args & IW_PRIV_TYPE_MASK) != IW_PRIV_TYPE_INT)
				break;
			if ((priv[i].set_args & IW_PRIV_SIZE_MASK) != 2)
				break;
			montype = MONITOR_RT2500;
			cmd = priv[i].cmd;
			break;
		}
		if (strcmp(priv[i].name, "monitor") == 0) {
			/*
			 * Either ACX100 or hostap, use this one.
			 * It turns monitor mode on.
			 * If it takes two arguments, it's ACX100;
			 * the first argument is 1 for DLT_PRISM
			 * or 2 for DLT_IEEE80211, and the second
			 * argument is the channel on which to
			 * run.  If it takes one argument, it's
			 * HostAP, and the argument is 2 for
			 * DLT_IEEE80211 and 3 for DLT_PRISM.
			 *
			 * If we see this, we don't quit, as this
			 * might be a version of the hostap driver
			 * that also supports "monitor_type".
			 */
			if ((priv[i].set_args & IW_PRIV_TYPE_MASK) != IW_PRIV_TYPE_INT)
				break;
			if (!(priv[i].set_args & IW_PRIV_SIZE_FIXED))
				break;
			switch (priv[i].set_args & IW_PRIV_SIZE_MASK) {

			case 1:
				montype = MONITOR_PRISM;
				cmd = priv[i].cmd;
				break;

			case 2:
				montype = MONITOR_ACX100;
				cmd = priv[i].cmd;
				break;

			default:
				break;
			}
		}
	}
	free(priv);

	/*
	 * XXX - ipw3945?  islism?
	 */

	/*
	 * Get the old mode.
	 */
	strncpy(ireq.ifr_ifrn.ifrn_name, device,
	    sizeof ireq.ifr_ifrn.ifrn_name);
	ireq.ifr_ifrn.ifrn_name[sizeof ireq.ifr_ifrn.ifrn_name - 1] = 0;
	if (ioctl(sock_fd, SIOCGIWMODE, &ireq) == -1) {
		/*
		 * We probably won't be able to set the mode, either.
		 */
		return PCAP_ERROR_RFMON_NOTSUP;
	}

	/*
	 * Is it currently in monitor mode?
	 */
	if (ireq.u.mode == IW_MODE_MONITOR) {
		/*
		 * Yes.  Just leave things as they are.
		 * We don't offer multiple link-layer types, as
		 * changing the link-layer type out from under
		 * somebody else capturing in monitor mode would
		 * be considered rude.
		 */
		return 1;
	}
	/*
	 * No.  We have to put the adapter into rfmon mode.
	 */

	/*
	 * If we haven't already done so, arrange to have
	 * "pcap_close_all()" called when we exit.
	 */
	if (!pcap_do_addexit(handle)) {
		/*
		 * "atexit()" failed; don't put the interface
		 * in rfmon mode, just give up.
		 */
		return PCAP_ERROR_RFMON_NOTSUP;
	}

	/*
	 * Save the old mode.
	 */
	handle->md.oldmode = ireq.u.mode;

	/*
	 * Put the adapter in rfmon mode.  How we do this depends
	 * on whether we have a special private ioctl or not.
	 */
	if (montype == MONITOR_PRISM) {
		/*
		 * We have the "monitor" private ioctl, but none of
		 * the other private ioctls.  Use this, and select
		 * the Prism header.
		 *
		 * If it fails, just fall back on SIOCSIWMODE.
		 */
		memset(&ireq, 0, sizeof ireq);
		strncpy(ireq.ifr_ifrn.ifrn_name, device,
		    sizeof ireq.ifr_ifrn.ifrn_name);
		ireq.ifr_ifrn.ifrn_name[sizeof ireq.ifr_ifrn.ifrn_name - 1] = 0;
		ireq.u.data.length = 1;	/* 1 argument */
		args[0] = 3;	/* request Prism header */
		memcpy(ireq.u.name, args, IFNAMSIZ);
		if (ioctl(sock_fd, cmd, &ireq) != -1) {
			/*
			 * Success.
			 * Note that we have to put the old mode back
			 * when we close the device.
			 */
			handle->md.must_clear |= MUST_CLEAR_RFMON;

			/*
			 * Add this to the list of pcaps to close
			 * when we exit.
			 */
			pcap_add_to_pcaps_to_close(handle);

			return 1;
		}

		/*
		 * Failure.  Fall back on SIOCSIWMODE.
		 */
	}

	/*
	 * First, turn monitor mode on.
	 */
	strncpy(ireq.ifr_ifrn.ifrn_name, device,
	    sizeof ireq.ifr_ifrn.ifrn_name);
	ireq.ifr_ifrn.ifrn_name[sizeof ireq.ifr_ifrn.ifrn_name - 1] = 0;
	ireq.u.mode = IW_MODE_MONITOR;
	if (ioctl(sock_fd, SIOCSIWMODE, &ireq) == -1) {
		/*
		 * Scientist, you've failed.
		 */
		return PCAP_ERROR_RFMON_NOTSUP;
	}

	/*
	 * XXX - airmon-ng does "iwconfig {if} key off" after setting
	 * monitor mode and setting the channel, and then does
	 * "iwconfig up".
	 */

	/*
	 * Now select the appropriate radio header.
	 */
	switch (montype) {

	case MONITOR_WEXT:
		/*
		 * We don't have any private ioctl to set the header.
		 */
		break;

	case MONITOR_HOSTAP:
		/*
		 * Select the AVS header if we can, otherwise
		 * select the Prism header.
		 */
		memset(&ireq, 0, sizeof ireq);
		strncpy(ireq.ifr_ifrn.ifrn_name, device,
		    sizeof ireq.ifr_ifrn.ifrn_name);
		ireq.ifr_ifrn.ifrn_name[sizeof ireq.ifr_ifrn.ifrn_name - 1] = 0;
		args[0] = 2;	/* request AVS header */
		memcpy(ireq.u.name, args, sizeof (int));
		if (ioctl(sock_fd, cmd, &ireq) == -1) {
			/*
			 * Failure - try the Prism header.
			 */
			memset(&ireq, 0, sizeof ireq);
			strncpy(ireq.ifr_ifrn.ifrn_name, device,
			    sizeof ireq.ifr_ifrn.ifrn_name);
			ireq.ifr_ifrn.ifrn_name[sizeof ireq.ifr_ifrn.ifrn_name - 1] = 0;
			args[0] = 1;	/* request Prism header */
			memcpy(ireq.u.name, args, sizeof (int));
			ioctl(sock_fd, cmd, &ireq);
		}
		break;

	case MONITOR_PRISM:
		/*
		 * The private ioctl failed.
		 */
		break;

	case MONITOR_PRISM54:
		/*
		 * Select the Prism header.
		 */
		memset(&ireq, 0, sizeof ireq);
		strncpy(ireq.ifr_ifrn.ifrn_name, device,
		    sizeof ireq.ifr_ifrn.ifrn_name);
		ireq.ifr_ifrn.ifrn_name[sizeof ireq.ifr_ifrn.ifrn_name - 1] = 0;
		args[0] = 3;	/* request Prism header */
		memcpy(ireq.u.name, args, sizeof (int));
		ioctl(sock_fd, cmd, &ireq);
		break;

	case MONITOR_ACX100:
		/*
		 * Get the current channel.
		 */
		memset(&ireq, 0, sizeof ireq);
		strncpy(ireq.ifr_ifrn.ifrn_name, device,
		    sizeof ireq.ifr_ifrn.ifrn_name);
		ireq.ifr_ifrn.ifrn_name[sizeof ireq.ifr_ifrn.ifrn_name - 1] = 0;
		if (ioctl(sock_fd, SIOCGIWFREQ, &ireq) == -1) {
			snprintf(handle->errbuf, PCAP_ERRBUF_SIZE,
			    "%s: SIOCGIWFREQ: %s", device,
			    pcap_strerror(errno));
			return PCAP_ERROR;
		}
		channel = ireq.u.freq.m;

		/*
		 * Select the Prism header, and set the channel to the
		 * current value.
		 */
		memset(&ireq, 0, sizeof ireq);
		strncpy(ireq.ifr_ifrn.ifrn_name, device,
		    sizeof ireq.ifr_ifrn.ifrn_name);
		ireq.ifr_ifrn.ifrn_name[sizeof ireq.ifr_ifrn.ifrn_name - 1] = 0;
		args[0] = 1;		/* request Prism header */
		args[1] = channel;	/* set channel */
		memcpy(ireq.u.name, args, 2*sizeof (int));
		ioctl(sock_fd, cmd, &ireq);
		break;

	case MONITOR_RT2500:
		/*
		 * Disallow transmission - that turns on the
		 * Prism header.
		 */
		memset(&ireq, 0, sizeof ireq);
		strncpy(ireq.ifr_ifrn.ifrn_name, device,
		    sizeof ireq.ifr_ifrn.ifrn_name);
		ireq.ifr_ifrn.ifrn_name[sizeof ireq.ifr_ifrn.ifrn_name - 1] = 0;
		args[0] = 0;	/* disallow transmitting */
		memcpy(ireq.u.name, args, sizeof (int));
		ioctl(sock_fd, cmd, &ireq);
		break;

	case MONITOR_RT2570:
		/*
		 * Force the Prism header.
		 */
		memset(&ireq, 0, sizeof ireq);
		strncpy(ireq.ifr_ifrn.ifrn_name, device,
		    sizeof ireq.ifr_ifrn.ifrn_name);
		ireq.ifr_ifrn.ifrn_name[sizeof ireq.ifr_ifrn.ifrn_name - 1] = 0;
		args[0] = 1;	/* request Prism header */
		memcpy(ireq.u.name, args, sizeof (int));
		ioctl(sock_fd, cmd, &ireq);
		break;

	case MONITOR_RT73:
		/*
		 * Force the Prism header.
		 */
		memset(&ireq, 0, sizeof ireq);
		strncpy(ireq.ifr_ifrn.ifrn_name, device,
		    sizeof ireq.ifr_ifrn.ifrn_name);
		ireq.ifr_ifrn.ifrn_name[sizeof ireq.ifr_ifrn.ifrn_name - 1] = 0;
		ireq.u.data.length = 1;	/* 1 argument */
		ireq.u.data.pointer = "1";
		ireq.u.data.flags = 0;
		ioctl(sock_fd, cmd, &ireq);
		break;

	case MONITOR_RTL8XXX:
		/*
		 * Force the Prism header.
		 */
		memset(&ireq, 0, sizeof ireq);
		strncpy(ireq.ifr_ifrn.ifrn_name, device,
		    sizeof ireq.ifr_ifrn.ifrn_name);
		ireq.ifr_ifrn.ifrn_name[sizeof ireq.ifr_ifrn.ifrn_name - 1] = 0;
		args[0] = 1;	/* request Prism header */
		memcpy(ireq.u.name, args, sizeof (int));
		ioctl(sock_fd, cmd, &ireq);
		break;
	}

	/*
	 * Note that we have to put the old mode back when we
	 * close the device.
	 */
	handle->md.must_clear |= MUST_CLEAR_RFMON;

	/*
	 * Add this to the list of pcaps to close when we exit.
	 */
	pcap_add_to_pcaps_to_close(handle);

	return 1;
#else
	/*
	 * We don't have the Wireless Extensions available, so we can't
	 * do monitor mode.
	 */
	return 0;
#endif // endif
}

#endif /* HAVE_PF_PACKET_SOCKETS */

/* ===== Functions to interface to the older kernels ================== */

/*
 * Try to open a packet socket using the old kernel interface.
 * Returns 1 on success and a PCAP_ERROR_ value on an error.
 */
static int
activate_old(pcap_t *handle)
{
	int		arptype;
	struct ifreq	ifr;
	const char	*device = handle->opt.source;
	struct utsname	utsname;
	int		mtu;

	/* Open the socket */

	handle->fd = socket(PF_INET, SOCK_PACKET, htons(ETH_P_ALL));
	if (handle->fd == -1) {
		snprintf(handle->errbuf, PCAP_ERRBUF_SIZE,
			 "socket: %s", pcap_strerror(errno));
		return PCAP_ERROR_PERM_DENIED;
	}

	/* It worked - we are using the old interface */
	handle->md.sock_packet = 1;

	/* ...which means we get the link-layer header. */
	handle->md.cooked = 0;

	/* Bind to the given device */

	if (!device) {
		strncpy(handle->errbuf, "pcap_activate: The \"any\" device isn't supported on 2.0[.x]-kernel systems",
			PCAP_ERRBUF_SIZE);
		return PCAP_ERROR;
	}
	if (iface_bind_old(handle->fd, device, handle->errbuf) == -1)
		return PCAP_ERROR;

	/*
	 * Try to get the link-layer type.
	 */
	arptype = iface_get_arptype(handle->fd, device, handle->errbuf);
	if (arptype < 0)
		return PCAP_ERROR;

	/*
	 * Try to find the DLT_ type corresponding to that
	 * link-layer type.
	 */
	map_arphrd_to_dlt(handle, arptype, 0);
	if (handle->linktype == -1) {
		snprintf(handle->errbuf, PCAP_ERRBUF_SIZE,
			 "unknown arptype %d", arptype);
		return PCAP_ERROR;
	}

	/* Go to promisc mode if requested */

	if (handle->opt.promisc) {
		memset(&ifr, 0, sizeof(ifr));
		strncpy(ifr.ifr_name, device, sizeof(ifr.ifr_name));
		if (ioctl(handle->fd, SIOCGIFFLAGS, &ifr) == -1) {
			snprintf(handle->errbuf, PCAP_ERRBUF_SIZE,
				 "SIOCGIFFLAGS: %s", pcap_strerror(errno));
			return PCAP_ERROR;
		}
		if ((ifr.ifr_flags & IFF_PROMISC) == 0) {
			/*
			 * Promiscuous mode isn't currently on,
			 * so turn it on, and remember that
			 * we should turn it off when the
			 * pcap_t is closed.
			 */

			/*
			 * If we haven't already done so, arrange
			 * to have "pcap_close_all()" called when
			 * we exit.
			 */
			if (!pcap_do_addexit(handle)) {
				/*
				 * "atexit()" failed; don't put
				 * the interface in promiscuous
				 * mode, just give up.
				 */
				return PCAP_ERROR;
			}

			ifr.ifr_flags |= IFF_PROMISC;
			if (ioctl(handle->fd, SIOCSIFFLAGS, &ifr) == -1) {
			        snprintf(handle->errbuf, PCAP_ERRBUF_SIZE,
					 "SIOCSIFFLAGS: %s",
					 pcap_strerror(errno));
				return PCAP_ERROR;
			}
			handle->md.must_clear |= MUST_CLEAR_PROMISC;

			/*
			 * Add this to the list of pcaps
			 * to close when we exit.
			 */
			pcap_add_to_pcaps_to_close(handle);
		}
	}

	/*
	 * Compute the buffer size.
	 *
	 * We're using SOCK_PACKET, so this might be a 2.0[.x]
	 * kernel, and might require special handling - check.
	 */
	if (uname(&utsname) < 0 ||
	    strncmp(utsname.release, "2.0", 3) == 0) {
		/*
		 * Either we couldn't find out what kernel release
		 * this is, or it's a 2.0[.x] kernel.
		 *
		 * In the 2.0[.x] kernel, a "recvfrom()" on
		 * a SOCK_PACKET socket, with MSG_TRUNC set, will
		 * return the number of bytes read, so if we pass
		 * a length based on the snapshot length, it'll
		 * return the number of bytes from the packet
		 * copied to userland, not the actual length
		 * of the packet.
		 *
		 * This means that, for example, the IP dissector
		 * in tcpdump will get handed a packet length less
		 * than the length in the IP header, and will
		 * complain about "truncated-ip".
		 *
		 * So we don't bother trying to copy from the
		 * kernel only the bytes in which we're interested,
		 * but instead copy them all, just as the older
		 * versions of libpcap for Linux did.
		 *
		 * The buffer therefore needs to be big enough to
		 * hold the largest packet we can get from this
		 * device.  Unfortunately, we can't get the MRU
		 * of the network; we can only get the MTU.  The
		 * MTU may be too small, in which case a packet larger
		 * than the buffer size will be truncated *and* we
		 * won't get the actual packet size.
		 *
		 * However, if the snapshot length is larger than
		 * the buffer size based on the MTU, we use the
		 * snapshot length as the buffer size, instead;
		 * this means that with a sufficiently large snapshot
		 * length we won't artificially truncate packets
		 * to the MTU-based size.
		 *
		 * This mess just one of many problems with packet
		 * capture on 2.0[.x] kernels; you really want a
		 * 2.2[.x] or later kernel if you want packet capture
		 * to work well.
		 */
		mtu = iface_get_mtu(handle->fd, device, handle->errbuf);
		if (mtu == -1)
			return PCAP_ERROR;
		handle->bufsize = MAX_LINKHEADER_SIZE + mtu;
		if (handle->bufsize < handle->snapshot)
			handle->bufsize = handle->snapshot;
	} else {
		/*
		 * This is a 2.2[.x] or later kernel.
		 *
		 * We can safely pass "recvfrom()" a byte count
		 * based on the snapshot length.
		 */
		handle->bufsize = handle->snapshot;
	}

	/*
	 * Default value for offset to align link-layer payload
	 * on a 4-byte boundary.
	 */
	handle->offset	 = 0;

	return 1;
}

/*
 *  Bind the socket associated with FD to the given device using the
 *  interface of the old kernels.
 */
static int
iface_bind_old(int fd, const char *device, char *ebuf)
{
	struct sockaddr	saddr;
	int		err;
	socklen_t	errlen = sizeof(err);

	memset(&saddr, 0, sizeof(saddr));
	strncpy(saddr.sa_data, device, sizeof(saddr.sa_data));
	if (bind(fd, &saddr, sizeof(saddr)) == -1) {
		snprintf(ebuf, PCAP_ERRBUF_SIZE,
			 "bind: %s", pcap_strerror(errno));
		return -1;
	}

	/* Any pending errors, e.g., network is down? */

	if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &errlen) == -1) {
		snprintf(ebuf, PCAP_ERRBUF_SIZE,
			"getsockopt: %s", pcap_strerror(errno));
		return -1;
	}

	if (err > 0) {
		snprintf(ebuf, PCAP_ERRBUF_SIZE,
			"bind: %s", pcap_strerror(err));
		return -1;
	}

	return 0;
}

/* ===== System calls available on all supported kernels ============== */

/*
 *  Query the kernel for the MTU of the given interface.
 */
static int
iface_get_mtu(int fd, const char *device, char *ebuf)
{
	struct ifreq	ifr;

	if (!device)
		return BIGGER_THAN_ALL_MTUS;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, device, sizeof(ifr.ifr_name));

	if (ioctl(fd, SIOCGIFMTU, &ifr) == -1) {
		snprintf(ebuf, PCAP_ERRBUF_SIZE,
			 "SIOCGIFMTU: %s", pcap_strerror(errno));
		return -1;
	}

	return ifr.ifr_mtu;
}

/*
 *  Get the hardware type of the given interface as ARPHRD_xxx constant.
 */
static int
iface_get_arptype(int fd, const char *device, char *ebuf)
{
	struct ifreq	ifr;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, device, sizeof(ifr.ifr_name));

	if (ioctl(fd, SIOCGIFHWADDR, &ifr) == -1) {
		snprintf(ebuf, PCAP_ERRBUF_SIZE,
			 "SIOCGIFHWADDR: %s", pcap_strerror(errno));
		if (errno == ENODEV) {
			/*
			 * No such device.
			 */
			return PCAP_ERROR_NO_SUCH_DEVICE;
		}
		return PCAP_ERROR;
	}

	return ifr.ifr_hwaddr.sa_family;
}

#ifdef SO_ATTACH_FILTER
static int
fix_program(pcap_t *handle, struct sock_fprog *fcode)
{
	size_t prog_size;
	register int i;
	register struct bpf_insn *p;
	struct bpf_insn *f;
	int len;

	/*
	 * Make a copy of the filter, and modify that copy if
	 * necessary.
	 */
	prog_size = sizeof(*handle->fcode.bf_insns) * handle->fcode.bf_len;
	len = handle->fcode.bf_len;
	f = (struct bpf_insn *)malloc(prog_size);
	if (f == NULL) {
		snprintf(handle->errbuf, PCAP_ERRBUF_SIZE,
			 "malloc: %s", pcap_strerror(errno));
		return -1;
	}
	memcpy(f, handle->fcode.bf_insns, prog_size);
	fcode->len = len;
	fcode->filter = (struct sock_filter *) f;

	for (i = 0; i < len; ++i) {
		p = &f[i];
		/*
		 * What type of instruction is this?
		 */
		switch (BPF_CLASS(p->code)) {

		case BPF_RET:
			/*
			 * It's a return instruction; is the snapshot
			 * length a constant, rather than the contents
			 * of the accumulator?
			 */
			if (BPF_MODE(p->code) == BPF_K) {
				/*
				 * Yes - if the value to be returned,
				 * i.e. the snapshot length, is anything
				 * other than 0, make it 65535, so that
				 * the packet is truncated by "recvfrom()",
				 * not by the filter.
				 *
				 * XXX - there's nothing we can easily do
				 * if it's getting the value from the
				 * accumulator; we'd have to insert
				 * code to force non-zero values to be
				 * 65535.
				 */
				if (p->k != 0)
					p->k = 65535;
			}
			break;

		case BPF_LD:
		case BPF_LDX:
			/*
			 * It's a load instruction; is it loading
			 * from the packet?
			 */
			switch (BPF_MODE(p->code)) {

			case BPF_ABS:
			case BPF_IND:
			case BPF_MSH:
				/*
				 * Yes; are we in cooked mode?
				 */
				if (handle->md.cooked) {
					/*
					 * Yes, so we need to fix this
					 * instruction.
					 */
					if (fix_offset(p) < 0) {
						/*
						 * We failed to do so.
						 * Return 0, so our caller
						 * knows to punt to userland.
						 */
						return 0;
					}
				}
				break;
			}
			break;
		}
	}
	return 1;	/* we succeeded */
}

static int
fix_offset(struct bpf_insn *p)
{
	/*
	 * What's the offset?
	 */
	if (p->k >= SLL_HDR_LEN) {
		/*
		 * It's within the link-layer payload; that starts at an
		 * offset of 0, as far as the kernel packet filter is
		 * concerned, so subtract the length of the link-layer
		 * header.
		 */
		p->k -= SLL_HDR_LEN;
	} else if (p->k == 14) {
		/*
		 * It's the protocol field; map it to the special magic
		 * kernel offset for that field.
		 */
		p->k = SKF_AD_OFF + SKF_AD_PROTOCOL;
	} else {
		/*
		 * It's within the header, but it's not one of those
		 * fields; we can't do that in the kernel, so punt
		 * to userland.
		 */
		return -1;
	}
	return 0;
}

static int
set_kernel_filter(pcap_t *handle, struct sock_fprog *fcode)
{
	int total_filter_on = 0;
	int save_mode;
	int ret;
	int save_errno;

	/*
	 * The socket filter code doesn't discard all packets queued
	 * up on the socket when the filter is changed; this means
	 * that packets that don't match the new filter may show up
	 * after the new filter is put onto the socket, if those
	 * packets haven't yet been read.
	 *
	 * This means, for example, that if you do a tcpdump capture
	 * with a filter, the first few packets in the capture might
	 * be packets that wouldn't have passed the filter.
	 *
	 * We therefore discard all packets queued up on the socket
	 * when setting a kernel filter.  (This isn't an issue for
	 * userland filters, as the userland filtering is done after
	 * packets are queued up.)
	 *
	 * To flush those packets, we put the socket in read-only mode,
	 * and read packets from the socket until there are no more to
	 * read.
	 *
	 * In order to keep that from being an infinite loop - i.e.,
	 * to keep more packets from arriving while we're draining
	 * the queue - we put the "total filter", which is a filter
	 * that rejects all packets, onto the socket before draining
	 * the queue.
	 *
	 * This code deliberately ignores any errors, so that you may
	 * get bogus packets if an error occurs, rather than having
	 * the filtering done in userland even if it could have been
	 * done in the kernel.
	 */
	if (setsockopt(handle->fd, SOL_SOCKET, SO_ATTACH_FILTER,
		       &total_fcode, sizeof(total_fcode)) == 0) {
		char drain[1];

		/*
		 * Note that we've put the total filter onto the socket.
		 */
		total_filter_on = 1;

		/*
		 * Save the socket's current mode, and put it in
		 * non-blocking mode; we drain it by reading packets
		 * until we get an error (which is normally a
		 * "nothing more to be read" error).
		 */
		save_mode = fcntl(handle->fd, F_GETFL, 0);
		if (save_mode != -1 &&
		    fcntl(handle->fd, F_SETFL, save_mode | O_NONBLOCK) >= 0) {
			while (recv(handle->fd, &drain, sizeof drain,
			       MSG_TRUNC) >= 0)
				;
			save_errno = errno;
			fcntl(handle->fd, F_SETFL, save_mode);
			if (save_errno != EAGAIN) {
				/* Fatal error */
				reset_kernel_filter(handle);
				snprintf(handle->errbuf, PCAP_ERRBUF_SIZE,
				 "recv: %s", pcap_strerror(save_errno));
				return -2;
			}
		}
	}

	/*
	 * Now attach the new filter.
	 */
	ret = setsockopt(handle->fd, SOL_SOCKET, SO_ATTACH_FILTER,
			 fcode, sizeof(*fcode));
	if (ret == -1 && total_filter_on) {
		/*
		 * Well, we couldn't set that filter on the socket,
		 * but we could set the total filter on the socket.
		 *
		 * This could, for example, mean that the filter was
		 * too big to put into the kernel, so we'll have to
		 * filter in userland; in any case, we'll be doing
		 * filtering in userland, so we need to remove the
		 * total filter so we see packets.
		 */
		save_errno = errno;

		/*
		 * XXX - if this fails, we're really screwed;
		 * we have the total filter on the socket,
		 * and it won't come off.  What do we do then?
		 */
		reset_kernel_filter(handle);

		errno = save_errno;
	}
	return ret;
}

static int
reset_kernel_filter(pcap_t *handle)
{
	/*
	 * setsockopt() barfs unless it get a dummy parameter.
	 * valgrind whines unless the value is initialized,
	 * as it has no idea that setsockopt() ignores its
	 * parameter.
	 */
	int dummy = 0;

	return setsockopt(handle->fd, SOL_SOCKET, SO_DETACH_FILTER,
				   &dummy, sizeof(dummy));
}
#endif // endif
