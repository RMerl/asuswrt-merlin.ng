/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * SGI UV Broadcast Assist Unit definitions
 *
 * Copyright (C) 2008-2011 Silicon Graphics, Inc. All rights reserved.
 */

#ifndef _ASM_X86_UV_UV_BAU_H
#define _ASM_X86_UV_UV_BAU_H

#include <linux/bitmap.h>
#define BITSPERBYTE 8

/*
 * Broadcast Assist Unit messaging structures
 *
 * Selective Broadcast activations are induced by software action
 * specifying a particular 8-descriptor "set" via a 6-bit index written
 * to an MMR.
 * Thus there are 64 unique 512-byte sets of SB descriptors - one set for
 * each 6-bit index value. These descriptor sets are mapped in sequence
 * starting with set 0 located at the address specified in the
 * BAU_SB_DESCRIPTOR_BASE register, set 1 is located at BASE + 512,
 * set 2 is at BASE + 2*512, set 3 at BASE + 3*512, and so on.
 *
 * We will use one set for sending BAU messages from each of the
 * cpu's on the uvhub.
 *
 * TLB shootdown will use the first of the 8 descriptors of each set.
 * Each of the descriptors is 64 bytes in size (8*64 = 512 bytes in a set).
 */

#define MAX_CPUS_PER_UVHUB		128
#define MAX_CPUS_PER_SOCKET		64
#define ADP_SZ				64 /* hardware-provided max. */
#define UV_CPUS_PER_AS			32 /* hardware-provided max. */
#define ITEMS_PER_DESC			8
/* the 'throttle' to prevent the hardware stay-busy bug */
#define MAX_BAU_CONCURRENT		3
#define UV_ACT_STATUS_MASK		0x3
#define UV_ACT_STATUS_SIZE		2
#define UV_DISTRIBUTION_SIZE		256
#define UV_SW_ACK_NPENDING		8
#define UV1_NET_ENDPOINT_INTD		0x38
#define UV2_NET_ENDPOINT_INTD		0x28
#define UV_NET_ENDPOINT_INTD		(is_uv1_hub() ?			\
			UV1_NET_ENDPOINT_INTD : UV2_NET_ENDPOINT_INTD)
#define UV_DESC_PSHIFT			49
#define UV_PAYLOADQ_PNODE_SHIFT		49
#define UV_PTC_BASENAME			"sgi_uv/ptc_statistics"
#define UV_BAU_BASENAME			"sgi_uv/bau_tunables"
#define UV_BAU_TUNABLES_DIR		"sgi_uv"
#define UV_BAU_TUNABLES_FILE		"bau_tunables"
#define WHITESPACE			" \t\n"
#define uv_mmask			((1UL << uv_hub_info->m_val) - 1)
#define uv_physnodeaddr(x)		((__pa((unsigned long)(x)) & uv_mmask))
#define cpubit_isset(cpu, bau_local_cpumask) \
	test_bit((cpu), (bau_local_cpumask).bits)

/* [19:16] SOFT_ACK timeout period  19: 1 is urgency 7  17:16 1 is multiplier */
/*
 * UV2: Bit 19 selects between
 *  (0): 10 microsecond timebase and
 *  (1): 80 microseconds
 *  we're using 560us, similar to UV1: 65 units of 10us
 */
#define UV1_INTD_SOFT_ACK_TIMEOUT_PERIOD (9UL)
#define UV2_INTD_SOFT_ACK_TIMEOUT_PERIOD (15UL)

#define UV_INTD_SOFT_ACK_TIMEOUT_PERIOD	(is_uv1_hub() ?			\
		UV1_INTD_SOFT_ACK_TIMEOUT_PERIOD :			\
		UV2_INTD_SOFT_ACK_TIMEOUT_PERIOD)
/* assuming UV3 is the same */

#define BAU_MISC_CONTROL_MULT_MASK	3

#define UVH_AGING_PRESCALE_SEL		0x000000b000UL
/* [30:28] URGENCY_7  an index into a table of times */
#define BAU_URGENCY_7_SHIFT		28
#define BAU_URGENCY_7_MASK		7

