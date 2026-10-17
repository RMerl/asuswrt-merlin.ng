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

#include "bp3_license.h"

#define MAX_TCP_TS_TRACK 4
#define BCM_MAX_BNL_NODES 256

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
#define BNL_MAX_BINS 8
#define BNL_MAX_BIN_CFGS 8
#define BNL_MAX_BIN_LIMITS (BNL_MAX_BINS -1)

struct bin_cfg {
	int limits[BNL_MAX_BIN_LIMITS];
};

unsigned int default_bin_cfg_limits[BNL_MAX_BIN_LIMITS] = {1000, 2000, 4000, 8000, 16000, 32000, 64000};

struct bin_data {
	unsigned int cnt;
	//u64 sum;
};

struct bnl_bin {
	struct bin_cfg cfg;
	struct bin_data data[BNL_MAX_BINS];
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
	u8 in_use_cnt;
};

struct bnl_ts_node {
	struct bnl_ts_state  orig_st;
	struct bnl_ts_state reply_st;
	struct nf_conn *ct;
	u32 bnl_groupid; /* provided by application */
	u32 bnl_id;      /* unique id for this connection */
	bool inuse;
};

struct {
	struct bnl_ts_node ts_nodes[BCM_MAX_BNL_NODES];
	spinlock_t ts_tbl_lock;
	struct bin_cfg bins_cfg[BNL_MAX_BIN_CFGS];
	u32 cumm_alloc;
	u32 cumm_free;
	u32 cumm_alloc_failures;
	u32 notsval_ct_skip; 
	u32 notsval_pkt_skip;
	u32 udp_no_quic_ct_skip; 
	u32 bnl_id_cntr; /*montonous increasing number to identify unique connections*/
	u16 ts_node_cnt;
	u16 ts_node_cur_idx;
	int debug_lvl;
	int activelist_clearonread;
	int donelist_clearonread;
	int activelist_print_only_nonzero;
	int print_fmt_singleline;
	int noreply_timeout_ms;
	int ignore_pure_ack_for_rtt;
} bcm_bnl_ts_g;

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

struct tcp_ts_record {
	/* TODO optimize this structure */
	struct ts_record_info  orig_st;
	struct ts_record_info  reply_st;
	u32 bnl_groupid; /* provided by application */
	u32 bnl_id;      /* unique id for this connection */
	bool inuse;
};

struct {
	struct tcp_ts_record ts_records[BCM_MAX_BNL_RECORDS];
	spinlock_t tbl_lock;
	u16 cnt;
	u16 cur_idx;
} bcm_tcp_ts_record_g;

#define BNL_RECORD_LOCK()		spin_lock_bh(&bcm_tcp_ts_record_g.tbl_lock)
#define BNL_RECORD_UNLOCK()		spin_unlock_bh(&bcm_tcp_ts_record_g.tbl_lock)

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

