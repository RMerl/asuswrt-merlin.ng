/* SPDX-License-Identifier: GPL-2.0 */
/*
 * TCP Support with SACK for file transfer.
 *
 * Copyright 2017 Duncan Hare, All rights reserved.
 */

#define TCP_ACTIVITY 127		/* Number of packets received   */
					/* before console progress mark */
/**
 * struct ip_tcp_hdr - IP and TCP header
 * @ip_hl_v: header length and version
 * @ip_tos: type of service
 * @ip_len: total length
 * @ip_id: identification
 * @ip_off: fragment offset field
 * @ip_ttl: time to live
 * @ip_p: protocol
 * @ip_sum: checksum
 * @ip_src: Source IP address
 * @ip_dst: Destination IP address
 * @tcp_src: TCP source port
 * @tcp_dst: TCP destination port
 * @tcp_seq: TCP sequence number
 * @tcp_ack: TCP Acknowledgment number
 * @tcp_hlen: 4 bits TCP header Length/4, 4 bits reserved, 2 more bits reserved
 * @tcp_flag: flags of TCP
 * @tcp_win: TCP windows size
 * @tcp_xsum: Checksum
 * @tcp_ugr: Pointer to urgent data
 */
struct ip_tcp_hdr {
	u8		ip_hl_v;
	u8		ip_tos;
	u16		ip_len;
	u16		ip_id;
	u16		ip_off;
	u8		ip_ttl;
	u8		ip_p;
	u16		ip_sum;
	struct in_addr	ip_src;
	struct in_addr	ip_dst;
	u16		tcp_src;
	u16		tcp_dst;
	u32		tcp_seq;
	u32		tcp_ack;
	u8		tcp_hlen;
	u8		tcp_flags;
	u16		tcp_win;
	u16		tcp_xsum;
	u16		tcp_ugr;
} __packed;

#define IP_TCP_HDR_SIZE		(sizeof(struct ip_tcp_hdr))
#define TCP_HDR_SIZE		(IP_TCP_HDR_SIZE  - IP_HDR_SIZE)

#define TCP_DATA	0x00	/* Data Packet - internal use only	*/
#define TCP_FIN		0x01	/* Finish flag				*/
#define TCP_SYN		0x02	/* Synch (start) flag			*/
#define TCP_RST		0x04	/* reset flag				*/
#define TCP_PUSH	0x08	/* Push - Notify app			*/
#define TCP_ACK		0x10	/* Acknowledgment of data received	*/
#define TCP_URG		0x20	/* Urgent				*/
#define TCP_ECE		0x40	/* Congestion control			*/
#define TCP_CWR		0x80	/* Congestion Control			*/

/*
 * TCP header options, Seq, MSS, and SACK
 */

#define TCP_SACK 32			/* Number of packets analyzed   */
					/* on leading edge of stream    */

#define TCP_O_END	0x00		/* End of option list		*/
#define TCP_1_NOP	0x01		/* Single padding NOP		*/
#define TCP_O_NOP	0x01010101	/* NOPs pad to 32 bit boundary	*/
#define TCP_O_MSS	0x02		/* MSS Size option		*/
#define TCP_O_SCL	0x03		/* Window Scale option		*/
#define TCP_P_SACK	0x04		/* SACK permitted		*/
#define TCP_V_SACK	0x05		/* SACK values			*/
#define TCP_O_TS	0x08		/* Timestamp option		*/
#define TCP_OPT_LEN_2	0x02
#define TCP_OPT_LEN_3	0x03
#define TCP_OPT_LEN_4	0x04
#define TCP_OPT_LEN_6	0x06
#define TCP_OPT_LEN_8	0x08
#define TCP_OPT_LEN_A	0x0a		/* Timestamp Length		*/
#define TCP_MSS		1460		/* Max segment size		*/
#define TCP_SCALE	0x01		/* Scale			*/

/**
 * struct tcp_mss - TCP option structure for MSS (Max segment size)
 * @kind: Field ID
 * @len: Field length
 * @mss: Segment size value
 */
struct tcp_mss {
	u8	kind;
	u8	len;
	u16	mss;
} __packed;

/**
 * struct tcp_scale - TCP option structure for Windows scale
 * @kind: Field ID
 * @len: Field length
 * @scale: windows shift value used for networks with many hops.
 *         Typically 4 or more hops
 */
struct tcp_scale {
	u8	kind;
	u8	len;
	u8	scale;
} __packed;

/**
 * struct tcp_sack_p - TCP option structure for SACK permitted
 * @kind: Field ID
 * @len: Field length
 */
struct tcp_sack_p {
	u8	kind;
	u8	len;
} __packed;

/**
 * struct sack_edges - structure for SACK edges
 * @l: Left edge of stream
 * @r: right edge of stream
 */
struct sack_edges {
	u32	l;
	u32	r;
} __packed;

#define TCP_SACK_SIZE (sizeof(struct sack_edges))

/*
 * A TCP stream has holes when packets are missing or disordered.
 * A hill is the inverse of a hole, and is data received.
 * TCP received hills (a sequence of data), and inferrs Holes
 * from the "hills" or packets received.
 */

#define TCP_SACK_HILLS	4

/**
 * struct tcp_sack_v - TCP option structure for SACK
 * @kind: Field ID
 * @len: Field length
 * @hill: L & R window edges
 */
