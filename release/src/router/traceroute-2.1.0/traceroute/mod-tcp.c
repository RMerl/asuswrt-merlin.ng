/*
    Copyright (c)  2006, 2007		Dmitry Butskoy
					<buc@citadel.stu.neva.ru>
    License:  GPL v2 or any later

    See COPYING for the status of this software.
*/

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <poll.h>
#include <netinet/icmp6.h>
#include <netinet/ip_icmp.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>


#include "traceroute.h"


#ifndef IP_MTU
#define IP_MTU	14
#endif


static sockaddr_any dest_addr = {{ 0, }, };
static unsigned int dest_port = 0;

static int raw_sk = -1;
static int last_ttl = 0;

static uint8_t buf[1024];	    /*  enough, enough...  */
static size_t csum_len = 0;
static struct tcphdr *th = NULL;

#define TH_FLAGS(TH)	(((uint8_t *) (TH))[13])
#define TH_FIN	0x01
#define TH_SYN	0x02
#define TH_RST	0x04
#define TH_PSH	0x08
#define TH_ACK	0x10
#define TH_URG	0x20
#define TH_ECE	0x40
#define TH_CWR	0x80


static int flags = 0;	    /*  & 0xff == tcp_flags ...  */
static int sysctl = 0;
static int reuse = 0;
static unsigned int mss = 0;
static int info = 0;

#define FL_FLAGS	0x0100
#define FL_ECN		0x0200
#define FL_SACK		0x0400
#define FL_TSTAMP	0x0800
#define FL_WSCALE	0x1000


static struct {
	const char *name;
	unsigned int flag;
} tcp_flags[] = {
	{ "fin", TH_FIN },
	{ "syn", TH_SYN },
	{ "rst", TH_RST },
	{ "psh", TH_PSH },
	{ "ack", TH_ACK },
	{ "urg", TH_URG },
	{ "ece", TH_ECE },
	{ "cwr", TH_CWR },
};

static char *names_by_flags (unsigned int flags) {
	int i;
	char str[64];	/*  enough...  */
	char *curr = str;
	char *end = str + sizeof (str) / sizeof (*str);

	for (i = 0; i < sizeof (tcp_flags) / sizeof (*tcp_flags); i++) {
	    const char *p;

	    if (!(flags & tcp_flags[i].flag))  continue;

	    if (curr > str && curr < end)  *curr++ = ',';
	    for (p = tcp_flags[i].name; *p && curr < end; *curr++ = *p++) ;
	}

	*curr = '\0';

	return  strdup (str);
}

static int set_tcp_flag (CLIF_option *optn, char *arg) {
	int i;

	for (i = 0; i < sizeof (tcp_flags) / sizeof (*tcp_flags); i++) {
	    if (!strcmp (optn->long_opt, tcp_flags[i].name)) {
		    flags |= tcp_flags[i].flag;
		    return 0;
	    }
	}

	return -1;
}

static int set_tcp_flags (CLIF_option *optn, char *arg) {
	char *q;
	unsigned long value;

	value = strtoul (arg, &q, 0);
	if (q == arg)  return -1;

	flags = (flags & ~0xff) | (value & 0xff) | FL_FLAGS;
	return 0;
}

static int set_flag (CLIF_option *optn, char *arg) {

	flags |= (unsigned long) optn->data;

	return 0;
}

