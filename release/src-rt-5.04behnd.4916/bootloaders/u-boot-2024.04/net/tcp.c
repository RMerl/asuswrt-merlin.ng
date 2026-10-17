// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2017 Duncan Hare, all rights reserved.
 */

/*
 * General Desription:
 *
 * TCP support for the wget command, for fast file downloading.
 *
 * HTTP/TCP Receiver:
 *
 *      Prerequisites:  - own ethernet address
 *                      - own IP address
 *                      - Server IP address
 *                      - Server with TCP
 *                      - TCP application (eg wget)
 *      Next Step       HTTPS?
 */
#include <common.h>
#include <command.h>
#include <console.h>
#include <env_internal.h>
#include <errno.h>
#include <net.h>
#include <net/tcp.h>

/*
 * TCP sliding window  control used by us to request re-TX
 */
static struct tcp_sack_v tcp_lost;

/* TCP option timestamp */
static u32 loc_timestamp;
static u32 rmt_timestamp;

static u32 tcp_seq_init;
static u32 tcp_ack_edge;

static int tcp_activity_count;

/*
 * Search for TCP_SACK and review the comments before the code section
 * TCP_SACK is the number of packets at the front of the stream
 */

enum pkt_state {PKT, NOPKT};
struct sack_r {
	struct sack_edges se;
	enum pkt_state st;
};

static struct sack_r edge_a[TCP_SACK];
static unsigned int sack_idx;
static unsigned int prev_len;

/*
 * TCP lengths are stored as a rounded up number of 32 bit words.
 * Add 3 to length round up, rounded, then divided into the
 * length in 32 bit words.
 */
#define LEN_B_TO_DW(x) ((x) >> 2)
#define ROUND_TCPHDR_LEN(x) (LEN_B_TO_DW((x) + 3))
#define SHIFT_TO_TCPHDRLEN_FIELD(x) ((x) << 4)
#define GET_TCP_HDR_LEN_IN_BYTES(x) ((x) >> 2)

/* TCP connection state */
static enum tcp_state current_tcp_state;

/* Current TCP RX packet handler */
static rxhand_tcp *tcp_packet_handler;

/**
 * tcp_get_tcp_state() - get current TCP state
 *
 * Return: Current TCP state
 */
enum tcp_state tcp_get_tcp_state(void)
{
	return current_tcp_state;
}

/**
 * tcp_set_tcp_state() - set current TCP state
 * @new_state: new TCP state
 */
void tcp_set_tcp_state(enum tcp_state new_state)
{
	current_tcp_state = new_state;
}

static void dummy_handler(uchar *pkt, u16 dport,
			  struct in_addr sip, u16 sport,
			  u32 tcp_seq_num, u32 tcp_ack_num,
			  u8 action, unsigned int len)
{
}

/**
 * tcp_set_tcp_handler() - set a handler to receive data
 * @f: handler
 */
void tcp_set_tcp_handler(rxhand_tcp *f)
{
	debug_cond(DEBUG_INT_STATE, "--- net_loop TCP handler set (%p)\n", f);
	if (!f)
		tcp_packet_handler = dummy_handler;
	else
		tcp_packet_handler = f;
}

/**
 * tcp_set_pseudo_header() - set TCP pseudo header
 * @pkt: the packet
 * @src: source IP address
 * @dest: destinaion IP address
 * @tcp_len: tcp length
 * @pkt_len: packet length
 *
 * Return: the checksum of the packet
 */
u16 tcp_set_pseudo_header(uchar *pkt, struct in_addr src, struct in_addr dest,
			  int tcp_len, int pkt_len)
{
	union tcp_build_pkt *b = (union tcp_build_pkt *)pkt;
	int checksum_len;

	/*
	 * Pseudo header
	 *
	 * Zero the byte after the last byte so that the header checksum
	 * will always work.
	 */
	pkt[pkt_len] = 0;

	net_copy_ip((void *)&b->ph.p_src, &src);
	net_copy_ip((void *)&b->ph.p_dst, &dest);
	b->ph.rsvd = 0;
	b->ph.p	= IPPROTO_TCP;
	b->ph.len = htons(tcp_len);
	checksum_len = tcp_len + PSEUDO_HDR_SIZE;

	debug_cond(DEBUG_DEV_PKT,
		   "TCP Pesudo  Header  (to=%pI4, from=%pI4, Len=%d)\n",
		   &b->ph.p_dst, &b->ph.p_src, checksum_len);

	return compute_ip_checksum(pkt + PSEUDO_PAD_SIZE, checksum_len);
}

