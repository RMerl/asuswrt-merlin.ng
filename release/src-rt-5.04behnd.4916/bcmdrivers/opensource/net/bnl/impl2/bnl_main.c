/*
	<:copyright-BRCM:2024:DUAL/GPL:standard

	Copyright (c) 2024 Broadcom
	All Rights Reserved

	Unless you and Broadcom execute a separate written software license
	agreement governing use of this software, this software is licensed
	to you under the terms of the GNU General Public License version 2
	(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
	with the following added to such license:

	As a special exception, the copyright holders of this software give
	you permission to link this software with independent modules, and
	to copy and distribute the resulting executable under terms of your
	choice, provided that you also meet, for each linked independent
	module, the terms and conditions of the license of that module.
	An independent module is a module which is not derived from this
	software.  The special exception does not apply to any modifications
	of the software.

	Not withstanding the above, under no circumstances may you combine
	this software in any way with any other Broadcom software provided
	under a license other than the GPL, without Broadcom's express prior
	written consent.

	:>
 */

/* Broadcom Network Latency Monitor */
#include <linux/types.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/bcm_log.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <net/tcp.h>
#include <linux/blog.h>
#include <linux/skbuff.h>
#include <linux/nbuff.h>
#include <linux/bcm_skb_defines.h>

#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <linux/bcm_nf_conntrack.h>

#include <linux/kthread.h>
#include <bcm_async_queue.h>

#include "bcm_score.h"
#include "bcm_bnl_ioctl.h"
#include "bnl.h"
#include "bcm_score_defs.h"
#include "bp3_license.h"

#define BNL_SW_DATA_QUEUE_SIZE (60*1024)

#define SCORE_BNL_QUEUE_SIZE 1024

#define MAX_TCP_TS_TRACK 4
#define BCM_MAX_BNL_NODES SCORE_NUM_OF_STREAM_IDX

#define IS_BIT_ON(bitmap, bit) ((bitmap) & (1 << (bit)))
#define SET_BIT(bitmap, bit) ((bitmap) |= (1 << (bit)))
#define CLR_BIT(bitmap, bit) ((bitmap) &= ~(1 << (bit)))

extern int score_hw_bnl_ring_create(uint32_t ring_size);
extern int score_hw_bnl_ring_destroy(uint32_t ring_size);
extern int score_hw_bnl_ring_info_get(score_bnl_data_t *bnl_pkt_info);

extern int bnl_cdev_init(void);
extern void bnl_cdev_uninit(void);

struct tcp_ts_option {
	u32	rcv_tsval;	/* Time stamp value	*/
	u32	rcv_tsecr;	/* Time stamp echo reply */
	ktime_t time;	/* Time we stored this entry */
	bool pure_ack;	/* is tsval from pure_ack */
	bool rtt_done;
};

struct quic_ts_option {
	bool spin_bit;
	bool prev_spin_bit;
	ktime_t time;	/* Time we stored this entry */
	bool rtt_done;
};

#define BNL_DEFAULT_NOREPLY_TIMEOUT_MS 1000


unsigned int default_bin_cfg_limits[BNL_MAX_BIN_LIMITS] = {10, 20, 40, 80, 160, 320, 640};

struct bnl_bin_cfg {
	uint32_t limits[BNL_MAX_BIN_LIMITS];
};

struct bnl_bin {
	uint32_t limits[BNL_MAX_BIN_LIMITS];
	uint32_t data[BNL_MAX_BINS];
};

struct bnl_ts_state {
	union {
		struct tcp_ts_option ts_opts[MAX_TCP_TS_TRACK];
		struct quic_ts_option quic_opt;
	};
	u32 last_seen_tsval;
	u32 rtt_min;
	u32 rtt_max;
	u64 rtt_sum;
	u64 rtt_square_sum;
	u32 total_cnt;
	u32 timeout_cnt;
	u32 no_slot_cnt;
	u32 skip_cnt;
	struct bnl_bin bin;
	int last_e2e_delay;
	int last_e2gw_delay;
	u8 in_use_cnt;
};

struct bnl_ts_node {
	struct bnl_ts_state  orig_st;
	struct bnl_ts_state reply_st;
	struct nf_conn *ct;
	u32 bnl_groupid; /* provided by application */
	u32 bnl_id;      /* unique id for this connection */
    blog_score_key_t score_key;
	bool inuse;
};

/*return timestamp in 100usec resolution*/
inline uint64_t bnl_sw_time_get(void)
{
	/*TODO use more efficient methong than ktime_get() */
	uint64_t tstamp = ktime_get();
	do_div(tstamp, 100000);/* adjist to 100 usec res */
	return tstamp;
}

inline uint64_t bnl_hw_time_to_bnl_time(uint64_t hw_tstamp)
{
#if defined(CONFIG_BCM96765) || defined(CONFIG_BCM96766)
	/* on these platforms resolution is already at 100 usec */
	return hw_tstamp; 
#else
	do_div(hw_tstamp, 100);/* adjust to 100 usec res */
	return hw_tstamp;
#endif
}
inline uint64_t bnl_time_to_msec(uint64_t bnl_time)
{
	do_div(bnl_time, 10);
	return bnl_time;
}
#define msec_to_bnl_time(msec) (msec * 10)
#define bnl_time_delta(later, earlier)  ((later) - (earlier))

#define BNL_LOCK()		spin_lock_bh(&bcm_bnl_ts_g.ts_tbl_lock)
#define BNL_UNLOCK()	spin_unlock_bh(&bcm_bnl_ts_g.ts_tbl_lock)

/* data of previously tracked connections*/
#define BCM_MAX_BNL_RECORDS 512
struct ts_record_info {
	struct nf_conntrack_tuple tuple;
	u32 rtt_min;
	u32 rtt_max;
	u64 rtt_sum;
	u64 rtt_square_sum;
	u32 total_cnt;
	u32 timeout_cnt;
	u32 no_slot_cnt;
	u32 skip_cnt;
	struct bnl_bin bin;
};

struct bnl_ts_record {
	/* TODO optimize this structure */
	struct ts_record_info  orig_st;
	struct ts_record_info  reply_st;
	u32 bnl_groupid; /* provided by application */
	u32 bnl_id;      /* unique id for this connection */
    blog_score_key_t score_key;
	bool inuse;
};

typedef struct {
	struct bnl_ts_record ts_records[BCM_MAX_BNL_RECORDS];
	spinlock_t tbl_lock;
	u16 cnt;
	u16 cur_idx;
} bcm_bnl_ts_record_t;

#if 0
#define BNL_RECORD_LOCK()		spin_lock_bh(&bcm_bnl_ts_record_g.tbl_lock)
#define BNL_RECORD_UNLOCK()		spin_unlock_bh(&bcm_tcp_ts_record_g.tbl_lock)
#else
#define BNL_RECORD_LOCK()
#define BNL_RECORD_UNLOCK()
#endif

struct profile_proc_iter_state {
	unsigned int index;
    void *profile;
};

typedef struct bnl_profile {
    struct bnl_profile *next;
	struct bnl_ts_node ts_nodes[BCM_MAX_BNL_NODES];
	struct bnl_bin_cfg bins_cfg[BNL_MAX_BIN_CFGS];
	u32 cumm_alloc;
	u32 cumm_free;
	u32 cumm_alloc_failures;
	u32 notsval_ct_skip; 
	u32 notsval_pkt_skip;
	u32 udp_no_quic_ct_skip; 
	u32 bnl_id_cntr; /*montonous increasing number to identify unique connections*/
	u16 ts_node_cnt;
	u16 ts_node_cur_idx;
	int activelist_clearonread;
	int donelist_clearonread;
	int activelist_print_only_nonzero;
	int print_fmt_singleline;
	int noreply_timeout_ms;
	int ignore_pure_ack_for_rtt;
    uint8_t id;
	bcm_bnl_ts_record_t donelist;

	/* proc entires */
	struct proc_dir_entry *proc_activelist;
	struct proc_dir_entry *proc_donelist;
	struct proc_dir_entry *proc_status;
	struct profile_proc_iter_state proc_iter_state;
	struct ctl_table_header *sysctl_header;
	struct ctl_table *sysctl_tbl;
} bnl_profile_t;

struct bnl_stream_state {
    uint8_t profile_bmap;
};

struct {
	spinlock_t ts_tbl_lock;
    struct task_struct     *event_thread;
	unsigned long event_thread_intvl; /* periodic refresh interval*/
    bcm_async_queue_t evt_queue;
	int debug_lvl;
	uint8_t profile_cnt;
    bnl_profile_t *profiles[BNL_MAX_PROFILES];
	struct bnl_stream_state streams[BCM_MAX_BNL_NODES];
} bcm_bnl_ts_g;
static inline void bnl_record_fill_from_st(struct ts_record_info  *record_info,
	  struct bnl_ts_state *ts_st)
{
	record_info->rtt_min = ts_st->rtt_min;
	record_info->rtt_max = ts_st->rtt_max;
	record_info->rtt_sum = ts_st->rtt_sum;
	record_info->rtt_square_sum = ts_st->rtt_square_sum;
	record_info->total_cnt = ts_st->total_cnt;
	record_info->timeout_cnt = ts_st->timeout_cnt;
	record_info->no_slot_cnt = ts_st->no_slot_cnt;
	record_info->skip_cnt = ts_st->skip_cnt;
	memcpy(&record_info->bin, &ts_st->bin, sizeof(record_info->bin));
}

u16 bcm_bnl_ts_record_store(bnl_profile_t *profile, struct bnl_ts_node *ts_node)
{
	struct bnl_ts_record *ts_record;

	BNL_RECORD_LOCK();

	ts_record =  &profile->donelist.ts_records[profile->donelist.cur_idx];
	profile->donelist.cur_idx++;
	if (profile->donelist.cur_idx >= BCM_MAX_BNL_RECORDS) {
		profile->donelist.cur_idx = 0;
	}

	ts_record->inuse = true;
	ts_record->bnl_groupid = ts_node->bnl_groupid;
	ts_record->bnl_id = ts_node->bnl_id;
	ts_record->score_key.key = ts_node->score_key.key;
	/*copy tuple info */
	memcpy(&ts_record->orig_st.tuple, nf_ct_tuple(ts_node->ct, IP_CT_DIR_ORIGINAL),
			sizeof(ts_record->orig_st.tuple));

	memcpy(&ts_record->reply_st.tuple, nf_ct_tuple(ts_node->ct, IP_CT_DIR_REPLY),
			sizeof(ts_record->reply_st.tuple));

	/*copy state info */
	bnl_record_fill_from_st(&ts_record->orig_st, &ts_node->orig_st);
	bnl_record_fill_from_st(&ts_record->reply_st, &ts_node->reply_st);

	BNL_RECORD_UNLOCK();
	return 0;
}