#define UVH_TRANSACTION_TIMEOUT		0x000000b200UL
/* [45:40] BAU - BAU transaction timeout select - a multiplier */
#define BAU_TRANS_SHIFT			40
#define BAU_TRANS_MASK			0x3f

/*
 * shorten some awkward names
 */
#define AS_PUSH_SHIFT UVH_LB_BAU_SB_ACTIVATION_CONTROL_PUSH_SHFT
#define SOFTACK_MSHIFT UVH_LB_BAU_MISC_CONTROL_ENABLE_INTD_SOFT_ACK_MODE_SHFT
#define SOFTACK_PSHIFT UVH_LB_BAU_MISC_CONTROL_INTD_SOFT_ACK_TIMEOUT_PERIOD_SHFT
#define SOFTACK_TIMEOUT_PERIOD UV_INTD_SOFT_ACK_TIMEOUT_PERIOD
#define PREFETCH_HINT_SHFT UV3H_LB_BAU_MISC_CONTROL_ENABLE_INTD_PREFETCH_HINT_SHFT
#define SB_STATUS_SHFT UV3H_LB_BAU_MISC_CONTROL_ENABLE_EXTENDED_SB_STATUS_SHFT
#define write_gmmr	uv_write_global_mmr64
#define write_lmmr	uv_write_local_mmr
#define read_lmmr	uv_read_local_mmr
#define read_gmmr	uv_read_global_mmr64

/*
 * bits in UVH_LB_BAU_SB_ACTIVATION_STATUS_0/1
 */
#define DS_IDLE				0
#define DS_ACTIVE			1
#define DS_DESTINATION_TIMEOUT		2
#define DS_SOURCE_TIMEOUT		3
/*
 * bits put together from HRP_LB_BAU_SB_ACTIVATION_STATUS_0/1/2
 * values 1 and 3 will not occur
 *        Decoded meaning              ERROR  BUSY    AUX ERR
 * -------------------------------     ----   -----   -------
 * IDLE                                 0       0        0
 * BUSY (active)                        0       1        0
 * SW Ack Timeout (destination)         1       0        0
 * SW Ack INTD rejected (strong NACK)   1       0        1
 * Source Side Time Out Detected        1       1        0
 * Destination Side PUT Failed          1       1        1
 */
#define UV2H_DESC_IDLE			0
#define UV2H_DESC_BUSY			2
#define UV2H_DESC_DEST_TIMEOUT		4
#define UV2H_DESC_DEST_STRONG_NACK	5
#define UV2H_DESC_SOURCE_TIMEOUT	6
#define UV2H_DESC_DEST_PUT_ERR		7

/*
 * delay for 'plugged' timeout retries, in microseconds
 */
#define PLUGGED_DELAY			10

/*
 * threshholds at which to use IPI to free resources
 */
/* after this # consecutive 'plugged' timeouts, use IPI to release resources */
#define PLUGSB4RESET			100
/* after this many consecutive timeouts, use IPI to release resources */
#define TIMEOUTSB4RESET			1
/* at this number uses of IPI to release resources, giveup the request */
#define IPI_RESET_LIMIT			1
/* after this # consecutive successes, bump up the throttle if it was lowered */
#define COMPLETE_THRESHOLD		5
/* after this # of giveups (fall back to kernel IPI's) disable the use of
   the BAU for a period of time */
#define GIVEUP_LIMIT			100

#define UV_LB_SUBNODEID			0x10

/* these two are the same for UV1 and UV2: */
#define UV_SA_SHFT UVH_LB_BAU_MISC_CONTROL_INTD_SOFT_ACK_TIMEOUT_PERIOD_SHFT
#define UV_SA_MASK UVH_LB_BAU_MISC_CONTROL_INTD_SOFT_ACK_TIMEOUT_PERIOD_MASK
/* 4 bits of software ack period */
#define UV2_ACK_MASK			0x7UL
#define UV2_ACK_UNITS_SHFT		3
#define UV2_EXT_SHFT UV2H_LB_BAU_MISC_CONTROL_ENABLE_EXTENDED_SB_STATUS_SHFT