u16 bcm_tcp_ts_record_store(struct bnl_ts_node *ts_node)
{
	struct tcp_ts_record *ts_record;

	BNL_RECORD_LOCK();

	ts_record =  &bcm_tcp_ts_record_g.ts_records[bcm_tcp_ts_record_g.cur_idx];
	bcm_tcp_ts_record_g.cur_idx++;
	if (bcm_tcp_ts_record_g.cur_idx >= BCM_MAX_BNL_RECORDS) {
		bcm_tcp_ts_record_g.cur_idx = 0;
	}

	ts_record->inuse = true;
	ts_record->bnl_groupid = ts_node->bnl_groupid;
	ts_record->bnl_id = ts_node->bnl_id;
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

static inline u16 bcm_bnl_traceid_alloc(void)
{
	int i;
	struct bnl_ts_node *ts_node;

	/*TODO allocate basedon bitmap */	
	/* entry 0 is invalid */
	for(i=1; i< BCM_MAX_BNL_NODES; i++) {
		ts_node = &bcm_bnl_ts_g.ts_nodes[i];
		if(ts_node->inuse == false) {
			memset(ts_node, 0, sizeof(struct bnl_ts_node));
			ts_node->inuse = true;
			ts_node->bnl_id = bcm_bnl_ts_g.bnl_id_cntr++;
			bcm_bnl_ts_g.cumm_alloc++;
			bcm_bnl_ts_g.ts_node_cnt++;
			return i;
		}
	}
	bcm_bnl_ts_g.cumm_alloc_failures++;
	return 0;
}

static inline struct bnl_ts_node *bnl_node_get_by_idx(u16 idx)
{
	if((idx <=0) || (idx >= BCM_MAX_BNL_NODES)) {
		pr_err("%s: inavlid idx=%d \n", __func__, idx);
		return NULL;
	}

	return &bcm_bnl_ts_g.ts_nodes[idx];
}

static inline void bnl_node_free(struct bnl_ts_node *ts_node)
{
	ts_node->inuse = false;

	bcm_bnl_ts_g.ts_node_cnt--;
	bcm_bnl_ts_g.cumm_free++;
}

static inline void bcm_bnl_node_free_by_idx(u16 idx)
{
	struct bnl_ts_node *ts_node;

	ts_node = bnl_node_get_by_idx(idx);

	if(ts_node && ts_node->inuse)
		bnl_node_free(ts_node);
	else
		pr_err("%s: Double free idx=%d \n", __func__, idx);
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
	struct bin_cfg *bincfg = &ts_st->bin.cfg;

	/*find bin*/
	for(idx=0; idx<BNL_MAX_BIN_LIMITS; idx++) {
		if(rtt < bincfg->limits[idx])
			break;
	}
	/*here idx, will be either a match are a catch all(last bin) */
	ts_st->bin.data[idx].cnt++;
	//ts_st->bin.data[idx].sum += rtt;
}

void bnl_bin_init(struct bnl_ts_state *ts_st, u8 bincfg_idx)
{
	if(bincfg_idx > BNL_MAX_BIN_CFGS) {
		pr_err("%s:Bad bincfg_idx =%d \n", __func__, bincfg_idx);
		bincfg_idx = 0; /*just use cfg 0 */
	}
	/*attach bin config limits, copy per flow is maintained, just in case they are changed after flow in learned*/
	memcpy(&ts_st->bin.cfg, &bcm_bnl_ts_g.bins_cfg[bincfg_idx], sizeof(ts_st->bin.cfg));
	/*clear bin data */
	memset(ts_st->bin.data, 0, sizeof(ts_st->bin.data));
}

int tcp_process_tsecr(struct bnl_ts_node *ts_node, u8 dir, struct tcp_ts_option *pkt_ts_opt)
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

			rtt = (u32)ktime_us_delta(pkt_ts_opt->time, ts_opt->time);

			if (unlikely(rtt > (bcm_bnl_ts_g.noreply_timeout_ms * 1000))) {
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

static inline void tcp_ts_opt_expiry(struct bnl_ts_state *tsval_st, struct tcp_ts_option *pkt_ts_opt)
{
	int i;
	struct tcp_ts_option *ts_opt;

	for (i=0; i<MAX_TCP_TS_TRACK; i++) {
		ts_opt = &tsval_st->ts_opts[i];

		if (!ts_opt->rtt_done && 
				(ktime_ms_delta(pkt_ts_opt->time, ts_opt->time) < bcm_bnl_ts_g.noreply_timeout_ms)) 
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

int tcp_store_ts(struct bnl_ts_node *ts_node, u8 dir, struct tcp_ts_option *pkt_ts_opt)
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
	if (bcm_bnl_ts_g.ignore_pure_ack_for_rtt && pkt_ts_opt->pure_ack)
		goto done;

	/* expire only when full */
	if (tsval_st->in_use_cnt >= MAX_TCP_TS_TRACK)
		tcp_ts_opt_expiry(tsval_st, pkt_ts_opt);

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

int bnl_quic_process_spinbit(struct bnl_ts_node *ts_node, u8 pkt_dir, struct quic_ts_option *pkt_ts_opt)
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

		rtt = (u32)ktime_us_delta(pkt_ts_opt->time, quic_opt->time);

		if (unlikely(rtt > (bcm_bnl_ts_g.noreply_timeout_ms * 1000))) {
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
			(ktime_ms_delta(pkt_ts_opt->time, quic_opt->time) >= bcm_bnl_ts_g.noreply_timeout_ms)) {
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

int bcm_bnltrace_pkt_rcv(void *param1, void *param2, void *param3)
{
	struct tcp_ts_option ts_opt; 
	struct quic_ts_option quic_opt; 
	struct bnl_ts_node *ts_node; 
	int ret=0;
	const Blog_t *blog = param1;
	const FkBuff_t *fkb = param2;
	const union blog_bnl_key *ts_key = &blog->bnl_key;

	if (blog->key.protocol == BLOG_IPPROTO_TCP) {

		struct tcphdr *th = (struct tcphdr *)(fkb->data + blog->l4_offset);

		/*extract timestamps from pkt */
		if (unlikely(bcm_tcp_parse_option_ts(th, &ts_opt) == false)) {
			/* no tcp timestamp */
			bcm_bnl_ts_g.notsval_pkt_skip++;
			ret = -1;
			goto exit;
		}

		ts_opt.time = ktime_get();
		/* check for pure ack */
		/* TODO parse packet instead of relying on blog(works only when ack prio is eanbled) */
		if (blog->key.tcp_pure_ack)
			ts_opt.pure_ack = true;
		 else
			ts_opt.pure_ack = false;

	} else if (blog->key.protocol == BLOG_IPPROTO_UDP) {
		
		struct udphdr *uh = (struct udphdr *)(fkb->data + blog->l4_offset);
		unsigned char *quichdr = (unsigned char *) (uh+1);


		/* check if packet has QUIC short header & spin bit */
		if (is_valid_quic_short_header(quichdr) == false)
			goto exit;

		quic_opt.time = ktime_get();
		quic_opt.spin_bit = bnl_udp_parse_quic_spinbit(quichdr);

	} else {
			/* unsupported protocol */
			pr_info("%s: unsupported protocol %u \n", __func__, blog->key.protocol);
			goto exit;
	}

	BNL_LOCK();
	/*check for tsecr match */
	ts_node = bnl_node_get_by_idx(ts_key->idx);
	if(!ts_node) {
		pr_err("%s: invalid bnl_traceid =%hu \n", __func__, ts_key->idx);
		ret = -1;
		goto unlock_exit;
	}

	if (blog->key.protocol == BLOG_IPPROTO_TCP) {
		tcp_process_tsecr(ts_node, ts_key->dir, &ts_opt);
		/*store new tsval */
		tcp_store_ts(ts_node, ts_key->dir,  &ts_opt);
	} else {
		bnl_quic_process_spinbit(ts_node, ts_key->dir,  &quic_opt);
	}
unlock_exit:
	BNL_UNLOCK();
exit:
	return ret;
}

void bcm_bnl_send_record(struct bnl_ts_node *ts_node)
{
	/* senq unique id along with rtt from src->CPE and CPE->dst  min, max, avg rtt along with
	 * num records and evnt type(end or reached aggregate limit)*/ 
}

static inline bool skb_has_tcp_timestamp(struct sk_buff *skb)
{
	/*TODO check if this sufficent for allcases of IPV6 & V4/
	 * or else parse teh packet
	 */
	struct tcphdr *th = (struct tcphdr *)skb_transport_header(skb);
	struct tcp_ts_option ts_opt;
	return bcm_tcp_parse_option_ts(th, &ts_opt);
}

int bcm_bnl_ct_start_trace(struct nf_conn *ct, u32 bnl_groupid, u8 bincfg_o_idx, u8 bincfg_r_idx)
{
	struct bnl_ts_node *ts_node;
	/* saftey check */
	if(ct->bcm_ext.bnl_idx) {
		pr_warn("%s: traceid already present id=%u \n", __func__, ct->bcm_ext.bnl_idx);
		goto done;
	}

	if ((nf_ct_protonum(ct) != IPPROTO_TCP) || (nf_ct_protonum(ct) != IPPROTO_UDP)) {
		pr_warn("%s: CT trace for non TCP/UDP proto =%u \n", __func__, nf_ct_protonum(ct));
		goto done;
	}

	/* allocate trace Id and store in CT and also in blog if present */ 
	BNL_LOCK();
	ct->bcm_ext.bnl_idx = bcm_bnl_traceid_alloc();
	if(ct->bcm_ext.bnl_idx) {
		ts_node = bnl_node_get_by_idx(ct->bcm_ext.bnl_idx);
		ts_node->ct = ct;

		/*setup bin cfg */
		bnl_bin_init(&ts_node->orig_st, bincfg_o_idx);
		bnl_bin_init(&ts_node->reply_st, bincfg_r_idx);
		ts_node->bnl_groupid = bnl_groupid;
		BNL_UNLOCK();

		/*TODO pass skb, and if skb has blog update it, this is to handle
		  transient state i.e monitor activate while flow is being learned*/

		/*update existing accelerated flows of this  CT */ 
		blog_notify(UPDATE_BNL, (void*)ct, ct->bcm_ext.bnl_idx, 0);
	} else
		BNL_UNLOCK();
	
done:
	return 0;
}
EXPORT_SYMBOL(bcm_bnl_ct_start_trace);

/*TODO decide of lock order of blog_lock and BNL_lock */

int bcm_bnl_ct_stop_trace(struct nf_conn *ct)
{
	struct bnl_ts_node *ts_node;
	if(ct->bcm_ext.bnl_idx) {

		/* update existing accelerated flows of this  CT */ 
		blog_notify(UPDATE_BNL, (void*)ct, 0, 0);

		BNL_LOCK();
		ts_node = bnl_node_get_by_idx(ct->bcm_ext.bnl_idx);
		if (ts_node) {
			/*log the record */
			bcm_tcp_ts_record_store(ts_node);
			/*free the node */
			bcm_bnl_node_free_by_idx(ct->bcm_ext.bnl_idx);
			ct->bcm_ext.bnl_idx = 0;
		}
		BNL_UNLOCK();
	}
	return 0;
}
EXPORT_SYMBOL(bcm_bnl_ct_stop_trace);

static int
bcm_bnl_ct_event(enum ip_conntrack_events events, struct nf_conn *ct, struct sk_buff *skb)
{
	struct bnl_ts_node *ts_node;
	u32 bnl_groupid;
	u8 bincfg_o_idx;
	u8 bincfg_r_idx;
	/* on new connection & marked for bnltrace try to allocate a traced id 
	 * store trace id in ct and also upate in blog(if present)*/

	if (events & (1 << IPCT_DESTROY)) {

		/*TODO stop tracking from time wait state instead of waiting for destroy */

		BNL_LOCK();
		if(ct->bcm_ext.bnl_idx) {
			ts_node = bnl_node_get_by_idx(ct->bcm_ext.bnl_idx);
			if (ts_node) {
				/*log the record */
				bcm_tcp_ts_record_store(ts_node);
				/*free the node */
				bcm_bnl_node_free_by_idx(ct->bcm_ext.bnl_idx);
				ct->bcm_ext.bnl_idx = 0;
			}
		}
		BNL_UNLOCK();
	}
	else if (events & ((1 << IPCT_NEW) | (1 << IPCT_RELATED))) {

		
		if (SKB_BCMMARK_GET_BNL_TRACE(skb->bcm_ext.bcmmark) != BCMMARK_TRACE_TYPE_BNL) {

			/*TODO how to avoid multiple CT's for same pkt allocating 
			 * multiple trace ID's when blog is not present
			 */
			goto done;
		}

		bnl_groupid = SKB_BCMMARK_GET_BNL_APPID(skb->bcm_ext.bcmmark);
		bincfg_o_idx = SKB_BCMMARK_GET_BNL_O_BINIDX(skb->bcm_ext.bcmmark);
		bincfg_r_idx = SKB_BCMMARK_GET_BNL_R_BINIDX(skb->bcm_ext.bcmmark);

		/* saftey check */
		if (nf_ct_protonum(ct) == IPPROTO_TCP) {
			/* check if skb has TCP timestamp */
			if (!skb_has_tcp_timestamp(skb)) {
				bcm_bnl_ts_g.notsval_ct_skip++;
				goto done;
			}
		} else if (nf_ct_protonum(ct) == IPPROTO_UDP) {
			struct udphdr *uh = (struct udphdr *)skb_transport_header(skb);
			/* check for quic header */
			if (is_valid_quic_header(uh) == false) {
				bcm_bnl_ts_g.udp_no_quic_ct_skip++;
				goto done;
			}
		} else {
			pr_warn(" CT trace for non TCP/UDP proto =%u \n", nf_ct_protonum(ct));
			goto done;
		}

		/*check if blog is present */
		if (skb->blog_p && skb->blog_p->bnl_key.idx) {
			/*already monitored by some other ct */
			goto done;
		}

		/* allocate trace Id and store in CT and also in blog if present */ 
		BNL_LOCK();
		ct->bcm_ext.bnl_idx = bcm_bnl_traceid_alloc();
		if(ct->bcm_ext.bnl_idx) {
			ts_node = bnl_node_get_by_idx(ct->bcm_ext.bnl_idx);
			ts_node->ct = ct;

			/*setup bin cfg */
			bnl_bin_init(&ts_node->orig_st, bincfg_o_idx);
			bnl_bin_init(&ts_node->reply_st, bincfg_r_idx);
			ts_node->bnl_groupid = bnl_groupid;

			blog_link(BNLTRACE, blog_ptr(skb),
					(void*)ct, IP_CT_DIR_ORIGINAL, ct->bcm_ext.bnl_idx);
		}
		BNL_UNLOCK();
	}
done:
	return 0;
}


#define BNL_PROCFS_TOP_DIR_PATH "bnl"
static struct proc_dir_entry *bcm_tcp_ts_proc_top_dir;
static struct proc_dir_entry *bcm_tcp_ts_proc_activelist_file;
static struct proc_dir_entry *bcm_tcp_ts_proc_donelist_file;

void bnl_bin_seqprint(struct seq_file *s, struct bnl_bin *bin)
{
	int idx;
	
	for(idx=0; idx<BNL_MAX_BIN_LIMITS; idx++)
		seq_printf(s,"[<%d] %u  ", bin->cfg.limits[idx], bin->data[idx].cnt);

	/* last overflow range*/
	seq_printf(s,"[>=%d] %u ", bin->cfg.limits[idx-1], bin->data[idx].cnt);
	if (!bcm_bnl_ts_g.print_fmt_singleline)
		seq_printf(s, "\n");
}

struct activelist_iter_state {
	unsigned int index;
};

static struct bnl_ts_node *tcp_ts_get_next_node(struct seq_file *seq, void *v)
{
	struct activelist_iter_state *st = seq->private;
	int i;
	struct bnl_ts_node *ts_node = NULL;


	for (i = st->index; i < BCM_MAX_BNL_NODES; i++) {
		ts_node = &bcm_bnl_ts_g.ts_nodes[i];
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
	struct activelist_iter_state *st = seq->private;
	struct bnl_ts_node *ts_node = NULL;

	/* instead of iterating from begining again till pos,
	 * as we are using array just using st->index
	 */

	if(*pos > BCM_MAX_BNL_NODES)
		return NULL;

	if (st->index && (st->index <= BCM_MAX_BNL_NODES))
		ts_node = &bcm_bnl_ts_g.ts_nodes[st->index - 1];/*incremented get_next node */

	if (ts_node && ts_node->inuse)
		return ts_node;
	else 
		return NULL;
}

static void *bnl_activelist_seq_start(struct seq_file *seq, loff_t *pos)
{
	struct activelist_iter_state *st = seq->private;
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

static inline void tcp_ts_clear_state(struct bnl_ts_state *ts_st)
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

	seq_printf(s, "Bnl_id=%u Bnl_groupid=%u ", ts_node->bnl_id, ts_node->bnl_groupid);
	bnl_tuple_seq_print(s, o_t);
	bnl_tuple_seq_print(s, r_t);
	seq_printf(s, "l4_proto=%u ", o_t->dst.protonum);
	if (!bcm_bnl_ts_g.print_fmt_singleline)
		seq_printf(s, "\n");

	rtt_avg = ts_st->rtt_sum;
	do_div(rtt_avg, ts_st->total_cnt);
	bnl_src_ip_seq_print(s, o_t);
	seq_printf(s, "%-8u %-8u %-8llu %-8u %-8u %-8u %-10llu %-llu ",
			ts_st->rtt_min, ts_st->rtt_max, rtt_avg, ts_st->total_cnt,
			ts_st->timeout_cnt, ts_st->no_slot_cnt, ts_st->rtt_sum, ts_st->rtt_square_sum);
	if (!bcm_bnl_ts_g.print_fmt_singleline) {
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
	if (!bcm_bnl_ts_g.print_fmt_singleline) {
		seq_printf(s, "\n");
		seq_printf(s, "\t");
	}
	bnl_bin_seqprint(s, &ts_st->bin);
	seq_printf(s, "\n");

	if (bcm_bnl_ts_g.activelist_clearonread) {
		tcp_ts_clear_state(&ts_node->reply_st);
		tcp_ts_clear_state(&ts_node->orig_st);
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

	if (v == SEQ_START_TOKEN) {
		seq_printf(s, "######## BCM_TCP_TS active monitered connections total=%d ########\n",
				bcm_bnl_ts_g.cumm_alloc - bcm_bnl_ts_g.cumm_free);
		seq_printf(s, "%-20s %-8s %-8s %8s %8s %8s %10s %8s %s\n\n", 
			"DIRECTION", "MIN_RTT", "MAX_RTT", "AVG_RTT", "COUNT", "TIMEOUT_CNT", "NOSLOT_CNT", "RTT_SUM", "RTT_SUM^2");
	} else {
		if ((!bcm_bnl_ts_g.activelist_print_only_nonzero) ||
			(bcm_bnl_ts_g.activelist_print_only_nonzero && is_activelist_node_nonzero(v)))
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
};

static struct tcp_ts_record *tcp_ts_get_next_record(struct seq_file *seq, void *v)
{
	struct donelist_iter_state *st = seq->private;
	int i;
	struct tcp_ts_record *ts_record = NULL;


	for (i = st->index; i < BCM_MAX_BNL_RECORDS; i++) {
		ts_record = &bcm_tcp_ts_record_g.ts_records[i];
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
	struct activelist_iter_state *st = seq->private;
	struct tcp_ts_record *ts_record = NULL;

	/* instead of iterating from begining again till pos,
	 * as we are using array just using st->index
	 */

	if(*pos > BCM_MAX_BNL_RECORDS)
		return NULL;

	if (st->index && (st->index <= BCM_MAX_BNL_RECORDS))
		ts_record = &bcm_tcp_ts_record_g.ts_records[st->index - 1];/*incremented get_next node */

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


static inline void  bnl_donelist_seqprint(struct seq_file *s, struct tcp_ts_record *ts_record)
{
	struct nf_conntrack_tuple *o_t = &ts_record->orig_st.tuple;
	struct nf_conntrack_tuple *r_t = &ts_record->reply_st.tuple;
	struct ts_record_info *ts_st = &ts_record->orig_st;
	u64 rtt_avg;

	seq_printf(s, "Bnl_id=%u Bnl_groupid=%u ", ts_record->bnl_id, ts_record->bnl_groupid);
	bnl_tuple_seq_print(s, o_t);
	bnl_tuple_seq_print(s, r_t);
	seq_printf(s, "l4_proto=%u ", o_t->dst.protonum);
	if (!bcm_bnl_ts_g.print_fmt_singleline)
		seq_printf(s, "\n");

	rtt_avg = ts_st->rtt_sum;
	do_div(rtt_avg, ts_st->total_cnt);
	bnl_src_ip_seq_print(s, o_t);
	seq_printf(s, "%-8u %-8u %-8llu %-8u %-8u %-8u %-10llu %-llu ",
			ts_st->rtt_min, ts_st->rtt_max, rtt_avg, ts_st->total_cnt,
			ts_st->timeout_cnt, ts_st->no_slot_cnt, ts_st->rtt_sum, ts_st->rtt_square_sum);
	if (!bcm_bnl_ts_g.print_fmt_singleline) {
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
	if (!bcm_bnl_ts_g.print_fmt_singleline) {
		seq_printf(s, "\n");
		seq_printf(s, "\t");
	}
	bnl_bin_seqprint(s, &ts_st->bin);
	seq_printf(s, "\n");

}

static int bnl_donelist_seq_show(struct seq_file *s, void *v)
{
	int ret = 0;

	if (v == SEQ_START_TOKEN) {
		seq_printf(s, "######## BCM_TCP_TS  recorded connections ########\n");
		seq_printf(s, "%-20s %-8s %-8s %8s %8s %8s %10s %8s %s\n", 
			"DIRECTION", "MIN_RTT", "MAX_RTT", "AVG_RTT", "COUNT", "TIMEOUT_CNT", "NOSLOT_CNT", "RTT_SUM", "RTT_SUM^2");
	} else {
		bnl_donelist_seqprint(s, v);

		if (seq_has_overflowed(s))
			ret = -ENOSPC;
		else if (bcm_bnl_ts_g.donelist_clearonread) {
			struct tcp_ts_record *ts_record = v;
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


static struct proc_dir_entry *bcm_bnl_proc_stats_file;
static int bnl_proc_stats_print(struct seq_file *s, void *v)
{
	seq_printf(s, "\t\t#### BNL Monitor Stats ####\n");
	seq_printf(s, "BNL TRACEID Alloc: %u  Free : %u  Active: %u Failures: %u \n",
			bcm_bnl_ts_g.cumm_alloc, bcm_bnl_ts_g.cumm_free,
			bcm_bnl_ts_g.ts_node_cnt, bcm_bnl_ts_g.cumm_alloc_failures);
	seq_printf(s, "No TCP timestamp skipped Connections : %u \n", bcm_bnl_ts_g.notsval_ct_skip);
	seq_printf(s, "No TCP timestamp skipped packets : %u \n", bcm_bnl_ts_g.notsval_pkt_skip);
	seq_printf(s, "No QUIC in UDP skipped Connections : %u \n", bcm_bnl_ts_g.udp_no_quic_ct_skip);
	seq_printf(s, "\n");
	 return 0;
}

static struct ctl_table_header *tcp_ts_sysctl_header;
static struct ctl_table tcp_ts_sysctl_table[] = {
	{
		.procname	= "debug",
		.data		= &bcm_bnl_ts_g.debug_lvl,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_douintvec,
	},
	{
		.procname	= "print_fmt_singleline",
		.data		= &bcm_bnl_ts_g.print_fmt_singleline,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_douintvec,
	},
	{
		.procname	= "activelist_clearOnread",
		.data		= &bcm_bnl_ts_g.activelist_clearonread,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_douintvec,
	},
	{
		.procname	= "donelist_clearOnread",
		.data		= &bcm_bnl_ts_g.donelist_clearonread,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_douintvec,
	},
	{
		.procname	= "activelist_print_only_nonzero",
		.data		= &bcm_bnl_ts_g.activelist_print_only_nonzero,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_douintvec,
	},
	{
		.procname	= "noreply_timeout_ms",
		.data		= &bcm_bnl_ts_g.noreply_timeout_ms,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_douintvec,
	},
	{
		.procname	= "ignore_pure_ack_for_rtt",
		.data		= &bcm_bnl_ts_g.ignore_pure_ack_for_rtt,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_douintvec,
	},
	{
		.procname	= "bin_cfg0",
		.data		= bcm_bnl_ts_g.bins_cfg[0].limits,
		.maxlen		= sizeof(bcm_bnl_ts_g.bins_cfg[0].limits),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "bin_cfg1",
		.data		= bcm_bnl_ts_g.bins_cfg[1].limits,
		.maxlen		= sizeof(bcm_bnl_ts_g.bins_cfg[1].limits),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "bin_cfg2",
		.data		= bcm_bnl_ts_g.bins_cfg[2].limits,
		.maxlen		= sizeof(bcm_bnl_ts_g.bins_cfg[2].limits),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "bin_cfg3",
		.data		= bcm_bnl_ts_g.bins_cfg[3].limits,
		.maxlen		= sizeof(bcm_bnl_ts_g.bins_cfg[3].limits),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "bin_cfg4",
		.data		= bcm_bnl_ts_g.bins_cfg[4].limits,
		.maxlen		= sizeof(bcm_bnl_ts_g.bins_cfg[4].limits),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "bin_cfg5",
		.data		= bcm_bnl_ts_g.bins_cfg[5].limits,
		.maxlen		= sizeof(bcm_bnl_ts_g.bins_cfg[5].limits),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "bin_cfg6",
		.data		= bcm_bnl_ts_g.bins_cfg[6].limits,
		.maxlen		= sizeof(bcm_bnl_ts_g.bins_cfg[6].limits),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "bin_cfg7",
		.data		= bcm_bnl_ts_g.bins_cfg[7].limits,
		.maxlen		= sizeof(bcm_bnl_ts_g.bins_cfg[7].limits),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{ }
};

static int bnl_proc_exit(void)
{
	if (bcm_tcp_ts_proc_activelist_file)
		remove_proc_entry(BNL_PROCFS_TOP_DIR_PATH"/activelist", NULL);

	if (bcm_tcp_ts_proc_donelist_file)
		remove_proc_entry(BNL_PROCFS_TOP_DIR_PATH"/donelist", NULL);

	if (bcm_bnl_proc_stats_file)
		remove_proc_entry(BNL_PROCFS_TOP_DIR_PATH"/stats", NULL);

	if (tcp_ts_sysctl_header)
		unregister_net_sysctl_table(tcp_ts_sysctl_header);

	if (bcm_tcp_ts_proc_top_dir)
		remove_proc_entry(BNL_PROCFS_TOP_DIR_PATH, NULL);

	return 0;
}

static int bnl_proc_init(void)
{
	bcm_tcp_ts_proc_top_dir = proc_mkdir(BNL_PROCFS_TOP_DIR_PATH, NULL);

	if (!bcm_tcp_ts_proc_top_dir) {
		pr_err("%s Unable to create %s proc directory ",
				__func__, BNL_PROCFS_TOP_DIR_PATH);
		goto error;
	}

	bcm_tcp_ts_proc_activelist_file = proc_create_seq_private("activelist", 0440, bcm_tcp_ts_proc_top_dir,
				&bnl_activelist_seq_ops, sizeof(struct activelist_iter_state), NULL);

	if (!bcm_tcp_ts_proc_activelist_file) {
		pr_err("%s Unable to create proc entry for activelist", __func__);
		goto error;
	}

	bcm_tcp_ts_proc_donelist_file = proc_create_seq_private("donelist", 0440, bcm_tcp_ts_proc_top_dir,
				&bnl_donelist_seq_ops, sizeof(struct donelist_iter_state), NULL);

	if (!bcm_tcp_ts_proc_donelist_file) {
		pr_err("%s Unable to create proc entry for donelist", __func__);
		goto error;
	}


	 bcm_bnl_proc_stats_file = proc_create_single(BNL_PROCFS_TOP_DIR_PATH"/stats",
		  S_IRUGO, NULL, bnl_proc_stats_print);

	if (!bcm_bnl_proc_stats_file) {
		pr_err("%s Unable to create proc entry for stats", __func__);
		goto error;
	}

	tcp_ts_sysctl_header = register_sysctl(BNL_PROCFS_TOP_DIR_PATH, tcp_ts_sysctl_table);
	if (!tcp_ts_sysctl_header) {
		pr_err("%s Unable to create tcp_ts_sysctl_table",	__func__);
		goto error;
	}

	return 0;
error:
	bnl_proc_exit();
	return -1;
}

static inline void default_bin_limits_init(void)
{
	int i;

	for (i=0; i<BNL_MAX_BIN_CFGS; i++)
		memcpy(bcm_bnl_ts_g.bins_cfg[i].limits, default_bin_cfg_limits,
				sizeof(bcm_bnl_ts_g.bins_cfg[i].limits));
}

extern int (*blog_bnltrace_fn)(void *arg1, void *arg2, void *arg3);

static int __init bcm_bnl_init(void)
{
	int ret;

	if (bcm_license_check(BP3_FEATURE_BNL) <= 0) {
		pr_err("**** Missing BP3 BNL license, BNL will not work **** \n");
		return -1;
	}

	ret = bnl_proc_init();
	if (ret < 0) {
		pr_err("%s: proc initialization failed\n", __func__);
		return ret;
	}

	default_bin_limits_init();
	bcm_bnl_ts_g.noreply_timeout_ms = BNL_DEFAULT_NOREPLY_TIMEOUT_MS;
	bcm_bnl_ts_g.donelist_clearonread=1;

	rcu_assign_pointer(bcm_bnltrace_ct_event_fn, bcm_bnl_ct_event);
	synchronize_rcu();

	blog_bnltrace_fn = (int (*)(void *arg1, void *arg2, void *arg3))bcm_bnltrace_pkt_rcv;
	return 0;
}

static void __exit bcm_bnl_exit(void)
{
	rcu_assign_pointer(bcm_bnltrace_ct_event_fn, NULL);
	synchronize_rcu();
	
	/*TODO need to update CT's and flush flows */
	blog_bnltrace_fn = NULL;
	bnl_proc_exit();
	return;
}

module_init(bcm_bnl_init);
module_exit(bcm_bnl_exit);
MODULE_LICENSE("GPL");