static inline struct bnl_ts_node *bnl_node_alloc_by_key(bnl_profile_t *profile, blog_score_key_t key)
{
    struct bnl_ts_node *ts_node;

    if((key.idx <=0) || (key.idx >= BCM_MAX_BNL_NODES)) {
        __logError("inavlid idx=%d \n", key.idx);
        return NULL;
    }

    ts_node = &profile->ts_nodes[key.idx];
    if(ts_node->inuse == false) {
        memset(ts_node, 0, sizeof(struct bnl_ts_node));
        ts_node->inuse = true;
        ts_node->bnl_id = profile->bnl_id_cntr++;
        profile->cumm_alloc++;
        profile->ts_node_cnt++;
    } else {
        __logError("idx=%d already in use\n", key.idx);
        ts_node = NULL;
        profile->cumm_alloc_failures++;
    }
    return ts_node;
}

static inline struct bnl_ts_node *bnl_node_get_by_idx(bnl_profile_t *profile, u16 idx)
{
	if((idx <=0) || (idx >= BCM_MAX_BNL_NODES)) {
		__logError("inavlid idx=%d \n", idx);
		return NULL;
	}

	return &profile->ts_nodes[idx];
}

static inline struct bnl_ts_node *bnl_node_get_by_key(bnl_profile_t *profile, blog_score_key_t key)
{
	struct bnl_ts_node *ts_node;

	ts_node = bnl_node_get_by_idx(profile, key.idx);
	if (!ts_node)
		return NULL;

    if(unlikely(key.incarn != ts_node->score_key.incarn)) {

        __logDebug("idx=%u key_incarn=%u node_incarn=%u\n", key.idx,
                key.incarn, ts_node->score_key.incarn);
        ts_node = NULL;
    }

	return ts_node;
}

static inline void bnl_node_free(bnl_profile_t *profile, struct bnl_ts_node *ts_node)
{
	ts_node->inuse = false;
	ts_node->ct = NULL;

	profile->ts_node_cnt--;
	profile->cumm_free++;
}

static inline void bcm_bnl_node_free_by_idx(bnl_profile_t *profile, u16 idx)
{
	struct bnl_ts_node *ts_node;

	ts_node = bnl_node_get_by_idx(profile, idx);

	if(ts_node && ts_node->inuse)
		bnl_node_free(profile, ts_node);
	else
		__logError("Double free idx=%d \n", idx);
}

static inline bool bcm_tcp_get_opt_ts(const struct tcphdr *th,  struct tcp_ts_option *ts_opt)
{
	const unsigned char *ptr;
	int length = (th->doff * 4) - sizeof(struct tcphdr);

	ptr = (const unsigned char *)(th + 1);

	while (length > 0) {
		int opcode = *ptr++;
		int opsize;

		switch (opcode) {
			case TCPOPT_EOL:
				return false;
			case TCPOPT_NOP:	/* Ref: RFC 793 section 3.1 */
				length--;
				continue;
			default:
				opsize = *ptr++;
				if (opsize < 2) /* "silly options" */
					return false;
				if (opsize > length)
					return false;	/* don't parse partial options */

				if ((opcode == TCPOPT_TIMESTAMP) && (opsize == TCPOLEN_TIMESTAMP)) {
					ts_opt->rcv_tsval = get_unaligned_be32(ptr);
					ts_opt->rcv_tsecr = get_unaligned_be32(ptr + 4);
					return true;
				}
				ptr += opsize-2;
				length -= opsize;
		}
	}
	return false;
}

static inline bool bcm_tcp_get_opt_ts_aligned(const struct tcphdr *th, struct tcp_ts_option *ts_opt)
{
	const __be32 *ptr = (const __be32 *)(th + 1);

	if (*ptr == htonl((TCPOPT_NOP << 24) | (TCPOPT_NOP << 16)
			  | (TCPOPT_TIMESTAMP << 8) | TCPOLEN_TIMESTAMP)) {
		++ptr;
		ts_opt->rcv_tsval = ntohl(*ptr);
		++ptr;
		ts_opt->rcv_tsecr = ntohl(*ptr);
		return true;
	}
	return false;
}

static bool bcm_tcp_parse_option_ts(const struct tcphdr *th, struct tcp_ts_option *ts_opt)
{
	if (th->doff == (sizeof(*th) / 4)) {
		return false;
	} else if (th->doff == ((sizeof(*th) + TCPOLEN_TSTAMP_ALIGNED) / 4)) {
		if (bcm_tcp_get_opt_ts_aligned(th, ts_opt))
			return true;
	}
	return bcm_tcp_get_opt_ts(th, ts_opt);
}

void bnl_bin_update(struct bnl_ts_state *ts_st, u32 rtt)
{
	int idx;
	struct bnl_bin *bin = &ts_st->bin;

	/*find bin*/
	for(idx=0; idx<BNL_MAX_BIN_LIMITS; idx++) {
		if(rtt < bin->limits[idx])
			break;
	}
	/*here idx, will be either a match are a catch all(last bin) */
	bin->data[idx]++;
}

void bnl_bin_init(bnl_profile_t *profile, struct bnl_ts_state *ts_st, u8 bincfg_idx)
{
	if(bincfg_idx > BNL_MAX_BIN_CFGS) {
		__logError("Bad bincfg_idx =%d \n", bincfg_idx);
		bincfg_idx = 0; /*just use cfg 0 */
	}
	/*attach bin config limits, copy per flow is maintained, just in case they are changed after flow in learned*/
	memcpy(ts_st->bin.limits, &profile->bins_cfg[bincfg_idx], sizeof(ts_st->bin.limits));
	/*clear bin data */
	memset(ts_st->bin.data, 0, sizeof(ts_st->bin.data));
}

int tcp_process_tsecr(bnl_profile_t *profile, struct bnl_ts_node *ts_node, u8 dir, struct tcp_ts_option *pkt_ts_opt)
{
	u32 rtt; 
	struct bnl_ts_state *tsecr_st;
	struct tcp_ts_option *ts_opt;
	int i;

	tsecr_st = dir ? &ts_node->reply_st : &ts_node->orig_st;

    /* check for tsecr match */
	for(i=0; i<MAX_TCP_TS_TRACK; i++) {
		ts_opt = &tsecr_st->ts_opts[i];
		if (!ts_opt->rtt_done && (ts_opt->rcv_tsval == pkt_ts_opt->rcv_tsecr)) {

			rtt = (u32)bnl_time_delta(pkt_ts_opt->time, ts_opt->time);

			if (unlikely(rtt > msec_to_bnl_time(profile->noreply_timeout_ms))) {
				tsecr_st->timeout_cnt++;
				goto skip;
			}

			if (!tsecr_st->rtt_min || (rtt < tsecr_st->rtt_min))
				tsecr_st->rtt_min = rtt;

			if (rtt > tsecr_st->rtt_max)
				tsecr_st->rtt_max = rtt;

			/* average */
			tsecr_st->rtt_sum += rtt;
			tsecr_st->rtt_square_sum += ((uint64_t)rtt * rtt);
			/*TODO handle overflow for rtt_square */
			tsecr_st->total_cnt++;

			/* update bin */
			bnl_bin_update(tsecr_st, rtt);

skip:
			ts_opt->rtt_done = true;;
			break;
		}
	}
	return 0;
}

static inline void tcp_ts_opt_expiry(bnl_profile_t *profile, struct bnl_ts_state *tsval_st, struct tcp_ts_option *pkt_ts_opt)
{
	int i;
	struct tcp_ts_option *ts_opt;

	for (i=0; i<MAX_TCP_TS_TRACK; i++) {
		ts_opt = &tsval_st->ts_opts[i];

		if (!ts_opt->rtt_done && 
				(bnl_time_delta(pkt_ts_opt->time, ts_opt->time) < msec_to_bnl_time(profile->noreply_timeout_ms))) 
			continue;

		/* currently this fucntion is called only when entries are full
         * we can assume tsval is non zero and entry is valid
         */
		if (!ts_opt->rtt_done)
			tsval_st->timeout_cnt++;

		/*clear record */
		memset(ts_opt, 0, sizeof(struct tcp_ts_option));
		tsval_st->in_use_cnt--;
	}
	return;
}

int tcp_store_ts(bnl_profile_t *profile, struct bnl_ts_node *ts_node, u8 dir, struct tcp_ts_option *pkt_ts_opt)
{
	struct bnl_ts_state *tsval_st;
	struct tcp_ts_option *ts_opt;
	int i;

	/* select state in opposite direction */
	tsval_st = dir ? &ts_node->orig_st : &ts_node->reply_st;

	/* check if new tsval > last seen, also handle rollover using some
	 * big diff
	 */
	if((pkt_ts_opt->rcv_tsval <= tsval_st->last_seen_tsval)
			&& ((tsval_st->last_seen_tsval-pkt_ts_opt->rcv_tsval)<(1UL<<31)))
		goto done;

	tsval_st->last_seen_tsval = pkt_ts_opt->rcv_tsval;

	/* pure_ack filter */
	if (profile->ignore_pure_ack_for_rtt && pkt_ts_opt->pure_ack)
		goto done;

	/* expire only when full */
	if (tsval_st->in_use_cnt >= MAX_TCP_TS_TRACK)
		tcp_ts_opt_expiry(profile, tsval_st, pkt_ts_opt);

	for (i=0; i<MAX_TCP_TS_TRACK; i++) {
		ts_opt = &tsval_st->ts_opts[i];
		if (ts_opt->rcv_tsval == 0) {
			ts_opt->rcv_tsval = pkt_ts_opt->rcv_tsval;
			ts_opt->rcv_tsecr = pkt_ts_opt->rcv_tsecr; /*not used for now */
			ts_opt->time = pkt_ts_opt->time;
			ts_opt->pure_ack = pkt_ts_opt->pure_ack;
			tsval_st->in_use_cnt++;
			break;
		}
	}
	if (i >=MAX_TCP_TS_TRACK)
		tsval_st->no_slot_cnt++;
done:
	return 0;
}


static inline bool is_valid_quic_short_header(unsigned char *hdr)
{
	/* The most significant bit (0x80) of byte 0 (the first octet) is set 
	 * to 0 for short headers
	 */
	if (*hdr & 0x80)
		return false;
	else
		return true;
}

static inline bool is_valid_quic_long_header(unsigned char *hdr)
{
	/* The most significant bit (0x80) of byte 0 (the first octet) is set 
	 * to 1 for long headers
	 */
	if (*hdr & 0x80)
		return true;
	else
		return false;
}

static inline bool is_valid_quic_header(struct udphdr *uh)
{
	//unsigned char *quichdr = (unsigned char *) (uh+1);
	/* there is no definite way to identify this  unless we see 
      first few packets with long header*/
	return true;
}


static inline bool bnl_udp_parse_quic_spinbit(unsigned char *hdr)
{
	/* the third most significant bit (0x20) of byte 0 (ithe first octet)
	 * is the latency spin bit
	 */
	return (*hdr & 0x20) ? true : false;
}