/*
 * number of entries in the destination side payload queue
 */
#define DEST_Q_SIZE			20
/*
 * number of destination side software ack resources
 */
#define DEST_NUM_RESOURCES		8
/*
 * completion statuses for sending a TLB flush message
 */
#define FLUSH_RETRY_PLUGGED		1
#define FLUSH_RETRY_TIMEOUT		2
#define FLUSH_GIVEUP			3
#define FLUSH_COMPLETE			4

/*
 * tuning the action when the numalink network is extremely delayed
 */
#define CONGESTED_RESPONSE_US		1000	/* 'long' response time, in
						   microseconds */
#define CONGESTED_REPS			10	/* long delays averaged over
						   this many broadcasts */
#define DISABLED_PERIOD			10	/* time for the bau to be
						   disabled, in seconds */
/* see msg_type: */
#define MSG_NOOP			0
#define MSG_REGULAR			1
#define MSG_RETRY			2

/*
 * Distribution: 32 bytes (256 bits) (bytes 0-0x1f of descriptor)
 * If the 'multilevel' flag in the header portion of the descriptor
 * has been set to 0, then endpoint multi-unicast mode is selected.
 * The distribution specification (32 bytes) is interpreted as a 256-bit
 * distribution vector. Adjacent bits correspond to consecutive even numbered
 * nodeIDs. The result of adding the index of a given bit to the 15-bit
 * 'base_dest_nasid' field of the header corresponds to the
 * destination nodeID associated with that specified bit.
 */
struct pnmask {
	unsigned long		bits[BITS_TO_LONGS(UV_DISTRIBUTION_SIZE)];
};

/*
 * mask of cpu's on a uvhub
 * (during initialization we need to check that unsigned long has
 *  enough bits for max. cpu's per uvhub)
 */
struct bau_local_cpumask {
	unsigned long		bits;
};

/*
 * Payload: 16 bytes (128 bits) (bytes 0x20-0x2f of descriptor)
 * only 12 bytes (96 bits) of the payload area are usable.
 * An additional 3 bytes (bits 27:4) of the header address are carried
 * to the next bytes of the destination payload queue.
 * And an additional 2 bytes of the header Suppl_A field are also
 * carried to the destination payload queue.
 * But the first byte of the Suppl_A becomes bits 127:120 (the 16th byte)
 * of the destination payload queue, which is written by the hardware
 * with the s/w ack resource bit vector.
 * [ effective message contents (16 bytes (128 bits) maximum), not counting
 *   the s/w ack bit vector  ]
 */

/*
 * The payload is software-defined for INTD transactions
 */
struct bau_msg_payload {
	unsigned long	address;		/* signifies a page or all
						   TLB's of the cpu */
	/* 64 bits */
	unsigned short	sending_cpu;		/* filled in by sender */
	/* 16 bits */
	unsigned short	acknowledge_count;	/* filled in by destination */
	/* 16 bits */
	unsigned int	reserved1:32;		/* not usable */
};


/*
 * UV1 Message header:  16 bytes (128 bits) (bytes 0x30-0x3f of descriptor)
 * see table 4.2.3.0.1 in broacast_assist spec.
 */
struct uv1_bau_msg_header {
	unsigned int	dest_subnodeid:6;	/* must be 0x10, for the LB */
	/* bits 5:0 */
	unsigned int	base_dest_nasid:15;	/* nasid of the first bit */
	/* bits 20:6 */				/* in uvhub map */
	unsigned int	command:8;		/* message type */
	/* bits 28:21 */
	/* 0x38: SN3net EndPoint Message */
	unsigned int	rsvd_1:3;		/* must be zero */
	/* bits 31:29 */
	/* int will align on 32 bits */
	unsigned int	rsvd_2:9;		/* must be zero */
	/* bits 40:32 */
	/* Suppl_A is 56-41 */
	unsigned int	sequence:16;		/* message sequence number */
	/* bits 56:41 */			/* becomes bytes 16-17 of msg */
						/* Address field (96:57) is
						   never used as an address
						   (these are address bits
						   42:3) */