/**
 * net_set_ack_options() - set TCP options in acknowledge packets
 * @b: the packet
 *
 * Return: TCP header length
 */
int net_set_ack_options(union tcp_build_pkt *b)
{
	b->sack.hdr.tcp_hlen = SHIFT_TO_TCPHDRLEN_FIELD(LEN_B_TO_DW(TCP_HDR_SIZE));

	b->sack.t_opt.kind = TCP_O_TS;
	b->sack.t_opt.len = TCP_OPT_LEN_A;
	b->sack.t_opt.t_snd = htons(loc_timestamp);
	b->sack.t_opt.t_rcv = rmt_timestamp;
	b->sack.sack_v.kind = TCP_1_NOP;
	b->sack.sack_v.len = 0;

	if (IS_ENABLED(CONFIG_PROT_TCP_SACK)) {
		if (tcp_lost.len > TCP_OPT_LEN_2) {
			debug_cond(DEBUG_DEV_PKT, "TCP ack opt lost.len %x\n",
				   tcp_lost.len);
			b->sack.sack_v.len = tcp_lost.len;
			b->sack.sack_v.kind = TCP_V_SACK;
			b->sack.sack_v.hill[0].l = htonl(tcp_lost.hill[0].l);
			b->sack.sack_v.hill[0].r = htonl(tcp_lost.hill[0].r);

			/*
			 * These SACK structures are initialized with NOPs to
			 * provide TCP header alignment padding. There are 4
			 * SACK structures used for both header padding and
			 * internally.
			 */
			b->sack.sack_v.hill[1].l = htonl(tcp_lost.hill[1].l);
			b->sack.sack_v.hill[1].r = htonl(tcp_lost.hill[1].r);
			b->sack.sack_v.hill[2].l = htonl(tcp_lost.hill[2].l);
			b->sack.sack_v.hill[2].r = htonl(tcp_lost.hill[2].r);
			b->sack.sack_v.hill[3].l = TCP_O_NOP;
			b->sack.sack_v.hill[3].r = TCP_O_NOP;
		}

		b->sack.hdr.tcp_hlen = SHIFT_TO_TCPHDRLEN_FIELD(ROUND_TCPHDR_LEN(TCP_HDR_SIZE +
										 TCP_TSOPT_SIZE +
										 tcp_lost.len));
	} else {
		b->sack.sack_v.kind = 0;
		b->sack.hdr.tcp_hlen = SHIFT_TO_TCPHDRLEN_FIELD(ROUND_TCPHDR_LEN(TCP_HDR_SIZE +
										 TCP_TSOPT_SIZE));
	}

	/*
	 * This returns the actual rounded up length of the
	 * TCP header to add to the total packet length
	 */

	return GET_TCP_HDR_LEN_IN_BYTES(b->sack.hdr.tcp_hlen);
}

/**
 * net_set_ack_options() - set TCP options in SYN packets
 * @b: the packet
 */
void net_set_syn_options(union tcp_build_pkt *b)
{
	if (IS_ENABLED(CONFIG_PROT_TCP_SACK))
		tcp_lost.len = 0;

	b->ip.hdr.tcp_hlen = 0xa0;

	b->ip.mss.kind = TCP_O_MSS;
	b->ip.mss.len = TCP_OPT_LEN_4;
	b->ip.mss.mss = htons(TCP_MSS);
	b->ip.scale.kind = TCP_O_SCL;
	b->ip.scale.scale = TCP_SCALE;
	b->ip.scale.len = TCP_OPT_LEN_3;
	if (IS_ENABLED(CONFIG_PROT_TCP_SACK)) {
		b->ip.sack_p.kind = TCP_P_SACK;
		b->ip.sack_p.len = TCP_OPT_LEN_2;
	} else {
		b->ip.sack_p.kind = TCP_1_NOP;
		b->ip.sack_p.len = TCP_1_NOP;
	}
	b->ip.t_opt.kind = TCP_O_TS;
	b->ip.t_opt.len = TCP_OPT_LEN_A;
	loc_timestamp = get_ticks();
	rmt_timestamp = 0;
	b->ip.t_opt.t_snd = 0;
	b->ip.t_opt.t_rcv = 0;
	b->ip.end = TCP_O_END;
}