void bnl_ts_state_update_stats(struct bnl_ts_state *ts_st, u32 rtt)
{
	if (!ts_st->rtt_min || (rtt < ts_st->rtt_min))
		ts_st->rtt_min = rtt;

	if (rtt > ts_st->rtt_max)
		ts_st->rtt_max = rtt;

	/* average */
	ts_st->rtt_sum += rtt;
	ts_st->rtt_square_sum += ((uint64_t)rtt * rtt);
	/*TODO handle overflow for rtt_square */
	ts_st->total_cnt++;
	/* update bin */
	bnl_bin_update(ts_st, rtt);
}

int bnl_quic_process_spinbit(bnl_profile_t *profile, struct bnl_ts_node *ts_node, u8 pkt_dir, struct quic_ts_option *pkt_ts_opt)
{
	u32 rtt; 
	struct bnl_ts_state *ts_st;
	struct quic_ts_option *quic_opt;


	ts_st = pkt_dir ? &ts_node->reply_st : &ts_node->orig_st;

	quic_opt = &ts_st->quic_opt;

	/* client always inverts the spinbit, server just refelcts teh spin bit
	 * from client. here client is originator of connection and it should be
	 * same as CT->orig_dir most of time (unless CT picks up and already
	 * connection. 
	 * for  rtt calculations
	 * client<->GW : match is pkt_spinbit != flow_spinbit
	 * server<->GW : match is pkt_spinbit == flow_spinbit
	 */

	if(!quic_opt->rtt_done &&
			( (pkt_dir && (quic_opt->spin_bit == pkt_ts_opt->spin_bit))
			  ||(!pkt_dir && (quic_opt->spin_bit != pkt_ts_opt->spin_bit)))) {

		rtt = (u32)bnl_time_delta(pkt_ts_opt->time, quic_opt->time);

		if (unlikely(rtt > msec_to_bnl_time(profile->noreply_timeout_ms))) {
			ts_st->timeout_cnt++;
			quic_opt->rtt_done = true;
			goto skip;
		}
		bnl_ts_state_update_stats(ts_st, rtt);
		quic_opt->rtt_done = true;
	}

	/* store in spin bit in other direction */
	ts_st = pkt_dir ? &ts_node->orig_st : &ts_node->reply_st;
	quic_opt = &ts_st->quic_opt;

	if (!quic_opt->rtt_done &&
			(bnl_time_delta(pkt_ts_opt->time, quic_opt->time) >= msec_to_bnl_time(profile->noreply_timeout_ms))) {
		quic_opt->rtt_done = true;
		ts_st->timeout_cnt++;
	}

	/* TODO here when timeout happens should we store current spin bit,
	   but skip it for next rtt computation */

	if (quic_opt->rtt_done && (quic_opt->spin_bit != pkt_ts_opt->spin_bit)) {
		quic_opt->spin_bit = pkt_ts_opt->spin_bit;
		quic_opt->time = pkt_ts_opt->time;
		quic_opt->rtt_done = false;
	}

skip:
	return 0;
}


#if 1
int tcp_process_oneway(bnl_profile_t *profile, struct bnl_ts_node *ts_node, u8 dir,
		struct tcp_ts_option *pkt_ts_opt)
{
    //struct bnl_ts_state *orig_st;
    //struct bnl_ts_state *reply_st;
    int e2e_delay, e2gw_delay;
    int e2e_lv, e2gw_lv;
    int gw_ts_msec;
	struct bnl_ts_state *tsval_st;

    /* select state in opposite direction */
    tsval_st = dir ? &ts_node->orig_st : &ts_node->reply_st;


    e2e_delay = pkt_ts_opt->rcv_tsval - pkt_ts_opt->rcv_tsecr;/* opp dir to pkt */
    e2e_lv = tsval_st->last_e2e_delay - e2e_delay;

    tsval_st->last_e2e_delay = e2e_delay;

    /*TODO update bins */


    /* select state in same direction as pkt */
    tsval_st = dir ? &ts_node->reply_st : &ts_node->orig_st;
    gw_ts_msec = pkt_ts_opt->time;
	
    //do_div(gw_ts_msec,100);/* 100 usec resolution */

    e2gw_delay = gw_ts_msec - pkt_ts_opt->rcv_tsval;
    e2gw_lv = tsval_st->last_e2gw_delay - e2gw_delay;
    /*TODO update bins */
    //us_pkt_itr_time = pkt_ts_opt->rcv_tsval - tsval_st->last_seen_tsval; /* udated every time, not sure if this is use ful*/   

    return 0;
}
#endif

static inline bool is_tcp_pure_ack_pkt(const Blog_t *blog, const FkBuff_t *fkb)  
{
    /* TODO parse packet instead of relying on blog(works only when ack prio is eanbled) */
    if (blog->key.tcp_pure_ack)
        return true;
    else
        return false;
}

int bcm_bnltrace_pkt_rcv(void *param1, void *param2, void *param3)
{
    struct tcp_ts_option ts_opt; 
    int ret=0;
    score_bnl_data_t *pkt_info;
    bcm_async_queue_t *sw_queue = &bcm_bnl_ts_g.evt_queue;
        const Blog_t *blog = param1;
    const FkBuff_t *fkb = param2;

    if(bcm_async_queue_not_full(sw_queue)) {
        /*point to the record mem in queue */
        pkt_info = (score_bnl_data_t *) bcm_async_queue_entry_write(sw_queue);
    } else {
        sw_queue->stats.discards++;
        goto exit;
    }

    pkt_info->timestamp = bnl_sw_time_get();


    pkt_info->score_stream_incarn = blog->score_info.score_key.incarn;
    pkt_info->score_stream_dir = blog->score_info.score_key.dir;
    pkt_info->score_stream_idx = blog->score_info.score_key.idx;

    if (blog->key.protocol == BLOG_IPPROTO_TCP) {

        struct tcphdr *th = (struct tcphdr *)(fkb->data + blog->l4_offset);
        pkt_info->protocol_is_tcp = 1;
        /*extract timestamps from pkt */
        if (unlikely(bcm_tcp_parse_option_ts(th, &ts_opt) == false)) {
            /* no tcp timestamp */
			/*TODO handle this cntr */
            //profile->notsval_pkt_skip++;
            ret = -1;
            goto exit;
        }

        /* check for pure ack */
        pkt_info->tcp_pure_ack = is_tcp_pure_ack_pkt(blog, fkb);
        pkt_info->tcp_tsval = ts_opt.rcv_tsval;
        pkt_info->tcp_tsecr = ts_opt.rcv_tsecr;
        pkt_info->tcp_seq = 0;
        pkt_info->tcp_ack = 0;

    } else if (blog->key.protocol == BLOG_IPPROTO_UDP) {

        struct udphdr *uh = (struct udphdr *)(fkb->data + blog->l4_offset);
        unsigned char *quichdr = (unsigned char *) (uh+1);

        /* check if packet has QUIC short header & spin bit */
        if (is_valid_quic_short_header(quichdr) == false)
            goto exit;

        pkt_info->protocol_is_tcp = 0;
        pkt_info->udp_spin_bit = bnl_udp_parse_quic_spinbit(quichdr);

    } else {
        /* unsupported protocol */
        __logNotice("unsupported protocol %u \n", blog->key.protocol);
        goto exit;
    }

    bcm_async_queue_entry_enqueue(sw_queue);
    sw_queue->stats.writes++;

    /*TODO add time out or cnt to wake up */
    /* wake up reader */
    if ((bcm_bnl_ts_g.event_thread->__state != TASK_RUNNING))
        wake_up_process(bcm_bnl_ts_g.event_thread);
    return 0;
exit:
    return ret;
}

int bnl_process_pkt_info(score_bnl_data_t *pkt_info)
{
	struct tcp_ts_option ts_opt; 
	struct quic_ts_option quic_opt; 
	struct bnl_ts_node *ts_node; 
	blog_score_key_t score_key;
	uint8_t profile_bmap;
	uint8_t idx;
	int ret=0;
	bnl_profile_t *profile;

	score_key.key = 0;
	score_key.idx = pkt_info->score_stream_idx;
	score_key.dir = pkt_info->score_stream_dir;
	score_key.incarn = pkt_info->score_stream_incarn;


	if (pkt_info->protocol_is_tcp) {
		ts_opt.time = pkt_info->timestamp;
		ts_opt.pure_ack = pkt_info->tcp_pure_ack;
		ts_opt.rcv_tsval = pkt_info->tcp_tsval;
		ts_opt.rcv_tsecr = pkt_info->tcp_tsecr;
	} else {
		quic_opt.time = pkt_info->timestamp;
		quic_opt.spin_bit = pkt_info->udp_spin_bit;
	}

	BNL_LOCK();
	profile_bmap = bcm_bnl_ts_g.streams[score_key.idx].profile_bmap;
	idx = 0;
	while (profile_bmap) { 

		if (profile_bmap &1) {
			profile = bcm_bnl_ts_g.profiles[idx];
		} else {
			goto next;
		}
		/*check for tsecr match */
		ts_node = bnl_node_get_by_key(profile, score_key);
		if(!ts_node) {
			__logError("invalid bnl_traceid =%hu \n", score_key.idx);
			ret = -1;
			goto unlock_exit;
		}

		if (pkt_info->protocol_is_tcp) {
			tcp_process_tsecr(profile, ts_node, score_key.dir, &ts_opt);
			tcp_process_oneway(profile, ts_node, score_key.dir, &ts_opt);
			/*store new tsval */
			tcp_store_ts(profile, ts_node, score_key.dir,  &ts_opt);
		} else {
			bnl_quic_process_spinbit(profile, ts_node, score_key.dir,  &quic_opt);
		}
next:
		profile_bmap >>= 1;
		idx++;
	}
unlock_exit:
	BNL_UNLOCK();
	return ret;
}

void bcm_bnl_send_record(struct bnl_ts_node *ts_node)
{
	/* senq unique id along with rtt from src->CPE and CPE->dst  min, max, avg rtt along with
	 * num records and evnt type(end or reached aggregate limit)*/ 
}

static inline void bnl_rtt_stats_fill_from_st(bnl_stream_rtt_stats_t *rtt_stats,
	  struct bnl_ts_state *ts_st)
{
        rtt_stats->min = ts_st->rtt_min;
        rtt_stats->max = ts_st->rtt_max;
        rtt_stats->sum = ts_st->rtt_sum;
        rtt_stats->square_sum = ts_st->rtt_square_sum;
        rtt_stats->count = ts_st->total_cnt;
        rtt_stats->timeout_cnt = ts_st->timeout_cnt;
        rtt_stats->skip_cnt = ts_st->skip_cnt;
        rtt_stats->no_slot_cnt = ts_st->no_slot_cnt;

	memcpy(&rtt_stats->bin_data, ts_st->bin.data, sizeof(rtt_stats->bin_data));
}

static inline void bnl_ts_clear_state(struct bnl_ts_state *ts_st)
{
	ts_st->rtt_max = 0;
	ts_st->rtt_min = 0;
	ts_st->rtt_sum = 0;
	ts_st->rtt_square_sum = 0;
	ts_st->total_cnt = 0;
	ts_st->timeout_cnt = 0;
	ts_st->no_slot_cnt = 0;
	ts_st->skip_cnt = 0;
	memset(ts_st->bin.data, 0, sizeof(ts_st->bin.data));
}

