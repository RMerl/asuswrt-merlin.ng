/*
	Copyright (C) Slava Astashonok <sla@0n.ru>

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License.

	$Id: fprobe.c,v 1.10.2.24 2005/01/30 08:43:35 sla Exp $
*/

#include <common.h>

/* stdout, stderr, freopen() */
#include <stdio.h>

/* atoi(), exit() */
#include <stdlib.h>

/* getopt(), alarm(), getpid(), sedsid(), chdir() */
#include <unistd.h>

/* strerror() */
#include <string.h>

/* sig*() */
#include <signal.h>

/* pcap_*() */
#include <pcap.h>

/* inet_*() (Linux, FreeBSD, Solaris), getpid() */
#include <sys/types.h>
#include <netinet/in_systm.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>

#include <sys/param.h>
#include <pwd.h>
#ifdef OS_LINUX
#include <grp.h>
#endif

/* pthread_*() */
#include <pthread.h>

/* errno */
#include <errno.h>

/* getaddrinfo() */
#include <netdb.h>

/* nanosleep() */
#include <time.h>

/* gettimeofday() */
#include <sys/time.h>

/* scheduling */
#include <sched.h>

/* select() (POSIX)*/
#include <sys/select.h>

/* open() */
#include <sys/stat.h>
#include <fcntl.h>

#include <fprobe.h>
#include <my_log.h>
#include <my_getopt.h>
#include <netflow.h>
#include <hash.h>
#include <mem.h>

struct DLT dlt[] = {
#ifdef DLT_NULL	
	{DLT_NULL, 0, 4, 4, "NULL"},
#endif
#ifdef DLT_EN10MB	
	{DLT_EN10MB, 12, 14, 17, "EN10MB"},
#endif
#ifdef DLT_IEEE802
	{DLT_IEEE802, 14, 22, 17, "IEEE802"},
#endif
#ifdef DLT_ARCNET
	{DLT_ARCNET, 2 , 6, 6, "ARCNET"},
#endif
#ifdef DLT_SLIP
	{DLT_SLIP, -1, 16, 16, "SLIP"},
#endif
#ifdef DLT_PPP
	{DLT_PPP, 2, 4, 4, "PPP"},
#endif
#ifdef DLT_FDDI
	/*
	FIXME
	See gencode.c from libpcap
	*/
	{DLT_FDDI, 13, 21, 16, "FDDI"},
#endif
#ifdef DLT_ATM_RFC1483
	{DLT_ATM_RFC1483, 0, 8, 3, "ATM_RFC1483"},
#endif
#ifdef DLT_RAW
	{DLT_RAW, -1, 0, 0, "RAW"},
#endif
#ifdef DLT_SLIP_BSDOS
	{DLT_SLIP_BSDOS, -1, 24, 24, "SLIP_BSDOS"},
#endif
#ifdef DLT_PPP_BSDOS
	{DLT_PPP_BSDOS, 5, 24, 24, "PPP_BSDOS"},
#endif
#ifdef DLT_ATM_CLIP
	{DLT_ATM_CLIP, 0, 8, 3, "ATM_CLIP"},
#endif
#ifdef DLT_PPP_SERIAL
	{DLT_PPP_SERIAL, 2, 4, 4, "PPP_SERIAL"},
#endif
#ifdef DLT_PPP_ETHER
	{DLT_PPP_ETHER, 6, 8, 8, "PPP_ETHER"},
#endif
#ifdef DLT_C_HDLC
	{DLT_C_HDLC, 2, 4, 4, "C_HDLC"},
#endif
#ifdef DLT_IEEE802_11
	{DLT_IEEE802_11, 24, 32, 27, "IEEE802_11"},
#endif
#ifdef DLT_LOOP
	{DLT_LOOP, 0, 4, 4, "LOOP"},
#endif
#ifdef DLT_LINUX_SLL
	{DLT_LINUX_SLL, 14, 16, 16, "LINUX_SLL"},
#endif
#ifdef DLT_LTALK
	{DLT_LTALK, -1, 0, 0, "LTALK"},
#endif
#ifdef DLT_PRISM_HEADER
	{DLT_PRISM_HEADER, 144 + 24, 144 + 30, 144 + 27, "PRISM_HEADER"},
#endif
#ifdef DLT_IP_OVER_FC
	{DLT_IP_OVER_FC, 16, 24, 19, "IP_OVER_FC"},
#endif
#ifdef DLT_SUNATM
	{DLT_SUNATM, 4, 4 + 8, 4 + 3, "SUNATM"},
#endif
#ifdef DLT_ARCNET_LINUX
	{DLT_ARCNET_LINUX, 4, 8, 8, "ARCNET_LINUX"},
#endif
#ifdef DLT_ENC
	{DLT_ENC, 0, 12, 12, "ENC"},
#endif
#ifdef DLT_FRELAY
	{DLT_FRELAY, -1, 0, 0, "FRELAY"},
#endif
#ifdef DLT_IEEE802_11_RADIO
	{DLT_IEEE802_11_RADIO, 64 + 24, 64 + 32, 64 + 27, "IEEE802_11_RADIO"},
#endif
#ifdef DLT_PFLOG
	{DLT_PFLOG, 0, 28, 28, "PFLOG"},
#endif
#ifdef DLT_LINUX_IRDA
	{DLT_LINUX_IRDA, -1, -1, -1, "LINUX_IRDA"},
#endif
#ifdef DLT_APPLE_IP_OVER_IEEE1394
	{DLT_APPLE_IP_OVER_IEEE1394, 16, 18, 0, "APPLE_IP_OVER_IEEE1394"},
#endif
#ifdef DLT_IEEE802_11_RADIO_AVS
	{DLT_IEEE802_11_RADIO_AVS, 64 + 24, 64 + 32, 64 + 27, "IEEE802_11_RADIO_AVS"},
#endif
#ifdef DLT_PFSYNC
	{DLT_PFSYNC, -1, 4, 4, "PFSYNC"},
#endif
	{-1, -1, -1, -1, "UNKNOWN"}
};

enum {
	aflag,
	Bflag,
	bflag,
	cflag,
	dflag,
	eflag,
	fflag,
	gflag,
	hflag,
	iflag,
	Kflag,
	kflag,
	lflag,
	mflag,
	nflag,
	pflag,
	qflag,
	rflag,
	Sflag,
	sflag,
	tflag,
	uflag,
	vflag,
	xflag,
};