int tcp_set_tcp_header(uchar *pkt, int dport, int sport, int payload_len,
		       u8 action, u32 tcp_seq_num, u32 tcp_ack_num)
{
	union tcp_build_pkt *b = (union tcp_build_pkt *)pkt;
	int pkt_hdr_len;
	int pkt_len;
	int tcp_len;

	/*
	 * Header: 5 32 bit words. 4 bits TCP header Length,
	 *         4 bits reserved options
	 */
	b->ip.hdr.tcp_flags = action;
	pkt_hdr_len = IP_TCP_HDR_SIZE;
	b->ip.hdr.tcp_hlen = SHIFT_TO_TCPHDRLEN_FIELD(LEN_B_TO_DW(TCP_HDR_SIZE));

	switch (action) {
	case TCP_SYN:
		debug_cond(DEBUG_DEV_PKT,
			   "TCP Hdr:SYN (%pI4, %pI4, sq=%u, ak=%u)\n",
			   &net_server_ip, &net_ip,
			   tcp_seq_num, tcp_ack_num);
		tcp_activity_count = 0;
		net_set_syn_options(b);
		tcp_seq_num = 0;
		tcp_ack_num = 0;
		pkt_hdr_len = IP_TCP_O_SIZE;
		if (current_tcp_state == TCP_SYN_SENT) {  /* Too many SYNs */
			action = TCP_FIN;
			current_tcp_state = TCP_FIN_WAIT_1;
		} else {
			current_tcp_state = TCP_SYN_SENT;
		}
		break;
	case TCP_SYN | TCP_ACK:
	case TCP_ACK:
		pkt_hdr_len = IP_HDR_SIZE + net_set_ack_options(b);
		b->ip.hdr.tcp_flags = action;
		debug_cond(DEBUG_DEV_PKT,
			   "TCP Hdr:ACK (%pI4, %pI4, s=%u, a=%u, A=%x)\n",
			   &net_server_ip, &net_ip, tcp_seq_num, tcp_ack_num,
			   action);
		break;
	case TCP_FIN:
		debug_cond(DEBUG_DEV_PKT,
			   "TCP Hdr:FIN  (%pI4, %pI4, s=%u, a=%u)\n",
			   &net_server_ip, &net_ip, tcp_seq_num, tcp_ack_num);
		payload_len = 0;
		pkt_hdr_len = IP_TCP_HDR_SIZE;
		current_tcp_state = TCP_FIN_WAIT_1;
		break;
	case TCP_RST | TCP_ACK:
	case TCP_RST:
		debug_cond(DEBUG_DEV_PKT,
			   "TCP Hdr:RST  (%pI4, %pI4, s=%u, a=%u)\n",
			   &net_server_ip, &net_ip, tcp_seq_num, tcp_ack_num);
		current_tcp_state = TCP_CLOSED;
		break;
	/* Notify connection closing */
	case (TCP_FIN | TCP_ACK):
	case (TCP_FIN | TCP_ACK | TCP_PUSH):
		if (current_tcp_state == TCP_CLOSE_WAIT)
			current_tcp_state = TCP_CLOSING;

		debug_cond(DEBUG_DEV_PKT,
			   "TCP Hdr:FIN ACK PSH(%pI4, %pI4, s=%u, a=%u, A=%x)\n",
			   &net_server_ip, &net_ip,
			   tcp_seq_num, tcp_ack_num, action);
		fallthrough;
	default:
		pkt_hdr_len = IP_HDR_SIZE + net_set_ack_options(b);
		b->ip.hdr.tcp_flags = action | TCP_PUSH | TCP_ACK;
		debug_cond(DEBUG_DEV_PKT,
			   "TCP Hdr:dft  (%pI4, %pI4, s=%u, a=%u, A=%x)\n",
			   &net_server_ip, &net_ip,
			   tcp_seq_num, tcp_ack_num, action);
	}

	pkt_len	= pkt_hdr_len + payload_len;
	tcp_len	= pkt_len - IP_HDR_SIZE;

	tcp_ack_edge = tcp_ack_num;
	/* TCP Header */
	b->ip.hdr.tcp_ack = htonl(tcp_ack_edge);
	b->ip.hdr.tcp_src = htons(sport);
	b->ip.hdr.tcp_dst = htons(dport);
	b->ip.hdr.tcp_seq = htonl(tcp_seq_num);

	/*
	 * TCP window size - TCP header variable tcp_win.
	 * Change tcp_win only if you have an understanding of network
	 * overrun, congestion, TCP segment sizes, TCP windows, TCP scale,
	 * queuing theory  and packet buffering. If there are too few buffers,
	 * there will be data loss, recovery may work or the sending TCP,
	 * the server, could abort the stream transmission.
	 * MSS is governed by maximum Ethernet frame length.
	 * The number of buffers is governed by the desire to have a queue of
	 * full buffers to be processed at the destination to maximize
	 * throughput. Temporary memory use for the boot phase on modern
	 * SOCs is may not be considered a constraint to buffer space, if
	 * it is, then the u-boot tftp or nfs kernel netboot should be
	 * considered.
	 */
	b->ip.hdr.tcp_win = htons(PKTBUFSRX * TCP_MSS >> TCP_SCALE);

	b->ip.hdr.tcp_xsum = 0;
	b->ip.hdr.tcp_ugr = 0;

	b->ip.hdr.tcp_xsum = tcp_set_pseudo_header(pkt, net_ip, net_server_ip,
						   tcp_len, pkt_len);

	net_set_ip_header((uchar *)&b->ip, net_server_ip, net_ip,
			  pkt_len, IPPROTO_TCP);

	return pkt_hdr_len;
}