struct tcp_sack_v {
	u8	kind;
	u8	len;
	struct	sack_edges hill[TCP_SACK_HILLS];
} __packed;

/**
 * struct tcp_t_opt - TCP option structure for time stamps
 * @kind: Field ID
 * @len: Field length
 * @t_snd: Sender timestamp
 * @t_rcv: Receiver timestamp
 */
struct tcp_t_opt {
	u8	kind;
	u8	len;
	u32	t_snd;
	u32	t_rcv;
} __packed;

#define TCP_TSOPT_SIZE (sizeof(struct tcp_t_opt))

/*
 * ip tcp  structure with options
 */

/**
 * struct ip_tcp_hdr_o - IP + TCP header + TCP options
 * @hdr: IP + TCP header
 * @mss: TCP MSS Option
 * @scale: TCP Windows Scale Option
 * @sack_p: TCP Sack-Permitted Option
 * @t_opt: TCP Timestamp Option
 * @end: end of options
 */
struct ip_tcp_hdr_o {
	struct	ip_tcp_hdr hdr;
	struct	tcp_mss	   mss;
	struct	tcp_scale  scale;
	struct	tcp_sack_p sack_p;
	struct	tcp_t_opt  t_opt;
	u8	end;
} __packed;

#define IP_TCP_O_SIZE (sizeof(struct ip_tcp_hdr_o))

/**
 * struct ip_tcp_hdr_s - IP + TCP header + TCP options
 * @hdr: IP + TCP header
 * @t_opt: TCP Timestamp Option
 * @sack_v: TCP SACK Option
 * @end: end of options
 */
struct ip_tcp_hdr_s {
	struct	ip_tcp_hdr	hdr;
	struct	tcp_t_opt	t_opt;
	struct	tcp_sack_v	sack_v;
	u8	end;
} __packed;

#define IP_TCP_SACK_SIZE (sizeof(struct ip_tcp_hdr_s))

/*
 * TCP pseudo header definitions
 */
#define PSEUDO_PAD_SIZE	8

/**
 * struct pseudo_hdr - Pseudo Header
 * @padding: pseudo hdr size = ip_tcp hdr size
 * @p_src: Source IP address
 * @p_dst: Destination IP address
 * @rsvd: reserved
 * @p: protocol
 * @len: length of header
 */
struct pseudo_hdr {
	u8 padding[PSEUDO_PAD_SIZE];
	struct in_addr p_src;
	struct in_addr p_dst;
	u8      rsvd;
	u8      p;
	u16     len;
} __packed;

#define PSEUDO_HDR_SIZE	(sizeof(struct pseudo_hdr)) - PSEUDO_PAD_SIZE

/**
 * union tcp_build_pkt - union for building TCP/IP packet.
 * @ph: pseudo header
 * @ip: IP and TCP header plus TCP options
 * @sack: IP and TCP header plus SACK options
 * @raw: buffer
 *
 * Build Pseudo header in packed buffer
 * first, calculate TCP checksum, then build IP header in packed buffer.
 *
 */
union tcp_build_pkt {
	struct pseudo_hdr ph;
	struct ip_tcp_hdr_o ip;
	struct ip_tcp_hdr_s sack;
	uchar  raw[1600];
} __packed;

/**
 * enum tcp_state - TCP State machine states for connection
 * @TCP_CLOSED: Need to send SYN to connect
 * @TCP_SYN_SENT: Trying to connect, waiting for SYN ACK
 * @TCP_SYN_RECEIVED: Initial SYN received, waiting for ACK
 * @TCP_ESTABLISHED: both server & client have a connection
 * @TCP_CLOSE_WAIT: Rec FIN, passed to app for FIN, ACK rsp
 * @TCP_CLOSING: Rec FIN, sent FIN, ACK waiting for ACK
 * @TCP_FIN_WAIT_1: Sent FIN waiting for response
 * @TCP_FIN_WAIT_2: Rec ACK from FIN sent, waiting for FIN
 */
enum tcp_state {
	TCP_CLOSED,
	TCP_SYN_SENT,
	TCP_SYN_RECEIVED,
	TCP_ESTABLISHED,
	TCP_CLOSE_WAIT,
	TCP_CLOSING,
	TCP_FIN_WAIT_1,
	TCP_FIN_WAIT_2
};

enum tcp_state tcp_get_tcp_state(void);
void tcp_set_tcp_state(enum tcp_state new_state);
int tcp_set_tcp_header(uchar *pkt, int dport, int sport, int payload_len,
		       u8 action, u32 tcp_seq_num, u32 tcp_ack_num);

/**
 * rxhand_tcp() - An incoming packet handler.
 * @pkt: pointer to the application packet
 * @dport: destination TCP port
 * @sip: source IP address
 * @sport: source TCP port
 * @tcp_seq_num: TCP sequential number
 * @tcp_ack_num: TCP acknowledgment number
 * @action: TCP action (SYN, ACK, FIN, etc)
 * @len: packet length
 */
typedef void rxhand_tcp(uchar *pkt, u16 dport,
			struct in_addr sip, u16 sport,
			u32 tcp_seq_num, u32 tcp_ack_num,
			u8 action, unsigned int len);
void tcp_set_tcp_handler(rxhand_tcp *f);

void rxhand_tcp_f(union tcp_build_pkt *b, unsigned int len);

u16 tcp_set_pseudo_header(uchar *pkt, struct in_addr src, struct in_addr dest,
			  int tcp_len, int pkt_len);