static struct getopt_parms parms[] = {
	{'a', MY_GETOPT_ARG_REQUIRED, 0, 0},
	{'B', MY_GETOPT_ARG_REQUIRED, 0, 0},
	{'b', MY_GETOPT_ARG_REQUIRED, 0, 0},
	{'c', MY_GETOPT_ARG_REQUIRED, 0, 0},
	{'d', MY_GETOPT_ARG_REQUIRED, 0, 0},
	{'e', MY_GETOPT_ARG_REQUIRED, 0, 0},
	{'f', MY_GETOPT_ARG_REQUIRED, 0, 0},
	{'g', MY_GETOPT_ARG_REQUIRED, 0, 0},
	{'h', 0, 0, 0},
	{'i', MY_GETOPT_ARG_REQUIRED, 0, 0},
	{'K', MY_GETOPT_ARG_REQUIRED, 0, 0},
	{'k', 0, 0, 0},
	{'l', MY_GETOPT_ARG_REQUIRED, 0, 0},
	{'m', MY_GETOPT_ARG_REQUIRED, 0, 0},
	{'n', MY_GETOPT_ARG_REQUIRED, 0, 0},
	{'p', 0, 0, 0},
	{'q', MY_GETOPT_ARG_REQUIRED, 0, 0},
	{'r', MY_GETOPT_ARG_REQUIRED, 0, 0},
	{'S', MY_GETOPT_ARG_REQUIRED, 0, 0},
	{'s', MY_GETOPT_ARG_REQUIRED, 0, 0},
	{'t', MY_GETOPT_ARG_REQUIRED, 0, 0},
	{'u', MY_GETOPT_ARG_REQUIRED, 0, 0},
	{'v', MY_GETOPT_ARG_REQUIRED, 0, 0},
	{'x', MY_GETOPT_ARG_REQUIRED, 0, 0},
	{0, 0, 0, 0}
};

extern char *optarg;
extern int optind, opterr, optopt;
extern int errno;

extern struct NetFlow NetFlow1;
extern struct NetFlow NetFlow5;
extern struct NetFlow NetFlow7;

static unsigned promisc;
static char *dev;
static char *filter = "";
static unsigned scan_interval = 5;
static int frag_lifetime = 30;
static int inactive_lifetime = 60;
static int active_lifetime = 300;
static int sockbufsize;
#define BULK_QUANTITY_MAX (unsigned)(mem_index_t)(-1)
#if (MEM_BITS == 0) || (MEM_BITS == 16)
#define BULK_QUANTITY 10000
#else
#define BULK_QUANTITY 200
#endif
static unsigned bulk_quantity = BULK_QUANTITY;
static unsigned pending_queue_length = 100;
static struct NetFlow *netflow = &NetFlow5;
static unsigned verbosity = 6;
static unsigned log_dest = MY_LOG_SYSLOG;
static unsigned snmp_input_index;
static unsigned snmp_output_index;
static int link_layer, link_layer_size = -1;
static struct Time start_time;
static long start_time_offset;
static int off_nl, off_tl;
/* From mem.c */
extern unsigned total_elements;
extern unsigned free_elements;
extern unsigned total_memory;
#if ((DEBUG) & DEBUG_I)
static unsigned emit_pkts, emit_queue;
static uint64_t size_total;
static unsigned pkts_total, pkts_total_fragmented;
static unsigned pkts_ignored, pkts_lost_capture, pkts_lost_unpending;
static unsigned pkts_pending, pkts_pending_done;
static unsigned pending_queue_trace, pending_queue_trace_candidate;
static unsigned flows_total, flows_fragmented;
static unsigned pkts_dropped;
static struct pcap_stat pstat;
#endif
static unsigned emit_count;
static uint32_t emit_sequence;
static unsigned emit_rate_bytes, emit_rate_delay;
static struct Time emit_time;
static uint8_t emit_packet[NETFLOW_MAX_PACKET];
static pcap_t* pcap_handle;
static pthread_t thid;
static sigset_t sig_mask;
static struct sched_param schedp;
static int sched_min, sched_max;
static int npeers, npeers_rot;
static struct peer *peers;
static int sigs;

static struct Flow *flows[1 << HASH_BITS];
static pthread_mutex_t flows_mutex[1 << HASH_BITS];

static pthread_mutex_t unpending_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t unpending_cond = PTHREAD_COND_INITIALIZER;

static pthread_mutex_t scan_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t scan_cond = PTHREAD_COND_INITIALIZER;
static struct Flow *pending_head, *pending_tail;
static struct Flow *scan_frag_dreg;

static pthread_mutex_t emit_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t emit_cond = PTHREAD_COND_INITIALIZER;
static struct Flow *flows_emit;

static char ident[256] = "fprobe";
static FILE *pidfile;
static char *pidfilepath;
static pid_t pid;
static int killed;
static int emit_timeout = EMIT_TIMEOUT, unpending_timeout = UNPENDING_TIMEOUT;

static struct passwd *pw = 0;

void usage()
{
	fprintf(stdout,
		"fprobe: a NetFlow probe. Version %s\n"
		"Usage: fprobe [options] remote:port[/[local][/type]] ...\n"
		"\n"
		"-h\t\tDisplay this help\n"
		"-p\t\tDon't put the interface into promiscuous mode\n"
		"-i <interface>\tNetwork interface name\n"
		"-f <expression>\tFilter expression (see tcpdump manual for details)\n"
		"-s <seconds>\tHow often scan for expired flows [5]\n"
		"-g <seconds>\tFragmented flow lifetime [30]\n"
		"-d <seconds>\tIdle flow lifetime (inactive timer) [60]\n"
		"-e <seconds>\tActive flow lifetime (active timer) [300]\n"
		"-n <version>\tNetFlow version for use (1, 5 or 7) [5]\n"
		"-a <address>\tUse <address> as source for NetFlow flow\n"
		"-x <id>[:<id>]\tWorkaround for SNMP interfaces indexes [0]\n"
		"-b <flows>\tMemory bulk size (1..%u) [%u]\n"
		"-m <kilobytes>\tMemory limit (0=no limit) [0]\n"
		"-q <flows>\tPending queue length [100]\n"
		"-B <kilobytes>\tKernel capture buffer size [0]\n"
		"-r <priority>\tReal-time priority (0=disabled, %d..%d) [0]\n"
		"-t <B:N>\tProduce <N> nanosecond delay after each <B> bytes sent [0:0]\n"
		"-S <bytes>\tSnaplen [256]\n"
		"-K <bytes>\tLink layer header size\n"
		"-k\t\tDon't exclude link layer header from packet size\n"
		"-c <directory>\tDirectory to chroot to\n"
		"-u <user>\tUser to run as\n"
		"-v <level>\tMaximum log level (0=EMERG, ..., 6=INFO, 7=DEBUG) [6]\n"
		"-l <[dst][:id]>\tLog destination and log/pidfile idetifier [1]\n"
		"remote:port\tAddress of the NetFlow collector\n",
		VERSION, BULK_QUANTITY_MAX, bulk_quantity, sched_min, sched_max);
	exit(0);
}

#if ((DEBUG) & DEBUG_I)
void info_debug()
{
	pcap_stats(pcap_handle, &pstat);
	pkts_dropped += pstat.ps_drop;
	my_log(LOG_DEBUG, "I: received:%d/%d (%lld) pending:%d/%d",
		pkts_total, pkts_total_fragmented, size_total,
		pkts_pending - pkts_pending_done, pending_queue_trace);
	my_log(LOG_DEBUG, "I: ignored:%d lost:%d+%d dropped:%d",
		pkts_ignored, pkts_lost_capture, pkts_lost_unpending, pkts_dropped);
	my_log(LOG_DEBUG, "I: cache:%d/%d emit:%d/%d/%d",
		flows_total, flows_fragmented, emit_sequence, emit_pkts, emit_queue);
	my_log(LOG_DEBUG, "I: memory:%d/%d (%d)",
		total_elements, free_elements, total_memory);
}
#endif

void sighandler(int sig)
{
	switch (sig) {
		case SIGTERM:
			sigs |= SIGTERM_MASK;
			break;
#if ((DEBUG) & DEBUG_I)
		case SIGUSR1:
			sigs |= SIGUSR1_MASK;
			break;
#endif
	}
}