int bcm_bnl_get_stats_by_streamid(bnlctl_stream_stats_t *stats)
{
    struct bnl_ts_node *ts_node;
    blog_score_key_t score_key;
    uint8_t dcp_id;
    bnl_profile_t *profile = NULL;
    int ret=SCORE_ERR_OK;

    score_key.key = stats->stream_id;
    dcp_id = stats->profile_id;

    BNL_LOCK();
    /* check if profile exists */
    if(dcp_id < BNL_MAX_PROFILES)
        profile = bcm_bnl_ts_g.profiles[dcp_id];

    if (!profile) {
        __logError("invalid profile/dcp_id %u: \n", dcp_id);
        ret = SCORE_ERR_DCP_ID_INVALID;
        goto unlock_done;
    }

    ts_node = bnl_node_get_by_key(profile, score_key);
    if (ts_node) {
        bnl_rtt_stats_fill_from_st(&stats->rtt_orig, &ts_node->orig_st);
        bnl_rtt_stats_fill_from_st(&stats->rtt_reply, &ts_node->reply_st);

        if (profile->activelist_clearonread) {
            bnl_ts_clear_state(&ts_node->reply_st);
            bnl_ts_clear_state(&ts_node->orig_st);
        }
    } else {
        __logError("node not found for key[%u:%u] \n", score_key.incarn, score_key.idx);
        ret = SCORE_ERR_STREAM_ID_INVALID;
    }
unlock_done:
    BNL_UNLOCK();
    return ret;
}

static inline int bnl_ct_tuple_to_stream_tuple(struct nf_conntrack_tuple *ct_tuple,
		score_stream_tuple_t *st_tuple)
{
	if (ct_tuple->src.l3num == AF_INET) {
		st_tuple->source_ip_addr.ipv4.u32 = ct_tuple->src.u3.ip;
		st_tuple->dest_ip_addr.ipv4.u32   = ct_tuple->dst.u3.ip;
		st_tuple->family = SCORE_FAMILY_IPV4;
	} else if (ct_tuple->src.l3num == AF_INET6) {
		memcpy(&st_tuple->source_ip_addr.ipv6, ct_tuple->src.u3.ip6,
				sizeof(st_tuple->source_ip_addr.ipv6));
		memcpy(&st_tuple->dest_ip_addr.ipv6, ct_tuple->dst.u3.ip6,
				sizeof(st_tuple->dest_ip_addr.ipv6));
		st_tuple->family = SCORE_FAMILY_IPV6;
	} else {
		__logNotice("%s: unsupported l3 protoccol %x\n", __func__, ntohs(ct_tuple->src.l3num));
		return -1;
	}

	st_tuple->source_port = ct_tuple->src.u.all;
	st_tuple->dest_port = ct_tuple->dst.u.all;
	st_tuple->proto = ct_tuple->dst.protonum;
	st_tuple->dir = ct_tuple->dst.dir;
	return 0;
}

struct bnl_ts_node *bnl_profile_get_next_valid_stream(bnl_profile_t *profile, 
		uint8_t dcp_id, uint16_t *cur_idx)
{
	unsigned int i;

	if ( *cur_idx  >= BCM_MAX_BNL_STREAMS) {
		return NULL;
	}

	for(i= *cur_idx; i<BCM_MAX_BNL_STREAMS; i++ ) {
		if(IS_BIT_ON(bcm_bnl_ts_g.streams[i].profile_bmap, dcp_id)){
			*cur_idx = i;
			return bnl_node_get_by_idx(profile, i);
		}
	}
	return NULL;
}

inline void __bnl_copy_tsnode_to_stream_dump(bnl_profile_t *profile, struct bnl_ts_node *ts_node, bnlctl_stream_dump_t *stream_dump)
{

	bnl_rtt_stats_fill_from_st(&stream_dump->rtt_orig, &ts_node->orig_st);
	bnl_rtt_stats_fill_from_st(&stream_dump->rtt_reply, &ts_node->reply_st);

	/* copy bin limits */
	memcpy(stream_dump->rtt_orig_bin_limits, ts_node->orig_st.bin.limits,
			sizeof(stream_dump->rtt_orig_bin_limits));
	memcpy(stream_dump->rtt_reply_bin_limits, ts_node->reply_st.bin.limits,
			sizeof(stream_dump->rtt_reply_bin_limits));

	stream_dump->app_group_id = ts_node->bnl_groupid;

	bnl_ct_tuple_to_stream_tuple(nf_ct_tuple(ts_node->ct, IP_CT_DIR_ORIGINAL),
			&stream_dump->orig_tuple);
	bnl_ct_tuple_to_stream_tuple(nf_ct_tuple(ts_node->ct, IP_CT_DIR_REPLY),
			&stream_dump->reply_tuple);

	if (profile->activelist_clearonread) {
		bnl_ts_clear_state(&ts_node->reply_st);
		bnl_ts_clear_state(&ts_node->orig_st);
	}
}

int bcm_bnl_stream_tbl_dump_get_next(bnlctl_stream_dump_t *stream_dump)
{
	struct bnl_ts_node *ts_node;
	uint8_t dcp_id;
	bnl_profile_t *profile = NULL;
	int ret=SCORE_ERR_OK;
	uint16_t cur_idx = stream_dump->cur_idx;

	dcp_id = stream_dump->profile_id;

	BNL_LOCK();
	/* check if profile exists */
	if(dcp_id < BNL_MAX_PROFILES)
		profile = bcm_bnl_ts_g.profiles[dcp_id];

	if (!profile) {
		__logError("invalid profile/dcp_id %u: \n", dcp_id);
		ret = SCORE_ERR_DCP_ID_INVALID;
		goto unlock_done;
	}

	ts_node = bnl_profile_get_next_valid_stream(profile, dcp_id, &cur_idx);
	if (ts_node && ts_node->ct) {
		stream_dump->stream_id = ts_node->score_key.key;
		stream_dump->next_idx = cur_idx+1;
		__bnl_copy_tsnode_to_stream_dump(profile, ts_node, stream_dump);
	} else {
		stream_dump->stream_id = SCORE_STREAM_ID_INVALID;
		stream_dump->next_idx = BCM_MAX_BNL_STREAMS;
	}
unlock_done:
	BNL_UNLOCK();
	return ret;
}

int bcm_bnl_get_stream_dump_by_streamid(bnlctl_stream_dump_t *stream_dump)
{
	struct bnl_ts_node *ts_node;
	blog_score_key_t score_key;
	uint8_t dcp_id;
	bnl_profile_t *profile = NULL;
	int ret=0;

	score_key.key = stream_dump->stream_id;
	dcp_id = stream_dump->profile_id;

	BNL_LOCK();
	/* check if profile exists */
	if(dcp_id < BNL_MAX_PROFILES)
		profile = bcm_bnl_ts_g.profiles[dcp_id];

	if (!profile) {
		__logError("invalid profile/dcp_id %u: \n", dcp_id);
		ret = SCORE_ERR_DCP_ID_INVALID;
		goto unlock_done;
	}

	ts_node = bnl_node_get_by_key(profile, score_key);
	if (ts_node && ts_node->ct) {
		__bnl_copy_tsnode_to_stream_dump(profile, ts_node, stream_dump);
	} 
	else {
		__logError("node not found for key[%u:%u] \n", score_key.incarn, score_key.idx);
		ret = SCORE_ERR_STREAM_ID_INVALID;
	}
unlock_done:
	BNL_UNLOCK();
	return ret;
}


static inline bool skb_has_tcp_timestamp(struct sk_buff *skb)
{
	/*TODO check if this sufficent for allcases of IPV6 & V4/
	 * or else parse the packet
	 */
	struct tcphdr *th = (struct tcphdr *)skb_transport_header(skb);
	struct tcp_ts_option ts_opt;
	return bcm_tcp_parse_option_ts(th, &ts_opt);
}

static inline bool is_ct_timestamp_present(const struct nf_conn *ct)
{
	bool ct_has_timestamps = true;

	/*check for tcp time stamps */
	if ((nf_ct_protonum(ct) == IPPROTO_TCP)) {

		if(!ct->bcm_ext.orig_tcp_ts_en) {
			ct_has_timestamps = false;
			__logInfo("ct=%px: no timestamp in orig direction\n", ct);
		}

		if (test_bit(IPS_SEEN_REPLY_BIT, &ct->status)) {
			if(!ct->bcm_ext.reply_tcp_ts_en) {
				ct_has_timestamps = false;
				__logInfo("ct=%px: no timestamp in reply direction\n", ct);
			}
		}
	}
	/* udp if slelcted assume quic, its resposibilty of user */
	return ct_has_timestamps;
}

int bcm_bnl_ct_start_trace(score_mon_start_data_t *start_cfg)
{
	struct bnl_ts_node *ts_node;
	struct nf_conn *ct = start_cfg->ct;
	score_bnl_stream_cfg_t *bnl_cfg = &start_cfg->stream_data.bnl_cfg;
	uint8_t rtt_orig_bincfg_id; 
	uint8_t rtt_reply_bincfg_id;
	bnl_profile_t *profile = NULL;

	blog_score_key_t score_key;
	int ret = SCORE_ERR_OK;

	/* saftey check */
	if(!ct) {
		__logInfo("CT trace for NULL \n");
		ret = SCORE_ERR_INTERNAL;
		goto done;
	}

	if ((nf_ct_protonum(ct) != IPPROTO_TCP) && (nf_ct_protonum(ct) != IPPROTO_UDP)) {
		__logInfo("CT trace for non TCP/UDP proto =%u \n", nf_ct_protonum(ct));
		ret = SCORE_ERR_STREAM_UNSUPPORTED_L4PROTO;
		goto done;
	}

	if (start_cfg->score_stream_id == SCORE_STREAM_ID_INVALID) {
		ret = SCORE_ERR_INTERNAL;
		goto done;
	}


	score_key.key = start_cfg->score_stream_id;

	if(bnl_cfg->use_bincfg_ids) {
		rtt_orig_bincfg_id = bnl_cfg->rtt_orig_bincfg_id;
		rtt_reply_bincfg_id = bnl_cfg->rtt_reply_bincfg_id;

		if ((rtt_orig_bincfg_id >= BNL_MAX_BIN_CFGS) || (rtt_reply_bincfg_id >= BNL_MAX_BIN_CFGS)) {

			__logError("invalid rtt_bincfg_id %u:%u max is %u \n", rtt_orig_bincfg_id,
					rtt_reply_bincfg_id, BNL_MAX_BIN_CFGS);
			ret = SCORE_ERR_CFG_INVALID;
			goto done;
		}
	} else {
		/*TODO support use_binlimits */
		/* use default bins */
		rtt_orig_bincfg_id = 0;
		rtt_reply_bincfg_id = 0;
	}

	/* allocate trace Id and store in CT and also in blog if present */
	BNL_LOCK();

	/* check if profile exists */
	if(start_cfg->dcp_id < BNL_MAX_PROFILES)
		profile = bcm_bnl_ts_g.profiles[start_cfg->dcp_id];

	if (!profile) {
		__logError("invalid profile/dcp_id %u: \n", start_cfg->dcp_id);
		ret = SCORE_ERR_DCP_ID_INVALID;
		goto unlock_done;
	}

	if (!is_ct_timestamp_present(ct)) {
		profile->notsval_ct_skip++;
		ret = SCORE_ERR_STREAM_NO_TCP_TIMESTAMP;
		goto unlock_done;
	}

	ts_node = bnl_node_alloc_by_key(profile, score_key);

	if(ts_node == NULL) {
		ret = SCORE_ERR_INTERNAL;
		goto unlock_done;
	}

	/*TODO copy ct tuples */
	ts_node->ct = ct;

	/*setup bin cfg */
	bnl_bin_init(profile, &ts_node->orig_st, rtt_orig_bincfg_id);
	bnl_bin_init(profile, &ts_node->reply_st, rtt_reply_bincfg_id);
	ts_node->bnl_groupid = bnl_cfg->app_group_id;
	ts_node->score_key.key = score_key.key;


	/* copy back bin limits to user */
	memcpy(bnl_cfg->rtt_orig_bin_limits, profile->bins_cfg[rtt_orig_bincfg_id].limits,
			sizeof(bnl_cfg->rtt_orig_bin_limits));
	memcpy(bnl_cfg->rtt_reply_bin_limits, profile->bins_cfg[rtt_reply_bincfg_id].limits,
			sizeof(bnl_cfg->rtt_reply_bin_limits));

	SET_BIT(bcm_bnl_ts_g.streams[score_key.idx].profile_bmap, profile->id); 
unlock_done:
	BNL_UNLOCK();
done:
	return ret;
}
EXPORT_SYMBOL(bcm_bnl_ct_start_trace);

