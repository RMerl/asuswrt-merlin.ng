/*
 * INET		An implementation of the TCP/IP protocol suite for the LINUX
 *		operating system.  INET is implemented using the  BSD Socket
 *		interface as the means of communication with the user level.
 *
 *		Definitions for the TCP protocol.
 *
 * Version:	@(#)tcp.h	1.0.2	04/28/93
 *
 * Author:	Fred N. van Kempen, <waltje@uWalt.NL.Mugnet.ORG>
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 */
#ifndef _LINUX_TCP_H
#define _LINUX_TCP_H


#include <linux/skbuff.h>
#include <net/sock.h>
#include <net/inet_connection_sock.h>
#include <net/inet_timewait_sock.h>
#include <uapi/linux/tcp.h>

static inline struct tcphdr *tcp_hdr(const struct sk_buff *skb)
{
	return (struct tcphdr *)skb_transport_header(skb);
}

static inline unsigned int __tcp_hdrlen(const struct tcphdr *th)
{
	return th->doff * 4;
}

static inline unsigned int tcp_hdrlen(const struct sk_buff *skb)
{
	return __tcp_hdrlen(tcp_hdr(skb));
}

static inline struct tcphdr *inner_tcp_hdr(const struct sk_buff *skb)
{
	return (struct tcphdr *)skb_inner_transport_header(skb);
}

static inline unsigned int inner_tcp_hdrlen(const struct sk_buff *skb)
{
	return inner_tcp_hdr(skb)->doff * 4;
}

static inline unsigned int tcp_optlen(const struct sk_buff *skb)
{
	return (tcp_hdr(skb)->doff - 5) * 4;
}

/* TCP Fast Open */
#define TCP_FASTOPEN_COOKIE_MIN	4	/* Min Fast Open Cookie size in bytes */
#define TCP_FASTOPEN_COOKIE_MAX	16	/* Max Fast Open Cookie size in bytes */
#if !defined(CONFIG_BCM_MPTCP) || !defined(CONFIG_BCM_KF_MPTCP)
#define TCP_FASTOPEN_COOKIE_SIZE 8	/* the size employed by this impl. */
#else
#define TCP_FASTOPEN_COOKIE_SIZE 4	/* the size employed by this impl. */
#endif

/* TCP Fast Open Cookie as stored in memory */
struct tcp_fastopen_cookie {
	s8	len;
	u8	val[TCP_FASTOPEN_COOKIE_MAX];
	bool	exp;	/* In RFC6994 experimental option format */
};

/* This defines a selective acknowledgement block. */
struct tcp_sack_block_wire {
	__be32	start_seq;
	__be32	end_seq;
};

struct tcp_sack_block {
	u32	start_seq;
	u32	end_seq;
};

#if defined(CONFIG_BCM_MPTCP) && defined(CONFIG_BCM_KF_MPTCP)
struct tcp_out_options {
	u16	options;	/* bit field of OPTION_* */
	u8	ws;		/* window scale, 0 to disable */
	u8	num_sack_blocks;/* number of SACK blocks to include */
	u8	hash_size;	/* bytes in hash_location */
	u16	mss;		/* 0 to disable */
	__u8	*hash_location;	/* temporary pointer, overloaded */
	__u32	tsval, tsecr;	/* need to include OPTION_TS */
	struct tcp_fastopen_cookie *fastopen_cookie;	/* Fast open cookie */
#ifdef CONFIG_MPTCP
	u16	mptcp_options;	/* bit field of MPTCP related OPTION_* */
	u8	dss_csum:1,	/* dss-checksum required? */
		add_addr_v4:1,
		add_addr_v6:1,
		mptcp_ver:4;

	union {
		struct {
			__u64	sender_key;	/* sender's key for mptcp */
			__u64	receiver_key;	/* receiver's key for mptcp */
		} mp_capable;

		struct {
			__u64	sender_truncated_mac;
			__u32	sender_nonce;
					/* random number of the sender */
			__u32	token;	/* token for mptcp */
			u8	low_prio:1;
		} mp_join_syns;
	};

	struct {
		__u64 trunc_mac;
		struct in_addr addr;
		u16 port;
		u8 addr_id;
	} add_addr4;