	unsigned int	rsvd_3:1;		/* must be zero */
	/* bit 57 */
	/* address bits 27:4 are payload */
	/* these next 24  (58-81) bits become bytes 12-14 of msg */
	/* bits 65:58 land in byte 12 */
	unsigned int	replied_to:1;		/* sent as 0 by the source to
						   byte 12 */
	/* bit 58 */
	unsigned int	msg_type:3;		/* software type of the
						   message */
	/* bits 61:59 */
	unsigned int	canceled:1;		/* message canceled, resource
						   is to be freed*/
	/* bit 62 */
	unsigned int	payload_1a:1;		/* not currently used */
	/* bit 63 */
	unsigned int	payload_1b:2;		/* not currently used */
	/* bits 65:64 */

	/* bits 73:66 land in byte 13 */
	unsigned int	payload_1ca:6;		/* not currently used */
	/* bits 71:66 */
	unsigned int	payload_1c:2;		/* not currently used */
	/* bits 73:72 */

	/* bits 81:74 land in byte 14 */
	unsigned int	payload_1d:6;		/* not currently used */
	/* bits 79:74 */
	unsigned int	payload_1e:2;		/* not currently used */
	/* bits 81:80 */

	unsigned int	rsvd_4:7;		/* must be zero */
	/* bits 88:82 */
	unsigned int	swack_flag:1;		/* software acknowledge flag */
	/* bit 89 */
						/* INTD trasactions at
						   destination are to wait for
						   software acknowledge */
	unsigned int	rsvd_5:6;		/* must be zero */
	/* bits 95:90 */
	unsigned int	rsvd_6:5;		/* must be zero */
	/* bits 100:96 */
	unsigned int	int_both:1;		/* if 1, interrupt both sockets
						   on the uvhub */
	/* bit 101*/
	unsigned int	fairness:3;		/* usually zero */
	/* bits 104:102 */
	unsigned int	multilevel:1;		/* multi-level multicast
						   format */
	/* bit 105 */
	/* 0 for TLB: endpoint multi-unicast messages */
	unsigned int	chaining:1;		/* next descriptor is part of
						   this activation*/
	/* bit 106 */
	unsigned int	rsvd_7:21;		/* must be zero */
	/* bits 127:107 */
};

/*
 * UV2 Message header:  16 bytes (128 bits) (bytes 0x30-0x3f of descriptor)
 * see figure 9-2 of harp_sys.pdf
 * assuming UV3 is the same
 */
struct uv2_3_bau_msg_header {
	unsigned int	base_dest_nasid:15;	/* nasid of the first bit */
	/* bits 14:0 */				/* in uvhub map */
	unsigned int	dest_subnodeid:5;	/* must be 0x10, for the LB */
	/* bits 19:15 */
	unsigned int	rsvd_1:1;		/* must be zero */
	/* bit 20 */
	/* Address bits 59:21 */
	/* bits 25:2 of address (44:21) are payload */
	/* these next 24 bits become bytes 12-14 of msg */
	/* bits 28:21 land in byte 12 */
	unsigned int	replied_to:1;		/* sent as 0 by the source to
						   byte 12 */
	/* bit 21 */
	unsigned int	msg_type:3;		/* software type of the
						   message */
	/* bits 24:22 */
	unsigned int	canceled:1;		/* message canceled, resource
						   is to be freed*/
	/* bit 25 */
	unsigned int	payload_1:3;		/* not currently used */
	/* bits 28:26 */

	/* bits 36:29 land in byte 13 */
	unsigned int	payload_2a:3;		/* not currently used */
	unsigned int	payload_2b:5;		/* not currently used */
	/* bits 36:29 */