/**
 * tcp_hole() - Selective Acknowledgment (Essential for fast stream transfer)
 * @tcp_seq_num: TCP sequence start number
 * @len: the length of sequence numbers
 */
void tcp_hole(u32 tcp_seq_num, u32 len)
{
	u32 idx_sack, sack_in;
	u32 sack_end = TCP_SACK - 1;
	u32 hill = 0;
	enum pkt_state expect = PKT;
	u32 seq = tcp_seq_num - tcp_seq_init;
	u32 hol_l = tcp_ack_edge - tcp_seq_init;
	u32 hol_r = 0;

	/* Place new seq number in correct place in receive array */
	if (prev_len == 0)
		prev_len = len;

	idx_sack = sack_idx + ((tcp_seq_num - tcp_ack_edge) / prev_len);
	if (idx_sack < TCP_SACK) {
		edge_a[idx_sack].se.l = tcp_seq_num;
		edge_a[idx_sack].se.r = tcp_seq_num + len;
		edge_a[idx_sack].st = PKT;

		/*
		 * The fin (last) packet is not the same length as data
		 * packets, and if it's length is recorded and used for
		 *  array index calculation, calculation breaks.
		 */
		if (prev_len < len)
			prev_len = len;
	}

	debug_cond(DEBUG_DEV_PKT,
		   "TCP 1 seq %d, edg %d, len %d, sack_idx %d, sack_end %d\n",
		    seq, hol_l, len, sack_idx, sack_end);

	/* Right edge of contiguous stream, is the left edge of first hill */
	hol_l = tcp_seq_num - tcp_seq_init;
	hol_r = hol_l + len;

	if (IS_ENABLED(CONFIG_PROT_TCP_SACK))
		tcp_lost.len = TCP_OPT_LEN_2;

	debug_cond(DEBUG_DEV_PKT,
		   "TCP 1 in %d, seq %d, pkt_l %d, pkt_r %d, sack_idx %d, sack_end %d\n",
		   idx_sack, seq, hol_l, hol_r, sack_idx, sack_end);

	for (sack_in = sack_idx; sack_in < sack_end && hill < TCP_SACK_HILLS;
	     sack_in++)  {
		switch (expect) {
		case NOPKT:
			switch (edge_a[sack_in].st) {
			case NOPKT:
				debug_cond(DEBUG_INT_STATE, "N");
				break;
			case PKT:
				debug_cond(DEBUG_INT_STATE, "n");
				if (IS_ENABLED(CONFIG_PROT_TCP_SACK)) {
					tcp_lost.hill[hill].l =
						edge_a[sack_in].se.l;
					tcp_lost.hill[hill].r =
						edge_a[sack_in].se.r;
				}
				expect = PKT;
				break;
			}
			break;
		case PKT:
			switch (edge_a[sack_in].st) {
			case NOPKT:
				debug_cond(DEBUG_INT_STATE, "p");
				if (sack_in > sack_idx &&
				    hill < TCP_SACK_HILLS) {
					hill++;
					if (IS_ENABLED(CONFIG_PROT_TCP_SACK))
						tcp_lost.len += TCP_OPT_LEN_8;
				}
				expect = NOPKT;
				break;
			case PKT:
				debug_cond(DEBUG_INT_STATE, "P");

				if (tcp_ack_edge == edge_a[sack_in].se.l) {
					tcp_ack_edge = edge_a[sack_in].se.r;
					edge_a[sack_in].st = NOPKT;
					sack_idx++;
				} else {
					if (IS_ENABLED(CONFIG_PROT_TCP_SACK) &&
					    hill < TCP_SACK_HILLS)
						tcp_lost.hill[hill].r =
							edge_a[sack_in].se.r;
				if (IS_ENABLED(CONFIG_PROT_TCP_SACK) &&
				    sack_in == sack_end - 1)
					tcp_lost.hill[hill].r =
						edge_a[sack_in].se.r;
				}
				break;
			}
			break;
		}
	}
	debug_cond(DEBUG_INT_STATE, "\n");
	if (!IS_ENABLED(CONFIG_PROT_TCP_SACK) || tcp_lost.len <= TCP_OPT_LEN_2)
		sack_idx = 0;
}

