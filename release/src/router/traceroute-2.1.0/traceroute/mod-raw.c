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
#include <netdb.h>

#include "traceroute.h"


static sockaddr_any dest_addr = {{ 0, }, };
static int protocol = DEF_RAW_PROT;

static char *data = NULL;
static size_t *length_p;

static int raw_sk = -1;
static int last_ttl = 0;
static int seq = 0;


static int set_protocol (CLIF_option *optn, char *arg) {
	char *q;

	protocol = strtoul (arg, &q, 0);
	if (q == arg) {
	    struct protoent *p = getprotobyname (arg);

	    if (!p)  return -1;
	    protocol = p->p_proto;
	}

	return 0;
}


static CLIF_option raw_options[] = {
	{ 0, "protocol", "PROT", "Use protocol %s (default is "
			_TEXT (DEF_RAW_PROT) ")",
			set_protocol, 0, 0, CLIF_ABBREV },
	CLIF_END_OPTION
};


static int raw_init (const sockaddr_any *dest,
			    unsigned int port_seq, size_t *packet_len_p) {
	int i;
	int af = dest->sa.sa_family;

	dest_addr = *dest;
	dest_addr.sin.sin_port = 0;

	if (port_seq)  protocol = port_seq;


	length_p = packet_len_p;

	if (*length_p &&
	    !(data = malloc (*length_p))
	)  error ("malloc");

        for (i = 0; i < *length_p; i++)
                data[i] = 0x40 + (i & 0x3f);


	raw_sk = socket (af, SOCK_RAW, protocol);
	if (raw_sk < 0)
		error_or_perm ("socket");

	tune_socket (raw_sk);

	/*  Don't want to catch packets from another hosts   */
	if (raw_can_connect () &&
	    connect (raw_sk, &dest_addr.sa, sizeof (dest_addr)) < 0
	)  error ("connect");

	use_recverr (raw_sk);


	add_poll (raw_sk, POLLIN | POLLERR);

	return 0;
}


static void raw_send_probe (probe *pb, int ttl) {

	if (ttl != last_ttl) {

	    set_ttl (raw_sk, ttl);

	    last_ttl = ttl;
	}


	pb->send_time = get_time ();

	if (do_send (raw_sk, data, *length_p, &dest_addr) < 0) {
	    pb->send_time = 0;
	    return;
	}


	pb->seq = ++seq;

	return;
}


static probe *raw_check_reply (int sk, int err, sockaddr_any *from,
						    char *buf, size_t len) {
	probe *pb;

	if (!equal_addr (&dest_addr, from))
		return NULL;

	pb = probe_by_seq (seq);
	if (!pb)  return NULL;

	if (!err)  pb->final = 1;

	return pb;
}


static void raw_recv_probe (int sk, int revents) {

	if (!(revents & (POLLIN | POLLERR)))
		return;

	recv_reply (sk, !!(revents & POLLERR), raw_check_reply);
}


static void raw_expire_probe (probe *pb) {

	probe_done (pb);
}


static tr_module raw_ops = {
	.name = "raw",
	.init = raw_init,
	.send_probe = raw_send_probe,
	.recv_probe = raw_recv_probe,
	.expire_probe = raw_expire_probe,
	.options = raw_options,
	.one_per_time = 1,
};

TR_MODULE (raw_ops);