	/* bits 44:37 land in byte 14 */
	unsigned int	payload_3:8;		/* not currently used */
	/* bits 44:37 */

	unsigned int	rsvd_2:7;		/* reserved */
	/* bits 51:45 */
	unsigned int	swack_flag:1;		/* software acknowledge flag */
	/* bit 52 */
	unsigned int	rsvd_3a:3;		/* must be zero */
	unsigned int	rsvd_3b:8;		/* must be zero */
	unsigned int	rsvd_3c:8;		/* must be zero */
	unsigned int	rsvd_3d:3;		/* must be zero */
	/* bits 74:53 */
	unsigned int	fairness:3;		/* usually zero */
	/* bits 77:75 */

	unsigned int	sequence:16;		/* message sequence number */
	/* bits 93:78  Suppl_A  */
	unsigned int	chaining:1;		/* next descriptor is part of
						   this activation*/
	/* bit 94 */
	unsigned int	multilevel:1;		/* multi-level multicast
						   format */
	/* bit 95 */
	unsigned int	rsvd_4:24;		/* ordered / source node /
						   source subnode / aging
						   must be zero */
	/* bits 119:96 */
	unsigned int	command:8;		/* message type */
	/* bits 127:120 */
};

/*
 * The activation descriptor:
 * The format of the message to send, plus all accompanying control
 * Should be 64 bytes
 */
struct bau_desc {
	struct pnmask				distribution;
	/*
	 * message template, consisting of header and payload:
	 */
	union bau_msg_header {
		struct uv1_bau_msg_header	uv1_hdr;
		struct uv2_3_bau_msg_header	uv2_3_hdr;
	} header;

	struct bau_msg_payload			payload;
};
/* UV1:
 *   -payload--    ---------header------
 *   bytes 0-11    bits 41-56  bits 58-81
 *       A           B  (2)      C (3)
 *
 *            A/B/C are moved to:
 *       A            C          B
 *   bytes 0-11  bytes 12-14  bytes 16-17  (byte 15 filled in by hw as vector)
 *   ------------payload queue-----------
 */
/* UV2:
 *   -payload--    ---------header------
 *   bytes 0-11    bits 70-78  bits 21-44
 *       A           B  (2)      C (3)
 *
 *            A/B/C are moved to:
 *       A            C          B
 *   bytes 0-11  bytes 12-14  bytes 16-17  (byte 15 filled in by hw as vector)
 *   ------------payload queue-----------
 */

/*
 * The payload queue on the destination side is an array of these.
 * With BAU_MISC_CONTROL set for software acknowledge mode, the messages
 * are 32 bytes (2 micropackets) (256 bits) in length, but contain only 17
 * bytes of usable data, including the sw ack vector in byte 15 (bits 127:120)
 * (12 bytes come from bau_msg_payload, 3 from payload_1, 2 from
 *  swack_vec and payload_2)
 * "Enabling Software Acknowledgment mode (see Section 4.3.3 Software
 *  Acknowledge Processing) also selects 32 byte (17 bytes usable) payload
 *  operation."
 */
struct bau_pq_entry {
	unsigned long	address;	/* signifies a page or all TLB's
					   of the cpu */
	/* 64 bits, bytes 0-7 */
	unsigned short	sending_cpu;	/* cpu that sent the message */
	/* 16 bits, bytes 8-9 */
	unsigned short	acknowledge_count; /* filled in by destination */
	/* 16 bits, bytes 10-11 */
	/* these next 3 bytes come from bits 58-81 of the message header */
	unsigned short	replied_to:1;	/* sent as 0 by the source */
	unsigned short	msg_type:3;	/* software message type */
	unsigned short	canceled:1;	/* sent as 0 by the source */
	unsigned short	unused1:3;	/* not currently using */
	/* byte 12 */
	unsigned char	unused2a;	/* not currently using */
	/* byte 13 */
	unsigned char	unused2;	/* not currently using */
	/* byte 14 */
	unsigned char	swack_vec;	/* filled in by the hardware */
	/* byte 15 (bits 127:120) */
	unsigned short	sequence;	/* message sequence number */
	/* bytes 16-17 */
	unsigned char	unused4[2];	/* not currently using bytes 18-19 */
	/* bytes 18-19 */
	int		number_of_cpus;	/* filled in at destination */
	/* 32 bits, bytes 20-23 (aligned) */
	unsigned char	unused5[8];	/* not using */
	/* bytes 24-31 */
};