void gettime(struct Time *now)
{
	struct timeval t;

	gettimeofday(&t, 0);
	now->sec = t.tv_sec;
	now->usec = t.tv_usec;
}

inline time_t cmpmtime(struct Time *t1, struct Time *t2)
{
	return (t1->sec - t2->sec) * 1000 + (t1->usec - t2->usec) / 1000;
}

/* Uptime in miliseconds */
uint32_t getuptime(struct Time *t)
{
	/* Maximum uptime is about 49/2 days */
	return cmpmtime(t, &start_time);
}

hash_t hash_flow(struct Flow *flow)
{
	if (flow->flags & FLOW_FRAG) return hash(flow, sizeof(struct Flow_F));
	else return hash(flow, sizeof(struct Flow_TL));
}

inline void copy_flow(struct Flow *src, struct Flow *dst)
{
	dst->sip = src->sip;
	dst->dip = src->dip;
	dst->tos = src->tos;
	dst->proto = src->proto;
	dst->tcp_flags = src->tcp_flags;
	dst->id = src->id;
	dst->sp = src->sp;
	dst->dp = src->dp;
	dst->pkts = src->pkts;
	dst->size = src->size;
	dst->sizeF = src->sizeF;
	dst->sizeP = src->sizeP;
	dst->ctime = src->ctime;
	dst->mtime = src->mtime;
	dst->flags = src->flags;
}

struct Flow *find(struct Flow *where, struct Flow *what, struct Flow ***prev)
{
	struct Flow **flowpp;

#ifdef WALL
	flowpp = 0;
#endif

	if (prev) flowpp = *prev;

	while (where) {
		if (where->sip.s_addr == what->sip.s_addr
			&& where->dip.s_addr == what->dip.s_addr
			&& where->proto == what->proto) {
			switch ((what->flags + where->flags) & FLOW_FRAGMASK) {
				case 0:
					/* Both unfragmented */
					if ((what->sp == where->sp)
						&& (what->dp == where->dp)) goto done;
					break;
				case 2:
					/* Both fragmented */
					if (where->id == what->id) goto done;
					break;
			}
		}
		flowpp = &where->next;
		where = where->next;
	}
done:
	if (prev) *prev = flowpp;
	return where;
}

int put_into(struct Flow *flow, int flag
#if ((DEBUG) & (DEBUG_S | DEBUG_U))
	, char *logbuf
#endif
)
{
	int ret = 0;
	hash_t h;
	struct Flow *flown, **flowpp;
#if ((DEBUG) & (DEBUG_S | DEBUG_U))
	char buf[64];
#endif

	h = hash_flow(flow);
#if ((DEBUG) & (DEBUG_S | DEBUG_U))
	sprintf(buf, " %x H:%04x", (unsigned) flow, h);
	strcat(logbuf, buf);
#endif
	pthread_mutex_lock(&flows_mutex[h]);
	flowpp = &flows[h];
	if (!(flown = find(flows[h], flow, &flowpp))) {
		/* No suitable flow found - add */
		if (flag == COPY_INTO) {
			if ((flown = mem_alloc())) {
				copy_flow(flow, flown);
				flow = flown;
			} else {
#if ((DEBUG) & (DEBUG_S | DEBUG_U)) || defined MESSAGES
				my_log(LOG_ERR, "%s %s. %s",
					"mem_alloc():", strerror(errno), "packet lost");
#endif
				return -1;
			}
		}
		flow->next = flows[h];
		flows[h] = flow;
#if ((DEBUG) & DEBUG_I)
		flows_total++;
		if (flow->flags & FLOW_FRAG) flows_fragmented++;
#endif
#if ((DEBUG) & (DEBUG_S | DEBUG_U))
		if (flown) {
			sprintf(buf, " => %x, flags: %x", (unsigned) flown, flown->flags);
			strcat(logbuf, buf);
		}
#endif
	} else {
		/* Found suitable flow - update */
#if ((DEBUG) & (DEBUG_S | DEBUG_U))
		sprintf(buf, " +> %x", (unsigned) flown);
		strcat(logbuf, buf);
#endif
		if (cmpmtime(&flow->mtime, &flown->mtime) > 0)
			flown->mtime = flow->mtime;
		if (cmpmtime(&flow->ctime, &flown->ctime) < 0)
			flown->ctime = flow->ctime;
		flown->tcp_flags |= flow->tcp_flags;
		flown->size += flow->size;
		flown->pkts += flow->pkts;
		if (flow->flags & FLOW_FRAG) {
			/* Fragmented flow require some additional work */
			if (flow->flags & FLOW_TL) {
				/*
				?FIXME?
				Several packets with FLOW_TL (attack)
				*/
				flown->sp = flow->sp;
				flown->dp = flow->dp;
			}
			if (flow->flags & FLOW_LASTFRAG) {
				/*
				?FIXME?
				Several packets with FLOW_LASTFRAG (attack)
				*/
				flown->sizeP = flow->sizeP;
			}
			flown->flags |= flow->flags;
			flown->sizeF += flow->sizeF;
			if ((flown->flags & FLOW_LASTFRAG)
				&& (flown->sizeF >= flown->sizeP)) {
				/* All fragments received - flow reassembled */
				*flowpp = flown->next;
				pthread_mutex_unlock(&flows_mutex[h]);
#if ((DEBUG) & DEBUG_I)
				flows_total--;
				flows_fragmented--;
#endif
				flown->id = 0;
				flown->flags &= ~FLOW_FRAG;
#if ((DEBUG) & (DEBUG_U | DEBUG_S))
				strcat(logbuf," R");
#endif
				ret = put_into(flown, MOVE_INTO
#if ((DEBUG) & (DEBUG_U | DEBUG_S))
						, logbuf
#endif
					);
			}
		}
		if (flag == MOVE_INTO) mem_free(flow);
	}
	pthread_mutex_unlock(&flows_mutex[h]);
	return ret;
}