	struct {
		__u64 trunc_mac;
		struct in6_addr addr;
		u16 port;
		u8 addr_id;
	} add_addr6;

	u16	remove_addrs;	/* list of address id */
	u8	addr_id;	/* address id (mp_join or add_address) */
#endif /* CONFIG_MPTCP */
};

#endif
/*These are used to set the sack_ok field in struct tcp_options_received */
#define TCP_SACK_SEEN     (1 << 0)   /*1 = peer is SACK capable, */
#define TCP_FACK_ENABLED  (1 << 1)   /*1 = FACK is enabled locally*/
#define TCP_DSACK_SEEN    (1 << 2)   /*1 = DSACK was received from peer*/

struct tcp_options_received {
/*	PAWS/RTTM data	*/
	long	ts_recent_stamp;/* Time we stored ts_recent (for aging) */
	u32	ts_recent;	/* Time stamp to echo next		*/
	u32	rcv_tsval;	/* Time stamp value             	*/
	u32	rcv_tsecr;	/* Time stamp echo reply        	*/
	u16 	saw_tstamp : 1,	/* Saw TIMESTAMP on last packet		*/
		tstamp_ok : 1,	/* TIMESTAMP seen on SYN packet		*/
		dsack : 1,	/* D-SACK is scheduled			*/
		wscale_ok : 1,	/* Wscale seen on SYN packet		*/
		sack_ok : 4,	/* SACK seen on SYN packet		*/
		snd_wscale : 4,	/* Window scaling received from sender	*/
		rcv_wscale : 4;	/* Window scaling to send to receiver	*/
	u8	num_sacks;	/* Number of SACK blocks		*/
	u16	user_mss;	/* mss requested by user in ioctl	*/
	u16	mss_clamp;	/* Maximal mss, negotiated at connection setup */
};

#if defined(CONFIG_BCM_MPTCP) && defined(CONFIG_BCM_KF_MPTCP)
struct mptcp_cb;
struct mptcp_tcp_sock;

#endif
static inline void tcp_clear_options(struct tcp_options_received *rx_opt)
{
	rx_opt->tstamp_ok = rx_opt->sack_ok = 0;
	rx_opt->wscale_ok = rx_opt->snd_wscale = 0;
}

/* This is the max number of SACKS that we'll generate and process. It's safe
 * to increase this, although since:
 *   size = TCPOLEN_SACK_BASE_ALIGNED (4) + n * TCPOLEN_SACK_PERBLOCK (8)
 * only four options will fit in a standard TCP header */
#define TCP_NUM_SACKS 4

struct tcp_request_sock_ops;

struct tcp_request_sock {
	struct inet_request_sock 	req;
	const struct tcp_request_sock_ops *af_specific;
	bool				tfo_listener;
	u32				rcv_isn;
	u32				snt_isn;
	u32				snt_synack; /* synack sent time */
	u32				last_oow_ack_time; /* last SYNACK */
	u32				rcv_nxt; /* the ack # by SYNACK. For
						  * FastOpen it's the seq#
						  * after data-in-SYN.
						  */
};

static inline struct tcp_request_sock *tcp_rsk(const struct request_sock *req)
{
	return (struct tcp_request_sock *)req;
}

#if defined(CONFIG_BCM_MPTCP) && defined(CONFIG_BCM_KF_MPTCP)
struct tcp_md5sig_key;

#endif
struct tcp_sock {
	/* inet_connection_sock has to be the first member of tcp_sock */
	struct inet_connection_sock	inet_conn;
	u16	tcp_header_len;	/* Bytes of tcp header to send		*/
	u16	gso_segs;	/* Max number of segs per GSO packet	*/

/*
 *	Header prediction flags
 *	0x5?10 << 16 + snd_wnd in net byte order
 */
	__be32	pred_flags;

/*
 *	RFC793 variables by their proper names. This means you can
 *	read the code and the spec side by side (and laugh ...)
 *	See RFC793 and RFC1122. The RFC writes these in capitals.
 */
	u64	bytes_received;	/* RFC4898 tcpEStatsAppHCThruOctetsReceived
				 * sum(delta(rcv_nxt)), or how many bytes
				 * were acked.
				 */
 	u32	rcv_nxt;	/* What we want to receive next 	*/
	u32	copied_seq;	/* Head of yet unread data		*/
	u32	rcv_wup;	/* rcv_nxt on last window update sent	*/
 	u32	snd_nxt;	/* Next sequence we send		*/