static CLIF_option tcp_options[] = {
	{ 0, "syn", 0, "Set tcp flag SYN (default if no other "
			"tcp flags specified)", set_tcp_flag, 0, 0, 0 },
	{ 0, "ack", 0, "Set tcp flag ACK,", set_tcp_flag, 0, 0, 0 },
	{ 0, "fin", 0, "FIN,", set_tcp_flag, 0, 0, 0 },
	{ 0, "rst", 0, "RST,", set_tcp_flag, 0, 0, 0 },
	{ 0, "psh", 0, "PSH,", set_tcp_flag, 0, 0, 0 },
	{ 0, "urg", 0, "URG,", set_tcp_flag, 0, 0, 0 },
	{ 0, "ece", 0, "ECE,", set_tcp_flag, 0, 0, 0 },
	{ 0, "cwr", 0, "CWR", set_tcp_flag, 0, 0, 0 },
	{ 0, "flags", "NUM", "Set tcp flags exactly to value %s",
				set_tcp_flags, 0, 0, CLIF_ABBREV },
	{ 0, "ecn", 0, "Send syn packet with tcp flags ECE and CWR "
			"(for Explicit Congestion Notification, rfc3168)",
				set_flag, (void *) FL_ECN, 0, 0 },
	{ 0, "sack", 0, "Use sack,",
				set_flag, (void *) FL_SACK, 0, 0 },
	{ 0, "timestamps", 0, "timestamps,",
				set_flag, (void *) FL_TSTAMP, 0, CLIF_ABBREV },
	{ 0, "window_scaling", 0, "window_scaling option for tcp",
				set_flag, (void *) FL_WSCALE, 0, CLIF_ABBREV },
	{ 0, "sysctl", 0, "Use current sysctl (/proc/sys/net/*) setting "
			"for the tcp options and ecn. Always set by default "
			"(with \"syn\") if nothing else specified",
				CLIF_set_flag, &sysctl, 0, 0 },
	{ 0, "reuse", 0, "Allow to reuse local port numbers "
			"for the huge workloads (SO_REUSEADDR)",
				CLIF_set_flag, &reuse, 0, 0 },
	{ 0, "mss", "NUM", "Use value of %s for maxseg tcp option (when syn)",
				CLIF_set_uint, &mss, 0, 0 },
	{ 0, "info", 0, "Print tcp flags of final tcp replies when target "
			"host is reached. Useful to determine whether "
			"an application listens the port etc.",
				CLIF_set_flag, &info, 0, 0 },
	CLIF_END_OPTION
};


#define SYSCTL_PREFIX	"/proc/sys/net/ipv4/tcp_"
static int check_sysctl (const char *name) {
	int fd, res;
	char buf[sizeof (SYSCTL_PREFIX) + strlen (name) + 1];
	uint8_t ch;

	strcpy (buf, SYSCTL_PREFIX);
	strcat (buf, name);

	fd = open (buf, O_RDONLY, 0);
	if (fd < 0)  return 0;

	res = read (fd, &ch, sizeof (ch));
	close (fd);

	if (res != sizeof (ch))
		return 0;

	/*  since kernel 2.6.31 "tcp_ecn" can have value of '2'...  */
	if (ch == '1')  return 1;

	return 0;
}