struct msg_desc {
	struct bau_pq_entry	*msg;
	int			msg_slot;
	struct bau_pq_entry	*queue_first;
	struct bau_pq_entry	*queue_last;
};

struct reset_args {
	int			sender;
};

/*
 * This structure is allocated per_cpu for UV TLB shootdown statistics.
 */
struct ptc_stats {
	/* sender statistics */
	unsigned long	s_giveup;		/* number of fall backs to
						   IPI-style flushes */
	unsigned long	s_requestor;		/* number of shootdown
						   requests */
	unsigned long	s_stimeout;		/* source side timeouts */
	unsigned long	s_dtimeout;		/* destination side timeouts */
	unsigned long	s_strongnacks;		/* number of strong nack's */
	unsigned long	s_time;			/* time spent in sending side */
	unsigned long	s_retriesok;		/* successful retries */
	unsigned long	s_ntargcpu;		/* total number of cpu's
						   targeted */
	unsigned long	s_ntargself;		/* times the sending cpu was
						   targeted */
	unsigned long	s_ntarglocals;		/* targets of cpus on the local
						   blade */
	unsigned long	s_ntargremotes;		/* targets of cpus on remote
						   blades */
	unsigned long	s_ntarglocaluvhub;	/* targets of the local hub */
	unsigned long	s_ntargremoteuvhub;	/* remotes hubs targeted */
	unsigned long	s_ntarguvhub;		/* total number of uvhubs
						   targeted */
	unsigned long	s_ntarguvhub16;		/* number of times target
						   hubs >= 16*/
	unsigned long	s_ntarguvhub8;		/* number of times target
						   hubs >= 8 */
	unsigned long	s_ntarguvhub4;		/* number of times target
						   hubs >= 4 */
	unsigned long	s_ntarguvhub2;		/* number of times target
						   hubs >= 2 */
	unsigned long	s_ntarguvhub1;		/* number of times target
						   hubs == 1 */
	unsigned long	s_resets_plug;		/* ipi-style resets from plug
						   state */
	unsigned long	s_resets_timeout;	/* ipi-style resets from
						   timeouts */
	unsigned long	s_busy;			/* status stayed busy past
						   s/w timer */
	unsigned long	s_throttles;		/* waits in throttle */
	unsigned long	s_retry_messages;	/* retry broadcasts */
	unsigned long	s_bau_reenabled;	/* for bau enable/disable */
	unsigned long	s_bau_disabled;		/* for bau enable/disable */
	unsigned long	s_uv2_wars;		/* uv2 workaround, perm. busy */
	unsigned long	s_uv2_wars_hw;		/* uv2 workaround, hiwater */
	unsigned long	s_uv2_war_waits;	/* uv2 workaround, long waits */
	unsigned long	s_overipilimit;		/* over the ipi reset limit */
	unsigned long	s_giveuplimit;		/* disables, over giveup limit*/
	unsigned long	s_enters;		/* entries to the driver */
	unsigned long	s_ipifordisabled;	/* fall back to IPI; disabled */
	unsigned long	s_plugged;		/* plugged by h/w bug*/
	unsigned long	s_congested;		/* giveup on long wait */
	/* destination statistics */
	unsigned long	d_alltlb;		/* times all tlb's on this
						   cpu were flushed */
	unsigned long	d_onetlb;		/* times just one tlb on this
						   cpu was flushed */
	unsigned long	d_multmsg;		/* interrupts with multiple
						   messages */
	unsigned long	d_nomsg;		/* interrupts with no message */
	unsigned long	d_time;			/* time spent on destination
						   side */
	unsigned long	d_requestee;		/* number of messages
						   processed */
	unsigned long	d_retries;		/* number of retry messages
						   processed */
	unsigned long	d_canceled;		/* number of messages canceled
						   by retries */
	unsigned long	d_nocanceled;		/* retries that found nothing
						   to cancel */
	unsigned long	d_resets;		/* number of ipi-style requests
						   processed */
	unsigned long	d_rcanceled;		/* number of messages canceled
						   by resets */
};