void *fill(int fields, uint16_t *format, struct Flow *flow, void *p)
{
	int i;

	for (i = 0; i < fields; i++) {
#if ((DEBUG) & DEBUG_F)
		my_log(LOG_DEBUG, "F: field %04d at %x", format[i], (unsigned) p);
#endif
		switch (format[i]) {
			case NETFLOW_IPV4_SRC_ADDR:
				((struct in_addr *) p)->s_addr = flow->sip.s_addr;
				p += NETFLOW_IPV4_SRC_ADDR_SIZE;
				break;

			case NETFLOW_IPV4_DST_ADDR:
				((struct in_addr *) p)->s_addr = flow->dip.s_addr;
				p += NETFLOW_IPV4_DST_ADDR_SIZE;
				break;

			case NETFLOW_INPUT_SNMP:
				*((uint16_t *) p) = snmp_input_index;
				p += NETFLOW_INPUT_SNMP_SIZE;
				break;

			case NETFLOW_OUTPUT_SNMP:
				*((uint16_t *) p) = snmp_output_index;
				p += NETFLOW_OUTPUT_SNMP_SIZE;
				break;

			case NETFLOW_PKTS_32:
				*((uint32_t *) p) = htonl(flow->pkts);
				p += NETFLOW_PKTS_32_SIZE;
				break;

			case NETFLOW_BYTES_32:
				*((uint32_t *) p) = htonl(flow->size);
				p += NETFLOW_BYTES_32_SIZE;
				break;

			case NETFLOW_FIRST_SWITCHED:
				*((uint32_t *) p) = htonl(getuptime(&flow->ctime));
				p += NETFLOW_FIRST_SWITCHED_SIZE;
				break;

			case NETFLOW_LAST_SWITCHED:
				*((uint32_t *) p) = htonl(getuptime(&flow->mtime));
				p += NETFLOW_LAST_SWITCHED_SIZE;
				break;

			case NETFLOW_L4_SRC_PORT:
				*((uint16_t *) p) = flow->sp;
				p += NETFLOW_L4_SRC_PORT_SIZE;
				break;

			case NETFLOW_L4_DST_PORT:
				*((uint16_t *) p) = flow->dp;
				p += NETFLOW_L4_DST_PORT_SIZE;
				break;

			case NETFLOW_PROT:
				*((uint8_t *) p) = flow->proto;
				p += NETFLOW_PROT_SIZE;
				break;

			case NETFLOW_SRC_TOS:
				*((uint8_t *) p) = flow->tos;
				p += NETFLOW_SRC_TOS_SIZE;
				break;

			case NETFLOW_TCP_FLAGS:
				*((uint8_t *) p) = flow->tcp_flags;
				p += NETFLOW_TCP_FLAGS_SIZE;
				break;

			case NETFLOW_VERSION:
				*((uint16_t *) p) = htons(netflow->Version);
				p += NETFLOW_VERSION_SIZE;
				break;

			case NETFLOW_COUNT:
				*((uint16_t *) p) = htons(emit_count);
				p += NETFLOW_COUNT_SIZE;
				break;

			case NETFLOW_UPTIME:
				*((uint32_t *) p) = htonl(getuptime(&emit_time));
				p += NETFLOW_UPTIME_SIZE;
				break;

			case NETFLOW_UNIX_SECS:
				*((uint32_t *) p) = htonl(emit_time.sec);
				p += NETFLOW_UNIX_SECS_SIZE;
				break;

			case NETFLOW_UNIX_NSECS:
				*((uint32_t *) p) = htonl(emit_time.usec * 1000);
				p += NETFLOW_UNIX_NSECS_SIZE;
				break;

			case NETFLOW_FLOW_SEQUENCE:
				//*((uint32_t *) p) = htonl(emit_sequence);
				*((uint32_t *) p) = 0;
				p += NETFLOW_FLOW_SEQUENCE_SIZE;
				break;

			case NETFLOW_PAD8:
			/* Unsupported (uint8_t) */
			case NETFLOW_ENGINE_TYPE:
			case NETFLOW_ENGINE_ID:
			case NETFLOW_FLAGS7_1:
			case NETFLOW_SRC_MASK:
			case NETFLOW_DST_MASK:
				*((uint8_t *) p) = 0;
				p += NETFLOW_PAD8_SIZE;
				break;

			case NETFLOW_PAD16:
			/* Unsupported (uint16_t) */
			case NETFLOW_SRC_AS:
			case NETFLOW_DST_AS:
			case NETFLOW_FLAGS7_2:
				*((uint16_t *) p) = 0;
				p += NETFLOW_PAD16_SIZE;
				break;

			case NETFLOW_PAD32:
			/* Unsupported (uint32_t) */
			case NETFLOW_IPV4_NEXT_HOP:
			case NETFLOW_ROUTER_SC:
				*((uint32_t *) p) = 0;
				p += NETFLOW_PAD32_SIZE;
				break;

			default:
				my_log(LOG_CRIT, "fill(): Unknown format at %x[%d]: %d",
					format, i, format[i]);
				exit(1);
		}
	}
#if ((DEBUG) & DEBUG_F)
	my_log(LOG_DEBUG, "F: return %x", (unsigned) p);
#endif
	return p;
}

#ifdef CLONEBASED_THREADS
void setuser() {
	/*
	Workaround for clone()-based threads
	Try to change EUID independently of main thread
	*/
	if (pw) {
		setgroups(0, NULL);
		setregid(pw->pw_gid, pw->pw_gid);
		setreuid(pw->pw_uid, pw->pw_uid);
	}
}
#endif

void *emit_thread()
{
	struct Flow *flow;
	void *p;
	struct timeval now;
	struct timespec timeout;
	int i, ret, sent = 0, size, delay, peer_rot_cur, peer_rot_work = 0;

#ifdef CLONEBASED_THREADS
	setuser();
#endif

	p = (void *) &emit_packet + netflow->HeaderSize;
	timeout.tv_nsec = 0;

	for (;;) {
		pthread_mutex_lock(&emit_mutex);
		while (!flows_emit) {
			gettimeofday(&now, 0);
			timeout.tv_sec = now.tv_sec + emit_timeout;
			/* Do not wait until emit_packet will filled - it may be too long */
			if (pthread_cond_timedwait(&emit_cond, &emit_mutex, &timeout) && emit_count) {
				pthread_mutex_unlock(&emit_mutex);
				goto sendit;
			}
		}
		flow = flows_emit;
		flows_emit = flows_emit->next;
#if ((DEBUG) & DEBUG_I)
		emit_queue--;
#endif		
		pthread_mutex_unlock(&emit_mutex);

#ifdef UPTIME_TRICK
		if (!emit_count) {
			gettime(&start_time);
			start_time.sec -= start_time_offset;
		}
#endif
		p = fill(netflow->FlowFields, netflow->FlowFormat, flow, p);
		mem_free(flow);
		emit_count++;
		if (emit_count == netflow->MaxFlows) {
		sendit:
			gettime(&emit_time);
			p = fill(netflow->HeaderFields, netflow->HeaderFormat, 0, &emit_packet);
			size = netflow->HeaderSize + emit_count * netflow->FlowSize;
			peer_rot_cur = 0;
			for (i = 0; i < npeers; i++) {
				if (peers[i].type == PEER_MIRROR) goto sendreal;
				if (peers[i].type == PEER_ROTATE) 
					if (peer_rot_cur++ == peer_rot_work) {
					sendreal:
						if (netflow->SeqOffset)
							*((uint32_t *) (emit_packet + netflow->SeqOffset)) = htonl(peers[i].seq);
						ret = send(peers[i].sock, emit_packet, size, 0);
						if (ret < size) {
#if ((DEBUG) & DEBUG_E) || defined MESSAGES
							my_log(LOG_ERR, "send(to #%d, seq %d, flows %d, size %d) == %d: %s",
								i + 1, peers[i].seq, emit_count, size, ret, strerror(errno));
#endif
						}
#if ((DEBUG) & DEBUG_E)
						else {
							my_log(LOG_DEBUG, "E: Emitted %d flow(s) to #%d, seq %d",
								emit_count, i + 1, peers[i].seq);
						}
#endif
						peers[i].seq += emit_count;

						/* Rate limit */
						if (emit_rate_bytes) {
							sent += size;
							delay = sent / emit_rate_bytes;
							if (delay) {
								sent %= emit_rate_bytes;
								timeout.tv_sec = 0;
								timeout.tv_nsec = emit_rate_delay * delay;
								while (nanosleep(&timeout, &timeout) == -1 && errno == EINTR);
							}
						}
					}
			}
			if (npeers_rot) peer_rot_work = (peer_rot_work + 1) % npeers_rot;
			emit_sequence += emit_count;
			emit_count = 0;
#if ((DEBUG) & DEBUG_I)
			emit_pkts++;
#endif
		}
	}
}	

