/*
    Copyright (c)  2006, 2007		Dmitry Butskoy
					<buc@citadel.stu.neva.ru>
    License:  GPL v2 or any later

    See COPYING for the status of this software.
*/

#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <poll.h>
#include <netinet/icmp6.h>
#include <netinet/ip_icmp.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <errno.h>

#include "traceroute.h"


static sockaddr_any dest_addr = {{ 0, }, };

static int icmp_sk = -1;


static int tcp_init (const sockaddr_any *dest,
			    unsigned int port_seq, size_t *packet_len_p) {
	int af = dest->sa.sa_family;

	dest_addr = *dest;
	dest_addr.sin.sin_port = htons (DEF_TCP_PORT);

	if (port_seq)
	    dest_addr.sin.sin_port = htons (port_seq);


	/*  Currently an ICMP socket is the only way
	  to obtain the needed info...
	*/
	icmp_sk = socket (af, SOCK_RAW, (af == AF_INET) ? IPPROTO_ICMP
							: IPPROTO_ICMPV6);
	if (icmp_sk < 0)
		error_or_perm ("socket");

	/*  icmp_sk not need full tune_socket() here, just a receiving one  */
	bind_socket (icmp_sk);
	use_timestamp (icmp_sk);
	use_recv_ttl (icmp_sk);

	add_poll (icmp_sk, POLLIN);

	return 0;
}


static void tcp_send_probe (probe *pb, int ttl) {
	int sk;
	int af = dest_addr.sa.sa_family;
	sockaddr_any addr;
	socklen_t length = sizeof (addr);


	sk = socket (af, SOCK_STREAM, 0);
	if (sk < 0)  error ("socket");

	tune_socket (sk);	/*  common stuff   */

	set_ttl (sk, ttl);


	pb->send_time = get_time ();

	if (connect (sk, &dest_addr.sa, sizeof (dest_addr)) < 0) {
	    if (errno != EINPROGRESS)
		    error ("connect");
	}


	if (getsockname (sk, &addr.sa, &length) < 0)
		error ("getsockname");

	pb->seq = addr.sin.sin_port;	/*  both ipv4/ipv6  */

	pb->sk = sk;

	add_poll (sk, POLLERR | POLLHUP | POLLOUT);

	return;
}


static probe *tcp_check_reply (int sk, int err, sockaddr_any *from,
						    char *buf, size_t len) {
	int af = dest_addr.sa.sa_family;
	int type, code, info;
	probe *pb;
	struct tcphdr *tcp;


	if (len < sizeof (struct icmphdr))
		return NULL;


	if (af == AF_INET) {
	    struct icmp *icmp = (struct icmp *) buf;
	    struct iphdr *ip;
	    int hlen;

	    type = icmp->icmp_type;
	    code = icmp->icmp_code;
	    info = icmp->icmp_void;

	    if (type != ICMP_TIME_EXCEEDED && type != ICMP_DEST_UNREACH)
		    return NULL;

	    if (len < sizeof (struct icmphdr) + sizeof (struct iphdr) + 8)
		    /* `8' - rfc1122: 3.2.2  */
		    return NULL;

	    ip = (struct iphdr *) (((char *)icmp) + sizeof(struct icmphdr));
	    hlen = ip->ihl << 2;

	    if (len < sizeof (struct icmphdr) + hlen + 8)
		    return NULL;
	    if (ip->protocol != IPPROTO_TCP)
		    return NULL;

	    tcp = (struct tcphdr *) (((char *) ip) + hlen);

	}
	else {	    /*  AF_INET6   */
	    struct icmp6_hdr *icmp6 = (struct icmp6_hdr *) buf;
	    struct ip6_hdr *ip6;

	    type = icmp6->icmp6_type;
	    code = icmp6->icmp6_code;
	    info = icmp6->icmp6_mtu;

	    if (type != ICMP6_TIME_EXCEEDED &&
		type != ICMP6_DST_UNREACH &&
		type != ICMP6_PACKET_TOO_BIG
	    )  return NULL;

	    if (len < sizeof(struct icmp6_hdr) + sizeof(struct ip6_hdr) + 8)
		    return NULL;

	    ip6 = (struct ip6_hdr *) (icmp6 + 1);
	    if (ip6->ip6_nxt != IPPROTO_TCP)
		    return NULL;

	    tcp = (struct tcphdr *) (ip6 + 1);

	}


	if (tcp->dest != dest_addr.sin.sin_port)
		return NULL;

	pb = probe_by_seq (tcp->source);
	if (!pb)  return NULL;


	/*  here only, high level has no data to do this   */
	parse_icmp_res (pb, type, code, info);

	return pb;
}


static void tcp_recv_probe (int sk, int revents) {

	if (sk != icmp_sk) {	/*  a tcp socket   */
	    probe *pb;

	    pb = probe_by_sk (sk);
	    if (!pb) {
		del_poll (sk);
		return;
	    }


	    /*  do connect() again and check errno, regardless of revents  */
	    if (connect (sk, &dest_addr.sa, sizeof (dest_addr)) < 0) {
		if (errno != EISCONN && errno != ECONNREFUSED)
			return;	/*  ICMP say more   */
	    }

	    /*  we have reached the dest host (either connected or refused)  */

	    memcpy (&pb->res, &dest_addr, sizeof (pb->res));

	    pb->final = 1;

	    pb->recv_time = get_time ();

	    probe_done (pb);

	    return;
	}


	/*  ICMP stuff   */

	if (!(revents & POLLIN))
		return;

	recv_reply (icmp_sk, 0, tcp_check_reply);
}


static void tcp_expire_probe (probe *pb) {

	probe_done (pb);
}


static tr_module tcp_ops = {
	.name = "tcpconn",
	.init = tcp_init,
	.send_probe = tcp_send_probe,
	.recv_probe = tcp_recv_probe,
	.expire_probe = tcp_expire_probe,
};

TR_MODULE (tcp_ops);