struct tunables {
	int			*tunp;
	int			deflt;
};

struct hub_and_pnode {
	short			uvhub;
	short			pnode;
};

struct socket_desc {
	short			num_cpus;
	short			cpu_number[MAX_CPUS_PER_SOCKET];
};

struct uvhub_desc {
	unsigned short		socket_mask;
	short			num_cpus;
	short			uvhub;
	short			pnode;
	struct socket_desc	socket[2];
};

/*
 * one per-cpu; to locate the software tables
 */
struct bau_control {
	struct bau_desc		*descriptor_base;
	struct bau_pq_entry	*queue_first;
	struct bau_pq_entry	*queue_last;
	struct bau_pq_entry	*bau_msg_head;
	struct bau_control	*uvhub_master;
	struct bau_control	*socket_master;
	struct ptc_stats	*statp;
	cpumask_t		*cpumask;
	unsigned long		timeout_interval;
	unsigned long		set_bau_on_time;
	atomic_t		active_descriptor_count;
	int			plugged_tries;
	int			timeout_tries;
	int			ipi_attempts;
	int			conseccompletes;
	short			nobau;
	short			baudisabled;
	short			cpu;
	short			osnode;
	short			uvhub_cpu;
	short			uvhub;
	short			uvhub_version;
	short			cpus_in_socket;
	short			cpus_in_uvhub;
	short			partition_base_pnode;
	short			busy;       /* all were busy (war) */
	unsigned short		message_number;
	unsigned short		uvhub_quiesce;
	short			socket_acknowledge_count[DEST_Q_SIZE];
	cycles_t		send_message;
	cycles_t		period_end;
	cycles_t		period_time;
	spinlock_t		uvhub_lock;
	spinlock_t		queue_lock;
	spinlock_t		disable_lock;
	/* tunables */
	int			max_concurr;
	int			max_concurr_const;
	int			plugged_delay;
	int			plugsb4reset;
	int			timeoutsb4reset;
	int			ipi_reset_limit;
	int			complete_threshold;
	int			cong_response_us;
	int			cong_reps;
	cycles_t		disabled_period;
	int			period_giveups;
	int			giveup_limit;
	long			period_requests;
	struct hub_and_pnode	*thp;
};

static inline void write_mmr_data_broadcast(int pnode, unsigned long mmr_image)
{
	write_gmmr(pnode, UVH_BAU_DATA_BROADCAST, mmr_image);
}

static inline void write_mmr_descriptor_base(int pnode, unsigned long mmr_image)
{
	write_gmmr(pnode, UVH_LB_BAU_SB_DESCRIPTOR_BASE, mmr_image);
}

static inline void write_mmr_activation(unsigned long index)
{
	write_lmmr(UVH_LB_BAU_SB_ACTIVATION_CONTROL, index);
}

static inline void write_gmmr_activation(int pnode, unsigned long mmr_image)
{
	write_gmmr(pnode, UVH_LB_BAU_SB_ACTIVATION_CONTROL, mmr_image);
}

static inline void write_mmr_payload_first(int pnode, unsigned long mmr_image)
{
	write_gmmr(pnode, UVH_LB_BAU_INTD_PAYLOAD_QUEUE_FIRST, mmr_image);
}

static inline void write_mmr_payload_tail(int pnode, unsigned long mmr_image)
{
	write_gmmr(pnode, UVH_LB_BAU_INTD_PAYLOAD_QUEUE_TAIL, mmr_image);
}