void *unpending_thread()
{
	struct timeval now;
	struct timespec timeout;
#if ((DEBUG) & (DEBUG_S | DEBUG_U))
	char logbuf[256];
#endif

#ifdef CLONEBASED_THREADS
	setuser();
#endif

	timeout.tv_nsec = 0;
	pthread_mutex_lock(&unpending_mutex);

	for (;;) {
		while (!(pending_tail->flags & FLOW_PENDING)) {
			gettimeofday(&now, 0);
			timeout.tv_sec = now.tv_sec + unpending_timeout;
			pthread_cond_timedwait(&unpending_cond, &unpending_mutex, &timeout);
		}

#if ((DEBUG) & (DEBUG_S | DEBUG_U))
		*logbuf = 0;
#endif
		if (put_into(pending_tail, COPY_INTO
#if ((DEBUG) & (DEBUG_S | DEBUG_U))
				, logbuf
#endif
			) < 0) {
#if ((DEBUG) & DEBUG_I)
			pkts_lost_unpending++;
#endif				
		}

#if ((DEBUG) & DEBUG_U)
		my_log(LOG_DEBUG, "%s%s", "U:", logbuf);
#endif

		pending_tail->flags = 0;
		pending_tail = pending_tail->next;
#if ((DEBUG) & DEBUG_I)
		pkts_pending_done++;
#endif
	}
}

void *scan_thread()
{
#if ((DEBUG) & (DEBUG_S | DEBUG_U))
	char logbuf[256];
#endif
	int i;
	struct Flow *flow, **flowpp;
	struct Time now;
	struct timespec timeout;

#ifdef CLONEBASED_THREADS
	setuser();
#endif

	timeout.tv_nsec = 0;
	pthread_mutex_lock(&scan_mutex);

	for (;;) {
		gettime(&now);
		timeout.tv_sec = now.sec + scan_interval;
		pthread_cond_timedwait(&scan_cond, &scan_mutex, &timeout);

		gettime(&now);
#if ((DEBUG) & DEBUG_S)
		my_log(LOG_DEBUG, "S: %d", now.sec);
#endif
		for (i = 0; i < 1 << HASH_BITS ; i++) {
			pthread_mutex_lock(&flows_mutex[i]);
			flow = flows[i];
			flowpp = &flows[i];
			while (flow) {
				if (flow->flags & FLOW_FRAG) {
					/* Process fragmented flow */
					if ((now.sec - flow->mtime.sec) > frag_lifetime) {
						/* Fragmented flow expired - put it into special chain */
#if ((DEBUG) & DEBUG_I)
						flows_fragmented--;
						flows_total--;
#endif
						*flowpp = flow->next;
						flow->id = 0;
						flow->flags &= ~FLOW_FRAG;
						flow->next = scan_frag_dreg;
						scan_frag_dreg = flow;
						flow = *flowpp;
						continue;
					}
				} else {
					/* Flow is not frgamented */
					if ((now.sec - flow->mtime.sec) > inactive_lifetime
						|| (flow->mtime.sec - flow->ctime.sec) > active_lifetime) {
						/* Flow expired */
#if ((DEBUG) & DEBUG_S)
						my_log(LOG_DEBUG, "S: E %x", flow);
#endif
#if ((DEBUG) & DEBUG_I)
						flows_total--;
#endif
						*flowpp = flow->next;
						pthread_mutex_lock(&emit_mutex);
						flow->next = flows_emit;
						flows_emit = flow;
#if ((DEBUG) & DEBUG_I)
						emit_queue++;
#endif				
						pthread_mutex_unlock(&emit_mutex);
						flow = *flowpp;
						continue;
					}
				}
				flowpp = &flow->next;
				flow = flow->next;
			} /* chain loop */
			pthread_mutex_unlock(&flows_mutex[i]);
		} /* hash loop */
		if (flows_emit) pthread_cond_signal(&emit_cond);

		while (scan_frag_dreg) {
			flow = scan_frag_dreg;
			scan_frag_dreg = flow->next;
#if ((DEBUG) & (DEBUG_S | DEBUG_U))
			*logbuf = 0;
#endif
			put_into(flow, MOVE_INTO
#if ((DEBUG) & (DEBUG_S | DEBUG_U))
				, logbuf
#endif
			);
#if ((DEBUG) & DEBUG_S)
			my_log(LOG_DEBUG, "%s%s", "S: FE", logbuf);
#endif
		}
	}
}