	u64	bytes_acked;	/* RFC4898 tcpEStatsAppHCThruOctetsAcked
				 * sum(delta(snd_una)), or how many bytes
				 * were acked.
				 */
	struct u64_stats_sync syncp; /* protects 64bit vars (cf tcp_get_info()) */

 	u32	snd_una;	/* First byte we want an ack for	*/
 	u32	snd_sml;	/* Last byte of the most recently transmitted small packet */
	u32	rcv_tstamp;	/* timestamp of last received ACK (for keepalives) */
	u32	lsndtime;	/* timestamp of last sent data packet (for restart window) */
	u32	last_oow_ack_time;  /* timestamp of last out-of-window ACK */

	u32	tsoffset;	/* timestamp offset */

#if !defined(CONFIG_BCM_KF_TCP_NO_TSQ) 
	struct list_head tsq_node; /* anchor in tsq_tasklet.head list */
#endif
	unsigned long	tsq_flags;

	/* Data for direct copy to user */
	struct {
		struct sk_buff_head	prequeue;
		struct task_struct	*task;
		struct msghdr		*msg;
		int			memory;
		int			len;
	} ucopy;

	u32	snd_wl1;	/* Sequence for window update		*/
	u32	snd_wnd;	/* The window we expect to receive	*/
	u32	max_window;	/* Maximal window ever seen from peer	*/
	u32	mss_cache;	/* Cached effective mss, not including SACKS */

	u32	window_clamp;	/* Maximal window to advertise		*/
	u32	rcv_ssthresh;	/* Current window clamp			*/

	u16	advmss;		/* Advertised MSS			*/
	u8	unused;
	u8	nonagle     : 4,/* Disable Nagle algorithm?             */
		thin_lto    : 1,/* Use linear timeouts for thin streams */
		thin_dupack : 1,/* Fast retransmit on first dupack      */
		repair      : 1,
		frto        : 1;/* F-RTO (RFC5682) activated in CA_Loss */
	u8	repair_queue;
	u8	do_early_retrans:1,/* Enable RFC5827 early-retransmit  */
		syn_data:1,	/* SYN includes data */
		syn_fastopen:1,	/* SYN includes Fast Open option */
		syn_fastopen_exp:1,/* SYN includes Fast Open exp. option */
		syn_data_acked:1,/* data in SYN is acked by SYN-ACK */
		is_cwnd_limited:1;/* forward progress limited by snd_cwnd? */
	u32	tlp_high_seq;	/* snd_nxt at the time of TLP retransmit. */

/* RTT measurement */
	u32	srtt_us;	/* smoothed round trip time << 3 in usecs */
	u32	mdev_us;	/* medium deviation			*/
	u32	mdev_max_us;	/* maximal mdev for the last rtt period	*/
	u32	rttvar_us;	/* smoothed mdev_max			*/
	u32	rtt_seq;	/* sequence number to update rttvar	*/

	u32	packets_out;	/* Packets which are "in flight"	*/
	u32	retrans_out;	/* Retransmitted packets out		*/
	u32	max_packets_out;  /* max packets_out in last window */
	u32	max_packets_seq;  /* right edge of max_packets_out flight */

	u16	urg_data;	/* Saved octet of OOB data and control flags */
	u8	ecn_flags;	/* ECN status bits.			*/
	u8	keepalive_probes; /* num of allowed keep alive probes	*/
	u32	reordering;	/* Packet reordering metric.		*/
	u32	snd_up;		/* Urgent pointer		*/

/*
 *      Options received (usually on last packet, some only on SYN packets).
 */
	struct tcp_options_received rx_opt;

/*
 *	Slow start and congestion control (see also Nagle, and Karn & Partridge)
 */
 	u32	snd_ssthresh;	/* Slow start size threshold		*/
 	u32	snd_cwnd;	/* Sending congestion window		*/
	u32	snd_cwnd_cnt;	/* Linear increase counter		*/
	u32	snd_cwnd_clamp; /* Do not allow snd_cwnd to grow above this */
	u32	snd_cwnd_used;
	u32	snd_cwnd_stamp;
	u32	prior_cwnd;	/* Congestion window at start of Recovery. */
	u32	prr_delivered;	/* Number of newly delivered packets to
				 * receiver in Recovery. */
	u32	prr_out;	/* Total number of pkts sent during Recovery. */