int bcm_bnl_ct_stop_trace(score_mon_stop_data_t *stop_cfg)
{
	struct bnl_ts_node *ts_node;
	bnl_profile_t *profile = NULL;
	//struct nf_conn *ct = stop_cfg->ct;
	blog_score_key_t score_key;
	int ret = SCORE_ERR_OK;

	score_key.key = stop_cfg->score_stream_id;

	BNL_LOCK();

	/* check if profile exists */
	if(stop_cfg->dcp_id < BNL_MAX_PROFILES)
		profile = bcm_bnl_ts_g.profiles[stop_cfg->dcp_id];

	if (!profile) {
		__logError("invalid profile/dcp_id %u: \n", stop_cfg->dcp_id);
		ret = SCORE_ERR_DCP_ID_INVALID;
		goto unlock_done;
	}

	ts_node = bnl_node_get_by_key(profile, score_key);
	if (ts_node) {
		/*log the record */
		bcm_bnl_ts_record_store(profile, ts_node);
		/*free the node */
		bcm_bnl_node_free_by_idx(profile, score_key.idx);
		CLR_BIT(bcm_bnl_ts_g.streams[score_key.idx].profile_bmap, profile->id); 
	} else {
		__logNotice(" node not found for key[%u:%u] \n", score_key.incarn, score_key.idx);
		ret = SCORE_ERR_STREAM_ID_INVALID;
	}
unlock_done:
	BNL_UNLOCK();
	return ret;
}
EXPORT_SYMBOL(bcm_bnl_ct_stop_trace);



#define BNL_PROCFS_TOP_DIR_PATH "bnl"
static struct proc_dir_entry *bcm_bnl_proc_top_dir;
struct proc_dir_entry *bcm_bnl_proc_status;

void bnl_bin_seqprint(struct seq_file *s, struct bnl_bin *bin)
{
	int idx;
	struct profile_proc_iter_state *st = s->private;
    bnl_profile_t *profile = st->profile;

	for(idx=0; idx<BNL_MAX_BIN_LIMITS; idx++)
		seq_printf(s,"[<%d] %u  ", bin->limits[idx], bin->data[idx]);

	/* last overflow range*/
	seq_printf(s,"[>=%d] %u ", bin->limits[idx-1], bin->data[idx]);
	if (!profile->print_fmt_singleline)
		seq_printf(s, "\n");
}


static struct bnl_ts_node *tcp_ts_get_next_node(struct seq_file *seq, void *v)
{
	struct profile_proc_iter_state *st = seq->private;
	int i;
    bnl_profile_t *profile = st->profile;
	struct bnl_ts_node *ts_node = NULL;


	for (i = st->index; i < BCM_MAX_BNL_NODES; i++) {
		ts_node = &profile->ts_nodes[i];
		if ((ts_node->inuse))
		{
			st->index = i+1;
			return ts_node;
		}
	}
	st->index = i;
	return NULL;
}

/* this function is used by start, when seq_oveflow happens,
 * we need to retreive the element where seq_overflow happened 
 */ 
static inline void *get_curr_ts_node_by_pos(struct seq_file *seq, loff_t *pos)
{
	struct profile_proc_iter_state *st = seq->private;
	struct bnl_ts_node *ts_node = NULL;

    bnl_profile_t *profile = st->profile;
	/* instead of iterating from begining again till pos,
	 * as we are using array just using st->index
	 */

	if(*pos > BCM_MAX_BNL_NODES)
		return NULL;

	if (st->index && (st->index <= BCM_MAX_BNL_NODES))
		ts_node = &profile->ts_nodes[st->index - 1];/*incremented get_next node */

	if (ts_node && ts_node->inuse)
		return ts_node;
	else 
		return NULL;
}

static void *bnl_activelist_seq_start(struct seq_file *seq, loff_t *pos)
{
	struct profile_proc_iter_state *st = seq->private;
	loff_t p = *pos;
	BNL_LOCK();
	/* start,next,show stop sequence can repeat if seq_overflow happpens
	 * or at the end of file, but *pos will be zero only for first sequence,
	 * handle *pos!=0 properly to break the loop 
	 */
	if (p != 0)
		return get_curr_ts_node_by_pos( seq, pos);

	st->index = 0;
	return SEQ_START_TOKEN;
}

static void *bnl_activelist_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	(*pos)++; /* cuurently not used */
	return tcp_ts_get_next_node(s, v);
}

static void bnl_activelist_seq_stop(struct seq_file *s, void *v)
{
	BNL_UNLOCK();
}


void bnl_tuple_seq_print(struct seq_file *s, struct nf_conntrack_tuple *t)
{
	if (t->src.l3num == AF_INET) {
		seq_printf(s, "src=%pI4 dst=%pI4 ",
			   &t->src.u3.ip, &t->dst.u3.ip);
	} else if (t->src.l3num == AF_INET6) {
		seq_printf(s, "src=%pI6 dst=%pI6 ",
			   t->src.u3.ip6, t->dst.u3.ip6);
	}

	seq_printf(s, "sport=%hu dport=%hu ",
			ntohs(t->src.u.tcp.port),
			ntohs(t->dst.u.tcp.port));
}

void bnl_src_ip_seq_print(struct seq_file *s, struct nf_conntrack_tuple *t)
{
	if (t->src.l3num == AF_INET)
		seq_printf(s, "%pI4<=>GW:\t", &t->src.u3.ip);
	else
		seq_printf(s, "%pI6<=>GW:\t", t->src.u3.all);
}

static inline void  bnl_activelist_seqprint(struct seq_file *s, struct bnl_ts_node *ts_node)
{
	struct nf_conntrack_tuple *o_t = nf_ct_tuple(ts_node->ct, IP_CT_DIR_ORIGINAL);
	struct nf_conntrack_tuple *r_t = nf_ct_tuple(ts_node->ct, IP_CT_DIR_REPLY);
	struct bnl_ts_state *ts_st = &ts_node->orig_st;
	u64 rtt_avg;
	struct profile_proc_iter_state *st = s->private;
    bnl_profile_t *profile = st->profile;

	//seq_printf(s, "Bnl_id=%u Bnl_groupid=%u ", ts_node->bnl_id, ts_node->bnl_groupid);
	seq_printf(s, "Bnl_stream_id=%u Bnl_groupid=%u ", ts_node->score_key.key, ts_node->bnl_groupid);
	bnl_tuple_seq_print(s, o_t);
	bnl_tuple_seq_print(s, r_t);
	seq_printf(s, "l4_proto=%u ", o_t->dst.protonum);
	if (!profile->print_fmt_singleline)
		seq_printf(s, "\n");

	rtt_avg = ts_st->rtt_sum;
	do_div(rtt_avg, ts_st->total_cnt);
	bnl_src_ip_seq_print(s, o_t);
	seq_printf(s, "%-8u %-8u %-8llu %-8u %-8u %-8u %-10llu %-llu ",
			ts_st->rtt_min, ts_st->rtt_max, rtt_avg, ts_st->total_cnt,
			ts_st->timeout_cnt, ts_st->no_slot_cnt, ts_st->rtt_sum, ts_st->rtt_square_sum);
	if (!profile->print_fmt_singleline) {
		seq_printf(s, "\n");
		seq_printf(s, "\t");
	}
	bnl_bin_seqprint(s, &ts_st->bin);
	//seq_printf(s, "\n");

	/* dump other direction */
	ts_st = &ts_node->reply_st;
	rtt_avg = ts_st->rtt_sum;
	do_div(rtt_avg, ts_st->total_cnt);
	bnl_src_ip_seq_print(s, r_t);
	seq_printf(s, "%-8u %-8u %-8llu %-8u %-8u %-8u %-10llu %-llu ",
			ts_st->rtt_min, ts_st->rtt_max, rtt_avg, ts_st->total_cnt,
			ts_st->timeout_cnt, ts_st->no_slot_cnt, ts_st->rtt_sum, ts_st->rtt_square_sum);
	if (!profile->print_fmt_singleline) {
		seq_printf(s, "\n");
		seq_printf(s, "\t");
	}
	bnl_bin_seqprint(s, &ts_st->bin);
	seq_printf(s, "\n");

	if (profile->activelist_clearonread) {
		bnl_ts_clear_state(&ts_node->reply_st);
		bnl_ts_clear_state(&ts_node->orig_st);
	}
}

static inline bool is_activelist_node_nonzero(struct bnl_ts_node *ts_node)
{
	struct bnl_ts_state *ts_st = &ts_node->orig_st;

	if (ts_st->total_cnt || ts_st->timeout_cnt || ts_st->no_slot_cnt || ts_st->skip_cnt)
		return true;
	
	ts_st = &ts_node->reply_st;
	if (ts_st->total_cnt || ts_st->timeout_cnt || ts_st->no_slot_cnt || ts_st->skip_cnt)
		return true;

	return false;
}