void pcap_callback(u_char *useless,
	const struct pcap_pkthdr* pkthdr, const u_char *packet)
{
	struct ip *nl;
	void *tl;
	struct Flow *flow;
	int off_frag, psize;
#if ((DEBUG) & DEBUG_C)
	char buf[64];
	char logbuf[256];
#endif

	if (killed) return; /* SIGTERM received - stop capturing */

#if ((DEBUG) & DEBUG_C)
	sprintf(logbuf, "C: %d/%d", pkthdr->caplen, pkthdr->len);
#endif

	/* Offset (from begin of captured packet) to network layer header */
	nl = (void *) packet + off_nl;
	psize = pkthdr->caplen - off_nl;

	/* Sanity check */
	if (psize < (signed) sizeof(struct ip) || nl->ip_v != 4) {
#if ((DEBUG) & DEBUG_C)
		strcat(logbuf, " U");
		my_log(LOG_DEBUG, "%s", logbuf);
#endif
#if ((DEBUG) & DEBUG_I)
		pkts_ignored++;
#endif
		return;
	}

	if (pending_head->flags) {
#if ((DEBUG) & DEBUG_C) || defined MESSAGES
		my_log(LOG_ERR,
# if ((DEBUG) & DEBUG_C)
			"%s %s %s", logbuf,
# else
			"%s %s",
# endif
			"pending queue full:", "packet lost");
#endif
#if ((DEBUG) & DEBUG_I)
		pkts_lost_capture++;
#endif
		goto done;
	}

#if ((DEBUG) & DEBUG_I)
	pkts_total++;
#endif

	flow = pending_head;

	/* ?FIXME? Add sanity check for ip_len? */
	flow->size = link_layer ? pkthdr->len : ntohs(nl->ip_len);
#if ((DEBUG) & DEBUG_I)
	size_total += flow->size;
#endif

	flow->sip = nl->ip_src;
	flow->dip = nl->ip_dst;
	flow->tos = nl->ip_tos;
	flow->proto = nl->ip_p;
	flow->id = 0;
	flow->tcp_flags = 0;
	flow->pkts = 1;
	flow->sizeF = 0;
	flow->sizeP = 0;
	flow->ctime.sec = pkthdr->ts.tv_sec;
	flow->ctime.usec = pkthdr->ts.tv_usec;
	flow->mtime = flow->ctime;

	off_frag = (ntohs(nl->ip_off) & IP_OFFMASK) << 3;

	/*
	Offset (from network layer) to transport layer header/IP data
	IOW IP header size ;-)

	?FIXME?
	Check ip_hl for valid value (>=5)? Maybe check CRC? No, thanks...
	*/
	off_tl = nl->ip_hl << 2;
	tl = (void *) nl + off_tl;

	/* THIS packet data size: data_size = total_size - ip_header_size*4 */
	flow->sizeF = ntohs(nl->ip_len) - off_tl;
	psize -= off_tl;
	if ((signed) flow->sizeF < 0) flow->sizeF = 0;
	if (psize > (signed) flow->sizeF) psize = flow->sizeF;

	if (ntohs(nl->ip_off) & (IP_MF | IP_OFFMASK)) {
		/* Fragmented packet (IP_MF flag == 1 or fragment offset != 0) */
#if ((DEBUG) & DEBUG_C)
		strcat(logbuf, " F");
#endif
#if ((DEBUG) & DEBUG_I)
		pkts_total_fragmented++;
#endif
		flow->flags |= FLOW_FRAG;
		flow->id = nl->ip_id;

		if (!(ntohs(nl->ip_off) & IP_MF)) {
			/* Packet whith IP_MF contains information about whole datagram size */
			flow->flags |= FLOW_LASTFRAG;
			/* size = frag_offset*8 + data_size */
			flow->sizeP = off_frag + flow->sizeF;
		}
	}

#if ((DEBUG) & DEBUG_C)
	sprintf(buf, " %s>", inet_ntoa(flow->sip));
	strcat(logbuf, buf);
	sprintf(buf, "%s P:%x", inet_ntoa(flow->dip), flow->proto);
	strcat(logbuf, buf);
#endif

	/*
	Fortunately most interesting transport layer information fit
	into first 8 bytes of IP data field (minimal nonzero size).
	Thus we don't need actual packet reassembling to build whole
	transport layer data. We only check the fragment offset for
	zero value to find packet with this information.
	*/
	if (!off_frag && psize >= 8) {
		switch (flow->proto) {
			case IPPROTO_TCP:
			case IPPROTO_UDP:
				flow->sp = ((struct udphdr *)tl)->source;
				flow->dp = ((struct udphdr *)tl)->dest;
				goto tl_known;

#ifdef ICMP_TRICK
			case IPPROTO_ICMP:
				flow->sp = htons(((struct icmp *)tl)->icmp_type);
				flow->dp = htons(((struct icmp *)tl)->icmp_code);
				goto tl_known;
#endif
#ifdef ICMP_TRICK_CISCO
			case IPPROTO_ICMP:
				flow->dp = *((int32_t *) tl);
				goto tl_known;
#endif

			default:
				/* Unknown transport layer */
#if ((DEBUG) & DEBUG_C)
				strcat(logbuf, " U");
#endif
				flow->sp = 0;
				flow->dp = 0;
				break;

			tl_known:
#if ((DEBUG) & DEBUG_C)
				sprintf(buf, " %d>%d", ntohs(flow->sp), ntohs(flow->dp));
				strcat(logbuf, buf);
#endif
				flow->flags |= FLOW_TL;
		}
	}

	/* Check for tcp flags presence (including CWR and ECE). */
	if (flow->proto == IPPROTO_TCP
		&& off_frag < 16
		&& psize >= 16 - off_frag) {
		flow->tcp_flags = *((uint8_t *)(tl + 13 - off_frag));
#if ((DEBUG) & DEBUG_C)
		sprintf(buf, " TCP:%x", flow->tcp_flags);
		strcat(logbuf, buf);
#endif
	}

#if ((DEBUG) & DEBUG_C)
	sprintf(buf, " => %x", (unsigned) flow);
	strcat(logbuf, buf);
	my_log(LOG_DEBUG, "%s", logbuf);
#endif

#if ((DEBUG) & DEBUG_I)
	pkts_pending++;
	pending_queue_trace_candidate = pkts_pending - pkts_pending_done;
	if (pending_queue_trace < pending_queue_trace_candidate)
		pending_queue_trace = pending_queue_trace_candidate;
#endif

	/* Flow complete - inform unpending_thread() about it */
	pending_head->flags |= FLOW_PENDING;
	pending_head = pending_head->next;
done:
	pthread_cond_signal(&unpending_cond);
#ifdef WALL
	return;
	useless = 0;
#endif
}

void *pcap_thread()
{

#ifdef CLONEBASED_THREADS
	setuser();
#endif

	pcap_loop(pcap_handle, -1, pcap_callback, 0);
	my_log(LOG_INFO, "pcap_loop() terminated: %s", strerror(errno));
	kill(pid, SIGTERM); /* Suicide */
#ifdef WALL
	return 0;
#endif
}