 	u32	rcv_wnd;	/* Current receiver window		*/
	u32	write_seq;	/* Tail(+1) of data held in tcp send buffer */
	u32	notsent_lowat;	/* TCP_NOTSENT_LOWAT */
	u32	pushed_seq;	/* Last pushed seq, required to talk to windows */
	u32	lost_out;	/* Lost packets			*/
	u32	sacked_out;	/* SACK'd packets			*/
	u32	fackets_out;	/* FACK'd packets			*/

	/* from STCP, retrans queue hinting */
	struct sk_buff* lost_skb_hint;
	struct sk_buff *retransmit_skb_hint;

	/* OOO segments go in this list. Note that socket lock must be held,
	 * as we do not use sk_buff_head lock.
	 */
	struct sk_buff_head	out_of_order_queue;

	/* SACKs data, these 2 need to be together (see tcp_options_write) */
	struct tcp_sack_block duplicate_sack[1]; /* D-SACK block */
	struct tcp_sack_block selective_acks[4]; /* The SACKS themselves*/

	struct tcp_sack_block recv_sack_cache[4];

	struct sk_buff *highest_sack;   /* skb just after the highest
					 * skb with SACKed bit set
					 * (validity guaranteed only if
					 * sacked_out > 0)
					 */

	int     lost_cnt_hint;
	u32     retransmit_high;	/* L-bits may be on up to this seqno */

	u32	lost_retrans_low;	/* Sent seq after any rxmit (lowest) */

	u32	prior_ssthresh; /* ssthresh saved at recovery start	*/
	u32	high_seq;	/* snd_nxt at onset of congestion	*/

	u32	retrans_stamp;	/* Timestamp of the last retransmit,
				 * also used in SYN-SENT to remember stamp of
				 * the first SYN. */
	u32	undo_marker;	/* snd_una upon a new recovery episode. */
	int	undo_retrans;	/* number of undoable retransmissions. */
	u32	total_retrans;	/* Total retransmits for entire connection */

	u32	urg_seq;	/* Seq of received urgent pointer */
	unsigned int		keepalive_time;	  /* time before keep alive takes place */
	unsigned int		keepalive_intvl;  /* time interval between keep alive probes */

	int			linger2;

/* Receiver side RTT estimation */
	struct {
		u32	rtt;
		u32	seq;
		u32	time;
	} rcv_rtt_est;

/* Receiver queue space */
	struct {
		int	space;
		u32	seq;
		u32	time;
	} rcvq_space;

/* TCP-specific MTU probe information. */
	struct {
		u32		  probe_seq_start;
		u32		  probe_seq_end;
	} mtu_probe;
	u32	mtu_info; /* We received an ICMP_FRAG_NEEDED / ICMPV6_PKT_TOOBIG
			   * while socket was owned by user.
			   */

#ifdef CONFIG_TCP_MD5SIG
/* TCP AF-Specific parts; only used by MD5 Signature support so far */
	const struct tcp_sock_af_ops	*af_specific;

/* TCP MD5 Signature Option information */
	struct tcp_md5sig_info	__rcu *md5sig_info;
#endif

/* TCP fastopen related information */
	struct tcp_fastopen_request *fastopen_req;
	/* fastopen_rsk points to request_sock that resulted in this big
	 * socket. Used to retransmit SYNACKs etc.
	 */
	struct request_sock *fastopen_rsk;
#if defined(CONFIG_BCM_KF_SPEEDYGET) && defined(CONFIG_BCM_SPEEDYGET)
    u8 tcp_nocopy;
#endif
#if defined(CONFIG_BCM_MPTCP) && defined(CONFIG_BCM_KF_MPTCP)

	/* MPTCP/TCP-specific callbacks */
	const struct tcp_sock_ops	*ops;

