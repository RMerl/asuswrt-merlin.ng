/*
 * Copyright (c) 1994, 1995, 1996
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the Computer Systems
 *	Engineering Group at Lawrence Berkeley Laboratory.
 * 4. Neither the name of the University nor of the Laboratory may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * @(#) $Header: /tcpdump/master/libpcap/pcap-int.h,v 1.85.2.9 2008-09-16 00:21:08 guy Exp $ (LBL)
 */

#ifndef pcap_int_h
#define	pcap_int_h

#include <pcap/pcap.h>

#ifdef __cplusplus
extern "C" {
#endif // endif

#ifdef HAVE_LIBDLPI
#include <libdlpi.h>
#endif // endif

#ifdef WIN32
#include <Packet32.h>
extern CRITICAL_SECTION g_PcapCompileCriticalSection;
#endif /* WIN32 */

#ifdef HAVE_TC_API
#include "pcap-tc.h"
#endif // endif

#ifdef MSDOS
#include <fcntl.h>
#include <io.h>
#endif // endif

#if (defined(_MSC_VER) && (_MSC_VER <= 1200)) /* we are compiling with Visual Studio 6, \
	that doesn't support the LL suffix*/

/*
 * Swap byte ordering of unsigned long long timestamp on a big endian
 * machine.
 */
#define SWAPLL(ull)  ((ull & 0xff00000000000000) >> 56) | \
                      ((ull & 0x00ff000000000000) >> 40) | \
                      ((ull & 0x0000ff0000000000) >> 24) | \
                      ((ull & 0x000000ff00000000) >> 8)  | \
                      ((ull & 0x00000000ff000000) << 8)  | \
                      ((ull & 0x0000000000ff0000) << 24) | \
                      ((ull & 0x000000000000ff00) << 40) | \
                      ((ull & 0x00000000000000ff) << 56)

#else /* A recent Visual studio compiler or not VC */

/*
 * Swap byte ordering of unsigned long long timestamp on a big endian
 * machine.
 */
#define SWAPLL(ull)  ((ull & 0xff00000000000000LL) >> 56) | \
                      ((ull & 0x00ff000000000000LL) >> 40) | \
                      ((ull & 0x0000ff0000000000LL) >> 24) | \
                      ((ull & 0x000000ff00000000LL) >> 8)  | \
                      ((ull & 0x00000000ff000000LL) << 8)  | \
                      ((ull & 0x0000000000ff0000LL) << 24) | \
                      ((ull & 0x000000000000ff00LL) << 40) | \
                      ((ull & 0x00000000000000ffLL) << 56)

#endif /* _MSC_VER */

/*
 * Savefile
 */
typedef enum {
	NOT_SWAPPED,
	SWAPPED,
	MAYBE_SWAPPED
} swapped_type_t;

/*
 * Used when reading a savefile.
 */
struct pcap_sf {
	FILE *rfile;
	int swapped;
	size_t hdrsize;
	swapped_type_t lengths_swapped;
	int version_major;
	int version_minor;
	u_char *base;
};

/*
 * Used when doing a live capture.
 */
struct pcap_md {
	struct pcap_stat stat;
	int use_bpf;		/* using kernel filter */
	u_long	TotPkts;	/* can't oflow for 79 hrs on ether */
	u_long	TotAccepted;	/* count accepted by filter */
	u_long	TotDrops;	/* count of dropped packets */
	long	TotMissed;	/* missed by i/f during this run */
	long	OrigMissed;	/* missed by i/f before this run */
	char	*device;	/* device name */
	int	timeout;	/* timeout for buffering */
	int	must_clear;	/* stuff we must clear when we close */
	struct pcap *next;	/* list of open pcaps that need stuff cleared on close */
#ifdef linux
	int	sock_packet;	/* using Linux 2.0 compatible interface */
	int	cooked;		/* using SOCK_DGRAM rather than SOCK_RAW */
	int	ifindex;	/* interface index of device we're bound to */
	int	lo_ifindex;	/* interface index of the loopback device */
	u_int	packets_read;	/* count of packets read with recvfrom() */
	bpf_u_int32 oldmode;	/* mode to restore when turning monitor mode off */
	u_int	tp_version;	/* version of tpacket_hdr for mmaped ring */
	u_int	tp_hdrlen;	/* hdrlen of tpacket_hdr for mmaped ring */
#endif /* linux */

#ifdef HAVE_DAG_API
#ifdef HAVE_DAG_STREAMS_API
	u_char	*dag_mem_bottom;	/* DAG card current memory bottom pointer */
	u_char	*dag_mem_top;	/* DAG card current memory top pointer */
#else /* HAVE_DAG_STREAMS_API */
	void	*dag_mem_base;	/* DAG card memory base address */
	u_int	dag_mem_bottom;	/* DAG card current memory bottom offset */
	u_int	dag_mem_top;	/* DAG card current memory top offset */
#endif /* HAVE_DAG_STREAMS_API */
	int	dag_fcs_bits;	/* Number of checksum bits from link layer */
	int	dag_offset_flags; /* Flags to pass to dag_offset(). */
	int	dag_stream;	/* DAG stream number */
	int	dag_timeout;	/* timeout specified to pcap_open_live.
				 * Same as in linux above, introduce
				 * generally? */
#endif /* HAVE_DAG_API */
#ifdef HAVE_ZEROCOPY_BPF
       /*
        * Zero-copy read buffer -- for zero-copy BPF.  'buffer' above will
        * alternative between these two actual mmap'd buffers as required.
        * As there is a header on the front size of the mmap'd buffer, only
        * some of the buffer is exposed to libpcap as a whole via bufsize;
        * zbufsize is the true size.  zbuffer tracks the current zbuf
        * assocated with buffer so that it can be used to decide which the
        * next buffer to read will be.
        */
       u_char *zbuf1, *zbuf2, *zbuffer;
       u_int zbufsize;
       u_int zerocopy;
       u_int interrupted;
       struct timespec firstsel;
       /*
        * If there's currently a buffer being actively processed, then it is
        * referenced here; 'buffer' is also pointed at it, but offset by the
        * size of the header.
        */
       struct bpf_zbuf_header *bzh;
#endif /* HAVE_ZEROCOPY_BPF */

#ifdef HAVE_REMOTE
/*!
	There is really a mess with previous variables, and it seems to me that they are not used
	(they are used in pcap_pf.c only). I think we have to start using them.
	The meaning is the following:

	- TotPkts: the amount of packets received by the bpf filter, *before* applying the filter
	- TotAccepted: the amount of packets that satisfies the filter
	- TotDrops: the amount of packet that were dropped into the kernel buffer because of lack of space
	- TotMissed: the amount of packets that were dropped by the physical interface; it is basically
		the value of the hardware counter into the card. This number is never put to zero, so this number
		takes into account the *total* number of interface drops starting from the interface power-on.
	- OrigMissed: the amount of packets that were dropped by the interface *when the capture begins*.
		This value is used to detect the number of packets dropped by the interface *during the present
		capture*, so that (ps_ifdrops= TotMissed - OrigMissed).
*/
	unsigned int TotNetDrops;       //!< keeps the number of packets that have been dropped by the network
/*!
	\brief It keeps the number of packets that have been received by the application.

	Packets dropped by the kernel buffer are not counted in this variable. The variable is always
	equal to (TotAccepted - TotDrops), exept for the case of remote capture, in which we have also
	packets in fligh, i.e. that have been transmitted by the remote host, but that have not been
	received (yet) from the client. In this case, (TotAccepted - TotDrops - TotNetDrops) gives a
	wrong result, since this number does not corresponds always to the number of packet received by
	the application. For this reason, in the remote capture we need another variable that takes
	into account of the number of packets actually received by the application.
*/
	unsigned int TotCapt;
#endif /* HAVE_REMOTE */

};

/*
 * Stuff to clear when we close.
 */
#define MUST_CLEAR_PROMISC	0x00000001	/* promiscuous mode */
#define MUST_CLEAR_RFMON	0x00000002	/* rfmon (monitor) mode */

struct pcap_opt {
	int	buffer_size;
	char	*source;
	int	promisc;
	int	rfmon;
};

/*
 * Ultrix, DEC OSF/1^H^H^H^H^H^H^H^H^HDigital UNIX^H^H^H^H^H^H^H^H^H^H^H^H
 * Tru64 UNIX, and some versions of NetBSD pad FDDI packets to make everything
 * line up on a nice boundary.
 */
#ifdef __NetBSD__
#include <sys/param.h>	/* needed to declare __NetBSD_Version__ */
#endif // endif

#if defined(ultrix) || defined(__osf__) || (defined(__NetBSD__) && __NetBSD_Version__ > \
	106000000)