int main(int argc, char **argv)
{
	char errbuf[PCAP_ERRBUF_SIZE >= 128 ? PCAP_ERRBUF_SIZE : 128];
	struct bpf_program bpf_filter;
	char *dhost, *dport, *lhost, *type = 0, *log_suffix = 0;
	int c, i, sock, memory_limit = 0, link_type, link_type_idx, snaplen;
	struct addrinfo hints, *res;
	struct sockaddr_in saddr;
	pthread_attr_t tattr;
	struct sigaction sigact;
	static void *threads[THREADS - 1] = {&emit_thread, &scan_thread, &unpending_thread, &pcap_thread};
	struct timeval timeout;

#ifdef WALL
	link_type_idx = 0;
#endif

	sched_min = sched_get_priority_min(SCHED);
	sched_max = sched_get_priority_max(SCHED);

	memset(&saddr, 0 , sizeof(saddr));
	memset(&hints, 0 , sizeof(hints));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	/* Process command line options */

	opterr = 0;
	while ((c = my_getopt(argc, argv, parms)) != -1) {
		switch (c) {
			case '?':
				usage();

			case 'h':
				usage();
		}
	}

	promisc = -(--parms[pflag].count);
	dev = parms[iflag].arg;
	if (dev) if (*dev == '-') log_dest = 2;
	if (parms[Sflag].count) snaplen = atoi(parms[Sflag].arg);
	else snaplen = 256;
	if (parms[sflag].count) scan_interval = atoi(parms[sflag].arg);
	if (parms[gflag].count) frag_lifetime = atoi(parms[gflag].arg);
	if (parms[dflag].count) inactive_lifetime = atoi(parms[dflag].arg);
	if (parms[eflag].count) active_lifetime = atoi(parms[eflag].arg);
	if (parms[nflag].count) {
		switch (atoi(parms[nflag].arg)) {
			case 1:
				netflow = &NetFlow1;
				break;

			case 5:
				break;

			case 7:
				netflow = &NetFlow7;
				break;

			default:
				fprintf(stderr, "Illegal %s\n", "NetFlow version");
				exit(1);
		}
	}
	if (parms[vflag].count) verbosity = atoi(parms[vflag].arg);
	if (parms[lflag].count) {
		if ((log_suffix = strchr(parms[lflag].arg, ':'))) {
			*log_suffix++ = 0;
			if (*log_suffix) {
				sprintf(errbuf, "[%s]", log_suffix);
				strcat(ident, errbuf);
			}
		}
		if (*parms[lflag].arg) log_dest = atoi(parms[lflag].arg);
		if (log_suffix) *--log_suffix = ':';
	}
	if (!(pidfilepath = malloc(sizeof(PID_DIR) + 1 + strlen(ident) + 1 + 3 + 1))) {
	err_malloc:
		fprintf(stderr, "malloc(): %s\n", strerror(errno));
		exit(1);
	}
	sprintf(pidfilepath, "%s/%s.pid", PID_DIR, ident);
	if (parms[qflag].count) {
		pending_queue_length = atoi(parms[qflag].arg);
		if (pending_queue_length < 1) {
			fprintf(stderr, "Illegal %s\n", "pending queue length");
			exit(1);
		}
	}
	if (parms[rflag].count) {
		schedp.sched_priority = atoi(parms[rflag].arg);
		if (schedp.sched_priority
			&& (schedp.sched_priority < sched_min
				|| schedp.sched_priority > sched_max)) {
			fprintf(stderr, "Illegal %s\n", "realtime priority");
			exit(1);
		}
	}
	if (parms[Bflag].count)
		sockbufsize = atoi(parms[Bflag].arg) << 10;
	if (parms[bflag].count) {
		bulk_quantity = atoi(parms[bflag].arg);
		if (bulk_quantity < 1 || bulk_quantity > BULK_QUANTITY_MAX) {
			fprintf(stderr, "Illegal %s\n", "bulk size");
			exit(1);
		}
	}
	if (parms[mflag].count) memory_limit = atoi(parms[mflag].arg) << 10;
	if (parms[xflag].count)
		if ((sscanf(parms[xflag].arg, "%d:%d", &snmp_input_index, &snmp_output_index)) == 1)
			snmp_output_index = snmp_input_index;
	if (parms[tflag].count)
		sscanf(parms[tflag].arg, "%d:%d", &emit_rate_bytes, &emit_rate_delay);
	if (parms[aflag].count) {
		if (getaddrinfo(parms[aflag].arg, 0, &hints, &res)) {
		bad_lhost:
			fprintf(stderr, "Illegal %s\n", "source address");
			exit(1);
		} else {
			saddr = *((struct sockaddr_in *) res->ai_addr);
			freeaddrinfo(res);
		}
	}
	if (parms[Kflag].count) link_layer_size = atoi(parms[Kflag].arg);
	link_layer = parms[kflag].count;
	if (parms[uflag].count) 
		if ((pw = getpwnam(parms[uflag].arg)) == NULL) {
			fprintf(stderr, "getpwnam(%s): %s\n", parms[uflag].arg, errno ? strerror(errno) : "Unknown user");
			exit(1);
		}

	/* Process collectors parameters. Brrrr... :-[ */

	npeers = argc - optind;
	if (npeers < 1) usage();
	if (!(peers = malloc(npeers * sizeof(struct peer)))) goto err_malloc;
	for (i = optind, npeers = 0; i < argc; i++, npeers++) {
		dhost = argv[i];
		if (!(dport = strchr(dhost, ':'))) goto bad_collector;
		*dport++ = 0;
		if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
			fprintf(stderr, "socket(): %s\n", strerror(errno));
			exit(1);
		}
		peers[npeers].sock = sock;
		peers[npeers].type = PEER_MIRROR;
		peers[npeers].laddr = saddr;
		peers[npeers].seq = 0;
		if ((lhost = strchr(dport, '/'))) {
			*lhost++ = 0;
			if ((type = strchr(lhost, '/'))) {
				*type++ = 0;
				switch (*type) {
					case 0:
					case 'm':
						break;

					case 'r':
						peers[npeers].type = PEER_ROTATE;
						npeers_rot++;
						break;

					default:
						goto bad_collector;
				}
			}
			if (*lhost) {
				if (getaddrinfo(lhost, 0, &hints, &res)) goto bad_lhost;
				peers[npeers].laddr = *((struct sockaddr_in *) res->ai_addr);
				freeaddrinfo(res);
			}
		}
		if (bind(sock, (struct sockaddr *) &peers[npeers].laddr,
				sizeof(struct sockaddr_in))) {
			fprintf(stderr, "bind(): %s\n", strerror(errno));
			exit(1);
		}
		if (getaddrinfo(dhost, dport, &hints, &res)) {
		bad_collector:
			fprintf(stderr, "Error in collector #%d parameters\n", npeers + 1);
			exit(1);
		}
		peers[npeers].addr = *((struct sockaddr_in *) res->ai_addr);
		freeaddrinfo(res);
		if (connect(sock, (struct sockaddr *) &peers[npeers].addr,
				sizeof(struct sockaddr_in))) {
			fprintf(stderr, "connect(): %s\n", strerror(errno));
			exit(1);
		}

		/* Restore command line */
		if (type) *--type = '/';
		if (lhost) *--lhost = '/';
		*--dport = ':';
	}

	/* Daemonize (if log destination stdout-free) */

	my_log_open(ident, verbosity, log_dest);
	if (!(log_dest & 2)) {
		switch (fork()) {
			case -1:
				fprintf(stderr, "fork(): %s", strerror(errno));
				exit(1);

			case 0:
				setsid();
				freopen("/dev/null", "r", stdin);
				freopen("/dev/null", "w", stdout);
				freopen("/dev/null", "w", stderr);
				break;

			default:
				exit(0);
		}
	} else {
		setvbuf(stdout, (char *)0, _IONBF, 0);
		setvbuf(stderr, (char *)0, _IONBF, 0);
	}

	pid = getpid();
	sprintf(errbuf, "[%ld]", (long) pid);
	strcat(ident, errbuf);

	/* Initialization */

    if (!dev)
		if (!(dev = pcap_lookupdev(errbuf))) {
			my_log(LOG_CRIT, "pcap_lookupdev(): %s\n", errbuf);
			exit(1);
		}

	if (*dev == '-') {
		pcap_handle = pcap_open_offline(dev, errbuf);
		if (!pcap_handle) {
			my_log(LOG_CRIT, "pcap_open_offline(): %s",errbuf);
			exit(1);
		}
	} else {
		pcap_handle = pcap_open_live(dev, snaplen, promisc, 1000, errbuf);
		if (!pcap_handle) {
			my_log(LOG_CRIT, "pcap_open_live(): %s",errbuf);
			exit(1);
		}
		if (sockbufsize)
			if (setsockopt(pcap_fileno(pcap_handle), SOL_SOCKET,
				SO_RCVBUF, &sockbufsize, sizeof(sockbufsize)) < 0)
				my_log(LOG_WARNING, "setsockopt(): %s", strerror(errno));
	}

	link_type = pcap_datalink(pcap_handle);
	for (i = 0;; i++)
		if (dlt[i].linktype == link_type || dlt[i].linktype == -1) {
			off_nl = dlt[i].offset_nl;
			link_type_idx = i;
			break;
		}
	if (link_layer_size >= 0) off_nl = link_layer_size;
	if (off_nl == -1) {
		my_log(LOG_CRIT, "Uknown data link type %d. Use -K option.", link_type);
		exit(1);
	}
	if (parms[fflag].arg) {
		filter = parms[fflag].arg;
		if (pcap_compile(pcap_handle, &bpf_filter, filter, 1, 0) == -1) {
			my_log(LOG_CRIT, "pcap_compile(): %s. Filter: %s",
				pcap_geterr(pcap_handle), filter);
			exit(1);
		}
		if (pcap_setfilter(pcap_handle, &bpf_filter) == -1) {
			my_log(LOG_CRIT, "pcap_setfilter(): %s", pcap_geterr(pcap_handle));
			exit(1);
		}
	} else my_log(LOG_WARNING, "Filter expression is empty! Are you sure?");

	hash_init(); /* Actually for crc16 only */
	mem_init(sizeof(struct Flow), bulk_quantity, memory_limit);
	for (i = 0; i < 1 << HASH_BITS; i++) pthread_mutex_init(&flows_mutex[i], 0);