static int tcp_init (const sockaddr_any *dest,
			    unsigned int port_seq, size_t *packet_len_p) {
	int af = dest->sa.sa_family;
	sockaddr_any src;
	int mtu;
	socklen_t len;
	uint8_t *ptr;
	uint16_t *lenp;


	dest_addr = *dest;
	dest_addr.sin.sin_port = 0;	/*  raw sockets can be confused   */

	if (!port_seq)  port_seq = DEF_TCP_PORT;
	dest_port = htons (port_seq);


	/*  Create raw socket for tcp   */

	raw_sk = socket (af, SOCK_RAW, IPPROTO_TCP);
	if (raw_sk < 0)
		error_or_perm ("socket");

	tune_socket (raw_sk);	    /*  including bind, if any   */

	if (connect (raw_sk, &dest_addr.sa, sizeof (dest_addr)) < 0)
		error ("connect");

	len = sizeof (src);
	if (getsockname (raw_sk, &src.sa, &len) < 0)
		error ("getsockname");


	len = sizeof (mtu);
	if (getsockopt (raw_sk, af == AF_INET ? SOL_IP : SOL_IPV6,
				af == AF_INET ? IP_MTU : IPV6_MTU,
				&mtu, &len) < 0 || mtu < 576
	)  mtu = 576;

	/*  mss = mtu - headers   */
	mtu -= af == AF_INET ? sizeof (struct iphdr) : sizeof (struct ip6_hdr);
	mtu -= sizeof (struct tcphdr);


	if (!raw_can_connect ()) {	/*  work-around for buggy kernels  */
	    close (raw_sk);
	    raw_sk = socket (af, SOCK_RAW, IPPROTO_TCP);
	    if (raw_sk < 0)  error ("socket");
	    tune_socket (raw_sk);
	    /*  but do not connect it...  */
	}


	use_recverr (raw_sk);

	add_poll (raw_sk, POLLIN | POLLERR);


	/*  Now create the sample packet.  */

	if (!flags)  sysctl = 1;

	if (sysctl) {
	    if (check_sysctl ("ecn"))  flags |= FL_ECN;
	    if (check_sysctl ("sack"))  flags |= FL_SACK;
	    if (check_sysctl ("timestamps"))  flags |= FL_TSTAMP;
	    if (check_sysctl ("window_scaling"))  flags |= FL_WSCALE;
	}

	if (!(flags & (FL_FLAGS | 0xff))) {	/*  no any tcp flag set   */
	    flags |= TH_SYN;
	    if (flags & FL_ECN)
		    flags |= TH_ECE | TH_CWR;
	}


	/*  For easy checksum computing:
	    saddr
	    daddr
	    length
	    protocol
	    tcphdr
	    tcpoptions
	*/

	ptr = buf;

	if (af == AF_INET) {
	    len = sizeof (src.sin.sin_addr);
	    memcpy (ptr, &src.sin.sin_addr, len);
	    ptr += len;
	    memcpy (ptr, &dest_addr.sin.sin_addr, len);
	    ptr += len;
	} else {
	    len = sizeof (src.sin6.sin6_addr);
	    memcpy (ptr, &src.sin6.sin6_addr, len);
	    ptr += len;
	    memcpy (ptr, &dest_addr.sin6.sin6_addr, len);
	    ptr += len;
	}

	lenp = (uint16_t *) ptr;
	ptr += sizeof (uint16_t);
	*((uint16_t *) ptr) = htons ((uint16_t) IPPROTO_TCP);
	ptr += sizeof (uint16_t);


	/*  Construct TCP header   */

	th = (struct tcphdr *) ptr;

	th->source = 0;	    /*  temporary   */
	th->dest = dest_port;
	th->seq = 0;	    /*  temporary   */
	th->ack_seq = 0;
	th->doff = 0;	    /*  later...  */
	TH_FLAGS(th) = flags & 0xff;
	th->window = htons (4 * mtu);
	th->check = 0;
	th->urg_ptr = 0;


	/*  Build TCP options   */

	ptr = (uint8_t *) (th + 1);

	if (flags & TH_SYN) {
	    *ptr++ = TCPOPT_MAXSEG;	/*  2   */
	    *ptr++ = TCPOLEN_MAXSEG;	/*  4   */
	    *((uint16_t *) ptr) = htons (mss ? mss : mtu);
	    ptr += sizeof (uint16_t);
	}

	if (flags & FL_TSTAMP) {

	    if (flags & FL_SACK) {
		*ptr++ = TCPOPT_SACK_PERMITTED;	/*  4   */
		*ptr++ = TCPOLEN_SACK_PERMITTED;/*  2   */
	    } else {
		*ptr++ = TCPOPT_NOP;	/*  1   */
		*ptr++ = TCPOPT_NOP;	/*  1   */
	    }
	    *ptr++ = TCPOPT_TIMESTAMP;	/*  8   */
	    *ptr++ = TCPOLEN_TIMESTAMP;	/*  10  */

	    *((uint32_t *) ptr) = random_seq ();	/*  really!  */
	    ptr += sizeof (uint32_t);
	    *((uint32_t *) ptr) = (flags & TH_ACK) ? random_seq () : 0;
	    ptr += sizeof (uint32_t);
	}
	else if (flags & FL_SACK) {
	    *ptr++ = TCPOPT_NOP;	/*  1   */
	    *ptr++ = TCPOPT_NOP;	/*  1   */
	    *ptr++ = TCPOPT_SACK_PERMITTED;	/*  4   */
	    *ptr++ = TCPOLEN_SACK_PERMITTED;	/*  2   */
	}

	if (flags & FL_WSCALE) {
	    *ptr++ = TCPOPT_NOP;	/*  1   */
	    *ptr++ = TCPOPT_WINDOW;	/*  3   */
	    *ptr++ = TCPOLEN_WINDOW;	/*  3   */
	    *ptr++ = 2;	/*  assume some corect value...  */
	}


	csum_len = ptr - buf;

	if (csum_len > sizeof (buf))
		error ("impossible");	/*  paranoia   */

	len = ptr - (uint8_t *) th;
	if (len & 0x03)  error ("impossible");	/*  as >>2 ...  */

	*lenp = htons (len);
	th->doff = len >> 2;


	*packet_len_p = len;

	return 0;
}