/**
 * tcp_parse_options() - parsing TCP options
 * @o: pointer to the option field.
 * @o_len: length of the option field.
 */
void tcp_parse_options(uchar *o, int o_len)
{
	struct tcp_t_opt  *tsopt;
	uchar *p = o;

	/*
	 * NOPs are options with a zero length, and thus are special.
	 * All other options have length fields.
	 */
	for (p = o; p < (o + o_len); p = p + p[1]) {
		if (!p[1])
			return; /* Finished processing options */

		switch (p[0]) {
		case TCP_O_END:
			return;
		case TCP_O_MSS:
		case TCP_O_SCL:
		case TCP_P_SACK:
		case TCP_V_SACK:
			break;
		case TCP_O_TS:
			tsopt = (struct tcp_t_opt *)p;
			rmt_timestamp = tsopt->t_snd;
			return;
		}

		/* Process optional NOPs */
		if (p[0] == TCP_O_NOP)
			p++;
	}
}

static u8 tcp_state_machine(u8 tcp_flags, u32 tcp_seq_num, int payload_len)
{
	u8 tcp_fin = tcp_flags & TCP_FIN;
	u8 tcp_syn = tcp_flags & TCP_SYN;
	u8 tcp_rst = tcp_flags & TCP_RST;
	u8 tcp_push = tcp_flags & TCP_PUSH;
	u8 tcp_ack = tcp_flags & TCP_ACK;
	u8 action = TCP_DATA;
	int i;

	/*
	 * tcp_flags are examined to determine TX action in a given state
	 * tcp_push is interpreted to mean "inform the app"
	 * urg, ece, cer and nonce flags are not supported.
	 *
	 * exe and crw are use to signal and confirm knowledge of congestion.
	 * This TCP only sends a file request and acks. If it generates
	 * congestion, the network is broken.
	 */
	debug_cond(DEBUG_INT_STATE, "TCP STATE ENTRY %x\n", action);
	if (tcp_rst) {
		action = TCP_DATA;
		current_tcp_state = TCP_CLOSED;
		net_set_state(NETLOOP_FAIL);
		debug_cond(DEBUG_INT_STATE, "TCP Reset %x\n", tcp_flags);
		return TCP_RST;
	}

	switch  (current_tcp_state) {
	case TCP_CLOSED:
		debug_cond(DEBUG_INT_STATE, "TCP CLOSED %x\n", tcp_flags);
		if (tcp_syn) {
			action = TCP_SYN | TCP_ACK;
			tcp_seq_init = tcp_seq_num;
			tcp_ack_edge = tcp_seq_num + 1;
			current_tcp_state = TCP_SYN_RECEIVED;
		} else if (tcp_ack || tcp_fin) {
			action = TCP_DATA;
		}
		break;
	case TCP_SYN_RECEIVED:
	case TCP_SYN_SENT:
		debug_cond(DEBUG_INT_STATE, "TCP_SYN_SENT | TCP_SYN_RECEIVED %x, %u\n",
			   tcp_flags, tcp_seq_num);
		if (tcp_fin) {
			action = action | TCP_PUSH;
			current_tcp_state = TCP_CLOSE_WAIT;
		} else if (tcp_ack || (tcp_syn && tcp_ack)) {
			action |= TCP_ACK;
			tcp_seq_init = tcp_seq_num;
			tcp_ack_edge = tcp_seq_num + 1;
			sack_idx = 0;
			edge_a[sack_idx].se.l = tcp_ack_edge;
			edge_a[sack_idx].se.r = tcp_ack_edge;
			prev_len = 0;
			current_tcp_state = TCP_ESTABLISHED;
			for (i = 0; i < TCP_SACK; i++)
				edge_a[i].st = NOPKT;

			if (tcp_syn && tcp_ack)
				action |= TCP_PUSH;
		} else {
			action = TCP_DATA;
		}
		break;
	case TCP_ESTABLISHED:
		debug_cond(DEBUG_INT_STATE, "TCP_ESTABLISHED %x\n", tcp_flags);
		if (payload_len > 0) {
			tcp_hole(tcp_seq_num, payload_len);
			tcp_fin = TCP_DATA;  /* cause standalone FIN */
		}

		if ((tcp_fin) &&
		    (!IS_ENABLED(CONFIG_PROT_TCP_SACK) ||
		     tcp_lost.len <= TCP_OPT_LEN_2)) {
			action = action | TCP_FIN | TCP_PUSH | TCP_ACK;
			current_tcp_state = TCP_CLOSE_WAIT;
		} else if (tcp_ack) {
			action = TCP_DATA;
		}

		if (tcp_syn)
			action = TCP_ACK + TCP_RST;
		else if (tcp_push)
			action = action | TCP_PUSH;
		break;
	case TCP_CLOSE_WAIT:
		debug_cond(DEBUG_INT_STATE, "TCP_CLOSE_WAIT (%x)\n", tcp_flags);
		action = TCP_DATA;
		break;
	case TCP_FIN_WAIT_2:
		debug_cond(DEBUG_INT_STATE, "TCP_FIN_WAIT_2 (%x)\n", tcp_flags);
		if (tcp_ack) {
			action = TCP_PUSH | TCP_ACK;
			current_tcp_state = TCP_CLOSED;
			puts("\n");
		} else if (tcp_syn) {
			action = TCP_DATA;
		} else if (tcp_fin) {
			action = TCP_DATA;
		}
		break;
	case TCP_FIN_WAIT_1:
		debug_cond(DEBUG_INT_STATE, "TCP_FIN_WAIT_1 (%x)\n", tcp_flags);
		if (tcp_fin) {
			tcp_ack_edge++;
			action = TCP_ACK | TCP_FIN;
			current_tcp_state = TCP_FIN_WAIT_2;
		}
		if (tcp_syn)
			action = TCP_RST;
		if (tcp_ack)
			current_tcp_state = TCP_CLOSED;
		break;
	case TCP_CLOSING:
		debug_cond(DEBUG_INT_STATE, "TCP_CLOSING (%x)\n", tcp_flags);
		if (tcp_ack) {
			action = TCP_PUSH;
			current_tcp_state = TCP_CLOSED;
			puts("\n");
		} else if (tcp_syn) {
			action = TCP_RST;
		} else if (tcp_fin) {
			action = TCP_DATA;
		}
		break;
	}
	return action;
}