#define       PCAP_FDDIPAD 3
#endif // endif

typedef int	(*activate_op_t)(pcap_t *);
typedef int	(*can_set_rfmon_op_t)(pcap_t *);
typedef int	(*read_op_t)(pcap_t *, int cnt, pcap_handler, u_char *);
typedef int	(*inject_op_t)(pcap_t *, const void *, size_t);
typedef int	(*setfilter_op_t)(pcap_t *, struct bpf_program *);
typedef int	(*setdirection_op_t)(pcap_t *, pcap_direction_t);
typedef int	(*set_datalink_op_t)(pcap_t *, int);
typedef int	(*getnonblock_op_t)(pcap_t *, char *);
typedef int	(*setnonblock_op_t)(pcap_t *, int, char *);
typedef int	(*stats_op_t)(pcap_t *, struct pcap_stat *);
#ifdef WIN32
typedef int	(*setbuff_op_t)(pcap_t *, int);
typedef int	(*setmode_op_t)(pcap_t *, int);
typedef int	(*setmintocopy_op_t)(pcap_t *, int);
#endif // endif
typedef void	(*cleanup_op_t)(pcap_t *);

struct pcap {
#ifdef WIN32
	ADAPTER *adapter;
	LPPACKET Packet;
	int nonblock;
#else
	int fd;
	int selectable_fd;
	int send_fd;
#endif /* WIN32 */

#ifdef HAVE_TC_API
	TC_INSTANCE TcInstance;
	TC_PACKETS_BUFFER TcPacketsBuffer;
	ULONG TcAcceptedCount;
	PCHAR PpiPacket;
#endif // endif

#ifdef HAVE_LIBDLPI
	dlpi_handle_t dlpi_hd;
#endif // endif
	int snapshot;
	int linktype;		/* Network linktype */
	int linktype_ext;       /* Extended information stored in the linktype field of a file */
	int tzoff;		/* timezone offset */
	int offset;		/* offset for proper alignment */
	int activated;		/* true if the capture is really started */
	int oldstyle;		/* if we're opening with pcap_open_live() */