static int bnl_activelist_seq_show(struct seq_file *s, void *v)
{
	int ret = 0;
	struct profile_proc_iter_state *st = s->private;
    bnl_profile_t *profile = st->profile;

	if (v == SEQ_START_TOKEN) {
		seq_printf(s, "######## BCM_TCP_TS active monitered connections total=%d ########\n",
				profile->cumm_alloc - profile->cumm_free);
		seq_printf(s, "%-20s %-8s %-8s %8s %8s %8s %10s %8s %s\n\n", 
			"DIRECTION", "MIN_RTT", "MAX_RTT", "AVG_RTT", "COUNT", "TIMEOUT_CNT", "NOSLOT_CNT", "RTT_SUM", "RTT_SUM^2");
	} else {
		if ((!profile->activelist_print_only_nonzero) ||
			(profile->activelist_print_only_nonzero && is_activelist_node_nonzero(v)))
			bnl_activelist_seqprint(s, v);
	}

	if (seq_has_overflowed(s))
		ret = -ENOSPC;

	return ret;
}

static const struct seq_operations bnl_activelist_seq_ops = {
	.start = bnl_activelist_seq_start,
	.next  = bnl_activelist_seq_next,
	.stop  = bnl_activelist_seq_stop,
	.show  = bnl_activelist_seq_show
};

struct donelist_iter_state {
	unsigned int index;
    bnl_profile_t *profile;
};

static struct bnl_ts_record *tcp_ts_get_next_record(struct seq_file *seq, void *v)
{
	struct donelist_iter_state *st = seq->private;
    bnl_profile_t *profile = st->profile;
	int i;
	struct bnl_ts_record *ts_record = NULL;


	for (i = st->index; i < BCM_MAX_BNL_RECORDS; i++) {
		ts_record = &profile->donelist.ts_records[i];
		if ((ts_record->inuse))
		{
			st->index = i+1;
			return ts_record;
		}
	}
	st->index = i;
	return NULL;
}

/* this function is used by start, when seq_oveflow happens,
 * we need to retreive the element where seq_overflow happened 
 */ 
static inline void *get_curr_ts_record_by_pos(struct seq_file *seq, loff_t *pos)
{
	struct donelist_iter_state *st = seq->private;
    bnl_profile_t *profile = st->profile;
	struct bnl_ts_record *ts_record = NULL;

	/* instead of iterating from begining again till pos,
	 * as we are using array just using st->index
	 */

	if(*pos > BCM_MAX_BNL_RECORDS)
		return NULL;

	if (st->index && (st->index <= BCM_MAX_BNL_RECORDS))
		ts_record = &profile->donelist.ts_records[st->index - 1];/*incremented get_next node */

	if (ts_record && ts_record->inuse)
		return ts_record;
	else 
		return NULL;
}

static void *bnl_donelist_seq_start(struct seq_file *seq, loff_t *pos)
{
	struct donelist_iter_state *st = seq->private;
	loff_t p = *pos;

	BNL_RECORD_LOCK();

	/* start,next,show stop sequence can repeat if seq_overflow happpens
	 * or at the end of file, but *pos will be zero only for first sequence,
	 * handle *pos!=0 properly to break the loop 
	 */
	if (p != 0)
		return get_curr_ts_record_by_pos(seq, pos);

	st->index = 0;
	return SEQ_START_TOKEN;
}

static void *bnl_donelist_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	(*pos)++; /* cuurently not used */
	return tcp_ts_get_next_record(s, v);
}

static void bnl_donelist_seq_stop(struct seq_file *s, void *v)
{
	BNL_RECORD_UNLOCK();
}


static inline void  bnl_donelist_seqprint(struct seq_file *s, struct bnl_ts_record *ts_record)
{
	struct nf_conntrack_tuple *o_t = &ts_record->orig_st.tuple;
	struct nf_conntrack_tuple *r_t = &ts_record->reply_st.tuple;
	struct ts_record_info *ts_st = &ts_record->orig_st;
	struct donelist_iter_state *st = s->private;
	bnl_profile_t *profile = st->profile;
	u64 rtt_avg;

	seq_printf(s, "Bnl_stream_id=%u Bnl_groupid=%u ", ts_record->score_key.key, ts_record->bnl_groupid);
	bnl_tuple_seq_print(s, o_t);
	bnl_tuple_seq_print(s, r_t);
	seq_printf(s, "l4_proto=%u ", o_t->dst.protonum);
	if (!profile->print_fmt_singleline)
		seq_printf(s, "\n");

	rtt_avg = ts_st->rtt_sum;
	do_div(rtt_avg, ts_st->total_cnt);
	bnl_src_ip_seq_print(s, o_t);
	seq_printf(s, "%-8u %-8u %-8llu %-8u %-8u %-8u %-10llu %-llu ",
			ts_st->rtt_min, ts_st->rtt_max, rtt_avg, ts_st->total_cnt,
			ts_st->timeout_cnt, ts_st->no_slot_cnt, ts_st->rtt_sum, ts_st->rtt_square_sum);
	if (!profile->print_fmt_singleline) {
		seq_printf(s, "\n");
		seq_printf(s, "\t");
	}
	bnl_bin_seqprint(s, &ts_st->bin);
	//seq_printf(s, "\n");

	/* dump other direction */
	ts_st = &ts_record->reply_st;
	rtt_avg = ts_st->rtt_sum;
	do_div(rtt_avg, ts_st->total_cnt);
	bnl_src_ip_seq_print(s, r_t);
	seq_printf(s, "%-8u %-8u %-8llu %-8u %-8u %-8u %-10llu %-llu ",
			ts_st->rtt_min, ts_st->rtt_max, rtt_avg, ts_st->total_cnt,
			ts_st->timeout_cnt, ts_st->no_slot_cnt, ts_st->rtt_sum, ts_st->rtt_square_sum);
	if (!profile->print_fmt_singleline) {
		seq_printf(s, "\n");
		seq_printf(s, "\t");
	}
	bnl_bin_seqprint(s, &ts_st->bin);
	seq_printf(s, "\n");

}

static int bnl_donelist_seq_show(struct seq_file *s, void *v)
{
	int ret = 0;
	struct donelist_iter_state *st = s->private;
	bnl_profile_t *profile = st->profile;

	if (v == SEQ_START_TOKEN) {
		seq_printf(s, "######## BCM_TCP_TS  recorded connections ########\n");
		seq_printf(s, "%-20s %-8s %-8s %8s %8s %8s %10s %8s %s\n", 
			"DIRECTION", "MIN_RTT", "MAX_RTT", "AVG_RTT", "COUNT", "TIMEOUT_CNT", "NOSLOT_CNT", "RTT_SUM", "RTT_SUM^2");
	} else {
		bnl_donelist_seqprint(s, v);

		if (seq_has_overflowed(s))
			ret = -ENOSPC;
		else if (profile->donelist_clearonread) {
			struct bnl_ts_record *ts_record = v;
			ts_record->inuse = false;
		}
	}

	return ret;
}

static const struct seq_operations bnl_donelist_seq_ops = {
	.start = bnl_donelist_seq_start,
	.next  = bnl_donelist_seq_next,
	.stop  = bnl_donelist_seq_stop,
	.show  = bnl_donelist_seq_show
};

static inline void __prin_bin_limits(struct seq_file *s, int bin_id, int *bin_limits)
{
	int i;
	seq_printf(s, "bin_cfg %d : ", bin_id);
	for(i=0; i<BNL_MAX_BIN_LIMITS; i++)
		seq_printf(s, "%-4d ", bin_limits[i]);

	seq_printf(s, "\n");
}

static int bnl_proc_profile_status_print(struct seq_file *s, void *v)
{
	struct donelist_iter_state *st = s->private;
	bnl_profile_t *profile = st->profile;
	int i;

	seq_printf(s, "#### BNL Monitor Stats ####\n\n");
	seq_printf(s, "BNL TRACEID Alloc: %u  Free : %u  Active: %u Failures: %u \n",
			profile->cumm_alloc, profile->cumm_free,
			profile->ts_node_cnt, profile->cumm_alloc_failures);
	seq_printf(s, "No TCP timestamp skipped Connections : %u \n", profile->notsval_ct_skip);
	seq_printf(s, "No TCP timestamp skipped packets : %u \n", profile->notsval_pkt_skip);
	seq_printf(s, "No QUIC in UDP skipped Connections : %u \n", profile->udp_no_quic_ct_skip);

	seq_printf(s, "#### CONFIG ####\n\n");
	seq_printf(s, "ignore_pure_ack_for_rtt : %u\n", profile->ignore_pure_ack_for_rtt);
	seq_printf(s, "noreply_timeout_ms : %u\n", profile->noreply_timeout_ms);

	seq_printf(s, "\n");
	for(i=0; i<BNL_MAX_BIN_CFGS; i++) {
		__prin_bin_limits(s, i, profile->bins_cfg[i].limits);
	}
	seq_printf(s, "\n");
	return 0;
}