/**
 * rxhand_tcp_f() - process receiving data and call data handler.
 * @b: the packet
 * @pkt_len: the length of packet.
 */
void rxhand_tcp_f(union tcp_build_pkt *b, unsigned int pkt_len)
{
	int tcp_len = pkt_len - IP_HDR_SIZE;
	u16 tcp_rx_xsum = b->ip.hdr.ip_sum;
	u8  tcp_action = TCP_DATA;
	u32 tcp_seq_num, tcp_ack_num;
	int tcp_hdr_len, payload_len;

	/* Verify IP header */
	debug_cond(DEBUG_DEV_PKT,
		   "TCP RX in RX Sum (to=%pI4, from=%pI4, len=%d)\n",
		   &b->ip.hdr.ip_src, &b->ip.hdr.ip_dst, pkt_len);

	b->ip.hdr.ip_src = net_server_ip;
	b->ip.hdr.ip_dst = net_ip;
	b->ip.hdr.ip_sum = 0;
	if (tcp_rx_xsum != compute_ip_checksum(b, IP_HDR_SIZE)) {
		debug_cond(DEBUG_DEV_PKT,
			   "TCP RX IP xSum Error (%pI4, =%pI4, len=%d)\n",
			   &net_ip, &net_server_ip, pkt_len);
		return;
	}

	/* Build pseudo header and verify TCP header */
	tcp_rx_xsum = b->ip.hdr.tcp_xsum;
	b->ip.hdr.tcp_xsum = 0;
	if (tcp_rx_xsum != tcp_set_pseudo_header((uchar *)b, b->ip.hdr.ip_src,
						 b->ip.hdr.ip_dst, tcp_len,
						 pkt_len)) {
		debug_cond(DEBUG_DEV_PKT,
			   "TCP RX TCP xSum Error (%pI4, %pI4, len=%d)\n",
			   &net_ip, &net_server_ip, tcp_len);
		return;
	}

	tcp_hdr_len = GET_TCP_HDR_LEN_IN_BYTES(b->ip.hdr.tcp_hlen);
	payload_len = tcp_len - tcp_hdr_len;

	if (tcp_hdr_len > TCP_HDR_SIZE)
		tcp_parse_options((uchar *)b + IP_TCP_HDR_SIZE,
				  tcp_hdr_len - TCP_HDR_SIZE);
	/*
	 * Incoming sequence and ack numbers are server's view of the numbers.
	 * The app must swap the numbers when responding.
	 */
	tcp_seq_num = ntohl(b->ip.hdr.tcp_seq);
	tcp_ack_num = ntohl(b->ip.hdr.tcp_ack);

	/* Packets are not ordered. Send to app as received. */
	tcp_action = tcp_state_machine(b->ip.hdr.tcp_flags,
				       tcp_seq_num, payload_len);

	tcp_activity_count++;
	if (tcp_activity_count > TCP_ACTIVITY) {
		puts("| ");
		tcp_activity_count = 0;
	}

	if ((tcp_action & TCP_PUSH) || payload_len > 0) {
		debug_cond(DEBUG_DEV_PKT,
			   "TCP Notify (action=%x, Seq=%u,Ack=%u,Pay%d)\n",
			   tcp_action, tcp_seq_num, tcp_ack_num, payload_len);

		(*tcp_packet_handler) ((uchar *)b + pkt_len - payload_len, b->ip.hdr.tcp_dst,
				       b->ip.hdr.ip_src, b->ip.hdr.tcp_src, tcp_seq_num,
				       tcp_ack_num, tcp_action, payload_len);

	} else if (tcp_action != TCP_DATA) {
		debug_cond(DEBUG_DEV_PKT,
			   "TCP Action (action=%x,Seq=%u,Ack=%u,Pay=%d)\n",
			   tcp_action, tcp_ack_num, tcp_ack_edge, payload_len);

		/*
		 * Warning: Incoming Ack & Seq sequence numbers are transposed
		 * here to outgoing Seq & Ack sequence numbers
		 */
		net_send_tcp_packet(0, ntohs(b->ip.hdr.tcp_src),
				    ntohs(b->ip.hdr.tcp_dst),
				    (tcp_action & (~TCP_PUSH)),
				    tcp_ack_num, tcp_ack_edge);
	}
}