	int break_loop;		/* flag set to force break from packet-reading loop */

#ifdef PCAP_FDDIPAD
	int fddipad;
#endif // endif

#ifdef MSDOS
        void (*wait_proc)(void); /*          call proc while waiting */
#endif // endif

	struct pcap_sf sf;
	struct pcap_md md;
	struct pcap_opt opt;

	/*
	 * Read buffer.
	 */
	int bufsize;
	u_char *buffer;
	u_char *bp;
	int cc;

	/*
	 * Place holder for pcap_next().
	 */
	u_char *pkt;

	/* We're accepting only packets in this direction/these directions. */
	pcap_direction_t direction;

	/*
	 * Methods.
	 */
	activate_op_t activate_op;
	can_set_rfmon_op_t can_set_rfmon_op;
	read_op_t read_op;
	inject_op_t inject_op;
	setfilter_op_t setfilter_op;
	setdirection_op_t setdirection_op;
	set_datalink_op_t set_datalink_op;
	getnonblock_op_t getnonblock_op;
	setnonblock_op_t setnonblock_op;
	stats_op_t stats_op;

#ifdef WIN32
	/*
	 * These are, at least currently, specific to the Win32 NPF
	 * driver.
	 */
	setbuff_op_t setbuff_op;
	setmode_op_t setmode_op;
	setmintocopy_op_t setmintocopy_op;
#endif // endif
	cleanup_op_t cleanup_op;

	/*
	 * Placeholder for filter code if bpf not in kernel.
	 */
	struct bpf_program fcode;

	char errbuf[PCAP_ERRBUF_SIZE + 1];
	int dlt_count;
	u_int *dlt_list;

	struct pcap_pkthdr pcap_header;	/* This is needed for the pcap_next_ex() to work */

#ifdef HAVE_REMOTE
	/*! \brief '1' if we're the network client; needed by several functions (like pcap_setfilter() ) to know if
		they have to use the socket or they have to open the local adapter. */
	int rmt_clientside;

	SOCKET rmt_sockctrl;		//!< socket ID of the socket used for the control connection
	SOCKET rmt_sockdata;		//!< socket ID of the socket used for the data connection
	int rmt_flags;				//!< we have to save flags, since they are passed by the pcap_open_live(), but they are used by the pcap_startcapture()
	int rmt_capstarted;			//!< 'true' if the capture is already started (needed to knoe if we have to call the pcap_startcapture()
	struct pcap_samp rmt_samp;	//!< Keeps the parameters related to the sampling process.
	char *currentfilter;		//!< Pointer to a buffer (allocated at run-time) that stores the current filter. Needed when flag PCAP_OPENFLAG_NOCAPTURE_RPCAP is turned on.
#endif /* HAVE_REMOTE */
};

/*
 * This is a timeval as stored in a savefile.
 * It has to use the same types everywhere, independent of the actual
 * `struct timeval'; `struct timeval' has 32-bit tv_sec values on some
 * platforms and 64-bit tv_sec values on other platforms, and writing
 * out native `struct timeval' values would mean files could only be
 * read on systems with the same tv_sec size as the system on which
 * the file was written.
 */

struct pcap_timeval {
    bpf_int32 tv_sec;		/* seconds */
    bpf_int32 tv_usec;		/* microseconds */
};