#ifdef UPTIME_TRICK
	/* Hope 12 days is enough :-/ */
	start_time_offset = 1 << 20;

	/* start_time_offset = active_lifetime + inactive_lifetime + scan_interval; */
#endif
	gettime(&start_time);

	/*
	Build static pending queue as circular buffer.
	We can't use dynamic flow allocation in pcap_callback() because
	memory routines shared between threads, including non-realtime.
	Collision (mem_mutex lock in mem_alloc()) of pcap_callback()
	with such (non-realtime) thread may cause intolerable slowdown
	and packets loss as effect.
	*/
	if (!(pending_head = mem_alloc())) goto err_mem_alloc;
	pending_tail = pending_head;
	for (i = pending_queue_length - 1; i--;) {
		if (!(pending_tail->next = mem_alloc())) {
		err_mem_alloc:
			my_log(LOG_CRIT, "mem_alloc(): %s", strerror(errno));
			exit(1);
		}
		pending_tail = pending_tail->next;
	}
	pending_tail->next = pending_head;
	pending_tail = pending_head;

	sigemptyset(&sig_mask);
	sigact.sa_handler = &sighandler;
	sigact.sa_mask = sig_mask;
	sigact.sa_flags = 0;
	sigaddset(&sig_mask, SIGTERM);
	sigaction(SIGTERM, &sigact, 0);
#if ((DEBUG) & DEBUG_I)
	sigaddset(&sig_mask, SIGUSR1);
	sigaction(SIGUSR1, &sigact, 0);
#endif
	if (pthread_sigmask(SIG_BLOCK, &sig_mask, 0)) {
		my_log(LOG_CRIT, "pthread_sigmask(): %s", strerror(errno));
		exit(1);
	}

	my_log(LOG_INFO, "Starting %s...", VERSION);

	if (parms[cflag].count) {
		if (chdir(parms[cflag].arg) || chroot(".")) {
			my_log(LOG_CRIT, "could not chroot to %s: %s", parms[cflag].arg, strerror(errno));
			exit(1);
		}
	}

#ifdef OS_SOLARIS
	pthread_setconcurrency(THREADS);
#endif

	schedp.sched_priority = schedp.sched_priority - THREADS + 2;
	pthread_attr_init(&tattr);
	for (i = 0; i < THREADS - 1; i++) {
		if (schedp.sched_priority > 0) {
			if ((pthread_attr_setschedpolicy(&tattr, SCHED)) ||
				(pthread_attr_setschedparam(&tattr, &schedp))) {
				my_log(LOG_CRIT, "pthread_attr_setschedpolicy(): %s", strerror(errno));
				exit(1);
			}
		}
		if (pthread_create(&thid, &tattr, threads[i], 0)) {
			my_log(LOG_CRIT, "pthread_create(): %s", strerror(errno));
			exit(1);
		}
		pthread_detach(thid);
		schedp.sched_priority++;
	}

	if (pw) {
		if (setgroups(0, NULL) < 0) {
			my_log(LOG_CRIT, "setgroups: %s", strerror(errno));
			exit(1);
		}
		if (setregid(pw->pw_gid, pw->pw_gid)) {
			my_log(LOG_CRIT, "setregid(%u): %s", pw->pw_gid, strerror(errno));
			exit(1);
		}
		if (setreuid(pw->pw_uid, pw->pw_uid)) {
			my_log(LOG_CRIT, "setreuid(%u): %s", pw->pw_uid, strerror(errno));
			exit(1);
		}
	}

	if (!(pidfile = fopen(pidfilepath, "w")))
		my_log(LOG_ERR, "Can't create pid file. fopen(): %s", strerror(errno));
	else {
		fprintf(pidfile, "%ld\n", (long) pid);
		fclose(pidfile);
	}

	my_log(LOG_INFO, "pid: %d", pid);
	my_log(LOG_INFO, "interface: %s, datalink: %s (%d)",
		dev, dlt[link_type_idx].descr, link_type);
	my_log(LOG_INFO, "filter: \"%s\"", filter);
	my_log(LOG_INFO, "options: p=%d s=%u g=%u d=%u e=%u n=%u a=%s x=%u:%u "
		"b=%u m=%u q=%u B=%u r=%u t=%u:%u S=%d K=%d k=%d c=%s u=%s v=%u l=%u%s",
		promisc, scan_interval, frag_lifetime, inactive_lifetime, active_lifetime,
		netflow->Version, inet_ntoa(saddr.sin_addr), snmp_input_index, snmp_output_index,
		bulk_quantity, memory_limit >> 10, pending_queue_length, sockbufsize >> 10,
		schedp.sched_priority - 1, emit_rate_bytes, emit_rate_delay, snaplen, off_nl, link_layer,
		parms[cflag].count ? parms[cflag].arg : "", parms[uflag].count ? parms[uflag].arg : "",
		verbosity, log_dest, log_suffix ? log_suffix : "");
	for (i = 0; i < npeers; i++) {
		switch (peers[i].type) {
			case PEER_MIRROR:
				c = 'm';
				break;
			case PEER_ROTATE:
				c = 'r';
				break;
		}
		snprintf(errbuf, sizeof(errbuf), "%s", inet_ntoa(peers[i].laddr.sin_addr));
		my_log(LOG_INFO,"collector #%d: %s:%u/%s/%c", i + 1,
			inet_ntoa(peers[i].addr.sin_addr), ntohs(peers[i].addr.sin_port), errbuf, c);
	}

	snmp_input_index = htons(snmp_input_index);
	snmp_output_index = htons(snmp_output_index);

	pthread_sigmask(SIG_UNBLOCK, &sig_mask, 0);

	timeout.tv_usec = 0;
	while (!killed
		|| (total_elements - free_elements - pending_queue_length)
		|| emit_count
		|| pending_tail->flags) {

		if (!sigs) {
			timeout.tv_sec = scan_interval;
			select(0, 0, 0, 0, &timeout);
		}

		if (sigs & SIGTERM_MASK && !killed) {
			sigs &= ~SIGTERM_MASK;
			my_log(LOG_INFO, "SIGTERM received. Emitting flows cache...");
			scan_interval = 1;
			frag_lifetime = -1;
			active_lifetime = -1;
			inactive_lifetime = -1;
			emit_timeout = 1;
			unpending_timeout = 1;
			killed = 1;
			pthread_cond_signal(&scan_cond);
			pthread_cond_signal(&unpending_cond);
		}

#if ((DEBUG) & DEBUG_I)
		if (sigs & SIGUSR1_MASK) {
			sigs &= ~SIGUSR1_MASK;
			info_debug();
		}
#endif
	}
	remove(pidfilepath);
#if ((DEBUG) & DEBUG_I)
	info_debug();
#endif
	my_log(LOG_INFO, "Done.");
#ifdef WALL
	return 0;
#endif
}