static inline void write_mmr_payload_last(int pnode, unsigned long mmr_image)
{
	write_gmmr(pnode, UVH_LB_BAU_INTD_PAYLOAD_QUEUE_LAST, mmr_image);
}

static inline void write_mmr_misc_control(int pnode, unsigned long mmr_image)
{
	write_gmmr(pnode, UVH_LB_BAU_MISC_CONTROL, mmr_image);
}

static inline unsigned long read_mmr_misc_control(int pnode)
{
	return read_gmmr(pnode, UVH_LB_BAU_MISC_CONTROL);
}

static inline void write_mmr_sw_ack(unsigned long mr)
{
	uv_write_local_mmr(UVH_LB_BAU_INTD_SOFTWARE_ACKNOWLEDGE_ALIAS, mr);
}

static inline void write_gmmr_sw_ack(int pnode, unsigned long mr)
{
	write_gmmr(pnode, UVH_LB_BAU_INTD_SOFTWARE_ACKNOWLEDGE_ALIAS, mr);
}

static inline unsigned long read_mmr_sw_ack(void)
{
	return read_lmmr(UVH_LB_BAU_INTD_SOFTWARE_ACKNOWLEDGE);
}

static inline unsigned long read_gmmr_sw_ack(int pnode)
{
	return read_gmmr(pnode, UVH_LB_BAU_INTD_SOFTWARE_ACKNOWLEDGE);
}

static inline void write_mmr_data_config(int pnode, unsigned long mr)
{
	uv_write_global_mmr64(pnode, UVH_BAU_DATA_CONFIG, mr);
}

static inline int bau_uvhub_isset(int uvhub, struct pnmask *dstp)
{
	return constant_test_bit(uvhub, &dstp->bits[0]);
}
static inline void bau_uvhub_set(int pnode, struct pnmask *dstp)
{
	__set_bit(pnode, &dstp->bits[0]);
}
static inline void bau_uvhubs_clear(struct pnmask *dstp,
				    int nbits)
{
	bitmap_zero(&dstp->bits[0], nbits);
}
static inline int bau_uvhub_weight(struct pnmask *dstp)
{
	return bitmap_weight((unsigned long *)&dstp->bits[0],
				UV_DISTRIBUTION_SIZE);
}

static inline void bau_cpubits_clear(struct bau_local_cpumask *dstp, int nbits)
{
	bitmap_zero(&dstp->bits, nbits);
}

extern void uv_bau_message_intr1(void);
#ifdef CONFIG_TRACING
#define trace_uv_bau_message_intr1 uv_bau_message_intr1
#endif
extern void uv_bau_timeout_intr1(void);

struct atomic_short {
	short counter;
};

/*
 * atomic_read_short - read a short atomic variable
 * @v: pointer of type atomic_short
 *
 * Atomically reads the value of @v.
 */
static inline int atomic_read_short(const struct atomic_short *v)
{
	return v->counter;
}

/*
 * atom_asr - add and return a short int
 * @i: short value to add
 * @v: pointer of type atomic_short
 *
 * Atomically adds @i to @v and returns @i + @v
 */
static inline int atom_asr(short i, struct atomic_short *v)
{
	short __i = i;
	asm volatile(LOCK_PREFIX "xaddw %0, %1"
			: "+r" (i), "+m" (v->counter)
			: : "memory");
	return i + __i;
}

/*
 * conditionally add 1 to *v, unless *v is >= u
 * return 0 if we cannot add 1 to *v because it is >= u
 * return 1 if we can add 1 to *v because it is < u
 * the add is atomic
 *
 * This is close to atomic_add_unless(), but this allows the 'u' value
 * to be lowered below the current 'v'.  atomic_add_unless can only stop
 * on equal.
 */
static inline int atomic_inc_unless_ge(spinlock_t *lock, atomic_t *v, int u)
{
	spin_lock(lock);
	if (atomic_read(v) >= u) {
		spin_unlock(lock);
		return 0;
	}
	atomic_inc(v);
	spin_unlock(lock);
	return 1;
}

#endif /* _ASM_X86_UV_UV_BAU_H */