/*
 * This is a `pcap_pkthdr' as actually stored in a savefile.
 *
 * Do not change the format of this structure, in any way (this includes
 * changes that only affect the length of fields in this structure),
 * and do not make the time stamp anything other than seconds and
 * microseconds (e.g., seconds and nanoseconds).  Instead:
 *
 *	introduce a new structure for the new format;
 *
 *	send mail to "tcpdump-workers@lists.tcpdump.org", requesting
 *	a new magic number for your new capture file format, and, when
 *	you get the new magic number, put it in "savefile.c";
 *
 *	use that magic number for save files with the changed record
 *	header;
 *
 *	make the code in "savefile.c" capable of reading files with
 *	the old record header as well as files with the new record header
 *	(using the magic number to determine the header format).
 *
 * Then supply the changes as a patch at
 *
 *	http://sourceforge.net/projects/libpcap/
 *
 * so that future versions of libpcap and programs that use it (such as
 * tcpdump) will be able to read your new capture file format.
 */

struct pcap_sf_pkthdr {
    struct pcap_timeval ts;	/* time stamp */
    bpf_u_int32 caplen;		/* length of portion present */
    bpf_u_int32 len;		/* length this packet (off wire) */
};

/*
 * How a `pcap_pkthdr' is actually stored in savefiles written
 * by some patched versions of libpcap (e.g. the ones in Red
 * Hat Linux 6.1 and 6.2).
 *
 * Do not change the format of this structure, in any way (this includes
 * changes that only affect the length of fields in this structure).
 * Instead, introduce a new structure, as per the above.
 */

struct pcap_sf_patched_pkthdr {
    struct pcap_timeval ts;	/* time stamp */
    bpf_u_int32 caplen;		/* length of portion present */
    bpf_u_int32 len;		/* length this packet (off wire) */
    int		index;
    unsigned short protocol;
    unsigned char pkt_type;
};

int	yylex(void);

#ifndef min
#define min(a, b) ((a) > (b) ? (b) : (a))
#endif // endif

int	pcap_offline_read(pcap_t *, int, pcap_handler, u_char *);
int	pcap_read(pcap_t *, int cnt, pcap_handler, u_char *);

#ifndef HAVE_STRLCPY
#define strlcpy(x, y, z) \
	(strncpy((x), (y), (z)), \
	 ((z) <= 0 ? 0 : ((x)[(z) - 1] = '\0')), \
	 strlen((y)))
#endif // endif

#include <stdarg.h>

#if !defined(HAVE_SNPRINTF)
#define snprintf pcap_snprintf
extern int snprintf (char *, size_t, const char *, ...);
#endif // endif

#if !defined(HAVE_VSNPRINTF)
#define vsnprintf pcap_vsnprintf
extern int vsnprintf (char *, size_t, const char *, va_list ap);
#endif // endif

/*
 * Routines that most pcap implementations can use for non-blocking mode.
 */
#if !defined(WIN32) && !defined(MSDOS)
int	pcap_getnonblock_fd(pcap_t *, char *);
int	pcap_setnonblock_fd(pcap_t *p, int, char *);
#endif // endif

pcap_t	*pcap_create_common(const char *, char *);
int	pcap_do_addexit(pcap_t *);
void	pcap_add_to_pcaps_to_close(pcap_t *);
void	pcap_remove_from_pcaps_to_close(pcap_t *);
void	pcap_cleanup_live_common(pcap_t *);
int	pcap_not_initialized(pcap_t *);
int	pcap_check_activated(pcap_t *);

/*
 * Internal interfaces for "pcap_findalldevs()".
 *
 * "pcap_platform_finddevs()" is a platform-dependent routine to
 * add devices not found by the "standard" mechanisms (SIOCGIFCONF,
 * "getifaddrs()", etc..
 *
 * "pcap_add_if()" adds an interface to the list of interfaces.
 */
int	pcap_platform_finddevs(pcap_if_t **, char *);
int	add_addr_to_iflist(pcap_if_t **, const char *, u_int, struct sockaddr *,
	    size_t, struct sockaddr *, size_t, struct sockaddr *, size_t,
	    struct sockaddr *, size_t, char *);
int	pcap_add_if(pcap_if_t **, const char *, u_int, const char *, char *);
struct sockaddr *dup_sockaddr(struct sockaddr *, size_t);
int	add_or_find_if(pcap_if_t **, pcap_if_t **, const char *, u_int,
	    const char *, char *);

#ifdef WIN32
char	*pcap_win32strerror(void);
#endif // endif

int	install_bpf_program(pcap_t *, struct bpf_program *);

int	pcap_strcasecmp(const char *, const char *);

#ifdef __cplusplus
}
#endif // endif

#endif // endif