struct bnl_bin_cfg bin_cfg_tmpl;
static struct ctl_table bnl_sysctl_tabl_tmpl[] = {
	{
		.procname	= "debug",
		.data		= &bcm_bnl_ts_g.debug_lvl,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_douintvec,
	},
	{
		.procname	= "print_fmt_singleline",
		.data		= NULL,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_douintvec,
	},
	{
		.procname	= "activelist_clearOnread",
		.data		= NULL,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_douintvec,
	},
	{
		.procname	= "donelist_clearOnread",
		.data		= NULL,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_douintvec,
	},
	{
		.procname	= "activelist_print_only_nonzero",
		.data		= NULL,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_douintvec,
	},
	{
		.procname	= "noreply_timeout_ms",
		.data		= NULL,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_douintvec,
	},
	{
		.procname	= "ignore_pure_ack_for_rtt",
		.data		= NULL,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_douintvec,
	},
	{
		.procname	= "bin_cfg0",
		.data		= NULL,
		.maxlen		= sizeof(bin_cfg_tmpl.limits),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "bin_cfg1",
		.data		= NULL,
		.maxlen		= sizeof(bin_cfg_tmpl.limits),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "bin_cfg2",
		.data		= NULL,
		.maxlen		= sizeof(bin_cfg_tmpl.limits),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "bin_cfg3",
		.data		= NULL,
		.maxlen		= sizeof(bin_cfg_tmpl.limits),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "bin_cfg4",
		.data		= NULL,
		.maxlen		= sizeof(bin_cfg_tmpl.limits),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "bin_cfg5",
		.data		= NULL,
		.maxlen		= sizeof(bin_cfg_tmpl.limits),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "bin_cfg6",
		.data		= NULL,
		.maxlen		= sizeof(bin_cfg_tmpl.limits),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "bin_cfg7",
		.data		= NULL,
		.maxlen		= sizeof(bin_cfg_tmpl.limits),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{ }
};

int bnl_systcl_profile_init(bnl_profile_t *profile)
{
	struct ctl_table *sysctl_tbl = vmalloc(sizeof(bnl_sysctl_tabl_tmpl));

	if(!sysctl_tbl) {
		__logError("profile creation failed: memi alloc failed \n");
		return -1;
	}
	memcpy(sysctl_tbl, bnl_sysctl_tabl_tmpl, sizeof(bnl_sysctl_tabl_tmpl));

	sysctl_tbl[0].data =&bcm_bnl_ts_g.debug_lvl;
	sysctl_tbl[1].data =&profile->print_fmt_singleline;
	sysctl_tbl[2].data =&profile->activelist_clearonread;
	sysctl_tbl[3].data =&profile->donelist_clearonread;
	sysctl_tbl[4].data =&profile->activelist_print_only_nonzero;
	sysctl_tbl[5].data =&profile->noreply_timeout_ms;
	sysctl_tbl[6].data =&profile->ignore_pure_ack_for_rtt;

	sysctl_tbl[7].data =&profile->bins_cfg[0].limits;
	sysctl_tbl[8].data =&profile->bins_cfg[1].limits;
	sysctl_tbl[9].data =&profile->bins_cfg[2].limits;
	sysctl_tbl[10].data =&profile->bins_cfg[3].limits;
	sysctl_tbl[11].data =&profile->bins_cfg[4].limits;
	sysctl_tbl[12].data =&profile->bins_cfg[5].limits;
	sysctl_tbl[13].data =&profile->bins_cfg[6].limits;
	sysctl_tbl[14].data =&profile->bins_cfg[7].limits;

	profile->sysctl_tbl = sysctl_tbl; 
	return 0;
}

#define BNL_MAX_FILENAME 200
char tmp_file_name[BNL_MAX_FILENAME];
static int bnl_profile_proc_exit(bnl_profile_t *profile)
{
	if (profile->proc_activelist) {
		snprintf(tmp_file_name, BNL_MAX_FILENAME,
				BNL_PROCFS_TOP_DIR_PATH"/profile_%u/activelist", profile->id);
		remove_proc_entry(tmp_file_name, NULL);
	}

	if (profile->proc_donelist) {
		snprintf(tmp_file_name, BNL_MAX_FILENAME,
				BNL_PROCFS_TOP_DIR_PATH"/profile_%u/donelist", profile->id);
		remove_proc_entry(tmp_file_name, NULL);
	}

	if (profile->proc_status) {
		snprintf(tmp_file_name, BNL_MAX_FILENAME,
				BNL_PROCFS_TOP_DIR_PATH"/profile_%u/status", profile->id);
		remove_proc_entry(tmp_file_name, NULL);
	}

	if (profile->sysctl_header)
		unregister_net_sysctl_table(profile->sysctl_header);

	if (profile->sysctl_tbl) 
		vfree(profile->sysctl_tbl);

	snprintf(tmp_file_name, BNL_MAX_FILENAME,
			BNL_PROCFS_TOP_DIR_PATH"/profile_%u", profile->id);
	remove_proc_entry(tmp_file_name, NULL);

	return 0;
}

int bnl_profile_proc_activelist_open(struct inode *inode, struct file *file)
{
	int ret;
	struct seq_file *s;

	if ((ret = seq_open(file, &bnl_activelist_seq_ops)) >= 0)
	{
		s = file->private_data;
		s->private = PDE_DATA(inode);
	}
	return ret;

}
static struct proc_ops bnl_profile_proc_activelist_ops = {
	.proc_open = bnl_profile_proc_activelist_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = seq_release,
};

int bnl_profile_proc_donelist_open(struct inode *inode, struct file *file)
{
	int ret;
	struct seq_file *s;

	if ((ret = seq_open(file, &bnl_donelist_seq_ops)) >= 0)
	{
		s = file->private_data;
		s->private = PDE_DATA(inode);
	}
	return ret;

}
static struct proc_ops bnl_profile_proc_donelist_ops = {
	.proc_open = bnl_profile_proc_donelist_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = seq_release,
};

static int bnl_proc_status_print(struct seq_file *s, void *v)
{
	seq_printf(s, "\t\t#### BNL Monitor Status ####\n");
	seq_printf(s, "Number of active profiles : %u \n", bcm_bnl_ts_g.profile_cnt);
	/*TODO print HW & SW queues stats */
	seq_printf(s, "\n");
	 return 0;
}

static int bnl_proc_exit(void)
{
	if (bcm_bnl_proc_status)
		remove_proc_entry(BNL_PROCFS_TOP_DIR_PATH"/status", NULL);

	if (bcm_bnl_proc_top_dir)
		remove_proc_entry(BNL_PROCFS_TOP_DIR_PATH, NULL);

	return 0;
}

static int bnl_proc_init(void)
{
	bcm_bnl_proc_top_dir = proc_mkdir(BNL_PROCFS_TOP_DIR_PATH, NULL);

	if (!bcm_bnl_proc_top_dir) {
		__logError("Unable to create %s proc directory ", BNL_PROCFS_TOP_DIR_PATH);
		return -1;
	}

	/*TODO add stats for record drops in HW & SW queues */
	bcm_bnl_proc_status = proc_create_single(BNL_PROCFS_TOP_DIR_PATH"/status",
			S_IRUGO, NULL, bnl_proc_status_print);

	if (!bcm_bnl_proc_status) {
		__logError("Unable to create proc entry for status");
		return -1;
	}
	return 0;
}

static int bnl_profile_proc_init(bnl_profile_t *profile)
{
	struct proc_dir_entry *profile_proc_top_dir;

	profile->proc_iter_state.profile = profile;

	snprintf(tmp_file_name, BNL_MAX_FILENAME,
			BNL_PROCFS_TOP_DIR_PATH"/profile_%u", profile->id);

	profile_proc_top_dir = proc_mkdir(tmp_file_name, NULL);

	if (!profile_proc_top_dir) {
		__logError("Unable to create %s proc directory ", tmp_file_name);
		return -1;
	}

	profile->proc_activelist = proc_create_data("activelist", 0440, profile_proc_top_dir,
				&bnl_profile_proc_activelist_ops, &profile->proc_iter_state);

	if (!profile->proc_activelist) {
		__logError("Unable to create proc entry for activelist");
		goto error;
	}

	profile->proc_donelist = proc_create_data("donelist", 0440, profile_proc_top_dir,
				&bnl_profile_proc_donelist_ops, &profile->proc_iter_state);

	if (!profile->proc_donelist) {
		__logError("Unable to create proc entry for donelist");
		goto error;
	}

	bnl_systcl_profile_init(profile);
	if (profile->sysctl_tbl) {
		profile->sysctl_header = register_sysctl(tmp_file_name, profile->sysctl_tbl);
		if (!profile->sysctl_header) {
			__logError("Unable to create bnl_sysctl_table");
			goto error;
		}
	}
	else
		goto error;

	snprintf(tmp_file_name, BNL_MAX_FILENAME,
			BNL_PROCFS_TOP_DIR_PATH"/profile_%u/status", profile->id);
	profile->proc_status = proc_create_single_data(tmp_file_name,
			S_IRUGO, NULL, bnl_proc_profile_status_print, &profile->proc_iter_state);

	if (!profile->proc_status) {
		__logError("Unable to create proc entry for status");
		goto error;
	}
	return 0;
error:
	bnl_profile_proc_exit(profile);
	return -1;
}

static inline void default_bin_limits_init(bnl_profile_t *profile)
{
	int i;

	for (i=0; i<BNL_MAX_BIN_CFGS; i++)
		memcpy(profile->bins_cfg[i].limits, default_bin_cfg_limits,
				sizeof(profile->bins_cfg[i].limits));
}

extern int (*blog_bnltrace_fn)(void *arg1, void *arg2, void *arg3);
extern __rcu int (*bcm_score_bnl_notify_cb)(uint32_t cmd, void *arg2, void **arg3);
extern __rcu int (*bcm_score_bnl_wakeup)(uint8_t dcp_id);

int bnl_score_event(uint32_t cmd, void *stream_cfg, void **dcp_priv)
{
	int ret = SCORE_ERR_INTERNAL;
	score_mon_start_data_t *start_cfg;
	score_mon_stop_data_t *stop_cfg;

	__logDebug("entered  cmd =%u \n", cmd);

	if (cmd == SCORE_DCP_CMD_STREAM_ADD) {
		start_cfg = (score_mon_start_data_t *)stream_cfg;
		ret = bcm_bnl_ct_start_trace(start_cfg);

	} else if (cmd == SCORE_DCP_CMD_STREAM_DELETE) {
		stop_cfg = (score_mon_stop_data_t *)stream_cfg;
		ret = bcm_bnl_ct_stop_trace(stop_cfg);
	} else 
		__logError("dcp stream cmd =%u not supported \n", cmd);

	return ret;
}

inline bnl_profile_t *__bnl_get_profile_by_id(uint8_t profile_id)
{
	bnl_profile_t *profile = NULL;

	if(profile_id < BNL_MAX_PROFILES) {
		profile = bcm_bnl_ts_g.profiles[profile_id];
	}

	if (profile == NULL)
		__logError(" Invalid profile Id %u \n", profile_id);

	return profile;
}

bool is_valid_bnl_profile(uint8_t profile_id)
{
	bool ret = false;
	BNL_LOCK();
	if(__bnl_get_profile_by_id(profile_id) != NULL)
		ret = true;
	BNL_UNLOCK();
	return ret;
}

int bcm_bnl_get_profile_stats(bnlctl_profile_stats_t *stats)
{
	bnl_profile_t *profile;
	int ret = SCORE_ERR_OK;

	BNL_LOCK();

	profile = __bnl_get_profile_by_id(stats->profile_id);
	if(!profile) {
		ret = SCORE_ERR_DCP_ID_INVALID;
		goto exit;
	}

	stats->cumm_alloc = profile->cumm_alloc;
	stats->cumm_free = profile->cumm_free;
	stats->cumm_alloc_failures = profile->cumm_alloc_failures;
	stats->notsval_ct_skip = profile->notsval_ct_skip;
	stats->notsval_pkt_skip = profile->notsval_pkt_skip;
	stats->udp_no_quic_ct_skip = profile->udp_no_quic_ct_skip;
	stats->total_pkt_drops =0 ;
	stats->total_ct_violations =0;

exit:
	BNL_UNLOCK();
	return ret;
}

int bcm_bnl_profile_add(bnlctl_profile_cfg_t *cfg)
{
	bnl_profile_t *profile;
	int ret = SCORE_ERR_OK;

	if(cfg->profile_id > BNL_MAX_PROFILES)
		return SCORE_ERR_DCP_ID_INVALID;

	profile = vmalloc(sizeof(bnl_profile_t));
	if(!profile) {
		__logError("profile creation failed: mem alloc failed \n");
		return SCORE_ERR_INTERNAL;
	}
	memset(profile, 0, sizeof(bnl_profile_t));

	profile->id = cfg->profile_id;
	/*initialize profile settings */
	ret = bnl_profile_proc_init(profile);
	if(ret)
		goto free_exit;
	default_bin_limits_init(profile);
	profile->noreply_timeout_ms = BNL_DEFAULT_NOREPLY_TIMEOUT_MS;
	profile->donelist_clearonread=1;
	BNL_LOCK();
	if (bcm_bnl_ts_g.profiles[cfg->profile_id] != NULL) {
		BNL_UNLOCK();
		bnl_profile_proc_exit(profile);
		__logError("profile creation failed: profile_id=%u already present\n", cfg->profile_id);
		ret = SCORE_ERR_INTERNAL;
		goto free_exit;
	} else {
		bcm_bnl_ts_g.profiles[cfg->profile_id] = profile;
		bcm_bnl_ts_g.profile_cnt++;
		try_module_get(THIS_MODULE);
		BNL_UNLOCK();
	}
	return ret;
free_exit:
	vfree(profile);
	return ret;
}

int bcm_bnl_profile_delete(bnlctl_profile_cfg_t *cfg)
{
	int i;
	int ret = SCORE_ERR_OK;
	bnl_profile_t *profile;

	if(cfg->profile_id < BNL_MAX_PROFILES) {

		BNL_LOCK();
		profile = bcm_bnl_ts_g.profiles[cfg->profile_id];
		if(!profile) {
			BNL_UNLOCK();
			__logError("profile %u already freed\n", cfg->profile_id);
			ret = SCORE_ERR_DCP_ID_INVALID;
			goto exit;
		}

		for(i=1; i< BCM_MAX_BNL_STREAMS; i++ ) {
			CLR_BIT(bcm_bnl_ts_g.streams[i].profile_bmap, cfg->profile_id); 
		}

		bcm_bnl_ts_g.profiles[cfg->profile_id] = NULL;
		bcm_bnl_ts_g.profile_cnt--;
		module_put(THIS_MODULE);
		BNL_UNLOCK();
		bnl_profile_proc_exit(profile);
		vfree(profile);
	} else {
		__logError(" Invalid profile Id %u \n", cfg->profile_id);
		ret = SCORE_ERR_DCP_ID_INVALID;
	}
exit:
	return ret;
}

int bcm_bnl_profile_config_get(bnlctl_profile_cfg_t *cfg)
{
	int ret = SCORE_ERR_OK;
	int i;
	bnl_profile_t *profile;

	BNL_LOCK();
	if(cfg->profile_id < BNL_MAX_PROFILES) {

		profile = bcm_bnl_ts_g.profiles[cfg->profile_id];
		if(!profile) {
			__logError("profile %u not yet created\n", cfg->profile_id);
			ret = SCORE_ERR_DCP_ID_INVALID;
			goto exit;
		}

		if(cfg->config_bin) {
			/*check bin_id is valid */
			if(cfg->bin_id > BNL_MAX_BIN_CFGS) {
				__logError(" Invalid bin_id %u \n", cfg->bin_id);
				ret = SCORE_ERR_CFG_INVALID;
				goto exit;
			}

			for (i=0; i<BNL_MAX_BIN_LIMITS; i++) {
				cfg->bin_limits[i] = profile->bins_cfg[cfg->bin_id].limits[i];
			}
		}

		if(cfg->config_ignore_pure_ack_for_rtt) {
			cfg->ignore_pure_ack_for_rtt_val = profile->ignore_pure_ack_for_rtt;
		}

		if(cfg->config_noreply_timeout) {
			cfg->noreply_timeout_val = profile->noreply_timeout_ms;
		}

	} else {
		__logError(" Invalid profile Id %u \n", cfg->profile_id);
		ret = SCORE_ERR_DCP_ID_INVALID;
	}
exit:
	BNL_UNLOCK();
	return ret;
}

int bcm_bnl_profile_config_set(bnlctl_profile_cfg_t *cfg)
{
	int ret = SCORE_ERR_OK;
	int i;
	bnl_profile_t *profile;

	BNL_LOCK();
	if(cfg->profile_id < BNL_MAX_PROFILES) {

		profile = bcm_bnl_ts_g.profiles[cfg->profile_id];
		if(!profile) {
			__logError("profile %u not yet created\n", cfg->profile_id);
			ret = SCORE_ERR_DCP_ID_INVALID;
			goto exit;
		}

		if(cfg->config_bin) {
			/*check bin_id is valid */
			if(cfg->bin_id > BNL_MAX_BIN_CFGS) {
				__logError(" Invalid bin_id %u \n", cfg->bin_id);
				ret = SCORE_ERR_CFG_INVALID;
				goto exit;
			}

			for (i=0; i<BNL_MAX_BIN_LIMITS; i++) {
				profile->bins_cfg[cfg->bin_id].limits[i] =cfg->bin_limits[i];
			}
		}

		if(cfg->config_ignore_pure_ack_for_rtt) {
			profile->ignore_pure_ack_for_rtt = cfg->ignore_pure_ack_for_rtt_val;
		}

		if(cfg->config_noreply_timeout) {
			profile->noreply_timeout_ms = cfg->noreply_timeout_val;
		}

	} else {
		__logError(" Invalid profile Id %u \n", cfg->profile_id);
		ret = SCORE_ERR_DCP_ID_INVALID;
	}
exit:
	BNL_UNLOCK();
	return ret;
}

int bnl_thread_wakeup(uint8_t sdr_id)
{
	if(bcm_bnl_ts_g.debug_lvl)
		__logDebug(" wakeup for queue_id = %u \n", sdr_id);

	if ((bcm_bnl_ts_g.event_thread->__state != TASK_RUNNING))
		wake_up_process(bcm_bnl_ts_g.event_thread);
	return 0;
}

int bnl_sw_data_queue_init(void)
{
	int ret;

	ret = bcm_async_queue_init(&bcm_bnl_ts_g.evt_queue, BNL_SW_DATA_QUEUE_SIZE, sizeof(score_bnl_data_t));

	if (ret)
		__logError("BNL:Failed to allocate evt_queue\n");

	return ret;
}

void bnl_sw_data_queue_uninit(void)
{
	bcm_async_queue_uninit(&bcm_bnl_ts_g.evt_queue);
}

static inline int bnl_process_hw_data_queue(int budget)
{
	score_bnl_data_t pkt_info;
	int cnt = budget;
	do {
		/*TODO: dow we need to clear pkt_info, 
		 * check if its already done internally for all platforms */
		if(score_hw_bnl_ring_info_get(&pkt_info) == 0) {
			/*convert hw timestamp to bnl time resolution */
			pkt_info.timestamp =  bnl_hw_time_to_bnl_time(pkt_info.timestamp);
			bnl_process_pkt_info(&pkt_info);
		}else
			break;
	} while(--cnt);

	if(bcm_bnl_ts_g.debug_lvl)
		__logDebug(" processed pkt records=%d \n", budget - cnt);

	if (budget)
		return 0;
	else
		return 1;
}

static inline int bnl_process_sw_data_queue(int budget)
{
    bcm_async_queue_t *sw_queue = &bcm_bnl_ts_g.evt_queue;
    score_bnl_data_t *pkt_info;
    int cnt = budget;

    do {

        if (bcm_async_queue_not_empty(sw_queue))
        {
            pkt_info = (score_bnl_data_t *) bcm_async_queue_entry_read(sw_queue);
            bnl_process_pkt_info(pkt_info);
            bcm_async_queue_entry_dequeue(sw_queue);
            sw_queue->stats.reads++;
        }
        else
            break;
    } while(--cnt);

	if(bcm_bnl_ts_g.debug_lvl)
		__logDebug(" processed pkt records=%d \n", budget - cnt);

    if(budget)
        return 0;
    else
        return 1;
}

static inline int bnl_process_event_queue(int budget)
{
    return 0;
}

static int bnl_event_thread_func(void *thread_data)
{
    //int event_budget = 64;
    int data_hw_budget = 64;
    int data_sw_budget = 64;
    int more_data;

	 while (!kthread_should_stop()) {
         more_data = 0;
//recheck:
        /*read & process the ring */
         //more_data |= bnl_process_event_queue(event_budget);
         more_data |= bnl_process_hw_data_queue(data_hw_budget);
         more_data |= bnl_process_sw_data_queue(data_sw_budget);

        /*TODO add fucntions to check if data is presentin qeuue */

         if(more_data) {
             yield();
         } else {
             //goto recheck;
             set_current_state(TASK_INTERRUPTIBLE);
             //schedule_timeout(bcm_bnl_ts_g.event_thread_intvl);
             schedule();
         }
	}
	return 0;
}

int bnl_event_thread_create(void)
{
	 struct task_struct *tsk;

	bcm_bnl_ts_g.event_thread_intvl = msecs_to_jiffies(1000); /*1sec */

	 tsk = kthread_create(bnl_event_thread_func, NULL, "bnl_event");
	 if (IS_ERR(tsk))
	 {
		__logError(" kthread creation failed\n");
		  return -1;
	 }

	bcm_bnl_ts_g.event_thread = tsk;
	wake_up_process(bcm_bnl_ts_g.event_thread);
	 __logNotice("BNL Event Thread created successfully\n");
	 return 0;
}

void bnl_event_thread_destroy(void)
{
	 kthread_stop(bcm_bnl_ts_g.event_thread);
}

static int __init bcm_bnl_init(void)
{
	int ret;

	if (bcm_license_check(BP3_FEATURE_BNL) <= 0) {
		__logError("**** Missing BP3 BNL license, BNL will not work **** \n");
		return -1;
	}

    ret = bnl_cdev_init();
	if (ret < 0) {
		__logError("char dev initialization failed\n");
		return ret;
	}

	ret = bnl_proc_init();
	if (ret < 0) {
		__logError("proc initialization failed\n");
		return ret;
	}

    bnl_sw_data_queue_init();

    ret = score_hw_bnl_ring_create(SCORE_BNL_QUEUE_SIZE);
    if (ret < 0)
    {
        __logError("score_queue_init(%d) error %d\n", SCORE_BNL_QUEUE_SIZE, ret);
        return ret;
    }

    bnl_event_thread_create();



	rcu_assign_pointer(bcm_score_bnl_notify_cb, bnl_score_event);
	rcu_assign_pointer(bcm_score_bnl_wakeup, bnl_thread_wakeup);

	synchronize_rcu();

	blog_bnltrace_fn = (int (*)(void *arg1, void *arg2, void *arg3))bcm_bnltrace_pkt_rcv;
	return 0;
}

static void __exit bcm_bnl_exit(void)
{
	rcu_assign_pointer(bcm_score_bnl_notify_cb, NULL);
	rcu_assign_pointer(bcm_score_bnl_wakeup, NULL);
	synchronize_rcu();
	
    score_hw_bnl_ring_destroy(SCORE_BNL_QUEUE_SIZE);
	/*TODO need to update CT's and flush flows */
	blog_bnltrace_fn = NULL;
	bnl_proc_exit();
    bnl_cdev_uninit();
    bnl_event_thread_destroy();
    bnl_sw_data_queue_uninit();
	return;
}

module_init(bcm_bnl_init);
module_exit(bcm_bnl_exit);
MODULE_LICENSE("GPL");