	struct mptcp_cb		*mpcb;
	struct sock		*meta_sk;
	/* We keep these flags even if CONFIG_MPTCP is not checked, because
	 * it allows checking MPTCP capability just by checking the mpc flag,
	 * rather than adding ifdefs everywhere.
	 */
	u16     mpc:1,          /* Other end is multipath capable */
		inside_tk_table:1, /* Is the tcp_sock inside the token-table? */
		send_mp_fclose:1,
		request_mptcp:1, /* Did we send out an MP_CAPABLE?
				  * (this speeds up mptcp_doit() in tcp_recvmsg)
				  */
		pf:1, /* Potentially Failed state: when this flag is set, we
		       * stop using the subflow
		       */
		mp_killed:1, /* Killed with a tcp_done in mptcp? */
		was_meta_sk:1,	/* This was a meta sk (in case of reuse) */
		is_master_sk:1,
		close_it:1,	/* Must close socket in mptcp_data_ready? */
		closing:1,
		mptcp_ver:4;
	struct mptcp_tcp_sock *mptcp;
#ifdef CONFIG_MPTCP
	struct hlist_nulls_node tk_table;
	u32		mptcp_loc_token;
	u64		mptcp_loc_key;
#endif /* CONFIG_MPTCP */
#endif
};

enum tsq_flags {
	TSQ_THROTTLED,
	TSQ_QUEUED,
	TCP_TSQ_DEFERRED,	   /* tcp_tasklet_func() found socket was owned */
	TCP_WRITE_TIMER_DEFERRED,  /* tcp_write_timer() found socket was owned */
	TCP_DELACK_TIMER_DEFERRED, /* tcp_delack_timer() found socket was owned */
	TCP_MTU_REDUCED_DEFERRED,  /* tcp_v{4|6}_err() could not call
				    * tcp_v{4|6}_mtu_reduced()
				    */
#if defined(CONFIG_BCM_MPTCP) && defined(CONFIG_BCM_KF_MPTCP)
	MPTCP_PATH_MANAGER, /* MPTCP deferred creation of new subflows */
	MPTCP_SUB_DEFERRED, /* A subflow got deferred - process them */
#endif
};

static inline struct tcp_sock *tcp_sk(const struct sock *sk)
{
	return (struct tcp_sock *)sk;
}

struct tcp_timewait_sock {
	struct inet_timewait_sock tw_sk;
	u32			  tw_rcv_nxt;
	u32			  tw_snd_nxt;
	u32			  tw_rcv_wnd;
	u32			  tw_ts_offset;
	u32			  tw_ts_recent;

	/* The time we sent the last out-of-window ACK: */
	u32			  tw_last_oow_ack_time;

	long			  tw_ts_recent_stamp;
#ifdef CONFIG_TCP_MD5SIG
	struct tcp_md5sig_key	  *tw_md5_key;
#endif
#if defined(CONFIG_BCM_MPTCP) && defined(CONFIG_BCM_KF_MPTCP)
	struct mptcp_tw		  *mptcp_tw;
#endif
};

static inline struct tcp_timewait_sock *tcp_twsk(const struct sock *sk)
{
	return (struct tcp_timewait_sock *)sk;
}

static inline bool tcp_passive_fastopen(const struct sock *sk)
{
	return (sk->sk_state == TCP_SYN_RECV &&
		tcp_sk(sk)->fastopen_rsk != NULL);
}

extern void tcp_sock_destruct(struct sock *sk);

static inline int fastopen_init_queue(struct sock *sk, int backlog)
{
	struct request_sock_queue *queue =
	    &inet_csk(sk)->icsk_accept_queue;

	if (queue->fastopenq == NULL) {
		queue->fastopenq = kzalloc(
		    sizeof(struct fastopen_queue),
		    sk->sk_allocation);
		if (queue->fastopenq == NULL)
			return -ENOMEM;

		sk->sk_destruct = tcp_sock_destruct;
		spin_lock_init(&queue->fastopenq->lock);
	}
	queue->fastopenq->max_qlen = backlog;
	return 0;
}

#if defined(CONFIG_BCM_KF_MISC_BACKPORTS)
/*CVE-2019-11477*/
int tcp_skb_shift(struct sk_buff *to, struct sk_buff *from, int pcount,
		  int shiftlen);
#endif //#if defined(CONFIG_BCM_KF_MISC_BACKPORTS)

#endif	/* _LINUX_TCP_H */