static void tcp_send_probe (probe *pb, int ttl) {
	int sk;
	int af = dest_addr.sa.sa_family;
	sockaddr_any addr;
	socklen_t len = sizeof (addr);


	/*  To make sure we have chosen a free unused "source port",
	   just create, (auto)bind and hold a socket while the port is needed.
	*/

	sk = socket (af, SOCK_STREAM, 0);
	if (sk < 0)  error ("socket");

	if (reuse && setsockopt (sk, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
		error ("setsockopt SO_REUSEADDR");

	bind_socket (sk);

	if (getsockname (sk, &addr.sa, &len) < 0)
		error ("getsockname");

	/*  When we reach the target host, it can send us either RST or SYN+ACK.
	  For RST all is OK (we and kernel just answer nothing), but
	  for SYN+ACK we should reply with our RST.
	    It is well-known "half-open technique", used by port scanners etc.
	  This way we do not touch remote applications at all, unlike
	  the ordinary connect(2) call.
	    As the port-holding socket neither connect() nor listen(),
	  it means "no such port yet" for remote ends, and kernel always
	  send RST in such a situation automatically (we have to do nothing).
	*/


	th->source = addr.sin.sin_port;

	th->seq = random_seq ();

	th->check = 0;
	th->check = in_csum (buf, csum_len);


	if (ttl != last_ttl) {

	    set_ttl (raw_sk, ttl);

	    last_ttl = ttl;
	}


	pb->send_time = get_time ();

	if (do_send (raw_sk, th, th->doff << 2, &dest_addr) < 0) {
	    close (sk);
	    pb->send_time = 0;
	    return;
	}


	pb->seq = th->source;

	pb->sk = sk;

	return;
}


static probe *tcp_check_reply (int sk, int err, sockaddr_any *from,
						    char *buf, size_t len) {
	probe *pb;
	struct tcphdr *tcp = (struct tcphdr *) buf;
	uint16_t sport, dport;


	if (len < 8)  return NULL;	    /*  too short   */


	if (err) {
	    sport = tcp->source;
	    dport = tcp->dest;
	} else {
	    sport = tcp->dest;
	    dport = tcp->source;
	}


	if (dport != dest_port)
		return NULL;

	if (!equal_addr (&dest_addr, from))
		return NULL;

	pb = probe_by_seq (sport);
	if (!pb)  return NULL;


	if (!err) {

	    pb->final = 1;

	    if (info)
		pb->ext = names_by_flags (TH_FLAGS(tcp));
	}

	return pb;
}


static void tcp_recv_probe (int sk, int revents) {

	if (!(revents & (POLLIN | POLLERR)))
		return;

	recv_reply (sk, !!(revents & POLLERR), tcp_check_reply);
}


static void tcp_expire_probe (probe *pb) {

	probe_done (pb);
}


static tr_module tcp_ops = {
	.name = "tcp",
	.init = tcp_init,
	.send_probe = tcp_send_probe,
	.recv_probe = tcp_recv_probe,
	.expire_probe = tcp_expire_probe,
	.options = tcp_options,
};

TR_MODULE (tcp_ops);
